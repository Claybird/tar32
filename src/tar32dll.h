/*
	Utility functions, classes for Tar32.dll
		by Yoshioka Tsuneo(QWF00133@nifty.ne.jp)
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
