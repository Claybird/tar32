#include "stdafx.h"
#include "rpm.h"
#include "util.h"

/* Reference: lib/rpmlib.h of rpm package. */
struct rpmlead{
	unsigned char magic[4];
	unsigned char major;	/* not supported  ver1, only support 2,3 and lator */
	unsigned char minor;
	short type;
	short archnum;
	char name[66];
	short osnum;
	short signature_type;
	char reserved[16];	/* pad to 96 bytes -- 8 byte aligned */
	int magic_check(){
		return magic[0] == 0xed && magic[1] == 0xab && magic[2] == 0xee && magic[3] == 0xdb;
	};
	short my_htons(short s){
		unsigned char *p = (unsigned char*)&s;
		return (p[0] << 8) + (p[1]);
	};
	void hton(){
		type = my_htons(type);
		archnum = my_htons(archnum);
		osnum = my_htons(osnum);
		signature_type = my_htons(signature_type);
	};
};

/* Reference: lib/header.c of rpm package. */
struct rpm_entryInfo{
	int tag;
	int type;
	int offset;	/* Offset from beginning of data segment, only defined on disk */
	int count;
};

/* case: signature_type == RPMSIG_HEADERSIG */
struct rpmsig_headersig{
	unsigned char magic[4];
	int reserved;
	int index_len;	/* count of index entries */
	int data_len;		/* number of bytes */
	int magic_check(){
		return magic[0] == 0x8e && magic[1] == 0xad && magic[2] == 0xe8 && magic[3]==0x01;
	};
	int get_lostheaderlen(){
		return index_len * sizeof(rpm_entryInfo) + data_len;  
	};
	long my_htonl(long s){
		unsigned char *p = (unsigned char*)&s;
		return (p[0] << 24) + (p[1]<<16) + (p[2]<<8) + p[3];
	};
	void hton(){
		index_len = my_htonl(index_len);
		data_len = my_htonl(data_len);
	};
};


/* Reference: lib/signature.h of rpm package */
#define RPMSIG_NONE         0  /* Do not change! */
/* The following types are no longer generated */
#define RPMSIG_PGP262_1024  1  /* No longer generated */ /* 256 byte */
/* These are the new-style signatures.  They are Header structures.    */
/* Inside them we can put any number of any type of signature we like. */

#define RPMSIG_HEADERSIG    5  /* New Header style signature */

size64 rpm_getheadersize(const char *arcfile)
{
	rpmlead lead;
	rpmsig_headersig sigheader,header;
	
	CAutoFile fs;
	fs.open(arcfile, "rb");
	if(!fs.is_opened()){return false;}
	
	size_t read = fread(&lead, 1, sizeof(lead), fs);
	if(read != sizeof(lead)){return -1;}
	if(! lead.magic_check()){return -1;}
	lead.hton();

	if(lead.signature_type == RPMSIG_NONE){
		;
	}else if(lead.signature_type == RPMSIG_PGP262_1024){
		if (0 != _fseeki64(fs, 256, SEEK_CUR)) { return false; }
	}else if(lead.signature_type == RPMSIG_HEADERSIG){
		read = fread(&sigheader,1, sizeof(sigheader), fs);
		if(read != sizeof(sigheader)){return -1;}
		if(!sigheader.magic_check()){return -1;}
		sigheader.hton();

		int siglen = sigheader.get_lostheaderlen();
		if (0 != _fseeki64(fs, siglen, SEEK_CUR)) { return -1; }

		size64 pos = _ftelli64(fs);
		if(pos == -1){return -1;}
		if((pos%8) != 0){
			/* 8 byte padding */
			// int pad = pos - (pos/8)*8;
			size64 pad = (pos/8 + 1) * 8 - pos ;  // fix by tsuneo 2001.05.14
			if (0 != _fseeki64(fs, pad, SEEK_CUR)) { return -1; }
		}
	}else{
		return -1;
	}

	read = fread(&header, 1, sizeof(header), fs);
	if(read != sizeof(header)){return -1;}
	if(!header.magic_check()){return -1;}
	header.hton();
	int hdrlen = header.get_lostheaderlen();
	if(hdrlen == -1){return -1;}
	if (0 != _fseeki64(fs, hdrlen, SEEK_CUR)) { return -1; }
	
	size64 size = _ftelli64(fs);
	return size;
}
