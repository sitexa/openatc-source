/*=====================================================================
ģ���� �����Ʒ�ʽ�ӿ�ģ��
�ļ��� ��LogicCtlMode.cpp
����ļ���LogicCtlMode.h
ʵ�ֹ��ܣ����Ʒ�ʽ�ӿ�ʵ��
���� ��������
��Ȩ ��<Copyright(c) 2019-2020 Suzhou Keda Technology Co.,Ltd. All right reserved.>
-----------------------------------------------------------------------
�޸ļ�¼��
�� ��           �汾     �޸���     �߶���     �޸ļ�¼
2019/9/14       V1.0     ������     ������     ����ģ��
2020/1/25       V1.0     ����Ƽ     ����Ƽ     �����ֶ���尴ť�л�ʱ������ͻ�ȡ���������ӿ�
2020/2/25       V1.0     ����Ƽ     ����Ƽ     �����жϷ������ʱ���Ƿ�����ĺ����͵�ǰ�׶ζ�Ӧ����λ�Ƿ����н����ĺ���
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
������ ��Init 
���� �����Ʒ�ʽ������Դ��ʼ��
�㷨ʵ�� �� 
����˵�� ��pParameter����������ָ��
           pRunStatus��ȫ������״̬��ָ��
		   pOpenATCLog����־ָ��
           nPlanNo��ָ���ķ�����,0��ʾʹ��ʱ�ζ�Ӧ�ķ��� 
����ֵ˵����
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ������          �������� 
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
������ ��SetLampClr
���� �����ݵ����������Ϣ��������ȫ�ֵ�ɫ״̬�ṹ��
�㷨ʵ�� �� 
����˵�� ��tLampClr��ȫ�ֵ�ɫ�ṹ������
����ֵ˵����
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ������          �������� 
====================================================================*/ 
void CLogicCtlMode::SetLampClr(TLampClrStatus & tLampClr)
{
    memcpy(tLampClr.m_achLampClr,m_achLampClr,sizeof(m_achLampClr)); 
}

/*==================================================================== 
������ ��SetGetParamFlag
���� ����ÿ������ѭ��������ʱ������ȫ��ȡ����״̬����
�㷨ʵ�� �� 
����˵�� ��bFlag��ȫ��ȡ����״ֵ̬
����ֵ˵����
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ������          �������� 
====================================================================*/ 
void CLogicCtlMode::SetGetParamFlag(bool bFlag)
{
    TMainCtlBoardRunStatus tRunStatus;
    m_pOpenATCRunStatus->GetMainCtlBoardRunStatus(tRunStatus);
    tRunStatus.m_bIsNeedGetParam = bFlag;
    m_pOpenATCRunStatus->SetMainCtlBoardRunStatus(tRunStatus);
}

/*==================================================================== 
������ ��GetCurCtlParam
���� ����ȡ��ǰ���Ʒ�ʽʹ�õĿ��Ʋ���
�㷨ʵ�� �� 
����˵�� ��
����ֵ˵����
        ����ָ�롣
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ������          �������� 
====================================================================*/ 
void * CLogicCtlMode::GetCurCtlParam()
{
    return NULL;
}

/*==================================================================== 
������ ��GetCurCtlParam
���� ����ȡ��ǰ���Ʒ�ʽʹ�õĿ��Ʋ���
�㷨ʵ�� �� 
����˵�� ��
����ֵ˵����
        ����ָ�롣
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ������          �������� 
====================================================================*/ 
void * CLogicCtlMode::GetCurLampClr()
{
    return m_achLampClr;
}

/*==================================================================== 
������ ��GetCurCtlParam
���� ����ȡ��ǰ���Ʒ�ʽʹ�õĿ��Ʋ���
�㷨ʵ�� �� 
����˵�� ��
����ֵ˵����
        ����ָ�롣
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ����Ƽ          �������� 
====================================================================*/ 
void * CLogicCtlMode::GetCurStageTable()
{
    return &m_tRunStageInfo;
}

/*==================================================================== 
������ ��GetCurChannelSplitMode
���� ����ȡ��ǰͨ�������ű�ģʽ
�㷨ʵ�� �� 
����˵�� ��
����ֵ˵����
        ����ָ�롣
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ����Ƽ          �������� 
====================================================================*/ 
void * CLogicCtlMode::GetCurChannelSplitMode()
{
    return m_nChannelSplitMode;
}

/*==================================================================== 
������ ��SetCtlDerivedParam
���� �����ÿ��Ʒ�ʽʹ�õĿ��Ʋ���,�����ֶ�����ʱ�Ĳ����̳�
�㷨ʵ�� �� 
����˵�� ��
        pParam���̳еĲ���ָ��.
����ֵ˵����
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ������          �������� 
====================================================================*/ 
void CLogicCtlMode::SetCtlDerivedParam(void * pParam,void * pLampClr,void * pStageTable,void * pChannelSplitMode)
{
    return;
}

/*==================================================================== 
������ ��SetUsrCtlFlag
���� �������û����Ʊ�־
�㷨ʵ�� �� 
����˵�� ��
        bFlag���û���Ԥ��־.
����ֵ˵����
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ������          �������� 
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
������ ��SetSystemUsrCtlFlag
���� ������ϵͳ�û����Ʊ�־
�㷨ʵ�� �� 
����˵�� ��
        bFlag��ϵͳ�û���Ԥ��־.
����ֵ˵����
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ������          �������� 
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
������ ��SetPreemptCtlFlag
���� ������ϵͳ�û����Ʊ�־
�㷨ʵ�� �� 
����˵�� ��
        bFlag��ϵͳ�û���Ԥ��־.
����ֵ˵����
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ������          �������� 
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
������ ��InitChannelParam
���� ����ʼ��ͨ������
�㷨ʵ�� �� 
����˵�� �� 
����ֵ˵����
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ������          �������� 
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
������ ��SetOneChannelOutput
���� ������ͨ����������ӵ�״̬
�㷨ʵ�� �� 
����˵�� ��pStart��ͨ���������״̬������׵�ַ
           chStage��ͨ�����״̬
����ֵ˵����
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ������          �������� 
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
������ ��GetPhaseRunStatus
���� �����ݿ��Ʒ�ʽ������ȫ������״̬
�㷨ʵ�� �� 
����˵�� ��tRunStatus������״̬�ṹ������
����ֵ˵����
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ������          �������� 
====================================================================*/ 
void CLogicCtlMode::GetPhaseRunStatus(TPhaseRunStatus & tRunStatus)
{
    //����ģʽ
    tRunStatus.m_nCurCtlPattern = 1; 

    TLogicCtlStatus tCtlStatus;
    m_pOpenATCRunStatus->GetLogicCtlStatus(tCtlStatus);
    tRunStatus.m_nCurCtlMode = tCtlStatus.m_nCurCtlMode;
}

/*==================================================================== 
������ ��GetLedScreenShowInfo
���� ��������ʾ����ʾ��Ϣ״̬
�㷨ʵ�� �� 
����˵�� ��tLedScreenShowInfo����ʾ����Ϣ
����ֵ˵������
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ����Ƽ          �������� 
====================================================================*/ 
void CLogicCtlMode::GetLedScreenShowInfo(TLedScreenShowInfo & tLedScreenShowInfo)
{
    return;    
}


/*==================================================================== 
������ ��IsChannelGreen
���� ���жϵ�ǰͨ���ĵ�ɫ�Ƿ�����ɫ
�㷨ʵ�� �� 
����˵�� ��byChannelType��ͨ�����ͣ�nDirectionIndex�������ţ�nChannelLockStatus������ͨ����ɫ
����ֵ˵������
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ����Ƽ          �������� 
====================================================================*/ 
bool CLogicCtlMode::IsChannelGreen(BYTE byChannelType, int nDirectionIndex, int nChannelLockStatus[])
{
	return true;
}

/*==================================================================== 
������ ��CalcCounter
���� ���������
�㷨ʵ�� �� 
����˵�� ��nStart����ʼ����
           nEnd����������
		   nMax��ȫ�ּ������ֵ
����ֵ˵����������
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ����Ƽ          �������� 
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
������ ��OpenATCGetCurTime
���� ����ȡ��ǰʱ��
�㷨ʵ�� �� 
����˵�� ����ǰʱ��
����ֵ˵������
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ����Ƽ          �������� 
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
������ ��GetTransRunStatusBeforeLockPhase
���� �������е�����֮ǰ�Ĺ�������״̬
�㷨ʵ�� �� 
����˵�� ��tRunStatus����λ������״̬
����ֵ˵������
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ����Ƽ          �������� 
====================================================================*/
void CLogicCtlMode::GetTransRunStatusBeforeLockPhase(TPhaseRunStatus & tRunStatus)
{

}

/*==================================================================== 
������ ��GetLockPhaseRunStatus
���� ������������λ������״̬
�㷨ʵ�� �� 
����˵�� ��tRunStatus����λ������״̬
����ֵ˵������
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ����Ƽ          �������� 
====================================================================*/
void CLogicCtlMode::GetLockPhaseRunStatus(TPhaseRunStatus & tRunStatus, bool bInvalidPhaseCmd, TPhaseLockPara tInvalidPhaseLockPara)
{

}

/*==================================================================== 
������ ��GetRunStageTable
���� ����ȡ�׶α�
�㷨ʵ�� �� 
����˵�� ����
����ֵ˵������
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ����Ƽ          �������� 
====================================================================*/
TRunStageInfo CLogicCtlMode::GetRunStageTable()
{
	return m_tRunStageInfo;
}

#ifdef VIRTUAL_DEVICE
/*====================================================================
������ ��GetVirtualTimeByGlobalCount
���� ����������ʱ����ȫ�ּ����������ǰ��ʱ��
�㷨ʵ�� ��
����˵�� ����ǰʱ��
����ֵ˵������
----------------------------------------------------------------------
�޸ļ�¼ ��
�� ��          �汾 �޸���  �߶���  �޸ļ�¼
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