/*
	ArcZstd.H
		Zstandatd file input/output class.
		by ICHIMARU Takeshi(ayakawa.m@gmail.com)
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

#ifndef _ARCZSTD_H
#define _ARCZSRD_H

#include "arcfile.h"
#include "zstd.h"
#include "tar32api.h" // ARCHIVETYPE_
#include "arczstddef.h"

class CTarArcFile_Zstd : public ITarArcFile {
public:
	CTarArcFile_Zstd();
	~CTarArcFile_Zstd();
	bool open(const char* arcfile, const char* mode, int compress_level);
	size64 read(void* buf, size64 size);
	size64 write(void* buf, size64 size);
	void close();
	int get_archive_type() { return ARCHIVETYPE_ZSTD; }
	virtual std::string get_orig_filename();
	static int check_head_format(unsigned char* buf, size_t buf_size);
	void	set_threads_num(int nThreads);
	void	set_dictionary_filename(const char* filename);
	bool	load_dictionary();
	void	set_train(ZSTD_TRAIN_MODE mode, size_t maxdict);
private:
	ZSTD_CCtx*	m_cctx;  // for compress
	ZSTD_DCtx*	m_dctx;  // for decompress
	size_t	m_buffInSize, m_buffOutSize, m_inOfs, m_outOfs;
	void* m_buffIn;
	void* m_buffOut;
	ZSTD_inBuffer	m_input;
	ZSTD_outBuffer	m_output;
	FILE* m_file;
	bool isWriteMode;
	std::string m_dictionary_filename;
	void* m_dict;
	size_t	m_dict_size;
	ZSTD_TRAIN_MODE	m_train;
	size_t	m_maxdict;

	int	m_threadNum; // 圧縮に使用するスレッド数
};

#endif

