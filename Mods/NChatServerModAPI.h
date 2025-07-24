//FOR DEVELOP NCHAT SERVER MOD, YOU NEED THIS
#ifndef __NCHAT_SERVER_MOD_API__
#define __NCHAT_SERVER_MOD_API__
#ifndef _TIME_H_
#include <time.h>
#endif
typedef void *HUSER;
typedef void (*MSGLISTENERFUNC)(const char*, const char*, int, const char*, time_t);//FakeUserName, UserName, MsgType(0=Text, 1=Join, 2=Left, 3=File, 4=SilencerAdd, 5=SilencerRemove), Message, Time
typedef void (*MODLOGOUTFUNC)(const char* Poster, const char* Type, int NoNewLine, const char* Format, ...);
typedef int (*MODGETUSERSLISTFUNC)(char** UsersList, int nMax);
typedef HUSER (*MODCREATEUSERFUNC)(const char* ModuleName, const char* UserName, MSGLISTENERFUNC MsgListener);
typedef void (*MODCLOSEUSERFUNC)(const char* ModuleName, HUSER hUser);
typedef void (*MODFAKEUSERSPEAK)(HUSER hUser, int NeedCallBack, const char *Message);
typedef int (*MODISMENTIONEDUSER)(const char* Message, const char* UserName, const char** FirstMentionedPos); 
typedef int (*MODISUSERONLINE)(const char* UserName);
typedef int (*MODMAKEUSERSILENCE)(const char* ModuleName, const char* UserName); 
typedef int (*MODMAKEUSERNOTSILENCE)(const char* ModuleName, const char* UserName);
typedef int (*MODGETCONFIGVALUE)(const char* NameSpace, const char* Key, char* Value);
typedef int (*MODSETCONFIGVALUE)(const char* NameSpace, const char* Key, const char* Value);
typedef struct{
	size_t cbSize;
	MODLOGOUTFUNC ModLogOut;//void ModLogOut(const char* ModuleName, const char* Type, int NoNewLine, const char* Format, ...);
	MODGETUSERSLISTFUNC ModGetUsersList;//int ModGetUsersList(char **Users, int iBufferMax);
	MODCREATEUSERFUNC ModCreateUser;//HUSER ModCreateUser(const char* UserName, MSGLISTENERFUNC MsgListener);
	MODCLOSEUSERFUNC ModCloseUser;//void ModCloseUser(const char* ModuleName, HUSER hUser);
	MODFAKEUSERSPEAK ModFakeUserSpeak;//void ModFakeUserSpeak(HUSER hUser, int NeedCallBack, const char *Message);
	MODISMENTIONEDUSER ModIsMentionedUser;//int ModIsMentionedUser(const char* Message, const char* UserName, const char** FirstMentionedPos);
	MODISUSERONLINE ModIsUserOnline;//int ModIsUserOnline(const char* UserName);
	MODMAKEUSERSILENCE ModMakeUserSilence;//int ModMakeUserSilence(const char* ModuleName, const char* UserName); 
	MODMAKEUSERNOTSILENCE ModMakeUserNotSilence;//int ModMakeUserNotSilence(const char* ModuleName, const char* UserName); 
	MODGETCONFIGVALUE ModGetConfigValue;//int ModGetConfigValue(const char* NameSpace, const char* Key, char* Value);
	MODSETCONFIGVALUE ModSetConfigValue;//void ModSetConfigValue(const char* NameSpace, const char* Key, const char* Value);
}MODINTERFACE, *LPMODINTERFACE;
#ifdef _WIN32
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT __attribute__((visibility("default")))
#endif
#endif
