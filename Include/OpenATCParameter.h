/*=====================================================================
模块名 ：主控板运行参数类
文件名 ：OpenATCParameter.h
相关文件：OpenATCParameter.cpp
实现功能：用于主控板运行所需的特征参数
作者 ：刘黎明
版权 ：<Copyright(c) 2019-2020 Suzhou Keda Technology Co.,Ltd. All right reserved.>
---------------------------------------------------------------------------------------------------------------------
修改记录：
日 期           版本     修改人     走读人     修改记录
2019/9/14       V1.0     刘黎明      梁厅       创建模块
=====================================================================*/

#ifndef OPENATCPARAMETER_H
#define OPENATCPARAMETER_H

#include "../Include/OpenATCParamConstDefine.h"
#include "../Include/OpenATCParamStructDefine.h"
#include "../Include/OpenATC20999ParamStructDefine.h"
#include "../Include/OpenATCRunStatus.h"
#include "../Include/OpenATCLog.h"
#include "../Include/OpenATCFlowProcDefine.h"
#include "../OpenATCManager/comctl/OpenATCComDef.h"

#ifdef VIRTUAL_DEVICE
#define JSON_FILE_TZ_PATH "./config/OpenATCTZParam.json"
#define JSON_FILE_HW_PATH "./config/OpenATCHWParam.json"
#define XML_FILE_CONFIG_PATH "./config/ConfigPort.xml"
#else
#define JSON_FILE_TZ_PATH "/mnt/OpenATCTZParam.json"
#define JSON_FILE_HW_PATH "/mnt/OpenATCHWParam.json"
#define XML_FILE_CONFIG_PATH "/usr/config/ConfigPort.xml"
#endif

#ifdef VIRTUAL_DEVICE
#define FAULT_FILE_PATH "./log/FAULT.json"
#else
#define FAULT_FILE_PATH "/usr/log/FAULT.json"
#endif

const int C_N_MAXJOSNBUFFER_SIZE			= 1024 * 1024;
const int C_N_MAX_DEVICE_PARAM_BUFFER_SIZE	= 1024;
const int C_N_MAX_TIMEBASESCHEDULE_MONTH	= 12;
const int C_N_MAX_TIMEBASESCHEDULE_WEEK		= 7;
const int C_N_MAX_TIMEBASESCHEDULE_DAY		= 31;
const int C_N_MAX_TIMEBASEDAYPLAN_HOUR		= 24;
const int C_N_MAX_TIMEBASEDAYPLAN_MINUTE	= 60;
const int C_N_MAXFAULTBUFFER_SIZE			= 1024 * 1024 * 2;
const int C_N_MAXSIDE_SIZE			        = 7;

class COpenATCParamCheck;

/*=====================================================================
类名 COpenATCParameter
功能 ：用于存储及获取主控板运行所需要的特征参数。
主要接口：
备注 ：
---------------------------------------------------------------------------------------------------------------------
修改记录：
日 期          版本     修改人     走读人       修改记录
2019/09/14     V1.0     刘黎明      梁厅         创建类
=====================================================================*/

#ifdef _WIN32
    #ifdef DLL_FILE
    class _declspec(dllexport) COpenATCParameter
    #else
    class _declspec(dllimport) COpenATCParameter
    #endif
#else
    class COpenATCParameter
#endif
{
public:
	COpenATCParameter();
	virtual ~COpenATCParameter();

	//初始化特征参数
	void Init(COpenATCRunStatus * pRunStatus, COpenATCLog * pOpenATCLog);

	//根据时区信息设置系统时区
	void SetSystemTimeZone();

	//获取时区信息
	void GetSystemTimeZone(int & nTimeZoneHour, int & nTimeZoneMinute);

	//获取特征参数
	void GetAscParamInfo(TAscParam &tAscParamInfo);

    //设置地址板ID
    void SetSiteID(int nSiteID);

	//获取地址板ID
	int GetSiteID();

	//根据日期获取对应的调度计划表
	void GetTimeBaseScheduleByTime(int nMonth, int nWeek, int nDate, TTimeBaseSchedule & tScheduleInfo, int &nCount);

	//根据时段获取对应的时段表信息
	void  GetTimeBaseDayPlanByTime(int nHour, int nMinute, BYTE wTimeBaseScheduleNumber, TTimeBaseDayPlan & tTimeBaseDayPlanInfo);

	//根据方案号获取方案信息
	void GetPatternByPlanNumber(int nPlanNumber, TPattern &tPatternInfo);

	//根据绿信比表号获取的绿信比信息表	
	void GetSplitBySplitNumber(int nSplitNumber, TSplit(&atSplitInfo)[MAX_PHASE_COUNT]);

	//根据时序表号获取时序表
	void GetSequenceBySequenceNumber(int nSequenceNumber, TSequence(&atSequenceInfo)[MAX_RING_COUNT]);

	//根据相位号获取对应的相位信息
	void GetPhaseByPhaseNumber(int nPhaseNumber, TPhase & tPhaseInfo);

	//从信号机参数结构体中获取跟随相位表
	void GetOverlapTable(TOverlapTable(&atOverlapTable)[MAX_OVERLAP_COUNT]);

	//从信号机参数结构体中获取通道表
	void GetChannelTable(TChannel(&atChannelTable)[MAX_CHANNEL_COUNT]);

	//从信号机参数结构体中获取相位表
	void GetPhaseTable(TPhase(&atPhaseTable)[MAX_PHASE_COUNT]);
	
	//从信号机参数结构中获取车辆检测器表
	void GetVehicleDetectorTable(TVehicleDetector(&atVehicleDetectorTable)[MAX_VEHICLEDETECTOR_COUNT]);

    //从信号机参数结构中获取行人检测器表
    void GetPedDetectorTable(TPedestrianDetector(&atPedDetectorTable)[MAX_PEDESTRIANDETECTOR_COUNT]);

	//从信号机参数结构中获取网卡设置参数
	void GetNetCardsTable(TAscNetCard(&atNetCardsTable)[MAX_NETCARD_TABLE_COUNT]);

	//从信号机参数结构中获取平台中心配置参数信息
	void GetCenterInfo(TAscCenter& tCenterInfo);

	//从信号机参数结构中获取仿真平台中心配置参数信息
	void GetSimulateInfo(TAscSimulate& tSimulateInfo);

	//从信号机参数结构中获取路口配置参数信息（路口号，区域号）
	void GetAscAreaInfo(TAscArea& tAreaInfo);
	
	//从信号机参数结构体中获取手动面板配置
	void GetManualPanelInfo(TAscManualPanel& tManualPanel);

	//根据方案号获取单点自适应控制参数
	void GetSingleOptimInfo(int nPlanNumber, TAscSingleOptim& tSingleOptimInfo, int RingIndex, int *PhaseIndex);

	//获取级联参数信息
	void GetAscCasCadeInfo(TAscCasCadeInfo &tCasCade);

	//获取通道锁定参数信息
	void GetChannelLockInfo(TAscOnePlanChannelLockInfo(&atChannelLockInfo)[MAX_SINGLE_DAYPLAN_COUNT]);

	//获取启动时序参数信息
	void GetStartSequenceInfo(TAscStartSequenceInfo &tStartSequenceInfo);

	//获取故障检测参数配置
	void GetFaultDetectInfo(TAscFaultCfg &tFaultCfg);

	//获取步进配置
	void GetStepInfo(TAscStepCfg &tStepCfg);

	//获取整体buffer参数信息
	unsigned char* GetParamData();

	//获取相位参数信息
	unsigned char* GetPhaseParamData();

	//获取方案参数信息
	unsigned char* GetPatternParamData();

	//获取日期参数信息
	unsigned char* GetDateParamData();

	//获取跟随相位参数信息
	unsigned char* GetOverLapParamData();

	//获取计划参数信息
	unsigned char* GetPlanParamData();

	//获取车辆检测器参数信息
	unsigned char* GetVecDetectorParamData();
    
	//获取行人检测器参数信息
	unsigned char* GetPedDetectorParamData();

	//获取通道参数信息
	unsigned char* GetChannelParamData();

	//获取信号机当前系统时间信息（回配置软件时间查询）
	unsigned char * GetATCLocalTime();

	//获取信号机识别码
	unsigned char * GetATCCode();

	//获取特征参数版本
	unsigned char * GetATCParamVersion();

	//获取信号机版本
	unsigned char * GetATCVersion(char * pATCVersion);

	//获取工作方式
	unsigned char * GetWorkPattern();

	//获取设备参数信息
	unsigned char * GetSystemCustom();

	//获取方案运行状态信息
	unsigned char* GetPatternRunStatusData();

	//获取故障上报信息
	unsigned char* GetATCFaultReportData(TAscFault *pTAscFault);
	
	//获得实时流量接口
	unsigned char* GetCurrentTrafficFlowData(TStatisticVehDetData* pTVehData);

	//获取查询故障信息(所有故障信息)
	unsigned char* GetATCFaultQueryData(int & iMyJsonBufferSize);

	//获取通道灯电压灯电流信息
	unsigned char* GetATCChannelStatus(TChannelStatusInfo(&atChannelStatusInfo)[MAX_CHANNEL_COUNT]);

	//获取通道灯色状态
	unsigned char* GetChannelLampStatusInfo();

	//注册平台数据内容
	unsigned char* GetAscLoginCenterData();
	
	//获取远程控制指令值
	int GetRemoteValueValue(unsigned char* pRemote);

	//解析工作方式
	void GetWorkModeParam(unsigned char* pWorkMode, TWorkModeParam &atWorkModeParam, TPhasePassCmdPhaseStatus &atPhasePhassCmdPhaseStatus, TChannelLockCtrlCmd &atChannelLockCtrlCmd, TPhaseLockCtrlCmd &atPhaseLockCtrlCmd, TPreemptCtlCmd &atPreemptCtlCmd);
	
	//解析信号机时间设置
	void GetAscTimeSetValue(unsigned char* pTimeSet,time_t &timeValue);

	//解析信号机远程调试设置
	void GetAscRemoteControlValue(unsigned char* pRemote, TAscRemoteControl& atRemote);

	//解析通道可检测设置信息
	void GetAscChannelVerifyValue(unsigned char* pChannelVerify, TAscChannelVerifyInfo& atChannelVerify);

	//解析平台发送获取历史流量数据指令
	void GetAscGainTrafficFlowCmd(unsigned char* pGainTrafficFlowCmd, TAscGainTafficFlowCmd& atGainTrafficFlowCmd);

	//解析平台发送的方案干预数据信息
	void GetAscInterruptPatternInfo(unsigned char* pInterruptPattern);

	//解析故障查询指令值
	int GetAscQueryFaultValue(unsigned char* pFault);

	//获取参数buffer有效参数大小
	int GetParamDataSize();

	//方案编号检验
	int CheckPatternNum(TWorkModeParam *pTWorkMode);
	
	//保存特征参数
	int SaveParameter(unsigned char * pData, int nSize);

	//保存设备参数
	int SavaSystemCustom(unsigned char * pData, int nSize);

	// 挂载U盘
	int MountUSBDevice(COpenATCRunStatus * pRunStatus, char & chFailedReason);
	
	// 卸载U盘
	int UnmountUSBDevice(COpenATCRunStatus * pRunStatus);

	// 设置GPS信息
	void SetGpsInfo(const TGpsData & tGpsData);

	// 更新设备参数
	bool GenDeviceParamFile(unsigned char *pJsonBuffer, const char* pFilePath);

    //通道锁定指令绿冲突校验
	int  CheckGreenConflictByChannelLock(int(&nChannelStatus)[MAX_CHANNEL_COUNT]);
	
	//校验部分下载的参数是否正确,正确的话,要更新全局buffer并生成新的参数文件
	int  CheckPartParamAndUpdateParam(BYTE byParamType, unsigned char *pData, TErrInfo(&atErrCodeTable)[C_N_MAX_ERR_SIZE]);
	
	//设置方案干预数据信息
	void SetAscInterruptPatternInfo(TInterruptPatternInfo tInterruptPatternInfo);

	//设置手动控制方案
	void SetManualControlPattern(TInterruptPatternInfo tInterruptPatternInfo);

	//从信号机参数结构中获取通道绿冲突
	void GetChannelGreenConflict(char tChannelGreenConflict[MAX_CHANNEL_COUNT][MAX_CHANNEL_COUNT]);

	//获取通道绿冲突表配置数量
	int GetChannelGreenConflictCount();

	//根据优先编号获取优先优先参数
	bool GetPreemptParam(int nPreemptIndex, TPreempt & tPreempt);
	
	//获取地址板ID
    void GetSiteID(char * pSiteIDFromATC, char * pSiteIDFromParam);

	//GB20999相关
	void Gb20999ToGb25280(TAsc20999Param tTempAsc20999Param, TAscParam& atTAscParam);

	//从现有参数获取20999格式参数
	void GetAscParamByRunParam(TAsc20999Param& tTempAsc20999Param);

	//获取相位的灯组信息
	void GetPhaseLightGroup(BYTE tPhaseNum, BYTE* tLightGroup);

	//获取相位的需求
	void GetPhaseCall(BYTE tPhaseNum, BYTE* tPhaseCall);

	//通过相位并发相位来获取相位冲突表
	void GetPhaseConflict(BYTE tPhaseNum, BYTE* tPhaseConflict);

	//20999参数结构转25280参数结构
	void Gb20999ToGb25280(TAscParam& atTAscParam, TAsc20999Param tTempAsc20999Param);

	//设置缓存特征参数
	void SetAscTempParamInfo(const TAscParam& tAscParamInfo);

	//20999参数结构数据保存
	int SaveGB20999ASCParam(bool isTransaction/*TAscParam* tAscParamInfo, TErrInfo(&atErrCodeTable)[C_N_MAX_ERR_SIZE]*/);

	//查询当且阶段是否已经存在
	bool CheckStageExistence(BYTE* tStageTablePhase, BYTE* tNewStagePhase);

	//获取相位阶段信息的相位阶段的相位
	void GetPhaseInStage(BYTE* tPhaseStage, BYTE* tPhaseInPhaseStage);

	//紧急优先已经找到的优先号
	bool HaveFindNum(int tPreempt, int* tPreemptNum);

	//由包含的相位来查找对应的阶段号
	int GetStageNumByPhaseInfo(BYTE* tPhaseInfo, int tPhaseStageCount, T20999PhaseStage* m_stPhaseStage);

	//从配置工具获取线程选择配置
	int GetCommThreadInfo();

	//获取日期数
	int GetDatePlanNum();

	//根据时段表号来获取时段表索引
	int GetDayPlanIndexByNum(BYTE byDayPlanNum);

#ifdef VIRTUAL_DEVICE
	//获取信号机周期加速运行选项
	int	GetSpeedyRunInfo();

	//判断信号机是否需要根据指定配置的时间运行 //Virtual_Test2022
	void SetStartTime();

	//蔡勒公式计算星期 //Virtual_Test2022
	int GetWeek(int nYear, int nMonth, int nDay);

	//判断闰年//Virtual_Test2022
	int isLeap(int year);
#endif // VIRTUAL_DEVICE
//Virtual_Test2022

private:
	//md5码校验
	bool CheckMd5Code(unsigned char * pdataJsonBuffer, int nDataJsonBufferSize);

	//地址码检验
	bool CheckSiteID(unsigned char* pdataJsonBuffer, int nDataJsonBufferSize, int nSiteID, bool bDeviceParamFlag);

	//在系统运行日志中记录校验错误原因
	void RecordVerifyErrorToSystemLogFile(TErrInfo *pCheckCode);

	//在故障日志中记录校验错误原因
	void RecordVerifyErrorToFaultLogFile(TErrInfo *pCheckCode);

	//设置方案干预参数
	void ReSetInterruptPatternInfo();

private:
	//清空缓存区
	void ClearBuffer();

	COpenATCRunStatus  *m_pOpenATCRunStatus;								//全局运行状态类指针
	 
	COpenATCLog        *m_pOpenATCLog;										//基础日志指针

	TAscParam  m_tAscParamInfo;												//特征参数

	TAscParam  m_tTempAscParamInfo;                                         //临时特征参数		//20999参数存储缓存

	unsigned char m_chParamBuffer[C_N_MAXJOSNBUFFER_SIZE];					//特征参数buffer

	int m_nJsonBufferSize;													//特征参数有效尺寸

	unsigned char m_chDeviceParamBuffer[C_N_MAX_DEVICE_PARAM_BUFFER_SIZE];	//设备参数buffer

	int m_nDeviceParamBufferSize;											//设备参数有效尺寸
 
	unsigned char m_chFaultBuffer[C_N_MAXFAULTBUFFER_SIZE];					//故障buffer

	int m_nSiteID;															//地址码

	TGpsData m_nGpsData;													//GPS信息

	COpenATCParamCheck*       m_openATCParamCheck;

	TInterruptPatternInfo    m_atInterruptPatternInfo;                      //特殊方案干预信息

	bool m_bIsSetSimStartTime;												//是否设置了仿真启动时间
	
	char m_szSiteIDFromATC[C_N_MAXSIDE_SIZE];                               //信号机实际地址码

	char m_szSiteIDFromParam[C_N_MAXSIDE_SIZE];							    //参数中的地址码

};

#endif // !ifndef OPENATCPARAMETER_H
