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
#include "arcbz2.h"
#include "util.h"
#include "bzlib.h"
#include <stdlib.h>

CTarArcFile_BZip2::CTarArcFile_BZip2()
{
	m_pbzFile = NULL;
}
CTarArcFile_BZip2::~CTarArcFile_BZip2()
{
	close();
}
bool CTarArcFile_BZip2::open(const char *arcfile, const char *mode)
{
	m_arcfile = arcfile;
	BZFILE * f = bzopen(arcfile, mode);
	m_pbzFile = f;
	return (f != NULL);
}
int CTarArcFile_BZip2::read(void *buf, int size)
{
	return bzread(m_pbzFile, buf, size);
}
int CTarArcFile_BZip2::write(void *buf, int size)
{
	return bzwrite(m_pbzFile, buf, size);
}
void CTarArcFile_BZip2::close()
{
	if(m_pbzFile){
		bzclose(m_pbzFile);
		m_pbzFile = NULL;
	}
}
string CTarArcFile_BZip2::get_orig_filename(){
	if(! m_orig_filename.empty()){return m_orig_filename;}
	string fname = get_filename(m_arcfile.c_str());
	if(fname.length()>4 && stricmp(fname.substr(fname.length()-4).c_str(),".bz2") == 0){
		return fname.substr(0, fname.length()-4);
	}
	return fname + "_extracted";
}

