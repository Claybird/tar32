#ifndef __TAR_H
#define __TAR_H

/*
 * tar header block definition
 *
 * From IEEE Std 1003.1-1988, pp.156
 */


#define	TMAGIC		"ustar"
#define	TMAGLEN		6

#define	TOMAGIC		"ustar  "
#define	TOMAGLEN	8

#define	TVERSION	"00"
#define	TVERSLEN	2

#define	REGTYPE		'0'	/* Regular file */
#define	AREGTYPE	'\0'	/* Regular file; old V7 format */
#define	LNKTYPE		'1'	/* Link */
#define	SYMTYPE		'2'	/* Symbolic link */
#define	CHRTYPE		'3'	/* Character special */
#define	BLKTYPE		'4'	/* Block special */
#define	DIRTYPE		'5'	/* Directory */
#define	FIFOTYPE	'6'	/* FIFO special */
#define	CONTTYPE	'7'	/* Continguous file */
#define LONGLINK	'L' /* Long name Link by tantan*/


#define	MULTYPE		'M'	/* Added by GNUtar, not POSIX */
#define	VOLTYPE		'V'	/* Added by GNUtar, not POSIX */


#define	TSUID		04000	/* Set UID on execution */
#define	TSGID		02000	/* Set GID on execution */
#define	TSVTX		01000	/* Reserved */
				/* File permissions */
#define	TUREAD		00400	/* read by owner */
#define	TUWRITE		00200	/* write by owner */
#define	TUEXEC		00100	/* execute/search by owner */
#define	TGREAD		00040	/* read by group */
#define	TGWRITE		00020	/* write by group */
#define	TGEXEC		00010	/* execute/search by group */
#define	TOREAD		00004	/* read by other */
#define	TOWRITE		00002	/* write by other */
#define	TOEXEC		00001	/* execute/search by other */


#define	TBLOCK	512
#define	NAMSIZ	100

typedef	union	hblock	{
	char	dummy[TBLOCK];
	struct	{
		char	name[NAMSIZ];
		char	mode[8];
		char	uid[8];
		char	gid[8];
		char	size[12];
		char	mtime[12];
		char	chksum[8];
		char	typeflag;
		char	linkname[NAMSIZ];
		char	magic[6];
		char	version[2];
		char	uname[32];
		char	gname[32];
		char	devmajor[8];
		char	devminor[8];
	/* Following fields were added by GNUtar */
		char	atime[12];
		char	ctime[12];
		char	offset[12];
	} dbuf;
	unsigned int compsum(){
		unsigned int sum = 0;
		int i;
		for(i=0;i<TBLOCK;i++){
			sum += (unsigned char)dummy[i];
		}
		/* calc without checksum field */
		for(i=0;i<sizeof(dbuf.chksum);i++){
			sum -= dbuf.chksum[i];
			sum += ' ';
		}
		return sum;
	}
} HEADER;



#endif	/* __TAR_H */
