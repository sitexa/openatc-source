/*=====================================================================
模块名 ：单点优化控制方式接口模块
文件名 ：LogicCtlOptim.cpp
相关文件：LogicCtlOptim.h,LogicCtlFixedTime.h
实现功能：单点优化控制方式实现
作者 ：刘黎明
版权 ：<Copyright(c) 2019-2020 Suzhou Keda Technology Co.,Ltd. All right reserved.>
-----------------------------------------------------------------------
修改记录：
日 期           版本     修改人     走读人     修改记录
2019/9/14       V1.0     刘黎明     刘黎明     创建模块
=====================================================================*/
#include "LogicCtlOptim.h"
#include <string.h>

CLogicCtlOptim::CLogicCtlOptim()
{

}

CLogicCtlOptim::~CLogicCtlOptim()
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
功能 ：单点优化控制方式类资源初始化
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
void CLogicCtlOptim::Init(COpenATCParameter * pParameter, COpenATCRunStatus * pRunStatus, COpenATCLog * pOpenATCLog, int nPlanNo)
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
    tCtlStatus.m_nCurCtlMode = CTL_MODE_SINGLEOPTIM;
    tCtlStatus.m_nCurPlanNo = (int)byPlanID;
    m_pOpenATCRunStatus->SetLogicCtlStatus(tCtlStatus);

    m_bCalcSplitTimeFlag = false;
    memset(&m_tOptimInfo,0,sizeof(TOptimInfo));
    memset(m_anDetectorTmpCounter,0,sizeof(m_anDetectorTmpCounter));
    memset(m_byDetectorTmpStatus,0,sizeof(m_byDetectorTmpStatus));

    RetsetAllChannelStatus();

    InitDetectorParam();

    InitOptimParam();

    GetRunStageTable();

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
函数名 ：OnePhaseRun 
功能 ：单点优化控制时单个机动车相位运行状态更新
算法实现 ： 
参数说明 ：nRingIndex，环索引 
返回值说明：
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 刘黎明          创建函数 
====================================================================*/ 
void CLogicCtlOptim::OnePhaseRun(int nRingIndex)
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
    }    

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
        //行人相位以机动车相位为准
        m_tFixTimeCtlInfo.m_bIsChgPedStage[nRingIndex] = true;       

        char chcurStage = ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseStage;
        PTPhaseCtlInfo pPhaseInfo = (PTPhaseCtlInfo)&(ptRingRunInfo->m_atPhaseInfo[nIndex]);
        WORD wStageTime = 0;
        char chNextStage =  this->GetNextPhaseStageInfo(chcurStage,pPhaseInfo,wStageTime);
        ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseStage = chNextStage;
        ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPhaseStageRunTime = wStageTime;

        if (chNextStage == C_CH_PHASESTAGE_G)
        {
            ResetPhaseOptimInfo(nRingIndex,nIndex);
        }

        m_tFixTimeCtlInfo.m_bIsChgStage[nRingIndex] = false;

        sprintf(szInfo, "OnePhaseRun RingIndex:%d CurPhaseIndex:%d CurStage:%c NextStage:%c StageTime:%d", nRingIndex, nIndex, chcurStage, chNextStage, wStageTime);
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

    if (ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseStage == C_CH_PHASESTAGE_G ||
        ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseStage == C_CH_PHASESTAGE_GF)
    {
        ProcPhaseOptimInfo(nRingIndex,nIndex);
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
功能 ：单点优化控制时单个行人相位运行状态更新
算法实现 ： 
参数说明 ：nRingIndex，环索引 
返回值说明：
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 刘黎明          创建函数 
====================================================================*/ 
void CLogicCtlOptim::OnePedPhaseRun(int nRingIndex)
{
    PTRingCtlInfo ptRingRunInfo = &(m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex]);
        
    int nIndex = m_tFixTimeCtlInfo.m_nCurPhaseIndex[nRingIndex];
    TPhaseLampClrRunCounter tRunCounter;
    m_pOpenATCRunStatus->GetPhaseLampClrRunCounter(tRunCounter);

    //当前相位阶段是否完成
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
            //m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "SetGreenLampPulse Ped RingIndex:%d PhaseIndex:%d bGreenPulse:%d", nRingIndex, nIndex, 1);
            bGreenPulse[nRingIndex] = true;
            SetGreenLampPulse(PED_CHA, ptRingRunInfo->m_atPhaseInfo[nIndex].m_tPhaseParam.m_byPhaseNumber, true);
        }
    }
    else
    {
        if (bGreenPulse[nRingIndex])
        {
            //m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "SetGreenLampPulse Ped RingIndex:%d PhaseIndex:%d bGreenPulse:%d", nRingIndex, nIndex, 0);
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
            //m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "SetRedLampPulse Ped RingIndex:%d PhaseIndex:%d bRedPulse:%d", nRingIndex, nIndex, 1);
            bRedPulse[nRingIndex] = true;
            SetRedLampPulse(PED_CHA, ptRingRunInfo->m_atPhaseInfo[nNextIndex].m_tPhaseParam.m_byPhaseNumber, true);  
        }
    } 
    else
    {
        if (bRedPulse[nRingIndex])
        {
            //m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "SetRedLampPulse Ped RingIndex:%d PhaseIndex:%d bRedPulse:%d", nRingIndex, nIndex, 0);
            bRedPulse[nRingIndex] = false;
            SetRedLampPulse(PED_CHA, ptRingRunInfo->m_atPhaseInfo[nNextIndex].m_tPhaseParam.m_byPhaseNumber, false);  
        }
    }
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
void CLogicCtlOptim::ReSetCtlStatus()
{
    CLogicCtlFixedTime::ReSetCtlStatus();
    m_bCalcSplitTimeFlag = true;
}   

/*==================================================================== 
函数名 ：CalcPhaseTime
功能 ：计算下一个周期的绿信比参数
算法实现 ： 
参数说明 ：
返回值说明：
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 刘黎明          创建函数 
====================================================================*/ 
void CLogicCtlOptim::CalcPhaseTime()
{
    int i = 0;
    int j = 0;
    int k = 0;

    int anPhaseDS[MAX_SEQUENCE_TABLE_COUNT];        //环内相位饱和度

    for (i = 0;i < m_tFixTimeCtlInfo.m_nRingCount;i ++)
    {
        int nTotalDS = 0;
        int nCycleLen = m_tFixTimeCtlInfo.m_wCycleLen;
        int nValidPhase = 0;
        memset(anPhaseDS,0,sizeof(anPhaseDS));
        int nCount = m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_nPhaseCount;
    
        for (j = 0;j < nCount;j++)
        {
            //只对能够计算饱和度的相位进行饱和度统计
            if (CalcPhaseDSInfo(i,j,anPhaseDS[j]))
            {
                nTotalDS += anPhaseDS[j];
                nValidPhase++;
            }
            else
            {
                nCycleLen -= m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[j].m_wPhaseTime;
            }
        }

        m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "CalcPhaseTime CycleLen:%d ValidPhase:%d TotalDS:%d", nCycleLen, nValidPhase, nTotalDS);   

        int nIndex = 1;
        int nLastSplit = nCycleLen;
        for (j = 0;j < nCount;j++)
        {
            //无法计算饱和度的相位使用预设的绿信比
            if (anPhaseDS[j] != 0)
            {
                if (nIndex < nValidPhase)
                {
                    WORD wSplitTime = nCycleLen * anPhaseDS[j] / nTotalDS;
                    PTPhase pPhaseInfo = &(m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[j].m_tPhaseParam);

                    if (wSplitTime < (pPhaseInfo->m_wPhaseMinimumGreen + pPhaseInfo->m_byGreenFlash + pPhaseInfo->m_byPhaseYellowChange + pPhaseInfo->m_byPhaseRedClear))
                    {
                        wSplitTime = pPhaseInfo->m_wPhaseMinimumGreen + pPhaseInfo->m_byGreenFlash + pPhaseInfo->m_byPhaseYellowChange + pPhaseInfo->m_byPhaseRedClear;
                    }
                    else if (wSplitTime > (pPhaseInfo->m_wPhaseMaximum1 + pPhaseInfo->m_byGreenFlash + pPhaseInfo->m_byPhaseYellowChange + pPhaseInfo->m_byPhaseRedClear))
                    {
                        wSplitTime = pPhaseInfo->m_wPhaseMaximum1 + pPhaseInfo->m_byGreenFlash + pPhaseInfo->m_byPhaseYellowChange + pPhaseInfo->m_byPhaseRedClear;
                    }

                    m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[j].m_wPhaseTime = wSplitTime;
                    m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[j].m_wPhaseGreenTime = wSplitTime - pPhaseInfo->m_byGreenFlash - pPhaseInfo->m_byPhaseYellowChange - pPhaseInfo->m_byPhaseRedClear;
                    m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[j].m_wPedPhaseGreenTime = wSplitTime - pPhaseInfo->m_byPhaseYellowChange - pPhaseInfo->m_byPhasePedestrianClear - pPhaseInfo->m_byPhaseRedClear;
                    nLastSplit -= wSplitTime;
                }
                else
                {
                    WORD wSplitTime = nLastSplit;
                    PTPhase pPhaseInfo = &(m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[j].m_tPhaseParam);

                    if (wSplitTime < (pPhaseInfo->m_wPhaseMinimumGreen + pPhaseInfo->m_byGreenFlash + pPhaseInfo->m_byPhaseYellowChange + pPhaseInfo->m_byPhaseRedClear))
                    {
                        wSplitTime = pPhaseInfo->m_wPhaseMinimumGreen + pPhaseInfo->m_byGreenFlash + pPhaseInfo->m_byPhaseYellowChange + pPhaseInfo->m_byPhaseRedClear;
                    }
                    else if (wSplitTime > (pPhaseInfo->m_wPhaseMaximum1 + pPhaseInfo->m_byGreenFlash + pPhaseInfo->m_byPhaseYellowChange + pPhaseInfo->m_byPhaseRedClear))
                    {
                        wSplitTime = pPhaseInfo->m_wPhaseMaximum1 + pPhaseInfo->m_byGreenFlash + pPhaseInfo->m_byPhaseYellowChange + pPhaseInfo->m_byPhaseRedClear;
                    }

                    m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[j].m_wPhaseTime = wSplitTime;
                    m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[j].m_wPhaseGreenTime = wSplitTime - pPhaseInfo->m_byGreenFlash - pPhaseInfo->m_byPhaseYellowChange - pPhaseInfo->m_byPhaseRedClear;
                    m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[j].m_wPedPhaseGreenTime = wSplitTime - pPhaseInfo->m_byPhaseYellowChange - pPhaseInfo->m_byPhasePedestrianClear - pPhaseInfo->m_byPhaseRedClear;
                }
                
                nIndex++;

                m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "CalcPhaseTime RingIndex:%d PhaseIndex:%d GreenTime:%d", i, j, m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[j].m_wPhaseGreenTime); 
            }
        }
    }
} 

/*==================================================================== 
函数名 ：CalcPhaseDSInfo
功能 ：计算上一个周期相位的饱和度
算法实现 ： 
参数说明 ：
        nRingIndex，环索引
        nPhaseIndex，相位索引
        nDS，饱和度(乘以100换算成整数)
返回值说明：
        计算成功返回true
        检测器故障无法计算返回false
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 刘黎明          创建函数 
====================================================================*/ 
bool CLogicCtlOptim::CalcPhaseDSInfo(int nRingIndex,int nPhaseIndex,int & nDS)
{
    PTRingCtlInfo ptRingRunInfo = &(m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex]);
    PTOnePhaseOptimInfo ptOnePhaseOptimInfo = &m_tOptimInfo.m_atRingOptimInfo[nRingIndex].m_atPhaseOptimInfo[nPhaseIndex];

    int i = 0;
    int j = 0;  
    int nValidDetCount = 0;
    int nTotalValidTime = 0;  
    for (i = 0;i < ptOnePhaseOptimInfo->m_nDetectorCount;i ++)
    {
        BYTE byDetID = ptOnePhaseOptimInfo->m_byDetectorID[i];

        if (ptOnePhaseOptimInfo->m_byDetectorStatus[i] == 0)//正常
        {
            nValidDetCount++;
            nTotalValidTime += ptOnePhaseOptimInfo->m_anValidPassTime[i];
        }
    }

    if (nValidDetCount == 0)
    {
        return false;
    }
    else
    {
        nDS = (nTotalValidTime * 100) / (nValidDetCount * ptOnePhaseOptimInfo->m_nTotalPassTime);
        m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "CalcPhaseDSInfo RingIndex:%d PhaseIndex:%d TotalValidTime:%d TotalPassTime:%d DS:%d", nRingIndex, nPhaseIndex, nTotalValidTime, ptOnePhaseOptimInfo->m_nTotalPassTime, nDS);
        return true; 
    }
}

/*==================================================================== 
函数名 ：InitOptimParam
功能 ：初始化优化需要的参数
算法实现 ： 
参数说明 ：
返回值说明：
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 刘黎明          创建函数 
====================================================================*/ 
void CLogicCtlOptim::InitOptimParam()
{
    int i = 0;
    int j = 0;
    int k = 0;

    m_tOptimInfo.m_nRingCount = m_tFixTimeCtlInfo.m_nRingCount;
    for (i = 0;i < m_tFixTimeCtlInfo.m_nRingCount;i ++)
    {
        int nCount = m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_nPhaseCount;
        m_tOptimInfo.m_atRingOptimInfo[i].m_nPhaseCount = nCount;
    
        for (j = 0;j < nCount;j++)
        {
            PTOnePhaseOptimInfo ptOnePhaseOptimInfo = &m_tOptimInfo.m_atRingOptimInfo[i].m_atPhaseOptimInfo[j];
            PTRingCtlInfo ptRingRunInfo = &(m_tFixTimeCtlInfo.m_atPhaseSeq[i]);

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

            m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "InitOptimParam RingIndex:%d PhaseIndex:%d TotalPassTime:%d", i, j, ptOnePhaseOptimInfo->m_nTotalPassTime); 
        }
    }
}

/*==================================================================== 
函数名 ：ResetPhaseOptimInfo
功能 ：重置相位优化参数
算法实现 ： 
参数说明 ：
        nRingIndex，环索引
        nPhaseIndex，相位索引
返回值说明：
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 刘黎明          创建函数 
====================================================================*/ 
void CLogicCtlOptim::ResetPhaseOptimInfo(int nRingIndex,int nPhaseIndex)
{
    PTOnePhaseOptimInfo ptOnePhaseOptimInfo = &m_tOptimInfo.m_atRingOptimInfo[nRingIndex].m_atPhaseOptimInfo[nPhaseIndex];
    int i = 0;    
    for (i = 0;i < ptOnePhaseOptimInfo->m_nDetectorCount;i ++)
    {
        ptOnePhaseOptimInfo->m_anValidPassTime[i] = 0;
        ptOnePhaseOptimInfo->m_byDetectorStatus[i] = 0;
    }
}

/*==================================================================== 
函数名 ：ProcPhaseOptimInfo
功能 ：计算相位相位优化参数
算法实现 ： 
参数说明 ：
        nRingIndex，环索引
        nPhaseIndex，相位索引
返回值说明：
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 刘黎明          创建函数 
====================================================================*/ 
void CLogicCtlOptim::ProcPhaseOptimInfo(int nRingIndex,int nPhaseIndex)
{
    PTRingCtlInfo ptRingRunInfo = &(m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex]);
    PTOnePhaseOptimInfo ptOnePhaseOptimInfo = &m_tOptimInfo.m_atRingOptimInfo[nRingIndex].m_atPhaseOptimInfo[nPhaseIndex];

    TRealTimeVehDetData tVehDetData;
    m_pOpenATCRunStatus->GetRTVehDetData(tVehDetData);
    int nGlobalCounter = m_pOpenATCRunStatus->GetGlobalCounter();

    int i = 0;
    int j = 0;    
    for (i = 0;i < ptOnePhaseOptimInfo->m_nDetectorCount;i ++)
    {
        BYTE byDetID = ptOnePhaseOptimInfo->m_byDetectorID[i];

        if (ptOnePhaseOptimInfo->m_byDetectorStatus[i] == 0)//正常
        {
            for (int j = 0;j < m_tFixTimeCtlInfo.m_nVehDetCount;j ++)
            {
                if (m_tFixTimeCtlInfo.m_atVehDetector[j].m_byVehicleDetectorNumber == byDetID)
                {
                    if (tVehDetData.m_bDetFaultStatus[j])//故障
                    {
                        //单个周期只要检测器曾经发生故障就把状态置为故障,计算绿信比时不使用该检测器
                        ptOnePhaseOptimInfo->m_byDetectorStatus[i] = 1;
                    }
                    break;
                }
            }    
        }

        if (ptOnePhaseOptimInfo->m_byDetectorStatus[i] == 1)//故障
        {
            break;
        }

        int nIndex = byDetID - 1;
        
        if (tVehDetData.m_chDetStatus[nIndex] == 1)//有车
        {
            if (m_byDetectorTmpStatus[nIndex] == 0)
            {
                m_byDetectorTmpStatus[nIndex] = 1;
                m_anDetectorTmpCounter[nIndex] = tVehDetData.m_anDetStatusCounter[nIndex];
            }
            else
            {
                ptOnePhaseOptimInfo->m_anValidPassTime[i] += C_N_TIMER_MILLSECOND * CalcCounter(m_anDetectorTmpCounter[nIndex],nGlobalCounter,C_N_MAXGLOBALCOUNTER);
                m_anDetectorTmpCounter[nIndex] = nGlobalCounter;
               
                //m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "ProcPhaseOptimInfo DetID:%d i:%d ValidPassTime:%d", byDetID, i, ptOnePhaseOptimInfo->m_anValidPassTime[i]); 
            }
        }
    }
}
