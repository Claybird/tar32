/*
	arcfile.h
		archive file input/output virtual class.
		by Yoshioka Tsuneo(QWF00133@nifty.ne.jp)
*/
#ifndef __ARCFILE_H
#define __ARCFILE_H
#include <string>
using namespace std;

#define ARCHIVETYPE_AUTO -1
// enum{TARARCFILE_AUTO, TARARCFILE_NORMAL,TARARCFILE_GZIP,TARARCFILE_BZIP2};
#define interface struct
interface ITarArcFile{
	static ITarArcFile *s_open(const char *arcfile, const char *mode, int type);
	static int s_get_archive_type(const char *arcfile);
	virtual bool open(const char *arcfile, const char *mode) = 0;
	virtual int read(void *buf, int size) = 0;
	virtual int write(void *buf, int size) = 0;
	virtual int seek(int offset, int origin);
	virtual void close() = 0;
	virtual int get_archive_type() = 0;
	ITarArcFile(){
		m_mtime = 0;
		m_orig_filesize = -1;
	}
	virtual ~ITarArcFile(){;};

	virtual string get_orig_filename(){
		if(! m_orig_filename.empty()){return m_orig_filename;}
		return m_arcfile + "_extracted";
	}
	virtual size_t get_orig_size(){return m_orig_filesize;}
	virtual time_t get_mtime(){return m_mtime;}
	string get_arc_filename(){return m_arcfile;}
protected:
	string m_arcfile;
	string m_orig_filename;
	size_t m_orig_filesize;
	time_t m_mtime;
};

#endif //__ARCFILE_H
