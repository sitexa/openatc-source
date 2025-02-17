/*====================================================================
模块名 ：和客户端配置软件交互的通信接口
文件名 ：OpenATCCommWithCfgSWThread.cpp
相关文件：OpenATCCommWithCfgSWThread.h
实现功能：和客户端配置进行交互
作者 ：李永萍
版权 ：<Copyright(C) 2019-2020 Suzhou Keda Technology Co., Ltd. All rights reserved.>
---------------------------------------------------------------------------------------------------------------------
修改记录：
日 期           版本    修改人             走读人             修改记录
2019/09/06      V1.0    李永萍             李永萍             创建模块
====================================================================*/
#include "OpenATCCommWithCfgSWThread.h"

#ifdef _WIN32
#	include <io.h>
#else
#	include <unistd.h>
#endif

#include "../OpenATCFlowProcManager.h"
#include "../OpenATCFlowProcLog.h"
#include "../../Include/OpenATCFaultProcManager.h"
#include "OpenATCMd5.h"

COpenATCCommWithCfgSWThread::COpenATCCommWithCfgSWThread()
{
	m_hThread		= 0;
    m_dwThreadRet	= 0;
	m_bExitFlag		= true;
    m_nDetachState	= 0;

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

	m_commHelper.SetRecvTimeOut(400);
	m_commHelper.SetSendTimeOut(400);

    m_msgBody   = NULL;
    m_chOutBuff = NULL;

    m_tAreaInfo.m_chAscRegionNo = 0;
	m_tAreaInfo.m_usAscRoadNo	= 0;

	m_nCfgPort = 8880;
	m_isCfgIP = false;


    memset(m_atNetConfig, 0, sizeof(m_atNetConfig));

	m_clientSock.SetSocketType(SOCK_STREAM);
    m_clientSock.SetComType(TCP_SERVICE);

	memset(m_szClientaIp, 0, sizeof(m_szClientaIp));  
    m_nClientPort = 0;

	m_chSendDetectorChgData = 0;
	memset(&m_tOldVehDetBoardData,0,sizeof(m_tOldVehDetBoardData));

	m_nLastTelnetOpenTime = 0;
	memset(&m_tCurTelnetCtrlStatus,0,sizeof(m_tCurTelnetCtrlStatus));

	memset(&m_atOldWorkModeParam,0,sizeof(m_atOldWorkModeParam));

	memset(m_nOldChannelLockStatus,0,sizeof(m_nOldChannelLockStatus));

	m_chSecretKeyBuff  = new unsigned char[PACKED_BUFFER_SIZE];
	memset(m_chSecretKeyBuff, 0x00, PACKED_BUFFER_SIZE);

	m_lastGetDeviceParam = time(NULL);
}

COpenATCCommWithCfgSWThread::~COpenATCCommWithCfgSWThread()
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
        delete []m_chSendBuff;
        m_chSendBuff = NULL;
    }

    if (m_chPackedBuff != NULL)
    {
        delete []m_chPackedBuff;
        m_chPackedBuff = NULL;
    }

    if (m_msgBody != NULL)
    {
        cJSON_Delete(m_msgBody);
        m_msgBody = NULL;
    }

    if (m_chOutBuff != NULL)
    {
        free(m_chOutBuff);
        m_chOutBuff = NULL;
    }

	if (m_byComType == UDP_SERVICE)
	{
		m_commHelper.Close();
	}
	else
	{
		m_clientSock.Close();
	}

	if (m_chSecretKeyBuff != NULL)
    {
        delete []m_chSecretKeyBuff;
        m_chSecretKeyBuff = NULL;
    }
}

int COpenATCCommWithCfgSWThread::Run()
{
#ifndef _WIN32
	prctl(15,"CommWithCfgSWThread",0,0,0);
#endif
	int				nRecvLength = 0;
    unsigned int	nPackLength = 0;
    int				nRet		= 0;
	bool			bFlag		= false;

	char szPeerIp[20] = {0};

	char			szInfo[255] = {0};

    m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "COpenATCCommWithCfgSWThread start!");

	while (!m_bExitFlag)
    {
		if (m_byComType == UDP_SERVICE)
		{
			if (m_commHelper.AcceptConnection("0.0.0.0.", m_nCfgPort, m_isCfgIP) == OPENATC_RTN_FAILED)
			{
				OpenATCSleep(500);
				continue;
			}
			else
			{
				m_packerUnPacker.ClearBuff();
			}
		}
		while (true)
		{
			long lCurTime = time(NULL);
			if (m_tCurTelnetCtrlStatus.m_byControlType == TELNET_ON && m_tCurTelnetCtrlStatus.m_wControlTime > 0 && labs(lCurTime - m_nLastTelnetOpenTime) > m_tCurTelnetCtrlStatus.m_wControlTime)
			{
#ifndef VIRTUAL_DEVICE
				//关闭telnet服务
				if (system("./TelnetCtrl off") < 0)
				{
					m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "######### AutoCtrl telnet off failed");
				}
				else
				{
					m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "######### AutoCtrl telnet off success");
					m_tCurTelnetCtrlStatus.m_byControlType = TELNET_OFF;
					m_tCurTelnetCtrlStatus.m_wControlTime  = 0;
				}
#endif
			}

			nRecvLength = 0;
			memset(szPeerIp, 0x00, sizeof(szPeerIp));
			if (m_byComType == UDP_SERVICE)
			{
				if (m_commHelper.Read(m_chRecvBuff, RECV_BUFFER_SIZE, nRecvLength, bFlag, szPeerIp) == OPENATC_RTN_OK)
				{
					//m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "COpenATCCommWithCfgSWThread recv data > 0 IP:%s  RecvLength:%d!", szPeerIp, nRecvLength);
					m_packerUnPacker.Write((unsigned char *)(char *)m_chRecvBuff, nRecvLength);

					m_pOpenATCRunStatus->SetComStatusWithCfg(true, false);
				}
				else
				{
					m_pOpenATCLog->LogOneMessage(LEVEL_INFO, "COpenATCCommWithCfgSWThread recv data <= 0 IP:%s RecvLength:%d!", szPeerIp, nRecvLength);
					m_pOpenATCRunStatus->SetComStatusWithCfg(false, false);
					OpenATCSleep(100);   
				}

				SendDetectorChgData();
			}
			else
			{   
				nRecvLength = m_clientSock.Read((char *)m_chRecvBuff, RECV_BUFFER_SIZE, 400);
				if (nRecvLength > 0)
				{
					m_packerUnPacker.Write((unsigned char *)(char *)m_chRecvBuff, nRecvLength);

					m_pOpenATCRunStatus->SetComStatusWithCfg(false, true);
				}
				else
				{
					if (nRecvLength == -1)
					{
						m_pOpenATCRunStatus->SetComStatusWithCfg(false, false);
					}
					OpenATCSleep(100);   
				}
			}
			static long  m_nLastReadDataTime = 0;
			if (nRecvLength > 0)
			{
				nPackLength = 0;
			    nRet = m_packerUnPacker.Read(m_chUnPackedBuff, nPackLength);
                if (nRet == ReadOk)
                {
                    ParserPack(m_chUnPackedBuff, nPackLength, szPeerIp);
                }
                else if (nRet != ReadNoData)
                {
                    //m_pOpenATCLog->LogOneMessage(LEVEL_ERROR, "COpenATCCommWithCfgSWThread unpacker read err!");
                }
				m_nLastReadDataTime = time(NULL);
            }
			else
			{
				long lCurTime = time(NULL);
				if (labs(lCurTime - m_nLastReadDataTime) > 30 && m_nLastReadDataTime != 0)
				{
					m_nLastReadDataTime = lCurTime;

					TManualCmd  tValidManualCmd;
					memset(&tValidManualCmd,0,sizeof(tValidManualCmd));
					m_pOpenATCRunStatus->GetValidManualCmd(tValidManualCmd); 

					if (tValidManualCmd.m_nCurCtlSource == CTL_SOURCE_SYSTEM && tValidManualCmd.m_nCtlMode != CTL_MODE_SELFCTL)
					{
						CreateManualCmdReturnToSelf(szPeerIp);
						m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "COpenATCCommWithCfgSWThread ConfigSoftware offline, ATC return to AutoSelf!");
					}
				}
			}

			GetDeviceParam();
        }

		m_commHelper.Close();	
    }

	m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "COpenATCCommWithCfgSWThread exit!");

    return OPENATC_RTN_OK;
}

void  COpenATCCommWithCfgSWThread::Init(COpenATCParameter *pParameter, COpenATCRunStatus *pRunStatus, COpenATCLog * pOpenATCLog, BYTE byComType, int nCfgPort, const char * pOpenATCVersion)
{
	m_pOpenATCParameter = pParameter;
	m_pOpenATCRunStatus = pRunStatus;
	m_pOpenATCLog       = pOpenATCLog;
	m_byComType         = byComType;

	memset(m_chOpenATCVersion, 0, sizeof(m_chOpenATCVersion));
	memcpy(m_chOpenATCVersion, pOpenATCVersion, strlen(pOpenATCVersion));

#if 1//配置工具版本添加
	//TAscNetCard tAscNetCardTable[MAX_NETCARD_TABLE_COUNT];//信号机两个网卡配置信息，0，网卡1. 1网卡2
	m_pOpenATCParameter->GetNetCardsTable(m_atNetConfig);
	if (strcmp(m_atNetConfig[0].m_chAscNetCardIp,"") == 0)
	{
		m_isCfgIP = true;
	}

#endif
    m_pOpenATCParameter->GetAscAreaInfo(m_tAreaInfo);
    m_pOpenATCParameter->GetNetCardsTable(m_atNetConfig);

	TAscCenter tAscCenter;
    m_pOpenATCParameter->GetCenterInfo(tAscCenter);
	m_commHelper.PreSetCenterPeerIP(tAscCenter.m_chAscCenterIp, m_pOpenATCLog);
#ifdef VIRTUAL_DEVICE
	TAscSimulate tSimulateInfo;
	m_pOpenATCParameter->GetSimulateInfo(tSimulateInfo);
	m_nCfgPort = tSimulateInfo.m_nCfgPort;
#endif
	m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "COpenATCCommWithCfgSWThread CfgPort:%d", m_nCfgPort);

}

int COpenATCCommWithCfgSWThread::Start()
{
    if (false == m_bExitFlag)
	{
        return OPENATC_RTN_OK;
	}

    bool bOK = true;
    m_bExitFlag = false;

#ifdef _WIN32
	m_hThread = CreateThread(NULL, 0, (unsigned long(COMMWITHCFG_CALLBACK *)(void *))&RunThread, this, 0, NULL);
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

int COpenATCCommWithCfgSWThread::Join()
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
		m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "COpenATCCommWithCfgSWThread Join ok!");
	}
	else
	{
		m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "COpenATCCommWithCfgSWThread Join fail!");
	}

	m_bExitFlag = true;
    m_hThread	= 0;

    return nRet;
}

int COpenATCCommWithCfgSWThread::Detach()
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

void* COMMWITHCFG_CALLBACK COpenATCCommWithCfgSWThread::RunThread(void *pParam)
{
    COpenATCCommWithCfgSWThread *pThis = (COpenATCCommWithCfgSWThread *)pParam;

    int nRet = pThis->Run();
    return (void *)nRet;
}

int COpenATCCommWithCfgSWThread::ParserPack(const unsigned char *chUnPackedBuff, unsigned int dwPackLength, char * chPeerIp)
{
    int nRet = OPENATC_RTN_OK;

    switch (chUnPackedBuff[LINKCODE_POS])
    {
	case LINK_CONTROL:
		{
			nRet = ParserPack_CtrlLink(chUnPackedBuff, dwPackLength, chPeerIp);
		}
		break;
	case LINK_CONFIG:
		{
			nRet = ParserPack_CfgLink(chUnPackedBuff, dwPackLength, chPeerIp);
		}
		break;
	case LINK_BASEINFO:
		{
			nRet = ParserPack_BaseInfoLink(chUnPackedBuff, dwPackLength, chPeerIp);
		}
		break;
	case LINK_ALTERNATEFEATUREPARA:
		{
			nRet = ParserPack_AlternateFeatureParaLink(chUnPackedBuff, dwPackLength, chPeerIp);
		}
		break;
	case LINK_INTERVENECOMMAND:
		{
			nRet = ParserPack_InterveneCommandLink(chUnPackedBuff, dwPackLength, chPeerIp);
		}
		break;
	case LINK_OPTIMIZE_CONTROL:
		{
			nRet = ParserPack_OptimizeControlLink(chUnPackedBuff, dwPackLength, chPeerIp);
		}
		break;
	default:
		break;
    }
        
    return nRet;
}

int COpenATCCommWithCfgSWThread::ParserPack_CtrlLink(const unsigned char *chUnPackedBuff, unsigned int dwPackLength, char * chPeerIp)
{
	int  nRet		= OPENATC_RTN_OK;
    int  nPackSize	= 0;
	char szCommand[FRM_MAX_COMMAND_LENGTH + 1] = {0};

	static char   szUpdateFileName[FRM_MAX_FILENAME_LENGTH + 1] = {0};
    static FILE   *s_UpdateFile = NULL; 
   
    switch (chUnPackedBuff[CMDCODE_POS])
    {
	//请求登陆
	case CTL_ASK_LOGIN:
		{
			m_pOpenATCLog->LogOneMessage(LEVEL_INFO, "COpenATCCommWithCfgSWThread receive login ask!");

			int nSize = chUnPackedBuff[DATALEN_POS];
			memcpy(&m_loginInfo, chUnPackedBuff + DATALEN_POS + 1, sizeof(TLoginInfo));
			nPackSize = AckCtl_AskLogin();
		}
		break; 
	//请求流量信息
    case CTL_TRAFFICFLOWINFO:
        {
            m_pOpenATCLog->LogOneMessage(LEVEL_INFO, "COpenATCCommWithCfgSWThread receive ask traffic flow!");

			nPackSize = AckCtl_AskTrafficFlow();
        }
        break;  
	//U盘更新请求
    case CTL_UDISK:
		{
			m_pOpenATCLog->LogOneMessage(LEVEL_INFO, "COpenATCCommWithCfgSWThread receive update Udisk ask!");

			nPackSize = AckCtl_AskUdiskUpdate();
            if (nPackSize > 0)
			{
				COpenATCFlowProcManager::getInstance()->BackUpLogFile(FLOW_FILE_DISK_PATH);
			}
		}
		break; 
  	case CTL_OPERATION_RECORD:
		{
        	m_pOpenATCLog->LogOneMessage(LEVEL_INFO, "COpenATCCommWithCfgSWThread receive operation record ask!");

			nPackSize = AckCtl_AskOperationRecord();
		}
		break;
  	case CTL_CHANNEL_CHECK://通道可检测
		{
		  	m_pOpenATCLog->LogOneMessage(LEVEL_INFO, "COpenATCCommWithCfgSWThread receive channel check ask!");
			
			if (!CheckSecretKey(chUnPackedBuff, dwPackLength, chPeerIp))
			{
				break;
			}

			TAscChannelVerifyInfo atChannelVerifyInfo;
			m_pOpenATCParameter->GetAscChannelVerifyValue((unsigned char *)&chUnPackedBuff[CMDCODE_POS + 6], atChannelVerifyInfo);
			m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "COpenATCCommWithCfgSWThread ChannelCheck ControlMode:%d", atChannelVerifyInfo.m_byControl);
			int nResult = CONTROL_SUCCEED;
			int nFailCode = 0;
			CreateManualCmd(atChannelVerifyInfo.m_byControl, chPeerIp, nResult, nFailCode);
			m_pOpenATCRunStatus->SetChannelCheckInfo(atChannelVerifyInfo);
			nPackSize = AckCtl_AskChannelCheck(nResult, nFailCode);

			//写操作记录日志
			TAscOperationRecord tAscOperationRecord;//操作记录
			memset(&tAscOperationRecord, 0, sizeof(tAscOperationRecord));
			tAscOperationRecord.m_unStartTime = time(NULL);
			tAscOperationRecord.m_unEndTime = time(NULL);
			tAscOperationRecord.m_bySubject = 1;
			tAscOperationRecord.m_byObject = 1;
			tAscOperationRecord.m_nInfoType = SYETEM_CHANNELCHECK;
			if (nResult == CONTROL_SUCCEED)
			{
				tAscOperationRecord.m_bStatus = true;
				sprintf((char *)tAscOperationRecord.m_byFailureValue, "IP is %s! ChannelCheck success", chPeerIp);
			}
			else
			{
				tAscOperationRecord.m_bStatus = false;
				sprintf((char *)tAscOperationRecord.m_byFailureValue, "IP is %s! ChannelCheck fail", chPeerIp);
			}

			m_tOpenATCOperationRecord.SaveOneOperationRecordMessage(&tAscOperationRecord, m_pOpenATCRunStatus);
	 	}
		break;
	 case CTL_VOLUME_LOG://流量日志请求，信号机准备获取流量环境，并应答
		  {

			  m_pOpenATCLog->LogOneMessage(LEVEL_INFO, "COpenATCCommWithCfgSWThread receive Volume  Log ask!");
			  TAscGainTafficFlowCmd atGainTafficFlowCmd;
			  m_pOpenATCParameter->GetAscGainTrafficFlowCmd((unsigned char *)&chUnPackedBuff[CMDCODE_POS + 6], atGainTafficFlowCmd);
			  if (atGainTafficFlowCmd.m_SetUDiskStatus == 1)		//挂载U盘
			  {
				  if (OPENATC_RTN_FAILED == MountUSBDevice(m_pOpenATCRunStatus))
				  {
					  m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "COpenATCCommWithCfgSWThread GainTrafficFlowLog mount usb device failed!");
					  nPackSize = AckCtl_AskVolumeLog(3);
				  }
				  else
				  {
					  m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "COpenATCCommWithCfgSWThread GainTrafficFlowLog mount usb device success!");
					  nPackSize = AckCtl_AskVolumeLog(1);
				  }
			  }
			  else if (atGainTafficFlowCmd.m_SetUDiskStatus == 2)	//卸载U盘
			  {
				  if (OPENATC_RTN_FAILED == UnmountUSBDevice(m_pOpenATCRunStatus))
				  {
					  m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "COpenATCCommWithCfgSWThread GainTrafficFlowLog unmount usb device failed!");
					  nPackSize = AckCtl_AskVolumeLog(4);
				  }
				  else
				  {
					  m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "COpenATCCommWithCfgSWThread GainTrafficFlowLog unmount usb device success!");
					  nPackSize = AckCtl_AskVolumeLog(2);
				  }
			  }
			  else
			  {
				  m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "COpenATCCommWithCfgSWThread GainTrafficFlowLog unusual!");
				  nPackSize = AckCtl_AskVolumeLog(0);
			  }
		  }
		  break;
	 case CTL_PATTERN_INTERRUPT: //方案干预
		  {
			  m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "COpenATCCommWithCfgSWThread receive pattern interrupt ask!");

			  if (!CheckSecretKey(chUnPackedBuff, dwPackLength, chPeerIp))
			  {
				 break;
			  }

			  m_pOpenATCParameter->GetAscInterruptPatternInfo((unsigned char *)&chUnPackedBuff[CMDCODE_POS + 6]);

			  int nResult = CONTROL_SUCCEED;
			  int nFailCode = 0;
			  CreateManualCmd(CTL_MODE_SYS_INTERRUPT, chPeerIp, nResult, nFailCode);
			  
			  nPackSize = AckCtl_AskPatternInterrupt(nResult, nFailCode);

			  //写操作记录日志
			  TAscOperationRecord tAscOperationRecord;//操作记录
			  memset(&tAscOperationRecord, 0, sizeof(tAscOperationRecord));
			  tAscOperationRecord.m_unStartTime = time(NULL);
			  tAscOperationRecord.m_unEndTime = time(NULL);
			  tAscOperationRecord.m_bySubject = 1;
			  tAscOperationRecord.m_byObject = 1;
			  tAscOperationRecord.m_nInfoType = SYETEM_PATTERNINTERRUPT;
			  if (nResult == CONTROL_SUCCEED)
			  {
				  tAscOperationRecord.m_bStatus = true;
				  sprintf((char *)tAscOperationRecord.m_byFailureValue, "IP is %s! Pattern Interrupt success", chPeerIp);
			  }
			  else
			  {
				  tAscOperationRecord.m_bStatus = false;
				  sprintf((char *)tAscOperationRecord.m_byFailureValue, "IP is %s! Pattern Interrupt fail", chPeerIp);
			  }

			  m_tOpenATCOperationRecord.SaveOneOperationRecordMessage(&tAscOperationRecord, m_pOpenATCRunStatus);
		  }
		  break;
     case CTL_HEART_BERAT: //心跳(java服务每10)
		  {
			  m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "COpenATCCommWithCfgSWThread receive java heart!");
			  m_pOpenATCRunStatus->SetJavaHeartTime(time(NULL));
		      //java服务每10秒发一次心跳给主控软件，心跳包无数据内容，收到心跳包就在显示屏上打印配置软件服务在线状态
		  }
		  break;
     case CTL_CHANNEL_STATUS_INFO: //通道状态信息（电压、电流数据）
		  {
			  nPackSize = AckCtl_AskChannelStatus();
		  }
		  break;
	case CTL_CHANNEK_LAMP_STATUS: //通道灯色状态
		  {
			  nPackSize = AckCtl_AskChannelLampStatus();
		  }
		  break;
	case CTL_SYSTEM_REMOTE:
		{
			//系统远程调试查询
			if (chUnPackedBuff[CMDCODE_POS - 1] == ASK_QUERY) 
			{
				nPackSize = AckCtl_AskSystemRemote();
			}
			//系统远程调试设置
			else if (chUnPackedBuff[CMDCODE_POS - 1] == ASK_SET) 
			{
				if (!CheckSecretKey(chUnPackedBuff, dwPackLength, chPeerIp))
				{
					break;
				}

				m_pOpenATCParameter->GetAscRemoteControlValue((unsigned char *)&chUnPackedBuff[CMDCODE_POS + 6], m_tRemoterControl);

				nPackSize = AckCtl_AskSetSystemRemote();	

				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "######### COpenATCCommWithCfgSWThread ParserPack_CtrlLink telnet ctrl. remote:%d, cur:%d.", m_tRemoterControl.m_byControlType, m_tCurTelnetCtrlStatus.m_byControlType);

				if (m_tRemoterControl.m_byControlType == TELNET_OFF && m_tCurTelnetCtrlStatus.m_byControlType == TELNET_ON)
				{
#ifndef _WIN32
					//关闭telnet服务
					if (system("./TelnetCtrl off") < 0)
					{
						m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "######### COpenATCCommWithCfgSWThread ParserPack_CtrlLink telnet off failed");
					}
					else
					{
						m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "######### COpenATCCommWithCfgSWThread ParserPack_CtrlLink telnet off success");
						m_tCurTelnetCtrlStatus.m_byControlType = TELNET_OFF;
						m_tCurTelnetCtrlStatus.m_wControlTime  = 0;
					}
#endif // !_WIN32
				}
				else if (m_tRemoterControl.m_byControlType == TELNET_ON && m_tCurTelnetCtrlStatus.m_byControlType == TELNET_OFF)
				{
#ifndef _WIN32
					if (system("./TelnetCtrl on") < 0)
					{
						m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "######### COpenATCCommWithCfgSWThread ParserPack_CtrlLink telnet on failed");
					}
					else
					{
						m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "######### COpenATCCommWithCfgSWThread ParserPack_CtrlLink telnet on success");
						m_nLastTelnetOpenTime = time(NULL);
						m_tCurTelnetCtrlStatus.m_byControlType = TELNET_ON;
						m_tCurTelnetCtrlStatus.m_wControlTime  = m_tRemoterControl.m_wControlTime;
					}
#endif // !_WIN32	
				}
			}
		}
		break;
	case CTL_UPDATESECRETKEY_STATUS:
		{
			m_pOpenATCLog->LogOneMessage(LEVEL_INFO, "COpenATCCommWithCfgSWThread receive update secretkey!");

			nPackSize = AckCtl_AskUpdateSecretKey();

			
		}
		break;
	default:
		{
            
		}
		break;
    }

    if (nPackSize > 0)
    {
        return SendAckToPeer(nPackSize);
    }

    return nRet;
}

int COpenATCCommWithCfgSWThread::ParserPack_CfgLink(const unsigned char *chUnPackedBuff, unsigned int dwPackLength, char * chPeerIp)
{
	int nRet			= OPENATC_RTN_OK;
    int nPackSize		= 0;
	int nSendDataSize	= 0;

	bool bSaveSuccess = false;

    switch (chUnPackedBuff[CMDCODE_POS])
    {
	//配置软件请求往主机下传数据
	case CFG_ASK_ASKSEND:
		{
			nPackSize = AckCfg_AskSend();
		}
		break;
	//配置软件往主机下传数据
	case CFG_ASK_SENDDATA:
		{
			memcpy(&nSendDataSize, chUnPackedBuff + DATALEN_POS, 4);
 			int nCheckResult = m_openATCParamCheck.CheckAscParam((unsigned char *)&chUnPackedBuff[DATACONTENT_POS]);
            if (nCheckResult == OPENATC_PARAM_CHECK_OK)
			{	
				int nCheckGreenConflict = m_openATCParamCheck.CheckGreenConflictByPanel((unsigned char *)&chUnPackedBuff[DATACONTENT_POS], nSendDataSize);
				if (nCheckGreenConflict == OPENATC_PARAM_CHECK_OK)
				{
					if (!CheckSecretKey(chUnPackedBuff, dwPackLength, chPeerIp))
					{
						break;
					}

					bool bCheckMd5Sta = false;
					bCheckMd5Sta = m_openATCParamCheck.CheckMd5Code((unsigned char *)&chUnPackedBuff[DATACONTENT_POS], nSendDataSize);
					if (bCheckMd5Sta)
					{
						int nSaveParameterRet = m_pOpenATCParameter->SaveParameter((unsigned char *)&chUnPackedBuff[DATACONTENT_POS], nSendDataSize);
						if (nSaveParameterRet == OPENATC_RTN_OK)
						{
							nPackSize = AckCfg_SendDataOK();//配置软件往主机下传数据成功
							m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "OpenATCParamter TZParam Issue Success, SystemControl IP is %s!", chPeerIp);
							
							//此处先不对新参数进行初始化
							TParamRunStatus tParamRunStatus;
							m_pOpenATCRunStatus->GetParamRunStatus(tParamRunStatus);
							tParamRunStatus.m_chIsParameterChg = C_CH_ISPARAMETERCHG_OK;
							if (tParamRunStatus.m_chParameterReady != C_CH_PARAMERREADY_OK)
							{
								m_pOpenATCParameter->Init(m_pOpenATCRunStatus, m_pOpenATCLog);
							}
							m_pOpenATCRunStatus->SetParamRunStatus(tParamRunStatus);

							bSaveSuccess = true;
						}
						else
						{
							TErrInfo tCurErrCode[C_N_MAX_CHECKERR_SIZE] = { 0 };

							if (nSaveParameterRet == OPENATC_SAVE_PARAM_FAILED_USB_NOT_FIND)
							{
								tCurErrCode[0].nCheckErr	= CheckUSBStatus_USB_Not_Find;
								tCurErrCode[0].nSubCheckErr = 0;
							}
							else if (nSaveParameterRet == OPENATC_SAVE_PARAM_FAILED_USB_MOUNT_FAILED)
							{
								tCurErrCode[0].nCheckErr	= CheckUSBStatus_USB_Mount_Fail;
								tCurErrCode[0].nSubCheckErr = 0;
							}
							else if (nSaveParameterRet == OPENATC_SAVE_PARAM_FAILED_PARAM_UPDATE_FAILED)
							{
								tCurErrCode[0].nCheckErr	= CheckParamBaseInfo_Update_Fail;
								tCurErrCode[0].nSubCheckErr = 0;
							}
							else if (nSaveParameterRet == OPENATC_SAVE_PARAM_FAILED_DEVPARAM_UPDATE_FAILED)
							{
								tCurErrCode[0].nCheckErr	= CheckDeviceParamInfo_Update_Fail;
								tCurErrCode[0].nSubCheckErr = 0;
							}

							RecordVerifyErrorToSystemLogFile(tCurErrCode);

							nPackSize = AckCfg_SendCheckDataFailed(tCurErrCode);		//参数保存失败
						}
					}
					else
					{
						TErrInfo tCurErrCode[C_N_MAX_CHECKERR_SIZE] = { 0 };
						m_openATCParamCheck.GetErrCode(tCurErrCode);

						RecordVerifyErrorToSystemLogFile(tCurErrCode);

						nPackSize = AckCfg_SendCheckDataFailed(tCurErrCode);			//MD5校验失败
					}
				}
				else
				{
					TErrInfo tCurErrCode[C_N_MAX_CHECKERR_SIZE] = { 0 };
					m_openATCParamCheck.GetErrCode(tCurErrCode);

					RecordVerifyErrorToSystemLogFile(tCurErrCode);

					nPackSize = AckCfg_SendCheckDataFailed(tCurErrCode);					//手动面板绿冲突校验失败
				}
            }
            else
			{
				//参数解析失败，包含格式错误和值错误
				TErrInfo tCurErrCode[C_N_MAX_CHECKERR_SIZE] = { 0 };
				m_openATCParamCheck.GetErrCode(tCurErrCode);
				if (tCurErrCode[0].nCheckErr == 0)
				{
					tCurErrCode[0].nCheckErr = CheckParamBaseInfo_JSON_Parse_Fail;
					tCurErrCode[0].nSubCheckErr = 0;
				}

				RecordVerifyErrorToSystemLogFile(tCurErrCode);

				nPackSize = AckCfg_SendCheckDataFailed(tCurErrCode);
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

				sprintf((char *)tAscOperationRecord.m_byFailureValue, "IP is %s! SaveParameter success", chPeerIp);

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

				sprintf((char *)tAscOperationRecord.m_byFailureValue, "IP is %s! SaveParameter fail", chPeerIp);

				m_tOpenATCOperationRecord.SaveOneOperationRecordMessage(&tAscOperationRecord, m_pOpenATCRunStatus);
			}
        }
		break;
	//主机上传数据到配置软件
	case CFG_ASK_ASKREAD:
		{
			m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "COpenATCCommWithCfgSWThread Ask Parameter!");
			int nDataSize = 0;
			unsigned char *pBuffer = m_pOpenATCParameter->GetParamData();
			if (pBuffer != NULL)
			{
				nDataSize = strlen((const char *)pBuffer);
			}

			//写操作记录日志
			TAscOperationRecord tAscOperationRecord;//操作记录
			memset(&tAscOperationRecord, 0, sizeof(tAscOperationRecord));
			tAscOperationRecord.m_unStartTime = time(NULL);
			tAscOperationRecord.m_unEndTime = time(NULL);
			tAscOperationRecord.m_bySubject = 1;
			tAscOperationRecord.m_byObject = 1;
			tAscOperationRecord.m_nInfoType = SYSTEM_UPLOADPARAM;

			if (nDataSize > 0)
			{
				//主机上传数据到配置软件成功
				nPackSize = AckCfg_ReadData(pBuffer, nDataSize);
				tAscOperationRecord.m_bStatus = true;
				sprintf((char *)tAscOperationRecord.m_byFailureValue, "IP is %s! UpLoadParameter success", chPeerIp);
			}
			else
			{
				//主机上传数据到配置软件失败
				nPackSize = AckCfg_AskReadFailed();
				tAscOperationRecord.m_bStatus = false;
				sprintf((char *)tAscOperationRecord.m_byFailureValue, "IP is %s! UpLoadParameter fail", chPeerIp);
			}

			m_tOpenATCOperationRecord.SaveOneOperationRecordMessage(&tAscOperationRecord, m_pOpenATCRunStatus);

			if (pBuffer != NULL)
			{
				free(pBuffer);
				pBuffer = NULL;
			}
		}
		break;
	case ASK_QUERY:
		{
			if (m_pOpenATCRunStatus->GetPhaseRunStatusReadFlag())
			{
				m_pOpenATCRunStatus->SetPhaseRunStatusReadFlag(false);
				m_pOpenATCRunStatus->SetPhaseRunStatusWriteFlag(false);
				nPackSize = AckCtl_AskHeart();  
				m_pOpenATCRunStatus->SetPhaseRunStatusReadFlag(true);
				m_pOpenATCRunStatus->SetPhaseRunStatusWriteFlag(true);
			} 
			else 
			{
				OpenATCSleep(100);
				if (m_pOpenATCRunStatus->GetPhaseRunStatusReadFlag())
				{
					m_pOpenATCRunStatus->SetPhaseRunStatusReadFlag(false);
					m_pOpenATCRunStatus->SetPhaseRunStatusWriteFlag(false);
					nPackSize = AckCtl_AskHeart();  
					m_pOpenATCRunStatus->SetPhaseRunStatusReadFlag(true);
					m_pOpenATCRunStatus->SetPhaseRunStatusWriteFlag(true);
				} 
			}
		}
    default:
		{

		}
        break;
    }

    if (nPackSize > 0)
    {
        return SendAckToPeer(nPackSize);
    }

    return nRet; 
}

int COpenATCCommWithCfgSWThread::ParserPack_BaseInfoLink(const unsigned char *chUnPackedBuff, unsigned int dwPackLength, char * chPeerIp)
{
	int nRet = OPENATC_RTN_OK;
    int nPackSize = 0;

	switch (chUnPackedBuff[CMDCODE_POS])
    {
	case CTL_WORKSTATUS:
		{
			//信号机工作状态查询
			if (chUnPackedBuff[CMDCODE_POS - 1] == ASK_QUERY)
			{
				m_pOpenATCLog->LogOneMessage(LEVEL_INFO, "ParserPack_BaseInfoLink receive work status query");

				m_pOpenATCRunStatus->SetPhaseRunStatusWriteFlag(true);
				OpenATCSleep(100);
                bool bIsReadFlag = true;
				if (m_pOpenATCRunStatus->GetPhaseRunStatusReadFlag())
				{
					m_pOpenATCRunStatus->SetPhaseRunStatusReadFlag(false);
					m_pOpenATCRunStatus->SetPhaseRunStatusWriteFlag(false);
					nPackSize = AckCtl_AskWorkStatus();
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
						nPackSize = AckCtl_AskWorkStatus();
						m_pOpenATCRunStatus->SetPhaseRunStatusReadFlag(true);
						m_pOpenATCRunStatus->SetPhaseRunStatusWriteFlag(true);
						bIsReadFlag = false;
					}
				}
				if (bIsReadFlag)
				{
					nPackSize = AckCtl_AskNullWorkStatus();
				}
			}
		}
		break;
		case CTL_LAMPCOLORSTATUS:
		{
			//灯色状态查询
			if (chUnPackedBuff[CMDCODE_POS - 1] == ASK_QUERY)
			{
				m_pOpenATCLog->LogOneMessage(LEVEL_INFO, "ParserPack_BaseInfoLink receive light color status query");
				nPackSize = AckCtl_AskLampClrStatus();
			}
		}
		break;
	case CTL_CURRENTTIME:
		{
			//时间查询
			if (chUnPackedBuff[CMDCODE_POS - 1] == ASK_QUERY)
			{
				m_pOpenATCLog->LogOneMessage(LEVEL_INFO, "ParserPack_BaseInfoLink receive time query");
				nPackSize = AckCtl_AskQueryATCLocalTime();
			}
			//时间设置命令
			else if (chUnPackedBuff[CMDCODE_POS - 1] == ASK_SET)
			{
				if (!CheckSecretKey(chUnPackedBuff, dwPackLength, chPeerIp))
				{
					break;
				}

				m_pOpenATCLog->LogOneMessage(LEVEL_INFO, "ParserPack_BaseInfoLink receive time set command");

				time_t nTime = 0;
				memcpy(&nTime, chUnPackedBuff + CMDCODE_POS + 2, 4);
				m_pOpenATCParameter->GetAscTimeSetValue((unsigned char *)&chUnPackedBuff[CMDCODE_POS + 6], nTime);
				SetSysTime((long)nTime);

				nPackSize = AckCtl_AskSetATCLocalTime();

				//写操作记录日志
				TAscOperationRecord tAscOperationRecord;//操作记录
				memset(&tAscOperationRecord, 0, sizeof(tAscOperationRecord));
				tAscOperationRecord.m_unStartTime = time(NULL);
				tAscOperationRecord.m_unEndTime = time(NULL);
				tAscOperationRecord.m_bySubject = 1;
				tAscOperationRecord.m_byObject = 1;
				tAscOperationRecord.m_nInfoType = SYETEM_SETTIME;
				tAscOperationRecord.m_bStatus = true;
				sprintf((char *)tAscOperationRecord.m_byFailureValue, "IP is %s! SetTime", chPeerIp);
				m_tOpenATCOperationRecord.SaveOneOperationRecordMessage(&tAscOperationRecord, m_pOpenATCRunStatus);
			}
		}
		break;
	case CTL_FAULT:
		{
			//信号机故障查询
			if (chUnPackedBuff[CMDCODE_POS - 1] == ASK_QUERY)
			{
				m_pOpenATCLog->LogOneMessage(LEVEL_INFO, "ParserPack_BaseInfoLink receive fault query");
				int nFaultType = m_pOpenATCParameter->GetAscQueryFaultValue((unsigned char *)&chUnPackedBuff[DATACONTENT_POS]);
				nPackSize = AckCtl_AskQueryFault(nFaultType);
			}
		}
		break;
	case CTL_VERSION:
		{
			//信号机版本查询
			if (chUnPackedBuff[CMDCODE_POS - 1] == ASK_QUERY)
			{
				m_pOpenATCLog->LogOneMessage(LEVEL_INFO, "ParserPack_BaseInfoLink receive version query");
				nPackSize = AckCtl_AskQueryVersion();
			}
		}
		break;
	default:
		break;
	}

    if (nPackSize > 0)
    {
        return SendAckToPeer(nPackSize);
    }

    return nRet; 
}

int COpenATCCommWithCfgSWThread::ParserPack_AlternateFeatureParaLink(const unsigned char *chUnPackedBuff, unsigned int dwPackLength, char * chPeerIp)
{
	int nRet = OPENATC_RTN_OK;
	int nPackSize = 0;
	int nSendDataSize = 0;

	switch (chUnPackedBuff[CMDCODE_POS])
    {
	case CTL_SIGNALLIGHTGROUP:
		{
			//信号灯组查询
			if (chUnPackedBuff[CMDCODE_POS - 1] == ASK_QUERY)
			{
				m_pOpenATCLog->LogOneMessage(LEVEL_INFO, "ParserPack_AlternateFeatureParaLink receive signal light group query");
				nPackSize = AckCtl_AskQuerySignalLightGroup();
			}
			//信号灯组设置命令
			else if (chUnPackedBuff[CMDCODE_POS - 1] == ASK_SET)
			{
				m_pOpenATCLog->LogOneMessage(LEVEL_INFO, "ParserPack_AlternateFeatureParaLink receive signal light group set command");
				
				if (!CheckSecretKey(chUnPackedBuff, dwPackLength, chPeerIp))
				{
					break;
				}

				nPackSize = AckCtl_AskSetSignalLightGroup();
			}
		}
		break;
		case CTL_PHASE:
		{
			//相位查询
			if (chUnPackedBuff[CMDCODE_POS - 1] == ASK_QUERY)
			{
				m_pOpenATCLog->LogOneMessage(LEVEL_INFO, "ParserPack_AlternateFeatureParaLink receive phase query");
				nPackSize = AckCtl_AskQueryPhaseParamData();
			}
			//相位设置命令
			else if (chUnPackedBuff[CMDCODE_POS - 1] == ASK_SET)
			{
				m_pOpenATCLog->LogOneMessage(LEVEL_INFO, "ParserPack_AlternateFeatureParaLink receive phase set command");

				if (!CheckSecretKey(chUnPackedBuff, dwPackLength, chPeerIp))
				{
					break;
				}

				nPackSize = AckCtl_AskSetPhaseParamData();
			}
		}
		break;
	case CTL_SIGNALMATCHTIME:
		{
			//信号配时方案查询
			if (chUnPackedBuff[CMDCODE_POS - 1] == ASK_QUERY)
			{
				m_pOpenATCLog->LogOneMessage(LEVEL_INFO, "ParserPack_AlternateFeatureParaLink receive signal match time pattern query");
				nPackSize = AckCtl_AskQueryPartParamData(CheckParam_Pattern);

				//写操作记录日志
				TAscOperationRecord tAscOperationRecord;//操作记录
				memset(&tAscOperationRecord, 0, sizeof(tAscOperationRecord));
				tAscOperationRecord.m_unStartTime = time(NULL);
				tAscOperationRecord.m_unEndTime = time(NULL);
				tAscOperationRecord.m_bySubject = 1;
				tAscOperationRecord.m_byObject = 1;
				tAscOperationRecord.m_nInfoType = SYETEM_UPLOADPATTERN;
				tAscOperationRecord.m_bStatus = true;
				sprintf((char *)tAscOperationRecord.m_byFailureValue, "IP is %s! UpLoad Pattern success", chPeerIp);
				m_tOpenATCOperationRecord.SaveOneOperationRecordMessage(&tAscOperationRecord, m_pOpenATCRunStatus);
			}
			//信号配时方案设置命令
			else if (chUnPackedBuff[CMDCODE_POS - 1] == ASK_SET)
			{
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "ParserPack_AlternateFeatureParaLink receive signal match time pattern set command");
				memcpy(&nSendDataSize, chUnPackedBuff + DATALEN_POS, 4);
				TErrInfo tCurErrCode[C_N_MAX_CHECKERR_SIZE] = { 0 };
				
				if (!CheckSecretKey(chUnPackedBuff, dwPackLength, chPeerIp))
				{
					break;
				}

				int nCheckResult = m_pOpenATCParameter->CheckPartParamAndUpdateParam(CheckParam_Pattern, (unsigned char*)&chUnPackedBuff[DATACONTENT_POS], tCurErrCode);

				//写操作记录日志
				TAscOperationRecord tAscOperationRecord;//操作记录
				memset(&tAscOperationRecord, 0, sizeof(tAscOperationRecord));
				tAscOperationRecord.m_unStartTime = time(NULL);
				tAscOperationRecord.m_unEndTime = time(NULL);
				tAscOperationRecord.m_bySubject = 1;
				tAscOperationRecord.m_byObject = 1;
				tAscOperationRecord.m_nInfoType = SYETEM_DOWNLOADPATTERN;

				if (nCheckResult == OPENATC_RTN_OK)
				{
					tAscOperationRecord.m_bStatus = true;
					sprintf((char*)tAscOperationRecord.m_byFailureValue, "IP is %s! Save Pattern success", chPeerIp);
				}
				else if (nCheckResult == OPENATC_RTN_FAILED)
				{
					RecordVerifyErrorToSystemLogFile(tCurErrCode);

					tAscOperationRecord.m_bStatus = false;
                    sprintf((char *)tAscOperationRecord.m_byFailureValue, "IP is %s! Save Pattern fail", chPeerIp);
				}
				
				m_tOpenATCOperationRecord.SaveOneOperationRecordMessage(&tAscOperationRecord, m_pOpenATCRunStatus);

				nPackSize = AckCtl_AskSetPartParamData(CheckParam_Pattern, nCheckResult, tCurErrCode);
			}
		}
		break;
	case CTL_PROGRAMMESCHEDULEPLAN:
		{
			//方案调度计划查询
			if (chUnPackedBuff[CMDCODE_POS - 1] == ASK_QUERY)
			{
				m_pOpenATCLog->LogOneMessage(LEVEL_INFO, "ParserPack_AlternateFeatureParaLink receive programme time base schedule plan query");
				nPackSize = AckCtl_AskQueryPartParamData(CheckParam_Plan);

				//写操作记录日志
				TAscOperationRecord tAscOperationRecord;//操作记录
				memset(&tAscOperationRecord, 0, sizeof(tAscOperationRecord));
				tAscOperationRecord.m_unStartTime = time(NULL);
				tAscOperationRecord.m_unEndTime = time(NULL);
				tAscOperationRecord.m_bySubject = 1;
				tAscOperationRecord.m_byObject = 1;
				tAscOperationRecord.m_nInfoType = SYETEM_UPLOADPLAN;
				tAscOperationRecord.m_bStatus = true;
				sprintf((char *)tAscOperationRecord.m_byFailureValue, "IP is %s! UpLoad Plan success", chPeerIp);
				m_tOpenATCOperationRecord.SaveOneOperationRecordMessage(&tAscOperationRecord, m_pOpenATCRunStatus);
			}
			//方案调度计划设置命令
			else if (chUnPackedBuff[CMDCODE_POS - 1] == ASK_SET)
			{
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "ParserPack_AlternateFeatureParaLink receive programme time base schedule plan set command");
				TErrInfo tCurErrCode[C_N_MAX_CHECKERR_SIZE] = { 0 };
				
				if (!CheckSecretKey(chUnPackedBuff, dwPackLength, chPeerIp))
				{
					break;
				}
				
				int nCheckResult = m_pOpenATCParameter->CheckPartParamAndUpdateParam(CheckParam_Plan, (unsigned char*)&chUnPackedBuff[DATACONTENT_POS], tCurErrCode);

				//写操作记录日志
				TAscOperationRecord tAscOperationRecord;//操作记录
				memset(&tAscOperationRecord, 0, sizeof(tAscOperationRecord));
				tAscOperationRecord.m_unStartTime = time(NULL);
				tAscOperationRecord.m_unEndTime = time(NULL);
				tAscOperationRecord.m_bySubject = 1;
				tAscOperationRecord.m_byObject = 1;
				tAscOperationRecord.m_nInfoType = SYETEM_DOWNLOADPLAN;

				if (nCheckResult == OPENATC_RTN_OK)
				{
					tAscOperationRecord.m_bStatus = true;
					sprintf((char*)tAscOperationRecord.m_byFailureValue, "IP is %s! Save Plan success", chPeerIp);
				}
				else if (nCheckResult == OPENATC_RTN_FAILED)
				{
					RecordVerifyErrorToSystemLogFile(tCurErrCode);

					tAscOperationRecord.m_bStatus = false;
					sprintf((char *)tAscOperationRecord.m_byFailureValue, "IP is %s! Save Plan fail", chPeerIp);
				}

				m_tOpenATCOperationRecord.SaveOneOperationRecordMessage(&tAscOperationRecord, m_pOpenATCRunStatus);

				nPackSize = AckCtl_AskSetPartParamData(CheckParam_Plan, nCheckResult, tCurErrCode);
			}
		}
		break;
	case CTL_OVERLAP:
		{
			//跟随相位查询
			if (chUnPackedBuff[CMDCODE_POS - 1] == ASK_QUERY)
			{
				m_pOpenATCLog->LogOneMessage(LEVEL_INFO, "ParserPack_AlternateFeatureParaLink receive programme time base schedule overlap query");
				nPackSize = AckCtl_AskQueryPartParamData(CheckParam_OverLap);
			}
			//跟随相位设置命令
			else if (chUnPackedBuff[CMDCODE_POS - 1] == ASK_SET)
			{
				m_pOpenATCLog->LogOneMessage(LEVEL_INFO, "ParserPack_AlternateFeatureParaLink receive programme time base schedule overlap set command");
				//int nCheckResult = m_pOpenATCParameter->CheckPartParam(CheckParam_OverLap, (unsigned char *)&chUnPackedBuff[DATACONTENT_POS]);

				if (!CheckSecretKey(chUnPackedBuff, dwPackLength, chPeerIp))
				{
					break;
				}

				//TErrInfo tCurErrCode[C_N_MAX_CHECKERR_SIZE] = { 0 };

				//nPackSize = AckCtl_AskSetPartParamData(CheckParam_OverLap, OPENATC_RTN_OK, tCurErrCode);
				nPackSize = AckCtl_AskSetPhaseParamData();
			}
		}
		break;
	case CTL_SCHEDUL_DATE:
		{
			//日期查询
			if (chUnPackedBuff[CMDCODE_POS - 1] == ASK_QUERY)
			{
				m_pOpenATCLog->LogOneMessage(LEVEL_INFO, "ParserPack_AlternateFeatureParaLink receive programme time base schedule date query");
				nPackSize = AckCtl_AskQueryPartParamData(CheckParam_Date);

				//写操作记录日志
				TAscOperationRecord tAscOperationRecord;//操作记录
				memset(&tAscOperationRecord, 0, sizeof(tAscOperationRecord));
				tAscOperationRecord.m_unStartTime = time(NULL);
				tAscOperationRecord.m_unEndTime = time(NULL);
				tAscOperationRecord.m_bySubject = 1;
				tAscOperationRecord.m_byObject = 1;
				tAscOperationRecord.m_nInfoType = SYETEM_UPLOADDATE;
				tAscOperationRecord.m_bStatus = true;
				sprintf((char *)tAscOperationRecord.m_byFailureValue, "IP is %s! UpLoad Date success", chPeerIp);
				m_tOpenATCOperationRecord.SaveOneOperationRecordMessage(&tAscOperationRecord, m_pOpenATCRunStatus);
			}
			//日期设置命令
			else if (chUnPackedBuff[CMDCODE_POS - 1] == ASK_SET)
			{
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "ParserPack_AlternateFeatureParaLink receive programme time base schedule date set command");
				TErrInfo tCurErrCode[C_N_MAX_CHECKERR_SIZE] = { 0 };

				if (!CheckSecretKey(chUnPackedBuff, dwPackLength, chPeerIp))
				{
					break;
				}

				int nCheckResult = m_pOpenATCParameter->CheckPartParamAndUpdateParam(CheckParam_Date, (unsigned char*)&chUnPackedBuff[DATACONTENT_POS], tCurErrCode);

				//写操作记录日志
				TAscOperationRecord tAscOperationRecord;//操作记录
				memset(&tAscOperationRecord, 0, sizeof(tAscOperationRecord));
				tAscOperationRecord.m_unStartTime = time(NULL);
				tAscOperationRecord.m_unEndTime = time(NULL);
				tAscOperationRecord.m_bySubject = 1;
				tAscOperationRecord.m_byObject = 1;
				tAscOperationRecord.m_nInfoType = SYETEM_DOWNLOADDATE;

				if (nCheckResult == OPENATC_RTN_OK)
				{
					tAscOperationRecord.m_bStatus = true;
					sprintf((char*)tAscOperationRecord.m_byFailureValue, "IP is %s! Save Date success", chPeerIp);
				}
				else if (nCheckResult == OPENATC_RTN_FAILED)
				{
					RecordVerifyErrorToSystemLogFile(tCurErrCode);

					tAscOperationRecord.m_bStatus = false;
					sprintf((char *)tAscOperationRecord.m_byFailureValue, "IP is %s! Save Date fail", chPeerIp);
				}
				m_tOpenATCOperationRecord.SaveOneOperationRecordMessage(&tAscOperationRecord, m_pOpenATCRunStatus);

				nPackSize = AckCtl_AskSetPartParamData(CheckParam_Date, nCheckResult, tCurErrCode);
			}
		}
		break;
	case CTL_SYSTEM_CUSTOM:
	{
		//设备信息查询
		if (chUnPackedBuff[CMDCODE_POS - 1] == ASK_QUERY)
		{
			m_pOpenATCLog->LogOneMessage(LEVEL_INFO, "ParserPack_AlternateFeatureParaLink receive system custom query");
 			nPackSize = AckCtl_AskQuerySystemCustomData();
		}
		//设备信息设置命令
		else if (chUnPackedBuff[CMDCODE_POS - 1] == ASK_SET)
		{
			if (!CheckSecretKey(chUnPackedBuff, dwPackLength, chPeerIp))
			{
				break;
			}

			bool bSaveSuccess = false;

			int nSendDataSize = 0;
			m_pOpenATCLog->LogOneMessage(LEVEL_INFO, "ParserPack_AlternateFeatureParaLink receive system custom set command");
			memcpy(&nSendDataSize, chUnPackedBuff + DATALEN_POS, 4);
			int nCheckResultSiteID = m_openATCParamCheck.CheckSystemDeviceInfo_SiteID((unsigned char *)&chUnPackedBuff[DATACONTENT_POS]);
			int nCheckResultOther  = m_openATCParamCheck.CheckSystemDeviceInfo_Other((unsigned char *)&chUnPackedBuff[DATACONTENT_POS], false);
			if (nCheckResultSiteID == OPENATC_PARAM_CHECK_OK && nCheckResultOther == OPENATC_PARAM_CHECK_OK)
			{
				int nCurSiteID = m_pOpenATCParameter->GetSiteID();
				bool bCheckSiteID = false;
				bCheckSiteID = m_openATCParamCheck.CheckSiteID((unsigned char *)&chUnPackedBuff[DATACONTENT_POS], nSendDataSize, nCurSiteID, true);
				if (bCheckSiteID)
				{
					int nSaveDeviceParameterRet = m_pOpenATCParameter->SavaSystemCustom((unsigned char *)&chUnPackedBuff[DATACONTENT_POS], nSendDataSize);
					if (nSaveDeviceParameterRet == OPENATC_RTN_OK)
					{
						// 如果原先设备信息异常，则清空
						if (m_pOpenATCRunStatus->GetDeviceParamOtherInfoFaultFlag())
						{
							m_pOpenATCRunStatus->SetDeviceParamOtherInfoFaultFlag(false);
						}

						nPackSize = AckCtl_AskSetSystemCustomData();

						TParamRunStatus tParamRunStatus;
						m_pOpenATCRunStatus->GetParamRunStatus(tParamRunStatus);
						tParamRunStatus.m_chIsParameterChg = C_CH_ISPARAMETERCHG_OK;
						if (tParamRunStatus.m_chParameterReady != C_CH_PARAMERREADY_OK)
						{
							m_pOpenATCParameter->Init(m_pOpenATCRunStatus, m_pOpenATCLog);
						}
						m_pOpenATCRunStatus->SetParamRunStatus(tParamRunStatus);
						bSaveSuccess = true;

					}
					else
					{
						TErrInfo tCurErrCode[C_N_MAX_CHECKERR_SIZE] = { 0 };

						if (nSaveDeviceParameterRet == OPENATC_SAVE_PARAM_FAILED_USB_NOT_FIND)
						{
							tCurErrCode[0].nCheckErr	= CheckUSBStatus_USB_Not_Find;
							tCurErrCode[0].nSubCheckErr = 0;
						}
						else if (nSaveDeviceParameterRet == OPENATC_SAVE_PARAM_FAILED_USB_MOUNT_FAILED)
						{
							tCurErrCode[0].nCheckErr	= CheckUSBStatus_USB_Mount_Fail;
							tCurErrCode[0].nSubCheckErr = 0;
						}
						else if (nSaveDeviceParameterRet == OPENATC_SAVE_PARAM_FAILED_DEVPARAM_UPDATE_FAILED)
						{
							tCurErrCode[0].nCheckErr	= CheckDeviceParamInfo_Update_Fail;
							tCurErrCode[0].nSubCheckErr = 0;
						}

						RecordVerifyErrorToSystemLogFile(tCurErrCode);

						nPackSize = AckCfg_SendCheckDataFailed(tCurErrCode);			//参数保存失败
					}
				}
				else
				{
					TErrInfo tCurErrCode[C_N_MAX_CHECKERR_SIZE] = { 0 };
					m_openATCParamCheck.GetErrCode(tCurErrCode);

					RecordVerifyErrorToSystemLogFile(tCurErrCode);

					nPackSize = AckCfg_SendCheckDataFailed(tCurErrCode);				//地址码校验失败
				}
			}
			else
			{
				//参数解析失败，包含格式错误和值错误
				TErrInfo tCurErrCode[C_N_MAX_CHECKERR_SIZE] = { 0 };
				m_openATCParamCheck.GetErrCode(tCurErrCode);
				if (tCurErrCode[0].nCheckErr == 0)
				{
					tCurErrCode[0].nCheckErr = CheckDeviceParamInfo_JSON_Parse_Fail;
					tCurErrCode[0].nSubCheckErr = 0;
				}

				RecordVerifyErrorToSystemLogFile(tCurErrCode);

				nPackSize = AckCfg_SendCheckDataFailed(tCurErrCode);
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
				tAscOperationRecord.m_nInfoType = SYSTEM_DOWNLOAD_HWPARAM;
				tAscOperationRecord.m_bStatus = true;

				sprintf((char *)tAscOperationRecord.m_byFailureValue, "IP is %s! SaveDeviceParameter success", chPeerIp);

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
				tAscOperationRecord.m_nInfoType = SYSTEM_DOWNLOAD_HWPARAM;
				tAscOperationRecord.m_bStatus = false;

				sprintf((char *)tAscOperationRecord.m_byFailureValue, "IP is %s! SaveDeviceParameter fail.", chPeerIp);

				m_tOpenATCOperationRecord.SaveOneOperationRecordMessage(&tAscOperationRecord, m_pOpenATCRunStatus);
			}
		}
	}
		break;

	case CTL_SYSTEM_UPDATE:
	{
		//ftp远程传输完成，设置更新命令
		if (chUnPackedBuff[CMDCODE_POS - 1] == ASK_SET)
		{
			m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "ParserPack_AlternateFeatureParaLink receive ftp update set command.");
			
			if (!CheckSecretKey(chUnPackedBuff, dwPackLength, chPeerIp))
			{
				break;
			}

#ifndef _WIN32
			if (system("./UpdateOpenATC") < 0)
			{
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "ParserPack_AlternateFeatureParaLink ftp file update failed.");
			}
			else
			{
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "ParserPack_AlternateFeatureParaLink ftp file update success.");
			}
#endif
			nPackSize = AckCtl_AskSetFTPFileUpdateCmd();
		}
	}
		break;
	default:
		break;
	}

	if (nPackSize > 0)
    {
        return SendAckToPeer(nPackSize);
    }

    return nRet;
}

int COpenATCCommWithCfgSWThread::ParserPack_InterveneCommandLink(const unsigned char *chUnPackedBuff, unsigned int dwPackLength, char * chPeerIp)
{
	int nRet		= OPENATC_RTN_OK;
    int nPackSize	= 0;
    bool bReboot	= false;

	switch (chUnPackedBuff[CMDCODE_POS])
    {
    case CTL_WORKWAY:
        {
			//工作方式查询
            if (chUnPackedBuff[CMDCODE_POS - 1] == ASK_QUERY)
			{
				m_pOpenATCLog->LogOneMessage(LEVEL_INFO, "ParserPack_BaseInfoLink receive work way query");

                nPackSize = AckCtl_AskQueryWorkWay();
			}
			//工作方式设置命令
			else if (chUnPackedBuff[CMDCODE_POS - 1] == ASK_SET)
			{
				m_pOpenATCLog->LogOneMessage(LEVEL_INFO, "ParserPack_BaseInfoLink receive work way set command");

				if (!CheckSecretKey(chUnPackedBuff, dwPackLength, chPeerIp))
				{
					break;
				}

                TWorkModeParam atWorkModeParam;
				TPhasePassCmdPhaseStatus atPhasePhassCmdPhaseStatus;
				TChannelLockCtrlCmd atChannelLockCtrlCmd;
				memset(&atChannelLockCtrlCmd,0,sizeof(TChannelLockCtrlCmd));
				TPhaseLockCtrlCmd atPhaseLockCtrlCmd;
				memset(&atPhaseLockCtrlCmd,0,sizeof(TPhaseLockCtrlCmd));
				TPreemptCtlCmd atPreemptCtlCmd;
				memset(&atPreemptCtlCmd,0,sizeof(atPreemptCtlCmd));
                m_pOpenATCParameter->GetWorkModeParam((unsigned char *)&chUnPackedBuff[CMDCODE_POS + 6], atWorkModeParam, atPhasePhassCmdPhaseStatus, atChannelLockCtrlCmd, atPhaseLockCtrlCmd, atPreemptCtlCmd);

				TManualCmd  tManualCmd;
				memset(&tManualCmd,0,sizeof(tManualCmd));

				int nResult = CONTROL_SUCCEED;
				int nFailCode = 0;

				TSystemControlStatus tSystemControlStatus;
				memset(&tSystemControlStatus, 0, sizeof(tSystemControlStatus));
				m_pOpenATCRunStatus->GetSystemControlStatus(tSystemControlStatus);

				TAscStepCfg tStepCfg;
				memset(&tStepCfg, 0, sizeof(tStepCfg));
				m_pOpenATCParameter->GetStepInfo(tStepCfg);

				if (atWorkModeParam.m_byControlType == CTL_MODE_PHASE_PASS_CONTROL)
				{
					for (int i = 0; i < MAX_PHASE_COUNT; i ++)
					{
						if (atPhasePhassCmdPhaseStatus.m_nPhasePassStatus[i] != PhasePassStatus_Normal)
						{
							if (CheckPhaseControl(i + 1) == CONTROL_FAILED)
							{
								nPackSize = AckCtl_AskSetCmdWorkWay(CTL_MODE_PHASE_PASS_CONTROL, NO_SUPPORT_CONTROL_PARAM);
								m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "PhaseID:%d Cannot be used, PhasePassControl Command Cannot Execute", i + 1);
								return SendAckToPeer(nPackSize);
							}
						}
					}

					if (tSystemControlStatus.m_nPhaseControlResult == CONTROL_SUCCEED)
					{
						atPhasePhassCmdPhaseStatus.m_bNewCmd = true;
						m_pOpenATCRunStatus->SetPhasePassCmdPhaseStatus(atPhasePhassCmdPhaseStatus);
						nPackSize = AckCtl_AskSetCmdWorkWay(CTL_MODE_PHASE_PASS_CONTROL, 0);
					}
					else
					{
						nPackSize = AckCtl_AskSetCmdWorkWay(CTL_MODE_PHASE_PASS_CONTROL, NO_SUPPORT_CONTROL_PARAM);
					}
				}
				else if (atWorkModeParam.m_byControlType == CTL_MODE_CHANNEL_LOCK)
				{
					m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "Receive ChannelLock Command");

					TSystemControlStatus tSystemControlStatus;
					memset(&tSystemControlStatus, 0, sizeof(tSystemControlStatus));
					m_pOpenATCRunStatus->GetSystemControlStatus(tSystemControlStatus);

					if (tSystemControlStatus.m_nChannelLockResult == CONTROL_FAILED)
					{
						nPackSize = AckCtl_AskSetCmdWorkWay(CTL_MODE_CHANNEL_LOCK, NO_SUPPORT_CONTROL_PARAM);
						m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "ChannelLock Command Cannot Execute, FailCode:%d", tSystemControlStatus.m_nChannelLockFailCode);
						return SendAckToPeer(nPackSize);
					}

					CreateChannelLockCmd(atChannelLockCtrlCmd, tManualCmd, chPeerIp);
					
					nPackSize = AckCtl_AskSetCmdWorkWay(CTL_MODE_CHANNEL_LOCK, 0);
				}
				else if (atWorkModeParam.m_byControlType == CTL_MODE_PHASE_LOCK)
				{
					m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "Receive PhaseLock Command");

					TSystemControlStatus tSystemControlStatus;
					memset(&tSystemControlStatus, 0, sizeof(tSystemControlStatus));
					m_pOpenATCRunStatus->GetSystemControlStatus(tSystemControlStatus);

					if (tSystemControlStatus.m_nChannelLockResult == CONTROL_FAILED)
					{
						nPackSize = AckCtl_AskSetCmdWorkWay(CTL_MODE_PHASE_LOCK, NO_SUPPORT_CONTROL_PARAM);
						m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "PhaseLock Command Cannot Execute, FailCode:%d", tSystemControlStatus.m_nChannelLockFailCode);
						return SendAckToPeer(nPackSize);
					}

					for (int i = 0; i < MAX_PHASE_COUNT; i ++)
					{
						if (atPhaseLockCtrlCmd.m_nPhaseLockType[i] != LOCK_TYPE_CANCEL)
						{
							if (CheckPhaseControl(i + 1) == CONTROL_FAILED)
							{
								nPackSize = AckCtl_AskSetCmdWorkWay(CTL_MODE_PHASE_LOCK, NO_SUPPORT_CONTROL_PARAM);
								m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "PhaseID:%d Cannot be used, PhaseLock Command Cannot Execute", i + 1);
								return SendAckToPeer(nPackSize);
							}
						}
					}

					if (TransPhaseLockCmdToChannelLockCmd(atPhaseLockCtrlCmd, atChannelLockCtrlCmd, tManualCmd, chPeerIp))
					{
						CreateChannelLockCmd(atChannelLockCtrlCmd, tManualCmd, chPeerIp);
					}
					else
					{
						CreateManualCmdReturnToSelf(chPeerIp);
					}
					nPackSize = AckCtl_AskSetCmdWorkWay(CTL_MODE_PHASE_LOCK, 0);
				}
				else if (atWorkModeParam.m_byControlType == CTL_MODE_PREEMPT)
				{
					TPreemptControlStatus tPreemptControlStatus;
					memset(&tPreemptControlStatus, 0, sizeof(tPreemptControlStatus));
					m_pOpenATCRunStatus->GetPreemptControlStatus(tPreemptControlStatus);

					if (tPreemptControlStatus.m_nPreemptControlResult == CONTROL_FAILED)
					{
						nPackSize = AckCtl_AskSetCmdWorkWay(CTL_MODE_PREEMPT, NO_SUPPORT_CONTROL_PARAM);
						m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "PreemptControl Command Cannot Execute, FailCode:%d", tPreemptControlStatus.m_nPreemptControlResultFailCode);
						return SendAckToPeer(nPackSize);
					}
					
					if (atPreemptCtlCmd.m_byPreemptPhaseID == 0 || CheckPhaseControl(atPreemptCtlCmd.m_byPreemptPhaseID) == CONTROL_FAILED)
					{
						nPackSize = AckCtl_AskSetCmdWorkWay(CTL_MODE_PREEMPT, NO_SUPPORT_CONTROL_PARAM);
						m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "PhaseID:%d Cannot be used, PreemptControl Command Cannot Execute", atPreemptCtlCmd.m_byPreemptPhaseID);
						return SendAckToPeer(nPackSize);
					}
			
					atPreemptCtlCmd.m_bNewCmd = true;
					atPreemptCtlCmd.m_nCmdSource = CTL_SOURCE_SYSTEM;
					memcpy(atPreemptCtlCmd.m_szPeerIp, chPeerIp, strlen(chPeerIp));
					m_pOpenATCRunStatus->SetPreemptCtlCmd(atPreemptCtlCmd);
					m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "PreemptControl Command From System, PreemptPhaseID:%d", atPreemptCtlCmd.m_byPreemptPhaseID);

					tManualCmd.m_bNewCmd = true;
					tManualCmd.m_nCmdSource = CTL_SOURCE_SYSTEM;
					memcpy(tManualCmd.m_szPeerIp, chPeerIp, strlen(chPeerIp));
					tManualCmd.m_bPreemptCtlCmd = true;
					m_pOpenATCRunStatus->SetManualCmd(tManualCmd);
					m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "PreemptControl Command From System Control IP is %s", chPeerIp);
	
					nPackSize = AckCtl_AskSetCmdWorkWay(CTL_MODE_PREEMPT, 0);
				}
				else
				{
					/*static int nFlagFlag = 2;
					if (nFlagFlag == 0 || nFlagFlag == 1)
					{
						ParaseWorkWay(atWorkModeParam, chPeerIp, tManualCmd, nResult, nFailCode, true);
						//nFlagFlag += 1;
					}
					else  if (nFlagFlag == 2)
					{
						//模拟面板测试使用
						static int nFlag = 0;
						if (nFlag == 0)
						{
							tManualCmd.m_bStepForwardCmd = true;//第一次面板进入手动，正常跑方案
						}
						else if (nFlag == 1)
						{
							//tManualCmd.m_bStepForwardCmd = true;//第二次面板进入步进，卡住不动
							//tManualCmd.m_tStepForwardCmd.m_byStepType = tStepCfg.m_byStepType;
					
							//tManualCmd.m_bPatternInterruptCmd = true;//方案干预
							//tManualCmd.m_tPatternInterruptCmd.m_nControlMode = CTL_MODE_SELFCTL;

							tManualCmd.m_bDirectionCmd = true;//方向控制
							tManualCmd.m_tDirectionCmd.m_nNextDirectionIndex = HWPANEL_DIR_EAST_WEST_STRAIGHT_INDEX;
						}
						else if (nFlag == 2)
						{
							//tManualCmd.m_bStepForwardCmd = true;//面板进入步进
							//tManualCmd.m_tStepForwardCmd.m_byStepType = tStepCfg.m_byStepType;

							//tManualCmd.m_bPatternInterruptCmd = true;//方案干预
							//tManualCmd.m_tPatternInterruptCmd.m_nControlMode = CTL_MODE_SELFCTL;

							tManualCmd.m_bDirectionCmd = true;//方向控制
						    tManualCmd.m_tDirectionCmd.m_nNextDirectionIndex = HWPANEL_DIR_EAST_WEST_STRAIGHT_INDEX;
						}
						else if (nFlag == 3)
						{
							//tManualCmd.m_bStepForwardCmd = true;//面板进入步进
							//tManualCmd.m_tStepForwardCmd.m_byStepType = tStepCfg.m_byStepType;

							//tManualCmd.m_bPatternInterruptCmd = true;//方案干预
							//tManualCmd.m_tPatternInterruptCmd.m_nControlMode = CTL_MODE_SELFCTL;

							tManualCmd.m_bDirectionCmd = true;//方向控制
							tManualCmd.m_tDirectionCmd.m_nNextDirectionIndex = HWPANEL_DIR_EAST_WEST_STRAIGHT_INDEX;
						}
						else if (nFlag == 4)
						{
							//tManualCmd.m_bStepForwardCmd  = true;//面板进入步进
							//tManualCmd.m_tStepForwardCmd.m_byStepType = tStepCfg.m_byStepType;

							//tManualCmd.m_bPatternInterruptCmd = true;//方案干预
							//tManualCmd.m_tPatternInterruptCmd.m_nControlMode = CTL_MODE_SELFCTL;

							tManualCmd.m_bDirectionCmd = true;//方向控制
							tManualCmd.m_tDirectionCmd.m_nNextDirectionIndex = HWPANEL_DIR_SOUTH_NORTH_STRAIGHT_INDEX;
						}
						else if (nFlag == 5)
						{
							tManualCmd.m_bPatternInterruptCmd = true;//方案干预
							tManualCmd.m_tPatternInterruptCmd.m_nControlMode = CTL_MODE_ALLRED;
						}
						else if (nFlag == 6)
						{
							tManualCmd.m_bDirectionCmd = true;//方向控制
							tManualCmd.m_tDirectionCmd.m_nNextDirectionIndex = HWPANEL_DIR_EAST_WEST_STRAIGHT_INDEX;
						}
						nFlag += 1;

						tManualCmd.m_bNewCmd = true;
						tManualCmd.m_nCmdSource = CTL_SOURCE_LOCAL;
						memcpy(tManualCmd.m_szPeerIp,chPeerIp,strlen(chPeerIp));
						tManualCmd.m_tStepForwardCmd.m_byStepType = tStepCfg.m_byStepType;
						m_pOpenATCRunStatus->SetManualCmd(tManualCmd);
					}*/

					bool bProcess = true;
					long nCurTime = time(NULL);
					static long nLastTime = time(NULL);
					if (memcmp(&m_atOldWorkModeParam, &atWorkModeParam, sizeof(atWorkModeParam)) == 0)
					{
						//连续两次间隔1秒的相同的指令，不予处理
						if (labs(nCurTime - nLastTime) < COMMAND_TIME_INTERVAL)
						{
							bProcess = false;
							m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "Same Command WithIn 2 Seconds, Not Process!");
						}
						else
						{
							nLastTime = nCurTime;
						}
					}
					else
					{
						memcpy(&m_atOldWorkModeParam, &atWorkModeParam, sizeof(atWorkModeParam));
						nLastTime = nCurTime;
					}
				    
					ParaseWorkWay(atWorkModeParam, chPeerIp, tManualCmd, nResult, nFailCode, bProcess);
					nPackSize = AckCtl_AskSetWorkWay(tManualCmd, nResult, nFailCode);
				}
			}
        }
        break;
	 case CTL_FEATUREPARAVERSION:
        {
			//特征参数版本查询
            if (chUnPackedBuff[CMDCODE_POS - 1] == ASK_QUERY)
			{
				m_pOpenATCLog->LogOneMessage(LEVEL_INFO, "ParserPack_BaseInfoLink receive feaure param query");

				nPackSize = AckCtl_AskQueryATCParamVersion();
			}
			//特征参数版本设置命令
			else if (chUnPackedBuff[CMDCODE_POS - 1] == ASK_SET)
			{
				if (!CheckSecretKey(chUnPackedBuff, dwPackLength, chPeerIp))
				{
					break;
				}

				m_pOpenATCLog->LogOneMessage(LEVEL_INFO, "ParserPack_BaseInfoLink receive feaure param set command");

                nPackSize = AckCtl_AskSetATCParamVersion();
			}
        }
        break;
	case CTL_INDENTIFYCODE:
        {
			//信号机识别码查询
            if (chUnPackedBuff[CMDCODE_POS - 1] == ASK_QUERY)
			{
				m_pOpenATCLog->LogOneMessage(LEVEL_INFO, "ParserPack_BaseInfoLink receive identify code query");

				nPackSize = AckCtl_AskQueryATCCode();
			}
        }
        break;
	case CTL_REMOTECONTROL:
        {
			//远程控制设置命令
            if (chUnPackedBuff[CMDCODE_POS - 1] == ASK_SET)
			{
				m_pOpenATCLog->LogOneMessage(LEVEL_INFO, "ParserPack_BaseInfoLink receive remote control set command");

				if (!CheckSecretKey(chUnPackedBuff, dwPackLength, chPeerIp))
				{
					break;
				}

                nPackSize = AckCtl_AskRemoteControl();
				//重启指令，临时写死
                if (m_pOpenATCParameter->GetRemoteValueValue((unsigned char *)&chUnPackedBuff[CMDCODE_POS + 6]) == 0x01)
                {
                    m_pOpenATCLog->LogOneMessage(LEVEL_INFO, "ParserPack_BaseInfoLink receive reboot command");

                    if (nPackSize > 0)
                    {
                        SendAckToPeer(nPackSize);
                        OpenATCSleep(10);
                    }

					//写操作记录日志
					TAscOperationRecord tAscOperationRecord;//操作记录
					memset(&tAscOperationRecord, 0, sizeof(tAscOperationRecord));

					tAscOperationRecord.m_unStartTime = time(NULL);
					tAscOperationRecord.m_unEndTime = time(NULL);
					tAscOperationRecord.m_bySubject = 1;
					tAscOperationRecord.m_byObject = 1;
					tAscOperationRecord.m_nInfoType = SYETEM_REBOOT;
					tAscOperationRecord.m_bStatus = true;

					sprintf((char *)tAscOperationRecord.m_byFailureValue, "IP is %s! Reboot ATC", chPeerIp);

					m_tOpenATCOperationRecord.SaveOneOperationRecordMessage(&tAscOperationRecord, m_pOpenATCRunStatus);
               
                    m_pOpenATCRunStatus->SetRebootStatus(true);
                    bReboot = true;
                }
				//手动授权，临时写死
                else if (m_pOpenATCParameter->GetRemoteValueValue((unsigned char *)&chUnPackedBuff[CMDCODE_POS + 6]) == 0x02)
                {
                    m_pOpenATCLog->LogOneMessage(LEVEL_INFO, "ParserPack_BaseInfoLink receive manual authorization command");
                }
				//用户自定义指令，临时写死
                else if (m_pOpenATCParameter->GetRemoteValueValue((unsigned char *)&chUnPackedBuff[CMDCODE_POS + 6]) == 0x03)
                {
                    m_pOpenATCLog->LogOneMessage(LEVEL_INFO, "ParserPack_BaseInfoLink receive user customization command");
                }
			}
        }
        break;
	case CTL_DETECTOR:
        {
			//检测器查询命令
            if (chUnPackedBuff[CMDCODE_POS - 1] == ASK_QUERY)
			{
				m_pOpenATCLog->LogOneMessage(LEVEL_INFO, "ParserPack_BaseInfoLink receive detector query");

				nPackSize = AckCtl_AskQueryVecDetectorParamData();
			}
			//检测器设置命令
			else if (chUnPackedBuff[CMDCODE_POS - 1] == ASK_SET)
			{
				if (!CheckSecretKey(chUnPackedBuff, dwPackLength, chPeerIp))
				{
					break;
				}

				m_pOpenATCLog->LogOneMessage(LEVEL_INFO, "ParserPack_BaseInfoLink receive detector set command");

                nPackSize = AckCtl_AskSetVecDetectorParamData();
			}
        }
        break;
	default:
		break;
	}

    if (bReboot)
    {
        return nRet;
    }

	if (nPackSize > 0)
    {
        return SendAckToPeer(nPackSize);
    }

    return nRet;
}

int COpenATCCommWithCfgSWThread::ParserPack_OptimizeControlLink(const unsigned char *chUnPackedBuff, unsigned int dwPackLength, char * chPeerIp)
{
	int nRet		= OPENATC_RTN_OK;
    int nPackSize	= 0;
    bool bReboot	= false;

	switch (chUnPackedBuff[CMDCODE_POS])
    {
	case CTL_DETECTOR:
		{
			//检测器数据查询命令
            if (chUnPackedBuff[CMDCODE_POS - 1] == ASK_QUERY)
			{
				m_pOpenATCLog->LogOneMessage(LEVEL_INFO, "ParserPack_OptimizeControlLink receive detector query");

				nPackSize = AckCtl_AskQueryVecDetectorAllData();
			}
			else if (chUnPackedBuff[CMDCODE_POS - 1] == ASK_SET)
			{
				if (!CheckSecretKey(chUnPackedBuff, dwPackLength, chPeerIp))
				{
					break;
				}

				m_pOpenATCLog->LogOneMessage(LEVEL_INFO, "ParserPack_OptimizeControlLink receive detector set");

				nPackSize = AckCtl_AskVecDetectorReportChgData();

				m_chSendDetectorChgData = chUnPackedBuff[DATACONTENT_POS];
			}
		}
		break;
	default:
		break;
	}

 
	if (nPackSize > 0)
    {
        return SendAckToPeer(nPackSize);
    }

    return nRet;
}

int COpenATCCommWithCfgSWThread::SendAckToPeer(int nPackSize)
{
    unsigned int dwSize = 0;
    m_packerUnPacker.PackBuffer(m_chSendBuff, nPackSize, m_chPackedBuff, dwSize);

    if (dwSize == 0)
    {
        return OPENATC_RTN_FAILED;
    }

	if (m_byComType == UDP_SERVICE)
	{
		return m_commHelper.Write(m_chPackedBuff, dwSize);
	}
	else 
	{
		return m_clientSock.Write(m_chPackedBuff, dwSize, 400);
	}
}

 int COpenATCCommWithCfgSWThread::AckCfg_AskSend()
 {
     unsigned int dwSendBuffLen = 0;

	 memset(m_chSendBuff, 0x00, SEND_BUFFER_SIZE);

	 m_chSendBuff[dwSendBuffLen++] = CB_VERSION_FLAG;										//版本号
	 m_chSendBuff[dwSendBuffLen++] = CB_ATC_FLAG;											//发送方标识
	 m_chSendBuff[dwSendBuffLen++] = CB_CONFIGSOFTWARE_FLAG;								//接收方标识
	 m_chSendBuff[dwSendBuffLen++] = LINK_CONFIG;											//数据链路码
	 m_chSendBuff[dwSendBuffLen++] = m_tAreaInfo.m_chAscRegionNo;							//区域号
	 memcpy(m_chSendBuff + dwSendBuffLen, &m_tAreaInfo.m_usAscRoadNo, MAX_ROADNO_LENGTH);	//路口号
     dwSendBuffLen += MAX_ROADNO_LENGTH;
	 m_chSendBuff[dwSendBuffLen++] = ACK_SET;												//操作类型
	 m_chSendBuff[dwSendBuffLen++] = CFG_ACK_ASKSEND;										//对象标识
	 m_chSendBuff[dwSendBuffLen++] = 0x01;													//保留
	 m_chSendBuff[dwSendBuffLen++] = 0x00;													//保留
	 m_chSendBuff[dwSendBuffLen++] = 0x00;													//保留
	 m_chSendBuff[dwSendBuffLen++] = 0x00;													//保留
	 m_chSendBuff[dwSendBuffLen++] = 0x00;													//保留

     if (m_msgBody != NULL)
     {
         cJSON_Delete(m_msgBody);
         m_msgBody = NULL;
     }

     if (m_chOutBuff != NULL)
     {
         free(m_chOutBuff);
         m_chOutBuff = NULL;
     }

     m_msgBody = cJSON_CreateObject();
     cJSON_AddStringToObject(m_msgBody, "return", "success");
     m_chOutBuff = cJSON_Print(m_msgBody);
     memcpy(m_chSendBuff + dwSendBuffLen, m_chOutBuff, strlen(m_chOutBuff));
     dwSendBuffLen += strlen(m_chOutBuff);

	 return dwSendBuffLen; 
 }

int COpenATCCommWithCfgSWThread::AckCfg_SendDataOK()
{
     unsigned int dwSendBuffLen = 0;

     memset(m_chSendBuff, 0x00, SEND_BUFFER_SIZE);

     m_chSendBuff[dwSendBuffLen++] = CB_VERSION_FLAG;										//版本号
	 m_chSendBuff[dwSendBuffLen++] = CB_ATC_FLAG;											//发送方标识
	 m_chSendBuff[dwSendBuffLen++] = CB_CONFIGSOFTWARE_FLAG;								//接收方标识
	 m_chSendBuff[dwSendBuffLen++] = LINK_CONFIG;											//数据链路码
	 m_chSendBuff[dwSendBuffLen++] = m_tAreaInfo.m_chAscRegionNo;							//区域号
	 memcpy(m_chSendBuff + dwSendBuffLen, &m_tAreaInfo.m_usAscRoadNo, MAX_ROADNO_LENGTH);	//路口号
     dwSendBuffLen += MAX_ROADNO_LENGTH;
	 m_chSendBuff[dwSendBuffLen++] = ACK_SET;												//操作类型
	 m_chSendBuff[dwSendBuffLen++] = CFG_ACK_SENDDATA_OK;									//对象标识
	 m_chSendBuff[dwSendBuffLen++] = 0x01;													//保留
	 m_chSendBuff[dwSendBuffLen++] = 0x00;													//保留
	 m_chSendBuff[dwSendBuffLen++] = 0x00;													//保留
	 m_chSendBuff[dwSendBuffLen++] = 0x00;													//保留
	 m_chSendBuff[dwSendBuffLen++] = 0x00;													//保留

     if (m_msgBody != NULL)
     {
         cJSON_Delete(m_msgBody);
         m_msgBody = NULL;
     }

     if (m_chOutBuff != NULL)
     {
         free(m_chOutBuff);
         m_chOutBuff = NULL;
     }

     m_msgBody = cJSON_CreateObject();
     cJSON_AddStringToObject(m_msgBody, "return", "success");
     m_chOutBuff = cJSON_Print(m_msgBody);
     memcpy(m_chSendBuff + dwSendBuffLen, m_chOutBuff, strlen(m_chOutBuff));
     dwSendBuffLen += strlen(m_chOutBuff);

     return dwSendBuffLen;	
}

int COpenATCCommWithCfgSWThread::AckCfg_SendCheckDataFailed(TErrInfo *pCheckCode)
{
	cJSON* erroCodeArray = NULL;
	cJSON* erroCodeArrayInfo[C_N_MAX_CHECKERR_SIZE] = { NULL };
	unsigned int dwSendBuffLen = 0;

	memset(m_chSendBuff, 0x00, SEND_BUFFER_SIZE);

    m_chSendBuff[dwSendBuffLen++] = CB_VERSION_FLAG;									//版本号
	m_chSendBuff[dwSendBuffLen++] = CB_ATC_FLAG;										//发送方标识
	m_chSendBuff[dwSendBuffLen++] = CB_CONFIGSOFTWARE_FLAG;								//接收方标识
	m_chSendBuff[dwSendBuffLen++] = LINK_CONFIG;										//数据链路码
	m_chSendBuff[dwSendBuffLen++] = m_tAreaInfo.m_chAscRegionNo;						//区域号
	memcpy(m_chSendBuff + dwSendBuffLen, &m_tAreaInfo.m_usAscRoadNo, MAX_ROADNO_LENGTH);//路口号
    dwSendBuffLen += MAX_ROADNO_LENGTH;
	m_chSendBuff[dwSendBuffLen++] = ACK_WRONG;										    //操作类型
	m_chSendBuff[dwSendBuffLen++] = CFG_ACK_SENDDATA_FAILED;							//对象标识
	m_chSendBuff[dwSendBuffLen++] = 0x01;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留

	if (m_msgBody != NULL)
	{
		cJSON_Delete(m_msgBody);
		m_msgBody = NULL;
	}

	if (m_chOutBuff != NULL)
	{
		free(m_chOutBuff);
		m_chOutBuff = NULL;
	}

	m_msgBody = cJSON_CreateObject();
	erroCodeArray = cJSON_CreateArray();
	cJSON_AddItemToObject(m_msgBody, "errorCode", erroCodeArray);
	for (int iErrList = 0; iErrList < C_N_MAX_ERR_SIZE; iErrList++)
	{
		if (pCheckCode[iErrList].nCheckErr > 0)
		{
			cJSON* errSubCodeArray = NULL;
			errSubCodeArray = cJSON_CreateArray();
			cJSON_AddNumberToObject(errSubCodeArray, "", pCheckCode[iErrList].nCheckErr);
			cJSON_AddNumberToObject(errSubCodeArray, "", pCheckCode[iErrList].nSubCheckErr);
			//子类型2
			cJSON_AddNumberToObject(errSubCodeArray, "", pCheckCode[iErrList].nSubTwoCheckErr);
			cJSON_AddItemToObject(erroCodeArray, "", errSubCodeArray);
		}
		else
		{
			break;
		}
	}
	m_chOutBuff = cJSON_Print(m_msgBody);
	memcpy(m_chSendBuff + dwSendBuffLen, m_chOutBuff, strlen(m_chOutBuff));

	dwSendBuffLen += strlen(m_chOutBuff);

	return dwSendBuffLen;
}

int COpenATCCommWithCfgSWThread::AckCtl_AskSetFTPFileUpdateCmd()
{
	unsigned int dwSendBuffLen = 0;

	memset(m_chSendBuff, 0x00, SEND_BUFFER_SIZE);

	m_chSendBuff[dwSendBuffLen++] = CB_VERSION_FLAG;									//版本号
	m_chSendBuff[dwSendBuffLen++] = CB_ATC_FLAG;										//发送方标识
	m_chSendBuff[dwSendBuffLen++] = CB_CONFIGSOFTWARE_FLAG;								//接收方标识
	m_chSendBuff[dwSendBuffLen++] = LINK_ALTERNATEFEATUREPARA;							//数据链路码
	m_chSendBuff[dwSendBuffLen++] = m_tAreaInfo.m_chAscRegionNo;						//区域号
	memcpy(m_chSendBuff + dwSendBuffLen, &m_tAreaInfo.m_usAscRoadNo, MAX_ROADNO_LENGTH);//路口号
	dwSendBuffLen += MAX_ROADNO_LENGTH;
	m_chSendBuff[dwSendBuffLen++] = ACK_SET;											//操作类型
	m_chSendBuff[dwSendBuffLen++] = CTL_SYSTEM_UPDATE;									//对象标识
	m_chSendBuff[dwSendBuffLen++] = 0x01;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留

	if (m_msgBody != NULL)
	{
		cJSON_Delete(m_msgBody);
		m_msgBody = NULL;
	}

	if (m_chOutBuff != NULL)
	{
		free(m_chOutBuff);
		m_chOutBuff = NULL;
	}

	m_msgBody = cJSON_CreateObject();
	cJSON_AddStringToObject(m_msgBody, "return", "success");
	m_chOutBuff = cJSON_Print(m_msgBody);
	memcpy(m_chSendBuff + dwSendBuffLen, m_chOutBuff, strlen(m_chOutBuff));
	dwSendBuffLen += strlen(m_chOutBuff);

	return dwSendBuffLen;	
}

int COpenATCCommWithCfgSWThread::AckCfg_AskReadFailed()
{
    unsigned int dwSendBuffLen = 0;

    memset(m_chSendBuff, 0x00, SEND_BUFFER_SIZE);

    m_chSendBuff[dwSendBuffLen++] = CB_VERSION_FLAG;									//版本号
	m_chSendBuff[dwSendBuffLen++] = CB_ATC_FLAG;										//发送方标识
	m_chSendBuff[dwSendBuffLen++] = CB_CONFIGSOFTWARE_FLAG;								//接收方标识
	m_chSendBuff[dwSendBuffLen++] = LINK_CONFIG;										//数据链路码
	m_chSendBuff[dwSendBuffLen++] = m_tAreaInfo.m_chAscRegionNo;						//区域号
	memcpy(m_chSendBuff + dwSendBuffLen, &m_tAreaInfo.m_usAscRoadNo, MAX_ROADNO_LENGTH);//路口号
    dwSendBuffLen += MAX_ROADNO_LENGTH;
	m_chSendBuff[dwSendBuffLen++] = ACK_WRONG;											//操作类型
	m_chSendBuff[dwSendBuffLen++] = CFG_ACK_ASKREAD_FAILED;								//对象标识
	m_chSendBuff[dwSendBuffLen++] = 0x01;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留

    if (m_msgBody != NULL)
    {
        cJSON_Delete(m_msgBody);
        m_msgBody = NULL;
    }

    if (m_chOutBuff != NULL)
    {
        free(m_chOutBuff);
        m_chOutBuff = NULL;
    }

    m_msgBody = cJSON_CreateObject();
    cJSON_AddStringToObject(m_msgBody, "return", "failed");
    m_chOutBuff = cJSON_Print(m_msgBody);
    memcpy(m_chSendBuff + dwSendBuffLen, m_chOutBuff, strlen(m_chOutBuff));

    dwSendBuffLen += strlen(m_chOutBuff);
    
    return dwSendBuffLen;
}

int COpenATCCommWithCfgSWThread::AckCfg_ReadData(void *pCfgData, int nSize)
{
    unsigned int dwSendBuffLen = 0;

    memset(m_chSendBuff, 0x00, SEND_BUFFER_SIZE);

	m_chSendBuff[dwSendBuffLen++] = CB_VERSION_FLAG;									//版本号
	m_chSendBuff[dwSendBuffLen++] = CB_ATC_FLAG;										//发送方标识
	m_chSendBuff[dwSendBuffLen++] = CB_CONFIGSOFTWARE_FLAG;								//接收方标识
	m_chSendBuff[dwSendBuffLen++] = LINK_CONFIG;										//数据链路码
	m_chSendBuff[dwSendBuffLen++] = m_tAreaInfo.m_chAscRegionNo;						//区域号
	memcpy(m_chSendBuff + dwSendBuffLen, &m_tAreaInfo.m_usAscRoadNo, MAX_ROADNO_LENGTH);//路口号
    dwSendBuffLen += MAX_ROADNO_LENGTH;
	m_chSendBuff[dwSendBuffLen++] = ACK_QUERY;											//操作类型
	m_chSendBuff[dwSendBuffLen++] = CFG_ACK_ASKSEND;									//对象标识
	m_chSendBuff[dwSendBuffLen++] = 0x01;												//保留
    memcpy(m_chSendBuff + dwSendBuffLen, &nSize, 4);
    dwSendBuffLen += 4;

    memcpy(m_chSendBuff + dwSendBuffLen, pCfgData, nSize);
    dwSendBuffLen += nSize;
    
    return dwSendBuffLen;
}

int COpenATCCommWithCfgSWThread::AckCtl_AskLogin()
{
    unsigned int dwSendBuffLen = 0;

    memset(m_chSendBuff, 0x00, SEND_BUFFER_SIZE);
	
	m_chSendBuff[dwSendBuffLen++] = CB_VERSION_FLAG;									//版本号
	m_chSendBuff[dwSendBuffLen++] = CB_ATC_FLAG;										//发送方标识
	m_chSendBuff[dwSendBuffLen++] = CB_CONFIGSOFTWARE_FLAG;								//接收方标识
	m_chSendBuff[dwSendBuffLen++] = LINK_CONTROL;										//数据链路码
	m_chSendBuff[dwSendBuffLen++] = m_tAreaInfo.m_chAscRegionNo;						//区域号
    memcpy(m_chSendBuff + dwSendBuffLen, &m_tAreaInfo.m_usAscRoadNo, MAX_ROADNO_LENGTH);//路口号
    dwSendBuffLen += MAX_ROADNO_LENGTH;
	m_chSendBuff[dwSendBuffLen++] = ACK_QUERY;											//操作类型
	m_chSendBuff[dwSendBuffLen++] = CTL_ACK_LOGIN;										//对象标识
	m_chSendBuff[dwSendBuffLen++] = 0x01;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留

	unsigned char *pBuffer = m_pOpenATCParameter->GetAscLoginCenterData();
	if (pBuffer != NULL)
	{
		memcpy(m_chSendBuff + dwSendBuffLen, pBuffer, strlen((const char *)pBuffer));
		dwSendBuffLen += strlen((const char *)pBuffer);
	}

	if (pBuffer != NULL)
	{
		free(pBuffer);
		pBuffer = NULL;
	}
	return dwSendBuffLen;
}

int COpenATCCommWithCfgSWThread::AckCtl_AskUpdateFile()
{   
    unsigned int dwSendBuffLen = 0;

    memset(m_chSendBuff, 0x00, SEND_BUFFER_SIZE);

    m_chSendBuff[dwSendBuffLen++] = CB_VERSION_FLAG;									//版本号
	m_chSendBuff[dwSendBuffLen++] = CB_ATC_FLAG;										//发送方标识
	m_chSendBuff[dwSendBuffLen++] = CB_CONFIGSOFTWARE_FLAG;								//接收方标识
	m_chSendBuff[dwSendBuffLen++] = LINK_CONTROL;										//数据链路码
	m_chSendBuff[dwSendBuffLen++] = m_tAreaInfo.m_chAscRegionNo;						//区域号
	memcpy(m_chSendBuff + dwSendBuffLen, &m_tAreaInfo.m_usAscRoadNo, MAX_ROADNO_LENGTH);//路口号
    dwSendBuffLen += MAX_ROADNO_LENGTH;
	m_chSendBuff[dwSendBuffLen++] = 0x81;												//操作类型
	m_chSendBuff[dwSendBuffLen++] = CTL_ACK_UPDATEFILE;									//对象标识
	m_chSendBuff[dwSendBuffLen++] = 0x01;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
 
    if (m_msgBody != NULL)
    {
         cJSON_Delete(m_msgBody);
         m_msgBody = NULL;
    }

    if (m_chOutBuff != NULL)
    {
         free(m_chOutBuff);
         m_chOutBuff = NULL;
    }

    m_msgBody = cJSON_CreateObject();
    cJSON_AddStringToObject(m_msgBody, "return", "success");
    m_chOutBuff = cJSON_Print(m_msgBody);
    memcpy(m_chSendBuff + dwSendBuffLen, m_chOutBuff, strlen(m_chOutBuff));
    dwSendBuffLen += strlen(m_chOutBuff);

    return dwSendBuffLen;
}

int COpenATCCommWithCfgSWThread::AckCtl_AskSendFileBlock()
{   
    unsigned int dwSendBuffLen = 0;

    memset(m_chSendBuff, 0x00, SEND_BUFFER_SIZE);

    m_chSendBuff[dwSendBuffLen++] = CB_VERSION_FLAG;									//版本号
	m_chSendBuff[dwSendBuffLen++] = CB_ATC_FLAG;										//发送方标识
	m_chSendBuff[dwSendBuffLen++] = CB_CONFIGSOFTWARE_FLAG;								//接收方标识
	m_chSendBuff[dwSendBuffLen++] = LINK_CONTROL;										//数据链路码
	m_chSendBuff[dwSendBuffLen++] = m_tAreaInfo.m_chAscRegionNo;						//区域号
	memcpy(m_chSendBuff + dwSendBuffLen, &m_tAreaInfo.m_usAscRoadNo, MAX_ROADNO_LENGTH);//路口号
    dwSendBuffLen += MAX_ROADNO_LENGTH;
	m_chSendBuff[dwSendBuffLen++] = 0x81;												//操作类型
	m_chSendBuff[dwSendBuffLen++] = CTL_ACK_UPDATEFILE;									//对象标识
	m_chSendBuff[dwSendBuffLen++] = 0x01;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留

    if (m_msgBody != NULL)
    {
        cJSON_Delete(m_msgBody);
        m_msgBody = NULL;
    }

    if (m_chOutBuff != NULL)
    {
        free(m_chOutBuff);
        m_chOutBuff = NULL;
    }

    m_msgBody = cJSON_CreateObject();
    cJSON_AddStringToObject(m_msgBody, "return", "success");
    m_chOutBuff = cJSON_Print(m_msgBody);
    memcpy(m_chSendBuff + dwSendBuffLen, m_chOutBuff, strlen(m_chOutBuff));
    dwSendBuffLen += strlen(m_chOutBuff);

    return dwSendBuffLen;
}

int COpenATCCommWithCfgSWThread::AckCtl_AskWorkStatus()
{
    unsigned int dwSendBuffLen = 0;

    memset(m_chSendBuff, 0x00, SEND_BUFFER_SIZE);

	m_chSendBuff[dwSendBuffLen++] = CB_VERSION_FLAG;									//版本号
	m_chSendBuff[dwSendBuffLen++] = CB_ATC_FLAG;										//发送方标识
	m_chSendBuff[dwSendBuffLen++] = CB_CONFIGSOFTWARE_FLAG;								//接收方标识
	m_chSendBuff[dwSendBuffLen++] = LINK_BASEINFO;										//数据链路码
	m_chSendBuff[dwSendBuffLen++] = m_tAreaInfo.m_chAscRegionNo;						//区域号
	memcpy(m_chSendBuff + dwSendBuffLen, &m_tAreaInfo.m_usAscRoadNo, MAX_ROADNO_LENGTH);//路口号
    dwSendBuffLen += MAX_ROADNO_LENGTH;
	m_chSendBuff[dwSendBuffLen++] = ACK_QUERY;											//操作类型
	m_chSendBuff[dwSendBuffLen++] = CTL_WORKSTATUS;										//对象标识
	m_chSendBuff[dwSendBuffLen++] = 0x01;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留

    unsigned char *pBuffer = m_pOpenATCParameter->GetPatternRunStatusData();
    if (pBuffer != NULL)
    {    
        memcpy(m_chSendBuff + dwSendBuffLen, pBuffer, strlen((const char *)pBuffer));
        dwSendBuffLen += strlen((const char *)pBuffer);
    }

    if (pBuffer != NULL)
    {
        free(pBuffer);
        pBuffer = NULL;
    }

    return dwSendBuffLen;
}

int COpenATCCommWithCfgSWThread::AckCtl_AskNullWorkStatus()
{
	unsigned int dwSendBuffLen = 0;

	memset(m_chSendBuff, 0x00, SEND_BUFFER_SIZE);

	m_chSendBuff[dwSendBuffLen++] = CB_VERSION_FLAG;									//版本号
	m_chSendBuff[dwSendBuffLen++] = CB_ATC_FLAG;										//发送方标识
	m_chSendBuff[dwSendBuffLen++] = CB_CONFIGSOFTWARE_FLAG;								//接收方标识
	m_chSendBuff[dwSendBuffLen++] = LINK_BASEINFO;										//数据链路码
	m_chSendBuff[dwSendBuffLen++] = m_tAreaInfo.m_chAscRegionNo;						//区域号
	memcpy(m_chSendBuff + dwSendBuffLen, &m_tAreaInfo.m_usAscRoadNo, MAX_ROADNO_LENGTH);//路口号
	dwSendBuffLen += MAX_ROADNO_LENGTH;
	m_chSendBuff[dwSendBuffLen++] = ACK_QUERY;											//操作类型
	m_chSendBuff[dwSendBuffLen++] = CTL_WORKSTATUS;										//对象标识
	m_chSendBuff[dwSendBuffLen++] = 0x01;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留

	if (m_msgBody != NULL)
	{
		cJSON_Delete(m_msgBody);
		m_msgBody = NULL;
	}

	if (m_chOutBuff != NULL)
	{
		free(m_chOutBuff);
		m_chOutBuff = NULL;
	}

	m_msgBody = cJSON_CreateObject();
	cJSON_AddStringToObject(m_msgBody, "return", "success");
	m_chOutBuff = cJSON_Print(m_msgBody);
	memcpy(m_chSendBuff + dwSendBuffLen, m_chOutBuff, strlen(m_chOutBuff));

	dwSendBuffLen += strlen(m_chOutBuff);

	return dwSendBuffLen;
}

int COpenATCCommWithCfgSWThread::AckCtl_AskLampClrStatus()
{
    int i = 0;
    unsigned int dwSendBuffLen = 0;

	memset(m_chSendBuff, 0x00, SEND_BUFFER_SIZE);

	m_chSendBuff[dwSendBuffLen++] = CB_VERSION_FLAG;									//版本号
	m_chSendBuff[dwSendBuffLen++] = CB_ATC_FLAG;										//发送方标识
	m_chSendBuff[dwSendBuffLen++] = CB_CONFIGSOFTWARE_FLAG;								//接收方标识
	m_chSendBuff[dwSendBuffLen++] = LINK_BASEINFO;										//数据链路码
	m_chSendBuff[dwSendBuffLen++] = m_tAreaInfo.m_chAscRegionNo;						//区域号
	memcpy(m_chSendBuff + dwSendBuffLen, &m_tAreaInfo.m_usAscRoadNo, MAX_ROADNO_LENGTH);//路口号
    dwSendBuffLen += MAX_ROADNO_LENGTH;
	m_chSendBuff[dwSendBuffLen++] = ACK_QUERY;											//操作类型
	m_chSendBuff[dwSendBuffLen++] = CTL_LAMPCOLORSTATUS;								//对象标识
	m_chSendBuff[dwSendBuffLen++] = 0x01;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留

    unsigned char *pBuffer = m_pOpenATCParameter->GetChannelLampStatusInfo();
    if (pBuffer != NULL)
    {    
        memcpy(m_chSendBuff + dwSendBuffLen, pBuffer, strlen((const char *)pBuffer));
        dwSendBuffLen += strlen((const char *)pBuffer);
    }

    if (pBuffer != NULL)
    {
        free(pBuffer);
        pBuffer = NULL;
    }

	return dwSendBuffLen;
}

int COpenATCCommWithCfgSWThread::AckCtl_AskQueryATCLocalTime()
{
    unsigned int dwSendBuffLen = 0;

	memset(m_chSendBuff, 0x00, SEND_BUFFER_SIZE);

	m_chSendBuff[dwSendBuffLen++] = CB_VERSION_FLAG;									//版本号
	m_chSendBuff[dwSendBuffLen++] = CB_ATC_FLAG;										//发送方标识
	m_chSendBuff[dwSendBuffLen++] = CB_CONFIGSOFTWARE_FLAG;								//接收方标识
	m_chSendBuff[dwSendBuffLen++] = LINK_BASEINFO;										//数据链路码
	m_chSendBuff[dwSendBuffLen++] = m_tAreaInfo.m_chAscRegionNo;						//区域号
	memcpy(m_chSendBuff + dwSendBuffLen, &m_tAreaInfo.m_usAscRoadNo, MAX_ROADNO_LENGTH);//路口号
    dwSendBuffLen += MAX_ROADNO_LENGTH;
	m_chSendBuff[dwSendBuffLen++] = ACK_QUERY;											//操作类型
	m_chSendBuff[dwSendBuffLen++] = CTL_CURRENTTIME;									//对象标识
	m_chSendBuff[dwSendBuffLen++] = 0x01;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留

    unsigned char *pBuffer = m_pOpenATCParameter->GetATCLocalTime();
    if (pBuffer != NULL)
    {    
        memcpy(m_chSendBuff + dwSendBuffLen, pBuffer, strlen((const char *)pBuffer));
        dwSendBuffLen += strlen((const char *)pBuffer);
    }

    if (pBuffer != NULL)
    {
        free(pBuffer);
        pBuffer = NULL;
    }

	return dwSendBuffLen;
}

int COpenATCCommWithCfgSWThread::AckCtl_AskSetATCLocalTime()
{
    unsigned int dwSendBuffLen = 0;

	memset(m_chSendBuff, 0x00, SEND_BUFFER_SIZE);

	m_chSendBuff[dwSendBuffLen++] = CB_VERSION_FLAG;									//版本号
	m_chSendBuff[dwSendBuffLen++] = CB_ATC_FLAG;										//发送方标识
	m_chSendBuff[dwSendBuffLen++] = CB_CONFIGSOFTWARE_FLAG;								//接收方标识
	m_chSendBuff[dwSendBuffLen++] = LINK_BASEINFO;										//数据链路码
	m_chSendBuff[dwSendBuffLen++] = m_tAreaInfo.m_chAscRegionNo;						//区域号
	memcpy(m_chSendBuff + dwSendBuffLen, &m_tAreaInfo.m_usAscRoadNo, MAX_ROADNO_LENGTH);//路口号
    dwSendBuffLen += MAX_ROADNO_LENGTH;
	m_chSendBuff[dwSendBuffLen++] = ACK_SET;											//操作类型
	m_chSendBuff[dwSendBuffLen++] = CTL_CURRENTTIME;									//对象标识
	m_chSendBuff[dwSendBuffLen++] = 0x01;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留

	return dwSendBuffLen;
}

int COpenATCCommWithCfgSWThread::AckCtl_AskQuerySignalLightGroup()
{
    unsigned int dwSendBuffLen = 0;

    memset(m_chSendBuff, 0x00, SEND_BUFFER_SIZE);

	m_chSendBuff[dwSendBuffLen++] = CB_VERSION_FLAG;									//版本号
	m_chSendBuff[dwSendBuffLen++] = CB_ATC_FLAG;										//发送方标识
	m_chSendBuff[dwSendBuffLen++] = CB_CONFIGSOFTWARE_FLAG;								//接收方标识
	m_chSendBuff[dwSendBuffLen++] = LINK_ALTERNATEFEATUREPARA;							//数据链路码
	m_chSendBuff[dwSendBuffLen++] = m_tAreaInfo.m_chAscRegionNo;						//区域号
	memcpy(m_chSendBuff + dwSendBuffLen, &m_tAreaInfo.m_usAscRoadNo, MAX_ROADNO_LENGTH);//路口号
    dwSendBuffLen += MAX_ROADNO_LENGTH;
	m_chSendBuff[dwSendBuffLen++] = ACK_QUERY;											//操作类型
	m_chSendBuff[dwSendBuffLen++] = CTL_SIGNALLIGHTGROUP;								//对象标识
	m_chSendBuff[dwSendBuffLen++] = 0x01;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留

    unsigned char *pBuffer = m_pOpenATCParameter->GetChannelParamData();
    if (pBuffer != NULL)
    {    
        memcpy(m_chSendBuff + dwSendBuffLen, pBuffer, strlen((const char *)pBuffer));
        dwSendBuffLen += strlen((const char *)pBuffer);
    }

    if (pBuffer != NULL)
    {
        free(pBuffer);
        pBuffer = NULL;
    }

	return dwSendBuffLen;
}

int COpenATCCommWithCfgSWThread::AckCtl_AskSetSignalLightGroup()
{
    unsigned int dwSendBuffLen = 0;

	memset(m_chSendBuff, 0x00, SEND_BUFFER_SIZE);

	m_chSendBuff[dwSendBuffLen++] = CB_VERSION_FLAG;									//版本号
	m_chSendBuff[dwSendBuffLen++] = CB_ATC_FLAG;										//发送方标识
	m_chSendBuff[dwSendBuffLen++] = CB_CONFIGSOFTWARE_FLAG;								//接收方标识
	m_chSendBuff[dwSendBuffLen++] = LINK_ALTERNATEFEATUREPARA;							//数据链路码
	m_chSendBuff[dwSendBuffLen++] = m_tAreaInfo.m_chAscRegionNo;						//区域号
	memcpy(m_chSendBuff + dwSendBuffLen, &m_tAreaInfo.m_usAscRoadNo, MAX_ROADNO_LENGTH);//路口号
    dwSendBuffLen += MAX_ROADNO_LENGTH;
	m_chSendBuff[dwSendBuffLen++] = ACK_SET;											//操作类型
	m_chSendBuff[dwSendBuffLen++] = CTL_SIGNALLIGHTGROUP;								//对象标识
	m_chSendBuff[dwSendBuffLen++] = 0x01;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留

	return dwSendBuffLen;
}

int COpenATCCommWithCfgSWThread::AckCtl_AskQueryPhaseParamData()
{
    unsigned int dwSendBuffLen = 0;
	memset(m_chSendBuff, 0x00, SEND_BUFFER_SIZE);

	m_chSendBuff[dwSendBuffLen++] = CB_VERSION_FLAG;									//版本号
	m_chSendBuff[dwSendBuffLen++] = CB_ATC_FLAG;										//发送方标识
	m_chSendBuff[dwSendBuffLen++] = CB_CONFIGSOFTWARE_FLAG;								//接收方标识
	m_chSendBuff[dwSendBuffLen++] = LINK_ALTERNATEFEATUREPARA;							//数据链路码
	m_chSendBuff[dwSendBuffLen++] = m_tAreaInfo.m_chAscRegionNo;						//区域号
	memcpy(m_chSendBuff + dwSendBuffLen, &m_tAreaInfo.m_usAscRoadNo, MAX_ROADNO_LENGTH);//路口号
    dwSendBuffLen += MAX_ROADNO_LENGTH;
	m_chSendBuff[dwSendBuffLen++] = ACK_QUERY;											//操作类型
	m_chSendBuff[dwSendBuffLen++] = CTL_PHASE;											//对象标识
	m_chSendBuff[dwSendBuffLen++] = 0x01;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留

    unsigned char *pBuffer = m_pOpenATCParameter->GetPhaseParamData();
    if (pBuffer != NULL)
    {    
        memcpy(m_chSendBuff + dwSendBuffLen, pBuffer, strlen((const char *)pBuffer));
        dwSendBuffLen += strlen((const char *)pBuffer);
    }

    if (pBuffer != NULL)
    {
        free(pBuffer);
        pBuffer = NULL;
    }
    
	return dwSendBuffLen;
}

int COpenATCCommWithCfgSWThread::AckCtl_AskSetPhaseParamData()
{
    unsigned int dwSendBuffLen = 0;

	memset(m_chSendBuff, 0x00, SEND_BUFFER_SIZE);

	m_chSendBuff[dwSendBuffLen++] = CB_VERSION_FLAG;									//版本号
	m_chSendBuff[dwSendBuffLen++] = CB_ATC_FLAG;										//发送方标识
	m_chSendBuff[dwSendBuffLen++] = CB_CONFIGSOFTWARE_FLAG;								//接收方标识
	m_chSendBuff[dwSendBuffLen++] = LINK_ALTERNATEFEATUREPARA;							//数据链路码
	m_chSendBuff[dwSendBuffLen++] = m_tAreaInfo.m_chAscRegionNo;						//区域号
	memcpy(m_chSendBuff + dwSendBuffLen, &m_tAreaInfo.m_usAscRoadNo, MAX_ROADNO_LENGTH);//路口号
    dwSendBuffLen += MAX_ROADNO_LENGTH;
	m_chSendBuff[dwSendBuffLen++] = ACK_WRONG;											//操作类型
	m_chSendBuff[dwSendBuffLen++] = CTL_PHASE;											//对象标识
	m_chSendBuff[dwSendBuffLen++] = 0x01;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	
	m_msgBody = cJSON_CreateObject();
	cJSON_AddNumberToObject(m_msgBody, "success", NO_SUPPORT_CONTROL_PARAM);
	m_chOutBuff = cJSON_Print(m_msgBody);
	memcpy(m_chSendBuff + dwSendBuffLen, m_chOutBuff, strlen(m_chOutBuff));

	dwSendBuffLen += strlen(m_chOutBuff);

	return dwSendBuffLen;
}

int COpenATCCommWithCfgSWThread::AckCtl_AskQueryPartParamData(BYTE byType)
{
    unsigned int dwSendBuffLen = 0;

	memset(m_chSendBuff, 0x00, SEND_BUFFER_SIZE);

	m_chSendBuff[dwSendBuffLen++] = CB_VERSION_FLAG;									//版本号
	m_chSendBuff[dwSendBuffLen++] = CB_ATC_FLAG;										//发送方标识
	m_chSendBuff[dwSendBuffLen++] = CB_CONFIGSOFTWARE_FLAG;								//接收方标识
	m_chSendBuff[dwSendBuffLen++] = LINK_ALTERNATEFEATUREPARA;							//数据链路码
	m_chSendBuff[dwSendBuffLen++] = m_tAreaInfo.m_chAscRegionNo;						//区域号
	memcpy(m_chSendBuff + dwSendBuffLen, &m_tAreaInfo.m_usAscRoadNo, MAX_ROADNO_LENGTH);//路口号
    dwSendBuffLen += MAX_ROADNO_LENGTH;
	m_chSendBuff[dwSendBuffLen++] = ACK_QUERY;											//操作类型
	if (byType == CheckParam_Plan)
	{
		m_chSendBuff[dwSendBuffLen++] = CTL_PROGRAMMESCHEDULEPLAN;						//对象标识
	} 
	else if (byType == CheckParam_Pattern)
	{
		m_chSendBuff[dwSendBuffLen++] = CTL_SIGNALMATCHTIME;							//对象标识
	}
	else if (byType == CheckParam_Date)
	{
		m_chSendBuff[dwSendBuffLen++] = CTL_SCHEDUL_DATE;								//对象标识
	}
	else if (byType == CheckParam_OverLap)
	{
		m_chSendBuff[dwSendBuffLen++] = CTL_OVERLAP;									//对象标识
	}
	
	m_chSendBuff[dwSendBuffLen++] = 0x01;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留

    unsigned char *pBuffer;

	if (byType == CheckParam_Plan)
	{
		pBuffer = m_pOpenATCParameter->GetPlanParamData();
	} 
	else if (byType == CheckParam_Pattern)
	{
		pBuffer = m_pOpenATCParameter->GetPatternParamData();
	}
	else if (byType == CheckParam_Date)
	{
		pBuffer = m_pOpenATCParameter->GetDateParamData();
	}
	else if (byType == CheckParam_OverLap)
	{
		pBuffer = m_pOpenATCParameter->GetOverLapParamData();
	}

    if (pBuffer != NULL)
    {    
        memcpy(m_chSendBuff + dwSendBuffLen, pBuffer, strlen((const char *)pBuffer));
        dwSendBuffLen += strlen((const char *)pBuffer);
    }

    if (pBuffer != NULL)
    {
        free(pBuffer);
        pBuffer = NULL;
    }

	return dwSendBuffLen;
}

int COpenATCCommWithCfgSWThread::AckCtl_AskQuerySystemCustomData()
{
	unsigned int dwSendBuffLen = 0;

	memset(m_chSendBuff, 0x00, SEND_BUFFER_SIZE);

	m_chSendBuff[dwSendBuffLen++] = CB_VERSION_FLAG;									//版本号
	m_chSendBuff[dwSendBuffLen++] = CB_ATC_FLAG;										//发送方标识
	m_chSendBuff[dwSendBuffLen++] = CB_CONFIGSOFTWARE_FLAG;								//接收方标识
	m_chSendBuff[dwSendBuffLen++] = LINK_ALTERNATEFEATUREPARA;							//数据链路码
	m_chSendBuff[dwSendBuffLen++] = m_tAreaInfo.m_chAscRegionNo;						//区域号
	memcpy(m_chSendBuff + dwSendBuffLen, &m_tAreaInfo.m_usAscRoadNo, MAX_ROADNO_LENGTH);//路口号
	dwSendBuffLen += MAX_ROADNO_LENGTH;
	m_chSendBuff[dwSendBuffLen++] = ACK_QUERY;											//操作类型
	m_chSendBuff[dwSendBuffLen++] = CTL_SYSTEM_CUSTOM;									//对象标识
	m_chSendBuff[dwSendBuffLen++] = 0x01;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留

	unsigned char *pBuffer = m_pOpenATCParameter->GetSystemCustom();
	if (pBuffer != NULL)
	{
		memcpy(m_chSendBuff + dwSendBuffLen, pBuffer, strlen((const char *)pBuffer));
		dwSendBuffLen += strlen((const char *)pBuffer);
	}
	return dwSendBuffLen;
}

int COpenATCCommWithCfgSWThread::AckCtl_AskSetSystemCustomData()
{
	unsigned int dwSendBuffLen = 0;

	memset(m_chSendBuff, 0x00, SEND_BUFFER_SIZE);

	m_chSendBuff[dwSendBuffLen++] = CB_VERSION_FLAG;									//版本号
	m_chSendBuff[dwSendBuffLen++] = CB_ATC_FLAG;										//发送方标识
	m_chSendBuff[dwSendBuffLen++] = CB_CONFIGSOFTWARE_FLAG;								//接收方标识
	m_chSendBuff[dwSendBuffLen++] = LINK_ALTERNATEFEATUREPARA;							//数据链路码
	m_chSendBuff[dwSendBuffLen++] = m_tAreaInfo.m_chAscRegionNo;						//区域号
	memcpy(m_chSendBuff + dwSendBuffLen, &m_tAreaInfo.m_usAscRoadNo, MAX_ROADNO_LENGTH);//路口号
	dwSendBuffLen += MAX_ROADNO_LENGTH;
	m_chSendBuff[dwSendBuffLen++] = ACK_SET;											//操作类型
	m_chSendBuff[dwSendBuffLen++] = CTL_SYSTEM_CUSTOM;									//对象标识
	m_chSendBuff[dwSendBuffLen++] = 0x01;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留

	return dwSendBuffLen;
}

int COpenATCCommWithCfgSWThread::AckCtl_AskSetPartParamData(BYTE byType, int nResult, TErrInfo *pCheckCode)
{
	cJSON* erroCodeArray = NULL;
	cJSON* erroCodeArrayInfo[C_N_MAX_CHECKERR_SIZE] = { NULL };
    unsigned int dwSendBuffLen = 0;

	memset(m_chSendBuff, 0x00, SEND_BUFFER_SIZE);

	m_chSendBuff[dwSendBuffLen++] = CB_VERSION_FLAG;									//版本号
	m_chSendBuff[dwSendBuffLen++] = CB_ATC_FLAG;										//发送方标识
	m_chSendBuff[dwSendBuffLen++] = CB_CONFIGSOFTWARE_FLAG;								//接收方标识
	m_chSendBuff[dwSendBuffLen++] = LINK_ALTERNATEFEATUREPARA;							//数据链路码
	m_chSendBuff[dwSendBuffLen++] = m_tAreaInfo.m_chAscRegionNo;						//区域号
	memcpy(m_chSendBuff + dwSendBuffLen, &m_tAreaInfo.m_usAscRoadNo, MAX_ROADNO_LENGTH);//路口号
    dwSendBuffLen += MAX_ROADNO_LENGTH;
	if (nResult == OPENATC_RTN_FAILED)
	{
		m_chSendBuff[dwSendBuffLen++] = ACK_WRONG;										//操作类型
	} 
	else
	{
		m_chSendBuff[dwSendBuffLen++] = ACK_SET;										//操作类型
	}
	if (byType == CheckParam_Plan)
	{
		m_chSendBuff[dwSendBuffLen++] = CTL_PROGRAMMESCHEDULEPLAN;						//对象标识
	} 
	else if (byType == CheckParam_Pattern)
	{
		m_chSendBuff[dwSendBuffLen++] = CTL_SIGNALMATCHTIME;							//对象标识
	}
	else if (byType == CheckParam_Date)
	{
		m_chSendBuff[dwSendBuffLen++] = CTL_SCHEDUL_DATE;								//对象标识
	}
	else if (byType == CheckParam_OverLap)
	{
		m_chSendBuff[dwSendBuffLen++] = CTL_OVERLAP;									//对象标识
	}
	m_chSendBuff[dwSendBuffLen++] = 0x01;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留

	if (m_msgBody != NULL)
	{
		cJSON_Delete(m_msgBody);
		m_msgBody = NULL;
	}

	if (m_chOutBuff != NULL)
	{
		free(m_chOutBuff);
		m_chOutBuff = NULL;
	}


	m_msgBody = cJSON_CreateObject();
	if (nResult == OPENATC_RTN_OK)
	{
		cJSON_AddStringToObject(m_msgBody, "return", "success");
	}
	else
	{
		erroCodeArray = cJSON_CreateArray();
		cJSON_AddItemToObject(m_msgBody, "errorCode", erroCodeArray);
		for (int iErrList = 0; iErrList < C_N_MAX_ERR_SIZE; iErrList++)
		{
			if (pCheckCode[iErrList].nCheckErr > 0)
			{
				cJSON* errSubCodeArray = NULL;
				errSubCodeArray = cJSON_CreateArray();
				cJSON_AddNumberToObject(errSubCodeArray, "", pCheckCode[iErrList].nCheckErr);
				cJSON_AddNumberToObject(errSubCodeArray, "", pCheckCode[iErrList].nSubCheckErr);
				cJSON_AddItemToObject(erroCodeArray, "", errSubCodeArray);
			}
			else
			{
				break;
			}
		}
	}
	m_chOutBuff = cJSON_Print(m_msgBody);
	memcpy(m_chSendBuff + dwSendBuffLen, m_chOutBuff, strlen(m_chOutBuff));
	dwSendBuffLen += strlen(m_chOutBuff);

	return dwSendBuffLen;
}

int COpenATCCommWithCfgSWThread::AckCtl_AskQueryWorkWay()
{
    unsigned int dwSendBuffLen = 0;

	memset(m_chSendBuff, 0x00, SEND_BUFFER_SIZE);

	m_chSendBuff[dwSendBuffLen++] = CB_VERSION_FLAG;									//版本号
	m_chSendBuff[dwSendBuffLen++] = CB_ATC_FLAG;										//发送方标识
	m_chSendBuff[dwSendBuffLen++] = CB_CONFIGSOFTWARE_FLAG;								//接收方标识
	m_chSendBuff[dwSendBuffLen++] = LINK_INTERVENECOMMAND;								//数据链路码
	m_chSendBuff[dwSendBuffLen++] = m_tAreaInfo.m_chAscRegionNo;						//区域号
	memcpy(m_chSendBuff + dwSendBuffLen, &m_tAreaInfo.m_usAscRoadNo, MAX_ROADNO_LENGTH);//路口号
    dwSendBuffLen += MAX_ROADNO_LENGTH;
	m_chSendBuff[dwSendBuffLen++] = ACK_QUERY;											//操作类型
	m_chSendBuff[dwSendBuffLen++] = CTL_WORKWAY;										//对象标识
	m_chSendBuff[dwSendBuffLen++] = 0x01;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留

    unsigned char *pBuffer = m_pOpenATCParameter->GetWorkPattern();
    if (pBuffer != NULL)
    {    
        memcpy(m_chSendBuff + dwSendBuffLen, pBuffer, strlen((const char *)pBuffer));
        dwSendBuffLen += strlen((const char *)pBuffer);
    }

    if (pBuffer != NULL)
    {
        free(pBuffer);
        pBuffer = NULL;
    }

	return dwSendBuffLen;
}

int COpenATCCommWithCfgSWThread::AckCtl_AskSetWorkWay(TManualCmd  tManualCmd, int nResult, int nFailCode)
{
    unsigned int dwSendBuffLen = 0;

	memset(m_chSendBuff, 0x00, SEND_BUFFER_SIZE);

	m_chSendBuff[dwSendBuffLen++] = CB_VERSION_FLAG;									//版本号
	m_chSendBuff[dwSendBuffLen++] = CB_ATC_FLAG;										//发送方标识
	m_chSendBuff[dwSendBuffLen++] = CB_CONFIGSOFTWARE_FLAG;								//接收方标识
	m_chSendBuff[dwSendBuffLen++] = LINK_INTERVENECOMMAND;								//数据链路码
	m_chSendBuff[dwSendBuffLen++] = m_tAreaInfo.m_chAscRegionNo;						//区域号
	memcpy(m_chSendBuff + dwSendBuffLen, &m_tAreaInfo.m_usAscRoadNo, MAX_ROADNO_LENGTH);//路口号
    dwSendBuffLen += MAX_ROADNO_LENGTH;
	m_chSendBuff[dwSendBuffLen++] = ACK_SET;											//操作类型
	m_chSendBuff[dwSendBuffLen++] = CTL_WORKWAY;										//对象标识
	m_chSendBuff[dwSendBuffLen++] = 0x01;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留

	TLogicCtlStatus tCtlStatus;
    m_pOpenATCRunStatus->GetLogicCtlStatus(tCtlStatus);

	m_msgBody = cJSON_CreateObject();
	if (tManualCmd.m_bStepForwardCmd)
	{
		cJSON_AddNumberToObject(m_msgBody, "control", CTL_MODE_MANUAL);
		cJSON_AddNumberToObject(m_msgBody, "terminal", 0);//表示方案号
		cJSON_AddNumberToObject(m_msgBody, "value", tManualCmd.m_tStepForwardCmd.m_nNextStageID);//步进时，表示阶段号
		cJSON_AddNumberToObject(m_msgBody, "delay",  tManualCmd.m_tStepForwardCmd.m_nDelayTime);
		cJSON_AddNumberToObject(m_msgBody, "duration", tManualCmd.m_tStepForwardCmd.m_nDurationTime);
	}
	else 
	{
		cJSON_AddNumberToObject(m_msgBody, "control", tManualCmd.m_tPatternInterruptCmd.m_nControlMode);
		cJSON_AddNumberToObject(m_msgBody, "terminal",  tManualCmd.m_tPatternInterruptCmd.m_nPatternNo);//表示方案号
		cJSON_AddNumberToObject(m_msgBody, "value", 0);//步进时，表示阶段号
		cJSON_AddNumberToObject(m_msgBody, "delay", 0);
		cJSON_AddNumberToObject(m_msgBody, "duration", 0);
	}
	
	if (nResult == CONTROL_SUCCEED)
	{
		cJSON_AddNumberToObject(m_msgBody, "success", 0);
	}
	else
	{
		cJSON_AddNumberToObject(m_msgBody, "success", nFailCode);
	}
    m_chOutBuff = cJSON_Print(m_msgBody);
    memcpy(m_chSendBuff + dwSendBuffLen, m_chOutBuff, strlen(m_chOutBuff));
    dwSendBuffLen += strlen(m_chOutBuff);

	return dwSendBuffLen;
}

int COpenATCCommWithCfgSWThread::AckCtl_AskSetCmdWorkWay(BYTE byControlWay, BYTE byResultCode)
{
	unsigned int dwSendBuffLen = 0;

	memset(m_chSendBuff, 0x00, SEND_BUFFER_SIZE);

	m_chSendBuff[dwSendBuffLen++] = CB_VERSION_FLAG;									//版本号
	m_chSendBuff[dwSendBuffLen++] = CB_ATC_FLAG;										//发送方标识
	m_chSendBuff[dwSendBuffLen++] = CB_CONFIGSOFTWARE_FLAG;								//接收方标识
	m_chSendBuff[dwSendBuffLen++] = LINK_INTERVENECOMMAND;								//数据链路码
	m_chSendBuff[dwSendBuffLen++] = m_tAreaInfo.m_chAscRegionNo;						//区域号
	memcpy(m_chSendBuff + dwSendBuffLen, &m_tAreaInfo.m_usAscRoadNo, MAX_ROADNO_LENGTH);//路口号
	dwSendBuffLen += MAX_ROADNO_LENGTH;
	m_chSendBuff[dwSendBuffLen++] = ACK_SET;											//操作类型
	m_chSendBuff[dwSendBuffLen++] = CTL_WORKWAY;										//对象标识
	m_chSendBuff[dwSendBuffLen++] = 0x01;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留

	TLogicCtlStatus tCtlStatus;
	m_pOpenATCRunStatus->GetLogicCtlStatus(tCtlStatus);

	m_msgBody = cJSON_CreateObject();
	
	cJSON_AddNumberToObject(m_msgBody, "control", byControlWay);
	cJSON_AddNumberToObject(m_msgBody, "terminal", 0);
	cJSON_AddNumberToObject(m_msgBody, "value", 0);
	cJSON_AddNumberToObject(m_msgBody, "delay", 0);
	cJSON_AddNumberToObject(m_msgBody, "duration", 0);
	cJSON_AddNumberToObject(m_msgBody, "success", byResultCode);
	
	m_chOutBuff = cJSON_Print(m_msgBody);
	memcpy(m_chSendBuff + dwSendBuffLen, m_chOutBuff, strlen(m_chOutBuff));
	dwSendBuffLen += strlen(m_chOutBuff);

	return dwSendBuffLen;
}

int COpenATCCommWithCfgSWThread::AckCtl_AskQueryFault(int nFaultType)
{
    unsigned int dwSendBuffLen = 0;

	memset(m_chSendBuff, 0x00, SEND_BUFFER_SIZE);

	m_chSendBuff[dwSendBuffLen++] = CB_VERSION_FLAG;									//版本号
	m_chSendBuff[dwSendBuffLen++] = CB_ATC_FLAG;										//发送方标识
	m_chSendBuff[dwSendBuffLen++] = CB_CONFIGSOFTWARE_FLAG;								//接收方标识
	m_chSendBuff[dwSendBuffLen++] = LINK_BASEINFO;										//数据链路码
	m_chSendBuff[dwSendBuffLen++] = m_tAreaInfo.m_chAscRegionNo;						//区域号
	memcpy(m_chSendBuff + dwSendBuffLen, &m_tAreaInfo.m_usAscRoadNo, MAX_ROADNO_LENGTH);//路口号
    dwSendBuffLen += MAX_ROADNO_LENGTH;
	m_chSendBuff[dwSendBuffLen++] = ACK_QUERY;											//操作类型
	m_chSendBuff[dwSendBuffLen++] = CTL_FAULT;											//对象标识
	m_chSendBuff[dwSendBuffLen++] = 0x01;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
  

	if (nFaultType == QUERY_COMMON_FAULT)
	{
		int nDataLen = 0;
		unsigned char *pBuffer = m_pOpenATCParameter->GetATCFaultQueryData(nDataLen);
		if (pBuffer != NULL)
		{    
			memcpy(m_chSendBuff + dwSendBuffLen, pBuffer, strlen((const char *)pBuffer));
			dwSendBuffLen += strlen((const char *)pBuffer);
		}
	
	}
	else if (nFaultType == QUERY_LIGHT_FAULT || nFaultType == QUERY_DETECTOR_FAULT)		//灯组故障或车检器故障
	{
		int i = 0, j = 0;
		if (nFaultType == QUERY_LIGHT_FAULT)
		{
			TLampCltBoardData   tLampCltBoardData;										//灯控板卡数据
			m_pOpenATCRunStatus->GetLampCtlBoardData(tLampCltBoardData);
			for (i = 0;i < C_N_MAXLAMPBOARD_NUM;i++)
			{
				for (j = 0;j < C_N_LAMPBORAD_OUTPUTNUM;j++)
				{
					tLampCltBoardData.m_atLampFault[i].m_bReadStatus[j] = true;
				}
			}
			m_pOpenATCRunStatus->SetLampCtlBoardData(tLampCltBoardData);
		}
		else if (nFaultType == QUERY_DETECTOR_FAULT)
		{
			TVehDetBoardData       tVehDetBoardData;									//车检板卡数据
			m_pOpenATCRunStatus->GetVehDetBoardData(tVehDetBoardData);
			for (i = 0;i < C_N_MAXDETBOARD_NUM;i++)
			{
				for (j = 0;j < C_N_MAXDETINPUT_NUM;j++)
				{
					tVehDetBoardData.m_atVehDetData[i].m_bReadStatus[j] = true;
				}
			}

			m_pOpenATCRunStatus->SetVehDetBoardData(tVehDetBoardData);
		}

		const char *pBuffer = COpenATCFaultProcManager::getInstance()->GenDetailedFaultInfo(nFaultType);
		if (pBuffer != NULL)
		{    
			memcpy(m_chSendBuff + dwSendBuffLen, pBuffer, strlen((const char *)pBuffer));
			dwSendBuffLen += strlen((const char *)pBuffer);
		}
		else
		{
			if (m_msgBody != NULL)
			{
				cJSON_Delete(m_msgBody);
				m_msgBody = NULL;
			}

			if (m_chOutBuff != NULL)
			{
				free(m_chOutBuff);
				m_chOutBuff = NULL;
			}

			m_msgBody = cJSON_CreateObject();
			cJSON* m_msgDetailBody = cJSON_CreateArray();
			cJSON_AddItemToObject(m_msgBody, "m_FaultDetailedInfo", m_msgDetailBody);
			m_chOutBuff = cJSON_Print(m_msgBody);
			memcpy(m_chSendBuff + dwSendBuffLen, m_chOutBuff, strlen(m_chOutBuff));
			dwSendBuffLen += strlen(m_chOutBuff);
		}
	}

	return dwSendBuffLen;
}


int COpenATCCommWithCfgSWThread::AckCtl_AskQueryVersion()
{
    unsigned int dwSendBuffLen = 0;

	memset(m_chSendBuff, 0x00, SEND_BUFFER_SIZE);

	m_chSendBuff[dwSendBuffLen++] = CB_VERSION_FLAG;									//版本号
	m_chSendBuff[dwSendBuffLen++] = CB_ATC_FLAG;										//发送方标识
	m_chSendBuff[dwSendBuffLen++] = CB_CONFIGSOFTWARE_FLAG;								//接收方标识
	m_chSendBuff[dwSendBuffLen++] = LINK_BASEINFO;										//数据链路码
	m_chSendBuff[dwSendBuffLen++] = m_tAreaInfo.m_chAscRegionNo;						//区域号
	memcpy(m_chSendBuff + dwSendBuffLen, &m_tAreaInfo.m_usAscRoadNo, MAX_ROADNO_LENGTH);//路口号
    dwSendBuffLen += MAX_ROADNO_LENGTH;
	m_chSendBuff[dwSendBuffLen++] = ACK_QUERY;											//操作类型
	m_chSendBuff[dwSendBuffLen++] = CTL_VERSION;										//对象标识
	m_chSendBuff[dwSendBuffLen++] = 0x01;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留

    unsigned char *pBuffer = m_pOpenATCParameter->GetATCVersion(m_chOpenATCVersion);
    if (pBuffer != NULL)
    {    
        memcpy(m_chSendBuff + dwSendBuffLen, pBuffer, strlen((const char *)pBuffer));
        dwSendBuffLen += strlen((const char *)pBuffer);
    }

    if (pBuffer != NULL)
    {
        free(pBuffer);
        pBuffer = NULL;
    }

	return dwSendBuffLen;
}

int COpenATCCommWithCfgSWThread::AckCtl_AskQueryATCParamVersion()
{
    unsigned int dwSendBuffLen = 0;

    memset(m_chSendBuff, 0x00, SEND_BUFFER_SIZE);

	m_chSendBuff[dwSendBuffLen++] = CB_VERSION_FLAG;									//版本号
	m_chSendBuff[dwSendBuffLen++] = CB_ATC_FLAG;										//发送方标识
	m_chSendBuff[dwSendBuffLen++] = CB_CONFIGSOFTWARE_FLAG;								//接收方标识
	m_chSendBuff[dwSendBuffLen++] = LINK_INTERVENECOMMAND;								//数据链路码
	m_chSendBuff[dwSendBuffLen++] = m_tAreaInfo.m_chAscRegionNo;						//区域号
	memcpy(m_chSendBuff + dwSendBuffLen, &m_tAreaInfo.m_usAscRoadNo, MAX_ROADNO_LENGTH);//路口号
    dwSendBuffLen += MAX_ROADNO_LENGTH;
	m_chSendBuff[dwSendBuffLen++] = ACK_QUERY;											//操作类型
	m_chSendBuff[dwSendBuffLen++] = CTL_FEATUREPARAVERSION;								//对象标识
	m_chSendBuff[dwSendBuffLen++] = 0x01;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留

    unsigned char *pBuffer = m_pOpenATCParameter->GetATCParamVersion();
    if (pBuffer != NULL)
    {    
        memcpy(m_chSendBuff + dwSendBuffLen, pBuffer, strlen((const char *)pBuffer));
        dwSendBuffLen += strlen((const char *)pBuffer);
    }

    if (pBuffer != NULL)
    {
        free(pBuffer);
        pBuffer = NULL;
    }

	return dwSendBuffLen;
}

int COpenATCCommWithCfgSWThread::AckCtl_AskSetATCParamVersion()
{
    unsigned int dwSendBuffLen = 0;

	memset(m_chSendBuff, 0x00, SEND_BUFFER_SIZE);

	m_chSendBuff[dwSendBuffLen++] = CB_VERSION_FLAG;									//版本号
	m_chSendBuff[dwSendBuffLen++] = CB_ATC_FLAG;										//发送方标识
	m_chSendBuff[dwSendBuffLen++] = CB_CONFIGSOFTWARE_FLAG;								//接收方标识
	m_chSendBuff[dwSendBuffLen++] = LINK_INTERVENECOMMAND;								//数据链路码
	m_chSendBuff[dwSendBuffLen++] = m_tAreaInfo.m_chAscRegionNo;						//区域号
	memcpy(m_chSendBuff + dwSendBuffLen, &m_tAreaInfo.m_usAscRoadNo, MAX_ROADNO_LENGTH);//路口号
    dwSendBuffLen += MAX_ROADNO_LENGTH;
	m_chSendBuff[dwSendBuffLen++] = ACK_SET;											//操作类型
	m_chSendBuff[dwSendBuffLen++] = CTL_FEATUREPARAVERSION;								//对象标识
	m_chSendBuff[dwSendBuffLen++] = 0x01;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留

	return dwSendBuffLen;
}

int COpenATCCommWithCfgSWThread::AckCtl_AskQueryATCCode()
{
    unsigned int dwSendBuffLen = 0;

	memset(m_chSendBuff, 0x00, SEND_BUFFER_SIZE);

	m_chSendBuff[dwSendBuffLen++] = CB_VERSION_FLAG;									//版本号
	m_chSendBuff[dwSendBuffLen++] = CB_ATC_FLAG;										//发送方标识
	m_chSendBuff[dwSendBuffLen++] = CB_CONFIGSOFTWARE_FLAG;								//接收方标识
	m_chSendBuff[dwSendBuffLen++] = LINK_INTERVENECOMMAND;								//数据链路码
	m_chSendBuff[dwSendBuffLen++] = m_tAreaInfo.m_chAscRegionNo;						//区域号
	memcpy(m_chSendBuff + dwSendBuffLen, &m_tAreaInfo.m_usAscRoadNo, MAX_ROADNO_LENGTH);//路口号
    dwSendBuffLen += MAX_ROADNO_LENGTH;
	m_chSendBuff[dwSendBuffLen++] = ACK_QUERY;											//操作类型
	m_chSendBuff[dwSendBuffLen++] = CTL_INDENTIFYCODE;									//对象标识
	m_chSendBuff[dwSendBuffLen++] = 0x01;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留

    unsigned char *pBuffer = m_pOpenATCParameter->GetATCCode();
    if (pBuffer != NULL)
    {    
        memcpy(m_chSendBuff + dwSendBuffLen, pBuffer, strlen((const char *)pBuffer));
        dwSendBuffLen += strlen((const char *)pBuffer);
    }

    if (pBuffer != NULL)
    {
        free(pBuffer);
        pBuffer = NULL;
    }

	return dwSendBuffLen;
}

int COpenATCCommWithCfgSWThread::AckCtl_AskRemoteControl()
{
    unsigned int dwSendBuffLen = 0;

	memset(m_chSendBuff, 0x00, SEND_BUFFER_SIZE);

	m_chSendBuff[dwSendBuffLen++] = CB_VERSION_FLAG;									//版本号
	m_chSendBuff[dwSendBuffLen++] = CB_ATC_FLAG;										//发送方标识
	m_chSendBuff[dwSendBuffLen++] = CB_CONFIGSOFTWARE_FLAG;								//接收方标识
	m_chSendBuff[dwSendBuffLen++] = LINK_INTERVENECOMMAND;								//数据链路码
	m_chSendBuff[dwSendBuffLen++] = m_tAreaInfo.m_chAscRegionNo;						//区域号
	memcpy(m_chSendBuff + dwSendBuffLen, &m_tAreaInfo.m_usAscRoadNo, MAX_ROADNO_LENGTH);//路口号
    dwSendBuffLen += MAX_ROADNO_LENGTH;
	m_chSendBuff[dwSendBuffLen++] = ACK_SET;											//操作类型
	m_chSendBuff[dwSendBuffLen++] = CTL_REMOTECONTROL;									//对象标识
	m_chSendBuff[dwSendBuffLen++] = 0x01;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留

	return dwSendBuffLen;
}

int COpenATCCommWithCfgSWThread::AckCtl_AskSystemRemote()
{
	unsigned int dwSendBuffLen = 0;

	memset(m_chSendBuff, 0x00, SEND_BUFFER_SIZE);

	m_chSendBuff[dwSendBuffLen++] = CB_VERSION_FLAG;									//版本号
	m_chSendBuff[dwSendBuffLen++] = CB_ATC_FLAG;										//发送方标识
	m_chSendBuff[dwSendBuffLen++] = CB_CONFIGSOFTWARE_FLAG;								//接收方标识
	m_chSendBuff[dwSendBuffLen++] = LINK_CONTROL;										//数据链路码
	m_chSendBuff[dwSendBuffLen++] = m_tAreaInfo.m_chAscRegionNo;						//区域号
	memcpy(m_chSendBuff + dwSendBuffLen, &m_tAreaInfo.m_usAscRoadNo, MAX_ROADNO_LENGTH);//路口号
	dwSendBuffLen += MAX_ROADNO_LENGTH;
	m_chSendBuff[dwSendBuffLen++] = ACK_QUERY;											//操作类型
	m_chSendBuff[dwSendBuffLen++] = CTL_SYSTEM_REMOTE;									//系统远程调试
	m_chSendBuff[dwSendBuffLen++] = 0x01;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留

	if (m_msgBody != NULL)
	{
		cJSON_Delete(m_msgBody);
		m_msgBody = NULL;
	}

	if (m_chOutBuff != NULL)
	{
		free(m_chOutBuff);
		m_chOutBuff = NULL;
	}


	m_msgBody = cJSON_CreateObject();
	cJSON_AddStringToObject(m_msgBody, "return", "success");
	m_chOutBuff = cJSON_Print(m_msgBody);
	memcpy(m_chSendBuff + dwSendBuffLen, m_chOutBuff, strlen(m_chOutBuff));
	dwSendBuffLen += strlen(m_chOutBuff);

	return dwSendBuffLen;
}

int COpenATCCommWithCfgSWThread::AckCtl_AskSetSystemRemote()
{
	unsigned int dwSendBuffLen = 0;

	memset(m_chSendBuff, 0x00, SEND_BUFFER_SIZE);

	m_chSendBuff[dwSendBuffLen++] = CB_VERSION_FLAG;									//版本号
	m_chSendBuff[dwSendBuffLen++] = CB_ATC_FLAG;										//发送方标识
	m_chSendBuff[dwSendBuffLen++] = CB_CONFIGSOFTWARE_FLAG;								//接收方标识
	m_chSendBuff[dwSendBuffLen++] = LINK_CONTROL;										//数据链路码
	m_chSendBuff[dwSendBuffLen++] = m_tAreaInfo.m_chAscRegionNo;						//区域号
	memcpy(m_chSendBuff + dwSendBuffLen, &m_tAreaInfo.m_usAscRoadNo, MAX_ROADNO_LENGTH);//路口号
	dwSendBuffLen += MAX_ROADNO_LENGTH;
	m_chSendBuff[dwSendBuffLen++] = ACK_SET;											//操作类型
	m_chSendBuff[dwSendBuffLen++] = CTL_SYSTEM_REMOTE;									//对象标识
	m_chSendBuff[dwSendBuffLen++] = 0x01;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留

	if (m_msgBody != NULL)
	{
		cJSON_Delete(m_msgBody);
		m_msgBody = NULL;
	}

	if (m_chOutBuff != NULL)
	{
		free(m_chOutBuff);
		m_chOutBuff = NULL;
	}

	m_msgBody = cJSON_CreateObject();
	cJSON_AddStringToObject(m_msgBody, "return", "success");
	m_chOutBuff = cJSON_Print(m_msgBody);
	memcpy(m_chSendBuff + dwSendBuffLen, m_chOutBuff, strlen(m_chOutBuff));
	dwSendBuffLen += strlen(m_chOutBuff);

	return dwSendBuffLen;
}

int COpenATCCommWithCfgSWThread::AckCtl_AskQueryVecDetectorParamData()
{
    int i = 0;
    int nCount = 0;
    unsigned int dwSendBuffLen = 0;

	memset(m_chSendBuff, 0x00, SEND_BUFFER_SIZE);

	m_chSendBuff[dwSendBuffLen++] = CB_VERSION_FLAG;									//版本号
	m_chSendBuff[dwSendBuffLen++] = CB_ATC_FLAG;										//发送方标识
	m_chSendBuff[dwSendBuffLen++] = CB_CONFIGSOFTWARE_FLAG;								//接收方标识
	m_chSendBuff[dwSendBuffLen++] = LINK_INTERVENECOMMAND;								//数据链路码
	m_chSendBuff[dwSendBuffLen++] = m_tAreaInfo.m_chAscRegionNo;						//区域号
	memcpy(m_chSendBuff + dwSendBuffLen, &m_tAreaInfo.m_usAscRoadNo, MAX_ROADNO_LENGTH);//路口号
    dwSendBuffLen += MAX_ROADNO_LENGTH;
	m_chSendBuff[dwSendBuffLen++] = ACK_QUERY;											//操作类型
	m_chSendBuff[dwSendBuffLen++] = CTL_DETECTOR;										//对象标识
	m_chSendBuff[dwSendBuffLen++] = 0x01;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留

    unsigned char *pBuffer = m_pOpenATCParameter->GetVecDetectorParamData();
    if (pBuffer != NULL)
    {    
        memcpy(m_chSendBuff + dwSendBuffLen, pBuffer, strlen((const char *)pBuffer));
        dwSendBuffLen += strlen((const char *)pBuffer);
    }

    if (pBuffer != NULL)
    {
        free(pBuffer);
        pBuffer = NULL;
    }

	return dwSendBuffLen;
}

int COpenATCCommWithCfgSWThread::AckCtl_AskSetVecDetectorParamData()
{
    unsigned int dwSendBuffLen = 0;

	memset(m_chSendBuff, 0x00, SEND_BUFFER_SIZE);

	m_chSendBuff[dwSendBuffLen++] = CB_VERSION_FLAG;									//版本号
	m_chSendBuff[dwSendBuffLen++] = CB_ATC_FLAG;										//发送方标识
	m_chSendBuff[dwSendBuffLen++] = CB_CONFIGSOFTWARE_FLAG;								//接收方标识
	m_chSendBuff[dwSendBuffLen++] = LINK_INTERVENECOMMAND;								//数据链路码
	m_chSendBuff[dwSendBuffLen++] = m_tAreaInfo.m_chAscRegionNo;						//区域号
	memcpy(m_chSendBuff + dwSendBuffLen, &m_tAreaInfo.m_usAscRoadNo, MAX_ROADNO_LENGTH);//路口号
    dwSendBuffLen += MAX_ROADNO_LENGTH;
	m_chSendBuff[dwSendBuffLen++] = ACK_SET;											//操作类型
	m_chSendBuff[dwSendBuffLen++] = CTL_DETECTOR;										//对象标识
	m_chSendBuff[dwSendBuffLen++] = 0x01;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留

	return dwSendBuffLen;
}

int COpenATCCommWithCfgSWThread::AckCtl_AskHeart()
{
    int i = 0, j = 0;
    unsigned int dwSendBuffLen = 0;

	memset(m_chSendBuff, 0x00, SEND_BUFFER_SIZE);

	m_chSendBuff[dwSendBuffLen++] = CB_VERSION_FLAG;									//版本号
	m_chSendBuff[dwSendBuffLen++] = CB_ATC_FLAG;										//发送方标识
	m_chSendBuff[dwSendBuffLen++] = CB_CONFIGSOFTWARE_FLAG;								//接收方标识
	m_chSendBuff[dwSendBuffLen++] = LINK_CONFIG;										//数据链路码
	m_chSendBuff[dwSendBuffLen++] = m_tAreaInfo.m_chAscRegionNo;						//区域号
	memcpy(m_chSendBuff + dwSendBuffLen, &m_tAreaInfo.m_usAscRoadNo, MAX_ROADNO_LENGTH);//路口号
    dwSendBuffLen += MAX_ROADNO_LENGTH;
	m_chSendBuff[dwSendBuffLen++] = ACK_QUERY;											//操作类型
	m_chSendBuff[dwSendBuffLen++] = CTL_HEART;											//对象标识
	m_chSendBuff[dwSendBuffLen++] = 0x01;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留

    TPhaseRunStatus tRunStatus;
    memset(&tRunStatus,0,sizeof(TPhaseRunStatus));
    m_pOpenATCRunStatus->GetPhaseRunStatus(tRunStatus);
    
    for (i = 0;i < tRunStatus.m_nRingCount;i++)
    {
        for (j = 0;j < MAX_SEQUENCE_TABLE_COUNT;j++)
        {
            if (tRunStatus.m_atRingRunStatus[i].m_atPhaseStatus[j].m_byPhaseID != 0)
            {
				//相位编号
                memcpy(m_chSendBuff + dwSendBuffLen, &tRunStatus.m_atRingRunStatus[i].m_atPhaseStatus[j].m_byPhaseID, 1);
                dwSendBuffLen += 1;
				//工作状态
                memcpy(m_chSendBuff + dwSendBuffLen, &tRunStatus.m_atRingRunStatus[i].m_atPhaseStatus[j].m_chPhaseStatus, 1);
                dwSendBuffLen += 1;
            }
        }
    }
   
    m_chSendBuff[dwSendBuffLen++] = tRunStatus.m_byPlanID;
	m_chSendBuff[dwSendBuffLen++] = tRunStatus.m_nCurCtlMode;

	return dwSendBuffLen;
}

int COpenATCCommWithCfgSWThread::IsLibFile(const char *szFileName)
{
    if (memcmp(szFileName, "lib", 3) == 0)
    {
        return OPENATC_RTN_OK;
    }
	
    return OPENATC_RTN_FAILED;
} 

void COpenATCCommWithCfgSWThread::ChangeLibFileName(char* szFileName)
{
	char szAbsName[FRM_MAX_FILENAME_LENGTH + 1] = {0};
    memset(szAbsName, 0x00, sizeof(szAbsName));
	#ifndef _WIN32
    snprintf(szAbsName, FRM_MAX_FILENAME_LENGTH, "./lib/%s", szFileName);
	#else
    snprintf(szAbsName, FRM_MAX_FILENAME_LENGTH, ".\\lib\\%s", szFileName);
	#endif
}

int COpenATCCommWithCfgSWThread::IsCfgFile(const char  *szFileName)
{
    if (strstr(szFileName, ".ini") != NULL || strstr(szFileName, ".json") != NULL)
    {
        return OPENATC_RTN_OK;
    }
	
    return OPENATC_RTN_FAILED;
} 

void COpenATCCommWithCfgSWThread::ChangeCfgFileName(char *szFileName)
{
	char szAbsName[FRM_MAX_FILENAME_LENGTH + 1] = {0};
    memset(szAbsName, 0x00, sizeof(szAbsName));
    #ifndef _WIN32
    snprintf(szAbsName, FRM_MAX_FILENAME_LENGTH, "../config/%s", szFileName);
    #else
    snprintf(szAbsName, FRM_MAX_FILENAME_LENGTH, ".\\config\\%s", szFileName);
    #endif
    strncpy(szFileName, szAbsName, FRM_MAX_FILENAME_LENGTH);
}

int COpenATCCommWithCfgSWThread::AckCtl_AskTrafficFlow()
{
    unsigned int dwSendBuffLen = 0;

	memset(m_chSendBuff, 0x00, SEND_BUFFER_SIZE);

	m_chSendBuff[dwSendBuffLen++] = CB_VERSION_FLAG;									//版本号
	m_chSendBuff[dwSendBuffLen++] = CB_ATC_FLAG;										//发送方标识
	m_chSendBuff[dwSendBuffLen++] = CB_CONFIGSOFTWARE_FLAG;								//接收方标识
	m_chSendBuff[dwSendBuffLen++] = LINK_CONTROL;										//数据链路码
	m_chSendBuff[dwSendBuffLen++] = m_tAreaInfo.m_chAscRegionNo;						//区域号
	memcpy(m_chSendBuff + dwSendBuffLen, &m_tAreaInfo.m_usAscRoadNo, MAX_ROADNO_LENGTH);//路口号
    dwSendBuffLen += MAX_ROADNO_LENGTH;
	m_chSendBuff[dwSendBuffLen++] = ACK_QUERY;											//操作类型
	m_chSendBuff[dwSendBuffLen++] = CTL_TRAFFICFLOWINFO;								//对象标识
	m_chSendBuff[dwSendBuffLen++] = 0x01;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留

    unsigned char *pBuffer = m_pOpenATCParameter->GetCurrentTrafficFlowData(&COpenATCFlowProcManager::getInstance()->GetCurrentStatisticVehDetData());
    if (pBuffer != NULL)
    {    
        memcpy(m_chSendBuff + dwSendBuffLen, pBuffer, strlen((const char *)pBuffer));
        dwSendBuffLen += strlen((const char *)pBuffer);
    }

	if (pBuffer != NULL)
	{
		delete pBuffer;
		pBuffer = NULL;
	}

	return dwSendBuffLen;
}

int COpenATCCommWithCfgSWThread::AckCtl_AskUdiskUpdate()
{
    unsigned int dwSendBuffLen = 0;

    memset(m_chSendBuff, 0x00, SEND_BUFFER_SIZE);
	
	m_chSendBuff[dwSendBuffLen++] = CB_VERSION_FLAG;									//版本号
	m_chSendBuff[dwSendBuffLen++] = CB_ATC_FLAG;										//发送方标识
	m_chSendBuff[dwSendBuffLen++] = CB_CONFIGSOFTWARE_FLAG;								//接收方标识
	m_chSendBuff[dwSendBuffLen++] = LINK_CONTROL;										//数据链路码
	m_chSendBuff[dwSendBuffLen++] = m_tAreaInfo.m_chAscRegionNo;						//区域号
    memcpy(m_chSendBuff + dwSendBuffLen, &m_tAreaInfo.m_usAscRoadNo, MAX_ROADNO_LENGTH);//路口号
    dwSendBuffLen += MAX_ROADNO_LENGTH;
	m_chSendBuff[dwSendBuffLen++] = ACK_SET;											//操作类型
	m_chSendBuff[dwSendBuffLen++] = CTL_UDISK;											//对象标识
	m_chSendBuff[dwSendBuffLen++] = 0x01;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留

    if (m_msgBody != NULL)
    {
        cJSON_Delete(m_msgBody);
        m_msgBody = NULL;
    }

    if (m_chOutBuff != NULL)
    {
        free(m_chOutBuff);
        m_chOutBuff = NULL;
    }

    m_msgBody = cJSON_CreateObject();
    cJSON_AddStringToObject(m_msgBody, "return", "success");
    m_chOutBuff = cJSON_Print(m_msgBody);
    memcpy(m_chSendBuff + dwSendBuffLen, m_chOutBuff, strlen(m_chOutBuff));
    dwSendBuffLen += strlen(m_chOutBuff);

    return dwSendBuffLen;
}

void COpenATCCommWithCfgSWThread::OpenATCSleep(long nMsec)
{
#ifdef _WIN32
    Sleep(nMsec);
#else
    usleep(nMsec*1000);
#endif
}

bool COpenATCCommWithCfgSWThread::SetSysTime(const long nTime)
{
	m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "COpenATCCommWithCfgSWThread SetSysTime");
    timeval sysTime;
    sysTime.tv_sec  = nTime;
    sysTime.tv_usec = 0;
#ifndef _WIN32
	if (settimeofday(&sysTime,NULL) != 0)
    {
        return false;
    }
	//system("hwclock -w");
    return true;
#else
    SYSTEMTIME winTime;
    tm  tmTime;
    tmTime	= *::localtime((time_t*)&sysTime.tv_sec);
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

void COpenATCCommWithCfgSWThread::GetGropStatusByRYG(int nBoardIndex,int nChannelIndex,char chR,char chY,char chG,TCanData & tCanData)
{
    tCanData.CanData.io1 = 0;
    tCanData.CanData.io2 = 0;
    tCanData.CanData.io3 = 0;
    tCanData.CanData.io4 = 0;
    tCanData.CanData.io5 = 0;
    tCanData.CanData.io6 = 0;
    tCanData.CanData.io7 = 0;
    tCanData.CanData.io8 = 0;
    if (chR == LAMP_CLR_FLASH || chY == LAMP_CLR_FLASH || chG == LAMP_CLR_FLASH) 
    {
        if (nChannelIndex == 0)
        {
            tCanData.CanData.io1 = 0;
            tCanData.CanData.io2 = 0;
        }
        else if (nChannelIndex == 3)
        {
            tCanData.CanData.io3 = 0;
            tCanData.CanData.io4 = 0;
        }
        else if (nChannelIndex == 6)
        {
            tCanData.CanData.io5 = 0;
            tCanData.CanData.io6 = 0;
        }
        else if (nChannelIndex == 9)
        {
            tCanData.CanData.io7 = 0;
            tCanData.CanData.io8 = 0;
        }
    }
    else if (chR == LAMP_CLR_ON && chY == LAMP_CLR_OFF && chG == LAMP_CLR_OFF) 
    {
        if (nChannelIndex == 0)
        {
            tCanData.CanData.io1 = 1;
            tCanData.CanData.io2 = 0;
        }
        else if (nChannelIndex == 3)
        {
            tCanData.CanData.io3 = 1;
            tCanData.CanData.io4 = 0;
        }
        else if (nChannelIndex == 6)
        {
            tCanData.CanData.io5 = 1;
            tCanData.CanData.io6 = 0;
        }
        else if (nChannelIndex == 9)
        {
            tCanData.CanData.io7 = 1;
            tCanData.CanData.io8 = 0;
        }
    }
    else if (chR == LAMP_CLR_OFF && chY == LAMP_CLR_ON && chG == LAMP_CLR_OFF) 
    {
        if (nChannelIndex == 0)
        {
            tCanData.CanData.io1 = 0;
            tCanData.CanData.io2 = 1;
        }
        else if (nChannelIndex == 3)
        {
            tCanData.CanData.io3 = 0;
            tCanData.CanData.io4 = 1;
        }
        else if (nChannelIndex == 6)
        {
            tCanData.CanData.io5 = 0;
            tCanData.CanData.io6 = 1;
        }
        else if (nChannelIndex == 9)
        {
            tCanData.CanData.io7 = 0;
            tCanData.CanData.io8 = 1;
        }
    }
    else if (chR == LAMP_CLR_OFF && chY == LAMP_CLR_OFF && chG == LAMP_CLR_ON) 
    {
        if (nChannelIndex == 0)
        {
            tCanData.CanData.io1 = 1;
            tCanData.CanData.io2 = 1;
        }
        else if (nChannelIndex == 3)
        {
            tCanData.CanData.io3 = 1;
            tCanData.CanData.io4 = 1;
        }
        else if (nChannelIndex == 6)
        {
            tCanData.CanData.io5 = 1;
            tCanData.CanData.io6 = 1;
        }
        else if (nChannelIndex == 9)
        {
            tCanData.CanData.io7 = 1;
            tCanData.CanData.io8 = 1;
        }
    }
}

int COpenATCCommWithCfgSWThread::AckCtl_AskOperationRecord()
{
    unsigned int dwSendBuffLen = 0;

    memset(m_chSendBuff, 0x00, SEND_BUFFER_SIZE);
	
	m_chSendBuff[dwSendBuffLen++] = CB_VERSION_FLAG;									//版本号
	m_chSendBuff[dwSendBuffLen++] = CB_ATC_FLAG;										//发送方标识
	m_chSendBuff[dwSendBuffLen++] = CB_CONFIGSOFTWARE_FLAG;								//接收方标识
	m_chSendBuff[dwSendBuffLen++] = LINK_CONTROL;										//数据链路码
	m_chSendBuff[dwSendBuffLen++] = m_tAreaInfo.m_chAscRegionNo;						//区域号
    memcpy(m_chSendBuff + dwSendBuffLen, &m_tAreaInfo.m_usAscRoadNo, MAX_ROADNO_LENGTH);//路口号
    dwSendBuffLen += MAX_ROADNO_LENGTH;
	m_chSendBuff[dwSendBuffLen++] = ACK_QUERY;											//操作类型
	m_chSendBuff[dwSendBuffLen++] = CTL_OPERATION_RECORD;								//对象标识
	m_chSendBuff[dwSendBuffLen++] = 0x01;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留

	if (access(OPERATIONRECORD_FILE_PATH, 0) == 0)										//检查文件目录
	{
		FILE *pf = fopen(OPERATIONRECORD_FILE_PATH, "rb");
		if (pf)
		{
			int nSize = fread(m_chSendBuff + dwSendBuffLen, 1, C_N_MAXOPERATIONRECORDBUFFER_SIZE, pf);
            dwSendBuffLen += nSize;
			fclose(pf);
		}
	}

    return dwSendBuffLen;
}

int COpenATCCommWithCfgSWThread::AckCtl_AskChannelCheck(int nResult, int nFailCode)
{
	unsigned int dwSendBuffLen = 0;

	memset(m_chSendBuff, 0x00, SEND_BUFFER_SIZE);

	m_chSendBuff[dwSendBuffLen++] = CB_VERSION_FLAG;									//版本号
	m_chSendBuff[dwSendBuffLen++] = CB_ATC_FLAG;										//发送方标识
	m_chSendBuff[dwSendBuffLen++] = CB_CONFIGSOFTWARE_FLAG;								//接收方标识
	m_chSendBuff[dwSendBuffLen++] = LINK_CONTROL;										//数据链路码
	m_chSendBuff[dwSendBuffLen++] = m_tAreaInfo.m_chAscRegionNo;						//区域号
	memcpy(m_chSendBuff + dwSendBuffLen, &m_tAreaInfo.m_usAscRoadNo, MAX_ROADNO_LENGTH);//路口号
	dwSendBuffLen += MAX_ROADNO_LENGTH;
	m_chSendBuff[dwSendBuffLen++] = ACK_SET;											//操作类型
	m_chSendBuff[dwSendBuffLen++] = CTL_CHANNEL_CHECK;									//对象标识
	m_chSendBuff[dwSendBuffLen++] = 0x01;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留

	if (m_msgBody != NULL)
	{
		cJSON_Delete(m_msgBody);
		m_msgBody = NULL;
	}

	if (m_chOutBuff != NULL)
	{
		free(m_chOutBuff);
		m_chOutBuff = NULL;
	}

	m_msgBody = cJSON_CreateObject();
	if (nResult == CONTROL_SUCCEED)
	{
		cJSON_AddNumberToObject(m_msgBody, "success", 0);
	}
	else
	{
		cJSON_AddNumberToObject(m_msgBody, "success", nFailCode);
	}
	m_chOutBuff = cJSON_Print(m_msgBody);
	memcpy(m_chSendBuff + dwSendBuffLen, m_chOutBuff, strlen(m_chOutBuff));

	dwSendBuffLen += strlen(m_chOutBuff);

	return dwSendBuffLen;
}

int COpenATCCommWithCfgSWThread::AckCtl_AskPatternInterrupt(int nResult, int nFailCode)
{
	unsigned int dwSendBuffLen = 0;

	memset(m_chSendBuff, 0x00, SEND_BUFFER_SIZE);

	m_chSendBuff[dwSendBuffLen++] = CB_VERSION_FLAG;									//版本号
	m_chSendBuff[dwSendBuffLen++] = CB_ATC_FLAG;										//发送方标识
	m_chSendBuff[dwSendBuffLen++] = CB_CONFIGSOFTWARE_FLAG;								//接收方标识
	m_chSendBuff[dwSendBuffLen++] = LINK_CONTROL;										//数据链路码
	m_chSendBuff[dwSendBuffLen++] = m_tAreaInfo.m_chAscRegionNo;						//区域号
	memcpy(m_chSendBuff + dwSendBuffLen, &m_tAreaInfo.m_usAscRoadNo, MAX_ROADNO_LENGTH);//路口号
	dwSendBuffLen += MAX_ROADNO_LENGTH;
	m_chSendBuff[dwSendBuffLen++] = ACK_SET;											//操作类型
	m_chSendBuff[dwSendBuffLen++] = CTL_PATTERN_INTERRUPT;								//对象标识
	m_chSendBuff[dwSendBuffLen++] = 0x01;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留

	if (m_msgBody != NULL)
	{
		cJSON_Delete(m_msgBody);
		m_msgBody = NULL;
	}

	if (m_chOutBuff != NULL)
	{
		free(m_chOutBuff);
		m_chOutBuff = NULL;
	}

	m_msgBody = cJSON_CreateObject();
	if (nResult == CONTROL_SUCCEED)
	{
		cJSON_AddNumberToObject(m_msgBody, "success", 0);
	}
	else
	{
		cJSON_AddNumberToObject(m_msgBody, "success", nFailCode);
	}
	m_chOutBuff = cJSON_Print(m_msgBody);
	memcpy(m_chSendBuff + dwSendBuffLen, m_chOutBuff, strlen(m_chOutBuff));

	dwSendBuffLen += strlen(m_chOutBuff);

	return dwSendBuffLen;
}

int COpenATCCommWithCfgSWThread::AckCtl_AskVolumeLog(int nUDiskStatus)
{
	unsigned int dwSendBuffLen = 0;

	memset(m_chSendBuff, 0x00, SEND_BUFFER_SIZE);

	m_chSendBuff[dwSendBuffLen++] = CB_VERSION_FLAG;									//版本号
	m_chSendBuff[dwSendBuffLen++] = CB_ATC_FLAG;										//发送方标识
	m_chSendBuff[dwSendBuffLen++] = CB_CONFIGSOFTWARE_FLAG;								//接收方标识
	m_chSendBuff[dwSendBuffLen++] = LINK_CONTROL;										//数据链路码
	m_chSendBuff[dwSendBuffLen++] = m_tAreaInfo.m_chAscRegionNo;						//区域号
	memcpy(m_chSendBuff + dwSendBuffLen, &m_tAreaInfo.m_usAscRoadNo, MAX_ROADNO_LENGTH);//路口号
	dwSendBuffLen += MAX_ROADNO_LENGTH;
	m_chSendBuff[dwSendBuffLen++] = ACK_SET;											//操作类型
	m_chSendBuff[dwSendBuffLen++] = CTL_VOLUME_LOG;										//对象标识
	m_chSendBuff[dwSendBuffLen++] = 0x01;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留

	if (m_msgBody != NULL)
	{
		cJSON_Delete(m_msgBody);
		m_msgBody = NULL;
	}

	if (m_chOutBuff != NULL)
	{
		free(m_chOutBuff);
		m_chOutBuff = NULL;
	}

	m_msgBody = cJSON_CreateObject();
	cJSON_AddNumberToObject(m_msgBody, "udiskstatus", nUDiskStatus);
	m_chOutBuff = cJSON_Print(m_msgBody);
	memcpy(m_chSendBuff + dwSendBuffLen, m_chOutBuff, strlen(m_chOutBuff));

	dwSendBuffLen += strlen(m_chOutBuff);

	return dwSendBuffLen;
}

int COpenATCCommWithCfgSWThread::AckCtl_AskChannelStatus()
{
    unsigned int dwSendBuffLen = 0;

	memset(m_chSendBuff, 0x00, SEND_BUFFER_SIZE);

	m_chSendBuff[dwSendBuffLen++] = CB_VERSION_FLAG;									//版本号
	m_chSendBuff[dwSendBuffLen++] = CB_ATC_FLAG;										//发送方标识
	m_chSendBuff[dwSendBuffLen++] = CB_CONFIGSOFTWARE_FLAG;								//接收方标识
	m_chSendBuff[dwSendBuffLen++] = LINK_CONTROL;										//数据链路码
	m_chSendBuff[dwSendBuffLen++] = m_tAreaInfo.m_chAscRegionNo;						//区域号
	memcpy(m_chSendBuff + dwSendBuffLen, &m_tAreaInfo.m_usAscRoadNo, MAX_ROADNO_LENGTH);//路口号
	dwSendBuffLen += MAX_ROADNO_LENGTH;
	m_chSendBuff[dwSendBuffLen++] = ACK_QUERY;											//操作类型
	m_chSendBuff[dwSendBuffLen++] = CTL_CHANNEL_STATUS_INFO;							//对象标识
	m_chSendBuff[dwSendBuffLen++] = 0x01;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留

    TLampCltBoardData   tLampCltBoardData;												//灯控板卡数据
	m_pOpenATCRunStatus->GetLampCtlBoardData(tLampCltBoardData);
	tLampCltBoardData.m_bReadParaStatus = true;
	m_pOpenATCRunStatus->SetLampCtlBoardData(tLampCltBoardData);

	TChannelStatusInfo tChannelStatusInfo[MAX_CHANNEL_COUNT];
	memset(tChannelStatusInfo, 0x00, sizeof(tChannelStatusInfo));

	for (int i = 0;i < MAX_CHANNEL_COUNT;i++)
	{
	    m_pOpenATCRunStatus->GetChannelStatusInfo(i, tChannelStatusInfo[i]);
	}

	unsigned char *pBuffer = m_pOpenATCParameter->GetATCChannelStatus(tChannelStatusInfo);
    if (pBuffer != NULL)
    {    
        memcpy(m_chSendBuff + dwSendBuffLen, pBuffer, strlen((const char *)pBuffer));
        dwSendBuffLen += strlen((const char *)pBuffer);
    }

    if (pBuffer != NULL)
    {
        free(pBuffer);
        pBuffer = NULL;
    }

    return dwSendBuffLen;
}

int COpenATCCommWithCfgSWThread::AckCtl_AskChannelLampStatus()
{
    unsigned int dwSendBuffLen = 0;

	memset(m_chSendBuff, 0x00, SEND_BUFFER_SIZE);

	m_chSendBuff[dwSendBuffLen++] = CB_VERSION_FLAG;									//版本号
	m_chSendBuff[dwSendBuffLen++] = CB_ATC_FLAG;										//发送方标识
	m_chSendBuff[dwSendBuffLen++] = CB_CONFIGSOFTWARE_FLAG;								//接收方标识
	m_chSendBuff[dwSendBuffLen++] = LINK_CONTROL;										//数据链路码
	m_chSendBuff[dwSendBuffLen++] = m_tAreaInfo.m_chAscRegionNo;						//区域号
	memcpy(m_chSendBuff + dwSendBuffLen, &m_tAreaInfo.m_usAscRoadNo, MAX_ROADNO_LENGTH);//路口号
	dwSendBuffLen += MAX_ROADNO_LENGTH;
	m_chSendBuff[dwSendBuffLen++] = ACK_QUERY;											//操作类型
	m_chSendBuff[dwSendBuffLen++] = CTL_CHANNEK_LAMP_STATUS;							//对象标识
	m_chSendBuff[dwSendBuffLen++] = 0x01;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留

	unsigned char *pBuffer = m_pOpenATCParameter->GetChannelLampStatusInfo();
    if (pBuffer != NULL)
    {    
        memcpy(m_chSendBuff + dwSendBuffLen, pBuffer, strlen((const char *)pBuffer));
        dwSendBuffLen += strlen((const char *)pBuffer);
    }

    if (pBuffer != NULL)
    {
        free(pBuffer);
        pBuffer = NULL;
    }

    return dwSendBuffLen;
}

/*====================================================================
函数名 ：MountUSBDevice
功能   ：挂载U盘
算法实现：
参数说明：
返回值说明 ：无
------------------------------------------------------------------------------------------------------------------
修改记录：
日 期           版本    修改人             走读人             修改记录
2019/10/9       V1.0    梁厅                梁厅                 无
====================================================================*/
int COpenATCCommWithCfgSWThread::MountUSBDevice(COpenATCRunStatus * pRunStatus)
{
#ifndef _WIN32
	FILE *pf = fopen("/dev/sda1", "rb");//未发现U盘
	if (pf == NULL)
	{
		//chFailedReason = USB_NOT_FIND;
		m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "USB not find.");
		return OPENATC_RTN_FAILED;
	}
	fclose(pf);

	char cmd[128] = {0};

	memset(cmd, 0, sizeof(cmd));
	sprintf(cmd, "mount /dev/sda1 /mnt");
	pid_t status;    
	status = system(cmd);
	if (-1 != status && WIFEXITED(status) && 0 == WEXITSTATUS(status))
	{
		pRunStatus->SetUSBMountFlag(true);
	}
	else
	{
		m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "USB mount failed.");
		return OPENATC_RTN_FAILED;
	}
#endif
	return OPENATC_RTN_OK;
}

/*====================================================================
函数名 ：UnmountUSBDevice
功能   ：卸载U盘
算法实现：
参数说明：
返回值说明 ：无
------------------------------------------------------------------------------------------------------------------
修改记录：
日 期           版本    修改人             走读人             修改记录
2019/10/9       V1.0    梁厅                梁厅                 无
====================================================================*/
int COpenATCCommWithCfgSWThread::UnmountUSBDevice(COpenATCRunStatus * pRunStatus)
{
#ifndef _WIN32
	char cmd[128] = {0};

	memset(cmd, 0, sizeof(cmd));
	sprintf(cmd, "umount /dev/sda1");
	pid_t status;    
	status = system(cmd);
	if (-1 != status && WIFEXITED(status) && 0 == WEXITSTATUS(status))
	{
		pRunStatus->SetUSBMountFlag(false);
	}
	else
	{
		m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "USB unmount failed.");
		return OPENATC_RTN_FAILED;
	}
#endif
	return OPENATC_RTN_OK;
}

void  COpenATCCommWithCfgSWThread::SetClientSocket(SOCKET & socket)
{
	m_clientSock.Close();
	m_clientSock.SetSocketType(SOCK_STREAM);
    m_clientSock.SetComType(TCP_SERVICE);
	m_clientSock.SetSocket(socket);
	m_byComType = TCP_SERVICE;
}

void  COpenATCCommWithCfgSWThread::SetClientInfo(char *chClientIp, int nClientPort)
{
	memcpy(m_szClientaIp, chClientIp, sizeof(m_szClientaIp));  
    m_nClientPort = nClientPort;
}

void COpenATCCommWithCfgSWThread::RecordVerifyErrorToSystemLogFile(TErrInfo *pCheckCode)
{
	int nErrIndex = 0;
	string strLog = "";
	char cTemp[256] = {0};

	for (nErrIndex = 0; nErrIndex < C_N_MAX_ERR_SIZE; nErrIndex++)
	{
		if (pCheckCode[nErrIndex].nCheckErr > 0)
		{
			memset(cTemp, 0, sizeof(cTemp));
			sprintf(cTemp, "[%d,%d]", pCheckCode[nErrIndex].nCheckErr, pCheckCode[nErrIndex].nSubCheckErr);
			strLog += cTemp;
		}
	}

	m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "Parameter file update failed.Reason:%s.", strLog.c_str());
}

void COpenATCCommWithCfgSWThread::ParaseWorkWay(TWorkModeParam atWorkModeParam, char * chPeerIp, TManualCmd & tManualCmd, int & nResult, int & nFailCode, bool bProcess)
{
	TSystemControlStatus tSystemControlStatus;
	memset(&tSystemControlStatus, 0, sizeof(tSystemControlStatus));
	m_pOpenATCRunStatus->GetSystemControlStatus(tSystemControlStatus);

	TAscStepCfg tStepCfg;
	memset(&tStepCfg, 0, sizeof(tStepCfg));
	m_pOpenATCParameter->GetStepInfo(tStepCfg);

    if (tSystemControlStatus.m_nSpecicalControlResult != CONTROL_FAILED || tSystemControlStatus.m_nPatternControlResult != CONTROL_FAILED || tSystemControlStatus.m_nStageControlResult != CONTROL_FAILED)
	{
		if (m_pOpenATCParameter->CheckPatternNum(&atWorkModeParam))
		{
			if (atWorkModeParam.m_byControlType == CTL_MODE_FLASH || atWorkModeParam.m_byControlType == CTL_MODE_ALLRED || atWorkModeParam.m_byControlType == CTL_MODE_OFF)
			{
				if (tSystemControlStatus.m_nSpecicalControlResult != CONTROL_FAILED)
				{
					if (bProcess)
					{
						tManualCmd.m_bNewCmd = true;
						tManualCmd.m_nCmdSource = CTL_SOURCE_SYSTEM;
						memcpy(tManualCmd.m_szPeerIp, chPeerIp, strlen(chPeerIp));
						tManualCmd.m_bPatternInterruptCmd = true;
						tManualCmd.m_tPatternInterruptCmd.m_nControlMode = atWorkModeParam.m_byControlType;
						tManualCmd.m_tPatternInterruptCmd.m_nPatternNo = atWorkModeParam.m_wControlNumber;
						m_pOpenATCRunStatus->SetManualCmd(tManualCmd);
						m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "SetWorkWay, Mode is %d Plan No is %d System Control IP is %s", atWorkModeParam.m_byControlType, atWorkModeParam.m_wControlNumber, chPeerIp);
					}
				}
				else
				{
					nResult = tSystemControlStatus.m_nSpecicalControlResult;
					nFailCode = tSystemControlStatus.m_nSpecicalControlFailCode;
				}
			}
			else if (atWorkModeParam.m_byControlType == CTL_MODE_MANUAL)//手动控制
			{
				if (tSystemControlStatus.m_nStageControlResult != CONTROL_FAILED)
				{
					if (bProcess)
					{
						tManualCmd.m_bNewCmd = true;
						tManualCmd.m_nCmdSource = CTL_SOURCE_SYSTEM;
						memcpy(tManualCmd.m_szPeerIp, chPeerIp, strlen(chPeerIp));
						tManualCmd.m_tStepForwardCmd.m_byStepType = tStepCfg.m_byStepType;
						tManualCmd.m_bStepForwardCmd = true;
						tManualCmd.m_tStepForwardCmd.m_nNextStageID = atWorkModeParam.m_wControlValue;
						tManualCmd.m_tStepForwardCmd.m_nDelayTime = atWorkModeParam.m_nDelay;
						tManualCmd.m_tStepForwardCmd.m_nDurationTime = atWorkModeParam.m_nDuration;
						tManualCmd.m_tStepForwardCmd.m_byStepType = tStepCfg.m_byStepType;
						m_pOpenATCRunStatus->SetManualCmd(tManualCmd);
						m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "SetWorkWay, Mode is %d Terminal is %d Delay is %d Duration is %d System Control IP is %s", atWorkModeParam.m_byControlType, atWorkModeParam.m_wControlValue, atWorkModeParam.m_nDelay, atWorkModeParam.m_nDuration, chPeerIp);
					}
				}
				else
				{
					nResult = tSystemControlStatus.m_nStageControlResult;
					nFailCode = tSystemControlStatus.m_nStageControlFailCode;
				}
			}
			else if (atWorkModeParam.m_byControlType == CTL_MODE_MANUAL_CONTROL_PATTERN)//手动控制方案
			{
				if (tSystemControlStatus.m_nStageControlResult != CONTROL_FAILED)
				{
					if (bProcess)
					{
						nResult = CheckManualControlPattern(atWorkModeParam, nFailCode);
						if (nResult == CONTROL_SUCCEED)
						{
							tManualCmd.m_bNewCmd = true;
							tManualCmd.m_nCmdSource = CTL_SOURCE_SYSTEM;
							memcpy(tManualCmd.m_szPeerIp, chPeerIp, strlen(chPeerIp));
							tManualCmd.m_bPatternInterruptCmd = true;
							tManualCmd.m_tPatternInterruptCmd.m_nControlMode = CTL_MODE_MANUAL_CONTROL_PATTERN;
							tManualCmd.m_tPatternInterruptCmd.m_nPatternNo = MAX_PATTERN_COUNT;
							memcpy(&tManualCmd.m_tPatternInterruptCmd.m_tManualControlPattern, &atWorkModeParam.m_atManualControlPattern, sizeof(atWorkModeParam.m_atManualControlPattern));
							tManualCmd.m_tPatternInterruptCmd.m_tManualControlPattern.m_nDurationTime = atWorkModeParam.m_nDuration;
							tManualCmd.m_tPatternInterruptCmd.m_tManualControlPattern.m_nDelayTime = atWorkModeParam.m_nDelay;
							m_pOpenATCRunStatus->SetManualCmd(tManualCmd);
							m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "ManualControlPattern, Mode is %d Plan No is %d Delay is %d Duration is %d CycleTime is %d System Control IP is %s", atWorkModeParam.m_byControlType, atWorkModeParam.m_wControlNumber, atWorkModeParam.m_nDelay, atWorkModeParam.m_nDuration, tManualCmd.m_tPatternInterruptCmd.m_tManualControlPattern.m_nCycleTime, chPeerIp);
						}
				    }
				}
				else
				{
					nResult = tSystemControlStatus.m_nStageControlResult;
					nFailCode = tSystemControlStatus.m_nStageControlFailCode;
				}
			}
			else //自主或方案干预
			{
				if (tSystemControlStatus.m_nPatternControlFailCode != CONTROL_FAILED)
				{
					if (bProcess)
					{
						tManualCmd.m_bNewCmd = true;
						tManualCmd.m_nCmdSource = CTL_SOURCE_SYSTEM;
						memcpy(tManualCmd.m_szPeerIp, chPeerIp, strlen(chPeerIp));
						tManualCmd.m_bPatternInterruptCmd = true;
						tManualCmd.m_tPatternInterruptCmd.m_nControlMode = atWorkModeParam.m_byControlType;
						tManualCmd.m_tPatternInterruptCmd.m_nPatternNo = atWorkModeParam.m_wControlNumber;
						m_pOpenATCRunStatus->SetManualCmd(tManualCmd);
						m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "SetWorkWay, Mode is %d Plan No is %d Delay is %d Duration is %d System Control IP is %s", atWorkModeParam.m_byControlType, atWorkModeParam.m_wControlNumber, atWorkModeParam.m_nDelay, atWorkModeParam.m_nDuration, chPeerIp);
					}
				}
				else
				{
					nResult = tSystemControlStatus.m_nPatternControlResult;
					nFailCode = tSystemControlStatus.m_nPatternControlFailCode;
				}
			}
		}
		else
		{
			nResult = CONTROL_FAILED;
			nFailCode = NULL_PATTERNNUM;
			m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "Check Pattern Num Failed");
		}
	}
	else //面板控制时，系统指令不生效
	{
		nResult = CONTROL_FAILED;
		nFailCode = HIGH_PRIORITY_USER_CONTROL_NO_EXECUT;
	}
}

void COpenATCCommWithCfgSWThread::CreateManualCmdReturnToSelf(char * chPeerIp)
{
	TSystemControlStatus tSystemControlStatus;
	memset(&tSystemControlStatus, 0, sizeof(tSystemControlStatus));
	m_pOpenATCRunStatus->GetSystemControlStatus(tSystemControlStatus);

	if (tSystemControlStatus.m_nPatternControlResult != CONTROL_FAILED)
	{
		TManualCmd tManualCmd;
		memset(&tManualCmd, 0, sizeof(tManualCmd));

		tManualCmd.m_bNewCmd = true;
		tManualCmd.m_nCmdSource = CTL_SOURCE_SYSTEM;//命令来源设置为系统控制，因为LogicManager处理面板指令和系统指令的时候，是根据命令源来判断的
		memcpy(tManualCmd.m_szPeerIp, chPeerIp, strlen(chPeerIp));
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
		m_pOpenATCRunStatus->SetManualCmd(tManualCmd);
	}
}

void COpenATCCommWithCfgSWThread::CreateManualCmd(int nCtlWay, char * chPeerIp, int & nResult, int & nFailCode)
{
	TSystemControlStatus tSystemControlStatus;
	memset(&tSystemControlStatus, 0, sizeof(tSystemControlStatus));
	m_pOpenATCRunStatus->GetSystemControlStatus(tSystemControlStatus);

	if (tSystemControlStatus.m_nPatternControlResult != CONTROL_FAILED)
	{
		TManualCmd  tManualCmd;
		memset(&tManualCmd, 0, sizeof(tManualCmd));

		tManualCmd.m_bPatternInterruptCmd = true;
		tManualCmd.m_tPatternInterruptCmd.m_nControlMode = nCtlWay;
		if (nCtlWay != CTL_MODE_SYS_INTERRUPT)
		{
			tManualCmd.m_tPatternInterruptCmd.m_nPatternNo = 0;
			m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "CreateManualCmd To Channel Check, System Control IP is %s", chPeerIp);
		}
		else//特殊方案干预
		{
			tManualCmd.m_tPatternInterruptCmd.m_nPatternNo = MAX_PATTERN_COUNT;//方案干预对应的特殊方案号
			m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "CreateManualCmd To Pattern Intervene, System Control IP is %s", chPeerIp);
		}

		tManualCmd.m_bNewCmd = true;
		tManualCmd.m_nCmdSource = CTL_SOURCE_SYSTEM;
		memcpy(tManualCmd.m_szPeerIp, chPeerIp, strlen(chPeerIp));
		m_pOpenATCRunStatus->SetManualCmd(tManualCmd);
	}

	nResult = tSystemControlStatus.m_nPatternControlResult;
	nFailCode = tSystemControlStatus.m_nPatternControlFailCode;
}

int COpenATCCommWithCfgSWThread::AckCtl_AskQueryVecDetectorAllData()
{
	TVehDetBoardData tVehDetBoardData;                  
    m_pOpenATCRunStatus->GetVehDetBoardData(tVehDetBoardData);

    unsigned int  dwSendBuffLen = 0;
    unsigned int  dwPackedBuffSize = 0; 

	memset(m_chSendBuff, 0x00, SEND_BUFFER_SIZE);

	m_chSendBuff[dwSendBuffLen++] = CB_VERSION_FLAG;									//版本号
	m_chSendBuff[dwSendBuffLen++] = CB_ATC_FLAG;										//发送方标识
	m_chSendBuff[dwSendBuffLen++] = CB_CONFIGSOFTWARE_FLAG;								//接收方标识
	m_chSendBuff[dwSendBuffLen++] = LINK_OPTIMIZE_CONTROL;								//数据链路码
	m_chSendBuff[dwSendBuffLen++] = m_tAreaInfo.m_chAscRegionNo;						//区域号
	memcpy(m_chSendBuff + dwSendBuffLen, &m_tAreaInfo.m_usAscRoadNo, MAX_ROADNO_LENGTH);//路口号
    dwSendBuffLen += MAX_ROADNO_LENGTH;
	m_chSendBuff[dwSendBuffLen++] = ACK_QUERY;											//操作类型
	m_chSendBuff[dwSendBuffLen++] = CTL_DETECTOR;										//对象标识
	m_chSendBuff[dwSendBuffLen++] = 0x01;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留

	int nVehDetBoardCount = 0;

	int  i = 0, j = 0;
	for (i = 0;i < C_N_MAXDETBOARD_NUM;i++)
	{
		if (tVehDetBoardData.m_atVehDetData[i].m_nVehDetBoardID != 0)
		{
			nVehDetBoardCount += 1;
		}
	}

	m_chSendBuff[dwSendBuffLen++] = nVehDetBoardCount;									//车检板数量

	TVehicleDetector atRealVehDetector[MAX_VEHICLEDETECTOR_COUNT];
    memset(atRealVehDetector, 0, sizeof(atRealVehDetector));
    m_pOpenATCParameter->GetVehicleDetectorTable(atRealVehDetector);
   
	for (i = 0;i < nVehDetBoardCount;i++)
	{
		m_chSendBuff[dwSendBuffLen++] = tVehDetBoardData.m_atVehDetData[i].m_nVehDetBoardID;

		for (j = 0;j < C_N_MAXDETINPUT_NUM;j++)
		{
			m_chSendBuff[dwSendBuffLen++] = tVehDetBoardData.m_atVehDetData[i].m_achVehTimerVal[j];
			m_chSendBuff[dwSendBuffLen++] = atRealVehDetector[i * C_N_MAXDETINPUT_NUM + j].m_byVehicleDetectorCallPhase;
		}
	}

	return dwSendBuffLen;
}

int COpenATCCommWithCfgSWThread::AckCtl_AskVecDetectorReportChgData(void)
{
    unsigned int  dwSendBuffLen = 0;
    unsigned int  dwPackedBuffSize = 0; 

	memset(m_chSendBuff, 0x00, SEND_BUFFER_SIZE);

	m_chSendBuff[dwSendBuffLen++] = CB_VERSION_FLAG;									//版本号
	m_chSendBuff[dwSendBuffLen++] = CB_ATC_FLAG;										//发送方标识
	m_chSendBuff[dwSendBuffLen++] = CB_CONFIGSOFTWARE_FLAG;								//接收方标识
	m_chSendBuff[dwSendBuffLen++] = LINK_OPTIMIZE_CONTROL;								//数据链路码
	m_chSendBuff[dwSendBuffLen++] = m_tAreaInfo.m_chAscRegionNo;						//区域号
	memcpy(m_chSendBuff + dwSendBuffLen, &m_tAreaInfo.m_usAscRoadNo, MAX_ROADNO_LENGTH);//路口号
    dwSendBuffLen += MAX_ROADNO_LENGTH;
	m_chSendBuff[dwSendBuffLen++] = ACK_SET;											//操作类型
	m_chSendBuff[dwSendBuffLen++] = CTL_DETECTOR;										//对象标识
	m_chSendBuff[dwSendBuffLen++] = 0x01;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留

	return dwSendBuffLen;
}

int COpenATCCommWithCfgSWThread::SendDetectorChgData()
{
	if (m_chSendDetectorChgData == 0)
	{
		return 0;
	}

	TVehDetBoardData tVehDetBoardData;                  
    m_pOpenATCRunStatus->GetVehDetBoardData(tVehDetBoardData);

	if (memcmp(&m_tOldVehDetBoardData,&tVehDetBoardData,sizeof(TVehDetBoardData)) == 0)
	{
		return 0;
	}

	TVetDetectorChgData  tVetDetectorChgData[C_N_MAXDETBOARD_NUM];
	memset(tVetDetectorChgData,0,sizeof(tVetDetectorChgData));

	int  i = 0, j = 0;
	int  nVehDetBoardCount = 0;
	int  nVehDetectorCount = 0;
	bool bFlag[C_N_MAXDETBOARD_NUM];
	memset(bFlag,0,sizeof(bFlag));
	char chVehDetectorIndex[C_N_MAXDETINPUT_NUM];
	memset(chVehDetectorIndex,0,sizeof(chVehDetectorIndex));

	for (i = 0;i < C_N_MAXDETBOARD_NUM;i++)
	{
		for (j = 0;j < C_N_MAXDETINPUT_NUM;j++)
		{
			if (m_tOldVehDetBoardData.m_atVehDetData[i].m_achVehChgVal[j] == 0 && tVehDetBoardData.m_atVehDetData[i].m_achVehChgVal[j] == 1)
			{
				bFlag[i] = true;
				chVehDetectorIndex[nVehDetectorCount] = j;
				nVehDetectorCount += 1;
			}
		}

		if (bFlag[i])
		{
			tVetDetectorChgData[nVehDetBoardCount].nVehDetBoardIndex = i;
			tVetDetectorChgData[nVehDetBoardCount].nVehDetBoardID = tVehDetBoardData.m_atVehDetData[i].m_nVehDetBoardID;
			tVetDetectorChgData[nVehDetBoardCount].nVehDetectorCount = nVehDetectorCount;
			for (j = 0;j < nVehDetectorCount;j++)
			{
				tVetDetectorChgData[nVehDetBoardCount].chVehDetectorIndex[j] = chVehDetectorIndex[j];
			}
			nVehDetBoardCount += 1;
		}
	}

	memcpy(&m_tOldVehDetBoardData,&tVehDetBoardData,sizeof(TVehDetBoardData));

	if (nVehDetBoardCount == 0)
	{
		return 0;
	}

    unsigned int  dwSendBuffLen = 0;
    unsigned int  dwPackedBuffSize = 0; 

	memset(m_chSendBuff, 0x00, SEND_BUFFER_SIZE);

	m_chSendBuff[dwSendBuffLen++] = CB_VERSION_FLAG;									//版本号
	m_chSendBuff[dwSendBuffLen++] = CB_ATC_FLAG;										//发送方标识
	m_chSendBuff[dwSendBuffLen++] = CB_CONFIGSOFTWARE_FLAG;								//接收方标识
	m_chSendBuff[dwSendBuffLen++] = LINK_OPTIMIZE_CONTROL;								//数据链路码
	m_chSendBuff[dwSendBuffLen++] = m_tAreaInfo.m_chAscRegionNo;						//区域号
	memcpy(m_chSendBuff + dwSendBuffLen, &m_tAreaInfo.m_usAscRoadNo, MAX_ROADNO_LENGTH);//路口号
    dwSendBuffLen += MAX_ROADNO_LENGTH;
	m_chSendBuff[dwSendBuffLen++] = REPORT_STATUS;										//操作类型
	m_chSendBuff[dwSendBuffLen++] = CTL_DETECTOR;										//对象标识
	m_chSendBuff[dwSendBuffLen++] = 0x01;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留

	TVehicleDetector atRealVehDetector[MAX_VEHICLEDETECTOR_COUNT];
    memset(atRealVehDetector, 0, sizeof(atRealVehDetector));
    m_pOpenATCParameter->GetVehicleDetectorTable(atRealVehDetector);

	m_chSendBuff[dwSendBuffLen++] = nVehDetBoardCount;									//车检板数量

	for (i = 0;i < nVehDetBoardCount;i++)
	{
		m_chSendBuff[dwSendBuffLen++] = tVetDetectorChgData[i].nVehDetBoardID;			//车检板ID

		for (j = 0;j < tVetDetectorChgData[i].nVehDetectorCount;j++)
		{
			int  nVetDetBoardIndex = tVetDetectorChgData[i].nVehDetBoardIndex;
			char chVehDetectorIndex = tVetDetectorChgData[i].chVehDetectorIndex[j];

			m_chSendBuff[dwSendBuffLen++] = tVetDetectorChgData[i].chVehDetectorIndex[j] + 1; //车检器ID
			m_chSendBuff[dwSendBuffLen++] = atRealVehDetector[nVetDetBoardIndex * C_N_MAXDETINPUT_NUM + chVehDetectorIndex].m_byVehicleDetectorCallPhase;
		}
	}
	m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "SendDetectorChgData succeed");
	return SendAckToPeer(dwSendBuffLen);
}

int  COpenATCCommWithCfgSWThread::CreateChannelLockCmd(TChannelLockCtrlCmd atChannelLockCtrlCmd, TManualCmd & tManualCmd, char * chPeerIp)
{
	bool bFlag = false;
	int  i = 0, nPackSize = 0;
	for (i = 0; i < MAX_CHANNEL_COUNT;i++)
	{
		if (atChannelLockCtrlCmd.m_nChannelLockStatus[i] == CHANNEL_STATUS_GREEN)
		{
			bFlag = true;
			m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "ChannelLock, ChannelID:%d Green", i + 1);
		}
		else if (atChannelLockCtrlCmd.m_nChannelLockStatus[i] == CHANNEL_STATUS_YELLOW)
		{
			bFlag = true;
			m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "ChannelLock, ChannelID:%d Yellow", i + 1);
		}
		else if (atChannelLockCtrlCmd.m_nChannelLockStatus[i] == CHANNEL_STATUS_RED)
		{
			if (m_nOldChannelLockStatus[i] == CHANNEL_STATUS_GREEN)
			{
				bFlag = true;
			}
			m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "ChannelLock, ChannelID:%d Red", i + 1);
		}
		else if (atChannelLockCtrlCmd.m_nChannelLockStatus[i] == CHANNEL_STATUS_OFF)
		{
			if (m_nOldChannelLockStatus[i] == CHANNEL_STATUS_GREEN)
			{
				bFlag = true;
			}
			m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "ChannelLock, ChannelID:%d Off", i + 1);
		}
	}

	if (!bFlag)
	{
		if (!tManualCmd.m_bPhaseToChannelLock)
		{
			nPackSize = AckCtl_AskSetCmdWorkWay(CTL_MODE_CHANNEL_LOCK, NO_SUPPORT_CONTROL_PARAM);
		}
		else
		{
			nPackSize = AckCtl_AskSetCmdWorkWay(CTL_MODE_PHASE_LOCK, NO_SUPPORT_CONTROL_PARAM);
		}
		m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "ChannelLock AllChannel Default");
		return SendAckToPeer(nPackSize);
	}

	if (m_pOpenATCParameter->CheckGreenConflictByChannelLock(atChannelLockCtrlCmd.m_nChannelLockStatus) == OPENATC_PARAM_CHECK_FAILED)
	{
		if (!tManualCmd.m_bPhaseToChannelLock)
		{
			nPackSize = AckCtl_AskSetCmdWorkWay(CTL_MODE_CHANNEL_LOCK, CONFIG_INCLUDE_GREENCONFLICT);
		}
		else
		{
			nPackSize = AckCtl_AskSetCmdWorkWay(CTL_MODE_PHASE_LOCK, CONFIG_INCLUDE_GREENCONFLICT);
		}
		m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "CheckGreenConflictByChannelLock Failed");
		return SendAckToPeer(nPackSize);
	}

	tManualCmd.m_bNewCmd = true;
	tManualCmd.m_nCmdSource = CTL_SOURCE_SYSTEM;
	memcpy(tManualCmd.m_szPeerIp, chPeerIp, strlen(chPeerIp));
	tManualCmd.m_bChannelLockCmd = true;
	tManualCmd.m_tChannelLockCmd.m_tNextChannelLockCtrlCmd.m_nDuration = atChannelLockCtrlCmd.m_nDuration;
	tManualCmd.m_tChannelLockCmd.m_tNextChannelLockCtrlCmd.m_nGreenFlash = atChannelLockCtrlCmd.m_nGreenFlash;
	tManualCmd.m_tChannelLockCmd.m_tNextChannelLockCtrlCmd.m_nYellow = atChannelLockCtrlCmd.m_nYellow;
	tManualCmd.m_tChannelLockCmd.m_tNextChannelLockCtrlCmd.m_nRedClear = atChannelLockCtrlCmd.m_nRedClear;
	tManualCmd.m_tChannelLockCmd.m_tNextChannelLockCtrlCmd.m_nMinGreen = atChannelLockCtrlCmd.m_nMinGreen;
	for (i = 0; i < MAX_CHANNEL_COUNT;i++)
	{
		tManualCmd.m_tChannelLockCmd.m_tNextChannelLockCtrlCmd.m_nChannelLockStatus[i] = atChannelLockCtrlCmd.m_nChannelLockStatus[i];
		m_nOldChannelLockStatus[i] = atChannelLockCtrlCmd.m_nChannelLockStatus[i];
	}
	m_pOpenATCRunStatus->SetManualCmd(tManualCmd);

	m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "ChannelLock, Duration:%d GreenFlash:%d Yellow:%d Red:%d MinGreen:%d, System Control IP is %s",
		atChannelLockCtrlCmd.m_nDuration, atChannelLockCtrlCmd.m_nGreenFlash, atChannelLockCtrlCmd.m_nYellow, atChannelLockCtrlCmd.m_nRedClear, atChannelLockCtrlCmd.m_nMinGreen, chPeerIp);

}

bool COpenATCCommWithCfgSWThread::TransPhaseLockCmdToChannelLockCmd(TPhaseLockCtrlCmd atPhaseLockCtrlCmd, TChannelLockCtrlCmd & atChannelLockCtrlCmd, TManualCmd & tManualCmd, char * chPeerIp)
{
	bool bChannelLockFlag = false;
	bool bOverlapChannelLockFlag = false;
	int  i = 0, j = 0, nCount = 0;

	TOverlapTable atOverlapTable[MAX_OVERLAP_COUNT];
	memset(atOverlapTable, 0, sizeof(atOverlapTable));
    m_pOpenATCParameter->GetOverlapTable(atOverlapTable);

	TChannel atChannelInfo[MAX_CHANNEL_COUNT];
	memset(atChannelInfo, 0, sizeof(atChannelInfo));
	TChannel atRealChannelInfo[MAX_CHANNEL_COUNT];
	memset(atRealChannelInfo, 0, sizeof(atRealChannelInfo));
	m_pOpenATCParameter->GetChannelTable(atRealChannelInfo);

	atChannelLockCtrlCmd.m_nDuration = atPhaseLockCtrlCmd.m_nDuration;
	atChannelLockCtrlCmd.m_nGreenFlash = atPhaseLockCtrlCmd.m_nGreenFlash;
	atChannelLockCtrlCmd.m_nYellow = atPhaseLockCtrlCmd.m_nYellow;
	atChannelLockCtrlCmd.m_nRedClear = atPhaseLockCtrlCmd.m_nRedClear;
	atChannelLockCtrlCmd.m_nMinGreen = atPhaseLockCtrlCmd.m_nMinGreen;

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

		if (atChannelInfo[i].m_byChannelControlSource != 0)
		{
			//配置的通道默认锁定为红色，没有配置的通道默认锁定为空
			atChannelLockCtrlCmd.m_nChannelLockStatus[i] = CHANNEL_STATUS_RED;
		}
	}

	for (i = 0; i < MAX_PHASE_COUNT;i++)
	{
		if (atPhaseLockCtrlCmd.m_nPhaseLockType[i] != LOCK_TYPE_CANCEL)
		{
			if (atPhaseLockCtrlCmd.m_nPhaseLockType[i] == LOCK_TYPE_ALL)//锁定机动车相位，机动车跟随相位，行人相位，行人跟随相位
			{
				tManualCmd.m_tPhaseLockPara.m_nPhaseLockID[tManualCmd.m_tPhaseLockPara.m_nPhaseLockCount] = i + 1;
				tManualCmd.m_tPhaseLockPara.m_nPhaseLockType[tManualCmd.m_tPhaseLockPara.m_nPhaseLockCount] = LOCK_TYPE_ALL;
				tManualCmd.m_tPhaseLockPara.m_nPhaseLockCount += 1;

				bOverlapChannelLockFlag = false;
				for (int nOverlapIndex = 0;nOverlapIndex < MAX_OVERLAP_COUNT;nOverlapIndex++)
				{
					for (int nIncludedPhaseIndex = 0;nIncludedPhaseIndex < MAX_PHASE_COUNT_IN_OVERLAP;nIncludedPhaseIndex++)
					{
						if (atOverlapTable[nOverlapIndex].m_byArrOverlapIncludedPhases[nIncludedPhaseIndex] == i + 1)
						{
				            tManualCmd.m_tPhaseLockPara.m_nOverlapPhaseLockID[tManualCmd.m_tPhaseLockPara.m_nOverlapPhaseLockCount] = nOverlapIndex + 1;
							tManualCmd.m_tPhaseLockPara.m_nOverlapPhaseLockType[tManualCmd.m_tPhaseLockPara.m_nOverlapPhaseLockCount] = LOCK_TYPE_ALL;
							tManualCmd.m_tPhaseLockPara.m_nOverlapPhaseLockCount += 1;
							break;
						}				
					}  
				}
	
			}
			if (atPhaseLockCtrlCmd.m_nPhaseLockType[i] == LOCK_TYPE_VEH)//锁定机动车相位，机动车跟随相位
			{
				tManualCmd.m_tPhaseLockPara.m_nPhaseLockID[tManualCmd.m_tPhaseLockPara.m_nPhaseLockCount] = i + 1;
				tManualCmd.m_tPhaseLockPara.m_nPhaseLockType[tManualCmd.m_tPhaseLockPara.m_nPhaseLockCount] = LOCK_TYPE_VEH;
				tManualCmd.m_tPhaseLockPara.m_nPhaseLockCount += 1;

				bOverlapChannelLockFlag = false;
				for (int nOverlapIndex = 0;nOverlapIndex < MAX_OVERLAP_COUNT;nOverlapIndex++)
				{
					for (int nIncludedPhaseIndex = 0;nIncludedPhaseIndex < MAX_PHASE_COUNT_IN_OVERLAP;nIncludedPhaseIndex++)
					{
						if (atOverlapTable[nOverlapIndex].m_byArrOverlapIncludedPhases[nIncludedPhaseIndex] == i + 1)
						{
				            tManualCmd.m_tPhaseLockPara.m_nOverlapPhaseLockID[tManualCmd.m_tPhaseLockPara.m_nOverlapPhaseLockCount] = nOverlapIndex + 1;
							tManualCmd.m_tPhaseLockPara.m_nOverlapPhaseLockType[tManualCmd.m_tPhaseLockPara.m_nOverlapPhaseLockCount] = LOCK_TYPE_VEH;
							tManualCmd.m_tPhaseLockPara.m_nOverlapPhaseLockCount += 1;
							break;
						}				
					}  
				}
			}
			if (atPhaseLockCtrlCmd.m_nPhaseLockType[i] == LOCK_TYPE_PED)//锁定行人相位，行人跟随相位
			{
				tManualCmd.m_tPhaseLockPara.m_nPhaseLockID[tManualCmd.m_tPhaseLockPara.m_nPhaseLockCount] = i + 1;
				tManualCmd.m_tPhaseLockPara.m_nPhaseLockType[tManualCmd.m_tPhaseLockPara.m_nPhaseLockCount] = LOCK_TYPE_PED;
				tManualCmd.m_tPhaseLockPara.m_nPhaseLockCount += 1;

				bOverlapChannelLockFlag = false;
				for (int nOverlapIndex = 0;nOverlapIndex < MAX_OVERLAP_COUNT;nOverlapIndex++)
				{
					for (int nIncludedPhaseIndex = 0;nIncludedPhaseIndex < MAX_PHASE_COUNT_IN_OVERLAP;nIncludedPhaseIndex++)
					{
						if (atOverlapTable[nOverlapIndex].m_byArrOverlapIncludedPhases[nIncludedPhaseIndex] == i + 1)
						{
				            tManualCmd.m_tPhaseLockPara.m_nOverlapPhaseLockID[tManualCmd.m_tPhaseLockPara.m_nOverlapPhaseLockCount] = nOverlapIndex + 1;
							tManualCmd.m_tPhaseLockPara.m_nOverlapPhaseLockType[tManualCmd.m_tPhaseLockPara.m_nOverlapPhaseLockCount] = LOCK_TYPE_PED;
							tManualCmd.m_tPhaseLockPara.m_nOverlapPhaseLockCount += 1;
							break;
						}				
					}  
				}
			}

			for (j = 0; j < MAX_CHANNEL_COUNT;j++)
			{
				bChannelLockFlag = false;
				bOverlapChannelLockFlag = false;

				if (atChannelInfo[j].m_byChannelControlType == VEH_CHA)
				{
					if (atChannelInfo[j].m_byChannelControlSource == i + 1 && (atPhaseLockCtrlCmd.m_nPhaseLockType[i] == LOCK_TYPE_ALL || atPhaseLockCtrlCmd.m_nPhaseLockType[i] == LOCK_TYPE_VEH))
					{
						atChannelLockCtrlCmd.m_nChannelLockStatus[j] = CHANNEL_STATUS_GREEN;
						bChannelLockFlag = true;
					}
				}
				else if (atChannelInfo[j].m_byChannelControlType == PED_CHA)
				{
					if (atChannelInfo[j].m_byChannelControlSource == i + 1 && (atPhaseLockCtrlCmd.m_nPhaseLockType[i] == LOCK_TYPE_ALL || atPhaseLockCtrlCmd.m_nPhaseLockType[i] == LOCK_TYPE_PED))
					{
						atChannelLockCtrlCmd.m_nChannelLockStatus[j] = CHANNEL_STATUS_GREEN;
						bChannelLockFlag = true;
					}
				}
				else if (atChannelInfo[j].m_byChannelControlType == OVERLAP_CHA || atChannelInfo[j].m_byChannelControlType == OVERLAP_PED_CHA)
				{   
					if ((atChannelInfo[j].m_byChannelControlType == OVERLAP_CHA && (atPhaseLockCtrlCmd.m_nPhaseLockType[i] == LOCK_TYPE_ALL || atPhaseLockCtrlCmd.m_nPhaseLockType[i] == LOCK_TYPE_VEH)) || 
						(atChannelInfo[j].m_byChannelControlType == OVERLAP_PED_CHA && (atPhaseLockCtrlCmd.m_nPhaseLockType[i] == LOCK_TYPE_ALL || atPhaseLockCtrlCmd.m_nPhaseLockType[i] == LOCK_TYPE_PED)))
					{
						for (int nOverlapIndex = 0;nOverlapIndex < MAX_OVERLAP_COUNT;nOverlapIndex++)
						{
							if (atChannelInfo[j].m_byChannelControlSource == atOverlapTable[nOverlapIndex].m_byOverlapNumber)
							{
								bOverlapChannelLockFlag = false;
								for (int nIncludedPhaseIndex = 0;nIncludedPhaseIndex < MAX_PHASE_COUNT_IN_OVERLAP;nIncludedPhaseIndex++)
								{
									if (atOverlapTable[nOverlapIndex].m_byArrOverlapIncludedPhases[nIncludedPhaseIndex] == i + 1)
									{
										bOverlapChannelLockFlag = true;
										break;
									}				
								}  

								if (bOverlapChannelLockFlag)
								{
									atChannelLockCtrlCmd.m_nChannelLockStatus[j] = CHANNEL_STATUS_GREEN;
								}
							}
						}
					}
				}
			}
		}
		else
		{
			nCount += 1;
		}
	}

	if (nCount == MAX_PHASE_COUNT)
	{
		m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "Set all phase unLock!");
		return false;
	}
	else
	{
		tManualCmd.m_bPhaseToChannelLock = true;
	}

	return true;
}

int COpenATCCommWithCfgSWThread::CheckManualControlPattern(TWorkModeParam & atWorkModeParam, int & nFailCode)
{
	int nPhaseCount[MAX_RING_COUNT];
	memset(nPhaseCount,0,sizeof(nPhaseCount));

    int nRingCount = 0, nPhaseIndex = 0, nPhaseNum = 0;
    int i = 0, j = 0, k = 0;
    for (i = 0;i < MAX_RING_COUNT;i++)
    {
        nPhaseIndex = 0;
        for (j = 0;j < MAX_SEQUENCE_DATA_LENGTH;j++)
        {
            nPhaseNum = (int)atWorkModeParam.m_atManualControlPattern.InterruptSequence[i].m_bySequenceData[j];
            if (nPhaseNum == 0)
            {
                break;
            }
        
			WORD wSplitTime = 0;
            for (k = 0;k < MAX_PHASE_COUNT;k ++)
            {
                if (atWorkModeParam.m_atManualControlPattern.InterruptSplit[k].m_bySplitPhase == nPhaseNum/* && atWorkModeParam.m_atManualControlPattern.InterruptSplit[k].m_bySplitMode != NEGLECT_MODE*/)
                {
                    wSplitTime = atWorkModeParam.m_atManualControlPattern.InterruptSplit[k].m_wSplitTime;  
                    break; 
                }
            }

            if (wSplitTime == 0)
            {
                continue;
            }

            nPhaseIndex++;
        }

		if (nPhaseIndex != 0)
		{
			nPhaseCount[nRingCount] = nPhaseIndex;
			nRingCount++;
		}
    }

	//校验周期是否相等
	int nCycleTime = 0;
	int nCycleCalc = 0;
	int nSplitIndex = 0;
	for (i = 0;i < nRingCount;i++)
	{
		nCycleCalc = 0;
		for (j = 0;j < nPhaseCount[i];j++)
		{
			if (nRingCount > 1)
			{
				//if (atWorkModeParam.m_atManualControlPattern.InterruptSplit[nSplitIndex].m_bySplitMode == NEGLECT_MODE)
				{
					if (i == 0)
					{
						//atWorkModeParam.m_atManualControlPattern.InterruptSplit[nSplitIndex].m_wSplitTime = atWorkModeParam.m_atManualControlPattern.InterruptSplit[nSplitIndex + nPhaseCount[0]].m_wSplitTime;
					}
					else
					{
						//atWorkModeParam.m_atManualControlPattern.InterruptSplit[nSplitIndex].m_wSplitTime = atWorkModeParam.m_atManualControlPattern.InterruptSplit[nSplitIndex - nPhaseCount[0]].m_wSplitTime;
					}
				}
			}

			nCycleCalc += atWorkModeParam.m_atManualControlPattern.InterruptSplit[nSplitIndex].m_wSplitTime;
			nSplitIndex += 1;
		}

		if (nCycleTime == 0)
		{
			nCycleTime = nCycleCalc;
		}
		else
		{
			if (nCycleCalc > 0 && nCycleTime != nCycleCalc)
			{
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "CheckManualControlPattern Cycle Inconsistent In Different Ring");
				nFailCode = CYCLE_INCONSISTENT_IN_DIFFENT_RING;
				return CONTROL_FAILED;
			}
		}
	}

	//校验绿灯时间
	TPhase atPhaseTable[MAX_PHASE_COUNT];
    m_pOpenATCParameter->GetPhaseTable(atPhaseTable);
	int nGreenTime = 0;
	for (i = 0;i < MAX_PHASE_COUNT;i++)
	{
		for (j = 0;j < MAX_PHASE_COUNT;j++)
		{
			if (atWorkModeParam.m_atManualControlPattern.InterruptSplit[i].m_bySplitPhase == atPhaseTable[j].m_byPhaseNumber &&
				atWorkModeParam.m_atManualControlPattern.InterruptSplit[i].m_bySplitMode != NEGLECT_MODE)
			{
				nGreenTime = atWorkModeParam.m_atManualControlPattern.InterruptSplit[i].m_wSplitTime - 
					 atPhaseTable[j].m_byGreenFlash - atPhaseTable[j].m_byPhaseYellowChange - atPhaseTable[j].m_byPhaseRedClear;
				if (nGreenTime < (atPhaseTable[j].m_wPhaseMinimumGreen - atPhaseTable[j].m_byGreenFlash))
				{
					m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "CheckManualControlPattern GreenTime Less Than MinGreen");
					nFailCode = SPLIT_TIME_LESS_THAN_MIN_GREEN;
				    return CONTROL_FAILED;
				}
				else if (nGreenTime > (atPhaseTable[j].m_wPhaseMaximum1 - atPhaseTable[j].m_byGreenFlash))
				{
					m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "CheckManualControlPattern GreenTime More Than MinGreen");
					nFailCode = SPLIT_TIME_MORE_THAN_MAX_GREEN;
				    return CONTROL_FAILED;
				}
			}
		}
	}

	nSplitIndex = 0;//初始化

	TRunStageInfo   tRunStageInfo;
    memset(&tRunStageInfo,0,sizeof(tRunStageInfo));

	int nStageTime[MAX_RING_COUNT][MAX_PHASE_COUNT] = {0};//环内相位节点  
    int nAllStageTime[MAX_STAGE_COUNT] = {0};//阶段节点表

	int m = 0;
	int n = 0;
	int nAllStageCount = 0;

	//生成阶段节点表
	for (i = 0;i < nRingCount;i++)
	{
		for (j = 0;j < nPhaseCount[i];j++)
		{
			/*if (atWorkModeParam.m_atManualControlPattern.InterruptSplit[nSplitIndex].m_bySplitMode == NEGLECT_MODE)
			{
				nSplitIndex += 1;
			}*/

			if (j == 0)
			{
				nStageTime[i][0] = atWorkModeParam.m_atManualControlPattern.InterruptSplit[nSplitIndex].m_wSplitTime;
			}
			else
			{
				nStageTime[i][j] = atWorkModeParam.m_atManualControlPattern.InterruptSplit[nSplitIndex].m_wSplitTime + nStageTime[i][j - 1];
			}

			nSplitIndex += 1;

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

	nSplitIndex = 0;//初始化
    
	//生成阶段相位表
	nPhaseIndex = 0;
	for (i = 0; i < nRingCount; i++)
	{
		for (j = 0; j < nPhaseCount[i]; j++)
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

			/*if (atWorkModeParam.m_atManualControlPattern.InterruptSplit[nSplitIndex].m_bySplitMode == NEGLECT_MODE)
			{
				nSplitIndex += 1;
			}*/
			
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
        
					tRunStageInfo.m_PhaseRunstageInfo[m].m_nConcurrencyPhase[i] = atWorkModeParam.m_atManualControlPattern.InterruptSplit[nSplitIndex].m_bySplitPhase;
					tRunStageInfo.m_PhaseRunstageInfo[m].m_nConcurrencyPhaseCount += 1;
				}
			}

			nSplitIndex += 1;
		}
	}

	 /*************************************************************************/
    //通过生成的阶段表来反推相序
    TRunStageInfo   tTempRunStageInfo;
    memset(&tTempRunStageInfo, 0, sizeof(tTempRunStageInfo));

    int nConcurrencyPhaseCount[MAX_STAGE_COUNT];
    memset(nConcurrencyPhaseCount, 0, sizeof(nConcurrencyPhaseCount));

    int nConcurrencyPhase[MAX_RING_COUNT][MAX_STAGE_COUNT];
    memset(nConcurrencyPhase, 0, sizeof(nConcurrencyPhase));

    int nNeglectStageCount = 0;
    int nNeglectStage[MAX_STAGE_COUNT];
    memset(nNeglectStage, 0, sizeof(nNeglectStage));

    bool bOverStage[MAX_RING_COUNT];

    for (i = 0; i < tRunStageInfo.m_nRunStageCount; i++)
    {
        nConcurrencyPhaseCount[i] = tRunStageInfo.m_PhaseRunstageInfo[i].m_nConcurrencyPhaseCount;

        for (j = 0; j < tRunStageInfo.m_PhaseRunstageInfo[i].m_nConcurrencyPhaseCount; j++)
        {
            for (m = 0; m < MAX_PHASE_COUNT; m++)
            {
                if (tRunStageInfo.m_PhaseRunstageInfo[i].m_nConcurrencyPhase[j] == atWorkModeParam.m_atManualControlPattern.InterruptSplit[m].m_bySplitPhase && (atWorkModeParam.m_atManualControlPattern.InterruptSplit[m].m_bySplitMode == NEGLECT_MODE || atWorkModeParam.m_atManualControlPattern.InterruptSplit[m].m_bySplitMode == SHIELD_MODE))
                {
                    nConcurrencyPhase[j][i] = tRunStageInfo.m_PhaseRunstageInfo[i].m_nConcurrencyPhase[j];
                    nConcurrencyPhaseCount[i] -= 1;
                }
            }
        }

        if (nConcurrencyPhaseCount[i] != 0)
        {
            memcpy(&tTempRunStageInfo.m_PhaseRunstageInfo[tTempRunStageInfo.m_nRunStageCount], &tRunStageInfo.m_PhaseRunstageInfo[i], sizeof(tRunStageInfo.m_PhaseRunstageInfo[i]));
            tTempRunStageInfo.m_PhaseRunstageInfo[tTempRunStageInfo.m_nRunStageCount].m_nStageIndex = tTempRunStageInfo.m_nRunStageCount;
            tTempRunStageInfo.m_nRunStageCount += 1;
        }
        else
        {
            //阶段为忽略阶段
            nNeglectStage[nNeglectStageCount++] = i + 1;
        }
    }

    //根据阶段表重新生成相序
    //考虑是否有跨阶段
    //根据nNeglectStage定位到忽略的阶段，再对应的前后两个阶段是否为相同的相位，如果是，则保留该相序，并减去对应阶段时间
    //前后两个阶段如果没有相同的相位，则直接在相序中忽略该相位
    for (int iNeglectStageIndex = 0; iNeglectStageIndex < nNeglectStageCount; iNeglectStageIndex++)
    {
        //m_tFixTimeCtlInfo.m_wCycleLen = m_tFixTimeCtlInfo.m_wCycleLen - (tRunStageInfo.m_PhaseRunstageInfo[nNeglectStage[iNeglectStageIndex] - 1].m_nStageEndTime - m_tRunStageInfo.m_PhaseRunstageInfo[nNeglectStage[iNeglectStageIndex] - 1].m_nStageStartTime);
        memset(bOverStage, 0, sizeof(bOverStage));
        for (int jRingIndex = 0; jRingIndex < tRunStageInfo.m_PhaseRunstageInfo[nNeglectStage[iNeglectStageIndex] - 1].m_nConcurrencyPhaseCount; jRingIndex++)
        {
            if (nNeglectStage[iNeglectStageIndex] == 1)
            {
                if (nConcurrencyPhase[jRingIndex][0] != nConcurrencyPhase[jRingIndex][1])
                {
                    bOverStage[jRingIndex] = true;              //该阶段对应jRingIndex环的相位直接忽略掉
                }
            }
            else if (nNeglectStage[iNeglectStageIndex] == tRunStageInfo.m_nRunStageCount)
            {
                //忽略阶段为最后一个阶段，只需要判断和前一阶段是否跨阶段即可
                if (nConcurrencyPhase[jRingIndex][nNeglectStage[iNeglectStageIndex] - 2] != nConcurrencyPhase[jRingIndex][nNeglectStage[iNeglectStageIndex] - 1])
                {
                    bOverStage[jRingIndex] = true;              //该阶段对应jRingIndex环的相位直接忽略掉
                }
            }
            else
            {
                if ((nConcurrencyPhase[jRingIndex][nNeglectStage[iNeglectStageIndex] - 2] != nConcurrencyPhase[jRingIndex][nNeglectStage[iNeglectStageIndex] - 1]) &&
                    (nConcurrencyPhase[jRingIndex][nNeglectStage[iNeglectStageIndex] - 1] != nConcurrencyPhase[jRingIndex][nNeglectStage[iNeglectStageIndex]]))
                {
                    bOverStage[jRingIndex] = true;              //该阶段对应jRingIndex环的相位直接忽略掉
                }
            }
        }
    }

    if (tTempRunStageInfo.m_nRunStageCount > 0 && memcmp(&tRunStageInfo, &tTempRunStageInfo, sizeof(tRunStageInfo) != 0))
    {
        memcpy(&tRunStageInfo, &tTempRunStageInfo, sizeof(tRunStageInfo));
    }

	/*TRunStageInfo   tTempRunStageInfo;
    memset(&tTempRunStageInfo,0,sizeof(tTempRunStageInfo));

	int nConcurrencyPhaseCount[MAX_STAGE_COUNT];
	memset(nConcurrencyPhaseCount,0,sizeof(nConcurrencyPhaseCount));

	for (i = 0;i < tRunStageInfo.m_nRunStageCount;i++)      
	{
		nConcurrencyPhaseCount[i] = tRunStageInfo.m_PhaseRunstageInfo[i].m_nConcurrencyPhaseCount;

		for (j = 0; j < tRunStageInfo.m_PhaseRunstageInfo[i].m_nConcurrencyPhaseCount;j++)
		{
			for (k = 0; k < MAX_PHASE_COUNT; k++)
			{
				if (tRunStageInfo.m_PhaseRunstageInfo[i].m_nConcurrencyPhase[j] == atWorkModeParam.m_atManualControlPattern.InterruptSplit[k].m_bySplitPhase &&
					atWorkModeParam.m_atManualControlPattern.InterruptSplit[k].m_bySplitMode == NEGLECT_MODE)
				{
					nConcurrencyPhaseCount[i] -= 1;
				}
			}
		}

		if (nConcurrencyPhaseCount[i] != 0)
		{
			memcpy(&tTempRunStageInfo.m_PhaseRunstageInfo[tTempRunStageInfo.m_nRunStageCount], &tRunStageInfo.m_PhaseRunstageInfo[i], sizeof(tRunStageInfo.m_PhaseRunstageInfo[i]));
			tTempRunStageInfo.m_PhaseRunstageInfo[tTempRunStageInfo.m_nRunStageCount].m_nStageIndex = tTempRunStageInfo.m_nRunStageCount;
			tTempRunStageInfo.m_nRunStageCount += 1;
		}
	}

	if (tTempRunStageInfo.m_nRunStageCount > 0 && memcmp(&tRunStageInfo, &tTempRunStageInfo, sizeof(tRunStageInfo) != 0))
	{
		memcpy(&tRunStageInfo, &tTempRunStageInfo, sizeof(tRunStageInfo));
	}*/

	bool bFlag = false;

	//校验并发相位
	for (i = 0;i < tRunStageInfo.m_nRunStageCount;i++)      
	{
		bFlag = false;
		for (j = 1; j < tRunStageInfo.m_PhaseRunstageInfo[i].m_nConcurrencyPhaseCount;j++)
		{
			for (k = 0;k < MAX_PHASE_COUNT;k++)
			{
				if (tRunStageInfo.m_PhaseRunstageInfo[i].m_nConcurrencyPhase[0] == atPhaseTable[k].m_byPhaseNumber)
				{
					 for (m = 0;m < MAX_PHASE_CONCURRENCY_COUNT;m ++)
					 {
						 if (tRunStageInfo.m_PhaseRunstageInfo[i].m_nConcurrencyPhase[j] == atPhaseTable[k].m_byPhaseConcurrency[m])
						 {
							 bFlag = true;
							 break;
						 }
					 }	
				}
			}
		}

		if (tRunStageInfo.m_PhaseRunstageInfo[i].m_nConcurrencyPhaseCount == 1)
		{
		    bFlag = true;
		}

		if (!bFlag)
		{
			m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "CheckManualControlPattern GreenConflict");
			nFailCode = CONFIG_INCLUDE_GREENCONFLICT;
			return CONTROL_FAILED;
		}
	}

	atWorkModeParam.m_atManualControlPattern.m_nOffset = atWorkModeParam.m_atManualControlPattern.m_nOffset % atWorkModeParam.m_atManualControlPattern.m_nCycleTime;
   
	return CONTROL_SUCCEED;
}

int COpenATCCommWithCfgSWThread::CheckPhaseControl(int nPhaseID)
{
	TPhaseRunStatus tAscPhaseRunStatus;
	memset(&tAscPhaseRunStatus, 0x00, sizeof(tAscPhaseRunStatus));

	m_pOpenATCRunStatus->GetPhaseRunStatus(tAscPhaseRunStatus);

	bool bFlag = false;

	for (int i = 0;i < tAscPhaseRunStatus.m_nRingCount;i ++)
	{
		for (int j = 0;j < tAscPhaseRunStatus.m_atRingRunStatus[i].m_nPhaseCount;j ++)
		{
			if (tAscPhaseRunStatus.m_atRingRunStatus[i].m_atPhaseStatus[j].m_byPhaseID == nPhaseID)
			{
				bFlag = true;

				if (tAscPhaseRunStatus.m_atRingRunStatus[i].m_atPhaseStatus[j].m_chPhaseControlStatus == 1 ||
					tAscPhaseRunStatus.m_atRingRunStatus[i].m_atPhaseStatus[j].m_chPhaseControlStatus == 2)
				{
					return CONTROL_FAILED;
				}
			}
		}
	}

	if (!bFlag)
	{
		return CONTROL_FAILED;
	}

	return CONTROL_SUCCEED;
}

int COpenATCCommWithCfgSWThread::AckCtl_AskUpdateSecretKey()
{
	unsigned int dwSendBuffLen = 0;

	memset(m_chSendBuff, 0x00, SEND_BUFFER_SIZE);

	m_chSendBuff[dwSendBuffLen++] = CB_VERSION_FLAG;									//版本号
	m_chSendBuff[dwSendBuffLen++] = CB_ATC_FLAG;										//发送方标识
	m_chSendBuff[dwSendBuffLen++] = CB_CONFIGSOFTWARE_FLAG;								//接收方标识
	m_chSendBuff[dwSendBuffLen++] = LINK_CONTROL;							            //数据链路码
	m_chSendBuff[dwSendBuffLen++] = m_tAreaInfo.m_chAscRegionNo;						//区域号
	memcpy(m_chSendBuff + dwSendBuffLen, &m_tAreaInfo.m_usAscRoadNo, MAX_ROADNO_LENGTH);//路口号
	dwSendBuffLen += MAX_ROADNO_LENGTH;
	m_chSendBuff[dwSendBuffLen++] = ACK_QUERY;											//操作类型
	m_chSendBuff[dwSendBuffLen++] = CTL_UPDATESECRETKEY_STATUS;							//对象标识
	m_chSendBuff[dwSendBuffLen++] = 0x01;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留

	if (m_msgBody != NULL)
	{
		cJSON_Delete(m_msgBody);
		m_msgBody = NULL;
	}

	if (m_chOutBuff != NULL)
	{
		free(m_chOutBuff);
		m_chOutBuff = NULL;
	}

	m_msgBody = cJSON_CreateObject();
	cJSON_AddNumberToObject(m_msgBody, "success", 0);
	m_chOutBuff = cJSON_Print(m_msgBody);
	memcpy(m_chSendBuff + dwSendBuffLen, m_chOutBuff, strlen(m_chOutBuff));

	dwSendBuffLen += strlen(m_chOutBuff);

    return dwSendBuffLen;
}

int COpenATCCommWithCfgSWThread::AckCfg_SendCheckSecretKeyResult(int nErrorCode)
{
	unsigned int dwSendBuffLen = 0;

	memset(m_chSendBuff, 0x00, SEND_BUFFER_SIZE);

    m_chSendBuff[dwSendBuffLen++] = CB_VERSION_FLAG;									//版本号
	m_chSendBuff[dwSendBuffLen++] = CB_ATC_FLAG;										//发送方标识
	m_chSendBuff[dwSendBuffLen++] = CB_CONFIGSOFTWARE_FLAG;								//接收方标识
	m_chSendBuff[dwSendBuffLen++] = LINK_CONFIG;										//数据链路码
	m_chSendBuff[dwSendBuffLen++] = m_tAreaInfo.m_chAscRegionNo;						//区域号
	memcpy(m_chSendBuff + dwSendBuffLen, &m_tAreaInfo.m_usAscRoadNo, MAX_ROADNO_LENGTH);//路口号
    dwSendBuffLen += MAX_ROADNO_LENGTH;
	if (nErrorCode == CONTROL_SUCCEED)
	{
		m_chSendBuff[dwSendBuffLen++] = ACK_SET;										//操作类型
	}
	else
	{
		m_chSendBuff[dwSendBuffLen++] = ACK_WRONG;										//操作类型
	}
	m_chSendBuff[dwSendBuffLen++] = m_chUnPackedBuff[8];							    //对象标识
	m_chSendBuff[dwSendBuffLen++] = 0x01;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留

	if (m_msgBody != NULL)
	{
		cJSON_Delete(m_msgBody);
		m_msgBody = NULL;
	}

	if (m_chOutBuff != NULL)
	{
		free(m_chOutBuff);
		m_chOutBuff = NULL;
	}

	m_msgBody = cJSON_CreateObject();
	if (nErrorCode == CONTROL_SUCCEED)
	{
		cJSON_AddNumberToObject(m_msgBody, "success", 0);
	}
	else
	{
		cJSON_AddNumberToObject(m_msgBody, "success", nErrorCode);
	}
	m_chOutBuff = cJSON_Print(m_msgBody);
	memcpy(m_chSendBuff + dwSendBuffLen, m_chOutBuff, strlen(m_chOutBuff));

	dwSendBuffLen += strlen(m_chOutBuff);

	return dwSendBuffLen;
}

bool COpenATCCommWithCfgSWThread::CheckSecretKey(const unsigned char *chUnPackedBuff, unsigned int dwPackLength, char * chPeerIp)
{
	//#ifdef VIRTUAL_DEVICE
	//return true;
    //#endif

	unsigned char szRoadNo[MAX_ROADNO_LENGTH + 1];
	memset(szRoadNo, 0, MAX_ROADNO_LENGTH + 1);
	memcpy(szRoadNo, m_chUnPackedBuff + CMDCODE_POS - 3, MAX_ROADNO_LENGTH);//路口号

	int nRoadNo = 0;
    nRoadNo = (int)(szRoadNo[1] & 0x00ff | ((szRoadNo[0]) << 8) & 0xff00);

	if (nRoadNo != m_tAreaInfo.m_usAscRoadNo)
	{
		int nPackSize = AckCfg_SendCheckSecretKeyResult(ROAD_INDEX_NO_EXIST);
		SendAckToPeer(nPackSize);
		return false;
	}

	unsigned char szRoadNoInKeyFile[20];
	memset(szRoadNoInKeyFile, 0, 20);
	
	unsigned char szKeyFile[C_N_MAX_DEVICE_PARAM_BUFFER_SIZE];
	memset(szKeyFile, 0, C_N_MAX_DEVICE_PARAM_BUFFER_SIZE);

	char szSecretInKeyFile[C_N_SECRET_SIZE];  
	memset(szSecretInKeyFile, 0, C_N_SECRET_SIZE);

	int nPackSize = 0;

	if (chUnPackedBuff[CMDCODE_POS + 1] == ADD_SECRETKEY)
	{
		if (access(SECRETKEY_FILE_CONFIG_PATH, 0) == 0)  //检查文件目录
		{
			FILE *pf = fopen(SECRETKEY_FILE_CONFIG_PATH, "rb");
			if(!pf)
			{
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "COpenATCCommWithCfgSWThread Open SecretKey File Failed!");
				nPackSize = AckCfg_SendCheckSecretKeyResult(KEYFILE_NO_EXIST);
				SendAckToPeer(nPackSize);
				return false;
			}
			else
			{
				memset(szKeyFile, 0x00, sizeof(unsigned char) * C_N_MAX_DEVICE_PARAM_BUFFER_SIZE);
				int nKeyFileSize = fread(szKeyFile, 1, C_N_MAX_DEVICE_PARAM_BUFFER_SIZE, pf);
				fclose(pf);

				cJSON *allBodyJson = NULL;
				allBodyJson = cJSON_Parse((char*)szKeyFile);
				if (!allBodyJson)
				{
					m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "COpenATCCommWithCfgSWThread Parse SecretKey File Failed!");
				    nPackSize = AckCfg_SendCheckSecretKeyResult(KEYFILE_NO_EXIST);
					SendAckToPeer(nPackSize);
					return false;
				}

				/*cJSON* pClassItem = NULL;
				pClassItem = cJSON_GetObjectItem(allBodyJson, "id");
				if (pClassItem)
				{
					memcpy(szRoadNoInKeyFile, pClassItem->valuestring, 20);
					if (atoi((const char*)szRoadNoInKeyFile) != nRoadNo)
					{
						m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "COpenATCCommWithCfgSWThread Check RoadNo Failed!");
						nPackSize = AckCfg_SendCheckSecretKeyResult(ROAD_INDEX_NO_EXIST);
						SendAckToPeer(nPackSize);
						return false;
					}
				}*/

				cJSON* pClassItem = NULL;
				pClassItem = cJSON_GetObjectItem(allBodyJson, "secretkey");
				if (pClassItem)
				{
					memcpy(szSecretInKeyFile, pClassItem->valuestring, C_N_SECRET_SIZE);
				}
			}
		}
		else
		{
			m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "COpenATCCommWithCfgSWThread SecretKey File No Exist!");
			nPackSize = AckCfg_SendCheckSecretKeyResult(KEYFILE_NO_EXIST);
			SendAckToPeer(nPackSize);
			return false;
		}
	}
	else
	{
		m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "COpenATCCommWithCfgSWThread NonCenter Command!");
		nPackSize = AckCfg_SendCheckSecretKeyResult(INVALID_PROTOCOL);
		SendAckToPeer(nPackSize);
		return false;
	}

	if (dwPackLength < 16)
	{
		m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "COpenATCCommWithCfgSWThread PackLength Below 16!");
		nPackSize = AckCfg_SendCheckSecretKeyResult(CERTIFIED_FAILED);
		SendAckToPeer(nPackSize);
		return false;
	}

	memset(m_chSecretKeyBuff, 0x00, 100);
	memcpy(m_chSecretKeyBuff, m_chUnPackedBuff, dwPackLength - 16);
	COpenATCMD5 m_openMd5;
	char m_getMd5Value[C_N_MAX_MD5BUFF_SIZE] = { '\0' };
	memset(m_getMd5Value, 0x00, sizeof(char) * C_N_MAX_MD5BUFF_SIZE);
	memcpy(m_getMd5Value, m_chUnPackedBuff + dwPackLength - 16 , 16);
	char calMd5Value[C_N_MAX_MD5BUFFER_SIZE] = { '\0' };
	memset(calMd5Value, 0x00, sizeof(char) * C_N_MAX_MD5BUFFER_SIZE);
	memcpy(m_chSecretKeyBuff + dwPackLength - 16, szSecretInKeyFile, strlen(szSecretInKeyFile));
	m_chSecretKeyBuff[dwPackLength - 16 + strlen(szSecretInKeyFile)] = '\0';//封字符串
	m_openMd5.getMd5BaseLength(m_chSecretKeyBuff, dwPackLength - 16 + strlen(szSecretInKeyFile), calMd5Value);
	if (strcmp(calMd5Value, m_getMd5Value) == 0)
	{
		m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "COpenATCCommWithCfgSWThread Check SecretKey File Succeed!");
		//nPackSize = AckCfg_SendCheckSecretKeyResult(0);
		//SendAckToPeer(nPackSize);
		return true;
	}
	else
	{
		m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "COpenATCCommWithCfgSWThread Check SecretKey File Failed!");
		nPackSize = AckCfg_SendCheckSecretKeyResult(CERTIFIED_FAILED);
		SendAckToPeer(nPackSize);
		return false;
	}
	
}

void  COpenATCCommWithCfgSWThread::GetDeviceParam()
{
	long nCurTime = time(NULL);
	if (labs(nCurTime - m_lastGetDeviceParam) > COMM_ISPARAMCHG_TIME)
	{
		m_lastGetDeviceParam = nCurTime;

		m_pOpenATCParameter->GetAscAreaInfo(m_tAreaInfo);
	}
}