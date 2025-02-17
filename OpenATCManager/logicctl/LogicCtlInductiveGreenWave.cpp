/*=====================================================================
模块名 ：感应式绿波控控制方式接口模块
文件名 ：LogicCtlInductiveGreenWave.cpp
相关文件：LogicCtlInductiveGreenWave.h,LogicCtlInductiveGreenWave.h
实现功能：感应式绿波控控制方式实现
作者 ：梁厅
版权 ：<Copyright(c) 2019-2020 Suzhou Keda Technology Co.,Ltd. All right reserved.>
-----------------------------------------------------------------------
修改记录：
日 期           版本     修改人     走读人     修改记录
2019/9/14       V1.0     梁厅       梁厅      创建模块
=====================================================================*/
#include "LogicCtlInductiveGreenWave.h"
#include <string.h>
#include <stdlib.h>
#include <time.h>

CLogicCtlInductiveGreenWave::CLogicCtlInductiveGreenWave()
{
}

CLogicCtlInductiveGreenWave::~CLogicCtlInductiveGreenWave()
{
    for (int i = 0;i < m_tFixTimeCtlInfo.m_nRingCount;i++)
	{
		 PTRingCtlInfo ptRingRunInfo = &(m_tFixTimeCtlInfo.m_atPhaseSeq[i]);
         int nIndex = m_tFixTimeCtlInfo.m_nCurPhaseIndex[i];

	     SetGreenLampPulse(VEH_CHA, ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhaseNumber, false);
         SetRedLampPulse(VEH_CHA, ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhaseNumber, false); 

		 SetGreenLampPulse(PED_CHA,ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhaseNumber, false);
		 SetRedLampPulse(PED_CHA, ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhaseNumber, false); 
	}
}

/*==================================================================== 
函数名 ：Init 
功能 ：无电缆线控控制方式类资源初始化
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
void CLogicCtlInductiveGreenWave::Init(COpenATCParameter * pParameter, COpenATCRunStatus * pRunStatus, COpenATCLog * pOpenATCLog, int nPlanNo)
{
    m_pOpenATCParameter = pParameter;
    m_pOpenATCRunStatus = pRunStatus;
	m_pOpenATCLog       = pOpenATCLog;
    m_nRealOffset		= 0;
    m_nGreenStageChgLen = 0;
    m_bCalcOffsetFlag	= true;
	m_bOffsetCoordinateCloseFlag = false;//协调结束标识
	m_bCycleCoordinateCloseFlag = false;
    memset(&m_tFixTimeCtlInfo,0,sizeof(m_tFixTimeCtlInfo));
    memset(&m_tRunStageInfo, 0x00, sizeof(m_tRunStageInfo));
    memset(m_nChannelSplitMode,0x00,sizeof(m_nChannelSplitMode)); 
	memset(CurRunTotalCounter, 0x00, sizeof(CurRunTotalCounter));
	memset(m_bShieldStatus, 0x00, sizeof(m_bShieldStatus));
	memset(m_bProhibitStatus, 0x00, sizeof(m_bProhibitStatus));
	memset(m_bOldShieldStatus, 0x00, sizeof(m_bOldShieldStatus));
	memset(m_bOldProhibitStatus, 0x00, sizeof(m_bOldProhibitStatus));

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
	
    SetChannelSplitMode();

    TLogicCtlStatus tCtlStatus;
    m_pOpenATCRunStatus->GetLogicCtlStatus(tCtlStatus);
    tCtlStatus.m_nCurCtlMode = CTL_MODE_CABLELESS;
    tCtlStatus.m_nCurPlanNo = (int)byPlanID;
    m_pOpenATCRunStatus->SetLogicCtlStatus(tCtlStatus);

    RetsetAllChannelStatus();
	InitDetectorParam();
    GetRunStageTable();

	GetLastPhaseBeforeBarrier();

    memset(&m_nSplitTime,0,sizeof(m_nSplitTime));  

	for (int i = 0;i < MAX_RING_COUNT;i++)
	{
		m_tPhasePulseStatus[i].m_nPhaseIndex = 0;
		m_tPhasePulseStatus[i].m_nPhaseNum = m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[m_tPhasePulseStatus[i].m_nPhaseIndex].m_tPhaseParam.m_byPhaseNumber;
		m_tPhasePulseStatus[i].m_bGreenPulseStatus = false;
		m_tPhasePulseStatus[i].m_bRedPulseStatus = false;
		m_tPhasePulseStatus[i].m_nGreenPulseSendStatus = SEND_INIT;
		m_tPhasePulseStatus[i].m_nRedPulseSendStatus = SEND_INIT;
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
2019/09/14     V1.0 李永萍          创建函数 
====================================================================*/ 
void CLogicCtlInductiveGreenWave::ReSetCtlStatus()
{
    CLogicCtlFixedTime::ReSetCtlStatus();
    m_bCalcOffsetFlag = true;
	int nCurCycle = 0;
	if (m_bCycleCoordinateCloseFlag==false)
	{
		for (int i = 0; i < MAX_SEQUENCE_TABLE_COUNT; i++)
		{
			if (m_nSplitTime[0][i] > 0)
			{
				nCurCycle += m_nSplitTime[0][i];
			}
		}
		if (nCurCycle == m_tFixTimeCtlInfo.m_wCycleLen)
			m_bCycleCoordinateCloseFlag = true;
		else
			m_bCycleCoordinateCloseFlag = false;
	}
	
	if (m_bCycleCoordinateCloseFlag&&m_bOffsetCoordinateCloseFlag)
	{
		CalcGreenWavePhaseTime();
	}
}


/*==================================================================== 
函数名 ：OnePhaseRun 
功能 ：无电缆线控控制时单个机动车相位运行状态更新
算法实现 ： 
参数说明 ：nRingIndex，环索引 
返回值说明：
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 刘黎明          创建函数 
====================================================================*/ 
void CLogicCtlInductiveGreenWave::OnePhaseRun(int nRingIndex)
{
	if (m_bOffsetCoordinateCloseFlag == false||m_bCycleCoordinateCloseFlag==false)
	{
		char szInfo[256] = { 0 };
		if (m_bCalcOffsetFlag)
		{
			m_nRealOffset = CalcRealOffset();
			m_bCalcOffsetFlag = false;
			m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "Calcute Real Offset is %d", m_nRealOffset);
		}

		PTRingCtlInfo ptRingRunInfo = &(m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex]);

		int nIndex = m_tFixTimeCtlInfo.m_nCurPhaseIndex[nRingIndex];
		TPhaseLampClrRunCounter tRunCounter;
		m_pOpenATCRunStatus->GetPhaseLampClrRunCounter(tRunCounter);

		//当前相位阶段是否完成
		if (tRunCounter.m_nLampClrTime[nRingIndex] >= ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPhaseStageRunTime)
		{
			if (nRingIndex == 0)
			{
				m_tFixTimeCtlInfo.m_wCycleRunTime += ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPhaseStageRunTime;
			}

			m_tFixTimeCtlInfo.m_bIsChgStage[nRingIndex] = true;

			sprintf(szInfo, "OnePhaseRun RingIndex:%d CurPhaseIndex:%d LampClrTime:%d PhaseStageRunTime:%d", nRingIndex, nIndex, tRunCounter.m_nLampClrTime[nRingIndex], ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPhaseStageRunTime);
			m_pOpenATCLog->LogOneMessage(LEVEL_INFO, szInfo);
		}

		if (m_tFixTimeCtlInfo.m_bIsChgStage[nRingIndex])
		{
			char chcurStage = ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseStage;
			PTPhaseCtlInfo pPhaseInfo = (PTPhaseCtlInfo)&(ptRingRunInfo->m_atPhaseInfo[nIndex]);
			WORD wStageTime = 0;
			char chNextStage = this->GetNextPhaseStageInfo(chcurStage, pPhaseInfo, wStageTime);
			ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseStage = chNextStage;
			ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPhaseStageRunTime = wStageTime;

			if (chNextStage == C_CH_PHASESTAGE_G && nRingIndex == 0)
			{
				if (m_nRealOffset == 0)
				{
					m_nGreenStageChgLen = 0;
				}
				else
				{
					m_nGreenStageChgLen = CalcRealAdjVal(m_nRealOffset,
						wStageTime,
						pPhaseInfo->m_tPhaseParam.m_wPhaseMinimumGreen,
						pPhaseInfo->m_tPhaseParam.m_wPhaseMaximum1);
				}

				sprintf(szInfo, "CalcRealAdjVal CurPhaseIndex:%d RealOffset:%d GreenStageChgLen:%d", nIndex, m_nRealOffset, m_nGreenStageChgLen);
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, szInfo);
			}

			if (chNextStage == C_CH_PHASESTAGE_G)
			{
				ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPhaseStageRunTime -= m_nGreenStageChgLen;
				if (nRingIndex == 0 && m_nRealOffset != 0)
				{
					if (m_nRealOffset < 0)
					{
						if (m_nGreenStageChgLen > 0)
						{
							m_nRealOffset += m_nGreenStageChgLen;
						}
						else
						{
							m_nRealOffset -= m_nGreenStageChgLen;
						}
					}
					else
					{
						if (m_nGreenStageChgLen > 0)
						{
							m_nRealOffset -= m_nGreenStageChgLen;
						}
						else
						{
							m_nRealOffset += m_nGreenStageChgLen;
						}
					}
				}

				sprintf(szInfo, "After Adjust RingIndex:%d CurPhaseIndex:%d PhaseStageRunTime:%d RealOffset:%d", nRingIndex, nIndex, ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPhaseStageRunTime, m_nRealOffset);
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, szInfo);

				m_nSplitTime[nRingIndex][nIndex] = ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPhaseStageRunTime + ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPhaseTime - ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPhaseGreenTime;
			}

			m_tFixTimeCtlInfo.m_bIsChgStage[nRingIndex] = false;

			sprintf(szInfo, "OnePhaseRun RingIndex:%d CurPhaseIndex:%d CurStage:%c NextStage:%c StageTime:%d", nRingIndex, nIndex, chcurStage, chNextStage, ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPhaseStageRunTime);
			m_pOpenATCLog->LogOneMessage(LEVEL_INFO, szInfo);

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

		static bool bGreenPulse[MAX_RING_COUNT] = { false, false, false, false };
		static bool bRedPulse[MAX_RING_COUNT] = { false, false, false, false };

		if (ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseStage == C_CH_PHASESTAGE_G && tRunCounter.m_nLampClrTime[nRingIndex] ==
			(ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPhaseStageRunTime + ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byGreenFlash - ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byGreenPulse))
		{
			if (!bGreenPulse[nRingIndex])
			{
				bGreenPulse[nRingIndex] = true;
				SetGreenLampPulse(VEH_CHA, ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhaseNumber, true);

				m_tPhasePulseStatus[nRingIndex].m_nPhaseIndex = nIndex;
				m_tPhasePulseStatus[nRingIndex].m_nPhaseNum = ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhaseNumber;
				m_tPhasePulseStatus[nRingIndex].m_bGreenPulseStatus = true;
			}
		}
		else
		{
			if (bGreenPulse[nRingIndex])
			{
				bGreenPulse[nRingIndex] = false;
				SetGreenLampPulse(VEH_CHA, ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhaseNumber, false);

				m_tPhasePulseStatus[nRingIndex].m_nPhaseIndex = nIndex;
				m_tPhasePulseStatus[nRingIndex].m_nPhaseNum = ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhaseNumber;
				m_tPhasePulseStatus[nRingIndex].m_bGreenPulseStatus = false;
			}
		}

		int nNextIndex = nIndex + 1;
		if (nNextIndex == ptRingRunInfo->m_nPhaseCount)
		{
			nNextIndex = 0;
		}

		if (ptRingRunInfo->m_atPhaseInfo[nNextIndex].m_tPhaseParam.m_byRedPulse != 0 && GetPhaseCycleRemainTime(nRingIndex, (PTPhaseCtlInfo)&(ptRingRunInfo->m_atPhaseInfo[nIndex]), tRunCounter) == ptRingRunInfo->m_atPhaseInfo[nNextIndex].m_tPhaseParam.m_byRedPulse)
		{
			if (!bRedPulse[nRingIndex])
			{
				bRedPulse[nRingIndex] = true;
				SetRedLampPulse(VEH_CHA, ptRingRunInfo->m_atPhaseInfo[nNextIndex].m_tPhaseParam.m_byPhaseNumber, true);

				m_tPhasePulseStatus[nRingIndex].m_nPhaseIndex = nIndex;
				m_tPhasePulseStatus[nRingIndex].m_nPhaseNum = ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhaseNumber;
				m_tPhasePulseStatus[nRingIndex].m_bRedPulseStatus = true;
			}
		}
		else
		{
			if (bRedPulse[nRingIndex])
			{
				bRedPulse[nRingIndex] = false;
				SetRedLampPulse(VEH_CHA, ptRingRunInfo->m_atPhaseInfo[nNextIndex].m_tPhaseParam.m_byPhaseNumber, false);

				m_tPhasePulseStatus[nRingIndex].m_nPhaseIndex = nIndex;
				m_tPhasePulseStatus[nRingIndex].m_nPhaseNum = ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhaseNumber;
				m_tPhasePulseStatus[nRingIndex].m_bRedPulseStatus = false;
			}
		}
	}
	if (m_bOffsetCoordinateCloseFlag && m_bCycleCoordinateCloseFlag)//相位差执行结束实行感应控制
    {
		char szInfo[256] = { 0 };
		PTRingCtlInfo ptRingRunInfo = &(m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex]);

		int nIndex = m_tFixTimeCtlInfo.m_nCurPhaseIndex[nRingIndex];
		TPhaseLampClrRunCounter tRunCounter;
		m_pOpenATCRunStatus->GetPhaseLampClrRunCounter(tRunCounter);

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
				if (nIndex==0)
				{
					ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPhaseStageRunTime = m_nGreenPhaseSplitTime[nRingIndex] - pPhaseInfo->m_byGreenFlash - pPhaseInfo->m_byPhaseRedClear - pPhaseInfo->m_byPhaseYellowChange;
				}
				else
				{
					ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPhaseStageRunTime = pPhaseInfo->m_wPhaseMinimumGreen;
				}
				
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "RingIndex:%d PhaseIndex:%d Set Green RunTime To PhaseMinimumGreen:%d", nRingIndex, nIndex, pPhaseInfo->m_wPhaseMinimumGreen);
			}

			if (chNextStage == C_CH_PHASESTAGE_G)
			{
				m_nSplitTime[nRingIndex][nIndex] = ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPhaseStageRunTime + ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPhaseTime - ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPhaseGreenTime;
			}

			m_tFixTimeCtlInfo.m_bIsChgStage[nRingIndex] = false;

			//sprintf(szInfo, "OnePhaseRun RingIndex:%d CurPhaseIndex:%d CurStage:%c NextStage:%c StageTime:%d", nRingIndex, nIndex, chcurStage, chNextStage, wStageTime);
			//m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, szInfo);

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

		//判断是否需要延长绿灯时间
		if (ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseStage == C_CH_PHASESTAGE_G&&nIndex!=0)
		{
			ProcExtendGreen(nRingIndex, nIndex, tRunCounter.m_nLampClrTime[nRingIndex]);
		}

		//屏障前的最后一个降级相位绿灯时，其并发相位增加延长绿到最大绿
		if (m_bLastPhaseBeforeBarrier[nRingIndex][nIndex] && ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseStage == C_CH_PHASESTAGE_G && ProcPhaseDetStatus(nRingIndex, nIndex))
		{
			LastConcurrencyPhaseBeforeBarrierExtendGreen(nRingIndex, nIndex, tRunCounter.m_nLampClrTime[nRingIndex]);
		}
   }
}

/*==================================================================== 
函数名 ：OnePedPhaseRun
功能 ：无电缆线控控制时单个行人相位运行状态更新
算法实现 ： 
参数说明 ：nRingIndex，环索引 
返回值说明：
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 刘黎明          创建函数 
====================================================================*/ 
void CLogicCtlInductiveGreenWave::OnePedPhaseRun(int nRingIndex)
{
	if (m_bOffsetCoordinateCloseFlag == false || m_bCycleCoordinateCloseFlag == false)
	{
		PTRingCtlInfo ptRingRunInfo = &(m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex]);

		int nIndex = m_tFixTimeCtlInfo.m_nCurPhaseIndex[nRingIndex];
		TPhaseLampClrRunCounter tRunCounter;
		m_pOpenATCRunStatus->GetPhaseLampClrRunCounter(tRunCounter);

		//当前相位阶段是否完成
		if (tRunCounter.m_nPedLampClrTime[nRingIndex] >= ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPedPhaseStageRunTime)
		{
			m_tFixTimeCtlInfo.m_bIsChgPedStage[nRingIndex] = true;
		}

		if (m_tFixTimeCtlInfo.m_bIsChgPedStage[nRingIndex])
		{
			char chcurPedStage = ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPedPhaseStage;
			PTPhaseCtlInfo pPhaseInfo = (PTPhaseCtlInfo)&(ptRingRunInfo->m_atPhaseInfo[nIndex]);
			WORD wPedStageTime = 0;
			char chNextPedStage = this->GetPedNextPhaseStageInfo(chcurPedStage, pPhaseInfo, wPedStageTime);
			ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPedPhaseStage = chNextPedStage;
			ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPedPhaseStageRunTime = wPedStageTime;

			//行人相位跟随机动车相位一起调整
			if (chNextPedStage == C_CH_PHASESTAGE_G)
			{
				ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPedPhaseStageRunTime -= m_nGreenStageChgLen;
			}

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

		static bool bGreenPulse[MAX_RING_COUNT] = { false, false, false, false };
		static bool bRedPulse[MAX_RING_COUNT] = { false, false, false, false };

		if (ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPedPhaseStage == C_CH_PHASESTAGE_G && tRunCounter.m_nPedLampClrTime[nRingIndex] ==
			(ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPedPhaseStageRunTime + ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhasePedestrianClear - ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byGreenPulse))
		{
			if (!bGreenPulse[nRingIndex])
			{
				bGreenPulse[nRingIndex] = true;
				SetGreenLampPulse(PED_CHA, ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhaseNumber, true);
			}
		}
		else
		{
			if (bGreenPulse[nRingIndex])
			{
				bGreenPulse[nRingIndex] = false;
				SetGreenLampPulse(PED_CHA, ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhaseNumber, false);
			}
		}

		int nNextIndex = nIndex + 1;
		if (nNextIndex == ptRingRunInfo->m_nPhaseCount)
		{
			nNextIndex = 0;
		}

		if (ptRingRunInfo->m_atPhaseInfo[nNextIndex].m_tPhaseParam.m_byRedPulse != 0 && GetPedPhaseCycleRemainTime(nRingIndex, (PTPhaseCtlInfo)&(ptRingRunInfo->m_atPhaseInfo[nIndex]), tRunCounter) == ptRingRunInfo->m_atPhaseInfo[nNextIndex].m_tPhaseParam.m_byRedPulse)
		{
			if (!bRedPulse[nRingIndex])
			{
				bRedPulse[nRingIndex] = true;
				SetRedLampPulse(PED_CHA, ptRingRunInfo->m_atPhaseInfo[nNextIndex].m_tPhaseParam.m_byPhaseNumber, true);
			}
		}
		else
		{
			if (bRedPulse[nRingIndex])
			{
				bRedPulse[nRingIndex] = false;
				SetRedLampPulse(PED_CHA, ptRingRunInfo->m_atPhaseInfo[nNextIndex].m_tPhaseParam.m_byPhaseNumber, false);
			}
		}
	}
	if (m_bOffsetCoordinateCloseFlag && m_bCycleCoordinateCloseFlag)
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
}

/*==================================================================== 
函数名 ：CalcRealOffset
功能 ：无电缆线控控制时计算实际相位差,修正相位绿灯时间
算法实现 ： 
参数说明 ：
返回值说明：
        当前实际的相位差，以0点作为统一基准.
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 刘黎明          创建函数 
====================================================================*/ 
short CLogicCtlInductiveGreenWave::CalcRealOffset()
{
	char szInfo[256] = { 0 };
    int nYear,nMonth,nDay,nHour,nMin,nSec,nWeek;
    OpenATCGetCurTime(nYear,nMonth,nDay,nHour,nMin,nSec,nWeek);

    long wSecOffset = time(NULL);//(WORD)(nHour * C_N_SECONDS_PERHOUR + nMin * C_N_SECONDS_PERMIN + nSec);

    WORD wRealOffset = wSecOffset % m_tFixTimeCtlInfo.m_wCycleLen;

    if (wRealOffset == m_tFixTimeCtlInfo.m_wPhaseOffset)
    {
		m_bOffsetCoordinateCloseFlag = true;
		sprintf(szInfo, "m_bOffsetCoordinateCloseFlag = true");
		m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, szInfo);
        return 0;
    }

    short nDiff = (short)wRealOffset - (short)m_tFixTimeCtlInfo.m_wPhaseOffset;

    if (nDiff > 0)
    {
        if (nDiff > m_tFixTimeCtlInfo.m_wCycleLen / 2)
        {
            nDiff = nDiff - m_tFixTimeCtlInfo.m_wCycleLen;
        }

    }
    else
    { 
        if (nDiff + m_tFixTimeCtlInfo.m_wCycleLen / 2 < 0)
        {
            nDiff = nDiff + m_tFixTimeCtlInfo.m_wCycleLen;
        }   
    }

    return nDiff;     
}

short CLogicCtlInductiveGreenWave::CalcRealAdjVal(short nOffset, WORD wGreenTime, WORD wMinGreen, WORD wMaxGreen)
{
    short nTmpAdj = (short)(wGreenTime * C_F_ADJ_KEY_MODE);

    if (abs(nTmpAdj) >= abs(nOffset))
    {
        nTmpAdj = nOffset;
    }
    else
    {   
        if (nOffset < 0)
        {
            nTmpAdj = -nTmpAdj;
        }
    }

    if (nTmpAdj < 0)
    {
        if ((wGreenTime - nTmpAdj) > wMaxGreen)
        {
            nTmpAdj = wGreenTime - wMaxGreen;
        }
    }
    else
    {
        if ((wGreenTime - nTmpAdj) < wMinGreen)
        {
            nTmpAdj = wGreenTime - wMinGreen;
        }
    }

    return nTmpAdj;
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
2021/03/26     V1.0 梁厅			创建函数
====================================================================*/
void CLogicCtlInductiveGreenWave::ProcExtendGreen(int nRingIndex, int nPhaseIndex, int nCurRunTime)
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
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "Having NewVeh Come in:%d", i);
				tVehDetData.m_bIsNewVehCome[i] = false;
			}
		}
	}

	if (bVehCome)
	{
		PTPhase pPhaseInfo = &(ptRingRunInfo->m_atPhaseInfo[nPhaseIndex].m_tPhaseParam);
		//相位初始执行时间为相位最小绿
		
		ptRingRunInfo->m_atPhaseInfo[nPhaseIndex].m_wPhaseStageRunTime = nCurRunTime + pPhaseInfo->m_byPhasePassage;
		if (ptRingRunInfo->m_atPhaseInfo[nPhaseIndex].m_wPhaseStageRunTime > pPhaseInfo->m_wPhaseMaximum1)
		{
			ptRingRunInfo->m_atPhaseInfo[nPhaseIndex].m_wPhaseStageRunTime = pPhaseInfo->m_wPhaseMaximum1;
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
			int nDstPhaseIndex = m_tFixTimeCtlInfo.m_nCurPhaseIndex[i];
			TPhaseLampClrRunCounter tRunCounter;
			m_pOpenATCRunStatus->GetPhaseLampClrRunCounter(tRunCounter);
			if (m_bLastPhaseBeforeBarrier[i][nDstPhaseIndex] && m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_chPhaseStage == C_CH_PHASESTAGE_G)
			{
				m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_wPhaseStageRunTime = tRunCounter.m_nLampClrTime[i] + pPhaseInfo->m_byPhasePassage;
				CurRunTotalCounter[i][nDstPhaseIndex] = CurRunTotalCounter[i][nDstPhaseIndex] + pPhaseInfo->m_byPhasePassage;
				if (m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_wPhaseStageRunTime > m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_tPhaseParam.m_wPhaseMaximum1)
				{
					m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_wPhaseStageRunTime = m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_tPhaseParam.m_wPhaseMaximum1;
				}
				if (m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_wPhaseStageRunTime < m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_tPhaseParam.m_wPhaseMinimumGreen)
				{
					m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_wPhaseStageRunTime = m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_tPhaseParam.m_wPhaseMinimumGreen;
				}

				sprintf(szInfo, "ProcExtendGreen DstRingIndex:%d DstPhaseIndex:%d DstPhaseRunTime:%d MaxGreenTime:%d", i, nPhaseIndex, m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_wPhaseStageRunTime, m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_tPhaseParam.m_wPhaseMaximum1);
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, szInfo);

				m_nSplitTime[i][nDstPhaseIndex] = m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_wPhaseStageRunTime + ptRingRunInfo->m_atPhaseInfo[nDstPhaseIndex].m_wPhaseTime - ptRingRunInfo->m_atPhaseInfo[nDstPhaseIndex].m_wPhaseGreenTime;
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
void CLogicCtlInductiveGreenWave::LastConcurrencyPhaseBeforeBarrierExtendGreen(int nRingIndex, int nPhaseIndex, int nCurRunTime)
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
					if (m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_wPhaseStageRunTime > m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_tPhaseParam.m_wPhaseMaximum1)
					{
						m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_wPhaseStageRunTime = m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_tPhaseParam.m_wPhaseMaximum1;
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
函数名 ：CalcGreenWavePhaseTime
功能 ：周期结束后计算绿波相位的时间
算法实现 ：协调相位的绿灯时间为非协调相位的最大绿灯时间减去其实际运行绿时间
参数说明 ：
返回值说明：
----------------------------------------------------------------------
修改记录 ：
日 期          版本 修改人  走读人  修改记录
2019/09/14     V1.0 梁厅          创建函数
====================================================================*/
void CLogicCtlInductiveGreenWave::CalcGreenWavePhaseTime()
{
	memset(m_nGreenPhaseSplitTime, 0, MAX_RING_COUNT);
	int nNoGreenPhaseSplitTime[MAX_RING_COUNT] = { 0 };
	for (int i = 0; i < MAX_RING_COUNT;i++)
	{
		for (int j = 1; j < MAX_SEQUENCE_TABLE_COUNT;j++)
		{
			if (m_nSplitTime[i][j]>0)
			{
				nNoGreenPhaseSplitTime[i]+= m_nSplitTime[i][j];
			}
		}
		
	}
	for (int n = 0; n < MAX_RING_COUNT;n++)
	{
		m_nGreenPhaseSplitTime[n] = m_tFixTimeCtlInfo.m_wCycleLen - nNoGreenPhaseSplitTime[n];
	}
}