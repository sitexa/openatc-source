/*=====================================================================
模块名 ：感应控制方式接口模块
文件名 ：LogicCtlActuate.cpp
相关文件：LogicCtlActuate.h,LogicCtlFixedTime.h
实现功能：感应控制方式实现
作者 ：刘黎明
版权 ：<Copyright(c) 2019-2020 Suzhou Keda Technology Co.,Ltd. All right reserved.>
-----------------------------------------------------------------------
修改记录：
日 期           版本     修改人     走读人     修改记录
2019/9/14       V1.0     刘黎明     刘黎明     创建模块
=====================================================================*/
#include "LogicCtlAdaptiveGreenWave.h"
#include <string.h>
#include <cmath>

const float m_fStartUpLostTime = (float)3.0;
const float m_fMinValidPassTime = (float)3.0;

CLogicCtlAdaptiveGreenWave::CLogicCtlAdaptiveGreenWave()
{
}

CLogicCtlAdaptiveGreenWave::~CLogicCtlAdaptiveGreenWave()
{
}

/*====================================================================
函数名 ：Init
功能 ：感应控制方式资源初始化
算法实现 ：
参数说明 ：pParameter，特征参数指针
pRunStatus，全局运行状态类指针
pOpenATCLog，日志指针
nPlanNo，指定的方案号,0表示使用时段对应的方案
返回值说明：
----------------------------------------------------------------------
修改记录 ：
日 期          版本 修改人  走读人  修改记录
2019/09/14     V1.0 刘黎明          创建函数
====================================================================*/
void CLogicCtlAdaptiveGreenWave::Init(COpenATCParameter * pParameter, COpenATCRunStatus * pRunStatus, COpenATCLog * pOpenATCLog, int nPlanNo)
{
	m_pOpenATCParameter = pParameter;
	m_pOpenATCRunStatus = pRunStatus;
	m_pOpenATCLog = pOpenATCLog;

	memset(&m_tFixTimeCtlInfo, 0, sizeof(m_tFixTimeCtlInfo));
	memset(&m_nPhaseNum, 0, sizeof(m_nPhaseNum));
	memset(&m_tRunStageInfo, 0x00, sizeof(m_tRunStageInfo));
	memset(&m_bLastPhaseBeforeBarrier, 0x00, sizeof(m_bLastPhaseBeforeBarrier));
	memset(m_nChannelSplitMode, 0x00, sizeof(m_nChannelSplitMode));
	memset(&m_tAscSingleOptimInfo, 0x00, sizeof(m_tAscSingleOptimInfo));//单点自适应初始化
	memset(&m_nValidSplitTime, 0x00, sizeof(m_nValidSplitTime));
	memset(m_bShieldStatus, 0x00, sizeof(m_bShieldStatus));
	memset(m_bProhibitStatus, 0x00, sizeof(m_bProhibitStatus));
	memset(m_bOldShieldStatus, 0x00, sizeof(m_bOldShieldStatus));
	memset(m_bOldProhibitStatus, 0x00, sizeof(m_bOldProhibitStatus));
	m_fBalanceTotalFlowRatio = (float)0.0;
	for (int i = 0; i < MAX_RING_COUNT; i++)
	{
		for (int j = 0; j < MAX_SEQUENCE_TABLE_COUNT; j++)
		{
			m_fPhaseValidMinVehPassTime[i][j] = m_fMinValidPassTime;
		}
	}

	BYTE byPlanID = 0;
	if (nPlanNo == 0)
	{
		InitByTimeSeg(byPlanID);
	}
	else
	{
		byPlanID = (BYTE)nPlanNo;
		InitByPlan(byPlanID);
	}
	InitOverlapParam();
	InitChannelParam();
	for (int iRingIndex = 0; iRingIndex < m_tFixTimeCtlInfo.m_nRingCount; iRingIndex++)
	{
		m_nPhaseNum[iRingIndex] = m_tFixTimeCtlInfo.m_atPhaseSeq[iRingIndex].m_nPhaseCount;
	}
	m_pOpenATCParameter->GetSingleOptimInfo(byPlanID, m_tAscSingleOptimInfo, m_tFixTimeCtlInfo.m_nRingCount, m_nPhaseNum);
	SetChannelSplitMode();

	TLogicCtlStatus tCtlStatus;
	m_pOpenATCRunStatus->GetLogicCtlStatus(tCtlStatus);
	tCtlStatus.m_nCurCtlMode = CTL_MODE_ACTUATE;
	tCtlStatus.m_nCurPlanNo = (int)byPlanID;
	m_pOpenATCRunStatus->SetLogicCtlStatus(tCtlStatus);
	memset(&m_tWebsterOptimInfo, 0, sizeof(TWebsterOptimInfo));
	memset(m_anDetectorTmpCounter, 0, sizeof(m_anDetectorTmpCounter));
	memset(m_byDetectorTmpStatus, 0, sizeof(m_byDetectorTmpStatus));

	RetsetAllChannelStatus();

	InitDetectorParam();
	InitWebsterOptimParam();

	GetRunStageTable();

	GetLastPhaseBeforeBarrier();

	m_bCalcSplitTimeFlag = false;
	
	
	memset(&m_nSplitTime, 0, sizeof(m_nSplitTime));

	for (int i = 0; i < MAX_RING_COUNT; i++)
	{
		m_tPhasePulseStatus[i].m_nPhaseIndex = 0;
		m_tPhasePulseStatus[i].m_nPhaseNum = m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[m_tPhasePulseStatus[i].m_nPhaseIndex].m_tPhaseParam.m_byPhaseNumber;
		m_tPhasePulseStatus[i].m_bGreenPulseStatus = false;
		m_tPhasePulseStatus[i].m_bRedPulseStatus = false;
		m_tPhasePulseStatus[i].m_nGreenPulseSendStatus = SEND_INIT;
		m_tPhasePulseStatus[i].m_nRedPulseSendStatus = SEND_INIT;
	}

	memset(m_wOriPlanPhaseSplitTimePlan, 0, sizeof(m_wOriPlanPhaseSplitTimePlan));
	for (int i = 0; i < m_tFixTimeCtlInfo.m_nRingCount; i++)
	{
		int nCount = m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_nPhaseCount;

		for (int j = 0; j < nCount; j++)
		{
			for (int k = 0; k < MAX_PHASE_COUNT; k++)
			{
				if (m_atSplitInfo[k].m_bySplitPhase == m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[j].m_tPhaseParam.m_byPhaseNumber)
				{
					m_wOriPlanPhaseSplitTimePlan[i][j] = m_atSplitInfo[k].m_wSplitTime;
					break;
				}
			}
		}
	}
}

/*====================================================================
函数名 ：OnePhaseRun
功能 ：感应控制时单个机动车相位运行状态更新
算法实现 ：
参数说明 ：nRingIndex，环索引
返回值说明：
----------------------------------------------------------------------
修改记录 ：
日 期          版本 修改人  走读人  修改记录
2019/09/14     V1.0 刘黎明          创建函数
====================================================================*/
void CLogicCtlAdaptiveGreenWave::OnePhaseRun(int nRingIndex)
{
	char szInfo[256] = { 0 };
	PTRingCtlInfo ptRingRunInfo = &(m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex]);

	int nIndex = m_tFixTimeCtlInfo.m_nCurPhaseIndex[nRingIndex];
	TPhaseLampClrRunCounter tRunCounter;
	m_pOpenATCRunStatus->GetPhaseLampClrRunCounter(tRunCounter);

	if (m_bCalcSplitTimeFlag)
	{
		CalcPhaseTime();
		m_bCalcSplitTimeFlag = false;
		memset(m_anDetectorTmpCounter, 0, sizeof(m_anDetectorTmpCounter));
		memset(m_byDetectorTmpStatus, 0, sizeof(m_byDetectorTmpStatus));
		memset(&m_nPhaseVehNumInfo, 0x00, sizeof(m_nPhaseVehNumInfo));
	}
	//当前相位阶段是否完成
	if (tRunCounter.m_nLampClrTime[nRingIndex] >= ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPhaseStageRunTime)
	{
		if (nRingIndex == 0)
		{
			m_tFixTimeCtlInfo.m_wCycleRunTime += ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPhaseStageRunTime;
		}

		m_tFixTimeCtlInfo.m_bIsChgStage[nRingIndex] = true;
	}

	if (m_tFixTimeCtlInfo.m_bIsChgStage[nRingIndex])
	{
		//行人相位以机动车相位为准
		m_tFixTimeCtlInfo.m_bIsChgPedStage[nRingIndex] = true;

		char chcurStage = ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseStage;
		PTPhaseCtlInfo pPhaseInfo = (PTPhaseCtlInfo)&(ptRingRunInfo->m_atPhaseInfo[nIndex]);
		WORD wStageTime = 0;
		char chNextStage = this->GetNextPhaseStageInfo(chcurStage, pPhaseInfo, wStageTime);
		ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseStage = chNextStage;
		ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPhaseStageRunTime = wStageTime;

		if (chNextStage == C_CH_PHASESTAGE_G && !ProcPhaseDetStatus(nRingIndex, nIndex))
		{
			PTPhase pPhaseInfo = &(m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nIndex].m_tPhaseParam);
			ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPhaseStageRunTime = pPhaseInfo->m_wPhaseMinimumGreen;

			m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "RingIndex:%d PhaseIndex:%d Set Green RunTime To PhaseMinimumGreen:%d", nRingIndex, nIndex, pPhaseInfo->m_wPhaseMinimumGreen);
		}

		if (chNextStage == C_CH_PHASESTAGE_G)
		{
			ResetPhaseWebsterOptimInfo(nRingIndex, nIndex);//新增
			m_nSplitTime[nRingIndex][nIndex] = ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPhaseStageRunTime + ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPhaseTime - ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPhaseGreenTime;
		}

		m_tFixTimeCtlInfo.m_bIsChgStage[nRingIndex] = false;

		if (chNextStage == C_CH_PHASESTAGE_GF)
		{
			GetGreenFalshCount(VEH_CHA, (PTPhaseCtlInfo)&(ptRingRunInfo->m_atPhaseInfo[nIndex]), wStageTime);
		}

		//设置环内相位灯色，重置时间
		SetLampClrByRing(nRingIndex, nIndex, ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseStage, ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPedPhaseStage);
		tRunCounter.m_nLampClrTime[nRingIndex] = 0;
		tRunCounter.m_nLampClrStartCounter[nRingIndex] = tRunCounter.m_nCurCounter;
		m_pOpenATCRunStatus->SetPhaseLampClrRunCounter(tRunCounter);
		m_bIsLampClrChg = true;
	}

	if (ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseStage == C_CH_PHASESTAGE_G ||
		ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseStage == C_CH_PHASESTAGE_GF)
	{
		ProcPhaseWebsterOptimInfo(nRingIndex, nIndex);
	}
	//判断是否需要延长绿灯时间
	if (ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseStage == C_CH_PHASESTAGE_G)
	{
		ProcExtendGreen(nRingIndex, nIndex, tRunCounter.m_nLampClrTime[nRingIndex]);
	}

	//屏障前的最后一个降级相位绿灯时，其并发相位增加延长绿到最大绿
	if (m_bLastPhaseBeforeBarrier[nRingIndex][nIndex] && ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseStage == C_CH_PHASESTAGE_G && ProcPhaseDetStatus(nRingIndex, nIndex))
	{
		LastConcurrencyPhaseBeforeBarrierExtendGreen(nRingIndex, nIndex, tRunCounter.m_nLampClrTime[nRingIndex]);
	}
}

/*====================================================================
函数名 ：OnePedPhaseRun
功能 ：感应控制时单个行人相位运行状态更新
算法实现 ：
参数说明 ：nRingIndex，环索引
返回值说明：
----------------------------------------------------------------------
修改记录 ：
日 期          版本 修改人  走读人  修改记录
2019/09/14     V1.0 刘黎明          创建函数
====================================================================*/
void CLogicCtlAdaptiveGreenWave::OnePedPhaseRun(int nRingIndex)
{
	PTRingCtlInfo ptRingRunInfo = &(m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex]);

	int nIndex = m_tFixTimeCtlInfo.m_nCurPhaseIndex[nRingIndex];
	TPhaseLampClrRunCounter tRunCounter;
	m_pOpenATCRunStatus->GetPhaseLampClrRunCounter(tRunCounter);

	if (m_tFixTimeCtlInfo.m_bIsChgPedStage[nRingIndex])
	{
		char chcurPedStage = ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPedPhaseStage;
		PTPhaseCtlInfo pPhaseInfo = (PTPhaseCtlInfo)&(ptRingRunInfo->m_atPhaseInfo[nIndex]);
		WORD wPedStageTime = 0;
		char chNextPedStage = this->GetPedNextPhaseStageInfo(chcurPedStage, pPhaseInfo, wPedStageTime);
		ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPedPhaseStage = chNextPedStage;

		m_tFixTimeCtlInfo.m_bIsChgPedStage[nRingIndex] = false;

		if (chNextPedStage == C_CH_PHASESTAGE_GF)
		{
			GetGreenFalshCount(PED_CHA, (PTPhaseCtlInfo)&(ptRingRunInfo->m_atPhaseInfo[nIndex]), wPedStageTime);
		}

		//设置环内相位灯色，重置时间
		SetLampClrByRing(nRingIndex, nIndex, ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseStage, ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPedPhaseStage);
		tRunCounter.m_nPedLampClrTime[nRingIndex] = 0;
		tRunCounter.m_nPedLampClrStartCounter[nRingIndex] = tRunCounter.m_nCurCounter;
		m_pOpenATCRunStatus->SetPhaseLampClrRunCounter(tRunCounter);
		m_bIsLampClrChg = true;
	}
}

/*====================================================================
函数名 ：ProcExtendGreen
功能 ：处理相位是否需要延长绿灯时间直到最大绿
算法实现 ：
参数说明 ：nRingIndex，当前运行的环索引
nPhaseIndex，当前运行的相位索引
nCurRunTime，当前阶段的运行时间
返回值说明：
----------------------------------------------------------------------
修改记录 ：
日 期          版本 修改人  走读人  修改记录
2019/09/14     V1.0 刘黎明          创建函数
2020/03/18     V1.0 李永萍          其他环的屏障前的最后一个相位有车过时也增加绿灯时间
====================================================================*/
void CLogicCtlAdaptiveGreenWave::ProcExtendGreen(int nRingIndex, int nPhaseIndex, int nCurRunTime)
{
	PTRingCtlInfo ptRingRunInfo = &(m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex]);

	bool bVehCome = false;
	char szInfo[256] = { 0 };

	TRealTimeVehDetData tVehDetData;
	m_pOpenATCRunStatus->GetRTVehDetData(tVehDetData);

	for (int i = 0; i < m_tFixTimeCtlInfo.m_nVehDetCount; i++)
	{
		if (m_tFixTimeCtlInfo.m_atVehDetector[i].m_byVehicleDetectorCallPhase ==
			ptRingRunInfo->m_atPhaseInfo[nPhaseIndex].m_tPhaseParam.m_byPhaseNumber)
		{
			if (tVehDetData.m_bIsNewVehCome[i])
			{
				bVehCome = true;
				tVehDetData.m_bIsNewVehCome[i] = false;
			}
		}
	}

	if (bVehCome)
	{
		PTPhase pPhaseInfo = &(ptRingRunInfo->m_atPhaseInfo[nPhaseIndex].m_tPhaseParam);
		//相位初始执行时间为相位最小绿
		ptRingRunInfo->m_atPhaseInfo[nPhaseIndex].m_wPhaseStageRunTime = nCurRunTime + pPhaseInfo->m_byPhasePassage;
		if (ptRingRunInfo->m_atPhaseInfo[nPhaseIndex].m_wPhaseStageRunTime > ptRingRunInfo->m_atPhaseInfo[nPhaseIndex].m_wPhaseGreenTime)
		{
			ptRingRunInfo->m_atPhaseInfo[nPhaseIndex].m_wPhaseStageRunTime = ptRingRunInfo->m_atPhaseInfo[nPhaseIndex].m_wPhaseGreenTime;
		}
		if (ptRingRunInfo->m_atPhaseInfo[nPhaseIndex].m_wPhaseStageRunTime < pPhaseInfo->m_wPhaseMinimumGreen)
		{
			ptRingRunInfo->m_atPhaseInfo[nPhaseIndex].m_wPhaseStageRunTime = pPhaseInfo->m_wPhaseMinimumGreen;
		}

		sprintf(szInfo, "ProcExtendGreen RingIndex:%d CurPhaseIndex:%d PhaseRunTime:%d MaxGreenTime:%d", nRingIndex, nPhaseIndex, ptRingRunInfo->m_atPhaseInfo[nPhaseIndex].m_wPhaseStageRunTime, pPhaseInfo->m_wPhaseMaximum1);
		m_pOpenATCLog->LogOneMessage(LEVEL_INFO, szInfo);

		m_pOpenATCRunStatus->SetRTVehDetData(tVehDetData);

		for (int i = 0; i < m_tFixTimeCtlInfo.m_nRingCount; i++)
		{
			if (i != nRingIndex)
			{
				int nDstPhaseIndex = m_tFixTimeCtlInfo.m_nCurPhaseIndex[i];
				TPhaseLampClrRunCounter tRunCounter;
				m_pOpenATCRunStatus->GetPhaseLampClrRunCounter(tRunCounter);

				if (m_bLastPhaseBeforeBarrier[i][nDstPhaseIndex] && m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_chPhaseStage == C_CH_PHASESTAGE_G)
				{
					m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_wPhaseStageRunTime = tRunCounter.m_nLampClrTime[i] + pPhaseInfo->m_byPhasePassage;
					if (m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_wPhaseStageRunTime > m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_wPhaseGreenTime)//优化的绿灯有效时间
					{
						m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_wPhaseStageRunTime = m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_wPhaseGreenTime;
					}
					if (m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_wPhaseStageRunTime < m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_tPhaseParam.m_wPhaseMinimumGreen)
					{
						m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_wPhaseStageRunTime = m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_tPhaseParam.m_wPhaseMinimumGreen;
					}

					sprintf(szInfo, "ProcExtendGreen DstRingIndex:%d DstPhaseIndex:%d DstPhaseRunTime:%d MaxGreenTime:%d", i, nPhaseIndex, m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_wPhaseStageRunTime, m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_wPhaseGreenTime);
					m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, szInfo);

					m_nSplitTime[i][nDstPhaseIndex] = m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_wPhaseStageRunTime + ptRingRunInfo->m_atPhaseInfo[nDstPhaseIndex].m_wPhaseTime - ptRingRunInfo->m_atPhaseInfo[nDstPhaseIndex].m_wPhaseGreenTime;
				}
			}
		}

		m_nSplitTime[nRingIndex][nPhaseIndex] = ptRingRunInfo->m_atPhaseInfo[nPhaseIndex].m_wPhaseStageRunTime + ptRingRunInfo->m_atPhaseInfo[nPhaseIndex].m_wPhaseTime - ptRingRunInfo->m_atPhaseInfo[nPhaseIndex].m_wPhaseGreenTime;
	}
}

/*====================================================================
函数名 ：LastConcurrencyPhaseBeforeBarrierExtendGreen
功能 ：屏障前的最后一个降级相位(没有配置线圈或线圈故障)的并发相位延长绿
算法实现 ：
参数说明 ：nRingIndex，当前运行的环索引
nPhaseIndex，当前运行的相位索引
nCurRunTime，当前阶段的运行时间
返回值说明：
----------------------------------------------------------------------
修改记录 ：
日 期          版本 修改人  走读人  修改记录
2019/09/14     V1.0 刘黎明          创建函数
====================================================================*/
void CLogicCtlAdaptiveGreenWave::LastConcurrencyPhaseBeforeBarrierExtendGreen(int nRingIndex, int nPhaseIndex, int nCurRunTime)
{
	int   i = 0;

	PTRingCtlInfo ptRingRunInfo = &(m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex]);
	PTPhase pPhaseInfo = &(ptRingRunInfo->m_atPhaseInfo[nPhaseIndex].m_tPhaseParam);

	if ((ptRingRunInfo->m_atPhaseInfo[nPhaseIndex].m_wPhaseStageRunTime - nCurRunTime) >= pPhaseInfo->m_byPhasePassage)
	{
		for (i = 0; i < m_tFixTimeCtlInfo.m_nRingCount; i++)
		{
			if (i != nRingIndex)
			{
				int nDstPhaseIndex = m_tFixTimeCtlInfo.m_nCurPhaseIndex[i];

				TPhaseLampClrRunCounter tRunCounter;
				m_pOpenATCRunStatus->GetPhaseLampClrRunCounter(tRunCounter);

				if (m_bLastPhaseBeforeBarrier[i][nDstPhaseIndex] && m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_chPhaseStage == C_CH_PHASESTAGE_G)
				{
					m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_wPhaseStageRunTime = tRunCounter.m_nLampClrTime[i] + pPhaseInfo->m_byPhasePassage;
					if (m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_wPhaseStageRunTime > m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_wPhaseGreenTime)
					{
						m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_wPhaseStageRunTime = m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_wPhaseGreenTime;
					}
					if (m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_wPhaseStageRunTime < m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_tPhaseParam.m_wPhaseMinimumGreen)
					{
						m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_wPhaseStageRunTime = m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_tPhaseParam.m_wPhaseMinimumGreen;
					}

					//m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "LastConcurrencyPhaseBeforeBarrierExtendGreen DstRingIndex:%d DstPhaseIndex:%d DstPhaseRunTime:%d MaxGreenTime:%d", i, nPhaseIndex, m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_wPhaseStageRunTime, m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_tPhaseParam.m_wPhaseMaximum1);

					m_nSplitTime[i][nDstPhaseIndex] = m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_wPhaseStageRunTime + ptRingRunInfo->m_atPhaseInfo[nDstPhaseIndex].m_wPhaseTime - ptRingRunInfo->m_atPhaseInfo[nDstPhaseIndex].m_wPhaseGreenTime;
				}
			}
		}
	}
}


/*====================================================================
函数名 ：ReSetCtlStatus
功能 ：周期运行结束重置相位运行状态信息
算法实现 ：
参数说明 ：
返回值说明：
----------------------------------------------------------------------
修改记录 ：
日 期          版本 修改人  走读人  修改记录
2019/09/14     V1.0 陈涵燕          创建函数
====================================================================*/
void CLogicCtlAdaptiveGreenWave::ReSetCtlStatus()
{
	CLogicCtlFixedTime::ReSetCtlStatus();
	m_bCalcSplitTimeFlag = true;
}

/*====================================================================
函数名 ：CalcPhaseTime
功能 ：计算下一个周期的绿信比参数
算法实现 ：
参数说明 ：
返回值说明：
----------------------------------------------------------------------
修改记录 ：
日 期          版本 修改人  走读人  修改记录
2019/09/14     V1.0 陈涵燕          创建函数
====================================================================*/
void CLogicCtlAdaptiveGreenWave::CalcPhaseTime()
{
	int  nBestCycleLength = 0;
	int  nIndex = 0;
	bool bIsNeedCalc = false;
	int  nCycleLostTime = 0;
	float fFlowRatio = (float)0.0;


	/// 计算最佳周期时长
	bIsNeedCalc = CalcWebsterBestCycleLength(nIndex, nBestCycleLength, nCycleLostTime, fFlowRatio);

	/// 重新计算绿信比
	if (bIsNeedCalc)
	{
		m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "CalcPhaseTime calculated best cycle length success. RingIndex=%d, nBestCycleLength=%d.", nIndex, nBestCycleLength);

		RecalcWebsterBestSplitTime(nIndex, nBestCycleLength, nCycleLostTime, fFlowRatio);
	}
	else
	{
		RecalcWebsterBestSplitTimeByPlan();
	}
}

/*====================================================================
函数名 ：RecalcWebsterBestSplitTimeByPlan
功能 ：重新计算各相位的绿信比
算法实现 ：
参数说明 ：
返回值说明：
----------------------------------------------------------------------
修改记录 ：
日 期          版本 修改人  走读人  修改记录
2019/09/14     V1.0 陈涵燕          创建函数
2019/10/14     V1.0 梁厅            增加更新分配后的绿信比，用于重新计算饱和度
====================================================================*/
void CLogicCtlAdaptiveGreenWave::RecalcWebsterBestSplitTimeByPlan()
{
	int i = 0;
	int j = 0;
	int nPhaseCount = 0;
	for (i = 0; i < m_tFixTimeCtlInfo.m_nRingCount; i++)
	{
		nPhaseCount = m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_nPhaseCount;
		for (j = 0; j < nPhaseCount; j++)
		{
			if (CheckPhaseIfHavaEffectiveDet(i, j))
			{
				PTPhase pPhaseInfo = &(m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[j].m_tPhaseParam);
				m_wOriPlanPhaseSplitTime[i][j] = pPhaseInfo->m_wPhaseMinimumGreen + pPhaseInfo->m_byGreenFlash + pPhaseInfo->m_byPhaseYellowChange + pPhaseInfo->m_byPhaseRedClear;
			}
			else
			{
				m_wOriPlanPhaseSplitTime[i][j] = m_wOriPlanPhaseSplitTimePlan[i][j];
			}
			SetPhaseSplitTime(i, j, m_wOriPlanPhaseSplitTime[i][j], false);
		}
	}
}
/*====================================================================
函数名 ：RecalcWebsterBestSplitTime
功能 ：重新计算各相位的绿信比
算法实现 ：
参数说明 ：
nIndex，最佳周期长的环索引
nBestCycleTimeLength，最佳周期长
返回值说明：
----------------------------------------------------------------------
修改记录 ：
日 期          版本 修改人  走读人  修改记录
2019/09/14     V1.0 陈涵燕          创建函数
2019/10/14     V1.0 梁厅            增加相位静态因子减少绿灯空放
====================================================================*/
void CLogicCtlAdaptiveGreenWave::RecalcWebsterBestSplitTime(int nIndex, int nBestCycleLength, int nCycleLostTime, float & fFlowRatio)
{
	int i = 0;
	int j = 0;
	int k = 0;
	int nPhaseCount = 0;
	int	nBestCycleLengthInUse = 0;
	int nStartIndex = 0;
	int nBarrierIndex = 0;
	int nRecalcCycleTime = 0;
	int nRecalePhaseNum = 0;
	int nRecalePhaseIndex = 0;
	int nCycleVaildGreenTime = 0;
	int nPhaseGreenTime = 0;

	WORD wOriSplitTime = 0;
	WORD wSplitTime = 0;
	WORD wTotalSplitTime = 0;
	WORD wTotalOriSplitTime = 0;
	WORD wBarrierSplitTimeArr[MAX_SEQUENCE_TABLE_COUNT];

	/// 先处理标准环各个相位的绿信比，记录屏障间的绿信比，供其他环相位绿信比计算时使用
	nPhaseCount = m_tFixTimeCtlInfo.m_atPhaseSeq[nIndex].m_nPhaseCount;
	memset(&wBarrierSplitTimeArr, 0, sizeof(wBarrierSplitTimeArr));

	// 计算用于计算最佳周期时长的相位的绿信比总和及相位的个数
	for (i = 0; i < nPhaseCount; i++)
	{
		if (m_nPhaseVehNumInfo[nIndex][i] == WEBSTER_MULTI_VEH_PASS)
		{
			nRecalcCycleTime += m_wOriPlanPhaseSplitTime[nIndex][i];
			nRecalePhaseNum++;
		}
	}

	if (nRecalePhaseNum > 0)
	{
		nBestCycleLengthInUse = nBestCycleLength - (m_tFixTimeCtlInfo.m_wCycleLen - nRecalcCycleTime);
		nCycleVaildGreenTime = nBestCycleLengthInUse - nCycleLostTime; //需要重分配的总有效绿灯时间，包含绿闪
		for (i = 0; i < nPhaseCount; i++)
		{
			wOriSplitTime = m_wOriPlanPhaseSplitTime[nIndex][i];

			if (m_nPhaseVehNumInfo[nIndex][i] == WEBSTER_MIN_SPLIT_TIME)
			{
				PTPhase pPhaseInfo = &(m_tFixTimeCtlInfo.m_atPhaseSeq[nIndex].m_atPhaseInfo[i].m_tPhaseParam);
				wSplitTime = pPhaseInfo->m_wPhaseMinimumGreen + pPhaseInfo->m_byGreenFlash + pPhaseInfo->m_byPhaseYellowChange + pPhaseInfo->m_byPhaseRedClear;
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "### 333333333 min");
			}
			else if (m_nPhaseVehNumInfo[nIndex][i] == WEBSTER_MAX_SPLIT_TIME)
			{
				PTPhase pPhaseInfo = &(m_tFixTimeCtlInfo.m_atPhaseSeq[nIndex].m_atPhaseInfo[i].m_tPhaseParam);
				wSplitTime = pPhaseInfo->m_wPhaseMaximum1 + pPhaseInfo->m_byGreenFlash + pPhaseInfo->m_byPhaseYellowChange + pPhaseInfo->m_byPhaseRedClear;
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "### 333333333 max");
			}
			else if (m_nPhaseVehNumInfo[nIndex][i] == WEBSTER_MULTI_VEH_PASS)
			{
				nRecalePhaseIndex++;
				PTPhase pPhaseInfo = &(m_tFixTimeCtlInfo.m_atPhaseSeq[nIndex].m_atPhaseInfo[i].m_tPhaseParam);
				if (nRecalePhaseIndex != nRecalePhaseNum)
				{
					float fTotalflowRatio = m_fPhaseBalanceFlowRatio[nIndex][i] / m_fBalanceTotalFlowRatio;
					m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "### 11111111 fTotalflowRatio=%f", fTotalflowRatio);
					nPhaseGreenTime = (int)(nCycleVaildGreenTime * fTotalflowRatio) - pPhaseInfo->m_byPhaseYellowChange + m_tAscSingleOptimInfo.m_fYellowEndLoss + m_tAscSingleOptimInfo.m_fGreenStartUpLoss;
					m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "### 11111111 m_fPhaseFlowRatio[nIndex][%d]=%f. fFlowRatio=%f.nCycleVaildGreenTime=%d", i, m_fPhaseFlowRatio[nIndex][i], fFlowRatio, nCycleVaildGreenTime);
					wSplitTime = nPhaseGreenTime + pPhaseInfo->m_byPhaseYellowChange + pPhaseInfo->m_byPhaseRedClear;
					nBestCycleLengthInUse -= wSplitTime;
					m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "### 11111111 wSplitTime=%d.nBestCycleLengthInUse=%d.nPhaseGreenTime=%d", wSplitTime, nBestCycleLengthInUse, nPhaseGreenTime);
				}
				else
				{
					wSplitTime = nBestCycleLengthInUse;
					m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "### 2222222 wSplitTime=%d.nBestCycleLengthInUse=%d.", wSplitTime, nBestCycleLengthInUse);
				}
			}
			else
			{
				wSplitTime = wOriSplitTime;
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "### 333333333 plan");
			}
			SetPhaseSplitTime(nIndex, i, wSplitTime, false);
			m_wOriPlanPhaseSplitTime[nIndex][i] = wSplitTime;
			m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "### 44444444444 wSplitTime=%d.", wSplitTime);
			wTotalSplitTime += wSplitTime;

			// 到达屏障前了
			if (m_bLastPhaseBeforeBarrier[nIndex][i])
			{
				wBarrierSplitTimeArr[nBarrierIndex] = wTotalSplitTime;
				nBarrierIndex++;

				wTotalSplitTime = 0;
			}
		}
	}
	else
	{
		nBestCycleLengthInUse = nBestCycleLength;

		for (i = 0; i < nPhaseCount; i++)
		{
			wOriSplitTime = m_wOriPlanPhaseSplitTime[nIndex][i];

			if (m_nPhaseVehNumInfo[nIndex][i] == WEBSTER_MIN_SPLIT_TIME)
			{
				PTPhase pPhaseInfo = &(m_tFixTimeCtlInfo.m_atPhaseSeq[nIndex].m_atPhaseInfo[i].m_tPhaseParam);
				wSplitTime = pPhaseInfo->m_wPhaseMinimumGreen + pPhaseInfo->m_byGreenFlash + pPhaseInfo->m_byPhaseYellowChange + pPhaseInfo->m_byPhaseRedClear;
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "### 333333333 min");
			}
			else if (m_nPhaseVehNumInfo[nIndex][i] == WEBSTER_MAX_SPLIT_TIME)
			{
				PTPhase pPhaseInfo = &(m_tFixTimeCtlInfo.m_atPhaseSeq[nIndex].m_atPhaseInfo[i].m_tPhaseParam);
				wSplitTime = pPhaseInfo->m_wPhaseMaximum1 + pPhaseInfo->m_byGreenFlash + pPhaseInfo->m_byPhaseYellowChange + pPhaseInfo->m_byPhaseRedClear;
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "### 333333333 max");
			}
			else
			{
				wSplitTime = wOriSplitTime;
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "### 333333333 plan");
			}
			wSplitTime = SetPhaseSplitTime(nIndex, i, wSplitTime, false);
			m_wOriPlanPhaseSplitTime[nIndex][i] = wSplitTime;
			wTotalSplitTime += wSplitTime;

			// 到达屏障前了
			if (m_bLastPhaseBeforeBarrier[nIndex][i])
			{
				wBarrierSplitTimeArr[nBarrierIndex] = wTotalSplitTime;
				nBarrierIndex++;

				wTotalSplitTime = 0;
			}
		}
	}

	/// 再处理其他环
	for (k = 0; k < m_tFixTimeCtlInfo.m_nRingCount; k++)
	{
		if (k != nIndex)	// 非标准环
		{
			nPhaseCount = m_tFixTimeCtlInfo.m_atPhaseSeq[k].m_nPhaseCount;
			nBarrierIndex = 0;
			for (i = 0; i < nPhaseCount; i++)
			{
				wTotalOriSplitTime += m_wOriPlanPhaseSplitTime[k][i];

				// 其他环到达屏障前了
				if (m_bLastPhaseBeforeBarrier[k][i])
				{
					// 计算出的屏障间的绿信比总和
					wTotalSplitTime = wBarrierSplitTimeArr[nBarrierIndex];
					nBarrierIndex++;

					// 屏障前除最后一个相位外的相位的绿信比计算
					for (j = nStartIndex; j<i; j++)
					{
						// 计算单个相位绿信比
						wOriSplitTime = m_wOriPlanPhaseSplitTime[k][j];
						wSplitTime = wTotalSplitTime * wOriSplitTime / wTotalOriSplitTime;

						wSplitTime = SetPhaseSplitTime(k, j, wSplitTime, false);
						m_wOriPlanPhaseSplitTime[k][j] = wSplitTime;
						wTotalSplitTime -= wSplitTime;
					}
					// 屏障前最后一个相位的绿信比计算
					wSplitTime = wTotalSplitTime;
					SetPhaseSplitTime(k, i, wSplitTime, true);
					m_wOriPlanPhaseSplitTime[k][i] = wSplitTime;
					// 设置下个屏障开始起点
					nStartIndex = i + 1;

					wTotalOriSplitTime = 0;
				}
			}
		}
	}
}

/*====================================================================
函数名 ：SetPhaseSplitTime
功能 ：设置相位绿信比
算法实现 ：
参数说明 ：
nRingIndex，环索引
nPhaseIndex，相位索引
nSplitTime，绿信比
bFlag, 是否为屏障前最后一个相位
返回值说明：
----------------------------------------------------------------------
修改记录 ：
日 期          版本 修改人  走读人  修改记录
2019/09/14     V1.0 陈涵燕          创建函数
====================================================================*/
int CLogicCtlAdaptiveGreenWave::SetPhaseSplitTime(int nRingIndex, int nPhaseIndex, WORD wSplitTime, bool bFlag)
{

	PTPhase pPhaseInfo = &(m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_tPhaseParam);
	if (!bFlag)
	{
		if (wSplitTime < (pPhaseInfo->m_wPhaseMinimumGreen + pPhaseInfo->m_byGreenFlash + pPhaseInfo->m_byPhaseYellowChange + pPhaseInfo->m_byPhaseRedClear))
		{
			wSplitTime = pPhaseInfo->m_wPhaseMinimumGreen + pPhaseInfo->m_byGreenFlash + pPhaseInfo->m_byPhaseYellowChange + pPhaseInfo->m_byPhaseRedClear;
		}
		else if (wSplitTime >(pPhaseInfo->m_wPhaseMaximum1 + pPhaseInfo->m_byGreenFlash + pPhaseInfo->m_byPhaseYellowChange + pPhaseInfo->m_byPhaseRedClear))
		{
			wSplitTime = pPhaseInfo->m_wPhaseMaximum1 + pPhaseInfo->m_byGreenFlash + pPhaseInfo->m_byPhaseYellowChange + pPhaseInfo->m_byPhaseRedClear;
		}
	}

	m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_wPhaseTime = wSplitTime;
	m_nValidSplitTime[nRingIndex][nPhaseIndex] = wSplitTime - pPhaseInfo->m_byGreenFlash - pPhaseInfo->m_byPhaseYellowChange - pPhaseInfo->m_byPhaseRedClear;
	m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "m_nValidSplitTime[nRingIndex][nPhaseIndex]. Ring%d,phase%d, m_wOriPlanPhaseSplitTime[nRingIndex][nPhaseIndex]=%d.", nRingIndex, nPhaseIndex, m_nValidSplitTime[nRingIndex][nPhaseIndex]);
	m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_wPhaseGreenTime = wSplitTime - pPhaseInfo->m_byGreenFlash - pPhaseInfo->m_byPhaseYellowChange - pPhaseInfo->m_byPhaseRedClear;
	m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_wPedPhaseGreenTime = wSplitTime - pPhaseInfo->m_byPhaseYellowChange - pPhaseInfo->m_byPhasePedestrianClear - pPhaseInfo->m_byPhaseRedClear;

	return wSplitTime;
}

/*====================================================================
函数名 ：CalcWebsterBestCycleLength
功能 ：计算上一个周期相位的饱和度
算法实现 ：
参数说明 ：
nIndex，环索引
nBestCycleTimeLength，最佳周期长
返回值说明：
计算成功返回true
检测器故障无法计算返回false
----------------------------------------------------------------------
修改记录 ：
日 期          版本 修改人  走读人  修改记录
2019/09/14     V1.0 陈涵燕          创建函数
2019/10/14     V1.0 梁厅            饱和度增加非自学习模式
2019/11/14     V1.0 梁厅            总饱和度
====================================================================*/
bool CLogicCtlAdaptiveGreenWave::CalcWebsterBestCycleLength(int & nIndex, int & nBestCycleTimeLength, int & nCycleLostTime, float & fFlowRatio)
{
	int i = 0;
	int j = 0;
	int k = 0;
	int nMaxDetVehNum = 0;
	int nPerPhaseDetVehNum = 0;
	int nSaturationFlow = 0;
	int nValidDetCount = 0;
	int nPhaseCount = 0;
	int nValidPhaseCount = 0;
	int nTempBestCycleLength = 0;
	int nRecalcCycleTime = 0;
	int nRecalcValidCycleTime = 0;
	int nTotalCycleLostTime = 0;

	bool bIsCalc = false;

	float fPerPhaseValidVehPassTime = (float)0.0;
	float fTotalFlowRatio = (float)0.0;
	m_fBalanceTotalFlowRatio = (float)0.0;

	nIndex = 0;
	nBestCycleTimeLength = 0;
	memset(&m_nPhaseVehNumInfo, 0x00, sizeof(m_nPhaseVehNumInfo));
	memset(&m_nPhaseValidVehNum, 0x00, sizeof(m_nPhaseValidVehNum));
	memset(m_nMinBestCycleLength, 0x00, sizeof(m_nMinBestCycleLength));
	memset(m_nMaxBestCycleLength, 0x00, sizeof(m_nMaxBestCycleLength));
	memset(&m_fPhaseFlowRatio, 0x00, sizeof(m_fPhaseFlowRatio));
	memset(&m_fPhaseBalanceFlowRatio, 0x00, sizeof(m_fPhaseBalanceFlowRatio));
	for (i = 0; i < m_tFixTimeCtlInfo.m_nRingCount; i++)
	{
		nPhaseCount = m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_nPhaseCount;
		nValidDetCount = 0;
		nValidPhaseCount = 0;
		fTotalFlowRatio = (float)0.0;
		for (j = 0; j < nPhaseCount; j++)
		{
			m_fPhaseValidVehPassTime[i][j] = (float)100.0;
			PTOnePhaseWebsterOptimInfo ptOnePhaseOptimInfo = &m_tWebsterOptimInfo.m_atRingOptimInfo[i].m_atPhaseOptimInfo[j];
			if (ptOnePhaseOptimInfo->m_nDetectorCount)
			{
				for (k = 0; k < ptOnePhaseOptimInfo->m_nDetectorCount; k++)
				{
					BYTE byDetID = ptOnePhaseOptimInfo->m_byDetectorID[k];

					if (ptOnePhaseOptimInfo->m_byDetectorStatus[k] == 0 && ptOnePhaseOptimInfo->m_anValidPassVeh[k] > 0)//正常
					{
						if (ptOnePhaseOptimInfo->m_anValidPassVeh[k] > 1)
						{
							fPerPhaseValidVehPassTime = (float)(ptOnePhaseOptimInfo->m_anValidPassTime[k]) / 1000;
							m_nPhaseVehNumInfo[i][j] = WEBSTER_MULTI_VEH_PASS;

							if (fPerPhaseValidVehPassTime < m_fPhaseValidVehPassTime[i][j])
							{
								m_fPhaseValidVehPassTime[i][j] = fPerPhaseValidVehPassTime;
							}
						}
						else
						{
							if (m_nPhaseVehNumInfo[i][j] != WEBSTER_MULTI_VEH_PASS)
							{
								m_nPhaseVehNumInfo[i][j] = WEBSTER_MIN_SPLIT_TIME;
							}
						}
						nValidDetCount++;
					}
				}
			}
		}

		if (nValidDetCount > 0)
		{
			// 获取具有有效检测器的相位的周期长及饱和流量
			nRecalcCycleTime = 0;
			nRecalcValidCycleTime = 0;
			nSaturationFlow = 0;
			int nPhaseValidGreenTime = 0;
			for (j = 0; j < nPhaseCount; j++)
			{
				PTPhase pPhaseInfo = &(m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[j].m_tPhaseParam);

				if (m_nPhaseVehNumInfo[i][j] == WEBSTER_MULTI_VEH_PASS)
				{
					nPhaseValidGreenTime = m_wOriPlanPhaseSplitTime[i][j] - pPhaseInfo->m_byPhaseYellowChange - pPhaseInfo->m_byPhaseRedClear + m_tAscSingleOptimInfo.m_fYellowEndLoss;
					m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "m_wOriPlanPhaseSplitTime[i][j]. Ring%d,phase%d, m_wOriPlanPhaseSplitTime[i][j]=%d.", i, j, m_wOriPlanPhaseSplitTime[i][j]);
					nRecalcValidCycleTime += nPhaseValidGreenTime;
					nRecalcCycleTime += m_wOriPlanPhaseSplitTime[i][j];

					m_nMinBestCycleLength[i] += pPhaseInfo->m_wPhaseMinimumGreen + pPhaseInfo->m_byGreenFlash + pPhaseInfo->m_byPhaseYellowChange + pPhaseInfo->m_byPhaseRedClear;
					m_nMaxBestCycleLength[i] += pPhaseInfo->m_wPhaseMaximum1 + pPhaseInfo->m_byGreenFlash + pPhaseInfo->m_byPhaseYellowChange + pPhaseInfo->m_byPhaseRedClear;
					if (m_fPhaseValidVehPassTime[i][j]<m_fPhaseValidMinVehPassTime[i][j])
					{
						m_fPhaseValidMinVehPassTime[i][j] = m_fPhaseValidVehPassTime[i][j];
					}
					// 饱和流量
					if (m_tAscSingleOptimInfo.m_bySelfLearn == 1)
					{
						nSaturationFlow += ((int)(round(nPhaseValidGreenTime / m_fPhaseValidMinVehPassTime[i][j])));

					}
					else
					{
						nSaturationFlow += ((int)(round(nPhaseValidGreenTime * m_tAscSingleOptimInfo.m_nMaxSaturatedFlow / 3600.00)));
					}
					m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "CalcPhaseTime calculated best cycle length. Ring%d,phase%d, nPhaseValidGreenTime=%d, m_fPhaseValidMinVehPassTime=%f.", i, j, nPhaseValidGreenTime, m_fPhaseValidMinVehPassTime[i][j]);
					nValidPhaseCount++;
				}
			}
			m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "CalcPhaseTime calculated best cycle length. nValidPhaseCount=%d, nSaturationFlow=%d.", nValidPhaseCount, nSaturationFlow);
			if (nValidPhaseCount > 0)
			{
				for (j = 0; j < nPhaseCount; j++)
				{
					PTOnePhaseWebsterOptimInfo ptOnePhaseOptimInfo = &m_tWebsterOptimInfo.m_atRingOptimInfo[i].m_atPhaseOptimInfo[j];
					if (ptOnePhaseOptimInfo->m_nDetectorCount)
					{
						for (k = 0; k < ptOnePhaseOptimInfo->m_nDetectorCount; k++)
						{
							BYTE byDetID = ptOnePhaseOptimInfo->m_byDetectorID[k];

							if (ptOnePhaseOptimInfo->m_byDetectorStatus[k] == 0)//正常
							{
								nPerPhaseDetVehNum = ptOnePhaseOptimInfo->m_anValidPassVeh[k];
								if (nPerPhaseDetVehNum > m_nPhaseValidVehNum[i][j])
								{
									m_nPhaseValidVehNum[i][j] = nPerPhaseDetVehNum;
								}
							}
						}
					}
					if (m_nPhaseValidVehNum[i][j] > 1)
					{
						// 计算最大流量比之和
						m_fPhaseFlowRatio[i][j] = (float)m_nPhaseValidVehNum[i][j] / (float)nSaturationFlow;
						m_fPhaseBalanceFlowRatio[i][j] = m_fPhaseFlowRatio[i][j] * (1 - m_tAscSingleOptimInfo.m_fStaticWeight) + m_tAscSingleOptimInfo.m_fStaticWeight * m_tAscSingleOptimInfo.m_fPhaseStaticWeight[i][j];
						fTotalFlowRatio += m_fPhaseFlowRatio[i][j];
						m_fBalanceTotalFlowRatio += m_fPhaseBalanceFlowRatio[i][j];
						m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "CalcPhaseTime calculated best cycle length. Ring[%d] Phase[%d] nMaxDetVehNum=%d, TotalFlowRatio=%f...%f.\n", i, j, m_nPhaseValidVehNum[i][j], fTotalFlowRatio, m_fPhaseFlowRatio[i][j]);
						m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "CalcPhaseTime calculated best cycle length. Ring[%d] Phase[%d] nMaxDetVehNum=%d, BalanceTotalFlowRatio=%f...%f.\n", i, j, m_nPhaseValidVehNum[i][j], m_fBalanceTotalFlowRatio, m_fPhaseBalanceFlowRatio[i][j]);
					}
				}

				if (fTotalFlowRatio < 0.9 && fTotalFlowRatio > 0.0)
				{
					// 计算周期总损失时间
					nTotalCycleLostTime = (int)(nValidPhaseCount * (m_tAscSingleOptimInfo.m_fGreenStartUpLoss + m_tAscSingleOptimInfo.m_fYellowEndLoss));

					for (j = 0; j<nPhaseCount; j++)
					{
						if (m_nPhaseVehNumInfo[i][j] == WEBSTER_MULTI_VEH_PASS)
						{
							PTPhase pPhaseInfo = &(m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[j].m_tPhaseParam);
							nTotalCycleLostTime += pPhaseInfo->m_byPhaseRedClear;
						}
					}

					// 计算最佳周期时长
					nTempBestCycleLength = (int)((1.5 * nTotalCycleLostTime + 5) / (1 - fTotalFlowRatio));
					m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "CalcPhaseTime calculated best cycle length. Ring[%d] nTotalCycleLostTime=%d, Old nTempBestCycleLength=%d, min=%d, max=%d.", i, nTotalCycleLostTime, nTempBestCycleLength, m_nMinBestCycleLength[i], m_nMaxBestCycleLength[i]);
					nTempBestCycleLength = (int)(m_tAscSingleOptimInfo.m_fCycleAdjustFactor * nTempBestCycleLength / 100);
					m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "m_fCycleAdjustFactor=%d.", m_tAscSingleOptimInfo.m_fCycleAdjustFactor);
					m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "CalcPhaseTime calculated best cycle length. Ring[%d] nTotalCycleLostTime=%d, nTempBestCycleLength=%d, min=%d, max=%d.", i, nTotalCycleLostTime, nTempBestCycleLength, m_nMinBestCycleLength[i], m_nMaxBestCycleLength[i]);
					if (nTempBestCycleLength < m_nMinBestCycleLength[i])
					{
						nTempBestCycleLength = m_nMinBestCycleLength[i];
						for (j = 0; j < nPhaseCount; j++)
						{
							if (m_nPhaseVehNumInfo[i][j] == WEBSTER_MULTI_VEH_PASS)
							{
								m_nPhaseVehNumInfo[i][j] = WEBSTER_MIN_SPLIT_TIME;
							}
						}
					}
					else if (nTempBestCycleLength > m_nMaxBestCycleLength[i])
					{
						nTempBestCycleLength = m_nMaxBestCycleLength[i];
						for (j = 0; j < nPhaseCount; j++)
						{
							if (m_nPhaseVehNumInfo[i][j] == WEBSTER_MULTI_VEH_PASS)
							{
								m_nPhaseVehNumInfo[i][j] = WEBSTER_MAX_SPLIT_TIME;
							}
						}
					}
					nTempBestCycleLength = nTempBestCycleLength + m_tFixTimeCtlInfo.m_wCycleLen - nRecalcCycleTime;

					if (nTempBestCycleLength > nBestCycleTimeLength)
					{
						nIndex = i;
						nBestCycleTimeLength = nTempBestCycleLength;
						nCycleLostTime = nTotalCycleLostTime;
						fFlowRatio = fTotalFlowRatio;
					}
				}
				else if (fTotalFlowRatio > 0.9 && fTotalFlowRatio < 1.0)
				{
					// 计算周期总损失时间
					nTotalCycleLostTime = (int)(nValidPhaseCount * (m_tAscSingleOptimInfo.m_fGreenStartUpLoss + m_tAscSingleOptimInfo.m_fYellowEndLoss));

					for (j = 0; j < nPhaseCount; j++)
					{
						if (m_nPhaseVehNumInfo[i][j] == WEBSTER_MULTI_VEH_PASS)
						{
							PTPhase pPhaseInfo = &(m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[j].m_tPhaseParam);
							nTotalCycleLostTime += pPhaseInfo->m_byPhaseRedClear;
						}
					}

					// 计算最佳周期时长
					nTempBestCycleLength = (int)((1.6 * nTotalCycleLostTime + 6) / (1 - fTotalFlowRatio));
					m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "CalcPhaseTime calculated best cycle length. Ring[%d] nTotalCycleLostTime=%d, Old nTempBestCycleLength=%d, min=%d, max=%d.", i, nTotalCycleLostTime, nTempBestCycleLength, m_nMinBestCycleLength[i], m_nMaxBestCycleLength[i]);
					nTempBestCycleLength = (int)(m_tAscSingleOptimInfo.m_fCycleAdjustFactor * nTempBestCycleLength / 100);
					m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "m_fCycleAdjustFactor=%d.", m_tAscSingleOptimInfo.m_fCycleAdjustFactor);
					m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "CalcPhaseTime calculated best cycle length. Ring[%d] nTotalCycleLostTime=%d, nTempBestCycleLength=%d, min=%d, max=%d.", i, nTotalCycleLostTime, nTempBestCycleLength, m_nMinBestCycleLength[i], m_nMaxBestCycleLength[i]);
					if (nTempBestCycleLength < m_nMinBestCycleLength[i])
					{
						nTempBestCycleLength = m_nMinBestCycleLength[i];
						for (j = 0; j < nPhaseCount; j++)
						{
							if (m_nPhaseVehNumInfo[i][j] == WEBSTER_MULTI_VEH_PASS)
							{
								m_nPhaseVehNumInfo[i][j] = WEBSTER_MIN_SPLIT_TIME;
							}
						}
					}
					else if (nTempBestCycleLength > m_nMaxBestCycleLength[i])
					{
						nTempBestCycleLength = m_nMaxBestCycleLength[i];
						for (j = 0; j < nPhaseCount; j++)
						{
							if (m_nPhaseVehNumInfo[i][j] == WEBSTER_MULTI_VEH_PASS)
							{
								m_nPhaseVehNumInfo[i][j] = WEBSTER_MAX_SPLIT_TIME;
							}
						}
					}
					nTempBestCycleLength = nTempBestCycleLength + m_tFixTimeCtlInfo.m_wCycleLen - nRecalcCycleTime;

					if (nTempBestCycleLength > nBestCycleTimeLength)
					{
						nIndex = i;
						nBestCycleTimeLength = nTempBestCycleLength;
						nCycleLostTime = nTotalCycleLostTime;
						fFlowRatio = fTotalFlowRatio;
					}
				}
				else
				{
					for (j = 0; j < nPhaseCount; j++)
					{
						m_nPhaseVehNumInfo[i][j] = WEBSTER_NO_VEH_PASS;
					}
				}
			}
			else
			{
				nBestCycleTimeLength = 0;
			}
		}
	}

	// 没有检测器
	bIsCalc = false;
	for (i = 0; i < m_tFixTimeCtlInfo.m_nRingCount; i++)
	{
		nPhaseCount = m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_nPhaseCount;
		for (j = 0; j < nPhaseCount; j++)
		{
			if (m_nPhaseVehNumInfo[i][j] != WEBSTER_NO_VEH_PASS)
			{
				bIsCalc = true;
				break;
			}
		}

		if (bIsCalc)
		{
			break;
		}
	}

	if (!bIsCalc)
	{
		return false;
	}

	return true;
}

/*====================================================================
函数名 ：InitOptimParam
功能 ：初始化优化需要的参数
算法实现 ：
参数说明 ：
返回值说明：
----------------------------------------------------------------------
修改记录 ：
日 期          版本 修改人  走读人  修改记录
2019/09/14     V1.0 陈涵燕          创建函数
====================================================================*/
void CLogicCtlAdaptiveGreenWave::InitWebsterOptimParam()
{
	int i = 0;
	int j = 0;
	int k = 0;
	memset(m_wOriPlanPhaseSplitTime, 0x00, sizeof(m_wOriPlanPhaseSplitTime)); //初始计划绿信比时间

	m_tWebsterOptimInfo.m_nRingCount = m_tFixTimeCtlInfo.m_nRingCount;
	for (i = 0; i < m_tFixTimeCtlInfo.m_nRingCount; i++)
	{
		int nCount = m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_nPhaseCount;
		m_tWebsterOptimInfo.m_atRingOptimInfo[i].m_nPhaseCount = nCount;

		for (j = 0; j < nCount; j++)
		{
			PTOnePhaseWebsterOptimInfo ptOnePhaseOptimInfo = &m_tWebsterOptimInfo.m_atRingOptimInfo[i].m_atPhaseOptimInfo[j];
			PTRingCtlInfo ptRingRunInfo = &(m_tFixTimeCtlInfo.m_atPhaseSeq[i]);

			m_wOriPlanPhaseSplitTime[i][j] = m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[j].m_wPhaseTime;

			int nDetCount = 0;
			for (int k = 0; k < m_tFixTimeCtlInfo.m_nVehDetCount; k++)
			{
				if (m_tFixTimeCtlInfo.m_atVehDetector[k].m_byVehicleDetectorCallPhase ==
					ptRingRunInfo->m_atPhaseInfo[j].m_tPhaseParam.m_byPhaseNumber)
				{
					ptOnePhaseOptimInfo->m_byDetectorID[nDetCount] = m_tFixTimeCtlInfo.m_atVehDetector[k].m_byVehicleDetectorNumber;
					nDetCount++;
				}
			}

			ptOnePhaseOptimInfo->m_nTotalPassTime = 1000 * (ptRingRunInfo->m_atPhaseInfo[j].m_wPhaseGreenTime + ptRingRunInfo->m_atPhaseInfo[j].m_tPhaseParam.m_byGreenFlash);
			ptOnePhaseOptimInfo->m_nDetectorCount = nDetCount;

			m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "InitWebsterOptimParam RingIndex:%d PhaseIndex:%d TotalPassTime:%d", i, j, ptOnePhaseOptimInfo->m_nTotalPassTime);
		}
	}
}

/*====================================================================
函数名 ：ResetPhaseOptimInfo
功能 ：重置相位优化参数
算法实现 ：
参数说明 ：
nRingIndex，环索引
nPhaseIndex，相位索引
返回值说明：
----------------------------------------------------------------------
修改记录 ：
日 期          版本 修改人  走读人  修改记录
2019/09/14     V1.0 陈涵燕          创建函数
====================================================================*/
void CLogicCtlAdaptiveGreenWave::ResetPhaseWebsterOptimInfo(int nRingIndex, int nPhaseIndex)
{
	PTOnePhaseWebsterOptimInfo ptOnePhaseOptimInfo = &m_tWebsterOptimInfo.m_atRingOptimInfo[nRingIndex].m_atPhaseOptimInfo[nPhaseIndex];
	int i = 0;
	for (i = 0; i < ptOnePhaseOptimInfo->m_nDetectorCount; i++)
	{
		ptOnePhaseOptimInfo->m_anValidPassVeh[i] = 0;
		ptOnePhaseOptimInfo->m_byDetectorStatus[i] = 0;
	}
}

/*====================================================================
函数名 ：ProcPhaseOptimInfo
功能 ：计算相位相位优化参数
算法实现 ：
参数说明 ：
nRingIndex，环索引
nPhaseIndex，相位索引
返回值说明：
----------------------------------------------------------------------
修改记录 ：
日 期          版本 修改人  走读人  修改记录
2019/09/14     V1.0 陈涵燕          创建函数
====================================================================*/
void CLogicCtlAdaptiveGreenWave::ProcPhaseWebsterOptimInfo(int nRingIndex, int nPhaseIndex)
{
	PTOnePhaseWebsterOptimInfo ptOnePhaseOptimInfo = &m_tWebsterOptimInfo.m_atRingOptimInfo[nRingIndex].m_atPhaseOptimInfo[nPhaseIndex];

	TRealTimeVehDetData tVehDetData;
	m_pOpenATCRunStatus->GetRTVehDetData(tVehDetData);
	int nGlobalCounter = m_pOpenATCRunStatus->GetGlobalCounter();

	int i = 0;
	int j = 0;
	int nIndex = 0;
	int nDevCount = 0;
	int nTotalCount = 0;
	for (i = 0; i < ptOnePhaseOptimInfo->m_nDetectorCount; i++)
	{
		BYTE byDetID = ptOnePhaseOptimInfo->m_byDetectorID[i];

		if (ptOnePhaseOptimInfo->m_byDetectorStatus[i] == 0)//正常
		{
			for (int j = 0; j < m_tFixTimeCtlInfo.m_nVehDetCount; j++)
			{
				if (m_tFixTimeCtlInfo.m_atVehDetector[j].m_byVehicleDetectorNumber == byDetID)
				{
					if (tVehDetData.m_bDetFaultStatus[j])//故障
					{
						//单个周期只要检测器曾经发生故障就把状态置为故障,计算绿信比时不使用该检测器
						ptOnePhaseOptimInfo->m_byDetectorStatus[i] = 1;
					}
					nIndex = j;
					break;
				}
			}
		}

		if (ptOnePhaseOptimInfo->m_byDetectorStatus[i] == 1)//故障
		{
			break;
		}

		if (tVehDetData.m_chDetStatus[nIndex] == 1)//有车
		{
			if (m_byDetectorTmpStatus[nIndex] == 0)
			{
				m_byDetectorTmpStatus[nIndex] = 1;

				int nPassTime = C_N_TIMER_MILLSECOND * CalcCounter(m_anDetectorTmpCounter[nIndex], nGlobalCounter, C_N_MAXGLOBALCOUNTER);


				if (nPassTime < (float)1000.0)	//过滤掉太小的值
					return;

				if (ptOnePhaseOptimInfo->m_anValidPassTime[i] == 0)
				{
					ptOnePhaseOptimInfo->m_anValidPassTime[i] = nPassTime;
				}
				else if (nPassTime < ptOnePhaseOptimInfo->m_anValidPassTime[i])
				{
					ptOnePhaseOptimInfo->m_anValidPassTime[i] = nPassTime;
				}
				ptOnePhaseOptimInfo->m_anValidPassVeh[i] += 1;
				m_anDetectorTmpCounter[nIndex] = nGlobalCounter;
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "###### ProcPhaseOptimInfo DetID:%d i:%d ValidPassTime:%d ValidPassVeh:%d", byDetID, i, ptOnePhaseOptimInfo->m_anValidPassTime[i], ptOnePhaseOptimInfo->m_anValidPassVeh[i]);
			}
		}
		else
		{
			if (m_byDetectorTmpStatus[nIndex] == 1)
			{
				m_byDetectorTmpStatus[nIndex] = 0;
			}
		}
	}
}

/*====================================================================
函数名 ：CheckPhaseIfHavaEffectiveDet
功能 ：判断当前相位是否有正常使用的检测器
算法实现 ：
参数说明 ：
nRingIndex，环索引
nPhaseIndex，相位索引
返回值说明：
----------------------------------------------------------------------
修改记录 ：
日 期          版本 修改人  走读人  修改记录
2021/01/16     V1.0 陈涵燕          创建函数
====================================================================*/
bool CLogicCtlAdaptiveGreenWave::CheckPhaseIfHavaEffectiveDet(int nRingIndex, int nPhaseIndex)
{
	bool bFlag = false;

	PTOnePhaseWebsterOptimInfo ptOnePhaseOptimInfo = &m_tWebsterOptimInfo.m_atRingOptimInfo[nRingIndex].m_atPhaseOptimInfo[nPhaseIndex];

	TRealTimeVehDetData tVehDetData;
	m_pOpenATCRunStatus->GetRTVehDetData(tVehDetData);

	int i = 0;
	int j = 0;
	for (i = 0; i < ptOnePhaseOptimInfo->m_nDetectorCount; i++)
	{
		BYTE byDetID = ptOnePhaseOptimInfo->m_byDetectorID[i];

		if (ptOnePhaseOptimInfo->m_byDetectorStatus[i] == 0)//正常
		{
			for (int j = 0; j < m_tFixTimeCtlInfo.m_nVehDetCount; j++)
			{
				if (m_tFixTimeCtlInfo.m_atVehDetector[j].m_byVehicleDetectorNumber == byDetID)
				{
					if (tVehDetData.m_bDetFaultStatus[j])//故障
					{
						m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "tVehDetData.m_bDetFaultStatus %d  Fault", j);
						continue;
					}
					else
					{
						bFlag = true;
						break;
					}
				}
			}
		}

		if (bFlag)	//存在正常的检测器
		{
			m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "tVehDetData.m_bDetFaultStatus hava good");
			break;
		}
	}
	return bFlag;
}
