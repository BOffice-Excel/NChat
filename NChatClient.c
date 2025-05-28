#include <WinSock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <pthread.h>
#include <signal.h>
char Name[512], InvitationCode[256], ReceiveData[32767], Message[32767], IP[128], Port[8];
void RecvFull(SOCKET sockfd, char *Buffer, int size) {
	int iEnum = 0, DataCount = recv(sockfd, Buffer + iEnum, size, 0);
	while(DataCount > 0) {
		iEnum += (DataCount = recv(sockfd, Buffer + iEnum, size, 0));
	}
	return;
} 
void* InputThread(void* lParam) {
	SOCKET* sockfd = (SOCKET*)lParam;
	gets(Message);
	while(1) {
		gets(Message);
		if(*sockfd == 0) return NULL;
		if(Message[0] == '\0') continue;
		if(strcmp(Message, "quit") == 0) {
			break;
		}
		if(send(*sockfd, "\xB", 1, 0) <= 0) return NULL;
		send(*sockfd, Message, strlen(Message), 0);
	}
	send(*sockfd, "\xF", 1, 0);
	return NULL;
}
int main() {
	WSADATA wsaData;
	if(WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		return -1;
	}
	printf("Please enter your user name here: ");
	scanf("%s", Name);
	printf("Please enter your Invitation Code here: ");
	scanf("%s", InvitationCode);
	printf("Please enter the IP of your chat server: ");
	scanf("%s", IP);
	printf("Please enter the Port of your chat server: ");
	scanf("%s", Port);
	struct addrinfo hints, *res;
	int status;
	
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	
	if ((status = getaddrinfo(IP, Port, &hints, &res)) != 0) {
	    printf("Getaddrinfo Failed: %s(%d)\n", gai_strerror(status), status);
	    return -1;
	}
	
	SOCKET sockfd = socket(res -> ai_family, res -> ai_socktype, res -> ai_protocol);
	if(sockfd == INVALID_SOCKET) {
        return -1;
    }
    int iResult = connect(sockfd, res->ai_addr, res->ai_addrlen), iLen = strlen(Name);
    if(iResult == SOCKET_ERROR) {
		closesocket(sockfd);
    	return -1;
	}
    iResult = send(sockfd, "\xA", 1, 0);
	if(iResult == SOCKET_ERROR) {
		closesocket(sockfd);
		return -1;
    }
    char VersionName[5];
    VersionName[0] = 1;
	VersionName[1] = 0;
	VersionName[2] = 0;
    send(sockfd, VersionName, 3, 0);
    send(sockfd, InvitationCode, 128, 0);
    send(sockfd, (char*)&iLen, 1, 0);
    send(sockfd, Name, iLen, 0);
    recv(sockfd, ReceiveData, sizeof(ReceiveData), 0);
    //RecvFull(sockfd, ReceiveData, sizeof(ReceiveData));
    if(ReceiveData[0] != '\x1') {
    	printf("Cannot connect to server: %s(%d)\n", ReceiveData + 2, ReceiveData[0]);
    	WSACleanup();
    	system("pause");
    	return 0;
	}
	else {
    	printf("Connect to server successfully\n");
	}
	pthread_t tid_t;
	pthread_create(&tid_t, NULL, InputThread, (void*)&sockfd);
	memset(ReceiveData, 0, sizeof(ReceiveData));
	while(recv(sockfd, ReceiveData, 1, 0) > 0) {
		switch(ReceiveData[0]) {
			case '\xA': {
				recv(sockfd, ReceiveData, 1, 0);
				recv(sockfd, ReceiveData, ReceiveData[0], 0);
				printf("%s joined the chat room.", ReceiveData);
				break;
			}
			case '\xB': {
				recv(sockfd, ReceiveData, 1, 0);
				recv(sockfd, ReceiveData, ReceiveData[0], 0);
				printf("%s left the chat room.", ReceiveData);
				break;
			}
			case '\xF': {
				recv(sockfd, ReceiveData, 1, 0);
				recv(sockfd, ReceiveData, ReceiveData[0], 0);
				printf("<%s> ", ReceiveData);
				memset(ReceiveData, 0, sizeof(ReceiveData));
				recv(sockfd, ReceiveData, sizeof(ReceiveData), 0);
				printf("%s", ReceiveData);
				break;
			}
		}
		printf("\n");
		memset(ReceiveData, 0, sizeof(ReceiveData));
	}
	send(sockfd, "\xF", 1, 0);
    closesocket(sockfd);
    sockfd = 0;
	WSACleanup();
	pthread_kill(tid_t, SIGINT);
	system("pause");
	exit(0);
	return 0;
}
