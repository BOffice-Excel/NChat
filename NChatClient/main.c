#include <WinSock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <pthread.h>
#include <windows.h>
#include <uxtheme.h>
#include "..\Json.h"
#include "NChatClient-GUI_private.h"
#define key_press(key) (GetAsyncKeyState(key)&0x8000) //Define macro for detect keyboard
typedef struct tagNEWLVTILEINFO {//From Microsoft Learn
    UINT  cbSize;
    int   iItem;
    UINT  cColumns;
    PUINT puColumns;
    int   *piColFmt;
} NEWLVTILEINFO, *PNEWLVTILEINFO;
char Name[512], InvitationCode[256], ReceiveData[32767], Message[32767], IP[256], Port[8], JsonConfigFile[32767 * 16];
int ChatRoomsCount, ChatRoom_LastChoose = -1, PeopleCount, UsersCount = 0, UseDarkMode = 1;
HWND hWndMain;
HIMAGELIST hImageList;
struct People {
	char Name[512];
	struct People *Next, *Prev;
} *Head, *Last;
struct User {
	char Name[512];
	HBITMAP hProfilePicture;
} Users[32777];
HDC hDC;
struct CHATROOM {
	char Name[512], InvitationCode[256], IP[256], Port[8];
}ChatRooms[128];
HFONT hFont, hIconFont;
SOCKET sockfd;
void RecvFull(SOCKET sockfd, char *Buffer, int size) {
	int iEnum = 0, DataCount = recv(sockfd, Buffer + iEnum, size, 0);
	while(DataCount > 0) {
		iEnum += (DataCount = recv(sockfd, Buffer + iEnum, size, 0));
	}
	return;
} 
HBITMAP MakeBitmapWithName(const char *Name) {
	if(hIconFont == NULL) hIconFont = CreateFont(40, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "Consolas");
	char* ShowInBitmap = (char*)calloc(5, sizeof(char));
	int i, Count = 0;
	COLORREF BackgroundColor, ForeColor;
	for(i = 0; i < strlen(Name); i++) {
		if(Name[i] >= 'A' && Name[i] <= 'Z') {
			ShowInBitmap[Count++] = Name[i];
			if(Count == 2) break;
		}
	}
	switch(Count) {
		case 0: {
			ShowInBitmap[0] = Name[0];
			ShowInBitmap[1] = Name[1];
			BackgroundColor = RGB(Name[0] * 2 % 256, Name[1] * 2 % 256, (unsigned char)(255 - (Name[0] + Name[1])) * 2 % 256);
			break;
		}
		case 1: 
		case 2: {
			BackgroundColor = RGB(Name[0] * 2 % 256, Name[1] * 2 % 256, (unsigned char)(255 - (Name[0] + Name[1])) * 2 % 256);
			break;
		}
	}
	if(Count == 0) {
		if(ShowInBitmap[0] > 0 && ShowInBitmap[1] < 0) ShowInBitmap[2] = Name[2];
	}
	ForeColor = RGB(255 - GetRValue(BackgroundColor), 255 - GetGValue(BackgroundColor), 255 - GetBValue(BackgroundColor));
	HDC hdc = GetDC(0), hMemDC = CreateCompatibleDC(hdc);
    HBITMAP hBitmap = CreateCompatibleBitmap(hdc, 64, 64);
    SelectObject(hMemDC, hBitmap);
    RECT rect = {0, 0, 64, 64};
    FillRect(hMemDC, &rect, CreateSolidBrush(BackgroundColor));
    SetBkMode(hMemDC, TRANSPARENT);
    SetTextColor(hMemDC, ForeColor);
	SelectObject(hMemDC, hIconFont);
    DrawText(hMemDC, ShowInBitmap, strlen(ShowInBitmap), &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    DeleteDC(hMemDC);
    return hBitmap;
}
void* RecvMessageThread(void* lParam) {
	memset(ReceiveData, 0, sizeof(ReceiveData));
	while(recv(sockfd, ReceiveData, 1, 0) > 0) {
		switch(ReceiveData[0]) {
			case '\xA': {
				char Str[114514];
				recv(sockfd, ReceiveData, 1, 0);
				recv(sockfd, ReceiveData, ReceiveData[0], 0);
				sprintf(Str, "%s joined the chat room", ReceiveData);
				PeopleCount += 1;
				int i;
				for(i = 0; i < UsersCount; i++) {
					if(strcmp(Users[i].Name, ReceiveData) == 0) {
						break;
					}
				}
				if(i == UsersCount) {
					strcpy(Users[UsersCount].Name, ReceiveData);
					Users[UsersCount].hProfilePicture = MakeBitmapWithName(ReceiveData);
			    	ImageList_Add(hImageList, Users[UsersCount].hProfilePicture, NULL);
					UsersCount += 1;
					ListView_SetImageList(GetDlgItem(hWndMain, 2), hImageList, LVSIL_NORMAL);
					ListView_SetImageList(GetDlgItem(hWndMain, 8), hImageList, LVSIL_NORMAL);
				}
				LVITEMA lvi;
				lvi.pszText = Str;
				lvi.mask = LVIF_TEXT;
				lvi.iSubItem = 0;
				lvi.iItem = ListView_GetItemCount(GetDlgItem(hWndMain, 2));
				ListView_InsertItem(GetDlgItem(hWndMain, 2), &lvi);
		    	lvi.pszText = Str;
		    	lvi.mask = LVIF_TEXT;
		    	int k;
				lvi.cchTextMax = 114514;
				for(k = 0; k < ListView_GetItemCount(GetDlgItem(hWndMain, 8)); k++) {
					lvi.iItem = k;
					ListView_GetItem(GetDlgItem(hWndMain, 8), &lvi);
					if(strcmp(lvi.pszText, ReceiveData) == 0) break;
				}
				if(k == ListView_GetItemCount(GetDlgItem(hWndMain, 8))) {
					lvi.pszText = ReceiveData;
					lvi.mask = LVIF_TEXT | LVIF_IMAGE;
					lvi.iImage = i + 2;
		    		ListView_InsertItem(GetDlgItem(hWndMain, 8), &lvi);
				}
				break;
			}
			case '\xB': {
				char Str[114514];
				recv(sockfd, ReceiveData, 1, 0);
				recv(sockfd, ReceiveData, ReceiveData[0], 0);
				sprintf(Str, "%s left the chat room", ReceiveData);
				PeopleCount -= 1;
				LVITEMA lvi;
				lvi.pszText = Str;
				lvi.mask = LVIF_TEXT;
				lvi.iSubItem = 0;
				lvi.iItem = ListView_GetItemCount(GetDlgItem(hWndMain, 2));
				ListView_InsertItem(GetDlgItem(hWndMain, 2), &lvi);
				int i;
				lvi.cchTextMax = 114514;
				for(i = 0; i < ListView_GetItemCount(GetDlgItem(hWndMain, 8)); i++) {
					lvi.iItem = i;
					ListView_GetItem(GetDlgItem(hWndMain, 8), &lvi);
					if(strcmp(lvi.pszText, ReceiveData) == 0) break;
				}
				if(i != ListView_GetItemCount(GetDlgItem(hWndMain, 8))) {
					ListView_DeleteItem(GetDlgItem(hWndMain, 8), i);
				}
				break;
			}
			case '\xC': {
				EnableWindow(GetDlgItem(hWndMain, 9), FALSE);
				MessageBox(NULL, "You has kicked out by server!", "Connection closed by server", MB_ICONINFORMATION);
				DestroyWindow(hWndMain);
				SendMessage(hWndMain, WM_DESTROY, 0, 0);
				closesocket(sockfd);
				WSACleanup();
				return NULL;
				break;
			}
			case '\xD': {
			    recv(sockfd, (char*)&PeopleCount, sizeof(PeopleCount), 0);
			    int i;
				char length;
			    struct People* PNew;
			    while(Last != NULL) {
			    	Last = Last -> Prev;
			    	if(Last != NULL) free(Last -> Next);
				}
				if(hImageList != NULL) ImageList_Destroy(hImageList);
				hImageList = ImageList_Create(64, 64, ILC_COLOR32 | ILC_MASK, 1, 1);
		    	ImageList_Add(hImageList, MakeBitmapWithName("SERVER"), NULL);
		    	ImageList_Add(hImageList, MakeBitmapWithName("UnknownName"), NULL);
				Head = Last = (struct People*)calloc(1, sizeof(struct People));
				UsersCount = PeopleCount;
				LVITEMA lvi;
				ListView_DeleteAllItems(GetDlgItem(hWndMain, 8));
			    for(i = 0; i < PeopleCount; i++) {
			    	PNew = (struct People*)calloc(1, sizeof(struct People));
			    	recv(sockfd, &length, 1, 0);
			    	recv(sockfd, PNew -> Name, length, 0);
			    	strcpy(Users[i].Name, PNew -> Name);
			    	if(Users[i].hProfilePicture != NULL) DeleteObject(Users[i].hProfilePicture);
			    	Users[i].hProfilePicture = MakeBitmapWithName(Users[i].Name);
			    	ImageList_Add(hImageList, Users[i].hProfilePicture, NULL);
			    	lvi.pszText = PNew -> Name;
			    	lvi.mask = LVIF_TEXT | LVIF_IMAGE;
			    	lvi.iItem = lvi.iSubItem = 0;
			    	lvi.iImage = i + 2;
			    	ListView_InsertItem(GetDlgItem(hWndMain, 8), &lvi);
			    	PNew -> Prev = Last;
			    	Last -> Next = PNew;
			    	Last = PNew;
				}
				ListView_SetImageList(GetDlgItem(hWndMain, 2), hImageList, LVSIL_NORMAL);
				ListView_SetImageList(GetDlgItem(hWndMain, 8), hImageList, LVSIL_NORMAL);
				break;
			}
			case '\xF': {
				recv(sockfd, ReceiveData, 1, 0);
				recv(sockfd, ReceiveData, ReceiveData[0], 0);
				LVITEMA lvi;
				lvi.pszText = (char*)calloc(114514, sizeof(char));
				int idx, iReceived, i;
				sprintf(lvi.pszText, "<%s> ", ReceiveData);
				if(strcmp(ReceiveData, "Server") == 0) i = 0;
				else {
					for(i = 0; i < UsersCount; i++) {
						if(strcmp(Users[i].Name, ReceiveData) == 0) break;
					}
					if(i == UsersCount) i = -1;
					i += 2;
				}
				idx = strlen(lvi.pszText);
				do {
					memset(ReceiveData, 0, sizeof(ReceiveData));
					iReceived = recv(sockfd, ReceiveData, sizeof(ReceiveData), 0);
					memcpy(lvi.pszText + idx, ReceiveData, iReceived);
					idx += iReceived;
					if(iReceived < sizeof(ReceiveData)) break;
				}while(iReceived > 0);
				//printf("%s", ReceiveData);
				lvi.mask = LVIF_TEXT | LVIF_IMAGE;
				lvi.iSubItem = 0;
				lvi.iItem = ListView_GetItemCount(GetDlgItem(hWndMain, 2));
				lvi.iImage = i;
				printf("%s\n", lvi.pszText);
				ListView_InsertItem(GetDlgItem(hWndMain, 2), &lvi);
				free(lvi.pszText);
				break;
			}
		}
		if(IsDlgButtonChecked(hWndMain, 10) == BST_CHECKED) {
			int iItemCount = ListView_GetItemCount(GetDlgItem(hWndMain, 2));
			if (iItemCount > 0) {
				ListView_EnsureVisible(GetDlgItem(hWndMain, 2), iItemCount - 1, FALSE);
			}
		}
		//printf("\n");
		memset(ReceiveData, 0, sizeof(ReceiveData));
	}
	EnableWindow(GetDlgItem(hWndMain, 9), FALSE);
	if(MessageBox(hWndMain, "Connection closed by server.\nDo you want do quit from this page?", "Connection interrupted", MB_ICONQUESTION | MB_YESNO) == IDYES) {
		DestroyWindow(hWndMain);
		SendMessage(hWndMain, WM_DESTROY, 0, 0);
	}
	send(sockfd, "\xF", 1, 0);
    closesocket(sockfd);
	WSACleanup();
	return NULL;
}
void UpdateUsersList() {
    send(sockfd, "\xD", 1, 0);
	return ;
}
BOOL Signin(const char *IP, const char *Port, const char *Name, const char *InvitationCode) {
	struct addrinfo hints, *res;
	int status;
	
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	
	if ((status = getaddrinfo(IP, Port, &hints, &res)) != 0) {
		sprintf(ReceiveData, "\xFF|ERR_ADDRESS_INVALID");
	    printf("Getaddrinfo Failed: %s(%d)\n", gai_strerror(status), status);
	    return FALSE;
	}
	
	sockfd = socket(res -> ai_family, res -> ai_socktype, res -> ai_protocol);
	if(sockfd == INVALID_SOCKET) {
		sprintf(ReceiveData, "\xFE|ERR_SOCKET_FAILED");
        return FALSE;
    }
    int iResult = connect(sockfd, res->ai_addr, res->ai_addrlen), iLen = strlen(Name);
    if(iResult == SOCKET_ERROR) {
		sprintf(ReceiveData, "\xFD|ERR_CONNECTION_REFUSED");
		closesocket(sockfd);
    	return FALSE;
	}
    iResult = send(sockfd, "\xA", 1, 0);
	if(iResult == SOCKET_ERROR) {
		sprintf(ReceiveData, "\xFC|ERR_SEND_FAILED");
		closesocket(sockfd);
		return FALSE;
    }
    char VersionName[5];
    VersionName[0] = VER_MAJOR;
	VersionName[1] = VER_MINOR;
	VersionName[2] = VER_RELEASE;
    send(sockfd, VersionName, 3, 0);
    send(sockfd, InvitationCode, 128, 0);
    send(sockfd, (char*)&iLen, 1, 0);
    send(sockfd, Name, iLen, 0);
    recv(sockfd, ReceiveData, sizeof(ReceiveData), 0);
    //RecvFull(sockfd, ReceiveData, sizeof(ReceiveData));
    if(ReceiveData[0] != '\x1') {
    	printf("Cannot connect to server: %s(%d)\n", ReceiveData + 2, ReceiveData[0]);
    	return FALSE;
	}
	else {
    	printf("Connect to chat server successfully\n");
	}
	return TRUE;
}
LRESULT CALLBACK LoginWndProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam) {
	switch(Message) {
		case WM_CREATE: {
			int i;
			CreateWindowExA((UseDarkMode == 0) * WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_GROUP | ES_AUTOHSCROLL | (UseDarkMode * WS_BORDER), 0, 0, 0, 0, hWnd, (HMENU)1, NULL, NULL);
			CreateWindowExA(0, WC_COMBOBOXA, NULL, WS_CHILD | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWN, 0, 0, 0, 0, hWnd, (HMENU)2, NULL, NULL);
			CreateWindowExA((UseDarkMode == 0) * WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL | (UseDarkMode * WS_BORDER), 0, 0, 0, 0, hWnd, (HMENU)3, NULL, NULL);
			CreateWindowExA((UseDarkMode == 0) * WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL | (UseDarkMode * WS_BORDER), 0, 0, 0, 0, hWnd, (HMENU)4, NULL, NULL);
			CreateWindowExA((UseDarkMode == 0) * WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_VISIBLE | ES_NUMBER | WS_TABSTOP | ES_AUTOHSCROLL | (UseDarkMode * WS_BORDER), 0, 0, 0, 0, hWnd, (HMENU)5, NULL, NULL);
			CreateWindowExA(0, "BUTTON", "Save", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 0, 0, 0, 0, hWnd, (HMENU)6, NULL, NULL);
			CreateWindowExA(0, "BUTTON", "Join In", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON | WS_TABSTOP, 0, 0, 0, 0, hWnd, (HMENU)7, NULL, NULL);
			CreateWindowExA(0, "BUTTON", "Cancel", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 0, 0, 0, 0, hWnd, (HMENU)8, NULL, NULL);
			SendDlgItemMessage(hWnd, 1, EM_SETCUEBANNER, TRUE, (LPARAM)L"Your User Name");
			SendDlgItemMessage(hWnd, 1, EM_LIMITTEXT, 512, 0);
			SendDlgItemMessage(hWnd, 3, EM_SETCUEBANNER, TRUE, (LPARAM)L"Your Invitation Code(It will show in the NChat Server's Log)");
			SendDlgItemMessage(hWnd, 3, EM_LIMITTEXT, 128, 0);
			SendDlgItemMessage(hWnd, 4, EM_SETCUEBANNER, TRUE, (LPARAM)L"Your NChat Server's IP/Hostname");
			SendDlgItemMessage(hWnd, 4, EM_LIMITTEXT, 256, 0);
			SendDlgItemMessage(hWnd, 5, EM_SETCUEBANNER, TRUE, (LPARAM)L"Your NChat Server's Port");
			SendDlgItemMessage(hWnd, 5, EM_LIMITTEXT, 5, 0);
			for(i = 0; i < ChatRoomsCount; i++) {
				SendDlgItemMessage(hWnd, 2, CB_ADDSTRING, 0, (LPARAM)ChatRooms[i].Name);
			}
			SendDlgItemMessage(hWnd, 2, CB_ADDSTRING, 0, (LPARAM)"New Chat Room");
			SendDlgItemMessage(hWnd, 2, CB_SETCURSEL, 0, 0);
			SendMessage(hWnd, WM_COMMAND, MAKEWPARAM(2, CBN_SELCHANGE), (LPARAM)GetDlgItem(hWnd, 2));
			for(i = 1; i <= 8; i++) {
				SendDlgItemMessage(hWnd, i, WM_SETFONT, (WPARAM)hFont, 0);
				SetWindowTheme(GetDlgItem(hWnd, i), (UseDarkMode == 0) ? L"Explorer" : L"DarkMode_Explorer", NULL);
			}
			if(Name[0] != '\0') SetDlgItemText(hWnd, 1, Name);
			break;
		}
		case WM_COMMAND: {
			switch(LOWORD(wParam)) {
				case 2: {
					switch(HIWORD(wParam)) {
						case CBN_SELCHANGE: {
							if(ChatRoom_LastChoose != -1) {
								GetDlgItemText(hWnd, 2, ChatRooms[ChatRoom_LastChoose].Name, sizeof(ChatRooms[ChatRoom_LastChoose].Name));
								GetDlgItemText(hWnd, 3, ChatRooms[ChatRoom_LastChoose].InvitationCode, sizeof(ChatRooms[ChatRoom_LastChoose].InvitationCode));
								GetDlgItemText(hWnd, 4, ChatRooms[ChatRoom_LastChoose].IP, sizeof(ChatRooms[ChatRoom_LastChoose].IP));
								GetDlgItemText(hWnd, 5, ChatRooms[ChatRoom_LastChoose].Port, sizeof(ChatRooms[ChatRoom_LastChoose].Port));
							}
							int id = SendMessage((HWND)lParam, CB_GETCURSEL, 0, 0);
							ChatRoom_LastChoose = id;
							SetDlgItemText(hWnd, 3, ChatRooms[id].InvitationCode);
							SetDlgItemText(hWnd, 4, ChatRooms[id].IP);
							SetDlgItemText(hWnd, 5, ChatRooms[id].Port);
							break;
						}
					}
					break;
				}
				case 6: {
					GetDlgItemText(hWnd, 2, ChatRooms[ChatRoom_LastChoose].Name, sizeof(ChatRooms[ChatRoom_LastChoose].Name));
					GetDlgItemText(hWnd, 3, ChatRooms[ChatRoom_LastChoose].InvitationCode, sizeof(ChatRooms[ChatRoom_LastChoose].InvitationCode));
					GetDlgItemText(hWnd, 4, ChatRooms[ChatRoom_LastChoose].IP, sizeof(ChatRooms[ChatRoom_LastChoose].IP));
					GetDlgItemText(hWnd, 5, ChatRooms[ChatRoom_LastChoose].Port, sizeof(ChatRooms[ChatRoom_LastChoose].Port));
					int i, ShouldBeSelect = SendDlgItemMessage(hWnd, 2, CB_GETCURSEL, 0, 0);
					if(ChatRoomsCount == SendDlgItemMessage(hWnd, 2, CB_GETCURSEL, 0, 0) || SendDlgItemMessage(hWnd, 2, CB_GETCURSEL, 0, 0) == -1) {
						ShouldBeSelect = ChatRoomsCount;
						ChatRoomsCount += 1;
					}
					SendDlgItemMessage(hWnd, 2, CB_RESETCONTENT, 0, 0);
					for(i = 0; i < ChatRoomsCount; i++) {
						SendDlgItemMessage(hWnd, 2, CB_ADDSTRING, 0, (LPARAM)ChatRooms[i].Name);
					}
					SendDlgItemMessage(hWnd, 2, CB_ADDSTRING, 0, (LPARAM)"New Chat Room");
					SendDlgItemMessage(hWnd, 2, CB_SETCURSEL, ShouldBeSelect, 0);
					SendMessage(hWnd, WM_COMMAND, MAKEWPARAM(2, CBN_SELCHANGE), (LPARAM)GetDlgItem(hWnd, 2));
					break;
				}
				case 7: {
					SendMessage(hWnd, WM_COMMAND, 6, 0);
					memset(Name, 0, sizeof(Name));
					GetDlgItemText(hWnd, 1, Name, sizeof(Name));
					if(Name[0] == '\0') {
						EDITBALLOONTIP ebt;
						ebt.cbStruct = sizeof(ebt);
						ebt.pszText = L"This option is empty!";
						ebt.pszTitle = L"Oops";
						ebt.ttiIcon = TTI_INFO;
						Edit_ShowBalloonTip(GetDlgItem(hWnd, 1), &ebt);
						break;
					}
					memset(InvitationCode, 0, sizeof(InvitationCode));
					GetDlgItemText(hWnd, 3, InvitationCode, sizeof(InvitationCode));
					if(InvitationCode[0] == '\0') {
						EDITBALLOONTIP ebt;
						ebt.cbStruct = sizeof(ebt);
						ebt.pszText = L"This option is empty!";
						ebt.pszTitle = L"Oops";
						ebt.ttiIcon = TTI_INFO;
						Edit_ShowBalloonTip(GetDlgItem(hWnd, 3), &ebt);
						break;
					}
					memset(IP, 0, sizeof(IP));
					GetDlgItemText(hWnd, 4, IP, sizeof(IP));
					if(IP[0] == '\0') {
						EDITBALLOONTIP ebt;
						ebt.cbStruct = sizeof(ebt);
						ebt.pszText = L"This option is empty!";
						ebt.pszTitle = L"Oops";
						ebt.ttiIcon = TTI_INFO;
						Edit_ShowBalloonTip(GetDlgItem(hWnd, 4), &ebt);
						break;
					}
					memset(Port, 0, sizeof(Port));
					GetDlgItemText(hWnd, 5, Port, sizeof(Port));
					if(Port[0] == '\0') {
						EDITBALLOONTIP ebt;
						ebt.cbStruct = sizeof(ebt);
						ebt.pszText = L"This option is empty!";
						ebt.pszTitle = L"Oops";
						ebt.ttiIcon = TTI_INFO;
						Edit_ShowBalloonTip(GetDlgItem(hWnd, 5), &ebt);
						break;
					}
					char Buffer[114514];
					GetDlgItemText(hWnd,2, Buffer, 114514);
					printf("%s %d\n", Buffer, SendDlgItemMessage(hWnd, 2, CB_GETCURSEL, 0, 0));
					if(Signin(IP, Port, Name, InvitationCode) == TRUE) {
						DestroyWindow(hWnd);
						PostQuitMessage(1);
					}
					else {
						char TipsString[32767];
						sprintf(TipsString, "Cannot connect to server: %s(%d)", ReceiveData + 2, ReceiveData[0]);
						MessageBox(hWnd, TipsString, "Connected to server failed!", MB_ICONERROR);
					}
					break;
				}
				case 8: {
					SendMessage(hWnd, WM_COMMAND, 6, 0);
					DestroyWindow(hWnd);
					break;
				}
			}
			break;
		}
		case WM_CTLCOLORBTN: 
		case WM_CTLCOLORSTATIC:
		case WM_CTLCOLOREDIT:
		case WM_CTLCOLORMSGBOX:
		case WM_CTLCOLORDLG:
		case WM_CTLCOLORLISTBOX:
		case WM_CTLCOLORSCROLLBAR: {
			if(UseDarkMode == 1) {
				static HBRUSH hbrBkgnd = NULL;
		        HDC hdcStatic = (HDC)wParam;
		        SetTextColor(hdcStatic, RGB(255, 255, 255));
		        SetBkColor(hdcStatic, RGB(25, 25, 25));
		    	if(hbrBkgnd == NULL) {
		            hbrBkgnd = CreateSolidBrush(RGB(25, 25, 25));
		        }
		        return (INT_PTR)hbrBkgnd;
			}
			else {
				static HBRUSH hbrBkgnd = NULL;
		        HDC hdcStatic = (HDC)wParam;
		        SetBkColor(hdcStatic, RGB(255, 255, 255));
		    	if(hbrBkgnd == NULL) {
		            hbrBkgnd = CreateSolidBrush(RGB(255, 255, 255));
		        }
		        return (INT_PTR)hbrBkgnd;
	    	}
	    	break;
	    }
		case WM_SIZE: {
			RECT Rect;
			GetClientRect(hWnd, &Rect);
			SetWindowPos(GetDlgItem(hWnd, 1), NULL, 20, 20, Rect.right - 40, 30, SWP_NOZORDER);
			SetWindowPos(GetDlgItem(hWnd, 2), NULL, 20, 70, Rect.right - 40, 30, SWP_NOZORDER);
			SetWindowPos(GetDlgItem(hWnd, 3), NULL, 20, 120, Rect.right - 40, 30, SWP_NOZORDER);
			SetWindowPos(GetDlgItem(hWnd, 4), NULL, 20, 170, Rect.right - 40, 30, SWP_NOZORDER);
			SetWindowPos(GetDlgItem(hWnd, 5), NULL, 20, 220, Rect.right - 40, 30, SWP_NOZORDER);
			SetWindowPos(GetDlgItem(hWnd, 6), NULL, Rect.right - 190, 270, 150, 30, SWP_NOZORDER);
			SetWindowPos(GetDlgItem(hWnd, 7), NULL, Rect.right - 360, Rect.bottom - 50, 150, 30, SWP_NOZORDER);
			SetWindowPos(GetDlgItem(hWnd, 8), NULL, Rect.right - 190, Rect.bottom - 50, 150, 30, SWP_NOZORDER);
			break;
		}
		case WM_DESTROY: {
			PostQuitMessage(0);
			break;
		}
		default: return DefWindowProc(hWnd, Message, wParam, lParam);
	}
	return 0;
}
LRESULT CALLBACK WndProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam) {
	switch(Message) {
		case WM_CREATE: {
			UpdateUsersList();
		    CreateWindowExA(0, "STATIC", "Chat List: ",
				WS_CHILD | WS_TABSTOP | WS_VISIBLE, 0, 0, 0, 0, hWnd, (HMENU)1, NULL, NULL);
			CreateWindowExA(0, WC_LISTVIEWA, "Chat List",
				WS_CHILD | WS_TABSTOP | WS_GROUP | WS_VISIBLE | LVS_SINGLESEL, 0, 0, 0, 0, hWnd, (HMENU)2, NULL, NULL);
			CreateWindowExA((UseDarkMode == 0) * WS_EX_CLIENTEDGE, "EDIT", "",
				WS_CHILD | WS_TABSTOP | WS_VISIBLE | ES_AUTOHSCROLL | ES_AUTOVSCROLL | WS_HSCROLL | WS_VSCROLL | ES_MULTILINE | (UseDarkMode * WS_BORDER), 0, 0, 0, 0, hWnd, (HMENU)3, NULL, NULL);
			CreateWindowExA(0, "STATIC", "User Name: ",
				WS_CHILD | WS_TABSTOP | WS_VISIBLE, 0, 0, 0, 0, hWnd, (HMENU)4, NULL, NULL);
			CreateWindowExA(0, "STATIC", "Server Name: ",
				WS_CHILD | WS_TABSTOP | WS_VISIBLE, 0, 0, 0, 0, hWnd, (HMENU)5, NULL, NULL);
			CreateWindowExA((UseDarkMode == 0) * WS_EX_CLIENTEDGE, "EDIT", "",
				WS_CHILD | WS_TABSTOP | WS_VISIBLE | ES_AUTOHSCROLL | ES_READONLY | (UseDarkMode * WS_BORDER), 0, 0, 0, 0, hWnd, (HMENU)6, NULL, NULL);
			CreateWindowExA((UseDarkMode == 0) * WS_EX_CLIENTEDGE, "EDIT", "",
				WS_CHILD | WS_TABSTOP | WS_VISIBLE | ES_AUTOHSCROLL | ES_READONLY | (UseDarkMode * WS_BORDER), 0, 0, 0, 0, hWnd, (HMENU)7, NULL, NULL);
			CreateWindowExA(0, WC_LISTVIEWA, "Users List",
				WS_CHILD | WS_TABSTOP | WS_VISIBLE | LVS_SINGLESEL, 0, 0, 0, 0, hWnd, (HMENU)8, NULL, NULL);
			CreateWindowExA(0, "BUTTON", "Send",
				WS_CHILD | WS_TABSTOP | WS_VISIBLE | BS_DEFPUSHBUTTON, 0, 0, 0, 0, hWnd, (HMENU)9, NULL, NULL);
			CreateWindowExA(0, "BUTTON", "Auto Scroll To Bottom",
				WS_CHILD | WS_TABSTOP | WS_VISIBLE | BS_AUTOCHECKBOX, 0, 0, 0, 0, hWnd, (HMENU)10, NULL, NULL);
			SendDlgItemMessage(hWnd, 3, EM_SETLIMITTEXT, 32766, 0);
			ListView_SetView(GetDlgItem(hWnd, 2), LV_VIEW_TILE);
			if(UseDarkMode == 1) {
				ListView_SetBkColor(GetDlgItem(hWnd, 2), RGB(25, 25, 25));
				ListView_SetTextBkColor(GetDlgItem(hWnd, 2), RGB(25, 25, 25));
				ListView_SetTextColor(GetDlgItem(hWnd, 2), RGB(255, 255, 255));
				ListView_SetBkColor(GetDlgItem(hWnd, 8), RGB(25, 25, 25));
				ListView_SetTextBkColor(GetDlgItem(hWnd, 8), RGB(25, 25, 25));
				ListView_SetTextColor(GetDlgItem(hWnd, 8), RGB(255, 255, 255));
			}
			else {
				ListView_SetTextColor(GetDlgItem(hWnd, 2), RGB(0, 0, 0));
				ListView_SetTextColor(GetDlgItem(hWnd, 8), RGB(0, 0, 0));
			}
			int i;
			for(i = 1; i <= 10; i++) {
				SendDlgItemMessage(hWnd, i, WM_SETFONT, (WPARAM)hFont, 0);
				SetWindowTheme(GetDlgItem(hWnd, i), (UseDarkMode == 0) ? L"Explorer" : L"DarkMode_Explorer", NULL);
			}
			break;
		}
		case WM_SIZE: {
			RECT Rect;
			GetClientRect(hWnd, &Rect);
			SetWindowPos(GetDlgItem(hWnd, 1), NULL, 20, 20, (Rect.right - 110) * 0.7, 30, SWP_NOZORDER);
			SetWindowPos(GetDlgItem(hWnd, 2), NULL, 20, 70, (Rect.right - 110) * 0.7, Rect.bottom - 240, SWP_NOZORDER);
			SetWindowPos(GetDlgItem(hWnd, 3), NULL, 20, Rect.bottom - 150, (Rect.right - 110) * 0.7, 130, SWP_NOZORDER);
			SetWindowPos(GetDlgItem(hWnd, 4), NULL, (Rect.right - 110) * 0.7 + 40, 20, (Rect.right - 110) * 0.1, 30, SWP_NOZORDER);
			SetWindowPos(GetDlgItem(hWnd, 5), NULL, (Rect.right - 110) * 0.7 + 40, 60, (Rect.right - 110) * 0.1, 30, SWP_NOZORDER);
			SetWindowPos(GetDlgItem(hWnd, 6), NULL, (Rect.right - 110) * 0.8 + 60, 20, (Rect.right - 110) * 0.23, 30, SWP_NOZORDER);
			SetWindowPos(GetDlgItem(hWnd, 7), NULL, (Rect.right - 110) * 0.8 + 60, 60, (Rect.right - 110) * 0.23, 30, SWP_NOZORDER);
			SetWindowPos(GetDlgItem(hWnd, 8), NULL, (Rect.right - 110) * 0.7 + 40, 110, (Rect.right - 110) * 0.3, (Rect.bottom - 130) * 0.6, SWP_NOZORDER);
			SetWindowPos(GetDlgItem(hWnd, 9), NULL, (Rect.right - 110) * 0.7 + 40, Rect.bottom - 50, (Rect.right - 110) * 0.1, 30, SWP_NOZORDER);
			SetWindowPos(GetDlgItem(hWnd, 10), NULL, (Rect.right - 110) * 0.7 + 40, (Rect.bottom - 130) * 0.6 + 130, (Rect.right - 110) * 0.3, 30, SWP_NOZORDER);
			LVTILEVIEWINFO tileViewInfo;
            tileViewInfo.cbSize = sizeof(LVTILEVIEWINFO);
            tileViewInfo.dwFlags = LVTVIF_FIXEDSIZE;
            tileViewInfo.dwMask = LVTVIM_COLUMNS | LVTVIM_TILESIZE;
            tileViewInfo.cLines = 2;
            tileViewInfo.sizeTile.cx = (Rect.right - 110) * 0.7 - 30;
            tileViewInfo.sizeTile.cy = 70;
            ListView_SetTileViewInfo(GetDlgItem(hWnd, 2), &tileViewInfo);//Set Tile View Information
			break;
		}
		case WM_CTLCOLORBTN: 
		case WM_CTLCOLORSTATIC:
		case WM_CTLCOLOREDIT:
		case WM_CTLCOLORMSGBOX:
		case WM_CTLCOLORDLG:
		case WM_CTLCOLORLISTBOX:
		case WM_CTLCOLORSCROLLBAR: {
			if(UseDarkMode == 1) {
				static HBRUSH hbrBkgnd = NULL;
		        HDC hdcStatic = (HDC)wParam;
		        SetTextColor(hdcStatic, RGB(255, 255, 255));
		        SetBkColor(hdcStatic, RGB(25, 25, 25));
		    	if(hbrBkgnd == NULL) {
		            hbrBkgnd = CreateSolidBrush(RGB(25, 25, 25));
		        }
		        return (INT_PTR)hbrBkgnd;
			}
			else {
				static HBRUSH hbrBkgnd = NULL;
		        HDC hdcStatic = (HDC)wParam;
		        SetBkColor(hdcStatic, RGB(255, 255, 255));
		    	if(hbrBkgnd == NULL) {
		            hbrBkgnd = CreateSolidBrush(RGB(255, 255, 255));
		        }
		        return (INT_PTR)hbrBkgnd;
	    	}
	        break;
        }
        case WM_COMMAND: {
        	switch(LOWORD(wParam)) {
        		case 9: {
        			char *lpstrMessage = (char*)calloc(65534, sizeof(char));
        			GetDlgItemTextA(hWnd, 3, lpstrMessage, 65534);
        			send(sockfd, "\xB", 1, 0); 
        			send(sockfd, lpstrMessage, strlen(lpstrMessage), 0);
        			SetDlgItemTextA(hWnd, 3, "");
					break;
				}
			}
			break;
		}
		case WM_TIMER: {
			//DrawState(hDC, NULL, NULL, (LPARAM)GetPropA(hwnd, "bitmap"), 0, 0, 0, 64, 64, DST_BITMAP);
			break;
		}
		case WM_DESTROY: {
			PostQuitMessage(0);
			break;
		}
		default: return DefWindowProc(hWnd, Message, wParam, lParam);
	}
	return 0;
}
int main() {
	INITCOMMONCONTROLSEX icex;
	icex.dwSize = sizeof(icex);
	icex.dwICC = ICC_LISTVIEW_CLASSES;
	InitCommonControlsEx(&icex);
	WSADATA wsaData;
	if(WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		return -1;
	}
	typedef BOOL WINAPI (*SPDA)(VOID);
	SPDA SetProcessDPIAware;
	HMODULE hModule = LoadLibrary("user32.dll");
	if(hModule) {//Exception handling
		SetProcessDPIAware = (SPDA)GetProcAddress(hModule, "SetProcessDPIAware");
		if(SetProcessDPIAware) SetProcessDPIAware();
	}
	hFont = CreateFont(20, 0, 0, 0, 0, 0, 0, 0, DEFAULT_CHARSET, 0, 0, 0, 0, "Segoe UI");
	HANDLE hFile = CreateFile("Config.json", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if(hFile != INVALID_HANDLE_VALUE && hFile != NULL) {
		DWORD dw;
		ReadFile(hFile, JsonConfigFile, sizeof(JsonConfigFile), &dw, NULL);
		CloseHandle(hFile);
		JsonObjectType Jot;
		JSONOBJ* lpConfigHandle = BuildJson(JsonConfigFile, strlen(JsonConfigFile), &Jot), *lpRooms, *lpUserName;
		lpRooms = FindKey(1, lpConfigHandle, "ChatRooms");
		if(lpRooms != NULL) {
			lpRooms = lpRooms -> Childs;
			ChatRoomsCount = 0;
			while(lpRooms != NULL) {
				JSONOBJ* lpRoom_Name, *lpRoom_IP, *lpRoom_Port, *lpRoom_InvitationCode;
				lpRoom_Name = FindKey(1, lpRooms, "Name");
				if(lpRoom_Name != NULL) {
					lpRoom_IP = FindKey(1, lpRooms, "IP");
					if(lpRoom_IP != NULL) {
						lpRoom_Port = FindKey(1, lpRooms, "Port");
						if(lpRoom_Port != NULL) {
							lpRoom_InvitationCode = FindKey(1, lpRooms, "InvitationCode");
							if(lpRoom_InvitationCode != NULL) {
								strcpy(ChatRooms[ChatRoomsCount].Name, lpRoom_Name -> lpstrValue);
								strcpy(ChatRooms[ChatRoomsCount].IP, lpRoom_IP -> lpstrValue);
								strcpy(ChatRooms[ChatRoomsCount].Port, lpRoom_Port -> lpstrValue);
								strcpy(ChatRooms[ChatRoomsCount].InvitationCode, lpRoom_InvitationCode -> lpstrValue);
								ChatRoomsCount += 1;
							}
						}
					}
				}
				lpRooms = lpRooms -> Next;
			}
			lpUserName = FindKey(1, lpConfigHandle, "UserName");
			if(lpUserName != NULL) {
				strcpy(Name, lpUserName -> lpstrValue);
			}
			else ChatRoomsCount = 0;
			lpUserName = FindKey(1, lpConfigHandle, "DarkMode");
			if(lpUserName != NULL) {
				UseDarkMode = lpUserName -> TorF;
			}
			CloseJson(lpConfigHandle);
		}
	}
	WNDCLASSEXA wc;
	MSG Msg;

	memset(&wc, 0, sizeof(wc));
	wc.cbSize		 = sizeof(WNDCLASSEX);
	wc.lpfnWndProc	 = LoginWndProc;
	wc.hInstance	 = GetModuleHandle(NULL);
	wc.hCursor		 = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (UseDarkMode == 1) ? CreateSolidBrush(RGB(25, 25, 25)) : CreateSolidBrush(RGB(255, 255, 255));
	wc.lpszClassName = "NChat-Client-Login";
	wc.hIcon		 = LoadIcon(wc.hInstance, "A"); /* use "A" as icon name when you want to use the project icon */
	wc.hIconSm		 = LoadIcon(wc.hInstance, "A"); /* as above */
	if(!RegisterClassEx(&wc)) {
		MessageBox(NULL, "Window Registration Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}
	wc.lpfnWndProc	 = WndProc;
	wc.lpszClassName = "NChat-Client";
	if(!RegisterClassEx(&wc)) {
		MessageBox(NULL, "Window Registration Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
		return -1;
	}
	hWndMain = CreateWindowEx(0, "NChat-Client-Login", "Login the chat room",
		WS_VISIBLE | WS_MINIMIZEBOX | WS_SYSMENU | WS_CAPTION,
		CW_USEDEFAULT, CW_USEDEFAULT, 1024, 512, NULL, NULL, NULL, NULL);
	if(hWndMain == NULL) {
		MessageBox(NULL, "Window Creation Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
		return -1;
	}
	while(GetMessage(&Msg, NULL, 0, 0) > 0) {
		if(IsDialogMessage(hWndMain, &Msg) == FALSE) {
			TranslateMessage(&Msg);
			DispatchMessage(&Msg);
		}
	}
	hFile = CreateFileA("Config.json", GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hFile == INVALID_HANDLE_VALUE || hFile == NULL) MessageBox(NULL, "Tried to create file 'Config.json' failed!", "Error!", MB_ICONWARNING);
	else {
		DWORD dw;
		WriteFile(hFile, "{\"UserName\": \"", 14, &dw, NULL);
		WriteFile(hFile, Name, strlen(Name), &dw, NULL);
		WriteFile(hFile, "\",\"ChatRooms\": [", 16, &dw, NULL);
		int i;
		for(i = 0; i < ChatRoomsCount; i++) {
			WriteFile(hFile, "{\"Name\": \"", 10, &dw, NULL);
			WriteFile(hFile, ChatRooms[i].Name, strlen(ChatRooms[i].Name), &dw, NULL);
			WriteFile(hFile, "\",\"IP\": \"", 9, &dw, NULL);
			WriteFile(hFile, ChatRooms[i].IP, strlen(ChatRooms[i].IP), &dw, NULL);
			WriteFile(hFile, "\",\"Port\": \"", 11, &dw, NULL);
			WriteFile(hFile, ChatRooms[i].Port, strlen(ChatRooms[i].Port), &dw, NULL);
			WriteFile(hFile, "\",\"InvitationCode\": \"", 21, &dw, NULL);
			WriteFile(hFile, ChatRooms[i].InvitationCode, strlen(ChatRooms[i].InvitationCode), &dw, NULL);
			WriteFile(hFile, "\"}", 2, &dw, NULL);
			if(i != ChatRoomsCount - 1) WriteFile(hFile, ",", 1, &dw, NULL);
		}
		WriteFile(hFile, "],\"DarkMode\": ", 14, &dw, NULL);
		WriteFile(hFile, UseDarkMode == 1 ? "true" : "false", 5 - UseDarkMode, &dw, NULL);
		WriteFile(hFile, "}", 1, &dw, NULL);
		CloseHandle(hFile);
	}
	if(Msg.wParam == 0) return 0;
	hWndMain = CreateWindowEx(0, "NChat-Client", "NChat",
		WS_VISIBLE | WS_MINIMIZEBOX | WS_SYSMENU | WS_CAPTION | WS_MAXIMIZEBOX | WS_THICKFRAME,
		CW_USEDEFAULT, CW_USEDEFAULT, 1536, 768, NULL, NULL, NULL, NULL);
	if(hWndMain == NULL) {
		MessageBox(NULL, "Window Creation Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
		return -1;
	}
	pthread_t MessageHandler_t;
	pthread_create(&MessageHandler_t, NULL, RecvMessageThread, NULL);
	while(GetMessage(&Msg, NULL, 0, 0) > 0) {
		if(IsDialogMessage(hWndMain, &Msg) == FALSE) {
			TranslateMessage(&Msg);
			DispatchMessage(&Msg);
		}
	}
	closesocket(sockfd);
	WSACleanup();
	return Msg.wParam;
}
