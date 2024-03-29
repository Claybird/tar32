/*
	minitar
	tar32.dll test program
	by Yoshioka Tsuneo(QWF00133@nifty.ne.jp)
	Copy,Edit,Re-distribute FREE!

	Modified by ICHIMARU Takeshi(ayakawa.m@gmail.com)
*/
#include <windows.h>
#include <stdio.h>

#define FNAME_MAX32		512
typedef struct {
	DWORD 			dwOriginalSize;
	DWORD 			dwCompressedSize;
	DWORD			dwCRC;
	UINT			uFlag;
	UINT			uOSType;
	WORD			wRatio;
	WORD			wDate;
	WORD 			wTime;
	char			szFileName[FNAME_MAX32 + 1];
	char			dummy1[3];
	char			szAttribute[8];
	char			szMode[8];
}	INDIVIDUALINFO;

typedef WORD(WINAPI* fncTarGetVersion)(VOID);
typedef int (WINAPI* fncTar)(HWND hwnd, char* cmd, char* buff, int buflen);
typedef int (WINAPI* fncTarCheckArchive)(char* fname, int mode);
typedef HANDLE(WINAPI* fncTarOpenArchive)(HANDLE hwnd, char* fname, int mode);
typedef int	(WINAPI* fncTarCloseArchive)(HANDLE harc);
typedef int (WINAPI* fncTarFindFirst)(HANDLE hwnd, char* fname, INDIVIDUALINFO* pinfo);
typedef int (WINAPI* fncTarFindNext)(HANDLE hwnd, INDIVIDUALINFO* pinfo);
typedef BOOL(WINAPI* fncTarConfigDialog)(const HWND _hwnd, LPSTR _lpszComBuffer, const int _iMode);
typedef int (WINAPI* fncTarGetFileCount)(LPCSTR _szArcFile);

fncTarGetVersion TarGetVersion;
fncTar Tar;
fncTarCheckArchive TarCheckArchive;
fncTarOpenArchive TarOpenArchive;
fncTarCloseArchive TarCloseArchive;
fncTarFindFirst TarFindFirst;
fncTarFindNext TarFindNext;
fncTarConfigDialog TarConfigDialog;
fncTarGetFileCount TarGetFileCount;


void SelfTest()
{
#define TESTFILE "testfile"
	FILE *fp;
	int i;
	fp = fopen(TESTFILE,"wb");
	for(i=0;i<100;i++){
		fprintf(fp,"%d line.\n",i);
	}
	fclose(fp);

}
int main(int argc,char *argv[])
{
	char cmd[1000] = "";
	int i;
	char buff[1001];
	int ret;
	HANDLE hLib;
	int ver;

	hLib = LoadLibrary("tar32.dll");
	TarGetVersion = GetProcAddress(hLib, "TarGetVersion");
	TarCheckArchive = GetProcAddress(hLib,"TarCheckArchive");
	TarConfigDialog = GetProcAddress(hLib,"TarConfigDialog");
	Tar = GetProcAddress(hLib,"Tar");
	
	ver = TarGetVersion();
	printf("tar32.dll version <%d>\n", ver);

	for(i=1;i<argc;i++){
		strcat(cmd,argv[i]);
		strcat(cmd," ");
	}

	if(argc>=2 && strcmp(argv[1],"selftest") == 0){
		SelfTest();
	}
	if(argc>=2 && strcmp(argv[1],"config") == 0){
		TarConfigDialog(NULL,NULL,0);
	}
	if(argc==3 && (strcmp(argv[1],"check")==0 || strcmp(argv[1],"find")==0)){
		TarCheckArchive = GetProcAddress(hLib,"TarCheckArchive");
		ret = TarCheckArchive(argv[2],0);
	}
	if(argc==3 && (strcmp(argv[1],"count")==0 || strcmp(argv[1],"find")==0)){
		TarGetFileCount = GetProcAddress(hLib,"TarGetFileCount");
		ret = TarGetFileCount(argv[2]);
		printf("count=%d\n", ret);
	}
	
	if(argc>=3 && strcmp(argv[1],"find")==0){
		HANDLE harc;
		INDIVIDUALINFO info;
		char *wildname="";

		TarFindFirst = GetProcAddress(hLib,"TarFindFirst");
		TarFindNext = GetProcAddress(hLib,"TarFindNext");
		TarOpenArchive = GetProcAddress(hLib,"TarOpenArchive");
		TarCloseArchive = GetProcAddress(hLib,"TarCloseArchive");
		
		if((harc = TarOpenArchive(NULL,argv[2],0))==NULL){
			exit(1);
		}
		if(argc>=4){wildname=argv[3];}
		ret = TarFindFirst(harc,wildname,&info);
		while(ret!=-1){
			ret = TarFindNext(harc,&info);
		}
		ret = TarCloseArchive(harc);
	}
	if(0){
		/* system("command.com"); */
		TarGetFileCount = GetProcAddress(hLib,"TarGetFileCount");
		ret = TarGetFileCount("c:\\dostool\\utool\\uxtl305.tgz");
		ret = TarGetFileCount("c:\\dostool\\utool\\uxtl305.tar.bz2");
/*		ret = TarGetFileCount("c:\\dostool\\utool\\uxtl305.tgz");
		ret = TarGetFileCount("c:\\dostool\\utool\\uxtl305.tgz");*/
		//ret = TarGetFileCount("aiu.tgz");
	}
	{
		ret = Tar(NULL,cmd,buff,1000);
	}
	FreeLibrary(hLib);
	puts(buff);
	return 0;
}

