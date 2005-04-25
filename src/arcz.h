/*
	arcgz.h
		gzip archive input/output class.
		by Yoshioka Tsuneo(QWF00133@nifty.ne.jp)
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
	bool eof();
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
