/*
	cmdline.h

		by Yoshioka Tsuneo(QWF00133@niftyserve.or.jp)
		welcome any e-mail!!
		You can use this file as Public Domain Software.
		Copy,Edit,Re-distibute and for any purpose,you can use this file.

*/

#ifndef ___CMDLINE_H
#define ___CMDLINE_H

#ifdef __cplusplus
extern "C"{
#endif

/* ptr の配列の長さを求める(NULLポインタが終端) */
//int ptrarraylen(void  **ptr);

/* 文字列の配列をまるごとコピー（複製）する */
/* 一列にまとめることで一回freeを呼ぶだけで配列まるごと開放できる*/
//char **strarraydup(char **ptrptr);

/*	コマンドライン引数を分割する
	(レスポンスファイルは利用しない。)*/
void split_cmdline(const char *cmdline,std::vector<std::string> &files);

/*ファイルを読み込んで文字列に入れる*/
bool loadfile(const char *fname,std::string &str);

/* reallocを呼んで返り値を*ptrに入れる (NULLの場合は入れない)*/
//void *realloc2(void **ptr,int size);

/* コマンドラインを展開 */
/*  @ではじまるレスポンスファイルがあればそれも展開する。*/
bool split_cmdline_with_response(const char *cmdline,std::vector<std::string> &files);

#ifdef __cplusplus
}
#endif
#endif /* CMDLINE_H */

