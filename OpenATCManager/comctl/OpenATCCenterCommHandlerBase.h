/*=====================================================================
模块名 ：和上位机的通信协议模块
文件名 ：OpenATCCenterCommHandlerBase.h
相关文件：
实现功能：信号机和上位机的通信
作者 ：李永萍
版权 ：<Copyright(c) 2019-2020 Suzhou Keda Technology Co.,Ltd. All right reserved.>
--------------------------------------------------------------------------------------------------------------------
修改记录：
日 期           版本    修改人             走读人             修改记录
2019/09/06      V1.0    李永萍）           李永萍             创建模块
====================================================================*/

#ifndef OPENATCCENTERCOMMHANDLERBASE__H
#define OPENATCCENTERCOMMHANDLERBASE__H

#include "../../Include/OpenATCParameter.h"
#include "../../Include/OpenATCRunStatus.h"

/*=====================================================================
类名 ：COpenATCCenterCommHandlerBase
功能 ：信号机和上位机的通信
主要接口：int ConnectToCenter：连接到中心 DisconnectToCenter：断开连接
备注 ：
--------------------------------------------------------------------------------------------------------------------
修改记录：
日 期           版本    修改人             走读人             修改记录
2019/09/06      V1.0    李永萍             李永萍             创建类
====================================================================*/
class COpenATCCenterCommHandlerBase
{
public:
    COpenATCCenterCommHandlerBase(){}
    virtual ~COpenATCCenterCommHandlerBase(){}

    /****************************************************
	函数名：ConnectToCenter
    功能：连接中心
	算法实现:
    参数说明 ： 无
    返回值说明：ATC_OK，连接成功
                ATC_FAIL，连接失败
    --------------------------------------------------------------------------------------------------------------------
	修改记录：
	日 期           版本    修改人             走读人             修改记录
	2019/09/06      V1.0    李永萍             李永萍             创建
	====================================================================*/
    virtual int ConnectToCenter() = 0;

	/****************************************************
	函数名：DisconnectToCenter
    功能：和中心断开
	算法实现:
    参数说明 ： 无
    返回值说明：ATC_OK，断开连接成功
                ATC_FAIL，断开连接失败
    --------------------------------------------------------------------------------------------------------------------
	修改记录：
	日 期           版本    修改人             走读人             修改记录
	2019/09/06      V1.0    李永萍             李永萍             创建
	====================================================================*/
    virtual int DisconnectToCenter() = 0;

	/****************************************************
	函数名：HandleEventFromCenter
    功能：从中心读数据
	算法实现:
    参数说明 ： 无
    返回值说明：ATC_OK，读数据成功
                ATC_FAIL，读数据失败
    --------------------------------------------------------------------------------------------------------------------
	修改记录：
	日 期           版本    修改人             走读人             修改记录
	2019/09/06      V1.0    李永萍             李永萍             创建
	====================================================================*/
    virtual int HandleEventFromCenter() = 0;

	/****************************************************
	函数名：SendEventToCenter
    功能：向中心发送数据
	算法实现:
    参数说明 ： 无
    返回值说明：ATC_OK，发送数据成功
                ATC_FAIL，发送数据失败
    --------------------------------------------------------------------------------------------------------------------
	修改记录：
	日 期           版本    修改人             走读人             修改记录
	2019/09/06      V1.0    李永萍             李永萍             创建
	====================================================================*/
	virtual int SendEventToCenter() = 0;

	/****************************************************
	函数名：SendEventToCenter
    功能：定时向中心发心跳信息并从中心读心跳返回信息
	算法实现:
    参数说明 ： 无
    返回值说明：ATC_OK，操作成功
                ATC_FAIL，操作失败
    --------------------------------------------------------------------------------------------------------------------
	修改记录：
	日 期           版本    修改人             走读人             修改记录
	2019/09/06      V1.0    李永萍             李永萍             创建
	====================================================================*/
	virtual int TimerJob(bool conStatus) = 0;

	/****************************************************
	函数名：SetComPara
    功能：设置和服务端通信类型
	算法实现:
    参数说明 ： 无
    返回值说明：nComType，通信类型
    --------------------------------------------------------------------------------------------------------------------
	修改记录：
	日 期           版本    修改人             走读人             修改记录
	2019/09/06      V1.0    李永萍             李永萍             创建
	====================================================================*/
    virtual void SetComPara(int nComType) = 0;

	/****************************************************
	函数名：IsCenterParamChg
    功能：判断通信参数是否被修改
	算法实现:
    参数说明 ： 无
    返回值说明：OPENATC_RTN_OK，参数被修改
	            OPENATC_RTN_FAILED，参数没有被修改
    --------------------------------------------------------------------------------------------------------------------
	修改记录：
	日 期           版本    修改人             走读人             修改记录
	2019/09/06      V1.0    李永萍             李永萍             创建
	====================================================================*/
    virtual int IsCenterParamChg() = 0;

protected:
	
};

#endif // !ifndef OPENATCCENTERCOMMHANDLERBASE__H

