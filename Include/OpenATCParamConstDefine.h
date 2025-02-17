#pragma once

typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned long DWORD;
//信号机特征参数配置常量定义文件
#define     MAX_PANEL_KEY_CFG_COUNT             12      //手动面板可配置按键数量
#define		MAX_SITEID_COUNT					60		//地址码大小
#define		MAX_ROADNO_LENGTH					2		//路口id大小
#define		MAX_IPSTR_LEN						25		//网卡ip
#define		MAX_NETCARD_TABLE_COUNT			    2		//网卡数量
#define     MAX_FAULT_TABLE_COUNT				500     //故障表大小
#define     MAX_FAULT_DESC_LEN				    64      //故障描述长度
#define	    MAX_OPERATIONRECORD_DESC_LEN		256	    //操作记录描述长度
#define		MAX_PHASE_COUNT					    40		//相位表最大行数
#define		MAX_LPHASE_COUNT					12		//车道灯相位表最大行数
#define		MAX_PHASE_CONCURRENCY_COUNT		    40		//最大并发相位数
#define		MAX_PHASE_STATUS_COUNT				2		//相位状态组表最大行数
#define		MAX_PHASE_CONTROL_COUNT			    4		//相位控制组表最大行数
#define		GREEN_TIME_CALCULATE_TIME			2		//相位连续使用最大绿1次数

#define		MAX_VEHICLEDETECTOR_COUNT			64	    //车辆检测器表最大行数 trap
#define		MAX_VEHICLEDETECTOR_COUNT_S		    48 		//车辆检测器表最大行数 snmp
#define		MAX_VEHICLEDETECTOR_STATUS_COUNT	8		//车辆检测器状态组表最大行数
#define		MAX_PEDESTRIANDETECTOR_COUNT		8		//行人检测器表最大行数

#define		MAX_DESC_LENGTH						32		//描述最大长度

#define		MAX_VOLUMEOCCUPANCY_COUNT			MAX_VEHICLEDETECTOR_COUNT		//流量占有率表最大行数
#define		MAX_PATTERN_COUNT					109		//方案表数
#define		MAX_SPLIT_COUNT					    36		//绿信比表最大行数

#define		MAX_OFFSET_COUNT					3		//绿信比表对应的相位差总数

#define		MAX_TIMEBASE_ACTION_COUNT		    255		//时基表最大行数
#define		MAX_SCHEDULE_COUNT			        255		//调度计划表最大行数

#define		MAX_DAYPLAN_TABLE_COUNT		  	    16		//最大时段表数
#define		MAX_SINGLE_DAYPLAN_COUNT			48		//每个时段表最大数(即，最大DAYPLANEVNET)

#define		MAX_SEQUENCE_TABLE_COUNT			32		//时序方案表数
#define		MAX_RING_COUNT					    4		//时序方案表最大行数（即最大的ring数）
#define		MAX_SEQUENCE_DATA_LENGTH			32		//时序数据的最大长度

#define		MAX_CHANNEL_COUNT					80		//通道表最大行数
#define		MAX_CHANNEL_STATUS_COUNT			4		//通道状态组表最大行数

#define		MAX_CHANNEL_LOCK_COUNT				48		//通道锁定表最大行数

#define		MAX_OVERLAP_COUNT					16		//重叠表最大行数
#define		MAX_PHASE_COUNT_IN_OVERLAP			40		//重叠中的最大include相位数
#define		MAX_PHASE_COUNT_MO_OVERLAP			16		//重叠中的最大Modifier相位数
#define		MAX_OVERLAP_STATUS_COUNT			2		//重叠状态表最大行数
#define		MAX_MODULE_COUNT					255		//模块表最大行数
#define		MAX_MODULE_STRING_LENGTH			128		//模块中字符串长度

#define		MAX_EVENTLOG_COUNT				    255		//事件表最大行数
#define		MAX_EVENTCLASS_COUNT				1		//事件类型最大值，实际为3
#define		MAX_EVENTCONFIG_COUNT				50		//事件配置表最大行

#define		MAX_PREEMPT_COUNT					8		//优先一致组表数
#define		MAX_PEDESTRIANPHASE_COUNT			8  		//行人相位表行数

#define		MAX_COUNTDOWNBOARD_COUNT			24		//最大倒计时牌个数
#define		MAX_COUNTDOWNBOARD_INCLUDEDPHASES	16		//每个倒计时牌所能包含的最多相位数

#define		MAX_SPECIALFUNCOUTPUT_COUNT		    8		//最大支持的特殊功能行数
#define		MAX_DYNPATTERNSEL_COUNT			    8		//最大动态方案选择配置表行数
#define		MAX_RS232PORT_COUNT			 	    3		//最大支持的端口配置表行数
#define     MAX_SIGNALTRANS_LIMIT	            20		//信号转换序列配置时间限制
#define     MAX_ALLSTOPPHASE_LIMIT              32      //跟随相位号限制 
#define		MAX_BYTE_VALUE			            255
#define		MAX_2BYTE_VALUE			            65535

#define		MAX_TRANSINTENSITYCHOICE_COUNT		90		//交通强度周期选择表最大行数
#define     MAX_PHASE_TYPE_COUNT                3       //相位类型   

#define     MAX_ITSCONSERI_COUNT                6       //串口表最大表行数    
#define     MAX_ITSCONIO_COUNT                  24      //IO 配置表最大行数   
#define     MAX_ITSCONCHANNEL_COUNT             32      //冲突通道表最大行数  
#define	    MAX_STAGE_COUNT					    40	    //阶段最大数
#define     C_N_MAX_LAMP_OUTPUT_NUM				240     //最大灯控输出端子个数

typedef enum tagPhaseSrcType
{
    PHASE_SRC = 2,
    OVERLAP_SRC = 4,
}EPhaseSrcType;

typedef enum tagLogicCtlMode
{
    CTL_MODE_SELFCTL		        = 0,						//自主控制
    CTL_MODE_FLASH			        = 1,						//黄闪
    CTL_MODE_ALLRED			        = 2,						//全红
    CTL_MODE_OFF			        = 3,						//关灯
    CTL_MODE_MANUAL			        = 4,						//手动控制
    CTL_MODE_FIXTIME		        = 5,						//定周期控制
    CTL_MODE_ACTUATE		        = 6,						//单点感应控制
    CTL_MODE_ADVACTUATE		        = 7,						//协调感应控制
    CTL_MODE_SELPLAN		        = 8,						//方案选择控制
    CTL_MODE_SINGLEOPTIM	        = 9,						//自适应控制
    CTL_MODE_CABLELESS		        = 10,						//无电缆控制
    CTL_MODE_CABLEL			        = 11,						//有电缆控制
    CTL_MODE_PEDCROSTREET	        = 12,						//行人过街
	CTL_MODE_MANUAL_RECOVER			= 13,				        //方案恢复过渡
	CTL_MODE_PHASE_STAY	            = 14,						//相位驻留
	CTL_MODE_CHANNEL_CHECK 	        = 15,						//通道检测
	CTL_MODE_CHANNEL_LOCK 	        = 16,						//通道锁定
	CTL_MODE_WEBSTER_OPTIM	        = 17,						//Webster单点控制
	CTL_MODE_ACTUATE_PEDCROSTREET   = 19,                       //感应式行人过街
	CTL_MODE_PHASE_LOCK 	        = 22,						//相位锁定
	CTL_MODE_PHASE_PASS_CONTROL		= 23,                       //相位放行控制
	CTL_MODE_PREEMPT		        = 24,                       //优先控制

	CTL_MODE_PANEL_TRAN             = 51,				        //手动面板步进过渡时的控制模式值
	CTL_MODE_SYSTEM_TRAN            = 52,				        //系统手动回自主

	CTL_MODE_SYS_INTERRUPT          = 20,                       //方案干预
	CTL_MODE_MANUAL_CONTROL_PATTERN = 100,                      //手动控制方案

    CTL_MODE_UNDEFINE		        = 255,
}ELogicCtlMode;

typedef enum tagChannelSrc
{
	DISABLE_CHA			= 0,							//不启用	
    OTHER_CHA			= 1,							//其他通道	
    VEH_CHA				= 2,							//机动车通道
    PED_CHA				= 3,							//行人通道
    OVERLAP_CHA 		= 4,							//机动车跟随通道
	OVERLAP_PED_CHA		= 5,							//行人跟随通道
    LANEWAY_LIGHT_CHA	= 6,							//车道灯
}EChannelSrc;

#define     OPENATC_RTN_OK		1
#define     OPENATC_RTN_FAILED	0

typedef enum tagSaveParameterFailedVal
{
	OPENATC_SAVE_PARAM_FAILED_USB_NOT_FIND				= 2,
	OPENATC_SAVE_PARAM_FAILED_USB_MOUNT_FAILED			= 3,
	OPENATC_SAVE_PARAM_FAILED_PARAM_UPDATE_FAILED		= 4,
	OPENATC_SAVE_PARAM_FAILED_DEVPARAM_UPDATE_FAILED	= 5,
}ESaveParameterFailedVal;

enum{
	HWPANEL_BTN_AUTO						= 0x00,
	HWPANEL_BTN_MANUAL						= 0x01,
	HWPANEL_BTN_STEP						= 0x02,
	HWPANEL_BTN_ALL_RED						= 0x03,
	HWPANEL_BTN_YELLOW_FLASH				= 0x04,
	HWPANEL_BTN_DIR_EAST_WEST_STRAIGHT		= 0x05,
	HWPANEL_BTN_DIR_EAST_WEST_TURN_LEFT		= 0x06,
	HWPANEL_BTN_DIR_SOUTH_NORTH_STRAIGHT	= 0x07,
	HWPANEL_BTN_DIR_SOUTH_NORTH_TURN_LEFT	= 0x08,
	HWPANEL_BTN_DIR_EAST					= 0x09,
	HWPANEL_BTN_DIR_WEST					= 0x0A,
	HWPANEL_BTN_DIR_SOUTH					= 0x0B,
	HWPANEL_BTN_DIR_NORTH					= 0x0C,
	HWPANEL_BTN_REMOTE_CTRL					= 0x0D,
	HWPANEL_BTN_Y1							= 0x0E,
	HWPANEL_BTN_Y2							= 0x0F,
	HWPANEL_BTN_Y3							= 0x10,
	HWPANEL_BTN_Y4							= 0x11,
};

enum{
	HWPANEL_DIR_EAST_WEST_STRAIGHT_INDEX	= 1,
    HWPANEL_DIR_NORTH_INDEX					= 2,
	HWPANEL_DIR_EAST_WEST_TURN_LEFT_INDEX	= 3,
    HWPANEL_DIR_WEST_INDEX                  = 4,
    HWPANEL_DIR_EAST_INDEX					= 5,
	HWPANEL_DIR_SOUTH_NORTH_STRAIGHT_INDEX	= 6,
    HWPANEL_DIR_SOUTH_INDEX					= 7,
	HWPANEL_DIR_SOUTH_NORTH_TURN_LEFT_INDEX	= 8,
	HWPANEL_Y1_INDEX						= 9,
	HWPANEL_Y2_INDEX						= 10,
	HWPANEL_Y3_INDEX						= 11,
	HWPANEL_Y4_INDEX						= 12,
	HWPANEL_REMOTE_CTRL_INDEX				= 13,
};
enum
{
	REMOTE_CONTROL_PAIR_REQUEST		= 0X00,
};

enum
{
	REMOTE_CONTROL_PAIR_FAIL		= 0X00,
	REMOTE_CONTROL_PAIR_SUCCESS		= 0X01,
};

typedef enum tagSplitMode
{
    OTHER_MODE				= 1,
    UNDEFINE_MODE			= 2,
    MINVEHICLE_MODE			= 3,
    MAXVEHICLE_MODE			= 4,
    PED_MODE				= 5,
    MAXVEHICLEANDPED_MODE	= 6,
    NEGLECT_MODE			= 7,
	SHIELD_MODE				= 8,
}ESplitMode;

typedef enum tagDetectorType
{
    LOOP_DETECTOR 	= 0,
    VIDEO_DETECTOR 	= 1,
}EDetectorType;

#define     OPENATC_PARAM_OUTLIERS			-1
#define     OPENATC_PARAM_CHECK_OK			1
#define     OPENATC_PARAM_CHECK_FAILED		0
#define     MAX_UP_LIMIT_VALUE				255
#define     MAX_UP_LIMIT_HOUR				23
#define     MAX_UP_LIMIT_MINUTE				59
#define     MAX_UP_LIMIT_MON				12
#define     MAX_UP_LIMIT_WEEK				6
#define     MAX_UP_LIMIT_DATE				31
#define     MAX_UP_LIMIT_CONTROL_NUM		12
#define     MAX_UP_LIMIT_LAMP_TIME			25


const int C_N_MAX_MD5BUFF_SIZE = 33;
const int C_N_MAX_ERR_SIZE = 50;
const int C_N_NUM1 = 1;
const int C_N_NUM2 = 2;
const int C_N_NUM3 = 3;
const int C_N_NUM4 = 4;
const int SPLITMODE_IGNORE = 7;
const int SPLITMODE_OTHER = 1;
const int HOUR_TO_MIN = 60;

typedef enum tagCheckPhaseCode
{
	CheckPhase_ID = 101,      //相位编号超出限值
	CheckPhase_PedClear = 102,      //行人绿闪时间超出限值
	CheckPhase_MinGreen = 103,      //最小绿应大于行人绿灯时间
	CheckPhase_Max1 = 104,      //最大绿1应大于最小绿时间
	CheckPhase_Max2 = 105,      //最大绿2应大于最大绿1时间
	CheckPhase_Passage = 106,      //单位延长绿灯时间超出限值
	CheckPhase_Yellow = 107,      //黄灯时间超出限值
	CheckPhase_RedClear = 108,      //全红时间超出限值
	CheckPhase_FlashGreen = 109,      //绿闪时间应小于最小绿
	CheckPhase_Ring = 110,      //环数量超出限值
	CheckPhase_ConCurConflict = 111,		//相位并发配置冲突
	CheckPhase_OneRingOnePhase = 112,		//所有环不能只配一个相位
	CheckPhase_RingIndexStartFromFaultValue = 113,		//环索引应从1开始配置
	CheckPhase_RingIndexDiscontinuous = 114,		//环索引应连续配置
}ECheckPhaseCode;

typedef enum tagCheckOverlapCode
{
	CheckOverlap_ID = 201,		//跟随相位数量超出限值
	CheckOverlap_IncludePhaseNull = 202,		//跟随相位的母相位为空
	CheckOverlap_IncludePhase = 203,		//跟随相位配置未知母相位
}ECheckOverlapCode;

typedef enum tagCheckPatternCode
{
	CheckPattern_ID = 301,		//方案数量超出限值
	CheckPattern_Offset = 302,		//相位差应小于周期时间
	CheckPattern_RingNoPhaseIndex = 303,		//环内配置未知相位
	CheckPattern_Split = 304,		//绿信比应大于相位的最小绿+黄灯+全红
	CheckPattern_PhaseConCurConflict = 305,		//方案中存在环内相位并发冲突，带参数1对应方案1，2对应方案2，一次类推，108代表方案108
	CheckPattern_InconsistentCycleTime = 306,		//方案中存在各个环周期时长不一致，带参数1对应方案1，2对应方案2，一次类推，108代表方案108
}ECheckPatternCode;

typedef enum tagCheckDayPlanCode
{
	CheckDayPlan_ID = 401,		//计划数量超出限值
	CheckDayPlan_Control = 402,		//控制方式不存在
	CheckDayPlan_DayPlanID = 403,		//时段数量超出限值
	CheckDayPlan_Minute = 404,		//分钟超出限值
	CheckDayPlan_Hour = 405,		//小时超出限值
	CheckDayPlan_TimeOrder = 406,		//时间顺序配置错误
	CheckDayPlan_PatternID = 407,		//计划中配置未知方案
	CheckDayPlan_PatternNull = 408,		//计划中方案未配置
	CheckDayPlan_Null = 409,		//存在未配置的计划
}ECheckDayPlanCode;

typedef enum tagCheckScheduleCode
{
	CheckSchedule_ID = 501,		//调度计划数量超出限值
	CheckSchedule_Month = 502,		//月份超出限值
	CheckSchedule_Week = 503,		//星期超出限值
	CheckSchedule_Date = 504,		//日期值超出限值
	CheckSchedule_PlanFlag = 505,		//配置未知计划号
	CheckSchedule_AllYear = 506,	//包含普通计划的日期未覆盖全年
	CheckSchedule_NoMonth = 507,	//调度计划未配置月份
}ECheckScheduleCode;

typedef enum tagCheckChannelCode
{
	CheckChannel_ID = 601,		//通道数超出限值
	CheckChannel_ControlSource = 602,		//通道配置未知控制源
	CheckChannel_ControlSourceNull = 603,		//通道控制源未配置
	CheckChannel_ControlTypeNull = 604,		//通道控制类型未配置
	CheckChannel_ControlType = 605,		//通道未知控制类型
}ECheckChannelCode;

typedef enum tagCheckVecDetetorCode
{
	CheckVecDetetor_ID = 701,		//车辆检测器数量超出限值
	CheckVecDetetor_NoActivity = 702,		//车辆检测器无响应时间超出限值
	CheckVecDetetor_MaxPresence = 703,		//车辆检测器最大持续时间超出限值
	CheckVecDetetor_ErraticCounts = 704,		//车辆检测器最大车辆数超出限值
	CheckVecDetetor_FailTime = 705,		//车辆检测器失败时间超出限值
	CheckVecDetetor_CallPhase = 706,		//车辆检测器配置未知请求相位
}ECheckVecDetetorCode;

typedef enum tagCheckPedDetetorCode
{
	CheckPedDetetor_ID = 801,		//行人检测器数量超出限值
	CheckPedDetetor_NoActivity = 802,		//行人检测器无响应时间超出限值
	CheckPedDetetor_MaxPresence = 803,		//行人检测器最大持续时间超出限值
	CheckPedDetetor_ErraticCounts = 804,		//行人检测器最大车辆数超出限值
	CheckPedDetetor_FailTime = 805,		//行人检测器失败时间超出限值
	CheckPedDetetor_CallPhase = 806,		//行人检测器配置未知请求相位
}ECheckPedDetetorCode;

typedef enum tagCheckManualPanelCode
{
	CheckManualPanel_ID = 901,		//手动面板配置未知通道
	CheckManualPanel_NULL = 902,		//手动面板参数未配置
	CheckManualPanel_GreenConflictKey1 = 903,		//手动面板东西直行按键通道绿冲突
	CheckManualPanel_GreenConflictKey2 = 904,		//手动面板北向通行按键通道绿冲突
	CheckManualPanel_GreenConflictKey3 = 905,		//手动面板东西左转按键通道绿冲突
	CheckManualPanel_GreenConflictKey4 = 906,		//手动面板西向通行按键通道绿冲突
	CheckManualPanel_GreenConflictKey5 = 907,		//手动面板东向通行按键通道绿冲突
	CheckManualPanel_GreenConflictKey6 = 908,		//手动面板南北直行按键通道绿冲突
	CheckManualPanel_GreenConflictKey7 = 909,		//手动面板南向通行按键通道绿冲突
	CheckManualPanel_GreenConflictKey8 = 910,		//手动面板南北左转按键通道绿冲突
	CheckManualPanel_GreenConflictKey9 = 911,		//手动面板Y1自定义按键通道绿冲突
	CheckManualPanel_GreenConflictKey10 = 912,      //手动面板Y2自定义按键通道绿冲突
	CheckManualPanel_GreenConflictKey11 = 913,      //手动面板Y3自定义按键通道绿冲突
	CheckManualPanel_GreenConflictKey12 = 914,      //手动面板Y4自定义按键通道绿冲突
	CheckManualPanel_DurationErr = 915,		//手动面板参数持续时间应不小于最小绿时间
}ECheckManualPanelCode;

typedef enum tagCheckChannelLockInfo
{
	CheckChannelLockInfo_Conflict = 1001,		//时段%通道状态锁定冲突   %代表时段编号
	CheckChannelLockInfo_Source = 1002,		//时段%锁定通道的控制源未被忽略   %代表时段编号
	CheckChannelLockInfo_ID = 1003,		//通道锁定时段数量超出限值
	CheckChannelLockInfo_IllegalCtrlSrc = 1004,		//车道灯以外的通道配置了通道锁定状态
}ECheckChannelLockInfo;

typedef enum tagCheckParamBaseInfo
{
	CheckParamBaseInfo_Check_MD5_Fail = 1101,		//MD5码值校验失败
	CheckParamBaseInfo_Update_Fail = 1102,		//特征参数同步失败
	CheckParamBaseInfo_JSON_Parse_Fail = 1103,		//参数JSON格式错误
}ECheckParamBaseInfo;

typedef enum tagCheckDeviceParamInfo
{
	CheckDeviceParamInfo_SiteID_NULL = 2001,		//信号机地址码未配置
	CheckDeviceParamInfo_NetCards = 2002,		//信号机两个网卡都未配置
	CheckDeviceParamInfo_Check_SiteID_Fail = 2003,		//信号机地址码配置错误
	CheckDeviceParamInfo_JSON_Parse_Fail = 2004,		//参数JSON格式错误
	CheckDeviceParamInfo_Update_Fail = 2005,		//设备参数同步失败
	CheckDeviceParamInfo_NetCards_Illegal = 2006,		//信号机配置非法网卡信息
	CheckDeviceParamInfo_RoadID_Illegal = 2007,		//信号机路口ID不在0-65535之间
}ECheckDeviceParamInfo;

typedef enum tagCheckUSBStatus
{
	CheckUSBStatus_USB_Mount_Fail = 3001,		//U盘挂载失败
	CheckUSBStatus_USB_Not_Find = 3002,		//未找到U盘
}ECheckUSBStatus;

typedef enum tagCheckSingleOptim
{
	CheckCheckSingleOptim_StaticWeight = 1201,		//权重因子值域应为[0,100]
	CheckCheckSingleOptim_SumOfPhaseWeight = 1202,		//环相位权重因子和不为100
	CheckCheckSingleOptim_Null = 1203,		//环相位权重因子配置为空
}ECheckSingleOptim;

typedef enum tagCheckChannelGreenConflict
{
	CheckChannelGreenConflict_Concurrency = 1301,		//两并发相位的通道不能配置成通道绿冲突
}ECheckChannelGreenConflict;

typedef enum tagCheckPartParam
{
	CheckParam_Pattern				            = 1,		//方案
	CheckParam_Plan  					        = 2,		//计划
	CheckParam_Date  					        = 3,		//日期
	CheckParam_OverLap 					        = 4,		//相位
}ECheckPartParam;
