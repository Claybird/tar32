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

typedef USHORT dbcschar;	//2バイト文字はUpperByte*256+LowerByte、1バイト文字はそのまま
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

void find_files(const char *regexp, std::list<std::string> &files)
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

//8進数表記の数字を解析する
//念のため入力バッファサイズを受け取ることとする
__int64 parseOctNum(const char *oct,int length)
{
	__int64 num = 0;
	for(int i=0;i<length;i++){
		char c = oct[i];
		//NOTE:プログラムの文字コードとしてASCIIを仮定している
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
	const char* units[]={
		"Byte",
		"KB",
		"MB",
		"GB",
		"TB",
	};

	int level;
	for(level=0;level<COUNTOF(units);level++){
		if(fsize<1024*16)break;
		fsize/=1024;
	}

	char buf[16];
	sprintf(buf,"%I64u %s",fsize,units[level]);
	return std::string(buf);
}
