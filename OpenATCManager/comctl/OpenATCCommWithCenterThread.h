/*=====================================================================
ģ���� �������Ľ������߳�ģ��
�ļ��� ��OpenATCCommWithCenterThread.h
����ļ���OpenATCDataPackUnpack.h OpenATCComDef.h
ʵ�ֹ��ܣ������ĵĽ���
���� ������Ƽ
��Ȩ ��<Copyright(c) 2019-2020 Suzhou Keda Technology Co.,Ltd. All right reserved.>
--------------------------------------------------------------------------------------------------------------------
�޸ļ�¼��
�� ��           �汾    �޸���             �߶���             �޸ļ�¼
2019/09/06      V1.0    ����Ƽ��           ����Ƽ             ����ģ��
====================================================================*/

#ifndef OPENATCCOMMWITHCENTERTHREAD_H
#define OPENATCCOMMWITHCENTERTHREAD_H

#include "../../Include/OpenATCParameter.h"
#include "../../Include/OpenATCRunStatus.h"
#include "OpenATCDataPackUnpack.h"
#include "OpenATCComWithControlCenterImpl.h"
#include "OpenATCSocket.h"
#include "OpenATCComDef.h"
#include "OpenATCCenterCommHandlerBase.h"

#ifdef _WIN32
	#define COMMWITHCENTER_CALLBACK WINAPI
	typedef HANDLE               COMMWITHCENTERHANDLE;
#else
	#define COMMWITHCENTER_CALLBACK
	typedef pthread_t            COMMWITHCENTERHANDLE;
#endif

class COpenATCParameter;
class COpenATCDataPackUnpack;
class COpenATCRunStatus;

/*=====================================================================
���� ��COpenATCCommWithCenterThread
���� ���Ϳͻ�����������Ľ���
��Ҫ�ӿڣ�void Init����ʼ����������һ������Ϊ���ò������ڶ�������Ϊ״̬
��ע ��
--------------------------------------------------------------------------------------------------------------------
�޸ļ�¼��
�� ��           �汾    �޸���             �߶���             �޸ļ�¼
2019/09/06      V1.0    ����Ƽ             ����Ƽ             ������
====================================================================*/
class COpenATCCommWithCenterThread
{
public:
    COpenATCCommWithCenterThread();
    virtual ~COpenATCCommWithCenterThread();

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
	void        Init(COpenATCParameter *pParameter, COpenATCRunStatus *pRunStatus, COpenATCLog * pOpenATCLog, int nComType, const char * pOpenATCVersion);

    int         Start();
    int         Join();
    int         Detach();


private:
    static void *COMMWITHCENTER_CALLBACK RunThread(void *pParam);

	void OpenATCSleep(long nMsec);

    COMMWITHCENTERHANDLE             m_hThread;
    unsigned long                    m_dwThreadRet;
	bool                             m_bExitFlag;
    int                              m_nDetachState;

	COpenATCParameter                *m_pOpenATCParameter;

	COpenATCRunStatus                *m_pOpenATCRunStatus;

	COpenATCLog                      *m_pOpenATCLog;

	COpenATCCenterCommHandlerBase    *m_centerHandler;

	int                              m_nComType;

	char							 m_chOpenATCVersion[128];
};

#endif // !ifndef OPENATCCOMMWITHCENTERTHREAD_H
