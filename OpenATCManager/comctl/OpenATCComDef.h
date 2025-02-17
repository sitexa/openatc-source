#pragma once

#ifdef _WIN32
#include <stdio.h>
//#include <winsock2.h>
//#include <WS2tcpip.h>
#include <Windows.h>
#include <process.h>
#include <string.h>
#include <time.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <dlfcn.h>
#include <semaphore.h>
#include <time.h>
#include <string>
#include <string.h>
// #include <malloc.h>
#include <iostream>
#include <cstring>
#include <errno.h>
#include <stdint.h> 
#include <cctype>
#include <algorithm>
#include <stdarg.h>
#include <pthread.h>
#include <sys/syscall.h>
#pragma comment (lib, "pthreadVC2.lib")
#endif

/**
 *  定义Linux下信号处理相关宏
 */
#ifndef _WIN32
// #include <sys/prctl.h>
#include <signal.h>
#define SIGNAL(SignalID, SignalHandler)    signal((SignalID), (SignalHandler))
#define SIG_SEGMENT                        SIGSEGV 
#define SIG_ALARM						   SIGALRM
#define ALARM(Interval)                    alarm((Interval))
#define SHELL_MOVE		                   "mv"
#define SHELL_DELETE                       "rm -f"
#else 
#define SIGNAL(SignalID, SignalHandler)
#define SIG_SEGMENT
#define SIG_ALARM							 
#define ALARM(Interval)  
#define SHELL_MOVE		                  "move"
#define SHELL_DELETE                      "del"
#endif

#if _MSC_VER
#define snprintf _snprintf
#endif

#ifdef _WIN32
typedef int                 socklen_t;
typedef unsigned long       DWORD;
#else
typedef unsigned char       BYTE;
typedef unsigned short      WORD;
typedef int                 SOCKET;
typedef struct sockaddr     SOCKADDR;
typedef struct sockaddr_in  SOCKADDR_IN;

#define     INVALID_SOCKET                      -1
#define     SOCKET_ERROR                        -1
#define     FD_READ  	                        0x01
#define     FD_WRITE 	                        0x02
#define     FD_READWRITE                        0x03
#define     FD_NONE                             0x04
#endif

/////////////////////////////////////////////////////////////////////////////////
#define     FRM_MAX_FILENAME_LENGTH             255					//最大文件长度
#define     FRM_MAX_COMMAND_LENGTH              1024				//最大命令长度

/////////////////////////////////////////////////////////////////////////////////

#define     OPENATC_RTN_TIMEOUT                  -2
#define     UDP_CLIENT                           0
#define     UDP_SERVICE                          1
#define     TCP_CLIENT                           2
#define     TCP_SERVICE                          3

#define     CFG_SERVICE_LISTERN                  0
#define     CAMERA_SERVICE_LISTERN               1

/////////////////////////////////////////////////////////////////////////////////
const unsigned char      LINK_COM                   = 0x01;			//通信规程链路
const unsigned char      LINK_BASEINFO              = 0x02;			//基本信息链路
const unsigned char      LINK_ALTERNATEFEATUREPARA  = 0x03;			//特征参数一般交互链路
const unsigned char      LINK_INTERVENECOMMAND      = 0x04;			//干预指令链路
const unsigned char      LINK_CONTROL               = 0x05;			//和配置软件的命令链路
const unsigned char      LINK_CONFIG                = 0x06;			//和配置软件的参数链路
const unsigned char      LINK_OPTIMIZE_CONTROL      = 0x07;			//和优化模块的命令链路

/////////////////////////////////////////////////////////////////////////////////
const unsigned char	     CTL_ONLINEMACHINE          = 0x01;			//联机
const unsigned char	     CTL_TRAFFICFLOWINFO        = 0x02;			//交通流信息
const unsigned char	     CTL_WORKSTATUS             = 0x03;			//工作状态
const unsigned char      CTL_LAMPCOLORSTATUS        = 0x04;			//灯色状态
const unsigned char	     CTL_CURRENTTIME            = 0x05;			//当前时间
const unsigned char	     CTL_SIGNALLIGHTGROUP       = 0x06;			//信号灯组查询
const unsigned char	     CTL_PHASE                  = 0x07;			//相位
const unsigned char	     CTL_SIGNALMATCHTIME        = 0x08;			//信号配时方案
const unsigned char	     CTL_PROGRAMMESCHEDULEPLAN  = 0x09;			//方案调度计划
const unsigned char	     CTL_WORKWAY                = 0x0A;			//工作方式
const unsigned char		 CTL_FAULT                  = 0x0B;			//故障
const unsigned char	     CTL_VERSION                = 0x0C;			//版本
const unsigned char	     CTL_FEATUREPARAVERSION     = 0x0D;			//特征参数版本
const unsigned char	     CTL_INDENTIFYCODE          = 0x0E;			//识别码
const unsigned char		 CTL_REMOTECONTROL          = 0x0F;			//远程控制
const unsigned char		 CTL_DETECTOR               = 0x10;			//检测器
const unsigned char		 CTL_OVERLAP                = 0x11;			//跟随相位
const unsigned char		 CTL_SCHEDUL_DATE           = 0x12;			//日期
const unsigned char		 CTL_HEART                  = 0xA1;			//心跳    
const unsigned char      CTL_UDISK                  = 0xA5;			//U盘     
const unsigned char      CTL_SYSTEM_REMOTE          = 0xA6;			//系统远程调试  
const unsigned char      CTL_OPERATION_RECORD       = 0xA7;			//操作记录日志  
const unsigned char      CTL_CHANNEL_CHECK          = 0xA8;			//通道可检测
const unsigned char      CTL_VOLUME_LOG				= 0xA9;			//流量日志
const unsigned char		 CTL_PATTERN_INTERRUPT		= 0xAA;         //方案干预
const unsigned char		 CTL_CHANNEL_STATUS_INFO	= 0xAB;			//通道状态信息
const unsigned char      CTL_CHANNEK_LAMP_STATUS    = 0xAC;			//通道灯色状态
const unsigned char      CTL_SYSTEM_CUSTOM			= 0xAD;			//设备参数
const unsigned char      CTL_SYSTEM_UPDATE			= 0xAF;			//FTP文件升级
const unsigned char      CTL_DETECTOR_STATUS        = 0xB0;         //检测器状态
const unsigned char      CTL_UPDATESECRETKEY_STATUS = 0xC0;         //信号机更新密钥，对应的操作类型是0

const unsigned char      CFG_ASK_ASKSEND            = 0x17;			//配置软件请求发送数据
const unsigned char      CFG_ACK_ASKSEND            = 0x18;			//主机回应配置软件请求发送数据
const unsigned char      CFG_ASK_ASKREAD            = 0x19;			//配置软件请求读数据
const unsigned char      CFG_ACK_ASKREAD_OK         = 0x20;			//主机回应配置软件请求读数据成功
const unsigned char      CFG_ACK_ASKREAD_FAILED     = 0x21;			//主机回应配置软件请求读数据失败
const unsigned char      CFG_ASK_SENDDATA           = 0x22;			//配置软件发送数据
const unsigned char      CFG_ACK_SENDDATA_OK        = 0x23;			//配置软件发送数据成功
const unsigned char      CFG_ACK_SENDDATA_FAILED    = 0x24;			//配置软件发送数据失败
const unsigned char      CFG_ACK_SENDDTA_END        = 0x25;			//配置软件发送数据结束

const unsigned char      CTL_ASK_LOGIN              = 0x50;			//配置软件请求登陆
const unsigned char      CTL_ACK_LOGIN              = 0x51;			//主机回应配置软件请求登陆
const unsigned char      CTL_HEART_BERAT			= 0x52;         //心跳包
const unsigned char      CTL_ASK_VERSION            = 0x60;			//配置软件请求版本
const unsigned char      CTL_ACK_VERSION            = 0x61;			//主机回应配置软件请求版本
const unsigned char      CTL_ASK_UPDATEFILE         = 0x51;			//配置软件请求更新文件
const unsigned char      CTL_ACK_UPDATEFILE         = 0x53;			//主机回应配置软件请求更新文件
const unsigned char      CTL_ASK_SENDFILEBLOCK      = 0x54;			//配置软件请求取消文件发送
const unsigned char      CTL_ASK_CANCELSENDFILE     = 0x62;			//主机回应配置软件请求取消文件发送
const unsigned char      CTL_ACKED_SENDFILEEND      = 0x58;			//配置软件请求文件发送完毕确认
const unsigned char      CTL_ASK_STARTUPDATE        = 0x55;			//主机回应配置软件请求文件发送完毕确认
const unsigned char      CTL_ASK_UPDATEONE		    = 0x56;			//配置软件请求文件发送完毕
const unsigned char      CTL_ASK_ADJUSTTIME         = 0x12;			//配置软件请求对时
const unsigned char      CTL_ACK_ASKTIME	        = 0x13;			//主机回应配置软件请求对时
const unsigned char      CTL_ASK_REBOOT             = 0x14;			//配置软件请求重启
const unsigned char      CTL_ASK_TIME	            = 0x18;			//配置软件请求时间

const unsigned char      CTL_TRAFFICFLOW_INFO       = 0x02;			//交通流信息
const unsigned char      CTL_REALDETECTOR_INFO      = 0x51;			//车检器实时检测信息
const unsigned char      CTL_VEHILCEQUEUE_INFO      = 0x52;		    //车辆排队信息
const unsigned char      CTL_PEDDETECTOR_INFO       = 0x53;		    //行人检测信息
const unsigned char      CTL_DETECTORFAULT_INFO     = 0x54;		    //车检器故障状态信息

const unsigned char      CTL_PREEMPT_INFO           = 0x5B;		    //优先信息

const unsigned char      CB_VERSION_FLAG            = 0x10;			//版本号
const unsigned char      CB_ATC_FLAG                = 0x10;			//信号机身份标识
const unsigned char      CB_CONFIGSOFTWARE_FLAG     = 0x20;			//接收方标识
const unsigned char      CB_FLOWCOLLECTDEVICE_FLAG  = 0x80;			//流量采集设备身份标识
const unsigned char      CB_TRAFFICSIMULATE_FLAG    = 0x81;			//交通仿真身份标识
const unsigned char      CB_AIDEVICE_FLAG           = 0x82;			//AI信控身份标识

const unsigned char      CB_CONTENT_CHARACETER      = 0x01;         //字节码
const unsigned char      CB_CONTENT_JSON            = 0x02;         //JSON
const unsigned char      CB_CONTENT_XML             = 0x03;         //XML

////////////////////////////////////////////////////////////////////////////////////
const unsigned char      LINKCODE_POS               = 3;
const unsigned char      CMDCODE_POS                = 8;
const unsigned char      DATALEN_POS                = 10;
const unsigned char      DATACONTENT_POS            = 14;

///////////////////////////////////////////////////////////////////////////////////
const unsigned char      ASK_QUERY                  = 0x80;
const unsigned char      ACK_QUERY                  = 0x83;
const unsigned char      ASK_SET                    = 0x81;
const unsigned char      ACK_SET                    = 0x84;
const unsigned char      REPORT_STATUS              = 0x82;
const unsigned char      ACK_WRONG                  = 0x85;

enum
{
    COM_WITH_CENTER      = 1,
    COM_WITH_SIMULATE    = 2,
	COM_WITH_ITS300      = 3,
};

///////////////////////////////////////////////////////////////////////////////////
//GB20999
typedef enum tagComType_GB20999
{
	COM_TCP = 1,							//TCP
	COM_UDP = 2,							//UDP
	COM_RS232 = 3,							//RS232
}EComType_GB20999;

typedef enum tagFrameType_GB20999
{
	FRAME_TYPE_QUERY = 0x10,				//查询
	FRAME_TYPE_QUERY_REPLY = 0x20,			//查询应答
	FRAME_TYPE_QUERY_ERRORREPLY = 0x21,		//查询出错回复
	FRAME_TYPE_SET = 0x30,					//设置
	FRAME_TYPE_SET_REPLY = 0x40,			//设置应答
	FRAME_TYPE_SET_ERRORREPLY = 0x41,		//设置出错回复
	FRAME_TYPE_BROADCAST = 0x50,			//广播
	FRAME_TYPE_TRAP = 0x60,					//主动上报
	FRAME_TYPE_HEART_QUERY = 0x70,			//心跳查询
	FRAME_TYPE_HEART_REPLY = 0x80,			//心跳应答
}EFrameType_GB20999;

typedef enum tagBadValue_GB20999
{
	BAD_VALUE_STATUS = 0x10,				//值错误 badValue
	BAD_VALUE_WRONGLENGTH = 0x11,			//值长度错误
	BAD_VALUE_OVERFLOW = 0x12,				//值越界
	BAD_VALUE_READONLY = 0x20,				//值只读
	BAD_VALUE_NULL = 0x30,					//值不存在
	BAD_VALUE_ERROR = 0x40,					//值一般错误
	BAD_VALUE_CONTROLFAIL = 0x50,			//控制失败
}EBadValue_GB20999;

typedef enum tagLightType_GB20999
{
	LIGHT_TYPE_VEHICLE = 0x01,				//机动车灯组
	LIGHT_TYPE_NONVEHICLE = 0x02,			//非机动车灯组
	LIGHT_TYPE_PEDESTRIAN = 0x03,			//行人灯组
	LIGHT_TYPE_ROAD = 0x04,					//车道灯组

}ELightType_GB20999;

typedef enum tagLightStatus_GB20999
{
	LIGHT_STATUS_OFF = 0x01,				//灭灯
	LIGHT_STATUS_RED = 0x10,				//红灯
	LIGHT_STATUS_REDFLASH = 0x11,			//红闪
	LIGHT_STATUS_REDFASTFLASH = 0x12,		//红快闪
	LIGHT_STATUS_GREEN = 0x20,				//绿灯
	LIGHT_STATUS_GREENFLASH = 0x21,			//绿闪
	LIGHT_STATUS_GREENFASTFLASH = 0x22,		//绿快闪
	LIGHT_STATUS_YELLOW = 0x30,				//黄灯
	LIGHT_STATUS_YELLOWFLASH = 0x31,		//黄闪
	LIGHT_STATUS_YELLOWFASTFLASH = 0x32,	//黄快闪
	LIGHT_STATUS_REDYELLOW = 0x40,			//红黄灯
}ELightStatus_GB20999;

typedef enum tagDetectorType_GB20999
{
	DETECTOR_TYPE_COIL = 0x01,				//线圈
	DETECTOR_TYPE_VIDEO = 0x02,				//视频
	DETECTOR_TYPE_GEOMAGNETIC = 0x03,		//地磁
	DETECTOR_TYPE_MICROWAVE = 0x04,			//微波检测器
	DETECTOR_TYPE_ULTRASONIC = 0x05,		//超声波检测器
	DETECTOR_TYPE_INFRARED = 0x06,			//红外检测器
}EDetectorType_GB20999;

typedef enum tagPhaseStageType_GB20999
{
	PHASE_STAGE_TYPE_FIX = 0x10,			//相位阶段固定出现
	PHASE_STAGE_TYPE_DEMAND = 0x20,			//相位阶段按需求出现
}EPhaseStageType_GB20999;

typedef enum tagPhaseStageStatus_GB20999
{
	PHASE_STAGE_STATUS_NOTOFWAY = 0x10,		//相位阶段未放行
	PHASE_STAGE_STATUS_ONTHEWAY = 0x20,		//相位阶段正在放行
	PHASE_STAGE_STATUS_TRANSITON = 0x30,	//相位阶段过渡
}EPhaseStageStatus_GB20999;

typedef enum tagControlMode_GB20999
{
	MODE_CENTER_Control = 0x10,				//中心控制模式
	MODE_LOCAL_Control = 0x20,				//本地控制模式
	MODE_SPECIAL_Control = 0x30,			//特殊控制
}EControlMode_GB20999;

typedef enum tagCenterControlMode_GB20999
{
	MODE_CENTER_TIMETABLE_CONTROL = 0x11,   //中心日计划控制
	MODE_CENTER_OPTIMIZATION_CONTROL = 0x12,//中心优化控制
	MODE_CENTER_COORDINATION_CONTROL = 0x13,//中心协调控制
	MODE_CENTER_ADAPTIVE_CONTROL = 0x14,	//中心自适应控制
	MODE_CENTER_MANUAL_CONTROL = 0x15,		//中心手动控制
}ECenterControlMode_GB20999;

typedef enum tagLocalControlMode_GB20999
{
	MODE_LOCAL_FIXCYCLE_CONTROL = 0x21,		//本地定周期控制
	MODE_LOCAL_VA_CONTROL = 0x22,			//本地感应控制
	MODE_LOCAL_COORDINATION_CONTROL = 0x23, //本地协调控制
	MODE_LOCAL_ADAPTIVE_CONTROL = 0x24,		//本地自适应控制
	MODE_LOCAL_MANUAL_CONTROL = 0x25,		//本地手动控制
}ELocalControlMode_GB20999;

typedef enum tagSpecialControlMode_GB20999
{
	MODE_SPECIAL_FLASH_CONTROL = 0x31,		//黄闪控制
	MODE_SPECIAL_ALLRED_CONTROL = 0x32,		//全红控制
	MODE_SPECIAL_ALLOFF_CONTROL = 0x33,		//关灯控制
}ESpecialControlMode_GB20999;

typedef enum tagAlarmType_GB20999
{
	TYPE_ALARM_LIGHT = 0x10,				//信号灯报警
	TYPE_ALARM_DETECTOR = 0x30,				//检测器报警
	TYPE_ALARM_DEVICE = 0x40,				//设备故障报警
	TYPE_ALARM_ENVIRONMENT = 0x60,			//工作环境异常报警
}EAlarmType_GB20999;

typedef enum tagFaultType_GB20999
{
	TYPE_FAULT_GREENCONFLICT = 0x10,		//绿冲突故障
	TYPE_FAULT_GREENREDCONFLICT = 0x11,		//红绿冲突故障
	TYPE_FAULT_REDLIGHT = 0x20,				//红灯故障
	TYPE_FAULT_YELLOWLIGHT = 0x21,			//黄灯故障
	TYPE_FAULT_COMMUNICATION = 0x30,		//通信故障
	TYPE_FAULT_SELF = 0x40,					//自检故障
	TYPE_FAULT_DETECTOR = 0x41,				//检测器故障
	TYPE_FAULT_RELAY = 0x42,				//继电器故障
	TYPE_FAULT_MEMORY = 0x43,				//存储器故障
	TYPE_FAULT_CLOCK = 0x44,				//时钟故障
	TYPE_FAULT_MOTHERBOARD = 0x45,			//主板故障
	TYPE_FAULT_PHASEBOARD = 0x46,			//相位板故障
	TYPE_FAULT_DETECTORBOARD = 0x47,		//检测板故障
	TYPE_FAULT_CONFIG = 0x50,				//配置故障
	TYPE_FAULT_RESPONSE = 0x70,				//控制响应故障
}EFaultType_GB20999;

typedef enum tagSwitchOperation_GB20999
{
	SWITCH_NULL = 0x00,						//无故障动作
	SWITCH_TO_FLASH = 0x10,					//切换到黄闪
	SWITCH_TO_OFF = 0x20,					//切换到灭灯
	SWITCH_TO_RED = 0x30,					//切换到全红
	SWITCH_TO_LOCAL_FIXCYCLE = 0x40,		//切换到本地定周期
	SWITCH_TO_COORDINATION = 0x50,			//切换到本地协调
}ESwitchOperation_GB20999;

typedef enum tagOrderValue_GB20999
{
	ORDER_FLASH = 0x01,						//黄闪
	ORDER_RED = 0x02,						//全红
	ORDER_ON = 0x03,						//开灯
	ORDER_OFF = 0x04,						//关灯
	ORDER_RESET = 0x05,						//重启
	ORDER_CANCEL = 0x00,					//取消命令
}EOrderValue_GB20999;

typedef enum tagDataType_GB20999
{
	DEVICE_INFO = 1,						//设备信息
	BASE_INFO = 2,							//基础信息
	LIGHTGROUP_INFO = 3,					//灯组信息
	PHASE_INFO = 4,							//相位信息
	DETECTOR_INFO = 5,						//检测器信息
	PHASESTAGE_INFO = 6,					//相位阶段信息
	PHASESAFETY_INFO = 7,					//相位安全信息
	EMERGENCY_PRIORITY = 8,					//紧急优先
	PATTERN_INFO = 9,						//方案信息
	TRANSITION_RETRAIN = 10,				//过渡约束
	DAY_PLAN = 11,							//日计划
	SCHEDULE_TABLE = 12,					//调度表
	RUN_STATUS = 13,						//运行状态
	TRAFFIC_DATA = 14,						//交通数据
	ALARM_DATA = 15,						//报警数据
	FAULT_DATA = 16,						//故障数据
	CENTER_CONTROL = 17,					//中心控制
	ORDER_PIPE = 18,						//命令管道
	PRIVATE_DATE = 128						//私有数据类
}EDataType_GB20999;

typedef enum tagDeviceInfo_GB20999
{
	MANUFACTURER = 1,						//制造厂商
	DEVICE_VERSION = 2,						//设备版本
	DEVICE_ID = 3,							//设备编号
	MANUFACTUR_DATE = 4,					//出厂日期
	CONFIG_DATE = 5,						//配置日期
}EDeviceInfo_GB20999;

typedef enum tagBaseInfo_GB20999
{
	INSTALLATION_ROAD = 1,					//安装路口
	ATC_IPV4_NETCONFIG = 2,					//信号机IPV4网络配置
	HOST_IPV4_NETCONFIG = 3,				//上位机IPV4网络配置
	ATC_TIMEZONE = 4,						//信号机所属时区
	ATC_ID = 5,								//信号机ID
	ATC_ROADCOUNT = 6,						//信号机控制路口数量
	GPS_CLOCK_FLAG = 7,						//GPS时钟标志
	ATC_IPV6_NETCONFIG = 8,					//信号机IPV6网络配置
	HOST_IPV6_NETCONFIG = 9,				//上位机IPV6网络配置
}EBaseInfo_GB20999;

typedef enum tagATCIPV4NetConfig_GB20999
{
	ATC_IP_ADDRESS = 1,						//IP地址
	SUB_NET = 2,							//子网掩码
	GATE_WAY = 3,							//网关
}EATCIPV4NetConfig_GB20999;

typedef enum tagHostIPV4NetConfig_GB20999
{
	HOST_IP_ADDRESS = 1,					//IP地址
	COM_PORT = 2,							//通信端口
	COM_TYPE = 3,							//通信类型
}EHostIPV4NetConfig_GB20999;

typedef enum tagLightGroupInfo_GB20999
{
	LIGHT_GROUP_COUNT = 1,					//实际灯组数
	LIGHT_GROUP_CONFIG_TABLE = 2,			//灯组配置表
	LIGHT_GROUP_STATUS_TABLE = 3,			//灯组状态表
	LIGHT_GROUP_CONTROL_TABLE = 4,			//灯组控制表
}ELightGroupInfo_GB20999;

typedef enum tagLightGroupConfigTable_GB20999
{
	LIGHT_GROUP_TYPE_ID = 1,				//灯组编号
	LIGHT_GROUP_TYPE = 2,					//灯组类型
}ELightGroupConfigTable_GB20999;

typedef enum tagLightGroupStatusTable_GB20999
{
	LIGHT_GROUP_STATUS_ID = 1,				//灯组编号
	LIGHT_GROUP_STATUS = 2,					//灯组状态
}ELightGroupStatusTable_GB20999;

typedef enum tagControlTable_GB20999
{
	CONTROL_ID = 1,							//编号
	CONTROL_SHIELD = 2,						//屏蔽
	CONTROL_PROHIBIT = 3,					//禁止
}EControlTable_GB20999;

typedef enum tagLightGroupType_GB20999
{
	LIGHT_GROUP_VEHICLE = 1,				//机动车
	LIGHT_GROUP_NOVEHICLE = 2,				//非机动车
	LIGHT_GROUP_PED = 3,					//行人
	LIGHT_GROUP_ROAD = 4,					//车道
	LIGHT_GROUP_ALTERABLE_TRAFFIC = 5,		//可变交通标志
	LIGHT_GROUP_BUS = 6,					//公交专用灯具
	LIGHT_GROUP_TRAM = 7,					//有轨电车专用灯具
	LIGHT_GROUP_SPECIALBUS = 8,				//特殊公交
}ELightGroupType_GB20999;

typedef enum tagPhaseInfo_GB20999
{
	PHASE_COUNT = 1,						//实际相位数
	PHASE_CONFIG_TABLE = 2,					//相位配置表
	PHASE_CONTROL_TABLE = 3,				//相位控制表
}EPhaseInfo_GB20999;

typedef enum tagPhaseConfigTable_GB20999
{
	PHASE_CONFIG_ID = 1,					//相位编号
	PHASE_LIGHTGROUP = 2,					//相位的灯组
	PHASE_LOSETRANSITIONTYPE1 = 3,			//失去路权过渡灯色类型1
	PHASE_LOSETRANSITIONTIME1 = 4,			//失去路权过渡灯色时间1
	PHASE_LOSETRANSITIONTYPE2 = 5,			//失去路权过渡灯色类型2
	PHASE_LOSETRANSITIONTIME2 = 6,			//失去路权过渡灯色时间2
	PHASE_LOSETRANSITIONTYPE3 = 7,			//失去路权过渡灯色类型3
	PHASE_LOSETRANSITIONTIME3 = 8,			//失去路权过渡灯色时间3
	PHASE_GETTRANSITIONTYPE1 = 9,			//获得路权过渡灯色类型1
	PHASE_GETTRANSITIONTIME1 = 10,			//获取路权过渡灯色时间1
	PHASE_GETTRANSITIONTYPE2 = 11,			//获得路权过渡灯色类型2
	PHASE_GETTRANSITIONTIME2 = 12,			//获取路权过渡灯色时间2
	PHASE_GETTRANSITIONTYPE3 = 13,			//获得路权过渡灯色类型3
	PHASE_GETTRANSITIONTIME3 = 14,			//获取路权过渡灯色时间3
	PHASE_TURNONGETTRANSITIONTYPE1 = 15,	//开机获得路权过渡灯色类型1
	PHASE_TURNONGETTRANSITIONTIME1 = 16,	//开机获得路权过渡灯色时间1
	PHASE_TURNONGETTRANSITIONTYPE2 = 17,	//开机获得路权过渡灯色类型2
	PHASE_TURNONGETTRANSITIONTIME2 = 18,	//开机获得路权过渡灯色时间2
	PHASE_TURNONGETTRANSITIONTYPE3 = 19,	//开机获得路权过渡灯色类型3
	PHASE_TURNONGETTRANSITIONTIME3 = 20,	//开机获得路权过渡灯色时间3
	PHASE_TURNONLOSETRANSITIONTYPE1 = 21,	//开机失去路权过渡灯色类型1
	PHASE_TURNONLOSETRANSITIONTIME1 = 22,	//开机失去路权过渡灯色时间1
	PHASE_TURNONLOSETRANSITIONTYPE2 = 23,	//开机失去路权过渡灯色类型1
	PHASE_TURNONLOSETRANSITIONTIME2 = 24,	//开机失去路权过渡灯色时间2
	PHASE_TURNONLOSETRANSITIONTYPE3 = 25,	//开机失去路权过渡灯色类型3
	PHASE_TURNONLOSETRANSITIONTIME3 = 26,	//开机失去路权过渡灯色时间3
	PHASE_MIN_GREENTIME = 27,				//最小绿时间
	PHASE_MAX1_GREENTIME = 28,				//最大绿时间1
	PHASE_MAX2_GREENTIME = 29,				//最大绿时间2
	PHASE_PASSAGE_GREENTIME = 30,			//延长绿时间
	PHASE_CALL = 31,						//相位的需求
}EPhaseConfigTable_GB20999;

typedef enum tagPhaseControlTable_GB20999
{
	PHASE_CONTROL_ID = 1,					//相位ID
	PHASE_CONTROL_SHIELD = 2,				//相位屏蔽
	PHASE_CONTROL_PROHIBIT = 3,				//相位禁止
}EPhaseControlTable_GB20999;

typedef enum tagDetectorInfo_GB20999
{
	DETECTOR_COUNT = 1,						//实际检测器数
	DETECTOR_CONFIG_TABLE = 2,				//检测器配置表
	DETECTOR_DATA_TABLE = 3,				//检测器数据表
}EDetectorInfo_GB20999;

typedef enum tagDetectorConfigTable_GB20999
{
	DETECTOR_ID_CONFIG = 1,					//检测器编号
	DETECTOR_TYPE = 2,						//检测器类型
	FLOW_CYCLE = 3,							//流量采集周期
	OCCUPY_CYCLE = 4,						//占有率采集周期
	INSTALL_POS = 5,						//安装位置
}EDetectorConfigTable_GB20999;

typedef enum tagDetectorDataTable_GB20999
{
	DETECTOR_ID_DATA = 1,					//检测器编号
	EXIST_STATUS = 2,						//检测器车辆存在状态
	VEHICLE_SPEED = 3,						//检测器车辆速度
	VEHICLE_TYPE = 4,						//检测到的车辆类型
	VEHICLE_PLATE = 5,						//检测到的车辆号牌
	QUEUE_LENGTH = 6,						//所在车道排队长度
}EDetectorDataTable_GB20999;

typedef enum tagVehiclType_GB20999
{
	VEHICLE_TYPE_UNDEF = 0,					//无车
	VEHICLE_TYPE_SMALL = 1,					//小型车
	VEHICLE_TYPE_MIDDLE = 2,				//中型车
	VEHICLE_TYPE_LARGE = 3,					//大型车
	VEHICLE_TYPE_BUS = 4,					//公交车
	VEHICLE_TYPE_TRAMCAR = 5,				//有轨电车
	VEHICLE_TYPE_SPEICAL = 6,				//特种车辆
}EVehiclType_GB20999;

typedef enum tagPhaseStageInfo_GB20999
{
	PHASE_STAGE_COUNT = 1,					//实际相位阶段数
	PHASE_STAGE_CONFIG_TABLE = 2,			//相位阶段配置表
	PHASE_STAGE_STATUS_TABLE = 3,			//相位阶段状态表
	PHASE_STAGE_CONTROL_TABLE = 4,			//相位阶段控制表
}EPhaseStageInfo_GB20999;

typedef enum tagPhaseStageConfigTable_GB20999
{
	PHASE_STAGE_ID_CONFIG = 1,				//编号
	PHASE_STAGE_PHASE = 2,					//相位
	PHASE_STAGE_LATE_START = 3,				//晚启动时间
	PHASE_STAGE_EARLY_END = 4,				//早结束时间
}EPhaseStageConfigTable_GB20999;

typedef enum tagPhaseStageStatusTable_GB20999
{
	PHASE_STAGE_ID_STATUS = 1,				//编号
	PHASE_STAGE_STATUS = 2,					//状态
	PHASE_STAGE_RUN_TIME = 3,				//运行时间
	PHASE_STAGE_REMAIN_TIME = 4,			//剩余时间
}EPhaseStageStatusTable_GB20999;

typedef enum tagPhaseStageControlTable_GB21999
{
	PHASE_STAGE_ID_CONTROL = 1,				//编号
	PHASE_STAGE_SOFTWARECALL = 2,			//软件需求
	PHASE_STAGE_SHIELD = 3,					//相位阶段屏蔽
	PHASE_STAGE_PROHIBIT = 4,				//相位阶段禁止
}EPhaseStageControlTable_GB21999;

typedef enum tagPhaseSafetyInfo_GB20999
{
	PHASE_CONFLICT_TABLE = 1,				//相位冲突配置表
	PHASE_GREENINTERVAL_TABLE = 2,			//相位绿间隔配置表
}EPhaseSafetyInfo_GB20999;

typedef enum tagPhaseConflictTable_GB21999
{
	PHASE_ID_CONFLICT = 1,					//编号
	PHASE_CONFLICT_ARRAY = 2,				//冲突相位序列
}EPhaseConflictTable_GB21999;

typedef enum tagPhaseGreenIntervalTable_GB21999
{
	PHASE_ID_GREENINTERVAL = 1,				//编号
	PHASE_GREENINTERVAL_ARRAY = 2,			//相位绿间隔序列
}EPhaseGreenIntervalTable_GB21999;

typedef enum tagEmergencyPriority_GB20999
{
	PRIORITY_COUNT = 1,						//实际优先数量
	PRIORITY_CONFIG_TABLE = 2,				//优先配置表
	PRIORITY_STATUS_TABLE = 3,				//优先状态表
	EMERGENCY_COUNT = 4,					//紧急数量
	EMERGENCY_CONFIG_TABLE = 5,				//紧急配置表
	EMERGENCY_STATUS_TABLE = 6,				//紧急状态表
}EEmergencyPriority_GB20999;

typedef enum tagPriorityConfig_GB20999
{
	PRIORITY_ID_CONFIG = 1,					//优先信号编号
	PRIORITY_APPLY_PHASESTAGE = 2,			//优先信号申请相位阶段
	PRIORITY_APPLY_GRADE = 3,				//优先信号申请优先级
	PRIORITY_SHIELD = 4,					//优先信号屏蔽标志
	PRIORITY_SOURCE = 5,					//优先信号来源(检测器编号)
}EPriorityConfig_GB20999;

typedef enum tagPriorityStatus_GB20999
{
	PRIORITY_ID_STATUS = 1,					//优先信号编号
	PRIORITY_APPLY_STATUS = 2,				//优先信号申请状态
	PRIORITY_EXECUTE_STATUS = 3,			//优先信号执行状态
}EPriorityStatus_GB20999;

typedef enum tagEmergencyConfig_GB20999
{
	EMERGENCY_ID_CONFIG = 1,				//紧急信号编号
	EMERGENCY_APPLY_PHASESTAGE = 2,			//紧急信号申请相位阶段
	EMERGENCY_APPLY_GRADE = 3,				//紧急信号申请优先级
	EMERGENCY_SHIELD = 4,					//紧急信号屏蔽标志
	EMERGENCY_SOURCE = 5,					//紧急信号来源(检测器编号)
}EEmergencyConfig_GB20999;

typedef enum tagEmergencyStatus_GB20999
{
	EMERGENCY_ID_STATUS = 1,				//紧急信号编号
	EMERGENCY_APPLY_STATUS = 2,				//紧急信号申请状态
	EMERGENCY_EXECUTE_STATUS = 3,			//紧急信号执行状态
}EEmergencyStatus_GB20999;

typedef enum tagPatternInfo_GB20999
{
	PATTERN_COUNT = 1,						//方案数量
	PATTERN_CONFIGTABLE = 2,				//方案配置表
}EPatternInfo_GB20999;

typedef enum tagPatternConfigTable_GB20999
{
	PATTERN_ID = 1,							//方案编号
	PATTERN_ROADID = 2,						//路口序号
	PATTERN_CYCLELEN = 3,					//方案周期
	PATTERN_ADJUST_STAGEID = 4,				//协调序号
	PATTERN_OFFSET = 5,						//相位差时间
	PATTERN_STAGE_CHAIN = 6,				//阶段链
	PATTERN_STAGE_CHAINTIME = 7,			//阶段链时间
	PATTERN_STAGE_TYPE = 8,					//阶段类型
}EPatternConfigTable_GB20999;

typedef enum tagTransitionRetain_GB20999
{
	PHASE_TRANSITIONSTAGE_ID = 1,			//相位阶段编号               
	PHASE_TRANSITIONSTAGE_RETAIN = 2,		//相位阶段过渡约束值   
}ETransitionRetain_GB20999;

typedef enum tagDayPlanInfo_GB20999
{
	DAYPLAN_COUNT = 1,						//实际日计划数量               
	DAYPLAN_CONFIG = 2,						//日计划配置   
}EDayPlanInfo_GB20999;

typedef enum tagDayPlanConfigTable_GB20999
{
	DAYPLAN_ID = 1,							//日计划编号               
	DAYPLAN_ROADID = 2,						//路口序号   
	DAYPLAN_STARTTIMECHAIN = 3,				//开始时间链   
	DAYPLAN_PATTERNCHAIN = 4,				//执行方案链   
	DAYPLAN_RUNMODECHAIN = 5,				//运行模式链   
	DAYPLAN_TIMESPANACTCHAIN1 = 6,			//时段动作链1   
	DAYPLAN_TIMESPANACTCHAIN2 = 7,			//时段动作链2  
	DAYPLAN_TIMESPANACTCHAIN3 = 8,			//时段动作链3  
	DAYPLAN_TIMESPANACTCHAIN4 = 9,			//时段动作链4  
	DAYPLAN_TIMESPANACTCHAIN5 = 10,			//时段动作链5  
	DAYPLAN_TIMESPANACTCHAIN6 = 11,			//时段动作链6  
	DAYPLAN_TIMESPANACTCHAIN7 = 12,			//时段动作链7  
	DAYPLAN_TIMESPANACTCHAIN8 = 13,			//时段动作链8  
}EDayPlanConfigTable_GB20999;

typedef enum tagScheduleTable_GB20999
{
	SCHEDULE_COUNT = 1,						//调度表数量              
	SCHEDULE_CONFIG_TABLE = 2,				//调度表配置表   
}EDayScheduleTable_GB20999;

typedef enum tagScheduleConfigTable_GB20999
{
	SCHEDULE_ID = 1,						//调度表编号               
	SCHEDULE_ROADID = 2,					//路口序号   
	SCHEDULE_PRIORITY = 3,					//优先级   
	SCHEDULE_WEEK = 4,						//星期值   
	SCHEDULE_MONTH = 5,						//月份值   
	SCHEDULE_DAY = 6,						//日期值   
	SCHEDULE_DAYPLAN_ID = 7,				//日计划编号
}EDayScheduleeConfigTable_GB20999;

typedef enum tagRunStatus_GB20999
{
	DEVICE_STATUS = 1,						//设备状态               
	CONTROL_STATUS = 2,						//控制状态   
}ERunStatus_GB20999;

typedef enum tagDeviceStatus_GB20999
{
	DETECTOR_STATUS = 1,					//检测器状态               
	DEVICEMODULE_STATUS = 2,				//设备模块状态   
	ATC_DOOR_STATUS = 3,					//柜门状态
	VOLTAGE_VALUE = 4,						//电压值
	CURRENT_VALUE = 5,						//电流值
	TEMPERATURE_VALUE = 6,					//温度值
	HUMIDITY_VALUE = 7,						//湿度值
	WATERLOGIN_VALUE = 8,					//水浸值
	SMOKE_VALUE = 9,						//烟雾值
	STANDARD_TIME = 10,						//标准时间
	LOCAL_TIME = 11,						//本地时间
}EDeviceStatus_GB20999;

typedef enum tagControlStatus_GB20999
{
	CONTROL_ROADID = 1,						//路口序号               
	ROAD_RUNMODE = 2,						//路口运行模式   
	ROAD_PATTERN = 3,						//路口当前方案   
	ROAD_STAGE = 4,							//路口当前阶段   
}EControlStatus_GB20999;

typedef enum tagTrafficData_GB20999
{
	REALTIME_DATA = 1,						//实时数据               
	STATISTIC_DATA = 2,						//统计数据表   
}ETrafficData_GB20999;

typedef enum tagStatisticData_GB20999
{
	DETECTOR_ID_STATISTIC = 1,				//检测器编号               
	DETECTOR_FLOW = 2,						//检测器流量 
	DETECTOR_OCCUPY = 3,					//检测器占有率 
	AVERAGE_SPEED = 4,						//平均车速
}EStatisticData_GB20999;

typedef enum tagAlarmData_GB20999
{
	ALARM_COUNT = 1,						//报警数量               
	ALARMDATA_TABLE = 2,					//报警数据表   
}EAlarmData_GB20999;

typedef enum tagAlarmDataTable_GB20999
{
	ALARM_ID = 1,							//报警编号            
	ALARM_TYPE = 2,							//报警类型
	ALARM_VALUE = 3,						//报警值
	ALARM_TIME = 4,							//报警时间
}EAlarmDataTable_GB20999;

typedef enum tagFaultData_GB20999
{
	FAULT_COUNT = 1,						//故障数量               
	FAULT_RECORD = 2,						//故障记录表   
}EFaultData_GB20999;

typedef enum tagFaultRecordTable_GB20999
{
	FAULT_ID = 1,							//故障记录编号            
	FAULT_TYPE = 2,							//故障类型
	FAULT_TIME = 3,							//故障时间
	FAULT_PARAM = 4,						//故障动作参数
}EFaultRecordDataTable_GB20999;

typedef enum tagCenterControlTable_GB20999
{
	CENTERCONTROL_ROADID = 1,				//路口序号               
	CENTERCONTROL_PHASESTAGE = 2,			//指定相位阶段   
	CENTERCONTROL_PATTERN = 3,				//指定方案  
	CENTERCONTROL_RUNMODE = 4,				//指定运行模式
}ECenterControlTable_GB20999;