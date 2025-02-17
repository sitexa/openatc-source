/*=====================================================================
模块名 ：韦伯斯特自适应实现模块
文件名 ：LogicCtlActuateAdaptive.h
相关文件：
实现功能：用于定义韦伯斯特自适应模式实现接口
作者 ：刘黎明
版权 ：<Copyright(c) 2019-2020 Suzhou Keda Technology Co.,Ltd. All right reserved.>
-----------------------------------------------------------------------
修改记录：
日 期           版本     修改人     走读人     修改记录
2019/9/26       V1.0     刘黎明     刘黎明     创建模块
=====================================================================*/

#ifndef LOGICCTLACTUATEADAPTIVE_H
#define LOGICCTLACTUATEADAPTIVE_H

#include "LogicCtlFixedTime.h"
#include "LogicCtlWebsterOptim.h"

class CLogicCtlActuateAdaptive : public CLogicCtlFixedTime
{
public:
	CLogicCtlActuateAdaptive();
	virtual ~CLogicCtlActuateAdaptive();

	//初始化自适应控制方式需要的参数
	virtual void Init(COpenATCParameter * pParameter, COpenATCRunStatus * pRunStatus, COpenATCLog * pOpenATCLog, int nPlanNo);

protected:
	//环内机动车相位的运行状态更新
	virtual void OnePhaseRun(int nRingIndex);

	//环内行人相位的运行状态更新
	virtual void OnePedPhaseRun(int nRingIndex);

	//处理相位延长绿
	virtual void ProcExtendGreen(int nRingIndex, int nPhaseIndex, int nCurRunTime);

	//处理相位延长绿
	virtual void LastConcurrencyPhaseBeforeBarrierExtendGreen(int nRingIndex, int nPhaseIndex, int nCurRunTime);

	//周期结束重置控制状态
	virtual void ReSetCtlStatus();
private:
	enum
	{
		WEBSTER_NO_VEH_PASS = 0,
		WEBSTER_MIN_SPLIT_TIME = 1,
		WEBSTER_MAX_SPLIT_TIME = 2,
		WEBSTER_MULTI_VEH_PASS = 3,
	};
	//根据当前流量信息计算下一周期的绿信比
	void CalcPhaseTime();

	//初始化优化相关参数
	void InitWebsterOptimParam();

	//重置初始化相位计数
	void ResetPhaseWebsterOptimInfo(int nRingIndex, int nPhaseIndex);

	//检测器最小车头时距统计计算
	void ProcPhaseDetMinTimeDis(int nRingIndex, int nPhaseIndex);

	//计算相位饱和度
	bool CalcWebsterBestCycleLength(int & nIndex, int & nBestCycleTimeLength, int & nCycleLostTime, float & fFlowRatio);

	//重新计算各相位绿信比
	void RecalcWebsterBestSplitTime(int nIndex, int nBestCycleLength, int nCycleLostTime, float & fFlowRatio);
	void RecalcWebsterBestSplitTimeByPlan();

	//设置相位绿信比
	int  SetPhaseSplitTime(int nRingIndex, int nPhaseIndex, WORD wSplitTime, bool bFlag);

	//判断当前相位是否有正常使用的检测器
	bool CheckPhaseIfHavaEffectiveDet(int nRingIndex, int nPhaseIndex);

	bool m_bCalcSplitTimeFlag;																	    //绿信比计算标志

	bool m_bIsFirstCycle;																			//判断是否进入自适应控制的第一个周期

	BYTE m_byDetectorTmpStatus[MAX_VEHICLEDETECTOR_COUNT];										    //临时存储检测器状态,用于计算有效通行时间
	int m_anDetectorTmpCounter[C_N_MAXDETBOARD_NUM * C_N_MAXDETINPUT_NUM];						    //临时车检器发生时间戳计数,用于计算有效通行时间

    //用于优化计算的车检器的信息，包含状态，过车时间，车辆数等等
	TWebsterOptimInfo   m_tWebsterOptimInfo;													    //优化信息

    //各个环内各个相位的经过的车辆信息，目前就取了4个值，0，1，2，3
	int					m_nPhaseVehNumInfo[MAX_RING_COUNT][MAX_PHASE_COUNT];               
	
    //统计记录各个环内各个相位的最大的车流量信息 
    int					m_nPhaseValidVehNum[MAX_RING_COUNT][MAX_PHASE_COUNT];              
		
    //记录各个环内各个相位的最小车头时距辆信息，有个初始值，后面会动态计算
    float				m_fPhaseValidMinVehPassTime[MAX_RING_COUNT][MAX_PHASE_COUNT];      

    //计算各个环内各个相位的流量比，用相位的最大流量除以饱和流量	
    float				m_fPhaseFlowRatio[MAX_RING_COUNT][MAX_PHASE_COUNT];                
	   
    //计算各个环内各个相位的平衡流量比，通过m_fPhaseFlowRatio和自适应配置参数计算得到
    float               m_fPhaseBalanceFlowRatio[MAX_RING_COUNT][MAX_PHASE_COUNT]; 

    //计算各个环内各个相位的绿信比，初始值是特征参数设置的绿信比
	WORD				m_wOriPlanPhaseSplitTime[MAX_RING_COUNT][MAX_PHASE_COUNT];
	
    //方案中设置的绿信比
    WORD				m_wOriPlanPhaseSplitTimePlan[MAX_RING_COUNT][MAX_PHASE_COUNT];

    //记录每个相位优化后的绿信比时间，目前看没有用处，先注释掉
//	int					m_nValidSplitTime[MAX_RING_COUNT][MAX_PHASE_COUNT];
						    
    //根据特征参数计算各个环的最小的可能的周期长
	int					m_nMinBestCycleLength[MAX_RING_COUNT];
    //根据特征参数计算各个环的最大的可能的周期长
	int					m_nMaxBestCycleLength[MAX_RING_COUNT]; 
    
    //各个相位的流量比求和
	float               m_fBalanceTotalFlowRatio;

    //用于优化计算的特征参数
	TAscSingleOptim m_tAscSingleOptimInfo;

	//上周期实际运行的绿信比时间
	WORD				m_wRealSplitTime[MAX_RING_COUNT][MAX_PHASE_COUNT];

	//记录上个周期每个环真实的运行周期
	int					m_nRealCycleLength[MAX_RING_COUNT];

	//每个环对应的相位数
	int					m_nPhaseNum[MAX_RING_COUNT];
};

#endif // !ifndef LOGICCTLACTUATEADAPTIVE_H



















