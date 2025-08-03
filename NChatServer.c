#define _FILE_OFFSET_BITS 64
#include "Plugins/NChatServerPluginAPI.h"
#include <pthread.h>
//#include <dlfcn.h>
#ifdef _WIN32
#include <WinSock2.h>
#include <ws2tcpip.h>
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <dlfcn.h>
#include <dirent.h> 
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
#include <errno.h>
#define PLAIN_HTML "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n"
#define PLAIN_DATA "HTTP/1.1 200 OK\r\nContent-Type: application/octet-stream\r\n\r\n"
#define NOT_FOUND "HTTP/1.1 404\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE html><html><head><title>404 Not Found</title></head><body><center><h1>404 Not Found</h1><hr>NChat Server</center></body>"
PLUGININTERFACE PluginInterface;
typedef void (*PLUGININITFUNC)(LPPLUGININTERFACE, char*);
char *PluginsName[32767];
char PathToTrieConfigs[256 + 32];
typedef struct tagTRIETWIGS{
	int size;
	char *Value;
	struct tagTRIETWIGS *Next[132];
}TRIETWIGS, *LPTRIETWIGS;
LPTRIETWIGS lpTrieRoot = NULL;
int GetTrieValue(const char* Key, char* Value, LPTRIETWIGS lpRoot) {
	int i;
	LPTRIETWIGS lpTrieThis = lpRoot;
	for(i = 0; i < strlen(Key); i += 1) {
		lpTrieThis = lpTrieThis -> Next[Key[i]];
		if(lpTrieThis == NULL) return -1;
	}
	if(lpTrieThis -> Value == NULL) return -2;
	strcpy(Value, lpTrieThis -> Value);
	return 0;
	/*if(lpRoot == NULL) return -1;
	if(Key[0] == '\0') {
		if(lpRoot -> Value != NULL) {
			strcpy(Value, lpRoot -> Value);
			return 0;
		}
		return -1;
	}
	return GetTrieValue(Key + 1, Value, lpRoot -> Next[Key[0]]);*/
}
void SaveTrieConfigs(FILE* lpFile, LPTRIETWIGS lpRoot, int deep) {
	if(lpRoot == NULL) return ;
	if(lpRoot -> Value != NULL) {
		fwrite(&deep, sizeof(char), 1, lpFile);
		fwrite(PathToTrieConfigs, sizeof(char), deep, lpFile);
		int DataLength = strlen(lpRoot -> Value);
		fwrite(&DataLength, sizeof(DataLength), 1, lpFile);
		fwrite(lpRoot -> Value, sizeof(char), DataLength, lpFile);
	}
	int i;
	for(i = 0; i <= 128; i += 1) {
		if(lpRoot -> Next[i] != NULL) {
			PathToTrieConfigs[deep] = i;
			SaveTrieConfigs(lpFile, lpRoot -> Next[i], deep + 1);
		}
	}
	return ;
}
void SetTrieValue(const char* Key, const char* Value, LPTRIETWIGS lpRoot) {
	int i;
	LPTRIETWIGS lpTrieThis = lpRoot;
	for(i = 0; i < strlen(Key); i += 1) {
		if(lpTrieThis -> Next[Key[i]] == NULL) lpTrieThis -> Next[Key[i]] = (LPTRIETWIGS)calloc(1, sizeof(TRIETWIGS));
		lpTrieThis = lpTrieThis -> Next[Key[i]];
	}
	if(lpTrieThis -> Value == NULL) {
		lpTrieThis -> Value = (char*)calloc(strlen(Value) + 5, sizeof(char));
		lpTrieThis -> size = strlen(Value);
	}
	if(lpTrieThis -> size < strlen(Value)) {
		free(lpTrieThis -> Value);
		lpTrieThis -> Value = (char*)calloc(strlen(Value) + 5, sizeof(char));
		lpTrieThis -> size = strlen(Value);
	}
	strcpy(lpTrieThis -> Value, Value);
	return ;
	/*if(Key[0] == '\0') {
		if(lpRoot -> Value == NULL) {
			lpRoot -> Value = (char*)calloc(strlen(Value) + 5, sizeof(char));
			lpRoot -> size = strlen(Value) + 5;
		}
		else if(strlen(Value) > lpRoot -> size) {
			free(lpRoot -> Value);
			lpRoot -> Value = (char*)calloc(strlen(Value) + 5, sizeof(char));
			lpRoot -> size = strlen(Value) + 5;
		}
		strcpy(lpRoot -> Value, Value);
		return ;
	}
	if(lpRoot -> Next[Key[0]] == NULL) lpRoot -> Next[Key[0]] = (LPTRIETWIGS)calloc(1, sizeof(TRIETWIGS));
	SetTrieValue(Key + 1, Value, lpRoot -> Next[Key[0]]);*/
	return ;
}
unsigned short ListenPort = 7900, BlackListCount, WhiteListCount, RealBLC, RealWLC, SilencerListCount, RealSLC;
char LogBuf[1145], LogBuf2[1145141910], InvitationCode[256], *BlackList[65546], *WhiteList[65546], *SilencerList[65546], EnableBlackList, EnableWhiteList, RoomName[512];
const char *VersionData = "\x1\x3\x0";
struct ULIST{
	char UserName[512];
	SOCKET UserBindClient;
	MSGLISTENERFUNC VirtualUserMsgHandler;
	struct ULIST *Next, *Last;
}*UL_Head, *UL_Last;
unsigned int UsersCount = 0, UserLimit;
unsigned long long MessagesCount = 0;
SOCKET ListenSocket;
pthread_mutex_t thread_lock, thread_lock_Sync;
#ifdef _WIN32
HMODULE hPlugins[32767];
#else
void* hPlugins[32767];
#endif
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
    strftime(LogBuf, sizeof(LogBuf), "%Y-%m-%d %H:%M:%S", localTime);
    vsprintf(LogBuf2, Format, v);
    printf("\033[34m[\033[33m%s\033[34m]\033[0m \033[34m[\033[0m", LogBuf);
    int i;
    for(i = 0; i < strlen(Poster); i += 1) {
    	if(Poster[i] == '/') {
    		if(strcmp(Poster + i + 1, "INFO") == 0) {
    			printf("/\033[32mINFO\033[0m");
			}
    		else if(strcmp(Poster + i + 1, "WARN") == 0) {
    			printf("/\033[33mWARN\033[0m");
			}
    		else if(strcmp(Poster + i + 1, "ERROR") == 0) {
    			printf("/\033[31mERROR\033[0m");
			}
			else {
				printf("%s", Poster + i);
			}
			break;
		}
		printf("%c", Poster[i]);
	}
	printf("\033[34m]\033[0m: %s", LogBuf2);
    if(NoNewLine == 0) printf("\n");
	va_end(v);
	return ;
}
void vLogOut(const char* Poster, int NoNewLine, const char* Format, va_list v) {
	time_t CurrentTime;
	time(&CurrentTime);
	struct tm* localTime;
    localTime = localtime(&CurrentTime);
    strftime(LogBuf, sizeof(LogBuf), "%Y-%m-%d %H:%M:%S", localTime);
    vsprintf(LogBuf2, Format, v);
    printf("\033[34m[\033[33m%s\033[34m]\033[0m \033[34m[\033[0m", LogBuf);
    int i;
    for(i = 0; i < strlen(Poster); i += 1) {
    	if(Poster[i] == '/') {
    		if(strcmp(Poster + i + 1, "INFO") == 0) {
    			printf("/\033[32mINFO\033[0m");
			}
    		else if(strcmp(Poster + i + 1, "WARN") == 0) {
    			printf("/\033[33mWARN\033[0m");
			}
    		else if(strcmp(Poster + i + 1, "ERROR") == 0) {
    			printf("/\033[31mERROR\033[0m");
			}
			else {
				printf("%s", Poster + i);
			}
			break;
		}
		printf("%c", Poster[i]);
	}
	printf("\033[34m]\033[0m: %s", LogBuf2);
    if(NoNewLine == 0) printf("\n");
	return ;
}
void PluginLogOut_API(const char* ModuleName, const char* Type, int NoNewLine, const char* Format, ...) {
	char Poster[32767];
	va_list v;
	va_start(v, Format);
	sprintf(Poster, "%s/%s", ModuleName, Type);
	vLogOut(Poster, NoNewLine, Format, v);
	va_end(v);
	return ;
}
int PluginGetUsersList_API(char **Users, int iBufferMax) {
	if(Users != NULL) {
		int i = 0;
		struct ULIST *This = UL_Head -> Next;
		while(This != NULL) {
			if(i >= iBufferMax) break;
			strcpy(Users[i], This -> UserName);
			This = This -> Next;
			i += 1;
		}
	}
	return UsersCount;
}
HUSER PluginCreateUser_API(const char* ModuleName, const char* UserName, MSGLISTENERFUNC MsgListener) {
	if(UsersCount >= UserLimit) return NULL;
	struct ULIST *This = UL_Head -> Next;
	int HasSame = 0;
	while(This != NULL) {
		if(strcmp(This -> UserName, UserName) == 0) {
			HasSame = 1;
			break;
		}
		This = This -> Next;
	}
	if(HasSame == 1) return NULL;
	struct ULIST *NewUser = (struct ULIST*)calloc(1, sizeof(struct ULIST));
	strcpy(NewUser -> UserName, UserName);
	NewUser -> VirtualUserMsgHandler = MsgListener;
	time_t Time = time(NULL);
	unsigned char length = strlen(UserName);
	This = UL_Head -> Next;
	while(This != NULL) {
		if(This -> UserBindClient != 0) {
			send(This -> UserBindClient, "\xA", 1, 0);
			send(This -> UserBindClient, (const char*)&Time, sizeof(Time), 0);
			send(This -> UserBindClient, (const char*)&length, 1, 0);
			send(This -> UserBindClient, UserName, length, 0);
		}
		else {
			This -> VirtualUserMsgHandler(This -> UserName, UserName, 1, NULL, Time);
		}
		This = This -> Next;
	}
	NewUser -> Last = UL_Last;
	UL_Last -> Next = NewUser;
	UL_Last = NewUser;
	UsersCount += 1;
	pthread_mutex_lock(&thread_lock_Sync);
#ifdef _WIN32
	FILE *lpFile = fopen("configs\\chathistory.nchatserver", "ab");
#else
	FILE *lpFile = fopen("configs/chathistory.nchatserver", "ab");
#endif
	if(lpFile != NULL) {
		fwrite("\xA", sizeof(char), 1, lpFile);
		fwrite(&Time, sizeof(Time), 1, lpFile);
		fwrite(&length, sizeof(char), 1, lpFile);
		fwrite(UserName, sizeof(char), length, lpFile);
		fclose(lpFile);
	}
	pthread_mutex_unlock(&thread_lock_Sync);
	LogOut("Plugin API/INFO", 0, "%s(%s) joined the chat room", UserName, ModuleName);
	return NewUser;
}
void PluginCloseUser_API(const char* ModuleName, HUSER hUser) {
	if(hUser == NULL || ModuleName == NULL) return;
	struct ULIST *This = (struct ULIST*)hUser, *lpEnum;
	This -> Last -> Next = This -> Next;
	if(This -> Next != NULL) This -> Next -> Last = This -> Last;
	else UL_Last = This -> Last;
	char SendMsg[4];
	SendMsg[0] = '\xB';
	SendMsg[1] = strlen(This -> UserName);
	time_t Time;
	Time = time(NULL);
	lpEnum = UL_Head -> Next;
	while(lpEnum != NULL) {
		if(lpEnum -> UserBindClient != 0) {
			send(lpEnum -> UserBindClient, SendMsg, 1, 0);
			send(lpEnum -> UserBindClient, (const char*)&Time, sizeof(Time), 0);
			send(lpEnum -> UserBindClient, SendMsg + 1, 1, 0);
			send(lpEnum -> UserBindClient, This -> UserName, SendMsg[1], 0);
		}
		else lpEnum -> VirtualUserMsgHandler(lpEnum -> UserName, This -> UserName, 2, NULL, Time);
		lpEnum = lpEnum -> Next;
	}
	LogOut("Plugin API/INFO", 0, "%s left the chat room(%s)", This -> UserName, ModuleName);
	pthread_mutex_lock(&thread_lock_Sync);
#ifdef _WIN32
	FILE *lpFile = fopen("configs\\chathistory.nchatserver", "ab");
#else
	FILE *lpFile = fopen("configs/chathistory.nchatserver", "ab");
#endif
	if(lpFile != NULL) {
		fwrite(SendMsg, sizeof(char), 1, lpFile);
		fwrite(&Time, sizeof(Time), 1, lpFile);
		fwrite(SendMsg + 1, sizeof(char), 1, lpFile);
		fwrite(This -> UserName, sizeof(char), SendMsg[1], lpFile);
		fclose(lpFile);
	}
	pthread_mutex_unlock(&thread_lock_Sync);
	free(This);
	UsersCount -= 1;
	return ;
}
void PluginFakeUserSpeak_API(HUSER hUser, int NeedCallBack, const char *Message) {
	if(hUser == NULL || Message == NULL || (Message != NULL && strcmp(Message, "") == 0)) return;
	struct ULIST *This = UL_Head -> Next, *uUser = (struct ULIST*)hUser;
	time_t Time;
	Time = time(NULL);
	unsigned char Len = strlen(uUser -> UserName);
	int iLength = strlen(Message);
	while(This != NULL) {
		if(NeedCallBack == 0 && strcmp(This -> UserName, uUser -> UserName) == 0) {
			This = This -> Next;
			continue;
		}
		if(This -> UserBindClient != 0) {
			send(This -> UserBindClient, "\xF", 1, 0); 
			send(This -> UserBindClient, (const char*)&Time, sizeof(Time), 0);
			send(This -> UserBindClient, (const char*)&Len, 1, 0);
			send(This -> UserBindClient, uUser -> UserName, Len, 0);
			send(This -> UserBindClient, (const char*)&iLength, sizeof(iLength), 0);
			send(This -> UserBindClient, Message, iLength, 0);
		}
		else This -> VirtualUserMsgHandler(This -> UserName, uUser -> UserName, 0, Message, Time);
		This = This -> Next;
	}
	pthread_mutex_lock(&thread_lock_Sync);
#ifdef _WIN32
	FILE *lpFile = fopen("configs\\chathistory.nchatserver", "ab");
#else
	FILE *lpFile = fopen("configs/chathistory.nchatserver", "ab");
#endif
	if(lpFile != NULL) {
		fwrite("\xF", sizeof(char), 1, lpFile);
		fwrite(&Time, sizeof(Time), 1, lpFile);
		fwrite(&Len, sizeof(char), 1, lpFile);
		fwrite(uUser -> UserName, sizeof(char), Len, lpFile);
		fwrite(&iLength, sizeof(iLength), 1, lpFile);
		fwrite(Message, sizeof(char), iLength, lpFile);
		fclose(lpFile);
	}
	pthread_mutex_unlock(&thread_lock_Sync);
	char FileName[32767];
#ifdef _WIN32
	sprintf(FileName, "ChatMessages\\Message-%lld.nmsg", MessagesCount);
#else
	sprintf(FileName, "ChatMessages/Message-%lld.nmsg", MessagesCount);
#endif
	lpFile = fopen(FileName, "w");
	if(lpFile == NULL) LogOut("Server Thread/ERROR", 0, "Saved Message Failed: Cannot create file '%s'!", FileName);
	else {
		fprintf(lpFile, "<%s> %s", uUser -> UserName, Message);
		fclose(lpFile);
	}
	LogOut("Server Thread/INFO", 0, "[Message-%lld] <%s> %s", MessagesCount, uUser -> UserName, Message);
	MessagesCount += 1;
	return ;
}
int PluginIsMentionedUser_API(const char* Message, const char* UserName, const char** FirstMentionedPos) {
	int len = strlen(Message), i;
	for(i = 0; i < len; i += 1) {
		if(Message[i] == '@') {
			char* Position = strchr(Message + 1 + i, ' ');
			if(Position > Message + i && strncmp(Message + i + 1, UserName, Position - Message - i - 1) == 0) {
				if(FirstMentionedPos != NULL) *FirstMentionedPos = Message + i;
				return 1;
			}
		}
	}
	return 0;
}
int PluginIsUserOnline_API(const char* UserName) {
	struct ULIST *This = UL_Head -> Next;
	while(This != NULL) {
		if(strcmp(This -> UserName, UserName) == 0) return 1;
		This = This -> Next;
	}
	return 0;
}
int PluginMakeUserSilence_API(const char* ModuleName, const char* UserName) {
	struct ULIST *This = UL_Head -> Next;
	while(This != NULL) {
		if(strcmp(This -> UserName, UserName) == 0) {
			int i, FirstFreePlace = -1;
			for(i = 1; i <= SilencerListCount; i += 1) {
				if(SilencerList[i] == NULL) FirstFreePlace = i;
				if(SilencerList[i] != NULL && strcmp(UserName, SilencerList[i]) == 0) {
					return -2;
				}
			}
			if(FirstFreePlace == -1) {
				FirstFreePlace = SilencerListCount + 1;
				SilencerListCount += 1;
			}
			SilencerList[FirstFreePlace] = (char*)calloc(32767, sizeof(char));
			strcpy(SilencerList[FirstFreePlace], UserName);
			char SendMsg[4];
			SendMsg[0] = '\x10';
			SendMsg[1] = strlen(SilencerList[FirstFreePlace]);
			time_t Time;
			time(&Time);
#ifdef _WIN32
			FILE *lpFile = fopen("configs\\chathistory.nchatserver", "ab");
#else
			FILE *lpFile = fopen("configs/chathistory.nchatserver", "ab");
#endif
			fwrite(SendMsg, sizeof(char), 1, lpFile);
			fwrite(&Time, sizeof(Time), 1, lpFile);
			fwrite(SendMsg + 1, sizeof(char), 1, lpFile);
			fwrite(SilencerList[FirstFreePlace], sizeof(char), SendMsg[1], lpFile);
			fclose(lpFile);
			This = UL_Head -> Next;
			while(This != NULL) {
				if(This -> UserBindClient != 0) {
					send(This -> UserBindClient, SendMsg, 1, 0);
					send(This -> UserBindClient, (const char*)&Time, sizeof(Time), 0);
					send(This -> UserBindClient, SendMsg + 1, 1, 0);
					send(This -> UserBindClient, SilencerList[FirstFreePlace], SendMsg[1], 0);
				}
				else This -> VirtualUserMsgHandler(This -> UserName, SilencerList[FirstFreePlace], 4, NULL, Time);
				This = This -> Next;
			}
			LogOut("Server Thread/INFO", 0, "User %s is a silencer now by %s.", SilencerList[FirstFreePlace], ModuleName);
			return 0;
		}
		This = This -> Next;
	}
	return -1;
}
int PluginMakeUserNotSilence_API(const char* ModuleName, const char* UserName) {
	struct ULIST *This = UL_Head -> Next;
	while(This != NULL) {
		if(strcmp(This -> UserName, UserName) == 0) {
			int i;
			for(i = 1; i <= SilencerListCount; i += 1) {
				if(SilencerList[i] != NULL && strcmp(UserName, SilencerList[i]) == 0) {
					char SendMsg[4];
					SendMsg[0] = '\x11';
					SendMsg[1] = strlen(SilencerList[i]);
					time_t Time;
					time(&Time);
#ifdef _WIN32
					FILE *lpFile = fopen("configs\\chathistory.nchatserver", "ab");
#else
					FILE *lpFile = fopen("configs/chathistory.nchatserver", "ab");
#endif
					fwrite(SendMsg, sizeof(char), 1, lpFile);
					fwrite(&Time, sizeof(Time), 1, lpFile);
					fwrite(SendMsg + 1, sizeof(char), 1, lpFile);
					fwrite(SilencerList[i], sizeof(char), SendMsg[1], lpFile);
					fclose(lpFile);
					This = UL_Head -> Next;
					while(This != NULL) {
						if(This -> UserBindClient != 0) {
							send(This -> UserBindClient, SendMsg, 1, 0);
							send(This -> UserBindClient, (const char*)&Time, sizeof(Time), 0);
							send(This -> UserBindClient, SendMsg + 1, 1, 0);
							send(This -> UserBindClient, SilencerList[i], SendMsg[1], 0);
						}
						else This -> VirtualUserMsgHandler(This -> UserName, SilencerList[i], 5, NULL, Time);
						This = This -> Next;
					}
					LogOut("Server Thread/INFO", 0, "User %s has already unbanned from speaking by %s.", SilencerList[i], ModuleName);
					free(SilencerList[i]);
					SilencerList[i] = NULL;
					return 0;
				}
			}
			return -2;
		}
		This = This -> Next;
	}
	return -1;
}
int PluginGetConfigValue_API(const char* NameSpace, const char* Key, char* Value) {
	if(NameSpace == NULL || Key == NULL || Value == NULL || strlen(NameSpace) == 0 || strlen(Key) == 0 || strlen(NameSpace) + strlen(Key) + 1 >= 256) return -1;
	char* RealKey = (char*)calloc(256 + 32, sizeof(char));
	sprintf(RealKey, "%s:%s", NameSpace, Key);
	int v = GetTrieValue(RealKey, Value, lpTrieRoot);
	free(RealKey);
	return v;
}
int PluginSetConfigValue_API(const char* NameSpace, const char* Key, const char* Value) {
	if(NameSpace == NULL || Key == NULL || Value == NULL || strlen(NameSpace) == 0 || strlen(Key) == 0 || strlen(NameSpace) + strlen(Key) + 1 >= 256) return -1;
	char* RealKey = (char*)calloc(256 + 32, sizeof(char));
	sprintf(RealKey, "%s:%s", NameSpace, Key);
	SetTrieValue(RealKey, Value, lpTrieRoot);
	free(RealKey);
	return 0;
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
			closesocket(ListenSocket);
#ifdef _WIN32
			WSACleanup();
#endif
			return NULL;
		}
		else if(strcmp(lpstrCommand, "list") == 0) {
			LogOut("Server Thread/INFO", 1, "There are %d of a max of %d users online: ", UsersCount, UserLimit);
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
				//free(This);
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
					if(BlackList[i] != NULL && strcmp(BlackList[i], lpstrInput + 14) == 0) {
						j = 1;
						break;
					}
				}
				if(j == 1) {
					LogOut("Server Thread/ERROR", 0, "There was already a user named '%s' in the blacklist.", lpstrInput + 14);
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
					free(BlackList[i]);
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
					if(WhiteList[i] != NULL && strcmp(WhiteList[i], lpstrInput + 14) == 0) {
						j = 1;
						break;
					}
				}
				if(j == 1) {
					LogOut("Server Thread/ERROR", 0, "There was already a user named '%s' in the whitelist.", lpstrInput + 14);
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
					free(WhiteList[i]);
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
				//free(This);
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
			char FileName[32767];
			time_t Time = time(NULL);
			int iLength = strlen(lpstrInput + 4);
#ifdef _WIN32
			sprintf(FileName, "ChatMessages\\Message-%lld.nmsg", MessagesCount);
#else
			sprintf(FileName, "ChatMessages/Message-%lld.nmsg", MessagesCount);
#endif
			FILE *lpFile = fopen(FileName, "w");
			if(lpFile == NULL) LogOut("Server Thread/ERROR", 0, "Saved Message Failed: Cannot create file '%s'!", FileName);
			else {
				fprintf(lpFile, "<Server> %s", lpstrInput + 4);
				fclose(lpFile);
			}
#ifdef _WIN32
			lpFile = fopen("configs\\chathistory.nchatserver", "ab");
#else
			lpFile = fopen("configs/chathistory.nchatserver", "ab");
#endif
			if(lpFile != NULL) {
				fwrite("\xF", 1, 1, lpFile);
				fwrite(&Time, sizeof(Time), 1, lpFile);
				fwrite("\x6Server", sizeof(char), 7, lpFile);
				fwrite((const char*)&iLength, sizeof(iLength), 1, lpFile);
				fwrite(lpstrInput + 4, sizeof(char), iLength, lpFile);
				fclose(lpFile);
			}
			LogOut("Server Thread/INFO", 0, "[Message-%lld] <Server> %s", MessagesCount, lpstrInput + 4);
			MessagesCount += 1;
			struct ULIST *This = UL_Head -> Next;
			while(This != NULL) {
				if(This -> UserBindClient != 0) {
					send(This -> UserBindClient, "\xF", 1, 0); 
					send(This -> UserBindClient, (const char*)&Time, sizeof(Time), 0);
					send(This -> UserBindClient, "\x6Server", 7, 0);
					send(This -> UserBindClient, (const char*)&iLength, sizeof(iLength), 0);
					send(This -> UserBindClient, lpstrInput + 4, iLength, 0);
				}
				else This -> VirtualUserMsgHandler(This -> UserName, "Server", 0, lpstrInput + 4, Time);
				This = This -> Next;
			}
		}
		else if(strcmp(lpstrCommand, "viewmsg") == 0) {
			if(lpstrInput[7] != ' ') {
				LogOut("Server Thread/ERROR", 0, "Incomplete arguments for command viewmsg!");
				continue;
			}
			char FileName[32767];
#ifdef _WIN32
			sprintf(FileName, "ChatMessages\\Message-%s.nmsg", lpstrInput + 8);
#else
			sprintf(FileName, "ChatMessages/Message-%s.nmsg", lpstrInput + 8);
#endif
			FILE *lpFile = fopen(FileName, "r");
			if(lpFile == NULL) LogOut("Server Thread/ERROR", 0, "The message record file has been deleted!");
			else {
				char *Data = (char*)calloc(114514, sizeof(char));
				fread(Data, sizeof(char), 114514, lpFile);
				LogOut("Server Thread/INFO", 0, "[Message Recorder] %s", Data);
				fclose(lpFile);
				free(Data);
			}
		}
		else if(strcmp(lpstrCommand, "silencer") == 0) {
			if(lpstrInput[8] != ' ') {
				LogOut("Server Thread/ERROR", 0, "Incomplete arguments for command silencer!");
				continue;
			}
			sscanf(lpstrInput + 9, "%s", lpstrCommand);
			if(strcmp(lpstrCommand, "help") == 0) {
				LogOut("Server Thread/INFO", 0, "silencer Options: add <UserName>, list, remove <UserName>, help");
			}
			else if(strcmp(lpstrCommand, "add") == 0) {
				if(lpstrInput[12] != ' ') {
					LogOut("Server Thread/ERROR", 0, "Incomplete arguments for command silencer's option add!");
					continue;
				}
				j = 0;
				for(i = 1; i <= SilencerListCount; i += 1) {
					if(SilencerList[i] != NULL && strcmp(SilencerList[i], lpstrInput + 13) == 0) {
						j = 1;
						break;
					}
				}
				if(j == 1) {
					LogOut("Server Thread/ERROR", 0, "%s is already a silencer.", lpstrInput + 13);
					continue;
				}
				j = 0;
				for(i = 1; i <= SilencerListCount; i += 1) {
					if(SilencerList[i] == NULL) {
						j = 1;
						break;
					}
				}
				RealSLC += 1;
				SilencerList[i] = (char*)calloc(strlen(lpstrInput + 13) + 10, sizeof(char));
				strcpy(SilencerList[i], lpstrInput + 13);
				if(j == 0) SilencerListCount += 1;
				struct ULIST *This = UL_Head -> Next;
				char SendMsg[4];
				SendMsg[0] = '\x10';
				SendMsg[1] = strlen(SilencerList[i]);
				time_t Time;
				time(&Time);
#ifdef _WIN32
				FILE *lpFile = fopen("configs\\chathistory.nchatserver", "ab");
#else
				FILE *lpFile = fopen("configs/chathistory.nchatserver", "ab");
#endif
				fwrite(SendMsg, sizeof(char), 1, lpFile);
				fwrite(&Time, sizeof(Time), 1, lpFile);
				fwrite(SendMsg + 1, sizeof(char), 1, lpFile);
				fwrite(SilencerList[i], sizeof(char), SendMsg[1], lpFile);
				fclose(lpFile);
				while(This != NULL) {
					if(This -> UserBindClient != 0) {
						send(This -> UserBindClient, SendMsg, 1, 0);
						send(This -> UserBindClient, (const char*)&Time, sizeof(Time), 0);
						send(This -> UserBindClient, SendMsg + 1, 1, 0);
						send(This -> UserBindClient, SilencerList[i], SendMsg[1], 0);
					}
					else This -> VirtualUserMsgHandler(This -> UserName, SilencerList[i], 4, NULL, Time);
					This = This -> Next;
				}
				LogOut("Server Thread/INFO", 0, "User %s is a silencer.", SilencerList[i]);
			}
			else if(strcmp(lpstrCommand, "remove") == 0) {
				if(lpstrInput[15] != ' ') {
					LogOut("Server Thread/ERROR", 0, "Incomplete arguments for command silencer's option remove!");
					continue;
				}
				j = 0;
				for(i = 1; i <= SilencerListCount; i += 1) {
					if(WhiteList[i] != NULL && strcmp(SilencerList[i], lpstrInput + 16) == 0) {
						j = 1;
						break;
					}
				}
				if(j == 1) {
					struct ULIST *This = UL_Head -> Next;
					char SendMsg[4];
					SendMsg[0] = '\x11';
					SendMsg[1] = strlen(SilencerList[i]);
					time_t Time;
					time(&Time);
#ifdef _WIN32
					FILE *lpFile = fopen("configs\\chathistory.nchatserver", "ab");
#else
					FILE *lpFile = fopen("configs/chathistory.nchatserver", "ab");
#endif
					fwrite(SendMsg, sizeof(char), 1, lpFile);
					fwrite(&Time, sizeof(Time), 1, lpFile);
					fwrite(SendMsg + 1, sizeof(char), 1, lpFile);
					fwrite(SilencerList[i], sizeof(char), SendMsg[1], lpFile);
					fclose(lpFile);
					while(This != NULL) {
						if(This -> UserBindClient != 0) {
							send(This -> UserBindClient, SendMsg, 1, 0);
							send(This -> UserBindClient, (const char*)&Time, sizeof(Time), 0);
							send(This -> UserBindClient, SendMsg + 1, 1, 0);
							send(This -> UserBindClient, SilencerList[i], SendMsg[1], 0);
						}
						else This -> VirtualUserMsgHandler(This -> UserName, SilencerList[i], 5, NULL, Time);
						This = This -> Next;
					}
					RealSLC -= 1;
					LogOut("Server Thread/INFO", 0, "User %s is already not a silencer now.", SilencerList[i]);
					free(SilencerList[i]);
					SilencerList[i] = NULL;
				}
				else LogOut("Server Thread/ERROR", 0, "User %s was not a silencer.", lpstrInput + 16);
			}
			else if(strcmp(lpstrCommand, "list") == 0) {
				LogOut("Server Thread/INFO", 1, "There are %d silencers: ", RealSLC);
				j = 0;
				for(i = 1; i <= SilencerListCount; i += 1) {
					if(SilencerList[i] != NULL) {
						if(j == 1) printf(", ");
						printf("%s", SilencerList[i]);
						j = 1;
					}
				}
				printf("\n");
			}
			else LogOut("Server Thread/ERROR", 0, "silencer %s<-- Unknown Option", lpstrCommand);
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
  say -> Say something..., Method: say <Something you want to say: String>\n\
  silencer -> Manage the Silencer List, Method: silencer <add, help, list, remove> [User Name(Only add or remove option): String]\n\
  viewmsg -> View the message details in the past, Method: viewmsg <Message Id: Int>\n\
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
			if(ReceiveData[0] != '\xA' && ReceiveData[0] != 'G' && ReceiveData[0] != '\xE' && ReceiveData[0] != '\x9' && ReceiveData[0] != '\xC' && ReceiveData[0] != '\x10' && beLogined == 0) {
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
							State = 0x0;
							struct ULIST *This_Check = UL_Head -> Next;
							while(This_Check != NULL) {
								if(strcmp(UL_New -> UserName, This_Check -> UserName) == 0) {
									State = 0x1;
									break;
								}
								This_Check = This_Check -> Next;
							}
							if(State == 0x1) {
								send(ClientSocket, "\xB|ERR_SAME_NAME", 15, 0);
								closesocket(ClientSocket);
								UL_New -> Last -> Next = UL_New -> Next;
								if(UL_New -> Next != NULL) UL_New -> Next -> Last = UL_New -> Last;
								free(UL_New);
								return NULL;
							}
							UL_Last -> Next = UL_New;
							UL_New -> UserBindClient = ClientSocket;
							UL_Last = UL_New;
							send(ClientSocket, "\x1", 1, 0);
							pthread_mutex_lock(&thread_lock_Sync);
#ifdef _WIN32
							FILE *lpFile = fopen("configs\\chathistory.nchatserver", "rb");
#else
							FILE *lpFile = fopen("configs/chathistory.nchatserver", "rb");
#endif
							if(lpFile != NULL) {
								char *Data = (char*)calloc(32767, sizeof(char));
								int iRead;
								while((iRead = fread(Data, sizeof(char), 32767, lpFile)) > 0) {
									send(ClientSocket, Data, iRead, 0);
								}
								free(Data); 
								fclose(lpFile);
							}
							send(ClientSocket, "\xFF", 1, 0);
							pthread_mutex_unlock(&thread_lock_Sync);
							time_t Time;
							time(&Time);
							LogOut("Server Thread/INFO", 0, "%s joined the chat room", UL_New -> UserName);
							struct ULIST *This = UL_Head -> Next;
							char SendMsg[4];
							SendMsg[0] = '\xA';
							SendMsg[1] = strlen(UL_New -> UserName);
							while(This != NULL) {
								if(This -> UserBindClient != 0) {
									send(This -> UserBindClient, SendMsg, 1, 0);
									send(This -> UserBindClient, (const char*)&Time, sizeof(Time), 0);
									send(This -> UserBindClient, SendMsg + 1, 1, 0);
									send(This -> UserBindClient, UL_New -> UserName, SendMsg[1], 0);
								}
								else This -> VirtualUserMsgHandler(This -> UserName, UL_New -> UserName, 1, NULL, Time);
								This = This -> Next;
							}
							pthread_mutex_lock(&thread_lock_Sync);
#ifdef _WIN32
							lpFile = fopen("configs\\chathistory.nchatserver", "ab");
#else
							lpFile = fopen("configs/chathistory.nchatserver", "ab");
#endif
							if(lpFile != NULL) {
								fwrite(SendMsg, sizeof(char), 1, lpFile);
								fwrite(&Time, sizeof(Time), 1, lpFile);
								fwrite(SendMsg + 1, sizeof(char), 1, lpFile);
								fwrite(UL_New -> UserName, sizeof(char), SendMsg[1], lpFile);
								fclose(lpFile);
							}
							pthread_mutex_unlock(&thread_lock_Sync);
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
/*#ifdef _WIN32
					__int64 iSize;
#else
					__int64_t iSize;//Linux just use __int64_t
#endif*/
					int i, isOk = 0;
					for(i = 0; i <= SilencerListCount; i += 1) {
						if(SilencerList[i] != NULL && strcmp(UL_New -> UserName, SilencerList[i]) == 0) {
							//send(ClientSocket, "\x9", 1, 0);
							isOk = 1;
							break;
						}
					}
					//if(isOk == 1) break;
					//send(ClientSocket, "\x0", 1, 0);
					unsigned int iLen, iCount = 0;
					struct ULIST *This = UL_Head -> Next;
					if(isOk == 1) This = NULL;
					char SendMsg[4];
					SendMsg[0] = '\xF';
					SendMsg[1] = strlen(UL_New -> UserName);
					recv(ClientSocket, (char*)&iLen, sizeof(iLen), 0);
					time_t Time;
					time(&Time);
					while(This != NULL) {
						if(This -> UserBindClient != 0) {
							send(This -> UserBindClient, SendMsg, 1, 0);
							send(This -> UserBindClient, (const char*)&Time, sizeof(Time), 0);
							send(This -> UserBindClient, SendMsg + 1, 1, 0);
							send(This -> UserBindClient, UL_New -> UserName, SendMsg[1], 0);
							send(This -> UserBindClient, (const char*)&iLen, sizeof(iLen), 0);
						}
						This = This -> Next;
					}
					if(isOk == 0) {
						char FileName[32767], *Message = (char*)calloc(262144, sizeof(char));
#ifdef _WIN32
						sprintf(FileName, "ChatMessages\\Message-%lld.nmsg", MessagesCount);
#else
						sprintf(FileName, "ChatMessages/Message-%lld.nmsg", MessagesCount);
#endif
						FILE *lpFile = fopen(FileName, "w");
						if(lpFile == NULL) LogOut("Server Thread/ERROR", 0, "Saved Message Failed: Cannot create file '%s'!", FileName);
						else fprintf(lpFile, "<%s> ", UL_New -> UserName);
						LogOut("Server Thread/INFO", 1, "[Message-%lld] <%s> ", MessagesCount, UL_New -> UserName);//Who sent the message
						//pthread_mutex_lock(&thread_lock_Sync);
#ifdef _WIN32
						FILE *lpFileHistory = fopen("configs\\chathistory.nchatserver", "ab");
#else
						FILE *lpFileHistory = fopen("configs/chathistory.nchatserver", "ab");
#endif
						if(lpFileHistory != NULL) {
							fwrite(SendMsg, sizeof(char), 1, lpFileHistory);
							fwrite(&Time, sizeof(Time), 1, lpFileHistory);
							fwrite(SendMsg + 1, sizeof(char), 1, lpFileHistory);
							fwrite(UL_New -> UserName, sizeof(char), SendMsg[1], lpFileHistory);
							fwrite((const char*)&iLen, sizeof(iLen), 1, lpFileHistory);
						}
						while(iCount < iLen) {
							memset(ReceiveData, 0, sizeof(ReceiveData));
							iResult = recv(ClientSocket, ReceiveData, sizeof(ReceiveData) - 1, 0);//Read Message
							strcat(Message, ReceiveData);
							if(iResult <= 0) break;
							iCount += iResult;
							if(lpFile != NULL) fprintf(lpFile, "%s", ReceiveData);
							This = UL_Head -> Next;
							while(This != NULL) {
								if(This -> UserBindClient != 0) send(This -> UserBindClient, ReceiveData, iResult, 0);//Send Message
								This = This -> Next;
							}
							if(lpFileHistory != NULL) fwrite(ReceiveData, sizeof(char), iResult, lpFileHistory);
							if(iCount <= iResult) {
								int i, Disabled = 0;
								for(i = 0; i < 30; i += 1) {
									if(ReceiveData[i] == '\n' || ReceiveData[i] == '\r') {
										strcpy(ReceiveData + i, "...(More)");
										Disabled = 1;
										break;
									}
								}
								if(Disabled == 0) ReceiveData[30] = '\0';
								printf("%s", ReceiveData);//Print Message
							}
							//if(iResult < sizeof(ReceiveData) - 1) break;
						}
						if(lpFile != NULL) fclose(lpFile);
						if(lpFileHistory != NULL) fclose(lpFileHistory);
						//pthread_mutex_unlock(&thread_lock_Sync);
						printf("\n");
						MessagesCount += 1;
						This = UL_Head -> Next;
						while(This != NULL) {
							if(This -> UserBindClient == 0) {
								This -> VirtualUserMsgHandler(This -> UserName, UL_New -> UserName, 0, Message, Time);
							}
							This = This -> Next;
						}
						free(Message);
					}
					else {
						LogOut("Server Thread/WARN", 0, "Silencer '%s' is trying to send message.", UL_New -> UserName);
						char UselessBuffer_For_Stupids[114514];
						while(iCount < iLen) {
							iResult = recv(ClientSocket, UselessBuffer_For_Stupids, sizeof(UselessBuffer_For_Stupids) - 1, 0);//Read Message
							if(iResult <= 0) break;
							iCount += iResult;
						}
					}
					break;
				}
				case 0xC: {//Send Message(File) 
					char UserName[512], FileName[512];
					unsigned char NameLength = 0, FileNameLength = 0;
					memset(UserName, 0, sizeof(UserName));
					memset(FileName, 0, sizeof(FileName));
					recv(ClientSocket, (char*)&NameLength, 1, 0);
					recv(ClientSocket, UserName, NameLength, 0);
					struct ULIST *This = UL_Head -> Next;
					while(This != NULL) {
						if(strcmp(This -> UserName, UserName) == 0) break;
						This = This -> Next;
					}
					if(This == NULL) {
						send(ClientSocket, "\x8|ERR_USER_DIDNOTLOGIN", 22, 0);
					}
					else {
						int i, isOk = 0;
						for(i = 0; i <= SilencerListCount; i += 1) {
							if(SilencerList[i] != NULL && strcmp(UserName, SilencerList[i]) == 0) {
								isOk = 1;
								break;
							}
						}
						if(isOk == 1) {
							send(ClientSocket, "\x9|ERR_BE_SILENT", 15, 0);
							goto ContinueUploadFile;
						}
						send(ClientSocket, "\x0", 1, 0);
#ifdef _WIN32
						strcpy(FileName, "ChatFiles\\");
#else
						strcpy(FileName, "ChatFiles/");
#endif
						recv(ClientSocket, (char*)&FileNameLength, sizeof(FileNameLength), 0);
						recv(ClientSocket, FileName + strlen(FileName), FileNameLength, 0);
						FILE *lpFile = fopen(FileName, "rb");
						if(lpFile != NULL) {
							fclose(lpFile);
							int i = strlen(FileName) - 1;
							while(i >= 0 && FileName[i] != '.') i -= 1;
							if(i == -1) {
								int FileNameLength = strlen(FileName), id = 0;
								while((lpFile = fopen(FileName, "rb")) != NULL) {
									fclose(lpFile);
									id += 1;
									sprintf(FileName + FileNameLength, " (%d)", id);
								}
							}
							else {
								char *FileSuffix = (char*)calloc(256, sizeof(char));
								strcpy(FileSuffix, FileName + i);
								FileName[i] = '\0';
								int FileNameLength = strlen(FileName), id = 1;
								sprintf(FileName + FileNameLength, " (%d)%s", id, FileSuffix);
								while((lpFile = fopen(FileName, "rb")) != NULL) {
									fclose(lpFile);
									id += 1;
									sprintf(FileName + FileNameLength, " (%d)%s", id, FileSuffix);
								}
								free(FileSuffix);
							}
						}
						lpFile = fopen(FileName, "wb");
						if(lpFile == NULL) {
							send(ClientSocket, "\x1", 1, 0);
							closesocket(ClientSocket);
							return NULL;
						}
						else send(ClientSocket, "\x0", 1, 0);
						char *ReceiveData = (char*)calloc(32767, sizeof(char));
						//unsigned int FileSize = 0, BytesReadCount = 0, BytesRead;
#ifdef _WIN32 
						_off64_t FileSize = 0, BytesReadCount = 0, BytesRead;
#else
						__int64_t FileSize = 0, BytesReadCount = 0, BytesRead;
#endif
						recv(ClientSocket, (char*)&FileSize, sizeof(FileSize), 0);
						while(BytesReadCount < FileSize) {
							BytesReadCount += BytesRead = recv(ClientSocket, ReceiveData, 32767, 0);
							fwrite(ReceiveData, sizeof(char), BytesRead, lpFile);
						}
						fclose(lpFile);
						free(ReceiveData);
						This = UL_Head -> Next;
						FileNameLength = strlen(FileName + 10);
						time_t Time;
						time(&Time);
						while(This != NULL) {
							if(This -> UserBindClient != 0) {
								send(This -> UserBindClient, "\xE", 1, 0);
								send(This -> UserBindClient, (const char*)&Time, sizeof(Time), 0);
								send(This -> UserBindClient, (const char*)&NameLength, sizeof(NameLength), 0);
								send(This -> UserBindClient, UserName, NameLength, 0);
								send(This -> UserBindClient, (const char*)&FileNameLength, sizeof(FileNameLength), 0);
								send(This -> UserBindClient, FileName + 10, FileNameLength, 0);
								send(This -> UserBindClient, (const char*)&FileSize, sizeof(FileSize), 0);
							}
							else This -> VirtualUserMsgHandler(This -> UserName, UserName, 3, FileName + 10, Time);
							This = This -> Next;
						}
						pthread_mutex_lock(&thread_lock_Sync);
#ifdef _WIN32 
						FILE* lpFileHistory = fopen("configs\\chathistory.nchatserver", "ab");
#else
						FILE* lpFileHistory = fopen("configs/chathistory.nchatserver", "ab");
#endif
						if(lpFileHistory != NULL) {
							fwrite("\xE", 1, 1, lpFileHistory);
							fwrite(&Time, sizeof(Time), 1, lpFileHistory);
							fwrite((const char*)&NameLength, sizeof(NameLength), 1, lpFileHistory);
							fwrite(UserName, sizeof(char), NameLength, lpFileHistory);
							fwrite((const char*)&FileNameLength, sizeof(FileNameLength), 1, lpFileHistory);
							fwrite(FileName + 10, sizeof(char), FileNameLength, lpFileHistory);
							fwrite((const char*)&FileSize, sizeof(FileSize), 1, lpFileHistory);
							fclose(lpFileHistory);
						}
						pthread_mutex_unlock(&thread_lock_Sync);
						LogOut("Server Thread/INFO", 0, "User %s sent a file named '%s'(Size: %.2lf Kib).", UserName, FileName + 10, FileSize * 1.0 / 1024.0);
					}
					ContinueUploadFile:
					closesocket(ClientSocket);
					return NULL;
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
				case 0x10: {//Download File
					char UserName[512], FileName[512];
					unsigned char NameLength = 0, FileNameLength = 0;
					memset(UserName, 0, sizeof(UserName));
					memset(FileName, 0, sizeof(FileName));
					recv(ClientSocket, (char*)&NameLength, sizeof(NameLength), 0);
					recv(ClientSocket, UserName, NameLength, 0);
					struct ULIST *This = UL_Head -> Next;
					while(This != NULL) {
						if(strcmp(This -> UserName, UserName) == 0) break;
						This = This -> Next;
					}
					if(This == NULL) {
						send(ClientSocket, "\x8|ERR_USER_DIDNOTLOGIN", 22, 0);
					}
					else {
						send(ClientSocket, "\x0", 1, 0);
#ifdef _WIN32
						strcpy(FileName, "ChatFiles\\");
#else
						strcpy(FileName, "ChatFiles/");
#endif
						recv(ClientSocket, (char*)&FileNameLength, sizeof(FileNameLength), 0);
						recv(ClientSocket, FileName + strlen(FileName), FileNameLength, 0);
						FILE *lpFile = fopen(FileName, "rb");
						if(lpFile == NULL) {
							send(ClientSocket, "\x9|ERR_FILE_NOTFOUND", 19, 0);
						}
						else {
							send(ClientSocket, "\x0", 1, 0);
							char *ReadData = (char*)calloc(32767, sizeof(char));
#ifdef _WIN32
							fseeko64(lpFile, 0, SEEK_END);
#else
							fseeko(lpFile, 0, SEEK_END);
#endif
							//unsigned int Size = ftell(lpFile), BytesRead = 0;
#ifdef _WIN32
							_off64_t Size = ftello64(lpFile), BytesRead = 0;
#else
							__int64_t Size = ftello(lpFile), BytesRead = 0;
#endif
							//fgetpos(lpFile, &Size);
							send(ClientSocket, (const char*)&Size, sizeof(Size), 0);
#ifdef WIN32
							fseeko64(lpFile, 0, SEEK_SET);
#else
							fseeko(lpFile, 0, SEEK_SET);
#endif
							LogOut("Server Thread/INFO", 0, "User %s is downloading file '%s'(Size: %.2lf Kib).", UserName, FileName + 10, Size * 1.0 / 1024);
							while((BytesRead = fread(ReadData, sizeof(char), 32767, lpFile)) > 0) {
								send(ClientSocket, ReadData, BytesRead, 0);
							}
							fclose(lpFile);
						}
					}
					closesocket(ClientSocket);
					return NULL;
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
		time_t Time;
		time(&Time);
		while(This != NULL) {
			if(This -> UserBindClient != 0) {
				send(This -> UserBindClient, SendMsg, 1, 0);
				send(This -> UserBindClient, (const char*)&Time, sizeof(Time), 0);
				send(This -> UserBindClient, SendMsg + 1, 1, 0);
				send(This -> UserBindClient, UL_New -> UserName, SendMsg[1], 0);
			}
			else This -> VirtualUserMsgHandler(This -> UserName, UL_New -> UserName, 2, NULL, Time);
			This = This -> Next;
		}
		pthread_mutex_lock(&thread_lock_Sync);
#ifdef _WIN32
		FILE* lpFileHistory = fopen("configs\\chathistory.nchatserver", "ab");
#else
		FILE* lpFileHistory = fopen("configs/chathistory.nchatserver", "ab");
#endif
		if(lpFileHistory != NULL) {
			fwrite(SendMsg, sizeof(char), 1, lpFileHistory);
			fwrite(&Time, sizeof(Time), 1, lpFileHistory);
			fwrite(SendMsg + 1, sizeof(char), 1, lpFileHistory);
			fwrite(UL_New -> UserName, sizeof(char), SendMsg[1], lpFileHistory);
			fclose(lpFileHistory);
		}
		pthread_mutex_unlock(&thread_lock_Sync);
		free(UL_New);
		UsersCount -= 1;
	}
	//send(ClientSocket, "\x1|OK_LOGOUT", 11, 0);
	return NULL;
}
int main() {
	LogOut("Server Startup/INFO", 0, "Welcome to NChat - Server(Version %d.%d.%d)", (unsigned char)VersionData[0], (unsigned char)VersionData[1], (unsigned char)VersionData[2]);
	LogOut("Server Startup/INFO", 0, "Loading config files...");
	lpTrieRoot = (LPTRIETWIGS)calloc(1, sizeof(TRIETWIGS));
	FILE *lpConfig = fopen(
#ifdef _WIN32
	"configs\\trie.nchatserver"
#else
	"configs/trie.nchatserver"
#endif
	, "rb");
	if(lpConfig != NULL) {
		char Key[256], *pValue = NULL;
		int iKeyLength = 0, iValueLength = 0;
		while(1) {
			memset(Key, 0, sizeof(Key));
			if(fread(&iKeyLength, sizeof(char), 1, lpConfig) <= 0) break;
			fread(Key, sizeof(char), iKeyLength, lpConfig);
			fread(&iValueLength, sizeof(iValueLength), 1, lpConfig);
			pValue = (char*)calloc(sizeof(char), iValueLength + 5);
			fread(pValue, sizeof(char), iValueLength, lpConfig);
			SetTrieValue(Key, pValue, lpTrieRoot);
			free(pValue);
		}
		LogOut("Config Loader/INFO", 0, "Trie Config File loaded \033[32msuccessfully\033[0m");
	}
	else LogOut("Config Loader/WARN", 0, "Loaded trie config file \033[31mfailed\033[0m! %s", strerror(errno));
	int iResult, iEnum, iTmp;
	unsigned char ICSize, ICSize2;
	lpConfig = fopen(
#ifdef _WIN32
	"configs\\config.nchatserver"
#else
	"configs/config.nchatserver"
#endif
	, "rb");
	srand(time(NULL));
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
			LogOut("Config Loader/INFO", 0, "Server Config File loaded \033[32msuccessfully\033[0m");
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
		lpConfig = fopen(
#ifdef _WIN32
	"configs\\silencerlist.nchatserver"
#else
	"configs/silencerlist.nchatserver"
#endif
	, "rb");
		if(lpConfig != NULL) {
			fread(&SilencerListCount, sizeof(SilencerListCount), 1, lpConfig);
			RealSLC = SilencerListCount;
			for(iEnum = 1; iEnum <= SilencerListCount; iEnum += 1) {
				fread(&ICSize, sizeof(char), 1, lpConfig);
				SilencerList[iEnum] = (char*)calloc(ICSize + 10, sizeof(char));
				fread(SilencerList[iEnum], sizeof(char), ICSize + 1, lpConfig);
				if(SilencerList[iEnum][ICSize] != 0x40) LogOut("Server Thread/WARN", 0, "There is a mistake in silencer list: The %dth silencer list user should have 0x40 as the terminator.", iEnum);
				SilencerList[iEnum][ICSize] = '\0';
			}
			fclose(lpConfig);
		}
	}
	else {
		LogOut("Config Loader/WARN", 0, "Loaded trie config file \033[31mfailed\033[0m! %s", strerror(errno));
		for(iEnum = 0; iEnum < 128; iEnum++) {
			int iRand = rand() % 62;
			if(iRand < 26) InvitationCode[iEnum] = iRand + 'A';
			else if(iRand < 36) InvitationCode[iEnum] = iRand + '0' - 26;
			else InvitationCode[iEnum] = iRand + 'a' - 36;
		}
		UserLimit = 20;
	}
	//strcpy(InvitationCode, "o6kYMTvGkhFhZBg8QxAUeqLuKkw4sSaCOxgJ9urA0Rb3jQ5FTQ5Vr41JyVgm8VkrE3ZARVQaTn0huDTkNunBKP9pbdmBUrry9uxZopG5IkoX1BnHkeQeLqFIDl6x8nqn");
	UL_Head = UL_Last = (struct ULIST*)calloc(1, sizeof(struct ULIST));
	LogOut("Plugin Loader/INFO", 0, "Loading Plugins...");
	PluginInterface.cbSize = sizeof(PluginInterface);
	PluginInterface.PluginLogOut = PluginLogOut_API;
	PluginInterface.PluginGetUsersList = PluginGetUsersList_API;
	PluginInterface.PluginCreateUser = PluginCreateUser_API;
	PluginInterface.PluginCloseUser = PluginCloseUser_API;
	PluginInterface.PluginFakeUserSpeak = PluginFakeUserSpeak_API;
	PluginInterface.PluginIsMentionedUser = PluginIsMentionedUser_API;
	PluginInterface.PluginIsUserOnline = PluginIsUserOnline_API;
	PluginInterface.PluginMakeUserSilence = PluginMakeUserSilence_API;
	PluginInterface.PluginMakeUserNotSilence = PluginMakeUserNotSilence_API;
	PluginInterface.PluginGetConfigValue = PluginGetConfigValue_API;
	PluginInterface.PluginSetConfigValue = PluginSetConfigValue_API;
#ifdef _WIN32
	WIN32_FIND_DATAA wfd;
	HANDLE hFind = FindFirstFileA("Plugins\\*.dll", &wfd);
	if(hFind == NULL || hFind == INVALID_HANDLE_VALUE) {
		LogOut("Plugin Loader/WARN", 0, "Directory 'Plugins' does not exist!");
		LogOut("Plugin Loader/WARN", 0, "Canceled loading Plugins.");
	}
	else {
		char *PluginPath = (char*)calloc(32767, sizeof(char));
		int PluginCount = 0;
		do {
			if((wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
				//int i = strlen(wfd.cFileName) - 1;
				//while(i >= 0 && wfd.cFileName[i] != '.') i -= 1;
				//if(i >= 0 && strcmp(wfd.cFileName + i + 1, "dll"))
				GetCurrentDirectory(32766, PluginPath);
				sprintf(PluginPath + strlen(PluginPath), "\\Plugins\\%s", wfd.cFileName);
				LogOut("Plugin Loader/INFO", 0, "Scanned Plugin File Name: %s", wfd.cFileName);
				hPlugins[++PluginCount] = LoadLibraryA(PluginPath);
				if(hPlugins[PluginCount] != NULL && hPlugins[PluginCount] != INVALID_HANDLE_VALUE) {
					LogOut("Plugin Loader/INFO", 0, "Loading Plugin '%s'...", wfd.cFileName);
					PLUGININITFUNC PluginInit = (PLUGININITFUNC)GetProcAddress(hPlugins[PluginCount], "InitInterfaces");
					if(PluginInit == NULL) {
						LogOut("Plugin Loader/ERROR", 0, "Plugin '%s' is not available!", wfd.cFileName);
						PluginCount -= 1;
					}
					else {
						PluginsName[PluginCount] = (char*)calloc(256, sizeof(char));
						PluginInit(&PluginInterface, PluginsName[PluginCount]);
						LogOut("Plugin Loader/INFO", 0, "Plugin %s(%s) loaded \033[32msuccessfully\033[0m. ", PluginsName[PluginCount], wfd.cFileName);
					}
				}
				else {
					LogOut("Plugin Loader/WARN", 0, "Plugin '%s' loaded failed!", wfd.cFileName);
					PluginCount -= 1;
				}
			}
		}while(FindNextFileA(hFind, &wfd));
		free(PluginPath);
		LogOut("Plugin Loader/INFO", 0, "Loaded %d Plugins.", PluginCount);
	}
#else //DONE: Add the POSIX Systems Support
	DIR *lpDir;
	struct dirent *lpEntry;
	lpDir = opendir("./Plugins");
	if(lpDir != NULL) {
		char *PluginPath = (char*)calloc(32767, sizeof(char));
		int PluginCount;
		while((lpEntry = readdir(lpDir)) != NULL) {
			if(strcmp(lpEntry -> d_name, ".") != 0 && strcmp(lpEntry -> d_name, "..") != 0) {
				int i = strlen(lpEntry -> d_name) - 1;
				while(i >= 0 && lpEntry -> d_name[i] != '.') i -= 1;
				if(i >= 0 && lpEntry -> d_name[i] == '.' && strcmp(lpEntry -> d_name + i, ".so") == 0) {
					sprintf(PluginPath, "./Plugins/%s", lpEntry -> d_name);
					LogOut("Plugin Loader/INFO", 0, "Scanned Plugin File Name: %s", lpEntry -> d_name);
					hPlugins[++PluginCount] = dlopen(PluginPath, RTLD_LAZY);
					if(hPlugins[PluginCount] != NULL) {
						LogOut("Plugin Loader/INFO", 0, "Loading Plugin '%s'...", lpEntry -> d_name);
						PLUGININITFUNC PluginInit = (PLUGININITFUNC)dlsym(hPlugins[PluginCount], "InitInterfaces");
						char *lpError;
						if((lpError = dlerror()) != NULL) {
							LogOut("Plugin Loader/ERROR", 0, "Plugin '%s' is not available!", lpEntry -> d_name);
							PluginCount -= 1;
						}
						else {
							PluginsName[PluginCount] = (char*)calloc(256, sizeof(char));
							PluginInit(&PluginInterface, PluginsName[PluginCount]);
							LogOut("Plugin Loader/INFO", 0, "Plugin %s(%s) loaded \033[32msuccessfully\033[0m. ", PluginsName[PluginCount], lpEntry -> d_name);
						}
					}
					else {
						LogOut("Plugin Loader/WARN", 0, "Plugin '%s' loaded failed!", lpEntry -> d_name);
						PluginCount -= 1;
					}
				}
			}
		}
		closedir(lpDir);
		free(PluginPath);
		LogOut("Plugin Loader/INFO", 0, "Loaded %d Plugins.", PluginCount);
	}
	else LogOut("Plugin Loader/ERROR", 0, "opendir failed! Cannot load Plugins!");
#endif
#ifdef _WIN32 //Windows Server
	WSADATA wsaData;
	if(WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		LogOut("Server Startup/ERROR", 0, "WSAStartup \033[31mfailed\033[0m!");
		return -1;
	}
	LogOut("Server Startup/INFO", 0, "Initialized WinSock2 \033[32msuccessfully\033[0m.");
#endif //_WIN32
	SOCKET ClientSocket;
	ListenSocket = socket(AF_INET, SOCK_STREAM, 0);
	if(ListenSocket == INVALID_SOCKET) {
		LogOut("Server Startup/ERROR", 0, "Socket \033[31mFailed\033[0m!");
#ifdef _WIN32
		WSACleanup();
#endif //_WIN32
		return 1;
	}
	LogOut("Server Startup/INFO", 0, "Create socket \033[32msuccessfully\033[0m.");
	struct sockaddr_in SockAddr;
	SockAddr.sin_family = AF_INET;
	SockAddr.sin_port = htons(ListenPort);
	SockAddr.sin_addr.s_addr = htonl(INADDR_ANY); 
	iResult = bind(ListenSocket, (struct sockaddr*)&SockAddr, sizeof(SockAddr));
	if(iResult == SOCKET_ERROR) {
		LogOut("Server Startup/ERROR", 0, "Bind Listening \033[31mFailed\033[0m!");
		closesocket(ListenSocket);
#ifdef _WIN32
		WSACleanup();
#endif //_WIN32
		return 1;
	}
	LogOut("Server Startup/INFO", 0, "Bind Listening Port \033[32msuccessfully\033[0m.");
	iResult = listen(ListenSocket, 114514);
	if(iResult == SOCKET_ERROR) {
		LogOut("Server Startup/ERROR", 0, "Listen Failed!");
		closesocket(ListenSocket);
#ifdef _WIN32
		WSACleanup();
#endif
		return 1;
	}
	LogOut("Server Startup/INFO", 0, "Set queue of listening \033[32msuccessfully\033[0m.");
	LogOut("Server Startup/INFO", 0, "Your Invitation Code is \033[1m\033[36m%s\033[0m.", InvitationCode);
    LogOut("Server Startup/INFO", 0, "Listening on \033[4mhttp://localhost:%d\033[0m...", ListenPort);
    pthread_mutex_init(&thread_lock, NULL);
    pthread_mutex_init(&thread_lock_Sync, NULL);
    pthread_t ThreadId;
    pthread_create(&ThreadId, NULL, InputThread, NULL);
	system("mkdir configs");
	system("mkdir ChatFiles");
	system("mkdir ChatMessages");
	while(1) {
		ClientSocket = accept(ListenSocket, NULL, NULL);
	    if(ClientSocket == INVALID_SOCKET || ClientSocket == 0) {
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
		LogOut("Server Close/INFO", 0, "Main Config file saved \033[32msuccessfully\033[0m.");
	}
	else LogOut("Server Close/INFO", 0, "Main Config file saved \033[31mfailed\033[0m.");
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
		LogOut("Server Close/INFO", 0, "Blacklist saved \033[32msuccessfully\033[0m.");
	}
	else LogOut("Server Close/INFO", 0, "Blacklist saved \033[31mfailed\033[0m.");
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
		LogOut("Server Close/INFO", 0, "Whitelist saved \033[32msuccessfully\033[0m.");
	}
	else LogOut("Server Close/INFO", 0, "Whitelist saved \033[31mfailed\033[0m.");
	lpConfig = fopen(
#ifdef _WIN32
	"configs\\silencerlist.nchatserver"
#else
	"configs/silencerlist.nchatserver"
#endif
	, "wb");
	if(lpConfig != NULL) {
		fwrite(&RealSLC, sizeof(RealSLC), 1, lpConfig);
		for(iEnum = 1; iEnum <= SilencerListCount; iEnum += 1) {
			if(SilencerList[iEnum] != NULL) {
				iTmp = strlen(SilencerList[iEnum]);
				fwrite(&iTmp, sizeof(char), 1, lpConfig);
				fwrite(SilencerList[iEnum], sizeof(char), iTmp, lpConfig);
				fwrite("\x40", sizeof(char), 1, lpConfig);
			}
		}
		fclose(lpConfig);
		LogOut("Server Close/INFO", 0, "Silencerlist saved \033[32msuccessfully\033[0m.");
	}
	else LogOut("Server Close/INFO", 0, "Silencerlist saved \033[31mfailed\033[0m.");
	lpConfig = fopen(
#ifdef _WIN32
	"configs\\trie.nchatserver"
#else
	"configs/trie.nchatserver"
#endif
	, "wb");
	if(lpConfig != NULL) {
		SaveTrieConfigs(lpConfig, lpTrieRoot, 0);
		fclose(lpConfig);
		LogOut("Server Close/INFO", 0, "Trie Configs File saved \033[32msuccessfully\033[0m.");
	}
	return 0;
}
