/*
	tar32.h
		tar/tgz/tbz/bz/gz archive manipulation class.
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
#include "arcfile.h"

#include "tar.h"

struct CTar32FileStatus{
	CTar32FileStatus(){

		compress_size = original_size = 0;
		blocksize = mode = uid = gid = mtime = chksum = typeflag = devmajor = devminor = atime = ctime = 0;
		offset = 0;
		mode = 0666;	// _S_IWRITE|_S_IREAD
		//memcpy(magic_version, TMAGIC "\0" TVERSION/*"ustar\000"*/,8);	//POSIX
		memcpy(magic_version, TOMAGIC/*"ustar  \0"*/,TOMAGLEN);	//GNU tar
		strcpy(uname, "root");
	}
	std::string filename;
	size64 original_size;
	size64 compress_size;
	int blocksize;

	int mode;
	int uid;
	int gid;
	time_t mtime;
	unsigned int chksum;
	int typeflag;
	std::string linkname;
	char magic_version[8];
	char uname[32];
	char gname[32];
	int  devmajor;
	int  devminor;
	time_t atime;
	time_t ctime;
	size64 offset;

	bool SetFromFile(const char *fname){
		struct _stati64 st;
		if(_stati64(fname,&st)==-1){return false;}
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
protected:
	bool readdir_TAR(CTar32FileStatus &stat);
	bool readdir_CPIO(CTar32FileStatus &stat);
	bool readdir_AR(CTar32FileStatus &stat);
public:
	CTar32();
	virtual ~CTar32();
	static int s_get_archive_type(const char *arcfile);
	bool open(const char *arcfile, const char*mode,int compress_level, int archive_type/* = ARCHIVETYPE_AUTO*/,int archive_charset);
	bool close();

	bool readTarHeader(HEADER &tar_header);
	bool readdir(CTar32FileStatus *stat);
	bool readskip();	// service function(not neccesary)
	bool extract(const char *file=NULL);		// service function(not neccesary)

	bool addheader(const CTar32FileStatus &stat);
	bool addbody(const char *file);	// survice function(not neccesary)

	CTar32FileStatus m_currentfile_status;
	int m_archive_type;
	std::string get_arc_filename(){return m_pfile->get_arc_filename();}
private:
	ITarArcFile *m_pfile;
	int m_filecount;	
	bool m_write_mode;
	int m_error_code;
	int m_archive_charset;	//filename charset of content of this archive

	/* for ar(a.out) format */
	//char *longfilenames_buf;
	std::vector<char> longfilenames_buf;

	friend CTar32InternalFile; // use m_currentfile_status.size
};


class CTar32InternalFile
{
public:
	CTar32InternalFile();
	~CTar32InternalFile();
	// open after CTar32::readdir() or CTar32()::addheader()
	bool open(CTar32 *pTar32, bool bWrite = false);
	size64 write(void *buf, size64 size);
	size64 read(void *buf, size64 size);
	bool close();
private:
	ITarArcFile *m_pfile;
	size64 m_readsize;
	size64 m_size;
	int m_blocksize;
	bool m_write;
};


