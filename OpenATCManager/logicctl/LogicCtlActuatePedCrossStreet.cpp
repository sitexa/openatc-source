/*=====================================================================
模块名 ：感应式行人过街控制方式接口模块
文件名 ：LogicCtlActuatePedCrossStreet.cpp
相关文件：LogicCtlActuatePedCrossStreet.h,LogicCtlFixedTime.h
实现功能：感应式行人过街控制方式实现
作者 ：陈涵燕
版权 ：<Copyright(c) 2019-2021 Suzhou Keda Technology Co.,Ltd. All right reserved.>
-----------------------------------------------------------------------
修改记录：
日 期           版本     修改人     走读人     修改记录
2020/11/24       V1.0     陈涵燕     陈涵燕     创建模块
=====================================================================*/
#include "LogicCtlActuatePedCrossStreet.h"
#include <string.h>
#include <stdlib.h>


const int MAX_VEH_QUEUE_LENGTH = 10;
const int MAX_PED_WAIT_LENGTH  = 10;
CLogicCtlActuatePedCrossStreet::CLogicCtlActuatePedCrossStreet()
{

}

CLogicCtlActuatePedCrossStreet::~CLogicCtlActuatePedCrossStreet()
{

}

/*==================================================================== 
函数名 ：Init 
功能 ：感应式行人过街控制方式类资源初始化
算法实现 ： 
参数说明 ：pParameter，特征参数指针
           pRunStatus，全局运行状态类指针
		   pOpenATCLog，日志指针
           nPlanNo，指定的方案号,0表示使用时段对应的方案  
返回值说明：
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2020/11/24     V1.0 陈涵燕          创建函数 
====================================================================*/ 
void CLogicCtlActuatePedCrossStreet::Init(COpenATCParameter * pParameter, COpenATCRunStatus * pRunStatus, COpenATCLog * pOpenATCLog, int nPlanNo)
{
    m_pOpenATCParameter = pParameter;
    m_pOpenATCRunStatus = pRunStatus;
	m_pOpenATCLog       = pOpenATCLog;
    memset(&m_tFixTimeCtlInfo,0,sizeof(m_tFixTimeCtlInfo));
    memset(&m_tRunStageInfo, 0x00, sizeof(m_tRunStageInfo));
    memset(m_nChannelSplitMode,0x00,sizeof(m_nChannelSplitMode)); 
	memset(m_bShieldStatus, 0x00, sizeof(m_bShieldStatus));
	memset(m_bProhibitStatus, 0x00, sizeof(m_bProhibitStatus));
	memset(m_bOldShieldStatus, 0x00, sizeof(m_bOldShieldStatus));
	memset(m_bOldProhibitStatus, 0x00, sizeof(m_bOldProhibitStatus));

	//当前运行模式为感应式行人过街控制
	m_nCurRunMode = CTL_MODE_ACTUATE_PEDCROSTREET;

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
	m_byPlanID = byPlanID;
    InitOverlapParam();
    InitChannelParam();
    InitPedDetectorParam();

    SetChannelSplitMode();

    TLogicCtlStatus tCtlStatus;
    m_pOpenATCRunStatus->GetLogicCtlStatus(tCtlStatus);
    tCtlStatus.m_nCurCtlMode = CTL_MODE_ACTUATE;
    tCtlStatus.m_nCurPlanNo = (int)byPlanID;
    m_pOpenATCRunStatus->SetLogicCtlStatus(tCtlStatus);

    RetsetAllChannelStatus();

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

	memset(m_nPedDetecetorType, 0 , sizeof(m_nPedDetecetorType));
	memset(m_nPhaseThresholdVehicle, 0 , sizeof(m_nPhaseThresholdVehicle));
	memset(m_nPhaseThresholdPedWait, 0 , sizeof(m_nPhaseThresholdPedWait));

	memset(m_bKeepGreenChannelBeforeControlChannelFlag, false, sizeof(m_bKeepGreenChannelBeforeControlChannelFlag));
}

/*==================================================================== 
函数名 ：OnePhaseRun 
功能 ：感应式行人过街控制时单个机动车相位运行状态更新
算法实现 ： 
参数说明 ：nRingIndex，环索引 
返回值说明：
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2020/11/24     V1.0 陈涵燕          创建函数 
====================================================================*/ 
void CLogicCtlActuatePedCrossStreet::OnePhaseRun(int nRingIndex)
{
	char szInfo[256] = {0};
	PTRingCtlInfo ptRingRunInfo = &(m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex]);
	int nIndex = m_tFixTimeCtlInfo.m_nCurPhaseIndex[nRingIndex];

	TPhaseLampClrRunCounter tRunCounter;
	m_pOpenATCRunStatus->GetPhaseLampClrRunCounter(tRunCounter);

	if (m_atSplitInfo[ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhaseNumber - 1].m_bySplitMode != PED_MODE)
	{
		//当前相位阶段是否完成
		if (tRunCounter.m_nLampClrTime[nRingIndex] >= ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPhaseStageRunTime)
		{
			if (nRingIndex == 0)
			{
				m_tFixTimeCtlInfo.m_wCycleRunTime += ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPhaseStageRunTime;            
			}
			m_tFixTimeCtlInfo.m_bIsChgStage[nRingIndex] = true;
		}
	}

	bool bFlag = true;
	if (m_atSplitInfo[ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhaseNumber - 1].m_bySplitMode == PED_MODE)
	{
		bFlag = ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPedPhaseStage != C_CH_PHASESTAGE_U;
	}

	if (m_tFixTimeCtlInfo.m_bIsChgStage[nRingIndex] && bFlag)
	{
		char chcurStage = ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseStage;
		PTPhaseCtlInfo pPhaseInfo = (PTPhaseCtlInfo)&(ptRingRunInfo->m_atPhaseInfo[nIndex]);
		WORD wStageTime = 0;
		char chNextStage = 0;

		if (m_atSplitInfo[ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhaseNumber - 1].m_bySplitMode != PED_MODE)
		{
			//行人相位以机动车相位为准
			m_tFixTimeCtlInfo.m_bIsChgPedStage[nRingIndex] = true;
			chNextStage =  this->GetNextPhaseStageInfo(chcurStage,pPhaseInfo,wStageTime);
			ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPhaseStageRunTime = wStageTime;
		}
		else
		{
			chNextStage =  this->GetPedNextPhaseStageInfo(chcurStage,pPhaseInfo,wStageTime);
			ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPhaseStageRunTime = wStageTime;
			//ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPedPhaseStageRunTime = wStageTime;
		}

		ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseStage = chNextStage;
		
		if (chNextStage == C_CH_PHASESTAGE_G)
		{
			if (m_atSplitInfo[ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhaseNumber - 1].m_bySplitMode != PED_MODE)
			{
				m_nSplitTime[nRingIndex][nIndex] = ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPhaseStageRunTime + ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPhaseTime - ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPhaseGreenTime;
			}
			else
			{
				m_nSplitTime[nRingIndex][nIndex] = ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPedPhaseStageRunTime + ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPhaseTime - ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPedPhaseGreenTime;
			}
		}

		m_tFixTimeCtlInfo.m_bIsChgStage[nRingIndex] = false;

		if (chNextStage == C_CH_PHASESTAGE_GF)
		{
			GetGreenFalshCount(VEH_CHA, (PTPhaseCtlInfo)&(ptRingRunInfo->m_atPhaseInfo[nIndex]), wStageTime);
		}

		//设置环内相位灯色，重置时间
		SetLampClrByRing(nRingIndex,nIndex,ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseStage,ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPedPhaseStage);
		tRunCounter.m_nLampClrTime[nRingIndex] = 0;
		tRunCounter.m_nLampClrStartCounter[nRingIndex] = tRunCounter.m_nCurCounter;    
		m_pOpenATCRunStatus->SetPhaseLampClrRunCounter(tRunCounter);
		m_bIsLampClrChg = true;
		if (chcurStage == C_CH_PHASESTAGE_F && chNextStage == C_CH_PHASESTAGE_F)
		{
			m_bIsLampClrChg = false;
		}
	} 

	if (m_atSplitInfo[ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhaseNumber - 1].m_bySplitMode != PED_MODE)
	{
		//判断是否需要延长绿灯时间
		if (ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseStage == C_CH_PHASESTAGE_G)
		{
			ProcExtendGreen(nRingIndex,nIndex,tRunCounter.m_nLampClrTime[nRingIndex]);
		}

		//屏障前的最后一个降级相位绿灯时，其并发相位增加延长绿到最大绿
		if (m_bLastPhaseBeforeBarrier[nRingIndex][nIndex] && ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseStage == C_CH_PHASESTAGE_G && ProcPhaseDetStatus(nRingIndex,nIndex))
		{
			//LastConcurrencyPhaseBeforeBarrierExtendGreen(nRingIndex,nIndex,tRunCounter.m_nLampClrTime[nRingIndex]);
		}
	}
}

/*==================================================================== 
函数名 ：OnePedPhaseRun
功能 ：感应式行人过街控制时单个行人相位运行状态更新
算法实现 ： 
参数说明 ：nRingIndex，环索引 
返回值说明：
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2020/11/24     V1.0 陈涵燕          创建函数 
====================================================================*/ 
void CLogicCtlActuatePedCrossStreet::OnePedPhaseRun(int nRingIndex)
{
	PTRingCtlInfo ptRingRunInfo = &(m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex]);

	int nIndex = m_tFixTimeCtlInfo.m_nCurPhaseIndex[nRingIndex];
	TPhaseLampClrRunCounter tRunCounter;
	m_pOpenATCRunStatus->GetPhaseLampClrRunCounter(tRunCounter);

	if (m_atSplitInfo[ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhaseNumber - 1].m_bySplitMode == PED_MODE)
	{
		//当前相位阶段是否完成
		if (tRunCounter.m_nPedLampClrTime[nRingIndex] >= ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPedPhaseStageRunTime)
		{
			if (nRingIndex == 0)
			{
				m_tFixTimeCtlInfo.m_wCycleRunTime += ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPedPhaseStageRunTime;            
			}
			m_tFixTimeCtlInfo.m_bIsChgPedStage[nRingIndex] = true;
		}
	}

	if (m_tFixTimeCtlInfo.m_bIsChgPedStage[nRingIndex])
	{
		if (m_atSplitInfo[ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhaseNumber - 1].m_bySplitMode == PED_MODE)
		{
			//机动车相位以行人相位为准
			m_tFixTimeCtlInfo.m_bIsChgStage[nRingIndex] = true;
		}
		char chcurPedStage = ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPedPhaseStage;
		PTPhaseCtlInfo pPhaseInfo = (PTPhaseCtlInfo)&(ptRingRunInfo->m_atPhaseInfo[nIndex]);   
		WORD wPedStageTime = 0;
		char chNextPedStage =  this->GetPedNextPhaseStageInfo(chcurPedStage,pPhaseInfo,wPedStageTime);
		
		ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPedPhaseStage = chNextPedStage;
		ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPedPhaseStageRunTime = wPedStageTime;
		if (m_atSplitInfo[ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhaseNumber - 1].m_bySplitMode == PED_MODE)
		{
			ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPhaseStageRunTime = wPedStageTime;
		}

		m_tFixTimeCtlInfo.m_bIsChgPedStage[nRingIndex] = false;

		if (chNextPedStage == C_CH_PHASESTAGE_GF)
		{
			GetGreenFalshCount(PED_CHA, (PTPhaseCtlInfo)&(ptRingRunInfo->m_atPhaseInfo[nIndex]), wPedStageTime);
		}

		//设置环内相位灯色，重置时间
		SetLampClrByRing(nRingIndex,nIndex,ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseStage,ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPedPhaseStage);
		tRunCounter.m_nPedLampClrTime[nRingIndex] = 0;
		tRunCounter.m_nPedLampClrStartCounter[nRingIndex] = tRunCounter.m_nCurCounter;    
		m_pOpenATCRunStatus->SetPhaseLampClrRunCounter(tRunCounter);
		m_bIsLampClrChg = true;
	} 

	if (m_atSplitInfo[ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhaseNumber - 1].m_bySplitMode == PED_MODE)
	{
		//判断是否需要延长绿灯时间
		if (ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPedPhaseStage == C_CH_PHASESTAGE_G)
		{
			ProcExtendGreen(nRingIndex,nIndex,tRunCounter.m_nLampClrTime[nRingIndex]);
		}

		//屏障前的最后一个降级相位绿灯时，其并发相位增加延长绿到最大绿
		if (m_bLastPhaseBeforeBarrier[nRingIndex][nIndex] && ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPedPhaseStage == C_CH_PHASESTAGE_G && ProcPhaseDetStatus(nRingIndex,nIndex))
		{
			//LastConcurrencyPhaseBeforeBarrierExtendGreen(nRingIndex,nIndex,tRunCounter.m_nLampClrTime[nRingIndex]);
		}
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
2020/11/24     V1.0 陈涵燕          创建函数 
====================================================================*/ 
void CLogicCtlActuatePedCrossStreet::ProcExtendGreen(int nRingIndex,int nPhaseIndex,int nCurRunTime)
{
	PTRingCtlInfo ptRingRunInfo = &(m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex]);
	PTPhase pPhaseInfo = &(ptRingRunInfo->m_atPhaseInfo[nPhaseIndex].m_tPhaseParam);

    char szInfo[256] = {0};
	int i = 0;
	int nMaxVehicleQueueLength = 0;
	int nMaxPedWaitLength = 0;
	int nMaxPedCrossStreetLength = 0;

    // 获取机动车排队长度
	m_pOpenATCRunStatus->GetVehicleQueueUpInfo(m_tVehicleQueueUpInfo);
	// 获取行人行人检测信息
	m_pOpenATCRunStatus->GetPedDetectInfo(m_tPedDetectInfo);

	for (i=0; i<MAX_VEHICLEDETECTOR_COUNT; i++)
	{
		if (m_tVehicleQueueUpInfo[i].m_byVehicleDetectorCallPhase == pPhaseInfo->m_byPhaseNumber)
		{
			if (nMaxVehicleQueueLength < m_tVehicleQueueUpInfo[i].m_nVehicleQueueUpLength)
			{
				nMaxVehicleQueueLength = m_tVehicleQueueUpInfo[i].m_nVehicleQueueUpLength;
			}
		}

		// 等待区域人数
		if (m_tPedDetectInfo[i].m_byVehicleDetectorCallPhase == pPhaseInfo->m_byPhaseNumber && m_nPedDetecetorType[i] == PED_DETECTOR_WAIT_AREA)
		{
			if (nMaxPedWaitLength < m_tPedDetectInfo[i].m_nPedCount)
			{
				nMaxPedWaitLength = m_tPedDetectInfo[i].m_nPedCount;
			}
		}

		// 过街区域人数
		if (m_tPedDetectInfo[i].m_byVehicleDetectorCallPhase == pPhaseInfo->m_byPhaseNumber && m_nPedDetecetorType[i] == PED_DETECTOR_CROSS_AREA)
		{
			if (nMaxPedCrossStreetLength < m_tPedDetectInfo[i].m_nPedCount)
			{
				nMaxPedCrossStreetLength = m_tPedDetectInfo[i].m_nPedCount;
			}
		}
	}

	// 获取行人过街区域
	if (nMaxVehicleQueueLength || nMaxPedWaitLength)
	{
		m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "###### PhaseNum[%d], nMaxVehicleQueueLength=%d, nMaxPedWaitLength=%d.", pPhaseInfo->m_byPhaseNumber, nMaxVehicleQueueLength, nMaxPedWaitLength);
	}
	else
	{ 
		return;
	}
        
	// 如果机动车排队长度超过设定阈值，或者机动车排队长度未超过设定阈值但是行人等待区域人数也未超过设定阈值
    if ((nMaxVehicleQueueLength >= MAX_VEH_QUEUE_LENGTH)
		|| nMaxPedWaitLength < MAX_PED_WAIT_LENGTH
		|| nMaxPedCrossStreetLength)
    {
		if (m_atSplitInfo[ptRingRunInfo->m_atPhaseInfo[nPhaseIndex].m_tPhaseParam.m_byPhaseNumber - 1].m_bySplitMode == PED_MODE)
		{
			//相位初始执行时间为相位绿信比
			ptRingRunInfo->m_atPhaseInfo[nPhaseIndex].m_wPedPhaseStageRunTime = nCurRunTime + pPhaseInfo->m_byPhasePassage;
			if (ptRingRunInfo->m_atPhaseInfo[nPhaseIndex].m_wPedPhaseStageRunTime > pPhaseInfo->m_wPhaseMaximum1)
			{
				ptRingRunInfo->m_atPhaseInfo[nPhaseIndex].m_wPedPhaseStageRunTime = pPhaseInfo->m_wPhaseMaximum1;
			}
			if (ptRingRunInfo->m_atPhaseInfo[nPhaseIndex].m_wPedPhaseStageRunTime < ptRingRunInfo->m_atPhaseInfo[nPhaseIndex].m_wPhaseGreenTime)
			{
				ptRingRunInfo->m_atPhaseInfo[nPhaseIndex].m_wPedPhaseStageRunTime = ptRingRunInfo->m_atPhaseInfo[nPhaseIndex].m_wPhaseGreenTime;
			}

			sprintf(szInfo, "ProcExtendGreen RingIndex:%d CurPhaseIndex:%d PhaseRunTime:%d MaxGreenTime:%d", nRingIndex, nPhaseIndex, ptRingRunInfo->m_atPhaseInfo[nPhaseIndex].m_wPhaseStageRunTime, pPhaseInfo->m_wPhaseMaximum1);
			m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, szInfo);

			for (int i = 0;i < m_tFixTimeCtlInfo.m_nRingCount;i++)
			{
				if (i != nRingIndex)
				{
					int nDstPhaseIndex = m_tFixTimeCtlInfo.m_nCurPhaseIndex[i];

					TPhaseLampClrRunCounter tRunCounter;
					m_pOpenATCRunStatus->GetPhaseLampClrRunCounter(tRunCounter);

					if (m_bLastPhaseBeforeBarrier[i][nDstPhaseIndex] && m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_chPedPhaseStage == C_CH_PHASESTAGE_G)
					{
						m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_wPedPhaseStageRunTime = tRunCounter.m_nLampClrTime[i] + pPhaseInfo->m_byPhasePassage;
						if (m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_wPedPhaseStageRunTime > m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_tPhaseParam.m_wPhaseMaximum1)
						{
							m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_wPedPhaseStageRunTime = m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_tPhaseParam.m_wPhaseMaximum1;
						}
						if (m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_wPedPhaseStageRunTime < m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_wPhaseGreenTime)
						{
							m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_wPedPhaseStageRunTime = m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_wPhaseGreenTime;
						}

						sprintf(szInfo, "ProcExtendGreen DstRingIndex:%d DstPhaseIndex:%d DstPhaseRunTime:%d MaxGreenTime:%d", i, nPhaseIndex, m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_wPedPhaseStageRunTime, m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_tPhaseParam.m_wPhaseMaximum1);
						m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, szInfo);

						m_nSplitTime[i][nDstPhaseIndex] = m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_wPedPhaseStageRunTime + ptRingRunInfo->m_atPhaseInfo[nDstPhaseIndex].m_wPhaseTime - ptRingRunInfo->m_atPhaseInfo[nDstPhaseIndex].m_wPhaseGreenTime;
					}
				}
			}

			m_nSplitTime[nRingIndex][nPhaseIndex] = ptRingRunInfo->m_atPhaseInfo[nPhaseIndex].m_wPedPhaseStageRunTime + ptRingRunInfo->m_atPhaseInfo[nPhaseIndex].m_wPhaseTime - ptRingRunInfo->m_atPhaseInfo[nPhaseIndex].m_wPhaseGreenTime;
		}
		else
		{
			//相位初始执行时间为相位绿信比
			ptRingRunInfo->m_atPhaseInfo[nPhaseIndex].m_wPhaseStageRunTime = nCurRunTime + pPhaseInfo->m_byPhasePassage;
			if (ptRingRunInfo->m_atPhaseInfo[nPhaseIndex].m_wPhaseStageRunTime > pPhaseInfo->m_wPhaseMaximum1)
			{
				ptRingRunInfo->m_atPhaseInfo[nPhaseIndex].m_wPhaseStageRunTime = pPhaseInfo->m_wPhaseMaximum1;
			}
			if (ptRingRunInfo->m_atPhaseInfo[nPhaseIndex].m_wPhaseStageRunTime < ptRingRunInfo->m_atPhaseInfo[nPhaseIndex].m_wPhaseGreenTime)
			{
				ptRingRunInfo->m_atPhaseInfo[nPhaseIndex].m_wPhaseStageRunTime = ptRingRunInfo->m_atPhaseInfo[nPhaseIndex].m_wPhaseGreenTime;
			}

			sprintf(szInfo, "ProcExtendGreen RingIndex:%d CurPhaseIndex:%d PhaseRunTime:%d MaxGreenTime:%d", nRingIndex, nPhaseIndex, ptRingRunInfo->m_atPhaseInfo[nPhaseIndex].m_wPhaseStageRunTime, pPhaseInfo->m_wPhaseMaximum1);
			m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, szInfo);

			for (int i = 0;i < m_tFixTimeCtlInfo.m_nRingCount;i++)
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
						if (m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_wPhaseStageRunTime < m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_wPhaseGreenTime)
						{
							m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_wPhaseStageRunTime = m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_wPhaseGreenTime;
						}

						sprintf(szInfo, "ProcExtendGreen DstRingIndex:%d DstPhaseIndex:%d DstPhaseRunTime:%d MaxGreenTime:%d", i, nPhaseIndex, m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_wPhaseStageRunTime, m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_tPhaseParam.m_wPhaseMaximum1);
						m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, szInfo);

						m_nSplitTime[i][nDstPhaseIndex] = m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_wPhaseStageRunTime + ptRingRunInfo->m_atPhaseInfo[nDstPhaseIndex].m_wPhaseTime - ptRingRunInfo->m_atPhaseInfo[nDstPhaseIndex].m_wPhaseGreenTime;
					}
				}
			}

			m_nSplitTime[nRingIndex][nPhaseIndex] = ptRingRunInfo->m_atPhaseInfo[nPhaseIndex].m_wPhaseStageRunTime + ptRingRunInfo->m_atPhaseInfo[nPhaseIndex].m_wPhaseTime - ptRingRunInfo->m_atPhaseInfo[nPhaseIndex].m_wPhaseGreenTime;
		}
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
2020/11/24     V1.0 陈涵燕          创建函数 
====================================================================*/ 
void CLogicCtlActuatePedCrossStreet::LastConcurrencyPhaseBeforeBarrierExtendGreen(int nRingIndex,int nPhaseIndex,int nCurRunTime)
{
	int   i = 0;

    PTRingCtlInfo ptRingRunInfo = &(m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex]);
	PTPhase pPhaseInfo = &(ptRingRunInfo->m_atPhaseInfo[nPhaseIndex].m_tPhaseParam);

	if ((ptRingRunInfo->m_atPhaseInfo[nPhaseIndex].m_wPhaseStageRunTime - nCurRunTime) >= pPhaseInfo->m_byPhasePassage)
	{
        for (i = 0;i < m_tFixTimeCtlInfo.m_nRingCount;i++)
        {
            if (i != nRingIndex)
            {
                int nDstPhaseIndex = m_tFixTimeCtlInfo.m_nCurPhaseIndex[i];

                TPhaseLampClrRunCounter tRunCounter;
                m_pOpenATCRunStatus->GetPhaseLampClrRunCounter(tRunCounter);

				if (m_atSplitInfo[ptRingRunInfo->m_atPhaseInfo[nPhaseIndex].m_tPhaseParam.m_byPhaseNumber - 1].m_bySplitMode == PED_MODE)
				{
					if (m_bLastPhaseBeforeBarrier[i][nDstPhaseIndex] && m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_chPedPhaseStage == C_CH_PHASESTAGE_G)
					{
						m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_wPedPhaseStageRunTime = tRunCounter.m_nLampClrTime[i] + pPhaseInfo->m_byPhasePassage;
						if (m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_wPedPhaseStageRunTime > m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_tPhaseParam.m_wPhaseMaximum1)
						{
							m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_wPedPhaseStageRunTime = m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_tPhaseParam.m_wPhaseMaximum1;
						}
						if (m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_wPedPhaseStageRunTime < m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_wPhaseGreenTime)
						{
							m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_wPedPhaseStageRunTime = m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_wPhaseGreenTime;
						}

						//m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "LastConcurrencyPhaseBeforeBarrierExtendGreen DstRingIndex:%d DstPhaseIndex:%d DstPhaseRunTime:%d MaxGreenTime:%d", i, nPhaseIndex, m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_wPhaseStageRunTime, m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_tPhaseParam.m_wPhaseMaximum1);

						m_nSplitTime[i][nDstPhaseIndex] = m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_wPedPhaseStageRunTime + ptRingRunInfo->m_atPhaseInfo[nDstPhaseIndex].m_wPhaseTime - ptRingRunInfo->m_atPhaseInfo[nDstPhaseIndex].m_wPhaseGreenTime;
					}
				}
				else
				{
					if (m_bLastPhaseBeforeBarrier[i][nDstPhaseIndex] && m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_chPhaseStage == C_CH_PHASESTAGE_G)
					{
						m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_wPhaseStageRunTime = tRunCounter.m_nLampClrTime[i] + pPhaseInfo->m_byPhasePassage;
						if (m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_wPhaseStageRunTime > m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_tPhaseParam.m_wPhaseMaximum1)
						{
							m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_wPhaseStageRunTime = m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_tPhaseParam.m_wPhaseMaximum1;
						}
						if (m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_wPhaseStageRunTime < m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_wPhaseGreenTime)
						{
							m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_wPhaseStageRunTime = m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_wPhaseGreenTime;
						}

						//m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "LastConcurrencyPhaseBeforeBarrierExtendGreen DstRingIndex:%d DstPhaseIndex:%d DstPhaseRunTime:%d MaxGreenTime:%d", i, nPhaseIndex, m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_wPhaseStageRunTime, m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_tPhaseParam.m_wPhaseMaximum1);

						m_nSplitTime[i][nDstPhaseIndex] = m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_wPhaseStageRunTime + ptRingRunInfo->m_atPhaseInfo[nDstPhaseIndex].m_wPhaseTime - ptRingRunInfo->m_atPhaseInfo[nDstPhaseIndex].m_wPhaseGreenTime;
					}
				}
			}
        }
	}
}

