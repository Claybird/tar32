/*
	ar.h
	archives format (for a.out/b.out/COFF)
*/
#ifndef ___AR_H
#define ___AR_H

/* Reference: GNU ar,  include/aout/ar.h */
struct ar_hdr{
	char name[16];
	char date[12];
	char uid[6];
	char gid[6];
	char mode[8];
	char size[10];
	char fmag[2];		/* should contain "`\012" */
	int magic_check(){
		return memcmp(fmag, "`\012",2) == 0;
	};
};

struct ar_first_hdr{
	char magic[8];	/* shoud conjtain "!<arch>\012"(a.out) or "!<bout>\012"(b.out)*/
	ar_hdr hdr;
};
#endif /* ___AR_H */
