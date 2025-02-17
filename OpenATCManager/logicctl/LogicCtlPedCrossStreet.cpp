/*=====================================================================
模块名 ：行人过街控制方式接口模块
文件名 ：LogicCtlPedCrossStreet.cpp
相关文件：LogicCtlPedCrossStreet.h,LogicCtlFixedTime.h
实现功能：行人过街控制方式实现
作者 ：刘黎明
版权 ：<Copyright(c) 2019-2020 Suzhou Keda Technology Co.,Ltd. All right reserved.>
-----------------------------------------------------------------------
修改记录：
日 期           版本     修改人     走读人     修改记录
2019/9/14       V1.0     刘黎明     刘黎明     创建模块
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
函数名 ：Init 
功能 ：行人过街控制方式类资源初始化
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

    //当前运行模式为行人过街控制
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
函数名 ：OnePhaseRun 
功能 ：行人过街控制时单个机动车相位运行状态更新
算法实现 ： 
参数说明 ：nRingIndex，环索引 
返回值说明：
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 刘黎明          创建函数 
2020/05/29     V1.0 李永萍          添加实时更新非行人相位的实际相位运行时间 
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
        if (m_bIsPedAskPhase)//有行人请求时，行人请求相位执行绿信比，非行人请求相位的绿灯时间默认执行最小绿
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
        else//没有行人请求时，行人请求相位的绿灯时间默认执行最小绿，非行人请求相位的绿灯时间默认执行最大绿
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
    
    //当前相位阶段是否完成
    bool bChgStage = false;
    if (tRunCounter.m_nLampClrTime[nRingIndex] >= ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPhaseStageRunTime)
    {
        if (ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseStage == C_CH_PHASESTAGE_G)
        {
            if (ptRingRunInfo->m_atPhaseInfo[nIndex].m_bIsPedAskPhase)
            {
                if (m_bIsPedAskPhase)//有行人请求，行人请求相位的绿灯时间执行绿信比的绿灯时间
                {
                    if (tRunCounter.m_nLampClrTime[nRingIndex] >= ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPhaseGreenTime)
                    {
                        bChgStage = true;  
                    }
                }  
                else//没有行人请求，行人请求相位的绿灯时间执行最小绿
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
                //清空行人请求
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
        //行人相位以机动车相位为准,行人相位把机动车时间也配置好.
        m_tFixTimeCtlInfo.m_bIsChgPedStage[nRingIndex] = true;       

        char chcurStage = ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseStage;
        PTPhaseCtlInfo pPhaseInfo = (PTPhaseCtlInfo)&(ptRingRunInfo->m_atPhaseInfo[nIndex]);
        WORD wStageTime = 0;
        char chNextStage =  this->GetNextPhaseStageInfo(chcurStage,pPhaseInfo,wStageTime);
        ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseStage = chNextStage;
        ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPhaseStageRunTime = wStageTime;

        if (chNextStage == C_CH_PHASESTAGE_G)//所有相位的绿灯时间先置为最小绿
        {
            PTPhase pPhaseInfo = &(m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nIndex].m_tPhaseParam);
            ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPhaseStageRunTime = pPhaseInfo->m_wPhaseMinimumGreen;   
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
    }

}

/*==================================================================== 
函数名 ：OnePedPhaseRun
功能 ：行人过街控制时单个行人相位运行状态更新
算法实现 ： 
参数说明 ：nRingIndex，环索引 
返回值说明：
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 刘黎明          创建函数 
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

        //设置环内相位灯色，重置时间
        SetLampClrByRing(nRingIndex,nIndex,ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseStage,ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPedPhaseStage);
        tRunCounter.m_nPedLampClrTime[nRingIndex] = 0;
        tRunCounter.m_nPedLampClrStartCounter[nRingIndex] = tRunCounter.m_nCurCounter;    
        m_pOpenATCRunStatus->SetPhaseLampClrRunCounter(tRunCounter);
        m_bIsLampClrChg = true;
    }
}

/*==================================================================== 
函数名 ：IsPedAskCrossStreet
功能 ：行人相位是否有人请求通过
算法实现 ： 
参数说明 ：
返回值说明：
        有行人请求返回true
        无行人请求返回false.
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 刘黎明          创建函数 
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

