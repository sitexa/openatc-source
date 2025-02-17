/*=====================================================================
模块名 ：行人过街控制方式实现模块
文件名 ：LogicCtlCablelessLine.h
相关文件：
实现功能：用于定义行人过街控制模式实现接口
作者 ：刘黎明
版权 ：<Copyright(c) 2019-2020 Suzhou Keda Technology Co.,Ltd. All right reserved.>
-----------------------------------------------------------------------
修改记录：
日 期           版本     修改人     走读人     修改记录
2019/9/26       V1.0     刘黎明     刘黎明     创建模块
=====================================================================*/

#ifndef LOGICCTLPEDCROSSSTREET_H
#define LOGICCTLPEDCROSSSTREET_H

#include "LogicCtlFixedTime.h"

class CLogicCtlPedCrossStreet : public CLogicCtlFixedTime  
{
public:
	CLogicCtlPedCrossStreet();
	virtual ~CLogicCtlPedCrossStreet();

    //初始化行人过街控制方式需要的参数
    virtual void Init(COpenATCParameter * pParameter, COpenATCRunStatus * pRunStatus, COpenATCLog * pOpenATCLog,  int nPlanNo);

protected:
    //环内机动车相位的运行状态更新
    virtual void OnePhaseRun(int nRingIndex);

    //环内行人相位的运行状态更新
    virtual void OnePedPhaseRun(int nRingIndex);

    //行人检测器是否有行人请求
    virtual bool IsPedAskCrossStreet();

private:
    bool  m_bIsPedAskPhase;

	int   m_nGreenRunTime[MAX_RING_COUNT][MAX_SEQUENCE_TABLE_COUNT];
};

#endif // ifndef LOGICCTLPEDCROSSSTREET_H
