/*====================================================================
模块名 ：通信工厂模块
文件名 ：OpenATCCenterCommHandlerSimpleFactory.cpp
相关文件：OpenATCCenterCommHandlerSimpleFactory.h
实现功能：定义通信工厂，创建通信对象
作者 ：李永萍
版权 ：<Copyright(C) 2019-2020 Suzhou Keda Technology Co., Ltd. All rights reserved.>
---------------------------------------------------------------------------------------------------------------------
修改记录：
日 期           版本    修改人             走读人             修改记录
2019/09/06      V1.0    李永萍             李永萍             创建模块
====================================================================*/

#include "OpenATCCenterCommHandlerSimpleFactory.h"
#include "OpenATCComWithControlCenterImpl.h"

COpenATCCenterCommHandlerBase* COpenATCCenterCommHandlerSimpleFactory::Create(COpenATCParameter *pParameter, COpenATCRunStatus *pRunStatus, COpenATCLog * pOpenATCLog, const char * pOpenATCVersion)
{
    COpenATCCenterCommHandlerBase* centerHandler;

	centerHandler = new COpenATCComWithControlCenterImpl(pParameter, pRunStatus, pOpenATCLog, pOpenATCVersion);

    return centerHandler;
	
}

