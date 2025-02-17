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
#ifndef OPENATCPARAMSTRUCTDEFINE_H
#define OPENATCPARAMSTRUCTDEFINE_H

#include "../Include/OpenATCParamConstDefine.h"

#pragma warning (disable:4819)
//#pragma pack(push)//保存对齐状态
//#pragma pack(1)  //设置为1字节对齐

typedef struct tagAscStepCfg
{
	BYTE m_byStepType;			//默认0 步进，1 色步
}TAscStepCfg, *PTAscStepCfg;

typedef struct tagAscFaultCfg
{
	BYTE m_byIsCloseGreenAndRedOn;			//默认1 为开启，0 为关闭
	WORD m_wDetectGapTimeByGreenAndRedOn;	//红绿同亮检测时间间隔
	BYTE m_byIsCloseNoRedOn;
	WORD m_byDetectGapTimeByNoRedOn;		//无红灯亮起检测时间间隔
	WORD m_byDetectGapTimeByGreenConflict;	//绿冲突检测时间间隔
}TAscFaultCfg, *PTAscFaultCfg;

//按钮通道配置
typedef struct tagAscChannelCfg
{
	BYTE m_byChannelID;           //通道编号  (为已配通道编号)1-32
	BYTE m_byChannelStatus;       //通道状态   1--红灯  2--黄灯  3--绿灯  4--绿闪  5--灭灯     
}TAscChannelCfg, *PAscChannelCfg;

typedef struct tagAscRemoteControl
{
	BYTE m_byControlType;
	WORD m_wControlTime;
}TAscRemoteControl, *PAscRemoteControl;
//单按钮配置信息
typedef struct tagAscPanelOneKeyCfg
{
	BYTE m_byKeyNum;         /*    1--东西直行
							 2--北向通行
							 3--东西左转
							 4--西向通行
							 5--东向通行
							 6--南北直行
							 7--南向通行
							 8--南北左转
							 9--Y1自定义按钮
							 10--Y2自定义按钮
							 11--Y3自定义按钮
							 12--Y4自定义按钮*/
	char m_cPanelOneKeyDesc[MAX_DESC_LENGTH];		   /*描述*/
	TAscChannelCfg m_ChannelCfg[MAX_CHANNEL_COUNT];    //单个方向按钮对应的通道配置信息
}TAscPanelOneKeyCfg, *PAscPanelOneKeyCfg;

//手动面板配置
typedef struct tagAscManualPanel
{
	WORD m_wDuration;                  //单命令按键最大持续时间  0--无限制时间   1-600秒 
	BYTE m_byGreenFlash;               //过渡绿闪时间    单位秒   范围0-100整数
	BYTE m_byYellow;                   //过渡黄灯时间   单位秒   范围0-100整数
	BYTE m_byRedClear;                 //过渡全红时间   单位秒   范围0-100整数
	BYTE m_byMinGreen;                 //最小绿时间    单位秒   范围0-255整数
	BYTE m_byMinGreenIgnore;           //最小绿是否忽略 0 不忽略 1 忽略
	TAscPanelOneKeyCfg m_stPanelKeyCfg[MAX_PANEL_KEY_CFG_COUNT];
}TAscManualPanel, *PAscManualPanel;

//信号机识别码（地址的地址码）结构体定义
typedef struct tagAscAddressCode
{
	char m_chAscAddressCode[MAX_SITEID_COUNT];
}TAscAddressCode, *PTAscAddressCode;

//信号机网卡信息配置 结构体定义
typedef struct tagAscNetCard
{
	char m_chAscNetCardIp[MAX_IPSTR_LEN];		//ip
	char m_chAscSubnet[MAX_IPSTR_LEN];			//子网掩码
	char m_chGateway[MAX_IPSTR_LEN];			//网关
}TAscNetCard, *PTAscNetCard;

//信号机时区信息配置 结构体定义
typedef struct tagAscTimeZoneInfo
{
	int		m_nAscHour;							//时(东为负，西为正)
	int		m_nAscMinute;						//分
}TAscTimeZoneInfo, *PTAscTimeZoneInfo;

//平台ip和port定义
typedef struct tagAscCenter
{
	char m_chAscCenterIp[MAX_IPSTR_LEN];		//ip
	int m_chAscAscCenterPort;					//port
}TAscCenter, *PTAscCenter;

//仿真平台定义
typedef struct tagAscSimulate
{
	char m_chSimulateIP[MAX_IPSTR_LEN];			//ip
	int  m_nSimulatePort;						//port
	int  m_nCfgPort;
    int  m_nDetectorPort;
	int  m_nVideoDetectorPort;
}TAscSimulate, *PTAscSimulate;

//单点自适应控制参数
typedef struct tagAscSingleOptim
{
	BYTE m_byPattern;												//单点自适应控制关联方案号
	BYTE m_bySelfLearn;												//饱和流量自学习开关
	int	m_nMaxSaturatedFlow;										//最大饱和流量
	float m_fGreenStartUpLoss;										//绿灯启动损失时间
	float m_fYellowEndLoss;											//黄灯结束损失时间
	int m_fCycleAdjustFactor;										//周期调整因子
	float m_fStaticWeight;											//静态权重
	float m_fPhaseStaticWeight[MAX_RING_COUNT][MAX_PHASE_COUNT];	//相位静态权重因子
}TAscSingleOptim, *PTAscSingleOptim;

//信号机故障结构体定义
typedef struct tagAscFault
{
	WORD m_wFaultID;            //故障ID,用于标识故障，便于后期故障删除、故障恢复、故障查询、识别是否为同一故障等功能 值1000起
	BYTE m_byBoardType;         //故障板卡类型    主控板--1，灯控板--2，   车检板--3，   I/O板--4
	long m_unFaultOccurTime;    //故障发生的时间   离格林尼治时间的秒数
	long m_unFaultRenewTime;    //故障恢复的时间   离格林尼治时间的秒数
	BYTE m_byFaultLevel;        //故障所属等级   一般故障--1，降级故障--2，严重故障--3
	WORD m_wFaultType;          //故障主类型
	WORD m_wSubFaultType;		  //故障子类型	（若无子类型值为0）
	                           /* m_wFaultType
									101          CanBus Fault              can总线通信故障
									102           Yellow Flasher Fault      黄闪器故障
									------------------------------------------------------------------
									103       TZParam Fault             特征参数故障
									                m_wSubFaultType     
									          ----->      1    Non-existent           特征参数不存在
									          ----->      2    File Is Unreadable     特征参数文件不可读
										     ----->      3    File Artificial Changes   特征参数人为修改
										     ----->      4    File Open Fail         特征参数文件打开失败
										     ----->      5    File Update Fail       特征参数文件更新失败
										     ----->      6    File Check SiteID Fail   信号机地址码校验失败
										     ----->      7    Format Error           特征参数内容格式错误
									------------------------------------------------------------------
									104     FaultDet Offline          故障检测板不在线
									201     LampBoard ID Fault        灯控板ID故障
									202     LampBoard Offline         灯控板脱机
									203     No Red Lamp Is On         无红灯亮起
									204     Red And Green Conflict    红绿同亮
									205     Green Conflict            绿冲突
									-----------------------------------------------------------------
									206    Red Lamp Voltage  Fault     红灯灯电压故障
									207    Yellow Lamp Voltage  Fault  黄灯灯电压故障
									208    Green Lamp Voltage  Fault   绿灯灯电压故障
									               m_wSubFaultType     
									             ---->  1    Output Volatage Is Fail      未输出有效电压
									             ---->  2    Output Volatage Is Low     输出电压低于输入电压过多
									             ---->  3    Output Volatage Is High      输出电压高于输入电压
											    ---->  4    Off Output Volatage Is high      关闭输出但实际电压仍然输出
											    ---->  5    Off Output Volatage Is low      关闭输出但实际电压部分输出
								                 ---->  6    Residual Voltage Is Over-High      线路残留电压过高

									------------------------------------------------------------------

									209   Red Lamp Lamp Power Fault       红灯灯功率故障
									210   Yellow Lamp Lamp Power Fault    黄灯灯功率故障
									211   Green Lamp Lamp Power Fault     绿灯灯功率故障
									               m_wSubFaultType     
									           ---->    1   Output Power Is Up           功率异常增加
									           ---->    2   Output Power Is Down         功率异常减少
									           ---->    3   Output Power Is Zero         功率无输出
									           ---->    4   Off Output Power Is High     关闭状态有功率输出
									-----------------------------------------------------------------
									301      VehDetBoard Is Not Init         车检板未初始化
									302      VehDetBoard Is Offline          车检板脱机
									303      VehDetector Short Circiut       车辆检测器短路
									304      VehDetector Open  Circiut       车辆检测器断路
																	*/
								
	BYTE m_byFaultSupple[MAX_CHANNEL_COUNT]; //故障补充说明值
							/*
							  值表示板卡编号、通道编号
							  当m_wFaultDescValue为其他值时，表示对应通道或者板卡编号
							*/
	

}TAscFault, *PAscFault;

//信号机操作记录结构体定义
typedef struct tagAscOperationRecord
{
    WORD m_wRecordID;										//操作记录ID，用于标识操作记录
	long m_unStartTime;										//操作记录的起始时间   离格林尼治时间的秒数
    long m_unEndTime;										//操作记录的结束时间   离格林尼治时间的秒数
	BYTE m_bySubject;										//主体标识  平台-0，配置软件-1，获取手动面板-2
    BYTE m_byObject;										//客体标识  一般指被操作的对象
    int  m_nInfoType;										//事件类型 
    bool m_bStatus;											//操作状态 true-成功 false-失败
 	BYTE m_byFailureValue[MAX_OPERATIONRECORD_DESC_LEN];	//操作失败描述                           							
}TAscOperationRecord, *PTAscOperationRecord;

//级联参数
typedef struct tagAscCasCadeInfo
{
	BYTE m_byLocalLampBoardNum;								//本机灯控板数量
	BYTE m_byLocalVehDetBoardNum;							//本机车检板数量
	BYTE m_byLocalIOBoardNum;								//本机IO板数量  
	BYTE m_byJoinOffset;									//级联偏移量
}TAscCasCadeInfo, *PTAscCasCadeInfo;

typedef struct tagAscChannelLockInfo
{
	BYTE m_byChannelID;										//通道编号
	BYTE m_byChannelLockStatus;								//通道锁定状态   0 为不指定状态     1--红灯  2--黄灯  3--绿灯  4--绿闪  5--灭灯   6--红闪
}TAscChannelLockStatus, *PAscChannelLockStatus;

//通道锁定参数
typedef struct tagAscOnePlanChannelLockInfo
{
	BYTE m_byStartHour;     //开始时间时
	BYTE m_byStartMin;      //开始时间分
	BYTE m_byStartSec;      //开始时间秒

	BYTE m_byEndHour;       //结束时间时
	BYTE m_byEndMin;        //结束时间分
	BYTE m_byEndSec;        //结束时间秒

	BYTE m_byGreenFlash;    //过渡绿闪时间
	BYTE m_byYellow;       //过渡黄灯时间
	
	TAscChannelLockStatus m_stChannelLockStatus[MAX_CHANNEL_COUNT];   

}TAscOnePlanChannelLockInfo, *PTAscOnePlanChannelLockInfo;



//车检检测器控制参数
typedef struct tagVehDetCtrParam
{
	BYTE m_byEnable;    //使能
	BYTE m_byMode;       //模式  0，默认，1，Boost模式
	WORD m_wPresentTime;/*存在时间  单位秒，
						超过这个时间设置的检测目标会自动清除，
						如果设置为0，则实际对应240秒*/
	BYTE m_bySense;     /*灵敏度  Sense，0~9预设灵敏度，数值越高灵敏度越高，
						一般初始化在3，预设灵敏度后面的几个参数不起作用9以上为自定义，
						自定义的时候后面几个参数才会产生作用*/
	WORD m_wSampleTime;  /*采样时间，单位微秒，时间越长累积信噪比越好，但是会导致测速精度变差，相应变慢*/
	WORD m_wInThresh;    /*进入阈值 越小越灵敏，当然误检概率也高，一般范围50~100*/
	WORD m_wOutThresh;   /*离开阈值  离开阈值，越小越灵敏，一般20~30，一定比InThresh小，否则工作不正常*/
	WORD m_wInFilter;      /*进入滤波 含义是连续检出多少次才认为是真正检测正确，一般3~5*/
	WORD m_wOutFilter;     /*离开滤波 ,连续检出多少次离开才认为离开，一般3~5*/

}TVehDetCtrParam, *PVehDetCtrParam;

//灯控板控制参数
typedef struct tagLampBoardCtrParam
{
	WORD m_wVoltThresh;   /*灯电压检测阈值 单位V，目前默认50V，
						   低于这个电压认为灯电压检测结果是0，高于是1，这个数据目前的信息是现场很有可能要改*/
	WORD m_wPacThresh;     /*灯功率检测阈值 单位0.1W，目前默认30，也就是功率超过3W检出*/
	WORD m_wPeakHThresh;   /*高功率故障阈值 百分比，也就是灯控检出瞬时功率超过平均功率多少认为故障，目前默认400，就是400%，
						   相当于超过平均功率4倍才报警*/
	WORD m_wPeakLThresh;   /*低功率故障阈值 瞬时功率低于平均功率的100/LThresh报警，目前默认130，
						   也就是低于当前平均功率的100/130报警*/
}TLampBoardCtrParam, *PLampBoardCtrParam;

//设备所属区域定义
typedef struct tagAscArea
{
	unsigned char m_chAscRegionNo;					//区域号
	unsigned short m_usAscRoadNo;					//路口号
}TAscArea, *PTAscArea;

//版本号结构定义
typedef struct tagVersionNumber
{
    DWORD m_dwMajorVersion;							//主版本号：大于等于1的整型数，小于DWORD的最大范围
    DWORD m_dwMinorVersion;							//次版本号：大于等于0的整型数，小于DWORD的最大范围
}TVersionNumber, *PVersionNumber;


//信号机特征参数版本结构定义
typedef struct tagAscParamFileHeader
{
	TVersionNumber m_stAscParamFileVersion;			//信号机特征参数脱机文件版本号
	BYTE m_byCtrlSpotAscType;						//控制点信号机类型：1-交叉口、2-快速路出口、3-快速路入口、4-车道灯、等
	BYTE m_byAscParamFileAuthor;					//信号机脱机文件的作者：1-掌上电脑、2-信号机维护工具、3-中心系统客户端、等
	TVersionNumber m_stAscParamFileAuthorVer;		//信号机脱机文件作者的版本号
	DWORD m_dwFileHeaderReserve;					//文件头的保留字节，用于以后的扩展	
}TAscParamFileHeader, *PTAscParamFileHeader;


//相位定义
typedef struct tagPhase
{
	BYTE m_byPhaseNumber;			/*2.2.2.1 相位号，不能超过maxPhases所定义的值。(1..32)*/
	WORD m_wPhaseWalk;				/*2.2.2.2行人放行时间，控制行人过街绿灯的秒数。(0..255),second*/
	BYTE m_byPhasePedestrianClear;	/* 2.2.2.3 行人清空时间（行人绿闪时间）(0..255)*/
	WORD m_wPhaseMinimumGreen;		/* 2.2.2.4最小绿灯时间，简称最小绿。															一般根据入口检测器与停车线间的车辆排队情况确定。(0..255)*/
	BYTE m_byPhasePassage;			/*2.2.2.5单位绿灯延长时间(0-25.5 sec)，简称单位延长绿。(0..255)*/
	WORD m_wPhaseMaximum1;			/* 2.2.2.6最大绿灯时间1，简称最大绿1。(0..255)*/
	WORD m_wPhaseMaximum2;			/* 2.2.2.7最大绿灯时间2，简称最大绿2。(0..255)*/
	BYTE m_byPhaseYellowChange;		/*2.2.2.8相位黄灯时间*/
	BYTE m_byPhaseRedClear;			/*2.2.2.9相位红灯清空时间*/
	BYTE m_byPhaseRedRevert;			/*2.2.2.10保证黄灯以后不能直接返回绿灯控制的红灯时间保护*/
	BYTE m_byPhaseAddedInitial;		/*2.2.2.11相位增加初始值*/
	BYTE m_byPhaseMaximumInitial;	/*2.2.2.12相位最大初始值*/
	BYTE m_byPhaseTimeBeforeReduction;/*2.2.2.13 gap递减之前的时间与2.2.2.14综合使用*/
	BYTE m_byPhaseCarsBeforeReduction;/*2.2.2.14 gap递减之前通过的车辆数*/
	BYTE m_byPhaseTimeToReduce;		/*2.2.2.15 gap递减到minigap的时间*/
	BYTE m_byPhaseReduceBy;			/*2.2.2.16单位递减率，与2.2.2.15和16任选一个*/
	BYTE m_byPhaseMinimumGap;		/*2.2.2.17可以递减到的最小gap，应该和phaseTimeToReduce综合使用*/
	BYTE m_byPhaseDynamicMaxLimit;	/*2.2.2.18运行MAX的限定值。当小于此值时，运行MAX1，反之运行MAX2*/
	BYTE m_byPhaseDynamicMaxStep;	/*2.2.2.19动态调整步长*/
	BYTE m_byPhaseStartup;			/*2.2.2.20相位初始化设置
									other(1)相位不使能（非定义）标志位（或者phaseOption的bit0=0或者phaseRing=0）
									phaseNotON(2)相位初始为红（非活动）
									greenWalk(3)相位初始为最小绿和行人时间
									greenNoWalk(4)相位初始为最小绿的开始
									yellowChange(5)相位初始为黄灯开始
									redClear(6)相位初始化为红灯开始*/
	WORD m_wPhaseOptions;			/*2.2.2.21相位选项
									Bit 0 - Enabled Phase相位使能
									Bit 1 – 当进入自动闪光操作时，设置为1时此相位进行闪光操作，闪光前先进行红灯操作
									Bit 2 – 当闪光结束时，设置为1时首先运行此相位。
									Bit 3 – 非感应1
									Bit 4 -  非感应2
									Bit 5 – 设置为0时，检测器请求从相位黄灯开始记录。设置为1时，检测器请求操作依赖于detectorOptions对象。
									Bit 6 - Min Vehicle Recall人为设置为最小绿请求
									Bit 7 - Max Vehicle Recall人为设置为最大绿请求
									Bit 8 - Ped Recall人为设置为行人请求
									Bit 9 - Soft Vehicle Recall：当本相位没有实际请求而所有的冲突相位的MAX RECALL时间用完也没有请求驻留在绿灯时，给本相位一个soft recall，使相位转换到本相位。
									Bit 10 - Dual Entry Phase：设置为1时使一个没有检测器请求的相位跟随另一个环中同时放行的相位一起放行。
									Bit 11 - Simultaneous Gap Disable：设置为1使不进行Gap操作
									Bit 12 - Guaranteed Passage：设置为1时保证感应相位最后安全绿（passage）
									Bit 13 - Actuated Rest In Walk：1代表感应相位在冲突方没有请求时保持行人放行
									Bit 14 - Conditional Service Enable：相位时间没用完，把剩余的时间给在同一个barrier的相位。
									Bit 15 - Added Initial Calculation：设置为1时，选择检测器的最大值，设置为0时，选择检测器的加和。*/
	BYTE m_byPhaseRing;				/*2.2.2.22用到该相位的ring表号*/
	BYTE m_byPhaseConcurrency[MAX_PHASE_CONCURRENCY_COUNT];		/*2.2.2.23可以并发的相位，每个字节代表一个可并发的相位号*/
	BYTE m_byRedYellow;				//红黄时间(BYTE)		
	BYTE m_byGreenFlash;				//绿闪时间(BYTE)		
	BYTE m_bySafeRed;				//安全红灯时间(BYTE)	
	BYTE m_byRedPulse;				//红脉冲时间
	BYTE m_byGreenPulse;			//绿脉冲时间
	WORD m_wVehicleQueueThresh;		//车辆排队阈值
	WORD m_wPedestrianWaitThresh;	//行人等待阈值
	BYTE m_byPulseType;				// 脉冲类型   0，关闭行人及机动车脉冲； 1，发送机动车脉冲；2，发送行人脉冲 ；3 ，发送行人及机动车脉冲
	BYTE m_byForbiddenFlag;         //相位禁止标志 -GB20999
	BYTE m_byScreenFlag;			//相位屏蔽标志 -GB20999
}TPhase, *PTPhase;


//跟随相位定义
typedef struct tagOverlapTable
{
	BYTE m_byOverlapNumber;			/*2.10.2.1 overlapNumber：overlap号，	不超过maxOverlaps。1 = Overlap A, 2 = Overlap B etc */
	BYTE m_byOverlapType;				/*2.10.2.2 overlap操作类型，枚举如下：
									other(1) ：未在此描述的操作类型。
									normal(2)：此种操作类型时，overlap的输出受overlapIncludedPhases参数控制。有下列情形时overlap输出绿灯：
									当overlap包含的相位是绿灯时。
									当overlap包含的相位是黄灯（或者全红red clearance）且overlap包含下一相位（included phase is next）时。
									如果overlap包含的相位是黄灯且overlap不包含下一相位（included phase is not next），overlap输出黄灯。如果overlap的绿灯和黄灯无效，将输出红灯。
									minusGreenYellow(3)：此种操作类型时，overlap的输出受overlapIncludedPhases和overlapModifierPhases参数控制。有下列情形时overlap输出绿灯：
									当overlap包含相位是绿灯且overlap的修正相位不是绿灯时（NOT green）
									当overlap包含的相位是黄灯（或者全红red clearance）且overlap包含下一相位（included phase is next）且overlap的修正相位不是绿灯时。
									如果overlap包含的相位是黄灯且overlap的修正相位不是黄灯且overlap不包含下一相位（included phase is not next），overlap输出黄灯。如果overlap的绿灯和黄灯无效，将输出红灯。*/
	BYTE m_byArrOverlapIncludedPhases[MAX_PHASE_COUNT_IN_OVERLAP];	/*2.10.2.3 overlap包含的相位，每字节表示一个相位号。*/
	BYTE m_byArrOverlapModifierPhases[MAX_PHASE_COUNT_MO_OVERLAP];	/*2.10.2.4 overlap的修正相位，每字节表示一个相位号。	如果为空值（null），overlapType为normal，如果为非空值（non-null），overlapType为minusGreenYellow。*/
	BYTE m_byOverlapTrailGreen;		/*2.10.2.5  0-255秒，	如果此参数大于0且overlap的绿灯正常结束，overlap绿灯将延长此参数设定的秒数。*/
	BYTE m_byOverlapTrailYellow;		/*2.10.2.6  3-25.5秒。如果overlap的绿灯被延长	（Trailing Green大于零），此参数将决定overlap Yellow Change的时间间隔长度。*/
	BYTE m_byOverlapTrailRed;			/*2.10.2.7  0-25.5秒。如果overlap的绿灯被延长	（Trailing Green），此参数将决定overlap Red Clearance的时间间隔长度。*/
	BYTE m_byPulseType;				// 脉冲类型   0，关闭行人及机动车脉冲； 1，发送机动车脉冲；2，发送行人脉冲 ；3 ，发送行人及机动车脉冲
}TOverlapTable, *PTOverlapTable;


//方案定义
typedef struct tagPattern
{
	BYTE m_byPatternNumber;						/*2.5.7.1 方案表中该行属于哪个方案*/
	WORD m_wPatternCycleTime;					/*2.5.7.2 方案周期长,一个无感应的行人相位最小需求包括：
													miniGreen＋Walk＋PedClear＋yellow＋Red
													一个感应的行人相位最小需求包括：miniGreen+Yellow+Red */
	WORD m_byPatternOffsetTime;					/*2.5.7.3 相位差大小*/
	BYTE m_byPatternSplitNumber;				/*2.5.7.4方案对应的绿信比表号*/
	BYTE m_byPatternSequenceNumber;				/*2.5.7.5方案对应的sequence表号*/
	char m_cPatternDesc[MAX_DESC_LENGTH];		/*描述*/
}TPattern, *PTPattern;


//绿信比定义
typedef struct tagSplit
{
	BYTE m_bySplitNumber;			/* 2.5.9.1定义绿信比组号，一个组内的该字段相同*/
	BYTE m_bySplitPhase;			/*2.5.9.2该行对应的相位号*/
	WORD m_wSplitTime;			/*2.5.9.3对应绿信比相位绿时间*/
	BYTE m_bySplitMode;			/*2.5.9.4对于该相位应如何操作
								    1---其他
  								    2---未定义，非绿信比控制模式
								    3---最小车辆响应：感应控制时，机动车相位被强制直行最小绿。此属性优先级高于相位参数中的“机动车自动请求”属性
									4---最大车辆响应：感应控制时，机动车相位被强制直行最大绿。此属性优先级高于相位参数中的“机动车自动请求”属性
									5---行人响应：感应控制时，行人相位被强制获取放行权。此属性优先级高于相位参数中的“行人自动请求”属性
									6---最大车辆/行人响应：感应控制时，机动车相位被强制直行最大绿，行人相位被强制获取放行权。此属性优先级高于相位参数中的“机动车自动请求”属性和“行人自动请求”属性
									7---忽略相位，该相位在绿信比模式下，从方案中去除。
									8---关断相位，该相位在绿性比模式下，一直处于红灯。
									*/
	BYTE m_bySplitCoordPhase;		/*2.5.9.5定义是否作为协调相位处理*/
}TSplit, *PTSplit;


//相序定义
typedef struct tagSequence
{
	BYTE m_bySequenceNumber;			/*2.8.3.1时序方案号*/
	BYTE m_bySequenceRingNumber;		/* 2.8.3.2时序方案对应的ring号*/
	BYTE m_bySequenceData[MAX_SEQUENCE_DATA_LENGTH];		/*2.8.3.3 最大支持的ring个数*/
}TSequence, *PTSequence;

//阶段链表定义
typedef struct tagStagesList
{
	WORD m_wSplitTimeInStage;			                       /*阶段里面的绿信比时间*/
	BYTE m_byPhaseInStage[MAX_RING_COUNT];		               /*阶段里面的相位信息*/
}TStagesList, *PTStagesList;

//阶段定义
typedef struct tagStages
{
	BYTE m_byPhaseInStage[MAX_RING_COUNT];		               /*阶段里面的相位信息*/
}TStages, *PTStages;

//屏障定义
typedef struct tagBarrier
{
	BYTE m_byBarrierNumber;			                           /*屏障编号*/
	BYTE m_byBarrierLength;		                               /*屏障时长*/
	BYTE m_byRingCount;                                        /*屏障里面的环数量*/
	BYTE m_byRingNumber[MAX_RING_COUNT];                       /*屏障里面的环信息*/
	BYTE m_byPhaseInBarrier[MAX_RING_COUNT][MAX_PHASE_COUNT];  /*屏障里面的环对应的相位信息*/
}TBarrier, *PTBarrier;

//时基参数定义
typedef struct tagTimeBaseVariable
{
	WORD m_wTimebaseAscPatternSync;	/*2.6.1在经过午夜很短时间内的方案同步参考。设置为0XFFFF时，信号控制单元将把ACTION TIME作为方案的同步参考。*/
	BYTE m_byTimebaseAscActionStatus;/*2.6.4 这个对象表明当前用到的时基表号。*/
	DWORD m_dwGlobalTime;				/*2.4.1 UTC（或GMT）时间，从1970/1/1 00:00:00至今的秒数*/
	BYTE m_byGlobalDaylightSaving;	/*2.4.2夏令时
									other (1)：DST定义机制没有在本标准中。
									disableDST (2)：不使用DST
									enableUSDST (3)：DST使用美国习惯
									enableEuropeDST (4)：DST使用欧洲习惯
									enableAustraliaDST (5)：DST使用澳大利亚习惯
									enableTasmaniaDST (6) DST使用塔斯马尼亚习惯*/
	BYTE m_byDayPlanStatus;			/*2．4．4．4表示活动时段表的编号。0表示没有活动的时段表*/
	int m_iGlobalLocalTimeDifferential;	/*2．4．5 时差*/
	int m_iGcontrollerStandardTimeZone;	/*2．4．6 本地标准时间与GMT的时差（秒）。正值表示本地时间在东半球，负值表示本地时间在西半球*/
	DWORD m_dwControllerLocalTime;	/*2．4．7 本地时间，等于1970/1/1 00:00:00以来的秒数*/
}TTimeBaseVariable, *PTTimeBaseVariable;


//动作定义
typedef struct tagTimeBaseAscAction
{
	BYTE m_byTimebaseAscActionNumber;	/* 2.6.3.1行号*/
	BYTE m_byTimebaseAscPattern;		/* 2.6.3.2感应方案号。这个参数不能超过maxPatterns, flash,或者free的值。设置为0表明没有方案被选择。*/
	BYTE m_byTimebaseAscAuxillaryFunction;/* 2.6.3.3相位表号*/
	BYTE m_byTimebaseAscSpecialFunction;/* 2.6.3.4通道表号*/
}TTimeBaseAscAction, *PTTimeBaseAscAction;


//调度计划定义
typedef struct tagTimeBaseSchedule
{
	WORD m_wTimeBaseScheduleNumber;	    /*2.4.3.2.1 调度计划号，由timeBaseScheduleMonth、timeBaseScheduleDate、timeBaseScheduleDate、timeBaseScheduleDayPlan四个参数共同决定计划是否可以执行。(1..40)*/
	WORD m_wTimeBaseScheduleMonth;		/*2.4.3.2.2 bit1-bit12，每位表示一个月。置1表示允许对应计划在该月执行。(0..65535)*/
	BYTE m_byTimeBaseScheduleDay;		/*2.4.3.2.3 bit1-bit7，每位表示一周中的一天。置1表示允许对应计划在该天执行。(0..255)*/
	DWORD m_dwTimeBaseScheduleDate;		/* 2.4.3.2.4 bit1-bit31，每位表示一月中的一天。置1表示允许对应计划在该天执行。(0..4294967295)*/
	BYTE m_byTimeBaseScheduleDayPlan;	/*2.4.3.2.5时段表号，指向timeBaseScheduleDayPlan。0表示本行无效。(0..255)*/
	char m_cTimeBaseScheduleDesc[MAX_DESC_LENGTH];		/*描述*/
}TTimeBaseSchedule, *PTTimeBaseSchedule;


//时段定义
typedef struct tagTimeBaseDayPlan
	/*表的总行数等于maxDayPlans与maxDayPlanEvents object的乘积。Day Plan Number和Day Plan Event Number都从1开始计数。
	在信号机中，action就是信号控制方案pattern。当action与上位系统或其他命令发生冲突时，信号机应该根据预先定义的优先级选择执行哪个命令。只有DayPlan有效且处于活动时间，并且action的优先级高于设备当前操作的优先级时，时段表定义的action才可以被执行。
	如果两个action在同一时间执行，则执行结果是设备相关的。当重新启动或改变globalTime时，应该优先使用前24小时以内的上一个被调度执行的event，而不是使用当前时间的event。*/
{
	BYTE m_byDayPlanNumber;			/*时段表号(1..16)，索引*/
	BYTE m_byDayPlanEventNumber;		/*时段（事件）号。(1..48)，索引
										几个不同的事件可以在一天中的不同时段执行。如果两个事件出现的时段相同，则时段号小的先执行*/
	BYTE m_byDayPlanHour;			    /*开始执行时刻的整点数，用时间（24时制）。*/
	BYTE m_byDayPlanMinute;			/*开始执行的整分数*/
	BYTE m_byDayPlanControl;       /*      0 – 自主控制
      									1 – 黄闪
 										2 – 全红
 										3 – 关灯
										4 – 步进
										5  - 定周期控制		
										6 – 单点感应控制
										7 – 协调感应控制 		
										8 – 方案选择控制
										9 – 自适应控制
										10 – 无电缆控制
										11 – 有电缆控制（上位机控制）
										12  - 行人过街*/
	BYTE m_byDayPlanActionNumberOID;      /*配时方案号*/
	char m_cDayPlanDesc[MAX_DESC_LENGTH]; /*描述*/
	BYTE m_byCoorDination;				//协调计划标志 0：不是协调计划 1：是协调计划
}TTimeBaseDayPlan, *PTTimeBaseDayPlan;


//车辆检测器定义
typedef struct tagVehicleDetector
{
	BYTE m_byVehicleDetectorNumber;     /*2.3.2.1车辆检测器序列号。(1..48)*/
	BYTE m_byVehicleDetectorOptions;	/*2.3.2.2车辆检测器选项
			Bit 0 – 流量检测器
			Bit 1 – 占有率检测器
			Bit 2 -  相连相位在黄灯时间纪录车辆数
			Bit 3 -  相连相位在红灯时间纪录车辆数
			Bit 4 –感应相位增加单位延长绿（passage）
			Bit 5 –公交优先检测器
			Bit 6 – 排队检测器
			Bit 7 – 请求检测器*/
	BYTE m_byVehicleDetectorCallPhase;/*2.3.2.3该检测器对应的请求相位*/
	BYTE m_byVehicleDetectorSwitchPhase;/*2.3.2.4是个相位号,该相位可接收该车辆检测器的请求，当assigned phase 为红灯或黄灯时并且the Switch Phase时绿灯时，该相位被转换*/
	WORD m_wVehicleDetectorDelay;	/*2.3.2.5当assigned phase不是绿灯时，检测器的输入将被延迟一段时间（00-999秒）。一旦发生了延迟，那麽之后发生的延迟将被累计*/
	BYTE m_byVehicleDetectorExtend;/*2.3.2.6当assigned phase是绿灯时，检测器的每次输入，相位都将在绿灯的终止点被延长一段时间（00-999秒）*/
	BYTE m_byVehicleDetectorQueueLimit;/*2.3.2.7检测器排队长度限制，当超过这一限制时，到达的车辆不再有效*/
	BYTE m_byVehicleDetectorNoActivity;/*2.3.2.8 0-255分钟，在这段指定的时间中检测器没有发出感应信息则被判断为一个错误，如果值为0，则不会被判断*/
	BYTE m_byVehicleDetectorMaxPresence;/*2.3.2.9 0-255分钟，在这段时间内，检测器持续发出感应信息，则被判断为一个错误，如果值为0，则不会被判断*/
	BYTE m_byVehicleDetectorErraticCounts;	/*2.3.2.10每分钟0-255次，如果检测器发出的感应信息的频率超过这个值，则被判断为一个错误，如果值为0，则不会被判断*/
	BYTE m_byVehicleDetectorFailTime;/*2.3.2.11 检测器失败时间，单位：秒*/
	BYTE m_byVehicleDetectorAlarms;	/*2.3.2.12 检测器告警
					Bit 7: Other Fault – 其他故障
					Bit 6: Reserved.
					Bit 5: Reserved.
					Bit 4: Configuration Fault – 配置的检测器没有使用或跟一个不存在的相位联系
					Bit 3: Communications Fault –检测器通信错误
					Bit 2: Erratic Output Fault – 检测器计数错误(过快或过慢)
					Bit 1: Max Presence Fault – 检测器一直有车
					Bit 0：No Activity Fault - 检测器一直没车*/
	BYTE m_byVehicleDetectorReportedAlarms; /*2.3.2.12检测器报告
					Bit 7: Reserved.
					Bit 6: Reserved.
					Bit 5: Reserved.
					Bit 4: Excessive Change Fault - 检测器计数过多。
					Bit 3: Shorted Loop Fault - 检测器闭环。
					Bit 2: Open Loop Fault – 检测器开环
					Bit 1: Watchdog Fault -  watchdog 错误
					Bit 0: Other – 其他错误*/
	BYTE m_byVehicleDetectorReset;	/*2.3.2.13 当该对象被设置为非零时，将引起cu命令检测器重启。在cu发出重启命令之后该对象被自动设置为0*/

	TVehDetCtrParam m_stVehDetCtrParam;  //车检板控制参数
	BYTE m_byDetectorType;//检测器类型
	WORD m_wSaturatedFlow;//检测器饱和流量
	
}TVehicleDetector, *PTVehicleDetector;


//行人检测器定义
typedef struct tagPedestrianDetector
{
	BYTE m_byPedestrianDetectorNumber;		/*2.3.7.1 行人检测器行号*/
	BYTE m_byDetectorType;					/*0线圈,1雷达,2视频*/
	BYTE m_byPedestrianDetectorCallPhase;	/*2.3.7.2 行人检测器对应的请求相位*/
	BYTE m_byPedestrianDetectorNoActivity;	/*2.3.7.3 0-255分钟，在这段指定的时间中行人检测器没有发出感应信息则被判断为一个错误，如果值为0，则不会被判断*/
	BYTE m_byPedestrianDetectorMaxPresence;	/*2.3.7.4  0-255分钟，在这段时间内，行人检测器持续发出感应信息，则被判断为一个错误，如果值为0，则不会被判断*/
	BYTE m_byPedestrianDetectorErraticCounts;/*2.3.7.5 每分钟0-255次，如果行人检测器发出的感应信息的频率超过这个值，则被判断为一个错误，如果值为0，则不会被判断*/
	//BYTE  byPedestrianDetectorAlarms;		/*2.3.7.6 行人检测器警告信息，
	//		Bit 7: Other Fault – 其他错误
	//		Bit 6: Reserved.
	//		Bit 5: Reserved.
	//		Bit 4: Configuration Fault – 配置错误
	//		Bit 3: Communications Fault – 通信错误
	//		Bit 2: Erratic Output Fault – 计数过多
	//		Bit 1: Max Presence Fault – 长期又车
	//		Bit 0: No Activity Fault – 长期没车*/
	BYTE m_byDetectorRegion;	/*0默认,1等待区域 ,2过街区域*/
}TPedestrianDetector, *PTPedestrianDetector;


//单元参数定义
typedef struct tagGeneralParam
{
	BYTE m_byUnitStartUpFlash;					// 2.4.1 启动时的闪光控制时间(秒)。启动时的闪光控制在掉电恢复后出现。掉电恢复具体包括哪些情况由设备定义。在这期间，硬件黄闪和信号灯监视是不活动的（如果有的话）。
	BYTE m_byUnitAutoPedestrianClear;	       // 2.4.2行人自动清空参数（1 = False/Disable 2=True/Enable）。	当设置为1并且手动操作有效时，信号机便放行行人自动清空时间，以防止行人清空时间被预先设置的时间终止。
	WORD m_wUnitBackupTime;						// 2.4.3信号机离线后到降级前的时间。
	BYTE m_byUnitRedRevert;						// 2.4.4最小红灯时间。此参数为所有的相位提供最小红灯时间（如：如果此值大于一个相位的红灯时间，则这个相位的红灯时间用这个参数来代替）。这个对象为黄灯之后和下一个绿灯之前提供这段时间提供一个最小指示。
	BYTE m_byUnitControl;							// .10 允许远程控制实体激活信号机的某些功能( 0 = False / Disabled, 1 = True / Enabled)：
	//Bit 7: 灰度使能。设置为1时，表示进行通道灰度操作。为了实现这个功能，timebaseAscAuxillaryFunction参数必须设置为true。
	//Bit 6: 有缆协调 - 设置为1时，表示作为有缆协调的发起机。
	//Bit 5：Walk Rest Modifier - 当设置为1时，如果冲突相位没有服务请求，则感应相位停留在放行状态
	//Bit 4：Call to Non-Actuated 2 - 当设置为1时，使在phaseOptions字段中设置Non-Actuated 1的相位运行在非感应状态。
	//Bit3: Call to Non-Actuated 1 - 当设置为1时，使在phaseOptions字段中设置Non-Actuated 2的相位运行在非感应状态。
	//Bit2:External Minimum Recall -当设置为1时，使所有相位运行在最小请求状态
	//Bit 1～0: Reserved。
	BYTE m_byCoordOperationalMode; // 2.5.1协调的操作方式Automatic（0）：自动方式，可以为协调，感应，闪光等可以
	//ManualPattern（1~253）：运行手动设定的方案
	//ManualFree（254）：无协调自动感应
	//ManualFlash(255)：无协调自动闪光
	BYTE m_byCoordCorrectionMode;// 2.5.2协调方式
	//other(1)协调建立在一个没有在本对象中定义的新的相位差
	//dwell(2)协调通过驻留协调相位立即变化达到相位差
	//shortway(3)协调通过某种限制周期变化的来减少和增加时间达到相位差，即平滑过渡
	//addOnly(4)协调通过某种限制周期变化的习惯来增加时间来达到相位差
	BYTE m_byCoordMaximumMode;	// 2.5.3 协调的最大的方式
	//other(1)：不在此所定义的未知方式
	//maximum1(2)：Max1有效的协调
	//maximum2(3)：Max2有效的协调
	//maxinhibit(4)：当运行协调模式时，禁止运行最大绿
	BYTE m_byCoordForceMode;		// 2.5.4 Pattern强制模式
	//other(1)：信号机使用在此没有定义的模式
	//floating(2)：每个相位激活后强制到绿灯时间，允许不用的相位时间转化到协调相位
	//fixed(3)：每个相位被强制到周期固定位置，不用的相位时间加到接下来的相位中

	BYTE m_byFlashFrequency;			//闪光频率(BYTE)			
	BYTE m_byThroughStreetTimeGap;		//二次过街时差(BYTE)
	BYTE m_byFluxCollectCycle;			//流量采集周期(BYTE)
	BYTE m_bySecondTimeDiff;			//二次过街逆向协调参数
	BYTE m_byStartAllRedTime;			//启动全红时间
	BYTE m_byCollectCycleUnit;			//流量采集单位，秒 / 分钟 ( 0/1)
	BYTE m_byUseStartOrder;			//启用启动灯序
	BYTE m_byCommOutTime;				//通信超时时间
	WORD m_wSpeedCoef;					//速度计算因子
	//自定义部分                         
	WORD m_wDelayTime;       //行人放行延迟起效时间，收到请求后，延迟这个值，然后开始放行，单位为秒
	WORD m_wWaitTime;		   //无请求强制放行时间，一直没有行人请求，最大这么长时间就会强制放行一次，单位为分；0表示没请求就不放行
	WORD m_wPassGap;			//按钮两次触发最小时间间隔，每次放行之后，再次处理行人请求的最短等待时间，单位为秒
	unsigned char m_acGatwayIp[7];     //预留

	BYTE m_byTransCycle;				//平滑过渡周期
	BYTE m_byOption;		    //选项参数，按位取值
	//BIT 7---------高压不黄闪
	//BIT 6---------保留
	//BIT 5---------保留
	//BIT 4---------保留
	//BIT 3---------保留
	//BIT 2---------第一周期启用
	//BIT 1---------大灯启动序列
	//BIT 0---------启用板密码
	BYTE m_byUnitTransIntensityCalCo; //交通强度计算系数
}TGeneralParam, *PTGeneralParam;


//协调参数定义
typedef struct tagCoordinationVariable
{
	BYTE m_byCoordOperationalMode;		/*2.5.1协调的操作方式
					Automatic（0）：自动方式，可以为协调，感应，闪光等可以
					ManualPattern（1~253）：运行手动设定的方案
					ManualFree（254）：无协调自动感应
					ManualFlash(255)：无协调自动闪光*/
	BYTE m_byCoordCorrectionMode;	/*2.5.2协调方式
					other(1)协调建立在一个没有在本对象中定义的新的相位差
					dwell(2)协调通过驻留协调相位立即变化达到相位差
					shortway(3)协调通过某种限制周期变化的来减少和增加时间达到相位差，即平滑过渡
					addOnly(4)协调通过某种限制周期变化的习惯来增加时间来达到相位差*/
	BYTE m_byCoordMaximumMode;		/*2.5.3 协调的最大的方式
					other(1)：不在此所定义的未知方式
					maximum1(2)：Max1有效的协调
					maximum2(3)：Max2有效的协调
					maxinhibit(4)：当运行协调模式时，禁止运行最大绿*/
	BYTE m_byCoordForceMode;			/*2.5.4 Pattern强制模式
					other(1)：信号机使用在此没有定义的模式
					floating(2)：每个相位激活后强制到绿灯时间，允许不用的相位时间转化到协调相位
					fixed(3)：每个相位被强制到周期固定位置，不用的相位时间加到接下来的相位中*/
	BYTE m_byPatternTableType;		/*2.5.6定义方案表中需要的特殊组织结构
					other(1)：此处没有定义的
					patterns(2)：方案表的每一行代表唯一的一个方案，而且不依赖其他行
					offset3(3)：每个方案有3个相位差，占方案表的3行
					offset5(4)：每个方案有5个相位差，占5行*/
	BYTE m_byCoordPatternStatus;		/*2.5.10 协调方案状态
				Not used（0）
				Pattern -（1-253）当前运行方案。
				Free - (254)感应
				Flash - (255)闪光*/
	BYTE m_byLocalFreeStatus;		/*2.5.11 Free控制状态
				other: 其他状态
				notFree: 没有进行free控制
				commandFree: 
				transitionFree: 过渡free即将进入协调
				inputFree: 信号机输入导致free而不响应协调
				coordFree: the CU programming for the called pattern is to run Free.
				badPlan: 调用的方案不合法所以进行free
				badCycleTime: 周期不合法（不满足最小需求）所以进行free
				splitOverrun: 时间越界free
				invalidOffset: 保留或不用
				failed: 周期诊断导致free*/
	WORD m_wCoordCycleStatus;		/* 2.5.12 协调方案的周期状态（0-510sec），从周期长一直减少到0*/
	WORD m_wCoordSyncStatus;			/*2.5.13协调同步状态（0－510）从协调基准点到目前周期的时间，从0记录到下个周期基准点。可以超过周期长*/
	BYTE m_bySystemPatternControl;	/*2.5.14系统方案控制
				Standby(0)系统放弃控制
				Pattern(1-253)系统控制方案号
				Free(254)call free 
				Flash(255)自动Flash */
	BYTE m_bySystemSyncControl;		/*2.5.14 建立系统同步基准点*/
}TCoordinationVariable, *PTCoordinationVariable;


//通道定义
typedef struct tagChannel
{
	BYTE m_byChannelNumber;		/*2.9.2.1通道号，不能大于maxChannels。(1..32)*/
	BYTE m_byChannelControlSource;	/*通道控制源[相位(phase)或者重叠(overlap)]，
				由channelControlType决定是相位或是重叠，不能大于最大相位数和最大重叠数。*/
	BYTE m_byChannelControlType;	/*2.9.2.2通道控制类型，
																			other(1), 其他
																			phaseVehicle (2), 机动车相位
																			phasePedestrian (3), 行人相位
																			overlap (4)，跟随相位
																			overlapPedestrian(5)跟随行人
																			Lane lamp(5), 可变车道灯*/

	BYTE m_byChannelFlash;			/*2.9.2.3自动闪光状态。
			Bit 7: Reserved
			Bit 6: Reserved
			Bit 5: Reserved
			Bit 4: Reserved
			Bit 3: 交替闪光
				Bit=0: Off/Disabled & Bit=1: On/Enabled
			Bit 2: 红闪
				Bit=0: Off/Red Dark & Bit=1: On/Flash Red
			Bit 1: 黄闪
				Bit=0: Off/Yellow Dark & Bit=1: On/Flash Yellow
			Bit 0: Reserved
				Bit 1 和 Bit 2 同时为1的效果是Bit 1 = 0，Bit 2 = 1，Reserved位必须为0，否则返回badValue(3)错误。*/
	BYTE m_byChannelDim;		/*2.9.2.4通道灰度状态
			Bit 7: Reserved
			Bit 6: Reserved
			Bit 5: Reserved
			Bit 4: Reserved
			Bit 3: Dim Alternate Half Line Cycle
				Bit=0: Off/+ half cycle &
				Bit=1: On/- half cycle
			Bit 2: Dim Red
				Bit=0: Off/Red Not Dimmed &
				Bit=1: On/Dimmed Red
			Bit 1: Dim Yellow
				Bit=0: Off / Yellow Not Dimmed &
				Bit=1: On / Dimmed Yellow
			Bit 0: Dim Green
				Bit=0: Off / Green Not Dimmed &
				Bit=1: On / Dimmed Green*/

	TLampBoardCtrParam m_stLampBoardCtrParam;   //灯控板控制参数
}TChannel, *PTChannel;


//优先配置定义
typedef struct tagPreempt
{
	BYTE m_byPreemptID;        //优先检测号
	BYTE m_byPreemptLevel;     //优先级 1-5，等级越高，优先级越高
	BYTE m_byPreemptType;      /*优先类型 0：常规优先，收到优先请求时，如果优先相位非当前相位，其他相位按相序执行最小绿
										  1：紧急优先，收到优先请求时，如果优先相位非当前相位，直接跳过其他相位*/
	WORD m_wPreemptDelay;      //延迟时间-代表从检查器位置到路口需要行驶的自由流时间
	WORD m_wPreemptDuration;   //持续时间-代表优先车辆通过路口所需的最大时间
	BYTE m_byPreemptPhaseID;   //优先相位
	BYTE m_byPreemptOmit;      //屏蔽 0：屏蔽 1：开启
	BYTE m_byPreemptSource;	   //检测器信号源

	//BYTE m_byPreemptNumber;
	//BYTE m_byPreemptControl;
	//BYTE m_byPreemptLink;
	//WORD m_wPreemptDelay;
	//WORD m_wPreemptMinimumDuration;
	//BYTE m_byPreemptMinimumGreen;
	//BYTE m_byPreemptMinimumWalk;
	//BYTE m_byPreemptEnterPedClear;
	//BYTE m_byPreemptTrackGreen;
	//BYTE m_byPreemptDwellGreen;
	//WORD m_wPreemptMaximumPresence;
	//BYTE m_abyPreemptTrackPhase[MAX_PHASE_COUNT];
	//BYTE m_byPreemptTrackPhaseLen;
	//BYTE m_abyPreemptDwellPhase[MAX_PHASE_COUNT];
	//BYTE m_byPreemptDwellPhaseLen;
	//BYTE m_abyPreemptDwellPed[MAX_PEDESTRIANPHASE_COUNT];
	//BYTE m_byPreemptDwellPedLen;
	//BYTE m_abyPreemptExitPhase[MAX_PHASE_COUNT];
	//BYTE m_byPreemptExitPhaseLen;
	//BYTE m_byPreemptState;
	//BYTE m_abyPreemptTrackOverlap[MAX_OVERLAP_COUNT];
	//BYTE m_byPreemptTrackOverlapLen;
	//BYTE m_abyPreemptDwellOverlap[MAX_OVERLAP_COUNT];
	//BYTE m_byPreemptDwellOverlapLen;
	//BYTE m_abyPreemptCyclingPhase[MAX_PHASE_COUNT];
	//BYTE m_byPreemptCyclingPhaseLen;
	//BYTE m_abyPreemptCyclingPed[MAX_PEDESTRIANPHASE_COUNT];
	//BYTE m_byPreemptCyclingPedLen;
	//BYTE m_abyPreemptCyclingOverlap[MAX_OVERLAP_COUNT];
	//BYTE m_byPreemptCyclingOverlapLen;
	//BYTE m_byPreemptEnterYellowChange;
	//BYTE m_byPreemptEnterRedClear;
	//BYTE m_byPreemptTrackYellowChange;
	//BYTE m_byPreemptTrackRedClear;
}TPreempt, *PTPreempt;


typedef struct tagAddress
{
    WORD m_wSerialAddr;        //串口地址
    BYTE m_abyIpAddr[4];       //IP地址
    DWORD m_dwSubNetMask;        //子网掩码
    BYTE m_abyGatwayIp[4];     //网关
    BYTE m_byPort;            //端口号
    BYTE m_abyucMacAddr[6];    //MAC地址
    BYTE m_byreserved1;	    //以下为保留字段
    BYTE m_byreserved2;
    BYTE m_byreserved3;
    BYTE m_byreserved4;
    BYTE m_byreserved5;
    BYTE m_byreserved6;
    BYTE m_byreserved7;
    BYTE m_byreserved8;
}TAddress, *PTAddress;


typedef struct tagUnitParamEx
{
	BYTE m_byReserved1;		//交通强度计算系数
	BYTE m_byReserved2;
	BYTE m_byReserved3;
	BYTE m_byReserved4;
	BYTE m_byReserved5;
	BYTE m_byReserved6;
	BYTE m_byReserved7;
	BYTE m_byReserved8;
	BYTE m_byReserved9;
	BYTE m_byReserved10;
	BYTE m_byReserved11;
	BYTE m_byReserved12;
	BYTE m_byReserved13;
	BYTE m_byReserved14;
}TUnitParamEX, *PTUnitParamEX;


typedef struct tagExtraParamCountDown
{
	int m_iCountDownMode;					// 倒计时模式
										//	0:自学习(缺省)
										//	1:脉冲全程倒计时模式
										//	2:脉冲半程倒计时模式
										//	485协议
										//	3: 国家标准
										//      4:莱斯标准
										//      5:海信标准
	int m_iFreeGreenTime;                     //感应检测时间，缺省值为3
	int m_iPulseGreenTime;				//脉冲绿灯倒计时时间
	int m_iPulseRedTime;				       //脉冲红灯倒计时时间
	TPhase m_iPhaseOfChannel[32];                    //通道对应的相位(目前最大支持32个通道)
	int m_iOption;								//bit[0]:代表黄灯时是否闪烁
	int m_iRedFlashTime;						//红灯倒计时闪烁时间
}TExtraParamCountDown, *PTExtraParamCountDown;

//启动时序参数
typedef struct tagAscStartSequenceInfo
{
	BYTE m_byStartYellowFlash;   //启动黄闪
	BYTE m_byStartAllRed;        //启动全红
	BYTE m_byGreenWaveCycle;     //启动绿波过渡周期
}TAscStartSequenceInfo, *PTAscStartSequenceInfo;

//通道可检测
typedef struct tagAscChannelVerifyInfo
{
	int m_byControl;											//控制方式
	char m_achLampClr[C_N_MAX_LAMP_OUTPUT_NUM];                  //灯控板灯输出端子状态信息数组
}TAscChannelVerifyInfo, *PTAscChannelVerifyInfo;

//获取U盘流量数据时平台指令
typedef struct tagAscGainTrafficFlowCmd
{
	int m_SetUDiskStatus;                
	int m_GainTrafficFlowStatus;
}TAscGainTafficFlowCmd, *PTAscGainTafficFlowCmd;

//方案干预信息
typedef struct tagInterruptPatternInfo
{
	int m_nDurationTime;					    //持续时长
	int m_nDelayTime;						    //延迟时间
	int m_nOffset;
	int m_nCycleTime;
	int m_nRingNum;
	TSplit InterruptSplit[MAX_PHASE_COUNT];     //干预的绿信比信息
	TSequence InterruptSequence[MAX_RING_COUNT]; //干预的时序信息
}TInterruptPatternInfo, *PTInterruptPatternInfo;

//自定义对应国标的工作方式
typedef struct tagWorkModeParam
{
	BYTE m_byControlMode;
	BYTE m_byControlType;
	WORD m_wControlNumber;
	WORD m_wControlValue;
	int  m_nDelay;                                                      //延迟时间，单位秒
	int  m_nDuration;                                                   //持续时间，单位秒
	TInterruptPatternInfo    m_atManualControlPattern;                  //手动控制方案             
}TWorkModeParam, *PTWorkModeParam;

typedef struct tagChannelStatusInfo
{
	BYTE m_byChannelID;
	float m_fRedResVolt;				//该通道红灯残留电压
	float m_fRedOutputVolt;				//该通道红灯输出电压
	float m_fRedOffResPower;			//该通道红灯关闭后残留功率
	float m_fRedOnOutputPower;			//该通道红灯亮灯输出功率

	float m_fYellowResVolt;				//该通道红灯残留电压
	float m_fYellowOutputVolt;			//该通道红灯输出电压
	float m_fYellowOffResPower;		    //该通道红灯关闭后残留功率
	float m_fYellowOnOutputPower;		//该通道红灯亮灯输出功率

	float m_fGreenResVolt;				//该通道红灯残留电压
	float m_fGreenOutputVolt;			//该通道红灯输出电压
	float m_fGreenOffResPower;			//该通道红灯关闭后残留功率
	float m_fGreenOnOutputPower;		//该通道红灯亮灯输出功率

	int   m_nInVolt;					//通道输入电压

}TChannelStatusInfo, *PTChannelStatusInfo;

//通道绿冲突配置表
typedef struct tagAscChannelGreenConflictInfo
{
	BYTE m_byChannelNum;										//通道号
	BYTE m_byGreenConflict[MAX_CHANNEL_COUNT];					//通道的绿冲突
}TAscChannelGreenConflictInfo, * PTAscChannelGreenConflictInfo;

typedef struct tagAscParam
{
	TAscStepCfg m_stAscStepCfg;

	TAscAddressCode m_stAscAddressCode;															//信号机识别码

	TAscNetCard m_stAscNetCardTable[MAX_NETCARD_TABLE_COUNT];									//信号机两个网卡配置信息，0，网卡1. 1网卡2

	TAscTimeZoneInfo m_stAscTimeZoneInfo;														//信号机时区设置信息（含时区、令时等信息）
	
	TAscCenter m_stCenterVariable;																//平台中心ip和端口配置

	TAscManualPanel m_stAscManualPanel;															//手动面板配置

	TAscArea m_stAscArea;																		//路口和区域号

	TAscFaultCfg m_stAscFaultCfg;

	int m_stSingleOptimTableValidSize;
	TAscSingleOptim m_stAscSingleOptimTable[MAX_PATTERN_COUNT];									//单点自适应控制参数
	
	int m_stPhaseTableValidSize;																//相位表有效表长度
	TPhase m_stAscPhaseTable[MAX_PHASE_COUNT];													//相位表	

	int m_stVehicleDetectorTableValidSize;														//车辆检测器表有效表长度
	TVehicleDetector m_stAscVehicleDetectorTable[MAX_VEHICLEDETECTOR_COUNT];					//车辆检测器表

	int m_stPedestrianDetectorTableValidSize;													//行人检测器有效表长度
	TPedestrianDetector m_stAscPedestrianDetectorTable[MAX_PEDESTRIANDETECTOR_COUNT];

	TGeneralParam m_stAscUnitVariable;															//信号机单元级参数	

	TCoordinationVariable m_stAscCoordinationVariable;											//协调参数

	int m_stPatternTableValidSize;
	TPattern m_stAscPatternTable[MAX_PATTERN_COUNT];											//方案表

	int m_stSplitTableValidHeight;																//绿信比表有效高度
	int m_stSplitTableValidWidth[MAX_SPLIT_COUNT];												//绿信比表每行有效宽度
	TSplit m_stAscSplitTable[MAX_SPLIT_COUNT][MAX_PHASE_COUNT];									//绿信比表	

	TTimeBaseVariable m_stAscTimeBaseVariable;													//时基参数
	
	int m_stTimeActionTableValidSize;
	TTimeBaseAscAction m_stAscTimeBaseActionTable[MAX_TIMEBASE_ACTION_COUNT];					//动作表

	int m_stScheduleTableValidSize;
	TTimeBaseSchedule m_stAscScheduleTable[MAX_SCHEDULE_COUNT];									//调度计划表

	int m_stDayPlanTableValidHeight;															//二维表有效高度
	int m_stDayPlanTableValidWidth[MAX_DAYPLAN_TABLE_COUNT];									//时段表每行有效数据长度
	TTimeBaseDayPlan m_stAscDayPlanTable[MAX_DAYPLAN_TABLE_COUNT][MAX_SINGLE_DAYPLAN_COUNT];	//时段表

	int m_stSequenceTableValidHeight;															//时序表有效高度
	int m_stSequenceTableValidWidth[MAX_SEQUENCE_TABLE_COUNT];
	TSequence m_stAscSequenceTable[MAX_SEQUENCE_TABLE_COUNT][MAX_RING_COUNT];					//时序表	

	int m_stChannelTableValidSize;
	TChannel m_stAscChannelTable[MAX_CHANNEL_COUNT];											//通道表	

	int m_stOverlapTableValidSize;
	TOverlapTable m_stAscOverlapTable[MAX_OVERLAP_COUNT];										//跟随相位表
	
	int m_stPreemptTableValidSize;
	TPreempt m_stAscPreemptTable[MAX_PREEMPT_COUNT];											//优先表

	TUnitParamEX m_stAscUnitParamEx;															//增加的单元参数内容

	TExtraParamCountDown m_stAscCountDownParam;													//倒计时牌

	TAscCasCadeInfo m_stAscCasCadeInfo;															//级联参数

	int m_stChannelLockTableValidSize;
	TAscOnePlanChannelLockInfo m_stAscChannelLockInfo[MAX_SINGLE_DAYPLAN_COUNT];				// 通道锁定信息

	TAscStartSequenceInfo m_stAscStartSequenceInfo;												//启动时序信息

	int m_stStagesListTableValidHeight;															
	int m_stStagesListTableValidWidth[MAX_PATTERN_COUNT];
	TStagesList m_stAscStagesList[MAX_PATTERN_COUNT][MAX_STAGE_COUNT];                          //阶段链表信息

	int m_stStagesTableValidHeight;															
	int m_stStagesTableValidWidth[MAX_PATTERN_COUNT];
	TStages m_stAscStages[MAX_PATTERN_COUNT][MAX_STAGE_COUNT];                                  //阶段表信息

	int m_stBarrierTableValidHeight;															
	int m_stBarrierTableValidWidth[MAX_PATTERN_COUNT];
	TBarrier m_stAscBarrier[MAX_PATTERN_COUNT][MAX_SEQUENCE_TABLE_COUNT];                       //屏障信息

	int m_stChannelConflictValidSize;
	TAscChannelGreenConflictInfo m_stAscChannelConflictTable[MAX_CHANNEL_COUNT];				//通道绿冲突表

	char m_stChannelGreenConflictInfo[MAX_CHANNEL_COUNT][MAX_CHANNEL_COUNT];						//通道绿冲突信息表,0表示不能同时亮绿灯,1表示可以同时亮绿灯
}TAscParam, *PTAscParam;


typedef struct tagPhaseInfo
{
	int nPhaseID;											//相位id
	int nPhaseSplitTime;									//相位对应的绿信比值
	int nPhaseSplitMode;                  					//绿信比模式
}TPhaseInfo, * PTPhaseInfo;

typedef struct tagOneStageInfo
{
	int  nConcurrencyPhaseCount;
	int  nConcurrencyPhase[MAX_RING_COUNT];
}TOneStageInfo, * PTOneStageInfo;

typedef struct tagPatternStageInfo
{
	int  nStageCount;
	TOneStageInfo  atPhaseRunStageInfo[MAX_STAGE_COUNT];
}TPatternStageInfo, * PTPatternStageInfo;

typedef struct tagRingInfo
{
	int nPhaseCount;										//环内相位数量
	TPhaseInfo atPhaseInfo[MAX_PHASE_COUNT];				//环内相位信息表
}TRingInfo, * PTRingInfo;

//方案运行信息
typedef struct tagPatternInfo
{
	int nRingCount;											//一个方案中环数量
	TRingInfo atRingInfo[MAX_RING_COUNT];					//方案中所有环的环内相位信息
}TPatternInfo, * PTPatternInfo;

typedef struct tagErrInfo
{
	int nCheckErr;
	int nSubCheckErr;
	int nSubTwoCheckErr;
}TErrInfo, * PTErrInfo;
//#pragma pack(pop)//恢复对齐状态
#endif 
