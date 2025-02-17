/*=====================================================================
ģ���� �����Ʒ�ʽ�����๤��
�ļ��� ��LogicCtlModeSimpleFactory.cpp
����ļ���LogicCtlModeSimpleFactory.h
          LogicCtlMode.h
ʵ�ֹ��ܣ����ݿ��Ʒ�ʽ����������Ʒ�ʽʵ�ֶ���
���� ��������
��Ȩ ��<Copyright(c) 2019-2020 Suzhou Keda Technology Co.,Ltd. All right reserved.>
-----------------------------------------------------------------------
�޸ļ�¼��
�� ��           �汾     �޸���     �߶���     �޸ļ�¼
2019/9/14       V1.0     ������     ������     ����ģ��
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
������ ��Create 
���� �����ؾ�����Ʒ�ʽ����ָ��
�㷨ʵ�� �� 
����˵�� ��nCtlMode�����Ʒ�ʽ 
����ֵ˵����nCtlMode��ʾ�Ŀ��Ʒ�ʽʵ�ֶ����ָ��
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ������          �������� 
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
