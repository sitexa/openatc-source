/*=====================================================================
模块名 ：控制方式实现模块
文件名 ：LogicCtlMode.h
相关文件：
实现功能：用于定义控制模式实现接口
作者 ：刘黎明
版权 ：<Copyright(c) 2019-2020 Suzhou Keda Technology Co.,Ltd. All right reserved.>
-----------------------------------------------------------------------
修改记录：
日 期           版本     修改人     走读人     修改记录
2019/9/25       V1.0     刘黎明     王 五      创建模块
=====================================================================*/

#ifndef LOGICCTLMODE_H
#define LOGICCTLMODE_H

#include "../../Include/OpenATCRunStatus.h"
#include "../../Include/OpenATCParameter.h"
#include "../../Include/OpenATCLog.h"

enum
{
	DIRECTION_TRAN_NOTEND               = 0,
	DIRECTION_SWITCH_TO_DIRECTION       = 1,
	DIRECTION_SWITCH_TO_AUTOCTL	        = 2,
	DIRECTION_END_RETURN_TO_STEPFORWARD = 3,
	DIRECTION_END_RETUEN_TO_AUTOCTL	    = 4,
};


/*=====================================================================
类名 ：CLogicCtlMode
功能 ：控制方式实现接口
主要接口：Init
          Run
          Release
备注 ：
-----------------------------------------------------------------------
修改记录：
日 期          版本     修改人     走读人       修改记录
2019/09/14     V1.0     刘黎明     刘黎明       创建类
2020/1/25      V1.0     李永萍     李永萍       增加手动面板按钮切换时，保存和获取特征参数接口
2020/2/25      V1.0     李永萍     李永萍       增加判断方向持续时间是否结束的函数和当前阶段对应的相位是否运行结束的函数
=====================================================================*/
class CLogicCtlMode  
{
public:
	CLogicCtlMode();
	virtual ~CLogicCtlMode();

    //初始化控制方式内部参数,用于人工干预方案时使用
    virtual void Init(COpenATCParameter * pParameter, COpenATCRunStatus * pRunStatus, COpenATCLog * pOpenATCLog, int nPlanNo);

    //控制方式主流程接口
    virtual void Run() = 0;

    //控制方式资源释放接口
    virtual void Release();

    //根据m_atLampClr设置全局灯色状态
    virtual void SetLampClr(TLampClrStatus & tLampClr);

    //设置全局取参数状态
    virtual void SetGetParamFlag(bool bFlag);

    //获取当前使用的控制参数及控制状态
    virtual void * GetCurCtlParam(); 

    //获取当前灯色
    virtual void * GetCurLampClr();

    //获取阶段表
    virtual void * GetCurStageTable();

    //获取通道的绿信比模式
    virtual void * GetCurChannelSplitMode();

    //设置衍生控制参数和控制状态，用于手动控制时的参数继承
    virtual void SetCtlDerivedParam(void * pParam,void * pLampClr,void * pStageTable,void * pChannelSplitMode);

    //设置用户控制标志
    virtual void SetUsrCtlFlag(bool bFlag);

    //设置系统用户控制标志
    virtual void SetSystemUsrCtlFlag(bool bFlag);

	//设置优先控制标志
    virtual void SetPreemptCtlFlag(bool bFlag);

    //设置运行状态
    virtual void GetPhaseRunStatus(TPhaseRunStatus & tRunStatus);

    //设置显示屏显示信息状态
    virtual void GetLedScreenShowInfo(TLedScreenShowInfo & tLedScreenShowInfo);

	//判断当前通道的灯色是否是绿色
    virtual bool IsChannelGreen(BYTE byChannelType, int nDirectionIndex, int nChannelLockStatus[]);

	//设置切到锁定之前的过渡运行状态
	virtual void GetTransRunStatusBeforeLockPhase(TPhaseRunStatus & tRunStatus);

	//设置锁定运行状态
	virtual void GetLockPhaseRunStatus(TPhaseRunStatus & tRunStatus, bool bInvalidPhaseCmd, TPhaseLockPara tInvalidPhaseLockPara);

	TRunStageInfo GetRunStageTable();

protected:
    //初始化通道参数
    virtual void InitChannelParam();

    //设置单个通道的灯色输出状态
    void SetOneChannelOutput(char * pStart,char chStage);

	//计算计数的函数
	unsigned long CalcCounter(unsigned long nStart, unsigned long nEnd, unsigned long nMax);  

	//获取当前时间
	void OpenATCGetCurTime(int & nYear, int & nMon, int & nDay, int & nHour, int & nMin, int & nSec, int & nWeek);

#ifdef VIRTUAL_DEVICE
    //倍速运行时根据全局计数推算出当前的时间//Virtual_Test2022
    void GetVirtualTimeByGlobalCount(int& nYear, int& nMon, int& nDay, int& nHour, int& nMin, int& nSec, int& nWeek);
#endif // VIRTUAL_DEVICE
//Virtual_Test2022

    COpenATCRunStatus * m_pOpenATCRunStatus;
    COpenATCParameter * m_pOpenATCParameter;
	COpenATCLog       * m_pOpenATCLog;

    char m_achLampClr[C_N_MAXLAMPOUTPUT_NUM];                                   //灯控板灯输出端子状态信息数组

    bool m_bIsLampClrChg;                                                       //环灯色是否发生变化

    bool m_bIsUsrCtl;                                                           //是否有用户干预 

    bool m_bIsSystemCtl;                                                        //是否有平台干预

	bool m_bIsPreemptCtl;                                                       //是否有优先控制

    int m_nChannelCount;                                                        //通道数量

    TChannel m_atChannelInfo[MAX_CHANNEL_COUNT];                                //通道数组

    TRunStageInfo m_tRunStageInfo;                                              //阶段表

    int m_nChannelSplitMode[MAX_CHANNEL_COUNT];                                 //通道对应的绿信比模式

	bool m_bKeepGreenChannelBeforeControlChannelFlag[MAX_CHANNEL_COUNT];        //控制通道开始前继续保持绿色的通道

    bool m_bShieldStatus[MAX_CHANNEL_COUNT];                                    //通道的屏蔽状态

    bool m_bProhibitStatus[MAX_CHANNEL_COUNT];                                  //通道的禁止状态

    bool m_bOldShieldStatus[MAX_CHANNEL_COUNT];                                 //通道的历史屏蔽状态

    bool m_bOldProhibitStatus[MAX_CHANNEL_COUNT];                               //通道的历史禁止状态

    int m_nCurRunMode;                                                          //当前信号机的运行模式
};

#endif //ifndef LOGICCTLMODE_H
