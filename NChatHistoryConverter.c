/*
********************************
This Converter is only designed 
for convert chat history version
 from v1.2.3 to v1.2.4 or from 
v1.2.7 to v1.2.8!
********************************
*/
#include <stdio.h>
#include <time.h>
#include <string.h>
int main(int argc, char *argv[]) {
	printf("Welcome to NChat History Converter v2.0\n");
	if(argc > 2) {
		FILE *lpInput = fopen(argv[1], "rb"), *lpOutput = fopen(argv[2], "wb");
		if(lpInput != NULL && lpOutput != NULL) {
			printf("Please choose an option to convert chat histroy: \n\
 1. From v1.2.3 to v1.2.4\n\
 2. From v1.2.7 to v1.2.8\n\n\
Please choose an option: ");
			int opt;
			scanf("%d", &opt);
			switch(opt) {
				case 1: {
					char option, Data[114514];
					time_t Time = time(NULL);
					while(fread(&option, sizeof(char), 1, lpInput) > 0) {
						memset(Data, 0, sizeof(Data));
						fwrite(&option, sizeof(char), 1, lpOutput);
						switch(option) {
							case '\xA':
							case '\xB': {
								fwrite(&Time, sizeof(Time), 1, lpOutput);
								fread(Data, sizeof(char), 1, lpInput);
								fwrite(Data, sizeof(char), 1, lpOutput);
								int iRead = fread(Data, sizeof(char), Data[0], lpInput);
								fwrite(Data, sizeof(char), iRead, lpOutput);
								printf("User: %s, Login\\Exit(%x)\n", Data, option);
								break;
							}
							case '\xE': {
								fwrite(&Time, sizeof(Time), 1, lpOutput);
								fread(Data, sizeof(char), 1, lpInput);
								fwrite(Data, sizeof(char), 1, lpOutput);
								int iRead = fread(Data, sizeof(char), Data[0], lpInput);
								fwrite(Data, sizeof(char), iRead, lpOutput);
								printf("User: %s, File Name: ", Data);
								fread(Data, sizeof(char), 1, lpInput);
								fwrite(Data, sizeof(char), 1, lpOutput);
								iRead = fread(Data, sizeof(char), Data[0], lpInput);
								fwrite(Data, sizeof(char), iRead, lpOutput);
								printf("%s, File Size: ", Data);
								unsigned int FileSize = 0;
								fread(&FileSize, sizeof(FileSize), 1, lpInput);
								fwrite(&FileSize, sizeof(FileSize), 1, lpOutput);
								printf("%d Bytes\n", FileSize);
								break;
							}
							case '\xF': {
								fwrite(&Time, sizeof(Time), 1, lpOutput);
								fread(Data, sizeof(char), 1, lpInput);
								fwrite(Data, sizeof(char), 1, lpOutput);
								unsigned int iRead = fread(Data, sizeof(char), Data[0], lpInput), iLen;
								fwrite(Data, sizeof(char), iRead, lpOutput);
								printf("User Name: %s, Message: ", Data);
								fread(&iLen, sizeof(iLen), 1, lpInput);
								fwrite(&iLen, sizeof(iLen), 1, lpOutput);
								int iReceived, iCount = 0;
								while(1) {
									memset(Data, 0, sizeof(char));
									iReceived = fread(Data, sizeof(char), (iLen - iCount > sizeof(Data) - 1) ? sizeof(Data) - 1 : (iLen - iCount), lpInput);
									printf("%s", Data);
									fwrite(Data, sizeof(char), iReceived, lpOutput);
									iCount += iReceived;
									if(iCount >= iLen) break;
								}
								printf("\n");
								break;
							}
						}
					}
					fclose(lpInput);
					fclose(lpOutput);
					break;
				}
				case 2: {
					char option, Data[114514];
					time_t Time = 0;
					while(fread(&option, sizeof(char), 1, lpInput) > 0) {
						memset(Data, 0, sizeof(Data));
						fwrite(&option, sizeof(char), 1, lpOutput);
						switch(option) {
							case '\xA':
							case '\xB': {
								fread(&Time, sizeof(Time), 1, lpInput);
								fwrite(&Time, sizeof(Time), 1, lpOutput);
								fread(Data, sizeof(char), 1, lpInput);
								fwrite(Data, sizeof(char), 1, lpOutput);
								int iRead = fread(Data, sizeof(char), Data[0], lpInput);
								fwrite(Data, sizeof(char), iRead, lpOutput);
								printf("User: %s, Login\\Exit(%x)\n", Data, option);
								break;
							}
							case '\xE': {
								fread(&Time, sizeof(Time), 1, lpInput);
								fwrite(&Time, sizeof(Time), 1, lpOutput);
								fread(Data, sizeof(char), 1, lpInput);
								fwrite(Data, sizeof(char), 1, lpOutput);
								int iRead = fread(Data, sizeof(char), Data[0], lpInput);
								fwrite(Data, sizeof(char), iRead, lpOutput);
								printf("User: %s, File Name: ", Data);
								fread(Data, sizeof(char), 1, lpInput);
								fwrite(Data, sizeof(char), 1, lpOutput);
								iRead = fread(Data, sizeof(char), Data[0], lpInput);
								fwrite(Data, sizeof(char), iRead, lpOutput);
								printf("%s, File Size: ", Data);
								fpos_t FileSize = 0;
								fread(&FileSize, 4, 1, lpInput);
								fwrite(&FileSize, sizeof(FileSize), 1, lpOutput);
								printf("%lld Bytes\n", FileSize);
								break;
							}
							case '\xF': {
								fread(&Time, sizeof(Time), 1, lpInput);
								fwrite(&Time, sizeof(Time), 1, lpOutput);
								fread(Data, sizeof(char), 1, lpInput);
								fwrite(Data, sizeof(char), 1, lpOutput);
								unsigned int iRead = fread(Data, sizeof(char), Data[0], lpInput), iLen;
								fwrite(Data, sizeof(char), iRead, lpOutput);
								printf("User Name: %s, Message: ", Data);
								fread(&iLen, sizeof(iLen), 1, lpInput);
								fwrite(&iLen, sizeof(iLen), 1, lpOutput);
								int iReceived, iCount = 0;
								while(1) {
									memset(Data, 0, sizeof(char));
									iReceived = fread(Data, sizeof(char), (iLen - iCount > sizeof(Data) - 1) ? sizeof(Data) - 1 : (iLen - iCount), lpInput);
									printf("%s", Data);
									fwrite(Data, sizeof(char), iReceived, lpOutput);
									iCount += iReceived;
									if(iCount >= iLen) break;
								}
								printf("\n");
								break;
							}
						}
					}
					fclose(lpInput);
					fclose(lpOutput);
					break;
				}
				default: printf("Error: Unknown option.\n");
			}
			return 0;
		}
		if(lpInput != NULL) {
			fclose(lpInput);
		}
		else printf("Error: Cannot open file '%s'.\n", argv[1]);
		if(lpOutput != NULL) {
			fclose(lpOutput);
		}
		else printf("Error: Cannot open file '%s'.\n", argv[2]);
		return 0;
	}
	printf("Error: not enough arguments.\nFormat: NCHATHISTORYCONVERTER.EXE <Full Old History Path(Input)> <Full New History Path(Output)>\n");
	return 0;
}
