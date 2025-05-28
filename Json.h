#ifndef __JSON__
#define __JSON__
#ifndef _INC_STDIO
#include<stdio.h>
#endif //_INC_STDIO
#ifndef _INC_STRING
#include<string.h>
#endif //_INC_STRING
#ifndef _INC_STDLIB
#include<stdlib.h>
#endif //_INC_STDLIB
#ifndef _APISETSTRING_
#include <stringapiset.h>
#endif //_APISETSTRING_
typedef enum tagJsonObjectType{
	OT_STRING=0,
	OT_BOOLEAN,
	OT_NUMBER,
	OT_LIST,
	OT_CHILD,
	OT_LSTCHILDSTR,
	OT_LSTCHILDNUM,
	OT_LSTCHILDBOOL,
	OT_UNKNOWN
}JsonObjectType,*lpJsonObjectType;
typedef struct tagJSONOBJ{
	JsonObjectType iMask;
	char *lpstrKey;
	JsonObjectType FatherMask;
	struct tagJSONOBJ *Next;
	int DoubleLength;
	union{
		char *lpstrValue;
		struct tagJSONOBJ *Childs;
		int TorF;
		double Number;
	};
}JSONOBJ,*LPJSONOBJ;
void WriteOutJsonText(char *DestJsonText,LPJSONOBJ lpJsonObj,JsonObjectType FatherType){
	int idx=0;
	DestJsonText[idx++]=((FatherType==OT_CHILD)?'{':'[');
	LPJSONOBJ pJsonObj=lpJsonObj; 
	while(pJsonObj!=NULL){
		switch(pJsonObj->iMask){
			case OT_STRING:{
				sprintf(DestJsonText+idx,"\"%s\":\"%s\"",pJsonObj->lpstrKey,pJsonObj->lpstrValue);
				idx=strlen(DestJsonText);
				break;
			}
			case OT_BOOLEAN:{
				sprintf(DestJsonText+idx,"\"%s\":%s",pJsonObj->lpstrKey,(pJsonObj->TorF==1)?"true":"false");
				idx=strlen(DestJsonText);
				break;
			}
			case OT_NUMBER:{
				char FormatText[114];
				sprintf(FormatText,"\"%%s\":%%.%dlf",pJsonObj->DoubleLength);
				sprintf(DestJsonText+idx,FormatText,pJsonObj->lpstrKey,pJsonObj->Number);
				idx=strlen(DestJsonText);
				break;
			}
			case OT_LSTCHILDSTR:{
				sprintf(DestJsonText+idx,"\"%s\"",pJsonObj->lpstrValue);
				idx=strlen(DestJsonText);
				break;
			}
			case OT_LSTCHILDNUM:{
				char FormatText[114];
				sprintf(FormatText,"%%.%dlf",pJsonObj->DoubleLength);
				sprintf(DestJsonText+idx,FormatText,pJsonObj->Number);
				idx=strlen(DestJsonText);
				break;
			}
			case OT_LSTCHILDBOOL:{
				sprintf(DestJsonText+idx,"%s",(pJsonObj->TorF==1)?"true":"false");
				idx=strlen(DestJsonText);
				break;
			}
			case OT_CHILD:
			case OT_LIST:{
				if(pJsonObj->lpstrKey!=NULL){
					sprintf(DestJsonText+idx,"\"%s\":",pJsonObj->lpstrKey);
					idx=strlen(DestJsonText);
				}
				WriteOutJsonText(DestJsonText+idx,pJsonObj->Childs,pJsonObj->iMask);
				break;
			}
		}
		idx=strlen(DestJsonText);
		if(pJsonObj->Next!=NULL) strcat(DestJsonText+idx,","),idx++;
		pJsonObj=pJsonObj->Next;
	}
	DestJsonText[idx++]=((FatherType==OT_CHILD)?'}':']');
} 
//GBK Charset, doesnot support UTF-8
LPJSONOBJ BuildJson(const char *JsonText,int Length,JsonObjectType *FatherType){
	//for(int i=0;i<Length;i++) printf("%c",JsonText[i]);
	//printf("\n");
	LPJSONOBJ FirstChild=NULL,ThisObject=NULL;
	int i;
	for(i=0;i<Length;i++){
		if(JsonText[i]==','||JsonText[i]=='\n'||JsonText[i]=='\r'||JsonText[i]==' '||JsonText[i]=='\t') continue;
		if(JsonText[i]=='{'){
			int Braces=0,inString=0,j;
			for(j=i+1;j<Length;j++){
				if(JsonText[j]=='"'&&JsonText[j-1]!='\\'){
					if(inString==0) inString=1;
					else inString=0;
				}
				else if(inString==1) continue;
				else if(JsonText[j]=='{'||JsonText[j]=='[') Braces++;
				else if(JsonText[j]=='}'||JsonText[j]==']'){
					if(Braces==0){
						LPJSONOBJ obj=BuildJson(JsonText+i+1,j-i-1,NULL);
						if(ThisObject==NULL) FirstChild=ThisObject=(LPJSONOBJ)calloc(1,sizeof(JSONOBJ));
						else{
							ThisObject->Next=(LPJSONOBJ)calloc(1,sizeof(JSONOBJ));
							ThisObject=ThisObject->Next;
						}
						ThisObject->Childs=obj;
						ThisObject->iMask=OT_CHILD;
						break;
					}
					else Braces--;
				}
			}
			i=j+1;
			for(;JsonText[i]=='\t'||JsonText[i]=='\n'||JsonText[i]==' '||JsonText[i]==','||JsonText[i]=='\r';i++);
			i--;
		}
		else if(JsonText[i]=='['){
			int Braces=0,inString=0,j;
			for(j=i+1;j<Length;j++){
				if(JsonText[j]=='"'&&JsonText[j-1]!='\\'){
					if(inString==0) inString=1;
					else inString=0;
				}
				else if(inString==1) continue;
				else if(JsonText[j]=='{'||JsonText[j]=='[') Braces++;
				else if(JsonText[j]=='}'||JsonText[j]==']'){
					if(Braces==0){
						LPJSONOBJ obj=BuildJson(JsonText+i+1,j-i-1,NULL);
						if(ThisObject==NULL) FirstChild=ThisObject=(LPJSONOBJ)calloc(1,sizeof(JSONOBJ));
						else{
							ThisObject->Next=(LPJSONOBJ)calloc(1,sizeof(JSONOBJ));
							ThisObject=ThisObject->Next;
						}
						ThisObject->Childs=obj;
						ThisObject->iMask=OT_LIST;
						break;
					}
					else Braces--;
				}
			}
			i=j+1;
			for(;JsonText[i]=='\t'||JsonText[i]=='\n'||JsonText[i]==' '||JsonText[i]==','||JsonText[i]=='\r';i++);
			i--;
		}
		else if(JsonText[i]<='9'&&JsonText[i]>='0'){
			double Value=0,Sm=0.0;
			int j,SmCnt=0;
			for(j=0;JsonText[i+j]>='0'&&JsonText[i+j]<='9';j++) Value=Value*10+JsonText[i+j]-'0';
			i+=j+1;
			if(ThisObject==NULL) FirstChild=ThisObject=(LPJSONOBJ)calloc(1,sizeof(JSONOBJ));
			else{
				ThisObject->Next=(LPJSONOBJ)calloc(1,sizeof(JSONOBJ));
				ThisObject=ThisObject->Next;
			}
			if(JsonText[i-1]=='.'){
				for(j=-1;JsonText[i+j+1]>='0'&&JsonText[i+j+1]<='9';j++) Sm=Sm*10+JsonText[i+j+1]-'0',SmCnt++;
				ThisObject->DoubleLength=SmCnt;
				while(SmCnt--) Sm/=10;
				Value+=Sm;
				i+=j+1;
			}
			ThisObject->Number=Value;
			ThisObject->iMask=OT_LSTCHILDNUM;
		}
		else if(JsonText[i]=='t'||JsonText[i]=='f'||JsonText[i]=='T'||JsonText[i]=='F'){
			char *Key=(char*)calloc(1145,sizeof(char));
			int j;
			for(j=0;(JsonText[i+j]>='a'&&JsonText[i+j]<='z'||JsonText[i+j]>='A'&&JsonText[i+j]<='Z')&&i+j<Length;j++) Key[j]=((JsonText[i+j]<='Z')?JsonText[i+j]+'a'-'A':JsonText[i+j]);
			if(ThisObject==NULL) FirstChild=ThisObject=(LPJSONOBJ)calloc(1,sizeof(JSONOBJ));
			else{
				ThisObject->Next=(LPJSONOBJ)calloc(1,sizeof(JSONOBJ));
				ThisObject=ThisObject->Next;
			}
			ThisObject->TorF=(strcmp(Key,"true")==0);
			ThisObject->iMask=OT_LSTCHILDBOOL;
			i+=j;
		}
		else if(JsonText[i]=='"'){
			char *Key=(char*)calloc(114514,sizeof(char));
			int j;
			for(j=0;(JsonText[i+j+1]!='"'||JsonText[i+j]=='\\')&&i+j+1<Length;j++) Key[j]=JsonText[i+j+1];
			i+=j+2;
			while(JsonText[i]==' '||JsonText[i]==':'||JsonText[i]=='\t'||JsonText[i]=='\n'||JsonText[i]=='\r') i++;
			if(JsonText[i]==','||JsonText[i]==']'||JsonText[i]=='}'){
				if(ThisObject==NULL) FirstChild=ThisObject=(LPJSONOBJ)calloc(1,sizeof(JSONOBJ));
				else{
					ThisObject->Next=(LPJSONOBJ)calloc(1,sizeof(JSONOBJ));
					ThisObject=ThisObject->Next;
				}
				ThisObject->lpstrValue=Key;
				ThisObject->iMask=OT_LSTCHILDSTR;
			}
			else if(JsonText[i]=='"'){
				char *Value=(char*)calloc(114514,sizeof(char));
				int Index=0;
				for(j=0;JsonText[i+j+1]!='"';j++){
					if(JsonText[i+j+1]=='\\'){
						switch(JsonText[i+j+2]){
							case '\\':{
								Value[Index++]='\\';
								j++;
								break;
							}
							case '"':{
								Value[Index++]='"';
								j++;
								break;
							}
							case 't':{
								Value[Index++]='\t';
								j++;
								break;
							}
							case 'n':{
								Value[Index++]='\n';
								j++;
								break;
							}
							case 'b':{
								Value[Index++]='\b';
								j++;
								break;
							}
							case 'r':{
								Value[Index++]='\r';
								j++;
								break;
							}
							case 'f':{
								Value[Index++]='\f';
								j++; 
								break;
							}
							case '/':{
								Value[Index++]='/';
								j++;
								break;
							}
							case 'u':{
								wchar_t ch=(((JsonText[i+j+3]>'9')?((JsonText[i+j+3]>='a')?(JsonText[i+j+3]-'a'+10):(JsonText[i+j+3]-'A'+10)):(JsonText[i+j+3]-'0'))*16+((JsonText[i+j+4]>'9')?((JsonText[i+j+4]>='a')?(JsonText[i+j+4]-'a'+10):(JsonText[i+j+4]-'A'+10)):(JsonText[i+j+4]-'0')))*16;
								j+=2;
								ch+=((JsonText[i+j+3]>'9')?((JsonText[i+j+3]>='a')?(JsonText[i+j+3]-'a'+10):(JsonText[i+j+3]-'A'+10)):(JsonText[i+j+3]-'0'))*16+((JsonText[i+j+4]>'9')?((JsonText[i+j+4]>='a')?(JsonText[i+j+4]-'a'+10):(JsonText[i+j+4]-'A'+10)):(JsonText[i+j+4]-'0'));
								wchar_t str_w[3];
								str_w[0]=ch;
								str_w[1]='\0';
								int len=WideCharToMultiByte(CP_UTF8,0,str_w,-1,NULL,0,NULL,NULL);
								char *tmp=(char*)calloc(len+10,sizeof(char));
								WideCharToMultiByte(CP_UTF8,0,str_w,-1,tmp,len+1,NULL,NULL);
								strcat(Value,tmp);
								Index=strlen(Value);
								j+=3;
								break;
							}
						}
					}
					else Value[Index++]=JsonText[i+j+1];
				}
				if(ThisObject==NULL) FirstChild=ThisObject=(LPJSONOBJ)calloc(1,sizeof(JSONOBJ));
				else{
					ThisObject->Next=(LPJSONOBJ)calloc(1,sizeof(JSONOBJ));
					ThisObject=ThisObject->Next;
				}
				ThisObject->lpstrKey=Key;
				ThisObject->lpstrValue=Value;
				ThisObject->iMask=OT_STRING;
				i+=j+2;
			}
			else if(JsonText[i]=='t'||JsonText[i]=='f'){
				char *Value=(char*)calloc(1145,sizeof(char));
				for(j=-1;JsonText[i+j+1]!=','&&JsonText[i+j+1]!=' '&&JsonText[i+j+1]!='}';j++) Value[j+1]=JsonText[i+j+1];
				if(ThisObject==NULL) FirstChild=ThisObject=(LPJSONOBJ)calloc(1,sizeof(JSONOBJ));
				else{
					ThisObject->Next=(LPJSONOBJ)calloc(1,sizeof(JSONOBJ));
					ThisObject=ThisObject->Next;
				}
				ThisObject->lpstrKey=Key;
				ThisObject->TorF=(strcmp(Value,"true")==0);
				ThisObject->iMask=OT_BOOLEAN;
				i+=j+2;
			}
			else if(JsonText[i]<='9'&&JsonText[i]>='0'){
				double Value=0,Sm=0.0,SmCnt=0;
				for(j=-1;JsonText[i+j+1]>='0'&&JsonText[i+j+1]<='9';j++) Value=Value*10+JsonText[i+j+1]-'0';
				i+=j+2;
				if(ThisObject==NULL) FirstChild=ThisObject=(LPJSONOBJ)calloc(1,sizeof(JSONOBJ));
				else{
					ThisObject->Next=(LPJSONOBJ)calloc(1,sizeof(JSONOBJ));
					ThisObject=ThisObject->Next;
				}
				if(JsonText[i-1]=='.'){
					for(j=-1;JsonText[i+j+1]>='0'&&JsonText[i+j+1]<='9';j++) Sm=Sm*10+JsonText[i+j+1]-'0',SmCnt++;
					ThisObject->DoubleLength=SmCnt;
					while(SmCnt--) Sm/=10;
					Value+=Sm;
					i+=j+1;
				}
				ThisObject->lpstrKey=Key;
				ThisObject->Number=Value;
				ThisObject->iMask=OT_NUMBER;
			}
			else if(JsonText[i]=='{'||JsonText[i]=='['){
				int Braces=0,inString=0,j;
				for(j=i+1;j<Length;j++){
					if(JsonText[j]=='"'&&JsonText[j-1]!='\\'){
						if(inString==0) inString=1;
						else inString=0;
					}
					else if(inString==1) continue;
					else if(JsonText[j]=='{'||JsonText[j]=='[') Braces++;
					else if(JsonText[j]=='}'||JsonText[j]==']'){
						if(Braces==0){
							LPJSONOBJ obj=BuildJson(JsonText+i+1,j-i-1,NULL);
							if(ThisObject==NULL) FirstChild=ThisObject=(LPJSONOBJ)calloc(1,sizeof(JSONOBJ));
							else{
								ThisObject->Next=(LPJSONOBJ)calloc(1,sizeof(JSONOBJ));
								ThisObject=ThisObject->Next;
							}
							ThisObject->Childs=obj;
							ThisObject->lpstrKey=Key;
							ThisObject->iMask=((JsonText[i]=='{')?OT_CHILD:OT_LIST);
							break;
						}
						else Braces--;
					}
				}
				i=j;
			}
			else{
				ThisObject->Next=(LPJSONOBJ)calloc(1,sizeof(JSONOBJ));
				ThisObject=ThisObject->Next;
				ThisObject->lpstrKey=Key;
				ThisObject->iMask=OT_UNKNOWN;
				i+=j+2;
			}
			for(;JsonText[i]==' '||JsonText[i]=='\t'||JsonText[i]==','||JsonText[i]=='\n'||JsonText[i]=='\r';i++);
			i--;
		}
	}
	if(FatherType) *FatherType=FirstChild->iMask;
	//if(Length==strlen(JsonText)) return FirstChild->Childs;
	return FirstChild; 
}
void CloseJson(LPJSONOBJ JsonLst){
	while(JsonLst!=NULL){
		if(JsonLst->iMask==OT_CHILD||JsonLst->iMask==OT_LIST) CloseJson(JsonLst->Childs);
		LPJSONOBJ Old=JsonLst;
		JsonLst=JsonLst->Next;
		free(Old);
	}
	return;
}
void printDeep(int Deep){int i;for(i=1;i<=Deep;i++) printf("  ");}
void TryList(LPJSONOBJ JsonLst,int Deep,JsonObjectType ParentType){
	printf("%c\n",((ParentType==OT_CHILD)?'{':'['));
	while(JsonLst!=NULL){
		switch(JsonLst->iMask){
			case OT_NUMBER:{
				printDeep(Deep+1);
				char *str=(char*)calloc(114,1);
				sprintf(str,"\"%s\": %%.%dlf",JsonLst->lpstrKey,JsonLst->DoubleLength);
				printf(str,JsonLst->Number);
				break;
			}
			case OT_BOOLEAN:{
				printDeep(Deep+1);
				printf("\"%s\": %d",JsonLst->lpstrKey,JsonLst->TorF);
				break;
			}
			case OT_STRING:{
				printDeep(Deep+1);
				printf("\"%s\": \"%s\"",JsonLst->lpstrKey,JsonLst->lpstrValue);
				break;
			}
			case OT_CHILD:
			case OT_LIST:{
				printDeep(Deep+1);
				if(JsonLst->lpstrKey!=NULL) printf("\"%s\": ",JsonLst->lpstrKey);
				TryList(JsonLst->Childs,Deep+1,JsonLst->iMask);
				break;
			}
			case OT_LSTCHILDSTR:{
				printDeep(Deep+1);
				printf("\"%s\"",JsonLst->lpstrValue);
				break;
			}
			case OT_LSTCHILDNUM:{
				printDeep(Deep+1);
				char *str=(char*)calloc(114,1);
				sprintf(str,"%%.%dlf",JsonLst->DoubleLength);
				printf(str,JsonLst->Number);
				break;
			}
			case OT_LSTCHILDBOOL:{
				printDeep(Deep+1);
				printf("%d",JsonLst->TorF);
				break;
			}
			default:printDeep(Deep+1);printf("Others,iMask: %d",JsonLst->iMask);
		}
		if(JsonLst->Next!=NULL) printf(",");
		printf("\n");
		JsonLst=JsonLst->Next;
	}
	printDeep(Deep);
	printf("%c",((ParentType==OT_CHILD)?'}':']'));
	return;
}
JSONOBJ* FindKey(int IsInParentNode, JSONOBJ* lpJsonObj, const char *Key) {
	if(lpJsonObj == NULL || Key == NULL) return NULL;
	JSONOBJ* lpFind = lpJsonObj;
	if(IsInParentNode == 1) lpFind = lpJsonObj -> Childs;
	while(lpFind != NULL) {
		if(strcmp(lpFind -> lpstrKey, Key) == 0) return lpFind;
		lpFind = lpFind -> Next;
	}
	return NULL;
}
#endif //__JSON__
