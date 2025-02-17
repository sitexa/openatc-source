/*====================================================================
模块名 ：信号机和上位机的通信模块
文件名 ：OpenaTCOpenATCComWithControlCenterImpl.cpp
相关文件：OpenaTCOpenATCComWithControlCenterImpl.h
实现功能：信号机和上位机的交互
作者 ：李永萍
版权 ：<Copyright(C) 2019-2020 Suzhou Keda Technology Co., Ltd. All rights reserved.>
---------------------------------------------------------------------------------------------------------------------
修改记录：
日 期           版本    修改人             走读人             修改记录
2019/09/06      V1.0    李永萍             李永萍             创建模块
====================================================================*/

#include "OpenATCComWithControlCenterImpl.h"
#include <time.h>

#include "../OpenATCFlowProcManager.h"
#include "OpenATCPackUnpackSimpleFactory.h"


COpenATCComWithControlCenterImpl::COpenATCComWithControlCenterImpl(COpenATCParameter *pParameter, COpenATCRunStatus *pRunStatus, COpenATCLog *pOpenATCLog, const char * pOpenATCVersion)
{
    m_pOpenATCParameter = pParameter;
    m_pOpenATCRunStatus = pRunStatus;
	m_pOpenATCLog       = pOpenATCLog;

	memset(m_chOpenATCVersion, 0, sizeof(m_chOpenATCVersion));
	memcpy(m_chOpenATCVersion, pOpenATCVersion, strlen(pOpenATCVersion));

	m_chRecvBuff = new unsigned char[RECV_BUFFER_SIZE];
	memset(m_chRecvBuff, 0, RECV_BUFFER_SIZE);
    m_chUnPackedBuff = new unsigned char[UNPACKED_BUFFER_SIZE];
	memset(m_chUnPackedBuff, 0x00, UNPACKED_BUFFER_SIZE);
    m_chSendBuff = new unsigned char[SEND_BUFFER_SIZE];
	memset(m_chSendBuff, 0x00, SEND_BUFFER_SIZE);
    m_chPackedBuff = new unsigned char[PACKED_BUFFER_SIZE];
	memset(m_chPackedBuff, 0x00, PACKED_BUFFER_SIZE);
	memset(m_achPastLampClr, 0x00, C_N_MAXLAMPOUTPUT_NUM);
	memset(m_achLampClrToSimulate, 0x00, C_N_MAXLAMPOUTPUT_NUM);

	m_clientSock.SetSocketType(SOCK_DGRAM);
    m_clientSock.SetComType(UDP_CLIENT);

    m_lastSendTime				= time(NULL);
    m_lastSendTrafficFlowTime	= time(NULL);
	m_lastGetCenterParam        = time(NULL);
   
    m_nSendTimeOut = 100;
	m_nRecvTimeOut = 100;

    m_bConnectStatus = false;
    m_bQueryStatus   = false;
   
    m_tAreaInfo.m_chAscRegionNo = 0;
    m_tAreaInfo.m_usAscRoadNo	= 0;
    m_pOpenATCParameter->GetAscAreaInfo(m_tAreaInfo);

    m_chWorkWay			= 0;
    m_chParamVersion	= 1;
   
    m_bSetSignalLightGroup	= false;
    m_bSetPhaseParam		= false;
    m_bSetPatternParam		= false;
    m_bSetPlanParam			= false;
    m_bSetWorkWay			= false;
    m_bSetVedetectorParam	= false;

    memset(m_nLastPhaseStatus, 0x00, sizeof(m_nLastPhaseStatus));
	m_nLastCurCtlMode = CTL_MODE_SELFCTL;

	m_pDataPackUnpackMode = COpenATCPackUnpackSimpleFactory::Create(PACK_UNPACK_MODE_COMM);

	m_msgBody   = NULL;
    m_chOutBuff = NULL;
	memset(&m_tOldVehDetBoardData,0,sizeof(m_tOldVehDetBoardData));
    memset(&m_tOldIOBoardData,0,sizeof(m_tOldIOBoardData));
}

COpenATCComWithControlCenterImpl::~COpenATCComWithControlCenterImpl()
{
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

	if (m_pDataPackUnpackMode != NULL)
	{
		delete m_pDataPackUnpackMode;
		m_pDataPackUnpackMode = NULL;
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
}

int COpenATCComWithControlCenterImpl::ConnectToCenter()
{
	char szInfo[255] = {0};
    int  nRet = OPENATC_RTN_OK;

    TAscCenter tAscCenter;
    m_pOpenATCParameter->GetCenterInfo(tAscCenter);

	TAscSimulate tSimulateInfo;
    m_pOpenATCParameter->GetSimulateInfo(tSimulateInfo);

	memcpy(&m_tOldAscCenter, &tAscCenter, sizeof(tAscCenter));

	m_clientSock.SetSocketType(SOCK_DGRAM);
    m_clientSock.SetComType(UDP_CLIENT);

	if (m_nComType == COM_WITH_CENTER)
    {
	    nRet = m_clientSock.Connect(tAscCenter.m_chAscCenterIp, tAscCenter.m_chAscAscCenterPort, strlen(tAscCenter.m_chAscCenterIp));
		if (nRet == OPENATC_RTN_OK)
        {
			m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "Connect to Center: %s %d Success", tAscCenter.m_chAscCenterIp, tAscCenter.m_chAscAscCenterPort);      
			SendAskOnlineQuery();
		}
		else
		{
			//m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "Connect to Center: %s %d Failed", tAscCenter.m_chAscCenterIp, tAscCenter.m_chAscAscCenterPort);      
		}
    }
    else
    {
        nRet = m_clientSock.Connect(tSimulateInfo.m_chSimulateIP, tSimulateInfo.m_nSimulatePort, strlen( tSimulateInfo.m_chSimulateIP));
		if (nRet == OPENATC_RTN_OK)
		{
			m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "Connect to Simulate: %s %d Success",  tSimulateInfo.m_chSimulateIP,  tSimulateInfo.m_nSimulatePort);
		}   
		else
		{
			m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "Connect to Simulate: %s %d Failed", tSimulateInfo.m_chSimulateIP, tSimulateInfo.m_nSimulatePort);      
		}
    }
  
	return nRet;
}

int COpenATCComWithControlCenterImpl::DisconnectToCenter()
{
	m_clientSock.Close();
   
    m_bConnectStatus = false;
    m_bQueryStatus   = false;

	return OPENATC_RTN_OK;
}

int COpenATCComWithControlCenterImpl::HandleEventFromCenter()
{
	int nRet = 0;
	unsigned int nPackLength = 0;

	int nRecvLength = m_clientSock.RecvFrom((char *)m_chRecvBuff, RECV_BUFFER_SIZE, m_nRecvTimeOut);
	if (nRecvLength > 0)
	{
		m_packerUnPacker.Write((unsigned char *)(char *)m_chRecvBuff, nRecvLength);
        nRet = m_packerUnPacker.Read(m_chUnPackedBuff, nPackLength);
        if (nRet == ReadOk)
        {
            ParserPack(m_chUnPackedBuff, nPackLength);
        }
        else if (nRet != ReadNoData)
        {
            m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "COpenATCComWithControlCenterImpl ParserPack fail!");
        }       
	}

	return OPENATC_RTN_OK;
}

int COpenATCComWithControlCenterImpl::SendEventToCenter()
{
    if (m_nComType == COM_WITH_SIMULATE)
    {
        TLampClrStatus tLampClrStatus;
	    m_pOpenATCRunStatus->GetLampClrStatus(tLampClrStatus);

		if (memcmp(m_achLampClrToSimulate, tLampClrStatus.m_achLampClr, sizeof(m_achLampClrToSimulate))!=0)
		{
            memcpy(m_achLampClrToSimulate, tLampClrStatus.m_achLampClr, sizeof(m_achPastLampClr));

	        int  i = 0;
            unsigned char chTempBuff[C_N_LAMPBORAD_OUTPUTNUM + 1] = {0};	//发送缓冲区
            unsigned char chSendBuff[42] = {0};
	        TCanData tCanData[2];//tCanData[0]对应第一块板卡的前六个端子，tCanData[1]对应第一块板卡的后六个端子
	        for (i = 0;i < 2;i++)
	        {
		        memset(&tCanData[i], 0x00, sizeof(TCanData));
	        }

            //int nTemp = ntohs(m_tAreaInfo.m_chAscRoadNo);
			memcpy(chSendBuff, &m_tAreaInfo.m_usAscRoadNo, MAX_ROADNO_LENGTH);//路口号
   
            for (i = 0;i < C_N_MAXLAMPBOARD_NUM;i++)
            {
		        memset(chTempBuff, 0x00, C_N_LAMPBORAD_OUTPUTNUM + 1);
                memcpy(chTempBuff, tLampClrStatus.m_achLampClr + i * C_N_LAMPBORAD_OUTPUTNUM, C_N_LAMPBORAD_OUTPUTNUM);
      
                GetCanData(chTempBuff, tCanData);

                memcpy(chSendBuff + 2 + i * 2, &tCanData[0].chCanData, 1);
                memcpy(chSendBuff + 2 + i * 2 + 1, &tCanData[1].chCanData, 1);

                //m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "GetCanData tCanData:0x%X 0x%X", tCanData[0].chCanData, tCanData[1].chCanData);
            }

            m_clientSock.SendTo((char *)chSendBuff, 2 + C_N_MAXLAMPBOARD_NUM * 2);
	    }
    }
	else
	{
		return OPENATC_RTN_FAILED;
	}

	return OPENATC_RTN_OK;
}

int COpenATCComWithControlCenterImpl::TimerJob(bool conStatus)
{
	bool bFlag = false;

	TLampClrStatus tLampClrStatus;
	memset(&tLampClrStatus, 0, sizeof(TLampClrStatus));
	m_pOpenATCRunStatus->GetLampClrStatus(tLampClrStatus);
	if (memcmp(m_achPastLampClr, tLampClrStatus.m_achLampClr, sizeof(m_achPastLampClr))!=0)
	{
		bFlag = true;
		if (m_nComType == COM_WITH_SIMULATE)
		{
			SendLampClrStatus();
		}
		memcpy(m_achPastLampClr, tLampClrStatus.m_achLampClr, sizeof(m_achPastLampClr));
	}
    if (conStatus == true)
    {
        long curTime = time(NULL);
		if (labs(curTime - m_lastSendTime) >= ASK_ONLINEREQUST_TIME)
		{
			m_lastSendTime = curTime; 
			SendAskOnlineQuery();
			if (m_bConnectStatus == true)
			{
				SendFault();
				SendDetectorStatusData();
			}
		}
        if (labs(curTime - m_lastSendTrafficFlowTime) >= 300)
        {
            m_lastSendTrafficFlowTime = curTime; 
            if (m_bConnectStatus == true)
            {
                SendTrafficFlow();     
            } 
        }

		if (m_bConnectStatus == true)
		{
			SendDetectorChgData();
		}

        if (bFlag)
        {
            if (m_bConnectStatus == true)
            {
                #if 0
                SendAskOnlineRequest();
                SendLampClrStatus();
                SendVersion();
                #endif

                //SendAskOnlineQuery();

                m_pOpenATCRunStatus->SetPhaseRunStatusWriteFlag(true);
                OpenATCSleep(100);
                
                if (m_pOpenATCRunStatus->GetPhaseRunStatusReadFlag())
                {
                    m_pOpenATCRunStatus->SetPhaseRunStatusReadFlag(false);
                    m_pOpenATCRunStatus->SetPhaseRunStatusWriteFlag(false);
                    SendWorkStatus();
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
                        SendWorkStatus();
                        m_pOpenATCRunStatus->SetPhaseRunStatusReadFlag(true);
                        m_pOpenATCRunStatus->SetPhaseRunStatusWriteFlag(true);
                    }
                }

                //SendFault();
            }
            else
            {
                //SendAskOnlineQuery();
            }
        }
    }

	return OPENATC_RTN_OK;
}

int COpenATCComWithControlCenterImpl::ParserPack(const unsigned char *chUnPackedBuff, unsigned int dwPackLength)
{
    int nRet = OPENATC_RTN_OK;

    switch (chUnPackedBuff[LINKCODE_POS])
    {
	case LINK_COM:
		{
			nRet = ParserPack_ComLink(chUnPackedBuff, dwPackLength);
		}
		break;
	case LINK_CONTROL:
		{
			nRet = ParserPack_CtrlLink(chUnPackedBuff, dwPackLength);
		}
		break;
	case LINK_CONFIG:
		{
			nRet = ParserPack_CfgLink(chUnPackedBuff, dwPackLength);
		}
		break;
	case LINK_BASEINFO:
		{
			nRet = ParserPack_BaseInfoLink(chUnPackedBuff, dwPackLength);
		}
		break;
	case LINK_ALTERNATEFEATUREPARA:
		{
			nRet = ParserPack_AlternateFeatureParaLink(chUnPackedBuff, dwPackLength);
		}
		break;
	case LINK_INTERVENECOMMAND:
		{
			nRet = ParserPack_InterveneCommandLink(chUnPackedBuff, dwPackLength);
		}
		break;
	default:
		break;
    }
        
    return nRet;
}

int COpenATCComWithControlCenterImpl::ParserPack_ComLink(const unsigned char *chUnPackedBuff, unsigned int dwPackLength)
{
	int nRet = OPENATC_RTN_OK;
    int nPackSize = 0;

    if (chUnPackedBuff[CMDCODE_POS] == CTL_ONLINEMACHINE)
    {
		//联机请求询应答
        if (chUnPackedBuff[CMDCODE_POS - 1] == ACK_SET)
		{
			m_pOpenATCLog->LogOneMessage(LEVEL_INFO, "ParserPack_ComLink receive inline query ack");
            m_bConnectStatus = true;
		}
		//联机查询应答
		else if (chUnPackedBuff[CMDCODE_POS - 1] == ACK_QUERY)
		{
			m_pOpenATCLog->LogOneMessage(LEVEL_INFO, "ParserPack_ComLink receive inline ask ack");
            m_bQueryStatus = true;
		}
    }

	return nRet;
}

int COpenATCComWithControlCenterImpl::ParserPack_CtrlLink(const unsigned char *chUnPackedBuff, unsigned int dwPackLength)
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
			//		
		}
		break; 
	//请求更新文件
	case CTL_ASK_UPDATEFILE:
		{
			//	
		}
		break;
	//开始更新文件
	case CTL_ASK_SENDFILEBLOCK:
		{
			//	
		}
		break;
	//取消文件发送
	case CTL_ASK_CANCELSENDFILE:
		{
			//
		}
		break;
	//文件发送完毕
	case CTL_ASK_UPDATEONE:
		{
			//	
		}
		break;
	//文件发送完毕确认
	case CTL_ACKED_SENDFILEEND:
		{
			//	 
		}
        break;
	//开始更新文件
	case CTL_ASK_STARTUPDATE:
		{
			//	
		}
		break;
	//请求重启
	case CTL_ASK_REBOOT:
		{
			//
		}
		break;
	default:
		break;
    }

    return nRet;
}

int COpenATCComWithControlCenterImpl::ParserPack_CfgLink(const unsigned char *chUnPackedBuff, unsigned int dwPackLength)
{
	int nRet = OPENATC_RTN_OK;
    int nPackSize = 0;
	int nSendDataSize = 0;

    switch (chUnPackedBuff[CMDCODE_POS])
    {
	//中心软件请求往主机下传数据
    case CFG_ASK_ASKSEND:
        {
			//
        }
        break;
	//中心软件往主机下传数据
	case CFG_ASK_SENDDATA:
        {
			//
        }
        break;
	//主机上传数据到中心软件
	case CFG_ASK_ASKREAD:
        {
			//
        }
        break;
    case ASK_QUERY:
        {
            //
        }
    default:
        break;
    }

    return nRet; 
}

int COpenATCComWithControlCenterImpl::ParserPack_BaseInfoLink(const unsigned char *chUnPackedBuff, unsigned int dwPackLength)
{
	int nRet = OPENATC_RTN_OK;

	switch (chUnPackedBuff[CMDCODE_POS])
    {
    case CTL_WORKSTATUS:
        {
			//信号机工作状态查询
            if (chUnPackedBuff[CMDCODE_POS - 1] == ASK_QUERY)
			{
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "ParserPack_BaseInfoLink receive work status query");

                m_pOpenATCRunStatus->SetPhaseRunStatusWriteFlag(true);
                OpenATCSleep(100);

                if (m_pOpenATCRunStatus->GetPhaseRunStatusReadFlag())
                {
                    m_pOpenATCRunStatus->SetPhaseRunStatusReadFlag(false);
                    m_pOpenATCRunStatus->SetPhaseRunStatusWriteFlag(false);
                    nRet = AckCtl_AskWorkStatus();
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
                        nRet = AckCtl_AskWorkStatus();
                        m_pOpenATCRunStatus->SetPhaseRunStatusReadFlag(true);
                        m_pOpenATCRunStatus->SetPhaseRunStatusWriteFlag(true);
                    }
                }	
			}
        }
        break;
	 case CTL_LAMPCOLORSTATUS:
        {
			//灯色状态查询
            if (chUnPackedBuff[CMDCODE_POS - 1] == ASK_QUERY)
			{
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "ParserPack_BaseInfoLink receive light color status query");
				nRet = AckCtl_AskLampClrStatus();
			}
        }
        break;
	case CTL_CURRENTTIME:
        {
			//时间查询
            if (chUnPackedBuff[CMDCODE_POS - 1] == ASK_QUERY)
			{
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "ParserPack_BaseInfoLink receive time query");
				nRet = AckCtl_AskQueryATCLocalTime();
			}
			//时间设置命令
			else if (chUnPackedBuff[CMDCODE_POS - 1] == ASK_SET)
			{
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "ParserPack_BaseInfoLink receive time set command");
				//设置口令
                if (chUnPackedBuff[CMDCODE_POS + 1] == 0x31 && chUnPackedBuff[CMDCODE_POS + 2] == 0x30 && chUnPackedBuff[CMDCODE_POS + 3] == 0x30 &&
                    chUnPackedBuff[CMDCODE_POS + 4] == 0x30 && chUnPackedBuff[CMDCODE_POS + 5] == 0x30)
                {
                    long nTime = 0;
                    memcpy(&nTime, chUnPackedBuff + CMDCODE_POS + 6, 4);
                    SetSysTime(nTime);

                    nRet = AckCtl_AskSetATCLocalTime(false);
                }
                else
                {
                    nRet = AckCtl_AskSetATCLocalTime(true);
                }
			}
        }
        break;
	case CTL_FAULT:
        {
			//信号机故障查询
            if (chUnPackedBuff[CMDCODE_POS - 1] == ASK_QUERY)
			{
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "ParserPack_BaseInfoLink receive fault query");
				nRet = AckCtl_AskQueryFault();
			}
        }

        break;
	case CTL_VERSION:
        {
			//信号机版本查询
            if (chUnPackedBuff[CMDCODE_POS - 1] == ASK_QUERY)
			{
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "ParserPack_BaseInfoLink receive version query");
				nRet = AckCtl_AskQueryVersion();
			}
        }
        break;
	default:
		break;
	}

    return nRet; 
}

int COpenATCComWithControlCenterImpl::ParserPack_AlternateFeatureParaLink(const unsigned char *chUnPackedBuff, unsigned int dwPackLength)
{
	int nRet = OPENATC_RTN_OK;
  
	switch (chUnPackedBuff[CMDCODE_POS])
    {
    case CTL_SIGNALLIGHTGROUP:
        {
			//信号灯组查询
            if (chUnPackedBuff[CMDCODE_POS - 1] == ASK_QUERY)
			{
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "ParserPack_BaseInfoLink receive signal light group query");
				nRet = AckCtl_AskQuerySignalLightGroup();
			}
			//信号灯组设置命令
			else if (chUnPackedBuff[CMDCODE_POS - 1] == ASK_SET)
			{
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "ParserPack_BaseInfoLink receive signal light group set command");
				//设置口令
                if (chUnPackedBuff[CMDCODE_POS + 1] == 0x31 && chUnPackedBuff[CMDCODE_POS + 2] == 0x30 && chUnPackedBuff[CMDCODE_POS + 3] == 0x30 &&
                    chUnPackedBuff[CMDCODE_POS + 4] == 0x30 && chUnPackedBuff[CMDCODE_POS + 5] == 0x30)
                {
                    m_bSetSignalLightGroup = true;
                    memset(m_chSignalLightGroupBuffer, 0x00, sizeof(m_chSignalLightGroupBuffer));
             
                    m_chSignalLightGroupBuffer[0] = chUnPackedBuff[CMDCODE_POS + 6];  
                    memcpy(m_chSignalLightGroupBuffer + 1, chUnPackedBuff + CMDCODE_POS + 7,  m_chSignalLightGroupBuffer[0] * 12);

			        nRet = AckCtl_AskSetSignalLightGroup(false);    
                }
                else
                {
                    nRet = AckCtl_AskSetSignalLightGroup(true);       
                }
            }
        }
        break;
	 case CTL_PHASE:
        {
			//相位查询
            if (chUnPackedBuff[CMDCODE_POS - 1] == ASK_QUERY)
			{
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "ParserPack_BaseInfoLink receive phase query");

                m_pOpenATCRunStatus->SetPhaseRunStatusWriteFlag(true);
                OpenATCSleep(100);
                
                if (m_pOpenATCRunStatus->GetPhaseRunStatusReadFlag())
                {
                    m_pOpenATCRunStatus->SetPhaseRunStatusReadFlag(false);
                    m_pOpenATCRunStatus->SetPhaseRunStatusWriteFlag(false);
                    nRet = AckCtl_AskQueryPhaseParamData();
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
                        nRet = AckCtl_AskQueryPhaseParamData();
                        m_pOpenATCRunStatus->SetPhaseRunStatusReadFlag(true);
                        m_pOpenATCRunStatus->SetPhaseRunStatusWriteFlag(true);
                    }
				}
			}
			//相位设置命令
			else if (chUnPackedBuff[CMDCODE_POS - 1] == ASK_SET)
			{
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "ParserPack_BaseInfoLink receive phase set command");
				//设置口令
                if (chUnPackedBuff[CMDCODE_POS + 1] == 0x31 && chUnPackedBuff[CMDCODE_POS + 2] == 0x30 && chUnPackedBuff[CMDCODE_POS + 3] == 0x30 &&
                    chUnPackedBuff[CMDCODE_POS + 4] == 0x30 && chUnPackedBuff[CMDCODE_POS + 5] == 0x30)
                {
                    m_bSetPhaseParam = true;
                    memset(m_chPhaseParamBuffer, 0x00, sizeof(m_chPhaseParamBuffer));
             
                    m_chPhaseParamBuffer[0] = chUnPackedBuff[CMDCODE_POS + 6];  
                    memcpy(m_chPhaseParamBuffer + 1, chUnPackedBuff + CMDCODE_POS + 7,  m_chPhaseParamBuffer[0] * 24);

                    nRet = AckCtl_AskSetPhaseParamData(false);    
                }
                else
                {
                    nRet = AckCtl_AskSetPhaseParamData(true);    
                }
			}
        }
        break;
	case CTL_SIGNALMATCHTIME:
        {
			//信号配时方案查询
            if (chUnPackedBuff[CMDCODE_POS - 1] == ASK_QUERY)
			{
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "ParserPack_BaseInfoLink receive signal match time pattern query");
				nRet = AckCtl_AskQueryPatternParamData();
			}
			//信号配时方案设置命令
			else if (chUnPackedBuff[CMDCODE_POS - 1] == ASK_SET)
			{
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "ParserPack_BaseInfoLink receive signal match time pattern set command");
				//设置口令
                if (chUnPackedBuff[CMDCODE_POS + 1] == 0x31 && chUnPackedBuff[CMDCODE_POS + 2] == 0x30 && chUnPackedBuff[CMDCODE_POS + 3] == 0x30 &&
                    chUnPackedBuff[CMDCODE_POS + 4] == 0x30 && chUnPackedBuff[CMDCODE_POS + 5] == 0x30)
                {
                    m_bSetPatternParam = true;
                    memset(m_chPatternParamBuffer, 0x00, sizeof(m_chPatternParamBuffer));
             
                    m_chPatternParamBuffer[0] = chUnPackedBuff[CMDCODE_POS + 6];  
                    memcpy(m_chPatternParamBuffer + 1, chUnPackedBuff + CMDCODE_POS + 7,  m_chPatternParamBuffer[0] * 24);

                    nRet = AckCtl_AskSetPatternParamData(false);    
                }
                else
                {
                    nRet = AckCtl_AskSetPatternParamData(true);       
                }
			}
        }
        break;
	case CTL_PROGRAMMESCHEDULEPLAN:
        {
			//方案调度计划查询
            if (chUnPackedBuff[CMDCODE_POS - 1] == ASK_QUERY)
			{
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "ParserPack_BaseInfoLink receive programme time base schedule plan query");
				nRet = AckCtl_AskQueryPlanParamData();
			}
			//方案调度计划设置命令
			else if (chUnPackedBuff[CMDCODE_POS - 1] == ASK_SET)
			{
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "ParserPack_BaseInfoLink receive programme time base schedule plan set command");
				//设置口令
                if (chUnPackedBuff[CMDCODE_POS + 1] == 0x31 && chUnPackedBuff[CMDCODE_POS + 2] == 0x30 && chUnPackedBuff[CMDCODE_POS + 3] == 0x30 &&
                    chUnPackedBuff[CMDCODE_POS + 4] == 0x30 && chUnPackedBuff[CMDCODE_POS + 5] == 0x30)
                {
                    m_bSetPlanParam = true;
                    memset(m_chPlanParamBuffer, 0x00, sizeof(m_chPlanParamBuffer));
             
                    m_chPlanParamBuffer[0] = chUnPackedBuff[CMDCODE_POS + 6];  
                    memcpy(m_chPlanParamBuffer + 1, chUnPackedBuff + CMDCODE_POS + 7,  m_chPlanParamBuffer[0] * 12);

                    nRet = AckCtl_AskSetPlanParamData(true);      
                }
                else
                {
                    nRet = AckCtl_AskSetPlanParamData(true);       
                }
			}
        }
        break;
	default:
		break;
	}

    return nRet;
}

int COpenATCComWithControlCenterImpl::ParserPack_InterveneCommandLink(const unsigned char *chUnPackedBuff, unsigned int dwPackLength)
{
	int nRet = OPENATC_RTN_OK;
 
	switch (chUnPackedBuff[CMDCODE_POS])
    {
    case CTL_WORKWAY:
        {
			//工作方式查询
            if (chUnPackedBuff[CMDCODE_POS - 1] == ASK_QUERY)
			{
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "ParserPack_BaseInfoLink receive work way query");
              
                nRet = AckCtl_AskQueryWorkWay();
			}
			//工作方式设置命令
			else if (chUnPackedBuff[CMDCODE_POS - 1] == ASK_SET)
			{
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "ParserPack_BaseInfoLink receive work way set command");
				//设置口令
                if (chUnPackedBuff[CMDCODE_POS + 1] == 0x31 && chUnPackedBuff[CMDCODE_POS + 2] == 0x30 && chUnPackedBuff[CMDCODE_POS + 3] == 0x30 &&
                    chUnPackedBuff[CMDCODE_POS + 4] == 0x30 && chUnPackedBuff[CMDCODE_POS + 5] == 0x30)
                {
                    m_bSetWorkWay = true;
					//工作方式
                    m_chWorkWay = chUnPackedBuff[CMDCODE_POS + 6];

                    nRet = AckCtl_AskSetWorkWay(false);    
                }
                else
                {
                    nRet = AckCtl_AskSetWorkWay(true);       
                } 
			}
        }
        break;
	 case CTL_FEATUREPARAVERSION:
        {
			//特征参数版本查询
            if (chUnPackedBuff[CMDCODE_POS - 1] == ASK_QUERY)
			{
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "ParserPack_BaseInfoLink receive feaure param query");
				nRet = AckCtl_AskQueryATCParamVersion();
			}
			//特征参数版本设置命令
			else if (chUnPackedBuff[CMDCODE_POS - 1] == ASK_SET)
			{
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "ParserPack_BaseInfoLink receive feaure param set command");
				//设置口令
                if (chUnPackedBuff[CMDCODE_POS + 1] == 0x31 && chUnPackedBuff[CMDCODE_POS + 2] == 0x30 && chUnPackedBuff[CMDCODE_POS + 3] == 0x30 &&
                    chUnPackedBuff[CMDCODE_POS + 4] == 0x30 && chUnPackedBuff[CMDCODE_POS + 5] == 0x30)
                {
					//特征参数版本
                    m_chParamVersion = chUnPackedBuff[CMDCODE_POS + 6];

                    nRet = AckCtl_AskSetATCParamVersion(false);       
                }
                else
                {
                    nRet = AckCtl_AskSetATCParamVersion(true);     
                } 
			}
        }
        break;
	case CTL_INDENTIFYCODE:
        {
			//信号机识别码查询
            if (chUnPackedBuff[CMDCODE_POS - 1] == ASK_QUERY)
			{
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "ParserPack_BaseInfoLink receive identify code query");
				nRet = AckCtl_AskQueryATCCode();
			}
        }
        break;
	case CTL_REMOTECONTROL:
        {
			//远程控制设置命令
            if (chUnPackedBuff[CMDCODE_POS - 1] == ASK_SET)
			{
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "ParserPack_BaseInfoLink receive remote control set command");
                //设置口令
                if (chUnPackedBuff[CMDCODE_POS + 1] == 0x31 && chUnPackedBuff[CMDCODE_POS + 2] == 0x30 && chUnPackedBuff[CMDCODE_POS + 3] == 0x30 &&
                    chUnPackedBuff[CMDCODE_POS + 4] == 0x30 && chUnPackedBuff[CMDCODE_POS + 5] == 0x30)
                {
                    nRet = AckCtl_AskRemoteControl(false);
					//重启指令，临时写死
                    if (chUnPackedBuff[CMDCODE_POS + 6] == 0x01)
                    {
                        m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "ParserPack_BaseInfoLink receive reboot command");
                        m_pOpenATCRunStatus->SetRebootStatus(true);
                    }
					//手动授权，临时写死
                    else if (chUnPackedBuff[CMDCODE_POS + 6] == 0x02)
                    {
                        m_pOpenATCLog->LogOneMessage(LEVEL_INFO, "ParserPack_BaseInfoLink receive manual authorization command");
                    }
					//用户自定义指令，临时写死
                    else if (chUnPackedBuff[CMDCODE_POS + 6] == 0x03)
                    {
                        m_pOpenATCLog->LogOneMessage(LEVEL_INFO, "ParserPack_BaseInfoLink receive user customization command");
                    }     
                }
                else
                {
                     nRet = AckCtl_AskRemoteControl(true);  
                }  
			}
        }
        break;
	case CTL_DETECTOR:
        {
			//检测器查询命令
            if (chUnPackedBuff[CMDCODE_POS - 1] == ASK_QUERY)
			{
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "ParserPack_BaseInfoLink receive detector query");
			    nRet = AckCtl_AskQueryVecDetectorParamData();
			}
			//检测器设置命令
			else if (chUnPackedBuff[CMDCODE_POS - 1] == ASK_SET)
			{
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "ParserPack_BaseInfoLink receive detector set command");
				//设置口令
                if (chUnPackedBuff[CMDCODE_POS + 1] == 0x31 && chUnPackedBuff[CMDCODE_POS + 2] == 0x30 && chUnPackedBuff[CMDCODE_POS + 3] == 0x30 &&
                    chUnPackedBuff[CMDCODE_POS + 4] == 0x30 && chUnPackedBuff[CMDCODE_POS + 5] == 0x30)
                {
                    m_bSetVedetectorParam = true;
                    memset(m_chVecDetectorParamBuffer, 0x00, sizeof(m_chVecDetectorParamBuffer));

                    m_chVecDetectorParamBuffer[0] = chUnPackedBuff[CMDCODE_POS + 6];
                    memcpy(m_chVecDetectorParamBuffer + 1, chUnPackedBuff + CMDCODE_POS + 7,  m_chVecDetectorParamBuffer[0] * 15);

                    nRet = AckCtl_AskSetVecDetectorParamData(false);    
                }
                else
                {
                    nRet = AckCtl_AskSetVecDetectorParamData(true);      
                }
			}
        }
        break;
	default:
		break;
	}

    return nRet;
}

int COpenATCComWithControlCenterImpl::SendAskOnlineRequest(void)
{
    unsigned int  dwBufLen = 0; 
	unsigned int  dwPackedBuffSize = 0;

    memset(m_chSendBuff, 0x00, SEND_BUFFER_SIZE);

	m_chSendBuff[dwBufLen++] = CB_VERSION_FLAG;										//版本号
	m_chSendBuff[dwBufLen++] = CB_ATC_FLAG;											//发送方标识
	m_chSendBuff[dwBufLen++] = CB_CONFIGSOFTWARE_FLAG;								//接收方标识
	m_chSendBuff[dwBufLen++] = LINK_COM;											//数据链路码
	m_chSendBuff[dwBufLen++] = m_tAreaInfo.m_chAscRegionNo;							//区域号
	memcpy(m_chSendBuff + dwBufLen, &m_tAreaInfo.m_usAscRoadNo, MAX_ROADNO_LENGTH);	//路口号
    dwBufLen += MAX_ROADNO_LENGTH;
	m_chSendBuff[dwBufLen++] = ASK_QUERY;											//操作类型
	m_chSendBuff[dwBufLen++] = CTL_ONLINEMACHINE;									//对象标识
	m_chSendBuff[dwBufLen++] = 0x01;												//保留
	m_chSendBuff[dwBufLen++] = 0x00;												//保留
	m_chSendBuff[dwBufLen++] = 0x00;												//保留
	m_chSendBuff[dwBufLen++] = 0x00;												//保留
	m_chSendBuff[dwBufLen++] = 0x00;												//保留

	m_pDataPackUnpackMode->PackBuffer(m_chSendBuff, dwBufLen, m_chPackedBuff, dwPackedBuffSize);

	return m_clientSock.SendTo((char *)m_chPackedBuff, dwPackedBuffSize);
}

int COpenATCComWithControlCenterImpl::SendAskOnlineQuery(void)
{
    unsigned int  dwBufLen = 0; 
	unsigned int  dwPackedBuffSize = 0;

    memset(m_chSendBuff, 0x00, SEND_BUFFER_SIZE);

	m_chSendBuff[dwBufLen++] = CB_VERSION_FLAG;										//版本号
	m_chSendBuff[dwBufLen++] = CB_ATC_FLAG;											//发送方标识
	m_chSendBuff[dwBufLen++] = CB_CONFIGSOFTWARE_FLAG;								//接收方标识
	m_chSendBuff[dwBufLen++] = LINK_COM;											//数据链路码
	m_chSendBuff[dwBufLen++] = m_tAreaInfo.m_chAscRegionNo;							//区域号
	memcpy(m_chSendBuff + dwBufLen, &m_tAreaInfo.m_usAscRoadNo, MAX_ROADNO_LENGTH);	//路口号
    dwBufLen += MAX_ROADNO_LENGTH;
	m_chSendBuff[dwBufLen++] = ASK_SET;												//操作类型
	m_chSendBuff[dwBufLen++] = CTL_ONLINEMACHINE;									//对象标识
	m_chSendBuff[dwBufLen++] = 0x01;												//保留
	m_chSendBuff[dwBufLen++] = 0x00;												//保留
	m_chSendBuff[dwBufLen++] = 0x00;												//保留
	m_chSendBuff[dwBufLen++] = 0x00;												//保留
	m_chSendBuff[dwBufLen++] = 0x00;												//保留

    unsigned char *pBuffer = m_pOpenATCParameter->GetAscLoginCenterData();
    if (pBuffer != NULL)
    {    
        memcpy(m_chSendBuff + dwBufLen, pBuffer, strlen((const char *)pBuffer));
        dwBufLen += strlen((const char *)pBuffer);
    }
    
	m_pDataPackUnpackMode->PackBuffer(m_chSendBuff, dwBufLen, m_chPackedBuff, dwPackedBuffSize);

    if (pBuffer != NULL)
    {
        free(pBuffer);
        pBuffer = NULL;
    }

	return m_clientSock.SendTo((char *)m_chPackedBuff, dwPackedBuffSize);
}

int COpenATCComWithControlCenterImpl::SendTrafficFlow()
{
    unsigned int  dwBufLen = 0; 
	unsigned int  dwPackedBuffSize = 0;

    memset(m_chSendBuff, 0x00, SEND_BUFFER_SIZE);

	m_chSendBuff[dwBufLen++] = CB_VERSION_FLAG;										//版本号
	m_chSendBuff[dwBufLen++] = CB_ATC_FLAG;											//发送方标识
	m_chSendBuff[dwBufLen++] = CB_CONFIGSOFTWARE_FLAG;								//接收方标识
	m_chSendBuff[dwBufLen++] = LINK_BASEINFO;										//数据链路码
	m_chSendBuff[dwBufLen++] = m_tAreaInfo.m_chAscRegionNo;							//区域号
	memcpy(m_chSendBuff + dwBufLen, &m_tAreaInfo.m_usAscRoadNo, MAX_ROADNO_LENGTH);	//路口号
    dwBufLen += MAX_ROADNO_LENGTH;
	m_chSendBuff[dwBufLen++] = REPORT_STATUS;										//操作类型
	m_chSendBuff[dwBufLen++] = CTL_TRAFFICFLOWINFO;									//对象标识
	m_chSendBuff[dwBufLen++] = 0x01;												//保留
	m_chSendBuff[dwBufLen++] = 0x00;												//保留
	m_chSendBuff[dwBufLen++] = 0x00;												//保留
	m_chSendBuff[dwBufLen++] = 0x00;												//保留
	m_chSendBuff[dwBufLen++] = 0x00;												//保留

	unsigned char *pBuffer = m_pOpenATCParameter->GetCurrentTrafficFlowData(&COpenATCFlowProcManager::getInstance()->GetCurrentStatisticVehDetData());
    if (pBuffer != NULL)
    {    
        memcpy(m_chSendBuff + dwBufLen, pBuffer, strlen((const char *)pBuffer));
        dwBufLen += strlen((const char *)pBuffer);
    }

#if 0
    TStatisticVehDetData statisticVehDetData = COpenATCFlowProcManager::getInstance()->GetCurrentStatisticVehDetData();

	m_chSendBuff[dwBufLen++] = (char)statisticVehDetData.m_nDetNum;					//检测器数量

    for (int i = 0; i < statisticVehDetData.m_nDetNum; i++)
    {
        memcpy(m_chSendBuff + dwBufLen, &statisticVehDetData.m_atDetFlowInfo[i].m_nLargeVehNum, 2);
        dwBufLen += 2;

        memcpy(m_chSendBuff + dwBufLen, &statisticVehDetData.m_atDetFlowInfo[i].m_nMiddleVehNum, 2);
        dwBufLen += 2;

        memcpy(m_chSendBuff + dwBufLen, &statisticVehDetData.m_atDetFlowInfo[i].m_nSmallVehNum, 2);
        dwBufLen += 2;
    }
#endif

	m_pDataPackUnpackMode->PackBuffer(m_chSendBuff, dwBufLen, m_chPackedBuff, dwPackedBuffSize);

    if (pBuffer != NULL)
    {
        free(pBuffer);
        pBuffer = NULL;
    }

	return m_clientSock.SendTo((char *)m_chPackedBuff, dwPackedBuffSize);
}

int COpenATCComWithControlCenterImpl::SendWorkStatus()
{
    TPhaseRunStatus tAscPhaseRunStatus;
    memset(&tAscPhaseRunStatus, 0x00, sizeof(tAscPhaseRunStatus));

    m_pOpenATCRunStatus->GetPhaseRunStatus(tAscPhaseRunStatus);

    bool bChg = false;
    int  i = 0, j = 0;
    int  nPhaseIndex = 0;
    int  nLastPhaseStatus[MAX_RING_COUNT * MAX_SEQUENCE_TABLE_COUNT];
    memset(nLastPhaseStatus, 0x00, sizeof(nLastPhaseStatus));

	if (m_nLastCurCtlMode != tAscPhaseRunStatus.m_nCurCtlMode)
	{
		bChg = true;
		m_nLastCurCtlMode = tAscPhaseRunStatus.m_nCurCtlMode;
	}
   
    for (i = 0;i < tAscPhaseRunStatus.m_nRingCount;i++)
    {
        for (j = 0;j < tAscPhaseRunStatus.m_atRingRunStatus[i].m_nPhaseCount;j++)
        {
            int nPhaseIndex = i * tAscPhaseRunStatus.m_atRingRunStatus[i].m_nPhaseCount + j;
            nLastPhaseStatus[nPhaseIndex] = tAscPhaseRunStatus.m_atRingRunStatus[i].m_atPhaseStatus[j].m_chPhaseStatus;

            if (nLastPhaseStatus[nPhaseIndex] != m_nLastPhaseStatus[nPhaseIndex])
            {
                bChg = true;
            }
                        
            m_nLastPhaseStatus[nPhaseIndex] = nLastPhaseStatus[nPhaseIndex];           
        }
    }

    if (!bChg)
    {
        return true;    
    }
	
    unsigned int  dwBufLen = 0; 
	unsigned int  dwPackedBuffSize = 0;

    memset(m_chSendBuff, 0x00, SEND_BUFFER_SIZE);

	m_chSendBuff[dwBufLen++] = CB_VERSION_FLAG;										//版本号
	m_chSendBuff[dwBufLen++] = CB_ATC_FLAG;											//发送方标识
	m_chSendBuff[dwBufLen++] = CB_CONFIGSOFTWARE_FLAG;								//接收方标识
	m_chSendBuff[dwBufLen++] = LINK_BASEINFO;										//数据链路码
	m_chSendBuff[dwBufLen++] = m_tAreaInfo.m_chAscRegionNo;							//区域号
	memcpy(m_chSendBuff + dwBufLen, &m_tAreaInfo.m_usAscRoadNo, MAX_ROADNO_LENGTH);	//路口号
    dwBufLen += MAX_ROADNO_LENGTH;
	m_chSendBuff[dwBufLen++] = REPORT_STATUS;										//操作类型
	m_chSendBuff[dwBufLen++] = CTL_WORKSTATUS;										//对象标识
	m_chSendBuff[dwBufLen++] = 0x01;												//保留
	m_chSendBuff[dwBufLen++] = 0x00;												//保留
	m_chSendBuff[dwBufLen++] = 0x00;												//保留
	m_chSendBuff[dwBufLen++] = 0x00;												//保留
	m_chSendBuff[dwBufLen++] = 0x00;												//保留

    unsigned char *pBuffer = m_pOpenATCParameter->GetPatternRunStatusData();
    if (pBuffer != NULL)
    {    
        memcpy(m_chSendBuff + dwBufLen, pBuffer, strlen((const char *)pBuffer));
        dwBufLen += strlen((const char *)pBuffer);
    }

#if 0
    TPhaseRunStatus m_tAscPhaseRunStatus;
    memset(&m_tAscPhaseRunStatus, 0x00, sizeof(TPhaseRunStatus));

    m_pOpenATCRunStatus->GetPhaseRunStatus(m_tAscPhaseRunStatus);

    unsigned char workStatus[7] = {0};
    workStatus[0] = m_tAscPhaseRunStatus.m_nCurCtlPattern;							//控制模式
    workStatus[1] = m_tAscPhaseRunStatus.m_nCurCtlMode;								//控制方式
    workStatus[2] = m_tAscPhaseRunStatus.m_byPlanID;								//方案编号
    workStatus[3] = m_tAscPhaseRunStatus.m_atRingRunStatus[0].m_nCurRunPhaseIndex + 1;//当前相位
    workStatus[4] = m_tAscPhaseRunStatus.m_atRingRunStatus[0].m_atPhaseStatus[m_tAscPhaseRunStatus.m_atRingRunStatus[0].m_nCurRunPhaseIndex].m_chPhaseStatus;//当前相位状态

    memcpy(m_chSendBuff + dwBufLen, workStatus, 6);
    dwBufLen += 6;
#endif

	m_pDataPackUnpackMode->PackBuffer(m_chSendBuff, dwBufLen, m_chPackedBuff, dwPackedBuffSize);

    if (pBuffer != NULL)
    {
        free(pBuffer);
        pBuffer = NULL;
    }

	return m_clientSock.SendTo((char *)m_chPackedBuff, dwPackedBuffSize);
}

int COpenATCComWithControlCenterImpl::SendLampClrStatus()
{
	unsigned int  dwBufLen = 0;
	unsigned int  dwPackedBuffSize = 0;

	memset(m_chSendBuff, 0x00, SEND_BUFFER_SIZE);

	m_chSendBuff[dwBufLen++] = CB_VERSION_FLAG;										//版本号
	m_chSendBuff[dwBufLen++] = CB_ATC_FLAG;											//发送方标识
	m_chSendBuff[dwBufLen++] = CB_CONFIGSOFTWARE_FLAG;								//接收方标识
	m_chSendBuff[dwBufLen++] = LINK_BASEINFO;										//数据链路码
	m_chSendBuff[dwBufLen++] = m_tAreaInfo.m_chAscRegionNo;							//区域号
	memcpy(m_chSendBuff + dwBufLen, &m_tAreaInfo.m_usAscRoadNo, MAX_ROADNO_LENGTH);	//路口号
    dwBufLen += MAX_ROADNO_LENGTH;
	m_chSendBuff[dwBufLen++] = REPORT_STATUS;										//操作类型
	m_chSendBuff[dwBufLen++] = CTL_LAMPCOLORSTATUS;									//对象标识
	m_chSendBuff[dwBufLen++] = 0x01;												//保留
	m_chSendBuff[dwBufLen++] = 0x00;												//保留
	m_chSendBuff[dwBufLen++] = 0x00;												//保留
	m_chSendBuff[dwBufLen++] = 0x00;												//保留
	m_chSendBuff[dwBufLen++] = 0x00;												//保留

	unsigned char *pBuffer = m_pOpenATCParameter->GetChannelLampStatusInfo();
	if (pBuffer != NULL)
	{
		memcpy(m_chSendBuff + dwBufLen, pBuffer, strlen((const char *)pBuffer));
		dwBufLen += strlen((const char *)pBuffer);
	}

	m_pDataPackUnpackMode->PackBuffer(m_chSendBuff, dwBufLen, m_chPackedBuff, dwPackedBuffSize);

	if (pBuffer != NULL)
	{
		free(pBuffer);
		pBuffer = NULL;
	}

	return m_clientSock.SendTo((char *)m_chPackedBuff, dwPackedBuffSize);
}

int COpenATCComWithControlCenterImpl::SendFault()
{
    TAscFault fault;
    memset(&fault, 0x00, sizeof(TAscFault));
    m_pOpenATCRunStatus->PopFaultFromQueue(fault);

    if (fault.m_byBoardType == 0)
    {
        return 1;
    }

    unsigned int  dwBufLen = 0; 
	unsigned int  dwPackedBuffSize = 0;
    
    memset(m_chSendBuff, 0x00, SEND_BUFFER_SIZE);

	m_chSendBuff[dwBufLen++] = CB_VERSION_FLAG;										//版本号
	m_chSendBuff[dwBufLen++] = CB_ATC_FLAG;											//发送方标识
	m_chSendBuff[dwBufLen++] = CB_CONFIGSOFTWARE_FLAG;								//接收方标识
	m_chSendBuff[dwBufLen++] = LINK_BASEINFO;										//数据链路码
	m_chSendBuff[dwBufLen++] = m_tAreaInfo.m_chAscRegionNo;							//区域号
    memcpy(m_chSendBuff + dwBufLen, &m_tAreaInfo.m_usAscRoadNo, MAX_ROADNO_LENGTH);	//路口号
    dwBufLen += MAX_ROADNO_LENGTH;
	m_chSendBuff[dwBufLen++] = REPORT_STATUS;										//操作类型
	m_chSendBuff[dwBufLen++] = CTL_FAULT;											//对象标识
	m_chSendBuff[dwBufLen++] = 0x01;												//保留
	m_chSendBuff[dwBufLen++] = 0x00;												//保留
	m_chSendBuff[dwBufLen++] = 0x00;												//保留
	m_chSendBuff[dwBufLen++] = 0x00;												//保留
	m_chSendBuff[dwBufLen++] = 0x00;												//保留

    unsigned char *pBuffer = m_pOpenATCParameter->GetATCFaultReportData(&fault);
    if (pBuffer != NULL)
    {    
        memcpy(m_chSendBuff + dwBufLen, pBuffer, strlen((const char *)pBuffer));
        dwBufLen += strlen((const char *)pBuffer);
    }

 #if 0
    TAscFault fault;
    memset(&fault, 0x00, sizeof(TAscFault));
    m_pOpenATCRunStatus->PopFaultFromQueue(fault);
 
    unsigned char reportData[14] = {0};
    reportData[0] = 1;
    reportData[1] = fault.m_byBoardType;
    memcpy(reportData + 2, &fault.m_unFaultOccurTime, sizeof(long));
    reportData[2 + sizeof(long)] = fault.m_byFaultLevel;
    memcpy(reportData + 3 + sizeof(long), &fault.m_wFaultType, sizeof(long));

    memcpy(m_chSendBuff + dwBufLen, reportData, 13);
    dwBufLen += 13;
#endif
    
	m_pDataPackUnpackMode->PackBuffer(m_chSendBuff, dwBufLen, m_chPackedBuff, dwPackedBuffSize);

    if (pBuffer != NULL)
    {
        free(pBuffer);
        pBuffer = NULL;
    }

	return m_clientSock.SendTo((char *)m_chPackedBuff, dwPackedBuffSize);
}

int COpenATCComWithControlCenterImpl::SendVersion()
{
    unsigned int  dwBufLen = 0; 
	unsigned int  dwPackedBuffSize = 0;

    memset(m_chSendBuff, 0x00, SEND_BUFFER_SIZE);

	m_chSendBuff[dwBufLen++] = CB_VERSION_FLAG;										//版本号
	m_chSendBuff[dwBufLen++] = CB_ATC_FLAG;											//发送方标识
	m_chSendBuff[dwBufLen++] = CB_CONFIGSOFTWARE_FLAG;								//接收方标识
	m_chSendBuff[dwBufLen++] = LINK_BASEINFO;										//数据链路码
	m_chSendBuff[dwBufLen++] = m_tAreaInfo.m_chAscRegionNo;							//区域号
	memcpy(m_chSendBuff + dwBufLen, &m_tAreaInfo.m_usAscRoadNo, MAX_ROADNO_LENGTH);	//路口号
    dwBufLen += MAX_ROADNO_LENGTH;
	m_chSendBuff[dwBufLen++] = REPORT_STATUS;										//操作类型
	m_chSendBuff[dwBufLen++] = CTL_VERSION;											//对象标识
	m_chSendBuff[dwBufLen++] = 0x01;												//保留
	m_chSendBuff[dwBufLen++] = 0x00;												//保留
	m_chSendBuff[dwBufLen++] = 0x00;												//保留
	m_chSendBuff[dwBufLen++] = 0x00;												//保留
	m_chSendBuff[dwBufLen++] = 0x00;												//保留

    unsigned char *pBuffer = m_pOpenATCParameter->GetATCVersion(m_chOpenATCVersion);
    if (pBuffer != NULL)
    {    
        memcpy(m_chSendBuff + dwBufLen, pBuffer, strlen((const char *)pBuffer));
        dwBufLen += strlen((const char *)pBuffer);
    }

#if 0
    unsigned char chVersion[21] = {0};
    memcpy(chVersion, "version:1.0000000000", strlen("version:1.0000000000"));

    memcpy(m_chSendBuff + dwBufLen, chVersion, 20);
    dwBufLen += 20;
#endif

	m_pDataPackUnpackMode->PackBuffer(m_chSendBuff, dwBufLen, m_chPackedBuff, dwPackedBuffSize);

    if (pBuffer != NULL)
    {
        free(pBuffer);
        pBuffer = NULL;
    }

	return m_clientSock.SendTo((char *)m_chPackedBuff, dwPackedBuffSize);
}

int COpenATCComWithControlCenterImpl::AckCtl_AskWorkStatus()
{
    unsigned int  dwSendBuffLen = 0;
    unsigned int  dwPackedBuffSize = 0;

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

#if 0
    unsigned char workStatus[7] = {0};
    TPhaseRunStatus m_tAscPhaseRunStatus;
	memset(&m_tAscPhaseRunStatus, 0x00, sizeof(TPhaseRunStatus));

	m_pOpenATCRunStatus->GetPhaseRunStatus(m_tAscPhaseRunStatus);

    workStatus[0] = m_tAscPhaseRunStatus.m_nCurCtlPattern;								//控制模式
    workStatus[1] = m_tAscPhaseRunStatus.m_nCurCtlMode;									//控制方式
    workStatus[2] = m_tAscPhaseRunStatus.m_byPlanID;									//方案编号
    workStatus[3] = m_tAscPhaseRunStatus.m_atRingRunStatus[0].m_nCurRunPhaseIndex + 1;	//当前相位
    workStatus[4] = m_tAscPhaseRunStatus.m_atRingRunStatus[0].m_atPhaseStatus[m_tAscPhaseRunStatus.m_atRingRunStatus[0].m_nCurRunPhaseIndex].m_chPhaseStatus;//当前相位状态

    memcpy(m_chSendBuff + dwSendBuffLen, workStatus, 6);
    dwSendBuffLen += 6;
 
#endif
    
    m_pDataPackUnpackMode->PackBuffer(m_chSendBuff, dwSendBuffLen, m_chPackedBuff, dwPackedBuffSize);

    if (pBuffer != NULL)
    {
        free(pBuffer);
        pBuffer = NULL;
    }

	return m_clientSock.SendTo((char *)m_chPackedBuff, dwPackedBuffSize);
}

int COpenATCComWithControlCenterImpl::AckCtl_AskLampClrStatus()
{
    int i = 0;
    unsigned int  dwSendBuffLen = 0;
    unsigned int  dwPackedBuffSize = 0;

	memset(m_chSendBuff, 0x00, SEND_BUFFER_SIZE);

    TLampClrStatus tLampClrStatus;
    m_pOpenATCRunStatus->GetLampClrStatus(tLampClrStatus);

    TCanData tCanData[20];

    for (i = 0;i < C_N_MAXLAMPOUTPUT_NUM;i += C_N_CHANNEL_OUTPUTNUM)
    {
        GetGropStatusByRYG(i / C_N_LAMPBORAD_OUTPUTNUM, i % 12, tLampClrStatus.m_achLampClr[i], tLampClrStatus.m_achLampClr[i + 1], 
							tLampClrStatus.m_achLampClr[i + 2], tCanData[i / C_N_LAMPBORAD_OUTPUTNUM]);    
    }

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

    for (i = 0;i < 12;i++)
    {
	    memcpy(m_chSendBuff + dwSendBuffLen, &tCanData[i].chCanData, 1);
        dwSendBuffLen += 1;
    }

	m_pDataPackUnpackMode->PackBuffer(m_chSendBuff, dwSendBuffLen, m_chPackedBuff, dwPackedBuffSize);

	return m_clientSock.SendTo((char *)m_chPackedBuff, dwPackedBuffSize);
}

int COpenATCComWithControlCenterImpl::AckCtl_AskQueryATCLocalTime()
{
    unsigned int  dwSendBuffLen = 0;
    unsigned int  dwPackedBuffSize = 0;

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
  
#if 0
    time_t nowTime;
	nowTime = time(NULL) + 8 * 3600;

   	memcpy(m_chSendBuff + dwSendBuffLen, &nowTime, 4);
    dwSendBuffLen += 4;

#endif

    m_pDataPackUnpackMode->PackBuffer(m_chSendBuff, dwSendBuffLen, m_chPackedBuff, dwPackedBuffSize);

    if (pBuffer != NULL)
    {
        free(pBuffer);
        pBuffer = NULL;
    }

	return m_clientSock.SendTo((char *)m_chPackedBuff, dwPackedBuffSize);
}

int COpenATCComWithControlCenterImpl::AckCtl_AskSetATCLocalTime(bool bWrongAck)
{
    unsigned int  dwSendBuffLen = 0;
    unsigned int  dwPackedBuffSize = 0;

	memset(m_chSendBuff, 0x00, SEND_BUFFER_SIZE);

	m_chSendBuff[dwSendBuffLen++] = CB_VERSION_FLAG;									//版本号
	m_chSendBuff[dwSendBuffLen++] = CB_ATC_FLAG;										//发送方标识
	m_chSendBuff[dwSendBuffLen++] = CB_CONFIGSOFTWARE_FLAG;								//接收方标识
	m_chSendBuff[dwSendBuffLen++] = LINK_BASEINFO;										//数据链路码
	m_chSendBuff[dwSendBuffLen++] = m_tAreaInfo.m_chAscRegionNo;						//区域号
	memcpy(m_chSendBuff + dwSendBuffLen, &m_tAreaInfo.m_usAscRoadNo, MAX_ROADNO_LENGTH);//路口号
    dwSendBuffLen += MAX_ROADNO_LENGTH;
    if (!bWrongAck)
    {
	    m_chSendBuff[dwSendBuffLen++] = ACK_SET;										//操作类型
    }
    else
    {
        m_chSendBuff[dwSendBuffLen++] = ACK_WRONG;										//操作类型
    }
	m_chSendBuff[dwSendBuffLen++] = CTL_CURRENTTIME;									//对象标识
	m_chSendBuff[dwSendBuffLen++] = 0x01;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留

    m_pDataPackUnpackMode->PackBuffer(m_chSendBuff, dwSendBuffLen, m_chPackedBuff, dwPackedBuffSize);

	return m_clientSock.SendTo((char *)m_chPackedBuff, dwPackedBuffSize);
}

int COpenATCComWithControlCenterImpl::AckCtl_AskQueryFault()
{
    TAscFault fault;
    memset(&fault, 0x00, sizeof(TAscFault));
    m_pOpenATCRunStatus->PopFaultFromQueue(fault);

    unsigned int  dwSendBuffLen = 0;
    unsigned int  dwPackedBuffSize = 0;

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
 
    unsigned char *pBuffer = m_pOpenATCParameter->GetATCFaultReportData(&fault);
    if (pBuffer != NULL)
    {    
        memcpy(m_chSendBuff + dwSendBuffLen, pBuffer, strlen((const char *)pBuffer));
        dwSendBuffLen += strlen((const char *)pBuffer);
    }
    
#if 0
    unsigned char reportData[14] = {0};
    reportData[0] = 1;
    reportData[1] = fault.m_byBoardType;
    memcpy(reportData + 2, &fault.m_unFaultOccurTime, sizeof(long));
    reportData[2 + sizeof(long)] = fault.m_byFaultLevel;
    memcpy(reportData + 3 + sizeof(long), &fault.m_wFaultType, sizeof(long));

    memcpy(m_chSendBuff + dwSendBuffLen, reportData, 13);
    dwSendBuffLen += 13;
#endif
    
	m_pDataPackUnpackMode->PackBuffer(m_chSendBuff, dwSendBuffLen, m_chPackedBuff, dwPackedBuffSize);

    if (pBuffer != NULL)
    {
        free(pBuffer);
        pBuffer = NULL;
    }

	return m_clientSock.SendTo((char *)m_chPackedBuff, dwPackedBuffSize);
}

int COpenATCComWithControlCenterImpl::AckCtl_AskQueryVersion()
{
    unsigned int  dwSendBuffLen = 0;
    unsigned int  dwPackedBuffSize = 0;

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

#if 0
    unsigned char chVersion[21] = {0};

    memcpy(chVersion, "version:1.0000000000", strlen("version:1.0000000000"));

	memcpy(m_chSendBuff + dwSendBuffLen, chVersion, 20);
    dwSendBuffLen += 20;

#endif

	m_pDataPackUnpackMode->PackBuffer(m_chSendBuff, dwSendBuffLen, m_chPackedBuff, dwPackedBuffSize);

    if (pBuffer != NULL)
    {
        free(pBuffer);
        pBuffer = NULL;
    }

	return m_clientSock.SendTo((char *)m_chPackedBuff, dwPackedBuffSize);
}

int COpenATCComWithControlCenterImpl::AckCtl_AskQuerySignalLightGroup()
{
    int i = 0;
    unsigned int  dwSendBuffLen = 0;
    unsigned int  dwPackedBuffSize = 0;

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

#if 0
    if (!m_bSetSignalLightGroup)
    {
        TAscParam tAscParamInfo;
        m_pOpenATCParameter->GetAscParamInfo(tAscParamInfo);

        memset(m_chSignalLightGroupBuffer, 0x00, 1 + MAX_CHANNEL_COUNT * 12);
   
        m_chSignalLightGroupBuffer[0] = tAscParamInfo.m_stChannelTableValidSize;		//通道数量

        for (i = 0;i < tAscParamInfo.m_stChannelTableValidSize;i++)
        {
            if (tAscParamInfo.m_stAscChannelTable[i].m_byChannelNumber != 0)
            {
                m_chSignalLightGroupBuffer[i * 12 + 1] = (unsigned char)tAscParamInfo.m_stAscChannelTable[i].m_byChannelNumber;
                m_chSignalLightGroupBuffer[i * 12 + 2] = (unsigned char)tAscParamInfo.m_stAscChannelTable[i].m_byChannelControlSource;
                m_chSignalLightGroupBuffer[i * 12 + 3] = (unsigned char)tAscParamInfo.m_stAscChannelTable[i].m_byChannelControlType;
            }
        }
    }
    
    memcpy(m_chSendBuff + dwSendBuffLen, m_chSignalLightGroupBuffer, 1 + m_chSignalLightGroupBuffer[0] * 12);
    dwSendBuffLen += (1 + m_chSignalLightGroupBuffer[0] * 12);
#endif

	m_pDataPackUnpackMode->PackBuffer(m_chSendBuff, dwSendBuffLen, m_chPackedBuff, dwPackedBuffSize);

    if (pBuffer != NULL)
    {
        free(pBuffer);
        pBuffer = NULL;
    }

	return m_clientSock.SendTo((char *)m_chPackedBuff, dwPackedBuffSize);
}

int COpenATCComWithControlCenterImpl::AckCtl_AskSetSignalLightGroup(bool bWrongAck)
{
    unsigned int  dwSendBuffLen = 0;
    unsigned int  dwPackedBuffSize = 0;

	memset(m_chSendBuff, 0x00, SEND_BUFFER_SIZE);

	m_chSendBuff[dwSendBuffLen++] = CB_VERSION_FLAG;									//版本号
	m_chSendBuff[dwSendBuffLen++] = CB_ATC_FLAG;										//发送方标识
	m_chSendBuff[dwSendBuffLen++] = CB_CONFIGSOFTWARE_FLAG;								//接收方标识
	m_chSendBuff[dwSendBuffLen++] = LINK_ALTERNATEFEATUREPARA;							//数据链路码
	m_chSendBuff[dwSendBuffLen++] = m_tAreaInfo.m_chAscRegionNo;						//区域号
	memcpy(m_chSendBuff + dwSendBuffLen, &m_tAreaInfo.m_usAscRoadNo, MAX_ROADNO_LENGTH);//路口号
    dwSendBuffLen += MAX_ROADNO_LENGTH;
	if (!bWrongAck)
    {
	    m_chSendBuff[dwSendBuffLen++] = ACK_SET;										//操作类型
    }
    else
    {
        m_chSendBuff[dwSendBuffLen++] = ACK_WRONG;										//操作类型
    }
	m_chSendBuff[dwSendBuffLen++] = CTL_SIGNALLIGHTGROUP;								//对象标识
	m_chSendBuff[dwSendBuffLen++] = 0x01;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留

	m_pDataPackUnpackMode->PackBuffer(m_chSendBuff, dwSendBuffLen, m_chPackedBuff, dwPackedBuffSize);

	return m_clientSock.SendTo((char *)m_chPackedBuff, dwPackedBuffSize);
}

int COpenATCComWithControlCenterImpl::AckCtl_AskQueryPhaseParamData()
{
    int i = 0, j = 0;
    unsigned int  dwSendBuffLen = 0;
    unsigned int  dwPackedBuffSize = 0;
    unsigned int  dwParamBufferLen = 0;

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

#if 0
    if (!m_bSetPhaseParam)
    {
        TPhaseRunStatus tRunStatus;
        memset(&tRunStatus,0,sizeof(TPhaseRunStatus));
        m_pOpenATCRunStatus->GetPhaseRunStatus(tRunStatus);

        memset(m_chPhaseParamBuffer, 0x00, 1 + MAX_PHASE_COUNT * 24);

        for (i = 0;i < tRunStatus.m_nRingCount;i++)
        {
            m_chPhaseParamBuffer[0] += tRunStatus.m_atRingRunStatus[i].m_nPhaseCount;//相位数量
            dwParamBufferLen += 1;
            for (j = 0;j < tRunStatus.m_atRingRunStatus[i].m_nPhaseCount;j++)
            {
                if (tRunStatus.m_atRingRunStatus[i].m_atPhaseStatus[j].m_byPhaseID != 0)
                {
                    memcpy(m_chPhaseParamBuffer + dwParamBufferLen, &tRunStatus.m_atRingRunStatus[i].m_atPhaseStatus[j].m_byPhaseID, 1);	//相位编号
                    dwParamBufferLen += 1;
                    memcpy(m_chPhaseParamBuffer + dwParamBufferLen, &tRunStatus.m_atRingRunStatus[i].m_atPhaseStatus[j].m_chPhaseStatus, 1);//工作状态
                    dwParamBufferLen += 1;
                }
            }
        }    
    }

    memcpy(m_chSendBuff + dwSendBuffLen, m_chPhaseParamBuffer, 1 + m_chPhaseParamBuffer[0] * 24);
    dwSendBuffLen += (1 + m_chPhaseParamBuffer[0] * 24);
#endif

	m_pDataPackUnpackMode->PackBuffer(m_chSendBuff, dwSendBuffLen, m_chPackedBuff, dwPackedBuffSize);

    if (pBuffer != NULL)
    {
        free(pBuffer);
        pBuffer = NULL;
    }

	return m_clientSock.SendTo((char *)m_chPackedBuff, dwPackedBuffSize);
}

int COpenATCComWithControlCenterImpl::AckCtl_AskSetPhaseParamData(bool bWrongAck)
{
    unsigned int  dwSendBuffLen = 0;
    unsigned int  dwPackedBuffSize = 0; 

	memset(m_chSendBuff, 0x00, SEND_BUFFER_SIZE);

	m_chSendBuff[dwSendBuffLen++] = CB_VERSION_FLAG;									//版本号
	m_chSendBuff[dwSendBuffLen++] = CB_ATC_FLAG;										//发送方标识
	m_chSendBuff[dwSendBuffLen++] = CB_CONFIGSOFTWARE_FLAG;								//接收方标识
	m_chSendBuff[dwSendBuffLen++] = LINK_ALTERNATEFEATUREPARA;							//数据链路码
	m_chSendBuff[dwSendBuffLen++] = m_tAreaInfo.m_chAscRegionNo;						//区域号
	memcpy(m_chSendBuff + dwSendBuffLen, &m_tAreaInfo.m_usAscRoadNo, MAX_ROADNO_LENGTH);//路口号
    dwSendBuffLen += MAX_ROADNO_LENGTH;
	if (!bWrongAck)
    {
	    m_chSendBuff[dwSendBuffLen++] = ACK_SET;										//操作类型
    }
    else
    {
        m_chSendBuff[dwSendBuffLen++] = ACK_WRONG;										//操作类型
    }
	m_chSendBuff[dwSendBuffLen++] = CTL_PHASE;											//对象标识
	m_chSendBuff[dwSendBuffLen++] = 0x01;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留

	m_pDataPackUnpackMode->PackBuffer(m_chSendBuff, dwSendBuffLen, m_chPackedBuff, dwPackedBuffSize);

	return m_clientSock.SendTo((char *)m_chPackedBuff, dwPackedBuffSize);
}

int COpenATCComWithControlCenterImpl::AckCtl_AskQueryPatternParamData()
{
    unsigned int  dwSendBuffLen = 0;
    unsigned int  dwPackedBuffSize = 0; 

	memset(m_chSendBuff, 0x00, SEND_BUFFER_SIZE);

	m_chSendBuff[dwSendBuffLen++] = CB_VERSION_FLAG;									//版本号
	m_chSendBuff[dwSendBuffLen++] = CB_ATC_FLAG;										//发送方标识
	m_chSendBuff[dwSendBuffLen++] = CB_CONFIGSOFTWARE_FLAG;								//接收方标识
	m_chSendBuff[dwSendBuffLen++] = LINK_ALTERNATEFEATUREPARA;							//数据链路码
	m_chSendBuff[dwSendBuffLen++] = m_tAreaInfo.m_chAscRegionNo;						//区域号
	memcpy(m_chSendBuff + dwSendBuffLen, &m_tAreaInfo.m_usAscRoadNo, MAX_ROADNO_LENGTH);//路口号
    dwSendBuffLen += MAX_ROADNO_LENGTH;
	m_chSendBuff[dwSendBuffLen++] = ACK_QUERY;											//操作类型
	m_chSendBuff[dwSendBuffLen++] = CTL_SIGNALMATCHTIME;								//对象标识
	m_chSendBuff[dwSendBuffLen++] = 0x01;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留

    unsigned char *pBuffer = m_pOpenATCParameter->GetPatternParamData();
    if (pBuffer != NULL)
    {    
        memcpy(m_chSendBuff + dwSendBuffLen, pBuffer, strlen((const char *)pBuffer));
        dwSendBuffLen += strlen((const char *)pBuffer);
    }

#if 0
    if (!m_bSetPatternParam)
    {
        TAscParam tAscParamInfo;
        m_pOpenATCParameter->GetAscParamInfo(tAscParamInfo);

        memset(m_chPatternParamBuffer, 0x00, 1 + MAX_PATTERN_COUNT * 24);
   
        m_chPatternParamBuffer[0] = tAscParamInfo.m_stPatternTableValidSize;			//方案数量
    
        for (int i = 0;i < tAscParamInfo.m_stPatternTableValidSize;i++)
        {
            if (tAscParamInfo.m_stAscPatternTable[i].m_byPatternNumber != 0)
            {
                m_chPatternParamBuffer[i * 24 + 1] = (unsigned char)tAscParamInfo.m_stAscPatternTable[i].m_byPatternNumber;
                memcpy(m_chPatternParamBuffer + i * 24 + 2,  &tAscParamInfo.m_stAscPatternTable[i].m_wPatternCycleTime, sizeof(WORD));
                memcpy(m_chPatternParamBuffer + i * 24 + 2 + sizeof(WORD),  &tAscParamInfo.m_stAscPatternTable[i].m_byPatternOffsetTime, sizeof(WORD));
                m_chPatternParamBuffer[i * 24 + 2 + sizeof(WORD) + sizeof(WORD)] = (unsigned char)tAscParamInfo.m_stAscPatternTable[i].m_byPatternSplitNumber;
                m_chPatternParamBuffer[i * 24 + 2 + sizeof(WORD) + sizeof(WORD) + 1] = (unsigned char)tAscParamInfo.m_stAscPatternTable[i].m_byPatternSequenceNumber;
            } 
        }
    }


    memcpy(m_chSendBuff + dwSendBuffLen, m_chPatternParamBuffer, 1 + m_chPatternParamBuffer[0] * 24);
    dwSendBuffLen += (1 + m_chPatternParamBuffer[0] * 24);

#endif

	m_pDataPackUnpackMode->PackBuffer(m_chSendBuff, dwSendBuffLen, m_chPackedBuff, dwPackedBuffSize);
	
	if (pBuffer != NULL)
    {
        free(pBuffer);
        pBuffer = NULL;
    }

	return m_clientSock.SendTo((char *)m_chPackedBuff, dwPackedBuffSize);
}

int COpenATCComWithControlCenterImpl::AckCtl_AskSetPatternParamData(bool bWrongAck)
{
    unsigned int  dwSendBuffLen = 0;
    unsigned int  dwPackedBuffSize = 0; 

	memset(m_chSendBuff, 0x00, SEND_BUFFER_SIZE);

	m_chSendBuff[dwSendBuffLen++] = CB_VERSION_FLAG;									//版本号
	m_chSendBuff[dwSendBuffLen++] = CB_ATC_FLAG;										//发送方标识
	m_chSendBuff[dwSendBuffLen++] = CB_CONFIGSOFTWARE_FLAG;								//接收方标识
	m_chSendBuff[dwSendBuffLen++] = LINK_ALTERNATEFEATUREPARA;							//数据链路码
	m_chSendBuff[dwSendBuffLen++] = m_tAreaInfo.m_chAscRegionNo;						//区域号
	memcpy(m_chSendBuff + dwSendBuffLen, &m_tAreaInfo.m_usAscRoadNo, MAX_ROADNO_LENGTH);//路口号
    dwSendBuffLen += MAX_ROADNO_LENGTH;
	if (!bWrongAck)
    {
	    m_chSendBuff[dwSendBuffLen++] = ACK_SET;										//操作类型
    }
    else
    {
        m_chSendBuff[dwSendBuffLen++] = ACK_WRONG;										//操作类型
    }
	m_chSendBuff[dwSendBuffLen++] = CTL_SIGNALMATCHTIME;								//对象标识
	m_chSendBuff[dwSendBuffLen++] = 0x01;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留

	m_pDataPackUnpackMode->PackBuffer(m_chSendBuff, dwSendBuffLen, m_chPackedBuff, dwPackedBuffSize);

	return m_clientSock.SendTo((char *)m_chPackedBuff, dwPackedBuffSize);
}

int COpenATCComWithControlCenterImpl::AckCtl_AskQueryPlanParamData()
{
    unsigned int  dwSendBuffLen = 0;
    unsigned int  dwPackedBuffSize = 0; 

	memset(m_chSendBuff, 0x00, SEND_BUFFER_SIZE);

	m_chSendBuff[dwSendBuffLen++] = CB_VERSION_FLAG;									//版本号
	m_chSendBuff[dwSendBuffLen++] = CB_ATC_FLAG;										//发送方标识
	m_chSendBuff[dwSendBuffLen++] = CB_CONFIGSOFTWARE_FLAG;								//接收方标识
	m_chSendBuff[dwSendBuffLen++] = LINK_ALTERNATEFEATUREPARA;							//数据链路码
	m_chSendBuff[dwSendBuffLen++] = m_tAreaInfo.m_chAscRegionNo;						//区域号
	memcpy(m_chSendBuff + dwSendBuffLen, &m_tAreaInfo.m_usAscRoadNo, MAX_ROADNO_LENGTH);//路口号
    dwSendBuffLen += MAX_ROADNO_LENGTH;
	m_chSendBuff[dwSendBuffLen++] = ACK_QUERY;											//操作类型
	m_chSendBuff[dwSendBuffLen++] = CTL_PROGRAMMESCHEDULEPLAN;							//对象标识
	m_chSendBuff[dwSendBuffLen++] = 0x01;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留

    unsigned char *pBuffer = m_pOpenATCParameter->GetPlanParamData();
    if (pBuffer != NULL)
    {    
        memcpy(m_chSendBuff + dwSendBuffLen, pBuffer, strlen((const char *)pBuffer));
        dwSendBuffLen += strlen((const char *)pBuffer);
    }

#if 0
    if (!m_bSetPlanParam)
    {
        TAscParam tAscParamInfo;
        m_pOpenATCParameter->GetAscParamInfo(tAscParamInfo);

        memset(m_chPlanParamBuffer, 0x00, 1 + MAX_SCHEDULE_COUNT * 12);
   
        m_chPlanParamBuffer[0] = tAscParamInfo.m_stScheduleTableValidSize;				//调度计划数量 

        for (int i = 0;i < tAscParamInfo.m_stScheduleTableValidSize;i++)
        {
            if (tAscParamInfo.m_stAscScheduleTable[i].m_wTimeBaseScheduleNumber != 0)
            {
                m_chPlanParamBuffer[i * 12 + 1] = (unsigned char)tAscParamInfo.m_stAscScheduleTable[i].m_wTimeBaseScheduleMonth;
                m_chPlanParamBuffer[i * 12 + 2] = (unsigned char)tAscParamInfo.m_stAscScheduleTable[i].m_byTimeBaseScheduleDay;
                m_chPlanParamBuffer[i * 12 + 3] = (unsigned char)tAscParamInfo.m_stAscScheduleTable[i].m_dwTimeBaseScheduleDate;
                m_chPlanParamBuffer[i * 12 + 4] = (unsigned char)tAscParamInfo.m_stAscScheduleTable[i].m_byTimeBaseScheduleDayPlan;
            }
        }
    }

    memcpy(m_chSendBuff + dwSendBuffLen, m_chPlanParamBuffer, 1 + m_chPlanParamBuffer[0] * 12);
    dwSendBuffLen += (1 + m_chPlanParamBuffer[0] * 12);

#endif

	m_pDataPackUnpackMode->PackBuffer(m_chSendBuff, dwSendBuffLen, m_chPackedBuff, dwPackedBuffSize);

	if (pBuffer != NULL)
    {
        free(pBuffer);
        pBuffer = NULL;
    }

	return m_clientSock.SendTo((char *)m_chPackedBuff, dwPackedBuffSize);
}

int COpenATCComWithControlCenterImpl::AckCtl_AskSetPlanParamData(bool bWrongAck)
{
    unsigned int  dwSendBuffLen = 0;
    unsigned int  dwPackedBuffSize = 0; 

	memset(m_chSendBuff, 0x00, SEND_BUFFER_SIZE);

	m_chSendBuff[dwSendBuffLen++] = CB_VERSION_FLAG;									//版本号
	m_chSendBuff[dwSendBuffLen++] = CB_ATC_FLAG;										//发送方标识
	m_chSendBuff[dwSendBuffLen++] = CB_CONFIGSOFTWARE_FLAG;								//接收方标识
	m_chSendBuff[dwSendBuffLen++] = LINK_ALTERNATEFEATUREPARA;							//数据链路码
	m_chSendBuff[dwSendBuffLen++] = m_tAreaInfo.m_chAscRegionNo;						//区域号
	memcpy(m_chSendBuff + dwSendBuffLen, &m_tAreaInfo.m_usAscRoadNo, MAX_ROADNO_LENGTH);//路口号
    dwSendBuffLen += MAX_ROADNO_LENGTH;
	if (!bWrongAck)
    {
	    m_chSendBuff[dwSendBuffLen++] = ACK_SET;										//操作类型
    }
    else
    {
        m_chSendBuff[dwSendBuffLen++] = ACK_WRONG;										//操作类型
    }
	m_chSendBuff[dwSendBuffLen++] = CTL_PROGRAMMESCHEDULEPLAN;							//对象标识
	m_chSendBuff[dwSendBuffLen++] = 0x01;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留

	m_pDataPackUnpackMode->PackBuffer(m_chSendBuff, dwSendBuffLen, m_chPackedBuff, dwPackedBuffSize);

	return m_clientSock.SendTo((char *)m_chPackedBuff, dwPackedBuffSize);
}

int COpenATCComWithControlCenterImpl::AckCtl_AskQueryWorkWay()
{
    unsigned int  dwSendBuffLen = 0;
    unsigned int  dwPackedBuffSize = 0; 

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

#if 0
    if (!m_bSetWorkWay)
    {
        TLogicCtlStatus tCtlStatus;
        m_pOpenATCRunStatus->GetLogicCtlStatus(tCtlStatus);

        m_chWorkWay = tCtlStatus.m_nCurCtlMode;											//控制方式
    }

    m_chSendBuff[dwSendBuffLen++] = m_chWorkWay;
#endif
 
	m_pDataPackUnpackMode->PackBuffer(m_chSendBuff, dwSendBuffLen, m_chPackedBuff, dwPackedBuffSize);

	if (pBuffer != NULL)
    {
        free(pBuffer);
        pBuffer = NULL;
    }

	return m_clientSock.SendTo((char *)m_chPackedBuff, dwPackedBuffSize);
}

int COpenATCComWithControlCenterImpl::AckCtl_AskSetWorkWay(bool bWrongAck)
{
    unsigned int  dwSendBuffLen = 0;
    unsigned int  dwPackedBuffSize = 0; 

	memset(m_chSendBuff, 0x00, SEND_BUFFER_SIZE);

	m_chSendBuff[dwSendBuffLen++] = CB_VERSION_FLAG;									//版本号
	m_chSendBuff[dwSendBuffLen++] = CB_ATC_FLAG;										//发送方标识
	m_chSendBuff[dwSendBuffLen++] = CB_CONFIGSOFTWARE_FLAG;								//接收方标识
	m_chSendBuff[dwSendBuffLen++] = LINK_INTERVENECOMMAND;								//数据链路码
	m_chSendBuff[dwSendBuffLen++] = m_tAreaInfo.m_chAscRegionNo;						//区域号
	memcpy(m_chSendBuff + dwSendBuffLen, &m_tAreaInfo.m_usAscRoadNo, MAX_ROADNO_LENGTH);//路口号
    dwSendBuffLen += 2;
	if (!bWrongAck)
    {
	    m_chSendBuff[dwSendBuffLen++] = ACK_SET;										//操作类型
    }
    else
    {
        m_chSendBuff[dwSendBuffLen++] = ACK_WRONG;										//操作类型
    }
	m_chSendBuff[dwSendBuffLen++] = CTL_WORKWAY;										//对象标识
	m_chSendBuff[dwSendBuffLen++] = 0x01;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留

	m_pDataPackUnpackMode->PackBuffer(m_chSendBuff, dwSendBuffLen, m_chPackedBuff, dwPackedBuffSize);

	return m_clientSock.SendTo((char *)m_chPackedBuff, dwPackedBuffSize);
}

int COpenATCComWithControlCenterImpl::AckCtl_AskQueryATCParamVersion()
{
    unsigned int  dwSendBuffLen = 0;
    unsigned int  dwPackedBuffSize = 0; 

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

#if 0
    m_chSendBuff[dwSendBuffLen++] = m_chParamVersion;
#endif

	m_pDataPackUnpackMode->PackBuffer(m_chSendBuff, dwSendBuffLen, m_chPackedBuff, dwPackedBuffSize);

	if (pBuffer != NULL)
    {
        free(pBuffer);
        pBuffer = NULL;
    }

	return m_clientSock.SendTo((char *)m_chPackedBuff, dwPackedBuffSize);
}

int COpenATCComWithControlCenterImpl::AckCtl_AskSetATCParamVersion(bool bWrongAck)
{
    unsigned int  dwSendBuffLen = 0;
    unsigned int  dwPackedBuffSize = 0; 

	memset(m_chSendBuff, 0x00, SEND_BUFFER_SIZE);

	m_chSendBuff[dwSendBuffLen++] = CB_VERSION_FLAG;									//版本号
	m_chSendBuff[dwSendBuffLen++] = CB_ATC_FLAG;										//发送方标识
	m_chSendBuff[dwSendBuffLen++] = CB_CONFIGSOFTWARE_FLAG;								//接收方标识
	m_chSendBuff[dwSendBuffLen++] = LINK_INTERVENECOMMAND;								//数据链路码
	m_chSendBuff[dwSendBuffLen++] = m_tAreaInfo.m_chAscRegionNo;						//区域号
	memcpy(m_chSendBuff + dwSendBuffLen, &m_tAreaInfo.m_usAscRoadNo, MAX_ROADNO_LENGTH);//路口号
    dwSendBuffLen += MAX_ROADNO_LENGTH;
	if (!bWrongAck)
    {
	    m_chSendBuff[dwSendBuffLen++] = ACK_SET;										//操作类型
    }
    else
    {
        m_chSendBuff[dwSendBuffLen++] = ACK_WRONG;										//操作类型
    }
	m_chSendBuff[dwSendBuffLen++] = CTL_FEATUREPARAVERSION;								//对象标识
	m_chSendBuff[dwSendBuffLen++] = 0x01;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留

	m_pDataPackUnpackMode->PackBuffer(m_chSendBuff, dwSendBuffLen, m_chPackedBuff, dwPackedBuffSize);

	return m_clientSock.SendTo((char *)m_chPackedBuff, dwPackedBuffSize);
}

int COpenATCComWithControlCenterImpl::AckCtl_AskQueryATCCode()
{
    unsigned int  dwSendBuffLen = 0;
    unsigned int  dwPackedBuffSize = 0; 

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

#if 0
    char szAddressCode[15] = {0};
    memcpy(szAddressCode, "12345620010001", 14);
	
    memcpy(m_chSendBuff + dwSendBuffLen, szAddressCode, 14);
    dwSendBuffLen += 14;
#endif

	m_pDataPackUnpackMode->PackBuffer(m_chSendBuff, dwSendBuffLen, m_chPackedBuff, dwPackedBuffSize);

	if (pBuffer != NULL)
    {
        free(pBuffer);
        pBuffer = NULL;
    }

	return m_clientSock.SendTo((char *)m_chPackedBuff, dwPackedBuffSize);
}

int COpenATCComWithControlCenterImpl::AckCtl_AskRemoteControl(bool bWrongAck)
{
    unsigned int  dwSendBuffLen = 0;
    unsigned int  dwPackedBuffSize = 0; 

	memset(m_chSendBuff, 0x00, SEND_BUFFER_SIZE);

	m_chSendBuff[dwSendBuffLen++] = CB_VERSION_FLAG;									//版本号
	m_chSendBuff[dwSendBuffLen++] = CB_ATC_FLAG;										//发送方标识
	m_chSendBuff[dwSendBuffLen++] = CB_CONFIGSOFTWARE_FLAG;								//接收方标识
	m_chSendBuff[dwSendBuffLen++] = LINK_INTERVENECOMMAND;								//数据链路码
	m_chSendBuff[dwSendBuffLen++] = m_tAreaInfo.m_chAscRegionNo;						//区域号
	memcpy(m_chSendBuff + dwSendBuffLen, &m_tAreaInfo.m_usAscRoadNo, MAX_ROADNO_LENGTH);//路口号
    dwSendBuffLen += MAX_ROADNO_LENGTH;
	if (!bWrongAck)
    {
	    m_chSendBuff[dwSendBuffLen++] = ACK_SET;										//操作类型
    }
    else
    {
        m_chSendBuff[dwSendBuffLen++] = ACK_WRONG;										//操作类型
    }
	m_chSendBuff[dwSendBuffLen++] = CTL_REMOTECONTROL;									//对象标识
	m_chSendBuff[dwSendBuffLen++] = 0x01;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留

	m_pDataPackUnpackMode->PackBuffer(m_chSendBuff, dwSendBuffLen, m_chPackedBuff, dwPackedBuffSize);

	return m_clientSock.SendTo((char *)m_chPackedBuff, dwPackedBuffSize);
}

int COpenATCComWithControlCenterImpl::AckCtl_AskQueryVecDetectorParamData()
{
    unsigned int  dwSendBuffLen = 0;
    unsigned int  dwPackedBuffSize = 0; 

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

#if 0
    if (!m_bSetVedetectorParam)
    {
        TAscParam tAscParamInfo;
        m_pOpenATCParameter->GetAscParamInfo(tAscParamInfo);
  
        TVehicleDetector atVehDetector[MAX_VEHICLEDETECTOR_COUNT];
        memset(atVehDetector, 0, sizeof(TVehicleDetector) * MAX_VEHICLEDETECTOR_COUNT);
        m_pOpenATCParameter->GetVehicleDetectorTable(atVehDetector);
   
        memset(m_chVecDetectorParamBuffer, 0x00, 1 + MAX_VEHICLEDETECTOR_COUNT * 15);
   
        m_chVecDetectorParamBuffer[0] = tAscParamInfo.m_stVehicleDetectorTableValidSize;//检测器数量

        for (int i = 0;i < tAscParamInfo.m_stVehicleDetectorTableValidSize;i++)
        {
            memcpy(m_chVecDetectorParamBuffer + 1 + i * 15, atVehDetector + i, sizeof(TVehicleDetector));
        }
    }
   
    memcpy(m_chSendBuff + dwSendBuffLen, m_chVecDetectorParamBuffer, 1 + m_chVecDetectorParamBuffer[0] * 15);
    dwSendBuffLen += (1 + m_chVecDetectorParamBuffer[0] * 15);
#endif

	m_pDataPackUnpackMode->PackBuffer(m_chSendBuff, dwSendBuffLen, m_chPackedBuff, dwPackedBuffSize);
	
	if (pBuffer != NULL)
    {
        free(pBuffer);
        pBuffer = NULL;
    }

	return m_clientSock.SendTo((char *)m_chPackedBuff, dwPackedBuffSize);
}

int COpenATCComWithControlCenterImpl::AckCtl_AskSetVecDetectorParamData(bool bWrongAck)
{
    unsigned int  dwSendBuffLen = 0;
    unsigned int  dwPackedBuffSize = 0; 

	memset(m_chSendBuff, 0x00, SEND_BUFFER_SIZE);

	m_chSendBuff[dwSendBuffLen++] = CB_VERSION_FLAG;									//版本号
	m_chSendBuff[dwSendBuffLen++] = CB_ATC_FLAG;										//发送方标识
	m_chSendBuff[dwSendBuffLen++] = CB_CONFIGSOFTWARE_FLAG;								//接收方标识
	m_chSendBuff[dwSendBuffLen++] = LINK_INTERVENECOMMAND;								//数据链路码
	m_chSendBuff[dwSendBuffLen++] = m_tAreaInfo.m_chAscRegionNo;						//区域号
	memcpy(m_chSendBuff + dwSendBuffLen, &m_tAreaInfo.m_usAscRoadNo, MAX_ROADNO_LENGTH);//路口号
    dwSendBuffLen += MAX_ROADNO_LENGTH;
	if (!bWrongAck)
    {
	    m_chSendBuff[dwSendBuffLen++] = ACK_SET;										//操作类型
    }
    else
    {
        m_chSendBuff[dwSendBuffLen++] = ACK_WRONG;										//操作类型
    }
	m_chSendBuff[dwSendBuffLen++] = CTL_DETECTOR;										//对象标识
	m_chSendBuff[dwSendBuffLen++] = 0x01;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留
	m_chSendBuff[dwSendBuffLen++] = 0x00;												//保留

	m_pDataPackUnpackMode->PackBuffer(m_chSendBuff, dwSendBuffLen, m_chPackedBuff, dwPackedBuffSize);

	return m_clientSock.SendTo((char *)m_chPackedBuff, dwPackedBuffSize);
}

void COpenATCComWithControlCenterImpl::OpenATCSleep(long nMsec)
{
#ifdef _WIN32
    Sleep(nMsec);
#else
    usleep(nMsec*1000);
#endif
}

bool COpenATCComWithControlCenterImpl::SetSysTime(const long nTime)
{
	m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "COpenATCComWithControlCenterImpl SetSysTime");
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

void COpenATCComWithControlCenterImpl::GetGropStatusByRYG(int nBoardIndex,int nChannelIndex,char chR,char chY,char chG,TCanData & tCanData)
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

void  COpenATCComWithControlCenterImpl::GetCanData(unsigned char chLampClr[], TCanData tCanData[])
{
    tCanData[0].CanData.io1 = 0;  
    tCanData[0].CanData.io2 = 0;  
    tCanData[0].CanData.io3 = 0;  
    tCanData[0].CanData.io4 = 0;  

    tCanData[0].CanData.io5 = 0;  
    tCanData[0].CanData.io6 = 0;  
    tCanData[0].CanData.io7 = 0;  
    tCanData[0].CanData.io8 = 0;  

    tCanData[1].CanData.io1 = 0;  
    tCanData[1].CanData.io2 = 0;  
    tCanData[1].CanData.io3 = 0;  
    tCanData[1].CanData.io4 = 0;  

    tCanData[1].CanData.io5 = 0;  
    tCanData[1].CanData.io6 = 0;  
    tCanData[1].CanData.io7 = 0;  
    tCanData[1].CanData.io8 = 0;  

    if (chLampClr[0] == LAMP_CLR_ON)//红灯亮
    {
        tCanData[0].CanData.io5 = 1;
        tCanData[0].CanData.io6 = 0;
        tCanData[0].CanData.io7 = 0;
        tCanData[0].CanData.io8 = 0;
    }
    else if (chLampClr[1] == LAMP_CLR_ON)//黄灯亮
    {
        tCanData[0].CanData.io5 = 0;
        tCanData[0].CanData.io6 = 1;
        tCanData[0].CanData.io7 = 0;
        tCanData[0].CanData.io8 = 0;
    }
    else if (chLampClr[1] == LAMP_CLR_FLASH)//黄灯闪
    {
        tCanData[0].CanData.io5 = 1;
        tCanData[0].CanData.io6 = 0;
        tCanData[0].CanData.io7 = 1;
        tCanData[0].CanData.io8 = 0;
    }
    else if (chLampClr[2] == LAMP_CLR_ON)//绿灯亮
    {
        tCanData[0].CanData.io5 = 1;
        tCanData[0].CanData.io6 = 1;
        tCanData[0].CanData.io7 = 0;
        tCanData[0].CanData.io8 = 0;    
    }
    else if (chLampClr[2] == LAMP_CLR_FLASH)//绿灯闪
    {
        tCanData[0].CanData.io5 = 0;
        tCanData[0].CanData.io6 = 0;
        tCanData[0].CanData.io7 = 1;
        tCanData[0].CanData.io8 = 0;
    }

    if (chLampClr[3] == LAMP_CLR_ON)//红灯亮
    {
        tCanData[0].CanData.io1 = 1;
        tCanData[0].CanData.io2 = 0;
        tCanData[0].CanData.io3 = 0;
        tCanData[0].CanData.io4 = 0;
    }
    else if (chLampClr[4] == LAMP_CLR_ON)//黄灯亮
    {
        tCanData[0].CanData.io1 = 0;
        tCanData[0].CanData.io2 = 1;
        tCanData[0].CanData.io3 = 0;
        tCanData[0].CanData.io4 = 0;
    }
    else if (chLampClr[4] == LAMP_CLR_FLASH)//黄灯闪
    {
        tCanData[0].CanData.io1 = 1;
        tCanData[0].CanData.io2 = 0;
        tCanData[0].CanData.io3 = 1;
        tCanData[0].CanData.io4 = 0;
    }
    else if (chLampClr[5] == LAMP_CLR_ON)//绿灯亮
    {
        tCanData[0].CanData.io1 = 1;
        tCanData[0].CanData.io2 = 1;
        tCanData[0].CanData.io3 = 0;
        tCanData[0].CanData.io4 = 0;    
    }
    else if (chLampClr[5] == LAMP_CLR_FLASH)//绿灯闪
    {
        tCanData[0].CanData.io1 = 0;
        tCanData[0].CanData.io2 = 0;
        tCanData[0].CanData.io3 = 1;
        tCanData[0].CanData.io4 = 0;
    }
  
    ////////////////////////////////////////////////////////////////////
    if (chLampClr[6] == LAMP_CLR_ON)//红灯亮
    {
        tCanData[1].CanData.io5 = 1;
        tCanData[1].CanData.io6 = 0;
        tCanData[1].CanData.io7 = 0;
        tCanData[1].CanData.io8 = 0;
    }
    else if (chLampClr[7] == LAMP_CLR_ON)//黄灯亮
    {
        tCanData[1].CanData.io5 = 0;
        tCanData[1].CanData.io6 = 1;
        tCanData[1].CanData.io7 = 0;
        tCanData[1].CanData.io8 = 0;
    }
    else if (chLampClr[7] == LAMP_CLR_FLASH)//黄灯闪
    {
        tCanData[1].CanData.io5 = 1;
        tCanData[1].CanData.io6 = 0;
        tCanData[1].CanData.io7 = 1;
        tCanData[1].CanData.io8 = 0;
    }
    else if (chLampClr[8] == LAMP_CLR_ON)//绿灯亮
    {
        tCanData[1].CanData.io5 = 1;
        tCanData[1].CanData.io6 = 1;
        tCanData[1].CanData.io7 = 0;
        tCanData[1].CanData.io8 = 0;    
    }
    else if (chLampClr[8] == LAMP_CLR_FLASH)//绿灯闪
    {
        tCanData[1].CanData.io5 = 0;
        tCanData[1].CanData.io6 = 0;
        tCanData[1].CanData.io7 = 1;
        tCanData[1].CanData.io8 = 0;
    }

    if (chLampClr[9] == LAMP_CLR_ON)//红灯亮
    {
        tCanData[1].CanData.io1 = 1;
        tCanData[1].CanData.io2 = 0;
        tCanData[1].CanData.io3 = 0;
        tCanData[1].CanData.io4 = 0;
    }
    else if (chLampClr[10] == LAMP_CLR_ON)//黄灯亮
    {
        tCanData[1].CanData.io1 = 0;
        tCanData[1].CanData.io2 = 1;
        tCanData[1].CanData.io3 = 0;
        tCanData[1].CanData.io4 = 0;
    }
    else if (chLampClr[10] == LAMP_CLR_FLASH)//黄灯闪
    {
        tCanData[1].CanData.io1 = 1;
        tCanData[1].CanData.io2 = 0;
        tCanData[1].CanData.io3 = 1;
        tCanData[1].CanData.io4 = 0;
    }
    else if (chLampClr[11] == LAMP_CLR_ON)//绿灯亮
    {
        tCanData[1].CanData.io1 = 1;
        tCanData[1].CanData.io2 = 1;
        tCanData[1].CanData.io3 = 0;
        tCanData[1].CanData.io4 = 0;    
    }
    else if (chLampClr[11] == LAMP_CLR_FLASH)//绿灯闪
    {
        tCanData[1].CanData.io1 = 0;
        tCanData[1].CanData.io2 = 0;
        tCanData[1].CanData.io3 = 1;
        tCanData[1].CanData.io4 = 0;
    }
}

void COpenATCComWithControlCenterImpl::SetComPara(int nComType)
{
	 m_nComType = nComType;
}

int  COpenATCComWithControlCenterImpl::IsCenterParamChg()
{
	if (m_nComType == COM_WITH_CENTER)
	{
		long nCurTime = time(NULL);
		if (labs(nCurTime - m_lastGetCenterParam) > COMM_ISPARAMCHG_TIME)
		{
			m_lastGetCenterParam = nCurTime;

		    m_pOpenATCParameter->GetAscAreaInfo(m_tAreaInfo);

			TAscCenter tAscCenter;
			m_pOpenATCParameter->GetCenterInfo(tAscCenter);

			if (memcmp(&m_tOldAscCenter, &tAscCenter, sizeof(tAscCenter)) != 0)
			{
                memcpy(&m_tOldAscCenter, &tAscCenter, sizeof(tAscCenter));
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "COpenATCComWithControlCenterImpl IsCenterParamChg");
				return OPENATC_RTN_OK;
			}
		}
	}

	return OPENATC_RTN_FAILED;
}

int COpenATCComWithControlCenterImpl::SendDetectorChgData()
{
	TVehDetBoardData tVehDetBoardData;                  
    m_pOpenATCRunStatus->GetVehDetBoardData(tVehDetBoardData);

	TIOBoardData tIOBoardData;                  
    m_pOpenATCRunStatus->GetIOBoardData(tIOBoardData);

	if (memcmp(&m_tOldVehDetBoardData,&tVehDetBoardData,sizeof(TVehDetBoardData)) == 0 && memcmp(&m_tOldIOBoardData,&tIOBoardData,sizeof(TIOBoardData)) == 0)
	{
		return 0;
	}

	int  i = 0, j = 0;
	int  nVehDetectorCount = 0;
	int  nIODetectorCount = 0;
	char chVehDetectorID[C_N_MAXDETBOARD_NUM * C_N_MAXDETINPUT_NUM];
	memset(chVehDetectorID,0,sizeof(chVehDetectorID));
	char chVehDetectorStatus[C_N_MAXDETBOARD_NUM * C_N_MAXDETINPUT_NUM];
	memset(chVehDetectorStatus,0,sizeof(chVehDetectorStatus));
	char chIODetectorID[C_N_MAXIOBOARD_NUM * C_N_MAXIOINPUT_NUM];
	memset(chIODetectorID,0,sizeof(chIODetectorID));
	char chIODetectorStatus[C_N_MAXIOBOARD_NUM * C_N_MAXIOINPUT_NUM];
	memset(chIODetectorStatus,0,sizeof(chIODetectorStatus));

	for (i = 0;i < C_N_MAXDETBOARD_NUM;i++)
	{
		for (j = 0;j < C_N_MAXDETINPUT_NUM;j++)
		{
			if (m_tOldVehDetBoardData.m_atVehDetData[i].m_achVehChgVal[j] != tVehDetBoardData.m_atVehDetData[i].m_achVehChgVal[j])
			{
				chVehDetectorID[nVehDetectorCount] = i * C_N_MAXDETINPUT_NUM + j + 1;
				chVehDetectorStatus[nVehDetectorCount] =  tVehDetBoardData.m_atVehDetData[i].m_achVehChgVal[j];
				if (tVehDetBoardData.m_atVehDetData[i].m_bDetFaultStatus[j])
				{
					chVehDetectorStatus[nVehDetectorCount] = FAULT_LEVELDETECTOR;
				}
				else
				{
					if (tVehDetBoardData.m_atVehDetData[i].m_achVehChgVal[j])
					{
						chVehDetectorStatus[nVehDetectorCount] = HIGH_LEVEL_DETECTOR;
					}
					else
					{
						chVehDetectorStatus[nVehDetectorCount] = LOW_LEVEL_DETECTOR;
					}
				}
				nVehDetectorCount += 1;
			}
		}
	}

	if (nVehDetectorCount > 0)
	{
		memcpy(&m_tOldVehDetBoardData,&tVehDetBoardData,sizeof(TVehDetBoardData));
	}

	for (i = 0;i < C_N_MAXIOBOARD_NUM;i++)
	{
		for (j = 0;j < C_N_MAXIOINPUT_NUM;j++)
		{
			if (m_tOldIOBoardData.m_atIOBoardData[i].m_achIOStatus[j] != tIOBoardData.m_atIOBoardData[i].m_achIOStatus[j])
			{
				chIODetectorID[nIODetectorCount] = i * C_N_MAXIOBOARD_NUM + j + 1;
				chIODetectorStatus[nIODetectorCount] =  tIOBoardData.m_atIOBoardData[i].m_achIOStatus[j];
				nIODetectorCount += 1;
			}
		}
	}

	if (nIODetectorCount > 0)
	{
		memcpy(&m_tOldIOBoardData,&tIOBoardData,sizeof(TIOBoardData));
	}

	if (nVehDetectorCount == 0 && nIODetectorCount == 0)
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
	m_chSendBuff[dwSendBuffLen++] = CTL_DETECTOR_STATUS;							    //对象标识
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
	if (m_msgBody)
	{
		if (nVehDetectorCount)
		{
			cJSON* vehDetectorStatusInfo = cJSON_CreateArray();
			cJSON_AddItemToObject(m_msgBody, "detector", vehDetectorStatusInfo);
			for (i = 0; i < nVehDetectorCount;i++)
			{
				cJSON* curVehDetectorStatus = cJSON_CreateObject();
				if (curVehDetectorStatus)
				{
					cJSON_AddNumberToObject(curVehDetectorStatus, "id", chVehDetectorID[i]);
					cJSON_AddNumberToObject(curVehDetectorStatus, "state", chVehDetectorStatus[i]);
					cJSON_AddItemToObject(vehDetectorStatusInfo, "", curVehDetectorStatus);
				}
			}
		}
		if (nIODetectorCount)
		{
			cJSON* ioDetectorStatusInfo = cJSON_CreateArray();
			cJSON_AddItemToObject(m_msgBody, "io", ioDetectorStatusInfo);
			for (i = 0; i < nIODetectorCount;i++)
			{
				cJSON* curIODetectorStatus = cJSON_CreateObject();
				if (curIODetectorStatus)
				{
					cJSON_AddNumberToObject(curIODetectorStatus, "id", chIODetectorID[i]);
					cJSON_AddNumberToObject(curIODetectorStatus, "state", chIODetectorStatus[i]);
					cJSON_AddItemToObject(ioDetectorStatusInfo, "", curIODetectorStatus);
				}
			}
		}
	}

	m_chOutBuff = cJSON_Print(m_msgBody);
    memcpy(m_chSendBuff + dwSendBuffLen, m_chOutBuff, strlen(m_chOutBuff));
	dwSendBuffLen += strlen(m_chOutBuff);

	m_pDataPackUnpackMode->PackBuffer(m_chSendBuff, dwSendBuffLen, m_chPackedBuff, dwPackedBuffSize);

	return m_clientSock.SendTo((char *)m_chPackedBuff, dwPackedBuffSize);
}

int COpenATCComWithControlCenterImpl::SendDetectorStatusData()
{
	//获取配置软件配置的车检板数量
    TVehicleDetector atVehDetector[MAX_VEHICLEDETECTOR_COUNT];
    memset(atVehDetector,0,sizeof(TVehicleDetector) * MAX_VEHICLEDETECTOR_COUNT);
    m_pOpenATCParameter->GetVehicleDetectorTable(atVehDetector);

	//初始化IO板使用信息数组
	TPedestrianDetector atPedDetector[MAX_PEDESTRIANDETECTOR_COUNT];
    memset(atPedDetector,0,sizeof(TPedestrianDetector) * MAX_PEDESTRIANDETECTOR_COUNT);
    m_pOpenATCParameter->GetPedDetectorTable(atPedDetector);

	TVehDetBoardData tVehDetBoardData;                  
    m_pOpenATCRunStatus->GetVehDetBoardData(tVehDetBoardData);

	TIOBoardData tIOBoardData;                  
    m_pOpenATCRunStatus->GetIOBoardData(tIOBoardData);

	int  i = 0, j = 0;
	int  nVehDetectorCount = 0;
	int  nIODetectorCount = 0;
	char chVehDetectorID[C_N_MAXDETBOARD_NUM * C_N_MAXDETINPUT_NUM];
	memset(chVehDetectorID,0,sizeof(chVehDetectorID));
	char chVehDetectorStatus[C_N_MAXDETBOARD_NUM * C_N_MAXDETINPUT_NUM];
	memset(chVehDetectorStatus,0,sizeof(chVehDetectorStatus));
	char chIODetectorID[C_N_MAXIOBOARD_NUM * C_N_MAXIOINPUT_NUM];
	memset(chIODetectorID,0,sizeof(chIODetectorID));
	char chIODetectorStatus[C_N_MAXIOBOARD_NUM * C_N_MAXIOINPUT_NUM];
	memset(chIODetectorStatus,0,sizeof(chIODetectorStatus));

	for (i = 0;i < C_N_MAXDETBOARD_NUM;i++)
	{
		for (j = 0;j < C_N_MAXDETINPUT_NUM;j++)
		{
			chVehDetectorID[nVehDetectorCount] = i * C_N_MAXDETINPUT_NUM + j + 1;
			if (tVehDetBoardData.m_atVehDetData[i].m_bDetFaultStatus[j])
			{
				chVehDetectorStatus[nVehDetectorCount] = FAULT_LEVELDETECTOR;
			}
			else
			{
				if (tVehDetBoardData.m_atVehDetData[i].m_achVehChgVal[j])
				{
					chVehDetectorStatus[nVehDetectorCount] = HIGH_LEVEL_DETECTOR;
				}
				else
				{
					chVehDetectorStatus[nVehDetectorCount] = LOW_LEVEL_DETECTOR;
				}
			}
			nVehDetectorCount += 1;
		}
	}

	int  nRealVehDetectorCount = 0;
	int  nRealIODetectorCount = 0;
	char chRealVehDetectorID[C_N_MAXDETBOARD_NUM * C_N_MAXDETINPUT_NUM];
	memset(chRealVehDetectorID,0,sizeof(chRealVehDetectorID));
	char chRealVehDetectorStatus[C_N_MAXDETBOARD_NUM * C_N_MAXDETINPUT_NUM];
	memset(chRealVehDetectorStatus,0,sizeof(chRealVehDetectorStatus));
	char chRealIODetectorID[C_N_MAXIOBOARD_NUM * C_N_MAXIOINPUT_NUM];
	memset(chRealIODetectorID,0,sizeof(chRealIODetectorID));
	char chRealIODetectorStatus[C_N_MAXIOBOARD_NUM * C_N_MAXIOINPUT_NUM];
	memset(chRealIODetectorStatus,0,sizeof(chRealIODetectorStatus));

	if (nVehDetectorCount > 0)
	{
		for (i = 0;i < MAX_VEHICLEDETECTOR_COUNT;i++)
		{
			if (atVehDetector[i].m_byVehicleDetectorNumber == 0)
			{
				continue;
			}

			for (j = 0;j < nVehDetectorCount;j++)
			{
				if (atVehDetector[i].m_byVehicleDetectorNumber == chVehDetectorID[j])
				{
					chRealVehDetectorID[nRealVehDetectorCount] = chVehDetectorID[j];
					chRealVehDetectorStatus[nRealVehDetectorCount] = chVehDetectorStatus[j];
					nRealVehDetectorCount += 1;
				}
			}
		}
	}

	for (i = 0;i < C_N_MAXIOBOARD_NUM;i++)
	{
		for (j = 0;j < C_N_MAXIOINPUT_NUM;j++)
		{
			chIODetectorID[nIODetectorCount] = i * C_N_MAXIOBOARD_NUM + j + 1;
			chIODetectorStatus[nIODetectorCount] = tIOBoardData.m_atIOBoardData[i].m_achIOStatus[j];
			nIODetectorCount += 1;
		}
	}

	if (nIODetectorCount)
	{
		for (i = 0;i < MAX_PEDESTRIANDETECTOR_COUNT;i++)
		{
			if (atPedDetector[i].m_byPedestrianDetectorNumber == 0)
			{
				continue;
			}

			for (j = 0;j < nIODetectorCount;j++)
			{
				if (atPedDetector[i].m_byPedestrianDetectorNumber == chIODetectorID[j])
				{
					chRealIODetectorID[nRealIODetectorCount] = chIODetectorID[j];
					chRealIODetectorStatus[nRealIODetectorCount] = chIODetectorStatus[j];
					nRealIODetectorCount += 1;
				}
			}
		}
	}

	if (nRealVehDetectorCount == 0 && nRealIODetectorCount == 0)
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
	m_chSendBuff[dwSendBuffLen++] = CTL_DETECTOR_STATUS;								//对象标识
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
	if (m_msgBody)
	{
		if (nRealVehDetectorCount)
		{
			cJSON* vehDetectorStatusInfo = cJSON_CreateArray();
			cJSON_AddItemToObject(m_msgBody, "detector", vehDetectorStatusInfo);
			for (i = 0; i < nRealVehDetectorCount;i++)
			{
				cJSON* curVehDetectorStatus = cJSON_CreateObject();
				if (curVehDetectorStatus)
				{
					cJSON_AddNumberToObject(curVehDetectorStatus, "id", chRealVehDetectorID[i]);
					cJSON_AddNumberToObject(curVehDetectorStatus, "state", chRealVehDetectorStatus[i]);
					cJSON_AddItemToObject(vehDetectorStatusInfo, "", curVehDetectorStatus);
				}
			}
		}
		if (nRealIODetectorCount)
		{
			cJSON* ioDetectorStatusInfo = cJSON_CreateArray();
			cJSON_AddItemToObject(m_msgBody, "io", ioDetectorStatusInfo);
			for (i = 0; i < nRealIODetectorCount;i++)
			{
				cJSON* curIODetectorStatus = cJSON_CreateObject();
				if (curIODetectorStatus)
				{
					cJSON_AddNumberToObject(curIODetectorStatus, "id", chRealIODetectorID[i]);
					cJSON_AddNumberToObject(curIODetectorStatus, "state", chRealIODetectorStatus[i]);
					cJSON_AddItemToObject(ioDetectorStatusInfo, "", curIODetectorStatus);
				}
			}
		}
	}

	m_chOutBuff = cJSON_Print(m_msgBody);
    memcpy(m_chSendBuff + dwSendBuffLen, m_chOutBuff, strlen(m_chOutBuff));
    dwSendBuffLen += strlen(m_chOutBuff);

	m_pDataPackUnpackMode->PackBuffer(m_chSendBuff, dwSendBuffLen, m_chPackedBuff, dwPackedBuffSize);

	return m_clientSock.SendTo((char *)m_chPackedBuff, dwPackedBuffSize);
}
