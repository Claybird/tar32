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

int mkdir_recursive(const char *dirname_);
std::string get_dirname(const char *pathname_);
std::string get_filename(const char *pathname);
std::string make_pathname(const char *dirname_, const char *filename_);
std::string escape_absolute_paths(const char *pathname_);
void convert_yen_to_slash(std::string &pathname);
void convert_slash_to_backslash(std::string &pathname);
void get_full_path(const char *pPath,std::string &full);
bool is_regexp_match_dbcs(const char *regexp, const char *str);
void find_files(const char *regexp, std::vector<std::string> &files);
__int64 parseOctNum(const char *oct,int length);
void UnixTimeToFileTime(time_t t, FILETIME &ft);
std::string fsizeToString(size64 fsize);

//---�����R�[�h�ϊ�
class CConvertCharsetHelper{
protected:
	typedef HRESULT ( APIENTRY *LPCONVERTINETMULTIBYTETOUNICODE )(LPDWORD,DWORD,LPCSTR,LPINT,LPWSTR,LPINT);
	LPCONVERTINETMULTIBYTETOUNICODE m_lpfnConvertINetMultiByteToUnicode;
	HMODULE m_hDLL;
protected:
	CConvertCharsetHelper();
public:
	virtual ~CConvertCharsetHelper();
	static CConvertCharsetHelper& getInstance(){	//singleton
		static CConvertCharsetHelper instance;
		return instance;
	}
	bool init();
	void finish();

	bool utf16_to_sjis(std::string &strRet,LPCWSTR lpcStr);
	bool utf8_to_utf16(std::wstring &strRet,const char* lpcByte,size_t length);
	bool utf8_to_sjis(std::string &strRet,const char* lpcByte,size_t length);
	std::string utf8_to_sjis(const char* lpcByte,size_t length);
	bool eucjp_to_utf16(std::wstring &strRet,const char* lpcByte,size_t length);
	bool eucjp_to_sjis(std::string &strRet,const char* lpcByte,size_t length);
	std::string eucjp_to_sjis(const char* lpcByte,size_t length);
};

#ifndef CP_EUCJP
#define CP_EUCJP	51932
#endif

//---�����R�[�h����
#define ESC 				0x1B	/* �G�X�P�[�v�R�[�h */
#define CHARSET_DONTCARE	0	//�����R�[�h�𔻒肵�Ȃ�
#define CHARSET_UNKNOWN		1
#define CHARSET_SJIS		2
#define CHARSET_JIS			3
#define CHARSET_EUCJP		4
#define CHARSET_UTF8N		5
#define CHARSET_UTF8		6

int detect_charset(const char *buf);

//parses PAX extended header
bool parsePaxExtHeader(const char* lpContent,size_t length,std::string &strFileName,size64 &filesize,time_t &atime,time_t &ctime,time_t &mtime);
