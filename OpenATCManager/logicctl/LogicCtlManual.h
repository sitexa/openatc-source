/*=====================================================================
模块名 ：手动控制方式实现模块
文件名 ：LogicCtlManual.h
相关文件：
实现功能：用于定义手动控制模式实现接口
作者 ：刘黎明
版权 ：<Copyright(c) 2019-2020 Suzhou Keda Technology Co.,Ltd. All right reserved.>
-----------------------------------------------------------------------
修改记录：
日 期           版本     修改人     走读人     修改记录
2019/9/26       V1.0     刘黎明     刘黎明      创建模块
2019/2/28       V1.0     李永萍     刘黎明      修改系统用户控制逻辑
2019/3/28       V1.0     李永萍     刘黎明      增加手动面板按钮切换处理
=====================================================================*/

#ifndef LOGICCTLMANUAL_H
#define LOGICCTLMANUAL_H

#include "LogicCtlFixedTime.h"

const WORD MAX_GREEN_TIME = 32000;

typedef struct tagLockPhaseData
{
	int   nCurLockRingIndex;                    //当前锁定相位环号
	int   nCurLockPhaseID;                      //当前锁定相位ID
	int   nCurLockStageIndex;                   //当前锁定阶段编号
	int   nTargetPhaseIDInCurRing;              //当前环中目标相位ID，和非当前环的目标相位处于同一个阶段
	int   nTargetStageIndex;                    //目标阶段编号，非当前环的目标相位对应的阶段编号
	int   nLockChannelCount;                    //当前锁定相位对应通道数量           
	int   nLockChannelID[MAX_CHANNEL_COUNT];    //当前锁定相位对应通道编号   
	bool  bNeedTransFlag;                       //需要往目标相位切换标志
	bool  bSwitchLockChannelToNextPhase;        //锁定相位过渡结束以后，往目标相位切换，开始切下一个相位标志
	bool  bSwitchSuccessFlag;                   //成功切换相位到另外一个环的锁定相位的同阶段标志
	bool  bLockPhaseInSameStageFlag;            //锁定相位在同一个阶段标志
}TLockPhaseData,PLockPhaseData;

typedef struct tagLockPhaseStage
{
	int   nLockPhaseCount;                      //用于记录锁定相位数量
	int   nLockPhaseID[MAX_PHASE_COUNT];        //用于记录当前锁定相位ID
	char  chLockPhaseStage[MAX_PHASE_COUNT];    //用于记录当前锁定相位状态
	int   nLockPhaseCounter[MAX_PHASE_COUNT];	//用于记录当前锁定相位运行时间
}TLockPhaseStage,PLockPhaseStage;

class CLogicCtlManual : public CLogicCtlFixedTime  
{
public:
	CLogicCtlManual();
	virtual ~CLogicCtlManual();

    //初始化手动控制方式需要的参数和状态,主要用于初始化状态,参数从手动之前的控制方式继承而来
    virtual void Init(COpenATCParameter * pParameter, COpenATCRunStatus * pRunStatus, COpenATCLog * pOpenATCLog, int nPlanNo);

    //手动控制方式主流程
    virtual void Run();

    //设置手动控制时的参数
    virtual void SetCtlDerivedParam(void * pParam,void * pLampClr,void * pStageTable,void * pChannelSplitMode);

protected:
    //手动控制环内机动车相位的运行状态更新
    virtual void ManualOnePhaseRun(int nRingIndex,TManualCmd tValidManualCmd);

    //系统干预环内机动车相位的运行状态更新
    virtual void SysCtlOnePhaseRun(TManualCmd  & tValidManualCmd);

    //手动控制环内行人相位的运行状态更新
    virtual void ManualOnePedPhaseRun(int nRingIndex,TManualCmd tValidManualCmd);

    //多环周期运行结束判断更新
    virtual void ManualCycleChg(bool bIsAutoCtl);

	//设置方案切到锁定过渡运行状态
	virtual void GetTransRunStatusBeforeLockPhase(TPhaseRunStatus & tRunStatus);

	//设置锁定运行状态
	virtual void GetLockPhaseRunStatus(TPhaseRunStatus & tRunStatus, bool bInvalidPhaseCmd, TPhaseLockPara tInvalidPhaseLockPara);

private:
	enum
	{
		MANUAL_CONTROL_STATUS = 1,
		MANUAL_STAGE_TRANS = 2,
	};

	enum
	{
		MANUAL_MINI_GREEN			= 1,
		MANUAL_PHASE_RUN_GREEN      = 2,
		MANUAL_PHASE_SPLIT_GREEN    = 3,
		MANUAL_PHASE_TOPLIMIT_GREEN = 4,
	};

	enum
	{
		STEPWARD_TO_NONSTEPWARD = 1,
		STEPWARD_TO_STEPWARD    = 2,
		NONSTEPWARD_TO_STEPWARD = 3,
		FIRSTPANEL_TO_DIRECTION = 4,
		FIRSTPANEL_TO_PATTERN   = 5,
		PATTERN_TO_CHANNELLOCK  = 6,
		PATTERN_TO_PATTERN      = 7,
		FIRSTPANEL_TO_FIRSTSTEPWARD = 8,
		LOCKCHANNEL_TO_DIRECTION    = 9,
	};

    bool  ManualSwitchStage(TManualCmd  tValidManualCmd, int & nStageIndex);

	void  ChangeChannelClr();

    bool  TransitChannelClr(BYTE byChannelType, bool bLockCmd);

    void  InitParamBeforeDirection(int nNextDirectionIndex);

	bool  IsTimeInSpan(int nCurHour, int nCurMin, int nCurSec, int nStartHour, int nStartMin, int nStartSec, int nEndHour, int nEndMin, int nEndSec);

	bool  CheckIfNeedToRunToNextStage(TManualCmd & tValidManualCmd);

	bool  CheckIfNeedToStepForwardToNextStage(TManualCmd & tValidManualCmd);

	void  CheckIfStageHavePhaseInGreen(int & nGreenPhaseCount, int & nClosePhaseCount);

	void  SendCountDownPulse(int nRingIndex, bool bNeedSendRedPulseFlag);

	void  SendPedCountDownPulse(int nRingIndex, bool bNeedSendRedPulseFlag);

    void  ProcessDirection(TManualCmd & tValidManualCmd);
	
	void  GetChannelData(int nNextDirectionIndex);

	void  ProcessStepward(TManualCmd  & tValidManualCmd);

	int   GetPhaseRemainTime(int nRingIndex, TPhaseCtlInfo * pPhaseCtlInfo, TPhaseLampClrRunCounter tRunCounter, DWORD wPhaseTime);

	int   GetPedPhaseRemainTime(int nRingIndex, TPhaseCtlInfo * pPhaseCtlInfo, TPhaseLampClrRunCounter tRunCounter, DWORD wPhaseTime);

	void  SendRedPulse(int nPhaseType, PTRingCtlInfo ptRingRunInfo, int nRingIndex, int nIndex, int nNextIndex, bool bFlag);

	void  ResetNextStagePhaseGreenTime(int nRingIndex, int nPhaseIndex, int nNextStageIndex, bool bChangePhaseFlag, int nDuration);

	void  RecalculteStageTimeByDelayTimeAndDuration(TManualCmd  tValidManualCmd);

	void  RecalculteCurPhaseGreenTimeAndSetStatus(TManualCmd  & tValidManualCmd);

	void  InitNextStagePhase(int nRingIndex, int nPhaseIndex, int nNextStageIndex, bool bChangePhaseFlag);

	void  CreateManualCmdReturnToSelf(TManualCmd tValidManualCmd);

	void  SetTransStatus(TManualCmd  & tValidManualCmd);

	void  ProcessFirstManual(TManualCmd  & tValidManualCmd);

	void  ProcessPhasePassControlStatus(TManualCmd  & tValidManualCmd);

	void  ClearPulse();

	void  PanelFirstStepWard(int nStageIndexTarget);

	void  NonStepWardToStepWard(int nStageIndexTarget);

	void  FirstPanelManualSwitchToDirectionOrPattern(TManualCmd  & tValidManualCmd);

	void  BackUpPhaseTime(int nRefreshStageTime);

	bool  AdjustPhaseGreenTime(int nRingIndex, int nIndex, int nStageIndexTarget, int nGreenTimeFlag);

	bool  AdjustPedPhaseGreenTime(int nRingIndex, int nIndex, int nStageIndexTarget, int nGreenTimeFlag);

	//设置跟随相位灯色
	void  SetOverlapPhaseLampClr(int nNextStageIndex);

	void  InitParamBeforeChannelLock(int nChannelLockStatus[]);

	void  ProcessChannelLock(TManualCmd & tValidManualCmd);

	void  PatternSwitchToChannelLock(TManualCmd  & tValidManualCmd);

	void  SetKeepGreenChannelBeforeControlChannel(BYTE byGreenStageVehPhaseID, BYTE byGreenStagePedPhaseID, TManualCmd tValidManualCmd);

	bool  CheckSendGreenPulse(BYTE byPhaseNumber);

	void  PhaseTransBasedOnControlChannelFlag();

	bool  IsChannelNeedTranClr(BYTE byChannelIndex);

	void  ProcessAllClosePhaseInCurStage(TManualCmd  tValidManualCmd, int nNextStageIndex);

	bool  IsNeedTransBeforeControlChannel(int nPhaseID);

	void  GetPhaseGreenTimeAfterLockPhase(int nRingIndex, int nPhaseIndex, int nNextStageIndex, bool bChangePhaseFlag);

	void  GetLockPhaseData(TManualCmd  & tValidManualCmd);

	void  GetNextPhaseAfterLockEnd(bool bInSameStage, int nLockPhaseCount);

	bool  IsTransChannel(BYTE byChannelType, int nChannelIndex);

	void  SwitchLockChannelPhaseToNext();

	bool  IsTargetPhaseChannel(int nChannelIndex);

	bool  IsOverlapNeedKeepGreen(BYTE byOverlapIndex, int nOverlapPhaseType);

	bool  IsHasTargetPhaseIncludedOldLockPhase(BYTE byOverlapIndex, int nOverlapPhaseType, int & nLockPhaseIndex);

	void  GetOldLockPhaseStatus(BYTE byOverlapIndex, int nOverlapPhaseType, char & chStatus);

	bool  LockChannelTransClr();

	int   ProcessLockChannelToPanel(TManualCmd  tValidManualCmd);

	bool  IsNeglectPhase(int nPhaseID);

	int   SetNeglectPhaseRunTime(int nRingIndex, bool bPedPhase);

	void  ReSetNeglectPhaseStageRunTime();

	void  ReSetNeglectPhasetBackUpTime();

	void  SetNeglectChannelBoforePhaseLock();

	void  SetLockPhaseList(TManualCmd  tValidManualCmd);

	void  SetLockPhaseStage(TManualCmd  tValidManualCmd);

 private:
	TManualCmd           m_tOldValidManualCmd;								//缓存最近一次有效的手动控制命令

    TFixTimeCtlInfo      m_tOldFixTimeCtlInfo;								//缓存手动面板切换时最近的特征参数和控制状态信息

    bool                 m_bChangeChannelClr[MAX_CHANNEL_COUNT];			//用于记录是否改变通道颜色
   
    char                 m_chChannelStage[MAX_CHANNEL_COUNT];				//用于记录方向控制时的机动车通道灯色阶段

	char                 m_chChannelStatus[MAX_CHANNEL_COUNT];				//用于记录切换方向控制时配置的机动车通道灯色阶段

	int                  m_nChannelCounter[MAX_CHANNEL_COUNT];				//用于记录切入方向控制时通道灯色运行时间

    int                  m_nChannelDurationCounter[MAX_CHANNEL_COUNT];		//用于记录切入方向控制时通道灯色运行时间

	bool                 m_bChannelTran[MAX_CHANNEL_COUNT];					//用于判断记录方向控制时的机动车通道是否过渡完成

	bool                 m_bClearPulseFlag;									//用于记录是否清除脉冲

	bool                 m_bSendRedPulse[MAX_RING_COUNT];					//用于记录机动车红脉冲是否计算完成

	bool                 m_bSendPedRedPulse[MAX_RING_COUNT];				//用于记录行人红脉冲是否计算完成

	int                  m_nManualCurStatus;								//用于过渡状态

	int                  m_nNextStageIndex;									//用于记录下个阶段的索引

	bool                 m_bCycleEndFlag;									//用于记录是否过渡完整个周期

	int					 m_nReturnAutoCtrlStageIndex;						//用于用户干预超时，自动返回自主控制

	bool                 m_bTransToAutoFlag;								//用于记录是否切回自主

	bool                 m_bIsDirectionChannelClrChg;						//方向通道灯色是否发生变化

	TPhasePassCmdPhaseStatus m_tPhasePassCmdPhaseStatus;					//用于记录当前相位放行状态

	TPhasePassCmdPhaseStatus m_tPhasePassCmdPhaseStatusFromUser;			//用于记录相位放行状态控制指令

	int					m_nStageRunTime[MAX_RING_COUNT];					//用于记录阶段运行时长

	int					m_nStageTimeForPhasePass[MAX_RING_COUNT];			//用于记录阶段时长

	bool				m_bPhaseColorChgToYellowFlag[MAX_RING_COUNT];		//用于记录机动车相位是否切往黄灯	

	bool				m_bIfPhasePassCmdPhaseStatusHaveRefreshedToInit;	//用于记录手动控制模式下，相位放行状态控制表是否已刷新为全部放行

	int                 m_nDirectionGreenTime[MAX_CHANNEL_COUNT];           //用于记录方向控制时的绿灯持续时间

	int                 m_nChannelLockGreenTime[MAX_CHANNEL_COUNT];         //用于通道锁定时的绿灯持续时间

	bool                m_bChannelKeepGreenFlag[MAX_CHANNEL_COUNT];         //用于通道锁定切换时的继续保持绿灯标志

	bool                m_bNextClrStageIsFInColorStep;                      //用于记录色步模式下一个灯色状态为可结束的标志

	bool                m_bStageStepwardInColorStep;                        //用于记录色步模式下阶段步进的标志

	TLockPhaseData      m_tLockPhaseData[MAX_PHASE_COUNT];                  //用于记录相位锁定数据

	bool                m_bTargetLockPhaseChannelFlag[MAX_CHANNEL_COUNT];   //用于记录目标锁定相位通道绿灯标志

	bool                m_bNonTargetLockPhaseEndFlag[MAX_CHANNEL_COUNT];    //用于记录非目标锁定相位通道锁定结束标志

	TPhaseLockPara      m_tNewPhaseLockPara;                                //用于记录新的相位锁定指令参数

	bool                m_bNeglectChannelBoforePhaseLock[MAX_CHANNEL_COUNT];//用于记录在相位锁定之前的特殊通道(忽略或关断)

	bool                m_bOldLockChannelCmdEndFlag;                        //用于记录老的通道锁定指令结束标志

	TLockPhaseStage     m_tLockPhaseStage;                                  //用于记录锁定相位的灯色阶段
};

#endif // ifndef LOGICCTLMANUAL_H
