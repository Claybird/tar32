/*
	tar32.cpp
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
#include "tar.h"
#include "cpio.h"
#include "ar.h"

#include "tar32.h"
#include "util.h"
#include "tar32dll.h" // CTar32Exception
#include "tar32api.h" // API ERROR codes
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h> 
#include <mbstring.h>

#include <algorithm>
#include <string>
#include <memory>
#include <fstream>
using namespace std;

CTar32::CTar32()
{
	m_pfile = NULL;
	m_archive_type = 0;
	m_filecount = 0;
	m_write_mode = false;
	m_error_code = 0;
	longfilenames_buf = NULL;
}
CTar32::~CTar32()
{
	close();
}
int CTar32::s_get_archive_type(const char *arcfile)
{
	auto_ptr<ITarArcFile> pfile(ITarArcFile::s_open(arcfile,"rb",ARCHIVETYPE_AUTO));
	if(pfile.get()==NULL){return -1;}

	//int archive_type = ITarArcFile::s_get_archive_type();
	//if(archive_type == -1){return -1;}
	
	// HEADER tar_header;
	int ret;

	int archive_type = pfile->get_archive_type();

	union archive_header{
		HEADER tar;
		new_cpio_header cpio;
		ar_first_hdr ar;
	};
	archive_header arc_header;
	ret = pfile->read(&arc_header,sizeof(arc_header));
	if(ret >= sizeof(arc_header.tar)
		&& arc_header.tar.compsum() == strtol(arc_header.tar.dbuf.chksum, NULL, 8)){
		switch(archive_type){
		case ARCHIVETYPE_NORMAL:
			archive_type = ARCHIVETYPE_TAR;break;
		case ARCHIVETYPE_GZ:
			archive_type = ARCHIVETYPE_TARGZ;break;
		case ARCHIVETYPE_Z:
			archive_type = ARCHIVETYPE_TARZ;break;
		case ARCHIVETYPE_BZ2:
			archive_type = ARCHIVETYPE_TARBZ2;break;
		}
	}else if(ret >= sizeof(arc_header.cpio)
		&& arc_header.cpio.magic_check()){
		switch(archive_type){
		case ARCHIVETYPE_NORMAL:
			archive_type = ARCHIVETYPE_CPIO;break;
		case ARCHIVETYPE_GZ:
			archive_type = ARCHIVETYPE_CPIOGZ;break;
		case ARCHIVETYPE_Z:
			archive_type = ARCHIVETYPE_CPIOZ;break;
		case ARCHIVETYPE_BZ2:
			archive_type = ARCHIVETYPE_CPIOBZ2;break;
		}
	}else if(ret >= sizeof(arc_header.ar)
		&& (memcmp(arc_header.ar.magic,"!<arch>\012",8) == 0  || memcmp(arc_header.ar.magic,"!<bout>\012",8) == 0)
		&& arc_header.ar.hdr.magic_check()){
		switch(archive_type){
		case ARCHIVETYPE_NORMAL:
			archive_type = ARCHIVETYPE_AR;break;
		case ARCHIVETYPE_GZ:
			archive_type = ARCHIVETYPE_ARGZ;break;
		case ARCHIVETYPE_Z:
			archive_type = ARCHIVETYPE_ARZ;break;
		case ARCHIVETYPE_BZ2:
			archive_type = ARCHIVETYPE_ARBZ2;break;
		}
	}
	return archive_type;
}
bool CTar32::open(const char *arcfile,const char *mode,int archive_type /*= ARCHIVETYPE_AUTO*/)
{
	m_archive_type = archive_type;
	m_write_mode = (strchr(mode,'w') != NULL);
	if((!strchr(mode,'w')) && archive_type == ARCHIVETYPE_AUTO){ // if read mode
		m_archive_type = s_get_archive_type(arcfile);
		if(m_archive_type == -1 || m_archive_type == ARCHIVETYPE_NORMAL){
			return false;
		}
	}
	m_pfile = ITarArcFile::s_open(arcfile,mode,m_archive_type);
	if(!m_pfile){return false;}
	return true;
}
bool CTar32::close()
{
	if(m_write_mode && m_pfile && m_error_code==0){
		if(m_archive_type == ARCHIVETYPE_TAR 
			|| m_archive_type == ARCHIVETYPE_TARGZ 
			|| m_archive_type == ARCHIVETYPE_TARZ 
			|| m_archive_type == ARCHIVETYPE_TARBZ2){
			// If success to create tar file, append 1024(512*2) byte null block to the end of file.
			char buf[1024/*512 * 2*/]; memset(buf,0,sizeof(buf));
			m_pfile->write(buf,sizeof(buf));
		}
	}
	if(m_pfile){
		m_pfile->close();
		delete m_pfile;
		m_pfile = NULL;
	}
	if(longfilenames_buf){
		delete [] longfilenames_buf;
		longfilenames_buf = NULL;
	}
	return true;
}
bool CTar32::readdir(CTar32FileStatus *pstat)
{
	HEADER tar_header;
	int ret;
	CTar32FileStatus stat;

	
	if(m_archive_type == ARCHIVETYPE_TAR 
		|| m_archive_type == ARCHIVETYPE_TARGZ 
		|| m_archive_type == ARCHIVETYPE_TARZ 
		|| m_archive_type == ARCHIVETYPE_TARBZ2){
		{
			ret = m_pfile->read(&tar_header,sizeof(tar_header));
			if(ret == 0){return false;}
			if(ret != sizeof(tar_header)){
				throw CTar32Exception("can't read tar header",ERROR_HEADER_BROKEN);
			}
			HEADER zero_header;
			memset(&zero_header,0,sizeof(zero_header));
			if(memcmp(&tar_header,&zero_header,sizeof(tar_header)) == 0){
				return false;
			}
			if((unsigned long)tar_header.compsum() != (unsigned long)strtol(tar_header.dbuf.chksum , NULL, 8)){
				throw CTar32Exception("tar header checksum error.",ERROR_HEADER_CRC);
			}
		}
		stat.filename	=	tar_header.dbuf.name;
		stat.original_size		=	strtol(tar_header.dbuf.size, NULL, 8);
		stat.blocksize  =   512;
		if(tar_header.dbuf.typeflag == LONGLINK){	// tar_header.dbuf.name == "././@LongLink"
			char longfilename[2000] = "";
			int readsize = ((stat.original_size-1)/512+1)*512;
			ret = m_pfile->read(longfilename, readsize);
			if(ret == 0){
				throw CTar32Exception("can't get filename(LongLink)",ERROR_HEADER_BROKEN);
			}
			longfilename[stat.original_size]='\0';
			{
				ret = m_pfile->read(&tar_header,sizeof(tar_header));
				if(ret == 0){return false;}
				if(ret != sizeof(tar_header)){
					throw CTar32Exception("can't read tar header(LongLink)",ERROR_HEADER_BROKEN);
				}
				HEADER zero_header;
				memset(&zero_header,0,sizeof(zero_header));
				if(memcmp(&tar_header,&zero_header,sizeof(tar_header)) == 0){
					return false;
				}
				if((unsigned long)tar_header.compsum() != (unsigned long)strtol(tar_header.dbuf.chksum , NULL, 8)){
					throw CTar32Exception("tar header checksum error.(LongLink)",ERROR_HEADER_CRC);
				}
			}
			stat.filename = longfilename;
			stat.original_size		=	strtol(tar_header.dbuf.size, NULL, 8);
		}
		
		stat.mode		=   strtol(tar_header.dbuf.mode, NULL, 8);
		stat.uid		=   strtol(tar_header.dbuf.uid , NULL, 8);
		stat.gid		=   strtol(tar_header.dbuf.gid , NULL, 8);
		stat.mtime		=   strtol(tar_header.dbuf.mtime , NULL, 8);
		stat.chksum		=   strtol(tar_header.dbuf.chksum , NULL, 8);
		stat.typeflag	=   tar_header.dbuf.typeflag;
		stat.linkname	=	tar_header.dbuf.linkname;
		strncpy(stat.magic_version, tar_header.dbuf.magic,8);
		strncpy(stat.uname, tar_header.dbuf.uname, 32);
		strncpy(stat.gname, tar_header.dbuf.gname, 32);
		stat.devmajor	=   strtol(tar_header.dbuf.devmajor , NULL, 8);
		stat.devminor	=   strtol(tar_header.dbuf.devminor , NULL, 8);
		stat.atime		=   strtol(tar_header.dbuf.atime , NULL, 8);
		stat.ctime		=   strtol(tar_header.dbuf.ctime , NULL, 8);
		stat.offset		=   strtol(tar_header.dbuf.offset , NULL, 8);
		if(stat.typeflag == DIRTYPE){
			stat.mode &= ~_S_IFMT;
			stat.mode |= _S_IFDIR;
		}
		if((stat.mode & _S_IFMT) == _S_IFDIR){
			const char * f = stat.filename.c_str();
			if((char*)max(_mbsrchr((unsigned char*)f, '/'), _mbsrchr((unsigned char*)f,'\\')) != f+strlen(f)-1){
				stat.filename = stat.filename + "/";
			}
		}
	}else if(m_archive_type == ARCHIVETYPE_CPIO
		|| m_archive_type == ARCHIVETYPE_CPIOGZ
		|| m_archive_type == ARCHIVETYPE_CPIOZ
		|| m_archive_type == ARCHIVETYPE_CPIOBZ2){
		new_cpio_header cpio_header;
		{
			ret = m_pfile->read(&cpio_header,sizeof(cpio_header));
			if(ret == 0){return false;}
			if(ret != sizeof(cpio_header)){
				throw CTar32Exception("can't read tar header",ERROR_HEADER_BROKEN);
			}
			new_cpio_header zero_header;
			memset(&zero_header,0,sizeof(zero_header));
			if(! cpio_header.magic_check()){
				throw CTar32Exception("tar header checksum error.",ERROR_HEADER_CRC);
			}
		}
		int fnamelen;
		int dum;
		int nret = sscanf(((char*)&cpio_header)+6, "%8lx%8lx%8lx%8lx%8lx%8lx%8lx%8lx%8lx%8lx%8lx%8lx%8lx"
			,&dum, &stat.mode, &stat.uid, &stat.gid, &dum, &stat.mtime
			,&stat.original_size,&stat.devmajor, &stat.devminor, &dum, &dum
			,&fnamelen, &stat.chksum);
		if(nret != 13){
				throw CTar32Exception("can't read tar header",ERROR_HEADER_BROKEN);
		}

		if(fnamelen<=0 || fnamelen>=1000){return false;}
		char fname[1000];
		int fnamelen2 = (((fnamelen+sizeof(new_cpio_header) -1)/4)+1)*4 - sizeof(new_cpio_header);	/* 4 byte padding for ("new cpio header" + "filename") */
		m_pfile->read(fname,fnamelen2);
		stat.filename	=	fname;
		stat.blocksize		= 4;
		if(stat.filename == "TRAILER!!!"){
			return false;
		}
		if((stat.mode & _S_IFMT) == _S_IFDIR){
			stat.filename = stat.filename + "/";
		}
	}else if(m_archive_type == ARCHIVETYPE_AR
		|| m_archive_type == ARCHIVETYPE_ARGZ
		|| m_archive_type == ARCHIVETYPE_ARZ
		|| m_archive_type == ARCHIVETYPE_ARBZ2){
		if(m_filecount == 0){
			char magic[8];
			ret = m_pfile->read(magic,8);	/* skip "!<arch>\012" */
		}
		ar_hdr header;
		{
			ret = m_pfile->read(&header,sizeof(header));
			if(ret == 0){return false;}
			if(ret != sizeof(header)){
				throw CTar32Exception("can't read tar header",ERROR_HEADER_BROKEN);
			}
			ar_hdr zero_header;
			memset(&zero_header,0,sizeof(zero_header));
			if(! header.magic_check()){
				throw CTar32Exception("tar header checksum error.",ERROR_HEADER_CRC);
			}
		}
		char *p = strchr(header.name,' '); *p = '\0';			// cut from ' '
		if(strcmp(header.name,"//")!=0 && p-1 > header.name && strrchr(header.name,'/') == p-1){ *(p-1) = '\0';}	// cut last '/'
		stat.filename = header.name;
		stat.mtime = strtol(header.date,NULL,10);
		stat.uid = strtol(header.uid,NULL,10);
		stat.gid = strtol(header.gid,NULL,10);
		stat.mode = strtol(header.mode,NULL,8);
		stat.original_size = strtol(header.size,NULL,10);
		stat.blocksize		= 2;

		if(stat.filename == "/"){
			char buf[1000];
			sprintf(buf,"root%d",m_filecount);
			stat.filename = buf;
		}
		if(stat.filename == "//"){
			/* long file name support */
			CTar32InternalFile file;
			m_currentfile_status = stat;
			if(!file.open(this)){throw CTar32Exception("can't read tar header",ERROR_HEADER_BROKEN);}
			longfilenames_buf = new char[stat.original_size]; // (char*)malloc(stat.original_size);
			int n = file.read(longfilenames_buf,stat.original_size);
			if(n!=stat.original_size){throw CTar32Exception("can't read tar header",ERROR_HEADER_BROKEN);}
			file.close();
			CTar32FileStatus nextstat;
			bool bRet = readdir(&nextstat); m_filecount --;
			stat = nextstat;
			//if(!bRet || stat.filename != "/0"){
			//	throw CTar32Exception("can't read tar header",ERROR_HEADER_BROKEN);
			//}
			//stat.filename = longfilename;
		}
		{
			/* if use longfilename, substitute "/\d+" to filename */
			int bytes; int num;
			int n = sscanf(stat.filename.c_str(), "/%d%n", &bytes, &num);
			int len = stat.filename.length();
			if(n == 1 && num == len && longfilenames_buf){
				stat.filename = longfilenames_buf+bytes;
			}
		}

	}else{
		if(m_filecount != 0){return false;}
		stat.filename		= m_pfile->get_orig_filename();
		stat.original_size	= m_pfile->get_orig_size();
		stat.mtime			= m_pfile->get_mtime();
		stat.blocksize		= 1;
	}
	m_filecount ++;
	*pstat = stat;
	m_currentfile_status = stat;
	return true;
}
bool CTar32::readskip()
{
	CTar32InternalFile file;
	if(! file.open(this)){
		return false;
	}
	if(! file.close()){
		return false;
	}
#if 0
	int size;
	int ret;

	if(m_currentfile_status.size == 0){return true;}

	size = (((m_currentfile_status.size-1)/512)+1)*512;
	ret = m_pfile->seek(size, SEEK_CUR);
	if(ret == -1){return false;}
#endif
	return true;
}
bool CTar32::extract(const char *fname_extract_to)
{
	string fname;
	if(fname_extract_to){
		fname = fname_extract_to;
	}else{
		fname = m_currentfile_status.filename;
	}
	int filesize = m_currentfile_status.original_size;
	const int buf_size = 4096;
	char buf[buf_size];

	CTar32InternalFile file;
	file.open(this);

	//string dirname = get_dirname(fname.c_str());
	//mkdir_recursive(dirname.c_str());
	mkdir_recursive(get_dirname(fname.c_str()).c_str());

	ofstream fs_w;
	fs_w.open(fname.c_str(), ios::out|ios::binary);
	if(fs_w.fail()){return false;}
	//FILE *fp_w = fopen(fname.c_str(), "wb");
	//if(fp_w == NULL){
	//	return false;
	//}
	int readsize = 0;
	while(readsize<filesize){
		int nextreadsize = min((int)filesize-readsize,(int)sizeof(buf));
		int n = file.read(buf,nextreadsize);
		fs_w.write(buf,nextreadsize);
		if(fs_w.fail()){return false;}
		//fwrite(buf,1,ret,fp_w);
		readsize += n;
		if(n != nextreadsize){/*fclose(fp_w);*/return false;}
	}
	//fclose(fp_w);
	return true;
}

bool CTar32::addheader(const CTar32FileStatus &stat)
{
	int ret;
	int blocksize = 1;
	if(m_archive_type == ARCHIVETYPE_TAR 
		|| m_archive_type == ARCHIVETYPE_TARGZ 
		|| m_archive_type == ARCHIVETYPE_TARZ 
		|| m_archive_type == ARCHIVETYPE_TARBZ2){
		blocksize = 512;
		HEADER tar_header;
		{
			HEADER *pblock = &tar_header;
			memset(pblock, 0, sizeof(*pblock));
			const CTar32FileStatus *p = &stat;
			string fname = p->filename;

			if(fname.length() >= sizeof(pblock->dbuf.name)/*100*/){
				CTar32FileStatus tmpstat = stat;
				tmpstat.filename = "././@LongLink";
				tmpstat.typeflag = LONGLINK;
				tmpstat.original_size = fname.length();
				bool bret = addheader(tmpstat);
				char filename[2000];
				strcpy(filename, fname.c_str());
				int writesize = ((fname.length() - 1)/512+1)*512;
				ret = m_pfile->write(filename, writesize);
				if(ret != writesize){
					throw CTar32Exception("LongLink filename write error", ERROR_CANNOT_WRITE);
				}
			}
			strncpy(pblock->dbuf.name, fname.c_str(),NAMSIZ-1); /* tarnt 0.96->0.97 */
			sprintf(pblock->dbuf.mode, "%6o ", (unsigned int)p->mode);
			sprintf(pblock->dbuf.uid, "%06o ",p->uid);
			sprintf(pblock->dbuf.gid, "%06o ",p->gid);
			sprintf(pblock->dbuf.size, "%11lo ", p->original_size);
			sprintf(pblock->dbuf.mtime, "%11lo ", p->mtime);
			pblock->dbuf.typeflag = p->typeflag;
			memcpy(pblock->dbuf.magic, p->magic_version, sizeof(p->magic_version));
			strncpy(pblock->dbuf.uname, p->uname, sizeof pblock->dbuf.uname);
			sprintf(pblock->dbuf.chksum, "%6o ", pblock->compsum());
		}

		ret = m_pfile->write(&tar_header,sizeof(tar_header));
		if(ret != sizeof(tar_header)){
			throw CTar32Exception("header write error", ERROR_CANNOT_WRITE);
		}
	}else{
		;
	}
	m_currentfile_status = stat;
	m_currentfile_status.blocksize = blocksize;
	return true;
}
bool CTar32::addbody(const char *file)
{
	struct _stat st;
	if(_stat(file, &st) == -1){
		char msg[1000];
		sprintf(msg, "can't read file[%s]", file);
		throw CTar32Exception(msg, ERROR_CANNOT_READ);
	}
	if(st.st_size == 0){return true;}
	//FILE *fp_r;
	//fp_r = fopen(file, "rb");
	ifstream fs_r;
	fs_r.open(file,ios::in|ios::binary);
	//if(!fp_r){
	if(fs_r.fail()){
		throw CTar32Exception("can't read file", ERROR_CANNOT_READ);
	}
	int size = 0;

	int n;
	char buf[4096];
	//while((n = fread(buf,1,sizeof(buf),fp_r))>0){
	while(fs_r.read(buf,sizeof(buf)),(n=fs_r.gcount())>0){
		int m = m_pfile->write(buf, n);
		if(m>0){size += m;}
		if(n!=m){
			throw CTar32Exception("can't write to arcfile", ERROR_CANNOT_WRITE);
		}
	}

	if(m_archive_type == ARCHIVETYPE_TAR 
		|| m_archive_type == ARCHIVETYPE_TARGZ 
		|| m_archive_type == ARCHIVETYPE_TARZ 
		|| m_archive_type == ARCHIVETYPE_TARBZ2){
		/* padding 512-byte block */
		int writesize;
		if(size%512 == 0){
			writesize = 0;
		}else{
			writesize = 512 - size%512;
		}
		memset(buf,0,writesize);
		m_pfile->write(buf, writesize);
	}
	return true;
}







//
//  CTar32InternalFile
//
//
CTar32InternalFile::CTar32InternalFile(){
	m_pfile = NULL;
}
CTar32InternalFile::~CTar32InternalFile(){
	if(m_pfile)close();
}
// open after CTar32::readdir() or CTar32()::addheader()
bool CTar32InternalFile::open(CTar32 *pTar32, bool bWrite){
	m_pfile = pTar32->m_pfile;
	m_size = pTar32->m_currentfile_status.original_size;
	m_readsize = 0;
	m_blocksize = pTar32->m_currentfile_status.blocksize;
	m_write = bWrite;
	return true;
}
int  CTar32InternalFile::write(void *buf, int size){
	int n = m_pfile->write(buf, size);
	if(n>0){m_readsize += n;}
	return n;
}
int  CTar32InternalFile::read(void *buf, int size){
	int n = m_pfile->read(buf, m_size==-1 ? size : min(size, m_size-m_readsize));
	if(n>0){m_readsize += n;}
	return n;
}
bool CTar32InternalFile::close(){
	int size;
	if(m_write){
		size = m_readsize;
	}else{
		size = m_size;
	}
	bool bret = true;
	if(size != -1 && size!=0){
		size = (((size-1)/m_blocksize)+1) * m_blocksize;
		size = size - m_readsize;
		if(m_write){
			char buf[512];

			memset(buf,0,sizeof(buf));
			while(size>0){
				int s;
				if(size>sizeof(buf)){s=sizeof(buf);}else{s=size;}
				int ret = this->write(buf, s);
				if(ret>0){size -= ret;}
				if(ret != s){bret=false;break;}
			}
		}else{
			int ret = m_pfile->seek(size, SEEK_CUR);
			bret = (ret != -1);
		}
	}
	m_pfile = NULL;
	return bret;
}
