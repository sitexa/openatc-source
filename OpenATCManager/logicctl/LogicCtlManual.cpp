/*=====================================================================
模块名 ：手动控制方式接口模块
文件名 ：LogicCtlManual.cpp
相关文件：LogicCtlManual.h,LogicCtlFixedTime.h
实现功能：手动控制方式实现
作者 ：刘黎明
版权 ：<Copyright(c) 2019-2020 Suzhou Keda Technology Co.,Ltd. All right reserved.>
-----------------------------------------------------------------------
修改记录：
日 期           版本     修改人     走读人     修改记录
2019/9/14       V1.0     刘黎明     王 五      创建模块
2019/2/28       V1.0     李永萍     刘黎明      修改系统用户控制逻辑
2019/3/28       V1.0     李永萍     刘黎明      增加手动面板按钮切换处理
=====================================================================*/
#include "LogicCtlManual.h"
#include <string.h>

CLogicCtlManual::CLogicCtlManual()
{

}

CLogicCtlManual::~CLogicCtlManual()
{

}

/*==================================================================== 
函数名 ：Init 
功能 ：手动控制方式类资源初始化
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
void CLogicCtlManual::Init(COpenATCParameter * pParameter, COpenATCRunStatus * pRunStatus, COpenATCLog * pOpenATCLog, int nPlanNo)
{
    m_pOpenATCParameter = pParameter;
    m_pOpenATCRunStatus = pRunStatus;
	m_pOpenATCLog       = pOpenATCLog;
    memset(&m_tFixTimeCtlInfo,0,sizeof(m_tFixTimeCtlInfo));

	//当前运行模式为手动控制
	m_nCurRunMode = CTL_MODE_MANUAL;

    TLogicCtlStatus tCtlStatus;
    m_pOpenATCRunStatus->GetLogicCtlStatus(tCtlStatus);
    tCtlStatus.m_nCurCtlMode = CTL_MODE_MANUAL;
    m_pOpenATCRunStatus->SetLogicCtlStatus(tCtlStatus);

    InitChannelParam();
    
	for (int i = 0;i < MAX_CHANNEL_COUNT;i++)
    {
        m_bChangeChannelClr[i] = false;
        m_nChannelCounter[i] = 0;
        m_nChannelDurationCounter[i] = 0;
        m_chChannelStatus[i] = C_CH_PHASESTAGE_U;
		m_chChannelStage[i] = C_CH_PHASESTAGE_U;
		m_bChannelTran[i] = false;

		m_nDirectionGreenTime[i] = 0;
		m_nChannelLockGreenTime[i] = 0;
		m_bChannelKeepGreenFlag[i] = false;

        m_nChannelDurationTime[i] = 0;
		m_bKeepGreenChannelBeforeControlChannelFlag[i] = false;
	}
    
	memset(&m_tOldValidManualCmd, 0, sizeof(m_tOldValidManualCmd));
    memset(&m_tOldFixTimeCtlInfo, 0, sizeof(m_tOldFixTimeCtlInfo));
	memset(m_bShieldStatus, 0x00, sizeof(m_bShieldStatus));
	memset(m_bProhibitStatus, 0x00, sizeof(m_bProhibitStatus));
	memset(m_bOldShieldStatus, 0x00, sizeof(m_bOldShieldStatus));
	memset(m_bOldProhibitStatus, 0x00, sizeof(m_bOldProhibitStatus));

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
	m_bTransToAutoFlag = false;

	m_bIsDirectionChannelClrChg = false;

	memset(m_bPhaseColorChgToYellowFlag, 0, sizeof(m_bPhaseColorChgToYellowFlag));
	memset(m_nStageRunTime, 0, sizeof(m_nStageRunTime));
	memset(&m_tPhasePassCmdPhaseStatus, 0, sizeof(m_tPhasePassCmdPhaseStatus));
	memset(m_nStageTimeForPhasePass, 0, sizeof(m_nStageTimeForPhasePass));

	m_bIfPhasePassCmdPhaseStatusHaveRefreshedToInit = false;

	m_bNextClrStageIsFInColorStep = false;
	m_bStageStepwardInColorStep   = false;

	memset(m_tLockPhaseData, 0, sizeof(m_tLockPhaseData));
    memset(m_bTargetLockPhaseChannelFlag, 0, sizeof(m_bTargetLockPhaseChannelFlag));
	memset(m_bNonTargetLockPhaseEndFlag, 0, sizeof(m_bNonTargetLockPhaseEndFlag));

	memset(&m_tNewPhaseLockPara, 0, sizeof(m_tNewPhaseLockPara));

	memset(m_bNeglectChannelBoforePhaseLock, 0, sizeof(m_bNeglectChannelBoforePhaseLock));

	m_bOldLockChannelCmdEndFlag = true;
	memset(&m_tLockPhaseStage, 0, sizeof(m_tLockPhaseStage));
}

/*==================================================================== 
函数名 ：Run 
功能 ：手动控制方式主流程实现
算法实现 ： 
参数说明 ： 
返回值说明：
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 刘黎明          创建函数 
2020/02/18     V1.0 李永萍          增加了方向控制 
====================================================================*/ 
void CLogicCtlManual::Run()
{
	TManualCmd  tValidManualCmd;
	memset(&tValidManualCmd,0,sizeof(tValidManualCmd));
	m_pOpenATCRunStatus->GetValidManualCmd(tValidManualCmd);

	if (tValidManualCmd.m_nSubCtlMode != CTL_MANUAL_SUBMODE_PANEL_DIRECTION && tValidManualCmd.m_nSubCtlMode != CTL_MANUAL_SUBMODE_SYSTEM_CHANNEL_LOCK)
	{
		ProcessPhasePassControlStatus(tValidManualCmd);

		ProcessStepward(tValidManualCmd);
	}
	else 
	{
		if (tValidManualCmd.m_nSubCtlMode == CTL_MANUAL_SUBMODE_PANEL_DIRECTION)
		{
			ProcessDirection(tValidManualCmd);
		}
		else if (tValidManualCmd.m_nSubCtlMode == CTL_MANUAL_SUBMODE_SYSTEM_CHANNEL_LOCK)
		{
			ProcessChannelLock(tValidManualCmd);
		}
	}

	ClearPulse();//清除脉冲

	if (m_bIsLampClrChg || m_bIsDirectionChannelClrChg)
    {
		if (m_bIsLampClrChg && !m_bIsDirectionChannelClrChg)
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
		m_bIsDirectionChannelClrChg = false;
    }

    bool bFlag = false;
	if (tValidManualCmd.m_bPatternInterruptCmd && tValidManualCmd.m_tPatternInterruptCmd.m_nControlMode == CTL_MODE_SELFCTL)
    {   
        bFlag = true;
    }
    
	ManualCycleChg(bFlag);
}

/*==================================================================== 
函数名 ：ManualOnePhaseRun
功能 ：手动控制时单个机动车相位运行状态更新
算法实现 ： 
参数说明 ：nRingIndex：环索引，tValidManualCmd：手动指令
返回值说明：
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 刘黎明          创建函数 
====================================================================*/ 
void CLogicCtlManual::ManualOnePhaseRun(int nRingIndex, TManualCmd tValidManualCmd)
{
	char szInfo[256] = {0};

    PTRingCtlInfo ptRingRunInfo = &(m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex]);
    int nIndex = m_tFixTimeCtlInfo.m_nCurPhaseIndex[nRingIndex];
    PTPhaseCtlInfo pPhaseInfo = (PTPhaseCtlInfo)&(ptRingRunInfo->m_atPhaseInfo[nIndex]);
        
    TPhaseLampClrRunCounter tRunCounter;
    m_pOpenATCRunStatus->GetPhaseLampClrRunCounter(tRunCounter);

	TAscStepCfg tStepCfg;
	memset(&tStepCfg, 0, sizeof(tStepCfg));
	m_pOpenATCParameter->GetStepInfo(tStepCfg);

	TAscManualPanel tAscManualPanel;
    memset(&tAscManualPanel, 0, sizeof(TAscManualPanel)); 
    m_pOpenATCParameter->GetManualPanelInfo(tAscManualPanel);

	bool bFlag = true;
	int nGreenTime = ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPhaseGreenTime;
	if (tStepCfg.m_byStepType == STEP_COLOR)
	{
		if (m_nManualCurStatus == MANUAL_CONTROL_STATUS)
		{
			if (tValidManualCmd.m_tStepForwardCmd.m_nNextStageID == 0)//色步步进
			{
				if (!m_bStageStepwardInColorStep)
				{
					nGreenTime = ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_wPhaseMinimumGreen;//在控制阶段才能走最小绿
				}
				if (!tValidManualCmd.m_bStepForwardCmd && !m_bStageStepwardInColorStep)
				{
					bFlag = false;//色步步进时，在控制阶段，绿灯时间超过最小绿，并且按了步进才能变成绿闪
				}
				if (tValidManualCmd.m_nSubCtlMode == CTL_MANUAL_SUBMODE_PANEL_STEPFOWARD && tAscManualPanel.m_byMinGreenIgnore == 1)//忽略最小绿
				{
					nGreenTime = 0;
				}
			}
		}
	}
	else//阶段步进的控制
	{
		if (tValidManualCmd.m_nSubCtlMode == CTL_MANUAL_SUBMODE_PANEL_STEPFOWARD && tAscManualPanel.m_byMinGreenIgnore == 1)//忽略最小绿
		{
			if (m_nManualCurStatus == MANUAL_CONTROL_STATUS && tValidManualCmd.m_tStepForwardCmd.m_nNextStageID == 0)
			{
				if (tValidManualCmd.m_bStepForwardCmd)
				{
					nGreenTime = 0;
				}
			}
		}
	}

	int nNextStageIndex = tValidManualCmd.m_tStepForwardCmd.m_nNextStageID;
	
    //当前相位阶段是否完成
    if (ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseStage == C_CH_PHASESTAGE_G)
    {
        if (tRunCounter.m_nLampClrTime[nRingIndex] >= nGreenTime && bFlag)
        {
			//跨阶段相位，要保持绿灯状态，运行到跨的最后一个阶段时才能切到绿闪
			if (nNextStageIndex == 0)
			{
				nNextStageIndex = m_tFixTimeCtlInfo.m_nCurStageIndex + 1;
			}
			else
			{
				if (m_tFixTimeCtlInfo.m_nCurStageIndex == nNextStageIndex - 1)
				{
					nNextStageIndex = m_tFixTimeCtlInfo.m_nCurStageIndex + 1;
				}
				else
				{
					nNextStageIndex = nNextStageIndex - 1;
				}
			}
			if (nNextStageIndex >= m_tRunStageInfo.m_nRunStageCount)
			{
				nNextStageIndex = 0;
			}

			//在控制过程中的相位绿灯没有到该相位的最后一个阶段都不切，其他都切
			if (m_nManualCurStatus == MANUAL_CONTROL_STATUS && 
				m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[m_tFixTimeCtlInfo.m_nCurPhaseIndex[nRingIndex]].m_tPhaseParam.m_byPhaseNumber == m_tRunStageInfo.m_PhaseRunstageInfo[nNextStageIndex].m_nConcurrencyPhase[nRingIndex])
			{
				m_tFixTimeCtlInfo.m_bIsChgStage[nRingIndex] = false;
				if (m_tPhasePassCmdPhaseStatus.m_nPhasePassStatus[ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhaseNumber - 1] == PhasePassStatus_Close)
				{
					m_tFixTimeCtlInfo.m_bIsChgStage[nRingIndex] = true;
				}
			}
			else
			{
				m_tFixTimeCtlInfo.m_bIsChgStage[nRingIndex] = true;
			}
		}
    }
    else
    {
		if (tStepCfg.m_byStepType == STEP_STAGE || m_nManualCurStatus == MANUAL_STAGE_TRANS)//阶段步进和过渡状态时，非绿灯色只要大于运行时间就切换灯色
		{
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
				if (m_tPhasePassCmdPhaseStatus.m_nPhasePassStatus[nPhaseID - 1] == PhasePassStatus_Normal)
				{
					if (nRingIndex == 0)
					{
						m_tFixTimeCtlInfo.m_wCycleRunTime += tRunCounter.m_nLampClrTime[nRingIndex];            
					}
					m_tFixTimeCtlInfo.m_bIsChgStage[nRingIndex] = true;
				}
			}
		}
		else //色步步进非绿灯色切换时，在控制阶段，如果是步进，则一次切一次，如果是切阶段，则只要大于运行时间就切换
		{
			bool bSwitch = true;
			if (tStepCfg.m_byStepType == STEP_COLOR)
			{
				if (tValidManualCmd.m_tStepForwardCmd.m_nNextStageID == 0)
				{
					if (!tValidManualCmd.m_bStepForwardCmd)//第一次按面板步进卡在当前灯色
					{
						bSwitch = false;
					}

					//色步模式下切阶段，超过时长切到下一个灯色
					if (m_bStageStepwardInColorStep && tRunCounter.m_nLampClrTime[nRingIndex] >= ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPhaseStageRunTime)
					{
						bSwitch = true;
					}
				}
				else
				{
					if (tRunCounter.m_nLampClrTime[nRingIndex] < ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPhaseStageRunTime)
					{
						 bSwitch = false;
					}
				}

				if (ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseStage == C_CH_PHASESTAGE_U)
				{
					int nPhaseID = ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhaseNumber;
					if (m_tPhasePassCmdPhaseStatus.m_nPhasePassStatus[nPhaseID - 1] == PhasePassStatus_Close)
					{
						bSwitch = false;
					}
				}
			}

			if (bSwitch || (tValidManualCmd.m_bPatternInterruptCmd && tRunCounter.m_nLampClrTime[nRingIndex] >= ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPhaseStageRunTime &&
				            m_tPhasePassCmdPhaseStatus.m_nPhasePassStatus[ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhaseNumber - 1] == PhasePassStatus_Normal))
			{
				if (nRingIndex == 0)
				{
					m_tFixTimeCtlInfo.m_wCycleRunTime += tRunCounter.m_nLampClrTime[nRingIndex];            
				}
				m_tFixTimeCtlInfo.m_bIsChgStage[nRingIndex] = true;
			}
		}
    }

    if (m_tFixTimeCtlInfo.m_bIsChgStage[nRingIndex])
    {   
		if (tStepCfg.m_byStepType == STEP_COLOR)
		{
			//色步步进，行人相位以机动车相位为准
			m_tFixTimeCtlInfo.m_bIsChgPedStage[nRingIndex] = true;    
		}

        char chcurStage = ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseStage;
        WORD wStageTime = 0;
        char chNextStage =  this->GetNextPhaseStageInfo(chcurStage,pPhaseInfo,wStageTime);
		ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseStage = chNextStage;

		if (tStepCfg.m_byStepType == STEP_COLOR && m_nManualCurStatus == MANUAL_CONTROL_STATUS)
		{
			/*//色步步进，灯色从黄切到红时，状态置为可结束，这样再按步进则直接切到绿，否则要多按一次步进
			if (chNextStage == C_CH_PHASESTAGE_R)
			{
				ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseStage = C_CH_PHASESTAGE_F;
			}*/
			m_bNextClrStageIsFInColorStep = false;
			if (chNextStage == C_CH_PHASESTAGE_F)
			{
				m_bNextClrStageIsFInColorStep = true;
			}
		}

		if (chNextStage != C_CH_PHASESTAGE_G && !IsNeedTransBeforeControlChannel(ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhaseNumber))
		{
			wStageTime = 0;
		}

		if ((ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseMode == NEGLECT_MODE || ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseMode == SHIELD_MODE) &&
			 chNextStage == C_CH_PHASESTAGE_R)
		{
			wStageTime = SetNeglectPhaseRunTime(nRingIndex, false);
		}

		// 绿灯时间在RecalculteStepForwardGreenTime函数中已经计算过了
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
			sprintf(szInfo, "ManualOnePhaseRun RingIndex:%d CurPhaseIndex:%d CurStage:%c NextStage:%c StageTime:%d", nRingIndex, nIndex, chcurStage, chNextStage, wStageTime);
			m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, szInfo);

			m_nStageRunTime[nRingIndex] += tRunCounter.m_nLampClrTime[nRingIndex];

			//设置环内相位灯色，重置时间
			SetLampClrByRing(nRingIndex,nIndex,ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseStage,ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPedPhaseStage);
			tRunCounter.m_nLampClrTime[nRingIndex] = 0;
			tRunCounter.m_nLampClrStartCounter[nRingIndex] = tRunCounter.m_nCurCounter;    
			m_pOpenATCRunStatus->SetPhaseLampClrRunCounter(tRunCounter);
			m_bIsLampClrChg = true;
		}
    }

	if (m_bIsSystemCtl)
	{
		if (!tValidManualCmd.m_bDirectionCmd && !tValidManualCmd.m_bChannelLockCmd)
		{
			SendCountDownPulse(nRingIndex, true);
		}
		else
		{
			//过渡到面板方向或通道锁定的过程中，不发红脉冲
			SendCountDownPulse(nRingIndex, false);
		}
	}
}

/*==================================================================== 
函数名 ：ManualOnePedPhaseRun
功能 ：手动控制时单个行人相位运行状态更新
算法实现 ： 
参数说明 ：nRingIndex，环索引 
返回值说明：
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 刘黎明          创建函数 
====================================================================*/ 
void CLogicCtlManual::ManualOnePedPhaseRun(int nRingIndex,TManualCmd tValidManualCmd)
{
    PTRingCtlInfo ptRingRunInfo = &(m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex]);
        
    int nIndex = m_tFixTimeCtlInfo.m_nCurPhaseIndex[nRingIndex];
    TPhaseLampClrRunCounter tRunCounter;
    m_pOpenATCRunStatus->GetPhaseLampClrRunCounter(tRunCounter);

	TAscStepCfg tStepCfg;
	memset(&tStepCfg, 0, sizeof(tStepCfg));
	m_pOpenATCParameter->GetStepInfo(tStepCfg);

	char chcurPedStage = ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPedPhaseStage;
	int nNextStageIndex = tValidManualCmd.m_tStepForwardCmd.m_nNextStageID;

	//当前相位阶段是否完成
	if (tStepCfg.m_byStepType == STEP_STAGE)
	{
		//行人相位绿闪跟机动车相位绿闪一起结束
		if (chcurPedStage == C_CH_PHASESTAGE_GF)
		{
			if (m_bPhaseColorChgToYellowFlag[nRingIndex])
			{
				m_tFixTimeCtlInfo.m_bIsChgPedStage[nRingIndex] = true;
				m_bPhaseColorChgToYellowFlag[nRingIndex] = false;
			}
		}
		else if (chcurPedStage == C_CH_PHASESTAGE_G)
		{
			if (tRunCounter.m_nPedLampClrTime[nRingIndex] >= ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPedPhaseGreenTime)
			{
				m_tFixTimeCtlInfo.m_bIsChgPedStage[nRingIndex] = true;

				//跨阶段相位，要保持绿灯状态，运行到跨的最后一个阶段时才能切到绿闪
				if (nNextStageIndex == 0)
				{
					nNextStageIndex = m_tFixTimeCtlInfo.m_nCurStageIndex + 1;
				}
				else
				{
					if (m_tFixTimeCtlInfo.m_nCurStageIndex == nNextStageIndex - 1)
					{
						nNextStageIndex = m_tFixTimeCtlInfo.m_nCurStageIndex + 1;
					}
					else
					{
						nNextStageIndex = nNextStageIndex - 1;
					}
				}
				if (nNextStageIndex >= m_tRunStageInfo.m_nRunStageCount)
				{
					nNextStageIndex = 0;
				}
				
				//在控制过程中的相位绿灯没有到该相位的最后一个阶段都不切，其他都切
				if (m_nManualCurStatus == MANUAL_CONTROL_STATUS && 
					m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[m_tFixTimeCtlInfo.m_nCurPhaseIndex[nRingIndex]].m_tPhaseParam.m_byPhaseNumber == m_tRunStageInfo.m_PhaseRunstageInfo[nNextStageIndex].m_nConcurrencyPhase[nRingIndex])
				{
					m_tFixTimeCtlInfo.m_bIsChgPedStage[nRingIndex] = false;
					if (m_tPhasePassCmdPhaseStatus.m_nPhasePassStatus[ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhaseNumber - 1] == PhasePassStatus_Close)
					{
						m_tFixTimeCtlInfo.m_bIsChgPedStage[nRingIndex] = true;
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
			int nPhaseID = ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhaseNumber;
			if (m_tPhasePassCmdPhaseStatus.m_nPhasePassStatus[nPhaseID - 1] == PhasePassStatus_Normal)
			{
				m_tFixTimeCtlInfo.m_bIsChgPedStage[nRingIndex] = true;
			}
		}
		else
		{
			if (tRunCounter.m_nPedLampClrTime[nRingIndex] >= ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPedPhaseStageRunTime)
			{
				m_tFixTimeCtlInfo.m_bIsChgPedStage[nRingIndex] = true;
			}
		}
	}
	
    if (m_tFixTimeCtlInfo.m_bIsChgPedStage[nRingIndex])
    {
        PTPhaseCtlInfo pPhaseInfo = (PTPhaseCtlInfo)&(ptRingRunInfo->m_atPhaseInfo[nIndex]);
        WORD wPedStageTime = 0;
        char chNextPedStage =  this->GetPedNextPhaseStageInfo(chcurPedStage,pPhaseInfo,wPedStageTime);
		ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPedPhaseStage = chNextPedStage;
		ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPedPhaseStageRunTime = wPedStageTime;

		if (chNextPedStage != C_CH_PHASESTAGE_G && !IsNeedTransBeforeControlChannel(ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhaseNumber))
		{
			wPedStageTime = 0;
		}

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
			m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "ManualOnePedPhaseRun RingIndex:%d CurPhaseIndex:%d PedCurStage:%c PedNextStage:%c PedStageTime:%d", nRingIndex, nIndex, chcurPedStage, chNextPedStage, wPedStageTime);

			//设置环内相位灯色，重置时间
			SetLampClrByRing(nRingIndex,nIndex,ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseStage,ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPedPhaseStage);
			tRunCounter.m_nPedLampClrTime[nRingIndex] = 0;
			tRunCounter.m_nPedLampClrStartCounter[nRingIndex] = tRunCounter.m_nCurCounter;    
			m_pOpenATCRunStatus->SetPhaseLampClrRunCounter(tRunCounter);
			m_bIsLampClrChg = true;
		}
    }

	if (m_bIsSystemCtl)
	{
		if (!tValidManualCmd.m_bDirectionCmd && !tValidManualCmd.m_bChannelLockCmd)
		{
			SendPedCountDownPulse(nRingIndex, true);
		}
		else
		{
			//过渡到面板方向或通道锁定的过程中，不发红脉冲
			SendPedCountDownPulse(nRingIndex, false);
		}
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
2019/09/14     V1.0 刘黎明          创建函数 
====================================================================*/ 
void CLogicCtlManual::SetCtlDerivedParam(void * pParam,void * pLampClr,void * pStageTable,void * pChannelSplitMode)
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
2019/09/14     V1.0 刘黎明          创建函数 
====================================================================*/ 
void CLogicCtlManual::ManualCycleChg(bool bIsAutoCtl)
{
    char szInfo[256] = {0};
    bool bCycleChg = true;
    for (int i = 0;i < m_tFixTimeCtlInfo.m_nRingCount;i ++)
    {
        PTRingCtlInfo ptRingRunInfo = &(m_tFixTimeCtlInfo.m_atPhaseSeq[i]);
        int nIndex = m_tFixTimeCtlInfo.m_nCurPhaseIndex[i];
        if (ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseStage == C_CH_PHASESTAGE_F && 
            nIndex == ptRingRunInfo->m_nPhaseCount - 1)
        {
             //sprintf(szInfo, "ManualCycleChg RingIndex:%d CurPhaseIndex:%d CurStageIndex:%d", i, nIndex, m_tFixTimeCtlInfo.m_nCurStageIndex);
             //m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, szInfo);       
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

    //当前周期是否完成
    if (bCycleChg || m_bCycleEndFlag)
    {
		if (m_nManualCurStatus == MANUAL_STAGE_TRANS)
		{
			SetGetParamFlag(true);
		}

        ReSetCtlStatus();
        m_tFixTimeCtlInfo.m_wCycleRunTime = 0;

        if (bIsAutoCtl)
        {
            SetGetParamFlag(true);
        }
		m_bClearPulseFlag = false;
        m_pOpenATCRunStatus->SetCycleChgStatus(true);
    }
}

/*==================================================================== 
函数名 ：SysCtlOnePhaseRun
功能 ：手动干预时相位运行状态更新
算法实现 ： 
参数说明 ：tValidManualCmd，手动指令
返回值说明：
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 李永萍          创建函数 
====================================================================*/ 
void CLogicCtlManual::SysCtlOnePhaseRun(TManualCmd & tValidManualCmd)
{
	static int nCycleEnd = 0;
	int nStageIndex = 0;
	if (ManualSwitchStage(tValidManualCmd, nStageIndex))//切换到指定阶段
	{
		m_bTransToAutoFlag = false;
		if (tValidManualCmd.m_nSubCtlMode != CTL_MANUAL_SUBMODE_PANEL_FITSTMANUAL)
		{
			nCycleEnd = 0;
			if (m_tFixTimeCtlInfo.m_nCurStageIndex + 1 == m_tRunStageInfo.m_nRunStageCount && m_nManualCurStatus == MANUAL_STAGE_TRANS 
				&& (tValidManualCmd.m_nCtlMode == CTL_MODE_SELFCTL || tValidManualCmd.m_bPatternInterruptCmd))
			{
				m_bCycleEndFlag = true;
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "Switch Stage Succeed, Set CycleEndFlag");
			}
			else
			{
				// 由于无用户干预指令回到自主,有通道锁定指令来则不能回自主
				if (m_nReturnAutoCtrlStageIndex == m_tFixTimeCtlInfo.m_nCurStageIndex && !tValidManualCmd.m_bChannelLockCmd)
				{
					CreateManualCmdReturnToSelf(tValidManualCmd);//生成回到自主的命令，COpenATCLogicCtlManager可以根据这个命令调用自主函数回到自主
					m_bTransToAutoFlag = true;
					m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "Switch Stage Succeed, Set TransToAutoFlag And CreateManualCmdReturnToSelf");

					//开始切自主，要清除相位控制表
					if (!m_bIfPhasePassCmdPhaseStatusHaveRefreshedToInit)
					{
						memset(&m_tPhasePassCmdPhaseStatus, 0, sizeof(m_tPhasePassCmdPhaseStatus));
						m_pOpenATCRunStatus->SetLocalPhasePassStatus(m_tPhasePassCmdPhaseStatus);
						m_bIfPhasePassCmdPhaseStatusHaveRefreshedToInit = true;
					}
				}

				m_tFixTimeCtlInfo.m_nCurStageIndex = nStageIndex;
			}
		}
		else
		{
			if (m_tFixTimeCtlInfo.m_nCurStageIndex + 1 == m_tRunStageInfo.m_nRunStageCount && m_nManualCurStatus == MANUAL_STAGE_TRANS)
			{
				nCycleEnd = ++nCycleEnd % 2;
				if (nCycleEnd == 0)//手动面板按钮按下后，无其他操作情况下，第二个周期开始回自主
				{
					CreateManualCmdReturnToSelf(tValidManualCmd);//生成回到自主的命令，COpenATCLogicCtlManager可以根据这个命令调用自主函数回到自主
					m_bTransToAutoFlag = true;
					m_bCycleEndFlag = true;
					m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "FirstManual Trans Succeed, Set TransToAutoFlag And CreateManualCmdReturnToSelf");
				}
			}

			m_tFixTimeCtlInfo.m_nCurStageIndex = nStageIndex;
		}

		//判断当前是不是色步模式下的阶段驻留
		TAscStepCfg tStepCfg;
		memset(&tStepCfg, 0, sizeof(tStepCfg));
		m_pOpenATCParameter->GetStepInfo(tStepCfg);
		if (tValidManualCmd.m_tStepForwardCmd.m_nNextStageID != 0 && tStepCfg.m_byStepType == STEP_COLOR)
		{
			m_bStageStepwardInColorStep = true;
		}
		else
		{
			m_bStageStepwardInColorStep = false;
		}

		m_nNextStageIndex = m_tFixTimeCtlInfo.m_nCurStageIndex + 1;
		if (m_nNextStageIndex >= m_tRunStageInfo.m_nRunStageCount)
		{
			m_nNextStageIndex = 0;
		}
		//当前阶段过渡结束，准备切方向
		if (tValidManualCmd.m_bDirectionCmd)
		{
			if (tValidManualCmd.m_nSubCtlMode == CTL_MANUAL_SUBMODE_PANEL_STEPFOWARD)
			{
				tValidManualCmd.m_tDirectionCmd.m_bStepFowardToDirection = true;
			}

            tValidManualCmd.m_nSubCtlMode = CTL_MANUAL_SUBMODE_PANEL_DIRECTION;
			tValidManualCmd.m_tDirectionCmd.m_nTargetStageIndex = m_tFixTimeCtlInfo.m_nCurStageIndex;//记录下切方向之前的下一个阶段

			tValidManualCmd.m_bDirectionCmd = false;//切到方向控制子模式下，清除当前方向指令标志
		   
			//初始化方向参数，开始切方向
			InitParamBeforeDirection(tValidManualCmd.m_tDirectionCmd.m_nNextDirectionIndex);
			ChangeChannelClr();
			TAscManualPanel tAscManualPanel;
			memset(&tAscManualPanel, 0, sizeof(TAscManualPanel)); 
			m_pOpenATCParameter->GetManualPanelInfo(tAscManualPanel);
			for (int i = 0;i < MAX_CHANNEL_COUNT;i++)
			{
				m_nDirectionGreenTime[i] = tAscManualPanel.m_wDuration;
				m_bChannelKeepGreenFlag[i] = false;
			}
			m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "Switch Stage Succeed, Remember NextStageIndex:%d And InitParamBeforeDirection, Calculate DirectionIndex:%d GreenTime:%d", m_tFixTimeCtlInfo.m_nCurStageIndex, tValidManualCmd.m_tDirectionCmd.m_nNextDirectionIndex, tAscManualPanel.m_wDuration);
			//因为修改了方向的参数，所以指令还要备份过一次
			memcpy(&m_tOldValidManualCmd, &tValidManualCmd, sizeof(tValidManualCmd));
			//切到方向以后，方向开始前继续保持绿色的通道的标志要清零
			memset(m_bKeepGreenChannelBeforeControlChannelFlag, false, sizeof(m_bKeepGreenChannelBeforeControlChannelFlag));
		}
		else if (tValidManualCmd.m_bPatternInterruptCmd && m_nManualCurStatus == MANUAL_STAGE_TRANS)
		{
			//收到方案干预指令，要清除相位控制表
			if (!m_bIfPhasePassCmdPhaseStatusHaveRefreshedToInit)
			{
				memset(&m_tPhasePassCmdPhaseStatus, 0, sizeof(m_tPhasePassCmdPhaseStatus));
				m_pOpenATCRunStatus->SetLocalPhasePassStatus(m_tPhasePassCmdPhaseStatus);
				m_bIfPhasePassCmdPhaseStatusHaveRefreshedToInit = true;
			}
		}
		else if (tValidManualCmd.m_bChannelLockCmd)
		{
			memset(m_bNeglectChannelBoforePhaseLock, 0, sizeof(m_bNeglectChannelBoforePhaseLock));
			if (tValidManualCmd.m_bPhaseToChannelLock)
			{
				SetNeglectChannelBoforePhaseLock();
			}

			m_bOldLockChannelCmdEndFlag = true;
			SetLockPhaseList(tValidManualCmd);

			if (tValidManualCmd.m_nSubCtlMode == CTL_MANUAL_SUBMODE_SYSTEM_STEPFOWARD)
			{
				tValidManualCmd.m_tChannelLockCmd.m_bStepFowardToChannelLock = true;
				tValidManualCmd.m_tChannelLockCmd.m_nDelayTime = m_tOldValidManualCmd.m_tStepForwardCmd.m_nDelayTime;
				tValidManualCmd.m_tChannelLockCmd.m_nDurationTime = m_tOldValidManualCmd.m_tStepForwardCmd.m_nDurationTime;
			}

            tValidManualCmd.m_nSubCtlMode = CTL_MANUAL_SUBMODE_SYSTEM_CHANNEL_LOCK;
			tValidManualCmd.m_tChannelLockCmd.m_nTargetStageIndex = m_tFixTimeCtlInfo.m_nCurStageIndex;//记录下切通道锁定之前的下一个阶段

			tValidManualCmd.m_bNewCmd = false;
			tValidManualCmd.m_bChannelLockCmd = false;//切到通道锁定控制子模式下，清除当前通道锁定指令标志
		   
			//初始化通道锁定参数，开始切通道锁定
			InitParamBeforeChannelLock(tValidManualCmd.m_tChannelLockCmd.m_tNextChannelLockCtrlCmd.m_nChannelLockStatus);
			ChangeChannelClr();
			for (int i = 0;i < MAX_CHANNEL_COUNT;i++)
			{
				m_nChannelLockGreenTime[i] = tValidManualCmd.m_tChannelLockCmd.m_tNextChannelLockCtrlCmd.m_nDuration;
				m_bChannelKeepGreenFlag[i] = false;
			}
			m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "Switch Stage Succeed, Remember NextStageIndex:%d And InitParamBeforeChannelelLock, Calculate GreenTime:%d", m_tFixTimeCtlInfo.m_nCurStageIndex, tValidManualCmd.m_tChannelLockCmd.m_tNextChannelLockCtrlCmd.m_nDuration);
			//因为修改了通道锁定的参数，所以指令还要备份过一次
			memcpy(&m_tOldValidManualCmd, &tValidManualCmd, sizeof(tValidManualCmd));
			//切到通道锁定以后，锁定通道开始前继续保持绿色的通道的标志要清零
			memset(m_bKeepGreenChannelBeforeControlChannelFlag, false, sizeof(m_bKeepGreenChannelBeforeControlChannelFlag));

			//获取锁定相位参数
			GetLockPhaseData(tValidManualCmd);
		}
	
		tValidManualCmd.m_bStepForwardCmd = false;//手动控制模式下，清除当前步进指令标志
		tValidManualCmd.m_tStepForwardCmd.m_nNextStageID = 0;
		m_pOpenATCRunStatus->SetValidManualCmd(tValidManualCmd);

		m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "Switch Stage Succeed, ControlMode:%d SubControlMode:%d StepForwardCmd Status:%d", tValidManualCmd.m_nCtlMode, tValidManualCmd.m_nSubCtlMode, tValidManualCmd.m_bStepForwardCmd);
	}
}

/*==================================================================== 
函数名 ：GetChannelData
功能 ：获取通道有关的参数
算法实现 ： 
参数说明 ：nNextDirectionIndex，方向编号
返回值说明：无
 ----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 李永萍          创建函数 
====================================================================*/ 
void CLogicCtlManual::GetChannelData(int nNextDirectionIndex)
{
	int i = 0, j = 0, k = 0;

	TAscManualPanel tAscManualPanel;
    memset(&tAscManualPanel, 0, sizeof(TAscManualPanel)); 
    m_pOpenATCParameter->GetManualPanelInfo(tAscManualPanel);

	
	//获取当前通道需要切换的目标灯色
    for (i = 0;i < MAX_PANEL_KEY_CFG_COUNT;i++)
	{
		if (tAscManualPanel.m_stPanelKeyCfg[i].m_byKeyNum == nNextDirectionIndex)
		{    
			for (j = 0;j < m_nChannelCount;j++)
			{
				if (m_atChannelInfo[j].m_byChannelNumber == 0)
				{
					continue;
				}

				for (k = 0;k < MAX_CHANNEL_COUNT;k++)
				{
					if (tAscManualPanel.m_stPanelKeyCfg[i].m_ChannelCfg[k].m_byChannelID == 0)
					{
						continue;
					}

					if (tAscManualPanel.m_stPanelKeyCfg[i].m_ChannelCfg[k].m_byChannelID == m_atChannelInfo[j].m_byChannelNumber && tAscManualPanel.m_stPanelKeyCfg[i].m_ChannelCfg[k].m_byChannelStatus != CHANNEL_STATUS_DEFAULT)
					{
						m_bChangeChannelClr[j] = true;

						m_chChannelStatus[j] = tAscManualPanel.m_stPanelKeyCfg[i].m_ChannelCfg[k].m_byChannelStatus;
					}
					else
					{ 
						//m_bChangeChannelClr[j] = false;

						//m_chChannelStatus[j] = C_CH_PHASESTAGE_U;
					}

					//m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "GetChannelData KeyIndex:%d ChannelIndex:%d ChannelStatus:%d ChangeChannelClr:%d", nNextDirectionIndex, j, m_chChannelStatus[j], m_bChangeChannelClr[j]);
				}
			}
  
            break;
		} 
	}

	//获取当前通道的灯色
    for (j = 0;j < m_nChannelCount;j++)
    {
		if (m_atChannelInfo[j].m_byChannelNumber == 0)
		{
			continue;
		}

        if (m_achLampClr[(m_atChannelInfo[j].m_byChannelNumber - 1) * 3] == LAMP_CLR_ON)
		{
            m_chChannelStage[j] = C_CH_PHASESTAGE_R;
		}

		if (m_achLampClr[(m_atChannelInfo[j].m_byChannelNumber - 1) * 3 + 1] == LAMP_CLR_ON)
		{
            m_chChannelStage[j] = C_CH_PHASESTAGE_Y;
		}
		else if (m_achLampClr[(m_atChannelInfo[j].m_byChannelNumber - 1) * 3 + 1] == LAMP_CLR_FLASH)
		{
            m_chChannelStage[j] = C_CH_PHASESTAGE_YF;
		}
		
		if (m_achLampClr[(m_atChannelInfo[j].m_byChannelNumber - 1) * 3 + 2] == LAMP_CLR_ON)
		{
            m_chChannelStage[j] = C_CH_PHASESTAGE_G;
		}

		if (m_achLampClr[(m_atChannelInfo[j].m_byChannelNumber - 1) * 3] == LAMP_CLR_OFF &&
			m_achLampClr[(m_atChannelInfo[j].m_byChannelNumber - 1) * 3 + 1] == LAMP_CLR_OFF &&
			m_achLampClr[(m_atChannelInfo[j].m_byChannelNumber - 1) * 3 + 2] == LAMP_CLR_OFF)
		{
			m_chChannelStage[j] = C_CH_PHASESTAGE_R;
		}
    }    
}

/*==================================================================== 
函数名 ：ProcessDirection
功能 ：处理方向。
算法实现 ： 
参数说明 ：tValidManualCmd，手动命令
返回值说明：无
 ----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 李永萍          创建函数 
====================================================================*/ 
void  CLogicCtlManual::ProcessDirection(TManualCmd & tValidManualCmd)
{
	if (tValidManualCmd.m_bNewCmd)
	{
		TAscManualPanel tAscManualPanel;
		memset(&tAscManualPanel, 0, sizeof(TAscManualPanel)); 
		m_pOpenATCParameter->GetManualPanelInfo(tAscManualPanel);

		if (m_tOldValidManualCmd.m_nCtlMode == CTL_MODE_SELFCTL && m_tOldValidManualCmd.m_nSubCtlMode == CTL_MANUAL_SUBMODE_DEFAULT &&
			m_tOldValidManualCmd.m_nCmdSource == CTL_SOURCE_SELF)
		{
			m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "From YellowFlah, AllRed Or LampOff Enter Into Direction!");
			memcpy(&m_tOldValidManualCmd, &tValidManualCmd, sizeof(tValidManualCmd));//从黄闪，全红或关灯直接进入方向的，需要备份数据
		}

		//面板进入方向以后，根据新来的指令计算绿灯时间
		if ((tValidManualCmd.m_bPatternInterruptCmd && tValidManualCmd.m_tPatternInterruptCmd.m_nControlMode == CTL_MODE_SELFCTL) || //面板进入方向以后，按了自主
			tValidManualCmd.m_bDirectionCmd || //面板进入方向以后，又按方向
			tValidManualCmd.m_bStepForwardCmd)//面板进入方向以后，按了自动，方向还没有来得及过渡到自动，又按了手动
		{
			//方向1切到方向2，方向1还没有过渡到最小绿，又切方向1，绿灯时间还是走绿灯过渡时间，其他情况绿灯应走最小绿
			//方向1正在过渡灯色(绿闪，黄，红)，又切方向1，则方向1走完，再回到方向1
			if (tValidManualCmd.m_bDirectionCmd && tValidManualCmd.m_tDirectionCmd.m_nNextDirectionIndex == m_tOldValidManualCmd.m_tDirectionCmd.m_nNextDirectionIndex)
			{
				//获取当前通道需要切换的目标灯色
				for (int i = 0;i < MAX_PANEL_KEY_CFG_COUNT;i++)
				{
					if (tAscManualPanel.m_stPanelKeyCfg[i].m_byKeyNum == m_tOldValidManualCmd.m_tDirectionCmd.m_nNextDirectionIndex)
					{    
						for (int j = 0;j < m_nChannelCount;j++)
						{
							if (tAscManualPanel.m_stPanelKeyCfg[i].m_ChannelCfg[j].m_byChannelStatus != CHANNEL_STATUS_DEFAULT && 
								m_chChannelStage[j] == C_CH_PHASESTAGE_G)
							{
								tValidManualCmd.m_bDirectionCmd = false;//方向1切到方向2，方向1还没有过渡到最小绿，又切方向1，该相同的值指令不执行
							}
						}
  
						break;
					} 
				}

				for (int i = 0;i < MAX_CHANNEL_COUNT;i++)
				{
					m_nDirectionGreenTime[i] = tAscManualPanel.m_wDuration;	
					m_bChannelKeepGreenFlag[i] = false;
				}
			}
			else
			{
				int  i = 0, j = 0, k = 0;
				BYTE byOldChannelStatus[MAX_CHANNEL_COUNT];
				memset(byOldChannelStatus,0,MAX_CHANNEL_COUNT);
				BYTE byNewChannelStatus[MAX_CHANNEL_COUNT];
				memset(byNewChannelStatus,0,MAX_CHANNEL_COUNT);
				//获取当前通道需要切换的目标灯色
				for (i = 0;i < MAX_PANEL_KEY_CFG_COUNT;i++)
				{
					if (tAscManualPanel.m_stPanelKeyCfg[i].m_byKeyNum == m_tOldValidManualCmd.m_tDirectionCmd.m_nNextDirectionIndex)
					{    
						for (k = 0;k < m_nChannelCount;k++)
						{
							for (j = 0;j < MAX_CHANNEL_COUNT;j++)
							{
								if (tAscManualPanel.m_stPanelKeyCfg[i].m_ChannelCfg[j].m_byChannelID == k + 1)
								{
									byOldChannelStatus[k] = tAscManualPanel.m_stPanelKeyCfg[i].m_ChannelCfg[j].m_byChannelStatus;
								}
							}
						}
					}
					if (tAscManualPanel.m_stPanelKeyCfg[i].m_byKeyNum == tValidManualCmd.m_tDirectionCmd.m_nNextDirectionIndex)
					{    
						for (k = 0;k < m_nChannelCount;k++)
						{
							for (j = 0;j < MAX_CHANNEL_COUNT;j++)
							{
								if (tAscManualPanel.m_stPanelKeyCfg[i].m_ChannelCfg[j].m_byChannelID == k + 1)
								{
									byNewChannelStatus[k] = tAscManualPanel.m_stPanelKeyCfg[i].m_ChannelCfg[j].m_byChannelStatus;
								}
							}
						}
					}
				}

				for (i = 0;i < MAX_CHANNEL_COUNT;i++)
				{
					if (byOldChannelStatus[i] == CHANNEL_STATUS_GREEN && m_chChannelStage[i] == C_CH_PHASESTAGE_G)
					{
						if (byNewChannelStatus[i] == CHANNEL_STATUS_GREEN)
						{
							m_nDirectionGreenTime[i] = tAscManualPanel.m_wDuration;	
							m_bChannelKeepGreenFlag[i] = true;
						}
						else
						{
							m_nDirectionGreenTime[i] = tAscManualPanel.m_byMinGreen;	
							m_bChannelKeepGreenFlag[i] = false;
							m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "ProcessDirection, ChannelID:%d SetCurrentDirectionGreenTime:%d ", i + 1, m_nDirectionGreenTime[i]);
						}
					}
				}
			} 
		}
		else//刚开始进入面板方向
		{
			//初始化方向参数，开始切方向
			InitParamBeforeDirection(tValidManualCmd.m_tDirectionCmd.m_nNextDirectionIndex);
			ChangeChannelClr();

			for (int i = 0;i < MAX_CHANNEL_COUNT;i++)
			{
				m_nDirectionGreenTime[i] = tAscManualPanel.m_wDuration;	
				m_bChannelKeepGreenFlag[i] = false;
			}
		}

		tValidManualCmd.m_bNewCmd = false;
		m_pOpenATCRunStatus->SetValidManualCmd(tValidManualCmd);
	}

	bool bLockCmd = false;//下一个指令是否还是锁定指令标志
	if (tValidManualCmd.m_bDirectionCmd)
	{
		bLockCmd = true;
	}

	bool bSelRun = TransitChannelClr(CHANNEL_TYPE_DIRECTION, bLockCmd);
	if (bSelRun)
	{
		if ((tValidManualCmd.m_bPatternInterruptCmd && tValidManualCmd.m_tPatternInterruptCmd.m_nControlMode == CTL_MODE_SELFCTL) || 
			tValidManualCmd.m_bDirectionCmd ||
			tValidManualCmd.m_bStepForwardCmd)
		{
			if (tValidManualCmd.m_bDirectionCmd)//方向过渡结束，进入新的方向
			{
				//初始化方向参数，开始方向控制
				InitParamBeforeDirection(tValidManualCmd.m_tDirectionCmd.m_nNextDirectionIndex);
				ChangeChannelClr();
				TAscManualPanel tAscManualPanel;
				memset(&tAscManualPanel, 0, sizeof(TAscManualPanel)); 
				m_pOpenATCParameter->GetManualPanelInfo(tAscManualPanel);
				for (int i = 0;i < MAX_CHANNEL_COUNT;i++)
				{
					m_nDirectionGreenTime[i] = tAscManualPanel.m_wDuration;	
					m_bChannelKeepGreenFlag[i] = false;
				}
				//清除新的方向指令标志，因为是方向切方向所以当前控制子模式还是方向控制
				tValidManualCmd.m_bNewCmd = false;
				tValidManualCmd.m_bDirectionCmd = false;
				m_tOldValidManualCmd.m_tDirectionCmd.m_nNextDirectionIndex = tValidManualCmd.m_tDirectionCmd.m_nNextDirectionIndex;
				//memcpy(&tValidManualCmd.m_tDirectionCmd,&m_tOldValidManualCmd.m_tDirectionCmd,sizeof(TDirectionCmd));
				m_pOpenATCRunStatus->SetValidManualCmd(tValidManualCmd);
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "ProcessDirection, TransitDirection End, Trans To NewDirectionIndex:%d GreenTime:%d", tValidManualCmd.m_tDirectionCmd.m_nNextDirectionIndex, tAscManualPanel.m_wDuration);
			}
			else
			{
				//因为面板按了自主指令，所以方向过渡结束后，修改当前控制模式和控制子模式
				if (tValidManualCmd.m_bPatternInterruptCmd)
				{
					tValidManualCmd.m_nCtlMode = CTL_MODE_SELFCTL;
					tValidManualCmd.m_nSubCtlMode = CTL_MANUAL_SUBMODE_DEFAULT;
					tValidManualCmd.m_bNewCmd = false;
					m_pOpenATCRunStatus->SetValidManualCmd(tValidManualCmd);
					m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "ProcessDirection, TransitDirection End, Trans To SelfCtl");
					m_nManualCurStatus = MANUAL_STAGE_TRANS;//进入过渡

					//此时为了硬件模块的m_bManualBtn能置false，需要再次返回自主按钮按下状态
					THWPanelBtnStatus tHWPanelBtnStatus;
					tHWPanelBtnStatus.m_nHWPanelBtnIndex = HWPANEL_BTN_AUTO;
					tHWPanelBtnStatus.m_bHWPanelBtnStatus = true;
					m_pOpenATCRunStatus->SetPanelBtnStatusList(tHWPanelBtnStatus);
				}
				else //面板进入方向以后，按了自动，方向还没有来得及过渡到自动，又按了手动，进入第一次手动过渡模式
				{
					tValidManualCmd.m_nSubCtlMode = CTL_MANUAL_SUBMODE_PANEL_FITSTMANUAL;
					tValidManualCmd.m_bNewCmd = false;
					m_pOpenATCRunStatus->SetValidManualCmd(tValidManualCmd);
					m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "ProcessDirection, TransitDirection End, Trans To Panel First Manual");
				}

				//下发了自主指令或手动指令，方向过渡结束，直接修改控制模式和子模式，备份指令
			    memcpy(&m_tOldValidManualCmd, &tValidManualCmd, sizeof(tValidManualCmd));
			}
		}
		else
		{
			if (tValidManualCmd.m_tDirectionCmd.m_bStepFowardToDirection)//回到方向前的步进
			{
				TAscStepCfg tStepCfg;
				memset(&tStepCfg, 0, sizeof(tStepCfg));
				m_pOpenATCParameter->GetStepInfo(tStepCfg);
				//生成回到自主前的步进指令，回到切方向前的阶段的下一个阶段
				tValidManualCmd.m_bNewCmd = true;
				tValidManualCmd.m_nCtlMode = CTL_MODE_MANUAL;
				tValidManualCmd.m_nSubCtlMode = CTL_MANUAL_SUBMODE_PANEL_STEPFOWARD;
				tValidManualCmd.m_bStepForwardCmd = true;
				tValidManualCmd.m_tStepForwardCmd.m_byStepType = tStepCfg.m_byStepType;
		        tValidManualCmd.m_tStepForwardCmd.m_nNextStageID = tValidManualCmd.m_tDirectionCmd.m_nTargetStageIndex + 1;
				tValidManualCmd.m_tStepForwardCmd.m_nDurationTime = 0;
				tValidManualCmd.m_tStepForwardCmd.m_nDelayTime = 0;
				tValidManualCmd.m_bDirectionCmd = false;
				memset(&tValidManualCmd.m_tDirectionCmd,0,sizeof(TDirectionCmd));
				tValidManualCmd.m_bPatternInterruptCmd = false;
				memset(&tValidManualCmd.m_tPatternInterruptCmd,0,sizeof(TPatternInterruptCmd));
				tValidManualCmd.m_bChannelLockCmd = false;
				memset(&tValidManualCmd.m_tChannelLockCmd,0,sizeof(TChannelLockCmd));
				memset(&tValidManualCmd.m_tPhaseLockPara,0,sizeof(TPhaseLockPara));
				tValidManualCmd.m_bPhaseToChannelLock = false;
				tValidManualCmd.m_bPreemptCtlCmd = false;
				m_pOpenATCRunStatus->SetValidManualCmd(tValidManualCmd);
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "ProcessDirection, Direction Duration End, Return To StepWard");

				//初始化下一个阶段的相位
				for (int i = 0;i < m_tFixTimeCtlInfo.m_nRingCount;i++)
				{
					InitNextStagePhase(i, m_tFixTimeCtlInfo.m_nCurPhaseIndex[i], tValidManualCmd.m_tDirectionCmd.m_nTargetStageIndex, true);
				}

				THWPanelBtnStatus tHWPanelBtnStatus;
				tHWPanelBtnStatus.m_nHWPanelBtnIndex = HWPANEL_BTN_STEP;
				tHWPanelBtnStatus.m_bHWPanelBtnStatus = true;
				m_pOpenATCRunStatus->SetPanelBtnStatusList(tHWPanelBtnStatus);
			}
			else //用户干预超时，回到自主
			{
				m_nManualCurStatus = MANUAL_STAGE_TRANS;//需要置为过渡状态
                CreateManualCmdReturnToSelf(tValidManualCmd);//生成回到自主的命令，COpenATCLogicCtlManager可以根据这个命令调用自主函数回到自主
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "ProcessDirection, Direction Duration End, CreateManualCmdReturnToSelf");
			}
		}
	}
	else
	{
		if (m_nManualCurStatus == MANUAL_STAGE_TRANS)
		{
			m_nManualCurStatus = MANUAL_CONTROL_STATUS;
		}
	}
}

/*==================================================================== 
函数名 ：ManualSwitchStage
功能 ：切到指定目标阶段后，初始化下一个阶段的相位
算法实现 ： 
参数说明 ：tValidManualCmd：手动命令，nStageIndex，目标阶段号
返回值说明：无
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 李永萍          创建函数 
====================================================================*/ 
bool  CLogicCtlManual::ManualSwitchStage(TManualCmd  tValidManualCmd, int & nStageIndex)
{
	int nNextStageIndex = tValidManualCmd.m_tStepForwardCmd.m_nNextStageID;
	if (nNextStageIndex == 0)
	{
		nStageIndex = m_tFixTimeCtlInfo.m_nCurStageIndex + 1;
	}
	else
	{
		if (m_tFixTimeCtlInfo.m_nCurStageIndex == nNextStageIndex - 1)
		{
			nStageIndex = m_tFixTimeCtlInfo.m_nCurStageIndex + 1;
		}
		else
		{
			nStageIndex = nNextStageIndex - 1;
		}
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
		int nPhaseID = m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[m_tFixTimeCtlInfo.m_nCurPhaseIndex[i]].m_tPhaseParam.m_byPhaseNumber;
        if ((m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[m_tFixTimeCtlInfo.m_nCurPhaseIndex[i]].m_chPhaseStage == C_CH_PHASESTAGE_F) ||
		    (m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[m_tFixTimeCtlInfo.m_nCurPhaseIndex[i]].m_chPhaseStage == C_CH_PHASESTAGE_U && m_tPhasePassCmdPhaseStatus.m_nPhasePassStatus[nPhaseID - 1] == PhasePassStatus_Close))
        {
			nCount += 1;
			bChangePhaseFlag[i] = true;
	
			if (m_tPhasePassCmdPhaseStatus.m_nPhasePassStatus[nPhaseID - 1] == PhasePassStatus_Close)
			{
				bPhaseCloseFlag = true;
				nPhaseCloseCount += 1;
			}
        }
		else
		{
			nCurStageIndex = m_tFixTimeCtlInfo.m_nCurStageIndex;
			if (m_tRunStageInfo.m_PhaseRunstageInfo[nCurStageIndex].m_nConcurrencyPhase[i] == m_tRunStageInfo.m_PhaseRunstageInfo[nStageIndex].m_nConcurrencyPhase[i])
			{
				nRepeatCnt += 1;
				bChangePhaseFlag[i] = false;
			}
			else
			{
				bChangePhaseFlag[i] = true;
			}
		}
    }

	if (tValidManualCmd.m_bDirectionCmd ||  tValidManualCmd.m_bChannelLockCmd)
	{
        nRepeatCnt = 0;//第一次手动切方向控制时，不考虑跨阶段
	}

    if (nCount + nRepeatCnt == m_tFixTimeCtlInfo.m_nRingCount)
	{
		//阶段1包含相位1和3，阶段2包含相位1和4，阶段3包含相位2和4，阶段1时关断相位1或3，步进到阶段2要卡住
		if (bPhaseCloseFlag && !tValidManualCmd.m_bPatternInterruptCmd && !tValidManualCmd.m_bChannelLockCmd)
		{
			for (i = 0;i < m_tFixTimeCtlInfo.m_nRingCount;i++)
			{
				if (!bChangePhaseFlag[i] && (tRunCounter.m_nLampClrTime[i] != m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[m_tFixTimeCtlInfo.m_nCurPhaseIndex[i]].m_wPhaseGreenTime))
				{
					return bRet;
				}
			}
		}

		if (nPhaseCloseCount == m_tFixTimeCtlInfo.m_nRingCount)
		{
			ProcessAllClosePhaseInCurStage(tValidManualCmd, nStageIndex);
			return bRet;
		}
		

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
					//从带方案的控制方式进去通道锁定控制，要置过渡状态
					if (tValidManualCmd.m_nSubCtlMode == CTL_MANUAL_SUBMODE_DEFAULT && tValidManualCmd.m_bChannelLockCmd)
					{
						m_nManualCurStatus = MANUAL_STAGE_TRANS;
					}

					ResetNextStagePhaseGreenTime(i, j, nStageIndex, bChangePhaseFlag[i], tValidManualCmd.m_tStepForwardCmd.m_nDurationTime);

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
函数名 ：ChangeChannelClr
功能 ：手动面板方向控制机动车通道状态更新
算法实现 ： 
参数说明 ：无
返回值说明：
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 李永萍          创建函数 
====================================================================*/ 
void  CLogicCtlManual::ChangeChannelClr()
{
	int i = 0;
    char * pStartPos = NULL;
   
	int nGlobalCounter = m_pOpenATCRunStatus->GetGlobalCounter();

    bool bIsChgStage = false;

	for (i = 0;i < m_nChannelCount;i++)
    {
		if (m_atChannelInfo[i].m_byChannelNumber == 0)
		{
			continue;
		}

        if (m_bChangeChannelClr[i])
        {
		    if (m_chChannelStatus[i] == CHANNEL_STATUS_GREEN)//切换成绿色
		    {   
			    m_nChannelCounter[i] = nGlobalCounter;
                m_nChannelDurationCounter[i] = nGlobalCounter;
			    bIsChgStage = true;

		        m_chChannelStage[i] = C_CH_PHASESTAGE_G;
			    //m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "SwitchChannel:%d To Green GlobalCounter:%d", i, nGlobalCounter);
		    }
		    else if (m_chChannelStatus[i] == CHANNEL_STATUS_RED)//切换成红色
		    {
                m_nChannelCounter[i] = nGlobalCounter;
                m_nChannelDurationCounter[i] = nGlobalCounter;
			    bIsChgStage = true;

		        m_chChannelStage[i] = C_CH_PHASESTAGE_R;
			    //m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "SwitchChannel:%d To Red GlobalCounter:%d", i, nGlobalCounter);
		    }
		    else if (m_chChannelStatus[i] == CHANNEL_STATUS_OFF)//切换成关灯
		    {
                m_nChannelCounter[i] = nGlobalCounter;
                m_nChannelDurationCounter[i] = nGlobalCounter;
			    bIsChgStage = true;

		        m_chChannelStage[i] = C_CH_PHASESTAGE_OF;
			    //m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "SwitchChannel:%d To Off GlobalCounter:%d", i, nGlobalCounter);
		    }
            else if (m_chChannelStatus[i] == CHANNEL_STATUS_DEFAULT)//切换成默认
		    {
                //m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "SwitchChannel:%d To Default GlobalCounter:%d", i, nGlobalCounter);  
		    }
            else
            {
                //m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "Not SwitchChannel:%d ChannelStatus:%c", i, m_chChannelStatus[i]);  
            }
        }
        else
        {
            //m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "Not SwitchChannel:%d ChangeChannelClr:%d", i, m_bChangeChannelClr[i]);  
        }
	}

    if (bIsChgStage)
    {
		bIsChgStage = false;

        for (i = 0;i < m_nChannelCount;i++)
        {
			if (m_atChannelInfo[i].m_byChannelNumber == 0)
			{
				continue;
			}

            if (m_bChangeChannelClr[i])
            { 
				pStartPos = m_achLampClr + (m_atChannelInfo[i].m_byChannelNumber - 1) * 3;

				SetOneChannelOutput(pStartPos, m_chChannelStage[i]);
				m_bIsDirectionChannelClrChg = true;
            }
        }
    }

	for (i = 0; i < m_tLockPhaseStage.nLockPhaseCount;i++)
	{
		m_tLockPhaseStage.chLockPhaseStage[i] = C_CH_PHASESTAGE_G;
		m_tLockPhaseStage.nLockPhaseCounter[i] = nGlobalCounter;
	}
}

/*==================================================================== 
函数名 ：TransitChannelClr
功能 ：
算法实现 ：方向过渡未结束的时候，用户进行干预，这个时候按照配置的最小绿时间运行，否则按照配置的持续时间运行
参数说明 ：byChannelType, 通道类型，0表示方向通道，1表示锁定通道
返回值说明：true，方向过渡结束
            false，方向过渡没有结束
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 李永萍          创建函数 
====================================================================*/ 
bool  CLogicCtlManual::TransitChannelClr(BYTE byChannelType, bool bLockCmd)
{
    int  i = 0;
	bool bIsChgStage = false;
    bool bSelRun[MAX_CHANNEL_COUNT];
	memset(bSelRun, 0x00, sizeof(bSelRun));

	int nGreenTime[MAX_CHANNEL_COUNT], nGreenFlashTime = 0, nYellowTime = 0, nRedTime = 0;
	memset(nGreenTime, 0, MAX_CHANNEL_COUNT);
	if (byChannelType == CHANNEL_TYPE_DIRECTION)
	{
		TAscManualPanel tAscManualPanel;
		memset(&tAscManualPanel, 0, sizeof(TAscManualPanel)); 
		m_pOpenATCParameter->GetManualPanelInfo(tAscManualPanel);

		for (int i = 0;i < m_nChannelCount;i++)
		{
			nGreenTime[i] = m_nDirectionGreenTime[i];
		}
		nGreenFlashTime = tAscManualPanel.m_byGreenFlash;
		nYellowTime = tAscManualPanel.m_byYellow;
		nRedTime = tAscManualPanel.m_byRedClear;
	}
	else
	{
		for (int i = 0;i < m_nChannelCount;i++)
		{
			nGreenTime[i] = m_nChannelLockGreenTime[i];
		}
		nGreenFlashTime = m_tOldValidManualCmd.m_tChannelLockCmd.m_tNextChannelLockCtrlCmd.m_nGreenFlash;
		nYellowTime = m_tOldValidManualCmd.m_tChannelLockCmd.m_tNextChannelLockCtrlCmd.m_nYellow;
		nRedTime = m_tOldValidManualCmd.m_tChannelLockCmd.m_tNextChannelLockCtrlCmd.m_nRedClear;
	}

	int  nGlobalCounter = m_pOpenATCRunStatus->GetGlobalCounter();

	bool  bChannelNoNeedTransFlag[MAX_CHANNEL_COUNT];
	memset(bChannelNoNeedTransFlag,0,MAX_CHANNEL_COUNT);

	for (i = 0;i < m_nChannelCount;i++)
	{
		if (m_atChannelInfo[i].m_byChannelNumber == 0)
		{
			continue;
		}

        if (m_bChangeChannelClr[i])
        {
			bSelRun[i] = true;

		    unsigned long nCounterDiff = CalcCounter(m_nChannelCounter[i], nGlobalCounter, C_N_MAXGLOBALCOUNTER);

            m_nChannelDurationTime[i] = CalcCounter(m_nChannelDurationCounter[i], nGlobalCounter, C_N_MAXGLOBALCOUNTER) / C_N_TIMER_TIMER_COUNTER;
                
		    if (m_chChannelStage[i] == C_CH_PHASESTAGE_G)
		    {
			    bSelRun[i] = false;

			    if (nGreenTime[i] > 0)
			    {
				    if (nCounterDiff >= (nGreenTime[i] * C_N_TIMER_TIMER_COUNTER) && IsTransChannel(byChannelType, i))
				    {
                        m_nChannelCounter[i] = nGlobalCounter;

					    bIsChgStage = true;
					    m_chChannelStage[i] = C_CH_PHASESTAGE_GF;//切换成绿闪

                        //m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "TransitChannelClr Channel:%d Green To FreenFlash", i);
				    }
			    }
			    else
			    {
                    m_nChannelCounter[i] = nGlobalCounter;

				    bIsChgStage = true;
				    m_chChannelStage[i] = C_CH_PHASESTAGE_GF;//切换成绿闪
			    }
		    }
		    else if (m_chChannelStage[i] == C_CH_PHASESTAGE_GF)
		    {
			    bSelRun[i] = false;

			    if (nGreenFlashTime > 0)
			    {
				    if (nCounterDiff >= (nGreenFlashTime * C_N_TIMER_TIMER_COUNTER))
				    {
                        m_nChannelCounter[i] = nGlobalCounter;

					    bIsChgStage = true;
                        if (m_atChannelInfo[i].m_byChannelControlType != PED_CHA && m_atChannelInfo[i].m_byChannelControlType != OVERLAP_PED_CHA)
                        {
					        m_chChannelStage[i] = C_CH_PHASESTAGE_Y;//切换成黄色

                            //m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "TransitChannelClr Channel:%d GreenFlash To Yellow", i);
                        }
                        else
                        {
                            m_chChannelStage[i] = C_CH_PHASESTAGE_R;//切换成红色

                            //m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "TransitChannelClr Channel:%d GreenFlash To Red", i);
                        }
				    }
			    }
			    else
			    {
                    m_nChannelCounter[i] = nGlobalCounter;

				    bIsChgStage = true;
                    if (m_atChannelInfo[i].m_byChannelControlType != PED_CHA && m_atChannelInfo[i].m_byChannelControlType != OVERLAP_PED_CHA)
                    {
				        m_chChannelStage[i] = C_CH_PHASESTAGE_Y;//切换成黄色
                    }
                    else
                    {
                        m_chChannelStage[i] = C_CH_PHASESTAGE_R;//切换成红色
                    }
			    }
		    }
            else if (m_chChannelStage[i] == C_CH_PHASESTAGE_Y)
		    {
			    bSelRun[i] = false;

			    if (nYellowTime > 0)
			    {
				    if (nCounterDiff >= (nYellowTime * C_N_TIMER_TIMER_COUNTER))
				    {
                        m_nChannelCounter[i] = nGlobalCounter;

					    bIsChgStage = true;
					    m_chChannelStage[i] = C_CH_PHASESTAGE_R;//切换成红色

                        //m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "TransitChannelClr Channel:%d Yellow To Red", i);
				    }
			    }
			    else
			    {
                    m_nChannelCounter[i] = nGlobalCounter;

				    bIsChgStage = true;
				    m_chChannelStage[i] = C_CH_PHASESTAGE_R;//切换成红色
			    }
		    }
		    else if (m_chChannelStage[i] == C_CH_PHASESTAGE_R)
		    {
			    if (nRedTime > 0)
			    {
					if (!m_bChannelTran[i])
					{
						if (nCounterDiff >= (nRedTime * C_N_TIMER_TIMER_COUNTER))
						{
							m_nChannelCounter[i] = nGlobalCounter;

							bIsChgStage = true;
							m_chChannelStage[i] = C_CH_PHASESTAGE_R;//继续保持红色

							m_bChannelTran[i] = true;
							//m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "TransitChannelClr Channel:%d Red To End", i); 
						}
						else
						{
							bSelRun[i] = false;    
						}
					}
					else
					{
						if (byChannelType == CHANNEL_TYPE_LOCK)
						{
							m_bNonTargetLockPhaseEndFlag[i] = true;
						}
					}
			    } 
                else
                {
                     m_nChannelCounter[i] = nGlobalCounter;

				     bIsChgStage = true;
				     m_chChannelStage[i] = C_CH_PHASESTAGE_R;//继续保持红色    

					 m_bChannelTran[i] = true;
                }  
		    }
        }

		if (bIsChgStage && m_bChannelKeepGreenFlag[i])
		{
			m_chChannelStage[i] = C_CH_PHASESTAGE_G;//通道跨方向或跨锁定时，继续保持绿色  
			m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "ChannelID:%d Need Keep Green!", i + 1);
		}

	}

	if (bIsChgStage)
    {
		bIsChgStage = false;

		char * pStartPos = NULL;

        for (i = 0;i < m_nChannelCount;i++)
        {
			if (m_atChannelInfo[i].m_byChannelNumber == 0)
			{
				continue;
			}

            if (m_bChangeChannelClr[i])
            { 
				pStartPos = m_achLampClr + (m_atChannelInfo[i].m_byChannelNumber - 1) * 3;

                if (m_chChannelStage[i] == C_CH_PHASESTAGE_GF)
                {
                    m_pOpenATCRunStatus->SetGreenFlashCount((m_atChannelInfo[i].m_byChannelNumber - 1) * 3 + 2, nGreenFlashTime * 2);
                }   

				if (!m_bNonTargetLockPhaseEndFlag[i])
				{
					SetOneChannelOutput(pStartPos, m_chChannelStage[i]);
				}
				m_bIsDirectionChannelClrChg = true;

				if (m_chChannelStatus[i] != CHANNEL_STATUS_OFF && m_chChannelStatus[i] != CHANNEL_STATUS_RED && m_chChannelStage[i] != C_CH_PHASESTAGE_G && !bLockCmd && !IsChannelNeedTranClr(i))
				{
					if (m_nManualCurStatus == MANUAL_STAGE_TRANS)
					{
						m_chChannelStage[i] = C_CH_PHASESTAGE_F;
					}
					SetOneChannelOutput(pStartPos, C_CH_PHASESTAGE_G);//下一个阶段的相位是通道的母相位，不需要过渡灯色
					m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "Channel:%d No Need Trans Color!", i);
				    bChannelNoNeedTransFlag[i] = true;
				}
            }
        }
    }

	for (i = 0;i < m_nChannelCount;i++)
	{
		if (m_bChannelKeepGreenFlag[i])
		{
			bSelRun[i] = true;//通道锁定时，绿灯继续保持，则可以切换到下一个指令
		}
		if (bChannelNoNeedTransFlag[i])
		{
			bSelRun[i] = true;//通道锁定结束过渡时，下一个阶段的相位是通道的母相位，不需要过渡灯色，则可以切换到下一个指令
		}
		if (m_bChangeChannelClr[i] && !bSelRun[i])
		{
			//1：有通道处于绿或绿闪或黄，则说明通道锁定没有结束
			//2：锁定相位不在同一个阶段时，目标相位一直保持绿，需要过渡的非目标相位成功过渡到目标相位后，锁定相位处于同一个阶段，锁定结束
			for (int nPhaseIndex = 0; nPhaseIndex < MAX_PHASE_COUNT; nPhaseIndex++)
			{
				if (m_tLockPhaseData[nPhaseIndex].nCurLockPhaseID == 0)
				{
					continue;
				}

				if (m_tLockPhaseData[nPhaseIndex].bNeedTransFlag && m_tLockPhaseData[nPhaseIndex].bSwitchSuccessFlag && m_chChannelStage[i] == C_CH_PHASESTAGE_G)
				{
					m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "LockPhaseSwitchToNextStage!");
					return true;
				}
			}

            return false;
		}
	}

	return true;
}

/*==================================================================== 
函数名 ：InitParamBeforeDirection
功能 ：
算法实现 ：在切换方向之前先初始化参数
参数说明 ：tCtlCmd，手动面板命令
返回值说明：无
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 李永萍          创建函数 
====================================================================*/ 
void  CLogicCtlManual::InitParamBeforeDirection(int nNextDirectionIndex)
{
	for (int i = 0;i < MAX_CHANNEL_COUNT;i++)
	{
        m_bChangeChannelClr[i] = false;
        m_nChannelCounter[i] = 0;
        m_nChannelDurationCounter[i] = 0;
        m_chChannelStatus[i] = C_CH_PHASESTAGE_U;
		m_chChannelStage[i] = C_CH_PHASESTAGE_U;
		m_bChannelTran[i] = false;

        m_nChannelDurationTime[i] = 0;
	}
	//m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "Direction Ctl GetChannelData!");
	GetChannelData(nNextDirectionIndex);//切换到方向之前需要读取通道的目标灯色
}

/*==================================================================== 
函数名 ：IsTimeInSpan
功能 ：判断当前时间是否在时间段内
算法实现 ： 
参数说明 ：
           nCurHour，当前时
		   nCurMin，当前分
		   nCurSec，当前秒
           nStartHour，起始时
           nStartMin，起始分
		   nStartSec，起始秒
           nEndHour，结束时
		   nEndMin，结束分
		   nEndSec，结束秒
返回值说明：当前时间是否在时间段内
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 李永萍          创建函数 
====================================================================*/
bool CLogicCtlManual::IsTimeInSpan(int nCurHour, int nCurMin, int nCurSec, int nStartHour, int nStartMin, int nStartSec, int nEndHour, int nEndMin, int nEndSec)
{
    int iBegLessThanEnd = 0; //1-配置的开始时间小于结束时间 0-配置的开始时间大于结束时间

	if (nStartHour < nEndHour || (nStartHour == nEndHour && nStartMin <= nEndMin))
	{
		iBegLessThanEnd = 1;
	}
	else
	{
		iBegLessThanEnd = 0;
	}

	if (iBegLessThanEnd)   //开始时间小于结束时间
	{
		if ((nCurHour > nStartHour || (nCurHour == nStartHour && nCurMin >= nStartMin))
			&& (nCurHour < nEndHour || (nCurHour == nEndHour && nCurMin <= nEndMin)))
		{
			if (nCurHour == nStartHour && nCurMin == nStartMin)
			{
				if (nCurSec < nStartSec)
				{
					return false;
				}
			}

			if (nCurHour == nEndHour && nCurMin == nEndMin)
			{
				if (nCurSec > nEndSec)
				{
					return false;
				}
			}

			return true;
		}
		else
		{
			return false;
		}
	}
	else   //开始时间大于结束时间, 跨天的情况
	{
		if ((nCurHour > nStartHour || (nCurHour == nStartHour && nCurMin >= nStartMin))
			|| (nCurHour < nEndHour || (nCurHour == nEndHour && nCurMin <= nEndMin)))
		{
			if (nCurHour == nStartHour && nCurMin == nStartMin)
			{
				if (nCurSec < nStartSec)
				{
					return false;
				}
			}

			if (nCurHour == nEndHour && nCurMin == nEndMin)
			{
				if (nCurSec > nEndSec)
				{
					return false;
				}
			}

			return true;
		}
		else
		{
			return false;
		}
    }
}

/*==================================================================== 
函数名 ：CheckIfNeedToRunToNextStage
功能 ：正在过渡的相位的绿灯达到切换的条件时，进入过渡状态
算法实现 ： 
参数说明 ：无
返回值说明：true，可以切到下一个阶段，false：不能切到下一个阶段
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 李永萍          创建函数 
====================================================================*/
bool CLogicCtlManual::CheckIfNeedToRunToNextStage(TManualCmd & tValidManualCmd)
{
	bool bFlag = false;
	int  nIndex = 0;
	int  nCnt = 0;

	int nStageIndexTarget = m_tFixTimeCtlInfo.m_nCurStageIndex + 1;
	if (nStageIndexTarget >= m_tRunStageInfo.m_nRunStageCount)
	{
		nStageIndexTarget = 0;
	}

	int  i = 0;
	for (i = 0; i < m_tFixTimeCtlInfo.m_nRingCount; i++)
	{
		nIndex = m_tFixTimeCtlInfo.m_nCurPhaseIndex[i];
		
		if (AdjustPhaseGreenTime(i, nIndex, nStageIndexTarget, MANUAL_PHASE_SPLIT_GREEN))
		{
            nCnt += 1;
		}

		AdjustPedPhaseGreenTime(i, nIndex, nStageIndexTarget, MANUAL_PHASE_SPLIT_GREEN);
	}

	if (nCnt == m_tFixTimeCtlInfo.m_nRingCount)
	{
		bFlag = true;
	}

	return bFlag;
}

/*==================================================================== 
函数名 ：CheckIfNeedToStepForwardToNextStage
功能 ：正在过渡的相位的绿灯达到切换的条件时，回其他非步进控制模式时，有设置过持续时长的，超过持续时长再切换灯色，否则按最小绿
算法实现 ： 
参数说明 ：tValidManualCmd：手动指令
返回值说明：true，可以切到下一个阶段，false：不能切到下一个阶段
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 陈涵燕          创建函数 
====================================================================*/
bool CLogicCtlManual::CheckIfNeedToStepForwardToNextStage(TManualCmd & tValidManualCmd)
{
	PTPhase pPhaseInfo;
	PTRingCtlInfo ptRingRunInfo;

	bool bFlag = false;
	int nIndex = 0;
	int nCnt = 0;
	int nCurStageIndex	= m_tFixTimeCtlInfo.m_nCurStageIndex;
	
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
			// 判断当前相位是否跨下一个阶段和当前阶段
			if (m_tRunStageInfo.m_PhaseRunstageInfo[nCurStageIndex].m_nConcurrencyPhase[i] == m_tRunStageInfo.m_PhaseRunstageInfo[nStageIndexTarget].m_nConcurrencyPhase[i])
			{
				if (tValidManualCmd.m_bDirectionCmd || tValidManualCmd.m_bChannelLockCmd)//步进切方向或通道锁定，跨阶段的相位需要在当前阶段立刻过渡完
				{
					if (AdjustPhaseGreenTime(i, nIndex, nStageIndexTarget, MANUAL_MINI_GREEN))
					{
						nCnt++;
					}

					AdjustPedPhaseGreenTime(i, nIndex, nStageIndexTarget, MANUAL_MINI_GREEN);
				}
				else
				{
					nCnt++;
				}
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
参数说明 ：nGreenPhaseCount，绿灯相位数量，nClosePhaseCount，关闭相位数量
返回值说明：当前相位是否处于绿灯
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 陈涵燕          创建函数 
====================================================================*/
void CLogicCtlManual::CheckIfStageHavePhaseInGreen(int & nGreenPhaseCount, int & nClosePhaseCount)
{
	PTRingCtlInfo ptRingRunInfo;
	int i = 0;
	int nIndex = 0;

	int nStageIndex = 0;
	int nNextStageIndex = 0;
	int nPhaseID = 0;

	nGreenPhaseCount = 0;
	nClosePhaseCount = 0;

	for (i=0; i<m_tFixTimeCtlInfo.m_nRingCount; i++)
	{
		nStageIndex = m_tFixTimeCtlInfo.m_nCurStageIndex;
		nNextStageIndex = nStageIndex + 1;
		if (nNextStageIndex == m_tRunStageInfo.m_nRunStageCount)
		{
			nNextStageIndex = 0;
		}

		nIndex = m_tFixTimeCtlInfo.m_nCurPhaseIndex[i];
		ptRingRunInfo = &(m_tFixTimeCtlInfo.m_atPhaseSeq[i]);
		nPhaseID = ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhaseNumber;
		if (ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseStage == C_CH_PHASESTAGE_G)
		{
			nGreenPhaseCount++;
		}
		else if (m_tPhasePassCmdPhaseStatus.m_nPhasePassStatus[nPhaseID - 1] == PhasePassStatus_Close)
		{
			nClosePhaseCount++;
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
2019/09/14     V1.0 陈涵燕          创建函数 
====================================================================*/ 
void CLogicCtlManual::SendCountDownPulse(int nRingIndex, bool bNeedSendRedPulseFlag)
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


		if ((m_tPhasePassCmdPhaseStatus.m_nPhasePassStatus[nCurPhaseID - 1] == PhasePassStatus_Close && ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseStage == C_CH_PHASESTAGE_U) ||
			 ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseMode == NEGLECT_MODE || ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseMode == SHIELD_MODE)
		{
			// 当前相位为关闭放行状态，且未放行过，不发绿脉冲
		}
		else
		{			
			if (ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseStage == C_CH_PHASESTAGE_G && tRunCounter.m_nLampClrTime[nRingIndex] == 
				(nGreenTime + ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byGreenFlash - ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byGreenPulse))
			{
				if (!bGreenPulse[nRingIndex] && CheckSendGreenPulse(ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhaseNumber))
				{
					bGreenPulse[nRingIndex] = true;
					SetGreenLampPulse(VEH_CHA, ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhaseNumber, true);

					m_tPhasePulseStatus[nRingIndex].m_nPhaseIndex = nIndex;
					m_tPhasePulseStatus[nRingIndex].m_nPhaseNum = ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhaseNumber;
					m_tPhasePulseStatus[nRingIndex].m_bGreenPulseStatus = true;

					nGreenPulseIndex[nRingIndex] = nIndex;
					nGreenPulseNextStageIndex[nRingIndex] = nNextStageIndex;
					m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "SendCountDownPulse GreenPulse On RingIndex:%d PhaseIndex:%d, nPhaseTime:%d.", nRingIndex, nIndex, nPhaseTime);
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
		}

		if (m_tPhasePassCmdPhaseStatus.m_nPhasePassStatus[nNextPhaseID - 1] == PhasePassStatus_Close || !bNeedSendRedPulseFlag ||
			ptRingRunInfo->m_atPhaseInfo[nNextIndex].m_chPhaseMode == NEGLECT_MODE || ptRingRunInfo->m_atPhaseInfo[nNextIndex].m_chPhaseMode == SHIELD_MODE)
		{
			// 下一相位为关闭放行状态，不发红脉冲
		}
		else
		{
			if (ptRingRunInfo->m_atPhaseInfo[nNextIndex].m_tPhaseParam.m_byRedPulse != 0 && GetPhaseRemainTime(nRingIndex, (PTPhaseCtlInfo)&(ptRingRunInfo->m_atPhaseInfo[nIndex]), tRunCounter, nPhaseTime) == ptRingRunInfo->m_atPhaseInfo[nNextIndex].m_tPhaseParam.m_byRedPulse)
			{
				if (!bRedPulse[nRingIndex])
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
2019/09/14     V1.0 陈涵燕         创建函数 
====================================================================*/ 
void CLogicCtlManual::SendPedCountDownPulse(int nRingIndex, bool bNeedSendRedPulseFlag)
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

		if ((m_tPhasePassCmdPhaseStatus.m_nPhasePassStatus[nCurPhaseID - 1] == PhasePassStatus_Close && ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseStage == C_CH_PHASESTAGE_U) ||
			 ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseMode == NEGLECT_MODE || ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseMode == SHIELD_MODE)
		{
			// 当前相位为关闭放行状态，且未放行过，不发绿脉冲
		}
		else
		{			
			if (ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPedPhaseStage == C_CH_PHASESTAGE_G && tRunCounter.m_nPedLampClrTime[nRingIndex] == 
				(nPedGreenTime + ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhasePedestrianClear - ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byGreenPulse))
			{
				if (!bGreenPulse[nRingIndex] && CheckSendGreenPulse(ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhaseNumber))
				{
					bGreenPulse[nRingIndex] = true;
					SetGreenLampPulse(PED_CHA, ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhaseNumber, true);

					nGreenPulseIndex[nRingIndex] = nIndex;
					nGreenPulseNextStageIndex[nRingIndex] = nNextStageIndex;
					m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "SendPedCountDownPulse GreenPulse On RingIndex:%d PhaseIndex:%d", nRingIndex, nIndex);
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
		}

		if (m_tPhasePassCmdPhaseStatus.m_nPhasePassStatus[nNextPhaseID - 1] == PhasePassStatus_Close || !bNeedSendRedPulseFlag ||
			ptRingRunInfo->m_atPhaseInfo[nNextIndex].m_chPhaseMode == NEGLECT_MODE || ptRingRunInfo->m_atPhaseInfo[nNextIndex].m_chPhaseMode == SHIELD_MODE)
		{
			// 下一相位为关闭放行状态，不发红脉冲
		}
		else
		{
			if (ptRingRunInfo->m_atPhaseInfo[nNextIndex].m_tPhaseParam.m_byRedPulse != 0 && 
				GetPedPhaseRemainTime(nRingIndex, (PTPhaseCtlInfo)&(ptRingRunInfo->m_atPhaseInfo[nIndex]), tRunCounter, nPhaseTime) == ptRingRunInfo->m_atPhaseInfo[nNextIndex].m_tPhaseParam.m_byRedPulse)
			{
				if (!bRedPulse[nRingIndex])
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
函数名 ：ProcessStepward
功能 ：处理步进。
算法实现 ： 
参数说明 ：tValidManualCmd，手动命令
返回值说明：无
 ----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 李永萍          创建函数 
====================================================================*/ 
void CLogicCtlManual::ProcessStepward(TManualCmd  & tValidManualCmd)
{
	int i = 0;
	int j = 0;

	if (tValidManualCmd.m_bNewCmd)
	{
		int nRet = ProcessLockChannelToPanel(tValidManualCmd);
		if (!nRet || nRet == LOCKCHANNEL_TO_DIRECTION)
		{
			return;
		}

		if (tValidManualCmd.m_nSubCtlMode != CTL_MANUAL_SUBMODE_PANEL_FITSTMANUAL)
		{
			RecalculteCurPhaseGreenTimeAndSetStatus(tValidManualCmd);
		}
		else
		{
			m_bClearPulseFlag = false;//系统控制切到手动控制时，要先清除脉冲
			ProcessFirstManual(tValidManualCmd);
		}
	}
  
    for (i = 0;i < m_tFixTimeCtlInfo.m_nRingCount;i ++)
	{
		ManualOnePhaseRun(i, tValidManualCmd);
	    ManualOnePedPhaseRun(i, tValidManualCmd); 
	}

	SysCtlOnePhaseRun(tValidManualCmd); 
	
	if (tValidManualCmd.m_bStepForwardCmd)
	{
		if (tValidManualCmd.m_tStepForwardCmd.m_byStepType == STEP_COLOR && tValidManualCmd.m_tStepForwardCmd.m_nNextStageID == 0)//色步步进每运行一次，要清一次步进状态
		{
			tValidManualCmd.m_bNewCmd = false;
			tValidManualCmd.m_bStepForwardCmd = false;

			//色步步进，灯色从红切到可结束时，步进状态不清零，这样直接切到下一个相位的绿，否则要多按一次步进
			if (m_bNextClrStageIsFInColorStep)
			{
				tValidManualCmd.m_bStepForwardCmd = true;
			}
			m_pOpenATCRunStatus->SetValidManualCmd(tValidManualCmd);
		}
	}
	else
	{
		if (m_bNextClrStageIsFInColorStep && tValidManualCmd.m_tStepForwardCmd.m_byStepType == STEP_COLOR && tValidManualCmd.m_tStepForwardCmd.m_nNextStageID == 0)
		{
			//色步步进，灯色从红切到可结束时，步进状态不清零，这样直接切到下一个相位的绿，否则要多按一次步进
			tValidManualCmd.m_bStepForwardCmd = true;
			m_pOpenATCRunStatus->SetValidManualCmd(tValidManualCmd);
		}
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
2019/09/14     V1.0 陈涵燕          创建函数 
====================================================================*/ 
int CLogicCtlManual::GetPhaseRemainTime(int nRingIndex, TPhaseCtlInfo * pPhaseCtlInfo, TPhaseLampClrRunCounter tRunCounter, DWORD wPhaseTime)
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
2019/09/14     V1.0 陈涵燕          创建函数 
====================================================================*/ 
int CLogicCtlManual::GetPedPhaseRemainTime(int nRingIndex, TPhaseCtlInfo * pPhaseCtlInfo, TPhaseLampClrRunCounter tRunCounter, DWORD wPhaseTime)
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
2019/09/14     V1.0 陈涵燕          创建函数 
====================================================================*/ 
void CLogicCtlManual::SendRedPulse(int nPhaseType, PTRingCtlInfo ptRingRunInfo, int nRingIndex, int nIndex, int nNextIndex, bool bFlag)
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
2019/09/14     V1.0  陈涵燕         创建函数 
====================================================================*/ 
void  CLogicCtlManual::ResetNextStagePhaseGreenTime(int nRingIndex, int nPhaseIndex, int nNextStageIndex, bool bChangePhaseFlag, int nDuration)
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
		if (m_nReturnAutoCtrlStageIndex == m_tFixTimeCtlInfo.m_nCurStageIndex)												//已经切到下一个阶段，接下来准备过渡回自主的第一个阶段的相位时间
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
功能 ：根据延迟时间和持续时间重新计算手动控制时的绿灯时间
算法实现 ： 
参数说明 ：tValidManualCmd，手动命令
返回值说明：无
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 陈涵燕          创建函数 
====================================================================*/ 
void CLogicCtlManual::RecalculteStageTimeByDelayTimeAndDuration(TManualCmd  tValidManualCmd)
{
	TPhaseLampClrRunCounter tRunCounter;
	m_pOpenATCRunStatus->GetPhaseLampClrRunCounter(tRunCounter);
	
	int nStageIndexTarget = 0;
	if (tValidManualCmd.m_tStepForwardCmd.m_nNextStageID == 0)
	{
		nStageIndexTarget = m_tFixTimeCtlInfo.m_nCurStageIndex + 1;
	}
	else
	{
		nStageIndexTarget = tValidManualCmd.m_tStepForwardCmd.m_nNextStageID - 1;
	}

	if (nStageIndexTarget >= m_tRunStageInfo.m_nRunStageCount)
	{
		nStageIndexTarget = 0;
	}

	int i = 0;
	int nPhaseID = 0;
	int nGreenTime = 0;
	int	nGreenPhaseCount = 0;
	int nClosePhaseCount = 0; 
	CheckIfStageHavePhaseInGreen(nGreenPhaseCount, nClosePhaseCount);

	TAscManualPanel tAscManualPanel;
    memset(&tAscManualPanel, 0, sizeof(TAscManualPanel)); 
    m_pOpenATCParameter->GetManualPanelInfo(tAscManualPanel);

	if (nGreenPhaseCount > 0)
	{
		for (i = 0;i < m_tFixTimeCtlInfo.m_nRingCount;i++)
		{
			PTRingCtlInfo ptRingRunInfo = &(m_tFixTimeCtlInfo.m_atPhaseSeq[i]);
			int nIndex = m_tFixTimeCtlInfo.m_nCurPhaseIndex[i];
			PTPhaseCtlInfo pPhaseInfo = (PTPhaseCtlInfo)&(ptRingRunInfo->m_atPhaseInfo[nIndex]);
			nPhaseID = pPhaseInfo->m_tPhaseParam.m_byPhaseNumber;

			if ((ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseStage == C_CH_PHASESTAGE_G 
				&& m_tPhasePassCmdPhaseStatus.m_nPhasePassStatus[nPhaseID-1] != PhasePassStatus_Close) 
				|| m_tPhasePassCmdPhaseStatus.m_nPhasePassStatus[nPhaseID-1] == PhasePassStatus_Close)
			{
				if (tRunCounter.m_nLampClrTime[i] < pPhaseInfo->m_tPhaseParam.m_wPhaseMinimumGreen)
				{
					nGreenTime = pPhaseInfo->m_tPhaseParam.m_wPhaseMinimumGreen + tValidManualCmd.m_tStepForwardCmd.m_nDelayTime;
				}
				else
				{
					nGreenTime = tRunCounter.m_nLampClrTime[i] + tValidManualCmd.m_tStepForwardCmd.m_nDelayTime;
				}

				if (tValidManualCmd.m_nSubCtlMode == CTL_MANUAL_SUBMODE_PANEL_STEPFOWARD && tAscManualPanel.m_byMinGreenIgnore == 1)//忽略最小绿
				{
					nGreenTime = tRunCounter.m_nLampClrTime[i] + tValidManualCmd.m_tStepForwardCmd.m_nDelayTime;
				}

				if (m_tFixTimeCtlInfo.m_nCurStageIndex == nStageIndexTarget)
				{
					nGreenTime = nGreenTime + tValidManualCmd.m_tStepForwardCmd.m_nDurationTime;//阶段驻留，加上持续时间
				}

				pPhaseInfo->m_wPhaseTime = nGreenTime + 
					pPhaseInfo->m_tPhaseParam.m_byGreenFlash + pPhaseInfo->m_tPhaseParam.m_byPhaseYellowChange + 
					pPhaseInfo->m_tPhaseParam.m_byPhaseRedClear;

				//关断相位的绿灯计算已经在收到关断相位时计算过
				if (m_tPhasePassCmdPhaseStatus.m_nPhasePassStatus[nPhaseID-1] != PhasePassStatus_Close)
				{
					pPhaseInfo->m_wPhaseGreenTime = nGreenTime;				

					if (ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPedPhaseStage == C_CH_PHASESTAGE_G)
					{
						pPhaseInfo->m_wPedPhaseGreenTime = pPhaseInfo->m_wPhaseTime - pPhaseInfo->m_tPhaseParam.m_byPhasePedestrianClear
															- pPhaseInfo->m_tPhaseParam.m_byPhaseYellowChange 
															- pPhaseInfo->m_tPhaseParam.m_byPhaseRedClear;

						AdjustPedPhaseGreenTime(i, nIndex, nStageIndexTarget, MANUAL_PHASE_RUN_GREEN);
					}
				}

				AdjustPhaseGreenTime(i, nIndex, nStageIndexTarget, MANUAL_PHASE_RUN_GREEN);
			
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "RecalculteStageTimeByDelayTimeAndDuration RingIndex:%d GreenTime:%d, PedPhaseGreenTime:%d.", i, pPhaseInfo->m_wPhaseGreenTime, pPhaseInfo->m_wPedPhaseGreenTime);
				//步进控制时，行人走最小绿，单独计算
				if (m_tFixTimeCtlInfo.m_nCurStageIndex == nStageIndexTarget)
				{
					if (tValidManualCmd.m_tStepForwardCmd.m_nDurationTime == 0)
					{
						//卡住不动给最大值
						if (m_tPhasePassCmdPhaseStatus.m_nPhasePassStatus[nPhaseID-1] != PhasePassStatus_Close)
						{
							pPhaseInfo->m_wPhaseGreenTime = MAX_GREEN_TIME;
						
							AdjustPedPhaseGreenTime(i, nIndex, nStageIndexTarget, MANUAL_MINI_GREEN);
						}
						else
						{
							pPhaseInfo->m_wPhaseTime = MAX_GREEN_TIME;
						}
						m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "RecalculteStageTimeByDelayTimeAndDuration nStageIndexTarget:%d MaxGreenTime:%d, PedPhaseGreenTime:%d.", tValidManualCmd.m_tStepForwardCmd.m_nNextStageID, pPhaseInfo->m_wPhaseGreenTime, pPhaseInfo->m_wPedPhaseGreenTime);
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
		}

		ReSetNeglectPhaseStageRunTime();
	}

	m_nNextStageIndex = nStageIndexTarget;

	if (tValidManualCmd.m_tStepForwardCmd.m_nDurationTime > 0)
	{
		m_nReturnAutoCtrlStageIndex = nStageIndexTarget;
	}
	else
	{
		m_nReturnAutoCtrlStageIndex = -1;
	}
}

/*==================================================================== 
函数名 ：RecalculteCurPhaseGreenTimeAndSetStatus
功能 ：重新计算手动控制时的绿灯时间和状态
算法实现 ： 
参数说明 ：tValidManualCmd，手动命令
返回值说明：无
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 陈涵燕          创建函数 
====================================================================*/ 
void  CLogicCtlManual::RecalculteCurPhaseGreenTimeAndSetStatus(TManualCmd  & tValidManualCmd)
{
	int i = 0;
	int nStageIndexTarget = 0;
	int nRefreshStageTime = 0;
	if (tValidManualCmd.m_tStepForwardCmd.m_nNextStageID == 0)
	{
		nStageIndexTarget = m_tFixTimeCtlInfo.m_nCurStageIndex + 1;
	}
	else
	{
		nStageIndexTarget = tValidManualCmd.m_tStepForwardCmd.m_nNextStageID - 1;
	}

	if (nStageIndexTarget >= m_tRunStageInfo.m_nRunStageCount)
	{
		nStageIndexTarget = 0;
	}
	
	//步进到非步进，计算当前阶段对应的相位的绿灯时间
	if (!tValidManualCmd.m_bStepForwardCmd && m_tOldValidManualCmd.m_bStepForwardCmd && m_nManualCurStatus != MANUAL_STAGE_TRANS &&
		(tValidManualCmd.m_bDirectionCmd || tValidManualCmd.m_bPatternInterruptCmd || tValidManualCmd.m_bChannelLockCmd))//在步进时，按了自主或其他非步进控制模式
	{
		if (m_bTransToAutoFlag)
		{
			m_bTransToAutoFlag = false;
			m_nManualCurStatus = MANUAL_STAGE_TRANS;
		 	
			memcpy(&m_tOldValidManualCmd, &tValidManualCmd, sizeof(tValidManualCmd));
			m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "StepToNoStep, Set Trans Status Base On TransToAutoFlag");
		}
		else
		{
			SetTransStatus(tValidManualCmd);
		}
		nRefreshStageTime = STEPWARD_TO_NONSTEPWARD;
	}
	else if (tValidManualCmd.m_bStepForwardCmd && m_tOldValidManualCmd.m_bStepForwardCmd &&//步进到步进，计算当前阶段对应的相位的绿灯时间
			tValidManualCmd.m_nCtlMode == CTL_MODE_MANUAL &&
			tValidManualCmd.m_nSubCtlMode != CTL_MANUAL_SUBMODE_DEFAULT)
	{
		if (m_tOldValidManualCmd.m_nSubCtlMode != CTL_MANUAL_SUBMODE_PANEL_FITSTMANUAL)
		{
			if (m_nManualCurStatus == MANUAL_STAGE_TRANS)
			{
				m_nManualCurStatus = MANUAL_CONTROL_STATUS;
			}
			m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "StepToStep, RecalculteStageTimeByDelayTimeAndDuration");
			RecalculteStageTimeByDelayTimeAndDuration(tValidManualCmd);
			memcpy(&m_tOldValidManualCmd, &tValidManualCmd, sizeof(tValidManualCmd));
			nRefreshStageTime = STEPWARD_TO_STEPWARD;
		}
		else //面板第一次按步进，卡住不动
		{
			if (m_nManualCurStatus == MANUAL_STAGE_TRANS)
			{
				m_nManualCurStatus = MANUAL_CONTROL_STATUS;
			}

			PanelFirstStepWard(nStageIndexTarget);
			memcpy(&m_tOldValidManualCmd, &tValidManualCmd, sizeof(tValidManualCmd));
			nRefreshStageTime = FIRSTPANEL_TO_FIRSTSTEPWARD;
		}
	}
	else if (tValidManualCmd.m_bStepForwardCmd && m_tOldValidManualCmd.m_nSubCtlMode == CTL_MANUAL_SUBMODE_SYSTEM_CHANNEL_LOCK &&
		     tValidManualCmd.m_nSubCtlMode == CTL_MANUAL_SUBMODE_PANEL_STEPFOWARD)//从通道锁定到步进
	{
		if (m_nManualCurStatus == MANUAL_STAGE_TRANS)
		{
			m_nManualCurStatus = MANUAL_CONTROL_STATUS;
		}

		PanelFirstStepWard(nStageIndexTarget);
		memcpy(&m_tOldValidManualCmd, &tValidManualCmd, sizeof(tValidManualCmd));
		nRefreshStageTime = FIRSTPANEL_TO_FIRSTSTEPWARD;
	}
	else if (tValidManualCmd.m_bStepForwardCmd  && !m_tOldValidManualCmd.m_bStepForwardCmd)//从非步进到步进，配置工具下发的指令，目前只有这种情况
	{
		//从过渡状态再次收到平台指令，重新进入控制状态
		if (m_nManualCurStatus == MANUAL_STAGE_TRANS)
		{
			m_nManualCurStatus = MANUAL_CONTROL_STATUS;
		}

		if (tValidManualCmd.m_tStepForwardCmd.m_nDelayTime != 0 || tValidManualCmd.m_tStepForwardCmd.m_nDurationTime != 0)
		{
			m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "NonStepToStep, RecalculteStageTimeByDelayTimeAndDuration");
			RecalculteStageTimeByDelayTimeAndDuration(tValidManualCmd);
		}
		else
		{
			NonStepWardToStepWard(nStageIndexTarget);	
			m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "NonStepToStep, Trans To Stage:%d", m_nNextStageIndex);

			if (m_tOldValidManualCmd.m_bDirectionCmd && m_tOldValidManualCmd.m_nSubCtlMode == CTL_MANUAL_SUBMODE_DEFAULT)
			{
				//面板按了方向但是还没有运行到方向，又按下了步进按钮，需要清除那些因为当前通道和方向一致需要继续保持绿的标志
				memset(m_bKeepGreenChannelBeforeControlChannelFlag, false, sizeof(m_bKeepGreenChannelBeforeControlChannelFlag));
				tValidManualCmd.m_nSubCtlMode = CTL_MANUAL_SUBMODE_PANEL_STEPFOWARD;
			}
		}

		memcpy(&m_tOldValidManualCmd, &tValidManualCmd, sizeof(tValidManualCmd));
		nRefreshStageTime = NONSTEPWARD_TO_STEPWARD;
	}
	else if ((m_tOldValidManualCmd.m_nSubCtlMode == CTL_MANUAL_SUBMODE_PANEL_FITSTMANUAL || m_tOldValidManualCmd.m_nSubCtlMode == CTL_MANUAL_SUBMODE_SYSTEM_CHANNEL_LOCK) &&//从通道锁定切到方向或自主
		    (tValidManualCmd.m_bDirectionCmd || //从面板手动切到方向
		     tValidManualCmd.m_bPatternInterruptCmd))//从面板手动切到自主
	{
		FirstPanelManualSwitchToDirectionOrPattern(tValidManualCmd);
		if (tValidManualCmd.m_bDirectionCmd)
		{
			nRefreshStageTime = FIRSTPANEL_TO_DIRECTION;
		}
		else
		{
			nRefreshStageTime = FIRSTPANEL_TO_PATTERN;
			for (i = 0;i < MAX_CHANNEL_COUNT;i++)
			{
				m_bKeepGreenChannelBeforeControlChannelFlag[i] = false;
			}
		}
	}
	else if (m_tOldValidManualCmd.m_nSubCtlMode == CTL_MANUAL_SUBMODE_DEFAULT &&
		    (tValidManualCmd.m_bChannelLockCmd || //在有方案的控制方式下，下发了通道锁定指令，从默认模式切到锁定指令
		     tValidManualCmd.m_bPatternInterruptCmd))//在有方案的控制方式下，下发了通道锁定指令，还没有切到通道锁定指令，又下发了方案干预
	{
		PatternSwitchToChannelLock(tValidManualCmd);
		if (tValidManualCmd.m_bChannelLockCmd)
		{
			m_nManualCurStatus = MANUAL_STAGE_TRANS;
			nRefreshStageTime = PATTERN_TO_CHANNELLOCK;
		}
		else
		{
			m_nManualCurStatus = MANUAL_STAGE_TRANS;
			nRefreshStageTime = PATTERN_TO_PATTERN;
			PhaseTransBasedOnControlChannelFlag();
            for (i = 0;i < MAX_CHANNEL_COUNT;i++)
			{
				m_bKeepGreenChannelBeforeControlChannelFlag[i] = false;
			}
		}
	}
	else 
	{	
		//方向结束以后自己回到自主,通道锁定结束以后自己回到自主
		if (tValidManualCmd.m_bPatternInterruptCmd  && (m_tOldValidManualCmd.m_bDirectionCmd || m_tOldValidManualCmd.m_bChannelLockCmd))
		{
			m_nManualCurStatus = MANUAL_STAGE_TRANS;
		 	
			memcpy(&m_tOldValidManualCmd, &tValidManualCmd, sizeof(tValidManualCmd));
			m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "DirectionToSelf, Set Trans Status");
		}

		//从面板方向回自主
		if (tValidManualCmd.m_bPatternInterruptCmd && m_tOldValidManualCmd.m_nSubCtlMode == CTL_MANUAL_SUBMODE_PANEL_DIRECTION &&
			m_nManualCurStatus == MANUAL_STAGE_TRANS)
		{
			memcpy(&m_tOldValidManualCmd, &tValidManualCmd, sizeof(tValidManualCmd));
			m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "In DirectionToSelf, Start Trans");
		}
		//从通道锁定回自主
		if (tValidManualCmd.m_bPatternInterruptCmd && m_tOldValidManualCmd.m_nSubCtlMode == CTL_MANUAL_SUBMODE_SYSTEM_CHANNEL_LOCK && 
			m_nManualCurStatus == MANUAL_STAGE_TRANS)
		{
			memcpy(&m_tOldValidManualCmd, &tValidManualCmd, sizeof(tValidManualCmd));
			m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "In ChannelLockToSelf, Start Trans");
		}
	}

	if (memcmp(&m_tOldValidManualCmd, &tValidManualCmd, sizeof(tValidManualCmd)) == 0)
	{
		if (nRefreshStageTime == FIRSTPANEL_TO_FIRSTSTEPWARD)
		{
			tValidManualCmd.m_bStepForwardCmd = false;//面板手动指令按下后，第一次按下步进，要卡在当前状态，所以步进状态要清除。
		}

		tValidManualCmd.m_bNewCmd = false;
		m_pOpenATCRunStatus->SetValidManualCmd(tValidManualCmd);

		m_tOldValidManualCmd.m_bNewCmd = false;
	}

	if (nRefreshStageTime > 0)
	{
		BackUpPhaseTime(nRefreshStageTime);
		if (nRefreshStageTime == PATTERN_TO_CHANNELLOCK || nRefreshStageTime == FIRSTPANEL_TO_DIRECTION)
		{
			m_bClearPulseFlag = false;//切到通道锁定或方向时，有可能给下个相位的红脉冲已经发过，但是还没有来得及清除
		}
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
void CLogicCtlManual::InitNextStagePhase(int nRingIndex, int nPhaseIndex, int nNextStageIndex, bool bChangePhaseFlag)
{
	if (bChangePhaseFlag)//切阶段，切相位
	{
		m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[m_tFixTimeCtlInfo.m_nCurPhaseIndex[nRingIndex]].m_chPhaseStage = C_CH_PHASESTAGE_F;   
		m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[m_tFixTimeCtlInfo.m_nCurPhaseIndex[nRingIndex]].m_chPedPhaseStage = C_CH_PHASESTAGE_F; 

		m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_chPhaseStage = C_CH_PHASESTAGE_U;   
		m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_chPedPhaseStage = C_CH_PHASESTAGE_U; 

		TAscStepCfg tStepCfg;
		memset(&tStepCfg, 0, sizeof(tStepCfg));
		m_pOpenATCParameter->GetStepInfo(tStepCfg);
		if (tStepCfg.m_byStepType == STEP_COLOR)
		{
			m_pOpenATCRunStatus->SetIncludedCurPhaseInPhaseStatusCmdInStepColorFlag(false);
		}

		if (m_tFixTimeCtlInfo.m_nCurStageIndex != nNextStageIndex)
		{
			m_tFixTimeCtlInfo.m_nCurPhaseIndex[nRingIndex] = nPhaseIndex;  
		}

		m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_wPhaseStageRunTime = 0;
		m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_wPedPhaseStageRunTime = 0;
	}
}

/*==================================================================== 
函数名 ：CreateManualCmdReturnToSelf
功能 ：生成回到自主的手动命令
算法实现 ： 
参数说明 ：tValidManualCmd：手动指令
返回值说明：无
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 李永萍          创建函数 
====================================================================*/ 
void CLogicCtlManual::CreateManualCmdReturnToSelf(TManualCmd tValidManualCmd)
{
	TManualCmd tManualCmd;
	memset(&tManualCmd,0,sizeof(TManualCmd));
	tManualCmd.m_bNewCmd = true;
	tManualCmd.m_nCmdSource = tValidManualCmd.m_nCmdSource;//命令来源保留，因为LogicManager处理面板指令和系统指令的时候，是根据命令源来判断的
	tManualCmd.m_nCurCtlSource = tValidManualCmd.m_nCurCtlSource;//控制源保留
	memcpy(tManualCmd.m_szPeerIp,tValidManualCmd.m_szPeerIp,strlen(tValidManualCmd.m_szPeerIp));
	tManualCmd.m_bStepForwardCmd = false;
	memset(&tManualCmd.m_tStepForwardCmd,0,sizeof(TStepForwardCmd));
	tManualCmd.m_bDirectionCmd = false;
	memset(&tManualCmd.m_tDirectionCmd,0,sizeof(TDirectionCmd));
	tManualCmd.m_bPatternInterruptCmd = true;
	tManualCmd.m_tPatternInterruptCmd.m_nControlMode = tValidManualCmd.m_tPatternInterruptCmd.m_nControlMode;
	tManualCmd.m_tPatternInterruptCmd.m_nPatternNo = tValidManualCmd.m_tPatternInterruptCmd.m_nPatternNo;
	memcpy(&tManualCmd.m_tPatternInterruptCmd.m_tManualControlPattern, &tValidManualCmd.m_tPatternInterruptCmd.m_tManualControlPattern, sizeof(tValidManualCmd.m_tPatternInterruptCmd.m_tManualControlPattern));
	tValidManualCmd.m_bChannelLockCmd = false;
	memset(&tValidManualCmd.m_tChannelLockCmd,0,sizeof(TChannelLockCmd));
	memset(&tValidManualCmd.m_tPhaseLockPara,0,sizeof(TPhaseLockPara));
	tValidManualCmd.m_bPhaseToChannelLock = false;
	m_pOpenATCRunStatus->SetManualCmd(tManualCmd);
}

/*==================================================================== 
函数名 ：SetTransStatus
功能 ：设置过渡状态
算法实现 ： 
参数说明 ：tValidManualCmd：手动指令
返回值说明：无
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 李永萍          创建函数 
====================================================================*/ 
void  CLogicCtlManual::SetTransStatus(TManualCmd  & tValidManualCmd)
{
	int	nGreenPhaseCount = 0;
	int nClosePhaseCount = 0; 
	CheckIfStageHavePhaseInGreen(nGreenPhaseCount, nClosePhaseCount);
	if (nGreenPhaseCount > 0)
	{
		//判断正在过渡的相位的绿灯是否达到切换的条件，回其他非步进控制模式时，有设置过持续时长的，超过持续时长再切换灯色，否则按最小绿
		if (CheckIfNeedToStepForwardToNextStage(tValidManualCmd))
		{
			m_nManualCurStatus = MANUAL_STAGE_TRANS;
			
			//通道锁定时,暂时先不备份指令
			if (!tValidManualCmd.m_bChannelLockCmd)
			{
				memcpy(&m_tOldValidManualCmd, &tValidManualCmd, sizeof(tValidManualCmd));//步进切方向或自主时，指令备份
			}
			m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "StepToNoStep, CheckIfNeedToStepForwardToNextStage, Set Trans Status");
		}	
	}
	else
	{
		m_nManualCurStatus = MANUAL_STAGE_TRANS;
		//通道锁定时,暂时先不备份指令
		if (!tValidManualCmd.m_bChannelLockCmd)
		{
			memcpy(&m_tOldValidManualCmd, &tValidManualCmd, sizeof(tValidManualCmd));//步进切方向或自主时，指令备份
		}
		m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "StepToNoStep, Non Green, Set Trans Status");
	}

	ReSetNeglectPhaseStageRunTime();
}

/*==================================================================== 
函数名 ：ProcessFirstManualProcessFirstManual
功能 ：处理第一次按下手动面板按钮，进入待命状态
算法实现 ： 
参数说明 ：tValidManualCmd：手动指令
返回值说明：无
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 李永萍          创建函数 
====================================================================*/ 
void  CLogicCtlManual::ProcessFirstManual(TManualCmd  & tValidManualCmd)
{
	int	nGreenPhaseCount = 0;
	int nClosePhaseCount = 0; 
	CheckIfStageHavePhaseInGreen(nGreenPhaseCount, nClosePhaseCount);
	if (nGreenPhaseCount > 0)
	{
		if (CheckIfNeedToRunToNextStage(tValidManualCmd))
		{
			m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "Panel Fisrt Manual, CheckIfNeedToRunToNextStage, Set Trans Status");
		}	
	}
	else
	{
		m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "Panel Fisrt Manual, Non Green, Set Trans Status");
	}

	if (memcmp(&m_tOldValidManualCmd, &tValidManualCmd, sizeof(tValidManualCmd)) != 0)
	{
		memcpy(&m_tOldValidManualCmd, &tValidManualCmd, sizeof(tValidManualCmd));//手动切步进，方向和自主时，指令备份
	}

	m_nManualCurStatus = MANUAL_STAGE_TRANS;

	tValidManualCmd.m_bNewCmd = false;
	m_pOpenATCRunStatus->SetValidManualCmd(tValidManualCmd);

	m_tOldValidManualCmd.m_bNewCmd = false;

	m_nReturnAutoCtrlStageIndex = -1;//如果之前是系统步进或者跳阶段，面板第一次按下手动后要给默认值，

	ReSetNeglectPhaseStageRunTime();
}

/*==================================================================== 
函数名 ：ProcessPhasePassControlStatus
功能 ：处理相位放行控制状态值
算法实现 ： 
参数说明 ：无
返回值说明：无
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 李永萍          创建函数 
====================================================================*/
void CLogicCtlManual::ProcessPhasePassControlStatus(TManualCmd  & tValidManualCmd)
{
	memset(&m_tPhasePassCmdPhaseStatusFromUser, 0, sizeof(m_tPhasePassCmdPhaseStatusFromUser));
	m_pOpenATCRunStatus->GetPhasePassCmdPhaseStatus(m_tPhasePassCmdPhaseStatusFromUser);

	// 系统控制的情况下，相位关断控制有效
	if (m_bIsSystemCtl)
	{
		if (m_tPhasePassCmdPhaseStatusFromUser.m_bNewCmd)
		{
			m_bIfPhasePassCmdPhaseStatusHaveRefreshedToInit = false;

			m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "ProcessPhasePassControlStatus UsrCmd Phase Status:Phase1:%d, Phase2:%d, Phase3:%d, Phase4:%d, Phase5:%d, Phase6:%d, Phase7:%d, Phase8:%d Phase9:%d, Phase10:%d.", 
				m_tPhasePassCmdPhaseStatusFromUser.m_nPhasePassStatus[0], m_tPhasePassCmdPhaseStatusFromUser.m_nPhasePassStatus[1], m_tPhasePassCmdPhaseStatusFromUser.m_nPhasePassStatus[2],
				m_tPhasePassCmdPhaseStatusFromUser.m_nPhasePassStatus[3], m_tPhasePassCmdPhaseStatusFromUser.m_nPhasePassStatus[4], m_tPhasePassCmdPhaseStatusFromUser.m_nPhasePassStatus[5], 
				m_tPhasePassCmdPhaseStatusFromUser.m_nPhasePassStatus[6], m_tPhasePassCmdPhaseStatusFromUser.m_nPhasePassStatus[7], m_tPhasePassCmdPhaseStatusFromUser.m_nPhasePassStatus[8], 
				m_tPhasePassCmdPhaseStatusFromUser.m_nPhasePassStatus[9]);

			TPhaseLampClrRunCounter tRunCounter;
			m_pOpenATCRunStatus->GetPhaseLampClrRunCounter(tRunCounter);

			int nPhaseID		= 0;
			int nPhaseIndex		= 0;
			int nRingIndex		= 0;
			int nIndex			= 0;
			int nStageIndex		= 0;
			int nStageTime		= 0;
			int nPhaseRemain	= 0;
			int nMinPhaseTime	= 0;
			int nPedGreenTime	= 0;
			PTRingCtlInfo ptRingRunInfo;

			int nPhaseIDArr[MAX_RING_COUNT];
			memset(nPhaseIDArr, 0, sizeof(nPhaseIDArr));

			int nStageIndexTarget = 0;
			if (tValidManualCmd.m_tStepForwardCmd.m_nNextStageID == 0)
			{
				nStageIndexTarget = m_tFixTimeCtlInfo.m_nCurStageIndex + 1;
			}
			else
			{
				nStageIndexTarget = tValidManualCmd.m_tStepForwardCmd.m_nNextStageID - 1;
			}

			if (nStageIndexTarget >= m_tRunStageInfo.m_nRunStageCount)
			{
				nStageIndexTarget = 0;
			}

			//更新本地的相位放行状态表(当前阶段运行的相位)
			for (nRingIndex=0; nRingIndex<m_tFixTimeCtlInfo.m_nRingCount; nRingIndex++)
			{
				ptRingRunInfo = &(m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex]);
				nIndex = m_tFixTimeCtlInfo.m_nCurPhaseIndex[nRingIndex];
				nStageIndex = m_tFixTimeCtlInfo.m_nCurStageIndex;

				nPhaseID = ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhaseNumber;
				nPhaseIDArr[nRingIndex] = nPhaseID;
			
				//下发的用户指令里包含当前运行阶段的相位
				if (m_tPhasePassCmdPhaseStatusFromUser.m_bUpdatePhasePassStatus[nPhaseID - 1])
				{
					TAscStepCfg tStepCfg;
					memset(&tStepCfg, 0, sizeof(tStepCfg));
					m_pOpenATCParameter->GetStepInfo(tStepCfg);

					//处理绿灯状态下的指令
					if (ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPedPhaseStage == C_CH_PHASESTAGE_G || ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseStage == C_CH_PHASESTAGE_G)
					{
						if (m_tPhasePassCmdPhaseStatusFromUser.m_nPhasePassStatus[nPhaseID - 1] == PhasePassStatus_Close)
						{
							if (ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPedPhaseStage == C_CH_PHASESTAGE_G)
							{
								nPedGreenTime = ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_wPhaseMinimumGreen
													+ ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byGreenFlash 
													- ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhasePedestrianClear;

								if (tRunCounter.m_nPedLampClrTime[nRingIndex] >= nPedGreenTime)
								{
									ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPedPhaseGreenTime = tRunCounter.m_nPedLampClrTime[nRingIndex] + 1;
								}
								else
								{
									ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPedPhaseGreenTime = nPedGreenTime;
								}
							}

							if (ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseStage == C_CH_PHASESTAGE_G)
							{
								//绿灯时收到关闭运行的指令，将绿灯时间改为最小绿
								if (tRunCounter.m_nLampClrTime[nRingIndex] >= ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_wPhaseMinimumGreen)
								{
									ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPhaseGreenTime = tRunCounter.m_nLampClrTime[nRingIndex] + 1;
								}
								else
								{
									ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPhaseGreenTime = ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_wPhaseMinimumGreen;
								}

								if (ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPedPhaseStage == C_CH_PHASESTAGE_G)
								{
									nPedGreenTime = ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPhaseGreenTime 
													+ ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byGreenFlash 
													- ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhasePedestrianClear;

									if (tRunCounter.m_nPedLampClrTime[nRingIndex] >= nPedGreenTime)
									{
										ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPedPhaseGreenTime = tRunCounter.m_nPedLampClrTime[nRingIndex] + 1;
									}
									else
									{
										ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPedPhaseGreenTime = nPedGreenTime;
									}
								}
							}
						}

						if (tStepCfg.m_byStepType == STEP_COLOR)
						{
							m_pOpenATCRunStatus->SetIncludedCurPhaseInPhaseStatusCmdInStepColorFlag(true);
						}
						//更新相位放行状态表
						m_tPhasePassCmdPhaseStatus.m_nPhasePassStatus[nPhaseID - 1] = m_tPhasePassCmdPhaseStatusFromUser.m_nPhasePassStatus[nPhaseID - 1];
						m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "ProcessPhasePassControlStatus (light green) RingIndex:%d Index:%d PhaseGreenTime:%d", nRingIndex, nIndex, ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPhaseGreenTime);
						continue;
					}

					//处理绿闪和黄灯状态的指令
					if (ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseStage == C_CH_PHASESTAGE_GF || 
						ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseStage == C_CH_PHASESTAGE_Y ||
						ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseStage == C_CH_PHASESTAGE_R ||
						ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseStage == C_CH_PHASESTAGE_F)
					{
						if (tStepCfg.m_byStepType == STEP_COLOR)
						{
							m_pOpenATCRunStatus->SetIncludedCurPhaseInPhaseStatusCmdInStepColorFlag(true);
						}
						//更新相位放行状态表
						m_tPhasePassCmdPhaseStatus.m_nPhasePassStatus[nPhaseID - 1] = m_tPhasePassCmdPhaseStatusFromUser.m_nPhasePassStatus[nPhaseID - 1];
						continue;
					}

					//处理红灯状态的指令
					if (ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseStage == C_CH_PHASESTAGE_U)
					{
						//用户下发相位运行指令，需判断剩余时间是否足够最小绿+过渡灯色时长
						if (m_tPhasePassCmdPhaseStatusFromUser.m_nPhasePassStatus[nPhaseIDArr[nRingIndex] - 1] == PhasePassStatus_Normal)
						{
							nStageTime = m_nStageTimeForPhasePass[nRingIndex];	

							//在下一个阶段明确时，判断当前相位是否跨阶段
							if (m_nManualCurStatus == MANUAL_CONTROL_STATUS && nStageIndex != m_nNextStageIndex)
							{
								if (nPhaseIDArr[nRingIndex] != m_tRunStageInfo.m_PhaseRunstageInfo[m_nNextStageIndex].m_nConcurrencyPhase[nRingIndex])
								{
									//不跨阶段
								}
								else
								{
									//跨阶段，加上下一个阶段的阶段时间
									nStageTime = nStageTime + m_tRunStageInfo.m_PhaseRunstageInfo[m_nNextStageIndex].m_nStageEndTime - m_tRunStageInfo.m_PhaseRunstageInfo[m_nNextStageIndex].m_nStageStartTime;
								}
							}

							nPhaseRemain = nStageTime - (m_nStageRunTime[nRingIndex] + tRunCounter.m_nLampClrTime[nRingIndex]);
							nMinPhaseTime = ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_wPhaseMinimumGreen + ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byGreenFlash + 
												ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhaseYellowChange + ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhaseRedClear;

							//当前相位不跨阶段的时候，相位剩余运行时长小于最小绿时，认为还在过渡，指令无效
							if (nPhaseRemain < nMinPhaseTime)
							{
								if (tStepCfg.m_byStepType == STEP_COLOR)
								{
									m_pOpenATCRunStatus->SetIncludedCurPhaseInPhaseStatusCmdInStepColorFlag(true);
								}
								//更新相位放行状态表
								m_tPhasePassCmdPhaseStatus.m_nPhasePassStatus[nPhaseID - 1] = m_tPhasePassCmdPhaseStatusFromUser.m_nPhasePassStatus[nPhaseID - 1];
								ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseStage 	= C_CH_PHASESTAGE_F;
								ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPedPhaseStage 	= C_CH_PHASESTAGE_F;
								continue;
							}
							else
							{						
								//根据阶段剩余时长，重新计算绿灯时间
								ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPhaseGreenTime = nPhaseRemain - ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byGreenFlash - 
																									ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhaseYellowChange - 
																									ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhaseRedClear;
								ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPedPhaseGreenTime = ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPhaseGreenTime +
																							ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byGreenFlash - 
																							ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhasePedestrianClear;

								if (tStepCfg.m_byStepType == STEP_COLOR)
								{
									m_pOpenATCRunStatus->SetIncludedCurPhaseInPhaseStatusCmdInStepColorFlag(true);
								}
								//更新相位放行状态表
								m_tPhasePassCmdPhaseStatus.m_nPhasePassStatus[nPhaseID - 1] = m_tPhasePassCmdPhaseStatusFromUser.m_nPhasePassStatus[nPhaseID - 1];

								m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "ProcessPhasePassControlStatus (light red) RingIndex:%d Index:%d PhaseGreenTime:%d m_nStageTimeForPhasePass:%d nPhaseRemain:%d nMinPhaseTime:%d", nRingIndex, nIndex, ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPhaseGreenTime, m_nStageTimeForPhasePass, nPhaseRemain, nMinPhaseTime);
							}
						}
						//用户下发相位关闭运行指令
						else
						{
							if (tStepCfg.m_byStepType == STEP_COLOR)
							{
								m_pOpenATCRunStatus->SetIncludedCurPhaseInPhaseStatusCmdInStepColorFlag(true);
							}
							//更新相位放行状态表
							m_tPhasePassCmdPhaseStatus.m_nPhasePassStatus[nPhaseID - 1] = m_tPhasePassCmdPhaseStatusFromUser.m_nPhasePassStatus[nPhaseID - 1];
						}
					}
				}
			}

			//更新本地的相位放行状态表(非当前阶段运行的相位)
			bool bCurStagePhaseFlag	= false;
			for (nPhaseIndex=0; nPhaseIndex<MAX_PHASE_COUNT; nPhaseIndex++)
			{
				//是否需要更新放行状态
				if (m_tPhasePassCmdPhaseStatusFromUser.m_bUpdatePhasePassStatus[nPhaseIndex])
				{
					//是否当前阶段运行的相位
					nPhaseID = nPhaseIndex + 1;
					bCurStagePhaseFlag = false;
					for (int i=0; i<m_tFixTimeCtlInfo.m_nRingCount; i++)
					{
						if (nPhaseID == nPhaseIDArr[i])
						{
							bCurStagePhaseFlag = true;
							break;
						}
					}

					// 非当前运行阶段的相位
					if (!bCurStagePhaseFlag)
					{
						m_tPhasePassCmdPhaseStatus.m_nPhasePassStatus[nPhaseIndex] = m_tPhasePassCmdPhaseStatusFromUser.m_nPhasePassStatus[nPhaseIndex];
					
					}
				}
			}

			// 初始化用户给的相位放行状态表
			memset(&m_tPhasePassCmdPhaseStatusFromUser, 0, sizeof(m_tPhasePassCmdPhaseStatusFromUser));
			m_pOpenATCRunStatus->SetPhasePassCmdPhaseStatus(m_tPhasePassCmdPhaseStatusFromUser);

			m_pOpenATCRunStatus->SetLocalPhasePassStatus(m_tPhasePassCmdPhaseStatus);

			m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "ProcessPhasePassControlStatus Phase Status:Phase1:%d, Phase2:%d, Phase3:%d, Phase4:%d, Phase5:%d, Phase6:%d, Phase7:%d, Phase8:%d Phase9:%d, Phase10:%d.", 
															m_tPhasePassCmdPhaseStatus.m_nPhasePassStatus[0], m_tPhasePassCmdPhaseStatus.m_nPhasePassStatus[1], m_tPhasePassCmdPhaseStatus.m_nPhasePassStatus[2],
															m_tPhasePassCmdPhaseStatus.m_nPhasePassStatus[3], m_tPhasePassCmdPhaseStatus.m_nPhasePassStatus[4], m_tPhasePassCmdPhaseStatus.m_nPhasePassStatus[5], 
															m_tPhasePassCmdPhaseStatus.m_nPhasePassStatus[6], m_tPhasePassCmdPhaseStatus.m_nPhasePassStatus[7], m_tPhasePassCmdPhaseStatus.m_nPhasePassStatus[8], 
															m_tPhasePassCmdPhaseStatus.m_nPhasePassStatus[9]);
		}
		
	}
	// 本地控制的情况下，相位关断控制无效，相位默认全部放行
	else
	{
		if (!m_bIfPhasePassCmdPhaseStatusHaveRefreshedToInit)
		{
			memset(&m_tPhasePassCmdPhaseStatus, 0, sizeof(m_tPhasePassCmdPhaseStatus));
			m_pOpenATCRunStatus->SetLocalPhasePassStatus(m_tPhasePassCmdPhaseStatus);

			m_bIfPhasePassCmdPhaseStatusHaveRefreshedToInit = true;
		}
	}
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
void  CLogicCtlManual::ClearPulse()
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
函数名 ：PanelFirstStepWard
功能 ：处理面板第一次步进
算法实现 ： 
参数说明 ：nStageIndexTarget，目标阶段
返回值说明：无
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 李永萍          创建函数 
====================================================================*/
void CLogicCtlManual::PanelFirstStepWard(int nStageIndexTarget)
{
	TPhaseLampClrRunCounter tRunCounter;
	m_pOpenATCRunStatus->GetPhaseLampClrRunCounter(tRunCounter);

	int  i = 0;
	for (i = 0;i < m_tFixTimeCtlInfo.m_nRingCount;i++)
	{
		PTRingCtlInfo ptRingRunInfo = &(m_tFixTimeCtlInfo.m_atPhaseSeq[i]);
		int nIndex = m_tFixTimeCtlInfo.m_nCurPhaseIndex[i];
		PTPhaseCtlInfo pPhaseInfo = (PTPhaseCtlInfo)&(ptRingRunInfo->m_atPhaseInfo[nIndex]);
		m_nNextStageIndex = m_tFixTimeCtlInfo.m_nCurStageIndex + 1;
		int nPhaseID = pPhaseInfo->m_tPhaseParam.m_byPhaseNumber;

		//卡住不动给最大值
		if ((ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseStage == C_CH_PHASESTAGE_G || 
			(m_tOldValidManualCmd.m_nSubCtlMode == CTL_MANUAL_SUBMODE_SYSTEM_CHANNEL_LOCK && 
			ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseStage == C_CH_PHASESTAGE_U))&& 
			m_tPhasePassCmdPhaseStatus.m_nPhasePassStatus[nPhaseID-1] != PhasePassStatus_Close &&
			ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseMode != NEGLECT_MODE && ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseMode != SHIELD_MODE)
		{
			pPhaseInfo->m_wPhaseGreenTime = MAX_GREEN_TIME;
			AdjustPedPhaseGreenTime(i, nIndex, nStageIndexTarget, MANUAL_MINI_GREEN);
		}

		if (m_tPhasePassCmdPhaseStatus.m_nPhasePassStatus[nPhaseID-1] == PhasePassStatus_Close)
		{
			pPhaseInfo->m_wPhaseTime = MAX_GREEN_TIME;
		}

		m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "Panel First Step, Stop In Current Stage");
	}

	ReSetNeglectPhaseStageRunTime();
}

/*==================================================================== 
函数名 ：NonStepWardToStepWard
功能 ：处理非步进到步进
算法实现 ： 
参数说明 ：nStageIndexTarget，目标阶段
返回值说明：无
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 李永萍          创建函数 
====================================================================*/
void CLogicCtlManual::NonStepWardToStepWard(int nStageIndexTarget)
{
	int i = 0;

	TPhaseLampClrRunCounter tRunCounter;
	m_pOpenATCRunStatus->GetPhaseLampClrRunCounter(tRunCounter);

	//当前阶段就是目标阶段就卡住，否则要切到下一个阶段
	if (m_tFixTimeCtlInfo.m_nCurStageIndex == nStageIndexTarget)
	{
		for (i = 0;i < m_tFixTimeCtlInfo.m_nRingCount;i++)
		{
			PTRingCtlInfo ptRingRunInfo = &(m_tFixTimeCtlInfo.m_atPhaseSeq[i]);
			int nIndex = m_tFixTimeCtlInfo.m_nCurPhaseIndex[i];
			PTPhaseCtlInfo pPhaseInfo = (PTPhaseCtlInfo)&(ptRingRunInfo->m_atPhaseInfo[nIndex]);
			int nPhaseID = pPhaseInfo->m_tPhaseParam.m_byPhaseNumber;

			//卡住不动给最大值
			if ((ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseStage == C_CH_PHASESTAGE_U || 
				ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseStage == C_CH_PHASESTAGE_G) && 
				m_tPhasePassCmdPhaseStatus.m_nPhasePassStatus[nPhaseID-1] != PhasePassStatus_Close &&
				ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseMode != NEGLECT_MODE && ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseMode != SHIELD_MODE)
			{
				pPhaseInfo->m_wPhaseGreenTime = MAX_GREEN_TIME;
				if (ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPedPhaseStage == C_CH_PHASESTAGE_U)
				{
					pPhaseInfo->m_wPedPhaseGreenTime = pPhaseInfo->m_tPhaseParam.m_wPhaseMinimumGreen + pPhaseInfo->m_tPhaseParam.m_byGreenFlash - pPhaseInfo->m_tPhaseParam.m_byPhasePedestrianClear;
				}
				else if (ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPedPhaseStage == C_CH_PHASESTAGE_G)
				{
					AdjustPedPhaseGreenTime(i, nIndex, nStageIndexTarget, MANUAL_MINI_GREEN);
				}
			}

			if (m_tPhasePassCmdPhaseStatus.m_nPhasePassStatus[nPhaseID-1] == PhasePassStatus_Close)
			{
				pPhaseInfo->m_wPhaseTime = MAX_GREEN_TIME;
			}

			m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "NonStepToStep, Stop In Current Stage, curent stage is next stage!");
		}

		ReSetNeglectPhaseStageRunTime();

		m_nNextStageIndex = m_tFixTimeCtlInfo.m_nCurStageIndex + 1;
	}
	else
	{
		for (i = 0;i < m_tFixTimeCtlInfo.m_nRingCount;i++)
		{
			PTRingCtlInfo ptRingRunInfo = &(m_tFixTimeCtlInfo.m_atPhaseSeq[i]);
			int nIndex = m_tFixTimeCtlInfo.m_nCurPhaseIndex[i];
			PTPhaseCtlInfo pPhaseInfo = (PTPhaseCtlInfo)&(ptRingRunInfo->m_atPhaseInfo[nIndex]);
			int nPhaseID = pPhaseInfo->m_tPhaseParam.m_byPhaseNumber;
							
			if (ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseStage == C_CH_PHASESTAGE_G && 
				m_tPhasePassCmdPhaseStatus.m_nPhasePassStatus[nPhaseID-1] != PhasePassStatus_Close)
			{
				//尽快切到下一个阶段
				if (m_tRunStageInfo.m_PhaseRunstageInfo[m_tFixTimeCtlInfo.m_nCurStageIndex].m_nConcurrencyPhase[i] !=
					m_tRunStageInfo.m_PhaseRunstageInfo[nStageIndexTarget].m_nConcurrencyPhase[i])
				{
					AdjustPhaseGreenTime(i, nIndex, nStageIndexTarget, MANUAL_MINI_GREEN);
					AdjustPedPhaseGreenTime(i, nIndex, nStageIndexTarget, MANUAL_PHASE_RUN_GREEN);

					m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "NonStepToStep, Step to next stage GreenTime:%d PedGreenTime:%d", pPhaseInfo->m_wPhaseGreenTime, pPhaseInfo->m_wPedPhaseGreenTime);
				}
			}

			if (m_tPhasePassCmdPhaseStatus.m_nPhasePassStatus[nPhaseID-1] == PhasePassStatus_Close)
			{
				//非步进到步进，时间不确定，红脉冲无法发送
				if (tRunCounter.m_nLampClrTime[i] >= pPhaseInfo->m_tPhaseParam.m_wPhaseMinimumGreen)
				{
					pPhaseInfo->m_wPhaseTime = tRunCounter.m_nLampClrTime[i] + pPhaseInfo->m_tPhaseParam.m_byGreenFlash + pPhaseInfo->m_tPhaseParam.m_byPhaseYellowChange + pPhaseInfo->m_tPhaseParam.m_byPhaseRedClear;
				}
				else
				{
					pPhaseInfo->m_wPhaseTime = pPhaseInfo->m_tPhaseParam.m_wPhaseMinimumGreen + pPhaseInfo->m_tPhaseParam.m_byGreenFlash + pPhaseInfo->m_tPhaseParam.m_byPhaseYellowChange + pPhaseInfo->m_tPhaseParam.m_byPhaseRedClear;
				}
			}
		}

		ReSetNeglectPhaseStageRunTime();

		m_nNextStageIndex = nStageIndexTarget;
	}
}

/*==================================================================== 
函数名 ：FirstPanelManualSwitchToDirectionOrPattern
功能 ：处理第一次面板手动按钮切方向或自主
算法实现 ： 
参数说明 ：tValidManualCmd，手动命令
返回值说明：无
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 李永萍          创建函数 
====================================================================*/
void CLogicCtlManual::FirstPanelManualSwitchToDirectionOrPattern(TManualCmd  & tValidManualCmd)
{
	int	nGreenPhaseCount = 0;
	int nClosePhaseCount = 0; 
	CheckIfStageHavePhaseInGreen(nGreenPhaseCount, nClosePhaseCount);
	if (nGreenPhaseCount > 0)
	{
		PTPhase pPhaseInfo;
		PTRingCtlInfo ptRingRunInfo;

		bool bFlag = false;
		int nIndex = 0;
		int nCnt = 0;
		int nCurStageIndex	= m_tFixTimeCtlInfo.m_nCurStageIndex;
	
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
				// 判断当前相位是否跨下一个阶段和当前阶段
				if (m_tRunStageInfo.m_PhaseRunstageInfo[nCurStageIndex].m_nConcurrencyPhase[i] == m_tRunStageInfo.m_PhaseRunstageInfo[nStageIndexTarget].m_nConcurrencyPhase[i])
				{
					if (tValidManualCmd.m_bDirectionCmd)//步进切方向，跨阶段的相位需要在当前阶段立刻过渡完
					{
						m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "PanelSwitchToDirection, RingIndex:%d PhaseIndex:%d SetGreenTime", i, nIndex);
						AdjustPhaseGreenTime(i, nIndex, nStageIndexTarget, MANUAL_MINI_GREEN);
						AdjustPedPhaseGreenTime(i, nIndex, nStageIndexTarget, MANUAL_MINI_GREEN);
					}
				}
				else
				{
					if (tValidManualCmd.m_bDirectionCmd)
					{
						m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "PanelSwitchToDirection, RingIndex:%d PhaseIndex:%d SetGreenTime", i, nIndex);
					}
					else
					{
						m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "PanelSwitchToPattern, RingIndex:%d PhaseIndex:%d SetGreenTime", i, nIndex);
					}
					AdjustPhaseGreenTime(i, nIndex, nStageIndexTarget, MANUAL_MINI_GREEN);
					AdjustPedPhaseGreenTime(i, nIndex, nStageIndexTarget, MANUAL_MINI_GREEN);
				}
				//方向开始前继续保持绿色的通道
				if (tValidManualCmd.m_bDirectionCmd)
				{
					if (ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPedPhaseStage == C_CH_PHASESTAGE_G)
					{
						SetKeepGreenChannelBeforeControlChannel(pPhaseInfo->m_byPhaseNumber, pPhaseInfo->m_byPhaseNumber, tValidManualCmd);
					}
					else
					{
						SetKeepGreenChannelBeforeControlChannel(pPhaseInfo->m_byPhaseNumber, 0, tValidManualCmd);
					}
				}
			}
		}
	}
	
	ReSetNeglectPhaseStageRunTime();
	
	memcpy(&m_tOldValidManualCmd, &tValidManualCmd, sizeof(tValidManualCmd));
}

/*==================================================================== 
函数名 ：BackUpPhaseTime
功能 ：备份相位时间
算法实现 ： 
参数说明 ：nRefreshStageTime，备份来源
返回值说明：无
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 李永萍          创建函数 
====================================================================*/
void CLogicCtlManual::BackUpPhaseTime(int nRefreshStageTime)
{
	int  i = 0;
	for (i = 0;i < m_tFixTimeCtlInfo.m_nRingCount;i++)
	{
		PTRingCtlInfo ptRingRunInfo = &(m_tFixTimeCtlInfo.m_atPhaseSeq[i]);
		int nIndex = m_tFixTimeCtlInfo.m_nCurPhaseIndex[i];
		PTPhaseCtlInfo pPhaseInfo = (PTPhaseCtlInfo)&(ptRingRunInfo->m_atPhaseInfo[nIndex]);
		int nPhaseID = pPhaseInfo->m_tPhaseParam.m_byPhaseNumber;

		if (m_tPhasePassCmdPhaseStatus.m_nPhasePassStatus[nPhaseID-1] != PhasePassStatus_Close)
		{
			m_nStageTimeForPhasePass[i] = pPhaseInfo->m_wPhaseGreenTime + pPhaseInfo->m_tPhaseParam.m_byGreenFlash + pPhaseInfo->m_tPhaseParam.m_byPhaseYellowChange + pPhaseInfo->m_tPhaseParam.m_byPhaseRedClear;

		}
		else
		{
			m_nStageTimeForPhasePass[i] = pPhaseInfo->m_wPhaseTime;
		}

		m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "BackUpPhaseTime RingIndex:%d PhasePassTime:%d.", i, m_nStageTimeForPhasePass[i]);
	}

	ReSetNeglectPhasetBackUpTime();
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
bool  CLogicCtlManual::AdjustPhaseGreenTime(int nRingIndex, int nIndex, int nStageIndexTarget, int nGreenTimeFlag)
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
bool  CLogicCtlManual::AdjustPedPhaseGreenTime(int nRingIndex, int nIndex, int nStageIndexTarget, int nGreenTimeFlag)
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

		if ((m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nIndex].m_wPhaseGreenTime + m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nIndex].m_tPhaseParam.m_byGreenFlash) 
			< m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhasePedestrianClear)
		{
			nPedGreenTime = 0;
		}
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
2019/09/14     V1.0 刘黎明          创建函数 
====================================================================*/ 
void CLogicCtlManual::SetOverlapPhaseLampClr(int nNextStageIndex)
{
	if (nNextStageIndex >= m_tRunStageInfo.m_nRunStageCount)
	{
		nNextStageIndex = 0;
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
				bool bIsNeglectPhase = IsNeglectPhase(byMainPhaseNum);

				GetPhaseIndexByPhaseNumber(nRingIndex, nPhaseIndex, (int)byMainPhaseNum);
				chRet = GetPhaseStatus(byMainPhaseNum, false);

				if (chRet == C_CH_PHASESTAGE_G && !bIsNeglectPhase)
                {
                    bGreenFlag = true;
                }

				int nStageIndex = m_tFixTimeCtlInfo.m_nCurStageIndex;
				for (m = 0;m < m_tRunStageInfo.m_PhaseRunstageInfo[nStageIndex].m_nConcurrencyPhaseCount;m++)
				{
					if (!bIsNeglectPhase && m_tRunStageInfo.m_PhaseRunstageInfo[nStageIndex].m_nConcurrencyPhase[m] == byMainPhaseNum && m_tPhasePassCmdPhaseStatus.m_nPhasePassStatus[byMainPhaseNum-1] == PhasePassStatus_Normal)
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

				if (IsOverlapNeedKeepGreen(i, OVERLAP_CHA))
				{
					bGreenFlag = true;
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
						if (!IsNeglectPhase(nNextPhaseID) && IsPhaseNumInOverlap(nNextPhaseID, i) && m_tPhasePassCmdPhaseStatus.m_nPhasePassStatus[nNextPhaseID-1] == PhasePassStatus_Normal)
						{
							for (n = 0;n < m_tRunStageInfo.m_PhaseRunstageInfo[nStageIndex].m_nConcurrencyPhaseCount;n++)
							{
								nPhaseID = m_tRunStageInfo.m_PhaseRunstageInfo[nStageIndex].m_nConcurrencyPhase[n];
								if (IsPhaseNumInOverlap(nPhaseID, i) && (m_tPhasePassCmdPhaseStatus.m_nPhasePassStatus[nPhaseID-1] == PhasePassStatus_Normal ||
									m_pOpenATCRunStatus->GetIncludedCurPhaseInPhaseStatusCmdInStepColorFlag()))//色步模式下下发的关断指令里包含当前运行阶段的相位，则跟随相位继续绿
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
						//用户控制时按了方向键或者系统控制时下发了通道锁定指令，则跟随相位即使跟随下一个阶段的相位，也需要和当前阶段的相位一起过渡灯色
						if (m_bIsUsrCtl || m_bIsSystemCtl)
						{
							TManualCmd  tValidManualCmd;
							memset(&tValidManualCmd,0,sizeof(tValidManualCmd));
							m_pOpenATCRunStatus->GetValidManualCmd(tValidManualCmd);
							if (tValidManualCmd.m_bDirectionCmd || tValidManualCmd.m_bChannelLockCmd)
							{
								bGreenFlag = false;
							}
							
							//从相位锁定正在过渡准备切到面板方向的过程中，跟随相位如果还跟随下一个相位则继续绿
							if (tValidManualCmd.m_bDirectionCmd && m_tOldValidManualCmd.m_bPhaseToChannelLock &&
								m_tOldValidManualCmd.m_nSubCtlMode == CTL_MANUAL_SUBMODE_SYSTEM_CHANNEL_LOCK)
							{
								bGreenFlag = true;
							}
						}
					}

					if (bGreenFlag)
					{
						break;
					}
				}

				chRet = GetPhaseStatus(byMainPhaseNum, false);
				GetOldLockPhaseStatus(i, OVERLAP_CHA, chRet);
				if (chRet == C_CH_PHASESTAGE_GF)
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
				if (chRet == C_CH_PHASESTAGE_Y)
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
				bool bIsNeglectPhase = IsNeglectPhase(byMainPhaseNum);

				GetPhaseIndexByPhaseNumber(nRingIndex, nPhaseIndex, (int)byMainPhaseNum);
				chPedRet = GetPhaseStatus(byMainPhaseNum, true);

				if (chPedRet == C_CH_PHASESTAGE_G && !bIsNeglectPhase)
                {
                    bPedGreenFlag = true;
                }

				int nStageIndex = m_tFixTimeCtlInfo.m_nCurStageIndex;
				for (m = 0;m < m_tRunStageInfo.m_PhaseRunstageInfo[nStageIndex].m_nConcurrencyPhaseCount;m++)
				{
					if (!bIsNeglectPhase && m_tRunStageInfo.m_PhaseRunstageInfo[nStageIndex].m_nConcurrencyPhase[m] == byMainPhaseNum 
						&& m_tPhasePassCmdPhaseStatus.m_nPhasePassStatus[byMainPhaseNum-1] == PhasePassStatus_Normal)
					{
						if (chPedRet == C_CH_PHASESTAGE_G 
							|| (chPedRet == C_CH_PHASESTAGE_U && m_tFixTimeCtlInfo.m_atOverlapInfo[i].m_chPedOverlapStage == C_CH_PHASESTAGE_G))
						{
							bPedGreenFlag = true;
							break;
						}
					}
				}

				if (IsOverlapNeedKeepGreen(i, OVERLAP_PED_CHA))
				{
					bPedGreenFlag = true;
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
						if (!IsNeglectPhase(nNextPhaseID) && IsPhaseNumInOverlap(nNextPhaseID, i) && m_tPhasePassCmdPhaseStatus.m_nPhasePassStatus[nNextPhaseID-1] == PhasePassStatus_Normal)
						{
							for (n = 0;n < m_tRunStageInfo.m_PhaseRunstageInfo[nStageIndex].m_nConcurrencyPhaseCount;n++)
							{
								nPhaseID = m_tRunStageInfo.m_PhaseRunstageInfo[nStageIndex].m_nConcurrencyPhase[n];
								if (IsPhaseNumInOverlap(nPhaseID, i) && (m_tPhasePassCmdPhaseStatus.m_nPhasePassStatus[nPhaseID-1] == PhasePassStatus_Normal ||
									m_pOpenATCRunStatus->GetIncludedCurPhaseInPhaseStatusCmdInStepColorFlag()))//色步模式下下发的关断指令里包含当前运行阶段的行人相位，则行人跟随相位继续绿
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
						//用户控制时按了方向键或者系统控制时下发了通道锁定指令，则跟随相位即使跟随下一个阶段的相位，也需要和当前阶段的相位一起过渡灯色
						if (m_bIsUsrCtl || m_bIsSystemCtl)
						{
							TManualCmd  tValidManualCmd;
							memset(&tValidManualCmd,0,sizeof(tValidManualCmd));
							m_pOpenATCRunStatus->GetValidManualCmd(tValidManualCmd);
							if (tValidManualCmd.m_bDirectionCmd || tValidManualCmd.m_bChannelLockCmd)
							{
								bPedGreenFlag = false;
							}

							//从相位锁定正在过渡准备切到面板方向的过程中，跟随相位如果还跟随下一个相位则继续绿
							if (tValidManualCmd.m_bDirectionCmd && m_tOldValidManualCmd.m_bPhaseToChannelLock &&
								m_tOldValidManualCmd.m_nSubCtlMode == CTL_MANUAL_SUBMODE_SYSTEM_CHANNEL_LOCK)
							{
								bPedGreenFlag = true;
							}
						}
					}

					if (bPedGreenFlag)
					{
						break;
					}
				}

				chPedRet = GetPhaseStatus(byMainPhaseNum, true);
				GetOldLockPhaseStatus(i, OVERLAP_PED_CHA, chPedRet);
				if (chPedRet == C_CH_PHASESTAGE_GF)
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
				if (chPedRet == C_CH_PHASESTAGE_Y)
				{
					if (m_tFixTimeCtlInfo.m_atOverlapInfo[i].m_chPedOverlapStage == C_CH_PHASESTAGE_GF)
					{
						for (m = 0;m < m_tRunStageInfo.m_PhaseRunstageInfo[nNextStageIndex].m_nConcurrencyPhaseCount;m++)
						{
							if ((m_tRunStageInfo.m_PhaseRunstageInfo[nNextStageIndex].m_nConcurrencyPhase[m] == byMainPhaseNum 
								&& m_tPhasePassCmdPhaseStatus.m_nPhasePassStatus[byMainPhaseNum-1] == PhasePassStatus_Normal)
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
			m_tFixTimeCtlInfo.m_atOverlapInfo[i].m_chOverlapStage = C_CH_PHASESTAGE_G;
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
			m_tFixTimeCtlInfo.m_atOverlapInfo[i].m_chOverlapStage = C_CH_PHASESTAGE_R;
		} 

		if (bPedGreenFlag)
		{
			m_tFixTimeCtlInfo.m_atOverlapInfo[i].m_chPedOverlapStage = C_CH_PHASESTAGE_G;
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
			m_tFixTimeCtlInfo.m_atOverlapInfo[i].m_chPedOverlapStage = C_CH_PHASESTAGE_R;
		} 

		SetChannelStatus((int)byOverlapNum,OVERLAP_SRC,m_tFixTimeCtlInfo.m_atOverlapInfo[i].m_chOverlapStage,m_tFixTimeCtlInfo.m_atOverlapInfo[i].m_chPedOverlapStage);
	}
}

/*==================================================================== 
函数名 ：InitParamBeforeChannelLock
功能 ：
算法实现 ：在切换方向之前先初始化参数
参数说明 ：tCtlCmd，手动面板命令
返回值说明：无
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 李永萍          创建函数 
====================================================================*/ 
void  CLogicCtlManual::InitParamBeforeChannelLock(int nChannelLockStatus[])
{
	int  i = 0, j = 0;
	for (i = 0;i < MAX_CHANNEL_COUNT;i++)
	{
        m_bChangeChannelClr[i] = false;
        m_nChannelCounter[i] = 0;
        m_nChannelDurationCounter[i] = 0;
        m_chChannelStatus[i] = C_CH_PHASESTAGE_U;
		m_chChannelStage[i] = C_CH_PHASESTAGE_U;
		m_bChannelTran[i] = false;

        m_nChannelDurationTime[i] = 0;
	}

	//获取当前通道需要切换的目标灯色
    for (i = 0;i < m_nChannelCount;i++)
	{
		if (m_atChannelInfo[i].m_byChannelNumber == 0)
		{
			continue;
		}

		if (nChannelLockStatus[i] != CHANNEL_STATUS_DEFAULT)
		{
            m_bChangeChannelClr[i] = true;
			m_chChannelStatus[i] = nChannelLockStatus[i];

			if (m_bNeglectChannelBoforePhaseLock[i])
			{
				m_bChangeChannelClr[i] = false;
				m_chChannelStatus[i] = C_CH_PHASESTAGE_U;
			}
		}
        else
        { 
            m_bChangeChannelClr[i] = false;
            m_chChannelStatus[i] = C_CH_PHASESTAGE_U;
        }
	}

	//获取当前通道的灯色
    for (j = 0;j < m_nChannelCount;j++)
    {
		if (m_atChannelInfo[j].m_byChannelNumber == 0)
		{
			continue;
		}

        if (m_achLampClr[(m_atChannelInfo[j].m_byChannelNumber - 1) * 3] == LAMP_CLR_ON)
		{
            m_chChannelStage[j] = C_CH_PHASESTAGE_R;
		}

		if (m_achLampClr[(m_atChannelInfo[j].m_byChannelNumber - 1) * 3 + 1] == LAMP_CLR_ON)
		{
            m_chChannelStage[j] = C_CH_PHASESTAGE_Y;
		}
		else if (m_achLampClr[(m_atChannelInfo[j].m_byChannelNumber - 1) * 3 + 1] == LAMP_CLR_FLASH)
		{
            m_chChannelStage[j] = C_CH_PHASESTAGE_YF;
		}
		
		if (m_achLampClr[(m_atChannelInfo[j].m_byChannelNumber - 1) * 3 + 2] == LAMP_CLR_ON)
		{
            m_chChannelStage[j] = C_CH_PHASESTAGE_G;
		}
    }    
}

/*==================================================================== 
函数名 ：ProcessChannelLock
功能 ：处理通道锁定。
算法实现 ： 
参数说明 ：tValidManualCmd，手动命令
返回值说明：无
 ----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 李永萍          创建函数 
====================================================================*/ 
void  CLogicCtlManual::ProcessChannelLock(TManualCmd & tValidManualCmd)
{
	if (tValidManualCmd.m_bNewCmd)
	{
		//进入通道锁定以后，根据新来的指令计算绿灯时间
		if (tValidManualCmd.m_bPatternInterruptCmd ||//进入通道锁定以后，点了自主或其他控制方式
			tValidManualCmd.m_bChannelLockCmd)//进入通道锁定以后，又下发通道锁定指令
		{
			//通道锁定1切到通道锁定2，通道锁定1还没有过渡到最小绿，又切通道锁定1，绿灯时间还是走绿灯过渡时间，其他情况绿灯应走最小绿
			//通道锁定1正在过渡灯色(绿闪，黄，红)，又切通道锁定1，则通道锁定1走完，再回到通道锁定1
			if (tValidManualCmd.m_bChannelLockCmd && memcmp(tValidManualCmd.m_tChannelLockCmd.m_tNextChannelLockCtrlCmd.m_nChannelLockStatus, 
			    m_tOldValidManualCmd.m_tChannelLockCmd.m_tNextChannelLockCtrlCmd.m_nChannelLockStatus, MAX_CHANNEL_COUNT) == 0)
			{
				//获取当前通道需要切换的目标灯色
				for (int i = 0;i < MAX_CHANNEL_COUNT;i++)
				{
					if (m_tOldValidManualCmd.m_tChannelLockCmd.m_tNextChannelLockCtrlCmd.m_nChannelLockStatus[i] != CHANNEL_STATUS_DEFAULT &&
						m_chChannelStage[i] == C_CH_PHASESTAGE_G)
					{    
						tValidManualCmd.m_bChannelLockCmd = false;//通道锁定1切到通道锁定2，通道锁定1还没有过渡到最小绿，又切通道锁定1，该相同的值指令不执行
						break;
					} 
				}

				for (int i = 0;i < MAX_CHANNEL_COUNT;i++)
				{
					m_nChannelLockGreenTime[i] = m_tOldValidManualCmd.m_tChannelLockCmd.m_tNextChannelLockCtrlCmd.m_nDuration;	
					m_bChannelKeepGreenFlag[i] = false;
				}
			}
			else
			{
				for (int i = 0;i < MAX_CHANNEL_COUNT;i++)
				{
					if (m_tOldValidManualCmd.m_tChannelLockCmd.m_tNextChannelLockCtrlCmd.m_nChannelLockStatus[i] == CHANNEL_STATUS_GREEN && 
						m_chChannelStage[i] == C_CH_PHASESTAGE_G)
					{
						if (tValidManualCmd.m_tChannelLockCmd.m_tNextChannelLockCtrlCmd.m_nChannelLockStatus[i] == CHANNEL_STATUS_GREEN)
						{
							m_nChannelLockGreenTime[i] = m_tOldValidManualCmd.m_tChannelLockCmd.m_tNextChannelLockCtrlCmd.m_nDuration;	
							m_bChannelKeepGreenFlag[i] = true;
						}
						else
						{
							m_nChannelLockGreenTime[i] = m_tOldValidManualCmd.m_tChannelLockCmd.m_tNextChannelLockCtrlCmd.m_nMinGreen;	
							m_bChannelKeepGreenFlag[i] = false;
							m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "ProcessChannelLock, ChannelID:%d SetCurrentChannelLockGreenTime:%d ", i + 1, m_nChannelLockGreenTime[i]);
						}
					}

					if (!m_bChannelKeepGreenFlag[i] && tValidManualCmd.m_bChannelLockCmd && tValidManualCmd.m_bPhaseToChannelLock)
					{
						m_bTargetLockPhaseChannelFlag[i] = false;//新的锁定指令不需要继续保持绿的目标锁定相位通道需要过渡
					}
				}

				if (tValidManualCmd.m_bChannelLockCmd && tValidManualCmd.m_bPhaseToChannelLock)
				{
					m_tFixTimeCtlInfo.m_nCurStageIndex = m_tRunStageInfo.m_nRunStageCount;
					m_nNextStageIndex = m_tRunStageInfo.m_nRunStageCount; 
					memset(&m_tNewPhaseLockPara, 0, sizeof(m_tNewPhaseLockPara));
					memcpy(&m_tNewPhaseLockPara, &tValidManualCmd.m_tPhaseLockPara, sizeof(m_tNewPhaseLockPara));
				}
			} 

			m_bOldLockChannelCmdEndFlag = false;
		}
		else//刚开始进入通道锁定
		{
			if (tValidManualCmd.m_bChannelLockCmd)
			{
				memset(m_bNeglectChannelBoforePhaseLock, 0, sizeof(m_bNeglectChannelBoforePhaseLock));
				if (tValidManualCmd.m_bPhaseToChannelLock)
				{
					SetNeglectChannelBoforePhaseLock();
				}
			}

			m_bOldLockChannelCmdEndFlag = true;
			SetLockPhaseList(tValidManualCmd);

			//初始化通道锁定参数，开始切通道锁定
			InitParamBeforeChannelLock(tValidManualCmd.m_tChannelLockCmd.m_tNextChannelLockCtrlCmd.m_nChannelLockStatus);
			ChangeChannelClr();

			for (int i = 0;i < MAX_CHANNEL_COUNT;i++)
			{
				m_nChannelLockGreenTime[i] = tValidManualCmd.m_tChannelLockCmd.m_tNextChannelLockCtrlCmd.m_nDuration;	
				m_bChannelKeepGreenFlag[i] = false;
			}

			memcpy(&m_tOldValidManualCmd, &tValidManualCmd, sizeof(tValidManualCmd));

			//获取锁定相位参数
			GetLockPhaseData(tValidManualCmd);
		}

		tValidManualCmd.m_bNewCmd = false;
		m_pOpenATCRunStatus->SetValidManualCmd(tValidManualCmd);
	}

	bool bLockCmd = false;//下一个指令是否还是锁定指令标志
	if (tValidManualCmd.m_bChannelLockCmd)
	{
		bLockCmd = true;
	}

	SetLockPhaseStage(tValidManualCmd);

	bool bSelRun = TransitChannelClr(CHANNEL_TYPE_LOCK, bLockCmd);
	if (bSelRun)
	{
		m_bOldLockChannelCmdEndFlag = true;
		SetLockPhaseList(tValidManualCmd);

		bool bFlag = true;
		if (tValidManualCmd.m_bChannelLockCmd)
		{
			for (int i = 0; i < tValidManualCmd.m_tPhaseLockPara.m_nPhaseLockCount;i++)
			{
				if (tValidManualCmd.m_tPhaseLockPara.m_nPhaseLockType[i] != LOCK_TYPE_CANCEL)
				{
					bFlag = false;//只要有一个相位的锁定类型不是恢复，就认为是新的锁定指令，则非目标相位不需要过渡到目标相位,直接切到新的相位锁定
				}
			}
		}
		//三种情况需要非目标相位过渡到目标相位:1.相位锁定持续时间到了 2.点了自主按钮 3.本来锁定的相位点了恢复(此时所有的相位锁定类型都是恢复)
		//非目标相位过渡到目标相位的过程:目标锁定相位因为需要继续锁定保持绿或者过渡结束，非目标相位环内因为需要继续锁定保持绿或者当前相位切换结束
		if (bFlag)
		{
			for (int i = 0; i < MAX_PHASE_COUNT; i++)
			{
				if (m_tLockPhaseData[i].nCurLockPhaseID == 0)
				{
					continue;
				}
				//1.目标相位过渡结束，非目标相位所在环的当前相位还没有切换结束
				//2.目标相位因为要继续锁定保持绿，非目标相位所在环的当前相位还没有切换结束
				if (m_tLockPhaseData[i].bNeedTransFlag && !m_tLockPhaseData[i].bSwitchSuccessFlag)
				{
					SwitchLockChannelPhaseToNext();
					return;
				}
			}
		}

		memset(m_tLockPhaseData, 0, sizeof(m_tLockPhaseData));
		memset(m_bTargetLockPhaseChannelFlag, 0, sizeof(m_bTargetLockPhaseChannelFlag));
	    memset(m_bNonTargetLockPhaseEndFlag, 0, sizeof(m_bNonTargetLockPhaseEndFlag));

		memset(&m_tNewPhaseLockPara, 0, sizeof(m_tNewPhaseLockPara));

		if (tValidManualCmd.m_bPatternInterruptCmd || 
			tValidManualCmd.m_bChannelLockCmd)
		{
			if (tValidManualCmd.m_bChannelLockCmd)//通道锁定过渡结束，进入新的通道锁定
			{
				memset(m_bNeglectChannelBoforePhaseLock, 0, sizeof(m_bNeglectChannelBoforePhaseLock));
				if (tValidManualCmd.m_bPhaseToChannelLock)
				{
					SetNeglectChannelBoforePhaseLock();
				}

				//初始化通道锁定参数，开始通道锁定控制
				InitParamBeforeChannelLock(tValidManualCmd.m_tChannelLockCmd.m_tNextChannelLockCtrlCmd.m_nChannelLockStatus);
				ChangeChannelClr();
				for (int i = 0;i < MAX_CHANNEL_COUNT;i++)
				{
					m_nChannelLockGreenTime[i] = tValidManualCmd.m_tChannelLockCmd.m_tNextChannelLockCtrlCmd.m_nDuration;
					m_bChannelKeepGreenFlag[i] = false;
				}
				//清除新的通道锁定指令标志，因为是通道锁定切通道锁定所以当前控制子模式还是通道锁定
				tValidManualCmd.m_bNewCmd = false;
				tValidManualCmd.m_bChannelLockCmd = false;
				memcpy(&m_tOldValidManualCmd.m_tChannelLockCmd.m_tNextChannelLockCtrlCmd, &tValidManualCmd.m_tChannelLockCmd.m_tNextChannelLockCtrlCmd, sizeof(TChannelLockCtrlCmd));
				//memcpy(&tValidManualCmd.m_tDirectionCmd,&m_tOldValidManualCmd.m_tDirectionCmd,sizeof(TDirectionCmd));
				memcpy(&m_tOldValidManualCmd.m_tPhaseLockPara, &tValidManualCmd.m_tPhaseLockPara, sizeof(TPhaseLockPara));
				m_pOpenATCRunStatus->SetValidManualCmd(tValidManualCmd);
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "ProcessChannelLock, TransitChannelLock End, Trans To NewChannelLock GreenTime:%d", tValidManualCmd.m_tChannelLockCmd.m_tNextChannelLockCtrlCmd.m_nDuration);

				//获取锁定相位参数
				GetLockPhaseData(tValidManualCmd);
			}
			else
			{
				//因为点了自主，所以通道锁定过渡结束后，修改当前控制模式和控制子模式
				if (tValidManualCmd.m_bPatternInterruptCmd)
				{
					tValidManualCmd.m_nCtlMode = CTL_MODE_SELFCTL;
					tValidManualCmd.m_nSubCtlMode = CTL_MANUAL_SUBMODE_DEFAULT;
					tValidManualCmd.m_bNewCmd = false;
					m_pOpenATCRunStatus->SetValidManualCmd(tValidManualCmd);
					m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "ProcessChannelLock, TransitChannelLock End, Trans To SelfCtl");
					m_nManualCurStatus = MANUAL_STAGE_TRANS;//进入过渡
				}

				//下发了自主指令或手动指令，通道锁定过渡结束，直接修改控制模式和子模式，备份指令
			    memcpy(&m_tOldValidManualCmd, &tValidManualCmd, sizeof(tValidManualCmd));
			}
		}
		else
		{
			if (tValidManualCmd.m_tChannelLockCmd.m_bStepFowardToChannelLock)//回到通道锁定前的步进
			{
				TAscStepCfg tStepCfg;
				memset(&tStepCfg, 0, sizeof(tStepCfg));
				m_pOpenATCParameter->GetStepInfo(tStepCfg);
				//生成回到自主前的步进指令，回到切通道锁定前的阶段的下一个阶段
				tValidManualCmd.m_bNewCmd = true;
				tValidManualCmd.m_nCtlMode = CTL_MODE_MANUAL;
				tValidManualCmd.m_nSubCtlMode = CTL_MANUAL_SUBMODE_SYSTEM_STEPFOWARD;
				tValidManualCmd.m_bStepForwardCmd = true;
				tValidManualCmd.m_tStepForwardCmd.m_byStepType = tStepCfg.m_byStepType;
		        tValidManualCmd.m_tStepForwardCmd.m_nNextStageID = tValidManualCmd.m_tChannelLockCmd.m_nTargetStageIndex + 1;
				tValidManualCmd.m_tStepForwardCmd.m_nDurationTime = tValidManualCmd.m_tChannelLockCmd.m_nDurationTime;
				tValidManualCmd.m_tStepForwardCmd.m_nDelayTime = tValidManualCmd.m_tChannelLockCmd.m_nDelayTime;
				tValidManualCmd.m_bDirectionCmd = false;
				memset(&tValidManualCmd.m_tDirectionCmd,0,sizeof(TDirectionCmd));
				tValidManualCmd.m_bPatternInterruptCmd = false;
				memset(&tValidManualCmd.m_tPatternInterruptCmd,0,sizeof(TPatternInterruptCmd));
				tValidManualCmd.m_bChannelLockCmd = false;
				memset(&tValidManualCmd.m_tChannelLockCmd,0,sizeof(TChannelLockCmd));
				memset(&tValidManualCmd.m_tPhaseLockPara,0,sizeof(TPhaseLockPara));
				tValidManualCmd.m_bPhaseToChannelLock = false;
				tValidManualCmd.m_bPreemptCtlCmd = false;
				m_pOpenATCRunStatus->SetValidManualCmd(tValidManualCmd);
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "ProcessChannelLock, ChannelLock Duration End, Return To StepWard");

				//初始化下一个阶段的相位
				for (int i = 0;i < m_tFixTimeCtlInfo.m_nRingCount;i++)
				{
					InitNextStagePhase(i, m_tFixTimeCtlInfo.m_nCurPhaseIndex[i], tValidManualCmd.m_tChannelLockCmd.m_nTargetStageIndex, true);
				}

				THWPanelBtnStatus tHWPanelBtnStatus;
				tHWPanelBtnStatus.m_nHWPanelBtnIndex = HWPANEL_BTN_STEP;
				tHWPanelBtnStatus.m_bHWPanelBtnStatus = true;
				m_pOpenATCRunStatus->SetPanelBtnStatusList(tHWPanelBtnStatus);
			}
			else //用户干预超时，回到自主
			{
				m_nManualCurStatus = MANUAL_STAGE_TRANS;//需要置为过渡状态
                CreateManualCmdReturnToSelf(tValidManualCmd);//生成回到自主的命令，COpenATCLogicCtlManager可以根据这个命令调用自主函数回到自主
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "ProcessChannelLock, ChannelLock Duration End, CreateManualCmdReturnToSelf");
			}
		}
	}
	else
	{
		//过渡失败的原因有：1.同阶段的锁定相位的灯色正在过渡
		//2.目标相位的灯色一直保持绿，非目标相位所在环的相位正在过渡 
		//3.目标相位的灯色在过渡，非目标相位所在环的相位正在过渡
		SwitchLockChannelPhaseToNext();

		if (m_nManualCurStatus == MANUAL_STAGE_TRANS)
		{
			m_nManualCurStatus = MANUAL_CONTROL_STATUS;
		}

		if (m_nManualCurStatus == MANUAL_CONTROL_STATUS)
		{
			//通道锁定时，目标相位正在锁定，其他相位需要过渡到目标相位所在阶段时，置过渡状态
			for (int nPhaseIndex = 0; nPhaseIndex < MAX_PHASE_COUNT; nPhaseIndex++)
			{
				if (m_tLockPhaseData[nPhaseIndex].nCurLockPhaseID == 0)
				{
					continue;
				}

				if (m_tLockPhaseData[nPhaseIndex].bNeedTransFlag && !m_tLockPhaseData[nPhaseIndex].bSwitchSuccessFlag)
				{
					m_nManualCurStatus = MANUAL_STAGE_TRANS;
				}
			}
		}
	}
}

/*==================================================================== 
函数名 ：PatternSwitchToChannelLock
功能 ：带方案的控制方式切到通道锁定
算法实现 ： 
参数说明 ：tValidManualCmd，手动命令
返回值说明：无
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 李永萍          创建函数 
====================================================================*/
void  CLogicCtlManual::PatternSwitchToChannelLock(TManualCmd  & tValidManualCmd)
{
	int	nGreenPhaseCount = 0;
	int nClosePhaseCount = 0; 
	CheckIfStageHavePhaseInGreen(nGreenPhaseCount, nClosePhaseCount);
	if (nGreenPhaseCount > 0)
	{
		PTPhase pPhaseInfo;
		PTRingCtlInfo ptRingRunInfo;

		bool bFlag = false;
		int nIndex = 0;
		int nCnt = 0;
		int nCurStageIndex	= m_tFixTimeCtlInfo.m_nCurStageIndex;
	
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
				// 判断当前相位是否跨下一个阶段和当前阶段
				if (m_tRunStageInfo.m_PhaseRunstageInfo[nCurStageIndex].m_nConcurrencyPhase[i] == m_tRunStageInfo.m_PhaseRunstageInfo[nStageIndexTarget].m_nConcurrencyPhase[i])
				{
					if (tValidManualCmd.m_bChannelLockCmd)//切通道锁定，跨阶段的相位需要在当前阶段立刻过渡完
					{
						m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "PatternSwitchToChannelLock, RingIndex:%d PhaseIndex:%d SetGreenTime", i, nIndex);
						AdjustPhaseGreenTime(i, nIndex, nStageIndexTarget, MANUAL_MINI_GREEN);
						AdjustPedPhaseGreenTime(i, nIndex, nStageIndexTarget, MANUAL_MINI_GREEN);
					}
				}
				else
				{
					if (tValidManualCmd.m_bChannelLockCmd)
					{
						m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "PatternSwitchToChannelLock, RingIndex:%d PhaseIndex:%d SetGreenTime", i, nIndex);
					}
					else
					{
						m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "PatternSwitchToChannelUnLock, RingIndex:%d PhaseIndex:%d SetGreenTime", i, nIndex);
					}
					AdjustPhaseGreenTime(i, nIndex, nStageIndexTarget, MANUAL_MINI_GREEN);
					AdjustPedPhaseGreenTime(i, nIndex, nStageIndexTarget, MANUAL_MINI_GREEN);
				}
				//通道锁定开始前继续保持绿色的通道
				if (tValidManualCmd.m_bChannelLockCmd)
				{
					if (ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPedPhaseStage == C_CH_PHASESTAGE_G)
					{
						SetKeepGreenChannelBeforeControlChannel(pPhaseInfo->m_byPhaseNumber, pPhaseInfo->m_byPhaseNumber, tValidManualCmd);
					}
					else
					{
						SetKeepGreenChannelBeforeControlChannel(pPhaseInfo->m_byPhaseNumber, 0, tValidManualCmd);
					}
				}
			}
		}
		
		ReSetNeglectPhaseStageRunTime();
	}
	
	memcpy(&m_tOldValidManualCmd, &tValidManualCmd, sizeof(tValidManualCmd));
}

/*==================================================================== 
函数名 ：GetTransRunStatusBeforeLockPhase
功能 ：设置锁定相位的运行状态
算法实现 ： 
参数说明 ：tRunStatus，相位的运行状态
返回值说明：无
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 李永萍          创建函数 
====================================================================*/
void CLogicCtlManual::GetTransRunStatusBeforeLockPhase(TPhaseRunStatus & tRunStatus)
{
	int i = 0, j = 0, k = 0;
	for (i = 0;i < tRunStatus.m_nRingCount;i ++)
    {
        for (k = 0;k < tRunStatus.m_atRingRunStatus[i].m_nPhaseCount;k ++)
		{
			//if (tRunStatus.m_atRingRunStatus[i].m_atPhaseStatus[k].m_byPhaseID == tValidManualCmd.m_tPhaseLockPara.m_nPhaseLockID[k])
			{
				//获取当前机动车通道的灯色
				for (j = 0;j < MAX_CHANNEL_COUNT;j++)
				{
					if (m_atChannelInfo[j].m_byChannelNumber == 0)
					{
						continue;
					}

					if ((m_atChannelInfo[j].m_byChannelControlType == VEH_CHA || m_atChannelInfo[j].m_byChannelControlType == PED_CHA) &&
						 m_atChannelInfo[j].m_byChannelControlSource == tRunStatus.m_atRingRunStatus[i].m_atPhaseStatus[k].m_byPhaseID)
					{
						if (m_bKeepGreenChannelBeforeControlChannelFlag[j])
						{
							tRunStatus.m_atRingRunStatus[i].m_atPhaseStatus[k].m_chPhaseStatus = C_CH_PHASESTAGE_G;
							tRunStatus.m_atRingRunStatus[i].m_atPhaseStatus[k].m_chPedPhaseStatus = C_CH_PHASESTAGE_G;
						}
						//m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "GetTransRunStatusBeforeLockPhase PhaseID:%d Channel:%d", tRunStatus.m_atRingRunStatus[i].m_atPhaseStatus[k].m_byPhaseID, j + 1);
					}
				}
			}
		}
	}

	for (i = 0;i < tRunStatus.m_atOverlapRunStatus.m_nOverlapPhaseCount;i++)
	{
		//获取当前机动车通道的灯色
		for (j = 0;j < MAX_CHANNEL_COUNT;j++)
		{
			if (m_atChannelInfo[j].m_byChannelNumber == 0)
			{
				continue;
			}

			if ((m_atChannelInfo[j].m_byChannelControlType == OVERLAP_CHA || m_atChannelInfo[j].m_byChannelControlType == OVERLAP_PED_CHA) && 
				m_atChannelInfo[j].m_byChannelControlSource == tRunStatus.m_atOverlapRunStatus.m_atOverlapPhaseRunStatus[i].m_byOverlapPhaseID)
			{
				if (m_bKeepGreenChannelBeforeControlChannelFlag[j])
				{
					tRunStatus.m_atOverlapRunStatus.m_atOverlapPhaseRunStatus[i].m_byOverlapType = GetOverlapType(C_CH_PHASESTAGE_G);
					tRunStatus.m_atOverlapRunStatus.m_atOverlapPhaseRunStatus[i].m_byOverlapPedType = GetOverlapType(C_CH_PHASESTAGE_G);
				}
			}
		}
	}
}

/*==================================================================== 
函数名 ：GetLockPhaseRunStatus
功能 ：设置锁定相位的运行状态
算法实现 ： 
参数说明 ：tRunStatus，相位的运行状态
返回值说明：无
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 李永萍          创建函数 
====================================================================*/
void CLogicCtlManual::GetLockPhaseRunStatus(TPhaseRunStatus & tRunStatus, bool bInvalidPhaseCmd, TPhaseLockPara tInvalidPhaseLockPara)
{
	if (m_tLockPhaseStage.nLockPhaseCount == 0)
	{
		return;
	}

	int i = 0;
	int j = 0;
	int k = 0;
	int nPedPhaseIndex = 0;
	TLogicCtlStatus tCtlStatus;
	m_pOpenATCRunStatus->GetLogicCtlStatus(tCtlStatus);

	tRunStatus.m_nCurCtlMode = CTL_MODE_PHASE_LOCK;

	TManualCmd  tValidManualCmd;
	memset(&tValidManualCmd,0,sizeof(tValidManualCmd));
	m_pOpenATCRunStatus->GetValidManualCmd(tValidManualCmd);

	int nDuration = tValidManualCmd.m_tChannelLockCmd.m_tNextChannelLockCtrlCmd.m_nDuration;
	int nGreenFlash = tValidManualCmd.m_tChannelLockCmd.m_tNextChannelLockCtrlCmd.m_nGreenFlash;
	int nYellow = tValidManualCmd.m_tChannelLockCmd.m_tNextChannelLockCtrlCmd.m_nYellow;
	int nRedClear = tValidManualCmd.m_tChannelLockCmd.m_tNextChannelLockCtrlCmd.m_nRedClear;

	int nPhaseTime = nDuration + nGreenFlash + nYellow + nRedClear;

	static bool bLockRing[MAX_RING_COUNT] = {false, false, false, false};
	static int  nLockPhaseIndex[MAX_RING_COUNT] = {0, 0, 0, 0};
	static bool bStartTransToEnd[MAX_RING_COUNT] = {false, false, false, false};

	bool bFindLockPhase[MAX_RING_COUNT] = {false, false, false, false};
	bool bSetNotFindLockPhaseStatus[MAX_RING_COUNT] = {false, false, false, false};

	static bool bFirstInvalidPhaseLockCmd = true;

	for (i = 0;i < tRunStatus.m_nRingCount;i++)
	{
		for (j = 0;j < m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_nPhaseCount;j++)
		{
			tRunStatus.m_atRingRunStatus[i].m_atPhaseStatus[j].m_chPhaseLockStatus = 0;//相位锁定状态

			tRunStatus.m_atRingRunStatus[i].m_atPhaseStatus[j].m_chPhaseStatus = C_CH_PHASESTAGE_R;
			tRunStatus.m_atRingRunStatus[i].m_atPhaseStatus[j].m_chPedPhaseStatus = C_CH_PHASESTAGE_R;

			if (!bInvalidPhaseCmd)
			{
				for (k = 0; k < m_tLockPhaseStage.nLockPhaseCount;k++)
				{
					if (tRunStatus.m_atRingRunStatus[i].m_atPhaseStatus[j].m_byPhaseID == m_tLockPhaseStage.nLockPhaseID[k])
					{
						tRunStatus.m_atRingRunStatus[i].m_nCurRunPhaseIndex = j;//当前运行相位索引
						tRunStatus.m_atRingRunStatus[i].m_atPhaseStatus[j].m_chPhaseLockStatus = 1;
						tRunStatus.m_atRingRunStatus[i].m_atPhaseStatus[j].m_chPhaseStatus = m_tLockPhaseStage.chLockPhaseStage[k];
						if (m_tLockPhaseStage.chLockPhaseStage[k] != C_CH_PHASESTAGE_Y)
						{
							tRunStatus.m_atRingRunStatus[i].m_atPhaseStatus[j].m_chPedPhaseStatus = m_tLockPhaseStage.chLockPhaseStage[k];
						}
						else
						{
							tRunStatus.m_atRingRunStatus[i].m_atPhaseStatus[j].m_chPedPhaseStatus = C_CH_PHASESTAGE_R;
						}

						//m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "GetLockPhaseRunStatus RingIndex:%d CurRunPhaseIndex:%d PhaseID:%d PhaseLockStatus:%d PhaseStatus:%c", 
						//i, tRunStatus.m_atRingRunStatus[i].m_nCurRunPhaseIndex, tRunStatus.m_atRingRunStatus[i].m_atPhaseStatus[j].m_byPhaseID,
						//tRunStatus.m_atRingRunStatus[i].m_atPhaseStatus[j].m_chPhaseLockStatus, tRunStatus.m_atRingRunStatus[i].m_atPhaseStatus[j].m_chPhaseStatus);
						break;
					}
				}

				bFirstInvalidPhaseLockCmd = true;
			}
			else
			{
				tRunStatus.m_atRingRunStatus[i].m_atPhaseStatus[j].m_chPhaseStatus = C_CH_PHASESTAGE_R;
				tRunStatus.m_atRingRunStatus[i].m_atPhaseStatus[j].m_chPedPhaseStatus = C_CH_PHASESTAGE_R;

				if (bFirstInvalidPhaseLockCmd)
				{
					bFirstInvalidPhaseLockCmd = false;

					m_tLockPhaseStage.nLockPhaseCount = 0;
					memset(m_tLockPhaseStage.nLockPhaseID, 0, sizeof(m_tLockPhaseStage. nLockPhaseID));

					for (i = 0; i < tInvalidPhaseLockPara.m_nPhaseLockCount; i++)
					{
						m_tLockPhaseStage.nLockPhaseID[m_tLockPhaseStage.nLockPhaseCount] = tInvalidPhaseLockPara.m_nPhaseLockID[i];
						m_tLockPhaseStage.nLockPhaseCount += 1;

						m_tLockPhaseStage.chLockPhaseStage[i] = C_CH_PHASESTAGE_G;
					}
				}

				for (k = 0; k < m_tLockPhaseStage.nLockPhaseCount;k++)
				{
					if (tRunStatus.m_atRingRunStatus[i].m_atPhaseStatus[j].m_byPhaseID == m_tLockPhaseStage.nLockPhaseID[k])
					{
						tRunStatus.m_atRingRunStatus[i].m_nCurRunPhaseIndex = j;//当前运行相位索引
						tRunStatus.m_atRingRunStatus[i].m_atPhaseStatus[j].m_chPhaseLockStatus = 1;
						tRunStatus.m_atRingRunStatus[i].m_atPhaseStatus[j].m_chPhaseStatus = m_tLockPhaseStage.chLockPhaseStage[k];
						if (m_tLockPhaseStage.chLockPhaseStage[k] != C_CH_PHASESTAGE_Y)
						{
							tRunStatus.m_atRingRunStatus[i].m_atPhaseStatus[j].m_chPedPhaseStatus = m_tLockPhaseStage.chLockPhaseStage[k];
						}
						else
						{
							tRunStatus.m_atRingRunStatus[i].m_atPhaseStatus[j].m_chPedPhaseStatus = C_CH_PHASESTAGE_R;
						}

						//m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "GetLockPhaseRunStatus RingIndex:%d CurRunPhaseIndex:%d PhaseID:%d PhaseLockStatus:%d PhaseStatus:%c", 
						//i, tRunStatus.m_atRingRunStatus[i].m_nCurRunPhaseIndex, tRunStatus.m_atRingRunStatus[i].m_atPhaseStatus[j].m_byPhaseID,
						//tRunStatus.m_atRingRunStatus[i].m_atPhaseStatus[j].m_chPhaseLockStatus, tRunStatus.m_atRingRunStatus[i].m_atPhaseStatus[j].m_chPhaseStatus);
						break;
					}
				}
			}

			//获取当前机动车通道的灯色
			/*for (k = 0;k < m_nChannelCount;k++)
			{
				if (m_atChannelInfo[k].m_byChannelNumber == 0)
				{
					continue;
				}

				if ((m_atChannelInfo[k].m_byChannelControlType == VEH_CHA || m_atChannelInfo[k].m_byChannelControlType == PED_CHA) &&
					 m_atChannelInfo[k].m_byChannelControlSource == tRunStatus.m_atRingRunStatus[i].m_atPhaseStatus[j].m_byPhaseID)
				{
					
					for (int nIndex = 0;nIndex < MAX_PHASE_COUNT;nIndex++)
					{
						if (bInvalidPhaseCmd)
						{
							if (tRunStatus.m_atRingRunStatus[i].m_atPhaseStatus[j].m_byPhaseID == tInvalidPhaseLockPara.m_nPhaseLockID[nIndex])
							{
								bFindLockPhase[i] = true;
							}
						}
						else
						{
							if (!m_bOldLockChannelCmdEndFlag)
							{
								if (tRunStatus.m_atRingRunStatus[i].m_atPhaseStatus[j].m_byPhaseID == m_tOldValidManualCmd.m_tPhaseLockPara.m_nPhaseLockID[nIndex])
								{
									bFindLockPhase[i] = true;
								}
							}
							else
							{
								if (tRunStatus.m_atRingRunStatus[i].m_atPhaseStatus[j].m_byPhaseID == tValidManualCmd.m_tPhaseLockPara.m_nPhaseLockID[nIndex])
								{
									bFindLockPhase[i] = true;
								}
							}
						}
					}

					if (m_achLampClr[(m_atChannelInfo[k].m_byChannelNumber - 1) * 3] == LAMP_CLR_OFF && 
						m_achLampClr[(m_atChannelInfo[k].m_byChannelNumber - 1) * 3 + 1] == LAMP_CLR_OFF &&
						m_achLampClr[(m_atChannelInfo[k].m_byChannelNumber - 1) * 3 + 2] == LAMP_CLR_OFF)
					{
						tRunStatus.m_atRingRunStatus[i].m_atPhaseStatus[j].m_chPhaseStatus = C_CH_PHASESTAGE_OF;
						tRunStatus.m_atRingRunStatus[i].m_atPhaseStatus[j].m_chPedPhaseStatus = C_CH_PHASESTAGE_OF;
					}

					if (m_achLampClr[(m_atChannelInfo[k].m_byChannelNumber - 1) * 3] == LAMP_CLR_ON)
					{
						tRunStatus.m_atRingRunStatus[i].m_atPhaseStatus[j].m_chPhaseStatus = C_CH_PHASESTAGE_R;
						tRunStatus.m_atRingRunStatus[i].m_atPhaseStatus[j].m_chPedPhaseStatus = C_CH_PHASESTAGE_R;
						//tRunStatus.m_atRingRunStatus[i].m_atPhaseStatus[j].m_nCurStageRemainTime = nDuration + nGreenFlash + nYellow + nRedClear - m_nChannelDurationTime[j];
					}

					if (m_achLampClr[(m_atChannelInfo[k].m_byChannelNumber - 1) * 3 + 1] == LAMP_CLR_ON)
					{
						tRunStatus.m_atRingRunStatus[i].m_atPhaseStatus[j].m_chPhaseStatus = C_CH_PHASESTAGE_Y;
						tRunStatus.m_atRingRunStatus[i].m_atPhaseStatus[j].m_chPedPhaseStatus = C_CH_PHASESTAGE_R;
						//tRunStatus.m_atRingRunStatus[i].m_atPhaseStatus[j].m_nCurStageRemainTime = nDuration + nGreenFlash + nYellow - m_nChannelDurationTime[j];
					}
		              
					if (m_achLampClr[(m_atChannelInfo[k].m_byChannelNumber - 1) * 3 + 2] == LAMP_CLR_ON)
					{
						tRunStatus.m_atRingRunStatus[i].m_atPhaseStatus[j].m_chPhaseStatus = C_CH_PHASESTAGE_G;
						tRunStatus.m_atRingRunStatus[i].m_atPhaseStatus[j].m_chPedPhaseStatus = C_CH_PHASESTAGE_G;
						//tRunStatus.m_atRingRunStatus[i].m_atPhaseStatus[j].m_nCurStageRemainTime = nDuration - m_nChannelDurationTime[j];

						bStartTransToEnd[i] = false;
						if (!bStartTransToEnd[i])
						{
							bLockRing[i] = false;
						    nLockPhaseIndex[i] = 0;
						}

						bLockRing[i] = true;
						nLockPhaseIndex[i] = j;
					}

					if (m_achLampClr[(m_atChannelInfo[k].m_byChannelNumber - 1) * 3 + 2] == LAMP_CLR_FLASH)
					{
						tRunStatus.m_atRingRunStatus[i].m_atPhaseStatus[j].m_chPhaseStatus = C_CH_PHASESTAGE_GF;
						tRunStatus.m_atRingRunStatus[i].m_atPhaseStatus[j].m_chPedPhaseStatus = C_CH_PHASESTAGE_GF;
						//tRunStatus.m_atRingRunStatus[i].m_atPhaseStatus[j].m_nCurStageRemainTime = nDuration + nGreenFlash - m_nChannelDurationTime[j];

						bLockRing[i] = true;
						nLockPhaseIndex[i] = j;

						bStartTransToEnd[i] = true;
					}
				}
			}*/
		}

		/*if (!bInvalidPhaseCmd && bLockRing[i])
		{
			if (m_bOldLockChannelCmdEndFlag && bStartTransToEnd[i])
			{

			}
			else
			{
				tRunStatus.m_atRingRunStatus[i].m_nCurRunPhaseIndex = nLockPhaseIndex[i];//当前运行相位索引
				tRunStatus.m_atRingRunStatus[i].m_atPhaseStatus[nLockPhaseIndex[i]].m_chPhaseLockStatus = 1;
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "GetLockPhaseRunStatus RingIndex:%d CurRunPhaseIndex:%d PhaseLockStatus:%d", i, tRunStatus.m_atRingRunStatus[i].m_nCurRunPhaseIndex, tRunStatus.m_atRingRunStatus[i].m_atPhaseStatus[nLockPhaseIndex[i]].m_chPhaseLockStatus);
			}
		}*/
	}

	static bool bOverlapLockRing[MAX_RING_COUNT] = {false, false, false, false};
	static int  nOverlapLockPhaseIndex[MAX_RING_COUNT] = {0, 0, 0, 0};
	static bool bOverlapStartTransToEnd[MAX_RING_COUNT] = {false, false, false, false};

	int nRingIndex = 0, nPhaseIndex = 0, nIndex = 0;

	for (i = 0;i < tRunStatus.m_atOverlapRunStatus.m_nOverlapPhaseCount;i++)
	{
		for (k = 0;k < MAX_PHASE_COUNT_IN_OVERLAP;k++)
		{
			if (tRunStatus.m_atOverlapRunStatus.m_atOverlapPhaseRunStatus[i].m_nMotherPhase[k] > 0)
			{
				/*GetPhaseIndexByPhaseNumber(nRingIndex, nPhaseIndex, (int)tRunStatus.m_atOverlapRunStatus.m_atOverlapPhaseRunStatus[i].m_nMotherPhase[k]);

				bool bFlag = false;

				if (bInvalidPhaseCmd)
				{
					for (nIndex = 0;nIndex < MAX_PHASE_COUNT;nIndex++)
					{
						if ((int)tRunStatus.m_atOverlapRunStatus.m_atOverlapPhaseRunStatus[i].m_nMotherPhase[k] == tInvalidPhaseLockPara.m_nPhaseLockID[nIndex])
						{
							bFlag = true;
						}
					}
				}
				else
				{
					for (nIndex = 0;nIndex < MAX_PHASE_COUNT;nIndex++)
					{
						if (m_bOldLockChannelCmdEndFlag)
						{
							if (tRunStatus.m_atRingRunStatus[nRingIndex].m_atPhaseStatus[nPhaseIndex].m_byPhaseID == tValidManualCmd.m_tPhaseLockPara.m_nPhaseLockID[nIndex])
							{
								bFlag = true;
							}
						}
						else
						{
							if (tRunStatus.m_atRingRunStatus[nRingIndex].m_atPhaseStatus[nPhaseIndex].m_byPhaseID == m_tOldValidManualCmd.m_tPhaseLockPara.m_nPhaseLockID[nIndex])
							{
								bFlag = true;
							}
						}
					}
				}
				
				if (!bFlag)
				{
					continue;
				}*/

				//获取当前跟随通道的灯色
				for (j = 0;j < m_nChannelCount;j++)
				{
					if (m_atChannelInfo[j].m_byChannelNumber == 0)
					{
						continue;
					}

					if ((m_atChannelInfo[j].m_byChannelControlType == OVERLAP_CHA || m_atChannelInfo[j].m_byChannelControlType == OVERLAP_PED_CHA) && 
						 m_atChannelInfo[j].m_byChannelControlSource == tRunStatus.m_atOverlapRunStatus.m_atOverlapPhaseRunStatus[i].m_byOverlapPhaseID)
					{
						if (m_achLampClr[(m_atChannelInfo[j].m_byChannelNumber - 1) * 3] == LAMP_CLR_OFF && 
							m_achLampClr[(m_atChannelInfo[j].m_byChannelNumber - 1) * 3 + 1] == LAMP_CLR_OFF &&
							m_achLampClr[(m_atChannelInfo[j].m_byChannelNumber - 1) * 3 + 2] == LAMP_CLR_OFF)
						{
							//tRunStatus.m_atRingRunStatus[nRingIndex].m_atPhaseStatus[nPhaseIndex].m_chPhaseStatus = C_CH_PHASESTAGE_OF;
							//tRunStatus.m_atRingRunStatus[nRingIndex].m_atPhaseStatus[nPhaseIndex].m_chPedPhaseStatus = C_CH_PHASESTAGE_OF;
						}

						if (m_achLampClr[(m_atChannelInfo[j].m_byChannelNumber - 1) * 3] == LAMP_CLR_ON)
						{
							tRunStatus.m_atOverlapRunStatus.m_atOverlapPhaseRunStatus[i].m_byOverlapType = GetOverlapType(C_CH_PHASESTAGE_R);
							tRunStatus.m_atOverlapRunStatus.m_atOverlapPhaseRunStatus[i].m_byOverlapPedType = GetOverlapType(C_CH_PHASESTAGE_R);
							//tRunStatus.m_atOverlapRunStatus.m_atOverlapPhaseRunStatus[i].m_nOverlapRemainTime = nRedClear - m_nChannelDurationTime[j];

							//tRunStatus.m_atRingRunStatus[nRingIndex].m_atPhaseStatus[nPhaseIndex].m_chPhaseStatus = C_CH_PHASESTAGE_R;
							//tRunStatus.m_atRingRunStatus[nRingIndex].m_atPhaseStatus[nPhaseIndex].m_chPedPhaseStatus = C_CH_PHASESTAGE_R;
						}

						if (m_achLampClr[(m_atChannelInfo[j].m_byChannelNumber - 1) * 3 + 1] == LAMP_CLR_ON)
						{
							tRunStatus.m_atOverlapRunStatus.m_atOverlapPhaseRunStatus[i].m_byOverlapType = GetOverlapType(C_CH_PHASESTAGE_Y);
							tRunStatus.m_atOverlapRunStatus.m_atOverlapPhaseRunStatus[i].m_byOverlapPedType = GetOverlapType(C_CH_PHASESTAGE_R);
							//tRunStatus.m_atOverlapRunStatus.m_atOverlapPhaseRunStatus[i].m_nOverlapRemainTime = nYellow - m_nChannelDurationTime[j];

							//tRunStatus.m_atRingRunStatus[nRingIndex].m_atPhaseStatus[nPhaseIndex].m_chPhaseStatus = C_CH_PHASESTAGE_Y;
							//tRunStatus.m_atRingRunStatus[nRingIndex].m_atPhaseStatus[nPhaseIndex].m_chPedPhaseStatus = C_CH_PHASESTAGE_R;
						}
		              
						if (m_achLampClr[(m_atChannelInfo[j].m_byChannelNumber - 1) * 3 + 2] == LAMP_CLR_ON)
						{
							tRunStatus.m_atOverlapRunStatus.m_atOverlapPhaseRunStatus[i].m_byOverlapType = GetOverlapType(C_CH_PHASESTAGE_G);
							tRunStatus.m_atOverlapRunStatus.m_atOverlapPhaseRunStatus[i].m_byOverlapPedType = GetOverlapType(C_CH_PHASESTAGE_G);
							//tRunStatus.m_atOverlapRunStatus.m_atOverlapPhaseRunStatus[i].m_nOverlapRemainTime = nDuration - m_nChannelDurationTime[j];

							//tRunStatus.m_atRingRunStatus[nRingIndex].m_atPhaseStatus[nPhaseIndex].m_chPhaseStatus = C_CH_PHASESTAGE_G;
							//tRunStatus.m_atRingRunStatus[nRingIndex].m_atPhaseStatus[nPhaseIndex].m_chPedPhaseStatus = C_CH_PHASESTAGE_G;

							/*bool bOverlapOldPhaseFlag = false;
							bool bOverlapNewPhaseFlag = false;
							int  nOverlapOldPhase = 0;
							if (!bInvalidPhaseCmd && !m_bOldLockChannelCmdEndFlag && !bFindLockPhase[nRingIndex])
							{
								for (nIndex = 0;nIndex < m_tOldValidManualCmd.m_tPhaseLockPara.m_nPhaseLockCount;nIndex++)
								{
									if (IsPhaseNumInOverlap(m_tOldValidManualCmd.m_tPhaseLockPara.m_nPhaseLockID[nIndex], i))
									{  
										bOverlapOldPhaseFlag = true;
										nOverlapOldPhase = m_tOldValidManualCmd.m_tPhaseLockPara.m_nPhaseLockID[nIndex];
										break;
									}
								}

								for (nIndex = 0;nIndex < tValidManualCmd.m_tPhaseLockPara.m_nPhaseLockCount;nIndex++)
								{
									if (tValidManualCmd.m_tPhaseLockPara.m_nPhaseLockID[nIndex] == nOverlapOldPhase &&
										IsPhaseNumInOverlap(tValidManualCmd.m_tPhaseLockPara.m_nPhaseLockID[nIndex], i))
									{
										bOverlapNewPhaseFlag = true;
										break;
									}
								}
							}

							if (bOverlapOldPhaseFlag && bOverlapNewPhaseFlag)
							{
								tRunStatus.m_atRingRunStatus[nRingIndex].m_atPhaseStatus[nPhaseIndex].m_chPhaseStatus = C_CH_PHASESTAGE_G;
								tRunStatus.m_atRingRunStatus[nRingIndex].m_atPhaseStatus[nPhaseIndex].m_chPedPhaseStatus = C_CH_PHASESTAGE_G;
								bSetNotFindLockPhaseStatus[nRingIndex] = true;
								m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "1111111111111 RingIndex:%d PhaseIndex:%d PhaseStatus:%c", nRingIndex, nPhaseIndex, tRunStatus.m_atRingRunStatus[nRingIndex].m_atPhaseStatus[nPhaseIndex].m_chPhaseStatus);
							}*/

							bOverlapStartTransToEnd[nRingIndex] = false;
							if (!bOverlapStartTransToEnd[nRingIndex])
							{
								bOverlapLockRing[nRingIndex] = false;
								nOverlapLockPhaseIndex[nRingIndex] = 0;
							}

							bOverlapLockRing[nRingIndex] = true;
							nOverlapLockPhaseIndex[nRingIndex] = nPhaseIndex;
						}

						if (m_achLampClr[(m_atChannelInfo[j].m_byChannelNumber - 1) * 3 + 2] == LAMP_CLR_FLASH)
						{
							tRunStatus.m_atOverlapRunStatus.m_atOverlapPhaseRunStatus[i].m_byOverlapType = GetOverlapType(C_CH_PHASESTAGE_GF);
							tRunStatus.m_atOverlapRunStatus.m_atOverlapPhaseRunStatus[i].m_byOverlapPedType = GetOverlapType(C_CH_PHASESTAGE_GF);
							//tRunStatus.m_atOverlapRunStatus.m_atOverlapPhaseRunStatus[i].m_nOverlapRemainTime = nGreenFlash - m_nChannelDurationTime[j];

							//tRunStatus.m_atRingRunStatus[nRingIndex].m_atPhaseStatus[nPhaseIndex].m_chPhaseStatus = C_CH_PHASESTAGE_GF;
							//tRunStatus.m_atRingRunStatus[nRingIndex].m_atPhaseStatus[nPhaseIndex].m_chPedPhaseStatus = C_CH_PHASESTAGE_GF;

							bOverlapLockRing[nRingIndex] = true;
							nOverlapLockPhaseIndex[nRingIndex] = nPhaseIndex;

							bOverlapStartTransToEnd[nRingIndex] = true;
						}
					}
				}

				/*if (!bInvalidPhaseCmd && bOverlapLockRing[nRingIndex])
				{
					if (m_bOldLockChannelCmdEndFlag && bOverlapStartTransToEnd[nRingIndex])
					{

					}
					else
					{
					   tRunStatus.m_atRingRunStatus[nRingIndex].m_nCurRunPhaseIndex = nOverlapLockPhaseIndex[nRingIndex];//当前运行相位索引
					   tRunStatus.m_atRingRunStatus[nRingIndex].m_atPhaseStatus[nOverlapLockPhaseIndex[nRingIndex]].m_chPhaseLockStatus = 1;

					   m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "GetLockOverlapPhaseRunStatus RingIndex:%d CurRunPhaseIndex:%d PhaseLockStatus:%d", nRingIndex, tRunStatus.m_atRingRunStatus[nRingIndex].m_nCurRunPhaseIndex, tRunStatus.m_atRingRunStatus[nRingIndex].m_atPhaseStatus[nOverlapLockPhaseIndex[nRingIndex]].m_chPhaseLockStatus);
					}
				}*/
			}
		}
	}

	if (bInvalidPhaseCmd)
	{
		for (nIndex = 0;nIndex < tInvalidPhaseLockPara.m_nPhaseLockCount;nIndex++)
		{
			GetPhaseIndexByPhaseNumber(nRingIndex, nPhaseIndex, tInvalidPhaseLockPara.m_nPhaseLockID[nIndex]);
			tRunStatus.m_atRingRunStatus[nRingIndex].m_nCurRunPhaseIndex = nPhaseIndex;
			tRunStatus.m_atRingRunStatus[nRingIndex].m_atPhaseStatus[nPhaseIndex].m_chPhaseLockStatus = 1;

			//m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "InvalidPhaseCmd GetLockPhaseRunStatus RingIndex:%d CurRunPhaseIndex:%d PhaseLockStatus:%d", nRingIndex, nPhaseIndex, tRunStatus.m_atRingRunStatus[nRingIndex].m_atPhaseStatus[nPhaseIndex].m_chPhaseLockStatus);
		}
	}

	for (i = 0;i < tRunStatus.m_nRingCount;i++)
	{
		for (j = 0;j < m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_nPhaseCount;j++)
		{
			if (tRunStatus.m_atRingRunStatus[i].m_atPhaseStatus[j].m_chPhaseLockStatus == 0 &&
				tRunStatus.m_atRingRunStatus[i].m_atPhaseStatus[j].m_chPhaseStatus == C_CH_PHASESTAGE_G)
			{
				tRunStatus.m_atRingRunStatus[i].m_atPhaseStatus[j].m_chPhaseStatus = C_CH_PHASESTAGE_R;
			}
		}
	}

	/*int nNotHaveLockPhaseRingIndex = -1;
	int nHaveLockPhaseRingIndex = -1;
	int nNotFindLockPhaseIndex = -1;
	for (i = 0;i < tRunStatus.m_nRingCount;i++)
	{
		if (!bFindLockPhase[i] && !bSetNotFindLockPhaseStatus[i])
		{
			if (i == 0)
			{
				nNotHaveLockPhaseRingIndex = 0;
				nHaveLockPhaseRingIndex = 1;
			}
			else if (i == 1)
			{
				nNotHaveLockPhaseRingIndex = 1;
				nHaveLockPhaseRingIndex = 0;
			}
			
			for (j = 0;j < m_tFixTimeCtlInfo.m_atPhaseSeq[nNotHaveLockPhaseRingIndex].m_nPhaseCount;j++)
			{
				for (nIndex = 0;nIndex < MAX_PHASE_COUNT;nIndex++)
				{
					if (bInvalidPhaseCmd)
					{
						if (tRunStatus.m_atRingRunStatus[nNotHaveLockPhaseRingIndex].m_atPhaseStatus[j].m_byPhaseID == tInvalidPhaseLockPara.m_nPhaseLockID[nIndex])
						{
							nNotFindLockPhaseIndex = j;
						}
					}
					else
					{
						if (!m_bOldLockChannelCmdEndFlag)
						{
							if (tRunStatus.m_atRingRunStatus[nNotHaveLockPhaseRingIndex].m_atPhaseStatus[j].m_byPhaseID == m_tOldValidManualCmd.m_tPhaseLockPara.m_nPhaseLockID[nIndex])
							{
								nNotFindLockPhaseIndex = j;
							}
						}
						else
						{
							if (tRunStatus.m_atRingRunStatus[nNotHaveLockPhaseRingIndex].m_atPhaseStatus[j].m_byPhaseID == tValidManualCmd.m_tPhaseLockPara.m_nPhaseLockID[nIndex])
							{
								nNotFindLockPhaseIndex = j;
							}
						}
					}
				}
			}

			for (j = 0;j < m_tFixTimeCtlInfo.m_atPhaseSeq[nHaveLockPhaseRingIndex].m_nPhaseCount;j++)
			{
				for (nIndex = 0;nIndex < MAX_PHASE_COUNT;nIndex++)
				{
					if (bInvalidPhaseCmd)
					{
						if (tRunStatus.m_atRingRunStatus[nHaveLockPhaseRingIndex].m_atPhaseStatus[j].m_byPhaseID == tInvalidPhaseLockPara.m_nPhaseLockID[nIndex])
						{
							tRunStatus.m_atRingRunStatus[nNotHaveLockPhaseRingIndex].m_atPhaseStatus[nNotFindLockPhaseIndex].m_chPhaseStatus = tRunStatus.m_atRingRunStatus[nHaveLockPhaseRingIndex].m_atPhaseStatus[j].m_chPhaseStatus;
							tRunStatus.m_atRingRunStatus[nNotHaveLockPhaseRingIndex].m_atPhaseStatus[nNotFindLockPhaseIndex].m_chPedPhaseStatus = tRunStatus.m_atRingRunStatus[nHaveLockPhaseRingIndex].m_atPhaseStatus[j].m_chPedPhaseStatus;
							m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "222222222222222 RingIndex:%d PhaseIndex:%d PhaseStatus:%c", nNotHaveLockPhaseRingIndex, nNotFindLockPhaseIndex, tRunStatus.m_atRingRunStatus[nNotHaveLockPhaseRingIndex].m_atPhaseStatus[nNotFindLockPhaseIndex].m_chPhaseStatus);
						}
					}
					else
					{
						if (!m_bOldLockChannelCmdEndFlag)
						{
							if (tRunStatus.m_atRingRunStatus[nHaveLockPhaseRingIndex].m_atPhaseStatus[j].m_byPhaseID == m_tOldValidManualCmd.m_tPhaseLockPara.m_nPhaseLockID[nIndex])
							{
								tRunStatus.m_atRingRunStatus[nNotHaveLockPhaseRingIndex].m_atPhaseStatus[nNotFindLockPhaseIndex].m_chPhaseStatus = tRunStatus.m_atRingRunStatus[nHaveLockPhaseRingIndex].m_atPhaseStatus[j].m_chPhaseStatus;
								tRunStatus.m_atRingRunStatus[nNotHaveLockPhaseRingIndex].m_atPhaseStatus[nNotFindLockPhaseIndex].m_chPedPhaseStatus = tRunStatus.m_atRingRunStatus[nHaveLockPhaseRingIndex].m_atPhaseStatus[j].m_chPedPhaseStatus;
								m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "333333333333333 RingIndex:%d PhaseIndex:%d PhaseStatus:%c", nNotHaveLockPhaseRingIndex, nNotFindLockPhaseIndex, tRunStatus.m_atRingRunStatus[nNotHaveLockPhaseRingIndex].m_atPhaseStatus[nNotFindLockPhaseIndex].m_chPhaseStatus);
							}
						}
						else
						{
							if (tRunStatus.m_atRingRunStatus[nHaveLockPhaseRingIndex].m_atPhaseStatus[j].m_byPhaseID == tValidManualCmd.m_tPhaseLockPara.m_nPhaseLockID[nIndex])
							{
								tRunStatus.m_atRingRunStatus[nNotHaveLockPhaseRingIndex].m_atPhaseStatus[nNotFindLockPhaseIndex].m_chPhaseStatus = tRunStatus.m_atRingRunStatus[nHaveLockPhaseRingIndex].m_atPhaseStatus[j].m_chPhaseStatus;
								tRunStatus.m_atRingRunStatus[nNotHaveLockPhaseRingIndex].m_atPhaseStatus[nNotFindLockPhaseIndex].m_chPedPhaseStatus = tRunStatus.m_atRingRunStatus[nHaveLockPhaseRingIndex].m_atPhaseStatus[j].m_chPedPhaseStatus;
								m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "444444444444444 RingIndex:%d PhaseIndex:%d PhaseStatus:%c", nNotHaveLockPhaseRingIndex, nNotFindLockPhaseIndex, tRunStatus.m_atRingRunStatus[nNotHaveLockPhaseRingIndex].m_atPhaseStatus[nNotFindLockPhaseIndex].m_chPhaseStatus);
							}
						}
					}
				}
			}

			break;
		}
	}*/
}

/*==================================================================== 
函数名 ：SetKeepGreenChannelBeforeControlChannel
功能 ：通道控制开始前继续保持绿色的通道
算法实现 ： 
参数说明 ：byGreenStageVehPhaseID, 当前处于绿色阶段的机动车相位ID
           byGreenStagePedPhaseID，当前处于绿色阶段的行人相位ID
           tValidManualCmd，有效的控制指令
返回值说明：无
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 李永萍          创建函数 
====================================================================*/
void CLogicCtlManual::SetKeepGreenChannelBeforeControlChannel(BYTE byGreenStageVehPhaseID, BYTE byGreenStagePedPhaseID, TManualCmd tValidManualCmd)
{
	if (IsNeglectPhase(byGreenStageVehPhaseID) || IsNeglectPhase(byGreenStagePedPhaseID))
	{
		return;
	}

	TOverlapTable atOverlapTable[MAX_OVERLAP_COUNT];
    m_pOpenATCParameter->GetOverlapTable(atOverlapTable);

	TAscManualPanel tAscManualPanel;
	memset(&tAscManualPanel, 0, sizeof(TAscManualPanel)); 
	m_pOpenATCParameter->GetManualPanelInfo(tAscManualPanel);

	int  nPanelKeyIndex = 0;
	int  nOverlapIndex = 0;
	int  i = 0, j = 0;
	if (tValidManualCmd.m_bDirectionCmd)
	{
        for (i = 0;i < MAX_PANEL_KEY_CFG_COUNT;i++)
	    {
		    if (tAscManualPanel.m_stPanelKeyCfg[i].m_byKeyNum == tValidManualCmd.m_tDirectionCmd.m_nNextDirectionIndex)
		    {    
				nPanelKeyIndex = i;
				break;
		    } 
	    }
	}

	BYTE byChannelStatus = CHANNEL_STATUS_DEFAULT;
	for (i = 0;i < m_nChannelCount;i++)
	{
		if (tValidManualCmd.m_bDirectionCmd)
		{
			byChannelStatus = tAscManualPanel.m_stPanelKeyCfg[nPanelKeyIndex].m_ChannelCfg[i].m_byChannelStatus;
		}
		else if (tValidManualCmd.m_bChannelLockCmd)
		{
			byChannelStatus = tValidManualCmd.m_tChannelLockCmd.m_tNextChannelLockCtrlCmd.m_nChannelLockStatus[i];
		}

		if (byChannelStatus == CHANNEL_STATUS_GREEN)
		{
			if (byGreenStageVehPhaseID && m_atChannelInfo[i].m_byChannelControlType == VEH_CHA)
			{
				if (m_atChannelInfo[i].m_byChannelControlSource == byGreenStageVehPhaseID)
				{
					m_bKeepGreenChannelBeforeControlChannelFlag[i] = true;
				}
			}
			else if (byGreenStagePedPhaseID && m_atChannelInfo[i].m_byChannelControlType == PED_CHA)
			{
				if (m_atChannelInfo[i].m_byChannelControlSource == byGreenStagePedPhaseID)
				{
					m_bKeepGreenChannelBeforeControlChannelFlag[i] = true;
				}
			}
			else if (byGreenStageVehPhaseID && m_atChannelInfo[i].m_byChannelControlType == OVERLAP_CHA)
			{
				for (j = 0;j < MAX_OVERLAP_COUNT;j++)
				{
					if (atOverlapTable[j].m_byOverlapNumber == m_atChannelInfo[i].m_byChannelControlSource)
					{
						for (nOverlapIndex = 0;nOverlapIndex < MAX_PHASE_COUNT_IN_OVERLAP;nOverlapIndex++)
						{
							if (atOverlapTable[j].m_byArrOverlapIncludedPhases[nOverlapIndex] == byGreenStageVehPhaseID)
							{
								m_bKeepGreenChannelBeforeControlChannelFlag[i] = true;
							}
						}    
					}
				}
			}
			else if (byGreenStagePedPhaseID && m_atChannelInfo[i].m_byChannelControlType == OVERLAP_PED_CHA)
			{
				for (j = 0;j < MAX_OVERLAP_COUNT;j++)
				{
					if (atOverlapTable[j].m_byOverlapNumber == m_atChannelInfo[i].m_byChannelControlSource)
					{
						for (nOverlapIndex = 0;nOverlapIndex < MAX_PHASE_COUNT_IN_OVERLAP;nOverlapIndex++)
						{
							if (atOverlapTable[j].m_byArrOverlapIncludedPhases[nOverlapIndex] == byGreenStagePedPhaseID)
							{
								m_bKeepGreenChannelBeforeControlChannelFlag[i] = true;
							}
						}    
					}
				}
			}
		}
	}
}

/*==================================================================== 
函数名 ：CheckSendGreenPulse
功能 ：检查绿脉冲能否发出
算法实现 ： 
参数说明 ：byPhaseNumber，相位ID
返回值说明：无
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 李永萍          创建函数 
====================================================================*/
bool CLogicCtlManual::CheckSendGreenPulse(BYTE byPhaseNumber)
{
	int  i = 0;
    for (i = 0;i < m_nChannelCount;i++)
    {
		if (m_atChannelInfo[i].m_byChannelNumber == 0)
		{
			continue;
		}

		if (m_bKeepGreenChannelBeforeControlChannelFlag[i])
		{
			if ((m_atChannelInfo[i].m_byChannelControlType == VEH_CHA || m_atChannelInfo[i].m_byChannelControlType == PED_CHA) &&
				(int)m_atChannelInfo[i].m_byChannelControlSource == byPhaseNumber)
			{
				return false;
			}
		}
	}
	
	return true;
}

/*==================================================================== 
函数名 ：PhaseTransBasedOnControlChannelFlag
功能 ：根据通道锁定切换时的继续保持绿灯标志，过渡当前相位
算法实现 ： 
参数说明 ：无
返回值说明：无
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 李永萍          创建函数 
====================================================================*/
void CLogicCtlManual::PhaseTransBasedOnControlChannelFlag()
{
	PTPhaseCtlInfo pPhaseInfo;
	PTRingCtlInfo ptRingRunInfo;
	int nIndex = 0, nPhaseID = 0;

	int nCurStageIndex	= m_tFixTimeCtlInfo.m_nCurStageIndex;
	
	int nStageIndexTarget = m_tFixTimeCtlInfo.m_nCurStageIndex + 1;
	if (nStageIndexTarget >= m_tRunStageInfo.m_nRunStageCount)
	{
		nStageIndexTarget = 0;
	}

	int  i = 0, j = 0;
	for (i = 0; i < m_tFixTimeCtlInfo.m_nRingCount; i++)
	{
		nIndex = m_tFixTimeCtlInfo.m_nCurPhaseIndex[i];
		ptRingRunInfo = &(m_tFixTimeCtlInfo.m_atPhaseSeq[i]);
		pPhaseInfo = (PTPhaseCtlInfo)&(ptRingRunInfo->m_atPhaseInfo[nIndex]);
		nPhaseID = pPhaseInfo->m_tPhaseParam.m_byPhaseNumber;

		if (ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseStage != C_CH_PHASESTAGE_G && !IsNeglectPhase(nPhaseID))
		{
			for (j = 0;j < m_nChannelCount;j++)
			{
				if (m_atChannelInfo[j].m_byChannelNumber == 0)
				{
					continue;
				}

				if (m_bKeepGreenChannelBeforeControlChannelFlag[j] && (int)m_atChannelInfo[j].m_byChannelControlSource == nPhaseID)
				{
					if (m_atChannelInfo[j].m_byChannelControlType == VEH_CHA || m_atChannelInfo[j].m_byChannelControlType == PED_CHA)
					{
						ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseStage = C_CH_PHASESTAGE_G;
						ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPedPhaseStage = C_CH_PHASESTAGE_G;
					}
				}	
			}

			if (ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseStage == C_CH_PHASESTAGE_G)
			{
				// 判断当前相位是否跨下一个阶段和当前阶段
				if (m_tRunStageInfo.m_PhaseRunstageInfo[nCurStageIndex].m_nConcurrencyPhase[i] == m_tRunStageInfo.m_PhaseRunstageInfo[nStageIndexTarget].m_nConcurrencyPhase[i])
				{
				
				}
				else
				{
					m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "PhaseTransBasedOnControlChannelFlag, RingIndex:%d PhaseIndex:%d SetGreenTime", i, nIndex);

					AdjustPhaseGreenTime(i, nIndex, nStageIndexTarget, MANUAL_MINI_GREEN);
					AdjustPedPhaseGreenTime(i, nIndex, nStageIndexTarget, MANUAL_MINI_GREEN);
				}
			}
		}
	}

	ReSetNeglectPhaseStageRunTime();
}

/*==================================================================== 
函数名 ：IsChannelNeedTranClr
功能 ：当前通道的灯色是否需要过度
算法实现 ： 
参数说明 ： byChannelIndex，通道编号
返回值说明：true，需要过渡
            false，不需要过渡
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 李永萍          创建函数 
====================================================================*/
bool CLogicCtlManual::IsChannelNeedTranClr(BYTE byChannelIndex)
{
	int  i = 0, j = 0, k = 0;
	for (i = 0;i < m_tFixTimeCtlInfo.m_nRingCount;i++)
	{
		int nPhaseIndex = m_tFixTimeCtlInfo.m_nCurPhaseIndex[i];
		PTRingCtlInfo ptRingRunInfo = &(m_tFixTimeCtlInfo.m_atPhaseSeq[i]);
		PTPhaseCtlInfo pPhaseInfo = (PTPhaseCtlInfo)&(ptRingRunInfo->m_atPhaseInfo[nPhaseIndex]);
		int nPhaseID = pPhaseInfo->m_tPhaseParam.m_byPhaseNumber;

		if (IsNeglectPhase(nPhaseID))
		{
			 return true;
		}

		if ((m_atChannelInfo[byChannelIndex].m_byChannelControlType == VEH_CHA || m_atChannelInfo[byChannelIndex].m_byChannelControlType == PED_CHA) && 
		     m_atChannelInfo[byChannelIndex].m_byChannelControlSource == nPhaseID)
		{
			 return false;
		}

		if (m_atChannelInfo[byChannelIndex].m_byChannelControlType == OVERLAP_CHA || m_atChannelInfo[byChannelIndex].m_byChannelControlType == OVERLAP_PED_CHA)
		{
			for (j = 0;j < m_tFixTimeCtlInfo.m_nOverlapCount;j++)
			{
				BYTE byOverlapNum = m_tFixTimeCtlInfo.m_atOverlapInfo[j].m_tOverlapParam.m_byOverlapNumber;
				if (byOverlapNum == m_atChannelInfo[byChannelIndex].m_byChannelControlSource)
				{
					for (k = 0;k < MAX_PHASE_COUNT_IN_OVERLAP;k++)
					{
						BYTE byMainPhaseNum = m_tFixTimeCtlInfo.m_atOverlapInfo[j].m_tOverlapParam.m_byArrOverlapIncludedPhases[k];
						if ((int)m_tFixTimeCtlInfo.m_atOverlapInfo[j].m_tOverlapParam.m_byArrOverlapIncludedPhases[k] == nPhaseID)
						{
							return false;
						}
					}
				}
			}
		}
	}

	return true;

}

/*==================================================================== 
函数名 ：ProcessAllClosePhaseInCurStage
功能 ：处理当前阶段所有相位都关断以后，卡在下一个阶段
算法实现 ： 
参数说明 ： tValidManualCmd：手动命令，nNextStageIndex，目标阶段号
返回值说明：无
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 李永萍          创建函数 
====================================================================*/
void CLogicCtlManual::ProcessAllClosePhaseInCurStage(TManualCmd  tValidManualCmd, int nNextStageIndex)
{
	memset(m_nStageRunTime, 0, sizeof(m_nStageRunTime));

	int  i = 0, j = 0, nNextPhaseID = 0;
	for (i = 0;i < m_tFixTimeCtlInfo.m_nRingCount;i++)
    {
		m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "ProcessAllClosePhaseInCurStage RingIndex:%d CurPhaseIndex:%d CurStageIndex:%d", i, m_tFixTimeCtlInfo.m_nCurPhaseIndex[i], m_tFixTimeCtlInfo.m_nCurStageIndex);

		m_nStageTimeForPhasePass[i] = 0;

		for (j = 0;j < m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_nPhaseCount;j++)
		{
			nNextPhaseID = m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[j].m_tPhaseParam.m_byPhaseNumber;
			if (m_tRunStageInfo.m_PhaseRunstageInfo[nNextStageIndex].m_nConcurrencyPhase[i] == nNextPhaseID &&
				m_tPhasePassCmdPhaseStatus.m_nPhasePassStatus[nNextPhaseID - 1] != PhasePassStatus_Close &&
				m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[j].m_chPhaseMode != NEGLECT_MODE && 
				m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[j].m_chPhaseMode != SHIELD_MODE)
			{
				m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[m_tFixTimeCtlInfo.m_nCurPhaseIndex[i]].m_chPhaseStage = C_CH_PHASESTAGE_F;   
				m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[m_tFixTimeCtlInfo.m_nCurPhaseIndex[i]].m_chPedPhaseStage = C_CH_PHASESTAGE_F; 

				m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[j].m_chPhaseStage = C_CH_PHASESTAGE_U;   
				m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[j].m_chPedPhaseStage = C_CH_PHASESTAGE_U; 
				m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[j].m_wPhaseStageRunTime = 0;
				m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[j].m_wPedPhaseStageRunTime = 0;

				TAscStepCfg tStepCfg;
				memset(&tStepCfg, 0, sizeof(tStepCfg));
				m_pOpenATCParameter->GetStepInfo(tStepCfg);
				if (tStepCfg.m_byStepType == STEP_COLOR)
				{
					m_pOpenATCRunStatus->SetIncludedCurPhaseInPhaseStatusCmdInStepColorFlag(false);
				}

				if (m_tFixTimeCtlInfo.m_nCurStageIndex != nNextStageIndex)
				{
					m_tFixTimeCtlInfo.m_nCurPhaseIndex[i] = j;  
				}

				/*int nPhaseGreenFlashTime	= m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[j].m_tPhaseParam.m_byGreenFlash;
				int nPedPhaseGreenFlashTime = m_tFixTimeCtlInfo.m_atPhaseSeq[j].m_atPhaseInfo[j].m_tPhaseParam.m_byPhasePedestrianClear;
				int nPhaseYellowTime		= m_tFixTimeCtlInfo.m_atPhaseSeq[j].m_atPhaseInfo[j].m_tPhaseParam.m_byPhaseYellowChange;
				int nPhaseRedTime			= m_tFixTimeCtlInfo.m_atPhaseSeq[j].m_atPhaseInfo[j].m_tPhaseParam.m_byPhaseRedClear;

				char chcurStage				= m_tFixTimeCtlInfo.m_atPhaseSeq[j].m_atPhaseInfo[j].m_chPhaseStage;
				char chPedcurStage			= m_tFixTimeCtlInfo.m_atPhaseSeq[j].m_atPhaseInfo[j].m_chPedPhaseStage;

				int nPhaseTransLightTime	= nPhaseGreenFlashTime + nPhaseYellowTime + nPhaseRedTime;
				int nPedPhaseTransLightTime = nPedPhaseGreenFlashTime + nPhaseRedTime;

				int nStageTime = m_tRunStageInfo.m_PhaseRunstageInfo[nNextStageIndex].m_nStageEndTime - m_tRunStageInfo.m_PhaseRunstageInfo[nNextStageIndex].m_nStageStartTime;

				int nSecNextStage = nNextStageIndex + 1;
				if (nSecNextStage >= m_tRunStageInfo.m_nRunStageCount)
				{
					nSecNextStage = 0;
				}
				int nSecNextStageTime = m_tRunStageInfo.m_PhaseRunstageInfo[nSecNextStage].m_nStageEndTime - m_tRunStageInfo.m_PhaseRunstageInfo[nSecNextStage].m_nStageStartTime;

				if (m_tRunStageInfo.m_PhaseRunstageInfo[nNextStageIndex].m_bPhaseStartTransitInCurStage[i])		//下个相位在下一个阶段过渡灯色
				{
					if (m_tRunStageInfo.m_PhaseRunstageInfo[nNextStageIndex].m_bDirectTransit[i])
					{
						m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[j].m_wPhaseGreenTime = nStageTime - nPhaseTransLightTime;
					}
					else
					{
						m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[j].m_wPhaseGreenTime = nStageTime - m_tRunStageInfo.m_PhaseRunstageInfo[nNextStageIndex].m_nPhaseCurStageTransitTime[i];
					}
				}
				else																									
				{
					if (m_tRunStageInfo.m_PhaseRunstageInfo[nSecNextStage].m_bDirectTransit[i])
					{
						m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[j].m_wPhaseGreenTime = nStageTime + nSecNextStageTime - nPhaseTransLightTime;
					}
					else
					{
						m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[j].m_wPhaseGreenTime = nStageTime + nSecNextStageTime;
					}
				}
				
				if (m_tRunStageInfo.m_PhaseRunstageInfo[nNextStageIndex].m_bPedPhaseStartTransitInCurStage[i])	//下个行人相位在下一个阶段过渡灯色
				{
					if (m_tRunStageInfo.m_PhaseRunstageInfo[nNextStageIndex].m_bDirectTransit[i])
					{
						m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[j].m_wPedPhaseGreenTime = nStageTime - nPedPhaseTransLightTime;
					}
					else
					{
						m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[j].m_wPedPhaseGreenTime = nStageTime - m_tRunStageInfo.m_PhaseRunstageInfo[nNextStageIndex].m_nPedPhaseCurStageTransitTime[i];
					}
				}
				else
				{
					if (m_tRunStageInfo.m_PhaseRunstageInfo[nSecNextStage].m_bDirectTransit[i])
					{
						m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[j].m_wPedPhaseGreenTime = nStageTime + nSecNextStageTime - nPedPhaseTransLightTime;
					}
					else
					{
						m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[j].m_wPedPhaseGreenTime = nStageTime + nSecNextStageTime;
					}
				}*/

				m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[j].m_wPhaseGreenTime = tValidManualCmd.m_tStepForwardCmd.m_nDelayTime +
                                   tValidManualCmd.m_tStepForwardCmd.m_nDurationTime;

				m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[j].m_wPedPhaseGreenTime = m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[j].m_wPhaseGreenTime +
				    m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[i].m_tPhaseParam.m_byGreenFlash - 
					m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[j].m_tPhaseParam.m_byPhasePedestrianClear;

				m_nStageTimeForPhasePass[i] = m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[j].m_wPhaseGreenTime + 
												m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[j].m_tPhaseParam.m_byGreenFlash +
												m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[j].m_tPhaseParam.m_byPhaseYellowChange +
												m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[j].m_tPhaseParam.m_byPhaseRedClear;
			
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "ProcessAllClosePhaseInCurStage RingIndex:%d NextPhaseIndex:%d PhaseTime:%d Stepward To Next Stage Success!", i, m_tFixTimeCtlInfo.m_nCurPhaseIndex[i], m_nStageTimeForPhasePass[i]);
				break;
			}
		}
    }

	ReSetNeglectPhasetBackUpTime();

	m_tFixTimeCtlInfo.m_nCurStageIndex = nNextStageIndex;
}

/*==================================================================== 
函数名 ：IsNeedTransBeforeControlChannel
功能 ：在通道控制之前是否需要过渡相位
算法实现 ： 
参数说明 ： nPhaseID，相位ID
返回值说明：true，是锁定相位
            false，不是锁定相位
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 李永萍          创建函数 
====================================================================*/
bool CLogicCtlManual::IsNeedTransBeforeControlChannel(int nPhaseID)
{
	int  i = 0;
	for (i = 0;i < MAX_CHANNEL_COUNT;i++)
	{
		if (m_atChannelInfo[i].m_byChannelNumber == 0)
		{
			continue;
		}

		if ((m_atChannelInfo[i].m_byChannelControlType == VEH_CHA || m_atChannelInfo[i].m_byChannelControlType == PED_CHA) &&
			 m_atChannelInfo[i].m_byChannelControlSource == nPhaseID)
		{
			if (m_bKeepGreenChannelBeforeControlChannelFlag[i])
			{
				return false;
			}
		}
	}

	return true;
}

/*==================================================================== 
函数名 ：GetPhaseGreenTimeAfterLockPhase
功能 ：获取锁定相位后的下一个阶段对应的相位的时间
算法实现 ： 
参数说明 ： nRingIndex：环号，nPhaseIndex：相位号，nNextStageIndex：阶段号，bChangeStageFlag：切阶段和切相位标志
返回值说明：无
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 李永萍          创建函数 
====================================================================*/
void  CLogicCtlManual::GetPhaseGreenTimeAfterLockPhase(int nRingIndex, int nPhaseIndex, int nNextStageIndex, bool bChangePhaseFlag)
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
		//if (chcurStage == C_CH_PHASESTAGE_G)
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

			//if (chPedcurStage == C_CH_PHASESTAGE_G)
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

	m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "GetPhaseGreenTimeAfterLockPhase, RingIndex=%d PhaseIndex=%d StageIndex=%d PhaseGreenTime=%d PedPhaseGreenTime=%d.", 
												nRingIndex, nPhaseIndex,nNextStageIndex, m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_wPhaseGreenTime,
												m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_wPedPhaseGreenTime);

}

/*==================================================================== 
函数名 ：GetLockPhaseData
功能 ：获取锁定相位参数
算法实现 ： 
参数说明 ： tValidManualCmd：手动命令
返回值说明：无
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 李永萍          创建函数 
====================================================================*/
void  CLogicCtlManual::GetLockPhaseData(TManualCmd  & tValidManualCmd)
{
	if (tValidManualCmd.m_tPhaseLockPara.m_nPhaseLockCount == 0)
	{
		return;
	}

	bool bFlag = false;
	int  i = 0, j = 0, k = 0, nCount = 0;
	int  nChannelCount[MAX_PHASE_COUNT];
	memset(nChannelCount, 0, sizeof(nChannelCount));

	memset(m_tLockPhaseData, 0, sizeof(m_tLockPhaseData));
	memset(m_bTargetLockPhaseChannelFlag, 0, sizeof(m_bTargetLockPhaseChannelFlag));
	memset(m_bNonTargetLockPhaseEndFlag, 0, sizeof(m_bNonTargetLockPhaseEndFlag));
	
	for (i = 0; i < tValidManualCmd.m_tPhaseLockPara.m_nPhaseLockCount; i++)
	{
		for (j = 0; j < m_tRunStageInfo.m_nRunStageCount; j++)      
		{
			for (k = 0;k < m_tRunStageInfo.m_PhaseRunstageInfo[j].m_nConcurrencyPhaseCount;k++)    
			{
				if (tValidManualCmd.m_tPhaseLockPara.m_nPhaseLockID[i] == m_tRunStageInfo.m_PhaseRunstageInfo[j].m_nConcurrencyPhase[k])
				{
					bFlag = false;
					for (int nIndex = 0;nIndex < nCount;nIndex++)
					{
						if (m_tLockPhaseData[nIndex].nCurLockPhaseID == tValidManualCmd.m_tPhaseLockPara.m_nPhaseLockID[i])
						{
							bFlag = true;
							m_tLockPhaseData[nIndex].nCurLockStageIndex = j;//相位跨阶段时，记录最后一个阶段编号
						}
					}
					if (!bFlag)
					{
						m_tLockPhaseData[nCount].nCurLockRingIndex = k;
						m_tLockPhaseData[nCount].nCurLockPhaseID = tValidManualCmd.m_tPhaseLockPara.m_nPhaseLockID[i];
						m_tLockPhaseData[nCount].nCurLockStageIndex = j;
						nCount += 1;
					}
					break;
				}
			}
		}
	}

	//多环只配一个锁定相位，且该锁定相位跨阶段，则当前阶段编号以跨的第一个阶段编号为准
	/*bFlag = false;
	if (m_tFixTimeCtlInfo.m_nRingCount > 1 && nCount == 1)
	{
		for (i = 0; i < m_tRunStageInfo.m_nRunStageCount; i++)      
		{
			for (j = 0;j < m_tRunStageInfo.m_PhaseRunstageInfo[i].m_nConcurrencyPhaseCount;j++)  
			{
				if (m_tLockPhaseData[0].nCurLockPhaseID == m_tRunStageInfo.m_PhaseRunstageInfo[i].m_nConcurrencyPhase[m_tLockPhaseData[0].nCurLockRingIndex])
				{
					m_tLockPhaseData[0].nCurLockStageIndex = i;
					bFlag = true;
					break;
				}
			}

			if (bFlag)
			{
				break;
			}
		}
	}*/

	//获取锁定相位对应的通道
	TChannel atChannelInfo[MAX_CHANNEL_COUNT];
	memset(atChannelInfo, 0, sizeof(atChannelInfo));
	m_pOpenATCParameter->GetChannelTable(atChannelInfo);

	TOverlapTable atOverlapTable[MAX_OVERLAP_COUNT];
	memset(atOverlapTable, 0, sizeof(atOverlapTable));
    m_pOpenATCParameter->GetOverlapTable(atOverlapTable);

	for (i = 0; i < nCount; i++)
	{
		for (j = 0;j < MAX_CHANNEL_COUNT;j++)
		{
			if (atChannelInfo[j].m_byChannelNumber == 0)
			{
				continue;
			}

			if (atChannelInfo[j].m_byChannelControlType == VEH_CHA || atChannelInfo[j].m_byChannelControlType == PED_CHA)
			{
				if (atChannelInfo[j].m_byChannelControlSource == m_tLockPhaseData[i].nCurLockPhaseID)
				{
					m_tLockPhaseData[i].nLockChannelID[nChannelCount[i]] = j + 1;
					nChannelCount[i] += 1;
				}
			}
			else if (atChannelInfo[j].m_byChannelControlType == OVERLAP_CHA || atChannelInfo[j].m_byChannelControlType == OVERLAP_PED_CHA)
			{
				for (int nOverlapIndex = 0;nOverlapIndex < MAX_OVERLAP_COUNT;nOverlapIndex++)
				{
					if (atChannelInfo[j].m_byChannelControlSource == atOverlapTable[nOverlapIndex].m_byOverlapNumber)
					{
						for (int nIncludedPhaseIndex = 0;nIncludedPhaseIndex < MAX_PHASE_COUNT_IN_OVERLAP;nIncludedPhaseIndex++)
						{
							if (atOverlapTable[nOverlapIndex].m_byArrOverlapIncludedPhases[nIncludedPhaseIndex] == m_tLockPhaseData[i].nCurLockPhaseID)
							{
								m_tLockPhaseData[i].nLockChannelID[nChannelCount[i]] = j + 1;
								nChannelCount[i] += 1;
								break;
							}
						}
					}
				}
			}
		}
	}

	for (i = 0; i < nCount; i++)
	{
		m_tLockPhaseData[i].nLockChannelCount = nChannelCount[i];
	}

	//找出锁定相位中最小最大阶段编号，置目标相位和目标阶段编号
	int nMinPhaseIndex = 0;
	int nMaxPhaseIndex = 0;
	for (i = 0; i < nCount; i++)
	{
		if (m_tLockPhaseData[i].nCurLockStageIndex < m_tLockPhaseData[nMinPhaseIndex].nCurLockStageIndex)
		{
			nMinPhaseIndex = i;
		}
		if (m_tLockPhaseData[i].nCurLockStageIndex > m_tLockPhaseData[nMaxPhaseIndex].nCurLockStageIndex)
		{
			nMaxPhaseIndex = i;
		}
	}

	if (nMinPhaseIndex != nMaxPhaseIndex)//比如锁定的是同阶段相位3和相位7,相位3记录的阶段编号是4，相位7跨4,5阶段，但是记录的阶段编号是5，把锁定相位7的阶段编号改为4
	{
		bFlag = false;
	    int nStageIndex = 0;

		for (i = 0; i < m_tRunStageInfo.m_nRunStageCount; i++)      
		{
			for (j = 0;j < m_tRunStageInfo.m_PhaseRunstageInfo[i].m_nConcurrencyPhaseCount;j++)  
			{
				if (m_tLockPhaseData[nMinPhaseIndex].nCurLockPhaseID == m_tRunStageInfo.m_PhaseRunstageInfo[i].m_nConcurrencyPhase[m_tLockPhaseData[nMinPhaseIndex].nCurLockRingIndex] &&
					m_tLockPhaseData[nMaxPhaseIndex].nCurLockPhaseID == m_tRunStageInfo.m_PhaseRunstageInfo[i].m_nConcurrencyPhase[m_tLockPhaseData[nMaxPhaseIndex].nCurLockRingIndex])
				{
					//同阶段相位
					bFlag = true;
					nStageIndex = i;
				}
			}
		}

		if (bFlag)//同阶段相位
		{
			for (i = 0; i < nCount; i++)
			{
				m_tLockPhaseData[i].nCurLockStageIndex = nStageIndex;
				m_tLockPhaseData[i].nTargetStageIndex = nStageIndex;
				m_tLockPhaseData[i].nTargetPhaseIDInCurRing = m_tRunStageInfo.m_PhaseRunstageInfo[m_tLockPhaseData[i].nTargetStageIndex].m_nConcurrencyPhase[m_tLockPhaseData[i].nCurLockRingIndex];
				m_tLockPhaseData[i].bNeedTransFlag = false;
				m_tLockPhaseData[i].bSwitchSuccessFlag = false;
				m_tLockPhaseData[i].bLockPhaseInSameStageFlag = true;
			}

			tValidManualCmd.m_tChannelLockCmd.m_nTargetStageIndex = nStageIndex;//修改目标阶段编号

			GetNextPhaseAfterLockEnd(true, nCount);//锁定不同环的同阶段的相位

			m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "GetLockPhaseData LockPhaseInSameStage StageIndex:%d", nStageIndex);
		}
		else//不同阶段相位
		{
			for (i = 0; i < nCount; i++)
			{
				m_tLockPhaseData[i].nTargetStageIndex = m_tLockPhaseData[nMaxPhaseIndex].nCurLockStageIndex;

				if (i == nMinPhaseIndex)
				{
					m_tLockPhaseData[i].bNeedTransFlag = true;//最小阶段编号的相位置最先开始过渡标志
					m_tLockPhaseData[i].nTargetPhaseIDInCurRing = m_tRunStageInfo.m_PhaseRunstageInfo[m_tLockPhaseData[i].nTargetStageIndex].m_nConcurrencyPhase[m_tLockPhaseData[i].nCurLockRingIndex];
				}
				else
				{
					m_tLockPhaseData[i].bNeedTransFlag = false;
					m_tLockPhaseData[i].nTargetPhaseIDInCurRing = m_tRunStageInfo.m_PhaseRunstageInfo[m_tLockPhaseData[i].nTargetStageIndex].m_nConcurrencyPhase[m_tLockPhaseData[i].nCurLockRingIndex];
				}

				m_tLockPhaseData[i].bSwitchSuccessFlag = false;
				m_tLockPhaseData[i].bLockPhaseInSameStageFlag = false;

				tValidManualCmd.m_tChannelLockCmd.m_nTargetStageIndex = m_tLockPhaseData[nMaxPhaseIndex].nCurLockStageIndex;//修改目标阶段编号
			}

			GetNextPhaseAfterLockEnd(false, nCount);//锁定不同环的不同阶段的相位

			m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "GetLockPhaseData LockPhaseInDifferentStage TargetStageIndex:%d", m_tLockPhaseData[nMaxPhaseIndex].nCurLockStageIndex);
		}
	}
	else//同阶段相位
	{
		for (i = 0; i < nCount; i++)
		{
			m_tLockPhaseData[i].nTargetStageIndex = m_tLockPhaseData[i].nCurLockStageIndex;
			m_tLockPhaseData[i].nTargetPhaseIDInCurRing = m_tRunStageInfo.m_PhaseRunstageInfo[m_tLockPhaseData[i].nTargetStageIndex].m_nConcurrencyPhase[m_tLockPhaseData[i].nCurLockRingIndex];

			tValidManualCmd.m_tChannelLockCmd.m_nTargetStageIndex = m_tLockPhaseData[i].nTargetStageIndex;//修改目标阶段编号

			m_tLockPhaseData[i].bNeedTransFlag = false;
			m_tLockPhaseData[i].bSwitchSuccessFlag = false;
			m_tLockPhaseData[i].bLockPhaseInSameStageFlag = true;
		}

		GetNextPhaseAfterLockEnd(true, nCount);//同阶段相位，包括两种情况，一种是单环锁定一个相位，一种是多环只锁定一个相位

		m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "GetLockPhaseData LockPhaseInSameStage StageIndex:%d", tValidManualCmd.m_tChannelLockCmd.m_nTargetStageIndex);
	}

	//置目标相位对应通道保持绿标志
	for (i = 0; i < MAX_PHASE_COUNT; i++)
	{
		if (m_tLockPhaseData[i].nCurLockPhaseID == 0)
		{
			continue;
		}
			
		for (j = 0; j < m_tLockPhaseData[i].nLockChannelCount;j++)
		{
			//目标相位，不需要过渡，锁定相位也不是同一个阶段的保持绿
			if (!m_tLockPhaseData[i].bNeedTransFlag && !m_tLockPhaseData[i].bLockPhaseInSameStageFlag)
			{
				for (k = 0;k < MAX_CHANNEL_COUNT;k++)
				{
					if (m_tLockPhaseData[i].nLockChannelID[j] == k + 1)
					{
						m_bTargetLockPhaseChannelFlag[k] = true;
					}
				}	
			}
		}
	}
}

/*==================================================================== 
函数名 ：GetNextPhaseAfterLockEnd
功能 ：如果锁定的是同一个阶段，获取锁定相位的下一个阶段对应的相位
       如果锁定的不是同一个阶段，获取需要过渡的锁定相位的下一个相位
算法实现 ： 
参数说明 ： true，锁定相位在同一个阶段
            false，锁定相位不在同一个阶段
			nLockPhaseCount，锁定相位数量
返回值说明：无
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 李永萍          创建函数 
====================================================================*/
void CLogicCtlManual::GetNextPhaseAfterLockEnd(bool bInSameStage, int nLockPhaseCount)
{
	int i = 0, j = 0;
	int nCurRingIndex = 0;
	int nCurPhaseIndex = 0;
	int nLockPhaseIndexInRing = 0;
	
	if (bInSameStage)//锁定相位在同一个阶段
	{
		memset(m_nStageRunTime, 0, sizeof(m_nStageRunTime));

		if (m_tFixTimeCtlInfo.m_nRingCount > 1 && nLockPhaseCount == 1)//多环只锁定一个相位
		{
			bool bDirectTransit = true;//默认锁定相位不跨阶段
			for (i = 0;i < m_tRunStageInfo.m_nRunStageCount;i++)      
			{
				for (j = 0; j < m_tRunStageInfo.m_PhaseRunstageInfo[i].m_nConcurrencyPhaseCount;j++)
				{
					if (m_tRunStageInfo.m_PhaseRunstageInfo[i].m_nConcurrencyPhase[j] == m_tLockPhaseData[0].nCurLockPhaseID)
					{
						if (!m_tRunStageInfo.m_PhaseRunstageInfo[i].m_bDirectTransit[j])
						{
							bDirectTransit = false;//锁定相位跨阶段
						}
					}
				}
			}

			for (i = 0;i < m_tFixTimeCtlInfo.m_nRingCount;i++)
			{
				//假设是前一个阶段向锁定相位所在阶段过渡
				int nCurLockStageIndex = m_tLockPhaseData[0].nCurLockStageIndex - 1;
				if (m_tLockPhaseData[0].nCurLockStageIndex == 0)
				{
					nCurLockStageIndex = m_tRunStageInfo.m_nRunStageCount - 1;
				}

				int nPhaseID = m_tRunStageInfo.m_PhaseRunstageInfo[nCurLockStageIndex].m_nConcurrencyPhase[i];
				GetPhaseIndexByPhaseNumber(nCurRingIndex, nLockPhaseIndexInRing, nPhaseID);

				int nNextStageIndex = m_tLockPhaseData[0].nCurLockStageIndex;//锁定相位所在阶段

				int nSecNextStageIndex = nNextStageIndex + 1;
				if (nSecNextStageIndex >= m_tRunStageInfo.m_nRunStageCount)
				{
					nSecNextStageIndex = 0;
				}
		
				if (m_tRunStageInfo.m_PhaseRunstageInfo[nCurLockStageIndex].m_nConcurrencyPhase[i] == m_tRunStageInfo.m_PhaseRunstageInfo[nNextStageIndex].m_nConcurrencyPhase[i])
				{
					//锁定相位对应其他环的相位跨阶段，当前相位还是锁定相位对应其他环的相位
					int nRingIndex = 0, nPhaseIndex = 0;
					GetPhaseIndexByPhaseNumber(nRingIndex, nPhaseIndex, m_tLockPhaseData[0].nCurLockPhaseID);
					if (i == nRingIndex)
					{
						m_tFixTimeCtlInfo.m_nCurPhaseIndex[i] = nLockPhaseIndexInRing; 	
					}
					else
					{
						m_tFixTimeCtlInfo.m_nCurPhaseIndex[i] = nPhaseIndex + 1; 
						if (m_tFixTimeCtlInfo.m_nCurPhaseIndex[i] == m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_nPhaseCount)
						{
							 m_tFixTimeCtlInfo.m_nCurPhaseIndex[i] = 0;
						}
						nNextStageIndex = m_tLockPhaseData[0].nCurLockStageIndex + 1;
						if (nNextStageIndex >= m_tRunStageInfo.m_nRunStageCount)
						{
							nNextStageIndex = 0;
						}
					}

					m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[m_tFixTimeCtlInfo.m_nCurPhaseIndex[i]].m_chPhaseStage = C_CH_PHASESTAGE_U;
					m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[m_tFixTimeCtlInfo.m_nCurPhaseIndex[i]].m_chPedPhaseStage = C_CH_PHASESTAGE_U;

					GetPhaseGreenTimeAfterLockPhase(i, m_tFixTimeCtlInfo.m_nCurPhaseIndex[i], nNextStageIndex, false);
				}
				else
				{
					if (m_tRunStageInfo.m_PhaseRunstageInfo[nNextStageIndex].m_nConcurrencyPhase[i] == m_tRunStageInfo.m_PhaseRunstageInfo[nSecNextStageIndex].m_nConcurrencyPhase[i] ||
						m_tRunStageInfo.m_PhaseRunstageInfo[nCurLockStageIndex].m_nConcurrencyPhase[m_tLockPhaseData[0].nCurLockRingIndex] == m_tRunStageInfo.m_PhaseRunstageInfo[m_tLockPhaseData[0].nCurLockStageIndex].m_nConcurrencyPhase[m_tLockPhaseData[0].nCurLockRingIndex])
					{
						if (m_tRunStageInfo.m_PhaseRunstageInfo[nNextStageIndex].m_nConcurrencyPhase[i] == m_tRunStageInfo.m_PhaseRunstageInfo[nSecNextStageIndex].m_nConcurrencyPhase[i] &&
							m_tRunStageInfo.m_PhaseRunstageInfo[nNextStageIndex].m_nConcurrencyPhase[i] != m_tLockPhaseData[0].nCurLockPhaseID && bDirectTransit)
						{
							nNextStageIndex = nSecNextStageIndex;
						}
			
						m_tFixTimeCtlInfo.m_nCurPhaseIndex[i] = nLockPhaseIndexInRing + 1; 
						if (m_tFixTimeCtlInfo.m_nCurPhaseIndex[i] >= m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_nPhaseCount)
						{
							m_tFixTimeCtlInfo.m_nCurPhaseIndex[i] = 0;
							nNextStageIndex = 1;
						}
					}
					else
					{
						//锁定相位对应其他环的相位不跨阶段，当前相位是锁定相位对应其他环的相位后面的相位
						int nRingIndex = 0, nPhaseIndex = 0;
						GetPhaseIndexByPhaseNumber(nRingIndex, nPhaseIndex, m_tLockPhaseData[0].nCurLockPhaseID);
						m_tFixTimeCtlInfo.m_nCurPhaseIndex[i] = nPhaseIndex + 1; 
						if (m_tFixTimeCtlInfo.m_nCurPhaseIndex[i] >= m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_nPhaseCount)
						{
							m_tFixTimeCtlInfo.m_nCurPhaseIndex[i] = 0;
						}
						nNextStageIndex = m_tLockPhaseData[0].nCurLockStageIndex + 1;
						if (nNextStageIndex >= m_tRunStageInfo.m_nRunStageCount)
						{
							nNextStageIndex = 0;
						}
					}

					GetPhaseGreenTimeAfterLockPhase(i, m_tFixTimeCtlInfo.m_nCurPhaseIndex[i], nNextStageIndex, true);
				}

				m_nStageTimeForPhasePass[i] = m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[m_tFixTimeCtlInfo.m_nCurPhaseIndex[i]].m_wPhaseGreenTime + 
												m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[m_tFixTimeCtlInfo.m_nCurPhaseIndex[i]].m_tPhaseParam.m_byGreenFlash +
												m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[m_tFixTimeCtlInfo.m_nCurPhaseIndex[i]].m_tPhaseParam.m_byPhaseYellowChange +
												m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[m_tFixTimeCtlInfo.m_nCurPhaseIndex[i]].m_tPhaseParam.m_byPhaseRedClear;

				m_tFixTimeCtlInfo.m_nCurStageIndex = m_tLockPhaseData[0].nCurLockStageIndex;//转到锁定相位所在阶段
				if (m_tRunStageInfo.m_PhaseRunstageInfo[nNextStageIndex].m_nConcurrencyPhase[i] == m_tRunStageInfo.m_PhaseRunstageInfo[nSecNextStageIndex].m_nConcurrencyPhase[i] &&
					m_tRunStageInfo.m_PhaseRunstageInfo[nNextStageIndex].m_nConcurrencyPhase[i] != m_tLockPhaseData[0].nCurLockPhaseID && bDirectTransit)
				{
					m_tFixTimeCtlInfo.m_nCurStageIndex = m_tLockPhaseData[0].nCurLockStageIndex + 1;
					if (m_tFixTimeCtlInfo.m_nCurStageIndex >= m_tRunStageInfo.m_nRunStageCount)
					{
						m_tFixTimeCtlInfo.m_nCurStageIndex = 0;
					}
				}
				m_nNextStageIndex = m_tFixTimeCtlInfo.m_nCurStageIndex + 1;
				if (m_nNextStageIndex >= m_tRunStageInfo.m_nRunStageCount)
				{
					m_nNextStageIndex = 0;
				}

				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "SameStage SwitchLockChannelPhaseToNext RingIndex:%d SwitchToPhaseIndex:%d SwitchToStageIndex:%d PhaseTime:%d", i, m_tFixTimeCtlInfo.m_nCurPhaseIndex[i], m_tFixTimeCtlInfo.m_nCurStageIndex, m_nStageTimeForPhasePass[i]);
			}

			ReSetNeglectPhasetBackUpTime();
		}
		else
		{
			for (i = 0;i < m_tFixTimeCtlInfo.m_nRingCount;i++)
			{
				for (j = 0; j < MAX_PHASE_COUNT; j++)
				{
					if (m_tLockPhaseData[j].nCurLockPhaseID == 0)
					{
						continue;
					}

					if (m_tLockPhaseData[j].bLockPhaseInSameStageFlag && i == m_tLockPhaseData[j].nCurLockRingIndex)
					{
						GetPhaseIndexByPhaseNumber(nCurRingIndex, nLockPhaseIndexInRing, m_tLockPhaseData[j].nCurLockPhaseID);

						int nNextStageIndex = 0;
						nNextStageIndex = m_tLockPhaseData[j].nCurLockStageIndex + 1;//锁定相位的下一个阶段
						if (nNextStageIndex >= m_tRunStageInfo.m_nRunStageCount)
						{
							nNextStageIndex = 0;
						}

						if (m_tRunStageInfo.m_PhaseRunstageInfo[m_tLockPhaseData[j].nCurLockStageIndex].m_nConcurrencyPhase[i] == m_tRunStageInfo.m_PhaseRunstageInfo[nNextStageIndex].m_nConcurrencyPhase[i])
						{
							//锁定相位跨阶段，当前相位还是锁定相位
							m_tFixTimeCtlInfo.m_nCurPhaseIndex[i] = nLockPhaseIndexInRing; 	

							m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[m_tFixTimeCtlInfo.m_nCurPhaseIndex[i]].m_chPhaseStage = C_CH_PHASESTAGE_U;
							m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[m_tFixTimeCtlInfo.m_nCurPhaseIndex[i]].m_chPedPhaseStage = C_CH_PHASESTAGE_U;

							GetPhaseGreenTimeAfterLockPhase(i, m_tFixTimeCtlInfo.m_nCurPhaseIndex[i], nNextStageIndex, false);
						}
						else
						{
							//锁定相位不跨阶段，当前相位是锁定相位后面的相位
							m_tFixTimeCtlInfo.m_nCurPhaseIndex[i] = nLockPhaseIndexInRing + 1; 
							if (m_tFixTimeCtlInfo.m_nCurPhaseIndex[i] >= m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_nPhaseCount)
							{
								m_tFixTimeCtlInfo.m_nCurPhaseIndex[i] = 0;
							}

							GetPhaseGreenTimeAfterLockPhase(i, m_tFixTimeCtlInfo.m_nCurPhaseIndex[i], nNextStageIndex, true);
						}

						m_nStageTimeForPhasePass[i] = m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[m_tFixTimeCtlInfo.m_nCurPhaseIndex[i]].m_wPhaseGreenTime + 
													  m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[m_tFixTimeCtlInfo.m_nCurPhaseIndex[i]].m_tPhaseParam.m_byGreenFlash +
													  m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[m_tFixTimeCtlInfo.m_nCurPhaseIndex[i]].m_tPhaseParam.m_byPhaseYellowChange +
													  m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[m_tFixTimeCtlInfo.m_nCurPhaseIndex[i]].m_tPhaseParam.m_byPhaseRedClear;

						m_tFixTimeCtlInfo.m_nCurStageIndex = m_tLockPhaseData[i].nTargetStageIndex + 1;//转到目标阶段的下一个阶段
						if (m_tFixTimeCtlInfo.m_nCurStageIndex >= m_tRunStageInfo.m_nRunStageCount)
						{
							m_tFixTimeCtlInfo.m_nCurStageIndex = 0;
						}
						m_nNextStageIndex = m_tFixTimeCtlInfo.m_nCurStageIndex + 1;
						if (m_nNextStageIndex >= m_tRunStageInfo.m_nRunStageCount)
						{
							m_nNextStageIndex = 0;
						}

						m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "SameStage SwitchLockChannelPhaseToNext RingIndex:%d SwitchToPhaseIndex:%d SwitchToStageIndex:%d PhaseTime:%d", i, m_tFixTimeCtlInfo.m_nCurPhaseIndex[i], m_tFixTimeCtlInfo.m_nCurStageIndex, m_nStageTimeForPhasePass[i]);
						break;
					}
				}
			}

			ReSetNeglectPhasetBackUpTime();
		}
	}
	else//锁定相位不在同一个阶段
	{
		for (i = 0; i < MAX_PHASE_COUNT; i++)
		{
			if (m_tLockPhaseData[i].nCurLockPhaseID == 0)
			{
				continue;
			}

			if (m_tLockPhaseData[i].bNeedTransFlag && !m_tLockPhaseData[i].bSwitchSuccessFlag)
			{
				int nTempRingIndex = 0;
				GetPhaseIndexByPhaseNumber(nTempRingIndex, nLockPhaseIndexInRing, m_tLockPhaseData[i].nCurLockPhaseID);

				//从锁定相位后面的相位开始过渡
				m_tFixTimeCtlInfo.m_nCurPhaseIndex[m_tLockPhaseData[i].nCurLockRingIndex] = nLockPhaseIndexInRing + 1; 
				if (m_tFixTimeCtlInfo.m_nCurPhaseIndex[m_tLockPhaseData[i].nCurLockRingIndex] >= m_tFixTimeCtlInfo.m_atPhaseSeq[m_tLockPhaseData[i].nCurLockRingIndex].m_nPhaseCount)
				{
					m_tFixTimeCtlInfo.m_nCurPhaseIndex[m_tLockPhaseData[i].nCurLockRingIndex] = 0;
				}
				m_tFixTimeCtlInfo.m_atPhaseSeq[m_tLockPhaseData[i].nCurLockRingIndex].m_atPhaseInfo[m_tFixTimeCtlInfo.m_nCurPhaseIndex[m_tLockPhaseData[i].nCurLockRingIndex]].m_chPhaseStage = C_CH_PHASESTAGE_U;   
				m_tFixTimeCtlInfo.m_atPhaseSeq[m_tLockPhaseData[i].nCurLockRingIndex].m_atPhaseInfo[m_tFixTimeCtlInfo.m_nCurPhaseIndex[m_tLockPhaseData[i].nCurLockRingIndex]].m_chPedPhaseStage = C_CH_PHASESTAGE_U;  
				m_tFixTimeCtlInfo.m_atPhaseSeq[m_tLockPhaseData[i].nCurLockRingIndex].m_atPhaseInfo[m_tFixTimeCtlInfo.m_nCurPhaseIndex[m_tLockPhaseData[i].nCurLockRingIndex]].m_wPhaseStageRunTime = 0;
				m_tFixTimeCtlInfo.m_atPhaseSeq[m_tLockPhaseData[i].nCurLockRingIndex].m_atPhaseInfo[m_tFixTimeCtlInfo.m_nCurPhaseIndex[m_tLockPhaseData[i].nCurLockRingIndex]].m_wPedPhaseStageRunTime = 0;
				break;
			}
		}
	}
}

/*==================================================================== 
函数名 ：IsTransChannel
功能 ：是否是过渡通道
算法实现 ： 
参数说明 ： byChannelType, 通道类型，0表示方向通道，1表示锁定通道, 
            nChannelIndex, 通道编号
返回值说明：true，可过渡
            false，不可过渡
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 李永萍          创建函数 
====================================================================*/
bool CLogicCtlManual::IsTransChannel(BYTE byChannelType, int nChannelIndex)
{
	if (byChannelType == CHANNEL_TYPE_LOCK  && m_bTargetLockPhaseChannelFlag[nChannelIndex])//锁定通道是目标通道则继续保持绿
	{
		return false;
	}

	return true;
}

/*==================================================================== 
函数名 ：SwitchLockChannelPhaseToNext
功能 ：SwitchLockChannelPhaseToNext切换锁定相位到下一个相位
算法实现 ： 
参数说明 ： 无
返回值说明：无
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 李永萍          创建函数 
====================================================================*/
void CLogicCtlManual::SwitchLockChannelPhaseToNext()
{
	bool bFlag = false;
	int  nCurRingIndex = 0;
	int  nCurPhaseIndex = 0;
	int  i = 0, j = 0, k = 0;
	for (i = 0; i < MAX_PHASE_COUNT; i++)
	{
		if (m_tLockPhaseData[i].nCurLockPhaseID == 0)
		{
			continue;
		}

		for (j = 0; j < m_tLockPhaseData[i].nLockChannelCount;j++)
		{
			if (m_tLockPhaseData[i].bNeedTransFlag && !m_tLockPhaseData[i].bSwitchSuccessFlag)
			{
				bFlag = true;
				nCurRingIndex = m_tLockPhaseData[i].nCurLockRingIndex;
				nCurPhaseIndex = i;
				break;
			}
		}
	}

	if (!bFlag)
	{
		return;
	}
	else
	{
		//非目标相位的通道全部锁定过渡结束才能进行环内相位切换
		for (i = 0; i < m_tLockPhaseData[nCurPhaseIndex].nLockChannelCount; i++)
		{
			for (j = 0;j < MAX_CHANNEL_COUNT;j++)
			{
				if (m_tLockPhaseData[nCurPhaseIndex].nLockChannelID[i] == j + 1)
				{
					if (!m_bNonTargetLockPhaseEndFlag[j] && !IsTargetPhaseChannel(j))
					{
						return;
					}
				}
			}
		}
	}

	if (!m_tLockPhaseData[nCurPhaseIndex].bSwitchLockChannelToNextPhase)
	{
		m_tLockPhaseData[nCurPhaseIndex].bSwitchLockChannelToNextPhase = true;

		for (i = 0; i < m_tRunStageInfo.m_nRunStageCount; i++)      
		{
			for (j = 0;j < m_tRunStageInfo.m_PhaseRunstageInfo[i].m_nConcurrencyPhaseCount;j++)    
			{
				if (m_tFixTimeCtlInfo.m_atPhaseSeq[nCurRingIndex].m_atPhaseInfo[m_tFixTimeCtlInfo.m_nCurPhaseIndex[nCurRingIndex]].m_tPhaseParam.m_byPhaseNumber == 
					m_tRunStageInfo.m_PhaseRunstageInfo[i].m_nConcurrencyPhase[j])
				{
					m_tFixTimeCtlInfo.m_nCurStageIndex = i;//相位跨阶段时，用最后一个阶段编号
				}
			}
		}
		m_nNextStageIndex = m_tFixTimeCtlInfo.m_nCurStageIndex + 1;
		if (m_nNextStageIndex >= m_tRunStageInfo.m_nRunStageCount)
		{
			m_nNextStageIndex = 0;
		}

		m_nManualCurStatus = MANUAL_STAGE_TRANS;

		OnePhaseRun(nCurRingIndex);
	    OnePedPhaseRun(nCurRingIndex); 
	    SetOverLapPulse(false, 0);

		m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "1111111111 SwitchLockChannelPhaseToNext RingIndex:%d SwitchToPhaseIndex:%d StageIndex:%d", nCurRingIndex, m_tFixTimeCtlInfo.m_nCurPhaseIndex[nCurRingIndex], m_tFixTimeCtlInfo.m_nCurStageIndex);
	}
	else
	{
		//已经运行到目标相位
		if (m_tFixTimeCtlInfo.m_atPhaseSeq[nCurRingIndex].m_atPhaseInfo[m_tFixTimeCtlInfo.m_nCurPhaseIndex[nCurRingIndex]].m_tPhaseParam.m_byPhaseNumber ==
			m_tLockPhaseData[nCurPhaseIndex].nTargetPhaseIDInCurRing)
		{
			memset(m_nStageRunTime, 0, sizeof(m_nStageRunTime));

			for (i = 0;i < m_tFixTimeCtlInfo.m_nRingCount;i++)
			{
				int nTempTargetRingIndex = 0;
				int nTargetLockPhaseIndexInRing = 0;
				int nTargetPhaseID = m_tRunStageInfo.m_PhaseRunstageInfo[m_tLockPhaseData[i].nTargetStageIndex].m_nConcurrencyPhase[i];
				GetPhaseIndexByPhaseNumber(nTempTargetRingIndex, nTargetLockPhaseIndexInRing, nTargetPhaseID);

				//假设是从目标阶段的前一个阶段向目标阶段过渡
				int nCurStageIndex = m_tLockPhaseData[i].nTargetStageIndex - 1;
				if (m_tLockPhaseData[i].nTargetStageIndex == 0)
				{
					nCurStageIndex = m_tRunStageInfo.m_nRunStageCount - 1;
				}

				int nNextStageIndex = m_tLockPhaseData[i].nTargetStageIndex;
				if (nNextStageIndex >= m_tRunStageInfo.m_nRunStageCount)
				{
					nNextStageIndex = 0;
				}

				m_tFixTimeCtlInfo.m_nCurPhaseIndex[i] = nTargetLockPhaseIndexInRing; 

				if (m_tRunStageInfo.m_PhaseRunstageInfo[nCurStageIndex].m_nConcurrencyPhase[i] == m_tRunStageInfo.m_PhaseRunstageInfo[nNextStageIndex].m_nConcurrencyPhase[i])
				{
					m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[m_tFixTimeCtlInfo.m_nCurPhaseIndex[i]].m_chPhaseStage = C_CH_PHASESTAGE_U;
					m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[m_tFixTimeCtlInfo.m_nCurPhaseIndex[i]].m_chPedPhaseStage = C_CH_PHASESTAGE_U;

					GetPhaseGreenTimeAfterLockPhase(i, m_tFixTimeCtlInfo.m_nCurPhaseIndex[i], nNextStageIndex, false);
				}
				else
				{
					GetPhaseGreenTimeAfterLockPhase(i, m_tFixTimeCtlInfo.m_nCurPhaseIndex[i], nNextStageIndex, true);
				}

				m_nStageTimeForPhasePass[i] = m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[m_tFixTimeCtlInfo.m_nCurPhaseIndex[i]].m_wPhaseGreenTime + 
												m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[m_tFixTimeCtlInfo.m_nCurPhaseIndex[i]].m_tPhaseParam.m_byGreenFlash +
												m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[m_tFixTimeCtlInfo.m_nCurPhaseIndex[i]].m_tPhaseParam.m_byPhaseYellowChange +
												m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[m_tFixTimeCtlInfo.m_nCurPhaseIndex[i]].m_tPhaseParam.m_byPhaseRedClear;


				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "3333333333 SwitchLockChannelPhaseToNext RingIndex:%d SwitchToPhaseIndex:%d PhaseTime:%d", i, m_tFixTimeCtlInfo.m_nCurPhaseIndex[i], m_nStageTimeForPhasePass[i]);
			}

			ReSetNeglectPhasetBackUpTime();

			m_tFixTimeCtlInfo.m_nCurStageIndex = m_tLockPhaseData[nCurPhaseIndex].nTargetStageIndex;//转到目标环的锁定相位的所在阶段
			m_nNextStageIndex = m_tFixTimeCtlInfo.m_nCurStageIndex + 1;
			if (m_nNextStageIndex >= m_tRunStageInfo.m_nRunStageCount)
			{
				m_nNextStageIndex = 0;
			}

			m_tLockPhaseData[nCurPhaseIndex].bSwitchSuccessFlag = true;
			m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "SwitchLockChannelPhaseToNext SwitchToStageIndex:%d Succeed!", m_tFixTimeCtlInfo.m_nCurStageIndex);
		}
		else
		{
			TManualCmd  tValidManualCmd;
			memset(&tValidManualCmd,0,sizeof(tValidManualCmd));
			m_pOpenATCRunStatus->GetValidManualCmd(tValidManualCmd); 

			OnePhaseRun(nCurRingIndex);
	        OnePedPhaseRun(nCurRingIndex); 
			SetOverLapPulse(false, 0);

			//非目标相位在环内相位切换过程中，又有了新的相位锁定指令,如果当前相位继续锁定，当前相位灯色处于绿色，则继续绿，非绿色则过渡到可结束，不需要再切到下一个相位了
			if (memcmp(tValidManualCmd.m_tChannelLockCmd.m_tNextChannelLockCtrlCmd.m_nChannelLockStatus, m_tOldValidManualCmd.m_tChannelLockCmd.m_tNextChannelLockCtrlCmd.m_nChannelLockStatus, MAX_CHANNEL_COUNT) != 0)
			{
				PTRingCtlInfo ptRingRunInfo = &(m_tFixTimeCtlInfo.m_atPhaseSeq[m_tLockPhaseData[nCurRingIndex].nCurLockRingIndex]);
				int nIndex = m_tFixTimeCtlInfo.m_nCurPhaseIndex[m_tLockPhaseData[nCurRingIndex].nCurLockRingIndex];
				PTPhaseCtlInfo pPhaseInfo = (PTPhaseCtlInfo)&(ptRingRunInfo->m_atPhaseInfo[nIndex]);
				if (pPhaseInfo->m_chPhaseStage == C_CH_PHASESTAGE_G)
				{
					//非绿色过渡到可结束的时候，需要置最小绿
					//ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPhaseStageRunTime = ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_wPhaseMinimumGreen;
				}
				if (pPhaseInfo->m_chPedPhaseStage == C_CH_PHASESTAGE_G)
				{
					//非绿色过渡到可结束的时候，需要置最小绿
					//ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPedPhaseStageRunTime = ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_wPhaseMinimumGreen;
				}

				bool bFlag = false;
				for (int i = 0; i < tValidManualCmd.m_tPhaseLockPara.m_nPhaseLockCount;i++)
				{
					if (tValidManualCmd.m_tPhaseLockPara.m_nPhaseLockType[i] != LOCK_TYPE_CANCEL)
					{
						bFlag = true;//只要有一个相位的锁定类型不是恢复，就认为是新的锁定指令，非目标相位不需要再切到下一个相位
					}
				}

				if (bFlag)
				{
					return;
				}
			}
		
			//当前环相位切换结束
			if (m_tFixTimeCtlInfo.m_atPhaseSeq[nCurRingIndex].m_atPhaseInfo[m_tFixTimeCtlInfo.m_nCurPhaseIndex[nCurRingIndex]].m_chPhaseStage == C_CH_PHASESTAGE_F &&
				m_tFixTimeCtlInfo.m_atPhaseSeq[nCurRingIndex].m_atPhaseInfo[m_tFixTimeCtlInfo.m_nCurPhaseIndex[nCurRingIndex]].m_tPhaseParam.m_byPhaseNumber !=
			    m_tLockPhaseData[nCurPhaseIndex].nTargetPhaseIDInCurRing)
			{
				//环内转相位
				m_tFixTimeCtlInfo.m_nCurPhaseIndex[nCurRingIndex]++; 
				if (m_tFixTimeCtlInfo.m_nCurPhaseIndex[nCurRingIndex] >= m_tFixTimeCtlInfo.m_atPhaseSeq[nCurRingIndex].m_nPhaseCount)
				{
					m_tFixTimeCtlInfo.m_nCurPhaseIndex[nCurRingIndex] = 0;
				}
				m_tFixTimeCtlInfo.m_atPhaseSeq[nCurRingIndex].m_atPhaseInfo[m_tFixTimeCtlInfo.m_nCurPhaseIndex[nCurRingIndex]].m_chPhaseStage = C_CH_PHASESTAGE_U;   
				m_tFixTimeCtlInfo.m_atPhaseSeq[nCurRingIndex].m_atPhaseInfo[m_tFixTimeCtlInfo.m_nCurPhaseIndex[nCurRingIndex]].m_chPedPhaseStage = C_CH_PHASESTAGE_U;  
				m_tFixTimeCtlInfo.m_atPhaseSeq[nCurRingIndex].m_atPhaseInfo[m_tFixTimeCtlInfo.m_nCurPhaseIndex[nCurRingIndex]].m_wPhaseStageRunTime = 0;
				m_tFixTimeCtlInfo.m_atPhaseSeq[nCurRingIndex].m_atPhaseInfo[m_tFixTimeCtlInfo.m_nCurPhaseIndex[nCurRingIndex]].m_wPedPhaseStageRunTime = 0;

				for (i = 0; i < m_tRunStageInfo.m_nRunStageCount; i++)      
				{
					for (j = 0;j < m_tRunStageInfo.m_PhaseRunstageInfo[i].m_nConcurrencyPhaseCount;j++)    
					{
						if (m_tFixTimeCtlInfo.m_atPhaseSeq[nCurRingIndex].m_atPhaseInfo[m_tFixTimeCtlInfo.m_nCurPhaseIndex[nCurRingIndex]].m_tPhaseParam.m_byPhaseNumber == 
							m_tRunStageInfo.m_PhaseRunstageInfo[i].m_nConcurrencyPhase[j])
						{
							m_tFixTimeCtlInfo.m_nCurStageIndex = i;//相位跨阶段时，用最后一个阶段编号
						}
					}
				}
				m_nNextStageIndex = m_tFixTimeCtlInfo.m_nCurStageIndex + 1;
				if (m_nNextStageIndex >= m_tRunStageInfo.m_nRunStageCount)
				{
					m_nNextStageIndex = 0;
				}

				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "2222222222 SwitchLockChannelPhaseToNext RingIndex:%d SwitchToPhaseIndex:%d StageIndex:%d", nCurRingIndex, m_tFixTimeCtlInfo.m_nCurPhaseIndex[nCurRingIndex], m_tFixTimeCtlInfo.m_nCurStageIndex);
			}
		}
	}
}

/*==================================================================== 
函数名 ：IsTargetPhaseChannel
功能 ：判断是否是目标相位通道
算法实现 ： 
参数说明 ： nChannelIndex，通道编号
返回值说明：目标相位通道标志
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 李永萍          创建函数 
====================================================================*/
bool CLogicCtlManual::IsTargetPhaseChannel(int nChannelIndex)
{
	bool bFlag = false;
	int  nCurLockPhaseID = 0;
	int  i = 0, j = 0;
	for (i = 0; i < MAX_PHASE_COUNT; i++)
	{
		if (m_tLockPhaseData[i].nCurLockPhaseID == 0)
		{
			continue;
		}

		for (j = 0; j < m_tLockPhaseData[i].nLockChannelCount;j++)
		{
			if (!m_tLockPhaseData[i].bNeedTransFlag)
			{
				bFlag = true;
				nCurLockPhaseID = m_tLockPhaseData[i].nCurLockPhaseID;
				break;
			}
		}
	}

	if (bFlag)
	{
		if (m_atChannelInfo[nChannelIndex].m_byChannelControlType == VEH_CHA || m_atChannelInfo[nChannelIndex].m_byChannelControlType == PED_CHA)
		{
			if (m_atChannelInfo[nChannelIndex].m_byChannelControlSource == nCurLockPhaseID)
			{
				return true;
			}
		}
		else if (m_atChannelInfo[nChannelIndex].m_byChannelControlType == OVERLAP_CHA || m_atChannelInfo[nChannelIndex].m_byChannelControlType == OVERLAP_PED_CHA)
		{
			TOverlapTable atOverlapTable[MAX_OVERLAP_COUNT];
            m_pOpenATCParameter->GetOverlapTable(atOverlapTable);

			for (i = 0;i < MAX_OVERLAP_COUNT;i++)
			{
				if (m_atChannelInfo[nChannelIndex].m_byChannelControlSource == atOverlapTable[i].m_byOverlapNumber)
				{
					for (j = 0;j < MAX_PHASE_COUNT_IN_OVERLAP;j++)
					{
						if (atOverlapTable[i].m_byArrOverlapIncludedPhases[j] == nCurLockPhaseID)
						{
							return true;
						}
					}
				}
			}
		}
	}

	return false;
}

/*==================================================================== 
函数名 ：IsOverlapNeedKeepGreen
功能 ：跟随相位是否需要继续保持绿色
算法实现 ： 
参数说明 ： byOverlapIndex，跟随相位编号 nOverlapPhaseType，跟随相位类型
返回值说明：跟随相位是否需要继续保持绿色标志
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 李永萍          创建函数 
====================================================================*/
bool CLogicCtlManual::IsOverlapNeedKeepGreen(BYTE byOverlapIndex, int nOverlapPhaseType)
{
	BYTE byMainPhaseNum = 0;
	int  i = 0, j = 0;
	for (i = 0;i < MAX_PHASE_COUNT_IN_OVERLAP;i++)
	{
		byMainPhaseNum = m_tFixTimeCtlInfo.m_atOverlapInfo[byOverlapIndex].m_tOverlapParam.m_byArrOverlapIncludedPhases[i];
		if (byMainPhaseNum > 0)
		{
			if (m_tNewPhaseLockPara.m_nPhaseLockCount == 0)//没有下发新的相位锁定指令
			{
				for (j = 0; j < MAX_PHASE_COUNT; j++)
				{
					if (m_tLockPhaseData[j].nCurLockPhaseID == 0)
					{
						continue;
					}

					if (!m_tLockPhaseData[j].bNeedTransFlag && byMainPhaseNum == m_tLockPhaseData[j].nTargetPhaseIDInCurRing)
					{
						return true;//跟随相位的母相位中有目标相位，并且目标相位还在绿则继续保持绿
					}
				}
			}
			else//在相位锁定时，下发了新的相位锁定指令，并且跟随相位的母相位包含新的锁定相位，则跟随相位还在绿则继续保持绿
			{
				for (j = 0; j < m_tNewPhaseLockPara.m_nPhaseLockCount;j++)
				{
					if (m_tNewPhaseLockPara.m_nPhaseLockType[j] != LOCK_TYPE_CANCEL && byMainPhaseNum == m_tNewPhaseLockPara.m_nPhaseLockID[j])
					{
						if (nOverlapPhaseType == OVERLAP_CHA && m_tFixTimeCtlInfo.m_atOverlapInfo[byOverlapIndex].m_chOverlapStage == C_CH_PHASESTAGE_G)
						{
							return true;
						}
						else if (nOverlapPhaseType == OVERLAP_PED_CHA && m_tFixTimeCtlInfo.m_atOverlapInfo[byOverlapIndex].m_chPedOverlapStage == C_CH_PHASESTAGE_G)
						{
							return true;
						}
					}
				}
			}
		}
	}

	return false;
}

/*==================================================================== 
函数名 ：IsHasTargetPhaseIncludedPhase
功能 ：跟随相位的母相位中是否包含旧的锁定相位
算法实现 ： 
参数说明 ： byOverlapIndex，跟随相位编号 nOverlapPhaseType，跟随相位类型
返回值说明：跟随相位的母相位中是否包含旧的锁定相位标志
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 李永萍          创建函数 
====================================================================*/
bool CLogicCtlManual::IsHasTargetPhaseIncludedOldLockPhase(BYTE byOverlapIndex, int nOverlapPhaseType, int & nLockPhaseIndex)
{
	bool bFlag = false;
	BYTE byMainPhaseNum = 0;
	int  i = 0, j = 0;
	for (i = 0;i < MAX_PHASE_COUNT_IN_OVERLAP;i++)
	{
		byMainPhaseNum = m_tFixTimeCtlInfo.m_atOverlapInfo[byOverlapIndex].m_tOverlapParam.m_byArrOverlapIncludedPhases[i];
		if (byMainPhaseNum > 0)
		{
			for (j = 0; j < MAX_PHASE_COUNT; j++)
			{
				if (m_tLockPhaseData[j].nCurLockPhaseID == 0)
				{
					continue;
				}

				if (!m_tLockPhaseData[j].bNeedTransFlag && byMainPhaseNum == m_tLockPhaseData[j].nTargetPhaseIDInCurRing)
				{
					nLockPhaseIndex = j;
					bFlag = true;//在相位锁定时，跟随相位的母相位不包含新的锁定相位，但是包含旧的锁定目标相位
					break;
				}
			}
		}

		if (bFlag)
		{
			break;
		}
	}

	if (bFlag)
	{
		return true;
	}
	else
	{
	   return false;
	}
}

/*==================================================================== 
函数名 ：GetOldLockPhaseStatus
功能 ：获取旧的锁定相位的状态
算法实现 ： 
参数说明 ： byOverlapIndex，跟随相位编号 nOverlapPhaseType，跟随相位类型
返回值说明：旧的锁定相位的状态
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 李永萍          创建函数 
====================================================================*/
void CLogicCtlManual::GetOldLockPhaseStatus(BYTE byOverlapIndex, int nOverlapPhaseType, char & chStatus)
{
	int i = 0;
	int nLockPhaseIndex = 0;
	if (IsHasTargetPhaseIncludedOldLockPhase(byOverlapIndex, nOverlapPhaseType, nLockPhaseIndex))
	{
		if (m_tLockPhaseData[nLockPhaseIndex].nLockChannelCount > 0)
		{
			BYTE byChannelID = m_tLockPhaseData[nLockPhaseIndex].nLockChannelID[0];
			for (i = 0;i < m_nChannelCount;i++)
			{
				if (m_atChannelInfo[i].m_byChannelNumber == byChannelID && m_atChannelInfo[i].m_byChannelControlType == nOverlapPhaseType)
				{
					chStatus = m_chChannelStage[i];
					return;
				}
			}
		}
	}
}


/*==================================================================== 
函数名 ：LockChannelTransClr
功能 ：在通道锁定时，按下面板手动按钮，此时需要把通道灯色过渡完
算法实现 ： 
参数说明 ：无
返回值说明：通道过渡成功标志
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 李永萍          创建函数 
====================================================================*/ 
bool CLogicCtlManual::LockChannelTransClr()
{
	int  i = 0;
	for (i = 0;i < m_nChannelCount;i++)
	{
		m_nChannelLockGreenTime[i] = m_tOldValidManualCmd.m_tChannelLockCmd.m_tNextChannelLockCtrlCmd.m_nMinGreen;	
		m_bChannelKeepGreenFlag[i] = false;
	}

	bool bSelRun = TransitChannelClr(CHANNEL_TYPE_LOCK, false);
	if (bSelRun)
	{
		for (i = 0; i < MAX_PHASE_COUNT; i++)
		{
			if (m_tLockPhaseData[i].nCurLockPhaseID == 0)
			{
				continue;
			}
			//1.目标相位过渡结束，非目标相位所在环的当前相位还没有切换结束
			//2.目标相位因为要继续锁定保持绿，非目标相位所在环的当前相位还没有切换结束
			if (m_tLockPhaseData[i].bNeedTransFlag && !m_tLockPhaseData[i].bSwitchSuccessFlag)
			{
				SwitchLockChannelPhaseToNext();
				return false;
			}
		}

		//memset(m_tLockPhaseData, 0, sizeof(m_tLockPhaseData));
		memset(m_bTargetLockPhaseChannelFlag, 0, sizeof(m_bTargetLockPhaseChannelFlag));
	    memset(m_bNonTargetLockPhaseEndFlag, 0, sizeof(m_bNonTargetLockPhaseEndFlag));
		memset(&m_tNewPhaseLockPara, 0, sizeof(m_tNewPhaseLockPara));
		m_bIsDirectionChannelClrChg = false;
		return true;
	}
	else
	{
		SwitchLockChannelPhaseToNext();
	}

	return false;
}

/*==================================================================== 
函数名 ：ProcessLockChannelToPanel
功能 ：通道锁定切换到面板
算法实现 ： 
参数说明 ：tValidManualCmd：有效的控制指令
返回值说明：通道锁定切换到面板完成返回值
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 李永萍          创建函数 
====================================================================*/ 
int CLogicCtlManual::ProcessLockChannelToPanel(TManualCmd  tValidManualCmd)
{
	if (m_tOldValidManualCmd.m_nSubCtlMode == CTL_MANUAL_SUBMODE_SYSTEM_CHANNEL_LOCK)//进入通道锁定以后，先按下面板手动按钮,又按下步进或者方向或者自主
	{
		if (tValidManualCmd.m_nSubCtlMode == CTL_MANUAL_SUBMODE_PANEL_FITSTMANUAL || 
			tValidManualCmd.m_nSubCtlMode == CTL_MANUAL_SUBMODE_PANEL_STEPFOWARD || 
			tValidManualCmd.m_bDirectionCmd ||
			tValidManualCmd.m_bPatternInterruptCmd)
		{
			if (LockChannelTransClr())
			{
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "LockChannelTransClr End");

				//通道锁定或者锁定在同一个阶段的相位，过渡结束可直接切到方向
				if (tValidManualCmd.m_bDirectionCmd && (!m_tOldValidManualCmd.m_bPhaseToChannelLock || m_tLockPhaseData[0].bLockPhaseInSameStageFlag))
				{
					tValidManualCmd.m_nSubCtlMode = CTL_MANUAL_SUBMODE_PANEL_DIRECTION;
					tValidManualCmd.m_tDirectionCmd.m_nTargetStageIndex = m_tFixTimeCtlInfo.m_nCurStageIndex;//记录下切方向之前的下一个阶段

					tValidManualCmd.m_bDirectionCmd = false;//切到方向控制子模式下，清除当前方向指令标志
		   
					//初始化方向参数，开始切方向
					InitParamBeforeDirection(tValidManualCmd.m_tDirectionCmd.m_nNextDirectionIndex);
					ChangeChannelClr();
					TAscManualPanel tAscManualPanel;
					memset(&tAscManualPanel, 0, sizeof(TAscManualPanel)); 
					m_pOpenATCParameter->GetManualPanelInfo(tAscManualPanel);
					for (int i = 0;i < MAX_CHANNEL_COUNT;i++)
					{
						m_nDirectionGreenTime[i] = tAscManualPanel.m_wDuration;
						m_bChannelKeepGreenFlag[i] = false;
					}
					m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "Switch Stage Succeed, Remember NextStageIndex:%d And InitParamBeforeDirection, Calculate DirectionIndex:%d GreenTime:%d", m_tFixTimeCtlInfo.m_nCurStageIndex, tValidManualCmd.m_tDirectionCmd.m_nNextDirectionIndex, tAscManualPanel.m_wDuration);
					//因为修改了方向的参数，所以指令还要备份过一次
					memcpy(&m_tOldValidManualCmd, &tValidManualCmd, sizeof(tValidManualCmd));
					//切到方向以后，方向开始前继续保持绿色的通道的标志要清零
					memset(m_bKeepGreenChannelBeforeControlChannelFlag, false, sizeof(m_bKeepGreenChannelBeforeControlChannelFlag));

					tValidManualCmd.m_bNewCmd = false;
					m_pOpenATCRunStatus->SetValidManualCmd(tValidManualCmd);

					memset(m_tLockPhaseData, 0, sizeof(m_tLockPhaseData));

					return LOCKCHANNEL_TO_DIRECTION;
				}
				else if (tValidManualCmd.m_nSubCtlMode == CTL_MANUAL_SUBMODE_PANEL_FITSTMANUAL || tValidManualCmd.m_bPatternInterruptCmd)
				{
					m_nManualCurStatus = MANUAL_STAGE_TRANS;
				}

				memset(m_tLockPhaseData, 0, sizeof(m_tLockPhaseData));

				return true;
			}
			else
			{
				return false;
			}
		}
	}

	return true;
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
bool CLogicCtlManual::IsNeglectPhase(int nPhaseID)
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
int CLogicCtlManual::SetNeglectPhaseRunTime(int nRingIndex, bool bPedPhase)
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
void CLogicCtlManual::ReSetNeglectPhaseStageRunTime()
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

				//////////////////机动车相位已运行的时间//////////////////////////////////////
				m_nLampClrTime = 0;
				if (ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseStage == C_CH_PHASESTAGE_G)
				{
					if (ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPhaseGreenTime > tRunCounter.m_nLampClrTime[1])
					{
						m_nLampClrTime = tRunCounter.m_nLampClrTime[1];
					}
					else
					{
						m_nLampClrTime = ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPhaseGreenTime;
					}
				}
				else if (ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseStage == C_CH_PHASESTAGE_GF)
				{
					if (ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byGreenFlash > tRunCounter.m_nLampClrTime[1])
					{
						m_nLampClrTime = ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPhaseGreenTime + tRunCounter.m_nLampClrTime[1];
					}
					else
					{
						m_nLampClrTime = ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPhaseGreenTime + ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byGreenFlash;
					}
				}
				else if (ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseStage == C_CH_PHASESTAGE_Y)
				{
					if (ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhaseYellowChange > tRunCounter.m_nLampClrTime[1])
					{
						m_nLampClrTime = ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPhaseGreenTime + 
							ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byGreenFlash + 
							tRunCounter.m_nLampClrTime[1];
					}
					else
					{
						m_nLampClrTime = ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPhaseGreenTime + 
							ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byGreenFlash + 
							ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhaseYellowChange;
					}
				}
				else if (ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseStage == C_CH_PHASESTAGE_R)
				{
					if (ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhaseRedClear > tRunCounter.m_nLampClrTime[1])
					{
						m_nLampClrTime = ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPhaseGreenTime + 
							ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byGreenFlash + 
							ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhaseYellowChange + 
							tRunCounter.m_nLampClrTime[1];
					}
					else
					{
						m_nLampClrTime = ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPhaseGreenTime + 
							ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byGreenFlash + 
							ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhaseYellowChange + 
							ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhaseRedClear;
					}
				}
				else if (ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseStage == C_CH_PHASESTAGE_F)
				{
					m_nLampClrTime = ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPhaseGreenTime + 
						ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byGreenFlash + 
						ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhaseYellowChange + 
						ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhaseRedClear;
				}
				//////////////////行人相位已运行的时间//////////////////////////////////////
				m_nPedLampClrTime = 0;
				if (ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPedPhaseStage == C_CH_PHASESTAGE_G)
				{
					if (ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPedPhaseGreenTime > tRunCounter.m_nPedLampClrTime[1])
					{
						m_nPedLampClrTime = tRunCounter.m_nPedLampClrTime[1];
					}
					else
					{
						m_nPedLampClrTime = ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPedPhaseGreenTime;
					}
				}
				else if (ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPedPhaseStage == C_CH_PHASESTAGE_GF)
				{
					if (ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhasePedestrianClear > tRunCounter.m_nPedLampClrTime[1])
					{
						m_nPedLampClrTime = ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPedPhaseGreenTime + tRunCounter.m_nPedLampClrTime[1];
					}
					else
					{
						m_nPedLampClrTime = ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPedPhaseGreenTime + ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhasePedestrianClear;
					}
				}
				else if (ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPedPhaseStage == C_CH_PHASESTAGE_R)
				{
					if (ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhaseRedClear > tRunCounter.m_nPedLampClrTime[1])
					{
						m_nPedLampClrTime = ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPedPhaseGreenTime + ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhasePedestrianClear +
							tRunCounter.m_nPedLampClrTime[1];
					}
					else
					{
						m_nPedLampClrTime = ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPedPhaseGreenTime + ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhasePedestrianClear +
							ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhaseRedClear;
					}
				}
				else if (ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPedPhaseStage == C_CH_PHASESTAGE_F)
				{
					m_nPedLampClrTime = ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPedPhaseGreenTime + ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhasePedestrianClear +
							ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhaseRedClear;
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

				//////////////////机动车相位已运行的时间//////////////////////////////////////
				m_nLampClrTime = 0;
				if (ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseStage == C_CH_PHASESTAGE_G)
				{
					if (ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPhaseGreenTime > tRunCounter.m_nLampClrTime[0])
					{
						m_nLampClrTime = tRunCounter.m_nLampClrTime[0];
					}
					else
					{
						m_nLampClrTime = ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPhaseGreenTime;
					}
				}
				else if (ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseStage == C_CH_PHASESTAGE_GF)
				{
					if (ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byGreenFlash > tRunCounter.m_nLampClrTime[0])
					{
						m_nLampClrTime = ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPhaseGreenTime + tRunCounter.m_nLampClrTime[0];
					}
					else
					{
						m_nLampClrTime = ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPhaseGreenTime + ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byGreenFlash;
					}
				}
				else if (ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseStage == C_CH_PHASESTAGE_Y)
				{
					if (ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhaseYellowChange > tRunCounter.m_nLampClrTime[0])
					{
						m_nLampClrTime = ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPhaseGreenTime + 
							ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byGreenFlash + 
							tRunCounter.m_nLampClrTime[0];
					}
					else
					{
						m_nLampClrTime = ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPhaseGreenTime + 
							ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byGreenFlash + 
							ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhaseYellowChange;
					}
				}
				else if (ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseStage == C_CH_PHASESTAGE_R)
				{
					if (ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhaseRedClear > tRunCounter.m_nLampClrTime[0])
					{
						m_nLampClrTime = ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPhaseGreenTime + 
							ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byGreenFlash + 
							ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhaseYellowChange + 
							tRunCounter.m_nLampClrTime[0];
					}
					else
					{
						m_nLampClrTime = ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPhaseGreenTime + 
							ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byGreenFlash + 
							ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhaseYellowChange + 
							ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhaseRedClear;
					}
				}
				else if (ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseStage == C_CH_PHASESTAGE_F)
				{
					m_nLampClrTime = ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPhaseGreenTime + 
						ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byGreenFlash + 
						ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhaseYellowChange + 
						ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhaseRedClear;
				}
				//////////////////行人相位已运行的时间//////////////////////////////////////
				m_nPedLampClrTime = 0;
				if (ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPedPhaseStage == C_CH_PHASESTAGE_G)
				{
					if (ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPedPhaseGreenTime > tRunCounter.m_nPedLampClrTime[1])
					{
						m_nPedLampClrTime = tRunCounter.m_nPedLampClrTime[1];
					}
					else
					{
						m_nPedLampClrTime = ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPedPhaseGreenTime;
					}
				}
				else if (ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPedPhaseStage == C_CH_PHASESTAGE_GF)
				{
					if (ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhasePedestrianClear > tRunCounter.m_nPedLampClrTime[1])
					{
						m_nPedLampClrTime = ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPedPhaseGreenTime + tRunCounter.m_nPedLampClrTime[1];
					}
					else
					{
						m_nPedLampClrTime = ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPedPhaseGreenTime + ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhasePedestrianClear;
					}
				}
				else if (ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPedPhaseStage == C_CH_PHASESTAGE_R)
				{
					if (ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhaseRedClear > tRunCounter.m_nPedLampClrTime[1])
					{
						m_nPedLampClrTime = ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPedPhaseGreenTime + ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhasePedestrianClear +
							tRunCounter.m_nPedLampClrTime[1];
					}
					else
					{
						m_nPedLampClrTime = ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPedPhaseGreenTime + ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhasePedestrianClear +
							ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhaseRedClear;
					}
				}
				else if (ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPedPhaseStage == C_CH_PHASESTAGE_F)
				{
					m_nPedLampClrTime = ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPedPhaseGreenTime + ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhasePedestrianClear +
							ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhaseRedClear;
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
void CLogicCtlManual::ReSetNeglectPhasetBackUpTime()
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

/*==================================================================== 
函数名 ：SetNeglectChannelBoforePhaseLock
功能 ：记录在相位锁定之前的特殊通道(忽略或关断)
算法实现 ： 
参数说明 ：nStageIndex，阶段编号
返回值说明：无
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 李永萍          创建函数 
====================================================================*/ 
void CLogicCtlManual::SetNeglectChannelBoforePhaseLock()
{
	int  i = 0, j = 0, k = 0;

	int nStageIndex = 0;
	if (m_tFixTimeCtlInfo.m_nCurStageIndex > 0)
	{
		nStageIndex = m_tFixTimeCtlInfo.m_nCurStageIndex - 1;
	}
	else
	{
		nStageIndex = m_tRunStageInfo.m_nRunStageCount - 1;
	}

	for (i = 0;i < m_nChannelCount;i++)
	{
		if (m_atChannelInfo[i].m_byChannelControlType == VEH_CHA || m_atChannelInfo[i].m_byChannelControlType == PED_CHA)
		{
			if (IsNeglectPhase(m_atChannelInfo[i].m_byChannelControlSource))
			{
				m_bNeglectChannelBoforePhaseLock[i] = true;
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "SetNeglectChannelBoforePhaseLock ChannelIndex:%d.", i);
			}
		}
		else if (m_atChannelInfo[i].m_byChannelControlType == OVERLAP_CHA || m_atChannelInfo[i].m_byChannelControlType == OVERLAP_PED_CHA)
		{
			for (j = 0;j < m_tFixTimeCtlInfo.m_nOverlapCount;j++)
			{
				if (m_atChannelInfo[i].m_byChannelControlSource == m_tFixTimeCtlInfo.m_atOverlapInfo[j].m_tOverlapParam.m_byOverlapNumber)
				{
					for (k = 0;k < MAX_PHASE_COUNT_IN_OVERLAP;k++)
					{
						BYTE byMainPhaseNum = m_tFixTimeCtlInfo.m_atOverlapInfo[j].m_tOverlapParam.m_byArrOverlapIncludedPhases[k];
						if (byMainPhaseNum > 0)
						{
							for (int m = 0;m < m_tRunStageInfo.m_PhaseRunstageInfo[nStageIndex].m_nConcurrencyPhaseCount;m++)
							{
								if (m_tRunStageInfo.m_PhaseRunstageInfo[nStageIndex].m_nConcurrencyPhase[m] == byMainPhaseNum && IsNeglectPhase(byMainPhaseNum))
								{
									m_bNeglectChannelBoforePhaseLock[i] = true;
									m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "SetNeglectChannelBoforePhaseLock ChannelIndex:%d.", i);
									break;
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
函数名 ：SetLockPhaseList
功能 ：记录锁定相位表
算法实现 ： 
参数说明 ：tValidManualCmd：有效的控制指令
返回值说明：无
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 李永萍          创建函数 
====================================================================*/ 
void CLogicCtlManual::SetLockPhaseList(TManualCmd  tValidManualCmd)
{
	if (!tValidManualCmd.m_bChannelLockCmd || !tValidManualCmd.m_bPhaseToChannelLock)
	{
		return;
	}

	memset(&m_tLockPhaseStage, 0, sizeof(m_tLockPhaseStage));

	int  i = 0;
	for (i = 0; i < tValidManualCmd.m_tPhaseLockPara.m_nPhaseLockCount; i++)
	{
		m_tLockPhaseStage.nLockPhaseID[m_tLockPhaseStage.nLockPhaseCount] = tValidManualCmd.m_tPhaseLockPara.m_nPhaseLockID[i];
		m_tLockPhaseStage.nLockPhaseCount += 1;

		m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "SetLockPhaseList LockPhaseID:%d.", tValidManualCmd.m_tPhaseLockPara.m_nPhaseLockID[i]);
	}
}

/*==================================================================== 
函数名 ：SetLockPhaseStage
功能 ：记录锁定相位解阶段
算法实现 ： 
参数说明 ：tValidManualCmd：有效的控制指令
返回值说明：无
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 李永萍          创建函数 
====================================================================*/ 
void CLogicCtlManual::SetLockPhaseStage(TManualCmd  tValidManualCmd)
{
	if (m_tLockPhaseStage.nLockPhaseCount == 0)
	{
		return;
	}

	if (!m_bOldLockChannelCmdEndFlag)
	{
		if (!m_tOldValidManualCmd.m_bPhaseToChannelLock)
		{
			return;
		}
	}
	else
	{
		if (!tValidManualCmd.m_bPhaseToChannelLock)
		{
			return;
		}
	}


	int nGreenTime = m_tOldValidManualCmd.m_tChannelLockCmd.m_tNextChannelLockCtrlCmd.m_nDuration;
	int nGreenFlashTime = m_tOldValidManualCmd.m_tChannelLockCmd.m_tNextChannelLockCtrlCmd.m_nGreenFlash;
	int nYellowTime = m_tOldValidManualCmd.m_tChannelLockCmd.m_tNextChannelLockCtrlCmd.m_nYellow;
	int nRedTime = m_tOldValidManualCmd.m_tChannelLockCmd.m_tNextChannelLockCtrlCmd.m_nRedClear;

	unsigned int nGlobalCounter = m_pOpenATCRunStatus->GetGlobalCounter();

	bool bFlag = false;
	int  i = 0, j = 0, k = 0;
	for (i = 0; i < m_tLockPhaseStage.nLockPhaseCount;i++)
	{
		unsigned long nCounterDiff = CalcCounter(m_tLockPhaseStage.nLockPhaseCounter[i], nGlobalCounter, C_N_MAXGLOBALCOUNTER);

		if (m_tLockPhaseStage.chLockPhaseStage[i] == C_CH_PHASESTAGE_G)
		{
			if (!m_bOldLockChannelCmdEndFlag)
			{
				bool bFlag = false;
				for (j = 0; j < tValidManualCmd.m_tPhaseLockPara.m_nPhaseLockCount; j++)
				{
					if (m_tLockPhaseStage.nLockPhaseID[i] == tValidManualCmd.m_tPhaseLockPara.m_nPhaseLockID[j])
					{
						bFlag = true;
						break;
					}
				}

				if (!bFlag)
				{
					bool bChangeFlag = true;
					if (tValidManualCmd.m_bPatternInterruptCmd)
					{
						if (!m_tLockPhaseData[0].bLockPhaseInSameStageFlag)
						{
							for (k = 0;k < MAX_PHASE_COUNT;k++)
							{
								if (m_tLockPhaseStage.nLockPhaseID[i] == m_tLockPhaseData[k].nCurLockPhaseID)
								{
									if (!m_tLockPhaseData[k].bNeedTransFlag && !m_tLockPhaseData[k].bLockPhaseInSameStageFlag)
									{
										bChangeFlag = false;
									}
								}
							}
						}
						else
						{
							for (k = 0;k < m_tFixTimeCtlInfo.m_nRingCount;k++)
							{
								int nPhaseIndex = m_tFixTimeCtlInfo.m_nCurPhaseIndex[k];
								PTRingCtlInfo ptRingRunInfo = &(m_tFixTimeCtlInfo.m_atPhaseSeq[k]);
								PTPhaseCtlInfo pPhaseInfo = (PTPhaseCtlInfo)&(ptRingRunInfo->m_atPhaseInfo[nPhaseIndex]);
								int nPhaseID = pPhaseInfo->m_tPhaseParam.m_byPhaseNumber;

								if (m_tLockPhaseStage.nLockPhaseID[i] == nPhaseID)
								{
									bChangeFlag = false;
								}
							}
						}
					}

					if (bChangeFlag && nCounterDiff >= (m_tOldValidManualCmd.m_tChannelLockCmd.m_tNextChannelLockCtrlCmd.m_nMinGreen * C_N_TIMER_TIMER_COUNTER))
					{
						m_tLockPhaseStage.nLockPhaseCounter[i] = nGlobalCounter;
						m_tLockPhaseStage.chLockPhaseStage[i] = C_CH_PHASESTAGE_GF;
						m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "SetLockPhaseStage LockPhaseID:%d TransTo GreenFlash.", m_tLockPhaseStage.nLockPhaseID[i]);
					}
				}
				else
				{
					if (nCounterDiff >= (nGreenTime * C_N_TIMER_TIMER_COUNTER))
					{
						m_tLockPhaseStage.nLockPhaseCounter[i] = nGlobalCounter;
						m_tLockPhaseStage.chLockPhaseStage[i] = C_CH_PHASESTAGE_GF;
						m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "SetLockPhaseStage LockPhaseID:%d TransTo GreenFlash.", m_tLockPhaseStage.nLockPhaseID[i]);
					}
				}
			}
			else
			{
				if (nCounterDiff >= (nGreenTime * C_N_TIMER_TIMER_COUNTER))
				{
					bool bChangeFlag = true;
					if (!m_tLockPhaseData[0].bLockPhaseInSameStageFlag)
					{
						for (k = 0;k < MAX_PHASE_COUNT;k++)
						{
							if (m_tLockPhaseStage.nLockPhaseID[i] == m_tLockPhaseData[k].nCurLockPhaseID)
							{
								if (!m_tLockPhaseData[k].bNeedTransFlag && !m_tLockPhaseData[k].bLockPhaseInSameStageFlag)
								{
									bChangeFlag = false;
								}
							}
						}
					}
					else
					{
						for (k = 0;k < m_tFixTimeCtlInfo.m_nRingCount;k++)
						{
							int nPhaseIndex = m_tFixTimeCtlInfo.m_nCurPhaseIndex[k];
							PTRingCtlInfo ptRingRunInfo = &(m_tFixTimeCtlInfo.m_atPhaseSeq[k]);
							PTPhaseCtlInfo pPhaseInfo = (PTPhaseCtlInfo)&(ptRingRunInfo->m_atPhaseInfo[nPhaseIndex]);
							int nPhaseID = pPhaseInfo->m_tPhaseParam.m_byPhaseNumber;

							if (m_tLockPhaseStage.nLockPhaseID[i] == nPhaseID)
							{
								bChangeFlag = false;
							}
						}
					}

					if (bChangeFlag)
					{
						m_tLockPhaseStage.nLockPhaseCounter[i] = nGlobalCounter;
						m_tLockPhaseStage.chLockPhaseStage[i] = C_CH_PHASESTAGE_GF;
						m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "SetLockPhaseStage LockPhaseID:%d TransTo GreenFlash.", m_tLockPhaseStage.nLockPhaseID[i]);
					}
				}
			}
		}
		else if (m_tLockPhaseStage.chLockPhaseStage[i] == C_CH_PHASESTAGE_GF)
		{
			if (nCounterDiff >= (nGreenFlashTime * C_N_TIMER_TIMER_COUNTER))
			{
				m_tLockPhaseStage.nLockPhaseCounter[i] = nGlobalCounter;
				m_tLockPhaseStage.chLockPhaseStage[i] = C_CH_PHASESTAGE_Y;
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "SetLockPhaseStage LockPhaseID:%d TransTo Yellow.", m_tLockPhaseStage.nLockPhaseID[i]);
			}
		}
		else if (m_tLockPhaseStage.chLockPhaseStage[i] == C_CH_PHASESTAGE_Y)
		{
			if (nCounterDiff >= (nYellowTime * C_N_TIMER_TIMER_COUNTER))
			{
				m_tLockPhaseStage.nLockPhaseCounter[i] = nGlobalCounter;
				m_tLockPhaseStage.chLockPhaseStage[i] = C_CH_PHASESTAGE_R;
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "SetLockPhaseStage LockPhaseID:%d TransTo Red.", m_tLockPhaseStage.nLockPhaseID[i]);
			}
		}
		else if (m_tLockPhaseStage.chLockPhaseStage[i] == C_CH_PHASESTAGE_R)
		{
			if (nCounterDiff >= (nRedTime * C_N_TIMER_TIMER_COUNTER))
			{
				m_tLockPhaseStage.nLockPhaseCounter[i] = nGlobalCounter;
				m_tLockPhaseStage.chLockPhaseStage[i] = C_CH_PHASESTAGE_R;
			}
		}
	}
}