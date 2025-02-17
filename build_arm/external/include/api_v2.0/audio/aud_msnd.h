/*******************************************************************************
 * ģ����  : MINI_SOUND
 * �ļ���  : aud_msnd.h
 * ����ļ�: aud_msnd.c
 * �ļ�ʵ�ֹ���: �������������ӿ�
 * ����    : �ŷ���
 * �汾    : 1.0.0.0.0
 * -----------------------------------------------------------------------------
 * �޸ļ�¼:
 * ��  ��      �汾        �޸���      �޸�����
 * 2013/11/06  1.1.1       �ŷ���      ����
*******************************************************************************/
#ifndef __AUD_MSND_H
#define __AUD_MSND_H

#include "common/kdvtype.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/*==============================================================================
 * ģ��İ汾�������涨:
 * �ܵĽṹ: mn.mm.ii.cc.tttt
 *      ��  Osp 1.1.7.20040318 ��ʾ
 * ģ������Osp
 * ģ��1�汾
 * �ӿ�1�汾
 * ʵ��7�汾
 * 2004��3��18���޸�
 *----------------------------------------------------------------------------*/
#define VER_MSND_DRV         (const char*)"MSND_DRV 1.1.1.20131106"

/* ����ģ�鷵��ֵ���� */
#define MSND_EFLUSH      -15  /* The command was flushed (success). */
#define MSND_EPRIME      -14  /* The command was primed (success). */
#define MSND_EFIRSTFIELD -13  /* Only the first field was processed (success)*/
#define MSND_EBITERROR   -12  /* There was a non fatal bit error (success). */
#define MSND_ETIMEOUT    -11  /* The operation was timed out (success). */
#define MSND_EEOF        -10  /* The operation reached end of file */
#define MSND_EAGAIN      -9   /* The command needs to be rerun (success). */
#define MSND_ELEN        -8   /* len err (failure). */
#define MSND_ENOOPEN     -7   /* IO no open (failure). */
#define MSND_EBUSY       -6   /* An IO busy occured (failure). */
#define MSND_EINVAL      -5   /* Invalid input arguments (failure). */
#define MSND_ENOMEM      -4   /* No memory available (failure). */
#define MSND_EIO         -3   /* An IO error occured (failure). */
#define MSND_ENOTIMPL    -2   /* Functionality not implemented (failure). */
#define MSND_EFAIL       -1   /* General failure code (failure). */
#define MSND_EOK          0   /* General success code (success). */

/* ����ֵ���� */
#define MSND_DEV_MAX_NUM  8   /* Ŀǰ���֧��8����Ƶ�豸 */
#define MSND_DEV_ID_McASP 0   /* 0=asp0 1=asp1 2=asp2 for SOC or DSP */
#define MSND_DEV_ID_HDMI  3   /* hdmi audio */
#define MSND_DEV_ID_PCI   4   /* pci sound card */

/* ��Ƶ�豸��ģʽ����  */
#define MSND_IOM_INPUT    0   /* ����ģʽ�����ɼ� */
#define MSND_IOM_OUTPUT   1   /* ���ģʽ�������� */

/* MSndCtrl�����붨�� */
#define MSND_GET_RX_STAT  0   /* ��Ƶ����ͳ��״̬��ѯ */
#define MSND_GET_TX_STAT  1   /* ��Ƶ����ͳ��״̬��ѯ */

/* ���Ͷ��� */
typedef void * HMSndDev;

/*
 * ��Ƶ���ݿڶ��壬���ڶ�·��Ƶ�豸��Ч������ָ����ǰ�����󶨵�����
 * �������I2S��·��һ��serial�ڶ�Ӧһ��i2s������
 */
#define MSND_SER0    (1 << 0)
#define MSND_SER1    (1 << 1)
#define MSND_SER2    (1 << 2)
#define MSND_SER3    (1 << 3)
#define MSND_SER4    (1 << 4)
#define MSND_SER5    (1 << 5)
#define MSND_SER6    (1 << 6)
#define MSND_SER7    (1 << 7)
#define MSND_SER8    (1 << 8)
#define MSND_SER9    (1 << 9)
#define MSND_SER10   (1 << 10)
#define MSND_SER11   (1 << 11)
#define MSND_SER12   (1 << 12)
#define MSND_SER13   (1 << 13)
#define MSND_SER14   (1 << 14)
#define MSND_SER15   (1 << 15)
#define MSND_SER16   (1 << 16)
#define MSND_SER17   (1 << 17)
#define MSND_SER18   (1 << 18)

/*
 * ��Ƶ֡Buffer���ݸ�ʽ���Ͷ���:
 * SER : serial����д����Ӧ����������(��I2S�����߻�TDMʱ�ָ���������)
 * SLOT: ÿ·����������2��ͬ���ź�֮������ж����Ƶ�������ô��䣬��ʱ϶�ű�ʾ
 * ##������·���ݴ���ģ�����£�
 *  Fs: __|~~|__________________________|~~|___________________________|~~|_
 *        |<----- sample 0 ------------>|<----------sample 1 --------->|
 *
 * +------+-------+-------+-----+-------+-------+-------+---------------
 *  SER0: | SLOT0 | SLOT1 | ... | SLOTx | SLOT0 | ...
 * +------+-------+-------+-----+-------+-------+-------+---------------
 *  SER1: | SLOT0 | SLOT1 | ... | SLOTx | SLOT0 | ...
 * +------+-------+-------+-----+-------+-------+-------+---------------
 *   ...
 * +------+-------+-------+-----+-------+-------+-------+---------------
 *  SERn: | SLOT0 | SLOT1 | ... | SLOTx | SLOT0 | ...
 * +------+-------+-------+-----+-------+-------+-------+---------------
 *
 * ����ÿ��serial��ÿ��slot��Ӧһ����Ƶ������
 * ���磺Chnl[0]->SER0_SLOT0; Chnl[1]->SER1_SLOT0; Chnl[2]->SER0_SLOT1
 * ��Щ�ɼ��豸�ɼ�������������DMA�����Ʊ��뽻֯��ţ�
 * ��Щ���Խ�ÿ�������������������һ���ڴ�������
 */
enum
{
    /*
     * �����������ݽ�֯��ţ��磬������2��serial+2��ʱ϶Ϊ��˵�����з�ʽ
     * (ÿ����һ��ʱ������Ƶ���ݣ�������֮���ڴ�����):
     * SER0_SLOT0[0] SER1_SLOT0[0] SER0_SLOT1[0] SER1_SLOT1[0];
     * SER0_SLOT0[1] SER1_SLOT0[1] SER0_SLOT1[1] SER1_SLOT1[1];
     * ...
     * SER0_SLOT0[s] SER1_SLOT0[s] SER0_SLOT1[s] SER1_SLOT1[s]
     */
    MSND_DATA_FMT_CHNL_INTERLEAVED,

    /*
     * ÿ���������ݶ���������ţ�������2��serial+2��ʱ϶Ϊ��˵�����з�ʽ
     * (������֮���ڴ�����):
     * Single Chnl[0]: SER0_SLOT0[0] SER0_SLOT0[1] ... SER0_SLOT0[s];
     * Single Chnl[1]: SER1_SLOT0[0] SER1_SLOT0[1] ... SER1_SLOT0[s];
     * Single Chnl[2]: SER0_SLOT1[0] SER0_SLOT1[1] ... SER0_SLOT1[s];
     * Single Chnl[3]: SER1_SLOT1[0] SER1_SLOT1[1] ... SER1_SLOT1[s];
     */
    MSND_DATA_FMT_CHNL_NON_INTERLEAVED,

    /*
     * ÿ��Serial���������ݶ���������ţ�����I2S��˵���Ƕ�·�������ɼ�ʱ
     * ÿ·���������ݶ���������ţ�������2��serial+2��ʱ϶Ϊ��˵�����з�ʽ
     * (������֮���ڴ�����):
     * Stereo[0]: SER0_SLOT0[0] SER0_SLOT1[0] SER0_SLOT0[1] SER0_SLOT1[1]...;
     * Stereo[1]: SER1_SLOT0[0] SER1_SLOT1[0] SER1_SLOT0[1] SER1_SLOT1[1]...;
     */
    MSND_DATA_FMT_SER_NON_INTERLEAVED
};


typedef struct{
    u32    dwFBufId;    /* ֡BUF�������ţ������ڲ�ʹ�ã��û������޸� */
    u8    *pbyFBuf;     /* ֡BUF��ָ�룬ָ��֡����Buf��
                           �û������NULL�Ļ������Զ�����1������BUF������ʹ����
                           ��ָ���ĵ�ַ��Ϊ����BUF��
                           !!! ������û����䣬��һЩ��������:
                          1���û����뱣֤Buf�Ķ��룬����ʼ��ַ������128�ֽڶ���;
                            BUF��С=dwBytesPerSample*dwSamplePerFrame*dwChnlNum;
                          2���ر��豸ʱ���������ͷ���Щ�ڴ� */
    BOOL32 bUseCache;   /* �������û�����Buf��Ч�������Զ������Ϊ��cache�ģ�
                           ΪTRUE��ʾ�û������Buf��cache�����������ˢcache����
                           ΪFALSEΪ����cache�ģ���������ˢcache���� */
    u32    dwTimeStamp; /* ֡��ʱ������ɼ�ʱ�û����Զ�ȡ��ǰ֡��ʱ��� */
} TMSndFBufDesc;

/* ��ƵIO�豸�����Ĳ����ṹ����
 *  ��buffer�� AUD_SER0 �� AUD_SERn ·�����ҽ���ֲ�������Ϊbuffer��n��֮1,
    �޷����� AUD_SERn ��buffer�е�λ�á���ɼ�����SER1 SER2 �� SER3�����ڴ���
    ��������Ϊ:
        SER1_L SER2_L SER3_L SER1_R SER2_R SER3_R ... ���DM647��ͬ
 *
 *  dwChnlCfg���þ���:
 *                  �����                  ��Ӧ��
 *  H600:
 *      Asp0�ɼ�    ģ����Ƶ                MSND_SER0
 *                  ����MIC                 MSND_SER4 (�̶�48K 32λ����)
 *      Asp0����    3.5��Ƶ�ӿ�             MSND_SER1
 *                  ������                  MSND_SER5
 *  H700:
 *      Asp0�ɼ�    RCAģ����Ƶ             MSND_SER1
 *                  ��ũMIC                 MSND_SER3
 *                  ����MIC                 MSND_SER4 (�̶�48K 32λ����)
 *      Asp0����    RCAģ����Ƶ             MSND_SER0
 *                  6.5ƽ�����             MSND_SER2
 *                  ������                  MSND_SER5
 *      Asp1�ɼ�    HDMI��Ƶ                MSND_SER0 (������Դ��ʽ)
 *
 */
typedef struct{
    u32   dwBytesPerSample;   /* һ���������ֽ���: 1 2 4 */
    u32   dwSamplePerFrame;   /* һ֡���������� */
    u32   dwChnlNum;          /* ����������2������������n·��������1֡���ֽ���
                                 =dwBytesPerSample*dwSamplePerFrame*dwChnlNum */
    u32   dwChnlCfg;          /* ��0��ʾ��Ĭ������serial���������·, ����ֵ��
                                 MSND_SER0�ȵļ����壬�߼��û�ʹ�� */
    u32   dwFrameNum;         /* ����Frame�ĸ�������Χ: 2~MSND_BUF_MAX_NUM-1 */
    u32   dwSampleFreq;       /* 8000��48000��96000��192000Hz ... */
    u32   dwSampleBit;        /* ����λ��: 8 12 16 20 24 28 32,
                                 ��0���Զ�����ΪdwBytesPerSample*8
                                !!��dwSampleBit=32��ʾ��·����Ϊ32λ������������
                                ƵIO�豸��������dwBytesPerSample=2ֻȡ��16λ */
    u32   dwDataFmt;          /* ��Ƶ���ݸ�ʽ��MSND_DATA_FMT_CHNL_INTERLEAVED */
    TMSndFBufDesc *pBufDescs; /* ָ���û������FBufDesc�ṹ����������׵�ַ��
                                 �û������Լ���������Buf����ָ�봫�ݸ�����
                                 �������ΪdwFrameNum,
                                 ! ע��: ��Ƶ��Ҫ128�ֽڱ߽���룻
                                 ���ڲ����Լ�����Buf���û���ΪNULLʱ���ɣ�
                                 �����ᰴ��ǰ��Ĳ����Զ�����BUF */
} TMSndDevParam;

/* ��Ƶ�ɼ�ͳ��״̬�ṹ���壬��Ӧ������: MSND_GET_RX_STAT */
typedef struct{
    u32   dwFrameTotal;       /* ����ܻ������Ƶ����֡������ */
    u32   dwFrameSize;        /* һ֡��Ƶ���ݵ��ֽ���,��·�������ܺ� */
    u32   dwFramesCanRd;      /* �ܶ�ȡ����Ƶ����֡�ĸ��� */
    u32   dwBytesCanRd;       /* �ܶ�ȡ����Ƶ�����ֽ�������������ʱҪ�� */
    u32   dwLostBytes;        /* ���ڲɼ���ʾ�������ֽ�����û�п���bufʱ���� */
    u32   dwDmaErr;           /* dma����Ĵ��� */
    u32   dwOverRunErr;       /* Overrun����Ĵ��� */
    u32   dwSyncErr;          /* ֡ͬ������Ĵ��� */
    u32   dwPingPongErr;      /* ping-pong��ת����Ĵ��� */
    u32   adwReserved[5];     /* reserved */
} TMSndRxStat;

/* ��Ƶ����״̬�ṹ���壬��Ӧ������: MSND_GET_TX_STAT */
typedef struct{
    u32   dwFrameTotal;       /* ����ܻ������Ƶ����֡������ */
    u32   dwFrameSize;        /* һ֡��Ƶ���ݵ��ֽ��� */
    u32   dwFramesCanWrt;     /* ��д�����Ƶ����֡�ĸ��� */
    u32   dwBytesCanWrt;      /* ��������д�����Ƶ�����ֽ�����
                                 dwFrameTotal*dwFrameSize-dwBytesCanWrt=��ǰ����
                                 �ŵ���Ƶ�����ֽ��� */
    u32   dwMuteBytes;        /* ���ڲ��ű�ʾ���ž������ֽ�����һ����û����Ƶ��
                                 ��ʱ��������������ʱҪ�� */
    u32   dwDmaErr;           /* dma����Ĵ��� */
    u32   dwUnderRunErr;      /* Underrun����Ĵ��� */
    u32   dwSyncErr;          /* ֡ͬ������Ĵ��� */
    u32   dwPingPongErr;      /* ping-pong��ת����Ĵ��� */
    u32   adwReserved[5];     /* reserved */
} TMSndTxStat;


/*==============================================================================
    ������      : MSndOpen
    ����        : ��ƵIO�豸�򿪣�1��dwDevId���Դ�2�Σ��ֱ�ΪINPUT/OUPUT
    �������˵��: dwDevId: 0~MSND_DEV_MAX_NUM-1���磺MSND_DEV_ID_McASP;
                  nMode: MSND_IOM_INPUT/MSND_IOM_OUTPUT
                  ptParam: �򿪵Ĳ���
                  phDev: �豸���ƾ��ָ��
    ����ֵ˵��  : ���󷵻�MSND_EFAIL������룻�ɹ�����MSND_EOK�Ϳ��ƾ��
------------------------------------------------------------------------------*/
s32 MSndOpen(u32 dwDevId, s32 nMode, TMSndDevParam *ptParam, HMSndDev *phDev);

/*==============================================================================
    ������      : MSndClose
    ����        : ��ƵIO�豸�رա�
    �������˵��: hDev: MSndOpen�������صľ��;
    ����ֵ˵��  : ���󷵻�MSND_EFAIL������룻�ɹ�����MSND_EOK
------------------------------------------------------------------------------*/
s32 MSndClose(HMSndDev hDev);

/*==============================================================================
    ������      : MSndRead
    ����        : ����Ƶ�豸�����ݣ���ȡ���ȱ�����dwBytesPerSample*dwChnlNum��
                  ����������Ҫ�ƻ������������ԡ�
    �������˵��: hDev: ��MSND_IOM_INPUTģʽ����MSndOpen�������صľ��;
                  pBuf: ָ���û������Buf��������Ųɼ�����Ƶ����
                  size: Ҫ��ȡ�������ֽ���
                  nTimeoutMs: -1=wait forever; 0=no wait;������ֵΪ��ʱ������
    ����ֵ˵��  : ���󷵻�MSND_EFAIL����ʱ����0���ɹ����ض������ֽ���(=size)
------------------------------------------------------------------------------*/
s32 MSndRead(HMSndDev hDev, void *pBuf, size_t size, s32 nTimeoutMs);

/*==============================================================================
    ������      : MSndWrite
    ����        : ����Ƶ�豸д���ݣ����ݳ��ȱ�����dwBytesPerSample*dwChnlNum��
                  ����������Ҫ�ƻ������������ԡ�
    �������˵��: hDev: ��MSND_IOM_OUTPUTģʽ����MSndOpen�������صľ��;
                  pData: ָ���û���Ŵ����ŵ���Ƶ����
                  size: Ҫ���ŵ������ֽ���
                  nTimeoutMs: -1=wait forever; 0=no wait;������ֵΪ��ʱ������
    ����ֵ˵��  : ���󷵻�MSND_EFAIL����ʱ����0���ɹ�����д����ֽ���(=size)
------------------------------------------------------------------------------*/
s32 MSndWrite(HMSndDev hDev, void *pData, size_t size, s32 nTimeoutMs);

/*==============================================================================
    ������      : MSndCtrl
    ����        : ��ƵIO�豸���ƣ�Ŀǰ������
                    MSND_GET_RX_STAT: pArgsΪTMSndRxStat *
                    MSND_GET_TX_STAT: pArgsΪTMSndTxStat *
                  ......
    �������˵��: hDev: ����MSndOpen�������صľ��;
                  nCmd: �����룻pArgs: ����ָ��
    ����ֵ˵��  : ���󷵻�MSND_EFAIL������룻�ɹ�����MSND_EOK
------------------------------------------------------------------------------*/
s32 MSndCtrl(HMSndDev hDev, s32 nCmd, void *pArgs);

/*==============================================================================
    ������      : MSndGetVer
    ����        : ģ��汾�Ų�ѯ��
    �������˵��: pchVer:  �����Ĵ�Ű汾��Ϣ��bufָ��
                  dwBufLen: ����buf�ĳ���
    ����ֵ˵��  : �汾��ʵ���ַ������ȡ�С��0Ϊ����;
                  ���ʵ���ַ������ȴ���dwBufLen����ֵΪ0
------------------------------------------------------------------------------*/
s32 MSndGetVer(char *pchVer, u32 dwBufLen);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __AUD_MSND_H */
