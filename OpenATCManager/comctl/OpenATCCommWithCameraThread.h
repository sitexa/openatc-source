/*=====================================================================
ģ���� ��������������߳�ģ��
�ļ��� OpenATCCommWithCameraThread.h
����ļ���OpenATCDataPackUnpack.h OpenATCComDef.h
ʵ�ֹ��ܣ�������Ľ���
���� ������Ƽ
��Ȩ ��<Copyright(c) 2019-2020 Suzhou Keda Technology Co.,Ltd. All right reserved.>
--------------------------------------------------------------------------------------------------------------------
�޸ļ�¼��
�� ��           �汾    �޸���             �߶���             �޸ļ�¼
2019/09/06      V1.0    ����Ƽ��           ����Ƽ             ����ģ��
====================================================================*/

#ifndef OPENATCCOMMWITHCAMERATHREAD_H
#define OPENATCCOMMWITHCAMERATHREAD_H

#include "../../Include/OpenATCParameter.h"
#include "../../Include/OpenATCRunStatus.h"
#include "../../Include/CanBusManager.h"
#include "OpenATCCameraDataPackUnpack.h"
#include "OpenATCCfgCommHelper.h"
#include "OpenATCComDef.h"

class COpenATCSocket;

#ifdef _WIN32
	#define COMMWITHCAMERA_CALLBACK WINAPI
	typedef HANDLE               COMMWITHCAMERAHANDLE;
#else
	#define COMMWITHCAMERA_CALLBACK
	typedef pthread_t            COMMWITHCAMERAHANDLE;
#endif

class COpenATCParameter;
class COpenATCRunStatus;

/*=====================================================================
���� ��COpenATCCommWithCameraThread
���� ��������Ľ���
��Ҫ�ӿڣ�void Init����ʼ����������һ������Ϊ���ò������ڶ�������Ϊ״̬
��ע ��
--------------------------------------------------------------------------------------------------------------------
�޸ļ�¼��
�� ��           �汾    �޸���             �߶���             �޸ļ�¼
2019/09/06      V1.0    ����Ƽ             ����Ƽ             ������
====================================================================*/
class COpenATCCommWithCameraThread
{
public:
    COpenATCCommWithCameraThread();
    virtual ~COpenATCCommWithCameraThread();

    virtual int Run();

	/****************************************************
	��������Init
    ���ܣ���ʼ������
	�㷨ʵ��:
    ����˵�� �� pParameter������
	            pRunStatus��״̬
    ����ֵ˵������
    --------------------------------------------------------------------------------------------------------------------
	�޸ļ�¼��
	�� ��           �汾    �޸���             �߶���             �޸ļ�¼
	2019/09/06      V1.0    ����Ƽ             ����Ƽ             ����
	====================================================================*/
	void        Init(COpenATCParameter *pParameter, COpenATCRunStatus *pRunStatus, COpenATCLog * pOpenATCLog);

    int         Start();
    int         Join();
    int         Detach();

	/****************************************************
	��������SetClientSocket
    ���ܣ����������Ϣ
	�㷨ʵ��:
    ����˵�� �� clientSock�����socket
    ����ֵ˵������
    --------------------------------------------------------------------------------------------------------------------
	�޸ļ�¼��
	�� ��           �汾    �޸���             �߶���             �޸ļ�¼
	2019/09/06      V1.0    ����Ƽ             ����Ƽ             ����
	====================================================================*/
	void        SetClientSocket(SOCKET & socket);

	/****************************************************
	��������SetCameraInfo
    ���ܣ����������Ϣ
	�㷨ʵ��:
    ����˵�� �� chCameraIp�����IP
	            nCameraPort������˿�
    ����ֵ˵������
    --------------------------------------------------------------------------------------------------------------------
	�޸ļ�¼��
	�� ��           �汾    �޸���             �߶���             �޸ļ�¼
	2019/09/06      V1.0    ����Ƽ             ����Ƽ             ����
	====================================================================*/
	void        SetCameraInfo(char *chCameraIp, int nCameraPort);

	/****************************************************
	��������GetCameraInfo
    ���ܣ����������Ϣ
	�㷨ʵ��:
    ����˵�� �� ��
    ����ֵ˵�������IP
    --------------------------------------------------------------------------------------------------------------------
	�޸ļ�¼��
	�� ��           �汾    �޸���             �߶���             �޸ļ�¼
	2019/09/06      V1.0    ����Ƽ             ����Ƽ             ����
	====================================================================*/
	char *     GetCameraInfo();

private:
	enum
	{
		MAX_HEARTDATA_SIZE      = 4,
		MAX_LAMPCLRDATA_SIZE    = 16,
		MAX_DATA_SIZE           = 19,

		HEART_INTERVAL_TIME     = 5,

		PACK_HEART_HEAD         = 0xfd,
		PACK_LAMPCLR_HEAD       = 0xfe,
		PACK_TAIL               = 0x55,

		MAX_HEART_INDEX         = 255,
	};
    static void *COMMWITHCAMERA_CALLBACK RunThread(void *pParam);

	void            SendLampClrDataToCamera();

	void            SendHeartToCamera();

	void            OpenATCSleep(long nMsec);

	//CANͨ�ŷ��͵ĵ���״̬�ֽ�ɺ�ƻƵ��̵�״̬
	void			GetRYGStatusByGroup(char chGroup,char & chR,char & chY,char & chG);

    COMMWITHCAMERAHANDLE             m_hThread;
    unsigned long                    m_dwThreadRet;
	bool                             m_bExitFlag;
    int                              m_nDetachState;

	COpenATCParameter                *m_pOpenATCParameter;

	COpenATCRunStatus                *m_pOpenATCRunStatus;

	COpenATCLog                      *m_pOpenATCLog;

	int                              m_nSendTimeOut;

	int                              m_nRecvTimeOut;
    
	COpenATCPackUnpackBase           *m_pDataPackUnpackMode;

	//��socket�н��յ����ݻ�����.
	unsigned char                    m_chRecvBuff[MAX_HEARTDATA_SIZE + 1];

	//�����Э�����ݵĻ�����.
	unsigned char                    m_chUnPackedBuff[MAX_HEARTDATA_SIZE + 1];

	unsigned char                    m_chOldSendBuff[MAX_LAMPCLRDATA_SIZE + 1];

	short                            m_sCameraCmdType;

	char	                         m_szCameraIp[20];

    int                              m_nCameraPort;

	bool                             m_bConnectStatus;

	unsigned char                    m_chHeartIndex;

	time_t                           m_lastSendHeartTime;

	time_t                           m_lastReadOkTime;

	COpenATCSocket                   m_clientSock;
          
};

#endif // !ifndef OPENATCCOMMWITHCAMERATHREAD_H
