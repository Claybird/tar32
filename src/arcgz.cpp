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
#include "zlib.h"
#include <stdlib.h>

CTarArcFile_GZip::CTarArcFile_GZip()
{
	m_gzFile = NULL;
}
CTarArcFile_GZip::~CTarArcFile_GZip()
{
}
bool CTarArcFile_GZip::open(const char *arcfile, const char *mode)
{
	m_arcfile = arcfile;
	gzFile f = gzopen(arcfile, mode);
	m_gzFile = f;
	return (f != NULL);
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
	int ret = gzclose(m_gzFile);
}

string CTarArcFile_GZip::get_orig_filename(){
	if(! m_orig_filename.empty()){return m_orig_filename;}
	if(m_arcfile.length()>3 && stricmp(m_arcfile.substr(m_arcfile.length()-2).c_str(),"gz") == 0){
		return m_arcfile.substr(0, m_arcfile.length()-3);
	}
	return m_arcfile + "_extracted";
}
