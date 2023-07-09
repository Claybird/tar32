/*
	tarcmd.cpp
		Tar() API implementation class.
		by Yoshioka Tsuneo(QWF00133@nifty.ne.jp)

		Modified by ICHIMARU Takeshi(ayakawa.m@gmail.com)
*/
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
#include "tar32api.h"
#include "cmdline.h"
#include "tar32.h"
#include "tar32dll.h"
#include "util.h"
#include "dlg.h"
#include "tar32res.h"

#include "fast_stl.h"
#include "tarcmd.h"

#include "zstd.h"
#include "arczstd.h"

// Zstandardで別スレッドを使用しての圧縮を有効にします。但し、速度はほぼ変わらず＆メモリ大量消費
// なのでお勧めはしません
//#define USE_OTHER_THREADS_WITH_ZSTD

CTar32CmdInfo::CTar32CmdInfo(char *s, int len) : output(s,len), exception("",0){
	hTar32StatusDialog = NULL;
	b_use_directory = true;
	b_absolute_paths = false;
	b_display_dialog = true;
	b_message_loop = true;
	b_inverse_procresult = false;
	b_print = false;
	b_confirm_overwrite = false;
	b_sort_by_path=false;
	b_sort_by_ext=false;
	b_store_in_utf8 = true;

	b_archive_tar = true;
	archive_type = ARCHIVETYPE_NORMAL;
	compress_level = 0;
	
	b_zstd_ultra = false;
	zstd_c_thread_num = ZSTD_DEFAULT_THREADS_NUM;

	wm_main_thread_end = 0;
	hParentWnd = NULL;
	idMessageThread = 0;

	archive_charset=CHARSET_UNKNOWN;	//従来のプログラムでも文字変換が働くように、デフォルトで変換を行うようにする
	//archive_charset=CHARSET_DONTCARE;	//デフォルトでは変換無し
}


static void cmd_create(CTar32CmdInfo &cmdinfo);
static void cmd_extract(CTar32CmdInfo &cmdinfo);
static void cmd_list(CTar32CmdInfo &cmdinfo);
static void cmd_usage(CTar32CmdInfo &info)
{
	info.output 
#ifdef _WIN64
		<< "usage: TAR64.DLL <command> <option> archive.{tar,tar.gz,tar.bz2} filenames...\n"
#else
		<< "usage: TAR32.DLL <command> <option> archive.{tar,tar.gz,tar.bz2} filenames...\n"
#endif
		<< "    command:\n"
		<< "       EXTRACT: -x <files.tgz> [files...] \n"
		<< "       CREATE:  -c <files.tgz> [files...] \n"
		<< "       LIST:    -l <files.tgz> [files...] \n"
		<< "       PRINT:   -p <files.tgz> [files...] \n"
		<< "    option: (default value)\n"
		<< "       -z[N]     compress by gzip(.tar.gz) with level N(default:6)\n"
		<< "       -B[N] -j[N]  compress by bzip2(.tar.bz2) with level N(default:9)\n"
		<< "       -J[N]        compress by xz(.tar.xz) with level N(default:6)\n"
		<< "       -G        not make tar archive.(.tar/.tar.gz/.tar.bz2/.tar.zstd)\n"
		<< "                 make compress only archive.(.gz/.bz2/.zstd)\n"
		// << "       -Z[N]     compress by compress(LZW) (NOT IMPLEMENTED)\n"
		<< "       --use-directory=[0|1](1)  effective directory name\n"
		<< "       --absolute-paths=[0|1](0)  extract absolute paths(/, .., xx:)\n"
		<< "       --display-dialog=[0|1](1)  display dialog box\n"
		<< "       --message-loop=[0|1](1)  run message loop\n"
		<< "       --inverse-procresult=[0|1](0)  inverse result value of ARCHIVERPROC\n"
		<< "       --bzip2=[N]     compress by bzip2 with level N(default:9)\n"
		<< "       --lzma=[N]      compress by lzma with level N(default:6)\n"
		<< "       --xz=[N]        compress by xz with level N(default:6)\n"
		<< "       --zstd=[N]      compress by Zstandard with level N(default:3)\n"
		<< "       --zstd-ultra    enabled level from 20 to 22 for compressing with Zstd\n"
#ifdef USE_OTHER_THREADS_WITH_ZSTD
		<< "       --zstd-threads=[N](0)  number of other threads when compressing with Zstd\n"
#endif
		<< "       --confirm-overwrite=[0|1](0) ask for confirmation for\n"
		<< "                                    overwriting existing file\n"
		<< "       --convert-charset=[none|auto|sjis|eucjp|utf8](auto)\n"
		<< "                       convert charset of filename. If charset is not specified,\n"
		<< "                       charset is detected automatically.\n"
		<< "       --sort-by-ext=[0|1](0)  sort files by extension while making an archive.\n"
		<< "       --sort-by-path=[0|1](0)  sort files by path while making an archive.\n"
		<< "       --store-in-utf8=[0|1](1) store filename in utf-8 while making an archive.\n"
		<< "    ignore option,command: a,v,V,I,i,f,e,g,S,A,b,N,U,--xxxx=xxx\n"
		;
}

static int tar_cmd_itr(const HWND hwnd, LPCSTR szCmdLine,LPSTR szOutput, const DWORD dwSize,CTar32CmdInfo &cmdinfo);
int tar_cmd(const HWND hwnd, LPCSTR szCmdLine,LPSTR szOutput, const DWORD dwSize, int *pWriteSize)
{
	CTar32CmdInfo cmdinfo(szOutput, dwSize-1);
	int ret;
	try{
		tar_cmd_parser(szCmdLine,cmdinfo);
		ret =  tar_cmd_itr(hwnd, szCmdLine,szOutput,dwSize,cmdinfo);
	}catch(CTar32Exception &e){
#ifdef _WIN64
		cmdinfo.output << "TAR64 Error(0x" << std::hex << e.m_code << "): " << e.m_str << "\n";
		cmdinfo.output << "Tar((HWND)" << (unsigned __int64)hwnd << ",(LPCSTR)" << szCmdLine << ",,(DWORD)" << (unsigned)dwSize << ")\n";
#else
		cmdinfo.output << "TAR32 Error(0x" << std::hex << e.m_code << "): " << e.m_str << "\n";
		cmdinfo.output << "Tar((HWND)" << (unsigned)hwnd << ",(LPCSTR)" << szCmdLine << ",,(DWORD)" << (unsigned)dwSize << ")\n";
#endif
		ret =  e.m_code;
		cmd_usage(cmdinfo);
	}
	// int len = strlen(cmdinfo.output.str());
	int len = (int)cmdinfo.output.rdbuf()->pcount();
	if(dwSize>0)szOutput[len] = '\0';
	if(pWriteSize){*pWriteSize = len;}
	return ret;
}
static void _cdecl tar_cmd_main_thread(LPVOID param);
void tar_cmd_parser(LPCSTR szCmdLine,CTar32CmdInfo &cmdinfo)
{
	std::vector<std::string> args;	// command line array
	if(!split_cmdline_with_response(szCmdLine,args)){
		throw CTar32Exception("commandline split error", ERROR_COMMAND_NAME);
	}

	char command = 0;	// main command. ('x','c','l')
	//std::string current_directory; -> into CTar32CmdInfo
	std::vector<std::string>::iterator argi = args.begin();
	bool option_end = false;

	int tmp_zstd_level = -1;
	while(argi != args.end()){
		//string::iterator stri = (*argi).begin();
		const char *stri = (*argi).c_str();
		if(argi==args.begin() || (*stri == '-' && *(stri+1) != '\0' && option_end == false)){
			if(*stri == '-'){
				stri++;
			}
			if (*stri == '-' && *(stri + 1) != '\0') {
				stri++;
				const std::string long_option = (*argi).substr(stri - (*argi).c_str());
				std::string key = long_option;
				std::string val;
				std::string::size_type len;
				if ((len = long_option.find('=')) != std::string::npos) {
					key = long_option.substr(0, len);
					val = long_option.substr(len + 1);
				}
				if (key == "use-directory") {
					cmdinfo.b_use_directory = ((val == "") ? true : (0 != atoi(val.c_str())));
				}
				else if (key == "absolute-paths") {
					cmdinfo.b_absolute_paths = ((val == "") ? true : (0 != atoi(val.c_str())));
				}
				else if (key == "display-dialog") {
					cmdinfo.b_display_dialog = ((val == "") ? true : (0 != atoi(val.c_str())));
				}
				else if (key == "message-loop") {
					cmdinfo.b_message_loop = ((val == "") ? true : (0 != atoi(val.c_str())));
				}
				else if (key == "inverse-procresult") {
					cmdinfo.b_inverse_procresult = ((val == "") ? true : (0 != atoi(val.c_str())));
				}
				else if (key == "bzip2" || key == "bzip") {
					cmdinfo.archive_type = ARCHIVETYPE_BZ2;
					cmdinfo.compress_level = atoi(val.c_str());
					if (cmdinfo.compress_level < 1) cmdinfo.compress_level = 9;
					if (cmdinfo.compress_level > 9) cmdinfo.compress_level = 9;
				}
				else if (key == "gzip") {
					cmdinfo.archive_type = ARCHIVETYPE_GZ;
					cmdinfo.compress_level = atoi(val.c_str());
					if (cmdinfo.compress_level < 1) cmdinfo.compress_level = 5;
					if (cmdinfo.compress_level > 9) cmdinfo.compress_level = 5;
				}
				else if (key == "lzma") {
					cmdinfo.archive_type = ARCHIVETYPE_LZMA;
					cmdinfo.compress_level = atoi(val.c_str());
					if (cmdinfo.compress_level < 0) cmdinfo.compress_level = 6;
					if (cmdinfo.compress_level > 9) cmdinfo.compress_level = 6;
				}
				else if (key == "xz") {
					cmdinfo.archive_type = ARCHIVETYPE_XZ;
					cmdinfo.compress_level = atoi(val.c_str());
					if (cmdinfo.compress_level < 0) cmdinfo.compress_level = 6;
					if (cmdinfo.compress_level > 9) cmdinfo.compress_level = 6;
				}
				else if (key == "zstd") {
					cmdinfo.archive_type = ARCHIVETYPE_ZSTD;
					cmdinfo.compress_level = atoi(val.c_str());
					tmp_zstd_level = -1;
					if (cmdinfo.compress_level == 0) cmdinfo.compress_level = ZSTD_defaultCLevel();
					if (cmdinfo.compress_level < ZSTD_minCLevel()) cmdinfo.compress_level = ZSTD_defaultCLevel();
					if (cmdinfo.b_zstd_ultra) {
						if (cmdinfo.compress_level > ZSTD_maxCLevel()) cmdinfo.compress_level = ZSTD_defaultCLevel();
					}
					else {
						if (ZSTD_NORMAL_MAX_LEVEL < cmdinfo.compress_level && cmdinfo.compress_level <= ZSTD_maxCLevel())
							tmp_zstd_level = cmdinfo.compress_level;
						if (cmdinfo.compress_level > ZSTD_NORMAL_MAX_LEVEL) cmdinfo.compress_level = ZSTD_defaultCLevel();
					}
				}
				else if (key == "zstd-ultra") {
					cmdinfo.b_zstd_ultra = true;
					if (cmdinfo.archive_type == ARCHIVETYPE_ZSTD && ZSTD_NORMAL_MAX_LEVEL < tmp_zstd_level && tmp_zstd_level <= ZSTD_maxCLevel()) {
						cmdinfo.compress_level = tmp_zstd_level;
						tmp_zstd_level = -1;
					}
#ifdef USE_OTHER_THREADS_WITH_ZSTD
				}else if (key == "zstd-threads") {
					cmdinfo.zstd_c_thread_num = atoi(val.c_str());
					if (cmdinfo.zstd_c_thread_num < 0) cmdinfo.zstd_c_thread_num = ZSTD_DEFAULT_THREADS_NUM;
#endif
				}else if(key == "confirm-overwrite"){
					cmdinfo.b_confirm_overwrite = ((val=="") ? true : (0!=atoi(val.c_str())));
				}else if(key == "convert-charset"){
					if(val=="")cmdinfo.archive_charset = CHARSET_UNKNOWN;
					else if(stricmp(val.c_str(),"none")==0)cmdinfo.archive_charset  = CHARSET_DONTCARE;
					else if(stricmp(val.c_str(),"auto")==0)cmdinfo.archive_charset  = CHARSET_UNKNOWN;
					else if(stricmp(val.c_str(),"sjis")==0)cmdinfo.archive_charset  = CHARSET_SJIS;
					else if(stricmp(val.c_str(),"eucjp")==0)cmdinfo.archive_charset = CHARSET_EUCJP;
					else if(stricmp(val.c_str(),"utf8")==0)cmdinfo.archive_charset  = CHARSET_UTF8;
					else cmdinfo.archive_charset = CHARSET_DONTCARE;
				}else if(key == "sort-by-ext"){
					cmdinfo.b_sort_by_ext = ((val=="") ? true : (0!=atoi(val.c_str())));
				} else if (key == "sort-by-path") {
					cmdinfo.b_sort_by_path = ((val == "") ? true : (0 != atoi(val.c_str())));
				} else if (key == "store-in-utf8") {
					cmdinfo.b_store_in_utf8 = ((val == "") ? true : (0 != atoi(val.c_str())));
				}else{
					/* igonore */;
				}
				argi++;
				continue;
			}
			std::vector<std::string>::iterator cur_argi = argi;
			//while(stri != (*cur_argi).end()){
			while(stri != (*cur_argi).c_str()+(*cur_argi).length()){
				switch(*stri){
				case 'x':
					command = 'x';break;
				case 'c':
					command = 'c';break;
				case 'l':
				case 't': // どんぞ：追加
					command = 'l';break;
				case 'p':
					command = 'x';
					cmdinfo.b_print = true;
					break;
				case 'f':
					if(++argi == args.end()){
						throw CTar32Exception("'f' follow no directory name", ERROR_COMMAND_NAME);
					}
					cmdinfo.arcfile = *argi;
					break;
				case 'o':
					if(++argi == args.end()){
						throw CTar32Exception("'o' follow no directory name", ERROR_COMMAND_NAME);
					}
					cmdinfo.current_directory = *argi;
					// stri = argi->end()-1;
					break;
				case 'z':
					cmdinfo.archive_type = ARCHIVETYPE_GZ;
					if(isdigit(*(stri+1))){
						stri++;
						cmdinfo.compress_level = ((char)*stri) - '0';
					}else{
						cmdinfo.compress_level = 6;
					}
					break;
				case 'B':
				case 'j':
					cmdinfo.archive_type = ARCHIVETYPE_BZ2;
					if(isdigit(*(stri+1))){
						stri++;
						cmdinfo.compress_level = ((char)*stri) - '0';
					}else{
						cmdinfo.compress_level = 9;
					}
					break;
				case 'J':
					cmdinfo.archive_type = ARCHIVETYPE_XZ;
					if(isdigit(*(stri+1))){
						stri++;
						cmdinfo.compress_level = ((char)*stri) - '0';
					}else{
						cmdinfo.compress_level = 6;
					}
					break;
				case 'Z':
					cmdinfo.archive_type = ARCHIVETYPE_Z;
					if(isdigit(*(stri+1))){
						stri++;
						cmdinfo.compress_level = ((char)*stri) - '0';
					}else{
						cmdinfo.compress_level = 6;
					}
					break;
				case 'G':
					cmdinfo.b_archive_tar = false;
					if(cmdinfo.archive_type == ARCHIVETYPE_NORMAL){
						cmdinfo.archive_type = ARCHIVETYPE_GZ;
						cmdinfo.compress_level = 6;
					}
					break;
				case 'P':
					cmdinfo.b_absolute_paths = true;
					break;
				case 'a':
				case 'v':
				case 'V':
				case 'I':
				case 'i':
				case 'e':
				case 'g':
				case 'S':
				case 'R':	// added by tsuneo 2001.05.14
					/*ignore*/
					break;
				case 'A':
					stri++;
					/*ignore*/
					break;
				case 'b':
				case 'N':
					argi++;
					//stri = argi->end()-1;
					stri = (*argi).c_str()+(*argi).length()-1;
					/*ignore*/
					break;

				case 'U':
					if(isdigit(*(stri+1))){
						stri++;
					}
					/*ignore*/
					break;
				case '-':	// added by tsuneo on 2006/08/04. Patch is provided by Niiyama(HEROPA)-san. Special Thanks to Niiyama(HEROPA)-san.
					option_end = true;
					break;
				default:
					throw CTar32Exception("Invalid Arguments", ERROR_COMMAND_NAME);
				}
				stri++;
			}
			argi++;
		}else{
			const char *file = (*argi).c_str();
			if(*file && ((char*)_mbsrchr((const unsigned char*)file, '\\') == file + strlen(file) - 1)){
				cmdinfo.current_directory = (char*)file;
			}else if(cmdinfo.arcfile.length() == 0){
				cmdinfo.arcfile = *argi;
			}else{
				std::string file = *argi;
				cmdinfo.argfiles.push_back(CTar32CmdInfo::CArgs(file,cmdinfo.current_directory));
			}
			argi++;
		}
	}

	cmdinfo.command = command;
}

static int tar_cmd_itr(const HWND hwnd, LPCSTR szCmdLine,LPSTR szOutput, const DWORD dwSize,CTar32CmdInfo &cmdinfo)
{
	if(cmdinfo.arcfile.empty()){
		throw CTar32Exception("Archive File is not specified.", ERROR_NOT_ARC_FILE);
	}
	if(cmdinfo.argfiles.empty()){
		if(cmdinfo.command == 'x' || cmdinfo.command == 'l'){
			// If no file to extract/create is specified, I assume as all file is specified.
			cmdinfo.argfiles.push_back(CTar32CmdInfo::CArgs("*",cmdinfo.current_directory));
		}else{
			throw CTar32Exception("no file to archive is specified.", ERROR_NOT_ARC_FILE);
		}
	}
	if(cmdinfo.b_archive_tar){
		switch(cmdinfo.archive_type){
		case ARCHIVETYPE_NORMAL:
			cmdinfo.archive_type = ARCHIVETYPE_TAR;break;
		case ARCHIVETYPE_GZ:
			cmdinfo.archive_type = ARCHIVETYPE_TARGZ;break;
		case ARCHIVETYPE_Z:
			cmdinfo.archive_type = ARCHIVETYPE_TARZ;break;
		case ARCHIVETYPE_BZ2:
			cmdinfo.archive_type = ARCHIVETYPE_TARBZ2;break;
		case ARCHIVETYPE_LZMA:
			cmdinfo.archive_type = ARCHIVETYPE_TARLZMA;break;
		case ARCHIVETYPE_XZ:
			cmdinfo.archive_type = ARCHIVETYPE_TARXZ;break;
		case ARCHIVETYPE_ZSTD:
			cmdinfo.archive_type = ARCHIVETYPE_TARZSTD; break;
		}
	}
	//string arcfile = *argi++;
	//list<string> files(argi,args.end());
	
	//CTar32CmdInfo cmdinfo;
	//cmdinfo.arcfile = arcfile;
	//cmdinfo.files = files;

	/*
	CTar32StatusDialog dlg;
	if(cmdinfo.b_display_dialog){
		cmdinfo.hTar32StatusDialog = dlg.Create(hwnd);
	}
	switch(command){
	case 'x':
		cmd_extract(cmdinfo);
		break;
	case 'c':
		cmd_create(cmdinfo);
		break;
	case 'l':
		cmd_list(cmdinfo);
		break;
	default:
		throw CTar32Exception("Command not specified.", ERROR_COMMAND_NAME);
	}
	*/
	//cmdinfo.command = command;
	cmdinfo.hParentWnd = hwnd;
	int func_ret = 0;
	// extern static void _cdecl tar_cmd_main_thread(LPVOID param);
	//DWORD process_id,thread_id;
	//thread_id = GetWindowThreadProcessId(hwnd,&process_id);
	if(cmdinfo.b_message_loop && cmdinfo.hParentWnd /*&& process_id == GetCurrentProcessId() && thread_id == GetCurrentThreadId()*/){
		cmdinfo.wm_main_thread_end = RegisterWindowMessage("wm_main_thread_end");
		cmdinfo.idMessageThread = GetCurrentThreadId();
		MSG msg;
		HANDLE hThread = (HANDLE)_beginthread(tar_cmd_main_thread,0,(void*)&cmdinfo);
		while(GetMessage(&msg,NULL,0,0)){
			if(msg.message == cmdinfo.wm_main_thread_end){
				unsigned long exitcode;
				while(GetExitCodeThread(hThread,&exitcode) && (exitcode==STILL_ACTIVE)){
					Sleep(1);
				}
				break;
			}
			if(!IsDialogMessage(cmdinfo.hTar32StatusDialog,&msg)){
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
	}else{
		/*func_ret =*/ tar_cmd_main_thread((LPVOID)&cmdinfo);
	}
	if(cmdinfo.exception.m_code != 0){
		throw cmdinfo.exception;
	}
	return func_ret;
}
static void _cdecl tar_cmd_main_thread(LPVOID param)
{
	CTar32CmdInfo *pCmdInfo = (CTar32CmdInfo*)param;
	CTar32CmdInfo &cmdinfo = *pCmdInfo;
	CTar32StatusDialog dlg;

	try{
		if(cmdinfo.b_display_dialog){
			cmdinfo.hTar32StatusDialog = dlg.Create(cmdinfo.hParentWnd);
		}
		switch(cmdinfo.command){
		case 'x':
			cmd_extract(cmdinfo);
			break;
		case 'c':
			cmd_create(cmdinfo);
			break;
		case 'l':
			cmd_list(cmdinfo);
			break;
		default:
			throw CTar32Exception("Command not specified.", ERROR_COMMAND_NAME);
		}
		// メッセージループの終了前にダイアログを閉じる!  2000/03/03 by tsuneo
		dlg.Destroy();
		if(pCmdInfo->wm_main_thread_end)PostThreadMessage(pCmdInfo->idMessageThread, pCmdInfo->wm_main_thread_end, 0, 0);
	}catch(CTar32Exception &e){
		dlg.Destroy();
		if(pCmdInfo->wm_main_thread_end)PostThreadMessage(pCmdInfo->idMessageThread, pCmdInfo->wm_main_thread_end, 0, 0);
		cmdinfo.exception = e;
		// throw e;
	}catch(...){
		dlg.Destroy();
	}
	//return 0;
}

int SendArcMessage(CTar32CmdInfo &cmdinfo, int arcmode, EXTRACTINGINFOEX *pExtractingInfoEx,EXTRACTINGINFOEX64 *pExtractingInfoEx64)
{
	extern UINT wm_arcextract;
	int ret1,ret2,ret3;
	extern HWND g_hwndOwnerWindow;
	extern ARCHIVERPROC *g_pArcProc;
	ret1 = ret2 = ret3 = 0;

	if(cmdinfo.hTar32StatusDialog){
		//ret1 = ::SendMessage(cmdinfo.hTar32StatusDialog, wm_arcextract, arcmode, (long)pExtractingInfoEx);
		//EXTRACTINGINFOEX64 : internal use only
		ret1 = (int)::SendMessage(cmdinfo.hTar32StatusDialog, wm_arcextract, arcmode, (LPARAM)pExtractingInfoEx64);
	}
	if(g_hwndOwnerWindow){
		ret2 = (int)::SendMessage(g_hwndOwnerWindow,wm_arcextract, arcmode,(LPARAM)pExtractingInfoEx);
	}
	if(g_pArcProc){
		ret3 = (*g_pArcProc)(g_hwndOwnerWindow, wm_arcextract, arcmode, pExtractingInfoEx);
		if (cmdinfo.b_inverse_procresult)
		{
			ret3 = ! ret3;
		}
	}
	return (ret1 || ret2 || ret3);
}

int ConfirmOverwrite(const CTar32CmdInfo &cmdinfo,EXTRACTINGINFOEX64 &ExtractingInfoEx64)
{
	extern HINSTANCE dll_instance;
	extern HWND g_hwndOwnerWindow;
	std::string path=ExtractingInfoEx64.szDestFileName;
	convert_slash_to_backslash(path);

	//存在確認
	if(PathFileExists(path.c_str()) && !PathIsDirectory(path.c_str())){
		HWND hWnd=NULL;
		if(cmdinfo.hTar32StatusDialog){
			hWnd=cmdinfo.hTar32StatusDialog;
		}else if(g_hwndOwnerWindow){
			hWnd=g_hwndOwnerWindow;
		}

		get_full_path(path.c_str(),path);

		std::stringstream msg;
		msg << "File " << path << " already exists.\r\n"
			<< "Do you want to overwrite?";
		INT_PTR ret=::DialogBoxParam(dll_instance,MAKEINTRESOURCE(IDD_CONFIRM_OVERWRITE),hWnd,Tar32ConfirmOverwriteDialogProc,(LPARAM)(const char*)(msg.str().c_str()));
		switch(ret){
		case IDCANCEL:
			return -1;
		case IDC_BUTTON_OVERWRITE:
			return 0;
		case IDC_BUTTON_OVERWRITE_ALL:
			return 1;
		default:
			return 0;
		}
	}
	return 0;
}

void MakeExtractingInfo(CTar32* pTarfile,const char *fname,EXTRACTINGINFOEX &extractinfo,EXTRACTINGINFOEX64 &exinfo64)
{
	CTar32FileStatus &stat = pTarfile->m_currentfile_status;

	//EXTRACTINGINFOEX extractinfo;
	{
		memset(&extractinfo,0,sizeof(extractinfo));
		extractinfo.exinfo.dwFileSize = (DWORD)stat.original_size;
		extractinfo.exinfo.dwWriteSize = 0;
		strncpy(extractinfo.exinfo.szSourceFileName, pTarfile->get_arc_filename().c_str(),FNAME_MAX32+1);
		strncpy(extractinfo.exinfo.szDestFileName, fname, FNAME_MAX32+1);
		extractinfo.dwCompressedSize = (DWORD)stat.compress_size;
		extractinfo.dwCRC = stat.chksum;
		extractinfo.uOSType = 0;
		extractinfo.wRatio = (WORD)(extractinfo.exinfo.dwFileSize ? (1000 * extractinfo.dwCompressedSize / extractinfo.exinfo.dwFileSize) : 0);
		extractinfo.wDate = GetARCDate(stat.mtime);
		extractinfo.wTime = GetARCTime(stat.mtime);
		GetARCAttribute(stat.mode, extractinfo.szAttribute,sizeof(extractinfo.szAttribute));
		GetARCMethod(pTarfile->m_archive_type, extractinfo.szMode, sizeof(extractinfo.szMode));
	}
	//EXTRACTINGINFOEX64 exinfo64;
	{
		memset(&exinfo64,0,sizeof(exinfo64));
		exinfo64.dwStructSize=sizeof(exinfo64);

		exinfo64.exinfo=extractinfo.exinfo;

		exinfo64.llFileSize			=stat.original_size;
		exinfo64.llCompressedSize	=stat.compress_size;
		exinfo64.llWriteSize		=0;
		exinfo64.dwAttributes		=GetARCAttribute(stat.mode);
		exinfo64.dwCRC				=extractinfo.dwCRC;
		exinfo64.uOSType			=extractinfo.uOSType;
		exinfo64.wRatio				=extractinfo.wRatio;
		UnixTimeToFileTime(stat.ctime,exinfo64.ftCreateTime);
		UnixTimeToFileTime(stat.atime,exinfo64.ftAccessTime);
		UnixTimeToFileTime(stat.mtime,exinfo64.ftWriteTime);
		strncpy(exinfo64.szMode,extractinfo.szMode,sizeof(exinfo64.szMode));
		strncpy(exinfo64.szSourceFileName, extractinfo.exinfo.szSourceFileName,FNAME_MAX32+1);
		strncpy(exinfo64.szDestFileName, extractinfo.exinfo.szDestFileName, FNAME_MAX32+1);
	}
}


bool extract_file(CTar32CmdInfo &cmdinfo, CTar32 *pTarfile, const char *fname,std::vector<char> &buffer)
{
	CTar32FileStatus &stat = pTarfile->m_currentfile_status;
	std::string fname2 = fname;

	EXTRACTINGINFOEX extractinfo;
	EXTRACTINGINFOEX64 exinfo64;
	MakeExtractingInfo(pTarfile,fname2.c_str(),extractinfo,exinfo64);
	{
		int ret = SendArcMessage(cmdinfo, ARCEXTRACT_BEGIN, &extractinfo,&exinfo64);
		if(ret){throw CTar32Exception("Cancel button was pushed.",ERROR_USER_CANCEL);}
		fname2 = extractinfo.exinfo.szDestFileName;
	}

	//上書き確認
	if(cmdinfo.b_confirm_overwrite){
		switch(ConfirmOverwrite(cmdinfo, exinfo64)){
		case -1://cancel(abort)
			throw CTar32Exception("Cancel button was pushed.",ERROR_USER_CANCEL);
			break;
		case 1:	//yes to all
			cmdinfo.b_confirm_overwrite=false;
			break;
		}
	}

	size64 filesize = pTarfile->m_currentfile_status.original_size;

	CTar32InternalFile file; file.open(pTarfile);


	//std::ofstream fs_w;
	fast_fstream fs_w;
	if(!cmdinfo.b_print){
		mkdir_recursive(get_dirname(fname2.c_str()).c_str());
		fs_w.open(fname2.c_str(), std::ios::out|std::ios::binary);
		if(fs_w.fail()){return false;}
	}

	size64 readsize = 0;
	//static std::vector<char> buf;
	//const int bufsize=512*1024;
	//buf.resize(bufsize);
	const size_t bufsize=buffer.size();
	while(filesize ==-1 || readsize<filesize){
		size64 nextreadsize;
		if(filesize == -1){ // case ".gz",".Z",".bz2"
			//nextreadsize = sizeof(buf);
			nextreadsize=bufsize;
		}else{
			size64 nextreadsize64 = filesize-readsize;
			if(nextreadsize64 > bufsize){nextreadsize64 = bufsize;}
			nextreadsize = nextreadsize64;
			if(nextreadsize==0){
				Sleep(0);
			}
			// nextreadsize = (int)min(filesize-readsize, sizeof(buf));
		}
		if(nextreadsize==0){
			Sleep(0);
		}
		size64 n = file.read(&buffer[0],nextreadsize);
		readsize += n;
		if(cmdinfo.b_print){
			cmdinfo.output.write(&buffer[0],(size_t)n);	//TODO:size lost
		}else{
			fs_w.write(&buffer[0],n);
			if(fs_w.fail()){return false;}
		}
		if(n != nextreadsize){
			if(filesize == -1){ // case .gz/.Z/.bz2"
				break;
			}else{
				return false;
			}
		}
//		if(cmdinfo.hTar32StatusDialog){
			extractinfo.exinfo.dwWriteSize = (DWORD)readsize;
			exinfo64.exinfo.dwWriteSize = (DWORD)readsize;
			exinfo64.llWriteSize = readsize;
			int ret = SendArcMessage(cmdinfo, ARCEXTRACT_INPROCESS, &extractinfo,&exinfo64);
			if(ret){throw CTar32Exception("Cancel button was pushed.",ERROR_USER_CANCEL);}
//		}
	}
	if(!cmdinfo.b_print){
		fs_w.close();
		struct _utimbuf ut;
		ut.actime = (stat.atime ? stat.atime : time(NULL));
		ut.modtime = (stat.mtime ? stat.mtime : time(NULL));
		int ret;
		ret = _utime(fname2.c_str(), &ut);
		ret = _chmod(fname2.c_str(), stat.mode);
	}
//	if(cmdinfo.hTar32StatusDialog){
		extractinfo.exinfo.dwWriteSize = (DWORD)readsize;
		exinfo64.exinfo.dwWriteSize = (DWORD)readsize;
		exinfo64.llWriteSize = readsize;
		int ret = SendArcMessage(cmdinfo, 6, &extractinfo,&exinfo64);
		if(ret){throw CTar32Exception("Cancel button was pushed.",ERROR_USER_CANCEL);}
//	}
	return true;
}
static void cmd_extract(CTar32CmdInfo &cmdinfo)
{
	{
		EXTRACTINGINFOEX extractinfo;
		memset(&extractinfo,0,sizeof(extractinfo));
		EXTRACTINGINFOEX64 exinfo64;
		memset(&exinfo64,0,sizeof(exinfo64));
		exinfo64.dwStructSize=sizeof(exinfo64);

		strncpy(extractinfo.exinfo.szSourceFileName, cmdinfo.arcfile.c_str() ,FNAME_MAX32+1);
		exinfo64.exinfo=extractinfo.exinfo;
		strncpy(exinfo64.szSourceFileName, extractinfo.exinfo.szSourceFileName ,FNAME_MAX32+1);
		int ret = SendArcMessage(cmdinfo, ARCEXTRACT_OPEN, &extractinfo,&exinfo64);
		if(ret){throw CTar32Exception("Cancel button was pushed.",ERROR_USER_CANCEL);}
	}

	CTar32 tarfile;
	int ret;
	ret = tarfile.open(cmdinfo.arcfile.c_str(), "rb",-1,ARCHIVETYPE_AUTO,cmdinfo.archive_charset,cmdinfo.zstd_c_thread_num);
	if(!ret){
		throw CTar32Exception("can't open archive file", ERROR_ARC_FILE_OPEN);
	}

	CTar32FileStatus stat;
	std::vector<char> buffer;
	buffer.resize(1024*1024);
	while(true){
		bool bret = tarfile.readdir(&stat);
		if(!bret){break;}

		std::string file_internal = stat.filename;
		std::string file_external;
		{
			const std::list<CTar32CmdInfo::CArgs> &args = cmdinfo.argfiles;
			std::list<CTar32CmdInfo::CArgs>::const_iterator filei;
			for(filei = args.begin();filei!=args.end();filei++){
				if(::is_regexp_match_dbcs(filei->file.c_str(), file_internal.c_str())){
					std::string file_internal2 = file_internal;
					if(! cmdinfo.b_absolute_paths){
						file_internal2 = escape_absolute_paths(file_internal2.c_str());
					}
					if(!cmdinfo.b_use_directory){
						file_internal2 = get_filename(file_internal2.c_str());
					}
					file_external = make_pathname(filei->current_dir.c_str(), file_internal2.c_str());
					break;
				}
			}
		}
		if(file_external.empty()){
			bret = tarfile.readskip();
		}else{
			bool bret2 = extract_file(cmdinfo,&tarfile,file_external.c_str(),buffer);
		}
	}

	{
		EXTRACTINGINFOEX extractinfo;
		memset(&extractinfo,0,sizeof(extractinfo));
		EXTRACTINGINFOEX64 exinfo64;
		memset(&exinfo64,0,sizeof(exinfo64));
		exinfo64.dwStructSize=sizeof(exinfo64);
		int ret = SendArcMessage(cmdinfo, ARCEXTRACT_END, &extractinfo,&exinfo64);
		if(ret){throw CTar32Exception("Cancel button was pushed.",ERROR_USER_CANCEL);}
	}
}
static bool add_file(CTar32CmdInfo &cmdinfo, CTar32 *pTarfile, const char *fname,std::vector<char> &buffer)
{
	CTar32FileStatus &stat = pTarfile->m_currentfile_status;
	std::string fname2 = fname;

	EXTRACTINGINFOEX extractinfo;
	EXTRACTINGINFOEX64 exinfo64;
	MakeExtractingInfo(pTarfile,fname2.c_str(),extractinfo,exinfo64);
	{
		int ret = SendArcMessage(cmdinfo, ARCEXTRACT_BEGIN, &extractinfo,&exinfo64);
		if(ret){throw CTar32Exception("Cancel button was pushed.",ERROR_USER_CANCEL);}
		// fname2 = extractinfo.exinfo.szDestFileName;
	}

	size64 filesize = pTarfile->m_currentfile_status.original_size;
	if(filesize == 0){return true;}
	CTar32InternalFile file; file.open(pTarfile, /*write*/true);


	//std::ifstream fs_r;
	fast_fstream fs_r;
	fs_r.open(fname2.c_str(), std::ios::in|std::ios::binary);
	if(fs_r.fail()){throw CTar32Exception("can't read file", ERROR_CANNOT_READ);return false;}

	size64 readsize = 0;
	size64 n;
	while(fs_r.read(&buffer[0],buffer.size()),(n=fs_r.gcount())>0){
		size64 m = file.write(&buffer[0], n);
		if(m>0){readsize += m;}
		if(n!=m){
			throw CTar32Exception("can't write to arcfile", ERROR_CANNOT_WRITE);
		}
//		if(cmdinfo.hTar32StatusDialog){
			extractinfo.exinfo.dwWriteSize = (DWORD)readsize;
			exinfo64.exinfo.dwWriteSize = (DWORD)readsize;
			exinfo64.llWriteSize = readsize;
			int ret = SendArcMessage(cmdinfo, ARCEXTRACT_INPROCESS, &extractinfo,&exinfo64);
			if(ret){throw CTar32Exception("Cancel button was pushed.",ERROR_USER_CANCEL);}
//		}
	}
	bool bret = file.close();
	if(!bret){throw CTar32Exception("can't write to arcfile", ERROR_CANNOT_WRITE);}
//	if(cmdinfo.hTar32StatusDialog){
		extractinfo.exinfo.dwWriteSize = (DWORD)readsize;
		exinfo64.exinfo.dwWriteSize = (DWORD)readsize;
		exinfo64.llWriteSize = readsize;
		int ret = SendArcMessage(cmdinfo, 6, &extractinfo,&exinfo64);
		if(ret){throw CTar32Exception("Cancel button was pushed.",ERROR_USER_CANCEL);}
//	}
	return true;
}

struct FILE_TO_ADD{
	std::string fullPath;
	std::string relativePath;
};

static void cmd_create(CTar32CmdInfo &cmdinfo)
{
	{
		EXTRACTINGINFOEX extractinfo;
		memset(&extractinfo,0,sizeof(extractinfo));
		EXTRACTINGINFOEX64 exinfo64;
		memset(&exinfo64,0,sizeof(exinfo64));
		exinfo64.dwStructSize=sizeof(exinfo64);

		strncpy(extractinfo.exinfo.szSourceFileName, cmdinfo.arcfile.c_str() ,FNAME_MAX32+1);
		exinfo64.exinfo=extractinfo.exinfo;
		strncpy(exinfo64.szSourceFileName, extractinfo.exinfo.szSourceFileName ,FNAME_MAX32+1);
		int ret = SendArcMessage(cmdinfo, ARCEXTRACT_OPEN, &extractinfo,&exinfo64);
		if(ret){throw CTar32Exception("Cancel button was pushed.",ERROR_USER_CANCEL);}
	}

	CTar32 tarfile;
	int ret;
	bool bret;
	int filenum = 0;
	//char mode[10];

	//sprintf(mode, "wb%d", cmdinfo.compress_level);
	ret = tarfile.open(cmdinfo.arcfile.c_str(), "wb",cmdinfo.compress_level, cmdinfo.archive_type,cmdinfo.archive_charset,cmdinfo.zstd_c_thread_num);
	if(!ret){
		throw CTar32Exception("can't open archive file", ERROR_ARC_FILE_OPEN);
	}

	std::vector<char> buffer;
	buffer.resize(1024*1024);
	// const list<string> &files = cmdinfo.files;
	const std::list<CTar32CmdInfo::CArgs> &args = cmdinfo.argfiles;
	std::list<CTar32CmdInfo::CArgs>::const_iterator filei;

	//enumerate files
	std::vector<FILE_TO_ADD> files_to_add;
	for(filei = args.begin();filei!=args.end();filei++){
		std::vector<std::string> files_enumed;
		{
			std::string file_internal = (*filei).file;
			std::string file_external = make_pathname((*filei).current_dir.c_str(), (*filei).file.c_str());

			find_files(file_external.c_str(),files_enumed);
			if(_mbsrchr((const unsigned char*)file_external.c_str(),'*')==0 && files_enumed.empty()){
				// fixed by tsuneo. 2001.05.15
				throw CTar32Exception((std::string("can't find file [") + file_external + "]").c_str(), ERROR_FILE_OPEN);
			}
		}
		for(std::vector<std::string>::iterator files2i = files_enumed.begin(); files2i != files_enumed.end(); files2i++){
			std::string file_path = *files2i;
			std::string file_relative;
			if(cmdinfo.b_use_directory){
				file_relative = file_path.substr((*filei).current_dir.length());
			}else{
				file_relative = get_filename(file_path.c_str());
			}
			//
			FILE_TO_ADD file_entry;
			file_entry.fullPath=file_path;
			file_entry.relativePath=file_relative;
			files_to_add.push_back(file_entry);
		}
	}
	//sort? : by filepath or by extension
	if(cmdinfo.b_sort_by_ext){
		struct SORTER_BY_EXT{
			bool operator()(const FILE_TO_ADD& a,const FILE_TO_ADD& b){
				//sort by ext
				int ret=stricmp(PathFindExtension(a.relativePath.c_str()),PathFindExtension(b.relativePath.c_str()));
				if(ret==0){
					//sort by path if ext is same
					return stricmp(a.relativePath.c_str(),b.relativePath.c_str())<0;
				}else{
					return ret<0;
				}
			}
		};
		std::sort(files_to_add.begin(),files_to_add.end(),SORTER_BY_EXT());
	}else if(cmdinfo.b_sort_by_path){
		struct SORTER_BY_PATH{
			bool operator()(const FILE_TO_ADD& a,const FILE_TO_ADD& b){
				return stricmp(a.relativePath.c_str(),b.relativePath.c_str())<0;
			}
		};
		std::sort(files_to_add.begin(),files_to_add.end(),SORTER_BY_PATH());
	}
	//compression
	{
		for(std::vector<FILE_TO_ADD>::const_iterator ite=files_to_add.begin();ite!=files_to_add.end();++ite){
			const FILE_TO_ADD &file_entry=*ite;
			CTar32FileStatus stat;
			if(!stat.SetFromFile(file_entry.fullPath.c_str())){
				continue;
			}
			std::string filepath_to_store=file_entry.relativePath;
			convert_yen_to_slash(filepath_to_store);
			{
				// if file is directory, add '/' to the tail of filename.
				struct _stat st;
				if(_stat(file_entry.fullPath.c_str(), &st)!=-1 && st.st_mode & _S_IFDIR){
					char const*f = filepath_to_store.c_str();
					if((char*)max(_mbsrchr((unsigned char*)f, '/'), _mbsrchr((unsigned char*)f,'\\')) != f+strlen(f)-1){
						filepath_to_store.append(1,'/');
					}
				}
			}
			stat.filename = filepath_to_store;
			bret = tarfile.addheader(stat, cmdinfo.b_store_in_utf8);
			// bret = tarfile.addbody(file_external2.c_str());
			bool bret2 = add_file(cmdinfo,&tarfile,file_entry.fullPath.c_str(),buffer);
			filenum ++;
		}
	}

	{
		EXTRACTINGINFOEX extractinfo;
		memset(&extractinfo,0,sizeof(extractinfo));
		EXTRACTINGINFOEX64 exinfo64;
		memset(&exinfo64,0,sizeof(exinfo64));
		exinfo64.dwStructSize=sizeof(exinfo64);

		int ret = SendArcMessage(cmdinfo, ARCEXTRACT_END, &extractinfo,&exinfo64);
		if(ret){throw CTar32Exception("Cancel button was pushed.",ERROR_USER_CANCEL);}
	}

	if(filenum == 0){
		// fixed by tsuneo. 2001.05.14
		throw CTar32Exception("There is no file to archive. ", ERROR_FILE_OPEN);
	}
}
static void cmd_list(CTar32CmdInfo &cmdinfo)
{
	CTar32 tarfile;
	bool bret;
	bret = tarfile.open(cmdinfo.arcfile.c_str(), "rb",-1,ARCHIVETYPE_AUTO,cmdinfo.archive_charset,cmdinfo.zstd_c_thread_num);
	if(!bret){
		throw CTar32Exception("can't open archive file", ERROR_ARC_FILE_OPEN);
	}
	CTar32FileStatus stat;
	bret = true;
	cmdinfo.output << "filename" << "\t" << "filesize" << "\n";
	
	while(1){
		bret = tarfile.readdir(&stat);
		if(!bret){break;}
		bret = tarfile.readskip();
		if(!bret){break;}
		cmdinfo.output << stat.filename << "\t" << stat.original_size << "\n";
	}
}
