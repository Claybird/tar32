/*
	normal.h
		normal file input/output class.
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

class CTarArcFile_Normal : public ITarArcFile{
public:
	CTarArcFile_Normal::~CTarArcFile_Normal(){
		close();
	};
	bool open(const char *arcfile, const char *mode, int /*compress_level*/){
		m_arcfile = arcfile;
		m_fp = fopen(arcfile,mode);
		if(m_fp)setvbuf(m_fp,NULL,_IOFBF,1024*1024);
		return m_fp!=NULL;
	};
	size64 read(void *buf, size64 size){
		return fread(buf,1,(size_t)size,m_fp);	//TODO:size lost
	};
	size64 write(void *buf, size64 size){
		return fwrite(buf,1,(size_t)size,m_fp);	//TODO:size lost
	};
	size64 seek(size64 offset, int origin){
		return _fseeki64(m_fp, offset,origin);
	}
	void close(){
		if(m_fp){
			fclose(m_fp);m_fp=NULL;
		}
	}
	int get_archive_type(){return ARCHIVETYPE_NORMAL;}
private:
	FILE *m_fp;
};