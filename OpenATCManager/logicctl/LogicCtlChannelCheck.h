/*=====================================================================
模块名 ：通道检测控制方式实现模块
文件名 ：LogicCtlChannelCheck.h
相关文件：
实现功能：用于通道检测控制方式实现接口
作者 ：刘黎明
版权 ：<Copyright(c) 2019-2020 Suzhou Keda Technology Co.,Ltd. All right reserved.>
-----------------------------------------------------------------------
修改记录：
日 期           版本     修改人     走读人     修改记录
2019/9/26       V1.0     刘黎明     王 五      创建模块
=====================================================================*/

#ifndef LOGICCHANNELCHECK_H
#define LOGICCHANNELCHECK_H

#include "LogicCtlFixedTime.h"

class CLogicCtlChannelCheck : public CLogicCtlMode
{
public:
	CLogicCtlChannelCheck();
	virtual ~CLogicCtlChannelCheck();

	//初始化通道检测控制方式需要的参数和状态,主要用于初始化状态,参数从之前的控制方式继承而来
	virtual void Init(COpenATCParameter * pParameter, COpenATCRunStatus * pRunStatus, COpenATCLog * pOpenATCLog, int nPlanNo);

	//通道检测控制方式主流程
	virtual void Run();

	
protected:



	
};

#endif // ifndef LOGICCHANNELCHECK_H
