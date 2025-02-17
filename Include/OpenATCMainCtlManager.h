/*=====================================================================
模块名 ：系统运行主调度模块
文件名 ：OpenATCMainCtlManager.h
相关文件：OpenATCParameter.h OpenATCRunStatus.h
实现功能：主控模块整体调度类，用于管理系统的参数，运行状态。
作者 ：刘黎明
版权 ：<Copyright(c) 2019-2020 Suzhou Keda Technology Co.,Ltd. All right reserved.>
----------------------------------------------------------------------
修改记录：
日 期           版本     修改人     走读人     修改记录
2019/9/14       V1.0     刘黎明     王 五      创建模块
=====================================================================*/

#ifndef OPENATCMAINCTLMANAGER_H
#define OPENATCMAINCTLMANAGER_H

#include "../Include/OpenATCParameter.h"
#include "../Include/OpenATCRunStatus.h"
#include "../Include/OpenATCLog.h"

const int C_N_FEEDDOG_INTERVAL = 5;
const int C_N_FEEDDOG_TIMEOUT = 20;

const int C_N_GPSADJUSTTIME_INTERVAL = 2;

/*=====================================================================
类名 ：COpenATCMainCtlManager
功能 ：程序的主调度类，用于参数和变量初始化，通讯任务初始化，定时器初始化，全局参数和变量的管理。
主要接口：
备注 ：
-----------------------------------------------------------------------
修改记录：
日 期          版本     修改人     走读人       修改记录
2019/09/14     V1.0     刘黎明     李四         创建类
=====================================================================*/

#ifdef _WIN32
    #ifdef DLL_FILE
    class _declspec(dllexport) COpenATCMainCtlManager
    #else
    class _declspec(dllimport) COpenATCMainCtlManager
    #endif
#else
    class COpenATCMainCtlManager
#endif
{
public:
    //类定义为单件
    static COpenATCMainCtlManager * getInstance();

    //任务初始化
    void Init(COpenATCParameter * pParameter, COpenATCRunStatus * pRunStatus, COpenATCLog * pOpenATCLog);

    //任务停止
    void Stop();  

    //模块主流程
    void Work();
	
    //获取状态类指针
	COpenATCRunStatus * GetOpenATCRunStatus();

	//获取日志类指针
	COpenATCLog * GetOpenATCLog();

    //设置退出停狗状态
    void SetStopDogFlag(bool bFlag);

private:
	COpenATCMainCtlManager();
	~COpenATCMainCtlManager();

    //处理linux系统的信号
    void ProcSystemSignal();

    //初始化看门狗
    bool InitWatchDog();

    //喂狗
    bool FeedWatchDog();

    //停狗
    bool StopWatchDog();

	//设置系统时间
	bool SetSysTime(const long nTime);

    static COpenATCMainCtlManager * s_pData;

	COpenATCRunStatus * m_pLogicCtlStatus;              //运行状态类指针
    bool m_bStopDogFlag;                                //退出时的停狗标志

	COpenATCLog       * m_pOpenATCLog;                  //日志类指针
};

#endif // !ifndef OPENATCMAINCTLMANAGER_H
