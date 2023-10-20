/*
	ArcZstdDef.H
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

#ifndef _ARCZSTDDEF_H
#define _ARCZSTDDEF_H

// default��thread�� 0��blocking mode / 1�ȏと���k�͕ʃX���b�h�ōs���c�c�����܂荷�͏o�Ȃ����A���������������
#define	ZSTD_DEFAULT_THREADS_NUM	0

// Zstandard�̒ʏ�̍ő刳�kLevel
#define ZSTD_NORMAL_MAX_LEVEL	19

// Zstandard��dictionarty��default �ő�T�C�Y
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
//��O�F�����t�@�C�����Ȃ�
struct ArcFileZstdDictError:CTar32Exception {
	ArcFileZstdDictError() :CTar32Exception("Failed to open proper zstd dict file", ERROR_ARC_FILE_OPEN) {}
	virtual ~ArcFileZstdDictError() {}
};


#endif
