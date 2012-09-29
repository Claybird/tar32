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
#include "stdafx.h"
#include "util.h"

int mkdir_recursive(const char *dirname_)
{
	std::string dirname = dirname_;
	int pos = 0;
	int ret;
	
	while(1){
		std::string make_dirname;
		char *ptr = (char*)_mbspbrk((const unsigned char*)dirname.c_str()+pos,(const unsigned char*)"/\\");

		if(ptr==NULL){
			break;
		}

		pos = ptr - dirname.c_str();
		make_dirname = dirname.substr(0,pos);
		ret = _mkdir(make_dirname.c_str());
		pos++;
	}
	ret = _mkdir(dirname.c_str());
	return 0;
}
std::string get_dirname(const char *pathname)
{
	std::string filename = pathname;
	//int pos1 = filename.rfind('/');
	//int pos2 = filename.rfind('\\');
	//int pos = max(pos1 ,pos2);
	//if(pos == string::npos){return "";}
	//string dirname = filename.substr(0, pos);
	// fixed by tsuneo 2001.05.14
	char *p = (char*)max(_mbsrchr((const unsigned char*)pathname,'/'),_mbsrchr((const unsigned char*)pathname,'\\'));
	if(!p){
		return "";
	}
	std::string dirname = filename.substr(0, p - pathname);
	return dirname;
}
std::string get_filename(const char *pathname)
{

	char *p = (char*)max(_mbsrchr((const unsigned char*)pathname,'/'),_mbsrchr((const unsigned char*)pathname,'\\'));
	if(p){
		return std::string(p+1);
	}else{
		return std::string(pathname);
	}
}
bool is_absolute_pathname(const char *fname)
{
	if(*fname == '\\'){return true;}
	if(isalpha(*fname) && fname[1] == ':' && fname[2] == '\\'){return true;}
	return false;
}
std::string make_pathname(const char *dirname_, const char *filename_)
{
	if(*dirname_ == '\0'){return std::string(filename_);}
	if(is_absolute_pathname(filename_)){
		return std::string(filename_);
	}
	std::string pathname = dirname_;
	// if(_mbsrchr((const unsigned char *)dirname_,'\\') == (const unsigned char *)dirname_ + _mbslen((const unsigned char *)dirname_) -1){
	if(_mbsrchr((const unsigned char *)dirname_,'\\') == (const unsigned char *)dirname_ + strlen(dirname_) -1){	// fixed by tsuneo 2001.05.15
		;
	}else{
		pathname.append(1, '\\');
	}
	pathname.append(filename_);
	return pathname;
}

static void escape_filename(std::string &filename)
{
	for(unsigned int i=0;i<filename.length();){
		if(filename[i] == ':'){
			filename[i] = '_';
		}
		i += _mbclen((const unsigned char*)filename.c_str()+i);
	}
}

std::string escape_absolute_paths(const char *pathname_)
{
	while(*pathname_ == '/' || *pathname_== '\\' ){pathname_ ++;}

	std::string esc_path;
	std::string pathname = pathname_;
	int oldpos=0;
	unsigned int pos = 0;
	
	while(pos < pathname.length()){
		std::string fname;

		const char *ptr = (char*)_mbspbrk((const unsigned char*)pathname.c_str()+pos,(const unsigned char*)"/\\");
		if(ptr==NULL){
			ptr = pathname.c_str() + pathname.length();
		}
		pos = ptr - pathname.c_str();
		fname = pathname.substr(oldpos,pos-oldpos);
		escape_filename(fname);
		if(fname == ".."){
			fname = "__";
		}
		esc_path.append(fname);
		if(*ptr=='/' || *ptr=='\\'){
			esc_path.append(1,'/');
		}
		pos++;
		oldpos = pos;
	}
	return esc_path;
}

typedef USHORT dbcschar;	//2�o�C�g������UpperByte*256+LowerByte�A1�o�C�g�����͂��̂܂�
typedef std::basic_string<dbcschar> dbcsstring;

static bool is_regexp_match(const dbcschar* regexp_begin, const dbcschar* regexp_end
					 , const dbcschar* str_begin, const dbcschar* str_end)
{
	switch(*regexp_begin){
	case '*':
		if(str_begin == str_end){return true;}
		return is_regexp_match(regexp_begin+1, regexp_end, str_begin  , str_end) 
			|| is_regexp_match(regexp_begin  , regexp_end, str_begin+1, str_end);
		break;
	case '?':
		if(str_begin == str_end){return false;}
		return is_regexp_match(regexp_begin+1, regexp_end, str_begin+1, str_end);
		break;
	case '\0':
		if(str_begin == str_end){return true;}
		return false;
	default:
		if((*regexp_begin == '/' ||  *regexp_begin == '\\')
			&& (*str_begin == '/' ||  *str_begin == '\\')){
			;
		}else if(*regexp_begin != *str_begin){return false;}
		return is_regexp_match(regexp_begin+1, regexp_end, str_begin+1, str_end);
	}
}
static dbcsstring str2dbcsstring(const char *str)
{
	dbcsstring dbcsstr;
	while(*str){
		dbcschar ch = *str;
		if(::IsDBCSLeadByte(*str)){
			str++;
			ch=ch*256+*str;
		}
		dbcsstr.append(1, ch);
		str++;
	}
	return dbcsstr;
}
bool is_regexp_match_dbcs(const char *regexp, const char *str)
{
	dbcsstring regexp_dbcsstr = str2dbcsstring(regexp);
	dbcsstring str_dbcsstr = str2dbcsstring(str);
	return is_regexp_match(regexp_dbcsstr.c_str(), regexp_dbcsstr.c_str()+regexp_dbcsstr.length()
							, str_dbcsstr.c_str(), str_dbcsstr.c_str()+str_dbcsstr.length());	
}
void convert_yen_to_slash(std::string &pathname)
{
	for(unsigned int i=0;i<pathname.length();){
		if(pathname[i] == '\\'){
			pathname[i] = '/';
		}
		i += _mbclen((unsigned char*)pathname.c_str() + i);
	}
}

void convert_slash_to_backslash(std::string &pathname)
{
	for(unsigned int i=0;i<pathname.length();){
		if(pathname[i] == '/'){
			pathname[i] = '\\';
		}
		i += _mbclen((unsigned char*)pathname.c_str() + i);
	}
}

void get_full_path(const char *pPath,std::string &full)
{
	char buf[_MAX_PATH];
	_fullpath(buf,pPath,COUNTOF(buf));

	full=buf;
}

void find_files(const char *regexp, std::vector<std::string> &files)
{
	//WIN32_FIND_DATA finddata;
	//HANDLE hFindFile;
	long handle;
	struct _finddata_t finddata;
	std::string dirname = get_dirname(regexp);

	handle = _findfirst(regexp, &finddata);
	int ret;
	// hFindFile = ::FindFirstFile(regexp, &finddata);
	ret = (handle != -1);
	while(ret){
		std::string fname = finddata.name;
		if(fname == "." || fname == ".."){
			;
		}else{
			std::string pathname = make_pathname(dirname.c_str(), fname.c_str());
			files.push_back(pathname);
			if((finddata.attrib & _A_SUBDIR) == _A_SUBDIR){
				std::string searchname = make_pathname(pathname.c_str(), "*.*");
				find_files(searchname.c_str(), files);
			}
		}
		ret = (_findnext(handle, &finddata) == 0);
		// ret = ::FindNextFile(hFindFile, finddata);
	}
	if(handle != -1){
		_findclose(handle);
	}
}

//8�i���\�L�̐�������͂���
//�O�̂��ߓ��̓o�b�t�@�T�C�Y���󂯎�邱�ƂƂ���
__int64 parseOctNum(const char *oct,int length)
{
	__int64 num = 0;
	for(int i=0;i<length;i++){
		char c = oct[i];
		//NOTE:�v���O�����̕����R�[�h�Ƃ���ASCII�����肵�Ă���
		if('0'<=c && c<='9'){
			num = num * 8 + (c - '0');
		}
	}
	return num;
}

//http://support.microsoft.com/kb/167296/ja
void UnixTimeToFileTime(time_t t, FILETIME &ft)
{
	// Note that LONGLONG is a 64-bit value
	LONGLONG ll;

	ll = Int32x32To64(t, 10000000) + 116444736000000000;
	ft.dwLowDateTime = (DWORD)ll;
	ft.dwHighDateTime = (DWORD)(ll >> 32);
}

std::string fsizeToString(size64 fsize)
{
	if(fsize==size64(-1))return "---";

	const char* units[]={
		"Byte",
		"KB",
		"MB",
		"GB",
		"TB",
	};

	int level;
	for(level=0;level<COUNTOF(units)-1;level++){
		if(fsize<1024*16)break;
		fsize/=1024;
	}

	char buf[64];
	sprintf(buf,"%I64u %s",fsize,units[level]);
	return std::string(buf);
}


//---�����R�[�h����
//Special Thanks to �S������
static int high_ones(int c) 
{
	int n;
	for (n = 0; (c & 0x80) == 0x80; c <<= 1)
		++n;
	return n;
}

//Special Thanks to �S������
int detect_charset(const char *buf)
{
	int n, remaining_bytes;

	unsigned long line, len, col;
	BYTE multi_byte = 0;
	const char* p = buf;

	len = 1;
	remaining_bytes = 0;
	line = 1;
	col = 1;
	while ( *p )
	{
		if( *p==ESC && *(p+1)=='$' ) return CHARSET_JIS;

		if ((BYTE)*p==(BYTE)0xEF && (BYTE)p[1]==(BYTE)0xBB && (BYTE)p[2]==(BYTE)0xBF)	// BOM
		{
			return CHARSET_UTF8;
		}

		n = high_ones(*p);
		if (remaining_bytes > 0) 
		{
			if (n == 1)
			{
				--remaining_bytes;
				if (remaining_bytes == 0)
					++col;
			}
			else
				goto other_char;
		}
		else if (n == 0)
		{
			// 7-bit character, skip, but adjust position
			if (*p == '\n')
			{
				++line;
				col = 1;
			}
			else
				++col;
		}
		else if (n == 1)
		{
			goto other_char; // wrong place for continuation byte
		}
		else
		{
			remaining_bytes = n - 1; // start of multi-byte sequence
			multi_byte++;
		}

		len++;
		p++;
	}

	if (remaining_bytes > 0)
		goto other_char;

	if (len==col)
		//return CHARSET_SJIS;
		return CHARSET_UNKNOWN;	//�m��ł���
	else if (len>col+1)
		return CHARSET_UTF8N;

other_char:

	int result_char = CHARSET_UNKNOWN;
	unsigned int c, d, e;
	p = buf;
	while ( *p )
	{
		c = *(p++) & 0xFF;
		d = *p & 0xFF;
		e = p[1] && 0xFF;

		if ((c >= 0xa1 && c <= 0xfe) && (d >= 0xa1 && d <= 0xfe))
			result_char = CHARSET_EUCJP;

		if (c==0x8e && (d>=0xa1 && d<=0xdf))	// EUC ���p�̉\����
		{
			if (e==0x8e && !(e>=0xA1 && e<=0xDF))
				return CHARSET_EUCJP;
		}
		else if (c!=0x8e && (d>=0x80 && d<=0xa0))
		{
			return CHARSET_SJIS;		// S-JIS �m��
		}

		if (c==0x8F)
		{
			if ((d>=0xA1 && d<=0xFE) && (e>=0xA1 && e<=0xFE))
			{
				if ( (d==0xFD || d==0xFE) || (e==0xFD || e==0xFE))
					return CHARSET_EUCJP;		// EUC �m��
			}
			else if ((d>=0x80 && d<=0xA0) && (e>=0x80 && e<=0xa0))
			{
				return CHARSET_SJIS;	// S-JIS �m��
			}
		}
	}

	return result_char;
}



//---�����R�[�h�ϊ��w���p
//�����ł�ConvertINetMultiByteToUnicode��ϊ����C�u�����Ɏg�p
CConvertCharsetHelper::CConvertCharsetHelper():
	m_lpfnConvertINetMultiByteToUnicode(NULL),m_hDLL(NULL)
{
}

CConvertCharsetHelper::~CConvertCharsetHelper()
{
	finish();
}

bool CConvertCharsetHelper::init()
{
	finish();

	m_hDLL=LoadLibrary("mlang.dll");
	if(m_hDLL){
		m_lpfnConvertINetMultiByteToUnicode=(LPCONVERTINETMULTIBYTETOUNICODE)GetProcAddress(m_hDLL,"ConvertINetMultiByteToUnicode");
		if(m_lpfnConvertINetMultiByteToUnicode){
			return true;
		}
	}
	finish();
	return false;
}

void CConvertCharsetHelper::finish()
{
	if(m_hDLL){
		m_lpfnConvertINetMultiByteToUnicode=NULL;
		HMODULE hTmp=m_hDLL;
		m_hDLL=NULL;
		FreeLibrary(hTmp);
	}
}

bool CConvertCharsetHelper::utf16_to_sjis(std::string &strRet,LPCWSTR lpcStr)
{
	DWORD dwFlags=WC_DISCARDNS|WC_COMPOSITECHECK|WC_DEFAULTCHAR|WC_NO_BEST_FIT_CHARS;
	std::vector<char> buf(::WideCharToMultiByte(CP_ACP,dwFlags,lpcStr,-1,NULL,0,"_",NULL)+1);	//�o�b�t�@�m��
	//�ϊ�
	if(!::WideCharToMultiByte(CP_ACP,dwFlags,lpcStr,-1,&buf[0],buf.size(),"_",NULL)){
		return false;
	}
	strRet=&buf[0];
	return true;
}

bool CConvertCharsetHelper::utf8_to_utf16(std::wstring &strRet,const char* lpcByte,size_t length)
{
	if(lpcByte[0]==0xEF && lpcByte[1]==0xBB && lpcByte[2]==0xBF){	//BOM check
		lpcByte+=3;
		length-=3;
	}
	std::vector<wchar_t> buf(::MultiByteToWideChar(CP_UTF8,0,(LPCSTR)lpcByte,length,NULL,0)+1);	//�o�b�t�@�m��
	//�ϊ�
	if(!::MultiByteToWideChar(CP_UTF8,0,(LPCSTR)lpcByte,length,&buf[0],buf.size())){
		return false;
	}
	strRet=(LPCWSTR)&buf[0];
	return true;
}

bool CConvertCharsetHelper::utf8_to_sjis(std::string &strRet,const char* lpcByte,size_t length)
{
	std::wstring utf16str;
	if(!utf8_to_utf16(utf16str,lpcByte,length))return false;
	return utf16_to_sjis(strRet,utf16str.c_str());
}

std::string CConvertCharsetHelper::utf8_to_sjis(const char* lpcByte,size_t length)
{
	std::string strRet;
	if(!utf8_to_sjis(strRet,lpcByte,length))strRet=lpcByte;		//���������o��Ō��̕������Ԃ�
	return strRet;
}

bool CConvertCharsetHelper::eucjp_to_utf16(std::wstring &strRet,const char* lpcByte,size_t length)
{
	if(!m_lpfnConvertINetMultiByteToUnicode)return false;
	//�蔲��
	std::vector<wchar_t> buf(length+1);
	DWORD dwMode=0;
	int nWideCharCount=buf.size();
	if(FAILED(m_lpfnConvertINetMultiByteToUnicode(&dwMode,CP_EUCJP,(LPCSTR)lpcByte,NULL,&buf[0],&nWideCharCount)))return false;

	strRet.assign(&buf[0],&buf[0]+nWideCharCount);
	return true;
}

bool CConvertCharsetHelper::eucjp_to_sjis(std::string &strRet,const char* lpcByte,size_t length)
{
	std::wstring utf16str;
	if(!eucjp_to_utf16(utf16str,lpcByte,length))return false;
	return utf16_to_sjis(strRet,utf16str.c_str());
}

std::string CConvertCharsetHelper::eucjp_to_sjis(const char* lpcByte,size_t length)
{
	std::string strRet;
	if(!eucjp_to_sjis(strRet,lpcByte,length))strRet=lpcByte;	//���������o��Ō��̕������Ԃ�
	return strRet;
}

//----
//parses PAX extended header
//pax file type flags are: 'x' and 'g'
//wideFileName contains the original file name in UTF16 if available, otherwise empty
//filesize contains original file size in bytes if available, otherwise -1
bool parsePaxExtHeader(const char* lpContent,size_t length,std::string &strFileName,size64 &filesize,time_t &atime,time_t &ctime,time_t &mtime)
{
	if(!lpContent)return false;
	//pax metadata format:
	//<length of the line in bytes> <key>=<value>\n
	//example:
	//25 ctime=1084839148.1212\n
	strFileName.clear();
	filesize=-1;
	atime=0;
	ctime=0;
	mtime=0;

	const char* lpEnd=lpContent+length;

	for(;lpContent!=lpEnd && *lpContent;){
		int ll=atoi(lpContent);
		if(ll<=0){
			//unformatted data:empty line or invalid length
			return false;
		}
		//looking for first space
		int fs=0;
		for(;fs<ll;fs++){
			if(' '==lpContent[fs])break;
		}
		//looking for '='
		int li=fs+1;
		for(;li<ll;li++){
			if('='==lpContent[li])break;
		}
		if(li==ll){
			//unformatted data:value not found
			return false;
		}
		//retrieve data
		std::string strKey;
		strKey.assign(lpContent+fs+1,lpContent+li);
		const char* lpValue;
		lpValue=lpContent+li+1;

		if("path"==strKey){
			//convert strValue from UTF8 to SJIS
			CConvertCharsetHelper::getInstance().utf8_to_sjis(strFileName,lpValue,ll-li-2);
		}else if("size"==strKey){
			filesize=_atoi64(lpValue);
		}else if("atime"==strKey){
			atime=(time_t)atof(lpValue);
		}else if("ctime"==strKey){
			ctime=(time_t)atof(lpValue);
		}else if("mtime"==strKey){
			mtime=(time_t)atof(lpValue);
		}

		//skip to next line
		lpContent+=ll;
	}

	return true;
}
