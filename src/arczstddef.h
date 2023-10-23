/*
	ArcZstdDef.H
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

#ifndef _ARCZSTDDEF_H
#define _ARCZSTDDEF_H

// defaultのthread数 0→blocking mode / 1以上→圧縮は別スレッドで行う……があまり差は出ないし、メモリをがっつり喰う
#define	ZSTD_DEFAULT_THREADS_NUM	0

// Zstandardの通常の最大圧縮Level
#define ZSTD_NORMAL_MAX_LEVEL	19

// Zstandardのdictionartyのdefault 最大サイズ
#define ZSTD_DEFAULT_MAXDICT	112640

// train mode
enum ZSTD_TRAIN_MODE {
	zt_none,			// compress/decompress
	//zt_train,           // default mode -> same as fastcover
	zt_train_cover,     // cover mode
	zt_train_fastcover, // fast cover mode
	zt_train_legacy		// legacy mode
};

// 
struct ExtraTarArcFileOptions {
	ExtraTarArcFileOptions() {
		zstd_thread_num = 0;
		zstd_dictionary_filename = "";
		zstd_train = zt_none;
		zstd_maxdict = ZSTD_DEFAULT_MAXDICT;
	}

	int				zstd_thread_num;
	std::string		zstd_dictionary_filename;
	ZSTD_TRAIN_MODE	zstd_train;
	size_t			zstd_maxdict;
};

#include "tar32dll.h"
#include "tar32api.h"
//例外：辞書ファイルがない
struct ArcFileZstdDictError:CTar32Exception {
	ArcFileZstdDictError() :CTar32Exception("Failed to open proper zstd dict file", ERROR_ARC_FILE_OPEN) {}
	virtual ~ArcFileZstdDictError() {}
};


#endif
