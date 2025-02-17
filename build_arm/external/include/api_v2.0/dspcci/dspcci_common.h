/*******************************************************************************
 * ģ����  : DSPCCI
 * �ļ���  : dspcci_common.h
 * ����ļ�: .c
 * �ļ�ʵ�ֹ���: host��dsp˫��cciͨ��Э����صĺ�����ݽṹ���壬������ͬʱ������
 *           ע�⣺������ͷ�ļ�ǰ���������Ӧ����������ͷ�ļ���host����kdvtype.h
 *                 dsp�����dsp_typedefs.h�����ɻ�����
 * ����    : �ŷ���
 * �汾    : V1.0  Copyright(C) 2014-2020 KEDACOM, All rights reserved.
 * -----------------------------------------------------------------------------
 * �޸ļ�¼:
 * ��  ��      �汾        �޸���      �޸�����
 * 2014/03/01  1.1.1       �ŷ���      ����
*******************************************************************************/
#ifndef __DSPCCI_COMMON_H
#define __DSPCCI_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif


/* �汾��Ϣ */
#define DSPCCI_MOD_VERSION          (const char *)"DspCciLib 1.3.19.20100701"

/* ���޶��� */
#define DSPCCI_DSP_MAXNUM               5   /* ���֧�ֵ�DSP�ĸ��� */

/* ��Ϣ��ӡ�궨�� */
#define DSPCCI_MAX_PRT_MSGS             100 /* ��ӡ��Ϣ�������� */
#define DSPCCI_MAX_PRT_MSGLEN           320 /* ��ӡ��Ϣ����󳤶� */

/* DSPCCIͨ����Ϣ���Եĺ궨�� */
#define DSPCCI_DN_MSGQ_MAX              256 /* ���������Ϣ������ */
#define DSPCCI_UP_MSGQ_MAX              256 /* ���������Ϣ������ */
#define DSPCCI_ERR_LOG_MAX_NUM          32  /* DSP�����¼�������� */
#define DSPCCI_MSG_ALIGN_BYTES          32  /* n�ֽڶ��룬Ҫ����TDspCciMsgHead
                                               �ṹ��С�������� */
#define DSPCCI_PROTOCOL_VERSION         1   /* DSPCCI��Э��汾�ţ�Э�鷢���ı�
                                               ʱ��1��HOST��DSP���뱣��һ�� */

/* �����궨�� */
#define DSPCCI_MAGIC_NUMBER             0xbeafbeaf
#define DSPCCI_START_MAGIC_NUMBER       0xdaaddeed

/* ��Ϣ���ͺ궨�� */
#define DSPCCI_IS_USR_MSG               0   /* �û���Ϣ */
#define DSPCCI_IS_HOST_LOOPBACK_MSG     1   /* �����Ի�������Ϣ */
#define DSPCCI_IS_HOST_SEND_TEST_MSG    2   /* �������Ͳ�����Ϣ*/
#define DSPCCI_IS_DSP_SEND_TEST_MSG     3   /* DSP���Ͳ�����Ϣ */
#define DSPCCI_IS_DSP_PRINT_MSG         4   /* DSP��ӡ��Ϣ */

/* ����ֵ�궨�� */
#define DSPCCI_SUCCESS                  0   /* CCIͨ�����ӽ����ɹ� */
#define DSPCCI_FAILURE                 -1   /* CCI����ʧ�� */
#define DSPCCI_NOT_CONNECTED           -2   /* CCIͨ��û�н��� */
#define DSPCCI_SMEM_CORRUPT            -3   /* CCI�����ڴ������ƻ� */
#define DSPCCI_LENGTH_ERROR            -4   /* CCIͨ����Ϣ���ȴ��� */
#define DSPCCI_QUEUE_FULL              -5   /* CCI��Ϣ�������� */
#define DSPCCI_MSG_LOST                -6   /* CCI��Ϣ��ʧ */
#define DSPCCI_PARAM_ERR               -7   /* �������� */
#define DSPCCI_NOT_SUPPORT             -8   /* ��֧�ֵĲ��� */
#define DSPCCI_MULTI_OPEN              -9   /* ��δ��豸 */
#define DSPCCI_NOT_OPEN                -10  /* �豸û�д� */
#define DSPCCI_OPEN_FAIL               -11  /* �豸��ʧ�� */
#define DSPCCI_IOC_FAIL                -12  /* �豸ioctlʧ�� */
#define DSPCCI_NO_MEM                  -13  /* �ڴ治�� */
#define DSPCCI_TIMEOUT                 -14  /* ������ʱ */
#define DSPCCI_QUEUE_EMPTY             -15  /* CCI��Ϣ���п� */
#define DSPCCI_PEER_CLOSED             -16  /* CCIͨ��Զ���豸�Ͽ� */

/* CCIͨ����Ϣ���Եĺ궨�� */
#define DSPCCI_UPCHNL                   0   /* ����ͨ����� */
#define DSPCCI_DNCHNL                   1   /* ����ͨ����� */


/* -------------------------------------------------------------------------- *
 * ���½ṹ��������HOST����ֱ�ӷ���DSP���ڴ��ͨ�ŷ�ʽ(��PCI��PCIE��HPI)      *
 * ����������Ƚ������ͨ�ŷ�ʽ�Ը��Ե�˽��Э��Ϊ׼�������spi�ӿڵ�fifoͨ��  *
 *----------------------------------------------------------------------------*/

/* START�������RAM_BASE��ƫ��������ǰ��0x20�ռ䱣��,
   PCIͨ��ʱ����0x20��С����reset_vector�������������� */
#define DSPCCI_START_SHM_OFFSET         0x20

/* ���Ͷ��� */
typedef void * HDspCciObj;                  /* DSPͨ�Ŷ������� */


/* START���ṹ���� */
typedef struct {
    /* start���Ļ�����DSP�ȳ�ʼ��Ϊ0xdaaddeed,֮��������ʼ��Ϊ0xbeafbeaf */
    volatile u32 dwCciStartMarker;
    /* ����������־�� 1--���� */
    volatile u32 dwCciHostStartupFlag;
    /* Info����Ч��־��������ʼ��Ϊ0����DSP������INFO���ڴ������Ϊ1 */
    volatile u32 dwCciInfoAvailableFlag;
    /* info������ַ��������ʼ��Ϊ0 */
    volatile u32 dwCciInfoBaseAddr;
    /* DSP��CCI�汾��,��DSP����д��������У���Ƿ�ƥ�� */
    volatile u32 dwDspCciVer;
    /* ������CCI�汾��,����������д��DSP��У���Ƿ�ƥ�� */
    volatile u32 dwHostCciVer;
    /* ʹ��brdwrapperdef.h�е�������ID�ź궨�� */
    volatile u32 dwBrdID;
    /* Ӳ���汾�� */
    volatile u32 dwHwVer;
    /* EPLD/FPGA/CPLD�ĳ���汾�� */
    volatile u32 dwFpgaVer;
    /* ��ʶ��ǰ����һ��dsp����0��ʼ��� */
    volatile u32 dwDspId;
} TDspCciStartBuf;


/* ͨ��������Ϣ�ṹ���� */
typedef struct{
    /* �ڴ�ػ���ַ, DSPCCI_MSG_ALIGN_BYTES���� */
    volatile u32 dwBufBase;
    /* ��Ϣ����󳤶�(��BYTE��)��Ҫ��8�������� */
    volatile u32 dwMaxMsgLen;
    /* ��໺�����Ϣ���� */
    volatile u32 dwMaxMsgs;
    /* �ڴ�ش�С,Ϊ(dwMaxMsgLen*dwMaxMsgs)��DSPCCI_MSG_ALIGN_BYTES���� */
    volatile u32 dwBufSize;
    /* �ڴ�ض�ָ��,Ϊ����ڴ�ػ���ַ��ƫ���� */
    volatile u32 dwReadPtr;
    /* �ڴ��дָ��,Ϊ����ڴ�ػ���ַ��ƫ���� */
    volatile u32 dwWritePtr;
    /* �Ѿ�д������ݰ����� */
    volatile u32 dwWriteNum;
    /* �Ѿ����������ݰ����� */
    volatile u32 dwReadNum;
    /* ������һ��������Ϣ�ĵ�ַ */
    volatile u32 dwNextRcvMsgPtr;
    /* DSP����/�հ����� */
    volatile u32 dwDspRxTxMsgs;
    /* DSP�ɹ�����/���մ������ */
    volatile u32 dwDspRxErrOrTxOkMsgs;
    /* DSP����/����������KByte */
    volatile u32 dwDspRxTxKBytes;
    /* DSP����/����������Byte */
    volatile u32 dwDspRxTxBytes;
	/* DSPͨ��д���� */
	volatile u32 dwReadPtrBusy;
	/* DSPͨ��������e */
	volatile u32 dwWritePtrBusy;
} TDspCciChnlInfo;


/* INFO���ṹ���� */
typedef struct{
    volatile u32 dwMsgDbg[4];
    /* INFO���Ļ���,��ЧֵΪ0xbeafbeaf */
    volatile u32 dwCciInfoMarker;
    /* ����������־��1-�������ͨ����������3-�������ͨ���ڴ�أ�
                     7-֪ͨdspͨ�Ŵ���ok */
    volatile u32 dwHostRdyFlag;
    /* DSP������־��1--�ѷ���ͨ����������3--�ѷ���ͨ���ڴ�أ� */
    volatile u32 dwDspRdyFlag;
    /* DSP��������DSP���������ۼӣ��������� */
    volatile u32 dwDspHeartBeat;
    volatile u32 dwUpChnlNum;                /* ����ͨ������ */
    volatile u32 dwDnChnlNum;                /* ����ͨ������  */
    volatile u32 dwUpChnlInfoBase;           /* ����ͨ������������ַ */
    volatile u32 dwDnChnlInfoBase;           /* ����ͨ������������ַ  */

    volatile u32 dwDspPrtEn;                 /* �����ӡ��־  */
    volatile TDspCciChnlInfo tPrtChnlInfo;   /* ��ӡͨ��������Ϣ */
    /* DSP�����¼����DSP�������д */
    volatile u32 dwDspErrLog[DSPCCI_ERR_LOG_MAX_NUM];
} TDspCciInfoBuf;


/* ��Ϣͷ���ṹ���� */
typedef struct{
	volatile u32 dwMsgDbg[4];
    volatile u32 dwMsgMarker;   /* ��Ϣͷ����,��ЧֵΪ0xbeafbeaf */
    volatile u32 dwMsgType;     /* ��Ϣ���ͣ��ο�: ��Ϣ���ͺ궨�� */
    volatile u32 dwMsgLen;      /* ��Ϣ�峤�� */
    volatile u32 dwNextMsgAddr; /* ��һ����Ϣ��ƫ�Ƶ�ַ(���dwBufBase)��
                                   DSPCCI_MSG_ALIGN_NUMBER�ֽڶ��� */
} TDspCciMsgHead;

/* -------------------------------------------------------------------------- *
 * ���Ͻṹ��������HOST����ֱ�ӷ���DSP���ڴ��ͨ�ŷ�ʽ(��PCI��PCIE��HPI)  *
 * ����������Ƚ������ͨ�ŷ�ʽ�Ը��Ե�˽��Э��Ϊ׼�������spi�ӿڵ�fifoͨ��  *
 *----------------------------------------------------------------------------*/


/* ��Ϣ�����ṹ���� */
typedef struct{
    u32   dwMsgType;            /* ��Ϣ���ͣ���: ��Ϣ���ͺ궨�� */
    void *pbyMsg1;              /* �û���Ϣ1ָ�� */
    void *pbyMsg2;              /* �û���Ϣ2ָ�룬����ʱ�ö���Ч */
    u32   dwMsg1Len;            /* �û���Ϣ1���� */
    u32   dwMsg2Len;            /* �û���Ϣ2���ȣ�����ʱ�ö���Ч */
} TDspCciMsgDesc;


#ifdef __cplusplus
}
#endif

#endif /* __DSPCCI_COMMON_H */
