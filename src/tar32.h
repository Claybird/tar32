/*
	tar32.h
		tar/tgz/tbz/bz/gz archive manipulation class.
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
#include "arcfile.h"
#include <string>

#include "tar.h"
#include <sys/types.h>	// stat
#include <sys/stat.h>	// stat

using namespace std;

struct CTar32FileStatus{
	CTar32FileStatus(){

		compress_size = original_size = blocksize = mode = uid = gid = mtime = chksum = typeflag = devmajor = devminor = atime = ctime = offset = 0;
		mode = 0666;	// _S_IWRITE|_S_IREAD
		memcpy(magic_version, TMAGIC "\0" TVERSION/*"ustar\000"*/,8);
		strcpy(uname, "root");
	}
	string filename;
	size_t original_size;
	size_t compress_size;
	int blocksize;

	int mode;
	int uid;
	int gid;
	time_t mtime;
	unsigned int chksum;
	int typeflag;
	string linkname;
	char magic_version[8];
	char uname[32];
	char gname[32];
	int  devmajor;
	int  devminor;
	time_t atime;
	time_t ctime;
	int offset;
	bool SetFromFile(const char *fname){
		struct _stat st;
		if(_stat(fname,&st)==-1){return false;}
		mode = st.st_mode;
		uid = st.st_uid;
		gid = st.st_gid;
		mtime = st.st_mtime;
		atime = st.st_atime;
		ctime = st.st_ctime;
		original_size = st.st_size;
		if((st.st_mode & _S_IFMT) == _S_IFDIR){
			typeflag = DIRTYPE;
		}
		return true;
	}
};

class CTar32InternalFile;
class CTar32{
public:
	CTar32();
	virtual ~CTar32();
	static int s_get_archive_type(const char *arcfile);
	bool open(const char *arcfile, const char*mode, int archive_type = ARCHIVETYPE_AUTO);
	bool close();

	bool readdir(CTar32FileStatus *stat);
	bool readskip();	// service function(not neccesary)
	bool extract(const char *file=NULL);		// service function(not neccesary)

	bool addheader(const CTar32FileStatus &stat);
	bool addbody(const char *file);	// survice function(not neccesary)

	CTar32FileStatus m_currentfile_status;
	int m_archive_type;
	string get_arc_filename(){return m_pfile->get_arc_filename();}
private:
	ITarArcFile *m_pfile;
	int m_filecount;	
	bool m_write_mode;
	int m_error_code;

	/* for ar(a.out) format */
	char *longfilenames_buf;

	friend CTar32InternalFile; // use m_currentfile_status.size
};


class CTar32InternalFile
{
public:
	CTar32InternalFile();
	~CTar32InternalFile();
	// open after CTar32::readdir() or CTar32()::addheader()
	bool open(CTar32 *pTar32, bool bWrite = false);
	int write(void *buf, int size);
	int read(void *buf, int size);
	bool close();
private:
	ITarArcFile *m_pfile;
	int m_readsize;
	int m_size;
	int m_blocksize;
	bool m_write;
};


