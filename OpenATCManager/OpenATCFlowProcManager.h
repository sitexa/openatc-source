/*=====================================================================
模块名 ：流量处理模块
文件名 ：OpenATCFlowProcManager.h
相关文件：OpenATCParameter.h OpenATCRunStatus.h
实现功能：流量处理模块调度类，用于流量统计和实时数据生成。
作者 ：刘黎明
版权 ：<Copyright(c) 2019-2020 Suzhou Keda Technology Co.,Ltd. All right reserved.>
------------------------------------------------------------------------
修改记录：
日 期           版本     修改人     走读人     修改记录
2019/9/26       V1.0     刘黎明     刘黎明      创建模块
=====================================================================*/

#ifndef OPENATCFLOWPROCMANAGER_H
#define OPENATCFLOWPROCMANAGER_H

#include "OpenATCFlowProcLog.h"
#include "../Include/OpenATCParameter.h"
#include "../Include/OpenATCRunStatus.h"
#include "../Include/OpenATCLog.h"
#include "../Include/OneWayQueue.h"
#include "../Include/OpenATCFlowProcDefine.h"

#ifdef _WIN32
#define FLOWPROCMANAGER_CALLBACK WINAPI
typedef HANDLE               FLOWPROCMANAGERHANDLE;
#else
#define FLOWPROCMANAGER_CALLBACK
typedef pthread_t            FLOWPROCMANAGERHANDLE;
#endif

class COpenATCFlowProcManager  
{
public:
    //类定义为单件
    static COpenATCFlowProcManager * getInstance();

    //类的初始化操作
    void Init(COpenATCParameter * pParameter,COpenATCRunStatus * pRunStatus, COpenATCLog * pOpenATCLog);

    //类的主流程
    void Work();

    //类的停止与释放
    void Stop();

	FLOWPROCMANAGERHANDLE  GetHandle();

	virtual int Run();

	//获取需要存储的流量队列
    inline COneWayQueue<TStatisticVehDetData> & GetFlowDataQueue()
    {
        return m_FlowDataQueue;
    }
    //设置需要存储的流量队列
    inline void SetFlowDataQueue(COneWayQueue<TStatisticVehDetData> & oneWayQueue)
    {
        memcpy(&m_FlowDataQueue,&oneWayQueue,sizeof(m_FlowDataQueue));     
    }

	//将本地流量数据写入U盘
	inline void BackUpLogFile(const char* pdir)
	{
        m_OpenATCFlowProcLog.BackUpLogFile(FLOW_FILE_DISK_PATH, m_pLogicCtlStatus, m_pOpenATCLog);
	}

private:
	COpenATCFlowProcManager();
	~COpenATCFlowProcManager();

    int  ReadConfig(char szXmlFile[]);

    int  SetLogFileName(const char *szLogFilePath, const char *szLogFileName);

    void CreateNewLogFile();	

    void LogOneMessage(const char *szFormat, ...);

    static COpenATCFlowProcManager * s_pData;               //单件类指针
    COpenATCParameter * m_pLogicCtlParam;                   //特征参数类指针
    COpenATCRunStatus * m_pLogicCtlStatus;                  //运行状态类指针
	COpenATCLog       * m_pOpenATCLog;                      //日志类指针

	FLOWPROCMANAGERHANDLE         m_hThread;
    unsigned long                 m_dwThreadRet;
	static void *FLOWPROCMANAGER_CALLBACK FlowDataThread(void *pParam);


    //处理实时车检数据
    void ProcRTVehDetData();

    //处理车辆统计数据
    void ProcSTVehDetData();

    //根据车辆持续时间判断车型
    int GetVehType(long nExistTimeMs);

	//延迟
	void OpenATCSleep(long nMsec);

	//计算计数的函数
	unsigned long CalcCounter(unsigned long nStart, unsigned long nEnd, unsigned long nMax);  

	void  SetDetectorStatusCounter();//绿灯跨多个统计周期时，更新检测器状态计数器

    TStatisticVehDetData m_tStatisticFlowData;                              //流量统计数据

	TStatisticVehDetData m_tOldStatisticFlowData;                           //最近一次的流量统计数据

	ofstream        m_outPutStream;
	size_t          m_nFileMaxSize;      //日志文件最大大小 超过该大小将被删除 默认为10MB
	int				m_nFileMaxNum;		 //记录的日志文件数量

	char			m_szLogFilePath[FRM_MAX_LOGFILENAME_LENGTH];			//日志文件所在目录
	char			m_szLogFileName[FRM_MAX_LOGFILENAME_LENGTH];			//日志类别的程序名 
	char			m_szCurrLogFileName[FRM_MAX_LOGFILENAME_LENGTH];	    //当前日志文件名称

    int             m_nLastMin;

	bool           m_bGreenStartFlag[MAX_VEHICLEDETECTOR_COUNT];            
	unsigned long  m_nGreenStartCounter[MAX_VEHICLEDETECTOR_COUNT];

    COneWayQueue<TStatisticVehDetData>  m_FlowDataQueue; 

	COpenATCFlowProcLog                 m_OpenATCFlowProcLog;

public:
    TStatisticVehDetData  &  GetCurrentStatisticVehDetData();

};

#endif //ifndef OPENATCFLOWPROCMANAGER_H
