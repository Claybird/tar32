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
#include "util.h"
#include <string>
#include <direct.h> // _mkdir
#include <wtypes.h>
#include <winnls.h>
#include <io.h>
#include <mbstring.h>

int mkdir_recursive(const char *dirname_)
{
	string dirname = dirname_;
	int pos = 0;
	int ret;
	
	while(pos != string::npos){
		string make_dirname = dirname.substr(0,pos);
		ret = _mkdir(make_dirname.c_str());
		int pos1 = dirname.find('/',pos+1);
		int pos2 = dirname.find('\\',pos+1);
		if(pos1 == string::npos && pos2 == string::npos){
			pos = string::npos;
		}else if(pos1 == string::npos && pos2 != string::npos){
			pos = pos2;
		}else if(pos1 != string::npos && pos2 == string::npos){
			pos = pos1;
		}else if(pos1 != string::npos && pos2 != string::npos){
			pos = min(pos1, pos2);
		}
	}
	ret = _mkdir(dirname.c_str());
	return 0;
}
string get_dirname(const char *pathname_)
{
	string filename = pathname_;
	int pos1 = filename.rfind('/');
	int pos2 = filename.rfind('\\');
	int pos = max(pos1 ,pos2);
	if(pos == string::npos){return "";}
	string dirname = filename.substr(0,pos);
	return dirname;
}
string get_filename(const char *pathname)
{

	char *p = (char*)max(_mbsrchr((const unsigned char*)pathname,'/'),_mbsrchr((const unsigned char*)pathname,'\\'));
	if(p){
		return string(p+1);
	}else{
		return string(pathname);
	}
}
bool is_absolute_pathname(const char *fname)
{
	if(*fname == '\\'){return true;}
	if(isalpha(*fname) && fname[1] == ':' && fname[2] == '\\'){return true;}
	return false;
}
string make_pathname(const char *dirname_, const char *filename_)
{
	if(*dirname_ == '\0'){return string(filename_);}
	if(is_absolute_pathname(filename_)){
		return string(filename_);
	}
	string pathname = dirname_;
	if(_mbsrchr((const unsigned char *)dirname_,'\\') == (const unsigned char *)dirname_ + _mbslen((const unsigned char *)dirname_) -1){
		;
	}else{
		pathname.append(1, '\\');
	}
	pathname.append(filename_);
	return pathname;
}


class dbcschar{
public:
	dbcschar(const char *str){
		if(str == NULL){highbyte = 0; lowbyte = 0;return;}
		if(::IsDBCSLeadByte(*str) && *(str+1)){
			highbyte = *(str);
			lowbyte  = *(str+1);
		}else{
			highbyte = 0;
			lowbyte  = *str;
		}
	};
	bool isnull(){
		return (highbyte == 0) && (lowbyte == 0);
	}
	operator int(){
		return highbyte*256 + lowbyte;
	}
private:
	char highbyte;
	char lowbyte;
	friend bool operator==(dbcschar c1, dbcschar c2);
};
bool operator==(dbcschar c1, dbcschar c2)
{
	/* ignore case comaration */
	if(c1.highbyte == 0 && c2.highbyte == 0){
		return tolower(c1.lowbyte) == tolower(c2.lowbyte);
	}else{
		return c1.highbyte == c2.highbyte && c1.lowbyte == c2.lowbyte;
	}
}
/*
	ASCII-Caracter	=> 0x00cc
	HANKAKU-KANA    => 0x00cc
	2 byte KANJI    => 0xcccc
*/
typedef basic_string<dbcschar> dbcsstring;

static bool is_regexp_match(dbcsstring::iterator regexp_begin, dbcsstring::iterator regexp_end
					 , dbcsstring::iterator str_begin, dbcsstring::iterator str_end)
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
		dbcschar ch = str;
		// ch.str2dbcschar(str);
		dbcsstr.append(1, ch);
		if(::IsDBCSLeadByte(*str)){
			str++;
		}
		str++;
	}
	return dbcsstr;
}
bool is_regexp_match_dbcs(const char *regexp, const char *str)
{
	dbcsstring regexp_dbcsstr = str2dbcsstring(regexp);
	dbcsstring str_dbcsstr = str2dbcsstring(str);
	return is_regexp_match(regexp_dbcsstr.begin(), regexp_dbcsstr.end()
							, str_dbcsstr.begin(), str_dbcsstr.end());	
}
void convert_yen_to_slash(char *pathname)
{
	while(*pathname){
		if(*pathname == '\\'){
			*pathname = '/';
		}
		pathname += _mbclen((unsigned char*)pathname);
	}
}
static void find_files(const char *regexp, list<string> &files)
{
	//WIN32_FIND_DATA finddata;
	//HANDLE hFindFile;
	long handle;
	struct _finddata_t finddata;
	string dirname = get_dirname(regexp);

	handle = _findfirst(regexp, &finddata);
	int ret;
	// hFindFile = ::FindFirstFile(regexp, &finddata);
	ret = (handle != -1);
	while(ret){
		string fname = finddata.name;
		if(fname == "." || fname == ".."){
			;
		}else{
			string pathname = make_pathname(dirname.c_str(), fname.c_str());
			files.push_back(pathname);
			if((finddata.attrib & _A_SUBDIR) == _A_SUBDIR){
				string searchname = make_pathname(pathname.c_str(), "*.*");
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

list<string> find_files(const char *regexp)
{
	list<string> files;
	find_files(regexp, files);
	return files;
}
