/*=====================================================================
模块名 ：感应式绿波控制模块
文件名 ：LogicCtlInductiveGreenWave.h
相关文件：
实现功能：用于定义感应式绿波控制实现接口
作者 ：梁厅
版权 ：<Copyright(c) 2019-2020 Suzhou Keda Technology Co.,Ltd. All right reserved.>
-----------------------------------------------------------------------
修改记录：
日 期           版本     修改人     走读人     修改记录
2019/9/26       V1.0     梁厅       梁厅       创建模块
=====================================================================*/

#ifndef LOGICCTL_INDUCTIVE_GREEN_WAVE_H
#define LOGICCTL_INDUCTIVE_GREEN_WAVE_H

#include "LogicCtlFixedTime.h"

const float C_F_ADJ_KEY_MODE = (float)0.3;
class CLogicCtlInductiveGreenWave :public CLogicCtlFixedTime
{
public:
	CLogicCtlInductiveGreenWave();
	virtual ~CLogicCtlInductiveGreenWave();

	//初始化感应控制方式需要的参数
	virtual void Init(COpenATCParameter * pParameter, COpenATCRunStatus * pRunStatus, COpenATCLog * pOpenATCLog, int nPlanNo);

protected:
	//环内机动车相位的运行状态更新
	virtual void OnePhaseRun(int nRingIndex);

	//环内行人相位的运行状态更新
	virtual void OnePedPhaseRun(int nRingIndex);

	//周期结束重置控制状态
	virtual void ReSetCtlStatus();

	//处理相位延长绿
	virtual void ProcExtendGreen(int nRingIndex,int nPhaseIndex,int nCurRunTime);

	//处理相位延迟绿
	virtual void LastConcurrencyPhaseBeforeBarrierExtendGreen(int nRingIndex,int nPhaseIndex,int nCurRunTime);

	//计算绿波相位的绿灯时间
	virtual void CalcGreenWavePhaseTime();

private:
	//计算实际相位差,调整相位绿灯时间
	short CalcRealOffset();

	//根据相位差,绿灯时间和最小绿，最大绿来计算实际调整步长
	short CalcRealAdjVal(short nOffset, WORD wGreenTime, WORD wMinGreen, WORD wMaxGreen);

	//short m_nRealOffset;									//实际相位差

	short m_nGreenStageChgLen;								//当前绿灯阶段可调整的长度

	bool m_bCalcOffsetFlag;									//计算相位差标志

	bool m_bOffsetCoordinateCloseFlag;						//相位差协调停止标志
	bool m_bCycleCoordinateCloseFlag;						//周期协调结束标志
	int m_nGreenPhaseSplitTime[MAX_RING_COUNT];
	int CurRunTotalCounter[MAX_RING_COUNT][MAX_PHASE_COUNT];//各相位运行累积时间
};
#endif //#!LOGICCTL_INDUCTIVE_GREEN_WAVE_H