/*
	cpio.h
*/

#ifndef ___CPIO_H
#define ___CPIO_H
#include <memory.h>

/*	Reference: GNU cpio cpiohdr.h */
/* "New" portable format and CRC format:

   Each file has a 110 byte header,
   a variable length, NUL terminated filename,
   and variable length file data.
   A header for a filename "TRAILER!!!" indicates the end of the archive.  */

/* All the fields in the header are ISO 646 (approximately ASCII) strings
   of hexadecimal numbers, left padded, not NUL terminated. */
struct new_cpio_header
{
  char magic[6];	/* "070701" for "new" portable format, "070702" for CRC format */
  char inode[8];
  char mode[8];
  char uid[8];
  char gid[8];
  char nlink[8];
  char mtime[8];
  char size[8];	/* must be 0 for FIFOs and directories */
  char devmajor[8];
  char devminor[8];
  char rdevmajor[8];	/*only valid for chr and blk special files*/
  char rdevminor[8];  /*only valid for chr and blk special files*/
  char namesize[8]; /*count includes terminating NUL in pathname*/
  char chksum[8];  /* 0 for "new" portable format; for CRC format the sum of all the bytes in the file  */
  int magic_check(){
	  return memcmp(magic,"070701",6) == 0 || memcmp(magic,"070702",6) == 0;
  };
};

#endif /* CPIO_H */
