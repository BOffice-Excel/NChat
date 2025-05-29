#include <pthread.h>
#include <WinSock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
short ListenPort = 7900;
char LogBuf[1145], LogBuf2[1145], InvitationCode[256];
const char *VersionData = "\x1\x0\x1";
struct ULIST{
	char UserName[512];
	SOCKET UserBindClient;
	struct ULIST *Next, *Last;
}*UL_Head, *UL_Last;
int UsersCount = 0;
void LogOut(const char* Poster, int NoNewLine, const char* Format, ...) {
	va_list v;
	va_start(v, Format);
	time_t CurrentTime;
	time(&CurrentTime);
	struct tm* localTime;
    localTime = localtime(&CurrentTime);
    strftime(LogBuf, sizeof(LogBuf), "%H:%M:%S", localTime);
    vsprintf(LogBuf2, Format, v);
    printf("[%s] [%s]: %s", LogBuf, Poster, LogBuf2);
    if(NoNewLine == 0) printf("\n");
	va_end(v);
	return ;
}
void* InputThread(void* lParam) {
	char *lpstrInput = (char*)calloc(114514, sizeof(char)), *lpstrCommand = (char*)calloc(32767, sizeof(char));
	while(1) {
		memset(lpstrInput, 0, sizeof(char) * 114514);
		gets(lpstrInput);
		sscanf(lpstrInput, "%s", lpstrCommand);
		if(strcmp(lpstrCommand, "exit") == 0) {
			LogOut("Server Thread/INFO", 0, "Stopping the Server");
#ifdef _WIN32
			WSACleanup();
#endif
			exit(0);
		}
		else if(strcmp(lpstrCommand, "list") == 0) {
			LogOut("Server Thread/INFO", 1, "There are %d people in the chat room: ", UsersCount);
			struct ULIST* This = UL_Head -> Next;
			while(This != NULL) {
				printf("%s", This -> UserName);
				This = This -> Next;
				if(This != NULL) printf(", ");
			}
			printf("\n");
		}
		else {
			LogOut("Server Thread/ERROR", 0, "%s <--Unknown Command", lpstrInput);
		}
	}
	return NULL;
}
void* SocketHandler(void* lParam) {
	SOCKET ClientSocket = (SOCKET)lParam;
	char ReceiveData[32767];
	int beLogined = 0;
	struct ULIST* UL_New = NULL;
	while(1) {
		memset(ReceiveData, 0, sizeof(ReceiveData));
		int iResult = recv(ClientSocket, ReceiveData, 1, 0);
		if(iResult <= 0 || iResult == SOCKET_ERROR) {
			closesocket(ClientSocket);
			break;
		}
		else if(iResult > 0) {
			switch(ReceiveData[0]) {
				case 0xA: {//Login, Verify invitation code
					if(beLogined == 0) {
						recv(ClientSocket, ReceiveData, 3, 0);
						if(memcmp(ReceiveData, VersionData, 3) != 0) {
							send(ClientSocket, "\x3|ERR_VERSION_MISMATCH", 22, 0);
							closesocket(ClientSocket);
							return NULL;
						}
						recv(ClientSocket, ReceiveData, 128, 0);
						if(strcmp(ReceiveData, InvitationCode) == 0) {
							char BytesOfUserName;
							recv(ClientSocket, &BytesOfUserName, 1, 0);
							UL_New = (struct ULIST*)calloc(1, sizeof(ULIST));
							UL_Last -> Next = UL_New;
							UL_New -> Last = UL_Last;
							UL_Last = UL_New;
							recv(ClientSocket, UL_New -> UserName, BytesOfUserName, 0);
							UL_New -> UserBindClient = ClientSocket;
							send(ClientSocket, "\x1|OK_CONNECTION", 16, 0);
							LogOut("Server Thread/INFO", 0, "%s joined the chat room", UL_New -> UserName);
							ULIST *This = UL_Head -> Next;
							char SendMsg[4];
							SendMsg[0] = '\xA';
							SendMsg[1] = strlen(UL_New -> UserName);
							while(This != NULL) {
								send(This -> UserBindClient, SendMsg, 2, 0);
								send(This -> UserBindClient, UL_New -> UserName, SendMsg[1], 0);
								This = This -> Next;
							}
							UsersCount += 1; 
							beLogined = 1;
						}
						else {
							send(ClientSocket, "\x2|ERR_CONNECTION_REFUSED", 24, 0);
							closesocket(ClientSocket);
							return NULL;
						}
					}
					else {
						send(ClientSocket, "\xA|ERR_CONNECTION_AGAIN", 22, 0);
						LogOut("Server Thread/WARN", 0, "%s tried to join the chat room again!", UL_New -> UserName);
						closesocket(ClientSocket);
						return NULL;
					}
					break;
				}
				case 0xB: {//Send Message(Text)
					__int64 iSize;
					ULIST *This = UL_Head -> Next;
					char SendMsg[4];
					SendMsg[0] = '\xF';
					SendMsg[1] = strlen(UL_New -> UserName);
					while(This != NULL) {
						send(This -> UserBindClient, SendMsg, 2, 0);
						send(This -> UserBindClient, UL_New -> UserName, SendMsg[1], 0);
						This = This -> Next;
					}
					printf("<%s> ", UL_New -> UserName);
					while(1) {
						memset(ReceiveData, 0, sizeof(ReceiveData));
						iResult = recv(ClientSocket, ReceiveData, sizeof(ReceiveData) - 1, 0);
						if(iResult <= 0) break;
						printf("%s", ReceiveData);
						This = UL_Head -> Next;
						while(This != NULL) {
							send(This -> UserBindClient, ReceiveData, iResult, 0);
							This = This -> Next;
						}
						if(iResult < sizeof(ReceiveData) - 1) break;
					}
					printf("\n");
					break;
				}
				case 0xC: {//Send Message(File) 
					
					break;
				}
				case 0xD: {//List Users
					ULIST *This = UL_Head -> Next;
					send(ClientSocket, "\xD", 1, 0);
					send(ClientSocket, (const char*)&UsersCount, sizeof(UsersCount), 0);
					while(This != NULL) {
						char SData = strlen(This -> UserName);
						send(ClientSocket, &SData, 1, 0);
						send(ClientSocket, This -> UserName, SData, 0);
						This = This -> Next;
					}
					//send(ClientSocket, "", 1, 0);
					break;
				}
				case 0xF: {//Exit
					closesocket(ClientSocket);
					break;
				}
			}
		}
	}
	if(beLogined == 1 && UL_New != NULL) {
		if(UL_New -> Next != NULL) UL_New -> Next -> Last = UL_New -> Last;
		else UL_Last = UL_New -> Last;
		UL_New -> Last -> Next = UL_New -> Next;
		LogOut("Server Thread/INFO", 0, "%s left the chat room", UL_New -> UserName);
		ULIST *This = UL_Head -> Next;
		char SendMsg[4];
		SendMsg[0] = '\xB';
		SendMsg[1] = strlen(UL_New -> UserName);
		while(This != NULL) {
			send(This -> UserBindClient, SendMsg, 2, 0);
			send(This -> UserBindClient, UL_New -> UserName, SendMsg[1], 0);
			This = This -> Next;
		}
		free(UL_New);
	}
	UsersCount -= 1;
	//send(ClientSocket, "\x1|OK_LOGOUT", 11, 0);
	return NULL;
}
int main() {
	int iSendResult, iResult, iEnum;
	srand(time(NULL));
	/*for(iEnum = 0; iEnum < 128; iEnum++) {
		int iRand = rand() % 62;
		if(iRand < 26) InvitationCode[iEnum] = iRand + 'A';
		else if(iRand < 36) InvitationCode[iEnum] = iRand + '0' - 26;
		else InvitationCode[iEnum] = iRand + 'a' - 36;
	}*/
	strcpy(InvitationCode, "o6kYMTvGkhFhZBg8QxAUeqLuKkw4sSaCOxgJ9urA0Rb3jQ5FTQ5Vr41JyVgm8VkrE3ZARVQaTn0huDTkNunBKP9pbdmBUrry9uxZopG5IkoX1BnHkeQeLqFIDl6x8nqn");
	UL_Head = UL_Last = (struct ULIST*)calloc(1, sizeof(ULIST));
	LogOut("Server Startup/INFO", 0, "Welcome to NChat - Server(Version %d.%d.%d)", (unsigned char)VersionData[0], (unsigned char)VersionData[1], (unsigned char)VersionData[2]);
	LogOut("Server Startup/INFO", 0, "Loading config file...");
#ifdef _WIN32 //Windows Server
	WSADATA wsaData;
	if(WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		LogOut("Server Startup/ERROR", 0, "WSAStartup failed!");
		return -1;
	}
	LogOut("Server Startup/INFO", 0, "Initialized WinSock2 successfully.");
#endif //_WIN32
	SOCKET ListenSocket, ClientSocket;
	ListenSocket = socket(AF_INET, SOCK_STREAM, 0);
	if(ListenSocket == INVALID_SOCKET) {
		LogOut("Server Startup/ERROR", 0, "Socket Failed!");
#ifdef _WIN32
		WSACleanup();
#endif //_WIN32
		return 1;
	}
	LogOut("Server Startup/INFO", 0, "Create socket successfully.");
	struct sockaddr_in SockAddr;
	SockAddr.sin_family = AF_INET;
	SockAddr.sin_port = htons(ListenPort);
	SockAddr.sin_addr.s_addr = inet_addr("127.0.0.1"); 
	iResult = bind(ListenSocket, (struct sockaddr*)&SockAddr, sizeof(SockAddr));
	if(iResult == SOCKET_ERROR) {
		LogOut("Server Startup/ERROR", 0, "Bind Listening Failed!");
		closesocket(ListenSocket);
#ifdef _WIN32
		WSACleanup();
#endif //_WIN32
		return 1;
	}
	LogOut("Server Startup/INFO", 0, "Bind Listening Port successfully.");
	iResult = listen(ListenSocket, 114514);
	if(iResult == SOCKET_ERROR) {
		LogOut("Server Startup/ERROR", 0, "Listen Failed!");
		closesocket(ListenSocket);
#ifdef _WIN32
		WSACleanup();
#endif
		return 1;
	}
	LogOut("Server Startup/INFO", 0, "Set queue of listening successfully.");
	LogOut("Server Startup/INFO", 0, "Your Invitation Code is %s.", InvitationCode);
    LogOut("Server Startup/INFO", 0, "Listening on http://localhost:%d...", ListenPort);
    pthread_t ThreadId;
    pthread_create(&ThreadId, NULL, InputThread, NULL);
	while(1) {
		ClientSocket = accept(ListenSocket, NULL, NULL);
	    if(ClientSocket == INVALID_SOCKET) {
	        LogOut("Server Thread/ERROR", 0, "Accept Failed!");
	        closesocket(ListenSocket);
#ifdef _WIN32
			WSACleanup();
#endif
			return 1;
		}
		pthread_t Handler;
		pthread_create(&Handler, NULL, SocketHandler, (void*)ClientSocket); 
	}
	closesocket(ListenSocket);
#ifdef _WIN32 //Windows Clean Up
	WSACleanup();
#endif
	LogOut("Server Thread/INFO", 0, "Stopping the Server");
	return 0;
}
