/*=====================================================================
模块名 ：感应控制方式接口模块
文件名 ：LogicCtlActuate.cpp
相关文件：LogicCtlActuate.h,LogicCtlFixedTime.h
实现功能：感应控制方式实现
作者 ：刘黎明
版权 ：<Copyright(c) 2019-2020 Suzhou Keda Technology Co.,Ltd. All right reserved.>
-----------------------------------------------------------------------
修改记录：
日 期           版本     修改人     走读人     修改记录
2019/9/14       V1.0     刘黎明     刘黎明     创建模块
=====================================================================*/
#include "LogicCtlActuate.h"
#include <string.h>

CLogicCtlActuate::CLogicCtlActuate()
{
}

CLogicCtlActuate::~CLogicCtlActuate()
{
}

/*==================================================================== 
函数名 ：Init 
功能 ：感应控制方式资源初始化
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
void CLogicCtlActuate::Init(COpenATCParameter * pParameter, COpenATCRunStatus * pRunStatus, COpenATCLog * pOpenATCLog, int nPlanNo)
{
    m_pOpenATCParameter = pParameter;
    m_pOpenATCRunStatus = pRunStatus;
	m_pOpenATCLog       = pOpenATCLog;

    memset(&m_tFixTimeCtlInfo,0,sizeof(m_tFixTimeCtlInfo));
    memset(&m_tRunStageInfo, 0x00, sizeof(m_tRunStageInfo));
    memset(&m_bLastPhaseBeforeBarrier, 0x00, sizeof(m_bLastPhaseBeforeBarrier));
	memset(m_nChannelSplitMode,0x00,sizeof(m_nChannelSplitMode)); 
	memset(m_bChangeToYellow, 0, sizeof(m_bChangeToYellow));
    memset(m_bShieldStatus, 0x00, sizeof(m_bShieldStatus));
    memset(m_bProhibitStatus, 0x00, sizeof(m_bProhibitStatus));
    memset(m_bOldShieldStatus, 0x00, sizeof(m_bOldShieldStatus));
    memset(m_bOldProhibitStatus, 0x00, sizeof(m_bOldProhibitStatus));
    memset(m_bySplitPhaseMode, 0x00, sizeof(m_bySplitPhaseMode));
    memset(m_nSplitPhaseTime, 0x00, sizeof(m_nSplitPhaseTime));

    //当前运行模式为单点感应控制
    m_nCurRunMode = CTL_MODE_ACTUATE;

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
    tCtlStatus.m_nCurCtlMode = CTL_MODE_ACTUATE;
    tCtlStatus.m_nCurPlanNo  = (int)byPlanID;
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
	memset(m_nGreenTimeIsMax1Time, 0, sizeof(m_nGreenTimeIsMax1Time));  
	memset(m_nGreenTimeNotExceedMax1Time, 0, sizeof(m_nGreenTimeNotExceedMax1Time));  
	memset(m_bGreenTimeExceedMax1Time, 0, sizeof(m_bGreenTimeExceedMax1Time)); 

	memset(m_bKeepGreenChannelBeforeControlChannelFlag, false, sizeof(m_bKeepGreenChannelBeforeControlChannelFlag));
}

/*==================================================================== 
函数名 ：OnePhaseRun 
功能 ：感应控制时单个机动车相位运行状态更新
算法实现 ： 
参数说明 ：nRingIndex，环索引 
返回值说明：
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 刘黎明          创建函数 
====================================================================*/ 
void CLogicCtlActuate::OnePhaseRun(int nRingIndex)
{
    char szInfo[256] = {0};
    PTRingCtlInfo ptRingRunInfo = &(m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex]);
        
    int nIndex = m_tFixTimeCtlInfo.m_nCurPhaseIndex[nRingIndex];
    TPhaseLampClrRunCounter tRunCounter;
    m_pOpenATCRunStatus->GetPhaseLampClrRunCounter(tRunCounter);

    //感应控制无需等待，所以相位完全忽略和部分忽略全都置为结束可等待
    if (ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseStage != C_CH_PHASESTAGE_F && (m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nIndex].m_chPhaseMode == NEGLECT_MODE || m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nIndex].m_chPhaseMode == SHIELD_MODE))
    {
        ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseStage = C_CH_PHASESTAGE_F;
    }

    //当前相位阶段是否完成
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

        if (chNextStage == C_CH_PHASESTAGE_G && !ProcPhaseDetStatus(nRingIndex,nIndex))
        {
            PTPhase pPhaseInfo = &(m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nIndex].m_tPhaseParam);
            ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPhaseStageRunTime = pPhaseInfo->m_wPhaseMinimumGreen;  
			//ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPedPhaseStageRunTime = pPhaseInfo->m_wPhaseMinimumGreen + pPhaseInfo->m_byGreenFlash - pPhaseInfo->m_byPhasePedestrianClear;  

            m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "RingIndex:%d PhaseIndex:%d Set Green RunTime To PhaseMinimumGreen:%d", nRingIndex, nIndex, pPhaseInfo->m_wPhaseMinimumGreen);                     
        }

        if (chNextStage == C_CH_PHASESTAGE_G)
        {
            m_nSplitTime[nRingIndex][nIndex] = ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPhaseStageRunTime + ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPhaseTime - ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPhaseGreenTime;
        }

        m_tFixTimeCtlInfo.m_bIsChgStage[nRingIndex] = false;

        sprintf(szInfo, "OnePhaseRun RingIndex:%d CurPhaseIndex:%d CurStage:%c NextStage:%c StageTime:%d", nRingIndex, nIndex, chcurStage, chNextStage, wStageTime);
        m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, szInfo);

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

    //判断是否需要延长绿灯时间
    if (ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseStage == C_CH_PHASESTAGE_G)
    {
        ProcExtendGreen(nRingIndex,nIndex,tRunCounter.m_nLampClrTime[nRingIndex]);
    }

	//屏障前的最后一个降级相位绿灯时，其并发相位增加延长绿到最大绿
	if (m_bLastPhaseBeforeBarrier[nRingIndex][nIndex] && ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseStage == C_CH_PHASESTAGE_G && ProcPhaseDetStatus(nRingIndex,nIndex))
	{
        //LastConcurrencyPhaseBeforeBarrierExtendGreen(nRingIndex,nIndex,tRunCounter.m_nLampClrTime[nRingIndex]);
	}
}

/*==================================================================== 
函数名 ：OnePedPhaseRun
功能 ：感应控制时单个行人相位运行状态更新
算法实现 ： 
参数说明 ：nRingIndex，环索引 
返回值说明：
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 刘黎明          创建函数 
====================================================================*/ 
void CLogicCtlActuate::OnePedPhaseRun(int nRingIndex)
{
    PTRingCtlInfo ptRingRunInfo = &(m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex]);
        
    int nIndex = m_tFixTimeCtlInfo.m_nCurPhaseIndex[nRingIndex];
    TPhaseLampClrRunCounter tRunCounter;
	m_pOpenATCRunStatus->GetPhaseLampClrRunCounter(tRunCounter);
	char chcurPedStage = ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPedPhaseStage;

    //感应控制无需等待，所以相位完全忽略和部分忽略全都置为结束可等待
    if (ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPedPhaseStage != C_CH_PHASESTAGE_F && (m_bySplitPhaseMode[nRingIndex][nIndex] == 1 || m_bySplitPhaseMode[nRingIndex][nIndex] == 2))
    {
        ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPedPhaseStage = C_CH_PHASESTAGE_F;
    }

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
        PTPhaseCtlInfo pPhaseInfo	= (PTPhaseCtlInfo)&(ptRingRunInfo->m_atPhaseInfo[nIndex]);
        WORD wPedStageTime			= 0;
        char chNextPedStage			= this->GetPedNextPhaseStageInfo(chcurPedStage,pPhaseInfo,wPedStageTime);
		ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPedPhaseStage = chNextPedStage;
        ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPedPhaseStageRunTime = wPedStageTime;

		if (chNextPedStage != C_CH_PHASESTAGE_G)
		{
			ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPedPhaseStageRunTime = wPedStageTime;
		}
		else
		{
			if (chNextPedStage == C_CH_PHASESTAGE_G && !ProcPhaseDetStatus(nRingIndex,nIndex))
			{
				PTPhase pPhaseInfo = &(m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nIndex].m_tPhaseParam);
				ptRingRunInfo->m_atPhaseInfo[nIndex].m_wPedPhaseStageRunTime = pPhaseInfo->m_wPhaseMinimumGreen + pPhaseInfo->m_byGreenFlash - pPhaseInfo->m_byPhasePedestrianClear;               
			}
		}

        m_tFixTimeCtlInfo.m_bIsChgPedStage[nRingIndex] = false;

        if (chNextPedStage == C_CH_PHASESTAGE_GF)
        {
            GetGreenFalshCount(PED_CHA, (PTPhaseCtlInfo)&(ptRingRunInfo->m_atPhaseInfo[nIndex]), wPedStageTime);
        }

		if (chcurPedStage != chNextPedStage)
		{
			m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL,  "OnePedPhaseRun RingIndex:%d CurPhaseIndex:%d CurPedStage:%c NextPedStage:%c PedStageTime:%d", nRingIndex, nIndex, chcurPedStage, chNextPedStage, wPedStageTime);

			//设置环内相位灯色，重置时间
			SetLampClrByRing(nRingIndex,nIndex,ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPhaseStage,ptRingRunInfo->m_atPhaseInfo[nIndex].m_chPedPhaseStage);
			tRunCounter.m_nPedLampClrTime[nRingIndex]			= 0;
			tRunCounter.m_nPedLampClrStartCounter[nRingIndex]	= tRunCounter.m_nCurCounter;    
			m_pOpenATCRunStatus->SetPhaseLampClrRunCounter(tRunCounter);
			m_bIsLampClrChg = true;
		}
    }
}

/*==================================================================== 
函数名 ：ProcExtendGreen
功能 ：处理相位是否需要延长绿灯时间直到最大绿
算法实现 ： 
参数说明 ：nRingIndex，当前运行的环索引 
           nPhaseIndex，当前运行的相位索引
           nCurRunTime，当前阶段的运行时间
返回值说明：
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 刘黎明          创建函数 
2020/03/18     V1.0 李永萍          其他环的屏障前的最后一个相位有车过时也增加绿灯时间
====================================================================*/ 
void CLogicCtlActuate::ProcExtendGreen(int nRingIndex,int nPhaseIndex,int nCurRunTime)
{
    PTRingCtlInfo ptRingRunInfo = &(m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex]);

    bool bVehCome = false;
    char szInfo[256] = {0};
	int nPhaseID = 0;

    TRealTimeVehDetData tVehDetData;
    m_pOpenATCRunStatus->GetRTVehDetData(tVehDetData);
        
    for (int i = 0;i < m_tFixTimeCtlInfo.m_nVehDetCount;i ++)
    {
        if (m_tFixTimeCtlInfo.m_atVehDetector[i].m_byVehicleDetectorCallPhase == 
            ptRingRunInfo->m_atPhaseInfo[nPhaseIndex].m_tPhaseParam.m_byPhaseNumber)
        {
            if (tVehDetData.m_bIsNewVehCome[i])
            {
                bVehCome = true;
                tVehDetData.m_bIsNewVehCome[i] = false;
            }
        }
    }        

    if (bVehCome)
    {
        PTPhase pPhaseInfo = &(ptRingRunInfo->m_atPhaseInfo[nPhaseIndex].m_tPhaseParam);
		nPhaseID = ptRingRunInfo->m_atPhaseInfo[nPhaseIndex].m_tPhaseParam.m_byPhaseNumber;

		//相位初始执行时间为相位最小绿
		ptRingRunInfo->m_atPhaseInfo[nPhaseIndex].m_wPhaseStageRunTime = nCurRunTime + pPhaseInfo->m_byPhasePassage;
		if (m_nGreenTimeIsMax1Time[nPhaseID - 1] >= GREEN_TIME_CALCULATE_TIME)
		{
			if (ptRingRunInfo->m_atPhaseInfo[nPhaseIndex].m_wPhaseStageRunTime > pPhaseInfo->m_wPhaseMaximum2)
			{
				ptRingRunInfo->m_atPhaseInfo[nPhaseIndex].m_wPhaseStageRunTime = pPhaseInfo->m_wPhaseMaximum2;
			}

			if (ptRingRunInfo->m_atPhaseInfo[nPhaseIndex].m_wPhaseStageRunTime >= pPhaseInfo->m_wPhaseMaximum1)
			{
				m_bGreenTimeExceedMax1Time[nPhaseID - 1] = true;
			}
		}
		else
		{
			if (ptRingRunInfo->m_atPhaseInfo[nPhaseIndex].m_wPhaseStageRunTime >= pPhaseInfo->m_wPhaseMaximum1)
			{
				ptRingRunInfo->m_atPhaseInfo[nPhaseIndex].m_wPhaseStageRunTime = pPhaseInfo->m_wPhaseMaximum1;
				m_bGreenTimeExceedMax1Time[nPhaseID - 1] = true;
			}
		}

		if (ptRingRunInfo->m_atPhaseInfo[nPhaseIndex].m_wPhaseStageRunTime < pPhaseInfo->m_wPhaseMinimumGreen)
		{
			ptRingRunInfo->m_atPhaseInfo[nPhaseIndex].m_wPhaseStageRunTime = pPhaseInfo->m_wPhaseMinimumGreen;
		}

		if (ptRingRunInfo->m_atPhaseInfo[nPhaseIndex].m_chPedPhaseStage == C_CH_PHASESTAGE_G)
		{
			ptRingRunInfo->m_atPhaseInfo[nPhaseIndex].m_wPedPhaseStageRunTime = ptRingRunInfo->m_atPhaseInfo[nPhaseIndex].m_wPhaseStageRunTime + 
																			ptRingRunInfo->m_atPhaseInfo[nPhaseIndex].m_tPhaseParam.m_byGreenFlash - 
																			ptRingRunInfo->m_atPhaseInfo[nPhaseIndex].m_tPhaseParam.m_byPhasePedestrianClear;
		}

        sprintf(szInfo, "ProcExtendGreen RingIndex:%d CurPhaseIndex:%d PhaseRunTime:%d MaxGreenTime:%d", nRingIndex, nPhaseIndex, ptRingRunInfo->m_atPhaseInfo[nPhaseIndex].m_wPhaseStageRunTime, pPhaseInfo->m_wPhaseMaximum1);
        m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, szInfo);

        m_pOpenATCRunStatus->SetRTVehDetData(tVehDetData);

        for (int i = 0;i < m_tFixTimeCtlInfo.m_nRingCount;i++)
        {
            if (i != nRingIndex)
            {
                int nDstPhaseIndex = m_tFixTimeCtlInfo.m_nCurPhaseIndex[i];

                TPhaseLampClrRunCounter tRunCounter;
                m_pOpenATCRunStatus->GetPhaseLampClrRunCounter(tRunCounter);

                if (m_bLastPhaseBeforeBarrier[i][nDstPhaseIndex] && m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_chPhaseStage == C_CH_PHASESTAGE_G)
				{
                    for (int n = 0; n < m_tFixTimeCtlInfo.m_nVehDetCount; n++)      //[暂定]如果屏障前最后一个相位的并发没有配置检测器或者配置的检测器故障或者配置的检测器不存在则跑配置的绿信比
                    {
                        if (m_tFixTimeCtlInfo.m_atVehDetector[n].m_byVehicleDetectorCallPhase ==
                            m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_tPhaseParam.m_byPhaseNumber && !tVehDetData.m_bDetFaultStatus[n] && tVehDetData.m_bVehDetExist[n])
                        {
							m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_wPhaseStageRunTime = tRunCounter.m_nLampClrTime[i] + pPhaseInfo->m_byPhasePassage;

							nPhaseID = m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_tPhaseParam.m_byPhaseNumber;
							if (m_nGreenTimeIsMax1Time[nPhaseID - 1] >= GREEN_TIME_CALCULATE_TIME)
							{
								if (m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_wPhaseStageRunTime > m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_tPhaseParam.m_wPhaseMaximum2)
								{
									m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_wPhaseStageRunTime = m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_tPhaseParam.m_wPhaseMaximum2;
								}

								if (m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_wPhaseStageRunTime >= m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_tPhaseParam.m_wPhaseMaximum1)
								{
									m_bGreenTimeExceedMax1Time[nPhaseID - 1] = true;
								}
							}
							else
							{
								if (m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_wPhaseStageRunTime >= m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_tPhaseParam.m_wPhaseMaximum1)
								{
									m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_wPhaseStageRunTime = m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_tPhaseParam.m_wPhaseMaximum1;
									m_bGreenTimeExceedMax1Time[nPhaseID - 1] = true;
								}
							}

							if (m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_wPhaseStageRunTime < m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_tPhaseParam.m_wPhaseMinimumGreen)
							{
								m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_wPhaseStageRunTime = m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_tPhaseParam.m_wPhaseMinimumGreen;
							}

							if (m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_chPedPhaseStage == C_CH_PHASESTAGE_G)
							{
								m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_wPedPhaseStageRunTime = m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_wPhaseStageRunTime +
									m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_tPhaseParam.m_byGreenFlash -
									m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_tPhaseParam.m_byPhasePedestrianClear;
							}

							sprintf(szInfo, "ProcExtendGreen DstRingIndex:%d DstPhaseIndex:%d DstPhaseRunTime:%d MaxGreenTime:%d", i, nPhaseIndex, m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_wPhaseStageRunTime, m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_tPhaseParam.m_wPhaseMaximum1);
							m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, szInfo);

							m_nSplitTime[i][nDstPhaseIndex] = m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_wPhaseStageRunTime + ptRingRunInfo->m_atPhaseInfo[nDstPhaseIndex].m_wPhaseTime - ptRingRunInfo->m_atPhaseInfo[nDstPhaseIndex].m_wPhaseGreenTime;
                        }
                    }
                }
            }
        }

        m_nSplitTime[nRingIndex][nPhaseIndex] = ptRingRunInfo->m_atPhaseInfo[nPhaseIndex].m_wPhaseStageRunTime + ptRingRunInfo->m_atPhaseInfo[nPhaseIndex].m_wPhaseTime - ptRingRunInfo->m_atPhaseInfo[nPhaseIndex].m_wPhaseGreenTime;
    }
}

/*==================================================================== 
函数名 ：LastConcurrencyPhaseBeforeBarrierExtendGreen
功能 ：屏障前的最后一个降级相位(没有配置线圈或线圈故障)的并发相位延长绿
算法实现 ： 
参数说明 ：nRingIndex，当前运行的环索引 
           nPhaseIndex，当前运行的相位索引
           nCurRunTime，当前阶段的运行时间
返回值说明：
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 刘黎明          创建函数 
====================================================================*/ 
void CLogicCtlActuate::LastConcurrencyPhaseBeforeBarrierExtendGreen(int nRingIndex,int nPhaseIndex,int nCurRunTime)
{
	int i = 0;
	int nPhaseID = 0;

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

                if (m_bLastPhaseBeforeBarrier[i][nDstPhaseIndex] && m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_chPhaseStage == C_CH_PHASESTAGE_G)
                {
                    m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_wPhaseStageRunTime = tRunCounter.m_nLampClrTime[i] + pPhaseInfo->m_byPhasePassage;
					nPhaseID = m_tFixTimeCtlInfo.m_atPhaseSeq[nRingIndex].m_atPhaseInfo[nPhaseIndex].m_tPhaseParam.m_byPhaseNumber;

					if (m_nGreenTimeIsMax1Time[nPhaseID - 1] >= GREEN_TIME_CALCULATE_TIME)
					{
						if (m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_wPhaseStageRunTime > m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_tPhaseParam.m_wPhaseMaximum2)
						{
							m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_wPhaseStageRunTime = m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_tPhaseParam.m_wPhaseMaximum2;
						}

						if (m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_wPhaseStageRunTime >= m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_tPhaseParam.m_wPhaseMaximum1)
						{
							m_bGreenTimeExceedMax1Time[nPhaseID - 1] = true;
						}
					}
					else
					{
						if (m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_wPhaseStageRunTime >= m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_tPhaseParam.m_wPhaseMaximum1)
						{
							m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_wPhaseStageRunTime = m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_tPhaseParam.m_wPhaseMaximum1;
							m_bGreenTimeExceedMax1Time[nPhaseID - 1] = true;
						}
					}

                    if (m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_wPhaseStageRunTime < m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_tPhaseParam.m_wPhaseMinimumGreen)
                    {
                        m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_wPhaseStageRunTime = m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_tPhaseParam.m_wPhaseMinimumGreen;
                    }

					if (m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_chPedPhaseStage == C_CH_PHASESTAGE_G)
					{
						m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_wPedPhaseStageRunTime = m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_wPhaseStageRunTime + 
																												m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_tPhaseParam.m_byGreenFlash - 
																												m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_tPhaseParam.m_byPhasePedestrianClear;
					}

                    m_nSplitTime[i][nDstPhaseIndex] = m_tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nDstPhaseIndex].m_wPhaseStageRunTime + ptRingRunInfo->m_atPhaseInfo[nDstPhaseIndex].m_wPhaseTime - ptRingRunInfo->m_atPhaseInfo[nDstPhaseIndex].m_wPhaseGreenTime;
                }
            }
        }
	}
}