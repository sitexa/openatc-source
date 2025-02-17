/*=====================================================================
模块名 ：和ITS300交互的线程模块
文件名 OpenATCCommWithITS300Thread.h
相关文件：OpenATCDataPackUnpack.h OpenATCComDef.h
实现功能：和ITS300的交互
作者 ：李永萍
版权 ：<Copyright(c) 2019-2020 Suzhou Keda Technology Co.,Ltd. All right reserved.>
--------------------------------------------------------------------------------------------------------------------
修改记录：
日 期           版本    修改人             走读人             修改记录
2019/09/06      V1.0    李永萍             李永萍             创建模块
====================================================================*/

#ifndef OPENATCCOMMWITHITS300THREAD_H
#define OPENATCCOMMWITHITS300THREAD_H

#include "../../Include/OpenATCParameter.h"
#include "../../Include/OpenATCRunStatus.h"
#include "../../Include/CanBusManager.h"
#include "OpenATCComDef.h"
#include "OpenATCITS300DataPackUnpack.h"
#include "OpenATCSocket.h"
#include "OpenATCCfgCommHelper.h"

class COpenATCSocket;

#ifdef _WIN32
	#define COMMWITHITS300_CALLBACK WINAPI
	typedef HANDLE               COMMWITHITS300HANDLE;
#else
	#define COMMWITHITS300_CALLBACK
	typedef pthread_t            COMMWITHITS300HANDLE;
#endif

class COpenATCParameter;
class COpenATCRunStatus;

const int C_N_DETECTOR_LOW   = 4;
const int C_N_DETECTOR_HIGH  = 5;
const int C_N_DETECTOR_COUNT = 19;

const int C_N_DETECTOR_INDEX = 19;
const int C_N_DETECTOR_VALUE = 20;

const int C_N_QUEUE_LENGTH      = 19;
const int C_N_PEDDETECTOR_COUNT = 19;

const int C_N_PREEMPT_INDEX     = 19;

/*=====================================================================
类名 ：COpenATCCommWithITS300Thread
功能 ：和客户端配置软件的交互
主要接口：void Init：初始化参数，第一个参数为配置参数，第二个参数为状态
备注 ：
--------------------------------------------------------------------------------------------------------------------
修改记录：
日 期           版本    修改人             走读人             修改记录
2019/09/06      V1.0    李永萍             李永萍             创建类
====================================================================*/
class COpenATCCommWithITS300Thread
{
public:
    COpenATCCommWithITS300Thread();
    virtual ~COpenATCCommWithITS300Thread();

    virtual int Run();

	/****************************************************
	函数名：Init
    功能：初始化参数
	算法实现:
    参数说明 ： pParameter，参数
	            pRunStatus，状态
				nComType，通信类型
    返回值说明：无
    --------------------------------------------------------------------------------------------------------------------
	修改记录：
	日 期           版本    修改人             走读人             修改记录
	2019/09/06      V1.0    李永萍             李永萍             创建
	====================================================================*/
	void        Init(COpenATCParameter *pParameter, COpenATCRunStatus *pRunStatus, COpenATCLog * pOpenATCLog, int nComType);

    int         Start();
    int         Join();
    int         Detach();


private:
	enum
	{
		RECV_BUFFER_SIZE		= 1024,
		UNPACKED_BUFFER_SIZE	= 1024,  
		SEND_BUFFER_SIZE        = 1024,
		PACKED_BUFFER_SIZE      = 1024,

		HEART_INTERVAL_TIME     = 15,
	};
    static void *COMMWITHITS300_CALLBACK RunThread(void *pParam);

	void OpenATCSleep(long nMsec);

    int  ParserPack(const unsigned char *chUnPackedBuff, unsigned int dwPackLength, char * chPeerIp);

    int  ParserPack_CtrlLink(const unsigned char *chUnPackedBuff, unsigned int dwPackLength, char * chPeerIp);

	int  ParserPack_BaseInfoLink(const unsigned char *chUnPackedBuff, unsigned int dwPackLength);

	int  SendAckToPeer(int nPackSize);

	void SetTrafficFlowInfo();

	void SetRealDetectorInfo();

	void SetVehicleQueueUpInfo();

	void SetPedDetectInfo();

	void SetDetectorFaultInfo();

	void SetPreemptInfo();

	int  AckCtl_AskHeart();

	char GetBit(unsigned int nInput, char chNum); //从一个值中取出任意位

    COMMWITHITS300HANDLE             m_hThread;
    unsigned long                    m_dwThreadRet;
	bool                             m_bExitFlag;
    int                              m_nDetachState;

	COpenATCParameter                *m_pOpenATCParameter;

	COpenATCRunStatus                *m_pOpenATCRunStatus;

	COpenATCLog                      *m_pOpenATCLog;

	COpenATCCfgCommHelper			  m_commHelper;

	COpenATCPackUnpackBase   *		  m_pDataPackUnpackMode;

	unsigned char            *		  m_chRecvBuff;

	unsigned char            *        m_chSendBuff;

	unsigned char            *        m_chPackedBuff;

	unsigned char            *		  m_chUnPackedBuff;

	unsigned int                      m_unMasterCount;

	int                               m_nVehicleCount[C_N_MAXDETBOARD_NUM][C_N_MAXDETINPUT_NUM];

	TAscArea                          m_tAreaInfo;

	long                              m_nHeartLastTime;

	int                               m_nComType;

	int                               m_nPort;                     
};

#endif // !ifndef OPENATCCOMMWITHITS300THREAD_H
