/*=====================================================================
模块名 ：感应控制方式实现模块
文件名 ：LogicCtlActuate.h
相关文件：
实现功能：用于定义感应控制模式实现接口
作者 ：刘黎明
版权 ：<Copyright(c) 2019-2020 Suzhou Keda Technology Co.,Ltd. All right reserved.>
-----------------------------------------------------------------------
修改记录：
日 期           版本     修改人     走读人     修改记录
2019/9/26       V1.0     刘黎明     刘黎明     创建模块
=====================================================================*/

#ifndef LOGICCTLACTUATE_H
#define LOGICCTLACTUATE_H

#include "LogicCtlFixedTime.h"

class CLogicCtlActuate : public CLogicCtlFixedTime
{
public:
	CLogicCtlActuate();
	virtual ~CLogicCtlActuate();

    //初始化感应控制方式需要的参数
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
	bool				m_bChangeToYellow[MAX_RING_COUNT];										//行人通道的绿闪要跟机动车通道的绿闪同时结束
};

#endif // !ifndef LOGICCTLACTUATE_H





















