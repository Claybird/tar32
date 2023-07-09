/*
	arcfile.h
		archive file input/output virtual class.
		by Yoshioka Tsuneo(QWF00133@nifty.ne.jp)
*/
#ifndef __ARCFILE_H
#define __ARCFILE_H

#define ARCHIVETYPE_AUTO -1
// enum{TARARCFILE_AUTO, TARARCFILE_NORMAL,TARARCFILE_GZIP,TARARCFILE_BZIP2};
/*#ifndef interface
 #define interface struct
#endif*/
struct/*interface*/ ITarArcFile{
	static ITarArcFile *s_open(const char *arcfile, const char *mode, int compress_level,int type, int threads_num);
	static int s_get_archive_type(const char *arcfile);
	virtual bool open(const char *arcfile, const char *mode, int compress_level) = 0;
	virtual size64 read(void *buf, size64 size) = 0;
	virtual size64 write(void *buf, size64 size) = 0;
	virtual size64 seek(size64 offset, int origin);
	virtual void close() = 0;
	virtual int get_archive_type() = 0;
	ITarArcFile(){
		m_mtime = 0;
		m_orig_filesize = -1;
	}
	virtual ~ITarArcFile(){;};

	virtual std::string get_orig_filename(){
		if(! m_orig_filename.empty()){return m_orig_filename;}
		return m_arcfile + "_extracted";
	}
	virtual size64 get_orig_size(){return m_orig_filesize;}
	virtual time_t get_mtime(){return m_mtime;}
	std::string get_arc_filename(){return m_arcfile;}
protected:
	std::string m_arcfile;
	std::string m_orig_filename;
	size64 m_orig_filesize;
	time_t m_mtime;
};

#endif //__ARCFILE_H
