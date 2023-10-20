/*
	ArcZstd.cpp
		zstandard file input/output class.
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
#include "stdafx.h"
#include "arczstd.h"
#include "util.h"
#include "zstd.h"
#include "zstd_errors.h"
#include <iostream>
#include <fstream>

using namespace std;

CTarArcFile_Zstd::CTarArcFile_Zstd()
{
	m_cctx = NULL;
	m_dctx = NULL;
	m_buffIn = NULL;
	m_buffInSize = 0;
	m_buffOut = NULL;
	m_buffOutSize = 0;
	m_inOfs = 0;
	m_outOfs = 0;
	m_file = NULL;
	isWriteMode = false;
	m_threadNum = ZSTD_DEFAULT_THREADS_NUM;
	m_dict = NULL;
	m_dict_size = 0;
	m_dictionary_filename = "";
	m_train = zt_none;
	m_maxdict = 0;
}
CTarArcFile_Zstd::~CTarArcFile_Zstd()
{
	close();
}

void CTarArcFile_Zstd::set_threads_num(int nThreads)
{
	if (nThreads < 0) {
		m_threadNum = 0;
	}
	else {
		m_threadNum = nThreads;
	};
}

void CTarArcFile_Zstd::set_dictionary_filename(const char* filename)
{
	m_dictionary_filename = std::string(filename);
	load_dictionary();
}

void CTarArcFile_Zstd::reopen_with_dictionary(const char* filename)
{
	m_dictionary_filename = std::string(filename);
	open(m_arcfile.c_str(), "r", -1);
}

bool CTarArcFile_Zstd::load_dictionary()
{
	if (m_dictionary_filename.empty()) return true;
	//
	if (PathFileExists(m_dictionary_filename.c_str()) && !PathIsDirectory(m_dictionary_filename.c_str()))
	{
		struct _stati64 st;
		if (_stati64(m_dictionary_filename.c_str(), &st) != 0) return false;
		//
		m_dict_size = (size_t)st.st_size;
		m_dict = (void*)new char[m_dict_size];

		ifstream fin(m_dictionary_filename.c_str(), ios::in | ios::binary);
		if (!fin) {
			delete[] m_dict;
			m_dict = NULL;
			m_dict_size = 0;
			return false;
		}
		fin.read((char*)m_dict, m_dict_size);
		if (fin.fail()) {
			delete[] m_dict;
			m_dict = NULL;
			m_dict_size = 0;
			return false;
		}
		fin.close();
		return true;
	}
	else
		return false;
}

void CTarArcFile_Zstd::set_train(ZSTD_TRAIN_MODE mode, size_t maxdict)
{
	m_train = mode;
	m_maxdict = maxdict;
}

bool CTarArcFile_Zstd::open(const char* arcfile, const char* mode, int compress_level)
{
	close();
	bool allOK = false;
	m_arcfile = arcfile;

	if (!load_dictionary()) return false;

	bool bReadMode = (NULL != strchr(mode, 'r'));
	if (bReadMode) {
		// decompress
		m_file = fopen(arcfile, "rb");
		if (m_file) {
			m_buffInSize = ZSTD_DStreamInSize();
			m_buffOutSize = ZSTD_DStreamOutSize();
			m_buffIn = new char[m_buffInSize];
			m_buffOut = new char[m_buffOutSize];
			m_dctx = ZSTD_createDCtx();
			if (m_dctx && m_dict) {
				size_t result = ZSTD_DCtx_loadDictionary(m_dctx, m_dict, m_dict_size);
				allOK = !ZSTD_isError(result);
			}
			else
				allOK = (m_dctx != NULL);
			if (allOK) {
				m_input = { m_buffIn, 0, 0 };
				m_output = { m_buffOut, 0, 0 };
				m_outOfs = 0;
			}
			isWriteMode = false;
		};
	}
	else {
		// compress
		m_file = fopen(arcfile, "wb");
		if (m_file) {
			m_buffInSize = ZSTD_CStreamInSize();
			m_buffOutSize = ZSTD_CStreamOutSize();
			m_buffIn = new char[m_buffInSize];
			m_buffOut = new char[m_buffOutSize];
			m_cctx = ZSTD_createCCtx();
			if (m_cctx != NULL) {
				bool bDictOk = true;
				if (m_dict) {
					size_t result = ZSTD_CCtx_loadDictionary(m_cctx, m_dict, m_dict_size);
					bDictOk = !ZSTD_isError(result);
				}
				if (bDictOk) {
					if (compress_level < ZSTD_minCLevel()) {
						compress_level = ZSTD_minCLevel();
					}
					else if (compress_level > ZSTD_maxCLevel()) {
						compress_level = ZSTD_maxCLevel();
					};
					if (!ZSTD_isError(ZSTD_CCtx_setParameter(m_cctx, ZSTD_c_compressionLevel, compress_level))) {
						if (!ZSTD_isError(ZSTD_CCtx_setParameter(m_cctx, ZSTD_c_checksumFlag, 1))) {
							ZSTD_CCtx_setParameter(m_cctx, ZSTD_c_nbWorkers, m_threadNum); // 0=blocking mode
							m_input = { m_buffIn, m_buffInSize, 0 };
							m_output = { m_buffOut, 0, 0 };
							isWriteMode = true;
							m_inOfs = 0;
							allOK = true;
						};
					};
				};
			};
		};
	}
	if (!allOK) {
		if (m_dctx) {
			ZSTD_freeDCtx(m_dctx);
			m_dctx = NULL;
		}
		if (m_cctx) {
			ZSTD_freeCCtx(m_cctx);
			m_cctx = NULL;
		}
		if (m_buffIn) {
			delete[] m_buffIn;
			m_buffIn = NULL;
			m_buffInSize = 0;
		}
		if (m_buffOut) {
			delete[] m_buffOut;
			m_buffOut = NULL;
			m_buffOutSize = 0;
		}
		if (m_file) {
			fclose(m_file);
			m_file = NULL;
		}
		if (m_dict) {
			delete[] m_dict;
			m_dict = NULL;
			m_dict_size = 0;
		}
		isWriteMode = false;
	}
	return allOK;
}

size64 CTarArcFile_Zstd::read(void* buf, size64 size)
{
	size64 copiedSize = 0;
	while (copiedSize < size) {
		// m_output.dstの残りがある？
		if (m_output.pos <= m_outOfs) {
			// 既にm_output.dstを使い切ったので、次のデータを展開する。
			m_output = { m_buffOut, 0, 0 };
			m_outOfs = 0;
			// m_input.srcに残りがある？
			if (m_input.size <= m_input.pos) {
				// 既にm_input.srcを使い切ったのでarcfileから次のデータを読み込む
				m_input = { m_buffIn, 0, 0 };
				if (feof(m_file)) {
					// 既にEOFなのでこれ以上読み込めない！
					break; // return copiedSize;
				}
				// arcfileからデータを読み込む
				size_t read = fread(m_buffIn, 1, m_buffInSize, m_file);
				if (read == 0) {
					// データが無い → なんかエラー
					break; // return copiedSize;
				}
				// データを展開する用意
				m_input = { m_buffIn, read, 0 };
			}
			// データを展開
			m_output = { m_buffOut, m_buffOutSize, 0 };
			size_t const ret = ZSTD_decompressStream(m_dctx, &m_output, &m_input);
			if (ZSTD_isError(ret)) {
				if (ret == -ZSTD_error_dictionary_wrong) {
					//辞書が不正; 交換を促す
					throw ArcFileZstdDictError();
				} else {
					// エラー発生
					m_input = { m_buffIn, 0, 0 };
					m_output = { m_buffOut, 0, 0 };
					m_outOfs = 0;
					break; // return copiedSize;
				}
			}
		}
		// m_output.dstからコピー 
		size64 need_copy_size = size - copiedSize;        // 現在必要とされるコピーサイズを計算
		size64 remain_buf_size = m_output.pos - m_outOfs; // 現在バッファに残っているサイズを計算
		size64 copy_size = need_copy_size < remain_buf_size ? need_copy_size : remain_buf_size; // 小さい方を採用
		memcpy((BYTE *)buf + copiedSize, (BYTE *)(m_output.dst) + m_outOfs, (size_t)copy_size);
		m_outOfs += (size_t)copy_size;    // m_output.dstの使用済みメモリoffset更新
		copiedSize += copy_size;  // コピーされたバイト量を更新
	};
	return copiedSize;
}

size64 CTarArcFile_Zstd::write(void* buf, size64 size)
{
	size64 copiedSize = 0;
	while (copiedSize < size) {
		size64 buff_remain_size = m_input.size - m_inOfs; // m_input.srcの空き容量
		if (buff_remain_size > 0) {
			// input用buffに空きがあるので圧縮用ワークにコピー
			size64 copy_remain_size = size - copiedSize;  // 書き出す分の残りサイズ
			size64 copy_size = buff_remain_size < copy_remain_size ? buff_remain_size : copy_remain_size; // 小さい方を採用
			memcpy((BYTE*)(m_input.src) + m_inOfs, (BYTE*)buf + copiedSize, (size_t)copy_size);
			m_inOfs += (size_t)copy_size;
			copiedSize += copy_size;
		};
		// 圧縮用ワークが埋まった？
		if (m_inOfs == m_input.size) {
			// 埋まったので圧縮！
			do {
				m_output = { m_buffOut, m_buffOutSize, 0 };
				size_t const ret = ZSTD_compressStream2(m_cctx, &m_output, &m_input, ZSTD_e_continue); 
				if (!ZSTD_isError(ret)) {
					// 圧縮成功したので書き込み
					size_t writeSize = fwrite(m_output.dst, 1, m_output.pos, m_file);
					if (writeSize < m_output.pos) {
						// 書き込み失敗
						m_input = { m_buffIn, 0, 0 };
						m_inOfs = 0;
						return copiedSize;
					}
				}
				else {
					// 圧縮失敗
					/* if setting 32bit compress_level>=20 uusing threads, we must fail...
					char dbgbuff[200];
					sprintf_s(dbgbuff, 200, "ret = %u / %s\n", ret, ZSTD_getErrorName(ret));
					OutputDebugString(dbgbuff);
					*/
					m_input = { m_buffIn, 0, 0 };
					m_inOfs = 0;
					return copiedSize;
				}
			} while (m_input.pos != m_input.size);
			// 圧縮用ワークを再初期化
			m_input = { m_buffIn, m_buffInSize, 0 };
			m_inOfs = 0;
		}
	}

	return copiedSize;
}

void CTarArcFile_Zstd::close()
{
	if (isWriteMode) {
		// input用bufferが空でも最後を確定するために実行する必要がある
		size_t ret;
		m_input = { m_buffIn, m_inOfs, 0 };
		do {
			m_output = { m_buffOut, m_buffOutSize, 0 };
			ret = ZSTD_compressStream2(m_cctx, &m_output, &m_input, ZSTD_e_end); // 最後確定
			if (ZSTD_isError(ret)) {
				// エラーが発生してもここでは何も出来ない
				break;
			}
			// 最後の書き込み
			fwrite(m_output.dst, 1, m_output.pos, m_file); // これがエラーでも何も出来ない
		} while (ret != 0);
		// 後始末
		if (m_cctx) {
			ZSTD_freeCCtx(m_cctx);
			m_cctx = NULL;
		};
		if (m_buffIn) {
			delete[] m_buffIn;
			m_buffIn = NULL;
		}
		if (m_buffOut) {
			delete[] m_buffOut;
			m_buffOut = NULL;
		}
		m_inOfs = 0;
	}
	else {
		if (m_dctx) {
			ZSTD_freeDCtx(m_dctx);
			m_dctx = NULL;
		};
		if (m_buffIn) {
			delete[] m_buffIn;
			m_buffIn = NULL;
		}
		if (m_buffOut) {
			delete[] m_buffOut;
			m_buffOut = NULL;
		}
		m_outOfs = 0;
	}
	m_buffInSize = 0;
	m_buffOutSize = 0;
	if (m_file) {
		fclose(m_file);
		m_file = NULL;
	}
	if (m_dict) {
		delete[] m_dict;
		m_dict = NULL;
		m_dict_size = 0;
	}
	isWriteMode = false;
}

std::string CTarArcFile_Zstd::get_orig_filename() {
	if (!m_orig_filename.empty()) { return m_orig_filename; }
	std::string fname = get_filename(m_arcfile.c_str());
	if (fname.length() > 5 && stricmp(fname.substr(fname.length() - 5).c_str(), ".zstd") == 0) {
		return fname.substr(0, fname.length() - 5);
	}
	if (fname.length() > 4 && stricmp(fname.substr(fname.length() - 4).c_str(), ".zst") == 0) {
		return fname.substr(0, fname.length() - 4);
	}
	return fname + "_extracted";
}

int CTarArcFile_Zstd::check_head_format(unsigned char* buf, size_t buf_size)
{
	// magic_number 0xFD2FB528(Standard Frame) or 0x184D2A5(0-F)(Skip Frame) or 0xEC30A437(Dictionary Frame)
	return (buf_size >= 4) &&
		(buf[0] == 0x28 && buf[1] == 0xB5 && buf[2] == 0x2F && buf[3] == 0xFD) ||
		((buf[0] & 0xF0) == 0x50 && buf[1] == 0x2A && buf[2] == 0x4D && buf[3] == 0x18) ||
		(buf[0] == 0x37 && buf[1] == 0xA4 && buf[2] == 0x30 && buf[3] == 0xEC);
}
