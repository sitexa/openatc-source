/*=====================================================================
ģ���� ����Ӧʽ�̲��ؿ��Ʒ�ʽ�ӿ�ģ��
�ļ��� ��LogicCtlInductiveGreenWave.cpp
����ļ���LogicCtlInductiveGreenWave.h,LogicCtlInductiveGreenWave.h
ʵ�ֹ��ܣ���Ӧʽ�̲��ؿ��Ʒ�ʽʵ��
���� ������
��Ȩ ��<Copyright(c) 2019-2020 Suzhou Keda Technology Co.,Ltd. All right reserved.>
-----------------------------------------------------------------------
�޸ļ�¼��
�� ��           �汾     �޸���     �߶���     �޸ļ�¼
2019/9/14       V1.0     ����       ����      ����ģ��
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
������ ��Init 
���� ���޵����߿ؿ��Ʒ�ʽ����Դ��ʼ��
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
void CLogicCtlInductiveGreenWave::Init(COpenATCParameter * pParameter, COpenATCRunStatus * pRunStatus, COpenATCLog * pOpenATCLog, int nPlanNo)
{
    m_pOpenATCParameter = pParameter;
    m_pOpenATCRunStatus = pRunStatus;
	m_pOpenATCLog       = pOpenATCLog;
    m_nRealOffset		= 0;
    m_nGreenStageChgLen = 0;
    m_bCalcOffsetFlag	= true;
	m_bOffsetCoordinateCloseFlag = false;//Э��������ʶ
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
������ ��ReSetCtlStatus
���� ���������н���������λ����״̬��Ϣ
�㷨ʵ�� �� 
����˵�� ��
����ֵ˵����
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ����Ƽ          �������� 
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
������ ��OnePhaseRun 
���� ���޵����߿ؿ���ʱ������������λ����״̬����
�㷨ʵ�� �� 
����˵�� ��nRingIndex�������� 
����ֵ˵����
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ������          �������� 
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

		//��ǰ��λ�׶��Ƿ����
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

			//���û�����λ��ɫ������ʱ��
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
	if (m_bOffsetCoordinateCloseFlag && m_bCycleCoordinateCloseFlag)//��λ��ִ�н���ʵ�и�Ӧ����
    {
		char szInfo[256] = { 0 };
		PTRingCtlInfo ptRingRunInfo = &(m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex]);

		int nIndex = m_tFixTimeCtlInfo.m_nCurPhaseIndex[nRingIndex];
		TPhaseLampClrRunCounter tRunCounter;
		m_pOpenATCRunStatus->GetPhaseLampClrRunCounter(tRunCounter);

		//��ǰ��λ�׶��Ƿ����
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
			//������λ�Ի�������λΪ׼
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

			//���û�����λ��ɫ������ʱ��
			SetLampClrByRing(nRingIndex, nIndex, ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseStage, ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPedPhaseStage);
			tRunCounter.m_nLampClrTime[nRingIndex] = 0;
			tRunCounter.m_nLampClrStartCounter[nRingIndex] = tRunCounter.m_nCurCounter;
			m_pOpenATCRunStatus->SetPhaseLampClrRunCounter(tRunCounter);
			m_bIsLampClrChg = true;
		}

		//�ж��Ƿ���Ҫ�ӳ��̵�ʱ��
		if (ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseStage == C_CH_PHASESTAGE_G&&nIndex!=0)
		{
			ProcExtendGreen(nRingIndex, nIndex, tRunCounter.m_nLampClrTime[nRingIndex]);
		}

		//����ǰ�����һ��������λ�̵�ʱ���䲢����λ�����ӳ��̵������
		if (m_bLastPhaseBeforeBarrier[nRingIndex][nIndex] && ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseStage == C_CH_PHASESTAGE_G && ProcPhaseDetStatus(nRingIndex, nIndex))
		{
			LastConcurrencyPhaseBeforeBarrierExtendGreen(nRingIndex, nIndex, tRunCounter.m_nLampClrTime[nRingIndex]);
		}
   }
}

/*==================================================================== 
������ ��OnePedPhaseRun
���� ���޵����߿ؿ���ʱ����������λ����״̬����
�㷨ʵ�� �� 
����˵�� ��nRingIndex�������� 
����ֵ˵����
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ������          �������� 
====================================================================*/ 
void CLogicCtlInductiveGreenWave::OnePedPhaseRun(int nRingIndex)
{
	if (m_bOffsetCoordinateCloseFlag == false || m_bCycleCoordinateCloseFlag == false)
	{
		PTRingCtlInfo ptRingRunInfo = &(m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex]);

		int nIndex = m_tFixTimeCtlInfo.m_nCurPhaseIndex[nRingIndex];
		TPhaseLampClrRunCounter tRunCounter;
		m_pOpenATCRunStatus->GetPhaseLampClrRunCounter(tRunCounter);

		//��ǰ��λ�׶��Ƿ����
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

			//������λ�����������λһ�����
			if (chNextPedStage == C_CH_PHASESTAGE_G)
			{
				ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPedPhaseStageRunTime -= m_nGreenStageChgLen;
			}

			m_tFixTimeCtlInfo.m_bIsChgPedStage[nRingIndex] = false;

			if (chNextPedStage == C_CH_PHASESTAGE_GF)
			{
				GetGreenFalshCount(PED_CHA, (PTPhaseCtlInfo)&(ptRingRunInfo->m_atPhaseInfo[nIndex]), wPedStageTime);
			}

			//���û�����λ��ɫ������ʱ��
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

		   //���û�����λ��ɫ������ʱ��
		   SetLampClrByRing(nRingIndex, nIndex, ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseStage, ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPedPhaseStage);
		   tRunCounter.m_nPedLampClrTime[nRingIndex] = 0;
		   tRunCounter.m_nPedLampClrStartCounter[nRingIndex] = tRunCounter.m_nCurCounter;
		   m_pOpenATCRunStatus->SetPhaseLampClrRunCounter(tRunCounter);
		   m_bIsLampClrChg = true;
	   }
   }
}

/*==================================================================== 
������ ��CalcRealOffset
���� ���޵����߿ؿ���ʱ����ʵ����λ��,������λ�̵�ʱ��
�㷨ʵ�� �� 
����˵�� ��
����ֵ˵����
        ��ǰʵ�ʵ���λ���0����Ϊͳһ��׼.
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ������          �������� 
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
������ ��ProcExtendGreen
���� ��������λ�Ƿ���Ҫ�ӳ��̵�ʱ��ֱ�������
�㷨ʵ�� ��
����˵�� ��nRingIndex����ǰ���еĻ�����
nPhaseIndex����ǰ���е���λ����
nCurRunTime����ǰ�׶ε�����ʱ��
����ֵ˵����
----------------------------------------------------------------------
�޸ļ�¼ ��
�� ��          �汾 �޸���  �߶���  �޸ļ�¼
2021/03/26     V1.0 ����			��������
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
		//��λ��ʼִ��ʱ��Ϊ��λ��С��
		
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
������ ��LastConcurrencyPhaseBeforeBarrierExtendGreen
���� ������ǰ�����һ��������λ(û��������Ȧ����Ȧ����)�Ĳ�����λ�ӳ���
�㷨ʵ�� ��
����˵�� ��nRingIndex����ǰ���еĻ�����
nPhaseIndex����ǰ���е���λ����
nCurRunTime����ǰ�׶ε�����ʱ��
����ֵ˵����
----------------------------------------------------------------------
�޸ļ�¼ ��
�� ��          �汾 �޸���  �߶���  �޸ļ�¼
2019/09/14     V1.0 ������          ��������
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
������ ��CalcGreenWavePhaseTime
���� �����ڽ���������̲���λ��ʱ��
�㷨ʵ�� ��Э����λ���̵�ʱ��Ϊ��Э����λ������̵�ʱ���ȥ��ʵ��������ʱ��
����˵�� ��
����ֵ˵����
----------------------------------------------------------------------
�޸ļ�¼ ��
�� ��          �汾 �޸���  �߶���  �޸ļ�¼
2019/09/14     V1.0 ����          ��������
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