#include <pthread.h>
#ifdef _WIN32
#include <WinSock2.h>
#include <ws2tcpip.h>
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#ifndef SOCKET
#if 1
typedef unsigned long long	SOCKET;
#else
typedef long long		SOCKET;
#endif
#endif
#ifndef SOCKET_ERROR
#define INVALID_SOCKET	(SOCKET)(~0)
#define SOCKET_ERROR	(-1)
#endif
#define closesocket(sock) close(sock)
#endif
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <stdarg.h>
#define PLAIN_HTML "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n"
#define PLAIN_DATA "HTTP/1.1 200 OK\r\nContent-Type: application/octet-stream\r\n\r\n"
#define NOT_FOUND "HTTP/1.1 404\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE html><html><head><title>404 Not Found</title></head><body><center><h1>404 Not Found</h1><hr>NChat Server</center></body>"
unsigned short ListenPort = 7900, BlackListCount, WhiteListCount, RealBLC, RealWLC;
char LogBuf[1145], LogBuf2[1145], InvitationCode[256], *BlackList[65546], *WhiteList[65546], EnableBlackList, EnableWhiteList, RoomName[512];
const char *VersionData = "\x1\x1\x8";
struct ULIST{
	char UserName[512];
	SOCKET UserBindClient;
	struct ULIST *Next, *Last;
}*UL_Head, *UL_Last;
unsigned int UsersCount = 0, UserLimit;
SOCKET ListenSocket;
pthread_mutex_t thread_lock;
int send_lock(SOCKET s, const char *buf, int len, int flags) {
	pthread_mutex_lock(&thread_lock);
	int iResult = send(s, buf, len, flags);
	pthread_mutex_unlock(&thread_lock);
	return iResult;
}
#define send send_lock
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
	int i, j;
	while(1) {
		memset(lpstrInput, 0, sizeof(char) * 114514);
		memset(lpstrCommand, 0, sizeof(char) * 32767);
		fgets(lpstrInput, 114514, stdin);
		if(lpstrInput[strlen(lpstrInput) - 1] == '\n') lpstrInput[strlen(lpstrInput) - 1] = '\0';
		int NotSpaceAppeared = 0;
		for(i = 0, j = 0; i < strlen(lpstrInput) ; i += 1) {
			if(NotSpaceAppeared == 0 && lpstrInput[i] != ' ') NotSpaceAppeared = 1;
			if(NotSpaceAppeared == 0) continue;
			if(i != 0 && lpstrInput[i] == ' ' && lpstrInput[i - 1] == ' ') continue;
			else {
				lpstrInput[j] = lpstrInput[i];
				j += 1;
			}
		}
		lpstrInput[j] = '\0';
		if(lpstrInput[j - 1] == ' ') lpstrInput[j - 1] = '\0';
		sscanf(lpstrInput, "%s", lpstrCommand);
		if(strcmp(lpstrCommand, "exit") == 0) {
			//LogOut("Server Thread/INFO", 0, "Stopping the Server");
			closesocket(ListenSocket);/*
#ifdef _WIN32
			WSACleanup();
#endif*/
			return NULL;
		}
		else if(strcmp(lpstrCommand, "list") == 0) {
			LogOut("Server Thread/INFO", 1, "There are %d of a max of %d players online: ", UsersCount, UserLimit);
			struct ULIST* This = UL_Head -> Next;
			while(This != NULL) {
				printf("%s", This -> UserName);
				This = This -> Next;
				if(This != NULL) printf(", ");
			}
			printf("\n");
		}
		else if(strcmp(lpstrCommand, "kick") == 0) {
			if(lpstrInput[4] != ' ') {
				LogOut("Server Thread/ERROR", 0, "Incomplete arguments for command kick!");
				continue;
			}
			struct ULIST *This = UL_Head;
			while(This != NULL) {
				if(strcmp(This -> UserName, lpstrInput + 5) == 0) break;
				This = This -> Next;
			}
			if(This != NULL) {
				This -> Last -> Next = This -> Next;
				if(This -> Next != NULL) This -> Next -> Last = This -> Last;
				send(This -> UserBindClient, "\xC", 1, 0);
				closesocket(This -> UserBindClient);
				LogOut("Server Thread/INFO", 0, "Kicked %s", lpstrInput + 5);
				free(This);
			}
			else LogOut("Server Thread/INFO", 0, "No user named '%s' was found", lpstrInput + 5);
		}
		else if(strcmp(lpstrCommand, "blacklist") == 0) {
			if(lpstrInput[9] != ' ') {
				LogOut("Server Thread/INFO", 0, "Blacklist is %s.", (EnableBlackList == 0) ? "disabled": "enabled");
				//LogOut("Server Thread/ERROR", 0, "Incomplete arguments for command blacklist!");
				continue;
			}
			sscanf(lpstrInput + 10, "%s", lpstrCommand);
			if(strcmp(lpstrCommand, "help") == 0) {
				LogOut("Server Thread/INFO", 0, "blacklist Command: add <UserName>, disable, enable, list, remove <UserName>, help");
			}
			else if(strcmp(lpstrCommand, "add") == 0) {
				if(lpstrInput[13] != ' ') {
					LogOut("Server Thread/ERROR", 0, "Incomplete arguments for command blacklist's option add!");
					continue;
				}
				j = 0;
				for(i = 1; i <= BlackListCount; i += 1) {
					if(BlackList[i] == NULL) {
						j = 1;
						break;
					}
				}
				RealBLC += 1;
				BlackList[i] = (char*)calloc(strlen(lpstrInput + 14) + 10, sizeof(char));
				strcpy(BlackList[i], lpstrInput + 14);
				if(j == 0) BlackListCount += 1;
				LogOut("Server Thread/INFO", 0, "User %s was added to the blacklist.", BlackList[i]);
			}
			else if(strcmp(lpstrCommand, "remove") == 0) {
				if(lpstrInput[16] != ' ') {
					LogOut("Server Thread/ERROR", 0, "Incomplete arguments for command blacklist's option remove!");
					continue;
				}
				j = 0;
				for(i = 1; i <= BlackListCount; i += 1) {
					if(BlackList[i] != NULL && strcmp(BlackList[i], lpstrInput + 17) == 0) {
						j = 1;
						break;
					}
				}
				if(j == 1) {
					RealBLC -= 1;
					LogOut("Server Thread/INFO", 0, "User %s was removed from the blacklist.", BlackList[i]);
					BlackList[i] = NULL;
				}
				else LogOut("Server Thread/ERROR", 0, "User %s is not in the blacklist.", lpstrInput + 17);
			}
			else if(strcmp(lpstrCommand, "list") == 0) {
				LogOut("Server Thread/INFO", 1, "There are %d user in the blacklist: ", RealBLC);
				j = 0;
				for(i = 1; i <= BlackListCount; i += 1) {
					if(BlackList[i] != NULL) {
						if(j == 1) printf(", ");
						printf("%s", BlackList[i]);
						j = 1;
					}
				}
				printf("\n");
			}
			else if(strcmp(lpstrCommand, "enable") == 0) {
				EnableBlackList = 1;
				LogOut("Server Thread/INFO", 0, "Blacklist was already enabled just now.");
			}
			else if(strcmp(lpstrCommand, "disable") == 0) {
				EnableBlackList = 0;
				LogOut("Server Thread/INFO", 0, "Blacklist was already disabled just now.");
			}
			else LogOut("Server Thread/ERROR", 0, "blacklist %s<-- Unknown Option", lpstrCommand);
		}
		else if(strcmp(lpstrCommand, "whitelist") == 0) {
			if(lpstrInput[9] != ' ') {
				LogOut("Server Thread/INFO", 0, "Whitelist is %s.", (EnableWhiteList == 0) ? "disabled": "enabled");
				//LogOut("Server Thread/ERROR", 0, "Incomplete arguments for command whitelist!");
				continue;
			}
			sscanf(lpstrInput + 10, "%s", lpstrCommand);
			if(strcmp(lpstrCommand, "help") == 0) {
				LogOut("Server Thread/INFO", 0, "whitelist Command: add <UserName>, disable, enable, list, remove <UserName>, help");
			}
			else if(strcmp(lpstrCommand, "add") == 0) {
				if(lpstrInput[13] != ' ') {
					LogOut("Server Thread/ERROR", 0, "Incomplete arguments for command whitelist's option add!");
					continue;
				}
				j = 0;
				for(i = 1; i <= WhiteListCount; i += 1) {
					if(WhiteList[i] == NULL) {
						j = 1;
						break;
					}
				}
				RealWLC += 1;
				WhiteList[i] = (char*)calloc(strlen(lpstrInput + 14) + 10, sizeof(char));
				strcpy(WhiteList[i], lpstrInput + 14);
				if(j == 0) WhiteListCount += 1;
				LogOut("Server Thread/INFO", 0, "User %s was added to the whitelist.", WhiteList[i]);
			}
			else if(strcmp(lpstrCommand, "remove") == 0) {
				if(lpstrInput[16] != ' ') {
					LogOut("Server Thread/ERROR", 0, "Incomplete arguments for command whitelist's option remove!");
					continue;
				}
				j = 0;
				for(i = 1; i <= WhiteListCount; i += 1) {
					if(WhiteList[i] != NULL && strcmp(WhiteList[i], lpstrInput + 17) == 0) {
						j = 1;
						break;
					}
				}
				if(j == 1) {
					RealWLC -= 1;
					LogOut("Server Thread/INFO", 0, "User %s was removed from the whitelist.", WhiteList[i]);
					WhiteList[i] = NULL;
				}
				else LogOut("Server Thread/ERROR", 0, "User %s is not in the whitelist.", lpstrInput + 17);
			}
			else if(strcmp(lpstrCommand, "list") == 0) {
				LogOut("Server Thread/INFO", 1, "There are %d user in the whitelist: ", RealWLC);
				j = 0;
				for(i = 1; i <= WhiteListCount; i += 1) {
					if(WhiteList[i] != NULL) {
						if(j == 1) printf(", ");
						printf("%s", WhiteList[i]);
						j = 1;
					}
				}
				printf("\n");
			}
			else if(strcmp(lpstrCommand, "enable") == 0) {
				EnableWhiteList = 1;
				LogOut("Server Thread/INFO", 0, "Whitelist was already enabled just now.");
			}
			else if(strcmp(lpstrCommand, "disable") == 0) {
				EnableWhiteList = 0;
				LogOut("Server Thread/INFO", 0, "Whitelist was already disabled just now.");
			}
			else LogOut("Server Thread/ERROR", 0, "whitelist %s<-- Unknown Option", lpstrCommand);
		}
		else if(strcmp(lpstrCommand, "ban") == 0) {
			if(lpstrInput[3] != ' ') {
				LogOut("Server Thread/ERROR", 0, "Incomplete arguments for command ban!");
				continue;
			}
			struct ULIST *This = UL_Head;
			while(This != NULL) {
				if(strcmp(This -> UserName, lpstrInput + 4) == 0) break;
				This = This -> Next;
			}
			if(This != NULL) {
				This -> Last -> Next = This -> Next;
				if(This -> Next != NULL) This -> Next -> Last = This -> Last;
				send(This -> UserBindClient, "\xC", 1, 0);
				closesocket(This -> UserBindClient);
				j = 0;
				for(i = 1; i <= BlackListCount; i += 1) {
					if(BlackList[i] == NULL) {
						j = 1;
						break;
					}
				}
				RealBLC += 1;
				BlackList[i] = (char*)calloc(strlen(lpstrInput + 4) + 10, sizeof(char));
				strcpy(BlackList[i], lpstrInput + 4);
				if(j == 0) BlackListCount += 1;
				LogOut("Server Thread/INFO", 0, "Banned %s", lpstrInput + 4);
				free(This);
			}
			else LogOut("Server Thread/INFO", 0, "No user named '%s' was found", lpstrInput + 5);
		}
		else if(strcmp(lpstrCommand, "limit") == 0) {
			if(lpstrInput[5] != ' ') {
				LogOut("Server Thread/INFO", 0, "User Maximum Limit: %d.", UserLimit);
				continue;
			}
			sscanf(lpstrInput + 6, "%d", &UserLimit);
			LogOut("Server Thread/INFO", 0, "User Maximum Limit was set to %d.", UserLimit);
		}
		else if(strcmp(lpstrCommand, "roomname") == 0) {
			if(lpstrInput[8] != ' ') {
				LogOut("Server Thread/INFO", 0, "Chat Room Name: %s", RoomName);
				continue;
			}
			if(strlen(lpstrInput + 9) > 255) {
				LogOut("Server Thread/ERROR", 0, "Chat Room Name Length can't be longer than 255");
				continue;
			}
			strcpy(RoomName, lpstrInput + 9);
			LogOut("Server Thread/INFO", 0, "Chat Room Name has been changed to %s", RoomName);
		}
		else if(strcmp(lpstrCommand, "say") == 0) {
			if(lpstrInput[3] != ' ') {
				LogOut("Server Thread/ERROR", 0, "Incomplete arguments for command say!");
				continue;
			}
			struct ULIST *This = UL_Head;
			int iLength = strlen(lpstrInput + 4);
			while(This != NULL) {
				send(This -> UserBindClient, "\xF\x6Server", 8, 0);
				send(This -> UserBindClient, (const char*)&iLength, sizeof(iLength), 0);
				send(This -> UserBindClient, lpstrInput + 4, iLength, 0);
				This = This -> Next;
			}
			LogOut("Server Thread/INFO", 0, "<Server> %s", lpstrInput + 4);
		}
		else if(strcmp(lpstrCommand, "help") == 0) {
			LogOut("Server Thread/INFO", 0, "Commands: \n\
  ban -> Ban a user(ban = blacklist add + kick), Method: ban <UserName: String>\n\
  blacklist -> Manage the blacklist, Method: blacklist <add, disable, enable, help, list, remove> [User Name(Only add or remove option): String]\n\
  exit -> Stop the server, Method: exit\n\
  help -> Show this help text, Method: help\n\
  limit -> Set or query a limit on the number of room users, Method: limit [New Limit on the Number of Users: Int]\n\
  list -> List the users, Method: list\n\
  kick -> Kick user out of the room, Method: kick <User Name: String>\n\
  roomname -> Set or query the chat room name, Method: roomname [New Room Name: String]\n\
  whitelist -> Manage the whitelist, Method: whitelist <add, disable, enable, help, list, remove> [User Name(Only add or remove option): String]");
		}
		else {
			LogOut("Server Thread/ERROR", 0, "%s<--Unknown Command", lpstrCommand);
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
			if(ReceiveData[0] != '\xA' && ReceiveData[0] != 'G' && ReceiveData[0] != '\xE' && ReceiveData[0] != '\x9' && beLogined == 0) {
				send(ClientSocket, "\x4|ERR_LOGIN_FIRST", 17, 0);//The client has problem could do this
			}
			else switch(ReceiveData[0]) {
				case 0x9: {//For testing is keeping alive
					send(ClientSocket, "\x9", 1, 0);
					break;
				}
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
							if(UsersCount == UserLimit) {
								send(ClientSocket, "\x7|ERR_USERS_MAXLIMIT", 20, 0);
								closesocket(ClientSocket);
								return NULL;
							}
							unsigned char BytesOfUserName, State = 0x00;
							recv(ClientSocket, (char*)&BytesOfUserName, 1, 0);
							UL_New = (struct ULIST*)calloc(1, sizeof(struct ULIST));
							UL_Last -> Next = UL_New;
							UL_New -> Last = UL_Last;
							recv(ClientSocket, UL_New -> UserName, BytesOfUserName, 0);
							int i;
							if(EnableWhiteList == 1){
								for(i = 1; i <= WhiteListCount; i += 1) {
									if(WhiteList[i] != NULL && strcmp(UL_New -> UserName, WhiteList[i]) == 0) {
										State = 0x00;
										break;
									} 
								}
								if(i == WhiteListCount + 1) State = 0x01;
							}
							if(State == 0x00 && EnableBlackList == 1) {
								for(i = 1; i <= BlackListCount; i += 1) {
									if(BlackList[i] != NULL && strcmp(UL_New -> UserName, BlackList[i]) == 0) {
										State = 0x02;
										break;
									}
								}
							}
							if(State == 0x01) {
								send(ClientSocket, "\x5|ERR_NOTON_WHITELIST", 21, 0);
								closesocket(ClientSocket);
								UL_New -> Last -> Next = UL_New -> Next;
								if(UL_New -> Next != NULL) UL_New -> Next -> Last = UL_New -> Last;
								free(UL_New);
								return NULL;
							}
							else if(State == 0x02) {
								send(ClientSocket, "\x6|ERR_ON_BLACKLIST", 18, 0);
								closesocket(ClientSocket);
								UL_New -> Last -> Next = UL_New -> Next;
								if(UL_New -> Next != NULL) UL_New -> Next -> Last = UL_New -> Last;
								free(UL_New);
								return NULL;
							}
							UL_New -> UserBindClient = ClientSocket;
							UL_Last = UL_New;
							send(ClientSocket, "\x1|OK_CONNECTION", 16, 0);
							LogOut("Server Thread/INFO", 0, "%s joined the chat room", UL_New -> UserName);
							struct ULIST *This = UL_Head -> Next;
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
#ifdef _WIN32
					__int64 iSize;
#else
					__int64_t iSize;//Linux just use __int64_t
#endif
					unsigned int iLen, iCount = 0;
					struct ULIST *This = UL_Head -> Next;
					char SendMsg[4];
					SendMsg[0] = '\xF';
					SendMsg[1] = strlen(UL_New -> UserName);
					recv(ClientSocket, (char*)&iLen, sizeof(iLen), 0);
					while(This != NULL) {
						send(This -> UserBindClient, SendMsg, 2, 0);
						send(This -> UserBindClient, UL_New -> UserName, SendMsg[1], 0);
						send(This -> UserBindClient, (const char*)&iLen, sizeof(iLen), 0);
						This = This -> Next;
					}
					printf("<%s> ", UL_New -> UserName);//Who sent the message
					while(iCount < iLen) {
						memset(ReceiveData, 0, sizeof(ReceiveData));
						iResult = recv(ClientSocket, ReceiveData, sizeof(ReceiveData) - 1, 0);//Read Message
						if(iResult <= 0) break;
						iCount += iResult;
						printf("%s", ReceiveData);//Print Message
						This = UL_Head -> Next;
						while(This != NULL) {
							send(This -> UserBindClient, ReceiveData, iResult, 0);//Send Message
							This = This -> Next;
						}
						//if(iResult < sizeof(ReceiveData) - 1) break;
					}
					printf("\n");
					break;
				}
				case 0xC: {//Send Message(File) 
					
					break;
				}
				case 0xD: {//List Users, no receiving anything
					struct ULIST *This = UL_Head -> Next;
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
				case 0xE: {//Query some information from server
					send(ClientSocket, "\xE", 1, 0);
					send(ClientSocket, (const char*)&UsersCount, sizeof(UsersCount), 0);
					send(ClientSocket, (const char*)&UserLimit, sizeof(UserLimit), 0);
					char Length = strlen(RoomName);
					send(ClientSocket, &Length, 1, 0);
					send(ClientSocket, RoomName, Length, 0);
					break;
				}
				case 0xF: {//Exit
					closesocket(ClientSocket);
					break;
				}
				case 'G': {//For webBrowser view
					char RealPath[1145];
					memset(RealPath, 0, sizeof(RealPath));
					strcpy(RealPath, "./Files");
					recv(ClientSocket, ReceiveData, sizeof(ReceiveData), 0);
					sscanf(ReceiveData, "ET %s HTTP/1.1", RealPath + strlen(RealPath));
					LogOut("Server Thread/INFO", 0, "WebBrowser sent a request: GET %s HTTP/1.1", RealPath + 7);
					int i, ReadData;
#ifdef _WIN32
					for(i = 0; i < strlen(RealPath); i++) {
						if(RealPath[i] == '/') RealPath[i] = '\\';
					}
#endif
					FILE* lpFile;
					if(strcmp(RealPath, ".\\Files\\") == 0 || strcmp(RealPath, "./Files/") == 0) {
						lpFile = fopen(
#ifdef _WIN32
						".\\Files\\dl.html"
#else
						"./Files/dl.html"
#endif
						, "rb");
					}
					else lpFile = fopen(RealPath + 2, "rb");
					if(lpFile == NULL) {
						send(ClientSocket, NOT_FOUND, strlen(NOT_FOUND), 0);
						closesocket(ClientSocket);
						return NULL;
					}
					if(strcmp(RealPath, ".\\Files\\") == 0 || strcmp(RealPath, "./Files/") == 0) {
						send(ClientSocket, PLAIN_HTML, strlen(PLAIN_HTML), 0);
					}
					else send(ClientSocket, PLAIN_DATA, strlen(PLAIN_DATA), 0);
					do {
						ReadData = fread(ReceiveData, sizeof(char), sizeof(ReceiveData) / sizeof(char), lpFile);
						send(ClientSocket, ReceiveData, ReadData, 0);
					} while(ReadData > 0);
					fclose(lpFile);
					closesocket(ClientSocket);
					return NULL;
					break;
				}
				default: {
					closesocket(ClientSocket);
					return NULL;
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
		struct ULIST *This = UL_Head -> Next;
		char SendMsg[4];
		SendMsg[0] = '\xB';
		SendMsg[1] = strlen(UL_New -> UserName);
		while(This != NULL) {
			send(This -> UserBindClient, SendMsg, 2, 0);
			send(This -> UserBindClient, UL_New -> UserName, SendMsg[1], 0);
			This = This -> Next;
		}
		free(UL_New);
		UsersCount -= 1;
	}
	//send(ClientSocket, "\x1|OK_LOGOUT", 11, 0);
	return NULL;
}
int main() {
	int iSendResult, iResult, iEnum, iTmp;
	unsigned char ICSize, ICSize2;
	FILE *lpConfig = fopen(
#ifdef _WIN32
	"configs\\config.nchatserver"
#else
	"configs/config.nchatserver"
#endif
	, "rb");
	if(lpConfig != NULL) {
		fread(InvitationCode, sizeof(char), 25, lpConfig);
		if(memcmp(InvitationCode, "NSV\xFFN\0C\0H\0A\0T\0V\0E\0R\0I\0F\0Y", 25) == 0) {
			fread(&ListenPort, sizeof(ListenPort), 1, lpConfig);
			fread(&ICSize, sizeof(ICSize), 1, lpConfig);
			ICSize2 = ICSize;
			if(ICSize > 128) {
				LogOut("Server Thread/WARN", 0, "The size of InvitationCode cannot be larger than 128.");
				for(iEnum = 0; iEnum < 128; iEnum++) {
					int iRand = rand() % 62;
					if(iRand < 26) InvitationCode[iEnum] = iRand + 'A';
					else if(iRand < 36) InvitationCode[iEnum] = iRand + '0' - 26;
					else InvitationCode[iEnum] = iRand + 'a' - 36;
				}
			}
			else fread(InvitationCode, sizeof(char), ICSize, lpConfig);
			fseek(lpConfig, 284, SEEK_SET);
			fread(&UserLimit, sizeof(UserLimit), 1, lpConfig);
			fread(RoomName, sizeof(char), 256, lpConfig);
		} 
		else LogOut("Server Thread/WARN", 0, "Illegal config file.");
		fclose(lpConfig);
		lpConfig = fopen(
#ifdef _WIN32
	"configs\\blacklist.nchatserver"
#else
	"configs/blacklist.nchatserver"
#endif
	, "rb");
		if(lpConfig != NULL) {
			fread(&iTmp, sizeof(char), 1, lpConfig);
			EnableBlackList = (iTmp & 1);
			fread(&BlackListCount, sizeof(BlackListCount), 1, lpConfig);
			RealBLC = BlackListCount;
			for(iEnum = 1; iEnum <= BlackListCount; iEnum += 1) {
				fread(&ICSize, sizeof(char), 1, lpConfig);
				BlackList[iEnum] = (char*)calloc(ICSize + 10, sizeof(char));
				fread(BlackList[iEnum], sizeof(char), ICSize + 1, lpConfig);
				if(BlackList[iEnum][ICSize] != 0x3E) LogOut("Server Thread/WARN", 0, "There is a mistake in white list: The %dth blacklist user should have 0x3E as the terminator.", iEnum);
				BlackList[iEnum][ICSize] = '\0';
			}
			fclose(lpConfig);
		}
		lpConfig = fopen(
#ifdef _WIN32
	"configs\\whitelist.nchatserver"
#else
	"configs/whitelist.nchatserver"
#endif
	, "rb");
		if(lpConfig != NULL) {
			fread(&iTmp, sizeof(char), 1, lpConfig);
			EnableWhiteList = (iTmp & 1);
			fread(&WhiteListCount, sizeof(WhiteListCount), 1, lpConfig);
			RealWLC = WhiteListCount;
			for(iEnum = 1; iEnum <= WhiteListCount; iEnum += 1) {
				fread(&ICSize, sizeof(char), 1, lpConfig);
				WhiteList[iEnum] = (char*)calloc(ICSize + 10, sizeof(char));
				fread(WhiteList[iEnum], sizeof(char), ICSize + 1, lpConfig);
				if(WhiteList[iEnum][ICSize] != 0x3F) LogOut("Server Thread/WARN", 0, "There is a mistake in white list: The %dth whitelist user should have 0x3F as the terminator.", iEnum);
				WhiteList[iEnum][ICSize] = '\0';
			}
			fclose(lpConfig);
		}
	}
	else for(iEnum = 0; iEnum < 128; iEnum++) {
		int iRand = rand() % 62;
		if(iRand < 26) InvitationCode[iEnum] = iRand + 'A';
		else if(iRand < 36) InvitationCode[iEnum] = iRand + '0' - 26;
		else InvitationCode[iEnum] = iRand + 'a' - 36;
	}
	//strcpy(InvitationCode, "o6kYMTvGkhFhZBg8QxAUeqLuKkw4sSaCOxgJ9urA0Rb3jQ5FTQ5Vr41JyVgm8VkrE3ZARVQaTn0huDTkNunBKP9pbdmBUrry9uxZopG5IkoX1BnHkeQeLqFIDl6x8nqn");
	UL_Head = UL_Last = (struct ULIST*)calloc(1, sizeof(struct ULIST));
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
	SOCKET ClientSocket;
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
	SockAddr.sin_addr.s_addr = htonl(INADDR_ANY); 
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
    pthread_mutex_init(&thread_lock, NULL);
    pthread_t ThreadId;
    pthread_create(&ThreadId, NULL, InputThread, NULL);
	while(1) {
		ClientSocket = accept(ListenSocket, NULL, NULL);
	    if(ClientSocket == INVALID_SOCKET) {
	        //closesocket(ListenSocket);
#ifdef _WIN32
			WSACleanup();
#endif
			break;
		}
		pthread_t Handler;
		pthread_create(&Handler, NULL, SocketHandler, (void*)ClientSocket); 
	}
	closesocket(ListenSocket);
	pthread_mutex_destroy(&thread_lock);
#ifdef _WIN32 //Windows Clean Up
	WSACleanup();
#endif
	LogOut("Server Close/INFO", 0, "Stopping the Server");
	lpConfig = fopen(
#ifdef _WIN32
	"configs\\config.nchatserver"
#else
	"configs/config.nchatserver"
#endif
	, "wb");
	if(lpConfig != NULL) {
		fwrite("NSV\xFFN\0C\0H\0A\0T\0V\0E\0R\0I\0F\0Y", sizeof(char), 25, lpConfig);
		fwrite(&ListenPort, sizeof(ListenPort), 1, lpConfig);
		fwrite(&ICSize2, sizeof(ICSize2), 1, lpConfig);
		fwrite(InvitationCode, sizeof(char), 256, lpConfig);
		fwrite(&UserLimit, sizeof(UserLimit), 1, lpConfig);
		fwrite(RoomName, sizeof(char), 256, lpConfig);
		fclose(lpConfig);
		LogOut("Server Close/INFO", 0, "Main Config file saved.");
	}
	lpConfig = fopen(
#ifdef _WIN32
	"configs\\blacklist.nchatserver"
#else
	"configs/blacklist.nchatserver"
#endif
	, "wb");
	if(lpConfig != NULL) {
		fwrite(&EnableBlackList, sizeof(char), 1, lpConfig);
		fwrite(&RealBLC, sizeof(RealBLC), 1, lpConfig);
		for(iEnum = 1; iEnum <= BlackListCount; iEnum += 1) {
			if(BlackList[iEnum] != NULL) {
				iTmp = strlen(BlackList[iEnum]);
				fwrite(&iTmp, sizeof(char), 1, lpConfig);
				fwrite(BlackList[iEnum], sizeof(char), iTmp, lpConfig);
				fwrite("\x3E", sizeof(char), 1, lpConfig);
			}
		}
		fclose(lpConfig);
		LogOut("Server Close/INFO", 0, "Blacklist saved.");
	}
	lpConfig = fopen(
#ifdef _WIN32
	"configs\\whitelist.nchatserver"
#else
	"configs/whitelist.nchatserver"
#endif
	, "wb");
	if(lpConfig != NULL) {
		fwrite(&EnableWhiteList, sizeof(char), 1, lpConfig);
		fwrite(&RealWLC, sizeof(RealWLC), 1, lpConfig);
		for(iEnum = 1; iEnum <= WhiteListCount; iEnum += 1) {
			if(WhiteList[iEnum] != NULL) {
				iTmp = strlen(WhiteList[iEnum]);
				fwrite(&iTmp, sizeof(char), 1, lpConfig);
				fwrite(WhiteList[iEnum], sizeof(char), iTmp, lpConfig);
				fwrite("\x3F", sizeof(char), 1, lpConfig);
			}
		}
		fclose(lpConfig);
		LogOut("Server Close/INFO", 0, "Whitelist saved.");
	}
	return 0;
}
