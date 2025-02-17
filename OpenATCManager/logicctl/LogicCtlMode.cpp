/*=====================================================================
模块名 ：控制方式接口模块
文件名 ：LogicCtlMode.cpp
相关文件：LogicCtlMode.h
实现功能：控制方式接口实现
作者 ：刘黎明
版权 ：<Copyright(c) 2019-2020 Suzhou Keda Technology Co.,Ltd. All right reserved.>
-----------------------------------------------------------------------
修改记录：
日 期           版本     修改人     走读人     修改记录
2019/9/14       V1.0     刘黎明     刘黎明     创建模块
2020/1/25       V1.0     李永萍     李永萍     增加手动面板按钮切换时，保存和获取特征参数接口
2020/2/25       V1.0     李永萍     李永萍     增加判断方向持续时间是否结束的函数和当前阶段对应的相位是否运行结束的函数
=====================================================================*/

#include "LogicCtlMode.h"
#include <memory.h> 
#ifdef _WIN32
#else
#include <stdlib.h>
#endif

CLogicCtlMode::CLogicCtlMode()
{
    m_bIsLampClrChg = false;
    m_bIsUsrCtl = false;
    m_bIsSystemCtl = false;    
	m_bIsPreemptCtl = false;
}

CLogicCtlMode::~CLogicCtlMode()
{

}

/*==================================================================== 
函数名 ：Init 
功能 ：控制方式基类资源初始化
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
void CLogicCtlMode::Init(COpenATCParameter * pParameter, COpenATCRunStatus * pRunStatus, COpenATCLog * pOpenATCLog, int nPlanNo)
{
    m_pOpenATCParameter = pParameter;
    m_pOpenATCRunStatus = pRunStatus;
	m_pOpenATCLog       = pOpenATCLog;

    memset(m_achLampClr,LAMP_CLR_UNDEF,sizeof(m_achLampClr));  
    memset(&m_tRunStageInfo, 0x00, sizeof(m_tRunStageInfo)); 
    memset(m_nChannelSplitMode,0x00,sizeof(m_nChannelSplitMode)); 
    memset(m_bShieldStatus, 0x00, sizeof(m_bShieldStatus));
    memset(m_bProhibitStatus, 0x00, sizeof(m_bProhibitStatus));
    memset(m_bOldShieldStatus, 0x00, sizeof(m_bOldShieldStatus));
    memset(m_bOldProhibitStatus, 0x00, sizeof(m_bOldProhibitStatus));

	memset(m_bKeepGreenChannelBeforeControlChannelFlag, false, sizeof(m_bKeepGreenChannelBeforeControlChannelFlag));
}

void CLogicCtlMode::Release()
{

}

/*==================================================================== 
函数名 ：SetLampClr
功能 ：根据灯输出端子信息数组设置全局灯色状态结构体
算法实现 ： 
参数说明 ：tLampClr，全局灯色结构体引用
返回值说明：
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 刘黎明          创建函数 
====================================================================*/ 
void CLogicCtlMode::SetLampClr(TLampClrStatus & tLampClr)
{
    memcpy(tLampClr.m_achLampClr,m_achLampClr,sizeof(m_achLampClr)); 
}

/*==================================================================== 
函数名 ：SetGetParamFlag
功能 ：在每个控制循环结束的时候设置全局取参数状态变量
算法实现 ： 
参数说明 ：bFlag，全局取参数状态值
返回值说明：
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 刘黎明          创建函数 
====================================================================*/ 
void CLogicCtlMode::SetGetParamFlag(bool bFlag)
{
    TMainCtlBoardRunStatus tRunStatus;
    m_pOpenATCRunStatus->GetMainCtlBoardRunStatus(tRunStatus);
    tRunStatus.m_bIsNeedGetParam = bFlag;
    m_pOpenATCRunStatus->SetMainCtlBoardRunStatus(tRunStatus);
}

/*==================================================================== 
函数名 ：GetCurCtlParam
功能 ：获取当前控制方式使用的控制参数
算法实现 ： 
参数说明 ：
返回值说明：
        参数指针。
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 刘黎明          创建函数 
====================================================================*/ 
void * CLogicCtlMode::GetCurCtlParam()
{
    return NULL;
}

/*==================================================================== 
函数名 ：GetCurCtlParam
功能 ：获取当前控制方式使用的控制参数
算法实现 ： 
参数说明 ：
返回值说明：
        参数指针。
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 刘黎明          创建函数 
====================================================================*/ 
void * CLogicCtlMode::GetCurLampClr()
{
    return m_achLampClr;
}

/*==================================================================== 
函数名 ：GetCurCtlParam
功能 ：获取当前控制方式使用的控制参数
算法实现 ： 
参数说明 ：
返回值说明：
        参数指针。
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 李永萍          创建函数 
====================================================================*/ 
void * CLogicCtlMode::GetCurStageTable()
{
    return &m_tRunStageInfo;
}

/*==================================================================== 
函数名 ：GetCurChannelSplitMode
功能 ：获取当前通道的绿信比模式
算法实现 ： 
参数说明 ：
返回值说明：
        参数指针。
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 李永萍          创建函数 
====================================================================*/ 
void * CLogicCtlMode::GetCurChannelSplitMode()
{
    return m_nChannelSplitMode;
}

/*==================================================================== 
函数名 ：SetCtlDerivedParam
功能 ：设置控制方式使用的控制参数,用于手动控制时的参数继承
算法实现 ： 
参数说明 ：
        pParam：继承的参数指针.
返回值说明：
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 刘黎明          创建函数 
====================================================================*/ 
void CLogicCtlMode::SetCtlDerivedParam(void * pParam,void * pLampClr,void * pStageTable,void * pChannelSplitMode)
{
    return;
}

/*==================================================================== 
函数名 ：SetUsrCtlFlag
功能 ：设置用户控制标志
算法实现 ： 
参数说明 ：
        bFlag：用户干预标志.
返回值说明：
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 刘黎明          创建函数 
====================================================================*/ 
void CLogicCtlMode::SetUsrCtlFlag(bool bFlag)
{
    m_bIsUsrCtl = bFlag;
	if (bFlag)
	{
		m_bIsSystemCtl = false;
		m_bIsPreemptCtl = false;
	}
}

/*==================================================================== 
函数名 ：SetSystemUsrCtlFlag
功能 ：设置系统用户控制标志
算法实现 ： 
参数说明 ：
        bFlag：系统用户干预标志.
返回值说明：
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 刘黎明          创建函数 
====================================================================*/ 
void CLogicCtlMode::SetSystemUsrCtlFlag(bool bFlag)
{
    m_bIsSystemCtl = bFlag;
	if (bFlag)
	{
		m_bIsUsrCtl = false;
		m_bIsPreemptCtl = false;
	}
}

/*==================================================================== 
函数名 ：SetPreemptCtlFlag
功能 ：设置系统用户控制标志
算法实现 ： 
参数说明 ：
        bFlag：系统用户干预标志.
返回值说明：
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 刘黎明          创建函数 
====================================================================*/ 
void CLogicCtlMode::SetPreemptCtlFlag(bool bFlag)
{
    m_bIsPreemptCtl = bFlag;
	if (bFlag)
	{
		m_bIsUsrCtl = false;
		m_bIsSystemCtl = false;
	}
}

/*==================================================================== 
函数名 ：InitChannelParam
功能 ：初始化通道参数
算法实现 ： 
参数说明 ： 
返回值说明：
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 刘黎明          创建函数 
====================================================================*/ 
void CLogicCtlMode::InitChannelParam()
{
	TChannel atChannelInfo[MAX_CHANNEL_COUNT]; 
    memset(atChannelInfo, 0, sizeof(atChannelInfo));
	m_pOpenATCParameter->GetChannelTable(atChannelInfo);

    m_nChannelCount = 0;
    memset(m_atChannelInfo,0,sizeof(m_atChannelInfo));

	int  i = 0, j = 0, nMaxChannelID = 0;
	for (i = 0;i < MAX_CHANNEL_COUNT;i++)
	{
		m_atChannelInfo[i].m_byChannelNumber = i + 1;
	}

	for (i = 0;i < MAX_CHANNEL_COUNT;i++)
	{
		for (j = 0;j < MAX_CHANNEL_COUNT;j++)
		{
			if (atChannelInfo[j].m_byChannelNumber == 0)
			{
				continue;
			}

			if (m_atChannelInfo[i].m_byChannelNumber == atChannelInfo[j].m_byChannelNumber)
			{
				memcpy(&m_atChannelInfo[i], &atChannelInfo[j], sizeof(TChannel));
			}
		}

		if (m_atChannelInfo[i].m_byChannelControlSource == 0)
		{
			m_atChannelInfo[i].m_byChannelNumber = 0;
		}

		if (atChannelInfo[i].m_byChannelNumber > nMaxChannelID)
		{
			nMaxChannelID = atChannelInfo[i].m_byChannelNumber;
		}
	}

	m_nChannelCount = nMaxChannelID;
}

/*==================================================================== 
函数名 ：SetOneChannelOutput
功能 ：设置通道的输出端子的状态
算法实现 ： 
参数说明 ：pStart，通道输出端子状态数组的首地址
           chStage，通道输出状态
返回值说明：
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 刘黎明          创建函数 
====================================================================*/ 
void CLogicCtlMode::SetOneChannelOutput(char * pStart,char chStage)
{
    if (chStage == C_CH_PHASESTAGE_G)
    {
        *pStart = (char)LAMP_CLR_OFF;
        *(pStart+1) = (char)LAMP_CLR_OFF;
        *(pStart+2) = (char)LAMP_CLR_ON; 
    }
    else if (chStage == C_CH_PHASESTAGE_GF)
    {
        *pStart = (char)LAMP_CLR_OFF;
        *(pStart+1) = (char)LAMP_CLR_OFF;
        *(pStart+2) = (char)LAMP_CLR_FLASH; 
    }
    else if (chStage == C_CH_PHASESTAGE_Y)
    {
        *pStart = (char)LAMP_CLR_OFF;
        *(pStart+1) = (char)LAMP_CLR_ON;
        *(pStart+2) = (char)LAMP_CLR_OFF; 
    }
    else if (chStage == C_CH_PHASESTAGE_YF)
    {
        *pStart = (char)LAMP_CLR_OFF;
        *(pStart+1) = (char)LAMP_CLR_FLASH;
        *(pStart+2) = (char)LAMP_CLR_OFF; 
    }
    else if (chStage == C_CH_PHASESTAGE_R)
    {
        *pStart = (char)LAMP_CLR_ON;
        *(pStart+1) = (char)LAMP_CLR_OFF;
        *(pStart+2) = (char)LAMP_CLR_OFF; 
    }
    else if (chStage == C_CH_PHASESTAGE_RF)
    {
        *pStart = (char)LAMP_CLR_FLASH;
        *(pStart+1) = (char)LAMP_CLR_OFF;
        *(pStart+2) = (char)LAMP_CLR_OFF; 
    }
    else if (chStage == C_CH_PHASESTAGE_U)
    {
        *pStart = (char)LAMP_CLR_ON;
        *(pStart+1) = (char)LAMP_CLR_OFF;
        *(pStart+2) = (char)LAMP_CLR_OFF; 
    }
    else if (chStage == C_CH_PHASESTAGE_F)
    {
        *pStart = (char)LAMP_CLR_ON;
        *(pStart+1) = (char)LAMP_CLR_OFF;
        *(pStart+2) = (char)LAMP_CLR_OFF; 
    } 
    else if (chStage == C_CH_PHASESTAGE_OF)
    {
        *pStart = (char)LAMP_CLR_OFF;
        *(pStart+1) = (char)LAMP_CLR_OFF;
        *(pStart+2) = (char)LAMP_CLR_OFF; 
    }   
}

/*==================================================================== 
函数名 ：GetPhaseRunStatus
功能 ：根据控制方式来设置全局运行状态
算法实现 ： 
参数说明 ：tRunStatus，运行状态结构体引用
返回值说明：
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 刘黎明          创建函数 
====================================================================*/ 
void CLogicCtlMode::GetPhaseRunStatus(TPhaseRunStatus & tRunStatus)
{
    //自主模式
    tRunStatus.m_nCurCtlPattern = 1; 

    TLogicCtlStatus tCtlStatus;
    m_pOpenATCRunStatus->GetLogicCtlStatus(tCtlStatus);
    tRunStatus.m_nCurCtlMode = tCtlStatus.m_nCurCtlMode;
}

/*==================================================================== 
函数名 ：GetLedScreenShowInfo
功能 ：设置显示屏显示信息状态
算法实现 ： 
参数说明 ：tLedScreenShowInfo，显示屏信息
返回值说明：无
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 李永萍          创建函数 
====================================================================*/ 
void CLogicCtlMode::GetLedScreenShowInfo(TLedScreenShowInfo & tLedScreenShowInfo)
{
    return;    
}


/*==================================================================== 
函数名 ：IsChannelGreen
功能 ：判断当前通道的灯色是否是绿色
算法实现 ： 
参数说明 ：byChannelType，通道类型，nDirectionIndex，方向编号，nChannelLockStatus，锁定通道灯色
返回值说明：无
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 李永萍          创建函数 
====================================================================*/ 
bool CLogicCtlMode::IsChannelGreen(BYTE byChannelType, int nDirectionIndex, int nChannelLockStatus[])
{
	return true;
}

/*==================================================================== 
函数名 ：CalcCounter
功能 ：计算计数
算法实现 ： 
参数说明 ：nStart，起始计数
           nEnd，结束计数
		   nMax，全局计数最大值
返回值说明：计数差
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 李永萍          创建函数 
====================================================================*/ 
unsigned long CLogicCtlMode::CalcCounter(unsigned long nStart,unsigned long nEnd,unsigned long nMax)
{
    if (nEnd >= nStart)
    {
        return (nEnd - nStart);
    }
    else
    {
        return (nEnd + nMax - nStart);
    }
}

/*==================================================================== 
函数名 ：OpenATCGetCurTime
功能 ：获取当前时间
算法实现 ： 
参数说明 ：当前时间
返回值说明：无
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 李永萍          创建函数 
====================================================================*/ 
void CLogicCtlMode::OpenATCGetCurTime(int & nYear, int & nMon, int & nDay, int & nHour, int & nMin, int & nSec, int & nWeek)
{
#ifdef _WIN32
    SYSTEMTIME		stLocal;
    GetLocalTime(&stLocal);
    nYear = stLocal.wYear;
    nMon  = stLocal.wMonth;
    nDay  = stLocal.wDay;
    nWeek = stLocal.wDayOfWeek;
    nHour = stLocal.wHour;
    nMin  = stLocal.wMinute;
    nSec  = stLocal.wSecond;
#else
    long dwCurTime = time(NULL);
    struct tm tLocalTime = {0};
    localtime_r(&dwCurTime, &tLocalTime);
    nYear = 1900 + tLocalTime.tm_year;
    nMon  = 1 + tLocalTime.tm_mon;
    nDay  = tLocalTime.tm_mday;
    nWeek = tLocalTime.tm_wday;
    nHour = tLocalTime.tm_hour;
    nMin  = tLocalTime.tm_min;
    nSec  = tLocalTime.tm_sec;
#endif
}

/*==================================================================== 
函数名 ：GetTransRunStatusBeforeLockPhase
功能 ：设置切到锁定之前的过渡运行状态
算法实现 ： 
参数说明 ：tRunStatus，相位的运行状态
返回值说明：无
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 李永萍          创建函数 
====================================================================*/
void CLogicCtlMode::GetTransRunStatusBeforeLockPhase(TPhaseRunStatus & tRunStatus)
{

}

/*==================================================================== 
函数名 ：GetLockPhaseRunStatus
功能 ：设置锁定相位的运行状态
算法实现 ： 
参数说明 ：tRunStatus，相位的运行状态
返回值说明：无
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 李永萍          创建函数 
====================================================================*/
void CLogicCtlMode::GetLockPhaseRunStatus(TPhaseRunStatus & tRunStatus, bool bInvalidPhaseCmd, TPhaseLockPara tInvalidPhaseLockPara)
{

}

/*==================================================================== 
函数名 ：GetRunStageTable
功能 ：获取阶段表
算法实现 ： 
参数说明 ：无
返回值说明：无
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 李永萍          创建函数 
====================================================================*/
TRunStageInfo CLogicCtlMode::GetRunStageTable()
{
	return m_tRunStageInfo;
}

#ifdef VIRTUAL_DEVICE
/*====================================================================
函数名 ：GetVirtualTimeByGlobalCount
功能 ：倍速运行时根据全局计数推算出当前的时间
算法实现 ：
参数说明 ：当前时间
返回值说明：无
----------------------------------------------------------------------
修改记录 ：
日 期          版本 修改人  走读人  修改记录
2022/15/18
====================================================================*/
void CLogicCtlMode::GetVirtualTimeByGlobalCount(int& nYear, int& nMon, int& nDay, int& nHour, int& nMin, int& nSec, int& nWeek)
{
    unsigned long nCounterDiff = 0;
    int diffTime = 0;
    int tSpeedyRunInfo = 0;
    int monthDays[13][2] = { {0,0},{31,31},{28,29},{31,31},{30,30},{31,31},{30,30},{31,31},{31,31},{30,30},{31,31},{30,30},{31,31} };
    unsigned long nGlobalCount;
    TVirtualRunTime mVirtualTimeData;
    memset(&mVirtualTimeData, 0, sizeof(mVirtualTimeData));
    tSpeedyRunInfo = m_pOpenATCParameter->GetSpeedyRunInfo();
    m_pOpenATCRunStatus->GetVirtualTimeData(mVirtualTimeData);
    nGlobalCount = m_pOpenATCRunStatus->GetGlobalCounter();
    switch (tSpeedyRunInfo)
    {
    case 0:
        nCounterDiff = CalcCounter(mVirtualTimeData.TempGlobalCount, nGlobalCount, C_N_MAXGLOBALCOUNTER);
        break;
    case 1:
        nCounterDiff = CalcCounter(mVirtualTimeData.TempGlobalCount, nGlobalCount, C_N_MAXGLOBALCOUNTER);
        break;
    case 2:
        nCounterDiff = CalcCounter(mVirtualTimeData.TempGlobalCount, nGlobalCount, C_N_MAXGLOBALCOUNTER);
        break;
    case 3:
        nCounterDiff = CalcCounter(mVirtualTimeData.TempGlobalCount, nGlobalCount, C_N_MAXGLOBALCOUNTER);
        break;
    default:
        nCounterDiff = CalcCounter(mVirtualTimeData.TempGlobalCount, nGlobalCount, C_N_MAXGLOBALCOUNTER);
        break;
    }
    if (nCounterDiff > 20)
    {
        diffTime = nCounterDiff / 20;
        mVirtualTimeData.VirtualSec = mVirtualTimeData.VirtualSec + diffTime;
        if (mVirtualTimeData.VirtualSec > 59)
        {
            mVirtualTimeData.VirtualMin = mVirtualTimeData.VirtualSec / 60 + mVirtualTimeData.VirtualMin;
            mVirtualTimeData.VirtualSec = mVirtualTimeData.VirtualSec % 60;
        }
        if (mVirtualTimeData.VirtualMin > 59)
        {
            mVirtualTimeData.VirtualHour = mVirtualTimeData.VirtualMin / 60 + mVirtualTimeData.VirtualHour;
            mVirtualTimeData.VirtualMin = mVirtualTimeData.VirtualMin % 60;
        }
        if (mVirtualTimeData.VirtualHour > 23)
        {
            mVirtualTimeData.VirtualDay = mVirtualTimeData.VirtualHour / 24 + mVirtualTimeData.VirtualDay;
            mVirtualTimeData.VirtualHour = mVirtualTimeData.VirtualHour % 24;
            mVirtualTimeData.VirtualWeek++;
            if (mVirtualTimeData.VirtualWeek == 7)
            {
                mVirtualTimeData.VirtualWeek = 0;
            }
        }
        if (mVirtualTimeData.VirtualDay == monthDays[mVirtualTimeData.VirtualMon][m_pOpenATCParameter->isLeap(mVirtualTimeData.VirtualYear)]+1)
        {
            mVirtualTimeData.VirtualDay = 1;
            mVirtualTimeData.VirtualMon++;
        }
        if (mVirtualTimeData.VirtualMon == 13)
        {
            mVirtualTimeData.VirtualYear++;
            mVirtualTimeData.VirtualMon = 1;
        }
        mVirtualTimeData.TempGlobalCount = nGlobalCount;
        m_pOpenATCRunStatus->SetVirtualTimeData(mVirtualTimeData);
    }
    nYear = mVirtualTimeData.VirtualYear;
    nMon = mVirtualTimeData.VirtualMon;
    nDay = mVirtualTimeData.VirtualDay;
    nHour = mVirtualTimeData.VirtualHour;
    nMin = mVirtualTimeData.VirtualMin;
    nSec = mVirtualTimeData.VirtualSec;
    nWeek = mVirtualTimeData.VirtualWeek;
}
#endif // VIRTUAL_DEVICE
//Virtual_Test2022