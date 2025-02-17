/*=====================================================================
模块名 ：单点优化控制方式实现模块
文件名 ：LogicCtlActuate.h
相关文件：
实现功能：用于定义单点优化控制模式实现接口
作者 ：刘黎明
版权 ：<Copyright(c) 2019-2020 Suzhou Keda Technology Co.,Ltd. All right reserved.>
-----------------------------------------------------------------------
修改记录：
日 期           版本     修改人     走读人     修改记录
2019/9/26       V1.0     刘黎明     刘黎明     创建模块
=====================================================================*/

#ifndef LOGICCTLOPTIM_H
#define LOGICCTLOPTIM_H

#include "LogicCtlFixedTime.h"

typedef struct tagOnePhaseOptimInfo
{
    int m_nDetectorCount;                                   //相位检测器数量
    BYTE m_byDetectorID[MAX_VEHICLEDETECTOR_COUNT];         //检测器编号
    BYTE m_byDetectorStatus[MAX_VEHICLEDETECTOR_COUNT];     //检测器状态,0为正常,1为故障
    int m_anValidPassTime[MAX_VEHICLEDETECTOR_COUNT];      //检测器的有效通行时间，毫秒为单位
    int m_nTotalPassTime;                                 //相位总通行时间,毫秒为单位
}TOnePhaseOptimInfo,*PTOnePhaseOptimInfo;

typedef struct tagOneRingOptimInfo
{
    int m_nPhaseCount;                                                      //相位数量
    TOnePhaseOptimInfo m_atPhaseOptimInfo[MAX_SEQUENCE_TABLE_COUNT];        //环内相位优化信息
}TOneRingOptimInfo,*PTOneRingOptimInfo;

typedef struct tagOptimInfo
{
    int m_nRingCount;                                                       //环数量
    TOneRingOptimInfo m_atRingOptimInfo[MAX_RING_COUNT];                    //环优化信息
}TOptimInfo,*PTOptimInfo;

class CLogicCtlOptim : public CLogicCtlFixedTime
{
public:
	CLogicCtlOptim();
	virtual ~CLogicCtlOptim();

    //初始化感应控制方式需要的参数
    virtual void Init(COpenATCParameter * pParameter, COpenATCRunStatus * pRunStatus, COpenATCLog * pOpenATCLog, int nPlanNo);

protected:
    //环内机动车相位的运行状态更新
    virtual void OnePhaseRun(int nRingIndex);

    //环内行人相位的运行状态更新
    virtual void OnePedPhaseRun(int nRingIndex);

    //周期结束重置控制状态
    virtual void ReSetCtlStatus();
private:
    //根据当前流量信息计算下一周期的绿信比
    void CalcPhaseTime(); 

    //初始化优化相关参数
    void InitOptimParam();

    //重置初始化相位计数
    void ResetPhaseOptimInfo(int nRingIndex,int nPhaseIndex);

    //处理相位优化参数
    void ProcPhaseOptimInfo(int nRingIndex,int nPhaseIndex);

    //计算相位饱和度
    bool CalcPhaseDSInfo(int nRingIndex,int nPhaseIndex,int & nDS);

    bool m_bCalcSplitTimeFlag;          //绿信比计算标志

    BYTE m_byDetectorTmpStatus[MAX_VEHICLEDETECTOR_COUNT];                      //临时存储检测器状态,用于计算有效通行时间
    int m_anDetectorTmpCounter[C_N_MAXDETBOARD_NUM * C_N_MAXDETINPUT_NUM];      //临时车检器发生时间戳计数,用于计算有效通行时间

    TOptimInfo m_tOptimInfo;            //优化信息
};

#endif // !ifndef LOGICCTLOPTIM_H





















