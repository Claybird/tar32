/*
	ArcZstd.H
		Zstandatd file input/output class.
		by ICHIMARU Takeshi(ayakawa.m@gmail.com)
*/
/*
	���̃t�@�C���̗��p�����F
		���̃\�[�X�t�@�C���̗��p�����͈�؂���܂���B
		�\�[�X�̈ꕔ�A�S�������p�A�񏤗p�ȂǖړI��
		�������Ȃ����̃v���O�����Ŏ��R�Ɏg�p�ł��܂��B
		�p�u���b�N�E�h���C���E�\�t�g�E�F�A�Ɠ��l�Ɉ����܂��B

	�v���O���}�����̗v�](�����ł͂���܂���)�F
		�\�[�X���ɉ��P���ׂ��_������΂��m�点���������B
		�\�[�X�R�[�h���Ƀo�O���������ꍇ�͕񍐂��Ă��������B
		�����������Ȃǂ���܂����狳���Ă��������B
		�f�ГI�ȏ��ł����\�ł��B
		���̃t�@�C���𗘗p�����ꍇ�͂Ȃ�ׂ������Ă��������B
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
#include "zstd.h"
#include "tar32api.h" // ARCHIVETYPE_

// default��thread�� 0��blocking mode / 1�ȏと���k�͕ʃX���b�h�ōs���c�c�����܂荷�͏o�Ȃ����A���������������
#define	ZSTD_DEFAULT_THREADS_NUM	0

// Zstandard�̒ʏ�̍ő刳�kLevel
#define ZSTD_NORMAL_MAX_LEVEL	19

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

	int	m_threadNum; // ���k�Ɏg�p����X���b�h��
};
#pragma once
