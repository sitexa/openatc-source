/*=====================================================================
模块名 ：本文档内为脱机文档中的结构内容
文件名 ：OpenATCParamStructDefine.h
相关文件：
实现功能：信号机参数结构体定义
作者 ：
版权 ：<Copyright(c) 2019-2020 Suzhou Keda Technology Co.,Ltd. All right reserved.>
-----------------------------------------------------------------------
修改记录：
日 期           版本     修改人     走读人     修改记录
2019/9/14       V1.0               
=====================================================================*/
#ifndef OPENATCNEWPARAMSTRUCTDEFINE_H
#define OPENATCNEWPARAMSTRUCTDEFINE_H

#include "../Include/OpenATCParamConstDefine.h"

#define MAX_PHASE_COUNT_20999    64
#define MAX_DAY_PLAN_SIZE 128
#define MAX_VEC_DET_SIZE 128

#pragma warning (disable:4819)
//#pragma pack(push)//保存对齐状态
//#pragma pack(1)  //设置为1字节对齐

//信号机IPV4的网络配置
typedef struct tagAscIPV4NetCard
{
	BYTE m_chAscIPV4IP[4];			//信号机IPV4的IP地址
	BYTE m_chAscIPV4SubNet[4];		//信号机IPV4的子网掩码
	BYTE m_chAscIPV4GateWay[4];		//信号机IPV4的网关
}TAscIPV4NetCard, * PTAscIPV4NetCard;

//上位机机IPV4的网络配置
typedef struct tagCerterIPV4NetCard
{
	BYTE m_chCerterIPV4IP[4];		//上位机IPV4的IP地址
	WORD m_chCerterIPV4Port;		//上位机IPV4通讯端口
	BYTE m_chCerterIPV4Type;		//上位机IPV4通讯类型 1.TCP 2.UDP 3.RS232
}TCerterIPV4NetCard, * PTCerterIPV4NetCard;

//信号机IPV6的网络配置
typedef struct tagAscIPV6NetCard
{
	BYTE m_chAscIPV6IP[16];			//信号机IPV4的IP地址
	BYTE m_chAscIPV6SubNet[16];		//信号机IPV4的子网掩码
	BYTE m_chAscIPV6GateWay[16];		//信号机IPV4的网关
}TAscIPV6NetCard, * PTAscIPV6NetCard;

//上位机机IPV6的网络配置
typedef struct tagCerterIPV6NetCard
{
	BYTE m_chCerterIPV6IP[16];		//上位机IPV6的IP地址
	WORD m_chCerterIPV6Port;		//上位机IPV6通讯端口
	BYTE m_chCerterIPV6Type;		//上位机IPV6通讯类型 1.TCP 2.UDP 3.RS232
}TCerterIPV6NetCard, * PTCerterIPV6NetCard;

//设备信息
typedef struct tagDeviceInfo
{
	char m_chManufacturer[128];		//制造厂商
	BYTE m_byDeviceVersion[4];		//设备版本
	char m_chDeviceNum[16];			//设备编号
	BYTE m_byDateOfProduction[7];	//出厂日期
	BYTE m_byDateOfConfig[7];		//配置日期
}TDeviceInfo, * PTDeviceInfo;

//基础信息
typedef struct tagBaseInfo
{
	BYTE m_bySiteRoadID[128];			//信号机安装路口
	TAscIPV4NetCard m_stAscIPV4;		//信号机IPV4网络配置
	TCerterIPV4NetCard m_stCerterIPV4;	//上位机IPV4网络配置
	long m_wTimeZone;					//信号机所属时区
	DWORD m_wATCCode;					//信号机编号
	BYTE m_byCrossRoadNum;				//信号机控制路口数量
	bool m_bGpsClockFlag;				//GPS时钟标志
	TAscIPV6NetCard m_stAscIPV6;		//信号机IPV6网络配置
	TCerterIPV6NetCard m_stCerterIPV6;	//上位机IPV6网络配置
}TBaseInfo, *PTBaseInfo;

//灯组定义
typedef struct tag20999Channel
{
	BYTE m_byChannelNumber;		/*2.9.2.1通道号，不能大于maxChannels。(1..32)*/
	BYTE m_byLightControlType;	//1 机动车 2 非机动车 3 行人 4 车道 5 可变交通标志 6 公交专用灯具 7 有轨电车专用灯具 8 特殊公交
	BYTE m_byForbiddenFlag;		//通道禁止标志
	BYTE m_byScreenFlag;		//通道屏蔽标志
	BYTE m_byRoutineFlag;		//通道程序使用标志，0表示给主程序使用，1表示给辅程序使用
}T20999Channel, * PT20999Channel;

//相位定义
typedef struct tag20999Phase
{
	BYTE m_byPhaseNumber;			/*2.2.2.1 相位号，不能超过maxPhases所定义的值。(1..32)*/
	WORD m_wPhaseMinimumGreen;		/* 2.2.2.4最小绿灯时间，简称最小绿。*/
	WORD m_wPhaseMaximum1;			/* 2.2.2.6最大绿灯时间1，简称最大绿1。(0..255)*/
	WORD m_wPhaseMaximum2;			/* 2.2.2.7最大绿灯时间2，简称最大绿2。(0..255)*/
	//20999
	BYTE m_byLightGroup[8];         //相位的灯组
	BYTE m_byPhaseCall[8];          //相位需求
	//BYTE m_byConflictPhase[8];      //冲突相位序列
	BYTE m_byForbiddenFlag;         //相位禁止标志
	BYTE m_byScreenFlag;			//相位屏蔽标志
	BYTE m_nLoseControlLampOneType;
	BYTE m_nLoseControlLampOneTime;
	BYTE m_nLoseControlLampTwoType;
	BYTE m_nLoseControlLampTwoTime;
	BYTE m_nLoseControlLampThreeType;
	BYTE m_nLoseControlLampThreeTime;
	BYTE m_nGetControlLampOneType;
	BYTE m_nGetControlLampOneTime;
	BYTE m_nGetControlLampTwoType;
	BYTE m_nGetControlLampTwoTime;
	BYTE m_nGetControlLampThreeType;
	BYTE m_nGetControlLampThreeTime;
	BYTE m_nPowerOnGetControlLampOneType;
	BYTE m_nPowerOnGetControlLampOneTime;
	BYTE m_nPowerOnGetControlLampTwoType;
	BYTE m_nPowerOnGetControlLampTwoTime;
	BYTE m_nPowerOnGetControlLampThreeType;
	BYTE m_nPowerOnGetControlLampThreeTime;
	BYTE m_nPowerOnLossControlLampOneType;
	BYTE m_nPowerOnLossControlLampOneTime;
	BYTE m_nPowerOnLossControlLampTwoType;
	BYTE m_nPowerOnLossControlLampTwoTime;
	BYTE m_nPowerOnLossControlLampThreeType;
	BYTE m_nPowerOnLossControlLampThreeTime;
	BYTE m_byPhaseExtend;				//相位延长绿时间，20999单位为0.1秒
}T20999Phase, * PT20999Phase;

//车辆检测器定义
typedef struct tag20999VehicleDetector
{
	BYTE m_byVehicleDetectorNumber;     /*2.3.2.1车辆检测器序列号。(1..48)*/
	BYTE m_byDetectorType;		//20999参数
	WORD m_wFlowGatherCycle;//流量采集周期
	WORD m_wOccupancyGatherCycle;//占用率采集周期
	char m_chFixPosition[MAX_MODULE_STRING_LENGTH];//安装位置
}T20999VehicleDetector, * PT20999VehicleDetector;

//20999 相位阶段
typedef struct tag20999PhaseStage
{
	BYTE m_byPhaseStageNumber;						//相位阶段编号
	BYTE m_byPhase[8];								//相位阶段的相位
	BYTE m_byLaterStartTime[MAX_PHASE_COUNT_20999]; //阶段中相位晚启动时间
	BYTE m_byEarlyEndTime[MAX_PHASE_COUNT_20999];   //阶段中相位早结束时间

	bool m_bSoftCall;                              //软件需求
	bool m_bScreen;                                //相位阶段屏蔽                        
	bool m_bForbidden;                             //相位阶段禁止
}T20999PhaseStage, * PT20999PhaseStage;

//相位安全信息20999  相位冲突表
typedef struct tagPhaseConflictInfo
{
	BYTE m_byPhaseNumber;
	BYTE m_byConflictSequenceInfo[8];	//冲突序列
}TPhaseConflictInfo, * PTPhaseConflictInfo;

//相位绿间隔配置表
typedef struct tagPhaseGreenGap
{
	BYTE m_byPhaseNumber;
	BYTE m_byGreenGapSequenceInfo[64];	//绿间隔序列
}TPhaseGreenGapInfo, * PTPhaseGreenGapInfo;

//优先信息20999
typedef struct tagPriorityInfo
{
	BYTE m_byPrioritySignalNumber;		//优先信号编号
	BYTE m_byPrioritySignalPhaseStage;	//优先信号申请相位阶段
	BYTE m_byPrioritySignalGrade;		//优先信号申请优先级
	BYTE m_blPrioritySignalScreen;		//优先信号屏蔽标志
	BYTE m_byPrioritySignalSource;      //优先信号来源(检测器编号)
}TPriorityInfo, * PTPriorityInfo;

//紧急信息20999
typedef struct tagEmergyInfo
{
	BYTE m_byEmergySignalID;			//紧急信号编号	
	BYTE m_byEmergySignalPhaseStage;	//紧急信号申请相位阶段
	BYTE m_byEmergySignalGrade;			//紧急信号申请优先级
	BYTE m_bEmergySignalScreen;			//紧急信号屏蔽标志
	BYTE m_byEmergySignalSource;        //紧急信号来源(检测器编号)
}TEmergyInfo, * PTEmergyInfo;

//方案定义
typedef struct tag20999Pattern
{
	BYTE m_byPatternNumber;		/*2.5.7.1 方案表中该行属于哪个方案*/
	BYTE m_byRoadID;				   //方案所属路口序号
	BYTE m_byCoorditionStage;		   //方案的协调序号
	DWORD m_wPatternCycleTime;		   //方案的周期
	WORD m_byPatternOffsetTime;		   //方案相位差时间
	BYTE m_byPatternStage[16];		   //方案相位阶段链
	BYTE m_byPatternStageTime[32];	   //方案相位阶段链时间
	BYTE m_byPatternStageOccurType[16];//方案相位阶段出现类型
	//BYTE m_byForbiddenStage[16];       //方案中被禁止的相位阶段
	//BYTE m_byScreenStage[16];          //方案中被屏蔽的相位阶段
}T20999Pattern, * PT20999Pattern;

//过渡约束20999
typedef struct tagTransitBound
{
	BYTE m_byPhaseStageNumber;
	BYTE m_byTransitBound[MAX_PHASE_COUNT_20999];
}TTransitBound, * PTTransitBound;

//日计划20999
typedef struct tagDayPlanInfo
{
	BYTE m_byDayPlanID;					//日计划编号
	BYTE m_byRoadID;					//日计划所属路口编号
	BYTE m_byDayPlanStartTime[96];		//时段开始时间链
	BYTE m_byDayPlanActionPattern[48];	//时段开始方案链
	BYTE m_byDayPlanRunMode[48];		//时段运行模式链
	BYTE m_byActionChainOne[96];		//动作1
	BYTE m_byActionChainTwo[96];		//动作2
	BYTE m_byActionChainThree[96];		//动作3
	BYTE m_byActionChainFour[96];		//动作4
	BYTE m_byActionChainFive[96];		//动作5
	BYTE m_byActionChainSix[96];		//动作6
	BYTE m_byActionChainSeven[96];		//动作7
	BYTE m_byActionChainEight[96];		//动作8
}TDayPlanInfo, * PTDayPlanInfo;

//调度计划20999
typedef struct tagSchedulePlanInfo
{
	BYTE m_SchedulePlanID;				//调度计划编号
	BYTE m_byRoadID;					//调度计划所属路口
	BYTE m_byPriority;					//调度表优先级
	BYTE m_byWeek;						//星期值
	WORD m_byMonth;						//月份值
	DWORD m_byDate;						//日期值
	BYTE m_byScheduleOfDayPlanID;	    //日计划编号
}TSchedulePlanInfo, * PTSchedulePlanInfo;

//中心控制信息20999
typedef struct tagCenterCtrlInfo
{
	BYTE m_byIntersectionID;//路口序号
	BYTE m_byPhaseStage;	//指定相位阶段
	BYTE m_byPattern;		//指定方案
	BYTE m_byRunMode;		//指定运行模式
}TCenterCtrlInfo, * PTCenterCtrlInfo;

typedef struct tagAsc20999Param
{
	TDeviceInfo m_stDeviceInfo;											//设备信息

	TBaseInfo m_stBaseInfo;												//信号机基础信息

	int m_stChannelTableValidSize;										//有效的通道表数量
	T20999Channel m_stAscChannelTable[MAX_PHASE_COUNT_20999];			//通道表(灯组)

	int m_stPhaseTableValidSize;										//有效的相位表数量
	T20999Phase m_stAscPhaseTable[MAX_PHASE_COUNT_20999];				//相位表

	int m_stVehicleDetectorTableValidSize;								//有效的车辆检测器数量
	T20999VehicleDetector m_stAscVehicleDetectorTable[MAX_VEC_DET_SIZE];//车辆检测器表

	int m_stPhaseStageValidSize;										//有效的相位阶段表数量
	T20999PhaseStage m_stPhaseStage[MAX_PHASE_COUNT_20999];				//相位阶段

	int m_stPhaseConflictValidSize;
	TPhaseConflictInfo m_stPhaseConflictInfo[MAX_PHASE_COUNT_20999];	//相位绿冲突信息

	int m_stPhaseGreenGapValidSize;
	TPhaseGreenGapInfo m_stPhaseGreenGapInfo[MAX_PHASE_COUNT_20999];	//相位绿间隔信息

	int m_stPriorityValidSize;
	TPriorityInfo m_stPriorityInfo[MAX_PHASE_COUNT_20999];				//优先信息

	int m_stEmergyValidSize;
	TEmergyInfo m_stEmergyInfo[MAX_PHASE_COUNT_20999];					//紧急信息

	int m_stPatternTableValidSize;
	T20999Pattern m_stAscPatternTable[MAX_DAY_PLAN_SIZE];				//方案表

	int m_stTransitBoundValidSize;
	TTransitBound m_stTransitBound[MAX_PHASE_COUNT_20999];				//过渡约束

	int m_stDayPlanValidSize;
	TDayPlanInfo m_stDayPlanInfo[MAX_DAY_PLAN_SIZE];					//日计划

	int m_stSchedulePlanValidSize;
	TSchedulePlanInfo m_stSchedulePlanInfo[MAX_DAY_PLAN_SIZE];			//调度计划

	TCenterCtrlInfo m_stCenterCtrlInfo;									//中心控制信息
}TAsc20999Param, * PTAsc20999Param;
#endif 
