/*
	ArcBZ2.cpp
		bzip2 file input/output class.
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
#include "arcbz2.h"
#include "util.h"
#include "bzlib.h"

CTarArcFile_BZip2::CTarArcFile_BZip2()
{
	m_pbzFile = NULL;
	m_file = NULL;
	isWriteMode = false;
}
CTarArcFile_BZip2::~CTarArcFile_BZip2()
{
	close();
}
bool CTarArcFile_BZip2::open(const char *arcfile, const char *mode, int compress_level)
{
	close();
	m_arcfile = arcfile;
	bool bReadMode=(NULL!=strchr(mode,'r'));
	int error = BZ_OK;
	if (bReadMode) {
		m_file = fopen(arcfile, "rb");
		if (m_file) {
			m_pbzFile = BZ2_bzReadOpen(&error, m_file, 0, 0, NULL, 0);
		}
		isWriteMode = false;
	}else{
		m_file = fopen(arcfile, "wb");
		if (m_file) {
			int workFactor = 30;
			m_pbzFile = BZ2_bzWriteOpen(&error, m_file, compress_level, 0, workFactor);
		}
		isWriteMode = true;
	}
	return (m_pbzFile != NULL);
}

static bool myfeof(FILE* f)
{
	int c = fgetc(f);
	if (c == EOF) return true;
	ungetc(c, f);
	return false;
}

size64 CTarArcFile_BZip2::read(void *buf, size64 size)
{
	size64 read = 0;
	while (true) {
		int error = BZ_OK;
		read += BZ2_bzRead(&error, m_pbzFile, ((unsigned char*)buf)+read, (int)(size-read));
		if (error == BZ_OK) {
			return read;
		} else if (error == BZ_STREAM_END) {
			if (ferror(m_file))return read;	//error

			void* unusedTmpV;
			int nUnused;
			BZ2_bzReadGetUnused(&error, m_pbzFile, &unusedTmpV, &nUnused);
			if (error != BZ_OK) return read;	//error
			unsigned char* unusedTmp = (unsigned char*)unusedTmpV;
			unsigned char unused[BZ_MAX_UNUSED];
			for (int i = 0; i < nUnused; i++) unused[i] = unusedTmp[i];

			BZ2_bzReadClose(&error, m_pbzFile);
			if (error != BZ_OK) return read;	//error

			if (nUnused == 0 && myfeof(m_file)) return read;

			m_pbzFile = BZ2_bzReadOpen(&error, m_file, 0, 0, unused, nUnused);
			if (error != BZ_OK) return read;	//error

			if (read >= size) {
				return read;
			}
		} else {
			//error
			return read;
		}
	}
}
size64 CTarArcFile_BZip2::write(void *buf, size64 size)
{
	int error = BZ_OK;
	BZ2_bzWrite(&error, m_pbzFile, buf, (int)size);
	return size;
}
void CTarArcFile_BZip2::close()
{
	if(m_pbzFile){
		int error = BZ_OK;
		if (isWriteMode) {
			BZ2_bzWriteClose(&error, m_pbzFile, 0, NULL, NULL);
			if (error != BZ_OK) {
				BZ2_bzWriteClose(&error, m_pbzFile, 1, NULL, NULL);
			}
		} else {
			BZ2_bzReadClose(&error, m_pbzFile);
		}
		m_pbzFile = NULL;
	}
	if (m_file) {
		fclose(m_file);
		m_file = NULL;
	}
	isWriteMode = false;
}
std::string CTarArcFile_BZip2::get_orig_filename(){
	if(! m_orig_filename.empty()){return m_orig_filename;}
	std::string fname = get_filename(m_arcfile.c_str());
	if(fname.length()>4 && stricmp(fname.substr(fname.length()-4).c_str(),".bz2") == 0){
		return fname.substr(0, fname.length()-4);
	}
	return fname + "_extracted";
}

