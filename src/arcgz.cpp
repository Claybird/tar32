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
#include "stdafx.h"
#include "arcgz.h"
#include "util.h"
#include "zlib.h"

#include "rpm.h"
#include "fast_stl.h"

CTarArcFile_GZip::CTarArcFile_GZip()
{
	m_gzFile = NULL;
}
CTarArcFile_GZip::~CTarArcFile_GZip()
{
	close();
}

bool CTarArcFile_GZip::open(const char *arcfile, const char *mode, int compress_level)
{
	m_arcfile = arcfile;
	gzFile f = NULL;
	int fd = -1;
	size64 rpmlen = 0;

	char buf[16];
	bool bReadMode=(NULL!=strchr(mode,'r'));
	if(bReadMode){
		rpmlen = rpm_getheadersize(arcfile);
		if(rpmlen == -1){rpmlen = 0;}
		fd = _open(arcfile, _O_BINARY|_O_RDONLY, 0);
		if(fd == -1){return false;}
		_lseeki64(fd, rpmlen, SEEK_CUR);
	}else{
		fd = _open(arcfile, _O_BINARY|_O_CREAT|_O_RDWR|_O_TRUNC, _S_IREAD | _S_IWRITE);

		_snprintf(buf,COUNTOF(buf),"%s%d",mode,compress_level);
		mode=buf;	//モードの文字列をすり替え
	}

	f = gzdopen(fd, mode);
	if(f==NULL){
		_close(fd);
		return false;
	}
	gzbuffer(f,1024*1024);
	m_gzFile = f;

	if(bReadMode){
		/* retrieve GZIP header information(filename, time,...) */
		//std::ifstream fs_r;
		fast_fstream fs_r;
		fs_r.open(arcfile, std::ios::in|std::ios::binary);
		int c;
		if(fs_r.fail()){return false;}
		fs_r.seekg(rpmlen, std::ios_base::cur);		/* skip rpm header */
		if(fs_r.get()!=0x1f || fs_r.get()!=0x8b){return false;}
		if((c=fs_r.get())==EOF){return false;}

		m_gzip_compress_method = c;
		if((c=fs_r.get())==EOF){return false;}
		int flags = m_gzip_flags = c;
		if((flags & GZIP_FLAG_ENCRYPTED)||(flags & GZIP_FLAG_CONTINUATION)||(flags & GZIP_FLAG_RESERVED)){return true;}
		time_t stamp;
		stamp = fs_r.get();
		stamp |= (fs_r.get()<<8);
		stamp |= (fs_r.get()<<16);
		stamp |= (fs_r.get()<<24);
		if(stamp<0){stamp=0;}
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
			std::string fname;
			while((c=fs_r.get())!=EOF && c!='\0'){
				fname += c;
			}
			m_orig_filename = m_gzip_orig_name = fname;
		}
		if(flags & GZIP_FLAG_COMMENT){
			std::string comment;
			while((c=fs_r.get())!=EOF && c!='\0'){
				comment += c;
			}
			m_gzip_comment = comment;
		}
		fs_r.seekg(-4, std::ios_base::end);
		size64 size;
		size = fs_r.get();
		size |= fs_r.get()<<8;
		size |= fs_r.get()<<16;
		size |= fs_r.get()<<24;
		m_orig_filesize = size;
	}
	return true;
	//return (f != NULL);
}
size64 CTarArcFile_GZip::read(void *buf, size64 size)
{
	return gzread(m_gzFile, buf, (unsigned int)size);
}
size64 CTarArcFile_GZip::write(void *buf, size64 size)
{
	return gzwrite(m_gzFile, buf, (size_t)size);	//TODO:size lost
}
size64 CTarArcFile_GZip::seek(size64 offset, int origin)
{
	return gzseek(m_gzFile, (size_t)offset, origin);	//TODO:size lost
}
void CTarArcFile_GZip::close()
{
	if(m_gzFile){
		int ret = gzclose(m_gzFile);
		m_gzFile = NULL;
	}
}

std::string CTarArcFile_GZip::get_orig_filename(){
	if(! m_orig_filename.empty()){return m_orig_filename;}
	std::string fname = get_filename(m_arcfile.c_str());
	if(fname.length()>3 && _stricmp(fname.substr(fname.length()-3).c_str(),".gz") == 0){
		return fname.substr(0, fname.length()-3);
	}
	return fname + "_extracted";
}
