/*
	ArcBZ2.H
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

#include "arcfile.h"
#include "bzlib.h"
#include "tar32api.h" // ARCHIVETYPE_

class CTarArcFile_BZip2 : public ITarArcFile{
public:
	CTarArcFile_BZip2();
	~CTarArcFile_BZip2();
	bool open(const char *arcfile, const char *mode, int compress_level);
	size64 read(void *buf, size64 size);
	size64 write(void *buf, size64 size);
	void close();
	int get_archive_type(){return ARCHIVETYPE_BZ2;}
	virtual std::string get_orig_filename();
private:
	BZFILE *m_pbzFile;
};
