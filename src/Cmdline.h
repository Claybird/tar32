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

/* ptr �̔z��̒��������߂�(NULL�|�C���^���I�[) */
int ptrarraylen(void  **ptr);

/* ������̔z����܂邲�ƃR�s�[�i�����j���� */
/* ���ɂ܂Ƃ߂邱�Ƃň��free���ĂԂ����Ŕz��܂邲�ƊJ���ł���*/
char **strarraydup(char **ptrptr);

/*	�R�}���h���C�������𕪊�����
	(���X�|���X�t�@�C���͗��p���Ȃ��B)*/
char ** split_cmdline(const char *cmdline);

/*�t�@�C����ǂݍ���ŕ�����ɓ����*/
char *loadfile(char *fname);

/* realloc���Ă�ŕԂ�l��*ptr�ɓ���� (NULL�̏ꍇ�͓���Ȃ�)*/
void *realloc2(void **ptr,int size);

/* �R�}���h���C����W�J */
/*  @�ł͂��܂郌�X�|���X�t�@�C��������΂�����W�J����B*/
char **split_cmdline_with_response(const char *cmdline);

#ifdef __cplusplus
}
#endif
#endif /* CMDLINE_H */

