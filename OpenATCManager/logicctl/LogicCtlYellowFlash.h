/*=====================================================================
模块名 ：黄闪控制方式实现模块
文件名 ：LogicCtlYellowFlash.h
相关文件：
实现功能：用于定义关灯控制模式实现接口
作者 ：刘黎明
版权 ：<Copyright(c) 2019-2020 Suzhou Keda Technology Co.,Ltd. All right reserved.>
-----------------------------------------------------------------------
修改记录：
日 期           版本     修改人     走读人     修改记录
2019/9/26       V1.0     刘黎明     刘黎明     创建模块
=====================================================================*/

#ifndef LOGICCTLYELLOWFLASH_H
#define LOGICCTLYELLOWFLASH_H

#include "LogicCtlMode.h"

/*=====================================================================
类名 ：CLogicCtlYellowFlash
功能 ：黄闪控制方式实现类
主要接口：Run
备注 ：
-----------------------------------------------------------------------
修改记录：
日 期          版本     修改人     走读人       修改记录
2019/09/14     V1.0     刘黎明     李四         创建类
=====================================================================*/
class CLogicCtlYellowFlash : public CLogicCtlMode  
{
public:
	CLogicCtlYellowFlash();
	virtual ~CLogicCtlYellowFlash();

    //初始化黄闪控制方式需要的参数
    virtual void Init(COpenATCParameter * pParameter, COpenATCRunStatus * pRunStatus, COpenATCLog * pOpenATCLog, int nPlanNo);

    //黄闪控制方式主流程
    virtual void Run();
protected:
};

#endif // !ifndef LOGICCTLYELLOWFLASH_H
