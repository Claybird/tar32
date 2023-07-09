/*
	ArcZstd.cpp
		zstandard file input/output class.
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
#include "stdafx.h"
#include "arczstd.h"
#include "util.h"
#include "zstd.h"

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

bool CTarArcFile_Zstd::open(const char* arcfile, const char* mode, int compress_level)
{
	close();
	bool allOK = false;
	m_arcfile = arcfile;
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
		isWriteMode = false;
	}
	return allOK;
}

size64 CTarArcFile_Zstd::read(void* buf, size64 size)
{
	size64 copiedSize = 0;
	while (copiedSize < size) {
		// m_output.dst�̎c�肪����H
		if (m_output.pos <= m_outOfs) {
			// ����m_output.dst���g���؂����̂ŁA���̃f�[�^��W�J����B
			m_output = { m_buffOut, 0, 0 };
			m_outOfs = 0;
			// m_input.src�Ɏc�肪����H
			if (m_input.size <= m_input.pos) {
				// ����m_input.src���g���؂����̂�arcfile���玟�̃f�[�^��ǂݍ���
				m_input = { m_buffIn, 0, 0 };
				if (feof(m_file)) {
					// ����EOF�Ȃ̂ł���ȏ�ǂݍ��߂Ȃ��I
					break; // return copiedSize;
				}
				// arcfile����f�[�^��ǂݍ���
				size_t read = fread(m_buffIn, 1, m_buffInSize, m_file);
				if (read == 0) {
					// �f�[�^������ �� �Ȃ񂩃G���[
					break; // return copiedSize;
				}
				// �f�[�^��W�J����p��
				m_input = { m_buffIn, read, 0 };
			}
			// �f�[�^��W�J
			m_output = { m_buffOut, m_buffOutSize, 0 };
			size_t const ret = ZSTD_decompressStream(m_dctx, &m_output, &m_input);
			if (ZSTD_isError(ret)) {
				// �G���[����
				m_input = { m_buffIn, 0, 0 };
				m_output = { m_buffOut, 0, 0 };
				m_outOfs = 0;
				break; // return copiedSize;
			}
		}
		// m_output.dst����R�s�[ 
		size64 need_copy_size = size - copiedSize;        // ���ݕK�v�Ƃ����R�s�[�T�C�Y���v�Z
		size64 remain_buf_size = m_output.pos - m_outOfs; // ���݃o�b�t�@�Ɏc���Ă���T�C�Y���v�Z
		size64 copy_size = need_copy_size < remain_buf_size ? need_copy_size : remain_buf_size; // �����������̗p
		memcpy((BYTE *)buf + copiedSize, (BYTE *)(m_output.dst) + m_outOfs, (size_t)copy_size);
		m_outOfs += (size_t)copy_size;    // m_output.dst�̎g�p�ς݃�����offset�X�V
		copiedSize += copy_size;  // �R�s�[���ꂽ�o�C�g�ʂ��X�V
	};
	return copiedSize;
}

size64 CTarArcFile_Zstd::write(void* buf, size64 size)
{
	size64 copiedSize = 0;
	while (copiedSize < size) {
		size64 buff_remain_size = m_input.size - m_inOfs; // m_input.src�̋󂫗e��
		if (buff_remain_size > 0) {
			// input�pbuff�ɋ󂫂�����̂ň��k�p���[�N�ɃR�s�[
			size64 copy_remain_size = size - copiedSize;  // �����o�����̎c��T�C�Y
			size64 copy_size = buff_remain_size < copy_remain_size ? buff_remain_size : copy_remain_size; // �����������̗p
			memcpy((BYTE*)(m_input.src) + m_inOfs, (BYTE*)buf + copiedSize, (size_t)copy_size);
			m_inOfs += (size_t)copy_size;
			copiedSize += copy_size;
		};
		// ���k�p���[�N�����܂����H
		if (m_inOfs == m_input.size) {
			// ���܂����̂ň��k�I
			do {
				m_output = { m_buffOut, m_buffOutSize, 0 };
				size_t const ret = ZSTD_compressStream2(m_cctx, &m_output, &m_input, ZSTD_e_continue); 
				if (!ZSTD_isError(ret)) {
					// ���k���������̂ŏ�������
					size_t writeSize = fwrite(m_output.dst, 1, m_output.pos, m_file);
					if (writeSize < m_output.pos) {
						// �������ݎ��s
						m_input = { m_buffIn, 0, 0 };
						m_inOfs = 0;
						return copiedSize;
					}
				}
				else {
					// ���k���s
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
			// ���k�p���[�N���ď�����
			m_input = { m_buffIn, m_buffInSize, 0 };
			m_inOfs = 0;
		}
	}

	return copiedSize;
}

void CTarArcFile_Zstd::close()
{
	if (isWriteMode) {
		// input�pbuffer����ł��Ō���m�肷�邽�߂Ɏ��s����K�v������
		size_t ret;
		m_input = { m_buffIn, m_inOfs, 0 };
		do {
			m_output = { m_buffOut, m_buffOutSize, 0 };
			ret = ZSTD_compressStream2(m_cctx, &m_output, &m_input, ZSTD_e_end); // �Ō�m��
			if (ZSTD_isError(ret)) {
				// �G���[���������Ă������ł͉����o���Ȃ�
				break;
			}
			// �Ō�̏�������
			fwrite(m_output.dst, 1, m_output.pos, m_file); // ���ꂪ�G���[�ł������o���Ȃ�
		} while (ret != 0);
		// ��n��
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
