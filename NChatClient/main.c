#define NTDDI_VERSION 0x06010000
#include <WinSock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <time.h>
#include <pthread.h>
#include <uxtheme.h>
#include <errno.h>
#include <exdisp.h>
#include <exdispid.h>
#include "NChatClient-GUI_private.h"
#define key_press(key) (GetAsyncKeyState(key)&0x8000) //Define macro for detect keyboard
/*typedef struct IOleObjectVtbl2 {
    // IUnknown 方法
    HRESULT (STDMETHODCALLTYPE *QueryInterface)(
        IOleObject* This,
        REFIID riid,
        void** ppvObject);
 
    ULONG (STDMETHODCALLTYPE *AddRef)(
        IOleObject* This);
 
    ULONG (STDMETHODCALLTYPE *Release)(
        IOleObject* This);
 
    // IOleObject 特有方法
    HRESULT (STDMETHODCALLTYPE *SetClientSite)(
        IOleObject* This,
        IOleClientSite* pClientSite);
 
    HRESULT (STDMETHODCALLTYPE *GetClientSite)(
        IOleObject* This,
        IOleClientSite** ppClientSite);
 
    HRESULT (STDMETHODCALLTYPE *SetHostNames)(
        IOleObject* This,
        LPCOLESTR szContainerApp,
        LPCOLESTR szContainerObj);
 
    HRESULT (STDMETHODCALLTYPE *Close)(
        IOleObject* This,
        DWORD dwSaveOption);
 
    HRESULT (STDMETHODCALLTYPE *SetMoniker)(
        IOleObject* This,
        DWORD dwWhichMoniker,
        IMoniker* pmk);
 
    HRESULT (STDMETHODCALLTYPE *GetMoniker)(
        IOleObject* This,
        DWORD dwAssign,
        DWORD dwWhichMoniker,
        IMoniker** ppmk);
 
    HRESULT (STDMETHODCALLTYPE *InitFromData)(
        IOleObject* This,
        IDataObject* pDataObject,
        BOOL fCreation,
        DWORD dwReserved);
 
    HRESULT (STDMETHODCALLTYPE *GetClipboardData)(
        IOleObject* This,
        DWORD dwReserved,
        IDataObject** ppDataObject);
 
    HRESULT (STDMETHODCALLTYPE *DoVerb)(
        IOleObject* This,
        LONG iVerb,
        LPMSG lpmsg,
        IOleClientSite* pActiveSite,
        LONG lindex,
        HWND hwndParent,
        LPCRECT lprcPosRect);
 
    HRESULT (STDMETHODCALLTYPE *EnumVerbs)(
        IOleObject* This,
        IEnumOLEVERB** ppEnumOleVerb);
 
    HRESULT (STDMETHODCALLTYPE *Update)(
        IOleObject* This);
 
    HRESULT (STDMETHODCALLTYPE *IsUpToDate)(
        IOleObject* This);
 
    HRESULT (STDMETHODCALLTYPE *GetUserClassID)(
        IOleObject* This,
        CLSID* pClsid);
 
    HRESULT (STDMETHODCALLTYPE *GetUserType)(
        IOleObject* This,
        DWORD dwFormOfType,
        LPOLESTR* pszUserType);
 
    HRESULT (STDMETHODCALLTYPE *SetExtent)(
        IOleObject* This,
        DWORD dwDrawAspect,
        SIZEL* psizel);
 
    HRESULT (STDMETHODCALLTYPE *GetExtent)(
        IOleObject* This,
        DWORD dwDrawAspect,
        SIZEL* psizel);
 
    HRESULT (STDMETHODCALLTYPE *Advise)(
        IOleObject* This,
        IAdviseSink* pAdvSink,
        DWORD* pdwConnection);
 
    HRESULT (STDMETHODCALLTYPE *Unadvise)(
        IOleObject* This,
        DWORD dwConnection);
 
    HRESULT (STDMETHODCALLTYPE *EnumAdvise)(
        IOleObject* This,
        IEnumSTATDATA** ppenumAdvise);
 
    HRESULT (STDMETHODCALLTYPE *GetMiscStatus)(
        IOleObject* This,
        DWORD dwAspect,
        DWORD* pdwStatus);
 
    HRESULT (STDMETHODCALLTYPE *SetColorScheme)(
        IOleObject* This,
        LOGPALETTE* pLogpal);
 
} IOleObjectVtbl2;
typedef struct IOleObject2 {
    const IOleObjectVtbl* lpVtbl;
} IOleObject2;
#define IOleObject IOleObject2
#define IOleObjectVtbl IOleObjectVtbl2
typedef struct IOleClientSiteVtbl2 {
    // IUnknown 方法
    HRESULT (STDMETHODCALLTYPE *QueryInterface)(
        IOleClientSite* This,
        REFIID riid,
        void** ppvObject);
 
    ULONG (STDMETHODCALLTYPE *AddRef)(
        IOleClientSite* This);
 
    ULONG (STDMETHODCALLTYPE *Release)(
        IOleClientSite* This);
 
    // IOleClientSite 特有方法
    HRESULT (STDMETHODCALLTYPE *SaveObject)(
        IOleClientSite* This);
 
    HRESULT (STDMETHODCALLTYPE *GetMoniker)(
        IOleClientSite* This,
        DWORD dwAssign,
        DWORD dwWhichMoniker,
        IMoniker** ppmk);
 
    HRESULT (STDMETHODCALLTYPE *GetContainer)(
        IOleClientSite* This,
        IOleContainer** ppContainer);
 
    HRESULT (STDMETHODCALLTYPE *ShowObject)(
        IOleClientSite* This);
 
    HRESULT (STDMETHODCALLTYPE *OnShowWindow)(
        IOleClientSite* This,
        BOOL fShow);
 
    HRESULT (STDMETHODCALLTYPE *RequestNewObjectLayout)(
        IOleClientSite* This);
 
} IOleClientSiteVtbl2;
 
typedef struct IOleClientSite2 {
    const IOleClientSiteVtbl2* lpVtbl;
} IOleClientSite2;
#define IOleClientSiteVtbl IOleClientSiteVtbl2
#define IOleClientSite IOleClientSite2*/
enum MESSAGETYPE {
	MT_PLAINTEXT = 0,
	MT_FILE,
	MT_SYSTEMNOTIFICATION
};
typedef struct tagNEWLVTILEINFO {//From Microsoft Learn
    UINT  cbSize;
    int   iItem;
    UINT  cColumns;
    PUINT puColumns;
    int   *piColFmt;
} NEWLVTILEINFO, *PNEWLVTILEINFO;
char Name[512], InvitationCode[256], ReceiveData[32767], Message[32767], IP[256], Port[8], JsonConfigFile[32767 * 16], AliveInLast5Minute = 0, InFocus, ThisDirectory[32767], LastThisDirectory[32767];
int ChatRoomsCount, ChatRoom_LastChoose = -1, PeopleCount, UseDarkMode = 0, ShowToast = FALSE;
unsigned int UsersCount = 0;
HWND hWndMain, hWndNotify;
HIMAGELIST hImageList;
NOTIFYICONDATAA nid;
pthread_mutex_t Lock;
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
HHOOK hFocus;
SOCKET sockfd;
int FileHandleCount = 0, MemoryPointerCount = 0;
FILE *__cdecl fopen2(const char * __restrict__ _Filename,const char * __restrict__ _Mode, const char* File, int Line) {
	getcwd(LastThisDirectory);
	chdir(ThisDirectory);
	FILE* lpFile = fopen(_Filename, _Mode);
	if(lpFile != NULL) {
		errno = 0;
		FileHandleCount += 1;
		printf("[File Manager/INFO] Opened File: %p(OF: %s, Mode: %s, File: %s, Line: %d, File Handle Count: %d)\n", lpFile, _Filename, _Mode, File, Line, FileHandleCount);
	}
	else {
		printf("[File Manager/ERROR] Opened File failed! (OF: %s, Mode: %s, File: %s, Line: %d, File Handle Count: %d)\n", _Filename, _Mode, File, Line, FileHandleCount);
		printf("[File Manager/ERROR] Code: %s(%d)\n", strerror(errno), errno);
	}
	chdir(LastThisDirectory);
	return lpFile;
}
int __cdecl fclose2(FILE *_File, const char* File, int Line) {
	int iResult = fclose(_File);
	if(_File != NULL) {
		FileHandleCount -= 1;
		printf("[File Mangaer/INFO] Closed File: %p(File: %s, Line: %d, File Handle Count: %d)\n", _File, File, Line, FileHandleCount);
	}
	else printf("[File Manager/ERROR] Closed file failed: file handle is empty! (File: %s, Line: %d, File Handle Count: %d)\n", File, Line, FileHandleCount);
	return iResult;
}
void *__cdecl calloc2(size_t _NumOfElements, size_t _SizeOfElements, const char* File, int Line) {
	void *Pointer = calloc(_NumOfElements, _SizeOfElements);
	if(Pointer != NULL) MemoryPointerCount += 1;
	printf("[Memory Manager/INFO] Alloced memory %p(Size: %d * %d, File: %s, Line: %d, Count: %d)\n", Pointer, _NumOfElements, _SizeOfElements, File, Line, MemoryPointerCount);
	return Pointer;
}
void __cdecl free2(void *_Memory, const char* File, int Line) {
	if(_Memory == NULL) {
		printf("[Memory Manager/ERROR] Freed error! Empty pointer! (File: %s, Line: %d, Count: %d)\n", File, Line, MemoryPointerCount);
	}
	free(_Memory);
	MemoryPointerCount -= 1;
	printf("[Memory Manager/INFO] Freed memory (File: %s, Line: %d, Pointer: %p, Count: %d)\n", File, Line, _Memory, MemoryPointerCount);
	return ;
}
#define fopen(a, b) fopen2(a, b, __FILE__, __LINE__)
#define fclose(a) fclose2(a, __FILE__, __LINE__)
#define calloc(a, b) calloc2(a, b, __FILE__, __LINE__)
#define free(a) free2(a, __FILE__, __LINE__)
#include "..\Json.h"
BOOL ShowToastMessage(DWORD dwIcon, const char *Title, const char *Details, BOOL bAlwaysShow) {
	if(ShowToast == 0) return TRUE;
	if(bAlwaysShow == 0 && InFocus == 1) return TRUE;
	NOTIFYICONDATAA nid2;
	memset(&nid2, 0, sizeof(nid2));
	nid2.cbSize = sizeof(nid);
	nid2.uFlags = NIF_INFO | NIF_GUID;
	nid2.guidItem = nid.guidItem;
	nid2.dwInfoFlags = dwIcon | NIIF_RESPECT_QUIET_TIME;
	nid2.hIcon = nid.hIcon;
	strcpy(nid2.szInfoTitle, Title);
	strncpy(nid2.szInfo, Details, sizeof(nid2.szInfo) - 1);
	if(bAlwaysShow == 0) {
		if(strlen(Details) > 50) {
			int i = 0;
			if(nid2.szInfo[48] < 0) {
				i = 47;
				while(i >= 0 && nid2.szInfo[i] < 0) i -= 1;
				i = 48 - i;
			}
			if(i % 2 == 1) {
				nid2.szInfo[50] = nid2.szInfo[49] = nid2.szInfo[48] = '.';
				nid2.szInfo[51] = '\0';
			}
			else {
				nid2.szInfo[49] = nid2.szInfo[48] = nid2.szInfo[47] = '.';
				nid2.szInfo[50] = '\0';
			}
		}
	}
	return Shell_NotifyIconA(NIM_MODIFY, &nid2);
}
LRESULT CALLBACK FocusProc(int nCode, WPARAM wParam, LPARAM lParam) {
	if(nCode == HC_ACTION) {
		LPCWPSTRUCT lpcs = (LPCWPSTRUCT)lParam;
		if(lpcs -> message == WM_KILLFOCUS && (GetParent(lpcs -> hwnd) == hWndMain || lpcs -> hwnd == hWndMain)) InFocus = 0;
		if(lpcs -> message == WM_SETFOCUS) InFocus = 1;
		if(lpcs -> message == WM_KILLFOCUS && (GetParent(lpcs -> hwnd) == hWndMain || lpcs -> hwnd == hWndMain)) printf("Msg: WM_KILLFOCUS, MainWindow = %p, Window = %p, Parent = %p\n", hWndMain, lpcs -> hwnd, GetParent(lpcs -> hwnd));
		if(lpcs -> message == WM_SETFOCUS) printf("Msg: WM_SETFOCUS, MainWindow = %p, Window = %p, Parent = %p\n", hWndMain, lpcs -> hwnd, GetParent(lpcs -> hwnd));
	}
	return CallNextHookEx(hFocus, nCode, wParam, lParam);
}
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
    free(ShowInBitmap);
    return hBitmap;
}
void* RecvMessageThread(void* lParam) {
	memset(ReceiveData, 0, sizeof(ReceiveData));
	while(recv(sockfd, ReceiveData, 1, 0) > 0) {
		int LastLVCount = ListView_GetItemCount(GetDlgItem(hWndMain, 2));
		switch(ReceiveData[0]) {
			case '\x9': {
				//Empty, testing is the server keeping alive
				break;
			}
			case '\xA': {
				char Str[114514];
				time_t Time;
				recv(sockfd, (char*)&Time, sizeof(Time), 0);
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
				lvi.mask = LVIF_TEXT | LVIF_PARAM;
				lvi.lParam = MT_SYSTEMNOTIFICATION;
				lvi.iImage = -1;
				lvi.iSubItem = 0;
				lvi.iItem = ListView_GetItemCount(GetDlgItem(hWndMain, 2));
				ListView_InsertItem(GetDlgItem(hWndMain, 2), &lvi);
				struct tm* TimeStruct = localtime(&Time);
				int iStart = strlen(lvi.pszText) + 1;
				strftime(lvi.pszText + iStart, 114514, "%Y/%m/%d %H:%M:%S", TimeStruct);
				ListView_SetItemText(GetDlgItem(hWndMain, 2), lvi.iItem, 1, lvi.pszText + iStart);
				NEWLVTILEINFO lti;
				memset(&lti, 0, sizeof(lti));
	            lti.puColumns = (LPUINT)calloc(5, sizeof(UINT));
	            lti.puColumns[0] = 1;
	            lti.puColumns[1] = 2;
		        lti.piColFmt = (LPINT)calloc(5, sizeof(INT));
				lti.cbSize = sizeof(lti);
				lti.cColumns = 2;
				lti.iItem = lvi.iItem;
				ListView_SetTileInfo(GetDlgItem(hWndMain, 2), &lti);
				free(lti.puColumns);
				free(lti.piColFmt);
		    	lvi.pszText = Str;
		    	lvi.mask = LVIF_TEXT;
		    	int k;
				lvi.cchTextMax = 114514;
				Str[iStart - 1] = '\n';
				ShowToastMessage(NIIF_NONE, "Message", Str, 1);
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
				time_t Time;
				recv(sockfd, (char*)&Time, sizeof(Time), 0);
				recv(sockfd, ReceiveData, 1, 0);
				recv(sockfd, ReceiveData, ReceiveData[0], 0);
				sprintf(Str, "%s left the chat room", ReceiveData);
				PeopleCount -= 1;
				LVITEMA lvi;
				lvi.pszText = Str;
				lvi.mask = LVIF_TEXT | LVIF_PARAM;
				lvi.lParam = MT_SYSTEMNOTIFICATION;
				lvi.iImage = -1;
				lvi.iSubItem = 0;
				lvi.iItem = ListView_GetItemCount(GetDlgItem(hWndMain, 2));
				ListView_InsertItem(GetDlgItem(hWndMain, 2), &lvi);
				struct tm* TimeStruct = localtime(&Time);
				int iStart = strlen(lvi.pszText) + 1;
				strftime(lvi.pszText + iStart, 114514, "%Y/%m/%d %H:%M:%S", TimeStruct);
				ListView_SetItemText(GetDlgItem(hWndMain, 2), lvi.iItem, 1, lvi.pszText + iStart);
				NEWLVTILEINFO lti;
				memset(&lti, 0, sizeof(lti));
	            lti.puColumns = (LPUINT)calloc(5, sizeof(UINT));
	            lti.puColumns[0] = 1;
	            lti.puColumns[1] = 2;
		        lti.piColFmt = (LPINT)calloc(5, sizeof(INT));
				lti.cbSize = sizeof(lti);
				lti.cColumns = 2;
				lti.iItem = lvi.iItem;
				ListView_SetTileInfo(GetDlgItem(hWndMain, 2), &lti);
				free(lti.puColumns);
				free(lti.piColFmt);
		    	lvi.pszText = Str;
		    	lvi.mask = LVIF_TEXT;
		    	int k;
				lvi.cchTextMax = 114514;
				Str[iStart - 1] = '\n';
				ShowToastMessage(NIIF_NONE, "Message", Str, 1);
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
				ShowToastMessage(NIIF_INFO, "You has kicked out by server!", "Connection closed by server", 1);
				MessageBox(NULL, "You has kicked out by server!", "Connection closed by server", MB_ICONINFORMATION);
				SendMessage(hWndNotify, WM_COMMAND, 10, 0);
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
			    	if(Last == Head) free(Last);
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
			case '\xE': {//File Notification
				char UserName[512], FileName[512];
				memset(UserName, 0, sizeof(UserName));
				memset(FileName, 0, sizeof(FileName));
				//unsigned int FileSize;
				fpos_t FileSize;
				unsigned char UserNameLength, FileNameLength;
				time_t Time;
				recv(sockfd, (char*)&Time, sizeof(Time), 0);
				recv(sockfd, (char*)&UserNameLength, sizeof(UserNameLength), 0);
				recv(sockfd, UserName, UserNameLength, 0);
				recv(sockfd, (char*)&FileNameLength, sizeof(FileNameLength), 0);
				recv(sockfd, FileName, FileNameLength, 0);
				recv(sockfd, (char*)&FileSize, sizeof(FileSize), 0);
				LVITEMA lvi;
				int i;
				if(strcmp(UserName, "Server") == 0) i = 0;
				else {
					for(i = 0; i < UsersCount; i++) {
						if(strcmp(Users[i].Name, UserName) == 0) break;
					}
					if(i == UsersCount) i = -1;
					i += 2;
				}
				lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
				lvi.lParam = MT_FILE;
				lvi.iSubItem = 0;
				lvi.iItem = ListView_GetItemCount(GetDlgItem(hWndMain, 2));
				lvi.iImage = i;
				lvi.pszText = (char*)calloc(512, sizeof(char));
				sprintf(lvi.pszText, "[File] %s", FileName);
				ListView_InsertItem(GetDlgItem(hWndMain, 2), &lvi);
				struct tm* TimeStruct = localtime(&Time);
				sprintf(lvi.pszText, "%s sent a file (", UserName);
				strftime(lvi.pszText + strlen(lvi.pszText), 1145, "%Y/%m/%d %H:%M:%S", TimeStruct);
				lvi.pszText[strlen(lvi.pszText)] = ')';
				if(IsDlgButtonChecked(hWndMain, 11) == BST_CHECKED) ShowToastMessage(NIIF_NONE, "New file", lvi.pszText, 0);
				lvi.iSubItem = 1;
				ListView_SetItemText(GetDlgItem(hWndMain, 2), lvi.iItem, 1, lvi.pszText);
				if(FileSize < 1024.0) {
					sprintf(lvi.pszText, "%lld Bytes", FileSize);
				}
				else if(FileSize * 1.0 / 1024.0 < 1024.0) {
					sprintf(lvi.pszText, "%.2lf Kib", FileSize * 1.0 / 1024.0);
				}
				else if(FileSize * 1.0 / 1024.0 / 1024.0 < 1024.0) {
					sprintf(lvi.pszText, "%.2lf Mib", FileSize * 1.0 / 1024.0 / 1024.0);
				}
				else if(FileSize * 1.0 / 1024.0 / 1024.0 / 1024.0 < 1024.0) {
					sprintf(lvi.pszText, "%.2lf Gib", FileSize * 1.0 / 1024.0 / 1024.0 / 1024.0);
				}
				else if(FileSize * 1.0 / 1024.0 / 1024.0 / 1024.0 / 1024.0 < 1024.0) {
					sprintf(lvi.pszText, "%.2lf Tib", FileSize * 1.0 / 1024.0 / 1024.0 / 1024.0 / 1024.0);
				}
				else if(FileSize * 1.0 / 1024.0 / 1024.0 / 1024.0 / 1024.0 / 1024.0 < 1024.0) {
					sprintf(lvi.pszText, "%.2lf Pib", FileSize * 1.0 / 1024.0 / 1024.0 / 1024.0 / 1024.0 / 1024.0);
				}
				lvi.iSubItem = 2;
				ListView_SetItemText(GetDlgItem(hWndMain, 2), lvi.iItem, 2, lvi.pszText);
				free(lvi.pszText);
				NEWLVTILEINFO lti;
				memset(&lti, 0, sizeof(lti));
	            lti.puColumns = (LPUINT)calloc(5, sizeof(UINT));
	            lti.puColumns[0] = 1;
	            lti.puColumns[1] = 2;
	            lti.puColumns[2] = 3;
		        lti.piColFmt = (LPINT)calloc(5, sizeof(INT));
				lti.cbSize = sizeof(lti);
				lti.cColumns = 3;
				lti.iItem = lvi.iItem;
				ListView_SetTileInfo(GetDlgItem(hWndMain, 2), &lti);
				free(lti.puColumns);
				free(lti.piColFmt);
				break;
			}
			case '\xF': {//Message Notification
				unsigned int iLen, iCount = 0;
				time_t Time;
				recv(sockfd, (char*)&Time, sizeof(Time), 0);
				recv(sockfd, ReceiveData, 1, 0);
				recv(sockfd, ReceiveData, ReceiveData[0], 0);
				recv(sockfd, (char*)&iLen, sizeof(iLen), 0);
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
				while(1) {
					memset(ReceiveData, 0, sizeof(ReceiveData));
					iReceived = recv(sockfd, ReceiveData, (iLen - iCount > sizeof(ReceiveData)) ? sizeof(ReceiveData) : (iLen - iCount), 0);
					memcpy(lvi.pszText + idx, ReceiveData, iReceived);
					idx += iReceived;
					iCount += iReceived;
					if(iCount >= iLen) break;
				}
				//printf("%s", ReceiveData);
				lvi.mask = LVIF_TEXT | LVIF_IMAGE;
				lvi.iSubItem = 0;
				lvi.iItem = ListView_GetItemCount(GetDlgItem(hWndMain, 2));
				lvi.iImage = i;
				char *ChatHistoryName = (char*)calloc(32767, sizeof(char));
				sprintf(ChatHistoryName, "ChatMessages\\Message-%d.msg", ListView_GetItemCount(GetDlgItem(hWndMain, 2)));
				printf("New Message Saved Path: '%s'\n", ChatHistoryName);
				FILE* lpMsgFile = fopen(ChatHistoryName, "wb");
				for(i = 0; i < strlen(lvi.pszText); i += 1) {
					if(lvi.pszText[i] == '\n') {
						fwrite("\r\n", 1, 2, lpMsgFile);
					}
					else {
						char *iRes = strchr(lvi.pszText + i, '\n');
						int idx;
						if(iRes == NULL) idx = strlen(lvi.pszText + i);
						else idx = iRes - lvi.pszText - i;
						fwrite(lvi.pszText + i, sizeof(char), idx, lpMsgFile);
						i += idx - 1;
					}
				}
				fclose(lpMsgFile);
				free(ChatHistoryName);
				//printf("%s\n", lvi.pszText);
				//if(IsDlgButtonChecked(hWndMain, 11) == BST_CHECKED) ShowToastMessage(NIIF_NONE, "New message", lvi.pszText, 0);
				BOOL $Value = FALSE;
				for(i = 0; i < strlen(lvi.pszText); i += 1) {
					if(lvi.pszText[i] == '@') {
						char* Position = strchr(lvi.pszText + 1 + i, ' ');
						if(Position > lvi.pszText + i && strncmp(lvi.pszText + i + 1, Name, Position - lvi.pszText - i - 1) == 0) {
							ShowToastMessage(NIIF_NONE, "Mentioned You", lvi.pszText, 1);
						}
					}
				}
				if(InFocus == FALSE) FlashWindow(hWndMain, FALSE);
				ListView_InsertItem(GetDlgItem(hWndMain, 2), &lvi);
				struct tm* TimeStruct = localtime(&Time);
				strftime(lvi.pszText, 114514, "%Y/%m/%d %H:%M:%S", TimeStruct);
				ListView_SetItemText(GetDlgItem(hWndMain, 2), lvi.iItem, 1, lvi.pszText);
				NEWLVTILEINFO lti;
				memset(&lti, 0, sizeof(lti));
	            lti.puColumns = (LPUINT)calloc(5, sizeof(UINT));
	            lti.puColumns[0] = 1;
	            lti.puColumns[1] = 2;
		        lti.piColFmt = (LPINT)calloc(5, sizeof(INT));
				lti.cbSize = sizeof(lti);
				lti.cColumns = 2;
				lti.iItem = lvi.iItem;
				ListView_SetTileInfo(GetDlgItem(hWndMain, 2), &lti);
				free(lti.puColumns);
				free(lti.piColFmt);
				free(lvi.pszText);
				break;
			}
			case '\x10': {//Be A Silencer
				time_t Time;
				int Length = 0, LenTips;
				char SilencerUserName[32767];
				memset(SilencerUserName, 0, sizeof(SilencerUserName));
				sprintf(SilencerUserName, "User ");
				LenTips = strlen(SilencerUserName);
				recv(sockfd, (char*)&Time, sizeof(Time), 0);
				recv(sockfd, (char*)&Length, sizeof(char), 0);
				recv(sockfd, SilencerUserName + LenTips, Length, 0);
				if(strcmp(SilencerUserName + LenTips, Name) == 0) {
					EnableWindow(GetDlgItem(hWndMain, 3), FALSE);
					EnableWindow(GetDlgItem(hWndMain, 9), FALSE);
					EnableWindow(GetDlgItem(hWndMain, 12), FALSE);
				}
				strcat(SilencerUserName, " has been banned from speaking");
				LVITEMA lvi;
				lvi.pszText = SilencerUserName;
				lvi.mask = LVIF_TEXT | LVIF_PARAM;
				lvi.lParam = MT_SYSTEMNOTIFICATION;
				lvi.iImage = -1;
				lvi.iSubItem = 0;
				lvi.iItem = ListView_GetItemCount(GetDlgItem(hWndMain, 2));
				ListView_InsertItem(GetDlgItem(hWndMain, 2), &lvi);
				lvi.pszText = (char*)calloc(114514, sizeof(char));
				struct tm* TimeStruct = localtime(&Time);
				strftime(lvi.pszText, 114514, "%Y/%m/%d %H:%M:%S", TimeStruct);
				ListView_SetItemText(GetDlgItem(hWndMain, 2), lvi.iItem, 1, lvi.pszText);
				NEWLVTILEINFO lti;
				memset(&lti, 0, sizeof(lti));
	            lti.puColumns = (LPUINT)calloc(5, sizeof(UINT));
	            lti.puColumns[0] = 1;
	            lti.puColumns[1] = 2;
		        lti.piColFmt = (LPINT)calloc(5, sizeof(INT));
				lti.cbSize = sizeof(lti);
				lti.cColumns = 2;
				lti.iItem = lvi.iItem;
				ListView_SetTileInfo(GetDlgItem(hWndMain, 2), &lti);
				free(lti.puColumns);
				free(lti.piColFmt);
				free(lvi.pszText);
				break;
			}
			case '\x11': {//Remove from Silencers List
				time_t Time;
				int Length = 0, LenTips;
				char SilencerUserName[32767];
				memset(SilencerUserName, 0, sizeof(SilencerUserName));
				sprintf(SilencerUserName, "User ");
				LenTips = strlen(SilencerUserName);
				recv(sockfd, (char*)&Time, sizeof(Time), 0);
				recv(sockfd, (char*)&Length, sizeof(char), 0);
				recv(sockfd, SilencerUserName + LenTips, Length, 0);
				if(strcmp(SilencerUserName + LenTips, Name) == 0) {
					EnableWindow(GetDlgItem(hWndMain, 3), TRUE);
					EnableWindow(GetDlgItem(hWndMain, 9), TRUE);
					EnableWindow(GetDlgItem(hWndMain, 12), TRUE);
				}
				strcat(SilencerUserName, " has been unbanned from speaking");
				LVITEMA lvi;
				lvi.pszText = SilencerUserName;
				lvi.mask = LVIF_TEXT | LVIF_PARAM;
				lvi.lParam = MT_SYSTEMNOTIFICATION;
				lvi.iImage = -1;
				lvi.iSubItem = 0;
				lvi.iItem = ListView_GetItemCount(GetDlgItem(hWndMain, 2));
				ListView_InsertItem(GetDlgItem(hWndMain, 2), &lvi);
				lvi.pszText = (char*)calloc(114514, sizeof(char));
				struct tm* TimeStruct = localtime(&Time);
				strftime(lvi.pszText, 114514, "%Y/%m/%d %H:%M:%S", TimeStruct);
				ListView_SetItemText(GetDlgItem(hWndMain, 2), lvi.iItem, 1, lvi.pszText);
				NEWLVTILEINFO lti;
				memset(&lti, 0, sizeof(lti));
	            lti.puColumns = (LPUINT)calloc(5, sizeof(UINT));
	            lti.puColumns[0] = 1;
	            lti.puColumns[1] = 2;
		        lti.piColFmt = (LPINT)calloc(5, sizeof(INT));
				lti.cbSize = sizeof(lti);
				lti.cColumns = 2;
				lti.iItem = lvi.iItem;
				ListView_SetTileInfo(GetDlgItem(hWndMain, 2), &lti);
				free(lti.puColumns);
				free(lti.piColFmt);
				free(lvi.pszText);
				break;
			}
			case '\xFF': {//All chat records have been sent
				ShowToast = TRUE;
				break;
			}
		}
		AliveInLast5Minute = 1;
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
		SendMessage(hWndNotify, WM_COMMAND, 10, 0);
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
void *UploadFile(void *lParam) {
	char *FileName = (char*)calloc(32767, sizeof(char)), *Details = (char*)calloc(32767, sizeof(char)), *ReadData = (char*)calloc(32767, sizeof(char)), ErrMsg[32767], NameLength, iReceive;
	strcpy(FileName, (char*)lParam);
	sprintf(Details, "File '%s' is uploading...", FileName);
	ShowToastMessage(NIIF_INFO, "File Uploading is In Progress", Details, 1);
	FILE* lpFile = fopen(FileName, "rb");
	if(lpFile == NULL) {
		ShowToastMessage(NIIF_INFO, "File Uploaded failed!", strerror(errno), 1);
	    free(FileName);
	    free(Details);
	    free(ReadData);
	    return FALSE;
	}
	//unsigned int size;//Task: Convert to fpos_t type
	_off64_t size = 0;
	fseeko64(lpFile, 0, SEEK_END);
	//size = ftell(lpFile);
	size = ftello64(lpFile);
	//fgetpos(lpFile, &size);
	fseeko64(lpFile, 0, SEEK_SET);
	
	struct addrinfo hints, *res;
	int status;
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	if ((status = getaddrinfo(IP, Port, &hints, &res)) != 0) {
	    sprintf(ErrMsg, "Getaddrinfo Failed: %s(%d)", gai_strerror(status), status);
	    MessageBox(hWndMain, ErrMsg, "Uploaded file failed!", MB_ICONERROR);
	    free(FileName);
	    free(Details);
	    free(ReadData);
	    return FALSE;
	}
	SOCKET sockfd = socket(res -> ai_family, res -> ai_socktype, res -> ai_protocol);
	if(sockfd == INVALID_SOCKET) {
	    MessageBox(hWndMain, "Socket Failed!", "Uploaded file failed!", MB_ICONERROR);
	    free(FileName);
	    free(Details);
	    free(ReadData);
        return FALSE;
    }
    int iResult = connect(sockfd, res->ai_addr, res->ai_addrlen), iLen = strlen(Name);
    if(iResult == SOCKET_ERROR) {
	    MessageBox(hWndMain, "Connect Failed!", "Uploaded file failed!", MB_ICONERROR);
		closesocket(sockfd);
	    free(FileName);
	    free(Details);
	    free(ReadData);
    	return FALSE;
	}
    iResult = send(sockfd, "\xC", 1, 0);
	if(iResult == SOCKET_ERROR) {
	    MessageBox(hWndMain, "Send Failed!", "Uploaded file failed!", MB_ICONERROR);
		closesocket(sockfd);
	    free(FileName);
	    free(Details);
	    free(ReadData);
		return FALSE;
    }
    NameLength = strlen(Name);
    send(sockfd, (const char*)&NameLength, sizeof(NameLength), 0);
    send(sockfd, Name, NameLength, 0);
    recv(sockfd, &iReceive, sizeof(iReceive), 0);
    if(iReceive != 0) {
    	memset(Details, 0, 32767);
    	sprintf(Details, "Error Code: %d", iReceive);
    	recv(sockfd, Details + strlen(Details), 32767, 0);
    	MessageBox(hWndMain, Details, "Uploaded file failed!", MB_ICONWARNING);
    	closesocket(sockfd);
    	fclose(lpFile);
	    free(FileName);
	    free(Details);
	    free(ReadData);
    	return NULL;
	}
	int i = strlen(FileName) - 1;
	while(i >= 0 && FileName[i] != '\\') i -= 1;
	NameLength = strlen(FileName + i + 1);
    send(sockfd, (const char*)&NameLength, sizeof(NameLength), 0);
    send(sockfd, FileName + i + 1, NameLength, 0);
    recv(sockfd, &iReceive, sizeof(iReceive), 0);
    if(iReceive != 0) {
    	sprintf(Details, "File uploaded failed!(Error Code: %d)", iReceive);
    	MessageBox(hWndMain, Details, "Uploaded file failed!", MB_ICONWARNING);
    	closesocket(sockfd);
    	fclose(lpFile);
    	free(ReadData);
    	free(FileName);
    	free(Details);
    	return NULL;
	}
	send(sockfd, (const char*)&size, sizeof(size), 0);
	while((iResult = fread(ReadData, sizeof(char), 32767, lpFile)) > 0) {
		send(sockfd, ReadData, iResult, 0);
	}
	fclose(lpFile);
	sprintf(Details, "File '%s' was uploaded.", FileName);
	ShowToastMessage(NIIF_INFO, "File Uploaded successfully", Details, 1);
	free(ReadData);
	free(FileName);
	free(Details);
	return NULL;
}
void *DownloadingFileThread(void *lParam) {
	HWND hWndOwner = (HWND)lParam;
	char *FileName = (char*)calloc(32767, sizeof(char)), *DownloadPath = (char*)calloc(32767, sizeof(char)), *Details = (char*)calloc(32767, sizeof(char)), *ReadData = (char*)calloc(32767, sizeof(char)), ErrMsg[32767], NameLength, iReceive;
	strcpy(FileName, (char*)GetPropA(hWndOwner, "FileName"));
	free(GetPropA(hWndOwner, "FileName"));
	SetPropA(hWndOwner, "FileName", NULL);
	strcpy(DownloadPath, (char*)GetPropA(hWndOwner, "DownloadPath"));
	free(GetPropA(hWndOwner, "DownloadPath"));
	printf("DownloadPath: %s\nFileName: %s\n", DownloadPath, FileName);
	sprintf(Details, "File '%s' is downloading...", FileName);
	ShowToastMessage(NIIF_INFO, "File Downloading is In Progress", Details, 1);
	
	struct addrinfo hints, *res;
	int status;
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	if ((status = getaddrinfo(IP, Port, &hints, &res)) != 0) {
	    sprintf(ErrMsg, "Getaddrinfo Failed: %s(%d)", gai_strerror(status), status);
	    MessageBox(hWndMain, ErrMsg, "Downloaded file failed!", MB_ICONERROR);
	    free(FileName);
	    free(DownloadPath);
	    free(Details);
		free(ReadData);
	    return FALSE;
	}
	SOCKET sockfd2 = socket(res -> ai_family, res -> ai_socktype, res -> ai_protocol);
	if(sockfd == INVALID_SOCKET) {
	    MessageBox(hWndMain, "Socket Failed!", "Downloaded file failed!", MB_ICONERROR);
	    free(FileName);
	    free(DownloadPath);
	    free(Details);
		free(ReadData);
        return FALSE;
    }
    int iResult = connect(sockfd2, res->ai_addr, res->ai_addrlen);
    if(iResult == SOCKET_ERROR) {
	    MessageBox(hWndMain, "Connect Failed!", "Downloaded file failed!", MB_ICONERROR);
		closesocket(sockfd2);
	    free(FileName);
	    free(DownloadPath);
	    free(Details);
		free(ReadData);
    	return FALSE;
	}
    iResult = send(sockfd2, "\x10", 1, 0);
	if(iResult == SOCKET_ERROR) {
	    MessageBox(hWndMain, "Send Failed!", "Downloaded file failed!", MB_ICONERROR);
		closesocket(sockfd2);
	    free(FileName);
	    free(DownloadPath);
	    free(Details);
		free(ReadData);
		return FALSE;
    }
    NameLength = strlen(Name);
    send(sockfd2, (const char*)&NameLength, sizeof(NameLength), 0);
    send(sockfd2, Name, NameLength, 0);
    recv(sockfd2, &iReceive, 1, 0);
    if(iReceive != 0) {
    	memset(Details, 0, 32767);
    	sprintf(Details, "Error Code: %d", iReceive);
    	recv(sockfd2, Details + strlen(Details), 512, 0);
    	MessageBox(hWndMain, Details, "Downloaded file failed!", MB_ICONWARNING);
    	closesocket(sockfd2);
	    free(FileName);
	    free(DownloadPath);
	    free(Details);
		free(ReadData);
    	return NULL;
	}
	NameLength = strlen(FileName);
    send(sockfd2, (const char*)&NameLength, sizeof(NameLength), 0);
    send(sockfd2, FileName, NameLength, 0);
    iReceive = 1;
    recv(sockfd2, &iReceive, 1, 0);
    if(iReceive != 0) {
    	sprintf(Details, "File downloaded failed! Error Code: %d", iReceive);
    	recv(sockfd2, Details + strlen(Details), 512, 0);
    	MessageBox(hWndMain, Details, "Downloaded file failed!", MB_ICONWARNING);
    	closesocket(sockfd2);
	    free(FileName);
	    free(DownloadPath);
	    free(Details);
		free(ReadData);
    	return NULL;
	}
	FILE* lpFile = fopen(DownloadPath, "wb");
	//unsigned int Size = 0;
	_off64_t Size = 0;
	recv(sockfd2, (char*)&Size, sizeof(Size), 0);
	while((iResult = recv(sockfd2, ReadData, 32767, 0)) > 0) {
		fwrite(ReadData, sizeof(char), iResult, lpFile);
	}
	fclose(lpFile);
	sprintf(Details, "File '%s' was downloaded.", FileName);
	ShowToastMessage(NIIF_INFO, "File Downloaded successfully", Details, 1);
    free(FileName);
    free(DownloadPath);
    free(Details);
	free(ReadData);
	closesocket(sockfd2);
	return NULL;
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
    memset(ReceiveData, 0, sizeof(ReceiveData));
    recv(sockfd, ReceiveData, 1, 0);
    //RecvFull(sockfd, ReceiveData, sizeof(ReceiveData));
    if(ReceiveData[0] != '\x1') {
    	recv(sockfd, ReceiveData + 1, sizeof(ReceiveData) - 1, 0);
    	printf("Cannot connect to server: %s(%d)\n", ReceiveData + 2, ReceiveData[0]);
    	closesocket(sockfd);
    	sockfd = 0;
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
			CreateWindowExA((UseDarkMode == 0) * WS_EX_CLIENTEDGE, "EDIT", "Server Information:\r\nN/A", WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_MULTILINE | ES_READONLY | WS_HSCROLL | WS_VSCROLL | (UseDarkMode * WS_BORDER), 0, 0, 0, 0, hWnd, (HMENU)6, NULL, NULL);
			CreateWindowExA(0, "BUTTON", "Refresh", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 0, 0, 0, 0, hWnd, (HMENU)7, NULL, NULL);
			CreateWindowExA(0, "BUTTON", "Save", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 0, 0, 0, 0, hWnd, (HMENU)8, NULL, NULL);
			CreateWindowExA(0, "BUTTON", "Join In", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON | WS_TABSTOP, 0, 0, 0, 0, hWnd, (HMENU)9, NULL, NULL);
			CreateWindowExA(0, "BUTTON", "Cancel", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 0, 0, 0, 0, hWnd, (HMENU)10, NULL, NULL);
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
			for(i = 1; i <= 10; i++) {
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
				case 7: {
					char IP[32767], Port[32767], Message[65564], Recv, ChatRoomName[512];
					unsigned int UserLimit = 0, UserCount = 0;
					memset(IP, 0, sizeof(IP));
					memset(Port, 0, sizeof(Port));
					memset(ChatRoomName, 0, sizeof(ChatRoomName));
					GetDlgItemTextA(hWnd, 4, IP, sizeof(IP));
					GetDlgItemTextA(hWnd, 5, Port, sizeof(Port));
					struct addrinfo hints, *res;
					int stat;
					memset(&hints, 0, sizeof hints);
					hints.ai_family = AF_UNSPEC;
					hints.ai_socktype = SOCK_STREAM;
					if ((stat = getaddrinfo(IP, Port, &hints, &res)) != 0) {
					    sprintf(Message, "Getaddrinfo Failed: %s(%d)\n", gai_strerror(stat), stat);
					    MessageBox(hWnd, Message, "Query Information Failed!", MB_ICONERROR);
					    return FALSE;
					}
					SOCKET sockfd2 = socket(res -> ai_family, res -> ai_socktype, res -> ai_protocol);
					if(sockfd2 == INVALID_SOCKET) {
					    MessageBox(hWnd, "Socket Failed!", "Query Information Failed!", MB_ICONERROR);
				        return FALSE;
				    }
				    int iResult = connect(sockfd2, res -> ai_addr, res -> ai_addrlen), iLen = strlen(Name);
				    if(iResult == SOCKET_ERROR) {
					    MessageBox(hWnd, "Connect Failed!", "Query Information Failed!", MB_ICONERROR);
						closesocket(sockfd2);
				    	return FALSE;
					}
					send(sockfd2, "\xE", 1, 0);
					recv(sockfd2, &Recv, sizeof(Recv), 0);
					recv(sockfd2, (char*)&UserCount, sizeof(UserCount), 0);
					recv(sockfd2, (char*)&UserLimit, sizeof(UserLimit), 0);
					recv(sockfd2, &Recv, 1, 0);
					recv(sockfd2, ChatRoomName, Recv, 0);
					closesocket(sockfd2);
					sprintf(Message, "Server Information:\r\nUser: %u/%u\r\nName: %s", UserCount, UserLimit, ChatRoomName);
					SetDlgItemText(hWnd, 6, Message);
					break;
				}
				case 8: {
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
				case 9: {
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
					//printf("%s %d\n", Buffer, SendDlgItemMessage(hWnd, 2, CB_GETCURSEL, 0, 0));
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
				case 10: {
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
			SetWindowPos(GetDlgItem(hWnd, 6), NULL, 20, 270, (Rect.right - 40) * 0.5, 120, SWP_NOZORDER);
			SetWindowPos(GetDlgItem(hWnd, 7), NULL, (Rect.right - 40) * 0.5 + 40, 270, 150, 30, SWP_NOZORDER);
			SetWindowPos(GetDlgItem(hWnd, 8), NULL, Rect.right - 190, 270, 150, 30, SWP_NOZORDER);
			SetWindowPos(GetDlgItem(hWnd, 9), NULL, Rect.right - 360, Rect.bottom - 50, 150, 30, SWP_NOZORDER);
			SetWindowPos(GetDlgItem(hWnd, 10), NULL, Rect.right - 190, Rect.bottom - 50, 150, 30, SWP_NOZORDER);
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
			//SendDlgItemMessage(hWnd, 3, EM_SETCUEBANNER, TRUE, (LPARAM)L"Ctrl + Enter Line Break");
			CreateWindowExA(0, "STATIC", "User Name: ",
				WS_CHILD | WS_TABSTOP | WS_VISIBLE, 0, 0, 0, 0, hWnd, (HMENU)4, NULL, NULL);
			CreateWindowExA(0, "STATIC", "Server Name: ",
				WS_CHILD | WS_TABSTOP | WS_VISIBLE, 0, 0, 0, 0, hWnd, (HMENU)5, NULL, NULL);
			CreateWindowExA((UseDarkMode == 0) * WS_EX_CLIENTEDGE, "EDIT", Name,
				WS_CHILD | WS_TABSTOP | WS_VISIBLE | ES_AUTOHSCROLL | ES_READONLY | (UseDarkMode * WS_BORDER), 0, 0, 0, 0, hWnd, (HMENU)6, NULL, NULL);
			char Recv, ChatRoomName[512], Message__[65564];
			unsigned int UserLimit = 0, UserCount = 0;
			memset(ChatRoomName, 0, sizeof(ChatRoomName));
			struct addrinfo hints, *res;
			int stat;
			memset(&hints, 0, sizeof hints);
			hints.ai_family = AF_UNSPEC;
			hints.ai_socktype = SOCK_STREAM;
			if ((stat = getaddrinfo(IP, Port, &hints, &res)) != 0) {
			    sprintf(Message__, "Getaddrinfo Failed: %s(%d)\n", gai_strerror(stat), stat);
			    MessageBox(hWnd, Message__, "Query Information Failed!", MB_ICONERROR);
			    strcpy(ChatRoomName, "UNKNOWN");
			    //return FALSE;
			}
			else {
				SOCKET sockfd2 = socket(res -> ai_family, res -> ai_socktype, res -> ai_protocol);
				if(sockfd2 == INVALID_SOCKET) {
				    MessageBox(hWnd, "Socket Failed!", "Query Information Failed!", MB_ICONERROR);
				    strcpy(ChatRoomName, "UNKNOWN");
			        //return FALSE;
			    }
			    else {
				    int iResult = connect(sockfd2, res -> ai_addr, res -> ai_addrlen), iLen = strlen(Name);
				    if(iResult == SOCKET_ERROR) {
					    MessageBox(hWnd, "Connect Failed!", "Query Information Failed!", MB_ICONERROR);
						closesocket(sockfd2);
					    strcpy(ChatRoomName, "UNKNOWN");
				    	//return FALSE;
					}
					else {
						send(sockfd2, "\xE", 1, 0);
						recv(sockfd2, &Recv, sizeof(Recv), 0);
						recv(sockfd2, (char*)&UserCount, sizeof(UserCount), 0);
						recv(sockfd2, (char*)&UserLimit, sizeof(UserLimit), 0);
						recv(sockfd2, &Recv, 1, 0);
						recv(sockfd2, ChatRoomName, Recv, 0);
						closesocket(sockfd2);
					}
				}
			}
			CreateWindowExA((UseDarkMode == 0) * WS_EX_CLIENTEDGE, "EDIT", ChatRoomName,
				WS_CHILD | WS_TABSTOP | WS_VISIBLE | ES_AUTOHSCROLL | ES_READONLY | (UseDarkMode * WS_BORDER), 0, 0, 0, 0, hWnd, (HMENU)7, NULL, NULL);
			CreateWindowExA(0, WC_LISTVIEWA, "Users List",
				WS_CHILD | WS_TABSTOP | WS_VISIBLE | LVS_SINGLESEL, 0, 0, 0, 0, hWnd, (HMENU)8, NULL, NULL);
			CreateWindowExA(0, "BUTTON", "Send",
				WS_CHILD | WS_TABSTOP | WS_VISIBLE | BS_DEFPUSHBUTTON, 0, 0, 0, 0, hWnd, (HMENU)9, NULL, NULL);
			CreateWindowExA(0, "BUTTON", "Auto Scroll To Bottom",
				WS_CHILD | WS_TABSTOP | WS_VISIBLE | BS_AUTOCHECKBOX, 0, 0, 0, 0, hWnd, (HMENU)10, NULL, NULL);
			//CreateWindowExA(0, "BUTTON", "Always show notifications",
			//	WS_CHILD | WS_TABSTOP | WS_VISIBLE | BS_AUTOCHECKBOX, 0, 0, 0, 0, hWnd, (HMENU)11, NULL, NULL);
			CreateWindowExA(0, "BUTTON", "Send A File",
				WS_CHILD | WS_TABSTOP | WS_VISIBLE, 0, 0, 0, 0, hWnd, (HMENU)12, NULL, NULL);
			SendDlgItemMessage(hWnd, 3, EM_SETLIMITTEXT, 32766, 0);
			CheckDlgButton(hWnd, 10, BST_CHECKED);
			//CheckDlgButton(hWnd, 11, BST_CHECKED);
			ListView_SetView(GetDlgItem(hWnd, 2), LV_VIEW_TILE);
			LVCOLUMN lc;
			lc.mask = 0;
			ListView_InsertColumn(GetDlgItem(hWnd, 2), 1, &lc);
			ListView_InsertColumn(GetDlgItem(hWnd, 2), 2, &lc);
			ListView_InsertColumn(GetDlgItem(hWnd, 2), 3, &lc);
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
			for(i = 1; i <= 12; i++) {
				SendDlgItemMessage(hWnd, i, WM_SETFONT, (WPARAM)hFont, 0);
				SetWindowTheme(GetDlgItem(hWnd, i), (UseDarkMode == 0) ? L"Explorer" : L"DarkMode_Explorer", NULL);
			}
			SetTimer(hWnd, 1, 300000, NULL);
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
			SetWindowPos(GetDlgItem(hWnd, 11), NULL, (Rect.right - 110) * 0.7 + 40, (Rect.bottom - 130) * 0.6 + 160, (Rect.right - 110) * 0.3, 30, SWP_NOZORDER);
			SetWindowPos(GetDlgItem(hWnd, 12), NULL, (Rect.right - 110) * 0.7 + 40, (Rect.bottom - 130) * 0.6 + 190, (Rect.right - 110) * 0.1, 30, SWP_NOZORDER);
			LVTILEVIEWINFO tileViewInfo;
            tileViewInfo.cbSize = sizeof(LVTILEVIEWINFO);
            tileViewInfo.dwFlags = LVTVIF_FIXEDSIZE;
            tileViewInfo.dwMask = LVTVIM_COLUMNS | LVTVIM_TILESIZE;
            tileViewInfo.cLines = 3;
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
        			char *lpstrMessage = (char*)calloc(65534, sizeof(char)), Result = 0;
        			unsigned int iLen;
        			GetDlgItemTextA(hWnd, 3, lpstrMessage, 65534);
        			if(lpstrMessage[0] == '\0') {
        				free(lpstrMessage);
						break;
					}
        			iLen = strlen(lpstrMessage) + 1;
        			//pthread_mutex_lock(&Lock);
        			send(sockfd, "\xB", 1, 0);
        			/*recv(sockfd, &Result, 1, 0);
        			switch(Result) {
        				case 0x9: {
        					MessageBox(hWnd, "Error: You have been banned from speaking!", "Error", MB_ICONWARNING);
							break;
						}
						case 0x0: {*/
		        			send(sockfd, (const char*)&iLen, sizeof(iLen), 0);
		        			send(sockfd, lpstrMessage, iLen, 0);
		        			SetDlgItemTextA(hWnd, 3, "");
					/*		break;
						}
					}*/
					//pthread_mutex_unlock(&Lock);
        			free(lpstrMessage);
					break;
				}
				case 12: {
					char *FilePath = (char*)calloc(32767, sizeof(char));
					OPENFILENAMEA ofn;
					memset(&ofn, 0, sizeof(ofn));
					ofn.lStructSize = sizeof(ofn);
					ofn.lpstrFile = FilePath;
					ofn.nMaxFile = 32750;
					ofn.lpstrFilter = "All Files(*.*)\0*.*\0\0";
					ofn.hwndOwner = hWnd;
					if(GetOpenFileNameA(&ofn) == FALSE) {
						free(FilePath);
						break;
					}
					pthread_t ptid_t;
					pthread_create(&ptid_t, NULL, UploadFile, FilePath);
					Sleep(50);
					free(FilePath);
					break;
				}
			}
			break;
		}
		case WM_TIMER: {
			switch(wParam) {
				case 1: {
					if(AliveInLast5Minute == 0) {
						KillTimer(hWnd, 1);
						ShowToastMessage(NIIF_WARNING, "Error: The client and server have been disconnected.", "Connection Closed", 1);
						if(MessageBox(hWnd, "Error: The client and server have been disconnected.\nDo you want to stay in this page?", "Connection Closed", MB_ICONQUESTION | MB_YESNO) != IDYES) {
							SendMessage(hWndNotify, WM_COMMAND, 10, 0);
						}
						break;
					}
					AliveInLast5Minute = 0;
					if(send(sockfd, "\x9", 1, 0) < 0) {
						KillTimer(hWnd, 1);
						ShowToastMessage(NIIF_WARNING, "Error: The client and server have been disconnected.", "Connection Closed", 1);
						if(MessageBox(hWnd, "Error: The client and server have been disconnected.\nDo you want to stay in this page?", "Connection Closed", MB_ICONQUESTION | MB_YESNO) != IDYES) {
							SendMessage(hWndNotify, WM_COMMAND, 10, 0);
						}
					}
					break;
				}
			}
			//DrawState(hDC, NULL, NULL, (LPARAM)GetPropA(hwnd, "bitmap"), 0, 0, 0, 64, 64, DST_BITMAP);
			break;
		}
	    case WM_NOTIFY: {
			switch(((LPNMHDR)lParam) -> code) {
				case LVN_ITEMACTIVATE: {
					LPNMITEMACTIVATE lpnmia = (LPNMITEMACTIVATE)lParam;
					switch(lpnmia -> hdr.idFrom) {
						case 2: {
							int id = lpnmia -> iItem;
							LVITEMA lvi;
							lvi.mask = LVIF_PARAM | LVIF_TEXT;
							lvi.iItem = lpnmia -> iItem;
							lvi.iSubItem = 0;
							lvi.pszText = (char*)calloc(256, sizeof(char));
							lvi.cchTextMax = 256;
							ListView_GetItem(GetDlgItem(hWnd, 2), &lvi);
							switch(lvi.lParam) {
								case MT_PLAINTEXT: {
									//EnableWindow(hWnd, FALSE);
									//HWND hWndWeb = CreateWindowExA(0, "NChat-File-View", "NChat File Browser", WS_VISIBLE | WS_MINIMIZEBOX | WS_SYSMENU | WS_CAPTION | WS_MAXIMIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT, 845, 480, hWnd, NULL, NULL, NULL); 
									/*HMODULE hModule = LoadLibraryA("mshtml.dll");
									if(hModule != NULL) {
										FARPROC RunHTMLApplication = GetProcAddress(hModule, "RunHTMLApplication");
										if(RunHTMLApplication != NULL) {
											(HRESULT (*)(HINSTANCE,HINSTANCE,LPSTR,int))RunHTMLApplication(NULL, NULL, "https://boffice-excel.github.io/Website", SW_SHOW);
										}
										FreeLibrary(hModule);
									}*//*
									STARTUPINFOA si;memset(&si, 0, sizeof(si));
									PROCESS_INFORMATION pi;memset(&pi, 0, sizeof(pi));
									CreateProcessA(NULL, "mshta.exe .\MessageViewer.html", NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);*/
									char *Details = (char*)calloc(114514, sizeof(char));
									sprintf(Details, "ChatMessages\\Message-%d.msg", lvi.iItem);
									printf("%s\n",Details);
									FILE *lpFile = fopen(Details, "rb");
									if(lpFile == NULL) {
										free(Details);
										free(lvi.pszText);
										MessageBox(hWnd, "The chat log file has been deleted!", "Error", MB_ICONWARNING);
										return 0;
									}
									memset(Details, 0, 114514);
									fread(Details, 114514, sizeof(char), lpFile);
									fclose(lpFile);
									int i;
									for(i = 0; i < strlen(Details); i += 1) {
										if(Details[i] == '>' && Details[i + 1] == ' ') {
											i += 2;
											break;
										}
									}
									Details[i - 1] = '\0';
									Details[i - 2] = '\0';
									Details[0] = '\0';
									HWND hMsgWnd = CreateWindowExA(0, "NChat-Message-Viewer", "Message From User", WS_VISIBLE | WS_MINIMIZEBOX | WS_SYSMENU | WS_CAPTION, CW_USEDEFAULT, CW_USEDEFAULT, 845, 480, NULL, NULL, NULL, NULL);
									SetPropA(hMsgWnd, "Message", Details + i);
									SetPropA(hMsgWnd, "UserName", Details + 1);
									if(strcmp(Details + 1, "Server") == 0) SetPropA(hMsgWnd, "ProfilePicture", MakeBitmapWithName("SERVER"));
									else SetPropA(hMsgWnd, "ProfilePicture", MakeBitmapWithName(Details + 1));
									SetPropA(hMsgWnd, "hWndOwner", hWnd);
									SendMessage(hMsgWnd, WM_TIMER, 1, 0);
									free(Details);
									DeleteObject(GetPropA(hMsgWnd, "ProfilePicture"));
									//EnableWindow(hWnd, TRUE);
									break;
								}
								case MT_FILE: {
									EnableWindow(hWnd, FALSE);
									HWND hFileWnd = CreateWindowExA(0, "NChat-File-View", "Chat File", WS_VISIBLE | WS_MINIMIZEBOX | WS_SYSMENU | WS_CAPTION, CW_USEDEFAULT, CW_USEDEFAULT, 845, 480, hWnd, NULL, NULL, NULL);
									SetPropA(hFileWnd, "FileName", lvi.pszText + 7);
									SetPropA(hFileWnd, "hWndOwner", hWnd);
									SendMessage(hFileWnd, WM_TIMER, 1, 0);
									break;
								}
							}
							free(lvi.pszText);
							break;
						}
						case 8: {
							LVITEMA lvi;
							lvi.mask = LVIF_PARAM | LVIF_TEXT;
							lvi.iItem = lpnmia -> iItem;
							lvi.iSubItem = 0;
							lvi.pszText = (char*)calloc(32767, sizeof(char));
							lvi.cchTextMax = 32767;
							char *Text = (char*)calloc(114514, sizeof(char));
							ListView_GetItem(GetDlgItem(hWnd, 8), &lvi);
							GetDlgItemTextA(hWnd, 3, Text, 114514); 
							if(strlen(lvi.pszText) + strlen(Text) + 2 <= 32766) {
								sprintf(Text + strlen(Text), "@%s ", lvi.pszText);
								SetDlgItemText(hWnd, 3, Text);
							}
							free(lvi.pszText);
							free(Text);
							break;
						}
					}
					break;
				}
			}
			break;
		}
		case WM_CLOSE: {
			ShowWindow(hWnd, SW_HIDE);
			break;
		}
		default: return DefWindowProc(hWnd, Message, wParam, lParam);
	}
	return 0;
}
LRESULT CALLBACK NotifyProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam) {
	switch(Message) {
		case WM_DESTROY: {
			PostQuitMessage(0);
			break;
		}
		case WM_COMMAND: {
			switch(LOWORD(wParam)) {
				case 9: {
					SendMessage(hWnd, WM_APP + 255, 0, NIN_SELECT);
					break;
				}
				case 10: {
					DestroyWindow(hWnd);
					break;
				}
			}
			break;
		}
		case WM_APP + 255: {
			switch(LOWORD(lParam)) {
				case NIN_SELECT:
				case NIN_BALLOONUSERCLICK: {
					SwitchToThisWindow(hWndMain, FALSE);
					ShowWindow(hWndMain, SW_SHOW);
					break;
				}
				case WM_CONTEXTMENU: {
					SetForegroundWindow(hWnd);
					HMENU hMenu = CreatePopupMenu();
					AppendMenuA(hMenu, MF_STRING, 9, "&Switch To Chat Window");
					AppendMenuA(hMenu, MF_STRING, 10, "&Exit");
                	TrackPopupMenuEx(hMenu, TPM_RIGHTBUTTON | TPM_RIGHTALIGN, LOWORD(wParam), HIWORD(wParam), hWnd, NULL);
                	DestroyMenu(hMenu);
					break;
				}
			}
			break;
		}
		default: return DefWindowProc(hWnd, Message, wParam, lParam);
	}
	return 0;
}
LRESULT CALLBACK FileViewProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam) {
	switch(Message) {
		case WM_CREATE: {
			CreateWindowExA(0, "STATIC", NULL, WS_CHILD | WS_VISIBLE | SS_ICON | SS_CENTERIMAGE, 0, 0, 0, 0, hWnd, (HMENU)1, NULL, NULL);
			CreateWindowExA(0, "STATIC", NULL, WS_CHILD | WS_VISIBLE | SS_CENTER, 0, 0, 0, 0, hWnd, (HMENU)2, NULL, NULL);
			CreateWindowExA(0, "BUTTON", "&Download", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON | WS_TABSTOP | WS_GROUP, 0, 0, 0, 0, hWnd, (HMENU)3, NULL, NULL);
			CreateWindowExA(0, "BUTTON", "&Cancel", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 0, 0, 0, 0, hWnd, (HMENU)4, NULL, NULL);
			SendDlgItemMessage(hWnd, 1, STM_SETIMAGE, IMAGE_ICON, (LPARAM)ExtractIcon(GetModuleHandle(NULL), "imageres.dll", 2));
			int i;
			for(i = 1; i <= 4; i++) {
				SendDlgItemMessage(hWnd, i, WM_SETFONT, (WPARAM)hFont, 0);
				SetWindowTheme(GetDlgItem(hWnd, i), (UseDarkMode == 0) ? L"Explorer" : L"DarkMode_Explorer", NULL);
			}
			break;
		}
		case WM_TIMER: {
			switch(wParam) {
				case 1: {
					char *lpstrDetails = (char*)calloc(32767, sizeof(char)), *lpstrFileName = (char*)calloc(32767, sizeof(char));
					sprintf(lpstrDetails, "File: %s", (char*)GetPropA(hWnd, "FileName"));
					strcpy(lpstrFileName, (char*)GetPropA(hWnd, "FileName"));
					SetPropA(hWnd, "FileName", lpstrFileName);
					SetDlgItemText(hWnd, 2, lpstrDetails);
					free(lpstrDetails);
					break;
				}
			}
			break;
		}
		case WM_COMMAND: {
			switch(LOWORD(wParam)) {
				case 3: {
					OPENFILENAMEA ofn;
					memset(&ofn, 0, sizeof(ofn));
					ofn.lStructSize = sizeof(ofn);
					ofn.lpstrFile = (LPSTR)calloc(32767, sizeof(char));
					ofn.nMaxFile = 32767;
					ofn.lpstrFilter = "All Files(*.*)\0*.*\0\0";
					ofn.nFilterIndex = 1;
					ofn.hwndOwner = hWnd;
					ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_OVERWRITEPROMPT;
					//strcpy(ofn.lpstrFile, GetPropA(hWnd, "FileName"));
					ofn.lpstrTitle = GetPropA(hWnd, "FileName");
					SetPropA(hWnd, "DownloadPath", ofn.lpstrFile);
					if(GetSaveFileNameA(&ofn) == FALSE) {
						free(ofn.lpstrFile);
						break;
					}
					pthread_t ptid_t;
					pthread_create(&ptid_t, NULL, DownloadingFileThread, (void*)hWnd);
					Sleep(200);
					//free(ofn.lpstrFile);
					DestroyWindow(hWnd);
					break;
				}
				case 4: {
					DestroyWindow(hWnd);
					break;
				}
			}
			break;
		}
		case WM_SIZE: {
			RECT Rect;
			GetClientRect(hWnd, &Rect);
			SetWindowPos(GetDlgItem(hWnd, 1), NULL, Rect.right * 0.4, Rect.bottom * 0.1, Rect.right * 0.2, Rect.right * 0.2, SWP_NOZORDER);
			SetWindowPos(GetDlgItem(hWnd, 2), NULL, Rect.right * 0.1, Rect.bottom * 0.1 + Rect.right * 0.2 + 20, Rect.right * 0.8, 20, SWP_NOZORDER);
			SetWindowPos(GetDlgItem(hWnd, 3), NULL, Rect.right * 0.4 - 10, Rect.bottom * 0.1 + Rect.right * 0.2 + 60, Rect.right * 0.1, 30, SWP_NOZORDER);
			SetWindowPos(GetDlgItem(hWnd, 4), NULL, Rect.right * 0.5 + 10, Rect.bottom * 0.1 + Rect.right * 0.2 + 60, Rect.right * 0.1, 30, SWP_NOZORDER);
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
		case WM_DESTROY: {
			EnableWindow(GetPropA(hWnd, "hWndOwner"), TRUE);
			if(GetPropA(hWnd, "FileName") != NULL) {
				free(GetPropA(hWnd, "FileName"));
			}
			break;
		}
		default: return DefWindowProc(hWnd, Message, wParam, lParam);
	}
	return 0;
}
LRESULT CALLBACK MessageViewerProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam) {
	switch(Message) {
		case WM_CREATE: {
			CreateWindowExA(0, "STATIC", NULL, WS_CHILD | WS_VISIBLE | SS_BITMAP | SS_CENTERIMAGE, 0, 0, 0, 0, hWnd, (HMENU)1, NULL, NULL);
			CreateWindowExA(0, "EDIT", NULL, WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | ES_READONLY, 0, 0, 0, 0, hWnd, (HMENU)2, NULL, NULL);
			CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", NULL, WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | ES_AUTOVSCROLL | WS_VSCROLL | WS_HSCROLL | ES_MULTILINE | ES_READONLY, 0, 0, 0, 0, hWnd, (HMENU)3, NULL, NULL);
			int i;
			for(i = 1; i <= 3; i++) {
				SendDlgItemMessage(hWnd, i, WM_SETFONT, (WPARAM)hFont, 0);
				SetWindowTheme(GetDlgItem(hWnd, i), (UseDarkMode == 0) ? L"Explorer" : L"DarkMode_Explorer", NULL);
			}
			break;
		}
		case WM_TIMER: {
			switch(wParam) {
				case 1: {
					SendDlgItemMessage(hWnd, 1, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)GetPropA(hWnd, "ProfilePicture"));
					SetDlgItemText(hWnd, 2, (char*)GetPropA(hWnd, "UserName"));
					SetDlgItemText(hWnd, 3, (char*)GetPropA(hWnd, "Message"));
					break;
				}
			}
			break;
		}
		case WM_SIZE: {
			RECT Rect;
			GetClientRect(hWnd, &Rect);
			SetWindowPos(GetDlgItem(hWnd, 1), NULL, Rect.right * 0.1, Rect.bottom * 0.1, 64, 64, SWP_NOZORDER);
			SetWindowPos(GetDlgItem(hWnd, 2), NULL, Rect.right * 0.1 + 84, Rect.bottom * 0.1 + 17, Rect.right * 0.8 - 84, 30, SWP_NOZORDER);
			SetWindowPos(GetDlgItem(hWnd, 3), NULL, Rect.right * 0.1, Rect.bottom * 0.1 + 84, Rect.right * 0.8, Rect.bottom * 0.9 - 104, SWP_NOZORDER);
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
		case WM_DESTROY: {
			EnableWindow(GetPropA(hWnd, "hWndOwner"), TRUE);
			break;
		}
		default: return DefWindowProc(hWnd, Message, wParam, lParam);
	}
	return 0;
}
/*LRESULT CALLBACK FileBrowserProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam) {
	switch(Message) {
		case WM_CREATE: {
			HWND hWndBrowser = CreateWindowExA(0, "STATIC", "", WS_CHILD | WS_VISIBLE | WS_BORDER, 0, 0, 845, 480, hWnd, (HMENU)20, GetModuleHandle(0), NULL);
			IWebBrowser2 *pBrowser;
			CoInitialize(NULL);
//const struct IID CLSID_WebBrowser2 = ;
#define DEFINE_GUID2(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
const GUID name = { l, w1, w2, { b1, b2, b3, b4, b5, b6, b7, b8 } }
DEFINE_GUID2(CLSID_WebBrowser__, // {8856F961-340A-11D0-A96B-00C04FD705A2}
0x8856F961, 0x340A, 0x11D0, 
0xA9, 0x6B, 0x00, 0xC0, 0x4F, 0xD7, 0x05, 0xA2);
DEFINE_GUID2(IID_IWebBrowser2__, // {D30C1661-CDAF-11D0-8A3E-00C04FC9E26E}
0xD30C1661, 0xCDAF, 0x11D0, 
0x8A, 0x3E, 0x00, 0xC0, 0x4F, 0xC9, 0xE2, 0x6E);
DEFINE_GUID2(IID_IOleObject__, 
0x00000112, 0x0000, 0x0000, 
0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46);
			CoCreateInstance(&CLSID_WebBrowser__, NULL, CLSCTX_ALL, &IID_IWebBrowser2__, (void**)&pBrowser);
			IWebBrowser2Vtbl *pBrowserVtbl = pBrowser -> lpVtbl;
			IOleObject2* pOleObj;
			pBrowserVtbl -> QueryInterface(pBrowser, &IID_IOleObject__, (void**)&pOleObj);
			printf("ok");
			IOleClientSite* pClientSite = NULL;
			CreateOleClientSite(hWndBrowser, &pClientSite);
			pOleObj -> lpVtbl -> SetClientSite((IOleObject*)pOleObj, (IOleClientSite*)pClientSite);
			printf("ok");
			RECT rc;
			GetClientRect(hWndBrowser, &rc);
			printf("ok");
			pOleObj -> lpVtbl -> DoVerb(pOleObj,
			    OLEIVERB_SHOW,
			    NULL,
			    pClientSite,
			    0,
			    hWnd,
			    &rc
			);
			printf("ok");
			pOleObj -> lpVtbl -> Release((IOleObject*)pOleObj);
			printf("ok");
			//NavigateFunc Navigate = (NavigateFunc)((void**)(*(void***)pBrowser))[3];
			pBrowserVtbl -> Navigate(pBrowser,
			    L"http://example.com",
			    NULL,
			    NULL,
			    NULL,
			    NULL
			);
			//((IUnknown*)pBrowser)->lpVtbl -> Release(pBrowser);
			//pBrowserVtbl -> Release(pBrowser);
			break;
		}
		case WM_DESTROY: {
			PostQuitMessage(0);
			break;
		}
		default: return DefWindowProc(hWnd, Message, wParam, lParam);
	}
	return 0;
}*/
int main() {
	GetModuleFileName(NULL, ThisDirectory, 32767);
	int LastSpliter = strlen(ThisDirectory) - 1;
	while(ThisDirectory[LastSpliter] != '\\') LastSpliter -= 1;
	ThisDirectory[LastSpliter] = '\0'; 
	pthread_mutex_init(&Lock, NULL);
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
		}
		CloseJson(lpConfigHandle);
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
	if(RegisterClassEx(&wc) == FALSE) {
		MessageBox(NULL, "Window Registration Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}
	wc.lpfnWndProc	 = WndProc;
	wc.lpszClassName = "NChat-Client";
	if(RegisterClassEx(&wc) == FALSE) {
		MessageBox(NULL, "Window Registration Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
		return -1;
	}
	wc.lpfnWndProc	 = NotifyProc;
	wc.lpszClassName = "NChat-Notify";
	if(RegisterClassEx(&wc) == FALSE) {
		MessageBox(NULL, "Window Registration Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
		return -1;
	}
	wc.lpfnWndProc	 = FileViewProc;
	wc.lpszClassName = "NChat-File-View";
	if(RegisterClassEx(&wc) == FALSE) {
		MessageBox(NULL, "Window Registration Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
		return -1;
	}
	wc.lpfnWndProc	 = MessageViewerProc;
	wc.lpszClassName = "NChat-Message-Viewer";
	if(RegisterClassEx(&wc) == FALSE) {
		MessageBox(NULL, "Window Registration Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
		return -1;
	}
	/*wc.lpfnWndProc	 = FileBrowserProc;
	wc.lpszClassName = "NChat-File-Browser";
	if(RegisterClassEx(&wc) == FALSE) {
		MessageBox(NULL, "Window Registration Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
		return -1;
	}
	hWndMain = CreateWindowEx(0, "NChat-File-Browser", "Login the chat room",
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
	return 0;*/
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
	CreateDirectory("ChatMessages", NULL);
	hWndMain = CreateWindowEx(0, "NChat-Client", "NChat",
		WS_VISIBLE | WS_MINIMIZEBOX | WS_SYSMENU | WS_CAPTION | WS_MAXIMIZEBOX | WS_THICKFRAME,
		CW_USEDEFAULT, CW_USEDEFAULT, 1000, 550, NULL, NULL, NULL, NULL);
	if(hWndMain == NULL) {
		MessageBox(NULL, "Window Creation Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
		return -1;
	}
	hWndNotify = CreateWindowEx(0, "NChat-Notify", "NChat-NotifyHandler", 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL);
	if(hWndNotify == NULL) {
		MessageBox(NULL, "Notification Handler Window Creation Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
		return -1;
	}
	nid.cbSize = sizeof(nid);
	nid.hWnd = hWndNotify;
	nid.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE | NIF_SHOWTIP | NIF_GUID;
	nid.hIcon = wc.hIcon;
	nid.uVersion = NOTIFYICON_VERSION_4;
	nid.dwInfoFlags = NIIF_INFO | NIIF_RESPECT_QUIET_TIME;
	nid.uCallbackMessage = WM_APP + 255;
	CoCreateGuid(&nid.guidItem);
	Shell_NotifyIconA(NIM_ADD, &nid);
	Shell_NotifyIconA(NIM_SETVERSION, &nid);
	pthread_t MessageHandler_t;
	pthread_create(&MessageHandler_t, NULL, RecvMessageThread, NULL);
	//hFocus = SetWindowsHookExA(WH_CALLWNDPROC, FocusProc, GetModuleHandleA(NULL), 0);
	while(GetMessage(&Msg, NULL, 0, 0) > 0) {
		if(IsDialogMessage(hWndMain, &Msg) == FALSE) {
			TranslateMessage(&Msg);
			DispatchMessage(&Msg);
		}
		/*if(Msg.message == WM_KILLFOCUS && (GetParent(Msg.hwnd) == hWndMain || Msg.hwnd == hWndMain)) InFocus = 0;
		if(Msg.message == WM_SETFOCUS) InFocus = 1;
		if(Msg.message == WM_KILLFOCUS && (GetParent(Msg.hwnd) == hWndMain || Msg.hwnd == hWndMain)) printf("Msg: WM_KILLFOCUS, MainWindow = %p, Window = %p, Parent = %p", hWndMain, Msg.hwnd, GetParent(Msg.hwnd));
		if(Msg.message == WM_SETFOCUS) printf("Msg: WM_SETFOCUS, MainWindow = %p, Window = %p, Parent = %p", hWndMain, Msg.hwnd, GetParent(Msg.hwnd));
	*/}
	closesocket(sockfd);
	WSACleanup();
	//UnhookWindowsHookEx(hFocus);
	Shell_NotifyIconA(NIM_DELETE, &nid);
	return Msg.wParam;
}
