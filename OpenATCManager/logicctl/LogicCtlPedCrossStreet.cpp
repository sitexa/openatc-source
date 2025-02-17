/*=====================================================================
ģ���� �����˹��ֿ��Ʒ�ʽ�ӿ�ģ��
�ļ��� ��LogicCtlPedCrossStreet.cpp
����ļ���LogicCtlPedCrossStreet.h,LogicCtlFixedTime.h
ʵ�ֹ��ܣ����˹��ֿ��Ʒ�ʽʵ��
���� ��������
��Ȩ ��<Copyright(c) 2019-2020 Suzhou Keda Technology Co.,Ltd. All right reserved.>
-----------------------------------------------------------------------
�޸ļ�¼��
�� ��           �汾     �޸���     �߶���     �޸ļ�¼
2019/9/14       V1.0     ������     ������     ����ģ��
=====================================================================*/
#include "LogicCtlPedCrossStreet.h"
#include <string.h>
#include <stdlib.h>

CLogicCtlPedCrossStreet::CLogicCtlPedCrossStreet()
{

}

CLogicCtlPedCrossStreet::~CLogicCtlPedCrossStreet()
{

}

/*==================================================================== 
������ ��Init 
���� �����˹��ֿ��Ʒ�ʽ����Դ��ʼ��
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
void CLogicCtlPedCrossStreet::Init(COpenATCParameter * pParameter, COpenATCRunStatus * pRunStatus, COpenATCLog * pOpenATCLog, int nPlanNo)
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

    //��ǰ����ģʽΪ���˹��ֿ���
    m_nCurRunMode = CTL_MODE_PEDCROSTREET;

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
    tCtlStatus.m_nCurCtlMode = CTL_MODE_PEDCROSTREET;
    tCtlStatus.m_nCurPlanNo = (int)byPlanID;
    m_pOpenATCRunStatus->SetLogicCtlStatus(tCtlStatus);

    RetsetAllChannelStatus();

    GetRunStageTable();

	GetLastPhaseBeforeBarrier();

    memset(&m_nSplitTime,0,sizeof(m_nSplitTime));   

    m_bIsPedAskPhase = false;
	memset(&m_nGreenRunTime,0,sizeof(m_nGreenRunTime)); 

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
������ ��OnePhaseRun 
���� �����˹��ֿ���ʱ������������λ����״̬����
�㷨ʵ�� �� 
����˵�� ��nRingIndex�������� 
����ֵ˵����
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ������          �������� 
2020/05/29     V1.0 ����Ƽ          ���ʵʱ���·�������λ��ʵ����λ����ʱ�� 
====================================================================*/ 
void CLogicCtlPedCrossStreet::OnePhaseRun(int nRingIndex)
{
    char szInfo[256] = {0};
    PTRingCtlInfo ptRingRunInfo = &(m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex]);
        
    int nIndex = m_tFixTimeCtlInfo.m_nCurPhaseIndex[nRingIndex];
    TPhaseLampClrRunCounter tRunCounter;
    m_pOpenATCRunStatus->GetPhaseLampClrRunCounter(tRunCounter);

    IsPedAskCrossStreet();
    
    for (int i = 0;i < ptRingRunInfo->m_nPhaseCount;i++)
    {
        if (m_bIsPedAskPhase)//����������ʱ������������λִ�����űȣ�������������λ���̵�ʱ��Ĭ��ִ����С��
        {
            if (ptRingRunInfo->m_atPhaseInfo[i].m_bIsPedAskPhase)
            {
                if (nIndex == i)
                {
                    m_nGreenRunTime[nRingIndex][i] = ptRingRunInfo->m_atPhaseInfo[i].m_wPhaseGreenTime;
					m_nSplitTime[nRingIndex][i] = ptRingRunInfo->m_atPhaseInfo[i].m_wPhaseTime;
                }
            }
            else 
            {
                m_nGreenRunTime[nRingIndex][i] = ptRingRunInfo->m_atPhaseInfo[i].m_tPhaseParam.m_wPhaseMinimumGreen;
                m_nSplitTime[nRingIndex][i] = ptRingRunInfo->m_atPhaseInfo[i].m_tPhaseParam.m_wPhaseMinimumGreen + ptRingRunInfo->m_atPhaseInfo[i].m_wPhaseTime - ptRingRunInfo->m_atPhaseInfo[i].m_wPhaseGreenTime;  
            }
        }
        else//û����������ʱ������������λ���̵�ʱ��Ĭ��ִ����С�̣�������������λ���̵�ʱ��Ĭ��ִ�������
        {
            if (ptRingRunInfo->m_atPhaseInfo[i].m_bIsPedAskPhase)
            {
                m_nGreenRunTime[nRingIndex][i] = ptRingRunInfo->m_atPhaseInfo[i].m_tPhaseParam.m_wPhaseMinimumGreen;
                m_nSplitTime[nRingIndex][i] = ptRingRunInfo->m_atPhaseInfo[i].m_tPhaseParam.m_wPhaseMinimumGreen + ptRingRunInfo->m_atPhaseInfo[i].m_wPhaseTime - ptRingRunInfo->m_atPhaseInfo[i].m_wPhaseGreenTime;    
            }
            else 
            {
                m_nGreenRunTime[nRingIndex][i] = ptRingRunInfo->m_atPhaseInfo[i].m_tPhaseParam.m_wPhaseMaximum1;
                m_nSplitTime[nRingIndex][i] = ptRingRunInfo->m_atPhaseInfo[i].m_tPhaseParam.m_wPhaseMaximum1 + ptRingRunInfo->m_atPhaseInfo[i].m_wPhaseTime - ptRingRunInfo->m_atPhaseInfo[i].m_wPhaseGreenTime;
            }
        }
    }
    
    //��ǰ��λ�׶��Ƿ����
    bool bChgStage = false;
    if (tRunCounter.m_nLampClrTime[nRingIndex] >= ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPhaseStageRunTime)
    {
        if (ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseStage == C_CH_PHASESTAGE_G)
        {
            if (ptRingRunInfo->m_atPhaseInfo[nIndex].m_bIsPedAskPhase)
            {
                if (m_bIsPedAskPhase)//��������������������λ���̵�ʱ��ִ�����űȵ��̵�ʱ��
                {
                    if (tRunCounter.m_nLampClrTime[nRingIndex] >= ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPhaseGreenTime)
                    {
                        bChgStage = true;  
                    }
                }  
                else//û��������������������λ���̵�ʱ��ִ����С��
                {
                    bChgStage = true; 
                }
            }
            else
            {
                if ((WORD)tRunCounter.m_nLampClrTime[nRingIndex] >= ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_wPhaseMaximum1)
                {
                    bChgStage = true;
                }
                else
                {
                    if (m_bIsPedAskPhase)
                    {
                        m_nSplitTime[nRingIndex][nIndex] = tRunCounter.m_nLampClrTime[nRingIndex] + ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPhaseTime - ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPhaseGreenTime;
                        m_nGreenRunTime[nRingIndex][nIndex] = tRunCounter.m_nLampClrTime[nRingIndex];
                        bChgStage = true;
                    }
                }
            }
        }
        else if (ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseStage == C_CH_PHASESTAGE_GF)
        {
            bChgStage = true;
            if (ptRingRunInfo->m_atPhaseInfo[nIndex].m_bIsPedAskPhase)
            {
                //�����������
                m_bIsPedAskPhase = false; 
            }
        }
        else
        {
            bChgStage = true;
        }
    }
    
    if (bChgStage)
    {
        if (nRingIndex == 0)
        {
            if (ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseStage == C_CH_PHASESTAGE_G)
            {
                m_tFixTimeCtlInfo.m_wCycleRunTime += m_nGreenRunTime[nRingIndex][nIndex];
                m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "OnePhaseRun RingIndex:%d CurPhaseIndex:%d CurStage:%c PhaseStageRunTime:%d", nRingIndex, nIndex, ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseStage, m_nGreenRunTime[nRingIndex][nIndex]);
            }
            else
            {
                m_tFixTimeCtlInfo.m_wCycleRunTime += ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPhaseStageRunTime;
                m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "OnePhaseRun RingIndex:%d CurPhaseIndex:%d CurStage:%c PhaseStageRunTime:%d", nRingIndex, nIndex, ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseStage, ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPhaseStageRunTime);
            }            
        }

        m_tFixTimeCtlInfo.m_bIsChgStage[nRingIndex] = true;
    }

    if (m_tFixTimeCtlInfo.m_bIsChgStage[nRingIndex])
    {
        //������λ�Ի�������λΪ׼,������λ�ѻ�����ʱ��Ҳ���ú�.
        m_tFixTimeCtlInfo.m_bIsChgPedStage[nRingIndex] = true;       

        char chcurStage = ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseStage;
        PTPhaseCtlInfo pPhaseInfo = (PTPhaseCtlInfo)&(ptRingRunInfo->m_atPhaseInfo[nIndex]);
        WORD wStageTime = 0;
        char chNextStage =  this->GetNextPhaseStageInfo(chcurStage,pPhaseInfo,wStageTime);
        ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseStage = chNextStage;
        ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPhaseStageRunTime = wStageTime;

        if (chNextStage == C_CH_PHASESTAGE_G)//������λ���̵�ʱ������Ϊ��С��
        {
            PTPhase pPhaseInfo = &(m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nIndex].m_tPhaseParam);
            ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPhaseStageRunTime = pPhaseInfo->m_wPhaseMinimumGreen;   
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
    }

}

/*==================================================================== 
������ ��OnePedPhaseRun
���� �����˹��ֿ���ʱ����������λ����״̬����
�㷨ʵ�� �� 
����˵�� ��nRingIndex�������� 
����ֵ˵����
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ������          �������� 
====================================================================*/ 
void CLogicCtlPedCrossStreet::OnePedPhaseRun(int nRingIndex)
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
		ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPedPhaseStageRunTime = wPedStageTime;

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
}

/*==================================================================== 
������ ��IsPedAskCrossStreet
���� ��������λ�Ƿ���������ͨ��
�㷨ʵ�� �� 
����˵�� ��
����ֵ˵����
        ���������󷵻�true
        ���������󷵻�false.
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ������          �������� 
====================================================================*/ 
bool CLogicCtlPedCrossStreet::IsPedAskCrossStreet()
{
    TIOBoardData tBoardData;
    m_pOpenATCRunStatus->GetIOBoardData(tBoardData);

    int i = 0;
    bool bFind = false;
    for (i = 0;i < m_tFixTimeCtlInfo.m_nPedDetCount;i ++)
    {
        BYTE byPedDeId = m_tFixTimeCtlInfo.m_atPedDetector[i].m_byPedestrianDetectorNumber;
        int nBoardIndex = (byPedDeId - 1) / C_N_MAXIOINPUT_NUM;
        int nIOIndex = (byPedDeId - 1) % C_N_MAXIOINPUT_NUM;

        if (tBoardData.m_atIOBoardData[nBoardIndex].m_achIOStatus[nIOIndex] == 1)
        {
            if (!bFind)
            {
                bFind = true;
				break;
            }
        }
    }

    if (bFind)
    {
        m_bIsPedAskPhase = true;
    }

    return bFind;
}

