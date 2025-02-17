/*=====================================================================
模块名 ：通信控制管理模块
文件名 ：OpenATCComCtlManager.cpp
相关文件：OpenATCComCtlManager.h
          OpenATCParameter.h
          OpenATCRunStatus.h
实现功能：用于与配置工具及平台信息交互
作者 ：刘黎明
版权 ：<Copyright(c) 2019-2020 Suzhou Keda Technology Co.,Ltd. All right reserved.>
-----------------------------------------------------------------------
修改记录：
日 期           版本     修改人     走读人     修改记录
2019/9/14       V1.0     刘黎明     刘黎明      创建模块
=====================================================================*/

#include "OpenATCComCtlManager.h"

COpenATCComCtlManager * COpenATCComCtlManager::s_pData;

COpenATCComCtlManager::COpenATCComCtlManager()
{
	m_openATCCommWithCfgSWThread	= new COpenATCCommWithCfgSWThread();
	m_openATCCommWithCenterThread	= new COpenATCCommWithCenterThread();
	m_openATCCommWithITS300Thread	= new COpenATCCommWithITS300Thread();
	//m_openATCCommWithCameraListenThread = new COpenATCCommWithCameraListenThread();
	//m_openATCCommWithCfgListenThread    = new COpenATCCommWithCameraListenThread();
	m_openATCCommWithGB20999Thread = new COpenATCCommWithGB20999Thread();
	m_pLogicCtlStatus = NULL;
	m_pLogicCtlParam  = NULL;
	m_pOpenATCLog	  = NULL;

	m_openATCCommWithSimulateThread = new COpenATCCommWithCenterThread();
    m_openATCCommWithDetectorThread = new COpenATCCommWithITS300Thread();

}

COpenATCComCtlManager::~COpenATCComCtlManager()
{

}

/*==================================================================== 
函数名 ：getInstance 
功能 ：返回单件类COpenATCComCtlManager的实例指针 
算法实现 ： 
参数说明 ： 
返回值说明：单件类COpenATCComCtlManager的实例指针
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 刘黎明          创建函数 
====================================================================*/ 
COpenATCComCtlManager * COpenATCComCtlManager::getInstance()
{
    if (s_pData == NULL)
    {
        s_pData = new COpenATCComCtlManager();
    }

    return s_pData;
}

/*==================================================================== 
函数名 ：Init 
功能 ：用于对类COpenATCComCtlManager的初始化操作 
算法实现 ： 
参数说明 ： pParameter，特征参数类指针
            pRunStatus，运行状态类指针
返回值说明：无
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 刘黎明          创建函数 
====================================================================*/ 
void COpenATCComCtlManager::Init(COpenATCParameter * pParameter,COpenATCRunStatus * pRunStatus, COpenATCLog * pOpenATCLog, const char * pOpenATCVersion)
{
    if (pParameter != NULL)
    {
        m_pLogicCtlParam = pParameter;
    }

    if (pRunStatus != NULL)
    {
        m_pLogicCtlStatus = pRunStatus;
    }

	if (pOpenATCLog != NULL)
	{
		m_pOpenATCLog = pOpenATCLog;
	}

	//获取Comm选择
	int iComm = 0;
	iComm = m_pLogicCtlStatus->GetCommFlagStatus();
	switch (iComm)
	{
	case 1:
		m_openATCCommWithCfgSWThread->Init(pParameter, pRunStatus, pOpenATCLog, UDP_SERVICE, 0, pOpenATCVersion);
		m_openATCCommWithCfgSWThread->Start();
		delete m_openATCCommWithGB20999Thread;
		break;
	case 2:
		m_openATCCommWithGB20999Thread->Init(pParameter, pRunStatus, pOpenATCLog);
		m_openATCCommWithGB20999Thread->Start();
		delete m_openATCCommWithCfgSWThread;
		break;
	//case 3:
	//	//预留NTCIP！
	//	break;
	default:
		//默认25280
		m_openATCCommWithCfgSWThread->Init(pParameter, pRunStatus, pOpenATCLog, UDP_SERVICE, 0, pOpenATCVersion);
		m_openATCCommWithCfgSWThread->Start();
		delete m_openATCCommWithGB20999Thread;
		break;
	}
	m_openATCCommWithCenterThread->Init(pParameter, pRunStatus, pOpenATCLog, COM_WITH_CENTER, pOpenATCVersion);
	m_openATCCommWithCenterThread->Start();

	m_openATCCommWithITS300Thread->Init(pParameter, pRunStatus, pOpenATCLog, COM_WITH_ITS300);
	m_openATCCommWithITS300Thread->Start();

	//m_openATCCommWithCameraListenThread->Init(pParameter, pRunStatus, pOpenATCLog, CAMERA_SERVICE_LISTERN);
	//m_openATCCommWithCameraListenThread->Start();

	//m_openATCCommWithCfgListenThread->Init(pParameter, pRunStatus, pOpenATCLog, CFG_SERVICE_LISTERN);
	//m_openATCCommWithCfgListenThread->Start();

	TAscSimulate tSimulateInfo;
    m_pLogicCtlParam->GetSimulateInfo(tSimulateInfo);

	if (tSimulateInfo.m_nSimulatePort != 0)
	{
		//仿真接口没有配置时，通信线程则不需要开启
		m_openATCCommWithSimulateThread->Init(pParameter, pRunStatus, m_pOpenATCLog, COM_WITH_SIMULATE, pOpenATCVersion);
		m_openATCCommWithSimulateThread->Start();
	}

	m_openATCCommWithDetectorThread->Init(pParameter, pRunStatus, pOpenATCLog, COM_WITH_SIMULATE);
	m_openATCCommWithDetectorThread->Start();

    m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "COpenATCComCtlManager Init!"); 
}

void COpenATCComCtlManager::Stop()
{
	//获取Comm选择
	int iComm = 0;
	iComm = m_pLogicCtlStatus->GetCommFlagStatus();
	switch (iComm)
	{
	case 1:
		m_openATCCommWithCfgSWThread->Detach();
		m_openATCCommWithCfgSWThread->Join();
		break;
	case 2:
		m_openATCCommWithGB20999Thread->Detach();
		m_openATCCommWithGB20999Thread->Join();
		break;
	//case 3:
	//	//预留NTCIP！
	//	break;
	default:
		//默认25280
		m_openATCCommWithCfgSWThread->Detach();
		m_openATCCommWithCfgSWThread->Join();
		break;
	}

	m_openATCCommWithCenterThread->Detach();
	m_openATCCommWithCenterThread->Join();

	m_openATCCommWithITS300Thread->Detach();
	m_openATCCommWithITS300Thread->Join();

	//m_openATCCommWithCameraListenThread->Detach();
	//m_openATCCommWithCameraListenThread->Join();

	//m_openATCCommWithCfgListenThread->Detach();
	//m_openATCCommWithCfgListenThread->Join();

	TAscSimulate tSimulateInfo;
    m_pLogicCtlParam->GetSimulateInfo(tSimulateInfo);

	if (tSimulateInfo.m_nSimulatePort != 0)
	{
		m_openATCCommWithSimulateThread->Detach();
		m_openATCCommWithSimulateThread->Join();
	}

	m_openATCCommWithDetectorThread->Detach();
	m_openATCCommWithDetectorThread->Join();
}

/*=====================================================================
该程序是一个通信控制管理模块的实现，主要用于与配置工具及平台信息进行交互。以下是对程序的分析：

### 主要功能
1. **单例模式**：`COpenATCComCtlManager` 类实现了单例模式，通过 `getInstance` 方法返回该类的唯一实例。
2. **初始化功能**：`Init` 方法用于初始化通信线程，接收特征参数、运行状态和日志对象，并根据通信选择启动相应的线程。
3. **停止功能**：`Stop` 方法用于停止所有通信线程，确保在关闭时正确地分离和加入线程。

### 关键组件
- **线程管理**：程序中创建了多个线程对象，如 `COpenATCCommWithCfgSWThread`、`COpenATCCommWithCenterThread` 等，负责不同的通信任务。
- **条件判断**：通过 `GetCommFlagStatus` 方法获取当前的通信选择，并根据不同的选择初始化或停止相应的线程。
- **日志记录**：在初始化完成后，使用 `LogOneMessage` 方法记录初始化信息，便于后续的调试和监控。

### 代码结构
- **构造函数**：在构造函数中初始化所有线程对象，并将指针设置为 `NULL`。
- **析构函数**：析构函数为空，可能需要在未来实现资源释放。
- **注释**：代码中包含了中文注释，说明了每个函数的功能和修改记录，便于理解和维护。

### 改进建议
1. **错误处理**：在创建线程和初始化过程中，缺少错误处理机制，建议添加相应的异常处理。
2. **资源管理**：在析构函数中应释放动态分配的内存，避免内存泄漏。
3. **代码可读性**：可以考虑将一些复杂的逻辑拆分成更小的函数，以提高代码的可读性和可维护性。

总体来说，该程序实现了基本的通信控制管理功能，但在错误处理和资源管理方面还有提升的空间。

====================================================================*/

