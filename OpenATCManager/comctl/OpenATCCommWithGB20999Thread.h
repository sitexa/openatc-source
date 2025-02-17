/*=====================================================================
模块名 ：和GB20999协议测试软件交互的线程模块
文件名 OpenATCCommWithGB20999Thread.h
相关文件：OpenATCDataPackUnpack.h OpenATCComDef.h
实现功能：和GB20999协议测试软件的交互
作者 ：李永萍
版权 ：<Copyright(c) 2019-2020 Suzhou Keda Technology Co.,Ltd. All right reserved.>
--------------------------------------------------------------------------------------------------------------------
修改记录：
日 期           版本    修改人             走读人             修改记录
2019/09/06      V1.0    李永萍             李永萍             创建模块
====================================================================*/

#ifndef OPENATCCOMMWITHGB20999THREAD_H
#define OPENATCCOMMWITHGB20999THREAD_H

#include "../../Include/OpenATCParameter.h"
#include "../../Include/OpenATCRunStatus.h"
#include "../../Include/CanBusManager.h"
#include "../logicctl/LogicCtlFixedTime.h"
#include "../../Include/OpenATC20999ParamStructDefine.h"
#include "../../Include/OpenATCOperationRecord.h"
#include "OpenATCComDef.h"
#include "OpenATCITS300DataPackUnpack.h"
#include "OpenATCSocket.h"
#include "OpenATCCfgCommHelper.h"

class COpenATCSocket;

#ifdef _WIN32
	#define COMMWITHGB20999_CALLBACK WINAPI
	typedef HANDLE               COMMWITHGB20999HANDLE;
#else
	#define COMMWITHGB20999_CALLBACK
	typedef pthread_t            COMMWITHGB20999HANDLE;
#endif

class COpenATCParameter;
class COpenATCRunStatus;

const short C_N_AGREEMENT_VERSION       = 0x0101;            //协议版本

const BYTE  C_N_HOST_ID                 = 0x01;              //上位机ID
const BYTE  C_N_ROAD_ID                 = 0;                 //路口ID

const int   C_N_FRAME_ID_POS            = 10;                //帧流水号
const int   C_N_FRAME_TYPE_POS          = 11;                //帧类型
const int   C_N_DATAVALUE_COUNT_POS     = 12;                //数据值数量
const int   C_N_DATAVALUE_INDEX_POS     = 13;                //数据值索引
const int   C_N_DATAVALUE_LENGTH_POS    = 14;                //数据值长度
const int   C_N_DATACLASS_ID_POS        = 15;                //数据类ID
const int   C_N_OBJECT_ID_POS           = 16;                //对象ID
const int   C_N_ATTRIBUTE_ID_POS        = 17;                //属性ID
const int   C_N_ELEMENT_ID_POS          = 18;                //元素ID
const int   C_N_DATAVALUE_POS           = 19;                //数据值

const int   C_N_MAX_PLATE_LENGTH        = 16;                //最大号牌长度
const int   C_N_MAX_PHASE_LIGHTGROUP    = 8;                 //相位的灯组长度
const int   C_N_MAX_PHASE_DETECTOR      = 8;                 //相位的检测器长度
//const int   C_N_MAX_PHASE_RUNSTAGE      = 8;                 //相位的阶段长度
const int   C_N_MAX_PHASE_CONFLICT      = 8;                 //相位的冲突序列长度
const int   C_N_MAX_PHASE_GREENINTERVAL = 64;                //相位的绿间隔时间序列长度
//const int   C_N_MAX_PATTERN_STAGE_CHAIN = 16;                //方案的阶段长度
const int   C_N_MAX_DEVICE_MODULE       = 8;                 //设备模块长度
const int   C_N_MAX_TRANSIT_RETAIN      = 64;                //相位的阶段过渡约束值长度
const int   C_N_MAX_LIGHTGROUP_COUNT    = 64;                //最大灯组数量
const int   C_N_MAX_PHASE_COUNT         = 64;                //最大相位数量
const int   C_N_MAX_DETECTOR_COUNT      = 128;               //最大检测器数量
const int   C_N_MAX_RUNSTAGE_COUNT      = 64;                //最大运行阶段数量
const int   C_N_MAX_PRIORITY_COUNT      = 64;                //最大优先信号数量
const int   C_N_MAX_EMERGENCY_COUNT     = 64;                //最大紧急信号数量
const int   C_N_MAX_PATTERN_COUNT       = 128;               //最大方案数量
const int   C_N_MAX_DAYPLAN_COUNT       = 128;               //最大日计划数量
const int   C_N_MAX_TIMECHAIN_COUNT     = 96;                //最大时段链数量
const int   C_N_MAX_PATTERNCHAIN_COUNT  = 48;                //最大方案链数量
const int   C_N_MAX_RUNMODECHAIN_COUNT  = 48;                //最大模式链数量
const int   C_N_MAX_ACTCHAIN_COUNT      = 96;                //最大动作链数量
const int   C_N_MAX_SCHEDULE_COUNT      = 128;               //最大调度表数量

typedef struct tagDataConfig
{
	BYTE        m_byIndex;                                  //索引
	BYTE        m_byDataLength;                             //数据值长度
	BYTE        m_byDataClassID;                            //数据类ID    
	BYTE        m_byObjectID;                               //对象ID
	BYTE        m_byAttributeID;                            //属性ID
	BYTE        m_byElementID;                              //元素ID
	BYTE        m_byDataValueLength;                        //元素值长度
	BYTE        m_byDataValue[256];                         //元素值
}TDataConfig,*PTDataConfig;

typedef struct tagReturnData
{
	BYTE        m_byReturnCount;                            //返回值数量
	TDataConfig m_tDataConfig[256];                     
}TReturnData,*PTReturnData;

typedef struct tagDBManagement
{
	BYTE m_byDBCreateTransaction;		//normal(1)、transaction(2)、verify(3)、done(6)
	BYTE m_byDBVerifyStatus;			//notDone(1)、doneWithError(2)、doneWithNoError(3)
	BYTE m_byDBVerifyError;				//对验证处理发现的错误的文本描述。当且仅当DBCreateTransaction对象处于Done状态且dbVerifyStatus对象处于doneWithError状态时有效，其他对象值无效，但不会显示错误。
}TDBManagement, * PTDBManagement;

#ifdef _WIN32
#define GB20999_CALLBACK WINAPI
typedef HANDLE               GB20999HANDLE;
#else
#define GB20999_CALLBACK
typedef pthread_t            GB20999HANDLE;
#endif

#define NORMAL      1
#define TRANSACTION 2
#define VERIFYING    3
#define DONE        6

#define NOTDONE			1
#define DONEWITHERROR	2
#define DONEWITHNOERROR	3

/*=====================================================================
类名 ：COpenATCCommWithIGB20999Thread
功能 ：和GB20999协议测试软件的交互
主要接口：void Init：初始化参数，第一个参数y为配置参数，第二个参数为状态
备注 ：
--------------------------------------------------------------------------------------------------------------------
修改记录：
日 期           版本    修改人             走读人             修改记录
2019/09/06      V1.0    李永萍             李永萍             创建类
====================================================================*/
class COpenATCCommWithGB20999Thread
{
public:
    COpenATCCommWithGB20999Thread();
    virtual ~COpenATCCommWithGB20999Thread();

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
	void        Init(COpenATCParameter *pParameter, COpenATCRunStatus *pRunStatus, COpenATCLog * pOpenATCLog);

    int         Start();
    int         Join();
    int         Detach();


private:
	enum
	{
		RECV_BUFFER_SIZE		= 1024,
		UNPACKED_BUFFER_SIZE	= 1024,  
		SEND_BUFFER_SIZE        = 1024,
		PACKED_BUFFER_SIZE      = 1024,

		HEART_INTERVAL_TIME     = 15,
		ALARM_INTERVAL_TIME     = 60,
		FAULT_INTERVAL_TIME     = 60,
	};
    static void *COMMWITHGB20999_CALLBACK RunThread(void *pParam);

	void  OpenATCSleep(long nMsec);

	unsigned short Crc16(const unsigned char *buffer, int buffer_length);

    void  ParserPack(char* chPeerIp);//解析数据包

    void  ParserPack_QueryLink();//解析查询包

	void  ParserPack_SetLink(char* chPeerIp);//解析设置包

	int   SendAckToPeer(int nPackSize, int & nRet);//发送数据

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void  GetPatternParam(BYTE byPatternSplitNumber, BYTE byPatternSequenceNumber, TRunStageInfo & tRunStageInfo);//获取方案参数

	void  GetRunStageTable(TFixTimeCtlInfo tFixTimeCtlInfo, TRunStageInfo & tRunStageInfo);//获取阶段表 

	BYTE  TransAlarmTypeToHost(BYTE byFaultValue, char cFaultInfo1, char cFaultInfo2, BYTE & byAlarmValue); //转换报警类型到平台

	BYTE  TransFaultTypeToHost(BYTE byFaultValue); //转换故障类型到平台

	BYTE  TransRunModeToHost(BYTE byModeValue, BYTE byControlSource); //转换运行模式到平台

	BYTE  TransRunModeToASC(BYTE byModeValue); //转换运行模式到信号机

	BYTE  TransStageStatus(BYTE byStageStatus);	//转换阶段运行状态

	void  OpenATCGetCurTime(int & nYear, int & nMon, int & nDay, int & nHour, int & nMin, int & nSec, int & nWeek);//获取当前系统时间

	bool  SetSysTime(const long nTime);

	char  GetBit(unsigned int nInput, char chNum); //从一个值中取出任意位

	void  RemoveDuplates(BYTE byData[], int & nCnt);//删除数组中重复数据

	void  SetTSequenceInfo(BYTE byPatternSequenceNumber, TRunStageInfo tRunStageInfo);//设置相序参数

	void  SetTSplitInfo(BYTE byPatternSplitNumber, TRunStageInfo tRunStageInfo, bool bSetOrder, bool bSetSplitTime);//设置绿信比参数

	bool  SetStartAndEndTime(BYTE byPatternSplitNumber, BYTE byRunStageIndex, TRunStageInfo tRunStageInfo, bool bStartTime, bool bEndTime, BYTE byDataValuePos);//设置晚启动时间和早结束时间

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void  QueryLightGroupCount(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//获取灯组数量

	void  QueryAllElementLightGroupConfig(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//获取所有元素的灯组配置

	void  QueryAllElementLightGroupStatus(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//获取所有元素的灯组状态

	void  QueryAllElementLightGroupControl(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//获取所有元素的灯组控制

	void  QueryLightGroupConfig(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//获取指定元素的灯组配置

	void  QueryLightGroupStatus(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//获取指定元素的灯组状态

	void  QueryLightGroupControl(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//获取指定元素的灯组控制

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void  QueryPhaseCount(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//获取相位数量

	void  QueryAllElementPhaseConfig(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//获取所有元素的相位配置

	void  QueryAllElementPhaseControl(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//获取所有元素的相位控制

	void  QueryPhaseConfig(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//获取指定元素的相位配置

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void  QueryPhaseControl(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//获取指定元素的相位控制

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void  QueryDetectorCount(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//获取检测器数量

	void  QueryAllElementDetectorConfig(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//获取所有元素的检测器配置

	void  QueryAllElementDetectorData(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//获取所有元素的检测器数据

	void  QueryDetectorConfig(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//获取指定元素的检测器配置

	void  QueryDetectorData(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//获取指定元素的检测器数据

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void  QueryPhaseStageCount(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//获取阶段数量

	void  QueryAllElementPhaseStageConfig(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//获取所有元素的阶段配置

	void  QueryAllElementPhaseStageStatus(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//获取所有元素的阶段状态

	void  QueryAllElementPhaseStageControl(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//获取所有元素的阶段控制

	void  QueryPhaseStageConfig(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//获取指定元素的相位阶段配置

	void  QueryPhaseStageStatus(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//获取指定元素的相位阶段状态配置

	void  QueryPhaseStageControl(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//获取指定元素的相位阶段控制配置

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void  QueryAllElementPhaseeConflictInfo(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//获取所有元素的相位冲突表

	void  QueryAllElementPhasGreenInterval(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//获取所有元素的绿间隔表

	void  QueryPhaseConflict(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//获取指定元素的相位冲突

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void  QueryPhaseGreenInterval(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//获取指定元素的相位绿间隔序列

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void  QueryPriorityCount(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//获取优先数量

	void  QueryEmergencyCount(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//获取紧急数量

	void  QueryAllElementPriorityConfig(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//获取所有元素的优先配置

	void  QueryAllElementPriorityStatus(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//获取所有元素的优先状态

	void  QueryAllElementEmergencyConfig(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//获取所有元素的紧急配置

	void  QueryAllElementEmergencyStatus(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//获取所有元素的紧急状态

	void  QueryPriorityConfig(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//获取定元素的优先配置

	void  QueryPriorityStatus(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//获取定元素的优先状态

	void  QueryEmergencyConfig(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//获取定元素的紧急配置

	void  QueryEmergencyStatus(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//获取定元素的紧急状态

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void  QueryPatternCount(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//获取方案数量

	void  QueryAllElementPatternConfig(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//获取所有元素的方案

	void  QueryPatternConfig(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//获取指定元素的方案配置

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void  QueryAllElementTransitionRetainConfig(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//获取所有元素的过渡约束

	void  QueryTransitionRetainConfig(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//获取指定元素的过渡配置

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void  QueryDayPlanCount(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//获取日计划数量

	void  QueryAllElementDayPlanConfig(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//获取所有元素的日计划

	void  QueryDayPlanConfig(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//获取指定元素的日计划配置

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void  QueryScheduleTableCount(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//获取调度数量

	void  QueryAllElementScheduleTableConfig(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//获取所有元素的调度

	void  QueryScheduleTableConfig(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//获取指定元素的日计划配置

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void  QueryDeveiceStatus(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//获取设备状态

	void  QueryControlStatus(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//获取控制状态

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void  QueryAllElementStatisticData(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//查询所有元素的统计数据

	void  QueryRealTimeData(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//获取实时数据

	void  QueryStatisticData(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//获取统计数据

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void  QueryAlarmDataCount(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//获取报警数量

	void  QueryAllAlarmData(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//获取所有报警数据

	void  QueryAllElementAlarmData(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//获取所有元素的报警数据

	void  QueryAlarmData(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//获取指定元素的报警数据

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void  QueryFaultDataCount(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//获取故障数量

	void  QueryAllFaultData(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//获取所有故障数据

	void  QueryAllElementFaultData(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//获取所有元素的故障数据

	void  QueryFaultData(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//获取指定元素的故障数据

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void  QueryCentreControlTable(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//获取中心控制表

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void  QueryOrderPipeTable(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//获取命令管道

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void  QueryOverlapCount(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//获取所有跟随相位数 --HPH 2021.12.07

	void  QueryAllElementOverlapConfig(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//获取所有元素的跟随相位 --HPH 2021.12.07

	void  QueryOverlapConfig(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//获取跟随相位 --HPH 2021.12.07

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void  CreateSetReturnData(BYTE byRet, BYTE byIndex, BYTE byDataClassID, BYTE byObjectID, BYTE byAttributeID, BYTE byElementID, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//生成设置参数返回结构体

	void  SetDeviceInfo(BYTE byIndex, BYTE byObjectID, BYTE byAttributeID, BYTE byElementID, BYTE byDataValuePos, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData, BYTE byDataValueLength);//设置配置日期

	void  SetBaseInfo(BYTE byIndex, BYTE byObjectID, BYTE byAttributeID, BYTE byElementID, BYTE byDataValuePos, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData, BYTE byDataValueLength);//设置基础信息

	void  SetLightGroupInfo(BYTE byIndex, BYTE byObjectID, BYTE byAttributeID, BYTE byElementID, BYTE byDataValuePos, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData, BYTE byDataValueLength);//设置灯组信息

	void  SetLightGroupConfigInfo(BYTE byIndex, BYTE byObjectID, BYTE byAttributeID, BYTE byElementID, BYTE byDataValuePos, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData, BYTE byDataValueLength);//设置灯组配置信息

	void  SetLightGroupControlInfo(BYTE byIndex, BYTE byObjectID, BYTE byAttributeID, BYTE byElementID, BYTE byDataValuePos, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData, BYTE byDataValueLength);//设置灯组控制信息

	void  SetPhaseInfo(BYTE byIndex, BYTE byObjectID, BYTE byAttributeID, BYTE byElementID, BYTE byDataValuePos, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData, BYTE byDataValueLength);//设置相位信息

	void  SetPhaseConfigInfo(BYTE byIndex, BYTE byObjectID, BYTE byAttributeID, BYTE byElementID, BYTE byDataValuePos, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData, BYTE byDataValueLength);//设置相位配置信息

	void  SetPhaseControlInfo(BYTE byIndex, BYTE byObjectID, BYTE byAttributeID, BYTE byElementID, BYTE byDataValuePos, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData, BYTE byDataValueLength);//设置相位控制信息

	void  SetDetectorInfo(BYTE byIndex, BYTE byObjectID, BYTE byAttributeID, BYTE byElementID, BYTE byDataValuePos, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData, BYTE byDataValueLength);//设置检测器信息

	void  SetDetectorConfigInfo(BYTE byIndex, BYTE byObjectID, BYTE byAttributeID, BYTE byElementID, BYTE byDataValuePos, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData, BYTE byDataValueLength);//设置检测器配置信息

	void  SetPhaseStageInfo(BYTE byIndex, BYTE byObjectID, BYTE byAttributeID, BYTE byElementID, BYTE byDataValuePos, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData, BYTE byDataValueLength);//设置相位阶段信息

	void  SetPhaseStageConfigInfo(BYTE byIndex, BYTE byObjectID, BYTE byAttributeID, BYTE byElementID, BYTE byDataValuePos, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData, BYTE byDataValueLength);//设置相位阶段配置信息

	void  SetPhaseStageControlInfo(BYTE byIndex, BYTE byObjectID, BYTE byAttributeID, BYTE byElementID, BYTE byDataValuePos, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData, BYTE byDataValueLength);//设置相位阶段控制信息

	void  SetPhaseSafetyInfo(BYTE byIndex, BYTE byObjectID, BYTE byAttributeID, BYTE byElementID, BYTE byDataValuePos, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData, BYTE byDataValueLength);//设置相位安全信息

	void  SetPhaseConflictInfo(BYTE byIndex, BYTE byObjectID, BYTE byAttributeID, BYTE byElementID, BYTE byDataValuePos, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData, BYTE byDataValueLength);//设置相位绿冲突配置信息

	void  SetPhaseGreenIntervalInfo(BYTE byIndex, BYTE byObjectID, BYTE byAttributeID, BYTE byElementID, BYTE byDataValuePos, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData, BYTE byDataValueLength);//设置相位绿间隔配置信息

	void  SetEmergencyAndPriorityInfo(BYTE byIndex, BYTE byObjectID, BYTE byAttributeID, BYTE byElementID, BYTE byDataValuePos, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData, BYTE byDataValueLength);//设置紧急优先信息

	void  SetPriorityConfigInfo(BYTE byIndex, BYTE byObjectID, BYTE byAttributeID, BYTE byElementID, BYTE byDataValuePos, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData, BYTE byDataValueLength);//设置优先配置信息

	void  SetEmergencyConfigInfo(BYTE byIndex, BYTE byObjectID, BYTE byAttributeID, BYTE byElementID, BYTE byDataValuePos, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData, BYTE byDataValueLength);//设置紧急配置信息

	void  SetPatternInfo(BYTE byIndex, BYTE byObjectID, BYTE byAttributeID, BYTE byElementID, BYTE byDataValuePos, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData, BYTE byDataValueLength);//设置方案信息

	void  SetPatternConfigInfo(BYTE byIndex, BYTE byObjectID, BYTE byAttributeID, BYTE byElementID, BYTE byDataValuePos, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData, BYTE byDataValueLength);//设置方案配置信息

	void  SetTransitionRetain(BYTE byIndex, BYTE byObjectID, BYTE byAttributeID, BYTE byElementID, BYTE byDataValuePos, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData, BYTE byDataValueLength);//设置过渡约束信息

	void  SetDayPlanInfo(BYTE byIndex, BYTE byObjectID, BYTE byAttributeID, BYTE byElementID, BYTE byDataValuePos, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData, BYTE byDataValueLength);//设置日计划信息

	void  SetDayPlanConfigInfo(BYTE byIndex, BYTE byObjectID, BYTE byAttributeID, BYTE byElementID, BYTE byDataValuePos, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData, BYTE byDataValueLength);//设置日计划配置信息

	void  SetScheduleTable(BYTE byIndex, BYTE byObjectID, BYTE byAttributeID, BYTE byElementID, BYTE byDataValuePos, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData, BYTE byDataValueLength);//设置调度表信息

	void  SetScheduleTableConfig(BYTE byIndex, BYTE byObjectID, BYTE byAttributeID, BYTE byElementID, BYTE byDataValuePos, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData, BYTE byDataValueLength);//设置调度表配置信息

	void  SetRunStatusInfo(BYTE byIndex, BYTE byObjectID, BYTE byAttributeID, BYTE byElementID, BYTE byDataValuePos, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData, BYTE byDataValueLength);//设置运行状态信息

	void  SetATCStandardTime(BYTE byDataValuePos);//设置标准时间

	void  SetATCLocalTime(BYTE byDataValuePos);//设置本地时间

	void  SetCenterControl(BYTE byIndex, BYTE byObjectID, BYTE byAttributeID, BYTE byElementID, BYTE byDataValuePos, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData, BYTE byDataValueLength);//设置中心控制信息

	void  SetCenterControlConfig(BYTE byIndex, BYTE byObjectID, BYTE byAttributeID, BYTE byElementID, BYTE byDataValuePos, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData, BYTE byDataValueLength);//设置中心控制信息

	void  SetOrderPipe(BYTE byIndex, BYTE byObjectID, BYTE byAttributeID, BYTE byElementID, BYTE byDataValuePos, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData, BYTE byDataValueLength);//设置命令管道信息

	//void  SetPrivateData(BYTE byIndex, BYTE byObjectID, BYTE byAttributeID, BYTE byElementID, BYTE byDataValuePos, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData, BYTE byDataValueLength);//设置私有数据类 --HPH 2021.12.06

	void  SetOverlapConfig(BYTE byIndex, BYTE byObjectID, BYTE byAttributeID, BYTE byElementID, BYTE byDataValuePos, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData, BYTE byDataValueLength);//设置私有数据类:跟随相位 --HPH 2021.12.06

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void  CreateWrongQueryReturnData(BYTE byIndex, TReturnData tWrongReturnData);//生成查询错误返回结构体

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void  AckCtl_ReturnData(bool bCorrect, TReturnData & tReturnData);//应答查询返回

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////
	int   AckCtl_AskHeart();//应答心跳

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void  AckCtl_AskDeviceInfo(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void  AckCtl_AskBaseInfo(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void  AckCtl_AskLightGroupInfo(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void  AckCtl_AskPhaseInfo(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void  AckCtl_AskDetectorInfo(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void  AckCtl_AskPhaseStageInfo(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void  AckCtl_AskPhaseSafetyInfo(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void  AckCtl_AskEmergencyPriorityInfo(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void  AckCtl_AskPatternInfo(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void  AckCtl_AskTransitionRetain(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void  AckCtl_AskDayPlanInfo(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void  AckCtl_AskScheduleTable(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void  AckCtl_AskDeviceStatus(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);

	void  AckCtl_AskRunStatus(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void  AckCtl_AskTrafficData(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void  AckCtl_AskAlarmData(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void  AckCtl_AskFaultData(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void  AckCtl_AskCenterControl(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void  AckCtl_AskOrderPipe(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void  AckCtl_SetParamInfo(bool bCorrect, TReturnData & tReturnData);//应答参数设置

	////////////////////////////////////////////////////////////////////////////////////////////////////////////HPH 2021.12.06
	void  AckCtl_AskPrivateData(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//获取阶段运行时间以及剩余时间
	void GetStageRunTime(short &tRunTime, short &tRemainTime, int StageIndex, TPhaseRunStatus tPhaseRunStatus);

	void GetRoadTwoStageRunTime(short &tRunTime, short &tRemainTime, unsigned char & StageStatus, int StageIndex);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//生成故障信息
	void  AddFaultInfo(BYTE FalutType, BYTE FalutAction);

	void  GetSystemTimeZone(int & nTimeZoneHour, int & nTimeZoneMinute);

	void  SetSystemTimeZone();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////

    COMMWITHGB20999HANDLE            m_hThread;
    unsigned long                    m_dwThreadRet;
	bool                             m_bExitFlag;
    int                              m_nDetachState;

	COpenATCParameter         *      m_pOpenATCParameter;

	COpenATCRunStatus         *      m_pOpenATCRunStatus;

	COpenATCLog               *      m_pOpenATCLog;

	COpenATCPackUnpackBase   *		 m_pDataPackUnpackMode;

	COpenATCOperationRecord  m_tOpenATCOperationRecord;  //操作记录对象

	//-COpenATCParamCheck       m_openATCParamCheck;

	unsigned char            *		 m_chRecvBuff;

	unsigned char            *       m_chSendBuff;

	unsigned char            *       m_chPackedBuff;

	unsigned char            *		 m_chUnPackedBuff;

	time_t				             m_lastReadOkTime;

	int                              m_nSendTimeOut;

	int                              m_nRecvTimeOut;

	char                             m_szPeerIp[20];

	BYTE                             m_byFrameID;       

	TRunFaultInfo                    m_tRunAlarmInfo[C_N_MAX_FAULT_COUNT]; //报警信息

	TRunFaultInfo                    m_tRunFaultInfo[C_N_MAX_FAULT_COUNT]; //故障信息

	TAsc20999Param					 m_tAscParamInfo;						//20999信号机参数

	TAsc20999Param					 m_tVerifyParamInfo;					//用于事务机制参数设置的信号机缓存

	TAscParam                        m_tParamInfo;	//用于获取的信号机参数缓存

	//TAscParam                        m_tVerifyParamInfo;//用于事务机制参数设置的信号机缓存

	bool							 m_bIsNeedSave;		//判断是否需要保存参数（用于非事务机制）

	COpenATCCfgCommHelper			 m_commHelper;

#ifndef _WIN32
	struct tm						 Alarm_tm;	//linux报警时间--HPH

	struct tm						 Utc_tm;	//UTC时间--HPH

	struct tm						 Local_tm;	//Local时间--HPH

#else
	SYSTEMTIME                       m_stAlarmTime;

	SYSTEMTIME                       m_stUTCTime;

	SYSTEMTIME                       m_stLocalTime;
#endif

	GB20999HANDLE                    m_hTDataProesshread;

    unsigned long				     m_dwDataProessThreadRet;

	static void *GB20999_CALLBACK    DBDataProessThread(void *pParam);

	int                              RunDBDataProessThread();

	TDBManagement					 m_tDBManagement;

	int                              m_nFaultDataIndex;

	int                              m_nFaultDataCount;

	BYTE m_nPipeInfo[16];

	bool m_bPhaseControlChange;
};

#endif // !ifndef OPENATCCOMMWITHGB20999THREAD_H
