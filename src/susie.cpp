/* Special Thanks for patch to Jy. (jyz@cds.ne.jp) (2000/07/10)  */
/*
  TAR32.DLL version.2.xx
  Susie 32bit Plug-in Spec Rev4 Interface.
*/

#include "tar32api.h"
#include "tar32dll.h"
#include <time.h>
#include <windows.h>

/*********************************************************
	 Susie Plug-In APIs 
**********************************************************/

#pragma pack(push,1)
typedef struct
{
	unsigned char method[8];	/*圧縮法の種類*/
	unsigned long position;		/*ファイル上での位置*/
	unsigned long compsize;		/*圧縮されたサイズ*/
	unsigned long filesize;		/*元のファイルサイズ*/
	time_t timestamp;		/*ファイルの更新日時*/
	char path[200];			/*相対パス*/
	char filename[200];		/*ファイルネーム*/
	unsigned long crc;		/*CRC*/
} fileInfo;
#pragma pack(pop)

extern "C" int WINAPI _export GetPluginInfo(int infono, LPSTR buf,int buflen)
{
	int nRet=0;

	if(0==infono) {
		memcpy(buf,"00AM",nRet=min(buflen,5));
	} else if(1==infono) {
		const char *pPluginName="Tar32.DLL by Yoshioka Tsuneo(QWF00133@nifty.ne.jp)";
		memcpy(buf,pPluginName,nRet=min(buflen,strlen(pPluginName)+1));
	} else {
		const char *ppExtNames[]={"*.tar;*.tgz;*.tbz;*.gz;*.bz2;*.Z;"};
		const char *ppFmtNames[]={"tar/gz/bz2/Z format"};
		int nExtNum=sizeof(ppExtNames)/sizeof(ppExtNames[0]);
		infono-=2;
		if(infono>=nExtNum*2) {
			return 0;
		}
		if(0==(infono&1)) {
			memcpy(buf,ppExtNames[infono>>1],nRet=min(buflen,strlen(ppExtNames[infono>>1])+1));
		} else {
			memcpy(buf,ppFmtNames[infono>>1],nRet=min(buflen,strlen(ppFmtNames[infono>>1])+1));
		}
	}

	return nRet;
}

extern "C" int WINAPI _export IsSupported(LPSTR filename,DWORD dw)
{
	char szBuf[2000];
	int nRet;

	nRet=1;
	if(0==(HIWORD(dw))) {
		DWORD n;
		if(!ReadFile((HANDLE)dw,szBuf,sizeof(szBuf),&n,NULL)) {
			nRet=0;
		}
	} else {
		memcpy(szBuf,(void*)dw,2000);
	}

	if(1==nRet) {
		HANDLE hFile;
		char szTmpDir[MAX_PATH],szTmpFile[MAX_PATH];
		GetTempPath(MAX_PATH,szTmpDir);
		GetTempFileName(szTmpDir,"TAR",0,szTmpFile);
		if(INVALID_HANDLE_VALUE==(hFile=CreateFile(szTmpFile,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL))) {
			nRet=0;
		} else {
			DWORD nWrite;
			WriteFile(hFile,szBuf,sizeof(szBuf),&nWrite,NULL);
			CloseHandle(hFile);
			nRet=TarCheckArchive(szTmpFile,0);
			DeleteFile(szTmpFile);
		}
	}

	return nRet;
}

extern "C" int WINAPI _export GetArchiveInfo(LPSTR buf,long len, unsigned int flag,HLOCAL *lphInf)
{
	int nCnt,nPos,nStatus;
	HARC hArc;
	fileInfo *pInf;
	HLOCAL hInf;
	INDIVIDUALINFO iInfo;

	if(0!=(flag&0x0007)) {
		// FileImage Pointer not supported.
		return -1;
	}

	nCnt=TarGetFileCount(buf);
	if(NULL==(hInf=LocalAlloc(LHND,(nCnt+1)*sizeof(fileInfo)))) {
		return 4;
	}
	if(NULL==(pInf=(fileInfo*)LocalLock(hInf))) {
		return 5;
	}

	if(NULL==(hArc=TarOpenArchive(NULL,buf,0))) {
		LocalUnlock(hInf);
		LocalFree(hInf);
		return -1;
	}

	nPos=0;
	nStatus=TarFindFirst(hArc,"*",&iInfo);
	while(-1!=nStatus) {
		memset(pInf,0,sizeof(fileInfo));
		memcpy(pInf->method,iInfo.szMode,8);
		pInf->position=nPos++;
		pInf->compsize=iInfo.dwCompressedSize;
		pInf->filesize=iInfo.dwOriginalSize;
		pInf->timestamp=0; //iinfo.wData + iinfo.wTime;
		memcpy(pInf->filename,iInfo.szFileName,200);
		pInf->crc=iInfo.dwCRC;
		pInf++;
		nStatus=TarFindNext(hArc,&iInfo);
	}
	TarCloseArchive(hArc);
	LocalUnlock(hInf);
	*lphInf=hInf;

	return 0;
}
extern "C" int WINAPI _export GetFileInfo(LPSTR buf,long len, LPSTR filename, unsigned int flag,fileInfo *lpInfo)
{
	int nRet;
	HANDLE hInf;
	fileInfo *pInf;
	int (*CompareFunction)(const char*,const char*);

	CompareFunction=((0x0080==(flag&0x0080))?stricmp:strcmp);

	nRet=8;
	GetArchiveInfo(buf,0,flag,&hInf);
	pInf=(fileInfo*)GlobalLock(hInf);
	while('\0'!=pInf->method[0]) {
		if(0==((CompareFunction)(pInf->filename,filename))) {
			memcpy(lpInfo,pInf,sizeof(fileInfo));
			nRet=0;
			break;
		}
		pInf++;
	}
	GlobalUnlock(hInf);
	GlobalFree(hInf);

	return nRet;
}

extern "C" int WINAPI _export GetFile(LPSTR src,long len, LPSTR dest,unsigned int flag, FARPROC prgressCallback,long lData)
{
	int nRet;
	HANDLE hInf;
	fileInfo *pInf;
	char szCmd[1000],szName[MAX_PATH];
	unsigned long nSize;

	if(0!=(flag&0x0007)) {
		return -1;						// input must be file
	}

	GetArchiveInfo(src,0,flag,&hInf);
	pInf=(fileInfo*)GlobalLock(hInf);
	while('\0'!=pInf->method[0]) {
		if((unsigned long)pInf->position==(unsigned long)len) {
			nSize=pInf->filesize;
			strcpy(szName,pInf->filename);
			nRet=0;
			break;
		}
		pInf++;
	}
	GlobalUnlock(hInf);
	GlobalFree(hInf);
	if(0!=nRet) {
		return -1;
	}

	nRet=8;
	if(0x0000==(flag&0x0700)) {			// output is file
		char szBuf[1000];
		wsprintf(szCmd,"--display-dialog=0 --use-directory=0 -o \"%s\" -x \"%s\" \"%s\"",dest,src,szName);
		nRet=Tar(NULL,szCmd,szBuf,sizeof(szBuf));
	} else if(0x0100==(flag&0x0700)) {	// output is memory
		unsigned long nWrite=0;
		HANDLE hBuf;
		void *pBuf;
		wsprintf(szCmd,"--display-dialog=0 --use-directory=0 \"%s\" \"%s\"",src,szName);
		if(NULL==(hBuf=LocalAlloc(LHND,nSize+1))) {
			nRet=4;
		} else {
			if(NULL==(pBuf=LocalLock(hBuf))) {
				LocalFree(hBuf);
				nRet=5;
			} else {
				nRet=TarExtractMem(NULL,szCmd,(unsigned char*)pBuf,nSize,NULL,NULL,&nWrite);
				LocalUnlock(hBuf);
				*((HANDLE*)dest)=hBuf;
				nRet=0;
			}
		}
	} else {
		nRet=-1;
	}

	return nRet;
}

