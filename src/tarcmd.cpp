/*
	tarcmd.cpp
		Tar() API implementation class.
		by Yoshioka Tsuneo(QWF00133@nifty.ne.jp)
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
#include "tar32api.h"
#include "cmdline.h"
#include "tar32.h"
#include "tar32dll.h"
#include "tarcmd.h"
#include "util.h"
#include "dlg.h"
#include <mbstring.h>
#include <process.h> // _beginthread
#include <io.h>	// chmod
#include <sys/utime.h> // utime
#include <time.h> // time
#include <sys/types.h>
#include <sys/stat.h>

#pragma warning(disable: 4786)
#include <list>
#include <string>
#include <strstream>
#include <fstream>
using namespace std;

class CTar32CmdInfo
{
public:
	struct CArgs{
		CArgs(const string &f, const string &dir) : file(f), current_dir(dir){};
		string file;
		string current_dir;
	};
	CTar32CmdInfo(char *s, int len) : output(s,len), exception("",0){
		hTar32StatusDialog = NULL;
		b_use_directory = true;
		b_display_dialog = true;
		b_message_loop = true;
		b_print = false;

		b_archive_tar = true;
		archive_type = ARCHIVETYPE_NORMAL;
		compress_level = 0;
		
		wm_main_thread_end = 0;
		hParentWnd = NULL;
		idMessageThread = 0;

	};
	string arcfile;
	list<CArgs> argfiles;
	// list<string> files;
	strstream output;
	HWND hTar32StatusDialog;
	CTar32Exception exception;

	bool b_use_directory;
	bool b_display_dialog;
	bool b_message_loop;
	bool b_print;

	bool b_archive_tar;
	int archive_type;
	int compress_level;

	UINT wm_main_thread_end;
	HWND hParentWnd;
	DWORD idMessageThread;
	char command;
};

static void cmd_create(CTar32CmdInfo &cmdinfo);
static void cmd_extract(CTar32CmdInfo &cmdinfo);
static void cmd_list(CTar32CmdInfo &cmdinfo);
static int tar_cmd_itr(const HWND hwnd, LPCSTR szCmdLine,LPSTR szOutput, const DWORD dwSize,CTar32CmdInfo &info);
static void cmd_usage(CTar32CmdInfo &info)
{
	info.output 
		<< "usage: TAR32.DLL <command> <option> archive.{tar,tar.gz,tar.bz2} filenames...\n"
		<< "    command:\n"
		<< "       EXTRACT: -x <files.tgz> [files...] \n"
		<< "       CREATE:  -c <files.tgz> [files...] \n"
		<< "       LIST:    -l <files.tgz> [files...] \n"
		<< "       PRINT:   -p <files.tgz> [files...] \n"
		<< "    option: (default value)\n"
		<< "       -z[N]     compress by gzip(.tar.gz) with level N(default:6)\n"
		<< "       -B[N]     compress by bzip2(.tar.bz2) with level N(default:9)\n"
		<< "       -G        not make tar archive.(.tar/.tar.gz/.tar.bz2)\n"
		<< "                 make compress only archive.(.gz/.bz2)\n"
		// << "       -Z[N]     compress by compress(LZW) (NOT IMPLEMENTED)\n"
		<< "       --use-directory=[0|1](1)  effective directory name\n"
		<< "       --display-dialog=[0|1](1)  display dialog box\n"
		<< "       --message-loop=[0|1](1)  run message loop\n"
		<< "       --bzip2=[N]     compress by bzip2 with level N\n"
		;
}

int tar_cmd(const HWND hwnd, LPCSTR szCmdLine,LPSTR szOutput, const DWORD dwSize, int *pWriteSize)
{
	CTar32CmdInfo cmdinfo(szOutput, dwSize-1);
	int ret;
	try{
		ret =  tar_cmd_itr(hwnd, szCmdLine,szOutput,dwSize,cmdinfo);
	}catch(CTar32Exception &e){
		cmdinfo.output << "TAR32 Error(0x" << hex << e.m_code << "): " << e.m_str << "\n";
		cmdinfo.output << "Tar((HWND)" << (unsigned)hwnd << ",(LPCSTR)" << szCmdLine << ",,(DWORD)" << (unsigned)dwSize << ")\n";
		ret =  e.m_code;
		cmd_usage(cmdinfo);
	}
	// int len = strlen(cmdinfo.output.str());
	int len = cmdinfo.output.rdbuf()->pcount();
	if(dwSize>0)szOutput[len] = '\0';
	if(pWriteSize){*pWriteSize = len;}
	return ret;
}
static void _cdecl tar_cmd_main_thread(LPVOID param);
static int tar_cmd_itr(const HWND hwnd, LPCSTR szCmdLine,LPSTR szOutput, const DWORD dwSize,CTar32CmdInfo &cmdinfo)
{
	list<string> args;	// command line array
	{
		char **argv;
		argv = split_cmdline_with_response(szCmdLine);
		if(!argv){
			throw CTar32Exception("commandline split error", ERROR_COMMAND_NAME);
		}
		char **argv2 = argv;
		while(*argv2){
			args.push_back(*argv2);
			argv2++;
		}
		free(argv);
	}

	char command = 0;	// main command. ('x','c','l')
	string current_directory;
	list<string>::iterator argi = args.begin();

	while(argi != args.end()){
		string::iterator stri = (*argi).begin();
		if(argi==args.begin() || *stri == '-'){
			stri++;
			if(*stri == '-'){
				stri++;
				const string &long_option = (*argi).substr(stri - argi->begin());
				string key;
				string val = long_option;
				int len;
				if((len = long_option.find('=')) != string::npos){
					key = long_option.substr(0, len);
					val = long_option.substr(len + 1);
				}
				if(key == "use-directory"){
					cmdinfo.b_use_directory = (bool)atoi(val.c_str());
				}else if(key == "display-dialog"){
					cmdinfo.b_display_dialog = (bool)atoi(val.c_str());
				}else if(key == "message-loop"){
					cmdinfo.b_message_loop = (bool)atoi(val.c_str());
				}else if(key == "bzip2"){
					cmdinfo.archive_type = ARCHIVETYPE_BZ2;
					cmdinfo.compress_level = (bool)atoi(val.c_str());
				}else{
					/* igonore */;
				}
				argi++;
				continue;
			}
			while(stri != (*argi).end()){
				switch(*stri){
				case 'x':
					command = 'x';break;
				case 'c':
					command = 'c';break;
				case 'l':
					command = 'l';break;
				case 'p':
					command = 'x';
					cmdinfo.b_print = true;
					break;
				case 'o':
					if(argi++ == args.end()){
						throw CTar32Exception("'o' follow no directory name", ERROR_COMMAND_NAME);
					}
					current_directory = *argi;
					stri = argi->end()-1;
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
					cmdinfo.archive_type = ARCHIVETYPE_BZ2;
					if(isdigit(*(stri+1))){
						stri++;
						cmdinfo.compress_level = ((char)*stri) - '0';
					}else{
						cmdinfo.compress_level = 9;
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
				case 'a':
				case 'v':
				case 'V':
				case 'I':
				case 'f':
				case 'e':
				case 'g':
				case 'S':
					/*ignore*/
					break;
				case 'A':
					stri++;
					/*ignore*/
					break;
				case 'b':
				case 'N':
					argi++;
					stri = argi->end()-1;
					/*ignore*/
					break;

				case 'U':
					if(isdigit(*(stri+1))){
						stri++;
					}
					/*ignore*/
					break;
				default:
					throw CTar32Exception("Invalid Arguments", ERROR_COMMAND_NAME);
				}
				stri++;
			}
			argi++;
		}else{
			const unsigned char *file = (const unsigned char *) (*argi).c_str();
			if(*file && (_mbsrchr(file, '\\') == file + _mbslen(file) - 1)){
				current_directory = (char*)file;
			}else if(cmdinfo.arcfile.length() == 0){
				cmdinfo.arcfile = *argi;
			}else{
				string file = *argi;
				cmdinfo.argfiles.push_back(CTar32CmdInfo::CArgs(file,current_directory));
			}
			argi++;
		}
	}
	if(cmdinfo.arcfile.empty()){
		throw CTar32Exception("Archive File is not specified.", ERROR_NOT_ARC_FILE);
	}
	if(cmdinfo.argfiles.empty()){
		if(command == 'x' || command == 'l'){
			// If no file to extract/create is specified, I assume as all file is specified.
			cmdinfo.argfiles.push_back(CTar32CmdInfo::CArgs("*",current_directory));
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
	cmdinfo.command = command;
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
				while(GetExitCodeThread(hThread,&exitcode)==STILL_ACTIVE){
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
	try{
		CTar32StatusDialog dlg;
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
		if(pCmdInfo->wm_main_thread_end)PostThreadMessage(pCmdInfo->idMessageThread, pCmdInfo->wm_main_thread_end, 0, 0);
	}catch(CTar32Exception &e){
		if(pCmdInfo->wm_main_thread_end)PostThreadMessage(pCmdInfo->idMessageThread, pCmdInfo->wm_main_thread_end, 0, 0);
		cmdinfo.exception = e;
		// throw e;
	}
	//return 0;
}
static bool extract_file(CTar32CmdInfo &cmdinfo, CTar32 *pTarfile, const char *fname)
{
	CTar32FileStatus &stat = pTarfile->m_currentfile_status;
	string fname2 = fname;

	EXTRACTINGINFOEX extractinfo;
	{
		memset(&extractinfo,0,sizeof(extractinfo));
		extractinfo.exinfo.dwFileSize = stat.original_size;
		extractinfo.exinfo.dwWriteSize = 0;
		strncpy(extractinfo.exinfo.szSourceFileName, pTarfile->get_arc_filename().c_str(),FNAME_MAX32+1);
		strncpy(extractinfo.exinfo.szDestFileName, fname2.c_str(), FNAME_MAX32+1);
		extractinfo.dwCompressedSize = stat.compress_size;
		extractinfo.dwCRC = stat.chksum;
		extractinfo.uOSType = 0;
		extractinfo.wRatio = extractinfo.exinfo.dwFileSize ? (1000 * extractinfo.dwCompressedSize / extractinfo.exinfo.dwFileSize) : 0;
		extractinfo.wDate = GetARCDate(stat.mtime);
		extractinfo.wTime = GetARCTime(stat.mtime);
		GetARCAttribute(stat.mode, extractinfo.szAttribute,sizeof(extractinfo.szAttribute));
		GetARCMethod(pTarfile->m_archive_type, extractinfo.szMode, sizeof(extractinfo.szMode));
		if(cmdinfo.hTar32StatusDialog){
			extern UINT wm_arcextract;
			int ret = SendMessage(cmdinfo.hTar32StatusDialog, wm_arcextract, ARCEXTRACT_BEGIN, (long)&extractinfo);
			if(ret){throw CTar32Exception("Cancel button was pushed.",ERROR_USER_CANCEL);}
		}
		fname2 = extractinfo.exinfo.szDestFileName;
	}

	int filesize = pTarfile->m_currentfile_status.original_size;

	CTar32InternalFile file; file.open(pTarfile);


	ofstream fs_w;
	if(!cmdinfo.b_print){
		mkdir_recursive(get_dirname(fname2.c_str()).c_str());
		fs_w.open(fname2.c_str(), ios::out|ios::binary);
		if(fs_w.fail()){return false;}
	}

	int readsize = 0;
	while(readsize<filesize){
		char buf[65536];
		int nextreadsize = min((int)filesize-readsize,(int)sizeof(buf));
		int n = file.read(buf,nextreadsize);
		readsize += n;
		if(cmdinfo.b_print){
			cmdinfo.output.write(buf,n);
		}else{
			fs_w.write(buf,n);
			if(fs_w.fail()){return false;}
		}
		if(n != nextreadsize){return false;}
		if(cmdinfo.hTar32StatusDialog){
			extern UINT wm_arcextract;
			extractinfo.exinfo.dwWriteSize = readsize;
			int ret = SendMessage(cmdinfo.hTar32StatusDialog, wm_arcextract, ARCEXTRACT_INPROCESS, (LPARAM)&extractinfo);
			if(ret){throw CTar32Exception("Cancel button was pushed.",ERROR_USER_CANCEL);}
		}
	}
	if(!cmdinfo.b_print){
		struct _utimbuf ut;
		ut.actime = (stat.atime ? stat.atime : time(NULL));
		ut.modtime = stat.mtime;
		int ret;
		ret = _utime(fname2.c_str(), &ut);
		ret = _chmod(fname2.c_str(), stat.mode);
	}
	if(cmdinfo.hTar32StatusDialog){
		extern UINT wm_arcextract;
		extractinfo.exinfo.dwWriteSize = readsize;
		int ret = SendMessage(cmdinfo.hTar32StatusDialog, wm_arcextract, ARCEXTRACT_END,(LPARAM) &extractinfo);
		if(ret){throw CTar32Exception("Cancel button was pushed.",ERROR_USER_CANCEL);}
	}
	return true;
}
static void cmd_extract(CTar32CmdInfo &cmdinfo)
{
	{
		EXTRACTINGINFOEX extractinfo;
		memset(&extractinfo,0,sizeof(extractinfo));
		strncpy(extractinfo.exinfo.szSourceFileName, cmdinfo.arcfile.c_str() ,FNAME_MAX32);
		if(cmdinfo.hTar32StatusDialog){
		extern UINT wm_arcextract;
			SendMessage(cmdinfo.hTar32StatusDialog, wm_arcextract, ARCEXTRACT_BEGIN, (LPARAM)&extractinfo);
		}
	}
	
	CTar32 tarfile;
	int ret;
	ret = tarfile.open(cmdinfo.arcfile.c_str(), "rb");
	if(!ret){
		throw CTar32Exception("can't open archive file", ERROR_ARC_FILE_OPEN);
	}

	CTar32FileStatus stat;
	while(true){
		bool bret = tarfile.readdir(&stat);
		if(!bret){break;}
		
		string file_internal = stat.filename;
		string file_external;
		{
			const list<CTar32CmdInfo::CArgs> &args = cmdinfo.argfiles;
			list<CTar32CmdInfo::CArgs>::const_iterator filei;
			for(filei = args.begin();filei!=args.end();filei++){
				if(::is_regexp_match_dbcs(filei->file.c_str(), file_internal.c_str())){
					string file_internal2 = file_internal;
					if(!cmdinfo.b_use_directory){
						file_internal2 = get_filename(file_internal.c_str());
					}
					file_external = make_pathname(filei->current_dir.c_str(), file_internal2.c_str());
					break;
				}
			}
		}
		if(file_external.empty()){
			bret = tarfile.readskip();
		}else{
			bool bret2 = extract_file(cmdinfo,&tarfile,file_external.c_str());
		}
	}

	{
		EXTRACTINGINFOEX extractinfo;
		memset(&extractinfo,0,sizeof(extractinfo));
		if(cmdinfo.hTar32StatusDialog){
		extern UINT wm_arcextract;
			SendMessage(cmdinfo.hTar32StatusDialog, wm_arcextract, ARCEXTRACT_END, (LPARAM)&extractinfo);
		}
	}
}
static void cmd_create(CTar32CmdInfo &cmdinfo)
{
	CTar32 tarfile;
	int ret;
	bool bret;
	ret = tarfile.open(cmdinfo.arcfile.c_str(), "wb", cmdinfo.archive_type);
	if(!ret){
		throw CTar32Exception("can't open archive file", ERROR_ARC_FILE_OPEN);
	}

	// const list<string> &files = cmdinfo.files;
	const list<CTar32CmdInfo::CArgs> &args = cmdinfo.argfiles;
	list<CTar32CmdInfo::CArgs>::const_iterator filei;
	for(filei = args.begin();filei!=args.end();filei++){
		string file_internal = (*filei).file;
		string file_external = make_pathname((*filei).current_dir.c_str(), (*filei).file.c_str());

		list<string> files_internal2 = find_files(file_external.c_str());
		list<string>::iterator files2i;
		for(files2i = files_internal2.begin(); files2i != files_internal2.end(); files2i++){
			string file_external2 = *files2i;
			string file_internal2 = file_external2.substr((*filei).current_dir.length());
			CTar32FileStatus stat;
			if(!stat.SetFromFile(file_external2.c_str())){
				continue;
			}
			convert_yen_to_slash((char*)(file_internal2.c_str()));
			{
				// if file is directory, add '/' to the tail of filename.
				struct _stat st;
				if(_stat(file_external2.c_str(), &st)!=-1 && st.st_mode & _S_IFDIR){
					char *f = (char*)file_internal2.c_str();
					if((char*)max(_mbsrchr((unsigned char*)f, '/'), _mbsrchr((unsigned char*)f,'\\')) != f+strlen(f)-1){
						file_internal2.append(1,'/');
					}
				}
			}
			stat.filename = file_internal2;
			bret = tarfile.addheader(stat);
			bret = tarfile.addbody(file_external2.c_str());
		}
	}
}
static void cmd_list(CTar32CmdInfo &cmdinfo)
{
	CTar32 tarfile;
	bool bret;
	bret = tarfile.open(cmdinfo.arcfile.c_str(), "rb");
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
