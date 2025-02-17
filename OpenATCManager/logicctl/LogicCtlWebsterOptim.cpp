/*=====================================================================
ģ���� ��webster�����Ż����Ʒ�ʽ�ӿ�ģ��
�ļ��� ��LogicCtlOptim.cpp
����ļ���LogicCtlOptim.h,LogicCtlFixedTime.h
ʵ�ֹ��ܣ������Ż����Ʒ�ʽʵ��
���� ��������
��Ȩ ��<Copyright(c) 2019-2020 Suzhou Keda Technology Co.,Ltd. All right reserved.>
-----------------------------------------------------------------------
�޸ļ�¼��
�� ��           �汾     �޸���     �߶���     �޸ļ�¼
2019/9/14       V1.0     ������     ������     ����ģ��
=====================================================================*/
#include "LogicCtlWebsterOptim.h"
#include <string.h>
#include <math.h>

const float m_fStartUpLostTime = (float)3.0;
const float m_fMinValidPassTime = (float)3.0;
CLogicCtlWebsterOptim::CLogicCtlWebsterOptim()
{

}

CLogicCtlWebsterOptim::~CLogicCtlWebsterOptim()
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
���� ��webster�����Ż����Ʒ�ʽ����Դ��ʼ��
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
void CLogicCtlWebsterOptim::Init(COpenATCParameter * pParameter, COpenATCRunStatus * pRunStatus, COpenATCLog * pOpenATCLog, int nPlanNo)
{
    m_pOpenATCParameter = pParameter;
    m_pOpenATCRunStatus = pRunStatus;
	m_pOpenATCLog       = pOpenATCLog;
    memset(&m_tFixTimeCtlInfo,0,sizeof(m_tFixTimeCtlInfo));
	memset(&m_nPhaseNum, 0, sizeof(m_nPhaseNum));
    memset(&m_tRunStageInfo, 0x00, sizeof(m_tRunStageInfo));
	memset(m_nChannelSplitMode,0x00,sizeof(m_nChannelSplitMode)); 
	memset(&m_bLastPhaseBeforeBarrier, 0x00, sizeof(m_bLastPhaseBeforeBarrier));
	memset(&m_tAscSingleOptimInfo, 0x00, sizeof(m_tAscSingleOptimInfo));
	memset(m_bShieldStatus, 0x00, sizeof(m_bShieldStatus));
	memset(m_bProhibitStatus, 0x00, sizeof(m_bProhibitStatus));
	memset(m_bOldShieldStatus, 0x00, sizeof(m_bOldShieldStatus));
	memset(m_bOldProhibitStatus, 0x00, sizeof(m_bOldProhibitStatus));
	for (int i = 0; i < MAX_RING_COUNT;i++)
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
	m_byPlanID = byPlanID;
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
    tCtlStatus.m_nCurCtlMode = CTL_MODE_SINGLEOPTIM;
    tCtlStatus.m_nCurPlanNo = (int)byPlanID;
    m_pOpenATCRunStatus->SetLogicCtlStatus(tCtlStatus);

    m_bCalcSplitTimeFlag = false;
    memset(&m_tWebsterOptimInfo,0,sizeof(TWebsterOptimInfo));
    memset(m_anDetectorTmpCounter,0,sizeof(m_anDetectorTmpCounter));
    memset(m_byDetectorTmpStatus,0,sizeof(m_byDetectorTmpStatus));

    RetsetAllChannelStatus();

    InitDetectorParam();

    InitWebsterOptimParam();

    GetRunStageTable();

	GetLastPhaseBeforeBarrier();

	memset(&m_nSplitTime,0,sizeof(m_nSplitTime));

	int i = 0, j = 0, k = 0;
	for (i = 0;i < MAX_RING_COUNT;i++)
	{
		m_tPhasePulseStatus[i].m_nPhaseIndex = 0;
		m_tPhasePulseStatus[i].m_nPhaseNum = m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[m_tPhasePulseStatus[i].m_nPhaseIndex].m_tPhaseParam.m_byPhaseNumber;
		m_tPhasePulseStatus[i].m_bGreenPulseStatus = false;
		m_tPhasePulseStatus[i].m_bRedPulseStatus = false;
		m_tPhasePulseStatus[i].m_nGreenPulseSendStatus = SEND_INIT;
		m_tPhasePulseStatus[i].m_nRedPulseSendStatus = SEND_INIT;
	}

	memset(m_wOriPlanPhaseSplitTimePlan, 0, sizeof(m_wOriPlanPhaseSplitTimePlan));
	for (i = 0;i < m_tFixTimeCtlInfo.m_nRingCount;i ++)
	{
		int nCount = m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_nPhaseCount;

		for (j = 0;j < nCount;j++)
		{
			for (k = 0;k < MAX_PHASE_COUNT;k ++)
			{
				if (m_atSplitInfo[k].m_bySplitPhase == m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[j].m_tPhaseParam.m_byPhaseNumber)
				{
					m_wOriPlanPhaseSplitTimePlan[i][j] = m_atSplitInfo[k].m_wSplitTime;  
					break; 
				}
			}
		}
	}

	memset(m_bKeepGreenChannelBeforeControlChannelFlag, false, sizeof(m_bKeepGreenChannelBeforeControlChannelFlag));
}

/*==================================================================== 
������ ��OnePhaseRun 
���� ��webster�����Ż�����ʱ������������λ����״̬����
�㷨ʵ�� �� 
����˵�� ��nRingIndex�������� 
����ֵ˵����
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ������          �������� 
====================================================================*/ 
void CLogicCtlWebsterOptim::OnePhaseRun(int nRingIndex)
{
    char szInfo[256] = {0};
    PTRingCtlInfo ptRingRunInfo = &(m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex]);
        
    int nIndex = m_tFixTimeCtlInfo.m_nCurPhaseIndex[nRingIndex];
    TPhaseLampClrRunCounter tRunCounter;
    m_pOpenATCRunStatus->GetPhaseLampClrRunCounter(tRunCounter);

    if (m_bCalcSplitTimeFlag)
    {
        CalcPhaseTime();
        m_bCalcSplitTimeFlag = false;
        memset(m_anDetectorTmpCounter,0,sizeof(m_anDetectorTmpCounter));
        memset(m_byDetectorTmpStatus,0,sizeof(m_byDetectorTmpStatus));
		memset(&m_nPhaseVehNumInfo, 0x00, sizeof(m_nPhaseVehNumInfo));
    }    

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
        //������λ�Ի�������λΪ׼
        m_tFixTimeCtlInfo.m_bIsChgPedStage[nRingIndex] = true;       

        char chcurStage = ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseStage;
        PTPhaseCtlInfo pPhaseInfo = (PTPhaseCtlInfo)&(ptRingRunInfo->m_atPhaseInfo[nIndex]);
        WORD wStageTime = 0;
        char chNextStage =  this->GetNextPhaseStageInfo(chcurStage,pPhaseInfo,wStageTime);
        ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseStage = chNextStage;
        ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPhaseStageRunTime = wStageTime;

        if (chNextStage == C_CH_PHASESTAGE_G)
        {
            ResetPhaseWebsterOptimInfo(nRingIndex,nIndex);
        }

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
        m_bIsLampClrChg = true;
    } 

    if (ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseStage == C_CH_PHASESTAGE_G ||
        ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseStage == C_CH_PHASESTAGE_GF)
    {
        ProcPhaseWebsterOptimInfo(nRingIndex,nIndex);
    }

    static bool bGreenPulse[MAX_RING_COUNT] = {false, false, false, false};
    static bool bRedPulse[MAX_RING_COUNT] = {false, false, false, false};

    if (ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseMode != NEGLECT_MODE && ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseMode != SHIELD_MODE &&
		ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseStage == C_CH_PHASESTAGE_G && tRunCounter.m_nLampClrTime[nRingIndex] == 
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
   
    if (ptRingRunInfo->m_atPhaseInfo[nNextIndex].m_chPhaseMode != NEGLECT_MODE && ptRingRunInfo->m_atPhaseInfo[nNextIndex].m_chPhaseMode != SHIELD_MODE &&
		ptRingRunInfo->m_atPhaseInfo[nNextIndex].m_tPhaseParam.m_byRedPulse != 0 && GetPhaseCycleRemainTime(nRingIndex, (PTPhaseCtlInfo)&(ptRingRunInfo->m_atPhaseInfo[nIndex]), tRunCounter) == ptRingRunInfo->m_atPhaseInfo[nNextIndex].m_tPhaseParam.m_byRedPulse)
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

/*==================================================================== 
������ ��OnePedPhaseRun
���� ��webster�����Ż�����ʱ����������λ����״̬����
�㷨ʵ�� �� 
����˵�� ��nRingIndex�������� 
����ֵ˵����
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ������          �������� 
====================================================================*/ 
void CLogicCtlWebsterOptim::OnePedPhaseRun(int nRingIndex)
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
        char chNextPedStage =  this->GetPedNextPhaseStageInfo(chcurPedStage,pPhaseInfo,wPedStageTime);
        ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPedPhaseStage = chNextPedStage;

        m_tFixTimeCtlInfo.m_bIsChgPedStage[nRingIndex] = false;

        m_pOpenATCLog->LogOneMessage(LEVEL_INFO, "OnePedPhaseRun RingIndex:%d CurPhaseIndex:%d CurPedStage:%c NextPedStage:%c", nRingIndex, nIndex, chcurPedStage, chNextPedStage);

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

    static bool bGreenPulse[MAX_RING_COUNT] = {false, false, false, false};
    static bool bRedPulse[MAX_RING_COUNT] = {false, false, false, false};

    if (ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseMode != NEGLECT_MODE && ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseMode != SHIELD_MODE &&
		ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPedPhaseStage == C_CH_PHASESTAGE_G && tRunCounter.m_nPedLampClrTime[nRingIndex] == 
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

    if (ptRingRunInfo->m_atPhaseInfo[nNextIndex].m_chPhaseMode != NEGLECT_MODE && ptRingRunInfo->m_atPhaseInfo[nNextIndex].m_chPhaseMode != SHIELD_MODE &&
		ptRingRunInfo->m_atPhaseInfo[nNextIndex].m_tPhaseParam.m_byRedPulse != 0 && GetPedPhaseCycleRemainTime(nRingIndex, (PTPhaseCtlInfo)&(ptRingRunInfo->m_atPhaseInfo[nIndex]), tRunCounter) == ptRingRunInfo->m_atPhaseInfo[nNextIndex].m_tPhaseParam.m_byRedPulse)
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

/*==================================================================== 
������ ��ReSetCtlStatus
���� ���������н���������λ����״̬��Ϣ
�㷨ʵ�� �� 
����˵�� ��
����ֵ˵����
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 �º���          �������� 
====================================================================*/ 
void CLogicCtlWebsterOptim::ReSetCtlStatus()
{
    CLogicCtlFixedTime::ReSetCtlStatus();
    m_bCalcSplitTimeFlag = true;
}   

/*==================================================================== 
������ ��CalcPhaseTime
���� ��������һ�����ڵ����űȲ���
�㷨ʵ�� �� 
����˵�� ��
����ֵ˵����
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 �º���          �������� 
====================================================================*/ 
void CLogicCtlWebsterOptim::CalcPhaseTime()
{
	int  nBestCycleLength		= 0;
	int  nIndex					= 0;
	bool bIsNeedCalc			= false;
	int  nCycleLostTime			= 0;
	float fFlowRatio			= (float)0.0;


	/// �����������ʱ��
	bIsNeedCalc = CalcWebsterBestCycleLength(nIndex, nBestCycleLength, nCycleLostTime, fFlowRatio);

	/// ���¼������ű�
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
������ ��RecalcWebsterBestSplitTimeByPlan
���� �����¼������λ�����ű�
�㷨ʵ�� �� 
����˵�� ��
����ֵ˵����
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 �º���          �������� 
2019/10/14     V1.0 ����            ���Ӹ��·��������űȣ��������¼��㱥�Ͷ�
====================================================================*/ 
void CLogicCtlWebsterOptim::RecalcWebsterBestSplitTimeByPlan()
{
	int i = 0;
	int j = 0;
	int nPhaseCount = 0;
	for (i = 0;i < m_tFixTimeCtlInfo.m_nRingCount;i ++)
	{
		nPhaseCount		= m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_nPhaseCount;
		for (j = 0;j < nPhaseCount;j++)
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
������ ��RecalcWebsterBestSplitTime
���� �����¼������λ�����ű�
�㷨ʵ�� �� 
����˵�� ��
        nIndex��������ڳ��Ļ�����
        nBestCycleTimeLength��������ڳ�
����ֵ˵����
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 �º���          �������� 
2019/10/14     V1.0 ����            ������λ��̬���Ӽ����̵ƿշ�
====================================================================*/ 
void CLogicCtlWebsterOptim::RecalcWebsterBestSplitTime(int nIndex, int nBestCycleLength, int nCycleLostTime, float & fFlowRatio)
{
	int i = 0;
	int j = 0;
	int k = 0;
	int nPhaseCount				= 0;
	int	nBestCycleLengthInUse	= 0;
	int nStartIndex				= 0;
	int nBarrierIndex			= 0;
	int nRecalcCycleTime		= 0;
	int nRecalePhaseNum			= 0;
	int nRecalePhaseIndex		= 0;
	int nCycleVaildGreenTime	= 0;
	int nPhaseGreenTime			= 0;

	WORD wOriSplitTime			= 0;
	WORD wSplitTime				= 0;
	WORD wTotalSplitTime		= 0;
	WORD wTotalOriSplitTime		= 0;
	WORD wBarrierSplitTimeArr[MAX_SEQUENCE_TABLE_COUNT];
	
	/// �ȴ����׼��������λ�����űȣ���¼���ϼ�����űȣ�����������λ���űȼ���ʱʹ��
	nPhaseCount = m_tFixTimeCtlInfo.m_atPhaseSeq[nIndex].m_nPhaseCount;
	memset(&wBarrierSplitTimeArr, 0,sizeof(wBarrierSplitTimeArr));

	// �������ڼ����������ʱ������λ�����ű��ܺͼ���λ�ĸ���
	for (i = 0;i < nPhaseCount;i++)
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
		nCycleVaildGreenTime  = nBestCycleLengthInUse - nCycleLostTime; //��Ҫ�ط��������Ч�̵�ʱ�䣬��������
		for (i = 0;i < nPhaseCount;i++)
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
					m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "### 11111111 m_fPhaseFlowRatio[nIndex][%d]=%f. fFlowRatio=%f.nCycleVaildGreenTime=%d",i, m_fPhaseFlowRatio[nIndex][i], fFlowRatio, nCycleVaildGreenTime);
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

			// ��������ǰ��
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

		for (i = 0;i < nPhaseCount;i++)
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

			// ��������ǰ��
			if (m_bLastPhaseBeforeBarrier[nIndex][i])
			{
				wBarrierSplitTimeArr[nBarrierIndex] = wTotalSplitTime;
				nBarrierIndex++;

				wTotalSplitTime = 0;
			}
		}
	}

	/// �ٴ���������
	for (k = 0;k < m_tFixTimeCtlInfo.m_nRingCount;k ++)
	{
		if (k != nIndex)	// �Ǳ�׼��
		{
			nPhaseCount		= m_tFixTimeCtlInfo.m_atPhaseSeq[k].m_nPhaseCount;
			nBarrierIndex	= 0;
			for (i = 0;i < nPhaseCount;i++)
			{
				wTotalOriSplitTime += m_wOriPlanPhaseSplitTime[k][i];

				// ��������������ǰ��
				if (m_bLastPhaseBeforeBarrier[k][i])
				{
					// ����������ϼ�����ű��ܺ�
					wTotalSplitTime = wBarrierSplitTimeArr[nBarrierIndex];
					nBarrierIndex++;

					// ����ǰ�����һ����λ�����λ�����űȼ���
					for (j=nStartIndex; j<i; j++)
					{
						// ���㵥����λ���ű�
						wOriSplitTime = m_wOriPlanPhaseSplitTime[k][j];
						wSplitTime = wTotalSplitTime * wOriSplitTime / wTotalOriSplitTime;

						wSplitTime = SetPhaseSplitTime(k, j, wSplitTime, false);
						m_wOriPlanPhaseSplitTime[k][j] = wSplitTime;
						wTotalSplitTime -= wSplitTime;
					}
					// ����ǰ���һ����λ�����űȼ���
					wSplitTime = wTotalSplitTime;
					SetPhaseSplitTime(k, i, wSplitTime, true);
					m_wOriPlanPhaseSplitTime[k][i] = wSplitTime;
					// �����¸����Ͽ�ʼ���
					nStartIndex = i+1;	

					wTotalOriSplitTime = 0;
				}
			}
		}
	}
}

/*==================================================================== 
������ ��SetPhaseSplitTime
���� ��������λ���ű�
�㷨ʵ�� �� 
����˵�� ��
        nRingIndex��������
        nPhaseIndex����λ����
        nSplitTime�����ű�
		bFlag, �Ƿ�Ϊ����ǰ���һ����λ
����ֵ˵����
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 �º���          �������� 
====================================================================*/ 
int CLogicCtlWebsterOptim::SetPhaseSplitTime(int nRingIndex, int nPhaseIndex, WORD wSplitTime, bool bFlag)
{

	PTPhase pPhaseInfo = &(m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_tPhaseParam);
	if (!bFlag)
	{
		if (wSplitTime < (pPhaseInfo->m_wPhaseMinimumGreen + pPhaseInfo->m_byGreenFlash + pPhaseInfo->m_byPhaseYellowChange + pPhaseInfo->m_byPhaseRedClear))
		{
			wSplitTime = pPhaseInfo->m_wPhaseMinimumGreen + pPhaseInfo->m_byGreenFlash + pPhaseInfo->m_byPhaseYellowChange + pPhaseInfo->m_byPhaseRedClear;
		}
		else if (wSplitTime > (pPhaseInfo->m_wPhaseMaximum1 + pPhaseInfo->m_byGreenFlash + pPhaseInfo->m_byPhaseYellowChange + pPhaseInfo->m_byPhaseRedClear))
		{
			wSplitTime = pPhaseInfo->m_wPhaseMaximum1 + pPhaseInfo->m_byGreenFlash + pPhaseInfo->m_byPhaseYellowChange + pPhaseInfo->m_byPhaseRedClear;
		}
	}

	m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_wPhaseTime = wSplitTime;
	m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_wPhaseGreenTime = wSplitTime - pPhaseInfo->m_byGreenFlash - pPhaseInfo->m_byPhaseYellowChange - pPhaseInfo->m_byPhaseRedClear;
	m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_wPedPhaseGreenTime = wSplitTime - pPhaseInfo->m_byPhaseYellowChange - pPhaseInfo->m_byPhasePedestrianClear - pPhaseInfo->m_byPhaseRedClear;

	return wSplitTime;
}

/*==================================================================== 
������ ��CalcWebsterBestCycleLength
���� ��������һ��������λ�ı��Ͷ�
�㷨ʵ�� �� 
����˵�� ��
        nIndex��������
        nBestCycleTimeLength��������ڳ�
����ֵ˵����
        ����ɹ�����true
        ����������޷����㷵��false
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 �º���          �������� 
2019/10/14     V1.0 ����            ���Ͷ����ӷ���ѧϰģʽ
2019/11/14     V1.0 ����            �ܱ��Ͷ�
====================================================================*/ 
bool CLogicCtlWebsterOptim::CalcWebsterBestCycleLength(int & nIndex, int & nBestCycleTimeLength, int & nCycleLostTime, float & fFlowRatio)
{
	int i = 0;
	int j = 0;
	int k = 0;
	int nMaxDetVehNum			= 0;
	int nPerPhaseDetVehNum		= 0;
	int nSaturationFlow			= 0;
	int nValidDetCount			= 0;
	int nPhaseCount				= 0;
	int nValidPhaseCount		= 0;
	int nTempBestCycleLength	= 0;
	int nRecalcCycleTime		= 0;
	int nRecalcValidCycleTime	= 0;
	int nTotalCycleLostTime		= 0;

	bool bIsCalc = false;

	float fPerPhaseValidVehPassTime	= (float)0.0;
	float fTotalFlowRatio			= (float)0.0;
	m_fBalanceTotalFlowRatio = (float)0.0;

	nIndex = 0;
	nBestCycleTimeLength = 0;
	memset(&m_nPhaseVehNumInfo, 0x00, sizeof(m_nPhaseVehNumInfo));
	memset(&m_nPhaseValidVehNum, 0x00, sizeof(m_nPhaseValidVehNum));
	memset(m_nMinBestCycleLength, 0x00, sizeof(m_nMinBestCycleLength));
	memset(m_nMaxBestCycleLength, 0x00, sizeof(m_nMaxBestCycleLength));
	memset(&m_fPhaseFlowRatio, 0x00, sizeof(m_fPhaseFlowRatio));
	memset(&m_fPhaseBalanceFlowRatio, 0x00, sizeof(m_fPhaseBalanceFlowRatio));
	for (i = 0;i < m_tFixTimeCtlInfo.m_nRingCount;i ++)
	{
		nPhaseCount			= m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_nPhaseCount;
		nValidDetCount		= 0;
		nValidPhaseCount	= 0;
		fTotalFlowRatio		= (float)0.0;
		for (j = 0;j < nPhaseCount;j++)
		{
			m_fPhaseValidVehPassTime[i][j]		= (float)100.0;
			PTOnePhaseWebsterOptimInfo ptOnePhaseOptimInfo = &m_tWebsterOptimInfo.m_atRingOptimInfo[i].m_atPhaseOptimInfo[j];
			if (ptOnePhaseOptimInfo->m_nDetectorCount)
			{
				for (k = 0;k < ptOnePhaseOptimInfo->m_nDetectorCount;k ++)
				{
					BYTE byDetID = ptOnePhaseOptimInfo->m_byDetectorID[k];

					if (ptOnePhaseOptimInfo->m_byDetectorStatus[k] == 0 && ptOnePhaseOptimInfo->m_anValidPassVeh[k] > 0)//����
					{
						if (ptOnePhaseOptimInfo->m_anValidPassVeh[k] > 1)
						{
							fPerPhaseValidVehPassTime = (float)(ptOnePhaseOptimInfo->m_anValidPassTime[k])/1000;
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
			// ��ȡ������Ч���������λ�����ڳ�����������
			nRecalcCycleTime		= 0;
			nRecalcValidCycleTime	= 0;
			nSaturationFlow			=  0;
			int nPhaseValidGreenTime = 0;
			for (j = 0;j < nPhaseCount;j++)
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
					// ��������
					if (m_tAscSingleOptimInfo.m_bySelfLearn == 1)
					{
						nSaturationFlow += ((int)(nPhaseValidGreenTime / m_fPhaseValidMinVehPassTime[i][j] > 0.0) ? floor(nPhaseValidGreenTime / m_fPhaseValidMinVehPassTime[i][j] + 0.5) : ceil(nPhaseValidGreenTime / m_fPhaseValidMinVehPassTime[i][j] - 0.5));
					}
					else
					{
						nSaturationFlow += ((int)(nPhaseValidGreenTime * m_tAscSingleOptimInfo.m_nMaxSaturatedFlow / 3600.00 > 0.0) ? floor(nPhaseValidGreenTime * m_tAscSingleOptimInfo.m_nMaxSaturatedFlow / 3600.00 + 0.5) : ceil(nPhaseValidGreenTime * m_tAscSingleOptimInfo.m_nMaxSaturatedFlow / 3600.00 - 0.5));
					}
					m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "CalcPhaseTime calculated best cycle length. Ring%d,phase%d, nPhaseValidGreenTime=%d, m_fPhaseValidMinVehPassTime=%f.", i, j, nPhaseValidGreenTime, m_fPhaseValidMinVehPassTime[i][j]);
					nValidPhaseCount++;
				}
			}
			m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "CalcPhaseTime calculated best cycle length. nValidPhaseCount=%d, nSaturationFlow=%d.", nValidPhaseCount, nSaturationFlow);
			if (nValidPhaseCount > 0)
			{
				for (j = 0;j < nPhaseCount;j++)
				{
					PTOnePhaseWebsterOptimInfo ptOnePhaseOptimInfo = &m_tWebsterOptimInfo.m_atRingOptimInfo[i].m_atPhaseOptimInfo[j];
					if (ptOnePhaseOptimInfo->m_nDetectorCount)
					{
						for (k = 0;k < ptOnePhaseOptimInfo->m_nDetectorCount;k ++)
						{
							BYTE byDetID = ptOnePhaseOptimInfo->m_byDetectorID[k];

							if (ptOnePhaseOptimInfo->m_byDetectorStatus[k] == 0)//����
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
					// ������������ʧʱ��
					nTotalCycleLostTime = (int)(nValidPhaseCount * (m_tAscSingleOptimInfo.m_fGreenStartUpLoss + m_tAscSingleOptimInfo.m_fYellowEndLoss));
					
					for (j=0; j<nPhaseCount; j++)
					{
						if (m_nPhaseVehNumInfo[i][j] == WEBSTER_MULTI_VEH_PASS)
						{
							PTPhase pPhaseInfo = &(m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[j].m_tPhaseParam);
							nTotalCycleLostTime += pPhaseInfo->m_byPhaseRedClear;
						}
					}

					// �����������ʱ��
					nTempBestCycleLength = (int)((1.5 * nTotalCycleLostTime + 5)/(1 - fTotalFlowRatio));
					m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "CalcPhaseTime calculated best cycle length. Ring[%d] nTotalCycleLostTime=%d, Old nTempBestCycleLength=%d, min=%d, max=%d.", i, nTotalCycleLostTime, nTempBestCycleLength, m_nMinBestCycleLength[i], m_nMaxBestCycleLength[i]);
					nTempBestCycleLength = (int)(m_tAscSingleOptimInfo.m_fCycleAdjustFactor * nTempBestCycleLength / 100);
					m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "m_fCycleAdjustFactor=%d.", m_tAscSingleOptimInfo.m_fCycleAdjustFactor);
					m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "CalcPhaseTime calculated best cycle length. Ring[%d] nTotalCycleLostTime=%d, nTempBestCycleLength=%d, min=%d, max=%d.", i, nTotalCycleLostTime, nTempBestCycleLength, m_nMinBestCycleLength[i], m_nMaxBestCycleLength[i]);
					if (nTempBestCycleLength < m_nMinBestCycleLength[i])
					{
						nTempBestCycleLength = m_nMinBestCycleLength[i];
						for (j = 0;j < nPhaseCount;j++)
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
						for (j = 0;j < nPhaseCount;j++)
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
						nCycleLostTime		 = nTotalCycleLostTime;
						fFlowRatio			 = fTotalFlowRatio;
					}
				}
				else if (fTotalFlowRatio > 0.9 && fTotalFlowRatio < 1.0)
				{
					// ������������ʧʱ��
					nTotalCycleLostTime = (int)(nValidPhaseCount * (m_tAscSingleOptimInfo.m_fGreenStartUpLoss + m_tAscSingleOptimInfo.m_fYellowEndLoss));

					for (j = 0; j < nPhaseCount; j++)
					{
						if (m_nPhaseVehNumInfo[i][j] == WEBSTER_MULTI_VEH_PASS)
						{
							PTPhase pPhaseInfo = &(m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[j].m_tPhaseParam);
							nTotalCycleLostTime += pPhaseInfo->m_byPhaseRedClear;
						}
					}

					// �����������ʱ��
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
					for (j = 0;j < nPhaseCount;j++)
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

	// û�м����
	bIsCalc = false;
	for (i = 0;i < m_tFixTimeCtlInfo.m_nRingCount;i ++)
	{
		nPhaseCount = m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_nPhaseCount;
		for (j = 0;j < nPhaseCount;j++)
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
������ ��InitOptimParam
���� ����ʼ���Ż���Ҫ�Ĳ���
�㷨ʵ�� �� 
����˵�� ��
����ֵ˵����
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 �º���          �������� 
====================================================================*/ 
void CLogicCtlWebsterOptim::InitWebsterOptimParam()
{
    int i = 0;
    int j = 0;
    int k = 0;
	memset(m_wOriPlanPhaseSplitTime,0x00,sizeof(m_wOriPlanPhaseSplitTime)); //��ʼ�ƻ����ű�ʱ��

    m_tWebsterOptimInfo.m_nRingCount = m_tFixTimeCtlInfo.m_nRingCount;
    for (i = 0;i < m_tFixTimeCtlInfo.m_nRingCount;i ++)
    {
        int nCount = m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_nPhaseCount;
        m_tWebsterOptimInfo.m_atRingOptimInfo[i].m_nPhaseCount = nCount;
    
        for (j = 0;j < nCount;j++)
        {
            PTOnePhaseWebsterOptimInfo ptOnePhaseOptimInfo = &m_tWebsterOptimInfo.m_atRingOptimInfo[i].m_atPhaseOptimInfo[j];
            PTRingCtlInfo ptRingRunInfo = &(m_tFixTimeCtlInfo.m_atPhaseSeq[i]);

			m_wOriPlanPhaseSplitTime[i][j] = m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[j].m_wPhaseTime;

            int nDetCount = 0;
            for (int k = 0;k < m_tFixTimeCtlInfo.m_nVehDetCount;k ++)
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
������ ��ResetPhaseOptimInfo
���� ��������λ�Ż�����
�㷨ʵ�� �� 
����˵�� ��
        nRingIndex��������
        nPhaseIndex����λ����
����ֵ˵����
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 �º���          �������� 
====================================================================*/ 
void CLogicCtlWebsterOptim::ResetPhaseWebsterOptimInfo(int nRingIndex,int nPhaseIndex)
{
    PTOnePhaseWebsterOptimInfo ptOnePhaseOptimInfo = &m_tWebsterOptimInfo.m_atRingOptimInfo[nRingIndex].m_atPhaseOptimInfo[nPhaseIndex];
    int i = 0;    
    for (i = 0;i < ptOnePhaseOptimInfo->m_nDetectorCount;i ++)
    {
		//ptOnePhaseOptimInfo->m_anValidPassTime[i] = 0;
		ptOnePhaseOptimInfo->m_anValidPassVeh[i] = 0;
        ptOnePhaseOptimInfo->m_byDetectorStatus[i] = 0;
    }
}

/*==================================================================== 
������ ��ProcPhaseOptimInfo
���� ��������λ��λ�Ż�����
�㷨ʵ�� �� 
����˵�� ��
        nRingIndex��������
        nPhaseIndex����λ����
����ֵ˵����
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 �º���          �������� 
====================================================================*/ 
void CLogicCtlWebsterOptim::ProcPhaseWebsterOptimInfo(int nRingIndex,int nPhaseIndex)
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
    for (i = 0;i < ptOnePhaseOptimInfo->m_nDetectorCount;i ++)
    {
        BYTE byDetID = ptOnePhaseOptimInfo->m_byDetectorID[i];

        if (ptOnePhaseOptimInfo->m_byDetectorStatus[i] == 0)//����
        {
            for (int j = 0;j < m_tFixTimeCtlInfo.m_nVehDetCount;j ++)
            {
                if (m_tFixTimeCtlInfo.m_atVehDetector[j].m_byVehicleDetectorNumber == byDetID)
                {
                    if (tVehDetData.m_bDetFaultStatus[j])//����
                    {
                        //��������ֻҪ����������������ϾͰ�״̬��Ϊ����,�������ű�ʱ��ʹ�øü����
                        ptOnePhaseOptimInfo->m_byDetectorStatus[i] = 1;
                    }
					nIndex = j;
                    break;
                }
            }    
        }

        if (ptOnePhaseOptimInfo->m_byDetectorStatus[i] == 1)//����
        {
            break;
        }
        
        if (tVehDetData.m_chDetStatus[nIndex] == 1)//�г�
        {
            if (m_byDetectorTmpStatus[nIndex] == 0)
            {
                m_byDetectorTmpStatus[nIndex] = 1;

				int nPassTime = C_N_TIMER_MILLSECOND * CalcCounter(m_anDetectorTmpCounter[nIndex], nGlobalCounter, C_N_MAXGLOBALCOUNTER);
			

				if (nPassTime < (float)1000.0)	//���˵�̫С��ֵ
					return;

				if (ptOnePhaseOptimInfo->m_anValidPassTime[i] == 0)
				{
					ptOnePhaseOptimInfo->m_anValidPassTime[i] = nPassTime;
				}
				else if (nPassTime < ptOnePhaseOptimInfo->m_anValidPassTime[i])
				{
					ptOnePhaseOptimInfo->m_anValidPassTime[i] = nPassTime;
				}
				ptOnePhaseOptimInfo->m_anValidPassVeh[i]  += 1;
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
������ ��CheckPhaseIfHavaEffectiveDet
���� ���жϵ�ǰ��λ�Ƿ�������ʹ�õļ����
�㷨ʵ�� �� 
����˵�� ��
        nRingIndex��������
        nPhaseIndex����λ����
����ֵ˵����
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2021/01/16     V1.0 �º���          �������� 
====================================================================*/ 
bool CLogicCtlWebsterOptim::CheckPhaseIfHavaEffectiveDet(int nRingIndex,int nPhaseIndex)
{
	bool bFlag = false;

	PTOnePhaseWebsterOptimInfo ptOnePhaseOptimInfo = &m_tWebsterOptimInfo.m_atRingOptimInfo[nRingIndex].m_atPhaseOptimInfo[nPhaseIndex];

	TRealTimeVehDetData tVehDetData;
	m_pOpenATCRunStatus->GetRTVehDetData(tVehDetData);

	int i = 0;
	int j = 0;    
	for (i = 0;i < ptOnePhaseOptimInfo->m_nDetectorCount;i ++)
	{
		BYTE byDetID = ptOnePhaseOptimInfo->m_byDetectorID[i];

		if (ptOnePhaseOptimInfo->m_byDetectorStatus[i] == 0)//����
		{
			for (int j = 0;j < m_tFixTimeCtlInfo.m_nVehDetCount;j ++)
			{
				if (m_tFixTimeCtlInfo.m_atVehDetector[j].m_byVehicleDetectorNumber == byDetID)
				{
					if (tVehDetData.m_bDetFaultStatus[j])//����
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

		if (bFlag)	//���������ļ����
		{
			m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "tVehDetData.m_bDetFaultStatus hava good");
			break;
		}
	}
	return bFlag;
}
