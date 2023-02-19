/*
	tarcmd.cpp
		Tar() API implementation class.
		by Yoshioka Tsuneo(QWF00133@nifty.ne.jp)
*/
int tar_cmd(const HWND hwnd, LPCSTR szCmdLine,LPSTR szOutput, const DWORD dwSize, int *pWriteSize=NULL);

class CTar32CmdInfo
{
public:
	struct CArgs{
		CArgs(const std::string &f, const std::string &dir) : file(f), current_dir(dir){};
		std::string file;
		std::string current_dir;
	};
	CTar32CmdInfo(char *s, int len);
	std::string arcfile;
	std::list<CArgs> argfiles;
	// list<string> files;
	std::strstream output;
	HWND hTar32StatusDialog;
	CTar32Exception exception;

	bool b_use_directory;
	bool b_absolute_paths;
	bool b_display_dialog;
	bool b_message_loop;
	bool b_inverse_procresult;
	bool b_print;
	bool b_sort_by_path;
	bool b_sort_by_ext;
	bool b_store_in_utf8;

	bool b_confirm_overwrite;	//上書き確認(解凍時)

	int archive_charset;	//解凍時のファイル名文字コード変換挙動

	bool b_archive_tar;
	int archive_type;
	int compress_level;

	std::string current_directory;

	UINT wm_main_thread_end;
	HWND hParentWnd;
	DWORD idMessageThread;
	char command;
};

void tar_cmd_parser(LPCSTR szCmdLine,CTar32CmdInfo &cmdinfo);
