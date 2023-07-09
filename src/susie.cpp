/* Special Thanks for patch to Jy. (jyz@cds.ne.jp) (2000/07/10)  */
/*
  TAR32.DLL version.2.xx
  Susie 32bit Plug-in Spec Rev4 Interface.
*/
/*
  TAR64.DLL��.sph�Ή�(�A��W�nAPI�͔�Ή�) - TORO����iftwic�̃\�[�X���Q��( http://toro.d.dooo.jp/ )
    ICHIMARU Takeshi ( ayakawa.m@gmail.com )
*/
#include "stdafx.h"

#include "tar32api.h"
#include "tar32dll.h"

/*********************************************************
	 Susie Plug-In APIs 
**********************************************************/

typedef ULONG_PTR susie_time_t;
#pragma pack(push,1)
typedef struct
{
	unsigned char	method[8];		/*���k�@�̎��*/
	ULONG_PTR		position;		/*�t�@�C����ł̈ʒu*/
	ULONG_PTR		compsize;		/*���k���ꂽ�T�C�Y*/
	ULONG_PTR		filesize;		/*���̃t�@�C���T�C�Y*/
	susie_time_t	timestamp;		/*�t�@�C���̍X�V����*/
	char			path[200];		/*���΃p�X*/
	char			filename[200];	/*�t�@�C���l�[��*/
	unsigned long	crc;			/*CRC*/
#ifdef _WIN64
	char			dummy[4];
#endif
} fileInfo;
#pragma pack(pop)

extern "C" int WINAPI _export GetPluginInfo(int infono, LPSTR buf,int buflen)
{
	int nRet=0;

	if(0==infono) {
		memcpy(buf,"00AM",nRet=min(buflen,5));
	} else if(1==infono) {
#ifdef _WIN64
		const char* pPluginName = "Tar64.DLL by Yoshioka Tsuneo(QWF00133@nifty.ne.jp) & ICHIMARU Takeshi(ayakawa.m@gmail.com)";
#else
		const char *pPluginName="Tar32.DLL by Yoshioka Tsuneo(QWF00133@nifty.ne.jp)";
#endif
		memcpy(buf,pPluginName,nRet=min((unsigned int)buflen,strlen(pPluginName)+1));
	} else {
		const char *ppExtNames[]={"*.tar;*.tgz;*.tbz;*.txz;*.tlz;*.tzst;*.gz;*.bz2;*.xz;*.lz;*.lzma;*.Z;*.zst;*.zstd"};
		const char *ppFmtNames[]={"tar/gz/bz2/Z/xz/lzma/zstd format"};
		int nExtNum=sizeof(ppExtNames)/sizeof(ppExtNames[0]);
		infono-=2;
		if(infono>=nExtNum*2) {
			return 0;
		}
		if(0==(infono&1)) {
			memcpy(buf,ppExtNames[infono>>1],nRet=min((unsigned int)buflen,strlen(ppExtNames[infono>>1])+1));
		} else {
			memcpy(buf,ppFmtNames[infono>>1],nRet=min((unsigned int)buflen,strlen(ppFmtNames[infono>>1])+1));
		}
	}

	return nRet;
}


extern "C" int WINAPI _export IsSupported(LPSTR filename, VOID * dw)
{
	char szBuf[2000];
	int nRet;

	nRet=1;
#ifdef _WIN64
	if ((DWORD_PTR)dw & ~(DWORD_PTR)0xffff) {
		// 2K �������C���[�W
		memcpy(szBuf, (void*)dw, 2000);
	}
	else {
		DWORD size;
		if (ReadFile(reinterpret_cast<HANDLE>(dw), szBuf, sizeof(szBuf), &size, NULL) == FALSE) {
			nRet=0; // �ǂݍ��ݎ��s
		}
	}
#else
	if(0==(HIWORD(dw))) {
		DWORD n;
		if(!ReadFile((HANDLE)dw,szBuf,sizeof(szBuf),&n,NULL)) {
			nRet=0;
		}
	} else {
		memcpy(szBuf,(void*)dw,2000);
	}
#endif

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

extern "C" int WINAPI _export GetArchiveInfo(LPSTR buf,LONG_PTR len, unsigned int flag,HLOCAL *lphInf)
{
	int nCnt,nPos,nStatus;
	HARC hArc;
	fileInfo *pInf;
	HLOCAL hInf;
	INDIVIDUALINFO iInfo;
#ifdef _WIN64
	__int64 t;
#endif

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
#ifdef _WIN64
		if (TarGetWriteTime64(hArc, &t)) pInf->timestamp = (time_t)t;
#endif
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
extern "C" int WINAPI _export GetFileInfo(LPSTR buf,LONG_PTR len, LPSTR filename, unsigned int flag,fileInfo *lpInfo)
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

extern "C" int WINAPI _export GetFile(LPSTR src,LONG_PTR len, LPSTR dest,unsigned int flag, FARPROC prgressCallback,LONG_PTR lData)
{
	int nRet;
	HANDLE hInf;
	fileInfo *pInf;
	char szCmd[1000],szName[MAX_PATH];
	unsigned long nSize;

	if(0!=(flag&0x0007)) {
		return -1;						// input must be file
	}


	nRet = -1;
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

