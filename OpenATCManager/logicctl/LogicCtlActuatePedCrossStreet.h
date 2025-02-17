/*=====================================================================
模块名 ：感应式行人过街控制方式实现模块
文件名 ：LogicCtlActuatePedCrossStreet.h
相关文件：
实现功能：用于定义感应式行人过街控制模式实现接口
作者 ：陈涵燕
版权 ：<Copyright(c) 2019-2021 Suzhou Keda Technology Co.,Ltd. All right reserved.>
-----------------------------------------------------------------------
修改记录：
日 期           版本     修改人     走读人     修改记录
2020/11/24      V1.0     陈涵燕      	       创建模块
=====================================================================*/

#ifndef LOGICCTLACTUATEPEDCROSSSTREET_H
#define LOGICCTLACTUATEPEDCROSSSTREET_H

#include "LogicCtlFixedTime.h"

class CLogicCtlActuatePedCrossStreet : public CLogicCtlFixedTime  
{
public:
	CLogicCtlActuatePedCrossStreet();
	virtual ~CLogicCtlActuatePedCrossStreet();

    //初始化行人过街控制方式需要的参数
    virtual void Init(COpenATCParameter * pParameter, COpenATCRunStatus * pRunStatus, COpenATCLog * pOpenATCLog,  int nPlanNo);

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
	TVehicleQueueUpInfo m_tVehicleQueueUpInfo[MAX_VEHICLEDETECTOR_COUNT];//车辆排队信息

	TPedDetectInfo m_tPedDetectInfo[MAX_VEHICLEDETECTOR_COUNT];//行人检测信息

	enum
	{
		PED_DETECTOR_WAIT_AREA = 1,
		PED_DETECTOR_CROSS_AREA = 2,
	};

	int m_nPedDetecetorType[MAX_VEHICLEDETECTOR_COUNT];
	int m_nPhaseThresholdVehicle[MAX_VEHICLEDETECTOR_COUNT];
	int m_nPhaseThresholdPedWait[MAX_VEHICLEDETECTOR_COUNT];
};

#endif // ifndef LOGICCTLACTUATEPEDCROSSSTREET_H
