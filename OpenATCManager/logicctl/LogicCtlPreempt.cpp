/*=====================================================================
模块名 ：手动控制方式接口模块
文件名 ：LogicCtPreempt.cpp
相关文件：LogicCtlPreempt.h,LogicCtlFixedTime.h
实现功能：手动控制方式实现
作者 ：李永萍
版权 ：<Copyright(c) 2019-2020 Suzhou Keda Technology Co.,Ltd. All right reserved.>
-----------------------------------------------------------------------
修改记录：
日 期           版本     修改人     走读人     修改记录
2019/9/14       V1.0     李永萍     王 五      创建模块
=====================================================================*/
#include "LogicCtlPreempt.h"
#include <string.h>

CLogicCtlPreempt::CLogicCtlPreempt()
{

}

CLogicCtlPreempt::~CLogicCtlPreempt()
{

}

/*==================================================================== 
函数名 ：Init 
功能 ：优化控制方式类资源初始化
算法实现 ： 
参数说明 ：pParameter，特征参数指针
           pRunStatus，全局运行状态类指针
		   pOpenATCLog，日志指针
           nPlanNo，指定的方案号,0表示使用时段对应的方案  
返回值说明：
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 李永萍          创建函数 
====================================================================*/ 
void CLogicCtlPreempt::Init(COpenATCParameter * pParameter, COpenATCRunStatus * pRunStatus, COpenATCLog * pOpenATCLog, int nPlanNo)
{
    m_pOpenATCParameter = pParameter;
    m_pOpenATCRunStatus = pRunStatus;
	m_pOpenATCLog       = pOpenATCLog;
    memset(&m_tFixTimeCtlInfo,0,sizeof(m_tFixTimeCtlInfo));

    TLogicCtlStatus tCtlStatus;
    m_pOpenATCRunStatus->GetLogicCtlStatus(tCtlStatus);
    tCtlStatus.m_nCurCtlMode = CTL_MODE_PREEMPT;
    m_pOpenATCRunStatus->SetLogicCtlStatus(tCtlStatus);

    InitChannelParam();
  
    memset(&m_tOldFixTimeCtlInfo, 0, sizeof(m_tOldFixTimeCtlInfo));
	memset(m_bKeepGreenChannelBeforeControlChannelFlag, false, sizeof(m_bKeepGreenChannelBeforeControlChannelFlag));
	memset(m_bShieldStatus, 0x00, sizeof(m_bShieldStatus));
    memset(m_bProhibitStatus, 0x00, sizeof(m_bProhibitStatus));
    memset(m_bOldShieldStatus, 0x00, sizeof(m_bOldShieldStatus));
    memset(m_bOldProhibitStatus, 0x00, sizeof(m_bOldProhibitStatus));
	
	//当前运行模式为优先控制
    m_nCurRunMode = CTL_MODE_PREEMPT;

	for (int i = 0;i < MAX_RING_COUNT;i++)
	{
		m_tPhasePulseStatus[i].m_nPhaseIndex = 0;
		m_tPhasePulseStatus[i].m_nPhaseNum = m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[m_tPhasePulseStatus[i].m_nPhaseIndex].m_tPhaseParam.m_byPhaseNumber;
		m_tPhasePulseStatus[i].m_bGreenPulseStatus = false;
		m_tPhasePulseStatus[i].m_bRedPulseStatus = false;
		m_tPhasePulseStatus[i].m_nGreenPulseSendStatus = SEND_INIT;
		m_tPhasePulseStatus[i].m_nRedPulseSendStatus = SEND_INIT;
	}

	m_bClearPulseFlag = false;
	memset(m_bSendRedPulse, 0, sizeof(m_bSendRedPulse));
	memset(m_bSendPedRedPulse, 0, sizeof(m_bSendPedRedPulse));

	m_nManualCurStatus = MANUAL_CONTROL_STATUS;
	m_nNextStageIndex = -1;
	m_bCycleEndFlag = false;
	m_nReturnAutoCtrlStageIndex = -1;
	m_bPreemptCtlCmdEndFlag = true;

	memset(m_bPhaseColorChgToYellowFlag, 0, sizeof(m_bPhaseColorChgToYellowFlag));
	memset(m_nStageRunTime, 0, sizeof(m_nStageRunTime));
	memset(m_nStageTimeForPhasePass, 0, sizeof(m_nStageTimeForPhasePass));

	memset(m_bPreemptCtlStageProcessFlag, 0, sizeof(m_bPreemptCtlStageProcessFlag));
	memset(&m_tPreemptCtlCmd, 0, sizeof(TPreemptCtlCmd));
	memset(&m_tNextPreemptCtlCmd, 0, sizeof(TPreemptCtlCmd));
	m_tPreemptCtlCmd.m_byPreemptType = PREEMPT_TYPE_DEFAULT;
	m_tNextPreemptCtlCmd.m_byPreemptType = PREEMPT_TYPE_DEFAULT;

	memset(m_bOverlapChgFlag, 0, sizeof(m_bOverlapChgFlag));
	memset(m_bPedOverlapChgFlag, 0, sizeof(m_bPedOverlapChgFlag));
	memset(m_bCurAndNextCmdInSameStage, 0, sizeof(m_bCurAndNextCmdInSameStage));
}

/*==================================================================== 
函数名 ：Run 
功能 ：优先控制方式主流程实现
算法实现 ： 
参数说明 ： 
返回值说明：
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 李永萍          创建函数 
====================================================================*/ 
void CLogicCtlPreempt::Run()
{
	bool bFlag = GetPreemptCtlCmdFromList();

	int  i = 0;
	for (i = 0;i < m_tFixTimeCtlInfo.m_nRingCount;i ++)
	{
		PreemptCtlOnePhaseRun(i);
	    PreemptCtlOnePedPhaseRun(i); 
	}

	PreemptCtlPhaseRun();

	ClearPulse();//清除脉冲

	SetOverlapCurPreemptPhaseLampClrChgFlag();

	if (m_bIsLampClrChg)
    {
		if (m_bIsLampClrChg)
		{
			if (m_nNextStageIndex < 0 || m_nNextStageIndex == m_tFixTimeCtlInfo.m_nCurStageIndex)
			{
				SetOverlapPhaseLampClr(m_tFixTimeCtlInfo.m_nCurStageIndex + 1);
			}
			else
			{
				SetOverlapPhaseLampClr(m_nNextStageIndex);
			}
		}
        
        //设置全局灯色状态
        TLampClrStatus tLampClrStatus;
        memset(&tLampClrStatus,0,sizeof(TLampClrStatus)); 
        m_pOpenATCRunStatus->GetLampClrStatus(tLampClrStatus);
        SetLampClr(tLampClrStatus);
        tLampClrStatus.m_bIsRefreshClr = true;
        m_pOpenATCRunStatus->SetLampClrStatus(tLampClrStatus);
        
		m_bIsLampClrChg = false;
    }

	ManualCycleChg(!bFlag);
}

/*==================================================================== 
函数名 ：PreemptCtlOnePhaseRun
功能 ：优化控制时单个机动车相位运行状态更新
算法实现 ： 
参数说明 ：nRingIndex：环索引
返回值说明：
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 李永萍          创建函数 
====================================================================*/ 
void CLogicCtlPreempt::PreemptCtlOnePhaseRun(int nRingIndex)
{
    PTRingCtlInfo ptRingRunInfo = &(m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex]);
    int nIndex = m_tFixTimeCtlInfo.m_nCurPhaseIndex[nRingIndex];
    PTPhaseCtlInfo pPhaseInfo = (PTPhaseCtlInfo)&(ptRingRunInfo->m_atPhaseInfo[nIndex]);
        
    TPhaseLampClrRunCounter tRunCounter;
    m_pOpenATCRunStatus->GetPhaseLampClrRunCounter(tRunCounter);

	int nGreenTime = ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPhaseGreenTime;
	int nNextStageIndex = m_tFixTimeCtlInfo.m_nCurStageIndex + 1;
	if (nNextStageIndex >= m_tRunStageInfo.m_nRunStageCount)
	{
		nNextStageIndex = 0;
	}

	if (m_nManualCurStatus == MANUAL_CONTROL_STATUS && !m_tPreemptCtlCmd.m_bPatternInterruptCmd && m_tPreemptCtlCmd.m_byPreemptSwitchFlag == 0)
	{
		if (m_tPreemptCtlCmd.m_byPreemptType == PREEMPT_TYPE_NORMAL)
		{
			if (/*nRingIndex == GetRingIndex(m_tPreemptCtlCmd.m_byPreemptPhaseID) && */m_tFixTimeCtlInfo.m_nCurStageIndex != m_tPreemptCtlCmd.m_byPreemptStageIndex)
			{
				//常规优先控制，在优先相位所在阶段之前的阶段的绿灯走最小绿
				nGreenTime = ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_wPhaseMinimumGreen;
			}

			if (IsHaveUrgentCtlCmd())//按照阶段顺序处理普通优先控制的时候，如果队列中还要紧急优先控制，则当紧急优先处理
			{
				nNextStageIndex = m_tPreemptCtlCmd.m_byPreemptStageIndex;
			}
		}
		else if (m_tPreemptCtlCmd.m_byPreemptType == PREEMPT_TYPE_URGENT)
		{
			nNextStageIndex = m_tPreemptCtlCmd.m_byPreemptStageIndex;
		}
	}
	else
	{
		nNextStageIndex = GetNextStageIndex();
	}

    //当前相位阶段是否完成
    if (ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseStage == C_CH_PHASESTAGE_G)
    {
        if (tRunCounter.m_nLampClrTime[nRingIndex] >= nGreenTime)
        {
			//跨阶段相位，要保持绿灯状态，运行到跨的最后一个阶段时才能切到绿闪
			//在控制过程中的相位绿灯没有到该相位的最后一个阶段都不切，其他都切
			if (m_nManualCurStatus == MANUAL_CONTROL_STATUS && m_tFixTimeCtlInfo.m_nCurStageIndex != nNextStageIndex &&
				m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[m_tFixTimeCtlInfo.m_nCurPhaseIndex[nRingIndex]].m_tPhaseParam.m_byPhaseNumber == m_tRunStageInfo.m_PhaseRunstageInfo[nNextStageIndex].m_nConcurrencyPhase[nRingIndex])
			{
				m_tFixTimeCtlInfo.m_bIsChgStage[nRingIndex] = false;

				if (m_tFixTimeCtlInfo.m_nCurStageIndex == m_tPreemptCtlCmd.m_byPreemptStageIndex &&
					m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[m_tFixTimeCtlInfo.m_nCurPhaseIndex[nRingIndex]].m_tPhaseParam.m_byPhaseNumber !=
					m_tPreemptCtlCmd.m_byPreemptPhaseID && m_bPreemptCtlStageProcessFlag[m_tPreemptCtlCmd.m_byPreemptStageIndex] && !m_bPreemptCtlCmdEndFlag)
				{
					m_tFixTimeCtlInfo.m_bIsChgStage[nRingIndex] = true;//当前阶段就是需要优先控制的阶段，优先控制的相位对应环的同阶段的相位跨阶段，跨阶段相位需要过渡
				}

				if (nNextStageIndex == m_tPreemptCtlCmd.m_byPreemptStageIndex &&
					m_tRunStageInfo.m_PhaseRunstageInfo[nNextStageIndex].m_nConcurrencyPhase[nRingIndex] != m_tPreemptCtlCmd.m_byPreemptPhaseID &&
					m_bPreemptCtlStageProcessFlag[m_tPreemptCtlCmd.m_byPreemptStageIndex] && !m_bPreemptCtlCmdEndFlag)
				{
					m_tFixTimeCtlInfo.m_bIsChgStage[nRingIndex] = true;//下个阶段就是需要优先控制的阶段，优先控制的相位对应环的同阶段的相位跨阶段，跨阶段相位需要过渡
				}
			}
			else
			{
				m_tFixTimeCtlInfo.m_bIsChgStage[nRingIndex] = true;
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "PreemptCtlOnePhaseRun RingIndex:%d CurPhaseIndex:%d GreenTime:%d", nRingIndex, nIndex, nGreenTime);
			}
		}
    }
    else
    {
		//非绿灯色只要大于运行时间就切换灯色
		if (ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseStage != C_CH_PHASESTAGE_U)
		{
			if (tRunCounter.m_nLampClrTime[nRingIndex] >= ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPhaseStageRunTime)
			{
				if (nRingIndex == 0)
				{
					m_tFixTimeCtlInfo.m_wCycleRunTime += tRunCounter.m_nLampClrTime[nRingIndex];      
				}

				m_tFixTimeCtlInfo.m_bIsChgStage[nRingIndex] = true;
			}
		}
		else
		{
			int nPhaseID = ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhaseNumber;
			if (nRingIndex == 0)
			{
				m_tFixTimeCtlInfo.m_wCycleRunTime += tRunCounter.m_nLampClrTime[nRingIndex];            
			}
			m_tFixTimeCtlInfo.m_bIsChgStage[nRingIndex] = true;
		}
    }

    if (m_tFixTimeCtlInfo.m_bIsChgStage[nRingIndex])
    {   
        char chcurStage = ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseStage;
        WORD wStageTime = 0;
        char chNextStage =  this->GetNextPhaseStageInfo(chcurStage,pPhaseInfo,wStageTime);
		ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseStage = chNextStage;

		if ((ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseMode == NEGLECT_MODE || ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseMode == SHIELD_MODE) &&
			 chNextStage == C_CH_PHASESTAGE_R)
		{
			wStageTime = SetNeglectPhaseRunTime(nRingIndex, true);
		}

		if (chNextStage != C_CH_PHASESTAGE_G)
		{
			ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPhaseStageRunTime = wStageTime;
			ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPedPhaseStageRunTime = wStageTime;
		}

        m_tFixTimeCtlInfo.m_bIsChgStage[nRingIndex] = false;

		if (chNextStage == C_CH_PHASESTAGE_GF)
        {
            GetGreenFalshCount(VEH_CHA, (PTPhaseCtlInfo)&(ptRingRunInfo->m_atPhaseInfo[nIndex]), wStageTime);
        }
		else if (chcurStage == C_CH_PHASESTAGE_GF)
		{
			m_bPhaseColorChgToYellowFlag[nRingIndex] = true;
		}
		else if (chcurStage == C_CH_PHASESTAGE_G && chNextStage == C_CH_PHASESTAGE_Y)
		{
			m_bPhaseColorChgToYellowFlag[nRingIndex] = true;
		}

		if (chcurStage != chNextStage)
		{
			m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "PreemptCtlOnePhaseRun RingIndex:%d CurPhaseIndex:%d CurStage:%c NextStage:%c StageTime:%d", nRingIndex, nIndex, chcurStage, chNextStage, wStageTime);

			m_nStageRunTime[nRingIndex] += tRunCounter.m_nLampClrTime[nRingIndex];

			//设置环内相位灯色，重置时间
			SetLampClrByRing(nRingIndex,nIndex,ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseStage,ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPedPhaseStage);
			tRunCounter.m_nLampClrTime[nRingIndex] = 0;
			tRunCounter.m_nLampClrStartCounter[nRingIndex] = tRunCounter.m_nCurCounter;    
			m_pOpenATCRunStatus->SetPhaseLampClrRunCounter(tRunCounter);
			m_bIsLampClrChg = true;
		}
    }

	if (m_bIsPreemptCtl)
	{
		SendCountDownPulse(nRingIndex, true);
	}
}

/*==================================================================== 
函数名 ：PreemptCtlOnePedPhaseRun
功能 ：优化控制时单个行人相位运行状态更新
算法实现 ： 
参数说明 ：nRingIndex：环索引
返回值说明：
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 李永萍          创建函数 
====================================================================*/ 
void CLogicCtlPreempt::PreemptCtlOnePedPhaseRun(int nRingIndex)
{
    PTRingCtlInfo ptRingRunInfo = &(m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex]);
        
    int nIndex = m_tFixTimeCtlInfo.m_nCurPhaseIndex[nRingIndex];
    TPhaseLampClrRunCounter tRunCounter;
    m_pOpenATCRunStatus->GetPhaseLampClrRunCounter(tRunCounter);

	char chcurPedStage = ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPedPhaseStage;
	int nNextStageIndex = m_tFixTimeCtlInfo.m_nCurStageIndex + 1;
	if (nNextStageIndex >= m_tRunStageInfo.m_nRunStageCount)
	{
		nNextStageIndex = 0;
	}
	
	//行人相位绿闪跟机动车相位绿闪一起结束
	/*if (chcurPedStage == C_CH_PHASESTAGE_GF)
	{
		if (m_bPhaseColorChgToYellowFlag[nRingIndex])
		{
			m_tFixTimeCtlInfo.m_bIsChgPedStage[nRingIndex] = true;
			m_bPhaseColorChgToYellowFlag[nRingIndex] = false;
		}
	}
	else */if (chcurPedStage == C_CH_PHASESTAGE_G)
	{
		if (tRunCounter.m_nPedLampClrTime[nRingIndex] >= ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPedPhaseGreenTime)
		{
			m_tFixTimeCtlInfo.m_bIsChgPedStage[nRingIndex] = true;

			if (m_nManualCurStatus == MANUAL_CONTROL_STATUS && !m_tPreemptCtlCmd.m_bPatternInterruptCmd && m_tPreemptCtlCmd.m_byPreemptSwitchFlag == 0)
			{
				//跨阶段相位，要保持绿灯状态，运行到跨的最后一个阶段时才能切到绿闪
				if (m_tPreemptCtlCmd.m_byPreemptType == PREEMPT_TYPE_NORMAL)
				{
					if (IsHaveUrgentCtlCmd())//安装阶段顺序处理普通优先控制的时候，如果队列中还要紧急优先控制，则当紧急优先处理
					{
						nNextStageIndex = m_tPreemptCtlCmd.m_byPreemptStageIndex;
					}
				}
				else if (m_tPreemptCtlCmd.m_byPreemptType == PREEMPT_TYPE_URGENT)
				{
					nNextStageIndex = m_tPreemptCtlCmd.m_byPreemptStageIndex;
				}
			}
			else
			{
				nNextStageIndex = GetNextStageIndex();
			}
				
			//在控制过程中的相位绿灯没有到该相位的最后一个阶段都不切，其他都切
			if (m_nManualCurStatus == MANUAL_CONTROL_STATUS && m_tFixTimeCtlInfo.m_nCurStageIndex != nNextStageIndex &&
				m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[m_tFixTimeCtlInfo.m_nCurPhaseIndex[nRingIndex]].m_tPhaseParam.m_byPhaseNumber == m_tRunStageInfo.m_PhaseRunstageInfo[nNextStageIndex].m_nConcurrencyPhase[nRingIndex])
			{
				m_tFixTimeCtlInfo.m_bIsChgPedStage[nRingIndex] = false;

				if (m_tFixTimeCtlInfo.m_nCurStageIndex == m_tPreemptCtlCmd.m_byPreemptStageIndex &&
					m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[m_tFixTimeCtlInfo.m_nCurPhaseIndex[nRingIndex]].m_tPhaseParam.m_byPhaseNumber !=
					m_tPreemptCtlCmd.m_byPreemptPhaseID && m_bPreemptCtlStageProcessFlag[m_tPreemptCtlCmd.m_byPreemptStageIndex] && !m_bPreemptCtlCmdEndFlag)
				{
					m_tFixTimeCtlInfo.m_bIsChgPedStage[nRingIndex] = true;//当前阶段就是需要优先控制的阶段，优先控制的相位对应环的同阶段的相位跨阶段，跨阶段相位需要过渡
				}

				if (nNextStageIndex == m_tPreemptCtlCmd.m_byPreemptStageIndex &&
					m_tRunStageInfo.m_PhaseRunstageInfo[nNextStageIndex].m_nConcurrencyPhase[nRingIndex] != m_tPreemptCtlCmd.m_byPreemptPhaseID &&
					m_bPreemptCtlStageProcessFlag[m_tPreemptCtlCmd.m_byPreemptStageIndex] && !m_bPreemptCtlCmdEndFlag)
				{
					m_tFixTimeCtlInfo.m_bIsChgPedStage[nRingIndex] = true;//下个阶段就是需要优先控制的阶段，优先控制的相位对应环的同阶段的相位跨阶段，跨阶段相位需要过渡
				}
			}
			else
			{
				m_tFixTimeCtlInfo.m_bIsChgPedStage[nRingIndex] = true;
			}
		}
	}
	else if (chcurPedStage == C_CH_PHASESTAGE_U)
	{
		m_tFixTimeCtlInfo.m_bIsChgPedStage[nRingIndex] = true;
	}
	else
	{
		if (tRunCounter.m_nPedLampClrTime[nRingIndex] >= ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPedPhaseStageRunTime)
		{
			m_tFixTimeCtlInfo.m_bIsChgPedStage[nRingIndex] = true;
		}
	}
	
    if (m_tFixTimeCtlInfo.m_bIsChgPedStage[nRingIndex])
    {
        PTPhaseCtlInfo pPhaseInfo = (PTPhaseCtlInfo)&(ptRingRunInfo->m_atPhaseInfo[nIndex]);
        WORD wPedStageTime = 0;
        char chNextPedStage =  this->GetPedNextPhaseStageInfo(chcurPedStage,pPhaseInfo,wPedStageTime);
		ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPedPhaseStage = chNextPedStage;
		ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPedPhaseStageRunTime = wPedStageTime;

		if ((ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseMode == NEGLECT_MODE || ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseMode == SHIELD_MODE) &&
			 chNextPedStage == C_CH_PHASESTAGE_R)
		{
			wPedStageTime = SetNeglectPhaseRunTime(nRingIndex, true);
		}

        m_tFixTimeCtlInfo.m_bIsChgPedStage[nRingIndex] = false;

        if (chNextPedStage == C_CH_PHASESTAGE_GF)
        {
            GetGreenFalshCount(PED_CHA, (PTPhaseCtlInfo)&(ptRingRunInfo->m_atPhaseInfo[nIndex]), wPedStageTime);
        }

		if (chcurPedStage != chNextPedStage)
		{
			m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "PreemptCtlOnePedPhaseRun RingIndex:%d CurPhaseIndex:%d PedCurStage:%c PedNextStage:%c PedStageTime:%d", nRingIndex, nIndex, chcurPedStage, chNextPedStage, wPedStageTime);

			//设置环内相位灯色，重置时间
			SetLampClrByRing(nRingIndex,nIndex,ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseStage,ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPedPhaseStage);
			tRunCounter.m_nPedLampClrTime[nRingIndex] = 0;
			tRunCounter.m_nPedLampClrStartCounter[nRingIndex] = tRunCounter.m_nCurCounter;    
			m_pOpenATCRunStatus->SetPhaseLampClrRunCounter(tRunCounter);
			m_bIsLampClrChg = true;
		}
    }

	if (m_bIsPreemptCtl)
	{
		SendPedCountDownPulse(nRingIndex, true);
	}
}

/*==================================================================== 
函数名 ：SetCtlDerivedParam
功能 ：设置手动控制时使用的参数内容和当前的运行状态
算法实现 ： 
参数说明 ：
        pParam：继承的参数指针.
返回值说明：
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 李永萍          创建函数 
====================================================================*/ 
void CLogicCtlPreempt::SetCtlDerivedParam(void * pParam,void * pLampClr,void * pStageTable,void * pChannelSplitMode)
{ 
	if (pParam != NULL)
	{
        memcpy(&m_tFixTimeCtlInfo,pParam,sizeof(TFixTimeCtlInfo));
		memcpy(&m_tOldFixTimeCtlInfo,&m_tFixTimeCtlInfo,sizeof(TFixTimeCtlInfo));
	}
    memcpy(m_achLampClr,pLampClr,sizeof(m_achLampClr));
    memcpy(&m_tRunStageInfo,pStageTable,sizeof(TRunStageInfo));
    memcpy(&m_nChannelSplitMode,pChannelSplitMode,sizeof(m_nChannelSplitMode));
}

/*==================================================================== 
函数名 ：ManualCycleChg
功能 ：周期是否运行结束状态判断
算法实现 ： 
参数说明 ：
返回值说明：
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 李永萍          创建函数 
====================================================================*/ 
void CLogicCtlPreempt::ManualCycleChg(bool bIsAutoCtl)
{
    bool bCycleChg = true;
    for (int i = 0;i < m_tFixTimeCtlInfo.m_nRingCount;i ++)
    {
        PTRingCtlInfo ptRingRunInfo = &(m_tFixTimeCtlInfo.m_atPhaseSeq[i]);
        int nIndex = m_tFixTimeCtlInfo.m_nCurPhaseIndex[i];
        if (ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseStage == C_CH_PHASESTAGE_F && 
            nIndex == ptRingRunInfo->m_nPhaseCount - 1)
        {
             //m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "ManualCycleChg RingIndex:%d CurPhaseIndex:%d CurStageIndex:%d", i, nIndex, m_tFixTimeCtlInfo.m_nCurStageIndex);   
        }
        else
        {
            bCycleChg = false;
            break;    
        }
    }
	//没有参数的通道控制，在控制状态时，不能进入自主
	if (m_tFixTimeCtlInfo.m_nRingCount == 0 && m_nManualCurStatus == MANUAL_CONTROL_STATUS)
	{
		bCycleChg = false;
	}

	if (bIsAutoCtl)
	{
		//当前周期是否完成
		if (bCycleChg || m_bCycleEndFlag)
		{
			if (m_nManualCurStatus == MANUAL_STAGE_TRANS)
			{
				SetGetParamFlag(true);
			}

			ReSetCtlStatus();
			m_tFixTimeCtlInfo.m_wCycleRunTime = 0;

			m_bClearPulseFlag = false;
			m_pOpenATCRunStatus->SetCycleChgStatus(true);
		}
	}
}

/*==================================================================== 
函数名 ：PreemptCtlPhaseRun
功能 ：优化控制时相位运行状态更新
算法实现 ： 
参数说明 ：无
返回值说明：无
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 李永萍          创建函数 
====================================================================*/ 
void CLogicCtlPreempt::PreemptCtlPhaseRun()
{
	int nStageIndex = 0;
	if (ManualSwitchStage(nStageIndex))//切换到指定阶段
	{
		if (m_tFixTimeCtlInfo.m_nCurStageIndex + 1 == m_tRunStageInfo.m_nRunStageCount && m_nManualCurStatus == MANUAL_STAGE_TRANS)
		{
			m_bCycleEndFlag = true;
			m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "Switch Stage Succeed, Set CycleEndFlag");
		}
		else
		{
			m_tFixTimeCtlInfo.m_nCurStageIndex = nStageIndex;
		}
		
		m_nNextStageIndex = m_tFixTimeCtlInfo.m_nCurStageIndex + 1;
		if (m_nNextStageIndex >= m_tRunStageInfo.m_nRunStageCount)
		{
			m_nNextStageIndex = 0;
		}
	}
}

/*==================================================================== 
函数名 ：ManualSwitchStage
功能 ：切到指定目标阶段后，初始化下一个阶段的相位
算法实现 ： 
参数说明 ：nStageIndex，目标阶段号
返回值说明：无
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 李永萍          创建函数 
====================================================================*/ 
bool  CLogicCtlPreempt::ManualSwitchStage(int & nStageIndex)
{
	if (!m_tPreemptCtlCmd.m_bPatternInterruptCmd && m_tPreemptCtlCmd.m_byPreemptSwitchFlag == 0)
	{
		if (m_tPreemptCtlCmd.m_byPreemptType == PREEMPT_TYPE_NORMAL)
		{
			if (!IsHaveUrgentCtlCmd())//按照阶段顺序处理普通优先控制的时候，如果队列中还要紧急优先控制，则当紧急优先处理
			{
				nStageIndex = m_tFixTimeCtlInfo.m_nCurStageIndex + 1;
			}
			else
			{
				nStageIndex = m_tPreemptCtlCmd.m_byPreemptStageIndex;
			}
		}
		else if (m_tPreemptCtlCmd.m_byPreemptType == PREEMPT_TYPE_URGENT)
		{
			nStageIndex = m_tPreemptCtlCmd.m_byPreemptStageIndex;
		}
	}
	else
	{
		nStageIndex = GetNextStageIndex();
	}

	if (nStageIndex >= m_tRunStageInfo.m_nRunStageCount)
	{
		nStageIndex = 0;
	}

	bool bRet = false;
	int  nClosePhaseCount = 0;
	TPhaseLampClrRunCounter tRunCounter;
	m_pOpenATCRunStatus->GetPhaseLampClrRunCounter(tRunCounter);

	bool bPhaseCloseFlag = false;
	int  nPhaseCloseCount = 0;

    int i = 0, j = 0;
    int nCount = 0;
	int nRepeatCnt = 0;
	int nCurStageIndex = 0;
	bool bChangePhaseFlag[MAX_RING_COUNT];
	memset(bChangePhaseFlag, 0, sizeof(bChangePhaseFlag));
    for (i = 0;i < m_tFixTimeCtlInfo.m_nRingCount;i++)
    {
        if (m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[m_tFixTimeCtlInfo.m_nCurPhaseIndex[i]].m_chPhaseStage == C_CH_PHASESTAGE_F)
        {
			nCount += 1;
			bChangePhaseFlag[i] = true;

        }
		else
		{
			int nTempIndex = nStageIndex;
			if (m_tPreemptCtlCmd.m_byPreemptType == PREEMPT_TYPE_URGENT && nCurStageIndex == nStageIndex)
			{
				nTempIndex = m_tFixTimeCtlInfo.m_nCurStageIndex + 1;
				if (nTempIndex >= m_tRunStageInfo.m_nRunStageCount)
				{
					nTempIndex = 0;
				}
			}

			nCurStageIndex = m_tFixTimeCtlInfo.m_nCurStageIndex;
			if (m_tRunStageInfo.m_PhaseRunstageInfo[nCurStageIndex].m_nConcurrencyPhase[i] == m_tRunStageInfo.m_PhaseRunstageInfo[nTempIndex].m_nConcurrencyPhase[i])
			{
				nRepeatCnt += 1;
				bChangePhaseFlag[i] = false;

				if (GetRingIndex(m_tPreemptCtlCmd.m_byPreemptPhaseID) == i)
				{
					if (m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[m_tFixTimeCtlInfo.m_nCurPhaseIndex[i]].m_chPhaseStage == C_CH_PHASESTAGE_G &&
						tRunCounter.m_nLampClrTime[i] < m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[m_tFixTimeCtlInfo.m_nCurPhaseIndex[i]].m_wPhaseGreenTime)
					{
						nRepeatCnt = 0;//跨阶段的相位必须过渡完持续时间才能切换到下一个阶段
					}

					if (nCurStageIndex == nTempIndex)
					{
						if (m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[m_tFixTimeCtlInfo.m_nCurPhaseIndex[i]].m_chPhaseStage != C_CH_PHASESTAGE_F)
						{
							if (m_tNextPreemptCtlCmd.m_byPreemptType == PREEMPT_TYPE_URGENT || m_tNextPreemptCtlCmd.m_byPreemptType == PREEMPT_TYPE_NORMAL)
							{
								if (m_tNextPreemptCtlCmd.m_byPreemptPhaseID == m_tPreemptCtlCmd.m_byPreemptPhaseID)
								{
									
								}
								else
								{
									nRepeatCnt = 0;//跨阶段的相位必须过渡结束才能切换到下一个阶段
								}
							}
							else
							{
								nRepeatCnt = 0;//跨阶段的相位必须过渡结束才能切换到下一个阶段
							}
						}
					}
				}
			}
			else
			{
				bChangePhaseFlag[i] = true;
			}
		}
    }

    if (nCount + nRepeatCnt == m_tFixTimeCtlInfo.m_nRingCount)
	{
		if (nRepeatCnt != 0 && !m_tNextPreemptCtlCmd.m_bPatternInterruptCmd && (m_tNextPreemptCtlCmd.m_byPreemptType == PREEMPT_TYPE_URGENT || m_tNextPreemptCtlCmd.m_byPreemptType == PREEMPT_TYPE_NORMAL))
		{
			if (m_tNextPreemptCtlCmd.m_byPreemptType == PREEMPT_TYPE_URGENT)
			{
				if (m_tNextPreemptCtlCmd.m_byPreemptStageIndex != (m_tPreemptCtlCmd.m_byPreemptStageIndex + 1) &&
					m_tNextPreemptCtlCmd.m_byPreemptStageIndex != m_tPreemptCtlCmd.m_byPreemptStageIndex)
				{
					return false;//跨阶段的相位,下一个优先控制阶段不是当前阶段的下一个阶段时也不是当前阶段时需要过渡结束才能切换
				}
			}
		}

		//阶段切换结束以后，先不要直接默认给下个阶段赋值，先从优先控制队列中获取控制命令判断以后再赋值
		SetPreemptCtlCmd(nStageIndex);

		memset(m_nStageRunTime, 0, sizeof(m_nStageRunTime));

        for (i = 0;i < m_tFixTimeCtlInfo.m_nRingCount;i++)
        {
			m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "ManualSwitchStage RingIndex:%d CurPhaseIndex:%d CurStageIndex:%d", i, m_tFixTimeCtlInfo.m_nCurPhaseIndex[i], m_tFixTimeCtlInfo.m_nCurStageIndex);

			m_nStageTimeForPhasePass[i] = 0;

			for (j = 0;j < m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_nPhaseCount;j++)
			{
				if (m_tRunStageInfo.m_PhaseRunstageInfo[nStageIndex].m_nConcurrencyPhase[i] == 
					m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[j].m_tPhaseParam.m_byPhaseNumber)
				{
					
					ResetNextStagePhaseGreenTime(i, j, nStageIndex, bChangePhaseFlag[i], m_tPreemptCtlCmd.m_wPreemptDuration);

					m_nStageTimeForPhasePass[i] = m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[j].m_wPhaseGreenTime + 
													m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[j].m_tPhaseParam.m_byGreenFlash +
													m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[j].m_tPhaseParam.m_byPhaseYellowChange +
													m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[j].m_tPhaseParam.m_byPhaseRedClear;
					bRet = true;
					m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "ManualSwitchStage RingIndex:%d NextPhaseIndex:%d PhaseTime:%d Stepward To Next Stage Success!", i, m_tFixTimeCtlInfo.m_nCurPhaseIndex[i], m_nStageTimeForPhasePass[i]);
					break;
				}
			}
        }

		ReSetNeglectPhasetBackUpTime();
    }
	
	return bRet;
}

/*==================================================================== 
函数名 ：TransCurStageToNextStage
功能 ：正在过渡的相位的绿灯达到切换的条件时，回其他非优先控制模式时，有设置过持续时长的，超过持续时长再切换灯色，否则按最小绿
算法实现 ： 
参数说明 ：无
返回值说明：true，可以切到下一个阶段，false：不能切到下一个阶段
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 李永萍          创建函数 
====================================================================*/
bool CLogicCtlPreempt::TransCurStageToNextStage()
{
	PTPhase pPhaseInfo;
	PTRingCtlInfo ptRingRunInfo;

	bool bFlag = false;
	int  nIndex = 0;
	int  nCnt = 0;
	int  nCurStageIndex	= m_tFixTimeCtlInfo.m_nCurStageIndex;
	
	int nStageIndexTarget = m_tFixTimeCtlInfo.m_nCurStageIndex + 1;
	if (nStageIndexTarget >= m_tRunStageInfo.m_nRunStageCount)
	{
		nStageIndexTarget = 0;
	}

	int  i = 0;
	for (i = 0; i < m_tFixTimeCtlInfo.m_nRingCount; i++)
	{
		nIndex = m_tFixTimeCtlInfo.m_nCurPhaseIndex[i];
		ptRingRunInfo = &(m_tFixTimeCtlInfo.m_atPhaseSeq[i]);
		pPhaseInfo = &(ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam);
		if (ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseStage == C_CH_PHASESTAGE_G)
		{
			//判断当前相位是否跨下一个阶段和当前阶段
			if (m_tRunStageInfo.m_PhaseRunstageInfo[nCurStageIndex].m_nConcurrencyPhase[i] == m_tRunStageInfo.m_PhaseRunstageInfo[nStageIndexTarget].m_nConcurrencyPhase[i])
			{
				nCnt++;
			}
			else
			{
				if (AdjustPhaseGreenTime(i, nIndex, nStageIndexTarget, MANUAL_MINI_GREEN))
				{
					nCnt++;
				}

				AdjustPedPhaseGreenTime(i, nIndex, nStageIndexTarget, MANUAL_MINI_GREEN);
			}
		}
		else
		{
			nCnt++;
		}
	}

	if (nCnt == m_tFixTimeCtlInfo.m_nRingCount)
	{
		bFlag = true;
	}

	return bFlag;
}

/*==================================================================== 
函数名 ：CheckIfStageHavePhaseInGreen
功能 ：判断当前相位是否处于绿灯
算法实现 ： 
参数说明 ：nGreenPhaseCount，绿灯相位数量
返回值说明：当前相位是否处于绿灯
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 李永萍          创建函数 
====================================================================*/
void CLogicCtlPreempt::CheckIfStageHavePhaseInGreen(int & nGreenPhaseCount)
{
	PTRingCtlInfo ptRingRunInfo;
	nGreenPhaseCount = 0;

	int i = 0;
	int nIndex = 0;
	int nPhaseID = 0;
	for (i = 0;i < m_tFixTimeCtlInfo.m_nRingCount;i++)
	{
		nIndex = m_tFixTimeCtlInfo.m_nCurPhaseIndex[i];
		ptRingRunInfo = &(m_tFixTimeCtlInfo.m_atPhaseSeq[i]);
		nPhaseID = ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhaseNumber;
		if (ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseStage == C_CH_PHASESTAGE_G)
		{
			nGreenPhaseCount++;
		}
	}
}

/*==================================================================== 
函数名 ：SendPedCountDownPulse
功能 ：发送机动车倒计时脉冲。
算法实现 ： 
参数说明 ：nRingIndex，环号, bNeedSendRedPulseFlag，需要发送红脉冲标志
返回值说明：无
 ----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 李永萍          创建函数 
====================================================================*/ 
void CLogicCtlPreempt::SendCountDownPulse(int nRingIndex, bool bNeedSendRedPulseFlag)
{
	PTRingCtlInfo ptRingRunInfo = &(m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex]);
	int nIndex = m_tFixTimeCtlInfo.m_nCurPhaseIndex[nRingIndex];
	int nCurPhaseID = ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhaseNumber;

	TPhaseLampClrRunCounter tRunCounter;
	m_pOpenATCRunStatus->GetPhaseLampClrRunCounter(tRunCounter);

	int nNextStageIndex = m_nNextStageIndex;
	if (nNextStageIndex >= m_tRunStageInfo.m_nRunStageCount)
	{
		nNextStageIndex = 0;
	}

	if (nNextStageIndex == m_tFixTimeCtlInfo.m_nCurStageIndex)
	{
		nNextStageIndex = m_tFixTimeCtlInfo.m_nCurStageIndex + 1;
	}

	if (nNextStageIndex >= m_tRunStageInfo.m_nRunStageCount)
	{
		nNextStageIndex = 0;
	}

	int i = 0;
	
	static bool bGreenPulse[MAX_RING_COUNT] = {false, false, false, false};
	static bool bRedPulse[MAX_RING_COUNT] = {false, false, false, false};

	static int  nGreenPulseIndex[MAX_RING_COUNT] = {-1, -1, -1, -1};
	static int  nRedPulseIndex[MAX_RING_COUNT] = {-1, -1, -1, -1};
	static int  nRedPulseNextIndex[MAX_RING_COUNT] = {-1, -1, -1, -1};

	static int  nGreenPulseNextStageIndex[MAX_RING_COUNT] = {-1, -1, -1, -1};
	static int  nRedPulseNextStageIndex[MAX_RING_COUNT] = {-1, -1, -1, -1};

	bool bFlag = true;
	if (m_nManualCurStatus == MANUAL_CONTROL_STATUS && m_tRunStageInfo.m_PhaseRunstageInfo[m_tFixTimeCtlInfo.m_nCurStageIndex].m_nConcurrencyPhase[nRingIndex] == m_tRunStageInfo.m_PhaseRunstageInfo[nNextStageIndex].m_nConcurrencyPhase[nRingIndex])
	{
		bFlag = false;
	}

	if (m_nManualCurStatus == MANUAL_STAGE_TRANS && m_tRunStageInfo.m_PhaseRunstageInfo[m_tFixTimeCtlInfo.m_nCurStageIndex].m_nConcurrencyPhase[nRingIndex] == m_tRunStageInfo.m_PhaseRunstageInfo[nNextStageIndex].m_nConcurrencyPhase[nRingIndex])
	{
		nNextStageIndex = nNextStageIndex + 1;

		if (nNextStageIndex >= m_tRunStageInfo.m_nRunStageCount)
		{
			nNextStageIndex = 0;
		}
	}

	int nNextIndex = 0;
	int nNextPhaseID = m_tRunStageInfo.m_PhaseRunstageInfo[nNextStageIndex].m_nConcurrencyPhase[nRingIndex];
	for (i = 0;i < ptRingRunInfo->m_nPhaseCount;i++)
	{
		if (ptRingRunInfo->m_atPhaseInfo[i].m_tPhaseParam.m_byPhaseNumber == nNextPhaseID)
		{
			nNextIndex = i;
			break;
		}
	}

	if (bFlag)
	{
		int nGreenTime = ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPhaseGreenTime;
		int nPhaseTime = m_nStageTimeForPhasePass[nRingIndex];

		if (ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseMode != NEGLECT_MODE && ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseMode != SHIELD_MODE &&
			ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseStage == C_CH_PHASESTAGE_G && tRunCounter.m_nLampClrTime[nRingIndex] == 
			(nGreenTime + ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byGreenFlash - ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byGreenPulse))
		{
			if (!bGreenPulse[nRingIndex])
			{
				bGreenPulse[nRingIndex] = true;
				SetGreenLampPulse(VEH_CHA, ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhaseNumber, true);

				m_tPhasePulseStatus[nRingIndex].m_nPhaseIndex = nIndex;
				m_tPhasePulseStatus[nRingIndex].m_nPhaseNum = ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhaseNumber;
				m_tPhasePulseStatus[nRingIndex].m_bGreenPulseStatus = true;

				nGreenPulseIndex[nRingIndex] = nIndex;
				nGreenPulseNextStageIndex[nRingIndex] = nNextStageIndex;
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "SendCountDownPulse GreenPulse On RingIndex:%d PhaseIndex:%d, nPhaseTime:%d.", nRingIndex, nIndex, nPhaseTime);

				if (m_tPhasePulseStatus[nRingIndex].m_nPhaseNum == m_tPreemptCtlCmd.m_byPreemptPhaseID)
				{
					m_tPhasePulseStatus[nRingIndex].m_bGreenPulseStatus = false;
				}
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

				nGreenPulseIndex[nRingIndex] = -1;
				nGreenPulseNextStageIndex[nRingIndex] = -1;
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "SendCountDownPulse GreenPulse Off RingIndex:%d PhaseIndex:%d, nPhaseTime:%d.", nRingIndex, nIndex, nPhaseTime);
			}
		}
	

		if (ptRingRunInfo->m_atPhaseInfo[nNextIndex].m_chPhaseMode != NEGLECT_MODE && ptRingRunInfo->m_atPhaseInfo[nNextIndex].m_chPhaseMode != SHIELD_MODE &&
			ptRingRunInfo->m_atPhaseInfo[nNextIndex].m_tPhaseParam.m_byRedPulse != 0 && GetPhaseRemainTime(nRingIndex, (PTPhaseCtlInfo)&(ptRingRunInfo->m_atPhaseInfo[nIndex]), tRunCounter, nPhaseTime) == ptRingRunInfo->m_atPhaseInfo[nNextIndex].m_tPhaseParam.m_byRedPulse)
		{
			bool bFlag = true;
			if (m_tPreemptCtlCmd.m_byPreemptPhaseID != 0 && nNextStageIndex == m_tPreemptCtlCmd.m_byPreemptStageIndex && nNextPhaseID != m_tPreemptCtlCmd.m_byPreemptPhaseID)
			{
				bFlag = false;//下一个阶段是优先控制阶段，下一个阶段的非优先控制相位不发红脉冲
			}

			if (!bRedPulse[nRingIndex] && bFlag)
			{
				if (m_bSendRedPulse[nRingIndex])
				{
					bRedPulse[nRingIndex] = true;
					SendRedPulse(VEH_CHA, ptRingRunInfo, nRingIndex, nIndex, nNextIndex, true);

					m_bSendRedPulse[nRingIndex] = false;

					nRedPulseIndex[nRingIndex] = nIndex;
					nRedPulseNextIndex[nRingIndex] = nNextIndex;
					nRedPulseNextStageIndex[nRingIndex] = nNextStageIndex;
					m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "SendCountDownPulse RedPulse On RingIndex:%d PhaseIndex:%d NextPhaseIndex:%d.", nRingIndex, nIndex, nNextIndex);

					if (m_tPhasePulseStatus[nRingIndex].m_nPhaseNum == m_tPreemptCtlCmd.m_byPreemptPhaseID)
					{
						m_tPhasePulseStatus[nRingIndex].m_bRedPulseStatus = false;
					}
				}
				else
				{
					m_bSendRedPulse[nRingIndex] = true;
				}
			}
		} 
		else
		{
			if (bRedPulse[nRingIndex])
			{
				bRedPulse[nRingIndex] = false;
			
				SendRedPulse(VEH_CHA, ptRingRunInfo, nRingIndex, nIndex, nNextIndex, false);

				nRedPulseIndex[nRingIndex] = -1;
				nRedPulseNextIndex[nRingIndex] = -1;
				nRedPulseNextStageIndex[nRingIndex] = -1;
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "SendCountDownPulse RedPulse Off RingIndex:%d PhaseIndex:%d NextPhaseIndex:%d.", nRingIndex, nIndex, nNextIndex);
			}
		}
	}
		
	if (bGreenPulse[nRingIndex] && nNextStageIndex != nGreenPulseNextStageIndex[nRingIndex])
	{
		bGreenPulse[nRingIndex] = false;
		SetGreenLampPulse(VEH_CHA, ptRingRunInfo->m_atPhaseInfo[nGreenPulseIndex[nRingIndex]].m_tPhaseParam.m_byPhaseNumber, false); 
		m_tPhasePulseStatus[nRingIndex].m_nPhaseIndex = nGreenPulseIndex[nRingIndex];
		m_tPhasePulseStatus[nRingIndex].m_nPhaseNum = ptRingRunInfo->m_atPhaseInfo[nGreenPulseIndex[nRingIndex]].m_tPhaseParam.m_byPhaseNumber;
		m_tPhasePulseStatus[nRingIndex].m_bGreenPulseStatus = false;	
		m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "SendCountDownPulse GreenPulse Off RingIndex:%d PhaseIndex:%d", nRingIndex, nGreenPulseIndex[nRingIndex]);
		nGreenPulseIndex[nRingIndex] = -1;
		nGreenPulseNextStageIndex[nRingIndex] = -1;
	}
	if (bRedPulse[nRingIndex] && nNextStageIndex != nRedPulseNextStageIndex[nRingIndex])
	{
		bRedPulse[nRingIndex] = false;
		SendRedPulse(VEH_CHA, ptRingRunInfo, nRingIndex, nRedPulseIndex[nRingIndex], nRedPulseNextIndex[nRingIndex], false);
		m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "SendCountDownPulse RedPulse Off RingIndex:%d PhaseIndex:%d NextPhaseIndex:%d", nRingIndex, nRedPulseIndex[nRingIndex], nRedPulseNextIndex[nRingIndex]);
		nRedPulseIndex[nRingIndex] = -1;
		nRedPulseNextIndex[nRingIndex] = -1;
	    nRedPulseNextStageIndex[nRingIndex] = -1;
	}

	if (nNextStageIndex == m_tPreemptCtlCmd.m_byPreemptStageIndex && m_tPhasePulseStatus[nRingIndex].m_bRedPulseStatus)
	{
		m_tPhasePulseStatus[nRingIndex].m_bRedPulseStatus = false;//锁定当前阶段，跟随相位不发红脉冲
	}

	//设置跟随相位的脉冲，手动控制过程中，第一个参数是手动按照阶段查找下一个阶段的相位，否则，第一个参数是非手动，按照相位的顺序
	if (m_nManualCurStatus == MANUAL_CONTROL_STATUS)
	{
		SetOverLapPulse(true, nNextStageIndex);
	}
	else
	{
		SetOverLapPulse(false, nNextStageIndex);
	}
}

/*==================================================================== 
函数名 ：SendPedCountDownPulse
功能 ：发送行人倒计时脉冲。
算法实现 ： 
参数说明 ：nRingIndex，环号，bNeedSendRedPulseFlag，需要发送红脉冲标志
返回值说明：无
 ----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 李永萍         创建函数 
====================================================================*/ 
void CLogicCtlPreempt::SendPedCountDownPulse(int nRingIndex, bool bNeedSendRedPulseFlag)
{
	PTRingCtlInfo ptRingRunInfo = &(m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex]);
	int nIndex = m_tFixTimeCtlInfo.m_nCurPhaseIndex[nRingIndex];
	int nCurPhaseID = ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhaseNumber;

	TPhaseLampClrRunCounter tRunCounter;
	m_pOpenATCRunStatus->GetPhaseLampClrRunCounter(tRunCounter);

	int nNextStageIndex = m_nNextStageIndex;
	if (nNextStageIndex >= m_tRunStageInfo.m_nRunStageCount)
	{
		nNextStageIndex = 0;
	}

	if (nNextStageIndex == m_tFixTimeCtlInfo.m_nCurStageIndex)
	{
		nNextStageIndex = m_tFixTimeCtlInfo.m_nCurStageIndex + 1;
	}

	int i = 0;

	static bool bGreenPulse[MAX_RING_COUNT] = {false, false, false, false};
	static bool bRedPulse[MAX_RING_COUNT] = {false, false, false, false};

	static int  nGreenPulseIndex[MAX_RING_COUNT] = {-1, -1, -1, -1};
	static int  nRedPulseIndex[MAX_RING_COUNT] = {-1, -1, -1, -1};
	static int  nRedPulseNextIndex[MAX_RING_COUNT] = {-1, -1, -1, -1};

	static int  nGreenPulseNextStageIndex[MAX_RING_COUNT] = {-1, -1, -1, -1};
	static int  nRedPulseNextStageIndex[MAX_RING_COUNT] = {-1, -1, -1, -1};

	bool bFlag = true;
	if (m_nManualCurStatus == MANUAL_CONTROL_STATUS && m_tRunStageInfo.m_PhaseRunstageInfo[m_tFixTimeCtlInfo.m_nCurStageIndex].m_nConcurrencyPhase[nRingIndex] == m_tRunStageInfo.m_PhaseRunstageInfo[nNextStageIndex].m_nConcurrencyPhase[nRingIndex])
	{
		bFlag = false;
	}

	if (m_nManualCurStatus == MANUAL_STAGE_TRANS && m_tRunStageInfo.m_PhaseRunstageInfo[m_tFixTimeCtlInfo.m_nCurStageIndex].m_nConcurrencyPhase[nRingIndex] == m_tRunStageInfo.m_PhaseRunstageInfo[nNextStageIndex].m_nConcurrencyPhase[nRingIndex])
	{
		nNextStageIndex = nNextStageIndex + 1;

		if (nNextStageIndex >= m_tRunStageInfo.m_nRunStageCount)
		{
			nNextStageIndex = 0;
		}
	}

	int nNextIndex = 0;
	int nNextPhaseID = m_tRunStageInfo.m_PhaseRunstageInfo[nNextStageIndex].m_nConcurrencyPhase[nRingIndex];
	for (i = 0;i < ptRingRunInfo->m_nPhaseCount;i++)
	{
		if (ptRingRunInfo->m_atPhaseInfo[i].m_tPhaseParam.m_byPhaseNumber == nNextPhaseID)
		{
			nNextIndex = i;
			break;
		}
	}

	if (bFlag && ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPhaseGreenTime != MAX_GREEN_TIME)
	{
		int nPedGreenTime = ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPedPhaseGreenTime;
		int nPhaseTime = m_nStageTimeForPhasePass[nRingIndex];
	
		if (ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseMode != NEGLECT_MODE && ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseMode != SHIELD_MODE &&
			ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPedPhaseStage == C_CH_PHASESTAGE_G && tRunCounter.m_nPedLampClrTime[nRingIndex] == 
			(nPedGreenTime + ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhasePedestrianClear - ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byGreenPulse))
		{
			if (!bGreenPulse[nRingIndex])
			{
				bGreenPulse[nRingIndex] = true;
				SetGreenLampPulse(PED_CHA, ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhaseNumber, true);

				nGreenPulseIndex[nRingIndex] = nIndex;
				nGreenPulseNextStageIndex[nRingIndex] = nNextStageIndex;
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "SendPedCountDownPulse GreenPulse On RingIndex:%d PhaseIndex:%d", nRingIndex, nIndex);

				if (m_tPhasePulseStatus[nRingIndex].m_nPhaseNum == m_tPreemptCtlCmd.m_byPreemptPhaseID)
				{
					m_tPhasePulseStatus[nRingIndex].m_bGreenPulseStatus = false;
				}
			}
		}
		else
		{
			if (bGreenPulse[nRingIndex])
			{
				bGreenPulse[nRingIndex] = false;
				SetGreenLampPulse(PED_CHA, ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhaseNumber, false);

				nGreenPulseIndex[nRingIndex] = -1;
				nGreenPulseNextStageIndex[nRingIndex] = -1;
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "SendPedCountDownPulse GreenPulse Off RingIndex:%d PhaseIndex:%d", nRingIndex, nIndex);
			}
		}

		if (ptRingRunInfo->m_atPhaseInfo[nNextIndex].m_chPhaseMode != NEGLECT_MODE && ptRingRunInfo->m_atPhaseInfo[nNextIndex].m_chPhaseMode != SHIELD_MODE &&
			ptRingRunInfo->m_atPhaseInfo[nNextIndex].m_tPhaseParam.m_byRedPulse != 0 && 
			GetPedPhaseRemainTime(nRingIndex, (PTPhaseCtlInfo)&(ptRingRunInfo->m_atPhaseInfo[nIndex]), tRunCounter, nPhaseTime) == ptRingRunInfo->m_atPhaseInfo[nNextIndex].m_tPhaseParam.m_byRedPulse)
		{
			bool bFlag = true;
			if (m_tPreemptCtlCmd.m_byPreemptPhaseID != 0 && nNextStageIndex == m_tPreemptCtlCmd.m_byPreemptStageIndex && nNextPhaseID != m_tPreemptCtlCmd.m_byPreemptPhaseID)
			{
				bFlag = false;//下一个阶段是优先控制阶段，下一个阶段的非优先控制相位不发红脉冲
			}

			if (!bRedPulse[nRingIndex] && bFlag)
			{
				if (m_bSendPedRedPulse[nRingIndex])
				{
					bRedPulse[nRingIndex] = true;
					SendRedPulse(PED_CHA, ptRingRunInfo, nRingIndex, nIndex, nNextIndex, true);

					m_bSendPedRedPulse[nRingIndex] = false;

					nRedPulseIndex[nRingIndex] = nIndex;
					nRedPulseNextIndex[nRingIndex] = nNextIndex;
					nRedPulseNextStageIndex[nRingIndex] = nNextStageIndex;
					m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "SendPedCountDownPulse RedPulse On RingIndex:%d PhaseIndex:%d NextPhaseIndex:%d", nRingIndex, nIndex, nNextIndex);

					if (m_tPhasePulseStatus[nRingIndex].m_nPhaseNum == m_tPreemptCtlCmd.m_byPreemptPhaseID)
					{
						m_tPhasePulseStatus[nRingIndex].m_bRedPulseStatus = false;
					}
				}
				else
				{
					m_bSendPedRedPulse[nRingIndex] = true;
				}
			}
		} 
		else
		{
			if (bRedPulse[nRingIndex])
			{
				bRedPulse[nRingIndex] = false;
			
				SendRedPulse(PED_CHA, ptRingRunInfo, nRingIndex, nIndex, nNextIndex, false);
				
				nRedPulseIndex[nRingIndex] = -1;
				nRedPulseNextIndex[nRingIndex] = -1;
				nRedPulseNextStageIndex[nRingIndex] = -1;
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "SendPedCountDownPulse RedPulse Off RingIndex:%d PhaseIndex:%d NextPhaseIndex:%d", nRingIndex, nIndex, nNextIndex);
			}
		}
	}
	
	if (bGreenPulse[nRingIndex] && nNextStageIndex != nGreenPulseNextStageIndex[nRingIndex])
	{
		bGreenPulse[nRingIndex] = false;
		SetGreenLampPulse(PED_CHA, ptRingRunInfo->m_atPhaseInfo[nGreenPulseIndex[nRingIndex]].m_tPhaseParam.m_byPhaseNumber, false);
		m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "SendPedCountDownPulse GreenPulse Off RingIndex:%d PhaseIndex:%d", nRingIndex, nIndex);
		nGreenPulseIndex[nRingIndex] = -1;
		nGreenPulseNextStageIndex[nRingIndex] = -1;
	}
	if (bRedPulse[nRingIndex] && nNextStageIndex != nRedPulseNextStageIndex[nRingIndex])
	{
		bRedPulse[nRingIndex] = false;
		SendRedPulse(PED_CHA, ptRingRunInfo, nRingIndex, nRedPulseIndex[nRingIndex], nRedPulseNextIndex[nRingIndex], false);
		m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "SendPedCountDownPulse RedPulse Off RingIndex:%d PhaseIndex:%d NextPhaseIndex:%d", nRingIndex, nRedPulseIndex[nRingIndex], nRedPulseNextIndex[nRingIndex]);
		nRedPulseIndex[nRingIndex] = -1;
		nRedPulseNextIndex[nRingIndex] = -1;
	    nRedPulseNextStageIndex[nRingIndex] = -1;
	}

	if (nNextStageIndex == m_tPreemptCtlCmd.m_byPreemptStageIndex && m_tPhasePulseStatus[nRingIndex].m_bRedPulseStatus)
	{
		m_tPhasePulseStatus[nRingIndex].m_bRedPulseStatus = false;//锁定当前阶段，跟随相位不发红脉冲
	}

	//设置跟随相位的脉冲，手动控制过程中，第一个参数是手动按照阶段查找下一个阶段的相位，否则，第一个参数是非手动，按照相位的顺序
	if (m_nManualCurStatus == MANUAL_CONTROL_STATUS)
	{
		SetOverLapPulse(true, nNextStageIndex);
	}
	else
	{
		SetOverLapPulse(false, nNextStageIndex);
	}
}

/*==================================================================== 
函数名 ：GetPhaseRemainTime
功能 ：获取机动车相位剩余时间
算法实现 ： 
参数说明 ：nRingIndex：环编号，pPhaseCtlInfo：相位信息， tRunCounter：灯色计数，wPhaseTime：相位时间
返回值说明：无
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 李永萍          创建函数 
====================================================================*/ 
int CLogicCtlPreempt::GetPhaseRemainTime(int nRingIndex, TPhaseCtlInfo * pPhaseCtlInfo, TPhaseLampClrRunCounter tRunCounter, DWORD wPhaseTime)
{
	int nRemainTime = 0;
    if (pPhaseCtlInfo->m_chPhaseStage == C_CH_PHASESTAGE_G)
    { 
        nRemainTime = wPhaseTime - tRunCounter.m_nLampClrTime[nRingIndex];
    }
    else if (pPhaseCtlInfo->m_chPhaseStage == C_CH_PHASESTAGE_GF)
    { 
        nRemainTime = wPhaseTime - pPhaseCtlInfo->m_wPhaseGreenTime - tRunCounter.m_nLampClrTime[nRingIndex];
    }
    else if (pPhaseCtlInfo->m_chPhaseStage == C_CH_PHASESTAGE_Y)
    { 
        nRemainTime = wPhaseTime - pPhaseCtlInfo->m_wPhaseGreenTime - pPhaseCtlInfo->m_tPhaseParam.m_byGreenFlash - tRunCounter.m_nLampClrTime[nRingIndex];
    }
    else if (pPhaseCtlInfo->m_chPhaseStage == C_CH_PHASESTAGE_R)
    { 
        nRemainTime = wPhaseTime - pPhaseCtlInfo->m_wPhaseGreenTime - pPhaseCtlInfo->m_tPhaseParam.m_byGreenFlash - pPhaseCtlInfo->m_tPhaseParam.m_byPhaseYellowChange - tRunCounter.m_nLampClrTime[nRingIndex];
    }
	else
	{
		nRemainTime = m_nStageTimeForPhasePass[nRingIndex] - (m_nStageRunTime[nRingIndex] + tRunCounter.m_nLampClrTime[nRingIndex]);	
	}

    return nRemainTime;
}

/*==================================================================== 
函数名 ：GetPedPhaseRemainTime
功能 ：获取行人相位剩余时间
算法实现 ： 
参数说明 ：nRingIndex：环编号，pPhaseCtlInfo：相位信息， tRunCounter：灯色计数，wPhaseTime：相位时间
返回值说明：无
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 李永萍          创建函数 
====================================================================*/ 
int CLogicCtlPreempt::GetPedPhaseRemainTime(int nRingIndex, TPhaseCtlInfo * pPhaseCtlInfo, TPhaseLampClrRunCounter tRunCounter, DWORD wPhaseTime)
{
	int nRemainTime = 0;
    if (pPhaseCtlInfo->m_chPedPhaseStage == C_CH_PHASESTAGE_G)
    { 
        nRemainTime = wPhaseTime - tRunCounter.m_nPedLampClrTime[nRingIndex];
    }
    else if (pPhaseCtlInfo->m_chPedPhaseStage == C_CH_PHASESTAGE_GF)
    { 
        nRemainTime = wPhaseTime - pPhaseCtlInfo->m_wPedPhaseGreenTime - tRunCounter.m_nPedLampClrTime[nRingIndex];
    }
    else if (pPhaseCtlInfo->m_chPedPhaseStage == C_CH_PHASESTAGE_R)
    { 
        nRemainTime = wPhaseTime - pPhaseCtlInfo->m_wPhaseGreenTime - pPhaseCtlInfo->m_tPhaseParam.m_byGreenFlash - tRunCounter.m_nPedLampClrTime[nRingIndex];
	}
	else
	{
		nRemainTime = m_nStageTimeForPhasePass[nRingIndex] - (m_nStageRunTime[nRingIndex] + tRunCounter.m_nLampClrTime[nRingIndex]);	
    }

    return nRemainTime;
}

/*==================================================================== 
函数名 ：SendRedPulse
功能 ：发送红脉冲
算法实现 ： 
参数说明 ：ptRingRunInfo：环信息，nRingIndex：环编号， nIndex：相位编号，nIndex：下一个相位编号，bFlag：发送标志
返回值说明：无
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 李永萍          创建函数 
====================================================================*/ 
void CLogicCtlPreempt::SendRedPulse(int nPhaseType, PTRingCtlInfo ptRingRunInfo, int nRingIndex, int nIndex, int nNextIndex, bool bFlag)
{
	if (nPhaseType == VEH_CHA)
	{
		SetRedLampPulse(VEH_CHA, ptRingRunInfo->m_atPhaseInfo[nNextIndex].m_tPhaseParam.m_byPhaseNumber, bFlag);  
	}
	else if (nPhaseType == PED_CHA)
	{
		SetRedLampPulse(PED_CHA, ptRingRunInfo->m_atPhaseInfo[nNextIndex].m_tPhaseParam.m_byPhaseNumber, bFlag); 
	}

	m_tPhasePulseStatus[nRingIndex].m_nPhaseIndex = nIndex;
	m_tPhasePulseStatus[nRingIndex].m_nPhaseNum = ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhaseNumber;
	m_tPhasePulseStatus[nRingIndex].m_bRedPulseStatus = bFlag;
}

/*==================================================================== 
函数名 ：ResetNextStagePhaseGreenTime
功能 ：计算下一个阶段的相位的绿灯时间
算法实现 ： 
参数说明 ：nRingIndex：环号，nPhaseIndex：相位号，nNextStageIndex：阶段号，bChangeStageFlag：切阶段和切相位标志，nDuration：持续时间
返回值说明：无
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 李永萍         创建函数 
====================================================================*/ 
void  CLogicCtlPreempt::ResetNextStagePhaseGreenTime(int nRingIndex, int nPhaseIndex, int nNextStageIndex, bool bChangePhaseFlag, int nDuration)
{
	int nStageTime = 0;
	int nSecNextStage = 0;
	int nSecNextStageTime = 0;
	
	InitNextStagePhase(nRingIndex, nPhaseIndex, nNextStageIndex, bChangePhaseFlag);

	TPhaseLampClrRunCounter tRunCounter;
	m_pOpenATCRunStatus->GetPhaseLampClrRunCounter(tRunCounter);

	int nPhaseGreenFlashTime	= m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_tPhaseParam.m_byGreenFlash;
	int nPedPhaseGreenFlashTime = m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_tPhaseParam.m_byPhasePedestrianClear;
	int nPhaseYellowTime		= m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_tPhaseParam.m_byPhaseYellowChange;
	int nPhaseRedTime			= m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_tPhaseParam.m_byPhaseRedClear;

	char chcurStage				= m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_chPhaseStage;
	char chPedcurStage			= m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_chPedPhaseStage;

	int nPhaseTransLightTime	= nPhaseGreenFlashTime + nPhaseYellowTime + nPhaseRedTime;
	int nPedPhaseTransLightTime = nPedPhaseGreenFlashTime + nPhaseRedTime;

	nStageTime = m_tRunStageInfo.m_PhaseRunstageInfo[nNextStageIndex].m_nStageEndTime - m_tRunStageInfo.m_PhaseRunstageInfo[nNextStageIndex].m_nStageStartTime;

	nSecNextStage = nNextStageIndex + 1;
	if (nSecNextStage >= m_tRunStageInfo.m_nRunStageCount)
	{
		nSecNextStage = 0;
	}
	nSecNextStageTime = m_tRunStageInfo.m_PhaseRunstageInfo[nSecNextStage].m_nStageEndTime - m_tRunStageInfo.m_PhaseRunstageInfo[nSecNextStage].m_nStageStartTime;

	//计算绿灯时间，分步进阶段和过渡阶段，步进阶段分准备切和已经切到两个状态
	if (m_nManualCurStatus == MANUAL_CONTROL_STATUS)																		//在步进控制中
	{
		if (m_nReturnAutoCtrlStageIndex == m_tFixTimeCtlInfo.m_nCurStageIndex && m_bPreemptCtlCmdEndFlag)		            //已经切到下一个阶段，接下来准备过渡回自主的第一个阶段的相位时间
		{	
			if (bChangePhaseFlag)																							//当前相位结束，切到下一个相位
			{
				if (m_tRunStageInfo.m_PhaseRunstageInfo[nNextStageIndex].m_bPhaseStartTransitInCurStage[nRingIndex])		//下个相位在下一个阶段过渡灯色
				{
					if (m_tRunStageInfo.m_PhaseRunstageInfo[nNextStageIndex].m_bDirectTransit[nRingIndex])
					{
						m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_wPhaseGreenTime = nStageTime - nPhaseTransLightTime;
					}
					else
					{
						m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_wPhaseGreenTime = nStageTime - m_tRunStageInfo.m_PhaseRunstageInfo[nNextStageIndex].m_nPhaseCurStageTransitTime[nRingIndex];
					}
				}
				else																									
				{
					if (m_tRunStageInfo.m_PhaseRunstageInfo[nSecNextStage].m_bDirectTransit[nRingIndex])
					{
						m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_wPhaseGreenTime = nStageTime + nSecNextStageTime - nPhaseTransLightTime;
					}
					else
					{
						m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_wPhaseGreenTime = nStageTime + nSecNextStageTime;
					}
				}
				
				if (m_tRunStageInfo.m_PhaseRunstageInfo[nNextStageIndex].m_bPedPhaseStartTransitInCurStage[nRingIndex])	//下个行人相位在下一个阶段过渡灯色
				{
					if (m_tRunStageInfo.m_PhaseRunstageInfo[nNextStageIndex].m_bDirectTransit[nRingIndex])
					{
						m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_wPedPhaseGreenTime = nStageTime - nPedPhaseTransLightTime;
					}
					else
					{
						m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_wPedPhaseGreenTime = nStageTime - m_tRunStageInfo.m_PhaseRunstageInfo[nNextStageIndex].m_nPedPhaseCurStageTransitTime[nRingIndex];
					}
				}
				else
				{
					if (m_tRunStageInfo.m_PhaseRunstageInfo[nSecNextStage].m_bDirectTransit[nRingIndex])
					{
						m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_wPedPhaseGreenTime = nStageTime + nSecNextStageTime - nPedPhaseTransLightTime;
					}
					else
					{
						m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_wPedPhaseGreenTime = nStageTime + nSecNextStageTime;
					}
				}
				
			}
			//相位跨下一个阶段
			else
			{
				if (chcurStage == C_CH_PHASESTAGE_G)
				{
					if (m_tRunStageInfo.m_PhaseRunstageInfo[nNextStageIndex].m_bPhaseStartTransitInCurStage[nRingIndex])
					{
						if (m_tRunStageInfo.m_PhaseRunstageInfo[nNextStageIndex].m_bDirectTransit[nRingIndex])
						{
							m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_wPhaseGreenTime = tRunCounter.m_nLampClrTime[nRingIndex] + nStageTime - nPhaseTransLightTime;
						}
						else
						{
							m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_wPhaseGreenTime = tRunCounter.m_nLampClrTime[nRingIndex] + nStageTime - m_tRunStageInfo.m_PhaseRunstageInfo[nNextStageIndex].m_nPhaseCurStageTransitTime[nRingIndex];
						}
					}
					else
					{
						if (m_tRunStageInfo.m_PhaseRunstageInfo[nSecNextStage].m_bDirectTransit[nRingIndex])
						{
							m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_wPhaseGreenTime = tRunCounter.m_nLampClrTime[nRingIndex] + nStageTime + nSecNextStageTime - nPhaseTransLightTime;
						}
						else
						{
							m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_wPhaseGreenTime = tRunCounter.m_nLampClrTime[nRingIndex] + nStageTime + nSecNextStageTime;
						}
					}

					if (chPedcurStage == C_CH_PHASESTAGE_G)
					{
						if (m_tRunStageInfo.m_PhaseRunstageInfo[nNextStageIndex].m_bPedPhaseStartTransitInCurStage[nRingIndex])
						{
							if (m_tRunStageInfo.m_PhaseRunstageInfo[nNextStageIndex].m_bDirectTransit[nRingIndex])
							{
								m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_wPedPhaseGreenTime = tRunCounter.m_nPedLampClrTime[nRingIndex] + nStageTime - nPedPhaseTransLightTime;
							}
							else
							{
								m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_wPedPhaseGreenTime = tRunCounter.m_nPedLampClrTime[nRingIndex] + nStageTime - m_tRunStageInfo.m_PhaseRunstageInfo[nNextStageIndex].m_nPedPhaseCurStageTransitTime[nRingIndex];
							}
						}
						else
						{
							if (m_tRunStageInfo.m_PhaseRunstageInfo[nSecNextStage].m_bDirectTransit[nRingIndex])
							{
								m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_wPedPhaseGreenTime = tRunCounter.m_nPedLampClrTime[nRingIndex] + nStageTime + nSecNextStageTime - nPhaseTransLightTime;
							}
							else
							{
								m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_wPedPhaseGreenTime = tRunCounter.m_nPedLampClrTime[nRingIndex] + nStageTime + nSecNextStageTime;
							}
						}
					}
				}
			}

			if (m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_chPhaseMode == NEGLECT_MODE || m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_chPhaseMode == SHIELD_MODE)
			{
				//忽略相位的关灯灯时间和关断相位的绿灯时间设置为0
				m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_wPhaseGreenTime = 0;
				m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_wPedPhaseGreenTime = 0;
			}

			m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "In ControlStage, Prepare To TransStage, RingIndex=%d PhaseIndex=%d StageIndex=%d PhaseGreenTime=%d PedPhaseGreenTime=%d.", 
														nRingIndex, nPhaseIndex,nNextStageIndex, m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_wPhaseGreenTime,
														m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_wPedPhaseGreenTime);
		}
		else//准备切下一个阶段，按照持续时间计算绿灯时间
		{
			if (nRingIndex != GetRingIndex(m_tPreemptCtlCmd.m_byPreemptPhaseID) && m_tPreemptCtlCmd.m_byPreemptSwitchFlag == 1 && !m_tPreemptCtlCmd.m_bIncludeConcurPhase)
			{
				//if (m_bCurAndNextCmdInSameStage[nRingIndex])
				if (bChangePhaseFlag)
				{
					m_bCurAndNextCmdInSameStage[nRingIndex] = false;
					m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_chPhaseStage = C_CH_PHASESTAGE_R;
					m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_chPedPhaseStage = C_CH_PHASESTAGE_R;
				}

				//m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_wPhaseGreenTime = m_tOldFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_wPhaseGreenTime;
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "In ControlStage, No Need ResetNextStagePhaseGreenTime, RingIndex=%d PhaseIndex=%d StageIndex=%d PhaseGreenTime=%d PedPhaseGreenTime=%d.", 
														nRingIndex, nPhaseIndex,nNextStageIndex, 0, 0);
				return;
			}

			if (bChangePhaseFlag)
			{
				// 当前阶段的相位跨到下一个阶段，不需要考虑最小绿，按照持续时长
				if (nDuration > m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_tPhaseParam.m_wPhaseMinimumGreen)
				{
					m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_wPhaseGreenTime = nDuration;
				}
				else
				{
					if (nDuration == 0)
					{
						m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_wPhaseGreenTime = MAX_GREEN_TIME;
					}
					else
					{
						m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_wPhaseGreenTime = m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_tPhaseParam.m_wPhaseMinimumGreen;
					}
				}
			}
			else
			{
				if (chcurStage == C_CH_PHASESTAGE_G)
				{
					if (nDuration == 0)
					{
						m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_wPhaseGreenTime = MAX_GREEN_TIME;
					}
					else
					{
						if (tRunCounter.m_nLampClrTime[nRingIndex] + nDuration > m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_tPhaseParam.m_wPhaseMinimumGreen)
						{
							m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_wPhaseGreenTime = tRunCounter.m_nLampClrTime[nRingIndex] + nDuration;
						}
						else
						{
							m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_wPhaseGreenTime = m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_tPhaseParam.m_wPhaseMinimumGreen;
						}
					}
				}
			}

			// 处理行人相位
			if (m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_wPhaseGreenTime == MAX_GREEN_TIME)
			{
				m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_wPedPhaseGreenTime = m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_tPhaseParam.m_wPhaseMinimumGreen + nPhaseGreenFlashTime - nPedPhaseGreenFlashTime;
			}
			else
			{
				m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_wPedPhaseGreenTime = m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_wPhaseGreenTime + nPhaseGreenFlashTime - nPedPhaseGreenFlashTime;
			}

			if (!m_tPreemptCtlCmd.m_bPatternInterruptCmd && m_tPreemptCtlCmd.m_byPreemptSwitchFlag == 0 && m_tPreemptCtlCmd.m_byPreemptType == PREEMPT_TYPE_NORMAL)
			{
				//常规优先控制，在优先相位所在阶段之前的阶段的绿灯走最小绿
				m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_wPhaseGreenTime = m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_tPhaseParam.m_wPhaseMinimumGreen;
				m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_wPedPhaseGreenTime = m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_tPhaseParam.m_wPhaseMinimumGreen;
			}

			if (m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_chPhaseMode == NEGLECT_MODE || m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_chPhaseMode == SHIELD_MODE)
			{
				//忽略相位的关灯灯时间和关断相位的绿灯时间设置为0
				m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_wPhaseGreenTime = 0;
				m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_wPedPhaseGreenTime = 0;
			}

			m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "In ControlStage, Prepare To Next Stage, RingIndex=%d PhaseIndex=%d StageIndex=%d PhaseGreenTime=%d PedPhaseGreenTime=%d.", 
														nRingIndex, nPhaseIndex,nNextStageIndex, m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_wPhaseGreenTime,
														m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_wPedPhaseGreenTime);
		}
	}
	else//在过渡阶段
	{
		if (bChangePhaseFlag)																							//当前相位结束，切到下一个相位
		{
			if (m_tRunStageInfo.m_PhaseRunstageInfo[nNextStageIndex].m_bPhaseStartTransitInCurStage[nRingIndex])		//下个相位在下一个阶段过渡灯色
			{
				if (m_tRunStageInfo.m_PhaseRunstageInfo[nNextStageIndex].m_bDirectTransit[nRingIndex])
				{
					m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_wPhaseGreenTime = nStageTime - nPhaseTransLightTime;
				}
				else
				{
					m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_wPhaseGreenTime = nStageTime - m_tRunStageInfo.m_PhaseRunstageInfo[nNextStageIndex].m_nPhaseCurStageTransitTime[nRingIndex];
				}
			}
			else																									
			{
				if (m_tRunStageInfo.m_PhaseRunstageInfo[nSecNextStage].m_bDirectTransit[nRingIndex])
				{
					m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_wPhaseGreenTime = nStageTime + nSecNextStageTime - nPhaseTransLightTime;
				}
				else
				{
					m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_wPhaseGreenTime = nStageTime + nSecNextStageTime;
				}
			}

			if (m_tRunStageInfo.m_PhaseRunstageInfo[nNextStageIndex].m_bPedPhaseStartTransitInCurStage[nRingIndex])	//下个行人相位在下一个阶段过渡灯色
			{
				if (m_tRunStageInfo.m_PhaseRunstageInfo[nNextStageIndex].m_bDirectTransit[nRingIndex])
				{
					m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_wPedPhaseGreenTime = nStageTime - nPedPhaseTransLightTime;
				}
				else
				{
					m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_wPedPhaseGreenTime = nStageTime - m_tRunStageInfo.m_PhaseRunstageInfo[nNextStageIndex].m_nPedPhaseCurStageTransitTime[nRingIndex];
				}
			}
			else
			{
				if (m_tRunStageInfo.m_PhaseRunstageInfo[nSecNextStage].m_bDirectTransit[nRingIndex])
				{
					m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_wPedPhaseGreenTime = nStageTime + nSecNextStageTime - nPedPhaseTransLightTime;
				}
				else
				{
					m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_wPedPhaseGreenTime = nStageTime + nSecNextStageTime;
				}
			}

		}
		//相位跨下一个阶段
		else
		{
			if (chcurStage == C_CH_PHASESTAGE_G)
			{
				if (m_tRunStageInfo.m_PhaseRunstageInfo[nNextStageIndex].m_bPhaseStartTransitInCurStage[nRingIndex])
				{
					if (m_tRunStageInfo.m_PhaseRunstageInfo[nNextStageIndex].m_bDirectTransit[nRingIndex])
					{
						m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_wPhaseGreenTime = tRunCounter.m_nLampClrTime[nRingIndex] + nStageTime - nPhaseTransLightTime;
					}
					else
					{
						m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_wPhaseGreenTime = tRunCounter.m_nLampClrTime[nRingIndex] + nStageTime - m_tRunStageInfo.m_PhaseRunstageInfo[nNextStageIndex].m_nPhaseCurStageTransitTime[nRingIndex];
					}
				}
				else
				{
					if (m_tRunStageInfo.m_PhaseRunstageInfo[nSecNextStage].m_bDirectTransit[nRingIndex])
					{
						m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_wPhaseGreenTime = tRunCounter.m_nLampClrTime[nRingIndex] + nStageTime + nSecNextStageTime - nPhaseTransLightTime;
					}
					else
					{
						m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_wPhaseGreenTime = tRunCounter.m_nLampClrTime[nRingIndex] + nStageTime + nSecNextStageTime;
					}
				}

				if (chPedcurStage == C_CH_PHASESTAGE_G)
				{
					if (m_tRunStageInfo.m_PhaseRunstageInfo[nNextStageIndex].m_bPedPhaseStartTransitInCurStage[nRingIndex])
					{
						if (m_tRunStageInfo.m_PhaseRunstageInfo[nNextStageIndex].m_bDirectTransit[nRingIndex])
						{
							m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_wPedPhaseGreenTime = tRunCounter.m_nPedLampClrTime[nRingIndex] + nStageTime - nPedPhaseTransLightTime;
						}
						else
						{
							m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_wPedPhaseGreenTime = tRunCounter.m_nPedLampClrTime[nRingIndex] + nStageTime - m_tRunStageInfo.m_PhaseRunstageInfo[nNextStageIndex].m_nPedPhaseCurStageTransitTime[nRingIndex];
						}
					}
					else
					{
						if (m_tRunStageInfo.m_PhaseRunstageInfo[nSecNextStage].m_bDirectTransit[nRingIndex])
						{
							m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_wPedPhaseGreenTime = tRunCounter.m_nPedLampClrTime[nRingIndex] + nStageTime + nSecNextStageTime - nPhaseTransLightTime;
						}
						else
						{
							m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_wPedPhaseGreenTime = tRunCounter.m_nPedLampClrTime[nRingIndex] + nStageTime + nSecNextStageTime;
						}
					}
				}
			}
		}

		if (m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_wPhaseGreenTime < m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_tPhaseParam.m_wPhaseMinimumGreen)
		{
			m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_wPhaseGreenTime = m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_tPhaseParam.m_wPhaseMinimumGreen;
		}
		else if (m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_wPhaseGreenTime > m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_tPhaseParam.m_wPhaseMaximum1)
		{
			m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_wPhaseGreenTime = m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_tPhaseParam.m_wPhaseMaximum1;
		}
		
		m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_wPedPhaseGreenTime = m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_wPhaseGreenTime +
			m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_tPhaseParam.m_byGreenFlash - m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_tPhaseParam.m_byPhasePedestrianClear;
		
		if (m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_chPhaseMode == NEGLECT_MODE || m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_chPhaseMode == SHIELD_MODE)
		{
			//忽略相位的关灯灯时间和关断相位的绿灯时间设置为0
			m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_wPhaseGreenTime = 0;
			m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_wPedPhaseGreenTime = 0;
		}

		m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "In TransStage, RingIndex=%d PhaseIndex=%d StageIndex=%d PhaseGreenTime=%d PedPhaseGreenTime=%d.", 
													nRingIndex, nPhaseIndex,nNextStageIndex, m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_wPhaseGreenTime,
													m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_wPedPhaseGreenTime);
	}
}

/*==================================================================== 
函数名 ：RecalculteStageTimeByDelayTimeAndDuration
功能 ：根据延迟时间和持续时间重新计算优化控制时的绿灯时间
算法实现 ： 
参数说明 ：无
返回值说明：无
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 李永萍          创建函数 
====================================================================*/ 
void CLogicCtlPreempt::RecalculteStageTimeByDelayTimeAndDuration()
{
	TPhaseLampClrRunCounter tRunCounter;
	m_pOpenATCRunStatus->GetPhaseLampClrRunCounter(tRunCounter);

	int nRingIndex = GetRingIndex(m_tPreemptCtlCmd.m_byPreemptPhaseID);
	int nStageIndexTarget = m_tPreemptCtlCmd.m_byPreemptStageIndex;

	int i = 0;
	int nPhaseID = 0;
	int nGreenTime = 0;
	
	for (i = 0;i < m_tFixTimeCtlInfo.m_nRingCount;i++)
	{
		PTRingCtlInfo ptRingRunInfo = &(m_tFixTimeCtlInfo.m_atPhaseSeq[i]);
		int nIndex = m_tFixTimeCtlInfo.m_nCurPhaseIndex[i];
		PTPhaseCtlInfo pPhaseInfo = (PTPhaseCtlInfo)&(ptRingRunInfo->m_atPhaseInfo[nIndex]);
		nPhaseID = pPhaseInfo->m_tPhaseParam.m_byPhaseNumber;

		if (/*i == nRingIndex && */ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseStage == C_CH_PHASESTAGE_G)
		{
			if (nPhaseID == m_tPreemptCtlCmd.m_byPreemptPhaseID && nStageIndexTarget != m_tFixTimeCtlInfo.m_nCurStageIndex)
			{
				m_tPreemptCtlCmd.m_byPreemptStageIndex = m_tFixTimeCtlInfo.m_nCurStageIndex;
				nStageIndexTarget = m_tPreemptCtlCmd.m_byPreemptStageIndex; 
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "RecalculteStageTimeByDelayTimeAndDuration RingIndex:%d Modify NextStageIndex:%d.", i, nStageIndexTarget);
			}

			if (tRunCounter.m_nLampClrTime[i] < pPhaseInfo->m_tPhaseParam.m_wPhaseMinimumGreen)
			{
				nGreenTime = pPhaseInfo->m_tPhaseParam.m_wPhaseMinimumGreen + m_tPreemptCtlCmd.m_wPreemptDelay;
			}
			else
			{
				nGreenTime = tRunCounter.m_nLampClrTime[i] + m_tPreemptCtlCmd.m_wPreemptDelay;
			}

			if (i == nRingIndex && m_tFixTimeCtlInfo.m_nCurStageIndex == nStageIndexTarget)
			{
				nGreenTime = nGreenTime + m_tPreemptCtlCmd.m_wPreemptDuration;//当前阶段就是优先控制的目标阶段，加上持续时间
			}

			pPhaseInfo->m_wPhaseTime = nGreenTime + 
				pPhaseInfo->m_tPhaseParam.m_byGreenFlash + pPhaseInfo->m_tPhaseParam.m_byPhaseYellowChange + 
				pPhaseInfo->m_tPhaseParam.m_byPhaseRedClear;

			pPhaseInfo->m_wPhaseGreenTime = nGreenTime;				

			if (ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPedPhaseStage == C_CH_PHASESTAGE_G)
			{
				pPhaseInfo->m_wPedPhaseGreenTime = pPhaseInfo->m_wPhaseTime - pPhaseInfo->m_tPhaseParam.m_byPhasePedestrianClear
													- pPhaseInfo->m_tPhaseParam.m_byPhaseYellowChange 
													- pPhaseInfo->m_tPhaseParam.m_byPhaseRedClear;

				AdjustPedPhaseGreenTime(i, nIndex, nStageIndexTarget, MANUAL_PHASE_RUN_GREEN);
			}
				

			AdjustPhaseGreenTime(i, nIndex, nStageIndexTarget, MANUAL_PHASE_RUN_GREEN);
			
			m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "RecalculteStageTimeByDelayTimeAndDuration RingIndex:%d GreenTime:%d, PedPhaseGreenTime:%d.", i, pPhaseInfo->m_wPhaseGreenTime, pPhaseInfo->m_wPedPhaseGreenTime);
				
			if (i == nRingIndex && m_tFixTimeCtlInfo.m_nCurStageIndex == nStageIndexTarget)
			{
				if (m_tPreemptCtlCmd.m_wPreemptDuration == 0)
				{
					//机动车卡住不动给最大值，行人给最小绿
					pPhaseInfo->m_wPhaseGreenTime = MAX_GREEN_TIME;
					AdjustPedPhaseGreenTime(i, nIndex, nStageIndexTarget, MANUAL_MINI_GREEN);
					m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "RecalculteStageTimeByDelayTimeAndDuration nStageIndexTarget:%d MaxGreenTime:%d, PedPhaseGreenTime:%d.", nStageIndexTarget, pPhaseInfo->m_wPhaseGreenTime, pPhaseInfo->m_wPedPhaseGreenTime);
				}
			}
		}
		else
		{
			if (ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseMode == NEGLECT_MODE || ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseMode == SHIELD_MODE)
			{
				//忽略相位的关灯灯时间和关断相位的红灯时间设置为0
				//ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPhaseStageRunTime = 0;
				//ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPedPhaseStageRunTime = 0;
			}
		}

		m_nStageTimeForPhasePass[i] = pPhaseInfo->m_wPhaseGreenTime + pPhaseInfo->m_tPhaseParam.m_byGreenFlash + pPhaseInfo->m_tPhaseParam.m_byPhaseYellowChange + pPhaseInfo->m_tPhaseParam.m_byPhaseRedClear;
	}
	
	ReSetNeglectPhaseStageRunTime();
	ReSetNeglectPhasetBackUpTime();
	
	if (m_tPreemptCtlCmd.m_byPreemptType == PREEMPT_TYPE_URGENT)
	{
		m_nNextStageIndex = nStageIndexTarget;
	}
	else
	{
		m_nNextStageIndex = m_tFixTimeCtlInfo.m_nCurStageIndex + 1;
	}

	if (m_nNextStageIndex >= m_tRunStageInfo.m_nRunStageCount)
	{
		m_nNextStageIndex = 0;
	}

	if (m_tPreemptCtlCmd.m_wPreemptDuration > 0)
	{
		m_nReturnAutoCtrlStageIndex = nStageIndexTarget;
	}
	else
	{
		m_nReturnAutoCtrlStageIndex = -1;
	}

}

/*==================================================================== 
函数名 ：InitNextStagePhase
功能 ：初始化下一个阶段的相位
算法实现 ： 
参数说明 ：nRingIndex：环号,nPhaseIndex：相位号，nNextStageIndex：下一个阶段编号，bChangeStageFlag，同时切阶段和切相位的标志
返回值说明：无
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 李永萍          创建函数 
====================================================================*/ 
void CLogicCtlPreempt::InitNextStagePhase(int nRingIndex, int nPhaseIndex, int nNextStageIndex, bool bChangePhaseFlag)
{
	if (bChangePhaseFlag)//切阶段，切相位
	{
		m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[m_tFixTimeCtlInfo.m_nCurPhaseIndex[nRingIndex]].m_chPhaseStage = C_CH_PHASESTAGE_F;   
		m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[m_tFixTimeCtlInfo.m_nCurPhaseIndex[nRingIndex]].m_chPedPhaseStage = C_CH_PHASESTAGE_F; 

		m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_chPhaseStage = C_CH_PHASESTAGE_U;   
		m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_chPedPhaseStage = C_CH_PHASESTAGE_U; 

		if (m_tFixTimeCtlInfo.m_nCurStageIndex != nNextStageIndex)
		{
			m_tFixTimeCtlInfo.m_nCurPhaseIndex[nRingIndex] = nPhaseIndex;  
		}

		m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_wPhaseStageRunTime = 0;
		m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_wPedPhaseStageRunTime = 0;
	}
}

/*==================================================================== 
函数名 ：CreatePreemptCtlCmdReturnToSelf
功能 ：生成回到自主的优先控制命令
算法实现 ： 
参数说明 ：无
返回值说明：无
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 李永萍          创建函数 
====================================================================*/ 
void CLogicCtlPreempt::CreatePreemptCtlCmdReturnToSelf()
{
	TPreemptCtlCmd  tPreemptCtlCmdReturnToSelf;
	memset(&tPreemptCtlCmdReturnToSelf,0,sizeof(tPreemptCtlCmdReturnToSelf));
	tPreemptCtlCmdReturnToSelf.m_nCmdSource = CTL_SOURCE_PREEMPT;
	tPreemptCtlCmdReturnToSelf.m_nCurCtlSource = CTL_SOURCE_PREEMPT;
	memcpy(tPreemptCtlCmdReturnToSelf.m_szPeerIp, "127.0.0.1", strlen("127.0.0.1"));
	tPreemptCtlCmdReturnToSelf.m_bNewCmd = true;
	tPreemptCtlCmdReturnToSelf.m_byPreemptType = PREEMPT_TYPE_DEFAULT;
	tPreemptCtlCmdReturnToSelf.m_bPatternInterruptCmd = true;
	tPreemptCtlCmdReturnToSelf.m_tPatternInterruptCmd.m_nControlMode = CTL_MODE_SELFCTL;
	tPreemptCtlCmdReturnToSelf.m_tPatternInterruptCmd.m_nPatternNo = 0;
	//m_pOpenATCRunStatus->SetPreemptCtlCmd(tPreemptCtlCmdReturnToSelf);
	m_pOpenATCRunStatus->SePreemptCtlCmdList(tPreemptCtlCmdReturnToSelf);
}

/*==================================================================== 
函数名 ：SetTransStatus
功能 ：设置过渡状态
算法实现 ： 
参数说明 ：无
返回值说明：无
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 李永萍          创建函数 
====================================================================*/ 
void CLogicCtlPreempt::SetTransStatus()
{
	int	nGreenPhaseCount = 0;
	CheckIfStageHavePhaseInGreen(nGreenPhaseCount);
	if (nGreenPhaseCount > 0)
	{
		TransCurStageToNextStage();
	}
	
	ReSetNeglectPhaseStageRunTime();
	
	m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "PreemptCtlToNoPreemptCtl, CheckIfNeedToStepForwardToNextStage, Set Trans Status");	
}

/*==================================================================== 
函数名 ：ClearPulse
功能 ：清除绿脉冲，红脉冲
算法实现 ： 
参数说明 ：无
返回值说明：无
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 李永萍          创建函数 
====================================================================*/
void  CLogicCtlPreempt::ClearPulse()
{
	if (!m_bClearPulseFlag)
	{
		for (int i = 0;i < m_tFixTimeCtlInfo.m_nRingCount;i++)
		{
			PTRingCtlInfo ptRingRunInfo = &(m_tFixTimeCtlInfo.m_atPhaseSeq[i]);
			for (int j = 0;j < ptRingRunInfo->m_nPhaseCount;j++)
			{
				 PTRingCtlInfo ptRingRunInfo = &(m_tFixTimeCtlInfo.m_atPhaseSeq[i]);

				 int nNextIndex = m_tFixTimeCtlInfo.m_nCurPhaseIndex[i] + 1;
				 if (nNextIndex == ptRingRunInfo->m_nPhaseCount)
				 {
					nNextIndex = 0;
				 }

				 SetGreenLampPulse(VEH_CHA, ptRingRunInfo->m_atPhaseInfo[j].m_tPhaseParam.m_byPhaseNumber, false);
				 SetRedLampPulse(VEH_CHA, ptRingRunInfo->m_atPhaseInfo[nNextIndex].m_tPhaseParam.m_byPhaseNumber, false); 

				 SetGreenLampPulse(PED_CHA, ptRingRunInfo->m_atPhaseInfo[j].m_tPhaseParam.m_byPhaseNumber, false);
				 SetRedLampPulse(PED_CHA, ptRingRunInfo->m_atPhaseInfo[nNextIndex].m_tPhaseParam.m_byPhaseNumber, false); 

				 if (m_nNextStageIndex < 0 || m_nNextStageIndex == m_tFixTimeCtlInfo.m_nCurStageIndex)
				 {
					 SetOverLapGreenPulse(i, j, true, m_tFixTimeCtlInfo.m_nCurStageIndex + 1, false);
					 SetOverLapRedPulse(i, j, true, m_tFixTimeCtlInfo.m_nCurStageIndex + 1, false);
				 }
				 else
				 {
					 SetOverLapGreenPulse(i, j, true, m_nNextStageIndex, false);
					 SetOverLapRedPulse(i, j, true, m_nNextStageIndex, false);
				 }
			}
		}
	    m_bClearPulseFlag = true;
	}
}

/*==================================================================== 
函数名 ：AdjustPhaseGreenTime
功能 ：修改机动车相位绿灯时间，保证闪灯从整秒开始
算法实现 ： 
参数说明 ：nRingIndex，环编号 
           nIndex，相位编号 
		   nStageIndexTarget，目标阶段编号 
		   nGreenTimeFlag，绿灯类型 1：最小绿 2：参数配置里的绿灯时间 3：绿灯上限
返回值说明：相位时间是否有调整
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 李永萍          创建函数 
====================================================================*/
bool  CLogicCtlPreempt::AdjustPhaseGreenTime(int nRingIndex, int nIndex, int nStageIndexTarget, int nGreenTimeFlag)
{
	TPhaseLampClrRunCounter tRunCounter;
	m_pOpenATCRunStatus->GetPhaseLampClrRunCounter(tRunCounter);

	bool bAdjustGreen = false;
	int  nGreenTime = 0;

	if (nGreenTimeFlag == MANUAL_MINI_GREEN)
	{
		nGreenTime = m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nIndex].m_tPhaseParam.m_wPhaseMinimumGreen;
	}
	else if (nGreenTimeFlag == MANUAL_PHASE_TOPLIMIT_GREEN)
	{
		nGreenTime = MAX_GREEN_TIME;
	}
	else if (nGreenTimeFlag == MANUAL_PHASE_RUN_GREEN)
	{
		nGreenTime = m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nIndex].m_wPhaseGreenTime;
	}
	else 
	{
		//目前只有第一次按了面板手动以后，才进入该分支，此时绿灯时间用参数配置里的绿灯时间
		nGreenTime = m_tOldFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nIndex].m_wPhaseGreenTime;
	}
	
	if (m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nIndex].m_chPhaseStage == C_CH_PHASESTAGE_G)
	{
		if (tRunCounter.m_nLampClrTime[nRingIndex] >= nGreenTime)
		{
			m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nIndex].m_wPhaseGreenTime = tRunCounter.m_nLampClrTime[nRingIndex] + 1;
			
			if (nGreenTimeFlag == MANUAL_MINI_GREEN)
			{
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "SetMinGreen  RingIndex:%d PhaseIndex:%d PhaseGreenTime:%d", nRingIndex, nIndex, tRunCounter.m_nLampClrTime[nRingIndex] + 1);
			}
			else if (nGreenTimeFlag == MANUAL_PHASE_RUN_GREEN)
			{
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "SetPhaseRunGreen  RingIndex:%d PhaseIndex:%d PhaseGreenTime:%d", nRingIndex, nIndex, tRunCounter.m_nLampClrTime[nRingIndex] + 1);
			}
			else if (nGreenTimeFlag == MANUAL_PHASE_SPLIT_GREEN)
			{
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "SetPhaseSplitGreen  RingIndex:%d PhaseIndex:%d PhaseGreenTime:%d", nRingIndex, nIndex, tRunCounter.m_nLampClrTime[nRingIndex] + 1);
			}
			else if (nGreenTimeFlag == MANUAL_PHASE_TOPLIMIT_GREEN)
			{
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "SetTopLimitGreen  RingIndex:%d PhaseIndex:%d PhaseGreenTime:%d", nRingIndex, nIndex, tRunCounter.m_nLampClrTime[nRingIndex] + 1);
			}

			bAdjustGreen = true;
		}
		else
		{
			m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nIndex].m_wPhaseGreenTime = nGreenTime;
			if (nGreenTimeFlag == MANUAL_MINI_GREEN)
			{
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "SetMinGreen  RingIndex:%d PhaseIndex:%d PhaseGreenTime:%d", nRingIndex, nIndex, nGreenTime);
			}
			else if (nGreenTimeFlag == MANUAL_PHASE_RUN_GREEN)
			{
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "SetPhaseRunGreen  RingIndex:%d PhaseIndex:%d PhaseGreenTime:%d", nRingIndex, nIndex, nGreenTime);
			}
			else if (nGreenTimeFlag == MANUAL_PHASE_SPLIT_GREEN)
			{
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "SetPhaseSplitGreen  RingIndex:%d PhaseIndex:%d PhaseGreenTime:%d", nRingIndex, nIndex, nGreenTime);
			}
			else if (nGreenTimeFlag == MANUAL_PHASE_TOPLIMIT_GREEN)
			{
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "SetTopLimitGreen  RingIndex:%d PhaseIndex:%d PhaseGreenTime:%d", nRingIndex, nIndex, nGreenTime);
			}
		}
	}

	return bAdjustGreen;
}


/*==================================================================== 
函数名 ：AdjustPedPhaseGreenTime
功能 ：修改行人相位绿灯时间，保证闪灯从整秒开始
算法实现 ： 
参数说明 ：nRingIndex，环编号 
           nIndex，相位编号 
		   nStageIndexTarget，目标阶段编号 
		   nGreenTimeFlag，绿灯类型 1：最小绿 2：参数配置里的绿灯时间 3：绿灯上限
返回值说明：行人相位时间是否有调整
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 李永萍          创建函数 
====================================================================*/
bool  CLogicCtlPreempt::AdjustPedPhaseGreenTime(int nRingIndex, int nIndex, int nStageIndexTarget, int nGreenTimeFlag)
{
	TPhaseLampClrRunCounter tRunCounter;
	m_pOpenATCRunStatus->GetPhaseLampClrRunCounter(tRunCounter);

	bool bAdjustPedGreen = false;
	int  nPedGreenTime = 0;

	if (nGreenTimeFlag == MANUAL_MINI_GREEN || nGreenTimeFlag == MANUAL_PHASE_TOPLIMIT_GREEN)
	{
		nPedGreenTime = m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nIndex].m_tPhaseParam.m_wPhaseMinimumGreen +
						m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nIndex].m_tPhaseParam.m_byGreenFlash -
						m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhasePedestrianClear;
	}
	else if (nGreenTimeFlag == MANUAL_PHASE_RUN_GREEN)
	{
		nPedGreenTime = m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nIndex].m_wPhaseGreenTime +
						m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nIndex].m_tPhaseParam.m_byGreenFlash -
						m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhasePedestrianClear;
	}
	else
	{
		//目前只有第一次按了面板手动以后，才进入该分支，此时绿灯时间用参数配置里的绿灯时间
		nPedGreenTime = m_tOldFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nIndex].m_wPedPhaseGreenTime;
	}

	if (m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nIndex].m_chPedPhaseStage == C_CH_PHASESTAGE_G)
	{
		if (tRunCounter.m_nPedLampClrTime[nRingIndex] >= nPedGreenTime)
		{
			m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nIndex].m_wPedPhaseGreenTime = tRunCounter.m_nPedLampClrTime[nRingIndex] + 1;

			if (nGreenTimeFlag == MANUAL_MINI_GREEN)
			{
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "SetMinGreen  RingIndex:%d PhaseIndex:%d PedPhaseGreenTime:%d", nRingIndex, nIndex, tRunCounter.m_nPedLampClrTime[nRingIndex] + 1);
			}
			else if (nGreenTimeFlag == MANUAL_PHASE_RUN_GREEN)
			{
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "SetPhaseRunGreen  RingIndex:%d PhaseIndex:%d PedPhaseGreenTime:%d", nRingIndex, nIndex, tRunCounter.m_nPedLampClrTime[nRingIndex] + 1);
			}
			else if (nGreenTimeFlag == MANUAL_PHASE_SPLIT_GREEN)
			{
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "SetPhaseSplitGreen  RingIndex:%d PhaseIndex:%d PedPhaseGreenTime:%d", nRingIndex, nIndex, tRunCounter.m_nPedLampClrTime[nRingIndex] + 1);
			}
			else if (nGreenTimeFlag == MANUAL_PHASE_TOPLIMIT_GREEN)
			{
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "SetTopLimitGreen  RingIndex:%d PhaseIndex:%d PedPhaseGreenTime:%d", nRingIndex, nIndex, tRunCounter.m_nPedLampClrTime[nRingIndex] + 1);
			}

			bAdjustPedGreen = true;
		}
		else
		{
			m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nIndex].m_wPedPhaseGreenTime = nPedGreenTime;
			if (nGreenTimeFlag == MANUAL_MINI_GREEN)
			{
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "SetMinGreen  RingIndex:%d PhaseIndex:%d PedPhaseGreenTime:%d", nRingIndex, nIndex, nPedGreenTime);
			}
			else if (nGreenTimeFlag == MANUAL_PHASE_RUN_GREEN)
			{
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "SetPhaseRunGreen  RingIndex:%d PhaseIndex:%d PedPhaseGreenTime:%d", nRingIndex, nIndex, nPedGreenTime);
			}
			else if (nGreenTimeFlag == MANUAL_PHASE_SPLIT_GREEN)
			{
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "SetPhaseSplitGreen  RingIndex:%d PhaseIndex:%d PedPhaseGreenTime:%d", nRingIndex, nIndex, nPedGreenTime);
			}
			else if (nGreenTimeFlag == MANUAL_PHASE_TOPLIMIT_GREEN)
			{
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "SetTopLimitGreen  RingIndex:%d PhaseIndex:%d PedPhaseGreenTime:%d", nRingIndex, nIndex, nPedGreenTime);
			}
		}
	}

	return bAdjustPedGreen;
}

/*==================================================================== 
函数名 ：SetOverlapPhaseLampClr
功能 ：设置跟随相位的灯色
算法实现 ： 
参数说明 ：nNextStageIndex,下一个阶段编号
返回值说明：
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 李永萍          创建函数 
====================================================================*/ 
void CLogicCtlPreempt::SetOverlapPhaseLampClr(int nNextStageIndex)
{
	if (nNextStageIndex >= m_tRunStageInfo.m_nRunStageCount)
	{
		nNextStageIndex = 0;
	}

	int nTempPhaseID = 0;
	if (m_tPreemptCtlCmd.m_byPreemptPhaseID > 0)
	{
		for (int i = 0;i < m_tRunStageInfo.m_PhaseRunstageInfo[nNextStageIndex].m_nConcurrencyPhaseCount;i++)
		{
			if (m_tRunStageInfo.m_PhaseRunstageInfo[m_tPreemptCtlCmd.m_byPreemptStageIndex].m_nConcurrencyPhase[i] != 
				m_tPreemptCtlCmd.m_byPreemptPhaseID)
			{
				nTempPhaseID = m_tRunStageInfo.m_PhaseRunstageInfo[m_tPreemptCtlCmd.m_byPreemptStageIndex].m_nConcurrencyPhase[i];
				if (m_tPreemptCtlCmd.m_byPreemptStageIndex > 0 && (m_tFixTimeCtlInfo.m_nCurStageIndex == (m_tPreemptCtlCmd.m_byPreemptStageIndex - 1)) &&
					m_tRunStageInfo.m_PhaseRunstageInfo[m_tPreemptCtlCmd.m_byPreemptStageIndex - 1].m_nConcurrencyPhase[i] == nTempPhaseID)
				{
					nTempPhaseID = -1;//优先控制相位所在阶段对应其他环的相位跨阶段(跨优先控制相位所在阶段和优先控制相位所在阶段前的阶段)
				}
				break;
			}
		}
	}

	for (int i = 0;i < m_tFixTimeCtlInfo.m_nOverlapCount;i ++)
	{
		BYTE byOverlapNum = m_tFixTimeCtlInfo.m_atOverlapInfo[i].m_tOverlapParam.m_byOverlapNumber;
		if (byOverlapNum == 0)
		{
			continue;
		}

		int  nRingIndex = 0;
		int  nPhaseIndex = 0;
		int  m = 0, n = 0;
		char chRet = 0;
		char chPedRet = 0;
		int  nPhaseID = 0, nNextPhaseID = 0;

		bool bGreenFlag = false;
		bool bGreenFlashFlag = false;
		bool bYellowFlag = false;
		bool bPedGreenFlag = false;
		bool bPedGreenFlashFlag = false;
		bool bPedYellowFlag = false;

		// 先处理跟随机动车相位
		for (int j = 0;j < MAX_PHASE_COUNT_IN_OVERLAP;j ++)
		{
			BYTE byMainPhaseNum = m_tFixTimeCtlInfo.m_atOverlapInfo[i].m_tOverlapParam.m_byArrOverlapIncludedPhases[j];
			if (byMainPhaseNum > 0)
			{
				GetPhaseIndexByPhaseNumber(nRingIndex, nPhaseIndex, (int)byMainPhaseNum);
				chRet = GetPhaseStatus(byMainPhaseNum, false);

				int nStageIndex = m_tFixTimeCtlInfo.m_nCurStageIndex;
				for (m = 0;m < m_tRunStageInfo.m_PhaseRunstageInfo[nStageIndex].m_nConcurrencyPhaseCount;m++)
				{
					if (!IsNeglectPhase(byMainPhaseNum) && m_tRunStageInfo.m_PhaseRunstageInfo[nStageIndex].m_nConcurrencyPhase[m] == byMainPhaseNum &&
						byMainPhaseNum != m_tPreemptCtlCmd.m_byPreemptPhaseID && byMainPhaseNum != nTempPhaseID)
					{
						// 当前阶段运行的母相位正在绿，继续绿；当前阶段运行的相位为跟随相位即将跟随的非第一个母相位，跟随相位已经在绿，则继续绿
						if (chRet == C_CH_PHASESTAGE_G 
							|| (chRet == C_CH_PHASESTAGE_U && m_tFixTimeCtlInfo.m_atOverlapInfo[i].m_chOverlapStage == C_CH_PHASESTAGE_G))
						{
							bGreenFlag = true;
							break;
						}
					}
				}

				if (bGreenFlag)
				{
					break;
				}

				if (m_tFixTimeCtlInfo.m_atOverlapInfo[i].m_chOverlapStage == C_CH_PHASESTAGE_G)
				{
					for (m = 0;m < m_tRunStageInfo.m_PhaseRunstageInfo[nNextStageIndex].m_nConcurrencyPhaseCount;m++)
					{
					    nNextPhaseID = m_tRunStageInfo.m_PhaseRunstageInfo[nNextStageIndex].m_nConcurrencyPhase[m];
						// 当前阶段的相位与下一个阶段相位id不一致，且都是当前跟随相位的母相位，且下一个阶段的相位正常放行，则继续绿
						if (!IsNeglectPhase(nNextPhaseID) && IsPhaseNumInOverlap(nNextPhaseID, i) && nNextPhaseID != m_tPreemptCtlCmd.m_byPreemptPhaseID && nNextPhaseID != nTempPhaseID && nTempPhaseID != -1)
						{
							for (n = 0;n < m_tRunStageInfo.m_PhaseRunstageInfo[nStageIndex].m_nConcurrencyPhaseCount;n++)
							{
								nPhaseID = m_tRunStageInfo.m_PhaseRunstageInfo[nStageIndex].m_nConcurrencyPhase[n];
								if (IsPhaseNumInOverlap(nPhaseID, i))
								{
									bGreenFlag = true;
									break;
								}
							}
						}

						if (bGreenFlag)
						{
							break;
						}
					}

					if (bGreenFlag)
					{
						break;
					}
				}

				chRet = GetPhaseStatus(byMainPhaseNum, false);
				if (chRet == C_CH_PHASESTAGE_GF && byMainPhaseNum != m_tPreemptCtlCmd.m_byPreemptPhaseID && byMainPhaseNum != nTempPhaseID && m_tFixTimeCtlInfo.m_atOverlapInfo[i].m_chOverlapStage == C_CH_PHASESTAGE_G)
				{
					bGreenFlashFlag = true;

					for (int k = 0;k < m_nChannelCount;k++)
					{
						if (m_atChannelInfo[k].m_byChannelNumber == 0)
						{
							continue;
						}

						if ((int)m_atChannelInfo[k].m_byChannelControlSource == byOverlapNum)
						{
							if (m_atChannelInfo[k].m_byChannelControlType == OVERLAP_CHA)
							{
								m_pOpenATCRunStatus->SetGreenFlashCount((m_atChannelInfo[k].m_byChannelNumber - 1) * 3 + 2,m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_wPhaseStageRunTime * 2);
							}
						} 
					}
				}
				if (chRet == C_CH_PHASESTAGE_Y && byMainPhaseNum != m_tPreemptCtlCmd.m_byPreemptPhaseID && byMainPhaseNum != nTempPhaseID && (m_tFixTimeCtlInfo.m_atOverlapInfo[i].m_chOverlapStage == C_CH_PHASESTAGE_G || m_tFixTimeCtlInfo.m_atOverlapInfo[i].m_chOverlapStage == C_CH_PHASESTAGE_GF))
				{
					bYellowFlag = true;
				}    
			}    
		}

		// 再处理跟随行人相位
		for (int j = 0;j < MAX_PHASE_COUNT_IN_OVERLAP;j ++)
		{
			BYTE byMainPhaseNum = m_tFixTimeCtlInfo.m_atOverlapInfo[i].m_tOverlapParam.m_byArrOverlapIncludedPhases[j];
			if (byMainPhaseNum > 0)
			{
				GetPhaseIndexByPhaseNumber(nRingIndex, nPhaseIndex, (int)byMainPhaseNum);
				chPedRet = GetPhaseStatus(byMainPhaseNum, true);

				int nStageIndex = m_tFixTimeCtlInfo.m_nCurStageIndex;
				for (m = 0;m < m_tRunStageInfo.m_PhaseRunstageInfo[nStageIndex].m_nConcurrencyPhaseCount;m++)
				{
					if (!IsNeglectPhase(byMainPhaseNum) && m_tRunStageInfo.m_PhaseRunstageInfo[nStageIndex].m_nConcurrencyPhase[m] == byMainPhaseNum &&
						byMainPhaseNum != m_tPreemptCtlCmd.m_byPreemptPhaseID && byMainPhaseNum != nTempPhaseID)
					{
						if (chPedRet == C_CH_PHASESTAGE_G 
							|| (chPedRet == C_CH_PHASESTAGE_U && m_tFixTimeCtlInfo.m_atOverlapInfo[i].m_chPedOverlapStage == C_CH_PHASESTAGE_G))
						{
							bPedGreenFlag = true;
							break;
						}
					}
				}

				if (bPedGreenFlag)
				{
					break;
				}

				if (m_tFixTimeCtlInfo.m_atOverlapInfo[i].m_chPedOverlapStage == C_CH_PHASESTAGE_G)
				{
					for (m = 0;m < m_tRunStageInfo.m_PhaseRunstageInfo[nNextStageIndex].m_nConcurrencyPhaseCount;m++)
					{
						nNextPhaseID = m_tRunStageInfo.m_PhaseRunstageInfo[nNextStageIndex].m_nConcurrencyPhase[m];
						if (!IsNeglectPhase(nNextPhaseID) && IsPhaseNumInOverlap(nNextPhaseID, i) && nNextPhaseID != m_tPreemptCtlCmd.m_byPreemptPhaseID && nNextPhaseID != nTempPhaseID && nTempPhaseID != -1)
						{
							for (n = 0;n < m_tRunStageInfo.m_PhaseRunstageInfo[nStageIndex].m_nConcurrencyPhaseCount;n++)
							{
								nPhaseID = m_tRunStageInfo.m_PhaseRunstageInfo[nStageIndex].m_nConcurrencyPhase[n];
								if (IsPhaseNumInOverlap(nPhaseID, i))
								{
									bPedGreenFlag = true;
									break;
								}
							}
						}

						if (bPedGreenFlag)
						{
							break;
						}
					}

					if (bPedGreenFlag)
					{
						break;
					}
				}

				chPedRet = GetPhaseStatus(byMainPhaseNum, true);
				if (chPedRet == C_CH_PHASESTAGE_GF && byMainPhaseNum != m_tPreemptCtlCmd.m_byPreemptPhaseID && byMainPhaseNum != nTempPhaseID && m_tFixTimeCtlInfo.m_atOverlapInfo[i].m_chPedOverlapStage == C_CH_PHASESTAGE_G)
				{
					bPedGreenFlashFlag = true;

					for (int k = 0;k < m_nChannelCount;k++)
					{
						if (m_atChannelInfo[k].m_byChannelNumber == 0)
						{
							continue;
						}

						if ((int)m_atChannelInfo[k].m_byChannelControlSource == byOverlapNum)
						{
							if (m_atChannelInfo[k].m_byChannelControlType == OVERLAP_PED_CHA)
							{
								m_pOpenATCRunStatus->SetGreenFlashCount((m_atChannelInfo[k].m_byChannelNumber - 1) * 3 + 2,m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_wPedPhaseStageRunTime * 2);
							}
						} 
					}
				}
				if (chPedRet == C_CH_PHASESTAGE_Y && byMainPhaseNum != m_tPreemptCtlCmd.m_byPreemptPhaseID && byMainPhaseNum != nTempPhaseID && (m_tFixTimeCtlInfo.m_atOverlapInfo[i].m_chPedOverlapStage == C_CH_PHASESTAGE_G || m_tFixTimeCtlInfo.m_atOverlapInfo[i].m_chPedOverlapStage == C_CH_PHASESTAGE_GF))
				{
					if (m_tFixTimeCtlInfo.m_atOverlapInfo[i].m_chPedOverlapStage == C_CH_PHASESTAGE_GF)
					{
						for (m = 0;m < m_tRunStageInfo.m_PhaseRunstageInfo[nNextStageIndex].m_nConcurrencyPhaseCount;m++)
						{
							if (m_tRunStageInfo.m_PhaseRunstageInfo[nNextStageIndex].m_nConcurrencyPhase[m] == byMainPhaseNum
								&& m_tRunStageInfo.m_PhaseRunstageInfo[nStageIndex].m_nConcurrencyPhase[m] != m_tRunStageInfo.m_PhaseRunstageInfo[nNextStageIndex].m_nConcurrencyPhase[m])
							{
								bPedGreenFlashFlag = true;
								break;
							}
						}
					}

					if (bPedGreenFlashFlag)
					{
						break;
					}

					bPedYellowFlag = true;
				}    
			}    
		}

		if (bGreenFlag)
		{
			//m_tFixTimeCtlInfo.m_atOverlapInfo[i].m_chOverlapStage = C_CH_PHASESTAGE_G;

			if (m_tPreemptCtlCmd.m_byPreemptSwitchFlag == 1)
			{
				SetOverlapCurPreemptPhaseLampClr(i, byOverlapNum, false);
			}
			else
			{
				m_tFixTimeCtlInfo.m_atOverlapInfo[i].m_chOverlapStage = C_CH_PHASESTAGE_G;
			}
		}
		else if (bGreenFlashFlag)
		{
			m_tFixTimeCtlInfo.m_atOverlapInfo[i].m_chOverlapStage = C_CH_PHASESTAGE_GF;
		}
		else if (bYellowFlag)
		{
			m_tFixTimeCtlInfo.m_atOverlapInfo[i].m_chOverlapStage = C_CH_PHASESTAGE_Y;
		}
		else
		{
			SetOverlapCurPreemptPhaseLampClr(i, byOverlapNum, false);
		} 

		if (bPedGreenFlag)
		{
			//m_tFixTimeCtlInfo.m_atOverlapInfo[i].m_chPedOverlapStage = C_CH_PHASESTAGE_G;
		
			if (m_tPreemptCtlCmd.m_byPreemptSwitchFlag == 1)
			{
				SetOverlapCurPreemptPhaseLampClr(i, byOverlapNum, true);
			}
			else
			{
				m_tFixTimeCtlInfo.m_atOverlapInfo[i].m_chPedOverlapStage = C_CH_PHASESTAGE_G;
			}
		}
		else if (bPedGreenFlashFlag)
		{
			m_tFixTimeCtlInfo.m_atOverlapInfo[i].m_chPedOverlapStage = C_CH_PHASESTAGE_GF;
		}
		else if (bPedYellowFlag)
		{
			m_tFixTimeCtlInfo.m_atOverlapInfo[i].m_chPedOverlapStage = C_CH_PHASESTAGE_R;
		}
		else
		{
			SetOverlapCurPreemptPhaseLampClr(i, byOverlapNum, true);
		} 

		SetChannelStatus((int)byOverlapNum,OVERLAP_SRC,m_tFixTimeCtlInfo.m_atOverlapInfo[i].m_chOverlapStage,m_tFixTimeCtlInfo.m_atOverlapInfo[i].m_chPedOverlapStage);
	}
}

/*==================================================================== 
函数名 ：GetPreemptCtlCmdFromList
功能 ：在优先控制命令队列中获取要执行的优先控制指令
算法实现 ： 
参数说明 ： 无
返回值说明：无
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 李永萍          创建函数 
====================================================================*/
bool CLogicCtlPreempt::GetPreemptCtlCmdFromList()
{
	int  nIndex = 0;
	bool bSystemPatternInterruptCtlFlag = false;
	bool bPreemptPatternInterruptCtlFlag = false;
	bool bPreemptCtlFlag = false;

	TPreemptCtlCmd  tPreemptCtlCmd;
	memset(&tPreemptCtlCmd,0,sizeof(tPreemptCtlCmd));

	//查找方案干预指令
	if (m_pOpenATCRunStatus->GetPreemptCtlCmdListElement(CTL_SOURCE_SYSTEM, true, PREEMPT_TYPE_DEFAULT, 0, tPreemptCtlCmd))
	{
		bSystemPatternInterruptCtlFlag = true;
	}
	else
	{
		if (m_pOpenATCRunStatus->GetPreemptCtlCmdListElement(CTL_SOURCE_PREEMPT, true, PREEMPT_TYPE_DEFAULT, 0, tPreemptCtlCmd))
		{
			bPreemptPatternInterruptCtlFlag = true;
		}
	}
	//查找紧急优先指令
	if (!bSystemPatternInterruptCtlFlag && !bPreemptPatternInterruptCtlFlag)
	{
		bPreemptCtlFlag = false;
		memset(&tPreemptCtlCmd,0,sizeof(tPreemptCtlCmd));

		for (nIndex = 0;nIndex < m_tRunStageInfo.m_nRunStageCount;nIndex++)
		{
			if (m_pOpenATCRunStatus->GetPreemptCtlCmdListElement(CTL_SOURCE_SYSTEM, false, PREEMPT_TYPE_URGENT, nIndex, tPreemptCtlCmd))
			{
				bPreemptCtlFlag = true;
				break;
			}
		}

		if (!bPreemptCtlFlag)
		{
			for (nIndex = m_tFixTimeCtlInfo.m_nCurStageIndex + 1;nIndex < m_tFixTimeCtlInfo.m_nCurStageIndex + 1 + m_tRunStageInfo.m_nRunStageCount;nIndex++)
			{
				if (m_pOpenATCRunStatus->GetPreemptCtlCmdListElement(CTL_SOURCE_SYSTEM, false, PREEMPT_TYPE_NORMAL, nIndex % m_tRunStageInfo.m_nRunStageCount, tPreemptCtlCmd))
				{
					bPreemptCtlFlag = true;
					break;
				}
			}
		}
		
		if (!bPreemptCtlFlag)
		{
			for (nIndex = 0;nIndex < m_tRunStageInfo.m_nRunStageCount;nIndex++)
			{
				if (m_pOpenATCRunStatus->GetPreemptCtlCmdListElement(CTL_SOURCE_PREEMPT, false, PREEMPT_TYPE_URGENT, nIndex, tPreemptCtlCmd))
				{
					bPreemptCtlFlag = true;
					break;
				}
			}

			if (!bPreemptCtlFlag)
			{
				for (nIndex = m_tFixTimeCtlInfo.m_nCurStageIndex + 1;nIndex < m_tFixTimeCtlInfo.m_nCurStageIndex + 1 + m_tRunStageInfo.m_nRunStageCount;nIndex++)
				{
					if (m_pOpenATCRunStatus->GetPreemptCtlCmdListElement(CTL_SOURCE_PREEMPT, false, PREEMPT_TYPE_NORMAL, nIndex % m_tRunStageInfo.m_nRunStageCount, tPreemptCtlCmd))
					{
						bPreemptCtlFlag = true;
						break;
					}
				}
			}
		}

		if (bPreemptCtlFlag)
		{
			m_bClearPulseFlag = false;

			bool bSwitchToPreemptCtlFlag = false;//切换优先控制命令标志
			if (!m_bPreemptCtlCmdEndFlag)
			{
				if (m_tPreemptCtlCmd.m_byPreemptStageIndex > tPreemptCtlCmd.m_byPreemptStageIndex)
				{
					if (m_tFixTimeCtlInfo.m_nCurStageIndex <= tPreemptCtlCmd.m_byPreemptStageIndex)
					{
						if ((m_tPreemptCtlCmd.m_byPreemptType == PREEMPT_TYPE_URGENT && tPreemptCtlCmd.m_byPreemptType == PREEMPT_TYPE_URGENT) ||
							(m_tPreemptCtlCmd.m_byPreemptType == PREEMPT_TYPE_NORMAL && tPreemptCtlCmd.m_byPreemptType == PREEMPT_TYPE_URGENT) ||
							(m_tPreemptCtlCmd.m_byPreemptType == PREEMPT_TYPE_NORMAL && tPreemptCtlCmd.m_byPreemptType == PREEMPT_TYPE_NORMAL))
						{ 
							bSwitchToPreemptCtlFlag = true;//还没有切到当前阶段之后的阶段，又来了指定阶段之前的阶段，则直接运行新的优先命令
						}
					}
					else
					{
						if (m_tPreemptCtlCmd.m_byPreemptSwitchFlag == 0 && m_tPreemptCtlCmd.m_byPreemptType == PREEMPT_TYPE_NORMAL && tPreemptCtlCmd.m_byPreemptType == PREEMPT_TYPE_URGENT)
						{ 
							bSwitchToPreemptCtlFlag = true;//还没有切到当前阶段之前的阶段，又来了紧急优先指令，则直接运行新的优先命令
						}
					}
				}
				else if (m_tPreemptCtlCmd.m_byPreemptStageIndex < tPreemptCtlCmd.m_byPreemptStageIndex)
				{
					if (m_tFixTimeCtlInfo.m_nCurStageIndex > m_tPreemptCtlCmd.m_byPreemptStageIndex)
					{
						if ((m_tPreemptCtlCmd.m_byPreemptType == PREEMPT_TYPE_URGENT && tPreemptCtlCmd.m_byPreemptType == PREEMPT_TYPE_URGENT) ||
							(m_tPreemptCtlCmd.m_byPreemptType == PREEMPT_TYPE_NORMAL && tPreemptCtlCmd.m_byPreemptType == PREEMPT_TYPE_URGENT) ||
							(m_tPreemptCtlCmd.m_byPreemptType == PREEMPT_TYPE_NORMAL && tPreemptCtlCmd.m_byPreemptType == PREEMPT_TYPE_NORMAL))
						{ 
							bSwitchToPreemptCtlFlag = true;//还没有切到当前阶段之前的阶段，又来了当前阶段之后的阶段，则直接运行新的优先控制命令
						}
					}
					else
					{
						if (m_tPreemptCtlCmd.m_byPreemptSwitchFlag == 0 && m_tPreemptCtlCmd.m_byPreemptType == PREEMPT_TYPE_NORMAL && tPreemptCtlCmd.m_byPreemptType == PREEMPT_TYPE_URGENT)
						{ 
							bSwitchToPreemptCtlFlag = true;//还没有切到当前阶段之后的阶段，又来了紧急优先指令，则直接运行新的优先命令
						}
					}
				}
				else if (m_tPreemptCtlCmd.m_byPreemptStageIndex == tPreemptCtlCmd.m_byPreemptStageIndex)
				{
					if (!m_bPreemptCtlStageProcessFlag[tPreemptCtlCmd.m_byPreemptStageIndex] && GetPreemptCtlPhaseStage(tPreemptCtlCmd.m_byPreemptPhaseID) == C_CH_PHASESTAGE_G)
					{
						bSwitchToPreemptCtlFlag = true;//驻留当前阶段，直接运行新的优先控制命令
					}

					if (m_tPreemptCtlCmd.m_byPreemptSwitchFlag == 0 && tPreemptCtlCmd.m_byPreemptType == PREEMPT_TYPE_URGENT)
					{
						bSwitchToPreemptCtlFlag = true;//对同一个阶段的紧急优先指令，直接运行新的优先控制命令
					}
				}
			}
			else
			{
				bSwitchToPreemptCtlFlag = true;//当前优化控制命令结束，直接运行新的优先控制命令
			}
			
			if (memcmp(&m_tPreemptCtlCmd, &tPreemptCtlCmd, sizeof(tPreemptCtlCmd)) == 0)
			{
				bSwitchToPreemptCtlFlag = false;
			}

			if (bSwitchToPreemptCtlFlag)//非优先控制切优先控制，或者还没有开始切换到目标优先控制，又收到更高级别的优先控制
			{
				if (tPreemptCtlCmd.m_byPreemptType == PREEMPT_TYPE_URGENT)
				{
					m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "GetPreemptCtlCmdFromList, EmergentCtlCmd, PreemptPhaseID:%d TargetStageIndex:%d", tPreemptCtlCmd.m_byPreemptPhaseID, tPreemptCtlCmd.m_byPreemptStageIndex);
				}
				else
				{
					m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "GetPreemptCtlCmdFromList, NormalCtlCmd, PreemptPhaseID:%d TargetStageIndex:%d", tPreemptCtlCmd.m_byPreemptPhaseID, tPreemptCtlCmd.m_byPreemptStageIndex);
				}

				memcpy(&m_tPreemptCtlCmd, &tPreemptCtlCmd, sizeof(tPreemptCtlCmd));

				m_bPreemptCtlCmdEndFlag = false;
				m_nManualCurStatus = MANUAL_CONTROL_STATUS;
				memset(m_bPreemptCtlStageProcessFlag, 0, sizeof(m_bPreemptCtlStageProcessFlag));
				m_bPreemptCtlStageProcessFlag[tPreemptCtlCmd.m_byPreemptStageIndex] = true;

				memset(&m_tNextPreemptCtlCmd,0,sizeof(TPreemptCtlCmd));
				m_tNextPreemptCtlCmd.m_byPreemptType = PREEMPT_TYPE_DEFAULT;
				if (tPreemptCtlCmd.m_byPreemptStageIndex != m_tNextPreemptCtlCmd.m_byPreemptStageIndex)
				{
					m_bPreemptCtlStageProcessFlag[m_tNextPreemptCtlCmd.m_byPreemptStageIndex] = false;
				}

				RecalculteStageTimeByDelayTimeAndDuration();
				
				//优先控制的阶段是当前正在运行阶段，则修改完相位时间就置相位开始切换标志，这样当前阶段运行结束后不会再次进入当前阶段
				if (m_nReturnAutoCtrlStageIndex == m_tFixTimeCtlInfo.m_nCurStageIndex)
				{
					if (GetPreemptCtlPhaseStage(tPreemptCtlCmd.m_byPreemptPhaseID) == C_CH_PHASESTAGE_G)
					{
						m_tPreemptCtlCmd.m_byPreemptSwitchFlag = 1;//设置优先相位开始切换标志
						m_pOpenATCRunStatus->ModifyPreemptSwitchFlag(m_tPreemptCtlCmd.m_nCmdSource, false, m_tPreemptCtlCmd.m_byPreemptType, m_tPreemptCtlCmd.m_byPreemptPhaseID, m_tPreemptCtlCmd.m_bIncludeConcurPhase);
						m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "GetPreemptCtlCmdFromList, Modify SwitchFlag PreemptPhaseID:%d, TargetStageIndex:%d", m_tPreemptCtlCmd.m_byPreemptPhaseID, m_tPreemptCtlCmd.m_byPreemptStageIndex);
					}
					else
					{
						m_bPreemptCtlStageProcessFlag[tPreemptCtlCmd.m_byPreemptStageIndex] = false;//此时不能置开始处理标志
					}
				}
			}
			else//优先控制过程中，当前优先控制命令开始切换，还没有结束，备份下一个优先控制命令(该控制命令的优先等级低于当前优先控制命令)
			{
				if (m_tPreemptCtlCmd.m_byPreemptSwitchFlag == 1 && (!m_bPreemptCtlStageProcessFlag[tPreemptCtlCmd.m_byPreemptStageIndex] || m_tNextPreemptCtlCmd.m_bPatternInterruptCmd ||
				   (tPreemptCtlCmd.m_byPreemptType == PREEMPT_TYPE_URGENT && m_tNextPreemptCtlCmd.m_byPreemptStageIndex == tPreemptCtlCmd.m_byPreemptStageIndex) || 
			       (tPreemptCtlCmd.m_byPreemptType == PREEMPT_TYPE_URGENT && m_tPreemptCtlCmd.m_byPreemptStageIndex == tPreemptCtlCmd.m_byPreemptStageIndex)) && 
				    memcmp(&m_tNextPreemptCtlCmd, &tPreemptCtlCmd, sizeof(tPreemptCtlCmd)) != 0)
				{
					if (m_tNextPreemptCtlCmd.m_byPreemptStageIndex != m_tPreemptCtlCmd.m_byPreemptStageIndex)
					{
						m_bPreemptCtlStageProcessFlag[m_tNextPreemptCtlCmd.m_byPreemptStageIndex] = false;
					}
				
					if (tPreemptCtlCmd.m_byPreemptType == PREEMPT_TYPE_URGENT || tPreemptCtlCmd.m_byPreemptType == PREEMPT_TYPE_NORMAL)
					{
						if (tPreemptCtlCmd.m_byPreemptPhaseID == m_tPreemptCtlCmd.m_byPreemptPhaseID)
						{
							RecalculteStageTimeByDelayTimeAndDuration();
							if (GetPreemptCtlPhaseStage(tPreemptCtlCmd.m_byPreemptPhaseID) == C_CH_PHASESTAGE_G)
							{
								m_pOpenATCRunStatus->ModifyPreemptSwitchFlag(tPreemptCtlCmd.m_nCmdSource, false, tPreemptCtlCmd.m_byPreemptType, tPreemptCtlCmd.m_byPreemptPhaseID, tPreemptCtlCmd.m_bIncludeConcurPhase);
								m_pOpenATCRunStatus->DelteFromPreemptCtlCmdList(tPreemptCtlCmd.m_nCmdSource, false, tPreemptCtlCmd.m_byPreemptType, tPreemptCtlCmd.m_byPreemptPhaseID);
								return true;
							}
						}
					}
						
					m_bPreemptCtlStageProcessFlag[tPreemptCtlCmd.m_byPreemptStageIndex] = true;
					memcpy(&m_tNextPreemptCtlCmd, &tPreemptCtlCmd, sizeof(tPreemptCtlCmd));//备份下一个需要执行的控制命令
					if (tPreemptCtlCmd.m_byPreemptType == PREEMPT_TYPE_URGENT)
					{
						m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "GetPreemptCtlCmdFromList, NextEmergentCtlCmd, PreemptPhaseID:%d TargetStageIndex:%d", tPreemptCtlCmd.m_byPreemptPhaseID, tPreemptCtlCmd.m_byPreemptStageIndex);
					}
					else
					{
						m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "GetPreemptCtlCmdFromList, NextNormalCtlCmd, PreemptPhaseID:%d TargetStageIndex:%d", tPreemptCtlCmd.m_byPreemptPhaseID, tPreemptCtlCmd.m_byPreemptStageIndex);
					}
				}
			}
			return true;
		}
		else
		{
			if (m_tPreemptCtlCmd.m_byPreemptSwitchFlag == 1 && m_nManualCurStatus == MANUAL_CONTROL_STATUS && !m_tNextPreemptCtlCmd.m_bPatternInterruptCmd)
			{
				CreatePreemptCtlCmdReturnToSelf();//生成回到自主的命令，直接放到优先控制列表中
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "GetPreemptCtlCmdFromList, Prepare CreatePreemptCtlCmdReturnToSelf From Preempt");
			}
			return false;
		}
	}
	else
	{
		if (tPreemptCtlCmd.m_nCmdSource != CTL_SOURCE_PREEMPT)//1.面板按了指令，2.系统按了自主或下发了带方案的方案干预指令
		{
			if (m_tPreemptCtlCmd.m_byPreemptSwitchFlag == 0)//当前的优先控制还没有开始切换
			{
			
			}
			else//当前的优先控制已经开始切换
			{
				SetTransStatus();
			}

			m_bPreemptCtlCmdEndFlag = true;

			m_nReturnAutoCtrlStageIndex = -1;
			m_nManualCurStatus = MANUAL_STAGE_TRANS;
			memcpy(&m_tPreemptCtlCmd, &tPreemptCtlCmd, sizeof(tPreemptCtlCmd));

			memset(&m_tNextPreemptCtlCmd,0,sizeof(TPreemptCtlCmd));
			m_tNextPreemptCtlCmd.m_byPreemptType = PREEMPT_TYPE_DEFAULT;
			m_bPreemptCtlStageProcessFlag[m_tNextPreemptCtlCmd.m_byPreemptStageIndex] = false;

			m_pOpenATCRunStatus->DelteFromPreemptCtlCmdList(m_tPreemptCtlCmd.m_nCmdSource, true, PREEMPT_TYPE_DEFAULT, tPreemptCtlCmd.m_byPreemptPhaseID);

			m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "GetPreemptCtlCmdFromList, PatternInterruptCmd From System");
		}
		else//这个一般是优先控制过渡结束以后，默认生成回到自主的命令，优先级最低
		{
			memcpy(&m_tNextPreemptCtlCmd, &tPreemptCtlCmd, sizeof(tPreemptCtlCmd));
			m_pOpenATCRunStatus->DelteFromPreemptCtlCmdList(m_tNextPreemptCtlCmd.m_nCmdSource, true, PREEMPT_TYPE_DEFAULT, m_tNextPreemptCtlCmd.m_byPreemptPhaseID);

			m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "GetPreemptCtlCmdFromList, Prepare PatternInterruptCmd From Preempt");
		}

		return true;
	}
}

/*==================================================================== 
函数名 ：IsHaveUrgentCtlCmd
功能 ：优先控制命令队列中是否有紧急控制命令
算法实现 ： 
参数说明 ： 无
返回值说明：无
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 李永萍          创建函数 
====================================================================*/
bool CLogicCtlPreempt::IsHaveUrgentCtlCmd()
{
	TPreemptCtlCmd  tPreemptCtlCmd;
	memset(&tPreemptCtlCmd,0,sizeof(tPreemptCtlCmd));

	bool bFlag = false;

	int  i = 0;
	for (i = 0;i < m_tRunStageInfo.m_nRunStageCount;i++)
	{
		memset(&tPreemptCtlCmd,0,sizeof(tPreemptCtlCmd));
		if (m_pOpenATCRunStatus->GetPreemptCtlCmdListElement(CTL_SOURCE_SYSTEM, false, PREEMPT_TYPE_URGENT, i, tPreemptCtlCmd))
		{
			bFlag = true;
			break;
		}
	}

	if (!bFlag)
	{
		for (i = 0;i < m_tRunStageInfo.m_nRunStageCount;i++)
		{
			memset(&tPreemptCtlCmd,0,sizeof(tPreemptCtlCmd));
			if (m_pOpenATCRunStatus->GetPreemptCtlCmdListElement(CTL_SOURCE_PREEMPT, false, PREEMPT_TYPE_URGENT, i, tPreemptCtlCmd))
			{
				bFlag = true;
				break;
			}
		}
	}

	return bFlag;
}

/*==================================================================== 
函数名 ：GetRingIndex
功能 ：根据相位ID查找所在环号
算法实现 ： 
参数说明 ： 无
返回值说明：无
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 李永萍          创建函数 
====================================================================*/
int CLogicCtlPreempt::GetRingIndex(int nPhaseID)
{
	for (int nRingIndex = 0; nRingIndex < m_tFixTimeCtlInfo.m_nRingCount; nRingIndex++)
	{
		PTRingCtlInfo ptRingRunInfo = &(m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex]);
		for (int nPhaseIndex = 0;nPhaseIndex < ptRingRunInfo->m_nPhaseCount;nPhaseIndex++)
		{
			if (ptRingRunInfo->m_atPhaseInfo[nPhaseIndex].m_tPhaseParam.m_byPhaseNumber == nPhaseID)
			{
				return nRingIndex;
			}
		}
	}

	return -1;
}

/*==================================================================== 
函数名 ：GetNextStageIndex
功能 ：获取下一个阶段编号
算法实现 ： 
参数说明 ： 无
返回值说明：无
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 李永萍          创建函数 
====================================================================*/
int CLogicCtlPreempt::GetNextStageIndex()
{
	int nStageIndex = m_tFixTimeCtlInfo.m_nCurStageIndex + 1;
	if (!m_tNextPreemptCtlCmd.m_bPatternInterruptCmd)
	{
		if (m_tNextPreemptCtlCmd.m_byPreemptType == PREEMPT_TYPE_NORMAL)
		{
			if (!IsHaveUrgentCtlCmd())//按照阶段顺序处理普通优先控制的时候，如果队列中还要紧急优先控制，则当紧急优先处理
			{
				nStageIndex = m_tFixTimeCtlInfo.m_nCurStageIndex + 1;
			}
			else
			{
				nStageIndex = m_tNextPreemptCtlCmd.m_byPreemptStageIndex;
			}
		}
		else if (m_tNextPreemptCtlCmd.m_byPreemptType == PREEMPT_TYPE_URGENT)
		{
			nStageIndex = m_tNextPreemptCtlCmd.m_byPreemptStageIndex;
		}
	}
	else
	{
		nStageIndex = m_tFixTimeCtlInfo.m_nCurStageIndex + 1;
	}

	if (nStageIndex >= m_tRunStageInfo.m_nRunStageCount)
	{
		nStageIndex = 0;
	}

	return nStageIndex;
}


/*==================================================================== 
函数名 ：SetPreemptCtlCmd
功能 ：设置下一个优先控制命令
算法实现 ： 
参数说明 ： nStageIndex，阶段编号
返回值说明：无
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 李永萍          创建函数 
====================================================================*/
void CLogicCtlPreempt::SetPreemptCtlCmd(int nStageIndex)
{
	if (m_nReturnAutoCtrlStageIndex == m_tFixTimeCtlInfo.m_nCurStageIndex && !m_tPreemptCtlCmd.m_bPatternInterruptCmd)
	{
		if (!m_bPreemptCtlCmdEndFlag && m_nManualCurStatus == MANUAL_CONTROL_STATUS && m_bPreemptCtlStageProcessFlag[m_tPreemptCtlCmd.m_byPreemptStageIndex])
		{
			m_bPreemptCtlCmdEndFlag = true;//置切换结束标志
			m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "Switch Stage Succeed, PreemptPhaseID:%d SetPreemptCtlCmdEndFlag", m_tPreemptCtlCmd.m_byPreemptPhaseID);
		}

		m_bPreemptCtlStageProcessFlag[m_tPreemptCtlCmd.m_byPreemptStageIndex] = false;
	}

	if (m_bPreemptCtlCmdEndFlag)//切换结束，设置下一个优先控制指令
	{
		memset(m_bCurAndNextCmdInSameStage, 0, sizeof(m_bCurAndNextCmdInSameStage));

		if (m_tNextPreemptCtlCmd.m_bPatternInterruptCmd || m_tNextPreemptCtlCmd.m_byPreemptType == PREEMPT_TYPE_URGENT || m_tNextPreemptCtlCmd.m_byPreemptType == PREEMPT_TYPE_NORMAL)
		{
			//已经切到目标阶段，先把之前的优先控制指令从队列中删除
			m_pOpenATCRunStatus->DelteFromPreemptCtlCmdList(m_tPreemptCtlCmd.m_nCmdSource, false, m_tPreemptCtlCmd.m_byPreemptType, m_tPreemptCtlCmd.m_byPreemptPhaseID);
			m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "DelteFromPreemptCtlCmdList, PreemptPhaseID:%d TargetStageIndex:%d", m_tPreemptCtlCmd.m_byPreemptPhaseID, m_tPreemptCtlCmd.m_byPreemptStageIndex);

			if (m_tPreemptCtlCmd.m_byPreemptStageIndex == m_tNextPreemptCtlCmd.m_byPreemptStageIndex)
			{
				m_bCurAndNextCmdInSameStage[GetRingIndex(m_tPreemptCtlCmd.m_byPreemptPhaseID)] = true;
			}

			//已经切到目标阶段，开始执行下一个优先控制指令
			memcpy(&m_tPreemptCtlCmd,&m_tNextPreemptCtlCmd,sizeof(TPreemptCtlCmd));

			if (!m_tNextPreemptCtlCmd.m_bPatternInterruptCmd)
			{
				m_nManualCurStatus = MANUAL_CONTROL_STATUS;

				m_bPreemptCtlCmdEndFlag = false;
				m_bPreemptCtlStageProcessFlag[m_tNextPreemptCtlCmd.m_byPreemptStageIndex] = false;

				int nStageIndexTarget = m_tPreemptCtlCmd.m_byPreemptStageIndex;
				if (m_tPreemptCtlCmd.m_byPreemptType == PREEMPT_TYPE_URGENT)
				{
					m_nNextStageIndex = nStageIndexTarget;
				}
				else
				{
					m_nNextStageIndex = m_tFixTimeCtlInfo.m_nCurStageIndex + 1;
				}

				if (m_nNextStageIndex >= m_tRunStageInfo.m_nRunStageCount)
				{
					m_nNextStageIndex = 0;
				}

				if (m_tPreemptCtlCmd.m_wPreemptDuration > 0)
				{
					m_nReturnAutoCtrlStageIndex = nStageIndexTarget;
				}
				else
				{
					m_nReturnAutoCtrlStageIndex = -1;
				}

				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "SetPreemptCtlCmd, PreemptPhaseID:%d TargetStageIndex:%d", m_tPreemptCtlCmd.m_byPreemptPhaseID, m_tPreemptCtlCmd.m_byPreemptStageIndex);
			}
			else
			{
				if (m_tNextPreemptCtlCmd.m_nCmdSource == CTL_SOURCE_PREEMPT)
				{
					m_pOpenATCRunStatus->SetPreemptCtlCmd(m_tNextPreemptCtlCmd);//COpenATCLogicCtlManager可以根据这个命令调用自主函数回到自主
				}

				m_nManualCurStatus = MANUAL_STAGE_TRANS;

				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "SetPreemptCtlCmd, PatternInterruptCmd ControlMode:%d PatternNo:%d", m_tNextPreemptCtlCmd.m_tPatternInterruptCmd.m_nControlMode, m_tNextPreemptCtlCmd.m_tPatternInterruptCmd.m_nPatternNo);
			}

			memset(&m_tNextPreemptCtlCmd,0,sizeof(TPreemptCtlCmd));
			m_tNextPreemptCtlCmd.m_byPreemptType = PREEMPT_TYPE_DEFAULT;
		}
	}

	if (!m_tPreemptCtlCmd.m_bPatternInterruptCmd && nStageIndex == m_tPreemptCtlCmd.m_byPreemptStageIndex)
	{
		m_bPreemptCtlStageProcessFlag[m_tPreemptCtlCmd.m_byPreemptStageIndex] = true;//此时置开始处理标志
		m_tPreemptCtlCmd.m_byPreemptSwitchFlag = 1;//设置优先相位开始切换标志
		m_pOpenATCRunStatus->ModifyPreemptSwitchFlag(m_tPreemptCtlCmd.m_nCmdSource, false, m_tPreemptCtlCmd.m_byPreemptType, m_tPreemptCtlCmd.m_byPreemptPhaseID, m_tPreemptCtlCmd.m_bIncludeConcurPhase);
		m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "Switch Stage Succeed, PreemptPhaseID:%d TargetStageIndex:%d Start Switch", m_tPreemptCtlCmd.m_byPreemptPhaseID, nStageIndex);
	}
}

/*==================================================================== 
函数名 ：GetPreemptCtlPhaseStage
功能 ：获取优先控制相位的灯色
算法实现 ： 
参数说明 ： nPhaseID，相位ID
返回值说明：无
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 李永萍          创建函数 
====================================================================*/
char CLogicCtlPreempt::GetPreemptCtlPhaseStage(int nPhaseID)
{
	char chPhaseStage = C_CH_PHASESTAGE_U;
	for (int nRingIndex = 0; nRingIndex < m_tFixTimeCtlInfo.m_nRingCount; nRingIndex++)
	{
		PTRingCtlInfo ptRingRunInfo = &(m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex]);
		for (int nPhaseIndex = 0;nPhaseIndex < ptRingRunInfo->m_nPhaseCount;nPhaseIndex++)
		{
			if (ptRingRunInfo->m_atPhaseInfo[nPhaseIndex].m_tPhaseParam.m_byPhaseNumber == nPhaseID)
			{
				chPhaseStage =  ptRingRunInfo->m_atPhaseInfo[nPhaseIndex].m_chPhaseStage;
				break;
			}
		}
	}
	return chPhaseStage;
}


/*==================================================================== 
函数名 ：SetOverlapCurPreemptPhaseLampClrChgFlag
功能 ：设置跟随相位的母相位是优先相位的灯色变化标志
算法实现 ： 
参数说明 ： 无
返回值说明：无
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 李永萍          创建函数 
====================================================================*/
void CLogicCtlPreempt::SetOverlapCurPreemptPhaseLampClrChgFlag()
{
	if (m_nReturnAutoCtrlStageIndex == m_tFixTimeCtlInfo.m_nCurStageIndex && m_tPreemptCtlCmd.m_byPreemptSwitchFlag == 1)
	{
		static int  nOverlapGlobalCounter[MAX_OVERLAP_COUNT] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
		static int  nPedOverlapGlobalCounter[MAX_OVERLAP_COUNT] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

		static bool bOverlapChgToFlashFlag[MAX_OVERLAP_COUNT] = {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false};
		static bool bOverlapChgToYellowFlag[MAX_OVERLAP_COUNT] = {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false};
		static bool bPedOverlapChgToFlashFlag[MAX_OVERLAP_COUNT] = {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false};
		
		int nTempPhaseID = 0;
		if (m_tPreemptCtlCmd.m_byPreemptPhaseID > 0)
		{
			for (int i = 0;i < m_tRunStageInfo.m_PhaseRunstageInfo[m_nNextStageIndex].m_nConcurrencyPhaseCount;i++)
			{
				if (m_tRunStageInfo.m_PhaseRunstageInfo[m_tPreemptCtlCmd.m_byPreemptStageIndex].m_nConcurrencyPhase[i] != 
					m_tPreemptCtlCmd.m_byPreemptPhaseID)
				{
					nTempPhaseID = m_tRunStageInfo.m_PhaseRunstageInfo[m_tPreemptCtlCmd.m_byPreemptStageIndex].m_nConcurrencyPhase[i];
					break;
				}
			}
		}

		int  nRingIndex = 0;
		int  nPhaseIndex = 0;
		int  nGreenFlashTime = 0;
		int  nYellowChangeTime = 0;
		int  nPedGreenFlashTime = 0;
		int  i = 0, j = 0;
		for (i = 0;i < m_tFixTimeCtlInfo.m_nOverlapCount;i ++)
		{
			// 先处理跟随机动车相位
			for (j = 0;j < MAX_PHASE_COUNT_IN_OVERLAP;j ++)
			{
				if (m_tPreemptCtlCmd.m_byPreemptPhaseID == m_tFixTimeCtlInfo.m_atOverlapInfo[i].m_tOverlapParam.m_byArrOverlapIncludedPhases[j] ||
				    nTempPhaseID == m_tFixTimeCtlInfo.m_atOverlapInfo[i].m_tOverlapParam.m_byArrOverlapIncludedPhases[j])
				{
					GetPhaseIndexByPhaseNumber(nRingIndex, nPhaseIndex, (int)m_tFixTimeCtlInfo.m_atOverlapInfo[i].m_tOverlapParam.m_byArrOverlapIncludedPhases[j]);

					nGreenFlashTime = m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_tPhaseParam.m_byGreenFlash;
					nYellowChangeTime = m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_tPhaseParam.m_byPhaseYellowChange;
					nPedGreenFlashTime = m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_tPhaseParam.m_byPhasePedestrianClear;

					//机动车跟随相位
					if (m_tFixTimeCtlInfo.m_atOverlapInfo[i].m_chOverlapStage == C_CH_PHASESTAGE_G)
					{
						nOverlapGlobalCounter[i] = m_pOpenATCRunStatus->GetGlobalCounter();
						bOverlapChgToFlashFlag[i] = true;
						//m_bOverlapChgFlag[i] = true;
						m_bIsLampClrChg = true;
						m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "SetPreemptOverlapPhaseLampClr To GreenFlash OverlapPhase:%d: IncludedPhase:%d", m_tFixTimeCtlInfo.m_atOverlapInfo[i].m_tOverlapParam.m_byOverlapNumber, m_tFixTimeCtlInfo.m_atOverlapInfo[i].m_tOverlapParam.m_byArrOverlapIncludedPhases[j]);
					}
					if (m_tFixTimeCtlInfo.m_atOverlapInfo[i].m_chOverlapStage == C_CH_PHASESTAGE_GF)
					{
						//if (bOverlapChgToFlashFlag[i])
						{
							unsigned long nCounterDiff = CalcCounter(nOverlapGlobalCounter[i], m_pOpenATCRunStatus->GetGlobalCounter(), C_N_MAXGLOBALCOUNTER);
							if (nCounterDiff >= (nGreenFlashTime * C_N_TIMER_TIMER_COUNTER))
							{
								nOverlapGlobalCounter[i] = m_pOpenATCRunStatus->GetGlobalCounter();
								bOverlapChgToFlashFlag[i] = false;
								bOverlapChgToYellowFlag[i] = true;
								m_bOverlapChgFlag[i] = true;
								m_bIsLampClrChg = true;
								m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "SetPreemptOverlapPhaseLampClr To Yellow OverlapPhase:%d: IncludedPhase:%d", m_tFixTimeCtlInfo.m_atOverlapInfo[i].m_tOverlapParam.m_byOverlapNumber, m_tFixTimeCtlInfo.m_atOverlapInfo[i].m_tOverlapParam.m_byArrOverlapIncludedPhases[j]);
							}
						}
					}
					if (m_tFixTimeCtlInfo.m_atOverlapInfo[i].m_chOverlapStage == C_CH_PHASESTAGE_Y)
					{
						//if (bOverlapChgToYellowFlag[i])
						{
							unsigned long nCounterDiff = CalcCounter(nOverlapGlobalCounter[i], m_pOpenATCRunStatus->GetGlobalCounter(), C_N_MAXGLOBALCOUNTER);
							if (nCounterDiff >= (nYellowChangeTime * C_N_TIMER_TIMER_COUNTER))
							{
								bOverlapChgToYellowFlag[i] = false;
								m_bOverlapChgFlag[i] = true;
								m_bIsLampClrChg = true;
								m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "SetPreemptOverlapPhaseLampClr To Red OverlapPhase:%d: IncludedPhase:%d", m_tFixTimeCtlInfo.m_atOverlapInfo[i].m_tOverlapParam.m_byOverlapNumber, m_tFixTimeCtlInfo.m_atOverlapInfo[i].m_tOverlapParam.m_byArrOverlapIncludedPhases[j]);
							}
						}
					}
					//行人跟随相位
					if (m_tFixTimeCtlInfo.m_atOverlapInfo[i].m_chPedOverlapStage == C_CH_PHASESTAGE_G)
					{
						nPedOverlapGlobalCounter[i] = m_pOpenATCRunStatus->GetGlobalCounter();
						bPedOverlapChgToFlashFlag[i] = true;
						//m_bPedOverlapChgFlag[i] = true;
						m_bIsLampClrChg = true;
						m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "SetPreemptPedOverlapPhaseLampClr To GreenFlash OverlapPhase:%d: IncludedPhase:%d", m_tFixTimeCtlInfo.m_atOverlapInfo[i].m_tOverlapParam.m_byOverlapNumber, m_tFixTimeCtlInfo.m_atOverlapInfo[i].m_tOverlapParam.m_byArrOverlapIncludedPhases[j]);
					}

					if (m_tFixTimeCtlInfo.m_atOverlapInfo[i].m_chPedOverlapStage == C_CH_PHASESTAGE_GF)
					{
						//if (bPedOverlapChgToFlashFlag[i])
						{
							unsigned long nCounterDiff = CalcCounter(nPedOverlapGlobalCounter[i], m_pOpenATCRunStatus->GetGlobalCounter(), C_N_MAXGLOBALCOUNTER);
							if (nCounterDiff >= (nPedGreenFlashTime * C_N_TIMER_TIMER_COUNTER))
							{
								bPedOverlapChgToFlashFlag[i] = false;
								m_bPedOverlapChgFlag[i] = true;
								m_bIsLampClrChg = true;
								m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "SetPreemptOverlapPhaseLampClr To Red OverlapPhase:%d: IncludedPhase:%d", m_tFixTimeCtlInfo.m_atOverlapInfo[i].m_tOverlapParam.m_byOverlapNumber, m_tFixTimeCtlInfo.m_atOverlapInfo[i].m_tOverlapParam.m_byArrOverlapIncludedPhases[j]);
							}
						}
					}
				}
			}
		}
	}
	else
	{
		if (m_bPreemptCtlStageProcessFlag[m_tPreemptCtlCmd.m_byPreemptStageIndex]  && !m_bPreemptCtlCmdEndFlag)
		{
			int  nRingIndex = 0;
			int  nPhaseIndex = 0;
			int  i = 0, j = 0;
			for (i = 0;i < m_tFixTimeCtlInfo.m_nOverlapCount;i ++)
			{
				// 先处理跟随机动车相位
				for (j = 0;j < MAX_PHASE_COUNT_IN_OVERLAP;j ++)
				{
					if (m_tFixTimeCtlInfo.m_atOverlapInfo[i].m_tOverlapParam.m_byArrOverlapIncludedPhases[j] > 0 && m_tFixTimeCtlInfo.m_atOverlapInfo[i].m_chOverlapStage != C_CH_PHASESTAGE_R)
					{
						bool bFlag = false;//跟随相位是否需要立刻随着当前主相位过渡结束标志
						if (m_tPreemptCtlCmd.m_byPreemptType == PREEMPT_TYPE_URGENT)
						{
							bFlag = true;//紧急控制时需要立刻随着当前主相位过渡结束
						}
						else
						{
							if (m_tFixTimeCtlInfo.m_nCurStageIndex > m_tPreemptCtlCmd.m_byPreemptStageIndex)
							{
								if ((m_tFixTimeCtlInfo.m_nCurStageIndex + 1) == m_tRunStageInfo.m_nRunStageCount)
								{
									if (m_tPreemptCtlCmd.m_byPreemptStageIndex == 0)
									{
										bFlag = true;//优先控制的阶段是当前阶段的下一个阶段需要立刻随着当前主相位过渡结束
									}
								}
							}
							else
							{
								if ((m_tFixTimeCtlInfo.m_nCurStageIndex + 1) == m_tPreemptCtlCmd.m_byPreemptStageIndex)
								{
									bFlag = true;//优先控制的阶段是当前阶段的下一个阶段需要立刻随着当前主相位过渡结束
								}
							}
						}

						GetPhaseIndexByPhaseNumber(nRingIndex, nPhaseIndex, (int)m_tFixTimeCtlInfo.m_atOverlapInfo[i].m_tOverlapParam.m_byArrOverlapIncludedPhases[j]);

						if (bFlag)
						{
							if (m_tFixTimeCtlInfo.m_nCurPhaseIndex[nRingIndex] == nPhaseIndex || m_tFixTimeCtlInfo.m_nCurStageIndex == m_tPreemptCtlCmd.m_byPreemptStageIndex)
							{
								if (m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_chPhaseStage == C_CH_PHASESTAGE_R || 
									m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_chPhaseStage == C_CH_PHASESTAGE_F)
								{
									if (m_tFixTimeCtlInfo.m_atOverlapInfo[i].m_chOverlapStage != C_CH_PHASESTAGE_R ||
										m_tFixTimeCtlInfo.m_atOverlapInfo[i].m_chPedOverlapStage != C_CH_PHASESTAGE_R)
									{
										m_tFixTimeCtlInfo.m_atOverlapInfo[i].m_chOverlapStage = C_CH_PHASESTAGE_R;
										m_tFixTimeCtlInfo.m_atOverlapInfo[i].m_chPedOverlapStage = C_CH_PHASESTAGE_R;
										m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "11111111 SetPreemptOverlapPhaseLampClr To %c OverlapPhase:%d: IncludedPhase:%d", m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_chPhaseStage, m_tFixTimeCtlInfo.m_atOverlapInfo[i].m_tOverlapParam.m_byOverlapNumber, m_tFixTimeCtlInfo.m_atOverlapInfo[i].m_tOverlapParam.m_byArrOverlapIncludedPhases[j]);
									}
								}
							}
						}
					}
				}
			}
		}
	}
}

/*==================================================================== 
函数名 ：SetOverlapCurPreemptPhaseLampClrChgFlag
功能 ：设置跟随相位的母相位是优先相位的灯色
算法实现 ： 
参数说明 ： 无
返回值说明：无
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 李永萍          创建函数 
====================================================================*/
void CLogicCtlPreempt::SetOverlapCurPreemptPhaseLampClr(int nOverlapIndex, BYTE byOverlapNum, bool bPedOverlapPhase)
{
	if (!bPedOverlapPhase)
	{
		static int  nOverlapGlobalCounter[MAX_OVERLAP_COUNT] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	    
		if (m_tFixTimeCtlInfo.m_atOverlapInfo[nOverlapIndex].m_chOverlapStage == C_CH_PHASESTAGE_G)
		{
			int nGreenFlashTime = 0;
			for (int k = 0;k < m_nChannelCount;k++)
			{
				if (m_atChannelInfo[k].m_byChannelNumber == 0)
				{
					continue;
				}

				if ((int)m_atChannelInfo[k].m_byChannelControlSource == byOverlapNum)
				{
					if (m_atChannelInfo[k].m_byChannelControlType == OVERLAP_CHA)
					{
						int nRingIndex = 0, nPhaseIndex = 0;
						for (int i = 0;i < MAX_PHASE_COUNT_IN_OVERLAP;i++)
						{
							BYTE byMainPhaseNum = m_tFixTimeCtlInfo.m_atOverlapInfo[nOverlapIndex].m_tOverlapParam.m_byArrOverlapIncludedPhases[i];
							if (byMainPhaseNum > 0)
							{
								if (GetPhaseStatus(byMainPhaseNum, false) == C_CH_PHASESTAGE_G || GetPhaseStatus(byMainPhaseNum, false) == C_CH_PHASESTAGE_GF)
								{
									GetPhaseIndexByPhaseNumber(nRingIndex, nPhaseIndex, (int)byMainPhaseNum);
									nGreenFlashTime = m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_tPhaseParam.m_byGreenFlash;
									m_pOpenATCRunStatus->SetGreenFlashCount((m_atChannelInfo[k].m_byChannelNumber - 1) * 3 + 2,nGreenFlashTime * 2);
									nOverlapGlobalCounter[nOverlapIndex] = m_pOpenATCRunStatus->GetGlobalCounter();
									break;
								}
							}
						}
					}
				} 
			}
			if (nGreenFlashTime > 0)
			{
				m_tFixTimeCtlInfo.m_atOverlapInfo[nOverlapIndex].m_chOverlapStage = C_CH_PHASESTAGE_GF;
			}
			else
			{
				m_tFixTimeCtlInfo.m_atOverlapInfo[nOverlapIndex].m_chOverlapStage = C_CH_PHASESTAGE_Y;
			}
		}
		else if (m_tFixTimeCtlInfo.m_atOverlapInfo[nOverlapIndex].m_chOverlapStage == C_CH_PHASESTAGE_GF)
		{
			if (m_nReturnAutoCtrlStageIndex == m_tFixTimeCtlInfo.m_nCurStageIndex && m_tPreemptCtlCmd.m_byPreemptSwitchFlag == 1)
			{
				if (m_bOverlapChgFlag[nOverlapIndex])
				{
					m_bOverlapChgFlag[nOverlapIndex] = false;
					m_tFixTimeCtlInfo.m_atOverlapInfo[nOverlapIndex].m_chOverlapStage = C_CH_PHASESTAGE_Y;
				}
			}
			else
			{
				int nRingIndex = 0, nPhaseIndex = 0, nGreenFlashTime = 0;
				for (int i = 0;i < MAX_PHASE_COUNT_IN_OVERLAP;i++)
				{
					BYTE byMainPhaseNum = m_tFixTimeCtlInfo.m_atOverlapInfo[nOverlapIndex].m_tOverlapParam.m_byArrOverlapIncludedPhases[i];
					if (byMainPhaseNum > 0)
					{
						if (GetPhaseStatus(byMainPhaseNum, true) == C_CH_PHASESTAGE_G || GetPhaseStatus(byMainPhaseNum, true) == C_CH_PHASESTAGE_GF)
						{
							GetPhaseIndexByPhaseNumber(nRingIndex, nPhaseIndex, (int)byMainPhaseNum);
							nGreenFlashTime = m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_tPhaseParam.m_byPhasePedestrianClear;
							break;
						}
					}
				}

				unsigned long nCounterDiff = CalcCounter(nOverlapGlobalCounter[nOverlapIndex], m_pOpenATCRunStatus->GetGlobalCounter(), C_N_MAXGLOBALCOUNTER);
				if (nCounterDiff >= (nGreenFlashTime * C_N_TIMER_TIMER_COUNTER))
				{
					m_tFixTimeCtlInfo.m_atOverlapInfo[nOverlapIndex].m_chOverlapStage = C_CH_PHASESTAGE_R;
				}
				//m_tFixTimeCtlInfo.m_atOverlapInfo[nOverlapIndex].m_chOverlapStage = C_CH_PHASESTAGE_Y;
			}
		}
		else if (m_tFixTimeCtlInfo.m_atOverlapInfo[nOverlapIndex].m_chOverlapStage == C_CH_PHASESTAGE_Y)
		{
			if (m_nReturnAutoCtrlStageIndex == m_tFixTimeCtlInfo.m_nCurStageIndex && m_tPreemptCtlCmd.m_byPreemptSwitchFlag == 1)
			{
				if (m_bOverlapChgFlag[nOverlapIndex])
				{
					m_bOverlapChgFlag[nOverlapIndex] = false;
					m_tFixTimeCtlInfo.m_atOverlapInfo[nOverlapIndex].m_chOverlapStage = C_CH_PHASESTAGE_R;
				}
			}
			else
			{
				m_tFixTimeCtlInfo.m_atOverlapInfo[nOverlapIndex].m_chOverlapStage = C_CH_PHASESTAGE_R;
			}
		}
	}
	else
	{
		static int  nPedOverlapGlobalCounter[MAX_OVERLAP_COUNT] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

	    if (m_tFixTimeCtlInfo.m_atOverlapInfo[nOverlapIndex].m_chPedOverlapStage == C_CH_PHASESTAGE_G)
		{
			int nPedGreenFlashTime = 0;
			for (int k = 0;k < m_nChannelCount;k++)
			{
				if (m_atChannelInfo[k].m_byChannelNumber == 0)
				{
					continue;
				}

				if ((int)m_atChannelInfo[k].m_byChannelControlSource == byOverlapNum)
				{
					if (m_atChannelInfo[k].m_byChannelControlType == OVERLAP_PED_CHA)
					{
						int nRingIndex = 0, nPhaseIndex = 0;
						for (int i = 0;i < MAX_PHASE_COUNT_IN_OVERLAP;i++)
						{
							BYTE byMainPhaseNum = m_tFixTimeCtlInfo.m_atOverlapInfo[nOverlapIndex].m_tOverlapParam.m_byArrOverlapIncludedPhases[i];
							if (byMainPhaseNum > 0)
							{
								if (GetPhaseStatus(byMainPhaseNum, true) == C_CH_PHASESTAGE_G || GetPhaseStatus(byMainPhaseNum, true) == C_CH_PHASESTAGE_GF)
								{
									GetPhaseIndexByPhaseNumber(nRingIndex, nPhaseIndex, (int)byMainPhaseNum);
									nPedGreenFlashTime = m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_tPhaseParam.m_byPhasePedestrianClear;
									m_pOpenATCRunStatus->SetGreenFlashCount((m_atChannelInfo[k].m_byChannelNumber - 1) * 3 + 2,nPedGreenFlashTime * 2);
									nPedOverlapGlobalCounter[nOverlapIndex] = m_pOpenATCRunStatus->GetGlobalCounter();
									break;
								}
							}
						}
					}
				} 
			}

			if (nPedGreenFlashTime > 0)
			{
				m_tFixTimeCtlInfo.m_atOverlapInfo[nOverlapIndex].m_chPedOverlapStage = C_CH_PHASESTAGE_GF;
			}
			else
			{
				m_tFixTimeCtlInfo.m_atOverlapInfo[nOverlapIndex].m_chPedOverlapStage = C_CH_PHASESTAGE_R;
			}
		}
		else if (m_tFixTimeCtlInfo.m_atOverlapInfo[nOverlapIndex].m_chPedOverlapStage == C_CH_PHASESTAGE_GF)
		{
			if (m_nReturnAutoCtrlStageIndex == m_tFixTimeCtlInfo.m_nCurStageIndex && m_tPreemptCtlCmd.m_byPreemptSwitchFlag == 1)
			{
				if (m_bPedOverlapChgFlag[nOverlapIndex])
				{
					m_bPedOverlapChgFlag[nOverlapIndex] = false;
					m_tFixTimeCtlInfo.m_atOverlapInfo[nOverlapIndex].m_chPedOverlapStage = C_CH_PHASESTAGE_R;
				}
			}
			else
			{
			    int nRingIndex = 0, nPhaseIndex = 0, nPedGreenFlashTime = 0;
				for (int i = 0;i < MAX_PHASE_COUNT_IN_OVERLAP;i++)
				{
					BYTE byMainPhaseNum = m_tFixTimeCtlInfo.m_atOverlapInfo[nOverlapIndex].m_tOverlapParam.m_byArrOverlapIncludedPhases[i];
					if (byMainPhaseNum > 0)
					{
						if (GetPhaseStatus(byMainPhaseNum, true) == C_CH_PHASESTAGE_G || GetPhaseStatus(byMainPhaseNum, true) == C_CH_PHASESTAGE_GF)
						{
							GetPhaseIndexByPhaseNumber(nRingIndex, nPhaseIndex, (int)byMainPhaseNum);
							nPedGreenFlashTime = m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_tPhaseParam.m_byPhasePedestrianClear;
							break;
						}
					}
				}

				unsigned long nCounterDiff = CalcCounter(nPedOverlapGlobalCounter[nOverlapIndex], m_pOpenATCRunStatus->GetGlobalCounter(), C_N_MAXGLOBALCOUNTER);
				if (nCounterDiff >= (nPedGreenFlashTime * C_N_TIMER_TIMER_COUNTER))
				{
					m_tFixTimeCtlInfo.m_atOverlapInfo[nOverlapIndex].m_chPedOverlapStage = C_CH_PHASESTAGE_R;
				}
				//m_tFixTimeCtlInfo.m_atOverlapInfo[nOverlapIndex].m_chPedOverlapStage = C_CH_PHASESTAGE_R;
			}
		}
	}
}

/*==================================================================== 
函数名 ：IsNeglectPhase
功能 ：判断当前相位是否是忽略相位或关断相位
算法实现 ： 
参数说明 ：nPhaseID，相位ID
返回值说明：无
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 李永萍          创建函数 
====================================================================*/ 
bool CLogicCtlPreempt::IsNeglectPhase(int nPhaseID)
{
	int  i = 0, j = 0;
	for (i = 0;i < m_tFixTimeCtlInfo.m_nRingCount;i ++)
    {
        PTRingCtlInfo ptRingRunInfo = &(m_tFixTimeCtlInfo.m_atPhaseSeq[i]);
        
        for (j = 0;j < ptRingRunInfo->m_nPhaseCount;j ++)
        {
            if (nPhaseID == (int)(ptRingRunInfo->m_atPhaseInfo[j].m_tPhaseParam.m_byPhaseNumber) &&
			   (ptRingRunInfo->m_atPhaseInfo[j].m_chPhaseMode == NEGLECT_MODE || 
				ptRingRunInfo->m_atPhaseInfo[j].m_chPhaseMode == SHIELD_MODE))
            {
                return true;
            }
        }            
    }

	return false;
}

/*==================================================================== 
函数名 ：SetNeglectPhaseRunTime
功能 ：设置忽略相位和关断相位的运行时间
算法实现 ： 
参数说明 ：nRingIndex，环索引  bPedPhase，行人相位标志
返回值说明：运行时间
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 李永萍          创建函数 
====================================================================*/ 
int CLogicCtlPreempt::SetNeglectPhaseRunTime(int nRingIndex, bool bPedPhase)
{
	WORD wStageTime = 0;

	if (m_nManualCurStatus == MANUAL_STAGE_TRANS)
	{
		wStageTime = m_tRunStageInfo.m_PhaseRunstageInfo[m_tFixTimeCtlInfo.m_nCurStageIndex].m_nStageEndTime - m_tRunStageInfo.m_PhaseRunstageInfo[m_tFixTimeCtlInfo.m_nCurStageIndex].m_nStageStartTime;
	}
	else
	{
		for (int i = 0;i < m_tFixTimeCtlInfo.m_nRingCount;i++)
		{
			if (i != nRingIndex)
			{
				PTRingCtlInfo ptRingRunInfo = &(m_tFixTimeCtlInfo.m_atPhaseSeq[i]);
                int nIndex = m_tFixTimeCtlInfo.m_nCurPhaseIndex[i];

				if (!m_tRunStageInfo.m_PhaseRunstageInfo[m_tFixTimeCtlInfo.m_nCurStageIndex].m_bDirectTransit[i])
				{
					if (!bPedPhase)
					{
						if (!m_tRunStageInfo.m_PhaseRunstageInfo[m_tFixTimeCtlInfo.m_nCurStageIndex].m_bPhaseStartTransitInCurStage[i])
						{
							wStageTime = ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPhaseGreenTime;
						}
						else
						{
							wStageTime = ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byGreenFlash +
								ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhaseYellowChange +
								ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhaseRedClear;
						}
					}
					else
					{
						if (!m_tRunStageInfo.m_PhaseRunstageInfo[m_tFixTimeCtlInfo.m_nCurStageIndex].m_bPedPhaseStartTransitInCurStage[i])
						{
							wStageTime = ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPedPhaseGreenTime;
						}
						else
						{
							wStageTime = ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhaseYellowChange +
								ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhasePedestrianClear +
								ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhaseRedClear;
						}
					}
				}
				else
				{
					if (!bPedPhase)
					{
						wStageTime = ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPhaseGreenTime + 
							ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byGreenFlash +
							ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhaseYellowChange +
							ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhaseRedClear;
					}
					else
					{
						wStageTime = ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPedPhaseGreenTime + 
							ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhaseYellowChange +
							ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhasePedestrianClear +
							ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhaseRedClear;
					}
				}
			}
		}
	}

	return  wStageTime;
}

/*==================================================================== 
函数名 ：ReSetNeglectPhaseStageRunTime
功能 ：重新设置忽略相位和关断相位的当前阶段时间
算法实现 ： 
参数说明 ：无
返回值说明：无
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 李永萍          创建函数 
====================================================================*/ 
void CLogicCtlPreempt::ReSetNeglectPhaseStageRunTime()
{
	int m_nLampClrTime = 0; 
	int m_nPedLampClrTime = 0;

	TPhaseLampClrRunCounter tRunCounter;
	m_pOpenATCRunStatus->GetPhaseLampClrRunCounter(tRunCounter);

	int  i = 0;
	for (i = 0;i < m_tFixTimeCtlInfo.m_nRingCount;i++)
	{
		if (m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[m_tFixTimeCtlInfo.m_nCurPhaseIndex[i]].m_chPhaseMode == NEGLECT_MODE || 
			m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[m_tFixTimeCtlInfo.m_nCurPhaseIndex[i]].m_chPhaseMode == SHIELD_MODE)
		{
			if (i == 0)
			{
				PTRingCtlInfo ptRingRunInfo = &(m_tFixTimeCtlInfo.m_atPhaseSeq[1]);
				int nIndex = m_tFixTimeCtlInfo.m_nCurPhaseIndex[1];
				PTPhaseCtlInfo pPhaseInfo = (PTPhaseCtlInfo)&(ptRingRunInfo->m_atPhaseInfo[nIndex]);

				m_nLampClrTime = ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPhaseGreenTime;
				if (ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseStage == C_CH_PHASESTAGE_G &&
					ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPhaseGreenTime > tRunCounter.m_nLampClrTime[1])
				{
					m_nLampClrTime = tRunCounter.m_nLampClrTime[1];
				}

				m_nPedLampClrTime = ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPedPhaseGreenTime;
				if (ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPedPhaseStage == C_CH_PHASESTAGE_G &&
					ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPedPhaseGreenTime > tRunCounter.m_nPedLampClrTime[1])
				{
					m_nPedLampClrTime = tRunCounter.m_nPedLampClrTime[1];
				}
			
				if (!m_tRunStageInfo.m_PhaseRunstageInfo[m_tFixTimeCtlInfo.m_nCurStageIndex].m_bDirectTransit[1])
				{
					if (!m_tRunStageInfo.m_PhaseRunstageInfo[m_tFixTimeCtlInfo.m_nCurStageIndex].m_bPhaseStartTransitInCurStage[1])
					{
						m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[m_tFixTimeCtlInfo.m_nCurPhaseIndex[i]].m_wPhaseStageRunTime = 
								ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPhaseGreenTime + 
								ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byGreenFlash +
								ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhaseYellowChange +
								ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhaseRedClear - m_nLampClrTime;

						m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[m_tFixTimeCtlInfo.m_nCurPhaseIndex[i]].m_wPedPhaseStageRunTime = 
								ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPedPhaseGreenTime + 
								ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhaseYellowChange +
								ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhasePedestrianClear +
								ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhaseRedClear - m_nPedLampClrTime;
					}
					else
					{
						m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[m_tFixTimeCtlInfo.m_nCurPhaseIndex[i]].m_wPhaseStageRunTime = 
							    ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byGreenFlash +
							    ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhaseYellowChange +
							    ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhaseRedClear;

						m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[m_tFixTimeCtlInfo.m_nCurPhaseIndex[i]].m_wPedPhaseStageRunTime = 
								ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhaseYellowChange +
								ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhasePedestrianClear +
								ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhaseRedClear;
					}
				}
				else
				{
					m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[m_tFixTimeCtlInfo.m_nCurPhaseIndex[i]].m_wPhaseStageRunTime = 
							ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPhaseGreenTime + 
							ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byGreenFlash +
							ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhaseYellowChange +
							ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhaseRedClear - m_nLampClrTime;

					m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[m_tFixTimeCtlInfo.m_nCurPhaseIndex[i]].m_wPedPhaseStageRunTime = 
							ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPedPhaseGreenTime + 
							ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhaseYellowChange +
							ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhasePedestrianClear +
							ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhaseRedClear - m_nPedLampClrTime;
				}

			}
			else if (i == 1)
			{
				PTRingCtlInfo ptRingRunInfo = &(m_tFixTimeCtlInfo.m_atPhaseSeq[0]);
				int nIndex = m_tFixTimeCtlInfo.m_nCurPhaseIndex[0];
				PTPhaseCtlInfo pPhaseInfo = (PTPhaseCtlInfo)&(ptRingRunInfo->m_atPhaseInfo[nIndex]);

				m_nLampClrTime = ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPhaseGreenTime;
				if (ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseStage == C_CH_PHASESTAGE_G &&
					ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPhaseGreenTime > tRunCounter.m_nLampClrTime[0])
				{
					m_nLampClrTime = tRunCounter.m_nLampClrTime[0];
				}

				m_nPedLampClrTime = ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPedPhaseGreenTime;
				if (ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPedPhaseStage == C_CH_PHASESTAGE_G &&
					ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPedPhaseGreenTime > tRunCounter.m_nPedLampClrTime[0])
				{
					m_nPedLampClrTime = tRunCounter.m_nPedLampClrTime[0];
				}
			
				if (!m_tRunStageInfo.m_PhaseRunstageInfo[m_tFixTimeCtlInfo.m_nCurStageIndex].m_bDirectTransit[0])
				{
					if (!m_tRunStageInfo.m_PhaseRunstageInfo[m_tFixTimeCtlInfo.m_nCurStageIndex].m_bPhaseStartTransitInCurStage[0])
					{
						m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[m_tFixTimeCtlInfo.m_nCurPhaseIndex[i]].m_wPhaseStageRunTime = 
								ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPhaseGreenTime + 
								ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byGreenFlash +
								ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhaseYellowChange +
								ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhaseRedClear - m_nLampClrTime;

						m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[m_tFixTimeCtlInfo.m_nCurPhaseIndex[i]].m_wPedPhaseStageRunTime = 
								ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPedPhaseGreenTime + 
								ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhaseYellowChange +
								ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhasePedestrianClear +
								ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhaseRedClear - m_nPedLampClrTime;
					}
					else
					{
						m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[m_tFixTimeCtlInfo.m_nCurPhaseIndex[i]].m_wPhaseStageRunTime = 
							    ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byGreenFlash +
							    ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhaseYellowChange +
							    ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhaseRedClear;

						m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[m_tFixTimeCtlInfo.m_nCurPhaseIndex[i]].m_wPedPhaseStageRunTime = 
								ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhaseYellowChange +
								ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhasePedestrianClear +
								ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhaseRedClear;
					}
				}
				else
				{
					m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[m_tFixTimeCtlInfo.m_nCurPhaseIndex[i]].m_wPhaseStageRunTime = 
							ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPhaseGreenTime + 
							ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byGreenFlash +
							ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhaseYellowChange +
							ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhaseRedClear - m_nLampClrTime;

					m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[m_tFixTimeCtlInfo.m_nCurPhaseIndex[i]].m_wPedPhaseStageRunTime = 
							ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPedPhaseGreenTime + 
							ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhaseYellowChange +
							ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhasePedestrianClear +
							ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhaseRedClear - m_nPedLampClrTime;
				}

			}

			if (m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[m_tFixTimeCtlInfo.m_nCurPhaseIndex[i]].m_wPhaseStageRunTime < 0)
			{
				m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[m_tFixTimeCtlInfo.m_nCurPhaseIndex[i]].m_wPhaseStageRunTime = 0;
			}
			if (m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[m_tFixTimeCtlInfo.m_nCurPhaseIndex[i]].m_wPedPhaseStageRunTime < 0)
			{
				m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[m_tFixTimeCtlInfo.m_nCurPhaseIndex[i]].m_wPedPhaseStageRunTime = 0;
			}

			m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "ReSetNeglectPhaseStageRunTime RingIndex:%d PhasePassTime:%d.", i,m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[m_tFixTimeCtlInfo.m_nCurPhaseIndex[i]].m_wPhaseStageRunTime);
		}
	}
}

/*==================================================================== 
函数名 ：ReSetNeglectPhasetBackUpTime
功能 ：重新设置忽略相位和关断相位的备份时间
算法实现 ： 
参数说明 ：无
返回值说明：无
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 李永萍          创建函数 
====================================================================*/ 
void CLogicCtlPreempt::ReSetNeglectPhasetBackUpTime()
{
	int  i = 0;
	for (i = 0;i < m_tFixTimeCtlInfo.m_nRingCount;i++)
	{
		PTRingCtlInfo ptRingRunInfo = &(m_tFixTimeCtlInfo.m_atPhaseSeq[i]);
		int nIndex = m_tFixTimeCtlInfo.m_nCurPhaseIndex[i];
		PTPhaseCtlInfo pPhaseInfo = (PTPhaseCtlInfo)&(ptRingRunInfo->m_atPhaseInfo[nIndex]);
		int nPhaseID = pPhaseInfo->m_tPhaseParam.m_byPhaseNumber;

		if (pPhaseInfo->m_chPhaseMode == NEGLECT_MODE || pPhaseInfo->m_chPhaseMode == SHIELD_MODE)
		{
			if (i == 0)
			{
				m_nStageTimeForPhasePass[0] = m_nStageTimeForPhasePass[1];
			}
			else if (i == 1)
			{
				m_nStageTimeForPhasePass[1] = m_nStageTimeForPhasePass[0];
			}

			m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "ReSetNeglectPhasetBackUpTime RingIndex:%d PhasePassTime:%d.", i, m_nStageTimeForPhasePass[i]);
		}
	}
}