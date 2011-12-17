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
#include "stdafx.h"
#include "tar.h"
#include "cpio.h"
#include "ar.h"

#include "tar32.h"
#include "util.h"
#include "tar32dll.h" // CTar32Exception
#include "tar32api.h" // API ERROR codes
#include "fast_stl.h"

CTar32::CTar32()
{
	m_pfile = NULL;
	m_archive_type = 0;
	m_filecount = 0;
	m_write_mode = false;
	m_error_code = 0;
	m_archive_charset=CHARSET_DONTCARE;
}
CTar32::~CTar32()
{
	close();
}
int CTar32::s_get_archive_type(const char *arcfile)
{
	std::auto_ptr<ITarArcFile> pfile(ITarArcFile::s_open(arcfile,"rb",-1,ARCHIVETYPE_AUTO));
	if(pfile.get()==NULL){return -1;}

	//int archive_type = ITarArcFile::s_get_archive_type();
	//if(archive_type == -1){return -1;}
	
	// HEADER tar_header;

	int archive_type = pfile->get_archive_type();

	union archive_header{
		HEADER tar;
		new_cpio_header cpio;
		ar_first_hdr ar;
	};
	archive_header arc_header;
	size64 ret = pfile->read(&arc_header,sizeof(arc_header));
	if(ret >= sizeof(arc_header.tar)
		&& (arc_header.tar.compsum() == strtol(arc_header.tar.dbuf.chksum, NULL, 8) || arc_header.tar.compsum_oldtar() == strtol(arc_header.tar.dbuf.chksum, NULL, 8))){
		switch(archive_type){
		case ARCHIVETYPE_NORMAL:
			archive_type = ARCHIVETYPE_TAR;break;
		case ARCHIVETYPE_GZ:
			archive_type = ARCHIVETYPE_TARGZ;break;
		case ARCHIVETYPE_Z:
			archive_type = ARCHIVETYPE_TARZ;break;
		case ARCHIVETYPE_BZ2:
			archive_type = ARCHIVETYPE_TARBZ2;break;
		case ARCHIVETYPE_LZMA:
			archive_type = ARCHIVETYPE_TARLZMA;break;
		case ARCHIVETYPE_XZ:
			archive_type = ARCHIVETYPE_TARXZ;break;
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
		case ARCHIVETYPE_LZMA:
			archive_type = ARCHIVETYPE_CPIOLZMA;break;
		case ARCHIVETYPE_XZ:
			archive_type = ARCHIVETYPE_CPIOXZ;break;
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
		case ARCHIVETYPE_LZMA:
			archive_type = ARCHIVETYPE_ARLZMA;break;
		case ARCHIVETYPE_XZ:
			archive_type = ARCHIVETYPE_ARXZ;break;
		}
	}
	return archive_type;
}
bool CTar32::open(const char *arcfile,const char *mode,int compress_level,int archive_type /*= ARCHIVETYPE_AUTO*/,int archive_charset)
{
	m_archive_type = archive_type;
	m_archive_charset = archive_charset;
	m_write_mode = (strchr(mode,'w') != NULL);
	if((!strchr(mode,'w')) && archive_type == ARCHIVETYPE_AUTO){ // if read mode
		m_archive_type = s_get_archive_type(arcfile);
		if(m_archive_type == -1 || m_archive_type == ARCHIVETYPE_NORMAL){
			return false;
		}
	}
	m_pfile = ITarArcFile::s_open(arcfile,mode,compress_level,m_archive_type);
	if(!m_pfile){return false;}
	return true;
}
bool CTar32::close()
{
	if(m_write_mode && m_pfile && m_error_code==0){
		if(m_archive_type == ARCHIVETYPE_TAR 
			|| m_archive_type == ARCHIVETYPE_TARGZ 
			|| m_archive_type == ARCHIVETYPE_TARZ 
			|| m_archive_type == ARCHIVETYPE_TARBZ2
			|| m_archive_type == ARCHIVETYPE_TARLZMA
			|| m_archive_type == ARCHIVETYPE_TARXZ){
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
	longfilenames_buf.clear();
	return true;
}

bool CTar32::readTarHeader(HEADER &tar_header)
{
	size64 ret = m_pfile->read(&tar_header,sizeof(tar_header));
	if(ret == 0){return false;}
	if(ret != sizeof(tar_header)){
		throw CTar32Exception("can't read tar header",ERROR_HEADER_BROKEN);
	}
	{	//is zero header?
		int nIdx;
		for(nIdx=0;nIdx<sizeof(tar_header);nIdx++){
			if(((const char*)&tar_header)[nIdx]!=0)break;
		}
		if(nIdx==sizeof(tar_header))return false;	//EOF
	}
	int checksum=(int)parseOctNum(tar_header.dbuf.chksum , COUNTOF(tar_header.dbuf.chksum));
	if(tar_header.compsum() != checksum && tar_header.compsum_oldtar() != checksum){
		throw CTar32Exception("tar header checksum error.",ERROR_HEADER_CRC);
	}

	return true;
}

bool CTar32::readdir_TAR(CTar32FileStatus &stat)
{
	HEADER tar_header;
	if(!readTarHeader(tar_header))return false;

	//get tar format : GNU or POSIX
	int tar_format=tar_header.getFormat();

	// HP-UX's tar command create 100chars filename part. fixed on 2003.12.19
	char tmp_name[COUNTOF(tar_header.dbuf.name)+1];
	strncpy(tmp_name, tar_header.dbuf.name, COUNTOF(tar_header.dbuf.name));
	tmp_name[COUNTOF(tar_header.dbuf.name)] = '\0';
	stat.filename	=	tmp_name; /* tar_header.dbuf.name; */

	stat.original_size = parseOctNum(tar_header.dbuf.size,COUNTOF(tar_header.dbuf.size));
	if(tar_header.dbuf.typeflag == LNKTYPE){
		// Fixed on 2003/11/28. For "spencer_pwb.tar.gz". Thanks to rollo-san.
		stat.original_size = 0;
	}
	stat.blocksize  =   512;
	if(tar_header.dbuf.typeflag == LONGLINK){	// tar_header.dbuf.name == "././@LongLink"
		//NOTE:TAR32.DLL earlier than 2.33 makes LONGLINK entry with POSIX header
		tar_format=TAR_FORMAT_GNU;

		//char longfilename[2000] = "";
		std::vector<char> longfilename;
		size64 readsize = (size_t(stat.original_size-1)/512+1)*512;
		longfilename.resize((size_t)readsize+1);	//TODO:size lost
		size64 ret = m_pfile->read(&longfilename[0], readsize);
		if(ret == 0){
			throw CTar32Exception("can't get filename(LongLink)",ERROR_HEADER_BROKEN);
		}
		longfilename[(size_t)stat.original_size]='\0';	//TODO:size lost
		if(!readTarHeader(tar_header))return false;
		stat.filename = &longfilename[0];
		stat.original_size		=	parseOctNum(tar_header.dbuf.size, COUNTOF(tar_header.dbuf.size));
	}

	bool bPaxFilenameSupplied=false;
	time_t pax_atime=0,pax_ctime=0,pax_mtime=0;
	if(tar_header.dbuf.typeflag == PAX_GLOBAL || tar_header.dbuf.typeflag == PAX_ENTRTY){
		std::vector<char> content;
		size64 readsize = (size_t(stat.original_size-1)/512+1)*512;
		content.resize((size_t)readsize+1);	//TODO:size lost
		size64 ret = m_pfile->read(&content[0], readsize);
		if(ret == 0){
			throw CTar32Exception("can't get PAX Extended Global Header",ERROR_HEADER_BROKEN);
		}
		content[(size_t)stat.original_size]='\0';	//TODO:size lost
		if(!readTarHeader(tar_header))return false;
		stat.original_size		=	parseOctNum(tar_header.dbuf.size, COUNTOF(tar_header.dbuf.size));
		strncpy(tmp_name, tar_header.dbuf.name, COUNTOF(tar_header.dbuf.name));
		tmp_name[COUNTOF(tar_header.dbuf.name)] = '\0';
		stat.filename	=	tmp_name; /* tar_header.dbuf.name; */

		std::string extFilename;
		size64 filesize;
		if(!parsePaxExtHeader(&content[0],content.size(),extFilename,filesize,pax_atime,pax_ctime,pax_mtime)){
			if(tar_header.dbuf.typeflag == PAX_GLOBAL){
				throw CTar32Exception("Broken PAX Extended Global Header",ERROR_HEADER_BROKEN);
			}else{
				throw CTar32Exception("Broken PAX Extended Header",ERROR_HEADER_BROKEN);
			}
		}
		if(tar_header.dbuf.typeflag == PAX_GLOBAL){
			//global header
			//TODO:need test
			//if(filesize!=-1)m_currentfile_status.original_size=filesize;
		}else{
			//entry header
			//TODO:need test
			//if(filesize!=-1)stat.original_size=filesize;
			if(!extFilename.empty()){
				bPaxFilenameSupplied=true;
				stat.filename=extFilename;
			}
		}
	}

	//charset conversion
	if(m_archive_charset!=CHARSET_DONTCARE && !bPaxFilenameSupplied){
		if(m_archive_charset==CHARSET_UNKNOWN){
			//detect charset
			m_archive_charset=detect_charset(stat.filename.c_str());
		}

		switch(m_archive_charset){
		case CHARSET_EUCJP:
			stat.filename=CConvertCharsetHelper::getInstance().eucjp_to_sjis(stat.filename.c_str(),stat.filename.size());
			break;
		case CHARSET_UTF8N:	//FALLTHROUGH
		case CHARSET_UTF8:
			stat.filename=CConvertCharsetHelper::getInstance().utf8_to_sjis(stat.filename.c_str(),stat.filename.size());
			break;
		case CHARSET_JIS:
			//FALLTHROUGH
			/*
			 force to extract even if charset is not supported.
			 */
			//throw CTar32Exception("tar header charset error.",ERROR_NOT_SUPPORT);
			//break;
		case CHARSET_SJIS:	//FALLTHROUGH
		default:
			//nothing to do
			break;
		}
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
	if(tar_format==TAR_FORMAT_GNU){
		stat.atime		=   strtol(tar_header.dbuf.exthead.gnu.atime , NULL, 8);
		stat.ctime		=   strtol(tar_header.dbuf.exthead.gnu.ctime , NULL, 8);
		stat.offset		=   parseOctNum(tar_header.dbuf.exthead.gnu.offset , COUNTOF(tar_header.dbuf.exthead.gnu.offset));
	}else{	//POSIX
		int length=min(COUNTOF(tar_header.dbuf.exthead.posix.prefix),strlen(tar_header.dbuf.exthead.posix.prefix));
		if(length>0){
			std::string prefix(tar_header.dbuf.exthead.posix.prefix,tar_header.dbuf.exthead.posix.prefix+length);

			stat.filename= prefix + '/' + stat.filename;
		}
	}

	if(pax_atime!=0)stat.atime=pax_atime;
	if(pax_ctime!=0)stat.ctime=pax_ctime;
	if(pax_mtime!=0)stat.mtime=pax_mtime;

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
	return true;
}

bool CTar32::readdir_CPIO(CTar32FileStatus &stat)
{
	new_cpio_header cpio_header;
	{
		size64 ret = m_pfile->read(&cpio_header,sizeof(cpio_header));
		if(ret == 0){return false;}
		if(ret != sizeof(cpio_header)){
			throw CTar32Exception("can't read tar header",ERROR_HEADER_BROKEN);
		}
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
	return true;
}

bool CTar32::readdir_AR(CTar32FileStatus &stat)
{
	if(m_filecount == 0){
		char magic[8];
		/*size64 ret = */m_pfile->read(magic,8);	/* skip "!<arch>\012" */
	}
	ar_hdr header;
	{
		size64 ret = m_pfile->read(&header,sizeof(header));
		if(ret == 0){return false;}
		if(ret != sizeof(header)){
			throw CTar32Exception("can't read tar header",ERROR_HEADER_BROKEN);
		}
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
	//TODO:ここは10進数解釈?
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
		//longfilenames_buf = new char[stat.original_size]; // (char*)malloc(stat.original_size);
		longfilenames_buf.resize((unsigned int)stat.original_size+1);	//TODO:size lost
		size64 n = file.read(&longfilenames_buf[0],stat.original_size);
		if(n!=stat.original_size){throw CTar32Exception("can't read tar header",ERROR_HEADER_BROKEN);}
		file.close();

		longfilenames_buf[(unsigned int)stat.original_size]='\0';
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
		if(n == 1 && num == len && !longfilenames_buf.empty()){
			stat.filename = &longfilenames_buf[0]+bytes;
		}
	}
	return true;
}

bool CTar32::readdir(CTar32FileStatus *pstat)
{
	CTar32FileStatus stat;

	if(m_archive_type == ARCHIVETYPE_TAR 
		|| m_archive_type == ARCHIVETYPE_TARGZ 
		|| m_archive_type == ARCHIVETYPE_TARZ 
		|| m_archive_type == ARCHIVETYPE_TARBZ2
		|| m_archive_type == ARCHIVETYPE_TARLZMA
		|| m_archive_type == ARCHIVETYPE_TARXZ){
		if(!readdir_TAR(stat))return false;
	}else if(m_archive_type == ARCHIVETYPE_CPIO
		|| m_archive_type == ARCHIVETYPE_CPIOGZ
		|| m_archive_type == ARCHIVETYPE_CPIOZ
		|| m_archive_type == ARCHIVETYPE_CPIOBZ2
		|| m_archive_type == ARCHIVETYPE_CPIOLZMA
		|| m_archive_type == ARCHIVETYPE_CPIOXZ){
		if(!readdir_CPIO(stat))return false;
	}else if(m_archive_type == ARCHIVETYPE_AR
		|| m_archive_type == ARCHIVETYPE_ARGZ
		|| m_archive_type == ARCHIVETYPE_ARZ
		|| m_archive_type == ARCHIVETYPE_ARBZ2
		|| m_archive_type == ARCHIVETYPE_ARLZMA
		|| m_archive_type == ARCHIVETYPE_ARXZ){
		if(!readdir_AR(stat))return false;
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
	std::string fname;
	if(fname_extract_to){
		fname = fname_extract_to;
	}else{
		fname = m_currentfile_status.filename;
	}
	size64 filesize = m_currentfile_status.original_size;
	const int buf_size = 4096;
	char buf[buf_size];

	CTar32InternalFile file;
	file.open(this);

	//string dirname = get_dirname(fname.c_str());
	//mkdir_recursive(dirname.c_str());
	mkdir_recursive(get_dirname(fname.c_str()).c_str());

	//std::ofstream fs_w;
	fast_fstream fs_w;
	fs_w.open(fname.c_str(), std::ios::out|std::ios::binary);
	if(fs_w.fail()){return false;}
	//FILE *fp_w = fopen(fname.c_str(), "wb");
	//if(fp_w == NULL){
	//	return false;
	//}
	size64 readsize = 0;
	while(filesize==-1 || readsize<filesize){
		size64 nextreadsize = (filesize==-1) ? buf_size : min(filesize-readsize,buf_size);
		size64 n = file.read(buf,nextreadsize);
		fs_w.write(buf,nextreadsize);
		if(fs_w.fail()){
			if(filesize==-1){
				return true;
			}else{
				return false;
			}
		}
		//fwrite(buf,1,ret,fp_w);
		readsize += n;
		if(n != nextreadsize){/*fclose(fp_w);*/return false;}
	}
	//fclose(fp_w);
	return true;
}

bool CTar32::addheader(const CTar32FileStatus &stat)
{
	int blocksize = 1;
	if(m_archive_type == ARCHIVETYPE_TAR 
		|| m_archive_type == ARCHIVETYPE_TARGZ 
		|| m_archive_type == ARCHIVETYPE_TARZ 
		|| m_archive_type == ARCHIVETYPE_TARBZ2
		|| m_archive_type == ARCHIVETYPE_TARLZMA
		|| m_archive_type == ARCHIVETYPE_TARXZ){
		blocksize = 512;
		HEADER tar_header;
		{
			HEADER *pblock = &tar_header;
			memset(pblock, 0, sizeof(*pblock));
			const CTar32FileStatus *p = &stat;
			std::string fname = p->filename;

			if(fname.length() >= sizeof(pblock->dbuf.name)/*100*/){
				CTar32FileStatus tmpstat = stat;
				tmpstat.filename = "././@LongLink";
				tmpstat.typeflag = LONGLINK;
				tmpstat.original_size = fname.length();
				bool bret = addheader(tmpstat);
				char filename[2000];
				strcpy(filename, fname.c_str());
				size64 writesize = ((fname.length() - 1)/512+1)*512;
				size64 ret = m_pfile->write(filename, writesize);
				if(ret != writesize){
					throw CTar32Exception("LongLink filename write error", ERROR_CANNOT_WRITE);
				}
			}
			strncpy(pblock->dbuf.name, fname.c_str(),NAMSIZ-1); /* tarnt 0.96->0.97 */
			sprintf(pblock->dbuf.mode, "%6o ", (unsigned int)p->mode);
			sprintf(pblock->dbuf.uid, "%06o ",p->uid);
			sprintf(pblock->dbuf.gid, "%06o ",p->gid);
			sprintf(pblock->dbuf.size, "%11I64o ", p->original_size);
			sprintf(pblock->dbuf.mtime, "%11lo ", p->mtime);
			pblock->dbuf.typeflag = p->typeflag;
			memcpy(pblock->dbuf.magic, p->magic_version, sizeof(p->magic_version));
			strncpy(pblock->dbuf.uname, p->uname, sizeof pblock->dbuf.uname);
			sprintf(pblock->dbuf.chksum, "%6o ", pblock->compsum());
		}

		size64 ret = m_pfile->write(&tar_header,sizeof(tar_header));
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
	//std::ifstream fs_r;
	fast_fstream fs_r;
	fs_r.open(file,std::ios::in|std::ios::binary);
	//if(!fp_r){
	if(fs_r.fail()){
		throw CTar32Exception("can't read file", ERROR_CANNOT_READ);
	}
	size64 size = 0;

	size64 n;
	char buf[4096];
	//while((n = fread(buf,1,sizeof(buf),fp_r))>0){
	while(fs_r.read(buf,sizeof(buf)),(n=fs_r.gcount())>0){
		size64 m = m_pfile->write(buf, n);
		if(m>0){size += m;}
		if(n!=m){
			throw CTar32Exception("can't write to arcfile", ERROR_CANNOT_WRITE);
		}
	}

	if(m_archive_type == ARCHIVETYPE_TAR 
		|| m_archive_type == ARCHIVETYPE_TARGZ 
		|| m_archive_type == ARCHIVETYPE_TARZ 
		|| m_archive_type == ARCHIVETYPE_TARBZ2
		|| m_archive_type == ARCHIVETYPE_TARLZMA
		|| m_archive_type == ARCHIVETYPE_TARXZ){
		/* padding 512-byte block */
		size64 writesize;
		if(size%512 == 0){
			writesize = 0;
		}else{
			writesize = 512 - size%512;
		}
		memset(buf,0,(size_t)writesize);	//TODO:size lost
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
size64 CTar32InternalFile::write(void *buf, size64 size){
	size64 n = m_pfile->write(buf, size);
	if(n>0){m_readsize += n;}
	return n;
}
size64 CTar32InternalFile::read(void *buf, size64 size){
	size64 n = m_pfile->read(buf, m_size==-1 ? size : min(size, m_size-m_readsize));
	if(n>0){m_readsize += n;}
	return n;
}
bool CTar32InternalFile::close(){
	size64 size;
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
				size64 s;
				if(size>sizeof(buf)){s=sizeof(buf);}else{s=size;}
				size64 ret = this->write(buf, s);
				if(ret>0){size -= ret;}
				if(ret != s){bret=false;break;}
			}
		}else{
			while(size>0){
				size64 tmpsize = min(size,1000000000);
				size64 ret = m_pfile->seek(tmpsize, SEEK_CUR);
				bret = (ret != -1);
				if(bret==0){break;}
				size -= tmpsize;
			}
		}
	}
	m_pfile = NULL;
	return bret;
}
