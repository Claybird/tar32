/*===================================================================
	ARcToy.cpp
	Copyright(c) 1999-2000 by どんぞ(noda)  HZG02104@nifty.ne.jp
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
#include <stdio.h>
#include "comm-arc.h"
#include "ARcToy.h"

/* ＤＬＬ識別用 */
HINSTANCE hDLL = NULL;
int nDLL	   = 0;

/* 利用可能なＤＬＬを登録 */
DLLINFO DLLInfo[] = {
	/* {0, "UNLHA32.DLL"	, "Unlha"	, "LHA"	, ".lzh;.lzs"						}, */
	/* {0, "UNARJ32J.DLL"	, "Unarj"	, "ARJ"	, ".arj"							}, */
	/* {0, "ISH32.DLL"		, "Ish"		, "ISH"	, ".ish"							}, */
	/* {0, "UNZIP32.DLL"	, "UnZip"	, "ZIP"	, ".zip;.jar"						}, */
	{0, "ZIP32J.DLL"	, "Zip"		, "ZIP"	, ".zip"							},
	/* {0, "CAB32.DLL"		, "Cab"		, "CAB"	, ".cab"							}, */
	/* {0, "UNRAR32.DLL"	, "Unrar"	, "RAR"	, ".rar"							}, */
	/* {0, "BGA32.DLL"		, "Bga"		, "BGA"	, ".gza;.bza"						}, */
	/* {0, "JACK32.DLL"	, "Jack"	, "JACK", ".jak"							}, */
	{0, "FTP32.DLL"		, "Ftp"		, "FTP"	, "ftp://"							},
	{0, "TAR32.DLL"		, "Tar"		, "TAR"	, ".tar;.tgz;.taz;.Z;.gz;.tbz;.bz2"	}
};

/* 使用するＡＰＩのエントリーアドレスを取得 */
FARPROC GetFuncAddress(LPCSTR szBody)
{
	static int nDLLInfo = -1;
	char szFunc[FNAME_MAX32 + 1];
	FARPROC proc = NULL;
	
	/* 使用するＤＬＬが変更されたか判定 */
	if (nDLLInfo != nDLL)
	{
		FreeLibrary(hDLL);
		hDLL = LoadLibraryEx(DLLInfo[nDLL].szDLLName, NULL, 0);
		nDLLInfo = nDLL;
	}
	/* ＡＰＩのエントリーアドレスを取得 */
	if (hDLL)
	{
		strcpy(szFunc, DLLInfo[nDLL].szMainFunc);
		strcat(szFunc, szBody);
		proc = GetProcAddress(hDLL, szFunc);
	}
	
	return proc;
}

/* ＤＬＬをチェック */
void CheckDLL()
{
	/* バージョンチェック */
	ARC_GET_VERSION GetVersion;
	for (nDLL = 0; nDLL < Number(DLLInfo); nDLL++)
	{
		GetVersion = (ARC_GET_VERSION)GetFuncAddress(S_GET_VERSION);
		if (GetVersion)
			DLLInfo[nDLL].wVersion = GetVersion();
	}
}

/* ＡＰＩのリーダーから使用するＤＬＬを特定 */
BOOL SetDLLInfo(LPCSTR szMainFunc)
{
	for (nDLL = 0; nDLL < Number(DLLInfo); nDLL++)
	{
		if (!stricmp(szMainFunc, DLLInfo[nDLL].szMainFunc))
			return TRUE;
	}
	return FALSE;
}

/* ファイル名から使用するＤＬＬを特定 */
BOOL SetDLLInfoEx(LPCSTR szFileName)
{
	ARC_CHECK_ARCHIVE CheckArchive;
	for (nDLL = 0; nDLL < Number(DLLInfo); nDLL++)
	{
		CheckArchive = (ARC_CHECK_ARCHIVE)GetFuncAddress(S_CHECK_ARCHIVE);
		if (CheckArchive)
			if (CheckArchive(szFileName, CHECKARCHIVE_RAPID))
				return TRUE;
	}
	return FALSE;
}

/* ファイル名から使用するＤＬＬを特定２ */
BOOL SetDLLInfoEx2(LPCSTR szFileName)
{
	char szFname[_MAX_FNAME];
	char szExt[_MAX_EXT];
	char *szToken;
	
	/* パスを分解 */
	_splitpath(szFileName, NULL, NULL, szFname, szExt);
	
	ARC_CHECK_ARCHIVE CheckArchive;
	for (nDLL = 0; nDLL < Number(DLLInfo); nDLL++)
	{
		/* トークンを取得 */
		szToken = strtok(DLLInfo[nDLL].szSufix, ";");
		while (szToken != NULL)
		{
			/* ある程度判別 */
			if ((szToken[0] == '.') ? !stricmp(szExt, szToken) : !strnicmp(szFileName, szToken, sizeof(szToken)))
			{
				/* 厳密なチェック */
				CheckArchive = (ARC_CHECK_ARCHIVE)GetFuncAddress(S_CHECK_ARCHIVE);
				if (CheckArchive)
					if (CheckArchive(szFileName, CHECKARCHIVE_RAPID))
						return TRUE;
			}
			szToken = strtok(NULL, ";");
		}
	}
	/* 最後の手段 */
	return SetDLLInfoEx(szFileName);
}

/* コマンドラインをＤＬＬに渡す */
void DoARcToy(LPCSTR szCmdLine)
{
	/* オプション指定ダイアログにするかどうかの判定 */
	if (strlen(szCmdLine))
	{
		/* コマンドを実行 */
		ARC_DO_ARC DoArc = (ARC_DO_ARC)GetFuncAddress(S_DO_ARC);
		if (DoArc)
		{
			char szOutput[BUFFSIZE + 1] = "";
			
			DoArc(NULL, szCmdLine, szOutput, BUFFSIZE);
			szOutput[BUFFSIZE + 1] = '\0';
			printf("[CommandLine to %s : %s]\n%s",
				DLLInfo[nDLL].szDLLName, szCmdLine, szOutput);
		}
		else
			fprintf(stderr, "コマンドを実行するために必要な、ＡＰＩのエントリーアドレスが得られません。\n");
	}
	else
	{
		/* オプション指定ダイアログを表示 */
		ARC_CONFIG_DIALOG ConfigDialog = (ARC_CONFIG_DIALOG)GetFuncAddress(S_CONFIG_DIALOG);
		if (ConfigDialog)
		{
			char szOption[2][FNAME_MAX32 + 1] = {"", ""};
			
			ConfigDialog(NULL, szOption[0], UNPACK_CONFIG_MODE);
			ConfigDialog(NULL, szOption[1], PACK_CONFIG_MODE);
			printf(
				"[Config of %s]\n"
				"  UnPackOption : %s\n"
				"  PackOption   : %s\n",
				DLLInfo[nDLL].szDLLName, szOption[0], szOption[1]);
		}
		else
			fprintf(stderr, "オプション指定ダイアログを表示するために必要な、ＡＰＩのエントリーアドレスが得られません。\n");
	}
}

/* ＤＯＳ風の日付時刻を文字列に変換 */
LPSTR ToString(WORD wDate, WORD wTime)
{
	static char szDateTime[20];/* "####-##-## ##:##:##" */
	sprintf(szDateTime, "%04u-%02u-%02u %02u:%02u:%02u",
		(wDate >> 9) + 1980, (wDate >> 5) & 0x0F, (wDate & 0x1F),
		(wTime >> 11), (wTime >> 5) & 0x1F, (wTime & 0x1F) * 2);
	return szDateTime;
}

/* 格納ファイル一覧を独自フォーマットで出力 */
void PrintLists(LPCSTR szPath, LPCSTR szMask)
{
	ARC_OPEN_ARCHIVE  OpenArchive;
	ARC_CLOSE_ARCHIVE CloseArchive;
	ARC_FIND_FIRST	  FindFirst;
	ARC_FIND_NEXT	  FindNext;
	ARC_GET_ARC_DATE  GetArcDate;
	ARC_GET_ARC_TIME  GetArcTime;
	
	OpenArchive  = (ARC_OPEN_ARCHIVE)  GetFuncAddress(S_OPEN_ARCHIVE);
	CloseArchive = (ARC_CLOSE_ARCHIVE) GetFuncAddress(S_CLOSE_ARCHIVE);
	FindFirst	 = (ARC_FIND_FIRST)    GetFuncAddress(S_FIND_FIRST);
	FindNext	 = (ARC_FIND_NEXT)	   GetFuncAddress(S_FIND_NEXT);
	GetArcDate	 = (ARC_GET_ARC_DATE)  GetFuncAddress(S_GET_ARC_DATE);
	GetArcTime	 = (ARC_GET_ARC_TIME)  GetFuncAddress(S_GET_ARC_TIME);
	
	if (OpenArchive && CloseArchive && FindFirst && FindNext)
	{
		HARC hArc = OpenArchive(NULL, szPath, 0);
		if (hArc != NULL)
		{
			INDIVIDUALINFO Info;
			int nCounts = 0;
			DWORD dwOriginalSize = 0;
			
			printf(
				"[List of %s file : %s]\n"
				"  File name                    File size     Date      Time   Attrs Type\n"
				"-----------------------------  ---------- ---------- -------- ----- -----\n",
				DLLInfo[nDLL].szArcType, szPath);
			
			if (FindFirst(hArc, szMask, &Info) != -1)
			{
				do
				{
					printf("%s\n                                 %8lu %s %-5s %s\n",
						Info.szFileName, Info.dwOriginalSize,
						ToString(Info.wDate, Info.wTime),
						Info.szAttribute, Info.szMode);
					dwOriginalSize += Info.dwOriginalSize;
					nCounts++;
				} while (FindNext(hArc, &Info) != -1);
				printf(
					"-----------------------------  ---------- ---------- --------\n"
					"  %4u files                     %8lu %s\n",
					nCounts, dwOriginalSize,
					(GetArcDate && GetArcTime) ? ToString(GetArcDate(hArc), GetArcTime(hArc)) : "");
			}
			else
				printf("  no file\n");
			CloseArchive(hArc);
		}
		else
			fprintf(stderr, "書庫ファイルが開けません。ファイル名を確認してください。\n");
	}
	else
		fprintf(stderr, "格納ファイル一覧を得るために必要な、ＡＰＩのエントリーアドレスが得られません。\n");
}

/* 使い方＆ＤＬＬのバージョン情報を出力 */
void PrintAbout()
{
	CheckDLL();
	
	printf(
		"[Help of ARcToy]\n"
		"  Usage: ART <IsArc> [<CmdLine...>]\n"
		"     or  ART <Archive> [<WildName...>]\n"
		"  IsArc    - アーカイバＤＬＬが提供するＡＰＩのリーダー\n"
		"  CmdLine  - アーカイバＤＬＬに渡すコマンドライン\n"
		"             省略した場合、オプション指定ダイアログを表示\n"
		"  Archive  - 書庫ファイル名(拡張子を含む)\n"
		"  WildName - 検索するファイル名(複数指定が可能)\n\n");
	
	printf(
		"[Info of ArchiverDLL]\n"
		"  DLLName                 IsArc   Type    Sufix                           \n"
		"------------------------- ------- ----- ----------------------------------\n");
	for (nDLL = 0; nDLL < Number(DLLInfo); nDLL++)
	{
		printf("%c %-12s(Ver %u.%02u)  %-7s %-5s %-34s\n",
			(DLLInfo[nDLL].wVersion) ? '+' : '-',
			DLLInfo[nDLL].szDLLName,
			DLLInfo[nDLL].wVersion / 100, DLLInfo[nDLL].wVersion % 100,
			DLLInfo[nDLL].szMainFunc,
			DLLInfo[nDLL].szArcType,
			DLLInfo[nDLL].szSufix);
	}
	printf("------------------------- ------- ----- ----------------------------------\n");
}

/* ＤＬＬに渡すコマンドラインを生成 */
LPSTR BuildCmdLine(int argc, char *argv[])
{
	static char szCmdLine[FNAME_MAX32 + 1];
	for (int i = 2 ; i < argc ; i++)
	{
		/* 空白を含むファイル名対策 */
		if (i != 2)
			strcat(szCmdLine, " ");
		if (strchr(argv[i], ' '))
		{
			strcat(szCmdLine, "\"");
			strcat(szCmdLine, argv[i]);
			strcat(szCmdLine, "\"");
		}
		else
		{
			strcat(szCmdLine, argv[i]);
		}
	}
	return szCmdLine;
}

void main(int argc, char *argv[])
{
	/* 著作権 */
	printf("ARcToy 1.25 Copyright(c) 1999-2000 by どんぞ(noda)  HZG02104@nifty.ne.jp\n");
	if (argc > 1)
	{
		if (SetDLLInfo(argv[1]))
			DoARcToy(BuildCmdLine(argc, argv));
		else if (SetDLLInfoEx2(argv[1]))
			PrintLists(argv[1], BuildCmdLine(argc, argv));
		else
			fprintf(stderr, "指示されたファイルをサポートするＤＬＬが存在しません。\n");
	}
	else
		PrintAbout();
}
