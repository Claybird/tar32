/*
	normal.h
		normal file input/output class.
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
#include "arcfile.h"
#include <stdio.h>
#include "tar32api.h" // ARCHIVETYPE_

class CTarArcFile_Normal : public ITarArcFile{
public:
	CTarArcFile_Normal::~CTarArcFile_Normal(){
		close();
	};
	bool open(const char *arcfile, const char *mode){
		m_arcfile = arcfile;
		m_fp = fopen(arcfile,mode);
		return (bool)(m_fp!=NULL);
	};
	int read(void *buf, int size){
		return fread(buf,1,size,m_fp);
	};
	int write(void *buf, int size){
		return fwrite(buf,1,size,m_fp);
	};
	int seek(int offset, int origin){
		return fseek(m_fp, offset,origin);
	}
	void close(){
		if(m_fp){
			fclose(m_fp);m_fp=NULL;
		}
	}
	int get_archive_type(){return ARCHIVETYPE_NORMAL;}
private:
	FILE *m_fp;
};