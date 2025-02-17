/*=====================================================================
ģ���� ������������߳�ģ��
�ļ��� OpenATCCommWithCameraListenThread.h
����ļ���OpenATCDataPackUnpack.h OpenATCComDef.h
ʵ�ֹ��ܣ��������
���� ������Ƽ
��Ȩ ��<Copyright(c) 2019-2020 Suzhou Keda Technology Co.,Ltd. All right reserved.>
--------------------------------------------------------------------------------------------------------------------
�޸ļ�¼��
�� ��           �汾    �޸���             �߶���             �޸ļ�¼
2019/09/06      V1.0    ����Ƽ��           ����Ƽ             ����ģ��
====================================================================*/

#ifndef OPENATCCOMMWITHICAMERALISTENTHREAD_H
#define OPENATCCOMMWITHICAMERALISTENTHREAD_H

#include "../../Include/OpenATCParameter.h"
#include "../../Include/OpenATCRunStatus.h"
#include "../../Include/CanBusManager.h"
#include "OpenATCCameraDataPackUnpack.h"
#include "OpenATCCfgCommHelper.h"
#include "OpenATCComDef.h"
#include "OpenATCCommWithCameraThread.h"
#include "OpenATCCommWithCfgSWThread.h"

class COpenATCSocket;

#ifdef _WIN32
	#define COMMWITHCAMERALISTEN_CALLBACK WINAPI
	typedef HANDLE               COMMWITHCAMERALISTENHANDLE;
#else
	#define COMMWITHCAMERALISTEN_CALLBACK
	typedef pthread_t            COMMWITHCAMERALISTENHANDLE;
#endif

class COpenATCParameter;
class COpenATCRunStatus;

/*=====================================================================
���� ��COpenATCCommWithCameraListenThread
���� ���������
��Ҫ�ӿڣ�void Init����ʼ����������һ������Ϊ���ò������ڶ�������Ϊ״̬
��ע ��
--------------------------------------------------------------------------------------------------------------------
�޸ļ�¼��
�� ��           �汾    �޸���             �߶���             �޸ļ�¼
2019/09/06      V1.0    ����Ƽ             ����Ƽ             ������
====================================================================*/
class COpenATCCommWithCameraListenThread
{
public:
    COpenATCCommWithCameraListenThread();
    virtual ~COpenATCCommWithCameraListenThread();

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
	void        Init(COpenATCParameter *pParameter, COpenATCRunStatus *pRunStatus, COpenATCLog * pOpenATCLog, BYTE byServiceSource, char * pOpenATCVersion);

    int         Start();
    int         Join();
    int         Detach();


private:
	enum
	{
		SERVER_COM_PORT         = 5000,
		MAX_CAMERA_SIZE         = 8,
	};
    static void *COMMWITHCAMERALISTEN_CALLBACK RunThread(void *pParam);

	void            OpenATCSleep(long nMsec);

    COMMWITHCAMERALISTENHANDLE       m_hThread;
    unsigned long                    m_dwThreadRet;
	bool                             m_bExitFlag;
    int                              m_nDetachState;

	COpenATCParameter                *m_pOpenATCParameter;

	COpenATCRunStatus                *m_pOpenATCRunStatus;

	COpenATCLog                      *m_pOpenATCLog;

	COpenATCSocket                   m_acceptSock;

	COpenATCCommWithCameraThread     *m_pOpenATCCommWithCameraThread[MAX_CAMERA_SIZE];

	COpenATCCommWithCfgSWThread      *m_pOpenATCCommWithCfgThread;

	BYTE                             m_byServiceSource;

	char                             m_szClientIp[20];

	int                              m_nClientPort;

	char							 m_chOpenATCVersion[128];
          
};

#endif // !ifndef OPENATCCOMMWITHCAMERALISTENTHREAD_H
