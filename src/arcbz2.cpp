/*
	ArcBZ2.cpp
		bzip2 file input/output class.
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
#include "arcbz2.h"
#include "util.h"
#include "bzlib.h"
#include <stdlib.h>

CTarArcFile_BZip2::CTarArcFile_BZip2()
{
	m_pbzFile = NULL;
}
CTarArcFile_BZip2::~CTarArcFile_BZip2()
{
	close();
}
bool CTarArcFile_BZip2::open(const char *arcfile, const char *mode)
{
	m_arcfile = arcfile;
	BZFILE * f = BZ2_bzopen(arcfile, mode);
	m_pbzFile = f;
	return (f != NULL);
}
int CTarArcFile_BZip2::read(void *buf, int size)
{
	return BZ2_bzread(m_pbzFile, buf, size);
}
int CTarArcFile_BZip2::write(void *buf, int size)
{
	return BZ2_bzwrite(m_pbzFile, buf, size);
}
void CTarArcFile_BZip2::close()
{
	if(m_pbzFile){
		BZ2_bzclose(m_pbzFile);
		m_pbzFile = NULL;
	}
}
string CTarArcFile_BZip2::get_orig_filename(){
	if(! m_orig_filename.empty()){return m_orig_filename;}
	string fname = get_filename(m_arcfile.c_str());
	if(fname.length()>4 && stricmp(fname.substr(fname.length()-4).c_str(),".bz2") == 0){
		return fname.substr(0, fname.length()-4);
	}
	return fname + "_extracted";
}

