/*===================================================================
	ARcToy.h
	Copyright(c) 1999 by どんぞ(noda)  HZG02104@nifty.ne.jp
	Copy,Edit,Re-distribute FREE!

	書庫を操作するオモチャみたいなプログラム(Visual C++)
			Version 1.25	2000/01/05
			Version 1.24	1999/11/23
			Version 1.23	1999/10/11
			Version 1.22	1999/10/02
			Version 1.21	1999/09/23
			Version 1.20	1999/09/23
			Version 1.01	1999/09/07
=====================================================================*/
#if !defined(ARCTOY_H)
#define ARCTOY_H

typedef struct {
	WORD wVersion;
	char *szDLLName;
	char *szMainFunc;
	char *szArcType;
	char *szSufix;
} DLLINFO;

#define BUFFSIZE (16 * 1024)			// 16kB
#define Number(arr) (sizeof(arr) / sizeof(arr[0]))

#define S_DO_ARC		""
#define S_GET_VERSION	"GetVersion"
#define S_GET_RUNNING	"GetRunning"
#define S_CHECK_ARCHIVE "CheckArchive"
#define S_CONFIG_DIALOG "ConfigDialog"
#define S_OPEN_ARCHIVE	"OpenArchive"
#define S_CLOSE_ARCHIVE "CloseArchive"
#define S_FIND_FIRST	"FindFirst"
#define S_FIND_NEXT 	"FindNext"
#define S_GET_ARC_DATE	"GetArcDate"
#define S_GET_ARC_TIME	"GetArcTime"

typedef int  (WINAPI * ARC_DO_ARC)(const HWND, LPCSTR, LPSTR, const DWORD);
typedef WORD (WINAPI * ARC_GET_VERSION)(VOID);
typedef BOOL (WINAPI * ARC_GET_RUNNING)(VOID);
typedef BOOL (WINAPI * ARC_CHECK_ARCHIVE)(LPCSTR, const int);
typedef BOOL (WINAPI * ARC_CONFIG_DIALOG)(const HWND, LPSTR, const int);
typedef HARC (WINAPI * ARC_OPEN_ARCHIVE)(const HWND, LPCSTR, const DWORD);
typedef int  (WINAPI * ARC_CLOSE_ARCHIVE)(HARC);
typedef int  (WINAPI * ARC_FIND_FIRST)(HARC, LPCSTR, INDIVIDUALINFO *);
typedef int  (WINAPI * ARC_FIND_NEXT)(HARC, INDIVIDUALINFO *);
typedef WORD (WINAPI * ARC_GET_ARC_DATE)(HARC);
typedef WORD (WINAPI * ARC_GET_ARC_TIME)(HARC);

#endif	/* ARCTOY_H */
