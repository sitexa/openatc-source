/*=====================================================================
模块名 ：全红控制方式实现模块
文件名 ：LogicCtlAllRed.h
相关文件：
实现功能：用于定义全红控制模式实现接口
作者 ：刘黎明
版权 ：<Copyright(c) 2019-2020 Suzhou Keda Technology Co.,Ltd. All right reserved.>
-----------------------------------------------------------------------
修改记录：
日 期           版本     修改人     走读人     修改记录
2019/9/26       V1.0     刘黎明     刘黎明     创建模块
=====================================================================*/

#ifndef LOGICCTLALLRED_H
#define LOGICCTLALLRED_H

#include "LogicCtlMode.h"

class CLogicCtlAllRed : public CLogicCtlMode 
{
public:
	CLogicCtlAllRed();
	virtual ~CLogicCtlAllRed();

    //初始化全红控制方式需要的参数
    virtual void Init(COpenATCParameter * pParameter, COpenATCRunStatus * pRunStatus, COpenATCLog * pOpenATCLog, int nPlanNo);

    //全红控制方式主流程
    virtual void Run();


};

#endif // !ifndef LOGICCTLALLRED_H
