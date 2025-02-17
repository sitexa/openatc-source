/*=====================================================================
模块名 ：无电缆线控控制方式接口模块
文件名 ：LogicCtlCablelessLine.cpp
相关文件：LogicCtlCablelessLine.h,LogicCtlFixedTime.h
实现功能：无电缆线控控制方式实现
作者 ：刘黎明
版权 ：<Copyright(c) 2019-2020 Suzhou Keda Technology Co.,Ltd. All right reserved.>
-----------------------------------------------------------------------
修改记录：
日 期           版本     修改人     走读人     修改记录
2019/9/14       V1.0     刘黎明     刘黎明     创建模块
=====================================================================*/
#include "LogicCtlCablelessLine.h"
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <algorithm>

CLogicCtlCablelessLine::CLogicCtlCablelessLine()
{
}

CLogicCtlCablelessLine::~CLogicCtlCablelessLine()
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
函数名 ：Init 
功能 ：无电缆线控控制方式类资源初始化
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
void CLogicCtlCablelessLine::Init(COpenATCParameter * pParameter, COpenATCRunStatus * pRunStatus, COpenATCLog * pOpenATCLog, int nPlanNo)
{
    m_pOpenATCParameter = pParameter;
    m_pOpenATCRunStatus = pRunStatus;
	m_pOpenATCLog       = pOpenATCLog;
    m_nRealOffset		= 0;
    //m_nGreenStageChgLen = 0;
    m_bCalcOffsetFlag	= true;
    memset(&m_tFixTimeCtlInfo,0,sizeof(m_tFixTimeCtlInfo));
    memset(&m_tRunStageInfo, 0x00, sizeof(m_tRunStageInfo));
    memset(m_nChannelSplitMode,0x00,sizeof(m_nChannelSplitMode)); 
	memset(&m_bLastPhaseBeforeBarrier, 0x00, sizeof(m_bLastPhaseBeforeBarrier));
	memset(m_bChangeToYellow, 0, sizeof(m_bChangeToYellow));
	memset(m_bShieldStatus, 0x00, sizeof(m_bShieldStatus));
	memset(m_bProhibitStatus, 0x00, sizeof(m_bProhibitStatus));
	memset(m_bOldShieldStatus, 0x00, sizeof(m_bOldShieldStatus));
	memset(m_bOldProhibitStatus, 0x00, sizeof(m_bOldProhibitStatus));
	memset(m_bySplitPhaseMode, 0x00, sizeof(m_bySplitPhaseMode));
	memset(m_nSplitPhaseTime, 0x00, sizeof(m_nSplitPhaseTime));

	//当前运行模式为无电缆控制
	m_nCurRunMode = CTL_MODE_CABLELESS;

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
    tCtlStatus.m_nCurCtlMode = CTL_MODE_CABLELESS;
    tCtlStatus.m_nCurPlanNo = (int)byPlanID;
    m_pOpenATCRunStatus->SetLogicCtlStatus(tCtlStatus);

    RetsetAllChannelStatus();

    GetRunStageTable();

	SetSeqByStageInfo();

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

	m_nBarrierPhaseDiffArrCount = 0;
	memset(m_nAllGreenStageChgLen, 0, sizeof(m_nAllGreenStageChgLen));
	memset(m_nGreenStageChgLen, 0, sizeof(m_nGreenStageChgLen));
	memset(m_nSplitTimeBeforeBarrier, 0, sizeof(m_nSplitTimeBeforeBarrier));

	memset(m_bKeepGreenChannelBeforeControlChannelFlag, false, sizeof(m_bKeepGreenChannelBeforeControlChannelFlag));
}

/*==================================================================== 
函数名 ：ReSetCtlStatus
功能 ：周期运行结束重置相位运行状态信息
算法实现 ： 
参数说明 ：
返回值说明：
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 李永萍          创建函数 
====================================================================*/ 
void CLogicCtlCablelessLine::ReSetCtlStatus()
{
    CLogicCtlFixedTime::ReSetCtlStatus();
    m_bCalcOffsetFlag = true;
}


/*==================================================================== 
函数名 ：OnePhaseRun 
功能 ：无电缆线控控制时单个机动车相位运行状态更新
算法实现 ： 
参数说明 ：nRingIndex，环索引 
返回值说明：
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 刘黎明          创建函数 
====================================================================*/ 
void CLogicCtlCablelessLine::OnePhaseRun(int nRingIndex)
{
    char szInfo[256] = {0};
    if (m_bCalcOffsetFlag)
    {
        m_nRealOffset = CalcRealOffset();
        m_bCalcOffsetFlag = false;
        m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "Calcute Real Offset is %d", m_nRealOffset);

		if (nRingIndex == 0)
		{
			if (m_nRealOffset != 0)
			{
				CalculateAllPhaseGreenChgLen();
				ResetAllPhaseGreenTime();
			}
			else
			{
				SetPhaseSplitTimeFromParam();
			}
		}
    }

    PTRingCtlInfo ptRingRunInfo = &(m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex]);
        
    int nIndex = m_tFixTimeCtlInfo.m_nCurPhaseIndex[nRingIndex];
    TPhaseLampClrRunCounter tRunCounter;
    m_pOpenATCRunStatus->GetPhaseLampClrRunCounter(tRunCounter);

    //当前相位阶段是否完成
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

		if (chcurStage == C_CH_PHASESTAGE_GF || (chcurStage == C_CH_PHASESTAGE_G && chNextStage == C_CH_PHASESTAGE_Y))
		{
			m_bChangeToYellow[nRingIndex] = true;
		}

        if (chNextStage == C_CH_PHASESTAGE_G)
        {
            m_nSplitTime[nRingIndex][nIndex] = ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPhaseStageRunTime + ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPhaseTime - ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPhaseGreenTime;
        }

        m_tFixTimeCtlInfo.m_bIsChgStage[nRingIndex] = false;

        sprintf(szInfo, "OnePhaseRun RingIndex:%d CurPhaseIndex:%d CurStage:%c NextStage:%c StageTime:%d", nRingIndex, nIndex, chcurStage, chNextStage, ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPhaseStageRunTime);
        m_pOpenATCLog->LogOneMessage(LEVEL_INFO, szInfo);

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
函数名 ：OnePedPhaseRun
功能 ：无电缆线控控制时单个行人相位运行状态更新
算法实现 ： 
参数说明 ：nRingIndex，环索引 
返回值说明：
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 刘黎明          创建函数 
====================================================================*/ 
void CLogicCtlCablelessLine::OnePedPhaseRun(int nRingIndex)
{
    PTRingCtlInfo ptRingRunInfo = &(m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex]);
        
    int nIndex = m_tFixTimeCtlInfo.m_nCurPhaseIndex[nRingIndex];
    TPhaseLampClrRunCounter tRunCounter;
	m_pOpenATCRunStatus->GetPhaseLampClrRunCounter(tRunCounter);
	char chcurPedStage = ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPedPhaseStage;

	//当前相位阶段是否完成
	if (chcurPedStage != C_CH_PHASESTAGE_GF && tRunCounter.m_nPedLampClrTime[nRingIndex] >= ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPedPhaseStageRunTime)
	{
		m_tFixTimeCtlInfo.m_bIsChgPedStage[nRingIndex] = true;
	}
	else if (chcurPedStage == C_CH_PHASESTAGE_GF && m_bChangeToYellow[nRingIndex])
	{
		m_tFixTimeCtlInfo.m_bIsChgPedStage[nRingIndex] = true;
		m_bChangeToYellow[nRingIndex] = false;
	}

    if (m_tFixTimeCtlInfo.m_bIsChgPedStage[nRingIndex])
    {
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
函数名 ：CalcRealOffset
功能 ：无电缆线控控制时计算实际相位差,修正相位绿灯时间
算法实现 ： 
参数说明 ：
返回值说明：
        当前实际的相位差，以0点作为统一基准.
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 刘黎明          创建函数 
====================================================================*/ 
short CLogicCtlCablelessLine::CalcRealOffset()
{
    int nYear,nMonth,nDay,nHour,nMin,nSec,nWeek;
    OpenATCGetCurTime(nYear,nMonth,nDay,nHour,nMin,nSec,nWeek);

    long wSecOffset = time(NULL);//(WORD)(nHour * C_N_SECONDS_PERHOUR + nMin * C_N_SECONDS_PERMIN + nSec);

    WORD wRealOffset = wSecOffset % m_tFixTimeCtlInfo.m_wCycleLen;

    if (wRealOffset == m_tFixTimeCtlInfo.m_wPhaseOffset)
    {
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

short CLogicCtlCablelessLine::CalcRealAdjVal(short nOffset,WORD wGreenTime,WORD wMinGreen,WORD wMaxGreen)
{
    short nTmpAdj = (short)(wGreenTime * C_F_ADJ_KEY);

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

void  CLogicCtlCablelessLine::CalculateAllPhaseGreenChgLen()
{
	m_nBarrierPhaseDiffArrCount = 0;
	memset(m_nAllGreenStageChgLen, 0, sizeof(m_nAllGreenStageChgLen));
	memset(m_nGreenStageChgLen, 0, sizeof(m_nGreenStageChgLen));
	memset(m_nSplitTimeBeforeBarrier, 0, sizeof(m_nSplitTimeBeforeBarrier));

	int  i = 0, j = 0, k = 0;
	int nTempPhaseDiff = 0;
    for (i = 0;i < m_tFixTimeCtlInfo.m_nRingCount;i++)
	{
		 PTRingCtlInfo ptRingRunInfo = &(m_tFixTimeCtlInfo.m_atPhaseSeq[i]);

		 m_nBarrierPhaseDiffArrCount = 0;
		 nTempPhaseDiff = m_nRealOffset;
		 for (j = 0;j < ptRingRunInfo->m_nPhaseCount;j++)
		 {
			 PTPhase pPhaseInfo = &(m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[j].m_tPhaseParam);

			 for (k = 0;k < MAX_PHASE_COUNT;k++)
			 {
			     if (m_atSplitInfo[k].m_bySplitPhase == m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[j].m_tPhaseParam.m_byPhaseNumber && m_atSplitInfo[k].m_bySplitMode != NEGLECT_MODE && m_atSplitInfo[k].m_bySplitMode != SHIELD_MODE)
				 {
					 m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[j].m_wPhaseTime = m_atSplitInfo[k].m_wSplitTime;
					 m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[j].m_wPhaseGreenTime = m_atSplitInfo[k].m_wSplitTime - pPhaseInfo->m_byGreenFlash - pPhaseInfo->m_byPhaseYellowChange - pPhaseInfo->m_byPhaseRedClear;
					 m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[j].m_wPedPhaseGreenTime = m_atSplitInfo[k].m_wSplitTime - pPhaseInfo->m_byPhaseYellowChange - pPhaseInfo->m_byPhasePedestrianClear - pPhaseInfo->m_byPhaseRedClear;
			         m_nSplitTimeBeforeBarrier[i][m_nBarrierPhaseDiffArrCount] += m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[j].m_wPhaseGreenTime;
					 break;
				 }
			 }

		     m_nGreenStageChgLen[i][j] = CalcRealAdjVal(nTempPhaseDiff,
														ptRingRunInfo->m_atPhaseInfo[j].m_wPhaseGreenTime,
														ptRingRunInfo->m_atPhaseInfo[j].m_tPhaseParam.m_wPhaseMinimumGreen,
														ptRingRunInfo->m_atPhaseInfo[j].m_tPhaseParam.m_wPhaseMaximum1);
			 nTempPhaseDiff -= m_nGreenStageChgLen[i][j];

			 m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "CalculateAllPhaseGreenChgLen RingIndex:%d PhaseIndex:%d GreenStageChgLen:%d", i, j, m_nGreenStageChgLen[i][j]);

			 m_nAllGreenStageChgLen[i][m_nBarrierPhaseDiffArrCount] += m_nGreenStageChgLen[i][j];
			 if (m_bLastPhaseBeforeBarrier[i][j])
			 {
			 	 m_nBarrierPhaseDiffArrCount += 1;
			 }
		 }
	}
}

void  CLogicCtlCablelessLine::ResetAllPhaseGreenTime()
{
	int nCyclePhaseDiff = 0;

	int nMinRingIndex = 0;
	int nMinGreenStageChgLen = 0;
	int nUsedGreenChgLen = 0;
	
	WORD wSplitTime	= 0;
	int i = 0, j = 0, k = 0;

	int nBarrierPhaseIndexRecord[MAX_RING_COUNT];
	memset(nBarrierPhaseIndexRecord, 0, sizeof(nBarrierPhaseIndexRecord));

	for (int nBarrierIndex = 0;nBarrierIndex < m_nBarrierPhaseDiffArrCount;nBarrierIndex++)
	{
		nMinRingIndex = 0;
	    nMinGreenStageChgLen = m_nAllGreenStageChgLen[0][nBarrierIndex];

		for (i = 1;i < m_tFixTimeCtlInfo.m_nRingCount;i++)
		{
			if (m_nAllGreenStageChgLen[i][nBarrierIndex] < nMinGreenStageChgLen)
			{
				nMinGreenStageChgLen = m_nAllGreenStageChgLen[i][nBarrierIndex];
				nMinRingIndex = i;//找出屏障前的绿灯调整长度和最小的环
			}
		}

		nCyclePhaseDiff += nMinGreenStageChgLen;

		m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "BarrierIndex is %d MinRingIndex is %d, MinGreenStageChgLen is %d CyclePhaseDiff is %d", nBarrierIndex, nMinRingIndex, nMinGreenStageChgLen, nCyclePhaseDiff);

		//调整绿灯时间
		nUsedGreenChgLen = 0;
		for (i = 0;i < m_tFixTimeCtlInfo.m_nRingCount;i++)
		{
		     PTRingCtlInfo ptRingRunInfo = &(m_tFixTimeCtlInfo.m_atPhaseSeq[i]);

			 if (i != nMinRingIndex)
			 {
				 for (j = nBarrierPhaseIndexRecord[i];j < ptRingRunInfo->m_nPhaseCount;j++)
				 {
					 if (m_bLastPhaseBeforeBarrier[i][j])
					 {
						 int nGreenTime = nMinGreenStageChgLen - nUsedGreenChgLen;

						 //屏障前的相位的绿灯时间用基准环的屏障前的总的绿灯调整长度减去非屏障前的总的绿灯调整时间，如果大于绿灯调整长度，则用绿灯调整长度，否则用计算值
						 if (abs(nGreenTime) > abs(m_nGreenStageChgLen[i][j]))
						 {
							 ptRingRunInfo->m_atPhaseInfo[j].m_wPhaseGreenTime -= m_nGreenStageChgLen[i][j];
						 }
						 else
						 {
							 ptRingRunInfo->m_atPhaseInfo[j].m_wPhaseGreenTime -= nGreenTime;
						 }

						 nBarrierPhaseIndexRecord[i] = j + 1;

						 wSplitTime = ptRingRunInfo->m_atPhaseInfo[j].m_wPhaseGreenTime + ptRingRunInfo->m_atPhaseInfo[j].m_tPhaseParam.m_byGreenFlash +
						 ptRingRunInfo->m_atPhaseInfo[j].m_tPhaseParam.m_byPhaseYellowChange +  ptRingRunInfo->m_atPhaseInfo[j].m_tPhaseParam.m_byPhaseRedClear;

						 SetPhaseSplitTime(i, j, wSplitTime, false);

						 m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "CalcRealAdjVal CurRingIndex:%d CurPhaseIndex:%d GreenStageChgLen:%d %d GreenTime:%d", i, j, m_nGreenStageChgLen[i][j], nGreenTime, ptRingRunInfo->m_atPhaseInfo[j].m_wPhaseGreenTime);
						 break;
					 }
					 else
					 {
						 int nSplitTime = 0;
						 for (k = 0;k < MAX_PHASE_COUNT;k++)
						 {
							 if (m_atSplitInfo[k].m_bySplitPhase == m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[j].m_tPhaseParam.m_byPhaseNumber)
							 {
								 nSplitTime = m_atSplitInfo[k].m_wSplitTime - m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[j].m_tPhaseParam.m_byGreenFlash - m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[j].m_tPhaseParam.m_byPhaseYellowChange - m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[j].m_tPhaseParam.m_byPhaseRedClear;
								 break;
							 }
						 }

						 int nTemp = ((int)(m_nAllGreenStageChgLen[nMinRingIndex][nBarrierIndex] * (float)nSplitTime / (float)m_nSplitTimeBeforeBarrier[i][nBarrierIndex] * 10)) % 10;
						 int nGreenTime = m_nAllGreenStageChgLen[nMinRingIndex][nBarrierIndex] * (float)nSplitTime / (float)m_nSplitTimeBeforeBarrier[i][nBarrierIndex];
						 if (nTemp != 0)
						 {
							 if (nTemp > 0)
							 {
								 nGreenTime += 1;
							 }
							 else
							 {
								 nGreenTime -= 1;
							 }

							 m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "1111 CurRingIndex:%d CurPhaseIndex:%d GreenStageChgLen:%d", i, j, nGreenTime);
						 }

						 //非屏障前的相位按比例分配的绿灯时间如果大于绿灯调整长度，则用绿灯调整长度，如果小于绿灯调整长度，则用计算值
						 if (abs(nGreenTime) > abs(m_nGreenStageChgLen[i][j]))
						 {
							 ptRingRunInfo->m_atPhaseInfo[j].m_wPhaseGreenTime -= m_nGreenStageChgLen[i][j];
							 nUsedGreenChgLen += m_nGreenStageChgLen[i][j];
						 }
						 else
						 {
							 ptRingRunInfo->m_atPhaseInfo[j].m_wPhaseGreenTime -= nGreenTime;
							 nUsedGreenChgLen += nGreenTime;
						 }

						 wSplitTime = ptRingRunInfo->m_atPhaseInfo[j].m_wPhaseGreenTime + ptRingRunInfo->m_atPhaseInfo[j].m_tPhaseParam.m_byGreenFlash +
							ptRingRunInfo->m_atPhaseInfo[j].m_tPhaseParam.m_byPhaseYellowChange +  ptRingRunInfo->m_atPhaseInfo[j].m_tPhaseParam.m_byPhaseRedClear;

						 SetPhaseSplitTime(i, j, wSplitTime, false);

						 m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "CalcRealAdjVal CurRingIndex:%d CurPhaseIndex:%d GreenStageChgLen:%d %d GreenTime:%d", i, j, m_nGreenStageChgLen[i][j], nGreenTime, ptRingRunInfo->m_atPhaseInfo[j].m_wPhaseGreenTime);
					 }
				 }
			 }
			 else
			 {
				 for (j = nBarrierPhaseIndexRecord[i];j < ptRingRunInfo->m_nPhaseCount;j++)
				 {
					 ptRingRunInfo->m_atPhaseInfo[j].m_wPhaseGreenTime -= m_nGreenStageChgLen[i][j];

					 wSplitTime = ptRingRunInfo->m_atPhaseInfo[j].m_wPhaseGreenTime + ptRingRunInfo->m_atPhaseInfo[j].m_tPhaseParam.m_byGreenFlash +
							ptRingRunInfo->m_atPhaseInfo[j].m_tPhaseParam.m_byPhaseYellowChange +  ptRingRunInfo->m_atPhaseInfo[j].m_tPhaseParam.m_byPhaseRedClear;

					 SetPhaseSplitTime(i, j, wSplitTime, false);

					 m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "CalcRealAdjVal CurRingIndex:%d CurPhaseIndex:%d GreenStageChgLen:%d GreenTime:%d", i, j, m_nGreenStageChgLen[i][j], ptRingRunInfo->m_atPhaseInfo[j].m_wPhaseGreenTime);

					 if (m_bLastPhaseBeforeBarrier[i][j])
					 {
						 nBarrierPhaseIndexRecord[i] = j + 1;
                         break;
					 }
				 }
			 }
		 }
	}
}

int CLogicCtlCablelessLine::SetPhaseSplitTime(int nRingIndex, int nPhaseIndex, WORD wSplitTime, bool bFlag)
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

void  CLogicCtlCablelessLine::SetPhaseSplitTimeFromParam()
{
	int  i = 0, j = 0, k = 0;
    for (i = 0;i < m_tFixTimeCtlInfo.m_nRingCount;i++)
	{
		 PTRingCtlInfo ptRingRunInfo = &(m_tFixTimeCtlInfo.m_atPhaseSeq[i]);

		 for (j = 0;j < ptRingRunInfo->m_nPhaseCount;j++)
		 {
			 PTPhase pPhaseInfo = &(m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[j].m_tPhaseParam);

			 for (k = 0;k < MAX_PHASE_COUNT;k++)
			 {
			     if (m_atSplitInfo[k].m_bySplitPhase == m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[j].m_tPhaseParam.m_byPhaseNumber && m_atSplitInfo[k].m_bySplitMode != NEGLECT_MODE && m_atSplitInfo[k].m_bySplitMode != SHIELD_MODE)
				 {
				     m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[j].m_wPhaseTime = m_atSplitInfo[k].m_wSplitTime;
					 m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[j].m_wPhaseGreenTime = m_atSplitInfo[k].m_wSplitTime - pPhaseInfo->m_byGreenFlash - pPhaseInfo->m_byPhaseYellowChange - pPhaseInfo->m_byPhaseRedClear;
					 m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[j].m_wPedPhaseGreenTime = m_atSplitInfo[k].m_wSplitTime - pPhaseInfo->m_byPhaseYellowChange - pPhaseInfo->m_byPhasePedestrianClear - pPhaseInfo->m_byPhaseRedClear;
					 break;
				 }
			 }
		 }
	}
}

int CLogicCtlCablelessLine::FloatToInt(float f)
{
    int i = 0;
    if (f > 0) //正数
	{
        i = (f * 10 + 5) / 10;
	}
    else if (f < 0) //负数
	{
        i = (f * 10 - 5) / 10;
	}
    else
	{
		i = 0;
	}
 
    return i;
}