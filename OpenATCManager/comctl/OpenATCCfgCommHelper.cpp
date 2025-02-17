/*====================================================================
模块名 ：和客户端配置软件交互的通信接口模块
文件名 ：OpenATCCfgCommHelper.cpp
相关文件：OpenATCCfgCommHelper.h
实现功能：和客户端配置进行交互
作者 ：李永萍
版权 ：<Copyright(C) 2019-2020 Suzhou Keda Technology Co., Ltd. All rights reserved.>
---------------------------------------------------------------------------------------------------------------------
修改记录：
日 期           版本    修改人             走读人             修改记录
2019/09/06      V1.0    李永萍             李永萍             创建模块
====================================================================*/

#include "OpenATCCfgCommHelper.h"

#ifndef _WIN32
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <net/if.h>
#endif

COpenATCCfgCommHelper::COpenATCCfgCommHelper()
{
	m_wBindPort  = 8880;
	m_bBindState = false;

	m_acceptSock.SetSocketType(SOCK_DGRAM);
    m_acceptSock.SetComType(UDP_SERVICE);

	m_nSendTimeOut = 100;
	m_nRecvTimeOut = 100;

	for (int i = 0;i < MAX_CONNECT_COUNT;i++)
	{
		memset(m_tPeerStaus[i].m_szPeerIp, 0, sizeof(m_tPeerStaus[i].m_szPeerIp));  
		m_tPeerStaus[i].m_nPeerPort		 = 0;
		m_tPeerStaus[i].m_bConnectStatus = false;
		m_tPeerStaus[i].m_lastReadOkTime = time(NULL);
	}
}

COpenATCCfgCommHelper::~COpenATCCfgCommHelper()
{

}

int COpenATCCfgCommHelper::AcceptConnection(char chNetCardIp[], unsigned short wPort, bool isCfgIP)
{
	//m_acceptSock.SetSocketType(SOCK_DGRAM);
    //m_acceptSock.SetComType(UDP_SERVICE);

	if (m_wBindPort != wPort || m_bBindState == false)
	{
		m_wBindPort = wPort;
	
		if (m_acceptSock.Bind("", m_wBindPort, isCfgIP) == OPENATC_RTN_OK)
		{
			m_acceptSock.Listen(100);
			m_bBindState = true;
            return OPENATC_RTN_OK;
		}
        else
        {
            m_bBindState = false;
            return OPENATC_RTN_FAILED;
        }
	}
    else
    {
        return OPENATC_RTN_OK;
    }
}

int COpenATCCfgCommHelper::Read(unsigned char *chRecvBuff, int nRecvBuffSize, int &nRecvSize, bool &bSysTimeReset, char * chPeerIp)
{
    if (bSysTimeReset)
    {
        bSysTimeReset = false;
        m_tPeerStaus[0].m_lastReadOkTime = time(NULL);
		m_tPeerStaus[1].m_lastReadOkTime = time(NULL);
    }
	int i = 0;
    char szPeerIp[20]	= {0};
	memset(szPeerIp, 0x00, sizeof(szPeerIp));
    int  nPeerPort		= 0;

    nRecvSize = 0;
    
    int nReadSize = m_acceptSock.RecvFrom((char *)chRecvBuff, nRecvBuffSize, m_nRecvTimeOut);
    if (nReadSize > 0)
    {
        m_acceptSock.GetDestAddr(szPeerIp, &nPeerPort);
		memcpy(chPeerIp, szPeerIp, strlen(szPeerIp));
		//第一个连接固定给中心使用，接收数据
		if (memcmp(m_tPeerStaus[0].m_szPeerIp, szPeerIp, strlen(szPeerIp)) == 0)
		{
            m_tPeerStaus[0].m_nPeerPort		 = nPeerPort;
            m_tPeerStaus[0].m_bConnectStatus = true;
			m_tPeerStaus[0].m_lastReadOkTime = time(NULL);
			nRecvSize = nReadSize;

			//m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "COpenATCCfgCommHelper  11111");
		}
		else 
		{
			//第二个连接还没有建立，接收数据
			if (!m_tPeerStaus[1].m_bConnectStatus)
			{
				memset(m_tPeerStaus[1].m_szPeerIp, 0, sizeof(m_tPeerStaus[1].m_szPeerIp));  
				strncpy(m_tPeerStaus[1].m_szPeerIp, szPeerIp, strlen(szPeerIp));
                m_tPeerStaus[1].m_nPeerPort		 = nPeerPort;
                m_tPeerStaus[1].m_bConnectStatus = true;
				m_tPeerStaus[1].m_lastReadOkTime = time(NULL);
				nRecvSize = nReadSize;

				//m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "COpenATCCfgCommHelper  2222 %s %s", szPeerIp, m_tPeerStaus[1].m_szPeerIp);
			}
			else//第二个连接建立
			{
				//第二个连接是网页端，接收数据
				if (strcmp(m_tPeerStaus[1].m_szPeerIp, "127.0.0.1") == 0)
				{
					//m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "COpenATCCfgCommHelper  3333 %s %s", szPeerIp, m_tPeerStaus[1].m_szPeerIp);

					memset(m_tPeerStaus[1].m_szPeerIp, 0, sizeof(m_tPeerStaus[1].m_szPeerIp)); 
					strncpy(m_tPeerStaus[1].m_szPeerIp, szPeerIp, strlen(szPeerIp));
					m_tPeerStaus[1].m_nPeerPort		 = nPeerPort;
					m_tPeerStaus[1].m_bConnectStatus = true;
					m_tPeerStaus[1].m_lastReadOkTime = time(NULL);
					nRecvSize = nReadSize;

					//m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "COpenATCCfgCommHelper  4444 %s %s", szPeerIp, m_tPeerStaus[1].m_szPeerIp);
				}
				else//第二个连接是CS端
				{
					//来的数据是同一个CS端，接收数据
					if (memcmp(m_tPeerStaus[1].m_szPeerIp, szPeerIp, strlen(szPeerIp)) == 0)
					{
						//m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "COpenATCCfgCommHelper  5555 %s %s", szPeerIp, m_tPeerStaus[1].m_szPeerIp);

						memset(m_tPeerStaus[1].m_szPeerIp, 0, sizeof(m_tPeerStaus[1].m_szPeerIp)); 
						strncpy(m_tPeerStaus[1].m_szPeerIp, szPeerIp, strlen(szPeerIp));
						m_tPeerStaus[1].m_nPeerPort		 = nPeerPort;
						m_tPeerStaus[1].m_bConnectStatus = true;
						m_tPeerStaus[1].m_lastReadOkTime = time(NULL);
						nRecvSize = nReadSize;

						//m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "COpenATCCfgCommHelper  6666 %s %s", szPeerIp, m_tPeerStaus[1].m_szPeerIp);
					}
					else
					{
						time_t curTime = time(NULL);
						if (labs((long)(curTime - m_tPeerStaus[1].m_lastReadOkTime)) > COMM_HEART_BEAT)
						{
							//m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "COpenATCCfgCommHelper  7777  %s %s", szPeerIp, m_tPeerStaus[1].m_szPeerIp);

							//超时，不管来的数据是CS端或者网页端，都接收数据
							memset(m_tPeerStaus[1].m_szPeerIp, 0, sizeof(m_tPeerStaus[1].m_szPeerIp)); 
							strncpy(m_tPeerStaus[1].m_szPeerIp, szPeerIp, strlen(szPeerIp));
							m_tPeerStaus[1].m_nPeerPort = nPeerPort;
							m_tPeerStaus[1].m_bConnectStatus = true;
							m_tPeerStaus[1].m_lastReadOkTime = time(NULL);
							nRecvSize = nReadSize;
							//m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "COpenATCCfgCommHelper  8888  %s %s", szPeerIp, m_tPeerStaus[1].m_szPeerIp);
						}
						else
						{
							//不超时不接收数据
							nRecvSize = 0;
							//m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "COpenATCCfgCommHelper  9999  %s %s", szPeerIp, m_tPeerStaus[1].m_szPeerIp);
						}
					}
				}
			}
		}

        return OPENATC_RTN_OK;
    }
    else if (nReadSize == 0)
    {
        m_acceptSock.GetDestAddr(szPeerIp, &nPeerPort);
		for (i = 0;i < MAX_CONNECT_COUNT;i++)
		{
			if (strcmp(m_tPeerStaus[i].m_szPeerIp, szPeerIp) == 0 && m_tPeerStaus[i].m_bConnectStatus)
			{
				m_tPeerStaus[i].m_bConnectStatus = false;
			}
		}
		nRecvSize = 0;
        return OPENATC_RTN_FAILED;
    }
    else
    {
        time_t curTime = time(NULL);
        m_acceptSock.GetDestAddr(szPeerIp, &nPeerPort);
		for (i = 0;i < MAX_CONNECT_COUNT;i++)
		{
			if (strcmp(m_tPeerStaus[i].m_szPeerIp, szPeerIp) == 0 && m_tPeerStaus[i].m_bConnectStatus)
			{
				if (labs((long)(curTime - m_tPeerStaus[i].m_lastReadOkTime)) > COMM_HEART_BEAT)
				{
					m_tPeerStaus[i].m_bConnectStatus = false;

					nRecvSize = -1;//UDP连接断开
				}
			}
		}

		return OPENATC_RTN_FAILED;
    }

	return OPENATC_RTN_OK;
}

int COpenATCCfgCommHelper::Write(unsigned char *chSendBuff, int nSendSize)
{
    if (m_acceptSock.SendTo((char *)chSendBuff, (long)nSendSize) != (long)nSendSize)
    {
        return OPENATC_RTN_FAILED;
    }

    return OPENATC_RTN_OK;
}

void COpenATCCfgCommHelper::Close()
{
    m_acceptSock.Close();

	#ifdef _WIN32
	WSACleanup();
	#endif

	m_bBindState = false;

	for (int i = 0;i < MAX_CONNECT_COUNT;i++)
	{
		memset(m_tPeerStaus[i].m_szPeerIp, 0, sizeof(m_tPeerStaus[i].m_szPeerIp));  
		m_tPeerStaus[i].m_nPeerPort		 = 0;
		m_tPeerStaus[i].m_bConnectStatus = false;
		m_tPeerStaus[i].m_lastReadOkTime = time(NULL);
	}
}

void  COpenATCCfgCommHelper::SetSendTimeOut(int nMSec)
{
	m_nSendTimeOut = nMSec;
}

void  COpenATCCfgCommHelper::SetRecvTimeOut(int nMSec)
{
	m_nRecvTimeOut = nMSec;
}

int COpenATCCfgCommHelper::GetIpAddr(const char *ifname, char *ip)
{
#ifndef _WIN32
    int    sock_get_ip;
    int    nRet = 0;
    char   ipaddr[48] = {0};
    struct sockaddr_in *sin;
    struct ifreq ifr;
    
    if ((sock_get_ip = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        printf("socket create failed!\r\n");
        return -1;
    }
    
    memset(&ifr, 0, sizeof(ifr));
    memcpy(ifr.ifr_name, ifname, strlen(ifname));
    
    if (ioctl(sock_get_ip, SIOCGIFADDR, &ifr) <0)
    {
        perror("ioctl error!\n");
        nRet = -2;
        goto FAILED;
    }
    
    sin = (struct sockaddr_in *)&ifr.ifr_addr;
    if (NULL != ip)
    { 
        strncpy(ip, inet_ntoa(sin->sin_addr), strlen(inet_ntoa(sin->sin_addr)));
    }
    
FAILED:
    close(sock_get_ip);
    return nRet;
#else
    return -1;
#endif
}
 
void COpenATCCfgCommHelper::PreSetCenterPeerIP(char *ip, COpenATCLog * pOpenATCLog)
{
	strcpy(m_tPeerStaus[0].m_szPeerIp, ip);
	m_pOpenATCLog = pOpenATCLog;
}