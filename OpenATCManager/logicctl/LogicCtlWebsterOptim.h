/*=====================================================================
模块名 ：webster单点优化控制方式实现模块
文件名 LogicCtlWebsterOptim.h
相关文件：
实现功能：用于定义webster单点优化控制模式实现接口
作者 ：刘黎明
版权 ：<Copyright(c) 2019-2020 Suzhou Keda Technology Co.,Ltd. All right reserved.>
-----------------------------------------------------------------------
修改记录：
日 期           版本     修改人     走读人     修改记录
2019/9/26       V1.0     刘黎明     刘黎明     创建模块
=====================================================================*/

#ifndef LOGICCTLWEBSTEROPTIM_H
#define LOGICCTLWEBSTEROPTIM_H

#include "LogicCtlFixedTime.h"

typedef struct tagOnePhaseWebsterOptimInfo
{
    int m_nDetectorCount;                                   			             //相位检测器数量
    BYTE m_byDetectorID[MAX_VEHICLEDETECTOR_COUNT];         				         //检测器编号
    BYTE m_byDetectorStatus[MAX_VEHICLEDETECTOR_COUNT];     				         //检测器状态,0为正常,1为故障
	int m_anValidPassTime[MAX_VEHICLEDETECTOR_COUNT];      							 //检测器的有效通行时间，毫秒为单位
	int m_anValidPassVeh[MAX_VEHICLEDETECTOR_COUNT];      				             //检测器的通行车辆
    int m_nTotalPassTime;                                 				             //相位总通行时间,毫秒为单位
}TOnePhaseWebsterOptimInfo,*PTOnePhaseWebsterOptimInfo;

typedef struct tagOneRingWebsterOptimInfo
{
    int m_nPhaseCount;                                                      //相位数量
    TOnePhaseWebsterOptimInfo m_atPhaseOptimInfo[MAX_SEQUENCE_TABLE_COUNT]; //环内相位优化信息
}TOneRingWebsterOptimInfo,*PTOneRingWebsterOptimInfo;

typedef struct tagWebsterOptimInfo
{
    int m_nRingCount;                                                       //环数量
    TOneRingWebsterOptimInfo m_atRingOptimInfo[MAX_RING_COUNT];             //环优化信息
}TWebsterOptimInfo,*PTWebsterOptimInfo;

class CLogicCtlWebsterOptim : public CLogicCtlFixedTime
{
public:
	CLogicCtlWebsterOptim();
	virtual ~CLogicCtlWebsterOptim();

    //初始化webster单点优化控制方式需要的参数
    virtual void Init(COpenATCParameter * pParameter, COpenATCRunStatus * pRunStatus, COpenATCLog * pOpenATCLog, int nPlanNo);

protected:
    //环内机动车相位的运行状态更新
    virtual void OnePhaseRun(int nRingIndex);

    //环内行人相位的运行状态更新
    virtual void OnePedPhaseRun(int nRingIndex);

    //周期结束重置控制状态
    virtual void ReSetCtlStatus();
private:
	enum
	{
		WEBSTER_NO_VEH_PASS	= 0,
		WEBSTER_MIN_SPLIT_TIME	= 1,
		WEBSTER_MAX_SPLIT_TIME	= 2,
		WEBSTER_MULTI_VEH_PASS	= 3,
	};
    //根据当前流量信息计算下一周期的绿信比
    void CalcPhaseTime(); 

    //初始化优化相关参数
    void InitWebsterOptimParam();

    //重置初始化相位计数
    void ResetPhaseWebsterOptimInfo(int nRingIndex,int nPhaseIndex);

    //处理相位优化参数
    void ProcPhaseWebsterOptimInfo(int nRingIndex,int nPhaseIndex);

    //计算相位饱和度
    bool CalcWebsterBestCycleLength(int & nIndex, int & nBestCycleTimeLength, int & nCycleLostTime, float & fFlowRatio);

	//重新计算各相位绿信比
	void RecalcWebsterBestSplitTime(int nIndex, int nBestCycleLength, int nCycleLostTime, float & fFlowRatio);
	void RecalcWebsterBestSplitTimeByPlan();

	//设置相位绿信比
	int  SetPhaseSplitTime(int nRingIndex, int nPhaseIndex, WORD wSplitTime, bool bFlag);

	//判断当前相位是否有正常使用的检测器
	bool CheckPhaseIfHavaEffectiveDet(int nRingIndex,int nPhaseIndex);

    bool m_bCalcSplitTimeFlag;															//绿信比计算标志

    BYTE m_byDetectorTmpStatus[MAX_VEHICLEDETECTOR_COUNT];								//临时存储检测器状态,用于计算有效通行时间
    int m_anDetectorTmpCounter[C_N_MAXDETBOARD_NUM * C_N_MAXDETINPUT_NUM];				//临时车检器发生时间戳计数,用于计算有效通行时间

    TWebsterOptimInfo   m_tWebsterOptimInfo;											//优化信息
	int					m_nPhaseVehNumInfo[MAX_RING_COUNT][MAX_SEQUENCE_TABLE_COUNT];
	int					m_nPhaseValidVehNum[MAX_RING_COUNT][MAX_SEQUENCE_TABLE_COUNT];
	float				m_fPhaseValidVehPassTime[MAX_RING_COUNT][MAX_SEQUENCE_TABLE_COUNT];
	float				m_fPhaseValidMinVehPassTime[MAX_RING_COUNT][MAX_SEQUENCE_TABLE_COUNT]; //记录路口相位的最小车头时距
	float				m_fPhaseFlowRatio[MAX_RING_COUNT][MAX_SEQUENCE_TABLE_COUNT];
	WORD				m_wOriPlanPhaseSplitTime[MAX_RING_COUNT][MAX_SEQUENCE_TABLE_COUNT];
	WORD				m_wOriPlanPhaseSplitTimePlan[MAX_RING_COUNT][MAX_SEQUENCE_TABLE_COUNT];
	int					m_nMinBestCycleLength[MAX_RING_COUNT];
	int					m_nMaxBestCycleLength[MAX_RING_COUNT];
	float               m_fBalanceTotalFlowRatio;
	float               m_fPhaseBalanceFlowRatio[MAX_RING_COUNT][MAX_SEQUENCE_TABLE_COUNT];
	TAscSingleOptim m_tAscSingleOptimInfo;    //单点自适应控制参数

	//每个环对应的相位数
	int					m_nPhaseNum[MAX_RING_COUNT];
};

#endif // !ifndef LOGICCTLWEBSTEROPTIM_H





















