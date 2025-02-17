/*=====================================================================
模块名 ：逻辑控制管理模块
文件名 ：OpenATCLogicCtlManager.h
相关文件：OpenATCParameter.h OpenATCRunStatus.h
实现功能：逻辑控制模块整体调度类，用于启动时序控制，根据特征参数进行控制。
作者 ：刘黎明
版权 ：<Copyright(c) 2019-2020 Suzhou Keda Technology Co.,Ltd. All right reserved.>
------------------------------------------------------------------------
修改记录：
日 期           版本     修改人     走读人     修改记录
2019/9/26       V1.0     刘黎明     王 五      创建模块
=====================================================================*/

#ifndef OPENATCLOGICCTLMANAGER_H
#define OPENATCLOGICCTLMANAGER_H

#include "../Include/OpenATCParameter.h"
#include "../Include/OpenATCRunStatus.h"
#include "../Include/OpenATCRunStatusDefine.h"
#include "../Include/OpenATCLog.h"
#include "./logicctl/LogicCtlMode.h"
#include "OpenATCOperationRecord.h"

/*=====================================================================
类名 ：COpenATCLogicCtlManager
功能 ：逻辑控制调度类，用于启动时序控制，根据特征参数进行控制
主要接口：
备注 ：
-----------------------------------------------------------------------
修改记录：
日 期          版本     修改人     走读人       修改记录
2019/09/14     V1.0     刘黎明     李四         创建类
=====================================================================*/
class COpenATCLogicCtlManager  
{
public:
    //类定义为单件
    static COpenATCLogicCtlManager * getInstance();

    //类的初始化操作
    void Init(COpenATCParameter * pParameter,COpenATCRunStatus * pRunStatus, COpenATCLog * pOpenATCLog);

    //类的主流程
    void Work();

    //类的停止与释放
    void Stop();

private:
	COpenATCLogicCtlManager();
	~COpenATCLogicCtlManager();

    typedef enum tagStartUpSequenceTimeVal
    {
        TIME_STARTUP_FLASH = 10,
        TIME_STARTUP_RED = 5,
    }TStartUpSequenceTimeVal;

    //启动时序控制
    void StartUpTimeSequenceCtl();

    //启动黄闪                  
    void StartUpFlashCtl(); 

    //启动全红                        
    void StartUpAllRedCtl();  

    //启动时序结束后的控制
    void AfterStartUpTimeSequenceCtl();

    //准备当前时间段的特征参数                
    void PrepareParam();    

	//根据用户干预的控制方式和方案号准备参数
    void PrepareParamForSystemAsk(int nCtlMode,int nPlanNo); 

    //分析当前故障等级，做出对应的处理
    int ProcFault();  

    //计算运行时间
    void ProcLampClrRunCounter();  

	//特征参数发生变化时调用,用于判断灯控板使用是否发生变化
    bool IsLampCtlBoardChg();   

    //严重故障运行
    void CriticalFaultRun(); 

    //常规运行和用户控制运行
    void SelfAndUsrCtlRun(); 

    //本地用户干预控制
    bool LocalUsrCtlRun();

    //系统用户干预控制
    bool SystemUsrCtlRun();

	//优先控制
    bool PreemptCtlRun();

    //自主运行
    void SelfRun();

    //系统干预方案运行
    void SystemAskPlanRun();

	//优先干预方案运行
    void PreemptAskPlanRun();
                        
    //构造运行状态信息
    void ProcGlobalRunStatus(); 

	//进入手动控制时初始化手动控制对象
	void InitUsrCtlLogicObj();

	//创建控制模式
	void CreateCtlMode(int nCtlMode, int nCtlSource);

	//生成有效的控制指令
	void CreateValidManualCmd(TManualCmd tManualCmd, TManualCmd & tValidManualCmd);

    //手动面板黄闪处理      
    void UsrCtlYellowFalsh(TManualCmd tValidManualCmd);

	 //手动面板全红处理      
    void UsrCtlAllRed(TManualCmd tValidManualCmd);

	 //手动面板步进处理                    
    void UsrCtlStepForward(TManualCmd tManualCmd, TManualCmd & tValidManualCmd);

    //手动面板方向键处理      
    void UsrCtlDirectionKey(TManualCmd tManualCmd, TManualCmd & tValidManualCmd);

    //判断面板按钮能否使用
    bool CheckPanelBtnUseStatus(TManualCmd tManualCmd, TManualCmd tValidManualCmd);

	//判断配置软件指令能否使用
    bool CheckSystemCmdUseStatus(TManualCmd tManualCmd, TManualCmd tValidManualCmd);

	//判断优先控制指令能否使用
    bool CheckPreemptCmdUseStatus(TPreemptCtlCmd tPreemptCtlCmd);

    //设置按钮回复状态
    void SetPanelBtnStatusReply(TManualCmd tValidManualCmd, int nHWPanelBtnIndex);

    //写操作日志
    void WriteOperationRecord(int nCtlSource, int nOperationType, bool bStatus, char szPeerIp[]);

	//处理锁定通道灯色
	void ProcLockChannelLampClr();

	//锁定通道灯色控制
	void LockChannelCtl(TAscOnePlanChannelLockInfo tAscOnePlanChannelLockInfo, int nChannelCount);

	//锁定通道关灯
	void LockChannelOff(TAscOnePlanChannelLockInfo tAscOnePlanChannelLockInfo, int nChannelCount);

	//锁定通道的灯色过渡
	bool LockChannelTrans(TAscOnePlanChannelLockInfo tAscOnePlanChannelLockInfo, int nChannelIndex, bool bLockChannelOff, TLampClrStatus & tLampClrStatus);

	//获取所有时间段的锁定通道数量
	int  GetAllLockChannelCount(TAscOnePlanChannelLockInfo tAscOnePlanChannelLockInfo[], int nChannelCount);

	//判断当前时间是否在时间段内
	bool IsTimeInSpan(int nCurHour, int nCurMin, int nCurSec, int nStartHour, int nStartMin, int nStartSec, int nEndHour, int nEndMin, int nEndSec);

	//获取时间差(秒数)
	int  GetDiffTime(int nStartHour, int nStartMin, int nStartSec, int nEndHour, int nEndMin, int nEndSec);

	//计算计数的函数
	unsigned long CalcCounter(unsigned long nStart, unsigned long nEnd, unsigned long nMax);  

	//获取当前时间
	void OpenATCGetCurTime(int & nYear, int & nMon, int & nDay, int & nHour, int & nMin, int & nSec, int & nWeek);

	//根据索引获取面板按钮编号
	int GetManualBtnByIndex(int nIndex);

	//设置进入自主阶段
	void SetAllRedStage();

	//设置系统控制命令状态
	void SetSystemControlStatus(bool bSpecicalControlResult, int nSpecicalControlFailCode, bool bPatternControlResult, int nPatternControlFailCode,
		bool bStageControlResult, int nStageControlFailCode, bool bPhaseControlResult, int nPhaseControlFailCode,
		bool bChannelLockResult, int nChannelLockFailCode);

	//设置优先控制命令状态
	void SetPreemptControlStatus(bool bPreemptControlResult, int nPreemptControlFailCode);

	//锁定通道过渡切到方向控制
	void LockChannelTransToDirection(TAscOnePlanChannelLockInfo tAscOnePlanChannelLockInfo, int nChannelCount);

	void SwitchManualControlPatternToSelf();

	void SetPreemptStageIndex(BYTE byPreemptPhaseID, BYTE & byStageIndexTarget);

	void CreatePatternInterruptCmdInPreemptControl(TManualCmd  tManualCmd);
    
#ifdef VIRTUAL_DEVICE
    //倍速运行时根据全局计数推算出当前的时间//Virtual_Test2022
    void GetVirtualTimeByGlobalCount(int& nYear, int& nMon, int& nDay, int& nHour, int& nMin, int& nSec, int& nWeek);
#endif // VIRTUAL_DEVICE
//Virtual_Test2022

private:
    static COpenATCLogicCtlManager * s_pData;								//单件类指针

    COpenATCParameter * m_pLogicCtlParam;									//特征参数类指针

    COpenATCRunStatus * m_pLogicCtlStatus;									//运行状态类指针

	COpenATCLog       * m_pOpenATCLog;										//日志类指针

    int m_nLogicCtlStage;													//当前控制阶段。1表示启动黄闪，2表示启动全红，3表示自主控制

    int m_nCtlSource;														//干预指令来源

    int m_nCurPlanNo;														//当前方案号
    int m_nCurCtlMode;														//当前的控制方式    
    bool m_bIsCtlModeChg;													//控制方式是否发生变化

    bool m_bFirstInitFlag;													//用于标示是否第一次初始化参数

    CLogicCtlMode *		 m_pLogicCtlMode;									//控制方式实现类指针   

    TChannel             m_atOldChannelInfo[MAX_CHANNEL_COUNT];				//缓存最近的通道信息

    COpenATCOperationRecord m_tOpenATCOperationRecord;  					//操作记录对象

    TAscNetCard             m_atNetConfig[MAX_NETCARD_TABLE_COUNT];			//网络配置信息

	TAscStartSequenceInfo   m_tAscStartSequenceInfo;						//启动时序

	int                     m_nLockChannelTransStatus[MAX_CHANNEL_COUNT];   //用于记录锁定通道的过渡状态

	int                     m_nLockChannelCounter[MAX_CHANNEL_COUNT];		//用于锁定通道灯色运行时间

	TAscOnePlanChannelLockInfo  m_tOldAscOnePlanChannelLockInfo;			//用于缓存最近一次的锁定通道时间段

	TAscChannelVerifyInfo   m_tOldChannelCheckInfo;                         //用于缓存最近一次的通道检测数据

	TManualCmd              m_tOldValidManualCmd;                           //用于缓存最近一次有效的手动控制命令

	long					m_nManualControlPatternStartTime;               //用于记录手动控制方案的开始时间

	long					m_nManualControlPatternDurationTime;            //用于记录手动控制方案的持续时间

	TRunStageInfo           m_tRunStageTable;                               //阶段表

	TPreemptCtlCmd          m_tOldPreemptCtlCmd;                            //用于缓存最近一次的优先控制命令

	bool                    m_bInvalidPhaseCmd;                             //锁定相位对应相同的通道，指令不生效，但是发给平台的相位锁定状态要用后面发的相位

	TPhaseLockPara          m_tInvalidPhaseLockPara;                        //锁定相位对应相同的通道，不生效的指令

	time_t                  m_tPatternInterruptCmdTime;                     //方案干预指令下发时间

};

#endif // !ifndef OPENATCLOGICCTLMANAGER_H
