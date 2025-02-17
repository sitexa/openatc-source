/*=====================================================================
ģ���� ���ֶ����Ʒ�ʽ�ӿ�ģ��
�ļ��� ��LogicCtlManual.cpp
����ļ���LogicCtlManual.h,LogicCtlFixedTime.h
ʵ�ֹ��ܣ��ֶ����Ʒ�ʽʵ��
���� ��������
��Ȩ ��<Copyright(c) 2019-2020 Suzhou Keda Technology Co.,Ltd. All right reserved.>
-----------------------------------------------------------------------
�޸ļ�¼��
�� ��           �汾     �޸���     �߶���     �޸ļ�¼
2019/9/14       V1.0     ������     �� ��      ����ģ��
2019/2/28       V1.0     ����Ƽ     ������      �޸�ϵͳ�û������߼�
2019/3/28       V1.0     ����Ƽ     ������      �����ֶ���尴ť�л�����
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
������ ��Init 
���� ���ֶ����Ʒ�ʽ����Դ��ʼ��
�㷨ʵ�� �� 
����˵�� ��pParameter����������ָ��
           pRunStatus��ȫ������״̬��ָ��
		   pOpenATCLog����־ָ��
           nPlanNo��ָ���ķ�����,0��ʾʹ��ʱ�ζ�Ӧ�ķ���  
����ֵ˵����
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ������          �������� 
====================================================================*/ 
void CLogicCtlManual::Init(COpenATCParameter * pParameter, COpenATCRunStatus * pRunStatus, COpenATCLog * pOpenATCLog, int nPlanNo)
{
    m_pOpenATCParameter = pParameter;
    m_pOpenATCRunStatus = pRunStatus;
	m_pOpenATCLog       = pOpenATCLog;
    memset(&m_tFixTimeCtlInfo,0,sizeof(m_tFixTimeCtlInfo));

	//��ǰ����ģʽΪ�ֶ�����
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
������ ��Run 
���� ���ֶ����Ʒ�ʽ������ʵ��
�㷨ʵ�� �� 
����˵�� �� 
����ֵ˵����
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ������          �������� 
2020/02/18     V1.0 ����Ƽ          �����˷������ 
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

	ClearPulse();//�������

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
        
        //����ȫ�ֵ�ɫ״̬
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
������ ��ManualOnePhaseRun
���� ���ֶ�����ʱ������������λ����״̬����
�㷨ʵ�� �� 
����˵�� ��nRingIndex����������tValidManualCmd���ֶ�ָ��
����ֵ˵����
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ������          �������� 
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
			if (tValidManualCmd.m_tStepForwardCmd.m_nNextStageID == 0)//ɫ������
			{
				if (!m_bStageStepwardInColorStep)
				{
					nGreenTime = ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_wPhaseMinimumGreen;//�ڿ��ƽ׶β�������С��
				}
				if (!tValidManualCmd.m_bStepForwardCmd && !m_bStageStepwardInColorStep)
				{
					bFlag = false;//ɫ������ʱ���ڿ��ƽ׶Σ��̵�ʱ�䳬����С�̣����Ұ��˲������ܱ������
				}
				if (tValidManualCmd.m_nSubCtlMode == CTL_MANUAL_SUBMODE_PANEL_STEPFOWARD && tAscManualPanel.m_byMinGreenIgnore == 1)//������С��
				{
					nGreenTime = 0;
				}
			}
		}
	}
	else//�׶β����Ŀ���
	{
		if (tValidManualCmd.m_nSubCtlMode == CTL_MANUAL_SUBMODE_PANEL_STEPFOWARD && tAscManualPanel.m_byMinGreenIgnore == 1)//������С��
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
	
    //��ǰ��λ�׶��Ƿ����
    if (ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseStage == C_CH_PHASESTAGE_G)
    {
        if (tRunCounter.m_nLampClrTime[nRingIndex] >= nGreenTime && bFlag)
        {
			//��׶���λ��Ҫ�����̵�״̬�����е�������һ���׶�ʱ�����е�����
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

			//�ڿ��ƹ����е���λ�̵�û�е�����λ�����һ���׶ζ����У���������
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
		if (tStepCfg.m_byStepType == STEP_STAGE || m_nManualCurStatus == MANUAL_STAGE_TRANS)//�׶β����͹���״̬ʱ�����̵�ɫֻҪ��������ʱ����л���ɫ
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
		else //ɫ���������̵�ɫ�л�ʱ���ڿ��ƽ׶Σ�����ǲ�������һ����һ�Σ�������н׶Σ���ֻҪ��������ʱ����л�
		{
			bool bSwitch = true;
			if (tStepCfg.m_byStepType == STEP_COLOR)
			{
				if (tValidManualCmd.m_tStepForwardCmd.m_nNextStageID == 0)
				{
					if (!tValidManualCmd.m_bStepForwardCmd)//��һ�ΰ���岽�����ڵ�ǰ��ɫ
					{
						bSwitch = false;
					}

					//ɫ��ģʽ���н׶Σ�����ʱ���е���һ����ɫ
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
			//ɫ��������������λ�Ի�������λΪ׼
			m_tFixTimeCtlInfo.m_bIsChgPedStage[nRingIndex] = true;    
		}

        char chcurStage = ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseStage;
        WORD wStageTime = 0;
        char chNextStage =  this->GetNextPhaseStageInfo(chcurStage,pPhaseInfo,wStageTime);
		ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseStage = chNextStage;

		if (tStepCfg.m_byStepType == STEP_COLOR && m_nManualCurStatus == MANUAL_CONTROL_STATUS)
		{
			/*//ɫ����������ɫ�ӻ��е���ʱ��״̬��Ϊ�ɽ����������ٰ�������ֱ���е��̣�����Ҫ�ఴһ�β���
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

		// �̵�ʱ����RecalculteStepForwardGreenTime�������Ѿ��������
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

			//���û�����λ��ɫ������ʱ��
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
			//���ɵ���巽���ͨ�������Ĺ����У�����������
			SendCountDownPulse(nRingIndex, false);
		}
	}
}

/*==================================================================== 
������ ��ManualOnePedPhaseRun
���� ���ֶ�����ʱ����������λ����״̬����
�㷨ʵ�� �� 
����˵�� ��nRingIndex�������� 
����ֵ˵����
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ������          �������� 
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

	//��ǰ��λ�׶��Ƿ����
	if (tStepCfg.m_byStepType == STEP_STAGE)
	{
		//������λ��������������λ����һ�����
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

				//��׶���λ��Ҫ�����̵�״̬�����е�������һ���׶�ʱ�����е�����
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
				
				//�ڿ��ƹ����е���λ�̵�û�е�����λ�����һ���׶ζ����У���������
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

			//���û�����λ��ɫ������ʱ��
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
			//���ɵ���巽���ͨ�������Ĺ����У�����������
			SendPedCountDownPulse(nRingIndex, false);
		}
	}
}

/*==================================================================== 
������ ��SetCtlDerivedParam
���� �������ֶ�����ʱʹ�õĲ������ݺ͵�ǰ������״̬
�㷨ʵ�� �� 
����˵�� ��
        pParam���̳еĲ���ָ��.
����ֵ˵����
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ������          �������� 
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
������ ��ManualCycleChg
���� �������Ƿ����н���״̬�ж�
�㷨ʵ�� �� 
����˵�� ��
����ֵ˵����
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ������          �������� 
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
	//û�в�����ͨ�����ƣ��ڿ���״̬ʱ�����ܽ�������
	if (m_tFixTimeCtlInfo.m_nRingCount == 0 && m_nManualCurStatus == MANUAL_CONTROL_STATUS)
	{
		bCycleChg = false;
	}

    //��ǰ�����Ƿ����
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
������ ��SysCtlOnePhaseRun
���� ���ֶ���Ԥʱ��λ����״̬����
�㷨ʵ�� �� 
����˵�� ��tValidManualCmd���ֶ�ָ��
����ֵ˵����
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ����Ƽ          �������� 
====================================================================*/ 
void CLogicCtlManual::SysCtlOnePhaseRun(TManualCmd & tValidManualCmd)
{
	static int nCycleEnd = 0;
	int nStageIndex = 0;
	if (ManualSwitchStage(tValidManualCmd, nStageIndex))//�л���ָ���׶�
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
				// �������û���Ԥָ��ص�����,��ͨ������ָ�������ܻ�����
				if (m_nReturnAutoCtrlStageIndex == m_tFixTimeCtlInfo.m_nCurStageIndex && !tValidManualCmd.m_bChannelLockCmd)
				{
					CreateManualCmdReturnToSelf(tValidManualCmd);//���ɻص����������COpenATCLogicCtlManager���Ը����������������������ص�����
					m_bTransToAutoFlag = true;
					m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "Switch Stage Succeed, Set TransToAutoFlag And CreateManualCmdReturnToSelf");

					//��ʼ��������Ҫ�����λ���Ʊ�
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
				if (nCycleEnd == 0)//�ֶ���尴ť���º���������������£��ڶ������ڿ�ʼ������
				{
					CreateManualCmdReturnToSelf(tValidManualCmd);//���ɻص����������COpenATCLogicCtlManager���Ը����������������������ص�����
					m_bTransToAutoFlag = true;
					m_bCycleEndFlag = true;
					m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "FirstManual Trans Succeed, Set TransToAutoFlag And CreateManualCmdReturnToSelf");
				}
			}

			m_tFixTimeCtlInfo.m_nCurStageIndex = nStageIndex;
		}

		//�жϵ�ǰ�ǲ���ɫ��ģʽ�µĽ׶�פ��
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
		//��ǰ�׶ι��ɽ�����׼���з���
		if (tValidManualCmd.m_bDirectionCmd)
		{
			if (tValidManualCmd.m_nSubCtlMode == CTL_MANUAL_SUBMODE_PANEL_STEPFOWARD)
			{
				tValidManualCmd.m_tDirectionCmd.m_bStepFowardToDirection = true;
			}

            tValidManualCmd.m_nSubCtlMode = CTL_MANUAL_SUBMODE_PANEL_DIRECTION;
			tValidManualCmd.m_tDirectionCmd.m_nTargetStageIndex = m_tFixTimeCtlInfo.m_nCurStageIndex;//��¼���з���֮ǰ����һ���׶�

			tValidManualCmd.m_bDirectionCmd = false;//�е����������ģʽ�£������ǰ����ָ���־
		   
			//��ʼ�������������ʼ�з���
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
			//��Ϊ�޸��˷���Ĳ���������ָ�Ҫ���ݹ�һ��
			memcpy(&m_tOldValidManualCmd, &tValidManualCmd, sizeof(tValidManualCmd));
			//�е������Ժ󣬷���ʼǰ����������ɫ��ͨ���ı�־Ҫ����
			memset(m_bKeepGreenChannelBeforeControlChannelFlag, false, sizeof(m_bKeepGreenChannelBeforeControlChannelFlag));
		}
		else if (tValidManualCmd.m_bPatternInterruptCmd && m_nManualCurStatus == MANUAL_STAGE_TRANS)
		{
			//�յ�������Ԥָ�Ҫ�����λ���Ʊ�
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
			tValidManualCmd.m_tChannelLockCmd.m_nTargetStageIndex = m_tFixTimeCtlInfo.m_nCurStageIndex;//��¼����ͨ������֮ǰ����һ���׶�

			tValidManualCmd.m_bNewCmd = false;
			tValidManualCmd.m_bChannelLockCmd = false;//�е�ͨ������������ģʽ�£������ǰͨ������ָ���־
		   
			//��ʼ��ͨ��������������ʼ��ͨ������
			InitParamBeforeChannelLock(tValidManualCmd.m_tChannelLockCmd.m_tNextChannelLockCtrlCmd.m_nChannelLockStatus);
			ChangeChannelClr();
			for (int i = 0;i < MAX_CHANNEL_COUNT;i++)
			{
				m_nChannelLockGreenTime[i] = tValidManualCmd.m_tChannelLockCmd.m_tNextChannelLockCtrlCmd.m_nDuration;
				m_bChannelKeepGreenFlag[i] = false;
			}
			m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "Switch Stage Succeed, Remember NextStageIndex:%d And InitParamBeforeChannelelLock, Calculate GreenTime:%d", m_tFixTimeCtlInfo.m_nCurStageIndex, tValidManualCmd.m_tChannelLockCmd.m_tNextChannelLockCtrlCmd.m_nDuration);
			//��Ϊ�޸���ͨ�������Ĳ���������ָ�Ҫ���ݹ�һ��
			memcpy(&m_tOldValidManualCmd, &tValidManualCmd, sizeof(tValidManualCmd));
			//�е�ͨ�������Ժ�����ͨ����ʼǰ����������ɫ��ͨ���ı�־Ҫ����
			memset(m_bKeepGreenChannelBeforeControlChannelFlag, false, sizeof(m_bKeepGreenChannelBeforeControlChannelFlag));

			//��ȡ������λ����
			GetLockPhaseData(tValidManualCmd);
		}
	
		tValidManualCmd.m_bStepForwardCmd = false;//�ֶ�����ģʽ�£������ǰ����ָ���־
		tValidManualCmd.m_tStepForwardCmd.m_nNextStageID = 0;
		m_pOpenATCRunStatus->SetValidManualCmd(tValidManualCmd);

		m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "Switch Stage Succeed, ControlMode:%d SubControlMode:%d StepForwardCmd Status:%d", tValidManualCmd.m_nCtlMode, tValidManualCmd.m_nSubCtlMode, tValidManualCmd.m_bStepForwardCmd);
	}
}

/*==================================================================== 
������ ��GetChannelData
���� ����ȡͨ���йصĲ���
�㷨ʵ�� �� 
����˵�� ��nNextDirectionIndex��������
����ֵ˵������
 ----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ����Ƽ          �������� 
====================================================================*/ 
void CLogicCtlManual::GetChannelData(int nNextDirectionIndex)
{
	int i = 0, j = 0, k = 0;

	TAscManualPanel tAscManualPanel;
    memset(&tAscManualPanel, 0, sizeof(TAscManualPanel)); 
    m_pOpenATCParameter->GetManualPanelInfo(tAscManualPanel);

	
	//��ȡ��ǰͨ����Ҫ�л���Ŀ���ɫ
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

	//��ȡ��ǰͨ���ĵ�ɫ
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
������ ��ProcessDirection
���� ��������
�㷨ʵ�� �� 
����˵�� ��tValidManualCmd���ֶ�����
����ֵ˵������
 ----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ����Ƽ          �������� 
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
			memcpy(&m_tOldValidManualCmd, &tValidManualCmd, sizeof(tValidManualCmd));//�ӻ�����ȫ���ص�ֱ�ӽ��뷽��ģ���Ҫ��������
		}

		//�����뷽���Ժ󣬸���������ָ������̵�ʱ��
		if ((tValidManualCmd.m_bPatternInterruptCmd && tValidManualCmd.m_tPatternInterruptCmd.m_nControlMode == CTL_MODE_SELFCTL) || //�����뷽���Ժ󣬰�������
			tValidManualCmd.m_bDirectionCmd || //�����뷽���Ժ��ְ�����
			tValidManualCmd.m_bStepForwardCmd)//�����뷽���Ժ󣬰����Զ�������û�����ü����ɵ��Զ����ְ����ֶ�
		{
			//����1�е�����2������1��û�й��ɵ���С�̣����з���1���̵�ʱ�仹�����̵ƹ���ʱ�䣬��������̵�Ӧ����С��
			//����1���ڹ��ɵ�ɫ(�������ƣ���)�����з���1������1���꣬�ٻص�����1
			if (tValidManualCmd.m_bDirectionCmd && tValidManualCmd.m_tDirectionCmd.m_nNextDirectionIndex == m_tOldValidManualCmd.m_tDirectionCmd.m_nNextDirectionIndex)
			{
				//��ȡ��ǰͨ����Ҫ�л���Ŀ���ɫ
				for (int i = 0;i < MAX_PANEL_KEY_CFG_COUNT;i++)
				{
					if (tAscManualPanel.m_stPanelKeyCfg[i].m_byKeyNum == m_tOldValidManualCmd.m_tDirectionCmd.m_nNextDirectionIndex)
					{    
						for (int j = 0;j < m_nChannelCount;j++)
						{
							if (tAscManualPanel.m_stPanelKeyCfg[i].m_ChannelCfg[j].m_byChannelStatus != CHANNEL_STATUS_DEFAULT && 
								m_chChannelStage[j] == C_CH_PHASESTAGE_G)
							{
								tValidManualCmd.m_bDirectionCmd = false;//����1�е�����2������1��û�й��ɵ���С�̣����з���1������ͬ��ֵָ�ִ��
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
				//��ȡ��ǰͨ����Ҫ�л���Ŀ���ɫ
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
		else//�տ�ʼ������巽��
		{
			//��ʼ�������������ʼ�з���
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

	bool bLockCmd = false;//��һ��ָ���Ƿ�������ָ���־
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
			if (tValidManualCmd.m_bDirectionCmd)//������ɽ����������µķ���
			{
				//��ʼ�������������ʼ�������
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
				//����µķ���ָ���־����Ϊ�Ƿ����з������Ե�ǰ������ģʽ���Ƿ������
				tValidManualCmd.m_bNewCmd = false;
				tValidManualCmd.m_bDirectionCmd = false;
				m_tOldValidManualCmd.m_tDirectionCmd.m_nNextDirectionIndex = tValidManualCmd.m_tDirectionCmd.m_nNextDirectionIndex;
				//memcpy(&tValidManualCmd.m_tDirectionCmd,&m_tOldValidManualCmd.m_tDirectionCmd,sizeof(TDirectionCmd));
				m_pOpenATCRunStatus->SetValidManualCmd(tValidManualCmd);
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "ProcessDirection, TransitDirection End, Trans To NewDirectionIndex:%d GreenTime:%d", tValidManualCmd.m_tDirectionCmd.m_nNextDirectionIndex, tAscManualPanel.m_wDuration);
			}
			else
			{
				//��Ϊ��尴������ָ����Է�����ɽ������޸ĵ�ǰ����ģʽ�Ϳ�����ģʽ
				if (tValidManualCmd.m_bPatternInterruptCmd)
				{
					tValidManualCmd.m_nCtlMode = CTL_MODE_SELFCTL;
					tValidManualCmd.m_nSubCtlMode = CTL_MANUAL_SUBMODE_DEFAULT;
					tValidManualCmd.m_bNewCmd = false;
					m_pOpenATCRunStatus->SetValidManualCmd(tValidManualCmd);
					m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "ProcessDirection, TransitDirection End, Trans To SelfCtl");
					m_nManualCurStatus = MANUAL_STAGE_TRANS;//�������

					//��ʱΪ��Ӳ��ģ���m_bManualBtn����false����Ҫ�ٴη���������ť����״̬
					THWPanelBtnStatus tHWPanelBtnStatus;
					tHWPanelBtnStatus.m_nHWPanelBtnIndex = HWPANEL_BTN_AUTO;
					tHWPanelBtnStatus.m_bHWPanelBtnStatus = true;
					m_pOpenATCRunStatus->SetPanelBtnStatusList(tHWPanelBtnStatus);
				}
				else //�����뷽���Ժ󣬰����Զ�������û�����ü����ɵ��Զ����ְ����ֶ��������һ���ֶ�����ģʽ
				{
					tValidManualCmd.m_nSubCtlMode = CTL_MANUAL_SUBMODE_PANEL_FITSTMANUAL;
					tValidManualCmd.m_bNewCmd = false;
					m_pOpenATCRunStatus->SetValidManualCmd(tValidManualCmd);
					m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "ProcessDirection, TransitDirection End, Trans To Panel First Manual");
				}

				//�·�������ָ����ֶ�ָ�������ɽ�����ֱ���޸Ŀ���ģʽ����ģʽ������ָ��
			    memcpy(&m_tOldValidManualCmd, &tValidManualCmd, sizeof(tValidManualCmd));
			}
		}
		else
		{
			if (tValidManualCmd.m_tDirectionCmd.m_bStepFowardToDirection)//�ص�����ǰ�Ĳ���
			{
				TAscStepCfg tStepCfg;
				memset(&tStepCfg, 0, sizeof(tStepCfg));
				m_pOpenATCParameter->GetStepInfo(tStepCfg);
				//���ɻص�����ǰ�Ĳ���ָ��ص��з���ǰ�Ľ׶ε���һ���׶�
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

				//��ʼ����һ���׶ε���λ
				for (int i = 0;i < m_tFixTimeCtlInfo.m_nRingCount;i++)
				{
					InitNextStagePhase(i, m_tFixTimeCtlInfo.m_nCurPhaseIndex[i], tValidManualCmd.m_tDirectionCmd.m_nTargetStageIndex, true);
				}

				THWPanelBtnStatus tHWPanelBtnStatus;
				tHWPanelBtnStatus.m_nHWPanelBtnIndex = HWPANEL_BTN_STEP;
				tHWPanelBtnStatus.m_bHWPanelBtnStatus = true;
				m_pOpenATCRunStatus->SetPanelBtnStatusList(tHWPanelBtnStatus);
			}
			else //�û���Ԥ��ʱ���ص�����
			{
				m_nManualCurStatus = MANUAL_STAGE_TRANS;//��Ҫ��Ϊ����״̬
                CreateManualCmdReturnToSelf(tValidManualCmd);//���ɻص����������COpenATCLogicCtlManager���Ը����������������������ص�����
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
������ ��ManualSwitchStage
���� ���е�ָ��Ŀ��׶κ󣬳�ʼ����һ���׶ε���λ
�㷨ʵ�� �� 
����˵�� ��tValidManualCmd���ֶ����nStageIndex��Ŀ��׶κ�
����ֵ˵������
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ����Ƽ          �������� 
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
        nRepeatCnt = 0;//��һ���ֶ��з������ʱ�������ǿ�׶�
	}

    if (nCount + nRepeatCnt == m_tFixTimeCtlInfo.m_nRingCount)
	{
		//�׶�1������λ1��3���׶�2������λ1��4���׶�3������λ2��4���׶�1ʱ�ض���λ1��3���������׶�2Ҫ��ס
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
					//�Ӵ������Ŀ��Ʒ�ʽ��ȥͨ���������ƣ�Ҫ�ù���״̬
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
������ ��ChangeChannelClr
���� ���ֶ���巽����ƻ�����ͨ��״̬����
�㷨ʵ�� �� 
����˵�� ����
����ֵ˵����
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ����Ƽ          �������� 
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
		    if (m_chChannelStatus[i] == CHANNEL_STATUS_GREEN)//�л�����ɫ
		    {   
			    m_nChannelCounter[i] = nGlobalCounter;
                m_nChannelDurationCounter[i] = nGlobalCounter;
			    bIsChgStage = true;

		        m_chChannelStage[i] = C_CH_PHASESTAGE_G;
			    //m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "SwitchChannel:%d To Green GlobalCounter:%d", i, nGlobalCounter);
		    }
		    else if (m_chChannelStatus[i] == CHANNEL_STATUS_RED)//�л��ɺ�ɫ
		    {
                m_nChannelCounter[i] = nGlobalCounter;
                m_nChannelDurationCounter[i] = nGlobalCounter;
			    bIsChgStage = true;

		        m_chChannelStage[i] = C_CH_PHASESTAGE_R;
			    //m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "SwitchChannel:%d To Red GlobalCounter:%d", i, nGlobalCounter);
		    }
		    else if (m_chChannelStatus[i] == CHANNEL_STATUS_OFF)//�л��ɹص�
		    {
                m_nChannelCounter[i] = nGlobalCounter;
                m_nChannelDurationCounter[i] = nGlobalCounter;
			    bIsChgStage = true;

		        m_chChannelStage[i] = C_CH_PHASESTAGE_OF;
			    //m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "SwitchChannel:%d To Off GlobalCounter:%d", i, nGlobalCounter);
		    }
            else if (m_chChannelStatus[i] == CHANNEL_STATUS_DEFAULT)//�л���Ĭ��
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
������ ��TransitChannelClr
���� ��
�㷨ʵ�� ���������δ������ʱ���û����и�Ԥ�����ʱ�������õ���С��ʱ�����У����������õĳ���ʱ������
����˵�� ��byChannelType, ͨ�����ͣ�0��ʾ����ͨ����1��ʾ����ͨ��
����ֵ˵����true��������ɽ���
            false���������û�н���
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ����Ƽ          �������� 
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
					    m_chChannelStage[i] = C_CH_PHASESTAGE_GF;//�л�������

                        //m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "TransitChannelClr Channel:%d Green To FreenFlash", i);
				    }
			    }
			    else
			    {
                    m_nChannelCounter[i] = nGlobalCounter;

				    bIsChgStage = true;
				    m_chChannelStage[i] = C_CH_PHASESTAGE_GF;//�л�������
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
					        m_chChannelStage[i] = C_CH_PHASESTAGE_Y;//�л��ɻ�ɫ

                            //m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "TransitChannelClr Channel:%d GreenFlash To Yellow", i);
                        }
                        else
                        {
                            m_chChannelStage[i] = C_CH_PHASESTAGE_R;//�л��ɺ�ɫ

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
				        m_chChannelStage[i] = C_CH_PHASESTAGE_Y;//�л��ɻ�ɫ
                    }
                    else
                    {
                        m_chChannelStage[i] = C_CH_PHASESTAGE_R;//�л��ɺ�ɫ
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
					    m_chChannelStage[i] = C_CH_PHASESTAGE_R;//�л��ɺ�ɫ

                        //m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "TransitChannelClr Channel:%d Yellow To Red", i);
				    }
			    }
			    else
			    {
                    m_nChannelCounter[i] = nGlobalCounter;

				    bIsChgStage = true;
				    m_chChannelStage[i] = C_CH_PHASESTAGE_R;//�л��ɺ�ɫ
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
							m_chChannelStage[i] = C_CH_PHASESTAGE_R;//�������ֺ�ɫ

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
				     m_chChannelStage[i] = C_CH_PHASESTAGE_R;//�������ֺ�ɫ    

					 m_bChannelTran[i] = true;
                }  
		    }
        }

		if (bIsChgStage && m_bChannelKeepGreenFlag[i])
		{
			m_chChannelStage[i] = C_CH_PHASESTAGE_G;//ͨ���緽��������ʱ������������ɫ  
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
					SetOneChannelOutput(pStartPos, C_CH_PHASESTAGE_G);//��һ���׶ε���λ��ͨ����ĸ��λ������Ҫ���ɵ�ɫ
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
			bSelRun[i] = true;//ͨ������ʱ���̵Ƽ������֣�������л�����һ��ָ��
		}
		if (bChannelNoNeedTransFlag[i])
		{
			bSelRun[i] = true;//ͨ��������������ʱ����һ���׶ε���λ��ͨ����ĸ��λ������Ҫ���ɵ�ɫ��������л�����һ��ָ��
		}
		if (m_bChangeChannelClr[i] && !bSelRun[i])
		{
			//1����ͨ�������̻�������ƣ���˵��ͨ������û�н���
			//2��������λ����ͬһ���׶�ʱ��Ŀ����λһֱ�����̣���Ҫ���ɵķ�Ŀ����λ�ɹ����ɵ�Ŀ����λ��������λ����ͬһ���׶Σ���������
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
������ ��InitParamBeforeDirection
���� ��
�㷨ʵ�� �����л�����֮ǰ�ȳ�ʼ������
����˵�� ��tCtlCmd���ֶ��������
����ֵ˵������
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ����Ƽ          �������� 
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
	GetChannelData(nNextDirectionIndex);//�л�������֮ǰ��Ҫ��ȡͨ����Ŀ���ɫ
}

/*==================================================================== 
������ ��IsTimeInSpan
���� ���жϵ�ǰʱ���Ƿ���ʱ�����
�㷨ʵ�� �� 
����˵�� ��
           nCurHour����ǰʱ
		   nCurMin����ǰ��
		   nCurSec����ǰ��
           nStartHour����ʼʱ
           nStartMin����ʼ��
		   nStartSec����ʼ��
           nEndHour������ʱ
		   nEndMin��������
		   nEndSec��������
����ֵ˵������ǰʱ���Ƿ���ʱ�����
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ����Ƽ          �������� 
====================================================================*/
bool CLogicCtlManual::IsTimeInSpan(int nCurHour, int nCurMin, int nCurSec, int nStartHour, int nStartMin, int nStartSec, int nEndHour, int nEndMin, int nEndSec)
{
    int iBegLessThanEnd = 0; //1-���õĿ�ʼʱ��С�ڽ���ʱ�� 0-���õĿ�ʼʱ����ڽ���ʱ��

	if (nStartHour < nEndHour || (nStartHour == nEndHour && nStartMin <= nEndMin))
	{
		iBegLessThanEnd = 1;
	}
	else
	{
		iBegLessThanEnd = 0;
	}

	if (iBegLessThanEnd)   //��ʼʱ��С�ڽ���ʱ��
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
	else   //��ʼʱ����ڽ���ʱ��, ��������
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
������ ��CheckIfNeedToRunToNextStage
���� �����ڹ��ɵ���λ���̵ƴﵽ�л�������ʱ���������״̬
�㷨ʵ�� �� 
����˵�� ����
����ֵ˵����true�������е���һ���׶Σ�false�������е���һ���׶�
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ����Ƽ          �������� 
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
������ ��CheckIfNeedToStepForwardToNextStage
���� �����ڹ��ɵ���λ���̵ƴﵽ�л�������ʱ���������ǲ�������ģʽʱ�������ù�����ʱ���ģ���������ʱ�����л���ɫ��������С��
�㷨ʵ�� �� 
����˵�� ��tValidManualCmd���ֶ�ָ��
����ֵ˵����true�������е���һ���׶Σ�false�������е���һ���׶�
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 �º���          �������� 
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
			// �жϵ�ǰ��λ�Ƿ����һ���׶κ͵�ǰ�׶�
			if (m_tRunStageInfo.m_PhaseRunstageInfo[nCurStageIndex].m_nConcurrencyPhase[i] == m_tRunStageInfo.m_PhaseRunstageInfo[nStageIndexTarget].m_nConcurrencyPhase[i])
			{
				if (tValidManualCmd.m_bDirectionCmd || tValidManualCmd.m_bChannelLockCmd)//�����з����ͨ����������׶ε���λ��Ҫ�ڵ�ǰ�׶����̹�����
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
������ ��CheckIfStageHavePhaseInGreen
���� ���жϵ�ǰ��λ�Ƿ����̵�
�㷨ʵ�� �� 
����˵�� ��nGreenPhaseCount���̵���λ������nClosePhaseCount���ر���λ����
����ֵ˵������ǰ��λ�Ƿ����̵�
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 �º���          �������� 
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
������ ��SendPedCountDownPulse
���� �����ͻ���������ʱ���塣
�㷨ʵ�� �� 
����˵�� ��nRingIndex������, bNeedSendRedPulseFlag����Ҫ���ͺ������־
����ֵ˵������
 ----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 �º���          �������� 
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
			// ��ǰ��λΪ�رշ���״̬����δ���й�������������
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
			// ��һ��λΪ�رշ���״̬������������
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
	
	//���ø�����λ�����壬�ֶ����ƹ����У���һ���������ֶ����ս׶β�����һ���׶ε���λ�����򣬵�һ�������Ƿ��ֶ���������λ��˳��
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
������ ��SendPedCountDownPulse
���� ���������˵���ʱ���塣
�㷨ʵ�� �� 
����˵�� ��nRingIndex�����ţ�bNeedSendRedPulseFlag����Ҫ���ͺ������־
����ֵ˵������
 ----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 �º���         �������� 
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
			// ��ǰ��λΪ�رշ���״̬����δ���й�������������
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
			// ��һ��λΪ�رշ���״̬������������
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

	//���ø�����λ�����壬�ֶ����ƹ����У���һ���������ֶ����ս׶β�����һ���׶ε���λ�����򣬵�һ�������Ƿ��ֶ���������λ��˳��
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
������ ��ProcessStepward
���� ����������
�㷨ʵ�� �� 
����˵�� ��tValidManualCmd���ֶ�����
����ֵ˵������
 ----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ����Ƽ          �������� 
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
			m_bClearPulseFlag = false;//ϵͳ�����е��ֶ�����ʱ��Ҫ���������
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
		if (tValidManualCmd.m_tStepForwardCmd.m_byStepType == STEP_COLOR && tValidManualCmd.m_tStepForwardCmd.m_nNextStageID == 0)//ɫ������ÿ����һ�Σ�Ҫ��һ�β���״̬
		{
			tValidManualCmd.m_bNewCmd = false;
			tValidManualCmd.m_bStepForwardCmd = false;

			//ɫ����������ɫ�Ӻ��е��ɽ���ʱ������״̬�����㣬����ֱ���е���һ����λ���̣�����Ҫ�ఴһ�β���
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
			//ɫ����������ɫ�Ӻ��е��ɽ���ʱ������״̬�����㣬����ֱ���е���һ����λ���̣�����Ҫ�ఴһ�β���
			tValidManualCmd.m_bStepForwardCmd = true;
			m_pOpenATCRunStatus->SetValidManualCmd(tValidManualCmd);
		}
	}
}

/*==================================================================== 
������ ��GetPhaseRemainTime
���� ����ȡ��������λʣ��ʱ��
�㷨ʵ�� �� 
����˵�� ��nRingIndex������ţ�pPhaseCtlInfo����λ��Ϣ�� tRunCounter����ɫ������wPhaseTime����λʱ��
����ֵ˵������
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 �º���          �������� 
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
������ ��GetPedPhaseRemainTime
���� ����ȡ������λʣ��ʱ��
�㷨ʵ�� �� 
����˵�� ��nRingIndex������ţ�pPhaseCtlInfo����λ��Ϣ�� tRunCounter����ɫ������wPhaseTime����λʱ��
����ֵ˵������
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 �º���          �������� 
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
������ ��SendRedPulse
���� �����ͺ�����
�㷨ʵ�� �� 
����˵�� ��ptRingRunInfo������Ϣ��nRingIndex������ţ� nIndex����λ��ţ�nIndex����һ����λ��ţ�bFlag�����ͱ�־
����ֵ˵������
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 �º���          �������� 
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
������ ��ResetNextStagePhaseGreenTime
���� ��������һ���׶ε���λ���̵�ʱ��
�㷨ʵ�� �� 
����˵�� ��nRingIndex�����ţ�nPhaseIndex����λ�ţ�nNextStageIndex���׶κţ�bChangeStageFlag���н׶κ�����λ��־��nDuration������ʱ��
����ֵ˵������
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0  �º���         �������� 
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

	//�����̵�ʱ�䣬�ֲ����׶κ͹��ɽ׶Σ������׶η�׼���к��Ѿ��е�����״̬
	if (m_nManualCurStatus == MANUAL_CONTROL_STATUS)																		//�ڲ���������
	{
		if (m_nReturnAutoCtrlStageIndex == m_tFixTimeCtlInfo.m_nCurStageIndex)												//�Ѿ��е���һ���׶Σ�������׼�����ɻ������ĵ�һ���׶ε���λʱ��
		{	
			if (bChangePhaseFlag)																							//��ǰ��λ�������е���һ����λ
			{
				if (m_tRunStageInfo.m_PhaseRunstageInfo[nNextStageIndex].m_bPhaseStartTransitInCurStage[nRingIndex])		//�¸���λ����һ���׶ι��ɵ�ɫ
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
				
				if (m_tRunStageInfo.m_PhaseRunstageInfo[nNextStageIndex].m_bPedPhaseStartTransitInCurStage[nRingIndex])	//�¸�������λ����һ���׶ι��ɵ�ɫ
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
			//��λ����һ���׶�
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
				//������λ�ĹصƵ�ʱ��͹ض���λ���̵�ʱ������Ϊ0
				m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_wPhaseGreenTime = 0;
				m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_wPedPhaseGreenTime = 0;
			}

			m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "In ControlStage, Prepare To TransStage, RingIndex=%d PhaseIndex=%d StageIndex=%d PhaseGreenTime=%d PedPhaseGreenTime=%d.", 
														nRingIndex, nPhaseIndex,nNextStageIndex, m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_wPhaseGreenTime,
														m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_wPedPhaseGreenTime);
		}
		else//׼������һ���׶Σ����ճ���ʱ������̵�ʱ��
		{
			if (bChangePhaseFlag)
			{
				// ��ǰ�׶ε���λ�絽��һ���׶Σ�����Ҫ������С�̣����ճ���ʱ��
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

			// ����������λ
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
				//������λ�ĹصƵ�ʱ��͹ض���λ���̵�ʱ������Ϊ0
				m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_wPhaseGreenTime = 0;
				m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_wPedPhaseGreenTime = 0;
			}

			m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "In ControlStage, Prepare To Next Stage, RingIndex=%d PhaseIndex=%d StageIndex=%d PhaseGreenTime=%d PedPhaseGreenTime=%d.", 
														nRingIndex, nPhaseIndex,nNextStageIndex, m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_wPhaseGreenTime,
														m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_wPedPhaseGreenTime);
		}
	}
	else//�ڹ��ɽ׶�
	{
		if (bChangePhaseFlag)																							//��ǰ��λ�������е���һ����λ
		{
			if (m_tRunStageInfo.m_PhaseRunstageInfo[nNextStageIndex].m_bPhaseStartTransitInCurStage[nRingIndex])		//�¸���λ����һ���׶ι��ɵ�ɫ
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

			if (m_tRunStageInfo.m_PhaseRunstageInfo[nNextStageIndex].m_bPedPhaseStartTransitInCurStage[nRingIndex])	//�¸�������λ����һ���׶ι��ɵ�ɫ
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
		//��λ����һ���׶�
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
			//������λ�ĹصƵ�ʱ��͹ض���λ���̵�ʱ������Ϊ0
			m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_wPhaseGreenTime = 0;
			m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_wPedPhaseGreenTime = 0;
		}
		
		m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "In TransStage, RingIndex=%d PhaseIndex=%d StageIndex=%d PhaseGreenTime=%d PedPhaseGreenTime=%d.", 
													nRingIndex, nPhaseIndex,nNextStageIndex, m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_wPhaseGreenTime,
													m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_wPedPhaseGreenTime);
	}
}

/*==================================================================== 
������ ��RecalculteStageTimeByDelayTimeAndDuration
���� �������ӳ�ʱ��ͳ���ʱ�����¼����ֶ�����ʱ���̵�ʱ��
�㷨ʵ�� �� 
����˵�� ��tValidManualCmd���ֶ�����
����ֵ˵������
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 �º���          �������� 
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

				if (tValidManualCmd.m_nSubCtlMode == CTL_MANUAL_SUBMODE_PANEL_STEPFOWARD && tAscManualPanel.m_byMinGreenIgnore == 1)//������С��
				{
					nGreenTime = tRunCounter.m_nLampClrTime[i] + tValidManualCmd.m_tStepForwardCmd.m_nDelayTime;
				}

				if (m_tFixTimeCtlInfo.m_nCurStageIndex == nStageIndexTarget)
				{
					nGreenTime = nGreenTime + tValidManualCmd.m_tStepForwardCmd.m_nDurationTime;//�׶�פ�������ϳ���ʱ��
				}

				pPhaseInfo->m_wPhaseTime = nGreenTime + 
					pPhaseInfo->m_tPhaseParam.m_byGreenFlash + pPhaseInfo->m_tPhaseParam.m_byPhaseYellowChange + 
					pPhaseInfo->m_tPhaseParam.m_byPhaseRedClear;

				//�ض���λ���̵Ƽ����Ѿ����յ��ض���λʱ�����
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
				//��������ʱ����������С�̣���������
				if (m_tFixTimeCtlInfo.m_nCurStageIndex == nStageIndexTarget)
				{
					if (tValidManualCmd.m_tStepForwardCmd.m_nDurationTime == 0)
					{
						//��ס���������ֵ
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
					//������λ�ĹصƵ�ʱ��͹ض���λ�ĺ��ʱ������Ϊ0
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
������ ��RecalculteCurPhaseGreenTimeAndSetStatus
���� �����¼����ֶ�����ʱ���̵�ʱ���״̬
�㷨ʵ�� �� 
����˵�� ��tValidManualCmd���ֶ�����
����ֵ˵������
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 �º���          �������� 
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
	
	//�������ǲ��������㵱ǰ�׶ζ�Ӧ����λ���̵�ʱ��
	if (!tValidManualCmd.m_bStepForwardCmd && m_tOldValidManualCmd.m_bStepForwardCmd && m_nManualCurStatus != MANUAL_STAGE_TRANS &&
		(tValidManualCmd.m_bDirectionCmd || tValidManualCmd.m_bPatternInterruptCmd || tValidManualCmd.m_bChannelLockCmd))//�ڲ���ʱ�����������������ǲ�������ģʽ
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
	else if (tValidManualCmd.m_bStepForwardCmd && m_tOldValidManualCmd.m_bStepForwardCmd &&//���������������㵱ǰ�׶ζ�Ӧ����λ���̵�ʱ��
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
		else //����һ�ΰ���������ס����
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
		     tValidManualCmd.m_nSubCtlMode == CTL_MANUAL_SUBMODE_PANEL_STEPFOWARD)//��ͨ������������
	{
		if (m_nManualCurStatus == MANUAL_STAGE_TRANS)
		{
			m_nManualCurStatus = MANUAL_CONTROL_STATUS;
		}

		PanelFirstStepWard(nStageIndexTarget);
		memcpy(&m_tOldValidManualCmd, &tValidManualCmd, sizeof(tValidManualCmd));
		nRefreshStageTime = FIRSTPANEL_TO_FIRSTSTEPWARD;
	}
	else if (tValidManualCmd.m_bStepForwardCmd  && !m_tOldValidManualCmd.m_bStepForwardCmd)//�ӷǲ��������������ù����·���ָ�Ŀǰֻ���������
	{
		//�ӹ���״̬�ٴ��յ�ƽָ̨����½������״̬
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
				//��尴�˷����ǻ�û�����е������ְ����˲�����ť����Ҫ�����Щ��Ϊ��ǰͨ���ͷ���һ����Ҫ���������̵ı�־
				memset(m_bKeepGreenChannelBeforeControlChannelFlag, false, sizeof(m_bKeepGreenChannelBeforeControlChannelFlag));
				tValidManualCmd.m_nSubCtlMode = CTL_MANUAL_SUBMODE_PANEL_STEPFOWARD;
			}
		}

		memcpy(&m_tOldValidManualCmd, &tValidManualCmd, sizeof(tValidManualCmd));
		nRefreshStageTime = NONSTEPWARD_TO_STEPWARD;
	}
	else if ((m_tOldValidManualCmd.m_nSubCtlMode == CTL_MANUAL_SUBMODE_PANEL_FITSTMANUAL || m_tOldValidManualCmd.m_nSubCtlMode == CTL_MANUAL_SUBMODE_SYSTEM_CHANNEL_LOCK) &&//��ͨ�������е����������
		    (tValidManualCmd.m_bDirectionCmd || //������ֶ��е�����
		     tValidManualCmd.m_bPatternInterruptCmd))//������ֶ��е�����
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
		    (tValidManualCmd.m_bChannelLockCmd || //���з����Ŀ��Ʒ�ʽ�£��·���ͨ������ָ���Ĭ��ģʽ�е�����ָ��
		     tValidManualCmd.m_bPatternInterruptCmd))//���з����Ŀ��Ʒ�ʽ�£��·���ͨ������ָ���û���е�ͨ������ָ����·��˷�����Ԥ
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
		//��������Ժ��Լ��ص�����,ͨ�����������Ժ��Լ��ص�����
		if (tValidManualCmd.m_bPatternInterruptCmd  && (m_tOldValidManualCmd.m_bDirectionCmd || m_tOldValidManualCmd.m_bChannelLockCmd))
		{
			m_nManualCurStatus = MANUAL_STAGE_TRANS;
		 	
			memcpy(&m_tOldValidManualCmd, &tValidManualCmd, sizeof(tValidManualCmd));
			m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "DirectionToSelf, Set Trans Status");
		}

		//����巽�������
		if (tValidManualCmd.m_bPatternInterruptCmd && m_tOldValidManualCmd.m_nSubCtlMode == CTL_MANUAL_SUBMODE_PANEL_DIRECTION &&
			m_nManualCurStatus == MANUAL_STAGE_TRANS)
		{
			memcpy(&m_tOldValidManualCmd, &tValidManualCmd, sizeof(tValidManualCmd));
			m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "In DirectionToSelf, Start Trans");
		}
		//��ͨ������������
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
			tValidManualCmd.m_bStepForwardCmd = false;//����ֶ�ָ��º󣬵�һ�ΰ��²�����Ҫ���ڵ�ǰ״̬�����Բ���״̬Ҫ�����
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
			m_bClearPulseFlag = false;//�е�ͨ����������ʱ���п��ܸ��¸���λ�ĺ������Ѿ����������ǻ�û�����ü����
		}
	}
}

/*==================================================================== 
������ ��InitNextStagePhase
���� ����ʼ����һ���׶ε���λ
�㷨ʵ�� �� 
����˵�� ��nRingIndex������,nPhaseIndex����λ�ţ�nNextStageIndex����һ���׶α�ţ�bChangeStageFlag��ͬʱ�н׶κ�����λ�ı�־
����ֵ˵������
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ����Ƽ          �������� 
====================================================================*/ 
void CLogicCtlManual::InitNextStagePhase(int nRingIndex, int nPhaseIndex, int nNextStageIndex, bool bChangePhaseFlag)
{
	if (bChangePhaseFlag)//�н׶Σ�����λ
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
������ ��CreateManualCmdReturnToSelf
���� �����ɻص��������ֶ�����
�㷨ʵ�� �� 
����˵�� ��tValidManualCmd���ֶ�ָ��
����ֵ˵������
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ����Ƽ          �������� 
====================================================================*/ 
void CLogicCtlManual::CreateManualCmdReturnToSelf(TManualCmd tValidManualCmd)
{
	TManualCmd tManualCmd;
	memset(&tManualCmd,0,sizeof(TManualCmd));
	tManualCmd.m_bNewCmd = true;
	tManualCmd.m_nCmdSource = tValidManualCmd.m_nCmdSource;//������Դ��������ΪLogicManager�������ָ���ϵͳָ���ʱ���Ǹ�������Դ���жϵ�
	tManualCmd.m_nCurCtlSource = tValidManualCmd.m_nCurCtlSource;//����Դ����
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
������ ��SetTransStatus
���� �����ù���״̬
�㷨ʵ�� �� 
����˵�� ��tValidManualCmd���ֶ�ָ��
����ֵ˵������
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ����Ƽ          �������� 
====================================================================*/ 
void  CLogicCtlManual::SetTransStatus(TManualCmd  & tValidManualCmd)
{
	int	nGreenPhaseCount = 0;
	int nClosePhaseCount = 0; 
	CheckIfStageHavePhaseInGreen(nGreenPhaseCount, nClosePhaseCount);
	if (nGreenPhaseCount > 0)
	{
		//�ж����ڹ��ɵ���λ���̵��Ƿ�ﵽ�л����������������ǲ�������ģʽʱ�������ù�����ʱ���ģ���������ʱ�����л���ɫ��������С��
		if (CheckIfNeedToStepForwardToNextStage(tValidManualCmd))
		{
			m_nManualCurStatus = MANUAL_STAGE_TRANS;
			
			//ͨ������ʱ,��ʱ�Ȳ�����ָ��
			if (!tValidManualCmd.m_bChannelLockCmd)
			{
				memcpy(&m_tOldValidManualCmd, &tValidManualCmd, sizeof(tValidManualCmd));//�����з��������ʱ��ָ���
			}
			m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "StepToNoStep, CheckIfNeedToStepForwardToNextStage, Set Trans Status");
		}	
	}
	else
	{
		m_nManualCurStatus = MANUAL_STAGE_TRANS;
		//ͨ������ʱ,��ʱ�Ȳ�����ָ��
		if (!tValidManualCmd.m_bChannelLockCmd)
		{
			memcpy(&m_tOldValidManualCmd, &tValidManualCmd, sizeof(tValidManualCmd));//�����з��������ʱ��ָ���
		}
		m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "StepToNoStep, Non Green, Set Trans Status");
	}

	ReSetNeglectPhaseStageRunTime();
}

/*==================================================================== 
������ ��ProcessFirstManualProcessFirstManual
���� �������һ�ΰ����ֶ���尴ť���������״̬
�㷨ʵ�� �� 
����˵�� ��tValidManualCmd���ֶ�ָ��
����ֵ˵������
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ����Ƽ          �������� 
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
		memcpy(&m_tOldValidManualCmd, &tValidManualCmd, sizeof(tValidManualCmd));//�ֶ��в��������������ʱ��ָ���
	}

	m_nManualCurStatus = MANUAL_STAGE_TRANS;

	tValidManualCmd.m_bNewCmd = false;
	m_pOpenATCRunStatus->SetValidManualCmd(tValidManualCmd);

	m_tOldValidManualCmd.m_bNewCmd = false;

	m_nReturnAutoCtrlStageIndex = -1;//���֮ǰ��ϵͳ�����������׶Σ�����һ�ΰ����ֶ���Ҫ��Ĭ��ֵ��

	ReSetNeglectPhaseStageRunTime();
}

/*==================================================================== 
������ ��ProcessPhasePassControlStatus
���� ��������λ���п���״ֵ̬
�㷨ʵ�� �� 
����˵�� ����
����ֵ˵������
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ����Ƽ          �������� 
====================================================================*/
void CLogicCtlManual::ProcessPhasePassControlStatus(TManualCmd  & tValidManualCmd)
{
	memset(&m_tPhasePassCmdPhaseStatusFromUser, 0, sizeof(m_tPhasePassCmdPhaseStatusFromUser));
	m_pOpenATCRunStatus->GetPhasePassCmdPhaseStatus(m_tPhasePassCmdPhaseStatusFromUser);

	// ϵͳ���Ƶ�����£���λ�ضϿ�����Ч
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

			//���±��ص���λ����״̬��(��ǰ�׶����е���λ)
			for (nRingIndex=0; nRingIndex<m_tFixTimeCtlInfo.m_nRingCount; nRingIndex++)
			{
				ptRingRunInfo = &(m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex]);
				nIndex = m_tFixTimeCtlInfo.m_nCurPhaseIndex[nRingIndex];
				nStageIndex = m_tFixTimeCtlInfo.m_nCurStageIndex;

				nPhaseID = ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhaseNumber;
				nPhaseIDArr[nRingIndex] = nPhaseID;
			
				//�·����û�ָ���������ǰ���н׶ε���λ
				if (m_tPhasePassCmdPhaseStatusFromUser.m_bUpdatePhasePassStatus[nPhaseID - 1])
				{
					TAscStepCfg tStepCfg;
					memset(&tStepCfg, 0, sizeof(tStepCfg));
					m_pOpenATCParameter->GetStepInfo(tStepCfg);

					//�����̵�״̬�µ�ָ��
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
								//�̵�ʱ�յ��ر����е�ָ����̵�ʱ���Ϊ��С��
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
						//������λ����״̬��
						m_tPhasePassCmdPhaseStatus.m_nPhasePassStatus[nPhaseID - 1] = m_tPhasePassCmdPhaseStatusFromUser.m_nPhasePassStatus[nPhaseID - 1];
						m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "ProcessPhasePassControlStatus (light green) RingIndex:%d Index:%d PhaseGreenTime:%d", nRingIndex, nIndex, ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPhaseGreenTime);
						continue;
					}

					//���������ͻƵ�״̬��ָ��
					if (ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseStage == C_CH_PHASESTAGE_GF || 
						ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseStage == C_CH_PHASESTAGE_Y ||
						ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseStage == C_CH_PHASESTAGE_R ||
						ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseStage == C_CH_PHASESTAGE_F)
					{
						if (tStepCfg.m_byStepType == STEP_COLOR)
						{
							m_pOpenATCRunStatus->SetIncludedCurPhaseInPhaseStatusCmdInStepColorFlag(true);
						}
						//������λ����״̬��
						m_tPhasePassCmdPhaseStatus.m_nPhasePassStatus[nPhaseID - 1] = m_tPhasePassCmdPhaseStatusFromUser.m_nPhasePassStatus[nPhaseID - 1];
						continue;
					}

					//������״̬��ָ��
					if (ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseStage == C_CH_PHASESTAGE_U)
					{
						//�û��·���λ����ָ����ж�ʣ��ʱ���Ƿ��㹻��С��+���ɵ�ɫʱ��
						if (m_tPhasePassCmdPhaseStatusFromUser.m_nPhasePassStatus[nPhaseIDArr[nRingIndex] - 1] == PhasePassStatus_Normal)
						{
							nStageTime = m_nStageTimeForPhasePass[nRingIndex];	

							//����һ���׶���ȷʱ���жϵ�ǰ��λ�Ƿ��׶�
							if (m_nManualCurStatus == MANUAL_CONTROL_STATUS && nStageIndex != m_nNextStageIndex)
							{
								if (nPhaseIDArr[nRingIndex] != m_tRunStageInfo.m_PhaseRunstageInfo[m_nNextStageIndex].m_nConcurrencyPhase[nRingIndex])
								{
									//����׶�
								}
								else
								{
									//��׶Σ�������һ���׶εĽ׶�ʱ��
									nStageTime = nStageTime + m_tRunStageInfo.m_PhaseRunstageInfo[m_nNextStageIndex].m_nStageEndTime - m_tRunStageInfo.m_PhaseRunstageInfo[m_nNextStageIndex].m_nStageStartTime;
								}
							}

							nPhaseRemain = nStageTime - (m_nStageRunTime[nRingIndex] + tRunCounter.m_nLampClrTime[nRingIndex]);
							nMinPhaseTime = ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_wPhaseMinimumGreen + ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byGreenFlash + 
												ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhaseYellowChange + ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhaseRedClear;

							//��ǰ��λ����׶ε�ʱ����λʣ������ʱ��С����С��ʱ����Ϊ���ڹ��ɣ�ָ����Ч
							if (nPhaseRemain < nMinPhaseTime)
							{
								if (tStepCfg.m_byStepType == STEP_COLOR)
								{
									m_pOpenATCRunStatus->SetIncludedCurPhaseInPhaseStatusCmdInStepColorFlag(true);
								}
								//������λ����״̬��
								m_tPhasePassCmdPhaseStatus.m_nPhasePassStatus[nPhaseID - 1] = m_tPhasePassCmdPhaseStatusFromUser.m_nPhasePassStatus[nPhaseID - 1];
								ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseStage 	= C_CH_PHASESTAGE_F;
								ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPedPhaseStage 	= C_CH_PHASESTAGE_F;
								continue;
							}
							else
							{						
								//���ݽ׶�ʣ��ʱ�������¼����̵�ʱ��
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
								//������λ����״̬��
								m_tPhasePassCmdPhaseStatus.m_nPhasePassStatus[nPhaseID - 1] = m_tPhasePassCmdPhaseStatusFromUser.m_nPhasePassStatus[nPhaseID - 1];

								m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "ProcessPhasePassControlStatus (light red) RingIndex:%d Index:%d PhaseGreenTime:%d m_nStageTimeForPhasePass:%d nPhaseRemain:%d nMinPhaseTime:%d", nRingIndex, nIndex, ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPhaseGreenTime, m_nStageTimeForPhasePass, nPhaseRemain, nMinPhaseTime);
							}
						}
						//�û��·���λ�ر�����ָ��
						else
						{
							if (tStepCfg.m_byStepType == STEP_COLOR)
							{
								m_pOpenATCRunStatus->SetIncludedCurPhaseInPhaseStatusCmdInStepColorFlag(true);
							}
							//������λ����״̬��
							m_tPhasePassCmdPhaseStatus.m_nPhasePassStatus[nPhaseID - 1] = m_tPhasePassCmdPhaseStatusFromUser.m_nPhasePassStatus[nPhaseID - 1];
						}
					}
				}
			}

			//���±��ص���λ����״̬��(�ǵ�ǰ�׶����е���λ)
			bool bCurStagePhaseFlag	= false;
			for (nPhaseIndex=0; nPhaseIndex<MAX_PHASE_COUNT; nPhaseIndex++)
			{
				//�Ƿ���Ҫ���·���״̬
				if (m_tPhasePassCmdPhaseStatusFromUser.m_bUpdatePhasePassStatus[nPhaseIndex])
				{
					//�Ƿ�ǰ�׶����е���λ
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

					// �ǵ�ǰ���н׶ε���λ
					if (!bCurStagePhaseFlag)
					{
						m_tPhasePassCmdPhaseStatus.m_nPhasePassStatus[nPhaseIndex] = m_tPhasePassCmdPhaseStatusFromUser.m_nPhasePassStatus[nPhaseIndex];
					
					}
				}
			}

			// ��ʼ���û�������λ����״̬��
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
	// ���ؿ��Ƶ�����£���λ�ضϿ�����Ч����λĬ��ȫ������
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
������ ��ClearPulse
���� ����������壬������
�㷨ʵ�� �� 
����˵�� ����
����ֵ˵������
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ����Ƽ          �������� 
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
������ ��PanelFirstStepWard
���� ����������һ�β���
�㷨ʵ�� �� 
����˵�� ��nStageIndexTarget��Ŀ��׶�
����ֵ˵������
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ����Ƽ          �������� 
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

		//��ס���������ֵ
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
������ ��NonStepWardToStepWard
���� ������ǲ���������
�㷨ʵ�� �� 
����˵�� ��nStageIndexTarget��Ŀ��׶�
����ֵ˵������
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ����Ƽ          �������� 
====================================================================*/
void CLogicCtlManual::NonStepWardToStepWard(int nStageIndexTarget)
{
	int i = 0;

	TPhaseLampClrRunCounter tRunCounter;
	m_pOpenATCRunStatus->GetPhaseLampClrRunCounter(tRunCounter);

	//��ǰ�׶ξ���Ŀ��׶ξͿ�ס������Ҫ�е���һ���׶�
	if (m_tFixTimeCtlInfo.m_nCurStageIndex == nStageIndexTarget)
	{
		for (i = 0;i < m_tFixTimeCtlInfo.m_nRingCount;i++)
		{
			PTRingCtlInfo ptRingRunInfo = &(m_tFixTimeCtlInfo.m_atPhaseSeq[i]);
			int nIndex = m_tFixTimeCtlInfo.m_nCurPhaseIndex[i];
			PTPhaseCtlInfo pPhaseInfo = (PTPhaseCtlInfo)&(ptRingRunInfo->m_atPhaseInfo[nIndex]);
			int nPhaseID = pPhaseInfo->m_tPhaseParam.m_byPhaseNumber;

			//��ס���������ֵ
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
				//�����е���һ���׶�
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
				//�ǲ�����������ʱ�䲻ȷ�����������޷�����
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
������ ��FirstPanelManualSwitchToDirectionOrPattern
���� �������һ������ֶ���ť�з��������
�㷨ʵ�� �� 
����˵�� ��tValidManualCmd���ֶ�����
����ֵ˵������
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ����Ƽ          �������� 
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
				// �жϵ�ǰ��λ�Ƿ����һ���׶κ͵�ǰ�׶�
				if (m_tRunStageInfo.m_PhaseRunstageInfo[nCurStageIndex].m_nConcurrencyPhase[i] == m_tRunStageInfo.m_PhaseRunstageInfo[nStageIndexTarget].m_nConcurrencyPhase[i])
				{
					if (tValidManualCmd.m_bDirectionCmd)//�����з��򣬿�׶ε���λ��Ҫ�ڵ�ǰ�׶����̹�����
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
				//����ʼǰ����������ɫ��ͨ��
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
������ ��BackUpPhaseTime
���� ��������λʱ��
�㷨ʵ�� �� 
����˵�� ��nRefreshStageTime��������Դ
����ֵ˵������
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ����Ƽ          �������� 
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
������ ��AdjustPhaseGreenTime
���� ���޸Ļ�������λ�̵�ʱ�䣬��֤���ƴ����뿪ʼ
�㷨ʵ�� �� 
����˵�� ��nRingIndex������� 
           nIndex����λ��� 
		   nStageIndexTarget��Ŀ��׶α�� 
		   nGreenTimeFlag���̵����� 1����С�� 2��������������̵�ʱ�� 3���̵�����
����ֵ˵������λʱ���Ƿ��е���
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ����Ƽ          �������� 
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
		//Ŀǰֻ�е�һ�ΰ�������ֶ��Ժ󣬲Ž���÷�֧����ʱ�̵�ʱ���ò�����������̵�ʱ��
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
������ ��AdjustPedPhaseGreenTime
���� ���޸�������λ�̵�ʱ�䣬��֤���ƴ����뿪ʼ
�㷨ʵ�� �� 
����˵�� ��nRingIndex������� 
           nIndex����λ��� 
		   nStageIndexTarget��Ŀ��׶α�� 
		   nGreenTimeFlag���̵����� 1����С�� 2��������������̵�ʱ�� 3���̵�����
����ֵ˵����������λʱ���Ƿ��е���
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ����Ƽ          �������� 
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
		//Ŀǰֻ�е�һ�ΰ�������ֶ��Ժ󣬲Ž���÷�֧����ʱ�̵�ʱ���ò�����������̵�ʱ��
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
������ ��SetOverlapPhaseLampClr
���� �����ø�����λ�ĵ�ɫ
�㷨ʵ�� �� 
����˵�� ��nNextStageIndex,��һ���׶α��
����ֵ˵����
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ������          �������� 
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

		// �ȴ�������������λ
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
						// ��ǰ�׶����е�ĸ��λ�����̣������̣���ǰ�׶����е���λΪ������λ��������ķǵ�һ��ĸ��λ��������λ�Ѿ����̣��������
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
						// ��ǰ�׶ε���λ����һ���׶���λid��һ�£��Ҷ��ǵ�ǰ������λ��ĸ��λ������һ���׶ε���λ�������У��������
						if (!IsNeglectPhase(nNextPhaseID) && IsPhaseNumInOverlap(nNextPhaseID, i) && m_tPhasePassCmdPhaseStatus.m_nPhasePassStatus[nNextPhaseID-1] == PhasePassStatus_Normal)
						{
							for (n = 0;n < m_tRunStageInfo.m_PhaseRunstageInfo[nStageIndex].m_nConcurrencyPhaseCount;n++)
							{
								nPhaseID = m_tRunStageInfo.m_PhaseRunstageInfo[nStageIndex].m_nConcurrencyPhase[n];
								if (IsPhaseNumInOverlap(nPhaseID, i) && (m_tPhasePassCmdPhaseStatus.m_nPhasePassStatus[nPhaseID-1] == PhasePassStatus_Normal ||
									m_pOpenATCRunStatus->GetIncludedCurPhaseInPhaseStatusCmdInStepColorFlag()))//ɫ��ģʽ���·��Ĺض�ָ���������ǰ���н׶ε���λ���������λ������
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
						//�û�����ʱ���˷��������ϵͳ����ʱ�·���ͨ������ָ��������λ��ʹ������һ���׶ε���λ��Ҳ��Ҫ�͵�ǰ�׶ε���λһ����ɵ�ɫ
						if (m_bIsUsrCtl || m_bIsSystemCtl)
						{
							TManualCmd  tValidManualCmd;
							memset(&tValidManualCmd,0,sizeof(tValidManualCmd));
							m_pOpenATCRunStatus->GetValidManualCmd(tValidManualCmd);
							if (tValidManualCmd.m_bDirectionCmd || tValidManualCmd.m_bChannelLockCmd)
							{
								bGreenFlag = false;
							}
							
							//����λ�������ڹ���׼���е���巽��Ĺ����У�������λ�����������һ����λ�������
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

		// �ٴ������������λ
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
									m_pOpenATCRunStatus->GetIncludedCurPhaseInPhaseStatusCmdInStepColorFlag()))//ɫ��ģʽ���·��Ĺض�ָ���������ǰ���н׶ε�������λ�������˸�����λ������
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
						//�û�����ʱ���˷��������ϵͳ����ʱ�·���ͨ������ָ��������λ��ʹ������һ���׶ε���λ��Ҳ��Ҫ�͵�ǰ�׶ε���λһ����ɵ�ɫ
						if (m_bIsUsrCtl || m_bIsSystemCtl)
						{
							TManualCmd  tValidManualCmd;
							memset(&tValidManualCmd,0,sizeof(tValidManualCmd));
							m_pOpenATCRunStatus->GetValidManualCmd(tValidManualCmd);
							if (tValidManualCmd.m_bDirectionCmd || tValidManualCmd.m_bChannelLockCmd)
							{
								bPedGreenFlag = false;
							}

							//����λ�������ڹ���׼���е���巽��Ĺ����У�������λ�����������һ����λ�������
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
������ ��InitParamBeforeChannelLock
���� ��
�㷨ʵ�� �����л�����֮ǰ�ȳ�ʼ������
����˵�� ��tCtlCmd���ֶ��������
����ֵ˵������
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ����Ƽ          �������� 
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

	//��ȡ��ǰͨ����Ҫ�л���Ŀ���ɫ
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

	//��ȡ��ǰͨ���ĵ�ɫ
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
������ ��ProcessChannelLock
���� ������ͨ��������
�㷨ʵ�� �� 
����˵�� ��tValidManualCmd���ֶ�����
����ֵ˵������
 ----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ����Ƽ          �������� 
====================================================================*/ 
void  CLogicCtlManual::ProcessChannelLock(TManualCmd & tValidManualCmd)
{
	if (tValidManualCmd.m_bNewCmd)
	{
		//����ͨ�������Ժ󣬸���������ָ������̵�ʱ��
		if (tValidManualCmd.m_bPatternInterruptCmd ||//����ͨ�������Ժ󣬵����������������Ʒ�ʽ
			tValidManualCmd.m_bChannelLockCmd)//����ͨ�������Ժ����·�ͨ������ָ��
		{
			//ͨ������1�е�ͨ������2��ͨ������1��û�й��ɵ���С�̣�����ͨ������1���̵�ʱ�仹�����̵ƹ���ʱ�䣬��������̵�Ӧ����С��
			//ͨ������1���ڹ��ɵ�ɫ(�������ƣ���)������ͨ������1����ͨ������1���꣬�ٻص�ͨ������1
			if (tValidManualCmd.m_bChannelLockCmd && memcmp(tValidManualCmd.m_tChannelLockCmd.m_tNextChannelLockCtrlCmd.m_nChannelLockStatus, 
			    m_tOldValidManualCmd.m_tChannelLockCmd.m_tNextChannelLockCtrlCmd.m_nChannelLockStatus, MAX_CHANNEL_COUNT) == 0)
			{
				//��ȡ��ǰͨ����Ҫ�л���Ŀ���ɫ
				for (int i = 0;i < MAX_CHANNEL_COUNT;i++)
				{
					if (m_tOldValidManualCmd.m_tChannelLockCmd.m_tNextChannelLockCtrlCmd.m_nChannelLockStatus[i] != CHANNEL_STATUS_DEFAULT &&
						m_chChannelStage[i] == C_CH_PHASESTAGE_G)
					{    
						tValidManualCmd.m_bChannelLockCmd = false;//ͨ������1�е�ͨ������2��ͨ������1��û�й��ɵ���С�̣�����ͨ������1������ͬ��ֵָ�ִ��
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
						m_bTargetLockPhaseChannelFlag[i] = false;//�µ�����ָ���Ҫ���������̵�Ŀ��������λͨ����Ҫ����
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
		else//�տ�ʼ����ͨ������
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

			//��ʼ��ͨ��������������ʼ��ͨ������
			InitParamBeforeChannelLock(tValidManualCmd.m_tChannelLockCmd.m_tNextChannelLockCtrlCmd.m_nChannelLockStatus);
			ChangeChannelClr();

			for (int i = 0;i < MAX_CHANNEL_COUNT;i++)
			{
				m_nChannelLockGreenTime[i] = tValidManualCmd.m_tChannelLockCmd.m_tNextChannelLockCtrlCmd.m_nDuration;	
				m_bChannelKeepGreenFlag[i] = false;
			}

			memcpy(&m_tOldValidManualCmd, &tValidManualCmd, sizeof(tValidManualCmd));

			//��ȡ������λ����
			GetLockPhaseData(tValidManualCmd);
		}

		tValidManualCmd.m_bNewCmd = false;
		m_pOpenATCRunStatus->SetValidManualCmd(tValidManualCmd);
	}

	bool bLockCmd = false;//��һ��ָ���Ƿ�������ָ���־
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
					bFlag = false;//ֻҪ��һ����λ���������Ͳ��ǻָ�������Ϊ���µ�����ָ����Ŀ����λ����Ҫ���ɵ�Ŀ����λ,ֱ���е��µ���λ����
				}
			}
		}
		//���������Ҫ��Ŀ����λ���ɵ�Ŀ����λ:1.��λ��������ʱ�䵽�� 2.����������ť 3.������������λ���˻ָ�(��ʱ���е���λ�������Ͷ��ǻָ�)
		//��Ŀ����λ���ɵ�Ŀ����λ�Ĺ���:Ŀ��������λ��Ϊ��Ҫ�������������̻��߹��ɽ�������Ŀ����λ������Ϊ��Ҫ�������������̻��ߵ�ǰ��λ�л�����
		if (bFlag)
		{
			for (int i = 0; i < MAX_PHASE_COUNT; i++)
			{
				if (m_tLockPhaseData[i].nCurLockPhaseID == 0)
				{
					continue;
				}
				//1.Ŀ����λ���ɽ�������Ŀ����λ���ڻ��ĵ�ǰ��λ��û���л�����
				//2.Ŀ����λ��ΪҪ�������������̣���Ŀ����λ���ڻ��ĵ�ǰ��λ��û���л�����
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
			if (tValidManualCmd.m_bChannelLockCmd)//ͨ���������ɽ����������µ�ͨ������
			{
				memset(m_bNeglectChannelBoforePhaseLock, 0, sizeof(m_bNeglectChannelBoforePhaseLock));
				if (tValidManualCmd.m_bPhaseToChannelLock)
				{
					SetNeglectChannelBoforePhaseLock();
				}

				//��ʼ��ͨ��������������ʼͨ����������
				InitParamBeforeChannelLock(tValidManualCmd.m_tChannelLockCmd.m_tNextChannelLockCtrlCmd.m_nChannelLockStatus);
				ChangeChannelClr();
				for (int i = 0;i < MAX_CHANNEL_COUNT;i++)
				{
					m_nChannelLockGreenTime[i] = tValidManualCmd.m_tChannelLockCmd.m_tNextChannelLockCtrlCmd.m_nDuration;
					m_bChannelKeepGreenFlag[i] = false;
				}
				//����µ�ͨ������ָ���־����Ϊ��ͨ��������ͨ���������Ե�ǰ������ģʽ����ͨ������
				tValidManualCmd.m_bNewCmd = false;
				tValidManualCmd.m_bChannelLockCmd = false;
				memcpy(&m_tOldValidManualCmd.m_tChannelLockCmd.m_tNextChannelLockCtrlCmd, &tValidManualCmd.m_tChannelLockCmd.m_tNextChannelLockCtrlCmd, sizeof(TChannelLockCtrlCmd));
				//memcpy(&tValidManualCmd.m_tDirectionCmd,&m_tOldValidManualCmd.m_tDirectionCmd,sizeof(TDirectionCmd));
				memcpy(&m_tOldValidManualCmd.m_tPhaseLockPara, &tValidManualCmd.m_tPhaseLockPara, sizeof(TPhaseLockPara));
				m_pOpenATCRunStatus->SetValidManualCmd(tValidManualCmd);
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "ProcessChannelLock, TransitChannelLock End, Trans To NewChannelLock GreenTime:%d", tValidManualCmd.m_tChannelLockCmd.m_tNextChannelLockCtrlCmd.m_nDuration);

				//��ȡ������λ����
				GetLockPhaseData(tValidManualCmd);
			}
			else
			{
				//��Ϊ��������������ͨ���������ɽ������޸ĵ�ǰ����ģʽ�Ϳ�����ģʽ
				if (tValidManualCmd.m_bPatternInterruptCmd)
				{
					tValidManualCmd.m_nCtlMode = CTL_MODE_SELFCTL;
					tValidManualCmd.m_nSubCtlMode = CTL_MANUAL_SUBMODE_DEFAULT;
					tValidManualCmd.m_bNewCmd = false;
					m_pOpenATCRunStatus->SetValidManualCmd(tValidManualCmd);
					m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "ProcessChannelLock, TransitChannelLock End, Trans To SelfCtl");
					m_nManualCurStatus = MANUAL_STAGE_TRANS;//�������
				}

				//�·�������ָ����ֶ�ָ�ͨ���������ɽ�����ֱ���޸Ŀ���ģʽ����ģʽ������ָ��
			    memcpy(&m_tOldValidManualCmd, &tValidManualCmd, sizeof(tValidManualCmd));
			}
		}
		else
		{
			if (tValidManualCmd.m_tChannelLockCmd.m_bStepFowardToChannelLock)//�ص�ͨ������ǰ�Ĳ���
			{
				TAscStepCfg tStepCfg;
				memset(&tStepCfg, 0, sizeof(tStepCfg));
				m_pOpenATCParameter->GetStepInfo(tStepCfg);
				//���ɻص�����ǰ�Ĳ���ָ��ص���ͨ������ǰ�Ľ׶ε���һ���׶�
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

				//��ʼ����һ���׶ε���λ
				for (int i = 0;i < m_tFixTimeCtlInfo.m_nRingCount;i++)
				{
					InitNextStagePhase(i, m_tFixTimeCtlInfo.m_nCurPhaseIndex[i], tValidManualCmd.m_tChannelLockCmd.m_nTargetStageIndex, true);
				}

				THWPanelBtnStatus tHWPanelBtnStatus;
				tHWPanelBtnStatus.m_nHWPanelBtnIndex = HWPANEL_BTN_STEP;
				tHWPanelBtnStatus.m_bHWPanelBtnStatus = true;
				m_pOpenATCRunStatus->SetPanelBtnStatusList(tHWPanelBtnStatus);
			}
			else //�û���Ԥ��ʱ���ص�����
			{
				m_nManualCurStatus = MANUAL_STAGE_TRANS;//��Ҫ��Ϊ����״̬
                CreateManualCmdReturnToSelf(tValidManualCmd);//���ɻص����������COpenATCLogicCtlManager���Ը����������������������ص�����
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "ProcessChannelLock, ChannelLock Duration End, CreateManualCmdReturnToSelf");
			}
		}
	}
	else
	{
		//����ʧ�ܵ�ԭ���У�1.ͬ�׶ε�������λ�ĵ�ɫ���ڹ���
		//2.Ŀ����λ�ĵ�ɫһֱ�����̣���Ŀ����λ���ڻ�����λ���ڹ��� 
		//3.Ŀ����λ�ĵ�ɫ�ڹ��ɣ���Ŀ����λ���ڻ�����λ���ڹ���
		SwitchLockChannelPhaseToNext();

		if (m_nManualCurStatus == MANUAL_STAGE_TRANS)
		{
			m_nManualCurStatus = MANUAL_CONTROL_STATUS;
		}

		if (m_nManualCurStatus == MANUAL_CONTROL_STATUS)
		{
			//ͨ������ʱ��Ŀ����λ����������������λ��Ҫ���ɵ�Ŀ����λ���ڽ׶�ʱ���ù���״̬
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
������ ��PatternSwitchToChannelLock
���� ���������Ŀ��Ʒ�ʽ�е�ͨ������
�㷨ʵ�� �� 
����˵�� ��tValidManualCmd���ֶ�����
����ֵ˵������
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ����Ƽ          �������� 
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
				// �жϵ�ǰ��λ�Ƿ����һ���׶κ͵�ǰ�׶�
				if (m_tRunStageInfo.m_PhaseRunstageInfo[nCurStageIndex].m_nConcurrencyPhase[i] == m_tRunStageInfo.m_PhaseRunstageInfo[nStageIndexTarget].m_nConcurrencyPhase[i])
				{
					if (tValidManualCmd.m_bChannelLockCmd)//��ͨ����������׶ε���λ��Ҫ�ڵ�ǰ�׶����̹�����
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
				//ͨ��������ʼǰ����������ɫ��ͨ��
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
������ ��GetTransRunStatusBeforeLockPhase
���� ������������λ������״̬
�㷨ʵ�� �� 
����˵�� ��tRunStatus����λ������״̬
����ֵ˵������
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ����Ƽ          �������� 
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
				//��ȡ��ǰ������ͨ���ĵ�ɫ
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
		//��ȡ��ǰ������ͨ���ĵ�ɫ
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
������ ��GetLockPhaseRunStatus
���� ������������λ������״̬
�㷨ʵ�� �� 
����˵�� ��tRunStatus����λ������״̬
����ֵ˵������
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ����Ƽ          �������� 
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
			tRunStatus.m_atRingRunStatus[i].m_atPhaseStatus[j].m_chPhaseLockStatus = 0;//��λ����״̬

			tRunStatus.m_atRingRunStatus[i].m_atPhaseStatus[j].m_chPhaseStatus = C_CH_PHASESTAGE_R;
			tRunStatus.m_atRingRunStatus[i].m_atPhaseStatus[j].m_chPedPhaseStatus = C_CH_PHASESTAGE_R;

			if (!bInvalidPhaseCmd)
			{
				for (k = 0; k < m_tLockPhaseStage.nLockPhaseCount;k++)
				{
					if (tRunStatus.m_atRingRunStatus[i].m_atPhaseStatus[j].m_byPhaseID == m_tLockPhaseStage.nLockPhaseID[k])
					{
						tRunStatus.m_atRingRunStatus[i].m_nCurRunPhaseIndex = j;//��ǰ������λ����
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
						tRunStatus.m_atRingRunStatus[i].m_nCurRunPhaseIndex = j;//��ǰ������λ����
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

			//��ȡ��ǰ������ͨ���ĵ�ɫ
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
				tRunStatus.m_atRingRunStatus[i].m_nCurRunPhaseIndex = nLockPhaseIndex[i];//��ǰ������λ����
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

				//��ȡ��ǰ����ͨ���ĵ�ɫ
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
					   tRunStatus.m_atRingRunStatus[nRingIndex].m_nCurRunPhaseIndex = nOverlapLockPhaseIndex[nRingIndex];//��ǰ������λ����
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
������ ��SetKeepGreenChannelBeforeControlChannel
���� ��ͨ�����ƿ�ʼǰ����������ɫ��ͨ��
�㷨ʵ�� �� 
����˵�� ��byGreenStageVehPhaseID, ��ǰ������ɫ�׶εĻ�������λID
           byGreenStagePedPhaseID����ǰ������ɫ�׶ε�������λID
           tValidManualCmd����Ч�Ŀ���ָ��
����ֵ˵������
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ����Ƽ          �������� 
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
������ ��CheckSendGreenPulse
���� ������������ܷ񷢳�
�㷨ʵ�� �� 
����˵�� ��byPhaseNumber����λID
����ֵ˵������
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ����Ƽ          �������� 
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
������ ��PhaseTransBasedOnControlChannelFlag
���� ������ͨ�������л�ʱ�ļ��������̵Ʊ�־�����ɵ�ǰ��λ
�㷨ʵ�� �� 
����˵�� ����
����ֵ˵������
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ����Ƽ          �������� 
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
				// �жϵ�ǰ��λ�Ƿ����һ���׶κ͵�ǰ�׶�
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
������ ��IsChannelNeedTranClr
���� ����ǰͨ���ĵ�ɫ�Ƿ���Ҫ����
�㷨ʵ�� �� 
����˵�� �� byChannelIndex��ͨ�����
����ֵ˵����true����Ҫ����
            false������Ҫ����
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ����Ƽ          �������� 
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
������ ��ProcessAllClosePhaseInCurStage
���� ������ǰ�׶�������λ���ض��Ժ󣬿�����һ���׶�
�㷨ʵ�� �� 
����˵�� �� tValidManualCmd���ֶ����nNextStageIndex��Ŀ��׶κ�
����ֵ˵������
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ����Ƽ          �������� 
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

				if (m_tRunStageInfo.m_PhaseRunstageInfo[nNextStageIndex].m_bPhaseStartTransitInCurStage[i])		//�¸���λ����һ���׶ι��ɵ�ɫ
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
				
				if (m_tRunStageInfo.m_PhaseRunstageInfo[nNextStageIndex].m_bPedPhaseStartTransitInCurStage[i])	//�¸�������λ����һ���׶ι��ɵ�ɫ
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
������ ��IsNeedTransBeforeControlChannel
���� ����ͨ������֮ǰ�Ƿ���Ҫ������λ
�㷨ʵ�� �� 
����˵�� �� nPhaseID����λID
����ֵ˵����true����������λ
            false������������λ
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ����Ƽ          �������� 
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
������ ��GetPhaseGreenTimeAfterLockPhase
���� ����ȡ������λ�����һ���׶ζ�Ӧ����λ��ʱ��
�㷨ʵ�� �� 
����˵�� �� nRingIndex�����ţ�nPhaseIndex����λ�ţ�nNextStageIndex���׶κţ�bChangeStageFlag���н׶κ�����λ��־
����ֵ˵������
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ����Ƽ          �������� 
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

	if (bChangePhaseFlag)																							//��ǰ��λ�������е���һ����λ
	{
		if (m_tRunStageInfo.m_PhaseRunstageInfo[nNextStageIndex].m_bPhaseStartTransitInCurStage[nRingIndex])		//�¸���λ����һ���׶ι��ɵ�ɫ
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

		if (m_tRunStageInfo.m_PhaseRunstageInfo[nNextStageIndex].m_bPedPhaseStartTransitInCurStage[nRingIndex])	//�¸�������λ����һ���׶ι��ɵ�ɫ
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
	//��λ����һ���׶�
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
		//������λ�ĹصƵ�ʱ��͹ض���λ���̵�ʱ������Ϊ0
		m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_wPhaseGreenTime = 0;
		m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_wPedPhaseGreenTime = 0;
	}

	m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "GetPhaseGreenTimeAfterLockPhase, RingIndex=%d PhaseIndex=%d StageIndex=%d PhaseGreenTime=%d PedPhaseGreenTime=%d.", 
												nRingIndex, nPhaseIndex,nNextStageIndex, m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_wPhaseGreenTime,
												m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_wPedPhaseGreenTime);

}

/*==================================================================== 
������ ��GetLockPhaseData
���� ����ȡ������λ����
�㷨ʵ�� �� 
����˵�� �� tValidManualCmd���ֶ�����
����ֵ˵������
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ����Ƽ          �������� 
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
							m_tLockPhaseData[nIndex].nCurLockStageIndex = j;//��λ��׶�ʱ����¼���һ���׶α��
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

	//�໷ֻ��һ��������λ���Ҹ�������λ��׶Σ���ǰ�׶α���Կ�ĵ�һ���׶α��Ϊ׼
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

	//��ȡ������λ��Ӧ��ͨ��
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

	//�ҳ�������λ����С���׶α�ţ���Ŀ����λ��Ŀ��׶α��
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

	if (nMinPhaseIndex != nMaxPhaseIndex)//������������ͬ�׶���λ3����λ7,��λ3��¼�Ľ׶α����4����λ7��4,5�׶Σ����Ǽ�¼�Ľ׶α����5����������λ7�Ľ׶α�Ÿ�Ϊ4
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
					//ͬ�׶���λ
					bFlag = true;
					nStageIndex = i;
				}
			}
		}

		if (bFlag)//ͬ�׶���λ
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

			tValidManualCmd.m_tChannelLockCmd.m_nTargetStageIndex = nStageIndex;//�޸�Ŀ��׶α��

			GetNextPhaseAfterLockEnd(true, nCount);//������ͬ����ͬ�׶ε���λ

			m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "GetLockPhaseData LockPhaseInSameStage StageIndex:%d", nStageIndex);
		}
		else//��ͬ�׶���λ
		{
			for (i = 0; i < nCount; i++)
			{
				m_tLockPhaseData[i].nTargetStageIndex = m_tLockPhaseData[nMaxPhaseIndex].nCurLockStageIndex;

				if (i == nMinPhaseIndex)
				{
					m_tLockPhaseData[i].bNeedTransFlag = true;//��С�׶α�ŵ���λ�����ȿ�ʼ���ɱ�־
					m_tLockPhaseData[i].nTargetPhaseIDInCurRing = m_tRunStageInfo.m_PhaseRunstageInfo[m_tLockPhaseData[i].nTargetStageIndex].m_nConcurrencyPhase[m_tLockPhaseData[i].nCurLockRingIndex];
				}
				else
				{
					m_tLockPhaseData[i].bNeedTransFlag = false;
					m_tLockPhaseData[i].nTargetPhaseIDInCurRing = m_tRunStageInfo.m_PhaseRunstageInfo[m_tLockPhaseData[i].nTargetStageIndex].m_nConcurrencyPhase[m_tLockPhaseData[i].nCurLockRingIndex];
				}

				m_tLockPhaseData[i].bSwitchSuccessFlag = false;
				m_tLockPhaseData[i].bLockPhaseInSameStageFlag = false;

				tValidManualCmd.m_tChannelLockCmd.m_nTargetStageIndex = m_tLockPhaseData[nMaxPhaseIndex].nCurLockStageIndex;//�޸�Ŀ��׶α��
			}

			GetNextPhaseAfterLockEnd(false, nCount);//������ͬ���Ĳ�ͬ�׶ε���λ

			m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "GetLockPhaseData LockPhaseInDifferentStage TargetStageIndex:%d", m_tLockPhaseData[nMaxPhaseIndex].nCurLockStageIndex);
		}
	}
	else//ͬ�׶���λ
	{
		for (i = 0; i < nCount; i++)
		{
			m_tLockPhaseData[i].nTargetStageIndex = m_tLockPhaseData[i].nCurLockStageIndex;
			m_tLockPhaseData[i].nTargetPhaseIDInCurRing = m_tRunStageInfo.m_PhaseRunstageInfo[m_tLockPhaseData[i].nTargetStageIndex].m_nConcurrencyPhase[m_tLockPhaseData[i].nCurLockRingIndex];

			tValidManualCmd.m_tChannelLockCmd.m_nTargetStageIndex = m_tLockPhaseData[i].nTargetStageIndex;//�޸�Ŀ��׶α��

			m_tLockPhaseData[i].bNeedTransFlag = false;
			m_tLockPhaseData[i].bSwitchSuccessFlag = false;
			m_tLockPhaseData[i].bLockPhaseInSameStageFlag = true;
		}

		GetNextPhaseAfterLockEnd(true, nCount);//ͬ�׶���λ���������������һ���ǵ�������һ����λ��һ���Ƕ໷ֻ����һ����λ

		m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "GetLockPhaseData LockPhaseInSameStage StageIndex:%d", tValidManualCmd.m_tChannelLockCmd.m_nTargetStageIndex);
	}

	//��Ŀ����λ��Ӧͨ�������̱�־
	for (i = 0; i < MAX_PHASE_COUNT; i++)
	{
		if (m_tLockPhaseData[i].nCurLockPhaseID == 0)
		{
			continue;
		}
			
		for (j = 0; j < m_tLockPhaseData[i].nLockChannelCount;j++)
		{
			//Ŀ����λ������Ҫ���ɣ�������λҲ����ͬһ���׶εı�����
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
������ ��GetNextPhaseAfterLockEnd
���� �������������ͬһ���׶Σ���ȡ������λ����һ���׶ζ�Ӧ����λ
       ��������Ĳ���ͬһ���׶Σ���ȡ��Ҫ���ɵ�������λ����һ����λ
�㷨ʵ�� �� 
����˵�� �� true��������λ��ͬһ���׶�
            false��������λ����ͬһ���׶�
			nLockPhaseCount��������λ����
����ֵ˵������
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ����Ƽ          �������� 
====================================================================*/
void CLogicCtlManual::GetNextPhaseAfterLockEnd(bool bInSameStage, int nLockPhaseCount)
{
	int i = 0, j = 0;
	int nCurRingIndex = 0;
	int nCurPhaseIndex = 0;
	int nLockPhaseIndexInRing = 0;
	
	if (bInSameStage)//������λ��ͬһ���׶�
	{
		memset(m_nStageRunTime, 0, sizeof(m_nStageRunTime));

		if (m_tFixTimeCtlInfo.m_nRingCount > 1 && nLockPhaseCount == 1)//�໷ֻ����һ����λ
		{
			bool bDirectTransit = true;//Ĭ��������λ����׶�
			for (i = 0;i < m_tRunStageInfo.m_nRunStageCount;i++)      
			{
				for (j = 0; j < m_tRunStageInfo.m_PhaseRunstageInfo[i].m_nConcurrencyPhaseCount;j++)
				{
					if (m_tRunStageInfo.m_PhaseRunstageInfo[i].m_nConcurrencyPhase[j] == m_tLockPhaseData[0].nCurLockPhaseID)
					{
						if (!m_tRunStageInfo.m_PhaseRunstageInfo[i].m_bDirectTransit[j])
						{
							bDirectTransit = false;//������λ��׶�
						}
					}
				}
			}

			for (i = 0;i < m_tFixTimeCtlInfo.m_nRingCount;i++)
			{
				//������ǰһ���׶���������λ���ڽ׶ι���
				int nCurLockStageIndex = m_tLockPhaseData[0].nCurLockStageIndex - 1;
				if (m_tLockPhaseData[0].nCurLockStageIndex == 0)
				{
					nCurLockStageIndex = m_tRunStageInfo.m_nRunStageCount - 1;
				}

				int nPhaseID = m_tRunStageInfo.m_PhaseRunstageInfo[nCurLockStageIndex].m_nConcurrencyPhase[i];
				GetPhaseIndexByPhaseNumber(nCurRingIndex, nLockPhaseIndexInRing, nPhaseID);

				int nNextStageIndex = m_tLockPhaseData[0].nCurLockStageIndex;//������λ���ڽ׶�

				int nSecNextStageIndex = nNextStageIndex + 1;
				if (nSecNextStageIndex >= m_tRunStageInfo.m_nRunStageCount)
				{
					nSecNextStageIndex = 0;
				}
		
				if (m_tRunStageInfo.m_PhaseRunstageInfo[nCurLockStageIndex].m_nConcurrencyPhase[i] == m_tRunStageInfo.m_PhaseRunstageInfo[nNextStageIndex].m_nConcurrencyPhase[i])
				{
					//������λ��Ӧ����������λ��׶Σ���ǰ��λ����������λ��Ӧ����������λ
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
						//������λ��Ӧ����������λ����׶Σ���ǰ��λ��������λ��Ӧ����������λ�������λ
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

				m_tFixTimeCtlInfo.m_nCurStageIndex = m_tLockPhaseData[0].nCurLockStageIndex;//ת��������λ���ڽ׶�
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
						nNextStageIndex = m_tLockPhaseData[j].nCurLockStageIndex + 1;//������λ����һ���׶�
						if (nNextStageIndex >= m_tRunStageInfo.m_nRunStageCount)
						{
							nNextStageIndex = 0;
						}

						if (m_tRunStageInfo.m_PhaseRunstageInfo[m_tLockPhaseData[j].nCurLockStageIndex].m_nConcurrencyPhase[i] == m_tRunStageInfo.m_PhaseRunstageInfo[nNextStageIndex].m_nConcurrencyPhase[i])
						{
							//������λ��׶Σ���ǰ��λ����������λ
							m_tFixTimeCtlInfo.m_nCurPhaseIndex[i] = nLockPhaseIndexInRing; 	

							m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[m_tFixTimeCtlInfo.m_nCurPhaseIndex[i]].m_chPhaseStage = C_CH_PHASESTAGE_U;
							m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[m_tFixTimeCtlInfo.m_nCurPhaseIndex[i]].m_chPedPhaseStage = C_CH_PHASESTAGE_U;

							GetPhaseGreenTimeAfterLockPhase(i, m_tFixTimeCtlInfo.m_nCurPhaseIndex[i], nNextStageIndex, false);
						}
						else
						{
							//������λ����׶Σ���ǰ��λ��������λ�������λ
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

						m_tFixTimeCtlInfo.m_nCurStageIndex = m_tLockPhaseData[i].nTargetStageIndex + 1;//ת��Ŀ��׶ε���һ���׶�
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
	else//������λ����ͬһ���׶�
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

				//��������λ�������λ��ʼ����
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
������ ��IsTransChannel
���� ���Ƿ��ǹ���ͨ��
�㷨ʵ�� �� 
����˵�� �� byChannelType, ͨ�����ͣ�0��ʾ����ͨ����1��ʾ����ͨ��, 
            nChannelIndex, ͨ�����
����ֵ˵����true���ɹ���
            false�����ɹ���
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ����Ƽ          �������� 
====================================================================*/
bool CLogicCtlManual::IsTransChannel(BYTE byChannelType, int nChannelIndex)
{
	if (byChannelType == CHANNEL_TYPE_LOCK  && m_bTargetLockPhaseChannelFlag[nChannelIndex])//����ͨ����Ŀ��ͨ�������������
	{
		return false;
	}

	return true;
}

/*==================================================================== 
������ ��SwitchLockChannelPhaseToNext
���� ��SwitchLockChannelPhaseToNext�л�������λ����һ����λ
�㷨ʵ�� �� 
����˵�� �� ��
����ֵ˵������
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ����Ƽ          �������� 
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
		//��Ŀ����λ��ͨ��ȫ���������ɽ������ܽ��л�����λ�л�
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
					m_tFixTimeCtlInfo.m_nCurStageIndex = i;//��λ��׶�ʱ�������һ���׶α��
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
		//�Ѿ����е�Ŀ����λ
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

				//�����Ǵ�Ŀ��׶ε�ǰһ���׶���Ŀ��׶ι���
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

			m_tFixTimeCtlInfo.m_nCurStageIndex = m_tLockPhaseData[nCurPhaseIndex].nTargetStageIndex;//ת��Ŀ�껷��������λ�����ڽ׶�
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

			//��Ŀ����λ�ڻ�����λ�л������У��������µ���λ����ָ��,�����ǰ��λ������������ǰ��λ��ɫ������ɫ��������̣�����ɫ����ɵ��ɽ���������Ҫ���е���һ����λ��
			if (memcmp(tValidManualCmd.m_tChannelLockCmd.m_tNextChannelLockCtrlCmd.m_nChannelLockStatus, m_tOldValidManualCmd.m_tChannelLockCmd.m_tNextChannelLockCtrlCmd.m_nChannelLockStatus, MAX_CHANNEL_COUNT) != 0)
			{
				PTRingCtlInfo ptRingRunInfo = &(m_tFixTimeCtlInfo.m_atPhaseSeq[m_tLockPhaseData[nCurRingIndex].nCurLockRingIndex]);
				int nIndex = m_tFixTimeCtlInfo.m_nCurPhaseIndex[m_tLockPhaseData[nCurRingIndex].nCurLockRingIndex];
				PTPhaseCtlInfo pPhaseInfo = (PTPhaseCtlInfo)&(ptRingRunInfo->m_atPhaseInfo[nIndex]);
				if (pPhaseInfo->m_chPhaseStage == C_CH_PHASESTAGE_G)
				{
					//����ɫ���ɵ��ɽ�����ʱ����Ҫ����С��
					//ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPhaseStageRunTime = ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_wPhaseMinimumGreen;
				}
				if (pPhaseInfo->m_chPedPhaseStage == C_CH_PHASESTAGE_G)
				{
					//����ɫ���ɵ��ɽ�����ʱ����Ҫ����С��
					//ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPedPhaseStageRunTime = ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_wPhaseMinimumGreen;
				}

				bool bFlag = false;
				for (int i = 0; i < tValidManualCmd.m_tPhaseLockPara.m_nPhaseLockCount;i++)
				{
					if (tValidManualCmd.m_tPhaseLockPara.m_nPhaseLockType[i] != LOCK_TYPE_CANCEL)
					{
						bFlag = true;//ֻҪ��һ����λ���������Ͳ��ǻָ�������Ϊ���µ�����ָ���Ŀ����λ����Ҫ���е���һ����λ
					}
				}

				if (bFlag)
				{
					return;
				}
			}
		
			//��ǰ����λ�л�����
			if (m_tFixTimeCtlInfo.m_atPhaseSeq[nCurRingIndex].m_atPhaseInfo[m_tFixTimeCtlInfo.m_nCurPhaseIndex[nCurRingIndex]].m_chPhaseStage == C_CH_PHASESTAGE_F &&
				m_tFixTimeCtlInfo.m_atPhaseSeq[nCurRingIndex].m_atPhaseInfo[m_tFixTimeCtlInfo.m_nCurPhaseIndex[nCurRingIndex]].m_tPhaseParam.m_byPhaseNumber !=
			    m_tLockPhaseData[nCurPhaseIndex].nTargetPhaseIDInCurRing)
			{
				//����ת��λ
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
							m_tFixTimeCtlInfo.m_nCurStageIndex = i;//��λ��׶�ʱ�������һ���׶α��
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
������ ��IsTargetPhaseChannel
���� ���ж��Ƿ���Ŀ����λͨ��
�㷨ʵ�� �� 
����˵�� �� nChannelIndex��ͨ�����
����ֵ˵����Ŀ����λͨ����־
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ����Ƽ          �������� 
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
������ ��IsOverlapNeedKeepGreen
���� ��������λ�Ƿ���Ҫ����������ɫ
�㷨ʵ�� �� 
����˵�� �� byOverlapIndex��������λ��� nOverlapPhaseType��������λ����
����ֵ˵����������λ�Ƿ���Ҫ����������ɫ��־
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ����Ƽ          �������� 
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
			if (m_tNewPhaseLockPara.m_nPhaseLockCount == 0)//û���·��µ���λ����ָ��
			{
				for (j = 0; j < MAX_PHASE_COUNT; j++)
				{
					if (m_tLockPhaseData[j].nCurLockPhaseID == 0)
					{
						continue;
					}

					if (!m_tLockPhaseData[j].bNeedTransFlag && byMainPhaseNum == m_tLockPhaseData[j].nTargetPhaseIDInCurRing)
					{
						return true;//������λ��ĸ��λ����Ŀ����λ������Ŀ����λ�����������������
					}
				}
			}
			else//����λ����ʱ���·����µ���λ����ָ����Ҹ�����λ��ĸ��λ�����µ�������λ���������λ�����������������
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
������ ��IsHasTargetPhaseIncludedPhase
���� ��������λ��ĸ��λ���Ƿ�����ɵ�������λ
�㷨ʵ�� �� 
����˵�� �� byOverlapIndex��������λ��� nOverlapPhaseType��������λ����
����ֵ˵����������λ��ĸ��λ���Ƿ�����ɵ�������λ��־
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ����Ƽ          �������� 
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
					bFlag = true;//����λ����ʱ��������λ��ĸ��λ�������µ�������λ�����ǰ����ɵ�����Ŀ����λ
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
������ ��GetOldLockPhaseStatus
���� ����ȡ�ɵ�������λ��״̬
�㷨ʵ�� �� 
����˵�� �� byOverlapIndex��������λ��� nOverlapPhaseType��������λ����
����ֵ˵�����ɵ�������λ��״̬
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ����Ƽ          �������� 
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
������ ��LockChannelTransClr
���� ����ͨ������ʱ����������ֶ���ť����ʱ��Ҫ��ͨ����ɫ������
�㷨ʵ�� �� 
����˵�� ����
����ֵ˵����ͨ�����ɳɹ���־
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ����Ƽ          �������� 
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
			//1.Ŀ����λ���ɽ�������Ŀ����λ���ڻ��ĵ�ǰ��λ��û���л�����
			//2.Ŀ����λ��ΪҪ�������������̣���Ŀ����λ���ڻ��ĵ�ǰ��λ��û���л�����
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
������ ��ProcessLockChannelToPanel
���� ��ͨ�������л������
�㷨ʵ�� �� 
����˵�� ��tValidManualCmd����Ч�Ŀ���ָ��
����ֵ˵����ͨ�������л��������ɷ���ֵ
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ����Ƽ          �������� 
====================================================================*/ 
int CLogicCtlManual::ProcessLockChannelToPanel(TManualCmd  tValidManualCmd)
{
	if (m_tOldValidManualCmd.m_nSubCtlMode == CTL_MANUAL_SUBMODE_SYSTEM_CHANNEL_LOCK)//����ͨ�������Ժ��Ȱ�������ֶ���ť,�ְ��²������߷����������
	{
		if (tValidManualCmd.m_nSubCtlMode == CTL_MANUAL_SUBMODE_PANEL_FITSTMANUAL || 
			tValidManualCmd.m_nSubCtlMode == CTL_MANUAL_SUBMODE_PANEL_STEPFOWARD || 
			tValidManualCmd.m_bDirectionCmd ||
			tValidManualCmd.m_bPatternInterruptCmd)
		{
			if (LockChannelTransClr())
			{
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "LockChannelTransClr End");

				//ͨ����������������ͬһ���׶ε���λ�����ɽ�����ֱ���е�����
				if (tValidManualCmd.m_bDirectionCmd && (!m_tOldValidManualCmd.m_bPhaseToChannelLock || m_tLockPhaseData[0].bLockPhaseInSameStageFlag))
				{
					tValidManualCmd.m_nSubCtlMode = CTL_MANUAL_SUBMODE_PANEL_DIRECTION;
					tValidManualCmd.m_tDirectionCmd.m_nTargetStageIndex = m_tFixTimeCtlInfo.m_nCurStageIndex;//��¼���з���֮ǰ����һ���׶�

					tValidManualCmd.m_bDirectionCmd = false;//�е����������ģʽ�£������ǰ����ָ���־
		   
					//��ʼ�������������ʼ�з���
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
					//��Ϊ�޸��˷���Ĳ���������ָ�Ҫ���ݹ�һ��
					memcpy(&m_tOldValidManualCmd, &tValidManualCmd, sizeof(tValidManualCmd));
					//�е������Ժ󣬷���ʼǰ����������ɫ��ͨ���ı�־Ҫ����
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
������ ��IsNeglectPhase
���� ���жϵ�ǰ��λ�Ƿ��Ǻ�����λ��ض���λ
�㷨ʵ�� �� 
����˵�� ��nPhaseID����λID
����ֵ˵������
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ����Ƽ          �������� 
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
������ ��SetNeglectPhaseRunTime
���� �����ú�����λ�͹ض���λ������ʱ��
�㷨ʵ�� �� 
����˵�� ��nRingIndex��������  bPedPhase��������λ��־
����ֵ˵��������ʱ��
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ����Ƽ          �������� 
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
������ ��ReSetNeglectPhaseStageRunTime
���� ���������ú�����λ�͹ض���λ�ĵ�ǰ�׶�ʱ��
�㷨ʵ�� �� 
����˵�� ����
����ֵ˵������
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ����Ƽ          �������� 
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

				//////////////////��������λ�����е�ʱ��//////////////////////////////////////
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
				//////////////////������λ�����е�ʱ��//////////////////////////////////////
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

				//////////////////��������λ�����е�ʱ��//////////////////////////////////////
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
				//////////////////������λ�����е�ʱ��//////////////////////////////////////
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
������ ��ReSetNeglectPhasetBackUpTime
���� ���������ú�����λ�͹ض���λ�ı���ʱ��
�㷨ʵ�� �� 
����˵�� ����
����ֵ˵������
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ����Ƽ          �������� 
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
������ ��SetNeglectChannelBoforePhaseLock
���� ����¼����λ����֮ǰ������ͨ��(���Ի�ض�)
�㷨ʵ�� �� 
����˵�� ��nStageIndex���׶α��
����ֵ˵������
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ����Ƽ          �������� 
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
������ ��SetLockPhaseList
���� ����¼������λ��
�㷨ʵ�� �� 
����˵�� ��tValidManualCmd����Ч�Ŀ���ָ��
����ֵ˵������
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ����Ƽ          �������� 
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
������ ��SetLockPhaseStage
���� ����¼������λ��׶�
�㷨ʵ�� �� 
����˵�� ��tValidManualCmd����Ч�Ŀ���ָ��
����ֵ˵������
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ����Ƽ          �������� 
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