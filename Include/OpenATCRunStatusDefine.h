#pragma once

#include "../Include/OpenATCParamConstDefine.h"
#include "../Include/OpenATCParamStructDefine.h"

const int C_N_MAXLAMPOUTPUT_NUM		= 240;
const int C_N_MAXRING_NUM			= 4;
const int C_N_MAXLAMPBOARD_NUM		= 20;
const int C_N_MAXIOBOARD_NUM		= 4;
const int C_N_MAXCANBUS_NUM			= 2;
const int C_N_MAXDETBOARD_NUM		= 4;
const int C_N_MAXDETINPUT_NUM		= 16;
const int C_N_MAXIOINPUT_NUM		= 16;
const int C_N_LAMPBORAD_OUTPUTNUM	= 12;
const int C_N_CHANNEL_OUTPUTNUM		= 3;
const int C_N_CHANNELNUM_PER_BOARD	= 4;
const int C_N_MAX_FUNC_CARDSLOT_NUM = 15;
const int C_N_MAX_CTRL_CARDSLOT_NUM = 6;
const int C_N_MAX_SCREEN_RING_NUM	= 4;

const char C_CH_PHASESTAGE_RY	= 'a';									//红黄同亮
const char C_CH_PHASESTAGE_G	= 'g';									//绿
const char C_CH_PHASESTAGE_GF	= 'h';									//绿闪
const char C_CH_PHASESTAGE_Y	= 'y';									//黄
const char C_CH_PHASESTAGE_YF	= 'z';									//黄闪
const char C_CH_PHASESTAGE_R	= 'r';									//红
const char C_CH_PHASESTAGE_RF	= 's';									//红闪
const char C_CH_PHASESTAGE_U	= 'u';									//未定义
const char C_CH_PHASESTAGE_F	= 'f';									//可结束
const char C_CH_PHASESTAGE_OF	= 'c';									//关灯

typedef enum tagLampClrOutput
{
    LAMP_CLR_OFF	= 0,												//关灯
    LAMP_CLR_FLASH	= 1,												//闪灯
    LAMP_CLR_ON		= 2,                                                //亮灯
    LAMP_CLR_UNDEF	= 3,												//未定义
}ELampClrOutput;

typedef enum tagFaultLevel
{
    NO_FAULT		= 0,												//无故障
    COMMON_FAULT	= 1,												//普通故障
    CRITICAL_FAULT	= 2,												//严重故障
}EFaultLevel;

typedef enum tagLogicCtlStageVal
{
    CTL_STAGE_UNDEFINE		= 0,                                        //控制阶段未定义
    CTL_STAGE_STARTUP_FLASH = 1,										//启动时序黄闪
    CTL_STAGE_STARTUP_RED	= 2,										//启动时序全红
    CTL_STAGE_SELFCTL		= 3,                                        //自主控制
    CTL_STAGE_FAULTFORCE	= 4,										//严重故障黄闪
}ELogicCtlStageVal;

typedef enum tagCtlCmdSource
{
    CTL_SOURCE_SELF					= 0,                                 //自主控制
    CTL_SOURCE_LOCAL				= 1,                                 //本地手动
    CTL_SOURCE_SYSTEM				= 2,                                 //系统控制
    CTL_SOURCE_TZPARAM				= 3,                                 //配置软件控制
    CTL_SOURCE_RCONTROL				= 4,                                 //遥控器控制
    CTL_SOURCE_YELLOWFLASHTRIGGER	= 5,								 //黄闪器触发
	CTL_SOURCE_DEGRADE              = 6,                                 //降级控制
	CTL_SOURCE_PREEMPT				= 7,                                 //优先控制
}ECtlCmdSource;

typedef enum tagManualSubMode
{
	CTL_MANUAL_SUBMODE_DEFAULT	                = 0,                     //默认值
	CTL_MANUAL_SUBMODE_PANEL_FITSTMANUAL	    = 1,                     //面板第一次按下手动
    CTL_MANUAL_SUBMODE_PANEL_STEPFOWARD	        = 2,                     //面板步进
    CTL_MANUAL_SUBMODE_PANEL_DIRECTION          = 3,                     //面板方向
	CTL_MANUAL_SUBMODE_SYSTEM_STEPFOWARD        = 4,                     //系统步进
    CTL_MANUAL_SUBMODE_SYSTEM_INTERRUPT_PATTERN = 5,                     //系统干预方案
	CTL_MANUAL_SUBMODE_SYSTEM_CHANNEL_LOCK      = 6,                     //系统通道锁定
}EManualSubMode;

typedef enum tagChannelStatus
{
	CHANNEL_STATUS_DEFAULT	= 0,										//默认
    CHANNEL_STATUS_RED		= 1,										//红灯
    CHANNEL_STATUS_YELLOW	= 2,										//黄灯
    CHANNEL_STATUS_GREEN	= 3,										//绿灯
    CHANNEL_STATUS_OFF		= 4,										//灭灯
}EChannelStatus;

typedef enum tagLockChannelStatus
{
	LOCKCHANNEL_STATUS_DEFAULT	  = 0,                                     //默认
    LOCKCHANNEL_STATUS_RED		  = 1,                                     //红灯
    LOCKCHANNEL_STATUS_YELLOW	  = 2,                                     //黄灯
    LOCKCHANNEL_STATUS_GREEN	  = 3,                                     //绿灯
    LOCKCHANNEL_STATUS_GREENFLASH = 4,                                     //绿闪
	LOCKCHANNEL_STATUS_OFF  	  = 5,                                     //灭灯
    LOCKCHANNEL_STATUS_REDFLASH   = 6,                                     //红闪
}ELOCKChannelStatus;

typedef enum tagChannelType
{
	CHANNEL_TYPE_DIRECTION	      = 0,                                  //方向通道
    CHANNEL_TYPE_LOCK		      = 1,                                  //锁定通道
}EChannelType;

typedef enum tagPhaseLockType
{
	LOCK_TYPE_CANCEL	          = 0,                                  //取消锁定
    LOCK_TYPE_ALL		          = 1,                                  //机动车和行人都锁定
	LOCK_TYPE_VEH		          = 2,                                  //仅锁定机动车相位(包括跟随相位)
	LOCK_TYPE_PED		          = 3,                                  //仅锁定行人(包括跟随相位)
}EPhaseLockType;

typedef enum tagBoardType
{
	BOARD_MAINCTL   = 1,												//主控板
	BOARD_LAMP		= 2,												//灯控板
	BOARD_VEHDET	= 3,												//车检板
	BOARD_IO		= 4,												//IO板
	BOARD_FAULT     = 5,												//故障板
	TZPARAM_FAULT   = 6,												//特征参数
}EBoardType;

enum
{
	WORK_NORMAL   = 0,													//0正常
	WORK_ABNORMAL = 1,													//1异常
};

enum
{
	STEP_STAGE   = 0,													//阶段步进
	STEP_COLOR   = 1,													//色步步进
};

enum
{
	PREEMPT_TYPE_NORMAL  = 0,                                           //常规优先
	PREEMPT_TYPE_URGENT  = 1,                                           //紧急优先
	PREEMPT_TYPE_DEFAULT = 2,                                           //默认
};


typedef enum tagInfoType
{
	LOCAL_MANUALPANEL       = 0,          //本地面板控制
	SYSTEM_MANUALCONTROL    = 1,          //系统手动控制
	SYSTEM_DOWNLOAD_TZPARAM = 2,          //系统下载特征参数
	SYSTEM_DOWNLOAD_HWPARAM = 3,          //系统下载设备参数
	SYSTEM_UPLOADPARAM      = 4,          //系统上载参数
	SYETEM_REBOOT           = 5,          //系统重启信号机
	SYETEM_DOWNLOADPATTERN  = 6,          //系统下载方案
	SYETEM_UPLOADPATTERN    = 7,          //系统上载方案
	SYETEM_DOWNLOADPLAN     = 8,          //系统下载调度计划
	SYETEM_UPLOADPLAN       = 9,          //系统上载调度计划
	SYETEM_DOWNLOADDATE     = 10,         //系统下载日期
	SYETEM_UPLOADDATE       = 11,         //系统上载日期
	SYETEM_CHANNELCHECK     = 12,         //系统通道检测
	SYETEM_PATTERNINTERRUPT = 13,         //系统方案干预
	SYETEM_SETTIME          = 14,         //系统设置时间
}EInfoType;

typedef enum tagSelDetectStatus
{
    SELF_DETECT_INING     = 0,			//自检中
    SELF_DETECT_SUCCESS   = 1,			//自检成功
    SELF_DETECT_FAILED    = 2,			//自检失败
}ESelDetectStatus;

typedef enum tagSelDetectDetail
{
	NO_WRONG_INFO					= 0,		//无错误信息
	USB_MOUNT_FAILED				= 1,		//U盘Mount失败
    NO_FILE							= 2,		//参数文件不存在
    MD5_CHECK_FAILED				= 3,		//MD5校验失败
    SITEID_CHECK_FAILED				= 4,		//地址码不匹配
    FAULT_BOARD_NOTONLINE			= 5,		//故障板没有上线
	NOT_CONFIG_MASTER_COUNT			= 6,		//本机板卡数量没有配置
	LAMP_BOARD_NOTONLINE			= 7,		//灯控板没有上线
	LAMP_BOARD_ID_WRONG				= 8,		//灯控板ID错
	VETDET_BOARD_ID_WRONG			= 9,		//车检板ID错
	IO_BOARD_ID_WRONG				= 10,		//IO板ID错
	WONG_SLOT						= 11,		//灯控板插头编码错误
	WONG_PLUG						= 12,		//灯控板插槽编码错误
	RELAY_NOT_WORK					= 13,		//继电器没有吸合
	USB_NOT_FIND					= 14,		//未发现U盘
	OPEN_PARAM_FILE_FAILED			= 15,		//打开参数文件失败
	PARAM_JSON_FORMAT_ERR			= 16,		//配置文件json格式错误
	NO_DEVICE_PARAM_FILE			= 17,		//设备参数文件不存在
	OPEN_DEVICE_PARAM_FILE_FAILED	= 18,		//打开设备参数文件失败
	OPEN_DEVICE_PARAM_CHECK_FAILED	= 19,		//设备参数(除siteid外的信息)校验失败
}ESelDetectDetail;

typedef enum tagFaultValue
{
	FaultType_MainBoard                    = 101,     //Can总线通信故障
	FaultType_YellowFlash                  = 102,     //黄闪器故障
	FaultType_Relay_Not_Work			   = 103,     //继电器没有吸合

	FaultType_LampBoardID                  = 201,     //灯控板ID故障
	FaultType_LampBoardNum                 = 202,     //灯控板脱机
	FaultType_NoRedOn                      = 203,     //无红灯亮起
	FaultType_GreenAndRedOn                = 204,     //红绿同亮
	FaultType_GreenConflict                = 205,     //绿冲突

	FaultType_Red_Lamp_Volt_Fault			= 206,	  //红灯灯电压故障 
	FaultType_Yellow_Lamp_Volt_Fault		= 207,	  //黄灯灯电压故障
	FaultType_Green_Lamp_Volt_Fault			= 208,	  //绿灯灯电压故障

	FaultType_Red_Lamp_Power_Fault			= 209,	  //红灯灯功率故障
	FaultType_Yellow_Lamp_Power_Fault		= 210,	  //黄灯灯功率故障
	FaultType_Green_Lamp_Power_Fault		= 211,	  //绿灯灯功率故障

	FaultType_Lamp_Fault                    = 212,    //灯组故障
	FaultType_Detector_Fault                = 213,    //检测器故障
	FaultType_Wong_Slot						= 214,	  //灯控板插槽编码错误	
	FaultType_Wong_Plug						= 215,	  //灯控板插头编码错误	
	FaultType_Config_Master_Count			= 216,	  //本机灯控板数量没有配置
	

	FaultType_VetDetID						= 301,	  //车检板未初始化（ID错误）
	FaultType_VetDetNum						= 302,	  //车检板脱机
	FaultType_VetDetector_Short_Circuit		= 303,	  //检测器短路
	FaultType_VetDetector_Open_Circuit		= 304,	  //检测器断路

	FaultType_IOBoardID				    	= 401,	  //IO板未初始化（ID错误）
	FaultType_IOBoardNum					= 402,	  //IO板脱机

	FaultType_FaultDetBoard                 = 501,    //故障板脱机
	
	FaultType_TZParam                       = 601,     //特征参数故障
	FaultType_HWParam                       = 602,     //设备参数异常（不含siteid）
	
}EFaultValue;

//特征参数103 的故障子类型
typedef enum tagSubFaultValueTZParam
{
	FaultSubType_TZParam_NO_Exist			= 1,     //特征参数文件不存在
	FaultSubType_TZParam_File_NO_Read		= 2,     //特征参数文件不可读
	FaultSubType_TZParam_File_Changes		= 3,     //特征参数人为修改（MD5校验失败）
	FaultSubType_TZParam_File_Open_Fail		= 4,     //特征参数文件打开失败
	FaultSubType_TZParam_Update_Fail		= 5,     //特征参数文件更新失败
	FaultSubType_TZParam_SiteID_Failt		= 6,     //信号机地址码校验失败
	FaultSubType_TZParam_Format_Error		= 7,     //特征参数内容格式错误
	FaultSubType_TZParam_USB_Mount_Fail		= 8,	 //U盘挂载失败
	FaultSubType_TZParam_USB_Not_Find		= 9,	 //U盘没找到
	FaultSubType_HWParam_Check_Fault		= 10,	 //设备参数异常（不含siteid）
}ESubFaultValue;

//灯电压故障的故障子类型（包括红、黄、绿灯）
typedef enum tagSubFaultLampVolt
{
	FaultSubType_Lamp_Volt_Output_Fail		= 1,     //未输出有效电压
	FaultSubType_Lamp_Volt_Output_Low		= 2,     //输出电压低于输入电压过多
	FaultSubType_Lamp_Volt_Output_High		= 3,     //输出电压高于输入电压
	FaultSubType_Lamp_Volt_Off_Output_High	= 4,     //关闭输出但实际电压仍然输出
	FaultSubType_Lamp_Volt_Off_Output_Low	= 5,     //关闭输出但实际电压部分输出
	FaultSubType_Lamp_Volt_Residual_High	= 6,     //线路残留电压过高

}ESubFaultLampVolt;

//灯功率故障的故障子类型（包括红、黄、绿灯）
typedef enum tagSubFaultLampPower
{
	FaultSubType_Lamp_Power_Output_Up		= 1,     //功率异常增加
	FaultSubType_Lamp_Power_Output_Down		= 2,     //功率异常减少
	FaultSubType_Lamp_Power_Output_Zero		= 3,     //功率无输出
	FaultSubType_Lamp_Power_Output_High		= 4,     //关闭状态仍有功率输出

}ESubFaultLampPower;

//灯组故障的子类型
typedef enum tagSubLampGroupFault
{
	FaultSubType_Lamp_Group_Red		    = 1,		 //红灯故障
	FaultSubType_Lamp_Group_Yellow		= 2,		 //黄灯故障
	FaultSubType_Lamp_Group_Green		= 3,         //绿灯故障

}ESubLampGroupFault;

typedef enum tagSystemControlResultCode
{
    CONTROL_SUCCEED                     = 0,         //正确
    CONTROL_FAILED                      = 1,         //失败
}ESystemControlSuccessCode;

typedef enum tagSystemControlFailCode
{
    NO_SUPPORT_CONTROL_WAY                   = 1,     //不支持的控制方式
    NO_EXIST_PATTERN_NO                      = 2,     //不存在的方案编号
    NO_SUPPORT_CONTROL_PARAM                 = 3,     //不支持的控制参数
	DEVICE_INIT_NO_EXECUT                    = 4,     //设备初始化中，无法执行
	HIGH_PRIORITY_PATTERN_CONTROL_NO_EXECUT  = 5,     //优先级更高的方案控制中，无法执行
	HIGH_PRIORITY_USER_CONTROL_NO_EXECUT     = 6,     //优先级更高的用户控制中，无法执行
	NULL_PATTERNNUM							 = 7,	  //方案编号对应的方案不存在
	CONFIG_INCLUDE_GREENCONFLICT             = 8,     //配置包含绿冲突
	ROAD_INDEX_NO_EXIST                      = 9,     //不存在的路口编号
	PHASE_INDEX_NO_EXIST                     = 10,    //不存在的相位编号
	USER_NO_PERMISSION                       = 11,    //该用户没有权限
	AREA_OR_ROAD_NO_EXIST                    = 12,    //区域或路口不存在
	ACTION_LIST_INDEX_INVALID                = 13,    //无效的操作列表编号
	READ_FILE_FAILED                         = 14,    //文件读取失败
	ACTION_LIST_INDEX_NO_EXIST               = 15,    //操作列表编号不存在
	ACTION_LIST_COMMAND_ERROR                = 16,    //操作列表中命令错误
	ROAD_ID_NO_EXIST                         = 17,    //路口ID不存在
	CYCLE_INCONSISTENT_IN_DIFFENT_RING       = 18,    //不同环周期不一致
	SPLIT_TIME_LESS_THAN_MIN_GREEN           = 19,    //绿信比小于最小绿
	SPLIT_TIME_MORE_THAN_MAX_GREEN           = 20,    //绿信比小于最小绿
	NO_SUPPORT_PHASE_TYPE                    = 21,    //不支持的相位类型
	NO_SUPPORT_PHASE_COUNT                   = 22,    //不支持的相位数量
	CERTIFIED_FAILED                         = 23,    //认证失败
	KEYFILE_NO_EXIST                         = 24,    //校验信息不存在
	INVALID_PROTOCOL                         = 25,    //协议不兼容
}ESystemControlFailCode;


/*特征参数准备失败原因*/
const unsigned char C_CH_ISPARAMETERRET_NO				= 0x00;         //特征参数无异常
const unsigned char C_CH_ISPARAMETERRET_NOEXIST			= 0x01;			//文件不存在
const unsigned char C_CH_ISPARAMETERRET_NOREAD			= 0x02;			//文件不可读
const unsigned char C_CH_ISPARAMETERRET_CHKERR			= 0x03;			//文件校验失败
const unsigned char C_CH_ISPARAMETERRET_NO_OPEN			= 0x04;			//文件打开失败
const unsigned char C_CH_ISPARAMETERRET_NO_UPDATE		= 0x05;			//参数更新失败
const unsigned char C_CH_ISPARAMETERRET_NO_CHKERRSITE	= 0x06;			//地址码检验失败
const unsigned char C_CH_ISPARAMETERRET_NO_FORMAT_ERR	= 0x07;			//json格式错误
const unsigned char C_CH_ISPARAMETERRET_PARAM_ERR		= 0x08;			//特征参数校验失败
const unsigned char C_CH_ISPARAMETERRET_MOUNT_FAILED	= 0x09;         //USB挂载失败
const unsigned char C_CH_ISPARAMETERRET_USB_NOT_FIND	= 0x0A;         //USB没找到

/*特征参数变化状态*/
const unsigned char C_CH_ISPARAMETERCHG_NO = 0x00;						//特征参数无变化
const unsigned char C_CH_ISPARAMETERCHG_OK = 0x01;						//特征参数有变化    

/*特征参数准备状态*/
const unsigned char C_CH_PARAMERREADY_NO	= 0x00;						//特征参数准备失败
const unsigned char C_CH_PARAMERREADY_OK	= 0x01;						//特征参数准备完毕
const unsigned char C_CH_PARAMERREADY_NOT	= 0x02;						//特征参数未初始化

/*特征参数变化来源*/
const unsigned char C_CH_ISPARAMETERCHG_SRC_CHANNEL = 0x01;				//特征参数通道数量有变化
const unsigned char C_CH_ISPARAMETERCHG_SRC_OTHER	= 0x02;				//其他参数发生变化

const unsigned long C_N_MAXGLOBALCOUNTER = 65536;						//全局计数最大值

const unsigned long C_N_BOARDCOUNTER_KEY = 120;							//判断板卡是否在线的计数器阈值

const unsigned long C_N_LAMPCTLBOARDCOUNTER_KEY = 40;					//判断灯控板是否在线的计数器阈值

const unsigned long C_N_FLASHLAMPCLRSTATUS_COUNTER = 10;				//判断亮灭间隔的计数器阈值

const unsigned long C_N_PULSELAMPCLRSTATUS_COUNTER = 4;					//判断脉冲间隔的计数器阈值

const unsigned long C_N_TIMER_MILLSECOND = 50;							//毫秒定时器

const unsigned long C_N_TIMER_TIMER_COUNTER = 20;						//环相位灯色变化计数器阈值

const unsigned long C_N_LAMPCTLBOARDFAULT = 400;						//读故障板故障间隔的计数器阈值

const unsigned long C_N_MAXWRITECYCLECHGCOUNTER = 32768;				//往故障检测板写控制周期次数最大值

const unsigned long C_N_HWPANEL_BTN_COUNT = 20;							//手动面板按钮数量  

const unsigned long C_N_FAULT_TIMER_COUNTER = 20;						//故障间隔的计数器阈值         

const unsigned long C_N_MAX_FAULTQUEUE_SIZE = 3000;						//发到中心的故障队列长度

const unsigned long C_N_MAX_FAULT_COUNT = 300;                          //最大故障数量

const int C_N_DETBOARDID_DEFAULT_START = 0x31;                          //车检板ID默认起始值

typedef struct tagParamRunStatus
{
    unsigned char m_chParameterReady;									//特征参数是否准备完毕
    unsigned char m_chParameterRet;										//特征参数异常原因
    unsigned char m_chIsParameterChg;									//特征参数是否变化
    unsigned char m_chParameterChgSrc;									//特征参数变化来源
}TParamRunStatus,*PTParamRunStatus;

typedef struct tagOneLampCtlBoardData
{
    unsigned int  m_nID;												//板卡ID
    unsigned long  m_nCounter;											//最新状态计数器
	char m_achLampGroupStatus[C_N_CHANNELNUM_PER_BOARD];				//灯色状态数组
    char m_achLampGroupPower[C_N_CHANNELNUM_PER_BOARD];					//功率数组
	char m_achDeFault[2];												//保留
}TOneLampCltBoardData,*PTOneLampCltBoardData;

typedef struct tagOneLampCtlBoardFault
{
    unsigned int   m_nID;														//板卡ID
	bool           m_bFaultStatus[C_N_CHANNELNUM_PER_BOARD][C_N_CHANNEL_OUTPUTNUM];//灯组故障状态,true为故障,false为正常
	unsigned int   m_nFault[C_N_CHANNELNUM_PER_BOARD][C_N_CHANNEL_OUTPUTNUM];	//灯组故障
    unsigned long  m_nReadFaultTypeCounter[C_N_LAMPBORAD_OUTPUTNUM];			//读故障数据计数器
    unsigned long  m_bReadStatus[C_N_LAMPBORAD_OUTPUTNUM]; //灯组故障读取状态,true为需要读,false为不需要读
}TOneLampCltBoardFault,*PTOneLampCltBoardFault;

typedef struct tagLampCtlBoardData
{
    int m_nLampCltBoardCount;                                       //灯控板数量
    TOneLampCltBoardData  m_atLampData[C_N_MAXLAMPBOARD_NUM];       //灯控板灯色数组
	TOneLampCltBoardFault m_atLampFault[C_N_MAXLAMPBOARD_NUM];      //灯控板故障数组  
	bool                  m_bReadParaStatus;                        //灯组实时数据读取状态,true为需要读,false为不需要读
}TLampCltBoardData,*PTLampCltBoardData;

typedef struct tagOneVehDetBoardData
{
    int m_nVehDetBoardID;                               //车检板ID号,0表示未初始化
    char m_achVehChgVal[C_N_MAXDETINPUT_NUM];           //车检板检测信息变化量数组
    int m_anVehChgValCounter[C_N_MAXDETINPUT_NUM];      //车检板变化量时间戳数组
    char m_achVehTimerVal[C_N_MAXDETINPUT_NUM];         //车检板检测信息全量数组        
    int m_nVehTimerValCounter;                          //车检板全量检测信息时间戳
    bool m_bDetFaultStatus[C_N_MAXDETINPUT_NUM];        //线圈状态,true为故障,false为正常 
    unsigned int   m_nFault[C_N_MAXDETINPUT_NUM];       //线圈故障   
	unsigned long  m_nReadFaultTypeCounter[C_N_MAXDETINPUT_NUM];//读故障数据计数器
	bool m_bReadStatus[C_N_MAXDETINPUT_NUM];            //线圈故障读取状态,true为需要读,false为不需要读
	bool m_bVehDetExist[C_N_MAXDETINPUT_NUM];			//线圈是否存在，true为存在，false为不存在
}TOneVehDetBoardData,*PTOneVehDetBoardData;

typedef struct tagVehDetBoardData
{
    TOneVehDetBoardData m_atVehDetData[C_N_MAXDETBOARD_NUM];
}TVehDetBoardData,*PTVehDetBoardData;

typedef struct tagRealTimeVehDetData
{   
    int m_nDetNum;                                                              //检测器数量
    bool m_bIsNewVehCome[C_N_MAXDETBOARD_NUM * C_N_MAXDETINPUT_NUM];            //是否有车进入线圈
    bool m_bDetFaultStatus[C_N_MAXDETBOARD_NUM * C_N_MAXDETINPUT_NUM];          //线圈状态,true为故障,false为正常
    char m_chDetStatus[C_N_MAXDETBOARD_NUM * C_N_MAXDETINPUT_NUM];              //车检器最新的状态,0为没有车占有,1为有车占有
    int  m_anDetStatusCounter[C_N_MAXDETBOARD_NUM * C_N_MAXDETINPUT_NUM];       //车检器最新状态计数器
	bool m_bVehDetExist[C_N_MAXDETBOARD_NUM * C_N_MAXDETINPUT_NUM];				//线圈是否存在，true为存在，false为不存在
	char m_chDetStatusInGreen[C_N_MAXDETBOARD_NUM * C_N_MAXDETINPUT_NUM];       //绿灯时，车检器最新的状态,0为没有车占有,1为有车占有
    int  m_anDetStatusCounterInGreen[C_N_MAXDETBOARD_NUM * C_N_MAXDETINPUT_NUM];//绿灯时，车检器最新状态计数器
}TRealTimeVehDetData,*PTRealTimeVehDetData;                                     //用于感应和优化控制、流量统计

typedef struct tagOneIOBoardData
{
    int m_nIOBoardID;                                               //IO板ID号,0表示未初始化
    unsigned long  m_nCounter;                                      //最新数据计数器
    char m_achIOStatus[C_N_MAXIOINPUT_NUM];                         //IO板IO状态信息    
}TOneIOBoardData,*PTOneIOBoardData;

typedef struct tagIOBoardData
{
    TOneIOBoardData m_atIOBoardData[C_N_MAXIOBOARD_NUM];
}TIOBoardData,*PTIOBoardData;

typedef struct tagFaultDetBoardData
{
    int m_nFaultDetBoardID;												//故障检测板ID号,0表示未初始化
    unsigned long  m_nCounter;											//最新数据计数器
}TFaultDetBoardData,*PTFaultDetBoardData;

typedef struct tagMainCtlBoardRunStatus
{
	bool m_bIsNeedGetParam;												//逻辑控制模块是否需要获取特征参数
    bool m_bIsUseNewParamForFault;										//逻辑控制是否使用了新参数，通知故障检测模块
    bool m_bIsUseNewParamForHard;										//逻辑控制是否使用了新参数，通知硬件模块
}TMainCtlBoardRunStatus,*PTMainCtlBoardRunStatus;						//主控板运行状态信息结构体

typedef struct tagPhaseLampClrRunCounter
{
    unsigned long m_nCurCounter;										//当前计数器的值
    unsigned long m_nLampClrStartCounter[C_N_MAXRING_NUM];				//灯色起始计数，每环一个
    unsigned long m_nPedLampClrStartCounter[C_N_MAXRING_NUM];			//行人相位灯色起始计数，每环一个
    unsigned long m_nLampClrTime[C_N_MAXRING_NUM];                  //灯色计时，每环一个，其中m_nLampClrCounter[0]用于启动时序计时
    unsigned long m_nPedLampClrTime[C_N_MAXRING_NUM];					//行人相位灯色计时，每环一个，其中m_nLampClrCounter[0]用于启动时序计时
}TPhaseLampClrRunCounter,*PTPhaseLampClrRunCounter;

typedef struct tagLampClrStatus
{
    char m_achLampClr[C_N_MAXLAMPOUTPUT_NUM];							//灯控板端子的灯色输出
    bool m_bGreenLampPulse[C_N_MAXLAMPOUTPUT_NUM];						//相位是否有绿灯脉冲
    bool m_bRedLampPulse[C_N_MAXLAMPOUTPUT_NUM];						//相位是否有红灯脉冲
    bool m_bIsRefreshClr;												//是否需要刷新灯色
}TLampClrStatus,*PTLampClrStatus;										//灯色信息结构体

typedef struct tagLogicCtlStatus
{
    int m_nCurCtlMode;													//当前的控制方式
    int m_nCurPlanNo;													//当前使用的方案号
    int m_nRunStage;													//信号机当前运行阶段
}TLogicCtlStatus,*PTLogicCtlStatus;										//逻辑控制状态信息结构体

typedef struct tagFlashLampClrStatus
{
    bool m_bFlashLampClrStatus[C_N_MAXLAMPOUTPUT_NUM];					//闪灯状态 
    unsigned long m_nFlashLampClrStatusCounter[C_N_MAXLAMPOUTPUT_NUM];	//亮灭的计数器                                                      
}TFlashLampClrStatus,*PTFlashLampClrStatus;								//闪灯状态记录结构体

typedef struct tagLampPulseStatus
{
    bool m_bGreenLampPulse[C_N_MAXLAMPOUTPUT_NUM];						//绿灯脉冲状态  
    bool m_bRedLampPulse[C_N_MAXLAMPOUTPUT_NUM];						//红灯脉冲状态  
    unsigned long m_nGreenLampPulseCounter[C_N_MAXLAMPOUTPUT_NUM];		//绿灯脉冲的计数器 
    unsigned long m_nRedLampPulseCounter[C_N_MAXLAMPOUTPUT_NUM];		//红灯脉冲的计数器                                    
}TLampPulseStatus,*PTLampPulseStatus;									//灯脉冲状态记录结构体

typedef enum
{
	DeviceOnLine                                = -1,                //设备正常
	DeviceOffLine                               = 1,                 //设备离线 
	OpenTheDoor                                 = 2,                 //信号机开门 
	RedAndGreenConflict                         = 3,                 //红绿冲突 
	RedLightGoOut                               = 4,                 //红灯熄灭 
	GreenLighConflict                           = 5,                 //绿灯冲突 
	HighVoltage                                 = 6,                 //电压过高 
	LowVoltage                                  = 7,                 //电压过低 
	GreenLightGoOut                             = 8,                 //绿灯熄灭 
	FuseFailure                                 = 9,                 //保险丝故障
	PhasePlateReverseError                      = 10,                //相位板反较错误
	MultipleJunctionParameteConfigurationError  = 11,                //多路口参数配置错误
	LightsOff                                   = 12,                //灯控关闭
	PedestrianButtonOpeningFailure              = 13,                //行人按钮开环故障
	PedestrianButtonClosingFailure              = 14,                //行人按钮闭环故障
	CountDownCardCommunicationFailure           = 15,                //倒计时牌通信故障
	CommunicationFailureOfStrategicDetector     = 16,                //战略检测器通信故障
	DetectorIsNotRespondingLongTime             = 17,                //检测器长时间不响应
	DetectorClosedLoopError                     = 18,                //检测器闭环错误
	DetectorCountError                          = 19,                //检测器计数错误
	CabinetReportFrontDoorOpen                  = 20,                //机柜报告前门开
	CabinetReportBackDoorOpen                   = 21,                //机柜报告后门开
	CabinetReportSmallDoorOpen                  = 22,                //机柜报告小门开
	CabinetReportHighTempture                   = 23,                //机柜报告温度过高
	CabinetReportHighWaterLevel                 = 24,                //机柜报告水位过高
	CabinetReportBigHumidity                    = 25,                //机柜报告湿度过大
	CabinetReportPoint1Novoltage                = 26,                //机柜报告监测点1电压无
	CabinetReportPoint2Novoltage                = 27,                //机柜报告监测点2电压无
	CabinetReportPoint3Novoltage                = 28,                //机柜报告监测点3电压无
	CabinetReportPoint4Novoltage                = 29,                //机柜报告监测点4电压无
	CellReportOn                                = 30,                //机柜报告上电
	LockerReportIllegalOpen                     = 31,                //机柜报告非法开门
	LockerReportlegalOpen                       = 32,                //机柜报告合法开门
	CabinetReportsVibration                     = 33,                //机柜报告震动产生
	ManualControlNoOperation                    = 34,                //手动控制时，长时间无操作
	GreenLightTooShort                          = 35,                //绿灯时间过短（方案不合理）
}DeviceStatus;

typedef enum
{
	PhasePassStatus_Normal						= 0,
	PhasePassStatus_Close						= 1,
	PhasePassStatus_VehClose					= 2,
	PhasePassStatus_PedClose					= 3,
}PhasePassStatus;

typedef union tagFaultType
{
	unsigned short Hex;                     
	struct
	{
		unsigned  bDevice_Fault             :1;
		unsigned  bRedLamp_Fault            :1;
		unsigned  bVoltage_Fault            :1;
		unsigned  bGreenLamp_Fault          :1;
		unsigned  bFuse_Fault               :1;
		unsigned  bPhaseBoard_Fault		    :1;     
		unsigned  bConfig_Fault             :1;
		unsigned  bRing_Fault		        :1;	
		unsigned  bCom_Fault		        :1;	
		unsigned  bDetector_Fault		    :1;	
		unsigned  bCabinet_Fault		    :1;	
		unsigned  bReserved                 :8;
	}Bin;                                                            
}TFaultType,*PTFaultType;

typedef union tagCabinetFaultType
{
	unsigned  long	lFault;
	struct
	{
		unsigned	bCabinetReportHighTempture_Fault:1;		//温度故障
		unsigned	bCabinetReportHighWaterLevel_Fault:1;	//水位故障
		unsigned	bCabinetReportBigHumidity_Fault:1;		//湿度故障
		unsigned	bCabinetReportPoint1Novoltage_Fault:1;	//监测点1电压故障
		unsigned	bCabinetReportPoint2Novoltage_Fault:1;	//监测点2电压故障
		unsigned	bCabinetReportPoint3Novoltage_Fault:1;	//监测点3电压故障
		unsigned	bCabinetReportPoint4Novoltage_Fault:1;	//监测点4电压故障
		unsigned	bLockerReportIllegalOpen_Fault:1;		//开门故障
		unsigned	bCabinetReportsVibration_Fault:1;		//震动故障
		unsigned	bManualControlNoOperation_Fault:1;	    //手动控制时，长时间无操作故障
		unsigned	bGreenLightTooShort_Fault:1;            //绿灯时间过短
		unsigned	bReserved:21;
	}Bin;
}TCabinetFaultType,*PTCabinetFaultType;

typedef union tagDetectorFaultType
{
	unsigned char chFault;
	struct 
	{
		unsigned 	bCommunicationFailureOfStrategicDetector_Fault:1;//通信故障
		unsigned 	bDetectorIsNotRespondingLongTimeFault:1;		 //不回应故障
		unsigned 	bDetectorClosedLoopError_Fault:1;		         //闭环错误
		unsigned 	bDetectorCountError_Fault:1;		             //计数错误
		unsigned 	bReserved:4;
	}Bin;
}TDetectorFaultType,*PTDetectorFaultType;

typedef union tagLampFaultType
{
	unsigned char chFault;
	struct  
	{
		unsigned  bRedAndGreenConflict_Fault:1;//红绿冲突 
		unsigned  bRedLightGoOut_Fault:1;	   //红灯熄灭
		unsigned  bGreenLighConflict_Fault:1;  //绿灯冲突
		unsigned  bGreenLightGoOut_Fault:1;	   //绿灯熄灭
		unsigned  LightsOfft_Fault:1;          //灯控关闭
        unsigned  bLampNumger_Fault:1;         //灯控板数量错误
        unsigned  bFaultBoardOffline_Fault:1;  //故障板掉线
		unsigned  bReserved:2;
	}Bin;
}TLampFaultType,*PTLampFaultType;

#pragma pack(1)
typedef struct tagCommonFault
{
	union tagCabinetFaultType	    m_tCabinetFault;
	union tagDetectorFaultType	    m_tDetectorFault;
	union tagLampFaultType  	    m_tLampFault;
}TCommonFault,*PTCommonFault; 
#pragma pack()

typedef union tagCanData
{
	unsigned  char	chCanData;
	struct    
	{   
		unsigned io1:1;   
		unsigned io2:1;   
		unsigned io3:1;   
		unsigned io4:1;   
		unsigned io5:1;   
		unsigned io6:1;   
		unsigned io7:1;   
		unsigned io8:1; 
	 }CanData;
}TCanData, *PTCanData;

typedef struct tagAllBoardUseStatus
{
    char m_achLampCtlBoardStatus[C_N_MAXLAMPBOARD_NUM];								//灯控板使用信息数组
    char m_achVehDetBoardStatus[C_N_MAXDETBOARD_NUM];								//车检板使用信息数组
    char m_achIOBoardStatus[C_N_MAXIOBOARD_NUM];									//IO板使用信息数组
    char m_achDetectorUseStatus[C_N_MAXDETBOARD_NUM * C_N_MAXDETINPUT_NUM];			//车检器使用信息数组
}TAllBoardUseStatus,*PTAllBoardUseStatus;											//从特征参数中获得

typedef struct tagAllBoardOnlineStatus
{
    char m_achLampCtlBoardOnlineStatus[C_N_MAXLAMPBOARD_NUM];						//灯控板在线信息数组
    char m_achVehDetBoardOnlineStatus[C_N_MAXDETBOARD_NUM];							//车检板在线信息数组
    char m_achIOBoardOnlineStatus[C_N_MAXIOBOARD_NUM];								//IO板在线信息数组
    char m_chFaultDetectBoardStatus;												//故障检测板在线信息,1表示在线,0表示离线
}TAllBoardOnlineStatus,*PTAllBoardOnlineStatus;										//Can通信模块中获得

typedef struct tagOnePhaseRunStatus
{
    BYTE m_byPhaseID;																//相位编号
    int m_nSplitTime;																//绿信比时间
    char m_chPhaseStatus;															//相位的机动车状态
    int m_nCurStageRemainTime;														//当前阶段机动车剩余时间

	char m_chPedPhaseStatus;														//相位的行人状态
	int m_nCurPedRemainTime;														//相位的行人灯剩余时间

	BYTE m_byOverlapPhaseID;														//该相位作为母相位所对应的跟随相位的编号(如果不为母相位则默认为0)
	char m_chOverlapPhaseStatus;													//该相位作为母相位所对应的跟随相位的状态
	int m_nOverlapPhaseRemainTime;													//该相位作为母相位所对应的跟随相位的剩余时间

    BYTE m_abyPhaseConcurrency[MAX_PHASE_CONCURRENCY_COUNT];						//并发相位
	char m_chPhaseCloseStatus;                                                      //相位关断状态  1：关闭 0：取消关闭
	char m_chPhaseLockStatus;                                                       //相位锁定状态，1：锁定 0：取消锁定
	char m_chPhaseControlStatus;													//相位的屏蔽与禁止状态 0：无屏蔽与禁止 1：相位屏蔽 2：相位禁止
}TOnePhaseRunStatus,*PTOnePhaseRunStatus;

typedef struct tagOneOverlapPhaseRunStatus
{
	BYTE m_byOverlapPhaseID;														//跟随相位编号
	int m_nSplitTime;																//该跟随相位的绿信比时间
	BYTE m_byOverlapType;															//该跟随相位的机动车状态0关灯  1红 2 黄 3绿 4绿闪 5黄闪
	int m_nOverlapRemainTime;														//该跟随相位的机动车倒计时
	BYTE m_byOverlapPedType;														//该跟随相位的行人状态0关灯  1红 2 黄 3绿 4绿闪 5黄闪
	int m_nOverlapPedRemainTime;													//该跟随相位的行人倒计时
	int m_nMotherPhase[MAX_PHASE_COUNT_IN_OVERLAP];									//当前跟随相位的母相位
}TOneOverlapPhaseRunStatus, *PTOneOverlapPhaseRunStatus;

typedef struct tagOverlapRunStatus
{
	int m_nOverlapPhaseCount;														//跟随相位数量
	TOneOverlapPhaseRunStatus m_atOverlapPhaseRunStatus[MAX_OVERLAP_COUNT];
}TOverlapRunStatus, *PTOverlapRunStatus;

typedef struct tagOneRingRunStatus
{
    int m_nPhaseCount;																//环内相位数量
    int m_nCurRunPhaseIndex;														//当前运行的相位索引,从0开始
    int m_nCurStageIndex;															//当前运行的相位对应的阶段索引
    TOnePhaseRunStatus m_atPhaseStatus[MAX_SEQUENCE_TABLE_COUNT];					//环内运行状态
}TOneRingRunStatus,*PTOneRingRunStatus;

typedef struct tagOneRunStageInfo
{
    int  m_nStageIndex;
    int  m_nStageStartTime;
    int  m_nStageEndTime;
    int  m_nConcurrencyPhaseCount;
    int  m_nConcurrencyPhase[MAX_RING_COUNT];
	bool m_bDirectTransit[MAX_RING_COUNT];
	bool m_bPhaseStartTransitInCurStage[MAX_RING_COUNT];
	bool m_bPedPhaseStartTransitInCurStage[MAX_RING_COUNT];
	int  m_nPhaseCurStageTransitTime[MAX_RING_COUNT];
	int  m_nPedPhaseCurStageTransitTime[MAX_RING_COUNT];
}TOneRunStageInfo,*PTOneRunStageInfo;

typedef struct tagRunStageInfo
{
    int  m_nRunStageCount;
    TOneRunStageInfo  m_PhaseRunstageInfo[MAX_STAGE_COUNT];
}TRunStageInfo,*PTRunStageInfo;

typedef struct tagDirectionKeyChannelClr
{      
    int            m_nChannelID;                                                   //通道ID                                        
    char           m_achChannelClr;                                                //方向控制对应的通道灯色                          
    unsigned long  m_nChannelDurationTime;					                       //方向控制对应的通道灯色持续时间
}TDirectionKeyChannelClr,*PTDirectionKeyChannelClr;

typedef struct tagLockPhaseClr
{      
    BYTE           m_byPhaseID;                                                    //相位ID                                        
    char           m_achPhaseClr;                                                  //相位灯色                          
    unsigned long  m_nPhaseClrDurationTime;					                       //相位持续时间
}TLockPhaseClr,*PTLockPhaseClr;

typedef struct tagPhaseRunStatus
{
    int m_nCurCtlPattern;															//控制模式
    int m_nCurCtlMode;																//控制方式
    BYTE m_byPlanID;																//方案编号
    char m_achPlanName[12];															//方案名称
    int m_nCycleLen;																//周期长
    int m_nOffset;																	//相位差
	int m_nPatternOffset;                                                           //协调相位差
    int m_nCycleRunTime;															//周期运行时间
    int m_nCycleRemainTime;															//周期剩余时间
    int m_nRingCount;																//环数量
    TOneRingRunStatus m_atRingRunStatus[MAX_RING_COUNT];							//环运行状态
    TRunStageInfo     m_tRunStageInfo;												//阶段表
	TOverlapRunStatus m_atOverlapRunStatus;											//跟随相位运行状态
}TPhaseRunStatus,*PTPhaseRunStatus;													//当前方案运行状态信息

typedef struct tagHWPanelBtnStatus
{
    int  m_nHWPanelBtnIndex;														//面板按钮编号
    bool m_bHWPanelBtnStatus;														//面板按钮状态
}THWPanelBtnStatus,*PTHWPanelBtnStatus;              

typedef struct tagGpsData
{
    long	m_nGpsTime;                                                            //时间
    double	m_nLongitude;                                                          //经度
    double	m_nLatitude;                                                           //纬度
}TGpsData,*PTGpsData;    

typedef struct tagWholeDeviceStatusInfo
{
    char 	m_cSiteID[4];															//整机编码
	char	m_cHardwareVer[10];														//硬件版本号
    char	m_cVoltage;																//电压值
    char	m_cRunStatus;															//运行状态
}TWholeDeviceStatusInfo,*PTWholeDeviceStatusInfo;            

typedef struct tagBoardStatusInfo
{
    char 	m_cID;																	//槽位id
    char	m_cBoardType;															//板卡类型
    char	m_cRunStatus;															//运行状态
}TBoardStatusInfo,*PTBoardStatusInfo;     

typedef struct tagYellowFlashStatusInfo
{
    char	m_cRunStatus;															//运行状态
    char	m_cTriggerStatus;														//硬黄闪状态
}TYellowFlashStatusInfo,*PTYellowFlashStatusInfo;

typedef struct tagOpenATCStatusInfo
{
    TWholeDeviceStatusInfo	m_tWholeDevStatusInfo;									//整机状态
	char					m_cInUsedFuncBoardCount;
    TBoardStatusInfo		m_tFuncBoardStatusInfo[C_N_MAX_FUNC_CARDSLOT_NUM];		//功能板卡状态
	char					m_cInUsedCtrlBoardCount;
    TBoardStatusInfo		m_tCtrlBoardStatusInfo[C_N_MAX_CTRL_CARDSLOT_NUM];		//主控板卡状态
	TYellowFlashStatusInfo	m_tYellowFlashStatusInfo;								//黄闪器状态
}TOpenATCStatusInfo,*PTOpenATCStatusInfo;    

typedef struct tagLedScreenRingInfo
{
	int m_nCurRunPhaseIndex;														//当前运行相位索引值
	int m_nSplitTime;																//绿信比时间
}TLedScreenRingInfo,*PTLedScreenRingInfo;

typedef struct  tagRunFaultInfo
{
	char                     m_cBoardType;                                          //板卡类型
    char                     m_cFaultLevel;                                         //故障级别
	DWORD                    m_wFaultType;                                          //故障类型
	DWORD                    m_wSubFaultType;                                       //故障子类型
	char		             m_faultInfo[10];                                       //故障描述
	long                     m_unFaultOccurTime;   			                        //故障发生时间 
}TRunFaultInfo,*PTRunFaultInfo;   

typedef struct tagChannelLockCtrlCmd
{
	int  m_nDuration;									//持续时长
	int  m_nGreenFlash;									//绿闪过渡时间
	int  m_nYellow;										//黄灯过渡时间
	int  m_nRedClear;									//红灯过渡时间
	int	 m_nMinGreen;									//最小绿时间
	int  m_nChannelLockStatus[MAX_CHANNEL_COUNT];       //通道锁定状态状态，0为恢复默认状态，1为红灯，2为黄灯，3为绿灯，4为绿闪，5为灭灯，6为红闪
}TChannelLockCtrlCmd,*PTChannelLockCtrlCmd;

typedef struct tagPhaseLockCtrlCmd
{
	int  m_nDuration;									//持续时长
	int  m_nGreenFlash;									//绿闪过渡时间
	int  m_nYellow;										//黄灯过渡时间
	int  m_nRedClear;									//红灯过渡时间
	int	 m_nMinGreen;									//最小绿时间
	int  m_nPhaseLockType[MAX_PHASE_COUNT];             //相位锁定类型，0为取消锁定，1机动车和行人都锁定，2仅锁定机动车相位(包括跟随相位)，3仅锁定行人(包括跟随相位)
}TPhaseLockCtrlCmd,*PTPhaseLockCtrlCmd;

typedef struct tagPhaseLockPara
{
	int  m_nPhaseLockCount;                             //锁定相位数量
	int  m_nPhaseLockID[MAX_PHASE_COUNT];               //锁定相位ID
	int  m_nPhaseLockType[MAX_PHASE_COUNT];             //锁定相位类型
	int  m_nOverlapPhaseLockCount;                      //锁定跟随相位数量
	int  m_nOverlapPhaseLockID[MAX_PHASE_COUNT];        //锁定跟随相位ID
	int  m_nOverlapPhaseLockType[MAX_PHASE_COUNT];      //锁定跟随相位类型
}TPhaseLockPara,*PTPhaseLockPara;

typedef struct tagLedScreenShowInfo
{
	int m_nPlanNo;                                                                  //方案号
	int m_nCurCtlMode;																//当前的控制方式
	int m_nSubCtlMode;                                                              //当前的控制子模式
    int m_nCycleLen;																//周期长
    int m_nOffset;																	//相位差
    int m_nRingCount;																//环数量
	TLedScreenRingInfo m_tScreenRingInfo[C_N_MAX_SCREEN_RING_NUM];					//当前环运行状态

    bool                     m_bKeyDirectionControl;                                //是否按了方向键
    int                      m_nDirectionKeyIndex;                                  //方向键编号
    int                      m_nChannelCount;                                       //方向控制对应的通道数量   
    TDirectionKeyChannelClr  m_tDirectionKeyChannelClr[MAX_CHANNEL_COUNT];          //方向键控制的通道灯色

	bool                     m_bChannelCheck;                                       //是否按了通道检测键
	TDirectionKeyChannelClr  m_tChannelCheckClr;                                    //通道检测控制的通道灯色

	unsigned int             m_nRunFaultCount;                                      //运行故障数量
	TRunFaultInfo            m_tRunFaultInfo[C_N_MAX_FAULT_COUNT];                  //运行故障

	bool                     m_bChannelLockCheck;                                   //是否下发通道锁定指令
	TChannelLockCtrlCmd      m_tChannelLockCtrlCmd;                                 //通道锁定参数
	int                      m_nChannelLockCount;                                   //通道锁定对应的通道数量   
    TDirectionKeyChannelClr  m_tChannelLockChannelClr[MAX_CHANNEL_COUNT];           //通道锁定的通道灯色

	bool                     m_bPhaseToChannelLock;                                 //相位锁定转通道锁定标志
	int                      m_nPhaseLockCount;                                     //锁定相位数量   
	TLockPhaseClr            m_tPhaseClr[MAX_PHASE_COUNT];                          //锁定相位灯色

}TLedScreenShowInfo,*PTLedScreenShowInfo;        

typedef struct  tagSelfDetectInfo
{
    char    m_cSelfDetectStatus; 
    char    m_cSelfDetectFailedReason;
	char    m_cSelfDetectInfo[10]; 
}TSelfDetectInfo,*PTSelfDetectInfo;   

typedef struct tagVehicleQueueUpInfo
{
    int  m_nDetectorID;									//检测器ID
	BYTE m_byVehicleDetectorCallPhase;					//检测器对应的请求相位
	int  m_nVehicleQueueUpLength;						//排队长度
}TVehicleQueueUpInfo,*PTVehicleQueueUpInfo;

typedef struct tagPedDetectInfo
{
    int  m_nDetectorID;									//行人检测器ID
	BYTE m_byVehicleDetectorCallPhase;					//行人检测器对应的请求相位
	BYTE m_byDetectorType;								//检测器类型
	int  m_nPedCount;									//行人检测数量
}TPedDetectInfo,*PTPedDetectInfo;

typedef struct tagChannelLampStatus
{
	int m_nChannelID;
	int m_nLampLight;
	int m_nCountDown;
}TChannelLampStatus,*PTChannelLampStatus; 

typedef struct tagSystemControlStatus
{
	int  m_nSpecicalControlResult;						//特殊控制结果，如：黄闪，全红，关灯
	int  m_nPatternControlResult;						//方案控制结果
	int  m_nStageControlResult;							//阶段控制结果
	int  m_nPhaseControlResult;                         //相位关断控制结果
	int  m_nChannelLockResult;                          //通道锁定控制结果
	int  m_nSpecicalControlFailCode;					//特殊控制失败原因
	int  m_nPatternControlFailCode;						//方案控制失败原因
	int  m_nStageControlFailCode;						//阶段控制失败原因
	int  m_nPhaseControlFailCode;                       //相位关断控制失败原因
	int  m_nChannelLockFailCode;                        //通道锁定控制失败原因
}TSystemControlStatus,*PTSystemControlStatus; 

typedef struct tagStepForwardCmd
{
	int  m_byStepType;									//当前步进类型 0:阶段 1:灯色
	int  m_nNextStageID;								//下一个阶段编号，从1开始，0是步进
	int  m_nDurationTime;								//持续时长
	int  m_nDelayTime;									//延迟时间
}TStepForwardCmd,*PTStepForwardCmd; 

typedef struct tagDirectionCmd
{
	bool m_bStepFowardToDirection;						//true表示从步进切过来的方向，false表示方案切过来的方向
	int  m_nTargetStageIndex;							//切方向之前的阶段的下一个阶段编号
	int  m_nNextDirectionIndex;							//下一个方向编号
}TDirectionCmd,*PTDirectionCmd; 

typedef struct tagPatternInterruptCmd
{
	int  m_nControlMode;								//控制方式
	int  m_nPatternNo;									//方案号
	TInterruptPatternInfo m_tManualControlPattern;      //手动控制方案
}TPatternInterruptCmd,*PTPatternInterruptCmd;

typedef struct tagChannelLockCmd
{
	bool m_bStepFowardToChannelLock;					//true表示从步进切过来的通道锁定，false表示方案切过来的通道锁定
	int  m_nTargetStageIndex;							//切通道锁定之前的阶段的下一个阶段编号
	int  m_nDelayTime;                                  //从步进切到通道锁定时的延迟时间
	int  m_nDurationTime;                               //从步进切到通道锁定时的持续时间
	TChannelLockCtrlCmd  m_tNextChannelLockCtrlCmd;		//下一个通道锁定参数
}TChannelLockCmd,*PTChannelLockCmd;

typedef struct tagManualCmd
{
	int                  m_nCtlMode;                     //控制方式
	int                  m_nSubCtlMode;                  //控制子模式
	int                  m_nCmdSource;                   //指令来源
	int                  m_nCurCtlSource;                //当前控制源
	char                 m_szPeerIp[20];                 //控制源IP
	bool                 m_bNewCmd;                      //新指令
	bool                 m_bStepForwardCmd;              //下一步是否需要处理步进
	TStepForwardCmd      m_tStepForwardCmd;              //步进命令      
    bool                 m_bDirectionCmd;                //下一步是否需要处理方向
	TDirectionCmd        m_tDirectionCmd;                //方向命令
	bool                 m_bPatternInterruptCmd;         //下一步是否需要方案干预
	TPatternInterruptCmd m_tPatternInterruptCmd;         //方案干预命令
	bool                 m_bChannelLockCmd;              //下一步是否需要处理通道锁定
	TChannelLockCmd      m_tChannelLockCmd;              //通道锁定指令
	bool                 m_bPhaseToChannelLock;          //相位锁定转通道锁定标志
	TPhaseLockPara       m_tPhaseLockPara;               //相位锁定转通道锁定指令的相位锁定参数
	bool                 m_bPreemptCtlCmd;               //下一步是否需要优先控制
}TManualCmd,*PTManualCmd; 

typedef struct tagPhasePassCmdPhaseStatus
{
	bool m_bNewCmd;										//是否为新指令，0为无需处理的旧指令，1为需要处理的新指令
	bool m_bUpdatePhasePassStatus[MAX_PHASE_COUNT];     //相位是否需要更新状态，0为不需要，1为需要
	int  m_nPhasePassStatus[MAX_PHASE_COUNT];           //相位放行状态，0为正常，1为关闭放行
}TPhasePassCmdPhaseStatus,*PTPhasePassCmdPhaseStatus;

#ifdef VIRTUAL_DEVICE
typedef struct tagVirtualRunTime
{
	int VirtualYear;			//虚拟运行的时间——年
	int VirtualMon;				//虚拟运行的时间——月[1,12]
	int VirtualDay;				//虚拟运行的时间——日[1,31]
	int VirtualHour;			//虚拟运行的时间——时[0,23]
	int VirtualMin;				//虚拟运行的时间——分[0,59]
	int VirtualSec;				//虚拟运行的时间——秒[0.59]
	int VirtualWeek;			//虚拟运行的时间——星期[0,6]
	DWORD TempGlobalCount;		//记录时间时的全局计数（用于下一次时间的推算）
}TVirtualRunTime, * PTVirtualRunTime;
#endif // VIRTUAL_DEVICE
//Virtual_Test2022

typedef struct tagPreemptControlStatus
{
	int  m_nPreemptControlResult;					    //优先控制结果
	int  m_nPreemptControlResultFailCode;               //优先控制失败原因
}TPreemptControlStatus,*PTPreemptControlStatus; 

typedef struct tagPreemptCtlCmd
{
	int                  m_nCmdSource;                   //指令来源
	int                  m_nCurCtlSource;                //当前控制源
	char                 m_szPeerIp[20];                 //控制源IP
	bool                 m_bNewCmd;						 //是否为新指令，0为无需处理的旧指令，1为需要处理的新指令
	BYTE                 m_byPreemptType;                //优先类型 0:常规优先 1:紧急优先 0:常规优先 2:默认
	BYTE                 m_byPreemptPhaseID;             //优先相位ID
	BYTE                 m_byPreemptStageIndex;          //优先相位对应的阶段编号
	WORD                 m_wPreemptDelay;                //延迟时间
	WORD                 m_wPreemptDuration;             //持续时间
	BYTE                 m_byPreemptLevel;               //优先级 1-5，等级越高，优先级越高
	BYTE                 m_byPreemptSwitchFlag;          //优先相位开始切换标志，0未开始切换，1已开始切换
	bool                 m_bPatternInterruptCmd;         //是否收到方案干预命令
	TPatternInterruptCmd m_tPatternInterruptCmd;         //方案干预命令
	bool                 m_bIncludeConcurPhase;          //是否包含并发相位
	BYTE                 m_byPreemptConcurPhaseID;       //优先并发相位ID
}TPreemptCtlCmd,*PTPreemptCtlCmd; 

//20999预留结构体
typedef struct tagDeviceStatus
{
	BYTE  m_byDoorStatus;                //柜门状态
	short m_nVoltage;                    //电压
	short m_nCurrent;                    //电流
	char  m_chTemperature;               //温度          
	BYTE  m_byHumidity;                  //湿度
}TDeviceStatus, * PTDeviceStatus;