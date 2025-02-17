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
    long m_nLargeVehNum;                            //大车数
    long m_nMiddleVehNum;                           //中车数
    long m_nSmallVehNum;                            //小车数
    long m_nTotalVehCounter;                        //有车时间累计
	long m_nTotalVehCounterInGreen;                 //绿灯时，有车时间累计
	unsigned long m_nTotalGreenCounter;             //绿灯时间累计
    unsigned char m_chOccupyRate;                   //占有率
	unsigned char m_chGreenUsage;                   //绿灯时间占有率
}TOneDetFlowInfo,*PTOneDetFlowInfo;

typedef struct tagStatisticVehDetData
{
    int m_nCounter;                                                                 //统计计时
    int m_nDetNum;                                                                  //车检器数量
    bool m_bSaveFlag;                                                               //存储标记
    TOneDetFlowInfo m_atDetFlowInfo[C_N_MAXDETBOARD_NUM * C_N_MAXDETINPUT_NUM];     //流量统计信息数组
}TStatisticVehDetData,*PTStatisticVehDetData;
