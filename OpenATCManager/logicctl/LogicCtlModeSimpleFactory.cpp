/*=====================================================================
模块名 ：控制方式创建类工厂
文件名 ：LogicCtlModeSimpleFactory.cpp
相关文件：LogicCtlModeSimpleFactory.h
          LogicCtlMode.h
实现功能：根据控制方式创建具体控制方式实现对象
作者 ：刘黎明
版权 ：<Copyright(c) 2019-2020 Suzhou Keda Technology Co.,Ltd. All right reserved.>
-----------------------------------------------------------------------
修改记录：
日 期           版本     修改人     走读人     修改记录
2019/9/14       V1.0     刘黎明     刘黎明     创建模块
=====================================================================*/

#include "LogicCtlModeSimpleFactory.h"
#include "LogicCtlMode.h"
#include "LogicCtlFixedTime.h"
#include "LogicCtlLampOff.h"
#include "LogicCtlYellowFlash.h"
#include "LogicCtlActuate.h"
#include "LogicCtlAllRed.h"
#include "LogicCtlCablelessLine.h"
#include "LogicCtlManual.h"
#include "LogicCtlOptim.h"
#include "LogicCtlPedCrossStreet.h"
#include "LogicCtlWebsterOptim.h"
#include "LogicCtlChannelCheck.h"
#include "LogicCtlActuatePedCrossStreet.h"
#include "LogicCtlActuateAdaptive.h"
#include "LogicCtlPreempt.h"

CLogicCtlModeSimpleFactory::CLogicCtlModeSimpleFactory()
{

}

CLogicCtlModeSimpleFactory::~CLogicCtlModeSimpleFactory()
{

}

/*==================================================================== 
函数名 ：Create 
功能 ：返回具体控制方式对象指针
算法实现 ： 
参数说明 ：nCtlMode，控制方式 
返回值说明：nCtlMode表示的控制方式实现对象的指针
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 刘黎明          创建函数 
====================================================================*/ 
CLogicCtlMode * CLogicCtlModeSimpleFactory::Create(int nCtlMode)
{
    CLogicCtlMode * pBase = NULL;
    if (nCtlMode == CTL_MODE_OFF) 
    {
        pBase = new CLogicCtlLampOff();
    }
    else if (nCtlMode == CTL_MODE_FLASH)
    {
        pBase = new CLogicCtlYellowFlash();
    }
    else if (nCtlMode == CTL_MODE_ALLRED)
    {
        pBase = new CLogicCtlAllRed();
    }
    else if (nCtlMode == CTL_MODE_ACTUATE)
    {
        pBase = new CLogicCtlActuate();
    }
    else if (nCtlMode == CTL_MODE_CABLELESS)
    {
        pBase = new CLogicCtlCablelessLine();
    }
    else if (nCtlMode == CTL_MODE_MANUAL)
    {
        pBase = new CLogicCtlManual();
    }
    else if (nCtlMode == CTL_MODE_SINGLEOPTIM)
    {
        pBase = new CLogicCtlActuateAdaptive();
    }
    else if (nCtlMode == CTL_MODE_PEDCROSTREET)
    {
        pBase = new CLogicCtlPedCrossStreet();
    }
	else if (nCtlMode == CTL_MODE_CHANNEL_CHECK)
	{
		pBase = new CLogicCtlChannelCheck();
	}
	else if (nCtlMode == CTL_MODE_SYS_INTERRUPT || nCtlMode == CTL_MODE_MANUAL_CONTROL_PATTERN)
	{
		pBase = new CLogicCtlCablelessLine();
	}
	else if (nCtlMode == CTL_MODE_ACTUATE_PEDCROSTREET)
	{
		pBase = new CLogicCtlActuatePedCrossStreet();
	}
	else if (nCtlMode == CTL_MODE_PREEMPT)
	{
		pBase = new CLogicCtlPreempt();
	}
    else
    {
        pBase = new CLogicCtlFixedTime();
    }
    
    return pBase; 
}
