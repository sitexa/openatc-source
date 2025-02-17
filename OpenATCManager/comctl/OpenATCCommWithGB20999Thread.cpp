/*====================================================================
模块名 ：和GB20999协议测试软件交互的通信接口
文件名 ：OpenATCCommWithGB20999Thread.cpp
相关文件：OpenATCCommWithGB20999Thread.h
实现功能：和GB20999协议测试软件进行交互
作者 ：李永萍
版权 ：<Copyright(C) 2019-2020 Suzhou Keda Technology Co., Ltd. All rights reserved.>
---------------------------------------------------------------------------------------------------------------------
修改记录：
日 期           版本    修改人             走读人             修改记录
2019/09/06      V1.0    李永萍             李永萍             创建模块
====================================================================*/
#include "OpenATCCommWithGB20999Thread.h"
#include "OpenATCPackUnpackSimpleFactory.h"
#include "../../Include/OpenATCLog.h"
#include "../OpenATCFlowProcManager.h"
#include <algorithm>
#include <time.h>
#ifndef WIN32
#include <unistd.h>
#else
#pragma comment(lib, "Advapi32.lib")
//#include "TimeZoneInf.h"
#endif

COpenATCCommWithGB20999Thread::COpenATCCommWithGB20999Thread()
{
	m_hThread		= 0;
    m_dwThreadRet	= 0;
	m_bExitFlag		= true;
    m_nDetachState  = 0;

	m_pOpenATCParameter = NULL;
	m_pOpenATCRunStatus = NULL;
	m_pOpenATCLog       = NULL;

	m_chRecvBuff  = new unsigned char[RECV_BUFFER_SIZE];
	memset(m_chRecvBuff, 0, RECV_BUFFER_SIZE);
	m_chUnPackedBuff = new unsigned char[UNPACKED_BUFFER_SIZE];
	memset(m_chUnPackedBuff, 0x00, UNPACKED_BUFFER_SIZE);
	m_chSendBuff = new unsigned char[SEND_BUFFER_SIZE];
	memset(m_chSendBuff, 0x00, SEND_BUFFER_SIZE);
	m_chPackedBuff = new unsigned char[PACKED_BUFFER_SIZE];
	memset(m_chPackedBuff, 0x00, PACKED_BUFFER_SIZE);

	m_pDataPackUnpackMode = COpenATCPackUnpackSimpleFactory::Create(PACK_UNPACK_MODE_GB20999);

	m_lastReadOkTime = time(NULL);
	m_nSendTimeOut   = 100;
	m_nRecvTimeOut   = 100;

	m_byFrameID = 0;

	memset(m_tRunFaultInfo, 0, sizeof(m_tRunFaultInfo));
	memset(&m_tDBManagement,0,sizeof(TDBManagement));

	m_commHelper.SetRecvTimeOut(400);
	m_commHelper.SetSendTimeOut(400);

#ifndef _WIN32
	tzset();  
	time_t current_timet;  
	time(&current_timet);//得到当前时间秒数   
	localtime_r(&current_timet, &Alarm_tm);//得到GMT，即UTC时间  
#else
	GetLocalTime(&m_stAlarmTime);    
#endif  

	m_nFaultDataIndex = 0;
	m_nFaultDataCount = 0;
}

COpenATCCommWithGB20999Thread::~COpenATCCommWithGB20999Thread()
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

	if (m_chSendBuff != NULL)
	{
		delete[]m_chSendBuff;
		m_chSendBuff = NULL;
	}

	if (m_chPackedBuff != NULL)
	{
		delete[]m_chPackedBuff;
		m_chPackedBuff = NULL;
	}

	delete m_pDataPackUnpackMode;
	m_pDataPackUnpackMode = NULL;

	m_commHelper.Close();
}

int COpenATCCommWithGB20999Thread::Run()
{
#ifndef _WIN32
	prctl(15,"CommWithGB20999Thread",0,0,0);
#endif
	int				nRecvLength = 0;
    unsigned int	nPackLength = 0;
    int				nRet		= 0;
	int				i			= 0;
	unsigned int	nID			= 0;
	bool			bFlag		= false;

	m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "COpenATCCommWithGB20999Thread start!");
	char szPeerIp[20] = {0};


	while (!m_bExitFlag)
    {
		if (m_commHelper.AcceptConnection("", 8881, false) == OPENATC_RTN_FAILED)
		{
			m_pOpenATCLog->LogOneMessage(LEVEL_INFO, "20999 Connect failed!");
			OpenATCSleep(500);
			continue;
		}
		else
		{
			m_pOpenATCLog->LogOneMessage(LEVEL_INFO, "20999 Connect success!");
			m_pDataPackUnpackMode->ClearBuff();
		}

		 while (true)
		 {
		     nRecvLength = 0;
		     memset(szPeerIp, 0x00, sizeof(szPeerIp));
			 //m_pOpenATCLog->LogOneMessage(LEVEL_INFO, "wait recv!");
			 if (m_commHelper.Read(m_chRecvBuff, RECV_BUFFER_SIZE, nRecvLength, bFlag, szPeerIp) == OPENATC_RTN_OK)
			 {
				 if (nRecvLength > 0)
				 {
					 m_lastReadOkTime = time(NULL);
					 m_pDataPackUnpackMode->Write((unsigned char *)(char *)m_chRecvBuff, nRecvLength);
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
					printf("Recv Success!!!\n");
					//m_pOpenATCLog->LogOneMessage(LEVEL_INFO, COpenATCCommWithGB20999Thread ParserPack length is %d!", nPackLength);
					ParserPack(szPeerIp);
				}
				else if (nRet != ReadNoData)
				{
					//m_pOpenATCLog->LogOneMessage(LEVEL_ERROR, "COpenATCCommWithGB20999Thread unpacker read err!");
				}
			}

			long lCurTime = time(NULL);
			if (labs(lCurTime -  m_lastReadOkTime) > HEART_INTERVAL_TIME)
			{
				//break;
			}
		 }
    }

	m_pOpenATCLog->LogOneMessage(LEVEL_INFO, "COpenATCCommWithGB20999Thread exit!");
    return OPENATC_RTN_OK;
}

void  COpenATCCommWithGB20999Thread::Init(COpenATCParameter *pParameter, COpenATCRunStatus *pRunStatus, COpenATCLog * pOpenATCLog)
{
	m_pOpenATCParameter = pParameter;
	m_pOpenATCRunStatus = pRunStatus;
	m_pOpenATCLog       = pOpenATCLog;

	//m_pOpenATCParameter->GetAscParamInfo(m_tAscParamInfo);				//HPH 2021.12.07（查询）
	//m_pOpenATCParameter->GetAscTempParamInfo(m_tVerifyParamInfo);		//HPH 2022.06.27(事务设置缓存)

	m_bIsNeedSave = false;

	m_bPhaseControlChange = false;

	memset(&m_nPipeInfo,0,sizeof(m_nPipeInfo));

	//2022.06.27事务部分重构 --HPH
	m_tDBManagement.m_byDBCreateTransaction = NORMAL;
	m_tDBManagement.m_byDBVerifyStatus = NOTDONE;
	//2022.06.27事务部分重构 --HPH

	#ifdef _WIN32
	TIME_ZONE_INFORMATION   tzi; 
	GetSystemTime(&tzi.StandardDate);  
	GetTimeZoneInformation(&tzi);  
	int nTimeZone = tzi.Bias / -60 * 3600; //时区，如果是中国标准时间则得到8 
	m_tAscParamInfo.m_stBaseInfo.m_wTimeZone = nTimeZone;
	//m_tAscParamInfo.m_stBaseInfo.m_stTimeZone.m_wTimeZone = nTimeZone;
	//m_dTimeZone = nTimeZone;
	#else
	char *chTimeZone;
    if((chTimeZone = getenv("TZ")))
	{
         m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "COpenATCCommWithGB20999Thread ATC_TIMEZONE %s!", chTimeZone);
		 if (strcmp(chTimeZone, "GMT-1") == 0)
		 {
			 m_tAscParamInfo.m_stBaseInfo.m_wTimeZone = 1;
		 }
		 else if (strcmp(chTimeZone, "GMT+1") == 0)
		 {
			 m_tAscParamInfo.m_stBaseInfo.m_wTimeZone = -1;
		 }
		 else if (strcmp(chTimeZone, "GMT-2") == 0)
		 {
			 m_tAscParamInfo.m_stBaseInfo.m_wTimeZone = 2;
		 }
		 else if (strcmp(chTimeZone, "GMT+2") == 0)
		 {
			 m_tAscParamInfo.m_stBaseInfo.m_wTimeZone = -2;
		 }
		 else if (strcmp(chTimeZone, "GMT-3") == 0)
		 {
			 m_tAscParamInfo.m_stBaseInfo.m_wTimeZone = 3;
		 }
		 else if (strcmp(chTimeZone, "GMT+3") == 0)
		 {
			 m_tAscParamInfo.m_stBaseInfo.m_wTimeZone = -3;
		 }
		 else if (strcmp(chTimeZone, "GMT-4") == 0)
		 {
			 m_tAscParamInfo.m_stBaseInfo.m_wTimeZone = 4;
		 }
		 else if (strcmp(chTimeZone, "GMT+4") == 0)
		 {
			 m_tAscParamInfo.m_stBaseInfo.m_wTimeZone = -4;
		 }
		 else if (strcmp(chTimeZone, "GMT-5") == 0)
		 {
			 m_tAscParamInfo.m_stBaseInfo.m_wTimeZone = 5;
		 }
		 else if (strcmp(chTimeZone, "GMT+5") == 0)
		 {
			 m_tAscParamInfo.m_stBaseInfo.m_wTimeZone = -5;
		 }
		 else if (strcmp(chTimeZone, "GMT-6") == 0)
		 {
			 m_tAscParamInfo.m_stBaseInfo.m_wTimeZone = 6;
		 }
		 else if (strcmp(chTimeZone, "GMT+6") == 0)
		 {
			 m_tAscParamInfo.m_stBaseInfo.m_wTimeZone = -6;
		 }
		 else if (strcmp(chTimeZone, "GMT-7") == 0)
		 {
			 m_tAscParamInfo.m_stBaseInfo.m_wTimeZone = 7;
		 }
		 else if (strcmp(chTimeZone, "GMT+7") == 0)
		 {
			 m_tAscParamInfo.m_stBaseInfo.m_wTimeZone = -7;
		 }
		 else if (strcmp(chTimeZone, "GMT-8") == 0)
		 {
			 m_tAscParamInfo.m_stBaseInfo.m_wTimeZone = 8;
		 }
		 else if (strcmp(chTimeZone, "GMT+8") == 0)
		 {
			 m_tAscParamInfo.m_stBaseInfo.m_wTimeZone = -8;
		 }
		 else if (strcmp(chTimeZone, "GMT-9") == 0)
		 {
			 m_tAscParamInfo.m_stBaseInfo.m_wTimeZone = 9;
		 }
		 else if (strcmp(chTimeZone, "GMT+9") == 0)
		 {
			 m_tAscParamInfo.m_stBaseInfo.m_wTimeZone = -9;
		 }
		 else if (strcmp(chTimeZone, "GMT-10") == 0)
		 {
			 m_tAscParamInfo.m_stBaseInfo.m_wTimeZone = 10;
		 }
		 else if (strcmp(chTimeZone, "GMT+10") == 0)
		 {
			 m_tAscParamInfo.m_stBaseInfo.m_wTimeZone = -10;
		 }
		 else if (strcmp(chTimeZone, "GMT-11") == 0)
		 {
			 m_tAscParamInfo.m_stBaseInfo.m_wTimeZone = 11;
		 }
		 else if (strcmp(chTimeZone, "GMT+11") == 0)
		 {
			 m_tAscParamInfo.m_stBaseInfo.m_wTimeZone = -11;
		 }
		 else if (strcmp(chTimeZone, "GMT-12") == 0)
		 {
			 m_tAscParamInfo.m_stBaseInfo.m_wTimeZone = 12;
		 }
		 else if (strcmp(chTimeZone, "GMT+12") == 0)
		 {
			 m_tAscParamInfo.m_stBaseInfo.m_wTimeZone = -12;
	     }
	 }
	 m_tAscParamInfo.m_stBaseInfo.m_wTimeZone *= 3600;			//是秒数
     #endif
     
	 //m_pOpenATCParameter->SetAscTempParamInfo(m_tAscParamInfo);//设置参数信息
	 //m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "20999 set at once!");
	 //m_pOpenATCParameter->SaveSetParameter(true);
	 m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "create COpenATCCommWithGB20999Thread");
	//创建状态机数据处理线程
	#ifdef _WIN32
		m_hThread = CreateThread(NULL, 0, (unsigned long(GB20999_CALLBACK *)(void *))&DBDataProessThread, this, 0, NULL);
		if (NULL == m_hThread)
			m_pOpenATCLog->LogOneMessage(LEVEL_ERROR, "create COpenATCCommWithGB20999Thread failed!!");
	#else
		if (0 != pthread_create(&m_hThread, NULL, DBDataProessThread, this))
			m_pOpenATCLog->LogOneMessage(LEVEL_ERROR, "create COpenATCCommWithGB20999Thread failed!!");
	#endif
}

int COpenATCCommWithGB20999Thread::Start()
{
    if (false == m_bExitFlag)
	{
        return OPENATC_RTN_OK;
	}

    bool bOK = true;
    m_bExitFlag = false;

#ifdef _WIN32
	m_hThread = CreateThread(NULL, 0, (unsigned long(COMMWITHGB20999_CALLBACK *)(void *))&RunThread, this, 0, NULL);
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

int COpenATCCommWithGB20999Thread::Join()
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
		m_pOpenATCLog->LogOneMessage(LEVEL_INFO, "COpenATCCommWithGB20999Thread Join ok!");
	}
	else
	{
		m_pOpenATCLog->LogOneMessage(LEVEL_INFO, "COpenATCCommWithGB20999Thread Join fail!");
	}

	m_bExitFlag = true;
    m_hThread	= 0;
    return nRet;
}

int COpenATCCommWithGB20999Thread::Detach()
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

void* COMMWITHGB20999_CALLBACK COpenATCCommWithGB20999Thread::RunThread(void *pParam)
{
    COpenATCCommWithGB20999Thread *pThis = (COpenATCCommWithGB20999Thread *)pParam;

    int nRet = pThis->Run();
    return (void *)nRet;
}

void COpenATCCommWithGB20999Thread::OpenATCSleep(long nMsec)
{
#ifdef _WIN32
    Sleep(nMsec);
#else
    usleep(nMsec*1000);
#endif
}

unsigned short COpenATCCommWithGB20999Thread::Crc16(const unsigned char *buffer, int buffer_length)
{
	short usRc = 0;
	unsigned char i = 0;
	short usCrc16 = 0x0000;
	while (buffer_length--)
	{
		for (i = 0x80; i != 0; i >>= 1)
		{
			if ((usCrc16 & 0x8000) != 0)
			{
				usCrc16 = usCrc16 << 1;
				usCrc16 = usCrc16 ^ 0x1005;//x16+x12+x2+1
			}
			else
			{
				usCrc16 = usCrc16 << 1;
			}
			if ((*buffer & i) != 0)
			{
				usCrc16 = usCrc16 ^ 0x1005;//x16+x12+x2+1
			}
		}
		buffer++;
	}
	return usCrc16;
}

void COpenATCCommWithGB20999Thread::ParserPack(char* chPeerIp)
{
    int nRet = OPENATC_RTN_OK;
	int nPackSize = 0;
	//m_pOpenATCParameter->GetAscParamInfo(m_tAscParamInfo);	//信号机参数每次都获取更新一次
	m_bIsNeedSave = false;

    switch (m_chUnPackedBuff[C_N_FRAME_TYPE_POS])//帧类型
    {
	case FRAME_TYPE_QUERY:
		{
			ParserPack_QueryLink();
		}
		break;
	case FRAME_TYPE_SET:
		{
			ParserPack_SetLink(chPeerIp);
			//对信号机的非事务机制直接在m_tAscParamInfo中设置
			//对信号机的非事务机制参数直接将m_tAscParamInfo写入json（包含设备相关的）
			//对信号机事务机制参数进行存入m_tVerifyParamInfo缓存中，并通过事务线程对其进行校验
		}
		break;
	case FRAME_TYPE_BROADCAST:
		{
			
		}
		break;
	case FRAME_TYPE_HEART_QUERY:
		{
			nPackSize = AckCtl_AskHeart();
			if (nPackSize > 0)
			{
				SendAckToPeer(nPackSize, nRet);
			}
		}
		break;
	default:
		{
		    break;
		}
    }
}

void COpenATCCommWithGB20999Thread::ParserPack_QueryLink()
{
	BYTE byDataValueCount = m_chUnPackedBuff[C_N_DATAVALUE_COUNT_POS];
	BYTE byDataClassID = m_chUnPackedBuff[C_N_DATACLASS_ID_POS];

	TReturnData tCorrectReturnData;
    memset(&tCorrectReturnData, 0, sizeof(tCorrectReturnData));

	TReturnData tWrongReturnData;
    memset(&tWrongReturnData, 0, sizeof(tWrongReturnData));

	//每次查询都要获取参数表最新的参数
	//m_pOpenATCParameter->GetAscParamInfo(m_tAscParamInfo);
	m_pOpenATCParameter->GetAscParamByRunParam(m_tAscParamInfo);

	for (BYTE byIndex = 0;byIndex < byDataValueCount;byIndex++)
	{
		byDataClassID = m_chUnPackedBuff[C_N_DATACLASS_ID_POS + byIndex * 6];
	    
		switch (byDataClassID)//数据类型
		{
		case DEVICE_INFO:
			{
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "COpenATCCommWithGB20999Thread Receive Query Device Info!");

				AckCtl_AskDeviceInfo(byIndex, tCorrectReturnData, tWrongReturnData);
			}
			break; 
		case BASE_INFO:
			{
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "COpenATCCommWithGB20999Thread Receive Query Base Info!");

				AckCtl_AskBaseInfo(byIndex, tCorrectReturnData, tWrongReturnData);
			}
			break; 
		case LIGHTGROUP_INFO:
			{
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "COpenATCCommWithGB20999Thread Receive Query LightGroup Info!");

				AckCtl_AskLightGroupInfo(byIndex, tCorrectReturnData, tWrongReturnData);
			}
			break; 
		case PHASE_INFO:
			{
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "COpenATCCommWithGB20999Thread Receive Query Phase Info!");

				AckCtl_AskPhaseInfo(byIndex, tCorrectReturnData, tWrongReturnData);
			}
			break; 
		case DETECTOR_INFO:
			{
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "COpenATCCommWithGB20999Thread Receive Query Detector Info!");

				AckCtl_AskDetectorInfo(byIndex, tCorrectReturnData, tWrongReturnData);
			}
			break; 
		 case PHASESTAGE_INFO:
			{
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "COpenATCCommWithGB20999Thread Receive Query Phase Stage Info!");

				AckCtl_AskPhaseStageInfo(byIndex, tCorrectReturnData, tWrongReturnData);
			}
			break; 
		case PHASESAFETY_INFO:
			{
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "COpenATCCommWithGB20999Thread Receive Query Phase Safety Info!");

				AckCtl_AskPhaseSafetyInfo(byIndex, tCorrectReturnData, tWrongReturnData);
			}
			break; 
		case EMERGENCY_PRIORITY:
			{
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "COpenATCCommWithGB20999Thread Receive Query Emerygency Priority Info!");

				AckCtl_AskEmergencyPriorityInfo(byIndex, tCorrectReturnData, tWrongReturnData);
			}
			break; 
		case PATTERN_INFO:
			{
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "COpenATCCommWithGB20999Thread Receive Query Pattern Info!");

				AckCtl_AskPatternInfo(byIndex, tCorrectReturnData, tWrongReturnData);
			}
			break;
		case TRANSITION_RETRAIN:
			{
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "COpenATCCommWithGB20999Thread Receive Query Transition Retain Info!");

				AckCtl_AskTransitionRetain(byIndex, tCorrectReturnData, tWrongReturnData);
			}
			break;
		case DAY_PLAN:
			{
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "COpenATCCommWithGB20999Thread Receive Query Day Plan Info!");

				AckCtl_AskDayPlanInfo(byIndex, tCorrectReturnData, tWrongReturnData);
			}
			break;
		case SCHEDULE_TABLE:
			{
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "COpenATCCommWithGB20999Thread Receive Query Schedule Table Info!");

				AckCtl_AskScheduleTable(byIndex, tCorrectReturnData, tWrongReturnData);
			}
			break;
		case RUN_STATUS:
			{
				if (m_chUnPackedBuff[C_N_OBJECT_ID_POS + byIndex * 6] == DEVICE_STATUS)
				{
					m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "COpenATCCommWithGB20999Thread Receive Query Device Status Info!");

					AckCtl_AskDeviceStatus(byIndex, tCorrectReturnData, tWrongReturnData);
				}
				else if (m_chUnPackedBuff[C_N_OBJECT_ID_POS + byIndex * 6] == CONTROL_STATUS)
				{
					m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "COpenATCCommWithGB20999Thread Receive Query Control Status Info!");

					AckCtl_AskRunStatus(byIndex, tCorrectReturnData, tWrongReturnData);
				}
			}
			break;
		case TRAFFIC_DATA:
			{
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "COpenATCCommWithGB20999Thread Receive Traffic Data!");

				AckCtl_AskTrafficData(byIndex, tCorrectReturnData, tWrongReturnData);
			}
			break;
		case ALARM_DATA:
			{
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "COpenATCCommWithGB20999Thread Receive Alarm Data!");

				AckCtl_AskAlarmData(byIndex, tCorrectReturnData, tWrongReturnData);
			}
			break;
		case FAULT_DATA:
			{
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "COpenATCCommWithGB20999Thread Receive Fault Data!");

				AckCtl_AskFaultData(byIndex, tCorrectReturnData, tWrongReturnData);
			}
			break;
		case CENTER_CONTROL:
			{
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "COpenATCCommWithGB20999Thread Receive Center Control!");

				AckCtl_AskCenterControl(byIndex, tCorrectReturnData, tWrongReturnData);
			}
			break;
		case ORDER_PIPE:
			{
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "COpenATCCommWithGB20999Thread Receive Order Pipe!");

				AckCtl_AskOrderPipe(byIndex, tCorrectReturnData, tWrongReturnData);
			}
			break;
		//case PRIVATE_DATE:
		//	{
		//		m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "COpenATCCommWithGB20999Thread Receive Private Data!");

		//		AckCtl_AskPrivateData(byIndex, tCorrectReturnData, tWrongReturnData);
		//	}
		//	break;
		default:
			{
				break;
			}
		}
	}

	if (tCorrectReturnData.m_byReturnCount > 0)
	{
		AckCtl_ReturnData(true, tCorrectReturnData);
	}

	if (tWrongReturnData.m_byReturnCount > 0)
	{
		AckCtl_ReturnData(false, tWrongReturnData);
	}
}

void COpenATCCommWithGB20999Thread::ParserPack_SetLink(char* chPeerIp)
{
	TReturnData tCorrectReturnData;
    memset(&tCorrectReturnData, 0, sizeof(tCorrectReturnData));

	TReturnData tWrongReturnData;
    memset(&tWrongReturnData, 0, sizeof(tWrongReturnData));

	bool bSaveSuccess = false;

	if (m_tDBManagement.m_byDBCreateTransaction == NORMAL)
	{
		m_pOpenATCParameter->GetAscParamInfo(m_tParamInfo);	//信号机参数每次都获取更新一次

		m_pOpenATCParameter->GetAscParamByRunParam(m_tAscParamInfo);
	}

	//m_pOpenATCParameter->GetAscParamInfo(m_tParamInfo);	//信号机参数每次都获取更新一次

	//m_pOpenATCParameter->GetAscParamByRunParam(m_tAscParamInfo);

	//m_pOpenATCParameter->GetAscParamByRunParam(m_tVerifyParamInfo);

	//m_pOpenATCParameter->GetAscParamInfo(m_tVerifyParamInfo);	//测试那需要,可以单个配置。【因为，它下发是全表下发，所以也无关紧要，是否需要】 【事务机制时使用，所以移动到管道下发传输命令时】

	BYTE byDataValueCount = m_chUnPackedBuff[C_N_DATAVALUE_COUNT_POS];
	BYTE byDataValueIndex = m_chUnPackedBuff[C_N_DATAVALUE_INDEX_POS];
	BYTE byDataValueLength = m_chUnPackedBuff[C_N_DATAVALUE_LENGTH_POS];
	BYTE byDataValueLengthPos = C_N_DATAVALUE_LENGTH_POS;
	BYTE byDataValuePos = C_N_DATAVALUE_POS;

	BYTE byDataClassID = m_chUnPackedBuff[C_N_DATACLASS_ID_POS];
	BYTE byObjectID = m_chUnPackedBuff[C_N_OBJECT_ID_POS];
	BYTE byAttributeID = m_chUnPackedBuff[C_N_ATTRIBUTE_ID_POS];
	BYTE byElementID = m_chUnPackedBuff[C_N_ELEMENT_ID_POS];

	BYTE byStartPos = C_N_DATAVALUE_COUNT_POS + 2;

	for (BYTE byIndex = 0;byIndex < byDataValueCount;byIndex++)
	{
		byDataValueLength = m_chUnPackedBuff[byStartPos++];
		byDataClassID = m_chUnPackedBuff[byStartPos++];
		byObjectID = m_chUnPackedBuff[byStartPos++];
		byAttributeID = m_chUnPackedBuff[byStartPos++];
		byElementID = m_chUnPackedBuff[byStartPos++];
		byDataValuePos = byStartPos;
		byStartPos = byStartPos + byDataValueLength - 3;

		switch (byDataClassID)//数据类型
		{
		case DEVICE_INFO:
			{
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "COpenATCCommWithGB20999Thread Receive Set DeviceInfo!");

				SetDeviceInfo(byIndex, byObjectID, byAttributeID, byElementID, byDataValuePos, tCorrectReturnData, tWrongReturnData, byDataValueLength);
			}
			break; 
		case BASE_INFO:
			{
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "COpenATCCommWithGB20999Thread Receive Set BaseInfo!");

				SetBaseInfo(byIndex, byObjectID, byAttributeID, byElementID, byDataValuePos, tCorrectReturnData, tWrongReturnData, byDataValueLength);
			}
			break; 
		case LIGHTGROUP_INFO:
			{
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "COpenATCCommWithGB20999Thread Receive Set LightGroupInfo!");

				SetLightGroupInfo(byIndex, byObjectID, byAttributeID, byElementID, byDataValuePos, tCorrectReturnData, tWrongReturnData, byDataValueLength);
			}
			break; 
		case PHASE_INFO:
			{
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "COpenATCCommWithGB20999Thread Receive Set PhaseInfo!");

				SetPhaseInfo(byIndex, byObjectID, byAttributeID, byElementID, byDataValuePos, tCorrectReturnData, tWrongReturnData, byDataValueLength);
			}
			break; 
		case DETECTOR_INFO:
			{
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "COpenATCCommWithGB20999Thread Receive Set DetectorInfo!");

				SetDetectorInfo(byIndex, byObjectID, byAttributeID, byElementID, byDataValuePos, tCorrectReturnData, tWrongReturnData, byDataValueLength);
			}
			break; 
		 case PHASESTAGE_INFO:
			{
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "COpenATCCommWithGB20999Thread Receive Set PhaseStageInfo!");

				SetPhaseStageInfo(byIndex, byObjectID, byAttributeID, byElementID, byDataValuePos, tCorrectReturnData, tWrongReturnData, byDataValueLength);
			}
			break; 
		case PHASESAFETY_INFO:
			{
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "COpenATCCommWithGB20999Thread Receive Set PhaseSafetyInfo!");

				SetPhaseSafetyInfo(byIndex, byObjectID, byAttributeID, byElementID, byDataValuePos, tCorrectReturnData, tWrongReturnData, byDataValueLength);
			}
			break; 
		case EMERGENCY_PRIORITY:
			{
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "COpenATCCommWithGB20999Thread Receive Set EmerygencyPriorityInfo!");

				SetEmergencyAndPriorityInfo(byIndex, byObjectID, byAttributeID, byElementID, byDataValuePos, tCorrectReturnData, tWrongReturnData, byDataValueLength);
			}
			break; 
		case PATTERN_INFO:
			{
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "COpenATCCommWithGB20999Thread Receive Set PatternInfo!");

				SetPatternInfo(byIndex, byObjectID, byAttributeID, byElementID, byDataValuePos, tCorrectReturnData, tWrongReturnData, byDataValueLength);
			}
			break;
		case TRANSITION_RETRAIN:
			{
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "COpenATCCommWithGB20999Thread Receive Set TransitionRetainInfo!");

				SetTransitionRetain(byIndex, byObjectID, byAttributeID, byElementID, byDataValuePos, tCorrectReturnData, tWrongReturnData, byDataValueLength);
			}
			break;
		case DAY_PLAN:
			{
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "COpenATCCommWithGB20999Thread Receive Set DayPlanInfo!");

				SetDayPlanInfo(byIndex, byObjectID, byAttributeID, byElementID, byDataValuePos, tCorrectReturnData, tWrongReturnData, byDataValueLength);
			}
			break;
		case SCHEDULE_TABLE:
			{
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "COpenATCCommWithGB20999Thread Receive Set ScheduleTable Info!");

				SetScheduleTable(byIndex, byObjectID, byAttributeID, byElementID, byDataValuePos, tCorrectReturnData, tWrongReturnData, byDataValueLength);
			}
			break;
		case RUN_STATUS:
			{
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "COpenATCCommWithGB20999Thread Receive Set RunStatusInfo!");

				SetRunStatusInfo(byIndex, byObjectID, byAttributeID, byElementID, byDataValuePos, tCorrectReturnData, tWrongReturnData, byDataValueLength);
			}
			break;
		case TRAFFIC_DATA:
			{
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "COpenATCCommWithGB20999Thread Receive Set TrafficData!");

				CreateSetReturnData(BAD_VALUE_READONLY, byIndex, TRAFFIC_DATA, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
			}
			break;
		case ALARM_DATA:
			{
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "COpenATCCommWithGB20999Thread Receive Set AlarmData!");

				CreateSetReturnData(BAD_VALUE_READONLY, byIndex, ALARM_DATA, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
			}
			break;
		case FAULT_DATA:
			{
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "COpenATCCommWithGB20999Thread Receive Set FaultData!");

				CreateSetReturnData(BAD_VALUE_READONLY, byIndex, FAULT_DATA, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
			}
			break;
		case CENTER_CONTROL:
			{
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "COpenATCCommWithGB20999Thread Receive Set CenterControl!");

				SetCenterControl(byIndex, byObjectID, byAttributeID, byElementID, byDataValuePos, tCorrectReturnData, tWrongReturnData, byDataValueLength);
			}
			break;
		case ORDER_PIPE:
			{
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "COpenATCCommWithGB20999Thread Receive Order Pipe!");

				SetOrderPipe(byIndex, byObjectID, byAttributeID, byElementID, byDataValuePos, tCorrectReturnData, tWrongReturnData, byDataValueLength);
			}
			break;
		//私有数据类
		//case PRIVATE_DATE:
		//	{
		//		m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "COpenATCCommWithGB20999Thread Receive Private Data!");

		//		SetPrivateData(byIndex, byObjectID, byAttributeID, byElementID, byDataValuePos, tCorrectReturnData, tWrongReturnData, byDataValueLength);
		//	}
		//	break;
		default:
			{
				break;
			}
		}
	}

	if (m_bIsNeedSave)
	{
		m_pOpenATCParameter->Gb20999ToGb25280(m_tParamInfo, m_tAscParamInfo);
		m_pOpenATCParameter->SetAscTempParamInfo(m_tParamInfo);//设置参数信息
		m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "20999 set at once!");
		bSaveSuccess = m_pOpenATCParameter->SaveGB20999ASCParam(m_bIsNeedSave);

		if (bSaveSuccess)
		{
			//判断是否为相位控制
			if (m_bPhaseControlChange)
			{
				m_pOpenATCRunStatus->SetPhaseControlChange(true);
				m_bPhaseControlChange = false;
			}

			//此处先不对新参数进行初始化
			TParamRunStatus tParamRunStatus;
			m_pOpenATCRunStatus->GetParamRunStatus(tParamRunStatus);
			tParamRunStatus.m_chIsParameterChg = C_CH_ISPARAMETERCHG_OK;
			if (tParamRunStatus.m_chParameterReady != C_CH_PARAMERREADY_OK)
			{
				m_pOpenATCParameter->Init(m_pOpenATCRunStatus, m_pOpenATCLog);
			}
			m_pOpenATCRunStatus->SetParamRunStatus(tParamRunStatus);
		}

		if (bSaveSuccess)
		{
			//写操作记录日志
			TAscOperationRecord tAscOperationRecord;//操作记录
			memset(&tAscOperationRecord, 0, sizeof(tAscOperationRecord));

			tAscOperationRecord.m_unStartTime = time(NULL);
			tAscOperationRecord.m_unEndTime = time(NULL);
			tAscOperationRecord.m_bySubject = 1;
			tAscOperationRecord.m_byObject = 1;
			tAscOperationRecord.m_nInfoType = SYSTEM_DOWNLOAD_TZPARAM;
			tAscOperationRecord.m_bStatus = true;

			sprintf((char*)tAscOperationRecord.m_byFailureValue, "IP is %s! SaveParameter success", chPeerIp);

			m_tOpenATCOperationRecord.SaveOneOperationRecordMessage(&tAscOperationRecord, m_pOpenATCRunStatus);

		}
		else
		{
			//写操作记录日志
			TAscOperationRecord tAscOperationRecord;//操作记录
			memset(&tAscOperationRecord, 0, sizeof(tAscOperationRecord));

			tAscOperationRecord.m_unStartTime = time(NULL);
			tAscOperationRecord.m_unEndTime = time(NULL);
			tAscOperationRecord.m_bySubject = 1;
			tAscOperationRecord.m_byObject = 1;
			tAscOperationRecord.m_nInfoType = SYSTEM_DOWNLOAD_TZPARAM;
			tAscOperationRecord.m_bStatus = false;

			sprintf((char*)tAscOperationRecord.m_byFailureValue, "IP is %s! SaveParameter fail", chPeerIp);

			m_tOpenATCOperationRecord.SaveOneOperationRecordMessage(&tAscOperationRecord, m_pOpenATCRunStatus);
		}
		//m_pOpenATCParameter->SaveSetParameter(m_bIsNeedSave);
	}

	if (tCorrectReturnData.m_byReturnCount > 0)
	{
		AckCtl_SetParamInfo(true, tCorrectReturnData);
	}
	if (tWrongReturnData.m_byReturnCount > 0)
	{
		AckCtl_SetParamInfo(false, tWrongReturnData);
	}
}

int COpenATCCommWithGB20999Thread::SendAckToPeer(int nPackSize, int & nRet)
{
	unsigned int dwSize = 0;
    m_pDataPackUnpackMode->PackBuffer(m_chSendBuff, nPackSize, m_chPackedBuff, dwSize);

    if (dwSize == 0)
    {
        return OPENATC_RTN_FAILED;
    }

    return m_commHelper.Write(m_chPackedBuff, dwSize);
}

void COpenATCCommWithGB20999Thread::QueryLightGroupCount(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData)
{
	TDataConfig tDataConfig;
	tDataConfig.m_byIndex = byIndex + 1;
	tDataConfig.m_byDataClassID = m_chUnPackedBuff[C_N_DATACLASS_ID_POS + byIndex * 6];
	tDataConfig.m_byObjectID = m_chUnPackedBuff[C_N_OBJECT_ID_POS + byIndex * 6];
	tDataConfig.m_byAttributeID = m_chUnPackedBuff[C_N_ATTRIBUTE_ID_POS + byIndex * 6];
	tDataConfig.m_byElementID = m_chUnPackedBuff[C_N_ELEMENT_ID_POS + byIndex * 6];
	tDataConfig.m_byDataValueLength = 1;
	tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;

	//printf("m_stChannelTableValidSize:%d\n",m_tAscParamInfo.m_stChannelTableValidSize);
	tDataConfig.m_byDataValue[0] = m_tAscParamInfo.m_stChannelTableValidSize;
	memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
	tCorrectReturnData.m_byReturnCount += 1;
}

void COpenATCCommWithGB20999Thread::QueryAllElementLightGroupConfig(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData)
{
	BYTE byLightGroupType;
	for (int i = 0;i < m_tAscParamInfo.m_stChannelTableValidSize;i++)
	{
		byLightGroupType = m_tAscParamInfo.m_stAscChannelTable[i].m_byLightControlType;

		TDataConfig tDataConfig;
		//tDataConfig.m_byIndex = byIndex + 1;
		tDataConfig.m_byIndex = i + 1;	//HPH 2021.12.07
		tDataConfig.m_byDataClassID = m_chUnPackedBuff[C_N_DATACLASS_ID_POS + byIndex * 6];
		tDataConfig.m_byObjectID = m_chUnPackedBuff[C_N_OBJECT_ID_POS + byIndex * 6];
		tDataConfig.m_byAttributeID = m_chUnPackedBuff[C_N_ATTRIBUTE_ID_POS + byIndex * 6];
		tDataConfig.m_byElementID = i + 1;
		tDataConfig.m_byDataValueLength = 1;
		tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;

		if (tDataConfig.m_byAttributeID == LIGHT_GROUP_TYPE_ID || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValue[0] = tDataConfig.m_byElementID;	
		    memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
		}
		else if (tDataConfig.m_byAttributeID == LIGHT_GROUP_TYPE || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValue[0] = byLightGroupType;
		    memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
		}
	}
}

void  COpenATCCommWithGB20999Thread::QueryAllElementLightGroupStatus(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData)
{
	TLampClrStatus tLampClrStatus;
	memset(&tLampClrStatus, 0, sizeof(TLampClrStatus)); 
	m_pOpenATCRunStatus->GetLampClrStatus(tLampClrStatus);

	//test
	for (int j = 0;j < m_tAscParamInfo.m_stChannelTableValidSize;j++)
	{
		m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "LightGroup:%d, Red statues:%d", j,tLampClrStatus.m_achLampClr[j * 3] );
		m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "LightGroup:%d, Yellow statues:%d", j,tLampClrStatus.m_achLampClr[j * 3 + 1] );
		m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "LightGroup:%d, Green statues:%d", j,tLampClrStatus.m_achLampClr[j * 3 + 2] );
	}

	//

	BYTE byLightGroupStatus = LIGHT_STATUS_RED;
	for (int i = 0;i < m_tAscParamInfo.m_stChannelTableValidSize;i++)
	{
		if (tLampClrStatus.m_achLampClr[(m_tAscParamInfo.m_stAscChannelTable[i].m_byChannelNumber - 1) * 3] == LAMP_CLR_ON)
		{
			byLightGroupStatus = LIGHT_STATUS_RED;
		}
		else if (tLampClrStatus.m_achLampClr[(m_tAscParamInfo.m_stAscChannelTable[i].m_byChannelNumber - 1) * 3 + 1] == LAMP_CLR_ON)
		{
			byLightGroupStatus = LIGHT_STATUS_YELLOW;
		}
		else if (tLampClrStatus.m_achLampClr[(m_tAscParamInfo.m_stAscChannelTable[i].m_byChannelNumber - 1) * 3 + 1] == LAMP_CLR_FLASH)
		{
			byLightGroupStatus = LIGHT_STATUS_YELLOWFLASH;
		}
		/*else if (tLampClrStatus.m_achLampClr[(m_tAscParamInfo.m_stAscChannelTable[i].m_byChannelNumber - 1) * 3 + 2] == LIGHT_STATUS_GREEN)*/
		else if (tLampClrStatus.m_achLampClr[(m_tAscParamInfo.m_stAscChannelTable[i].m_byChannelNumber - 1) * 3 + 2] == LAMP_CLR_ON)
		{
			byLightGroupStatus = LIGHT_STATUS_GREEN;
		}
		else if (tLampClrStatus.m_achLampClr[(m_tAscParamInfo.m_stAscChannelTable[i].m_byChannelNumber - 1) * 3 + 2] == LAMP_CLR_FLASH)
		{
			byLightGroupStatus = LIGHT_STATUS_GREENFLASH;
		}
		else if (tLampClrStatus.m_achLampClr[(m_tAscParamInfo.m_stAscChannelTable[i].m_byChannelNumber - 1) * 3] == LAMP_CLR_OFF &&
				 tLampClrStatus.m_achLampClr[(m_tAscParamInfo.m_stAscChannelTable[i].m_byChannelNumber - 1) * 3 + 1] == LAMP_CLR_OFF &&
				 tLampClrStatus.m_achLampClr[(m_tAscParamInfo.m_stAscChannelTable[i].m_byChannelNumber - 1) * 3 + 2] == LAMP_CLR_OFF)
		{
			byLightGroupStatus = LIGHT_STATUS_OFF;
		}

		TDataConfig tDataConfig;
		//tDataConfig.m_byIndex = byIndex + 1;
		tDataConfig.m_byIndex = i + 1;	//HPH 2021.12.07
		tDataConfig.m_byDataClassID = m_chUnPackedBuff[C_N_DATACLASS_ID_POS + byIndex * 6];
		tDataConfig.m_byObjectID = m_chUnPackedBuff[C_N_OBJECT_ID_POS + byIndex * 6];
		tDataConfig.m_byAttributeID = m_chUnPackedBuff[C_N_ATTRIBUTE_ID_POS + byIndex * 6];
		tDataConfig.m_byElementID = i + 1;
		tDataConfig.m_byDataValueLength = 1;
		tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;

		if (tDataConfig.m_byAttributeID == LIGHT_GROUP_STATUS_ID || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValue[0] = tDataConfig.m_byElementID;		
		    memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == LIGHT_GROUP_STATUS || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValue[0] = byLightGroupStatus;		
		    memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
		}
	}
}

void COpenATCCommWithGB20999Thread::QueryAllElementLightGroupControl(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData)
{
	for (int i = 0;i < m_tAscParamInfo.m_stChannelTableValidSize;i++)
	{
		TDataConfig tDataConfig;
		//tDataConfig.m_byIndex = byIndex + 1;
		tDataConfig.m_byIndex = i + 1;	//HPH 2021.12.07
		tDataConfig.m_byDataClassID = m_chUnPackedBuff[C_N_DATACLASS_ID_POS + byIndex * 6];
		tDataConfig.m_byObjectID = m_chUnPackedBuff[C_N_OBJECT_ID_POS + byIndex * 6];
		tDataConfig.m_byAttributeID = m_chUnPackedBuff[C_N_ATTRIBUTE_ID_POS + byIndex * 6];
		tDataConfig.m_byElementID = i + 1;
		tDataConfig.m_byDataValueLength = 1;
		tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;

		if (tDataConfig.m_byAttributeID == LIGHT_GROUP_TYPE_ID || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValue[0] = tDataConfig.m_byElementID;
		    memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == CONTROL_SHIELD || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValue[0] = m_tAscParamInfo.m_stAscChannelTable[i].m_byScreenFlag;
		    memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == CONTROL_PROHIBIT || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValue[0] = m_tAscParamInfo.m_stAscChannelTable[i].m_byForbiddenFlag;
		    memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
		}
	}
}

void  COpenATCCommWithGB20999Thread::QueryLightGroupConfig(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData)
{
	TDataConfig tDataConfig;
	tDataConfig.m_byIndex = byIndex + 1;
	tDataConfig.m_byDataClassID = m_chUnPackedBuff[C_N_DATACLASS_ID_POS + byIndex * 6];
	tDataConfig.m_byObjectID = m_chUnPackedBuff[C_N_OBJECT_ID_POS + byIndex * 6];
	tDataConfig.m_byAttributeID = m_chUnPackedBuff[C_N_ATTRIBUTE_ID_POS + byIndex * 6];
	tDataConfig.m_byElementID = m_chUnPackedBuff[C_N_ELEMENT_ID_POS + byIndex * 6];
	tDataConfig.m_byDataValueLength = 1;
	tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;

	if (tDataConfig.m_byElementID < 1 || tDataConfig.m_byElementID > 64)
	{
		tDataConfig.m_byDataValue[0] = BAD_VALUE_NULL;
		memcpy(&tWrongReturnData.m_tDataConfig[tWrongReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		tWrongReturnData.m_byReturnCount += 1;
		return;
	}
	
	bool bFlag = false;
	BYTE byLightGroupType = LIGHT_GROUP_VEHICLE;
	for (int i = 0;i < m_tAscParamInfo.m_stChannelTableValidSize;i++)
	{
		if (m_tAscParamInfo.m_stAscChannelTable[i].m_byChannelNumber == tDataConfig.m_byElementID)
		{
			byLightGroupType = m_tAscParamInfo.m_stAscChannelTable[i].m_byLightControlType;

			bFlag = true;
			break;
		}
	}

	if (bFlag)
	{
		if (tDataConfig.m_byAttributeID == LIGHT_GROUP_TYPE_ID || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValue[0] = tDataConfig.m_byElementID;
			memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == LIGHT_GROUP_TYPE || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValue[0] = byLightGroupType;
		    memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
		}
	}
	else
	{
		tDataConfig.m_byDataValue[0] = BAD_VALUE_NULL;
		memcpy(&tWrongReturnData.m_tDataConfig[tWrongReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		tWrongReturnData.m_byReturnCount += 1;
	}
}

void  COpenATCCommWithGB20999Thread::QueryLightGroupStatus(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData)
{
	TDataConfig tDataConfig;
	tDataConfig.m_byIndex = byIndex + 1;
	tDataConfig.m_byDataClassID = m_chUnPackedBuff[C_N_DATACLASS_ID_POS + byIndex * 6];
	tDataConfig.m_byObjectID = m_chUnPackedBuff[C_N_OBJECT_ID_POS + byIndex * 6];
	tDataConfig.m_byAttributeID = m_chUnPackedBuff[C_N_ATTRIBUTE_ID_POS + byIndex * 6];
	tDataConfig.m_byElementID = m_chUnPackedBuff[C_N_ELEMENT_ID_POS + byIndex * 6];
	tDataConfig.m_byDataValueLength = 1;
	tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;

	if (tDataConfig.m_byElementID < 1 || tDataConfig.m_byElementID > 64)
	{
		tDataConfig.m_byDataValue[0] = BAD_VALUE_NULL;
		memcpy(&tWrongReturnData.m_tDataConfig[tWrongReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		tWrongReturnData.m_byReturnCount += 1;
		return;
	}

	TLampClrStatus tLampClrStatus;
	memset(&tLampClrStatus, 0, sizeof(TLampClrStatus)); 
	m_pOpenATCRunStatus->GetLampClrStatus(tLampClrStatus);

	bool bFlag = false;
	BYTE byLightGroupStatus = LIGHT_STATUS_RED;
	for (int i = 0;i < m_tAscParamInfo.m_stChannelTableValidSize;i++)
	{
		if (m_tAscParamInfo.m_stAscChannelTable[i].m_byChannelNumber == tDataConfig.m_byElementID)
		{
			if(tLampClrStatus.m_achLampClr[(m_tAscParamInfo.m_stAscChannelTable[i].m_byChannelNumber - 1) * 3] == LAMP_CLR_ON && tLampClrStatus.m_achLampClr[(m_tAscParamInfo.m_stAscChannelTable[i].m_byChannelNumber - 1) * 3 + 1] == LAMP_CLR_ON)
			{
				byLightGroupStatus = LIGHT_STATUS_REDYELLOW;
			}
			else if (tLampClrStatus.m_achLampClr[(m_tAscParamInfo.m_stAscChannelTable[i].m_byChannelNumber - 1) * 3] == LAMP_CLR_ON)
			{
				byLightGroupStatus = LIGHT_STATUS_RED;
			}
			else if(tLampClrStatus.m_achLampClr[(m_tAscParamInfo.m_stAscChannelTable[i].m_byChannelNumber - 1) * 3] == LAMP_CLR_FLASH)
			{
				byLightGroupStatus = LIGHT_STATUS_REDFLASH;
			}
			else if (tLampClrStatus.m_achLampClr[(m_tAscParamInfo.m_stAscChannelTable[i].m_byChannelNumber - 1) * 3 + 1] == LAMP_CLR_ON)
			{
				byLightGroupStatus = LIGHT_STATUS_YELLOW;
			}
			else if (tLampClrStatus.m_achLampClr[(m_tAscParamInfo.m_stAscChannelTable[i].m_byChannelNumber - 1) * 3 + 1] == LAMP_CLR_FLASH)
			{
				byLightGroupStatus = LIGHT_STATUS_YELLOWFLASH;
			}
			else if (tLampClrStatus.m_achLampClr[(m_tAscParamInfo.m_stAscChannelTable[i].m_byChannelNumber - 1) * 3 + 2] == LAMP_CLR_ON)
			{
				byLightGroupStatus = LIGHT_STATUS_GREEN;
			}
			else if (tLampClrStatus.m_achLampClr[(m_tAscParamInfo.m_stAscChannelTable[i].m_byChannelNumber - 1) * 3 + 2] == LAMP_CLR_FLASH)
			{
				byLightGroupStatus = LIGHT_STATUS_GREENFLASH;
			}
			else if (tLampClrStatus.m_achLampClr[(m_tAscParamInfo.m_stAscChannelTable[i].m_byChannelNumber - 1) * 3] == LAMP_CLR_OFF &&
					 tLampClrStatus.m_achLampClr[(m_tAscParamInfo.m_stAscChannelTable[i].m_byChannelNumber - 1) * 3 + 1] == LAMP_CLR_OFF &&
					 tLampClrStatus.m_achLampClr[(m_tAscParamInfo.m_stAscChannelTable[i].m_byChannelNumber - 1) * 3 + 2] == LAMP_CLR_OFF)
			{
				byLightGroupStatus = LIGHT_STATUS_OFF;
			}
			bFlag = true;
			break;
		}
	}

	if (bFlag)
	{
		if (tDataConfig.m_byAttributeID == PHASE_CONFIG_ID || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValue[0] = tDataConfig.m_byElementID;
			memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == PHASE_LIGHTGROUP || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValue[0] = byLightGroupStatus;
			memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
		}
	}
	else
	{
		tDataConfig.m_byDataValue[0] = BAD_VALUE_NULL;
		memcpy(&tWrongReturnData.m_tDataConfig[tWrongReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		tWrongReturnData.m_byReturnCount += 1;
	}
}

void  COpenATCCommWithGB20999Thread::QueryLightGroupControl(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData)
{
	TDataConfig tDataConfig;
	tDataConfig.m_byIndex = byIndex + 1;
	tDataConfig.m_byDataClassID = m_chUnPackedBuff[C_N_DATACLASS_ID_POS + byIndex * 6];
	tDataConfig.m_byObjectID = m_chUnPackedBuff[C_N_OBJECT_ID_POS + byIndex * 6];
	tDataConfig.m_byAttributeID = m_chUnPackedBuff[C_N_ATTRIBUTE_ID_POS + byIndex * 6];
	tDataConfig.m_byElementID = m_chUnPackedBuff[C_N_ELEMENT_ID_POS + byIndex * 6];
	tDataConfig.m_byDataValueLength = 1;
	tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;

	if (tDataConfig.m_byElementID < 1 || tDataConfig.m_byElementID > 64)
	{
		tDataConfig.m_byDataValue[0] = BAD_VALUE_NULL;
		memcpy(&tWrongReturnData.m_tDataConfig[tWrongReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		tWrongReturnData.m_byReturnCount += 1;
		return;
	}

	int  byChannelIndex = -1;
	for (int i = 0;i < m_tAscParamInfo.m_stChannelTableValidSize;i++)
	{
		if (m_tAscParamInfo.m_stAscChannelTable[i].m_byChannelNumber == tDataConfig.m_byElementID)
		{
			byChannelIndex = i;
			break;
		}
	}

	if (byChannelIndex != -1)
	{
		if (tDataConfig.m_byAttributeID == CONTROL_ID || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValue[0] = tDataConfig.m_byElementID;
			memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == CONTROL_SHIELD || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValue[0] = m_tAscParamInfo.m_stAscChannelTable[byChannelIndex].m_byScreenFlag;
			memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == CONTROL_PROHIBIT || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValue[0] = m_tAscParamInfo.m_stAscChannelTable[byChannelIndex].m_byForbiddenFlag;
			memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
		}
	}
	else
	{
		tDataConfig.m_byDataValue[0] = BAD_VALUE_NULL;
		memcpy(&tWrongReturnData.m_tDataConfig[tWrongReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		tWrongReturnData.m_byReturnCount += 1;
	}
}

void COpenATCCommWithGB20999Thread::QueryPhaseCount(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData)
{
	TDataConfig tDataConfig;
	tDataConfig.m_byIndex = byIndex + 1;
	tDataConfig.m_byDataClassID = m_chUnPackedBuff[C_N_DATACLASS_ID_POS + byIndex * 6];
	tDataConfig.m_byObjectID = m_chUnPackedBuff[C_N_OBJECT_ID_POS + byIndex * 6];
	tDataConfig.m_byAttributeID = m_chUnPackedBuff[C_N_ATTRIBUTE_ID_POS + byIndex * 6];
	tDataConfig.m_byElementID = m_chUnPackedBuff[C_N_ELEMENT_ID_POS + byIndex * 6];
	tDataConfig.m_byDataValueLength = 1;
	tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;

	tDataConfig.m_byDataValue[0] = m_tAscParamInfo.m_stPhaseTableValidSize;
	memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
	tCorrectReturnData.m_byReturnCount += 1;
}

void COpenATCCommWithGB20999Thread::QueryAllElementPhaseConfig(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData)
{
	for (int i = 0;i < m_tAscParamInfo.m_stPhaseTableValidSize;i++)
	{
		TDataConfig tDataConfig;
		//tDataConfig.m_byIndex = byIndex + 1;
		tDataConfig.m_byIndex = i + 1;	//HPH 2021.12.07
		tDataConfig.m_byDataClassID = m_chUnPackedBuff[C_N_DATACLASS_ID_POS + byIndex * 6];
		tDataConfig.m_byObjectID = m_chUnPackedBuff[C_N_OBJECT_ID_POS + byIndex * 6];
		tDataConfig.m_byAttributeID = m_chUnPackedBuff[C_N_ATTRIBUTE_ID_POS + byIndex * 6];
		tDataConfig.m_byElementID = i + 1;

		if (tDataConfig.m_byAttributeID == PHASE_CONFIG_ID || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValueLength = 1;
	        tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			tDataConfig.m_byDataValue[0] = tDataConfig.m_byElementID;
		    memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == PHASE_LIGHTGROUP || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValueLength = C_N_MAX_PHASE_LIGHTGROUP;
	        tDataConfig.m_byDataLength = 4 + C_N_MAX_PHASE_LIGHTGROUP;
			for (int j = 0;j < C_N_MAX_PHASE_LIGHTGROUP;j++)
			{
				tDataConfig.m_byDataValue[j] = m_tAscParamInfo.m_stAscPhaseTable[i].m_byLightGroup[j];
			}
		    memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == PHASE_LOSETRANSITIONTYPE1 || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValueLength = 1;
	        tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			tDataConfig.m_byDataValue[0] = m_tAscParamInfo.m_stAscPhaseTable[i].m_nLoseControlLampOneType;
		    memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == PHASE_LOSETRANSITIONTIME1 || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValueLength = 1;
	        tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			tDataConfig.m_byDataValue[0] = m_tAscParamInfo.m_stAscPhaseTable[i].m_nLoseControlLampOneTime;
		    memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == PHASE_LOSETRANSITIONTYPE2 || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValueLength = 1;
	        tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			tDataConfig.m_byDataValue[0] = m_tAscParamInfo.m_stAscPhaseTable[i].m_nLoseControlLampTwoType;
		    memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == PHASE_LOSETRANSITIONTIME2 || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValueLength = 1;
	        tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			tDataConfig.m_byDataValue[0] = m_tAscParamInfo.m_stAscPhaseTable[i].m_nLoseControlLampTwoTime;
		    memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == PHASE_LOSETRANSITIONTYPE3 || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValueLength = 1;
	        tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			tDataConfig.m_byDataValue[0] = m_tAscParamInfo.m_stAscPhaseTable[i].m_nLoseControlLampThreeType;
		    memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == PHASE_LOSETRANSITIONTIME3 || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValueLength = 1;
	        tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			tDataConfig.m_byDataValue[0] = m_tAscParamInfo.m_stAscPhaseTable[i].m_nLoseControlLampThreeTime;
		    memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == PHASE_GETTRANSITIONTYPE1 || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValueLength = 1;
	        tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			tDataConfig.m_byDataValue[0] = m_tAscParamInfo.m_stAscPhaseTable[i].m_nGetControlLampOneType;
		    memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == PHASE_GETTRANSITIONTIME1 || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValueLength = 1;
	        tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			tDataConfig.m_byDataValue[0] = m_tAscParamInfo.m_stAscPhaseTable[i].m_nGetControlLampOneTime;
		    memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == PHASE_GETTRANSITIONTYPE2 || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValueLength = 1;
	        tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			tDataConfig.m_byDataValue[0] = m_tAscParamInfo.m_stAscPhaseTable[i].m_nGetControlLampTwoType;
		    memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == PHASE_GETTRANSITIONTIME2 || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValueLength = 1;
	        tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			tDataConfig.m_byDataValue[0] = m_tAscParamInfo.m_stAscPhaseTable[i].m_nGetControlLampTwoTime;
		    memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == PHASE_GETTRANSITIONTYPE3 || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValueLength = 1;
	        tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			tDataConfig.m_byDataValue[0] = m_tAscParamInfo.m_stAscPhaseTable[i].m_nGetControlLampThreeType;
		    memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == PHASE_GETTRANSITIONTIME3 || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValueLength = 1;
	        tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			tDataConfig.m_byDataValue[0] = m_tAscParamInfo.m_stAscPhaseTable[i].m_nGetControlLampThreeTime;
		    memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == PHASE_TURNONGETTRANSITIONTYPE1 || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValueLength = 1;
	        tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			tDataConfig.m_byDataValue[0] = m_tAscParamInfo.m_stAscPhaseTable[i].m_nPowerOnGetControlLampOneType;
		    memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == PHASE_TURNONGETTRANSITIONTIME1 || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValueLength = 1;
	        tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			tDataConfig.m_byDataValue[0] = m_tAscParamInfo.m_stAscPhaseTable[i].m_nPowerOnGetControlLampOneTime;
		    memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == PHASE_TURNONGETTRANSITIONTYPE2 || tDataConfig.m_byAttributeID == 0)
		{
	   	    tDataConfig.m_byDataValueLength = 1;
	        tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			tDataConfig.m_byDataValue[0] = m_tAscParamInfo.m_stAscPhaseTable[i].m_nPowerOnGetControlLampTwoType;
		    memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == PHASE_TURNONGETTRANSITIONTIME2 || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValueLength = 1;
	        tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			tDataConfig.m_byDataValue[0] = m_tAscParamInfo.m_stAscPhaseTable[i].m_nPowerOnGetControlLampTwoTime;
		    memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == PHASE_TURNONGETTRANSITIONTYPE3 || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValueLength = 1;
	        tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			tDataConfig.m_byDataValue[0] = m_tAscParamInfo.m_stAscPhaseTable[i].m_nPowerOnGetControlLampThreeType;
		    memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == PHASE_TURNONGETTRANSITIONTIME3 || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValueLength = 1;
	        tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			tDataConfig.m_byDataValue[0] = m_tAscParamInfo.m_stAscPhaseTable[i].m_nPowerOnGetControlLampThreeTime;
		    memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == PHASE_TURNONLOSETRANSITIONTYPE1 || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValueLength = 1;
	        tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			tDataConfig.m_byDataValue[0] = m_tAscParamInfo.m_stAscPhaseTable[i].m_nPowerOnLossControlLampOneType;
		    memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == PHASE_TURNONLOSETRANSITIONTIME1 || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValueLength = 1;
	        tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			tDataConfig.m_byDataValue[0] = m_tAscParamInfo.m_stAscPhaseTable[i].m_nPowerOnLossControlLampOneTime;
		    memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == PHASE_TURNONLOSETRANSITIONTYPE2 || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValueLength = 1;
	        tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			tDataConfig.m_byDataValue[0] = m_tAscParamInfo.m_stAscPhaseTable[i].m_nPowerOnLossControlLampTwoType;
		    memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == PHASE_TURNONLOSETRANSITIONTIME2 || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValueLength = 1;
	        tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			tDataConfig.m_byDataValue[0] = m_tAscParamInfo.m_stAscPhaseTable[i].m_nPowerOnLossControlLampTwoTime;
		    memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == PHASE_TURNONLOSETRANSITIONTYPE3 || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValueLength = 1;
	        tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			tDataConfig.m_byDataValue[0] = m_tAscParamInfo.m_stAscPhaseTable[i].m_nPowerOnLossControlLampThreeType;
		    memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == PHASE_TURNONLOSETRANSITIONTIME3 || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValueLength = 1;
	        tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			tDataConfig.m_byDataValue[0] = m_tAscParamInfo.m_stAscPhaseTable[i].m_nPowerOnLossControlLampThreeTime;
		    memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == PHASE_MIN_GREENTIME || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValueLength = 2;
	        tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			short nTime = ntohs(m_tAscParamInfo.m_stAscPhaseTable[i].m_wPhaseMinimumGreen);
			memcpy(tDataConfig.m_byDataValue, &nTime, 2);
		    memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == PHASE_MAX1_GREENTIME || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValueLength = 2;
	        tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			short nTime = ntohs(m_tAscParamInfo.m_stAscPhaseTable[i].m_wPhaseMaximum1);
			memcpy(tDataConfig.m_byDataValue, &nTime, 2);
		    memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == PHASE_MAX2_GREENTIME || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValueLength = 2;
	        tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			short nTime = ntohs(m_tAscParamInfo.m_stAscPhaseTable[i].m_wPhaseMaximum2);
			memcpy(tDataConfig.m_byDataValue, &nTime, 2);
		    memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == PHASE_PASSAGE_GREENTIME || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValueLength = 2;
	        tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			//short nTime = ntohs(m_tAscParamInfo.m_stAscPhaseTable[i].m_byPhasePassage);
			short nTime = ntohs(m_tAscParamInfo.m_stAscPhaseTable[i].m_byPhaseExtend);
			memcpy(tDataConfig.m_byDataValue, &nTime, 2);
		    memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == PHASE_CALL || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValueLength = C_N_MAX_PHASE_DETECTOR;
	        tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			for (int j = 0;j < C_N_MAX_PHASE_DETECTOR;j++)
			{
				tDataConfig.m_byDataValue[j] = m_tAscParamInfo.m_stAscPhaseTable[i].m_byPhaseCall[j];
			}
		    memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
		}
	}
}

void COpenATCCommWithGB20999Thread::QueryAllElementPhaseControl(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData)
{
	for (int i = 0;i < m_tAscParamInfo.m_stPhaseTableValidSize;i++)
	{
		TDataConfig tDataConfig;
		//tDataConfig.m_byIndex = byIndex + 1;
		tDataConfig.m_byIndex = i + 1;	//HPH 2021.12.07
		tDataConfig.m_byDataClassID = m_chUnPackedBuff[C_N_DATACLASS_ID_POS + byIndex * 6];
		tDataConfig.m_byObjectID = m_chUnPackedBuff[C_N_OBJECT_ID_POS + byIndex * 6];
		tDataConfig.m_byAttributeID = m_chUnPackedBuff[C_N_ATTRIBUTE_ID_POS + byIndex * 6];
		tDataConfig.m_byElementID = i + 1;
		tDataConfig.m_byDataValueLength = 1;
		tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;

		if (tDataConfig.m_byAttributeID == CONTROL_ID || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValue[0] = tDataConfig.m_byElementID;
		    memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == CONTROL_SHIELD || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValue[0] = m_tAscParamInfo.m_stAscPhaseTable[i].m_byScreenFlag;
		    memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == CONTROL_PROHIBIT || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValue[0] = m_tAscParamInfo.m_stAscPhaseTable[i].m_byForbiddenFlag;
		    memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
		}
	}
}

void  COpenATCCommWithGB20999Thread::QueryPhaseConfig(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData)
{
	TDataConfig tDataConfig;
	tDataConfig.m_byIndex = byIndex + 1;
	tDataConfig.m_byDataClassID = m_chUnPackedBuff[C_N_DATACLASS_ID_POS + byIndex * 6];
	tDataConfig.m_byObjectID = m_chUnPackedBuff[C_N_OBJECT_ID_POS + byIndex * 6];
	tDataConfig.m_byAttributeID = m_chUnPackedBuff[C_N_ATTRIBUTE_ID_POS + byIndex * 6];
	tDataConfig.m_byElementID = m_chUnPackedBuff[C_N_ELEMENT_ID_POS + byIndex * 6];

	if (tDataConfig.m_byElementID < 1 || tDataConfig.m_byElementID > 64)
	{
		tDataConfig.m_byDataValueLength = 1;
	    tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
		tDataConfig.m_byDataValue[0] = BAD_VALUE_NULL;
		memcpy(&tWrongReturnData.m_tDataConfig[tWrongReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		tWrongReturnData.m_byReturnCount += 1;
		return;
	}

	int  byPhaseIndex = -1;
	int  i = 0;
	for (i = 0;i < m_tAscParamInfo.m_stPhaseTableValidSize;i++)
	{
		if (m_tAscParamInfo.m_stAscPhaseTable[i].m_byPhaseNumber == tDataConfig.m_byElementID)
		{
			byPhaseIndex = i;
			break;
		}
	}

	if (byPhaseIndex == -1)
	{
		tDataConfig.m_byDataValueLength = 1;
	    tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
		tDataConfig.m_byDataValue[0] = BAD_VALUE_NULL;
		memcpy(&tWrongReturnData.m_tDataConfig[tWrongReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		tWrongReturnData.m_byReturnCount += 1;
		return;
	}

	if (tDataConfig.m_byAttributeID == PHASE_CONFIG_ID ||  tDataConfig.m_byAttributeID == 0)
	{
		tDataConfig.m_byDataValueLength = 1;
	    tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
		tDataConfig.m_byDataValue[0] = tDataConfig.m_byElementID;
		memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		tCorrectReturnData.m_byReturnCount += 1;
	}
	if (tDataConfig.m_byAttributeID == PHASE_LIGHTGROUP || tDataConfig.m_byAttributeID == 0)
	{
		tDataConfig.m_byDataValueLength = C_N_MAX_PHASE_LIGHTGROUP;
		tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
		for (i = 0;i < C_N_MAX_PHASE_LIGHTGROUP;i++)
		{
			tDataConfig.m_byDataValue[i] = m_tAscParamInfo.m_stAscPhaseTable[byPhaseIndex].m_byLightGroup[i];
		}
		memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		tCorrectReturnData.m_byReturnCount += 1;
	}
	if (tDataConfig.m_byAttributeID == PHASE_LOSETRANSITIONTYPE1 || tDataConfig.m_byAttributeID == 0)
	{
		tDataConfig.m_byDataValueLength = 1;
	    tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
		tDataConfig.m_byDataValue[0] = m_tAscParamInfo.m_stAscPhaseTable[byPhaseIndex].m_nLoseControlLampOneType;
		memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		tCorrectReturnData.m_byReturnCount += 1;
	}
	if (tDataConfig.m_byAttributeID == PHASE_LOSETRANSITIONTIME1 || tDataConfig.m_byAttributeID == 0)
	{
		tDataConfig.m_byDataValueLength = 1;
	    tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
		tDataConfig.m_byDataValue[0] = m_tAscParamInfo.m_stAscPhaseTable[byPhaseIndex].m_nLoseControlLampOneTime;
		memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		tCorrectReturnData.m_byReturnCount += 1;
	}
	if (tDataConfig.m_byAttributeID == PHASE_LOSETRANSITIONTYPE2 || tDataConfig.m_byAttributeID == 0)
	{
		tDataConfig.m_byDataValueLength = 1;
	    tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
		tDataConfig.m_byDataValue[0] = m_tAscParamInfo.m_stAscPhaseTable[byPhaseIndex].m_nLoseControlLampTwoType;
		memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		tCorrectReturnData.m_byReturnCount += 1;
	}
	if (tDataConfig.m_byAttributeID == PHASE_LOSETRANSITIONTIME2 || tDataConfig.m_byAttributeID == 0)
	{
		tDataConfig.m_byDataValueLength = 1;
	    tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
		tDataConfig.m_byDataValue[0] = m_tAscParamInfo.m_stAscPhaseTable[byPhaseIndex].m_nLoseControlLampTwoTime;
		memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		tCorrectReturnData.m_byReturnCount += 1;
	}
	if (tDataConfig.m_byAttributeID == PHASE_LOSETRANSITIONTYPE3 || tDataConfig.m_byAttributeID == 0)
	{
		tDataConfig.m_byDataValueLength = 1;
	    tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
		tDataConfig.m_byDataValue[0] = m_tAscParamInfo.m_stAscPhaseTable[byPhaseIndex].m_nLoseControlLampThreeType;
		memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		tCorrectReturnData.m_byReturnCount += 1;
	}
	if (tDataConfig.m_byAttributeID == PHASE_LOSETRANSITIONTIME3 || tDataConfig.m_byAttributeID == 0)
	{
		tDataConfig.m_byDataValueLength = 1;
	    tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
		tDataConfig.m_byDataValue[0] = m_tAscParamInfo.m_stAscPhaseTable[byPhaseIndex].m_nLoseControlLampThreeTime;
		memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		tCorrectReturnData.m_byReturnCount += 1;
	}
	if (tDataConfig.m_byAttributeID == PHASE_GETTRANSITIONTYPE1 || tDataConfig.m_byAttributeID == 0)
	{
		tDataConfig.m_byDataValueLength = 1;
	    tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
		tDataConfig.m_byDataValue[0] = m_tAscParamInfo.m_stAscPhaseTable[byPhaseIndex].m_nGetControlLampOneType;
		memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		tCorrectReturnData.m_byReturnCount += 1;
	}
	if (tDataConfig.m_byAttributeID == PHASE_GETTRANSITIONTIME1 || tDataConfig.m_byAttributeID == 0)
	{
		tDataConfig.m_byDataValueLength = 1;
	    tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
		tDataConfig.m_byDataValue[0] = m_tAscParamInfo.m_stAscPhaseTable[byPhaseIndex].m_nGetControlLampOneTime;
		memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		tCorrectReturnData.m_byReturnCount += 1;
	}
	if (tDataConfig.m_byAttributeID == PHASE_GETTRANSITIONTYPE2 || tDataConfig.m_byAttributeID == 0)
	{
		tDataConfig.m_byDataValueLength = 1;
	    tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
		tDataConfig.m_byDataValue[0] = m_tAscParamInfo.m_stAscPhaseTable[byPhaseIndex].m_nGetControlLampTwoType;
		memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		tCorrectReturnData.m_byReturnCount += 1;
	}
	if (tDataConfig.m_byAttributeID == PHASE_GETTRANSITIONTIME2 || tDataConfig.m_byAttributeID == 0)
	{
		tDataConfig.m_byDataValueLength = 1;
	    tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
		tDataConfig.m_byDataValue[0] = m_tAscParamInfo.m_stAscPhaseTable[byPhaseIndex].m_nGetControlLampTwoTime;
		memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		tCorrectReturnData.m_byReturnCount += 1;
	}
	if (tDataConfig.m_byAttributeID == PHASE_GETTRANSITIONTYPE3 || tDataConfig.m_byAttributeID == 0)
	{
		tDataConfig.m_byDataValueLength = 1;
	    tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
		tDataConfig.m_byDataValue[0] = m_tAscParamInfo.m_stAscPhaseTable[byPhaseIndex].m_nGetControlLampThreeType;
		memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		tCorrectReturnData.m_byReturnCount += 1;
	}
	if (tDataConfig.m_byAttributeID == PHASE_GETTRANSITIONTIME3 || tDataConfig.m_byAttributeID == 0)
	{
		tDataConfig.m_byDataValueLength = 1;
	    tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
		tDataConfig.m_byDataValue[0] = m_tAscParamInfo.m_stAscPhaseTable[byPhaseIndex].m_nGetControlLampThreeTime;
		memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		tCorrectReturnData.m_byReturnCount += 1;
	}
	if (tDataConfig.m_byAttributeID == PHASE_TURNONGETTRANSITIONTYPE1 || tDataConfig.m_byAttributeID == 0)
	{
		tDataConfig.m_byDataValueLength = 1;
	    tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
		tDataConfig.m_byDataValue[0] = m_tAscParamInfo.m_stAscPhaseTable[byPhaseIndex].m_nPowerOnGetControlLampOneType;;
		memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		tCorrectReturnData.m_byReturnCount += 1;
	}
	if (tDataConfig.m_byAttributeID == PHASE_TURNONGETTRANSITIONTIME1 || tDataConfig.m_byAttributeID == 0)
	{
		tDataConfig.m_byDataValueLength = 1;
	    tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
		tDataConfig.m_byDataValue[0] = m_tAscParamInfo.m_stAscPhaseTable[byPhaseIndex].m_nPowerOnGetControlLampOneTime;
		memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		tCorrectReturnData.m_byReturnCount += 1;
	}
	if (tDataConfig.m_byAttributeID == PHASE_TURNONGETTRANSITIONTYPE2 || tDataConfig.m_byAttributeID == 0)
	{
		tDataConfig.m_byDataValueLength = 1;
	    tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
		tDataConfig.m_byDataValue[0] = m_tAscParamInfo.m_stAscPhaseTable[byPhaseIndex].m_nPowerOnGetControlLampTwoType;
		memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		tCorrectReturnData.m_byReturnCount += 1;
	}
	if (tDataConfig.m_byAttributeID == PHASE_TURNONGETTRANSITIONTIME2 || tDataConfig.m_byAttributeID == 0)
	{
		tDataConfig.m_byDataValueLength = 1;
	    tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
		tDataConfig.m_byDataValue[0] = m_tAscParamInfo.m_stAscPhaseTable[byPhaseIndex].m_nPowerOnGetControlLampTwoTime;
		memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		tCorrectReturnData.m_byReturnCount += 1;
	}
	if (tDataConfig.m_byAttributeID == PHASE_TURNONGETTRANSITIONTYPE3 || tDataConfig.m_byAttributeID == 0)
	{
		tDataConfig.m_byDataValueLength = 1;
	    tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
		tDataConfig.m_byDataValue[0] = m_tAscParamInfo.m_stAscPhaseTable[byPhaseIndex].m_nPowerOnGetControlLampThreeType;
		memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		tCorrectReturnData.m_byReturnCount += 1;
	}
	if (tDataConfig.m_byAttributeID == PHASE_TURNONGETTRANSITIONTIME3 || tDataConfig.m_byAttributeID == 0)
	{
		tDataConfig.m_byDataValueLength = 1;
	    tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
		tDataConfig.m_byDataValue[0] = m_tAscParamInfo.m_stAscPhaseTable[byPhaseIndex].m_nPowerOnGetControlLampThreeTime;
		memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		tCorrectReturnData.m_byReturnCount += 1;
	}
	if (tDataConfig.m_byAttributeID == PHASE_TURNONLOSETRANSITIONTYPE1 || tDataConfig.m_byAttributeID == 0)
	{
		tDataConfig.m_byDataValueLength = 1;
	    tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
		tDataConfig.m_byDataValue[0] = m_tAscParamInfo.m_stAscPhaseTable[byPhaseIndex].m_nPowerOnLossControlLampOneType;
		memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		tCorrectReturnData.m_byReturnCount += 1;
	}
	if (tDataConfig.m_byAttributeID == PHASE_TURNONLOSETRANSITIONTIME1 || tDataConfig.m_byAttributeID == 0)
	{
		tDataConfig.m_byDataValueLength = 1;
	    tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
		tDataConfig.m_byDataValue[0] = m_tAscParamInfo.m_stAscPhaseTable[byPhaseIndex].m_nPowerOnLossControlLampOneTime;
		memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		tCorrectReturnData.m_byReturnCount += 1;
	}
	if (tDataConfig.m_byAttributeID == PHASE_TURNONLOSETRANSITIONTYPE2 || tDataConfig.m_byAttributeID == 0)
	{
		tDataConfig.m_byDataValueLength = 1;
	    tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
		tDataConfig.m_byDataValue[0] = m_tAscParamInfo.m_stAscPhaseTable[byPhaseIndex].m_nPowerOnLossControlLampTwoType;
		memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		tCorrectReturnData.m_byReturnCount += 1;
	}
	if (tDataConfig.m_byAttributeID == PHASE_TURNONLOSETRANSITIONTIME2 || tDataConfig.m_byAttributeID == 0)
	{
		tDataConfig.m_byDataValueLength = 1;
	    tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
		tDataConfig.m_byDataValue[0] = m_tAscParamInfo.m_stAscPhaseTable[byPhaseIndex].m_nPowerOnLossControlLampTwoTime;
		memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		tCorrectReturnData.m_byReturnCount += 1;
	}
	if (tDataConfig.m_byAttributeID == PHASE_TURNONLOSETRANSITIONTYPE3 || tDataConfig.m_byAttributeID == 0)
	{
		tDataConfig.m_byDataValueLength = 1;
	    tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
		tDataConfig.m_byDataValue[0] = m_tAscParamInfo.m_stAscPhaseTable[byPhaseIndex].m_nPowerOnLossControlLampThreeType;
		memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		tCorrectReturnData.m_byReturnCount += 1;
	}
	if (tDataConfig.m_byAttributeID == PHASE_TURNONLOSETRANSITIONTIME3 || tDataConfig.m_byAttributeID == 0)
	{
		tDataConfig.m_byDataValueLength = 1;
	    tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
		tDataConfig.m_byDataValue[0] = m_tAscParamInfo.m_stAscPhaseTable[byPhaseIndex].m_nPowerOnLossControlLampThreeTime;
		memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		tCorrectReturnData.m_byReturnCount += 1;
	}
	if (tDataConfig.m_byAttributeID == PHASE_MIN_GREENTIME || tDataConfig.m_byAttributeID == 0)
	{
		tDataConfig.m_byDataValueLength = 2;
	    tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
		short nTime = ntohs(m_tAscParamInfo.m_stAscPhaseTable[byPhaseIndex].m_wPhaseMinimumGreen);
		memcpy(tDataConfig.m_byDataValue, &nTime, 2);
		//m_tAscParamInfo.m_stAscPhaseTable[byPhaseIndex].m_wPhaseMinimumGreen = nTime;
		memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		tCorrectReturnData.m_byReturnCount += 1;
	}
	if (tDataConfig.m_byAttributeID == PHASE_MAX1_GREENTIME || tDataConfig.m_byAttributeID == 0)
	{
		tDataConfig.m_byDataValueLength = 2;
	    tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
		short nTime = ntohs(m_tAscParamInfo.m_stAscPhaseTable[byPhaseIndex].m_wPhaseMaximum1);
		memcpy(tDataConfig.m_byDataValue, &nTime, 2);
		//m_tAscParamInfo.m_stAscPhaseTable[byPhaseIndex].m_wPhaseMaximum1 = nTime;
		memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		tCorrectReturnData.m_byReturnCount += 1;
	}
	if (tDataConfig.m_byAttributeID == PHASE_MAX2_GREENTIME || tDataConfig.m_byAttributeID == 0)
	{
		tDataConfig.m_byDataValueLength = 2;
	    tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
		short nTime = ntohs(m_tAscParamInfo.m_stAscPhaseTable[byPhaseIndex].m_wPhaseMaximum2);
		memcpy(tDataConfig.m_byDataValue, &nTime, 2);
		//m_tAscParamInfo.m_stAscPhaseTable[byPhaseIndex].m_wPhaseMaximum2 = nTime;
		memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		tCorrectReturnData.m_byReturnCount += 1;
	}
	if (tDataConfig.m_byAttributeID == PHASE_PASSAGE_GREENTIME || tDataConfig.m_byAttributeID == 0)
	{
		tDataConfig.m_byDataValueLength = 2;
	    tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
		//short nTime = ntohs(m_tAscParamInfo.m_stAscPhaseTable[byPhaseIndex].m_byPhasePassage);
		short nTime = ntohs(m_tAscParamInfo.m_stAscPhaseTable[byPhaseIndex].m_byPhaseExtend);
		memcpy(tDataConfig.m_byDataValue, &nTime, 2);
		//m_tAscParamInfo.m_stAscPhaseTable[byPhaseIndex].m_byPhasePassage = nTime;
		//m_tAscParamInfo.m_stAscPhaseTable[byPhaseIndex].m_byPhaseExtend = nTime;
		memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		tCorrectReturnData.m_byReturnCount += 1;
	}
	if (tDataConfig.m_byAttributeID == PHASE_CALL || tDataConfig.m_byAttributeID == 0)
	{
		tDataConfig.m_byDataValueLength = C_N_MAX_PHASE_DETECTOR;
		tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
		for (i = 0;i < C_N_MAX_PHASE_DETECTOR;i++)
		{
			tDataConfig.m_byDataValue[i] = m_tAscParamInfo.m_stAscPhaseTable[byPhaseIndex].m_byPhaseCall[i];
		}
		memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		tCorrectReturnData.m_byReturnCount += 1;
	}	
}

void  COpenATCCommWithGB20999Thread::QueryPhaseControl(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData)
{
	TDataConfig tDataConfig;
	tDataConfig.m_byIndex = byIndex + 1;
	tDataConfig.m_byDataClassID = m_chUnPackedBuff[C_N_DATACLASS_ID_POS + byIndex * 6];
	tDataConfig.m_byObjectID = m_chUnPackedBuff[C_N_OBJECT_ID_POS + byIndex * 6];
	tDataConfig.m_byAttributeID = m_chUnPackedBuff[C_N_ATTRIBUTE_ID_POS + byIndex * 6];
	tDataConfig.m_byElementID = m_chUnPackedBuff[C_N_ELEMENT_ID_POS + byIndex * 6];
	tDataConfig.m_byDataValueLength = 1;
	tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;

	if (tDataConfig.m_byElementID < 1 || tDataConfig.m_byElementID > 64)
	{
		tDataConfig.m_byDataValueLength = 1;
	    tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
		tDataConfig.m_byDataValue[0] = BAD_VALUE_NULL;
		memcpy(&tWrongReturnData.m_tDataConfig[tWrongReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		tWrongReturnData.m_byReturnCount += 1;
		return;
	}

	int  byPhaseIndex = -1;
	for (int i = 0;i < m_tAscParamInfo.m_stPhaseTableValidSize;i++)
	{
		if (m_tAscParamInfo.m_stAscPhaseTable[i].m_byPhaseNumber == tDataConfig.m_byElementID)
		{
			byPhaseIndex = i;
			break;
		}
	}

	if (byPhaseIndex != -1)
	{
		if (tDataConfig.m_byAttributeID == PHASE_CONTROL_ID || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValue[0] = tDataConfig.m_byElementID;
			memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == PHASE_CONTROL_SHIELD || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValue[0] = m_tAscParamInfo.m_stAscPhaseTable[byPhaseIndex].m_byScreenFlag;
			memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		    tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == PHASE_CONTROL_PROHIBIT || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValue[0] = m_tAscParamInfo.m_stAscPhaseTable[byPhaseIndex].m_byForbiddenFlag;
			memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		    tCorrectReturnData.m_byReturnCount += 1;
		}
	}
	else
	{
		tDataConfig.m_byDataValue[0] = BAD_VALUE_NULL;
		memcpy(&tWrongReturnData.m_tDataConfig[tWrongReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		tWrongReturnData.m_byReturnCount += 1;
	}
}

void  COpenATCCommWithGB20999Thread::QueryDetectorCount(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData)
{
	TDataConfig tDataConfig;
	tDataConfig.m_byIndex = byIndex + 1;
	tDataConfig.m_byDataClassID = m_chUnPackedBuff[C_N_DATACLASS_ID_POS + byIndex * 6];
	tDataConfig.m_byObjectID = m_chUnPackedBuff[C_N_OBJECT_ID_POS + byIndex * 6];
	tDataConfig.m_byAttributeID = m_chUnPackedBuff[C_N_ATTRIBUTE_ID_POS + byIndex * 6];
	tDataConfig.m_byElementID = m_chUnPackedBuff[C_N_ELEMENT_ID_POS + byIndex * 6];
	tDataConfig.m_byDataValueLength = 1;
	tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;

	tDataConfig.m_byDataValue[0] = m_tAscParamInfo.m_stVehicleDetectorTableValidSize;
	memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
	tCorrectReturnData.m_byReturnCount += 1;
}

void  COpenATCCommWithGB20999Thread::QueryAllElementDetectorConfig(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData)
{
	TVehDetBoardData tVehDetData;
    m_pOpenATCRunStatus->GetVehDetBoardData(tVehDetData);

	BYTE  byDetectorType = DETECTOR_TYPE_COIL;
	short nFlowCycle = 5 * 60;
	short nOccupyCycle = 5 * 60;

	for (int i = 0;i < m_tAscParamInfo.m_stVehicleDetectorTableValidSize;i++)
	{
		if (m_tAscParamInfo.m_stAscVehicleDetectorTable[i].m_byDetectorType == LOOP_DETECTOR)
		{
			byDetectorType = DETECTOR_TYPE_COIL;
		}
		else
		{
			byDetectorType = DETECTOR_TYPE_VIDEO;
		}
		if (m_tAscParamInfo.m_stAscVehicleDetectorTable[i].m_wFlowGatherCycle < 10)
		{
			m_tAscParamInfo.m_stAscVehicleDetectorTable[i].m_wFlowGatherCycle = 10;
		}
		if (m_tAscParamInfo.m_stAscVehicleDetectorTable[i].m_wOccupancyGatherCycle < 10)
		{
			m_tAscParamInfo.m_stAscVehicleDetectorTable[i].m_wOccupancyGatherCycle = 10;
		}

		nFlowCycle = ntohs(m_tAscParamInfo.m_stAscVehicleDetectorTable[i].m_wFlowGatherCycle);
		nOccupyCycle = ntohs(m_tAscParamInfo.m_stAscVehicleDetectorTable[i].m_wOccupancyGatherCycle);
	
		TDataConfig tDataConfig;
		//tDataConfig.m_byIndex = byIndex + 1;
		tDataConfig.m_byIndex = i + 1;	//HPH 2021.12.07
		tDataConfig.m_byDataClassID = m_chUnPackedBuff[C_N_DATACLASS_ID_POS + byIndex * 6];
		tDataConfig.m_byObjectID = m_chUnPackedBuff[C_N_OBJECT_ID_POS + byIndex * 6];
		tDataConfig.m_byAttributeID = m_chUnPackedBuff[C_N_ATTRIBUTE_ID_POS + byIndex * 6];
		tDataConfig.m_byElementID = i + 1;

		if (tDataConfig.m_byAttributeID == DETECTOR_ID_CONFIG || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValueLength = 1;
	        tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			tDataConfig.m_byDataValue[0] = tDataConfig.m_byElementID;
			memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
	        tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == DETECTOR_TYPE || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValueLength = 1;
	        tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			tDataConfig.m_byDataValue[0] = byDetectorType;
			memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
	        tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == FLOW_CYCLE || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValueLength = 2;
	        tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			memcpy(tDataConfig.m_byDataValue, &nFlowCycle, 2);
			memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
	        tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == OCCUPY_CYCLE || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValueLength = 2;
	        tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			memcpy(tDataConfig.m_byDataValue, &nOccupyCycle, 2);
			memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
	        tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == INSTALL_POS || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValueLength = 128;
	        tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			memcpy(tDataConfig.m_byDataValue, &m_tAscParamInfo.m_stAscVehicleDetectorTable[i].m_chFixPosition, 128);
			memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
	        tCorrectReturnData.m_byReturnCount += 1;
		}
	}
}

void  COpenATCCommWithGB20999Thread::QueryAllElementDetectorData(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData)
{
	TVehDetBoardData tVehDetData;
    m_pOpenATCRunStatus->GetVehDetBoardData(tVehDetData);

	bool  bExistStatus = false;;
	BYTE  bySpeed = 0;
	BYTE  byVehicleType = VEHICLE_TYPE_SMALL;
	BYTE  byPlate[C_N_MAX_PLATE_LENGTH];
	memset(byPlate, 0, sizeof(byPlate));
	short nQueueLength = 0;

	for (int i = 0;i < m_tAscParamInfo.m_stVehicleDetectorTableValidSize;i++)
	{
		if (i < MAX_VEHICLEDETECTOR_COUNT)
		{
			if (tVehDetData.m_atVehDetData[i / C_N_MAXDETINPUT_NUM].m_achVehChgVal[i % C_N_MAXDETINPUT_NUM] == 1)
			{
				bExistStatus = true;
				break;
			}
		}

		TDataConfig tDataConfig;
		//tDataConfig.m_byIndex = byIndex + 1;
		tDataConfig.m_byIndex = i + 1;	//HPH 2021.12.07
		tDataConfig.m_byDataClassID = m_chUnPackedBuff[C_N_DATACLASS_ID_POS + byIndex * 6];
		tDataConfig.m_byObjectID = m_chUnPackedBuff[C_N_OBJECT_ID_POS + byIndex * 6];
		tDataConfig.m_byAttributeID = m_chUnPackedBuff[C_N_ATTRIBUTE_ID_POS + byIndex * 6];
		tDataConfig.m_byElementID = i + 1;

		if (tDataConfig.m_byAttributeID == DETECTOR_ID_DATA || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValueLength = 1;
	        tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			tDataConfig.m_byDataValue[0] = tDataConfig.m_byElementID;
			memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
	        tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == EXIST_STATUS || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValueLength = 1;
	        tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			tDataConfig.m_byDataValue[0] = bExistStatus;
			memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
	        tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == VEHICLE_SPEED || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValueLength = 1;
	        tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			tDataConfig.m_byDataValue[0] = bySpeed;
			memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
	        tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == VEHICLE_TYPE || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValueLength = 1;
	        tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			tDataConfig.m_byDataValue[0] = byVehicleType;
			memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
	        tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == VEHICLE_PLATE || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValueLength = C_N_MAX_PLATE_LENGTH;
	        tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			memcpy(tDataConfig.m_byDataValue, byPlate, C_N_MAX_PLATE_LENGTH);
			memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
	        tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == QUEUE_LENGTH || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValueLength = 2;
	        tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			memcpy(tDataConfig.m_byDataValue, &nQueueLength, 2);
			memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
	        tCorrectReturnData.m_byReturnCount += 1;
		}
	}
}

void  COpenATCCommWithGB20999Thread::QueryDetectorConfig(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData)
{
	TDataConfig tDataConfig;
	tDataConfig.m_byIndex = byIndex + 1;
	tDataConfig.m_byDataClassID = m_chUnPackedBuff[C_N_DATACLASS_ID_POS + byIndex * 6];
	tDataConfig.m_byObjectID = m_chUnPackedBuff[C_N_OBJECT_ID_POS + byIndex * 6];
	tDataConfig.m_byAttributeID = m_chUnPackedBuff[C_N_ATTRIBUTE_ID_POS + byIndex * 6];
	tDataConfig.m_byElementID = m_chUnPackedBuff[C_N_ELEMENT_ID_POS + byIndex * 6];
	tDataConfig.m_byDataValueLength = 1;
	tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;

	if (tDataConfig.m_byElementID < 1 || tDataConfig.m_byElementID > 64)
	{
		tDataConfig.m_byDataValue[0] = BAD_VALUE_NULL;
		memcpy(&tWrongReturnData.m_tDataConfig[tWrongReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		tWrongReturnData.m_byReturnCount += 1;
		return;
	}

	TVehDetBoardData tVehDetData;
    m_pOpenATCRunStatus->GetVehDetBoardData(tVehDetData);

	int  byDetectorIndex = -1;
	BYTE byDetectorType = 0;
	for (int i = 0;i < m_tAscParamInfo.m_stVehicleDetectorTableValidSize;i++)
	{
		if (m_tAscParamInfo.m_stAscVehicleDetectorTable[i].m_byVehicleDetectorNumber == tDataConfig.m_byElementID)
		{
			//if (m_tAscParamInfo.m_stAscVehicleDetectorTable[i].m_byDetectorType == LOOP_DETECTOR)
			//{
			//	byDetectorType = DETECTOR_TYPE_COIL;
			//}
			//else
			//{
			//	byDetectorType = DETECTOR_TYPE_VIDEO;
			//}
			byDetectorType = m_tAscParamInfo.m_stAscVehicleDetectorTable[i].m_byDetectorType;
			byDetectorIndex = i;
			break;
		}
	}

	if (byDetectorIndex != -1)
	{
		if (tDataConfig.m_byAttributeID == DETECTOR_ID_CONFIG || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValueLength = 1;
	        tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			tDataConfig.m_byDataValue[0] = tDataConfig.m_byElementID;
			memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
	        tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == DETECTOR_TYPE || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValueLength = 1;
	        tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			tDataConfig.m_byDataValue[0] = byDetectorType;
			memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
	        tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == FLOW_CYCLE || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValueLength = 2;
	        tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			if (m_tAscParamInfo.m_stAscVehicleDetectorTable[byDetectorIndex].m_wFlowGatherCycle < 10)
			{
				m_tAscParamInfo.m_stAscVehicleDetectorTable[byDetectorIndex].m_wFlowGatherCycle = 10;
			}
			short nTemp = ntohs(m_tAscParamInfo.m_stAscVehicleDetectorTable[byDetectorIndex].m_wFlowGatherCycle);
			memcpy(tDataConfig.m_byDataValue, &nTemp, 2);
			memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
	        tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == OCCUPY_CYCLE || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValueLength = 2;
	        tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			if (m_tAscParamInfo.m_stAscVehicleDetectorTable[byDetectorIndex].m_wOccupancyGatherCycle < 10)
			{
				m_tAscParamInfo.m_stAscVehicleDetectorTable[byDetectorIndex].m_wOccupancyGatherCycle = 10;
			}
			short nTemp = ntohs(m_tAscParamInfo.m_stAscVehicleDetectorTable[byDetectorIndex].m_wOccupancyGatherCycle);
			memcpy(tDataConfig.m_byDataValue, &nTemp, 2);
			memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
	        tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == INSTALL_POS || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValueLength = MAX_MODULE_STRING_LENGTH;
	        tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			memcpy(tDataConfig.m_byDataValue, m_tAscParamInfo.m_stAscVehicleDetectorTable[byDetectorIndex].m_chFixPosition, MAX_MODULE_STRING_LENGTH);
			memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
	        tCorrectReturnData.m_byReturnCount += 1;
		}
	}
	else
	{
		tDataConfig.m_byDataValue[0] = BAD_VALUE_NULL;
		memcpy(&tWrongReturnData.m_tDataConfig[tWrongReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		tWrongReturnData.m_byReturnCount += 1;
	}
}

void  COpenATCCommWithGB20999Thread::QueryDetectorData(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData)
{
	TDataConfig tDataConfig;
	tDataConfig.m_byIndex = byIndex + 1;
	tDataConfig.m_byDataClassID = m_chUnPackedBuff[C_N_DATACLASS_ID_POS + byIndex * 6];
	tDataConfig.m_byObjectID = m_chUnPackedBuff[C_N_OBJECT_ID_POS + byIndex * 6];
	tDataConfig.m_byAttributeID = m_chUnPackedBuff[C_N_ATTRIBUTE_ID_POS + byIndex * 6];
	tDataConfig.m_byElementID = m_chUnPackedBuff[C_N_ELEMENT_ID_POS + byIndex * 6];

	if (tDataConfig.m_byElementID < 1 || tDataConfig.m_byElementID > 64)
	{
		tDataConfig.m_byDataValueLength = 1;
	    tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
		tDataConfig.m_byDataValue[0] = BAD_VALUE_NULL;
		memcpy(&tWrongReturnData.m_tDataConfig[tWrongReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		tWrongReturnData.m_byReturnCount += 1;
		return;
	}

	int  byDetectorIndex = -1;
	for (int i = 0;i < m_tAscParamInfo.m_stVehicleDetectorTableValidSize;i++)
	{
		if (m_tAscParamInfo.m_stAscVehicleDetectorTable[i].m_byVehicleDetectorNumber == tDataConfig.m_byElementID)
		{
			byDetectorIndex = i;
			break;
		}
	}

	if (byDetectorIndex == -1)
	{
		tDataConfig.m_byDataValueLength = 1;
	    tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
		tDataConfig.m_byDataValue[0] = BAD_VALUE_NULL;
		memcpy(&tWrongReturnData.m_tDataConfig[tWrongReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		tWrongReturnData.m_byReturnCount += 1;
		return;
	}

	TVehDetBoardData tVehDetData;
    m_pOpenATCRunStatus->GetVehDetBoardData(tVehDetData);

	bool bExistStatus = false;
	BYTE bySpeed = 0;
	BYTE byVehicleType = VEHICLE_TYPE_SMALL;
	BYTE byPlate[C_N_MAX_PLATE_LENGTH];
	memset(byPlate, 0, sizeof(byPlate));
	short nQueueLength = 0;

	if (tDataConfig.m_byAttributeID == DETECTOR_ID_DATA || tDataConfig.m_byAttributeID == 0)
	{
		tDataConfig.m_byDataValueLength = 1;
	    tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
		tDataConfig.m_byDataValue[0] = tDataConfig.m_byElementID;
		memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
	    tCorrectReturnData.m_byReturnCount += 1;
	}
	if (tDataConfig.m_byAttributeID == EXIST_STATUS || tDataConfig.m_byAttributeID == 0)
	{
		if ((tDataConfig.m_byElementID - 1) < MAX_VEHICLEDETECTOR_COUNT)
		{
			if (tVehDetData.m_atVehDetData[(tDataConfig.m_byElementID - 1) / C_N_MAXDETINPUT_NUM].m_achVehChgVal[(tDataConfig.m_byElementID - 1) % C_N_MAXDETINPUT_NUM] == 1)
			{
				bExistStatus = true;
			}
		}
		tDataConfig.m_byDataValueLength = 1;
	    tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
		tDataConfig.m_byDataValue[0] = bExistStatus;
		memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
	    tCorrectReturnData.m_byReturnCount += 1;
	}
	if (tDataConfig.m_byAttributeID == VEHICLE_SPEED || tDataConfig.m_byAttributeID == 0)
	{
		tDataConfig.m_byDataValueLength = 1;
	    tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
		tDataConfig.m_byDataValue[0] = bySpeed;
		memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
	    tCorrectReturnData.m_byReturnCount += 1;
	}
	if (tDataConfig.m_byAttributeID == VEHICLE_TYPE || tDataConfig.m_byAttributeID == 0)
	{
		tDataConfig.m_byDataValueLength = 1;
	    tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
		tDataConfig.m_byDataValue[0] = byVehicleType;
		memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
	    tCorrectReturnData.m_byReturnCount += 1;
	}
	if (tDataConfig.m_byAttributeID == VEHICLE_PLATE || tDataConfig.m_byAttributeID == 0)
	{
		tDataConfig.m_byDataValueLength = C_N_MAX_PLATE_LENGTH;
	    tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
		memcpy(tDataConfig.m_byDataValue, byPlate, C_N_MAX_PLATE_LENGTH);
		memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
	    tCorrectReturnData.m_byReturnCount += 1;
	}
	if (tDataConfig.m_byAttributeID == QUEUE_LENGTH || tDataConfig.m_byAttributeID == 0)
	{
		tDataConfig.m_byDataValueLength = 2;
	    tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
		memcpy(tDataConfig.m_byDataValue, &nQueueLength, 2);
		memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
	    tCorrectReturnData.m_byReturnCount += 1;
	}
}

void COpenATCCommWithGB20999Thread::QueryPhaseStageCount(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData)
{
	TDataConfig tDataConfig;
	tDataConfig.m_byIndex = byIndex + 1;
	tDataConfig.m_byDataClassID = m_chUnPackedBuff[C_N_DATACLASS_ID_POS + byIndex * 6];
	tDataConfig.m_byObjectID = m_chUnPackedBuff[C_N_OBJECT_ID_POS + byIndex * 6];
	tDataConfig.m_byAttributeID = m_chUnPackedBuff[C_N_ATTRIBUTE_ID_POS + byIndex * 6];
	tDataConfig.m_byElementID = m_chUnPackedBuff[C_N_ELEMENT_ID_POS + byIndex * 6];
	tDataConfig.m_byDataValueLength = 1;
	tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;

	tDataConfig.m_byDataValue[0] = m_tAscParamInfo.m_stPhaseStageValidSize;
	memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
	tCorrectReturnData.m_byReturnCount += 1;
}

void COpenATCCommWithGB20999Thread::QueryAllElementPhaseStageConfig(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData)
{
	for (int i = 0;i < m_tAscParamInfo.m_stPhaseStageValidSize;i++)
	{
		TDataConfig tDataConfig;
		//tDataConfig.m_byIndex = byIndex + 1;
		tDataConfig.m_byIndex = i + 1;	//HPH 2021.12.07
		tDataConfig.m_byDataClassID = m_chUnPackedBuff[C_N_DATACLASS_ID_POS + byIndex * 6];
		tDataConfig.m_byObjectID = m_chUnPackedBuff[C_N_OBJECT_ID_POS + byIndex * 6];
		tDataConfig.m_byAttributeID = m_chUnPackedBuff[C_N_ATTRIBUTE_ID_POS + byIndex * 6];
		tDataConfig.m_byElementID = i + 1;
		tDataConfig.m_byDataValueLength = 1;
		tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;

		if (tDataConfig.m_byAttributeID == PHASE_STAGE_ID_CONFIG || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValueLength = 1;
	        tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			tDataConfig.m_byDataValue[0] = tDataConfig.m_byElementID;
			memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
	        tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == PHASE_STAGE_PHASE || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValueLength = 8;
	        tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			for (int nValueIndex = 0; nValueIndex < 8;nValueIndex++)
			{
				tDataConfig.m_byDataValue[nValueIndex] = m_tAscParamInfo.m_stPhaseStage[i].m_byPhase[nValueIndex];
			}
			memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
	        tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == PHASE_STAGE_LATE_START || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValueLength = C_N_MAX_PHASE_COUNT;
	        tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			for (int nValueIndex = 0; nValueIndex < C_N_MAX_PHASE_COUNT;nValueIndex++)
			{
				tDataConfig.m_byDataValue[nValueIndex] = m_tAscParamInfo.m_stPhaseStage[i].m_byLaterStartTime[C_N_MAX_PHASE_COUNT - nValueIndex - 1];
			}
			memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
	        tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == PHASE_STAGE_EARLY_END || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValueLength = C_N_MAX_PHASE_COUNT;
	        tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			for (int nValueIndex = 0; nValueIndex < C_N_MAX_PHASE_COUNT;nValueIndex++)
			{
				tDataConfig.m_byDataValue[nValueIndex] = m_tAscParamInfo.m_stPhaseStage[i].m_byEarlyEndTime[C_N_MAX_PHASE_COUNT - nValueIndex - 1];
			}
			memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
	        tCorrectReturnData.m_byReturnCount += 1;
		}
	}
}

void COpenATCCommWithGB20999Thread::QueryAllElementPhaseStageStatus(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData)
{
	m_pOpenATCRunStatus->SetPhaseRunStatusWriteFlag(true);
	OpenATCSleep(100);
    bool bIsReadFlag = true;
	if (m_pOpenATCRunStatus->GetPhaseRunStatusReadFlag())
	{
		m_pOpenATCRunStatus->SetPhaseRunStatusReadFlag(false);
		m_pOpenATCRunStatus->SetPhaseRunStatusWriteFlag(false);
		m_pOpenATCRunStatus->SetPhaseRunStatusReadFlag(true);
		m_pOpenATCRunStatus->SetPhaseRunStatusWriteFlag(true);
		bIsReadFlag = false;
	}
	else
	{
		OpenATCSleep(100);
		if (m_pOpenATCRunStatus->GetPhaseRunStatusReadFlag())
		{
			m_pOpenATCRunStatus->SetPhaseRunStatusReadFlag(false);
			m_pOpenATCRunStatus->SetPhaseRunStatusWriteFlag(false);
			m_pOpenATCRunStatus->SetPhaseRunStatusReadFlag(true);
			m_pOpenATCRunStatus->SetPhaseRunStatusWriteFlag(true);
			bIsReadFlag = false;
		}
	}

	TPhaseRunStatus tPhaseRunStatus;
	memset(&tPhaseRunStatus, 0x00, sizeof(tPhaseRunStatus));
	m_pOpenATCRunStatus->GetPhaseRunStatus(tPhaseRunStatus);

	BYTE  byStageStatus = PHASE_STAGE_STATUS_TRANSITON;
	short nRunTime = 0;
	short nRemainTime = 0;

	for (int i = 0;i < m_tAscParamInfo.m_stPhaseStageValidSize;i++)
	{
		TDataConfig tDataConfig;
		//tDataConfig.m_byIndex = byIndex + 1;
		tDataConfig.m_byIndex = i + 1;	//HPH 2021.12.07
		tDataConfig.m_byDataClassID = m_chUnPackedBuff[C_N_DATACLASS_ID_POS + byIndex * 6];
		tDataConfig.m_byObjectID = m_chUnPackedBuff[C_N_OBJECT_ID_POS + byIndex * 6];
		tDataConfig.m_byAttributeID = m_chUnPackedBuff[C_N_ATTRIBUTE_ID_POS + byIndex * 6];
		tDataConfig.m_byElementID = i + 1;
		tDataConfig.m_byDataValueLength = 1;
		tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;

		/*数据准备*/
		//路口2
		//这里得先把阶段链和对应运行阶段对应起来[路口1的]

		bool bFlag = false;
		bool bTwoFlag = false;
		BYTE byPhaseStageIndex = 0;
		BYTE byTwoPhaseStageIndex = 0;
		int  j = 0;
		for (j = 0; j < 16; j++)
		{
			if (m_tAscParamInfo.m_stAscPatternTable[tPhaseRunStatus.m_byPlanID - 1].m_byPatternStage[j] == i+1 && m_tAscParamInfo.m_stAscPatternTable[tPhaseRunStatus.m_byPlanID - 1].m_byPatternStage[j] != 0)
			{
				bFlag = true;
				byPhaseStageIndex = j;
				break;
			}
		}

		/*数据准备*/
		//计算路口1运行时间
		GetStageRunTime(nRunTime, nRemainTime, byPhaseStageIndex, tPhaseRunStatus);

		if (tDataConfig.m_byAttributeID == PHASE_STAGE_ID_STATUS || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValueLength = 1;
			tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			tDataConfig.m_byDataValue[0] = i + 1;
			memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == PHASE_STAGE_STATUS || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValueLength = 1;
			tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			tDataConfig.m_byDataValue[0] = TransStageStatus(tPhaseRunStatus.m_atRingRunStatus[0].m_atPhaseStatus[byPhaseStageIndex].m_chPhaseStatus);	
			//tDataConfig.m_byDataValue[0] = StageType[tDataConfig.m_byElementID];
			memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == PHASE_STAGE_RUN_TIME || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValueLength = 2;
			tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			short nTemp;
			nTemp = ntohs(nRunTime);
			memcpy(tDataConfig.m_byDataValue, &nTemp, 2);
			memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == PHASE_STAGE_REMAIN_TIME || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValueLength = 2;
			tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			short tTemp;
			tTemp = ntohs(nRemainTime);
			memcpy(tDataConfig.m_byDataValue, &tTemp, 2);
			memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
		}
	}
}

void COpenATCCommWithGB20999Thread::QueryAllElementPhaseStageControl(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData)
{
	for (int i = 0;i < m_tAscParamInfo.m_stPhaseStageValidSize;i++)
	{
		TDataConfig tDataConfig;
		//tDataConfig.m_byIndex = byIndex + 1;
		tDataConfig.m_byIndex = i + 1;	//HPH 2021.12.07
		tDataConfig.m_byDataClassID = m_chUnPackedBuff[C_N_DATACLASS_ID_POS + byIndex * 6];
		tDataConfig.m_byObjectID = m_chUnPackedBuff[C_N_OBJECT_ID_POS + byIndex * 6];
		tDataConfig.m_byAttributeID = m_chUnPackedBuff[C_N_ATTRIBUTE_ID_POS + byIndex * 6];
		tDataConfig.m_byElementID = i + 1;
		tDataConfig.m_byDataValueLength = 1;
		tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;

		if (tDataConfig.m_byAttributeID == PHASE_STAGE_ID_CONTROL || tDataConfig.m_byAttributeID == 0)
		{
            tDataConfig.m_byDataValue[0] = tDataConfig.m_byElementID;
			memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
	        tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == PHASE_STAGE_SOFTWARECALL || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValue[0] = m_tAscParamInfo.m_stPhaseStage[i].m_bSoftCall;
			memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
	        tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == PHASE_STAGE_SHIELD || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValue[0] = m_tAscParamInfo.m_stPhaseStage[i].m_bScreen;
			memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
	        tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == PHASE_STAGE_PROHIBIT || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValue[0] = m_tAscParamInfo.m_stPhaseStage[i].m_bForbidden;
			memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
	        tCorrectReturnData.m_byReturnCount += 1;
		}
	}
}

void  COpenATCCommWithGB20999Thread::QueryPhaseStageConfig(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData)
{
	TDataConfig tDataConfig;
	tDataConfig.m_byIndex = byIndex + 1;
	tDataConfig.m_byDataClassID = m_chUnPackedBuff[C_N_DATACLASS_ID_POS + byIndex * 6];
	tDataConfig.m_byObjectID = m_chUnPackedBuff[C_N_OBJECT_ID_POS + byIndex * 6];
	tDataConfig.m_byAttributeID = m_chUnPackedBuff[C_N_ATTRIBUTE_ID_POS + byIndex * 6];
	tDataConfig.m_byElementID = m_chUnPackedBuff[C_N_ELEMENT_ID_POS + byIndex * 6];

	if (tDataConfig.m_byElementID < 1 || tDataConfig.m_byElementID > 64)
	{
		tDataConfig.m_byDataValueLength = 1;
	    tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
		tDataConfig.m_byDataValue[0] = BAD_VALUE_NULL;
		memcpy(&tWrongReturnData.m_tDataConfig[tWrongReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		tWrongReturnData.m_byReturnCount += 1;
		return;
	}
  
	bool bFlag = false;
	BYTE byPhaseStageIndex = 0;
	int  i = 0;
	for (i = 0;i < m_tAscParamInfo.m_stPhaseStageValidSize;i++)
	{
		if (m_tAscParamInfo.m_stPhaseStage[i].m_byPhaseStageNumber == tDataConfig.m_byElementID)
		{
			bFlag = true;
			byPhaseStageIndex = i;
			break;
		}
	}

	if (bFlag)
	{
		if (tDataConfig.m_byAttributeID == PHASE_STAGE_ID_CONFIG || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValueLength = 1;
			tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			tDataConfig.m_byDataValue[0] = tDataConfig.m_byElementID;
			memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == PHASE_STAGE_PHASE || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValueLength = 8;
	        tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			for (i = 0;i < 8;i++)
			{
				tDataConfig.m_byDataValue[i] = m_tAscParamInfo.m_stPhaseStage[byPhaseStageIndex].m_byPhase[i];
			}
			memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
			
		}
		if (tDataConfig.m_byAttributeID == PHASE_STAGE_LATE_START || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValueLength = C_N_MAX_PHASE_COUNT;
	        tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			for (i = 0;i < C_N_MAX_PHASE_COUNT;i++)
			{
				tDataConfig.m_byDataValue[i] = m_tAscParamInfo.m_stPhaseStage[byPhaseStageIndex].m_byLaterStartTime[C_N_MAX_PHASE_COUNT - i - 1];
			}
			memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
	        tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == PHASE_STAGE_EARLY_END || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValueLength = C_N_MAX_PHASE_COUNT;
	        tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			for (i = 0;i < C_N_MAX_PHASE_COUNT;i++)
			{
				tDataConfig.m_byDataValue[i] = m_tAscParamInfo.m_stPhaseStage[byPhaseStageIndex].m_byEarlyEndTime[C_N_MAX_PHASE_COUNT - i - 1];
			}
			memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
	        tCorrectReturnData.m_byReturnCount += 1;
		}	
	}
	else
	{
		tDataConfig.m_byDataValue[0] = BAD_VALUE_NULL;
		memcpy(&tWrongReturnData.m_tDataConfig[tWrongReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		tWrongReturnData.m_byReturnCount += 1;
	}
}

void  COpenATCCommWithGB20999Thread::QueryPhaseStageStatus(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData)
{
	TDataConfig tDataConfig;
	tDataConfig.m_byIndex = byIndex + 1;
	tDataConfig.m_byDataClassID = m_chUnPackedBuff[C_N_DATACLASS_ID_POS + byIndex * 6];
	tDataConfig.m_byObjectID = m_chUnPackedBuff[C_N_OBJECT_ID_POS + byIndex * 6];
	tDataConfig.m_byAttributeID = m_chUnPackedBuff[C_N_ATTRIBUTE_ID_POS + byIndex * 6];
	tDataConfig.m_byElementID = m_chUnPackedBuff[C_N_ELEMENT_ID_POS + byIndex * 6];
	tDataConfig.m_byDataValueLength = 1;
	tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;

	if (tDataConfig.m_byElementID < 1 || tDataConfig.m_byElementID > 64)
	{
		tDataConfig.m_byDataValue[0] = BAD_VALUE_OVERFLOW;
		memcpy(&tWrongReturnData.m_tDataConfig[tWrongReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		tWrongReturnData.m_byReturnCount += 1;
		return;
	}

	m_pOpenATCRunStatus->SetPhaseRunStatusWriteFlag(true);
	OpenATCSleep(100);
    bool bIsReadFlag = true;
	if (m_pOpenATCRunStatus->GetPhaseRunStatusReadFlag())
	{
		m_pOpenATCRunStatus->SetPhaseRunStatusReadFlag(false);
		m_pOpenATCRunStatus->SetPhaseRunStatusWriteFlag(false);
		m_pOpenATCRunStatus->SetPhaseRunStatusReadFlag(true);
		m_pOpenATCRunStatus->SetPhaseRunStatusWriteFlag(true);
		bIsReadFlag = false;
	}
	else
	{
		OpenATCSleep(100);
		if (m_pOpenATCRunStatus->GetPhaseRunStatusReadFlag())
		{
			m_pOpenATCRunStatus->SetPhaseRunStatusReadFlag(false);
			m_pOpenATCRunStatus->SetPhaseRunStatusWriteFlag(false);
			m_pOpenATCRunStatus->SetPhaseRunStatusReadFlag(true);
			m_pOpenATCRunStatus->SetPhaseRunStatusWriteFlag(true);
			bIsReadFlag = false;
		}
	}

	TPhaseRunStatus tPhaseRunStatus;
	memset(&tPhaseRunStatus, 0x00, sizeof(tPhaseRunStatus));
	m_pOpenATCRunStatus->GetPhaseRunStatus(tPhaseRunStatus);

	////获取路口2的方案阶段信息
	//TRoadTwoRunStatus tRoadTwoRunStatus;
	//memset(&tRoadTwoRunStatus,0,sizeof(tRoadTwoRunStatus));
	//m_pOpenATCRunStatus->GetRoadTwoRunStatus(tRoadTwoRunStatus);

	BYTE  byStageStatus = 0;
	short nRunTime = 0;
	short nRemainTime = 0;

	//这里得先把阶段链和对应运行阶段对应起来[路口1的]
	bool bFlag = false;		//所查询的是路口1
	bool bTwoFlag = false;	//所查询的是路口2
	BYTE byPhaseStageIndex = 0;
	int  i = 0;
	//获取运行阶段索引
	for (i = 0;i < 16;i++)
	{
		if (m_tAscParamInfo.m_stAscPatternTable[tPhaseRunStatus.m_byPlanID - 1].m_byPatternStage[i] == tDataConfig.m_byElementID && m_tAscParamInfo.m_stAscPatternTable[tPhaseRunStatus.m_byPlanID - 1].m_byPatternStage[i] != 0)
		{
			bFlag = true;				//路口1
			byPhaseStageIndex = i;
			break;
		}
	}

	//for(i = 0;i < MAX_PHASE_COUNT_20999;i++)
	//{
	//	if(m_tAscParamInfo.m_stAscPatternTable[tRoadTwoRunStatus.m_nPatternNo - 1].m_byPatternStage[i] == tDataConfig.m_byElementID && m_tAscParamInfo.m_stAscPatternTable[tRoadTwoRunStatus.m_nPatternNo - 1].m_byPatternStage[i] != 0)
	//	{
	//		bTwoFlag = true;				//路口1
	//		byPhaseStageIndex = i;
	//		break;
	//	}
	//}

	if (bFlag)	//路口1
	{
		GetStageRunTime(nRunTime, nRemainTime, byPhaseStageIndex, tPhaseRunStatus);

		if (tDataConfig.m_byAttributeID == PHASE_STAGE_ID_STATUS || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValueLength = 1;
			tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			tDataConfig.m_byDataValue[0] = tDataConfig.m_byElementID;
			memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == PHASE_STAGE_STATUS || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValueLength = 1;
			tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			tDataConfig.m_byDataValue[0] = TransStageStatus(tPhaseRunStatus.m_atRingRunStatus[0].m_atPhaseStatus[byPhaseStageIndex].m_chPhaseStatus);	
			//tDataConfig.m_byDataValue[0] = StageType[tDataConfig.m_byElementID];
			memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == PHASE_STAGE_RUN_TIME || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValueLength = 2;
			tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			short nTemp;
			nTemp = ntohs(nRunTime);
			memcpy(tDataConfig.m_byDataValue, &nTemp, 2);
			memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == PHASE_STAGE_REMAIN_TIME || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValueLength = 2;
			tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			short tTemp;
			tTemp = ntohs(nRemainTime);
			memcpy(tDataConfig.m_byDataValue, &tTemp, 2);
			memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
		}
	}
	//else if(bTwoFlag)	//路口2
	//{
	//	GetRoadTwoStageRunTime(nRunTime, nRemainTime, byStageStatus, byPhaseStageIndex);

	//	if (tDataConfig.m_byAttributeID == PHASE_STAGE_ID_STATUS || tDataConfig.m_byAttributeID == 0)
	//	{
	//		tDataConfig.m_byDataValueLength = 1;
	//		tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
	//		tDataConfig.m_byDataValue[0] = tDataConfig.m_byElementID;
	//		memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
	//		tCorrectReturnData.m_byReturnCount += 1;
	//	}
	//	if (tDataConfig.m_byAttributeID == PHASE_STAGE_STATUS || tDataConfig.m_byAttributeID == 0)
	//	{
	//		tDataConfig.m_byDataValueLength = 1;
	//		tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
	//		tDataConfig.m_byDataValue[0] = TransStageStatus(tPhaseRunStatus.m_atRingRunStatus[0].m_atPhaseStatus[byPhaseStageIndex].m_chPhaseStatus);	
	//		//tDataConfig.m_byDataValue[0] = StageType[tDataConfig.m_byElementID];
	//		memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
	//		tCorrectReturnData.m_byReturnCount += 1;
	//	}
	//	if (tDataConfig.m_byAttributeID == PHASE_STAGE_RUN_TIME || tDataConfig.m_byAttributeID == 0)
	//	{
	//		tDataConfig.m_byDataValueLength = 2;
	//		tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
	//		short nTemp;
	//		nTemp = ntohs(nRunTime);
	//		memcpy(tDataConfig.m_byDataValue, &nTemp, 2);
	//		memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
	//		tCorrectReturnData.m_byReturnCount += 1;
	//	}
	//	if (tDataConfig.m_byAttributeID == PHASE_STAGE_REMAIN_TIME || tDataConfig.m_byAttributeID == 0)
	//	{
	//		tDataConfig.m_byDataValueLength = 2;
	//		tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
	//		short tTemp;
	//		tTemp = ntohs(nRemainTime);
	//		memcpy(tDataConfig.m_byDataValue, &tTemp, 2);
	//		memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
	//		tCorrectReturnData.m_byReturnCount += 1;
	//	}
	//}
	else
	{
		tDataConfig.m_byDataValue[0] = BAD_VALUE_NULL;
		memcpy(&tWrongReturnData.m_tDataConfig[tWrongReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		tWrongReturnData.m_byReturnCount += 1;
	}
}

void  COpenATCCommWithGB20999Thread::QueryPhaseStageControl(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData)
{
	TDataConfig tDataConfig;
	tDataConfig.m_byIndex = byIndex + 1;
	tDataConfig.m_byDataClassID = m_chUnPackedBuff[C_N_DATACLASS_ID_POS + byIndex * 6];
	tDataConfig.m_byObjectID = m_chUnPackedBuff[C_N_OBJECT_ID_POS + byIndex * 6];
	tDataConfig.m_byAttributeID = m_chUnPackedBuff[C_N_ATTRIBUTE_ID_POS + byIndex * 6];
	tDataConfig.m_byElementID = m_chUnPackedBuff[C_N_ELEMENT_ID_POS + byIndex * 6];
	tDataConfig.m_byDataValueLength = 1;
	tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;

	if (tDataConfig.m_byElementID < 1 || tDataConfig.m_byElementID > 64)
	{
		tDataConfig.m_byDataValue[0] = BAD_VALUE_NULL;
		memcpy(&tWrongReturnData.m_tDataConfig[tWrongReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		tWrongReturnData.m_byReturnCount += 1;
		return;
	}

	bool bFlag = false;
	BYTE byPhaseStageIndex = 0;
	for (int i = 0;i < m_tAscParamInfo.m_stPhaseStageValidSize;i++)
	{
		if (m_tAscParamInfo.m_stPhaseStage[i].m_byPhaseStageNumber == tDataConfig.m_byElementID)
		{
			bFlag = true;
			byPhaseStageIndex = i;
			break;
		}
	}

	if (bFlag)
	{
		if (tDataConfig.m_byAttributeID == PHASE_STAGE_ID_CONTROL || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValue[0] = tDataConfig.m_byElementID;
			memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == PHASE_STAGE_SOFTWARECALL || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValue[0] = m_tAscParamInfo.m_stPhaseStage[byPhaseStageIndex].m_bSoftCall;
			memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == PHASE_STAGE_SHIELD || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValue[0] = m_tAscParamInfo.m_stPhaseStage[byPhaseStageIndex].m_bScreen;
			memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == PHASE_STAGE_PROHIBIT || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValue[0] = m_tAscParamInfo.m_stPhaseStage[byPhaseStageIndex].m_bForbidden;
			memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
		}
	}
	else
	{
		tDataConfig.m_byDataValue[0] = BAD_VALUE_NULL;
		memcpy(&tWrongReturnData.m_tDataConfig[tWrongReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		tWrongReturnData.m_byReturnCount += 1;
	}
}

void COpenATCCommWithGB20999Thread::QueryAllElementPhaseeConflictInfo(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData)
{
	for (int i = 0; i < m_tAscParamInfo.m_stPhaseConflictValidSize;i++)
	{	
		TDataConfig tDataConfig;
		//tDataConfig.m_byIndex = byIndex + 1;
		tDataConfig.m_byIndex = i + 1;	//HPH 2021.12.07
		tDataConfig.m_byDataClassID = m_chUnPackedBuff[C_N_DATACLASS_ID_POS + byIndex * 6];
		tDataConfig.m_byObjectID = m_chUnPackedBuff[C_N_OBJECT_ID_POS + byIndex * 6];
		tDataConfig.m_byAttributeID = m_chUnPackedBuff[C_N_ATTRIBUTE_ID_POS + byIndex * 6];
		tDataConfig.m_byElementID = i + 1;
		tDataConfig.m_byDataValueLength = 1;
		tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;

		if (tDataConfig.m_byAttributeID == PHASE_ID_CONFLICT || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValueLength = 1;
	        tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			tDataConfig.m_byDataValue[0] = tDataConfig.m_byElementID;
			memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
	        tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == PHASE_CONFLICT_ARRAY || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValueLength = C_N_MAX_PHASE_CONFLICT;
	        tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			memcpy(tDataConfig.m_byDataValue, m_tAscParamInfo.m_stPhaseConflictInfo[i].m_byConflictSequenceInfo, C_N_MAX_PHASE_CONFLICT);
			memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
	        tCorrectReturnData.m_byReturnCount += 1;
		}
	}
}

void COpenATCCommWithGB20999Thread::QueryAllElementPhasGreenInterval(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData)
{
	for (int i = 0;i < m_tAscParamInfo.m_stPhaseGreenGapValidSize;i++)
	{
		TDataConfig tDataConfig;
		//tDataConfig.m_byIndex = byIndex + 1;
		tDataConfig.m_byIndex = i + 1;	//HPH 2021.12.07
		tDataConfig.m_byDataClassID = m_chUnPackedBuff[C_N_DATACLASS_ID_POS + byIndex * 6];
		tDataConfig.m_byObjectID = m_chUnPackedBuff[C_N_OBJECT_ID_POS + byIndex * 6];
		tDataConfig.m_byAttributeID = m_chUnPackedBuff[C_N_ATTRIBUTE_ID_POS + byIndex * 6];
		tDataConfig.m_byElementID = i + 1;
		tDataConfig.m_byDataValueLength = 1;
		tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;

		if (tDataConfig.m_byAttributeID == PHASE_ID_GREENINTERVAL || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValueLength = 1;
			tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			tDataConfig.m_byDataValue[0] = tDataConfig.m_byElementID;
			memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == PHASE_GREENINTERVAL_ARRAY || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValueLength = C_N_MAX_PHASE_GREENINTERVAL;
			tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			memcpy(tDataConfig.m_byDataValue, m_tAscParamInfo.m_stPhaseGreenGapInfo[i].m_byGreenGapSequenceInfo, C_N_MAX_PHASE_GREENINTERVAL);
			memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
		}
	}
}

void COpenATCCommWithGB20999Thread::QueryPhaseConflict(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData)
{
	TDataConfig tDataConfig;
	tDataConfig.m_byIndex = byIndex + 1;
	tDataConfig.m_byDataClassID = m_chUnPackedBuff[C_N_DATACLASS_ID_POS + byIndex * 6];
	tDataConfig.m_byObjectID = m_chUnPackedBuff[C_N_OBJECT_ID_POS + byIndex * 6];
	tDataConfig.m_byAttributeID = m_chUnPackedBuff[C_N_ATTRIBUTE_ID_POS + byIndex * 6];
	tDataConfig.m_byElementID = m_chUnPackedBuff[C_N_ELEMENT_ID_POS + byIndex * 6];
	tDataConfig.m_byDataValueLength = 1;
	tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;

	if (tDataConfig.m_byElementID < 1 || tDataConfig.m_byElementID > 64)
	{
		tDataConfig.m_byDataValue[0] = BAD_VALUE_NULL;
		memcpy(&tWrongReturnData.m_tDataConfig[tWrongReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		tWrongReturnData.m_byReturnCount += 1;
		return;
	}

	int  byPhaseIndex = -1;
	for (int i = 0;i < MAX_PHASE_COUNT;i++)
	{
		if (m_tAscParamInfo.m_stPhaseConflictInfo[i].m_byPhaseNumber == tDataConfig.m_byElementID)
		{
			byPhaseIndex = i;
			break;
		}
	}
 
	if (byPhaseIndex == -1)
	{
		tDataConfig.m_byDataValue[0] = BAD_VALUE_NULL;
		memcpy(&tWrongReturnData.m_tDataConfig[tWrongReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		tWrongReturnData.m_byReturnCount += 1;
		return;
	}
	
	if (tDataConfig.m_byAttributeID == PHASE_ID_CONFLICT || tDataConfig.m_byAttributeID == 0)
	{
		tDataConfig.m_byDataValueLength = 1;
	    tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
		tDataConfig.m_byDataValue[0] = tDataConfig.m_byElementID;
		memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		tCorrectReturnData.m_byReturnCount += 1;
	}
	if (tDataConfig.m_byAttributeID == PHASE_CONFLICT_ARRAY || tDataConfig.m_byAttributeID == 0)
	{
		tDataConfig.m_byDataValueLength = C_N_MAX_PHASE_CONFLICT;
		tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
		memcpy(tDataConfig.m_byDataValue, m_tAscParamInfo.m_stPhaseConflictInfo[byPhaseIndex].m_byConflictSequenceInfo, C_N_MAX_PHASE_CONFLICT);
		memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		tCorrectReturnData.m_byReturnCount += 1;
	}
}

void  COpenATCCommWithGB20999Thread::QueryPhaseGreenInterval(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData)
{
	TDataConfig tDataConfig;
	tDataConfig.m_byIndex = byIndex + 1;
	tDataConfig.m_byDataClassID = m_chUnPackedBuff[C_N_DATACLASS_ID_POS + byIndex * 6];
	tDataConfig.m_byObjectID = m_chUnPackedBuff[C_N_OBJECT_ID_POS + byIndex * 6];
	tDataConfig.m_byAttributeID = m_chUnPackedBuff[C_N_ATTRIBUTE_ID_POS + byIndex * 6];
	tDataConfig.m_byElementID = m_chUnPackedBuff[C_N_ELEMENT_ID_POS + byIndex * 6];
	tDataConfig.m_byDataValueLength = 1;
	tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;

	if (tDataConfig.m_byElementID < 1 || tDataConfig.m_byElementID > 64)
	{
		tDataConfig.m_byDataValue[0] = BAD_VALUE_NULL;
		memcpy(&tWrongReturnData.m_tDataConfig[tWrongReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		tWrongReturnData.m_byReturnCount += 1;
		return;
	}

	int  byPhaseIndex = -1;
	for (int i = 0;i < C_N_MAX_PHASE_GREENINTERVAL;i++)
	{
		if (m_tAscParamInfo.m_stPhaseGreenGapInfo[i].m_byPhaseNumber == tDataConfig.m_byElementID)
		{
			byPhaseIndex = i;
			break;
		}
	}
   
	if (byPhaseIndex == -1)
	{
		byPhaseIndex = tDataConfig.m_byElementID - 1;
		m_tAscParamInfo.m_stPhaseGreenGapInfo[byPhaseIndex].m_byPhaseNumber = tDataConfig.m_byElementID;
	}

	if (tDataConfig.m_byAttributeID == PHASE_ID_GREENINTERVAL || tDataConfig.m_byAttributeID == 0)
	{
		tDataConfig.m_byDataValueLength = 1;
	    tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
		tDataConfig.m_byDataValue[0] = tDataConfig.m_byElementID;
		memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		tCorrectReturnData.m_byReturnCount += 1;
	}
	if (tDataConfig.m_byAttributeID == PHASE_GREENINTERVAL_ARRAY || tDataConfig.m_byAttributeID == 0)
	{
		tDataConfig.m_byDataValueLength = C_N_MAX_PHASE_GREENINTERVAL;
	    tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
		memcpy(tDataConfig.m_byDataValue, m_tAscParamInfo.m_stPhaseGreenGapInfo[byPhaseIndex].m_byGreenGapSequenceInfo, C_N_MAX_PHASE_GREENINTERVAL);
		memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		tCorrectReturnData.m_byReturnCount += 1;
	}
}

void COpenATCCommWithGB20999Thread::QueryPriorityCount(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData)
{
	TDataConfig tDataConfig;
	tDataConfig.m_byIndex = byIndex + 1;
	tDataConfig.m_byDataClassID = m_chUnPackedBuff[C_N_DATACLASS_ID_POS + byIndex * 6];
	tDataConfig.m_byObjectID = m_chUnPackedBuff[C_N_OBJECT_ID_POS + byIndex * 6];
	tDataConfig.m_byAttributeID = m_chUnPackedBuff[C_N_ATTRIBUTE_ID_POS + byIndex * 6];
	tDataConfig.m_byElementID = m_chUnPackedBuff[C_N_ELEMENT_ID_POS + byIndex * 6];
	tDataConfig.m_byDataValueLength = 1;
	tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;

	//if (m_tAscParamInfo.m_stPriorityValidSize == 0)
	//{
	//	m_tAscParamInfo.m_stPriorityValidSize = 2;//测试需要配置至少2个值
	//}

	tDataConfig.m_byDataValue[0] = m_tAscParamInfo.m_stPriorityValidSize;
	memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
	tCorrectReturnData.m_byReturnCount += 1;
}

void COpenATCCommWithGB20999Thread::QueryEmergencyCount(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData)
{
	TDataConfig tDataConfig;
	tDataConfig.m_byIndex = byIndex + 1;
	tDataConfig.m_byDataClassID = m_chUnPackedBuff[C_N_DATACLASS_ID_POS + byIndex * 6];
	tDataConfig.m_byObjectID = m_chUnPackedBuff[C_N_OBJECT_ID_POS + byIndex * 6];
	tDataConfig.m_byAttributeID = m_chUnPackedBuff[C_N_ATTRIBUTE_ID_POS + byIndex * 6];
	tDataConfig.m_byElementID = m_chUnPackedBuff[C_N_ELEMENT_ID_POS + byIndex * 6];
	tDataConfig.m_byDataValueLength = 1;
	tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;

	//if (m_tAscParamInfo.m_stEmergyValidSize == 0)
	//{
	//	m_tAscParamInfo.m_stEmergyValidSize = 2;//测试需要配置至少2个值
	//}

	tDataConfig.m_byDataValue[0] = m_tAscParamInfo.m_stEmergyValidSize;
	memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
	tCorrectReturnData.m_byReturnCount += 1;
}

void COpenATCCommWithGB20999Thread::QueryAllElementPriorityConfig(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData)
{
	for (int i = 0;i < m_tAscParamInfo.m_stPriorityValidSize;i++)
	{
		TDataConfig tDataConfig;
		tDataConfig.m_byIndex = byIndex + 1;
		tDataConfig.m_byDataClassID = m_chUnPackedBuff[C_N_DATACLASS_ID_POS + byIndex * 6];
		tDataConfig.m_byObjectID = m_chUnPackedBuff[C_N_OBJECT_ID_POS + byIndex * 6];
		tDataConfig.m_byAttributeID = m_chUnPackedBuff[C_N_ATTRIBUTE_ID_POS + byIndex * 6];
		tDataConfig.m_byElementID = i + 1;
		tDataConfig.m_byDataValueLength = 1;
		tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;

		if (tDataConfig.m_byAttributeID == PRIORITY_ID_CONFIG || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValue[0] = tDataConfig.m_byElementID;
		    memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == PRIORITY_APPLY_PHASESTAGE || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValue[0] = m_tAscParamInfo.m_stPriorityInfo[i].m_byPrioritySignalPhaseStage;
		    memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == PRIORITY_APPLY_GRADE || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValue[0] = m_tAscParamInfo.m_stPriorityInfo[i].m_byPrioritySignalGrade;
		    memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == PRIORITY_SHIELD || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValue[0] = m_tAscParamInfo.m_stPriorityInfo[i].m_blPrioritySignalScreen;
		    memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
		}
	}
}

void COpenATCCommWithGB20999Thread::QueryAllElementPriorityStatus(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData)
{
	for (int i = 0;i < m_tAscParamInfo.m_stPriorityValidSize;i++)
	{
		TDataConfig tDataConfig;
		tDataConfig.m_byIndex = byIndex + 1;
		tDataConfig.m_byDataClassID = m_chUnPackedBuff[C_N_DATACLASS_ID_POS + byIndex * 6];
		tDataConfig.m_byObjectID = m_chUnPackedBuff[C_N_OBJECT_ID_POS + byIndex * 6];
		tDataConfig.m_byAttributeID = m_chUnPackedBuff[C_N_ATTRIBUTE_ID_POS + byIndex * 6];
		tDataConfig.m_byElementID = i + 1;
		tDataConfig.m_byDataValueLength = 1;
		tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;

		if (tDataConfig.m_byAttributeID == PRIORITY_ID_STATUS || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValue[0] = i + 1;
		    memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == PRIORITY_APPLY_STATUS || tDataConfig.m_byAttributeID == 0)
		{
			//tDataConfig.m_byDataValue[0] = m_tAscParamInfo.m_stPriorityInfo[i].m_bPrioritySignalStatus;
			//tDataConfig.m_byDataValue[0] = m_pOpenATCRunStatus->GetPrioritySignalStatus(i);
		    memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == PRIORITY_EXECUTE_STATUS || tDataConfig.m_byAttributeID == 0)
		{
			//tDataConfig.m_byDataValue[0] = m_tAscParamInfo.m_stPriorityInfo[i].m_bPrioritySignalPerformStatus;
			//tDataConfig.m_byDataValue[0] = m_pOpenATCRunStatus->GetPrioritySignalPerformStatus(i);
		    memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
		}
	}
}

void COpenATCCommWithGB20999Thread::QueryAllElementEmergencyConfig(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData)
{
	for (int i = 0;i < m_tAscParamInfo.m_stEmergyValidSize;i++)
	{
		TDataConfig tDataConfig;
		tDataConfig.m_byIndex = byIndex + 1;
		tDataConfig.m_byDataClassID = m_chUnPackedBuff[C_N_DATACLASS_ID_POS + byIndex * 6];
		tDataConfig.m_byObjectID = m_chUnPackedBuff[C_N_OBJECT_ID_POS + byIndex * 6];
		tDataConfig.m_byAttributeID = m_chUnPackedBuff[C_N_ATTRIBUTE_ID_POS + byIndex * 6];
		tDataConfig.m_byElementID = i + 1;
		tDataConfig.m_byDataValueLength = 1;
		tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;

		if (tDataConfig.m_byAttributeID == PRIORITY_ID_CONFIG || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValue[0] = tDataConfig.m_byElementID;
		    memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == PRIORITY_APPLY_PHASESTAGE || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValue[0] = m_tAscParamInfo.m_stEmergyInfo[i].m_byEmergySignalPhaseStage;
		    memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == PRIORITY_APPLY_GRADE || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValue[0] = m_tAscParamInfo.m_stEmergyInfo[i].m_byEmergySignalGrade;
		    memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == PRIORITY_SHIELD || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValue[0] = m_tAscParamInfo.m_stEmergyInfo[i].m_bEmergySignalScreen;
		    memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
		}
	}
}

void COpenATCCommWithGB20999Thread::QueryAllElementEmergencyStatus(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData)
{
	for (int i = 0;i < m_tAscParamInfo.m_stEmergyValidSize;i++)
	{
		TDataConfig tDataConfig;
		tDataConfig.m_byIndex = byIndex + 1;
		tDataConfig.m_byDataClassID = m_chUnPackedBuff[C_N_DATACLASS_ID_POS + byIndex * 6];
		tDataConfig.m_byObjectID = m_chUnPackedBuff[C_N_OBJECT_ID_POS + byIndex * 6];
		tDataConfig.m_byAttributeID = m_chUnPackedBuff[C_N_ATTRIBUTE_ID_POS + byIndex * 6];
		tDataConfig.m_byElementID = i + 1;
		tDataConfig.m_byDataValueLength = 1;
		tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;

		if (tDataConfig.m_byAttributeID == PRIORITY_ID_STATUS || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValue[0] = i + 1;
		    memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == PRIORITY_APPLY_STATUS || tDataConfig.m_byAttributeID == 0)
		{
			//tDataConfig.m_byDataValue[0] = m_tAscParamInfo.m_stEmergyInfo[i].m_bEmergySignalStatus;
			//tDataConfig.m_byDataValue[0] = m_pOpenATCRunStatus->GetEmergySignalStatus(i);
		    memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == PRIORITY_EXECUTE_STATUS || tDataConfig.m_byAttributeID == 0)
		{
			//tDataConfig.m_byDataValue[0] = m_tAscParamInfo.m_stEmergyInfo[i].m_bEmergySignalPerformStatus;
			//tDataConfig.m_byDataValue[0] = m_pOpenATCRunStatus->GetEmergySignalPerformStatus(i);
		    memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
		}
	}
}

void  COpenATCCommWithGB20999Thread::QueryPriorityConfig(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData)
{
	TDataConfig tDataConfig;
	tDataConfig.m_byIndex = byIndex + 1;
	tDataConfig.m_byDataClassID = m_chUnPackedBuff[C_N_DATACLASS_ID_POS + byIndex * 6];
	tDataConfig.m_byObjectID = m_chUnPackedBuff[C_N_OBJECT_ID_POS + byIndex * 6];
	tDataConfig.m_byAttributeID = m_chUnPackedBuff[C_N_ATTRIBUTE_ID_POS + byIndex * 6];
	tDataConfig.m_byElementID = m_chUnPackedBuff[C_N_ELEMENT_ID_POS + byIndex * 6];
	tDataConfig.m_byDataValueLength = 1;
	tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;

	if (tDataConfig.m_byElementID < 1 || tDataConfig.m_byElementID > 64)
	{
		tDataConfig.m_byDataValue[0] = BAD_VALUE_NULL;
		memcpy(&tWrongReturnData.m_tDataConfig[tWrongReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		tWrongReturnData.m_byReturnCount += 1;
		return;
	}

	int  nPriorityIndex = -1;
	for (int i = 0;i < C_N_MAX_PRIORITY_COUNT;i++)
	{
		if (m_tAscParamInfo.m_stPriorityInfo[i].m_byPrioritySignalNumber == tDataConfig.m_byElementID)
		{
			nPriorityIndex = i;
			break;
		}
	}

	if (nPriorityIndex != -1)
	{
		if (tDataConfig.m_byAttributeID == PRIORITY_ID_CONFIG || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValue[0] = tDataConfig.m_byElementID;
		    memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == PRIORITY_APPLY_PHASESTAGE || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValue[0] = m_tAscParamInfo.m_stPriorityInfo[nPriorityIndex].m_byPrioritySignalPhaseStage;
		    memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == PRIORITY_APPLY_GRADE || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValue[0] = m_tAscParamInfo.m_stPriorityInfo[nPriorityIndex].m_byPrioritySignalGrade;
		    memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == PRIORITY_SHIELD || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValue[0] = m_tAscParamInfo.m_stPriorityInfo[nPriorityIndex].m_blPrioritySignalScreen;
		    memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == PRIORITY_SOURCE || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValue[0] = m_tAscParamInfo.m_stPriorityInfo[nPriorityIndex].m_byPrioritySignalSource;
		    memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
		}
	}
	else
	{
		tDataConfig.m_byDataValue[0] = BAD_VALUE_NULL;
		memcpy(&tWrongReturnData.m_tDataConfig[tWrongReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		tWrongReturnData.m_byReturnCount += 1;
	}
}

void  COpenATCCommWithGB20999Thread::QueryPriorityStatus(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData)
{
	TDataConfig tDataConfig;
	tDataConfig.m_byIndex = byIndex + 1;
	tDataConfig.m_byDataClassID = m_chUnPackedBuff[C_N_DATACLASS_ID_POS + byIndex * 6];
	tDataConfig.m_byObjectID = m_chUnPackedBuff[C_N_OBJECT_ID_POS + byIndex * 6];
	tDataConfig.m_byAttributeID = m_chUnPackedBuff[C_N_ATTRIBUTE_ID_POS + byIndex * 6];
	tDataConfig.m_byElementID = m_chUnPackedBuff[C_N_ELEMENT_ID_POS + byIndex * 6];
	tDataConfig.m_byDataValueLength = 1;
	tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;

	if (tDataConfig.m_byElementID < 1 || tDataConfig.m_byElementID > 64)
	{
		tDataConfig.m_byDataValue[0] = BAD_VALUE_NULL;
		memcpy(&tWrongReturnData.m_tDataConfig[tWrongReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		tWrongReturnData.m_byReturnCount += 1;
		return;
	}

	int  nPriorityIndex = -1;
	for (int i = 0;i < C_N_MAX_PRIORITY_COUNT;i++)
	{
		if (m_tAscParamInfo.m_stPriorityInfo[i].m_byPrioritySignalNumber == tDataConfig.m_byElementID)
		{
			nPriorityIndex = i;
			break;
		}
	}

	if (nPriorityIndex != -1)
	{
		if (tDataConfig.m_byAttributeID == PRIORITY_ID_STATUS || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValue[0] = tDataConfig.m_byElementID;
		    memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == PRIORITY_APPLY_STATUS || tDataConfig.m_byAttributeID == 0)
		{
			//tDataConfig.m_byDataValue[0] = m_tAscParamInfo.m_stPriorityInfo[nPriorityIndex].m_bPrioritySignalStatus;
			//tDataConfig.m_byDataValue[0] = m_pOpenATCRunStatus->GetPrioritySignalStatus(nPriorityIndex);
		    memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == PRIORITY_EXECUTE_STATUS || tDataConfig.m_byAttributeID == 0)
		{
			//tDataConfig.m_byDataValue[0] = m_tAscParamInfo.m_stPriorityInfo[nPriorityIndex].m_bPrioritySignalPerformStatus;
			//tDataConfig.m_byDataValue[0] = m_pOpenATCRunStatus->GetPrioritySignalPerformStatus(nPriorityIndex);
		    memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
		}
	}
	else
	{
		tDataConfig.m_byDataValue[0] = BAD_VALUE_NULL;
		memcpy(&tWrongReturnData.m_tDataConfig[tWrongReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		tWrongReturnData.m_byReturnCount += 1;
	}
}

void  COpenATCCommWithGB20999Thread::QueryEmergencyConfig(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData)
{
	TDataConfig tDataConfig;
	tDataConfig.m_byIndex = byIndex + 1;
	tDataConfig.m_byDataClassID = m_chUnPackedBuff[C_N_DATACLASS_ID_POS + byIndex * 6];
	tDataConfig.m_byObjectID = m_chUnPackedBuff[C_N_OBJECT_ID_POS + byIndex * 6];
	tDataConfig.m_byAttributeID = m_chUnPackedBuff[C_N_ATTRIBUTE_ID_POS + byIndex * 6];
	tDataConfig.m_byElementID = m_chUnPackedBuff[C_N_ELEMENT_ID_POS + byIndex * 6];
	tDataConfig.m_byDataValueLength = 1;
	tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;

	if (tDataConfig.m_byElementID < 1 || tDataConfig.m_byElementID > 64)
	{
		tDataConfig.m_byDataValue[0] = BAD_VALUE_NULL;
		memcpy(&tWrongReturnData.m_tDataConfig[tWrongReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		tWrongReturnData.m_byReturnCount += 1;
		return;
	}

	int  nEmergencyIndex = -1;
	for (int i = 0;i < C_N_MAX_EMERGENCY_COUNT;i++)
	{
		if (m_tAscParamInfo.m_stEmergyInfo[i].m_byEmergySignalID == tDataConfig.m_byElementID)
		{
			nEmergencyIndex = i;
			break;
		}
	}
	if (nEmergencyIndex != -1)
	{
		if (tDataConfig.m_byAttributeID == EMERGENCY_ID_CONFIG || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValue[0] = tDataConfig.m_byElementID;
		    memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == EMERGENCY_APPLY_PHASESTAGE || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValue[0] = m_tAscParamInfo.m_stEmergyInfo[nEmergencyIndex].m_byEmergySignalPhaseStage;
		    memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == EMERGENCY_APPLY_GRADE || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValue[0] = m_tAscParamInfo.m_stEmergyInfo[nEmergencyIndex].m_byEmergySignalGrade;
		    memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == EMERGENCY_SHIELD || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValue[0] = m_tAscParamInfo.m_stEmergyInfo[nEmergencyIndex].m_bEmergySignalScreen;
		    memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == EMERGENCY_SOURCE || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValue[0] = m_tAscParamInfo.m_stEmergyInfo[nEmergencyIndex].m_byEmergySignalSource;
		    memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
		}
	}
	else
	{
		tDataConfig.m_byDataValue[0] = BAD_VALUE_NULL;
		memcpy(&tWrongReturnData.m_tDataConfig[tWrongReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		tWrongReturnData.m_byReturnCount += 1;
	}
}

void  COpenATCCommWithGB20999Thread::QueryEmergencyStatus(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData)
{
	TDataConfig tDataConfig;
	tDataConfig.m_byIndex = byIndex + 1;
	tDataConfig.m_byDataClassID = m_chUnPackedBuff[C_N_DATACLASS_ID_POS + byIndex * 6];
	tDataConfig.m_byObjectID = m_chUnPackedBuff[C_N_OBJECT_ID_POS + byIndex * 6];
	tDataConfig.m_byAttributeID = m_chUnPackedBuff[C_N_ATTRIBUTE_ID_POS + byIndex * 6];
	tDataConfig.m_byElementID = m_chUnPackedBuff[C_N_ELEMENT_ID_POS + byIndex * 6];
	tDataConfig.m_byDataValueLength = 1;
	tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;

	if (tDataConfig.m_byElementID < 1 || tDataConfig.m_byElementID > 64)
	{
		tDataConfig.m_byDataValue[0] = BAD_VALUE_NULL;
		memcpy(&tWrongReturnData.m_tDataConfig[tWrongReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		tWrongReturnData.m_byReturnCount += 1;
		return;
	}

	int  nEmergencyIndex = 1;
	for (int i = 0;i < C_N_MAX_EMERGENCY_COUNT;i++)
	{
		if (m_tAscParamInfo.m_stEmergyInfo[i].m_byEmergySignalID == tDataConfig.m_byElementID)
		{
			nEmergencyIndex = i;
			break;
		}
	}

	if (nEmergencyIndex != -1)
	{
		if (tDataConfig.m_byAttributeID == EMERGENCY_ID_STATUS || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValue[0] = tDataConfig.m_byElementID;
		    memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == EMERGENCY_APPLY_STATUS || tDataConfig.m_byAttributeID == 0)
		{
			//tDataConfig.m_byDataValue[0] = m_tAscParamInfo.m_stEmergyInfo[nEmergencyIndex].m_bEmergySignalStatus;
			//tDataConfig.m_byDataValue[0] = m_pOpenATCRunStatus->GetEmergySignalStatus(nEmergencyIndex);
		    memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == EMERGENCY_EXECUTE_STATUS || tDataConfig.m_byAttributeID == 0)
		{
			//tDataConfig.m_byDataValue[0] = m_tAscParamInfo.m_stEmergyInfo[nEmergencyIndex].m_bEmergySignalPerformStatus;
			//tDataConfig.m_byDataValue[0] = m_pOpenATCRunStatus->GetEmergySignalPerformStatus(nEmergencyIndex);
		    memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
		}
	}
	else
	{
		tDataConfig.m_byDataValue[0] = BAD_VALUE_NULL;
		memcpy(&tWrongReturnData.m_tDataConfig[tWrongReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		tWrongReturnData.m_byReturnCount += 1;
	}
}

void  COpenATCCommWithGB20999Thread::GetRunStageTable(TFixTimeCtlInfo tFixTimeCtlInfo, TRunStageInfo & tRunStageInfo)
{
	int nStageTime[MAX_RING_COUNT][MAX_PHASE_COUNT] = {0};//环内相位节点  
	int nAllStageTime[MAX_STAGE_COUNT] = {0};//阶段节点表

	int i = 0;
    int j = 0;
    int m = 0;
    int n = 0;
    int nAllStageCount = 0;

    //生成阶段节点表
    for (i = 0;i < tFixTimeCtlInfo.m_nRingCount;i++)
    {
        for (j = 0;j < tFixTimeCtlInfo.m_atPhaseSeq[i].m_nPhaseCount;j++)
        {
            if (j == 0)
			{
                nStageTime[i][0] = tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[j].m_wPhaseTime;
			}
			else
			{
				nStageTime[i][j] = tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[j].m_wPhaseTime + nStageTime[i][j - 1];
			}

            if (nStageTime[i][j] > 0)
			{
				nAllStageTime[nAllStageCount++] = nStageTime[i][j];
			}
        }
    }

	std::sort(nAllStageTime, nAllStageTime + nAllStageCount);//阶段节点表排序

    //阶段节点表去掉重复的节点
    int nStageCount = 0;
    for (i = 0;i < nAllStageCount;i++)
    {
        if (nAllStageTime[i + 1] == nAllStageTime[i])
        {
            continue;
        }
        else
        {
            nAllStageTime[nStageCount + 1] = nAllStageTime[i + 1];
            nStageCount++;
        }
    }

    tRunStageInfo.m_nRunStageCount = nStageCount;
    
	//生成阶段相位表
	int nPhaseIndex = 0;
	for (i = 0; i < tFixTimeCtlInfo.m_nRingCount; i++)
	{
		for (j = 0; j < tFixTimeCtlInfo.m_atPhaseSeq[i].m_nPhaseCount; j++)
		{
			int nMaxStage = 0;
			int nMinStage = 0;
			int nStageSum = 0;
			
			for (m = 0; m < nStageCount; m++)//查找相位的相序时间在阶段相位表中对应的阶段编号的最大值
			{
				if (nStageTime[i][j] < nAllStageTime[m] || nStageTime[i][j] == nAllStageTime[m])
				{
					nStageSum = m + 1;
					break;
				}
			}

			nMaxStage = nStageSum;

			if (j == 0)
			{
				nMinStage = 1;
			}
			else
			{ 
				for (m = 0; m < nStageCount; m++)//查找相位的相序时间在阶段相位表中对应的阶段编号的最小值
				{
					if (nStageTime[i][j - 1] < nAllStageTime[m] || nStageTime[i][j] == nAllStageTime[m])
					{
						nStageSum = m + 1;
						break;
					}
				}

				nMinStage = nStageSum;
			}
			
			for (m = nMinStage - 1;m < nMaxStage;m++)       //相位在阶段相位表中对应区间的阶段赋值
			{
				if (tRunStageInfo.m_PhaseRunstageInfo[m].m_nConcurrencyPhase[i] == 0)
                {
                    tRunStageInfo.m_PhaseRunstageInfo[m].m_nStageIndex = m;
                    if (m == 0)
                    {
                        tRunStageInfo.m_PhaseRunstageInfo[m].m_nStageStartTime = 0;  
                    }
                    else
                    {
                        tRunStageInfo.m_PhaseRunstageInfo[m].m_nStageStartTime = nAllStageTime[m - 1];
                    }
                    tRunStageInfo.m_PhaseRunstageInfo[m].m_nStageEndTime = nAllStageTime[m];
        
                    tRunStageInfo.m_PhaseRunstageInfo[m].m_nConcurrencyPhase[i] = tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[j].m_tPhaseParam.m_byPhaseNumber;
                    tRunStageInfo.m_PhaseRunstageInfo[m].m_nConcurrencyPhaseCount += 1;
                }
			}
		}
	}
}

BYTE  COpenATCCommWithGB20999Thread::TransAlarmTypeToHost(BYTE byFaultValue, char cFaultInfo1, char cFaultInfo2, BYTE & byAlarmValue)
{
	BYTE byAlarmType = 0;
	if (byFaultValue == FaultType_Red_Lamp_Volt_Fault || byFaultValue == FaultType_Red_Lamp_Power_Fault)
	{
		byAlarmType = TYPE_ALARM_DEVICE;
		byAlarmValue = (cFaultInfo1 - 1) * 12 + cFaultInfo2;
	}
	else if (byFaultValue == FaultType_Yellow_Lamp_Volt_Fault || byFaultValue == FaultType_Yellow_Lamp_Power_Fault)
	{
		byAlarmType = TYPE_ALARM_DEVICE;
		byAlarmValue = (cFaultInfo1 - 1) * 12 + cFaultInfo2;
	}
	else if (byFaultValue == FaultType_Green_Lamp_Volt_Fault || byFaultValue == FaultType_Green_Lamp_Power_Fault)
	{
		byAlarmType = TYPE_ALARM_DEVICE;
		byAlarmValue = (cFaultInfo1 - 1) * 12 + cFaultInfo2;
	}
	else if (byFaultValue == FaultType_VetDetector_Short_Circuit || byFaultValue == FaultType_VetDetector_Open_Circuit || byFaultValue == FaultType_Detector_Fault) 
	{
		byAlarmType = TYPE_ALARM_DETECTOR;
		byAlarmValue = (cFaultInfo1 - 1) * 12 + cFaultInfo2;
	}
	else if (byFaultValue == FaultType_VetDetID || byFaultValue == FaultType_VetDetNum)
	{
		byAlarmType = TYPE_ALARM_DETECTOR;
		byAlarmValue = 41 + cFaultInfo1 - 1;
	}
	return byAlarmType;
}

BYTE  COpenATCCommWithGB20999Thread::TransFaultTypeToHost(BYTE byFaultValue)
{
	BYTE byFaultType = 0;
	if (byFaultValue == FaultType_GreenConflict)
	{
		byFaultType = TYPE_FAULT_GREENCONFLICT;
	}
	else if (byFaultValue == FaultType_GreenAndRedOn)
	{
		byFaultType = TYPE_FAULT_GREENREDCONFLICT;
	}
	else if (byFaultValue == FaultType_Red_Lamp_Volt_Fault || byFaultValue == FaultType_Red_Lamp_Power_Fault)
	{
		byFaultType = TYPE_FAULT_REDLIGHT;
	}
	else if (byFaultValue == FaultType_Yellow_Lamp_Volt_Fault || byFaultValue == FaultType_Yellow_Lamp_Power_Fault)
	{
		byFaultType = TYPE_FAULT_YELLOWLIGHT;
	}
	else if (byFaultValue == FaultType_Green_Lamp_Volt_Fault || byFaultValue == FaultType_Green_Lamp_Power_Fault)
	{
		byFaultType = TYPE_FAULT_YELLOWLIGHT;
	}
	else if (byFaultValue == FaultType_MainBoard)
	{
		byFaultType = TYPE_FAULT_COMMUNICATION;
	}
	else if (byFaultValue == FaultType_VetDetector_Short_Circuit || byFaultValue == FaultType_VetDetector_Open_Circuit || byFaultValue == FaultType_Detector_Fault)
	{
		byFaultType = TYPE_FAULT_DETECTOR;
	}
	else if (byFaultValue == FaultType_Relay_Not_Work)
	{
		byFaultType = TYPE_FAULT_RELAY;
	}
	else if (byFaultValue == FaultType_Wong_Slot || byFaultValue == FaultType_Wong_Plug || byFaultValue == FaultType_Config_Master_Count || byFaultValue == FaultType_LampBoardNum || byFaultValue == FaultType_Lamp_Fault)
	{
		byFaultType = TYPE_FAULT_PHASEBOARD;
	}
	else if (byFaultValue == FaultType_VetDetID || byFaultValue == FaultType_VetDetNum)
	{
		byFaultType = TYPE_FAULT_DETECTORBOARD;
	}
	else if (byFaultValue == FaultType_TZParam)
	{
		byFaultType = TYPE_FAULT_CONFIG;
	}
	return byFaultType;
}

BYTE COpenATCCommWithGB20999Thread::TransRunModeToHost(BYTE byModeValue, BYTE byControlSource)
{
	BYTE byRunMode = 0;
	if (byModeValue == CTL_MODE_FIXTIME)
	{
		byRunMode = MODE_LOCAL_FIXCYCLE_CONTROL;
	}
	else if (byModeValue == CTL_MODE_ACTUATE)
	{
		byRunMode = MODE_LOCAL_VA_CONTROL;
	}
	else if (byModeValue == CTL_MODE_CABLELESS && byControlSource == CTL_SOURCE_SYSTEM)
	{
		byRunMode = MODE_CENTER_COORDINATION_CONTROL;
	}
	else if (byModeValue == CTL_MODE_CABLELESS && byControlSource == CTL_SOURCE_SELF)
	{
		byRunMode = MODE_LOCAL_COORDINATION_CONTROL;
	}
	else if (byModeValue == CTL_MODE_SINGLEOPTIM)
	{
		byRunMode = MODE_LOCAL_ADAPTIVE_CONTROL;
	}
	else if (byModeValue == CTL_MODE_MANUAL && byControlSource == CTL_SOURCE_SYSTEM)
	{
		byRunMode = MODE_CENTER_MANUAL_CONTROL;
	}
	else if	(byModeValue == CTL_MODE_MANUAL && byControlSource == CTL_SOURCE_LOCAL)
	{
		byRunMode = MODE_LOCAL_MANUAL_CONTROL;
	}
	else if(byModeValue == CTL_MODE_FLASH)	//黄闪
	{
		byRunMode = MODE_SPECIAL_FLASH_CONTROL;
	}
	else if(byModeValue == CTL_MODE_ALLRED)	//全红
	{
		byRunMode = MODE_SPECIAL_ALLRED_CONTROL;
	}
	else if(byModeValue == CTL_MODE_OFF)	//关灯
	{
		byRunMode = MODE_SPECIAL_ALLOFF_CONTROL;
	}
	return byRunMode;
}

BYTE COpenATCCommWithGB20999Thread::TransRunModeToASC(BYTE byModeValue)
{
	BYTE byRunMode = 0;
	if (byModeValue == MODE_LOCAL_FIXCYCLE_CONTROL)
	{
		byRunMode = CTL_MODE_FIXTIME;
	}
	else if (byModeValue == MODE_LOCAL_VA_CONTROL || byModeValue == MODE_CENTER_OPTIMIZATION_CONTROL)
	{
		byRunMode = CTL_MODE_ACTUATE;
	}
	else if (byModeValue == MODE_LOCAL_COORDINATION_CONTROL || byModeValue == MODE_CENTER_COORDINATION_CONTROL)
	{
		byRunMode = CTL_MODE_CABLELESS;
	}
	else if (byModeValue == MODE_LOCAL_ADAPTIVE_CONTROL || byModeValue == MODE_CENTER_ADAPTIVE_CONTROL)
	{
		byRunMode = CTL_MODE_SINGLEOPTIM;
	}
	else if (byModeValue == MODE_LOCAL_MANUAL_CONTROL || byModeValue == MODE_CENTER_MANUAL_CONTROL)
	{
		byRunMode = CTL_MODE_MANUAL;
	}
	return byRunMode;
}

void COpenATCCommWithGB20999Thread::OpenATCGetCurTime(int & nYear, int & nMon, int & nDay, int & nHour, int & nMin, int & nSec, int & nWeek)
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

bool COpenATCCommWithGB20999Thread::SetSysTime(const long nTime)
{
    timeval sysTime;
    sysTime.tv_sec  = nTime;
    sysTime.tv_usec = 0;
#ifndef _WIN32
	if (settimeofday(&sysTime,NULL) != 0)
    {
        return false;
    }
	system("hwclock -w");
    return true;
#else
    SYSTEMTIME winTime;
    tm  tmTime;
    tmTime = *::localtime((time_t*)&sysTime.tv_sec);
    winTime.wYear			= tmTime.tm_year + 1900;
    winTime.wMonth			= tmTime.tm_mon + 1;
    winTime.wDay			= tmTime.tm_mday;
    winTime.wHour			= tmTime.tm_hour;
    winTime.wMinute			= tmTime.tm_min;
    winTime.wSecond			= tmTime.tm_sec;
    winTime.wMilliseconds	= 0;
    
    if (SetLocalTime(&winTime) == 0)
    {
        return false;
    }

    return true;
#endif
}

char COpenATCCommWithGB20999Thread::GetBit(unsigned int nInput, char chNum)    
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

void  COpenATCCommWithGB20999Thread::RemoveDuplates(BYTE byData[], int & nCnt)
{
	int  i = 0, j = 0, k = 0;
	for (i = 0;i < nCnt;i++)    //冒泡循环
	{
		for (j = i + 1;j < nCnt;j++)
		{
			if (byData[j] == byData[i])    //如果发现重复
			{
				for (k = j + 1;k < nCnt;k++)
				{
					byData[k - 1] = byData[k];    //将后面的数依次赋值给前一个位置
				}
				nCnt--;    //数组长度-1
				j--;       //重复点再次进行查重
			}
		}
	}
}
//
//void  COpenATCCommWithGB20999Thread::SetTSequenceInfo(BYTE byPatternSequenceNumber, TRunStageInfo tRunStageInfo)
//{
//	TSequence atSequenceInfo[MAX_RING_COUNT];//相序
//	memset(atSequenceInfo, 0, sizeof(atSequenceInfo));
//    m_pOpenATCParameter->GetSequenceBySequenceNumber(byPatternSequenceNumber, atSequenceInfo);
//
//	BYTE byPhaseID1[MAX_STAGE_COUNT];
//	memset(byPhaseID1, 0, sizeof(byPhaseID1));
//	BYTE byPhaseID2[MAX_STAGE_COUNT];
//	memset(byPhaseID2, 0, sizeof(byPhaseID2));
//	
//	int  i = 0;
//	for (i = 0;i < tRunStageInfo.m_nRunStageCount;i++)
//	{
//		byPhaseID1[i] = tRunStageInfo.m_PhaseRunstageInfo[i].m_nConcurrencyPhase[0];
//		byPhaseID2[i] = tRunStageInfo.m_PhaseRunstageInfo[i].m_nConcurrencyPhase[1];
//    }
//	
//	int nPhaseID1Len = strlen((char *)byPhaseID1);
//	int nPhaseID2Len = strlen((char *)byPhaseID2);
//	if (strlen((char *)byPhaseID1) > 0)
//	{
//		RemoveDuplates(byPhaseID1, nPhaseID1Len);
//	}
//	if (strlen((char *)byPhaseID2) > 0)
//	{
//		RemoveDuplates(byPhaseID2, nPhaseID2Len);
//	}
//
//	int nCount = strlen((char *)atSequenceInfo[0].m_bySequenceData) + strlen((char *)atSequenceInfo[1].m_bySequenceData);
//	if ((nPhaseID1Len + nPhaseID2Len) >= nCount)
//	{
//		memset(atSequenceInfo[0].m_bySequenceData, 0, sizeof(atSequenceInfo[0].m_bySequenceData));
//		for (i = 0;i < nPhaseID1Len;i++)
//		{
//			atSequenceInfo[0].m_bySequenceData[i] = byPhaseID1[i];
//		}
//		memset(atSequenceInfo[1].m_bySequenceData, 0, sizeof(atSequenceInfo[0].m_bySequenceData));
//		for (i = 0;i < nPhaseID2Len;i++)
//		{
//			atSequenceInfo[1].m_bySequenceData[i] = byPhaseID2[i];
//		}
//
//		m_pOpenATCParameter->SetSequenceBySequenceNumber(byPatternSequenceNumber, atSequenceInfo);
//	    m_pOpenATCParameter->SaveSetParameter(false);
//	}
//}
//
//void  COpenATCCommWithGB20999Thread::SetTSplitInfo(BYTE byPatternSplitNumber, TRunStageInfo tRunStageInfo, bool bSetOrder, bool bSetSplitTime)
//{
//	TSplit atSplitInfo[MAX_PHASE_COUNT_20999];//绿信比表
//	memset(atSplitInfo, 0,sizeof(atSplitInfo));
//	m_pOpenATCParameter->GetSplitBySplitNumber(byPatternSplitNumber, atSplitInfo);
//
//	TSplit atNewSplitInfo[MAX_PHASE_COUNT_20999];//新绿信比表
//	memset(atNewSplitInfo, 0,sizeof(atNewSplitInfo));
//
//	//默认2个环
//	BYTE byPhaseID1[MAX_STAGE_COUNT];
//	memset(byPhaseID1, 0, sizeof(byPhaseID1));
//	BYTE byPhaseID2[MAX_STAGE_COUNT];
//	memset(byPhaseID2, 0, sizeof(byPhaseID2));
//	int nPhaseID1Len = 0;
//	int nPhaseID2Len = 0;
//
//	int nCount = 0;
//	int nSplitCount = 0;
//	int i = 0, j = 0, k = 0;
//    if (bSetOrder)
//    {   
//		for (i = 0;i < tRunStageInfo.m_nRunStageCount;i++)
//		{
//			byPhaseID1[i] = tRunStageInfo.m_PhaseRunstageInfo[i].m_nConcurrencyPhase[0];
//			byPhaseID2[i] = tRunStageInfo.m_PhaseRunstageInfo[i].m_nConcurrencyPhase[1];
//		}
//	
//		nPhaseID1Len = strlen((char *)byPhaseID1);
//		nPhaseID2Len = strlen((char *)byPhaseID2);
//		if (strlen((char *)byPhaseID1) > 0)
//		{
//			RemoveDuplates(byPhaseID1, nPhaseID1Len);
//		}
//		if (strlen((char *)byPhaseID2) > 0)
//		{
//			RemoveDuplates(byPhaseID2, nPhaseID2Len);
//		}
//
//		for (i = 0;i < MAX_PHASE_COUNT;i++)
//		{
//			if (atSplitInfo[i].m_bySplitNumber != 0)
//			{
//				nCount += 1;
//			}
//		}
//
//		if ((nPhaseID1Len + nPhaseID2Len) >= nCount)
//		{
//			for (i = 0;i < nPhaseID1Len;i++)
//			{
//				atNewSplitInfo[nSplitCount].m_bySplitNumber = byPatternSplitNumber;
//				atNewSplitInfo[nSplitCount].m_bySplitPhase = byPhaseID1[i];
//				atNewSplitInfo[nSplitCount].m_bySplitMode = UNDEFINE_MODE;
//				sprintf(atNewSplitInfo[nSplitCount].m_byName, "phase%", nSplitCount + 1);
//				nSplitCount += 1;
//			}
//			for (i = 0;i < nPhaseID2Len;i++)
//			{
//				atNewSplitInfo[nSplitCount].m_bySplitNumber = byPatternSplitNumber;
//				atNewSplitInfo[nSplitCount].m_bySplitPhase = byPhaseID2[i];
//				atNewSplitInfo[nSplitCount].m_bySplitMode = UNDEFINE_MODE;
//				sprintf(atNewSplitInfo[nSplitCount].m_byName, "phase%", nSplitCount + 1);
//				nSplitCount += 1;
//			}
//		}
//		else if ((nPhaseID1Len + nPhaseID2Len) < nCount)
//		{
//			m_pOpenATCParameter->GetSplitBySplitNumber(byPatternSplitNumber, atNewSplitInfo);
//			for (i = 0; i < nCount;i++)
//			{
//				bool bFlag = false;
//				for (j = 0;j < nPhaseID1Len;j++)
//				{
//					if (atNewSplitInfo[i].m_bySplitPhase == byPhaseID1[j])
//					{
//						bFlag = true;
//					}
//				}
//
//				for (j = 0;j < nPhaseID2Len;j++)
//				{
//					if (atNewSplitInfo[i].m_bySplitPhase == byPhaseID2[j])
//					{
//						bFlag = true;
//					}
//				}
//
//				if (!bFlag)
//				{
//					atNewSplitInfo[i].m_bySplitMode = NEGLECT_MODE;//设置为忽略相位
//				}
//			}
//
//			nSplitCount = nCount;
//		}
//	}
//	else
//	{
//		m_pOpenATCParameter->GetSplitBySplitNumber(byPatternSplitNumber, atNewSplitInfo);
//		for (i = 0;i < MAX_PHASE_COUNT;i++)
//		{
//			if (atNewSplitInfo[i].m_bySplitNumber != 0)
//			{
//				nSplitCount += 1;
//			}
//		}
//	}
//
//	if (bSetSplitTime)
//	{
//		for (k = 0;k < nSplitCount;k++)
//		{
//			atNewSplitInfo[k].m_wSplitTime = 0;
//
//			for (i = 0;i < tRunStageInfo.m_nRunStageCount;i++)
//			{
//				for (j = 0;j < tRunStageInfo.m_PhaseRunstageInfo[i].m_nConcurrencyPhaseCount;j++)
//				{
//					if (atNewSplitInfo[k].m_bySplitPhase == tRunStageInfo.m_PhaseRunstageInfo[i].m_nConcurrencyPhase[j])
//					{
//						atNewSplitInfo[k].m_wSplitTime += (tRunStageInfo.m_PhaseRunstageInfo[i].m_nStageEndTime - tRunStageInfo.m_PhaseRunstageInfo[i].m_nStageStartTime);
//					}
//				}
//			}
//		}
//	}
//
//	m_pOpenATCParameter->SetSplitBySplitNumber(byPatternSplitNumber, atNewSplitInfo);
//	m_pOpenATCParameter->SaveSetParameter(false);
//}
//
//bool  COpenATCCommWithGB20999Thread::SetStartAndEndTime(BYTE byPatternSplitNumber, BYTE byRunStageIndex, TRunStageInfo tRunStageInfo, bool bStartTime, bool bEndTime, BYTE byDataValuePos)
//{
//	TSplit atSplitInfo[MAX_PHASE_COUNT_20999];//绿信比表
//	memset(atSplitInfo, 0x00, sizeof(atSplitInfo));
//	m_pOpenATCParameter->GetSplitBySplitNumber(byPatternSplitNumber, atSplitInfo);
//
//	TPhaseRunStatus tPhaseRunStatus;
//	memset(&tPhaseRunStatus, 0x00, sizeof(tPhaseRunStatus));
//	m_pOpenATCRunStatus->GetPhaseRunStatus(tPhaseRunStatus);
//
//	TPhase atPhaseTable[MAX_PHASE_COUNT];
//	memset(atPhaseTable, 0, sizeof(atPhaseTable));
//	m_pOpenATCParameter->GetPhaseTable(atPhaseTable);
//	//默认2个环
//	bool bRet[2] = {true, true};
//	int  nRingIndex = -1;
//	int  nSplitIndex = -1;
//	int  i = 0, j = 0, k = 0;
//	for (i = 0;i < 2;i++)
//	{
//		for (j = 0;j < MAX_PHASE_COUNT;j++)
//		{
//			if (tRunStageInfo.m_PhaseRunstageInfo[byRunStageIndex].m_nConcurrencyPhase[i] == atSplitInfo[j].m_bySplitPhase && atSplitInfo[j].m_bySplitPhase != 0)
//			{
//				nRingIndex = i;
//				nSplitIndex = j;
//
//				if (tPhaseRunStatus.m_atRingRunStatus[i].m_nCurStageIndex == byRunStageIndex)
//				{
//					for (k = 0;k < MAX_SEQUENCE_TABLE_COUNT;k++)
//					{
//						if (tPhaseRunStatus.m_atRingRunStatus[i].m_atPhaseStatus[k].m_byPhaseID == atSplitInfo[j].m_bySplitPhase)
//						{
//							if (tPhaseRunStatus.m_atRingRunStatus[i].m_atPhaseStatus[k].m_chPhaseStatus == C_CH_PHASESTAGE_G)//相位已经开始运行，则设置无效
//							{
//								if (bStartTime || bEndTime)
//								{
//									bRet[i] = false;
//								}
//							}
//						}
//					}
//				}
//			}
//		}
//
//		if (bRet[i])
//		{
//			BYTE byGreenTime = 0;
//			BYTE byGreenFlash = 0;
//			BYTE byYellowChange = 0;
//			BYTE byPhaseRedClear = 0;
//			for (k = 0;k < MAX_PHASE_COUNT;k++)
//			{
//				if (atPhaseTable[k].m_byPhaseNumber == atSplitInfo[nSplitIndex].m_bySplitPhase)
//				{
//					byGreenFlash = atPhaseTable[k].m_byGreenFlash;
//					byYellowChange = atPhaseTable[k].m_byPhaseYellowChange;
//					byPhaseRedClear = atPhaseTable[k].m_byPhaseRedClear;
//					byGreenTime = atSplitInfo[nSplitIndex].m_wSplitTime - byGreenFlash - byYellowChange - byPhaseRedClear;
//				}
//			}
//		
//			BYTE byTemp = atSplitInfo[nSplitIndex].m_byDelayStartTime + atSplitInfo[nSplitIndex].m_byEarlyEndTime;
//			if (bStartTime)
//			{
//				byTemp += m_chUnPackedBuff[byDataValuePos + atSplitInfo[nSplitIndex].m_bySplitPhase - 1];
//				if (byTemp >= byGreenTime)
//				{
//					bRet[i] = false;
//				}
//				else
//				{
//					atSplitInfo[nSplitIndex].m_byDelayStartTime += m_chUnPackedBuff[byDataValuePos + atSplitInfo[nSplitIndex].m_bySplitPhase - 1];
//				}
//			}
//			else if (bEndTime)
//			{
//				byTemp += m_chUnPackedBuff[byDataValuePos + atSplitInfo[nSplitIndex].m_bySplitPhase - 1];
//				if (byTemp >= byGreenTime)
//				{
//					bRet[i] = false;
//				}
//				else
//				{
//					atSplitInfo[nSplitIndex].m_byEarlyEndTime += m_chUnPackedBuff[byDataValuePos + atSplitInfo[nSplitIndex].m_bySplitPhase - 1];
//				}
//			}
//		}
//		else
//		{
//			return false;
//		}
//	}
//
//	if (bRet[0] && bRet[1])
//	{
//		m_pOpenATCParameter->SetSplitBySplitNumber(byPatternSplitNumber, atSplitInfo);
//		m_pOpenATCParameter->SaveSetParameter(false);
//		return true;
//	}
//	else
//	{
//		return false;
//	}
//}
//
//void  COpenATCCommWithGB20999Thread::GetPatternParam(BYTE byPatternSplitNumber, BYTE byPatternSequenceNumber, TRunStageInfo & tRunStageInfo)
//{
//	TSplit atSplitInfo[MAX_PHASE_COUNT_20999];//绿信比表
//	m_pOpenATCParameter->GetSplitBySplitNumber(byPatternSplitNumber, atSplitInfo);
//
//	TSequence atSequenceInfo[MAX_RING_COUNT];
//    m_pOpenATCParameter->GetSequenceBySequenceNumber(byPatternSequenceNumber, atSequenceInfo);
//
//	TFixTimeCtlInfo tFixTimeCtlInfo;	
//
//    int  nRingCount = 0;
//    int  i = 0;
//    int  j = 0;
//    for (i = 0;i < MAX_RING_COUNT;i ++)
//    {
//        int nPhaseIndex = 0;
//        for (j = 0;j < MAX_SEQUENCE_DATA_LENGTH;j ++)
//        {
//            int nPhaseNum = (int)atSequenceInfo[i].m_bySequenceData[j];
//
//            if (nPhaseNum == 0)
//            {
//                break;
//            }
//            
//            m_pOpenATCParameter->GetPhaseByPhaseNumber(nPhaseNum, tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nPhaseIndex].m_tPhaseParam);
//
//            WORD wSplitTime = 0;
//            for (int k = 0;k < MAX_PHASE_COUNT;k ++)
//            {
//                if (atSplitInfo[k].m_bySplitPhase == nPhaseNum && atSplitInfo[k].m_bySplitMode != NEGLECT_MODE)
//                {
//                    wSplitTime = atSplitInfo[k].m_wSplitTime;  
//                    break; 
//                }
//            }
//
//            if (wSplitTime == 0)
//            {
//                continue;
//            }
//            tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nPhaseIndex].m_wPhaseTime = wSplitTime;
//            PTPhase pPhaseInfo = &(tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nPhaseIndex].m_tPhaseParam);
//            tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nPhaseIndex].m_wPhaseGreenTime = wSplitTime - pPhaseInfo->m_byGreenFlash - pPhaseInfo->m_byPhaseYellowChange - pPhaseInfo->m_byPhaseRedClear;
//            tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nPhaseIndex].m_wPedPhaseGreenTime = wSplitTime - pPhaseInfo->m_byPhaseYellowChange - pPhaseInfo->m_byPhasePedestrianClear - pPhaseInfo->m_byPhaseRedClear;
//            tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nPhaseIndex].m_chPedPhaseStage = C_CH_PHASESTAGE_U;
//            tFixTimeCtlInfo.m_atPhaseSeq[i].m_atPhaseInfo[nPhaseIndex].m_chPhaseStage = C_CH_PHASESTAGE_U;
//
//            nPhaseIndex++;
//        }
//
//		if (nPhaseIndex != 0)
//		{
//			nRingCount++;
//		}
//
//        tFixTimeCtlInfo.m_atPhaseSeq[i].m_nPhaseCount = nPhaseIndex;      
//    }
//
//	tFixTimeCtlInfo.m_nRingCount = nRingCount;
//
//	GetRunStageTable(tFixTimeCtlInfo, tRunStageInfo);
//}

void COpenATCCommWithGB20999Thread::QueryPatternCount(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData)
{
	TDataConfig tDataConfig;
	tDataConfig.m_byIndex = byIndex + 1;
	tDataConfig.m_byDataClassID = m_chUnPackedBuff[C_N_DATACLASS_ID_POS + byIndex * 6];
	tDataConfig.m_byObjectID = m_chUnPackedBuff[C_N_OBJECT_ID_POS + byIndex * 6];
	tDataConfig.m_byAttributeID = m_chUnPackedBuff[C_N_ATTRIBUTE_ID_POS + byIndex * 6];
	tDataConfig.m_byElementID = m_chUnPackedBuff[C_N_ELEMENT_ID_POS + byIndex * 6];
	tDataConfig.m_byDataValueLength = 1;
	tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;

	tDataConfig.m_byDataValue[0] = m_tAscParamInfo.m_stPatternTableValidSize;
	memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
	tCorrectReturnData.m_byReturnCount += 1;
}

void  COpenATCCommWithGB20999Thread::QueryAllElementPatternConfig(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData)
{	
	for (int i = 0;i < m_tAscParamInfo.m_stPatternTableValidSize;i++)
	{
		TDataConfig tDataConfig;
		//tDataConfig.m_byIndex = byIndex + 1;
		tDataConfig.m_byIndex = i + 1;	//HPH 2021.12.07
		tDataConfig.m_byDataClassID = m_chUnPackedBuff[C_N_DATACLASS_ID_POS + byIndex * 6];
		tDataConfig.m_byObjectID = m_chUnPackedBuff[C_N_OBJECT_ID_POS + byIndex * 6];
		tDataConfig.m_byAttributeID = m_chUnPackedBuff[C_N_ATTRIBUTE_ID_POS + byIndex * 6];
		tDataConfig.m_byElementID = i + 1;
		tDataConfig.m_byDataValueLength = 1;
		tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;

		if (tDataConfig.m_byAttributeID == PATTERN_ID || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValueLength = 1;
	        tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			tDataConfig.m_byDataValue[0] = tDataConfig.m_byElementID;
			memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
	        tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == PATTERN_ROADID || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValueLength = 1;
	        tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			tDataConfig.m_byDataValue[0] = m_tAscParamInfo.m_stAscPatternTable[i].m_byRoadID;
			memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
	        tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == PATTERN_CYCLELEN || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValueLength = 4;
	        tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			int nTemp = ntohl(m_tAscParamInfo.m_stAscPatternTable[i].m_wPatternCycleTime);
			memcpy(tDataConfig.m_byDataValue, &nTemp, 4);
			memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
	        tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == PATTERN_ADJUST_STAGEID || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValueLength = 1;
	        tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			tDataConfig.m_byDataValue[0] = m_tAscParamInfo.m_stAscPatternTable[i].m_byCoorditionStage;
			memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
	        tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == PATTERN_OFFSET || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValueLength = 2;
	        tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			short nTemp = ntohs(m_tAscParamInfo.m_stAscPatternTable[i].m_byPatternOffsetTime);
			memcpy(tDataConfig.m_byDataValue, &nTemp, 2);
			memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
	        tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == PATTERN_STAGE_CHAIN || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValueLength = 16;
	        tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			for(int ipatternIndex = 0; ipatternIndex<16; ipatternIndex++)
			{
				tDataConfig.m_byDataValue[ipatternIndex] = m_tAscParamInfo.m_stAscPatternTable[i].m_byPatternStage[15-ipatternIndex];
			}
			//memcpy(tDataConfig.m_byDataValue, m_tAscParamInfo.m_stAscPatternTable[i].m_byPatternStage, C_N_MAX_PATTERN_STAGE_CHAIN);
			memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
	        tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == PATTERN_STAGE_CHAINTIME  || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValueLength = 32;
	        tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			for(int jpatternIndex = 0; jpatternIndex<32; jpatternIndex++)
			{
				tDataConfig.m_byDataValue[jpatternIndex] = m_tAscParamInfo.m_stAscPatternTable[i].m_byPatternStageTime[31-jpatternIndex];
			}
			//memcpy(tDataConfig.m_byDataValue, m_tAscParamInfo.m_stAscPatternTable[i].m_byPatternStageTime, C_N_MAX_PATTERN_STAGE_CHAIN);
			memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
	        tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == PATTERN_STAGE_TYPE || tDataConfig.m_byAttributeID == 0)
		{
			//for (int j = 0;j < C_N_MAX_PATTERN_STAGE_CHAIN;j++)
			//{
			//	if (m_tAscParamInfo.m_stAscPatternTable[i].m_byPatternStage[j]>0)	//HPH 2021.12.08
			//	{
			//		m_tAscParamInfo.m_stAscPatternTable[i].m_byPatternStageOccurType[j] = PHASE_STAGE_TYPE_FIX;
			//	}
			//	//m_tAscParamInfo.m_stAscPatternTable[i].m_byPatternStageOccurType[j] = PHASE_STAGE_TYPE_FIX;
			//}		--按需出现

			tDataConfig.m_byDataValueLength = 16;
	        tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			for(int patternIndex = 0; patternIndex<16; patternIndex++)
			{
				tDataConfig.m_byDataValue[patternIndex] = m_tAscParamInfo.m_stAscPatternTable[i].m_byPatternStageOccurType[15-patternIndex];
			}
			//memcpy(tDataConfig.m_byDataValue, m_tAscParamInfo.m_stAscPatternTable[i].m_byPatternStageOccurType, C_N_MAX_PATTERN_STAGE_CHAIN);
			memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
	        tCorrectReturnData.m_byReturnCount += 1;
		}
	}
}

void  COpenATCCommWithGB20999Thread::QueryPatternConfig(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData)
{
	TDataConfig tDataConfig;
	tDataConfig.m_byIndex = byIndex + 1;
	tDataConfig.m_byDataClassID = m_chUnPackedBuff[C_N_DATACLASS_ID_POS + byIndex * 6];
	tDataConfig.m_byObjectID = m_chUnPackedBuff[C_N_OBJECT_ID_POS + byIndex * 6];
	tDataConfig.m_byAttributeID = m_chUnPackedBuff[C_N_ATTRIBUTE_ID_POS + byIndex * 6];
	tDataConfig.m_byElementID = m_chUnPackedBuff[C_N_ELEMENT_ID_POS + byIndex * 6];
	tDataConfig.m_byDataValueLength = 1;
	tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;

	if (tDataConfig.m_byElementID < 1 || tDataConfig.m_byElementID > 128)
	{
		tDataConfig.m_byDataValue[0] = BAD_VALUE_NULL;
		memcpy(&tWrongReturnData.m_tDataConfig[tWrongReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		tWrongReturnData.m_byReturnCount += 1;
		return;
	}

	bool bFlag = false;
	BYTE byPatternIndex = 0;
	for (int i = 0;i < C_N_MAX_PATTERN_COUNT;i++)
	{
		if (m_tAscParamInfo.m_stAscPatternTable[i].m_byPatternNumber == tDataConfig.m_byElementID)
		{
			byPatternIndex = i;
			bFlag = true;
			break;
		}
	}

	if (bFlag)
	{
		if (tDataConfig.m_byAttributeID == PATTERN_ID || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValueLength = 1;
	        tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			tDataConfig.m_byDataValue[0] = tDataConfig.m_byElementID;
			memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
	        tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == PATTERN_ROADID || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValueLength = 1;
	        tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			tDataConfig.m_byDataValue[0] = m_tAscParamInfo.m_stAscPatternTable[byPatternIndex].m_byRoadID;
			memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
	        tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == PATTERN_CYCLELEN || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValueLength = 4;
	        tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			int nTemp = ntohl(m_tAscParamInfo.m_stAscPatternTable[byPatternIndex].m_wPatternCycleTime);
			memcpy(tDataConfig.m_byDataValue, &nTemp, 4);
			memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
	        tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == PATTERN_ADJUST_STAGEID || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValueLength = 1;
	        tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			tDataConfig.m_byDataValue[0] = m_tAscParamInfo.m_stAscPatternTable[byPatternIndex].m_byCoorditionStage;
			memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
	        tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == PATTERN_OFFSET || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValueLength = 2;
	        tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			short nTemp = ntohs(m_tAscParamInfo.m_stAscPatternTable[byPatternIndex].m_byPatternOffsetTime);
			memcpy(tDataConfig.m_byDataValue, &nTemp, 4);
			memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
	        tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == PATTERN_STAGE_CHAIN || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValueLength = 16;
	        tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			for(int iPatternindex = 0;iPatternindex<16;iPatternindex++)
			{
				tDataConfig.m_byDataValue[iPatternindex] = m_tAscParamInfo.m_stAscPatternTable[byPatternIndex].m_byPatternStage[15 - iPatternindex];
			}
			//memcpy(tDataConfig.m_byDataValue, m_tAscParamInfo.m_stAscPatternTable[byPatternIndex].m_byPatternStage, C_N_MAX_PATTERN_STAGE_CHAIN);
			memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
	        tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == PATTERN_STAGE_CHAINTIME  || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValueLength = 32;
	        tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			for(int jPatternindex = 0;jPatternindex<32;jPatternindex++)
			{
				tDataConfig.m_byDataValue[jPatternindex] = m_tAscParamInfo.m_stAscPatternTable[byPatternIndex].m_byPatternStageTime[31 - jPatternindex];
			}
			//memcpy(tDataConfig.m_byDataValue, m_tAscParamInfo.m_stAscPatternTable[byPatternIndex].m_byPatternStageTime, C_N_MAX_PATTERN_STAGE_CHAIN);
			memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
	        tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == PATTERN_STAGE_TYPE || tDataConfig.m_byAttributeID == 0)
		{
			//for (int i = 0;i < C_N_MAX_PATTERN_STAGE_CHAIN;i++)
			//{
			//	if (m_tAscParamInfo.m_stAscPatternTable[byPatternIndex].m_byPatternStage[i]>0)	//HPH 2021.12.08
			//	{
			//		m_tAscParamInfo.m_stAscPatternTable[byPatternIndex].m_byPatternStageOccurType[i] = PHASE_STAGE_TYPE_FIX;
			//	}
			//	//m_tAscParamInfo.m_stAscPatternTable[byPatternIndex].m_byPatternStageOccurType[i] = PHASE_STAGE_TYPE_FIX;
			//}		--按需出现

			tDataConfig.m_byDataValueLength = 16;
	        tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			for(int Patternindex = 0;Patternindex<16;Patternindex++)
			{
				tDataConfig.m_byDataValue[Patternindex] = m_tAscParamInfo.m_stAscPatternTable[byPatternIndex].m_byPatternStageOccurType[15 - Patternindex];
			}
			//memcpy(tDataConfig.m_byDataValue, m_tAscParamInfo.m_stAscPatternTable[byPatternIndex].m_byPatternStageOccurType, C_N_MAX_PATTERN_STAGE_CHAIN);
			memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
	        tCorrectReturnData.m_byReturnCount += 1;
		}
	}
	else
	{
		tDataConfig.m_byDataValue[0] = BAD_VALUE_NULL;
		memcpy(&tWrongReturnData.m_tDataConfig[tWrongReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		tWrongReturnData.m_byReturnCount += 1;
	}
}

void COpenATCCommWithGB20999Thread::QueryAllElementTransitionRetainConfig(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData)
{
	for (int i = 0;i < m_tAscParamInfo.m_stTransitBoundValidSize;i++)
	{
		TDataConfig tDataConfig;
		//tDataConfig.m_byIndex = byIndex + 1;
		tDataConfig.m_byIndex = i + 1;	//HPH 2021.12.07
		tDataConfig.m_byDataClassID = m_chUnPackedBuff[C_N_DATACLASS_ID_POS + byIndex * 6];
		tDataConfig.m_byObjectID = m_chUnPackedBuff[C_N_OBJECT_ID_POS + byIndex * 6];
		tDataConfig.m_byAttributeID = m_chUnPackedBuff[C_N_ATTRIBUTE_ID_POS + byIndex * 6];
		tDataConfig.m_byElementID = i + 1;

		if (tDataConfig.m_byAttributeID == PHASE_TRANSITIONSTAGE_ID || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValueLength = 1;
			tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			tDataConfig.m_byDataValue[0] = tDataConfig.m_byElementID;
			memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == PHASE_TRANSITIONSTAGE_RETAIN || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValueLength = C_N_MAX_TRANSIT_RETAIN;
			tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			//对应设置时的转换 HPH--2021.11.18
			BYTE TransitBound[MAX_PHASE_COUNT_20999];
			memset(TransitBound, 0x00 ,MAX_PHASE_COUNT_20999);
			for (int jTransitBoundCount = 0; jTransitBoundCount< m_tAscParamInfo.m_stPhaseStageValidSize; jTransitBoundCount++)
			{
				TransitBound[63-jTransitBoundCount] = m_tAscParamInfo.m_stTransitBound[i].m_byTransitBound[jTransitBoundCount];
			}
			memcpy(tDataConfig.m_byDataValue, TransitBound, C_N_MAX_TRANSIT_RETAIN);
			//memcpy(tDataConfig.m_byDataValue, m_tAscParamInfo.m_stTransitBound[i].m_byTransitBound, C_N_MAX_TRANSIT_RETAIN);
			memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
		}
	}
}

void  COpenATCCommWithGB20999Thread::QueryTransitionRetainConfig(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData)
{
	TDataConfig tDataConfig;
	tDataConfig.m_byIndex = byIndex + 1;
	tDataConfig.m_byDataClassID = m_chUnPackedBuff[C_N_DATACLASS_ID_POS + byIndex * 6];
	tDataConfig.m_byObjectID = m_chUnPackedBuff[C_N_OBJECT_ID_POS + byIndex * 6];
	tDataConfig.m_byAttributeID = m_chUnPackedBuff[C_N_ATTRIBUTE_ID_POS + byIndex * 6];
	tDataConfig.m_byElementID = m_chUnPackedBuff[C_N_ELEMENT_ID_POS + byIndex * 6];
	tDataConfig.m_byDataValueLength = 1;
	tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;

	if (tDataConfig.m_byElementID < 1 || tDataConfig.m_byElementID > 64)
	{
		tDataConfig.m_byDataValue[0] = BAD_VALUE_NULL;
		memcpy(&tWrongReturnData.m_tDataConfig[tWrongReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		tWrongReturnData.m_byReturnCount += 1;
		return;
	}

	int  byTransitionRetainIndex = tDataConfig.m_byElementID - 1;
	for (int i = 0;i < m_tAscParamInfo.m_stTransitBoundValidSize;i++)
	{
		if (m_tAscParamInfo.m_stTransitBound[i].m_byPhaseStageNumber == tDataConfig.m_byElementID)
		{
			byTransitionRetainIndex = i;
		    break;
		}
	}

	if (byTransitionRetainIndex == -1)
	{
		tDataConfig.m_byDataValue[0] = BAD_VALUE_NULL;
		memcpy(&tWrongReturnData.m_tDataConfig[tWrongReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		tWrongReturnData.m_byReturnCount += 1;
		return;
	}

	if (tDataConfig.m_byAttributeID == PHASE_TRANSITIONSTAGE_ID || tDataConfig.m_byAttributeID == 0)
	{
		tDataConfig.m_byDataValueLength = 1;
	    tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
	    tDataConfig.m_byDataValue[0] = tDataConfig.m_byElementID;
		memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		tCorrectReturnData.m_byReturnCount += 1;
	}
	if (tDataConfig.m_byAttributeID == PHASE_TRANSITIONSTAGE_RETAIN || tDataConfig.m_byAttributeID == 0)
	{
		tDataConfig.m_byDataValueLength = C_N_MAX_TRANSIT_RETAIN;
	    tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
		//对应设置时的转换 HPH--2021.11.18
		BYTE TransitBound[MAX_PHASE_COUNT_20999];
		memset(TransitBound, 0x00 ,MAX_PHASE_COUNT_20999);
		for (int jTransitBoundCount = 0; jTransitBoundCount< m_tAscParamInfo.m_stPhaseStageValidSize; jTransitBoundCount++)
		{
			TransitBound[63-jTransitBoundCount] = m_tAscParamInfo.m_stTransitBound[byTransitionRetainIndex].m_byTransitBound[jTransitBoundCount];
		}
		//memcpy(tDataConfig.m_byDataValue, m_tAscParamInfo.m_stTransitBound[byTransitionRetainIndex].m_byTransitBound, C_N_MAX_TRANSIT_RETAIN);
		memcpy(tDataConfig.m_byDataValue, TransitBound, C_N_MAX_TRANSIT_RETAIN);
		memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		tCorrectReturnData.m_byReturnCount += 1;
	}
}

void COpenATCCommWithGB20999Thread::QueryDayPlanCount(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData)
{
	TDataConfig tDataConfig;
	tDataConfig.m_byIndex = byIndex + 1;
	tDataConfig.m_byDataClassID = m_chUnPackedBuff[C_N_DATACLASS_ID_POS + byIndex * 6];
	tDataConfig.m_byObjectID = m_chUnPackedBuff[C_N_OBJECT_ID_POS + byIndex * 6];
	tDataConfig.m_byAttributeID = m_chUnPackedBuff[C_N_ATTRIBUTE_ID_POS + byIndex * 6];
	tDataConfig.m_byElementID = m_chUnPackedBuff[C_N_ELEMENT_ID_POS + byIndex * 6];
	tDataConfig.m_byDataValueLength = 1;
	tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;

	tDataConfig.m_byDataValue[0] = m_tAscParamInfo.m_stDayPlanValidSize;
	memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
	tCorrectReturnData.m_byReturnCount += 1;
}

void COpenATCCommWithGB20999Thread::QueryAllElementDayPlanConfig(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData)
{
	int  i = 0, j = 0;
	for (i = 0;i < m_tAscParamInfo.m_stDayPlanValidSize;i++)
	{
		TDataConfig tDataConfig;
		//tDataConfig.m_byIndex = byIndex + 1;
		tDataConfig.m_byIndex = i + 1;	//HPH 2021.12.07
		tDataConfig.m_byDataClassID = m_chUnPackedBuff[C_N_DATACLASS_ID_POS + byIndex * 6];
		tDataConfig.m_byObjectID = m_chUnPackedBuff[C_N_OBJECT_ID_POS + byIndex * 6];
		tDataConfig.m_byAttributeID = m_chUnPackedBuff[C_N_ATTRIBUTE_ID_POS + byIndex * 6];
		tDataConfig.m_byElementID = i + 1;
		tDataConfig.m_byDataValueLength = 1;
		tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;

		if (tDataConfig.m_byAttributeID == DAYPLAN_ID || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValueLength = 1;
	        tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			tDataConfig.m_byDataValue[0] = tDataConfig.m_byElementID;
			memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		    tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == DAYPLAN_ROADID || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValueLength = 1;
	        tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			tDataConfig.m_byDataValue[0] = m_tAscParamInfo.m_stDayPlanInfo[i].m_byRoadID;
			memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		    tCorrectReturnData.m_byReturnCount += 1;
		}
		 if (tDataConfig.m_byAttributeID == DAYPLAN_STARTTIMECHAIN || tDataConfig.m_byAttributeID == 0)
		 {
			 tDataConfig.m_byDataValueLength = C_N_MAX_TIMECHAIN_COUNT;
	         tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			 for (j = 0;j < C_N_MAX_TIMECHAIN_COUNT;j++)
			 {
			     tDataConfig.m_byDataValue[j] = m_tAscParamInfo.m_stDayPlanInfo[i].m_byDayPlanStartTime[C_N_MAX_TIMECHAIN_COUNT - j - 1];			//大端2022.07.07
			 }
			 memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		     tCorrectReturnData.m_byReturnCount += 1;
		 }
		 if (tDataConfig.m_byAttributeID == DAYPLAN_PATTERNCHAIN || tDataConfig.m_byAttributeID == 0)
		 {
			 tDataConfig.m_byDataValueLength = C_N_MAX_PATTERNCHAIN_COUNT;
	         tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			 for (j = 0;j < C_N_MAX_PATTERNCHAIN_COUNT;j++)
			 {
				 tDataConfig.m_byDataValue[j] = m_tAscParamInfo.m_stDayPlanInfo[i].m_byDayPlanActionPattern[C_N_MAX_PATTERNCHAIN_COUNT - j - 1];	//大端2022.07.07
			 }
			 memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		     tCorrectReturnData.m_byReturnCount += 1;
		 }
		 if (tDataConfig.m_byAttributeID == DAYPLAN_RUNMODECHAIN || tDataConfig.m_byAttributeID == 0)
		 {
			 tDataConfig.m_byDataValueLength = C_N_MAX_RUNMODECHAIN_COUNT;
	         tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			 for (j = 0;j < C_N_MAX_RUNMODECHAIN_COUNT;j++)
			 {
				 tDataConfig.m_byDataValue[j] = m_tAscParamInfo.m_stDayPlanInfo[i].m_byDayPlanRunMode[C_N_MAX_RUNMODECHAIN_COUNT - j - 1];			//大端2022.07.07
			 }
			 memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		     tCorrectReturnData.m_byReturnCount += 1;
		 }
		 if (tDataConfig.m_byAttributeID == DAYPLAN_TIMESPANACTCHAIN1 || tDataConfig.m_byAttributeID == 0)
		 {
			 tDataConfig.m_byDataValueLength = C_N_MAX_ACTCHAIN_COUNT;
	         tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			 for (j = 0;j < C_N_MAX_ACTCHAIN_COUNT;j++)
			 {
			     tDataConfig.m_byDataValue[j] = m_tAscParamInfo.m_stDayPlanInfo[i].m_byActionChainOne[j];
			 }
			 memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		     tCorrectReturnData.m_byReturnCount += 1;
		 }
		 if (tDataConfig.m_byAttributeID == DAYPLAN_TIMESPANACTCHAIN2 || tDataConfig.m_byAttributeID == 0)
		 {
			 tDataConfig.m_byDataValueLength = C_N_MAX_ACTCHAIN_COUNT;
	         tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			 for (j = 0;j < C_N_MAX_ACTCHAIN_COUNT;j++)
			 {
				 tDataConfig.m_byDataValue[j] = m_tAscParamInfo.m_stDayPlanInfo[i].m_byActionChainTwo[j];
			 }
			 memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		     tCorrectReturnData.m_byReturnCount += 1;
		 }
		 if (tDataConfig.m_byAttributeID == DAYPLAN_TIMESPANACTCHAIN3 || tDataConfig.m_byAttributeID == 0)
		 {
			 tDataConfig.m_byDataValueLength = C_N_MAX_ACTCHAIN_COUNT;
	         tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			 for (j = 0;j < C_N_MAX_ACTCHAIN_COUNT;j++)
			 {
				 tDataConfig.m_byDataValue[j] = m_tAscParamInfo.m_stDayPlanInfo[i].m_byActionChainThree[j];
			 }
			 memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		     tCorrectReturnData.m_byReturnCount += 1;
		 }
		 if (tDataConfig.m_byAttributeID == DAYPLAN_TIMESPANACTCHAIN4 || tDataConfig.m_byAttributeID == 0)
		 {
			 tDataConfig.m_byDataValueLength = C_N_MAX_ACTCHAIN_COUNT;
	         tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			 for (j = 0;j < C_N_MAX_ACTCHAIN_COUNT;j++)
			 {
				 tDataConfig.m_byDataValue[j] = m_tAscParamInfo.m_stDayPlanInfo[i].m_byActionChainFour[j];
			 }
			 memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		     tCorrectReturnData.m_byReturnCount += 1;
		 }
		 if (tDataConfig.m_byAttributeID == DAYPLAN_TIMESPANACTCHAIN5 || tDataConfig.m_byAttributeID == 0)
		 {
			 tDataConfig.m_byDataValueLength = C_N_MAX_ACTCHAIN_COUNT;
	         tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			 for (j = 0;j < C_N_MAX_ACTCHAIN_COUNT;j++)
			 {
				 tDataConfig.m_byDataValue[j] = m_tAscParamInfo.m_stDayPlanInfo[i].m_byActionChainFive[j];
			 }
			 memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		     tCorrectReturnData.m_byReturnCount += 1;
		 }
		 if (tDataConfig.m_byAttributeID == DAYPLAN_TIMESPANACTCHAIN6 || tDataConfig.m_byAttributeID == 0)
		 {
			 tDataConfig.m_byDataValueLength = C_N_MAX_ACTCHAIN_COUNT;
	         tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			 for (j = 0;j < C_N_MAX_ACTCHAIN_COUNT;j++)
			 {
				 tDataConfig.m_byDataValue[j] = m_tAscParamInfo.m_stDayPlanInfo[i].m_byActionChainSix[j];
			 }
			 memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		     tCorrectReturnData.m_byReturnCount += 1;
		 }
		 if (tDataConfig.m_byAttributeID == DAYPLAN_TIMESPANACTCHAIN7 || tDataConfig.m_byAttributeID == 0)
		 {
			 tDataConfig.m_byDataValueLength = C_N_MAX_ACTCHAIN_COUNT;
	         tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			 for (j = 0;j < C_N_MAX_ACTCHAIN_COUNT;j++)
			 {
				 tDataConfig.m_byDataValue[j] = m_tAscParamInfo.m_stDayPlanInfo[i].m_byActionChainSeven[j];
			 }
			 memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		     tCorrectReturnData.m_byReturnCount += 1;
		 }
		 if (tDataConfig.m_byAttributeID == DAYPLAN_TIMESPANACTCHAIN8 || tDataConfig.m_byAttributeID == 0)
		 {
			 tDataConfig.m_byDataValueLength = C_N_MAX_ACTCHAIN_COUNT;
	         tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			 for (j = 0;j < C_N_MAX_ACTCHAIN_COUNT;j++)
			 {
				 tDataConfig.m_byDataValue[j] = m_tAscParamInfo.m_stDayPlanInfo[i].m_byActionChainEight[j];
			 }
			 memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		     tCorrectReturnData.m_byReturnCount += 1;
		 }
	}
}

void  COpenATCCommWithGB20999Thread::QueryDayPlanConfig(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData)
{
	TDataConfig tDataConfig;
	tDataConfig.m_byIndex = byIndex + 1;
	tDataConfig.m_byDataClassID = m_chUnPackedBuff[C_N_DATACLASS_ID_POS + byIndex * 6];
	tDataConfig.m_byObjectID = m_chUnPackedBuff[C_N_OBJECT_ID_POS + byIndex * 6];
	tDataConfig.m_byAttributeID = m_chUnPackedBuff[C_N_ATTRIBUTE_ID_POS + byIndex * 6];
	tDataConfig.m_byElementID = m_chUnPackedBuff[C_N_ELEMENT_ID_POS + byIndex * 6];
	tDataConfig.m_byDataValueLength = 1;
	tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;

	if (tDataConfig.m_byElementID < 1 || tDataConfig.m_byElementID > 128)
	{
		tDataConfig.m_byDataValue[0] = BAD_VALUE_NULL;
		memcpy(&tWrongReturnData.m_tDataConfig[tWrongReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		tWrongReturnData.m_byReturnCount += 1;
		return;
	}

	int  byDayPlanIndex = -1;
	int  i = 0;
	for (i = 0;i < m_tAscParamInfo.m_stDayPlanValidSize;i++)
	{
		if (m_tAscParamInfo.m_stDayPlanInfo[i].m_byDayPlanID == tDataConfig.m_byElementID)
		{
			byDayPlanIndex = i;
			break;
		}
	}

	if (byDayPlanIndex != -1)
	{
		if (tDataConfig.m_byAttributeID == DAYPLAN_ID || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValueLength = 1;
	        tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			tDataConfig.m_byDataValue[0] = tDataConfig.m_byElementID;
			memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		    tCorrectReturnData.m_byReturnCount += 1;
		 }
		 if (tDataConfig.m_byAttributeID == DAYPLAN_ROADID || tDataConfig.m_byAttributeID == 0)
		 {
			 tDataConfig.m_byDataValueLength = 1;
	         tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			 //tDataConfig.m_byDataValue[0] = tDataConfig.m_byElementID;
			 tDataConfig.m_byDataValue[0] = m_tAscParamInfo.m_stDayPlanInfo[byDayPlanIndex].m_byRoadID;
		     memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			 tCorrectReturnData.m_byReturnCount += 1;
		 }
		 if (tDataConfig.m_byAttributeID == DAYPLAN_STARTTIMECHAIN || tDataConfig.m_byAttributeID == 0)
		 {
			 tDataConfig.m_byDataValueLength = C_N_MAX_TIMECHAIN_COUNT;
	         tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			 for (i = 0;i < C_N_MAX_TIMECHAIN_COUNT;i++)
			 {
			     tDataConfig.m_byDataValue[i] = m_tAscParamInfo.m_stDayPlanInfo[byDayPlanIndex].m_byDayPlanStartTime[C_N_MAX_TIMECHAIN_COUNT - i - 1];
			 }
			 memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		     tCorrectReturnData.m_byReturnCount += 1;
		 }
		 if (tDataConfig.m_byAttributeID == DAYPLAN_PATTERNCHAIN || tDataConfig.m_byAttributeID == 0)
		 {
			 tDataConfig.m_byDataValueLength = C_N_MAX_PATTERNCHAIN_COUNT;
	         tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			 for (i = 0;i < C_N_MAX_PATTERNCHAIN_COUNT;i++)
			 {
				 tDataConfig.m_byDataValue[i] = m_tAscParamInfo.m_stDayPlanInfo[byDayPlanIndex].m_byDayPlanActionPattern[C_N_MAX_PATTERNCHAIN_COUNT - i - 1];
			 }
			 memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		     tCorrectReturnData.m_byReturnCount += 1;
		 }
		 if (tDataConfig.m_byAttributeID == DAYPLAN_RUNMODECHAIN || tDataConfig.m_byAttributeID == 0)
		 {
			 tDataConfig.m_byDataValueLength = C_N_MAX_RUNMODECHAIN_COUNT;
	         tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			 for (i = 0;i < C_N_MAX_RUNMODECHAIN_COUNT;i++)
			 {
				 tDataConfig.m_byDataValue[i] = m_tAscParamInfo.m_stDayPlanInfo[byDayPlanIndex].m_byDayPlanRunMode[C_N_MAX_RUNMODECHAIN_COUNT - i - 1];
			 }
			 memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		     tCorrectReturnData.m_byReturnCount += 1;
		 }
		 if (tDataConfig.m_byAttributeID == DAYPLAN_TIMESPANACTCHAIN1 || tDataConfig.m_byAttributeID == 0)
		 {
			 tDataConfig.m_byDataValueLength = C_N_MAX_ACTCHAIN_COUNT;
	         tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			 for (i = 0;i < C_N_MAX_ACTCHAIN_COUNT;i++)
			 {
			     tDataConfig.m_byDataValue[i] = m_tAscParamInfo.m_stDayPlanInfo[byDayPlanIndex].m_byActionChainOne[i];
			 }
			 memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		     tCorrectReturnData.m_byReturnCount += 1;
		 }
		 if (tDataConfig.m_byAttributeID == DAYPLAN_TIMESPANACTCHAIN2 || tDataConfig.m_byAttributeID == 0)
		 {
			 tDataConfig.m_byDataValueLength = C_N_MAX_ACTCHAIN_COUNT;
	         tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			 for (i = 0;i < C_N_MAX_ACTCHAIN_COUNT;i++)
			 {
				 tDataConfig.m_byDataValue[i] = m_tAscParamInfo.m_stDayPlanInfo[byDayPlanIndex].m_byActionChainTwo[i];
			 }
			 memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		     tCorrectReturnData.m_byReturnCount += 1;
		 }
		 if (tDataConfig.m_byAttributeID == DAYPLAN_TIMESPANACTCHAIN3 || tDataConfig.m_byAttributeID == 0)
		 {
			 tDataConfig.m_byDataValueLength = C_N_MAX_ACTCHAIN_COUNT;
	         tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			 for (i = 0;i < C_N_MAX_ACTCHAIN_COUNT;i++)
			 {
				 tDataConfig.m_byDataValue[i] = m_tAscParamInfo.m_stDayPlanInfo[byDayPlanIndex].m_byActionChainThree[i];
			 }
			 memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		     tCorrectReturnData.m_byReturnCount += 1;
		 }
		 if (tDataConfig.m_byAttributeID == DAYPLAN_TIMESPANACTCHAIN4 || tDataConfig.m_byAttributeID == 0)
		 {
			 tDataConfig.m_byDataValueLength = C_N_MAX_ACTCHAIN_COUNT;
	         tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			 for (i = 0;i < C_N_MAX_ACTCHAIN_COUNT;i++)
			 {
				 tDataConfig.m_byDataValue[i] = m_tAscParamInfo.m_stDayPlanInfo[byDayPlanIndex].m_byActionChainFour[i];
			 }
			 memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		     tCorrectReturnData.m_byReturnCount += 1;
		 }
		 if (tDataConfig.m_byAttributeID == DAYPLAN_TIMESPANACTCHAIN5 || tDataConfig.m_byAttributeID == 0)
		 {
			 tDataConfig.m_byDataValueLength = C_N_MAX_ACTCHAIN_COUNT;
	         tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			 for (i = 0;i < C_N_MAX_ACTCHAIN_COUNT;i++)
			 {
				 tDataConfig.m_byDataValue[i] = m_tAscParamInfo.m_stDayPlanInfo[byDayPlanIndex].m_byActionChainFive[i];
			 }
			 memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		     tCorrectReturnData.m_byReturnCount += 1;
		 }
		 if (tDataConfig.m_byAttributeID == DAYPLAN_TIMESPANACTCHAIN6 || tDataConfig.m_byAttributeID == 0)
		 {
			 tDataConfig.m_byDataValueLength = C_N_MAX_ACTCHAIN_COUNT;
	         tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			 for (i = 0;i < C_N_MAX_ACTCHAIN_COUNT;i++)
			 {
				 tDataConfig.m_byDataValue[i] = m_tAscParamInfo.m_stDayPlanInfo[byDayPlanIndex].m_byActionChainSix[i];
			 }
			 memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		     tCorrectReturnData.m_byReturnCount += 1;
		 }
		 if (tDataConfig.m_byAttributeID == DAYPLAN_TIMESPANACTCHAIN7 || tDataConfig.m_byAttributeID == 0)
		 {
			 tDataConfig.m_byDataValueLength = C_N_MAX_ACTCHAIN_COUNT;
	         tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			 for (i = 0;i < C_N_MAX_ACTCHAIN_COUNT;i++)
			 {
				 tDataConfig.m_byDataValue[i] = m_tAscParamInfo.m_stDayPlanInfo[byDayPlanIndex].m_byActionChainSeven[i];
			 }
			 memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		     tCorrectReturnData.m_byReturnCount += 1;
		 }
		 if (tDataConfig.m_byAttributeID == DAYPLAN_TIMESPANACTCHAIN8 || tDataConfig.m_byAttributeID == 0)
		 {
			 tDataConfig.m_byDataValueLength = C_N_MAX_ACTCHAIN_COUNT;
	         tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			 for (i = 0;i < C_N_MAX_ACTCHAIN_COUNT;i++)
			 {
				 tDataConfig.m_byDataValue[i] = m_tAscParamInfo.m_stDayPlanInfo[byDayPlanIndex].m_byActionChainEight[i];
			 }
			 memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		     tCorrectReturnData.m_byReturnCount += 1;
		 }
	}
	else
	{
		tDataConfig.m_byDataValue[0] = BAD_VALUE_NULL;
		memcpy(&tWrongReturnData.m_tDataConfig[tWrongReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		tWrongReturnData.m_byReturnCount += 1;
	}
}

void COpenATCCommWithGB20999Thread::QueryScheduleTableCount(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData)
{
	TDataConfig tDataConfig;
	tDataConfig.m_byIndex = byIndex + 1;
	tDataConfig.m_byDataClassID = m_chUnPackedBuff[C_N_DATACLASS_ID_POS + byIndex * 6];
	tDataConfig.m_byObjectID = m_chUnPackedBuff[C_N_OBJECT_ID_POS + byIndex * 6];
	tDataConfig.m_byAttributeID = m_chUnPackedBuff[C_N_ATTRIBUTE_ID_POS + byIndex * 6];
	tDataConfig.m_byElementID = m_chUnPackedBuff[C_N_ELEMENT_ID_POS + byIndex * 6];
	tDataConfig.m_byDataValueLength = 1;
	tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;

	tDataConfig.m_byDataValue[0] = m_tAscParamInfo.m_stSchedulePlanValidSize;//调度计划数量
	memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
    tCorrectReturnData.m_byReturnCount += 1;
}

void COpenATCCommWithGB20999Thread::QueryAllElementScheduleTableConfig(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData)
{
	for (int i = 0;i < m_tAscParamInfo.m_stSchedulePlanValidSize;i++)					
	{
		TDataConfig tDataConfig;
		//tDataConfig.m_byIndex = byIndex + 1;
		tDataConfig.m_byIndex = i + 1;	//HPH 2021.12.07
		tDataConfig.m_byDataClassID = m_chUnPackedBuff[C_N_DATACLASS_ID_POS + byIndex * 6];
		tDataConfig.m_byObjectID = m_chUnPackedBuff[C_N_OBJECT_ID_POS + byIndex * 6];
		tDataConfig.m_byAttributeID = m_chUnPackedBuff[C_N_ATTRIBUTE_ID_POS + byIndex * 6];
		tDataConfig.m_byElementID = i + 1;
		tDataConfig.m_byDataValueLength = 1;
		tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;

		if (tDataConfig.m_byAttributeID == SCHEDULE_ID || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValue[0] = tDataConfig.m_byElementID;
			memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		    tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == SCHEDULE_ROADID || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValue[0] = m_tAscParamInfo.m_stSchedulePlanInfo[i].m_byRoadID;
			memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == SCHEDULE_PRIORITY || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValue[0] = (BYTE)m_tAscParamInfo.m_stSchedulePlanInfo[i].m_byPriority;
			memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == SCHEDULE_WEEK || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValue[0] = m_tAscParamInfo.m_stSchedulePlanInfo[i].m_byWeek;
			memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == SCHEDULE_MONTH || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValueLength = 2;
			tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			short nTemp = ntohs(m_tAscParamInfo.m_stSchedulePlanInfo[i].m_byMonth);
			memcpy(tDataConfig.m_byDataValue, &nTemp, 2);
			memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == SCHEDULE_DAY || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValueLength = 4;
			tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			unsigned long nTemp = ntohl(m_tAscParamInfo.m_stSchedulePlanInfo[i].m_byDate);
			memcpy(tDataConfig.m_byDataValue, &nTemp, 4);
			memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == SCHEDULE_DAYPLAN_ID || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValue[0] = m_tAscParamInfo.m_stSchedulePlanInfo[i].m_byScheduleOfDayPlanID;
			memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
		}
	}
}

void  COpenATCCommWithGB20999Thread::QueryScheduleTableConfig(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData)
{
	TDataConfig tDataConfig;
	tDataConfig.m_byIndex = byIndex + 1;
	tDataConfig.m_byDataClassID = m_chUnPackedBuff[C_N_DATACLASS_ID_POS + byIndex * 6];
	tDataConfig.m_byObjectID = m_chUnPackedBuff[C_N_OBJECT_ID_POS + byIndex * 6];
	tDataConfig.m_byAttributeID = m_chUnPackedBuff[C_N_ATTRIBUTE_ID_POS + byIndex * 6];
	tDataConfig.m_byElementID = m_chUnPackedBuff[C_N_ELEMENT_ID_POS + byIndex * 6];
	tDataConfig.m_byDataValueLength = 1;
	tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;

	if (tDataConfig.m_byElementID < 1 || tDataConfig.m_byElementID > 128)
	{
		tDataConfig.m_byDataValue[0] = BAD_VALUE_NULL;
		memcpy(&tWrongReturnData.m_tDataConfig[tWrongReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		tWrongReturnData.m_byReturnCount += 1;
		return;
	}

	 int  nScheduleIndex = -1;
     for (int i = 0;i < m_tAscParamInfo.m_stSchedulePlanValidSize;i++)
	 {
	     if (m_tAscParamInfo.m_stSchedulePlanInfo[i].m_SchedulePlanID == tDataConfig.m_byElementID)
		 {
			 nScheduleIndex = i;
			 break;
		 }
	 }

	if (nScheduleIndex == -1)
	{
		tDataConfig.m_byDataValue[0] = BAD_VALUE_NULL;
		memcpy(&tWrongReturnData.m_tDataConfig[tWrongReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		tWrongReturnData.m_byReturnCount += 1;
		return;
	}

	if (tDataConfig.m_byAttributeID == SCHEDULE_ID || tDataConfig.m_byAttributeID == 0)
	{
	    tDataConfig.m_byDataValue[0] = tDataConfig.m_byElementID;
		memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
	    tCorrectReturnData.m_byReturnCount += 1;
	}
	if (tDataConfig.m_byAttributeID == SCHEDULE_ROADID || tDataConfig.m_byAttributeID == 0)
	{
		tDataConfig.m_byDataValue[0] = m_tAscParamInfo.m_stSchedulePlanInfo[nScheduleIndex].m_byRoadID;
		memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
	    tCorrectReturnData.m_byReturnCount += 1;
	}
	if (tDataConfig.m_byAttributeID == SCHEDULE_PRIORITY || tDataConfig.m_byAttributeID == 0)
	{
		tDataConfig.m_byDataValue[0] = (BYTE)m_tAscParamInfo.m_stSchedulePlanInfo[nScheduleIndex].m_byPriority;
		memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
	    tCorrectReturnData.m_byReturnCount += 1;
	}
	if (tDataConfig.m_byAttributeID == SCHEDULE_WEEK || tDataConfig.m_byAttributeID == 0)
	{
		tDataConfig.m_byDataValue[0] = m_tAscParamInfo.m_stSchedulePlanInfo[nScheduleIndex].m_byWeek;
		memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
	    tCorrectReturnData.m_byReturnCount += 1;
	}
	if (tDataConfig.m_byAttributeID == SCHEDULE_MONTH || tDataConfig.m_byAttributeID == 0)
	{
		tDataConfig.m_byDataValueLength = 2;
	    tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
		short nTemp = ntohs(m_tAscParamInfo.m_stSchedulePlanInfo[nScheduleIndex].m_byMonth);
		memcpy(tDataConfig.m_byDataValue, &nTemp, 2);
		memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
	    tCorrectReturnData.m_byReturnCount += 1;
	}
	if (tDataConfig.m_byAttributeID == SCHEDULE_DAY || tDataConfig.m_byAttributeID == 0)
	{
		tDataConfig.m_byDataValueLength = 4;
	    tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
		unsigned long nTemp = ntohl(m_tAscParamInfo.m_stSchedulePlanInfo[nScheduleIndex].m_byDate);
		memcpy(tDataConfig.m_byDataValue, &nTemp, 4);
		memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
	    tCorrectReturnData.m_byReturnCount += 1;
	}
	if (tDataConfig.m_byAttributeID == SCHEDULE_DAYPLAN_ID || tDataConfig.m_byAttributeID == 0)
	{
		tDataConfig.m_byDataValue[0] = m_tAscParamInfo.m_stSchedulePlanInfo[nScheduleIndex].m_byScheduleOfDayPlanID;
		memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
	    tCorrectReturnData.m_byReturnCount += 1;
	}
}

void  COpenATCCommWithGB20999Thread::QueryDeveiceStatus(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData)
{
	TDataConfig tDataConfig;
	tDataConfig.m_byIndex = byIndex + 1;
	tDataConfig.m_byDataClassID = m_chUnPackedBuff[C_N_DATACLASS_ID_POS + byIndex * 6];
	tDataConfig.m_byObjectID = m_chUnPackedBuff[C_N_OBJECT_ID_POS + byIndex * 6];
	tDataConfig.m_byAttributeID = m_chUnPackedBuff[C_N_ATTRIBUTE_ID_POS + byIndex * 6];
	tDataConfig.m_byElementID = m_chUnPackedBuff[C_N_ELEMENT_ID_POS + byIndex * 6];
	tDataConfig.m_byDataValueLength = 1;
	tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;

	int i = 0;
	TDeviceStatus tDeviceStatus;
	memset(&tDeviceStatus, 0x00, sizeof(tDeviceStatus));
	m_pOpenATCRunStatus->GetDeviceStatus(tDeviceStatus);

	if (tDataConfig.m_byAttributeID == DETECTOR_STATUS || tDataConfig.m_byAttributeID == 0)
	{
		TVehDetBoardData tVehDetData;
		m_pOpenATCRunStatus->GetVehDetBoardData(tVehDetData);

		TCanData tDetectorStatus[C_N_MAX_PHASE_DETECTOR];
	    memset(tDetectorStatus, 0, sizeof(tDetectorStatus));

		for (i = 0;i < MAX_VEHICLEDETECTOR_COUNT;i++)
		{
			if (tVehDetData.m_atVehDetData[i / C_N_MAXDETINPUT_NUM].m_bDetFaultStatus[i % C_N_MAXDETINPUT_NUM])
			{
				if (i % C_N_MAX_PHASE_DETECTOR == 0)
				{
					tDetectorStatus[i / C_N_MAXDETINPUT_NUM].CanData.io1 = 1;
				}
				else if (i % C_N_MAX_PHASE_DETECTOR == 1)
				{
					tDetectorStatus[i / C_N_MAXDETINPUT_NUM].CanData.io2 = 1;
				}
				else if (i % C_N_MAX_PHASE_DETECTOR == 2)
				{
					tDetectorStatus[i / C_N_MAXDETINPUT_NUM].CanData.io3 = 1;
				}
				else if (i % C_N_MAX_PHASE_DETECTOR == 3)
				{
					tDetectorStatus[i / C_N_MAXDETINPUT_NUM].CanData.io4 = 1;
				}
				else if (i % C_N_MAX_PHASE_DETECTOR == 4)
				{
					tDetectorStatus[i / C_N_MAXDETINPUT_NUM].CanData.io5 = 1;
				}
				else if (i % C_N_MAX_PHASE_DETECTOR == 5)
				{
					tDetectorStatus[i / C_N_MAXDETINPUT_NUM].CanData.io6 = 1;
				}
				else if (i % C_N_MAX_PHASE_DETECTOR == 6)
				{
					tDetectorStatus[i / C_N_MAXDETINPUT_NUM].CanData.io7 = 1;
				}
				else if (i % C_N_MAX_PHASE_DETECTOR == 7)
				{
					tDetectorStatus[i / C_N_MAXDETINPUT_NUM].CanData.io8 = 1;
				}
			}
		}
		tDataConfig.m_byDataValueLength = C_N_MAX_PHASE_DETECTOR;
	    tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
		for (i = 0;i < C_N_MAX_PHASE_DETECTOR;i++)
		{
			tDataConfig.m_byDataValue[i] = tDetectorStatus[i].chCanData;
		}
		memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
	    tCorrectReturnData.m_byReturnCount += 1;
	}
	if (tDataConfig.m_byAttributeID == DEVICEMODULE_STATUS || tDataConfig.m_byAttributeID == 0)
	{
		TOpenATCStatusInfo tOpenATCStatusInfo;
		memset(&tOpenATCStatusInfo, 0x00, sizeof(tOpenATCStatusInfo));
		m_pOpenATCRunStatus->GetOpenATCStatusInfo(tOpenATCStatusInfo);

		TCanData tDeviceModuleStatus[C_N_MAX_DEVICE_MODULE];
	    memset(tDeviceModuleStatus, 0, sizeof(tDeviceModuleStatus));

		//主控板状态
		tDeviceModuleStatus[0].CanData.io1 = 0;
		tDeviceModuleStatus[0].CanData.io2 = 0;
		tDeviceModuleStatus[0].CanData.io3 = 0;
		tDeviceModuleStatus[0].CanData.io4 = 0;
		tDeviceModuleStatus[0].CanData.io5 = 0;
		tDeviceModuleStatus[0].CanData.io6 = 0;
		tDeviceModuleStatus[0].CanData.io7 = 0;
		tDeviceModuleStatus[0].CanData.io8 = 0;
		tDeviceModuleStatus[1].CanData.io1 = 0;
		tDeviceModuleStatus[1].CanData.io2 = 0;
		//相位板状态
		tDeviceModuleStatus[1].CanData.io3 = m_pOpenATCRunStatus->GetLampCtlBoardOffLine(0);
		tDeviceModuleStatus[1].CanData.io4 = m_pOpenATCRunStatus->GetLampCtlBoardOffLine(1);
		tDeviceModuleStatus[1].CanData.io5 = m_pOpenATCRunStatus->GetLampCtlBoardOffLine(2);
		tDeviceModuleStatus[1].CanData.io6 = m_pOpenATCRunStatus->GetLampCtlBoardOffLine(3);
		//车检板状态
		tDeviceModuleStatus[5].CanData.io1 = m_pOpenATCRunStatus->GetDetBoardOffLine(0);
		tDeviceModuleStatus[5].CanData.io2 = m_pOpenATCRunStatus->GetDetBoardOffLine(1);
		tDeviceModuleStatus[5].CanData.io3 = m_pOpenATCRunStatus->GetDetBoardOffLine(0);
		tDeviceModuleStatus[5].CanData.io4 = m_pOpenATCRunStatus->GetDetBoardOffLine(1);

		tDataConfig.m_byDataValueLength = C_N_MAX_DEVICE_MODULE;
	    tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
		for (i = 0;i < C_N_MAX_DEVICE_MODULE;i++)
		{
			tDataConfig.m_byDataValue[i] = tDeviceModuleStatus[i].chCanData;
		}
		memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
	    tCorrectReturnData.m_byReturnCount += 1;
	}
    if (tDataConfig.m_byAttributeID == ATC_DOOR_STATUS || tDataConfig.m_byAttributeID == 0)
	{
		tDataConfig.m_byDataValueLength = 1;
	    tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
		tDataConfig.m_byDataValue[0] = tDeviceStatus.m_byDoorStatus;
		memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
	    tCorrectReturnData.m_byReturnCount += 1;
	}
	if (tDataConfig.m_byAttributeID == VOLTAGE_VALUE || tDataConfig.m_byAttributeID == 0)
	{
		tDataConfig.m_byDataValueLength = 2;
	    tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
		short nTemp = ntohs(tDeviceStatus.m_nVoltage);
		memcpy(tDataConfig.m_byDataValue, &nTemp, 2);
		memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
	    tCorrectReturnData.m_byReturnCount += 1;
	}
	if (tDataConfig.m_byAttributeID == CURRENT_VALUE || tDataConfig.m_byAttributeID == 0)
	{
		tDataConfig.m_byDataValueLength = 2;
	    tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
		short nTemp = ntohs(tDeviceStatus.m_nCurrent);
		memcpy(tDataConfig.m_byDataValue, &nTemp, 2);
		memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
	    tCorrectReturnData.m_byReturnCount += 1;
	}
	if (tDataConfig.m_byAttributeID == TEMPERATURE_VALUE || tDataConfig.m_byAttributeID == 0)
	{
		tDataConfig.m_byDataValueLength = 1;
	    tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
		tDataConfig.m_byDataValue[0] = 18;//tDeviceStatus.m_chTemperature;
		memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
	    tCorrectReturnData.m_byReturnCount += 1;
	}
	if (tDataConfig.m_byAttributeID == HUMIDITY_VALUE || tDataConfig.m_byAttributeID == 0)
	{
		tDataConfig.m_byDataValueLength = 1;
	    tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
		tDataConfig.m_byDataValue[0] = 10;//tDeviceStatus.m_byHumidity;
		memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
	    tCorrectReturnData.m_byReturnCount += 1;
	}
	if (tDataConfig.m_byAttributeID == WATERLOGIN_VALUE || tDataConfig.m_byAttributeID == 0)
	{
		tDataConfig.m_byDataValueLength = 1;
	    tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
		tDataConfig.m_byDataValue[0] = 0;
		memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
	    tCorrectReturnData.m_byReturnCount += 1;
	}
	if (tDataConfig.m_byAttributeID == SMOKE_VALUE || tDataConfig.m_byAttributeID == 0)
	{
		tDataConfig.m_byDataValueLength = 1;
	    tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
		tDataConfig.m_byDataValue[0] = 0;
		memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
	    tCorrectReturnData.m_byReturnCount += 1;
	}
	if (tDataConfig.m_byAttributeID == STANDARD_TIME || tDataConfig.m_byAttributeID == 0)
	{
#ifndef WIN32
		tzset();  
		time_t current_timet;  
		time(&current_timet);//得到当前时间秒数   
		gmtime_r(&current_timet, &Utc_tm);//得到GMT，即UTC时间  
		int nYear = ntohs(Utc_tm.tm_year + 1900);
		tDataConfig.m_byDataValueLength = 7;
	    tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
		memcpy(tDataConfig.m_byDataValue, &nYear, 2);               //年
		tDataConfig.m_byDataValue[2] = Utc_tm.tm_mon + 1;           //月	
		tDataConfig.m_byDataValue[3] = Utc_tm.tm_mday;              //日	
		tDataConfig.m_byDataValue[4] = Utc_tm.tm_hour;              //时
		tDataConfig.m_byDataValue[5] = Utc_tm.tm_min;               //分
		tDataConfig.m_byDataValue[6] = Utc_tm.tm_sec;               //秒
		memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
	    tCorrectReturnData.m_byReturnCount += 1;
#else
		tDataConfig.m_byDataValueLength = 7;
		tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
		GetSystemTime(&m_stUTCTime);
		short nYear = ntohs(m_stUTCTime.wYear);
		memcpy(tDataConfig.m_byDataValue, &nYear, 2);                //年
		tDataConfig.m_byDataValue[2] = m_stUTCTime.wMonth;           //月	
		tDataConfig.m_byDataValue[3] = m_stUTCTime.wDay;             //日	
		tDataConfig.m_byDataValue[4] = m_stUTCTime.wHour;            //时
		tDataConfig.m_byDataValue[5] = m_stUTCTime.wMinute;          //分
		tDataConfig.m_byDataValue[6] = m_stUTCTime.wSecond;          //秒
		memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		tCorrectReturnData.m_byReturnCount += 1;
#endif
	
	}
	if (tDataConfig.m_byAttributeID == LOCAL_TIME || tDataConfig.m_byAttributeID == 0)
	{
		//linux下无SYSTEMTIME
#ifndef _WIN32
		tzset();  
		time_t current_timet;  
		time(&current_timet);//得到当前时间秒数  
		localtime_r(&current_timet, &Local_tm);//得到GMT，即UTC时间  
		int nYear = ntohs(Local_tm.tm_year + 1900);
		tDataConfig.m_byDataValueLength = 7;
		tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
		memcpy(tDataConfig.m_byDataValue, &nYear, 2);               //年
		tDataConfig.m_byDataValue[2] = Local_tm.tm_mon + 1;           //月	
		tDataConfig.m_byDataValue[3] = Local_tm.tm_mday;              //日	
		tDataConfig.m_byDataValue[4] = Local_tm.tm_hour;              //时
		tDataConfig.m_byDataValue[5] = Local_tm.tm_min;               //分
		tDataConfig.m_byDataValue[6] = Local_tm.tm_sec;               //秒
		memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		tCorrectReturnData.m_byReturnCount += 1;
#else
		tDataConfig.m_byDataValueLength = 7;
		tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
		GetLocalTime(&m_stLocalTime);
		short nYear = ntohs(m_stLocalTime.wYear);
		memcpy(tDataConfig.m_byDataValue, &nYear, 2);                //年
		tDataConfig.m_byDataValue[2] = m_stLocalTime.wMonth;         //月	
		tDataConfig.m_byDataValue[3] = m_stLocalTime.wDay;           //日	
		tDataConfig.m_byDataValue[4] = m_stLocalTime.wHour;          //时
		tDataConfig.m_byDataValue[5] = m_stLocalTime.wMinute;        //分
		tDataConfig.m_byDataValue[6] = m_stLocalTime.wSecond;        //秒
		memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		tCorrectReturnData.m_byReturnCount += 1;
#endif
	}
}

void  COpenATCCommWithGB20999Thread::QueryControlStatus(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData)
{
	TDataConfig tDataConfig;
	tDataConfig.m_byIndex = byIndex + 1;
	tDataConfig.m_byDataClassID = m_chUnPackedBuff[C_N_DATACLASS_ID_POS + byIndex * 6];
	tDataConfig.m_byObjectID = m_chUnPackedBuff[C_N_OBJECT_ID_POS + byIndex * 6];
	tDataConfig.m_byAttributeID = m_chUnPackedBuff[C_N_ATTRIBUTE_ID_POS + byIndex * 6];
	tDataConfig.m_byElementID = m_chUnPackedBuff[C_N_ELEMENT_ID_POS + byIndex * 6];
	tDataConfig.m_byDataValueLength = 1;
	tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;

	if (tDataConfig.m_byElementID < 1 || tDataConfig.m_byElementID > 8)
	{
		tDataConfig.m_byDataValue[0] = BAD_VALUE_NULL;
		memcpy(&tWrongReturnData.m_tDataConfig[tWrongReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		tWrongReturnData.m_byReturnCount += 1;
		return;
	}

	//HPH 2021.12.09
	m_pOpenATCRunStatus->SetPhaseRunStatusWriteFlag(true);
	OpenATCSleep(100);
	bool bIsReadFlag = true;
	if (m_pOpenATCRunStatus->GetPhaseRunStatusReadFlag())
	{
		m_pOpenATCRunStatus->SetPhaseRunStatusReadFlag(false);
		m_pOpenATCRunStatus->SetPhaseRunStatusWriteFlag(false);
		m_pOpenATCRunStatus->SetPhaseRunStatusReadFlag(true);
		m_pOpenATCRunStatus->SetPhaseRunStatusWriteFlag(true);
		bIsReadFlag = false;
	}
	else
	{
		OpenATCSleep(100);
		if (m_pOpenATCRunStatus->GetPhaseRunStatusReadFlag())
		{
			m_pOpenATCRunStatus->SetPhaseRunStatusReadFlag(false);
			m_pOpenATCRunStatus->SetPhaseRunStatusWriteFlag(false);
			m_pOpenATCRunStatus->SetPhaseRunStatusReadFlag(true);
			m_pOpenATCRunStatus->SetPhaseRunStatusWriteFlag(true);
			bIsReadFlag = false;
		}
	}	
	//HPH 2021.12.09

	TPhaseRunStatus tPhaseRunStatus;
	memset(&tPhaseRunStatus, 0x00, sizeof(tPhaseRunStatus));
	m_pOpenATCRunStatus->GetPhaseRunStatus(tPhaseRunStatus);

	if(tDataConfig.m_byElementID == 1)
	{

		if (tDataConfig.m_byAttributeID == CONTROL_ROADID || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValueLength = 1;
			tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			tDataConfig.m_byDataValue[0] = tDataConfig.m_byElementID;
			memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == ROAD_RUNMODE || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValueLength = 1;
			tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			tDataConfig.m_byDataValue[0] = TransRunModeToHost(tPhaseRunStatus.m_nCurCtlMode, tPhaseRunStatus.m_nCurCtlPattern);
			memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == ROAD_PATTERN || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValueLength = 1;
			tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			//tDataConfig.m_byDataValue[0] = tPhaseRunStatus.m_nCurCtlPattern;
			tDataConfig.m_byDataValue[0] = tPhaseRunStatus.m_byPlanID;	//HPH 2021.12.09
			memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == ROAD_STAGE || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValueLength = 1;
			tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			tDataConfig.m_byDataValue[0] = m_tAscParamInfo.m_stAscPatternTable[tPhaseRunStatus.m_byPlanID - 1].m_byPatternStage[tPhaseRunStatus.m_atRingRunStatus[0].m_nCurStageIndex];
			//tDataConfig.m_byDataValue[0] = tPhaseRunStatus.m_atRingRunStatus[0].m_nCurStageIndex + 1;
			memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
		}
	}
	//else	//路口2
	//{
	//	TRoadTwoRunStatus tRoadTwoRunStatus;
	//	memset(&tRoadTwoRunStatus,0,sizeof(tRoadTwoRunStatus));
	//	m_pOpenATCRunStatus->GetRoadTwoRunStatus(tRoadTwoRunStatus);
	//	if (tDataConfig.m_byAttributeID == CONTROL_ROADID || tDataConfig.m_byAttributeID == 0)
	//	{
	//		tDataConfig.m_byDataValueLength = 1;
	//		tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
	//		tDataConfig.m_byDataValue[0] = tDataConfig.m_byElementID;
	//		memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
	//		tCorrectReturnData.m_byReturnCount += 1;
	//	}
	//	if (tDataConfig.m_byAttributeID == ROAD_RUNMODE || tDataConfig.m_byAttributeID == 0)
	//	{
	//		tDataConfig.m_byDataValueLength = 1;
	//		tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
	//		tDataConfig.m_byDataValue[0] = TransRunModeToHost(tRoadTwoRunStatus.m_nControlMode, tRoadTwoRunStatus.m_nCurCtlPattern);
	//		memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
	//		tCorrectReturnData.m_byReturnCount += 1;
	//	}
	//	if (tDataConfig.m_byAttributeID == ROAD_PATTERN || tDataConfig.m_byAttributeID == 0)
	//	{
	//		tDataConfig.m_byDataValueLength = 1;
	//		tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
	//		//tDataConfig.m_byDataValue[0] = tPhaseRunStatus.m_nCurCtlPattern;
	//		tDataConfig.m_byDataValue[0] = tRoadTwoRunStatus.m_nPatternNo;	//HPH 2021.12.09
	//		memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
	//		tCorrectReturnData.m_byReturnCount += 1;
	//	}
	//	if (tDataConfig.m_byAttributeID == ROAD_STAGE || tDataConfig.m_byAttributeID == 0)
	//	{
	//		tDataConfig.m_byDataValueLength = 1;
	//		tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
	//		tDataConfig.m_byDataValue[0] = m_tAscParamInfo.m_stAscPatternTable[tRoadTwoRunStatus.m_nPatternNo - 1].m_byPatternStage[tRoadTwoRunStatus.m_nCurStageIndex];	//待验证
	//		//tDataConfig.m_byDataValue[0] = tRoadTwoRunStatus.m_nCurStageIndex;
	//		memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
	//		tCorrectReturnData.m_byReturnCount += 1;
	//	}
	//}
}
void  COpenATCCommWithGB20999Thread::QueryRealTimeData(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData)
{
	TDataConfig tDataConfig;
	tDataConfig.m_byIndex = byIndex + 1;
	tDataConfig.m_byDataClassID = m_chUnPackedBuff[C_N_DATACLASS_ID_POS + byIndex * 6];
	tDataConfig.m_byObjectID = m_chUnPackedBuff[C_N_OBJECT_ID_POS + byIndex * 6];
	tDataConfig.m_byAttributeID = m_chUnPackedBuff[C_N_ATTRIBUTE_ID_POS + byIndex * 6];
	tDataConfig.m_byElementID = m_chUnPackedBuff[C_N_ELEMENT_ID_POS + byIndex * 6];
	tDataConfig.m_byDataValueLength = C_N_MAX_PHASE_DETECTOR;
	tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;

	TRealTimeVehDetData tRTVehDetData;
    m_pOpenATCRunStatus->GetRTVehDetData(tRTVehDetData);

	TCanData tDetectorStatus[C_N_MAX_PHASE_DETECTOR];
	memset(tDetectorStatus, 0, sizeof(tDetectorStatus));

	int  i = 0;
	for (i = 0;i < MAX_VEHICLEDETECTOR_COUNT;i++)
	{
		if (tRTVehDetData.m_chDetStatus[i])
		{
			if (i % C_N_MAX_PHASE_DETECTOR == 0)
			{
				tDetectorStatus[i / C_N_MAXDETINPUT_NUM].CanData.io1 = 1;
			}
			else if (i % C_N_MAX_PHASE_DETECTOR == 1)
			{
				tDetectorStatus[i / C_N_MAXDETINPUT_NUM].CanData.io2 = 1;
			}
			else if (i % C_N_MAX_PHASE_DETECTOR == 2)
			{
				tDetectorStatus[i / C_N_MAXDETINPUT_NUM].CanData.io3 = 1;
			}
			else if (i % C_N_MAX_PHASE_DETECTOR == 3)
			{
				tDetectorStatus[i / C_N_MAXDETINPUT_NUM].CanData.io4 = 1;
			}
			else if (i % C_N_MAX_PHASE_DETECTOR == 4)
			{
				tDetectorStatus[i / C_N_MAXDETINPUT_NUM].CanData.io5 = 1;
			}
			else if (i % C_N_MAX_PHASE_DETECTOR == 5)
			{
				tDetectorStatus[i / C_N_MAXDETINPUT_NUM].CanData.io6 = 1;
			}
			else if (i % C_N_MAX_PHASE_DETECTOR == 6)
			{
				tDetectorStatus[i / C_N_MAXDETINPUT_NUM].CanData.io7 = 1;
			}
			else if (i % C_N_MAX_PHASE_DETECTOR == 7)
			{
				tDetectorStatus[i / C_N_MAXDETINPUT_NUM].CanData.io8 = 1;
			}
		}
	}

	for (i = 0;i < C_N_MAX_PHASE_DETECTOR;i++)
	{
		tDataConfig.m_byDataValue[i] = tDetectorStatus[C_N_MAX_PHASE_DETECTOR - i - 1].chCanData;
	}
	memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
	tCorrectReturnData.m_byReturnCount += 1;
}

void COpenATCCommWithGB20999Thread::QueryAllElementStatisticData(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData)
{
	TStatisticVehDetData statisticVehDetData = COpenATCFlowProcManager::getInstance()->GetCurrentStatisticVehDetData();

	int  i = 0;
	for (i = 0;i < C_N_MAX_DETECTOR_COUNT;i++)
	{
		TDataConfig tDataConfig;
		//tDataConfig.m_byIndex = byIndex + 1;
		tDataConfig.m_byIndex = i + 1;	//HPH 2021.12.07
		tDataConfig.m_byDataClassID = m_chUnPackedBuff[C_N_DATACLASS_ID_POS + byIndex * 6];
		tDataConfig.m_byObjectID = m_chUnPackedBuff[C_N_OBJECT_ID_POS + byIndex * 6];
		tDataConfig.m_byAttributeID = m_chUnPackedBuff[C_N_ATTRIBUTE_ID_POS + byIndex * 6];
		tDataConfig.m_byElementID = i + 1;
		tDataConfig.m_byDataValueLength = 1;
		tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;

		if (tDataConfig.m_byAttributeID == DETECTOR_ID_STATISTIC || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValueLength = 1;
	        tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			tDataConfig.m_byDataValue[0] = tDataConfig.m_byElementID;
			memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
	        tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == DETECTOR_FLOW || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValueLength = 2;
	        tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			short nFlow = 0;
			if (i < C_N_MAXDETBOARD_NUM * C_N_MAXDETINPUT_NUM)
			{
			    nFlow = ntohs(statisticVehDetData.m_atDetFlowInfo[i].m_nSmallVehNum);
			}
			memcpy(tDataConfig.m_byDataValue, &nFlow, 2);
			memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
	        tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == DETECTOR_OCCUPY || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValueLength = 1;
	        tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			if (i < C_N_MAXDETBOARD_NUM * C_N_MAXDETINPUT_NUM)
			{
			    tDataConfig.m_byDataValue[0] = statisticVehDetData.m_atDetFlowInfo[i].m_chOccupyRate;
			}
			else
			{
				tDataConfig.m_byDataValue[0] = 0;
			}
			memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
	        tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == AVERAGE_SPEED || tDataConfig.m_byAttributeID == 0)
		{
			short nSpeed = ntohs(80);
			tDataConfig.m_byDataValueLength = 2;
	        tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			memcpy(tDataConfig.m_byDataValue, &nSpeed, 2);
			memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
	        tCorrectReturnData.m_byReturnCount += 1;
		}
	}
}

void  COpenATCCommWithGB20999Thread::QueryStatisticData(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData)
{
	TDataConfig tDataConfig;
	tDataConfig.m_byIndex = byIndex + 1;
	tDataConfig.m_byDataClassID = m_chUnPackedBuff[C_N_DATACLASS_ID_POS + byIndex * 6];
	tDataConfig.m_byObjectID = m_chUnPackedBuff[C_N_OBJECT_ID_POS + byIndex * 6];
	tDataConfig.m_byAttributeID = m_chUnPackedBuff[C_N_ATTRIBUTE_ID_POS + byIndex * 6];
	tDataConfig.m_byElementID = m_chUnPackedBuff[C_N_ELEMENT_ID_POS + byIndex * 6];
	tDataConfig.m_byDataValueLength = 1;
	tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;

	if (tDataConfig.m_byElementID < 1 || tDataConfig.m_byElementID > 128)
	{
		tDataConfig.m_byDataValue[0] = BAD_VALUE_NULL;
		memcpy(&tWrongReturnData.m_tDataConfig[tWrongReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		tWrongReturnData.m_byReturnCount += 1;
		return;
	}

	TStatisticVehDetData statisticVehDetData = COpenATCFlowProcManager::getInstance()->GetCurrentStatisticVehDetData();

	if (tDataConfig.m_byAttributeID == DETECTOR_ID_STATISTIC || tDataConfig.m_byAttributeID == 0)
	{
		tDataConfig.m_byDataValueLength = 1;
	    tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
		tDataConfig.m_byDataValue[0] = tDataConfig.m_byElementID;
		memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
	    tCorrectReturnData.m_byReturnCount += 1;
	}
	if (tDataConfig.m_byAttributeID == DETECTOR_FLOW || tDataConfig.m_byAttributeID == 0)
	{
		tDataConfig.m_byDataValueLength = 2;
	    tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
		short nFlow = 0;
		if (tDataConfig.m_byElementID < C_N_MAXDETBOARD_NUM * C_N_MAXDETINPUT_NUM)
		{
			nFlow = ntohs(statisticVehDetData.m_atDetFlowInfo[tDataConfig.m_byElementID - 1].m_nSmallVehNum);
		}
		memcpy(tDataConfig.m_byDataValue, &nFlow, 2);
		memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
	    tCorrectReturnData.m_byReturnCount += 1;
	}
	if (tDataConfig.m_byAttributeID == DETECTOR_OCCUPY || tDataConfig.m_byAttributeID == 0)
	{
		tDataConfig.m_byDataValueLength = 1;
	    tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
		if (tDataConfig.m_byElementID < C_N_MAXDETBOARD_NUM * C_N_MAXDETINPUT_NUM)
		{
			tDataConfig.m_byDataValue[0] = statisticVehDetData.m_atDetFlowInfo[tDataConfig.m_byElementID - 1].m_chOccupyRate;
		}
		else
		{
			tDataConfig.m_byDataValue[0] = 0;
		}
		memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
	    tCorrectReturnData.m_byReturnCount += 1;
	}
	if (tDataConfig.m_byAttributeID == AVERAGE_SPEED || tDataConfig.m_byAttributeID == 0)
	{
		tDataConfig.m_byDataValueLength = 1;
	    tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
		short nSpeed = ntohs(80);
		memcpy(tDataConfig.m_byDataValue, &nSpeed, 2);
		memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
	    tCorrectReturnData.m_byReturnCount += 1;
	}
}

void  COpenATCCommWithGB20999Thread::QueryAlarmDataCount(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData)
{
	//4个字节
	TDataConfig tDataConfig;
	tDataConfig.m_byIndex = byIndex + 1;
	tDataConfig.m_byDataClassID = m_chUnPackedBuff[C_N_DATACLASS_ID_POS + byIndex * 6];
	tDataConfig.m_byObjectID = m_chUnPackedBuff[C_N_OBJECT_ID_POS + byIndex * 6];
	tDataConfig.m_byAttributeID = m_chUnPackedBuff[C_N_ATTRIBUTE_ID_POS + byIndex * 6];
	tDataConfig.m_byElementID = m_chUnPackedBuff[C_N_ELEMENT_ID_POS + byIndex * 6];
	tDataConfig.m_byDataValueLength = 4;
	tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
					   
	TLedScreenShowInfo tLedScreenShowInfo;
	memset(&tLedScreenShowInfo, 0, sizeof(tLedScreenShowInfo));
	m_pOpenATCRunStatus->GetLedScreenShowInfo(tLedScreenShowInfo);

	memset(m_tRunAlarmInfo, 0, C_N_MAX_FAULT_COUNT);
	int AlarmDataCount = 0;

	int  i = 0;
	for (i = 0;i < tLedScreenShowInfo.m_nRunFaultCount;i++)
	{
		if (tLedScreenShowInfo.m_tRunFaultInfo[i].m_wFaultType == FaultType_Red_Lamp_Volt_Fault || tLedScreenShowInfo.m_tRunFaultInfo[i].m_wFaultType == FaultType_Red_Lamp_Power_Fault ||
				tLedScreenShowInfo.m_tRunFaultInfo[i].m_wFaultType == FaultType_Yellow_Lamp_Volt_Fault || tLedScreenShowInfo.m_tRunFaultInfo[i].m_wFaultType == FaultType_Yellow_Lamp_Power_Fault ||
				tLedScreenShowInfo.m_tRunFaultInfo[i].m_wFaultType == FaultType_Green_Lamp_Volt_Fault || tLedScreenShowInfo.m_tRunFaultInfo[i].m_wFaultType == FaultType_Green_Lamp_Power_Fault ||
				tLedScreenShowInfo.m_tRunFaultInfo[i].m_wFaultType == FaultType_VetDetector_Short_Circuit || tLedScreenShowInfo.m_tRunFaultInfo[i].m_wFaultType == FaultType_VetDetector_Open_Circuit ||
				tLedScreenShowInfo.m_tRunFaultInfo[i].m_wFaultType == FaultType_VetDetID || tLedScreenShowInfo.m_tRunFaultInfo[i].m_wFaultType == FaultType_VetDetNum || tLedScreenShowInfo.m_tRunFaultInfo[i].m_wFaultType == FaultType_Detector_Fault)
		{
			memcpy(&m_tRunAlarmInfo[AlarmDataCount], &tLedScreenShowInfo.m_tRunFaultInfo[i], sizeof(m_tRunAlarmInfo[AlarmDataCount]));
			AlarmDataCount += 1;
		}
	}

	unsigned long nTemp = ntohl(AlarmDataCount);
	memcpy(tDataConfig.m_byDataValue, &nTemp, 4);
	memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
	tCorrectReturnData.m_byReturnCount += 1;
}

//获取所有报警数据
void  COpenATCCommWithGB20999Thread::QueryAllAlarmData(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData)
{
	TLedScreenShowInfo tLedScreenShowInfo;
	memset(&tLedScreenShowInfo, 0, sizeof(tLedScreenShowInfo));
	m_pOpenATCRunStatus->GetLedScreenShowInfo(tLedScreenShowInfo);

	memset(m_tRunAlarmInfo, 0, C_N_MAX_FAULT_COUNT);
	int AlarmDataCount = 0;

	for (int i = 0;i < tLedScreenShowInfo.m_nRunFaultCount;i++)
	{
		if (tLedScreenShowInfo.m_tRunFaultInfo[i].m_wFaultType == FaultType_Red_Lamp_Volt_Fault || tLedScreenShowInfo.m_tRunFaultInfo[i].m_wFaultType == FaultType_Red_Lamp_Power_Fault ||
				tLedScreenShowInfo.m_tRunFaultInfo[i].m_wFaultType == FaultType_Yellow_Lamp_Volt_Fault || tLedScreenShowInfo.m_tRunFaultInfo[i].m_wFaultType == FaultType_Yellow_Lamp_Power_Fault ||
				tLedScreenShowInfo.m_tRunFaultInfo[i].m_wFaultType == FaultType_Green_Lamp_Volt_Fault || tLedScreenShowInfo.m_tRunFaultInfo[i].m_wFaultType == FaultType_Green_Lamp_Power_Fault ||
				tLedScreenShowInfo.m_tRunFaultInfo[i].m_wFaultType == FaultType_VetDetector_Short_Circuit || tLedScreenShowInfo.m_tRunFaultInfo[i].m_wFaultType == FaultType_VetDetector_Open_Circuit ||
				tLedScreenShowInfo.m_tRunFaultInfo[i].m_wFaultType == FaultType_VetDetID || tLedScreenShowInfo.m_tRunFaultInfo[i].m_wFaultType == FaultType_VetDetNum || tLedScreenShowInfo.m_tRunFaultInfo[i].m_wFaultType == FaultType_Detector_Fault)
		{
			memcpy(&m_tRunAlarmInfo[AlarmDataCount], &tLedScreenShowInfo.m_tRunFaultInfo[i], sizeof(m_tRunAlarmInfo[AlarmDataCount]));
			AlarmDataCount += 1;
		}
	}

	TDataConfig tDataConfig;
	tDataConfig.m_byIndex = byIndex + 1;
	tDataConfig.m_byDataClassID = m_chUnPackedBuff[C_N_DATACLASS_ID_POS + byIndex * 6];
	tDataConfig.m_byObjectID = m_chUnPackedBuff[C_N_OBJECT_ID_POS + byIndex * 6];
	tDataConfig.m_byAttributeID = m_chUnPackedBuff[C_N_ATTRIBUTE_ID_POS + byIndex * 6];
	tDataConfig.m_byElementID = m_chUnPackedBuff[C_N_ELEMENT_ID_POS + byIndex * 6];
	tDataConfig.m_byDataValueLength = 1;
	tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;

	for(int i = 0; i < AlarmDataCount; i++)
	{
		BYTE byAlarmValue = 0;
		BYTE byAlarmType = TransAlarmTypeToHost(m_tRunAlarmInfo[i].m_wFaultType,m_tRunAlarmInfo[i].m_faultInfo[0],m_tRunAlarmInfo[i].m_faultInfo[1], byAlarmValue);

		tDataConfig.m_byDataClassID = m_chUnPackedBuff[C_N_DATACLASS_ID_POS + byIndex * 6];
		tDataConfig.m_byObjectID = m_chUnPackedBuff[C_N_OBJECT_ID_POS + byIndex * 6];
		tDataConfig.m_byElementID = i + 1;

		//报警编号
		tDataConfig.m_byAttributeID = ALARM_ID;
		tDataConfig.m_byDataValueLength = 4;
		tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
		unsigned long nTemp = ntohl(i + 1);
		memcpy(tDataConfig.m_byDataValue, &nTemp, 4);
		//tDataConfig.m_byDataValue[0] = tDataConfig.m_byElementID;
		memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		tCorrectReturnData.m_byReturnCount += 1;

		//报警类型
		tDataConfig.m_byAttributeID = ALARM_TYPE;
		tDataConfig.m_byDataValueLength = 1;
		tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
		tDataConfig.m_byDataValue[0] = byAlarmType;
		memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		tCorrectReturnData.m_byReturnCount += 1;

		//报警值
		tDataConfig.m_byAttributeID = ALARM_VALUE;
		tDataConfig.m_byDataValueLength = 1;
		tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
		tDataConfig.m_byDataValue[0] = byAlarmValue;
		memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		tCorrectReturnData.m_byReturnCount += 1;

		//报警时间
		tDataConfig.m_byAttributeID = ALARM_TIME;
		tDataConfig.m_byDataValueLength = 7;
		tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
#ifndef _WIN32
		short nYear = ntohs(Alarm_tm.tm_yday);
		memcpy(tDataConfig.m_byDataValue, &nYear, 2);              //年
		tDataConfig.m_byDataValue[2] = Alarm_tm.tm_mon;			   //月	
		tDataConfig.m_byDataValue[3] = Alarm_tm.tm_mday;           //日	
		tDataConfig.m_byDataValue[4] = Alarm_tm.tm_hour;           //时
		tDataConfig.m_byDataValue[5] = Alarm_tm.tm_min;			   //分
		tDataConfig.m_byDataValue[6] = Alarm_tm.tm_sec;			   //秒
		memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		tCorrectReturnData.m_byReturnCount += 1;
#else
		short nYear = ntohs(m_stAlarmTime.wYear);
		memcpy(tDataConfig.m_byDataValue, &nYear, 2);              //年
		tDataConfig.m_byDataValue[2] = m_stAlarmTime.wMonth;       //月	
		tDataConfig.m_byDataValue[3] = m_stAlarmTime.wDay;         //日	
		tDataConfig.m_byDataValue[4] = m_stAlarmTime.wHour;        //时
		tDataConfig.m_byDataValue[5] = m_stAlarmTime.wMinute;      //分
		tDataConfig.m_byDataValue[6] = m_stAlarmTime.wSecond;      //秒
		memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		tCorrectReturnData.m_byReturnCount += 1;
#endif
	}
}

void  COpenATCCommWithGB20999Thread::QueryAllElementAlarmData(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData)
{

	TLedScreenShowInfo tLedScreenShowInfo;
	memset(&tLedScreenShowInfo, 0, sizeof(tLedScreenShowInfo));
	m_pOpenATCRunStatus->GetLedScreenShowInfo(tLedScreenShowInfo);

	memset(m_tRunAlarmInfo, 0, C_N_MAX_FAULT_COUNT);
	int AlarmDataCount = 0;

	for (int i = 0;i < tLedScreenShowInfo.m_nRunFaultCount;i++)
	{
		if (tLedScreenShowInfo.m_tRunFaultInfo[i].m_wFaultType == FaultType_Red_Lamp_Volt_Fault || tLedScreenShowInfo.m_tRunFaultInfo[i].m_wFaultType == FaultType_Red_Lamp_Power_Fault ||
				tLedScreenShowInfo.m_tRunFaultInfo[i].m_wFaultType == FaultType_Yellow_Lamp_Volt_Fault || tLedScreenShowInfo.m_tRunFaultInfo[i].m_wFaultType == FaultType_Yellow_Lamp_Power_Fault ||
				tLedScreenShowInfo.m_tRunFaultInfo[i].m_wFaultType == FaultType_Green_Lamp_Volt_Fault || tLedScreenShowInfo.m_tRunFaultInfo[i].m_wFaultType == FaultType_Green_Lamp_Power_Fault ||
				tLedScreenShowInfo.m_tRunFaultInfo[i].m_wFaultType == FaultType_VetDetector_Short_Circuit || tLedScreenShowInfo.m_tRunFaultInfo[i].m_wFaultType == FaultType_VetDetector_Open_Circuit ||
				tLedScreenShowInfo.m_tRunFaultInfo[i].m_wFaultType == FaultType_VetDetID || tLedScreenShowInfo.m_tRunFaultInfo[i].m_wFaultType == FaultType_VetDetNum || tLedScreenShowInfo.m_tRunFaultInfo[i].m_wFaultType == FaultType_Detector_Fault)
		{
			memcpy(&m_tRunAlarmInfo[AlarmDataCount], &tLedScreenShowInfo.m_tRunFaultInfo[i], sizeof(m_tRunAlarmInfo[AlarmDataCount]));
			AlarmDataCount += 1;
		}
	}


	TDataConfig tDataConfig;
	tDataConfig.m_byIndex = byIndex + 1;
	tDataConfig.m_byDataClassID = m_chUnPackedBuff[C_N_DATACLASS_ID_POS + byIndex * 6];
	tDataConfig.m_byObjectID = m_chUnPackedBuff[C_N_OBJECT_ID_POS + byIndex * 6];
	tDataConfig.m_byAttributeID = m_chUnPackedBuff[C_N_ATTRIBUTE_ID_POS + byIndex * 6];
	tDataConfig.m_byElementID = m_chUnPackedBuff[C_N_ELEMENT_ID_POS + byIndex * 6];
	tDataConfig.m_byDataValueLength = 1;
	tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;

	/*tDataConfig.m_byDataValue[0] = BAD_VALUE_NULL;
	memcpy(&tWrongReturnData.m_tDataConfig[tWrongReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
	tWrongReturnData.m_byReturnCount += 1;*/

	for (int i = 0;i < AlarmDataCount;i++)
	{
		BYTE byAlarmValue = 0;
		BYTE byAlarmType = TransAlarmTypeToHost(m_tRunAlarmInfo[i].m_wFaultType,m_tRunAlarmInfo[i].m_faultInfo[0],m_tRunAlarmInfo[i].m_faultInfo[1], byAlarmValue);

		tDataConfig.m_byElementID = i + 1;

		if (tDataConfig.m_byAttributeID == 1)//报警编号	[4个字节]
		{
			tDataConfig.m_byDataValueLength = 4;
			tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			unsigned long nTemp = ntohl(i + 1);
			memcpy(tDataConfig.m_byDataValue, &nTemp, 4);
			//tDataConfig.m_byDataValue[0] = tDataConfig.m_byElementID;
			memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
		}
		else if (tDataConfig.m_byAttributeID == 2)//报警类型 2.11
		{
			tDataConfig.m_byDataValueLength = 1;
			tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			tDataConfig.m_byDataValue[0] = byAlarmType;
			memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
		}
		else if (tDataConfig.m_byAttributeID == 3)//报警值 2.12
		{
			tDataConfig.m_byDataValueLength = 1;
			tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			tDataConfig.m_byDataValue[0] = byAlarmValue;
			memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
		}
		else if (tDataConfig.m_byAttributeID == 4)//报警时间
		{
#ifndef _WIN32
			tDataConfig.m_byDataValueLength = 7;
			tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			short nYear = ntohs(Alarm_tm.tm_yday);
			memcpy(tDataConfig.m_byDataValue, &nYear, 2);              //年
			tDataConfig.m_byDataValue[2] = Alarm_tm.tm_mon;			   //月	
			tDataConfig.m_byDataValue[3] = Alarm_tm.tm_mday;           //日	
			tDataConfig.m_byDataValue[4] = Alarm_tm.tm_hour;           //时
			tDataConfig.m_byDataValue[5] = Alarm_tm.tm_min;			   //分
			tDataConfig.m_byDataValue[6] = Alarm_tm.tm_sec;			   //秒
			memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
#else
			tDataConfig.m_byDataValueLength = 7;
			tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			short nYear = ntohs(m_stAlarmTime.wYear);
			memcpy(tDataConfig.m_byDataValue, &nYear, 2);              //年
			tDataConfig.m_byDataValue[2] = m_stAlarmTime.wMonth;       //月	
			tDataConfig.m_byDataValue[3] = m_stAlarmTime.wDay;         //日	
			tDataConfig.m_byDataValue[4] = m_stAlarmTime.wHour;        //时
			tDataConfig.m_byDataValue[5] = m_stAlarmTime.wMinute;      //分
			tDataConfig.m_byDataValue[6] = m_stAlarmTime.wSecond;      //秒
			memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
#endif
		}
	}
}

void  COpenATCCommWithGB20999Thread::QueryAlarmData(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData)
{
	TLedScreenShowInfo tLedScreenShowInfo;
	memset(&tLedScreenShowInfo, 0, sizeof(tLedScreenShowInfo));
	m_pOpenATCRunStatus->GetLedScreenShowInfo(tLedScreenShowInfo);

	memset(m_tRunAlarmInfo, 0, C_N_MAX_FAULT_COUNT);
	int AlarmDataCount = 0;

	int  i = 0;
	for (i = 0;i < tLedScreenShowInfo.m_nRunFaultCount;i++)
	{
		if (tLedScreenShowInfo.m_tRunFaultInfo[i].m_wFaultType == FaultType_Red_Lamp_Volt_Fault || tLedScreenShowInfo.m_tRunFaultInfo[i].m_wFaultType == FaultType_Red_Lamp_Power_Fault ||
				tLedScreenShowInfo.m_tRunFaultInfo[i].m_wFaultType == FaultType_Yellow_Lamp_Volt_Fault || tLedScreenShowInfo.m_tRunFaultInfo[i].m_wFaultType == FaultType_Yellow_Lamp_Power_Fault ||
				tLedScreenShowInfo.m_tRunFaultInfo[i].m_wFaultType == FaultType_Green_Lamp_Volt_Fault || tLedScreenShowInfo.m_tRunFaultInfo[i].m_wFaultType == FaultType_Green_Lamp_Power_Fault ||
				tLedScreenShowInfo.m_tRunFaultInfo[i].m_wFaultType == FaultType_VetDetector_Short_Circuit || tLedScreenShowInfo.m_tRunFaultInfo[i].m_wFaultType == FaultType_VetDetector_Open_Circuit ||
				tLedScreenShowInfo.m_tRunFaultInfo[i].m_wFaultType == FaultType_VetDetID || tLedScreenShowInfo.m_tRunFaultInfo[i].m_wFaultType == FaultType_VetDetNum || tLedScreenShowInfo.m_tRunFaultInfo[i].m_wFaultType == FaultType_Detector_Fault)
		{
			memcpy(&m_tRunAlarmInfo[AlarmDataCount], &tLedScreenShowInfo.m_tRunFaultInfo[i], sizeof(m_tRunAlarmInfo[AlarmDataCount]));
			AlarmDataCount += 1;
		}
	}

	TDataConfig tDataConfig;
	tDataConfig.m_byIndex = byIndex + 1;
	tDataConfig.m_byDataClassID = m_chUnPackedBuff[C_N_DATACLASS_ID_POS + byIndex * 6];
	tDataConfig.m_byObjectID = m_chUnPackedBuff[C_N_OBJECT_ID_POS + byIndex * 6];
	tDataConfig.m_byAttributeID = m_chUnPackedBuff[C_N_ATTRIBUTE_ID_POS + byIndex * 6];
	tDataConfig.m_byElementID = m_chUnPackedBuff[C_N_ELEMENT_ID_POS + byIndex * 6];
	tDataConfig.m_byDataValueLength = 1;
	tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;

	if (tDataConfig.m_byElementID > AlarmDataCount)//默认有一条报警数据[查询错误]
	{
	    tDataConfig.m_byDataValue[0] = BAD_VALUE_NULL;
	    memcpy(&tWrongReturnData.m_tDataConfig[tWrongReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
	    tWrongReturnData.m_byReturnCount += 1;
		return;
	}

	BYTE byAlarmValue = 0;
	BYTE byAlarmType = TransAlarmTypeToHost(m_tRunAlarmInfo[tDataConfig.m_byElementID].m_wFaultType,m_tRunAlarmInfo[tDataConfig.m_byElementID].m_faultInfo[0],m_tRunAlarmInfo[tDataConfig.m_byElementID].m_faultInfo[1], byAlarmValue);

	if (tDataConfig.m_byAttributeID == 1)//报警编号
	{
		tDataConfig.m_byDataValueLength = 4;
	    tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
		unsigned long nTemp = ntohl(tDataConfig.m_byElementID);
		memcpy(tDataConfig.m_byDataValue, &nTemp, 4);
		/*tDataConfig.m_byDataValue[0] = tDataConfig.m_byElementID;*/
		memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
	    tCorrectReturnData.m_byReturnCount += 1;
	}
	else if (tDataConfig.m_byAttributeID == 2)//报警类型 2.11
	{
		tDataConfig.m_byDataValueLength = 1;
	    tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
		tDataConfig.m_byDataValue[0] = byAlarmType;
		memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
	    tCorrectReturnData.m_byReturnCount += 1;
	}
	else if (tDataConfig.m_byAttributeID == 3)//报警值 2.12
	{
		tDataConfig.m_byDataValueLength = 1;
	    tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
		tDataConfig.m_byDataValue[0] = byAlarmValue;
		memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
	    tCorrectReturnData.m_byReturnCount += 1;
	}
	else if (tDataConfig.m_byAttributeID == 4)//报警时间
	{
#ifndef _WIN32
		tDataConfig.m_byDataValueLength = 7;
		tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
		short nYear = ntohs(Alarm_tm.tm_yday);
		memcpy(tDataConfig.m_byDataValue, &nYear, 2);              //年
		tDataConfig.m_byDataValue[2] = Alarm_tm.tm_mon;			   //月	
		tDataConfig.m_byDataValue[3] = Alarm_tm.tm_mday;           //日	
		tDataConfig.m_byDataValue[4] = Alarm_tm.tm_hour;           //时
		tDataConfig.m_byDataValue[5] = Alarm_tm.tm_min;			   //分
		tDataConfig.m_byDataValue[6] = Alarm_tm.tm_sec;			   //秒
		memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		tCorrectReturnData.m_byReturnCount += 1;
#else
		tDataConfig.m_byDataValueLength = 7;
		tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
		short nYear = ntohs(m_stAlarmTime.wYear);
		memcpy(tDataConfig.m_byDataValue, &nYear, 2);              //年
		tDataConfig.m_byDataValue[2] = m_stAlarmTime.wMonth;       //月	
		tDataConfig.m_byDataValue[3] = m_stAlarmTime.wDay;         //日	
		tDataConfig.m_byDataValue[4] = m_stAlarmTime.wHour;        //时
		tDataConfig.m_byDataValue[5] = m_stAlarmTime.wMinute;      //分
		tDataConfig.m_byDataValue[6] = m_stAlarmTime.wSecond;      //秒
		memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		tCorrectReturnData.m_byReturnCount += 1;
#endif
	}
}

void COpenATCCommWithGB20999Thread::QueryFaultDataCount(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData)
{
	/*
	TTestFaultStatus tTestFaultStatus;
	m_pOpenATCRunStatus->GetTestFaultInfo(tTestFaultStatus);	//获取故障信息

	TDataConfig tDataConfig;
	tDataConfig.m_byIndex = byIndex + 1;
	tDataConfig.m_byDataClassID = m_chUnPackedBuff[C_N_DATACLASS_ID_POS + byIndex * 6];
	tDataConfig.m_byObjectID = m_chUnPackedBuff[C_N_OBJECT_ID_POS + byIndex * 6];
	tDataConfig.m_byAttributeID = m_chUnPackedBuff[C_N_ATTRIBUTE_ID_POS + byIndex * 6];
	tDataConfig.m_byElementID = m_chUnPackedBuff[C_N_ELEMENT_ID_POS + byIndex * 6];
	tDataConfig.m_byDataValueLength = 4;
	tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;

	unsigned long nTemp = ntohl(tTestFaultStatus.m_nFaultNum);
	memcpy(tDataConfig.m_byDataValue, &nTemp, 4);
	memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
	tCorrectReturnData.m_byReturnCount += 1;
	*/
					   
	//TSelfDetectInfo selfDetectInfo;
	//m_pOpenATCRunStatus->GetSelfDetectInfo(selfDetectInfo);

	//if (selfDetectInfo.m_cSelfDetectStatus == SELF_DETECT_FAILED)
	//{
	//	tDataConfig.m_byDataValue[0] = 1;
	//	memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
	//    tCorrectReturnData.m_byReturnCount += 1;
	//}
	//else
	//{
	//	TLedScreenShowInfo tLedScreenShowInfo;
	//	m_pOpenATCRunStatus->GetLedScreenShowInfo(tLedScreenShowInfo);

	//	memset(m_tRunFaultInfo, 0, C_N_MAX_FAULT_COUNT);
	//	m_nFaultDataCount = 0;

	//	int  i = 0;
	//	for (i = 0;i < tLedScreenShowInfo.m_nRunFaultCount;i++)
	//	{
	//		BYTE byFaultType = 0;
	//		if (tLedScreenShowInfo.m_tRunFaultInfo[i].m_wFaultType == FaultType_GreenConflict || tLedScreenShowInfo.m_tRunFaultInfo[i].m_wFaultType == FaultType_GreenAndRedOn ||
	//			tLedScreenShowInfo.m_tRunFaultInfo[i].m_wFaultType == FaultType_Red_Lamp_Volt_Fault || tLedScreenShowInfo.m_tRunFaultInfo[i].m_wFaultType == FaultType_Red_Lamp_Power_Fault ||
	//			tLedScreenShowInfo.m_tRunFaultInfo[i].m_wFaultType == FaultType_Yellow_Lamp_Volt_Fault || tLedScreenShowInfo.m_tRunFaultInfo[i].m_wFaultType == FaultType_Yellow_Lamp_Power_Fault ||
	//			tLedScreenShowInfo.m_tRunFaultInfo[i].m_wFaultType == FaultType_Green_Lamp_Volt_Fault || tLedScreenShowInfo.m_tRunFaultInfo[i].m_wFaultType == FaultType_Green_Lamp_Power_Fault ||
	//			tLedScreenShowInfo.m_tRunFaultInfo[i].m_wFaultType == FaultType_MainBoard || tLedScreenShowInfo.m_tRunFaultInfo[i].m_wFaultType == FaultType_Relay_Not_Work ||
	//			tLedScreenShowInfo.m_tRunFaultInfo[i].m_wFaultType == FaultType_VetDetector_Short_Circuit || tLedScreenShowInfo.m_tRunFaultInfo[i].m_wFaultType == FaultType_VetDetector_Open_Circuit || tLedScreenShowInfo.m_tRunFaultInfo[i].m_wFaultType == FaultType_Detector_Fault ||
	//			tLedScreenShowInfo.m_tRunFaultInfo[i].m_wFaultType == FaultType_Wong_Slot || tLedScreenShowInfo.m_tRunFaultInfo[i].m_wFaultType == FaultType_Wong_Plug || 
	//			tLedScreenShowInfo.m_tRunFaultInfo[i].m_wFaultType == FaultType_Config_Master_Count || tLedScreenShowInfo.m_tRunFaultInfo[i].m_wFaultType == FaultType_LampBoardNum || tLedScreenShowInfo.m_tRunFaultInfo[i].m_wFaultType == FaultType_Lamp_Fault ||
	//			tLedScreenShowInfo.m_tRunFaultInfo[i].m_wFaultType == FaultType_VetDetID || tLedScreenShowInfo.m_tRunFaultInfo[i].m_wFaultType == FaultType_VetDetNum ||
	//			tLedScreenShowInfo.m_tRunFaultInfo[i].m_wFaultType == FaultType_TZParam)
	//		{
	//			memcpy(&m_tRunFaultInfo[m_nFaultDataCount], &tLedScreenShowInfo.m_tRunFaultInfo[i], sizeof(m_tRunFaultInfo[m_nFaultDataCount]));
	//			m_nFaultDataCount += 1;
	//		}
	//	}

	//	//默认有一条故障数据
	//	tDataConfig.m_byDataValue[0] = m_nFaultDataCount;//tLedScreenShowInfo.m_nRunFaultCount;
	//	memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
	//    tCorrectReturnData.m_byReturnCount += 1;
	//}
}

void  COpenATCCommWithGB20999Thread::QueryAllFaultData(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData)
{
	/*
	TTestFaultStatus tTestFaultStatus;
	m_pOpenATCRunStatus->GetTestFaultInfo(tTestFaultStatus);	//获取故障信息

	TDataConfig tDataConfig;
	tDataConfig.m_byIndex = byIndex + 1;
	tDataConfig.m_byDataClassID = m_chUnPackedBuff[C_N_DATACLASS_ID_POS + byIndex * 6];
	tDataConfig.m_byObjectID = m_chUnPackedBuff[C_N_OBJECT_ID_POS + byIndex * 6];
	tDataConfig.m_byAttributeID = m_chUnPackedBuff[C_N_ATTRIBUTE_ID_POS + byIndex * 6];
	tDataConfig.m_byElementID = m_chUnPackedBuff[C_N_ELEMENT_ID_POS + byIndex * 6];
	tDataConfig.m_byDataValueLength = 1;
	tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;

	if (tTestFaultStatus.m_nFaultNum == 0)
	{
		tDataConfig.m_byDataValue[0] = BAD_VALUE_NULL;
		memcpy(&tWrongReturnData.m_tDataConfig[tWrongReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		tWrongReturnData.m_byReturnCount += 1;
		return;
	}
	*/

	//if(tTestFaultStatus.m_nFaultNum == 0)
	//{
	//	tDataConfig.m_byDataClassID = m_chUnPackedBuff[C_N_DATACLASS_ID_POS + byIndex * 6];
	//	tDataConfig.m_byObjectID = m_chUnPackedBuff[C_N_OBJECT_ID_POS + byIndex * 6];
	//	tDataConfig.m_byElementID = 1;

	//	//故障编号
	//	tDataConfig.m_byAttributeID = FAULT_ID;
	//	tDataConfig.m_byDataValueLength = 4;
	//	tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
	//	memset(tDataConfig.m_byDataValue, 0, 4);
	//	//tDataConfig.m_byDataValue[0] = tDataConfig.m_byElementID;
	//	memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
	//	tCorrectReturnData.m_byReturnCount += 1;

	//	//故障类型
	//	tDataConfig.m_byAttributeID = FAULT_TYPE;
	//	tDataConfig.m_byDataValueLength = 1;
	//	tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
	//	tDataConfig.m_byDataValue[0] = 0;
	//	memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
	//	tCorrectReturnData.m_byReturnCount += 1;

	//	//故障时间
	//	tDataConfig.m_byAttributeID = FAULT_TIME;
	//	tDataConfig.m_byDataValueLength = 7;
	//	tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
	//	memset(tDataConfig.m_byDataValue, 0, 7);
	//	memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
	//	tCorrectReturnData.m_byReturnCount += 1;

	//	//故障动作
	//	tDataConfig.m_byAttributeID = FAULT_PARAM;
	//	tDataConfig.m_byDataValueLength = 1;
	//	tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
	//	tDataConfig.m_byDataValue[0] = 0;
	//	memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
	//	tCorrectReturnData.m_byReturnCount += 1;
	//}
	//else
	//{

	/*
	for(int i = 0; i < tTestFaultStatus.m_nFaultNum; i++)
	{
		tDataConfig.m_byDataClassID = m_chUnPackedBuff[C_N_DATACLASS_ID_POS + byIndex * 6];
		tDataConfig.m_byObjectID = m_chUnPackedBuff[C_N_OBJECT_ID_POS + byIndex * 6];
		tDataConfig.m_byElementID = i + 1;

		//故障编号
		tDataConfig.m_byAttributeID = FAULT_ID;
		tDataConfig.m_byDataValueLength = 4;
		tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
		unsigned long nTemp = ntohl(i + 1);
		memcpy(tDataConfig.m_byDataValue, &nTemp, 4);
		memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		tCorrectReturnData.m_byReturnCount += 1;

		//故障类型
		tDataConfig.m_byAttributeID = FAULT_TYPE;
		tDataConfig.m_byDataValueLength = 1;
		tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
		tDataConfig.m_byDataValue[0] = tTestFaultStatus.m_tFaultInfo[i].m_wFaultType;
		memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		tCorrectReturnData.m_byReturnCount += 1;

		//故障时间
		tDataConfig.m_byAttributeID = FAULT_TIME;
		tDataConfig.m_byDataValueLength = 7;
		tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
		memcpy(tDataConfig.m_byDataValue, &tTestFaultStatus.m_tFaultInfo[i].m_wFaultTime, 7);
		memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		tCorrectReturnData.m_byReturnCount += 1;

		//故障动作
		tDataConfig.m_byAttributeID = FAULT_PARAM;
		tDataConfig.m_byDataValueLength = 1;
		tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
		tDataConfig.m_byDataValue[0] = tTestFaultStatus.m_tFaultInfo[i].m_unFaultAction;
		memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		tCorrectReturnData.m_byReturnCount += 1;
		//}
	}
	*/
}

void COpenATCCommWithGB20999Thread::QueryAllElementFaultData(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData)
{
	/*
	TTestFaultStatus tTestFaultStatus;
	m_pOpenATCRunStatus->GetTestFaultInfo(tTestFaultStatus);	//获取故障信息

	TDataConfig tDataConfig;
	tDataConfig.m_byIndex = byIndex + 1;
	tDataConfig.m_byDataClassID = m_chUnPackedBuff[C_N_DATACLASS_ID_POS + byIndex * 6];
	tDataConfig.m_byObjectID = m_chUnPackedBuff[C_N_OBJECT_ID_POS + byIndex * 6];
	tDataConfig.m_byAttributeID = m_chUnPackedBuff[C_N_ATTRIBUTE_ID_POS + byIndex * 6];
	tDataConfig.m_byElementID = m_chUnPackedBuff[C_N_ELEMENT_ID_POS + byIndex * 6];
	tDataConfig.m_byDataValueLength = 1;
	tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;

	if (tTestFaultStatus.m_nFaultNum == 0)
	{
		tDataConfig.m_byDataValue[0] = BAD_VALUE_NULL;
		memcpy(&tWrongReturnData.m_tDataConfig[tWrongReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		tWrongReturnData.m_byReturnCount += 1;
		return;
	}

	int  i = 0;
	for (i = 0;i < tTestFaultStatus.m_nFaultNum;i++)
	{	
		if (tDataConfig.m_byAttributeID == FAULT_ID || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValueLength = 4;
			tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			unsigned long nTemp = ntohl(i + 1);
			memcpy(tDataConfig.m_byDataValue, &nTemp, 4);
			memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == FAULT_TYPE || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValueLength = 1;
			tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			tDataConfig.m_byDataValue[0] = tTestFaultStatus.m_tFaultInfo[i].m_wFaultType;
			memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == FAULT_TIME || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValueLength = 7;
			tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			memcpy(tDataConfig.m_byDataValue, &tTestFaultStatus.m_tFaultInfo[i].m_wFaultTime, 7);
			memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == FAULT_PARAM || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValueLength = 1;
			tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			tDataConfig.m_byDataValue[0] = tTestFaultStatus.m_tFaultInfo[i].m_unFaultAction;
			memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
		}
	}
	*/
}

void  COpenATCCommWithGB20999Thread::QueryFaultData(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData)
{
	/*
	TTestFaultStatus tTestFaultStatus;
	m_pOpenATCRunStatus->GetTestFaultInfo(tTestFaultStatus);	//获取故障信息

	TDataConfig tDataConfig;
	tDataConfig.m_byIndex = byIndex + 1;
	tDataConfig.m_byDataClassID = m_chUnPackedBuff[C_N_DATACLASS_ID_POS + byIndex * 6];
	tDataConfig.m_byObjectID = m_chUnPackedBuff[C_N_OBJECT_ID_POS + byIndex * 6];
	tDataConfig.m_byAttributeID = m_chUnPackedBuff[C_N_ATTRIBUTE_ID_POS + byIndex * 6];
	tDataConfig.m_byElementID = m_chUnPackedBuff[C_N_ELEMENT_ID_POS + byIndex * 6];
	tDataConfig.m_byDataValueLength = 1;
	tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
	*/

	//BYTE byFaultType = 0;
	//BYTE byOperation = SWITCH_NULL;

	//TLedScreenShowInfo tLedScreenShowInfo;
	//memset(&tLedScreenShowInfo, 0, sizeof(tLedScreenShowInfo));
	//m_pOpenATCRunStatus->GetLedScreenShowInfo(tLedScreenShowInfo);

	//默认有一条故障数据
	/*
	if (tDataConfig.m_byElementID > tTestFaultStatus.m_nFaultNum)
	{
		tDataConfig.m_byDataValue[0] = BAD_VALUE_NULL;
		memcpy(&tWrongReturnData.m_tDataConfig[tWrongReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		tWrongReturnData.m_byReturnCount += 1;
		return;
	}

	if (tDataConfig.m_byAttributeID == FAULT_ID || tDataConfig.m_byAttributeID == 0)
	{
		tDataConfig.m_byDataValueLength = 4;
		tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
		unsigned long nTemp = ntohl(tDataConfig.m_byElementID);
		memcpy(tDataConfig.m_byDataValue, &nTemp, 4);
		memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		tCorrectReturnData.m_byReturnCount += 1;
	}
	if (tDataConfig.m_byAttributeID == FAULT_TYPE || tDataConfig.m_byAttributeID == 0)
	{
		tDataConfig.m_byDataValueLength = 1;
		tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
		tDataConfig.m_byDataValue[0] = tTestFaultStatus.m_tFaultInfo[tDataConfig.m_byElementID - 1].m_wFaultType;
		memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		tCorrectReturnData.m_byReturnCount += 1;
	}
	if (tDataConfig.m_byAttributeID == FAULT_TIME || tDataConfig.m_byAttributeID == 0)
	{
		tDataConfig.m_byDataValueLength = 7;
		tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
		memcpy(tDataConfig.m_byDataValue, &tTestFaultStatus.m_tFaultInfo[tDataConfig.m_byElementID - 1].m_wFaultTime, 7);
		memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		tCorrectReturnData.m_byReturnCount += 1;
	}
	if (tDataConfig.m_byAttributeID == FAULT_PARAM || tDataConfig.m_byAttributeID == 0)
	{
		tDataConfig.m_byDataValueLength = 1;
		tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
		tDataConfig.m_byDataValue[0] = tTestFaultStatus.m_tFaultInfo[tDataConfig.m_byElementID - 1].m_unFaultAction;//byOperation;
		memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		tCorrectReturnData.m_byReturnCount += 1;
	}
	*/
}

void  COpenATCCommWithGB20999Thread::QueryCentreControlTable(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData)
{
	TDataConfig tDataConfig;
	tDataConfig.m_byIndex = byIndex + 1;
	tDataConfig.m_byDataClassID = m_chUnPackedBuff[C_N_DATACLASS_ID_POS + byIndex * 6];
	tDataConfig.m_byObjectID = m_chUnPackedBuff[C_N_OBJECT_ID_POS + byIndex * 6];
	tDataConfig.m_byAttributeID = m_chUnPackedBuff[C_N_ATTRIBUTE_ID_POS + byIndex * 6];
	tDataConfig.m_byElementID = m_chUnPackedBuff[C_N_ELEMENT_ID_POS + byIndex * 6];
	tDataConfig.m_byDataValueLength = 1;
	tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;

	TPhaseRunStatus tPhaseRunStatus;
	memset(&tPhaseRunStatus, 0x00, sizeof(tPhaseRunStatus));
	m_pOpenATCRunStatus->GetPhaseRunStatus(tPhaseRunStatus);

    m_tAscParamInfo.m_stCenterCtrlInfo.m_byIntersectionID = 1;//默认路口ID为1

	if (m_tAscParamInfo.m_stCenterCtrlInfo.m_byIntersectionID == tDataConfig.m_byElementID)
	{
		if (tDataConfig.m_byAttributeID == CENTERCONTROL_ROADID || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValue[0] = m_tAscParamInfo.m_stCenterCtrlInfo.m_byIntersectionID;
			memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == CENTERCONTROL_PHASESTAGE || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValue[0] = m_tAscParamInfo.m_stCenterCtrlInfo.m_byPhaseStage;
			 memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == CENTERCONTROL_PATTERN || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValue[0] = m_tAscParamInfo.m_stCenterCtrlInfo.m_byPattern;
			memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
		}
		if (tDataConfig.m_byAttributeID == CENTERCONTROL_RUNMODE || tDataConfig.m_byAttributeID == 0)
		{
			tDataConfig.m_byDataValue[0] = m_tAscParamInfo.m_stCenterCtrlInfo.m_byRunMode;
			memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
		}
	}
	else
	{
		tDataConfig.m_byDataValue[0] = BAD_VALUE_NULL;
		memcpy(&tWrongReturnData.m_tDataConfig[tWrongReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		tWrongReturnData.m_byReturnCount += 1;
	}
}

void  COpenATCCommWithGB20999Thread::QueryOrderPipeTable(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData)
{
	TDataConfig tDataConfig;
	tDataConfig.m_byIndex = byIndex + 1;
	tDataConfig.m_byDataClassID = m_chUnPackedBuff[C_N_DATACLASS_ID_POS + byIndex * 6];
	tDataConfig.m_byObjectID = m_chUnPackedBuff[C_N_OBJECT_ID_POS + byIndex * 6];
	tDataConfig.m_byAttributeID = m_chUnPackedBuff[C_N_ATTRIBUTE_ID_POS + byIndex * 6];
	tDataConfig.m_byElementID = m_chUnPackedBuff[C_N_ELEMENT_ID_POS + byIndex * 6];
	tDataConfig.m_byDataValueLength = 16;
	tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;

	memcpy(tDataConfig.m_byDataValue, m_nPipeInfo, 16);
	memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
	tCorrectReturnData.m_byReturnCount += 1;
}

//私有类：跟随相位 --HPH 2021.12.07
//void  COpenATCCommWithGB20999Thread::QueryOverlapCount(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData)
//{
//	TDataConfig tDataConfig;
//	tDataConfig.m_byIndex = byIndex + 1;
//	tDataConfig.m_byDataClassID = m_chUnPackedBuff[C_N_DATACLASS_ID_POS + byIndex * 6];
//	tDataConfig.m_byObjectID = m_chUnPackedBuff[C_N_OBJECT_ID_POS + byIndex * 6];
//	tDataConfig.m_byAttributeID = m_chUnPackedBuff[C_N_ATTRIBUTE_ID_POS + byIndex * 6];
//	tDataConfig.m_byElementID = m_chUnPackedBuff[C_N_ELEMENT_ID_POS + byIndex * 6];
//	tDataConfig.m_byDataValueLength = 1;
//	tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
//
//	tDataConfig.m_byDataValue[0] = m_tAscParamInfo.m_stOverlapTableValidSize;
//	memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
//	tCorrectReturnData.m_byReturnCount += 1;
//}
//
//void  COpenATCCommWithGB20999Thread::QueryAllElementOverlapConfig(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData)
//{
//	for (int i = 0;i < m_tAscParamInfo.m_stOverlapTableValidSize;i++)
//	{
//		TDataConfig tDataConfig;
//		tDataConfig.m_byIndex = byIndex + 1;
//		tDataConfig.m_byDataClassID = m_chUnPackedBuff[C_N_DATACLASS_ID_POS + byIndex * 6];
//		tDataConfig.m_byObjectID = m_chUnPackedBuff[C_N_OBJECT_ID_POS + byIndex * 6];
//		tDataConfig.m_byAttributeID = m_chUnPackedBuff[C_N_ATTRIBUTE_ID_POS + byIndex * 6];
//		tDataConfig.m_byElementID = i + 1;
//		tDataConfig.m_byDataValueLength = 1;
//		tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
//
//		if (tDataConfig.m_byAttributeID == OVERLAP_ID || tDataConfig.m_byAttributeID == 0)
//		{
//			tDataConfig.m_byDataValueLength = 1;
//			tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
//			tDataConfig.m_byDataValue[0] = tDataConfig.m_byElementID;
//			memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
//			tCorrectReturnData.m_byReturnCount += 1;
//		}
//		if (tDataConfig.m_byAttributeID == OVERLAP_PULSETYPE || tDataConfig.m_byAttributeID == 0)
//		{
//			tDataConfig.m_byDataValueLength = 1;
//			tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
//			tDataConfig.m_byDataValue[0] = m_tAscParamInfo.m_stAscOverlapTable[i].m_byPulseType;
//			memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
//			tCorrectReturnData.m_byReturnCount += 1;
//		}
//		if (tDataConfig.m_byAttributeID == OVERLAP_INCLUDEDPHASE || tDataConfig.m_byAttributeID == 0)
//		{
//			tDataConfig.m_byDataValueLength = MAX_PHASE_COUNT_IN_OVERLAP;
//			tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
//			memcpy(tDataConfig.m_byDataValue, m_tAscParamInfo.m_stAscOverlapTable[i].m_byArrOverlapIncludedPhases, MAX_PHASE_COUNT_IN_OVERLAP);
//			memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
//			tCorrectReturnData.m_byReturnCount += 1;
//		}
//	}
//}
//
//void  COpenATCCommWithGB20999Thread::QueryOverlapConfig(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData)
//{
//	TDataConfig tDataConfig;
//	tDataConfig.m_byIndex = byIndex + 1;
//	tDataConfig.m_byDataClassID = m_chUnPackedBuff[C_N_DATACLASS_ID_POS + byIndex * 6];
//	tDataConfig.m_byObjectID = m_chUnPackedBuff[C_N_OBJECT_ID_POS + byIndex * 6];
//	tDataConfig.m_byAttributeID = m_chUnPackedBuff[C_N_ATTRIBUTE_ID_POS + byIndex * 6];
//	tDataConfig.m_byElementID = m_chUnPackedBuff[C_N_ELEMENT_ID_POS + byIndex * 6];
//	tDataConfig.m_byDataValueLength = 1;
//	tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
//
//	if (tDataConfig.m_byElementID < 1 || tDataConfig.m_byElementID > MAX_OVERLAP_COUNT)
//	{
//		tDataConfig.m_byDataValue[0] = BAD_VALUE_NULL;
//		memcpy(&tWrongReturnData.m_tDataConfig[tWrongReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
//		tWrongReturnData.m_byReturnCount += 1;
//		return;
//	}
//
//	bool bFlag = false;
//	BYTE byOverlapIndex = 0;
//	for (int i = 0;i < MAX_OVERLAP_COUNT;i++)
//	{
//		if (m_tAscParamInfo.m_stAscOverlapTable[i].m_byOverlapNumber = tDataConfig.m_byElementID)
//		{
//			byOverlapIndex = i;
//			bFlag = true;
//			break;
//		}
//	}
//
//	if (bFlag)
//	{
//		if (tDataConfig.m_byAttributeID == OVERLAP_ID || tDataConfig.m_byAttributeID == 0)
//		{
//			tDataConfig.m_byDataValueLength = 1;
//			tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
//			tDataConfig.m_byDataValue[0] = tDataConfig.m_byElementID;
//			memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
//			tCorrectReturnData.m_byReturnCount += 1;
//		}
//		if (tDataConfig.m_byAttributeID == OVERLAP_PULSETYPE || tDataConfig.m_byAttributeID == 0)
//		{
//			tDataConfig.m_byDataValueLength = 1;
//			tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
//			tDataConfig.m_byDataValue[0] = m_tAscParamInfo.m_stAscOverlapTable[byOverlapIndex].m_byPulseType;
//			memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
//			tCorrectReturnData.m_byReturnCount += 1;
//		}
//		
//		if (tDataConfig.m_byAttributeID == OVERLAP_INCLUDEDPHASE || tDataConfig.m_byAttributeID == 0)
//		{
//			tDataConfig.m_byDataValueLength = MAX_PHASE_COUNT_IN_OVERLAP;
//			tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
//			memcpy(tDataConfig.m_byDataValue, m_tAscParamInfo.m_stAscOverlapTable[byOverlapIndex].m_byArrOverlapIncludedPhases, MAX_PHASE_COUNT_IN_OVERLAP);
//			memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
//			tCorrectReturnData.m_byReturnCount += 1;
//		}
//		
//	}
//	else
//	{
//		tDataConfig.m_byDataValue[0] = BAD_VALUE_NULL;
//		memcpy(&tWrongReturnData.m_tDataConfig[tWrongReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
//		tWrongReturnData.m_byReturnCount += 1;
//	}
//}	//私有类：跟随相位 --HPH 2021.12.07
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void  COpenATCCommWithGB20999Thread::CreateSetReturnData(BYTE byRet, BYTE byIndex, BYTE byDataClassID, BYTE byObjectID, BYTE byAttributeID, BYTE byElementID, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData)
{
	if (byRet == 0)
	{
		tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount].m_byIndex = byIndex + 1;
		tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount].m_byDataLength = 4 + 1;
		tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount].m_byDataClassID = byDataClassID;
		tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount].m_byObjectID = byObjectID;
		tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount].m_byAttributeID = byAttributeID;
		tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount].m_byElementID = byElementID;
		tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount].m_byDataValueLength = 1;
		tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount].m_byDataValue[0] = 0;//设置正常
		tCorrectReturnData.m_byReturnCount += 1;
	}
	else
	{
		tWrongReturnData.m_tDataConfig[tWrongReturnData.m_byReturnCount].m_byIndex = byIndex + 1;
		tWrongReturnData.m_tDataConfig[tWrongReturnData.m_byReturnCount].m_byDataLength = 4 + 1;
		tWrongReturnData.m_tDataConfig[tWrongReturnData.m_byReturnCount].m_byDataClassID = byDataClassID;
		tWrongReturnData.m_tDataConfig[tWrongReturnData.m_byReturnCount].m_byObjectID = byObjectID;
		tWrongReturnData.m_tDataConfig[tWrongReturnData.m_byReturnCount].m_byAttributeID = byAttributeID;
		tWrongReturnData.m_tDataConfig[tWrongReturnData.m_byReturnCount].m_byElementID = byElementID;
		tWrongReturnData.m_tDataConfig[tWrongReturnData.m_byReturnCount].m_byDataValueLength = 1;
		tWrongReturnData.m_tDataConfig[tWrongReturnData.m_byReturnCount].m_byDataValue[0] = byRet;//设置不正常
		tWrongReturnData.m_byReturnCount += 1;
	}
}

void  COpenATCCommWithGB20999Thread::SetDeviceInfo(BYTE byIndex, BYTE byObjectID, BYTE byAttributeID, BYTE byElementID, BYTE byDataValuePos, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData, BYTE byDataValueLength)
{
	if (byObjectID > CONFIG_DATE)
	{
		CreateSetReturnData(BAD_VALUE_NULL, byIndex, DEVICE_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
	}
	else if (byObjectID == MANUFACTURER || byObjectID == DEVICE_VERSION || byObjectID == DEVICE_ID || byObjectID == MANUFACTUR_DATE)
	{
		CreateSetReturnData(BAD_VALUE_READONLY, byIndex, DEVICE_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
	}
	else if (byObjectID == CONFIG_DATE)
	{
		if (byDataValueLength != 11)
		{
			return CreateSetReturnData(BAD_VALUE_WRONGLENGTH, byIndex, DEVICE_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		}

		unsigned short nYear = 0;
		memcpy(&nYear, &m_chUnPackedBuff[byDataValuePos], 2);
		nYear = htons(nYear);
		if (nYear < 1000 || nYear > 9999)
		{
			return CreateSetReturnData(BAD_VALUE_STATUS, byIndex, DEVICE_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		}
		BYTE byMonth = m_chUnPackedBuff[byDataValuePos + 2];
		if (byMonth < 1 || byMonth > 12)
		{
			return CreateSetReturnData(BAD_VALUE_STATUS, byIndex, DEVICE_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		}
		BYTE byDay = m_chUnPackedBuff[byDataValuePos + 3];
		if (byDay < 1 || byDay > 31)
		{
			return CreateSetReturnData(BAD_VALUE_STATUS, byIndex, DEVICE_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		}
		BYTE byHour = m_chUnPackedBuff[byDataValuePos + 4];
		if (byHour < 1 || byHour > 12)
		{
			return CreateSetReturnData(BAD_VALUE_STATUS, byIndex, DEVICE_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		}
		BYTE byMinute = m_chUnPackedBuff[byDataValuePos + 5];
		if (byMinute < 1 || byMinute > 60)
		{
			return CreateSetReturnData(BAD_VALUE_STATUS, byIndex, DEVICE_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		}
		BYTE bySecond = m_chUnPackedBuff[byDataValuePos + 6];
		if (bySecond < 1 || bySecond > 60)
		{
			return CreateSetReturnData(BAD_VALUE_STATUS, byIndex, DEVICE_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		}
	
		memcpy(m_tAscParamInfo.m_stDeviceInfo.m_byDateOfConfig, m_chUnPackedBuff + byDataValuePos, 7);

		m_bIsNeedSave = true;
		CreateSetReturnData(0, byIndex, DEVICE_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
	}
}

void  COpenATCCommWithGB20999Thread::SetBaseInfo(BYTE byIndex, BYTE byObjectID, BYTE byAttributeID, BYTE byElementID, BYTE byDataValuePos, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData, BYTE byDataValueLength)
{
	if (byObjectID > HOST_IPV6_NETCONFIG)
	{
		CreateSetReturnData(BAD_VALUE_NULL, byIndex, BASE_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
	}
	else if (byObjectID == INSTALLATION_ROAD)
	{
		if (byDataValueLength != 132)
		{
			return CreateSetReturnData(BAD_VALUE_WRONGLENGTH, byIndex, BASE_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		}
		memset(m_tAscParamInfo.m_stBaseInfo.m_bySiteRoadID, 0, sizeof(m_tAscParamInfo.m_stBaseInfo.m_bySiteRoadID));
		memcpy(m_tAscParamInfo.m_stBaseInfo.m_bySiteRoadID, &m_chUnPackedBuff[byDataValuePos], sizeof(m_tAscParamInfo.m_stBaseInfo.m_bySiteRoadID));
		m_bIsNeedSave = true;
		CreateSetReturnData(0, byIndex, BASE_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
	}
	else if (byObjectID == ATC_IPV4_NETCONFIG || byObjectID == ATC_IPV6_NETCONFIG)
	{
		if (byObjectID == ATC_IPV4_NETCONFIG && (byAttributeID == ATC_IP_ADDRESS || byAttributeID == SUB_NET || byAttributeID == GATE_WAY) && byDataValueLength != 8)
		{
			return CreateSetReturnData(BAD_VALUE_WRONGLENGTH, byIndex, BASE_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		}
		if (byObjectID == ATC_IPV6_NETCONFIG &&  (byAttributeID == ATC_IP_ADDRESS || byAttributeID == SUB_NET || byAttributeID == GATE_WAY) && byDataValueLength != 20)
		{
			return CreateSetReturnData(BAD_VALUE_WRONGLENGTH, byIndex, BASE_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		}

		if (byAttributeID > GATE_WAY)
		{
			CreateSetReturnData(BAD_VALUE_NULL, byIndex, BASE_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		}
		else
		{
			if (byAttributeID == ATC_IP_ADDRESS)
			{
				if (byObjectID == ATC_IPV4_NETCONFIG)
				{
					memcpy(m_tAscParamInfo.m_stBaseInfo.m_stAscIPV4.m_chAscIPV4IP,  &m_chUnPackedBuff[byDataValuePos], 4);
				}
				else
				{
					memcpy(m_tAscParamInfo.m_stBaseInfo.m_stAscIPV6.m_chAscIPV6IP,  &m_chUnPackedBuff[byDataValuePos], 16);
				}
			}
			else if (byAttributeID == SUB_NET)
			{
				if (byObjectID == ATC_IPV4_NETCONFIG)
				{
					memcpy(m_tAscParamInfo.m_stBaseInfo.m_stAscIPV4.m_chAscIPV4SubNet,  &m_chUnPackedBuff[byDataValuePos], 4);
				}
				else
				{
					memcpy(m_tAscParamInfo.m_stBaseInfo.m_stAscIPV6.m_chAscIPV6SubNet,  &m_chUnPackedBuff[byDataValuePos], 16);
				}
			}
			else if (byAttributeID == GATE_WAY)
			{
				if (byObjectID == ATC_IPV4_NETCONFIG)
				{
					memcpy(m_tAscParamInfo.m_stBaseInfo.m_stAscIPV4.m_chAscIPV4GateWay,  &m_chUnPackedBuff[byDataValuePos], 4);
				}
				else
				{
					memcpy(m_tAscParamInfo.m_stBaseInfo.m_stAscIPV6.m_chAscIPV6GateWay,  &m_chUnPackedBuff[byDataValuePos], 16);
				}
			}
			m_bIsNeedSave = true;
			CreateSetReturnData(0, byIndex, BASE_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		}
	}
	else if (byObjectID == HOST_IPV4_NETCONFIG || byObjectID == HOST_IPV6_NETCONFIG)
	{
		if (byObjectID == HOST_IPV4_NETCONFIG && byAttributeID == HOST_IP_ADDRESS && byAttributeID == HOST_IP_ADDRESS && byDataValueLength != 8)
		{
			return CreateSetReturnData(BAD_VALUE_WRONGLENGTH, byIndex, BASE_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		}
		if (byObjectID == HOST_IPV6_NETCONFIG && byAttributeID == HOST_IP_ADDRESS && byDataValueLength != 20)
		{
			return CreateSetReturnData(BAD_VALUE_WRONGLENGTH, byIndex, BASE_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		}

		if (byAttributeID > COM_TYPE)
		{
			CreateSetReturnData(BAD_VALUE_NULL, byIndex, BASE_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		}
		else
		{
			if (byAttributeID == HOST_IP_ADDRESS)
			{
				if (byObjectID == HOST_IPV4_NETCONFIG)
				{
					memcpy(m_tAscParamInfo.m_stBaseInfo.m_stCerterIPV4.m_chCerterIPV4IP,  &m_chUnPackedBuff[byDataValuePos], 4);
				}
				else
				{
					memcpy(m_tAscParamInfo.m_stBaseInfo.m_stCerterIPV6.m_chCerterIPV6IP,  &m_chUnPackedBuff[byDataValuePos], 16);
				}
			}
			else if (byAttributeID == COM_PORT)
			{
				unsigned short nTemp = 0;
				memcpy(&nTemp, &m_chUnPackedBuff[byDataValuePos], 2);
				if (byObjectID == HOST_IPV4_NETCONFIG)
				{
					m_tAscParamInfo.m_stBaseInfo.m_stCerterIPV4.m_chCerterIPV4Port = htons(nTemp);
				}
				else
				{
					m_tAscParamInfo.m_stBaseInfo.m_stCerterIPV6.m_chCerterIPV6Port = htons(nTemp);
				}
			}
			else if (byAttributeID == COM_TYPE)
			{
				if (m_chUnPackedBuff[byDataValuePos] == 1 || m_chUnPackedBuff[byDataValuePos] == 2 || m_chUnPackedBuff[byDataValuePos] ==3)
				{
					if (byObjectID == HOST_IPV4_NETCONFIG)
					{
						m_tAscParamInfo.m_stBaseInfo.m_stCerterIPV4.m_chCerterIPV4Type = m_chUnPackedBuff[byDataValuePos];
					}
					else
					{
						m_tAscParamInfo.m_stBaseInfo.m_stCerterIPV6.m_chCerterIPV6Type = m_chUnPackedBuff[byDataValuePos];
					}
				}
				else
				{
					return CreateSetReturnData(BAD_VALUE_OVERFLOW, byIndex, BASE_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
				}
			}
			m_bIsNeedSave = true;
			CreateSetReturnData(0, byIndex, BASE_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		}
	}
	else if (byObjectID == ATC_TIMEZONE)
	{
		int nTimeZone = 0;
		memcpy(&nTimeZone, m_chUnPackedBuff + byDataValuePos, 4);
		nTimeZone = htonl(nTimeZone);
		if (nTimeZone > 43200 || nTimeZone < -43200)
		{
			return CreateSetReturnData(BAD_VALUE_OVERFLOW, byIndex, BASE_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		}

		m_tAscParamInfo.m_stBaseInfo.m_wTimeZone = nTimeZone;
		//CTimeZoneInf::Instance()->QueryTZI();
		//int nTZ = m_tAscParamInfo.m_stBaseInfo.m_stTimeZone.m_wTimeZone / 3600;
		//CTimeZoneInf::Instance()->SetTZI(nTZ);
		SetSystemTimeZone();
		m_bIsNeedSave = true;
		CreateSetReturnData(0, byIndex, BASE_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		
	}
	else if (byObjectID == ATC_ID)
	{
		int nATCID = 0;
		memcpy(&nATCID, &m_chUnPackedBuff[byDataValuePos], 4);
		m_tAscParamInfo.m_stBaseInfo.m_wATCCode = htonl(nATCID);
		CreateSetReturnData(0, byIndex, BASE_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
	}
	else if (byObjectID == ATC_ROADCOUNT)
	{
		if (m_chUnPackedBuff[byDataValuePos] >= 1 && m_chUnPackedBuff[byDataValuePos] <= 8)
		{
			m_tAscParamInfo.m_stBaseInfo.m_byCrossRoadNum = m_chUnPackedBuff[byDataValuePos];
		}
		else
		{
			return CreateSetReturnData(BAD_VALUE_OVERFLOW, byIndex, BASE_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		}
		m_bIsNeedSave = true;
		CreateSetReturnData(0, byIndex, BASE_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
	}
	else if (byObjectID == GPS_CLOCK_FLAG)
	{
		CreateSetReturnData(BAD_VALUE_READONLY, byIndex, BASE_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
	}
}

void  COpenATCCommWithGB20999Thread::SetLightGroupInfo(BYTE byIndex, BYTE byObjectID, BYTE byAttributeID, BYTE byElementID, BYTE byDataValuePos, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData, BYTE byDataValueLength)
{
	if (byObjectID == LIGHT_GROUP_COUNT || byObjectID == LIGHT_GROUP_STATUS_TABLE)
	{
		CreateSetReturnData(BAD_VALUE_READONLY, byIndex, LIGHTGROUP_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
	}
	else if (byObjectID == LIGHT_GROUP_CONFIG_TABLE)
	{
		if (byAttributeID == LIGHT_GROUP_TYPE_ID)
		{
			CreateSetReturnData(BAD_VALUE_READONLY, byIndex, LIGHTGROUP_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		}
		else if (byAttributeID > LIGHT_GROUP_TYPE)
		{
			CreateSetReturnData(BAD_VALUE_NULL, byIndex, LIGHTGROUP_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		}
		else
		{
			SetLightGroupConfigInfo(byIndex, byObjectID, byAttributeID, byElementID, byDataValuePos, tCorrectReturnData, tWrongReturnData, byDataValueLength);
		}
	}
	else if (byObjectID == LIGHT_GROUP_CONTROL_TABLE)
	{
		if (byAttributeID == LIGHT_GROUP_TYPE_ID)
		{
			CreateSetReturnData(BAD_VALUE_READONLY, byIndex, LIGHTGROUP_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		}
		else if (byAttributeID > CONTROL_PROHIBIT)
		{
			CreateSetReturnData(BAD_VALUE_NULL, byIndex, LIGHTGROUP_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		}
		else
		{
			SetLightGroupControlInfo(byIndex, byObjectID, byAttributeID, byElementID, byDataValuePos, tCorrectReturnData, tWrongReturnData, byDataValueLength);
		}
	}
}

void  COpenATCCommWithGB20999Thread::SetLightGroupConfigInfo(BYTE byIndex, BYTE byObjectID, BYTE byAttributeID, BYTE byElementID, BYTE byDataValuePos, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData, BYTE byDataValueLength)
{
	if (byElementID < 1 || byElementID > 64)
	{
		return CreateSetReturnData(BAD_VALUE_NULL, byIndex, LIGHTGROUP_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
	}

	int  byChannelIndex = -1;
	int  i = 0;
	for (i = 0;i < MAX_PHASE_COUNT_20999;i++)
	{
		if (m_tAscParamInfo.m_stAscChannelTable[i].m_byChannelNumber == byElementID)
		{
			byChannelIndex = i;
			break;
		}
	}

	if (byChannelIndex == -1)
	{
		byChannelIndex = byElementID - 1;
		m_tVerifyParamInfo.m_stAscChannelTable[byChannelIndex].m_byChannelNumber = byElementID;
		m_tAscParamInfo.m_stChannelTableValidSize += 1;			//计算通道表数量--HPH
	}
	
	if (byAttributeID == LIGHT_GROUP_TYPE)
	{
		if (m_chUnPackedBuff[byDataValuePos] == LIGHT_GROUP_VEHICLE ||m_chUnPackedBuff[byDataValuePos] == LIGHT_GROUP_NOVEHICLE || m_chUnPackedBuff[byDataValuePos] == LIGHT_GROUP_PED || m_chUnPackedBuff[byDataValuePos] == LIGHT_GROUP_ROAD || 
			m_chUnPackedBuff[byDataValuePos] == LIGHT_GROUP_ALTERABLE_TRAFFIC || m_chUnPackedBuff[byDataValuePos] == LIGHT_GROUP_BUS || m_chUnPackedBuff[byDataValuePos] == LIGHT_GROUP_TRAM || m_chUnPackedBuff[byDataValuePos] == LIGHT_GROUP_SPECIALBUS)
		{
			m_tVerifyParamInfo.m_stAscChannelTable[byChannelIndex].m_byLightControlType = m_chUnPackedBuff[byDataValuePos];
			//if (m_chUnPackedBuff[byDataValuePos] == LIGHT_GROUP_PED)
			//{
			//	m_tAscParamInfo.m_stAscChannelTable[byChannelIndex].m_byChannelControlType = PED_CHA;
			//}
			//else if (m_chUnPackedBuff[byDataValuePos] == LIGHT_GROUP_ROAD)
			//{
			//	m_tAscParamInfo.m_stAscChannelTable[byChannelIndex].m_byChannelControlType = LANEWAY_LIGHT_CHA;
			//}
			////else if (m_chUnPackedBuff[byDataValuePos] == LIGHT_GROUP_VEHICLE || m_chUnPackedBuff[byDataValuePos] == LIGHT_GROUP_NOVEHICLE)
			//else if (m_chUnPackedBuff[byDataValuePos] == LIGHT_GROUP_VEHICLE)
			//{
			//	m_tAscParamInfo.m_stAscChannelTable[byChannelIndex].m_byChannelControlType = VEH_CHA;
			//}
			//else if (m_chUnPackedBuff[byDataValuePos] == LIGHT_GROUP_NOVEHICLE)							//非机动车（算机动车车道）--HPH
			//{
			//	m_tAscParamInfo.m_stAscChannelTable[byChannelIndex].m_byChannelControlType = VEH_CHA;
			//}
			//else
			//{
			//	m_tAscParamInfo.m_stAscChannelTable[byChannelIndex].m_byChannelControlType = OTHER_CHA;
			//}
		}
		else
		{
			return CreateSetReturnData(BAD_VALUE_OVERFLOW, byIndex, LIGHTGROUP_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		}
	}
	else
	{
		return CreateSetReturnData(BAD_VALUE_OVERFLOW, byIndex, LIGHTGROUP_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
	}

	CreateSetReturnData(0, byIndex, LIGHTGROUP_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
}

void  COpenATCCommWithGB20999Thread::SetLightGroupControlInfo(BYTE byIndex, BYTE byObjectID, BYTE byAttributeID, BYTE byElementID, BYTE byDataValuePos, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData, BYTE byDataValueLength)
{
	//修改原因，怎加不通过事务机单独下发的能力。 --2022.07.22
	if(m_tDBManagement.m_byDBCreateTransaction == TRANSACTION)		//当处于事务机传输时
	{
		if (byElementID < 1 || byElementID > 64)
		{
			return CreateSetReturnData(BAD_VALUE_NULL, byIndex, LIGHTGROUP_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		}

		int  byChannelIndex = -1;
		int  i = 0;
		for (i = 0;i < MAX_PHASE_COUNT_20999;i++)
		{
			if (m_tVerifyParamInfo.m_stAscChannelTable[i].m_byChannelNumber == byElementID)
			{
				byChannelIndex = i;
				break;
			}
		}

		if (byChannelIndex == -1)
		{
			byChannelIndex = byElementID - 1;
			m_tVerifyParamInfo.m_stAscChannelTable[byChannelIndex].m_byChannelNumber = byElementID;
		}

		if (byAttributeID == CONTROL_SHIELD)
		{
			if (m_chUnPackedBuff[byDataValuePos] == 0 || m_chUnPackedBuff[byDataValuePos] == 1)
			{
				m_tVerifyParamInfo.m_stAscChannelTable[byChannelIndex].m_byScreenFlag = m_chUnPackedBuff[byDataValuePos];
			}
			else
			{
				return CreateSetReturnData(BAD_VALUE_OVERFLOW, byIndex, LIGHTGROUP_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
			}
		}
		else if (byAttributeID == CONTROL_PROHIBIT)
		{
			if (m_chUnPackedBuff[byDataValuePos] == 0 || m_chUnPackedBuff[byDataValuePos] == 1)
			{
				m_tVerifyParamInfo.m_stAscChannelTable[byChannelIndex].m_byForbiddenFlag = m_chUnPackedBuff[byDataValuePos];
			}
			else
			{
				return CreateSetReturnData(BAD_VALUE_OVERFLOW, byIndex, LIGHTGROUP_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
			}
		}
	
		CreateSetReturnData(0, byIndex, LIGHTGROUP_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
	}
	else
	{
		if (byElementID < 1 || byElementID > 64)
		{
			return CreateSetReturnData(BAD_VALUE_NULL, byIndex, LIGHTGROUP_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		}

		int  byChannelIndex = -1;
		int  i = 0;
		for (i = 0;i < MAX_PHASE_COUNT_20999;i++)
		{
			if (m_tAscParamInfo.m_stAscChannelTable[i].m_byChannelNumber == byElementID)
			{
				byChannelIndex = i;
				break;
			}
		}

		if (byChannelIndex == -1)
		{
			byChannelIndex = byElementID - 1;
			m_tAscParamInfo.m_stAscChannelTable[byChannelIndex].m_byChannelNumber = byElementID;
		}

		if (byAttributeID == CONTROL_SHIELD)
		{
			if (m_chUnPackedBuff[byDataValuePos] == 0 || m_chUnPackedBuff[byDataValuePos] == 1)
			{
				m_tAscParamInfo.m_stAscChannelTable[byChannelIndex].m_byScreenFlag = m_chUnPackedBuff[byDataValuePos];
			}
			else
			{
				return CreateSetReturnData(BAD_VALUE_OVERFLOW, byIndex, LIGHTGROUP_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
			}
		}
		else if (byAttributeID == CONTROL_PROHIBIT)
		{
			if (m_chUnPackedBuff[byDataValuePos] == 0 || m_chUnPackedBuff[byDataValuePos] == 1)
			{
				m_tAscParamInfo.m_stAscChannelTable[byChannelIndex].m_byForbiddenFlag = m_chUnPackedBuff[byDataValuePos];
			}
			else
			{
				return CreateSetReturnData(BAD_VALUE_OVERFLOW, byIndex, LIGHTGROUP_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
			}
		}
		
		m_bIsNeedSave = true;
		CreateSetReturnData(0, byIndex, LIGHTGROUP_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
	}


	/*if (byElementID < 1 || byElementID > 64)
	{
		return CreateSetReturnData(BAD_VALUE_NULL, byIndex, LIGHTGROUP_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
	}

	int  byChannelIndex = -1;
	int  i = 0;
	for (i = 0;i < MAX_CHANNEL_COUNT;i++)
	{
		if (m_tVerifyParamInfo.m_stAscChannelTable[i].m_byChannelNumber == byElementID)
		{
			byChannelIndex = i;
			break;
		}
	}

	if (byChannelIndex == -1)
	{
		byChannelIndex = byElementID - 1;
		m_tVerifyParamInfo.m_stAscChannelTable[byChannelIndex].m_byChannelNumber = byElementID;
	}

	if (byAttributeID == CONTROL_SHIELD)
	{
		if (m_chUnPackedBuff[byDataValuePos] == 0 || m_chUnPackedBuff[byDataValuePos] == 1)
		{
			m_tVerifyParamInfo.m_stAscChannelTable[byChannelIndex].m_byScreenFlag = m_chUnPackedBuff[byDataValuePos];
		}
		else
		{
			return CreateSetReturnData(BAD_VALUE_OVERFLOW, byIndex, LIGHTGROUP_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		}
	}
	else if (byAttributeID == CONTROL_PROHIBIT)
	{
		if (m_chUnPackedBuff[byDataValuePos] == 0 || m_chUnPackedBuff[byDataValuePos] == 1)
		{
			m_tVerifyParamInfo.m_stAscChannelTable[byChannelIndex].m_byForbiddenFlag = m_chUnPackedBuff[byDataValuePos];
		}
		else
		{
			return CreateSetReturnData(BAD_VALUE_OVERFLOW, byIndex, LIGHTGROUP_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		}
	}
	
	CreateSetReturnData(0, byIndex, LIGHTGROUP_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);*/
}

void  COpenATCCommWithGB20999Thread::SetPhaseInfo(BYTE byIndex, BYTE byObjectID, BYTE byAttributeID, BYTE byElementID, BYTE byDataValuePos, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData, BYTE byDataValueLength)
{
	if (byObjectID == PHASE_COUNT)
	{
		CreateSetReturnData(BAD_VALUE_READONLY, byIndex, PHASE_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
	}
	if (byObjectID == PHASE_CONFIG_TABLE)
	{
		if (byAttributeID == PHASE_CONFIG_ID)
		{
			CreateSetReturnData(BAD_VALUE_READONLY, byIndex, PHASE_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		}
		else if (byAttributeID > PHASE_CALL)
		{
			CreateSetReturnData(BAD_VALUE_NULL, byIndex, PHASE_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		}
		else
		{
			SetPhaseConfigInfo(byIndex, byObjectID, byAttributeID, byElementID, byDataValuePos, tCorrectReturnData, tWrongReturnData, byDataValueLength);
		}
	}
	else if (byObjectID == PHASE_CONTROL_TABLE)
	{
		if (byAttributeID == CONTROL_ID)
		{
			CreateSetReturnData(BAD_VALUE_NULL, byIndex, PHASE_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		}
		else if (byAttributeID > CONTROL_PROHIBIT)
		{
			CreateSetReturnData(BAD_VALUE_NULL, byIndex, PHASE_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		}
		else
		{
			SetPhaseControlInfo(byIndex, byObjectID, byAttributeID, byElementID, byDataValuePos, tCorrectReturnData, tWrongReturnData, byDataValueLength);
		}
	}
}

void  COpenATCCommWithGB20999Thread::SetPhaseConfigInfo(BYTE byIndex, BYTE byObjectID, BYTE byAttributeID, BYTE byElementID, BYTE byDataValuePos, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData, BYTE byDataValueLength)
{
	if (byElementID < 1 || byElementID > 64)
	{
		return CreateSetReturnData(BAD_VALUE_NULL, byIndex, PHASE_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
	}

	int  byPhaseIndex = -1;
	int  i = 0, j = 0;
	for (i = 0;i < MAX_PHASE_COUNT_20999;i++)
	{
		if (m_tVerifyParamInfo.m_stAscPhaseTable[i].m_byPhaseNumber == byElementID)
		{
			byPhaseIndex = i;
			break;
		}
	}

	if (byPhaseIndex == -1)
	{
		byPhaseIndex = byElementID - 1;
		m_tVerifyParamInfo.m_stAscPhaseTable[byPhaseIndex].m_byPhaseNumber = byElementID;
		m_tVerifyParamInfo.m_stPhaseTableValidSize += 1;
		//return CreateSetReturnData(BAD_VALUE_NULL, byIndex, PHASE_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
	}
	
	if (byAttributeID == PHASE_LIGHTGROUP)
	{
		if (byDataValueLength == 12)
		{
			for (i = 0;i < C_N_MAX_PHASE_LIGHTGROUP;i++)
			{
				m_tVerifyParamInfo.m_stAscPhaseTable[byPhaseIndex].m_byLightGroup[i] = m_chUnPackedBuff[byDataValuePos + i];
				//printf("%d ",m_tVerifyParamInfo.m_stAscPhaseTable[byPhaseIndex].m_byLightGroup[i]);
				//int nLight = 1;
				//int nIndex;
				//for(int j = 0;j<8;j++)
				//{
				//	if ((m_tVerifyParamInfo.m_stAscPhaseTable[byPhaseIndex].m_byLightGroup[i]&nLight) == nLight)
				//	{
				//		nIndex = (7-i)*8+j;
				//		m_tVerifyParamInfo.m_stAscChannelTable[nIndex].m_byChannelControlSource = m_tVerifyParamInfo.m_stAscPhaseTable[byPhaseIndex].m_byPhaseNumber;

				//	}
				//	nLight = nLight<<1;
				//}
				////设置通道表对应控制源--HPH
			}
		}
		else
		{
			return CreateSetReturnData(BAD_VALUE_WRONGLENGTH, byIndex, PHASE_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		}
	}
	else if (byAttributeID == PHASE_LOSETRANSITIONTYPE1)
	{
		if (m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_OFF && m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_RED &&
			m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_REDFLASH  && m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_REDFASTFLASH &&
			m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_GREEN  && m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_GREENFLASH &&
			m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_GREENFASTFLASH  && m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_YELLOW &&
			m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_YELLOWFLASH  && m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_YELLOWFASTFLASH && m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_REDYELLOW)
		{
			return CreateSetReturnData(BAD_VALUE_OVERFLOW, byIndex, PHASE_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		}
		else if (m_chUnPackedBuff[byDataValuePos] == LIGHT_STATUS_OFF || m_chUnPackedBuff[byDataValuePos] == LIGHT_STATUS_RED ||
			m_chUnPackedBuff[byDataValuePos] == LIGHT_STATUS_REDFLASH || m_chUnPackedBuff[byDataValuePos] == LIGHT_STATUS_REDFASTFLASH ||
			m_chUnPackedBuff[byDataValuePos] == LIGHT_STATUS_GREEN  || m_chUnPackedBuff[byDataValuePos] == LIGHT_STATUS_GREENFASTFLASH  ||
			m_chUnPackedBuff[byDataValuePos] == LIGHT_STATUS_YELLOWFLASH  || m_chUnPackedBuff[byDataValuePos] == LIGHT_STATUS_YELLOWFASTFLASH || 
			m_chUnPackedBuff[byDataValuePos] == LIGHT_STATUS_REDYELLOW)
		{
			return CreateSetReturnData(BAD_VALUE_STATUS, byIndex, PHASE_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		}
		m_tVerifyParamInfo.m_stAscPhaseTable[byPhaseIndex].m_nLoseControlLampOneType = m_chUnPackedBuff[byDataValuePos];
	}
	else if (byAttributeID == PHASE_LOSETRANSITIONTIME1)
	{
		m_tVerifyParamInfo.m_stAscPhaseTable[byPhaseIndex].m_nLoseControlLampOneTime = m_chUnPackedBuff[byDataValuePos];
	}
	else if (byAttributeID == PHASE_LOSETRANSITIONTYPE2)
	{
		if (m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_OFF && m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_RED &&
			m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_REDFLASH  && m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_REDFASTFLASH &&
			m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_GREEN  && m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_GREENFLASH &&
			m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_GREENFASTFLASH  && m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_YELLOW &&
			m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_YELLOWFLASH  && m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_YELLOWFASTFLASH && m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_REDYELLOW)
		{
			return CreateSetReturnData(BAD_VALUE_OVERFLOW, byIndex, PHASE_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		}
		m_tVerifyParamInfo.m_stAscPhaseTable[byPhaseIndex].m_nLoseControlLampTwoType = m_chUnPackedBuff[byDataValuePos];
	}
	else if (byAttributeID == PHASE_LOSETRANSITIONTIME2)
	{
		m_tVerifyParamInfo.m_stAscPhaseTable[byPhaseIndex].m_nLoseControlLampTwoTime = m_chUnPackedBuff[byDataValuePos];
	}
	else if (byAttributeID == PHASE_LOSETRANSITIONTYPE3)
	{
		if (m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_OFF && m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_RED &&
			m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_REDFLASH  && m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_REDFASTFLASH &&
			m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_GREEN  && m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_GREENFLASH &&
			m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_GREENFASTFLASH  && m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_YELLOW &&
			m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_YELLOWFLASH  && m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_YELLOWFASTFLASH && m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_REDYELLOW)
		{
			return CreateSetReturnData(BAD_VALUE_OVERFLOW, byIndex, PHASE_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		}
		m_tVerifyParamInfo.m_stAscPhaseTable[byPhaseIndex].m_nLoseControlLampThreeType = m_chUnPackedBuff[byDataValuePos];
	}
	else if (byAttributeID == PHASE_LOSETRANSITIONTIME3)
	{
		m_tVerifyParamInfo.m_stAscPhaseTable[byPhaseIndex].m_nLoseControlLampThreeTime = m_chUnPackedBuff[byDataValuePos];
	}
	else if (byAttributeID == PHASE_GETTRANSITIONTYPE1)
	{
		if (m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_OFF && m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_RED &&
			m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_REDFLASH  && m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_REDFASTFLASH &&
			m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_GREEN  && m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_GREENFLASH &&
			m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_GREENFASTFLASH  && m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_YELLOW &&
			m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_YELLOWFLASH  && m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_YELLOWFASTFLASH && m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_REDYELLOW)
		{
			return CreateSetReturnData(BAD_VALUE_OVERFLOW, byIndex, PHASE_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		}
		m_tVerifyParamInfo.m_stAscPhaseTable[byPhaseIndex].m_nGetControlLampOneType = m_chUnPackedBuff[byDataValuePos];
	}
	else if (byAttributeID == PHASE_GETTRANSITIONTIME1)
	{
		m_tVerifyParamInfo.m_stAscPhaseTable[byPhaseIndex].m_nGetControlLampOneTime = m_chUnPackedBuff[byDataValuePos];
	}
	else if (byAttributeID == PHASE_GETTRANSITIONTYPE2)
	{
		if (m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_OFF && m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_RED &&
			m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_REDFLASH  && m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_REDFASTFLASH &&
			m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_GREEN  && m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_GREENFLASH &&
			m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_GREENFASTFLASH  && m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_YELLOW &&
			m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_YELLOWFLASH  && m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_YELLOWFASTFLASH && m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_REDYELLOW)
		{
			return CreateSetReturnData(BAD_VALUE_OVERFLOW, byIndex, PHASE_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		}
		m_tVerifyParamInfo.m_stAscPhaseTable[byPhaseIndex].m_nGetControlLampTwoType = m_chUnPackedBuff[byDataValuePos];
	}
	else if (byAttributeID == PHASE_GETTRANSITIONTIME2)
	{
		m_tVerifyParamInfo.m_stAscPhaseTable[byPhaseIndex].m_nGetControlLampTwoTime = m_chUnPackedBuff[byDataValuePos];
	}
	else if (byAttributeID == PHASE_GETTRANSITIONTYPE3)
	{
		if (m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_OFF && m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_RED &&
			m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_REDFLASH  && m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_REDFASTFLASH &&
			m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_GREEN  && m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_GREENFLASH &&
			m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_GREENFASTFLASH  && m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_YELLOW &&
			m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_YELLOWFLASH  && m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_YELLOWFASTFLASH && m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_REDYELLOW)
		{
			return CreateSetReturnData(BAD_VALUE_OVERFLOW, byIndex, PHASE_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		}
		m_tVerifyParamInfo.m_stAscPhaseTable[byPhaseIndex].m_nGetControlLampThreeType = m_chUnPackedBuff[byDataValuePos];
	}
	else if (byAttributeID == PHASE_GETTRANSITIONTIME3)
	{
		m_tVerifyParamInfo.m_stAscPhaseTable[byPhaseIndex].m_nGetControlLampThreeTime = m_chUnPackedBuff[byDataValuePos];
	}
	else if (byAttributeID == PHASE_TURNONGETTRANSITIONTYPE1)
	{
		//根据信号机国标，开机至少10秒黄闪5秒全红，所以过度灯色1为黄闪、2为全红--HPH
		if (m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_OFF && m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_RED &&
			m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_REDFLASH  && m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_REDFASTFLASH &&
			m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_GREEN  && m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_GREENFLASH &&
			m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_GREENFASTFLASH  && m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_YELLOW &&
			m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_YELLOWFLASH  && m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_YELLOWFASTFLASH && m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_REDYELLOW)
		{
			return CreateSetReturnData(BAD_VALUE_OVERFLOW, byIndex, PHASE_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		}
		m_tVerifyParamInfo.m_stAscPhaseTable[byPhaseIndex].m_nPowerOnGetControlLampOneType = m_chUnPackedBuff[byDataValuePos];
	}
	else if (byAttributeID == PHASE_TURNONGETTRANSITIONTIME1)
	{
		/*if (m_chUnPackedBuff[byDataValuePos]<10)
		{
			return CreateSetReturnData(BAD_VALUE_STATUS, byIndex, PHASE_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		}
		else if (byPhaseIndex == 0)
		{
			m_tVerifyParamInfo.m_stAscPhaseTable[byPhaseIndex].m_nPowerOnGetControlLampOneTime = m_chUnPackedBuff[byDataValuePos];
		}
		else if (byPhaseIndex != 0 && m_chUnPackedBuff[byDataValuePos] != m_tVerifyParamInfo.m_stAscPhaseTable[0].m_nPowerOnGetControlLampOneTime)
		{
			return CreateSetReturnData(BAD_VALUE_STATUS, byIndex, PHASE_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		}
		else if (byPhaseIndex != 0 && m_chUnPackedBuff[byDataValuePos] == m_tVerifyParamInfo.m_stAscPhaseTable[0].m_nPowerOnGetControlLampOneTime)
		{
			m_tVerifyParamInfo.m_stAscPhaseTable[byPhaseIndex].m_nPowerOnGetControlLampOneTime = m_chUnPackedBuff[byDataValuePos];
		}*/
		m_tVerifyParamInfo.m_stAscPhaseTable[byPhaseIndex].m_nPowerOnGetControlLampOneTime = m_chUnPackedBuff[byDataValuePos];
		//m_tAscParamInfo.m_stAscPhaseTable[byPhaseIndex].m_nPowerOnGetControlLampOneTime = m_chUnPackedBuff[byDataValuePos];
		//m_tAscParamInfo.m_stAscStartSequenceInfo.m_byStartYellowFlash = m_chUnPackedBuff[byDataValuePos];
	}
	else if (byAttributeID == PHASE_TURNONGETTRANSITIONTYPE2)
	{
		if (m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_OFF && m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_RED &&
			m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_REDFLASH  && m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_REDFASTFLASH &&
			m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_GREEN  && m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_GREENFLASH &&
			m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_GREENFASTFLASH  && m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_YELLOW &&
			m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_YELLOWFLASH  && m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_YELLOWFASTFLASH && m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_REDYELLOW)
		{
			return CreateSetReturnData(BAD_VALUE_OVERFLOW, byIndex, PHASE_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		}
		m_tVerifyParamInfo.m_stAscPhaseTable[byPhaseIndex].m_nPowerOnGetControlLampTwoType = m_chUnPackedBuff[byDataValuePos];
	}
	else if (byAttributeID == PHASE_TURNONGETTRANSITIONTIME2)
	{
		/*if (m_chUnPackedBuff[byDataValuePos]<5)
		{
			return CreateSetReturnData(BAD_VALUE_STATUS, byIndex, PHASE_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		}
		else if (byPhaseIndex == 0)
		{
			m_tVerifyParamInfo.m_stAscPhaseTable[byPhaseIndex].m_nPowerOnGetControlLampTwoTime = m_chUnPackedBuff[byDataValuePos];
		}
		else if (byPhaseIndex != 0 && m_chUnPackedBuff[byDataValuePos] != m_tVerifyParamInfo.m_stAscPhaseTable[0].m_nPowerOnGetControlLampTwoTime)
		{
			return CreateSetReturnData(BAD_VALUE_STATUS, byIndex, PHASE_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		}
		else if (byPhaseIndex != 0 && m_chUnPackedBuff[byDataValuePos] != m_tVerifyParamInfo.m_stAscPhaseTable[0].m_nPowerOnGetControlLampTwoTime)
		{
			m_tVerifyParamInfo.m_stAscPhaseTable[byPhaseIndex].m_nPowerOnGetControlLampTwoTime = m_chUnPackedBuff[byDataValuePos];
		}*/

		m_tVerifyParamInfo.m_stAscPhaseTable[byPhaseIndex].m_nPowerOnGetControlLampTwoTime = m_chUnPackedBuff[byDataValuePos];
		//m_tAscParamInfo.m_stAscStartSequenceInfo.m_byStartAllRed = m_chUnPackedBuff[byDataValuePos];
	}
	else if (byAttributeID == PHASE_TURNONGETTRANSITIONTYPE3)
	{
		if (m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_OFF && m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_RED &&
			m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_REDFLASH  && m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_REDFASTFLASH &&
			m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_GREEN  && m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_GREENFLASH &&
			m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_GREENFASTFLASH  && m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_YELLOW &&
			m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_YELLOWFLASH  && m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_YELLOWFASTFLASH && m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_REDYELLOW)
		{
			return CreateSetReturnData(BAD_VALUE_OVERFLOW, byIndex, PHASE_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		}
		m_tVerifyParamInfo.m_stAscPhaseTable[byPhaseIndex].m_nPowerOnGetControlLampThreeType = m_chUnPackedBuff[byDataValuePos];
	}
	else if (byAttributeID == PHASE_TURNONGETTRANSITIONTIME3)
	{
		m_tVerifyParamInfo.m_stAscPhaseTable[byPhaseIndex].m_nPowerOnGetControlLampThreeTime = m_chUnPackedBuff[byDataValuePos];
	}
	else if (byAttributeID == PHASE_TURNONLOSETRANSITIONTYPE1)
	{
		if (m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_OFF && m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_RED &&
			m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_REDFLASH  && m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_REDFASTFLASH &&
			m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_GREEN  && m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_GREENFLASH &&
			m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_GREENFASTFLASH  && m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_YELLOW &&
			m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_YELLOWFLASH  && m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_YELLOWFASTFLASH && m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_REDYELLOW)
		{
			return CreateSetReturnData(BAD_VALUE_OVERFLOW, byIndex, PHASE_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		}
		m_tVerifyParamInfo.m_stAscPhaseTable[byPhaseIndex].m_nPowerOnLossControlLampOneType = m_chUnPackedBuff[byDataValuePos];
	}
	else if (byAttributeID == PHASE_TURNONLOSETRANSITIONTIME1)
	{
		if (m_chUnPackedBuff[byDataValuePos] == m_tVerifyParamInfo.m_stAscPhaseTable[byPhaseIndex].m_nPowerOnGetControlLampOneTime)
		{
			m_tVerifyParamInfo.m_stAscPhaseTable[byPhaseIndex].m_nPowerOnLossControlLampOneTime = m_chUnPackedBuff[byDataValuePos];
			//return CreateSetReturnData(BAD_VALUE_STATUS, byIndex, PHASE_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		}
		m_tVerifyParamInfo.m_stAscPhaseTable[byPhaseIndex].m_nPowerOnLossControlLampOneTime = m_chUnPackedBuff[byDataValuePos];
		//m_tAscParamInfo.m_stAscStartSequenceInfo.m_byStartYellowFlash = m_chUnPackedBuff[byDataValuePos];
	}
	else if (byAttributeID == PHASE_TURNONLOSETRANSITIONTYPE2)
	{
		if (m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_OFF && m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_RED &&
			m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_REDFLASH  && m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_REDFASTFLASH &&
			m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_GREEN  && m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_GREENFLASH &&
			m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_GREENFASTFLASH  && m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_YELLOW &&
			m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_YELLOWFLASH  && m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_YELLOWFASTFLASH && m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_REDYELLOW)
		{
			return CreateSetReturnData(BAD_VALUE_OVERFLOW, byIndex, PHASE_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		}
		m_tVerifyParamInfo.m_stAscPhaseTable[byPhaseIndex].m_nPowerOnLossControlLampTwoType = m_chUnPackedBuff[byDataValuePos];
	}
	else if (byAttributeID == PHASE_TURNONLOSETRANSITIONTIME2)
	{
		//if (m_chUnPackedBuff[byDataValuePos] == m_tVerifyParamInfo.m_stAscPhaseTable[byPhaseIndex].m_nPowerOnGetControlLampTwoTime)
		//{
		//	m_tVerifyParamInfo.m_stAscPhaseTable[byPhaseIndex].m_nPowerOnLossControlLampTwoTime = m_chUnPackedBuff[byDataValuePos];
		//	//return CreateSetReturnData(BAD_VALUE_STATUS, byIndex, PHASE_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		//}
		//else
		//{
		//	return CreateSetReturnData(BAD_VALUE_STATUS, byIndex, PHASE_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		//}
		m_tVerifyParamInfo.m_stAscPhaseTable[byPhaseIndex].m_nPowerOnLossControlLampTwoTime = m_chUnPackedBuff[byDataValuePos];
		//m_tAscParamInfo.m_stAscStartSequenceInfo.m_byStartAllRed = m_chUnPackedBuff[byDataValuePos];
	}
	else if (byAttributeID == PHASE_TURNONLOSETRANSITIONTYPE3)
	{
		if (m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_OFF && m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_RED &&
			m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_REDFLASH  && m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_REDFASTFLASH &&
			m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_GREEN  && m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_GREENFLASH &&
			m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_GREENFASTFLASH  && m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_YELLOW &&
			m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_YELLOWFLASH  && m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_YELLOWFASTFLASH && m_chUnPackedBuff[byDataValuePos] != LIGHT_STATUS_REDYELLOW)
		{
			return CreateSetReturnData(BAD_VALUE_OVERFLOW, byIndex, PHASE_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		}
		m_tVerifyParamInfo.m_stAscPhaseTable[byPhaseIndex].m_nPowerOnLossControlLampThreeType = m_chUnPackedBuff[byDataValuePos];
	}
	else if (byAttributeID == PHASE_TURNONLOSETRANSITIONTIME3)
	{
		m_tVerifyParamInfo.m_stAscPhaseTable[byPhaseIndex].m_nPowerOnLossControlLampThreeTime = m_chUnPackedBuff[byDataValuePos];
	}
	else if (byAttributeID == PHASE_MIN_GREENTIME)
	{
		unsigned short nTemp = 0;
		memcpy(&nTemp, &m_chUnPackedBuff[byDataValuePos], 2);
		m_tVerifyParamInfo.m_stAscPhaseTable[byPhaseIndex].m_wPhaseMinimumGreen = htons(nTemp);
	}
	else if (byAttributeID == PHASE_MAX1_GREENTIME)
	{
	    unsigned short nTemp = 0;
		memcpy(&nTemp, &m_chUnPackedBuff[byDataValuePos], 2);
		m_tVerifyParamInfo.m_stAscPhaseTable[byPhaseIndex].m_wPhaseMaximum1 = htons(nTemp);
	}
	else if (byAttributeID == PHASE_MAX2_GREENTIME)
	{
		unsigned short nTemp = 0;
		memcpy(&nTemp, &m_chUnPackedBuff[byDataValuePos], 2);
		m_tVerifyParamInfo.m_stAscPhaseTable[byPhaseIndex].m_wPhaseMaximum2 = htons(nTemp);
	}
	else if (byAttributeID == PHASE_PASSAGE_GREENTIME)
	{
		unsigned short nTemp = 0;
		memcpy(&nTemp, &m_chUnPackedBuff[byDataValuePos], 2);
		//m_tAscParamInfo.m_stAscPhaseTable[byPhaseIndex].m_byPhasePassage = htons(nTemp);
		m_tVerifyParamInfo.m_stAscPhaseTable[byPhaseIndex].m_byPhaseExtend = htons(nTemp);
	}
	else if (byAttributeID == PHASE_CALL)
	{
		if (byDataValueLength == 12)
		{
			for (i = 0;i < C_N_MAX_PHASE_DETECTOR;i++)
			{
				m_tVerifyParamInfo.m_stAscPhaseTable[byPhaseIndex].m_byPhaseCall[i] = m_chUnPackedBuff[byDataValuePos + i];
				printf("byPhaseIndex:%d m_byPhaseCall[%d]:%d m_chUnPackedBuff[byDataValuePos + %d]:%d\n",byPhaseIndex,i,m_tVerifyParamInfo.m_stAscPhaseTable[byPhaseIndex].m_byPhaseCall[i],i,m_chUnPackedBuff[byDataValuePos + i]);
			}
		}
		else
		{
			return CreateSetReturnData(BAD_VALUE_WRONGLENGTH, byIndex, PHASE_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		}
	}
	CreateSetReturnData(0, byIndex, PHASE_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
}

void  COpenATCCommWithGB20999Thread::SetPhaseControlInfo(BYTE byIndex, BYTE byObjectID, BYTE byAttributeID, BYTE byElementID, BYTE byDataValuePos, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData, BYTE byDataValueLength)
{
	//增加单个下发功能
	if(m_tDBManagement.m_byDBCreateTransaction == TRANSACTION)
	{
		if (byElementID < 1 || byElementID > 64)
		{
			return CreateSetReturnData(BAD_VALUE_NULL, byIndex, PHASE_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		}

		int  byPhaseIndex = -1;
		int  i = 0;
		for (i = 0;i < MAX_PHASE_COUNT;i++)
		{
			if (m_tVerifyParamInfo.m_stAscPhaseTable[i].m_byPhaseNumber == byElementID)
			{
				byPhaseIndex = i;
				break;
			}
		}

		if (byPhaseIndex == -1)
		{
			byPhaseIndex = byElementID - 1;
			m_tVerifyParamInfo.m_stAscPhaseTable[byPhaseIndex].m_byPhaseNumber = byElementID;
			//return CreateSetReturnData(BAD_VALUE_NULL, byIndex, PHASE_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		}
		if (byAttributeID == CONTROL_SHIELD)
		{
			if (m_chUnPackedBuff[byDataValuePos] == 0 || m_chUnPackedBuff[byDataValuePos] == 1)
			{
				m_tVerifyParamInfo.m_stAscPhaseTable[byPhaseIndex].m_byScreenFlag = m_chUnPackedBuff[byDataValuePos];
			}
			else 
			{
				return CreateSetReturnData(BAD_VALUE_OVERFLOW, byIndex, PHASE_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
			}
		}
		else if (byAttributeID == CONTROL_PROHIBIT)
		{
			if (m_chUnPackedBuff[byDataValuePos] == 0 || m_chUnPackedBuff[byDataValuePos] == 1)
			{
				m_tVerifyParamInfo.m_stAscPhaseTable[byPhaseIndex].m_byForbiddenFlag = m_chUnPackedBuff[byDataValuePos];	
			}
			else 
			{
				return CreateSetReturnData(BAD_VALUE_OVERFLOW, byIndex, PHASE_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
			}
		}
		CreateSetReturnData(0, byIndex, PHASE_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);

		m_bPhaseControlChange = true;
	}
	else
	{
		if (byElementID < 1 || byElementID > 64)
		{
			return CreateSetReturnData(BAD_VALUE_NULL, byIndex, PHASE_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		}

		int  byPhaseIndex = -1;
		int  i = 0;
		for (i = 0;i < MAX_PHASE_COUNT;i++)
		{
			if (m_tAscParamInfo.m_stAscPhaseTable[i].m_byPhaseNumber == byElementID)
			{
				byPhaseIndex = i;
				break;
			}
		}

		if (byPhaseIndex == -1)
		{
			byPhaseIndex = byElementID - 1;
			m_tAscParamInfo.m_stAscPhaseTable[byPhaseIndex].m_byPhaseNumber = byElementID;
			//return CreateSetReturnData(BAD_VALUE_NULL, byIndex, PHASE_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		}
		if (byAttributeID == CONTROL_SHIELD)
		{
			if (m_chUnPackedBuff[byDataValuePos] == 0 || m_chUnPackedBuff[byDataValuePos] == 1)
			{
				m_tAscParamInfo.m_stAscPhaseTable[byPhaseIndex].m_byScreenFlag = m_chUnPackedBuff[byDataValuePos];
			}
			else 
			{
				return CreateSetReturnData(BAD_VALUE_OVERFLOW, byIndex, PHASE_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
			}
		}
		else if (byAttributeID == CONTROL_PROHIBIT)
		{
			if (m_chUnPackedBuff[byDataValuePos] == 0 || m_chUnPackedBuff[byDataValuePos] == 1)
			{
				m_tAscParamInfo.m_stAscPhaseTable[byPhaseIndex].m_byForbiddenFlag = m_chUnPackedBuff[byDataValuePos];	
			}
			else 
			{
				return CreateSetReturnData(BAD_VALUE_OVERFLOW, byIndex, PHASE_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
			}
		}
		m_bIsNeedSave = true;
		m_bPhaseControlChange = true;
		CreateSetReturnData(0, byIndex, PHASE_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
	}
}

void  COpenATCCommWithGB20999Thread::SetDetectorInfo(BYTE byIndex, BYTE byObjectID, BYTE byAttributeID, BYTE byElementID, BYTE byDataValuePos, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData, BYTE byDataValueLength)
{
	if (byObjectID == DETECTOR_COUNT || byObjectID == DETECTOR_DATA_TABLE)
	{
		CreateSetReturnData(BAD_VALUE_READONLY, byIndex, DETECTOR_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
	}
	else if (byObjectID == DETECTOR_CONFIG_TABLE)
	{
		if (byAttributeID == DETECTOR_ID_CONFIG)
		{
			CreateSetReturnData(BAD_VALUE_READONLY, byIndex, DETECTOR_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		}
		else if (byAttributeID > INSTALL_POS)
		{
			CreateSetReturnData(BAD_VALUE_NULL, byIndex, DETECTOR_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		}
		else
		{
			SetDetectorConfigInfo(byIndex, byObjectID, byAttributeID, byElementID, byDataValuePos, tCorrectReturnData, tWrongReturnData, byDataValueLength);
		}
	}
	else 
	{
		CreateSetReturnData(BAD_VALUE_NULL, byIndex, DETECTOR_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
	}
}

void  COpenATCCommWithGB20999Thread::SetDetectorConfigInfo(BYTE byIndex, BYTE byObjectID, BYTE byAttributeID, BYTE byElementID, BYTE byDataValuePos, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData, BYTE byDataValueLength)
{
	if (byElementID < 1 || byElementID > 64)
	{
		return CreateSetReturnData(BAD_VALUE_NULL, byIndex, DETECTOR_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
	}

	int  nDetectorIndex = -1;
	int  i = 0;
	for (i = 0;i < MAX_VEHICLEDETECTOR_COUNT;i++)
	{
		if (m_tVerifyParamInfo.m_stAscVehicleDetectorTable[i].m_byVehicleDetectorNumber == byElementID)
		{
			nDetectorIndex = i;
			break;
		}
	}

	if (nDetectorIndex == -1)
	{
		nDetectorIndex = byElementID - 1;
		m_tVerifyParamInfo.m_stAscVehicleDetectorTable[nDetectorIndex].m_byVehicleDetectorNumber = byElementID;
		m_tVerifyParamInfo.m_stVehicleDetectorTableValidSize += 1;
	}
	
	if (byAttributeID == DETECTOR_TYPE)
	{
		if (m_chUnPackedBuff[byDataValuePos] > DETECTOR_TYPE_INFRARED)
		{
			return CreateSetReturnData(BAD_VALUE_OVERFLOW, byIndex, DETECTOR_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		}
		else
		{
			m_tVerifyParamInfo.m_stAscVehicleDetectorTable[nDetectorIndex].m_byDetectorType = m_chUnPackedBuff[byDataValuePos];
			//if (m_chUnPackedBuff[byDataValuePos] == DETECTOR_TYPE_COIL)
			//{
			//	m_tVerifyParamInfo.m_stAscVehicleDetectorTable[nDetectorIndex].DetectorType = m_chUnPackedBuff[byDataValuePos];		//2022.07.06
			//	m_tVerifyParamInfo.m_stAscVehicleDetectorTable[nDetectorIndex].m_byDetectorType = LOOP_DETECTOR;
			//}
			//else if (m_chUnPackedBuff[byDataValuePos] == DETECTOR_TYPE_VIDEO)
			//{
			//	m_tVerifyParamInfo.m_stAscVehicleDetectorTable[nDetectorIndex].DetectorType = m_chUnPackedBuff[byDataValuePos];		//2022.07.06
			//	m_tVerifyParamInfo.m_stAscVehicleDetectorTable[nDetectorIndex].m_byDetectorType = VIDEO_DETECTOR;
			//}
			//else
			//{
			//	m_tVerifyParamInfo.m_stAscVehicleDetectorTable[nDetectorIndex].DetectorType = m_chUnPackedBuff[byDataValuePos];		//2022.07.06
			//	m_tVerifyParamInfo.m_stAscVehicleDetectorTable[nDetectorIndex].m_byDetectorType = LOOP_DETECTOR;
			//}
		}
	}
	else if (byAttributeID == FLOW_CYCLE)
	{
		unsigned short nTemp = 0;
		memcpy(&nTemp, m_chUnPackedBuff + byDataValuePos, 2);
		nTemp = htons(nTemp);
		if (nTemp > 10)
		{
			m_tVerifyParamInfo.m_stAscVehicleDetectorTable[nDetectorIndex].m_wFlowGatherCycle = nTemp;
		}
		else
		{
            return CreateSetReturnData(BAD_VALUE_OVERFLOW, byIndex, DETECTOR_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		}
	}
	else if (byAttributeID == OCCUPY_CYCLE)
	{
		unsigned short nTemp = 0;
		memcpy(&nTemp, m_chUnPackedBuff + byDataValuePos, 2);
		nTemp = htons(nTemp);
		if (nTemp > 10)
		{
			m_tVerifyParamInfo.m_stAscVehicleDetectorTable[nDetectorIndex].m_wOccupancyGatherCycle = nTemp;
		}
		else
		{
            return CreateSetReturnData(BAD_VALUE_OVERFLOW, byIndex, DETECTOR_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		}
	}
	else if (byAttributeID == INSTALL_POS)
	{
		if (byDataValueLength == 132)
		{
			memcpy(&m_tVerifyParamInfo.m_stAscVehicleDetectorTable[nDetectorIndex].m_chFixPosition, m_chUnPackedBuff + byDataValuePos, sizeof(m_tVerifyParamInfo.m_stAscVehicleDetectorTable[nDetectorIndex].m_chFixPosition));
		}
		else
		{
			return CreateSetReturnData(BAD_VALUE_WRONGLENGTH, byIndex, DETECTOR_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		}
	}
	CreateSetReturnData(0, byIndex, DETECTOR_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
}

void  COpenATCCommWithGB20999Thread::SetPhaseStageInfo(BYTE byIndex, BYTE byObjectID, BYTE byAttributeID, BYTE byElementID, BYTE byDataValuePos, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData, BYTE byDataValueLength)
{
	if (byObjectID == PHASE_STAGE_COUNT)
	{
		CreateSetReturnData(BAD_VALUE_READONLY, byIndex, PHASESTAGE_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
	}
	else if (byObjectID == PHASE_STAGE_CONFIG_TABLE)
	{
		if (byAttributeID == PHASE_STAGE_ID_CONFIG)
		{
			CreateSetReturnData(BAD_VALUE_READONLY, byIndex, PHASESTAGE_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		}
		else if (byAttributeID > PHASE_STAGE_EARLY_END)
		{
			CreateSetReturnData(BAD_VALUE_NULL, byIndex, PHASESTAGE_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		}
		else
		{
			SetPhaseStageConfigInfo(byIndex, byObjectID, byAttributeID, byElementID, byDataValuePos, tCorrectReturnData, tWrongReturnData, byDataValueLength);
		}
	}
	else if (byObjectID == PHASE_STAGE_STATUS_TABLE)
	{
		CreateSetReturnData(BAD_VALUE_READONLY, byIndex, PHASESTAGE_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
	}
	else if (byObjectID == PHASE_STAGE_CONTROL_TABLE)
	{
		if (byAttributeID == PHASE_STAGE_ID_CONTROL)
		{
			CreateSetReturnData(BAD_VALUE_READONLY, byIndex, PHASESTAGE_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		}
		else if (byAttributeID > PHASE_STAGE_PROHIBIT)
		{
			CreateSetReturnData(BAD_VALUE_NULL, byIndex, PHASESTAGE_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		}
		else
		{
			SetPhaseStageControlInfo(byIndex, byObjectID, byAttributeID, byElementID, byDataValuePos, tCorrectReturnData, tWrongReturnData, byDataValueLength);
		}
	}
	else
	{
		CreateSetReturnData(BAD_VALUE_NULL, byIndex, PHASESTAGE_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
	}
}

void  COpenATCCommWithGB20999Thread::SetPhaseStageConfigInfo(BYTE byIndex, BYTE byObjectID, BYTE byAttributeID, BYTE byElementID, BYTE byDataValuePos, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData, BYTE byDataValueLength)
{
	if (byElementID < 1 || byElementID > 64)
	{
		return CreateSetReturnData(BAD_VALUE_NULL, byIndex, PHASESTAGE_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
	}
	if (byAttributeID == PHASE_STAGE_ID_CONFIG)
	{
		return CreateSetReturnData(BAD_VALUE_READONLY, byIndex, PHASESTAGE_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
	}
	//求配置的相位阶段数--HPH
	int  byPhaseStageIndex = -1;
	int  i = 0, j = 0;
	for (i = 0;i < MAX_PHASE_COUNT_20999;i++)
	{
		if (m_tVerifyParamInfo.m_stPhaseStage[i].m_byPhaseStageNumber == byElementID)
		{
			byPhaseStageIndex = i;
			break;
		}
	}

	if (byPhaseStageIndex == -1)
	{
		byPhaseStageIndex = byElementID - 1;
		m_tVerifyParamInfo.m_stPhaseStage[byPhaseStageIndex].m_byPhaseStageNumber = byElementID;
		m_tVerifyParamInfo.m_stPhaseStageValidSize += 1;	//相位阶段有效数字--HPH
	}

	if (byAttributeID == PHASE_STAGE_PHASE)
	{
		if (byDataValueLength == 12)
		{
			memcpy(m_tVerifyParamInfo.m_stPhaseStage[byElementID - 1].m_byPhase, m_chUnPackedBuff + byDataValuePos, 8);
			//GetRingInfoByPhase(m_tAscParamInfo.m_stPhaseStage[byElementID - 1].m_byPhase,byElementID);
		}
		else
		{
			return CreateSetReturnData(BAD_VALUE_WRONGLENGTH, byIndex, PHASESTAGE_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		}
	}
	else if (byAttributeID == PHASE_STAGE_LATE_START)
	{
		if (byDataValueLength == 68)
		{
			for (int iIndex = 0; iIndex<64; iIndex++)
			{
				m_tVerifyParamInfo.m_stPhaseStage[byElementID - 1].m_byLaterStartTime[64-iIndex-1] = m_chUnPackedBuff[byDataValuePos + iIndex];		//大小端，查询部分也需要修改
			}
		}
		else
		{
			return CreateSetReturnData(BAD_VALUE_WRONGLENGTH, byIndex, PHASESTAGE_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		}
	}
	else if (byAttributeID == PHASE_STAGE_EARLY_END)
	{
		if (byDataValueLength == 68)
		{
			for (int iIndex = 0; iIndex<64; iIndex++)
			{
				m_tVerifyParamInfo.m_stPhaseStage[byElementID - 1].m_byEarlyEndTime[64-iIndex-1] = m_chUnPackedBuff[byDataValuePos + iIndex];		//大小端，查询部分也需要修改
			}
		}
		else
		{
			return CreateSetReturnData(BAD_VALUE_WRONGLENGTH, byIndex, PHASESTAGE_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		}
	}
	CreateSetReturnData(0, byIndex, PHASESTAGE_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
}

void  COpenATCCommWithGB20999Thread::SetPhaseStageControlInfo(BYTE byIndex, BYTE byObjectID, BYTE byAttributeID, BYTE byElementID, BYTE byDataValuePos, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData, BYTE byDataValueLength)
{
	//需怎加单独配置功能 --2022.07.22
	if(m_tDBManagement.m_byDBCreateTransaction == TRANSACTION)
	{
		if (byElementID < 1 || byElementID > 64)
		{
			return CreateSetReturnData(BAD_VALUE_NULL, byIndex, PHASESTAGE_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		}

		if (byAttributeID == PHASE_STAGE_SOFTWARECALL)
		{
			if (m_chUnPackedBuff[byDataValuePos] == 0 || m_chUnPackedBuff[byDataValuePos] == 1)
			{
				m_tVerifyParamInfo.m_stPhaseStage[byElementID - 1].m_bSoftCall = m_chUnPackedBuff[byDataValuePos];
			}
			else
			{
				return CreateSetReturnData(BAD_VALUE_OVERFLOW, byIndex, PHASESTAGE_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
			}
		}
		else if (byAttributeID == PHASE_STAGE_SHIELD)
		{
			if (m_chUnPackedBuff[byDataValuePos] == 0 || m_chUnPackedBuff[byDataValuePos] == 1)
			{
				m_tVerifyParamInfo.m_stPhaseStage[byElementID - 1].m_bScreen = m_chUnPackedBuff[byDataValuePos];
			}
			else
			{
				return CreateSetReturnData(BAD_VALUE_OVERFLOW, byIndex, PHASESTAGE_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
			}
		}
		else if (byAttributeID == PHASE_STAGE_PROHIBIT)
		{
			if (m_chUnPackedBuff[byDataValuePos] == 0 || m_chUnPackedBuff[byDataValuePos] == 1)
			{
				m_tVerifyParamInfo.m_stPhaseStage[byElementID - 1].m_bForbidden = m_chUnPackedBuff[byDataValuePos];
			}
			else
			{
				return CreateSetReturnData(BAD_VALUE_OVERFLOW, byIndex, PHASESTAGE_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
			}
		}
		CreateSetReturnData(0, byIndex, PHASESTAGE_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
	}
	else
	{
		if (byElementID < 1 || byElementID > 64)
		{
			return CreateSetReturnData(BAD_VALUE_NULL, byIndex, PHASESTAGE_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		}

		if (byAttributeID == PHASE_STAGE_SOFTWARECALL)
		{
			if (m_chUnPackedBuff[byDataValuePos] == 0 || m_chUnPackedBuff[byDataValuePos] == 1)
			{
				m_tAscParamInfo.m_stPhaseStage[byElementID - 1].m_bSoftCall = m_chUnPackedBuff[byDataValuePos];
			}
			else
			{
				return CreateSetReturnData(BAD_VALUE_OVERFLOW, byIndex, PHASESTAGE_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
			}
		}
		else if (byAttributeID == PHASE_STAGE_SHIELD)
		{
			if (m_chUnPackedBuff[byDataValuePos] == 0 || m_chUnPackedBuff[byDataValuePos] == 1)
			{
				m_tAscParamInfo.m_stPhaseStage[byElementID - 1].m_bScreen = m_chUnPackedBuff[byDataValuePos];

				////方案转换
				//for (int nPatternSize = 0; nPatternSize < m_tAscParamInfo.m_stPatternTableValidSize; nPatternSize++)
				//{
				//	//方案阶段屏蔽与禁止
				//	for(int j = 0; j<16; j++)
				//	{
				//		if(m_tAscParamInfo.m_stAscPatternTable[nPatternSize].m_byPatternStage[j] == byElementID)
				//		{
				//			m_tAscParamInfo.m_stAscPatternTable[nPatternSize].m_byScreenStage[j] = m_tAscParamInfo.m_stPhaseStage[m_tAscParamInfo.m_stAscPatternTable[nPatternSize].m_byPatternStage[j] - 1].m_bScreen;			//屏蔽
				//		}
				//		//m_tAscParamInfo.m_stAscPatternTable[nPatternSize].m_byForbiddenStage[j] = m_tAscParamInfo.m_stPhaseStage[m_tAscParamInfo.m_stAscPatternTable[nPatternSize].m_byPatternStage[j] - 1].m_bForbidden;	//禁止
				//		//m_tAscParamInfo.m_stAscPatternTable[nPatternSize].m_byScreenStage[j] = m_tAscParamInfo.m_stPhaseStage[m_tAscParamInfo.m_stAscPatternTable[nPatternSize].m_byPatternStage[j] - 1].m_bScreen;			//屏蔽
				//	}
				//}
			}
			else
			{
				return CreateSetReturnData(BAD_VALUE_OVERFLOW, byIndex, PHASESTAGE_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
			}
		}
		else if (byAttributeID == PHASE_STAGE_PROHIBIT)
		{
			if (m_chUnPackedBuff[byDataValuePos] == 0 || m_chUnPackedBuff[byDataValuePos] == 1)
			{
				m_tAscParamInfo.m_stPhaseStage[byElementID - 1].m_bForbidden = m_chUnPackedBuff[byDataValuePos];

				////方案转换
				//for (int nPatternSize = 0; nPatternSize < m_tAscParamInfo.m_stPatternTableValidSize; nPatternSize++)
				//{
				//	//方案阶段屏蔽与禁止
				//	for(int j = 0; j<16; j++)
				//	{
				//		if(m_tAscParamInfo.m_stAscPatternTable[nPatternSize].m_byPatternStage[j] == byElementID)
				//		{
				//			m_tAscParamInfo.m_stAscPatternTable[nPatternSize].m_byForbiddenStage[j] = m_tAscParamInfo.m_stPhaseStage[m_tAscParamInfo.m_stAscPatternTable[nPatternSize].m_byPatternStage[j] - 1].m_bForbidden;			//屏蔽
				//		}
				//	}
				//}
			}
			else
			{
				return CreateSetReturnData(BAD_VALUE_OVERFLOW, byIndex, PHASESTAGE_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
			}
		}

		m_bIsNeedSave = true;
		CreateSetReturnData(0, byIndex, PHASESTAGE_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
	}
}

void  COpenATCCommWithGB20999Thread::SetPhaseSafetyInfo(BYTE byIndex, BYTE byObjectID, BYTE byAttributeID, BYTE byElementID, BYTE byDataValuePos, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData, BYTE byDataValueLength)
{
	if (byObjectID == PHASE_CONFLICT_TABLE)
	{
		if (byAttributeID == PHASE_ID_CONFLICT)
		{
			CreateSetReturnData(BAD_VALUE_READONLY, byIndex, PHASESAFETY_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		}
		else if (byAttributeID > PHASE_CONFLICT_ARRAY)
		{
			CreateSetReturnData(BAD_VALUE_OVERFLOW, byIndex, PHASESAFETY_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		}
		else
		{
			SetPhaseConflictInfo(byIndex, byObjectID, byAttributeID, byElementID, byDataValuePos, tCorrectReturnData, tWrongReturnData, byDataValueLength);
		}
	}
	else if (byObjectID == PHASE_GREENINTERVAL_TABLE)
	{
		if (byAttributeID == PHASE_ID_GREENINTERVAL)
		{
			CreateSetReturnData(BAD_VALUE_READONLY, byIndex, PHASESAFETY_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		}
		else if (byAttributeID > PHASE_GREENINTERVAL_ARRAY)
		{
			CreateSetReturnData(BAD_VALUE_OVERFLOW, byIndex, PHASESAFETY_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		}
		else
		{
			SetPhaseGreenIntervalInfo(byIndex, byObjectID, byAttributeID, byElementID, byDataValuePos, tCorrectReturnData, tWrongReturnData, byDataValueLength);
		}
	}
	else
	{
		CreateSetReturnData(BAD_VALUE_NULL, byIndex, PHASESAFETY_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
	}
}

void  COpenATCCommWithGB20999Thread::SetPhaseConflictInfo(BYTE byIndex, BYTE byObjectID, BYTE byAttributeID, BYTE byElementID, BYTE byDataValuePos, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData, BYTE byDataValueLength)
{
	if (byElementID < 1 || byElementID > 64)
	{
		return CreateSetReturnData(BAD_VALUE_NULL, byIndex, PHASESAFETY_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
	}

	int  byPhaseIndex = -1;
	int  i = 0;
	for (i = 0;i < MAX_PHASE_COUNT_20999;i++)
	{
		if (m_tVerifyParamInfo.m_stPhaseConflictInfo[i].m_byPhaseNumber == byElementID)
		{
			byPhaseIndex = i;
			break;
		}
	}

	if (byPhaseIndex == -1)
	{
		byPhaseIndex = byElementID - 1;
		m_tVerifyParamInfo.m_stPhaseConflictInfo[byPhaseIndex].m_byPhaseNumber = byElementID;
		//return CreateSetReturnData(BAD_VALUE_NULL, byIndex, PHASE_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
	}
	
	if (byAttributeID == PHASE_CONFLICT_ARRAY)
	{
		//memset(m_tVerifyParamInfo.m_stAscPhaseTable[byElementID-1].m_byPhaseConcurrency,0,sizeof(m_tVerifyParamInfo.m_stAscPhaseTable[byPhaseIndex].m_byPhaseConcurrency));	//初始化并发相位表--HPH
		//int	tCount = 0;
		//BYTE test[8];
		//memset(test,0,8);
		//for (int i = 0; i < m_tVerifyParamInfo.PhaseTableValidSize; i++)
		//{
		//	if (memcmp(m_tVerifyParamInfo.m_stAscPhaseTable[i].m_byLightGroup,test,8) != 0)
		//	{
		//		printf("%d\t",i);
		//		tCount++;
		//	}
		//}

		//if (byDataValueLength == 68)
		if (byDataValueLength == 12)	//应该8+4--HPH
		{
			//int count = 0;
			for (i = 0;i < C_N_MAX_PHASE_CONFLICT;i++)
			{
				//int nConcurrency = 1;
				//int	nIndex = 0;
				m_tVerifyParamInfo.m_stPhaseConflictInfo[byElementID - 1].m_byConflictSequenceInfo[i] = m_chUnPackedBuff[byDataValuePos + i];
				//for (int j=0;j<8;j++)
				//{
				//	nIndex = (7-i)*8+j;
				//	if (((m_tVerifyParamInfo.m_stPhaseConflictInfo[byElementID - 1].m_byConflictSequenceInfo[i]&nConcurrency) != nConcurrency) &&(nIndex < tCount))		//2022.07.06
				//	{
				//		m_tVerifyParamInfo.m_stAscPhaseTable[byElementID-1].m_byPhaseConcurrency[count] =nIndex + 1;
				//		count++;
				//	}
				//	nConcurrency = nConcurrency<<1;
				//}
			}		//通过相位冲突表获取相位并发表--HPH
		    CreateSetReturnData(0, byIndex, PHASESAFETY_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		}
		else
		{
			CreateSetReturnData(BAD_VALUE_WRONGLENGTH, byIndex, PHASESAFETY_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		}
	}
}

void  COpenATCCommWithGB20999Thread::SetPhaseGreenIntervalInfo(BYTE byIndex, BYTE byObjectID, BYTE byAttributeID, BYTE byElementID, BYTE byDataValuePos, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData, BYTE byDataValueLength)
{
	if (byElementID < 1 || byElementID > C_N_MAX_PHASE_GREENINTERVAL)
	{
		return CreateSetReturnData(BAD_VALUE_NULL, byIndex, PHASESAFETY_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
	}

	int  byPhaseIndex = -1;
	int  i = 0;
	for (i = 0;i < MAX_PHASE_COUNT_20999;i++)
	{
		if (m_tVerifyParamInfo.m_stPhaseGreenGapInfo[i].m_byPhaseNumber == byElementID)
		{
			byPhaseIndex = i;
		}
	}

	if (byPhaseIndex == -1)
	{
		byPhaseIndex = byElementID - 1;
		m_tVerifyParamInfo.m_stPhaseGreenGapInfo[byPhaseIndex].m_byPhaseNumber = byElementID;
		//return CreateSetReturnData(BAD_VALUE_NULL, byIndex, PHASE_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
	}
	
	if (byAttributeID == PHASE_GREENINTERVAL_ARRAY)
	{
		if (byDataValueLength == 68)
		{
			for (i = 0;i < C_N_MAX_PHASE_GREENINTERVAL;i++)
			{
				m_tVerifyParamInfo.m_stPhaseGreenGapInfo[byPhaseIndex].m_byGreenGapSequenceInfo[i] = m_chUnPackedBuff[byDataValuePos + i];
				//绿间隔间隔序列配置做一个相位过渡时间的校验。（过渡灯色1+过渡灯色2+过渡灯色3<=该相位的绿间隔） 2021.12.22
				//if (m_chUnPackedBuff[byDataValuePos + i] > 0 && (m_chUnPackedBuff[byDataValuePos + i] > (m_tVerifyParamInfo.m_stAscPhaseTable[byPhaseIndex].m_nLoseControlLampOneTime + m_tVerifyParamInfo.m_stAscPhaseTable[byPhaseIndex].m_nLoseControlLampTwoTime + m_tVerifyParamInfo.m_stAscPhaseTable[byPhaseIndex].m_nLoseControlLampThreeTime)*10))
				//{
				//	return CreateSetReturnData(BAD_VALUE_STATUS, byIndex, PHASESAFETY_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
				//}
				//绿间隔间隔序列配置做一个相位过渡时间的校验。（过渡灯色1+过渡灯色2+过渡灯色3<=该相位的绿间隔） 2021.12.22
			}
		}
		else
		{
			return CreateSetReturnData(BAD_VALUE_WRONGLENGTH, byIndex, PHASESAFETY_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		}
	}
	CreateSetReturnData(0, byIndex, PHASESAFETY_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
}

void  COpenATCCommWithGB20999Thread::SetEmergencyAndPriorityInfo(BYTE byIndex, BYTE byObjectID, BYTE byAttributeID, BYTE byElementID, BYTE byDataValuePos, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData, BYTE byDataValueLength)
{
	if (byObjectID == PRIORITY_COUNT || byObjectID == PRIORITY_STATUS_TABLE || byObjectID == EMERGENCY_COUNT || byObjectID == EMERGENCY_STATUS_TABLE)
	{
		CreateSetReturnData(BAD_VALUE_READONLY, byIndex, EMERGENCY_PRIORITY, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
	}
	else if (byObjectID == PRIORITY_CONFIG_TABLE)
	{
		if (byAttributeID == PRIORITY_ID_CONFIG)
		{
			CreateSetReturnData(BAD_VALUE_READONLY, byIndex, EMERGENCY_PRIORITY, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		}
		else if (byAttributeID > PRIORITY_SOURCE)
		{
			CreateSetReturnData(BAD_VALUE_NULL, byIndex, EMERGENCY_PRIORITY, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		}
		else
		{
			SetPriorityConfigInfo(byIndex, byObjectID, byAttributeID, byElementID, byDataValuePos, tCorrectReturnData, tWrongReturnData, byDataValueLength);
		}
	}
	else if (byObjectID == EMERGENCY_CONFIG_TABLE)
	{
		if (byAttributeID == EMERGENCY_ID_CONFIG)
		{
			CreateSetReturnData(BAD_VALUE_READONLY, byIndex, EMERGENCY_PRIORITY, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		}
		else if (byAttributeID > EMERGENCY_SOURCE)
		{
			CreateSetReturnData(BAD_VALUE_NULL, byIndex, EMERGENCY_PRIORITY, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		}
		else
		{
			SetEmergencyConfigInfo(byIndex, byObjectID, byAttributeID, byElementID, byDataValuePos, tCorrectReturnData, tWrongReturnData, byDataValueLength);
		}
	}
	else
	{
		CreateSetReturnData(BAD_VALUE_NULL, byIndex, EMERGENCY_PRIORITY, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
	}
}

void  COpenATCCommWithGB20999Thread::SetPriorityConfigInfo(BYTE byIndex, BYTE byObjectID, BYTE byAttributeID, BYTE byElementID, BYTE byDataValuePos, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData, BYTE byDataValueLength)
{
	//if(m_tDBManagement.m_byDBCreateTransaction == TRANSACTION)
	//{
	//	if (byElementID < 1 || byElementID > C_N_MAX_PRIORITY_COUNT)
	//	{
	//		return CreateSetReturnData(BAD_VALUE_NULL, byIndex, EMERGENCY_PRIORITY, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
	//	}

	//	int  byPriorityIndex = -1;
	//	int  i = 0;
	//	for (i = 0;i < C_N_MAX_PRIORITY_COUNT;i++)
	//	{
	//		if (m_tVerifyParamInfo.m_stPriorityInfo[i].m_byPrioritySignalNumber == byElementID)
	//		{
	//			byPriorityIndex = i;
	//			break;
	//		}
	//	}

	//	if (byPriorityIndex == -1)
	//	{
	//		byPriorityIndex = byElementID - 1;
	//		m_tVerifyParamInfo.m_stPriorityInfo[byPriorityIndex].m_byPrioritySignalNumber = byElementID;
	//		m_tVerifyParamInfo.m_stPriorityValidSize += 1;
	//	}
	//
	//	if (byAttributeID == PRIORITY_APPLY_PHASESTAGE)
	//	{
	//		if (m_chUnPackedBuff[byDataValuePos] >= 1 && m_chUnPackedBuff[byDataValuePos] <= 64)
	//		{
	//			m_tVerifyParamInfo.m_stPriorityInfo[byPriorityIndex].m_byPrioritySignalPhaseStage = m_chUnPackedBuff[byDataValuePos];

	//			//m_tVerifyParamInfo.m_stPriorityInfo[byPriorityIndex].m_bPrioritySignalStatus = 1;//申请状态
	//			//m_tVerifyParamInfo.m_stPriorityInfo[byPriorityIndex].m_bPrioritySignalPerformStatus = 0;//执行状态
	//		}
	//		else
	//		{
	//			return CreateSetReturnData(BAD_VALUE_OVERFLOW, byIndex, EMERGENCY_PRIORITY, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
	//		}
	//	}
	//	else if (byAttributeID == PRIORITY_APPLY_GRADE)
	//	{
	//		m_tVerifyParamInfo.m_stPriorityInfo[byPriorityIndex].m_byPrioritySignalGrade = m_chUnPackedBuff[byDataValuePos];
	//	}
	//	else if (byAttributeID == PRIORITY_SHIELD)
	//	{
	//		if (m_chUnPackedBuff[byDataValuePos] == 0 || m_chUnPackedBuff[byDataValuePos] == 1)
	//		{
	//			m_tVerifyParamInfo.m_stPriorityInfo[byPriorityIndex].m_blPrioritySignalScreen = m_chUnPackedBuff[byDataValuePos];
	//		}
	//		else
	//		{
	//			return CreateSetReturnData(BAD_VALUE_OVERFLOW, byIndex, EMERGENCY_PRIORITY, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
	//		}
	//	}
	//	else if (byAttributeID == PRIORITY_SOURCE)
	//	{
	//		m_tVerifyParamInfo.m_stPriorityInfo[byPriorityIndex].m_byPrioritySignalSource = m_chUnPackedBuff[byDataValuePos];
	//	}

	//	CreateSetReturnData(0, byIndex, EMERGENCY_PRIORITY, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
	//}
	//else
	//{
	//	if (byElementID < 1 || byElementID > C_N_MAX_PRIORITY_COUNT)
	//	{
	//		return CreateSetReturnData(BAD_VALUE_NULL, byIndex, EMERGENCY_PRIORITY, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
	//	}

	//	int  byPriorityIndex = -1;
	//	int  i = 0;
	//	for (i = 0;i < C_N_MAX_PRIORITY_COUNT;i++)
	//	{
	//		if (m_tAscParamInfo.m_stPriorityInfo[i].m_byPrioritySignalNumber == byElementID)
	//		{
	//			byPriorityIndex = i;
	//			break;
	//		}
	//	}

	//	if (byPriorityIndex == -1)
	//	{
	//		byPriorityIndex = byElementID - 1;
	//		m_tAscParamInfo.m_stPriorityInfo[byPriorityIndex].m_byPrioritySignalNumber = byElementID;
	//		m_tAscParamInfo.m_stPriorityValidSize += 1;
	//	}
	//
	//	if (byAttributeID == PRIORITY_APPLY_PHASESTAGE)		//优先信号申请相位阶段
	//	{
	//		if (m_chUnPackedBuff[byDataValuePos] >= 1 && m_chUnPackedBuff[byDataValuePos] <= 64)
	//		{
	//			m_tAscParamInfo.m_stPriorityInfo[byPriorityIndex].m_byPrioritySignalPhaseStage = m_chUnPackedBuff[byDataValuePos];

	//			//m_tVerifyParamInfo.m_stPriorityInfo[byPriorityIndex].m_bPrioritySignalStatus = 1;//申请状态
	//			//m_tVerifyParamInfo.m_stPriorityInfo[byPriorityIndex].m_bPrioritySignalPerformStatus = 0;//执行状态
	//		}
	//		else
	//		{
	//			return CreateSetReturnData(BAD_VALUE_OVERFLOW, byIndex, EMERGENCY_PRIORITY, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
	//		}
	//	}
	//	else if (byAttributeID == PRIORITY_APPLY_GRADE)		//优先信号申请优先级
	//	{
	//		m_tAscParamInfo.m_stPriorityInfo[byPriorityIndex].m_byPrioritySignalGrade = m_chUnPackedBuff[byDataValuePos];
	//	}
	//	else if (byAttributeID == PRIORITY_SHIELD)	//优先信号屏蔽
	//	{
	//		if (m_chUnPackedBuff[byDataValuePos] == 0 || m_chUnPackedBuff[byDataValuePos] == 1)
	//		{
	//			m_tAscParamInfo.m_stPriorityInfo[byPriorityIndex].m_blPrioritySignalScreen = m_chUnPackedBuff[byDataValuePos];
	//		}
	//		else
	//		{
	//			return CreateSetReturnData(BAD_VALUE_OVERFLOW, byIndex, EMERGENCY_PRIORITY, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
	//		}
	//	}
	//	else if (byAttributeID == PRIORITY_SOURCE)		//优先信号来源【检测器】
	//	{
	//		m_tAscParamInfo.m_stPriorityInfo[byPriorityIndex].m_byPrioritySignalSource = m_chUnPackedBuff[byDataValuePos];
	//	}
	//	m_bIsNeedSave = true;
	//	CreateSetReturnData(0, byIndex, EMERGENCY_PRIORITY, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
	//}
	
	//紧急优先通过事务机配置
	//紧急优先可通过事务机配置，也可以通过直接的修改

	if (m_tDBManagement.m_byDBCreateTransaction == TRANSACTION)
	{
		if (byElementID < 1 || byElementID > C_N_MAX_PRIORITY_COUNT)
		{
			return CreateSetReturnData(BAD_VALUE_NULL, byIndex, EMERGENCY_PRIORITY, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		}

		int  byPriorityIndex = -1;
		int  i = 0;
		for (i = 0; i < C_N_MAX_PRIORITY_COUNT; i++)
		{
			if (m_tVerifyParamInfo.m_stPriorityInfo[i].m_byPrioritySignalNumber == byElementID)
			{
				byPriorityIndex = i;
				break;
			}
		}

		if (byPriorityIndex == -1)
		{
			byPriorityIndex = byElementID - 1;
			m_tVerifyParamInfo.m_stPriorityInfo[byPriorityIndex].m_byPrioritySignalNumber = byElementID;
			m_tVerifyParamInfo.m_stPriorityValidSize += 1;
		}

		if (byAttributeID == PRIORITY_APPLY_PHASESTAGE)
		{
			if (m_chUnPackedBuff[byDataValuePos] >= 1 && m_chUnPackedBuff[byDataValuePos] <= 64)
			{
				m_tVerifyParamInfo.m_stPriorityInfo[byPriorityIndex].m_byPrioritySignalPhaseStage = m_chUnPackedBuff[byDataValuePos];

				//m_tVerifyParamInfo.m_stPriorityInfo[byPriorityIndex].m_bPrioritySignalStatus = 1;//申请状态
				//m_tVerifyParamInfo.m_stPriorityInfo[byPriorityIndex].m_bPrioritySignalPerformStatus = 0;//执行状态
			}
			else
			{
				return CreateSetReturnData(BAD_VALUE_OVERFLOW, byIndex, EMERGENCY_PRIORITY, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
			}
		}
		else if (byAttributeID == PRIORITY_APPLY_GRADE)
		{
			m_tVerifyParamInfo.m_stPriorityInfo[byPriorityIndex].m_byPrioritySignalGrade = m_chUnPackedBuff[byDataValuePos];
		}
		else if (byAttributeID == PRIORITY_SHIELD)
		{
			if (m_chUnPackedBuff[byDataValuePos] == 0 || m_chUnPackedBuff[byDataValuePos] == 1)
			{
				m_tVerifyParamInfo.m_stPriorityInfo[byPriorityIndex].m_blPrioritySignalScreen = m_chUnPackedBuff[byDataValuePos];
			}
			else
			{
				return CreateSetReturnData(BAD_VALUE_OVERFLOW, byIndex, EMERGENCY_PRIORITY, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
			}
		}
		else if (byAttributeID == PRIORITY_SOURCE)
		{
			m_tVerifyParamInfo.m_stPriorityInfo[byPriorityIndex].m_byPrioritySignalSource = m_chUnPackedBuff[byDataValuePos];
		}

		CreateSetReturnData(0, byIndex, EMERGENCY_PRIORITY, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
	}
	else
	{
		if (byElementID < 1 || byElementID > C_N_MAX_PRIORITY_COUNT)
		{
			return CreateSetReturnData(BAD_VALUE_NULL, byIndex, EMERGENCY_PRIORITY, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		}

		int  byPriorityIndex = -1;
		int  i = 0;
		for (i = 0; i < C_N_MAX_PRIORITY_COUNT; i++)
		{
			if (m_tAscParamInfo.m_stPriorityInfo[i].m_byPrioritySignalNumber == byElementID)
			{
				byPriorityIndex = i;
				break;
			}
		}

		if (byPriorityIndex == -1)
		{
			byPriorityIndex = byElementID - 1;
			m_tAscParamInfo.m_stPriorityInfo[byPriorityIndex].m_byPrioritySignalNumber = byElementID;
			m_tAscParamInfo.m_stPriorityValidSize += 1;
		}

		if (byAttributeID == PRIORITY_APPLY_PHASESTAGE)
		{
			if (m_chUnPackedBuff[byDataValuePos] >= 1 && m_chUnPackedBuff[byDataValuePos] <= 64)
			{
				m_tAscParamInfo.m_stPriorityInfo[byPriorityIndex].m_byPrioritySignalPhaseStage = m_chUnPackedBuff[byDataValuePos];

				//m_tVerifyParamInfo.m_stPriorityInfo[byPriorityIndex].m_bPrioritySignalStatus = 1;//申请状态
				//m_tVerifyParamInfo.m_stPriorityInfo[byPriorityIndex].m_bPrioritySignalPerformStatus = 0;//执行状态
			}
			else
			{
				return CreateSetReturnData(BAD_VALUE_OVERFLOW, byIndex, EMERGENCY_PRIORITY, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
			}
		}
		else if (byAttributeID == PRIORITY_APPLY_GRADE)
		{
			m_tAscParamInfo.m_stPriorityInfo[byPriorityIndex].m_byPrioritySignalGrade = m_chUnPackedBuff[byDataValuePos];
		}
		else if (byAttributeID == PRIORITY_SHIELD)
		{
			if (m_chUnPackedBuff[byDataValuePos] == 0 || m_chUnPackedBuff[byDataValuePos] == 1)
			{
				m_tAscParamInfo.m_stPriorityInfo[byPriorityIndex].m_blPrioritySignalScreen = m_chUnPackedBuff[byDataValuePos];
			}
			else
			{
				return CreateSetReturnData(BAD_VALUE_OVERFLOW, byIndex, EMERGENCY_PRIORITY, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
			}
		}
		else if (byAttributeID == PRIORITY_SOURCE)
		{
			m_tAscParamInfo.m_stPriorityInfo[byPriorityIndex].m_byPrioritySignalSource = m_chUnPackedBuff[byDataValuePos];
		}

		m_bIsNeedSave = true;
		CreateSetReturnData(0, byIndex, EMERGENCY_PRIORITY, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
	}
	
}

void  COpenATCCommWithGB20999Thread::SetEmergencyConfigInfo(BYTE byIndex, BYTE byObjectID, BYTE byAttributeID, BYTE byElementID, BYTE byDataValuePos, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData, BYTE byDataValueLength)
{
	//if(m_tDBManagement.m_byDBCreateTransaction == TRANSACTION)
	//{
	//	if (byElementID < 1 || byElementID > C_N_MAX_EMERGENCY_COUNT)
	//	{
	//		return CreateSetReturnData(BAD_VALUE_NULL, byIndex, EMERGENCY_PRIORITY, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
	//	}

	//	int  byEmergencyIndex = -1;
	//	int  i = 0;
	//	for (i = 0;i < C_N_MAX_EMERGENCY_COUNT;i++)
	//	{
	//		if ( m_tVerifyParamInfo.m_stEmergyInfo[i].m_byEmergySignalID == byElementID)
	//		{
	//			byEmergencyIndex = i;
	//			break;
	//		}
	//	}

	//	if (byEmergencyIndex == -1)
	//	{
	//		byEmergencyIndex = byElementID - 1;
	//		m_tVerifyParamInfo.m_stEmergyInfo[byEmergencyIndex].m_byEmergySignalID = byElementID;
	//		m_tVerifyParamInfo.EmergyValidSize += 1;
	//	}
	//
	//	if (byAttributeID == EMERGENCY_APPLY_PHASESTAGE)
	//	{
	//		if (m_chUnPackedBuff[byDataValuePos] >= 1 && m_chUnPackedBuff[byDataValuePos] <= 64)
	//		{
	//			m_tVerifyParamInfo.m_stEmergyInfo[byEmergencyIndex].m_byEmergySignalPhaseStage = m_chUnPackedBuff[byDataValuePos];

	//			//m_tVerifyParamInfo.m_stEmergyInfo[byEmergencyIndex].m_bEmergySignalStatus = 1;//申请状态
	//			//m_tVerifyParamInfo.m_stEmergyInfo[byEmergencyIndex].m_bEmergySignalPerformStatus = 0;//执行状态
	//		}
	//		else
	//		{
	//			return CreateSetReturnData(BAD_VALUE_OVERFLOW, byIndex, EMERGENCY_PRIORITY, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
	//		}
	//	}
	//	else if (byAttributeID == EMERGENCY_APPLY_GRADE)
	//	{
	//		m_tVerifyParamInfo.m_stEmergyInfo[byEmergencyIndex].m_byEmergySignalGrade = m_chUnPackedBuff[byDataValuePos];
	//	}	
	//	else if (byAttributeID == EMERGENCY_SHIELD)
	//	{
	//		if (m_chUnPackedBuff[byDataValuePos] == 0 || m_chUnPackedBuff[byDataValuePos] == 1)
	//		{
	//			m_tVerifyParamInfo.m_stEmergyInfo[byEmergencyIndex].m_bEmergySignalScreen = m_chUnPackedBuff[byDataValuePos];
	//		}
	//		else
	//		{
	//			return CreateSetReturnData(BAD_VALUE_OVERFLOW, byIndex, EMERGENCY_PRIORITY, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
	//		}
	//	}
	//	else if (byAttributeID == EMERGENCY_SOURCE)
	//	{
	//		m_tVerifyParamInfo.m_stEmergyInfo[byEmergencyIndex].m_byEmergySignalSource = m_chUnPackedBuff[byDataValuePos];
	//	}

	//	CreateSetReturnData(0, byIndex, EMERGENCY_PRIORITY, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
	//}
	//else
	//{
	//	if (byElementID < 1 || byElementID > C_N_MAX_EMERGENCY_COUNT)
	//	{
	//		return CreateSetReturnData(BAD_VALUE_NULL, byIndex, EMERGENCY_PRIORITY, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
	//	}

	//	int  byEmergencyIndex = -1;
	//	int  i = 0;
	//	for (i = 0;i < C_N_MAX_EMERGENCY_COUNT;i++)
	//	{
	//		if ( m_tAscParamInfo.m_stEmergyInfo[i].m_byEmergySignalID == byElementID)
	//		{
	//			byEmergencyIndex = i;
	//			break;
	//		}
	//	}

	//	if (byEmergencyIndex == -1)
	//	{
	//		byEmergencyIndex = byElementID - 1;
	//		m_tAscParamInfo.m_stEmergyInfo[byEmergencyIndex].m_byEmergySignalID = byElementID;
	//		m_tAscParamInfo.m_stEmergyValidSize += 1;
	//	}
	//
	//	if (byAttributeID == EMERGENCY_APPLY_PHASESTAGE)
	//	{
	//		if (m_chUnPackedBuff[byDataValuePos] >= 1 && m_chUnPackedBuff[byDataValuePos] <= 64)
	//		{
	//			m_tAscParamInfo.m_stEmergyInfo[byEmergencyIndex].m_byEmergySignalPhaseStage = m_chUnPackedBuff[byDataValuePos];

	//			//m_tAscParamInfo.m_stEmergyInfo[byEmergencyIndex].m_bEmergySignalStatus = 1;//申请状态
	//			//m_tAscParamInfo.m_stEmergyInfo[byEmergencyIndex].m_bEmergySignalPerformStatus = 0;//执行状态
	//		}
	//		else
	//		{
	//			return CreateSetReturnData(BAD_VALUE_OVERFLOW, byIndex, EMERGENCY_PRIORITY, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
	//		}
	//	}
	//	else if (byAttributeID == EMERGENCY_APPLY_GRADE)
	//	{
	//		m_tAscParamInfo.m_stEmergyInfo[byEmergencyIndex].m_byEmergySignalGrade = m_chUnPackedBuff[byDataValuePos];
	//	}	
	//	else if (byAttributeID == EMERGENCY_SHIELD)
	//	{
	//		if (m_chUnPackedBuff[byDataValuePos] == 0 || m_chUnPackedBuff[byDataValuePos] == 1)
	//		{
	//			m_tAscParamInfo.m_stEmergyInfo[byEmergencyIndex].m_bEmergySignalScreen = m_chUnPackedBuff[byDataValuePos];
	//		}
	//		else
	//		{
	//			return CreateSetReturnData(BAD_VALUE_OVERFLOW, byIndex, EMERGENCY_PRIORITY, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
	//		}
	//	}
	//	else if (byAttributeID == EMERGENCY_SOURCE)
	//	{
	//		m_tAscParamInfo.m_stEmergyInfo[byEmergencyIndex].m_byEmergySignalSource = m_chUnPackedBuff[byDataValuePos];
	//	}
	//	m_bIsNeedSave = true;
	//	CreateSetReturnData(0, byIndex, EMERGENCY_PRIORITY, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
	//}

	if (m_tDBManagement.m_byDBCreateTransaction == TRANSACTION)
	{
		if (byElementID < 1 || byElementID > C_N_MAX_EMERGENCY_COUNT)
		{
			return CreateSetReturnData(BAD_VALUE_NULL, byIndex, EMERGENCY_PRIORITY, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		}

		int  byEmergencyIndex = -1;
		int  i = 0;
		for (i = 0; i < C_N_MAX_EMERGENCY_COUNT; i++)
		{
			if (m_tVerifyParamInfo.m_stEmergyInfo[i].m_byEmergySignalID == byElementID)
			{
				byEmergencyIndex = i;
				break;
			}
		}

		if (byEmergencyIndex == -1)
		{
			byEmergencyIndex = byElementID - 1;
			m_tVerifyParamInfo.m_stEmergyInfo[byEmergencyIndex].m_byEmergySignalID = byElementID;
			m_tVerifyParamInfo.m_stEmergyValidSize += 1;
		}

		if (byAttributeID == EMERGENCY_APPLY_PHASESTAGE)
		{
			if (m_chUnPackedBuff[byDataValuePos] >= 1 && m_chUnPackedBuff[byDataValuePos] <= 64)
			{
				m_tVerifyParamInfo.m_stEmergyInfo[byEmergencyIndex].m_byEmergySignalPhaseStage = m_chUnPackedBuff[byDataValuePos];

				//m_tVerifyParamInfo.m_stEmergyInfo[byEmergencyIndex].m_bEmergySignalStatus = 1;//申请状态
				//m_tVerifyParamInfo.m_stEmergyInfo[byEmergencyIndex].m_bEmergySignalPerformStatus = 0;//执行状态
			}
			else
			{
				return CreateSetReturnData(BAD_VALUE_OVERFLOW, byIndex, EMERGENCY_PRIORITY, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
			}
		}
		else if (byAttributeID == EMERGENCY_APPLY_GRADE)
		{
			m_tVerifyParamInfo.m_stEmergyInfo[byEmergencyIndex].m_byEmergySignalGrade = m_chUnPackedBuff[byDataValuePos];
		}
		else if (byAttributeID == EMERGENCY_SHIELD)
		{
			if (m_chUnPackedBuff[byDataValuePos] == 0 || m_chUnPackedBuff[byDataValuePos] == 1)
			{
				m_tVerifyParamInfo.m_stEmergyInfo[byEmergencyIndex].m_bEmergySignalScreen = m_chUnPackedBuff[byDataValuePos];
			}
			else
			{
				return CreateSetReturnData(BAD_VALUE_OVERFLOW, byIndex, EMERGENCY_PRIORITY, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
			}
		}
		else if (byAttributeID == EMERGENCY_SOURCE)
		{
			m_tVerifyParamInfo.m_stEmergyInfo[byEmergencyIndex].m_byEmergySignalSource = m_chUnPackedBuff[byDataValuePos];
		}

		CreateSetReturnData(0, byIndex, EMERGENCY_PRIORITY, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
	}
	else
	{
		if (byElementID < 1 || byElementID > C_N_MAX_EMERGENCY_COUNT)
		{
			return CreateSetReturnData(BAD_VALUE_NULL, byIndex, EMERGENCY_PRIORITY, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		}

		int  byEmergencyIndex = -1;
		int  i = 0;
		for (i = 0; i < C_N_MAX_EMERGENCY_COUNT; i++)
		{
			if (m_tAscParamInfo.m_stEmergyInfo[i].m_byEmergySignalID == byElementID)
			{
				byEmergencyIndex = i;
				break;
			}
		}

		if (byEmergencyIndex == -1)
		{
			byEmergencyIndex = byElementID - 1;
			m_tAscParamInfo.m_stEmergyInfo[byEmergencyIndex].m_byEmergySignalID = byElementID;
			m_tAscParamInfo.m_stEmergyValidSize += 1;
		}

		if (byAttributeID == EMERGENCY_APPLY_PHASESTAGE)
		{
			if (m_chUnPackedBuff[byDataValuePos] >= 1 && m_chUnPackedBuff[byDataValuePos] <= 64)
			{
				m_tAscParamInfo.m_stEmergyInfo[byEmergencyIndex].m_byEmergySignalPhaseStage = m_chUnPackedBuff[byDataValuePos];

				//m_tVerifyParamInfo.m_stEmergyInfo[byEmergencyIndex].m_bEmergySignalStatus = 1;//申请状态
				//m_tVerifyParamInfo.m_stEmergyInfo[byEmergencyIndex].m_bEmergySignalPerformStatus = 0;//执行状态
			}
			else
			{
				return CreateSetReturnData(BAD_VALUE_OVERFLOW, byIndex, EMERGENCY_PRIORITY, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
			}
		}
		else if (byAttributeID == EMERGENCY_APPLY_GRADE)
		{
			m_tAscParamInfo.m_stEmergyInfo[byEmergencyIndex].m_byEmergySignalGrade = m_chUnPackedBuff[byDataValuePos];
		}
		else if (byAttributeID == EMERGENCY_SHIELD)
		{
			if (m_chUnPackedBuff[byDataValuePos] == 0 || m_chUnPackedBuff[byDataValuePos] == 1)
			{
				m_tAscParamInfo.m_stEmergyInfo[byEmergencyIndex].m_bEmergySignalScreen = m_chUnPackedBuff[byDataValuePos];
			}
			else
			{
				return CreateSetReturnData(BAD_VALUE_OVERFLOW, byIndex, EMERGENCY_PRIORITY, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
			}
		}
		else if (byAttributeID == EMERGENCY_SOURCE)
		{
			m_tAscParamInfo.m_stEmergyInfo[byEmergencyIndex].m_byEmergySignalSource = m_chUnPackedBuff[byDataValuePos];
		}

		m_bIsNeedSave = true;
		CreateSetReturnData(0, byIndex, EMERGENCY_PRIORITY, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
	}
}

void  COpenATCCommWithGB20999Thread::SetPatternInfo(BYTE byIndex, BYTE byObjectID, BYTE byAttributeID, BYTE byElementID, BYTE byDataValuePos, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData, BYTE byDataValueLength)
{
	if (byObjectID == PATTERN_COUNT)
	{
		CreateSetReturnData(BAD_VALUE_READONLY, byIndex, PATTERN_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
	}
	else if (byObjectID == PATTERN_CONFIGTABLE)
	{
		if (byAttributeID == PATTERN_ID)
		{
			CreateSetReturnData(BAD_VALUE_READONLY, byIndex, PATTERN_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		}
		else if (byAttributeID > PATTERN_STAGE_TYPE)
		{
			CreateSetReturnData(BAD_VALUE_NULL, byIndex, PATTERN_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		}
		else
		{
			SetPatternConfigInfo(byIndex, byObjectID, byAttributeID, byElementID, byDataValuePos, tCorrectReturnData, tWrongReturnData, byDataValueLength);
		}
	}
	else
	{
		CreateSetReturnData(BAD_VALUE_READONLY, byIndex, PATTERN_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
	}
}

void  COpenATCCommWithGB20999Thread::SetPatternConfigInfo(BYTE byIndex, BYTE byObjectID, BYTE byAttributeID, BYTE byElementID, BYTE byDataValuePos, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData, BYTE byDataValueLength)
{
	if (byElementID < 1 || byElementID > MAX_PATTERN_COUNT)
	{
		return CreateSetReturnData(BAD_VALUE_NULL, byIndex, PATTERN_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
	}

	int byPatternIndex = -1;

	int  i = 0, j = 0, k = 0;
	for (i = 0;i < MAX_PATTERN_COUNT;i++)
	{
		if (m_tVerifyParamInfo.m_stAscPatternTable[i].m_byPatternNumber == byElementID)
		{
			byPatternIndex = i;
			break;
		}
	}

	if (byPatternIndex == -1)
	{
		byPatternIndex = byElementID - 1;
		m_tVerifyParamInfo.m_stAscPatternTable[byPatternIndex].m_byPatternNumber = byElementID;
		m_tVerifyParamInfo.m_stPatternTableValidSize += 1;//方案表有效数--HPH
	}

	if (byAttributeID == PATTERN_ROADID)
	{
		if (m_chUnPackedBuff[byDataValuePos] >=1 && m_chUnPackedBuff[byDataValuePos] <= 8)
		{
			m_tVerifyParamInfo.m_stAscPatternTable[byPatternIndex].m_byRoadID = m_chUnPackedBuff[byDataValuePos];
		}
		else
		{
			return CreateSetReturnData(BAD_VALUE_OVERFLOW, byIndex, PATTERN_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		}
	}
	else if (byAttributeID == PATTERN_CYCLELEN)
	{
		unsigned int nTemp = 0;
		memcpy(&nTemp, m_chUnPackedBuff + byDataValuePos, 4);
		m_tVerifyParamInfo.m_stAscPatternTable[byPatternIndex].m_wPatternCycleTime = htonl(nTemp);
		//printf("Cycletime:%d\n",m_tVerifyParamInfo.m_stAscPatternTable[byPatternIndex].m_wPatternCycleTime);
	}
	else if (byAttributeID == PATTERN_ADJUST_STAGEID)
	{
		if (m_chUnPackedBuff[byDataValuePos] >= 1 && m_chUnPackedBuff[byDataValuePos] <= 16)
		{
			m_tVerifyParamInfo.m_stAscPatternTable[byPatternIndex].m_byCoorditionStage = m_chUnPackedBuff[byDataValuePos];
		}
		else
		{
			return CreateSetReturnData(BAD_VALUE_OVERFLOW, byIndex, PATTERN_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		}
	}
	else if (byAttributeID == PATTERN_OFFSET)
	{
		unsigned short nTemp = 0;
		memcpy(&nTemp, m_chUnPackedBuff + byDataValuePos, 2);
		m_tVerifyParamInfo.m_stAscPatternTable[byPatternIndex].m_byPatternOffsetTime = htons(nTemp);
	}
	else if (byAttributeID == PATTERN_STAGE_CHAIN)
	{
		if (byDataValueLength == 20)
		{
			for (i = 0;i < 16;i++)
			{
				m_tVerifyParamInfo.m_stAscPatternTable[byPatternIndex].m_byPatternStage[16 - i - 1] = m_chUnPackedBuff[byDataValuePos + i];			//大端2022.07.07
			}
		}
		else
		{
			return CreateSetReturnData(BAD_VALUE_WRONGLENGTH, byIndex, PATTERN_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		}
		
	}
	else if (byAttributeID == PATTERN_STAGE_CHAINTIME)
	{
		if (byDataValueLength == 36)
		{
			for (i = 0;i < 32;i++)
			{
				m_tVerifyParamInfo.m_stAscPatternTable[byPatternIndex].m_byPatternStageTime[32 - i - 1] = m_chUnPackedBuff[byDataValuePos + i];		//大端2022.07.07
			}
		}
		else
		{
			return CreateSetReturnData(BAD_VALUE_WRONGLENGTH, byIndex, PATTERN_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		}
	}
	else if (byAttributeID == PATTERN_STAGE_TYPE)
	{
		if (byDataValueLength == 20)
		{
			for (i = 0;i < 16;i++)
			{
				m_tVerifyParamInfo.m_stAscPatternTable[byPatternIndex].m_byPatternStageOccurType[16 - i - 1] = m_chUnPackedBuff[byDataValuePos + i];		//大端2022.07.07
			}
		}
		else
		{
			return CreateSetReturnData(BAD_VALUE_WRONGLENGTH, byIndex, PATTERN_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		}
	}

	CreateSetReturnData(0, byIndex, PATTERN_INFO, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
}

void  COpenATCCommWithGB20999Thread::SetTransitionRetain(BYTE byIndex, BYTE byObjectID, BYTE byAttributeID, BYTE byElementID, BYTE byDataValuePos, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData, BYTE byDataValueLength)
{
	//添加如果非事物机可以单个配置
	if(m_tDBManagement.m_byDBCreateTransaction == TRANSACTION)
	{
		if (byObjectID != 1)//对象为1表示过渡约束配置表
		{
			return CreateSetReturnData(BAD_VALUE_NULL, byIndex, TRANSITION_RETRAIN, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		}
		else
		{
			if (byAttributeID == PHASE_TRANSITIONSTAGE_ID)
			{
				return CreateSetReturnData(BAD_VALUE_READONLY, byIndex, TRANSITION_RETRAIN, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
			}
		}

		if (byElementID < 1 || byElementID > MAX_PHASE_COUNT_20999)
		{
			return CreateSetReturnData(BAD_VALUE_OVERFLOW, byIndex, TRANSITION_RETRAIN, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		}

		int  byRunStageIndex = -1;
		int  i = 0;
		for (i = 0;i < MAX_PHASE_COUNT_20999;i++)
		{
			if (m_tVerifyParamInfo.m_stTransitBound[i].m_byPhaseStageNumber == byElementID)
			{
				byRunStageIndex = i;
				break;
			}
		}

		if (byRunStageIndex == -1 && byElementID <= m_tAscParamInfo.m_stPhaseTableValidSize)
		{
			byRunStageIndex = byElementID - 1;
			m_tVerifyParamInfo.m_stTransitBound[byRunStageIndex].m_byPhaseStageNumber = byElementID;
		}/*
		else
		{
			return CreateSetReturnData(BAD_VALUE_OVERFLOW, byIndex, TRANSITION_RETRAIN, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		}*/

		if (byAttributeID == PHASE_TRANSITIONSTAGE_RETAIN)
		{
			if (byDataValueLength == 68)
			{
				for (i = 0;i < C_N_MAX_TRANSIT_RETAIN;i++)
				{
					m_tVerifyParamInfo.m_stTransitBound[byRunStageIndex].m_byTransitBound[63-i] = m_chUnPackedBuff[byDataValuePos + i];	//高字节在前，低字节在后--HPH
				}
			}
			else
			{
				return CreateSetReturnData(BAD_VALUE_WRONGLENGTH, byIndex, TRANSITION_RETRAIN, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
			}
		}
		CreateSetReturnData(0, byIndex, TRANSITION_RETRAIN, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
	}
	else
	{
		if (byObjectID != 1)//对象为1表示过渡约束配置表
		{
			return CreateSetReturnData(BAD_VALUE_NULL, byIndex, TRANSITION_RETRAIN, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		}
		else
		{
			if (byAttributeID == PHASE_TRANSITIONSTAGE_ID)
			{
				return CreateSetReturnData(BAD_VALUE_READONLY, byIndex, TRANSITION_RETRAIN, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
			}
		}

		if (byElementID < 1 || byElementID > MAX_PHASE_COUNT_20999)
		{
			return CreateSetReturnData(BAD_VALUE_OVERFLOW, byIndex, TRANSITION_RETRAIN, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		}

		int  byRunStageIndex = -1;
		int  i = 0;
		for (i = 0;i < MAX_PHASE_COUNT_20999;i++)
		{
			if (m_tAscParamInfo.m_stTransitBound[i].m_byPhaseStageNumber == byElementID)
			{
				byRunStageIndex = i;
				break;
			}
		}

		if (byRunStageIndex == -1 && byElementID <= m_tAscParamInfo.m_stPhaseTableValidSize)
		{
			byRunStageIndex = byElementID - 1;
			m_tAscParamInfo.m_stTransitBound[byRunStageIndex].m_byPhaseStageNumber = byElementID;
		}/*
		else
		{
			return CreateSetReturnData(BAD_VALUE_OVERFLOW, byIndex, TRANSITION_RETRAIN, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		}*/

		if (byAttributeID == PHASE_TRANSITIONSTAGE_RETAIN)
		{
			if (byDataValueLength == 68)
			{
				for (i = 0;i < C_N_MAX_TRANSIT_RETAIN;i++)
				{
					m_tAscParamInfo.m_stTransitBound[byRunStageIndex].m_byTransitBound[63-i] = m_chUnPackedBuff[byDataValuePos + i];	//高字节在前，低字节在后--HPH
				}
			}
			else
			{
				return CreateSetReturnData(BAD_VALUE_WRONGLENGTH, byIndex, TRANSITION_RETRAIN, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
			}
		}
		m_bIsNeedSave = true;
		CreateSetReturnData(0, byIndex, TRANSITION_RETRAIN, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
	}

	//if (byObjectID != 1)//对象为1表示过渡约束配置表
	//{
	//	return CreateSetReturnData(BAD_VALUE_NULL, byIndex, TRANSITION_RETRAIN, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
	//}
	//else
	//{
	//	if (byAttributeID == PHASE_TRANSITIONSTAGE_ID)
	//	{
	//		return CreateSetReturnData(BAD_VALUE_READONLY, byIndex, TRANSITION_RETRAIN, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
	//	}
	//}

	//if (byElementID < 1 || byElementID > MAX_PHASE_COUNT_20999)
	//{
	//	return CreateSetReturnData(BAD_VALUE_OVERFLOW, byIndex, TRANSITION_RETRAIN, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
	//}

	//int  byRunStageIndex = -1;
	//int  i = 0;
	//for (i = 0;i < MAX_PHASE_COUNT_20999;i++)
	//{
	//	if (m_tAscParamInfo.m_stTransitBound[i].m_byPhaseStageNumber == byElementID)
	//	{
	//		byRunStageIndex = i;
	//	    break;
	//	}
	//}

	//if (byRunStageIndex == -1 && byElementID <= m_tAscParamInfo.m_stPhaseTableValidSize)
	//{
	//	byRunStageIndex = byElementID - 1;
	//	m_tAscParamInfo.m_stTransitBound[byRunStageIndex].m_byPhaseStageNumber = byElementID;
	//}
	//else
	//{
	//	return CreateSetReturnData(BAD_VALUE_OVERFLOW, byIndex, TRANSITION_RETRAIN, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
	//}

	//if (byAttributeID == PHASE_TRANSITIONSTAGE_RETAIN)
	//{
	//	if (byDataValueLength == 68)
	//	{
	//		for (i = 0;i < C_N_MAX_TRANSIT_RETAIN;i++)
	//		{
	//			m_tAscParamInfo.m_stTransitBound[byRunStageIndex].m_byTransitBound[63-i] = m_chUnPackedBuff[byDataValuePos + i];	//高字节在前，低字节在后--HPH
	//		}
	//	}
	//	else
	//	{
	//		return CreateSetReturnData(BAD_VALUE_WRONGLENGTH, byIndex, TRANSITION_RETRAIN, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
	//	}
	//}
	//m_bIsNeedSave = true;
	//CreateSetReturnData(0, byIndex, TRANSITION_RETRAIN, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
}

void  COpenATCCommWithGB20999Thread::SetDayPlanInfo(BYTE byIndex, BYTE byObjectID, BYTE byAttributeID, BYTE byElementID, BYTE byDataValuePos, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData, BYTE byDataValueLength)
{
	if (byObjectID == DAYPLAN_COUNT)
	{
		CreateSetReturnData(BAD_VALUE_READONLY, byIndex, DAY_PLAN, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
	}
	else if (byObjectID == DAYPLAN_CONFIG)
	{
		if (byAttributeID == DAYPLAN_ID)
		{
			CreateSetReturnData(BAD_VALUE_READONLY, byIndex, DAY_PLAN, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		}
		else if (byAttributeID > DAYPLAN_TIMESPANACTCHAIN8)
		{
			CreateSetReturnData(BAD_VALUE_NULL, byIndex, DAY_PLAN, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		}
		else
		{
			SetDayPlanConfigInfo(byIndex, byObjectID, byAttributeID, byElementID, byDataValuePos, tCorrectReturnData, tWrongReturnData, byDataValueLength);
		}
	}
	else 
	{
		CreateSetReturnData(BAD_VALUE_NULL, byIndex, DAY_PLAN, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
	}
}

void  COpenATCCommWithGB20999Thread::SetDayPlanConfigInfo(BYTE byIndex, BYTE byObjectID, BYTE byAttributeID, BYTE byElementID, BYTE byDataValuePos, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData, BYTE byDataValueLength)
{
	if (byElementID < 1 || byElementID > 128)
	{
		return CreateSetReturnData(BAD_VALUE_NULL, byIndex, DAY_PLAN, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
	}
	
	//日计划编号和日计划有效值--HPH
	int  byDayPlanIndex = -1;
	int  i = 0, j = 0;
	for (int n = 0;n < MAX_DAY_PLAN_SIZE;n++)
	{
		if (m_tVerifyParamInfo.m_stDayPlanInfo[n].m_byDayPlanID == byElementID)
		{
			byDayPlanIndex = n;
			break;
		}
	}

	if (byDayPlanIndex == -1)
	{
		byDayPlanIndex = byElementID - 1;
		m_tVerifyParamInfo.m_stDayPlanInfo[byDayPlanIndex].m_byDayPlanID = byElementID;
		m_tVerifyParamInfo.m_stDayPlanValidSize += 1;
	}

	if (byAttributeID == DAYPLAN_ROADID)
	{
		if (m_chUnPackedBuff[C_N_DATAVALUE_POS] >= 1 && m_chUnPackedBuff[C_N_DATAVALUE_POS] <= 8)
		{
			m_tVerifyParamInfo.m_stDayPlanInfo[byElementID - 1].m_byRoadID = m_chUnPackedBuff[C_N_DATAVALUE_POS];
		}
		else
		{
			return CreateSetReturnData(BAD_VALUE_OVERFLOW, byIndex, DAY_PLAN, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		}
	}
	else if (byAttributeID == DAYPLAN_STARTTIMECHAIN)
	{
		if (byDataValueLength == 100)
		{
			for (i = 0;i < C_N_MAX_TIMECHAIN_COUNT;i++)
			{
				m_tVerifyParamInfo.m_stDayPlanInfo[byElementID - 1].m_byDayPlanStartTime[C_N_MAX_TIMECHAIN_COUNT - i - 1] = m_chUnPackedBuff[byDataValuePos + i];
			}
		}
		else
		{
			return CreateSetReturnData(BAD_VALUE_OVERFLOW, byIndex, DAY_PLAN, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		}
	}
	else if (byAttributeID == DAYPLAN_PATTERNCHAIN)
	{
		if (byDataValueLength == 52)
		{
			//memset(m_tVerifyParamInfo.m_stDayPlanInfo[byElementID - 1].m_byDayPlanActionPattern, 0x00, C_N_MAX_PATTERNCHAIN_COUNT);
			for (i = 0;i < C_N_MAX_PATTERNCHAIN_COUNT;i++)
			{
				m_tVerifyParamInfo.m_stDayPlanInfo[byElementID - 1].m_byDayPlanActionPattern[C_N_MAX_PATTERNCHAIN_COUNT - i - 1] = m_chUnPackedBuff[byDataValuePos + i];
			}
		}
		else
		{
			return CreateSetReturnData(BAD_VALUE_OVERFLOW, byIndex, DAY_PLAN, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		}
			
	}
	else if (byAttributeID == DAYPLAN_RUNMODECHAIN)
	{
		if (byDataValueLength == 52)
		{
			for (i = 0;i < C_N_MAX_PATTERNCHAIN_COUNT;i++)
			{
				m_tVerifyParamInfo.m_stDayPlanInfo[byElementID - 1].m_byDayPlanRunMode[C_N_MAX_PATTERNCHAIN_COUNT - i - 1] = m_chUnPackedBuff[byDataValuePos + i];
			}
		}
		else
		{
			return CreateSetReturnData(BAD_VALUE_OVERFLOW, byIndex, DAY_PLAN, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		}
	}
	else if (byAttributeID == DAYPLAN_TIMESPANACTCHAIN1)
	{
		if (byDataValueLength == 100)
		{
			for (i = 0;i < C_N_MAX_ACTCHAIN_COUNT;i++)
			{
				m_tVerifyParamInfo.m_stDayPlanInfo[byElementID - 1].m_byActionChainOne[i] = m_chUnPackedBuff[byDataValuePos + i];
			}
		}
		else
		{
			return CreateSetReturnData(BAD_VALUE_OVERFLOW, byIndex, DAY_PLAN, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		}
			
	}
	else if (byAttributeID == DAYPLAN_TIMESPANACTCHAIN2)
	{
		if (byDataValueLength == 100)
		{
			for (i = 0;i < C_N_MAX_ACTCHAIN_COUNT;i++)
			{
				m_tVerifyParamInfo.m_stDayPlanInfo[byElementID - 1].m_byActionChainTwo[i] = m_chUnPackedBuff[byDataValuePos + i];
			}
		}
		else
		{
			return CreateSetReturnData(BAD_VALUE_OVERFLOW, byIndex, DAY_PLAN, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		}
	}
	else if (byAttributeID == DAYPLAN_TIMESPANACTCHAIN3)
	{
		if (byDataValueLength == 100)
		{
			for (i = 0;i < C_N_MAX_ACTCHAIN_COUNT;i++)
			{
				m_tVerifyParamInfo.m_stDayPlanInfo[byElementID - 1].m_byActionChainThree[i] = m_chUnPackedBuff[byDataValuePos + i];
			}
		}
		else
		{
			return CreateSetReturnData(BAD_VALUE_OVERFLOW, byIndex, DAY_PLAN, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		}
	}
	else if (byAttributeID == DAYPLAN_TIMESPANACTCHAIN4)
	{
		if (byDataValueLength == 100)
		{
			for (i = 0;i < C_N_MAX_ACTCHAIN_COUNT;i++)
			{
				m_tVerifyParamInfo.m_stDayPlanInfo[byElementID - 1].m_byActionChainFour[i] = m_chUnPackedBuff[byDataValuePos + i];
			}
		}
		else
		{
			return CreateSetReturnData(BAD_VALUE_OVERFLOW, byIndex, DAY_PLAN, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		}
	}
    else if (byAttributeID == DAYPLAN_TIMESPANACTCHAIN5)
	{
		if (byDataValueLength == 100)
		{
			for (i = 0;i < C_N_MAX_ACTCHAIN_COUNT;i++)
			{
				m_tVerifyParamInfo.m_stDayPlanInfo[byElementID - 1].m_byActionChainFive[i] = m_chUnPackedBuff[byDataValuePos + i];
			}
		}
		else
		{
			return CreateSetReturnData(BAD_VALUE_OVERFLOW, byIndex, DAY_PLAN, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		}
	}
	else if (byAttributeID == DAYPLAN_TIMESPANACTCHAIN6)
	{
		if (byDataValueLength == 100)
		{
			for (i = 0;i < C_N_MAX_ACTCHAIN_COUNT;i++)
			{
				m_tVerifyParamInfo.m_stDayPlanInfo[byElementID - 1].m_byActionChainSix[i] = m_chUnPackedBuff[byDataValuePos + i];
			}
		}
		else
		{
			return CreateSetReturnData(BAD_VALUE_OVERFLOW, byIndex, DAY_PLAN, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		}
	}
	else if (byAttributeID == DAYPLAN_TIMESPANACTCHAIN7)
	{
		if (byDataValueLength == 100)
		{
			for (i = 0;i < C_N_MAX_ACTCHAIN_COUNT;i++)
			{
				m_tVerifyParamInfo.m_stDayPlanInfo[byElementID - 1].m_byActionChainSeven[i] = m_chUnPackedBuff[byDataValuePos + i];
			}
		}
		else
		{
			return CreateSetReturnData(BAD_VALUE_OVERFLOW, byIndex, DAY_PLAN, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		}
	}
	else if (byAttributeID == DAYPLAN_TIMESPANACTCHAIN8)
	{
		if (byDataValueLength == 100)
		{
			for (i = 0;i < C_N_MAX_ACTCHAIN_COUNT;i++)
			{
				m_tVerifyParamInfo.m_stDayPlanInfo[byElementID - 1].m_byActionChainEight[i] = m_chUnPackedBuff[byDataValuePos + i];
			}
		}
		else
		{
			return CreateSetReturnData(BAD_VALUE_OVERFLOW, byIndex, DAY_PLAN, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		}
	}
	CreateSetReturnData(0, byIndex, DAY_PLAN, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
}

void  COpenATCCommWithGB20999Thread::SetScheduleTable(BYTE byIndex, BYTE byObjectID, BYTE byAttributeID, BYTE byElementID, BYTE byDataValuePos, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData, BYTE byDataValueLength)
{
	if (byObjectID == SCHEDULE_COUNT)
	{
		CreateSetReturnData(BAD_VALUE_READONLY, byIndex, SCHEDULE_TABLE, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
	}
	else if (byObjectID == SCHEDULE_CONFIG_TABLE)
	{
		if (byAttributeID == SCHEDULE_ID)
		{
			CreateSetReturnData(BAD_VALUE_READONLY, byIndex, SCHEDULE_TABLE, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		}
		else if (byAttributeID > SCHEDULE_DAYPLAN_ID)
		{
			CreateSetReturnData(BAD_VALUE_NULL, byIndex, SCHEDULE_TABLE, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		}
		else
		{
			SetScheduleTableConfig(byIndex, byObjectID, byAttributeID, byElementID, byDataValuePos, tCorrectReturnData, tWrongReturnData, byDataValueLength);
		}
	}
	else if (byObjectID == SCHEDULE_COUNT)
	{
		CreateSetReturnData(BAD_VALUE_READONLY, byIndex, SCHEDULE_TABLE, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
	}
	else
	{
		CreateSetReturnData(BAD_VALUE_NULL, byIndex, SCHEDULE_TABLE, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
	}
}

void  COpenATCCommWithGB20999Thread::SetScheduleTableConfig(BYTE byIndex, BYTE byObjectID, BYTE byAttributeID, BYTE byElementID, BYTE byDataValuePos, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData, BYTE byDataValueLength)
{
	if (byElementID < 1 || byElementID > 128)
	{
		return CreateSetReturnData(BAD_VALUE_NULL, byIndex, SCHEDULE_TABLE, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
	}
	int  nScheduleIndex = -1;
	int  i = 0;
	for (i = 0;i < 128;i++)
	{
		//if (m_tVerifyParamInfo.m_stAscScheduleTable[i].m_wTimeBaseScheduleNumber == byElementID)
		if (m_tVerifyParamInfo.m_stSchedulePlanInfo[i].m_SchedulePlanID == byElementID)	//2021.11.02--HPH
		{
			nScheduleIndex = i;
			break;
		}
	}

	if (nScheduleIndex == -1)
	{
		nScheduleIndex = byElementID - 1;
		//m_tVerifyParamInfo.m_stAscScheduleTable[nScheduleIndex].m_wTimeBaseScheduleNumber = byElementID;
		m_tVerifyParamInfo.m_stSchedulePlanInfo[nScheduleIndex].m_SchedulePlanID = byElementID;
		m_tVerifyParamInfo.m_stSchedulePlanValidSize += 1; //调度表有效值--HPH
		//CreateSetReturnData(BAD_VALUE_NULL, byIndex, SCHEDULE_TABLE, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
	}
	
	if (byAttributeID == SCHEDULE_ROADID)
	{ 
		if (m_chUnPackedBuff[byDataValuePos] >= 1 && m_chUnPackedBuff[byDataValuePos] <= 8)
		{
			m_tVerifyParamInfo.m_stSchedulePlanInfo[nScheduleIndex].m_byRoadID = m_chUnPackedBuff[byDataValuePos];
		}
		else
		{
			return CreateSetReturnData(BAD_VALUE_OVERFLOW, byIndex, SCHEDULE_TABLE, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		}
	}
	else if (byAttributeID == SCHEDULE_PRIORITY)
	{
		m_tVerifyParamInfo.m_stSchedulePlanInfo[nScheduleIndex].m_byPriority = m_chUnPackedBuff[byDataValuePos];
	}
	else if (byAttributeID == SCHEDULE_WEEK)
	{
		m_tVerifyParamInfo.m_stSchedulePlanInfo[nScheduleIndex].m_byWeek = m_chUnPackedBuff[byDataValuePos];
	}
	else if (byAttributeID == SCHEDULE_MONTH)
	{
		unsigned short nTemp = 0;
		memcpy(&nTemp, m_chUnPackedBuff + byDataValuePos, 2);
		m_tVerifyParamInfo.m_stSchedulePlanInfo[nScheduleIndex].m_byMonth = htons(nTemp);
	}
	else if (byAttributeID == SCHEDULE_DAY)
	{
		unsigned long nTemp = 0;
		memcpy(&nTemp, m_chUnPackedBuff + byDataValuePos, 4);
		m_tVerifyParamInfo.m_stSchedulePlanInfo[nScheduleIndex].m_byDate = htonl(nTemp);
	}
	else if (byAttributeID == SCHEDULE_DAYPLAN_ID)
	{
		if (m_chUnPackedBuff[byDataValuePos] >= 1 && m_chUnPackedBuff[byDataValuePos] <= 128)
		{
			m_tVerifyParamInfo.m_stSchedulePlanInfo[nScheduleIndex].m_byScheduleOfDayPlanID = m_chUnPackedBuff[byDataValuePos];
		}
		else
		{
			return CreateSetReturnData(BAD_VALUE_OVERFLOW, byIndex, SCHEDULE_TABLE, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		}
	}
	CreateSetReturnData(0, byIndex, SCHEDULE_TABLE, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
}

void  COpenATCCommWithGB20999Thread::SetRunStatusInfo(BYTE byIndex, BYTE byObjectID, BYTE byAttributeID, BYTE byElementID, BYTE byDataValuePos, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData, BYTE byDataValueLength)
{
	if (byObjectID == DEVICE_STATUS)
	{
		if (byAttributeID == STANDARD_TIME)
		{
			SetATCStandardTime(byDataValuePos);
			CreateSetReturnData(0, byIndex, RUN_STATUS, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		}
		else if (byAttributeID == LOCAL_TIME)
		{
			SetATCLocalTime(byDataValuePos);
			CreateSetReturnData(0, byIndex, RUN_STATUS, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		}
		else
		{
			CreateSetReturnData(BAD_VALUE_READONLY, byIndex, RUN_STATUS, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		}
	}
	else if (byObjectID == CONTROL_STATUS)
	{
		CreateSetReturnData(BAD_VALUE_READONLY, byIndex, RUN_STATUS, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
	}
	else
	{
		CreateSetReturnData(BAD_VALUE_NULL, byIndex, RUN_STATUS, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
	}
}

void  COpenATCCommWithGB20999Thread::SetATCStandardTime(BYTE byDataValuePos)
{
#ifndef _WIN32
	time_t timep;
	struct timeval tv;
	struct timezone tz;
	BYTE byTime_l[7];
	memset(byTime_l,0,7);
	memcpy(byTime_l, m_chUnPackedBuff + byDataValuePos, 7);
	short nYear = 0;
	memcpy(&nYear, byTime_l, 2);//获取年
	nYear = htons(nYear);
	Utc_tm.tm_year	= nYear - 1900;
	Utc_tm.tm_mon	= byTime_l[2] - 1;
	Utc_tm.tm_mday	= byTime_l[3];
	Utc_tm.tm_hour	= byTime_l[4];
	Utc_tm.tm_min	= byTime_l[5];
	Utc_tm.tm_sec	= byTime_l[6];
	Utc_tm.tm_isdst	= -1;
	timep = mktime(&Utc_tm);

	printf("gmtoff after mktime: %ld\n", Utc_tm.tm_gmtoff);		//mktime所得秒数是utc时间-时区
    if (Utc_tm.tm_gmtoff != 0)
    {
        timep += Utc_tm.tm_gmtoff;
    }

	tv.tv_sec = timep;
	tv.tv_usec = 0;
	tz.tz_minuteswest = 0;
	tz.tz_dsttime = 0;
	settimeofday(&tv,&tz);
#else
	BYTE byTime[7];
	memset(byTime, 0, 7);
	memcpy(byTime, m_chUnPackedBuff + byDataValuePos, 7);

	short nYear = 0;
	memcpy(&nYear, byTime, 2);//获取年
	nYear = htons(nYear);

	m_stUTCTime.wYear		  = nYear;
	m_stUTCTime.wMonth		  = byTime[2];
	m_stUTCTime.wDay		  = byTime[3];
	m_stUTCTime.wHour		  = byTime[4];
	m_stUTCTime.wMinute		  = byTime[5];
	m_stUTCTime.wSecond		  = byTime[6];
	m_stUTCTime.wMilliseconds = 0;

	SystemTimeToTzSpecificLocalTime(NULL, &m_stUTCTime, &m_stLocalTime);

	SetLocalTime(&m_stLocalTime);
#endif
}

void  COpenATCCommWithGB20999Thread::SetATCLocalTime(BYTE byDataValuePos)
{
#ifndef _WIN32
	time_t timep;
	BYTE byTime_l[7];
	memset(byTime_l,0,7);
	memcpy(byTime_l, m_chUnPackedBuff + byDataValuePos, 7);
	short nYear = 0;
	memcpy(&nYear, byTime_l, 2);//获取年
	nYear = htons(nYear);
	Local_tm.tm_year	= nYear - 1900;
	Local_tm.tm_mon		= byTime_l[2] - 1;
	Local_tm.tm_mday	= byTime_l[3];
	Local_tm.tm_hour	= byTime_l[4];
	Local_tm.tm_min		= byTime_l[5];
	Local_tm.tm_sec		= byTime_l[6];
	Local_tm.tm_isdst	= -1;
	timep = mktime(&Local_tm);
	stime(&timep);
#else
	BYTE byTime[7];
	memset(byTime, 0, 7);
	memcpy(byTime, m_chUnPackedBuff + byDataValuePos, 7);

	short nYear = 0;
	memcpy(&nYear, byTime, 2);//获取年
	nYear = htons(nYear);

	//m_stLocalTime.wYear			= nYear + 1900;
	m_stLocalTime.wYear			= nYear;
	m_stLocalTime.wMonth		= byTime[2];
	m_stLocalTime.wDay			= byTime[3];
	m_stLocalTime.wHour			= byTime[4];
	m_stLocalTime.wMinute		= byTime[5];
	m_stLocalTime.wSecond		= byTime[6];
	m_stLocalTime.wMilliseconds	= 0;

	SetLocalTime(&m_stLocalTime);

	TzSpecificLocalTimeToSystemTime(NULL, &m_stLocalTime, &m_stUTCTime);
#endif
}

void  COpenATCCommWithGB20999Thread::SetCenterControl(BYTE byIndex, BYTE byObjectID, BYTE byAttributeID, BYTE byElementID, BYTE byDataValuePos, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData, BYTE byDataValueLength)
{
	if (byObjectID == 1)//中心控制表
	{
		if (byAttributeID == CENTERCONTROL_ROADID)
		{
			CreateSetReturnData(BAD_VALUE_READONLY, byIndex, CENTER_CONTROL, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		}
		else if (byAttributeID > CENTERCONTROL_RUNMODE)
		{
			CreateSetReturnData(BAD_VALUE_NULL, byIndex, CENTER_CONTROL, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		}
		else
		{
			//中心手动命令仍然需要修改
			//SetCenterControlConfig(byIndex, byObjectID, byAttributeID, byElementID, byDataValuePos, tCorrectReturnData, tWrongReturnData, byDataValueLength);
		}
	}
	else
	{
		CreateSetReturnData(BAD_VALUE_NULL, byIndex, CENTER_CONTROL, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
	}
}

//void  COpenATCCommWithGB20999Thread::SetCenterControlConfig(BYTE byIndex, BYTE byObjectID, BYTE byAttributeID, BYTE byElementID, BYTE byDataValuePos, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData, BYTE byDataValueLength)
//{
//	TManualCmd tManualCmd;
//	memset(&tManualCmd,0,sizeof(tManualCmd));
//
//	//用于路口2的手动命令
//	TCenterCtrlInfo tCenterCtrlInfo;
//	memset(&tCenterCtrlInfo,0,sizeof(tCenterCtrlInfo));
//
//	TAscStepCfg tStepCfg;
//	memset(&tStepCfg, 0, sizeof(tStepCfg));
//	m_pOpenATCParameter->GetStepInfo(tStepCfg);
//
//	m_pOpenATCParameter->GetPhaseStageCount(m_tAscParamInfo.m_stPhaseStageValidSize);	//需获取相应的阶段数，否则会一直为0 HPH--2021.11.18
//
//	m_pOpenATCRunStatus->SetPhaseRunStatusWriteFlag(true);
//	OpenATCSleep(100);
//	bool bIsReadFlag = true;
//	if (m_pOpenATCRunStatus->GetPhaseRunStatusReadFlag())
//	{
//		m_pOpenATCRunStatus->SetPhaseRunStatusReadFlag(false);
//		m_pOpenATCRunStatus->SetPhaseRunStatusWriteFlag(false);
//		m_pOpenATCRunStatus->SetPhaseRunStatusReadFlag(true);
//		m_pOpenATCRunStatus->SetPhaseRunStatusWriteFlag(true);
//		bIsReadFlag = false;
//	}
//	else
//	{
//		OpenATCSleep(100);
//		if (m_pOpenATCRunStatus->GetPhaseRunStatusReadFlag())
//		{
//			m_pOpenATCRunStatus->SetPhaseRunStatusReadFlag(false);
//			m_pOpenATCRunStatus->SetPhaseRunStatusWriteFlag(false);
//			m_pOpenATCRunStatus->SetPhaseRunStatusReadFlag(true);
//			m_pOpenATCRunStatus->SetPhaseRunStatusWriteFlag(true);
//			bIsReadFlag = false;
//		}
//	}
//
//	TPhaseRunStatus tRunStatus;
//	memset(&tRunStatus,0,sizeof(TPhaseRunStatus));
//	m_pOpenATCRunStatus->GetPhaseRunStatus(tRunStatus);
//
//	if (byAttributeID == CENTERCONTROL_PHASESTAGE)
//	{
//		if (m_chUnPackedBuff[byDataValuePos] >= 1 && m_chUnPackedBuff[byDataValuePos] <= 64)
//		{
//			if (m_chUnPackedBuff[byDataValuePos] > m_tAscParamInfo.m_stPhaseStageValidSize)
//			{
//				return CreateSetReturnData(BAD_VALUE_OVERFLOW, byIndex, CENTER_CONTROL, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
//			}
//			else
//			{
//				if (m_tAscParamInfo.m_stTransitBound[m_tAscParamInfo.m_stAscPatternTable[tRunStatus.m_byPlanID - 1].m_byPatternStage[tRunStatus.m_atRingRunStatus[0].m_nCurStageIndex] - 1].m_byTransitBound[m_chUnPackedBuff[byDataValuePos] - 1] == 255)
//				{
//					CreateSetReturnData(0, byIndex, CENTER_CONTROL, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
//				}
//				else
//				{
//					if (byElementID == 1)
//					{
//						tManualCmd.m_bNewCmd = true;
//						tManualCmd.m_nCmdSource = CTL_SOURCE_SYSTEM;
//						tManualCmd.m_tStepForwardCmd.m_byStepType = tStepCfg.m_byStepType;
//						tManualCmd.m_bStepForwardCmd = true;
//						tManualCmd.m_tStepForwardCmd.m_nNextStageID = m_chUnPackedBuff[byDataValuePos];
//						tManualCmd.m_tStepForwardCmd.m_nDelayTime = 0;
//						tManualCmd.m_tStepForwardCmd.m_nDurationTime = 0;
//						//tManualCmd.m_tStepForwardCmd.m_byStepType = tStepCfg.m_byStepType;
//						m_pOpenATCRunStatus->SetManualCmd(tManualCmd);
//
//						//m_pOpenATCRunStatus->SetStepInfo(true);
//					}
//					else
//					{
//						//命令路口2的跳阶段
//						tCenterCtrlInfo.m_byPhaseStage = m_chUnPackedBuff[byDataValuePos];
//						m_pOpenATCRunStatus->SetCenterCtrlInfo(tCenterCtrlInfo);
//						m_pOpenATCRunStatus->SetSendToRoadTwoStatus(true);
//					}
//					/*
//					m_tAscParamInfo.m_stCenterCtrlInfo.m_byPhaseStage = m_chUnPackedBuff[byDataValuePos];
//					//m_tAscParamInfo.m_stCenterCtrlInfo.m_byPhaseStage = CTL_MODE_MANUAL;//指定相位阶段
//					m_tAscParamInfo.m_stCenterCtrlInfo.m_byRunMode = 0x10;//运行模式进入中心控制模式
//
//					//TPhaseRunStatus tRunStatus;
//					//   memset(&tRunStatus,0,sizeof(TPhaseRunStatus));
//					//   m_pOpenATCRunStatus->GetPhaseRunStatus(tRunStatus);
//
//					tManualCmd.m_bNewCmd = true;
//					tManualCmd.m_nCmdSource = CTL_SOURCE_SYSTEM;
//					tManualCmd.m_tStepForwardCmd.m_byStepType = tStepCfg.m_byStepType;
//					tManualCmd.m_bStepForwardCmd = true;
//					tManualCmd.m_tStepForwardCmd.m_nNextStageID = m_chUnPackedBuff[byDataValuePos];
//					tManualCmd.m_tStepForwardCmd.m_nDelayTime = 0;
//					tManualCmd.m_tStepForwardCmd.m_nDurationTime = 0;
//					//tManualCmd.m_tStepForwardCmd.m_byStepType = tStepCfg.m_byStepType;
//					m_pOpenATCRunStatus->SetManualCmd(tManualCmd);
//
//					m_pOpenATCRunStatus->SetStepInfo(true);
//					*/
//				}
//
//				///////////////////////////////////////////////////////////////////////////////////// old
//				/*m_tAscParamInfo.m_stCenterCtrlInfo.m_byPhaseStage = m_chUnPackedBuff[byDataValuePos];
//				//m_tAscParamInfo.m_stCenterCtrlInfo.m_byPhaseStage = CTL_MODE_MANUAL;//指定相位阶段
//				m_tAscParamInfo.m_stCenterCtrlInfo.m_byRunMode = 0x10;//运行模式进入中心控制模式
//
//				//TPhaseRunStatus tRunStatus;
//			 //   memset(&tRunStatus,0,sizeof(TPhaseRunStatus));
//			 //   m_pOpenATCRunStatus->GetPhaseRunStatus(tRunStatus);
//
//				tManualCmd.m_bNewCmd = true;
//				tManualCmd.m_nCmdSource = CTL_SOURCE_SYSTEM;
//				tManualCmd.m_tStepForwardCmd.m_byStepType = tStepCfg.m_byStepType;
//				tManualCmd.m_bStepForwardCmd = true;
//				tManualCmd.m_tStepForwardCmd.m_nNextStageID = m_chUnPackedBuff[byDataValuePos];
//				tManualCmd.m_tStepForwardCmd.m_nDelayTime = 0;
//				tManualCmd.m_tStepForwardCmd.m_nDurationTime = 0;
//				//tManualCmd.m_tStepForwardCmd.m_byStepType = tStepCfg.m_byStepType;
//				m_pOpenATCRunStatus->SetManualCmd(tManualCmd);
//
//				m_pOpenATCRunStatus->SetStepInfo(true);*/
//				/////////////////////////////////////////////////////////////////////////////////////// old
//
//				//if (m_chUnPackedBuff[byDataValuePos] != 0)
//				//{
//				//	tManualCmd.m_bNewCmd = true;
//				//	tManualCmd.m_nCmdSource = CTL_SOURCE_SYSTEM;
//				//	tManualCmd.m_tStepForwardCmd.m_byStepType = tStepCfg.m_byStepType;
//				//	tManualCmd.m_bStepForwardCmd = true;
//				//	tManualCmd.m_tStepForwardCmd.m_nNextStageID = m_chUnPackedBuff[byDataValuePos];
//				//	tManualCmd.m_tStepForwardCmd.m_nDelayTime = 0;
//				//	tManualCmd.m_tStepForwardCmd.m_nDurationTime = 0;
//				//	tManualCmd.m_tStepForwardCmd.m_byStepType = tStepCfg.m_byStepType;
//				//	m_pOpenATCRunStatus->SetManualCmd(tManualCmd);
//				//}
//				//else
//				//{
//				//	tManualCmd.m_bNewCmd = true;
//				//	tManualCmd.m_nCmdSource = CTL_SOURCE_SYSTEM;
//				//	tManualCmd.m_bPatternInterruptCmd = true;
//				//	tManualCmd.m_tPatternInterruptCmd.m_nControlMode = tRunStatus.m_nCurCtlMode;//TransRunModeToASC(m_tAscParamInfo.m_stCenterCtrlInfo.m_byRunMode);
//				//	tManualCmd.m_tPatternInterruptCmd.m_nPatternNo = m_tAscParamInfo.m_stCenterCtrlInfo.m_byPattern;
//				//	m_pOpenATCRunStatus->SetManualCmd(tManualCmd);
//				//}
//			}
//		}
//		else
//		{
//			return CreateSetReturnData(BAD_VALUE_OVERFLOW, byIndex, CENTER_CONTROL, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
//		}
//	}
//	else if (byAttributeID == CENTERCONTROL_PATTERN)
//	{
//		if (m_chUnPackedBuff[byDataValuePos] >= 1 &&  m_chUnPackedBuff[byDataValuePos] <= 128)
//		{
//			if(byElementID == 1)	//路口1
//			{
//				tManualCmd.m_bNewCmd = true;
//				tManualCmd.m_nCmdSource = CTL_SOURCE_SYSTEM;
//				tManualCmd.m_bPatternInterruptCmd = true;
//				tManualCmd.m_tPatternInterruptCmd.m_nControlMode = tRunStatus.m_nCurCtlMode;//TransRunModeToASC(m_tAscParamInfo.m_stCenterCtrlInfo.m_byRunMode);
//				tManualCmd.m_tPatternInterruptCmd.m_nPatternNo = m_chUnPackedBuff[byDataValuePos];
//				m_pOpenATCRunStatus->SetManualCmd(tManualCmd);
//			}
//			else
//			{
//				//命令路口2的变方案
//				tCenterCtrlInfo.m_byPattern = m_chUnPackedBuff[byDataValuePos];
//				m_pOpenATCRunStatus->SetCenterCtrlInfo(tCenterCtrlInfo);
//				m_pOpenATCRunStatus->SetSendToRoadTwoStatus(true);
//			}
//			/*
//			 //这个实时控制不需要存储,但为了方便查询
//			 memset(&m_tAscParamInfo.m_stCenterCtrlInfo,0,sizeof(TCenterCtrlInfo));
//			 m_tAscParamInfo.m_stCenterCtrlInfo.m_byPattern = m_chUnPackedBuff[byDataValuePos];//指定方案
//
//			 //TPhaseRunStatus tRunStatus;
//			 //memset(&tRunStatus,0,sizeof(TPhaseRunStatus));
//			 //m_pOpenATCRunStatus->GetPhaseRunStatus(tRunStatus);
//
//			 tManualCmd.m_bNewCmd = true;
//			 tManualCmd.m_nCmdSource = CTL_SOURCE_SYSTEM;
//			 tManualCmd.m_bPatternInterruptCmd = true;
//			 tManualCmd.m_tPatternInterruptCmd.m_nControlMode = tRunStatus.m_nCurCtlMode;//TransRunModeToASC(m_tAscParamInfo.m_stCenterCtrlInfo.m_byRunMode);
//			 tManualCmd.m_tPatternInterruptCmd.m_nPatternNo = m_tAscParamInfo.m_stCenterCtrlInfo.m_byPattern;
//			 m_pOpenATCRunStatus->SetManualCmd(tManualCmd);
//			 */
//		}
//		else
//		{
//			return CreateSetReturnData(BAD_VALUE_OVERFLOW, byIndex, CENTER_CONTROL, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
//		}
//	}
//	else if (byAttributeID == CENTERCONTROL_RUNMODE)
//	{
//		if(byElementID == 1)	//路口1
//		{
//			//m_tAscParamInfo.m_stCenterCtrlInfo.m_byRunMode = m_chUnPackedBuff[byDataValuePos];//指定运行模式
//
//			//TPhaseRunStatus tRunStatus;
//			//memset(&tRunStatus,0,sizeof(TPhaseRunStatus));
//			//m_pOpenATCRunStatus->GetPhaseRunStatus(tRunStatus);
//
//			tManualCmd.m_bNewCmd = true;
//			tManualCmd.m_nCmdSource = CTL_SOURCE_SYSTEM;
//			tManualCmd.m_bPatternInterruptCmd = true;
//		
//			if (m_chUnPackedBuff[byDataValuePos] == 0x31)
//			{
//				tManualCmd.m_tPatternInterruptCmd.m_nControlMode = CTL_MODE_FLASH;
//				tManualCmd.m_tPatternInterruptCmd.m_nPatternNo = 0;
//				m_pOpenATCRunStatus->SetRunModeChange(true);
//			}
//			else if (m_chUnPackedBuff[byDataValuePos] == 0x32)
//			{
//				tManualCmd.m_tPatternInterruptCmd.m_nControlMode = CTL_MODE_ALLRED;
//				tManualCmd.m_tPatternInterruptCmd.m_nPatternNo = 0;
//				m_pOpenATCRunStatus->SetRunModeChange(true);
//			}
//			else if (m_chUnPackedBuff[byDataValuePos] == 0x33)
//			{
//				tManualCmd.m_tPatternInterruptCmd.m_nControlMode = CTL_MODE_OFF;
//				tManualCmd.m_tPatternInterruptCmd.m_nPatternNo = 0;
//				m_pOpenATCRunStatus->SetRunModeChange(true);
//			}
//			else if (m_chUnPackedBuff[byDataValuePos] == MODE_CENTER_MANUAL_CONTROL || m_chUnPackedBuff[byDataValuePos] == MODE_LOCAL_MANUAL_CONTROL)
//			{
//				//当为手动控制时，驻留在当且阶段【获取当且阶段，并且驻留】
//				tManualCmd.m_bNewCmd = true;
//				tManualCmd.m_nCmdSource = CTL_SOURCE_SYSTEM;
//				tManualCmd.m_tStepForwardCmd.m_byStepType = tStepCfg.m_byStepType;
//				tManualCmd.m_bStepForwardCmd = true;
//				tManualCmd.m_tStepForwardCmd.m_nNextStageID = m_tAscParamInfo.m_stAscPatternTable[tRunStatus.m_byPlanID - 1].m_byPatternStage[tRunStatus.m_atRingRunStatus[0].m_nCurStageIndex];
//				tManualCmd.m_tStepForwardCmd.m_nDelayTime = 0;
//				tManualCmd.m_tStepForwardCmd.m_nDurationTime = 0;
//				tManualCmd.m_bPatternInterruptCmd = false;
//
//				m_pOpenATCRunStatus->SetRunInManual(true);
//				//m_pOpenATCRunStatus->SetStepInfo(true);
//			}
//			else if (m_chUnPackedBuff[byDataValuePos] == MODE_CENTER_TIMETABLE_CONTROL)//日计划控制，就是按照当前配置的参数运行
//			{
//				tManualCmd.m_bPatternInterruptCmd = true;
//				tManualCmd.m_tPatternInterruptCmd.m_nControlMode = CTL_MODE_SELFCTL;
//				tManualCmd.m_tPatternInterruptCmd.m_nPatternNo = 0;
//				//return CreateSetReturnData(0, byIndex, CENTER_CONTROL, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
//			}
//			else
//			{
//				tManualCmd.m_bPatternInterruptCmd = true;
//				tManualCmd.m_tPatternInterruptCmd.m_nControlMode = TransRunModeToASC(m_chUnPackedBuff[byDataValuePos]);
//				tManualCmd.m_tPatternInterruptCmd.m_nPatternNo =  tRunStatus.m_byPlanID;//m_tAscParamInfo.m_stCenterCtrlInfo.m_byPattern;
//			}
//
//			/*m_pOpenATCRunStatus->SetRunModeChange(true);*/
//			m_pOpenATCRunStatus->SetManualCmd(tManualCmd);
//		}
//		else	//路口2
//		{
//			tCenterCtrlInfo.m_byRunMode = m_chUnPackedBuff[byDataValuePos];
//			m_pOpenATCRunStatus->SetCenterCtrlInfo(tCenterCtrlInfo);
//			m_pOpenATCRunStatus->SetSendToRoadTwoStatus(true);
//		}
//	}
//	CreateSetReturnData(0, byIndex, CENTER_CONTROL, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
//}

void  COpenATCCommWithGB20999Thread::SetOrderPipe(BYTE byIndex, BYTE byObjectID, BYTE byAttributeID, BYTE byElementID, BYTE byDataValuePos, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData, BYTE byDataValueLength)
{
	if (byObjectID != 1 && byObjectID != 2)
	{
		return CreateSetReturnData(BAD_VALUE_NULL, byIndex, ORDER_PIPE, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
	}

	if (byObjectID == 2)//交易跟踪
	{
		//if (byAttributeID == 0)
		if (byAttributeID == 1)		//创建交易18.2.1.0--HPH
		{
			/*2022.06.27事务部分重构 --HPH*/
			if (m_tDBManagement.m_byDBCreateTransaction == NORMAL)
			{
				if (m_chUnPackedBuff[byDataValuePos] == TRANSACTION)
				{
					m_tDBManagement.m_byDBCreateTransaction = TRANSACTION;

					m_pOpenATCParameter->GetAscParamInfo(m_tParamInfo);	//信号机参数每次都获取更新一次
					m_pOpenATCParameter->GetAscParamByRunParam(m_tVerifyParamInfo);
					//m_pOpenATCParameter->GetAscParamInfo(m_tVerifyParamInfo);	//测试那需要,可以单个配置。【因为，它下发是全表下发，所以也无关紧要，是否需要】
				}
				else
				{
					CreateSetReturnData(BAD_VALUE_STATUS, byIndex, ORDER_PIPE, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
				}
			}
			else if (m_tDBManagement.m_byDBCreateTransaction == TRANSACTION)
			{
				//maybe上位机ID
				if (m_chUnPackedBuff[byDataValuePos] == VERIFYING)
				{
					m_tDBManagement.m_byDBCreateTransaction = VERIFYING;
				}
				else if (m_chUnPackedBuff[byDataValuePos] == NORMAL)
				{
					//清空缓存
					memset(&m_tVerifyParamInfo,0,sizeof(TAscParam));
					//事务状态置为NORMAL
					memset(&m_tDBManagement,0,sizeof(TDBManagement));
					m_tDBManagement.m_byDBCreateTransaction = NORMAL;
					m_tDBManagement.m_byDBVerifyStatus = NOTDONE;
				}
				else
				{
					CreateSetReturnData(BAD_VALUE_STATUS, byIndex, ORDER_PIPE, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
				}
			}
			else if (m_tDBManagement.m_byDBCreateTransaction == VERIFYING)
			{
				//处于VERIFY状态下无法更改此状态
				CreateSetReturnData(BAD_VALUE_STATUS, byIndex, ORDER_PIPE, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
			}
			else	//DONE
			{
				if (m_chUnPackedBuff[byDataValuePos] == NORMAL)
				{
					if (m_tDBManagement.m_byDBVerifyStatus == DONEWITHNOERROR)
					{
						//将缓存写入数据库（这里是写入json）
						m_pOpenATCParameter->SaveGB20999ASCParam(false);
						//判断是否为相位控制
						if (m_bPhaseControlChange)
						{
							m_pOpenATCRunStatus->SetPhaseControlChange(true);
							m_bPhaseControlChange = false;
						}
						//m_pOpenATCParameter->SaveSetParameter(false);
						//清空缓存
						memset(&m_tVerifyParamInfo,0,sizeof(TAscParam));
						//事务状态置为NORMAL
						memset(&m_tDBManagement,0,sizeof(TDBManagement));
						m_tDBManagement.m_byDBCreateTransaction = NORMAL;
						m_tDBManagement.m_byDBVerifyStatus = NOTDONE;
						m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "Parameter have written!");
					}
					else if (m_tDBManagement.m_byDBVerifyStatus == DONEWITHERROR)
					{
						//记录故障
						//AddFaultInfo(TYPE_FAULT_CONFIG, SWITCH_NULL);
						//清空缓存
						memset(&m_tVerifyParamInfo,0,sizeof(TAscParam));
						//事务状态置为NORMAL
						memset(&m_tDBManagement,0,sizeof(TDBManagement));
						m_tDBManagement.m_byDBCreateTransaction = NORMAL;
						m_tDBManagement.m_byDBVerifyStatus = NOTDONE;
						m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "Parameter Verify Error!");
						//返回一个下发错误
						CreateSetReturnData(BAD_VALUE_ERROR, byIndex, ORDER_PIPE, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
					}
				}
				else if (m_chUnPackedBuff[byDataValuePos] == TRANSACTION)
				{
					m_tDBManagement.m_byDBCreateTransaction = TRANSACTION;
				}
				else
				{
					CreateSetReturnData(BAD_VALUE_STATUS, byIndex, ORDER_PIPE, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
				}
			}
			/*2022.06.27事务部分重构 --HPH*/
		}
		else
		{
			CreateSetReturnData(BAD_VALUE_NULL, byIndex, ORDER_PIPE, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		}
	}
	else//命令值
	{
		////用于路口2的手动命令
		//TCenterCtrlInfo tCenterCtrlInfo;
		//memset(&tCenterCtrlInfo,0,sizeof(tCenterCtrlInfo));

		//TManualCmd tManualCmd;
		//memset(&tManualCmd,0,sizeof(tManualCmd));

		//tManualCmd.m_bNewCmd = true;
		//tManualCmd.m_nCmdSource = CTL_SOURCE_SYSTEM;
		//tManualCmd.m_bPatternInterruptCmd = true;

		//if (m_chUnPackedBuff[byDataValuePos + 15] == 0x00 || m_chUnPackedBuff[byDataValuePos + 15] == 0x01 || m_chUnPackedBuff[byDataValuePos + 15] == 0x02 ||
		//	m_chUnPackedBuff[byDataValuePos + 15] == 0x03 || m_chUnPackedBuff[byDataValuePos + 15] == 0x04 || m_chUnPackedBuff[byDataValuePos + 15] == 0x05)
		//{
		//	if (m_chUnPackedBuff[byDataValuePos + 15] == 0x01)
		//	{
		//		tCenterCtrlInfo.m_byRunMode = 0x31;
		//		m_pOpenATCRunStatus->SetCenterCtrlInfo(tCenterCtrlInfo);
		//		m_pOpenATCRunStatus->SetSendToRoadTwoStatus(true);

		//		tManualCmd.m_tPatternInterruptCmd.m_nControlMode = CTL_MODE_FLASH;
		//		tManualCmd.m_tPatternInterruptCmd.m_nPatternNo = 0;
		//		m_pOpenATCRunStatus->SetManualCmd(tManualCmd);
		//		m_pOpenATCRunStatus->SetRunModeChange(true);
		//	}
		//	else if (m_chUnPackedBuff[byDataValuePos + 15] == 0x02)
		//	{
		//		tCenterCtrlInfo.m_byRunMode = 0x32;
		//		m_pOpenATCRunStatus->SetCenterCtrlInfo(tCenterCtrlInfo);
		//		m_pOpenATCRunStatus->SetSendToRoadTwoStatus(true);

		//		tManualCmd.m_tPatternInterruptCmd.m_nControlMode = CTL_MODE_ALLRED;
		//		tManualCmd.m_tPatternInterruptCmd.m_nPatternNo = 0;
		//		m_pOpenATCRunStatus->SetManualCmd(tManualCmd);
		//		m_pOpenATCRunStatus->SetRunModeChange(true);
		//	}
		//	else if (m_chUnPackedBuff[byDataValuePos + 15] == 0x03)				//开灯，恢复自主控制
		//	{
		//		tCenterCtrlInfo.m_byRunMode = 0x11;
		//		m_pOpenATCRunStatus->SetCenterCtrlInfo(tCenterCtrlInfo);
		//		m_pOpenATCRunStatus->SetSendToRoadTwoStatus(true);

		//		tManualCmd.m_bPatternInterruptCmd = true;
		//		tManualCmd.m_tPatternInterruptCmd.m_nControlMode = CTL_MODE_SELFCTL;
		//		tManualCmd.m_tPatternInterruptCmd.m_nPatternNo = 0;
		//		m_pOpenATCRunStatus->SetManualCmd(tManualCmd);
		//	}
		//	else if (m_chUnPackedBuff[byDataValuePos + 15] == 0x04)				//关灯
		//	{
		//		tCenterCtrlInfo.m_byRunMode = 0x33;
		//		m_pOpenATCRunStatus->SetCenterCtrlInfo(tCenterCtrlInfo);
		//		m_pOpenATCRunStatus->SetSendToRoadTwoStatus(true);

		//		tManualCmd.m_tPatternInterruptCmd.m_nControlMode = CTL_MODE_OFF;
		//		tManualCmd.m_tPatternInterruptCmd.m_nPatternNo = 0;
		//		m_pOpenATCRunStatus->SetManualCmd(tManualCmd);
		//		m_pOpenATCRunStatus->SetRunModeChange(true);
		//	}
		//	else if (m_chUnPackedBuff[byDataValuePos + 15] == 0x05)
		//	{
		//		m_pOpenATCRunStatus->SetRebootStatus(true);   
		//	}
		//	else if (m_chUnPackedBuff[byDataValuePos + 15] == 0x00)//取消
		//	{
		//		tCenterCtrlInfo.m_byRunMode = 0x11;
		//		m_pOpenATCRunStatus->SetCenterCtrlInfo(tCenterCtrlInfo);
		//		m_pOpenATCRunStatus->SetSendToRoadTwoStatus(true);

		//		tManualCmd.m_bPatternInterruptCmd = true;
		//		tManualCmd.m_tPatternInterruptCmd.m_nControlMode = CTL_MODE_SELFCTL;
		//		tManualCmd.m_tPatternInterruptCmd.m_nPatternNo = 0;
		//		m_pOpenATCRunStatus->SetManualCmd(tManualCmd);
		//	}
		//	memcpy(m_nPipeInfo, m_chUnPackedBuff + byDataValuePos, 16);
		//	//2022.07.04
		//	CreateSetReturnData(0, byIndex, ORDER_PIPE, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		//}
		//else
		//{
		//	CreateSetReturnData(BAD_VALUE_NULL, byIndex, ORDER_PIPE, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
		//}
	}
	CreateSetReturnData(0, byIndex, ORDER_PIPE, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
}

//私有类：跟随相位 --HPH 2021.12.06
//void  COpenATCCommWithGB20999Thread::SetPrivateData(BYTE byIndex, BYTE byObjectID, BYTE byAttributeID, BYTE byElementID, BYTE byDataValuePos, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData, BYTE byDataValueLength)
//{
//	if (byObjectID == OVERLAP_COUNT)
//	{
//		CreateSetReturnData(BAD_VALUE_READONLY, byIndex, PRIVATE_DATE, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
//	}
//	else if (byObjectID == OVERLAP_CONFIG)
//	{
//		if (byAttributeID == OVERLAP_ID)
//		{
//			CreateSetReturnData(BAD_VALUE_READONLY, byIndex, PRIVATE_DATE, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
//		}
//		else if (byAttributeID > OVERLAP_PULSETYPE)
//		{
//			CreateSetReturnData(BAD_VALUE_NULL, byIndex, PRIVATE_DATE, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
//		}
//		else
//		{
//			SetOverlapConfig(byIndex, byObjectID, byAttributeID, byElementID, byDataValuePos, tCorrectReturnData, tWrongReturnData, byDataValueLength);
//		}
//	}
//	else
//	{
//		CreateSetReturnData(BAD_VALUE_NULL, byIndex, PRIVATE_DATE, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
//	}
//}

//void  COpenATCCommWithGB20999Thread::SetOverlapConfig(BYTE byIndex, BYTE byObjectID, BYTE byAttributeID, BYTE byElementID, BYTE byDataValuePos, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData, BYTE byDataValueLength)
//{
//	if (byElementID < 1 || byElementID > MAX_OVERLAP_COUNT)
//	{
//		return CreateSetReturnData(BAD_VALUE_NULL, byIndex, PRIVATE_DATE, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
//	}
//
//	int byOverlapIndex = -1;
//
//	int  i = 0, j = 0, k = 0;
//	for (i = 0;i < MAX_OVERLAP_COUNT;i++)
//	{
//		if (m_tVerifyParamInfo.m_stAscOverlapTable[i].m_byOverlapNumber == byElementID)
//		{
//			byOverlapIndex = i;
//			break;
//		}
//	}
//
//	if (byOverlapIndex == -1)
//	{
//		byOverlapIndex = byElementID - 1;
//		m_tVerifyParamInfo.m_stAscOverlapTable[byOverlapIndex].m_byOverlapNumber = byElementID;
//		m_tVerifyParamInfo.m_stOverlapTableValidSize += 1;//跟随相位有效数--HPH
//	}
//
//	if (byAttributeID == OVERLAP_INCLUDEDPHASE)
//	{
//		if (byDataValueLength == 44)
//		{
//			int iCount = 0;
//			for (i = 0;i < MAX_PHASE_COUNT_IN_OVERLAP;i++)
//			{
//				if (m_chUnPackedBuff[byDataValuePos + i] > 0)
//				{
//					m_tVerifyParamInfo.m_stAscOverlapTable[byOverlapIndex].m_byArrOverlapIncludedPhases[iCount] = m_chUnPackedBuff[byDataValuePos + i];
//					iCount++;
//				}
//			}
//		}
//		else
//		{
//			return CreateSetReturnData(BAD_VALUE_WRONGLENGTH, byIndex, PRIVATE_DATE, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
//		}
//
//	}
//	else if (byAttributeID == OVERLAP_PULSETYPE)
//	{
//		m_tVerifyParamInfo.m_stAscOverlapTable[byOverlapIndex].m_byPulseType = m_chUnPackedBuff[byDataValuePos];
//	}
//
//	CreateSetReturnData(0, byIndex, PRIVATE_DATE, byObjectID, byAttributeID, byElementID, tCorrectReturnData, tWrongReturnData);
//}
////私有类：跟随相位 --HPH 2021.12.06

void  COpenATCCommWithGB20999Thread::CreateWrongQueryReturnData(BYTE byIndex, TReturnData tWrongReturnData)
{
	TDataConfig tDataConfig;
	tDataConfig.m_byIndex = byIndex + 1;
	tDataConfig.m_byDataClassID = m_chUnPackedBuff[C_N_DATACLASS_ID_POS + byIndex * 6];
	tDataConfig.m_byObjectID = m_chUnPackedBuff[C_N_OBJECT_ID_POS + byIndex * 6];
	tDataConfig.m_byAttributeID = m_chUnPackedBuff[C_N_ATTRIBUTE_ID_POS + byIndex * 6];
	tDataConfig.m_byElementID = m_chUnPackedBuff[C_N_ELEMENT_ID_POS + byIndex * 6];
	tDataConfig.m_byDataValueLength = 1;
	tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
	tDataConfig.m_byDataValue[0] = BAD_VALUE_NULL;
	memcpy(&tWrongReturnData.m_tDataConfig[tWrongReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
	tWrongReturnData.m_byReturnCount += 1;
}

void  COpenATCCommWithGB20999Thread::AckCtl_ReturnData(bool bCorrect, TReturnData & tReturnData)
{
	BYTE byFrameID = m_chUnPackedBuff[C_N_FRAME_ID_POS];

    unsigned int dwSendBuffLen = 0;
	memset(m_chSendBuff, 0x00, SEND_BUFFER_SIZE);

	short nAgreementVersion = C_N_AGREEMENT_VERSION;
    int   nATCID = htonl(m_pOpenATCParameter->GetSiteID());

	m_chSendBuff[dwSendBuffLen++] = 0;                                      //报文长度(先默认为0，后面再计算)  
	m_chSendBuff[dwSendBuffLen++] = 0;                                      //报文长度(先默认为0，后面再计算)  
	memcpy(m_chSendBuff + dwSendBuffLen, &nAgreementVersion, 2);            //协议版本
	dwSendBuffLen += 2;
	m_chSendBuff[dwSendBuffLen++] = C_N_HOST_ID;				            //上位机ID
	memcpy(m_chSendBuff + dwSendBuffLen, &nATCID, 4);			            //信号机ID
	dwSendBuffLen += 4;
	m_chSendBuff[dwSendBuffLen++] = C_N_ROAD_ID;							//路口ID
	m_chSendBuff[dwSendBuffLen++] = byFrameID;			                    //帧流水号
	if (bCorrect)
	{
		m_chSendBuff[dwSendBuffLen++] = FRAME_TYPE_QUERY_REPLY;             //帧类型
	}
	else
	{
		m_chSendBuff[dwSendBuffLen++] = FRAME_TYPE_QUERY_ERRORREPLY;        //帧类型
	}
	m_chSendBuff[dwSendBuffLen++] = tReturnData.m_byReturnCount;            //数据值数量

	for (int i = 0;i < tReturnData.m_byReturnCount;i++)
	{
		m_chSendBuff[dwSendBuffLen++] = tReturnData.m_tDataConfig[i].m_byIndex;      //数据值索引
		m_chSendBuff[dwSendBuffLen++] = tReturnData.m_tDataConfig[i].m_byDataLength; //数据值长度
		m_chSendBuff[dwSendBuffLen++] = tReturnData.m_tDataConfig[i].m_byDataClassID;//数据类ID
		m_chSendBuff[dwSendBuffLen++] = tReturnData.m_tDataConfig[i].m_byObjectID;   //对象ID
		m_chSendBuff[dwSendBuffLen++] = tReturnData.m_tDataConfig[i].m_byAttributeID;//属性ID
		m_chSendBuff[dwSendBuffLen++] = tReturnData.m_tDataConfig[i].m_byElementID;  //元素ID
		//元素值
		memcpy(m_chSendBuff + dwSendBuffLen, tReturnData.m_tDataConfig[i].m_byDataValue, tReturnData.m_tDataConfig[i].m_byDataValueLength);
		dwSendBuffLen += tReturnData.m_tDataConfig[i].m_byDataValueLength;
	}

	unsigned short nCrc16 = Crc16(m_chSendBuff, dwSendBuffLen);
	nCrc16 = htons(nCrc16);
	memcpy(m_chSendBuff + dwSendBuffLen, &nCrc16, 2);                       //校验
	dwSendBuffLen += 2;

	short nPacketLen = htons(dwSendBuffLen);
	memcpy(m_chSendBuff, &nPacketLen, 2);	                                //报文长度

	int nRet = 0;
	SendAckToPeer(dwSendBuffLen, nRet);
}

int COpenATCCommWithGB20999Thread::AckCtl_AskHeart()
{
	BYTE byFrameID = m_chUnPackedBuff[C_N_FRAME_ID_POS];

    unsigned int dwSendBuffLen = 0;
	memset(m_chSendBuff, 0x00, SEND_BUFFER_SIZE);

	short nAgreementVersion = C_N_AGREEMENT_VERSION;
    int   nATCID = htonl(m_pOpenATCParameter->GetSiteID());

	m_chSendBuff[dwSendBuffLen++] = 0;                                      //报文长度(先默认为0，后面再计算)  
	m_chSendBuff[dwSendBuffLen++] = 0;                                      //报文长度(先默认为0，后面再计算)  
	memcpy(m_chSendBuff + dwSendBuffLen, &nAgreementVersion, 2);            //协议版本
	dwSendBuffLen += 2;
	m_chSendBuff[dwSendBuffLen++] = C_N_HOST_ID;				            //上位机ID
	memcpy(m_chSendBuff + dwSendBuffLen, &nATCID, 4);			            //信号机ID
	dwSendBuffLen += 4;
	m_chSendBuff[dwSendBuffLen++] = C_N_ROAD_ID;							//路口ID
	m_chSendBuff[dwSendBuffLen++] = byFrameID;			                    //帧流水号
	m_chSendBuff[dwSendBuffLen++] = FRAME_TYPE_HEART_REPLY;                 //帧类型
	
	unsigned short nCrc16 = Crc16(m_chSendBuff, dwSendBuffLen);  
	nCrc16 = htons(nCrc16);
	memcpy(m_chSendBuff + dwSendBuffLen, &nCrc16, 2);                       //校验
	dwSendBuffLen += 2;

	short nPacketLen = htons(dwSendBuffLen);
	memcpy(m_chSendBuff, &nPacketLen, 2);	                                //报文长度
 
    return dwSendBuffLen;
}

void COpenATCCommWithGB20999Thread::AckCtl_AskDeviceInfo(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData)
{
	TDataConfig tDataConfig;
	tDataConfig.m_byIndex = byIndex + 1;
	tDataConfig.m_byDataClassID = m_chUnPackedBuff[C_N_DATACLASS_ID_POS + byIndex * 6];
	tDataConfig.m_byObjectID = m_chUnPackedBuff[C_N_OBJECT_ID_POS + byIndex * 6];
	tDataConfig.m_byAttributeID = m_chUnPackedBuff[C_N_ATTRIBUTE_ID_POS + byIndex * 6];
	tDataConfig.m_byElementID = m_chUnPackedBuff[C_N_ELEMENT_ID_POS + byIndex * 6];
	tDataConfig.m_byDataValueLength = 1;
	tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;

	if (tDataConfig.m_byObjectID == MANUFACTURER)
	{
		tDataConfig.m_byDataValueLength = 128;
	    tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
		memset(m_tAscParamInfo.m_stDeviceInfo.m_chManufacturer, 0, sizeof(m_tAscParamInfo.m_stDeviceInfo.m_chManufacturer));
		sprintf((char *)m_tAscParamInfo.m_stDeviceInfo.m_chManufacturer, "kedacom");
		memcpy(tDataConfig.m_byDataValue, m_tAscParamInfo.m_stDeviceInfo.m_chManufacturer, 128);//制造厂商
		memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		tCorrectReturnData.m_byReturnCount += 1;
	}
	else if (tDataConfig.m_byObjectID == DEVICE_VERSION)
	{
		//设备版本
		BYTE m_byDeviceVersion[4];
		tDataConfig.m_byDataValueLength = 4;
	    tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
		m_tAscParamInfo.m_stDeviceInfo.m_byDeviceVersion[0] = 0x01;
		m_tAscParamInfo.m_stDeviceInfo.m_byDeviceVersion[1] = 0x01;
		m_tAscParamInfo.m_stDeviceInfo.m_byDeviceVersion[2] = 0x01;
		m_tAscParamInfo.m_stDeviceInfo.m_byDeviceVersion[3] = 0x01;
		tDataConfig.m_byDataValue[0] = m_tAscParamInfo.m_stDeviceInfo.m_byDeviceVersion[0];//软件版本
		tDataConfig.m_byDataValue[1] = m_tAscParamInfo.m_stDeviceInfo.m_byDeviceVersion[1];
		tDataConfig.m_byDataValue[2] = m_tAscParamInfo.m_stDeviceInfo.m_byDeviceVersion[2];//硬件版本
		tDataConfig.m_byDataValue[3] = m_tAscParamInfo.m_stDeviceInfo.m_byDeviceVersion[3];
		memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		tCorrectReturnData.m_byReturnCount += 1;
	}
	else if (tDataConfig.m_byObjectID == DEVICE_ID)
	{
		//设备编号
		tDataConfig.m_byDataValueLength = 16;
	    tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
		memset(m_tAscParamInfo.m_stDeviceInfo.m_chDeviceNum, 0, sizeof(m_tAscParamInfo.m_stDeviceInfo.m_chDeviceNum));
		memcpy(tDataConfig.m_byDataValue, m_tAscParamInfo.m_stDeviceInfo.m_chDeviceNum, 16);//设备编号
		memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		tCorrectReturnData.m_byReturnCount += 1;
	}
	else if (tDataConfig.m_byObjectID == MANUFACTUR_DATE)
	{
		//出厂日期
		tDataConfig.m_byDataValueLength = 7;
		tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
		memcpy(tDataConfig.m_byDataValue, m_tAscParamInfo.m_stDeviceInfo.m_byDateOfProduction, 7);
		memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		tCorrectReturnData.m_byReturnCount += 1;
	}
	else if (tDataConfig.m_byObjectID == CONFIG_DATE)
	{
		//配置日期
		tDataConfig.m_byDataValueLength = 7;
		tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
		//m_pOpenATCLog->LogOneMessage(LEVEL_INFO, "month:%d day:%d hour:%d min:%d sec:%d",m_tAscParamInfo.m_stDeviceInfo.m_byDateOfConfig[2],m_tAscParamInfo.m_stDeviceInfo.m_byDateOfConfig[3],m_tAscParamInfo.m_stDeviceInfo.m_byDateOfConfig[4],m_tAscParamInfo.m_stDeviceInfo.m_byDateOfConfig[5],m_tAscParamInfo.m_stDeviceInfo.m_byDateOfConfig[6]);
		memcpy(tDataConfig.m_byDataValue, m_tAscParamInfo.m_stDeviceInfo.m_byDateOfConfig, 7);
		memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		tCorrectReturnData.m_byReturnCount += 1;
	}
	else
	{
		tDataConfig.m_byDataValue[0] = BAD_VALUE_NULL;
		memcpy(&tWrongReturnData.m_tDataConfig[tWrongReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		tWrongReturnData.m_byReturnCount += 1;
	}
}

void COpenATCCommWithGB20999Thread::AckCtl_AskBaseInfo(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData)
{
	TDataConfig tDataConfig;
	tDataConfig.m_byIndex = byIndex + 1;
	tDataConfig.m_byDataClassID = m_chUnPackedBuff[C_N_DATACLASS_ID_POS + byIndex * 6];
	tDataConfig.m_byObjectID = m_chUnPackedBuff[C_N_OBJECT_ID_POS + byIndex * 6];
	tDataConfig.m_byAttributeID = m_chUnPackedBuff[C_N_ATTRIBUTE_ID_POS + byIndex * 6];
	tDataConfig.m_byElementID = m_chUnPackedBuff[C_N_ELEMENT_ID_POS + byIndex * 6];
	tDataConfig.m_byDataValueLength = 1;
	tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
	int i = 0;
	if (tDataConfig.m_byObjectID == INSTALLATION_ROAD)
	{
		//安装路口，路口ID
		tDataConfig.m_byDataValueLength = 128;
	    tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
		memset(m_tAscParamInfo.m_stBaseInfo.m_bySiteRoadID, 0, sizeof(m_tAscParamInfo.m_stBaseInfo.m_bySiteRoadID));
		memcpy(tDataConfig.m_byDataValue, m_tAscParamInfo.m_stBaseInfo.m_bySiteRoadID, 128);
		memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		tCorrectReturnData.m_byReturnCount += 1;
	}
	else if (tDataConfig.m_byObjectID == ATC_IPV4_NETCONFIG || tDataConfig.m_byObjectID == ATC_IPV6_NETCONFIG)
	{
		if (tDataConfig.m_byAttributeID == ATC_IP_ADDRESS)
		{
			if (tDataConfig.m_byObjectID == ATC_IPV4_NETCONFIG)
			{
				tDataConfig.m_byDataValueLength = 4;
				tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
				for (i = 0;i < 4;i++)
				{
					tDataConfig.m_byDataValue[i] = m_tAscParamInfo.m_stBaseInfo.m_stAscIPV4.m_chAscIPV4IP[i];
				}
			}
			else
			{
			    tDataConfig.m_byDataValueLength = 16;
				tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
				for (i = 0;i < 16;i++)
				{
					tDataConfig.m_byDataValue[i] = m_tAscParamInfo.m_stBaseInfo.m_stAscIPV6.m_chAscIPV6IP[i];
				}
			}
			memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
		}
		else if (tDataConfig.m_byAttributeID == SUB_NET)
		{
			if (tDataConfig.m_byObjectID == ATC_IPV4_NETCONFIG)
			{
				tDataConfig.m_byDataValueLength = 4;
				tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
				for (i = 0;i < 4;i++)
				{
					tDataConfig.m_byDataValue[i] = m_tAscParamInfo.m_stBaseInfo.m_stAscIPV4.m_chAscIPV4SubNet[i];
				}
			}
			else
			{
			    tDataConfig.m_byDataValueLength = 16;
				tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
				for (i = 0;i < 16;i++)
				{
					tDataConfig.m_byDataValue[i] = m_tAscParamInfo.m_stBaseInfo.m_stAscIPV6.m_chAscIPV6SubNet[i];
				}
			}
			memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
		}
		else if (tDataConfig.m_byAttributeID == GATE_WAY)
		{
			if (tDataConfig.m_byObjectID == ATC_IPV4_NETCONFIG)
			{
				tDataConfig.m_byDataValueLength = 4;
				tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
				for (i = 0;i < 4;i++)
				{
					tDataConfig.m_byDataValue[i] = m_tAscParamInfo.m_stBaseInfo.m_stAscIPV4.m_chAscIPV4GateWay[i];
				}
			}
			else
			{
			    tDataConfig.m_byDataValueLength = 16;
				tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
				for (i = 0;i < 16;i++)
				{
					tDataConfig.m_byDataValue[i] = m_tAscParamInfo.m_stBaseInfo.m_stAscIPV6.m_chAscIPV6GateWay[i];
				}
			}
			memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
		}
		else
		{
			tDataConfig.m_byDataValue[0] = BAD_VALUE_NULL;
			memcpy(&tWrongReturnData.m_tDataConfig[tWrongReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tWrongReturnData.m_byReturnCount += 1;
		}
	}
	else if (tDataConfig.m_byObjectID == HOST_IPV4_NETCONFIG || tDataConfig.m_byObjectID == HOST_IPV6_NETCONFIG)
	{
		if (tDataConfig.m_byAttributeID == HOST_IP_ADDRESS)
		{
			if (tDataConfig.m_byObjectID == HOST_IPV4_NETCONFIG)
			{
				tDataConfig.m_byDataValueLength = 4;
				tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
				for (i = 0;i < 4;i++)
				{
					tDataConfig.m_byDataValue[i] = m_tAscParamInfo.m_stBaseInfo.m_stCerterIPV4.m_chCerterIPV4IP[i];
				}
			}
			else
			{
			    tDataConfig.m_byDataValueLength = 16;
				tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
				for (i = 0;i < 16;i++)
				{
					tDataConfig.m_byDataValue[i] = m_tAscParamInfo.m_stBaseInfo.m_stCerterIPV6.m_chCerterIPV6IP[i];
				}
			}
			memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
		}
		else if (tDataConfig.m_byAttributeID == COM_PORT)
		{
			tDataConfig.m_byDataValueLength = 2;
	        tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			int nPort = 0;
			if (tDataConfig.m_byObjectID == HOST_IPV4_NETCONFIG)
			{
				nPort = ntohs(m_tAscParamInfo.m_stBaseInfo.m_stCerterIPV4.m_chCerterIPV4Port);
				memcpy(tDataConfig.m_byDataValue, &nPort, 2);
			}
			else
			{
				nPort = ntohs(m_tAscParamInfo.m_stBaseInfo.m_stCerterIPV6.m_chCerterIPV6Port);
				memcpy(tDataConfig.m_byDataValue, &nPort, 2);
			}
			memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
		}
		else if (tDataConfig.m_byAttributeID == COM_TYPE)
		{
			tDataConfig.m_byDataValueLength = 1;
	        tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
			if (tDataConfig.m_byObjectID == HOST_IPV4_NETCONFIG)
			{
				tDataConfig.m_byDataValue[0] = m_tAscParamInfo.m_stBaseInfo.m_stCerterIPV4.m_chCerterIPV4Type;
			}
			else
			{
				tDataConfig.m_byDataValue[0] = m_tAscParamInfo.m_stBaseInfo.m_stCerterIPV6.m_chCerterIPV6Type;
			}
			memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
		}
		else
		{
			tDataConfig.m_byDataValue[0] = BAD_VALUE_NULL;
			memcpy(&tWrongReturnData.m_tDataConfig[tWrongReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tWrongReturnData.m_byReturnCount += 1;
		}
	}
	else if (tDataConfig.m_byObjectID == ATC_TIMEZONE)
	{
		tDataConfig.m_byDataValueLength = 4;
		tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
		int nTimeZone = ntohl(m_tAscParamInfo.m_stBaseInfo.m_wTimeZone);
	    memcpy(tDataConfig.m_byDataValue, &nTimeZone, 4);
		memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		tCorrectReturnData.m_byReturnCount += 1;
	}
	else if (tDataConfig.m_byObjectID == ATC_ID)
	{
		tDataConfig.m_byDataValueLength = 4;
	    tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
		int nATCID = ntohl(m_tAscParamInfo.m_stBaseInfo.m_wATCCode);
	    memcpy(tDataConfig.m_byDataValue, &nATCID, 4);
		memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		tCorrectReturnData.m_byReturnCount += 1;
	}
	else if (tDataConfig.m_byObjectID == ATC_ROADCOUNT)
	{
		/*TAscCasCadeInfo tCasCade;
		memset(&tCasCade, 0x00, sizeof(tCasCade));
		m_pOpenATCParameter->GetAscCasCadeInfo(tCasCade);
       
        if (tCasCade.m_byJoinOffset == 0 && tCasCade.m_byLocalLampBoardNum == 0)//没有级联信号机
		{
			tDataConfig.m_byDataValueLength = 1;
	        tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
		    tDataConfig.m_byDataValue[0] = 1;	
			memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		    tCorrectReturnData.m_byReturnCount += 1;
		}
		else
		{
			tDataConfig.m_byDataValueLength = 1;
	        tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
		    tDataConfig.m_byDataValue[0] = 2;	
			memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		    tCorrectReturnData.m_byReturnCount += 1;
		}*/

		tDataConfig.m_byDataValueLength = 1;
	    tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
		tDataConfig.m_byDataValue[0] = m_tAscParamInfo.m_stBaseInfo.m_byCrossRoadNum;
		memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		tCorrectReturnData.m_byReturnCount += 1;

	}
	else if (tDataConfig.m_byObjectID == GPS_CLOCK_FLAG)
	{
		tDataConfig.m_byDataValueLength = 1;
	    tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;
		tDataConfig.m_byDataValue[0] = 1;	
		memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		tCorrectReturnData.m_byReturnCount += 1;
	}
	else
	{
		tDataConfig.m_byDataValue[0] = BAD_VALUE_NULL;
		memcpy(&tWrongReturnData.m_tDataConfig[tWrongReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
		tWrongReturnData.m_byReturnCount += 1;
	}
}

void COpenATCCommWithGB20999Thread::AckCtl_AskLightGroupInfo(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData)
{
	BYTE byObjectID = m_chUnPackedBuff[C_N_OBJECT_ID_POS + byIndex * 6];
	BYTE byAttributeID = m_chUnPackedBuff[C_N_ATTRIBUTE_ID_POS + byIndex * 6];
	BYTE byElementID = m_chUnPackedBuff[C_N_ELEMENT_ID_POS + byIndex * 6];

	if (byObjectID == LIGHT_GROUP_COUNT)
	{
		return QueryLightGroupCount(byIndex, tCorrectReturnData, tWrongReturnData);
	}
	else if (byObjectID == LIGHT_GROUP_CONFIG_TABLE && byElementID == 0)
	{
		return QueryAllElementLightGroupConfig(byIndex, tCorrectReturnData, tWrongReturnData);
	}
	else if (byObjectID == LIGHT_GROUP_STATUS_TABLE && byElementID == 0)
	{
		return QueryAllElementLightGroupStatus(byIndex, tCorrectReturnData, tWrongReturnData);
	}
	else if (byObjectID == LIGHT_GROUP_CONTROL_TABLE && byElementID == 0)
	{
		return QueryAllElementLightGroupControl(byIndex, tCorrectReturnData, tWrongReturnData);
	}
	else if (byObjectID == LIGHT_GROUP_CONFIG_TABLE)
	{
		if (byAttributeID > LIGHT_GROUP_TYPE)
		{
			CreateWrongQueryReturnData(byIndex, tWrongReturnData);
		}
		else
		{
			QueryLightGroupConfig(byIndex, tCorrectReturnData, tWrongReturnData);
		}
	}
	else if (byObjectID == LIGHT_GROUP_STATUS_TABLE)
	{
		if (byAttributeID > LIGHT_GROUP_STATUS)
		{
			CreateWrongQueryReturnData(byIndex, tWrongReturnData);
		}
		else
		{
			QueryLightGroupStatus(byIndex, tCorrectReturnData, tWrongReturnData);
		}
	}
	else if (byObjectID == LIGHT_GROUP_CONTROL_TABLE)
	{
		if (byAttributeID > CONTROL_PROHIBIT)
		{
			CreateWrongQueryReturnData(byIndex, tWrongReturnData);
		}
		else
		{
			QueryLightGroupControl(byIndex, tCorrectReturnData, tWrongReturnData);
		}
	}
	else
	{
		CreateWrongQueryReturnData(byIndex, tWrongReturnData);
	}
}

void COpenATCCommWithGB20999Thread::AckCtl_AskPhaseInfo(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData)
{
	BYTE byObjectID = m_chUnPackedBuff[C_N_OBJECT_ID_POS + byIndex * 6];
	BYTE byAttributeID = m_chUnPackedBuff[C_N_ATTRIBUTE_ID_POS + byIndex * 6];
	BYTE byElementID = m_chUnPackedBuff[C_N_ELEMENT_ID_POS + byIndex * 6];

	if (byObjectID == PHASE_COUNT)
	{
		return QueryPhaseCount(byIndex, tCorrectReturnData, tWrongReturnData);
	}
	else if (byObjectID == PHASE_CONFIG_TABLE && byElementID == 0)
	{
		return QueryAllElementPhaseConfig(byIndex, tCorrectReturnData, tWrongReturnData);
	}
	else if (byObjectID == LIGHT_GROUP_CONTROL_TABLE && byElementID == 0)
	{
		return QueryAllElementPhaseControl(byIndex, tCorrectReturnData, tWrongReturnData);
	}
	else if (byObjectID == PHASE_CONFIG_TABLE)
	{
		if (byAttributeID > PHASE_CALL)
		{
			CreateWrongQueryReturnData(byIndex, tWrongReturnData);
		}
		else
		{
			QueryPhaseConfig(byIndex, tCorrectReturnData, tWrongReturnData);
		}
	}
	else if (byObjectID == PHASE_CONTROL_TABLE)
	{
		if (byAttributeID > PHASE_CONTROL_PROHIBIT)
		{
			CreateWrongQueryReturnData(byIndex, tWrongReturnData);
		}
		else
		{
			QueryPhaseControl(byIndex, tCorrectReturnData, tWrongReturnData);
		}
	}
	else
	{
		CreateWrongQueryReturnData(byIndex, tWrongReturnData);
	}
}

void COpenATCCommWithGB20999Thread::AckCtl_AskDetectorInfo(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData)
{
	BYTE byObjectID = m_chUnPackedBuff[C_N_OBJECT_ID_POS + byIndex * 6];
	BYTE byAttributeID = m_chUnPackedBuff[C_N_ATTRIBUTE_ID_POS + byIndex * 6];
	BYTE byElementID = m_chUnPackedBuff[C_N_ELEMENT_ID_POS + byIndex * 6];

	if (byObjectID == DETECTOR_COUNT)
	{
		return QueryDetectorCount(byIndex, tCorrectReturnData, tWrongReturnData);
	}
	else if (byObjectID == DETECTOR_CONFIG_TABLE && byElementID == 0)
	{
		return QueryAllElementDetectorConfig(byIndex, tCorrectReturnData, tWrongReturnData);
	}
	else if (byObjectID == DETECTOR_DATA_TABLE && byElementID == 0)
	{
		return QueryAllElementDetectorData(byIndex, tCorrectReturnData, tWrongReturnData);
	}
    else if (byObjectID == DETECTOR_CONFIG_TABLE)
	{
		if (byAttributeID > INSTALL_POS)
		{
			CreateWrongQueryReturnData(byIndex, tWrongReturnData);
		}
		else
		{
			QueryDetectorConfig(byIndex, tCorrectReturnData, tWrongReturnData);
		}
	}
	else  if (byObjectID == DETECTOR_DATA_TABLE)
	{
		if (byAttributeID > QUEUE_LENGTH)
		{
			CreateWrongQueryReturnData(byIndex, tWrongReturnData);
		}
		else
		{
			QueryDetectorData(byIndex, tCorrectReturnData, tWrongReturnData);
		}
	}
	else
	{
		CreateWrongQueryReturnData(byIndex, tWrongReturnData);
	}
}

void COpenATCCommWithGB20999Thread::AckCtl_AskPhaseStageInfo(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData)
{
	BYTE byObjectID = m_chUnPackedBuff[C_N_OBJECT_ID_POS + byIndex * 6];
	BYTE byAttributeID = m_chUnPackedBuff[C_N_ATTRIBUTE_ID_POS + byIndex * 6];
	BYTE byElementID = m_chUnPackedBuff[C_N_ELEMENT_ID_POS + byIndex * 6];

	if (byObjectID == PHASE_STAGE_COUNT)
	{
		return QueryPhaseStageCount(byIndex, tCorrectReturnData, tWrongReturnData);
	}
	else if (byObjectID == PHASE_STAGE_CONFIG_TABLE && byElementID == 0)
	{
		return QueryAllElementPhaseStageConfig(byIndex, tCorrectReturnData, tWrongReturnData);
	}
	else if (byObjectID == PHASE_STAGE_STATUS_TABLE && byElementID == 0)
	{
		return QueryAllElementPhaseStageStatus(byIndex, tCorrectReturnData, tWrongReturnData);
	}
	else if (byObjectID == PHASE_STAGE_CONTROL_TABLE && byElementID == 0)
	{
		return QueryAllElementPhaseStageControl(byIndex, tCorrectReturnData, tWrongReturnData);
	}
    else if (byObjectID == PHASE_STAGE_CONFIG_TABLE)
	{
		if (byAttributeID > PHASE_STAGE_EARLY_END)
		{
			CreateWrongQueryReturnData(byIndex, tWrongReturnData);
		}
		else
		{
			QueryPhaseStageConfig(byIndex, tCorrectReturnData, tWrongReturnData);
		}
	}
	else  if (byObjectID == PHASE_STAGE_STATUS_TABLE)
	{
		if (byAttributeID > PHASE_STAGE_REMAIN_TIME)
		{
			CreateWrongQueryReturnData(byIndex, tWrongReturnData);
		}
		else
		{
			QueryPhaseStageStatus(byIndex, tCorrectReturnData, tWrongReturnData);
		}
	}
	else  if (byObjectID == PHASE_STAGE_CONTROL_TABLE)
	{
		if (byAttributeID > PHASE_STAGE_PROHIBIT)
		{
			CreateWrongQueryReturnData(byIndex, tWrongReturnData);
		}
		else
		{
			QueryPhaseStageControl(byIndex, tCorrectReturnData, tWrongReturnData);
		}
	}
	else
	{
		CreateWrongQueryReturnData(byIndex, tWrongReturnData);
	}
}

void COpenATCCommWithGB20999Thread::AckCtl_AskPhaseSafetyInfo(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData)
{
	BYTE byObjectID = m_chUnPackedBuff[C_N_OBJECT_ID_POS + byIndex * 6];
	BYTE byAttributeID = m_chUnPackedBuff[C_N_ATTRIBUTE_ID_POS + byIndex * 6];
	BYTE byElementID = m_chUnPackedBuff[C_N_ELEMENT_ID_POS + byIndex * 6];

	if (byObjectID == PHASE_CONFLICT_TABLE && byElementID == 0)
	{
		return QueryAllElementPhaseeConflictInfo(byIndex, tCorrectReturnData, tWrongReturnData);
	}
	else if (byObjectID == PHASE_GREENINTERVAL_TABLE && byElementID == 0)
	{
		return QueryAllElementPhasGreenInterval(byIndex, tCorrectReturnData, tWrongReturnData);
	}
	else if (byObjectID == PHASE_CONFLICT_TABLE)
	{
		QueryPhaseConflict(byIndex, tCorrectReturnData, tWrongReturnData);
	}
	else if (byObjectID == PHASE_GREENINTERVAL_TABLE)
	{
		QueryPhaseGreenInterval(byIndex, tCorrectReturnData, tWrongReturnData);
	}
	else
	{
		CreateWrongQueryReturnData(byIndex, tWrongReturnData);
	}
}

void COpenATCCommWithGB20999Thread::AckCtl_AskEmergencyPriorityInfo(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData)
{
	BYTE byObjectID = m_chUnPackedBuff[C_N_OBJECT_ID_POS + byIndex * 6];
	BYTE byAttributeID = m_chUnPackedBuff[C_N_ATTRIBUTE_ID_POS + byIndex * 6];
	BYTE byElementID = m_chUnPackedBuff[C_N_ELEMENT_ID_POS + byIndex * 6];

	if (byObjectID == PRIORITY_COUNT)
	{
        return QueryPriorityCount(byIndex, tCorrectReturnData, tWrongReturnData);
	}
	else if (byObjectID == PRIORITY_CONFIG_TABLE)
	{
		if (byElementID == 0)
		{
			return QueryAllElementPriorityConfig(byIndex, tCorrectReturnData, tWrongReturnData);
		}
		else
		{
			return QueryPriorityConfig(byIndex, tCorrectReturnData, tWrongReturnData);
		}
	}
	else if (byObjectID == PRIORITY_STATUS_TABLE)
	{
		if (byElementID == 0)
		{
			return QueryAllElementPriorityStatus(byIndex, tCorrectReturnData, tWrongReturnData);
		}
		else
		{
			return QueryPriorityStatus(byIndex, tCorrectReturnData, tWrongReturnData);
		}
	}
	else if (byObjectID == EMERGENCY_COUNT)
	{
        return QueryEmergencyCount(byIndex, tCorrectReturnData, tWrongReturnData);
	}
	else if (byObjectID == EMERGENCY_CONFIG_TABLE)
	{
		if (byElementID == 0)
		{
			return QueryAllElementEmergencyConfig(byIndex, tCorrectReturnData, tWrongReturnData);
		}
		else
		{
			return QueryEmergencyConfig(byIndex, tCorrectReturnData, tWrongReturnData);
		}
	}
	else if (byObjectID == EMERGENCY_STATUS_TABLE)
	{
		if (byElementID == 0)
		{
			return QueryAllElementEmergencyStatus(byIndex, tCorrectReturnData, tWrongReturnData);
		}
		else
		{
			return QueryEmergencyStatus(byIndex, tCorrectReturnData, tWrongReturnData);
		}
	}
}

void COpenATCCommWithGB20999Thread::AckCtl_AskPatternInfo(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData)
{
	BYTE byObjectID = m_chUnPackedBuff[C_N_OBJECT_ID_POS + byIndex * 6];
	BYTE byAttributeID = m_chUnPackedBuff[C_N_ATTRIBUTE_ID_POS + byIndex * 6];
	BYTE byElementID = m_chUnPackedBuff[C_N_ELEMENT_ID_POS + byIndex * 6];

	if (byObjectID == PATTERN_COUNT)
	{
		return QueryPatternCount(byIndex, tCorrectReturnData, tWrongReturnData);
	}
	else if (byObjectID == PATTERN_CONFIGTABLE && byElementID == 0)
	{
		return QueryAllElementPatternConfig(byIndex, tCorrectReturnData, tWrongReturnData);
	}
	else if (byObjectID == PATTERN_CONFIGTABLE)
	{
		if (byAttributeID > PATTERN_STAGE_TYPE)
		{
			CreateWrongQueryReturnData(byIndex, tWrongReturnData);
		}
		else
		{
			QueryPatternConfig(byIndex, tCorrectReturnData, tWrongReturnData);
		}
	}
	else
	{
		CreateWrongQueryReturnData(byIndex, tWrongReturnData);
	}
}

void COpenATCCommWithGB20999Thread::AckCtl_AskTransitionRetain(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData)
{
	BYTE byObjectID = m_chUnPackedBuff[C_N_OBJECT_ID_POS + byIndex * 6];
	BYTE byAttributeID = m_chUnPackedBuff[C_N_ATTRIBUTE_ID_POS + byIndex * 6];
	BYTE byElementID = m_chUnPackedBuff[C_N_ELEMENT_ID_POS + byIndex * 6];

	if (byObjectID == 1 && byElementID == 0)
	{
		return QueryAllElementTransitionRetainConfig(byIndex, tCorrectReturnData, tWrongReturnData);
	}
    else if (byObjectID == 1)//相位阶段过渡约束配置表
	{
		if (byAttributeID > PHASE_TRANSITIONSTAGE_RETAIN)
		{
			CreateWrongQueryReturnData(byIndex, tWrongReturnData);
		}
		else
		{
			QueryTransitionRetainConfig(byIndex, tCorrectReturnData, tWrongReturnData);
		}
	}
	else
	{
		CreateWrongQueryReturnData(byIndex, tWrongReturnData);
	}
}

void COpenATCCommWithGB20999Thread::AckCtl_AskDayPlanInfo(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData)
{
	BYTE byObjectID = m_chUnPackedBuff[C_N_OBJECT_ID_POS + byIndex * 6];
	BYTE byAttributeID = m_chUnPackedBuff[C_N_ATTRIBUTE_ID_POS + byIndex * 6];
	BYTE byElementID = m_chUnPackedBuff[C_N_ELEMENT_ID_POS + byIndex * 6];

	if (byObjectID == DAYPLAN_COUNT)
	{
		return QueryDayPlanCount(byIndex, tCorrectReturnData, tWrongReturnData);
	}
	else if (byObjectID == DAYPLAN_CONFIG && byElementID == 0)
	{
		return QueryAllElementDayPlanConfig(byIndex, tCorrectReturnData, tWrongReturnData);
	}
	else if (byObjectID == DAYPLAN_CONFIG)
	{
		if (byAttributeID > DAYPLAN_TIMESPANACTCHAIN8)
		{
			CreateWrongQueryReturnData(byIndex, tWrongReturnData);
		}
		else
		{
			QueryDayPlanConfig(byIndex, tCorrectReturnData, tWrongReturnData);
		}
	}
	else
	{
		CreateWrongQueryReturnData(byIndex, tWrongReturnData);
	}
}

void  COpenATCCommWithGB20999Thread::AckCtl_AskScheduleTable(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData)
{
	BYTE byObjectID = m_chUnPackedBuff[C_N_OBJECT_ID_POS + byIndex * 6];
	BYTE byAttributeID = m_chUnPackedBuff[C_N_ATTRIBUTE_ID_POS + byIndex * 6];
	BYTE byElementID = m_chUnPackedBuff[C_N_ELEMENT_ID_POS + byIndex * 6];

	if (byObjectID == SCHEDULE_COUNT)
	{
		return QueryScheduleTableCount(byIndex, tCorrectReturnData, tWrongReturnData);
	}
	else if (byObjectID == SCHEDULE_CONFIG_TABLE && byElementID == 0)
	{
		return QueryAllElementScheduleTableConfig(byIndex, tCorrectReturnData, tWrongReturnData);
	}
	else if (byObjectID == SCHEDULE_CONFIG_TABLE)
	{
		if (byAttributeID > SCHEDULE_DAYPLAN_ID)
		{
			CreateWrongQueryReturnData(byIndex, tWrongReturnData);
		}
		else
		{
			QueryScheduleTableConfig(byIndex, tCorrectReturnData, tWrongReturnData);
		}
	}
	else
	{
		CreateWrongQueryReturnData(byIndex, tWrongReturnData);
	}
}

void  COpenATCCommWithGB20999Thread::AckCtl_AskDeviceStatus(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData)
{
	BYTE byObjectID = m_chUnPackedBuff[C_N_OBJECT_ID_POS + byIndex * 6];
	BYTE byAttributeID = m_chUnPackedBuff[C_N_ATTRIBUTE_ID_POS + byIndex * 6];
	BYTE byElementID = m_chUnPackedBuff[C_N_ELEMENT_ID_POS + byIndex * 6];

	if (byAttributeID > LOCAL_TIME)
	{
		CreateWrongQueryReturnData(byIndex, tWrongReturnData);
	}
	else
	{
		QueryDeveiceStatus(0, tCorrectReturnData, tWrongReturnData);
	}
}

void  COpenATCCommWithGB20999Thread::AckCtl_AskRunStatus(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData)
{
	BYTE byObjectID = m_chUnPackedBuff[C_N_OBJECT_ID_POS + byIndex * 6];
	BYTE byAttributeID = m_chUnPackedBuff[C_N_ATTRIBUTE_ID_POS + byIndex * 6];
	BYTE byElementID = m_chUnPackedBuff[C_N_ELEMENT_ID_POS + byIndex * 6];

	if (byAttributeID > ROAD_STAGE)
	{
		CreateWrongQueryReturnData(byIndex, tWrongReturnData);
	}
	else
	{
		QueryControlStatus(byIndex, tCorrectReturnData, tWrongReturnData);
	}
}

void  COpenATCCommWithGB20999Thread::AckCtl_AskTrafficData(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData)
{
	BYTE byObjectID = m_chUnPackedBuff[C_N_OBJECT_ID_POS + byIndex * 6];
	BYTE byAttributeID = m_chUnPackedBuff[C_N_ATTRIBUTE_ID_POS + byIndex * 6];
	BYTE byElementID = m_chUnPackedBuff[C_N_ELEMENT_ID_POS + byIndex * 6];

	if (byObjectID == STATISTIC_DATA && byElementID == 0)
	{
		return QueryAllElementStatisticData(byIndex, tCorrectReturnData, tWrongReturnData);
	}
	else if (byObjectID == REALTIME_DATA)
	{
		QueryRealTimeData(byIndex, tCorrectReturnData, tWrongReturnData);
	}
	else if (byObjectID == STATISTIC_DATA)
	{
		QueryStatisticData(byIndex, tCorrectReturnData, tWrongReturnData);
	}
	else
	{
		CreateWrongQueryReturnData(byIndex, tWrongReturnData);
	}
}

void  COpenATCCommWithGB20999Thread::AckCtl_AskAlarmData(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData)
{
	BYTE byObjectID = m_chUnPackedBuff[C_N_OBJECT_ID_POS + byIndex * 6];
	BYTE byAttributeID = m_chUnPackedBuff[C_N_ATTRIBUTE_ID_POS + byIndex * 6];
	BYTE byElementID = m_chUnPackedBuff[C_N_ELEMENT_ID_POS + byIndex * 6];

	if (byObjectID == ALARM_COUNT)
	{
		QueryAlarmDataCount(byIndex, tCorrectReturnData, tWrongReturnData);
	}
	else if (byObjectID == ALARMDATA_TABLE && byElementID == 0 && byAttributeID == 0)
	{
		QueryAllAlarmData(byIndex, tCorrectReturnData, tWrongReturnData);
	}
	else if (byObjectID == ALARMDATA_TABLE && byElementID == 0)
	{
		QueryAllElementAlarmData(byIndex, tCorrectReturnData, tWrongReturnData);
	}
	else if (byObjectID == ALARMDATA_TABLE)
	{
		QueryAlarmData(byIndex, tCorrectReturnData, tWrongReturnData);
	}
	else
	{
		CreateWrongQueryReturnData(byIndex, tWrongReturnData);
	}
}

void  COpenATCCommWithGB20999Thread::AckCtl_AskFaultData(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData)
{
	BYTE byObjectID = m_chUnPackedBuff[C_N_OBJECT_ID_POS + byIndex * 6];
	BYTE byAttributeID = m_chUnPackedBuff[C_N_ATTRIBUTE_ID_POS + byIndex * 6];
	BYTE byElementID = m_chUnPackedBuff[C_N_ELEMENT_ID_POS + byIndex * 6];

	if (byObjectID == FAULT_COUNT)
	{
		QueryFaultDataCount(byIndex, tCorrectReturnData, tWrongReturnData);
	}
	else if	(byObjectID == FAULT_RECORD && byElementID == 0 && byAttributeID == 0)
	{
		QueryAllFaultData(byIndex, tCorrectReturnData, tWrongReturnData);
	}
	else if (byObjectID == FAULT_RECORD && byElementID == 0)
	{
		QueryAllElementFaultData(byIndex, tCorrectReturnData, tWrongReturnData);
	}
	else if (byObjectID == FAULT_RECORD)
	{
		if (byAttributeID > FAULT_PARAM)
		{
			CreateWrongQueryReturnData(byIndex, tWrongReturnData);
		}
		else
		{
			QueryFaultData(byIndex, tCorrectReturnData, tWrongReturnData);
		}
	}
	else
	{
		CreateWrongQueryReturnData(byIndex, tWrongReturnData);
	}
}

void COpenATCCommWithGB20999Thread::AckCtl_AskCenterControl(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData)
{
	BYTE byObjectID = m_chUnPackedBuff[C_N_OBJECT_ID_POS + byIndex * 6];
	BYTE byAttributeID = m_chUnPackedBuff[C_N_ATTRIBUTE_ID_POS + byIndex * 6];
	BYTE byElementID = m_chUnPackedBuff[C_N_ELEMENT_ID_POS + byIndex * 6];

	if (byObjectID == 1)//中心控制表
	{
		if (byAttributeID == CENTERCONTROL_ROADID || byAttributeID > CENTERCONTROL_RUNMODE)
		{
			CreateWrongQueryReturnData(byIndex, tWrongReturnData);
		}
		else
		{
			QueryCentreControlTable(0, tCorrectReturnData, tWrongReturnData);
		}
	}
	else
	{
		CreateWrongQueryReturnData(byIndex, tWrongReturnData);
	}
}

void COpenATCCommWithGB20999Thread::AckCtl_AskOrderPipe(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData)
{
	BYTE byObjectID = m_chUnPackedBuff[C_N_OBJECT_ID_POS + byIndex * 6];
	BYTE byAttributeID = m_chUnPackedBuff[C_N_ATTRIBUTE_ID_POS + byIndex * 6];
	BYTE byElementID = m_chUnPackedBuff[C_N_ELEMENT_ID_POS + byIndex * 6];

	if (byObjectID == 1)//命令
	{
		QueryOrderPipeTable(0, tCorrectReturnData, tWrongReturnData);
	}
	else if (byObjectID == 2)//交易跟踪
	{
		if (byAttributeID > 3)
		{
			CreateWrongQueryReturnData(byIndex, tWrongReturnData);
		}
		else if (byAttributeID == 1 || byAttributeID == 2 || byAttributeID == 3)
		{
			TDataConfig tDataConfig;
			tDataConfig.m_byIndex = byIndex + 1;
			tDataConfig.m_byDataClassID = m_chUnPackedBuff[C_N_DATACLASS_ID_POS + byIndex * 6];
			tDataConfig.m_byObjectID = m_chUnPackedBuff[C_N_OBJECT_ID_POS + byIndex * 6];
			tDataConfig.m_byAttributeID = m_chUnPackedBuff[C_N_ATTRIBUTE_ID_POS + byIndex * 6];
			tDataConfig.m_byElementID = m_chUnPackedBuff[C_N_ELEMENT_ID_POS + byIndex * 6];
			tDataConfig.m_byDataValueLength = 1;
			tDataConfig.m_byDataLength = 4 + tDataConfig.m_byDataValueLength;

			if (byAttributeID == 1)
			{
				printf("m_byDBCreateTransaction:%d\n",m_tDBManagement.m_byDBCreateTransaction);
				//tDataConfig.m_byDataValue[0] = m_tAscParamInfo.m_stCmdPipeInfo.m_byCreateTransaction;
				tDataConfig.m_byDataValue[0] = m_tDBManagement.m_byDBCreateTransaction;
			}
			else if (byAttributeID == 2)
			{
				//tDataConfig.m_byDataValue[0] = m_tAscParamInfo.m_stCmdPipeInfo.m_byVeriyStatus;
				tDataConfig.m_byDataValue[0] = m_tDBManagement.m_byDBVerifyStatus;
			}
			else if (byAttributeID == 3)
			{
				//tDataConfig.m_byDataValue[0] = m_tAscParamInfo.m_stCmdPipeInfo.m_byVeriyError;
				tDataConfig.m_byDataValue[0] = m_tDBManagement.m_byDBVerifyError;
			}

			memcpy(&tCorrectReturnData.m_tDataConfig[tCorrectReturnData.m_byReturnCount], &tDataConfig, sizeof(tDataConfig));
			tCorrectReturnData.m_byReturnCount += 1;
		}
	}
	else
	{
		CreateWrongQueryReturnData(byIndex, tWrongReturnData);
	}
}

void COpenATCCommWithGB20999Thread::AckCtl_SetParamInfo(bool bCorrect, TReturnData & tReturnData)
{
	BYTE byFrameID = m_chUnPackedBuff[C_N_FRAME_ID_POS];
	BYTE byDataID = m_chUnPackedBuff[C_N_DATACLASS_ID_POS];
	BYTE byObjectID = m_chUnPackedBuff[C_N_OBJECT_ID_POS ];
	BYTE byAttributeID = m_chUnPackedBuff[C_N_ATTRIBUTE_ID_POS];
	BYTE byElementID = m_chUnPackedBuff[C_N_ELEMENT_ID_POS];

    unsigned int dwSendBuffLen = 0;
	memset(m_chSendBuff, 0x00, SEND_BUFFER_SIZE);

	short nAgreementVersion = C_N_AGREEMENT_VERSION;
    int   nATCID = htonl(m_pOpenATCParameter->GetSiteID());
    
	m_chSendBuff[dwSendBuffLen++] = 0;                                      //报文长度(先默认为0，后面再计算)  
	m_chSendBuff[dwSendBuffLen++] = 0;                                      //报文长度(先默认为0，后面再计算)  
	memcpy(m_chSendBuff + dwSendBuffLen, &nAgreementVersion, 2);            //协议版本
	dwSendBuffLen += 2;
	m_chSendBuff[dwSendBuffLen++] = C_N_HOST_ID;				            //上位机ID
	memcpy(m_chSendBuff + dwSendBuffLen, &nATCID, 4);			            //信号机ID
	dwSendBuffLen += 4;
	m_chSendBuff[dwSendBuffLen++] = C_N_ROAD_ID;							//路口ID
	m_chSendBuff[dwSendBuffLen++] = byFrameID;			                    //帧流水号
	if (bCorrect)
	{
		m_chSendBuff[dwSendBuffLen++] = FRAME_TYPE_SET_REPLY;               //帧类型
	}
	else
	{
		m_chSendBuff[dwSendBuffLen++] = FRAME_TYPE_SET_ERRORREPLY;          //帧类型
	}
	m_chSendBuff[dwSendBuffLen++] = tReturnData.m_byReturnCount;            //数据值数量
	for (int i = 0;i < tReturnData.m_byReturnCount;i++)
	{
		m_chSendBuff[dwSendBuffLen++] = tReturnData.m_tDataConfig[i].m_byIndex;	      //数据值索引
		m_chSendBuff[dwSendBuffLen++] = 5;                                            //数据值长度    
		m_chSendBuff[dwSendBuffLen++] = tReturnData.m_tDataConfig[i].m_byDataClassID; //数据类ID
		m_chSendBuff[dwSendBuffLen++] = tReturnData.m_tDataConfig[i].m_byObjectID;	  //对象ID
		m_chSendBuff[dwSendBuffLen++] = tReturnData.m_tDataConfig[i].m_byAttributeID; //属性ID
		m_chSendBuff[dwSendBuffLen++] = tReturnData.m_tDataConfig[i].m_byElementID;	  //元素ID
		m_chSendBuff[dwSendBuffLen++] = tReturnData.m_tDataConfig[i].m_byDataValue[0];//数据值
	}

	unsigned short nCrc16 = Crc16(m_chSendBuff, dwSendBuffLen);    
	nCrc16 = htons(nCrc16);
	memcpy(m_chSendBuff + dwSendBuffLen, &nCrc16, 2);                       //校验
	dwSendBuffLen += 2;

	short nPacketLen = htons(dwSendBuffLen);
	memcpy(m_chSendBuff, &nPacketLen, 2);	                               //报文长度

	int nRet = 0;
	SendAckToPeer(dwSendBuffLen, nRet);

	m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "AckCtl_SetParamInfo FrameID:%d", byFrameID); 
}

////私有类：跟随相位 --HPH 2021.12.06
//void  COpenATCCommWithGB20999Thread::AckCtl_AskPrivateData(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData)
//{
//	BYTE byObjectID = m_chUnPackedBuff[C_N_OBJECT_ID_POS + byIndex * 6];
//	BYTE byAttributeID = m_chUnPackedBuff[C_N_ATTRIBUTE_ID_POS + byIndex * 6];
//	BYTE byElementID = m_chUnPackedBuff[C_N_ELEMENT_ID_POS + byIndex * 6];
//
//	if (byObjectID == OVERLAP_COUNT)
//	{
//		return QueryOverlapCount(byIndex, tCorrectReturnData, tWrongReturnData);
//	}
//	else if (byObjectID == OVERLAP_CONFIG && byElementID == 0)
//	{
//		return QueryAllElementOverlapConfig(byIndex, tCorrectReturnData, tWrongReturnData);
//	}
//	else if (byObjectID == OVERLAP_CONFIG)
//	{
//		if (byAttributeID > OVERLAP_PULSETYPE)
//		{
//			CreateWrongQueryReturnData(byIndex, tWrongReturnData);
//		}
//		else
//		{
//			QueryOverlapConfig(byIndex, tCorrectReturnData, tWrongReturnData);
//		}
//	}
//	else
//	{
//		CreateWrongQueryReturnData(byIndex, tWrongReturnData);
//	}
//}
////私有类：跟随相位 --HPH 2021.12.06

void* COpenATCCommWithGB20999Thread::DBDataProessThread(void *pParam)
{
    COpenATCCommWithGB20999Thread *pThis = (COpenATCCommWithGB20999Thread *)pParam;
	
    int nRet = pThis->RunDBDataProessThread();
    return (void *)nRet;
}

int COpenATCCommWithGB20999Thread::RunDBDataProessThread()
{
	//循环等待命令管道--HPH
	m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "RunDBDataProessThread start!");
	while(!m_bExitFlag)
	{
		/*2022.06.27事务部分重构 --HPH*/
		if (m_tDBManagement.m_byDBCreateTransaction == VERIFYING)
		{
			int ErrorNum = 0;
			//ErrorNum = m_openATCParamCheck.CheckVerifyParam(m_tVerifyParamInfo);
			
			if(ErrorNum == 0)
			{
				m_pOpenATCParameter->Gb20999ToGb25280(m_tParamInfo, m_tVerifyParamInfo);
				m_pOpenATCParameter->SetAscTempParamInfo(m_tParamInfo);//设置参数信息
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "20999 To 25280 have done!");
				m_tDBManagement.m_byDBCreateTransaction = DONE;
				m_tDBManagement.m_byDBVerifyStatus = DONEWITHNOERROR;
				//m_pOpenATCParameter->Gb20999ToGb25280(m_tVerifyParamInfo);
				//ErrorNum = m_openATCParamCheck.CheckMinGreenVerifyParam(m_tVerifyParamInfo);
				//if(ErrorNum == 0)
				//{
				//	//m_pOpenATCParameter->SetAscTempParamInfo(m_tParamInfo);//设置参数信息
				//	//m_pOpenATCParameter->SetAscTempParamInfo(m_tVerifyParamInfo);//设置参数信息
				//	m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "20999 To 25280 have done!");
				//	m_tDBManagement.m_byDBCreateTransaction = DONE;
				//	m_tDBManagement.m_byDBVerifyStatus = DONEWITHNOERROR;
				//}
				//else
				//{
				//	m_tDBManagement.m_byDBCreateTransaction = DONE;
				//	m_tDBManagement.m_byDBVerifyError = 0x10;	//值错误
				//	m_tDBManagement.m_byDBVerifyStatus = DONEWITHERROR;
				//	//AddFaultInfo(TYPE_FAULT_CONFIG, SWITCH_NULL);
				//}
			}
			else
			{
				m_tDBManagement.m_byDBCreateTransaction = DONE;
				m_tDBManagement.m_byDBVerifyError = 0x10;	//值错误
				m_tDBManagement.m_byDBVerifyStatus = DONEWITHERROR;
				//AddFaultInfo(TYPE_FAULT_CONFIG, SWITCH_NULL);
			}

			/*测试时方便写入打开，其余时候关闭 顶*/
			//将缓存写入数据库（这里是写入json）
			//m_pOpenATCParameter->SaveSetParameter(false);
			//m_tDBManagement.m_byDBCreateTransaction = NORMAL;
			/*测试时方便写入打开，其余时候关闭 底*/
		}

		//这部分内容为20999时间回滚
//#ifndef _WIN32
//		tzset();  
//		time_t current_timet;
//		time(&current_timet);//得到当前时间秒数  
//		localtime_r(&current_timet, &Local_tm);//得到GMT，即UTC时间
//		m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "tm_year:%d, tm_mon:%d, tm_mday:%d, tm_hour:%d, tm_min:%d, tm_sec:%d",Local_tm.tm_year,Local_tm.tm_mon,Local_tm.tm_mday,Local_tm.tm_hour,Local_tm.tm_min,Local_tm.tm_sec);
//		if(Local_tm.tm_year == 136 && Local_tm.tm_mon == 11 && Local_tm.tm_mday == 31 && Local_tm.tm_hour == 23 && Local_tm.tm_min == 59 && Local_tm.tm_sec == 59)
//		{
//			time_t timep;
//			struct tm *newLocal_tm = new struct tm();
//			newLocal_tm->tm_year	= 70;
//			newLocal_tm->tm_mon		= 4;
//			newLocal_tm->tm_mday	= 1;
//			newLocal_tm->tm_hour	= 0;
//			newLocal_tm->tm_min		= 0;
//			newLocal_tm->tm_sec		= 0;
//			newLocal_tm->tm_isdst	= -1;
//			timep = mktime(newLocal_tm);
//			stime(&timep);
//			delete newLocal_tm;
//		}
//#endif
		OpenATCSleep(100);
		/*2022.06.27事务部分重构 --HPH*/
	}
	return 1;
}

BYTE COpenATCCommWithGB20999Thread::TransStageStatus(BYTE byStageStatus)
{
	BYTE nResult = 0;
	
	if(byStageStatus == C_CH_PHASESTAGE_G)
	{
		nResult = 0x20;
	}
	else if(byStageStatus == C_CH_PHASESTAGE_F || byStageStatus == C_CH_PHASESTAGE_U)
	{
		nResult = 0x10;
	}
	else
	{
		nResult = 0x30;
	}

	return nResult;
}

void COpenATCCommWithGB20999Thread::GetStageRunTime(short &tRunTime, short &tRemainTime, int StageIndex, TPhaseRunStatus tPhaseRunStatus)
{
	//int PowerOnTime = m_tAscParamInfo.m_stAscPhaseTable[0].m_nPowerOnGetControlLampOneTime + m_tAscParamInfo.m_stAscPhaseTable[0].m_nPowerOnGetControlLampTwoTime + m_tAscParamInfo.m_stAscPhaseTable[0].m_nPowerOnGetControlLampThreeTime;
	//int GetControlTime = m_tAscParamInfo.m_stAscPhaseTable[0].m_nGetControlLampOneTime + m_tAscParamInfo.m_stAscPhaseTable[0].m_nGetControlLampTwoTime + m_tAscParamInfo.m_stAscPhaseTable[0].m_nGetControlLampThreeTime;
	//if (tPhaseRunStatus.m_atRingRunStatus[0].m_nCurStageIndex == StageIndex)
	//{
	//	int nRunTime = 0;
	//	if (StageIndex == 0)
	//	{
	//		tRunTime = tPhaseRunStatus.m_nCycleRunTime;
	//		tRemainTime = tPhaseRunStatus.m_atRingRunStatus[0].m_atPhaseStatus[StageIndex].m_nSplitTime - tRunTime;
	//		if(m_pOpenATCRunStatus->GetIsInPowerOnFirstCycle())
	//		{
	//			tRemainTime = tPhaseRunStatus.m_atRingRunStatus[0].m_atPhaseStatus[StageIndex].m_nSplitTime + PowerOnTime - GetControlTime - tRunTime;
	//		}
	//	}
	//	else
	//	{
	//		for(int i = 0; i < StageIndex; i++)
	//		{
	//			nRunTime += tPhaseRunStatus.m_atRingRunStatus[0].m_atPhaseStatus[i].m_nSplitTime;
	//		}

	//		if(m_pOpenATCRunStatus->GetIsInPowerOnFirstCycle())		//如果为开机第一周期
	//		{
	//			nRunTime = nRunTime + PowerOnTime - GetControlTime;
	//		}
	//		tRunTime = tPhaseRunStatus.m_nCycleRunTime - nRunTime;
	//		tRemainTime = tPhaseRunStatus.m_atRingRunStatus[0].m_atPhaseStatus[StageIndex].m_nSplitTime - tRunTime;
	//	}
	//}
	//else if (tPhaseRunStatus.m_atRingRunStatus[0].m_nCurStageIndex > StageIndex)
	//{
	//	tRunTime = tPhaseRunStatus.m_atRingRunStatus[0].m_atPhaseStatus[StageIndex].m_nSplitTime;
	//	tRemainTime = 0;
	//	if(StageIndex == 0 && m_pOpenATCRunStatus->GetIsInPowerOnFirstCycle())
	//	{
	//		tRunTime = tPhaseRunStatus.m_atRingRunStatus[0].m_atPhaseStatus[StageIndex].m_nSplitTime + PowerOnTime - GetControlTime;
	//	}
	//}
	//else
	//{
	//	tRunTime = 0;
	//	tRemainTime = tPhaseRunStatus.m_atRingRunStatus[0].m_atPhaseStatus[StageIndex].m_nSplitTime;
	//	if(StageIndex == 0 && m_pOpenATCRunStatus->GetIsInPowerOnFirstCycle())
	//	{
	//		tRemainTime = tPhaseRunStatus.m_atRingRunStatus[0].m_atPhaseStatus[StageIndex].m_nSplitTime + PowerOnTime - GetControlTime;
	//	}
	//}
}

void COpenATCCommWithGB20999Thread::GetRoadTwoStageRunTime(short &tRunTime, short &tRemainTime, unsigned char & StageStatus, int StageIndex)
{
	////设置需要查询的阶段号
	//TRoadTwoStageNum tRoadTwoStageNum;
	//memset(&tRoadTwoStageNum,0,sizeof(tRoadTwoStageNum));
	//tRoadTwoStageNum.m_nStageNum = StageIndex;
	//m_pOpenATCRunStatus->SetRoadTwoStageNum(tRoadTwoStageNum);
	////设置查询标志
	//m_pOpenATCRunStatus->SetIsNeedSendAskRunTime(true);
	//OpenATCSleep(100);

	//while(!m_pOpenATCRunStatus->GetHaveRecvRunTime())
	//{
	//	//printf("wait!\n");
	//}

	//TRoadTwoStageRunTime tRoadTwoStageRunTime;
	//memset(&tRoadTwoStageRunTime,0,sizeof(tRoadTwoStageRunTime));
	//m_pOpenATCRunStatus->GetRoadTwoStageRunTime(tRoadTwoStageRunTime);

	//tRunTime = tRoadTwoStageRunTime.m_nRunTime;
	//tRemainTime = tRoadTwoStageRunTime.m_nRemainTime;
	//StageStatus = tRoadTwoStageRunTime.m_nStageType;

	//m_pOpenATCRunStatus->SetHaveRecvRunTime(false);
}

void  COpenATCCommWithGB20999Thread::AddFaultInfo(BYTE FalutType, BYTE FalutAction)
{
//	int timeIndex;
//	TTestFaultStatus tTestFaultStatus;
//	m_pOpenATCRunStatus->GetTestFaultInfo(tTestFaultStatus);
//
//	tTestFaultStatus.m_nFaultNum += 1;
//	if(tTestFaultStatus.m_nFaultNum > 16)
//	{
//		for(int i = 0; i< 15; i++)
//		{
//			memcpy(&tTestFaultStatus.m_tFaultInfo[i], &tTestFaultStatus.m_tFaultInfo[i + 1], sizeof(TTestFaultInfo));
//		}
//		tTestFaultStatus.m_nFaultNum = 16;
//	}
//
//	tTestFaultStatus.m_tFaultInfo[tTestFaultStatus.m_nFaultNum - 1].m_nFaultNum = tTestFaultStatus.m_nFaultNum;
//	tTestFaultStatus.m_tFaultInfo[tTestFaultStatus.m_nFaultNum - 1].m_wFaultType = FalutType;
//	tTestFaultStatus.m_tFaultInfo[tTestFaultStatus.m_nFaultNum - 1].m_unFaultAction = FalutAction;
//#ifndef _WIN32
//	tm	Fault_tm;	//linux报警时间--HPH
//	time_t current_timet;  
//	time(&current_timet);//得到当前时间秒数   
//	localtime_r(&current_timet, &Fault_tm);//得到GMT，即UTC时间
//	short nYear = ntohs(Fault_tm.tm_yday);
//	memcpy(tTestFaultStatus.m_tFaultInfo[tTestFaultStatus.m_nFaultNum - 1].m_wFaultTime, &nYear, 2);						//年
//	tTestFaultStatus.m_tFaultInfo[tTestFaultStatus.m_nFaultNum - 1].m_wFaultTime[2] = Fault_tm.tm_mon;
//	tTestFaultStatus.m_tFaultInfo[tTestFaultStatus.m_nFaultNum - 1].m_wFaultTime[3] = Fault_tm.tm_mday;
//	tTestFaultStatus.m_tFaultInfo[tTestFaultStatus.m_nFaultNum - 1].m_wFaultTime[4] = Fault_tm.tm_hour;
//	tTestFaultStatus.m_tFaultInfo[tTestFaultStatus.m_nFaultNum - 1].m_wFaultTime[5] = Fault_tm.tm_min;
//	tTestFaultStatus.m_tFaultInfo[tTestFaultStatus.m_nFaultNum - 1].m_wFaultTime[6] = Fault_tm.tm_sec;
//#else
//	SYSTEMTIME tFaultTime;
//	GetLocalTime(&tFaultTime);    
//	short nYear = ntohs(tFaultTime.wYear);
//	memcpy(tTestFaultStatus.m_tFaultInfo[tTestFaultStatus.m_nFaultNum - 1].m_wFaultTime, &nYear, 2);						//年
//	tTestFaultStatus.m_tFaultInfo[tTestFaultStatus.m_nFaultNum - 1].m_wFaultTime[2] = tFaultTime.wMonth;
//	tTestFaultStatus.m_tFaultInfo[tTestFaultStatus.m_nFaultNum - 1].m_wFaultTime[3] = tFaultTime.wDay;
//	tTestFaultStatus.m_tFaultInfo[tTestFaultStatus.m_nFaultNum - 1].m_wFaultTime[4] = tFaultTime.wHour;
//	tTestFaultStatus.m_tFaultInfo[tTestFaultStatus.m_nFaultNum - 1].m_wFaultTime[5] = tFaultTime.wMinute;
//	tTestFaultStatus.m_tFaultInfo[tTestFaultStatus.m_nFaultNum - 1].m_wFaultTime[6] = tFaultTime.wSecond;
//#endif 
//
//	m_pOpenATCRunStatus->SetTestFaultInfo(tTestFaultStatus);
}

void  COpenATCCommWithGB20999Thread::GetSystemTimeZone(int & nTimeZoneHour, int & nTimeZoneMinute)
{
	nTimeZoneHour	= m_tAscParamInfo.m_stBaseInfo.m_wTimeZone;
	nTimeZoneMinute = 0;
}

void  COpenATCCommWithGB20999Thread::SetSystemTimeZone()
{
	int nHour	= m_tAscParamInfo.m_stBaseInfo.m_wTimeZone/3600;	
	int nMinute = 0;

#ifndef _WIN32
	char *szTime = new char[16];
	if (nHour < 0)
	{
		sprintf(szTime, "GMT+%02d:%02d", labs(nHour), nMinute);
	}
	else
	{
		sprintf(szTime, "GMT-%02d:%02d", nHour, nMinute);
	}
	setenv("TZ", szTime, 1);
	tzset();

	m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "############ COpenATCCommWithGB20999Thread szTime[%s].", szTime);
#endif
}