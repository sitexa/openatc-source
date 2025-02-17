/*====================================================================
模块名 ：和中心交互的线程模块
文件名 ：OpenATCCommWithCameraListenThread.cpp
相关文件：OpenATCCommWithCameraListenThread.h
实现功能：侦听相机
作者 ：李永萍
版权 ：<Copyright(C) 2019-2020 Suzhou Keda Technology Co., Ltd. All rights reserved.>
---------------------------------------------------------------------------------------------------------------------
修改记录：
日 期           版本    修改人             走读人             修改记录
2019/09/06      V1.0    李永萍             李永萍             创建模块
====================================================================*/
#include "OpenATCCommWithCameraListenThread.h"
#include "OpenATCPackUnpackSimpleFactory.h"

COpenATCCommWithCameraListenThread::COpenATCCommWithCameraListenThread()
{
	m_hThread		= 0;
    m_dwThreadRet	= 0;
	m_bExitFlag		= true;
    m_nDetachState  = 0;

	m_pOpenATCParameter = NULL;
	m_pOpenATCRunStatus = NULL;
	m_pOpenATCLog       = NULL;

	m_acceptSock.SetSocketType(SOCK_STREAM);
    m_acceptSock.SetComType(TCP_SERVICE);

	for (int i = 0;i < MAX_CAMERA_SIZE;i++)
	{
		m_pOpenATCCommWithCameraThread[i] = NULL;
	}

	m_pOpenATCCommWithCfgThread = NULL;

	memset(m_szClientIp, 0, sizeof(m_szClientIp));
	m_nClientPort = 0;
}

COpenATCCommWithCameraListenThread::~COpenATCCommWithCameraListenThread()
{
    if (!m_bExitFlag && (m_nDetachState == 0))
    {
        Join();
    }

	m_acceptSock.Close();

	#ifdef _WIN32
	WSACleanup();
	#endif

	if (m_byServiceSource == CAMERA_SERVICE_LISTERN) 
	{
		for (int i = 0;i < MAX_CAMERA_SIZE;i++)
		{
			if (m_pOpenATCCommWithCameraThread[i])
			{
				m_pOpenATCCommWithCameraThread[i]->Join();
				delete m_pOpenATCCommWithCameraThread[i];
				m_pOpenATCCommWithCameraThread[i] = NULL;
			}
		}
	}
	else 
	{
		if (m_pOpenATCCommWithCfgThread)
		{
			m_pOpenATCCommWithCfgThread->Join();
			delete m_pOpenATCCommWithCfgThread;
			m_pOpenATCCommWithCfgThread = NULL;
		}
	}
}

int COpenATCCommWithCameraListenThread::Run()
{
#ifndef _WIN32
	prctl(15,"COpenATCCommWithCameraListenThread",0,0,0);
#endif
	int				nRecvLength  = 0;
    unsigned int	nPackLength  = 0;
	unsigned int    nMessageType = 0;
    int				nRet		 = 0;
	bool			bFlag		 = false;

	m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "COpenATCCommWithCameraListenThread start!");

	int  i = 0;

	char szCameraIp[20] = {0};
	int  nCameraPort = 0;

	SOCKET clentSock;

	int nBindPort = SERVER_COM_PORT;
	if (m_byServiceSource == CFG_SERVICE_LISTERN)//cfg 
	{
		nBindPort = 8880;
	}

	while (!m_bExitFlag)
    {
		if (m_acceptSock.Bind("", nBindPort, false) == OPENATC_RTN_FAILED)
		{
			OpenATCSleep(500);
			continue;
		}
		else
		{
			m_acceptSock.Listen(100);
		}

		while (true)
		{
			if (m_acceptSock.Accept(clentSock) == OPENATC_RTN_FAILED)
			{
				OpenATCSleep(100);
				continue;
			}
			else
			{
				m_acceptSock.GetDestAddr(szCameraIp, &nCameraPort);

				if (m_byServiceSource == CAMERA_SERVICE_LISTERN)//camera 
				{
					m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "Camera:%s Port:%d is connecting!", szCameraIp, nCameraPort);

					bFlag = false;
					for (i = 0;i < MAX_CAMERA_SIZE;i++)
					{
						if (m_pOpenATCCommWithCameraThread[i] != NULL)
						{
							if (memcmp(m_pOpenATCCommWithCameraThread[i]->GetCameraInfo(), szCameraIp, strlen(szCameraIp)) == 0)
							{
								m_pOpenATCCommWithCameraThread[i]->SetClientSocket(clentSock);
								m_pOpenATCCommWithCameraThread[i]->SetCameraInfo(szCameraIp, nCameraPort);
								bFlag = true;
							}
						}
					}
					if (!bFlag)
					{
						for (i = 0;i < MAX_CAMERA_SIZE;i++)
						{
							if (m_pOpenATCCommWithCameraThread[i] == NULL)
							{
								m_pOpenATCCommWithCameraThread[i] = new COpenATCCommWithCameraThread();
								m_pOpenATCCommWithCameraThread[i]->Init(m_pOpenATCParameter, m_pOpenATCRunStatus, m_pOpenATCLog);
								m_pOpenATCCommWithCameraThread[i]->SetClientSocket(clentSock);
								m_pOpenATCCommWithCameraThread[i]->SetCameraInfo(szCameraIp, nCameraPort);

								m_pOpenATCCommWithCameraThread[i]->Start();
								break;
							}
						}	
					}
				}
				else
				{
					bool m_bUDPComStatusWithCfg = false;
					bool m_bTCPComStatusWithCfg = false;
					m_pOpenATCRunStatus->GetComStatusWithCfg(m_bUDPComStatusWithCfg, m_bTCPComStatusWithCfg);

					if (m_pOpenATCCommWithCfgThread == NULL)
					{
						if (!m_bUDPComStatusWithCfg)
						{
							m_pOpenATCCommWithCfgThread = new COpenATCCommWithCfgSWThread();
							m_pOpenATCCommWithCfgThread->Init(m_pOpenATCParameter, m_pOpenATCRunStatus, m_pOpenATCLog, TCP_SERVICE, false, m_chOpenATCVersion);
							m_pOpenATCCommWithCfgThread->SetClientSocket(clentSock);
							m_pOpenATCCommWithCfgThread->SetClientInfo(szCameraIp, nCameraPort);
							m_pOpenATCCommWithCfgThread->Start();

							strncpy(m_szClientIp, szCameraIp, strlen(szCameraIp));
			                m_nClientPort = nCameraPort;
							m_pOpenATCRunStatus->SetComStatusWithCfg(false, true);
						}
					}
					else
					{
						if (memcmp(m_szClientIp, szCameraIp, strlen(szCameraIp)) != 0 || m_nClientPort != nCameraPort)
						{
							if (m_bTCPComStatusWithCfg)
							{
								m_pOpenATCCommWithCfgThread->SetClientSocket(clentSock);
							    m_pOpenATCCommWithCfgThread->SetClientInfo(szCameraIp, nCameraPort);

								strncpy(m_szClientIp, szCameraIp, strlen(szCameraIp));
			                    m_nClientPort = nCameraPort;
								m_pOpenATCRunStatus->SetComStatusWithCfg(false, true);
							}
						}
					}
				}
			}

			OpenATCSleep(100);
		}

		m_acceptSock.Close();
    }

	m_pOpenATCLog->LogOneMessage(LEVEL_INFO, "COpenATCCommWithCameraListenThread exit!");
    return OPENATC_RTN_OK;
}

void  COpenATCCommWithCameraListenThread::Init(COpenATCParameter *pParameter, COpenATCRunStatus *pRunStatus, COpenATCLog * pOpenATCLog, BYTE byServiceSource, char * pOpenATCVersion)
{
	m_pOpenATCParameter = pParameter;
	m_pOpenATCRunStatus = pRunStatus;
	m_pOpenATCLog       = pOpenATCLog;

	m_byServiceSource   = byServiceSource;
	
	memset(m_chOpenATCVersion, 0, sizeof(m_chOpenATCVersion));
	memcpy(m_chOpenATCVersion, pOpenATCVersion, strlen(pOpenATCVersion));
}

int COpenATCCommWithCameraListenThread::Start()
{
    if (false == m_bExitFlag)
	{
        return OPENATC_RTN_OK;
	}

    bool bOK = true;
    m_bExitFlag = false;

#ifdef _WIN32
	m_hThread = CreateThread(NULL, 0, (unsigned long(COMMWITHCAMERALISTEN_CALLBACK *)(void *))&RunThread, this, 0, NULL);
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

int COpenATCCommWithCameraListenThread::Join()
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
		m_pOpenATCLog->LogOneMessage(LEVEL_INFO, "COpenATCCommWithCameraListenThread Join ok!");
	}
	else
	{
		m_pOpenATCLog->LogOneMessage(LEVEL_INFO, "COpenATCCommWithCameraListenThread Join fail!");
	}

	m_bExitFlag = true;
    m_hThread	= 0;
    return nRet;
}

int COpenATCCommWithCameraListenThread::Detach()
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

void* COMMWITHCAMERALISTEN_CALLBACK COpenATCCommWithCameraListenThread::RunThread(void *pParam)
{
    COpenATCCommWithCameraListenThread *pThis = (COpenATCCommWithCameraListenThread *)pParam;

    int nRet = pThis->Run();
    return (void *)nRet;
}

void COpenATCCommWithCameraListenThread::OpenATCSleep(long nMsec)
{
#ifdef _WIN32
    Sleep(nMsec);
#else
    usleep(nMsec*1000);
#endif
}
