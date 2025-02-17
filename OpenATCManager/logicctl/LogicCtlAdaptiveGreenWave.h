/*=====================================================================
模块名 ：自适应式绿波控制方式实现模块
文件名 ：LogicCtlAdaptiveGreenWave.h
相关文件：
实现功能：用于自适应式绿波控制方式实现接口
作者 ：刘黎明
版权 ：<Copyright(c) 2019-2020 Suzhou Keda Technology Co.,Ltd. All right reserved.>
-----------------------------------------------------------------------
修改记录：
日 期           版本     修改人     走读人     修改记录
2019/9/26       V1.0     刘黎明     刘黎明     创建模块
=====================================================================*/

#ifndef LOGICCTLADAPTIVEGREENWAVE_H
#define LOGICCTLADAPTIVEGREENWAVE_H

#include "LogicCtlFixedTime.h"
#include "LogicCtlWebsterOptim.h"

class CLogicCtlAdaptiveGreenWave : public CLogicCtlFixedTime
{
public:
	CLogicCtlAdaptiveGreenWave();
	virtual ~CLogicCtlAdaptiveGreenWave();

	//初始化感应控制方式需要的参数
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

	//处理相位优化参数
	void ProcPhaseWebsterOptimInfo(int nRingIndex, int nPhaseIndex);

	//计算相位饱和度
	bool CalcWebsterBestCycleLength(int & nIndex, int & nBestCycleTimeLength, int & nCycleLostTime, float & fFlowRatio);

	//重新计算各相位绿信比
	void RecalcWebsterBestSplitTime(int nIndex, int nBestCycleLength, int nCycleLostTime, float & fFlowRatio);
	void RecalcWebsterBestSplitTimeByPlan();

	//设置相位绿信比
	int  SetPhaseSplitTime(int nRingIndex, int nPhaseIndex, WORD wSplitTime, bool bFlag);

	//判断当前相位是否有正常使用的检测器
	bool CheckPhaseIfHavaEffectiveDet(int nRingIndex, int nPhaseIndex);

	bool m_bCalcSplitTimeFlag;																	//绿信比计算标志

	BYTE m_byDetectorTmpStatus[MAX_VEHICLEDETECTOR_COUNT];										//临时存储检测器状态,用于计算有效通行时间
	int m_anDetectorTmpCounter[C_N_MAXDETBOARD_NUM * C_N_MAXDETINPUT_NUM];						//临时车检器发生时间戳计数,用于计算有效通行时间

	TWebsterOptimInfo   m_tWebsterOptimInfo;													//优化信息
	int					m_nPhaseVehNumInfo[MAX_RING_COUNT][MAX_SEQUENCE_TABLE_COUNT];
	int					m_nPhaseValidVehNum[MAX_RING_COUNT][MAX_SEQUENCE_TABLE_COUNT];
	float				m_fPhaseValidVehPassTime[MAX_RING_COUNT][MAX_SEQUENCE_TABLE_COUNT];
	float				m_fPhaseValidMinVehPassTime[MAX_RING_COUNT][MAX_SEQUENCE_TABLE_COUNT]; //记录路口相位的最小车头时距
	float				m_fPhaseFlowRatio[MAX_RING_COUNT][MAX_SEQUENCE_TABLE_COUNT];
	float               m_fPhaseBalanceFlowRatio[MAX_RING_COUNT][MAX_SEQUENCE_TABLE_COUNT];
	WORD				m_wOriPlanPhaseSplitTime[MAX_RING_COUNT][MAX_SEQUENCE_TABLE_COUNT];
	WORD				m_wOriPlanPhaseSplitTimePlan[MAX_RING_COUNT][MAX_SEQUENCE_TABLE_COUNT];
	int					m_nMinBestCycleLength[MAX_RING_COUNT];
	int					m_nMaxBestCycleLength[MAX_RING_COUNT];
	float               m_fBalanceTotalFlowRatio;
	int					m_nValidSplitTime[MAX_RING_COUNT][MAX_PHASE_COUNT];						//记录每个相位优化后的绿信比时间

	TAscSingleOptim m_tAscSingleOptimInfo;														//单点自适应控制参数

	//每个环对应的相位数
	int					m_nPhaseNum[MAX_RING_COUNT];
};

#endif // !ifndef LOGICCTLACTUATE_H



















