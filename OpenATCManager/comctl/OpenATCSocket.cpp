/*====================================================================
模块名 ：参数配置模块
文件名 ：OpenATCSocket.cpp
相关文件：OpenATCSocket.h
实现功能：信号机和配置软件的通信交互
作者 ：李永萍
版权 ：<Copyright(C) 2019-2020 Suzhou Keda Technology Co., Ltd. All rights reserved.>
---------------------------------------------------------------------------------------------------------------------
修改记录：
日 期           版本    修改人             走读人             修改记录
2019/09/06      V1.0    李永萍             李永萍             创建模块
====================================================================*/

#include "OpenATCSocket.h"

#define EINTR      4

COpenATCSocket::COpenATCSocket()
{
	#ifdef _WIN32
	WSADATA wsaData;
	WORD wVersionRequested = MAKEWORD(2, 2);
	WSAStartup(wVersionRequested, &wsaData);
    #endif

	m_socket		= SockInvalid;
	m_nSocketType	= SOCK_STREAM;
    m_nComType		= UDP_CLIENT;
	memset(&m_sockDest, 0, sizeof(m_sockDest));
	memset(&m_sockLocal, 0, sizeof(m_sockLocal));
}

COpenATCSocket::~COpenATCSocket()
{
	Close();
	#ifdef _WIN32
	WSACleanup();
	#endif
}

void  COpenATCSocket::SetSocketType(int nSocketType)
{
	int nRecvBufSize = RECVBUFFER_SIZE;
	int nSendBufSize = SENDBUFFER_SIZE;

	m_nSocketType = nSocketType;

	m_socket = socket(AF_INET, m_nSocketType, IPPROTO_IP);
	if (m_socket != SockInvalid)
	{
		setsockopt(m_socket, SOL_SOCKET, SO_RCVBUF, (char *)(&nRecvBufSize), sizeof(nRecvBufSize));
		setsockopt(m_socket, SOL_SOCKET, SO_SNDBUF, (char *)(&nSendBufSize), sizeof(nSendBufSize));
	}
}

void COpenATCSocket::SetComType(int nComType)
{
    m_nComType = nComType;
}

int COpenATCSocket::Connect(char *szDestAddr, int nDestPort, long nMSec)
{
	struct hostent *pHostent = NULL;
    struct sockaddr_in sin;
	int    nRet = 0;
	fd_set fdw, fdr;
	
	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_port   = htons((u_short)nDestPort);
	if (sin.sin_port == 0)
	{
		return OPENATC_RTN_FAILED;
	}

	pHostent = gethostbyname(szDestAddr);
	if (pHostent)
	{
		memcpy(&sin.sin_addr, pHostent->h_addr, pHostent->h_length);
	}
	else if ((sin.sin_addr.s_addr = inet_addr(szDestAddr))== INADDR_NONE)
	{
		return OPENATC_RTN_FAILED;
	}

	memcpy(&m_sockDest, &sin, sizeof(sin));	
	
    if (nMSec > 0)
	{
		SetNonBlockMode(1);
	}
	
	nRet = connect(m_socket, (struct sockaddr *)&m_sockDest, sizeof(m_sockDest));
	/*if (nRet != 0)
	{
		if (nMSec > 0)
		{   
			FD_ZERO(&fdw);
			FD_SET(m_socket, &fdw);
			fdr  = fdw;
			nRet = Select(&fdr, &fdw, NULL, nMSec);
			if (nRet <= 0)
			{
	    		SetNonBlockMode(0);
				#ifdef _WIN32
				closesocket(m_socket);
				#else
				close(m_socket);
				#endif
				m_socket = SockInvalid;
				return OPENATC_RTN_TIMEOUT;
			}
			else 
			{
				if (FD_ISSET(m_socket, &fdr) || !FD_ISSET(m_socket, &fdw))
				{
	                SetNonBlockMode(0);
					#ifdef _WIN32
					closesocket(m_socket);
					#else
					close(m_socket);
					#endif
					m_socket = SockInvalid;
					return OPENATC_RTN_FAILED;		
				}
			}
		}
		else
		{
			#ifdef _WIN32
			closesocket(m_socket);
			#else
			close(m_socket);
			#endif
			m_socket = SockInvalid;
		}
	}
    else
    { 
        #ifdef _WIN32
	    closesocket(m_socket);
	    #else
		close(m_socket);
		#endif
		m_socket = SockInvalid;
    }*/

	if (nMSec > 0)
	{
		SetNonBlockMode(0);
	}
	
	if (m_socket == SockInvalid) 
	{
		return OPENATC_RTN_FAILED;
	}
	else 
	{
		return OPENATC_RTN_OK;
	}
}

int COpenATCSocket::Bind(char *szLocalAddr, int nLocalPort, bool isCfgIP)
{
	char szPort[10] = {0};
	sprintf(szPort, "%d", nLocalPort);
	
	if (m_socket == SockInvalid)
	{
		return OPENATC_RTN_FAILED;
	}

    struct hostent *pHostent;
    struct sockaddr_in sin;
	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_port = htons((u_short)atoi(szPort));
	if (isCfgIP)
	{
		pHostent = gethostbyname(szLocalAddr);
		if (pHostent)
		{
			memcpy(&sin.sin_addr, pHostent->h_addr, pHostent->h_length);
		}
		else if ((sin.sin_addr.s_addr = inet_addr(szLocalAddr)) == INADDR_NONE)
		{
			sin.sin_addr.s_addr = INADDR_ANY;
		}
	}
	else
	{
		sin.sin_addr.s_addr = INADDR_ANY;
	}

	if (bind(m_socket,(struct sockaddr *)&sin, sizeof(sin)) == SockError)
	{
		return OPENATC_RTN_FAILED;
	}

	socklen_t namelen = sizeof(m_sockLocal);
	getsockname(m_socket, (SOCKADDR *)&m_sockLocal, &namelen);

	return OPENATC_RTN_OK;
}

int COpenATCSocket::Listen(int nBackLog)
{
	if (listen(m_socket, nBackLog) == 0)
	{
		return OPENATC_RTN_OK;
	}
	else
	{
		return OPENATC_RTN_FAILED;
	}	
}

int  COpenATCSocket::Accept(SOCKET & socket)
{
	SOCKET newsocket = SockInvalid;

	SetNonBlockMode(1);

	socklen_t addrlen = sizeof(struct sockaddr);
	newsocket = accept(m_socket, (SOCKADDR *)&m_sockDest, &addrlen);

	SetNonBlockMode(0);

	if (newsocket != SockInvalid)
	{
		socket = newsocket;
		return OPENATC_RTN_OK;	
	}
	else
	{
		return OPENATC_RTN_FAILED;
	}
}

int COpenATCSocket::Write(unsigned char *pData, int nLen, int nMSec)
{
	int nTempLen = 0, nSendLen = 0, nSelect = 0;
	fd_set fdw;
	
	while (nTempLen < nLen)
	{
		 if (nMSec > 0)
		 {
			 FD_ZERO(&fdw);
			 FD_SET(m_socket, &fdw);
			 nSelect = Select(NULL, &fdw, NULL, nMSec);
			 if (nSelect > 0)
			 {
			     if (m_socket == SockInvalid)
				 {
					 return OPENATC_RTN_FAILED;
				 }

				 #ifndef _WIN32
				 nSendLen = send(m_socket, (char *)pData + nTempLen, nLen - nTempLen, MSG_NOSIGNAL);
				 #else
				 nSendLen = send(m_socket, (char *)pData + nTempLen, nLen - nTempLen, 0);
				 #endif
			
		 		 if (nSendLen == -1)
				 {
					 return OPENATC_RTN_FAILED;
				 }
		 		 else if (nSendLen == 0) 
				 {
					 break;
				 }

		 		 nTempLen += nSendLen;
			 }
			 else
			 {
		         return nSelect;
			 }
		 }
		 else
		 {
			 if (m_socket == SockInvalid)
			 {
				 return OPENATC_RTN_FAILED;
			 }

			 #ifndef _WIN32
			 nSendLen = send(m_socket, (char *)pData + nTempLen, nLen - nTempLen, MSG_NOSIGNAL);
			 #else
			 nSendLen = send(m_socket, (char *)pData + nTempLen, nLen - nTempLen, 0);
			 #endif
			
		 	 if (nSendLen == -1)
			 {
				 return OPENATC_RTN_FAILED;
			 }
		 	 else if (nSendLen == 0) 
			 {
				 break;
			 }

		 	 nTempLen += nSendLen;
		 }
	}

	return nTempLen;
}

int COpenATCSocket::Read(char *pData, int nLen, int nMSec)
{
	int nTempLen = 0, nSelect = 0, nRecvLen = 0;
	fd_set  fdr;
	
	while (nTempLen < nLen)
	{
		if (nMSec > 0)
		{
		    FD_ZERO(&fdr);
			FD_SET(m_socket, &fdr);
			nSelect = Select(&fdr, NULL, NULL, nMSec);
			if (nSelect > 0)
			{ 
				if (m_socket == SockInvalid)
				{
					return OPENATC_RTN_FAILED;
				}

				nRecvLen = recv(m_socket, pData + nTempLen, nLen - nTempLen, 0);

				if (nRecvLen < 0)
				{
					 if (errno == EINTR)
					 {
						 nRecvLen = 0;
					 }
					 else 
					 {
						 return OPENATC_RTN_FAILED;
					 }
				}
				else if (nRecvLen == 0)
				{
					nTempLen = 0;
					break;
				}

				nTempLen += nRecvLen;
			}
			else
			{
				return nTempLen;
			}
		}
		else
		{
			if (m_socket == SockInvalid)
			{
				return OPENATC_RTN_FAILED;
			}

			nRecvLen = recv(m_socket, pData + nTempLen, nLen - nTempLen, 0);

			if (nRecvLen < 0)
			{
			     if (errno == EINTR)
				 {
					 nRecvLen = 0;
				 }
			     else 
				 {
					 return OPENATC_RTN_FAILED;
				 }
			}
			else if (nRecvLen == 0)
			{
			    nTempLen = 0;
			    break;
			}

			nTempLen += nRecvLen;
		}
	}

	return nTempLen;	
}

int COpenATCSocket::GetDestAddr(char *szAddr, int *nPort)
{
	sockaddr_in sin;
    if (m_nComType == UDP_CLIENT || m_nComType == TCP_SERVICE)
    {
	    memcpy(&sin, &m_sockDest, sizeof(sin));
    }
    else
    {
        memcpy(&sin, &m_sockRecvFrom, sizeof(sin));
    }
	sprintf(szAddr, "%s", inet_ntoa(sin.sin_addr));
	*nPort = ntohs(sin.sin_port);
	return OPENATC_RTN_OK;
}

int COpenATCSocket::SetNonBlockMode(unsigned long dwBlock)
{
	#ifdef _WIN32
	ioctlsocket(m_socket, FIONBIO, &dwBlock);
	#else
	ioctl(m_socket, FIONBIO, &dwBlock);
	#endif

	return OPENATC_RTN_OK;
}

int COpenATCSocket::Select(fd_set *readfds, fd_set *writefds, fd_set *exceptfds, long msec)
{
	struct timeval tv, *ptv = NULL;
	if (msec > 0)
	{
		tv.tv_sec = msec / 1000;
		tv.tv_usec = (msec % 1000) * 1000;
		ptv = &tv;
	}
	return select(m_socket + 1, readfds, writefds, exceptfds, ptv);
}

int COpenATCSocket::RecvFrom(char  *szData, int nLen, int nMSec)
{
	socklen_t slen = 0;
	int nRecvLen = 0;
	int nSelect = 0;

	fd_set fdr;

	if (m_socket == SockInvalid)
	{
		return OPENATC_RTN_FAILED;
	}

	if (nMSec <= 0) 
	{
		slen = sizeof(sockaddr);
        if (m_nComType == UDP_CLIENT)
        {
		    nRecvLen = recvfrom(m_socket, szData, nLen, 0, (struct sockaddr *)&m_sockDest, &slen);
        }
        else
        {
            nRecvLen = recvfrom(m_socket, szData, nLen, 0, (struct sockaddr *)&m_sockRecvFrom, &slen);
        }
		if (nRecvLen == SockError) 
		{
			return OPENATC_RTN_FAILED;
		}
		else 
		{
			return nRecvLen;
		}
	}

	FD_ZERO(&fdr);
	FD_SET(m_socket,&fdr);
	nSelect = Select(&fdr, NULL, NULL, nMSec);
	if (nSelect > 0)
	{
		slen = sizeof(sockaddr);
        if (m_nComType == UDP_CLIENT)
        {
		    nRecvLen = recvfrom(m_socket, szData, nLen, 0, (struct sockaddr *)&m_sockDest, &slen);
        }
        else
        {
            nRecvLen = recvfrom(m_socket, szData, nLen, 0, (struct sockaddr *)&m_sockRecvFrom, &slen);
        }
		if (nRecvLen == SockError)
		{
			return OPENATC_RTN_FAILED;
		}
		else 
		{
			return nRecvLen;
		}
	}
	else if (nSelect == 0)
	{
		return OPENATC_RTN_TIMEOUT;
	}
	else 
	{
		return OPENATC_RTN_FAILED;
	}
}

int COpenATCSocket::SendTo(char *szData, int nLen)
{
	if (m_socket == SockInvalid)
	{
		return OPENATC_RTN_FAILED;
	}

    int len = 0;
    if (m_nComType == UDP_CLIENT)
    {
        len = sendto(m_socket, szData, nLen, 0, (struct sockaddr *)&m_sockDest, sizeof(sockaddr));
    }
    else
    {
        len = sendto(m_socket, szData, nLen, 0, (struct sockaddr *)&m_sockRecvFrom, sizeof(sockaddr));
    }
	if (len == SockError) 
	{   
	    return OPENATC_RTN_FAILED;
	}
	else 
	{
		return len;
	}
}

void COpenATCSocket::Close()
{
	if (m_socket != SockInvalid)
	{
		#ifdef _WIN32
		closesocket(m_socket);
        #else
		close(m_socket);
        #endif
	}

	//m_socket = SockInvalid;
	//m_nSocketType = SOCK_STREAM;
    //m_nComType = UDP_CLIENT;
	memset(&m_sockDest, 0, sizeof(m_sockDest));
	memset(&m_sockLocal, 0, sizeof(m_sockLocal));
}	

void COpenATCSocket::SetSocket(SOCKET socket)
{
	m_socket = socket;
}