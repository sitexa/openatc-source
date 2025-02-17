#pragma once

#include "OpenATCRunStatus.h"

typedef enum tagVehType
{
    VEH_TYPE_UNDEF = 0,
    VEH_TYPE_SMALL = 1,
    VEH_TYPE_MIDDLE = 2,
    VEH_TYPE_LARGE = 3,
}EVehType;

typedef struct tagOneDetFlowInfo
{
    long m_nLargeVehNum;                            //����
    long m_nMiddleVehNum;                           //�г���
    long m_nSmallVehNum;                            //С����
    long m_nTotalVehCounter;                        //�г�ʱ���ۼ�
	long m_nTotalVehCounterInGreen;                 //�̵�ʱ���г�ʱ���ۼ�
	unsigned long m_nTotalGreenCounter;             //�̵�ʱ���ۼ�
    unsigned char m_chOccupyRate;                   //ռ����
	unsigned char m_chGreenUsage;                   //�̵�ʱ��ռ����
}TOneDetFlowInfo,*PTOneDetFlowInfo;

typedef struct tagStatisticVehDetData
{
    int m_nCounter;                                                                 //ͳ�Ƽ�ʱ
    int m_nDetNum;                                                                  //����������
    bool m_bSaveFlag;                                                               //�洢���
    TOneDetFlowInfo m_atDetFlowInfo[C_N_MAXDETBOARD_NUM * C_N_MAXDETINPUT_NUM];     //����ͳ����Ϣ����
}TStatisticVehDetData,*PTStatisticVehDetData;
