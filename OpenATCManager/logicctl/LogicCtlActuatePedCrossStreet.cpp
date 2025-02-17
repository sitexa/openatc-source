/*=====================================================================
ģ���� ����Ӧʽ���˹��ֿ��Ʒ�ʽ�ӿ�ģ��
�ļ��� ��LogicCtlActuatePedCrossStreet.cpp
����ļ���LogicCtlActuatePedCrossStreet.h,LogicCtlFixedTime.h
ʵ�ֹ��ܣ���Ӧʽ���˹��ֿ��Ʒ�ʽʵ��
���� ���º���
��Ȩ ��<Copyright(c) 2019-2021 Suzhou Keda Technology Co.,Ltd. All right reserved.>
-----------------------------------------------------------------------
�޸ļ�¼��
�� ��           �汾     �޸���     �߶���     �޸ļ�¼
2020/11/24       V1.0     �º���     �º���     ����ģ��
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
������ ��Init 
���� ����Ӧʽ���˹��ֿ��Ʒ�ʽ����Դ��ʼ��
�㷨ʵ�� �� 
����˵�� ��pParameter����������ָ��
           pRunStatus��ȫ������״̬��ָ��
		   pOpenATCLog����־ָ��
           nPlanNo��ָ���ķ�����,0��ʾʹ��ʱ�ζ�Ӧ�ķ���  
����ֵ˵����
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2020/11/24     V1.0 �º���          �������� 
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

	//��ǰ����ģʽΪ��Ӧʽ���˹��ֿ���
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
������ ��OnePhaseRun 
���� ����Ӧʽ���˹��ֿ���ʱ������������λ����״̬����
�㷨ʵ�� �� 
����˵�� ��nRingIndex�������� 
����ֵ˵����
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2020/11/24     V1.0 �º���          �������� 
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
		//��ǰ��λ�׶��Ƿ����
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
			//������λ�Ի�������λΪ׼
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

		//���û�����λ��ɫ������ʱ��
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
		//�ж��Ƿ���Ҫ�ӳ��̵�ʱ��
		if (ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseStage == C_CH_PHASESTAGE_G)
		{
			ProcExtendGreen(nRingIndex,nIndex,tRunCounter.m_nLampClrTime[nRingIndex]);
		}

		//����ǰ�����һ��������λ�̵�ʱ���䲢����λ�����ӳ��̵������
		if (m_bLastPhaseBeforeBarrier[nRingIndex][nIndex] && ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseStage == C_CH_PHASESTAGE_G && ProcPhaseDetStatus(nRingIndex,nIndex))
		{
			//LastConcurrencyPhaseBeforeBarrierExtendGreen(nRingIndex,nIndex,tRunCounter.m_nLampClrTime[nRingIndex]);
		}
	}
}

/*==================================================================== 
������ ��OnePedPhaseRun
���� ����Ӧʽ���˹��ֿ���ʱ����������λ����״̬����
�㷨ʵ�� �� 
����˵�� ��nRingIndex�������� 
����ֵ˵����
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2020/11/24     V1.0 �º���          �������� 
====================================================================*/ 
void CLogicCtlActuatePedCrossStreet::OnePedPhaseRun(int nRingIndex)
{
	PTRingCtlInfo ptRingRunInfo = &(m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex]);

	int nIndex = m_tFixTimeCtlInfo.m_nCurPhaseIndex[nRingIndex];
	TPhaseLampClrRunCounter tRunCounter;
	m_pOpenATCRunStatus->GetPhaseLampClrRunCounter(tRunCounter);

	if (m_atSplitInfo[ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhaseNumber - 1].m_bySplitMode == PED_MODE)
	{
		//��ǰ��λ�׶��Ƿ����
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
			//��������λ��������λΪ׼
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

		//���û�����λ��ɫ������ʱ��
		SetLampClrByRing(nRingIndex,nIndex,ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseStage,ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPedPhaseStage);
		tRunCounter.m_nPedLampClrTime[nRingIndex] = 0;
		tRunCounter.m_nPedLampClrStartCounter[nRingIndex] = tRunCounter.m_nCurCounter;    
		m_pOpenATCRunStatus->SetPhaseLampClrRunCounter(tRunCounter);
		m_bIsLampClrChg = true;
	} 

	if (m_atSplitInfo[ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhaseNumber - 1].m_bySplitMode == PED_MODE)
	{
		//�ж��Ƿ���Ҫ�ӳ��̵�ʱ��
		if (ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPedPhaseStage == C_CH_PHASESTAGE_G)
		{
			ProcExtendGreen(nRingIndex,nIndex,tRunCounter.m_nLampClrTime[nRingIndex]);
		}

		//����ǰ�����һ��������λ�̵�ʱ���䲢����λ�����ӳ��̵������
		if (m_bLastPhaseBeforeBarrier[nRingIndex][nIndex] && ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPedPhaseStage == C_CH_PHASESTAGE_G && ProcPhaseDetStatus(nRingIndex,nIndex))
		{
			//LastConcurrencyPhaseBeforeBarrierExtendGreen(nRingIndex,nIndex,tRunCounter.m_nLampClrTime[nRingIndex]);
		}
	}
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
2020/11/24     V1.0 �º���          �������� 
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

    // ��ȡ�������Ŷӳ���
	m_pOpenATCRunStatus->GetVehicleQueueUpInfo(m_tVehicleQueueUpInfo);
	// ��ȡ�������˼����Ϣ
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

		// �ȴ���������
		if (m_tPedDetectInfo[i].m_byVehicleDetectorCallPhase == pPhaseInfo->m_byPhaseNumber && m_nPedDetecetorType[i] == PED_DETECTOR_WAIT_AREA)
		{
			if (nMaxPedWaitLength < m_tPedDetectInfo[i].m_nPedCount)
			{
				nMaxPedWaitLength = m_tPedDetectInfo[i].m_nPedCount;
			}
		}

		// ������������
		if (m_tPedDetectInfo[i].m_byVehicleDetectorCallPhase == pPhaseInfo->m_byPhaseNumber && m_nPedDetecetorType[i] == PED_DETECTOR_CROSS_AREA)
		{
			if (nMaxPedCrossStreetLength < m_tPedDetectInfo[i].m_nPedCount)
			{
				nMaxPedCrossStreetLength = m_tPedDetectInfo[i].m_nPedCount;
			}
		}
	}

	// ��ȡ���˹�������
	if (nMaxVehicleQueueLength || nMaxPedWaitLength)
	{
		m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "###### PhaseNum[%d], nMaxVehicleQueueLength=%d, nMaxPedWaitLength=%d.", pPhaseInfo->m_byPhaseNumber, nMaxVehicleQueueLength, nMaxPedWaitLength);
	}
	else
	{ 
		return;
	}
        
	// ����������Ŷӳ��ȳ����趨��ֵ�����߻������Ŷӳ���δ�����趨��ֵ�������˵ȴ���������Ҳδ�����趨��ֵ
    if ((nMaxVehicleQueueLength >= MAX_VEH_QUEUE_LENGTH)
		|| nMaxPedWaitLength < MAX_PED_WAIT_LENGTH
		|| nMaxPedCrossStreetLength)
    {
		if (m_atSplitInfo[ptRingRunInfo->m_atPhaseInfo[nPhaseIndex].m_tPhaseParam.m_byPhaseNumber - 1].m_bySplitMode == PED_MODE)
		{
			//��λ��ʼִ��ʱ��Ϊ��λ���ű�
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
			//��λ��ʼִ��ʱ��Ϊ��λ���ű�
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
2020/11/24     V1.0 �º���          �������� 
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

