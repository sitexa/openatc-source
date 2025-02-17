/*=====================================================================
模块名 ：人工智能控制优化模块
文件名 ：LogicCtlAIOptim.h
相关文件：
实现功能：用于定义人工智能控制优化模块实现接口
作者 ：刘黎明
版权 ：<Copyright(c) 2019-2020 Suzhou Keda Technology Co.,Ltd. All right reserved.>
-----------------------------------------------------------------------
修改记录：
日 期           版本     修改人     走读人     修改记录
2019/9/26       V1.0     刘黎明     刘黎明     创建模块
=====================================================================*/

#ifndef LOGICCTLAIOPTIM_H
#define LOGICCTLAIOPTIM_H

#include "LogicCtlFixedTime.h"

typedef struct tagAIOptimInfo
{
	bool m_bPredetFlag;							//预测标志
	BYTE m_byOptimPhase[MAX_PHASE_COUNT];		//优化相位
	int  m_nRetainTime[MAX_PHASE_COUNT];		//相位绿灯持续时间
}TAIOptimInfo, *PTAIOptimInfo;


class LogicCtlAIOptim : public CLogicCtlFixedTime
{
public:
	LogicCtlAIOptim();
	virtual ~LogicCtlAIOptim();

    //初始化人工智能优化控制需要的参数
    virtual void Init(COpenATCParameter * pParameter, COpenATCRunStatus * pRunStatus, COpenATCLog * pOpenATCLog, int nPlanNo);

protected:
    //环内机动车相位的运行状态更新
    virtual void OnePhaseRun(int nRingIndex);

    //环内行人相位的运行状态更新
    virtual void OnePedPhaseRun(int nRingIndex);
    
    //处理相位延长绿
    virtual void ProcExtendGreen(int nRingIndex,int nPhaseIndex,int nCurRunTime);

	//处理相位延长绿
    virtual void LastConcurrencyPhaseBeforeBarrierExtendGreen(int nRingIndex,int nPhaseIndex,int nCurRunTime);
    
private:
};

#endif // !ifndef LOGICCTLAIOPTIM_H





















