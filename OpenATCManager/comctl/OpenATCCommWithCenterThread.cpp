/*====================================================================
模块名 ：和客户端配置软件交互的通信接口
文件名 ：OpenATCCommWithCenterThread.cpp
相关文件：OpenATCCommWithCenterThread.h
实现功能：和客户端配置进行交互
作者 ：李永萍
版权 ：<Copyright(C) 2019-2020 Suzhou Keda Technology Co., Ltd. All rights reserved.>
---------------------------------------------------------------------------------------------------------------------
修改记录：
日 期           版本    修改人             走读人             修改记录
2019/09/06      V1.0    李永萍             李永萍             创建模块
====================================================================*/
#include "OpenATCCommWithCenterThread.h"
#include "OpenATCCenterCommHandlerSimpleFactory.h"
#include "../../Include/OpenATCLog.h"

COpenATCCommWithCenterThread::COpenATCCommWithCenterThread()
{
	m_hThread		= 0;
    m_dwThreadRet	= 0;
	m_bExitFlag		= true;
    m_nDetachState  = 0;

	m_pOpenATCParameter = NULL;
	m_pOpenATCRunStatus = NULL;
	m_pOpenATCLog       = NULL;
    m_centerHandler		= NULL;
	m_nComType = COM_WITH_CENTER;
}

COpenATCCommWithCenterThread::~COpenATCCommWithCenterThread()
{
    if (!m_bExitFlag && (m_nDetachState == 0))
    {
        Join();
    }

    if (m_centerHandler)
    {
        delete m_centerHandler;
        m_centerHandler = NULL;
    }
}

int COpenATCCommWithCenterThread::Run()
{
#ifndef _WIN32
	prctl(15,"CommWithCenterThread",0,0,0);
#endif
	int				nRecvLength = 0;
    unsigned int	nPackLength = 0;
    int				nRet		= 0;
	bool			bFlag		= false;

	m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "COpenATCCommWithCenterThread start!");

	if (m_centerHandler == NULL)
    {
        m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "COpenATCCommWithCenterThread's implementation is not initialized!");

        return OPENATC_RTN_FAILED;
    }

	while (!m_bExitFlag)
    {
        if (m_centerHandler->ConnectToCenter() != OPENATC_RTN_OK)
        {
            OpenATCSleep(500);
            continue;
        }

        while (true)
        {
			m_centerHandler->HandleEventFromCenter();

            if (m_nComType == COM_WITH_CENTER)
            {
                m_centerHandler->TimerJob(true);
            }
            else
            {
			    m_centerHandler->SendEventToCenter();
            }

			if (m_nComType == COM_WITH_CENTER && m_centerHandler->IsCenterParamChg() == OPENATC_RTN_OK)
			{
				break;
			}

			OpenATCSleep(100);
        }
		
		m_centerHandler->DisconnectToCenter();
    }

	m_pOpenATCLog->LogOneMessage(LEVEL_INFO, "COpenATCCommWithCenterThread exit!");
    return OPENATC_RTN_OK;
}

void  COpenATCCommWithCenterThread::Init(COpenATCParameter *pParameter, COpenATCRunStatus *pRunStatus, COpenATCLog * pOpenATCLog, int nComType, const char * pOpenATCVersion)
{
	m_pOpenATCParameter = pParameter;
	m_pOpenATCRunStatus = pRunStatus;
	m_pOpenATCLog       = pOpenATCLog;
	m_nComType          = nComType;

	memset(m_chOpenATCVersion, 0, sizeof(m_chOpenATCVersion));
	memcpy(m_chOpenATCVersion, pOpenATCVersion, strlen(pOpenATCVersion));

    m_centerHandler = COpenATCCenterCommHandlerSimpleFactory::Create(m_pOpenATCParameter, m_pOpenATCRunStatus, m_pOpenATCLog, pOpenATCVersion);
    m_centerHandler->SetComPara(nComType);
}

int COpenATCCommWithCenterThread::Start()
{
    if (false == m_bExitFlag)
	{
        return OPENATC_RTN_OK;
	}

    bool bOK = true;
    m_bExitFlag = false;

#ifdef _WIN32
	m_hThread = CreateThread(NULL, 0, (unsigned long(COMMWITHCENTER_CALLBACK *)(void *))&RunThread, this, 0, NULL);
    if (NULL == m_hThread)
    {
        bOK = false;
    }
#else
    if (0 != pthread_create(&m_hThread, NULL, RunThread, this))
    {
        bOK = false;
    }
#endif

    if (bOK == false)
    {
        m_bExitFlag = true;
        return OPENATC_RTN_FAILED;
    }

    return OPENATC_RTN_OK;
}

int COpenATCCommWithCenterThread::Join()
{
    if (m_bExitFlag == true)
	{
        return OPENATC_RTN_FAILED;
	}

    if (m_nDetachState == 1)
	{
        return OPENATC_RTN_FAILED;
	}

    int nRet = OPENATC_RTN_FAILED;

#ifdef _WIN32
    while (1)
    {
        if (0 == GetExitCodeThread(m_hThread, &m_dwThreadRet))
        {
            nRet = OPENATC_RTN_FAILED;
        }
        else
        {
            if (m_dwThreadRet == STILL_ACTIVE)
            {
                OpenATCSleep(100);
            }
            CloseHandle(m_hThread);
            nRet = OPENATC_RTN_OK;
            break;
        }
    }

#else
	if (0 == pthread_join(m_hThread, (void **)&m_dwThreadRet))
	{
		nRet = OPENATC_RTN_OK;
	}
	else
	{
		nRet = OPENATC_RTN_FAILED;
	}
#endif

	if (nRet == OPENATC_RTN_OK)
	{
		m_pOpenATCLog->LogOneMessage(LEVEL_INFO, "COpenATCCommWithCenterThread Join ok!");
	}
	else
	{
		m_pOpenATCLog->LogOneMessage(LEVEL_INFO, "COpenATCCommWithCenterThread Join fail!");
	}

	m_bExitFlag = true;
    m_hThread	= 0;
    return nRet;
}

int COpenATCCommWithCenterThread::Detach()
{
    if (m_bExitFlag == true)
	{
        return OPENATC_RTN_FAILED;
	}

    if (m_nDetachState == 1)
	{
        return OPENATC_RTN_FAILED;
	}

    m_nDetachState = 1;

#ifdef _WIN32
	if (1 == CloseHandle(m_hThread))
	{
		return OPENATC_RTN_OK;
	}
	else
	{
		return OPENATC_RTN_FAILED;
	}
#else
	if (0 == pthread_detach(m_hThread))
	{
		return OPENATC_RTN_OK;
	}
	else
	{
		return OPENATC_RTN_FAILED;
	}
#endif
}

void* COMMWITHCENTER_CALLBACK COpenATCCommWithCenterThread::RunThread(void *pParam)
{
    COpenATCCommWithCenterThread *pThis = (COpenATCCommWithCenterThread *)pParam;

    int nRet = pThis->Run();
    return (void *)nRet;
}

void COpenATCCommWithCenterThread::OpenATCSleep(long nMsec)
{
#ifdef _WIN32
    Sleep(nMsec);
#else
    usleep(nMsec*1000);
#endif
}