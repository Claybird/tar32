/*
	arcfile.cpp
		archive file input/output virtual class.
		by Yoshioka Tsuneo(QWF00133@nifty.ne.jp)

		Modified by ICHIMARU Takeshi(ayakawa.m@gmail.com)
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
#include "arcfile.h"

#include "normal.h" // CTarArcFile_Normal
#include "arcgz.h"	// CTarArcFile_GZip
#include "arcbz2.h"	// CTarArcFile_BZip2
#include "arclzma.h"	// CTarArcFile_Lzma
#include "arcz.h"	// CTarArcFile_Compress
#include "arczstd.h"	// CTarArcFile_Zstd
#include "tar32api.h"
#include "rpm.h"


size64 ITarArcFile::seek(size64 offset, int origin)
{
	if(origin != SEEK_CUR){
		return -1;
	}
	const int bufsize=1024*16;
	std::vector<char> buf;
	buf.resize(bufsize);
	while(offset > 0){
		size64 size = min(offset,bufsize);
		size64 n = read(&buf[0],size);
		if(n != size){return -1;}
		offset -= n;
	}
	return 0;
}
/*static*/
ITarArcFile *ITarArcFile::s_open(const char *arcfile, const char *mode, int compress_level, int type, int threads_num)
{
	ITarArcFile *pfile = NULL;
	int ret = 0;

	if(strchr(mode,'r') != 0 && type == ARCHIVETYPE_AUTO){
		type = s_get_archive_type(arcfile);
	}

	switch(type){
	case ARCHIVETYPE_AUTO:
	case ARCHIVETYPE_NORMAL:
	case ARCHIVETYPE_TAR:
	case ARCHIVETYPE_CPIO:
	case ARCHIVETYPE_AR:
		pfile = new CTarArcFile_Normal;
		break;
	case ARCHIVETYPE_GZ:
	case ARCHIVETYPE_TARGZ:
	case ARCHIVETYPE_CPIOGZ:
	case ARCHIVETYPE_ARGZ:
		pfile = new CTarArcFile_GZip;
		break;
	case ARCHIVETYPE_Z:
	case ARCHIVETYPE_TARZ:
	case ARCHIVETYPE_CPIOZ:
	case ARCHIVETYPE_ARZ:
		pfile = new CTarArcFile_Compress;
		break;
	case ARCHIVETYPE_BZ2:
	case ARCHIVETYPE_TARBZ2:
	case ARCHIVETYPE_CPIOBZ2:
	case ARCHIVETYPE_ARBZ2:
		pfile = new CTarArcFile_BZip2;
		break;
	case ARCHIVETYPE_LZMA:
	case ARCHIVETYPE_TARLZMA:
	case ARCHIVETYPE_CPIOLZMA:
	case ARCHIVETYPE_ARLZMA:
		pfile = new CTarArcFile_Lzma;
		((CTarArcFile_Lzma *)pfile)->set_format(ARCHIVETYPE_LZMA);
		break;
	case ARCHIVETYPE_XZ:
	case ARCHIVETYPE_TARXZ:
	case ARCHIVETYPE_CPIOXZ:
	case ARCHIVETYPE_ARXZ:
		pfile = new CTarArcFile_Lzma;
		((CTarArcFile_Lzma *)pfile)->set_format(ARCHIVETYPE_XZ);
		break;
	case ARCHIVETYPE_ZSTD:
	case ARCHIVETYPE_TARZSTD:
	case ARCHIVETYPE_CPIOZSTD:
	case ARCHIVETYPE_ARZSTD:
		pfile = new CTarArcFile_Zstd;
		if (strchr(mode, 'w') != 0) ((CTarArcFile_Zstd*)pfile)->set_threads_num(threads_num); // write mode only
			break;
	default:
		return NULL;
	}
	ret = pfile->open(arcfile, mode, compress_level);
	if(!ret){delete pfile;return NULL;}
	return pfile;
}
int ITarArcFile::s_get_archive_type(const char *arcfile)
{
	FILE *fp = fopen(arcfile, "rb");
	if(fp==NULL){return -1;} // どんぞ：追加
	unsigned char buf[100]; memset(buf, 0, sizeof(buf));
	size_t n = fread(buf, 1, sizeof(buf), fp);
	if(buf[0] == 0xed && buf[1] == 0xab && buf[2] == 0xee && buf[3] == 0xdb){
		/* RPM */
		size64 rpmlen = rpm_getheadersize(arcfile);
		if(rpmlen == -1){rpmlen = 0;}
		_fseeki64(fp, rpmlen, SEEK_SET);
		n = fread(buf, 1, sizeof(buf), fp);
	}
	fclose(fp);

	if(buf[0] == 0x1f && buf[1] == 0x8b){
		return ARCHIVETYPE_GZ;
	}else if(buf[0] == 'B' && buf[1] == 'Z' && buf[2] == 'h'
		&& buf[4]==0x31 && buf[5]==0x41 && buf[6]==0x59 && buf[7]==0x26 && buf[8]==0x53 && buf[9]==0x59){
		return ARCHIVETYPE_BZ2;
	}else if(buf[0] == (unsigned char)'\037' && buf[1] == (unsigned char)'\235'){
		return ARCHIVETYPE_Z;
	}else if(n >= 6 && CTarArcFile_Lzma::check_head_format(buf,6,ARCHIVETYPE_XZ)){
		return ARCHIVETYPE_XZ;
	}else if(n >= 13 && CTarArcFile_Lzma::check_head_format(buf,13,ARCHIVETYPE_LZMA)){
		return ARCHIVETYPE_LZMA;
	}else if (n >= 4 && CTarArcFile_Zstd::check_head_format(buf, 4)) {
		return ARCHIVETYPE_ZSTD;
	}else{
		return ARCHIVETYPE_NORMAL;
	}
	return -1;
}

