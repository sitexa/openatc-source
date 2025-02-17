/*=====================================================================
模块名 ：无电缆线控控制方式实现模块
文件名 ：LogicCtlCablelessLine.h
相关文件：
实现功能：用于定义无电缆线控控制模式实现接口
作者 ：刘黎明
版权 ：<Copyright(c) 2019-2020 Suzhou Keda Technology Co.,Ltd. All right reserved.>
-----------------------------------------------------------------------
修改记录：
日 期           版本     修改人     走读人     修改记录
2019/9/26       V1.0     刘黎明     刘黎明     创建模块
2020/4/26       V1.0     李永萍     李永萍     修改调整系数
=====================================================================*/

#ifndef LOGICCTLCABLELESSLINE_H
#define LOGICCTLCABLELESSLINE_H

#include "LogicCtlFixedTime.h"

const int C_N_SECONDS_PERHOUR = 3600;
const int C_N_SECONDS_PERMIN = 60;
const float C_F_ADJ_KEY = (float)0.3;

class CLogicCtlCablelessLine : public CLogicCtlFixedTime  
{
public:
	CLogicCtlCablelessLine();
	virtual ~CLogicCtlCablelessLine();

    //初始化感应控制方式需要的参数
    virtual void Init(COpenATCParameter * pParameter, COpenATCRunStatus * pRunStatus, COpenATCLog * pOpenATCLog, int nPlanNo);

protected:
    //环内机动车相位的运行状态更新
    virtual void OnePhaseRun(int nRingIndex);

    //环内行人相位的运行状态更新
    virtual void OnePedPhaseRun(int nRingIndex);

    //周期结束重置控制状态
    virtual void ReSetCtlStatus();

private:
    //计算实际相位差,调整相位绿灯时间
    short CalcRealOffset();

    //根据相位差,绿灯时间和最小绿，最大绿来计算实际调整步长
    short CalcRealAdjVal(short nOffset,WORD wGreenTime,WORD wMinGreen,WORD wMaxGreen);

	//计算所有相位的绿灯变化长度
	void  CalculateAllPhaseGreenChgLen();

	//重置所有相位的绿灯时间
	void  ResetAllPhaseGreenTime();

	//设置相位绿信比
	int  SetPhaseSplitTime(int nRingIndex, int nPhaseIndex, WORD wSplitTime, bool bFlag);

	//设置原参数配置的相位绿信比
	void  SetPhaseSplitTimeFromParam();

	int   FloatToInt(float f);

    //short m_nRealOffset;														//实际相位差

    //short m_nGreenStageChgLen;												//当前绿灯阶段可调整的长度

    bool m_bCalcOffsetFlag;														//计算相位差标志

	int    m_nBarrierPhaseDiffArrCount;											//屏障数量

	short  m_nAllGreenStageChgLen[MAX_RING_COUNT][MAX_SEQUENCE_TABLE_COUNT];	//每个环的屏障前的绿灯调整长度和
	
	short  m_nGreenStageChgLen[MAX_RING_COUNT][MAX_SEQUENCE_TABLE_COUNT];		//每个环的每个相位的绿灯调整长度
	
	short  m_nSplitTimeBeforeBarrier[MAX_RING_COUNT][MAX_SEQUENCE_TABLE_COUNT];	//每个环的屏障前的绿信比和

	bool   m_bChangeToYellow[MAX_RING_COUNT];									//行人通道的绿闪要跟机动车通道的绿闪同时结束
};

#endif // ifndef LOGICCTLCABLELESSLINE_H
