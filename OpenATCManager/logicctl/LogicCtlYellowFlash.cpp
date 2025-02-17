/*=====================================================================
模块名 ：黄闪控制方式实现模块
文件名 ：LogicCtlYellowFlash.cpp
相关文件：LogicCtlYellowFlash.h
实现功能：黄闪控制方式接口实现
作者 ：刘黎明
版权 ：<Copyright(c) 2019-2020 Suzhou Keda Technology Co.,Ltd. All right reserved.>
-----------------------------------------------------------------------
修改记录：
日 期           版本     修改人     走读人     修改记录
2019/9/14       V1.0     刘黎明     刘黎明     创建模块
=====================================================================*/

#include "LogicCtlYellowFlash.h"
#include <memory.h> 

CLogicCtlYellowFlash::CLogicCtlYellowFlash()
{

}

CLogicCtlYellowFlash::~CLogicCtlYellowFlash()
{

}

/*==================================================================== 
函数名 ：Init 
功能 ：黄闪控制方式类资源初始化
算法实现 ： 
参数说明 ：pParameter，特征参数指针
           pRunStatus，全局运行状态类指针
		   pOpenATCLog，日志指针
           nPlanNo，指定的方案号,0表示使用时段对应的方案  
返回值说明：
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 刘黎明          创建函数 
====================================================================*/ 
void CLogicCtlYellowFlash::Init(COpenATCParameter * pParameter, COpenATCRunStatus * pRunStatus, COpenATCLog * pOpenATCLog, int nPlanNo)
{
    m_pOpenATCParameter = pParameter;
    m_pOpenATCRunStatus = pRunStatus;
	m_pOpenATCLog       = pOpenATCLog;

	memset(&m_tRunStageInfo, 0x00, sizeof(m_tRunStageInfo));
    memset(m_nChannelSplitMode,0x00,sizeof(m_nChannelSplitMode)); 

    //当前运行模式为黄闪
    m_nCurRunMode = CTL_MODE_FLASH;

    TLogicCtlStatus tCtlStatus;
    m_pOpenATCRunStatus->GetLogicCtlStatus(tCtlStatus);
    tCtlStatus.m_nCurCtlMode = CTL_MODE_FLASH;
    tCtlStatus.m_nCurPlanNo = 0;//黄闪时返回的方案号是0
    m_pOpenATCRunStatus->SetLogicCtlStatus(tCtlStatus); 

    InitChannelParam();  

    memset(m_achLampClr,LAMP_CLR_UNDEF,sizeof(m_achLampClr)); 
}

/*==================================================================== 
函数名 ：Run 
功能 ：黄闪控制方式主流程
算法实现 ： 
参数说明 ：
返回值说明：
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 刘黎明          创建函数 
====================================================================*/ 
void CLogicCtlYellowFlash::Run()
{
    bool bRefresh = false;
    char achLampClr[C_N_MAXLAMPBOARD_NUM * C_N_LAMPBORAD_OUTPUTNUM];

    memset(achLampClr,LAMP_CLR_OFF,sizeof(achLampClr)); 

    char * pStartPos = NULL;
    for (int i = 0;i < m_nChannelCount;i++)
    {
		if (m_atChannelInfo[i].m_byChannelNumber == 0)
		{
			continue;
		}

        pStartPos = achLampClr + (m_atChannelInfo[i].m_byChannelNumber - 1) * 3;
        if (m_atChannelInfo[i].m_byChannelControlType != PED_CHA && m_atChannelInfo[i].m_byChannelControlType != OVERLAP_PED_CHA)
        {
            SetOneChannelOutput(pStartPos,C_CH_PHASESTAGE_YF);
        }
    }   

    if (memcmp(m_achLampClr,achLampClr,sizeof(achLampClr)) != 0)
    {
        if (!bRefresh)
        {
            bRefresh = true;
        }
    }
    
    if (bRefresh)
    {
        memcpy(m_achLampClr,achLampClr,sizeof(achLampClr));

        //设置灯色
        TLampClrStatus tLampClrStatus;
        memset(&tLampClrStatus,0,sizeof(TLampClrStatus)); 
        m_pOpenATCRunStatus->GetLampClrStatus(tLampClrStatus);
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
