/*====================================================================
模块名 ：和相机的通信接口
文件名 ：OpenATCCommWithCameraThread.cpp
相关文件：OpenATCCommWithCameraThread.h
实现功能：和相机进行交互
作者 ：李永萍
版权 ：<Copyright(C) 2019-2020 Suzhou Keda Technology Co., Ltd. All rights reserved.>
---------------------------------------------------------------------------------------------------------------------
修改记录：
日 期           版本    修改人             走读人             修改记录
2019/09/06      V1.0    李永萍             李永萍             创建模块
====================================================================*/
#include "OpenATCCommWithCameraThread.h"
#include "OpenATCPackUnpackSimpleFactory.h"
#include "OpenATCSocket.h"

COpenATCCommWithCameraThread::COpenATCCommWithCameraThread()
{
	m_hThread		= 0;
    m_dwThreadRet	= 0;
	m_bExitFlag		= true;
    m_nDetachState  = 0;

	m_pOpenATCParameter = NULL;
	m_pOpenATCRunStatus = NULL;
	m_pOpenATCLog       = NULL;

	m_clientSock.SetSocketType(SOCK_STREAM);
    m_clientSock.SetComType(TCP_SERVICE);

	m_nSendTimeOut = 100;
	m_nRecvTimeOut = 100;

	memset(m_szCameraIp, 0, sizeof(m_szCameraIp));  
    m_nCameraPort = 0;
	m_bConnectStatus = true;
    m_chHeartIndex = 0;
	m_lastSendHeartTime = 0;
	m_lastReadOkTime = 0;

	m_pDataPackUnpackMode = COpenATCPackUnpackSimpleFactory::Create(PACK_UNPACK_MODE_CAMERA);

	memset(m_chRecvBuff, 0, MAX_HEARTDATA_SIZE + 1);
	memset(m_chUnPackedBuff, 0x00, MAX_HEARTDATA_SIZE + 1);
	memset(m_chOldSendBuff, 0x00, MAX_LAMPCLRDATA_SIZE + 1);
}

COpenATCCommWithCameraThread::~COpenATCCommWithCameraThread()
{
    if (!m_bExitFlag && (m_nDetachState == 0))
    {
        Join();
    }

	m_clientSock.Close();

    memset(m_szCameraIp, 0, sizeof(m_szCameraIp));  
    m_nCameraPort = 0;
	m_bConnectStatus = true;
    m_chHeartIndex = 0;
	m_lastSendHeartTime = 0;
	m_lastReadOkTime = 0;
}

int COpenATCCommWithCameraThread::Run()
{
#ifndef _WIN32
	prctl(15,"COpenATCCommWithCameraThread",0,0,0);
#endif
	int				nRecvLength  = 0;
    unsigned int	nPackLength  = 0;
	unsigned int    nMessageType = 0;
    int				nRet		 = 0;
	bool			bFlag		 = false;

	m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "COpenATCCommWithCameraThread ComWithCamera:%s Start!", m_szCameraIp);

	char szCameraIp[20] = {0};
	int  nCameraPort = 0;

	while (!m_bExitFlag)
    {
		SendHeartToCamera();//发送心跳数据
		SendLampClrDataToCamera();//发送灯色数据

	    nRecvLength = 0;
		memset(szCameraIp, 0x00, sizeof(szCameraIp));
		nRecvLength = m_clientSock.Read((char *)m_chRecvBuff, MAX_HEARTDATA_SIZE, m_nRecvTimeOut);
		if (nRecvLength > 0)
		{
			m_pDataPackUnpackMode->Write((unsigned char *)(char *)m_chRecvBuff, nRecvLength);
		}
		else
		{
			OpenATCSleep(100);   
			continue;
		}

		nPackLength = 0;
		nRet = m_pDataPackUnpackMode->Read(m_chUnPackedBuff, nPackLength);
		if (nRet == ReadOk)
		{
			//m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "COpenATCCommWithCameraThread ComWithCamera:%s Receive HeartIndex:%d!", m_szCameraIp, m_chUnPackedBuff[0]);
			//if (m_chHeartIndex == m_chUnPackedBuff[0])//收到心跳回应
			{
				m_lastReadOkTime = time(NULL);
				m_bConnectStatus = true;
			}
		}
    }

	m_pOpenATCLog->LogOneMessage(LEVEL_INFO, "COpenATCCommWithCameraThread ComWithCamera:%s Exit!", m_szCameraIp);
    return OPENATC_RTN_OK;
}

void  COpenATCCommWithCameraThread::Init(COpenATCParameter *pParameter, COpenATCRunStatus *pRunStatus, COpenATCLog * pOpenATCLog)
{
	m_pOpenATCParameter = pParameter;
	m_pOpenATCRunStatus = pRunStatus;
	m_pOpenATCLog       = pOpenATCLog;
}

int COpenATCCommWithCameraThread::Start()
{
    if (false == m_bExitFlag)
	{
        return OPENATC_RTN_OK;
	}

    bool bOK = true;
    m_bExitFlag = false;

#ifdef _WIN32
	m_hThread = CreateThread(NULL, 0, (unsigned long(COMMWITHCAMERA_CALLBACK *)(void *))&RunThread, this, 0, NULL);
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

int COpenATCCommWithCameraThread::Join()
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
		m_pOpenATCLog->LogOneMessage(LEVEL_INFO, "COpenATCCommWithCameraThread Join ok!");
	}
	else
	{
		m_pOpenATCLog->LogOneMessage(LEVEL_INFO, "COpenATCCommWithCameraThread Join fail!");
	}

	m_bExitFlag = true;
    m_hThread	= 0;
    return nRet;
}

int COpenATCCommWithCameraThread::Detach()
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

void* COMMWITHCAMERA_CALLBACK COpenATCCommWithCameraThread::RunThread(void *pParam)
{
    COpenATCCommWithCameraThread *pThis = (COpenATCCommWithCameraThread *)pParam;

    int nRet = pThis->Run();
    return (void *)nRet;
}

void  COpenATCCommWithCameraThread::SetClientSocket(SOCKET & socket)
{
	m_clientSock.Close();
	m_clientSock.SetSocket(socket);
	m_bConnectStatus = true;
}

void  COpenATCCommWithCameraThread::SetCameraInfo(char *chCameraIp, int nCameraPort)
{
	memcpy(m_szCameraIp, chCameraIp, sizeof(m_szCameraIp));  
    m_nCameraPort = nCameraPort;
}

char * COpenATCCommWithCameraThread::GetCameraInfo()
{
	return m_szCameraIp;
}

void COpenATCCommWithCameraThread::SendLampClrDataToCamera()
{
	int  i = 0, j = 0;
	char chR = 0, chY = 0, chG = 0;

	TCanData tCanData[MAX_LAMPCLRDATA_SIZE];//tCanData分别对应要发送的板卡的数据
	memset(tCanData, 0x00, sizeof(TCanData));

	unsigned char chSendBuff[MAX_LAMPCLRDATA_SIZE + 1] = {0};
	memset(chSendBuff, 0x00, sizeof(chSendBuff));

	TLampCltBoardData tLampCtlBoardInfo;
    memset(&tLampCtlBoardInfo,0,sizeof(tLampCtlBoardInfo));
    m_pOpenATCRunStatus->GetLampCtlBoardData(tLampCtlBoardInfo);

    for (i = 0;i < MAX_LAMPCLRDATA_SIZE / 2;i++)
    {
        PTOneLampCltBoardData pOne = &(tLampCtlBoardInfo.m_atLampData[i]);

        for (j = 0;j < C_N_CHANNELNUM_PER_BOARD;j ++)
        {
            GetRYGStatusByGroup(pOne->m_achLampGroupStatus[j], chR, chY, chG);

			if (j == 0)
			{
				tCanData[0 + i * 2].CanData.io1 = (chG == (char)LAMP_CLR_ON) ? 1:0;//绿
				tCanData[0 + i * 2].CanData.io2 = (chR == (char)LAMP_CLR_ON) ? 1:0;//红
				tCanData[0 + i * 2].CanData.io3 = (chY == (char)LAMP_CLR_ON) ? 1:0;//黄
			}
			else if (j == 1)
			{
				tCanData[0 + i * 2].CanData.io4 = (chG == (char)LAMP_CLR_ON) ? 1:0;//绿
				tCanData[0 + i * 2].CanData.io5 = (chR == (char)LAMP_CLR_ON) ? 1:0;//红
				tCanData[0 + i * 2].CanData.io6 = (chY == (char)LAMP_CLR_ON) ? 1:0;//黄
			}
			else if (j == 2)
			{
				tCanData[0 + i * 2].CanData.io7 = (chG == (char)LAMP_CLR_ON) ? 1:0;//绿
				tCanData[0 + i * 2].CanData.io8 = (chR == (char)LAMP_CLR_ON) ? 1:0;//红
				tCanData[1 + i * 2].CanData.io1 = (chY == (char)LAMP_CLR_ON) ? 1:0;//黄
			}
			else
			{
				tCanData[1 + i * 2].CanData.io2 = (chG == (char)LAMP_CLR_ON) ? 1:0;//绿
				tCanData[1 + i * 2].CanData.io3 = (chR == (char)LAMP_CLR_ON) ? 1:0;//红
				tCanData[1 + i * 2].CanData.io4 = (chY == (char)LAMP_CLR_ON) ? 1:0;//黄
			}
        }

		tCanData[1 + i * 2].CanData.io5 = 0;
		tCanData[1 + i * 2].CanData.io6 = 0;
		tCanData[1 + i * 2].CanData.io7 = 0;
		tCanData[1 + i * 2].CanData.io8 = 0;

		memcpy(chSendBuff + i * 2, &tCanData[0 + i * 2].chCanData, 1);
		memcpy(chSendBuff + 1 + i * 2, &tCanData[1 + i * 2].chCanData, 1);
    }    

	if (memcmp(m_chOldSendBuff, chSendBuff, MAX_LAMPCLRDATA_SIZE) != 0)
	{
		unsigned char chPackedBuff[MAX_DATA_SIZE + 1];
		memset(chPackedBuff, 0x00, sizeof(chPackedBuff));

		unsigned int  dwDstCount = 0;
		unsigned int  dwCheckValue = 0;

		memcpy(m_chOldSendBuff, chSendBuff, MAX_LAMPCLRDATA_SIZE);

		if (m_bConnectStatus)
		{
			chPackedBuff[0] = PACK_LAMPCLR_HEAD;
			dwDstCount = 1;

			dwCheckValue = PACK_LAMPCLR_HEAD;

			for (unsigned int i = 0;i < MAX_LAMPCLRDATA_SIZE;i++)
			{
				chPackedBuff[dwDstCount++] = chSendBuff[i];
				dwCheckValue ^= chSendBuff[i];
			}
			
			chPackedBuff[dwDstCount++] = dwCheckValue;
			chPackedBuff[dwDstCount++] = PACK_TAIL;

			if (m_clientSock.Write(chPackedBuff, dwDstCount, m_nSendTimeOut) == OPENATC_RTN_FAILED)
			{
				m_bConnectStatus = false;
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "SendLampClrDataToCamera ComWithCamera:%s Failed!", m_szCameraIp);
			}
			else
			{
				//m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "SendLampClrDataToCamera ComWithCamera:%s Succeed!", m_szCameraIp);
			}
		}
	}
}

void   COpenATCCommWithCameraThread::SendHeartToCamera()
{
	unsigned int dwDstCount = MAX_HEARTDATA_SIZE;
	unsigned char chPackedBuff[MAX_HEARTDATA_SIZE + 1];

	if (m_bConnectStatus && labs(time(NULL) - m_lastSendHeartTime) > HEART_INTERVAL_TIME)
	{
		chPackedBuff[0] = PACK_HEART_HEAD;
		chPackedBuff[1] = PACK_HEART_HEAD;
		chPackedBuff[2] = m_chHeartIndex;
		chPackedBuff[3] = PACK_TAIL;

		if (m_clientSock.Write(chPackedBuff, dwDstCount, m_nSendTimeOut) != OPENATC_RTN_FAILED)
		{
			//m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "SendHeartToCamera ComWithCamera:%s HeartIndex:%d Succeed!", m_szCameraIp, m_chHeartIndex);
	        m_lastSendHeartTime = time(NULL);
		}
		else
		{
			m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "SendHeartToCamera ComWithCamera:%s HeartIndex:%d Failed!", m_szCameraIp, m_chHeartIndex);
			m_bConnectStatus = false;
		}

		m_chHeartIndex += 1;
		if (m_chHeartIndex > MAX_HEART_INDEX)
		{
			m_chHeartIndex = 0;
		}
	}
}

void COpenATCCommWithCameraThread::OpenATCSleep(long nMsec)
{
#ifdef _WIN32
    Sleep(nMsec);
#else
    usleep(nMsec*1000);
#endif
}

void COpenATCCommWithCameraThread::GetRYGStatusByGroup(char chGroup,char & chR,char & chY,char & chG)
{
	TCanData tCanData;
	memset(&tCanData, 0x00, sizeof(tCanData));

	tCanData.chCanData = chGroup;

	if (tCanData.CanData.io1 == 1)
	{
		chR = LAMP_CLR_ON;
	}
	else
	{
		chR = LAMP_CLR_OFF;
	}

	if (tCanData.CanData.io2 == 1)
	{
		chY = LAMP_CLR_ON;
	}
	else
	{
		chY = LAMP_CLR_OFF;
	}

	if (tCanData.CanData.io3 == 1)
	{
		chG = LAMP_CLR_ON;
	}
	else
	{
		chG = LAMP_CLR_OFF;
	}
}