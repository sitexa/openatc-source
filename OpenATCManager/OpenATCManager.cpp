#include "../Include/OpenATCParameter.h"
#include "../Include/OpenATCRunStatus.h"
#include "../Include/OpenATCLog.h"
#include "../Include/OpenATCMainCtlManager.h"
#include "../Include/CanBusManager.h"
#include "../Include/OpenATCFaultProcManager.h"
#include "../Include/OpenATCHardwareManager.h"
#include "OpenATCLogicCtlManager.h"
#include "OpenATCFlowProcManager.h"
#include "OpenATCComCtlManager.h"

#ifdef _WIN32
#include <windows.h>
#include <process.h>
#include "mmSystem.h"
#else
#include <unistd.h>
#include <sys/reboot.h>
#include <sys/time.h>
#include <stdlib.h>
#endif

#include "i18n_utils.h"

#if (defined VIRTUAL_DEVICE) && (defined _WIN32)
#include <conio.h>
#include "../Include/getopt.h"
HWND hWnd = GetConsoleWindow();
#endif

void OpenATCSleep(long nMsec)
{
#ifdef _WIN32
    Sleep(nMsec);
#else
    usleep(nMsec*1000);
#endif
}

void RebootSystem()
{
#ifdef _WIN32
    exit(0);
#else
    system("hwclock -w");
    if (system("reboot") != 0)
    {
        reboot(RB_AUTOBOOT);
    }
#endif
}

#include "version.h.in"

static const char * GetOpenATCMainCtlVersion()
{
    return "OpenATCMainCtlManager_" OPENATC_VERSION;
}   

static void AddOneFaultMessage(char cBoardType, char cFaultLevel, DWORD wFaultType, DWORD wFaultSubType, char cFaultInfo1, char cFaultInfo2)
{
    COpenATCFaultProcManager::getInstance()->AddOneFaultMessage(cBoardType, cFaultLevel, wFaultType, wFaultSubType, cFaultInfo1, cFaultInfo2);
}

static void EraseOneFaultMessage(char cBoardType, char cFaultLevel, DWORD wFaultType, DWORD wFaultSubType, char cFaultInfo1, char cFaultInfo2)
{
    COpenATCFaultProcManager::getInstance()->EraseOneFaultMessage(cBoardType, cFaultLevel, wFaultType, wFaultSubType, cFaultInfo1, cFaultInfo2);
}


int main(int argc, char* argv[])
{
#if (defined VIRTUAL_DEVICE) && (defined _WIN32)
    int opt = 0;
    int wFlag = 0;
    int bResult = 0;
    while ((opt = getopt(argc, argv, "w")) != -1)
    {
        switch (opt)
        {
        case 'w':wFlag = 1; break;
        }
    }
    if (wFlag)
    {
        ShowWindow(hWnd, SW_HIDE);
    }
#endif
#ifdef _WIN32
    DWORD dwError, dwPriClass;
    if(!SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS))
    {
        dwError = GetLastError();
        printf("Failed to enter pri mode (%d)\n", dwError);
    }
    // Display priority class
    dwPriClass = GetPriorityClass(GetCurrentProcess());
    printf("Current priority class is 0x%x\n", dwPriClass);
#endif
    COpenATCLog   * m_pOpenATCLog = new COpenATCLog;
    m_pOpenATCLog->SetConfig(LOGCFG_NAME);
#ifdef VIRTUAL_DEVICE
    m_pOpenATCLog->SetLogFileName("./log", "OpenATCRun");
#else
    m_pOpenATCLog->SetLogFileName("/usr/log", "OpenATCRun");
#endif

    int nSystemLanguage = m_pOpenATCLog->ReadScreenLanguage();

    //todo: Use System Enviroment
    if (nSystemLanguage == LANGUAGE_ENGLISH)
    {
        putenv("LANGUAGE=en");
    }
    else
    {
        putenv("LANGUAGE=zh");
    }

	if (!bindtextdomain("openatc", "i18n"))
	{
		m_pOpenATCLog->LogOneMessage(LEVEL_ERROR, "Warning: Could not bind text domain");
	}
	if (!textdomain("openatc"))
	{
		m_pOpenATCLog->LogOneMessage(LEVEL_ERROR, "Warning: Could not set text domain");
	}
	
	string stOpenATCMainCtlVersion = GetOpenATCMainCtlVersion();
	if (stOpenATCMainCtlVersion.find_first_of("-") > 0)
	{
		stOpenATCMainCtlVersion = stOpenATCMainCtlVersion.substr(0, stOpenATCMainCtlVersion.find_first_of("-"));
	}
	else
	{
		stOpenATCMainCtlVersion = stOpenATCMainCtlVersion.substr(0, stOpenATCMainCtlVersion.length());
	}

    m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "OpenATC MainCTL Software Start,Version is %s.nSystemLanguage is %d.", stOpenATCMainCtlVersion.c_str(), nSystemLanguage);

    COpenATCRunStatus * pRunStatus = new COpenATCRunStatus();
    pRunStatus->Init();

    COpenATCHardwareManager::getInstance()->HardwareInit();
    TSelfDetectInfo selfDetectInfo;
    selfDetectInfo.m_cSelfDetectStatus = SELF_DETECT_INING;
    selfDetectInfo.m_cSelfDetectFailedReason = NO_WRONG_INFO;
    COpenATCHardwareManager::getInstance()->ShowSelfDetectFaultInfoInLedScreen(pRunStatus, selfDetectInfo, 25, 25, TEXT_SIZE_2, nSystemLanguage);

    int nSiteID = COpenATCHardwareManager::getInstance()->ReadSiteID();
    COpenATCParameter * pParameter = new COpenATCParameter();
    pParameter->SetSiteID(nSiteID);
    pParameter->Init(pRunStatus, m_pOpenATCLog);
    m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL,"Parameter Init SITEID is %d!", nSiteID);

    char ipAddr0[32] = {0};
    char ipAddr1[32] = {0};
    COpenATCHardwareManager::getInstance()->NetConfig(pParameter, ipAddr0, ipAddr1);
    char szInfoLine[255] = {0};
    sprintf(szInfoLine,"IP: %s, %s", ipAddr0, ipAddr1);
    COpenATCHardwareManager::getInstance()->ShowSelfDetectInfoInLedScreen(szInfoLine, 25, 25, TEXT_SIZE_2);


    bool bParameterReady = true;
    TParamRunStatus tOldParamStatus;
    TParamRunStatus tParamStatus;
    pRunStatus->GetParamRunStatus(tParamStatus);
    if (tParamStatus.m_chParameterReady != C_CH_PARAMERREADY_OK)
    {
        m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL,"Parameter Init Failed!");
        bParameterReady = false;

        memcpy(&tOldParamStatus,&tParamStatus,sizeof(tParamStatus));
        COpenATCFaultProcManager::getInstance()->SaveParamInitFault(tParamStatus, pRunStatus, m_pOpenATCLog);
    }
    pRunStatus->GetSelfDetectInfo(selfDetectInfo);
    if (selfDetectInfo.m_cSelfDetectStatus == SELF_DETECT_FAILED)
    {
		string pasteInfo = stOpenATCMainCtlVersion;
	    pasteInfo = pasteInfo.substr(pasteInfo.find_first_of("_")+1, pasteInfo.length() - 22);
		snprintf(szInfoLine, 255, _("Software Version:%s"), pasteInfo.c_str());
	    COpenATCHardwareManager::getInstance()->ShowSelfDetectInfoInLedScreen(szInfoLine, 25, 25, TEXT_SIZE_2);
	
		char cSelfDetectFailedReason = selfDetectInfo.m_cSelfDetectFailedReason;
        COpenATCHardwareManager::getInstance()->ShowSelfDetectFaultInfoInLedScreen(pRunStatus, selfDetectInfo, 25, 25, TEXT_SIZE_2, nSystemLanguage);
		if (cSelfDetectFailedReason == SITEID_CHECK_FAILED)
		{
			char szSiteIDFromATC[255] = {0};
			char szSiteIDFromParam[255] = {0};
			pParameter->GetSiteID(szSiteIDFromATC, szSiteIDFromParam);
			snprintf(szInfoLine,255, _("SiteID From Param: %s"), szSiteIDFromParam);
			COpenATCHardwareManager::getInstance()->ShowSelfDetectInfoInLedScreen(szInfoLine, 25, 25, TEXT_SIZE_2);
			snprintf(szInfoLine,255, _("Real SiteID: %s"), szSiteIDFromATC);
			COpenATCHardwareManager::getInstance()->ShowSelfDetectInfoInLedScreen(szInfoLine, 25, 25, TEXT_SIZE_2);
		}
    }
    else
    {
        COpenATCHardwareManager::getInstance()->ShowSelfDetectInfoInLedScreen(_("OpenATC Begin Init Parameter."), 25, 25, TEXT_SIZE_2);
        //COpenATCHardwareManager::getInstance()->ShowSelfDetectInfoInLedScreen("信号机参数初始化", 25, 25, TEXT_SIZE_2);
    }

    //或取通讯协议选择。
    int Comm = 0;
    Comm = pParameter->GetCommThreadInfo();

    if ((Comm > 3) || (Comm <= 0))
    {
        //配置的值错误，默认25280通讯协议启动
        m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "CommThread config failed, and default start COpenATCCommWithCfgSWThread");
        Comm = 1;
    }
    else
    {
        m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "CommThread config success, and CommThread choose %d!", Comm);
    }

    pRunStatus->SetCommFlagStatus(Comm);

    //无论参数是否准备就绪，主控模块必须先初始化
    COpenATCMainCtlManager::getInstance()->Init(pParameter,pRunStatus, m_pOpenATCLog);    
    COpenATCComCtlManager::getInstance()->Init(pParameter,pRunStatus,m_pOpenATCLog, (char *)stOpenATCMainCtlVersion.c_str());

    TSystemControlStatus tSystemControlStatus;
    memset(&tSystemControlStatus, 0, sizeof(tSystemControlStatus));
    pRunStatus->GetSystemControlStatus(tSystemControlStatus);
    tSystemControlStatus.m_nSpecicalControlResult = CONTROL_FAILED;
    tSystemControlStatus.m_nSpecicalControlFailCode = DEVICE_INIT_NO_EXECUT;
    tSystemControlStatus.m_nPatternControlResult = CONTROL_FAILED;
    tSystemControlStatus.m_nPatternControlFailCode = DEVICE_INIT_NO_EXECUT;
    tSystemControlStatus.m_nStageControlResult = CONTROL_FAILED;
    tSystemControlStatus.m_nStageControlFailCode = DEVICE_INIT_NO_EXECUT;
    tSystemControlStatus.m_nPhaseControlResult = CONTROL_FAILED;
    tSystemControlStatus.m_nPhaseControlFailCode = DEVICE_INIT_NO_EXECUT;
    tSystemControlStatus.m_nChannelLockResult = CONTROL_FAILED;
    tSystemControlStatus.m_nChannelLockFailCode = DEVICE_INIT_NO_EXECUT;
    pRunStatus->SetSystemControlStatus(tSystemControlStatus);

    // 初始化回调函数
    TFaultProcCallBacks* pfaultCallBacks = new TFaultProcCallBacks(); //TODO: free
    pfaultCallBacks->pfnAddOneFaultMessage = &AddOneFaultMessage;
    pfaultCallBacks->pfnEraseOneFaultMessage = &EraseOneFaultMessage;

    //特征参数必须要准备就绪
    m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL,"Wait For OpenATC Parameter Ready.");
    while (true)
    {
        if (pRunStatus->GetRebootStatus())
        {
            m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL,"OpenATC Need to Reboot.");
            goto lbl_exit;
        }

        if (pRunStatus->GetStopWorkStatus())
        {
            m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL,"OpenATC Need to Stop Work.");
            goto lbl_exit;
        }

        pRunStatus->GetParamRunStatus(tParamStatus);
        if (tParamStatus.m_chParameterReady == C_CH_PARAMERREADY_OK)
        {
            pParameter->SetSystemTimeZone();

            if (!bParameterReady)
            {
                COpenATCFaultProcManager::getInstance()->ClearParamInitFault(tOldParamStatus, pRunStatus, m_pOpenATCLog);
            }

            m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL,"OpenATC Parameter Init Success.");
            /// "信号机参数初始化成功，开始功能模块初始化"
            COpenATCHardwareManager::getInstance()->ShowSelfDetectInfoInLedScreen(_("OpenATC Parameter Init Success.OpenATC Begin Init ModuleManager."), 25, 25, TEXT_SIZE_2);
            break;
        }

        COpenATCMainCtlManager::getInstance()->Work();

        pRunStatus->GetSelfDetectInfo(selfDetectInfo);
        if (selfDetectInfo.m_cSelfDetectStatus == SELF_DETECT_FAILED)
        {
            COpenATCHardwareManager::getInstance()->ShowSelfDetectFaultInfoInLedScreen(pRunStatus, selfDetectInfo, 25, 25, TEXT_SIZE_2, nSystemLanguage);
        }

        OpenATCSleep(500);        
    }
    pRunStatus->GetSelfDetectInfo(selfDetectInfo);
    if (selfDetectInfo.m_cSelfDetectStatus == SELF_DETECT_FAILED)
    {
        COpenATCHardwareManager::getInstance()->ShowSelfDetectFaultInfoInLedScreen(pRunStatus, selfDetectInfo, 25, 25, TEXT_SIZE_2, nSystemLanguage);
    }

    //功能模块初始化
    m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL,"OpenATC Begin Init ModuleManager.");
#ifndef VIRTUAL_DEVICE
    COpenATCHardwareManager::getInstance()->Init(pParameter,pRunStatus,m_pOpenATCLog, (char *)stOpenATCMainCtlVersion.c_str(), m_pOpenATCLog->ReadScreenLanguage());
    CCanBusManager::getInstance()->Init(pParameter,pRunStatus,m_pOpenATCLog, pfaultCallBacks);
#endif
    COpenATCLogicCtlManager::getInstance()->Init(pParameter,pRunStatus,m_pOpenATCLog);
    COpenATCFaultProcManager::getInstance()->Init(pParameter,pRunStatus,m_pOpenATCLog);
    COpenATCFlowProcManager::getInstance()->Init(pParameter,pRunStatus,m_pOpenATCLog);
    COpenATCHardwareManager::getInstance()->ShowSelfDetectInfoInLedScreen(_("OpenATC ModuleManager Init ModuleManager Success.OpenATC Begin Self Detect."), 25, 25, TEXT_SIZE_2);
	//"信号机功能模块初始化成功，开始自检"
    

    //信号机自检
#ifndef VIRTUAL_DEVICE
    m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "OpenATC Begin Self Detect.");
    while (true)
    {
        if (pRunStatus->GetRebootStatus())
        {
            m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL,"OpenATC Need to Reboot.");
            goto lbl_exit;
        }

        if (pRunStatus->GetStopWorkStatus())
        {
            m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL,"OpenATC Need to Stop Work.");
            goto lbl_exit;
        }

        if (pRunStatus->GetSelfDetectStatus())
        {
            m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL,"OpenATC Self Detect Success.");
            COpenATCHardwareManager::getInstance()->ShowSelfDetectInfoInLedScreen(_("OpenATC Self Detect Success.OpenATC Begin StartHardwareResource."), 25, 25, TEXT_SIZE_2);
			//"信号机自检成功，开始硬件资源使能"
            break;
        }

        CCanBusManager::getInstance()->SelfDetect();
        COpenATCFaultProcManager::getInstance()->SelfDetect();
        COpenATCMainCtlManager::getInstance()->Work();

        pRunStatus->GetSelfDetectInfo(selfDetectInfo);
        if (selfDetectInfo.m_cSelfDetectStatus == SELF_DETECT_FAILED)
        {
            COpenATCHardwareManager::getInstance()->ShowSelfDetectFaultInfoInLedScreen(pRunStatus, selfDetectInfo, 25, 25, TEXT_SIZE_2, nSystemLanguage);
        }
        OpenATCSleep(500);        
    }
    
    //硬件资源使能
    m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL,"OpenATC Begin StartHardwareResource.");
    while (true)
    {
        CCanBusManager::getInstance()->PullInRelay(0);
        OpenATCSleep(500);
        if (COpenATCHardwareManager::getInstance()->CheckRelayStatus())
        {
            m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL,"StartHardwareResource Success.");

            //等待故障板停止工作
            bool bStop = false;
            m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL,"Wait FaultDetBoard Complete.");
            COpenATCHardwareManager::getInstance()->ShowSelfDetectInfoInLedScreen(_("StartHardwareResource Success.Wait FaultDetBoard Complete."), 25, 25, TEXT_SIZE_2);
			//"信号机硬件资源使能成功，等待故障检测板停止"
            while (true)
            {
                if (CCanBusManager::getInstance()->ReadFaultDetBoardCardStatus())
                {
                    CCanBusManager::getInstance()->ClearFaultDetBoardFaultStudyStatus();
                    CCanBusManager::getInstance()->ClearFaultDetBoardFaultStatus();
                    CCanBusManager::getInstance()->SendGreenConflicitTableToFaultDetBoard();
                    CCanBusManager::getInstance()->WriteCriticalFaultParamToFaultDetBoard();
                    bStop = true;
                    break;
                }
                else
                {
                    pRunStatus->SetFaultDetBoardControlStatus(true);
                }
                COpenATCMainCtlManager::getInstance()->Work();
                OpenATCSleep(200);  
            }

            if (bStop)
            {
                break;
            }
        }
        else
        {
            pRunStatus->GetSelfDetectInfo(selfDetectInfo);
            COpenATCHardwareManager::getInstance()->ShowSelfDetectFaultInfoInLedScreen(pRunStatus, selfDetectInfo, 25, 25, TEXT_SIZE_2, nSystemLanguage);
        }
        COpenATCMainCtlManager::getInstance()->Work();
    }
#endif

    //控制流程开始
    m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL,"OpenATC Begin Work.");
    COpenATCHardwareManager::getInstance()->ShowSelfDetectInfoInLedScreen(_("OpenATC Begin Work."), 25, 25, TEXT_SIZE_2);
	//"信号机开始工作"

#ifndef VIRTUAL_DEVICE
    COpenATCHardwareManager::getInstance()->ProcCtlLcdScreen();
#endif

    while (true)
    {
        if (pRunStatus->GetRebootStatus())
        {
            m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL,"OpenATC Need to Reboot.");
            break;
        }

        if (pRunStatus->GetStopWorkStatus())
        {
            m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL,"OpenATC Need to Stop Work.");
            break;
        }

#ifndef VIRTUAL_DEVICE
        COpenATCHardwareManager::getInstance()->Work();
        CCanBusManager::getInstance()->Work();
        COpenATCFaultProcManager::getInstance()->Work();
#endif
        COpenATCLogicCtlManager::getInstance()->Work();
        COpenATCFlowProcManager::getInstance()->Work();
        COpenATCMainCtlManager::getInstance()->Work();

        OpenATCSleep(5);     
    }

    //模块退出，释放资源
lbl_exit:
    m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL,"OpenATC Stop Work.");

#ifndef VIRTUAL_DEVICE
    COpenATCHardwareManager::getInstance()->Stop();
    CCanBusManager::getInstance()->Stop();
    COpenATCFaultProcManager::getInstance()->Stop();
#endif
    COpenATCMainCtlManager::getInstance()->Stop();
    COpenATCFlowProcManager::getInstance()->Stop();
    COpenATCComCtlManager::getInstance()->Stop();
    COpenATCLogicCtlManager::getInstance()->Stop();

    int nRebootFlag = 0;
    if (pRunStatus->GetRebootStatus())
    {
        nRebootFlag = 1;
    }
    //delete pParameter;
    //delete pRunStatus;
#ifdef _WIN32
    if(!SetPriorityClass(GetCurrentProcess(), NORMAL_PRIORITY_CLASS))
    {
        printf("Failed to end background mode (%d)\n", GetLastError());
    }
#endif
    if (nRebootFlag == 1)
    {
        RebootSystem();
    }

    return 1;
}
