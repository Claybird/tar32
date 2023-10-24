/*
	arcgz.cpp
		gzip archive input/output class.
		by Yoshioka Tsuneo(QWF00133@nifty.ne.jp)

		Modified by ICHIMARU Takeshi ( ayakawa.m@gmail.com )
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
		CAutoFile fs_r;
		fs_r.open(arcfile, "rb");
		int c;
		if(!fs_r.is_opened()){return false;}
		_fseeki64(fs_r, rpmlen, SEEK_CUR);		/* skip rpm header */
		if (fgetc(fs_r) != 0x1f || fgetc(fs_r) != 0x8b) { return false; }
		if((c= fgetc(fs_r))==EOF){return false;}

		m_gzip_compress_method = c;
		if((c= fgetc(fs_r))==EOF){return false;}
		int flags = m_gzip_flags = c;
		if((flags & GZIP_FLAG_ENCRYPTED)||(flags & GZIP_FLAG_CONTINUATION)||(flags & GZIP_FLAG_RESERVED)){return true;}
		time_t stamp;
		stamp = fgetc(fs_r);
		stamp |= (fgetc(fs_r)<<8);
		stamp |= (fgetc(fs_r)<<16);
		stamp |= (fgetc(fs_r)<<24);
		if(stamp<0){stamp=0;}
		m_mtime = m_gzip_time_stamp = stamp;
		m_gzip_ext_flag = fgetc(fs_r);
		m_gzip_os_type = fgetc(fs_r);
		if(flags & GZIP_FLAG_CONTINUATION){
			m_gzip_part = fgetc(fs_r);
		}
		if(flags & GZIP_FLAG_EXTRA_FIELD){
			int len = fgetc(fs_r);
			while(len<10000 && (len--)>0){
				fgetc(fs_r);
			}
		}
		if(flags & GZIP_FLAG_ORIG_NAME){
			std::string fname;
			while((c= fgetc(fs_r))!=EOF && c!='\0'){
				fname += c;
			}
			m_orig_filename = m_gzip_orig_name = fname;
		}
		if(flags & GZIP_FLAG_COMMENT){
			std::string comment;
			while((c= fgetc(fs_r))!=EOF && c!='\0'){
				comment += c;
			}
			m_gzip_comment = comment;
		}

		/*
		fs_r.seekg(-4, std::ios_base::end);
		size64 size;
		size = ((unsigned __int64)fs_r.get());
		size |= ((unsigned __int64)fs_r.get())<<8;
		size |= ((unsigned __int64)fs_r.get())<<16;
		size |= ((unsigned __int64)fs_r.get())<<24;

		//ファイルサイズの下位32bitのみが格納されているため、4GB以上のファイルサイズを正しく復元できない
		m_orig_filesize = size;*/
		m_orig_filesize=-1;
	}
	return true;
	//return (f != NULL);
}
size64 CTarArcFile_GZip::read(void *buf, size64 size)
{
	// sizeがINT_MAX以下ならそのまま → INT_MAXより大きいとgzreadがエラーを返す
	if (0 <= size && size <= INT_MAX)
		return gzread(m_gzFile, buf, (unsigned int)size);
	else
		return -1;
	//return gzread(m_gzFile, buf, (unsigned int)size);
}
size64 CTarArcFile_GZip::write(void *buf, size64 size)
{
	// sizeがINT_MAX以下ならば従来通り → UINT_MAXじゃ無いのはgzwriteの戻り値がintなので……
	if (0 <= size && size <= INT_MAX)
		return gzwrite(m_gzFile, buf, (unsigned int)size);
	else
		return 0;
	//return gzwrite(m_gzFile, buf, (size_t)size);	//TODO:size lost
}
size64 CTarArcFile_GZip::seek(size64 offset, int origin)
{
	return gzseek64(m_gzFile, (long)offset, origin);
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
