/*
   VC++6付属のSTLは遅いので単純な高速化バージョンを使う。
   SGIのSTLportも速いらしい... by tsuneo 2000/09/14
*/

#ifndef __FAST_OFSTREAM_H
#define __FAST_OFSTREAM_H

#include <stdio.h>

class fast_ofstream
{
	FILE *m_fp;
	bool m_berror;
public:
	fast_ofstream(){
		m_fp = NULL;
		m_berror = false;
	};
	~fast_ofstream(){
		close();
	};
	void open(const char *fname, int mode){
		m_fp = fopen(fname,"wb");
		m_berror = (m_fp == NULL);
	};
	bool fail(){
		return m_berror;
	};
	void write(const char *buf, int n){
		int m = fwrite(buf,1,n,m_fp);
		m_berror = (m!=n);
	};
	void close(){
		if(m_fp){
			fclose(m_fp);
			m_fp = NULL;
		}
	};
};

#endif // __FAST_OFSTREAM_H
