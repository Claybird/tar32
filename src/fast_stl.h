/*
   VC++6付属のSTLは余りに遅いので簡単な高速化バージョンを作る。
   # SGIのSTLportも速いらしい...
   展開速度が約3倍になった。
   by tsuneo. 2000/09/14
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

class fast_strstreambuf
{
	int buf_size;
	char *buf;
	int start_pos;
	int end_pos;
public:
	fast_strstreambuf(){
		buf = new char[2];
		buf_size = 2;
		start_pos = end_pos = 0;
	};
	~fast_strstreambuf(){
		delete [] buf;
	};
	int in_avail(){
		return ((end_pos - start_pos) & (buf_size - 1));
	};
	int write(char *writebuf, int n){
		int n2 = n;
		while(buf_size - in_avail() <= n){
			char *p = new char[buf_size*2];
			char *newbuf = p;
			while(start_pos != end_pos){
				*p++ = buf[start_pos++];
				start_pos &= (buf_size-1);
			}
			start_pos = 0;
			end_pos = p - newbuf;
			delete [] buf;
			buf = newbuf;
			buf_size *= 2;
		}
		while(n2>0){
			buf[end_pos ++] = *writebuf++;
			end_pos &= (buf_size-1);
			n2--;
		}
		return n-n2;
	};
	int read(char *readbuf, int n){
		int n2 = n;
		while(n2>0){
			*readbuf++ = buf[start_pos++];
			start_pos &= (buf_size-1);
			n2--;
		}
		return n-n2;
	};
};
class fast_strstream
{
	fast_strstreambuf streambuf;
public:
	int write(char *writebuf, int n){
		return streambuf.write(writebuf,n);
	};
	int read(char *writebuf, int n){
		return streambuf.read(writebuf,n);
	};
	void put(char c){
		write(&c, 1);
	};
	fast_strstreambuf *rdbuf(){
		return &streambuf;
	}
};

#endif // __FAST_OFSTREAM_H
