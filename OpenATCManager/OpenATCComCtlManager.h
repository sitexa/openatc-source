/*=====================================================================
模块名 ：通信控制管理模块
文件名 ：OpenATCComCtlManager.h
相关文件：
实现功能：通信控制模块整体调度类，用于与配置工具及平台信息交互。
作者 ：刘黎明
版权 ：<Copyright(c) 2019-2020 Suzhou Keda Technology Co.,Ltd. All right reserved.>
------------------------------------------------------------------------
修改记录：
日 期           版本     修改人     走读人     修改记录
2019/9/26       V1.0     刘黎明     王 五      创建模块
=====================================================================*/

#ifndef OPENATCCOMCTLMANAGER_H
#define OPENATCCOMCTLMANAGER_H

#include "../Include/OpenATCParameter.h"
#include "../Include/OpenATCRunStatus.h"
#include "../Include/OpenATCRunStatusDefine.h"
#include "../Include/OpenATCLog.h"

#include "comctl/OpenATCCommWithCenterThread.h"
#include "comctl/OpenATCCommWithCfgSWThread.h"
#include "comctl/OpenATCCommWithITS300Thread.h"
#include "comctl/OpenATCCommWithCameraListenThread.h"
#include "comctl/OpenATCCommWithGB20999Thread.h"

/*=====================================================================
类名 ：COpenATCComCtlManager
功能 ：通信控制模块整体调度类，用于与配置工具及平台信息交互。
主要接口：
备注 ：
-----------------------------------------------------------------------
修改记录：
日 期          版本     修改人     走读人       修改记录
2019/09/14     V1.0     刘黎明     李四         创建类
=====================================================================*/
class COpenATCComCtlManager  
{
public:
    //类定义为单件
    static COpenATCComCtlManager * getInstance();

    //类的初始化操作
    void Init(COpenATCParameter * pParameter,COpenATCRunStatus * pRunStatus, COpenATCLog * pOpenATCLog, const char * pOpenATCVersion);

    //类的停止与释放
    void Stop();

private:
	COpenATCComCtlManager();
	~COpenATCComCtlManager();

private:
    static COpenATCComCtlManager * s_pData;				//单件类指针

    COpenATCParameter * m_pLogicCtlParam;               //特征参数类指针

    COpenATCRunStatus * m_pLogicCtlStatus;              //运行状态类指针

	COpenATCLog       * m_pOpenATCLog;                  //日志类指针

public:
	COpenATCCommWithCfgSWThread   * m_openATCCommWithCfgSWThread;
	COpenATCCommWithCenterThread  * m_openATCCommWithCenterThread;
	COpenATCCommWithITS300Thread  * m_openATCCommWithITS300Thread;
	COpenATCCommWithCameraListenThread  * m_openATCCommWithCameraListenThread;
	COpenATCCommWithCameraListenThread  * m_openATCCommWithCfgListenThread;
	COpenATCCommWithCenterThread  * m_openATCCommWithSimulateThread;
    COpenATCCommWithITS300Thread  * m_openATCCommWithDetectorThread;
    COpenATCCommWithGB20999Thread* m_openATCCommWithGB20999Thread;
};

#endif // !ifndef OPENATCCOMCTLMANAGER_H


/*=====================================================================

该程序是一个C++头文件，定义了一个通信控制管理模块的类 `COpenATCComCtlManager`。以下是对该程序的分析：

### 1. 模块概述
- **模块名**：通信控制管理模块
- **功能**：该模块主要用于与配置工具及平台信息进行交互，负责整体调度。

### 2. 主要内容
- **包含的头文件**：程序包含了一些其他的头文件，这些头文件定义了参数、运行状态、日志等相关功能。
- **类定义**：`COpenATCComCtlManager` 类是该模块的核心，采用单例模式（Singleton Pattern），确保在整个程序中只有一个实例。

### 3. 类的成员
- **公有成员函数**：
  - `static COpenATCComCtlManager * getInstance();`：获取单例实例。
  - `void Init(...)`：初始化类的操作，接受多个参数以设置状态和日志。
  - `void Stop();`：停止和释放类的资源。

- **私有成员**：
  - 构造函数和析构函数：确保外部无法直接创建或销毁类的实例。
  - 一些指针成员变量，用于指向特征参数、运行状态和日志类的实例。

- **公有成员变量**：多个线程类的指针，负责与不同的系统组件进行通信。

### 4. 设计模式
- **单例模式**：通过 `getInstance()` 方法实现，确保类的唯一性。

### 5. 代码风格
- 代码结构清晰，注释详细，便于理解。
- 使用了有意义的变量和函数名称，符合代码可读性原则。

### 6. 可能的改进
- **错误处理**：在初始化和停止过程中，可能需要添加错误处理机制，以确保程序的健壮性。
- **文档注释**：可以考虑在函数实现中添加更多的中文注释，以便于后续维护。

总体来说，该程序结构合理，功能明确，适合用于通信控制管理的场景。
=====================================================================*/