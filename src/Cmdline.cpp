/*
	cmdline.c
		by Yoshioka Tsuneo(QWF00133@niftyserve.or.jp)
		welcome any e-mail!!
		You can use this file as Public Domain Software.
		Copy,Edit,Re-distibute and for any purpose,you can use this file.
*/
#include "stdafx.h"
#include "cmdline.h"

/* 多分記号は全部0x40未満だからうまくいくだろう・・・ */
#ifdef WIN32
#define KANJI
#endif


#ifdef KANJI
#define UCH(c)  ((unsigned char)(c))
#define iskanji1(c) ((0x81<=UCH(c)&&UCH(c)<=0x9F)||(0xE0<=UCH(c)&&UCH(c)<=0xFC))
#define iskanji2(c) ((0x40<=UCH(c)&&UCH(c)<=0x7E)||(0x80<=UCH(c)&&UCH(c)<=0xFC))
#endif

/*	コマンドライン引数を分割する
	(レスポンスファイルは利用しない。)*/
void split_cmdline(const char *cmdline,std::vector<std::string> &files)
{
	const char *ptr=cmdline;
	int quote_mode=0;

	files.clear();
	while(isspace((unsigned char)*ptr)){ptr++;}
	while(*ptr){
		std::string file;
		while(((!isspace((unsigned char)*ptr)) || quote_mode) && *ptr!='\0'){
			if(*ptr=='"'){
				quote_mode = !quote_mode;
				ptr++;
			}else{
#ifdef KANJI
				/* if 2 byte charactor then copy on more byte */
				if(iskanji1(*ptr) && iskanji2(*(ptr+1))){
					file.push_back(*ptr);
					ptr++;
				}
#endif
				file.push_back(*ptr);
				ptr++;
			}
		}
		files.push_back(file);
		while(isspace((unsigned char)*ptr)){ptr++;}
	}
}

/*ファイルを読み込んで文字列に入れる*/
bool loadfile(const char *fname,std::string &str)
{
	FILE *fp;
	struct _stati64 st;
	std::vector<char> buffer;

	if(_stati64(fname,&st)!=0){return false;}
	size64 size=st.st_size;

	if((fp=fopen(fname,"rb"))==NULL){
		return false;
	}
	buffer.resize((size_t)(size+1));	//TODO:size lost
	size64 l=fread(&buffer[0],1,(size_t)size,fp);	//TODO:size lost
	fclose(fp);
	if(l!=size){
		return false;
	}
	buffer[(size_t)size]='\0';	//TODO:size lost

	str=&buffer[0];
	return true;
}

/* コマンドラインを展開 */
/*  @ではじまるレスポンスファイルがあればそれも展開する。*/
bool split_cmdline_with_response(const char *cmdline,std::vector<std::string> &files)
{
	std::vector<std::string> filesTmp;
	split_cmdline(cmdline,filesTmp);

	for(UINT i=0;i<filesTmp.size();i++){
		if(filesTmp[i][0]=='@'){
			const char *fname=filesTmp[i].c_str()+1;
			std::string filebody;
			if(!loadfile(fname,filebody))return false;

			std::vector<std::string> work;
			split_cmdline(filebody.c_str(),work);
			files.insert(files.end(),work.begin(),work.end());	//連結
		}else{
			files.push_back(filesTmp[i]);
		}
	}
	return true;
}
