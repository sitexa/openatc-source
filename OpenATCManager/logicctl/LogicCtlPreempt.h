/*=====================================================================
模块名 ：优先控制方式实现模块
文件名 ：LogicCtlPreempt.h
相关文件：
实现功能：用于定义优先控制模式实现接口
作者 ：李永萍
版权 ：<Copyright(c) 2019-2020 Suzhou Keda Technology Co.,Ltd. All right reserved.>
-----------------------------------------------------------------------
修改记录：
日 期           版本     修改人     走读人     修改记录
2019/9/26       V1.0     李永萍     李永萍      创建模块
=====================================================================*/

#ifndef LOGICCTLPREEMPT_H
#define LOGICCTLPREEMPT_H

#include "LogicCtlFixedTime.h"

class CLogicCtlPreempt : public CLogicCtlFixedTime  
{
public:
	CLogicCtlPreempt();
	virtual ~CLogicCtlPreempt();

    //初始化手动控制方式需要的参数和状态,主要用于初始化状态,参数从手动之前的控制方式继承而来
    virtual void Init(COpenATCParameter * pParameter, COpenATCRunStatus * pRunStatus, COpenATCLog * pOpenATCLog, int nPlanNo);

    //优化控制方式主流程
    virtual void Run();

    //设置优化控制时的参数
    virtual void SetCtlDerivedParam(void * pParam,void * pLampClr,void * pStageTable,void * pChannelSplitMode);

protected:
	//优化控制环内机动车相位的运行状态更新
    void PreemptCtlOnePhaseRun(int nRingIndex);

	//优化控制环内行人相位的运行状态更新
    void PreemptCtlOnePedPhaseRun(int nRingIndex);

    //优化控制时相位运行状态更新
    void PreemptCtlPhaseRun();

    //多环周期运行结束判断更新
    void ManualCycleChg(bool bIsAutoCtl);

private:
	enum
	{
		MAX_GREEN_TIME = 32000,
	};

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
	};

    bool  ManualSwitchStage(int & nStageIndex);

	bool  TransCurStageToNextStage();

	void  CheckIfStageHavePhaseInGreen(int & nGreenPhaseCount);

	void  SendCountDownPulse(int nRingIndex, bool bNeedSendRedPulseFlag);

	void  SendPedCountDownPulse(int nRingIndex, bool bNeedSendRedPulseFlag);

	int   GetPhaseRemainTime(int nRingIndex, TPhaseCtlInfo * pPhaseCtlInfo, TPhaseLampClrRunCounter tRunCounter, DWORD wPhaseTime);

	int   GetPedPhaseRemainTime(int nRingIndex, TPhaseCtlInfo * pPhaseCtlInfo, TPhaseLampClrRunCounter tRunCounter, DWORD wPhaseTime);

	void  SendRedPulse(int nPhaseType, PTRingCtlInfo ptRingRunInfo, int nRingIndex, int nIndex, int nNextIndex, bool bFlag);

	void  ResetNextStagePhaseGreenTime(int nRingIndex, int nPhaseIndex, int nNextStageIndex, bool bChangePhaseFlag, int nDuration);

	void  RecalculteStageTimeByDelayTimeAndDuration();

	void  InitNextStagePhase(int nRingIndex, int nPhaseIndex, int nNextStageIndex, bool bChangePhaseFlag);

	void  CreatePreemptCtlCmdReturnToSelf();

	void  SetTransStatus();

	void  ClearPulse();

	bool  AdjustPhaseGreenTime(int nRingIndex, int nIndex, int nStageIndexTarget, int nGreenTimeFlag);

	bool  AdjustPedPhaseGreenTime(int nRingIndex, int nIndex, int nStageIndexTarget, int nGreenTimeFlag);

	void  SetOverlapPhaseLampClr(int nNextStageIndex);

	bool  GetPreemptCtlCmdFromList();

	bool  IsHaveUrgentCtlCmd();

	int   GetRingIndex(int nPhaseID);

	int   GetNextStageIndex();

	void  SetPreemptCtlCmd(int nStageIndex);

	char  GetPreemptCtlPhaseStage(int nPhaseID);

	void  SetOverlapCurPreemptPhaseLampClrChgFlag();

	void  SetOverlapCurPreemptPhaseLampClr(int nOverlapIndex, BYTE byOverlapNum, bool bPedOverlapPhase);

	bool  IsNeglectPhase(int nPhaseID);

	int   SetNeglectPhaseRunTime(int nRingIndex, bool bPedPhase);

	void  ReSetNeglectPhaseStageRunTime();

	void  ReSetNeglectPhasetBackUpTime();

 private:
    TFixTimeCtlInfo      m_tOldFixTimeCtlInfo;								//缓存手动面板切换时最近的特征参数和控制状态信息

	bool                 m_bClearPulseFlag;									//用于记录是否清除脉冲

	bool                 m_bSendRedPulse[MAX_RING_COUNT];					//用于记录机动车红脉冲是否计算完成

	bool                 m_bSendPedRedPulse[MAX_RING_COUNT];				//用于记录行人红脉冲是否计算完成

	int                  m_nManualCurStatus;								//用于过渡状态

	int                  m_nNextStageIndex;									//用于记录下个阶段的索引

	bool                 m_bCycleEndFlag;									//用于记录是否过渡完整个周期

	int					 m_nReturnAutoCtrlStageIndex;						//用于用户干预超时，自动返回自主控制

	int					 m_nStageRunTime[MAX_RING_COUNT];					//用于记录阶段运行时长

	int					 m_nStageTimeForPhasePass[MAX_RING_COUNT];			//用于记录阶段时长

	bool				 m_bPhaseColorChgToYellowFlag[MAX_RING_COUNT];		//用于记录机动车相位是否切往黄灯	

	bool                 m_bPreemptCtlCmdEndFlag;						    //用于记录优先控制指令执行结束标志

	bool                 m_bPreemptCtlStageProcessFlag[MAX_STAGE_COUNT];    //用于记录优先阶段处理标志

	TPreemptCtlCmd       m_tPreemptCtlCmd;                                  //未开始切优先控制之前的优先控制缓存

	TPreemptCtlCmd       m_tNextPreemptCtlCmd;                              //开始切优先控制之后的下一个优先控制缓存

	bool                 m_bOverlapChgFlag[MAX_OVERLAP_COUNT];              //机动车跟随相位的母相位是优先相位的灯色变化标志

	bool                 m_bPedOverlapChgFlag[MAX_OVERLAP_COUNT];           //行人跟随相位的母相位是优先相位的灯色变化标志

	bool                 m_bCurAndNextCmdInSameStage[MAX_RING_COUNT];       //当前优先控制和下一个优先控制在同一个阶段
};

#endif // ifndef LOGICCTLPREEMPT_H
