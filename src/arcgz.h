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
#include "zlib.h"
#include "tar32api.h" // ARCHIVETYPE_

class CTarArcFile_GZip : public ITarArcFile{
public:
	CTarArcFile_GZip();
	~CTarArcFile_GZip();
	bool open(const char *arcfile, const char *mode);
	int read(void *buf, int size);
	int write(void *buf, int size);
	int seek(int offset, int origin);
	void close();
	int get_archive_type(){return ARCHIVETYPE_GZ;}
	virtual string get_orig_filename();
private:
	gzFile m_gzFile;

	/* gzip flag byte */
	static const GZIP_FLAG_ASCII_FLAG;//   =0x01; /* bit 0 set: file probably ascii text */
	static const GZIP_FLAG_CONTINUATION;// =0x02; /* bit 1 set: continuation of multi-part gzip file */
	static const GZIP_FLAG_EXTRA_FIELD;//  =0x04; /* bit 2 set: extra field present */
	static const GZIP_FLAG_ORIG_NAME;//    =0x08; /* bit 3 set: original file name present */
	static const GZIP_FLAG_COMMENT;//      =0x10; /* bit 4 set: file comment present */
	static const GZIP_FLAG_ENCRYPTED;//    =0x20; /* bit 5 set: file is encrypted */
	static const GZIP_FLAG_RESERVED;//     =0xC0; /* bit 6,7:   reserved */
	static const GZIP_METHOD_DEFLATED;//   =8;

	int		m_gzip_compress_method;
	unsigned 		m_gzip_flags;
	time_t	m_gzip_time_stamp;
	unsigned		m_gzip_ext_flag;
	int		m_gzip_os_type;

	int		m_gzip_part;
	string	m_gzip_orig_name;
	string	m_gzip_comment;
};
