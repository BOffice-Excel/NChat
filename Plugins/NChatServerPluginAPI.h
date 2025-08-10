//FOR DEVELOP NCHAT SERVER PLUGIN, YOU NEED THIS
#ifndef __NCHAT_SERVER_PLUGIN_API__
#define __NCHAT_SERVER_PLUGIN_API__
#ifndef _TIME_H_
#include <time.h>
#endif
typedef void *HUSER, *HNOUSERLISTENERFUNC;
typedef void (*MSGLISTENERFUNC)(const char*, const char*, int, const char*, time_t);//FakeUserName, UserName, MsgType(0=Text, 1=Join, 2=Left, 3=File, 4=SilencerAdd, 5=SilencerRemove, 6=GlobalMessage), Message, Time
typedef void (*MSGLISTENERNOUSERFUNC)(const char*, int, const char*, time_t);//UserName, MsgType(0=Text, 1=Join, 2=Left, 3=File, 4=SilencerAdd, 5=SilencerRemove, 6=GlobalMessage), Message, Time
typedef void (*PLUGINLOGOUTFUNC)(const char* Poster, const char* Type, int NoNewLine, const char* Format, ...);
typedef int (*PLUGINGETUSERSLISTFUNC)(char** UsersList, int nMax);
typedef HUSER (*PLUGINCREATEUSERFUNC)(const char* ModuleName, const char* UserName, MSGLISTENERFUNC MsgListener);
typedef void (*PLUGINCLOSEUSERFUNC)(const char* ModuleName, HUSER hUser);
typedef void (*PLUGINFAKEUSERSPEAK)(HUSER hUser, int NeedCallBack, const char *Message);
typedef int (*PLUGINISMENTIONEDUSER)(const char* Message, const char* UserName, const char** FirstMentionedPos); 
typedef int (*PLUGINISUSERONLINE)(const char* UserName);
typedef int (*PLUGINMAKEUSERSILENCE)(const char* ModuleName, const char* UserName); 
typedef int (*PLUGINMAKEUSERNOTSILENCE)(const char* ModuleName, const char* UserName);
typedef int (*PLUGINGETCONFIGVALUE)(const char* NameSpace, const char* Key, char* Value);
typedef int (*PLUGINSETCONFIGVALUE)(const char* NameSpace, const char* Key, const char* Value);
typedef void (*PLUGINGLOBALMESSAGE)(const char* ModuleName, const char* Message);
typedef HNOUSERLISTENERFUNC (*PLUGINADDLISTENER)(MSGLISTENERNOUSERFUNC MsgListener);
typedef void (*PLUGINREMOVELISTENER)(HNOUSERLISTENERFUNC hNoUserListenerFunc);
typedef struct{
	size_t cbSize;
	PLUGINLOGOUTFUNC PluginLogOut;//void PluginLogOut(const char* ModuleName, const char* Type, int NoNewLine, const char* Format, ...);
	PLUGINGETUSERSLISTFUNC PluginGetUsersList;//int PluginGetUsersList(char **Users, int iBufferMax);
	PLUGINCREATEUSERFUNC PluginCreateUser;//HUSER PluginCreateUser(const char* UserName, MSGLISTENERFUNC MsgListener);
	PLUGINCLOSEUSERFUNC PluginCloseUser;//void PluginCloseUser(const char* ModuleName, HUSER hUser);
	PLUGINFAKEUSERSPEAK PluginFakeUserSpeak;//void PluginFakeUserSpeak(HUSER hUser, int NeedCallBack, const char *Message);
	PLUGINISMENTIONEDUSER PluginIsMentionedUser;//int PluginIsMentionedUser(const char* Message, const char* UserName, const char** FirstMentionedPos);
	PLUGINISUSERONLINE PluginIsUserOnline;//int PluginIsUserOnline(const char* UserName);
	PLUGINMAKEUSERSILENCE PluginMakeUserSilence;//int PluginMakeUserSilence(const char* ModuleName, const char* UserName); 
	PLUGINMAKEUSERNOTSILENCE PluginMakeUserNotSilence;//int PluginMakeUserNotSilence(const char* ModuleName, const char* UserName); 
	PLUGINGETCONFIGVALUE PluginGetConfigValue;//int PluginGetConfigValue(const char* NameSpace, const char* Key, char* Value);
	PLUGINSETCONFIGVALUE PluginSetConfigValue;//void PluginSetConfigValue(const char* NameSpace, const char* Key, const char* Value);
	PLUGINGLOBALMESSAGE PluginGolbalMessage;//void PluginGolbalMessage(const char* ModuleName, const char* Message);
	PLUGINADDLISTENER PluginAddListener;//HNOUSERLISTENERFUNC PluginAddListener(MSGLISTENERNOUSERFUNC MsgListener);
	PLUGINREMOVELISTENER PluginRemoveListener;//void PluginRemoveListener(HNOUSERLISTENERFUNC hNoUserListenerFunc);
}PLUGININTERFACE, *LPPLUGININTERFACE;
#ifdef _WIN32
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT __attribute__((visibility("default")))
#endif
#endif
