#include "arcz.h"
#include "util.h"
#include <stdlib.h>


const unsigned char	CTarArcFile_Compress::MAGIC_1 = '\037';	/* First byte of compressed file	*/
const unsigned char	CTarArcFile_Compress::MAGIC_2 = '\235';	/* Second byte of compressed file	*/

// static const int			C::HSIZE	= 1 << 17;	/* 131072 */
static const int			BITS	= 16;
static const int			INIT_BITS = 9;

static const int			BIT_MASK = 0x1f;
static const int			BLOCK_MODE	= 0x80; /* Block compresssion if table is full and compression rate is dropping flush tables*/

static const int			FIRST	= 257;
static const int			CLEAR	= 256;

CTarArcFile_Compress::CTarArcFile_Compress()
{
	m_pFile = NULL;
}
CTarArcFile_Compress::~CTarArcFile_Compress()
{
	if(m_pFile){fclose(m_pFile);}
}
bool CTarArcFile_Compress::open(const char *arcfile, const char *mode)
{
	m_arcfile = arcfile;

	/* only decompress implemented */
	if(strchr(mode,'w')){return false;}
	m_pFile = fopen(arcfile, "rb");

	m_insize = m_rsize = 0;

	m_insize = m_rsize = fread(m_inbuf, 1, BUFSIZ, m_pFile);

	char magic[2];
	memcpy(magic, m_inbuf, 2);
	//if(fread(magic, 1, 2, m_pFile) !=2){return false;}
	//m_insize += 2; m_rsize += 2;
	if(magic[0] != (char)MAGIC_1 || magic[1] != (char)MAGIC_2){return false;}

	char buf[1];
	memcpy(buf, m_inbuf+2, 1);
	//if(fread(buf, 1, 1, m_pFile) != 1){return false;}
	//m_insize += 1; m_rsize +=1;
	m_maxbits		= buf[0] & BIT_MASK;
	if(m_maxbits > BITS){return false;}
	m_block_mode	= ((buf[0] & BLOCK_MODE) != 0);

	m_eof = false;
	m_n_bits = INIT_BITS;
	m_maxcode = (1<<m_n_bits) - 1;
	m_oldcode = -1;
	m_finchar = 0;
	m_posbits = 3*8;
	m_free_ent = m_block_mode ? FIRST : 256;
	memset(m_codetab, 0, 256);
	for(int code=0; code<256; code++){
		m_htab[code]= code;
	}

	return true;
}
int CTarArcFile_Compress::read(void *buf, int size)
{
	int nread = 0;
	
	while(!m_eof && size > 0){
		int in_avail;
		in_avail = m_strstream.rdbuf()->in_avail();
		if(in_avail == 0){
			if(!readonce()){break;}
		}
		in_avail = m_strstream.rdbuf()->in_avail();
		int n = min(in_avail, size);
		m_strstream.read((char*)buf, n);
		nread += n;
		size -= n;
		buf = (void*)((char*)buf + n);
	}
	return nread;
}
bool CTarArcFile_Compress::readonce()
{
resetbuf:
	m_insize -= m_posbits/8;
	memmove(m_inbuf, m_inbuf + m_posbits/8, m_insize);
	m_posbits = 0;

	if(m_insize < sizeof(m_inbuf) - BUFSIZ){
		m_rsize = fread(m_inbuf + m_insize, 1, BUFSIZ, m_pFile);
		m_insize += m_rsize;
	}
	int inbits;
	if(m_rsize>0){
		inbits = (m_insize-m_insize%m_n_bits)*8;
	}else{
		inbits = (m_insize*8)-(m_n_bits-1);
	}
	while(inbits > m_posbits){
		if(m_free_ent > m_maxcode){
			m_posbits = (m_posbits-1)+((m_n_bits*8)-(m_posbits-1+(m_n_bits*8))%(m_n_bits*8));
			m_n_bits++;
			if(m_n_bits == m_maxbits){
				m_maxcode = 1 << m_maxbits;
			}else{
				m_maxcode = (1 << m_n_bits) - 1;
			}
			goto resetbuf;
		}
		// input()
		// input(inbuf, posbits, code, n_bits, bitmask);
		/*@3 = (*(long*)(&@1[@2>>3]) >> (@2&0x7)) & @5;
		@2 += @4;*/
		int code;
		{
			int bitmask = (1<<m_n_bits) - 1;
			code = (*(long*)(&m_inbuf[m_posbits/8]) >> (m_posbits&7)) & bitmask;
			m_posbits += m_n_bits;
		}

		if(m_oldcode == -1){
			// outbuf[outpos++] = (unsigned char)(code);
			m_strstream.put((unsigned char)code);
			m_oldcode = code;
			m_finchar = (int) code;
			continue;
		}
		if(code == CLEAR && m_block_mode){
			memset(m_codetab, 0, 256);
			m_free_ent = FIRST - 1;
			int tmp = (m_posbits-1) + (m_n_bits*8);
			m_posbits = tmp - tmp%(m_n_bits*8);
			m_n_bits = INIT_BITS;
			m_maxcode = (1 << m_n_bits) - 1;
			goto resetbuf;
		}
		int incode = code;
		unsigned char *stackp = m_htab + sizeof(m_htab);
		if(code > m_free_ent){
			return false;
		}else if(code == m_free_ent){	/* Special case for KwKwK string.	*/
			//m_posbits -= m_n_bits;
			// p = &inbuf[posbits/8];
			* --stackp = (unsigned char)m_finchar;
			code = m_oldcode;
		}
		while((unsigned int)code >= (unsigned)256){
			*--stackp	= m_htab[code];
			code		= m_codetab[code];
		}
		* --stackp = (unsigned char)(m_finchar = m_htab[code] );

		m_strstream.write((char*)stackp, m_htab + sizeof(m_htab) - stackp);
		/*
		if(outpos + (i=(de_stack - stackp)) > OBUFSIZ){
			do{
				if( i > OBUFSIZ - outpos){ i = OBUFSIZ - outpos; }
				memcpy(outbuf+outpos, stackp, i);
				 if(outpos >= OBUFSIZ){
					 m_stream.write(m_outbuf, outpos); outpos = 0;
				 }
				 stackp += i;
			}while((i=(de_stack - stackp))>0);
		}else{
			memcpy(outbuf + outpos, stackp, i);
			outpos += i;
		}
		*/
		if(m_free_ent < (1<<m_maxbits)){
			m_codetab[m_free_ent]	= (unsigned short)m_oldcode;
			m_htab[m_free_ent]		= m_finchar;
			m_free_ent ++;
		}
		m_oldcode = incode;
	}
	// bytes_in += rsize;
	if(m_rsize<=0){ m_eof = true; }
	// m_strstream.write(outbuf, outpos); outpos = 0;
	return true;
}
void CTarArcFile_Compress::close()
{
	if(m_pFile){fclose(m_pFile);m_pFile=NULL;}
}
string CTarArcFile_Compress::get_orig_filename(){
	if(! m_orig_filename.empty()){return m_orig_filename;}
	string fname = get_filename(m_arcfile.c_str());
	if(fname.length()>2 && stricmp(fname.substr(fname.length()-2).c_str(),".z") == 0){
		return fname.substr(0, fname.length()-2);
	}
	return fname + "_extracted";
}

