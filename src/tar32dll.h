/*
	Utility functions, classes for Tar32.dll
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
#include <string>
using namespace std;
#include <wtypes.h>

class CTar32Exception{
public:
	CTar32Exception(const char *str, int code){
		m_str = str;
		m_code = code;
	};
	string m_str;
	int m_code;
};

/* Tar32.DLL Global Critical Section */
void Tar32InitializeCriticalSection();
void Tar32DeleteCriticalSection();
void Tar32EnterCriticalSection();
void Tar32LeaveCriticalSection();


// time_t to CommonArchive-Time(?) converter
WORD GetARCDate(time_t ti);
WORD GetARCTime(time_t ti);
void GetARCAttribute(int st_mode, char *buf, int buflen);
bool GetARCMethod(int archive_type, char *buf, int buf_len);
