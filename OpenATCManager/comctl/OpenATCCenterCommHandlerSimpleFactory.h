/*=====================================================================
模块名 ：通信工厂模块
文件名 ：OpenATCCenterCommHandlerSimpleFactory.h
相关文件：OpenATCCenterCommHandlerBase.h
实现功能：定义通信工厂，创建通信对象
作者 ：李永萍
版权 ：<Copyright(c) 2019-2020 Suzhou Keda Technology Co.,Ltd. All right reserved.>
--------------------------------------------------------------------------------------------------------------------
修改记录：
日 期           版本    修改人             走读人             修改记录
2019/09/06      V1.0    李永萍）           李永萍             创建模块
====================================================================*/

#ifndef OPENATCCENTERCOMMHANDLERSIMPLEFACTORY__H
#define OPENATCCENTERCOMMHANDLERSIMPLEFACTORY__H

#include "OpenATCCenterCommHandlerBase.h"

/*=====================================================================
类名 ：COpenATCCenterCommHandlerSimpleFactory
功能 ：定义通信工厂，创建通信对象
主要接口：COpenATCCenterCommHandlerBase Create：创建通信对象
备注 ：
--------------------------------------------------------------------------------------------------------------------
修改记录：
日 期           版本    修改人             走读人             修改记录
2019/09/06      V1.0    李永萍             李永萍             创建类
====================================================================*/
class COpenATCCenterCommHandlerSimpleFactory
{
public:

	/****************************************************
	函数名：Create
    功能：创建通信对象
	算法实现:
    参数说明 ：无
    返回值说明：返回通信对象
    --------------------------------------------------------------------------------------------------------------------
	修改记录：
	日 期           版本    修改人             走读人             修改记录
	2019/09/06      V1.0    李永萍             李永萍             创建
	====================================================================*/
    static COpenATCCenterCommHandlerBase* Create(COpenATCParameter *pParameter, COpenATCRunStatus *pRunStatus,COpenATCLog * pOpenATCLog, const char * pOpenATCVersion);
};

#endif// !ifndef OPENATCCENTERCOMMHANDLERSIMPLEFACTORY__H



