/*******************************************************************************
 * ģ����  : DSPCCI
 * �ļ���  : dspcci_api_host.h
 * ����ļ�:
 * �ļ�ʵ�ֹ���: host��cciͨ�Žӿں�����ݽṹ����
 * ����    : �ŷ���
 * �汾    : V1.0  Copyright(C) 2014-2020 KEDACOM, All rights reserved.
 * -----------------------------------------------------------------------------
 * �޸ļ�¼:
 * ��  ��      �汾        �޸���      �޸�����
 * 2014/03/01  1.1.1       �ŷ���      ����
*******************************************************************************/
#ifndef __DSPCCI_API_HOST_H
#define __DSPCCI_API_HOST_H

#ifdef __cplusplus
extern "C" {
#endif


#include <stdlib.h>
#include "../common/kdvtype.h"
#include "dspcci_common.h"


/* �ֽ���ת���궨�� */
#ifdef __linux__
    #define _LITTLE_ENDIAN              0
    #define _BIG_ENDIAN                 1

    #if __BYTE_ORDER == __LITTLE_ENDIAN
    #   define _BYTE_ORDER     _LITTLE_ENDIAN
    #else
    #   define _BYTE_ORDER     _BIG_ENDIAN
    #endif
#endif

#if _BYTE_ORDER == _BIG_ENDIAN

#define ltohl(x)    ((((x) & 0x000000ffUL) << 24) | \
                     (((x) & 0x0000ff00UL) <<  8) | \
                     (((x) & 0x00ff0000UL) >>  8) | \
                     (((x) & 0xff000000UL) >> 24))

#define htoll(x)    ((((x) & 0x000000ffUL) << 24) | \
                     (((x) & 0x0000ff00UL) <<  8) | \
                     (((x) & 0x00ff0000UL) >>  8) | \
                     (((x) & 0xff000000UL) >> 24))

#define ltohs(x)    ((((x) & 0x00ff) << 8) | \
                     (((x) & 0xff00) >> 8))

#define htols(x)    ((((x) & 0x00ff) << 8) | \
                     (((x) & 0xff00) >> 8))

/* ʹ�ú���ת��������Ĳ���Ϊһ�����ʽ(�� *ptr, a+b)ʱ�����4�μ���Ĳ���
   һ���潵�ʹ���ִ��Ч�ʣ���һ���浱ָ��32λ��д������ڴ�����ᱻ���ε���4�Σ�
   ��������Host4�ε����ڼ��޸����ڴ��ֵ��Host������ֵ���λ�ҵ���
   ��˸�Ϊinline����
#define LTOHL(x)    ltohl((u32)(x))
#define LTOHS(x)    ltohs((u16)(x))
#define HTOLL(x)    htoll((u32)(x))
#define HTOLS(x)    htols((u16)(x))
*/

static inline u32 LTOHL(u32 x)
{
    return ltohl(x);
}

static inline u32 LTOHS(u16 x)
{
    return ltohs(x);
}

static inline u32 HTOLL(u32 x)
{
    return htoll(x);
}

static inline u32 HTOLS(u16 x)
{
    return htols(x);
}

#else

#define LTOHL(x)    ((u32)(x))
#define LTOHS(x)    ((u16)(x))
#define HTOLL(x)    ((u32)(x))
#define HTOLS(x)    ((u16)(x))

#endif  /* _BYTE_ORDER==_LITTLE_ENDIAN */


/* DSP���Ͷ��� */
#define DSP_TYPE_MAX_NUM        5
#define DSP_TYPE_DM647_PCI      0x01
#define DSP_TYPE_DM647_HPI      0x02
#define DSP_TYPE_DM6437_PCI     0x03
#define DSP_TYPE_DM6437_HPI     0x04
#define DSP_TYPE_FPGA_FIFO      0x05    /* ����FPGA��FIFO��DSPͨ�ŵķ�ʽ */
#define DSP_TYPE_DM674X_HPI     0x06
#define DSP_TYPE_DM674X_UART    0x07
#define DSP_TYPE_C665x_UART     0x08
/* �Լ�ģ��궨�� */
#define DSP_SELFTEST_MEM        0x80000000  /* �����ڴ��Լ� */

typedef void ( *TDspPrtFunc )(const char *pFmtStr, ...);

/* dsp ���ò����ṹ���� */
typedef struct {
    u32 dwDspType;      /* ��DSP���Ͷ��� */
    u32 dwChipId;       /* ������DSP��оƬ������ֵ��Χ: 0~DSPCCI_DSP_MAXNUM-1 */
    s8  *pbyLoadFile;   /* DSP���еĳ����ļ�����DSP�������Ļ���� */
    u32 dwDspFreqMHz;   /* DSP����Ƶ��, 0��ʾʹ��Ĭ��ֵ */
    u32 dwDDRFreqMHz;   /* DSP�ڴ�Ƶ��, 0��ʾʹ��Ĭ��ֵ */
    u32 dwSelfTest;     /* �Ƿ�����Լ죬0=���Լ�;
                           �ɻ�����Ӧ�ĺ�����Ӧģ���Լ죬��: DSP_SELFTEST_MEM */
} TDspCfg;


/* �û������ͨ��Buffer�����ṹ���� */
typedef struct {
    u32 dwMaxMsgLen;    /* ÿ����Ϣ����󳤶�(��BYTE��)��Ҫ��8�������� */
    u32 dwMaxMsgs;      /* ��໺���������Ϣ */
} TCciBufCfgDesc;

typedef struct {
    u32 dwUpChnls;                  /* ������Ϣͨ���� */
    u32 dwDnChnls;                  /* ������Ϣͨ���� */
    TCciBufCfgDesc *patUpChnlCfgs;  /* �û������������Ϣͨ������������׵�ַ */
    TCciBufCfgDesc *patDnChnlCfgs;  /* �û������������Ϣͨ������������׵�ַ */
} TCciParam;


/*==============================================================================
 *  ������      : DspCciOpen
 *  ����        : ��ָ��DSP��CCIͨ�Žӿ�
 *  �������˵��: dwDspId:    DSPȫ��������ȡֵ��Χ: 0~DSPCCI_DSP_MAXNUM-1;
 *                ptDspCfg:   DSP����������Ϣ���ɰ�DSPȫ��������Ӧ������оƬ;
 *                ptCciParam: CCIͨ�Žӿڲ���
 *                pptObj:     ���ظ��û��Ŀ��ƾ��
 *  ����ֵ˵��  : ���󷵻�DSPCCI_FAILURE������룻�ɹ�����DSPCCI_SUCCESS
 *----------------------------------------------------------------------------*/
s32 DspCciOpen(u32 dwDspId, TDspCfg *ptDspCfg , TCciParam *ptCciParam,
               HDspCciObj *pptObj);

/*==============================================================================
 *  ������      : DspCciClose
 *  ����        : �ر�ָ��DSP��CCIͨ�Žӿ�
 *  �������˵��: ptObj:        [I]CCIͨ�Žӿڿ��ƾ��
 *  ����ֵ˵��  : ���󷵻�DSPCCI_FAILURE������룻�ɹ�����DSPCCI_SUCCESS
 *----------------------------------------------------------------------------*/
s32 DspCciClose(HDspCciObj ptObj);

/*==============================================================================
 *  ������      : DspCciWriteMsg
 *  ����        : ͨ��ָ��DSP��CCIͨ�Žӿڵ�����ͨ��дһ����Ϣ����DSP
 *  �������˵��: ptObj:        [I]CCIͨ�Žӿڿ��ƾ��
 *                dwChnl:       [I]����ͨ������;
 *                ptMsgDesc:    [I]��Ϣ����;
 *  ����ֵ˵��  : ���󷵻�DSPCCI_FAILURE������룻�ɹ�����DSPCCI_SUCCESS
 *----------------------------------------------------------------------------*/
s32 DspCciWriteMsg(HDspCciObj ptObj, u32 dwChnl, TDspCciMsgDesc *ptMsgDesc);

/*==============================================================================
 *  ������      : DspCciReadMsg
 *  ����        : ͨ��ָ��DSP��CCIͨ�Žӿڵ�����ͨ����ȡһ����Ϣ�����û���Buf��
 *                ���ܺ�DspCciRecvMsg���á�
 *  �������˵��: ptObj:    [I]CCIͨ�Žӿڿ��ƾ��
 *                dwChnl:   [I]����ͨ������;
 *                pbyBuf:   [I]�û������Buf��������Ŷ���������;
 *                pdwSize:  [IO]�û����䣬����ʱָ���û���Buf�Ĵ�С������㹻��
 *                              ��д�����ݲ�����д������ݳ���;���򷵻ش���
 *                nTimeout: [I]��ʱֵ: 0=��������, -1=��Զ�ȴ�, >0=�ȴ�������
 *  ����ֵ˵��  : ���󷵻�DSPCCI_FAILURE������룻�ɹ�����DSPCCI_SUCCESS
 *----------------------------------------------------------------------------*/
s32 DspCciReadMsg(HDspCciObj ptObj, u32 dwChnl,
                  u8 *pbyBuf, u32 *pdwSize, s32 nTimeout);

/*==============================================================================
 *  ������      : DspCciRecvMsg
 *  ����        : ͨ��ָ��DSP��CCIͨ�Žӿڵ�����ͨ���ӿڽ���һ����Ϣ��������֪��
 *                ����Ϣ�ĵ�ַ���û�����������DspCciRecvMsgDone�ͷŸ���Ϣ����
 *                �����㿽��ģʽ�����ܺͿ���ģʽ��DspCciReadMsg�ӿڻ��á�
 *  �������˵��: ptObj:     [I]CCIͨ�Žӿڿ��ƾ��
 *                dwChnl:    [I]����ͨ������;
 *                ptMsgDesc: [O]�û����䣬������Ž��յ����ݰ��ĵ�ַ���ȵ���Ϣ;
 *                nTimeout:  [I]��ʱֵ: 0=��������, -1=��Զ�ȴ�, >0=�ȴ�������
 *  ����ֵ˵��  : ���󷵻�DSPCCI_FAILURE������룻�ɹ�����DSPCCI_SUCCESS
 *----------------------------------------------------------------------------*/
s32 DspCciRecvMsg(HDspCciObj ptObj, u32 dwChnl,
                  TDspCciMsgDesc *ptMsgDesc, s32 nTimeout);

/*==============================================================================
 *  ������      : DspCciRecvMsgDone
 *  ����        : ��DspCciRecvMsg���ʹ�ã��û�������DspCciRecvMsg�õ������ݺ�
 *                ����ñ������ӽ��ն������ͷŸ���Ϣ����
 *  �������˵��: ptObj:        [I]CCIͨ�Žӿڿ��ƾ��
 *                dwChnl:       [I]����ͨ������;
 *                ptMsgDesc:    [I]��Ҫ��DspCciRecvMsg����ʱ����Ľṹ��������;
 *  ����ֵ˵��  : ���󷵻�DSPCCI_FAILURE������룻�ɹ�����DSPCCI_SUCCESS
 *----------------------------------------------------------------------------*/
s32 DspCciRecvMsgDone(HDspCciObj ptObj, u32 dwChnl, TDspCciMsgDesc *ptMsgDesc);

/*==============================================================================
 *  ������      : DspCciGetDspId
 *  ����        : ����CCIͨ�Žӿڿ��ƾ����ѯ��Ӧ��DSP�����š�
 *  �������˵��: ptObj:        [I]CCIͨ�Žӿڿ��ƾ��
 *  ����ֵ˵��  : ���󷵻�DSPCCI_FAILURE���ɹ����ض�Ӧ��DSP������
 *----------------------------------------------------------------------------*/
s32 DspCciGetDspId(HDspCciObj ptObj);

/*==============================================================================
 *  ������      : DspCciGetHeartBeat
 *  ����        : ��ѯָ��DSP��������������������ۼӣ�˵��DSPCCIͨ���Ѿ�������
 *  �������˵��: ptObj:        [I]CCIͨ�Žӿڿ��ƾ��
 *                pdwCount:     [I]�������������ָ��;
 *  ����ֵ˵��  : ���󷵻�DSPCCI_FAILURE������룻�ɹ�����DSPCCI_SUCCESS
 *----------------------------------------------------------------------------*/
s32 DspCciGetHeartBeat(HDspCciObj ptObj, u32 *pdwCount);

/*==============================================================================
 *  ������      : DspCciGetErrLog
 *  ����        : ��ѯָ��DSP�Ĵ�����־��
 *  �������˵��: ptObj:        [I]CCIͨ�Žӿڿ��ƾ��
 *                dwErrId:      [I]������־������0~DSPCCI_ERR_LOG_MAX_NUM-1;
 *                pdwErrLog:    [I]��Ŵ�����־��ָ��;
 *  ����ֵ˵��  : ���󷵻�DSPCCI_FAILURE������룻�ɹ�����DSPCCI_SUCCESS
 *----------------------------------------------------------------------------*/
s32 DspCciGetErrLog(HDspCciObj ptObj, u32 dwErrId, u32 *pdwErrLog);

/*==============================================================================
 *  ������      : DspCciClearRxChnl
 *  ����        : ���ָ��������ͨ�����ݡ�
 *  �������˵��: ptObj:        [I]CCIͨ�Žӿڿ��ƾ��
 *                dwChnl:       [I]����ͨ������;
 *  ����ֵ˵��  : ���󷵻�DSPCCI_FAILURE������룻�ɹ�����DSPCCI_SUCCESS
 *----------------------------------------------------------------------------*/
s32 DspCciClearUpChnl(HDspCciObj ptObj, u32 dwChnl);

/*==============================================================================
 *  ������      : DspCciGetVer
 *  ����        : ģ��汾�Ų�ѯ��
 *  �������˵��: pchVer: �����Ĵ�Ű汾��Ϣ��bufָ��
 *                dwBufLen:����buf�ĳ���
 *  ����ֵ˵��  : �汾��ʵ���ַ������ȡ�С��0Ϊ����;
 *                ���ʵ���ַ������ȴ���dwBufLen����ֵΪ0
 *----------------------------------------------------------------------------*/
s32 DspCciGetVer(char *pchVer, u32 dwBufLen);

/*==============================================================================
 *  ������      : DspCciRegPrtFunc
 *  ����        : ע���ӡ������֧�����û��Ĵ�ӡ������dsp��Ĵ�ӡ��Ϣ��ӡ������
 *                ����û���ע��Ļ�����ʹ��printf������ӡ�����ڻ�����
 *  �������˵��: ptDspPrtFunc: �û��Ĵ�ӡ����ָ��
 *  ����ֵ˵��  : ���󷵻�DSPCCI_FAILURE������룻�ɹ�����DSPCCI_SUCCESS
 *----------------------------------------------------------------------------*/
s32 DspCciRegPrtFunc(TDspPrtFunc ptDspPrtFunc);

/*==============================================================================
 *  ������      : DspCciGetRxMsgNum
 *  ����        : ��ѯ���ջ���Buf�л������Ϣ������
 *  �������˵��: ptObj:        [I]CCIͨ�Žӿڿ��ƾ��
 *                dwChnl:       [I]����ͨ������;
 *                pdwMsgNum:    [O]���ظ��û�����Ϣ����;
 *  ����ֵ˵��  : ���󷵻�DSPCCI_FAILURE������룻�ɹ�����DSPCCI_SUCCESS
 *----------------------------------------------------------------------------*/
s32 DspCciGetRxMsgNum(HDspCciObj ptObj, u32 dwChnl, u32 *pdwMsgNum);


/* ����Ϊ�����ú��������û�ע���OSP */

/*==============================================================================
 *  ������      : DspCciPrtEnable
 *  ����        : �����Ƿ�ſ�DSP�Ĵ�ӡ
 *  �������˵��: dwDspId:  [I]DSPȫ��������ȡֵ��Χ: 0~DSPCCI_DSP_MAXNUM-1;
 *                bEnPrt:   [I]0=��ֹ��ӡ��1=�����ӡ
 *  ����ֵ˵��  : ��
 *----------------------------------------------------------------------------*/
void DspCciPrtEnable(u32 dwDspId, BOOL32 bEnPrt);

/*==============================================================================
 *  ������      : DspCciInfoShow
 *  ����        : ��ӡCCIЭ����INFO���������ݺ�������ͨ��������Ϣ
 *  �������˵��: dwDspId:  [I]DSPȫ��������ȡֵ��Χ: 0~DSPCCI_DSP_MAXNUM-1;
 *  ����ֵ˵��  : ��
 *----------------------------------------------------------------------------*/
void DspCciInfoShow(u32 dwDspId);

/*==============================================================================
 *  ������      : DspCciModuleShow
 *  ����        : ��ӡHOST����ؿ��ƽṹ��Ա�����Լ��շ�ͳ�Ƽ���
 *  �������˵��: dwDspId:  [I]DSPȫ��������ȡֵ��Χ: 0~DSPCCI_DSP_MAXNUM-1;
 *  ����ֵ˵��  : ��
 *----------------------------------------------------------------------------*/
void DspCciModuleShow(u32 dwDspId);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __DSPCCI_API_HOST_H */
