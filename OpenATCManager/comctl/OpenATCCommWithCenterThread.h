/*=====================================================================
模块名 ：和中心交互的线程模块
文件名 ：OpenATCCommWithCenterThread.h
相关文件：OpenATCDataPackUnpack.h OpenATCComDef.h
实现功能：和中心的交互
作者 ：李永萍
版权 ：<Copyright(c) 2019-2020 Suzhou Keda Technology Co.,Ltd. All right reserved.>
--------------------------------------------------------------------------------------------------------------------
修改记录：
日 期           版本    修改人             走读人             修改记录
2019/09/06      V1.0    李永萍）           李永萍             创建模块
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
类名 ：COpenATCCommWithCenterThread
功能 ：和客户端配置软件的交互
主要接口：void Init：初始化参数，第一个参数为配置参数，第二个参数为状态
备注 ：
--------------------------------------------------------------------------------------------------------------------
修改记录：
日 期           版本    修改人             走读人             修改记录
2019/09/06      V1.0    李永萍             李永萍             创建类
====================================================================*/
class COpenATCCommWithCenterThread
{
public:
    COpenATCCommWithCenterThread();
    virtual ~COpenATCCommWithCenterThread();

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
