/*
	arcgz.cpp
		gzip archive input/output class.
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
#include "arcgz.h"
#include "util.h"
#include "zlib.h"
#include <stdlib.h>
#include <fstream>

#include <io.h>	// open
#include <fcntl.h>
#include <sys/stat.h>	// _S_IREAD
#include "rpm.h"

/* gzip flag byte */
const CTarArcFile_GZip::GZIP_FLAG_ASCII_FLAG   =0x01; /* bit 0 set: file probably ascii text */
const CTarArcFile_GZip::GZIP_FLAG_CONTINUATION =0x02; /* bit 1 set: continuation of multi-part gzip file */
const CTarArcFile_GZip::GZIP_FLAG_EXTRA_FIELD  =0x04; /* bit 2 set: extra field present */
const CTarArcFile_GZip::GZIP_FLAG_ORIG_NAME    =0x08; /* bit 3 set: original file name present */
const CTarArcFile_GZip::GZIP_FLAG_COMMENT      =0x10; /* bit 4 set: file comment present */
const CTarArcFile_GZip::GZIP_FLAG_ENCRYPTED    =0x20; /* bit 5 set: file is encrypted */
const CTarArcFile_GZip::GZIP_FLAG_RESERVED     =0xC0; /* bit 6,7:   reserved */
const CTarArcFile_GZip::GZIP_METHOD_DEFLATED   =8;


CTarArcFile_GZip::CTarArcFile_GZip()
{
	m_gzFile = NULL;
}
CTarArcFile_GZip::~CTarArcFile_GZip()
{
	close();
}

bool CTarArcFile_GZip::open(const char *arcfile, const char *mode)
{
	m_arcfile = arcfile;
	gzFile f = NULL;
	int fd = -1;
	int rpmlen = 0;
	if(strchr(mode,'r')){
		rpmlen = rpm_getheadersize(arcfile);
		if(rpmlen == -1){rpmlen = 0;}
		fd = _open(arcfile, _O_BINARY|_O_RDONLY, 0);
		if(fd == -1){return false;}
		lseek(fd, rpmlen, SEEK_CUR);
	}else{
		fd = _open(arcfile, _O_BINARY|_O_CREAT|_O_RDWR|_O_TRUNC, _S_IREAD | _S_IWRITE);
	}

	f = gzdopen(fd, mode);
	if(f==NULL){ return false;}
	m_gzFile = f;

	if(strchr(mode,'r')){
		/* retrieve GZIP header information(filename, time,...) */
		ifstream fs_r;
		fs_r.open(arcfile, ios::in|ios::binary);
		int c;
		if(fs_r.fail()){return false;}
		fs_r.seekg(rpmlen, ios_base::cur);		/* skip rpm header */
		if(fs_r.get()!=0x1f || fs_r.get()!=0x8b){return false;}
		if((c=fs_r.get())==EOF){return false;}
		m_gzip_compress_method = c;
		if((c=fs_r.get())==EOF){return false;}
		int flags = m_gzip_flags = c;
		if((flags & GZIP_FLAG_ENCRYPTED)||(flags & GZIP_FLAG_CONTINUATION)||(flags & GZIP_FLAG_RESERVED)){return true;}
		time_t stamp;
		stamp = fs_r.get();
		stamp |= fs_r.get()<<8;
		stamp |= fs_r.get()<<16;
		stamp |= fs_r.get()<<24;
		m_mtime = m_gzip_time_stamp = stamp;
		m_gzip_ext_flag = fs_r.get();
		m_gzip_os_type = fs_r.get();
		if(flags & GZIP_FLAG_CONTINUATION){
			m_gzip_part = fs_r.get();
		}
		if(flags & GZIP_FLAG_EXTRA_FIELD){
			int len = fs_r.get();
			while(len<10000 && (len--)>0){
				fs_r.get();
			}
		}
		if(flags & GZIP_FLAG_ORIG_NAME){
			string fname;
			while((c=fs_r.get())!=EOF && c!='\0'){
				fname += c;
			}
			m_orig_filename = m_gzip_orig_name = fname;
		}
		if(flags & GZIP_FLAG_COMMENT){
			string comment;
			while((c=fs_r.get())!=EOF && c!='\0'){
				comment += c;
			}
			m_gzip_comment = comment;
		}
		fs_r.seekg(-4, ios_base::end);
		size_t size;
		size = fs_r.get();
		size |= fs_r.get()<<8;
		size |= fs_r.get()<<16;
		size |= fs_r.get()<<24;
		m_orig_filesize = size;
	}
	return true;
	//return (f != NULL);
}
int CTarArcFile_GZip::read(void *buf, int size)
{
	return gzread(m_gzFile, buf, size);
}
int CTarArcFile_GZip::write(void *buf, int size)
{
	return gzwrite(m_gzFile, buf, size);
}
int CTarArcFile_GZip::seek(int offset, int origin)
{
	return gzseek(m_gzFile, offset, origin);
}
void CTarArcFile_GZip::close()
{
	if(m_gzFile){
		int ret = gzclose(m_gzFile);
		m_gzFile = NULL;
	}
}

string CTarArcFile_GZip::get_orig_filename(){
	if(! m_orig_filename.empty()){return m_orig_filename;}
	string fname = get_filename(m_arcfile.c_str());
	if(fname.length()>3 && stricmp(fname.substr(fname.length()-3).c_str(),".gz") == 0){
		return fname.substr(0, fname.length()-3);
	}
	return fname + "_extracted";
}
