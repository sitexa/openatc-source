/*=====================================================================
ģ���� ����ITS300�������߳�ģ��
�ļ��� OpenATCCommWithITS300Thread.h
����ļ���OpenATCDataPackUnpack.h OpenATCComDef.h
ʵ�ֹ��ܣ���ITS300�Ľ���
���� ������Ƽ
��Ȩ ��<Copyright(c) 2019-2020 Suzhou Keda Technology Co.,Ltd. All right reserved.>
--------------------------------------------------------------------------------------------------------------------
�޸ļ�¼��
�� ��           �汾    �޸���             �߶���             �޸ļ�¼
2019/09/06      V1.0    ����Ƽ             ����Ƽ             ����ģ��
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
���� ��COpenATCCommWithITS300Thread
���� ���Ϳͻ�����������Ľ���
��Ҫ�ӿڣ�void Init����ʼ����������һ������Ϊ���ò������ڶ�������Ϊ״̬
��ע ��
--------------------------------------------------------------------------------------------------------------------
�޸ļ�¼��
�� ��           �汾    �޸���             �߶���             �޸ļ�¼
2019/09/06      V1.0    ����Ƽ             ����Ƽ             ������
====================================================================*/
class COpenATCCommWithITS300Thread
{
public:
    COpenATCCommWithITS300Thread();
    virtual ~COpenATCCommWithITS300Thread();

    virtual int Run();

	/****************************************************
	��������Init
    ���ܣ���ʼ������
	�㷨ʵ��:
    ����˵�� �� pParameter������
	            pRunStatus��״̬
				nComType��ͨ������
    ����ֵ˵������
    --------------------------------------------------------------------------------------------------------------------
	�޸ļ�¼��
	�� ��           �汾    �޸���             �߶���             �޸ļ�¼
	2019/09/06      V1.0    ����Ƽ             ����Ƽ             ����
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

	char GetBit(unsigned int nInput, char chNum); //��һ��ֵ��ȡ������λ

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
