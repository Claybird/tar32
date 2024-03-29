/*	
	このファイルの利用条件：
		このソースファイルの利用制限は一切ありません。
		ソースの一部、全部を商用、非商用など目的に
		かかわりなく他のプログラムで自由に使用できます。
		パブリック・ドメイン・ソフトウェアと同様に扱えます。
	
	プログラマ向けの要望(制限ではありません)：
		ソース中に改善すべき点があればお知らせください。
		ソースコード中にバグを見つけた場合は報告してください。
		直した部分などありましたら教えてください。
		断片的な情報でも結構です。
		このファイルを利用した場合はなるべく教えてください。
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
#include "stdafx.h"
#include "tar32dll.h"
#include "tar32api.h"

static CRITICAL_SECTION g_tar32_critsec;
void Tar32InitializeCriticalSection()
{
	InitializeCriticalSection(&g_tar32_critsec);
}
void Tar32DeleteCriticalSection()
{
	DeleteCriticalSection(&g_tar32_critsec);
}
void Tar32EnterCriticalSection()
{
	EnterCriticalSection(&g_tar32_critsec);
}
void Tar32LeaveCriticalSection()
{
	LeaveCriticalSection(&g_tar32_critsec);
}

WORD GetARCDate(time_t ti)
{
	struct tm tm,*tmptr;
	
	if(ti==0){
		memset(&tm,0,sizeof(struct tm));
		tm.tm_year = 1980 - 1900;
		tm.tm_mday = 1;
		tmptr = &tm;
	}else{
		tmptr=localtime(&ti);
	}
	return (((unsigned)tmptr->tm_year-(unsigned)80)<<9)+((tmptr->tm_mon+1)<<5)+(tmptr->tm_mday);
}
WORD GetARCTime(time_t ti)
{
	struct tm *tmptr;
		
	if(ti==0){return 0;}
	tmptr = localtime(&ti);
	return ((tmptr->tm_hour)<<11)+((tmptr->tm_min)<<5)+((tmptr->tm_sec)/2);
}
DWORD GetARCAttribute(int st_mode)
{
	return ((st_mode & S_IWRITE)==0)*FA_RDONLY /* Read Only */
		+((st_mode & S_IREAD)==0)*FA_HIDDEN /* Hidden */
		+ 0 *FA_SYSTEM						/* System File */
		+ 0 *FA_LABEL						/* volume label */
		+((st_mode & S_IFDIR)!=0)*FA_DIREC /* Directory */
		+0*FA_ARCH;							/* Archive Attribute */
}

void GetARCAttribute(int st_mode, char *buf, int buflen)
{
	int attr=GetARCAttribute(st_mode);

	buf[0]= (attr & FA_ARCH)	? 'A' : '-'; /* Archive */
	buf[1]= (attr & FA_SYSTEM)	? 'S' : '-'; /* System */
	buf[2]= (attr & FA_HIDDEN)	? 'H' : '-'; /* Hide */
	buf[3]= (attr & FA_RDONLY)	? 'R' : '-'; /* Read Only*/
	buf[4]='\0';
}
bool GetARCMethod(int archive_type, char *buf, int buf_len)
{
	if(buf_len<6){return false;}
	switch(archive_type){
	case ARCHIVETYPE_NORMAL:
		strncpy(buf, "-----",buf_len);break;
	case ARCHIVETYPE_TAR:
		strncpy(buf, "-tar-",buf_len);break;
	case ARCHIVETYPE_TARGZ:
		strncpy(buf, "-tgz-",buf_len);break;
	case ARCHIVETYPE_TARZ:
		strncpy(buf, "-taz-",buf_len);break;
	case ARCHIVETYPE_GZ:
		strncpy(buf, "- gz-",buf_len);break;
	case ARCHIVETYPE_Z:
		strncpy(buf, "-  z-",buf_len);break;
	case ARCHIVETYPE_TARBZ2:
		strncpy(buf, "-tbz-",buf_len);break;
	case ARCHIVETYPE_BZ2:
		strncpy(buf, "-bz2-",buf_len);break;
	case ARCHIVETYPE_TARLZMA:
		strncpy(buf, "-tlz-",buf_len);break;
	case ARCHIVETYPE_LZMA:
		strncpy(buf, "-lzm-",buf_len);break;
	case ARCHIVETYPE_TARXZ:
		strncpy(buf, "-txz-",buf_len);break;
	case ARCHIVETYPE_XZ:
		strncpy(buf, "- xz-",buf_len);break;
	case ARCHIVETYPE_TARZSTD:
		strncpy(buf, "tzstd", buf_len); break;
	case ARCHIVETYPE_ZSTD:
		strncpy(buf, "-zst-", buf_len); break;


	case ARCHIVETYPE_CPIO:
		strncpy(buf, "cpio",buf_len);break;
	case ARCHIVETYPE_CPIOGZ:
		strncpy(buf, "cpio.gz",buf_len);break;
	case ARCHIVETYPE_CPIOZ:
		strncpy(buf, "cpio.z",buf_len);break;
	case ARCHIVETYPE_CPIOBZ2:
		strncpy(buf, "cpiobz2",buf_len);break;
	case ARCHIVETYPE_CPIOLZMA:
		strncpy(buf, "cpiolzm",buf_len);break;
	case ARCHIVETYPE_CPIOXZ:
		strncpy(buf, "cpio.xz",buf_len);break;
	case ARCHIVETYPE_CPIOZSTD:
		strncpy(buf, "cpiozst", buf_len); break;

	case ARCHIVETYPE_AR:
		strncpy(buf, "ar",buf_len);break;
	case ARCHIVETYPE_ARGZ:
		strncpy(buf, "ar.gz",buf_len);break;
	case ARCHIVETYPE_ARZ:
		strncpy(buf, "ar.z",buf_len);break;
	case ARCHIVETYPE_ARBZ2:
		strncpy(buf, "ar.bz2",buf_len);break;
	case ARCHIVETYPE_ARLZMA:
		strncpy(buf, "ar.lzma",buf_len);break;
	case ARCHIVETYPE_ARXZ:
		strncpy(buf, "ar.xz",buf_len);break;
	case ARCHIVETYPE_ARZSTD:
		strncpy(buf, "ar.zstd", buf_len); break;

		
	default:
		strncpy(buf, "-err-",buf_len);break;
	}
	return true;
}

void TimetToFileTime(time_t t, LPFILETIME pft)
{
	ULARGE_INTEGER time_value;
	time_value.QuadPart = (t * 10000000LL) + 116444736000000000LL;
	pft->dwLowDateTime = time_value.LowPart;
	pft->dwHighDateTime = time_value.HighPart;
}

#ifdef UNIT_TEST
int wmain(int argc, wchar_t* argv[], wchar_t* envp[]) {
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
#endif
