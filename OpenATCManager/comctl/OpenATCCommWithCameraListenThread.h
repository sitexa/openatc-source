/*=====================================================================
模块名 ：侦听相机的线程模块
文件名 OpenATCCommWithCameraListenThread.h
相关文件：OpenATCDataPackUnpack.h OpenATCComDef.h
实现功能：侦听相机
作者 ：李永萍
版权 ：<Copyright(c) 2019-2020 Suzhou Keda Technology Co.,Ltd. All right reserved.>
--------------------------------------------------------------------------------------------------------------------
修改记录：
日 期           版本    修改人             走读人             修改记录
2019/09/06      V1.0    李永萍）           李永萍             创建模块
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
类名 ：COpenATCCommWithCameraListenThread
功能 ：侦听相机
主要接口：void Init：初始化参数，第一个参数为配置参数，第二个参数为状态
备注 ：
--------------------------------------------------------------------------------------------------------------------
修改记录：
日 期           版本    修改人             走读人             修改记录
2019/09/06      V1.0    李永萍             李永萍             创建类
====================================================================*/
class COpenATCCommWithCameraListenThread
{
public:
    COpenATCCommWithCameraListenThread();
    virtual ~COpenATCCommWithCameraListenThread();

    virtual int Run();

	/****************************************************
	函数名：Init
    功能：初始化参数
	算法实现:
    参数说明 ： pParameter，参数
	            pRunStatus，状态
    返回值说明：无
    --------------------------------------------------------------------------------------------------------------------
	修改记录：
	日 期           版本    修改人             走读人             修改记录
	2019/09/06      V1.0    李永萍             李永萍             创建
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
