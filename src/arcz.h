/*
	arcgz.h
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

#include "arcfile.h"
#include "tar32api.h" // ARCHIVETYPE_
#include "fast_stl.h"
// #include <sstream>
using namespace std;

class CTarArcFile_Compress : public ITarArcFile{
public:
	CTarArcFile_Compress();
	~CTarArcFile_Compress();
	bool open(const char *arcfile, const char *mode);
	int read(void *buf, int size);
	int write(void *buf, int size){return -1;}
	// int seek(int offset, int origin);
	void close();
	int get_archive_type(){return ARCHIVETYPE_Z;}
	virtual string get_orig_filename();

	static const unsigned char MAGIC_1;// = '\037';/* First byte of compressed file	*/
	static const unsigned char MAGIC_2;// = '\235';/* Second byte of compressed file	*/
private:
	  bool readonce();
	FILE *m_pFile;
	int m_maxbits;
#define HSIZE 1 << 17
	// static const int HSIZE;//	= 1<<17;
	unsigned short	m_codetab[HSIZE];
	unsigned char   m_htab[HSIZE];
	unsigned char m_inbuf[BUFSIZ + 64];
	bool m_eof;
	fast_strstream m_strstream;
	int				m_finchar;
	int				m_oldcode;
	int				m_posbits;
	int				m_free_ent;
	int				m_maxcode;
	int				m_n_bits;
	int				m_rsize;
	int				m_insize;
	bool			m_block_mode;
};
