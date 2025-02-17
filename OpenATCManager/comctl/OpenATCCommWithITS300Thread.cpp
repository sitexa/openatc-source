/*====================================================================
模块名 ：和ITS300交互的通信接口
文件名 ：OpenATCCommWithITS300Thread.cpp
相关文件：OpenATCCommWithITS300Thread.h
实现功能：和ITS300进行交互
作者 ：李永萍
版权 ：<Copyright(C) 2019-2020 Suzhou Keda Technology Co., Ltd. All rights reserved.>
---------------------------------------------------------------------------------------------------------------------
修改记录：
日 期           版本    修改人             走读人             修改记录
2019/09/06      V1.0    李永萍             李永萍             创建模块
====================================================================*/
#include "OpenATCCommWithITS300Thread.h"
#include "OpenATCPackUnpackSimpleFactory.h"
#include "../../Include/OpenATCLog.h"

COpenATCCommWithITS300Thread::COpenATCCommWithITS300Thread()
{
	m_hThread		= 0;
    m_dwThreadRet	= 0;
	m_bExitFlag		= true;
    m_nDetachState  = 0;

	m_pOpenATCParameter = NULL;
	m_pOpenATCRunStatus = NULL;
	m_pOpenATCLog       = NULL;

	m_commHelper.SetRecvTimeOut(400);
	m_commHelper.SetSendTimeOut(400);

	m_chRecvBuff  = new unsigned char[RECV_BUFFER_SIZE];
	memset(m_chRecvBuff, 0, RECV_BUFFER_SIZE);
	m_chUnPackedBuff = new unsigned char[UNPACKED_BUFFER_SIZE];
	memset(m_chUnPackedBuff, 0x00, UNPACKED_BUFFER_SIZE);
	m_chSendBuff = new unsigned char[SEND_BUFFER_SIZE];
	memset(m_chSendBuff, 0x00, SEND_BUFFER_SIZE);
	m_chPackedBuff = new unsigned char[PACKED_BUFFER_SIZE];
	memset(m_chPackedBuff, 0x00, PACKED_BUFFER_SIZE);

	m_unMasterCount = 0;
	memset(&m_nVehicleCount, 0x00, sizeof(m_nVehicleCount));

	m_pDataPackUnpackMode = COpenATCPackUnpackSimpleFactory::Create(PACK_UNPACK_MODE_ITS300);

	m_tAreaInfo.m_chAscRegionNo = 0;
    m_tAreaInfo.m_usAscRoadNo	= 0;

	m_nHeartLastTime = time(NULL);
}

COpenATCCommWithITS300Thread::~COpenATCCommWithITS300Thread()
{
    if (!m_bExitFlag && (m_nDetachState == 0))
    {
        Join();
    }

	if (m_chRecvBuff != NULL)
	{
		delete []m_chRecvBuff;
		m_chRecvBuff = NULL;
	}

	if (m_chUnPackedBuff != NULL)
	{
		delete []m_chUnPackedBuff;
		m_chUnPackedBuff = NULL;
	}

	m_commHelper.Close();

	delete m_pDataPackUnpackMode;
	m_pDataPackUnpackMode = NULL;
}

int COpenATCCommWithITS300Thread::Run()
{
#ifndef _WIN32
	prctl(15,"CommWithITS300Thread",0,0,0);
#endif
	int				nRecvLength = 0;
    unsigned int	nPackLength = 0;
    int				nRet		= 0;
	int				i			= 0;
	unsigned int	nID			= 0;
	bool			bFlag		= false;

	m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "COpenATCCommWithITS300Thread start!");
	char szPeerIp[20] = {0};

	while (!m_bExitFlag)
    {
		if (m_commHelper.AcceptConnection("", m_nPort, false) == OPENATC_RTN_FAILED)		// 端口号暂定30000
		{
			OpenATCSleep(500);
			continue;
		}
		else
		{
			m_pDataPackUnpackMode->ClearBuff();
		}

		while (true)
		{
			nRecvLength = 0;
			memset(szPeerIp, 0x00, sizeof(szPeerIp));
			if (m_commHelper.Read(m_chRecvBuff, RECV_BUFFER_SIZE, nRecvLength, bFlag, szPeerIp) == OPENATC_RTN_OK)
			{
				if (nRecvLength > 0)
				{
					m_pDataPackUnpackMode->Write((unsigned char *)(char *)m_chRecvBuff, nRecvLength);
				}
				else
				{
					OpenATCSleep(100);   
				}
			}
			else
			{
				OpenATCSleep(100);   
			}

			if (nRecvLength > 0)
			{
				nPackLength = 0;
				nRet = m_pDataPackUnpackMode->Read(m_chUnPackedBuff, nPackLength);
				if (nRet == ReadOk)
				{
					//m_pOpenATCLog->LogOneMessage(LEVEL_INFO, COpenATCCommWithITS300Thread ParserPack length is %d!", nPackLength);
					ParserPack(m_chUnPackedBuff, nPackLength, szPeerIp);
				}
				else if (nRet != ReadNoData)
				{
					//m_pOpenATCLog->LogOneMessage(LEVEL_ERROR, "COpenATCCommWithITS300Thread unpacker read err!");
				}
			}

			long lCurTime = time(NULL);
			if (labs(lCurTime - m_nHeartLastTime) > HEART_INTERVAL_TIME)
			{
				//break;
			}
		}

		m_commHelper.Close();
    }

	m_pOpenATCLog->LogOneMessage(LEVEL_INFO, "COpenATCCommWithITS300Thread exit!");
    return OPENATC_RTN_OK;
}

void  COpenATCCommWithITS300Thread::Init(COpenATCParameter *pParameter, COpenATCRunStatus *pRunStatus, COpenATCLog * pOpenATCLog, int nComType)
{
	m_pOpenATCParameter = pParameter;
	m_pOpenATCRunStatus = pRunStatus;
	m_pOpenATCLog       = pOpenATCLog;
	m_nComType          = nComType;

	if (m_nComType == COM_WITH_ITS300)
	{
		m_nPort = 30000;
	}
	else
	{
		TAscSimulate tSimulateInfo;
		m_pOpenATCParameter->GetSimulateInfo(tSimulateInfo);
		m_nPort = tSimulateInfo.m_nVideoDetectorPort;
	}

	m_pOpenATCParameter->GetAscAreaInfo(m_tAreaInfo);
}

int COpenATCCommWithITS300Thread::Start()
{
    if (false == m_bExitFlag)
	{
        return OPENATC_RTN_OK;
	}

    bool bOK = true;
    m_bExitFlag = false;

#ifdef _WIN32
	m_hThread = CreateThread(NULL, 0, (unsigned long(COMMWITHITS300_CALLBACK *)(void *))&RunThread, this, 0, NULL);
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

int COpenATCCommWithITS300Thread::Join()
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
		m_pOpenATCLog->LogOneMessage(LEVEL_INFO, "COpenATCCommWithITS300Thread Join ok!");
	}
	else
	{
		m_pOpenATCLog->LogOneMessage(LEVEL_INFO, "COpenATCCommWithITS300Thread Join fail!");
	}

	m_bExitFlag = true;
    m_hThread	= 0;
    return nRet;
}

int COpenATCCommWithITS300Thread::Detach()
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

void* COMMWITHITS300_CALLBACK COpenATCCommWithITS300Thread::RunThread(void *pParam)
{
    COpenATCCommWithITS300Thread *pThis = (COpenATCCommWithITS300Thread *)pParam;

    int nRet = pThis->Run();
    return (void *)nRet;
}

void COpenATCCommWithITS300Thread::OpenATCSleep(long nMsec)
{
#ifdef _WIN32
    Sleep(nMsec);
#else
    usleep(nMsec*1000);
#endif
}

int COpenATCCommWithITS300Thread::ParserPack(const unsigned char *chUnPackedBuff, unsigned int dwPackLength, char * chPeerIp)
{
    int nRet = OPENATC_RTN_OK;

    switch (chUnPackedBuff[LINKCODE_POS])//数据链路码
    {
	case LINK_COM:
		{
			nRet = ParserPack_CtrlLink(chUnPackedBuff, dwPackLength, chPeerIp);
		}
		break;
	case LINK_BASEINFO:
		{
			nRet = ParserPack_BaseInfoLink(chUnPackedBuff, dwPackLength);
		}
		break;
	default:
		{
		    break;
		}
    }
        
    return nRet;
}

int COpenATCCommWithITS300Thread::ParserPack_CtrlLink(const unsigned char *chUnPackedBuff, unsigned int dwPackLength, char * chPeerIp)
{
	int  nRet = OPENATC_RTN_OK;
    int  nPackSize = 0;

    switch (chUnPackedBuff[CMDCODE_POS])//对象标识
    {
	case CTL_ONLINEMACHINE:
		{
			m_pOpenATCLog->LogOneMessage(LEVEL_INFO, "COpenATCCommWithITS300Thread receive heart!");
			nPackSize = AckCtl_AskHeart();
			m_nHeartLastTime = time(NULL);
		}
		break; 
	default:
		{
			break;
		}
    }

    if (nPackSize > 0)
    {
        return SendAckToPeer(nPackSize);
    }

    return nRet;
}

int COpenATCCommWithITS300Thread::ParserPack_BaseInfoLink(const unsigned char *chUnPackedBuff, unsigned int dwPackLength)
{
	int nRet = OPENATC_RTN_OK;
    int nPackSize = 0;

	switch (chUnPackedBuff[CMDCODE_POS])//对象标识
    {
	case CTL_TRAFFICFLOW_INFO:
		{
			//交通流信息
			m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "ParserPack_BaseInfoLink receive traffic flow info");

			SetTrafficFlowInfo();
		}
		break;
	case CTL_REALDETECTOR_INFO:
		{
			//车检器实时检测信息
			m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "ParserPack_BaseInfoLink receive real detector info");

			SetRealDetectorInfo();
		}
	    break;
	case CTL_VEHILCEQUEUE_INFO:
		{
			//车辆排队信息
			m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "ParserPack_BaseInfoLink receive vehicle queue info");

			SetVehicleQueueUpInfo();
		}
		break;
	case CTL_PEDDETECTOR_INFO:
		{
			//行人检测信息
			m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "ParserPack_BaseInfoLink receive ped detector info");

			SetPedDetectInfo();
		}
		break;
	case CTL_DETECTORFAULT_INFO:
		{
			//车检器故障状态信息
			m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "ParserPack_BaseInfoLink receive detector fault info");

			SetDetectorFaultInfo();
		}
		break;
	case CTL_PREEMPT_INFO:
		{
			//优先控制信息
			m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "ParserPack_BaseInfoLink receive Preempt info");

			SetPreemptInfo();
		}
		break;
	default:
		{
		    break;
		}
	}

    if (nPackSize > 0)
    {
        return SendAckToPeer(nPackSize);
    }

    return nRet; 
}

int COpenATCCommWithITS300Thread::SendAckToPeer(int nPackSize)
{
    unsigned int dwSize = 0;
    m_pDataPackUnpackMode->PackBuffer(m_chSendBuff, nPackSize, m_chPackedBuff, dwSize);

    if (dwSize == 0)
    {
        return OPENATC_RTN_FAILED;
    }

    return m_commHelper.Write(m_chPackedBuff, dwSize);
}

int COpenATCCommWithITS300Thread::AckCtl_AskHeart()
{
    unsigned int dwSendBuffLen = 0;

    memset(m_chSendBuff, 0x00, SEND_BUFFER_SIZE);

	m_chSendBuff[dwSendBuffLen++] = CB_VERSION_FLAG;									//版本号
	m_chSendBuff[dwSendBuffLen++] = CB_ATC_FLAG;										//发送方标识
	m_chSendBuff[dwSendBuffLen++] = CB_FLOWCOLLECTDEVICE_FLAG;							//接收方标识
	m_chSendBuff[dwSendBuffLen++] = LINK_COM;											//数据链路码
	m_chSendBuff[dwSendBuffLen++] = m_tAreaInfo.m_chAscRegionNo;						//区域号
	memcpy(m_chSendBuff + dwSendBuffLen, &m_tAreaInfo.m_usAscRoadNo, MAX_ROADNO_LENGTH);//路口号
    dwSendBuffLen += MAX_ROADNO_LENGTH;
	m_chSendBuff[dwSendBuffLen++] = ACK_QUERY;											//操作类型
	m_chSendBuff[dwSendBuffLen++] = CTL_ONLINEMACHINE;									//对象标识
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = CB_CONTENT_CHARACETER;								//数据内容格式

    return dwSendBuffLen;
}

void COpenATCCommWithITS300Thread::SetTrafficFlowInfo()
{
    unsigned int  i = 0, j = 0;

	unsigned char chDetectorID[MAX_VEHICLEDETECTOR_COUNT];
	memset(chDetectorID, 0, sizeof(chDetectorID));

	unsigned char chDetectorValue[MAX_VEHICLEDETECTOR_COUNT];
	memset(chDetectorValue, 0, sizeof(chDetectorValue));

	int nDetectorIDPos = C_N_DETECTOR_COUNT;

	for (i = 0;i < m_chUnPackedBuff[C_N_DETECTOR_COUNT];i++)
	{
		nDetectorIDPos += 1;
		chDetectorID[i] = m_chUnPackedBuff[nDetectorIDPos];
		nDetectorIDPos += 1;
		chDetectorValue[i] = m_chUnPackedBuff[nDetectorIDPos];
		nDetectorIDPos += 1;
		nDetectorIDPos += 1;
	}

	//获取配置软件配置的车检板数量
    TVehicleDetector atVehDetector[MAX_VEHICLEDETECTOR_COUNT];
    memset(atVehDetector,0,sizeof(TVehicleDetector) * MAX_VEHICLEDETECTOR_COUNT);
    m_pOpenATCParameter->GetVehicleDetectorTable(atVehDetector);

	unsigned long nGlobalCounter = m_pOpenATCRunStatus->GetGlobalCounter();

	TVehDetBoardData tVehDetBoardData;                  
    m_pOpenATCRunStatus->GetVehDetBoardData(tVehDetBoardData);

	int nVehDetBoardIndex = 0;
	int nDetectorIndex = 0;

	//每一次线圈是否存在都重新判断
	for (int nBoardIndex = 0; nBoardIndex < 4; nBoardIndex++)
	{
		memset(tVehDetBoardData.m_atVehDetData[nBoardIndex].m_bVehDetExist, 0, sizeof(tVehDetBoardData.m_atVehDetData[nBoardIndex].m_bVehDetExist));
	}
	for (i = 0;i < MAX_VEHICLEDETECTOR_COUNT;i++)
	{
		if (atVehDetector[i].m_byVehicleDetectorNumber == 0 || atVehDetector[i].m_byDetectorType != VIDEO_DETECTOR)
		{
			continue;
		}

		for (j = 0;j < MAX_VEHICLEDETECTOR_COUNT;j++)
		{
			//if (atVehDetector[i].m_byVehicleDetectorNumber == chDetectorID[j])		//2022.6.7
			if (atVehDetector[i].m_byVehicleDetectorNumber == (chDetectorID[j] + 1))
			{
				//nVehDetBoardIndex = chDetectorID[j] / C_N_MAXDETINPUT_NUM;
				//nDetectorIndex = (chDetectorID[j] - 1) % C_N_MAXDETINPUT_NUM;
				nVehDetBoardIndex = chDetectorID[j] / C_N_MAXDETINPUT_NUM;
				nDetectorIndex = chDetectorID[j] % C_N_MAXDETINPUT_NUM;
				tVehDetBoardData.m_atVehDetData[nVehDetBoardIndex].m_achVehChgVal[nDetectorIndex] = chDetectorValue[j];
				tVehDetBoardData.m_atVehDetData[nVehDetBoardIndex].m_anVehChgValCounter[nDetectorIndex] = nGlobalCounter;
				//仿真对应配置的检测器存在则会发送交通流信息，此刻将线圈存在标志置为true
				tVehDetBoardData.m_atVehDetData[nVehDetBoardIndex].m_bVehDetExist[nDetectorIndex] = true;

				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "SetTrafficFlowInfo DetectorID:%d DetectorValue:%d", chDetectorID[j], chDetectorValue[j]);
				break;
			}
		}
	}

	m_pOpenATCRunStatus->SetVehDetBoardData(tVehDetBoardData);
}

void COpenATCCommWithITS300Thread::SetRealDetectorInfo()
{
	if (m_chUnPackedBuff[C_N_DETECTOR_INDEX] < 0 || m_chUnPackedBuff[C_N_DETECTOR_INDEX] > MAX_VEHICLEDETECTOR_COUNT - 1)	//仿真发送检测器Index是从0开始发送的
	{
		return;
	}

	//获取配置软件配置的车检板数量
    TVehicleDetector atVehDetector[MAX_VEHICLEDETECTOR_COUNT];
    memset(atVehDetector,0,sizeof(TVehicleDetector) * MAX_VEHICLEDETECTOR_COUNT);
    m_pOpenATCParameter->GetVehicleDetectorTable(atVehDetector);

	unsigned long nGlobalCounter = m_pOpenATCRunStatus->GetGlobalCounter();

    TVehDetBoardData tVehDetBoardData;                  
    m_pOpenATCRunStatus->GetVehDetBoardData(tVehDetBoardData);

	//unsigned int nDetectorIndex = m_chUnPackedBuff[C_N_DETECTOR_INDEX] % C_N_MAXDETINPUT_NUM - 1;
	//unsigned int nDetectorID = m_chUnPackedBuff[C_N_DETECTOR_INDEX];						
	//unsigned int nVehDetBoardIndex = (nDetectorID - 1) / C_N_MAXDETINPUT_NUM;	//2022.6.7
	unsigned int nDetectorIndex = m_chUnPackedBuff[C_N_DETECTOR_INDEX] % C_N_MAXDETINPUT_NUM;
	unsigned int nDetectorID = m_chUnPackedBuff[C_N_DETECTOR_INDEX] + 1;
	unsigned int nVehDetBoardIndex = (nDetectorID - 1) / C_N_MAXDETINPUT_NUM;

	bool bFlag = false;

	int  i = 0, j = 0;
	for (i = 0;i < MAX_VEHICLEDETECTOR_COUNT;i++)
	{
		if (atVehDetector[i].m_byVehicleDetectorNumber == 0 || atVehDetector[i].m_byDetectorType != VIDEO_DETECTOR)
		{
			continue;
		}

		for (j = 0;j < MAX_VEHICLEDETECTOR_COUNT;j++)
		{
			if (atVehDetector[i].m_byVehicleDetectorNumber == nDetectorID)
			{
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "SetRealDetectorInfo VehDetBoardIndex:%d DetectorIndex:%d DetectorID:%d DetectorValue:%d", nVehDetBoardIndex, nDetectorIndex, m_chUnPackedBuff[C_N_DETECTOR_INDEX], m_chUnPackedBuff[C_N_DETECTOR_VALUE]);

				if (tVehDetBoardData.m_atVehDetData[nVehDetBoardIndex].m_achVehChgVal[nDetectorIndex] == 1 && m_chUnPackedBuff[C_N_DETECTOR_VALUE] == 0)
				{
					m_nVehicleCount[nVehDetBoardIndex][nDetectorIndex] += 1;
					m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "SetRealDetectorInfo VehDetBoardIndex:%d DetectorIndex:%d DetectorID:%d VehicleCount:%d", nVehDetBoardIndex, nDetectorIndex, m_chUnPackedBuff[C_N_DETECTOR_INDEX], m_nVehicleCount[nVehDetBoardIndex][nDetectorIndex]);
				}
				tVehDetBoardData.m_atVehDetData[nVehDetBoardIndex].m_achVehChgVal[nDetectorIndex] = m_chUnPackedBuff[C_N_DETECTOR_VALUE];
				tVehDetBoardData.m_atVehDetData[nVehDetBoardIndex].m_anVehChgValCounter[nDetectorIndex] = nGlobalCounter;
				m_pOpenATCRunStatus->SetVehDetBoardData(tVehDetBoardData);
				bFlag = true;
				break;
			}
		}

		if (bFlag)
		{
			break;
		}
	}
}

void COpenATCCommWithITS300Thread::SetVehicleQueueUpInfo()
{
	//初始化车检板使用信息数组
    TVehicleDetector atVehDetector[MAX_VEHICLEDETECTOR_COUNT];
    memset(atVehDetector,0,sizeof(TVehicleDetector) * MAX_VEHICLEDETECTOR_COUNT);
    m_pOpenATCParameter->GetVehicleDetectorTable(atVehDetector);

	TVehDetBoardData tVehDetBoardData;                  
    m_pOpenATCRunStatus->GetVehDetBoardData(tVehDetBoardData);

	TVehicleQueueUpInfo tVehicleQueueUpInfo[MAX_VEHICLEDETECTOR_COUNT];
	memset(tVehicleQueueUpInfo,0,sizeof(tVehicleQueueUpInfo));

	unsigned int  i = 0, j = 0, k = 0;
	unsigned int  nVehDetBoardIndex = 0;
	unsigned int  nDetectorID = 0;
	unsigned int  nVehicleQueueUpLength = 0;
	unsigned int  nPos = C_N_QUEUE_LENGTH;
	unsigned int  nQueueLength = m_chUnPackedBuff[nPos];

	bool bFlag = false;

	for (i = 0;i < nQueueLength;i++)
	{
		nPos += 1;
		//nDetectorID = m_chUnPackedBuff[nPos + i]; 2022.6.7
		nDetectorID = m_chUnPackedBuff[nPos + i] + 1;

		nPos += 1;
		nVehicleQueueUpLength = m_chUnPackedBuff[nPos + i];

		nVehDetBoardIndex = (nDetectorID - 1) / C_N_MAXDETINPUT_NUM;
		if (nVehDetBoardIndex >= 0 && nVehDetBoardIndex < C_N_MAXDETBOARD_NUM)
		{
			for (j = 0;j < MAX_VEHICLEDETECTOR_COUNT;j++)
			{
				if (atVehDetector[j].m_byVehicleDetectorNumber == nDetectorID && atVehDetector[j].m_byDetectorType == VIDEO_DETECTOR)
				{
					bFlag = true;

					for (k = 0;k < MAX_VEHICLEDETECTOR_COUNT;k++)
					{
						if (tVehicleQueueUpInfo[k].m_nDetectorID == 0)
						{
							tVehicleQueueUpInfo[k].m_nDetectorID = nDetectorID;
							tVehicleQueueUpInfo[k].m_byVehicleDetectorCallPhase = atVehDetector[j].m_byVehicleDetectorCallPhase;
							tVehicleQueueUpInfo[k].m_nVehicleQueueUpLength = nVehicleQueueUpLength;
							m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "SetVehicleQueueUpInfo QueueLengthCount:%d DetectorID:%d VehicleQueueUpLength:%d", nQueueLength, nDetectorID, nVehicleQueueUpLength);
							break;
						}
					}
		
					break;
				}
			}
		}

		nPos += 1;
	}

	if (bFlag)
	{
		m_pOpenATCRunStatus->SetVehicleQueueUpInfo(tVehicleQueueUpInfo);
	}
}

void COpenATCCommWithITS300Thread::SetPedDetectInfo()
{
	//初始化IO板使用信息数组
	TPedestrianDetector atPedDetector[MAX_PEDESTRIANDETECTOR_COUNT];
    memset(atPedDetector,0,sizeof(TPedestrianDetector) * MAX_PEDESTRIANDETECTOR_COUNT);
    m_pOpenATCParameter->GetPedDetectorTable(atPedDetector);

	TIOBoardData tIOBoardData;                  
    m_pOpenATCRunStatus->GetIOBoardData(tIOBoardData);

	TPedDetectInfo tPedDetectInfo[MAX_PEDESTRIANDETECTOR_COUNT];
	memset(&tPedDetectInfo,0,sizeof(tPedDetectInfo));

	unsigned int  i = 0, j = 0, k = 0;
	unsigned int  nIOBoardIndex = 0;
	unsigned int  nDetectorID = 0;
	unsigned int  nPedCount = 0;
	unsigned int  nPos = C_N_PEDDETECTOR_COUNT;
	unsigned int  nPedDetectorCount = m_chUnPackedBuff[nPos];

	bool bFlag = false;

	for (i = 0;i < nPedDetectorCount;i++)
	{
		nPos += 1;
		//nDetectorID = m_chUnPackedBuff[nPos + i];  2022.6.7
		nDetectorID = m_chUnPackedBuff[nPos + i] + 1;

		nPos += 1;
		nPedCount = m_chUnPackedBuff[nPos + i];

		nIOBoardIndex = (nDetectorID - 1) / C_N_MAXIOINPUT_NUM;
		if (nIOBoardIndex >= 0 && nIOBoardIndex < C_N_MAXIOBOARD_NUM)
		{
			for (j = 0;j < MAX_PEDESTRIANDETECTOR_COUNT;j++)
			{
				if (atPedDetector[j].m_byPedestrianDetectorNumber == nDetectorID)// && atPedDetector[j].m_byDetectorType == VIDEO_DETECTOR)
				{
					bFlag = true;

					for (k = 0;k < MAX_PEDESTRIANDETECTOR_COUNT;k++)
					{
						if (tPedDetectInfo[k].m_nDetectorID == 0)
						{
							tPedDetectInfo[k].m_nDetectorID = nDetectorID;
							tPedDetectInfo[k].m_byVehicleDetectorCallPhase = atPedDetector[j].m_byPedestrianDetectorCallPhase;
							tPedDetectInfo[k].m_nPedCount = nPedCount;
							m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "SetPedDetectorInfo PedDetectorCount:%d DetectorID:%d PedCount:%d", nPedDetectorCount, nDetectorID, nPedCount);
							break;
						}
					}

					break;
				}
			}
		}

		nPos += 1;
	}

	if (bFlag)
	{
		m_pOpenATCRunStatus->SetPedDetectInfo(tPedDetectInfo);
	}
}

void COpenATCCommWithITS300Thread::SetDetectorFaultInfo()
{
	if (m_chUnPackedBuff[C_N_DETECTOR_INDEX] < 1 || m_chUnPackedBuff[C_N_DETECTOR_INDEX] > MAX_VEHICLEDETECTOR_COUNT)
	{
		return;
	}

	//获取配置软件配置的车检板数量
    TVehicleDetector atVehDetector[MAX_VEHICLEDETECTOR_COUNT];
    memset(atVehDetector,0,sizeof(TVehicleDetector) * MAX_VEHICLEDETECTOR_COUNT);
    m_pOpenATCParameter->GetVehicleDetectorTable(atVehDetector);

	unsigned long nGlobalCounter = m_pOpenATCRunStatus->GetGlobalCounter();

	TVehDetBoardData tVehDetBoardData;                  
    m_pOpenATCRunStatus->GetVehDetBoardData(tVehDetBoardData);

	//unsigned int nDetectorIndex = m_chUnPackedBuff[C_N_DETECTOR_INDEX] % C_N_MAXDETINPUT_NUM - 1;
	//unsigned int nDetectorID = m_chUnPackedBuff[C_N_DETECTOR_INDEX];
	//unsigned int nVehDetBoardIndex = (nDetectorID - 1) / C_N_MAXDETINPUT_NUM;  //2022.6.7
	unsigned int nDetectorIndex = m_chUnPackedBuff[C_N_DETECTOR_INDEX] % C_N_MAXDETINPUT_NUM;
	unsigned int nDetectorID = m_chUnPackedBuff[C_N_DETECTOR_INDEX] + 1;
	unsigned int nVehDetBoardIndex = (nDetectorID - 1) / C_N_MAXDETINPUT_NUM;

	bool bFlag = false;

	int  i = 0, j = 0;
	for (i = 0;i < MAX_VEHICLEDETECTOR_COUNT;i++)
	{
		if (atVehDetector[i].m_byVehicleDetectorNumber == 0 || atVehDetector[i].m_byDetectorType != VIDEO_DETECTOR)
		{
			continue;
		}

		for (j = 0;j < MAX_VEHICLEDETECTOR_COUNT;j++)
		{
			if (atVehDetector[i].m_byVehicleDetectorNumber == nDetectorID)
			{
				tVehDetBoardData.m_atVehDetData[nVehDetBoardIndex].m_bDetFaultStatus[nDetectorIndex] = m_chUnPackedBuff[C_N_DETECTOR_VALUE];
		
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "SetDetectorFaultInfo VehDetBoardIndex:%d DetectorIndex:%d DetectorID:%d DetectorValue:%d", nVehDetBoardIndex, nDetectorIndex, m_chUnPackedBuff[C_N_DETECTOR_INDEX], m_chUnPackedBuff[C_N_DETECTOR_VALUE]);

				tVehDetBoardData.m_atVehDetData[nVehDetBoardIndex].m_nVehTimerValCounter = nGlobalCounter;
				m_pOpenATCRunStatus->SetVehDetBoardData(tVehDetBoardData);
				bFlag = true;
				break;
			}
		}

		if (bFlag)
		{
			break;
		}
	}
}

void COpenATCCommWithITS300Thread::SetPreemptInfo()
{
	TPreemptControlStatus tPreemptControlStatus;
	memset(&tPreemptControlStatus, 0, sizeof(tPreemptControlStatus));
	m_pOpenATCRunStatus->GetPreemptControlStatus(tPreemptControlStatus);

	unsigned int  nPreemptIndex = m_chUnPackedBuff[C_N_PREEMPT_INDEX];

    if (tPreemptControlStatus.m_nPreemptControlResult != CONTROL_FAILED)
	{
		TPreempt  tPreempt;
		memset(&tPreempt,0,sizeof(tPreempt));
		if (m_pOpenATCParameter->GetPreemptParam(nPreemptIndex, tPreempt))
		{
			TPreemptCtlCmd  tPreemptCtlCmd;
			memset(&tPreemptCtlCmd,0,sizeof(tPreemptCtlCmd));
			tPreemptCtlCmd.m_bNewCmd = true;
			tPreemptCtlCmd.m_byPreemptType = tPreempt.m_byPreemptType;
			tPreemptCtlCmd.m_byPreemptPhaseID = tPreempt.m_byPreemptPhaseID;
			tPreemptCtlCmd.m_wPreemptDelay = tPreempt.m_wPreemptDelay;
			tPreemptCtlCmd.m_wPreemptDuration = tPreempt.m_wPreemptDuration;
			m_pOpenATCRunStatus->SetPreemptCtlCmd(tPreemptCtlCmd);
		}
		else //参数中不存在该优先编号
		{

		}
	}
	else //面板控制或系统控制时，指令不生效
	{
		
	}
}

char COpenATCCommWithITS300Thread::GetBit(unsigned int nInput, char chNum)    
{    
    char i = 0;
    char chResult = 0; 
    unsigned int nTemp = 0;   
       
    if (chNum > 8 * sizeof(nInput)) 
	{
        return 0; 
	}
       
    nTemp = nInput;    
    for (i = 0;i < chNum;i++)    
    {    
        chResult = nTemp % 2;    
        nTemp = nTemp / 2;    
    }    
       
    return chResult;    
}