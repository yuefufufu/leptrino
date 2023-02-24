/********************************************************************************/
/* File Name			: pComResInternal.h										*/
/* Description			: �R�}���X����������`									*/
/********************************************************************************/
#ifndef _PCOMRESINTERNAL_H
#define _PCOMRESINTERNAL_H

#include "pCommon.h"

/********************************************************************************/
/*	�萔��` 																	*/
/********************************************************************************/
/* �R�}���h�R�[�h */
#define CMD_GET_INF		0x2A			/* ���i���m�F 					*/
#define CMD_GET_LIMIT		0x2B			/* ��i�l�m�F 						*/
#define CMD_DATA_START		0x32			/* �f�[�^���M�J�n 					*/
#define CMD_DATA_STOP		0x33			/* �f�[�^���M��~ 					*/


/* ���X�|���X���� */
#define RES_ERR_OK		0x00			/* ����I�� 						*/
#define RES_ERR_LEN		0x01			/* �d�����ُ� 						*/
#define RES_ERR_UNDEF		0x02			/* ����`�R�}���h 					*/
#define RES_ERR_VAL		0x03			/* �ݒ�l�ُ� 						*/
#define RES_ERR_STATUS		0x04			/* ��Ԉُ� 						*/


/********************************************************************************/
/*	�\���̒�` 																	*/
/********************************************************************************/
/*** �R�}���h ***/
/* �R�}���h�w�b�_�[ */
typedef struct tagCmdHead {
	UCHAR	ucLen;							/* �����O�X 						*/
	UCHAR	ucTermNo;						/* �[��No. 							*/
	UCHAR	ucCmd;							/* �R�}���h��� 					*/
	UCHAR	ucRsv;							/* �\�� 							*/
} ST_CMD_HEAD;

/*** ���X�|���X ***/
/* ���X�|���X�w�b�_�[ */
typedef struct tagResHead {
	UCHAR	ucLen;							/* �����O�X 						*/
	UCHAR	ucTermNo;						/* �[��No. 							*/
	UCHAR	ucCmd;							/* �R�}���h��� 					*/
	UCHAR	ucResult;						/* ���� 							*/
} ST_RES_HEAD;

/* ���i���m�F */
typedef struct tagRGetInf {
	ST_RES_HEAD	stHead;						/* �w�b�_ 							*/
	SCHAR		scPName[P_NAME_SIZE];				/* ���i�^�� 						*/
	SCHAR		scSerial[SERIAL_SIZE];				/* �V���A��No.						*/
	SCHAR		scFVer[F_VER_SIZE];				/* �t�@�[���o�[�W���� 				*/
	SCHAR		scFreq[FREQ_SIZE];				/* �o�̓��[�g						*/
} ST_R_GET_INF;

/* �f�[�^�擾 */
typedef struct tagRDataGetF {
	ST_RES_HEAD	stHead;						/* �w�b�_ 							*/
	SSHORT		ssForce[FN_Num];				/* �̓f�[�^ 						*/
	SSHORT		ssTemp;						/* ���x�f�[�^ 						*/
	UCHAR		ucStatus;					/* �X�e�[�^�X 						*/
	UCHAR		ucRsv;						/* �\�� 							*/
} ST_R_DATA_GET_F;

/* ��i�l�m�F */
typedef struct tagRLepGetLimit {
	ST_RES_HEAD	stHead;						/* �w�b�_ 							*/
	float		fLimit[FN_Num];					/* ��i 							*/
} ST_R_LEP_GET_LIMIT;

/********************************************************************************/
/*	�O�����J�֐���` 															*/
/********************************************************************************/



#endif
/************************* (C) COPYRIGHT 2010 Leptrino **************************/
