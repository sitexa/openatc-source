/*=====================================================================
模块名 ：关灯控制方式实现模块
文件名 ：LogicCtlLampOff.h
相关文件：
实现功能：用于定义关灯控制模式实现接口
作者 ：刘黎明
版权 ：<Copyright(c) 2019-2020 Suzhou Keda Technology Co.,Ltd. All right reserved.>
-----------------------------------------------------------------------
修改记录：
日 期           版本     修改人     走读人     修改记录
2019/9/26       V1.0     刘黎明     刘黎明     创建模块
=====================================================================*/

#ifndef LOGICCTLLAMPOFF_H
#define LOGICCTLLAMPOFF_H

#include "LogicCtlMode.h"

/*=====================================================================
类名 ：CLogicCtlLampOff
功能 ：关灯控制方式实现类
主要接口：Run
备注 ：
-----------------------------------------------------------------------
修改记录：
日 期          版本     修改人     走读人       修改记录
2019/09/14     V1.0     刘黎明     刘黎明       创建类
=====================================================================*/
class CLogicCtlLampOff : public CLogicCtlMode 
{
public:
	CLogicCtlLampOff();
	virtual ~CLogicCtlLampOff();

    //初始化定周期控制方式需要的参数
    virtual void Init(COpenATCParameter * pParameter, COpenATCRunStatus * pRunStatus, COpenATCLog * pOpenATCLog, int nPlanNo);

    //关灯控制方式的主流程
    virtual void Run();

protected:

};

#endif // !ifndef LOGICCTLLAMPOFF_H
