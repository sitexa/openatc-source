/*=====================================================================
模块名 ：通道检测控制方式实现模块
文件名 ：LogicCtlChannelCheck.cpp
相关文件：LogicCtlChannelCheck.h
实现功能：通道检测接口实现
作者 ：刘黎明
版权 ：<Copyright(c) 2019-2020 Suzhou Keda Technology Co.,Ltd. All right reserved.>
-----------------------------------------------------------------------
修改记录：
日 期           版本     修改人     走读人     修改记录
2019/9/14       V1.0     刘黎明     王 五      创建模块
=====================================================================*/

#include "LogicCtlChannelCheck.h"
#include <memory.h> 

CLogicCtlChannelCheck::CLogicCtlChannelCheck()
{

}

CLogicCtlChannelCheck::~CLogicCtlChannelCheck()
{

}

/*====================================================================
函数名 ：Init
功能 ：关灯控制方式类资源初始化
算法实现 ：
参数说明 ：pParameter，特征参数指针
           pRunStatus，全局运行状态类指针
		   pOpenATCLog，日志指针
           nPlanNo，使用的方案号，0表示根据时段取方案 
返回值说明：
----------------------------------------------------------------------
修改记录 ：
日 期          版本 修改人  走读人  修改记录
2019/09/14     V1.0 刘黎明          创建函数
====================================================================*/
void CLogicCtlChannelCheck::Init(COpenATCParameter * pParameter, COpenATCRunStatus * pRunStatus, COpenATCLog * pOpenATCLog, int nPlanNo)
{
	m_pOpenATCParameter = pParameter;
	m_pOpenATCRunStatus = pRunStatus;

	TLogicCtlStatus tCtlStatus;
	m_pOpenATCRunStatus->GetLogicCtlStatus(tCtlStatus);
	tCtlStatus.m_nCurCtlMode = CTL_MODE_CHANNEL_CHECK;
	tCtlStatus.m_nCurPlanNo = 0;//通道检测时返回的方案号是0
	m_pOpenATCRunStatus->SetLogicCtlStatus(tCtlStatus);

	memset(m_achLampClr, LAMP_CLR_UNDEF, sizeof(m_achLampClr));

	memset(m_bKeepGreenChannelBeforeControlChannelFlag, false, sizeof(m_bKeepGreenChannelBeforeControlChannelFlag));
}

/*====================================================================
函数名 ：Run
功能 ：关灯控制方式主流程
算法实现 ：
参数说明 ：
返回值说明：
----------------------------------------------------------------------
修改记录 ：
日 期          版本 修改人  走读人  修改记录
2019/09/14     V1.0 刘黎明          创建函数
====================================================================*/
void CLogicCtlChannelCheck::Run()
{
	//获取的通道检测数据
	TAscChannelVerifyInfo	atChannelCheckInfo;	
	memset(&atChannelCheckInfo, 0, sizeof(atChannelCheckInfo));
	m_pOpenATCRunStatus->GetChannelCheckInfo(atChannelCheckInfo);
		
	//获取灯色状态信息
	bool bRefresh = false;
	char achLampClr[C_N_MAXLAMPBOARD_NUM * C_N_LAMPBORAD_OUTPUTNUM];
	memcpy(&achLampClr, atChannelCheckInfo.m_achLampClr, sizeof(achLampClr));

	//关灯指令
	char achLampOff[C_N_MAXLAMPBOARD_NUM * C_N_LAMPBORAD_OUTPUTNUM];
	memset(achLampOff, LAMP_CLR_OFF, sizeof(achLampOff));

	if (memcmp(m_achLampClr, achLampClr, sizeof(achLampClr)) != 0)
	{
		if (!bRefresh)
		{
			bRefresh = true;	//进入通道检测流程
		}
	}

	if (bRefresh)
	{
		TLampClrStatus tLampClrStatus;
		memset(&tLampClrStatus, 0, sizeof(TLampClrStatus));
		m_pOpenATCRunStatus->GetLampClrStatus(tLampClrStatus);
		if (memcmp(tLampClrStatus.m_achLampClr, achLampOff, sizeof(achLampOff)) != 0)	//如果当前状态未关灯进行关灯
		{
			memcpy(m_achLampClr, achLampOff, sizeof(achLampOff));
			SetLampClr(tLampClrStatus);
			tLampClrStatus.m_bIsRefreshClr = true;
			m_pOpenATCRunStatus->SetLampClrStatus(tLampClrStatus);
		}
		//通道检测设置灯色输出
		memcpy(m_achLampClr, achLampClr, sizeof(achLampClr));
		SetLampClr(tLampClrStatus);
		tLampClrStatus.m_bIsRefreshClr = true;
		m_pOpenATCRunStatus->SetLampClrStatus(tLampClrStatus);
	}

	if (m_bIsUsrCtl || m_bIsSystemCtl)
    {
      	TManualCmd  tValidManualCmd;
	    memset(&tValidManualCmd,0,sizeof(tValidManualCmd));
		m_pOpenATCRunStatus->GetValidManualCmd(tValidManualCmd); 

        if (tValidManualCmd.m_bPatternInterruptCmd || tValidManualCmd.m_bDirectionCmd)
        {
            SetGetParamFlag(true);

			if (m_bIsUsrCtl)
			{
				m_bIsUsrCtl = false;
			}
			if (m_bIsSystemCtl)
			{
				m_bIsSystemCtl = false;
			}
        }
    }
    else
    {
        SetGetParamFlag(true);
    } 
}
