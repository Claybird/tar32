/*
	cmdline.c
		by Yoshioka Tsuneo(QWF00133@niftyserve.or.jp)
		welcome any e-mail!!
		You can use this file as Public Domain Software.
		Copy,Edit,Re-distibute and for any purpose,you can use this file.
*/
#include "cmdline.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

/* 多分記号は全部0x40未満だからうまくいくだろう・・・ */
#ifdef WIN32
#define KANJI
#endif


#ifdef KANJI
#define UCH(c)  ((unsigned char)(c))
#define iskanji1(c) ((0x81<=UCH(c)&&UCH(c)<=0x9F)||(0xE0<=UCH(c)&&UCH(c)<=0xFC))
#define iskanji2(c) ((0x40<=UCH(c)&&UCH(c)<=0x7E)||(0x80<=UCH(c)&&UCH(c)<=0xFC))
#endif
/* ptr の配列の長さを求める(NULLポインタが終端) */
int ptrarraylen(void  **ptr)
{
	int len=0;
	while(*ptr++){
		len++;
	}
	return len;
}
/* 文字列の(ポインタの)配列をまるごとコピー（複製）する */
/* 一列にまとめることで一回freeを呼ぶだけで配列まるごと開放できる*/
/* strarray[0] strarray[1] strarray[2] NULL *strarray[0] *strarray[1] *strarray[2] */
char **strarraydup(char **strarray)
{
	char **files_dup=NULL;
	int filenum;
	int alllen;
	int i;
	char *ptr;

	filenum=ptrarraylen((void**)strarray);
	alllen=0;
	for(i=0;i<filenum;i++){
		alllen=alllen+strlen(strarray[i])+1;
	}
	if((files_dup=malloc(sizeof(char*)*(filenum+1)+alllen))==NULL)goto endlabel;
	ptr=(char *)files_dup+sizeof(char*)*(filenum+1);
	for(i=0;i<filenum;i++){
		strcpy(ptr,strarray[i]);
		files_dup[i]=ptr;
		ptr=ptr+strlen(ptr)+1;
	}
	files_dup[i]=NULL;
endlabel:
	/* free memory */
	/*	not free here!
	for(i=0;i<filenum;i++){
		free(files[i]);
	}
	free(files);
	*/
	return files_dup;
}
/*	コマンドライン引数を分割する
	(レスポンスファイルは利用しない。)*/
char ** split_cmdline(const char *cmdline)
{
	const char *ptr=cmdline;
	const char *oldpt=ptr;
	char **files=NULL;
	char **files2=NULL;
	char *file;
	int filelen=0;
	int filelen2=0;
	char *filep;
	int quote_mode=0;
	int filenum=0;
	int i;

	while(isspace((unsigned char)*ptr)){ptr++;}
	while(*ptr){
		filelen2=4096;
		filelen=0;
		if((file=malloc(filelen2))==NULL){
			goto endlabel;
		}
		filep=file;
		while(((!isspace((unsigned char)*ptr)) || quote_mode) && *ptr!='\0'){
			if(*ptr=='"'){
				quote_mode = !quote_mode;
				ptr++;
			}else{
				if(filelen>=filelen2-5){
					filelen2+=4096;
					if((realloc2((void**)&file,filelen2))==NULL) goto endlabel;
					filep=file+filelen;
				}
#ifdef KANJI
				/* if 2 byte charactor then copy on more byte */
				if(iskanji1(*ptr) && iskanji2(*(ptr+1))){
					*filep++=*ptr++;
					filelen+=1;
				}
#endif
				*filep++=*ptr++;
				filelen+=1;
			}
		}
		*filep='\0';
		/*if((p=malloc(sizeof(char)*(strlen(file)+1)))==NULL)
			goto errorlabel;
		strcpy(p,file);*/
		if(realloc2((void**)&files,sizeof(char *)*(filenum+1))==NULL){
			free(file);
			goto endlabel;
		}
		filenum++;
		files[filenum-1]=file;
		while(isspace((unsigned char)*ptr)){ptr++;}
	}
	if(realloc2((void**)&files,sizeof(char *)*(filenum+1))==NULL){
		free(file);
		goto endlabel;
	}
	files[filenum]=NULL;
	files2=strarraydup(files);

endlabel:
	if(files!=NULL){
		for(i=0;i<filenum;i++){
			free(files[i]);
		}
		free(files);
	}
	return files2;	
}

/*ファイルを読み込んで文字列に入れる*/
char *loadfile(char *fname)
{
	FILE *fp;
	char *filebody=NULL;
	int l;
	struct stat buf;
	int size;

	if(stat(fname,&buf)!=0){return NULL;}
	size=buf.st_size;
	
	if((fp=fopen(fname,"rb"))==NULL){
		return NULL;
	}
	if((filebody=malloc(size+1))==NULL){fclose(fp);return NULL;}
	l=fread(filebody,1,size,fp);
	fclose(fp);
	if(l!=size){
		free(filebody);
		return NULL;
	}
	filebody[size]='\0';
	return filebody;
}

/* reallocを呼んで返り値を*ptrに入れる (NULLの場合は入れない)*/
void *realloc2(void **ptr,int size)
{
	void *ret;
	ret=realloc(*ptr,size);
	if(ret!=NULL){
		*ptr=ret;
	}
	return ret;
}

/* コマンドラインを展開 */
/*  @ではじまるレスポンスファイルがあればそれも展開する。*/
char **split_cmdline_with_response(const char *cmdline)
{
	char **files=NULL;
	char **files2;
	char **files3=NULL;
	char ***files4=NULL;
	char **files5=NULL;
	int files4count=0;
	int files3count=0;

	if((files=split_cmdline(cmdline))==NULL){
		return NULL;
	}
	files2=files;


	while(*files2!=NULL){
		if(**files2=='@'){
			char *filebody;
			char **ptrptr;
			if((filebody=loadfile(*files2+1))==NULL)goto endlabel;
			if(realloc2((void**)&files4,(files4count+1)*sizeof(char *))==NULL)goto endlabel;
			files4count++;
			if((ptrptr=split_cmdline(filebody))==NULL)goto endlabel;
			files4[files4count-1]=ptrptr;
			free(filebody);
			while(*ptrptr!=NULL){
				if(realloc2((void**)&files3,(files3count+1)*sizeof(char *))==NULL)goto endlabel;
				files3[files3count]=*ptrptr++;
				files3count++;
			}
			files2++;
		}else{
			if(realloc2((void**)&files3,(files3count+1)*sizeof(char *))==NULL)goto endlabel;
			files3[files3count]=*files2++;
			files3count++;
		}
	}
	if(realloc2((void**)&files3,(files3count+1)*sizeof(char *))==NULL)goto endlabel;
	files3[files3count]=NULL;
	if((files5=strarraydup(files3))==NULL) goto endlabel;
endlabel:
	if(files!=NULL)free(files);
	if(files3!=NULL)free(files3);
	if(files4!=NULL){
		int i;
		for(i=0;i<files4count;i++) free(files4[i]);
		free(files4);
	}
	return files5;
}
