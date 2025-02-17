/*=====================================================================
模块名 ：主控板运行参数类
文件名 ：OpenATCFlowProLog.h
相关文件：OpenATCFlowProLog.cpp
实现功能：用于存储交通流的日志模块
作者 ：梁厅
版权 ：<Copyright(c) 2019-2020 Suzhou Keda Technology Co.,Ltd. All right reserved.>
---------------------------------------------------------------------------------------------------------------------
修改记录：
日 期           版本     修改人     走读人     修改记录
2019/9/14       V1.0     梁厅       梁厅      创建模块
=====================================================================*/
#ifndef OPENATCFLOWPROLOG_H 
#define OPENATCFLOWPROLOG_H

#include "cJSON.h"
#include "../Include/OpenATCRunStatus.h"
#include "../Include/OpenATCLog.h"
#include "../Include/OpenATCFlowProcDefine.h"

#ifdef _WIN32
#include <time.h>
#define FLOW_FILE_LOCAL_PATH "./TRAFFICFLOW.json"
#define FLOW_FILE_DISK_PATH  ".//TrafficFlowLog"  //U盘文件夹
#else
#define FLOW_FILE_LOCAL_PATH "/usr/log/TRAFFICFLOW.json"
#define FLOW_FILE_DISK_PATH  "/mnt/TrafficFlowLog"  //U盘文件夹
#include <sys/time.h>
#endif

const int C_N_MAX_VEHICLEDETECTOR_COUNT = 72;
const int C_N_MAXFLOWTBUFFER_SIZE		= 1024 * 1024 * 50;//buffer 最大尺寸
const int C_N_MAX_FLOW_FILE_SIZE		= 1024 * 1024 * 25; //文件单日存储最大尺寸
const int C_N_MAX_FILEPATH_SIZE			= 128;
const int C_N_MAX_TIME_CHAR_SIZE		= 12;
const int C_N_MAX_TIME_BUFF_SIZE		= 30;
const int FLOW_DATA_FILE_RETENTION_TIME = 1;	// 流量文件保留时长，单位：月


class COpenATCFlowProcLog
{
public:
	COpenATCFlowProcLog();
	virtual ~COpenATCFlowProcLog();

	//记录交通流信息
	void SaveTrafficFlowInfo(PTStatisticVehDetData, COpenATCRunStatus * pRunStatus, COpenATCLog * pOpenATCLog);
		
	//将本地流量数据写入U盘
	void BackUpLogFile(const char* pdir, COpenATCRunStatus * pRunStatus, COpenATCLog * pOpenATCLog);

	//U盘上的流量文件维护
	void FlowDataFilesMaintenance(COpenATCLog * pOpenATCLog);

private:
	//创建交通流日志json文件
	void CreateFlowFile();

	//判断指定目录文件夹是否存在，不存在创建文件夹
	void NewDirCreate(const char* pdir, int& nResult, COpenATCLog * pOpenATCLog);

	// 挂载U盘
	int MountUSBDevice(COpenATCRunStatus * pRunStatus, COpenATCLog * pOpenATCLog);

	// 卸载U盘
	int UnmountUSBDevice(COpenATCRunStatus * pRunStatus, COpenATCLog * pOpenATCLog);

	char *m_pFaultBuffer;

	unsigned long m_nAscFlowFileSize;  //当前日志维护大小,默认最大1G

};


#endif// !ifndef OPENATCFAULTLOG_H