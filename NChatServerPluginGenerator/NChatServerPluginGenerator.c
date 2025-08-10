#include <stdio.h>
#include <windows.h> 
const char PluginMainCode[] = "/*This is a mod example code...(Version 1.0)*/\n\
#include <string.h>\n\
#include \"NChatServerPluginAPI.h\"\n\
/*Functions(Except InitInterfaces function) should be static, so that they will not be exported.*/\n\
LPPLUGININTERFACE Interfaces;\n\
#ifdef __cplusplus//DO NOT MODIFY THESE PREPROCESSOR CODE. IT'S FOR COMPATIBLE WITH C++\n\
extern \"C\"\n\
#endif\n\
DLLEXPORT void InitInterfaces(LPPLUGININTERFACE API_Interfaces, char *PluginName) {/*Do not edit the defination this function*/\n\
	Interfaces = API_Interfaces;\n\
	/*DO NOT EDIT CODES IN THIS FUNCTION BEFORE THIS LINE. IT'S IMPORTANT FOR YOUR PLUGIN TO CALL API!!!*/\n\
	strcpy(PluginName, \"%s\");\n\
	/*Write Initialization Codes of This Plugin Here...*/\n\
	\n\
	return ;\n\
}";
const char PluginAPICode[] = "//FOR DEVELOP NCHAT SERVER PLUGIN, YOU NEED THIS\n\
#ifndef __NCHAT_SERVER_PLUGIN_API__\n\
#define __NCHAT_SERVER_PLUGIN_API__\n\
#ifndef _TIME_H_\n\
#include <time.h>\n\
#endif\n\
typedef void *HUSER, *HNOUSERLISTENERFUNC;\n\
typedef void (*MSGLISTENERFUNC)(const char*, const char*, int, const char*, time_t);//FakeUserName, UserName, MsgType(0=Text, 1=Join, 2=Left, 3=File, 4=SilencerAdd, 5=SilencerRemove, 6=GlobalMessage), Message, Time\n\
typedef void (*MSGLISTENERNOUSERFUNC)(const char*, int, const char*, time_t);//UserName, MsgType(0=Text, 1=Join, 2=Left, 3=File, 4=SilencerAdd, 5=SilencerRemove, 6=GlobalMessage), Message, Time\n\
typedef void (*PLUGINLOGOUTFUNC)(const char* Poster, const char* Type, int NoNewLine, const char* Format, ...);\n\
typedef int (*PLUGINGETUSERSLISTFUNC)(char** UsersList, int nMax);\n\
typedef HUSER (*PLUGINCREATEUSERFUNC)(const char* ModuleName, const char* UserName, MSGLISTENERFUNC MsgListener);\n\
typedef void (*PLUGINCLOSEUSERFUNC)(const char* ModuleName, HUSER hUser);\n\
typedef void (*PLUGINFAKEUSERSPEAK)(HUSER hUser, int NeedCallBack, const char *Message);\n\
typedef int (*PLUGINISMENTIONEDUSER)(const char* Message, const char* UserName, const char** FirstMentionedPos); \n\
typedef int (*PLUGINISUSERONLINE)(const char* UserName);\n\
typedef int (*PLUGINMAKEUSERSILENCE)(const char* ModuleName, const char* UserName);\n\
typedef int (*PLUGINMAKEUSERNOTSILENCE)(const char* ModuleName, const char* UserName);\n\
typedef int (*PLUGINGETCONFIGVALUE)(const char* NameSpace, const char* Key, char* Value);\n\
typedef int (*PLUGINSETCONFIGVALUE)(const char* NameSpace, const char* Key, const char* Value);\n\
typedef void (*PLUGINGLOBALMESSAGE)(const char* ModuleName, const char* Message);\n\
typedef HNOUSERLISTENERFUNC (*PLUGINADDLISTENER)(MSGLISTENERNOUSERFUNC MsgListener);\n\
typedef void (*PLUGINREMOVELISTENER)(HNOUSERLISTENERFUNC hNoUserListenerFunc);\n\
typedef struct{\n\
	size_t cbSize;\n\
	PLUGINLOGOUTFUNC PluginLogOut;//void PluginLogOut(const char* ModuleName, const char* Type, int NoNewLine, const char* Format, ...);\n\
	PLUGINGETUSERSLISTFUNC PluginGetUsersList;//int PluginGetUsersList(char **Users, int iBufferMax);\n\
	PLUGINCREATEUSERFUNC PluginCreateUser;//HUSER PluginCreateUser(const char* UserName, MSGLISTENERFUNC MsgListener);\n\
	PLUGINCLOSEUSERFUNC PluginCloseUser;//void PluginCloseUser(const char* ModuleName, HUSER hUser);\n\
	PLUGINFAKEUSERSPEAK PluginFakeUserSpeak;//void PluginFakeUserSpeak(HUSER hUser, int NeedCallBack, const char *Message);\n\
	PLUGINISMENTIONEDUSER PluginIsMentionedUser;//int PluginIsMentionedUser(const char* Message, const char* UserName, const char** FirstMentionedPos);\n\
	PLUGINISUSERONLINE PluginIsUserOnline;//int PluginIsUserOnline(const char* UserName);\n\
	PLUGINMAKEUSERSILENCE PluginMakeUserSilence;//int PluginMakeUserSilence(const char* ModuleName, const char* UserName);\n\
	PLUGINMAKEUSERNOTSILENCE PluginMakeUserNotSilence;//int PluginMakeUserNotSilence(const char* ModuleName, const char* UserName);\n\
	PLUGINGETCONFIGVALUE PluginGetConfigValue;//int PluginGetConfigValue(const char* NameSpace, const char* Key, char* Value);\n\
	PLUGINSETCONFIGVALUE PluginSetConfigValue;//void PluginSetConfigValue(const char* NameSpace, const char* Key, const char* Value);\n\
	PLUGINGLOBALMESSAGE PluginGolbalMessage;//void PluginGolbalMessage(const char* ModuleName, const char* Message);\n\
	PLUGINADDLISTENER PluginAddListener;//HNOUSERLISTENERFUNC PluginAddListener(MSGLISTENERNOUSERFUNC MsgListener);\n\
	PLUGINREMOVELISTENER PluginRemoveListener;//void PluginRemoveListener(HNOUSERLISTENERFUNC hNoUserListenerFunc);\n\
}PLUGININTERFACE, *LPPLUGININTERFACE;\n\
#ifdef _WIN32\n\
#define DLLEXPORT __declspec(dllexport)\n\
#else\n\
#define DLLEXPORT __attribute__((visibility(\"default\")))\n\
#endif\n\
#endif";
const char DevCode[] = "[Project]\n\
FileName=%s.dev\n\
Name=%s\n\
Type=3\n\
Ver=2\n\
ObjFiles=\n\
Includes=\n\
Libs=\n\
PrivateResource=\n\
ResourceIncludes=\n\
MakeIncludes=\n\
Compiler=-DBUILDING_DLL=1\n\
CppCompiler=-DBUILDING_DLL=1\n\
Linker=\n\
IsCpp=%d\n\
Icon=\n\
ExeOutput=\n\
ObjectOutput=\n\
LogOutput=\n\
LogOutputEnabled=0\n\
OverrideOutput=0\n\
OverrideOutputName=\n\
HostApplication=\n\
UseCustomMakefile=0\n\
CustomMakefile=\n\
CommandLine=\n\
Folders=\n\
IncludeVersionInfo=0\n\
SupportXPThemes=0\n\
CompilerSet=3\n\
CompilerSettings=0000000100000000000000000\n\
UnitCount=2\n\
\n\
[VersionInfo]\n\
Major=1\n\
Minor=0\n\
Release=0\n\
Build=0\n\
LanguageID=1033\n\
CharsetID=1252\n\
CompanyName=\n\
FileVersion=1.0.0.0\n\
FileDescription=Developed using the Dev-C++ IDE\n\
InternalName=\n\
LegalCopyright=\n\
LegalTrademarks=\n\
OriginalFilename=\n\
ProductName=\n\
ProductVersion=1.0.0.0\n\
AutoIncBuildNr=0\n\
SyncProduct=1\n\
\n\
[Unit1]\n\
FileName=PluginMain.%s\n\
CompileCpp=%d\n\
Folder=\n\
Compile=1\n\
Link=1\n\
Priority=1000\n\
OverrideBuildCmd=0\n\
BuildCmd=\n\
\n\
[Unit2]\n\
FileName=NChatServerPluginAPI.h\n\
CompileCpp=0\n\
Folder=\n\
Compile=1\n\
Link=1\n\
Priority=1000\n\
OverrideBuildCmd=0\n\
BuildCmd=\n\
\n\
";
char PluginName[256], lang[64], ProjectDirectory[32767], FilePath[32767];
int main() {
	RetryDir:
	printf("Please enter your project directory(Your project will create in ${Directory}\\${PluginName}): ");
	scanf("%s", ProjectDirectory);
	if(SetCurrentDirectoryA(ProjectDirectory) == FALSE) {
		printf("Target directory is invalid. Please try again...\n");
		goto RetryDir;
	}
	RetryName:
	printf("Please enter your Plugin Name: ");
	getchar();
	gets(PluginName);
	if(CreateDirectory(PluginName, NULL) == FALSE) {
		printf("Created directory failed! Please try again...\n");
		goto RetryName;
	}
	RetryLang:
	printf("Please choose your language(C\\C++): ");
	scanf("%s", lang);
	if(strcmp(lang, "C") != 0 && strcmp(lang, "C++") != 0) {
		printf("Invalid language! Please try again...\n");
		goto RetryLang;
	}
	printf("Starting init NChat Server Plugin project...\n");
	printf("Extracting File 'PluginMain.%s'...\n", (strcmp(lang, "C") == 0) ? "c" : "cpp");
	sprintf(FilePath, "%s\\%s\\PluginMain.%s", ProjectDirectory, PluginName, (strcmp(lang, "C") == 0) ? "c" : "cpp");
	FILE* lpFile = fopen(FilePath, "w");
	if(lpFile == NULL) {
		printf("Opening %s file failed! Operation Terminated.\n", FilePath);
		system("pause");
		return 0;
	}
	fprintf(lpFile, PluginMainCode, PluginName);
	fclose(lpFile);
	printf("Extracting File 'NChatServerPluginAPI.h'...\n");
	sprintf(FilePath, "%s\\%s\\NChatServerPluginAPI.h", ProjectDirectory, PluginName);
	lpFile = fopen(FilePath, "w");
	if(lpFile == NULL) {
		printf("Opening %s file failed! Operation Terminated.\n", FilePath);
		system("pause");
		return 0;
	}
	fprintf(lpFile, PluginAPICode);
	fclose(lpFile);
	printf("Extracting File '%s.dev'...\n", PluginName);
	sprintf(FilePath, "%s\\%s\\%s.dev", ProjectDirectory, PluginName, PluginName);
	lpFile = fopen(FilePath, "w");
	if(lpFile == NULL) {
		printf("Opening %s file failed! Operation Terminated.\n", FilePath);
		system("pause");
		return 0;
	}
	fprintf(lpFile, DevCode, PluginName, PluginName, (strcmp(lang, "C++") == 0) ? 1 : 0, (strcmp(lang, "C") == 0) ? "c" : "cpp", (strcmp(lang, "C++") == 0) ? 1 : 0);
	fclose(lpFile);
	return 0;
}
