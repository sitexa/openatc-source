/*=====================================================================
模块名 ：逻辑控制方式调度模块
文件名 ：OpenATCLogicCtlManager.cpp
相关文件：OpenATCLogicCtlManager.h
          LogicCtlMode.h
          OpenATCParameter.h
          OpenATCRunStatus.h
实现功能：用于记录整个主控模块的运行状态
作者 ：刘黎明
版权 ：<Copyright(c) 2019-2020 Suzhou Keda Technology Co.,Ltd. All right reserved.>
-----------------------------------------------------------------------
修改记录：
日 期           版本     修改人     走读人     修改记录
2019/9/14       V1.0     刘黎明     刘黎明      创建模块
=====================================================================*/

#include "OpenATCLogicCtlManager.h"

#include "../Include/OpenATCHardwareManager.h"
#include "../Include/CanBusManager.h"
#include "./logicctl/LogicCtlModeSimpleFactory.h"


COpenATCLogicCtlManager * COpenATCLogicCtlManager::s_pData;

COpenATCLogicCtlManager::COpenATCLogicCtlManager()
{
    memset(&m_tOldValidManualCmd,0,sizeof(TManualCmd)); 
    memset(&m_atNetConfig,0,sizeof(m_atNetConfig));
	memset(m_nLockChannelTransStatus,0,sizeof(m_nLockChannelTransStatus));
    memset(m_nLockChannelCounter,0, sizeof(m_nLockChannelCounter));
	memset(&m_tOldAscOnePlanChannelLockInfo,0,sizeof(m_tOldAscOnePlanChannelLockInfo));
	memset(&m_tOldChannelCheckInfo,0,sizeof(m_tOldChannelCheckInfo));
	m_nManualControlPatternStartTime = 0;
	m_nManualControlPatternDurationTime = 0;
	memset(&m_tOldPreemptCtlCmd,0,sizeof(m_tOldPreemptCtlCmd));
	m_tOldPreemptCtlCmd.m_byPreemptType = PREEMPT_TYPE_DEFAULT;
	m_bInvalidPhaseCmd = false;
	memset(&m_tInvalidPhaseLockPara, 0, sizeof(m_tInvalidPhaseLockPara));

	m_tPatternInterruptCmdTime = 0;
}

COpenATCLogicCtlManager::~COpenATCLogicCtlManager()
{

}

/*==================================================================== 
函数名 ：getInstance 
功能 ：返回单件类COpenATCMainCtlManager的实例指针 
算法实现 ： 
参数说明 ： 
返回值说明：单件类COpenATCMainCtlManager的实例指针
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 刘黎明          创建函数 
====================================================================*/ 
COpenATCLogicCtlManager * COpenATCLogicCtlManager::getInstance()
{
    if (s_pData == NULL)
    {
        s_pData = new COpenATCLogicCtlManager();
    }

    return s_pData;
}

/*==================================================================== 
函数名 ：Init 
功能 ：用于对类COpenATCLogicCtlManager的初始化操作 
算法实现 ： 
参数说明 ： pParameter，特征参数类指针
            pRunStatus，运行状态类指针
			pOpenATCLog，日志类指针
返回值说明：无
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 刘黎明          创建函数 
====================================================================*/ 
void COpenATCLogicCtlManager::Init(COpenATCParameter * pParameter,COpenATCRunStatus * pRunStatus, COpenATCLog * pOpenATCLog)
{
    if (pParameter != NULL)
    {
        m_pLogicCtlParam = pParameter;
    }

    if (pRunStatus != NULL)
    {
        m_pLogicCtlStatus = pRunStatus;
    }

	if (pOpenATCLog != NULL)
    {
        m_pOpenATCLog = pOpenATCLog;
    }

    m_nLogicCtlStage = CTL_STAGE_UNDEFINE;
    m_nCurPlanNo = 0;
    m_nCurCtlMode = CTL_MODE_UNDEFINE;
    m_bIsCtlModeChg = true;  
    m_pLogicCtlMode = NULL; 
    m_bFirstInitFlag = true;
    m_nCtlSource = CTL_SOURCE_SELF;

    m_pLogicCtlParam->GetNetCardsTable(m_atNetConfig);
	m_pLogicCtlParam->GetStartSequenceInfo(m_tAscStartSequenceInfo);
	if (m_tAscStartSequenceInfo.m_byStartYellowFlash == 0)
	{
        m_tAscStartSequenceInfo.m_byStartYellowFlash = TIME_STARTUP_FLASH;
	}
	if (m_tAscStartSequenceInfo.m_byStartAllRed == 0)
	{
        m_tAscStartSequenceInfo.m_byStartAllRed = TIME_STARTUP_RED;
	}

    m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "COpenATCLogicCtlManager Init!"); 
}

void COpenATCLogicCtlManager::Stop()
{

}

/*==================================================================== 
函数名 ：Work
功能 ：逻辑控制模块主流程函数
算法实现 ： 
参数说明 ：
返回值说明：无
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 刘黎明          创建函数 
====================================================================*/ 
void COpenATCLogicCtlManager::Work()
{
    if (m_pLogicCtlParam == NULL ||
        m_pLogicCtlStatus == NULL)
    {
        return;
    }

	ProcLampClrRunCounter();

	if (m_pLogicCtlStatus->GeFaultDetBoardControlStatus())
	{
		//信号机启动时，如果故障板正在控制，则信号机获得控制权以后先切到全红
		SetAllRedStage();
		m_pLogicCtlStatus->SetFaultDetBoardControlStatus(false);
	}
	
	StartUpTimeSequenceCtl();

	AfterStartUpTimeSequenceCtl();

	ProcGlobalRunStatus();

	if (m_nCurCtlMode != CTL_MODE_FLASH && m_nCurCtlMode != CTL_MODE_ALLRED && m_nCurCtlMode != CTL_MODE_OFF)
	{
		ProcLockChannelLampClr();
	}
}

/*==================================================================== 
函数名 ：StartUpTimeSequenceCtl
功能 ：用于实现启动时序控制，10秒黄闪，5秒全红，然后进入正常控制
算法实现 ： 
参数说明 ：无
返回值说明：无
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 刘黎明          创建函数 
====================================================================*/ 
void COpenATCLogicCtlManager::StartUpTimeSequenceCtl()
{
    if (m_nLogicCtlStage == CTL_STAGE_UNDEFINE)
    {
        TPhaseLampClrRunCounter tRunCounter;
        m_nLogicCtlStage = CTL_STAGE_STARTUP_FLASH;
        m_nCurCtlMode = CTL_MODE_FLASH;
        m_pLogicCtlStatus->GetPhaseLampClrRunCounter(tRunCounter);
        tRunCounter.m_nLampClrTime[0] = 0;
        tRunCounter.m_nLampClrStartCounter[0] = tRunCounter.m_nCurCounter;    
        m_pLogicCtlStatus->SetPhaseLampClrRunCounter(tRunCounter);

        TLogicCtlStatus tCtlStatus;
        m_pLogicCtlStatus->GetLogicCtlStatus(tCtlStatus);
        tCtlStatus.m_nRunStage = CTL_STAGE_STARTUP_FLASH;
        m_pLogicCtlStatus->SetLogicCtlStatus(tCtlStatus);

        if (m_pLogicCtlMode != NULL)
        {
            m_pLogicCtlMode->Release();
            delete m_pLogicCtlMode;
            m_pLogicCtlMode = NULL;
        }
        m_pLogicCtlMode = CLogicCtlModeSimpleFactory::Create(CTL_MODE_FLASH);
        m_pLogicCtlMode->Init(m_pLogicCtlParam,m_pLogicCtlStatus,m_pOpenATCLog,0);
        m_pLogicCtlMode->Run();
        m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "COpenATCLogicCtlManager Startup yellow flash!");

		//设置系统控制命令状态
		SetSystemControlStatus(CONTROL_FAILED, NO_SUPPORT_CONTROL_WAY, CONTROL_FAILED, NO_SUPPORT_CONTROL_WAY, CONTROL_FAILED, NO_SUPPORT_CONTROL_WAY,
			CONTROL_FAILED, NO_SUPPORT_CONTROL_WAY, CONTROL_FAILED, NO_SUPPORT_CONTROL_WAY);
		//设置优先控制命令状态
		SetPreemptControlStatus(CONTROL_FAILED, NO_SUPPORT_CONTROL_WAY);
    }
    else if (m_nLogicCtlStage == CTL_STAGE_STARTUP_FLASH)
    {
        StartUpFlashCtl();
    }
    else if (m_nLogicCtlStage == CTL_STAGE_STARTUP_RED)
    {
        StartUpAllRedCtl();
    }
}

/*==================================================================== 
函数名 ：StartUpFlashCtl
功能 ：用于实现启动时序黄闪控制
算法实现 ： 
参数说明 ：无
返回值说明：无
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 刘黎明          创建函数 
====================================================================*/ 
void COpenATCLogicCtlManager::StartUpFlashCtl()
{
    if (m_pLogicCtlMode != NULL)
    {
        m_pLogicCtlMode->Run();
    }

    TPhaseLampClrRunCounter tRunCounter;
    m_pLogicCtlStatus->GetPhaseLampClrRunCounter(tRunCounter);

    if (tRunCounter.m_nLampClrTime[0] >= m_tAscStartSequenceInfo.m_byStartYellowFlash)
    {
        m_nLogicCtlStage = CTL_STAGE_STARTUP_RED;
        m_nCurCtlMode = CTL_MODE_ALLRED;
        tRunCounter.m_nLampClrTime[0] = 0;
        tRunCounter.m_nLampClrStartCounter[0] = tRunCounter.m_nCurCounter;
        m_pLogicCtlStatus->SetPhaseLampClrRunCounter(tRunCounter);

        TLogicCtlStatus tCtlStatus;
        m_pLogicCtlStatus->GetLogicCtlStatus(tCtlStatus);
        tCtlStatus.m_nRunStage = CTL_STAGE_STARTUP_RED;
        m_pLogicCtlStatus->SetLogicCtlStatus(tCtlStatus);

        if (m_pLogicCtlMode != NULL)
        {
            m_pLogicCtlMode->Release();
            delete m_pLogicCtlMode;
            m_pLogicCtlMode = NULL;
        }
        m_pLogicCtlMode = CLogicCtlModeSimpleFactory::Create(CTL_MODE_ALLRED);
        m_pLogicCtlMode->Init(m_pLogicCtlParam,m_pLogicCtlStatus,m_pOpenATCLog,0);
        m_pLogicCtlMode->Run();
        m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "COpenATCLogicCtlManager Startup all red!");

		//设置系统控制命令状态
		SetSystemControlStatus(CONTROL_FAILED, NO_SUPPORT_CONTROL_WAY, CONTROL_FAILED, NO_SUPPORT_CONTROL_WAY, CONTROL_FAILED, NO_SUPPORT_CONTROL_WAY, 
			CONTROL_FAILED, NO_SUPPORT_CONTROL_WAY, CONTROL_FAILED, NO_SUPPORT_CONTROL_WAY);
		//设置优先控制命令状态
		SetPreemptControlStatus(CONTROL_FAILED, NO_SUPPORT_CONTROL_WAY);
    }
}

/*==================================================================== 
函数名 ：StartUpAllRedCtl
功能 ：用于实现启动时序全红控制
算法实现 ： 
参数说明 ：无
返回值说明：无
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 刘黎明          创建函数 
====================================================================*/ 
void COpenATCLogicCtlManager::StartUpAllRedCtl()
{
    if (m_pLogicCtlMode != NULL)
    {
        m_pLogicCtlMode->Run();
    }

    TPhaseLampClrRunCounter tRunCounter;
    m_pLogicCtlStatus->GetPhaseLampClrRunCounter(tRunCounter);
    if (tRunCounter.m_nLampClrTime[0] >= m_tAscStartSequenceInfo.m_byStartAllRed)
    {
        m_nLogicCtlStage = CTL_STAGE_SELFCTL;            
        tRunCounter.m_nLampClrTime[0] = 0;
        tRunCounter.m_nLampClrStartCounter[0] = tRunCounter.m_nCurCounter;
        m_pLogicCtlStatus->SetPhaseLampClrRunCounter(tRunCounter);

        TLogicCtlStatus tCtlStatus;
        m_pLogicCtlStatus->GetLogicCtlStatus(tCtlStatus);
        tCtlStatus.m_nRunStage = CTL_STAGE_SELFCTL;
        m_pLogicCtlStatus->SetLogicCtlStatus(tCtlStatus);
        m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "COpenATCLogicCtlManager Start Self Ctl!");

		//设置系统控制命令状态
		SetSystemControlStatus(CONTROL_SUCCEED, 0, CONTROL_SUCCEED, 0, CONTROL_SUCCEED, 0, CONTROL_FAILED, NO_SUPPORT_CONTROL_WAY, CONTROL_SUCCEED, 0);
		//设置优先控制命令状态
		SetPreemptControlStatus(CONTROL_SUCCEED, 0);
    }
}

/*==================================================================== 
函数名 ：AfterStartUpTimeSequenceCtl
功能 ：用于实现启动时序控制结束以后的正常控制
算法实现 ： 
参数说明 ：无
返回值说明：无
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 刘黎明          创建函数 
====================================================================*/ 
void COpenATCLogicCtlManager::AfterStartUpTimeSequenceCtl()
{
    if (m_nLogicCtlStage == CTL_STAGE_SELFCTL)
    {
        int nFaultLevel = ProcFault();
        if (nFaultLevel == CRITICAL_FAULT)
        {
            CriticalFaultRun();
        }
        else
        {
            SelfAndUsrCtlRun();
        }
    }
    else if (m_nLogicCtlStage == CTL_STAGE_FAULTFORCE)
    {
        if (m_pLogicCtlMode != NULL)
        {
            m_pLogicCtlMode->Run();              
        }
    }
}

/*==================================================================== 
函数名 ：SelfAndUsrCtlRun
功能 ：根据参数进行自主控制,响应用户干预
算法实现 ： 
参数说明 ：无
返回值说明：无
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 刘黎明          创建函数 
2020/05/25     V1.0 李永萍          增加了黄闪器触发控制流程 
====================================================================*/ 
void COpenATCLogicCtlManager::SelfAndUsrCtlRun()
{
    /*TOpenATCStatusInfo tOpenATCStatusInfo;
    memset(&tOpenATCStatusInfo, 0, sizeof(TOpenATCStatusInfo));
    m_pLogicCtlStatus->GetOpenATCStatusInfo(tOpenATCStatusInfo);
   
    if (tOpenATCStatusInfo.m_tYellowFlashStatusInfo.m_cTriggerStatus)
    {
        if (m_nCurCtlMode != CTL_MODE_FLASH)
        {
            m_pLogicCtlMode->Release();
            delete m_pLogicCtlMode;
            m_pLogicCtlMode = NULL;

            m_pLogicCtlMode = CLogicCtlModeSimpleFactory::Create(CTL_MODE_FLASH);
            m_nCurCtlMode = CTL_MODE_FLASH;
            m_pLogicCtlMode->Init(m_pLogicCtlParam,m_pLogicCtlStatus,m_pOpenATCLog,0);
            m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "Yellow Flash Trigger!");

            m_nCtlSource = CTL_SOURCE_YELLOWFLASHTRIGGER;
        } 

        m_pLogicCtlMode->Run();
    }
    else */if (LocalUsrCtlRun())
    {

    }
    else if (SystemUsrCtlRun())
    {
    
    }
	else if (PreemptCtlRun())
	{

	}
    else
    {
        SelfRun();
    }
}

/*==================================================================== 
函数名 ：LocalUsrCtlRun
功能 ：本地用户干预，优先级最高
算法实现 ： 
参数说明 ：无
返回值说明：无
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 刘黎明          创建函数 
====================================================================*/ 
bool COpenATCLogicCtlManager::LocalUsrCtlRun()
{
    bool bRet = true;

	TManualCmd  tManualCmd;
	memset(&tManualCmd,0,sizeof(tManualCmd));
	m_pLogicCtlStatus->GetManualCmd(tManualCmd); 

    //手动按钮被按下 
    if (tManualCmd.m_nCmdSource == CTL_SOURCE_LOCAL && tManualCmd.m_bNewCmd)
    { 
		TManualCmd  tValidManualCmd;
	    memset(&tValidManualCmd,0,sizeof(tValidManualCmd));
		m_pLogicCtlStatus->GetValidManualCmd(tValidManualCmd); 

		//自动控制按钮被按下
		if (tManualCmd.m_bPatternInterruptCmd && tManualCmd.m_tPatternInterruptCmd.m_nControlMode == CTL_MODE_SELFCTL)
		{
			//生成有效的控制指令
			CreateValidManualCmd(tManualCmd, tValidManualCmd);
			SetPanelBtnStatusReply(tValidManualCmd, HWPANEL_BTN_AUTO);
		}
		else
		{
			if (m_nCtlSource == CTL_SOURCE_LOCAL)//已经进入本地控制
			{
				if (tManualCmd.m_bPatternInterruptCmd && tManualCmd.m_tPatternInterruptCmd.m_nControlMode == CTL_MODE_ALLRED)
				{ 
					if (m_nCtlSource == CTL_SOURCE_PREEMPT)
					{
						m_pLogicCtlStatus->DelteAllPreemptCtlCmdFromList();
					}

					//生成有效的控制指令
					CreateValidManualCmd(tManualCmd, tValidManualCmd);
					UsrCtlAllRed(tValidManualCmd); 
				}
				else  if (tManualCmd.m_bPatternInterruptCmd && tManualCmd.m_tPatternInterruptCmd.m_nControlMode == CTL_MODE_FLASH)
				{
					if (m_nCtlSource == CTL_SOURCE_PREEMPT)
					{
						m_pLogicCtlStatus->DelteAllPreemptCtlCmdFromList();
					}

					//生成有效的控制指令
					CreateValidManualCmd(tManualCmd, tValidManualCmd);
					UsrCtlYellowFalsh(tValidManualCmd);   
				}
				else if (tManualCmd.m_bStepForwardCmd)
				{
					UsrCtlStepForward(tManualCmd, tValidManualCmd);
				}
				else if (tManualCmd.m_bDirectionCmd)
				{
					UsrCtlDirectionKey(tManualCmd, tValidManualCmd);
				}
			}
			else    //未进入本地手动控制，在这里进入本地手动控制
			{
				if (m_nCurCtlMode != CTL_MODE_MANUAL)//有可能是从自动还没有结束又切到手动
				{
					tValidManualCmd.m_nCtlMode = CTL_MODE_MANUAL;
					tValidManualCmd.m_nSubCtlMode = CTL_MANUAL_SUBMODE_PANEL_FITSTMANUAL;//子模式设置为第一次按下手动
				}
				else
				{
					//1.在方向的时候按了自动，方向还没有结束切到自动，又按下手动，子模式保留
			        //2.在方向的时候按了自动，切到自动以后，自动还没有结束，又按下手动，子模式设置为第一次按下手动
					//3.在系统控制的时候按了面板手动，子模式设置为第一次按下手动
					//4.在步进的时候按了自动，切到自动以后，自动还没有结束，又按下手动，子模式设置为第一次按下手动
					if (tValidManualCmd.m_nSubCtlMode == CTL_MANUAL_SUBMODE_DEFAULT || m_tOldValidManualCmd.m_nCurCtlSource == CTL_SOURCE_SYSTEM ||
						tValidManualCmd.m_nSubCtlMode == CTL_MANUAL_SUBMODE_PANEL_STEPFOWARD && m_tOldValidManualCmd.m_bPatternInterruptCmd)
					{
						tValidManualCmd.m_nCtlMode = CTL_MODE_MANUAL;
						tValidManualCmd.m_nSubCtlMode = CTL_MANUAL_SUBMODE_PANEL_FITSTMANUAL;//子模式设置为第一次按下手动
					}
				}
				tValidManualCmd.m_nCmdSource = CTL_SOURCE_LOCAL;
				tValidManualCmd.m_nCurCtlSource = CTL_SOURCE_LOCAL;
				memcpy(tValidManualCmd.m_szPeerIp,tManualCmd.m_szPeerIp,strlen(tManualCmd.m_szPeerIp));
				tValidManualCmd.m_bNewCmd = true;
				tValidManualCmd.m_bStepForwardCmd = tManualCmd.m_bStepForwardCmd;
				memcpy(&tValidManualCmd.m_tStepForwardCmd,&tManualCmd.m_tStepForwardCmd,sizeof(TStepForwardCmd));
				tValidManualCmd.m_bDirectionCmd = tManualCmd.m_bDirectionCmd;
				memcpy(&tValidManualCmd.m_tDirectionCmd,&tManualCmd.m_tDirectionCmd,sizeof(TDirectionCmd));
				tValidManualCmd.m_bPatternInterruptCmd = tManualCmd.m_bDirectionCmd;
				memcpy(&tValidManualCmd.m_tPatternInterruptCmd,&tManualCmd.m_tPatternInterruptCmd,sizeof(TPatternInterruptCmd));
				m_pLogicCtlStatus->SetValidManualCmd(tValidManualCmd);
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "CreateValidManualCmd, Trans To Panel First Manual");

				SetPanelBtnStatusReply(tValidManualCmd, HWPANEL_BTN_MANUAL);

				m_nCtlSource = CTL_SOURCE_LOCAL; 
				
				if (m_nCurCtlMode != CTL_MODE_FLASH && m_nCurCtlMode != CTL_MODE_ALLRED && m_nCurCtlMode != CTL_MODE_OFF && m_nCurCtlMode != CTL_MODE_MANUAL)
				{
					InitUsrCtlLogicObj();
					m_nCurCtlMode = CTL_MODE_MANUAL;
				}

				m_pLogicCtlMode->SetUsrCtlFlag(true); 

				//设置系统控制命令状态
				SetSystemControlStatus(CONTROL_FAILED, HIGH_PRIORITY_USER_CONTROL_NO_EXECUT, CONTROL_FAILED, HIGH_PRIORITY_USER_CONTROL_NO_EXECUT,
					CONTROL_FAILED, HIGH_PRIORITY_USER_CONTROL_NO_EXECUT, CONTROL_FAILED, HIGH_PRIORITY_USER_CONTROL_NO_EXECUT,
					CONTROL_FAILED, HIGH_PRIORITY_USER_CONTROL_NO_EXECUT);
				//设置优先控制命令状态
		        SetPreemptControlStatus(CONTROL_FAILED, HIGH_PRIORITY_USER_CONTROL_NO_EXECUT);
			} 
		}

		//清除指令
		memset(&tManualCmd,0,sizeof(tManualCmd));
	    m_pLogicCtlStatus->SetManualCmd(tManualCmd); 
    }

	if (m_nCtlSource == CTL_SOURCE_LOCAL)
	{
		if (m_tOldValidManualCmd.m_bPatternInterruptCmd && m_tOldValidManualCmd.m_tPatternInterruptCmd.m_nControlMode == CTL_MODE_SELFCTL)
		{
			TManualCmd  tValidManualCmd;
			memset(&tValidManualCmd,0,sizeof(tValidManualCmd));
			m_pLogicCtlStatus->GetValidManualCmd(tValidManualCmd); 
			if (tValidManualCmd.m_nSubCtlMode == CTL_MANUAL_SUBMODE_PANEL_DIRECTION)
			{
				m_pLogicCtlMode->Run();
			}
			else
			{
				bRet = false;
			}
		}
		else
		{
			m_pLogicCtlMode->Run();
		}
	}
	else
	{
		bRet = false;
	}
     
    return bRet;
}

/*==================================================================== 
函数名 ：SystemUsrCtlRun
功能 ：系统用户干预,优先级低于本地用户干预
算法实现 ： 
参数说明 ：无
返回值说明：无
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 刘黎明          创建函数 
2020/02/28     V1.0 李永萍          添加其他控制模式切到非自主模式 
====================================================================*/ 
bool COpenATCLogicCtlManager::SystemUsrCtlRun()
{
    bool bRet = true;
    char szInfo[256] = {0};

	TManualCmd  tManualCmd;
	memset(&tManualCmd,0,sizeof(tManualCmd));
	m_pLogicCtlStatus->GetManualCmd(tManualCmd);

	if (tManualCmd.m_nCmdSource == CTL_SOURCE_SYSTEM && tManualCmd.m_bNewCmd)//判断是否有新的系统命令
	{
		TManualCmd  tValidManualCmd;
	    memset(&tValidManualCmd,0,sizeof(tValidManualCmd));
		m_pLogicCtlStatus->GetValidManualCmd(tValidManualCmd); 

		//判断系统命令能否使用
		if (!CheckSystemCmdUseStatus(tManualCmd, tValidManualCmd))
		{
			//清除指令
			memset(&tManualCmd,0,sizeof(tManualCmd));
			m_pLogicCtlStatus->SetManualCmd(tManualCmd); 
			bRet = true;
			return bRet;
		}
		
		if (tManualCmd.m_bPatternInterruptCmd)
		{
		    if (tManualCmd.m_tPatternInterruptCmd.m_nControlMode == CTL_MODE_FLASH)
			{
				if (m_nCtlSource == CTL_SOURCE_PREEMPT)
				{
					m_pLogicCtlStatus->DelteAllPreemptCtlCmdFromList();
				}

				if (m_nCurCtlMode != CTL_MODE_FLASH)
				{
					//创建黄闪模式
          			CreateCtlMode(CTL_MODE_FLASH, CTL_SOURCE_SYSTEM);
					//生成有效的控制指令
					CreateValidManualCmd(tManualCmd, tValidManualCmd);
					//置系统控制命令状态
					SetSystemControlStatus(CONTROL_SUCCEED, 0, CONTROL_SUCCEED, 0, CONTROL_FAILED, NO_EXIST_PATTERN_NO, CONTROL_FAILED, NO_SUPPORT_CONTROL_WAY, CONTROL_SUCCEED, 0);
					//设置优先控制命令状态
		            SetPreemptControlStatus(CONTROL_FAILED, HIGH_PRIORITY_USER_CONTROL_NO_EXECUT);
				}
			}
			else if (tManualCmd.m_tPatternInterruptCmd.m_nControlMode == CTL_MODE_ALLRED)
			{
				if (m_nCtlSource == CTL_SOURCE_PREEMPT)
				{
					m_pLogicCtlStatus->DelteAllPreemptCtlCmdFromList();
				}

				if (m_nCurCtlMode != CTL_MODE_ALLRED)
				{
					//创建全红模式
					CreateCtlMode(CTL_MODE_ALLRED, CTL_SOURCE_SYSTEM);
					//生成有效的控制指令
					CreateValidManualCmd(tManualCmd, tValidManualCmd);
					//置系统控制命令状态
					SetSystemControlStatus(CONTROL_SUCCEED, 0, CONTROL_SUCCEED, 0, CONTROL_FAILED, NO_EXIST_PATTERN_NO, CONTROL_FAILED, NO_SUPPORT_CONTROL_WAY, CONTROL_SUCCEED, 0);
					//设置优先控制命令状态
		            SetPreemptControlStatus(CONTROL_FAILED, HIGH_PRIORITY_USER_CONTROL_NO_EXECUT);
				}
			}
			else if (tManualCmd.m_tPatternInterruptCmd.m_nControlMode == CTL_MODE_OFF)
			{
				if (m_nCtlSource == CTL_SOURCE_PREEMPT)
				{
					m_pLogicCtlStatus->DelteAllPreemptCtlCmdFromList();
				}

				if (m_nCurCtlMode != CTL_MODE_OFF)
				{
					//创建关灯模式
          			CreateCtlMode(CTL_MODE_OFF, CTL_SOURCE_SYSTEM);
					//生成有效的控制指令
					CreateValidManualCmd(tManualCmd, tValidManualCmd);
					//置系统控制命令状态
					SetSystemControlStatus(CONTROL_SUCCEED, 0, CONTROL_SUCCEED, 0, CONTROL_FAILED, NO_EXIST_PATTERN_NO, CONTROL_FAILED, NO_SUPPORT_CONTROL_WAY, CONTROL_SUCCEED, 0);
					//设置优先控制命令状态
		            SetPreemptControlStatus(CONTROL_FAILED, HIGH_PRIORITY_USER_CONTROL_NO_EXECUT);
				}
			}
			else //切到自主或者带方案的其他控制方式，如定周期，感应,通道检测等
			{
				if (m_nCtlSource == CTL_SOURCE_PREEMPT)//优化控制时，系统下发了方案干预指令
				{
					CreatePatternInterruptCmdInPreemptControl(tManualCmd);
				}
				else
				{
					m_nCtlSource = CTL_SOURCE_SYSTEM;
					m_pLogicCtlMode->SetSystemUsrCtlFlag(true);  
					//生成有效的控制指令
					CreateValidManualCmd(tManualCmd, tValidManualCmd);
					//置系统控制命令状态
					SetSystemControlStatus(CONTROL_SUCCEED, 0, CONTROL_SUCCEED, 0, CONTROL_SUCCEED, 0, CONTROL_FAILED, NO_SUPPORT_CONTROL_WAY, CONTROL_SUCCEED, 0);
					//设置优先控制命令状态
					SetPreemptControlStatus(CONTROL_SUCCEED, 0);
				}
			}
		}
		else if (tManualCmd.m_bStepForwardCmd || tManualCmd.m_bChannelLockCmd)
		{
			if (m_nCurCtlMode != CTL_MODE_ALLRED && m_nCurCtlMode != CTL_MODE_FLASH && m_nCurCtlMode != CTL_MODE_OFF &&
				m_nCurCtlMode != CTL_MODE_MANUAL)
			{
				CLogicCtlMode * pMode = CLogicCtlModeSimpleFactory::Create(CTL_MODE_MANUAL);
				m_nCurCtlMode = CTL_MODE_MANUAL;
				pMode->Init(m_pLogicCtlParam,m_pLogicCtlStatus,m_pOpenATCLog,0);
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "System Usr Ctl Manual!");

				void * pParam = m_pLogicCtlMode->GetCurCtlParam();
				void * pLampClr = m_pLogicCtlMode->GetCurLampClr();
				void * pStageTable = m_pLogicCtlMode->GetCurStageTable();
				void * pChannelSplitMode = m_pLogicCtlMode->GetCurChannelSplitMode();
				pMode->SetCtlDerivedParam(pParam,pLampClr,pStageTable,pChannelSplitMode);

				m_pLogicCtlMode->Release();
				delete m_pLogicCtlMode;
				m_pLogicCtlMode = pMode;
			}

			m_nCtlSource = CTL_SOURCE_SYSTEM;
			m_pLogicCtlMode->SetSystemUsrCtlFlag(true); 

			//生成有效的控制指令
			CreateValidManualCmd(tManualCmd, tValidManualCmd);
			//置系统控制命令状态
			if (tManualCmd.m_bStepForwardCmd)
			{
				SetSystemControlStatus(CONTROL_SUCCEED, 0, CONTROL_SUCCEED, 0, CONTROL_SUCCEED, 0, CONTROL_SUCCEED, 0, CONTROL_SUCCEED, 0);
			}
			else
			{
				SetSystemControlStatus(CONTROL_SUCCEED, 0, CONTROL_SUCCEED, 0, CONTROL_FAILED, NO_SUPPORT_CONTROL_WAY, CONTROL_FAILED, NO_SUPPORT_CONTROL_WAY, CONTROL_SUCCEED, 0);
			}
			//设置优先控制命令状态
		    SetPreemptControlStatus(CONTROL_FAILED, HIGH_PRIORITY_USER_CONTROL_NO_EXECUT);
		}
		else if (tManualCmd.m_bPreemptCtlCmd)
		{
			if (m_nCurCtlMode != CTL_MODE_MANUAL)
			{
				m_nCtlSource = CTL_SOURCE_PREEMPT;
			}
		}

		memcpy(&m_tOldValidManualCmd,&tValidManualCmd,sizeof(TManualCmd));
	
		//清除指令
		memset(&tManualCmd,0,sizeof(tManualCmd));
	    m_pLogicCtlStatus->SetManualCmd(tManualCmd); 
	}

	if (m_nCtlSource == CTL_SOURCE_SYSTEM)
	{
		if (m_tOldValidManualCmd.m_bPatternInterruptCmd)
		{
			if (m_tOldValidManualCmd.m_tPatternInterruptCmd.m_nControlMode != CTL_MODE_SELFCTL)
			{
				//用户干预和通道检测的时候，需要进行方案干预处理
				SystemAskPlanRun();
			}
			else 
			{
				bRet = false;//回到自主
			}
		}
		else
		{
			if (m_nCurCtlMode != CTL_MODE_MANUAL)
			{
				TMainCtlBoardRunStatus tMainCtlBoardRunStatus;
				m_pLogicCtlStatus->GetMainCtlBoardRunStatus(tMainCtlBoardRunStatus);
				if (tMainCtlBoardRunStatus.m_bIsNeedGetParam)
				{
                    SystemAskPlanRun();
					//tMainCtlBoardRunStatus.m_bIsNeedGetParam = false;
					//m_pLogicCtlStatus->SetMainCtlBoardRunStatus(tMainCtlBoardRunStatus);
				}
			}
			m_pLogicCtlMode->Run();

			SwitchManualControlPatternToSelf();
		}
	}
	else
	{
		bRet = false;//回到自主
	}

    return bRet;      
}

/*==================================================================== 
函数名 ：PreemptCtlRun
功能 ：优先控制,优先级低于本地用户干预和系统干预
算法实现 ： 
参数说明 ：无
返回值说明：无
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 刘黎明          创建函数 
2020/02/28     V1.0 李永萍          添加其他控制模式切到非自主模式 
====================================================================*/ 
bool COpenATCLogicCtlManager::PreemptCtlRun()
{
    bool bRet = true;
    char szInfo[256] = {0};

	TPreemptCtlCmd  tPreemptCtlCmd;
	memset(&tPreemptCtlCmd,0,sizeof(tPreemptCtlCmd));
	m_pLogicCtlStatus->GetPreemptCtlCmd(tPreemptCtlCmd);

	if (tPreemptCtlCmd.m_bNewCmd)//判断是否有新的优先控制命令
	{
		//判断优先控制命令能否使用
		if (!CheckPreemptCmdUseStatus(tPreemptCtlCmd))
		{
			//清除指令
			memset(&tPreemptCtlCmd,0,sizeof(tPreemptCtlCmd));
			m_pLogicCtlStatus->SetPreemptCtlCmd(tPreemptCtlCmd); 
			bRet = true;
			return bRet;
		}

		if (!tPreemptCtlCmd.m_bPatternInterruptCmd)
		{
			SetPreemptStageIndex(tPreemptCtlCmd.m_byPreemptPhaseID, tPreemptCtlCmd.m_byPreemptStageIndex);//设置相位对应的阶段号
		}

		if (tPreemptCtlCmd.m_bPatternInterruptCmd && tPreemptCtlCmd.m_nCmdSource == CTL_SOURCE_PREEMPT)
		{
			//CLogicCtlPreempt生成回到自主的命令，不需要放到优先控制命令列表中
		}
		else
		{
			tPreemptCtlCmd.m_bNewCmd = false;
			m_pLogicCtlStatus->SePreemptCtlCmdList(tPreemptCtlCmd);
		}

		if (m_nCurCtlMode != CTL_MODE_ALLRED && m_nCurCtlMode != CTL_MODE_FLASH && m_nCurCtlMode != CTL_MODE_OFF &&
			m_nCurCtlMode != CTL_MODE_PREEMPT)
		{
			CLogicCtlMode * pMode = CLogicCtlModeSimpleFactory::Create(CTL_MODE_PREEMPT);
			m_nCurCtlMode = CTL_MODE_PREEMPT;
			pMode->Init(m_pLogicCtlParam,m_pLogicCtlStatus,m_pOpenATCLog,0);
			m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "Preempt Usr Ctl Manual!");

			void * pParam = m_pLogicCtlMode->GetCurCtlParam();
			void * pLampClr = m_pLogicCtlMode->GetCurLampClr();
			void * pStageTable = m_pLogicCtlMode->GetCurStageTable();
			void * pChannelSplitMode = m_pLogicCtlMode->GetCurChannelSplitMode();
			pMode->SetCtlDerivedParam(pParam,pLampClr,pStageTable,pChannelSplitMode);

			m_pLogicCtlMode->Release();
			delete m_pLogicCtlMode;
			m_pLogicCtlMode = pMode;
		}

		m_nCtlSource = CTL_SOURCE_PREEMPT;
		m_pLogicCtlMode->SetPreemptCtlFlag(true); 

		//设置优先控制命令状态
		SetPreemptControlStatus(CONTROL_SUCCEED, 0);

		memcpy(&m_tOldPreemptCtlCmd,&tPreemptCtlCmd,sizeof(TPreemptCtlCmd));

		//清除指令
		memset(&tPreemptCtlCmd,0,sizeof(tPreemptCtlCmd));
	    m_pLogicCtlStatus->SetPreemptCtlCmd(tPreemptCtlCmd);
	}
	
	if (m_nCtlSource == CTL_SOURCE_PREEMPT)
	{
		if (m_tOldPreemptCtlCmd.m_bPatternInterruptCmd)
		{
			if (m_tOldPreemptCtlCmd.m_tPatternInterruptCmd.m_nControlMode != CTL_MODE_SELFCTL)
			{
				//系统干预的时候，需要进行方案干预处理
				PreemptAskPlanRun();
			}
			else 
			{
				bRet = false;//回到自主
			}
		}
		else
		{
			if (m_nCurCtlMode != CTL_MODE_MANUAL && m_nCurCtlMode != CTL_MODE_PREEMPT)
			{
				TMainCtlBoardRunStatus tMainCtlBoardRunStatus;
				m_pLogicCtlStatus->GetMainCtlBoardRunStatus(tMainCtlBoardRunStatus);
				if (tMainCtlBoardRunStatus.m_bIsNeedGetParam)
				{
					tMainCtlBoardRunStatus.m_bIsNeedGetParam = false;
					m_pLogicCtlStatus->SetMainCtlBoardRunStatus(tMainCtlBoardRunStatus);
				}
			}
			m_pLogicCtlMode->Run();
		}
	}
	else
	{
		bRet = false;//回到自主
	}

    return bRet;      
}

/*==================================================================== 
函数名 ：SelfRun
功能 ：根据参数进行自主控制
算法实现 ： 
参数说明 ：无
返回值说明：无
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 刘黎明          创建函数 
====================================================================*/ 
void COpenATCLogicCtlManager::SelfRun()
{
    char szInfo[256] = {0};

	if (m_nCtlSource != CTL_SOURCE_SELF)
	{
		//设置系统控制命令状态
		SetSystemControlStatus(CONTROL_SUCCEED, 0, CONTROL_SUCCEED, 0, CONTROL_SUCCEED, 0, CONTROL_FAILED, NO_SUPPORT_CONTROL_WAY, CONTROL_SUCCEED, 0);
		//设置优先控制命令状态
		SetPreemptControlStatus(CONTROL_SUCCEED, 0);
	}

	m_nCtlSource = CTL_SOURCE_SELF;

    TMainCtlBoardRunStatus tMainCtlBoardRunStatus;
    m_pLogicCtlStatus->GetMainCtlBoardRunStatus(tMainCtlBoardRunStatus);
    if (tMainCtlBoardRunStatus.m_bIsNeedGetParam)
    {
        PrepareParam();
        m_pLogicCtlStatus->GetMainCtlBoardRunStatus(tMainCtlBoardRunStatus);
        tMainCtlBoardRunStatus.m_bIsNeedGetParam = false;
        m_pLogicCtlStatus->SetMainCtlBoardRunStatus(tMainCtlBoardRunStatus);
    }
        
    if (m_bIsCtlModeChg)
    {
        if (m_pLogicCtlMode != NULL)
        {
            m_pLogicCtlMode->Release();
            delete m_pLogicCtlMode;
            m_pLogicCtlMode = NULL;
        }

		m_pLogicCtlMode = CLogicCtlModeSimpleFactory::Create(m_nCurCtlMode);
        m_pLogicCtlMode->Init(m_pLogicCtlParam,m_pLogicCtlStatus,m_pOpenATCLog,0);
        m_bIsCtlModeChg = false;

		if (m_nCurCtlMode == CTL_MODE_FLASH || m_nCurCtlMode == CTL_MODE_ALLRED || m_nCurCtlMode == CTL_MODE_OFF)
		{
			if (m_nCurCtlMode == CTL_MODE_FLASH)
			{
                m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "Param Ctl Yellow Flash!");
			}
			else if (m_nCurCtlMode == CTL_MODE_ALLRED)
			{
                m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "Param Ctl All Red!");
			}
			else
			{
                m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "Param Ctl Off!");
			}
		}
		else 
		{
			sprintf(szInfo, "Current Ctl Mode is %d, Plan No is %d", m_nCurCtlMode, m_nCurPlanNo);
            m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, szInfo);

			//设置系统控制命令状态
			//SetSystemControlStatus(CONTROL_SUCCEED, 0, CONTROL_SUCCEED, 0, CONTROL_SUCCEED, 0);
			 m_tRunStageTable = m_pLogicCtlMode->GetRunStageTable();
		}

		if (m_tOldValidManualCmd.m_bNewCmd)
		{
			//干预回到自主成功，把有效的手动指令和最近一次缓存的手动指令清除
			TManualCmd  tValidManualCmd;
			memset(&tValidManualCmd,0,sizeof(tValidManualCmd));
			m_pLogicCtlStatus->SetValidManualCmd(tValidManualCmd); 

			memset(&m_tOldValidManualCmd,0,sizeof(tValidManualCmd));
		}
    }    

    m_pLogicCtlMode->Run(); 
}

/*==================================================================== 
函数名 ：SystemAskPlanRun
功能 ：根据用户干预的控制方式和方案进行控制
算法实现 ： 
参数说明 ：无
返回值说明：无
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 刘黎明          创建函数 
====================================================================*/ 
void COpenATCLogicCtlManager::SystemAskPlanRun()
{
    TMainCtlBoardRunStatus tMainCtlBoardRunStatus;
    m_pLogicCtlStatus->GetMainCtlBoardRunStatus(tMainCtlBoardRunStatus);
    if (tMainCtlBoardRunStatus.m_bIsNeedGetParam)
    {
		TManualCmd  tValidManualCmd;
		memset(&tValidManualCmd,0,sizeof(tValidManualCmd));
		m_pLogicCtlStatus->GetValidManualCmd(tValidManualCmd);

		if (!tValidManualCmd.m_bNewCmd && !m_tOldValidManualCmd.m_bNewCmd)
		{
			m_pLogicCtlStatus->GetMainCtlBoardRunStatus(tMainCtlBoardRunStatus);
			tMainCtlBoardRunStatus.m_bIsNeedGetParam = false;
			m_pLogicCtlStatus->SetMainCtlBoardRunStatus(tMainCtlBoardRunStatus);
			return;
		}

		//判断手动控制的临时方案的延迟时间
		if (tValidManualCmd.m_tPatternInterruptCmd.m_nControlMode == CTL_MODE_MANUAL_CONTROL_PATTERN &&
			tValidManualCmd.m_tPatternInterruptCmd.m_nPatternNo == MAX_PATTERN_COUNT)
		{
			long lCurTime = time(NULL);
			if (labs(lCurTime - m_tPatternInterruptCmdTime) < 
				tValidManualCmd.m_tPatternInterruptCmd.m_tManualControlPattern.m_nDelayTime)
			{
				m_pLogicCtlMode->Run();
				return;
			}
		}

        PrepareParamForSystemAsk(tValidManualCmd.m_tPatternInterruptCmd.m_nControlMode,tValidManualCmd.m_tPatternInterruptCmd.m_nPatternNo);
    	    
        m_pLogicCtlStatus->GetMainCtlBoardRunStatus(tMainCtlBoardRunStatus);
        tMainCtlBoardRunStatus.m_bIsNeedGetParam = false;
        m_pLogicCtlStatus->SetMainCtlBoardRunStatus(tMainCtlBoardRunStatus);

        TLogicCtlStatus tCtlStatus;
        m_pLogicCtlStatus->GetLogicCtlStatus(tCtlStatus);
        if (tCtlStatus.m_nCurPlanNo != tValidManualCmd.m_tPatternInterruptCmd.m_nPatternNo ||
			tCtlStatus.m_nCurCtlMode != tValidManualCmd.m_tPatternInterruptCmd.m_nControlMode || tCtlStatus.m_nCurPlanNo == MAX_PATTERN_COUNT)
        {
            m_bIsCtlModeChg = true;
            m_nCurPlanNo = tValidManualCmd.m_tPatternInterruptCmd.m_nPatternNo; 
            m_nCurCtlMode = tValidManualCmd.m_tPatternInterruptCmd.m_nControlMode; 

			m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "System Ctl Mode is %d, Plan No is %d", tValidManualCmd.m_tPatternInterruptCmd.m_nControlMode,tValidManualCmd.m_tPatternInterruptCmd.m_nPatternNo);

			if (m_nCurCtlMode == CTL_MODE_CHANNEL_CHECK)
			{
				//通道检测时，方案干预参数清空
				tValidManualCmd.m_bPatternInterruptCmd = false;
				memset(&tValidManualCmd.m_tPatternInterruptCmd,0,sizeof(TPatternInterruptCmd));
				m_pLogicCtlStatus->SetValidManualCmd(tValidManualCmd);
			}
			else if (m_nCurCtlMode == CTL_MODE_MANUAL_CONTROL_PATTERN)//方案切到手动控制方案
			{
				m_pLogicCtlParam->SetManualControlPattern(tValidManualCmd.m_tPatternInterruptCmd.m_tManualControlPattern);
				m_nManualControlPatternStartTime = time(NULL);
				m_nManualControlPatternDurationTime = tValidManualCmd.m_tPatternInterruptCmd.m_tManualControlPattern.m_nDurationTime;

				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "ManualControlPattern StartTime:%d, DurationTime:%d, CycleTime:%d", m_nManualControlPatternStartTime, m_nManualControlPatternDurationTime, tValidManualCmd.m_tPatternInterruptCmd.m_tManualControlPattern.m_nCycleTime);
			}
        }
    }
        
    if (m_bIsCtlModeChg)
    {
        if (m_pLogicCtlMode != NULL)
        {
            m_pLogicCtlMode->Release();
            delete m_pLogicCtlMode;
            m_pLogicCtlMode = NULL;
        }

        m_pLogicCtlMode = CLogicCtlModeSimpleFactory::Create(m_nCurCtlMode);
        m_pLogicCtlMode->Init(m_pLogicCtlParam,m_pLogicCtlStatus,m_pOpenATCLog,m_nCurPlanNo);
        m_bIsCtlModeChg = false;
		m_pLogicCtlMode->SetSystemUsrCtlFlag(true);  
        m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "Current Ctl Mode is %d, Plan No is %d", m_nCurCtlMode, m_nCurPlanNo);

		//设置系统控制命令状态
		SetSystemControlStatus(CONTROL_SUCCEED, 0, CONTROL_SUCCEED, 0, CONTROL_SUCCEED, 0, CONTROL_FAILED, NO_SUPPORT_CONTROL_WAY, CONTROL_SUCCEED, 0);
		//设置优先控制命令状态
		//SetPreemptControlStatus(CONTROL_SUCCEED, 0);

		if (m_tOldValidManualCmd.m_bNewCmd)
		{
			//方案干预成功，把有效的手动指令和最近一次缓存的手动指令清除
			TManualCmd  tValidManualCmd;
			memset(&tValidManualCmd,0,sizeof(tValidManualCmd));
			m_pLogicCtlStatus->SetValidManualCmd(tValidManualCmd); 

			memset(&m_tOldValidManualCmd,0,sizeof(tValidManualCmd));
		}
    }    

	m_pLogicCtlMode->Run();
}

/*==================================================================== 
函数名 ：PreemptAskPlanRun
功能 ：根据用户干预的控制方式和方案进行控制
算法实现 ： 
参数说明 ：无
返回值说明：无
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 李永萍          创建函数 
====================================================================*/ 
void COpenATCLogicCtlManager::PreemptAskPlanRun()
{
	TMainCtlBoardRunStatus tMainCtlBoardRunStatus;
    m_pLogicCtlStatus->GetMainCtlBoardRunStatus(tMainCtlBoardRunStatus);
    if (tMainCtlBoardRunStatus.m_bIsNeedGetParam)
    {
	    PrepareParamForSystemAsk(m_tOldPreemptCtlCmd.m_tPatternInterruptCmd.m_nControlMode,m_tOldPreemptCtlCmd.m_tPatternInterruptCmd.m_nPatternNo);
    	    
        m_pLogicCtlStatus->GetMainCtlBoardRunStatus(tMainCtlBoardRunStatus);
        tMainCtlBoardRunStatus.m_bIsNeedGetParam = false;
        m_pLogicCtlStatus->SetMainCtlBoardRunStatus(tMainCtlBoardRunStatus);

        TLogicCtlStatus tCtlStatus;
        m_pLogicCtlStatus->GetLogicCtlStatus(tCtlStatus);
        if (tCtlStatus.m_nCurPlanNo != m_tOldPreemptCtlCmd.m_tPatternInterruptCmd.m_nPatternNo ||
			tCtlStatus.m_nCurCtlMode != m_tOldPreemptCtlCmd.m_tPatternInterruptCmd.m_nControlMode || tCtlStatus.m_nCurPlanNo == MAX_PATTERN_COUNT)
        {
            m_bIsCtlModeChg = true;
            m_nCurPlanNo = m_tOldPreemptCtlCmd.m_tPatternInterruptCmd.m_nPatternNo; 
            m_nCurCtlMode = m_tOldPreemptCtlCmd.m_tPatternInterruptCmd.m_nControlMode; 

			m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "Preempt Ctl Mode is %d, Plan No is %d", m_tOldPreemptCtlCmd.m_tPatternInterruptCmd.m_nControlMode,m_tOldPreemptCtlCmd.m_tPatternInterruptCmd.m_nPatternNo);

        }
    }
        
    if (m_bIsCtlModeChg)
    {
        if (m_pLogicCtlMode != NULL)
        {
            m_pLogicCtlMode->Release();
            delete m_pLogicCtlMode;
            m_pLogicCtlMode = NULL;
        }

        m_pLogicCtlMode = CLogicCtlModeSimpleFactory::Create(m_nCurCtlMode);
        m_pLogicCtlMode->Init(m_pLogicCtlParam,m_pLogicCtlStatus,m_pOpenATCLog,m_nCurPlanNo);
        m_bIsCtlModeChg = false;
		m_pLogicCtlMode->SetSystemUsrCtlFlag(true);  
        m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "Current Ctl Mode is %d, Plan No is %d", m_nCurCtlMode, m_nCurPlanNo);

		//设置系统控制命令状态
		SetSystemControlStatus(CONTROL_SUCCEED, 0, CONTROL_SUCCEED, 0, CONTROL_SUCCEED, 0, CONTROL_FAILED, NO_SUPPORT_CONTROL_WAY, CONTROL_SUCCEED, 0);
		//设置优先控制命令状态
		SetPreemptControlStatus(CONTROL_SUCCEED, 0);

		if (m_tOldPreemptCtlCmd.m_bNewCmd)
		{
			memset(&m_tOldPreemptCtlCmd,0,sizeof(TPreemptCtlCmd));
		}
    }    

	m_pLogicCtlMode->Run();
}

/*==================================================================== 
函数名 ：CriticalFaultRun
功能 ：严重故障时的运行流程
算法实现 ： 
参数说明 ：无
返回值说明：无
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 刘黎明          创建函数 
====================================================================*/ 
void COpenATCLogicCtlManager::CriticalFaultRun()
{
    m_nLogicCtlStage = CTL_STAGE_FAULTFORCE;
    if (m_pLogicCtlMode != NULL)
    {
        m_pLogicCtlMode->Release();
        delete m_pLogicCtlMode;
        m_pLogicCtlMode = NULL;
    }

    m_pLogicCtlMode = CLogicCtlModeSimpleFactory::Create(CTL_MODE_FLASH);
    m_nCurCtlMode = CTL_MODE_FLASH;
    m_pLogicCtlMode->Init(m_pLogicCtlParam,m_pLogicCtlStatus,m_pOpenATCLog,0);

    m_pLogicCtlMode->Run();

    TLogicCtlStatus tCtlStatus;
    m_pLogicCtlStatus->GetLogicCtlStatus(tCtlStatus);
    tCtlStatus.m_nRunStage = CTL_STAGE_FAULTFORCE;
    m_pLogicCtlStatus->SetLogicCtlStatus(tCtlStatus);

    m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "Enter serious yellow flash fault!");

	TSystemControlStatus tSystemControlStatus;
	memset(&tSystemControlStatus, 0, sizeof(tSystemControlStatus));
	m_pLogicCtlStatus->GetSystemControlStatus(tSystemControlStatus);
	tSystemControlStatus.m_nSpecicalControlResult = CONTROL_FAILED;
	tSystemControlStatus.m_nSpecicalControlFailCode = NO_SUPPORT_CONTROL_WAY;
	tSystemControlStatus.m_nPatternControlResult = CONTROL_FAILED;
	tSystemControlStatus.m_nPatternControlFailCode = NO_SUPPORT_CONTROL_WAY;
	tSystemControlStatus.m_nStageControlResult = CONTROL_FAILED;
	tSystemControlStatus.m_nStageControlFailCode = NO_SUPPORT_CONTROL_WAY;
	tSystemControlStatus.m_nPhaseControlResult = CONTROL_FAILED;
	tSystemControlStatus.m_nPhaseControlFailCode = NO_SUPPORT_CONTROL_WAY;
	m_pLogicCtlStatus->SetSystemControlStatus(tSystemControlStatus);
}

/*==================================================================== 
函数名 ：ProcGlobalRunStatus
功能 ：用于生成全局相位运行状态信息
算法实现 ： 
参数说明 ：无
返回值说明：无
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 刘黎明          创建函数 
====================================================================*/ 
void COpenATCLogicCtlManager::ProcGlobalRunStatus()
{
    if (m_pLogicCtlStatus->GetPhaseRunStatusWriteFlag())
    {
        m_pLogicCtlStatus->SetPhaseRunStatusWriteFlag(false);
        if (m_pLogicCtlMode != NULL)
        {
            m_pLogicCtlStatus->SetPhaseRunStatusReadFlag(false);

            TPhaseRunStatus tRunStatus;
            memset(&tRunStatus,0,sizeof(TPhaseRunStatus));   
            m_pLogicCtlStatus->GetPhaseRunStatus(tRunStatus);         
            m_pLogicCtlMode->GetPhaseRunStatus(tRunStatus);

			tRunStatus.m_nCurCtlMode = m_nCurCtlMode;

			TManualCmd  tValidManualCmd;
			memset(&tValidManualCmd,0,sizeof(tValidManualCmd));
			m_pLogicCtlStatus->GetValidManualCmd(tValidManualCmd);
			//方案干预可执行还没有切到方案干预的时候，发送到平台的控制方式要改成方案恢复过渡
			if (tValidManualCmd.m_bPatternInterruptCmd && tValidManualCmd.m_tPatternInterruptCmd.m_nControlMode == CTL_MODE_SYS_INTERRUPT)
			{
				tRunStatus.m_nCurCtlMode = CTL_MODE_MANUAL_RECOVER;
			}

			if (m_nCurCtlMode == CTL_MODE_MANUAL)
			{
				if (tValidManualCmd.m_nSubCtlMode == CTL_MANUAL_SUBMODE_DEFAULT && tValidManualCmd.m_bChannelLockCmd)
				{
					m_pLogicCtlMode->GetTransRunStatusBeforeLockPhase(tRunStatus);
				}

				if (tValidManualCmd.m_bPatternInterruptCmd && (tValidManualCmd.m_tPatternInterruptCmd.m_nControlMode == CTL_MODE_SELFCTL ||
					tValidManualCmd.m_tPatternInterruptCmd.m_nControlMode == CTL_MODE_FIXTIME ||
					tValidManualCmd.m_tPatternInterruptCmd.m_nControlMode == CTL_MODE_ACTUATE ||
					tValidManualCmd.m_tPatternInterruptCmd.m_nControlMode == CTL_MODE_ADVACTUATE ||
					tValidManualCmd.m_tPatternInterruptCmd.m_nControlMode == CTL_MODE_SINGLEOPTIM ||
					tValidManualCmd.m_tPatternInterruptCmd.m_nControlMode == CTL_MODE_CABLELESS ||
					tValidManualCmd.m_tPatternInterruptCmd.m_nControlMode == CTL_MODE_PEDCROSTREET ||
					tValidManualCmd.m_tPatternInterruptCmd.m_nControlMode == CTL_MODE_WEBSTER_OPTIM ||
					tValidManualCmd.m_tPatternInterruptCmd.m_nControlMode == CTL_MODE_ACTUATE_PEDCROSTREET ||
					tValidManualCmd.m_tPatternInterruptCmd.m_nControlMode == CTL_MODE_PREEMPT))
				{
					tRunStatus.m_nCurCtlMode = CTL_MODE_MANUAL_RECOVER;
				}

				if ((tValidManualCmd.m_nSubCtlMode == CTL_MANUAL_SUBMODE_PANEL_DIRECTION || tValidManualCmd.m_nSubCtlMode == CTL_MANUAL_SUBMODE_SYSTEM_CHANNEL_LOCK) &&
					tRunStatus.m_nCurCtlMode != CTL_MODE_MANUAL_RECOVER)
				{
					if (!tValidManualCmd.m_bPhaseToChannelLock)
					{
						tRunStatus.m_nCurCtlMode = CTL_MODE_CHANNEL_LOCK;
					}
					else
					{
						tRunStatus.m_nCurCtlMode = CTL_MODE_PHASE_LOCK;
					}
				}
			}

			if (tRunStatus.m_nCurCtlMode == CTL_MODE_PHASE_LOCK || tValidManualCmd.m_nSubCtlMode == CTL_MANUAL_SUBMODE_SYSTEM_CHANNEL_LOCK)
			{
				m_pLogicCtlMode->GetLockPhaseRunStatus(tRunStatus, m_bInvalidPhaseCmd, m_tInvalidPhaseLockPara);
			}

            tRunStatus.m_byPlanID = m_nCurPlanNo;
            tRunStatus.m_nCurCtlPattern = m_nCtlSource;

			if (m_nCtlSource == CTL_SOURCE_PREEMPT)
			{
				tRunStatus.m_nCurCtlPattern = CTL_SOURCE_SYSTEM;
			}

            TPhaseLampClrRunCounter tRunCounter;
            m_pLogicCtlStatus->GetPhaseLampClrRunCounter(tRunCounter);
            tRunStatus.m_nCycleRunTime += tRunCounter.m_nLampClrTime[0];
            tRunStatus.m_nCycleRemainTime = tRunStatus.m_nCycleLen - tRunStatus.m_nCycleRunTime;
            if (tRunStatus.m_nCycleRemainTime < 0)
            {
                tRunStatus.m_nCycleRemainTime = 0;
            }            

            m_pLogicCtlStatus->SetPhaseRunStatus(tRunStatus);

            m_pLogicCtlStatus->SetPhaseRunStatusReadFlag(true);
        }        
    }

    if (m_pLogicCtlMode != NULL)
    {
        TLedScreenShowInfo tLedScreenShowInfo;
        memset(&tLedScreenShowInfo,0,sizeof(tLedScreenShowInfo)); 
		m_pLogicCtlStatus->GetLedScreenShowInfo(tLedScreenShowInfo);

		if (m_nCurCtlMode == CTL_MODE_MANUAL)
		{
			TManualCmd  tValidManualCmd;
			memset(&tValidManualCmd,0,sizeof(tValidManualCmd));
			m_pLogicCtlStatus->GetValidManualCmd(tValidManualCmd);

			tLedScreenShowInfo.m_nCurCtlMode = tValidManualCmd.m_nCtlMode;
		    tLedScreenShowInfo.m_nSubCtlMode = tValidManualCmd.m_nSubCtlMode;
			if (tLedScreenShowInfo.m_nSubCtlMode == CTL_MANUAL_SUBMODE_PANEL_DIRECTION)
			{
				tLedScreenShowInfo.m_bKeyDirectionControl = true;
				if (!tValidManualCmd.m_bNewCmd && !tValidManualCmd.m_bDirectionCmd && !tValidManualCmd.m_bPatternInterruptCmd)
				{
					//非方向控制子模式切到方向控制子模式和在方向控制子模式下当前方向结束切到新的方向时，方向指令标志都会被清除，所以应该用缓存的方向编号
					tLedScreenShowInfo.m_nDirectionKeyIndex = m_tOldValidManualCmd.m_tDirectionCmd.m_nNextDirectionIndex;  
				}
			}
			else
			{
				tLedScreenShowInfo.m_bKeyDirectionControl = false;
				tLedScreenShowInfo.m_nDirectionKeyIndex = 0;
			}

			if (tLedScreenShowInfo.m_nSubCtlMode == CTL_MANUAL_SUBMODE_SYSTEM_CHANNEL_LOCK)
			{
				tLedScreenShowInfo.m_bChannelLockCheck = true;
				if (tValidManualCmd.m_bPhaseToChannelLock)
				{
					tLedScreenShowInfo.m_bPhaseToChannelLock = true;
					tLedScreenShowInfo.m_nPhaseLockCount = tValidManualCmd.m_tPhaseLockPara.m_nPhaseLockCount;
					for (int i = 0;i < tLedScreenShowInfo.m_nPhaseLockCount;i++)
					{
						tLedScreenShowInfo.m_tPhaseClr[i].m_byPhaseID = tValidManualCmd.m_tPhaseLockPara.m_nPhaseLockID[i];
					}
				}
				
				if (!tValidManualCmd.m_bNewCmd && !tValidManualCmd.m_bChannelLockCmd && !tValidManualCmd.m_bPatternInterruptCmd)
				{
					//非通道锁定子模式切到通道锁定子模式和在通道锁定子模式下当前通道锁定结束切到新的通道锁定时，通道锁定指令标志都会被清除，所以应该用缓存的通道灯色 
					memcpy(&tLedScreenShowInfo.m_tChannelLockCtrlCmd, &m_tOldValidManualCmd.m_tChannelLockCmd.m_tNextChannelLockCtrlCmd, sizeof(TChannelLockCtrlCmd));
				}
			}
			else
			{
				tLedScreenShowInfo.m_bChannelLockCheck = false;
				memset(&tLedScreenShowInfo.m_tChannelLockCtrlCmd, 0, sizeof(TChannelLockCtrlCmd));
			}

			if (tValidManualCmd.m_bPatternInterruptCmd && m_nCurCtlMode != tValidManualCmd.m_tPatternInterruptCmd.m_nControlMode)
			{
				//方案干预指令下发后，当前的控制方式还没有切到指定控制方式，则表示在过渡中
				/*if (m_nCtlSource == CTL_SOURCE_SYSTEM)
				{
					tLedScreenShowInfo.m_nCurCtlMode = CTL_MODE_SYSTEM_TRAN;
				}
				else
				{
					tLedScreenShowInfo.m_nCurCtlMode = CTL_MODE_PANEL_TRAN;
				}*/
			}
		}
		else
		{
			tLedScreenShowInfo.m_nCurCtlMode = m_nCurCtlMode;
		    tLedScreenShowInfo.m_nSubCtlMode = CTL_MANUAL_SUBMODE_DEFAULT;
			tLedScreenShowInfo.m_bKeyDirectionControl = false;
			tLedScreenShowInfo.m_nDirectionKeyIndex = 0;
			if (m_nCurCtlMode == CTL_MODE_FLASH || m_nCurCtlMode == CTL_MODE_ALLRED || m_nCurCtlMode == CTL_MODE_OFF)
			{
				tLedScreenShowInfo.m_nRingCount = 0;
			}
			tLedScreenShowInfo.m_bChannelLockCheck = false;
		}

		if (tLedScreenShowInfo.m_nCurCtlMode == CTL_MODE_CHANNEL_CHECK)
		{
			static unsigned long nChannelCheckGlobalCounter = 0;

			tLedScreenShowInfo.m_bChannelCheck = true;

            TAscChannelVerifyInfo	atChannelCheckInfo;	
	        memset(&atChannelCheckInfo, 0, sizeof(atChannelCheckInfo));
	        m_pLogicCtlStatus->GetChannelCheckInfo(atChannelCheckInfo);

			if (memcmp(&m_tOldChannelCheckInfo, &atChannelCheckInfo, sizeof(atChannelCheckInfo)) != 0)
			{
				nChannelCheckGlobalCounter = m_pLogicCtlStatus->GetGlobalCounter();
				tLedScreenShowInfo.m_tChannelCheckClr.m_nChannelDurationTime = 0;
				memcpy(&m_tOldChannelCheckInfo, &atChannelCheckInfo, sizeof(atChannelCheckInfo));
			}

			for (int i = 0;i < C_N_MAX_LAMP_OUTPUT_NUM;i++)
			{
			    if (atChannelCheckInfo.m_achLampClr[i] == LAMP_CLR_ON)
				{
					tLedScreenShowInfo.m_tChannelCheckClr.m_nChannelID = i / C_N_CHANNEL_OUTPUTNUM + 1;
					if (i % C_N_CHANNEL_OUTPUTNUM == 0)
					{
					    tLedScreenShowInfo.m_tChannelCheckClr.m_achChannelClr = C_CH_PHASESTAGE_R;
					}
					else if (i % C_N_CHANNEL_OUTPUTNUM == 1)
					{
					    tLedScreenShowInfo.m_tChannelCheckClr.m_achChannelClr = C_CH_PHASESTAGE_Y;
					}
					else if (i % C_N_CHANNEL_OUTPUTNUM == 2)
					{
					    tLedScreenShowInfo.m_tChannelCheckClr.m_achChannelClr = C_CH_PHASESTAGE_G;
					}

					unsigned long nCounterDiff = CalcCounter(nChannelCheckGlobalCounter, m_pLogicCtlStatus->GetGlobalCounter(), C_N_MAXGLOBALCOUNTER);
					if (nCounterDiff >= C_N_TIMER_MILLSECOND)
					{
						nChannelCheckGlobalCounter = m_pLogicCtlStatus->GetGlobalCounter();
					    tLedScreenShowInfo.m_tChannelCheckClr.m_nChannelDurationTime += 1;
					}

					break;
				}
			}
		}
		else
		{
			tLedScreenShowInfo.m_bChannelCheck = false;
			memset(&m_tOldChannelCheckInfo,0,sizeof(m_tOldChannelCheckInfo));
		}
        
        m_pLogicCtlMode->GetLedScreenShowInfo(tLedScreenShowInfo);
		if (m_pLogicCtlStatus->GetLedScreenShowInfoFlag())
		{
			m_pLogicCtlStatus->SetLedScreenShowInfoFlag(false);
			m_pLogicCtlStatus->SetLedScreenShowInfo(tLedScreenShowInfo);
			m_pLogicCtlStatus->SetLedScreenShowInfoFlag(true);
		}
    }
}                             

/*==================================================================== 
函数名 ：PrepareParam
功能 ：用于设置当前时间点运行需要的特征参数
算法实现 ： 
参数说明 ：无
返回值说明：无
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 刘黎明          创建函数 
====================================================================*/ 
void COpenATCLogicCtlManager::PrepareParam()
{
    TParamRunStatus tParamRunStatus;
    m_pLogicCtlStatus->GetParamRunStatus(tParamRunStatus);

    int nYear,nMonth,nDay,nHour,nMin,nSec,nWeek;
#ifdef VIRTUAL_DEVICE
	bool bSpeedyRunStatu = false;
	bSpeedyRunStatu = m_pLogicCtlStatus->GetIsSpeedyRunStatus();
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
	//获取日期数
	iDatePlanNum = m_pLogicCtlParam->GetDatePlanNum();
	
	for (int i = 0; i < iDatePlanNum; i++)
	{
		m_pLogicCtlParam->GetTimeBaseScheduleByTime(nMonth, nWeek, nDay, tScheduleInfo, nCount);
		m_pLogicCtlParam->GetTimeBaseDayPlanByTime(nHour, nMin, tScheduleInfo.m_byTimeBaseScheduleDayPlan, tTimeBaseDayPlanInfo);
		if (/*tTimeBaseDayPlanInfo.m_byDayPlanActionNumberOID == 0 && */ tTimeBaseDayPlanInfo.m_byCoorDination == 1 && tTimeBaseDayPlanInfo.m_byDayPlanControl == 0)
		{
			continue;
		}
		else
		{
			break;	
		}
	}
	m_pLogicCtlParam->GetPatternByPlanNumber(tTimeBaseDayPlanInfo.m_byDayPlanActionNumberOID, tPatternInfo);
    //TTimeBaseSchedule tScheduleInfo;
    //m_pLogicCtlParam->GetTimeBaseScheduleByTime(nMonth,nWeek,nDay,tScheduleInfo);
    //TTimeBaseDayPlan tTimeBaseDayPlanInfo;
    //m_pLogicCtlParam->GetTimeBaseDayPlanByTime(nHour,nMin,tScheduleInfo.m_byTimeBaseScheduleDayPlan,tTimeBaseDayPlanInfo);
    //TPattern tPatternInfo;
    //m_pLogicCtlParam->GetPatternByPlanNumber(tTimeBaseDayPlanInfo.m_byDayPlanActionNumberOID,tPatternInfo);
    
    if (tParamRunStatus.m_chIsParameterChg == C_CH_ISPARAMETERCHG_OK)
    {
        if (!m_bFirstInitFlag)
        {
            memset(m_atOldChannelInfo,0,sizeof(m_atOldChannelInfo));
            m_pLogicCtlParam->GetChannelTable(m_atOldChannelInfo);

            //使用新参数
            m_pLogicCtlParam->Init(m_pLogicCtlStatus,m_pOpenATCLog);//新参数先初始化

#ifndef VIRTUAL_DEVICE
            if (IsLampCtlBoardChg())
            {
                m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "Parameter LampCtlBoard Change,Need to Reboot!");
                m_pLogicCtlStatus->SetRebootStatus(true);   
            }
            else
            {
#endif
                m_bIsCtlModeChg = true;
                tParamRunStatus.m_chIsParameterChg = C_CH_ISPARAMETERCHG_NO;
                m_pLogicCtlStatus->SetParamRunStatus(tParamRunStatus);

                //需要通知故障检测模块特征参数发生变化，使用了新参数.
                TMainCtlBoardRunStatus tMainCtlBoardStatus;
                m_pLogicCtlStatus->GetMainCtlBoardRunStatus(tMainCtlBoardStatus);
                tMainCtlBoardStatus.m_bIsUseNewParamForFault = true;
                tMainCtlBoardStatus.m_bIsUseNewParamForHard = true;
                m_pLogicCtlStatus->SetMainCtlBoardRunStatus(tMainCtlBoardStatus);

				nCount = 0;
				int iDatePlanNum = 0;
				//获取日期数
				iDatePlanNum = m_pLogicCtlParam->GetDatePlanNum();

				for (int i = 0; i < iDatePlanNum; i++)
				{
					m_pLogicCtlParam->GetTimeBaseScheduleByTime(nMonth, nWeek, nDay, tScheduleInfo, nCount);
					m_pLogicCtlParam->GetTimeBaseDayPlanByTime(nHour, nMin, tScheduleInfo.m_byTimeBaseScheduleDayPlan, tTimeBaseDayPlanInfo);
					if (/*tTimeBaseDayPlanInfo.m_byDayPlanActionNumberOID == 0 && */ tTimeBaseDayPlanInfo.m_byCoorDination == 1 && tTimeBaseDayPlanInfo.m_byDayPlanControl == 0)
					{
						continue;
					}
					else
					{
						break;
					}
				}
				m_pLogicCtlParam->GetPatternByPlanNumber(tTimeBaseDayPlanInfo.m_byDayPlanActionNumberOID, tPatternInfo);
                //m_pLogicCtlParam->GetTimeBaseScheduleByTime(nMonth,nWeek,nDay,tScheduleInfo);
                //m_pLogicCtlParam->GetTimeBaseDayPlanByTime(nHour,nMin,tScheduleInfo.m_byTimeBaseScheduleDayPlan,tTimeBaseDayPlanInfo);
                //m_pLogicCtlParam->GetPatternByPlanNumber(tTimeBaseDayPlanInfo.m_byDayPlanActionNumberOID,tPatternInfo);
#ifndef VIRTUAL_DEVICE
            }
#endif
        }
        else
        {
            m_bIsCtlModeChg = true;
            tParamRunStatus.m_chIsParameterChg = C_CH_ISPARAMETERCHG_NO;
            m_pLogicCtlStatus->SetParamRunStatus(tParamRunStatus);
            
            //初始的参数默认变化，不需要通知故障检测模块
            m_bFirstInitFlag = false;
        }
    }
    else
    {
        TLogicCtlStatus tCtlStatus;
        m_pLogicCtlStatus->GetLogicCtlStatus(tCtlStatus);
        if (tCtlStatus.m_nCurPlanNo != (int)tTimeBaseDayPlanInfo.m_byDayPlanActionNumberOID ||
            tCtlStatus.m_nCurCtlMode != (int)tTimeBaseDayPlanInfo.m_byDayPlanControl)
        {
            m_bIsCtlModeChg = true;

			//需要通知故障检测模块判断下相位类型是否变成忽略相位，此时需要重新下发绿冲突表
            TMainCtlBoardRunStatus tMainCtlBoardStatus;
            m_pLogicCtlStatus->GetMainCtlBoardRunStatus(tMainCtlBoardStatus);
            tMainCtlBoardStatus.m_bIsUseNewParamForFault = true;
            m_pLogicCtlStatus->SetMainCtlBoardRunStatus(tMainCtlBoardStatus);
        }
    }

    if (m_bIsCtlModeChg)
    {
        m_nCurPlanNo = (int)tTimeBaseDayPlanInfo.m_byDayPlanActionNumberOID; 
        m_nCurCtlMode = (int)tTimeBaseDayPlanInfo.m_byDayPlanControl;       
    }
}

/*==================================================================== 
函数名 PrepareParamForSystemAsk
功能 ：用于用户干预控制方式和方案时需要的特征参数
算法实现 ： 
参数说明 ：nCtlMode：控制方式，nPlanNo：方案号
返回值说明：无
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 刘黎明          创建函数 
====================================================================*/ 
void COpenATCLogicCtlManager::PrepareParamForSystemAsk(int nCtlMode,int nPlanNo)
{
    TParamRunStatus tParamRunStatus;
    m_pLogicCtlStatus->GetParamRunStatus(tParamRunStatus);
    
    if (tParamRunStatus.m_chIsParameterChg == C_CH_ISPARAMETERCHG_OK)
    {
        if (!m_bFirstInitFlag)
        {
            memset(m_atOldChannelInfo,0,sizeof(m_atOldChannelInfo));
            m_pLogicCtlParam->GetChannelTable(m_atOldChannelInfo);

            //使用新参数
            m_pLogicCtlParam->Init(m_pLogicCtlStatus,m_pOpenATCLog);//新参数先初始化
			m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "PrepareParamForSystemAsk Use NewParam");
#ifndef VIRTUAL_DEVICE
            if (IsLampCtlBoardChg())
            {
                m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "Parameter LampCtlBoard Change,Need to Reboot!");
                m_pLogicCtlStatus->SetRebootStatus(true);   
            }
            else
            {
#endif
                m_bIsCtlModeChg = true;
                tParamRunStatus.m_chIsParameterChg = C_CH_ISPARAMETERCHG_NO;
                m_pLogicCtlStatus->SetParamRunStatus(tParamRunStatus);

                //需要通知故障检测模块特征参数发生变化，使用了新参数.
                TMainCtlBoardRunStatus tMainCtlBoardStatus;
                m_pLogicCtlStatus->GetMainCtlBoardRunStatus(tMainCtlBoardStatus);
                tMainCtlBoardStatus.m_bIsUseNewParamForFault = true;
                tMainCtlBoardStatus.m_bIsUseNewParamForHard = true;
                m_pLogicCtlStatus->SetMainCtlBoardRunStatus(tMainCtlBoardStatus);
#ifndef VIRTUAL_DEVICE
            }
#endif
        }
        else
        {
            m_bIsCtlModeChg = true;
            tParamRunStatus.m_chIsParameterChg = C_CH_ISPARAMETERCHG_NO;
            m_pLogicCtlStatus->SetParamRunStatus(tParamRunStatus);
            
            //初始的参数默认变化，不需要通知故障检测模块
            m_bFirstInitFlag = false;
        }
    }
    else
    {
        TLogicCtlStatus tCtlStatus;
        m_pLogicCtlStatus->GetLogicCtlStatus(tCtlStatus);
        if (tCtlStatus.m_nCurPlanNo != nPlanNo ||
            tCtlStatus.m_nCurCtlMode != nCtlMode)
        {
            m_bIsCtlModeChg = true;
        }
    }

    if (m_bIsCtlModeChg)
    {
        m_nCurPlanNo = nPlanNo; 
        m_nCurCtlMode = nCtlMode;       
    }
}

/*==================================================================== 
函数名 ：ProcFault
功能 ：用于分析当前的故障状态
算法实现 ： 
参数说明 ：无
返回值说明：当前故障等级
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 刘黎明          创建函数 
====================================================================*/ 
int COpenATCLogicCtlManager::ProcFault()
{
    TLampFaultType tFault;
    m_pLogicCtlStatus->GetLampFault(tFault);

    int nRet = NO_FAULT;    
    if (tFault.Bin.bGreenLighConflict_Fault == 1 ||
        tFault.Bin.bLampNumger_Fault == 1 ||
        tFault.Bin.bRedAndGreenConflict_Fault == 1 ||
        tFault.Bin.bRedLightGoOut_Fault == 1 ||
        tFault.Bin.bFaultBoardOffline_Fault)
    {
        nRet = CRITICAL_FAULT;
    }

    return nRet;
} 

/*==================================================================== 
函数名 ：ProcLampClrRunCounter
功能 ：用于计算环运行过程中的灯色计时
算法实现 ： 
参数说明 ：无
返回值说明：无
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 刘黎明          创建函数 
====================================================================*/ 
void COpenATCLogicCtlManager::ProcLampClrRunCounter()
{
    TPhaseLampClrRunCounter tRunCounter;
    m_pLogicCtlStatus->GetPhaseLampClrRunCounter(tRunCounter);

    tRunCounter.m_nCurCounter = m_pLogicCtlStatus->GetLampClrCounter();

    for (int i = 0;i < MAX_RING_COUNT;i ++)
    {
        tRunCounter.m_nLampClrTime[i] = CalcCounter(tRunCounter.m_nLampClrStartCounter[i],tRunCounter.m_nCurCounter,C_N_MAXGLOBALCOUNTER);
        tRunCounter.m_nPedLampClrTime[i] = CalcCounter(tRunCounter.m_nPedLampClrStartCounter[i],tRunCounter.m_nCurCounter,C_N_MAXGLOBALCOUNTER);
    }
    
    m_pLogicCtlStatus->SetPhaseLampClrRunCounter(tRunCounter);
}

/*==================================================================== 
函数名 ：IsLampCtlBoardChg
功能 ：用于计算灯控板使用状态是否发生变化
算法实现 ： 
参数说明 ：无
返回值说明：
        true,表示灯控板变化
        false,表示灯控板未变化
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 刘黎明          创建函数 
====================================================================*/ 
bool COpenATCLogicCtlManager::IsLampCtlBoardChg()
{
    char achOldLampCtlBoardStatus[C_N_MAXLAMPBOARD_NUM];
    memset(achOldLampCtlBoardStatus,0,sizeof(achOldLampCtlBoardStatus));
    int i = 0;
    for (i = 0;i < MAX_CHANNEL_COUNT;i ++)
    {
        if (m_atOldChannelInfo[i].m_byChannelNumber == 0)
        {
            break;
        }

        int nIndex = (m_atOldChannelInfo[i].m_byChannelNumber - 1) / C_N_CHANNELNUM_PER_BOARD;
        if (achOldLampCtlBoardStatus[nIndex] != 1)
        {
            achOldLampCtlBoardStatus[nIndex] = 1;
        }
    }

    TChannel atChannelInfo[MAX_CHANNEL_COUNT];
    memset(atChannelInfo,0,sizeof(atChannelInfo));
    m_pLogicCtlParam->GetChannelTable(atChannelInfo);

    char achNewLampCtlBoardStatus[C_N_MAXLAMPBOARD_NUM];
    memset(achNewLampCtlBoardStatus,0,sizeof(achNewLampCtlBoardStatus));
    for (i = 0;i < MAX_CHANNEL_COUNT;i ++)
    {
        if (atChannelInfo[i].m_byChannelNumber == 0)
        {
            break;
        }

        int nIndex = (atChannelInfo[i].m_byChannelNumber - 1) / C_N_CHANNELNUM_PER_BOARD;
        if (achNewLampCtlBoardStatus[nIndex] != 1)
        {
            achNewLampCtlBoardStatus[nIndex] = 1;
        }
    }

    if (memcmp(achOldLampCtlBoardStatus,achNewLampCtlBoardStatus,sizeof(achOldLampCtlBoardStatus)) != 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

/*==================================================================== 
函数名 ：InitUsrCtlLogicObj
功能 ：进入手动控制时初始化手动控制对象
算法实现 ： 
参数说明 ：无
返回值说明：无
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 李永萍          创建函数 
====================================================================*/ 
void COpenATCLogicCtlManager::InitUsrCtlLogicObj()
{
    CLogicCtlMode * pMode = CLogicCtlModeSimpleFactory::Create(CTL_MODE_MANUAL);
    m_nCurCtlMode = CTL_MODE_MANUAL;
    pMode->Init(m_pLogicCtlParam,m_pLogicCtlStatus,m_pOpenATCLog,0);
    m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "Usr Ctl Manual!");

    void * pParam = m_pLogicCtlMode->GetCurCtlParam();
    void * pLampClr = m_pLogicCtlMode->GetCurLampClr();
    void * pStageTable = m_pLogicCtlMode->GetCurStageTable();
    void * pChannelSplitMode = m_pLogicCtlMode->GetCurChannelSplitMode();
    pMode->SetCtlDerivedParam(pParam,pLampClr,pStageTable,pChannelSplitMode);

    m_pLogicCtlMode->Release();
    delete m_pLogicCtlMode;
    m_pLogicCtlMode = NULL;
    m_pLogicCtlMode = pMode;
}

/*==================================================================== 
函数名 ：CreateCtlMode
功能 ：生成控制逻辑
算法实现 ： 
参数说明 ：nCtlMode：控制方式，nCtlSource：控制源
返回值说明：无
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2020/05/14     V1.0 李永萍          创建函数 
====================================================================*/
void COpenATCLogicCtlManager::CreateCtlMode(int nCtlMode, int nCtlSource)
{
	m_pLogicCtlMode->Release();
    delete m_pLogicCtlMode;
    m_pLogicCtlMode = NULL;
            
    m_pLogicCtlMode = CLogicCtlModeSimpleFactory::Create(nCtlMode);
    m_nCurCtlMode = nCtlMode;
    m_pLogicCtlMode->Init(m_pLogicCtlParam,m_pLogicCtlStatus,m_pOpenATCLog,0);
   
    m_nCtlSource = nCtlSource;

	if (m_nCtlSource == CTL_SOURCE_SYSTEM)
	{
		m_pLogicCtlMode->SetSystemUsrCtlFlag(true);  

		if (nCtlMode == CTL_MODE_FLASH)
		{
			m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "System Ctl Yellow Flash!");
		}
		else if (nCtlMode == CTL_MODE_ALLRED)
		{
			m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "System Ctl All Red!");
		}
		else if (nCtlMode == CTL_MODE_OFF)
		{
			m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "System Ctl Lamp Off!");
		}
	}
	else
	{
		m_pLogicCtlMode->SetUsrCtlFlag(true);  

		if (nCtlMode == CTL_MODE_FLASH)
		{
			m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "User Ctl Yellow Flash!");
		}
		else if (nCtlMode == CTL_MODE_ALLRED)
		{
			m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "User Ctl All Red!");
		}
		else if (nCtlMode == CTL_MODE_OFF)
		{
			m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "User Ctl Lamp Off!");
		}
	}
}

/*==================================================================== 
函数名 ：CreateValidManualCmd
功能 ：生成有效的控制指令
算法实现 ： 
参数说明 ：tManualCmd：控制指令，tValidManualCmd：有效的控制指令
返回值说明：无
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2020/05/14     V1.0 李永萍          创建函数 
====================================================================*/
void COpenATCLogicCtlManager::CreateValidManualCmd(TManualCmd tManualCmd, TManualCmd & tValidManualCmd)
{
	tValidManualCmd.m_bNewCmd = true;

	if (tManualCmd.m_bPatternInterruptCmd)
	{
		if (tManualCmd.m_tPatternInterruptCmd.m_nControlMode == CTL_MODE_FLASH)
		{
			tValidManualCmd.m_nCtlMode = CTL_MODE_FLASH;
			tValidManualCmd.m_nSubCtlMode = CTL_MANUAL_SUBMODE_DEFAULT;
			tValidManualCmd.m_nCmdSource = tManualCmd.m_nCmdSource;
	        tValidManualCmd.m_nCurCtlSource = m_nCtlSource;
			memcpy(tValidManualCmd.m_szPeerIp,tManualCmd.m_szPeerIp,strlen(tManualCmd.m_szPeerIp));
			tValidManualCmd.m_bStepForwardCmd = false;
			memset(&tValidManualCmd.m_tStepForwardCmd,0,sizeof(TStepForwardCmd));
			tValidManualCmd.m_bDirectionCmd = false;
			memset(&tValidManualCmd.m_tDirectionCmd,0,sizeof(TDirectionCmd));
			tValidManualCmd.m_bPatternInterruptCmd = false;
			memset(&tValidManualCmd.m_tPatternInterruptCmd,0,sizeof(TPatternInterruptCmd));
			tValidManualCmd.m_bChannelLockCmd = false;
			memset(&tValidManualCmd.m_tChannelLockCmd,0,sizeof(TChannelLockCmd));
			memset(&tValidManualCmd.m_tPhaseLockPara,0,sizeof(TPhaseLockPara));
			tValidManualCmd.m_bPhaseToChannelLock = false;
			tValidManualCmd.m_bPreemptCtlCmd = false;
		}
		else if (tManualCmd.m_tPatternInterruptCmd.m_nControlMode == CTL_MODE_ALLRED)
		{
			tValidManualCmd.m_nCtlMode = CTL_MODE_ALLRED;
			tValidManualCmd.m_nSubCtlMode = CTL_MANUAL_SUBMODE_DEFAULT;
			tValidManualCmd.m_nCmdSource = tManualCmd.m_nCmdSource;
	        tValidManualCmd.m_nCurCtlSource = m_nCtlSource;
			memcpy(tValidManualCmd.m_szPeerIp,tManualCmd.m_szPeerIp,strlen(tManualCmd.m_szPeerIp));
			tValidManualCmd.m_bStepForwardCmd = false;
			memset(&tValidManualCmd.m_tStepForwardCmd,0,sizeof(TStepForwardCmd));
			tValidManualCmd.m_bDirectionCmd = false;
			memset(&tValidManualCmd.m_tDirectionCmd,0,sizeof(TDirectionCmd));
			tValidManualCmd.m_bPatternInterruptCmd = false;
			memset(&tValidManualCmd.m_tPatternInterruptCmd,0,sizeof(TPatternInterruptCmd));
			tValidManualCmd.m_bChannelLockCmd = false;
			memset(&tValidManualCmd.m_tChannelLockCmd,0,sizeof(TChannelLockCmd));
			memset(&tValidManualCmd.m_tPhaseLockPara,0,sizeof(TPhaseLockPara));
			tValidManualCmd.m_bPhaseToChannelLock = false;
			tValidManualCmd.m_bPreemptCtlCmd = false;
		}
		else if (tManualCmd.m_tPatternInterruptCmd.m_nControlMode == CTL_MODE_OFF)
		{
			tValidManualCmd.m_nCtlMode = CTL_MODE_OFF;
			tValidManualCmd.m_nSubCtlMode = CTL_MANUAL_SUBMODE_DEFAULT;
			tValidManualCmd.m_nCmdSource = tManualCmd.m_nCmdSource;
	        tValidManualCmd.m_nCurCtlSource = m_nCtlSource;
			memcpy(tValidManualCmd.m_szPeerIp,tManualCmd.m_szPeerIp,strlen(tManualCmd.m_szPeerIp));
			tValidManualCmd.m_bStepForwardCmd = false;
			memset(&tValidManualCmd.m_tStepForwardCmd,0,sizeof(TStepForwardCmd));
			tValidManualCmd.m_bDirectionCmd = false;
			memset(&tValidManualCmd.m_tDirectionCmd,0,sizeof(TDirectionCmd));
			tValidManualCmd.m_bPatternInterruptCmd = false;
			memset(&tValidManualCmd.m_tPatternInterruptCmd,0,sizeof(TPatternInterruptCmd));
			tValidManualCmd.m_bChannelLockCmd = false;
			memset(&tValidManualCmd.m_tChannelLockCmd,0,sizeof(TChannelLockCmd));
			memset(&tValidManualCmd.m_tPhaseLockPara,0,sizeof(TPhaseLockPara));
			tValidManualCmd.m_bPhaseToChannelLock = false;
			tValidManualCmd.m_bPreemptCtlCmd = false;
		}
		else //回到自主或方案干预
		{
			if (tManualCmd.m_nCurCtlSource != CTL_SOURCE_SELF)//用户干预超时回到自主命令，此时控制源只能是面板控制或是系统控制
			{
				tValidManualCmd.m_nCtlMode = CTL_MODE_SELFCTL;//自主控制
				tValidManualCmd.m_nSubCtlMode = CTL_MANUAL_SUBMODE_DEFAULT;
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "CreateValidManualCmd, User Intervention Timeout Return To Self");
			}
			else
			{
				if (tValidManualCmd.m_nSubCtlMode == CTL_MANUAL_SUBMODE_PANEL_FITSTMANUAL)
				{
					//子模式在面板手动模式下，直接切到默认模式，否则保留
					tValidManualCmd.m_nSubCtlMode = CTL_MANUAL_SUBMODE_DEFAULT;
				}
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "CreateValidManualCmd, Return To ControlMode:%d SubControlMode:%d", tManualCmd.m_tPatternInterruptCmd.m_nControlMode, tValidManualCmd.m_nSubCtlMode);
			}
			tValidManualCmd.m_nCmdSource = tManualCmd.m_nCmdSource;//设置命令来源
			tValidManualCmd.m_nCurCtlSource = m_nCtlSource;//设置控制源
			memcpy(tValidManualCmd.m_szPeerIp,tManualCmd.m_szPeerIp,strlen(tManualCmd.m_szPeerIp));
			tValidManualCmd.m_bStepForwardCmd = false;
			memset(&tValidManualCmd.m_tStepForwardCmd,0,sizeof(TStepForwardCmd));
			tValidManualCmd.m_bDirectionCmd = false;
			memset(&tValidManualCmd.m_tDirectionCmd,0,sizeof(TDirectionCmd));
			tValidManualCmd.m_bPatternInterruptCmd = true;
			tValidManualCmd.m_tPatternInterruptCmd.m_nControlMode = tManualCmd.m_tPatternInterruptCmd.m_nControlMode;
			tValidManualCmd.m_tPatternInterruptCmd.m_nPatternNo = tManualCmd.m_tPatternInterruptCmd.m_nPatternNo;
			if (tManualCmd.m_tPatternInterruptCmd.m_nControlMode == CTL_MODE_MANUAL_CONTROL_PATTERN)
			{
				tValidManualCmd.m_nSubCtlMode = CTL_MANUAL_SUBMODE_SYSTEM_INTERRUPT_PATTERN;
				memcpy(&tValidManualCmd.m_tPatternInterruptCmd.m_tManualControlPattern,&tManualCmd.m_tPatternInterruptCmd.m_tManualControlPattern,sizeof(tManualCmd.m_tPatternInterruptCmd.m_tManualControlPattern));
				m_tPatternInterruptCmdTime = time(NULL);
			}
			tValidManualCmd.m_bChannelLockCmd = false;
			memset(&tValidManualCmd.m_tChannelLockCmd,0,sizeof(TChannelLockCmd));
			memset(&tValidManualCmd.m_tPhaseLockPara,0,sizeof(TPhaseLockPara));
			tValidManualCmd.m_bPhaseToChannelLock = false;
			tValidManualCmd.m_bPreemptCtlCmd = false;
		}
	}
	else if (tManualCmd.m_bStepForwardCmd)
	{
		tValidManualCmd.m_nCtlMode = CTL_MODE_MANUAL;
		if (m_nCtlSource == CTL_SOURCE_LOCAL)
		{
			if (tValidManualCmd.m_nSubCtlMode == CTL_MANUAL_SUBMODE_PANEL_FITSTMANUAL)
			{
				//子模式在面板手动模式下，直接切到面板步进模式，否则保留
				tValidManualCmd.m_nSubCtlMode = CTL_MANUAL_SUBMODE_PANEL_STEPFOWARD;
			}
			m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "CreateValidManualCmd, Trans To Panel StepWard");
		}
		else
		{
			if (tValidManualCmd.m_nSubCtlMode == CTL_MANUAL_SUBMODE_DEFAULT ||
			   (m_nCtlSource == CTL_SOURCE_SELF && tValidManualCmd.m_nSubCtlMode == CTL_MANUAL_SUBMODE_PANEL_STEPFOWARD))
			{
				//1.子模式在默认模式下，直接切到系统步进模式，否则保留
				//2.面板步进在回自主的路上，按了系统步进，子模式改成系统步进模式
				tValidManualCmd.m_nSubCtlMode = CTL_MANUAL_SUBMODE_SYSTEM_STEPFOWARD;
				m_pLogicCtlMode->SetSystemUsrCtlFlag(true); 
			}
			m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "CreateValidManualCmd, Trans To System StepWard");
		}
		tValidManualCmd.m_nCmdSource = tManualCmd.m_nCmdSource;
	    tValidManualCmd.m_nCurCtlSource = m_nCtlSource;
		memcpy(tValidManualCmd.m_szPeerIp,tManualCmd.m_szPeerIp,strlen(tManualCmd.m_szPeerIp));
		tValidManualCmd.m_bStepForwardCmd = true;
		memcpy(&tValidManualCmd.m_tStepForwardCmd,&tManualCmd.m_tStepForwardCmd,sizeof(TStepForwardCmd));
		tValidManualCmd.m_bDirectionCmd = false;
		memset(&tValidManualCmd.m_tDirectionCmd,0,sizeof(TDirectionCmd));
		tValidManualCmd.m_bPatternInterruptCmd = false;
		memset(&tValidManualCmd.m_tPatternInterruptCmd,0,sizeof(TPatternInterruptCmd));
		tValidManualCmd.m_bChannelLockCmd = false;
		memset(&tValidManualCmd.m_tChannelLockCmd,0,sizeof(TChannelLockCmd));
		memset(&tValidManualCmd.m_tPhaseLockPara,0,sizeof(TPhaseLockPara));
		tValidManualCmd.m_bPhaseToChannelLock = false;
		tValidManualCmd.m_bPreemptCtlCmd = false;
	}
	else if (tManualCmd.m_bDirectionCmd)
	{
		tValidManualCmd.m_nCmdSource = tManualCmd.m_nCmdSource;
	    tValidManualCmd.m_nCurCtlSource = m_nCtlSource;
		memcpy(tValidManualCmd.m_szPeerIp,tManualCmd.m_szPeerIp,strlen(tManualCmd.m_szPeerIp));
		tValidManualCmd.m_bStepForwardCmd = false;
		memset(&tValidManualCmd.m_tStepForwardCmd,0,sizeof(TStepForwardCmd));
		tValidManualCmd.m_bDirectionCmd = true;
		memcpy(&tValidManualCmd.m_tDirectionCmd,&tManualCmd.m_tDirectionCmd,sizeof(TDirectionCmd));
		tValidManualCmd.m_bPatternInterruptCmd = false;
		memset(&tValidManualCmd.m_tPatternInterruptCmd,0,sizeof(TPatternInterruptCmd));
		tValidManualCmd.m_bChannelLockCmd = false;
		memset(&tValidManualCmd.m_tChannelLockCmd,0,sizeof(TChannelLockCmd));
		memset(&tValidManualCmd.m_tPhaseLockPara,0,sizeof(TPhaseLockPara));
		tValidManualCmd.m_bPhaseToChannelLock = false;
		tValidManualCmd.m_bPreemptCtlCmd = false;
		m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "CreateValidManualCmd, Trans To Panel DirectionIndex:%d", tValidManualCmd.m_tDirectionCmd.m_nNextDirectionIndex);

		if (m_tOldValidManualCmd.m_nSubCtlMode == CTL_MANUAL_SUBMODE_PANEL_FITSTMANUAL)
		{
			tValidManualCmd.m_nSubCtlMode = CTL_MANUAL_SUBMODE_DEFAULT;//子模式先置为默认，等当前阶段过渡结束开始方向控制时再置为方向子模式
		}

		//黄闪，全红和关灯切方向
		if (m_nCurCtlMode == CTL_MODE_FLASH || m_nCurCtlMode == CTL_MODE_ALLRED || m_nCurCtlMode == CTL_MODE_OFF)
		{
			InitUsrCtlLogicObj();
			m_nCurCtlMode = CTL_MODE_MANUAL;
			tValidManualCmd.m_nCtlMode = CTL_MODE_MANUAL;
			tValidManualCmd.m_nSubCtlMode = CTL_MANUAL_SUBMODE_PANEL_DIRECTION;//从黄闪，全红，关灯切到方向控制的直接把子模式置为方向子模式
			tValidManualCmd.m_bDirectionCmd = false;//切到方向控制子模式下，清除当前方向指令标志
		}
	}
	else if (tManualCmd.m_bChannelLockCmd)
	{
		tValidManualCmd.m_nCtlMode = CTL_MODE_MANUAL;
		tValidManualCmd.m_nCmdSource = tManualCmd.m_nCmdSource;
	    tValidManualCmd.m_nCurCtlSource = m_nCtlSource;
		memcpy(tValidManualCmd.m_szPeerIp,tManualCmd.m_szPeerIp,strlen(tManualCmd.m_szPeerIp));
		tValidManualCmd.m_bStepForwardCmd = false;
		memset(&tValidManualCmd.m_tStepForwardCmd,0,sizeof(TStepForwardCmd));
		tValidManualCmd.m_bDirectionCmd = false;
		memset(&tValidManualCmd.m_tDirectionCmd,0,sizeof(TDirectionCmd));
		tValidManualCmd.m_bPatternInterruptCmd = false;
		memset(&tValidManualCmd.m_tPatternInterruptCmd,0,sizeof(TPatternInterruptCmd));
		tValidManualCmd.m_bChannelLockCmd = true;
		memcpy(&tValidManualCmd.m_tChannelLockCmd,&tManualCmd.m_tChannelLockCmd,sizeof(TChannelLockCmd));
		tValidManualCmd.m_bPhaseToChannelLock = tManualCmd.m_bPhaseToChannelLock;
		memcpy(&tValidManualCmd.m_tPhaseLockPara,&tManualCmd.m_tPhaseLockPara,sizeof(TPhaseLockPara));
		tValidManualCmd.m_bPreemptCtlCmd = false;
		m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "CreateValidManualCmd, Trans To ChannelLock");

		//黄闪，全红和关灯切方向
		if (m_nCurCtlMode == CTL_MODE_FLASH || m_nCurCtlMode == CTL_MODE_ALLRED || m_nCurCtlMode == CTL_MODE_OFF)
		{
			InitUsrCtlLogicObj();
			m_nCurCtlMode = CTL_MODE_MANUAL;
			tValidManualCmd.m_nCtlMode = CTL_MODE_MANUAL;
			tValidManualCmd.m_nSubCtlMode = CTL_MANUAL_SUBMODE_SYSTEM_CHANNEL_LOCK;//从黄闪，全红，关灯切到方向控制的直接把子模式置为通道锁定子模式
			tValidManualCmd.m_bChannelLockCmd = false;//切到通道锁定子模式下，清除当前通道锁定指令标志
		}
	}

	m_pLogicCtlStatus->SetValidManualCmd(tValidManualCmd);
}

/*==================================================================== 
函数名 ：UsrCtlYellowFalsh
功能 ：手动面板黄闪处理
算法实现 ： 
参数说明 ：tValidManualCmd：有效的手动面板指令
返回值说明：无
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 李永萍          创建函数 
====================================================================*/ 
void COpenATCLogicCtlManager::UsrCtlYellowFalsh(TManualCmd tValidManualCmd)
{
    if (m_nCurCtlMode != CTL_MODE_FLASH)
    {
		//创建黄闪模式
        CreateCtlMode(CTL_MODE_FLASH, CTL_SOURCE_LOCAL);

        SetPanelBtnStatusReply(tValidManualCmd, HWPANEL_BTN_YELLOW_FLASH);
    } 
}

/*==================================================================== 
函数名 ：UsrCtlAllRed
功能 ：手动面板全红处理
算法实现 ： 
参数说明 ：tValidManualCmd：有效的手动面板指令
返回值说明：无
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 李永萍          创建函数 
====================================================================*/ 
void COpenATCLogicCtlManager::UsrCtlAllRed(TManualCmd tValidManualCmd)
{
    if (m_nCurCtlMode != CTL_MODE_ALLRED)
    {
		//创建全红模式
        CreateCtlMode(CTL_MODE_ALLRED, CTL_SOURCE_LOCAL);

        SetPanelBtnStatusReply(tValidManualCmd, HWPANEL_BTN_ALL_RED);
    }
}

/*==================================================================== 
函数名 ：UsrCtlStepForward
功能 ：手动面板步进处理
算法实现 ： 
参数说明 ：tManualCmd：手动面板步进命令，tValidManualCmd:有效的手动面板指令
返回值说明：无
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 李永萍          创建函数 
====================================================================*/ 
void COpenATCLogicCtlManager::UsrCtlStepForward(TManualCmd tManualCmd, TManualCmd & tValidManualCmd)
{
    bool bCheckBtnUseStatus = CheckPanelBtnUseStatus(tManualCmd, tValidManualCmd);

	//步进能执行
	if (bCheckBtnUseStatus)
	{
		//生成有效的控制指令
	    CreateValidManualCmd(tManualCmd, tValidManualCmd);
		SetPanelBtnStatusReply(tValidManualCmd, HWPANEL_BTN_STEP);
	}
}

/*==================================================================== 
函数名 ：UsrCtlDirectionKey
功能 ：手动面板方向键处理
算法实现 ： 
参数说明 ：tManualCmd：手动面板步进命令，tValidManualCmd:有效的手动面板指令
返回值说明：无
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 李永萍          创建函数 
====================================================================*/ 
void COpenATCLogicCtlManager::UsrCtlDirectionKey(TManualCmd tManualCmd, TManualCmd & tValidManualCmd)
{
    bool bCheckBtnUseStatus = CheckPanelBtnUseStatus(tManualCmd, tValidManualCmd);

	//判断方向是否能执行
	if (bCheckBtnUseStatus)
	{
		//生成有效的控制指令
	    CreateValidManualCmd(tManualCmd, tValidManualCmd);
		SetPanelBtnStatusReply(tValidManualCmd, GetManualBtnByIndex(tManualCmd.m_tDirectionCmd.m_nNextDirectionIndex));
	}
}

/*==================================================================== 
函数名 ：CheckPanelBtnUseStatus
功能 ：判断面板按钮能否使用
算法实现 ： 
参数说明 ：tManualCmd，手动面板指令
返回值说明：无
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 李永萍          创建函数 
====================================================================*/ 
bool COpenATCLogicCtlManager::CheckPanelBtnUseStatus(TManualCmd tManualCmd, TManualCmd tValidManualCmd)
{
    if (tManualCmd.m_bStepForwardCmd)
    {
		//当前控制模式是黄闪，全红，关灯或者控制子模式为方向控制的时候，步进命令不能执行
        if (m_nCurCtlMode == CTL_MODE_FLASH || m_nCurCtlMode == CTL_MODE_ALLRED || m_nCurCtlMode == CTL_MODE_OFF || tValidManualCmd.m_nSubCtlMode == CTL_MANUAL_SUBMODE_PANEL_DIRECTION)
        {
			m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "StepForward can not be use");
			//面板按完自动后，方向开始过渡时，再按手动是不能执行的，此时为了硬件模块的m_bManualBtn能置false，需要再次返回自主按钮按下状态
			/*if (tValidManualCmd.m_nSubCtlMode == CTL_MANUAL_SUBMODE_PANEL_DIRECTION)
			{
				THWPanelBtnStatus tHWPanelBtnStatus;
				tHWPanelBtnStatus.m_nHWPanelBtnIndex = HWPANEL_BTN_AUTO;
				tHWPanelBtnStatus.m_bHWPanelBtnStatus = true;
				m_pLogicCtlStatus->SetPanelBtnStatusList(tHWPanelBtnStatus);
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "Set PanelAutoBtn Press Status");
			}
			WriteOperationRecord(CTL_SOURCE_LOCAL, HWPANEL_BTN_STEP, false, tManualCmd.m_szPeerIp);*/
            return false;
        }
    }
    else
    {
		if (!tValidManualCmd.m_bDirectionCmd && tValidManualCmd.m_nSubCtlMode == CTL_MANUAL_SUBMODE_PANEL_DIRECTION &&//当前正在进行方向控制
			tManualCmd.m_tDirectionCmd.m_nNextDirectionIndex == m_tOldValidManualCmd.m_tDirectionCmd.m_nNextDirectionIndex)
		{
			if (m_pLogicCtlMode->IsChannelGreen(CHANNEL_TYPE_DIRECTION, m_tOldValidManualCmd.m_tDirectionCmd.m_nNextDirectionIndex, NULL))
			{
				 m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "Same Direction Key, Direction is Green, can not be use");
				 return false;//同一个方向，并且该方向还在绿，命令不执行
			}
		}

        int nChannelCount = 0, nMaxChannelID = 0;
        TChannel atChannelInfo[MAX_CHANNEL_COUNT];            //通道信息表
        memset(atChannelInfo,0,sizeof(atChannelInfo));
        m_pLogicCtlParam->GetChannelTable(atChannelInfo);

        int i = 0, j = 0;
        for (i = 0;i < MAX_CHANNEL_COUNT;i++)
        {
			if (atChannelInfo[i].m_byChannelNumber > nMaxChannelID)
			{
				nMaxChannelID = atChannelInfo[i].m_byChannelNumber;
			}
        }
		nChannelCount = nMaxChannelID;

        TAscManualPanel tAscManualPanel;
        memset(&tAscManualPanel, 0, sizeof(TAscManualPanel)); 
        m_pLogicCtlParam->GetManualPanelInfo(tAscManualPanel);

        bool bAllChannelDefault = true;
        //获取当前通道需要切换的目标灯色
        for (i = 0;i < MAX_PANEL_KEY_CFG_COUNT;i++)
	    {
		    if (tAscManualPanel.m_stPanelKeyCfg[i].m_byKeyNum == tManualCmd.m_tDirectionCmd.m_nNextDirectionIndex)
		    {    
			    for (j = 0;j < nChannelCount;j++)
			    {
				    if (tAscManualPanel.m_stPanelKeyCfg[i].m_ChannelCfg[j].m_byChannelStatus != CHANNEL_STATUS_DEFAULT)
				    {
					    bAllChannelDefault = false;
						break;
				    }
			    }
		    } 
	    }

        if (bAllChannelDefault)//方向按钮对应的灯色值全部是默认，则不予响应
        {
            m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "KeyIndex:%d! All Lamp is Default!", tManualCmd.m_tDirectionCmd.m_nNextDirectionIndex);
            WriteOperationRecord(CTL_SOURCE_LOCAL, tManualCmd.m_tDirectionCmd.m_nNextDirectionIndex, false, tManualCmd.m_szPeerIp);
			return false;
        }

        bool bAllChannelRed = true;
        //获取当前通道需要切换的目标灯色
        for (i = 0;i < MAX_PANEL_KEY_CFG_COUNT;i++)
	    {
		    if (tAscManualPanel.m_stPanelKeyCfg[i].m_byKeyNum == tManualCmd.m_tDirectionCmd.m_nNextDirectionIndex)
		    {    
			    for (j = 0;j < nChannelCount;j++)
			    {
				    if (tAscManualPanel.m_stPanelKeyCfg[i].m_ChannelCfg[j].m_byChannelStatus != CHANNEL_STATUS_RED)
				    {
					    bAllChannelRed = false;
						break;
				    }
			    }
		    } 
	    }

        if (bAllChannelRed)//方向按钮对应的灯色值全部是全红，则不予响应
        {
            m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "KeyIndex:%d! All Lamp is Red!", tManualCmd.m_tDirectionCmd.m_nNextDirectionIndex);
			WriteOperationRecord(CTL_SOURCE_LOCAL, tManualCmd.m_tDirectionCmd.m_nNextDirectionIndex, false, tManualCmd.m_szPeerIp);
            return false;
        }
    }

    return true;
}

/*==================================================================== 
函数名 ：CheckSystemCmdUseStatus
功能 ：判断系统软件指令能否使用
算法实现 ： 
参数说明 ：tManualCmd，系统软件指令
返回值说明：无
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 李永萍          创建函数 
====================================================================*/ 
bool COpenATCLogicCtlManager::CheckSystemCmdUseStatus(TManualCmd tManualCmd, TManualCmd tValidManualCmd)
{
	bool bFlag = true;
    if (m_nCtlSource == CTL_SOURCE_LOCAL && m_nCurCtlMode != CTL_MODE_SELFCTL)
    {
		bFlag = false;
    }

	if (tManualCmd.m_bStepForwardCmd)
    {
        if (m_nCurCtlMode == CTL_MODE_ALLRED || m_nCurCtlMode == CTL_MODE_FLASH || m_nCurCtlMode == CTL_MODE_OFF)
		{
			bFlag = false;
		}
	}

	if (tManualCmd.m_bChannelLockCmd)
	{
		m_bInvalidPhaseCmd = false;
		if (!tValidManualCmd.m_bChannelLockCmd && tValidManualCmd.m_nSubCtlMode == CTL_MANUAL_SUBMODE_SYSTEM_CHANNEL_LOCK &&//当前正在进行通道锁定控制
			memcmp(tManualCmd.m_tChannelLockCmd.m_tNextChannelLockCtrlCmd.m_nChannelLockStatus, 
			m_tOldValidManualCmd.m_tChannelLockCmd.m_tNextChannelLockCtrlCmd.m_nChannelLockStatus, MAX_CHANNEL_COUNT) == 0)
		{
			if (m_pLogicCtlMode->IsChannelGreen(CHANNEL_TYPE_LOCK, 0, m_tOldValidManualCmd.m_tChannelLockCmd.m_tNextChannelLockCtrlCmd.m_nChannelLockStatus))
			{
				 m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "Same ChannelLock, Channel is Green, can not be use");
				 
				 memset(&m_tInvalidPhaseLockPara, 0, sizeof(tManualCmd.m_tPhaseLockPara));
				 if (tManualCmd.m_bPhaseToChannelLock)
				 {
					 m_bInvalidPhaseCmd = true;
					 memcpy(&m_tInvalidPhaseLockPara, &tManualCmd.m_tPhaseLockPara, sizeof(tManualCmd.m_tPhaseLockPara));
				 }
				 return false;//同一个通道锁定，并且该通道还在绿，命令不执行
			}
		}
	}

	if (bFlag)
	{
		if (tManualCmd.m_bPatternInterruptCmd)
		{
			WriteOperationRecord(CTL_SOURCE_SYSTEM, tManualCmd.m_tPatternInterruptCmd.m_nControlMode, true, tManualCmd.m_szPeerIp);	
		}
		else 
		{
			WriteOperationRecord(CTL_SOURCE_SYSTEM, CTL_MODE_MANUAL, true, tManualCmd.m_szPeerIp);	
		}
	}
	else
	{
		if (tManualCmd.m_bPatternInterruptCmd)
		{
			WriteOperationRecord(CTL_SOURCE_SYSTEM, tManualCmd.m_tPatternInterruptCmd.m_nControlMode, false, tManualCmd.m_szPeerIp);	
		}
		else 
		{
			WriteOperationRecord(CTL_SOURCE_SYSTEM, CTL_MODE_MANUAL, false, tManualCmd.m_szPeerIp);	
		}
	}

    return bFlag;

}

/*==================================================================== 
函数名 ：CheckPreemptCmdUseStatus
功能 ：判断优先控制指令能否使用
算法实现 ： 
参数说明 ：tPreemptCtlCmd，优先控制命令
返回值说明：无
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 李永萍          创建函数 
====================================================================*/ 
bool COpenATCLogicCtlManager::CheckPreemptCmdUseStatus(TPreemptCtlCmd tPreemptCtlCmd)
{
	bool bFlag = true;
    if (m_nCtlSource == CTL_SOURCE_LOCAL || m_nCtlSource == CTL_SOURCE_SYSTEM)
    {
		bFlag = false;
    }
	//本地或系统下发了方案干预指令，命令来源是优先控制的优先控制命令不能响应
	if (m_tOldPreemptCtlCmd.m_nCmdSource == CTL_SOURCE_SYSTEM && m_tOldPreemptCtlCmd.m_bPatternInterruptCmd && tPreemptCtlCmd.m_nCmdSource == CTL_SOURCE_PREEMPT)
    {
		bFlag = false;
		m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "GetPreemptCtlCmdFromList, System PatternInterrupt, PreemptCtlCmd Can't Execute");
    }

    return bFlag;

}

/*==================================================================== 
函数名 ：SetPanelBtnStatusReply
功能 ：设置面板按钮回复状态
算法实现 ： 
参数说明 ：tValidManualCmd：有效的手动面板指令，nHWPanelBtnIndex：按键编号
返回值说明：无
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 李永萍          创建函数 
====================================================================*/ 
void COpenATCLogicCtlManager::SetPanelBtnStatusReply(TManualCmd tValidManualCmd, int nHWPanelBtnIndex)
{
	THWPanelBtnStatus tHWPanelBtnStatus;
    tHWPanelBtnStatus.m_nHWPanelBtnIndex = nHWPanelBtnIndex;
    tHWPanelBtnStatus.m_bHWPanelBtnStatus = true;
	m_pLogicCtlStatus->SetPanelBtnStatusList(tHWPanelBtnStatus);

    memcpy(&m_tOldValidManualCmd,&tValidManualCmd,sizeof(TManualCmd));
     
    WriteOperationRecord(CTL_SOURCE_LOCAL, nHWPanelBtnIndex, true, m_atNetConfig[0].m_chAscNetCardIp);
}

/*==================================================================== 
函数名 ：WriteOperationRecord
功能 ：写操作日志
算法实现 ： 
参数说明 ：nCtlSource，控制源
           nOperationType，操作类型
           bStatus，操作结果
		   szPeerIp，IP
返回值说明：无
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 李永萍          创建函数 
====================================================================*/ 
void COpenATCLogicCtlManager::WriteOperationRecord(int nCtlSource, int nOperationType, bool bStatus, char szPeerIp[])
{
    if (bStatus)
    {
        TAscOperationRecord tAscOperationRecord;//操作记录
        memset(&tAscOperationRecord, 0, sizeof(tAscOperationRecord));

	    tAscOperationRecord.m_unStartTime = time(NULL);
        tAscOperationRecord.m_unEndTime = time(NULL);
        if (nCtlSource == CTL_SOURCE_LOCAL)
        {
	        tAscOperationRecord.m_bySubject = 2;
        }
        else if (nCtlSource == CTL_SOURCE_TZPARAM)
        {
            tAscOperationRecord.m_bySubject = 1;
        }
		else if (nCtlSource == CTL_SOURCE_SYSTEM)
        {
            tAscOperationRecord.m_bySubject = 0;
        }
	    tAscOperationRecord.m_byObject = 1;
        tAscOperationRecord.m_bStatus = true;
		if (nCtlSource == CTL_SOURCE_LOCAL)
		{
			tAscOperationRecord.m_nInfoType = LOCAL_MANUALPANEL;
			switch (nOperationType)
			{
			case HWPANEL_BTN_AUTO:
				{
					sprintf((char *)tAscOperationRecord.m_byFailureValue, "IP is %s, Local Control Enter into AutoCtl", m_atNetConfig[0].m_chAscNetCardIp);
				}
				break;
			case HWPANEL_BTN_MANUAL:
				{
					sprintf((char *)tAscOperationRecord.m_byFailureValue, "IP is %s, Local Control Enter into ManualCtl", m_atNetConfig[0].m_chAscNetCardIp);
				}
				break;
			case HWPANEL_BTN_STEP:
				{
					 sprintf((char *)tAscOperationRecord.m_byFailureValue, "IP is %s, Local Control Enter into StepForward", m_atNetConfig[0].m_chAscNetCardIp);
				}
				break;
			case HWPANEL_BTN_ALL_RED:
				{
					sprintf((char *)tAscOperationRecord.m_byFailureValue, "IP is %s, Local Control Enter into AllRed", m_atNetConfig[0].m_chAscNetCardIp);
				}
				break;
			case HWPANEL_BTN_YELLOW_FLASH:
				{
					sprintf((char *)tAscOperationRecord.m_byFailureValue, "IP is %s, Local Control Enter into YellowFlash", m_atNetConfig[0].m_chAscNetCardIp);
				}
				break;
			case HWPANEL_BTN_DIR_EAST_WEST_STRAIGHT:
				{
					sprintf((char *)tAscOperationRecord.m_byFailureValue, "IP is %s, Local Control Enter into direction control east west straight", m_atNetConfig[0].m_chAscNetCardIp);
				}
				break;
			case HWPANEL_BTN_DIR_EAST_WEST_TURN_LEFT:
				{
					sprintf((char *)tAscOperationRecord.m_byFailureValue, "IP is %s, Local Control Enter into direction control east west turn left", m_atNetConfig[0].m_chAscNetCardIp);
				}
				break;
			case HWPANEL_BTN_DIR_SOUTH_NORTH_STRAIGHT:
				{
					sprintf((char *)tAscOperationRecord.m_byFailureValue, "IP is %s, Local Control Enter into direction control south north straight", m_atNetConfig[0].m_chAscNetCardIp);
				}
				break;
			case HWPANEL_BTN_DIR_SOUTH_NORTH_TURN_LEFT:
				{
					sprintf((char *)tAscOperationRecord.m_byFailureValue, "IP is %s, Local Control Enter into direction control south north turn left", m_atNetConfig[0].m_chAscNetCardIp);
				}
				break;
			case HWPANEL_BTN_DIR_EAST:
				{
					sprintf((char *)tAscOperationRecord.m_byFailureValue, "IP is %s, Local Control Enter into direction control east", m_atNetConfig[0].m_chAscNetCardIp);
				}
				break;
			case HWPANEL_BTN_DIR_WEST:
				{
					sprintf((char *)tAscOperationRecord.m_byFailureValue, "IP is %s, Local Control Enter into direction control west", m_atNetConfig[0].m_chAscNetCardIp);
				}
				break;
			case HWPANEL_BTN_DIR_SOUTH:
				{
					sprintf((char *)tAscOperationRecord.m_byFailureValue, "IP is %s, Local Control Enter into direction control south", m_atNetConfig[0].m_chAscNetCardIp);
				}
				break;
			case HWPANEL_BTN_DIR_NORTH:
				{
					sprintf((char *)tAscOperationRecord.m_byFailureValue, "IP is %s, Local Control Enter into direction control north", m_atNetConfig[0].m_chAscNetCardIp);
				}
				break;
			case HWPANEL_BTN_REMOTE_CTRL:
				{
					 sprintf((char *)tAscOperationRecord.m_byFailureValue, "IP is %s, Local Control Enter into remote control", m_atNetConfig[0].m_chAscNetCardIp);
				}
				break;
			case HWPANEL_BTN_Y1:
				{
					sprintf((char *)tAscOperationRecord.m_byFailureValue, "IP is %s, Local Control Enter into Reserve button1", m_atNetConfig[0].m_chAscNetCardIp);
				}
				break;
			case HWPANEL_BTN_Y2:
				{
					sprintf((char *)tAscOperationRecord.m_byFailureValue, "IP is %s, Local Control Enter into Reserve button2", m_atNetConfig[0].m_chAscNetCardIp);
				}
				break;
			case HWPANEL_BTN_Y3:
				{
					sprintf((char *)tAscOperationRecord.m_byFailureValue, "IP is %s, Local Control Enter into Reserve button3", m_atNetConfig[0].m_chAscNetCardIp);
				}
				break;
			case HWPANEL_BTN_Y4:
				{
					sprintf((char *)tAscOperationRecord.m_byFailureValue, "IP is %s, Local Control Enter into Reserve button4", m_atNetConfig[0].m_chAscNetCardIp);
				}
				break;
			default:
				break;
			}
		}
		else
		{
			tAscOperationRecord.m_nInfoType = SYSTEM_MANUALCONTROL;
			switch (nOperationType)
			{
			case CTL_MODE_SELFCTL:
				{
					sprintf((char *)tAscOperationRecord.m_byFailureValue, "IP is %s, System Control Enter into SelfCtl", szPeerIp);
				}
				break;
			case CTL_MODE_FLASH:
				{
					sprintf((char *)tAscOperationRecord.m_byFailureValue, "IP is %s, System Control Enter into YellowFlash", szPeerIp);
				}
				break;
			case CTL_MODE_ALLRED:
				{
			        sprintf((char *)tAscOperationRecord.m_byFailureValue, "IP is %s, System Control Enter into AllRed", szPeerIp);
				}
				break;
			case CTL_MODE_OFF:
				{
					sprintf((char *)tAscOperationRecord.m_byFailureValue, "IP is %s, System Control Enter into Off", szPeerIp);
				}
				break;
			case CTL_MODE_MANUAL:
				{
					sprintf((char *)tAscOperationRecord.m_byFailureValue, "IP is %s, System Control Enter into StepForward", szPeerIp);
				}
				break;
			case CTL_MODE_FIXTIME:
				{
					sprintf((char *)tAscOperationRecord.m_byFailureValue, "IP is %s, System Control Enter into FixTime", szPeerIp);
				}
				break;
			case CTL_MODE_ACTUATE:
				{
					sprintf((char *)tAscOperationRecord.m_byFailureValue, "IP is %s, System Control Enter into Actuate", szPeerIp);
				}
				break;
			case CTL_MODE_SINGLEOPTIM:
				{
					sprintf((char *)tAscOperationRecord.m_byFailureValue, "IP is %s, System Control Enter into SingleOptim", szPeerIp);
				}
				break;
			case CTL_MODE_CABLELESS:
				{
					sprintf((char *)tAscOperationRecord.m_byFailureValue, "IP is %s, System Control Enter into Cableless", szPeerIp);
				}
				break;
			case CTL_MODE_PEDCROSTREET:
				{
					sprintf((char *)tAscOperationRecord.m_byFailureValue, "IP is %s, System Control Enter into PedCrostreet", szPeerIp);
				}
				break;
			case CTL_MODE_ACTUATE_PEDCROSTREET:
				{
					sprintf((char *)tAscOperationRecord.m_byFailureValue, "IP is %s, System Control Enter into ActuatePedCrostreet", szPeerIp);
				}
				break;
			case CTL_MODE_PHASE_LOCK:
				{
					sprintf((char *)tAscOperationRecord.m_byFailureValue, "IP is %s, System Control Enter into PhaseLock", szPeerIp);
				}
				break;
			case CTL_MODE_PHASE_PASS_CONTROL:
				{
					sprintf((char *)tAscOperationRecord.m_byFailureValue, "IP is %s, System Control Enter into PhaseControl", szPeerIp);
				}
				break;
			case CTL_MODE_MANUAL_CONTROL_PATTERN:
				{
					sprintf((char *)tAscOperationRecord.m_byFailureValue, "IP is %s, System Control Enter into ManualControlPattern", szPeerIp);
				}
				break;
			case CTL_MODE_PREEMPT:
				{
					sprintf((char *)tAscOperationRecord.m_byFailureValue, "IP is %s, System Control Enter into PreemptControl", szPeerIp);
				}
				break;
			default:
				break;
			}
		}

	    m_tOpenATCOperationRecord.SaveOneOperationRecordMessage(&tAscOperationRecord, m_pLogicCtlStatus);
    }
    else
    {
        TAscOperationRecord tAscOperationRecord;//操作记录
        memset(&tAscOperationRecord, 0, sizeof(tAscOperationRecord));

	    tAscOperationRecord.m_unStartTime = time(NULL);
        tAscOperationRecord.m_unEndTime = time(NULL);
        if (nCtlSource == CTL_SOURCE_LOCAL)
        {
	        tAscOperationRecord.m_bySubject = 2;
        }
        else if (nCtlSource == CTL_SOURCE_TZPARAM)
        {
            tAscOperationRecord.m_bySubject = 1;
        }
		else if (nCtlSource == CTL_SOURCE_SYSTEM)
        {
            tAscOperationRecord.m_bySubject = 0;
        }

	    tAscOperationRecord.m_byObject = 1;
        tAscOperationRecord.m_bStatus = false;

		bool bSave = false;

        if (nCtlSource == CTL_SOURCE_LOCAL)
        {
			tAscOperationRecord.m_nInfoType = LOCAL_MANUALPANEL;
			if (nOperationType == HWPANEL_BTN_STEP)
			{
				if (m_nCurCtlMode == CTL_MODE_FLASH)
				{
					bSave = true;
                    sprintf((char*)tAscOperationRecord.m_byFailureValue, "IP is %s, Mode is in YellowFlash, Local Control Enter into Stepward invalid", m_atNetConfig[0].m_chAscNetCardIp);
				}
				else if (m_nCurCtlMode == CTL_MODE_ALLRED)
				{
					bSave = true;
					sprintf((char*)tAscOperationRecord.m_byFailureValue, "IP is %s, Mode is in YellowFlash, Local Control Enter into Stepward invalid", m_atNetConfig[0].m_chAscNetCardIp);
				}
				else if (m_nCurCtlMode == CTL_MODE_OFF)
				{
					bSave = true;
					sprintf((char*)tAscOperationRecord.m_byFailureValue, "IP is %s, Mode is in YellowFlash, Local Control Enter into Stepward invalid", m_atNetConfig[0].m_chAscNetCardIp);
				}
				else
				{
					bSave = true;
                    sprintf((char*)tAscOperationRecord.m_byFailureValue, "IP is %s, Mode is in Direction, Local Control Enter into Stepward invalid", m_atNetConfig[0].m_chAscNetCardIp);
				}
			}
			else
			{
				bSave = true;
                sprintf((char*)tAscOperationRecord.m_byFailureValue, "IP is %s, Direction cmd is invalid, Local Control Enter into Direction invalid", m_atNetConfig[0].m_chAscNetCardIp);
			}
		
			if (bSave)
			{
                m_tOpenATCOperationRecord.SaveOneOperationRecordMessage(&tAscOperationRecord, m_pLogicCtlStatus); 
			}
        }
        else
        {
			tAscOperationRecord.m_nInfoType = SYSTEM_MANUALCONTROL;
			if (m_nCurCtlMode == CTL_MODE_ALLRED && nOperationType != CTL_MODE_SELFCTL)
			{
                 sprintf((char*)tAscOperationRecord.m_byFailureValue, "IP is %s, Mode is in AllRed, System Control Enter into non Selfctl invalid", szPeerIp);
			}
			else if (m_nCurCtlMode == CTL_MODE_FLASH  && nOperationType != CTL_MODE_SELFCTL)
			{
                 sprintf((char*)tAscOperationRecord.m_byFailureValue, "IP is %s, Mode is in YellowFlash, System Control Enter into non Selfctl invalid", szPeerIp);
			}
			else if (m_nCurCtlMode == CTL_MODE_OFF && nOperationType != CTL_MODE_SELFCTL)
			{
                sprintf((char*)tAscOperationRecord.m_byFailureValue, "IP is %s, Mode is in Off, System Control Enter into non Selfctl invalid", szPeerIp);
			}
			else if (nCtlSource == CTL_SOURCE_LOCAL && m_nCurCtlMode != CTL_MODE_SELFCTL)
            {
                sprintf((char*)tAscOperationRecord.m_byFailureValue, "IP is %s, Usr is in Control, System Control is invalid", m_atNetConfig[0].m_chAscNetCardIp);
            }
			else
			{
				sprintf((char*)tAscOperationRecord.m_byFailureValue, "IP is %s, Mode is in Direction, Local Control Enter into Stepward invalid", m_atNetConfig[0].m_chAscNetCardIp);
			}
			m_tOpenATCOperationRecord.SaveOneOperationRecordMessage(&tAscOperationRecord, m_pLogicCtlStatus);
        } 
    }
}

/*==================================================================== 
函数名 ：ProcLockChannelLampClr
功能 ：处理锁定通道灯色
算法实现 ： 
参数说明 ：无
返回值说明：无
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 李永萍          创建函数 
====================================================================*/
void COpenATCLogicCtlManager::ProcLockChannelLampClr()
{
    int i = 0, j = 0, nMaxChannelID = 0;

	int nYear = 0, nMonth = 0, nDay = 0, nHour = 0, nMin = 0, nSec = 0, nWeek = 0;
    OpenATCGetCurTime(nYear, nMonth, nDay, nHour, nMin, nSec, nWeek);

	int nChannelCount = 0;
	TChannel atChannelInfo[MAX_CHANNEL_COUNT]; 
	memset(atChannelInfo, 0, sizeof(atChannelInfo));
	TChannel atRealChannelInfo[MAX_CHANNEL_COUNT];
	memset(atRealChannelInfo, 0, sizeof(atRealChannelInfo));
	m_pLogicCtlParam->GetChannelTable(atRealChannelInfo);

	for (i = 0;i < MAX_CHANNEL_COUNT;i++)
	{
		atChannelInfo[i].m_byChannelNumber = i + 1;
	}

	for (i = 0;i < MAX_CHANNEL_COUNT;i++)
	{
		for (j = 0;j < MAX_CHANNEL_COUNT;j++)
		{
			if (atRealChannelInfo[j].m_byChannelNumber == 0)
			{
				continue;
			}

			if (atChannelInfo[i].m_byChannelNumber == atRealChannelInfo[j].m_byChannelNumber)
			{
				memcpy(&atChannelInfo[i], &atRealChannelInfo[j], sizeof(TChannel));
			}
		}

		if (atChannelInfo[i].m_byChannelControlSource == 0)
		{
			atChannelInfo[i].m_byChannelNumber = 0;
		}

		if (atChannelInfo[i].m_byChannelNumber > nMaxChannelID)
		{
			nMaxChannelID = atChannelInfo[i].m_byChannelNumber;
		}
	}

	nChannelCount = nMaxChannelID;

	TAscOnePlanChannelLockInfo  tAscOnePlanChannelLockInfo[MAX_SINGLE_DAYPLAN_COUNT];
	memset(tAscOnePlanChannelLockInfo,0,sizeof(tAscOnePlanChannelLockInfo));
	m_pLogicCtlParam->GetChannelLockInfo(tAscOnePlanChannelLockInfo);

	if (GetAllLockChannelCount(tAscOnePlanChannelLockInfo, nChannelCount) == 0)
	{
		//没有配置锁定通道则不作处理
        return;
	}

	bool bLockChannel = false;

	if (m_nCtlSource == CTL_SOURCE_SELF)
	{
		for (i = 0;i < MAX_SINGLE_DAYPLAN_COUNT;i++)
		{
			if (IsTimeInSpan(nHour, nMin, nSec, tAscOnePlanChannelLockInfo[i].m_byStartHour, tAscOnePlanChannelLockInfo[i].m_byStartMin, tAscOnePlanChannelLockInfo[i].m_byStartSec,
				tAscOnePlanChannelLockInfo[i].m_byEndHour, tAscOnePlanChannelLockInfo[i].m_byEndMin, tAscOnePlanChannelLockInfo[i].m_byEndSec))
			{
				bLockChannel = true;

				LockChannelCtl(tAscOnePlanChannelLockInfo[i],nChannelCount);
				break;
			}
		}
	}
	else
	{
		TManualCmd  tValidManualCmd;
		memset(&tValidManualCmd,0,sizeof(tValidManualCmd));
		m_pLogicCtlStatus->GetValidManualCmd(tValidManualCmd);
		if (tValidManualCmd.m_nSubCtlMode != CTL_MANUAL_SUBMODE_PANEL_DIRECTION && tValidManualCmd.m_nSubCtlMode != CTL_MANUAL_SUBMODE_SYSTEM_CHANNEL_LOCK)
		{
			for (i = 0;i < MAX_SINGLE_DAYPLAN_COUNT;i++)
			{
				if (IsTimeInSpan(nHour, nMin, nSec, tAscOnePlanChannelLockInfo[i].m_byStartHour, tAscOnePlanChannelLockInfo[i].m_byStartMin, tAscOnePlanChannelLockInfo[i].m_byStartSec,
					tAscOnePlanChannelLockInfo[i].m_byEndHour, tAscOnePlanChannelLockInfo[i].m_byEndMin, tAscOnePlanChannelLockInfo[i].m_byEndSec))
				{
					bLockChannel = true;

					LockChannelCtl(tAscOnePlanChannelLockInfo[i],nChannelCount);
					break;
				}
			}
		}
	}

	if (!bLockChannel)
	{
		int nLockChannelCount = 0;
		for (i = 0;i < nChannelCount;i++)
		{
			if (m_tOldAscOnePlanChannelLockInfo.m_stChannelLockStatus[i].m_byChannelLockStatus != LOCKCHANNEL_STATUS_DEFAULT)
			{
                nLockChannelCount += 1;
			}
		}

		if (nLockChannelCount > 0)
		{
			if (m_nCtlSource == CTL_SOURCE_SELF)
			{
				//没有找到时间段,则以最近一次的时间段的锁定通道灯色先过渡再关灯
				LockChannelOff(m_tOldAscOnePlanChannelLockInfo,nChannelCount); 	
			}
			else
			{
				TManualCmd  tValidManualCmd;
				memset(&tValidManualCmd,0,sizeof(tValidManualCmd));
				m_pLogicCtlStatus->GetValidManualCmd(tValidManualCmd);

				if (tValidManualCmd.m_nSubCtlMode == CTL_MANUAL_SUBMODE_PANEL_DIRECTION || tValidManualCmd.m_nSubCtlMode == CTL_MANUAL_SUBMODE_SYSTEM_CHANNEL_LOCK)
				{
					LockChannelTransToDirection(m_tOldAscOnePlanChannelLockInfo,nChannelCount); 	
				}
			}
		}
	}
}

/*==================================================================== 
函数名 ：LockChannelCtl
功能 ：锁定通道的灯色控制
算法实现 ： 
参数说明 ：tAscOnePlanChannelLockInfo，锁定通道的时间段信息
           nChannelCount，通道数量
返回值说明：无
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 李永萍          创建函数 
====================================================================*/
void COpenATCLogicCtlManager::LockChannelCtl(TAscOnePlanChannelLockInfo tAscOnePlanChannelLockInfo, int nChannelCount)
{
    TLampClrStatus tLampClrStatus;
	memset(&tLampClrStatus,0,sizeof(TLampClrStatus)); 
	m_pLogicCtlStatus->GetLampClrStatus(tLampClrStatus);

	BYTE byChannelID = 0;

	bool bLockChannelTrans = true;
	
	int i = 0;		
	for (i = 0;i < nChannelCount;i++)
	{
		if (tAscOnePlanChannelLockInfo.m_stChannelLockStatus[i].m_byChannelLockStatus != LOCKCHANNEL_STATUS_DEFAULT)
		{
			byChannelID = tAscOnePlanChannelLockInfo.m_stChannelLockStatus[i].m_byChannelID;

			char * pStartPos = NULL;
			pStartPos = tLampClrStatus.m_achLampClr + (byChannelID - 1) * 3;

			if ((int)tAscOnePlanChannelLockInfo.m_stChannelLockStatus[i].m_byChannelLockStatus == LOCKCHANNEL_STATUS_GREEN)
			{
				if (m_nLockChannelTransStatus[i] == LOCKCHANNEL_STATUS_GREENFLASH || m_nLockChannelTransStatus[i] == LOCKCHANNEL_STATUS_YELLOW)
				{
					if (!LockChannelTrans(tAscOnePlanChannelLockInfo, i, false, tLampClrStatus))//如果当前通道正过渡到绿闪或黄灯时，对时对回到绿灯时段
					{
						bLockChannelTrans = false;
					}
				}
				else
				{
					*pStartPos = (char)LAMP_CLR_OFF;
					*(pStartPos + 1) = (char)LAMP_CLR_OFF;
					*(pStartPos + 2) = (char)LAMP_CLR_ON; 

					m_nLockChannelTransStatus[i] = LOCKCHANNEL_STATUS_GREEN;
				}
			}
			else if ((int)tAscOnePlanChannelLockInfo.m_stChannelLockStatus[i].m_byChannelLockStatus == LOCKCHANNEL_STATUS_GREENFLASH)
			{
				if (m_nLockChannelTransStatus[i] == LOCKCHANNEL_STATUS_GREENFLASH || m_nLockChannelTransStatus[i] == LOCKCHANNEL_STATUS_YELLOW)
				{
					if (!LockChannelTrans(tAscOnePlanChannelLockInfo, i, false, tLampClrStatus))//如果当前通道正过渡到绿闪或黄灯时，对时对回到绿闪时段
					{
						bLockChannelTrans = false;
					}
				}
				else
				{
					*pStartPos = (char)LAMP_CLR_OFF;
					*(pStartPos + 1) = (char)LAMP_CLR_OFF;
					*(pStartPos + 2) = (char)LAMP_CLR_FLASH; 

					int nDiff = GetDiffTime(tAscOnePlanChannelLockInfo.m_byStartHour, tAscOnePlanChannelLockInfo.m_byStartMin,
						tAscOnePlanChannelLockInfo.m_byStartSec, tAscOnePlanChannelLockInfo.m_byEndHour,
						tAscOnePlanChannelLockInfo.m_byEndMin, tAscOnePlanChannelLockInfo.m_byEndSec);
					m_pLogicCtlStatus->SetGreenFlashCount((byChannelID - 1) * 3 + 2, nDiff * 2);
				}
			}
			else if ((int)tAscOnePlanChannelLockInfo.m_stChannelLockStatus[i].m_byChannelLockStatus == LOCKCHANNEL_STATUS_YELLOW)
			{
				if (m_nLockChannelTransStatus[i] == LOCKCHANNEL_STATUS_GREENFLASH || m_nLockChannelTransStatus[i] == LOCKCHANNEL_STATUS_YELLOW)
				{
					if (!LockChannelTrans(tAscOnePlanChannelLockInfo, i, false, tLampClrStatus))//如果当前通道正过渡到绿闪或黄灯时，对时对回到绿灯时段
					{
						bLockChannelTrans = false;
					}
				}
				else
				{
					*pStartPos = (char)LAMP_CLR_OFF;
					*(pStartPos + 1) = (char)LAMP_CLR_ON;
					*(pStartPos + 2) = (char)LAMP_CLR_OFF; 
				}
			}
			else if ((int)tAscOnePlanChannelLockInfo.m_stChannelLockStatus[i].m_byChannelLockStatus == LOCKCHANNEL_STATUS_RED)
			{
 				if (!LockChannelTrans(tAscOnePlanChannelLockInfo, i, false, tLampClrStatus))//如果当前通道是绿灯则需要过渡到红灯
				{
                    bLockChannelTrans = false;
				}
			}
			else if ((int)tAscOnePlanChannelLockInfo.m_stChannelLockStatus[i].m_byChannelLockStatus == LOCKCHANNEL_STATUS_REDFLASH)
			{
				if (m_nLockChannelTransStatus[i] == LOCKCHANNEL_STATUS_GREENFLASH || m_nLockChannelTransStatus[i] == LOCKCHANNEL_STATUS_YELLOW)
				{
					if (!LockChannelTrans(tAscOnePlanChannelLockInfo, i, false, tLampClrStatus))//如果当前通道正过渡到绿闪或黄灯时，对时对回到绿灯时段
					{
						bLockChannelTrans = false;
					}
				}
				else
				{
					*pStartPos = (char)LAMP_CLR_FLASH;
					*(pStartPos + 1) = (char)LAMP_CLR_OFF;
					*(pStartPos + 2) = (char)LAMP_CLR_OFF; 

					int nDiff = GetDiffTime(tAscOnePlanChannelLockInfo.m_byStartHour, tAscOnePlanChannelLockInfo.m_byStartMin,
						tAscOnePlanChannelLockInfo.m_byStartSec, tAscOnePlanChannelLockInfo.m_byEndHour,
						tAscOnePlanChannelLockInfo.m_byEndMin, tAscOnePlanChannelLockInfo.m_byEndSec);
					m_pLogicCtlStatus->SetRedFlashCount((byChannelID - 1) * 3, nDiff * 2);
				}
			}
			else if ((int)tAscOnePlanChannelLockInfo.m_stChannelLockStatus[i].m_byChannelLockStatus == LOCKCHANNEL_STATUS_OFF)
			{
				if (!LockChannelTrans(tAscOnePlanChannelLockInfo, i, true, tLampClrStatus))//如果当前通道是绿灯则需要过渡到关灯
				{
				    bLockChannelTrans = false;
				}		
			}
		} 
	}

	m_pLogicCtlStatus->SetLampClrStatus(tLampClrStatus);
	
	if (bLockChannelTrans)
	{
        if (memcmp(&m_tOldAscOnePlanChannelLockInfo,&tAscOnePlanChannelLockInfo,sizeof(m_tOldAscOnePlanChannelLockInfo)) != 0)
		{
			memcpy(&m_tOldAscOnePlanChannelLockInfo,&tAscOnePlanChannelLockInfo,sizeof(m_tOldAscOnePlanChannelLockInfo));
		}
	}
}

/*==================================================================== 
函数名 ：LockChannelOff
功能 ：锁定通道的灯色关闭
算法实现 ： 
参数说明 ：tAscOnePlanChannelLockInfo，锁定通道的时间段信息
           nChannelCount，通道数量
返回值说明：无
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 李永萍          创建函数 
====================================================================*/
void COpenATCLogicCtlManager::LockChannelOff(TAscOnePlanChannelLockInfo tAscOnePlanChannelLockInfo, int nChannelCount)
{
	TLampClrStatus tLampClrStatus;
	memset(&tLampClrStatus,0,sizeof(TLampClrStatus)); 
    m_pLogicCtlStatus->GetLampClrStatus(tLampClrStatus);

	bool bLockChannelTrans = false;

	static int nCount = 0;
	int  nLockChannelCount = 0;
	int  i = 0;
	for (i = 0;i < nChannelCount;i++)
	{
		if (tAscOnePlanChannelLockInfo.m_stChannelLockStatus[i].m_byChannelLockStatus != LOCKCHANNEL_STATUS_DEFAULT)
		{
            nLockChannelCount += 1;
		}
	}
	
	for (i = 0;i < nChannelCount;i++)
	{
		if (LockChannelTrans(tAscOnePlanChannelLockInfo, i, true, tLampClrStatus))
		{
            nCount += 1;	
		}

		if (nCount == nLockChannelCount)
		{
			bLockChannelTrans = true;
		}
	}

	m_pLogicCtlStatus->SetLampClrStatus(tLampClrStatus);

	if (bLockChannelTrans)
	{
		nCount = 0;
		memset(&m_tOldAscOnePlanChannelLockInfo,0,sizeof(m_tOldAscOnePlanChannelLockInfo));
	}
}

/*==================================================================== 
函数名 ：LockChannelTrans
功能 ：锁定通道的灯色过渡
算法实现 ： 
参数说明 ：tAscOnePlanChannelLockInfo，锁定通道的时间段信息
           nChannelIndex，通道编号
		   tLampClrStatus，灯色
返回值说明：无
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 李永萍          创建函数 
====================================================================*/
bool COpenATCLogicCtlManager::LockChannelTrans(TAscOnePlanChannelLockInfo tAscOnePlanChannelLockInfo, int nChannelIndex, bool bLockChannelOff, TLampClrStatus & tLampClrStatus)
{
	unsigned long  nGlobalCounter = m_pLogicCtlStatus->GetGlobalCounter();

	BYTE byChannelID = 0;

	bool bLockChannelTrans = false;

	if (tAscOnePlanChannelLockInfo.m_stChannelLockStatus[nChannelIndex].m_byChannelLockStatus != LOCKCHANNEL_STATUS_DEFAULT)
	{
		unsigned long nCounterDiff = CalcCounter(m_nLockChannelCounter[nChannelIndex], nGlobalCounter, C_N_MAXGLOBALCOUNTER);

		byChannelID = tAscOnePlanChannelLockInfo.m_stChannelLockStatus[nChannelIndex].m_byChannelID;

		char * pStartPos = NULL;
		pStartPos = tLampClrStatus.m_achLampClr + (byChannelID - 1) * 3;

		if (m_nLockChannelTransStatus[nChannelIndex] == LOCKCHANNEL_STATUS_GREEN)//绿灯过渡
		{
			if (m_tOldAscOnePlanChannelLockInfo.m_byGreenFlash > 0)
			{
				m_nLockChannelCounter[nChannelIndex] = nGlobalCounter;

				*pStartPos = (char)LAMP_CLR_OFF;
				*(pStartPos + 1) = (char)LAMP_CLR_OFF;
				*(pStartPos + 2) = (char)LAMP_CLR_FLASH; 

				m_pLogicCtlStatus->SetGreenFlashCount((byChannelID - 1) * 3 + 2, m_tOldAscOnePlanChannelLockInfo.m_byGreenFlash * 2);
				
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "LockChannelTrans Channel:%d Green To GreenFlash", nChannelIndex);

				m_nLockChannelTransStatus[nChannelIndex] = LOCKCHANNEL_STATUS_GREENFLASH;
			}
			else if (m_tOldAscOnePlanChannelLockInfo.m_byYellow > 0)
			{
                m_nLockChannelCounter[nChannelIndex] = nGlobalCounter;

				*pStartPos = (char)LAMP_CLR_OFF;
				*(pStartPos + 1) = (char)LAMP_CLR_ON;
				*(pStartPos + 2) = (char)LAMP_CLR_OFF; 
				
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "LockChannelTrans Channel:%d Green To Yellow", nChannelIndex);

				m_nLockChannelTransStatus[nChannelIndex] = LOCKCHANNEL_STATUS_YELLOW; 
			}
			else
			{
				if (!bLockChannelOff)
				{
					*pStartPos = (char)LAMP_CLR_ON;
					*(pStartPos + 1) = (char)LAMP_CLR_OFF;
					*(pStartPos + 2) = (char)LAMP_CLR_OFF;

					m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "LockChannelTrans Channel:%d Green To Red", nChannelIndex);

					m_nLockChannelTransStatus[nChannelIndex] = LOCKCHANNEL_STATUS_RED;//绿灯直接过渡到红灯
				}
				else
				{
					*pStartPos = (char)LAMP_CLR_OFF;
					*(pStartPos + 1) = (char)LAMP_CLR_OFF;
					*(pStartPos + 2) = (char)LAMP_CLR_OFF;

					m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "LockChannelTrans Channel:%d Green To Off", nChannelIndex);

					m_nLockChannelTransStatus[nChannelIndex] = LOCKCHANNEL_STATUS_OFF;//绿灯直接过渡到关灯
				}

				bLockChannelTrans = true;//过渡成功
			}
		}
		else if (m_nLockChannelTransStatus[nChannelIndex] == LOCKCHANNEL_STATUS_GREENFLASH)//绿闪过渡
		{
			if (nCounterDiff >= (m_tOldAscOnePlanChannelLockInfo.m_byGreenFlash * C_N_TIMER_TIMER_COUNTER))
			{
				if (m_tOldAscOnePlanChannelLockInfo.m_byYellow > 0)
				{
					m_nLockChannelCounter[nChannelIndex] = nGlobalCounter;

					*pStartPos = (char)LAMP_CLR_OFF;
					*(pStartPos + 1) = (char)LAMP_CLR_ON;
					*(pStartPos + 2) = (char)LAMP_CLR_OFF;
				
					m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "LockChannelTrans Channel:%d GreenFlash To Yellow", nChannelIndex);

					m_nLockChannelTransStatus[nChannelIndex] = LOCKCHANNEL_STATUS_YELLOW;
				}
				else
				{
                    if (!bLockChannelOff)
					{
						*pStartPos = (char)LAMP_CLR_ON;
						*(pStartPos + 1) = (char)LAMP_CLR_OFF;
						*(pStartPos + 2) = (char)LAMP_CLR_OFF;

						m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "LockChannelTrans Channel:%d GreenFlash To Red", nChannelIndex);

						m_nLockChannelTransStatus[nChannelIndex] = LOCKCHANNEL_STATUS_RED;//绿闪直接过渡到红灯
					}
					else
					{
						*pStartPos = (char)LAMP_CLR_OFF;
						*(pStartPos + 1) = (char)LAMP_CLR_OFF;
						*(pStartPos + 2) = (char)LAMP_CLR_OFF;

						m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "LockChannelTrans Channel:%d GreenFlash To Off", nChannelIndex);

						m_nLockChannelTransStatus[nChannelIndex] = LOCKCHANNEL_STATUS_OFF;//绿闪直接过渡到关灯
					}

					bLockChannelTrans = true;//过渡成功
				}
			}
			else
			{
				*pStartPos = (char)LAMP_CLR_OFF;
				*(pStartPos + 1) = (char)LAMP_CLR_OFF;
				*(pStartPos + 2) = (char)LAMP_CLR_FLASH; 
			}
			
		}
		else if (m_nLockChannelTransStatus[nChannelIndex] == LOCKCHANNEL_STATUS_YELLOW)//黄灯过渡
		{
			if (nCounterDiff >= (m_tOldAscOnePlanChannelLockInfo.m_byYellow * C_N_TIMER_TIMER_COUNTER))
			{
                m_nLockChannelCounter[nChannelIndex] = nGlobalCounter;

				*pStartPos = (char)LAMP_CLR_ON;
				*(pStartPos + 1) = (char)LAMP_CLR_OFF;
				*(pStartPos + 2) = (char)LAMP_CLR_OFF;
				
                m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "LockChannelTrans Channel:%d Yellow To Red", nChannelIndex);

				m_nLockChannelTransStatus[nChannelIndex] = LOCKCHANNEL_STATUS_RED;

				bLockChannelTrans = true;//过渡成功
			}
			else
			{
				*pStartPos = (char)LAMP_CLR_OFF;
				*(pStartPos + 1) = (char)LAMP_CLR_ON;
				*(pStartPos + 2) = (char)LAMP_CLR_OFF;
			}
		}
		else
		{
			if (!bLockChannelOff)
			{
				*pStartPos = (char)LAMP_CLR_ON;
				*(pStartPos + 1) = (char)LAMP_CLR_OFF;
				*(pStartPos + 2) = (char)LAMP_CLR_OFF;

				m_nLockChannelTransStatus[nChannelIndex] = LOCKCHANNEL_STATUS_RED;//过渡到红灯
			}
			else
			{
			    *pStartPos = (char)LAMP_CLR_OFF;
				*(pStartPos + 1) = (char)LAMP_CLR_OFF;
				*(pStartPos + 2) = (char)LAMP_CLR_OFF;

				m_nLockChannelTransStatus[nChannelIndex] = LOCKCHANNEL_STATUS_OFF;//过渡到关灯

				//m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "LockChannelTrans Channel:%d To Off", nChannelIndex);
			}

			bLockChannelTrans = true;//过渡成功
		}
	}	

	return bLockChannelTrans;
}

/*==================================================================== 
函数名 ：GetAllLockChannelCount
功能 ：获取所有时间段的锁定通道数量
算法实现 ： 
参数说明 ：tAscOnePlanChannelLockInfo，锁定通道的时间段信息
           nChannelCount，通道数量
返回值说明：所有时间段的锁定通道数量
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 李永萍          创建函数 
====================================================================*/
int COpenATCLogicCtlManager::GetAllLockChannelCount(TAscOnePlanChannelLockInfo tAscOnePlanChannelLockInfo[], int nChannelCount)
{
	int i = 0, j = 0;
	int nAllLockChannelCount = 0;

	for (i = 0;i < MAX_SINGLE_DAYPLAN_COUNT;i++)
	{
		for (j = 0;j < nChannelCount;j++)
		{
			if (tAscOnePlanChannelLockInfo[i].m_stChannelLockStatus[j].m_byChannelLockStatus != LOCKCHANNEL_STATUS_DEFAULT)
			{
                nAllLockChannelCount += 1;
			}
		}
	}

	return nAllLockChannelCount;
}

/*==================================================================== 
函数名 ：IsTimeInSpan
功能 ：判断当前时间是否在时间段内
算法实现 ： 
参数说明 ：
           nCurHour，当前时
		   nCurMin，当前分
		   nCurSec，当前秒
           nStartHour，起始时
           nStartMin，起始分
		   nStartSec，起始秒
           nEndHour，结束时
		   nEndMin，结束分
		   nEndSec，结束秒
返回值说明：当前时间是否在时间段内
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 李永萍          创建函数 
====================================================================*/
bool COpenATCLogicCtlManager::IsTimeInSpan(int nCurHour, int nCurMin, int nCurSec, int nStartHour, int nStartMin, int nStartSec, int nEndHour, int nEndMin, int nEndSec)
{
    int iBegLessThanEnd = 0; //1-配置的开始时间小于结束时间 0-配置的开始时间大于结束时间

	if (nStartHour < nEndHour || (nStartHour == nEndHour && nStartMin <= nEndMin))
	{
		iBegLessThanEnd = 1;
	}
	else
	{
		iBegLessThanEnd = 0;
	}

	if (iBegLessThanEnd)   //开始时间小于结束时间
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
	else   //开始时间大于结束时间, 跨天的情况
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
函数名 ：GetDiffTime
功能 ：获取时间差(秒数)
算法实现 ： 
参数说明 ：nStartHour，起始时
           nStartMin，起始分
		   nStartSec，起始秒
           nEndHour，结束时
		   nEndMin，结束分
		   nEndSec，结束秒
返回值说明：无
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 李永萍          创建函数 
====================================================================*/
int  COpenATCLogicCtlManager::GetDiffTime(int nStartHour, int nStartMin, int nStartSec, int nEndHour, int nEndMin, int nEndSec)
{
	int nHour = 0, nMin = 0, nSec = 0;

	if (nEndSec < nStartSec)
    {
        nSec = 60 - nStartSec + nEndSec;
    }
    else
    {
        nSec = nEndSec - nStartSec;
    }

    if (nEndMin < nStartMin)
    {
        nMin = nEndMin + 60 - nStartMin;
        nEndHour--;
    }
    else
    {
        nMin = nEndMin - nStartMin;
    }

    if (nEndHour < nStartHour)
    {
        nHour = nEndHour + 24 - nStartHour;
    }
    else
    {
        nHour = nEndHour - nStartHour;
    }

	return nHour * 3600 + nMin * 60 + nSec;
}

/*==================================================================== 
函数名 ：GetManualBtnByIndex
功能 ：根据索引获取面板按钮编号
算法实现 ： 
参数说明 ：索引
返回值说明：按钮编号
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2020/05/14     V1.0                 创建函数 
====================================================================*/ 
int COpenATCLogicCtlManager::GetManualBtnByIndex(int nIndex)
{
    switch (nIndex)
	{
	case HWPANEL_DIR_EAST_WEST_STRAIGHT_INDEX:
		return HWPANEL_BTN_DIR_EAST_WEST_STRAIGHT;
		break;
	case HWPANEL_DIR_NORTH_INDEX:
		return HWPANEL_BTN_DIR_NORTH;
		break;
	case HWPANEL_DIR_EAST_WEST_TURN_LEFT_INDEX:
		return HWPANEL_BTN_DIR_EAST_WEST_TURN_LEFT;
		break;
	case HWPANEL_DIR_WEST_INDEX:
		return HWPANEL_BTN_DIR_WEST;
		break;
	case HWPANEL_DIR_EAST_INDEX:
		return HWPANEL_BTN_DIR_EAST;
		break;
    case HWPANEL_DIR_SOUTH_NORTH_STRAIGHT_INDEX:
		return HWPANEL_BTN_DIR_SOUTH_NORTH_STRAIGHT;
		break;
    case HWPANEL_DIR_SOUTH_INDEX:
		return HWPANEL_BTN_DIR_SOUTH;
		break;
    case HWPANEL_DIR_SOUTH_NORTH_TURN_LEFT_INDEX:
		return HWPANEL_BTN_DIR_SOUTH_NORTH_TURN_LEFT;
		break;
    case HWPANEL_Y1_INDEX:
		return HWPANEL_BTN_Y1;
		break;
    case HWPANEL_Y2_INDEX:
		return HWPANEL_BTN_Y2;
		break;
    case HWPANEL_Y3_INDEX:
		return HWPANEL_BTN_Y3;
		break;
    case HWPANEL_Y4_INDEX:
		return HWPANEL_BTN_Y4;
		break;
    case HWPANEL_REMOTE_CTRL_INDEX:
		return HWPANEL_BTN_REMOTE_CTRL;
		break;
	default:
		return 0;
		break;
    }
}

/*==================================================================== 
函数名 ：CalcCounter
功能 ：计算计数
算法实现 ： 
参数说明 ：nStart，起始计数
           nEnd，结束计数
		   nMax，全局计数最大值
返回值说明：计数差
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2020/05/14     V1.0                 创建函数 
====================================================================*/ 
unsigned long COpenATCLogicCtlManager::CalcCounter(unsigned long nStart,unsigned long nEnd,unsigned long nMax)
{
    if (nEnd >= nStart)
    {
        return (nEnd - nStart);
    }
    else
    {
        return (nEnd + nMax - nStart);
    }
}

/*==================================================================== 
函数名 ：OpenATCGetCurTime
功能 ：获取当前时间
算法实现 ： 
参数说明 ：当前时间
返回值说明：无
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2020/05/14     V1.0                 创建函数 
====================================================================*/ 
void COpenATCLogicCtlManager::OpenATCGetCurTime(int & nYear, int & nMon, int & nDay, int & nHour, int & nMin, int & nSec, int & nWeek)
{
#ifdef _WIN32
    SYSTEMTIME		stLocal;
    GetLocalTime(&stLocal);
    nYear = stLocal.wYear;
    nMon  = stLocal.wMonth;
    nDay  = stLocal.wDay;
    nWeek = stLocal.wDayOfWeek;
    nHour = stLocal.wHour;
    nMin  = stLocal.wMinute;
    nSec  = stLocal.wSecond;
#else
    long dwCurTime = time(NULL);
    struct tm tLocalTime = {0};
    localtime_r(&dwCurTime, &tLocalTime);
    nYear = 1900 + tLocalTime.tm_year;
    nMon  = 1 + tLocalTime.tm_mon;
    nDay  = tLocalTime.tm_mday;
    nWeek = tLocalTime.tm_wday;
    nHour = tLocalTime.tm_hour;
    nMin  = tLocalTime.tm_min;
    nSec  = tLocalTime.tm_sec;
#endif
}

/*==================================================================== 
函数名 ：SetAllRedStage
功能 ：设置进入全红阶段
算法实现 ： 
参数说明 ：无
返回值说明：无
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2020/05/14     V1.0                 创建函数 
====================================================================*/ 
void COpenATCLogicCtlManager::SetAllRedStage()
{
	if (m_nLogicCtlStage != CTL_STAGE_STARTUP_RED)
	{
		m_nLogicCtlStage = CTL_STAGE_STARTUP_RED;   
		TPhaseLampClrRunCounter tRunCounter;
		m_pLogicCtlStatus->GetPhaseLampClrRunCounter(tRunCounter);
		tRunCounter.m_nLampClrTime[0] = 0;
		tRunCounter.m_nLampClrStartCounter[0] = tRunCounter.m_nCurCounter;
		m_pLogicCtlStatus->SetPhaseLampClrRunCounter(tRunCounter);

		TLogicCtlStatus tCtlStatus;
		m_pLogicCtlStatus->GetLogicCtlStatus(tCtlStatus);
		tCtlStatus.m_nRunStage = CTL_STAGE_STARTUP_RED;
		m_pLogicCtlStatus->SetLogicCtlStatus(tCtlStatus);

		if (m_pLogicCtlMode != NULL)
        {
            m_pLogicCtlMode->Release();
            delete m_pLogicCtlMode;
            m_pLogicCtlMode = NULL;
        }
        m_pLogicCtlMode = CLogicCtlModeSimpleFactory::Create(CTL_MODE_ALLRED);
        m_pLogicCtlMode->Init(m_pLogicCtlParam,m_pLogicCtlStatus,m_pOpenATCLog,0);
        m_pLogicCtlMode->Run();

		m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "FaultDetBoard Lose of Control, COpenATCLogicCtlManager SetAllRedStage!");
	}
}

/*==================================================================== 
函数名 ：SetSystemControlStatus
功能 ：设置系统控制命令状态
算法实现 ： 
参数说明 ：命令的结果和失败原因
返回值说明：无
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2020/05/14     V1.0                 创建函数 
====================================================================*/ 
void COpenATCLogicCtlManager::SetSystemControlStatus(bool bSpecicalControlResult, int nSpecicalControlFailCode,
	bool bPatternControlResult, int nPatternControlFailCode, 
	bool bStageControlResult, int nStageControlFailCode, 
	bool bPhaseControlResult, int nPhaseControlFailCode, 
	bool bChannelLockResult, int nChannelLockFailCode)
{
	TSystemControlStatus tSystemControlStatus;
	memset(&tSystemControlStatus, 0, sizeof(tSystemControlStatus));
	m_pLogicCtlStatus->GetSystemControlStatus(tSystemControlStatus);
	tSystemControlStatus.m_nSpecicalControlResult = bSpecicalControlResult;
	tSystemControlStatus.m_nSpecicalControlFailCode = nSpecicalControlFailCode;
	tSystemControlStatus.m_nPatternControlResult = bPatternControlResult;
	tSystemControlStatus.m_nPatternControlFailCode = nPatternControlFailCode;
	tSystemControlStatus.m_nStageControlResult = bStageControlResult;
	tSystemControlStatus.m_nStageControlFailCode = nStageControlFailCode;
	tSystemControlStatus.m_nPhaseControlResult = bPhaseControlResult;
	tSystemControlStatus.m_nPhaseControlFailCode = nPhaseControlFailCode;
	tSystemControlStatus.m_nChannelLockResult = bChannelLockResult;
	tSystemControlStatus.m_nChannelLockFailCode = nChannelLockFailCode;
	m_pLogicCtlStatus->SetSystemControlStatus(tSystemControlStatus);
}

/*==================================================================== 
函数名 ：SetPreemptControlStatus
功能 ：设置优先控制命令状态
算法实现 ： 
参数说明 ：命令的结果和失败原因
返回值说明：无
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2020/05/14     V1.0                 创建函数 
====================================================================*/ 
void COpenATCLogicCtlManager::SetPreemptControlStatus(bool bPreemptControlResult, int nPreemptControlFailCode)
{
	TPreemptControlStatus tPreemptControlStatus;
	memset(&tPreemptControlStatus, 0, sizeof(tPreemptControlStatus));
	m_pLogicCtlStatus->GetPreemptControlStatus(tPreemptControlStatus);
	tPreemptControlStatus.m_nPreemptControlResult = bPreemptControlResult;
	tPreemptControlStatus.m_nPreemptControlResultFailCode = nPreemptControlFailCode;
	m_pLogicCtlStatus->SetPreemptControlStatus(tPreemptControlStatus);
}
/*==================================================================== 
函数名 ：LockChannelTransToDirection
功能 ：锁定通道的灯色过渡完再切到方向
算法实现 ： 
参数说明 ：tAscOnePlanChannelLockInfo，锁定通道的时间段信息
           nChannelCount，通道数量
返回值说明：无
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 李永萍          创建函数 
====================================================================*/
void COpenATCLogicCtlManager::LockChannelTransToDirection(TAscOnePlanChannelLockInfo tAscOnePlanChannelLockInfo, int nChannelCount)
{
	TLampClrStatus tLampClrStatus;
	memset(&tLampClrStatus,0,sizeof(TLampClrStatus)); 
    m_pLogicCtlStatus->GetLampClrStatus(tLampClrStatus);

	bool bLockChannelTrans = false;//过渡结束标志

	static int nCount = 0;
	int  nLockChannelCount = 0;
	int  i = 0, j = 0;
	for (j = 0;j < nChannelCount;j++)
	{
		if (tAscOnePlanChannelLockInfo.m_stChannelLockStatus[j].m_byChannelLockStatus != LOCKCHANNEL_STATUS_DEFAULT)
		{
            nLockChannelCount += 1;
		}
	}

	TAscManualPanel tAscManualPanel;
	memset(&tAscManualPanel, 0, sizeof(TAscManualPanel)); 
	m_pLogicCtlParam->GetManualPanelInfo(tAscManualPanel);

	TManualCmd  tValidManualCmd;
	memset(&tValidManualCmd,0,sizeof(tValidManualCmd));
	m_pLogicCtlStatus->GetValidManualCmd(tValidManualCmd);

	for (i = 0;i < MAX_PANEL_KEY_CFG_COUNT;i++)
	{
		if (tAscManualPanel.m_stPanelKeyCfg[i].m_byKeyNum == tValidManualCmd.m_tDirectionCmd.m_nNextDirectionIndex)
		{    
			for (j = 0;j < nChannelCount;j++)
			{
				if (tAscManualPanel.m_stPanelKeyCfg[i].m_ChannelCfg[j].m_byChannelStatus == m_nLockChannelTransStatus[j])
				{
					 nCount += 1;//锁定通道的灯色和方向控制的通道灯色一致，则不过渡
				}
				else
				{
					if (LockChannelTrans(tAscOnePlanChannelLockInfo, j, false, tLampClrStatus))
					{
						nCount += 1;
					}
				}

				if (nCount == nLockChannelCount)
				{
					bLockChannelTrans = true;
				}
			}

            break;
		}
	}
	
	m_pLogicCtlStatus->SetLampClrStatus(tLampClrStatus);

	if (bLockChannelTrans)
	{
		nCount = 0;
		memset(&m_tOldAscOnePlanChannelLockInfo,0,sizeof(m_tOldAscOnePlanChannelLockInfo));

		TChannel atChannelInfo[MAX_CHANNEL_COUNT];
		memset(atChannelInfo,0,sizeof(atChannelInfo));
		TChannel atRealChannelInfo[MAX_CHANNEL_COUNT];
		memset(atRealChannelInfo, 0, sizeof(atRealChannelInfo));
		m_pLogicCtlParam->GetChannelTable(atRealChannelInfo);

		for (i = 0;i < MAX_CHANNEL_COUNT;i++)
		{
			atChannelInfo[i].m_byChannelNumber = i + 1;
		}

		for (i = 0;i < MAX_CHANNEL_COUNT;i++)
		{
			for (j = 0;j < MAX_CHANNEL_COUNT;j++)
			{
				if (atRealChannelInfo[j].m_byChannelNumber == 0)
				{
					continue;
				}

				if (atChannelInfo[i].m_byChannelNumber == atRealChannelInfo[j].m_byChannelNumber)
				{
					memcpy(&atChannelInfo[i], &atRealChannelInfo[j], sizeof(TChannel));
				}
			}

			if (atChannelInfo[i].m_byChannelControlSource == 0)
			{
				atChannelInfo[i].m_byChannelNumber = 0;
			}
		}
	
		//获取当前通道需要切换的目标灯色
		for (i = 0;i < MAX_PANEL_KEY_CFG_COUNT;i++)
		{
			if (tAscManualPanel.m_stPanelKeyCfg[i].m_byKeyNum == tValidManualCmd.m_tDirectionCmd.m_nNextDirectionIndex)
			{    
				for (j = 0;j < nChannelCount;j++)
				{
					if (tAscManualPanel.m_stPanelKeyCfg[i].m_ChannelCfg[j].m_byChannelID == atChannelInfo[j].m_byChannelNumber &&
						tAscManualPanel.m_stPanelKeyCfg[i].m_ChannelCfg[j].m_byChannelStatus != CHANNEL_STATUS_DEFAULT)
					{
						char * pStartPos = NULL;
						pStartPos = tLampClrStatus.m_achLampClr + (tAscManualPanel.m_stPanelKeyCfg[i].m_ChannelCfg[j].m_byChannelID - 1) * 3;

						if (tAscManualPanel.m_stPanelKeyCfg[i].m_ChannelCfg[j].m_byChannelStatus == CHANNEL_STATUS_GREEN)
						{
							*pStartPos = (char)LAMP_CLR_OFF;
							*(pStartPos + 1) = (char)LAMP_CLR_OFF;
							*(pStartPos + 2) = (char)LAMP_CLR_ON; 
						}
						else if (tAscManualPanel.m_stPanelKeyCfg[i].m_ChannelCfg[j].m_byChannelStatus == CHANNEL_STATUS_RED)
						{
							*pStartPos = (char)LAMP_CLR_ON;
							*(pStartPos + 1) = (char)LAMP_CLR_OFF;
							*(pStartPos + 2) = (char)LAMP_CLR_OFF; 
						}
						else if (tAscManualPanel.m_stPanelKeyCfg[i].m_ChannelCfg[j].m_byChannelStatus == CHANNEL_STATUS_OFF)
						{
							*pStartPos = (char)LAMP_CLR_OFF;
							*(pStartPos + 1) = (char)LAMP_CLR_OFF;
							*(pStartPos + 2) = (char)LAMP_CLR_OFF; 
						}
						m_pLogicCtlStatus->SetLampClrStatus(tLampClrStatus);
					}
				}
  
				break;
			} 
		}
	}
}

/*==================================================================== 
函数名 ：SwitchManualControlPatternToSelf
功能 ：手动控制方案超时回到自主
算法实现 ： 
参数说明 ：无
返回值说明：无
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 李永萍          创建函数 
====================================================================*/
void COpenATCLogicCtlManager::SwitchManualControlPatternToSelf()
{
	//超时手动方案控制持续时间，发送回到自主命令
	if (m_nCurCtlMode == CTL_MODE_MANUAL_CONTROL_PATTERN && labs(time(NULL) - m_nManualControlPatternStartTime) > m_nManualControlPatternDurationTime &&
		m_nManualControlPatternDurationTime > 0)
	{
		m_nManualControlPatternStartTime = 0;
		m_nManualControlPatternDurationTime = 0;

		TManualCmd tManualCmd;
		memset(&tManualCmd, 0, sizeof(tManualCmd));

		tManualCmd.m_bNewCmd = true;
		tManualCmd.m_nCmdSource = CTL_SOURCE_SYSTEM;
		memcpy(tManualCmd.m_szPeerIp, "127.0.0.1", strlen( "127.0.0.1"));
		tManualCmd.m_bStepForwardCmd = false;
		memset(&tManualCmd.m_tStepForwardCmd,0,sizeof(TStepForwardCmd));
		tManualCmd.m_bDirectionCmd = false;
		memset(&tManualCmd.m_tDirectionCmd,0,sizeof(TDirectionCmd));
		tManualCmd.m_bPatternInterruptCmd = true;
		tManualCmd.m_tPatternInterruptCmd.m_nControlMode = CTL_MODE_SELFCTL;
		tManualCmd.m_tPatternInterruptCmd.m_nPatternNo = 0;
		tManualCmd.m_bChannelLockCmd = false;
		memset(&tManualCmd.m_tChannelLockCmd,0,sizeof(TChannelLockCmd));
		memset(&tManualCmd.m_tPhaseLockPara,0,sizeof(TPhaseLockPara));
		tManualCmd.m_bPhaseToChannelLock = false;
		m_pLogicCtlStatus->SetManualCmd(tManualCmd);

		m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "CreateManualCmd, ManualControlPattern Timeout Return To Self");
	}
}

/*==================================================================== 
函数名 ：SetPreemptStageIndex
功能 ：设置优先控制的阶段编号
算法实现 ： 
参数说明 ： byPreemptPhaseID,优先控制相位
返回值说明：byStageIndexTarget，优先控制相位对应的阶段号
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 李永萍          创建函数 
====================================================================*/
void COpenATCLogicCtlManager::SetPreemptStageIndex(BYTE byPreemptPhaseID, BYTE & byStageIndexTarget)
{
	bool bFlag = false;
	int nRunStageIndex = 0;
	int nPhaseIndex = 0;
	for (nRunStageIndex = 0;nRunStageIndex < m_tRunStageTable.m_nRunStageCount;nRunStageIndex++)
	{
		for (nPhaseIndex = 0;nPhaseIndex < m_tRunStageTable.m_PhaseRunstageInfo[nRunStageIndex].m_nConcurrencyPhaseCount;nPhaseIndex++)
		{
			if (m_tRunStageTable.m_PhaseRunstageInfo[nRunStageIndex].m_nConcurrencyPhase[nPhaseIndex] == byPreemptPhaseID)
			{
				bFlag = true;
				break;
			}
		}

		if (bFlag)
		{
			break;
		}
	}

	byStageIndexTarget = nRunStageIndex;

	if (!m_tRunStageTable.m_PhaseRunstageInfo[nRunStageIndex].m_bDirectTransit[nPhaseIndex])
	{
		TPhaseRunStatus tAscPhaseRunStatus;
		memset(&tAscPhaseRunStatus, 0x00, sizeof(tAscPhaseRunStatus));
		m_pLogicCtlStatus->GetPhaseRunStatus(tAscPhaseRunStatus);

		int  i = 0;
		for (i = 0;i < tAscPhaseRunStatus.m_atRingRunStatus[nPhaseIndex].m_nPhaseCount; i++)
		{
			if (tAscPhaseRunStatus.m_atRingRunStatus[nPhaseIndex].m_nCurRunPhaseIndex == i &&
				tAscPhaseRunStatus.m_atRingRunStatus[nPhaseIndex].m_atPhaseStatus[i].m_byPhaseID == byPreemptPhaseID)
			{
				if (tAscPhaseRunStatus.m_atRingRunStatus[nPhaseIndex].m_atPhaseStatus[i].m_chPhaseStatus == C_CH_PHASESTAGE_G)
				{
					byStageIndexTarget = tAscPhaseRunStatus.m_atRingRunStatus[nPhaseIndex].m_nCurStageIndex;
				}
			}
		}
	}

	m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "SetPreemptStageIndex:%d", byStageIndexTarget);
}

/*==================================================================== 
函数名 ：CreatePatternInterruptCmdInPreemptControl
功能 ：在优化控制时生成方案干预指令
算法实现 ： 
参数说明 ： tManualCmd，控制指令
返回值说明：无
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 李永萍          创建函数 
====================================================================*/
void COpenATCLogicCtlManager::CreatePatternInterruptCmdInPreemptControl(TManualCmd  tManualCmd)
{
	TPreemptCtlCmd  tPreemptCtlCmd;
	memset(&tPreemptCtlCmd,0,sizeof(tPreemptCtlCmd));
	tPreemptCtlCmd.m_nCmdSource = CTL_SOURCE_SYSTEM;
	tPreemptCtlCmd.m_nCurCtlSource = CTL_SOURCE_PREEMPT;
	memcpy(tPreemptCtlCmd.m_szPeerIp,tManualCmd.m_szPeerIp,strlen(tManualCmd.m_szPeerIp));
	tPreemptCtlCmd.m_bNewCmd = true;
	tPreemptCtlCmd.m_byPreemptType = PREEMPT_TYPE_DEFAULT;
	tPreemptCtlCmd.m_bPatternInterruptCmd = true;
	tPreemptCtlCmd.m_tPatternInterruptCmd.m_nControlMode = tManualCmd.m_tPatternInterruptCmd.m_nControlMode;
	tPreemptCtlCmd.m_tPatternInterruptCmd.m_nPatternNo = tManualCmd.m_tPatternInterruptCmd.m_nPatternNo;
	m_pLogicCtlStatus->SetPreemptCtlCmd(tPreemptCtlCmd);

	m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "CreatePatternInterruptCmdInPreemptControl, ControlMode:%d  PatternNo:%d", tPreemptCtlCmd.m_tPatternInterruptCmd.m_nControlMode, tPreemptCtlCmd.m_tPatternInterruptCmd.m_nPatternNo);
}

#ifdef VIRTUAL_DEVICE
/*====================================================================
函数名 ：GetVirtualTimeByGlobalCount
功能 ：倍速运行时根据全局计数推算出当前的时间
算法实现 ：
参数说明 ：当前时间
返回值说明：无
----------------------------------------------------------------------
修改记录 ：
日 期          版本 修改人  走读人  修改记录
2022/15/18
====================================================================*/
void COpenATCLogicCtlManager::GetVirtualTimeByGlobalCount(int& nYear, int& nMon, int& nDay, int& nHour, int& nMin, int& nSec, int& nWeek)
{
	unsigned long nCounterDiff = 0;
	int diffTime = 0;
	int tSpeedyRunInfo = 0;
	//int TempSec = 0;
	//int TempMin = 0;
	//int TempHour = 0;
	//int TempDay = 0;
	int monthDays[13][2] = { {0,0},{31,31},{28,29},{31,31},{30,30},{31,31},{30,30},{31,31},{31,31},{30,30},{31,31},{30,30},{31,31} };
	unsigned long nGlobalCount;
	TVirtualRunTime mVirtualTimeData;
	memset(&mVirtualTimeData, 0, sizeof(mVirtualTimeData));
	tSpeedyRunInfo = m_pLogicCtlParam->GetSpeedyRunInfo();
	m_pLogicCtlStatus->GetVirtualTimeData(mVirtualTimeData);
	nGlobalCount = m_pLogicCtlStatus->GetGlobalCounter();
	switch (tSpeedyRunInfo)
	{
	case 0:
		nCounterDiff = CalcCounter(mVirtualTimeData.TempGlobalCount, nGlobalCount, C_N_MAXGLOBALCOUNTER);
		break;
	case 1:
		nCounterDiff = CalcCounter(mVirtualTimeData.TempGlobalCount, nGlobalCount, C_N_MAXGLOBALCOUNTER);
		break;
	case 2:
		nCounterDiff = CalcCounter(mVirtualTimeData.TempGlobalCount, nGlobalCount, C_N_MAXGLOBALCOUNTER);
		break;
	case 3:
		nCounterDiff = CalcCounter(mVirtualTimeData.TempGlobalCount, nGlobalCount, C_N_MAXGLOBALCOUNTER);
		break;
	default:
		nCounterDiff = CalcCounter(mVirtualTimeData.TempGlobalCount, nGlobalCount, C_N_MAXGLOBALCOUNTER);
		break;
	}
	if (nCounterDiff > 20)
	{
		diffTime = nCounterDiff / 20;
		mVirtualTimeData.VirtualSec = mVirtualTimeData.VirtualSec + diffTime;
		if (mVirtualTimeData.VirtualSec > 59)
		{
			mVirtualTimeData.VirtualMin = mVirtualTimeData.VirtualSec / 60 + mVirtualTimeData.VirtualMin;
			mVirtualTimeData.VirtualSec = mVirtualTimeData.VirtualSec % 60;
		}
		if (mVirtualTimeData.VirtualMin > 59)
		{
			mVirtualTimeData.VirtualHour = mVirtualTimeData.VirtualMin / 60 + mVirtualTimeData.VirtualHour;
			mVirtualTimeData.VirtualMin = mVirtualTimeData.VirtualMin % 60;
		}
		if (mVirtualTimeData.VirtualHour > 23)
		{
			mVirtualTimeData.VirtualDay = mVirtualTimeData.VirtualHour / 24 + mVirtualTimeData.VirtualDay;
			mVirtualTimeData.VirtualHour = mVirtualTimeData.VirtualHour % 24;
			mVirtualTimeData.VirtualWeek++;
			if (mVirtualTimeData.VirtualWeek == 7)
			{
				mVirtualTimeData.VirtualWeek = 0;
			}
		}
		if (mVirtualTimeData.VirtualDay == monthDays[mVirtualTimeData.VirtualMon][m_pLogicCtlParam->isLeap(mVirtualTimeData.VirtualYear)] + 1)
		{
			mVirtualTimeData.VirtualDay = 1;
			mVirtualTimeData.VirtualMon++;
		}
		if (mVirtualTimeData.VirtualMon == 13)
		{
			mVirtualTimeData.VirtualYear++;
			mVirtualTimeData.VirtualMon = 1;
		}
		mVirtualTimeData.TempGlobalCount = nGlobalCount;
		m_pLogicCtlStatus->SetVirtualTimeData(mVirtualTimeData);
	}
	nYear = mVirtualTimeData.VirtualYear;
	nMon = mVirtualTimeData.VirtualMon;
	nDay = mVirtualTimeData.VirtualDay;
	nHour = mVirtualTimeData.VirtualHour;
	nMin = mVirtualTimeData.VirtualMin;
	nSec = mVirtualTimeData.VirtualSec;
	nWeek = mVirtualTimeData.VirtualWeek;
}
#endif // VIRTUAL_DEVICE
//Virtual_Test2022