/*=====================================================================
ģ���� �����Ʒ�ʽ�ӿ�ģ��
�ļ��� ��LogicCtlFixedTime.cpp
����ļ���LogicCtlFixedTime.h
ʵ�ֹ��ܣ������ڿ��Ʒ�ʽʵ��
���� ��������
��Ȩ ��<Copyright(c) 2019-2020 Suzhou Keda Technology Co.,Ltd. All right reserved.>
-----------------------------------------------------------------------
�޸ļ�¼��
�� ��           �汾     �޸���     �߶���     �޸ļ�¼
2019/9/14       V1.0     ������     ������     ����ģ��
2020/2/26       V1.0     ����Ƽ     ����Ƽ     �������崦��
2020/3/26       V1.0     ����Ƽ     ����Ƽ     ������λ���н׶α�ͻ��ڽ׶��л�״̬�жϸ���
2020/3/26       V1.0     ����Ƽ     ����Ƽ     �ҵ�ÿ����������ǰ�����һ����λ
=====================================================================*/

#include "LogicCtlFixedTime.h"
#include <string.h>
#include <algorithm>

CLogicCtlFixedTime::CLogicCtlFixedTime()
{
}

CLogicCtlFixedTime::~CLogicCtlFixedTime()
{
	for (int i = 0;i < m_tFixTimeCtlInfo.m_nRingCount;i++)
	{
		 PTRingCtlInfo ptRingRunInfo = &(m_tFixTimeCtlInfo.m_atPhaseSeq[i]);
         int nIndex = m_tFixTimeCtlInfo.m_nCurPhaseIndex[i];

		 int nNextIndex = m_tFixTimeCtlInfo.m_nCurPhaseIndex[i] + 1;
		 if (nNextIndex == ptRingRunInfo->m_nPhaseCount)
		 {
			 nNextIndex = 0;
		 }

	     SetGreenLampPulse(VEH_CHA, ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhaseNumber, false);
         SetRedLampPulse(VEH_CHA, ptRingRunInfo->m_atPhaseInfo[nNextIndex].m_tPhaseParam.m_byPhaseNumber, false); 

		 SetGreenLampPulse(PED_CHA, ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhaseNumber, false);
		 SetRedLampPulse(PED_CHA, ptRingRunInfo->m_atPhaseInfo[nNextIndex].m_tPhaseParam.m_byPhaseNumber, false); 

		 SetOverLapGreenPulse(i, nIndex, true, m_tFixTimeCtlInfo.m_nCurStageIndex + 1, false);
		 SetOverLapRedPulse(i, nIndex, true, m_tFixTimeCtlInfo.m_nCurStageIndex + 1, false);
	}
}

/*==================================================================== 
������ ��Init 
���� �������ڿ��Ʒ�ʽ����Դ��ʼ��
�㷨ʵ�� �� 
����˵�� ��pParameter����������ָ��
           pRunStatus��ȫ������״̬��ָ��
		   pOpenATCLog����־ָ��
           nPlanNo��ʹ�õķ����ţ�0��ʾ����ʱ��ȡ���� 
����ֵ˵����
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ������          �������� 
====================================================================*/ 
void CLogicCtlFixedTime::Init(COpenATCParameter * pParameter, COpenATCRunStatus * pRunStatus, COpenATCLog * pOpenATCLog, int nPlanNo)
{
    m_pOpenATCParameter = pParameter;
    m_pOpenATCRunStatus = pRunStatus;
	m_pOpenATCLog       = pOpenATCLog;
    memset(&m_tFixTimeCtlInfo,0,sizeof(m_tFixTimeCtlInfo));

    memset(&m_tRunStageInfo, 0x00, sizeof(m_tRunStageInfo));
    memset(&m_bLastPhaseBeforeBarrier, 0x00, sizeof(m_bLastPhaseBeforeBarrier));
    memset(m_nChannelSplitMode,0x00,sizeof(m_nChannelSplitMode)); 
    memset(m_bShieldStatus, 0x00, sizeof(m_bShieldStatus));
    memset(m_bProhibitStatus, 0x00, sizeof(m_bProhibitStatus));
    memset(m_bOldShieldStatus, 0x00, sizeof(m_bOldShieldStatus));
    memset(m_bOldProhibitStatus, 0x00, sizeof(m_bOldProhibitStatus));
    memset(m_bySplitPhaseMode, 0x00, sizeof(m_bySplitPhaseMode));
    memset(m_nSplitPhaseTime, 0x00, sizeof(m_nSplitPhaseTime));

    //��ǰ����ģʽΪ�����ڿ��ƿ���
    m_nCurRunMode = CTL_MODE_FIXTIME;

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

    SetChannelSplitMode();

    TLogicCtlStatus tCtlStatus;
    m_pOpenATCRunStatus->GetLogicCtlStatus(tCtlStatus);
    tCtlStatus.m_nCurCtlMode = CTL_MODE_FIXTIME;
    tCtlStatus.m_nCurPlanNo = (int)byPlanID;
    m_pOpenATCRunStatus->SetLogicCtlStatus(tCtlStatus);

    RetsetAllChannelStatus();

    GetRunStageTable();

    SetSeqByStageInfo();

    GetLastPhaseBeforeBarrier();

	for (int i = 0;i < MAX_RING_COUNT;i++)
	{
		m_tPhasePulseStatus[i].m_nPhaseIndex = 0;
		m_tPhasePulseStatus[i].m_nPhaseNum = m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[m_tPhasePulseStatus[i].m_nPhaseIndex].m_tPhaseParam.m_byPhaseNumber;
		m_tPhasePulseStatus[i].m_bGreenPulseStatus = false;
		m_tPhasePulseStatus[i].m_bRedPulseStatus = false;
		m_tPhasePulseStatus[i].m_nGreenPulseSendStatus = SEND_INIT;
		m_tPhasePulseStatus[i].m_nRedPulseSendStatus = SEND_INIT;
	}

	memset(m_bKeepGreenChannelBeforeControlChannelFlag, false, sizeof(m_bKeepGreenChannelBeforeControlChannelFlag));
}

/*==================================================================== 
������ ��InitOverlapParam
���� ����ʼ��������λ����
�㷨ʵ�� �� 
����˵�� �� 
����ֵ˵����
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ������          �������� 
====================================================================*/ 
void CLogicCtlFixedTime::InitOverlapParam()
{
    TOverlapTable atOverlapTable[MAX_OVERLAP_COUNT];
    m_pOpenATCParameter->GetOverlapTable(atOverlapTable);
    int i = 0;
    for (i = 0;i < MAX_OVERLAP_COUNT;i++)
    {
        memcpy(&m_tFixTimeCtlInfo.m_atOverlapInfo[i].m_tOverlapParam, &atOverlapTable[i], sizeof(TOverlapTable));
        m_tFixTimeCtlInfo.m_atOverlapInfo[i].m_chOverlapStage = C_CH_PHASESTAGE_U;
		m_tFixTimeCtlInfo.m_atOverlapInfo[i].m_chPedOverlapStage = C_CH_PHASESTAGE_U;
        if (atOverlapTable[i].m_byOverlapNumber != 0)
        {
            m_tFixTimeCtlInfo.m_nOverlapCount += 1;
        }
    }
}

/*==================================================================== 
������ ��InitDetectorParam
���� ����ʼ�����������
�㷨ʵ�� �� 
����˵�� �� 
����ֵ˵����
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ������          �������� 
====================================================================*/ 
void CLogicCtlFixedTime::InitDetectorParam()
{
    TVehicleDetector atVehDetector[MAX_VEHICLEDETECTOR_COUNT];
    memset(atVehDetector,0,sizeof(TVehicleDetector) * MAX_VEHICLEDETECTOR_COUNT);
    m_pOpenATCParameter->GetVehicleDetectorTable(atVehDetector);

    int i = 0;
    for (i = 0;i < MAX_VEHICLEDETECTOR_COUNT;i++)
    {
        if (atVehDetector[i].m_byVehicleDetectorNumber != 0)
        {
            memcpy(&m_tFixTimeCtlInfo.m_atVehDetector[i], &atVehDetector[i], sizeof(TVehicleDetector));
            m_tFixTimeCtlInfo.m_nVehDetCount += 1;
        }
    }
}

/*==================================================================== 
������ ��InitPedDetectorParam
���� ����ʼ�����˼��������
�㷨ʵ�� �� 
����˵�� �� 
����ֵ˵����
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ������          �������� 
====================================================================*/ 
void CLogicCtlFixedTime::InitPedDetectorParam()
{
	TPedestrianDetector atPedDetector[MAX_PEDESTRIANDETECTOR_COUNT];
    memset(atPedDetector,0,sizeof(TPedestrianDetector) * MAX_PEDESTRIANDETECTOR_COUNT);
    m_pOpenATCParameter->GetPedDetectorTable(atPedDetector);

    int i = 0;
    for (i = 0;i < MAX_PEDESTRIANDETECTOR_COUNT;i++)
    {
        if (atPedDetector[i].m_byPedestrianDetectorNumber != 0)
        {
            memcpy(&m_tFixTimeCtlInfo.m_atPedDetector[i], &atPedDetector[i], sizeof(TPedestrianDetector));
            m_tFixTimeCtlInfo.m_nPedDetCount += 1;

            int nDstRingIndex = 0;
            int nDstPhaseIndex = 0;

            if (GetPhaseIndexByPhaseNumber(nDstRingIndex, nDstPhaseIndex, (int)atPedDetector[i].m_byPedestrianDetectorCallPhase))
            {
                m_tFixTimeCtlInfo.m_atPhaseSeq[nDstRingIndex].m_atPhaseInfo[nDstPhaseIndex].m_bIsPedAskPhase = true;
            }

        }
    }
}

/*==================================================================== 
������ ��InitByTimeSeg
���� ������ʱ�γ�ʼ�����Ʋ����Ϳ���״̬
�㷨ʵ�� �� 
����˵�� �� 
����ֵ˵����
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ������          �������� 
====================================================================*/ 
void CLogicCtlFixedTime::InitByTimeSeg(BYTE & byPlanID)
{
    int nYear,nMonth,nDay,nHour,nMin,nSec,nWeek;
#ifdef VIRTUAL_DEVICE
    bool bSpeedyRunStatu = false;
    bSpeedyRunStatu = m_pOpenATCRunStatus->GetIsSpeedyRunStatus();
    if (bSpeedyRunStatu)
    {
        GetVirtualTimeByGlobalCount(nYear, nMonth, nDay, nHour, nMin, nSec, nWeek);
    }
    else
    {
        OpenATCGetCurTime(nYear, nMonth, nDay, nHour, nMin, nSec, nWeek);
    }
#else
    OpenATCGetCurTime(nYear, nMonth, nDay, nHour, nMin, nSec, nWeek);
#endif // VIRTUAL_DEVICE
    //Virtual_Test2022

    //Test
    TTimeBaseSchedule tScheduleInfo;
    TTimeBaseDayPlan tTimeBaseDayPlanInfo;
    TPattern tPatternInfo;
    int nCount = 0;
    int iDatePlanNum = 0;
    //��ȡ������
    iDatePlanNum = m_pOpenATCParameter->GetDatePlanNum();

    for (int i = 0; i < iDatePlanNum; i++)
    {
        m_pOpenATCParameter->GetTimeBaseScheduleByTime(nMonth, nWeek, nDay, tScheduleInfo, nCount);
        m_pOpenATCParameter->GetTimeBaseDayPlanByTime(nHour, nMin, tScheduleInfo.m_byTimeBaseScheduleDayPlan, tTimeBaseDayPlanInfo);
        if (/*tTimeBaseDayPlanInfo.m_byDayPlanActionNumberOID == 0 && */ tTimeBaseDayPlanInfo.m_byCoorDination == 1 && tTimeBaseDayPlanInfo.m_byDayPlanControl == 0)
        {
            continue;
        }
        else
        {
            break;
        }
    }
    m_pOpenATCParameter->GetPatternByPlanNumber(tTimeBaseDayPlanInfo.m_byDayPlanActionNumberOID, tPatternInfo);
    //TTimeBaseSchedule tScheduleInfo;
   	//m_pOpenATCParameter->GetTimeBaseScheduleByTime(nMonth,nWeek,nDay,tScheduleInfo);
    //TTimeBaseDayPlan tTimeBaseDayPlanInfo;
    //m_pOpenATCParameter->GetTimeBaseDayPlanByTime(nHour,nMin,tScheduleInfo.m_byTimeBaseScheduleDayPlan,tTimeBaseDayPlanInfo);
    //TPattern tPatternInfo;
   	//m_pOpenATCParameter->GetPatternByPlanNumber(tTimeBaseDayPlanInfo.m_byDayPlanActionNumberOID,tPatternInfo);
    m_tFixTimeCtlInfo.m_wPhaseOffset = tPatternInfo.m_byPatternOffsetTime;
    m_tFixTimeCtlInfo.m_wCycleLen = tPatternInfo.m_wPatternCycleTime;
	m_tFixTimeCtlInfo.m_wPatternNumber = tPatternInfo.m_byPatternNumber;

	m_pOpenATCParameter->GetSplitBySplitNumber(tPatternInfo.m_byPatternSplitNumber,m_atSplitInfo);

    TSequence atSequenceInfo[MAX_RING_COUNT];
    m_pOpenATCParameter->GetSequenceBySequenceNumber(tPatternInfo.m_byPatternSequenceNumber,atSequenceInfo);

    char szInfo[256] = {0};

    int nRingCount = 0;
    int i = 0;
    int j = 0;
    for (i = 0;i < MAX_RING_COUNT;i ++)
    {
        int nPhaseIndex = 0;
        for (j = 0;j < MAX_SEQUENCE_DATA_LENGTH;j ++)
        {
            int nPhaseNum = (int)atSequenceInfo[i].m_bySequenceData[j];

            if (nPhaseNum == 0)
            {
                break;
            }
            
            m_pOpenATCParameter->GetPhaseByPhaseNumber(nPhaseNum,m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nPhaseIndex].m_tPhaseParam);
			
			//��Ϊ�����1�������2����С�̰�������������Ҫ������ʱ��ȥ��
			m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nPhaseIndex].m_tPhaseParam.m_wPhaseMaximum1 -= m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nPhaseIndex].m_tPhaseParam.m_byGreenFlash;
            m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nPhaseIndex].m_tPhaseParam.m_wPhaseMaximum2 -= m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nPhaseIndex].m_tPhaseParam.m_byGreenFlash;
			m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nPhaseIndex].m_tPhaseParam.m_wPhaseMinimumGreen -= m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nPhaseIndex].m_tPhaseParam.m_byGreenFlash;
			
			WORD wSplitTime = 0;
            for (int k = 0;k < MAX_PHASE_COUNT;k ++)
            {
                if (m_atSplitInfo[k].m_bySplitPhase == nPhaseNum/* && m_atSplitInfo[k].m_bySplitMode != NEGLECT_MODE*/)
                {
                    wSplitTime = m_atSplitInfo[k].m_wSplitTime;  
                    break; 
                }
            }

            if (wSplitTime == 0)
            {
                continue;
            }
            m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nPhaseIndex].m_wPhaseTime = wSplitTime;
            PTPhase pPhaseInfo = &(m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nPhaseIndex].m_tPhaseParam);
            m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nPhaseIndex].m_wPhaseGreenTime = wSplitTime - pPhaseInfo->m_byGreenFlash - pPhaseInfo->m_byPhaseYellowChange - pPhaseInfo->m_byPhaseRedClear;
            m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nPhaseIndex].m_wPedPhaseGreenTime = wSplitTime - pPhaseInfo->m_byPhaseYellowChange - pPhaseInfo->m_byPhasePedestrianClear - pPhaseInfo->m_byPhaseRedClear;
            m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nPhaseIndex].m_chPedPhaseStage = C_CH_PHASESTAGE_U;
            m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nPhaseIndex].m_chPhaseStage = C_CH_PHASESTAGE_U;

            nPhaseIndex++;
        }

		if (nPhaseIndex != 0)
		{
			nRingCount++;
		}

        m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_nPhaseCount = nPhaseIndex;
        m_tFixTimeCtlInfo.m_bIsChgStage[i] = true;
        m_tFixTimeCtlInfo.m_bIsChgPedStage[i] = true;   

        sprintf(szInfo, "RingIndex:%d PhaseCount:%d", i, nPhaseIndex);
        m_pOpenATCLog->LogOneMessage(LEVEL_INFO, szInfo);         
    }

	m_tFixTimeCtlInfo.m_nRingCount = nRingCount;
    byPlanID = tTimeBaseDayPlanInfo.m_byDayPlanActionNumberOID;

    int iPhaseCount = 0;
    for (int iRingIndex = 0; iRingIndex < m_tFixTimeCtlInfo.m_nRingCount; iRingIndex++)
    {
        iPhaseCount = m_tFixTimeCtlInfo.m_atPhaseSeq[iRingIndex].m_nPhaseCount;
        for (i = 0; i < iPhaseCount; i++)
        {
            if (m_tFixTimeCtlInfo.m_atPhaseSeq[iRingIndex].m_atPhaseInfo[i].m_wPhaseTime > 0)
            {
                for (j = 0; j < MAX_PHASE_COUNT; j++)
                {
                    if (m_tFixTimeCtlInfo.m_atPhaseSeq[iRingIndex].m_atPhaseInfo[i].m_tPhaseParam.m_byPhaseNumber == m_atSplitInfo[j].m_bySplitPhase)
                    {
                        m_tFixTimeCtlInfo.m_atPhaseSeq[iRingIndex].m_atPhaseInfo[i].m_chPhaseMode = m_atSplitInfo[j].m_bySplitMode;
                        break;
                    }
                }
            }
        }
    }

	//int nNeglectPhaseCount = 0;
	//int nNeglectPhaseCountInRing[MAX_RING_COUNT];
	//memset(nNeglectPhaseCountInRing,0,sizeof(nNeglectPhaseCountInRing));
	//
	//TRingCtlInfo atPhaseSeq[MAX_RING_COUNT];
	//memset(atPhaseSeq,0,sizeof(atPhaseSeq));

	//for (i = 0;i < MAX_PHASE_COUNT;i ++)
	//{
	//	nNeglectPhaseCount = 0;

	//	for (j = 0;j < MAX_PHASE_COUNT;j ++)
	//	{
	//		if (m_tFixTimeCtlInfo.m_atPhaseSeq[0].m_atPhaseInfo[i].m_wPhaseTime > 0 &&
	//			m_tFixTimeCtlInfo.m_atPhaseSeq[0].m_atPhaseInfo[i].m_tPhaseParam.m_byPhaseNumber == m_atSplitInfo[j].m_bySplitPhase &&
	//			m_atSplitInfo[j].m_bySplitMode == NEGLECT_MODE)
	//		{
	//			nNeglectPhaseCount += 1;
	//			nNeglectPhaseCountInRing[0] += 1;
	//		}

	//		if (m_tFixTimeCtlInfo.m_nRingCount > 0)
	//		{
	//			if (m_tFixTimeCtlInfo.m_atPhaseSeq[1].m_atPhaseInfo[i].m_wPhaseTime > 0 &&
	//				m_tFixTimeCtlInfo.m_atPhaseSeq[1].m_atPhaseInfo[i].m_tPhaseParam.m_byPhaseNumber == m_atSplitInfo[j].m_bySplitPhase &&
	//				m_atSplitInfo[j].m_bySplitMode == NEGLECT_MODE)
	//			{
	//				nNeglectPhaseCount += 1;
	//				nNeglectPhaseCountInRing[1] += 1;
	//			}
	//		}
	//	}

	//	if (nNeglectPhaseCount != m_tFixTimeCtlInfo.m_nRingCount)
	//	{
	//		if (m_tFixTimeCtlInfo.m_atPhaseSeq[0].m_atPhaseInfo[i].m_wPhaseTime > 0)
	//		{
	//			memcpy(&atPhaseSeq[0].m_atPhaseInfo[atPhaseSeq[0].m_nPhaseCount], &m_tFixTimeCtlInfo.m_atPhaseSeq[0].m_atPhaseInfo[i], sizeof(m_tFixTimeCtlInfo.m_atPhaseSeq[0].m_atPhaseInfo[i]));
	//			atPhaseSeq[0].m_nPhaseCount += 1;
	//		}

	//		if (m_tFixTimeCtlInfo.m_nRingCount > 0 && m_tFixTimeCtlInfo.m_atPhaseSeq[1].m_atPhaseInfo[i].m_wPhaseTime > 0)
	//		{
	//			memcpy(&atPhaseSeq[1].m_atPhaseInfo[atPhaseSeq[1].m_nPhaseCount], &m_tFixTimeCtlInfo.m_atPhaseSeq[1].m_atPhaseInfo[i], sizeof(m_tFixTimeCtlInfo.m_atPhaseSeq[1].m_atPhaseInfo[i]));
	//			atPhaseSeq[1].m_nPhaseCount += 1;
	//		}
	//	}
	//}

	//if (nNeglectPhaseCountInRing[0] == m_tFixTimeCtlInfo.m_atPhaseSeq[0].m_nPhaseCount || nNeglectPhaseCountInRing[1] == m_tFixTimeCtlInfo.m_atPhaseSeq[0].m_nPhaseCount)
	//{
	//	m_tFixTimeCtlInfo.m_nRingCount -= 1;
	//}

	//if (nNeglectPhaseCountInRing[0] > 0 || nNeglectPhaseCountInRing[1] > 0)
	//{
	//	memcpy(m_tFixTimeCtlInfo.m_atPhaseSeq, &atPhaseSeq, sizeof(m_tFixTimeCtlInfo.m_atPhaseSeq));
	//}
}

/*==================================================================== 
������ ��InitByPlan
���� �����ݻ�������1��ʼ�����Ʋ����Ϳ���״̬
�㷨ʵ�� �� 
����˵�� �� 
����ֵ˵����
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ������          �������� 
====================================================================*/ 
void CLogicCtlFixedTime::InitByPlan(BYTE byPlanID)
{
    TPattern tPatternInfo;
   	m_pOpenATCParameter->GetPatternByPlanNumber(byPlanID,tPatternInfo);
    m_tFixTimeCtlInfo.m_wPhaseOffset = tPatternInfo.m_byPatternOffsetTime;
    m_tFixTimeCtlInfo.m_wCycleLen = tPatternInfo.m_wPatternCycleTime;
	m_tFixTimeCtlInfo.m_wPatternNumber = tPatternInfo.m_byPatternNumber;

	m_pOpenATCParameter->GetSplitBySplitNumber(tPatternInfo.m_byPatternSplitNumber,m_atSplitInfo);

    TSequence atSequenceInfo[MAX_RING_COUNT];
    m_pOpenATCParameter->GetSequenceBySequenceNumber(tPatternInfo.m_byPatternSequenceNumber,atSequenceInfo);

    char szInfo[256] = {0};

    int nRingCount = 0;
    int i = 0;
    int j = 0;
    for (i = 0;i < MAX_RING_COUNT;i ++)
    {
        int nPhaseIndex = 0;
        for (j = 0;j < MAX_SEQUENCE_DATA_LENGTH;j ++)
        {
            int nPhaseNum = (int)atSequenceInfo[i].m_bySequenceData[j];

            if (nPhaseNum == 0)
            {
                break;
            }
            
            m_pOpenATCParameter->GetPhaseByPhaseNumber(nPhaseNum,m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nPhaseIndex].m_tPhaseParam);

			//��Ϊ�����1�������2����С�̰�������������Ҫ������ʱ��ȥ��
			m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nPhaseIndex].m_tPhaseParam.m_wPhaseMaximum1 -= m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nPhaseIndex].m_tPhaseParam.m_byGreenFlash;
            m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nPhaseIndex].m_tPhaseParam.m_wPhaseMaximum2 -= m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nPhaseIndex].m_tPhaseParam.m_byGreenFlash;
			m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nPhaseIndex].m_tPhaseParam.m_wPhaseMinimumGreen -= m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nPhaseIndex].m_tPhaseParam.m_byGreenFlash;
		
            WORD wSplitTime = 0;
            for (int k = 0;k < MAX_PHASE_COUNT;k ++)
            {
                if (m_atSplitInfo[k].m_bySplitPhase == nPhaseNum/* && m_atSplitInfo[k].m_bySplitMode != NEGLECT_MODE*/)
                {
                    wSplitTime = m_atSplitInfo[k].m_wSplitTime;  
                    break; 
                }
            }

            if (wSplitTime == 0)
            {
                continue;
            }
            m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nPhaseIndex].m_wPhaseTime = wSplitTime;
            PTPhase pPhaseInfo = &(m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nPhaseIndex].m_tPhaseParam);
            m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nPhaseIndex].m_wPhaseGreenTime = wSplitTime - pPhaseInfo->m_byGreenFlash - pPhaseInfo->m_byPhaseYellowChange - pPhaseInfo->m_byPhaseRedClear;
            m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nPhaseIndex].m_wPedPhaseGreenTime = wSplitTime - pPhaseInfo->m_byPhaseYellowChange - pPhaseInfo->m_byPhasePedestrianClear - pPhaseInfo->m_byPhaseRedClear;
            m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nPhaseIndex].m_chPedPhaseStage = C_CH_PHASESTAGE_U;
            m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nPhaseIndex].m_chPhaseStage = C_CH_PHASESTAGE_U;

            nPhaseIndex++;
        }

		if (nPhaseIndex != 0)
		{
			nRingCount++;
		}

        m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_nPhaseCount = nPhaseIndex;
        m_tFixTimeCtlInfo.m_bIsChgStage[i] = true;
        m_tFixTimeCtlInfo.m_bIsChgPedStage[i] = true;   

        sprintf(szInfo, "RingIndex:%d PhaseCount:%d", i, nPhaseIndex);
        m_pOpenATCLog->LogOneMessage(LEVEL_INFO, szInfo);         
    }

	m_tFixTimeCtlInfo.m_nRingCount = nRingCount;

    int iPhaseCount = 0;
    for (int iRingIndex = 0; iRingIndex < m_tFixTimeCtlInfo.m_nRingCount; iRingIndex++)
    {
        iPhaseCount = m_tFixTimeCtlInfo.m_atPhaseSeq[iRingIndex].m_nPhaseCount;
        for (i = 0; i < iPhaseCount; i++)
        {
            if (m_tFixTimeCtlInfo.m_atPhaseSeq[iRingIndex].m_atPhaseInfo[i].m_wPhaseTime > 0)
            {
                for (j = 0; j < MAX_PHASE_COUNT; j++)
                {
                    if (m_tFixTimeCtlInfo.m_atPhaseSeq[iRingIndex].m_atPhaseInfo[i].m_tPhaseParam.m_byPhaseNumber == m_atSplitInfo[j].m_bySplitPhase)
                    {
                        m_tFixTimeCtlInfo.m_atPhaseSeq[iRingIndex].m_atPhaseInfo[i].m_chPhaseMode = m_atSplitInfo[j].m_bySplitMode;
                        break;
                    }
                }
            }
        }
    }

	//int nNeglectPhaseCount = 0;
	//int nNeglectPhaseCountInRing[MAX_RING_COUNT];
	//memset(nNeglectPhaseCountInRing,0,sizeof(nNeglectPhaseCountInRing));
	//
	//TRingCtlInfo atPhaseSeq[MAX_RING_COUNT];
	//memset(atPhaseSeq,0,sizeof(atPhaseSeq));

	//for (i = 0;i < MAX_PHASE_COUNT;i ++)
	//{
	//	nNeglectPhaseCount = 0;

	//	for (j = 0;j < MAX_PHASE_COUNT;j ++)
	//	{
	//		if (m_tFixTimeCtlInfo.m_atPhaseSeq[0].m_atPhaseInfo[i].m_wPhaseTime > 0 &&
	//			m_tFixTimeCtlInfo.m_atPhaseSeq[0].m_atPhaseInfo[i].m_tPhaseParam.m_byPhaseNumber == m_atSplitInfo[j].m_bySplitPhase &&
	//			m_atSplitInfo[j].m_bySplitMode == NEGLECT_MODE)
	//		{
	//			nNeglectPhaseCount += 1;
	//			nNeglectPhaseCountInRing[0] += 1;
	//		}

	//		if (m_tFixTimeCtlInfo.m_nRingCount > 0)
	//		{
	//			if (m_tFixTimeCtlInfo.m_atPhaseSeq[1].m_atPhaseInfo[i].m_wPhaseTime > 0 &&
	//				m_tFixTimeCtlInfo.m_atPhaseSeq[1].m_atPhaseInfo[i].m_tPhaseParam.m_byPhaseNumber == m_atSplitInfo[j].m_bySplitPhase &&
	//				m_atSplitInfo[j].m_bySplitMode == NEGLECT_MODE)
	//			{
	//				nNeglectPhaseCount += 1;
	//				nNeglectPhaseCountInRing[1] += 1;
	//			}
	//		}
	//	}

	//	if (nNeglectPhaseCount != m_tFixTimeCtlInfo.m_nRingCount)
	//	{
	//		if (m_tFixTimeCtlInfo.m_atPhaseSeq[0].m_atPhaseInfo[i].m_wPhaseTime > 0)
	//		{
	//			memcpy(&atPhaseSeq[0].m_atPhaseInfo[atPhaseSeq[0].m_nPhaseCount], &m_tFixTimeCtlInfo.m_atPhaseSeq[0].m_atPhaseInfo[i], sizeof(m_tFixTimeCtlInfo.m_atPhaseSeq[0].m_atPhaseInfo[i]));
	//			atPhaseSeq[0].m_nPhaseCount += 1;
	//		}

	//		if (m_tFixTimeCtlInfo.m_nRingCount > 0 && m_tFixTimeCtlInfo.m_atPhaseSeq[1].m_atPhaseInfo[i].m_wPhaseTime > 0)
	//		{
	//			memcpy(&atPhaseSeq[1].m_atPhaseInfo[atPhaseSeq[1].m_nPhaseCount], &m_tFixTimeCtlInfo.m_atPhaseSeq[1].m_atPhaseInfo[i], sizeof(m_tFixTimeCtlInfo.m_atPhaseSeq[1].m_atPhaseInfo[i]));
	//			atPhaseSeq[1].m_nPhaseCount += 1;
	//		}
	//	}
	//}

	//if (nNeglectPhaseCountInRing[0] == m_tFixTimeCtlInfo.m_atPhaseSeq[0].m_nPhaseCount || nNeglectPhaseCountInRing[1] == m_tFixTimeCtlInfo.m_atPhaseSeq[0].m_nPhaseCount)
	//{
	//	m_tFixTimeCtlInfo.m_nRingCount -= 1;
	//}

	//if (nNeglectPhaseCountInRing[0] > 0 || nNeglectPhaseCountInRing[1] > 0)
	//{
	//	memcpy(m_tFixTimeCtlInfo.m_atPhaseSeq, &atPhaseSeq, sizeof(m_tFixTimeCtlInfo.m_atPhaseSeq));
	//}
}

/*==================================================================== 
������ ��Run 
���� �������ڿ��Ʒ�ʽ������ʵ��
�㷨ʵ�� �� 
����˵�� �� 
����ֵ˵����
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ������          �������� 
====================================================================*/ 
void CLogicCtlFixedTime::Run()
{
    //��λ�Ľ�ֹ������
    if (m_pOpenATCRunStatus->GetPhaseControlChange())
    {
        SetChannelShieldAndProhibitStatus();
    }

    //��λ������
    for (int i = 0;i < m_tFixTimeCtlInfo.m_nRingCount;i ++)
    {
        //����ʵʱ����λ��ֹ��������Ϣ�Եƿ�״̬���е���
        if (m_pOpenATCRunStatus->GetPhaseControlChange())
        {
            SetLampByShieldAndProhibitStatus(i, m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_nPhaseCount, m_nChannelCount);
        }

        OnePhaseRun(i);
        OnePedPhaseRun(i);
        StageChg(i);
    }

    if (m_pOpenATCRunStatus->GetPhaseControlChange())
    {
        m_pOpenATCRunStatus->SetPhaseControlChange(false);
    }

    if (m_bIsLampClrChg)
    {
        //SetOverlapPhaseLampClr(m_tFixTimeCtlInfo.m_nCurStageIndex + 1);
        SetOverlapPhaseLampClr();
        
        //����ȫ�ֵ�ɫ״̬
        TLampClrStatus tLampClrStatus;
        memset(&tLampClrStatus,0,sizeof(TLampClrStatus)); 
        m_pOpenATCRunStatus->GetLampClrStatus(tLampClrStatus);
        SetLampClr(tLampClrStatus);
        tLampClrStatus.m_bIsRefreshClr = true;
        m_pOpenATCRunStatus->SetLampClrStatus(tLampClrStatus);

        m_bIsLampClrChg = false;
    }

	//���ø�����λ������
	SetOverLapPulse(false, 0);

    //��λ�л�,����������λ����״̬��Ϻ�������    
    for (int j = 0;j < m_tFixTimeCtlInfo.m_nRingCount;j ++)
    {
        PhaseChg(j);
    }

    CycleChg();
}

/*==================================================================== 
������ ��GetNextPhaseStage 
���� ����ȡ��������λ�׶ε���һ�����н׶�
�㷨ʵ�� �� 
����˵�� ��chCurStage����ǰ��λ�׶� 
           pPhaseInfo����λ����ָ��
           nStageTime�����ؽ׶�����ʱ��
����ֵ˵����chCurStage����Ľ׶ε���һ�����н׶�
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ������          �������� 
====================================================================*/ 
char CLogicCtlFixedTime::GetNextPhaseStageInfo(char chCurStage,const PTPhaseCtlInfo pPhaseInfo,WORD & nStageTime)
{
    char chRet = C_CH_PHASESTAGE_U;
    if (chCurStage == C_CH_PHASESTAGE_U)
    {
        chRet = C_CH_PHASESTAGE_G;
        nStageTime = pPhaseInfo->m_wPhaseGreenTime;
    }
    else if (chCurStage == C_CH_PHASESTAGE_G)
    {
        if (pPhaseInfo->m_tPhaseParam.m_byGreenFlash > 0)
        {
            chRet = C_CH_PHASESTAGE_GF;
            nStageTime = pPhaseInfo->m_tPhaseParam.m_byGreenFlash;
        }
        else if (pPhaseInfo->m_tPhaseParam.m_byPhaseYellowChange > 0)
        {
            chRet = C_CH_PHASESTAGE_Y;
            nStageTime = pPhaseInfo->m_tPhaseParam.m_byPhaseYellowChange;
        }
        else if (pPhaseInfo->m_tPhaseParam.m_byPhaseRedClear > 0)
        {
            chRet = C_CH_PHASESTAGE_R;
            nStageTime = pPhaseInfo->m_tPhaseParam.m_byPhaseRedClear;
        }
        else
        {
            chRet = C_CH_PHASESTAGE_F;
            nStageTime = 0;          
        }
    }
    else if (chCurStage == C_CH_PHASESTAGE_GF)
    {
        if (pPhaseInfo->m_tPhaseParam.m_byPhaseYellowChange > 0)
        {
            chRet = C_CH_PHASESTAGE_Y;
            nStageTime = pPhaseInfo->m_tPhaseParam.m_byPhaseYellowChange;
        }
        else if (pPhaseInfo->m_tPhaseParam.m_byPhaseRedClear > 0)
        {
            chRet = C_CH_PHASESTAGE_R;
            nStageTime = pPhaseInfo->m_tPhaseParam.m_byPhaseRedClear;
        }
        else
        {
            chRet = C_CH_PHASESTAGE_F;
            nStageTime = 0;          
        }
    }
    else if (chCurStage == C_CH_PHASESTAGE_Y)
    {
        if (pPhaseInfo->m_tPhaseParam.m_byPhaseRedClear > 0)
        {
            chRet = C_CH_PHASESTAGE_R;
            nStageTime = pPhaseInfo->m_tPhaseParam.m_byPhaseRedClear;
        }
        else
        {
            chRet = C_CH_PHASESTAGE_F;
            nStageTime = 0;
        }
    }
    else if (chCurStage == C_CH_PHASESTAGE_R)
    {
        chRet = C_CH_PHASESTAGE_F;
        nStageTime = 0;        
    }
    else if (chCurStage == C_CH_PHASESTAGE_F)
    {
        chRet = C_CH_PHASESTAGE_F;
        nStageTime = 0;        
    }
    else
    {
        chRet = C_CH_PHASESTAGE_U;
        nStageTime = 0;
    }

    return chRet;
}

/*==================================================================== 
������ ��GetNextPhaseStage 
���� ����ȡ������λ�׶ε���һ�����н׶�
�㷨ʵ�� �� 
����˵�� ��chCurStage����ǰ��λ�׶� 
           pPhaseInfo����λ����ָ��
           nStageTime�����ؽ׶�����ʱ��
����ֵ˵����chCurStage����Ľ׶ε���һ�����н׶�
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ������          �������� 
====================================================================*/ 
char CLogicCtlFixedTime::GetPedNextPhaseStageInfo(char chCurStage,const PTPhaseCtlInfo pPhaseInfo,WORD & wStageTime)
{
    char chRet = C_CH_PHASESTAGE_U;
    if (chCurStage == C_CH_PHASESTAGE_U)
    {
        chRet = C_CH_PHASESTAGE_G;
        wStageTime = pPhaseInfo->m_wPedPhaseGreenTime;
    }
    else if (chCurStage == C_CH_PHASESTAGE_G)
    {
        if (pPhaseInfo->m_tPhaseParam.m_byPhasePedestrianClear > 0)
        {
            chRet = C_CH_PHASESTAGE_GF;
            wStageTime = pPhaseInfo->m_tPhaseParam.m_byPhasePedestrianClear;
        }
        else if (pPhaseInfo->m_tPhaseParam.m_byPhaseRedClear > 0)
        {
            chRet = C_CH_PHASESTAGE_R;
            wStageTime = pPhaseInfo->m_tPhaseParam.m_byPhaseRedClear + pPhaseInfo->m_tPhaseParam.m_byPhaseYellowChange;
        }
        else
        {
            chRet = C_CH_PHASESTAGE_F;
            wStageTime = 0;          
        }
    }
    else if (chCurStage == C_CH_PHASESTAGE_GF)
    {
        if (pPhaseInfo->m_tPhaseParam.m_byPhaseRedClear > 0)
        {
            chRet = C_CH_PHASESTAGE_R;
            wStageTime = pPhaseInfo->m_tPhaseParam.m_byPhaseRedClear;
        }
        else
        {
            chRet = C_CH_PHASESTAGE_F;
            wStageTime = 0;          
        }
    }
    else if (chCurStage == C_CH_PHASESTAGE_R)
    {
        chRet = C_CH_PHASESTAGE_F;
        wStageTime = 0;        
    }
    else if (chCurStage == C_CH_PHASESTAGE_F)
    {
        chRet = C_CH_PHASESTAGE_F;
        wStageTime = 0;        
    }
    else
    {
        chRet = C_CH_PHASESTAGE_U;
        wStageTime = 0;
    }

    return chRet;
}

/*==================================================================== 
������ ��GetPhaseRemainTime
���� ����ȡ��λ��ʣ�µ�ʱ��
�㷨ʵ�� �� 
����˵�� ��pPhaseCtlInfo����λ����������״̬��Ϣ 
����ֵ˵������λ������ʣ�������ʱ��
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ������          �������� 
====================================================================*/ 
int CLogicCtlFixedTime::GetPhaseRemainTime(TPhaseCtlInfo * pPhaseCtlInfo)
{

    return 1;
}

/*==================================================================== 
������ ��SetLampClrByRing
���� �����õ�����λ�ĵ�ɫ
�㷨ʵ�� �� 
����˵�� ��nRingIndex��������
           nPhaseIndex����ǰ��λ����
           chPhaseStage����ǰ��λ�׶�
����ֵ˵����
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ������          �������� 
====================================================================*/ 
void CLogicCtlFixedTime::SetLampClrByRing(int nRingIndex, int nPhaseIndex, char chPhaseStage, char chPedPhaseStage)
{
    PTRingCtlInfo ptRingRunInfo = &(m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex]);
   
    char szInfo[256] = {0};
   
    SetChannelStatus((int)ptRingRunInfo->m_atPhaseInfo[nPhaseIndex].m_tPhaseParam.m_byPhaseNumber,PHASE_SRC,chPhaseStage,chPedPhaseStage);            
    sprintf(szInfo, "SetLampClrByRing RingIndex:%d PhaseNumber:%d PhaseStage:%c PedPhaseStage:%c", nRingIndex, (int)ptRingRunInfo->m_atPhaseInfo[nPhaseIndex].m_tPhaseParam.m_byPhaseNumber, chPhaseStage, chPedPhaseStage);
    m_pOpenATCLog->LogOneMessage(LEVEL_INFO, szInfo);
}

/*==================================================================== 
������ ��OnePhaseRun 
���� ��������������λ����״̬����
�㷨ʵ�� �� 
����˵�� ��nRingIndex�������� 
����ֵ˵����
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ������          �������� 
====================================================================*/ 
void CLogicCtlFixedTime::OnePhaseRun(int nRingIndex)
{
    char szInfo[256] = {0};
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
        char chNextStage =  this->GetNextPhaseStageInfo(chcurStage,pPhaseInfo,wStageTime);
        ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseStage = chNextStage;
        ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPhaseStageRunTime = wStageTime;

        m_tFixTimeCtlInfo.m_bIsChgStage[nRingIndex] = false;

        sprintf(szInfo, "OnePhaseRun RingIndex:%d CurPhaseIndex:%d CurStage:%c NextStage:%c StageTime:%d", nRingIndex, nIndex, chcurStage, chNextStage, wStageTime);
        m_pOpenATCLog->LogOneMessage(LEVEL_INFO, szInfo);

		if (chNextStage == C_CH_PHASESTAGE_GF)
        {
            GetGreenFalshCount(VEH_CHA, (PTPhaseCtlInfo)&(ptRingRunInfo->m_atPhaseInfo[nIndex]), wStageTime);
        }

        //���û�����λ��ɫ������ʱ��
        SetLampClrByRing(nRingIndex,nIndex,ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseStage,ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPedPhaseStage);
        tRunCounter.m_nLampClrTime[nRingIndex] = 0;
        tRunCounter.m_nLampClrStartCounter[nRingIndex] = tRunCounter.m_nCurCounter;    
        m_pOpenATCRunStatus->SetPhaseLampClrRunCounter(tRunCounter);
		if (chcurStage != chNextStage)
		{
			m_bIsLampClrChg = true;
		}
		if (chcurStage == C_CH_PHASESTAGE_F && chNextStage == C_CH_PHASESTAGE_F)
		{
			m_bIsLampClrChg = false;
		}
    } 

    static bool bGreenPulse[MAX_RING_COUNT] = {false, false, false, false};
    static bool bRedPulse[MAX_RING_COUNT] = {false, false, false, false};
    
    if (ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseMode != NEGLECT_MODE && ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseMode != SHIELD_MODE &&
		ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseStage == C_CH_PHASESTAGE_G && tRunCounter.m_nLampClrTime[nRingIndex] == 
       (ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPhaseStageRunTime + ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byGreenFlash - ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byGreenPulse))
    {
        if (!bGreenPulse[nRingIndex])
        {
            //m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "SetGreenLampPulse RingIndex:%d PhaseIndex:%d bGreenPulse:%d", nRingIndex, nIndex, 1);
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
           //m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "SetGreenLampPulse RingIndex:%d PhaseIndex:%d bGreenPulse:%d", nRingIndex, nIndex, 0);
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

    if (ptRingRunInfo->m_atPhaseInfo[nNextIndex].m_chPhaseMode != NEGLECT_MODE && ptRingRunInfo->m_atPhaseInfo[nNextIndex].m_chPhaseMode != SHIELD_MODE && 
		ptRingRunInfo->m_atPhaseInfo[nNextIndex].m_tPhaseParam.m_byRedPulse != 0 && GetPhaseCycleRemainTime(nRingIndex, (PTPhaseCtlInfo)&(ptRingRunInfo->m_atPhaseInfo[nIndex]), tRunCounter) == ptRingRunInfo->m_atPhaseInfo[nNextIndex].m_tPhaseParam.m_byRedPulse)
    {
        if (!bRedPulse[nRingIndex])
        {
            //m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "SetRedLampPulse RingIndex:%d PhaseIndex:%d bRedPulse:%d", nRingIndex, nIndex, 1);
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
            //m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "SetRedLampPulse RingIndex:%d PhaseIndex:%d bRedPulse:%d", nRingIndex, nIndex, 0);
            bRedPulse[nRingIndex] = false;
            SetRedLampPulse(VEH_CHA, ptRingRunInfo->m_atPhaseInfo[nNextIndex].m_tPhaseParam.m_byPhaseNumber, false);  

			m_tPhasePulseStatus[nRingIndex].m_nPhaseIndex = nIndex;
			m_tPhasePulseStatus[nRingIndex].m_nPhaseNum = ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhaseNumber;
			m_tPhasePulseStatus[nRingIndex].m_bRedPulseStatus = false;
        }
    }
}

/*==================================================================== 
������ ��OnePedPhaseRun
���� ������������λ����״̬����
�㷨ʵ�� �� 
����˵�� ��nRingIndex�������� 
����ֵ˵����
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ������          �������� 
====================================================================*/ 
void CLogicCtlFixedTime::OnePedPhaseRun(int nRingIndex)
{
    PTRingCtlInfo ptRingRunInfo = &(m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex]);
        
    int nIndex = m_tFixTimeCtlInfo.m_nCurPhaseIndex[nRingIndex];
    TPhaseLampClrRunCounter tRunCounter;
    m_pOpenATCRunStatus->GetPhaseLampClrRunCounter(tRunCounter);

    //��ǰ��λ�׶��Ƿ����
    if (tRunCounter.m_nPedLampClrTime[nRingIndex] >= ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPedPhaseStageRunTime)
    {
        m_tFixTimeCtlInfo.m_bIsChgPedStage[nRingIndex] = true;

        //m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "OnePedPhaseRun RingIndex:%d CurPedPhaseIndex:%d PedLampClrTime:%d PhasePedStageRunTime:%d", nRingIndex, nIndex, tRunCounter.m_nPedLampClrTime[nRingIndex], ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPedPhaseStageRunTime);
    }

    if (m_tFixTimeCtlInfo.m_bIsChgPedStage[nRingIndex])
    {
        char chcurPedStage = ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPedPhaseStage;
        PTPhaseCtlInfo pPhaseInfo = (PTPhaseCtlInfo)&(ptRingRunInfo->m_atPhaseInfo[nIndex]);
        WORD wPedStageTime = 0;
        char chNextPedStage =  this->GetPedNextPhaseStageInfo(chcurPedStage,pPhaseInfo,wPedStageTime);
        ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPedPhaseStage = chNextPedStage;
        ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPedPhaseStageRunTime = wPedStageTime;

        //m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "OnePedPhaseRun RingIndex:%d CurPedPhaseIndex:%d CurPedStage:%c NextPedStage:%c PedStageTime:%d", nRingIndex, nIndex, chcurPedStage, chNextPedStage, wPedStageTime);

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
        if (chcurPedStage != chNextPedStage)
		{
			m_bIsLampClrChg = true;
		}
    } 
   
    static bool bGreenPulse[MAX_RING_COUNT] = {false, false, false, false};
    static bool bRedPulse[MAX_RING_COUNT] = {false, false, false, false};
   
    if (ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseMode != NEGLECT_MODE && ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseMode != SHIELD_MODE &&
		ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPedPhaseStage == C_CH_PHASESTAGE_G && tRunCounter.m_nPedLampClrTime[nRingIndex] == 
       (ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPedPhaseStageRunTime + ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhasePedestrianClear - ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byGreenPulse))
    {
        if (!bGreenPulse[nRingIndex])
        {
            m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "SetGreenLampPulse Ped RingIndex:%d PhaseIndex:%d bGreenPulse:%d", nRingIndex, nIndex, 1);
            bGreenPulse[nRingIndex] = true;
            SetGreenLampPulse(PED_CHA, ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhaseNumber, true);
        }
    }
    else
    {
        if (bGreenPulse[nRingIndex])
        {
            m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "SetGreenLampPulse Ped RingIndex:%d PhaseIndex:%d bGreenPulse:%d", nRingIndex, nIndex, 0);
            bGreenPulse[nRingIndex] = false;
            SetGreenLampPulse(PED_CHA, ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhaseNumber, false);
        }
    }
    
	int nNextIndex = nIndex + 1;
    if (nNextIndex == ptRingRunInfo->m_nPhaseCount)
    {
        nNextIndex = 0;
    }

    if (ptRingRunInfo->m_atPhaseInfo[nNextIndex].m_chPhaseMode != NEGLECT_MODE && ptRingRunInfo->m_atPhaseInfo[nNextIndex].m_chPhaseMode != SHIELD_MODE &&
		ptRingRunInfo->m_atPhaseInfo[nNextIndex].m_tPhaseParam.m_byRedPulse != 0 && GetPedPhaseCycleRemainTime(nRingIndex, (PTPhaseCtlInfo)&(ptRingRunInfo->m_atPhaseInfo[nIndex]), tRunCounter) == ptRingRunInfo->m_atPhaseInfo[nNextIndex].m_tPhaseParam.m_byRedPulse)
    {
        if (!bRedPulse[nRingIndex])
        {
            m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "SetRedLampPulse Ped RingIndex:%d PhaseIndex:%d bRedPulse:%d", nRingIndex, nIndex, 1);
            bRedPulse[nRingIndex] = true;
            SetRedLampPulse(PED_CHA, ptRingRunInfo->m_atPhaseInfo[nNextIndex].m_tPhaseParam.m_byPhaseNumber, true);  
        }
    } 
    else
    {
        if (bRedPulse[nRingIndex])
        {
            m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "SetRedLampPulse Ped RingIndex:%d PhaseIndex:%d bRedPulse:%d", nRingIndex, nIndex, 0);
            bRedPulse[nRingIndex] = false;
            SetRedLampPulse(PED_CHA, ptRingRunInfo->m_atPhaseInfo[nNextIndex].m_tPhaseParam.m_byPhaseNumber, false);  
        }
    }
}

/*==================================================================== 
������ ��PhaseChg
���� ����λ�Ƿ�����л�״̬�ж�
�㷨ʵ�� �� 
����˵�� ��nRingIndex�������� 
����ֵ˵����
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ������          �������� 
====================================================================*/ 
void CLogicCtlFixedTime::PhaseChg(int nRingIndex)
{
	bool bPhaseChg[MAX_RING_COUNT];
    memset(bPhaseChg, 0x00, sizeof(bPhaseChg));
    int  i = 0, j = 0;
    int  nConcurrencyCount = 0;
    PTRingCtlInfo ptRingRunInfo = &(m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex]);
    int nIndex = m_tFixTimeCtlInfo.m_nCurPhaseIndex[nRingIndex];
    if (ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseStage == C_CH_PHASESTAGE_F)
    {
        //�Ƿ����һ����λ
        if (nIndex < ptRingRunInfo->m_nPhaseCount - 1)
        {
            for (i = 0;i < MAX_PHASE_CONCURRENCY_COUNT;i ++)
            {
                BYTE byPhaseNum = ptRingRunInfo->m_atPhaseInfo[nIndex+1].m_tPhaseParam.m_byPhaseConcurrency[i];
                if (byPhaseNum == 0)
                {
                    break;
                }

                int nDstRingIndex = 0;
                int nDstPhaseIndex = 0;
                bool bRun = false;
                bool bNextRun = false;

                if (GetPhaseIndexByPhaseNumber(nDstRingIndex, nDstPhaseIndex, (int)byPhaseNum))
                {
                     PTRingCtlInfo ptDstRingInfo = &(m_tFixTimeCtlInfo.m_atPhaseSeq[nDstRingIndex]);

                     if (nDstPhaseIndex == m_tFixTimeCtlInfo.m_nCurPhaseIndex[nDstRingIndex])
                     {
                         if (m_bLastPhaseBeforeBarrier[nDstRingIndex][m_tFixTimeCtlInfo.m_nCurPhaseIndex[nDstRingIndex]])//����ǰ�����һ����λ
                         {
                             //if (ptDstRingInfo->m_atPhaseInfo[nDstPhaseIndex].m_chPhaseStage != C_CH_PHASESTAGE_F)
                             {
                                 bRun = true;

                                 //m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "PhaseChg Run RingIndex:%d DstRingIndex:%d DstPhaseIndex:%d", nRingIndex, nDstRingIndex, nDstPhaseIndex);
                             }
                         }
                         else
                         {
                             bRun = true;

                             //m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "PhaseChg Run RingIndex:%d DstRingIndex:%d DstPhaseIndex:%d", nRingIndex, nDstRingIndex, nDstPhaseIndex);
                         }
                    }

                    if (nDstPhaseIndex == m_tFixTimeCtlInfo.m_nCurPhaseIndex[nDstRingIndex] + 1)
                    {
                        if (m_bLastPhaseBeforeBarrier[nDstRingIndex][m_tFixTimeCtlInfo.m_nCurPhaseIndex[nDstRingIndex]] ||
                            !IsConcurrencyPhase(nRingIndex, nDstRingIndex))//����ǰ�����һ����λ
                        {
                             if (ptDstRingInfo->m_atPhaseInfo[nDstPhaseIndex - 1].m_chPhaseStage == C_CH_PHASESTAGE_F)
                             {
                                 bNextRun = true;

                                 //m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "PhaseChg NexRun RingIndex:%d DstRingIndex:%d DstPhaseIndex:%d", nRingIndex, nDstRingIndex, nDstPhaseIndex);
                             }
                        }
                        else
                        {
                             bNextRun = true;

                             //m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "PhaseChg NexRun RingIndex:%d DstRingIndex:%d DstPhaseIndex:%d", nRingIndex, nDstRingIndex, nDstPhaseIndex);
                        }
                    }

                    if (bRun || bNextRun)
                    {
                        bPhaseChg[nDstRingIndex] = true;
                    } 
                }
            }
            
            if (m_tFixTimeCtlInfo.m_nRingCount == 1)
            {
                //����ת��λ�ж�
                m_tFixTimeCtlInfo.m_nCurPhaseIndex[nRingIndex]++; 
                ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseStage = C_CH_PHASESTAGE_U;   
                ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPedPhaseStage = C_CH_PHASESTAGE_U;  
				ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPhaseStageRunTime = 0;
				ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPedPhaseStageRunTime = 0;
   
                m_pOpenATCLog->LogOneMessage(LEVEL_INFO, "PhaseChg RingIndex:%d CurPhaseIndex:%d", nRingIndex, m_tFixTimeCtlInfo.m_nCurPhaseIndex[nRingIndex]);
            }
            else
            {
				nConcurrencyCount = 0;
				 for (j = 0;j < m_tFixTimeCtlInfo.m_nRingCount;j++)
				 {
			         if (bPhaseChg[j])
					 {
                         nConcurrencyCount += 1;
					 }
				 }
                 if (nConcurrencyCount == (m_tFixTimeCtlInfo.m_nRingCount - 1))
                 {
                     //����ת��λ�ж�
                     m_tFixTimeCtlInfo.m_nCurPhaseIndex[nRingIndex]++; 
                     ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseStage = C_CH_PHASESTAGE_U;   
                     ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPedPhaseStage = C_CH_PHASESTAGE_U;  
					 ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPhaseStageRunTime = 0;
					 ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPedPhaseStageRunTime = 0;

                     m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "PhaseChg RingIndex:%d CurPhaseIndex:%d ConcurrencyCount:%d", nRingIndex, m_tFixTimeCtlInfo.m_nCurPhaseIndex[nRingIndex], nConcurrencyCount);
                 }   
             }  
        }          
    }
}

/*==================================================================== 
������ ��CycleChg
���� �������Ƿ����н���״̬�ж�
�㷨ʵ�� �� 
����˵�� ��
����ֵ˵����
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ������          �������� 
====================================================================*/ 
void CLogicCtlFixedTime::CycleChg()
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
             sprintf(szInfo, "CycleChg RingIndex:%d CurPhaseIndex:%d", i, nIndex);
             m_pOpenATCLog->LogOneMessage(LEVEL_INFO, szInfo);       
        }
        else
        {
            bCycleChg = false;
            break;    
        }
    }

    //��ǰ�����Ƿ����
    if (bCycleChg)
    {
        ReSetCtlStatus();
        m_tFixTimeCtlInfo.m_wCycleRunTime = 0;  

		m_pOpenATCRunStatus->SetCycleChgStatus(true);

        SetGetParamFlag(true); 

		int i = 0;

		TLogicCtlStatus tCtlStatus;
		m_pOpenATCRunStatus->GetLogicCtlStatus(tCtlStatus);
		if (tCtlStatus.m_nCurCtlMode == CTL_MODE_ACTUATE)
		{
			for (i = 0;i < MAX_PHASE_COUNT;i++)
			{
				if (m_bGreenTimeExceedMax1Time[i])
				{
					m_nGreenTimeIsMax1Time[i]++;
					if (m_nGreenTimeIsMax1Time[i] > C_N_MAXGLOBALCOUNTER)
					{
						m_nGreenTimeIsMax1Time[i] = GREEN_TIME_CALCULATE_TIME;
					}

					m_nGreenTimeNotExceedMax1Time[i] = 0;
				}
				else
				{
					m_nGreenTimeNotExceedMax1Time[i]++;
					if (m_nGreenTimeNotExceedMax1Time[i] > C_N_MAXGLOBALCOUNTER)
					{
						m_nGreenTimeNotExceedMax1Time[i] = GREEN_TIME_CALCULATE_TIME;
					}

					if (m_nGreenTimeIsMax1Time[i] >= GREEN_TIME_CALCULATE_TIME)
					{
						if (m_nGreenTimeNotExceedMax1Time[i] == GREEN_TIME_CALCULATE_TIME)
						{
							m_nGreenTimeIsMax1Time[i] = 0;
						}
					}
					else
					{
						m_nGreenTimeIsMax1Time[i] = 0;
					}
				}		
			}

			memset(m_bGreenTimeExceedMax1Time, 0, sizeof(m_bGreenTimeExceedMax1Time));  
		}

		for (i = 0;i < MAX_RING_COUNT;i++)
		{
			m_tPhasePulseStatus[i].m_nPhaseIndex = 0;
			m_tPhasePulseStatus[i].m_nPhaseNum = m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[m_tPhasePulseStatus[i].m_nPhaseIndex].m_tPhaseParam.m_byPhaseNumber;
			m_tPhasePulseStatus[i].m_bGreenPulseStatus = false;
			m_tPhasePulseStatus[i].m_bRedPulseStatus = false;
			m_tPhasePulseStatus[i].m_nGreenPulseSendStatus = SEND_INIT;
			m_tPhasePulseStatus[i].m_nRedPulseSendStatus = SEND_INIT;
		}
    }
}

/*==================================================================== 
������ ��GetPhaseIndexByPhaseNumber
���� ��������λ�Ų�����λ���ڵĻ������ͻ��ڵ���λ����
�㷨ʵ�� �� 
����˵�� ��nRingIndex�����ػ�����
           nPhaseIndex�����ػ��ڵ���λ����
           nPhaseNum��Ҫ���ҵ���λ��
����ֵ˵�������ҳɹ�����true����֮����false��
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ������          �������� 
====================================================================*/ 
bool CLogicCtlFixedTime::GetPhaseIndexByPhaseNumber(int & nRingIndex,int & nPhaseIndex,int nPhaseNum)
{
    bool bFind = false;
    nRingIndex = -1;
    nPhaseIndex = -1;

    for (int i = 0;i < m_tFixTimeCtlInfo.m_nRingCount;i ++)
    {
        PTRingCtlInfo ptRingRunInfo = &(m_tFixTimeCtlInfo.m_atPhaseSeq[i]);
        
        for (int j = 0;j < ptRingRunInfo->m_nPhaseCount;j ++)
        {
            if (nPhaseNum == (int)(ptRingRunInfo->m_atPhaseInfo[j].m_tPhaseParam.m_byPhaseNumber))
            {
                bFind = true;
                nRingIndex = i;
                nPhaseIndex = j;
                break;
            }
        }            
    }

    return bFind;
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
void CLogicCtlFixedTime::SetOverlapPhaseLampClr(int nNextStageIndex)
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
		int  m = 0;
		char chRet = 0;
		char chPedRet = 0;

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
				GetPhaseIndexByPhaseNumber(nRingIndex, nPhaseIndex, (int)byMainPhaseNum);
				chRet = GetPhaseStatus(byMainPhaseNum, false);

				int nStageIndex = m_tFixTimeCtlInfo.m_nCurStageIndex;
				for (m = 0;m < m_tRunStageInfo.m_PhaseRunstageInfo[nStageIndex].m_nConcurrencyPhaseCount;m++)
				{
					if (m_tRunStageInfo.m_PhaseRunstageInfo[nStageIndex].m_nConcurrencyPhase[m] == byMainPhaseNum)
					{
						if (chRet == C_CH_PHASESTAGE_G || (chRet == C_CH_PHASESTAGE_U && m_tFixTimeCtlInfo.m_atOverlapInfo[i].m_chOverlapStage == C_CH_PHASESTAGE_G))
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
						if (m_tRunStageInfo.m_PhaseRunstageInfo[nNextStageIndex].m_nConcurrencyPhase[m] == byMainPhaseNum && m_tRunStageInfo.m_PhaseRunstageInfo[nStageIndex].m_nConcurrencyPhase[m] != m_tRunStageInfo.m_PhaseRunstageInfo[nNextStageIndex].m_nConcurrencyPhase[m])
						{
							bGreenFlag = true;
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
						}
					}

					if (bGreenFlag)
					{
						break;
					}
				}

				chRet = GetPhaseStatus(byMainPhaseNum, false);
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
				GetPhaseIndexByPhaseNumber(nRingIndex, nPhaseIndex, (int)byMainPhaseNum);
				chPedRet = GetPhaseStatus(byMainPhaseNum, true);

				int nStageIndex = m_tFixTimeCtlInfo.m_nCurStageIndex;
				for (m = 0;m < m_tRunStageInfo.m_PhaseRunstageInfo[nStageIndex].m_nConcurrencyPhaseCount;m++)
				{
					if (m_tRunStageInfo.m_PhaseRunstageInfo[nStageIndex].m_nConcurrencyPhase[m] == byMainPhaseNum)
					{
						if (chPedRet == C_CH_PHASESTAGE_G || (chPedRet == C_CH_PHASESTAGE_U && m_tFixTimeCtlInfo.m_atOverlapInfo[i].m_chPedOverlapStage == C_CH_PHASESTAGE_G))
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
						if (m_tRunStageInfo.m_PhaseRunstageInfo[nNextStageIndex].m_nConcurrencyPhase[m] == byMainPhaseNum && 
							m_tRunStageInfo.m_PhaseRunstageInfo[nStageIndex].m_nConcurrencyPhase[m] != m_tRunStageInfo.m_PhaseRunstageInfo[nNextStageIndex].m_nConcurrencyPhase[m])
						{
							bPedGreenFlag = true;
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
						}
					}

					if (bPedGreenFlag)
					{
						break;
					}
				}

				chPedRet = GetPhaseStatus(byMainPhaseNum, true);
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
������ ��GetPhaseStatus
���� ����ȡ��λ��ǰ������״̬
�㷨ʵ�� �� 
����˵�� ��nPhaseNum����λ��
           bPedPhase,������λ��־
����ֵ˵����������λ��ǰ������״̬
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ������          �������� 
====================================================================*/ 
char CLogicCtlFixedTime::GetPhaseStatus(int nPhaseNum, bool bPedPhase)
{
    char chRet = C_CH_PHASESTAGE_U;

    int nDstRingIndex = 0;
    int nDstPhaseIndex = 0;

    if (GetPhaseIndexByPhaseNumber(nDstRingIndex, nDstPhaseIndex, nPhaseNum))
    {
        PTRingCtlInfo ptDstRingInfo = &(m_tFixTimeCtlInfo.m_atPhaseSeq[nDstRingIndex]);
		if (bPedPhase)
		{
			chRet = ptDstRingInfo->m_atPhaseInfo[nDstPhaseIndex].m_chPedPhaseStage;
		}
		else
		{
			chRet = ptDstRingInfo->m_atPhaseInfo[nDstPhaseIndex].m_chPhaseStage;
		}
    }  

	if (m_byPlanID == MAX_PATTERN_COUNT)
	{
		for (int i = 0;i < MAX_PHASE_COUNT;i ++)
		{
			if (m_atSplitInfo[i].m_bySplitPhase == nPhaseNum && m_atSplitInfo[i].m_bySplitMode == NEGLECT_MODE)
			{
				chRet = C_CH_PHASESTAGE_OF;
			}
		} 
	}

    return chRet;
}

/*==================================================================== 
������ ��IsPhaseNumInOverlap
���� ����λ�Ƿ��Ǹ�����λ��ĸ��λ
�㷨ʵ�� �� 
����˵�� ��nPhaseNum����λ��
           nOverlapIndex��������λ����
����ֵ˵������λ�Ǹ�����λ��ĸ��λ����true����֮����false��
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ������          �������� 
====================================================================*/ 
bool CLogicCtlFixedTime::IsPhaseNumInOverlap(int nPhaseNum,int nOverlapIndex)
{
    bool bFind = false;

    for (int j = 0;j < MAX_PHASE_COUNT_IN_OVERLAP;j ++)
    {
        BYTE byMainPhaseNum = m_tFixTimeCtlInfo.m_atOverlapInfo[nOverlapIndex].m_tOverlapParam.m_byArrOverlapIncludedPhases[j];
        
        if (nPhaseNum == (int) byMainPhaseNum)
        {
            bFind = true;
            break;
        }
    }
    
    return bFind;
}

/*==================================================================== 
������ ��SetChannelStatus
���� ������ͨ����״̬
�㷨ʵ�� �� 
����˵�� ��nPhaseNum����λ��
           nPhaseSrc����λԴ������λ���߸�����λ
           chPhaseStage����������λ��״̬
           chPedPhaseStage��������λ��״̬
����ֵ˵����
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ������          �������� 
====================================================================*/ 
void CLogicCtlFixedTime::SetChannelStatus(int nPhaseNum, int nPhaseSrc, char chPhaseStage, char chPedPhaseStage)
{
    char szInfo[256] = {0};

    char * pStartPos = NULL;
    for (int i = 0;i < m_nChannelCount;i++)
    {
		if (m_atChannelInfo[i].m_byChannelNumber == 0)
		{
			continue;
		}

        pStartPos = m_achLampClr + (m_atChannelInfo[i].m_byChannelNumber - 1) * 3;
        if ((int)m_atChannelInfo[i].m_byChannelControlSource == nPhaseNum)
        {
            //ͨ����ֹ������
            if ((m_bProhibitStatus[i] == true) && (m_atChannelInfo[i].m_byChannelControlType == VEH_CHA))
            {
                SetOneChannelOutput(pStartPos, C_CH_PHASESTAGE_OF);
            }
            else if ((m_bShieldStatus[i] == true) && (m_atChannelInfo[i].m_byChannelControlType == VEH_CHA))
            {
                SetOneChannelOutput(pStartPos, C_CH_PHASESTAGE_R);
            }
            else if (m_nChannelSplitMode[i] == NEGLECT_MODE)
            {
                SetOneChannelOutput(pStartPos, C_CH_PHASESTAGE_OF);
            }
            else if (m_nChannelSplitMode[i] == SHIELD_MODE)
            {
                SetOneChannelOutput(pStartPos, C_CH_PHASESTAGE_R);
            }
            else if (m_nChannelSplitMode[i] != NEGLECT_MODE && m_nChannelSplitMode[i] != SHIELD_MODE)//ͨ����Ӧ����λ���Ǻ�����λ,���ǹض���λ 
            {
                if (nPhaseSrc == PHASE_SRC)
                {   
                    if (m_atChannelInfo[i].m_byChannelControlType == VEH_CHA)//����λ�Ļ�������λͨ��
                    {
                        SetOneChannelOutput(pStartPos,chPhaseStage);        
                    }
                    else if (m_atChannelInfo[i].m_byChannelControlType == PED_CHA)//����λ��������λͨ��
                    {
                        SetOneChannelOutput(pStartPos,chPedPhaseStage);
                    }    
                }
                else
                {
                    if (m_atChannelInfo[i].m_byChannelControlType == OVERLAP_CHA)//������λͨ��
                    {                
                        SetOneChannelOutput(pStartPos,chPhaseStage);
					    //m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "SetOverlapPhaseLampClr ChannelIndex:%d To %c", i, chPhaseStage);
                    }  
					else if (m_atChannelInfo[i].m_byChannelControlType == OVERLAP_PED_CHA)//����������λͨ��
					{
						SetOneChannelOutput(pStartPos,chPedPhaseStage);
						//m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "SetOverlapPhaseLampClr ChannelIndex:%d To %c", i, chPedPhaseStage);
					}
                }

				if (m_bKeepGreenChannelBeforeControlChannelFlag[i])
				{
					SetOneChannelOutput(pStartPos,C_CH_PHASESTAGE_G);  
				}
            }
            else
            {
                SetOneChannelOutput(pStartPos,C_CH_PHASESTAGE_OF);
            }

            sprintf(szInfo, "SetChannelStatus PhaseNum:%d PhaseStage:%c PedPhaseStage:%c", nPhaseNum, chPhaseStage, chPedPhaseStage);
            m_pOpenATCLog->LogOneMessage(LEVEL_INFO, szInfo); 

        }
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
void CLogicCtlFixedTime::ReSetCtlStatus()
{
    for (int i = 0;i < MAX_RING_COUNT;i ++)
    {
        for (int j = 0;j < MAX_SEQUENCE_TABLE_COUNT;j ++)
        {
            m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[j].m_chPedPhaseStage = C_CH_PHASESTAGE_U;
            m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[j].m_chPhaseStage = C_CH_PHASESTAGE_U;
        }
        m_tFixTimeCtlInfo.m_nCurPhaseIndex[i] = 0; 
        m_tFixTimeCtlInfo.m_nCurStageIndex = 0; 
    }
}

/*==================================================================== 
������ ��RetsetAllChannelStatus
���� ���Ѳ��������ѵ�ͨ������Ϊ��ƣ�δ���õ�ͨ������Ϊ�ص�
�㷨ʵ�� �� 
����˵�� ��
����ֵ˵����
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ����Ƽ          �������� 
====================================================================*/ 
void CLogicCtlFixedTime::RetsetAllChannelStatus()
{
    char * pStartPos = NULL;
    int i = 0;

    //��ȫ����ʼ��Ϊ�ص�
    for (i = 0;i < MAX_CHANNEL_COUNT;i ++)
    {
        pStartPos = m_achLampClr + i * C_N_CHANNEL_OUTPUTNUM;

        SetOneChannelOutput(pStartPos,C_CH_PHASESTAGE_OF);
    }

    //�������������õ�ͨ������Ϊ���
    for (i = 0;i < m_nChannelCount;i++)
    {
		if (m_atChannelInfo[i].m_byChannelNumber == 0)
		{
			continue;
		}

        if (m_nChannelSplitMode[i] != NEGLECT_MODE && m_atChannelInfo[i].m_byChannelControlType != LANEWAY_LIGHT_CHA)//ͨ����Ӧ����λ���Ǻ�����λ
        {
            pStartPos = m_achLampClr + (m_atChannelInfo[i].m_byChannelNumber - 1) * 3;
            SetOneChannelOutput(pStartPos,C_CH_PHASESTAGE_R);
        }
    }   
}

/*==================================================================== 
������ ��GetCurCtlParam
���� ����ȡ��ǰ���Ʒ�ʽʹ�õĿ��Ʋ���ָ������ݴ�С
�㷨ʵ�� �� 
����˵�� ��
����ֵ˵����
        ����ָ�롣
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ������          �������� 
====================================================================*/ 
void * CLogicCtlFixedTime::GetCurCtlParam()
{
    return (void *)&m_tFixTimeCtlInfo;
}

/*==================================================================== 
������ ��GetPhaseRunStatus
���� �����ݿ��Ʒ�ʽ������ȫ������״̬
�㷨ʵ�� �� 
����˵�� ��tRunStatus������״̬�ṹ������
����ֵ˵����
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ������          �������� 
====================================================================*/ 
void CLogicCtlFixedTime::GetPhaseRunStatus(TPhaseRunStatus & tRunStatus)
{
    int i = 0;
    int j = 0;

    TLogicCtlStatus tCtlStatus;
    m_pOpenATCRunStatus->GetLogicCtlStatus(tCtlStatus);

    WORD wCycleLen = m_tFixTimeCtlInfo.m_wCycleLen;
    if (tCtlStatus.m_nCurCtlMode == CTL_MODE_ACTUATE || tCtlStatus.m_nCurCtlMode == CTL_MODE_CABLELESS || tCtlStatus.m_nCurCtlMode == CTL_MODE_PEDCROSTREET ||
		tCtlStatus.m_nCurCtlMode == CTL_MODE_SINGLEOPTIM) 
    {
		WORD wTempCycleLen[MAX_RING_COUNT];
		memset(wTempCycleLen, 0, MAX_RING_COUNT);
		for (i = 0;i < m_tFixTimeCtlInfo.m_nRingCount;i ++)
		{
			for (j = 0;j < m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_nPhaseCount;j ++)
			{
				if (m_nSplitTime[i][j] > 0)
                {
                    wTempCycleLen[i] += m_nSplitTime[i][j];
                }
                else
                {
                    wTempCycleLen[i] += m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[j].m_wPhaseTime;
                }
			}

			if (i == 1)
			{
				if (wTempCycleLen[1] > wTempCycleLen[0])
				{
					wCycleLen = wTempCycleLen[1];
				}
				else
				{
					wCycleLen = wTempCycleLen[0];
				}
			}
		}
    }

    tRunStatus.m_nCurCtlMode = tCtlStatus.m_nCurCtlMode;
	if (tRunStatus.m_byPlanID == 0)
	{
		tRunStatus.m_byPlanID = m_byPlanID;
	}
    tRunStatus.m_nCycleRunTime = m_tFixTimeCtlInfo.m_wCycleRunTime;
	tRunStatus.m_nCycleRemainTime = wCycleLen - m_tFixTimeCtlInfo.m_wCycleRunTime;
	tRunStatus.m_nPatternOffset = m_tFixTimeCtlInfo.m_wPhaseOffset;
    if (tRunStatus.m_nCurCtlMode != CTL_MODE_CABLELESS)
    {
        tRunStatus.m_nOffset = 0;
    }
    else
    {
        tRunStatus.m_nOffset = abs(m_nRealOffset);
    }
    tRunStatus.m_nCycleLen = wCycleLen;
    tRunStatus.m_nRingCount = m_tFixTimeCtlInfo.m_nRingCount;

    memcpy(&tRunStatus.m_tRunStageInfo, &m_tRunStageInfo, sizeof(m_tRunStageInfo));

    int nIndex = m_tFixTimeCtlInfo.m_nCurPhaseIndex[i];

	TPhasePassCmdPhaseStatus tPhasePassCmdPhaseStatus;
	memset(&tPhasePassCmdPhaseStatus, 0, sizeof(tPhasePassCmdPhaseStatus));
	m_pOpenATCRunStatus->GetLocalPhasePassStatus(tPhasePassCmdPhaseStatus);

    for (i = 0;i < tRunStatus.m_nRingCount;i ++)
    {
        tRunStatus.m_atRingRunStatus[i].m_nPhaseCount = m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_nPhaseCount;//������λ����
		tRunStatus.m_atRingRunStatus[i].m_nCurRunPhaseIndex = m_tFixTimeCtlInfo.m_nCurPhaseIndex[i];//��ǰ������λ����
        tRunStatus.m_atRingRunStatus[i].m_nCurStageIndex = m_tFixTimeCtlInfo.m_nCurStageIndex;//��ǰ���н׶�����
        
        TPhaseLampClrRunCounter tRunCounter;
        m_pOpenATCRunStatus->GetPhaseLampClrRunCounter(tRunCounter);

        for (j = 0;j < m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_nPhaseCount;j ++)
        {
            tRunStatus.m_atRingRunStatus[i].m_atPhaseStatus[j].m_byPhaseID = 
                m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[j].m_tPhaseParam.m_byPhaseNumber;

			tRunStatus.m_atRingRunStatus[i].m_atPhaseStatus[j].m_chPhaseCloseStatus = tPhasePassCmdPhaseStatus.m_nPhasePassStatus[m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[j].m_tPhaseParam.m_byPhaseNumber - 1];//��λ�ر�״̬

            if (tRunStatus.m_nCurCtlMode == CTL_MODE_ACTUATE || tRunStatus.m_nCurCtlMode == CTL_MODE_CABLELESS || tRunStatus.m_nCurCtlMode == CTL_MODE_PEDCROSTREET ||
				tRunStatus.m_nCurCtlMode == CTL_MODE_SINGLEOPTIM) 
            {
                tRunStatus.m_atRingRunStatus[i].m_atPhaseStatus[j].m_nSplitTime = m_nSplitTime[i][j];
            }
            else
            {
                tRunStatus.m_atRingRunStatus[i].m_atPhaseStatus[j].m_nSplitTime = m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[j].m_wPhaseTime;
            }

            if (tRunStatus.m_atRingRunStatus[i].m_atPhaseStatus[j].m_nSplitTime <= 0)
            {
                tRunStatus.m_atRingRunStatus[i].m_atPhaseStatus[j].m_nSplitTime = m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[j].m_wPhaseTime;
            }

            //m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "GetPhaseRunStatus Ring:%d Phase:%d SplitTime:%d", i, j, tRunStatus.m_atRingRunStatus[i].m_atPhaseStatus[j].m_nSplitTime);         

            memcpy(tRunStatus.m_atRingRunStatus[i].m_atPhaseStatus[j].m_abyPhaseConcurrency,
                    m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[j].m_tPhaseParam.m_byPhaseConcurrency,
                    MAX_PHASE_CONCURRENCY_COUNT);

            //��λ�Ľ�ֹ������
            if (m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[j].m_tPhaseParam.m_byForbiddenFlag || m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[j].m_tPhaseParam.m_byScreenFlag ||
                m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[j].m_chPhaseMode == NEGLECT_MODE || m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[j].m_chPhaseMode == SHIELD_MODE)
            {
                if (m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[j].m_tPhaseParam.m_byForbiddenFlag || m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[j].m_chPhaseMode == NEGLECT_MODE)//��ֹ���ȼ���������
                {
                    tRunStatus.m_atRingRunStatus[i].m_atPhaseStatus[j].m_chPhaseStatus = C_CH_PHASESTAGE_OF;
                    tRunStatus.m_atRingRunStatus[i].m_atPhaseStatus[j].m_chPhaseControlStatus = 2;
                }
                else if (m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[j].m_tPhaseParam.m_byScreenFlag || m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[j].m_chPhaseMode == SHIELD_MODE)
                {
                    tRunStatus.m_atRingRunStatus[i].m_atPhaseStatus[j].m_chPhaseStatus = C_CH_PHASESTAGE_R;
                    tRunStatus.m_atRingRunStatus[i].m_atPhaseStatus[j].m_chPhaseControlStatus = 1;
                }

                tRunStatus.m_atRingRunStatus[i].m_atPhaseStatus[j].m_nCurStageRemainTime = 0;
            }
            else
            {
                tRunStatus.m_atRingRunStatus[i].m_atPhaseStatus[j].m_chPhaseStatus
                    = m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[j].m_chPhaseStage;

                tRunStatus.m_atRingRunStatus[i].m_atPhaseStatus[j].m_chPhaseControlStatus = 0;

                //��������λ������״̬
                if (tRunStatus.m_atRingRunStatus[i].m_atPhaseStatus[j].m_chPhaseStatus == C_CH_PHASESTAGE_F)
                {
                    tRunStatus.m_atRingRunStatus[i].m_atPhaseStatus[j].m_chPhaseStatus = C_CH_PHASESTAGE_R;
                }

                if (tRunStatus.m_atRingRunStatus[i].m_atPhaseStatus[j].m_chPhaseStatus == C_CH_PHASESTAGE_G)
                {
                    tRunStatus.m_atRingRunStatus[i].m_atPhaseStatus[j].m_nCurStageRemainTime = tRunStatus.m_atRingRunStatus[i].m_atPhaseStatus[j].m_nSplitTime -
                        m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[j].m_tPhaseParam.m_byPhaseYellowChange - m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[j].m_tPhaseParam.m_byPhaseRedClear - tRunCounter.m_nLampClrTime[i];
                }
                else
                {
                    tRunStatus.m_atRingRunStatus[i].m_atPhaseStatus[j].m_nCurStageRemainTime = m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[j].m_wPhaseStageRunTime - tRunCounter.m_nLampClrTime[i];
                }

                if (tRunStatus.m_atRingRunStatus[i].m_atPhaseStatus[j].m_nCurStageRemainTime < 0)
                {
                    tRunStatus.m_atRingRunStatus[i].m_atPhaseStatus[j].m_nCurStageRemainTime = 0;
                }
            }

            //������λ������״̬
            tRunStatus.m_atRingRunStatus[i].m_atPhaseStatus[j].m_chPedPhaseStatus = m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[j].m_chPedPhaseStage;

            if (tRunStatus.m_atRingRunStatus[i].m_atPhaseStatus[j].m_chPedPhaseStatus == C_CH_PHASESTAGE_F)
            {
                tRunStatus.m_atRingRunStatus[i].m_atPhaseStatus[j].m_chPedPhaseStatus = C_CH_PHASESTAGE_R;
            }
            
            if (tRunStatus.m_atRingRunStatus[i].m_atPhaseStatus[j].m_chPedPhaseStatus == C_CH_PHASESTAGE_G)
            {
                tRunStatus.m_atRingRunStatus[i].m_atPhaseStatus[j].m_nCurPedRemainTime = tRunStatus.m_atRingRunStatus[i].m_atPhaseStatus[j].m_nSplitTime - 
                m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[j].m_tPhaseParam.m_byPhaseRedClear - tRunCounter.m_nPedLampClrTime[i]; 
            }
            else
            {
                tRunStatus.m_atRingRunStatus[i].m_atPhaseStatus[j].m_nCurPedRemainTime = m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[j].m_wPedPhaseStageRunTime - tRunCounter.m_nPedLampClrTime[i];
            }
            
            if (tRunStatus.m_atRingRunStatus[i].m_atPhaseStatus[j].m_nCurPedRemainTime < 0)
            {
                tRunStatus.m_atRingRunStatus[i].m_atPhaseStatus[j].m_nCurPedRemainTime = 0;
            }
        }
    }

	tRunStatus.m_atOverlapRunStatus.m_nOverlapPhaseCount = m_tFixTimeCtlInfo.m_nOverlapCount;
	for (i = 0;i < tRunStatus.m_atOverlapRunStatus.m_nOverlapPhaseCount;i++)
	{
		tRunStatus.m_atOverlapRunStatus.m_atOverlapPhaseRunStatus[i].m_byOverlapPhaseID = m_tFixTimeCtlInfo.m_atOverlapInfo[i].m_tOverlapParam.m_byOverlapNumber;
		tRunStatus.m_atOverlapRunStatus.m_atOverlapPhaseRunStatus[i].m_byOverlapType = GetOverlapType(m_tFixTimeCtlInfo.m_atOverlapInfo[i].m_chOverlapStage);
		tRunStatus.m_atOverlapRunStatus.m_atOverlapPhaseRunStatus[i].m_byOverlapPedType = GetOverlapType(m_tFixTimeCtlInfo.m_atOverlapInfo[i].m_chPedOverlapStage);
		for (j = 0;j < MAX_PHASE_COUNT_IN_OVERLAP;j++)
		{
			tRunStatus.m_atOverlapRunStatus.m_atOverlapPhaseRunStatus[i].m_nMotherPhase[j] = m_tFixTimeCtlInfo.m_atOverlapInfo[i].m_tOverlapParam.m_byArrOverlapIncludedPhases[j];
			for (int m = 0;m < tRunStatus.m_nRingCount;m++)
			{
			    for (int n = 0;n < m_tFixTimeCtlInfo.m_atPhaseSeq[m].m_nPhaseCount;n++)
				{
				    if (m_tFixTimeCtlInfo.m_nCurPhaseIndex[m] == n && tRunStatus.m_atRingRunStatus[m].m_atPhaseStatus[n].m_byPhaseID == tRunStatus.m_atOverlapRunStatus.m_atOverlapPhaseRunStatus[i].m_nMotherPhase[j])
					{
						tRunStatus.m_atOverlapRunStatus.m_atOverlapPhaseRunStatus[i].m_nSplitTime = tRunStatus.m_atRingRunStatus[m].m_atPhaseStatus[n].m_nSplitTime;
						tRunStatus.m_atOverlapRunStatus.m_atOverlapPhaseRunStatus[i].m_nOverlapRemainTime = tRunStatus.m_atRingRunStatus[m].m_atPhaseStatus[n].m_nCurStageRemainTime;
						tRunStatus.m_atOverlapRunStatus.m_atOverlapPhaseRunStatus[i].m_nOverlapPedRemainTime = tRunStatus.m_atRingRunStatus[m].m_atPhaseStatus[n].m_nCurPedRemainTime;
					}
				}
			}
		}
	}
}

/*==================================================================== 
������ ��ProcPhaseDetStatus
���� ���ж���λ��Ӧ�ļ������״̬
�㷨ʵ�� �� 
����˵�� ��nRingIndex����ǰ���еĻ����� 
           nPhaseIndex����ǰ���е���λ����
����ֵ˵����
        true�����м��������
        false�������δȫ������
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ������          �������� 
====================================================================*/ 
bool CLogicCtlFixedTime::ProcPhaseDetStatus(int nRingIndex,int nPhaseIndex)
{
    PTRingCtlInfo ptRingRunInfo = &(m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex]);

    bool bStatus = true;

    TRealTimeVehDetData tVehDetData;
    m_pOpenATCRunStatus->GetRTVehDetData(tVehDetData);
        
    for (int i = 0;i < m_tFixTimeCtlInfo.m_nVehDetCount;i ++)
    {
        if (m_tFixTimeCtlInfo.m_atVehDetector[i].m_byVehicleDetectorCallPhase == 
            ptRingRunInfo->m_atPhaseInfo[nPhaseIndex].m_tPhaseParam.m_byPhaseNumber)
        {
            if (!tVehDetData.m_bDetFaultStatus[i] && tVehDetData.m_bVehDetExist[i])
            {
                bStatus = false;
                break;
            }
        }
    }

    return bStatus;
}    

/*==================================================================== 
������ ��SetGreenLampPulse
���� �������̵�����
�㷨ʵ�� �� 
����˵�� ��
         int nPhaseType����λ����
         BYTE byPhaseNumber����λID
����ֵ˵����
          ��
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ����Ƽ          �������� 
====================================================================*/ 
void CLogicCtlFixedTime::SetGreenLampPulse(int nPhaseType, BYTE byPhaseNumber, bool bState)
{
    int i = 0;
	size_t j = 0;

    TLampClrStatus tLampClrStatus;
    memset(&tLampClrStatus,0,sizeof(TLampClrStatus)); 
    m_pOpenATCRunStatus->GetLampClrStatus(tLampClrStatus);
    SetLampClr(tLampClrStatus);

    for (i = 0;i < m_nChannelCount;i++)
    {
		if (m_atChannelInfo[i].m_byChannelNumber == 0)
		{
			continue;
		}

        if ((int)m_atChannelInfo[i].m_byChannelControlSource == byPhaseNumber)
        {
            if (m_atChannelInfo[i].m_byChannelControlType == nPhaseType)
            {
                tLampClrStatus.m_bGreenLampPulse[(m_atChannelInfo[i].m_byChannelNumber - 1) * 3 + 2] = bState;
            }
        }
    } 

    m_pOpenATCRunStatus->SetLampClrStatus(tLampClrStatus);

}

/*==================================================================== 
������ ��SetRedLampPulse
���� �����ú������
�㷨ʵ�� �� 
����˵�� ��int nPhaseType����λ����
           int byNextPhaseNumber����һ����λID
����ֵ˵����
          ��
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ����Ƽ          �������� 
====================================================================*/ 
void CLogicCtlFixedTime::SetRedLampPulse(int nPhaseType, BYTE byNextPhaseNumber, bool bState)
{
    int i = 0, k = 0;
	size_t j = 0;
    TLampClrStatus tLampClrStatus;
    memset(&tLampClrStatus,0,sizeof(TLampClrStatus)); 
    m_pOpenATCRunStatus->GetLampClrStatus(tLampClrStatus);
    SetLampClr(tLampClrStatus);
    
    for (i = 0;i < m_nChannelCount;i++)
    {
		if (m_atChannelInfo[i].m_byChannelNumber == 0)
		{
			continue;
		}

        if ((int)m_atChannelInfo[i].m_byChannelControlSource == byNextPhaseNumber)
        {
            if (m_atChannelInfo[i].m_byChannelControlType == nPhaseType)
            {
                tLampClrStatus.m_bRedLampPulse[(m_atChannelInfo[i].m_byChannelNumber - 1) * 3] = bState;
            }
        }
    } 

    m_pOpenATCRunStatus->SetLampClrStatus(tLampClrStatus);
}

/*==================================================================== 
������ ��GetGreenFalshCount
���� ����ȡÿ��������Ӷ�Ӧ����������
�㷨ʵ�� �� 
����˵�� ��int nPhaseType����λ����
           pPhaseCtlInfo����λ����������״̬��Ϣ
           nSecond������ʱ��(��)
����ֵ˵����
          ��
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ����Ƽ          �������� 
====================================================================*/ 
void CLogicCtlFixedTime::GetGreenFalshCount(int nPhaseType, TPhaseCtlInfo * pPhaseCtlInfo, int nSecond)
{
    int nRingCount = 0;
     
    for (int i = 0;i < m_nChannelCount;i++)
    {
		if (m_atChannelInfo[i].m_byChannelNumber == 0)
		{
			continue;
		}

        if ((int)m_atChannelInfo[i].m_byChannelControlSource == pPhaseCtlInfo->m_tPhaseParam.m_byPhaseNumber)
        {
            if (m_atChannelInfo[i].m_byChannelControlType == nPhaseType)
            {
                m_pOpenATCRunStatus->SetGreenFlashCount((m_atChannelInfo[i].m_byChannelNumber - 1) * 3 + 2, nSecond * 2);
            }
        } 
    }
}

/*==================================================================== 
������ ��GetPhaseCycleRemainTime
���� ����ȡ��ǰ��λʣ�������ʱ��
�㷨ʵ�� �� 
����˵�� ��nRingIndex����ǰ���еĻ����� 
           pPhaseCtlInfo����λ����������״̬��Ϣ 
           tRunCounter��λ��ɫ���м�����
����ֵ˵����
          ��
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ����Ƽ          �������� 
====================================================================*/
int  CLogicCtlFixedTime::GetPhaseCycleRemainTime(int nRingIndex, TPhaseCtlInfo * pPhaseCtlInfo, TPhaseLampClrRunCounter tRunCounter)
{
    int nRemainTime = 0;
    if (pPhaseCtlInfo->m_chPhaseStage == C_CH_PHASESTAGE_G)
    { 
        nRemainTime = pPhaseCtlInfo->m_wPhaseTime - tRunCounter.m_nLampClrTime[nRingIndex];
    }
    else if (pPhaseCtlInfo->m_chPhaseStage == C_CH_PHASESTAGE_GF)
    { 
        nRemainTime = pPhaseCtlInfo->m_wPhaseTime - pPhaseCtlInfo->m_wPhaseGreenTime - tRunCounter.m_nLampClrTime[nRingIndex];
    }
    else if (pPhaseCtlInfo->m_chPhaseStage == C_CH_PHASESTAGE_Y)
    { 
        nRemainTime = pPhaseCtlInfo->m_wPhaseTime - pPhaseCtlInfo->m_wPhaseGreenTime - pPhaseCtlInfo->m_tPhaseParam.m_byGreenFlash - tRunCounter.m_nLampClrTime[nRingIndex];
    }
    else if (pPhaseCtlInfo->m_chPhaseStage == C_CH_PHASESTAGE_R)
    { 
        nRemainTime = pPhaseCtlInfo->m_wPhaseTime - pPhaseCtlInfo->m_wPhaseGreenTime - pPhaseCtlInfo->m_tPhaseParam.m_byGreenFlash - pPhaseCtlInfo->m_tPhaseParam.m_byPhaseYellowChange - tRunCounter.m_nLampClrTime[nRingIndex];
    }

    return nRemainTime;
}

/*==================================================================== 
������ ��GetPedPhaseRemainTime
���� ����ȡ��ǰ������λʣ�������ʱ��
�㷨ʵ�� �� 
����˵�� ��nRingIndex����ǰ���еĻ����� 
           pPhaseCtlInfo����λ����������״̬��Ϣ 
           tRunCounter��λ��ɫ���м�����
����ֵ˵����
          ��
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ����Ƽ          �������� 
====================================================================*/
int  CLogicCtlFixedTime::GetPedPhaseCycleRemainTime(int nRingIndex, TPhaseCtlInfo * pPhaseCtlInfo, TPhaseLampClrRunCounter tRunCounter)
{
    int nRemainTime = 0;
    if (pPhaseCtlInfo->m_chPedPhaseStage == C_CH_PHASESTAGE_G)
    { 
        nRemainTime = pPhaseCtlInfo->m_wPhaseTime - tRunCounter.m_nPedLampClrTime[nRingIndex];
    }
    else if (pPhaseCtlInfo->m_chPedPhaseStage == C_CH_PHASESTAGE_GF)
    { 
        nRemainTime = pPhaseCtlInfo->m_wPhaseTime - pPhaseCtlInfo->m_wPedPhaseGreenTime - tRunCounter.m_nPedLampClrTime[nRingIndex];
    }
    else if (pPhaseCtlInfo->m_chPedPhaseStage == C_CH_PHASESTAGE_R)
    { 
        nRemainTime = pPhaseCtlInfo->m_wPhaseTime - pPhaseCtlInfo->m_wPedPhaseGreenTime - pPhaseCtlInfo->m_tPhaseParam.m_byPhasePedestrianClear - tRunCounter.m_nPedLampClrTime[nRingIndex];
    }

    return nRemainTime;
}

/*==================================================================== 
������ ��GetRunStageTable
���� ����ȡ��λ���н׶α�
�㷨ʵ�� �� 
����˵�� ����
����ֵ˵����
          ��
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ����Ƽ          �������� 
====================================================================*/
void CLogicCtlFixedTime::GetRunStageTable()
{
    int nStageTime[MAX_RING_COUNT][MAX_PHASE_COUNT] = {0};//������λ�ڵ�  
	int nAllStageTime[MAX_STAGE_COUNT] = {0};//�׶νڵ��

	int i = 0;
    int j = 0;
    int m = 0;
    int n = 0;
    int nAllStageCount = 0;

    //���ɽ׶νڵ��
    for (i = 0;i < m_tFixTimeCtlInfo.m_nRingCount;i++)
    {
        for (j = 0;j < m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_nPhaseCount;j++)
        {
            if (j == 0)
			{
                nStageTime[i][0] = m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[j].m_wPhaseTime;
			}
			else
			{
				nStageTime[i][j] = m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[j].m_wPhaseTime + nStageTime[i][j - 1];
			}

            if (nStageTime[i][j] > 0)
			{
				nAllStageTime[nAllStageCount++] = nStageTime[i][j];
			}
        }
    }

	std::sort(nAllStageTime, nAllStageTime + nAllStageCount);//�׶νڵ������

    //�׶νڵ��ȥ���ظ��Ľڵ�
    int nStageCount = 0;
    for (i = 0;i < nAllStageCount;i++)
    {
        if (nAllStageTime[i + 1] == nAllStageTime[i])
        {
            continue;
        }
        else
        {
            nAllStageTime[nStageCount + 1] = nAllStageTime[i + 1];
            nStageCount++;
        }
    }

    m_tRunStageInfo.m_nRunStageCount = nStageCount;
    
	//���ɽ׶���λ��
	int nPhaseIndex = 0;
	for (i = 0; i < m_tFixTimeCtlInfo.m_nRingCount; i++)
	{
		for (j = 0; j < m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_nPhaseCount; j++)
		{
			int nMaxStage = 0;
			int nMinStage = 0;
			int nStageSum = 0;
			
			for (m = 0; m < nStageCount; m++)//������λ������ʱ���ڽ׶���λ���ж�Ӧ�Ľ׶α�ŵ����ֵ
			{
				if (nStageTime[i][j] < nAllStageTime[m] || nStageTime[i][j] == nAllStageTime[m])
				{
					nStageSum = m + 1;
					break;
				}
			}

			nMaxStage = nStageSum;

			if (j == 0)
			{
				nMinStage = 1;
			}
			else
			{ 
				for (m = 0; m < nStageCount; m++)//������λ������ʱ���ڽ׶���λ���ж�Ӧ�Ľ׶α�ŵ���Сֵ
				{
					if (nStageTime[i][j - 1] < nAllStageTime[m] || nStageTime[i][j] == nAllStageTime[m])
					{
						nStageSum = m + 1;
						break;
					}
				}

				nMinStage = nStageSum;
			}
			
			for (m = nMinStage - 1;m < nMaxStage;m++)       //��λ�ڽ׶���λ���ж�Ӧ����Ľ׶θ�ֵ
			{
				if (m_tRunStageInfo.m_PhaseRunstageInfo[m].m_nConcurrencyPhase[i] == 0)
                {
                    m_tRunStageInfo.m_PhaseRunstageInfo[m].m_nStageIndex = m;
                    if (m == 0)
                    {
                        m_tRunStageInfo.m_PhaseRunstageInfo[m].m_nStageStartTime = 0;  
                    }
                    else
                    {
                        m_tRunStageInfo.m_PhaseRunstageInfo[m].m_nStageStartTime = nAllStageTime[m - 1];
                    }
                    m_tRunStageInfo.m_PhaseRunstageInfo[m].m_nStageEndTime = nAllStageTime[m];
        
                    m_tRunStageInfo.m_PhaseRunstageInfo[m].m_nConcurrencyPhase[i] = m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[j].m_tPhaseParam.m_byPhaseNumber;
                    m_tRunStageInfo.m_PhaseRunstageInfo[m].m_nConcurrencyPhaseCount += 1;
                }
			}
		}
	}

	//TRunStageInfo   tTempRunStageInfo;
 //   memset(&tTempRunStageInfo,0,sizeof(tTempRunStageInfo));

	//int nConcurrencyPhaseCount[MAX_STAGE_COUNT];
	//memset(nConcurrencyPhaseCount,0,sizeof(nConcurrencyPhaseCount));

	//for (i = 0;i < m_tRunStageInfo.m_nRunStageCount;i++)      
	//{
	//	nConcurrencyPhaseCount[i] = m_tRunStageInfo.m_PhaseRunstageInfo[i].m_nConcurrencyPhaseCount;

	//	for (j = 0; j < m_tRunStageInfo.m_PhaseRunstageInfo[i].m_nConcurrencyPhaseCount;j++)
	//	{
	//		for (m = 0; m < MAX_PHASE_COUNT; m++)
	//		{
	//			if (m_tRunStageInfo.m_PhaseRunstageInfo[i].m_nConcurrencyPhase[j] == m_atSplitInfo[m].m_bySplitPhase && m_atSplitInfo[m].m_bySplitMode == NEGLECT_MODE)
	//			{
	//				nConcurrencyPhaseCount[i] -= 1;
	//			}
	//		}
	//	}

	//	if (nConcurrencyPhaseCount[i] != 0)
	//	{
	//		memcpy(&tTempRunStageInfo.m_PhaseRunstageInfo[tTempRunStageInfo.m_nRunStageCount], &m_tRunStageInfo.m_PhaseRunstageInfo[i], sizeof(m_tRunStageInfo.m_PhaseRunstageInfo[i]));
	//		tTempRunStageInfo.m_PhaseRunstageInfo[tTempRunStageInfo.m_nRunStageCount].m_nStageIndex = tTempRunStageInfo.m_nRunStageCount;
	//		tTempRunStageInfo.m_nRunStageCount += 1;
	//	}
	//}

	//if (tTempRunStageInfo.m_nRunStageCount > 0 && memcmp(&m_tRunStageInfo, &tTempRunStageInfo, sizeof(m_tRunStageInfo) != 0))
	//{
	//	memcpy(&m_tRunStageInfo, &tTempRunStageInfo, sizeof(m_tRunStageInfo));
	//}

    /*************************************************************************/
    //ͨ�����ɵĽ׶α�����������
    TRunStageInfo   tTempRunStageInfo;
    memset(&tTempRunStageInfo, 0, sizeof(tTempRunStageInfo));

    int nConcurrencyPhaseCount[MAX_STAGE_COUNT];
    memset(nConcurrencyPhaseCount, 0, sizeof(nConcurrencyPhaseCount));

    int nConcurrencyPhase[MAX_RING_COUNT][MAX_STAGE_COUNT];
    memset(nConcurrencyPhase, 0, sizeof(nConcurrencyPhase));

    int nNeglectStageCount = 0;
    int nNeglectStage[MAX_STAGE_COUNT];
    memset(nNeglectStage, 0, sizeof(nNeglectStage));

    bool bOverStage[MAX_RING_COUNT];

    for (i = 0; i < m_tRunStageInfo.m_nRunStageCount; i++)
    {
        nConcurrencyPhaseCount[i] = m_tRunStageInfo.m_PhaseRunstageInfo[i].m_nConcurrencyPhaseCount;

        for (j = 0; j < m_tRunStageInfo.m_PhaseRunstageInfo[i].m_nConcurrencyPhaseCount; j++)
        {
            for (m = 0; m < MAX_PHASE_COUNT; m++)
            {
                if (m_tRunStageInfo.m_PhaseRunstageInfo[i].m_nConcurrencyPhase[j] == m_atSplitInfo[m].m_bySplitPhase && (m_atSplitInfo[m].m_bySplitMode == NEGLECT_MODE || m_atSplitInfo[m].m_bySplitMode == SHIELD_MODE))
                {
                    nConcurrencyPhase[j][i] = m_tRunStageInfo.m_PhaseRunstageInfo[i].m_nConcurrencyPhase[j];
                    nConcurrencyPhaseCount[i] -= 1;
                }
            }
        }

        if (nConcurrencyPhaseCount[i] != 0)
        {
            memcpy(&tTempRunStageInfo.m_PhaseRunstageInfo[tTempRunStageInfo.m_nRunStageCount], &m_tRunStageInfo.m_PhaseRunstageInfo[i], sizeof(m_tRunStageInfo.m_PhaseRunstageInfo[i]));
            tTempRunStageInfo.m_PhaseRunstageInfo[tTempRunStageInfo.m_nRunStageCount].m_nStageIndex = tTempRunStageInfo.m_nRunStageCount;
            tTempRunStageInfo.m_nRunStageCount += 1;
        }
        else
        {
            //�׶�Ϊ���Խ׶�
            nNeglectStage[nNeglectStageCount++] = i + 1;
        }
    }

    //���ݽ׶α�������������
    //�����Ƿ��п�׶�
    //����nNeglectStage��λ�����ԵĽ׶Σ��ٶ�Ӧ��ǰ�������׶��Ƿ�Ϊ��ͬ����λ������ǣ����������򣬲���ȥ��Ӧ�׶�ʱ��
    //ǰ�������׶����û����ͬ����λ����ֱ���������к��Ը���λ
    for (int iNeglectStageIndex = 0; iNeglectStageIndex < nNeglectStageCount; iNeglectStageIndex++)
    {
        m_tFixTimeCtlInfo.m_wCycleLen = m_tFixTimeCtlInfo.m_wCycleLen - (m_tRunStageInfo.m_PhaseRunstageInfo[nNeglectStage[iNeglectStageIndex] - 1].m_nStageEndTime - m_tRunStageInfo.m_PhaseRunstageInfo[nNeglectStage[iNeglectStageIndex] - 1].m_nStageStartTime);
        memset(bOverStage, 0, sizeof(bOverStage));
        for (int jRingIndex = 0; jRingIndex < m_tRunStageInfo.m_PhaseRunstageInfo[nNeglectStage[iNeglectStageIndex] - 1].m_nConcurrencyPhaseCount; jRingIndex++)
        {
            if (nNeglectStage[iNeglectStageIndex] == 1)
            {
                if (nConcurrencyPhase[jRingIndex][0] != nConcurrencyPhase[jRingIndex][1])
                {
                    bOverStage[jRingIndex] = true;              //�ý׶ζ�ӦjRingIndex������λֱ�Ӻ��Ե�
                }
            }
            else if (nNeglectStage[iNeglectStageIndex] == m_tRunStageInfo.m_nRunStageCount)
            {
                //���Խ׶�Ϊ���һ���׶Σ�ֻ��Ҫ�жϺ�ǰһ�׶��Ƿ��׶μ���
                if (nConcurrencyPhase[jRingIndex][nNeglectStage[iNeglectStageIndex] - 2] != nConcurrencyPhase[jRingIndex][nNeglectStage[iNeglectStageIndex] - 1])
                {
                    bOverStage[jRingIndex] = true;              //�ý׶ζ�ӦjRingIndex������λֱ�Ӻ��Ե�
                }
            }
            else
            {
                if ((nConcurrencyPhase[jRingIndex][nNeglectStage[iNeglectStageIndex] - 2] != nConcurrencyPhase[jRingIndex][nNeglectStage[iNeglectStageIndex] - 1]) &&
                    (nConcurrencyPhase[jRingIndex][nNeglectStage[iNeglectStageIndex] - 1] != nConcurrencyPhase[jRingIndex][nNeglectStage[iNeglectStageIndex]]))
                {
                    bOverStage[jRingIndex] = true;              //�ý׶ζ�ӦjRingIndex������λֱ�Ӻ��Ե�
                }
            }
        }
        for (int iRingIndex = 0; iRingIndex < m_tFixTimeCtlInfo.m_nRingCount; iRingIndex++)
        {
            for (int iPhaseIndex = 0; iPhaseIndex < m_tFixTimeCtlInfo.m_atPhaseSeq[iRingIndex].m_nPhaseCount; iPhaseIndex++)
            {
                if ((m_tFixTimeCtlInfo.m_atPhaseSeq[iRingIndex].m_atPhaseInfo[iPhaseIndex].m_tPhaseParam.m_byPhaseNumber == nConcurrencyPhase[iRingIndex][nNeglectStage[iNeglectStageIndex] - 1]) && bOverStage[iRingIndex])
                {
                    m_bySplitPhaseMode[iRingIndex][iPhaseIndex] = 1;
                }
                else if (m_tFixTimeCtlInfo.m_atPhaseSeq[iRingIndex].m_atPhaseInfo[iPhaseIndex].m_tPhaseParam.m_byPhaseNumber == nConcurrencyPhase[iRingIndex][nNeglectStage[iNeglectStageIndex] - 1])
                {
                    m_bySplitPhaseMode[iRingIndex][iPhaseIndex] = 2;
                    m_nSplitPhaseTime[iRingIndex][iPhaseIndex] = m_tRunStageInfo.m_PhaseRunstageInfo[nNeglectStage[iNeglectStageIndex] - 1].m_nStageEndTime - m_tRunStageInfo.m_PhaseRunstageInfo[nNeglectStage[iNeglectStageIndex] - 1].m_nStageStartTime;
                }
            }
        }
    }

    if (tTempRunStageInfo.m_nRunStageCount > 0 && memcmp(&m_tRunStageInfo, &tTempRunStageInfo, sizeof(m_tRunStageInfo) != 0))
    {
        memcpy(&m_tRunStageInfo, &tTempRunStageInfo, sizeof(m_tRunStageInfo));
    }
    /*************************************************************************/

	int nNextIndex = 0;
	int nPhaseID = 0;
	int nPhaseLightTransTime = 0;
	int nPedPhaseLightTransTime = 0;
	int nCurStageTime = 0;
	int nLastStageTime = 0;

	TPhase atPhaseTable[MAX_PHASE_COUNT];
    m_pOpenATCParameter->GetPhaseTable(atPhaseTable);

	for (i = 0;i < m_tRunStageInfo.m_nRunStageCount;i++)
	{
		for (j = 0;j < m_tRunStageInfo.m_PhaseRunstageInfo[i].m_nConcurrencyPhaseCount;j++)
		{
			nNextIndex = i + 1;
			if (nNextIndex == m_tRunStageInfo.m_nRunStageCount)
			{
				nNextIndex = 0;
			}

			if (m_tRunStageInfo.m_PhaseRunstageInfo[i].m_nConcurrencyPhase[j] != m_tRunStageInfo.m_PhaseRunstageInfo[nNextIndex].m_nConcurrencyPhase[j])
			{
				m_tRunStageInfo.m_PhaseRunstageInfo[i].m_bDirectTransit[j] = true;
				nPhaseLightTransTime = 0;
				nPedPhaseLightTransTime = 0;

				if (i > 0)
				{
					if (!m_tRunStageInfo.m_PhaseRunstageInfo[i-1].m_bDirectTransit[j])
					{
						for (m = 0; m < MAX_PHASE_COUNT; m++)
						{
							if (atPhaseTable[m].m_byPhaseNumber == m_tRunStageInfo.m_PhaseRunstageInfo[i].m_nConcurrencyPhase[j])
							{
								nLastStageTime			= m_tRunStageInfo.m_PhaseRunstageInfo[i - 1].m_nStageEndTime - m_tRunStageInfo.m_PhaseRunstageInfo[i - 1].m_nStageStartTime;
								nCurStageTime			= m_tRunStageInfo.m_PhaseRunstageInfo[i].m_nStageEndTime - m_tRunStageInfo.m_PhaseRunstageInfo[i].m_nStageStartTime;
								nPhaseLightTransTime	= atPhaseTable[m].m_byGreenFlash + atPhaseTable[m].m_byPhaseYellowChange + atPhaseTable[m].m_byPhaseRedClear;
								nPedPhaseLightTransTime = atPhaseTable[m].m_byPhasePedestrianClear + atPhaseTable[m].m_byPhaseRedClear;
								break;
							}
						}

						if (nCurStageTime >= nPhaseLightTransTime)
						{
							m_tRunStageInfo.m_PhaseRunstageInfo[i].m_bPhaseStartTransitInCurStage[j] = true;
							m_tRunStageInfo.m_PhaseRunstageInfo[i].m_nPhaseCurStageTransitTime[j]	 = nPhaseLightTransTime;
						}
						else
						{
							if (nCurStageTime + nLastStageTime >= nPhaseLightTransTime)
							{
								m_tRunStageInfo.m_PhaseRunstageInfo[i - 1].m_bPhaseStartTransitInCurStage[j] = true;
								m_tRunStageInfo.m_PhaseRunstageInfo[i - 1].m_nPhaseCurStageTransitTime[j]	 = nPhaseLightTransTime - nCurStageTime;

								m_tRunStageInfo.m_PhaseRunstageInfo[i].m_bPhaseStartTransitInCurStage[j]	 = false;
								m_tRunStageInfo.m_PhaseRunstageInfo[i].m_nPhaseCurStageTransitTime[j]		 = nCurStageTime;
							}
						}

						if (nCurStageTime >= nPedPhaseLightTransTime)
						{
							m_tRunStageInfo.m_PhaseRunstageInfo[i].m_bPedPhaseStartTransitInCurStage[j] = true;
							m_tRunStageInfo.m_PhaseRunstageInfo[i].m_nPedPhaseCurStageTransitTime[j]	= nPedPhaseLightTransTime;
						}
						else
						{
							if (nCurStageTime + nLastStageTime >= nPedPhaseLightTransTime)
							{
								m_tRunStageInfo.m_PhaseRunstageInfo[i - 1].m_bPedPhaseStartTransitInCurStage[j] = true;
								m_tRunStageInfo.m_PhaseRunstageInfo[i - 1].m_nPedPhaseCurStageTransitTime[j]	= nPedPhaseLightTransTime - nCurStageTime;

								m_tRunStageInfo.m_PhaseRunstageInfo[i].m_bPedPhaseStartTransitInCurStage[j]		= false;
								m_tRunStageInfo.m_PhaseRunstageInfo[i].m_nPedPhaseCurStageTransitTime[j]		= nCurStageTime;
							}
						}
					}
					else
					{
						m_tRunStageInfo.m_PhaseRunstageInfo[i].m_bPhaseStartTransitInCurStage[j]	= true;
						m_tRunStageInfo.m_PhaseRunstageInfo[i].m_bPedPhaseStartTransitInCurStage[j] = true;
						m_tRunStageInfo.m_PhaseRunstageInfo[i].m_nPhaseCurStageTransitTime[j]		= nPhaseLightTransTime;
						m_tRunStageInfo.m_PhaseRunstageInfo[i].m_nPedPhaseCurStageTransitTime[j]	= nPedPhaseLightTransTime;
					}
				}
				else
				{
					m_tRunStageInfo.m_PhaseRunstageInfo[i].m_bPhaseStartTransitInCurStage[j]	= true;
					m_tRunStageInfo.m_PhaseRunstageInfo[i].m_bPedPhaseStartTransitInCurStage[j] = true;
					m_tRunStageInfo.m_PhaseRunstageInfo[i].m_nPhaseCurStageTransitTime[j]		= nPhaseLightTransTime;
					m_tRunStageInfo.m_PhaseRunstageInfo[i].m_nPedPhaseCurStageTransitTime[j]	= nPedPhaseLightTransTime;
				}
			}
		}
	}
}

/*==================================================================== 
������ ��StageChg
���� ���жϵ�ǰ����Ӧ�Ľ׶��Ƿ��б仯
�㷨ʵ�� �� 
����˵�� ��nRingIndex�������� 
����ֵ˵�����Ƿ��е���һ���׶�
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ����Ƽ          �������� 
====================================================================*/ 
bool CLogicCtlFixedTime::StageChg(int nRingIndex)
{
    int  i = 0, j = 0;
    int  nPhaseNumber[MAX_RING_COUNT] = {0};
    bool bStageChg = false;
    int  nCount = 0;

    int  nStageIndex[MAX_RING_COUNT] = {0};
    bool bFindStage = false;
   
    //�ҵ�ÿ�����ĵ�ǰ��λ
    for (i = 0; i < m_tFixTimeCtlInfo.m_nRingCount; i++)
    {
        nPhaseNumber[i] = m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[m_tFixTimeCtlInfo.m_nCurPhaseIndex[i]].m_tPhaseParam.m_byPhaseNumber;
    }
   
    for (i = 0;i < m_tRunStageInfo.m_nRunStageCount;i++)      
	{
        nCount = 0;

		for (j = 0; j < m_tRunStageInfo.m_PhaseRunstageInfo[i].m_nConcurrencyPhaseCount;j++)
		{
            if (m_tRunStageInfo.m_PhaseRunstageInfo[i].m_nConcurrencyPhase[j] == nPhaseNumber[j])//��ǰ�׶ζ�Ӧ����λ
            {
                nStageIndex[j] = m_tRunStageInfo.m_PhaseRunstageInfo[i].m_nStageIndex;//ÿ������ǰ��λ��Ӧ�Ľ׶�

                nCount += 1;//��ǰ�׶ζ�Ӧ����λ����
                if (nCount == m_tRunStageInfo.m_PhaseRunstageInfo[i].m_nConcurrencyPhaseCount)
                {
                    if (m_tFixTimeCtlInfo.m_nCurStageIndex != m_tRunStageInfo.m_PhaseRunstageInfo[i].m_nStageIndex)//�ҵ���ǰ�������еĽ׶κ�
                    {
                        m_tFixTimeCtlInfo.m_nCurStageIndex = m_tRunStageInfo.m_PhaseRunstageInfo[i].m_nStageIndex;  
                        m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "StageChg RingIndex:%d, CurPhaseIndex:%d CurStageIndex:%d", nRingIndex, m_tFixTimeCtlInfo.m_nCurPhaseIndex[nRingIndex], m_tFixTimeCtlInfo.m_nCurStageIndex);  
                        bStageChg = true;
                    }

                    bFindStage = true;
                    break;
                }
            }
		}

        if (bFindStage)
        {
            break;
        }
	}

    if (!bFindStage)
    {
        std::sort(nStageIndex, nStageIndex + m_tFixTimeCtlInfo.m_nRingCount);//�ѵ�ǰ��λ��Ӧ�Ľ׶�����
        int nMaxStageIndex = nStageIndex[m_tFixTimeCtlInfo.m_nRingCount - 1];
        m_tFixTimeCtlInfo.m_nCurStageIndex = m_tRunStageInfo.m_PhaseRunstageInfo[nMaxStageIndex].m_nStageIndex; 
        
        //m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "StageChg FindMaxStageIndex RingIndex:%d, CurPhaseIndex:%d CurStageIndex:%d", nRingIndex, m_tFixTimeCtlInfo.m_nCurPhaseIndex[nRingIndex], m_tFixTimeCtlInfo.m_nCurStageIndex[nRingIndex]);  
    }

    return bStageChg;
}

/*==================================================================== 
������ ��GetLastPhaseBeforeBarrier
���� ���ҵ�ÿ����������ǰ�����һ����λ
�㷨ʵ�� �� 
����˵�� ����
����ֵ˵������
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ����Ƽ          �������� 
====================================================================*/ 
void CLogicCtlFixedTime::GetLastPhaseBeforeBarrier()
{
    //int  i = 0, j = 0;
    //int  nRingIndex = 0;
    //int  nIndex = 0;

    //int  nDstRingIndex = 0;
    //int  nDstPhaseIndex = 0;

    //bool bIsLastPhase[MAX_RING_COUNT] = {false, false, false, false};

    //for (nRingIndex = 0;nRingIndex < m_tFixTimeCtlInfo.m_nRingCount;nRingIndex++)
    //{
    //    PTRingCtlInfo ptRingRunInfo = &(m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex]);

    //    for (nIndex = 0;nIndex < ptRingRunInfo->m_nPhaseCount;nIndex++)
    //    {
    //        bIsLastPhase[nRingIndex] = true;

    //        //�Ƿ����һ����λ
    //        if (nIndex < ptRingRunInfo->m_nPhaseCount - 1)
    //        {
    //            for (i = 0;i < MAX_PHASE_CONCURRENCY_COUNT;i ++)
    //            {
    //                BYTE byPhaseNum = ptRingRunInfo->m_atPhaseInfo[nIndex + 1].m_tPhaseParam.m_byPhaseConcurrency[i];
    //                if (byPhaseNum == 0)
    //                {
    //                    break;
    //                }

    //                nDstRingIndex = 0;
    //                nDstPhaseIndex = 0;
 
    //                if (GetPhaseIndexByPhaseNumber(nDstRingIndex, nDstPhaseIndex, (int)byPhaseNum))
    //                {
    //                    if (nRingIndex != nDstRingIndex) 
    //                    { 
    //                        //������λ������Ŀ�껷�ĵ�ǰ��λ
    //                        for (j = 0;j < MAX_PHASE_CONCURRENCY_COUNT;j ++)
    //                        {
    //                            if (ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhaseConcurrency[j] == byPhaseNum)
    //                            {
    //                                bIsLastPhase[nRingIndex] = false;
    //                                //m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "Not IsLastPhaseBeforeBarrier RingIndex:%d PhaseIndex:%d DstRingIndex:%d DstPhaseIndex:%d", nRingIndex, nIndex, nDstRingIndex, nDstPhaseIndex);
    //                            } 
    //                        }

    //                        for (j = 0;j < MAX_PHASE_CONCURRENCY_COUNT;j ++)
    //                        {
    //                            if (ptRingRunInfo->m_atPhaseInfo[nIndex + 1].m_tPhaseParam.m_byPhaseConcurrency[j] == m_tFixTimeCtlInfo.m_atPhaseSeq[nDstRingIndex].m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhaseNumber)
    //                            {
    //                                bIsLastPhase[nRingIndex] = false;
    //                                //m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "Not IsLastPhaseBeforeBarrier RingIndex:%d PhaseIndex:%d DstRingIndex:%d DstPhaseIndex:%d", nRingIndex, nIndex, nDstRingIndex, nDstPhaseIndex);
    //                            }  
    //                        }

    //                        for (j = 0;j < MAX_PHASE_CONCURRENCY_COUNT;j ++)
    //                        {
    //                            if (m_tFixTimeCtlInfo.m_atPhaseSeq[nDstRingIndex].m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhaseConcurrency[j] == ptRingRunInfo->m_atPhaseInfo[nIndex + 1].m_tPhaseParam.m_byPhaseNumber)
    //                            {
    //                                bIsLastPhase[nRingIndex] = false;
    //                                //m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "Not IsLastPhaseBeforeBarrier RingIndex:%d PhaseIndex:%d DstRingIndex:%d DstPhaseIndex:%d", nRingIndex, nIndex, nDstRingIndex, nDstPhaseIndex);
    //                            }  
    //                        }

    //                        for (j = 0;j < MAX_PHASE_CONCURRENCY_COUNT;j ++)
    //                        {
    //                            if (m_tFixTimeCtlInfo.m_atPhaseSeq[nDstRingIndex].m_atPhaseInfo[nDstPhaseIndex].m_tPhaseParam.m_byPhaseConcurrency[j] == ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhaseNumber)
    //                            {
    //                                bIsLastPhase[nRingIndex] = false;
    //                                //m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "Not IsLastPhaseBeforeBarrier RingIndex:%d PhaseIndex:%d DstRingIndex:%d DstPhaseIndex:%d", nRingIndex, nIndex, nDstRingIndex, nDstPhaseIndex);
    //                            }  
    //                        }
    //                    }
    //                }
    //            }
    //        }

    //        if (bIsLastPhase[nRingIndex])
    //        {
    //            m_bLastPhaseBeforeBarrier[nRingIndex][nIndex] = true;
    //            m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "IsLastPhaseBeforeBarrier RingIndex:%d PhaseIndex:%d", nRingIndex, nIndex);
    //        }
    //    } 
    //}

    int iStageIndex = 0;
    int iRingIndex = 0;
    int jRingIndex = 0;
    int iPhaseIndex = 0;
    int iPhaseNum = 0;
    int jPhaseNum = 0;
    bool isSame = false;
    bool isConcurrency = false;

    InitPhaseConcurrencyTable();

    for (iStageIndex = 0; iStageIndex < m_tRunStageInfo.m_nRunStageCount; iStageIndex++)
    {
        isSame = false;
        for (iRingIndex = 0; iRingIndex < m_tRunStageInfo.m_PhaseRunstageInfo[iStageIndex].m_nConcurrencyPhaseCount; iRingIndex++)
        {
            if (iStageIndex == m_tRunStageInfo.m_nRunStageCount - 1)
            {
                //���Ϊ���һ���׶Σ���׶ζ�Ӧ����λΪ����ǰ�����һ����λ
                for (iPhaseIndex = 0; iPhaseIndex < m_tFixTimeCtlInfo.m_atPhaseSeq[iRingIndex].m_nPhaseCount; iPhaseIndex++)
                {
                    if (m_tFixTimeCtlInfo.m_atPhaseSeq[iRingIndex].m_atPhaseInfo[iPhaseIndex].m_tPhaseParam.m_byPhaseNumber == m_tRunStageInfo.m_PhaseRunstageInfo[iStageIndex].m_nConcurrencyPhase[iRingIndex])
                    {
                        m_bLastPhaseBeforeBarrier[iRingIndex][iPhaseIndex] = true;
                        m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "IsLastPhaseBeforeBarrier RingIndex:%d PhaseIndex:%d", iRingIndex, iPhaseIndex);
                    }
                }
            }
            else
            {
                //���жϵ�ǰ�׶κͺ�һ���׶ζ�Ӧ����λ�Ƿ���ͬ����׶εĻ�һ����������
                if (m_tRunStageInfo.m_PhaseRunstageInfo[iStageIndex].m_nConcurrencyPhase[iRingIndex] == m_tRunStageInfo.m_PhaseRunstageInfo[iStageIndex + 1].m_nConcurrencyPhase[iRingIndex])
                {
                    isSame = true;
                    break;
                }
            }
        }

        //�����ǰ�׶β������һ���׶Σ����ҵ�ǰ�׶ΰ�����λ���¸��׶����ص���λ���ж��¸��׶ζ�Ӧ����λ�͵�ǰ�׶ε���λ�Ƿ��в���
        if (iStageIndex != m_tRunStageInfo.m_nRunStageCount - 1 && !isSame)
        {
            isConcurrency = false;
            for (iRingIndex = 0; iRingIndex < m_tRunStageInfo.m_PhaseRunstageInfo[iStageIndex].m_nConcurrencyPhaseCount; iRingIndex++)
            {
                for (jRingIndex = 0; jRingIndex < m_tRunStageInfo.m_PhaseRunstageInfo[iStageIndex + 1].m_nConcurrencyPhaseCount; jRingIndex++)
                {
                    iPhaseNum = m_tRunStageInfo.m_PhaseRunstageInfo[iStageIndex].m_nConcurrencyPhase[iRingIndex];
                    jPhaseNum = m_tRunStageInfo.m_PhaseRunstageInfo[iStageIndex + 1].m_nConcurrencyPhase[jRingIndex];
                    if (m_nPhaseConcurInfo[iPhaseNum - 1][jPhaseNum - 1] == 1)
                    {
                        isConcurrency = true;
                        break;
                    }
                }
                if (isConcurrency)
                {
                    break;
                }
            }
            if (!isConcurrency)
            {
                for (iRingIndex = 0; iRingIndex < m_tRunStageInfo.m_PhaseRunstageInfo[iStageIndex].m_nConcurrencyPhaseCount; iRingIndex++)
                {
                    for (iPhaseIndex = 0; iPhaseIndex < m_tFixTimeCtlInfo.m_atPhaseSeq[iRingIndex].m_nPhaseCount; iPhaseIndex++)
                    {
                        if (m_tFixTimeCtlInfo.m_atPhaseSeq[iRingIndex].m_atPhaseInfo[iPhaseIndex].m_tPhaseParam.m_byPhaseNumber == m_tRunStageInfo.m_PhaseRunstageInfo[iStageIndex].m_nConcurrencyPhase[iRingIndex])
                        {
                            //�ҵ�iStageIndex��Ӧ����λ
                            m_bLastPhaseBeforeBarrier[iRingIndex][iPhaseIndex] = true;
                            m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "IsLastPhaseBeforeBarrier RingIndex:%d PhaseIndex:%d", iRingIndex, iPhaseIndex);
                        }
                    }
                }
            }
        }
    }
}

/*==================================================================== 
������ ��GetLedScreenShowInfo
���� ��������ʾ����ʾ��Ϣ״̬
�㷨ʵ�� �� 
����˵�� ��tLedScreenShowInfo����ʾ����Ϣ
����ֵ˵������
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ����Ƽ          �������� 
====================================================================*/ 
void CLogicCtlFixedTime::GetLedScreenShowInfo(TLedScreenShowInfo & tLedScreenShowInfo)
{
    int i = 0, j = 0;
    static WORD m_wPhaseGreenTime[MAX_RING_COUNT];

    if (!tLedScreenShowInfo.m_bKeyDirectionControl && !tLedScreenShowInfo.m_bChannelLockCheck)
    {
        WORD wCycleLen = m_tFixTimeCtlInfo.m_wCycleLen;
        if (tLedScreenShowInfo.m_nCurCtlMode == CTL_MODE_ACTUATE || tLedScreenShowInfo.m_nCurCtlMode == CTL_MODE_CABLELESS || tLedScreenShowInfo.m_nCurCtlMode == CTL_MODE_PEDCROSTREET) 
        {
            WORD wTempCycleLen[MAX_RING_COUNT];
			memset(wTempCycleLen, 0, MAX_RING_COUNT);
			for (i = 0;i < m_tFixTimeCtlInfo.m_nRingCount;i ++)
			{
				for (j = 0;j < m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_nPhaseCount;j ++)
				{
					if (m_nSplitTime[i][j] > 0)
					{
						wTempCycleLen[i] += m_nSplitTime[i][j];
					}
					else
					{
						wTempCycleLen[i] += m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[j].m_wPhaseTime;
					}
				}

				if (i == 1)
				{
					if (wTempCycleLen[1] > wTempCycleLen[0])
					{
						wCycleLen = wTempCycleLen[1];
					}
					else
					{
						wCycleLen = wTempCycleLen[0];
					}
				}
			}
        }

		tLedScreenShowInfo.m_nPlanNo = m_tFixTimeCtlInfo.m_wPatternNumber;
        tLedScreenShowInfo.m_nCycleLen = wCycleLen;
        if (tLedScreenShowInfo.m_nCurCtlMode != CTL_MODE_CABLELESS)
        {
            tLedScreenShowInfo.m_nOffset = m_tFixTimeCtlInfo.m_wPhaseOffset;
        }
        else
        {
            tLedScreenShowInfo.m_nOffset = abs(m_nRealOffset);
        }
     
        tLedScreenShowInfo.m_nRingCount = m_tFixTimeCtlInfo.m_nRingCount;

        TPhaseLampClrRunCounter tRunCounter;
        m_pOpenATCRunStatus->GetPhaseLampClrRunCounter(tRunCounter);

        for (i = 0;i < m_tFixTimeCtlInfo.m_nRingCount;i ++)
        {
            int nIndex = m_tFixTimeCtlInfo.m_nCurPhaseIndex[i];

            PTRingCtlInfo ptRingRunInfo = &(m_tFixTimeCtlInfo.m_atPhaseSeq[i]);
            PTPhaseCtlInfo pPhaseCtlInfo = (PTPhaseCtlInfo)&(ptRingRunInfo->m_atPhaseInfo[nIndex]);

            tLedScreenShowInfo.m_tScreenRingInfo[i].m_nCurRunPhaseIndex = m_tFixTimeCtlInfo.m_nCurPhaseIndex[i];//��ǰ������λ����
       
            if (pPhaseCtlInfo->m_chPhaseStage == C_CH_PHASESTAGE_G)
            { 
                tLedScreenShowInfo.m_tScreenRingInfo[i].m_nSplitTime = tRunCounter.m_nLampClrTime[i];
                m_wPhaseGreenTime[i] = tRunCounter.m_nLampClrTime[i];
            }
            else if (pPhaseCtlInfo->m_chPhaseStage == C_CH_PHASESTAGE_GF)
            { 
                tLedScreenShowInfo.m_tScreenRingInfo[i].m_nSplitTime = m_wPhaseGreenTime[i] + tRunCounter.m_nLampClrTime[i];
            }
            else if (pPhaseCtlInfo->m_chPhaseStage == C_CH_PHASESTAGE_Y)
            { 
                tLedScreenShowInfo.m_tScreenRingInfo[i].m_nSplitTime = m_wPhaseGreenTime[i] + pPhaseCtlInfo->m_tPhaseParam.m_byGreenFlash + tRunCounter.m_nLampClrTime[i];
            }
            else if (pPhaseCtlInfo->m_chPhaseStage == C_CH_PHASESTAGE_R)
            { 
                tLedScreenShowInfo.m_tScreenRingInfo[i].m_nSplitTime = m_wPhaseGreenTime[i] + pPhaseCtlInfo->m_tPhaseParam.m_byGreenFlash + pPhaseCtlInfo->m_tPhaseParam.m_byPhaseYellowChange + tRunCounter.m_nLampClrTime[i];
            }
        }
    }
    else
    {
        int i = 0, j = 0, nChannelIndex = 0;

        if (tLedScreenShowInfo.m_bKeyDirectionControl)
        {
            tLedScreenShowInfo.m_nChannelCount = 0;

            TAscManualPanel tAscManualPanel;
            memset(&tAscManualPanel, 0, sizeof(TAscManualPanel)); 
            m_pOpenATCParameter->GetManualPanelInfo(tAscManualPanel);

            for (i = 0;i < MAX_PANEL_KEY_CFG_COUNT;i++)
	        {
		        if (tAscManualPanel.m_stPanelKeyCfg[i].m_byKeyNum == tLedScreenShowInfo.m_nDirectionKeyIndex)
		        {    
			        for (j = 0;j < m_nChannelCount;j++)
			        {
                        if (tAscManualPanel.m_stPanelKeyCfg[i].m_ChannelCfg[j].m_byChannelStatus == CHANNEL_STATUS_GREEN ||
                            tAscManualPanel.m_stPanelKeyCfg[i].m_ChannelCfg[j].m_byChannelStatus == CHANNEL_STATUS_YELLOW)
                        {
                            tLedScreenShowInfo.m_tDirectionKeyChannelClr[tLedScreenShowInfo.m_nChannelCount].m_nChannelID = tAscManualPanel.m_stPanelKeyCfg[i].m_ChannelCfg[j].m_byChannelID;
                            tLedScreenShowInfo.m_nChannelCount += 1;
                        }
			        }
  
                    break;
		        } 
	        }
       
    	    //��ȡ��ǰͨ���ĵ�ɫ
            for (i = 0;i < tLedScreenShowInfo.m_nChannelCount;i++)
            {
                for (j = 0;j < m_nChannelCount; j++)
                {
					if (m_atChannelInfo[j].m_byChannelNumber == 0)
					{
						continue;
					}

                    if (tLedScreenShowInfo.m_tDirectionKeyChannelClr[i].m_nChannelID == m_atChannelInfo[j].m_byChannelNumber)
                    {
                        if (m_achLampClr[(m_atChannelInfo[j].m_byChannelNumber - 1) * 3] == LAMP_CLR_ON)
		                {
                            tLedScreenShowInfo.m_tDirectionKeyChannelClr[i].m_achChannelClr = C_CH_PHASESTAGE_R;
		                }

		                if (m_achLampClr[(m_atChannelInfo[j].m_byChannelNumber - 1) * 3 + 1] == LAMP_CLR_ON)
		                {
                            tLedScreenShowInfo.m_tDirectionKeyChannelClr[i].m_achChannelClr = C_CH_PHASESTAGE_Y;
		                }
		              
		                if (m_achLampClr[(m_atChannelInfo[j].m_byChannelNumber - 1) * 3 + 2] == LAMP_CLR_ON ||
							m_achLampClr[(m_atChannelInfo[j].m_byChannelNumber - 1) * 3 + 2] == LAMP_CLR_FLASH)
		                {
                            tLedScreenShowInfo.m_tDirectionKeyChannelClr[i].m_achChannelClr = C_CH_PHASESTAGE_G;
		                }
              
                        tLedScreenShowInfo.m_tDirectionKeyChannelClr[i].m_nChannelDurationTime = m_nChannelDurationTime[j];
                    }
                }
            }    
        }
		else if (tLedScreenShowInfo.m_bChannelLockCheck)
		{
			tLedScreenShowInfo.m_nChannelLockCount = 0;

			for (i = 0;i < MAX_CHANNEL_COUNT;i++)
			{
				if (tLedScreenShowInfo.m_tChannelLockCtrlCmd.m_nChannelLockStatus[i] == CHANNEL_STATUS_GREEN ||
					tLedScreenShowInfo.m_tChannelLockCtrlCmd.m_nChannelLockStatus[i] == CHANNEL_STATUS_YELLOW)
				{
					tLedScreenShowInfo.m_tChannelLockChannelClr[tLedScreenShowInfo.m_nChannelLockCount].m_nChannelID = i + 1;
					tLedScreenShowInfo.m_nChannelLockCount += 1;
				}
			}
	  
			//��ȡ��ǰͨ���ĵ�ɫ
			for (i = 0;i < tLedScreenShowInfo.m_nChannelLockCount;i++)
			{
				for (j = 0;j < m_nChannelCount; j++)
				{
					if (m_atChannelInfo[j].m_byChannelNumber == 0)
					{
						continue;
					}

					if (tLedScreenShowInfo.m_tChannelLockChannelClr[i].m_nChannelID == m_atChannelInfo[j].m_byChannelNumber)
					{
						if (m_achLampClr[(m_atChannelInfo[j].m_byChannelNumber - 1) * 3] == LAMP_CLR_ON)
						{
							tLedScreenShowInfo.m_tChannelLockChannelClr[i].m_achChannelClr = C_CH_PHASESTAGE_R;
						}

						if (m_achLampClr[(m_atChannelInfo[j].m_byChannelNumber - 1) * 3 + 1] == LAMP_CLR_ON)
						{
							tLedScreenShowInfo.m_tChannelLockChannelClr[i].m_achChannelClr = C_CH_PHASESTAGE_Y;
						}
		              
						if (m_achLampClr[(m_atChannelInfo[j].m_byChannelNumber - 1) * 3 + 2] == LAMP_CLR_ON ||
							m_achLampClr[(m_atChannelInfo[j].m_byChannelNumber - 1) * 3 + 2] == LAMP_CLR_FLASH)
						{
							tLedScreenShowInfo.m_tChannelLockChannelClr[i].m_achChannelClr = C_CH_PHASESTAGE_G;
						}
              
						tLedScreenShowInfo.m_tChannelLockChannelClr[i].m_nChannelDurationTime = m_nChannelDurationTime[j];
						//��λ����ʱ����ʾ����λ����
						if (tLedScreenShowInfo.m_bPhaseToChannelLock)
						{
							for (int nPhaseIndex = 0;nPhaseIndex < tLedScreenShowInfo.m_nPhaseLockCount;nPhaseIndex++)
							{
								if (m_atChannelInfo[j].m_byChannelControlSource == tLedScreenShowInfo.m_tPhaseClr[nPhaseIndex].m_byPhaseID)
								{
									tLedScreenShowInfo.m_tPhaseClr[nPhaseIndex].m_achPhaseClr = tLedScreenShowInfo.m_tChannelLockChannelClr[i].m_achChannelClr;
									tLedScreenShowInfo.m_tPhaseClr[nPhaseIndex].m_nPhaseClrDurationTime = m_nChannelDurationTime[j];
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
������ ��SendOverlapPhasePulse
���� �����͸�����λ�ĵ���ʱ����
�㷨ʵ�� �� 
����˵�� ��nRingIndex������
           nCurPhaseNum����ǰ��λID
           nPhaseNum����ǰ��λ����һ����λID
           bRedPulse���Ǻ�����
           byOverlapNum��������λID����
           bSendPulse���Ƿ�����������
����ֵ˵������
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ����Ƽ          �������� 
====================================================================*/ 
void CLogicCtlFixedTime::SendOverlapPhasePulse(int nRingIndex, int nCurPhaseNum, int nPhaseNum, bool bRedPulse, BYTE byOverlapNum[], bool bSendPulse[], bool bManual, int nNextStage)
{
    int  i = 0, j = 0, k = 0;
    int  nCount = 0;

	bool bFlag = true;
	int  nCurStageIndex = 0;
	int  nNextStageIndex = 0;
	int  nConcurrencyPhaseIndex = 0;

    for (i = 0;i < m_tFixTimeCtlInfo.m_nOverlapCount;i++)
    {
        if (m_tFixTimeCtlInfo.m_atOverlapInfo[i].m_tOverlapParam.m_byOverlapNumber == 0)
        {
            continue;
        }

        for (j = 0;j < MAX_PHASE_COUNT_IN_OVERLAP;j++)
        {
            BYTE byMainPhaseNum = m_tFixTimeCtlInfo.m_atOverlapInfo[i].m_tOverlapParam.m_byArrOverlapIncludedPhases[j];
            if (byMainPhaseNum == 0)
            {
                continue;
            }
            
            if (byMainPhaseNum == nCurPhaseNum)
            {
		        if (!bRedPulse)
			    {
					bFlag = true;

					TPhasePassCmdPhaseStatus tPhasePassCmdPhaseStatus;
					memset(&tPhasePassCmdPhaseStatus, 0, sizeof(tPhasePassCmdPhaseStatus));
					m_pOpenATCRunStatus->GetLocalPhasePassStatus(tPhasePassCmdPhaseStatus);

					//��������ʱ�����������λ���浱ǰ���е���λ�����Ҹ��浱ǰ���е���λ����һ����λ���򲻷�������
					if (nCurPhaseNum != nPhaseNum && IsPhaseNumInOverlap(nPhaseNum, i) && tPhasePassCmdPhaseStatus.m_nPhasePassStatus[nPhaseNum - 1] == PhasePassStatus_Normal &&
						!IsNeglectPhase(nPhaseNum))
					{
						bFlag = false;
					}

					nCurStageIndex = m_tFixTimeCtlInfo.m_nCurStageIndex;
					for (nConcurrencyPhaseIndex = 0;nConcurrencyPhaseIndex < m_tRunStageInfo.m_PhaseRunstageInfo[nCurStageIndex].m_nConcurrencyPhaseCount;nConcurrencyPhaseIndex++)
					{
						if (nConcurrencyPhaseIndex != nRingIndex && IsPhaseNumInOverlap(m_tRunStageInfo.m_PhaseRunstageInfo[nCurStageIndex].m_nConcurrencyPhase[nConcurrencyPhaseIndex], i))
						{
							//��������ʱ�����������λ�������λ��ͬһ���׶Σ�ֻҪ����׶λ�����λ��������û�з��ͣ��򲻷�������
							if (m_tPhasePulseStatus[nConcurrencyPhaseIndex].m_nPhaseNum == m_tRunStageInfo.m_PhaseRunstageInfo[nCurStageIndex].m_nConcurrencyPhase[nConcurrencyPhaseIndex] &&
								m_tPhasePulseStatus[nConcurrencyPhaseIndex].m_nGreenPulseSendStatus != SEND_END)
							{
								bFlag = false;
								//m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "OverLapPhase %d IncludedPhase %d GreenPulse has not sended!", i, byMainPhaseNum);
								break;
							}
						}	
					}

					if (!bManual)
					{
						nNextStageIndex = m_tFixTimeCtlInfo.m_nCurStageIndex + 1;
					}
					else
					{
						nNextStageIndex = nNextStage;
					}

					if (nNextStageIndex >= m_tRunStageInfo.m_nRunStageCount)
					{
						nNextStageIndex = 0;
					}
					for (nConcurrencyPhaseIndex = 0;nConcurrencyPhaseIndex < m_tRunStageInfo.m_PhaseRunstageInfo[nNextStageIndex].m_nConcurrencyPhaseCount;nConcurrencyPhaseIndex++)
					{
						//��������ʱ�����������λ�������λ�����ڽ׶Σ��򲻷�������
						if (nConcurrencyPhaseIndex != nRingIndex && IsPhaseNumInOverlap(m_tRunStageInfo.m_PhaseRunstageInfo[nNextStageIndex].m_nConcurrencyPhase[nConcurrencyPhaseIndex], i) &&
						    tPhasePassCmdPhaseStatus.m_nPhasePassStatus[nCurPhaseNum - 1] == PhasePassStatus_Normal &&
							tPhasePassCmdPhaseStatus.m_nPhasePassStatus[m_tRunStageInfo.m_PhaseRunstageInfo[nNextStageIndex].m_nConcurrencyPhase[nConcurrencyPhaseIndex] - 1] == PhasePassStatus_Normal)
						{
							bFlag = false;
							//m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "OverLapPhase %d Follow Phase %d And Phase %d!", i, byMainPhaseNum, m_tRunStageInfo.m_PhaseRunstageInfo[nCurStageIndex].m_nConcurrencyPhase[nConcurrencyPhaseIndex]);
							break;
					    }
					}

					if (bFlag)
					{
						byOverlapNum[nCount] = m_tFixTimeCtlInfo.m_atOverlapInfo[i].m_tOverlapParam.m_byOverlapNumber;
						bSendPulse[nCount] = true;
						nCount += 1;
					}
			    }
            }
            else if (byMainPhaseNum == nPhaseNum)
            {   
				TPhasePassCmdPhaseStatus tPhasePassCmdPhaseStatus;
				memset(&tPhasePassCmdPhaseStatus, 0, sizeof(tPhasePassCmdPhaseStatus));
				m_pOpenATCRunStatus->GetLocalPhasePassStatus(tPhasePassCmdPhaseStatus);

                //���ͺ�����ʱ�����������λ�����浱ǰ���е���λ�����Ǹ��浱ǰ���е���һ����λ��������
                if (bRedPulse && (!IsPhaseNumInOverlap(nCurPhaseNum, i) || IsNeglectPhase(nCurPhaseNum))&& tPhasePassCmdPhaseStatus.m_nPhasePassStatus[nPhaseNum - 1] == PhasePassStatus_Normal &&
					!IsNeglectPhase(nPhaseNum))
                {
                    byOverlapNum[nCount] = m_tFixTimeCtlInfo.m_atOverlapInfo[i].m_tOverlapParam.m_byOverlapNumber;  
                    bSendPulse[nCount] = true;
                    nCount += 1;
                }
            }
        }
    }
}

/*==================================================================== 
������ ��SetChannelSplitMode
���� ������ͨ����Ӧ����λ����ͨ����Ӧ�����ű�ģʽ
�㷨ʵ�� �� 
����˵�� ����
����ֵ˵������
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ����Ƽ          �������� 
====================================================================*/
void CLogicCtlFixedTime::SetChannelSplitMode()
{
    //int i = 0, j = 0, k = 0;
    //bool bNeglectMode = true;

    ////for (i = 0;i < MAX_PHASE_COUNT;i ++)
    //{
    //    //if (m_atSplitInfo[i].m_bySplitMode == NEGLECT_MODE)
    //    {
    //        for (int nChannelIndex = 0;nChannelIndex < MAX_CHANNEL_COUNT;nChannelIndex++)
    //        {
				//if (m_atChannelInfo[nChannelIndex].m_byChannelNumber == 0)
				//{
				//	continue;
				//}

    //            bNeglectMode = true;

    //            if (m_atChannelInfo[nChannelIndex].m_byChannelControlType != OVERLAP_CHA && m_atChannelInfo[nChannelIndex].m_byChannelControlType != OVERLAP_PED_CHA)
    //            {
    //                //�ж϶�Ӧ�Ǹ�����λ��ͨ���Ŀ���Դ�ǲ��Ǻ�����λ
				//	for (i = 0;i < MAX_PHASE_COUNT;i ++)
				//	{
				//		if (m_atChannelInfo[nChannelIndex].m_byChannelControlSource == m_atSplitInfo[i].m_bySplitPhase &&
				//			m_atSplitInfo[i].m_bySplitMode != NEGLECT_MODE)
				//		{
				//			bNeglectMode = false;  
				//		}
				//	}
    //            }
    //            else
    //            {
    //                for (j = 0;j < m_tFixTimeCtlInfo.m_nOverlapCount;j ++)
    //                {
    //                    if (m_atChannelInfo[nChannelIndex].m_byChannelControlSource == m_tFixTimeCtlInfo.m_atOverlapInfo[j].m_tOverlapParam.m_byOverlapNumber)
    //                    {
    //                        for (k = 0;k < MAX_PHASE_COUNT_IN_OVERLAP;k ++)
    //                        {
    //                            //ͨ����Ӧ������λ���ж�ĸ��λ�ǲ��Ǻ�����λ
				//				for (i = 0;i < MAX_PHASE_COUNT;i ++)
				//				{
				//					if (m_tFixTimeCtlInfo.m_atOverlapInfo[j].m_tOverlapParam.m_byArrOverlapIncludedPhases[k] > 0 && 
				//						m_tFixTimeCtlInfo.m_atOverlapInfo[j].m_tOverlapParam.m_byArrOverlapIncludedPhases[k] == m_atSplitInfo[i].m_bySplitPhase && 
				//						m_atSplitInfo[i].m_bySplitMode != NEGLECT_MODE)
				//					{
				//						bNeglectMode = false;  
				//					}
				//				} 
    //                        }

    //                        if (!bNeglectMode)
    //                        {
    //                            break;
    //                        }
    //                    }
    //                }
    //            }

    //            if (bNeglectMode)
    //            {
    //               m_nChannelSplitMode[nChannelIndex] = NEGLECT_MODE;  
    //               m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "Channel ID:%d is Neglect", m_atChannelInfo[nChannelIndex].m_byChannelNumber);
    //            }
    //        }
    //    }
    //}

    int i = 0, j = 0, k = 0;
    for (int nChannelIndex = 0; nChannelIndex < MAX_CHANNEL_COUNT; nChannelIndex++)
    {
        //�����Ʋ��������
        if (m_atChannelInfo[nChannelIndex].m_byChannelNumber == 0 || m_atChannelInfo[nChannelIndex].m_byChannelControlType == LANEWAY_LIGHT_CHA)
        {
            continue;
        }

        //������Ǹ�����λ�����ǳ�����
        if (m_atChannelInfo[nChannelIndex].m_byChannelControlType != OVERLAP_CHA && m_atChannelInfo[nChannelIndex].m_byChannelControlType != OVERLAP_PED_CHA)
        {
            for (i = 0; i < MAX_PHASE_COUNT; i++)
            {
                if (m_atChannelInfo[nChannelIndex].m_byChannelControlSource == m_atSplitInfo[i].m_bySplitPhase && m_atSplitInfo[i].m_bySplitMode == NEGLECT_MODE)
                {
                    m_nChannelSplitMode[nChannelIndex] = NEGLECT_MODE;
                    m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "Channel ID:%d is Neglect", m_atChannelInfo[nChannelIndex].m_byChannelNumber);
                }
                else if (m_atChannelInfo[nChannelIndex].m_byChannelControlSource == m_atSplitInfo[i].m_bySplitPhase && m_atSplitInfo[i].m_bySplitMode == SHIELD_MODE)
                {
                    m_nChannelSplitMode[nChannelIndex] = SHIELD_MODE;
                    m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "Channel ID:%d is Shield", m_atChannelInfo[nChannelIndex].m_byChannelNumber);
                }
            }
        }
        else
        {
            bool bNeglectMode = true;
            //������λ:������λ�����������ƣ���ֱ�Ӷ�Ӧͨ��Ϊ�ضϡ�ȫ�졿
            for (j = 0; j < m_tFixTimeCtlInfo.m_nOverlapCount; j++)
            {
                if (m_atChannelInfo[nChannelIndex].m_byChannelControlSource == m_tFixTimeCtlInfo.m_atOverlapInfo[j].m_tOverlapParam.m_byOverlapNumber)
                {
                    for (k = 0; k < strlen((const char *)m_tFixTimeCtlInfo.m_atOverlapInfo[j].m_tOverlapParam.m_byArrOverlapIncludedPhases); k++)
                    {
                        //ͨ����Ӧ������λ���ж�ĸ��λ�ǲ��Ǻ�����λ
                        for (i = 0; i < MAX_PHASE_COUNT; i++)
                        {
                            if (m_tFixTimeCtlInfo.m_atOverlapInfo[j].m_tOverlapParam.m_byArrOverlapIncludedPhases[k] > 0 &&
                                m_tFixTimeCtlInfo.m_atOverlapInfo[j].m_tOverlapParam.m_byArrOverlapIncludedPhases[k] == m_atSplitInfo[i].m_bySplitPhase &&
                                m_atSplitInfo[i].m_bySplitMode != NEGLECT_MODE && m_atSplitInfo[i].m_bySplitMode != SHIELD_MODE)
                            {
                                bNeglectMode = false;
                            }

							if (strlen((const char *)m_tFixTimeCtlInfo.m_atOverlapInfo[j].m_tOverlapParam.m_byArrOverlapIncludedPhases) == 1)
							{
								if (m_tFixTimeCtlInfo.m_atOverlapInfo[j].m_tOverlapParam.m_byArrOverlapIncludedPhases[0] == m_atSplitInfo[i].m_bySplitPhase)
								{
									if (m_atSplitInfo[i].m_bySplitMode == NEGLECT_MODE)
									{
										m_nChannelSplitMode[nChannelIndex] = NEGLECT_MODE;
										m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "Channel ID:%d is Neglect", m_atChannelInfo[nChannelIndex].m_byChannelNumber);
									}
									else  if (m_atSplitInfo[i].m_bySplitMode == SHIELD_MODE)
									{
										m_nChannelSplitMode[nChannelIndex] = SHIELD_MODE;
										m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "Channel ID:%d is Shield", m_atChannelInfo[nChannelIndex].m_byChannelNumber);
									}
									break;
								}
							}
                        }
                    }
                    if (!bNeglectMode)
                    {
                        break;
                    }
                }
            }

            if (bNeglectMode)
            {
                //m_nChannelSplitMode[nChannelIndex] = SHIELD_MODE;
                //m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "Channel ID:%d is Shield", m_atChannelInfo[nChannelIndex].m_byChannelNumber);
            }
        }
    }
}


/*==================================================================== 
������ ��SetOverLapGreenPulse
���� �����ø�����λ���̵�����
�㷨ʵ�� �� 
����˵�� ��int nRingIndx��������
           int nIndex����λ����
		   bool bState��״̬
����ֵ˵����
          ��
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ����Ƽ          �������� 
====================================================================*/ 
void CLogicCtlFixedTime::SetOverLapGreenPulse(int nRingIndex, int nIndex, bool bManual, int nNextStage, bool bState)
{
    int i = 0;
	size_t j = 0;

    TLampClrStatus tLampClrStatus;
    memset(&tLampClrStatus,0,sizeof(TLampClrStatus)); 
    m_pOpenATCRunStatus->GetLampClrStatus(tLampClrStatus);
    SetLampClr(tLampClrStatus);

    BYTE byOverlapNum[MAX_PHASE_COUNT_IN_OVERLAP];
    memset(byOverlapNum, 0, sizeof(byOverlapNum));

    bool bSendPulse[MAX_PHASE_COUNT_IN_OVERLAP];
    memset(bSendPulse, 0, sizeof(bSendPulse));

    int nNextIndex = 0;
	if (!bManual)
	{
		nNextIndex = nIndex + 1;
		if (nNextIndex == m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_nPhaseCount)
		{
			nNextIndex = 0;
		}
	}
	else
	{
		int nNextPhaseID = m_tRunStageInfo.m_PhaseRunstageInfo[nNextStage].m_nConcurrencyPhase[nRingIndex];
		PTRingCtlInfo ptRingRunInfo = &(m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex]);
		for (i = 0;i < ptRingRunInfo->m_nPhaseCount;i++)
		{
			if (ptRingRunInfo->m_atPhaseInfo[i].m_tPhaseParam.m_byPhaseNumber == nNextPhaseID)
			{
				nNextIndex = i;
				break;
			}
		}
		/*nNextIndex = nIndex + 1;
		if (nNextIndex == m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_nPhaseCount)
		{
			nNextIndex = 0;
		}*/
	}

    SendOverlapPhasePulse(nRingIndex, m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhaseNumber, m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nNextIndex].m_tPhaseParam.m_byPhaseNumber, false, byOverlapNum, bSendPulse, bManual, nNextStage);

    if (strlen((const char *)byOverlapNum) > 0)
    { 
        for (i = 0;i < m_nChannelCount;i++)
        {
			if (m_atChannelInfo[i].m_byChannelNumber == 0)
			{
				continue;
			}

            if (m_atChannelInfo[i].m_byChannelControlType == OVERLAP_CHA || m_atChannelInfo[i].m_byChannelControlType == OVERLAP_PED_CHA)
            {
                for (j = 0;j < strlen((const char *)byOverlapNum);j++)
                {
                    if (m_atChannelInfo[i].m_byChannelControlSource == byOverlapNum[j] && bSendPulse[j])
                    {
                        tLampClrStatus.m_bGreenLampPulse[(m_atChannelInfo[i].m_byChannelNumber - 1) * 3 + 2] = bState;
                        if (bState)
                        {
                            //m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "SendOverlapPhaseGreenPulse RingIndex:%d OverlapNum:%d State:%d", nRingIndex, byOverlapNum[j], bState);
                        }
                    }
                }
            }
        } 
    }

    m_pOpenATCRunStatus->SetLampClrStatus(tLampClrStatus);
}

/*==================================================================== 
������ ��SetOverLapRedPulse
���� �����ø�����λ�ĺ������
�㷨ʵ�� �� 
����˵�� ��int nRingIndex��������
           int nIndex����λ����
		   bool bState��״̬
����ֵ˵����
          ��
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ����Ƽ          �������� 
====================================================================*/ 
void CLogicCtlFixedTime::SetOverLapRedPulse(int nRingIndex, int nIndex, bool bManual, int nNextStage, bool bState)
{
    int i = 0, k = 0;
	size_t j = 0;
    TLampClrStatus tLampClrStatus;
    memset(&tLampClrStatus,0,sizeof(TLampClrStatus)); 
    m_pOpenATCRunStatus->GetLampClrStatus(tLampClrStatus);
    SetLampClr(tLampClrStatus);

    int nNextIndex = 0;
	if (!bManual)
	{
		nNextIndex = nIndex + 1;
		if (nNextIndex == m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_nPhaseCount)
		{
			nNextIndex = 0;
		}
	}
	else
	{
		int nNextPhaseID = m_tRunStageInfo.m_PhaseRunstageInfo[nNextStage].m_nConcurrencyPhase[nRingIndex];
		PTRingCtlInfo ptRingRunInfo = &(m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex]);
		for (i = 0;i < ptRingRunInfo->m_nPhaseCount;i++)
		{
			if (ptRingRunInfo->m_atPhaseInfo[i].m_tPhaseParam.m_byPhaseNumber == nNextPhaseID)
			{
				nNextIndex = i;
				break;
			}
		}
	}

    BYTE byOverlapNum[MAX_PHASE_COUNT_IN_OVERLAP];
    memset(byOverlapNum, 0, sizeof(byOverlapNum));

    bool bSendPulse[MAX_PHASE_COUNT_IN_OVERLAP];
    memset(bSendPulse, 0, sizeof(bSendPulse));

    SendOverlapPhasePulse(nRingIndex, m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhaseNumber, m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nNextIndex].m_tPhaseParam.m_byPhaseNumber, true, byOverlapNum, bSendPulse, bManual, nNextStage);

    if (strlen((const char *)byOverlapNum) > 0)
    { 
        for (i = 0;i < m_nChannelCount;i++)
        {
			if (m_atChannelInfo[i].m_byChannelNumber == 0)
			{
				continue;
			}

            if (m_atChannelInfo[i].m_byChannelControlType == OVERLAP_CHA || m_atChannelInfo[i].m_byChannelControlType == OVERLAP_PED_CHA)
            {
                for (j = 0;j < strlen((const char *)byOverlapNum);j++)
                {
                    if (m_atChannelInfo[i].m_byChannelControlSource == byOverlapNum[j] && bSendPulse[j])
                    { 
                        if (m_achLampClr[(m_atChannelInfo[i].m_byChannelNumber - 1) * 3] == LAMP_CLR_ON)
                        {
                            tLampClrStatus.m_bRedLampPulse[(m_atChannelInfo[i].m_byChannelNumber - 1) * 3] = bState;
                        }
                        else if (m_achLampClr[(m_atChannelInfo[i].m_byChannelNumber - 1) * 3 + 2] == LAMP_CLR_ON)
                        {
                            //tLampClrStatus.m_bGreenLampPulse[(m_atChannelInfo[i].m_byChannelNumber - 1) * 3 + 2] = bState;
                        }
                        if (bState)
                        {
                            //m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "SendOverlapPhaseRedPulse RingIndex:%d OverlapNum:%d State:%d", nRingIndex, byOverlapNum[j], bState);
                        }
                    }
                }
            }
        } 
    }

    m_pOpenATCRunStatus->SetLampClrStatus(tLampClrStatus);
}

/*==================================================================== 
������ ��SetOverLapPulse
���� �����ø�����λ������
�㷨ʵ�� �� 
����˵�� ����
����ֵ˵����
          ��
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ����Ƽ          �������� 
====================================================================*/ 
void CLogicCtlFixedTime::SetOverLapPulse(bool bManual, int nNextStageIndex)
{
	int nRingIndex = 0;
	int nPhaseIndex = 0;
	for (nRingIndex = 0;nRingIndex < MAX_RING_COUNT;nRingIndex++)
	{
		nPhaseIndex = m_tPhasePulseStatus[nRingIndex].m_nPhaseIndex;

		if (m_tPhasePulseStatus[nRingIndex].m_bGreenPulseStatus)
		{
			m_tPhasePulseStatus[nRingIndex].m_nGreenPulseSendStatus = SEND_SRART;//�����忪ʼ����

			SetOverLapGreenPulse(nRingIndex, nPhaseIndex, bManual, nNextStageIndex, true);
		}
		else
		{
			if (m_tPhasePulseStatus[nRingIndex].m_nGreenPulseSendStatus == SEND_SRART)
			{
				SetOverLapGreenPulse(nRingIndex, nPhaseIndex, bManual, nNextStageIndex, false);

				m_tPhasePulseStatus[nRingIndex].m_nGreenPulseSendStatus = SEND_END;//�����巢�ͽ���
			}
		}

		if (m_tPhasePulseStatus[nRingIndex].m_bRedPulseStatus)
		{
			m_tPhasePulseStatus[nRingIndex].m_nRedPulseSendStatus = SEND_SRART;//�����忪ʼ����

			SetOverLapRedPulse(nRingIndex, nPhaseIndex, bManual, nNextStageIndex, true);
		}
		else
		{
			if (m_tPhasePulseStatus[nRingIndex].m_nRedPulseSendStatus == SEND_SRART)
			{
				SetOverLapRedPulse(nRingIndex, nPhaseIndex, bManual, nNextStageIndex, false);
				
				m_tPhasePulseStatus[nRingIndex].m_nRedPulseSendStatus = SEND_END;//�����巢�ͽ���
			}
		}
	}
}

char CLogicCtlFixedTime::GetOverlapType(char chOverlapStatus)
{
	char chRet = 1;
	if (chOverlapStatus == C_CH_PHASESTAGE_G)
	{
		chRet = 3;
	}
	else if (chOverlapStatus == C_CH_PHASESTAGE_GF)
	{
        chRet = 4;
	}
	else if (chOverlapStatus == C_CH_PHASESTAGE_Y)
	{
        chRet = 2;
	}
	else if (chOverlapStatus == C_CH_PHASESTAGE_R)
	{
        chRet = 1;
	}
	else if (chOverlapStatus == C_CH_PHASESTAGE_OF)
	{
        chRet = 0;
	}
	else if (chOverlapStatus == C_CH_PHASESTAGE_YF)
	{
        chRet = 5;
	}
    return chRet;
}

/*==================================================================== 
������ ��IsChannelGreen
���� ���жϵ�ǰͨ���ĵ�ɫ�Ƿ�����ɫ
�㷨ʵ�� �� 
����˵�� ��byChannelType��ͨ�����ͣ�nDirectionIndex�������ţ�nChannelLockStatus������ͨ����ɫ
����ֵ˵������
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ����Ƽ          �������� 
====================================================================*/ 
bool CLogicCtlFixedTime::IsChannelGreen(BYTE byChannelType, int nDirectionIndex, int nChannelLockStatus[])
{
	bool bFlag = false;
	int i = 0, j = 0;

	if (byChannelType == CHANNEL_TYPE_DIRECTION)
	{
		TAscManualPanel tAscManualPanel;
		memset(&tAscManualPanel, 0, sizeof(TAscManualPanel)); 
		m_pOpenATCParameter->GetManualPanelInfo(tAscManualPanel);

		for (i = 0;i < MAX_PANEL_KEY_CFG_COUNT;i++)
		{
			if (tAscManualPanel.m_stPanelKeyCfg[i].m_byKeyNum == nDirectionIndex)
			{    
				for (j = 0;j < m_nChannelCount;j++)
				{
					if (m_atChannelInfo[j].m_byChannelNumber == 0)
					{
						continue;
					}

					if (tAscManualPanel.m_stPanelKeyCfg[i].m_ChannelCfg[j].m_byChannelID == m_atChannelInfo[j].m_byChannelNumber && 
						tAscManualPanel.m_stPanelKeyCfg[i].m_ChannelCfg[j].m_byChannelStatus != CHANNEL_STATUS_DEFAULT)
					{
						if (m_achLampClr[(m_atChannelInfo[j].m_byChannelNumber - 1) * 3 + 2] == LAMP_CLR_ON)
						{
							bFlag = true;
							break;
						}
					}
				}
  
				break;
			} 
		}
	}
	else
	{
		for (i = 0;i < MAX_CHANNEL_COUNT;i++)
		{
			for (j = 0;j < m_nChannelCount;j++)
			{
				if (m_atChannelInfo[j].m_byChannelNumber == 0)
				{
					continue;
				}

				if ((i + 1) == m_atChannelInfo[j].m_byChannelNumber && nChannelLockStatus[i] != CHANNEL_STATUS_DEFAULT)
				{
					if (m_achLampClr[(m_atChannelInfo[j].m_byChannelNumber - 1) * 3 + 2] == LAMP_CLR_ON)
					{
						bFlag = true;
						break;
					}
				}
			}

			if (bFlag)
			{
				break;
			}
		}
	}

	return bFlag;
}

/*====================================================================
������ ��SetChannelShieldAndProhibitStatus
���� ������ͨ����Ӧ����λ����ͨ����Ӧ�����κͽ�ֹ״̬
�㷨ʵ�� ��
����˵�� ��
����ֵ˵������
----------------------------------------------------------------------
�޸ļ�¼ ��
�� ��          �汾 �޸���  �߶���  �޸ļ�¼

====================================================================*/
void CLogicCtlFixedTime::SetChannelShieldAndProhibitStatus()
{
    memset(m_bShieldStatus, 0x00, sizeof(m_bShieldStatus));
    memset(m_bProhibitStatus, 0x00, sizeof(m_bProhibitStatus));
    TAscParam tAscParam;
    memset(&tAscParam, 0, sizeof(tAscParam));
    m_pOpenATCParameter->GetAscParamInfo(tAscParam);

    int i = 0, j = 0, k = 0;
    for (i = 0; i < m_nChannelCount; i++)
    {
        //ͨ��
        //if (tAscParam.m_stAscChannelTable[i].m_byScreenFlag == 1)
        //{
        //    m_bShieldStatus[i] = true;
        //}
        //if (tAscParam.m_stAscChannelTable[i].m_byForbiddenFlag == 1)
        //{
        //    m_bProhibitStatus[i] = true;
        //}

        //��λ
        for (j = 0; j < MAX_PHASE_COUNT; j++)
        {
            if (tAscParam.m_stAscPhaseTable[j].m_byPhaseNumber == m_atChannelInfo[i].m_byChannelControlSource)
            {
                if (tAscParam.m_stAscPhaseTable[j].m_byScreenFlag == 1)
                {
                    m_bShieldStatus[i] = true;
                }
                if (tAscParam.m_stAscPhaseTable[j].m_byForbiddenFlag == 1)
                {
                    m_bProhibitStatus[i] = true;
                }
            }
        }

        //����
        //for (j = 0; j < 16; j++)
        //{
        //    if (tAscParam.m_stAscPatternTable[m_tFixTimeCtlInfo.m_wPatternNumber - 1].m_byScreenStage[j] == 1)
        //    {
        //        for (k = 0; k < MAX_RING_COUNT; k++)
        //        {
        //            if (m_tRunStageInfo.m_PhaseRunstageInfo[j].m_nConcurrencyPhase[k] == m_atChannelInfo[i].m_byChannelControlSource)
        //            {
        //                m_bShieldStatus[i] = true;
        //            }
        //        }
        //    }
        //    if (tAscParam.m_stAscPatternTable[m_tFixTimeCtlInfo.m_wPatternNumber - 1].m_byForbiddenStage[j] == 1)
        //    {
        //        for (k = 0; k < MAX_RING_COUNT; k++)
        //        {
        //            if (m_tRunStageInfo.m_PhaseRunstageInfo[j].m_nConcurrencyPhase[k] == m_atChannelInfo[i].m_byChannelControlSource)
        //            {
        //                m_bProhibitStatus[i] = true;
        //            }
        //        }
        //    }
        //}
    }
}

/*====================================================================
������ ��SetLampByShieldAndProhibitStatus
���� ����λ��ֹ��������������ó���
�㷨ʵ�� ��
����˵�� ��
����ֵ˵������
----------------------------------------------------------------------
�޸ļ�¼ ��
�� ��          �汾 �޸���  �߶���  �޸ļ�¼

====================================================================*/
void CLogicCtlFixedTime::SetLampByShieldAndProhibitStatus(int nRingNum, int nPhaseCount, int nChannelCount)
{
    //���Ǹ����н׶����ν�ֹ�ָ����в���
    for (int jPhaseIndex = 0; jPhaseIndex < nPhaseCount; jPhaseIndex++)
    {
        for (int iChannelIndex = 0; iChannelIndex < nChannelCount; iChannelIndex++)
        {
            if (m_atChannelInfo[iChannelIndex].m_byChannelControlSource == m_tFixTimeCtlInfo.m_atPhaseSeq[nRingNum].m_atPhaseInfo[jPhaseIndex].m_tPhaseParam.m_byPhaseNumber)
            {
                /*if(memcmp(&m_bShieldStatus,&m_bOldShieldStatus,sizeof(m_bShieldStatus)) || memcmp(&m_bProhibitStatus,&m_bOldProhibitStatus,sizeof(m_bProhibitStatus)))*/
                if (m_bShieldStatus[iChannelIndex] != m_bOldShieldStatus[iChannelIndex] || m_bProhibitStatus[iChannelIndex] != m_bOldProhibitStatus[iChannelIndex])
                {
                    m_tFixTimeCtlInfo.m_atPhaseSeq[nRingNum].m_atPhaseInfo[jPhaseIndex].m_tPhaseParam.m_byForbiddenFlag = m_bProhibitStatus[iChannelIndex];
                    m_tFixTimeCtlInfo.m_atPhaseSeq[nRingNum].m_atPhaseInfo[jPhaseIndex].m_tPhaseParam.m_byScreenFlag = m_bShieldStatus[iChannelIndex];
                    m_bOldShieldStatus[iChannelIndex] = m_bShieldStatus[iChannelIndex];
                    m_bOldProhibitStatus[iChannelIndex] = m_bProhibitStatus[iChannelIndex];
                    SetLampClrByRing(nRingNum, jPhaseIndex, m_tFixTimeCtlInfo.m_atPhaseSeq[nRingNum].m_atPhaseInfo[jPhaseIndex].m_chPhaseStage, m_tFixTimeCtlInfo.m_atPhaseSeq[nRingNum].m_atPhaseInfo[jPhaseIndex].m_chPedPhaseStage);
                    m_bIsLampClrChg = true;
                }
            }
        }
    }
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
bool CLogicCtlFixedTime::IsNeglectPhase(int nPhaseID)
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
������ ��IsConcurrencyPhase
���� ���жϣ���Ӧ����λ�Ƿ񲢷�
�㷨ʵ�� ��
����˵�� ��
����ֵ˵������
----------------------------------------------------------------------
�޸ļ�¼ ��
�� ��          �汾 �޸���  �߶���  �޸ļ�¼

====================================================================*/
bool CLogicCtlFixedTime::IsConcurrencyPhase(int nRingIndex, int nDstRingIndex)
{
    bool bConcurrency = false;
    int dstPhase = 0;
    int srcPhaseIndex = 0;
    int dstPhaseIndex = 0;

    srcPhaseIndex = m_tFixTimeCtlInfo.m_nCurPhaseIndex[nRingIndex] + 1;
    dstPhaseIndex = m_tFixTimeCtlInfo.m_nCurPhaseIndex[nDstRingIndex];
    dstPhase = m_tFixTimeCtlInfo.m_atPhaseSeq[nDstRingIndex].m_atPhaseInfo[dstPhaseIndex].m_tPhaseParam.m_byPhaseNumber;

    for (int i = 0; i < MAX_PHASE_CONCURRENCY_COUNT; i++)
    {
        if (m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[srcPhaseIndex].m_tPhaseParam.m_byPhaseConcurrency[i] == dstPhase)
        {
            //�ǲ�����λ
            bConcurrency = true;
            break;
        }
    }

    return bConcurrency;
}

/*====================================================================
������ ��SetSeqByStageInfo
���� ������m_bySplitPhaseMode��m_nSplitPhaseTime������������
�㷨ʵ�� ��
����˵�� ��
����ֵ˵������
----------------------------------------------------------------------
�޸ļ�¼ ��
�� ��          �汾 �޸���  �߶���  �޸ļ�¼

====================================================================*/
void CLogicCtlFixedTime::SetSeqByStageInfo()
{
    bool isChage = false;
    TRingCtlInfo atPhaseSeq[MAX_RING_COUNT];
    memset(atPhaseSeq, 0, sizeof(atPhaseSeq));

    for (int iRingIndex = 0; iRingIndex < m_tFixTimeCtlInfo.m_nRingCount; iRingIndex++)
    {
        for (int iPhaseIndex = 0; iPhaseIndex < m_tFixTimeCtlInfo.m_atPhaseSeq[iRingIndex].m_nPhaseCount; iPhaseIndex++)
        {
            if (m_bySplitPhaseMode[iRingIndex][iPhaseIndex] == 0 && m_tFixTimeCtlInfo.m_atPhaseSeq[iRingIndex].m_atPhaseInfo[iPhaseIndex].m_chPhaseMode != NEGLECT_MODE && m_tFixTimeCtlInfo.m_atPhaseSeq[iRingIndex].m_atPhaseInfo[iPhaseIndex].m_chPhaseMode != SHIELD_MODE)   //������λ
            {
                memcpy(&atPhaseSeq[iRingIndex].m_atPhaseInfo[atPhaseSeq[iRingIndex].m_nPhaseCount], &m_tFixTimeCtlInfo.m_atPhaseSeq[iRingIndex].m_atPhaseInfo[iPhaseIndex], sizeof(m_tFixTimeCtlInfo.m_atPhaseSeq[iRingIndex].m_atPhaseInfo[iPhaseIndex]));
                atPhaseSeq[iRingIndex].m_nPhaseCount += 1;
            }
            else if (m_bySplitPhaseMode[iRingIndex][iPhaseIndex] == 0 && (m_tFixTimeCtlInfo.m_atPhaseSeq[iRingIndex].m_atPhaseInfo[iPhaseIndex].m_chPhaseMode == NEGLECT_MODE || m_tFixTimeCtlInfo.m_atPhaseSeq[iRingIndex].m_atPhaseInfo[iPhaseIndex].m_chPhaseMode == SHIELD_MODE))
            {
                memcpy(&atPhaseSeq[iRingIndex].m_atPhaseInfo[atPhaseSeq[iRingIndex].m_nPhaseCount], &m_tFixTimeCtlInfo.m_atPhaseSeq[iRingIndex].m_atPhaseInfo[iPhaseIndex], sizeof(m_tFixTimeCtlInfo.m_atPhaseSeq[iRingIndex].m_atPhaseInfo[iPhaseIndex]));
                atPhaseSeq[iRingIndex].m_atPhaseInfo[atPhaseSeq[iRingIndex].m_nPhaseCount].m_wPhaseGreenTime = 0;
                atPhaseSeq[iRingIndex].m_atPhaseInfo[atPhaseSeq[iRingIndex].m_nPhaseCount].m_wPedPhaseGreenTime = 0;
                atPhaseSeq[iRingIndex].m_atPhaseInfo[atPhaseSeq[iRingIndex].m_nPhaseCount].m_tPhaseParam.m_byGreenFlash = 0;
                atPhaseSeq[iRingIndex].m_atPhaseInfo[atPhaseSeq[iRingIndex].m_nPhaseCount].m_tPhaseParam.m_byPhasePedestrianClear = 0;
                atPhaseSeq[iRingIndex].m_atPhaseInfo[atPhaseSeq[iRingIndex].m_nPhaseCount].m_tPhaseParam.m_byPhaseYellowChange = 0;
                atPhaseSeq[iRingIndex].m_atPhaseInfo[atPhaseSeq[iRingIndex].m_nPhaseCount].m_tPhaseParam.m_byPhaseRedClear = m_tFixTimeCtlInfo.m_atPhaseSeq[iRingIndex].m_atPhaseInfo[iPhaseIndex].m_wPhaseTime;
                atPhaseSeq[iRingIndex].m_atPhaseInfo[atPhaseSeq[iRingIndex].m_nPhaseCount].m_tPhaseParam.m_wPhaseMinimumGreen = 0;
                atPhaseSeq[iRingIndex].m_atPhaseInfo[atPhaseSeq[iRingIndex].m_nPhaseCount].m_tPhaseParam.m_wPhaseMaximum1 = 0;
                atPhaseSeq[iRingIndex].m_atPhaseInfo[atPhaseSeq[iRingIndex].m_nPhaseCount].m_tPhaseParam.m_wPhaseMaximum2 = 0;
                atPhaseSeq[iRingIndex].m_nPhaseCount += 1;
                isChage = true;
            }
            else if (m_bySplitPhaseMode[iRingIndex][iPhaseIndex] == 2)
            {
                memcpy(&atPhaseSeq[iRingIndex].m_atPhaseInfo[atPhaseSeq[iRingIndex].m_nPhaseCount], &m_tFixTimeCtlInfo.m_atPhaseSeq[iRingIndex].m_atPhaseInfo[iPhaseIndex], sizeof(m_tFixTimeCtlInfo.m_atPhaseSeq[iRingIndex].m_atPhaseInfo[iPhaseIndex]));
                atPhaseSeq[iRingIndex].m_atPhaseInfo[atPhaseSeq[iRingIndex].m_nPhaseCount].m_wPhaseTime = m_tFixTimeCtlInfo.m_atPhaseSeq[iRingIndex].m_atPhaseInfo[iPhaseIndex].m_wPhaseTime - m_nSplitPhaseTime[iRingIndex][iPhaseIndex];
                atPhaseSeq[iRingIndex].m_atPhaseInfo[atPhaseSeq[iRingIndex].m_nPhaseCount].m_wPhaseGreenTime = 0;
                atPhaseSeq[iRingIndex].m_atPhaseInfo[atPhaseSeq[iRingIndex].m_nPhaseCount].m_wPedPhaseGreenTime = 0;
                atPhaseSeq[iRingIndex].m_atPhaseInfo[atPhaseSeq[iRingIndex].m_nPhaseCount].m_tPhaseParam.m_byGreenFlash = 0;
                atPhaseSeq[iRingIndex].m_atPhaseInfo[atPhaseSeq[iRingIndex].m_nPhaseCount].m_tPhaseParam.m_byPhasePedestrianClear = 0;
                atPhaseSeq[iRingIndex].m_atPhaseInfo[atPhaseSeq[iRingIndex].m_nPhaseCount].m_tPhaseParam.m_byPhaseYellowChange = 0;
                atPhaseSeq[iRingIndex].m_atPhaseInfo[atPhaseSeq[iRingIndex].m_nPhaseCount].m_tPhaseParam.m_byPhaseRedClear = m_tFixTimeCtlInfo.m_atPhaseSeq[iRingIndex].m_atPhaseInfo[iPhaseIndex].m_wPhaseTime - m_nSplitPhaseTime[iRingIndex][iPhaseIndex];
                atPhaseSeq[iRingIndex].m_atPhaseInfo[atPhaseSeq[iRingIndex].m_nPhaseCount].m_tPhaseParam.m_wPhaseMinimumGreen = 0;
                atPhaseSeq[iRingIndex].m_atPhaseInfo[atPhaseSeq[iRingIndex].m_nPhaseCount].m_tPhaseParam.m_wPhaseMaximum1 = 0;
                atPhaseSeq[iRingIndex].m_atPhaseInfo[atPhaseSeq[iRingIndex].m_nPhaseCount].m_tPhaseParam.m_wPhaseMaximum2 = 0;
                atPhaseSeq[iRingIndex].m_nPhaseCount += 1;
                isChage = true;
            }
            else
            {
                isChage = true;
            }
        }
    }

    if (isChage)
    {
        memcpy(m_tFixTimeCtlInfo.m_atPhaseSeq, &atPhaseSeq, sizeof(m_tFixTimeCtlInfo.m_atPhaseSeq));
    }
}

/*====================================================================
������ ��InitPhaseConcurrencyTable
���� ���ж�������λ�Ƿ񲢷�
�㷨ʵ�� ��
����˵�� ��
����ֵ˵������
----------------------------------------------------------------------
�޸ļ�¼ ��
�� ��          �汾 �޸���  �߶���  �޸ļ�¼

====================================================================*/
void CLogicCtlFixedTime::InitPhaseConcurrencyTable()
{
    int iPhaseNum = 0;
    int jPhaseNum = 0;
    TPhase tPhaseTable[MAX_PHASE_COUNT];
    memset(m_nPhaseConcurInfo, 0x00, sizeof(m_nPhaseConcurInfo));
    m_pOpenATCParameter->GetPhaseTable(tPhaseTable);
    for (int iPhaseIndex = 0; iPhaseIndex < MAX_PHASE_COUNT; iPhaseIndex++)
    {
        iPhaseNum = tPhaseTable[iPhaseIndex].m_byPhaseNumber;
        if (iPhaseNum == 0)
        {
            continue;
        }
        m_nPhaseConcurInfo[iPhaseNum - 1][iPhaseNum - 1] = 1;
        for (int jPhaseIndex = 0; jPhaseIndex < MAX_PHASE_CONCURRENCY_COUNT; jPhaseIndex++)
        {
            jPhaseNum = tPhaseTable[iPhaseIndex].m_byPhaseConcurrency[jPhaseIndex];
            if (jPhaseNum == 0)
            {
                continue;
            }
            m_nPhaseConcurInfo[iPhaseNum - 1][jPhaseNum - 1] = 1;
            m_nPhaseConcurInfo[jPhaseNum - 1][iPhaseNum - 1] = 1;
        }
    }
}

/*====================================================================
������ ��SetOverlapPhaseLampClr
���� ������ʱ�����ø�����λ��ɫ
�㷨ʵ�� ��
����˵�� ��
����ֵ˵������
----------------------------------------------------------------------
�޸ļ�¼ ��
�� ��          �汾 �޸���  �߶���  �޸ļ�¼

====================================================================*/
void CLogicCtlFixedTime::SetOverlapPhaseLampClr()
{
    //ǰ�᣺������λ������ʵʱ·�ڳ��������ϱ仯���ᵼ��ʵ�����н׶�Ҳ��֮���ϵı仯������ԭ��������ʱ�׶α�ĸ�����λ��ɫ�ж��޷������Ӧ���Ƶĸ�����λ
    //���������õĸ�����λ���е�ɫ�ж�
    for (int i = 0; i < m_tFixTimeCtlInfo.m_nOverlapCount; i++)
    {
        //��ȡ������λ��ɫ
        BYTE byOverlapNum = m_tFixTimeCtlInfo.m_atOverlapInfo[i].m_tOverlapParam.m_byOverlapNumber;
        if (byOverlapNum == 0)
        {
            continue;
        }

        int  nRingIndex = 0;
        int  nPhaseIndex = 0;
        int nCurPhaseIndex = 0;
        int nNextPhaseIndex = 0;
        char chRet = 0;
        char chTempRet = 0;
        char chPedRet = 0;

        int nBarrierPhaseNum[MAX_RING_COUNT];
        int nBarrierIndex = 0;
        int iRingIndex = 0;
        int jRingIndex = 0;
        int j = 0;

        bool bGreenFlag = false;
        bool bGreenFlashFlag = false;
        bool bYellowFlag = false;
        bool bPedGreenFlag = false;
        bool bPedGreenFlashFlag = false;
        bool bPedYellowFlag = false;

        //��ÿ��������λ��ĸ��λ���д���
        //�ȴ�������������λ
        for (j = 0; j < MAX_PHASE_COUNT_IN_OVERLAP; j++)
        {
            BYTE byMainPhaseNum = m_tFixTimeCtlInfo.m_atOverlapInfo[i].m_tOverlapParam.m_byArrOverlapIncludedPhases[j];
            if (byMainPhaseNum > 0)
            {
                //�ж�ĸ��λ�Ƿ�Ϊ���Ի��߹ض���λ�����Ϊ���Ժ͹ض���λ���������λ�����к����жϴ���
                bool bIsNeglectPhase = IsNeglectPhase(byMainPhaseNum);
                GetPhaseIndexByPhaseNumber(nRingIndex, nPhaseIndex, (int)byMainPhaseNum);   //��ȡ��ĸ��λ�Ļ����������
                chRet = GetPhaseStatus(byMainPhaseNum, false);                              //��ȡ��ĸ��λ�ĵ�ɫ

                for (iRingIndex = 0; iRingIndex < m_tFixTimeCtlInfo.m_nRingCount; iRingIndex++)
                {
                    PTRingCtlInfo ptRingRunInfo = &(m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex]);
                    nCurPhaseIndex = m_tFixTimeCtlInfo.m_nCurPhaseIndex[nRingIndex];    //��ȡ��ǰ��λ����
                    if (nRingIndex == iRingIndex && nCurPhaseIndex == nPhaseIndex && !bIsNeglectPhase)
                    {
                        //ĸ��λΪ����������λ������Ϊ�̵ƻ��������λΪ�̵�
                        if (chRet == C_CH_PHASESTAGE_G || (chRet == C_CH_PHASESTAGE_U && m_tFixTimeCtlInfo.m_atOverlapInfo[i].m_chOverlapStage == C_CH_PHASESTAGE_G))
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

                //����������λ��ɫΪ�̵ƣ�����ĸ��λ��ǰ��ɫ��Ϊ��
                if (m_tFixTimeCtlInfo.m_atOverlapInfo[i].m_chOverlapStage == C_CH_PHASESTAGE_G && m_nCurRunMode != CTL_MODE_ACTUATE)
                {
                    //��ÿ�����������ж�
                    for (iRingIndex = 0; iRingIndex < m_tFixTimeCtlInfo.m_nRingCount; iRingIndex++)
                    {
                        PTRingCtlInfo ptRingRunInfo = &(m_tFixTimeCtlInfo.m_atPhaseSeq[iRingIndex]);
                        nCurPhaseIndex = m_tFixTimeCtlInfo.m_nCurPhaseIndex[iRingIndex];    //��ȡ��ǰ��λ����

                        //�жϻ�������ڹ��ɵ�ɫ������һ���������λ�Ƿ�Ϊ��ĸ��λ
                        chTempRet = GetPhaseStatus(ptRingRunInfo->m_atPhaseInfo[nCurPhaseIndex].m_tPhaseParam.m_byPhaseNumber, true);
                        //��ÿ���������ж�
                        if (chTempRet != C_CH_PHASESTAGE_G && chTempRet != C_CH_PHASESTAGE_U)
                        {
                            //�ж��Ƿ����Ϻ�����һ����λ
                            //���Ϊ���Ϻ�����һ����λ
                            memset(nBarrierPhaseNum, 0x00, sizeof(nBarrierPhaseNum));
                            nBarrierIndex = 0;
                            if (m_bLastPhaseBeforeBarrier[iRingIndex][nCurPhaseIndex])
                            {
                                //�����ǵڼ�������
                                for (int i = 0; i < nCurPhaseIndex + 1; i++)
                                {
                                    if (m_bLastPhaseBeforeBarrier[iRingIndex][i])
                                    {
                                        nBarrierIndex += 1;
                                    }
                                }

                                //��ȡÿ������λ��Ӧ������ǰ������
                                for (jRingIndex = 0; jRingIndex < m_tFixTimeCtlInfo.m_nRingCount; jRingIndex++)
                                {
                                    int nBarrierNum = 0;
                                    for (int jPhaseIndex = 0; jPhaseIndex < m_tFixTimeCtlInfo.m_atPhaseSeq[jRingIndex].m_nPhaseCount; jPhaseIndex++)
                                    {
                                        if (m_bLastPhaseBeforeBarrier[jRingIndex][jPhaseIndex])
                                        {
                                            nBarrierNum += 1;
                                            if (nBarrierNum == nBarrierIndex)
                                            {
                                                nBarrierPhaseNum[jRingIndex] = jPhaseIndex;
                                                break;
                                            }
                                        }
                                    }

                                    //�������Ϻ����λ�����ж�
                                    nNextPhaseIndex = nBarrierPhaseNum[jRingIndex];
                                    nNextPhaseIndex += 1;
                                    if (nNextPhaseIndex >= m_tFixTimeCtlInfo.m_atPhaseSeq[jRingIndex].m_nPhaseCount)
                                    {
                                        nNextPhaseIndex = 0;
                                    }

                                    if (m_tFixTimeCtlInfo.m_atPhaseSeq[jRingIndex].m_atPhaseInfo[nNextPhaseIndex].m_tPhaseParam.m_byPhaseNumber == byMainPhaseNum && m_tFixTimeCtlInfo.m_atPhaseSeq[jRingIndex].m_atPhaseInfo[nBarrierPhaseNum[jRingIndex]].m_tPhaseParam.m_byPhaseNumber != byMainPhaseNum && !bIsNeglectPhase)
                                    {
                                        bGreenFlag = true;
                                        break;
                                    }
                                }
                            }
                            else
                            {
                                //��ȡ��һ���Ǻ��Ժ͹ضϵ�������λ
                                nNextPhaseIndex = nCurPhaseIndex;
                                nNextPhaseIndex += 1;
                                if (nNextPhaseIndex >= ptRingRunInfo->m_nPhaseCount)
                                {
                                    nNextPhaseIndex = 0;
                                }
                                if (ptRingRunInfo->m_atPhaseInfo[nNextPhaseIndex].m_tPhaseParam.m_byPhaseNumber == byMainPhaseNum && ptRingRunInfo->m_atPhaseInfo[nCurPhaseIndex].m_tPhaseParam.m_byPhaseNumber != byMainPhaseNum && !bIsNeglectPhase)
                                {
                                    bGreenFlag = true;
                                    break;
                                }
                            }
                        }
                    }

                    if (bGreenFlag)
                    {
                        //�û�����ʱ���˷��������ϵͳ����ʱ�·���ͨ������ָ��������λ��ʹ������һ���׶ε���λ��Ҳ��Ҫ�͵�ǰ�׶ε���λһ����ɵ�ɫ
                        if (m_bIsUsrCtl || m_bIsSystemCtl)
                        {
                            TManualCmd  tValidManualCmd;
                            memset(&tValidManualCmd, 0, sizeof(tValidManualCmd));
                            m_pOpenATCRunStatus->GetValidManualCmd(tValidManualCmd);
                            if (tValidManualCmd.m_bDirectionCmd || tValidManualCmd.m_bChannelLockCmd)
                            {
                                bGreenFlag = false;
                            }
                        }
                    }

                    if (bGreenFlag)
                    {
                        break;
                    }
                }
                else if (m_tFixTimeCtlInfo.m_atOverlapInfo[i].m_chPedOverlapStage == C_CH_PHASESTAGE_G && m_nCurRunMode == CTL_MODE_ACTUATE)
                {
                    //����Ǹ�Ӧ���ƣ��򵥶��������λ������
                    for (iRingIndex = 0; iRingIndex < m_tFixTimeCtlInfo.m_nRingCount; iRingIndex++)
                    {
                        PTRingCtlInfo ptRingRunInfo = &(m_tFixTimeCtlInfo.m_atPhaseSeq[iRingIndex]);
                        nCurPhaseIndex = m_tFixTimeCtlInfo.m_nCurPhaseIndex[iRingIndex];    //��ȡ��ǰ��λ����

                        //�жϻ�������ڹ��ɵ�ɫ������һ���������λ�Ƿ�Ϊ��ĸ��λ
                        chTempRet = GetPhaseStatus(ptRingRunInfo->m_atPhaseInfo[nCurPhaseIndex].m_tPhaseParam.m_byPhaseNumber, false);
                        //��ÿ���������ж�
                        if (chTempRet != C_CH_PHASESTAGE_G && chTempRet != C_CH_PHASESTAGE_U)
                        {
                            //�ж��Ƿ����Ϻ�����һ����λ
                            //���Ϊ���Ϻ�����һ����λ
                            memset(nBarrierPhaseNum, 0x00, sizeof(nBarrierPhaseNum));
                            nBarrierIndex = 0;
                            if (m_bLastPhaseBeforeBarrier[iRingIndex][nCurPhaseIndex])
                            {
                                //�����ǵڼ�������
                                for (int i = 0; i < nCurPhaseIndex + 1; i++)
                                {
                                    if (m_bLastPhaseBeforeBarrier[iRingIndex][i])
                                    {
                                        nBarrierIndex += 1;
                                    }
                                }

                                //��ȡÿ������λ��Ӧ������ǰ������
                                for (jRingIndex = 0; jRingIndex < m_tFixTimeCtlInfo.m_nRingCount; jRingIndex++)
                                {
                                    int nBarrierNum = 0;
                                    for (int jPhaseIndex = 0; jPhaseIndex < m_tFixTimeCtlInfo.m_atPhaseSeq[jRingIndex].m_nPhaseCount; jPhaseIndex++)
                                    {
                                        if (m_bLastPhaseBeforeBarrier[jRingIndex][jPhaseIndex])
                                        {
                                            nBarrierNum += 1;
                                            if (nBarrierNum == nBarrierIndex)
                                            {
                                                nBarrierPhaseNum[jRingIndex] = jPhaseIndex;
                                                break;
                                            }
                                        }
                                    }

                                    //�������Ϻ����λ�����ж�
                                    nNextPhaseIndex = nBarrierPhaseNum[jRingIndex];

                                    for (int iPhaseCount = 0; iPhaseCount < m_tFixTimeCtlInfo.m_atPhaseSeq[jRingIndex].m_nPhaseCount; iPhaseCount++)
                                    {
                                        nNextPhaseIndex += 1;
                                        if (nNextPhaseIndex >= m_tFixTimeCtlInfo.m_atPhaseSeq[jRingIndex].m_nPhaseCount)
                                        {
                                            nNextPhaseIndex = 0;
                                        }

                                        //��������ֱ���˳�
                                        if (m_bLastPhaseBeforeBarrier[jRingIndex][nNextPhaseIndex] || !IsNeglectPhase(m_tFixTimeCtlInfo.m_atPhaseSeq[jRingIndex].m_atPhaseInfo[nNextPhaseIndex].m_tPhaseParam.m_byPhaseNumber))
                                        {
                                            break;
                                        }
                                    }

                                    if (m_tFixTimeCtlInfo.m_atPhaseSeq[jRingIndex].m_atPhaseInfo[nNextPhaseIndex].m_tPhaseParam.m_byPhaseNumber == byMainPhaseNum && m_tFixTimeCtlInfo.m_atPhaseSeq[jRingIndex].m_atPhaseInfo[nBarrierPhaseNum[jRingIndex]].m_tPhaseParam.m_byPhaseNumber != byMainPhaseNum && !bIsNeglectPhase)
                                    {
                                        bGreenFlag = true;
                                        break;
                                    }
                                }
                            }
                            else
                            {
                                //��ȡ��һ���Ǻ��Ժ͹ضϵ�������λ[��Ҫ�������ϵ��ж�]
                                nNextPhaseIndex = nCurPhaseIndex;
                                for (int iPhaseCount = 0; iPhaseCount < ptRingRunInfo->m_nPhaseCount; iPhaseCount++)
                                {
                                    nNextPhaseIndex += 1;
                                    if (nNextPhaseIndex >= ptRingRunInfo->m_nPhaseCount)
                                    {
                                        nNextPhaseIndex = 0;
                                    }

                                    //��������ֱ���˳�[����һ�ο����ϵ����]
                                    if (m_bLastPhaseBeforeBarrier[iRingIndex][nNextPhaseIndex] || !IsNeglectPhase(ptRingRunInfo->m_atPhaseInfo[nNextPhaseIndex].m_tPhaseParam.m_byPhaseNumber))
                                    {
                                        break;
                                    }
                                }

                                //�����������Ͻ������Ҹ�������������Ϊ���ԣ���������ǰ���һ��������
                                if (m_bLastPhaseBeforeBarrier[iRingIndex][nNextPhaseIndex] && IsNeglectPhase(ptRingRunInfo->m_atPhaseInfo[nNextPhaseIndex].m_tPhaseParam.m_byPhaseNumber))
                                {
                                    //�����ǵڼ�������
                                    for (int i = 0; i < nNextPhaseIndex + 1; i++)
                                    {
                                        if (m_bLastPhaseBeforeBarrier[iRingIndex][i])
                                        {
                                            nBarrierIndex += 1;
                                        }
                                    }

                                    //��ȡÿ������λ��Ӧ������ǰ������
                                    for (jRingIndex = 0; jRingIndex < m_tFixTimeCtlInfo.m_nRingCount; jRingIndex++)
                                    {
                                        int nBarrierNum = 0;
                                        for (int jPhaseIndex = 0; jPhaseIndex < m_tFixTimeCtlInfo.m_atPhaseSeq[jRingIndex].m_nPhaseCount; jPhaseIndex++)
                                        {
                                            if (m_bLastPhaseBeforeBarrier[jRingIndex][jPhaseIndex])
                                            {
                                                nBarrierNum += 1;
                                                if (nBarrierNum == nBarrierIndex)
                                                {
                                                    nBarrierPhaseNum[jRingIndex] = jPhaseIndex;
                                                    break;
                                                }
                                            }
                                        }

                                        //�������Ϻ����λ�����ж�
                                        int nTempNextPhaseIndex = nBarrierPhaseNum[jRingIndex];

                                        for (int iPhaseCount = 0; iPhaseCount < m_tFixTimeCtlInfo.m_atPhaseSeq[jRingIndex].m_nPhaseCount; iPhaseCount++)
                                        {
                                            nTempNextPhaseIndex += 1;
                                            if (nTempNextPhaseIndex >= m_tFixTimeCtlInfo.m_atPhaseSeq[jRingIndex].m_nPhaseCount)
                                            {
                                                nTempNextPhaseIndex = 0;
                                            }

                                            //��������ֱ���˳�
                                            if (m_bLastPhaseBeforeBarrier[jRingIndex][nTempNextPhaseIndex] || !IsNeglectPhase(m_tFixTimeCtlInfo.m_atPhaseSeq[jRingIndex].m_atPhaseInfo[nTempNextPhaseIndex].m_tPhaseParam.m_byPhaseNumber))
                                            {
                                                break;
                                            }
                                        }

                                        if (m_tFixTimeCtlInfo.m_atPhaseSeq[jRingIndex].m_atPhaseInfo[nTempNextPhaseIndex].m_tPhaseParam.m_byPhaseNumber == byMainPhaseNum && m_tFixTimeCtlInfo.m_atPhaseSeq[jRingIndex].m_atPhaseInfo[nBarrierPhaseNum[jRingIndex]].m_tPhaseParam.m_byPhaseNumber != byMainPhaseNum && !bIsNeglectPhase)
                                        {
                                            bGreenFlag = true;
                                            break;
                                        }
                                    }
                                }
                                else
                                {
                                    if (ptRingRunInfo->m_atPhaseInfo[nNextPhaseIndex].m_tPhaseParam.m_byPhaseNumber == byMainPhaseNum && ptRingRunInfo->m_atPhaseInfo[nCurPhaseIndex].m_tPhaseParam.m_byPhaseNumber != byMainPhaseNum && !bIsNeglectPhase)
                                    {
                                        bGreenFlag = true;
                                        break;
                                    }
                                }
                            }
                        }
                    }

                    if (bGreenFlag)
                    {
                        //�û�����ʱ���˷��������ϵͳ����ʱ�·���ͨ������ָ��������λ��ʹ������һ���׶ε���λ��Ҳ��Ҫ�͵�ǰ�׶ε���λһ����ɵ�ɫ
                        if (m_bIsUsrCtl || m_bIsSystemCtl)
                        {
                            TManualCmd  tValidManualCmd;
                            memset(&tValidManualCmd, 0, sizeof(tValidManualCmd));
                            m_pOpenATCRunStatus->GetValidManualCmd(tValidManualCmd);
                            if (tValidManualCmd.m_bDirectionCmd || tValidManualCmd.m_bChannelLockCmd)
                            {
                                bGreenFlag = false;
                            }
                        }
                    }

                    if (bGreenFlag)
                    {
                        break;
                    }
                }

                //��������ж϶�û�øø�����λ�����̵ƣ�����е�ɫ�仯
                if (chRet == C_CH_PHASESTAGE_GF && !bIsNeglectPhase)
                {
                    bGreenFlashFlag = true;

                    for (int k = 0; k < m_nChannelCount; k++)
                    {
                        if (m_atChannelInfo[k].m_byChannelNumber == 0)
                        {
                            continue;
                        }

                        if ((int)m_atChannelInfo[k].m_byChannelControlSource == byOverlapNum)
                        {
                            if (m_atChannelInfo[k].m_byChannelControlType == OVERLAP_CHA)
                            {
                                m_pOpenATCRunStatus->SetGreenFlashCount((m_atChannelInfo[k].m_byChannelNumber - 1) * 3 + 2, m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_wPhaseStageRunTime * 2);
                            }
                        }
                    }
                }
                if (chRet == C_CH_PHASESTAGE_Y && !bIsNeglectPhase)
                {
                    bYellowFlag = true;
                }
            }
        }

        //�������������λ
        for (j = 0; j < MAX_PHASE_COUNT_IN_OVERLAP; j++)
        {
            BYTE byMainPhaseNum = m_tFixTimeCtlInfo.m_atOverlapInfo[i].m_tOverlapParam.m_byArrOverlapIncludedPhases[j];
            if (byMainPhaseNum > 0)
            {
                bool bIsNeglectPhase = IsNeglectPhase(byMainPhaseNum);
                GetPhaseIndexByPhaseNumber(nRingIndex, nPhaseIndex, (int)byMainPhaseNum);
                chPedRet = GetPhaseStatus(byMainPhaseNum, true);

                for (iRingIndex = 0; iRingIndex < m_tFixTimeCtlInfo.m_nRingCount; iRingIndex++)
                {
                    PTRingCtlInfo ptRingRunInfo = &(m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex]);
                    nCurPhaseIndex = m_tFixTimeCtlInfo.m_nCurPhaseIndex[nRingIndex];    //��ȡ��ǰ��λ����
                    if (nRingIndex == iRingIndex && nCurPhaseIndex == nPhaseIndex && !bIsNeglectPhase)
                    {
                        //ĸ��λΪ����������λ������Ϊ�̵ƻ��������λΪ�̵�
                        if (chPedRet == C_CH_PHASESTAGE_G || (chPedRet == C_CH_PHASESTAGE_U && m_tFixTimeCtlInfo.m_atOverlapInfo[i].m_chPedOverlapStage == C_CH_PHASESTAGE_G))
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

                //����������λ��ɫΪ�̵ƣ�����ĸ��λ��ǰ��ɫ��Ϊ��
                //�����ǰ�л���û�н�������nNextPhaseIndex��Ӧ����λ��δ��������λ����������δ����������Ҳ��Ҫ�����¸���������жϡ�
                if (m_tFixTimeCtlInfo.m_atOverlapInfo[i].m_chPedOverlapStage == C_CH_PHASESTAGE_G && m_nCurRunMode != CTL_MODE_ACTUATE)
                {
                    //��ÿ�����������ж�
                    for (iRingIndex = 0; iRingIndex < m_tFixTimeCtlInfo.m_nRingCount; iRingIndex++)
                    {
                        PTRingCtlInfo ptRingRunInfo = &(m_tFixTimeCtlInfo.m_atPhaseSeq[iRingIndex]);
                        nCurPhaseIndex = m_tFixTimeCtlInfo.m_nCurPhaseIndex[iRingIndex];    //��ȡ��ǰ��λ����

                        //�жϻ�������ڹ��ɵ�ɫ������һ���������λ�Ƿ�Ϊ��ĸ��λ
                        chTempRet = GetPhaseStatus(ptRingRunInfo->m_atPhaseInfo[nCurPhaseIndex].m_tPhaseParam.m_byPhaseNumber, true);
                        //��ÿ���������ж�
                        if (chTempRet != C_CH_PHASESTAGE_G && chTempRet != C_CH_PHASESTAGE_U)
                        {
                            //�ж��Ƿ����Ϻ�����һ����λ
                            //���Ϊ���Ϻ�����һ����λ
                            memset(nBarrierPhaseNum, 0x00, sizeof(nBarrierPhaseNum));
                            nBarrierIndex = 0;
                            if (m_bLastPhaseBeforeBarrier[iRingIndex][nCurPhaseIndex])
                            {
                                //�����ǵڼ�������
                                for (int i = 0; i < nCurPhaseIndex + 1; i++)
                                {
                                    if (m_bLastPhaseBeforeBarrier[iRingIndex][i])
                                    {
                                        nBarrierIndex += 1;
                                    }
                                }

                                //��ȡÿ������λ��Ӧ������ǰ������
                                for (jRingIndex = 0; jRingIndex < m_tFixTimeCtlInfo.m_nRingCount; jRingIndex++)
                                {
                                    int nBarrierNum = 0;
                                    for (int jPhaseIndex = 0; jPhaseIndex < m_tFixTimeCtlInfo.m_atPhaseSeq[jRingIndex].m_nPhaseCount; jPhaseIndex++)
                                    {
                                        if (m_bLastPhaseBeforeBarrier[jRingIndex][jPhaseIndex])
                                        {
                                            nBarrierNum += 1;
                                            if (nBarrierNum == nBarrierIndex)
                                            {
                                                nBarrierPhaseNum[jRingIndex] = jPhaseIndex;
                                                break;
                                            }
                                        }
                                    }

                                    //�������Ϻ����λ�����ж�
                                    nNextPhaseIndex = nBarrierPhaseNum[jRingIndex];
                                    nNextPhaseIndex += 1;
                                    if (nNextPhaseIndex >= m_tFixTimeCtlInfo.m_atPhaseSeq[jRingIndex].m_nPhaseCount)
                                    {
                                        nNextPhaseIndex = 0;
                                    }

                                    if (m_tFixTimeCtlInfo.m_atPhaseSeq[jRingIndex].m_atPhaseInfo[nNextPhaseIndex].m_tPhaseParam.m_byPhaseNumber == byMainPhaseNum && m_tFixTimeCtlInfo.m_atPhaseSeq[jRingIndex].m_atPhaseInfo[nBarrierPhaseNum[jRingIndex]].m_tPhaseParam.m_byPhaseNumber != byMainPhaseNum && !bIsNeglectPhase)
                                    {
                                        bPedGreenFlag = true;
                                        break;
                                    }
                                }
                            }
                            else
                            {
                                //��ȡ��һ���Ǻ��Ժ͹ضϵ�������λ
                                nNextPhaseIndex = nCurPhaseIndex;
                                nNextPhaseIndex += 1;
                                if (nNextPhaseIndex >= ptRingRunInfo->m_nPhaseCount)
                                {
                                    nNextPhaseIndex = 0;
                                }
                                if (ptRingRunInfo->m_atPhaseInfo[nNextPhaseIndex].m_tPhaseParam.m_byPhaseNumber == byMainPhaseNum && ptRingRunInfo->m_atPhaseInfo[nCurPhaseIndex].m_tPhaseParam.m_byPhaseNumber != byMainPhaseNum && !bIsNeglectPhase)
                                {
                                    bPedGreenFlag = true;
                                    break;
                                }
                            }
                        }
                    }

                    if (bPedGreenFlag)
                    {
                        //�û�����ʱ���˷��������ϵͳ����ʱ�·���ͨ������ָ��������λ��ʹ������һ���׶ε���λ��Ҳ��Ҫ�͵�ǰ�׶ε���λһ����ɵ�ɫ
                        if (m_bIsUsrCtl || m_bIsSystemCtl)
                        {
                            TManualCmd  tValidManualCmd;
                            memset(&tValidManualCmd, 0, sizeof(tValidManualCmd));
                            m_pOpenATCRunStatus->GetValidManualCmd(tValidManualCmd);
                            if (tValidManualCmd.m_bDirectionCmd || tValidManualCmd.m_bChannelLockCmd)
                            {
                                bPedGreenFlag = false;
                            }
                        }
                    }

                    if (bPedGreenFlag)
                    {
                        break;
                    }
                }
                else if (m_tFixTimeCtlInfo.m_atOverlapInfo[i].m_chPedOverlapStage == C_CH_PHASESTAGE_G && m_nCurRunMode == CTL_MODE_ACTUATE)
                {
                    //����Ǹ�Ӧ���ƣ��򵥶��������λ������
                    for (iRingIndex = 0; iRingIndex < m_tFixTimeCtlInfo.m_nRingCount; iRingIndex++)
                    {
                        PTRingCtlInfo ptRingRunInfo = &(m_tFixTimeCtlInfo.m_atPhaseSeq[iRingIndex]);
                        nCurPhaseIndex = m_tFixTimeCtlInfo.m_nCurPhaseIndex[iRingIndex];    //��ȡ��ǰ��λ����

                        //�жϻ�������ڹ��ɵ�ɫ������һ���������λ�Ƿ�Ϊ��ĸ��λ
                        chTempRet = GetPhaseStatus(ptRingRunInfo->m_atPhaseInfo[nCurPhaseIndex].m_tPhaseParam.m_byPhaseNumber, true);
                        //��ÿ���������ж�
                        if (chTempRet != C_CH_PHASESTAGE_G && chTempRet != C_CH_PHASESTAGE_U)
                        {
                            //�ж��Ƿ����Ϻ�����һ����λ
                            //���Ϊ���Ϻ�����һ����λ
                            memset(nBarrierPhaseNum, 0x00, sizeof(nBarrierPhaseNum));
                            nBarrierIndex = 0;
                            if (m_bLastPhaseBeforeBarrier[iRingIndex][nCurPhaseIndex])
                            {
                                //�����ǵڼ�������
                                for (int i = 0; i < nCurPhaseIndex + 1; i++)
                                {
                                    if (m_bLastPhaseBeforeBarrier[iRingIndex][i])
                                    {
                                        nBarrierIndex += 1;
                                    }
                                }

                                //��ȡÿ������λ��Ӧ������ǰ������
                                for (jRingIndex = 0; jRingIndex < m_tFixTimeCtlInfo.m_nRingCount; jRingIndex++)
                                {
                                    int nBarrierNum = 0;
                                    for (int jPhaseIndex = 0; jPhaseIndex < m_tFixTimeCtlInfo.m_atPhaseSeq[jRingIndex].m_nPhaseCount; jPhaseIndex++)
                                    {
                                        if (m_bLastPhaseBeforeBarrier[jRingIndex][jPhaseIndex])
                                        {
                                            nBarrierNum += 1;
                                            if (nBarrierNum == nBarrierIndex)
                                            {
                                                nBarrierPhaseNum[jRingIndex] = jPhaseIndex;
                                                break;
                                            }
                                        }
                                    }

                                    //�������Ϻ����λ�����ж�
                                    nNextPhaseIndex = nBarrierPhaseNum[jRingIndex];

                                    for (int iPhaseCount = 0; iPhaseCount < m_tFixTimeCtlInfo.m_atPhaseSeq[jRingIndex].m_nPhaseCount; iPhaseCount++)
                                    {
                                        nNextPhaseIndex += 1;
                                        if (nNextPhaseIndex >= m_tFixTimeCtlInfo.m_atPhaseSeq[jRingIndex].m_nPhaseCount)
                                        {
                                            nNextPhaseIndex = 0;
                                        }

                                        //��������ֱ���˳�
                                        if (m_bLastPhaseBeforeBarrier[jRingIndex][nNextPhaseIndex] || !IsNeglectPhase(m_tFixTimeCtlInfo.m_atPhaseSeq[jRingIndex].m_atPhaseInfo[nNextPhaseIndex].m_tPhaseParam.m_byPhaseNumber))
                                        {
                                            break;
                                        }
                                    }

                                    if (m_tFixTimeCtlInfo.m_atPhaseSeq[jRingIndex].m_atPhaseInfo[nNextPhaseIndex].m_tPhaseParam.m_byPhaseNumber == byMainPhaseNum && m_tFixTimeCtlInfo.m_atPhaseSeq[jRingIndex].m_atPhaseInfo[nBarrierPhaseNum[jRingIndex]].m_tPhaseParam.m_byPhaseNumber != byMainPhaseNum && !bIsNeglectPhase)
                                    {
                                        bPedGreenFlag = true;
                                        break;
                                    }
                                }
                            }
                            else
                            {
                                //��ȡ��һ���Ǻ��Ժ͹ضϵ�������λ[��Ҫ�������ϵ��ж�]
                                nNextPhaseIndex = nCurPhaseIndex;
                                for (int iPhaseCount = 0; iPhaseCount < ptRingRunInfo->m_nPhaseCount; iPhaseCount++)
                                {
                                    nNextPhaseIndex += 1;
                                    if (nNextPhaseIndex >= ptRingRunInfo->m_nPhaseCount)
                                    {
                                        nNextPhaseIndex = 0;
                                    }

                                    //��������ֱ���˳�[����һ�ο����ϵ����]
                                    if (m_bLastPhaseBeforeBarrier[iRingIndex][nNextPhaseIndex] || !IsNeglectPhase(ptRingRunInfo->m_atPhaseInfo[nNextPhaseIndex].m_tPhaseParam.m_byPhaseNumber))
                                    {
                                        break;
                                    }
                                }

                                //�����������Ͻ������Ҹ�������������Ϊ���ԣ���������ǰ���һ��������
                                if (m_bLastPhaseBeforeBarrier[iRingIndex][nNextPhaseIndex] && IsNeglectPhase(ptRingRunInfo->m_atPhaseInfo[nNextPhaseIndex].m_tPhaseParam.m_byPhaseNumber))
                                {
                                    //�����ǵڼ�������
                                    for (int i = 0; i < nNextPhaseIndex + 1; i++)
                                    {
                                        if (m_bLastPhaseBeforeBarrier[iRingIndex][i])
                                        {
                                            nBarrierIndex += 1;
                                        }
                                    }

                                    //��ȡÿ������λ��Ӧ������ǰ������
                                    for (jRingIndex = 0; jRingIndex < m_tFixTimeCtlInfo.m_nRingCount; jRingIndex++)
                                    {
                                        int nBarrierNum = 0;
                                        for (int jPhaseIndex = 0; jPhaseIndex < m_tFixTimeCtlInfo.m_atPhaseSeq[jRingIndex].m_nPhaseCount; jPhaseIndex++)
                                        {
                                            if (m_bLastPhaseBeforeBarrier[jRingIndex][jPhaseIndex])
                                            {
                                                nBarrierNum += 1;
                                                if (nBarrierNum == nBarrierIndex)
                                                {
                                                    nBarrierPhaseNum[jRingIndex] = jPhaseIndex;
                                                    break;
                                                }
                                            }
                                        }

                                        //�������Ϻ����λ�����ж�
                                        int nTempNextPhaseIndex = nBarrierPhaseNum[jRingIndex];

                                        for (int iPhaseCount = 0; iPhaseCount < m_tFixTimeCtlInfo.m_atPhaseSeq[jRingIndex].m_nPhaseCount; iPhaseCount++)
                                        {
                                            nTempNextPhaseIndex += 1;
                                            if (nTempNextPhaseIndex >= m_tFixTimeCtlInfo.m_atPhaseSeq[jRingIndex].m_nPhaseCount)
                                            {
                                                nTempNextPhaseIndex = 0;
                                            }

                                            //��������ֱ���˳�
                                            if (m_bLastPhaseBeforeBarrier[jRingIndex][nTempNextPhaseIndex] || !IsNeglectPhase(m_tFixTimeCtlInfo.m_atPhaseSeq[jRingIndex].m_atPhaseInfo[nTempNextPhaseIndex].m_tPhaseParam.m_byPhaseNumber))
                                            {
                                                break;
                                            }
                                        }

                                        if (m_tFixTimeCtlInfo.m_atPhaseSeq[jRingIndex].m_atPhaseInfo[nTempNextPhaseIndex].m_tPhaseParam.m_byPhaseNumber == byMainPhaseNum && m_tFixTimeCtlInfo.m_atPhaseSeq[jRingIndex].m_atPhaseInfo[nBarrierPhaseNum[jRingIndex]].m_tPhaseParam.m_byPhaseNumber != byMainPhaseNum && !bIsNeglectPhase)
                                        {
                                            bPedGreenFlag = true;
                                            break;
                                        }
                                    }
                                }
                                else
                                {
                                    if (ptRingRunInfo->m_atPhaseInfo[nNextPhaseIndex].m_tPhaseParam.m_byPhaseNumber == byMainPhaseNum && ptRingRunInfo->m_atPhaseInfo[nCurPhaseIndex].m_tPhaseParam.m_byPhaseNumber != byMainPhaseNum && !bIsNeglectPhase)
                                    {
                                        bPedGreenFlag = true;
                                        break;
                                    }
                                }
                            }
                        }
                    }

                    if (bPedGreenFlag)
                    {
                        //�û�����ʱ���˷��������ϵͳ����ʱ�·���ͨ������ָ��������λ��ʹ������һ���׶ε���λ��Ҳ��Ҫ�͵�ǰ�׶ε���λһ����ɵ�ɫ
                        if (m_bIsUsrCtl || m_bIsSystemCtl)
                        {
                            TManualCmd  tValidManualCmd;
                            memset(&tValidManualCmd, 0, sizeof(tValidManualCmd));
                            m_pOpenATCRunStatus->GetValidManualCmd(tValidManualCmd);
                            if (tValidManualCmd.m_bDirectionCmd || tValidManualCmd.m_bChannelLockCmd)
                            {
                                bPedGreenFlag = false;
                            }
                        }
                    }

                    if (bPedGreenFlag)
                    {
                        break;
                    }
                }

                if (chPedRet == C_CH_PHASESTAGE_GF && !bIsNeglectPhase)
                {
                    bPedGreenFlashFlag = true;

                    for (int k = 0; k < m_nChannelCount; k++)
                    {
                        if (m_atChannelInfo[k].m_byChannelNumber == 0)
                        {
                            continue;
                        }

                        if ((int)m_atChannelInfo[k].m_byChannelControlSource == byOverlapNum)
                        {
                            if (m_atChannelInfo[k].m_byChannelControlType == OVERLAP_PED_CHA)
                            {
                                m_pOpenATCRunStatus->SetGreenFlashCount((m_atChannelInfo[k].m_byChannelNumber - 1) * 3 + 2, m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_wPedPhaseStageRunTime * 2);
                            }
                        }
                    }
                }
                if (chPedRet == C_CH_PHASESTAGE_Y && !bIsNeglectPhase)
                {
                    bPedYellowFlag = true;
                }
            }
        }

        //�Ըø�����λ���е�ɫ����
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

        SetChannelStatus((int)byOverlapNum, OVERLAP_SRC, m_tFixTimeCtlInfo.m_atOverlapInfo[i].m_chOverlapStage, m_tFixTimeCtlInfo.m_atOverlapInfo[i].m_chPedOverlapStage);
    }
}