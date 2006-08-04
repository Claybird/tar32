/*
	tar32.cpp
		TAR32.DLL API Implementation
		by Yoshioka Tsuneo(QWF00133@nifty.ne.jp)
*/
/*	
	���̃t�@�C���̗��p�����F
		���̃\�[�X�t�@�C���̗��p�����͈�؂���܂���B
		�\�[�X�̈ꕔ�A�S�������p�A�񏤗p�ȂǖړI��
		�������Ȃ����̃v���O�����Ŏ��R�Ɏg�p�ł��܂��B
		�p�u���b�N�E�h���C���E�\�t�g�E�F�A�Ɠ��l�Ɉ����܂��B
	
	�v���O���}�����̗v�](�����ł͂���܂���)�F
		�\�[�X���ɉ��P���ׂ��_������΂��m�点���������B
		�\�[�X�R�[�h���Ƀo�O���������ꍇ�͕񍐂��Ă��������B
		�����������Ȃǂ���܂����狳���Ă��������B
		�f�ГI�ȏ��ł����\�ł��B
		���̃t�@�C���𗘗p�����ꍇ�͂Ȃ�ׂ������Ă��������B
*/
/*
	LICENSE of this file:
		There is no restriction for using this file.
		You can use this file in your software for any purpose.
		In other words, you can use this file as Public Domain Software.

	RECOMMENDATION for Programmer(not restriction):
		If you find points to improve code, please report me.
		If you find bug in source code, please report me.
		If you fixed bug, please teach me.
		I want any trivial information.
		If you use this file, please report me.
*/
#include "tar32api.h"
#include "tarcmd.h"
#include "tar32dll.h"
#include "tar32.h"		// CTar32
#include "tar32res.h"
#include <time.h>
#include <windows.h>
#include <sys/stat.h>	// S_IWRITE
#include <crtdbg.h>		// _CrtMemCheckpoint, _CrtMemDumpAllObjectsSince


HINSTANCE dll_instance=NULL;	/* TAR32.DLL module handle */
UINT wm_arcextract;	/* Window Message */
BOOL WINAPI DllMain(HINSTANCE hInst,ULONG ul_reason_for_call,LPVOID lpReserved)
{
    switch( ul_reason_for_call ) 
    { 
        case DLL_PROCESS_ATTACH:
			dll_instance = hInst;
			Tar32InitializeCriticalSection();
			wm_arcextract = ::RegisterWindowMessage(WM_ARCEXTRACT);
            break;
        case DLL_THREAD_ATTACH:
            break;
        case DLL_THREAD_DETACH:
            break;
        case DLL_PROCESS_DETACH:
			dll_instance = NULL;
			Tar32LeaveCriticalSection();
            break;
    }
    return TRUE;
}

extern "C" WORD WINAPI _export TarGetVersion(VOID)
{
	WORD ver;
	VS_FIXEDFILEINFO *verinfo;
	HRSRC res;
	HGLOBAL hres;
	
	res = FindResource(dll_instance,MAKEINTRESOURCE(VS_VERSION_INFO),RT_VERSION);
	hres = LoadResource(dll_instance,res);
	verinfo = (VS_FIXEDFILEINFO *)((char *)LockResource(hres) + 40);
	ver = (WORD)((verinfo->dwFileVersionMS >> 16) * 100 + (verinfo->dwFileVersionMS & 0xffff));
	return ver;
}

extern "C" {
BOOL WINAPI _export TarGetRunning(VOID){return FALSE;}
BOOL WINAPI _export TarGetBackGroundMode(VOID){return FALSE;}
BOOL WINAPI _export TarSetBackGroundMode(const BOOL _BackGroundMode){return FALSE;}
static BOOL s_CursorMode;
BOOL WINAPI _export TarGetCursorMode(VOID){return s_CursorMode;}
BOOL WINAPI _export TarSetCursorMode(const BOOL _CursorMode){s_CursorMode = _CursorMode; return TRUE;}
static int s_CursorInterval;
WORD WINAPI _export TarGetCursorInterval(VOID){return s_CursorInterval;}
BOOL WINAPI _export TarSetCursorInterval(const WORD _Interval){s_CursorInterval; return TRUE;}
} // extern "C"

static HCURSOR s_orig_cursor = NULL;
static void tar32_cursor_set() // �ǂ񂼁Fvoid��ǉ�
{
	Tar32EnterCriticalSection();
	if(s_orig_cursor == NULL){
		HCURSOR hCursor = ::LoadCursor(dll_instance, MAKEINTRESOURCE(IDC_POINTER));
		s_orig_cursor = ::SetCursor(hCursor);
	}
	Tar32LeaveCriticalSection();
}
static void tar32_cursor_unset() // �ǂ񂼁Fvoid��ǉ�
{
	Tar32EnterCriticalSection();
	if(s_orig_cursor){
		::SetCursor(s_orig_cursor);
	}
	s_orig_cursor = NULL;
	Tar32LeaveCriticalSection();
}
extern "C" int WINAPI _export Tar(const HWND _hwnd, LPCSTR _szCmdLine,LPSTR _szOutput, const DWORD _dwSize){
	int ret;
	_CrtMemState memstate;
	_CrtMemCheckpoint(&memstate);

	ret = tar_cmd(_hwnd,_szCmdLine,_szOutput,_dwSize);
	
	_CrtMemDumpAllObjectsSince(&memstate);	// check memoryleaks. (the first time call happen memory-leak becaus of C/C++ runtime.)
	return ret;
}
extern "C" int WINAPI _export TarExtractMem(const HWND _hwndParent,LPCSTR _szCmdLine, LPBYTE _lpBuffer, const DWORD _dwSize,time_t *_lpTime, LPWORD _lpwAttr, LPDWORD _lpdwWriteSize){
	string cmd;
	cmd = cmd + "-p -x " + _szCmdLine;
	int ret = tar_cmd(_hwndParent,cmd.c_str(),(char*)_lpBuffer,_dwSize,(int*)_lpdwWriteSize);
	if(_lpTime){*_lpTime = 0;}
	if(_lpwAttr){*_lpwAttr = 0;}
	// return Tar(_hwndParent, cmd.c_str(), (char*)_lpBuffer, _dwSize);
	return ret;
}
extern "C" int WINAPI _export TarCompressMem(const HWND _hwndParent,LPCSTR _szCmdLine, const LPBYTE _lpBuffer, const DWORD _dwSize,const time_t *_lpTime, const LPWORD _lpwAttr,LPDWORD _lpdwWriteSize){
	string cmd;
	cmd = cmd + "-p -c " + _szCmdLine;
	return Tar(_hwndParent, cmd.c_str(), (char*)_lpBuffer, _dwSize);
}
extern "C" BOOL WINAPI _export TarCheckArchive(LPCSTR _szFileName, const int _iMode){
	// CTar32 tar32;
	// bool bret = tar32.open(_szFileName, "rb");
	// if(!bret){return FALSE;}
	// return TRUE;
	return (TarGetArchiveType(_szFileName) > 0) ? TRUE : FALSE; // �ǂ񂼁F�ύX
}
extern "C" BOOL WINAPI _export TarConfigDialog(const HWND _hwnd, LPSTR _lpszComBuffer,const int _iMode)
{
	char msg[10000]="";
	int ver = TarGetVersion();
	time_t ti;
	char fname[1000]="";
	GetModuleFileName(dll_instance,fname,sizeof(fname));
	{
		FILE *fp;
		fp=fopen(fname,"rb");
		IMAGE_DOS_HEADER idh;
		fread(&idh,1,sizeof(idh),fp);
		fseek(fp, idh.e_lfanew, SEEK_SET);
		fseek(fp,4,SEEK_CUR);// skip IMAGE_NT_SIGNATURE
		IMAGE_FILE_HEADER ifh;
		fread(&ifh,1,sizeof(ifh),fp);
		ti = ifh.TimeDateStamp;
		fclose(fp);
	}
	char tistr[1000]; strftime(tistr,1000,"%Y/%m/%d %H:%M:%S",localtime(&ti));

	sprintf(msg, "TAR32.DLL Configuration.\n"
			"ModuleFileName: %s\n"
			"TarGetVersion(): %d.%02d\n"
			"IMAGE_FILE_HEADER/TimeStamp: %d(%s)\n"
			, fname, ver/100,ver%100, ti, tistr);
	MessageBox(_hwnd, msg, "TAR32.DLL Configuration",0);
	// MessageBox(_hwnd, "There is no configuration item now.", "TAR32.DLL Configuration",0);
	return FALSE;
}
extern "C" int WINAPI _export TarGetFileCount(LPCSTR _szArcFile)
{
	HARC harc = TarOpenArchive(NULL,_szArcFile,0);
	if(harc == NULL){return -1;}
	int ret;
	int count=0;
	ret = TarFindFirst(harc,"",NULL);
	while(ret!=-1){
		ret = TarFindNext(harc,NULL);
		count++;
	}
	TarCloseArchive(harc);
	return count;
}


struct CTar32Find{
	CTar32 tar32;
};

CTar32 *HARC2PTAR32(HARC _harc)
{
	CTar32Find *pTar32Find = (CTar32Find*)_harc;
	return &(pTar32Find->tar32);
}
extern "C" HARC WINAPI _export TarOpenArchive(const HWND _hwnd, LPCSTR _szFileName,const DWORD _dwMode)
{
	CTar32Find *pTar32Find = new CTar32Find;
	bool bret = pTar32Find->tar32.open(_szFileName,"rb");
	if(!bret){delete pTar32Find;return 0;}
	return (HARC)pTar32Find;
}
extern "C" int WINAPI _export TarCloseArchive(HARC _harc)
{
	CTar32 *pTar32 = HARC2PTAR32(_harc);
	if(!pTar32){return -1;}
	bool bret = pTar32->close();
	delete pTar32;
	if(!bret)return -1;
	return 0;
}
extern "C" int WINAPI _export TarFindFirst(HARC _harc, LPCSTR _szWildName,INDIVIDUALINFO *_lpSubInfo)
{
	CTar32 *pTar32 = HARC2PTAR32(_harc);
	return TarFindNext(_harc, _lpSubInfo);
}
extern "C" int WINAPI _export TarFindNext(HARC _harc, INDIVIDUALINFO *_lpSubInfo)
{
	CTar32 *pTar32 = HARC2PTAR32(_harc);
	if(!pTar32){return -1; /*ERROR_HANDLE;*/}
	CTar32FileStatus stat;
	bool bret;
	try{
		bret = pTar32->readdir(&stat);
		if(!bret){return -1;}
		bret = pTar32->readskip();
		if(!bret){return -1;}
	}catch(CTar32Exception &e){
		return -1;
	}
	if(_lpSubInfo){
		/* convert from CTar32Status to INDIVIDUALINFO */
		_lpSubInfo->dwOriginalSize		= TarGetOriginalSize(_harc);
		_lpSubInfo->dwCompressedSize	= TarGetCompressedSize(_harc);
		_lpSubInfo->dwCRC				= TarGetCRC(_harc);
		_lpSubInfo->uFlag				= 0;
		_lpSubInfo->uOSType				= TarGetOSType(_harc);
		_lpSubInfo->wRatio				= TarGetRatio(_harc);
		_lpSubInfo->wDate				= TarGetDate(_harc);
		_lpSubInfo->wTime				= TarGetTime(_harc);
		TarGetFileName(_harc, _lpSubInfo->szFileName, sizeof(_lpSubInfo->szFileName));
		int attr						= TarGetAttribute(_harc);
		_lpSubInfo->szAttribute[0]= (attr & FA_ARCH)	? 'A' : '-'; /* Archive */
		_lpSubInfo->szAttribute[1]= (attr & FA_SYSTEM)	? 'S' : '-'; /* System */
		_lpSubInfo->szAttribute[2]= (attr & FA_HIDDEN)	? 'H' : '-'; /* Hide */
		_lpSubInfo->szAttribute[3]= (attr & FA_RDONLY)	? 'R' : '-'; /* Read Only*/
		_lpSubInfo->szAttribute[4]='\0';
		TarGetMethod(_harc, _lpSubInfo->szMode, sizeof(_lpSubInfo->szMode));
	}
	return 0;
}

extern "C" int WINAPI _export TarGetArcFileName(HARC _harc, LPSTR _lpBuffer,const int _nSize){return 0;}

extern "C" DWORD WINAPI _export TarGetArcFileSize(HARC _harc){
	return 0;}
extern "C" DWORD WINAPI _export TarGetArcOriginalSize(HARC _harc){
	return 0;}
extern "C" DWORD WINAPI _export TarGetArcCompressedSize(HARC _harc){
	return 0;}
extern "C" WORD WINAPI _export TarGetArcRatio(HARC _harc){
	return 0;}
extern "C" WORD WINAPI _export TarGetArcDate(HARC _harc){
	return 0;}
extern "C" WORD WINAPI _export TarGetArcTime(HARC _harc){
	return 0;}
/* NO IMPLEMENT:return -1: */
extern "C" UINT WINAPI _export TarGetArcOSType(HARC _harc){
	return 0;}
extern "C" int WINAPI _export TarGetFileName(HARC _harc, LPSTR _lpBuffer,const int _nSize){
	CTar32 *pTar32 = HARC2PTAR32(_harc);
	CTar32FileStatus *pstat = &pTar32->m_currentfile_status;
	strncpy(_lpBuffer, pTar32->m_currentfile_status.filename.c_str(), _nSize);
	return 0;
}
extern "C" int WINAPI _export TarGetMethod(HARC _harc, LPSTR _lpBuffer,const int _nSize){
	CTar32 *pTar32 = HARC2PTAR32(_harc);
	int type = pTar32->m_archive_type;
	bool bRet = GetARCMethod(type, _lpBuffer, _nSize);
	if(!bRet){return -1;}
	/*
	switch(type){
	case ARCHIVETYPE_NORMAL:
		strncpy(_lpBuffer, "-----",6);break;
	case ARCHIVETYPE_TAR:
		strncpy(_lpBuffer, "-tar-",6);break;
	case ARCHIVETYPE_TARGZ:
		strncpy(_lpBuffer, "-tgz-",6);break;
	case ARCHIVETYPE_TARZ:
		strncpy(_lpBuffer, "-taz-",6);break;
	case ARCHIVETYPE_GZ:
		strncpy(_lpBuffer, "- gz-",6);break;
	case ARCHIVETYPE_Z:
		strncpy(_lpBuffer, "-  z-",6);break;
	case ARCHIVETYPE_TARBZ2:
		strncpy(_lpBuffer, "-tbz-",6);break;
	case ARCHIVETYPE_BZ2:
		strncpy(_lpBuffer, "-bz2-",6);break;
	default:
		strncpy(_lpBuffer, "-err-",6);break;
	}
	*/
	return 0;
}
extern "C" BOOL WINAPI _export TarGetOriginalSizeEx(HARC _harc, __int64 *_lpllSize)
{
	CTar32 *pTar32 = HARC2PTAR32(_harc);
	CTar32FileStatus *pstat = &(pTar32->m_currentfile_status);
	// �T�C�Y���s���̏ꍇ�� 0��Ԃ�
	if(pstat->original_size == -1){
		*_lpllSize = 0;
		return 0;
	}
	*_lpllSize = pstat->original_size;
	return TRUE;
}
extern "C" DWORD WINAPI _export TarGetOriginalSize(HARC _harc)
{
	__int64 size = 0;
	TarGetOriginalSizeEx(_harc, &size);
	return (DWORD)size;
}
extern "C" BOOL WINAPI _export TarGetCompressedSizeEx(HARC _harc, __int64 *_lpllSize)
{
	CTar32 *pTar32 = HARC2PTAR32(_harc);
	CTar32FileStatus *pstat = &(pTar32->m_currentfile_status);
	*_lpllSize = pstat->compress_size;
	return TRUE;
}
extern "C" DWORD WINAPI _export TarGetCompressedSize(HARC _harc)
{
	__int64 size = 0;
	TarGetCompressedSizeEx(_harc, &size);
	return (DWORD)size;
}

extern "C" WORD WINAPI _export TarGetRatio(HARC _harc)
{
	if(TarGetOriginalSize(_harc)){
		return 1000*TarGetCompressedSize(_harc)/TarGetOriginalSize(_harc);
	}
	return 0;
}
extern "C" WORD WINAPI _export TarGetDate(HARC _harc)
{
	CTar32 *pTar32 = HARC2PTAR32(_harc);
	CTar32FileStatus *pstat = &(pTar32->m_currentfile_status);
	return GetARCDate(pstat->mtime);
}
extern "C" WORD WINAPI _export TarGetTime(HARC _harc)
{
	CTar32 *pTar32 = HARC2PTAR32(_harc);
	CTar32FileStatus *pstat = &(pTar32->m_currentfile_status);
	return GetARCTime(pstat->mtime);
}

extern "C" DWORD WINAPI _export TarGetWriteTime(HARC _harc)
{
	CTar32 *pTar32 = HARC2PTAR32(_harc);
	CTar32FileStatus *pstat = &(pTar32->m_currentfile_status);
	return pstat->mtime;
}
extern "C" DWORD WINAPI _export TarGetAccessTime(HARC _harc)
{
	CTar32 *pTar32 = HARC2PTAR32(_harc);
	CTar32FileStatus *pstat = &(pTar32->m_currentfile_status);
	return pstat->atime;
}
extern "C" DWORD WINAPI _export TarGetCreateTime(HARC _harc)
{
	CTar32 *pTar32 = HARC2PTAR32(_harc);
	CTar32FileStatus *pstat = &(pTar32->m_currentfile_status);
	return pstat->ctime;
}

extern "C" DWORD WINAPI _export TarGetCRC(HARC _harc)
{
	CTar32 *pTar32 = HARC2PTAR32(_harc);
	CTar32FileStatus *pstat = &(pTar32->m_currentfile_status);
	return pstat->chksum;
}
extern "C" int WINAPI _export TarGetAttribute(HARC _harc)
{
	CTar32 *pTar32 = HARC2PTAR32(_harc);
	CTar32FileStatus *pstat = &(pTar32->m_currentfile_status);
	int st_mode = pstat->mode;
	return 
		((st_mode & S_IWRITE)==0)*FA_RDONLY /* Read Only */
		+((st_mode & S_IREAD)==0)*FA_HIDDEN /* Hidden */
		+ 0 *FA_SYSTEM						/* System File */
		+ 0 *FA_LABEL						/* volume label */
		+((st_mode & S_IFDIR)!=0)*FA_DIREC /* Directory */
		+0*FA_ARCH;							/* Archive Attribute */
}

UINT WINAPI _export TarGetOSType(HARC _harc){return 0;}

// BOOL WINAPI _export TarQueryFunctionList(const int _iFunction);

HWND g_hwndOwnerWindow = NULL;
ARCHIVERPROC *g_pArcProc = NULL;
extern "C" BOOL WINAPI _export TarSetOwnerWindow(const HWND _hwnd)
{
	g_hwndOwnerWindow = _hwnd;
	return TRUE;
}
extern "C" BOOL WINAPI _export TarClearOwnerWindow(void)
{
	g_hwndOwnerWindow = NULL;
	return TRUE;
}
extern "C" BOOL WINAPI _export TarSetOwnerWindowEx(HWND _hwnd,ARCHIVERPROC *_lpArcProc)
{
	g_hwndOwnerWindow = _hwnd;
	g_pArcProc = _lpArcProc;
	return TRUE;
}
extern "C" BOOL WINAPI _export TarKillOwnerWindowEx(HWND _hwnd)
{
	g_hwndOwnerWindow = NULL;
	g_pArcProc = NULL;
	return TRUE;
}

extern "C" int WINAPI _export TarGetArchiveType(LPCSTR _szFileName)
{
	int ret;
	_CrtMemState memstate;
	_CrtMemCheckpoint(&memstate);

	ret =  CTar32::s_get_archive_type(_szFileName);
	
	_CrtMemDumpAllObjectsSince(&memstate);	// check memoryleaks. (the first time call happen memory-leak becaus of C/C++ runtime.)
	return ret;
}

extern "C" BOOL WINAPI _export TarQueryFunctionList(const int _iFunction)
{
	switch( _iFunction){
	case ISARC:
	case ISARC_GET_VERSION:
	//case ISARC_GET_CURSOR_INTERVAL:
	//case ISARC_SET_CURSOR_INTERVAL:
	//case ISARC_GET_BACK_GROUND_MODE:
	//case ISARC_SET_BACK_GROUND_MODE:
	//case ISARC_GET_CURSOR_MODE:
	//case ISARC_SET_CURSOR_MODE:
	case ISARC_GET_RUNNING:

	case ISARC_CHECK_ARCHIVE:
	case ISARC_CONFIG_DIALOG:
	case ISARC_GET_FILE_COUNT:
	case ISARC_QUERY_FUNCTION_LIST:
	//case ISARC_HOUT:
	//case ISARC_STRUCTOUT:         //
	//case ISARC_GET_ARC_FILE_INFO: // �ǂ񂼁F�R�����g�ɂ����B

	case ISARC_OPEN_ARCHIVE:
	case ISARC_CLOSE_ARCHIVE:
	case ISARC_FIND_FIRST:
	case ISARC_FIND_NEXT:
	//case ISARC_EXTRACT:
	//case ISARC_ADD:
	//case ISARC_MOVE:
	//case ISARC_DELETE:
	case ISARC_SETOWNERWINDOW:			/* UnlhaSetOwnerWindow */
	case ISARC_CLEAROWNERWINDOW:		/* UnlhaClearOwnerWindow */
	case ISARC_SETOWNERWINDOWEX:			/* UnlhaSetOwnerWindowEx */
	case ISARC_KILLOWNERWINDOWEX:		/* UnlhaKillOwnerWindowEx */

	//case ISARC_GET_ARC_FILE_NAME:
	//case ISARC_GET_ARC_FILE_SIZE:
	//case ISARC_GET_ARC_ORIGINAL_SIZE:
	//case ISARC_GET_ARC_COMPRESSED_SIZE:
	//case ISARC_GET_ARC_RATIO:
	//case ISARC_GET_ARC_DATE:
	//case ISARC_GET_ARC_TIME:
	//case ISARC_GET_ARC_OS_TYPE:
	//case ISARC_GET_ARC_IS_SFX_FILE:
	case ISARC_GET_FILE_NAME:
	case ISARC_GET_ORIGINAL_SIZE:
	case ISARC_GET_COMPRESSED_SIZE:
	case ISARC_GET_RATIO:
	case ISARC_GET_DATE:
	case ISARC_GET_TIME:
	case ISARC_GET_CRC:
	case ISARC_GET_ATTRIBUTE:
	case ISARC_GET_OS_TYPE:
	case ISARC_GET_METHOD:
	case ISARC_GET_WRITE_TIME:
	case ISARC_GET_CREATE_TIME:
	case ISARC_GET_ACCESS_TIME:

	case ISARC_GET_ORIGINAL_SIZE_EX:
	case ISARC_GET_COMPRESSED_SIZE_EX:

		return TRUE;
	default:
		return FALSE;
	}
}

/********************************************************
	rundll32 interface for execute Tar() function from command line
	  command line:
	     rundll32.exe tar32.dll,TarCommandLine <command>
		 ex) rundll32.exe tar32.dll,TarCommandLine  -xvf foo.tar.gz
      reference:
		[SDK32] Rundll �� Rundll32 �C���^�t�F�[�X
		http://www.microsoft.com/mscorp/worldwide/japan/support/kb/articles/j042/9/53.htm
********************************************************/
extern "C" void WINAPI/*CALLBACK*/ TarCommandLine(HWND hwnd, HINSTANCE hinst, LPSTR lpszCmdLine, int nCmdShow)
{
	char buf[64000];
	int iRet = Tar(hwnd,lpszCmdLine,buf,sizeof(buf));
}



#if 0
/*********************************************************
	 Susie Plug-In APIs 
**********************************************************/
extern "C" int WINAPI _export GetPluginInfo(int infono, LPSTR buf,int buflen)
{
	int func_ret=0;
	if(infono==0){
		memcpy(buf,"00AM",func_ret=min(5,buflen));
	}else if(infono==1){
		const char *plugin_name = "Tar32.DLL by Yoshioka Tsuneo(QWF00133@nifty.ne.jp)";
		strncpy(buf,plugin_name,func_ret = min(buflen-1, strlen(plugin_name))+1);
	}else{
		const char *ext_names[] = {"*.tar;*.tgz;*.tbz;*.gz:*.bz2"};
		const char *fmt_names[] = {"tar/gz/bz2 format"};
		int ext_nums = sizeof(ext_names)/sizeof(ext_names[0]);
		infono -= 2;
		if(infono >= ext_nums*2){return 0;}
		if(infono%2 == 0){
			strncpy(buf,ext_names[infono/2],func_ret = min(buflen-1, strlen(ext_names[infono/2]))+1);
		}else{
			strncpy(buf,fmt_names[infono/2],func_ret = min(buflen-1, strlen(fmt_names[infono/2]))+1);
		}
	}
	return func_ret;
}
extern "C" int WINAPI _export IsSupported(LPSTR filename,DWORD dw)
{
	char buf[2000];
	BOOL bRet;
	if((dw && 0xffff0000)==0){
		HANDLE hFile = (HANDLE) dw;
		DWORD n;
		bRet = ReadFile(hFile, buf, sizeof(buf), &n, NULL);
		if(!bRet){return 0;}
	}else{
		memcpy(buf, (void*)dw, 2000);
	}
	FILE *fp = fopen("C:\\tar_spi.tmp","wb");
	if(fp==NULL){return 0;}
	fwrite(buf,1,sizeof(buf),fp);
	fclose(fp);
	bRet = TarCheckArchive("c:\\tar_spi.tmp",0);
	unlink("c:\\tar_spi.tmp");
	return bRet;
}
typedef struct
{
    unsigned char method[8];    //���k�@�̎��
    unsigned long position;     //�t�@�C����ł̈ʒu
    unsigned long compsize;     //���k���ꂽ�T�C�Y
    unsigned long filesize;     //���̃t�@�C���T�C�Y
    time_t timestamp;           //�t�@�C���̍X�V����
    char path[200];             //���΃p�X
    char filename[200];         //�t�@�C���l�[��
    unsigned long crc;          //CRC
} fileInfo;

extern "C" int WINAPI _export GetArchiveInfo(LPSTR buf,long len, unsigned int flag,HLOCAL *lphInf)
{
	if((flag & 7)!=0){
		// FileImage Pointer not supported.
		return -1;
	}
	int count = TarGetFileCount(buf);
	HLOCAL hlocal = LocalAlloc(0,(count+1)*sizeof(fileInfo));
	LocalLock(hlocal);
	if(!hlocal){return NULL;}

	HARC harc = TarOpenArchive(NULL,buf,0);
	INDIVIDUALINFO iinfo;
	fileInfo finfo;
	int pos = 0;
	int i = 0;
	int ret = TarFindFirst(harc,"*",&iinfo);
	while(ret != -1){
		memset(&finfo,0,sizeof(finfo));
		memcpy(finfo.method, iinfo.szMode, 8);
		finfo.position = pos;
		finfo.compsize = iinfo.dwCompressedSize;
		finfo.filesize = iinfo.dwOriginalSize;
		finfo.timestamp = 0; //iinfo.wData + iinfo.wTime;
		memcpy(finfo.filename, iinfo.szFileName, 200);
		finfo.crc = iinfo.dwCRC;
		pos += finfo.compsize;
		((fileInfo*)hlocal)[i++] = finfo;
		ret = TarFindNext(harc,&iinfo);
	}
	TarCloseArchive(harc);
	((fileInfo*)hlocal)[i++].method[0] = '\0';
	*lphInf = hlocal;
	LocalUnlock(hlocal);
	return 0;
}
extern "C" int WINAPI _export GetFileInfo(LPSTR buf,long len, LPSTR filename, unsigned int flag,fileInfo *lpInfo)
{
	HLOCAL hlocals;
	int func_ret = -1;
	int ret = GetArchiveInfo(buf, len, flag, &hlocals);
	if(ret != 0){return -1;}
	fileInfo *finfos = (fileInfo*)LocalLock(hlocals);
	while(finfos->method[0] != '\0'){
		if(stricmp(filename,finfos->filename) == 0){
			*lpInfo = *finfos;
			func_ret = 0;
			break;
		}
		finfos++;
	}
	LocalUnlock(hlocals);
	LocalFree(hlocals);
	return func_ret;
}
extern "C" int WINAPI _export GetFile(LPSTR src,long len, LPSTR dest,unsigned int flag, FARPROC prgressCallback,long lData)
{
	if((flag & 0x7)==1){return -1;} // input must be file
	if(len != 0){return -1;} // offset
	if((flag & 0x700)==0){	// output is file
		char outbuf[1000];
		char cmd[1000];
		char *file = src + strlen(src) + 1;
		sprintf(cmd, "--use-directory=0  -o \"%s\" -x \"%s\" \"%s\"", dest, src, file);
		int ret = Tar(NULL,cmd,outbuf,sizeof(outbuf));
		return 0;
	}else{	// output is memory
		unsigned char outbuf[100000];
		char cmd[1000];
		char *file = src + strlen(src) + 1;
		sprintf(cmd, "--use-directory=0  -o \"%s\" -x \"%s\" \"%s\"", dest, src, file);
		unsigned long WriteSize = 0;
		int ret = TarExtractMem(NULL,cmd,outbuf,sizeof(outbuf),NULL,NULL,&WriteSize);
		if(ret!=0){return -1;}

		HLOCAL hlocal = LocalAlloc(0, WriteSize);
		LocalLock(hlocal);
		memcpy((void*)hlocal,outbuf,WriteSize);
		LocalUnlock(hlocal);
		*((HLOCAL*)dest) = hlocal;
		return 0;
	}
	return 0;
}

#endif /* if 0 */




