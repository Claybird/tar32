/*
   VC++6付属のSTLは余りに遅いので簡単な高速化バージョンを作る。
   # SGIのSTLportも速いらしい...
   展開速度が約3倍になった。
   by tsuneo. 2000/09/14


   VC2005標準のSTLでは32bitでコンパイルすると大容量(2^32)ファイルサイズを扱えないので、
   大容量ファイルを扱えるよう拡張(int->size64)

   とはいえ、一度の読み込み/書き込みは通常の環境であれば、バッファに使えるメモリの関係上
   32bitで扱えるサイズ以下であると仮定しているなので、size64をintにキャストしてしまっている

   by claybird. 2009/06/13

   入出力が遅いので、setvbuf()によりバッファを拡大する
   by claybird. 2010/11/3
*/

#ifndef __FAST_OFSTREAM_H
#define __FAST_OFSTREAM_H

#include <stdio.h>

class fast_fstream
{
	FILE *m_fp;
	bool m_berror;
	bool m_bwrite;
	size_t m_count;
public:
	fast_fstream(){
		m_fp = NULL;
		m_berror = false;
		m_bwrite = false;
		m_count = 0;
	};
	~fast_fstream(){
		close();
	};
	void open(const char *fname, int mode){
		if(mode & std::ios::out){m_bwrite=true;}
		m_fp = fopen(fname, (m_bwrite ? "wb" : "rb"));
		if(m_fp)setvbuf(m_fp,NULL,_IOFBF,1024*1024);
		m_berror = (m_fp == NULL);
	};
	bool fail(){
		return m_berror;
	};
	void write(const char *buf, size64 n){
		size_t m = fwrite(buf,1,(size_t)n,m_fp);	//TODO:size lost
		m_count = m;
		m_berror = (m!=n);
	};
	void read(char *buf, size64 n){
		size_t m = fread(buf,1,(size_t)n,m_fp);	//TODO:size lost
		m_count = m;
		m_berror = (m!=n);
	}
	void close(){
		if(m_fp){
			fclose(m_fp);
			m_fp = NULL;
		}
	};
	size_t gcount(){return m_count;}

	fast_fstream& seekg(size64 _Off,std::ios_base::seek_dir _Way){
		int _Origin=0;
		if(_Way & std::ios_base::beg)_Origin=SEEK_SET;
		if(_Way & std::ios_base::cur)_Origin=SEEK_CUR;
		if(_Way & std::ios_base::end)_Origin=SEEK_END;
		int ret=_fseeki64(m_fp,_Off,_Origin);
		m_berror = (0!=ret);
		return *this;
	}

	size64 tellg(){
		return _ftelli64(m_fp);
	}
	int get(){
		return fgetc(m_fp);
	}
};
/*
class fast_strstreambuf
{
	int buf_size;	//size64精度が必要になるような巨大なバッファは想定していない
	char *buf;
	size64 start_pos;
	size64 end_pos;
public:
	fast_strstreambuf(){
		buf = new char[2];
		buf_size = 2;
		start_pos = end_pos = 0;
	};
	~fast_strstreambuf(){
		delete [] buf;
	};
	size64 in_avail(){
		return ((end_pos - start_pos) & (buf_size - 1));
	};
	size64 write(char *writebuf, size64 n){
		size64 n2 = n;
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
	size64 read(char *readbuf, size64 n){
		size64 n2 = n;
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
	size64 write(char *writebuf, size64 n){
		return streambuf.write(writebuf,n);
	};
	size64 read(char *writebuf, size64 n){
		return streambuf.read(writebuf,n);
	};
	void put(char c){
		write(&c, 1);
	};
	fast_strstreambuf *rdbuf(){
		return &streambuf;
	}
};
*/
#endif // __FAST_OFSTREAM_H
