/*=====================================================================
ģ���� ���߼����Ʒ�ʽ����ģ��
�ļ��� ��OpenATCLogicCtlManager.cpp
����ļ���OpenATCLogicCtlManager.h
          LogicCtlMode.h
          OpenATCParameter.h
          OpenATCRunStatus.h
ʵ�ֹ��ܣ����ڼ�¼��������ģ�������״̬
���� ��������
��Ȩ ��<Copyright(c) 2019-2020 Suzhou Keda Technology Co.,Ltd. All right reserved.>
-----------------------------------------------------------------------
�޸ļ�¼��
�� ��           �汾     �޸���     �߶���     �޸ļ�¼
2019/9/14       V1.0     ������     ������      ����ģ��
=====================================================================*/

#include "OpenATCLogicCtlManager.h"

#include "../Include/OpenATCHardwareManager.h"
#include "../Include/CanBusManager.h"
#include "./logicctl/LogicCtlModeSimpleFactory.h"


COpenATCLogicCtlManager * COpenATCLogicCtlManager::s_pData;

COpenATCLogicCtlManager::COpenATCLogicCtlManager()
{
    memset(&m_tOldValidManualCmd,0,sizeof(TManualCmd)); 
    memset(&m_atNetConfig,0,sizeof(m_atNetConfig));
	memset(m_nLockChannelTransStatus,0,sizeof(m_nLockChannelTransStatus));
    memset(m_nLockChannelCounter,0, sizeof(m_nLockChannelCounter));
	memset(&m_tOldAscOnePlanChannelLockInfo,0,sizeof(m_tOldAscOnePlanChannelLockInfo));
	memset(&m_tOldChannelCheckInfo,0,sizeof(m_tOldChannelCheckInfo));
	m_nManualControlPatternStartTime = 0;
	m_nManualControlPatternDurationTime = 0;
	memset(&m_tOldPreemptCtlCmd,0,sizeof(m_tOldPreemptCtlCmd));
	m_tOldPreemptCtlCmd.m_byPreemptType = PREEMPT_TYPE_DEFAULT;
	m_bInvalidPhaseCmd = false;
	memset(&m_tInvalidPhaseLockPara, 0, sizeof(m_tInvalidPhaseLockPara));

	m_tPatternInterruptCmdTime = 0;
}

COpenATCLogicCtlManager::~COpenATCLogicCtlManager()
{

}

/*==================================================================== 
������ ��getInstance 
���� �����ص�����COpenATCMainCtlManager��ʵ��ָ�� 
�㷨ʵ�� �� 
����˵�� �� 
����ֵ˵����������COpenATCMainCtlManager��ʵ��ָ��
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ������          �������� 
====================================================================*/ 
COpenATCLogicCtlManager * COpenATCLogicCtlManager::getInstance()
{
    if (s_pData == NULL)
    {
        s_pData = new COpenATCLogicCtlManager();
    }

    return s_pData;
}

/*==================================================================== 
������ ��Init 
���� �����ڶ���COpenATCLogicCtlManager�ĳ�ʼ������ 
�㷨ʵ�� �� 
����˵�� �� pParameter������������ָ��
            pRunStatus������״̬��ָ��
			pOpenATCLog����־��ָ��
����ֵ˵������
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ������          �������� 
====================================================================*/ 
void COpenATCLogicCtlManager::Init(COpenATCParameter * pParameter,COpenATCRunStatus * pRunStatus, COpenATCLog * pOpenATCLog)
{
    if (pParameter != NULL)
    {
        m_pLogicCtlParam = pParameter;
    }

    if (pRunStatus != NULL)
    {
        m_pLogicCtlStatus = pRunStatus;
    }

	if (pOpenATCLog != NULL)
    {
        m_pOpenATCLog = pOpenATCLog;
    }

    m_nLogicCtlStage = CTL_STAGE_UNDEFINE;
    m_nCurPlanNo = 0;
    m_nCurCtlMode = CTL_MODE_UNDEFINE;
    m_bIsCtlModeChg = true;  
    m_pLogicCtlMode = NULL; 
    m_bFirstInitFlag = true;
    m_nCtlSource = CTL_SOURCE_SELF;

    m_pLogicCtlParam->GetNetCardsTable(m_atNetConfig);
	m_pLogicCtlParam->GetStartSequenceInfo(m_tAscStartSequenceInfo);
	if (m_tAscStartSequenceInfo.m_byStartYellowFlash == 0)
	{
        m_tAscStartSequenceInfo.m_byStartYellowFlash = TIME_STARTUP_FLASH;
	}
	if (m_tAscStartSequenceInfo.m_byStartAllRed == 0)
	{
        m_tAscStartSequenceInfo.m_byStartAllRed = TIME_STARTUP_RED;
	}

    m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "COpenATCLogicCtlManager Init!"); 
}

void COpenATCLogicCtlManager::Stop()
{

}

/*==================================================================== 
������ ��Work
���� ���߼�����ģ�������̺���
�㷨ʵ�� �� 
����˵�� ��
����ֵ˵������
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ������          �������� 
====================================================================*/ 
void COpenATCLogicCtlManager::Work()
{
    if (m_pLogicCtlParam == NULL ||
        m_pLogicCtlStatus == NULL)
    {
        return;
    }

	ProcLampClrRunCounter();

	if (m_pLogicCtlStatus->GeFaultDetBoardControlStatus())
	{
		//�źŻ�����ʱ��������ϰ����ڿ��ƣ����źŻ���ÿ���Ȩ�Ժ����е�ȫ��
		SetAllRedStage();
		m_pLogicCtlStatus->SetFaultDetBoardControlStatus(false);
	}
	
	StartUpTimeSequenceCtl();

	AfterStartUpTimeSequenceCtl();

	ProcGlobalRunStatus();

	if (m_nCurCtlMode != CTL_MODE_FLASH && m_nCurCtlMode != CTL_MODE_ALLRED && m_nCurCtlMode != CTL_MODE_OFF)
	{
		ProcLockChannelLampClr();
	}
}

/*==================================================================== 
������ ��StartUpTimeSequenceCtl
���� ������ʵ������ʱ����ƣ�10�������5��ȫ�죬Ȼ�������������
�㷨ʵ�� �� 
����˵�� ����
����ֵ˵������
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ������          �������� 
====================================================================*/ 
void COpenATCLogicCtlManager::StartUpTimeSequenceCtl()
{
    if (m_nLogicCtlStage == CTL_STAGE_UNDEFINE)
    {
        TPhaseLampClrRunCounter tRunCounter;
        m_nLogicCtlStage = CTL_STAGE_STARTUP_FLASH;
        m_nCurCtlMode = CTL_MODE_FLASH;
        m_pLogicCtlStatus->GetPhaseLampClrRunCounter(tRunCounter);
        tRunCounter.m_nLampClrTime[0] = 0;
        tRunCounter.m_nLampClrStartCounter[0] = tRunCounter.m_nCurCounter;    
        m_pLogicCtlStatus->SetPhaseLampClrRunCounter(tRunCounter);

        TLogicCtlStatus tCtlStatus;
        m_pLogicCtlStatus->GetLogicCtlStatus(tCtlStatus);
        tCtlStatus.m_nRunStage = CTL_STAGE_STARTUP_FLASH;
        m_pLogicCtlStatus->SetLogicCtlStatus(tCtlStatus);

        if (m_pLogicCtlMode != NULL)
        {
            m_pLogicCtlMode->Release();
            delete m_pLogicCtlMode;
            m_pLogicCtlMode = NULL;
        }
        m_pLogicCtlMode = CLogicCtlModeSimpleFactory::Create(CTL_MODE_FLASH);
        m_pLogicCtlMode->Init(m_pLogicCtlParam,m_pLogicCtlStatus,m_pOpenATCLog,0);
        m_pLogicCtlMode->Run();
        m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "COpenATCLogicCtlManager Startup yellow flash!");

		//����ϵͳ��������״̬
		SetSystemControlStatus(CONTROL_FAILED, NO_SUPPORT_CONTROL_WAY, CONTROL_FAILED, NO_SUPPORT_CONTROL_WAY, CONTROL_FAILED, NO_SUPPORT_CONTROL_WAY,
			CONTROL_FAILED, NO_SUPPORT_CONTROL_WAY, CONTROL_FAILED, NO_SUPPORT_CONTROL_WAY);
		//�������ȿ�������״̬
		SetPreemptControlStatus(CONTROL_FAILED, NO_SUPPORT_CONTROL_WAY);
    }
    else if (m_nLogicCtlStage == CTL_STAGE_STARTUP_FLASH)
    {
        StartUpFlashCtl();
    }
    else if (m_nLogicCtlStage == CTL_STAGE_STARTUP_RED)
    {
        StartUpAllRedCtl();
    }
}

/*==================================================================== 
������ ��StartUpFlashCtl
���� ������ʵ������ʱ���������
�㷨ʵ�� �� 
����˵�� ����
����ֵ˵������
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ������          �������� 
====================================================================*/ 
void COpenATCLogicCtlManager::StartUpFlashCtl()
{
    if (m_pLogicCtlMode != NULL)
    {
        m_pLogicCtlMode->Run();
    }

    TPhaseLampClrRunCounter tRunCounter;
    m_pLogicCtlStatus->GetPhaseLampClrRunCounter(tRunCounter);

    if (tRunCounter.m_nLampClrTime[0] >= m_tAscStartSequenceInfo.m_byStartYellowFlash)
    {
        m_nLogicCtlStage = CTL_STAGE_STARTUP_RED;
        m_nCurCtlMode = CTL_MODE_ALLRED;
        tRunCounter.m_nLampClrTime[0] = 0;
        tRunCounter.m_nLampClrStartCounter[0] = tRunCounter.m_nCurCounter;
        m_pLogicCtlStatus->SetPhaseLampClrRunCounter(tRunCounter);

        TLogicCtlStatus tCtlStatus;
        m_pLogicCtlStatus->GetLogicCtlStatus(tCtlStatus);
        tCtlStatus.m_nRunStage = CTL_STAGE_STARTUP_RED;
        m_pLogicCtlStatus->SetLogicCtlStatus(tCtlStatus);

        if (m_pLogicCtlMode != NULL)
        {
            m_pLogicCtlMode->Release();
            delete m_pLogicCtlMode;
            m_pLogicCtlMode = NULL;
        }
        m_pLogicCtlMode = CLogicCtlModeSimpleFactory::Create(CTL_MODE_ALLRED);
        m_pLogicCtlMode->Init(m_pLogicCtlParam,m_pLogicCtlStatus,m_pOpenATCLog,0);
        m_pLogicCtlMode->Run();
        m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "COpenATCLogicCtlManager Startup all red!");

		//����ϵͳ��������״̬
		SetSystemControlStatus(CONTROL_FAILED, NO_SUPPORT_CONTROL_WAY, CONTROL_FAILED, NO_SUPPORT_CONTROL_WAY, CONTROL_FAILED, NO_SUPPORT_CONTROL_WAY, 
			CONTROL_FAILED, NO_SUPPORT_CONTROL_WAY, CONTROL_FAILED, NO_SUPPORT_CONTROL_WAY);
		//�������ȿ�������״̬
		SetPreemptControlStatus(CONTROL_FAILED, NO_SUPPORT_CONTROL_WAY);
    }
}

/*==================================================================== 
������ ��StartUpAllRedCtl
���� ������ʵ������ʱ��ȫ�����
�㷨ʵ�� �� 
����˵�� ����
����ֵ˵������
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ������          �������� 
====================================================================*/ 
void COpenATCLogicCtlManager::StartUpAllRedCtl()
{
    if (m_pLogicCtlMode != NULL)
    {
        m_pLogicCtlMode->Run();
    }

    TPhaseLampClrRunCounter tRunCounter;
    m_pLogicCtlStatus->GetPhaseLampClrRunCounter(tRunCounter);
    if (tRunCounter.m_nLampClrTime[0] >= m_tAscStartSequenceInfo.m_byStartAllRed)
    {
        m_nLogicCtlStage = CTL_STAGE_SELFCTL;            
        tRunCounter.m_nLampClrTime[0] = 0;
        tRunCounter.m_nLampClrStartCounter[0] = tRunCounter.m_nCurCounter;
        m_pLogicCtlStatus->SetPhaseLampClrRunCounter(tRunCounter);

        TLogicCtlStatus tCtlStatus;
        m_pLogicCtlStatus->GetLogicCtlStatus(tCtlStatus);
        tCtlStatus.m_nRunStage = CTL_STAGE_SELFCTL;
        m_pLogicCtlStatus->SetLogicCtlStatus(tCtlStatus);
        m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "COpenATCLogicCtlManager Start Self Ctl!");

		//����ϵͳ��������״̬
		SetSystemControlStatus(CONTROL_SUCCEED, 0, CONTROL_SUCCEED, 0, CONTROL_SUCCEED, 0, CONTROL_FAILED, NO_SUPPORT_CONTROL_WAY, CONTROL_SUCCEED, 0);
		//�������ȿ�������״̬
		SetPreemptControlStatus(CONTROL_SUCCEED, 0);
    }
}

/*==================================================================== 
������ ��AfterStartUpTimeSequenceCtl
���� ������ʵ������ʱ����ƽ����Ժ����������
�㷨ʵ�� �� 
����˵�� ����
����ֵ˵������
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ������          �������� 
====================================================================*/ 
void COpenATCLogicCtlManager::AfterStartUpTimeSequenceCtl()
{
    if (m_nLogicCtlStage == CTL_STAGE_SELFCTL)
    {
        int nFaultLevel = ProcFault();
        if (nFaultLevel == CRITICAL_FAULT)
        {
            CriticalFaultRun();
        }
        else
        {
            SelfAndUsrCtlRun();
        }
    }
    else if (m_nLogicCtlStage == CTL_STAGE_FAULTFORCE)
    {
        if (m_pLogicCtlMode != NULL)
        {
            m_pLogicCtlMode->Run();              
        }
    }
}

/*==================================================================== 
������ ��SelfAndUsrCtlRun
���� �����ݲ���������������,��Ӧ�û���Ԥ
�㷨ʵ�� �� 
����˵�� ����
����ֵ˵������
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ������          �������� 
2020/05/25     V1.0 ����Ƽ          �����˻����������������� 
====================================================================*/ 
void COpenATCLogicCtlManager::SelfAndUsrCtlRun()
{
    /*TOpenATCStatusInfo tOpenATCStatusInfo;
    memset(&tOpenATCStatusInfo, 0, sizeof(TOpenATCStatusInfo));
    m_pLogicCtlStatus->GetOpenATCStatusInfo(tOpenATCStatusInfo);
   
    if (tOpenATCStatusInfo.m_tYellowFlashStatusInfo.m_cTriggerStatus)
    {
        if (m_nCurCtlMode != CTL_MODE_FLASH)
        {
            m_pLogicCtlMode->Release();
            delete m_pLogicCtlMode;
            m_pLogicCtlMode = NULL;

            m_pLogicCtlMode = CLogicCtlModeSimpleFactory::Create(CTL_MODE_FLASH);
            m_nCurCtlMode = CTL_MODE_FLASH;
            m_pLogicCtlMode->Init(m_pLogicCtlParam,m_pLogicCtlStatus,m_pOpenATCLog,0);
            m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "Yellow Flash Trigger!");

            m_nCtlSource = CTL_SOURCE_YELLOWFLASHTRIGGER;
        } 

        m_pLogicCtlMode->Run();
    }
    else */if (LocalUsrCtlRun())
    {

    }
    else if (SystemUsrCtlRun())
    {
    
    }
	else if (PreemptCtlRun())
	{

	}
    else
    {
        SelfRun();
    }
}

/*==================================================================== 
������ ��LocalUsrCtlRun
���� �������û���Ԥ�����ȼ����
�㷨ʵ�� �� 
����˵�� ����
����ֵ˵������
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ������          �������� 
====================================================================*/ 
bool COpenATCLogicCtlManager::LocalUsrCtlRun()
{
    bool bRet = true;

	TManualCmd  tManualCmd;
	memset(&tManualCmd,0,sizeof(tManualCmd));
	m_pLogicCtlStatus->GetManualCmd(tManualCmd); 

    //�ֶ���ť������ 
    if (tManualCmd.m_nCmdSource == CTL_SOURCE_LOCAL && tManualCmd.m_bNewCmd)
    { 
		TManualCmd  tValidManualCmd;
	    memset(&tValidManualCmd,0,sizeof(tValidManualCmd));
		m_pLogicCtlStatus->GetValidManualCmd(tValidManualCmd); 

		//�Զ����ư�ť������
		if (tManualCmd.m_bPatternInterruptCmd && tManualCmd.m_tPatternInterruptCmd.m_nControlMode == CTL_MODE_SELFCTL)
		{
			//������Ч�Ŀ���ָ��
			CreateValidManualCmd(tManualCmd, tValidManualCmd);
			SetPanelBtnStatusReply(tValidManualCmd, HWPANEL_BTN_AUTO);
		}
		else
		{
			if (m_nCtlSource == CTL_SOURCE_LOCAL)//�Ѿ����뱾�ؿ���
			{
				if (tManualCmd.m_bPatternInterruptCmd && tManualCmd.m_tPatternInterruptCmd.m_nControlMode == CTL_MODE_ALLRED)
				{ 
					if (m_nCtlSource == CTL_SOURCE_PREEMPT)
					{
						m_pLogicCtlStatus->DelteAllPreemptCtlCmdFromList();
					}

					//������Ч�Ŀ���ָ��
					CreateValidManualCmd(tManualCmd, tValidManualCmd);
					UsrCtlAllRed(tValidManualCmd); 
				}
				else  if (tManualCmd.m_bPatternInterruptCmd && tManualCmd.m_tPatternInterruptCmd.m_nControlMode == CTL_MODE_FLASH)
				{
					if (m_nCtlSource == CTL_SOURCE_PREEMPT)
					{
						m_pLogicCtlStatus->DelteAllPreemptCtlCmdFromList();
					}

					//������Ч�Ŀ���ָ��
					CreateValidManualCmd(tManualCmd, tValidManualCmd);
					UsrCtlYellowFalsh(tValidManualCmd);   
				}
				else if (tManualCmd.m_bStepForwardCmd)
				{
					UsrCtlStepForward(tManualCmd, tValidManualCmd);
				}
				else if (tManualCmd.m_bDirectionCmd)
				{
					UsrCtlDirectionKey(tManualCmd, tValidManualCmd);
				}
			}
			else    //δ���뱾���ֶ����ƣ���������뱾���ֶ�����
			{
				if (m_nCurCtlMode != CTL_MODE_MANUAL)//�п����Ǵ��Զ���û�н������е��ֶ�
				{
					tValidManualCmd.m_nCtlMode = CTL_MODE_MANUAL;
					tValidManualCmd.m_nSubCtlMode = CTL_MANUAL_SUBMODE_PANEL_FITSTMANUAL;//��ģʽ����Ϊ��һ�ΰ����ֶ�
				}
				else
				{
					//1.�ڷ����ʱ�����Զ�������û�н����е��Զ����ְ����ֶ�����ģʽ����
			        //2.�ڷ����ʱ�����Զ����е��Զ��Ժ��Զ���û�н������ְ����ֶ�����ģʽ����Ϊ��һ�ΰ����ֶ�
					//3.��ϵͳ���Ƶ�ʱ��������ֶ�����ģʽ����Ϊ��һ�ΰ����ֶ�
					//4.�ڲ�����ʱ�����Զ����е��Զ��Ժ��Զ���û�н������ְ����ֶ�����ģʽ����Ϊ��һ�ΰ����ֶ�
					if (tValidManualCmd.m_nSubCtlMode == CTL_MANUAL_SUBMODE_DEFAULT || m_tOldValidManualCmd.m_nCurCtlSource == CTL_SOURCE_SYSTEM ||
						tValidManualCmd.m_nSubCtlMode == CTL_MANUAL_SUBMODE_PANEL_STEPFOWARD && m_tOldValidManualCmd.m_bPatternInterruptCmd)
					{
						tValidManualCmd.m_nCtlMode = CTL_MODE_MANUAL;
						tValidManualCmd.m_nSubCtlMode = CTL_MANUAL_SUBMODE_PANEL_FITSTMANUAL;//��ģʽ����Ϊ��һ�ΰ����ֶ�
					}
				}
				tValidManualCmd.m_nCmdSource = CTL_SOURCE_LOCAL;
				tValidManualCmd.m_nCurCtlSource = CTL_SOURCE_LOCAL;
				memcpy(tValidManualCmd.m_szPeerIp,tManualCmd.m_szPeerIp,strlen(tManualCmd.m_szPeerIp));
				tValidManualCmd.m_bNewCmd = true;
				tValidManualCmd.m_bStepForwardCmd = tManualCmd.m_bStepForwardCmd;
				memcpy(&tValidManualCmd.m_tStepForwardCmd,&tManualCmd.m_tStepForwardCmd,sizeof(TStepForwardCmd));
				tValidManualCmd.m_bDirectionCmd = tManualCmd.m_bDirectionCmd;
				memcpy(&tValidManualCmd.m_tDirectionCmd,&tManualCmd.m_tDirectionCmd,sizeof(TDirectionCmd));
				tValidManualCmd.m_bPatternInterruptCmd = tManualCmd.m_bDirectionCmd;
				memcpy(&tValidManualCmd.m_tPatternInterruptCmd,&tManualCmd.m_tPatternInterruptCmd,sizeof(TPatternInterruptCmd));
				m_pLogicCtlStatus->SetValidManualCmd(tValidManualCmd);
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "CreateValidManualCmd, Trans To Panel First Manual");

				SetPanelBtnStatusReply(tValidManualCmd, HWPANEL_BTN_MANUAL);

				m_nCtlSource = CTL_SOURCE_LOCAL; 
				
				if (m_nCurCtlMode != CTL_MODE_FLASH && m_nCurCtlMode != CTL_MODE_ALLRED && m_nCurCtlMode != CTL_MODE_OFF && m_nCurCtlMode != CTL_MODE_MANUAL)
				{
					InitUsrCtlLogicObj();
					m_nCurCtlMode = CTL_MODE_MANUAL;
				}

				m_pLogicCtlMode->SetUsrCtlFlag(true); 

				//����ϵͳ��������״̬
				SetSystemControlStatus(CONTROL_FAILED, HIGH_PRIORITY_USER_CONTROL_NO_EXECUT, CONTROL_FAILED, HIGH_PRIORITY_USER_CONTROL_NO_EXECUT,
					CONTROL_FAILED, HIGH_PRIORITY_USER_CONTROL_NO_EXECUT, CONTROL_FAILED, HIGH_PRIORITY_USER_CONTROL_NO_EXECUT,
					CONTROL_FAILED, HIGH_PRIORITY_USER_CONTROL_NO_EXECUT);
				//�������ȿ�������״̬
		        SetPreemptControlStatus(CONTROL_FAILED, HIGH_PRIORITY_USER_CONTROL_NO_EXECUT);
			} 
		}

		//���ָ��
		memset(&tManualCmd,0,sizeof(tManualCmd));
	    m_pLogicCtlStatus->SetManualCmd(tManualCmd); 
    }

	if (m_nCtlSource == CTL_SOURCE_LOCAL)
	{
		if (m_tOldValidManualCmd.m_bPatternInterruptCmd && m_tOldValidManualCmd.m_tPatternInterruptCmd.m_nControlMode == CTL_MODE_SELFCTL)
		{
			TManualCmd  tValidManualCmd;
			memset(&tValidManualCmd,0,sizeof(tValidManualCmd));
			m_pLogicCtlStatus->GetValidManualCmd(tValidManualCmd); 
			if (tValidManualCmd.m_nSubCtlMode == CTL_MANUAL_SUBMODE_PANEL_DIRECTION)
			{
				m_pLogicCtlMode->Run();
			}
			else
			{
				bRet = false;
			}
		}
		else
		{
			m_pLogicCtlMode->Run();
		}
	}
	else
	{
		bRet = false;
	}
     
    return bRet;
}

/*==================================================================== 
������ ��SystemUsrCtlRun
���� ��ϵͳ�û���Ԥ,���ȼ����ڱ����û���Ԥ
�㷨ʵ�� �� 
����˵�� ����
����ֵ˵������
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ������          �������� 
2020/02/28     V1.0 ����Ƽ          �����������ģʽ�е�������ģʽ 
====================================================================*/ 
bool COpenATCLogicCtlManager::SystemUsrCtlRun()
{
    bool bRet = true;
    char szInfo[256] = {0};

	TManualCmd  tManualCmd;
	memset(&tManualCmd,0,sizeof(tManualCmd));
	m_pLogicCtlStatus->GetManualCmd(tManualCmd);

	if (tManualCmd.m_nCmdSource == CTL_SOURCE_SYSTEM && tManualCmd.m_bNewCmd)//�ж��Ƿ����µ�ϵͳ����
	{
		TManualCmd  tValidManualCmd;
	    memset(&tValidManualCmd,0,sizeof(tValidManualCmd));
		m_pLogicCtlStatus->GetValidManualCmd(tValidManualCmd); 

		//�ж�ϵͳ�����ܷ�ʹ��
		if (!CheckSystemCmdUseStatus(tManualCmd, tValidManualCmd))
		{
			//���ָ��
			memset(&tManualCmd,0,sizeof(tManualCmd));
			m_pLogicCtlStatus->SetManualCmd(tManualCmd); 
			bRet = true;
			return bRet;
		}
		
		if (tManualCmd.m_bPatternInterruptCmd)
		{
		    if (tManualCmd.m_tPatternInterruptCmd.m_nControlMode == CTL_MODE_FLASH)
			{
				if (m_nCtlSource == CTL_SOURCE_PREEMPT)
				{
					m_pLogicCtlStatus->DelteAllPreemptCtlCmdFromList();
				}

				if (m_nCurCtlMode != CTL_MODE_FLASH)
				{
					//��������ģʽ
          			CreateCtlMode(CTL_MODE_FLASH, CTL_SOURCE_SYSTEM);
					//������Ч�Ŀ���ָ��
					CreateValidManualCmd(tManualCmd, tValidManualCmd);
					//��ϵͳ��������״̬
					SetSystemControlStatus(CONTROL_SUCCEED, 0, CONTROL_SUCCEED, 0, CONTROL_FAILED, NO_EXIST_PATTERN_NO, CONTROL_FAILED, NO_SUPPORT_CONTROL_WAY, CONTROL_SUCCEED, 0);
					//�������ȿ�������״̬
		            SetPreemptControlStatus(CONTROL_FAILED, HIGH_PRIORITY_USER_CONTROL_NO_EXECUT);
				}
			}
			else if (tManualCmd.m_tPatternInterruptCmd.m_nControlMode == CTL_MODE_ALLRED)
			{
				if (m_nCtlSource == CTL_SOURCE_PREEMPT)
				{
					m_pLogicCtlStatus->DelteAllPreemptCtlCmdFromList();
				}

				if (m_nCurCtlMode != CTL_MODE_ALLRED)
				{
					//����ȫ��ģʽ
					CreateCtlMode(CTL_MODE_ALLRED, CTL_SOURCE_SYSTEM);
					//������Ч�Ŀ���ָ��
					CreateValidManualCmd(tManualCmd, tValidManualCmd);
					//��ϵͳ��������״̬
					SetSystemControlStatus(CONTROL_SUCCEED, 0, CONTROL_SUCCEED, 0, CONTROL_FAILED, NO_EXIST_PATTERN_NO, CONTROL_FAILED, NO_SUPPORT_CONTROL_WAY, CONTROL_SUCCEED, 0);
					//�������ȿ�������״̬
		            SetPreemptControlStatus(CONTROL_FAILED, HIGH_PRIORITY_USER_CONTROL_NO_EXECUT);
				}
			}
			else if (tManualCmd.m_tPatternInterruptCmd.m_nControlMode == CTL_MODE_OFF)
			{
				if (m_nCtlSource == CTL_SOURCE_PREEMPT)
				{
					m_pLogicCtlStatus->DelteAllPreemptCtlCmdFromList();
				}

				if (m_nCurCtlMode != CTL_MODE_OFF)
				{
					//�����ص�ģʽ
          			CreateCtlMode(CTL_MODE_OFF, CTL_SOURCE_SYSTEM);
					//������Ч�Ŀ���ָ��
					CreateValidManualCmd(tManualCmd, tValidManualCmd);
					//��ϵͳ��������״̬
					SetSystemControlStatus(CONTROL_SUCCEED, 0, CONTROL_SUCCEED, 0, CONTROL_FAILED, NO_EXIST_PATTERN_NO, CONTROL_FAILED, NO_SUPPORT_CONTROL_WAY, CONTROL_SUCCEED, 0);
					//�������ȿ�������״̬
		            SetPreemptControlStatus(CONTROL_FAILED, HIGH_PRIORITY_USER_CONTROL_NO_EXECUT);
				}
			}
			else //�е��������ߴ��������������Ʒ�ʽ���綨���ڣ���Ӧ,ͨ������
			{
				if (m_nCtlSource == CTL_SOURCE_PREEMPT)//�Ż�����ʱ��ϵͳ�·��˷�����Ԥָ��
				{
					CreatePatternInterruptCmdInPreemptControl(tManualCmd);
				}
				else
				{
					m_nCtlSource = CTL_SOURCE_SYSTEM;
					m_pLogicCtlMode->SetSystemUsrCtlFlag(true);  
					//������Ч�Ŀ���ָ��
					CreateValidManualCmd(tManualCmd, tValidManualCmd);
					//��ϵͳ��������״̬
					SetSystemControlStatus(CONTROL_SUCCEED, 0, CONTROL_SUCCEED, 0, CONTROL_SUCCEED, 0, CONTROL_FAILED, NO_SUPPORT_CONTROL_WAY, CONTROL_SUCCEED, 0);
					//�������ȿ�������״̬
					SetPreemptControlStatus(CONTROL_SUCCEED, 0);
				}
			}
		}
		else if (tManualCmd.m_bStepForwardCmd || tManualCmd.m_bChannelLockCmd)
		{
			if (m_nCurCtlMode != CTL_MODE_ALLRED && m_nCurCtlMode != CTL_MODE_FLASH && m_nCurCtlMode != CTL_MODE_OFF &&
				m_nCurCtlMode != CTL_MODE_MANUAL)
			{
				CLogicCtlMode * pMode = CLogicCtlModeSimpleFactory::Create(CTL_MODE_MANUAL);
				m_nCurCtlMode = CTL_MODE_MANUAL;
				pMode->Init(m_pLogicCtlParam,m_pLogicCtlStatus,m_pOpenATCLog,0);
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "System Usr Ctl Manual!");

				void * pParam = m_pLogicCtlMode->GetCurCtlParam();
				void * pLampClr = m_pLogicCtlMode->GetCurLampClr();
				void * pStageTable = m_pLogicCtlMode->GetCurStageTable();
				void * pChannelSplitMode = m_pLogicCtlMode->GetCurChannelSplitMode();
				pMode->SetCtlDerivedParam(pParam,pLampClr,pStageTable,pChannelSplitMode);

				m_pLogicCtlMode->Release();
				delete m_pLogicCtlMode;
				m_pLogicCtlMode = pMode;
			}

			m_nCtlSource = CTL_SOURCE_SYSTEM;
			m_pLogicCtlMode->SetSystemUsrCtlFlag(true); 

			//������Ч�Ŀ���ָ��
			CreateValidManualCmd(tManualCmd, tValidManualCmd);
			//��ϵͳ��������״̬
			if (tManualCmd.m_bStepForwardCmd)
			{
				SetSystemControlStatus(CONTROL_SUCCEED, 0, CONTROL_SUCCEED, 0, CONTROL_SUCCEED, 0, CONTROL_SUCCEED, 0, CONTROL_SUCCEED, 0);
			}
			else
			{
				SetSystemControlStatus(CONTROL_SUCCEED, 0, CONTROL_SUCCEED, 0, CONTROL_FAILED, NO_SUPPORT_CONTROL_WAY, CONTROL_FAILED, NO_SUPPORT_CONTROL_WAY, CONTROL_SUCCEED, 0);
			}
			//�������ȿ�������״̬
		    SetPreemptControlStatus(CONTROL_FAILED, HIGH_PRIORITY_USER_CONTROL_NO_EXECUT);
		}
		else if (tManualCmd.m_bPreemptCtlCmd)
		{
			if (m_nCurCtlMode != CTL_MODE_MANUAL)
			{
				m_nCtlSource = CTL_SOURCE_PREEMPT;
			}
		}

		memcpy(&m_tOldValidManualCmd,&tValidManualCmd,sizeof(TManualCmd));
	
		//���ָ��
		memset(&tManualCmd,0,sizeof(tManualCmd));
	    m_pLogicCtlStatus->SetManualCmd(tManualCmd); 
	}

	if (m_nCtlSource == CTL_SOURCE_SYSTEM)
	{
		if (m_tOldValidManualCmd.m_bPatternInterruptCmd)
		{
			if (m_tOldValidManualCmd.m_tPatternInterruptCmd.m_nControlMode != CTL_MODE_SELFCTL)
			{
				//�û���Ԥ��ͨ������ʱ����Ҫ���з�����Ԥ����
				SystemAskPlanRun();
			}
			else 
			{
				bRet = false;//�ص�����
			}
		}
		else
		{
			if (m_nCurCtlMode != CTL_MODE_MANUAL)
			{
				TMainCtlBoardRunStatus tMainCtlBoardRunStatus;
				m_pLogicCtlStatus->GetMainCtlBoardRunStatus(tMainCtlBoardRunStatus);
				if (tMainCtlBoardRunStatus.m_bIsNeedGetParam)
				{
                    SystemAskPlanRun();
					//tMainCtlBoardRunStatus.m_bIsNeedGetParam = false;
					//m_pLogicCtlStatus->SetMainCtlBoardRunStatus(tMainCtlBoardRunStatus);
				}
			}
			m_pLogicCtlMode->Run();

			SwitchManualControlPatternToSelf();
		}
	}
	else
	{
		bRet = false;//�ص�����
	}

    return bRet;      
}

/*==================================================================== 
������ ��PreemptCtlRun
���� �����ȿ���,���ȼ����ڱ����û���Ԥ��ϵͳ��Ԥ
�㷨ʵ�� �� 
����˵�� ����
����ֵ˵������
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ������          �������� 
2020/02/28     V1.0 ����Ƽ          �����������ģʽ�е�������ģʽ 
====================================================================*/ 
bool COpenATCLogicCtlManager::PreemptCtlRun()
{
    bool bRet = true;
    char szInfo[256] = {0};

	TPreemptCtlCmd  tPreemptCtlCmd;
	memset(&tPreemptCtlCmd,0,sizeof(tPreemptCtlCmd));
	m_pLogicCtlStatus->GetPreemptCtlCmd(tPreemptCtlCmd);

	if (tPreemptCtlCmd.m_bNewCmd)//�ж��Ƿ����µ����ȿ�������
	{
		//�ж����ȿ��������ܷ�ʹ��
		if (!CheckPreemptCmdUseStatus(tPreemptCtlCmd))
		{
			//���ָ��
			memset(&tPreemptCtlCmd,0,sizeof(tPreemptCtlCmd));
			m_pLogicCtlStatus->SetPreemptCtlCmd(tPreemptCtlCmd); 
			bRet = true;
			return bRet;
		}

		if (!tPreemptCtlCmd.m_bPatternInterruptCmd)
		{
			SetPreemptStageIndex(tPreemptCtlCmd.m_byPreemptPhaseID, tPreemptCtlCmd.m_byPreemptStageIndex);//������λ��Ӧ�Ľ׶κ�
		}

		if (tPreemptCtlCmd.m_bPatternInterruptCmd && tPreemptCtlCmd.m_nCmdSource == CTL_SOURCE_PREEMPT)
		{
			//CLogicCtlPreempt���ɻص��������������Ҫ�ŵ����ȿ��������б���
		}
		else
		{
			tPreemptCtlCmd.m_bNewCmd = false;
			m_pLogicCtlStatus->SePreemptCtlCmdList(tPreemptCtlCmd);
		}

		if (m_nCurCtlMode != CTL_MODE_ALLRED && m_nCurCtlMode != CTL_MODE_FLASH && m_nCurCtlMode != CTL_MODE_OFF &&
			m_nCurCtlMode != CTL_MODE_PREEMPT)
		{
			CLogicCtlMode * pMode = CLogicCtlModeSimpleFactory::Create(CTL_MODE_PREEMPT);
			m_nCurCtlMode = CTL_MODE_PREEMPT;
			pMode->Init(m_pLogicCtlParam,m_pLogicCtlStatus,m_pOpenATCLog,0);
			m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "Preempt Usr Ctl Manual!");

			void * pParam = m_pLogicCtlMode->GetCurCtlParam();
			void * pLampClr = m_pLogicCtlMode->GetCurLampClr();
			void * pStageTable = m_pLogicCtlMode->GetCurStageTable();
			void * pChannelSplitMode = m_pLogicCtlMode->GetCurChannelSplitMode();
			pMode->SetCtlDerivedParam(pParam,pLampClr,pStageTable,pChannelSplitMode);

			m_pLogicCtlMode->Release();
			delete m_pLogicCtlMode;
			m_pLogicCtlMode = pMode;
		}

		m_nCtlSource = CTL_SOURCE_PREEMPT;
		m_pLogicCtlMode->SetPreemptCtlFlag(true); 

		//�������ȿ�������״̬
		SetPreemptControlStatus(CONTROL_SUCCEED, 0);

		memcpy(&m_tOldPreemptCtlCmd,&tPreemptCtlCmd,sizeof(TPreemptCtlCmd));

		//���ָ��
		memset(&tPreemptCtlCmd,0,sizeof(tPreemptCtlCmd));
	    m_pLogicCtlStatus->SetPreemptCtlCmd(tPreemptCtlCmd);
	}
	
	if (m_nCtlSource == CTL_SOURCE_PREEMPT)
	{
		if (m_tOldPreemptCtlCmd.m_bPatternInterruptCmd)
		{
			if (m_tOldPreemptCtlCmd.m_tPatternInterruptCmd.m_nControlMode != CTL_MODE_SELFCTL)
			{
				//ϵͳ��Ԥ��ʱ����Ҫ���з�����Ԥ����
				PreemptAskPlanRun();
			}
			else 
			{
				bRet = false;//�ص�����
			}
		}
		else
		{
			if (m_nCurCtlMode != CTL_MODE_MANUAL && m_nCurCtlMode != CTL_MODE_PREEMPT)
			{
				TMainCtlBoardRunStatus tMainCtlBoardRunStatus;
				m_pLogicCtlStatus->GetMainCtlBoardRunStatus(tMainCtlBoardRunStatus);
				if (tMainCtlBoardRunStatus.m_bIsNeedGetParam)
				{
					tMainCtlBoardRunStatus.m_bIsNeedGetParam = false;
					m_pLogicCtlStatus->SetMainCtlBoardRunStatus(tMainCtlBoardRunStatus);
				}
			}
			m_pLogicCtlMode->Run();
		}
	}
	else
	{
		bRet = false;//�ص�����
	}

    return bRet;      
}

/*==================================================================== 
������ ��SelfRun
���� �����ݲ���������������
�㷨ʵ�� �� 
����˵�� ����
����ֵ˵������
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ������          �������� 
====================================================================*/ 
void COpenATCLogicCtlManager::SelfRun()
{
    char szInfo[256] = {0};

	if (m_nCtlSource != CTL_SOURCE_SELF)
	{
		//����ϵͳ��������״̬
		SetSystemControlStatus(CONTROL_SUCCEED, 0, CONTROL_SUCCEED, 0, CONTROL_SUCCEED, 0, CONTROL_FAILED, NO_SUPPORT_CONTROL_WAY, CONTROL_SUCCEED, 0);
		//�������ȿ�������״̬
		SetPreemptControlStatus(CONTROL_SUCCEED, 0);
	}

	m_nCtlSource = CTL_SOURCE_SELF;

    TMainCtlBoardRunStatus tMainCtlBoardRunStatus;
    m_pLogicCtlStatus->GetMainCtlBoardRunStatus(tMainCtlBoardRunStatus);
    if (tMainCtlBoardRunStatus.m_bIsNeedGetParam)
    {
        PrepareParam();
        m_pLogicCtlStatus->GetMainCtlBoardRunStatus(tMainCtlBoardRunStatus);
        tMainCtlBoardRunStatus.m_bIsNeedGetParam = false;
        m_pLogicCtlStatus->SetMainCtlBoardRunStatus(tMainCtlBoardRunStatus);
    }
        
    if (m_bIsCtlModeChg)
    {
        if (m_pLogicCtlMode != NULL)
        {
            m_pLogicCtlMode->Release();
            delete m_pLogicCtlMode;
            m_pLogicCtlMode = NULL;
        }

		m_pLogicCtlMode = CLogicCtlModeSimpleFactory::Create(m_nCurCtlMode);
        m_pLogicCtlMode->Init(m_pLogicCtlParam,m_pLogicCtlStatus,m_pOpenATCLog,0);
        m_bIsCtlModeChg = false;

		if (m_nCurCtlMode == CTL_MODE_FLASH || m_nCurCtlMode == CTL_MODE_ALLRED || m_nCurCtlMode == CTL_MODE_OFF)
		{
			if (m_nCurCtlMode == CTL_MODE_FLASH)
			{
                m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "Param Ctl Yellow Flash!");
			}
			else if (m_nCurCtlMode == CTL_MODE_ALLRED)
			{
                m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "Param Ctl All Red!");
			}
			else
			{
                m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "Param Ctl Off!");
			}
		}
		else 
		{
			sprintf(szInfo, "Current Ctl Mode is %d, Plan No is %d", m_nCurCtlMode, m_nCurPlanNo);
            m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, szInfo);

			//����ϵͳ��������״̬
			//SetSystemControlStatus(CONTROL_SUCCEED, 0, CONTROL_SUCCEED, 0, CONTROL_SUCCEED, 0);
			 m_tRunStageTable = m_pLogicCtlMode->GetRunStageTable();
		}

		if (m_tOldValidManualCmd.m_bNewCmd)
		{
			//��Ԥ�ص������ɹ�������Ч���ֶ�ָ������һ�λ�����ֶ�ָ�����
			TManualCmd  tValidManualCmd;
			memset(&tValidManualCmd,0,sizeof(tValidManualCmd));
			m_pLogicCtlStatus->SetValidManualCmd(tValidManualCmd); 

			memset(&m_tOldValidManualCmd,0,sizeof(tValidManualCmd));
		}
    }    

    m_pLogicCtlMode->Run(); 
}

/*==================================================================== 
������ ��SystemAskPlanRun
���� �������û���Ԥ�Ŀ��Ʒ�ʽ�ͷ������п���
�㷨ʵ�� �� 
����˵�� ����
����ֵ˵������
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ������          �������� 
====================================================================*/ 
void COpenATCLogicCtlManager::SystemAskPlanRun()
{
    TMainCtlBoardRunStatus tMainCtlBoardRunStatus;
    m_pLogicCtlStatus->GetMainCtlBoardRunStatus(tMainCtlBoardRunStatus);
    if (tMainCtlBoardRunStatus.m_bIsNeedGetParam)
    {
		TManualCmd  tValidManualCmd;
		memset(&tValidManualCmd,0,sizeof(tValidManualCmd));
		m_pLogicCtlStatus->GetValidManualCmd(tValidManualCmd);

		if (!tValidManualCmd.m_bNewCmd && !m_tOldValidManualCmd.m_bNewCmd)
		{
			m_pLogicCtlStatus->GetMainCtlBoardRunStatus(tMainCtlBoardRunStatus);
			tMainCtlBoardRunStatus.m_bIsNeedGetParam = false;
			m_pLogicCtlStatus->SetMainCtlBoardRunStatus(tMainCtlBoardRunStatus);
			return;
		}

		//�ж��ֶ����Ƶ���ʱ�������ӳ�ʱ��
		if (tValidManualCmd.m_tPatternInterruptCmd.m_nControlMode == CTL_MODE_MANUAL_CONTROL_PATTERN &&
			tValidManualCmd.m_tPatternInterruptCmd.m_nPatternNo == MAX_PATTERN_COUNT)
		{
			long lCurTime = time(NULL);
			if (labs(lCurTime - m_tPatternInterruptCmdTime) < 
				tValidManualCmd.m_tPatternInterruptCmd.m_tManualControlPattern.m_nDelayTime)
			{
				m_pLogicCtlMode->Run();
				return;
			}
		}

        PrepareParamForSystemAsk(tValidManualCmd.m_tPatternInterruptCmd.m_nControlMode,tValidManualCmd.m_tPatternInterruptCmd.m_nPatternNo);
    	    
        m_pLogicCtlStatus->GetMainCtlBoardRunStatus(tMainCtlBoardRunStatus);
        tMainCtlBoardRunStatus.m_bIsNeedGetParam = false;
        m_pLogicCtlStatus->SetMainCtlBoardRunStatus(tMainCtlBoardRunStatus);

        TLogicCtlStatus tCtlStatus;
        m_pLogicCtlStatus->GetLogicCtlStatus(tCtlStatus);
        if (tCtlStatus.m_nCurPlanNo != tValidManualCmd.m_tPatternInterruptCmd.m_nPatternNo ||
			tCtlStatus.m_nCurCtlMode != tValidManualCmd.m_tPatternInterruptCmd.m_nControlMode || tCtlStatus.m_nCurPlanNo == MAX_PATTERN_COUNT)
        {
            m_bIsCtlModeChg = true;
            m_nCurPlanNo = tValidManualCmd.m_tPatternInterruptCmd.m_nPatternNo; 
            m_nCurCtlMode = tValidManualCmd.m_tPatternInterruptCmd.m_nControlMode; 

			m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "System Ctl Mode is %d, Plan No is %d", tValidManualCmd.m_tPatternInterruptCmd.m_nControlMode,tValidManualCmd.m_tPatternInterruptCmd.m_nPatternNo);

			if (m_nCurCtlMode == CTL_MODE_CHANNEL_CHECK)
			{
				//ͨ�����ʱ��������Ԥ�������
				tValidManualCmd.m_bPatternInterruptCmd = false;
				memset(&tValidManualCmd.m_tPatternInterruptCmd,0,sizeof(TPatternInterruptCmd));
				m_pLogicCtlStatus->SetValidManualCmd(tValidManualCmd);
			}
			else if (m_nCurCtlMode == CTL_MODE_MANUAL_CONTROL_PATTERN)//�����е��ֶ����Ʒ���
			{
				m_pLogicCtlParam->SetManualControlPattern(tValidManualCmd.m_tPatternInterruptCmd.m_tManualControlPattern);
				m_nManualControlPatternStartTime = time(NULL);
				m_nManualControlPatternDurationTime = tValidManualCmd.m_tPatternInterruptCmd.m_tManualControlPattern.m_nDurationTime;

				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "ManualControlPattern StartTime:%d, DurationTime:%d, CycleTime:%d", m_nManualControlPatternStartTime, m_nManualControlPatternDurationTime, tValidManualCmd.m_tPatternInterruptCmd.m_tManualControlPattern.m_nCycleTime);
			}
        }
    }
        
    if (m_bIsCtlModeChg)
    {
        if (m_pLogicCtlMode != NULL)
        {
            m_pLogicCtlMode->Release();
            delete m_pLogicCtlMode;
            m_pLogicCtlMode = NULL;
        }

        m_pLogicCtlMode = CLogicCtlModeSimpleFactory::Create(m_nCurCtlMode);
        m_pLogicCtlMode->Init(m_pLogicCtlParam,m_pLogicCtlStatus,m_pOpenATCLog,m_nCurPlanNo);
        m_bIsCtlModeChg = false;
		m_pLogicCtlMode->SetSystemUsrCtlFlag(true);  
        m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "Current Ctl Mode is %d, Plan No is %d", m_nCurCtlMode, m_nCurPlanNo);

		//����ϵͳ��������״̬
		SetSystemControlStatus(CONTROL_SUCCEED, 0, CONTROL_SUCCEED, 0, CONTROL_SUCCEED, 0, CONTROL_FAILED, NO_SUPPORT_CONTROL_WAY, CONTROL_SUCCEED, 0);
		//�������ȿ�������״̬
		//SetPreemptControlStatus(CONTROL_SUCCEED, 0);

		if (m_tOldValidManualCmd.m_bNewCmd)
		{
			//������Ԥ�ɹ�������Ч���ֶ�ָ������һ�λ�����ֶ�ָ�����
			TManualCmd  tValidManualCmd;
			memset(&tValidManualCmd,0,sizeof(tValidManualCmd));
			m_pLogicCtlStatus->SetValidManualCmd(tValidManualCmd); 

			memset(&m_tOldValidManualCmd,0,sizeof(tValidManualCmd));
		}
    }    

	m_pLogicCtlMode->Run();
}

/*==================================================================== 
������ ��PreemptAskPlanRun
���� �������û���Ԥ�Ŀ��Ʒ�ʽ�ͷ������п���
�㷨ʵ�� �� 
����˵�� ����
����ֵ˵������
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ����Ƽ          �������� 
====================================================================*/ 
void COpenATCLogicCtlManager::PreemptAskPlanRun()
{
	TMainCtlBoardRunStatus tMainCtlBoardRunStatus;
    m_pLogicCtlStatus->GetMainCtlBoardRunStatus(tMainCtlBoardRunStatus);
    if (tMainCtlBoardRunStatus.m_bIsNeedGetParam)
    {
	    PrepareParamForSystemAsk(m_tOldPreemptCtlCmd.m_tPatternInterruptCmd.m_nControlMode,m_tOldPreemptCtlCmd.m_tPatternInterruptCmd.m_nPatternNo);
    	    
        m_pLogicCtlStatus->GetMainCtlBoardRunStatus(tMainCtlBoardRunStatus);
        tMainCtlBoardRunStatus.m_bIsNeedGetParam = false;
        m_pLogicCtlStatus->SetMainCtlBoardRunStatus(tMainCtlBoardRunStatus);

        TLogicCtlStatus tCtlStatus;
        m_pLogicCtlStatus->GetLogicCtlStatus(tCtlStatus);
        if (tCtlStatus.m_nCurPlanNo != m_tOldPreemptCtlCmd.m_tPatternInterruptCmd.m_nPatternNo ||
			tCtlStatus.m_nCurCtlMode != m_tOldPreemptCtlCmd.m_tPatternInterruptCmd.m_nControlMode || tCtlStatus.m_nCurPlanNo == MAX_PATTERN_COUNT)
        {
            m_bIsCtlModeChg = true;
            m_nCurPlanNo = m_tOldPreemptCtlCmd.m_tPatternInterruptCmd.m_nPatternNo; 
            m_nCurCtlMode = m_tOldPreemptCtlCmd.m_tPatternInterruptCmd.m_nControlMode; 

			m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "Preempt Ctl Mode is %d, Plan No is %d", m_tOldPreemptCtlCmd.m_tPatternInterruptCmd.m_nControlMode,m_tOldPreemptCtlCmd.m_tPatternInterruptCmd.m_nPatternNo);

        }
    }
        
    if (m_bIsCtlModeChg)
    {
        if (m_pLogicCtlMode != NULL)
        {
            m_pLogicCtlMode->Release();
            delete m_pLogicCtlMode;
            m_pLogicCtlMode = NULL;
        }

        m_pLogicCtlMode = CLogicCtlModeSimpleFactory::Create(m_nCurCtlMode);
        m_pLogicCtlMode->Init(m_pLogicCtlParam,m_pLogicCtlStatus,m_pOpenATCLog,m_nCurPlanNo);
        m_bIsCtlModeChg = false;
		m_pLogicCtlMode->SetSystemUsrCtlFlag(true);  
        m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "Current Ctl Mode is %d, Plan No is %d", m_nCurCtlMode, m_nCurPlanNo);

		//����ϵͳ��������״̬
		SetSystemControlStatus(CONTROL_SUCCEED, 0, CONTROL_SUCCEED, 0, CONTROL_SUCCEED, 0, CONTROL_FAILED, NO_SUPPORT_CONTROL_WAY, CONTROL_SUCCEED, 0);
		//�������ȿ�������״̬
		SetPreemptControlStatus(CONTROL_SUCCEED, 0);

		if (m_tOldPreemptCtlCmd.m_bNewCmd)
		{
			memset(&m_tOldPreemptCtlCmd,0,sizeof(TPreemptCtlCmd));
		}
    }    

	m_pLogicCtlMode->Run();
}

/*==================================================================== 
������ ��CriticalFaultRun
���� �����ع���ʱ����������
�㷨ʵ�� �� 
����˵�� ����
����ֵ˵������
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ������          �������� 
====================================================================*/ 
void COpenATCLogicCtlManager::CriticalFaultRun()
{
    m_nLogicCtlStage = CTL_STAGE_FAULTFORCE;
    if (m_pLogicCtlMode != NULL)
    {
        m_pLogicCtlMode->Release();
        delete m_pLogicCtlMode;
        m_pLogicCtlMode = NULL;
    }

    m_pLogicCtlMode = CLogicCtlModeSimpleFactory::Create(CTL_MODE_FLASH);
    m_nCurCtlMode = CTL_MODE_FLASH;
    m_pLogicCtlMode->Init(m_pLogicCtlParam,m_pLogicCtlStatus,m_pOpenATCLog,0);

    m_pLogicCtlMode->Run();

    TLogicCtlStatus tCtlStatus;
    m_pLogicCtlStatus->GetLogicCtlStatus(tCtlStatus);
    tCtlStatus.m_nRunStage = CTL_STAGE_FAULTFORCE;
    m_pLogicCtlStatus->SetLogicCtlStatus(tCtlStatus);

    m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "Enter serious yellow flash fault!");

	TSystemControlStatus tSystemControlStatus;
	memset(&tSystemControlStatus, 0, sizeof(tSystemControlStatus));
	m_pLogicCtlStatus->GetSystemControlStatus(tSystemControlStatus);
	tSystemControlStatus.m_nSpecicalControlResult = CONTROL_FAILED;
	tSystemControlStatus.m_nSpecicalControlFailCode = NO_SUPPORT_CONTROL_WAY;
	tSystemControlStatus.m_nPatternControlResult = CONTROL_FAILED;
	tSystemControlStatus.m_nPatternControlFailCode = NO_SUPPORT_CONTROL_WAY;
	tSystemControlStatus.m_nStageControlResult = CONTROL_FAILED;
	tSystemControlStatus.m_nStageControlFailCode = NO_SUPPORT_CONTROL_WAY;
	tSystemControlStatus.m_nPhaseControlResult = CONTROL_FAILED;
	tSystemControlStatus.m_nPhaseControlFailCode = NO_SUPPORT_CONTROL_WAY;
	m_pLogicCtlStatus->SetSystemControlStatus(tSystemControlStatus);
}

/*==================================================================== 
������ ��ProcGlobalRunStatus
���� ����������ȫ����λ����״̬��Ϣ
�㷨ʵ�� �� 
����˵�� ����
����ֵ˵������
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ������          �������� 
====================================================================*/ 
void COpenATCLogicCtlManager::ProcGlobalRunStatus()
{
    if (m_pLogicCtlStatus->GetPhaseRunStatusWriteFlag())
    {
        m_pLogicCtlStatus->SetPhaseRunStatusWriteFlag(false);
        if (m_pLogicCtlMode != NULL)
        {
            m_pLogicCtlStatus->SetPhaseRunStatusReadFlag(false);

            TPhaseRunStatus tRunStatus;
            memset(&tRunStatus,0,sizeof(TPhaseRunStatus));   
            m_pLogicCtlStatus->GetPhaseRunStatus(tRunStatus);         
            m_pLogicCtlMode->GetPhaseRunStatus(tRunStatus);

			tRunStatus.m_nCurCtlMode = m_nCurCtlMode;

			TManualCmd  tValidManualCmd;
			memset(&tValidManualCmd,0,sizeof(tValidManualCmd));
			m_pLogicCtlStatus->GetValidManualCmd(tValidManualCmd);
			//������Ԥ��ִ�л�û���е�������Ԥ��ʱ�򣬷��͵�ƽ̨�Ŀ��Ʒ�ʽҪ�ĳɷ����ָ�����
			if (tValidManualCmd.m_bPatternInterruptCmd && tValidManualCmd.m_tPatternInterruptCmd.m_nControlMode == CTL_MODE_SYS_INTERRUPT)
			{
				tRunStatus.m_nCurCtlMode = CTL_MODE_MANUAL_RECOVER;
			}

			if (m_nCurCtlMode == CTL_MODE_MANUAL)
			{
				if (tValidManualCmd.m_nSubCtlMode == CTL_MANUAL_SUBMODE_DEFAULT && tValidManualCmd.m_bChannelLockCmd)
				{
					m_pLogicCtlMode->GetTransRunStatusBeforeLockPhase(tRunStatus);
				}

				if (tValidManualCmd.m_bPatternInterruptCmd && (tValidManualCmd.m_tPatternInterruptCmd.m_nControlMode == CTL_MODE_SELFCTL ||
					tValidManualCmd.m_tPatternInterruptCmd.m_nControlMode == CTL_MODE_FIXTIME ||
					tValidManualCmd.m_tPatternInterruptCmd.m_nControlMode == CTL_MODE_ACTUATE ||
					tValidManualCmd.m_tPatternInterruptCmd.m_nControlMode == CTL_MODE_ADVACTUATE ||
					tValidManualCmd.m_tPatternInterruptCmd.m_nControlMode == CTL_MODE_SINGLEOPTIM ||
					tValidManualCmd.m_tPatternInterruptCmd.m_nControlMode == CTL_MODE_CABLELESS ||
					tValidManualCmd.m_tPatternInterruptCmd.m_nControlMode == CTL_MODE_PEDCROSTREET ||
					tValidManualCmd.m_tPatternInterruptCmd.m_nControlMode == CTL_MODE_WEBSTER_OPTIM ||
					tValidManualCmd.m_tPatternInterruptCmd.m_nControlMode == CTL_MODE_ACTUATE_PEDCROSTREET ||
					tValidManualCmd.m_tPatternInterruptCmd.m_nControlMode == CTL_MODE_PREEMPT))
				{
					tRunStatus.m_nCurCtlMode = CTL_MODE_MANUAL_RECOVER;
				}

				if ((tValidManualCmd.m_nSubCtlMode == CTL_MANUAL_SUBMODE_PANEL_DIRECTION || tValidManualCmd.m_nSubCtlMode == CTL_MANUAL_SUBMODE_SYSTEM_CHANNEL_LOCK) &&
					tRunStatus.m_nCurCtlMode != CTL_MODE_MANUAL_RECOVER)
				{
					if (!tValidManualCmd.m_bPhaseToChannelLock)
					{
						tRunStatus.m_nCurCtlMode = CTL_MODE_CHANNEL_LOCK;
					}
					else
					{
						tRunStatus.m_nCurCtlMode = CTL_MODE_PHASE_LOCK;
					}
				}
			}

			if (tRunStatus.m_nCurCtlMode == CTL_MODE_PHASE_LOCK || tValidManualCmd.m_nSubCtlMode == CTL_MANUAL_SUBMODE_SYSTEM_CHANNEL_LOCK)
			{
				m_pLogicCtlMode->GetLockPhaseRunStatus(tRunStatus, m_bInvalidPhaseCmd, m_tInvalidPhaseLockPara);
			}

            tRunStatus.m_byPlanID = m_nCurPlanNo;
            tRunStatus.m_nCurCtlPattern = m_nCtlSource;

			if (m_nCtlSource == CTL_SOURCE_PREEMPT)
			{
				tRunStatus.m_nCurCtlPattern = CTL_SOURCE_SYSTEM;
			}

            TPhaseLampClrRunCounter tRunCounter;
            m_pLogicCtlStatus->GetPhaseLampClrRunCounter(tRunCounter);
            tRunStatus.m_nCycleRunTime += tRunCounter.m_nLampClrTime[0];
            tRunStatus.m_nCycleRemainTime = tRunStatus.m_nCycleLen - tRunStatus.m_nCycleRunTime;
            if (tRunStatus.m_nCycleRemainTime < 0)
            {
                tRunStatus.m_nCycleRemainTime = 0;
            }            

            m_pLogicCtlStatus->SetPhaseRunStatus(tRunStatus);

            m_pLogicCtlStatus->SetPhaseRunStatusReadFlag(true);
        }        
    }

    if (m_pLogicCtlMode != NULL)
    {
        TLedScreenShowInfo tLedScreenShowInfo;
        memset(&tLedScreenShowInfo,0,sizeof(tLedScreenShowInfo)); 
		m_pLogicCtlStatus->GetLedScreenShowInfo(tLedScreenShowInfo);

		if (m_nCurCtlMode == CTL_MODE_MANUAL)
		{
			TManualCmd  tValidManualCmd;
			memset(&tValidManualCmd,0,sizeof(tValidManualCmd));
			m_pLogicCtlStatus->GetValidManualCmd(tValidManualCmd);

			tLedScreenShowInfo.m_nCurCtlMode = tValidManualCmd.m_nCtlMode;
		    tLedScreenShowInfo.m_nSubCtlMode = tValidManualCmd.m_nSubCtlMode;
			if (tLedScreenShowInfo.m_nSubCtlMode == CTL_MANUAL_SUBMODE_PANEL_DIRECTION)
			{
				tLedScreenShowInfo.m_bKeyDirectionControl = true;
				if (!tValidManualCmd.m_bNewCmd && !tValidManualCmd.m_bDirectionCmd && !tValidManualCmd.m_bPatternInterruptCmd)
				{
					//�Ƿ��������ģʽ�е����������ģʽ���ڷ��������ģʽ�µ�ǰ��������е��µķ���ʱ������ָ���־���ᱻ���������Ӧ���û���ķ�����
					tLedScreenShowInfo.m_nDirectionKeyIndex = m_tOldValidManualCmd.m_tDirectionCmd.m_nNextDirectionIndex;  
				}
			}
			else
			{
				tLedScreenShowInfo.m_bKeyDirectionControl = false;
				tLedScreenShowInfo.m_nDirectionKeyIndex = 0;
			}

			if (tLedScreenShowInfo.m_nSubCtlMode == CTL_MANUAL_SUBMODE_SYSTEM_CHANNEL_LOCK)
			{
				tLedScreenShowInfo.m_bChannelLockCheck = true;
				if (tValidManualCmd.m_bPhaseToChannelLock)
				{
					tLedScreenShowInfo.m_bPhaseToChannelLock = true;
					tLedScreenShowInfo.m_nPhaseLockCount = tValidManualCmd.m_tPhaseLockPara.m_nPhaseLockCount;
					for (int i = 0;i < tLedScreenShowInfo.m_nPhaseLockCount;i++)
					{
						tLedScreenShowInfo.m_tPhaseClr[i].m_byPhaseID = tValidManualCmd.m_tPhaseLockPara.m_nPhaseLockID[i];
					}
				}
				
				if (!tValidManualCmd.m_bNewCmd && !tValidManualCmd.m_bChannelLockCmd && !tValidManualCmd.m_bPatternInterruptCmd)
				{
					//��ͨ��������ģʽ�е�ͨ��������ģʽ����ͨ��������ģʽ�µ�ǰͨ�����������е��µ�ͨ������ʱ��ͨ������ָ���־���ᱻ���������Ӧ���û����ͨ����ɫ 
					memcpy(&tLedScreenShowInfo.m_tChannelLockCtrlCmd, &m_tOldValidManualCmd.m_tChannelLockCmd.m_tNextChannelLockCtrlCmd, sizeof(TChannelLockCtrlCmd));
				}
			}
			else
			{
				tLedScreenShowInfo.m_bChannelLockCheck = false;
				memset(&tLedScreenShowInfo.m_tChannelLockCtrlCmd, 0, sizeof(TChannelLockCtrlCmd));
			}

			if (tValidManualCmd.m_bPatternInterruptCmd && m_nCurCtlMode != tValidManualCmd.m_tPatternInterruptCmd.m_nControlMode)
			{
				//������Ԥָ���·��󣬵�ǰ�Ŀ��Ʒ�ʽ��û���е�ָ�����Ʒ�ʽ�����ʾ�ڹ�����
				/*if (m_nCtlSource == CTL_SOURCE_SYSTEM)
				{
					tLedScreenShowInfo.m_nCurCtlMode = CTL_MODE_SYSTEM_TRAN;
				}
				else
				{
					tLedScreenShowInfo.m_nCurCtlMode = CTL_MODE_PANEL_TRAN;
				}*/
			}
		}
		else
		{
			tLedScreenShowInfo.m_nCurCtlMode = m_nCurCtlMode;
		    tLedScreenShowInfo.m_nSubCtlMode = CTL_MANUAL_SUBMODE_DEFAULT;
			tLedScreenShowInfo.m_bKeyDirectionControl = false;
			tLedScreenShowInfo.m_nDirectionKeyIndex = 0;
			if (m_nCurCtlMode == CTL_MODE_FLASH || m_nCurCtlMode == CTL_MODE_ALLRED || m_nCurCtlMode == CTL_MODE_OFF)
			{
				tLedScreenShowInfo.m_nRingCount = 0;
			}
			tLedScreenShowInfo.m_bChannelLockCheck = false;
		}

		if (tLedScreenShowInfo.m_nCurCtlMode == CTL_MODE_CHANNEL_CHECK)
		{
			static unsigned long nChannelCheckGlobalCounter = 0;

			tLedScreenShowInfo.m_bChannelCheck = true;

            TAscChannelVerifyInfo	atChannelCheckInfo;	
	        memset(&atChannelCheckInfo, 0, sizeof(atChannelCheckInfo));
	        m_pLogicCtlStatus->GetChannelCheckInfo(atChannelCheckInfo);

			if (memcmp(&m_tOldChannelCheckInfo, &atChannelCheckInfo, sizeof(atChannelCheckInfo)) != 0)
			{
				nChannelCheckGlobalCounter = m_pLogicCtlStatus->GetGlobalCounter();
				tLedScreenShowInfo.m_tChannelCheckClr.m_nChannelDurationTime = 0;
				memcpy(&m_tOldChannelCheckInfo, &atChannelCheckInfo, sizeof(atChannelCheckInfo));
			}

			for (int i = 0;i < C_N_MAX_LAMP_OUTPUT_NUM;i++)
			{
			    if (atChannelCheckInfo.m_achLampClr[i] == LAMP_CLR_ON)
				{
					tLedScreenShowInfo.m_tChannelCheckClr.m_nChannelID = i / C_N_CHANNEL_OUTPUTNUM + 1;
					if (i % C_N_CHANNEL_OUTPUTNUM == 0)
					{
					    tLedScreenShowInfo.m_tChannelCheckClr.m_achChannelClr = C_CH_PHASESTAGE_R;
					}
					else if (i % C_N_CHANNEL_OUTPUTNUM == 1)
					{
					    tLedScreenShowInfo.m_tChannelCheckClr.m_achChannelClr = C_CH_PHASESTAGE_Y;
					}
					else if (i % C_N_CHANNEL_OUTPUTNUM == 2)
					{
					    tLedScreenShowInfo.m_tChannelCheckClr.m_achChannelClr = C_CH_PHASESTAGE_G;
					}

					unsigned long nCounterDiff = CalcCounter(nChannelCheckGlobalCounter, m_pLogicCtlStatus->GetGlobalCounter(), C_N_MAXGLOBALCOUNTER);
					if (nCounterDiff >= C_N_TIMER_MILLSECOND)
					{
						nChannelCheckGlobalCounter = m_pLogicCtlStatus->GetGlobalCounter();
					    tLedScreenShowInfo.m_tChannelCheckClr.m_nChannelDurationTime += 1;
					}

					break;
				}
			}
		}
		else
		{
			tLedScreenShowInfo.m_bChannelCheck = false;
			memset(&m_tOldChannelCheckInfo,0,sizeof(m_tOldChannelCheckInfo));
		}
        
        m_pLogicCtlMode->GetLedScreenShowInfo(tLedScreenShowInfo);
		if (m_pLogicCtlStatus->GetLedScreenShowInfoFlag())
		{
			m_pLogicCtlStatus->SetLedScreenShowInfoFlag(false);
			m_pLogicCtlStatus->SetLedScreenShowInfo(tLedScreenShowInfo);
			m_pLogicCtlStatus->SetLedScreenShowInfoFlag(true);
		}
    }
}                             

/*==================================================================== 
������ ��PrepareParam
���� ���������õ�ǰʱ���������Ҫ����������
�㷨ʵ�� �� 
����˵�� ����
����ֵ˵������
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ������          �������� 
====================================================================*/ 
void COpenATCLogicCtlManager::PrepareParam()
{
    TParamRunStatus tParamRunStatus;
    m_pLogicCtlStatus->GetParamRunStatus(tParamRunStatus);

    int nYear,nMonth,nDay,nHour,nMin,nSec,nWeek;
#ifdef VIRTUAL_DEVICE
	bool bSpeedyRunStatu = false;
	bSpeedyRunStatu = m_pLogicCtlStatus->GetIsSpeedyRunStatus();
	if (bSpeedyRunStatu)
	{
		GetVirtualTimeByGlobalCount(nYear, nMonth, nDay, nHour, nMin, nSec, nWeek);
	}
	else
	{
		OpenATCGetCurTime(nYear, nMonth, nDay, nHour, nMin, nSec, nWeek);
	}
#else
	OpenATCGetCurTime(nYear, nMonth, nDay, nHour, nMin, nSec, nWeek);
#endif // VIRTUAL_DEVICE
	//Virtual_Test2022

	//Test
	TTimeBaseSchedule tScheduleInfo;
	TTimeBaseDayPlan tTimeBaseDayPlanInfo;
	TPattern tPatternInfo;
	int nCount = 0;
	int iDatePlanNum = 0;
	//��ȡ������
	iDatePlanNum = m_pLogicCtlParam->GetDatePlanNum();
	
	for (int i = 0; i < iDatePlanNum; i++)
	{
		m_pLogicCtlParam->GetTimeBaseScheduleByTime(nMonth, nWeek, nDay, tScheduleInfo, nCount);
		m_pLogicCtlParam->GetTimeBaseDayPlanByTime(nHour, nMin, tScheduleInfo.m_byTimeBaseScheduleDayPlan, tTimeBaseDayPlanInfo);
		if (/*tTimeBaseDayPlanInfo.m_byDayPlanActionNumberOID == 0 && */ tTimeBaseDayPlanInfo.m_byCoorDination == 1 && tTimeBaseDayPlanInfo.m_byDayPlanControl == 0)
		{
			continue;
		}
		else
		{
			break;	
		}
	}
	m_pLogicCtlParam->GetPatternByPlanNumber(tTimeBaseDayPlanInfo.m_byDayPlanActionNumberOID, tPatternInfo);
    //TTimeBaseSchedule tScheduleInfo;
    //m_pLogicCtlParam->GetTimeBaseScheduleByTime(nMonth,nWeek,nDay,tScheduleInfo);
    //TTimeBaseDayPlan tTimeBaseDayPlanInfo;
    //m_pLogicCtlParam->GetTimeBaseDayPlanByTime(nHour,nMin,tScheduleInfo.m_byTimeBaseScheduleDayPlan,tTimeBaseDayPlanInfo);
    //TPattern tPatternInfo;
    //m_pLogicCtlParam->GetPatternByPlanNumber(tTimeBaseDayPlanInfo.m_byDayPlanActionNumberOID,tPatternInfo);
    
    if (tParamRunStatus.m_chIsParameterChg == C_CH_ISPARAMETERCHG_OK)
    {
        if (!m_bFirstInitFlag)
        {
            memset(m_atOldChannelInfo,0,sizeof(m_atOldChannelInfo));
            m_pLogicCtlParam->GetChannelTable(m_atOldChannelInfo);

            //ʹ���²���
            m_pLogicCtlParam->Init(m_pLogicCtlStatus,m_pOpenATCLog);//�²����ȳ�ʼ��

#ifndef VIRTUAL_DEVICE
            if (IsLampCtlBoardChg())
            {
                m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "Parameter LampCtlBoard Change,Need to Reboot!");
                m_pLogicCtlStatus->SetRebootStatus(true);   
            }
            else
            {
#endif
                m_bIsCtlModeChg = true;
                tParamRunStatus.m_chIsParameterChg = C_CH_ISPARAMETERCHG_NO;
                m_pLogicCtlStatus->SetParamRunStatus(tParamRunStatus);

                //��Ҫ֪ͨ���ϼ��ģ���������������仯��ʹ�����²���.
                TMainCtlBoardRunStatus tMainCtlBoardStatus;
                m_pLogicCtlStatus->GetMainCtlBoardRunStatus(tMainCtlBoardStatus);
                tMainCtlBoardStatus.m_bIsUseNewParamForFault = true;
                tMainCtlBoardStatus.m_bIsUseNewParamForHard = true;
                m_pLogicCtlStatus->SetMainCtlBoardRunStatus(tMainCtlBoardStatus);

				nCount = 0;
				int iDatePlanNum = 0;
				//��ȡ������
				iDatePlanNum = m_pLogicCtlParam->GetDatePlanNum();

				for (int i = 0; i < iDatePlanNum; i++)
				{
					m_pLogicCtlParam->GetTimeBaseScheduleByTime(nMonth, nWeek, nDay, tScheduleInfo, nCount);
					m_pLogicCtlParam->GetTimeBaseDayPlanByTime(nHour, nMin, tScheduleInfo.m_byTimeBaseScheduleDayPlan, tTimeBaseDayPlanInfo);
					if (/*tTimeBaseDayPlanInfo.m_byDayPlanActionNumberOID == 0 && */ tTimeBaseDayPlanInfo.m_byCoorDination == 1 && tTimeBaseDayPlanInfo.m_byDayPlanControl == 0)
					{
						continue;
					}
					else
					{
						break;
					}
				}
				m_pLogicCtlParam->GetPatternByPlanNumber(tTimeBaseDayPlanInfo.m_byDayPlanActionNumberOID, tPatternInfo);
                //m_pLogicCtlParam->GetTimeBaseScheduleByTime(nMonth,nWeek,nDay,tScheduleInfo);
                //m_pLogicCtlParam->GetTimeBaseDayPlanByTime(nHour,nMin,tScheduleInfo.m_byTimeBaseScheduleDayPlan,tTimeBaseDayPlanInfo);
                //m_pLogicCtlParam->GetPatternByPlanNumber(tTimeBaseDayPlanInfo.m_byDayPlanActionNumberOID,tPatternInfo);
#ifndef VIRTUAL_DEVICE
            }
#endif
        }
        else
        {
            m_bIsCtlModeChg = true;
            tParamRunStatus.m_chIsParameterChg = C_CH_ISPARAMETERCHG_NO;
            m_pLogicCtlStatus->SetParamRunStatus(tParamRunStatus);
            
            //��ʼ�Ĳ���Ĭ�ϱ仯������Ҫ֪ͨ���ϼ��ģ��
            m_bFirstInitFlag = false;
        }
    }
    else
    {
        TLogicCtlStatus tCtlStatus;
        m_pLogicCtlStatus->GetLogicCtlStatus(tCtlStatus);
        if (tCtlStatus.m_nCurPlanNo != (int)tTimeBaseDayPlanInfo.m_byDayPlanActionNumberOID ||
            tCtlStatus.m_nCurCtlMode != (int)tTimeBaseDayPlanInfo.m_byDayPlanControl)
        {
            m_bIsCtlModeChg = true;

			//��Ҫ֪ͨ���ϼ��ģ���ж�����λ�����Ƿ��ɺ�����λ����ʱ��Ҫ�����·��̳�ͻ��
            TMainCtlBoardRunStatus tMainCtlBoardStatus;
            m_pLogicCtlStatus->GetMainCtlBoardRunStatus(tMainCtlBoardStatus);
            tMainCtlBoardStatus.m_bIsUseNewParamForFault = true;
            m_pLogicCtlStatus->SetMainCtlBoardRunStatus(tMainCtlBoardStatus);
        }
    }

    if (m_bIsCtlModeChg)
    {
        m_nCurPlanNo = (int)tTimeBaseDayPlanInfo.m_byDayPlanActionNumberOID; 
        m_nCurCtlMode = (int)tTimeBaseDayPlanInfo.m_byDayPlanControl;       
    }
}

/*==================================================================== 
������ PrepareParamForSystemAsk
���� �������û���Ԥ���Ʒ�ʽ�ͷ���ʱ��Ҫ����������
�㷨ʵ�� �� 
����˵�� ��nCtlMode�����Ʒ�ʽ��nPlanNo��������
����ֵ˵������
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ������          �������� 
====================================================================*/ 
void COpenATCLogicCtlManager::PrepareParamForSystemAsk(int nCtlMode,int nPlanNo)
{
    TParamRunStatus tParamRunStatus;
    m_pLogicCtlStatus->GetParamRunStatus(tParamRunStatus);
    
    if (tParamRunStatus.m_chIsParameterChg == C_CH_ISPARAMETERCHG_OK)
    {
        if (!m_bFirstInitFlag)
        {
            memset(m_atOldChannelInfo,0,sizeof(m_atOldChannelInfo));
            m_pLogicCtlParam->GetChannelTable(m_atOldChannelInfo);

            //ʹ���²���
            m_pLogicCtlParam->Init(m_pLogicCtlStatus,m_pOpenATCLog);//�²����ȳ�ʼ��
			m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "PrepareParamForSystemAsk Use NewParam");
#ifndef VIRTUAL_DEVICE
            if (IsLampCtlBoardChg())
            {
                m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "Parameter LampCtlBoard Change,Need to Reboot!");
                m_pLogicCtlStatus->SetRebootStatus(true);   
            }
            else
            {
#endif
                m_bIsCtlModeChg = true;
                tParamRunStatus.m_chIsParameterChg = C_CH_ISPARAMETERCHG_NO;
                m_pLogicCtlStatus->SetParamRunStatus(tParamRunStatus);

                //��Ҫ֪ͨ���ϼ��ģ���������������仯��ʹ�����²���.
                TMainCtlBoardRunStatus tMainCtlBoardStatus;
                m_pLogicCtlStatus->GetMainCtlBoardRunStatus(tMainCtlBoardStatus);
                tMainCtlBoardStatus.m_bIsUseNewParamForFault = true;
                tMainCtlBoardStatus.m_bIsUseNewParamForHard = true;
                m_pLogicCtlStatus->SetMainCtlBoardRunStatus(tMainCtlBoardStatus);
#ifndef VIRTUAL_DEVICE
            }
#endif
        }
        else
        {
            m_bIsCtlModeChg = true;
            tParamRunStatus.m_chIsParameterChg = C_CH_ISPARAMETERCHG_NO;
            m_pLogicCtlStatus->SetParamRunStatus(tParamRunStatus);
            
            //��ʼ�Ĳ���Ĭ�ϱ仯������Ҫ֪ͨ���ϼ��ģ��
            m_bFirstInitFlag = false;
        }
    }
    else
    {
        TLogicCtlStatus tCtlStatus;
        m_pLogicCtlStatus->GetLogicCtlStatus(tCtlStatus);
        if (tCtlStatus.m_nCurPlanNo != nPlanNo ||
            tCtlStatus.m_nCurCtlMode != nCtlMode)
        {
            m_bIsCtlModeChg = true;
        }
    }

    if (m_bIsCtlModeChg)
    {
        m_nCurPlanNo = nPlanNo; 
        m_nCurCtlMode = nCtlMode;       
    }
}

/*==================================================================== 
������ ��ProcFault
���� �����ڷ�����ǰ�Ĺ���״̬
�㷨ʵ�� �� 
����˵�� ����
����ֵ˵������ǰ���ϵȼ�
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ������          �������� 
====================================================================*/ 
int COpenATCLogicCtlManager::ProcFault()
{
    TLampFaultType tFault;
    m_pLogicCtlStatus->GetLampFault(tFault);

    int nRet = NO_FAULT;    
    if (tFault.Bin.bGreenLighConflict_Fault == 1 ||
        tFault.Bin.bLampNumger_Fault == 1 ||
        tFault.Bin.bRedAndGreenConflict_Fault == 1 ||
        tFault.Bin.bRedLightGoOut_Fault == 1 ||
        tFault.Bin.bFaultBoardOffline_Fault)
    {
        nRet = CRITICAL_FAULT;
    }

    return nRet;
} 

/*==================================================================== 
������ ��ProcLampClrRunCounter
���� �����ڼ��㻷���й����еĵ�ɫ��ʱ
�㷨ʵ�� �� 
����˵�� ����
����ֵ˵������
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ������          �������� 
====================================================================*/ 
void COpenATCLogicCtlManager::ProcLampClrRunCounter()
{
    TPhaseLampClrRunCounter tRunCounter;
    m_pLogicCtlStatus->GetPhaseLampClrRunCounter(tRunCounter);

    tRunCounter.m_nCurCounter = m_pLogicCtlStatus->GetLampClrCounter();

    for (int i = 0;i < MAX_RING_COUNT;i ++)
    {
        tRunCounter.m_nLampClrTime[i] = CalcCounter(tRunCounter.m_nLampClrStartCounter[i],tRunCounter.m_nCurCounter,C_N_MAXGLOBALCOUNTER);
        tRunCounter.m_nPedLampClrTime[i] = CalcCounter(tRunCounter.m_nPedLampClrStartCounter[i],tRunCounter.m_nCurCounter,C_N_MAXGLOBALCOUNTER);
    }
    
    m_pLogicCtlStatus->SetPhaseLampClrRunCounter(tRunCounter);
}

/*==================================================================== 
������ ��IsLampCtlBoardChg
���� �����ڼ���ƿذ�ʹ��״̬�Ƿ����仯
�㷨ʵ�� �� 
����˵�� ����
����ֵ˵����
        true,��ʾ�ƿذ�仯
        false,��ʾ�ƿذ�δ�仯
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ������          �������� 
====================================================================*/ 
bool COpenATCLogicCtlManager::IsLampCtlBoardChg()
{
    char achOldLampCtlBoardStatus[C_N_MAXLAMPBOARD_NUM];
    memset(achOldLampCtlBoardStatus,0,sizeof(achOldLampCtlBoardStatus));
    int i = 0;
    for (i = 0;i < MAX_CHANNEL_COUNT;i ++)
    {
        if (m_atOldChannelInfo[i].m_byChannelNumber == 0)
        {
            break;
        }

        int nIndex = (m_atOldChannelInfo[i].m_byChannelNumber - 1) / C_N_CHANNELNUM_PER_BOARD;
        if (achOldLampCtlBoardStatus[nIndex] != 1)
        {
            achOldLampCtlBoardStatus[nIndex] = 1;
        }
    }

    TChannel atChannelInfo[MAX_CHANNEL_COUNT];
    memset(atChannelInfo,0,sizeof(atChannelInfo));
    m_pLogicCtlParam->GetChannelTable(atChannelInfo);

    char achNewLampCtlBoardStatus[C_N_MAXLAMPBOARD_NUM];
    memset(achNewLampCtlBoardStatus,0,sizeof(achNewLampCtlBoardStatus));
    for (i = 0;i < MAX_CHANNEL_COUNT;i ++)
    {
        if (atChannelInfo[i].m_byChannelNumber == 0)
        {
            break;
        }

        int nIndex = (atChannelInfo[i].m_byChannelNumber - 1) / C_N_CHANNELNUM_PER_BOARD;
        if (achNewLampCtlBoardStatus[nIndex] != 1)
        {
            achNewLampCtlBoardStatus[nIndex] = 1;
        }
    }

    if (memcmp(achOldLampCtlBoardStatus,achNewLampCtlBoardStatus,sizeof(achOldLampCtlBoardStatus)) != 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

/*==================================================================== 
������ ��InitUsrCtlLogicObj
���� �������ֶ�����ʱ��ʼ���ֶ����ƶ���
�㷨ʵ�� �� 
����˵�� ����
����ֵ˵������
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ����Ƽ          �������� 
====================================================================*/ 
void COpenATCLogicCtlManager::InitUsrCtlLogicObj()
{
    CLogicCtlMode * pMode = CLogicCtlModeSimpleFactory::Create(CTL_MODE_MANUAL);
    m_nCurCtlMode = CTL_MODE_MANUAL;
    pMode->Init(m_pLogicCtlParam,m_pLogicCtlStatus,m_pOpenATCLog,0);
    m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "Usr Ctl Manual!");

    void * pParam = m_pLogicCtlMode->GetCurCtlParam();
    void * pLampClr = m_pLogicCtlMode->GetCurLampClr();
    void * pStageTable = m_pLogicCtlMode->GetCurStageTable();
    void * pChannelSplitMode = m_pLogicCtlMode->GetCurChannelSplitMode();
    pMode->SetCtlDerivedParam(pParam,pLampClr,pStageTable,pChannelSplitMode);

    m_pLogicCtlMode->Release();
    delete m_pLogicCtlMode;
    m_pLogicCtlMode = NULL;
    m_pLogicCtlMode = pMode;
}

/*==================================================================== 
������ ��CreateCtlMode
���� �����ɿ����߼�
�㷨ʵ�� �� 
����˵�� ��nCtlMode�����Ʒ�ʽ��nCtlSource������Դ
����ֵ˵������
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2020/05/14     V1.0 ����Ƽ          �������� 
====================================================================*/
void COpenATCLogicCtlManager::CreateCtlMode(int nCtlMode, int nCtlSource)
{
	m_pLogicCtlMode->Release();
    delete m_pLogicCtlMode;
    m_pLogicCtlMode = NULL;
            
    m_pLogicCtlMode = CLogicCtlModeSimpleFactory::Create(nCtlMode);
    m_nCurCtlMode = nCtlMode;
    m_pLogicCtlMode->Init(m_pLogicCtlParam,m_pLogicCtlStatus,m_pOpenATCLog,0);
   
    m_nCtlSource = nCtlSource;

	if (m_nCtlSource == CTL_SOURCE_SYSTEM)
	{
		m_pLogicCtlMode->SetSystemUsrCtlFlag(true);  

		if (nCtlMode == CTL_MODE_FLASH)
		{
			m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "System Ctl Yellow Flash!");
		}
		else if (nCtlMode == CTL_MODE_ALLRED)
		{
			m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "System Ctl All Red!");
		}
		else if (nCtlMode == CTL_MODE_OFF)
		{
			m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "System Ctl Lamp Off!");
		}
	}
	else
	{
		m_pLogicCtlMode->SetUsrCtlFlag(true);  

		if (nCtlMode == CTL_MODE_FLASH)
		{
			m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "User Ctl Yellow Flash!");
		}
		else if (nCtlMode == CTL_MODE_ALLRED)
		{
			m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "User Ctl All Red!");
		}
		else if (nCtlMode == CTL_MODE_OFF)
		{
			m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "User Ctl Lamp Off!");
		}
	}
}

/*==================================================================== 
������ ��CreateValidManualCmd
���� ��������Ч�Ŀ���ָ��
�㷨ʵ�� �� 
����˵�� ��tManualCmd������ָ�tValidManualCmd����Ч�Ŀ���ָ��
����ֵ˵������
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2020/05/14     V1.0 ����Ƽ          �������� 
====================================================================*/
void COpenATCLogicCtlManager::CreateValidManualCmd(TManualCmd tManualCmd, TManualCmd & tValidManualCmd)
{
	tValidManualCmd.m_bNewCmd = true;

	if (tManualCmd.m_bPatternInterruptCmd)
	{
		if (tManualCmd.m_tPatternInterruptCmd.m_nControlMode == CTL_MODE_FLASH)
		{
			tValidManualCmd.m_nCtlMode = CTL_MODE_FLASH;
			tValidManualCmd.m_nSubCtlMode = CTL_MANUAL_SUBMODE_DEFAULT;
			tValidManualCmd.m_nCmdSource = tManualCmd.m_nCmdSource;
	        tValidManualCmd.m_nCurCtlSource = m_nCtlSource;
			memcpy(tValidManualCmd.m_szPeerIp,tManualCmd.m_szPeerIp,strlen(tManualCmd.m_szPeerIp));
			tValidManualCmd.m_bStepForwardCmd = false;
			memset(&tValidManualCmd.m_tStepForwardCmd,0,sizeof(TStepForwardCmd));
			tValidManualCmd.m_bDirectionCmd = false;
			memset(&tValidManualCmd.m_tDirectionCmd,0,sizeof(TDirectionCmd));
			tValidManualCmd.m_bPatternInterruptCmd = false;
			memset(&tValidManualCmd.m_tPatternInterruptCmd,0,sizeof(TPatternInterruptCmd));
			tValidManualCmd.m_bChannelLockCmd = false;
			memset(&tValidManualCmd.m_tChannelLockCmd,0,sizeof(TChannelLockCmd));
			memset(&tValidManualCmd.m_tPhaseLockPara,0,sizeof(TPhaseLockPara));
			tValidManualCmd.m_bPhaseToChannelLock = false;
			tValidManualCmd.m_bPreemptCtlCmd = false;
		}
		else if (tManualCmd.m_tPatternInterruptCmd.m_nControlMode == CTL_MODE_ALLRED)
		{
			tValidManualCmd.m_nCtlMode = CTL_MODE_ALLRED;
			tValidManualCmd.m_nSubCtlMode = CTL_MANUAL_SUBMODE_DEFAULT;
			tValidManualCmd.m_nCmdSource = tManualCmd.m_nCmdSource;
	        tValidManualCmd.m_nCurCtlSource = m_nCtlSource;
			memcpy(tValidManualCmd.m_szPeerIp,tManualCmd.m_szPeerIp,strlen(tManualCmd.m_szPeerIp));
			tValidManualCmd.m_bStepForwardCmd = false;
			memset(&tValidManualCmd.m_tStepForwardCmd,0,sizeof(TStepForwardCmd));
			tValidManualCmd.m_bDirectionCmd = false;
			memset(&tValidManualCmd.m_tDirectionCmd,0,sizeof(TDirectionCmd));
			tValidManualCmd.m_bPatternInterruptCmd = false;
			memset(&tValidManualCmd.m_tPatternInterruptCmd,0,sizeof(TPatternInterruptCmd));
			tValidManualCmd.m_bChannelLockCmd = false;
			memset(&tValidManualCmd.m_tChannelLockCmd,0,sizeof(TChannelLockCmd));
			memset(&tValidManualCmd.m_tPhaseLockPara,0,sizeof(TPhaseLockPara));
			tValidManualCmd.m_bPhaseToChannelLock = false;
			tValidManualCmd.m_bPreemptCtlCmd = false;
		}
		else if (tManualCmd.m_tPatternInterruptCmd.m_nControlMode == CTL_MODE_OFF)
		{
			tValidManualCmd.m_nCtlMode = CTL_MODE_OFF;
			tValidManualCmd.m_nSubCtlMode = CTL_MANUAL_SUBMODE_DEFAULT;
			tValidManualCmd.m_nCmdSource = tManualCmd.m_nCmdSource;
	        tValidManualCmd.m_nCurCtlSource = m_nCtlSource;
			memcpy(tValidManualCmd.m_szPeerIp,tManualCmd.m_szPeerIp,strlen(tManualCmd.m_szPeerIp));
			tValidManualCmd.m_bStepForwardCmd = false;
			memset(&tValidManualCmd.m_tStepForwardCmd,0,sizeof(TStepForwardCmd));
			tValidManualCmd.m_bDirectionCmd = false;
			memset(&tValidManualCmd.m_tDirectionCmd,0,sizeof(TDirectionCmd));
			tValidManualCmd.m_bPatternInterruptCmd = false;
			memset(&tValidManualCmd.m_tPatternInterruptCmd,0,sizeof(TPatternInterruptCmd));
			tValidManualCmd.m_bChannelLockCmd = false;
			memset(&tValidManualCmd.m_tChannelLockCmd,0,sizeof(TChannelLockCmd));
			memset(&tValidManualCmd.m_tPhaseLockPara,0,sizeof(TPhaseLockPara));
			tValidManualCmd.m_bPhaseToChannelLock = false;
			tValidManualCmd.m_bPreemptCtlCmd = false;
		}
		else //�ص������򷽰���Ԥ
		{
			if (tManualCmd.m_nCurCtlSource != CTL_SOURCE_SELF)//�û���Ԥ��ʱ�ص����������ʱ����Դֻ���������ƻ���ϵͳ����
			{
				tValidManualCmd.m_nCtlMode = CTL_MODE_SELFCTL;//��������
				tValidManualCmd.m_nSubCtlMode = CTL_MANUAL_SUBMODE_DEFAULT;
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "CreateValidManualCmd, User Intervention Timeout Return To Self");
			}
			else
			{
				if (tValidManualCmd.m_nSubCtlMode == CTL_MANUAL_SUBMODE_PANEL_FITSTMANUAL)
				{
					//��ģʽ������ֶ�ģʽ�£�ֱ���е�Ĭ��ģʽ��������
					tValidManualCmd.m_nSubCtlMode = CTL_MANUAL_SUBMODE_DEFAULT;
				}
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "CreateValidManualCmd, Return To ControlMode:%d SubControlMode:%d", tManualCmd.m_tPatternInterruptCmd.m_nControlMode, tValidManualCmd.m_nSubCtlMode);
			}
			tValidManualCmd.m_nCmdSource = tManualCmd.m_nCmdSource;//����������Դ
			tValidManualCmd.m_nCurCtlSource = m_nCtlSource;//���ÿ���Դ
			memcpy(tValidManualCmd.m_szPeerIp,tManualCmd.m_szPeerIp,strlen(tManualCmd.m_szPeerIp));
			tValidManualCmd.m_bStepForwardCmd = false;
			memset(&tValidManualCmd.m_tStepForwardCmd,0,sizeof(TStepForwardCmd));
			tValidManualCmd.m_bDirectionCmd = false;
			memset(&tValidManualCmd.m_tDirectionCmd,0,sizeof(TDirectionCmd));
			tValidManualCmd.m_bPatternInterruptCmd = true;
			tValidManualCmd.m_tPatternInterruptCmd.m_nControlMode = tManualCmd.m_tPatternInterruptCmd.m_nControlMode;
			tValidManualCmd.m_tPatternInterruptCmd.m_nPatternNo = tManualCmd.m_tPatternInterruptCmd.m_nPatternNo;
			if (tManualCmd.m_tPatternInterruptCmd.m_nControlMode == CTL_MODE_MANUAL_CONTROL_PATTERN)
			{
				tValidManualCmd.m_nSubCtlMode = CTL_MANUAL_SUBMODE_SYSTEM_INTERRUPT_PATTERN;
				memcpy(&tValidManualCmd.m_tPatternInterruptCmd.m_tManualControlPattern,&tManualCmd.m_tPatternInterruptCmd.m_tManualControlPattern,sizeof(tManualCmd.m_tPatternInterruptCmd.m_tManualControlPattern));
				m_tPatternInterruptCmdTime = time(NULL);
			}
			tValidManualCmd.m_bChannelLockCmd = false;
			memset(&tValidManualCmd.m_tChannelLockCmd,0,sizeof(TChannelLockCmd));
			memset(&tValidManualCmd.m_tPhaseLockPara,0,sizeof(TPhaseLockPara));
			tValidManualCmd.m_bPhaseToChannelLock = false;
			tValidManualCmd.m_bPreemptCtlCmd = false;
		}
	}
	else if (tManualCmd.m_bStepForwardCmd)
	{
		tValidManualCmd.m_nCtlMode = CTL_MODE_MANUAL;
		if (m_nCtlSource == CTL_SOURCE_LOCAL)
		{
			if (tValidManualCmd.m_nSubCtlMode == CTL_MANUAL_SUBMODE_PANEL_FITSTMANUAL)
			{
				//��ģʽ������ֶ�ģʽ�£�ֱ���е���岽��ģʽ��������
				tValidManualCmd.m_nSubCtlMode = CTL_MANUAL_SUBMODE_PANEL_STEPFOWARD;
			}
			m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "CreateValidManualCmd, Trans To Panel StepWard");
		}
		else
		{
			if (tValidManualCmd.m_nSubCtlMode == CTL_MANUAL_SUBMODE_DEFAULT ||
			   (m_nCtlSource == CTL_SOURCE_SELF && tValidManualCmd.m_nSubCtlMode == CTL_MANUAL_SUBMODE_PANEL_STEPFOWARD))
			{
				//1.��ģʽ��Ĭ��ģʽ�£�ֱ���е�ϵͳ����ģʽ��������
				//2.��岽���ڻ�������·�ϣ�����ϵͳ��������ģʽ�ĳ�ϵͳ����ģʽ
				tValidManualCmd.m_nSubCtlMode = CTL_MANUAL_SUBMODE_SYSTEM_STEPFOWARD;
				m_pLogicCtlMode->SetSystemUsrCtlFlag(true); 
			}
			m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "CreateValidManualCmd, Trans To System StepWard");
		}
		tValidManualCmd.m_nCmdSource = tManualCmd.m_nCmdSource;
	    tValidManualCmd.m_nCurCtlSource = m_nCtlSource;
		memcpy(tValidManualCmd.m_szPeerIp,tManualCmd.m_szPeerIp,strlen(tManualCmd.m_szPeerIp));
		tValidManualCmd.m_bStepForwardCmd = true;
		memcpy(&tValidManualCmd.m_tStepForwardCmd,&tManualCmd.m_tStepForwardCmd,sizeof(TStepForwardCmd));
		tValidManualCmd.m_bDirectionCmd = false;
		memset(&tValidManualCmd.m_tDirectionCmd,0,sizeof(TDirectionCmd));
		tValidManualCmd.m_bPatternInterruptCmd = false;
		memset(&tValidManualCmd.m_tPatternInterruptCmd,0,sizeof(TPatternInterruptCmd));
		tValidManualCmd.m_bChannelLockCmd = false;
		memset(&tValidManualCmd.m_tChannelLockCmd,0,sizeof(TChannelLockCmd));
		memset(&tValidManualCmd.m_tPhaseLockPara,0,sizeof(TPhaseLockPara));
		tValidManualCmd.m_bPhaseToChannelLock = false;
		tValidManualCmd.m_bPreemptCtlCmd = false;
	}
	else if (tManualCmd.m_bDirectionCmd)
	{
		tValidManualCmd.m_nCmdSource = tManualCmd.m_nCmdSource;
	    tValidManualCmd.m_nCurCtlSource = m_nCtlSource;
		memcpy(tValidManualCmd.m_szPeerIp,tManualCmd.m_szPeerIp,strlen(tManualCmd.m_szPeerIp));
		tValidManualCmd.m_bStepForwardCmd = false;
		memset(&tValidManualCmd.m_tStepForwardCmd,0,sizeof(TStepForwardCmd));
		tValidManualCmd.m_bDirectionCmd = true;
		memcpy(&tValidManualCmd.m_tDirectionCmd,&tManualCmd.m_tDirectionCmd,sizeof(TDirectionCmd));
		tValidManualCmd.m_bPatternInterruptCmd = false;
		memset(&tValidManualCmd.m_tPatternInterruptCmd,0,sizeof(TPatternInterruptCmd));
		tValidManualCmd.m_bChannelLockCmd = false;
		memset(&tValidManualCmd.m_tChannelLockCmd,0,sizeof(TChannelLockCmd));
		memset(&tValidManualCmd.m_tPhaseLockPara,0,sizeof(TPhaseLockPara));
		tValidManualCmd.m_bPhaseToChannelLock = false;
		tValidManualCmd.m_bPreemptCtlCmd = false;
		m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "CreateValidManualCmd, Trans To Panel DirectionIndex:%d", tValidManualCmd.m_tDirectionCmd.m_nNextDirectionIndex);

		if (m_tOldValidManualCmd.m_nSubCtlMode == CTL_MANUAL_SUBMODE_PANEL_FITSTMANUAL)
		{
			tValidManualCmd.m_nSubCtlMode = CTL_MANUAL_SUBMODE_DEFAULT;//��ģʽ����ΪĬ�ϣ��ȵ�ǰ�׶ι��ɽ�����ʼ�������ʱ����Ϊ������ģʽ
		}

		//������ȫ��͹ص��з���
		if (m_nCurCtlMode == CTL_MODE_FLASH || m_nCurCtlMode == CTL_MODE_ALLRED || m_nCurCtlMode == CTL_MODE_OFF)
		{
			InitUsrCtlLogicObj();
			m_nCurCtlMode = CTL_MODE_MANUAL;
			tValidManualCmd.m_nCtlMode = CTL_MODE_MANUAL;
			tValidManualCmd.m_nSubCtlMode = CTL_MANUAL_SUBMODE_PANEL_DIRECTION;//�ӻ�����ȫ�죬�ص��е�������Ƶ�ֱ�Ӱ���ģʽ��Ϊ������ģʽ
			tValidManualCmd.m_bDirectionCmd = false;//�е����������ģʽ�£������ǰ����ָ���־
		}
	}
	else if (tManualCmd.m_bChannelLockCmd)
	{
		tValidManualCmd.m_nCtlMode = CTL_MODE_MANUAL;
		tValidManualCmd.m_nCmdSource = tManualCmd.m_nCmdSource;
	    tValidManualCmd.m_nCurCtlSource = m_nCtlSource;
		memcpy(tValidManualCmd.m_szPeerIp,tManualCmd.m_szPeerIp,strlen(tManualCmd.m_szPeerIp));
		tValidManualCmd.m_bStepForwardCmd = false;
		memset(&tValidManualCmd.m_tStepForwardCmd,0,sizeof(TStepForwardCmd));
		tValidManualCmd.m_bDirectionCmd = false;
		memset(&tValidManualCmd.m_tDirectionCmd,0,sizeof(TDirectionCmd));
		tValidManualCmd.m_bPatternInterruptCmd = false;
		memset(&tValidManualCmd.m_tPatternInterruptCmd,0,sizeof(TPatternInterruptCmd));
		tValidManualCmd.m_bChannelLockCmd = true;
		memcpy(&tValidManualCmd.m_tChannelLockCmd,&tManualCmd.m_tChannelLockCmd,sizeof(TChannelLockCmd));
		tValidManualCmd.m_bPhaseToChannelLock = tManualCmd.m_bPhaseToChannelLock;
		memcpy(&tValidManualCmd.m_tPhaseLockPara,&tManualCmd.m_tPhaseLockPara,sizeof(TPhaseLockPara));
		tValidManualCmd.m_bPreemptCtlCmd = false;
		m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "CreateValidManualCmd, Trans To ChannelLock");

		//������ȫ��͹ص��з���
		if (m_nCurCtlMode == CTL_MODE_FLASH || m_nCurCtlMode == CTL_MODE_ALLRED || m_nCurCtlMode == CTL_MODE_OFF)
		{
			InitUsrCtlLogicObj();
			m_nCurCtlMode = CTL_MODE_MANUAL;
			tValidManualCmd.m_nCtlMode = CTL_MODE_MANUAL;
			tValidManualCmd.m_nSubCtlMode = CTL_MANUAL_SUBMODE_SYSTEM_CHANNEL_LOCK;//�ӻ�����ȫ�죬�ص��е�������Ƶ�ֱ�Ӱ���ģʽ��Ϊͨ��������ģʽ
			tValidManualCmd.m_bChannelLockCmd = false;//�е�ͨ��������ģʽ�£������ǰͨ������ָ���־
		}
	}

	m_pLogicCtlStatus->SetValidManualCmd(tValidManualCmd);
}

/*==================================================================== 
������ ��UsrCtlYellowFalsh
���� ���ֶ�����������
�㷨ʵ�� �� 
����˵�� ��tValidManualCmd����Ч���ֶ����ָ��
����ֵ˵������
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ����Ƽ          �������� 
====================================================================*/ 
void COpenATCLogicCtlManager::UsrCtlYellowFalsh(TManualCmd tValidManualCmd)
{
    if (m_nCurCtlMode != CTL_MODE_FLASH)
    {
		//��������ģʽ
        CreateCtlMode(CTL_MODE_FLASH, CTL_SOURCE_LOCAL);

        SetPanelBtnStatusReply(tValidManualCmd, HWPANEL_BTN_YELLOW_FLASH);
    } 
}

/*==================================================================== 
������ ��UsrCtlAllRed
���� ���ֶ����ȫ�촦��
�㷨ʵ�� �� 
����˵�� ��tValidManualCmd����Ч���ֶ����ָ��
����ֵ˵������
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ����Ƽ          �������� 
====================================================================*/ 
void COpenATCLogicCtlManager::UsrCtlAllRed(TManualCmd tValidManualCmd)
{
    if (m_nCurCtlMode != CTL_MODE_ALLRED)
    {
		//����ȫ��ģʽ
        CreateCtlMode(CTL_MODE_ALLRED, CTL_SOURCE_LOCAL);

        SetPanelBtnStatusReply(tValidManualCmd, HWPANEL_BTN_ALL_RED);
    }
}

/*==================================================================== 
������ ��UsrCtlStepForward
���� ���ֶ���岽������
�㷨ʵ�� �� 
����˵�� ��tManualCmd���ֶ���岽�����tValidManualCmd:��Ч���ֶ����ָ��
����ֵ˵������
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ����Ƽ          �������� 
====================================================================*/ 
void COpenATCLogicCtlManager::UsrCtlStepForward(TManualCmd tManualCmd, TManualCmd & tValidManualCmd)
{
    bool bCheckBtnUseStatus = CheckPanelBtnUseStatus(tManualCmd, tValidManualCmd);

	//������ִ��
	if (bCheckBtnUseStatus)
	{
		//������Ч�Ŀ���ָ��
	    CreateValidManualCmd(tManualCmd, tValidManualCmd);
		SetPanelBtnStatusReply(tValidManualCmd, HWPANEL_BTN_STEP);
	}
}

/*==================================================================== 
������ ��UsrCtlDirectionKey
���� ���ֶ���巽�������
�㷨ʵ�� �� 
����˵�� ��tManualCmd���ֶ���岽�����tValidManualCmd:��Ч���ֶ����ָ��
����ֵ˵������
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ����Ƽ          �������� 
====================================================================*/ 
void COpenATCLogicCtlManager::UsrCtlDirectionKey(TManualCmd tManualCmd, TManualCmd & tValidManualCmd)
{
    bool bCheckBtnUseStatus = CheckPanelBtnUseStatus(tManualCmd, tValidManualCmd);

	//�жϷ����Ƿ���ִ��
	if (bCheckBtnUseStatus)
	{
		//������Ч�Ŀ���ָ��
	    CreateValidManualCmd(tManualCmd, tValidManualCmd);
		SetPanelBtnStatusReply(tValidManualCmd, GetManualBtnByIndex(tManualCmd.m_tDirectionCmd.m_nNextDirectionIndex));
	}
}

/*==================================================================== 
������ ��CheckPanelBtnUseStatus
���� ���ж���尴ť�ܷ�ʹ��
�㷨ʵ�� �� 
����˵�� ��tManualCmd���ֶ����ָ��
����ֵ˵������
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ����Ƽ          �������� 
====================================================================*/ 
bool COpenATCLogicCtlManager::CheckPanelBtnUseStatus(TManualCmd tManualCmd, TManualCmd tValidManualCmd)
{
    if (tManualCmd.m_bStepForwardCmd)
    {
		//��ǰ����ģʽ�ǻ�����ȫ�죬�صƻ��߿�����ģʽΪ������Ƶ�ʱ�򣬲��������ִ��
        if (m_nCurCtlMode == CTL_MODE_FLASH || m_nCurCtlMode == CTL_MODE_ALLRED || m_nCurCtlMode == CTL_MODE_OFF || tValidManualCmd.m_nSubCtlMode == CTL_MANUAL_SUBMODE_PANEL_DIRECTION)
        {
			m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "StepForward can not be use");
			//��尴���Զ��󣬷���ʼ����ʱ���ٰ��ֶ��ǲ���ִ�еģ���ʱΪ��Ӳ��ģ���m_bManualBtn����false����Ҫ�ٴη���������ť����״̬
			/*if (tValidManualCmd.m_nSubCtlMode == CTL_MANUAL_SUBMODE_PANEL_DIRECTION)
			{
				THWPanelBtnStatus tHWPanelBtnStatus;
				tHWPanelBtnStatus.m_nHWPanelBtnIndex = HWPANEL_BTN_AUTO;
				tHWPanelBtnStatus.m_bHWPanelBtnStatus = true;
				m_pLogicCtlStatus->SetPanelBtnStatusList(tHWPanelBtnStatus);
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "Set PanelAutoBtn Press Status");
			}
			WriteOperationRecord(CTL_SOURCE_LOCAL, HWPANEL_BTN_STEP, false, tManualCmd.m_szPeerIp);*/
            return false;
        }
    }
    else
    {
		if (!tValidManualCmd.m_bDirectionCmd && tValidManualCmd.m_nSubCtlMode == CTL_MANUAL_SUBMODE_PANEL_DIRECTION &&//��ǰ���ڽ��з������
			tManualCmd.m_tDirectionCmd.m_nNextDirectionIndex == m_tOldValidManualCmd.m_tDirectionCmd.m_nNextDirectionIndex)
		{
			if (m_pLogicCtlMode->IsChannelGreen(CHANNEL_TYPE_DIRECTION, m_tOldValidManualCmd.m_tDirectionCmd.m_nNextDirectionIndex, NULL))
			{
				 m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "Same Direction Key, Direction is Green, can not be use");
				 return false;//ͬһ�����򣬲��Ҹ÷������̣����ִ��
			}
		}

        int nChannelCount = 0, nMaxChannelID = 0;
        TChannel atChannelInfo[MAX_CHANNEL_COUNT];            //ͨ����Ϣ��
        memset(atChannelInfo,0,sizeof(atChannelInfo));
        m_pLogicCtlParam->GetChannelTable(atChannelInfo);

        int i = 0, j = 0;
        for (i = 0;i < MAX_CHANNEL_COUNT;i++)
        {
			if (atChannelInfo[i].m_byChannelNumber > nMaxChannelID)
			{
				nMaxChannelID = atChannelInfo[i].m_byChannelNumber;
			}
        }
		nChannelCount = nMaxChannelID;

        TAscManualPanel tAscManualPanel;
        memset(&tAscManualPanel, 0, sizeof(TAscManualPanel)); 
        m_pLogicCtlParam->GetManualPanelInfo(tAscManualPanel);

        bool bAllChannelDefault = true;
        //��ȡ��ǰͨ����Ҫ�л���Ŀ���ɫ
        for (i = 0;i < MAX_PANEL_KEY_CFG_COUNT;i++)
	    {
		    if (tAscManualPanel.m_stPanelKeyCfg[i].m_byKeyNum == tManualCmd.m_tDirectionCmd.m_nNextDirectionIndex)
		    {    
			    for (j = 0;j < nChannelCount;j++)
			    {
				    if (tAscManualPanel.m_stPanelKeyCfg[i].m_ChannelCfg[j].m_byChannelStatus != CHANNEL_STATUS_DEFAULT)
				    {
					    bAllChannelDefault = false;
						break;
				    }
			    }
		    } 
	    }

        if (bAllChannelDefault)//����ť��Ӧ�ĵ�ɫֵȫ����Ĭ�ϣ�������Ӧ
        {
            m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "KeyIndex:%d! All Lamp is Default!", tManualCmd.m_tDirectionCmd.m_nNextDirectionIndex);
            WriteOperationRecord(CTL_SOURCE_LOCAL, tManualCmd.m_tDirectionCmd.m_nNextDirectionIndex, false, tManualCmd.m_szPeerIp);
			return false;
        }

        bool bAllChannelRed = true;
        //��ȡ��ǰͨ����Ҫ�л���Ŀ���ɫ
        for (i = 0;i < MAX_PANEL_KEY_CFG_COUNT;i++)
	    {
		    if (tAscManualPanel.m_stPanelKeyCfg[i].m_byKeyNum == tManualCmd.m_tDirectionCmd.m_nNextDirectionIndex)
		    {    
			    for (j = 0;j < nChannelCount;j++)
			    {
				    if (tAscManualPanel.m_stPanelKeyCfg[i].m_ChannelCfg[j].m_byChannelStatus != CHANNEL_STATUS_RED)
				    {
					    bAllChannelRed = false;
						break;
				    }
			    }
		    } 
	    }

        if (bAllChannelRed)//����ť��Ӧ�ĵ�ɫֵȫ����ȫ�죬������Ӧ
        {
            m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "KeyIndex:%d! All Lamp is Red!", tManualCmd.m_tDirectionCmd.m_nNextDirectionIndex);
			WriteOperationRecord(CTL_SOURCE_LOCAL, tManualCmd.m_tDirectionCmd.m_nNextDirectionIndex, false, tManualCmd.m_szPeerIp);
            return false;
        }
    }

    return true;
}

/*==================================================================== 
������ ��CheckSystemCmdUseStatus
���� ���ж�ϵͳ���ָ���ܷ�ʹ��
�㷨ʵ�� �� 
����˵�� ��tManualCmd��ϵͳ���ָ��
����ֵ˵������
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ����Ƽ          �������� 
====================================================================*/ 
bool COpenATCLogicCtlManager::CheckSystemCmdUseStatus(TManualCmd tManualCmd, TManualCmd tValidManualCmd)
{
	bool bFlag = true;
    if (m_nCtlSource == CTL_SOURCE_LOCAL && m_nCurCtlMode != CTL_MODE_SELFCTL)
    {
		bFlag = false;
    }

	if (tManualCmd.m_bStepForwardCmd)
    {
        if (m_nCurCtlMode == CTL_MODE_ALLRED || m_nCurCtlMode == CTL_MODE_FLASH || m_nCurCtlMode == CTL_MODE_OFF)
		{
			bFlag = false;
		}
	}

	if (tManualCmd.m_bChannelLockCmd)
	{
		m_bInvalidPhaseCmd = false;
		if (!tValidManualCmd.m_bChannelLockCmd && tValidManualCmd.m_nSubCtlMode == CTL_MANUAL_SUBMODE_SYSTEM_CHANNEL_LOCK &&//��ǰ���ڽ���ͨ����������
			memcmp(tManualCmd.m_tChannelLockCmd.m_tNextChannelLockCtrlCmd.m_nChannelLockStatus, 
			m_tOldValidManualCmd.m_tChannelLockCmd.m_tNextChannelLockCtrlCmd.m_nChannelLockStatus, MAX_CHANNEL_COUNT) == 0)
		{
			if (m_pLogicCtlMode->IsChannelGreen(CHANNEL_TYPE_LOCK, 0, m_tOldValidManualCmd.m_tChannelLockCmd.m_tNextChannelLockCtrlCmd.m_nChannelLockStatus))
			{
				 m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "Same ChannelLock, Channel is Green, can not be use");
				 
				 memset(&m_tInvalidPhaseLockPara, 0, sizeof(tManualCmd.m_tPhaseLockPara));
				 if (tManualCmd.m_bPhaseToChannelLock)
				 {
					 m_bInvalidPhaseCmd = true;
					 memcpy(&m_tInvalidPhaseLockPara, &tManualCmd.m_tPhaseLockPara, sizeof(tManualCmd.m_tPhaseLockPara));
				 }
				 return false;//ͬһ��ͨ�����������Ҹ�ͨ�������̣����ִ��
			}
		}
	}

	if (bFlag)
	{
		if (tManualCmd.m_bPatternInterruptCmd)
		{
			WriteOperationRecord(CTL_SOURCE_SYSTEM, tManualCmd.m_tPatternInterruptCmd.m_nControlMode, true, tManualCmd.m_szPeerIp);	
		}
		else 
		{
			WriteOperationRecord(CTL_SOURCE_SYSTEM, CTL_MODE_MANUAL, true, tManualCmd.m_szPeerIp);	
		}
	}
	else
	{
		if (tManualCmd.m_bPatternInterruptCmd)
		{
			WriteOperationRecord(CTL_SOURCE_SYSTEM, tManualCmd.m_tPatternInterruptCmd.m_nControlMode, false, tManualCmd.m_szPeerIp);	
		}
		else 
		{
			WriteOperationRecord(CTL_SOURCE_SYSTEM, CTL_MODE_MANUAL, false, tManualCmd.m_szPeerIp);	
		}
	}

    return bFlag;

}

/*==================================================================== 
������ ��CheckPreemptCmdUseStatus
���� ���ж����ȿ���ָ���ܷ�ʹ��
�㷨ʵ�� �� 
����˵�� ��tPreemptCtlCmd�����ȿ�������
����ֵ˵������
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ����Ƽ          �������� 
====================================================================*/ 
bool COpenATCLogicCtlManager::CheckPreemptCmdUseStatus(TPreemptCtlCmd tPreemptCtlCmd)
{
	bool bFlag = true;
    if (m_nCtlSource == CTL_SOURCE_LOCAL || m_nCtlSource == CTL_SOURCE_SYSTEM)
    {
		bFlag = false;
    }
	//���ػ�ϵͳ�·��˷�����Ԥָ�������Դ�����ȿ��Ƶ����ȿ����������Ӧ
	if (m_tOldPreemptCtlCmd.m_nCmdSource == CTL_SOURCE_SYSTEM && m_tOldPreemptCtlCmd.m_bPatternInterruptCmd && tPreemptCtlCmd.m_nCmdSource == CTL_SOURCE_PREEMPT)
    {
		bFlag = false;
		m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "GetPreemptCtlCmdFromList, System PatternInterrupt, PreemptCtlCmd Can't Execute");
    }

    return bFlag;

}

/*==================================================================== 
������ ��SetPanelBtnStatusReply
���� ��������尴ť�ظ�״̬
�㷨ʵ�� �� 
����˵�� ��tValidManualCmd����Ч���ֶ����ָ�nHWPanelBtnIndex���������
����ֵ˵������
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ����Ƽ          �������� 
====================================================================*/ 
void COpenATCLogicCtlManager::SetPanelBtnStatusReply(TManualCmd tValidManualCmd, int nHWPanelBtnIndex)
{
	THWPanelBtnStatus tHWPanelBtnStatus;
    tHWPanelBtnStatus.m_nHWPanelBtnIndex = nHWPanelBtnIndex;
    tHWPanelBtnStatus.m_bHWPanelBtnStatus = true;
	m_pLogicCtlStatus->SetPanelBtnStatusList(tHWPanelBtnStatus);

    memcpy(&m_tOldValidManualCmd,&tValidManualCmd,sizeof(TManualCmd));
     
    WriteOperationRecord(CTL_SOURCE_LOCAL, nHWPanelBtnIndex, true, m_atNetConfig[0].m_chAscNetCardIp);
}

/*==================================================================== 
������ ��WriteOperationRecord
���� ��д������־
�㷨ʵ�� �� 
����˵�� ��nCtlSource������Դ
           nOperationType����������
           bStatus���������
		   szPeerIp��IP
����ֵ˵������
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ����Ƽ          �������� 
====================================================================*/ 
void COpenATCLogicCtlManager::WriteOperationRecord(int nCtlSource, int nOperationType, bool bStatus, char szPeerIp[])
{
    if (bStatus)
    {
        TAscOperationRecord tAscOperationRecord;//������¼
        memset(&tAscOperationRecord, 0, sizeof(tAscOperationRecord));

	    tAscOperationRecord.m_unStartTime = time(NULL);
        tAscOperationRecord.m_unEndTime = time(NULL);
        if (nCtlSource == CTL_SOURCE_LOCAL)
        {
	        tAscOperationRecord.m_bySubject = 2;
        }
        else if (nCtlSource == CTL_SOURCE_TZPARAM)
        {
            tAscOperationRecord.m_bySubject = 1;
        }
		else if (nCtlSource == CTL_SOURCE_SYSTEM)
        {
            tAscOperationRecord.m_bySubject = 0;
        }
	    tAscOperationRecord.m_byObject = 1;
        tAscOperationRecord.m_bStatus = true;
		if (nCtlSource == CTL_SOURCE_LOCAL)
		{
			tAscOperationRecord.m_nInfoType = LOCAL_MANUALPANEL;
			switch (nOperationType)
			{
			case HWPANEL_BTN_AUTO:
				{
					sprintf((char *)tAscOperationRecord.m_byFailureValue, "IP is %s, Local Control Enter into AutoCtl", m_atNetConfig[0].m_chAscNetCardIp);
				}
				break;
			case HWPANEL_BTN_MANUAL:
				{
					sprintf((char *)tAscOperationRecord.m_byFailureValue, "IP is %s, Local Control Enter into ManualCtl", m_atNetConfig[0].m_chAscNetCardIp);
				}
				break;
			case HWPANEL_BTN_STEP:
				{
					 sprintf((char *)tAscOperationRecord.m_byFailureValue, "IP is %s, Local Control Enter into StepForward", m_atNetConfig[0].m_chAscNetCardIp);
				}
				break;
			case HWPANEL_BTN_ALL_RED:
				{
					sprintf((char *)tAscOperationRecord.m_byFailureValue, "IP is %s, Local Control Enter into AllRed", m_atNetConfig[0].m_chAscNetCardIp);
				}
				break;
			case HWPANEL_BTN_YELLOW_FLASH:
				{
					sprintf((char *)tAscOperationRecord.m_byFailureValue, "IP is %s, Local Control Enter into YellowFlash", m_atNetConfig[0].m_chAscNetCardIp);
				}
				break;
			case HWPANEL_BTN_DIR_EAST_WEST_STRAIGHT:
				{
					sprintf((char *)tAscOperationRecord.m_byFailureValue, "IP is %s, Local Control Enter into direction control east west straight", m_atNetConfig[0].m_chAscNetCardIp);
				}
				break;
			case HWPANEL_BTN_DIR_EAST_WEST_TURN_LEFT:
				{
					sprintf((char *)tAscOperationRecord.m_byFailureValue, "IP is %s, Local Control Enter into direction control east west turn left", m_atNetConfig[0].m_chAscNetCardIp);
				}
				break;
			case HWPANEL_BTN_DIR_SOUTH_NORTH_STRAIGHT:
				{
					sprintf((char *)tAscOperationRecord.m_byFailureValue, "IP is %s, Local Control Enter into direction control south north straight", m_atNetConfig[0].m_chAscNetCardIp);
				}
				break;
			case HWPANEL_BTN_DIR_SOUTH_NORTH_TURN_LEFT:
				{
					sprintf((char *)tAscOperationRecord.m_byFailureValue, "IP is %s, Local Control Enter into direction control south north turn left", m_atNetConfig[0].m_chAscNetCardIp);
				}
				break;
			case HWPANEL_BTN_DIR_EAST:
				{
					sprintf((char *)tAscOperationRecord.m_byFailureValue, "IP is %s, Local Control Enter into direction control east", m_atNetConfig[0].m_chAscNetCardIp);
				}
				break;
			case HWPANEL_BTN_DIR_WEST:
				{
					sprintf((char *)tAscOperationRecord.m_byFailureValue, "IP is %s, Local Control Enter into direction control west", m_atNetConfig[0].m_chAscNetCardIp);
				}
				break;
			case HWPANEL_BTN_DIR_SOUTH:
				{
					sprintf((char *)tAscOperationRecord.m_byFailureValue, "IP is %s, Local Control Enter into direction control south", m_atNetConfig[0].m_chAscNetCardIp);
				}
				break;
			case HWPANEL_BTN_DIR_NORTH:
				{
					sprintf((char *)tAscOperationRecord.m_byFailureValue, "IP is %s, Local Control Enter into direction control north", m_atNetConfig[0].m_chAscNetCardIp);
				}
				break;
			case HWPANEL_BTN_REMOTE_CTRL:
				{
					 sprintf((char *)tAscOperationRecord.m_byFailureValue, "IP is %s, Local Control Enter into remote control", m_atNetConfig[0].m_chAscNetCardIp);
				}
				break;
			case HWPANEL_BTN_Y1:
				{
					sprintf((char *)tAscOperationRecord.m_byFailureValue, "IP is %s, Local Control Enter into Reserve button1", m_atNetConfig[0].m_chAscNetCardIp);
				}
				break;
			case HWPANEL_BTN_Y2:
				{
					sprintf((char *)tAscOperationRecord.m_byFailureValue, "IP is %s, Local Control Enter into Reserve button2", m_atNetConfig[0].m_chAscNetCardIp);
				}
				break;
			case HWPANEL_BTN_Y3:
				{
					sprintf((char *)tAscOperationRecord.m_byFailureValue, "IP is %s, Local Control Enter into Reserve button3", m_atNetConfig[0].m_chAscNetCardIp);
				}
				break;
			case HWPANEL_BTN_Y4:
				{
					sprintf((char *)tAscOperationRecord.m_byFailureValue, "IP is %s, Local Control Enter into Reserve button4", m_atNetConfig[0].m_chAscNetCardIp);
				}
				break;
			default:
				break;
			}
		}
		else
		{
			tAscOperationRecord.m_nInfoType = SYSTEM_MANUALCONTROL;
			switch (nOperationType)
			{
			case CTL_MODE_SELFCTL:
				{
					sprintf((char *)tAscOperationRecord.m_byFailureValue, "IP is %s, System Control Enter into SelfCtl", szPeerIp);
				}
				break;
			case CTL_MODE_FLASH:
				{
					sprintf((char *)tAscOperationRecord.m_byFailureValue, "IP is %s, System Control Enter into YellowFlash", szPeerIp);
				}
				break;
			case CTL_MODE_ALLRED:
				{
			        sprintf((char *)tAscOperationRecord.m_byFailureValue, "IP is %s, System Control Enter into AllRed", szPeerIp);
				}
				break;
			case CTL_MODE_OFF:
				{
					sprintf((char *)tAscOperationRecord.m_byFailureValue, "IP is %s, System Control Enter into Off", szPeerIp);
				}
				break;
			case CTL_MODE_MANUAL:
				{
					sprintf((char *)tAscOperationRecord.m_byFailureValue, "IP is %s, System Control Enter into StepForward", szPeerIp);
				}
				break;
			case CTL_MODE_FIXTIME:
				{
					sprintf((char *)tAscOperationRecord.m_byFailureValue, "IP is %s, System Control Enter into FixTime", szPeerIp);
				}
				break;
			case CTL_MODE_ACTUATE:
				{
					sprintf((char *)tAscOperationRecord.m_byFailureValue, "IP is %s, System Control Enter into Actuate", szPeerIp);
				}
				break;
			case CTL_MODE_SINGLEOPTIM:
				{
					sprintf((char *)tAscOperationRecord.m_byFailureValue, "IP is %s, System Control Enter into SingleOptim", szPeerIp);
				}
				break;
			case CTL_MODE_CABLELESS:
				{
					sprintf((char *)tAscOperationRecord.m_byFailureValue, "IP is %s, System Control Enter into Cableless", szPeerIp);
				}
				break;
			case CTL_MODE_PEDCROSTREET:
				{
					sprintf((char *)tAscOperationRecord.m_byFailureValue, "IP is %s, System Control Enter into PedCrostreet", szPeerIp);
				}
				break;
			case CTL_MODE_ACTUATE_PEDCROSTREET:
				{
					sprintf((char *)tAscOperationRecord.m_byFailureValue, "IP is %s, System Control Enter into ActuatePedCrostreet", szPeerIp);
				}
				break;
			case CTL_MODE_PHASE_LOCK:
				{
					sprintf((char *)tAscOperationRecord.m_byFailureValue, "IP is %s, System Control Enter into PhaseLock", szPeerIp);
				}
				break;
			case CTL_MODE_PHASE_PASS_CONTROL:
				{
					sprintf((char *)tAscOperationRecord.m_byFailureValue, "IP is %s, System Control Enter into PhaseControl", szPeerIp);
				}
				break;
			case CTL_MODE_MANUAL_CONTROL_PATTERN:
				{
					sprintf((char *)tAscOperationRecord.m_byFailureValue, "IP is %s, System Control Enter into ManualControlPattern", szPeerIp);
				}
				break;
			case CTL_MODE_PREEMPT:
				{
					sprintf((char *)tAscOperationRecord.m_byFailureValue, "IP is %s, System Control Enter into PreemptControl", szPeerIp);
				}
				break;
			default:
				break;
			}
		}

	    m_tOpenATCOperationRecord.SaveOneOperationRecordMessage(&tAscOperationRecord, m_pLogicCtlStatus);
    }
    else
    {
        TAscOperationRecord tAscOperationRecord;//������¼
        memset(&tAscOperationRecord, 0, sizeof(tAscOperationRecord));

	    tAscOperationRecord.m_unStartTime = time(NULL);
        tAscOperationRecord.m_unEndTime = time(NULL);
        if (nCtlSource == CTL_SOURCE_LOCAL)
        {
	        tAscOperationRecord.m_bySubject = 2;
        }
        else if (nCtlSource == CTL_SOURCE_TZPARAM)
        {
            tAscOperationRecord.m_bySubject = 1;
        }
		else if (nCtlSource == CTL_SOURCE_SYSTEM)
        {
            tAscOperationRecord.m_bySubject = 0;
        }

	    tAscOperationRecord.m_byObject = 1;
        tAscOperationRecord.m_bStatus = false;

		bool bSave = false;

        if (nCtlSource == CTL_SOURCE_LOCAL)
        {
			tAscOperationRecord.m_nInfoType = LOCAL_MANUALPANEL;
			if (nOperationType == HWPANEL_BTN_STEP)
			{
				if (m_nCurCtlMode == CTL_MODE_FLASH)
				{
					bSave = true;
                    sprintf((char*)tAscOperationRecord.m_byFailureValue, "IP is %s, Mode is in YellowFlash, Local Control Enter into Stepward invalid", m_atNetConfig[0].m_chAscNetCardIp);
				}
				else if (m_nCurCtlMode == CTL_MODE_ALLRED)
				{
					bSave = true;
					sprintf((char*)tAscOperationRecord.m_byFailureValue, "IP is %s, Mode is in YellowFlash, Local Control Enter into Stepward invalid", m_atNetConfig[0].m_chAscNetCardIp);
				}
				else if (m_nCurCtlMode == CTL_MODE_OFF)
				{
					bSave = true;
					sprintf((char*)tAscOperationRecord.m_byFailureValue, "IP is %s, Mode is in YellowFlash, Local Control Enter into Stepward invalid", m_atNetConfig[0].m_chAscNetCardIp);
				}
				else
				{
					bSave = true;
                    sprintf((char*)tAscOperationRecord.m_byFailureValue, "IP is %s, Mode is in Direction, Local Control Enter into Stepward invalid", m_atNetConfig[0].m_chAscNetCardIp);
				}
			}
			else
			{
				bSave = true;
                sprintf((char*)tAscOperationRecord.m_byFailureValue, "IP is %s, Direction cmd is invalid, Local Control Enter into Direction invalid", m_atNetConfig[0].m_chAscNetCardIp);
			}
		
			if (bSave)
			{
                m_tOpenATCOperationRecord.SaveOneOperationRecordMessage(&tAscOperationRecord, m_pLogicCtlStatus); 
			}
        }
        else
        {
			tAscOperationRecord.m_nInfoType = SYSTEM_MANUALCONTROL;
			if (m_nCurCtlMode == CTL_MODE_ALLRED && nOperationType != CTL_MODE_SELFCTL)
			{
                 sprintf((char*)tAscOperationRecord.m_byFailureValue, "IP is %s, Mode is in AllRed, System Control Enter into non Selfctl invalid", szPeerIp);
			}
			else if (m_nCurCtlMode == CTL_MODE_FLASH  && nOperationType != CTL_MODE_SELFCTL)
			{
                 sprintf((char*)tAscOperationRecord.m_byFailureValue, "IP is %s, Mode is in YellowFlash, System Control Enter into non Selfctl invalid", szPeerIp);
			}
			else if (m_nCurCtlMode == CTL_MODE_OFF && nOperationType != CTL_MODE_SELFCTL)
			{
                sprintf((char*)tAscOperationRecord.m_byFailureValue, "IP is %s, Mode is in Off, System Control Enter into non Selfctl invalid", szPeerIp);
			}
			else if (nCtlSource == CTL_SOURCE_LOCAL && m_nCurCtlMode != CTL_MODE_SELFCTL)
            {
                sprintf((char*)tAscOperationRecord.m_byFailureValue, "IP is %s, Usr is in Control, System Control is invalid", m_atNetConfig[0].m_chAscNetCardIp);
            }
			else
			{
				sprintf((char*)tAscOperationRecord.m_byFailureValue, "IP is %s, Mode is in Direction, Local Control Enter into Stepward invalid", m_atNetConfig[0].m_chAscNetCardIp);
			}
			m_tOpenATCOperationRecord.SaveOneOperationRecordMessage(&tAscOperationRecord, m_pLogicCtlStatus);
        } 
    }
}

/*==================================================================== 
������ ��ProcLockChannelLampClr
���� ����������ͨ����ɫ
�㷨ʵ�� �� 
����˵�� ����
����ֵ˵������
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ����Ƽ          �������� 
====================================================================*/
void COpenATCLogicCtlManager::ProcLockChannelLampClr()
{
    int i = 0, j = 0, nMaxChannelID = 0;

	int nYear = 0, nMonth = 0, nDay = 0, nHour = 0, nMin = 0, nSec = 0, nWeek = 0;
    OpenATCGetCurTime(nYear, nMonth, nDay, nHour, nMin, nSec, nWeek);

	int nChannelCount = 0;
	TChannel atChannelInfo[MAX_CHANNEL_COUNT]; 
	memset(atChannelInfo, 0, sizeof(atChannelInfo));
	TChannel atRealChannelInfo[MAX_CHANNEL_COUNT];
	memset(atRealChannelInfo, 0, sizeof(atRealChannelInfo));
	m_pLogicCtlParam->GetChannelTable(atRealChannelInfo);

	for (i = 0;i < MAX_CHANNEL_COUNT;i++)
	{
		atChannelInfo[i].m_byChannelNumber = i + 1;
	}

	for (i = 0;i < MAX_CHANNEL_COUNT;i++)
	{
		for (j = 0;j < MAX_CHANNEL_COUNT;j++)
		{
			if (atRealChannelInfo[j].m_byChannelNumber == 0)
			{
				continue;
			}

			if (atChannelInfo[i].m_byChannelNumber == atRealChannelInfo[j].m_byChannelNumber)
			{
				memcpy(&atChannelInfo[i], &atRealChannelInfo[j], sizeof(TChannel));
			}
		}

		if (atChannelInfo[i].m_byChannelControlSource == 0)
		{
			atChannelInfo[i].m_byChannelNumber = 0;
		}

		if (atChannelInfo[i].m_byChannelNumber > nMaxChannelID)
		{
			nMaxChannelID = atChannelInfo[i].m_byChannelNumber;
		}
	}

	nChannelCount = nMaxChannelID;

	TAscOnePlanChannelLockInfo  tAscOnePlanChannelLockInfo[MAX_SINGLE_DAYPLAN_COUNT];
	memset(tAscOnePlanChannelLockInfo,0,sizeof(tAscOnePlanChannelLockInfo));
	m_pLogicCtlParam->GetChannelLockInfo(tAscOnePlanChannelLockInfo);

	if (GetAllLockChannelCount(tAscOnePlanChannelLockInfo, nChannelCount) == 0)
	{
		//û����������ͨ����������
        return;
	}

	bool bLockChannel = false;

	if (m_nCtlSource == CTL_SOURCE_SELF)
	{
		for (i = 0;i < MAX_SINGLE_DAYPLAN_COUNT;i++)
		{
			if (IsTimeInSpan(nHour, nMin, nSec, tAscOnePlanChannelLockInfo[i].m_byStartHour, tAscOnePlanChannelLockInfo[i].m_byStartMin, tAscOnePlanChannelLockInfo[i].m_byStartSec,
				tAscOnePlanChannelLockInfo[i].m_byEndHour, tAscOnePlanChannelLockInfo[i].m_byEndMin, tAscOnePlanChannelLockInfo[i].m_byEndSec))
			{
				bLockChannel = true;

				LockChannelCtl(tAscOnePlanChannelLockInfo[i],nChannelCount);
				break;
			}
		}
	}
	else
	{
		TManualCmd  tValidManualCmd;
		memset(&tValidManualCmd,0,sizeof(tValidManualCmd));
		m_pLogicCtlStatus->GetValidManualCmd(tValidManualCmd);
		if (tValidManualCmd.m_nSubCtlMode != CTL_MANUAL_SUBMODE_PANEL_DIRECTION && tValidManualCmd.m_nSubCtlMode != CTL_MANUAL_SUBMODE_SYSTEM_CHANNEL_LOCK)
		{
			for (i = 0;i < MAX_SINGLE_DAYPLAN_COUNT;i++)
			{
				if (IsTimeInSpan(nHour, nMin, nSec, tAscOnePlanChannelLockInfo[i].m_byStartHour, tAscOnePlanChannelLockInfo[i].m_byStartMin, tAscOnePlanChannelLockInfo[i].m_byStartSec,
					tAscOnePlanChannelLockInfo[i].m_byEndHour, tAscOnePlanChannelLockInfo[i].m_byEndMin, tAscOnePlanChannelLockInfo[i].m_byEndSec))
				{
					bLockChannel = true;

					LockChannelCtl(tAscOnePlanChannelLockInfo[i],nChannelCount);
					break;
				}
			}
		}
	}

	if (!bLockChannel)
	{
		int nLockChannelCount = 0;
		for (i = 0;i < nChannelCount;i++)
		{
			if (m_tOldAscOnePlanChannelLockInfo.m_stChannelLockStatus[i].m_byChannelLockStatus != LOCKCHANNEL_STATUS_DEFAULT)
			{
                nLockChannelCount += 1;
			}
		}

		if (nLockChannelCount > 0)
		{
			if (m_nCtlSource == CTL_SOURCE_SELF)
			{
				//û���ҵ�ʱ���,�������һ�ε�ʱ��ε�����ͨ����ɫ�ȹ����ٹص�
				LockChannelOff(m_tOldAscOnePlanChannelLockInfo,nChannelCount); 	
			}
			else
			{
				TManualCmd  tValidManualCmd;
				memset(&tValidManualCmd,0,sizeof(tValidManualCmd));
				m_pLogicCtlStatus->GetValidManualCmd(tValidManualCmd);

				if (tValidManualCmd.m_nSubCtlMode == CTL_MANUAL_SUBMODE_PANEL_DIRECTION || tValidManualCmd.m_nSubCtlMode == CTL_MANUAL_SUBMODE_SYSTEM_CHANNEL_LOCK)
				{
					LockChannelTransToDirection(m_tOldAscOnePlanChannelLockInfo,nChannelCount); 	
				}
			}
		}
	}
}

/*==================================================================== 
������ ��LockChannelCtl
���� ������ͨ���ĵ�ɫ����
�㷨ʵ�� �� 
����˵�� ��tAscOnePlanChannelLockInfo������ͨ����ʱ�����Ϣ
           nChannelCount��ͨ������
����ֵ˵������
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ����Ƽ          �������� 
====================================================================*/
void COpenATCLogicCtlManager::LockChannelCtl(TAscOnePlanChannelLockInfo tAscOnePlanChannelLockInfo, int nChannelCount)
{
    TLampClrStatus tLampClrStatus;
	memset(&tLampClrStatus,0,sizeof(TLampClrStatus)); 
	m_pLogicCtlStatus->GetLampClrStatus(tLampClrStatus);

	BYTE byChannelID = 0;

	bool bLockChannelTrans = true;
	
	int i = 0;		
	for (i = 0;i < nChannelCount;i++)
	{
		if (tAscOnePlanChannelLockInfo.m_stChannelLockStatus[i].m_byChannelLockStatus != LOCKCHANNEL_STATUS_DEFAULT)
		{
			byChannelID = tAscOnePlanChannelLockInfo.m_stChannelLockStatus[i].m_byChannelID;

			char * pStartPos = NULL;
			pStartPos = tLampClrStatus.m_achLampClr + (byChannelID - 1) * 3;

			if ((int)tAscOnePlanChannelLockInfo.m_stChannelLockStatus[i].m_byChannelLockStatus == LOCKCHANNEL_STATUS_GREEN)
			{
				if (m_nLockChannelTransStatus[i] == LOCKCHANNEL_STATUS_GREENFLASH || m_nLockChannelTransStatus[i] == LOCKCHANNEL_STATUS_YELLOW)
				{
					if (!LockChannelTrans(tAscOnePlanChannelLockInfo, i, false, tLampClrStatus))//�����ǰͨ�������ɵ�������Ƶ�ʱ����ʱ�Իص��̵�ʱ��
					{
						bLockChannelTrans = false;
					}
				}
				else
				{
					*pStartPos = (char)LAMP_CLR_OFF;
					*(pStartPos + 1) = (char)LAMP_CLR_OFF;
					*(pStartPos + 2) = (char)LAMP_CLR_ON; 

					m_nLockChannelTransStatus[i] = LOCKCHANNEL_STATUS_GREEN;
				}
			}
			else if ((int)tAscOnePlanChannelLockInfo.m_stChannelLockStatus[i].m_byChannelLockStatus == LOCKCHANNEL_STATUS_GREENFLASH)
			{
				if (m_nLockChannelTransStatus[i] == LOCKCHANNEL_STATUS_GREENFLASH || m_nLockChannelTransStatus[i] == LOCKCHANNEL_STATUS_YELLOW)
				{
					if (!LockChannelTrans(tAscOnePlanChannelLockInfo, i, false, tLampClrStatus))//�����ǰͨ�������ɵ�������Ƶ�ʱ����ʱ�Իص�����ʱ��
					{
						bLockChannelTrans = false;
					}
				}
				else
				{
					*pStartPos = (char)LAMP_CLR_OFF;
					*(pStartPos + 1) = (char)LAMP_CLR_OFF;
					*(pStartPos + 2) = (char)LAMP_CLR_FLASH; 

					int nDiff = GetDiffTime(tAscOnePlanChannelLockInfo.m_byStartHour, tAscOnePlanChannelLockInfo.m_byStartMin,
						tAscOnePlanChannelLockInfo.m_byStartSec, tAscOnePlanChannelLockInfo.m_byEndHour,
						tAscOnePlanChannelLockInfo.m_byEndMin, tAscOnePlanChannelLockInfo.m_byEndSec);
					m_pLogicCtlStatus->SetGreenFlashCount((byChannelID - 1) * 3 + 2, nDiff * 2);
				}
			}
			else if ((int)tAscOnePlanChannelLockInfo.m_stChannelLockStatus[i].m_byChannelLockStatus == LOCKCHANNEL_STATUS_YELLOW)
			{
				if (m_nLockChannelTransStatus[i] == LOCKCHANNEL_STATUS_GREENFLASH || m_nLockChannelTransStatus[i] == LOCKCHANNEL_STATUS_YELLOW)
				{
					if (!LockChannelTrans(tAscOnePlanChannelLockInfo, i, false, tLampClrStatus))//�����ǰͨ�������ɵ�������Ƶ�ʱ����ʱ�Իص��̵�ʱ��
					{
						bLockChannelTrans = false;
					}
				}
				else
				{
					*pStartPos = (char)LAMP_CLR_OFF;
					*(pStartPos + 1) = (char)LAMP_CLR_ON;
					*(pStartPos + 2) = (char)LAMP_CLR_OFF; 
				}
			}
			else if ((int)tAscOnePlanChannelLockInfo.m_stChannelLockStatus[i].m_byChannelLockStatus == LOCKCHANNEL_STATUS_RED)
			{
 				if (!LockChannelTrans(tAscOnePlanChannelLockInfo, i, false, tLampClrStatus))//�����ǰͨ�����̵�����Ҫ���ɵ����
				{
                    bLockChannelTrans = false;
				}
			}
			else if ((int)tAscOnePlanChannelLockInfo.m_stChannelLockStatus[i].m_byChannelLockStatus == LOCKCHANNEL_STATUS_REDFLASH)
			{
				if (m_nLockChannelTransStatus[i] == LOCKCHANNEL_STATUS_GREENFLASH || m_nLockChannelTransStatus[i] == LOCKCHANNEL_STATUS_YELLOW)
				{
					if (!LockChannelTrans(tAscOnePlanChannelLockInfo, i, false, tLampClrStatus))//�����ǰͨ�������ɵ�������Ƶ�ʱ����ʱ�Իص��̵�ʱ��
					{
						bLockChannelTrans = false;
					}
				}
				else
				{
					*pStartPos = (char)LAMP_CLR_FLASH;
					*(pStartPos + 1) = (char)LAMP_CLR_OFF;
					*(pStartPos + 2) = (char)LAMP_CLR_OFF; 

					int nDiff = GetDiffTime(tAscOnePlanChannelLockInfo.m_byStartHour, tAscOnePlanChannelLockInfo.m_byStartMin,
						tAscOnePlanChannelLockInfo.m_byStartSec, tAscOnePlanChannelLockInfo.m_byEndHour,
						tAscOnePlanChannelLockInfo.m_byEndMin, tAscOnePlanChannelLockInfo.m_byEndSec);
					m_pLogicCtlStatus->SetRedFlashCount((byChannelID - 1) * 3, nDiff * 2);
				}
			}
			else if ((int)tAscOnePlanChannelLockInfo.m_stChannelLockStatus[i].m_byChannelLockStatus == LOCKCHANNEL_STATUS_OFF)
			{
				if (!LockChannelTrans(tAscOnePlanChannelLockInfo, i, true, tLampClrStatus))//�����ǰͨ�����̵�����Ҫ���ɵ��ص�
				{
				    bLockChannelTrans = false;
				}		
			}
		} 
	}

	m_pLogicCtlStatus->SetLampClrStatus(tLampClrStatus);
	
	if (bLockChannelTrans)
	{
        if (memcmp(&m_tOldAscOnePlanChannelLockInfo,&tAscOnePlanChannelLockInfo,sizeof(m_tOldAscOnePlanChannelLockInfo)) != 0)
		{
			memcpy(&m_tOldAscOnePlanChannelLockInfo,&tAscOnePlanChannelLockInfo,sizeof(m_tOldAscOnePlanChannelLockInfo));
		}
	}
}

/*==================================================================== 
������ ��LockChannelOff
���� ������ͨ���ĵ�ɫ�ر�
�㷨ʵ�� �� 
����˵�� ��tAscOnePlanChannelLockInfo������ͨ����ʱ�����Ϣ
           nChannelCount��ͨ������
����ֵ˵������
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ����Ƽ          �������� 
====================================================================*/
void COpenATCLogicCtlManager::LockChannelOff(TAscOnePlanChannelLockInfo tAscOnePlanChannelLockInfo, int nChannelCount)
{
	TLampClrStatus tLampClrStatus;
	memset(&tLampClrStatus,0,sizeof(TLampClrStatus)); 
    m_pLogicCtlStatus->GetLampClrStatus(tLampClrStatus);

	bool bLockChannelTrans = false;

	static int nCount = 0;
	int  nLockChannelCount = 0;
	int  i = 0;
	for (i = 0;i < nChannelCount;i++)
	{
		if (tAscOnePlanChannelLockInfo.m_stChannelLockStatus[i].m_byChannelLockStatus != LOCKCHANNEL_STATUS_DEFAULT)
		{
            nLockChannelCount += 1;
		}
	}
	
	for (i = 0;i < nChannelCount;i++)
	{
		if (LockChannelTrans(tAscOnePlanChannelLockInfo, i, true, tLampClrStatus))
		{
            nCount += 1;	
		}

		if (nCount == nLockChannelCount)
		{
			bLockChannelTrans = true;
		}
	}

	m_pLogicCtlStatus->SetLampClrStatus(tLampClrStatus);

	if (bLockChannelTrans)
	{
		nCount = 0;
		memset(&m_tOldAscOnePlanChannelLockInfo,0,sizeof(m_tOldAscOnePlanChannelLockInfo));
	}
}

/*==================================================================== 
������ ��LockChannelTrans
���� ������ͨ���ĵ�ɫ����
�㷨ʵ�� �� 
����˵�� ��tAscOnePlanChannelLockInfo������ͨ����ʱ�����Ϣ
           nChannelIndex��ͨ�����
		   tLampClrStatus����ɫ
����ֵ˵������
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ����Ƽ          �������� 
====================================================================*/
bool COpenATCLogicCtlManager::LockChannelTrans(TAscOnePlanChannelLockInfo tAscOnePlanChannelLockInfo, int nChannelIndex, bool bLockChannelOff, TLampClrStatus & tLampClrStatus)
{
	unsigned long  nGlobalCounter = m_pLogicCtlStatus->GetGlobalCounter();

	BYTE byChannelID = 0;

	bool bLockChannelTrans = false;

	if (tAscOnePlanChannelLockInfo.m_stChannelLockStatus[nChannelIndex].m_byChannelLockStatus != LOCKCHANNEL_STATUS_DEFAULT)
	{
		unsigned long nCounterDiff = CalcCounter(m_nLockChannelCounter[nChannelIndex], nGlobalCounter, C_N_MAXGLOBALCOUNTER);

		byChannelID = tAscOnePlanChannelLockInfo.m_stChannelLockStatus[nChannelIndex].m_byChannelID;

		char * pStartPos = NULL;
		pStartPos = tLampClrStatus.m_achLampClr + (byChannelID - 1) * 3;

		if (m_nLockChannelTransStatus[nChannelIndex] == LOCKCHANNEL_STATUS_GREEN)//�̵ƹ���
		{
			if (m_tOldAscOnePlanChannelLockInfo.m_byGreenFlash > 0)
			{
				m_nLockChannelCounter[nChannelIndex] = nGlobalCounter;

				*pStartPos = (char)LAMP_CLR_OFF;
				*(pStartPos + 1) = (char)LAMP_CLR_OFF;
				*(pStartPos + 2) = (char)LAMP_CLR_FLASH; 

				m_pLogicCtlStatus->SetGreenFlashCount((byChannelID - 1) * 3 + 2, m_tOldAscOnePlanChannelLockInfo.m_byGreenFlash * 2);
				
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "LockChannelTrans Channel:%d Green To GreenFlash", nChannelIndex);

				m_nLockChannelTransStatus[nChannelIndex] = LOCKCHANNEL_STATUS_GREENFLASH;
			}
			else if (m_tOldAscOnePlanChannelLockInfo.m_byYellow > 0)
			{
                m_nLockChannelCounter[nChannelIndex] = nGlobalCounter;

				*pStartPos = (char)LAMP_CLR_OFF;
				*(pStartPos + 1) = (char)LAMP_CLR_ON;
				*(pStartPos + 2) = (char)LAMP_CLR_OFF; 
				
				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "LockChannelTrans Channel:%d Green To Yellow", nChannelIndex);

				m_nLockChannelTransStatus[nChannelIndex] = LOCKCHANNEL_STATUS_YELLOW; 
			}
			else
			{
				if (!bLockChannelOff)
				{
					*pStartPos = (char)LAMP_CLR_ON;
					*(pStartPos + 1) = (char)LAMP_CLR_OFF;
					*(pStartPos + 2) = (char)LAMP_CLR_OFF;

					m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "LockChannelTrans Channel:%d Green To Red", nChannelIndex);

					m_nLockChannelTransStatus[nChannelIndex] = LOCKCHANNEL_STATUS_RED;//�̵�ֱ�ӹ��ɵ����
				}
				else
				{
					*pStartPos = (char)LAMP_CLR_OFF;
					*(pStartPos + 1) = (char)LAMP_CLR_OFF;
					*(pStartPos + 2) = (char)LAMP_CLR_OFF;

					m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "LockChannelTrans Channel:%d Green To Off", nChannelIndex);

					m_nLockChannelTransStatus[nChannelIndex] = LOCKCHANNEL_STATUS_OFF;//�̵�ֱ�ӹ��ɵ��ص�
				}

				bLockChannelTrans = true;//���ɳɹ�
			}
		}
		else if (m_nLockChannelTransStatus[nChannelIndex] == LOCKCHANNEL_STATUS_GREENFLASH)//��������
		{
			if (nCounterDiff >= (m_tOldAscOnePlanChannelLockInfo.m_byGreenFlash * C_N_TIMER_TIMER_COUNTER))
			{
				if (m_tOldAscOnePlanChannelLockInfo.m_byYellow > 0)
				{
					m_nLockChannelCounter[nChannelIndex] = nGlobalCounter;

					*pStartPos = (char)LAMP_CLR_OFF;
					*(pStartPos + 1) = (char)LAMP_CLR_ON;
					*(pStartPos + 2) = (char)LAMP_CLR_OFF;
				
					m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "LockChannelTrans Channel:%d GreenFlash To Yellow", nChannelIndex);

					m_nLockChannelTransStatus[nChannelIndex] = LOCKCHANNEL_STATUS_YELLOW;
				}
				else
				{
                    if (!bLockChannelOff)
					{
						*pStartPos = (char)LAMP_CLR_ON;
						*(pStartPos + 1) = (char)LAMP_CLR_OFF;
						*(pStartPos + 2) = (char)LAMP_CLR_OFF;

						m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "LockChannelTrans Channel:%d GreenFlash To Red", nChannelIndex);

						m_nLockChannelTransStatus[nChannelIndex] = LOCKCHANNEL_STATUS_RED;//����ֱ�ӹ��ɵ����
					}
					else
					{
						*pStartPos = (char)LAMP_CLR_OFF;
						*(pStartPos + 1) = (char)LAMP_CLR_OFF;
						*(pStartPos + 2) = (char)LAMP_CLR_OFF;

						m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "LockChannelTrans Channel:%d GreenFlash To Off", nChannelIndex);

						m_nLockChannelTransStatus[nChannelIndex] = LOCKCHANNEL_STATUS_OFF;//����ֱ�ӹ��ɵ��ص�
					}

					bLockChannelTrans = true;//���ɳɹ�
				}
			}
			else
			{
				*pStartPos = (char)LAMP_CLR_OFF;
				*(pStartPos + 1) = (char)LAMP_CLR_OFF;
				*(pStartPos + 2) = (char)LAMP_CLR_FLASH; 
			}
			
		}
		else if (m_nLockChannelTransStatus[nChannelIndex] == LOCKCHANNEL_STATUS_YELLOW)//�Ƶƹ���
		{
			if (nCounterDiff >= (m_tOldAscOnePlanChannelLockInfo.m_byYellow * C_N_TIMER_TIMER_COUNTER))
			{
                m_nLockChannelCounter[nChannelIndex] = nGlobalCounter;

				*pStartPos = (char)LAMP_CLR_ON;
				*(pStartPos + 1) = (char)LAMP_CLR_OFF;
				*(pStartPos + 2) = (char)LAMP_CLR_OFF;
				
                m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "LockChannelTrans Channel:%d Yellow To Red", nChannelIndex);

				m_nLockChannelTransStatus[nChannelIndex] = LOCKCHANNEL_STATUS_RED;

				bLockChannelTrans = true;//���ɳɹ�
			}
			else
			{
				*pStartPos = (char)LAMP_CLR_OFF;
				*(pStartPos + 1) = (char)LAMP_CLR_ON;
				*(pStartPos + 2) = (char)LAMP_CLR_OFF;
			}
		}
		else
		{
			if (!bLockChannelOff)
			{
				*pStartPos = (char)LAMP_CLR_ON;
				*(pStartPos + 1) = (char)LAMP_CLR_OFF;
				*(pStartPos + 2) = (char)LAMP_CLR_OFF;

				m_nLockChannelTransStatus[nChannelIndex] = LOCKCHANNEL_STATUS_RED;//���ɵ����
			}
			else
			{
			    *pStartPos = (char)LAMP_CLR_OFF;
				*(pStartPos + 1) = (char)LAMP_CLR_OFF;
				*(pStartPos + 2) = (char)LAMP_CLR_OFF;

				m_nLockChannelTransStatus[nChannelIndex] = LOCKCHANNEL_STATUS_OFF;//���ɵ��ص�

				//m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "LockChannelTrans Channel:%d To Off", nChannelIndex);
			}

			bLockChannelTrans = true;//���ɳɹ�
		}
	}	

	return bLockChannelTrans;
}

/*==================================================================== 
������ ��GetAllLockChannelCount
���� ����ȡ����ʱ��ε�����ͨ������
�㷨ʵ�� �� 
����˵�� ��tAscOnePlanChannelLockInfo������ͨ����ʱ�����Ϣ
           nChannelCount��ͨ������
����ֵ˵��������ʱ��ε�����ͨ������
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ����Ƽ          �������� 
====================================================================*/
int COpenATCLogicCtlManager::GetAllLockChannelCount(TAscOnePlanChannelLockInfo tAscOnePlanChannelLockInfo[], int nChannelCount)
{
	int i = 0, j = 0;
	int nAllLockChannelCount = 0;

	for (i = 0;i < MAX_SINGLE_DAYPLAN_COUNT;i++)
	{
		for (j = 0;j < nChannelCount;j++)
		{
			if (tAscOnePlanChannelLockInfo[i].m_stChannelLockStatus[j].m_byChannelLockStatus != LOCKCHANNEL_STATUS_DEFAULT)
			{
                nAllLockChannelCount += 1;
			}
		}
	}

	return nAllLockChannelCount;
}

/*==================================================================== 
������ ��IsTimeInSpan
���� ���жϵ�ǰʱ���Ƿ���ʱ�����
�㷨ʵ�� �� 
����˵�� ��
           nCurHour����ǰʱ
		   nCurMin����ǰ��
		   nCurSec����ǰ��
           nStartHour����ʼʱ
           nStartMin����ʼ��
		   nStartSec����ʼ��
           nEndHour������ʱ
		   nEndMin��������
		   nEndSec��������
����ֵ˵������ǰʱ���Ƿ���ʱ�����
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ����Ƽ          �������� 
====================================================================*/
bool COpenATCLogicCtlManager::IsTimeInSpan(int nCurHour, int nCurMin, int nCurSec, int nStartHour, int nStartMin, int nStartSec, int nEndHour, int nEndMin, int nEndSec)
{
    int iBegLessThanEnd = 0; //1-���õĿ�ʼʱ��С�ڽ���ʱ�� 0-���õĿ�ʼʱ����ڽ���ʱ��

	if (nStartHour < nEndHour || (nStartHour == nEndHour && nStartMin <= nEndMin))
	{
		iBegLessThanEnd = 1;
	}
	else
	{
		iBegLessThanEnd = 0;
	}

	if (iBegLessThanEnd)   //��ʼʱ��С�ڽ���ʱ��
	{
		if ((nCurHour > nStartHour || (nCurHour == nStartHour && nCurMin >= nStartMin))
			&& (nCurHour < nEndHour || (nCurHour == nEndHour && nCurMin <= nEndMin)))
		{
			if (nCurHour == nStartHour && nCurMin == nStartMin)
			{
				if (nCurSec < nStartSec)
				{
					return false;
				}
			}

			if (nCurHour == nEndHour && nCurMin == nEndMin)
			{
				if (nCurSec > nEndSec)
				{
					return false;
				}
			}

			return true;
		}
		else
		{
			return false;
		}
	}
	else   //��ʼʱ����ڽ���ʱ��, ��������
	{
		if ((nCurHour > nStartHour || (nCurHour == nStartHour && nCurMin >= nStartMin))
			|| (nCurHour < nEndHour || (nCurHour == nEndHour && nCurMin <= nEndMin)))
		{
			if (nCurHour == nStartHour && nCurMin == nStartMin)
			{
				if (nCurSec < nStartSec)
				{
					return false;
				}
			}

			if (nCurHour == nEndHour && nCurMin == nEndMin)
			{
				if (nCurSec > nEndSec)
				{
					return false;
				}
			}

			return true;
		}
		else
		{
			return false;
		}
    }
}

/*==================================================================== 
������ ��GetDiffTime
���� ����ȡʱ���(����)
�㷨ʵ�� �� 
����˵�� ��nStartHour����ʼʱ
           nStartMin����ʼ��
		   nStartSec����ʼ��
           nEndHour������ʱ
		   nEndMin��������
		   nEndSec��������
����ֵ˵������
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ����Ƽ          �������� 
====================================================================*/
int  COpenATCLogicCtlManager::GetDiffTime(int nStartHour, int nStartMin, int nStartSec, int nEndHour, int nEndMin, int nEndSec)
{
	int nHour = 0, nMin = 0, nSec = 0;

	if (nEndSec < nStartSec)
    {
        nSec = 60 - nStartSec + nEndSec;
    }
    else
    {
        nSec = nEndSec - nStartSec;
    }

    if (nEndMin < nStartMin)
    {
        nMin = nEndMin + 60 - nStartMin;
        nEndHour--;
    }
    else
    {
        nMin = nEndMin - nStartMin;
    }

    if (nEndHour < nStartHour)
    {
        nHour = nEndHour + 24 - nStartHour;
    }
    else
    {
        nHour = nEndHour - nStartHour;
    }

	return nHour * 3600 + nMin * 60 + nSec;
}

/*==================================================================== 
������ ��GetManualBtnByIndex
���� ������������ȡ��尴ť���
�㷨ʵ�� �� 
����˵�� ������
����ֵ˵������ť���
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2020/05/14     V1.0                 �������� 
====================================================================*/ 
int COpenATCLogicCtlManager::GetManualBtnByIndex(int nIndex)
{
    switch (nIndex)
	{
	case HWPANEL_DIR_EAST_WEST_STRAIGHT_INDEX:
		return HWPANEL_BTN_DIR_EAST_WEST_STRAIGHT;
		break;
	case HWPANEL_DIR_NORTH_INDEX:
		return HWPANEL_BTN_DIR_NORTH;
		break;
	case HWPANEL_DIR_EAST_WEST_TURN_LEFT_INDEX:
		return HWPANEL_BTN_DIR_EAST_WEST_TURN_LEFT;
		break;
	case HWPANEL_DIR_WEST_INDEX:
		return HWPANEL_BTN_DIR_WEST;
		break;
	case HWPANEL_DIR_EAST_INDEX:
		return HWPANEL_BTN_DIR_EAST;
		break;
    case HWPANEL_DIR_SOUTH_NORTH_STRAIGHT_INDEX:
		return HWPANEL_BTN_DIR_SOUTH_NORTH_STRAIGHT;
		break;
    case HWPANEL_DIR_SOUTH_INDEX:
		return HWPANEL_BTN_DIR_SOUTH;
		break;
    case HWPANEL_DIR_SOUTH_NORTH_TURN_LEFT_INDEX:
		return HWPANEL_BTN_DIR_SOUTH_NORTH_TURN_LEFT;
		break;
    case HWPANEL_Y1_INDEX:
		return HWPANEL_BTN_Y1;
		break;
    case HWPANEL_Y2_INDEX:
		return HWPANEL_BTN_Y2;
		break;
    case HWPANEL_Y3_INDEX:
		return HWPANEL_BTN_Y3;
		break;
    case HWPANEL_Y4_INDEX:
		return HWPANEL_BTN_Y4;
		break;
    case HWPANEL_REMOTE_CTRL_INDEX:
		return HWPANEL_BTN_REMOTE_CTRL;
		break;
	default:
		return 0;
		break;
    }
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
2020/05/14     V1.0                 �������� 
====================================================================*/ 
unsigned long COpenATCLogicCtlManager::CalcCounter(unsigned long nStart,unsigned long nEnd,unsigned long nMax)
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
2020/05/14     V1.0                 �������� 
====================================================================*/ 
void COpenATCLogicCtlManager::OpenATCGetCurTime(int & nYear, int & nMon, int & nDay, int & nHour, int & nMin, int & nSec, int & nWeek)
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
������ ��SetAllRedStage
���� �����ý���ȫ��׶�
�㷨ʵ�� �� 
����˵�� ����
����ֵ˵������
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2020/05/14     V1.0                 �������� 
====================================================================*/ 
void COpenATCLogicCtlManager::SetAllRedStage()
{
	if (m_nLogicCtlStage != CTL_STAGE_STARTUP_RED)
	{
		m_nLogicCtlStage = CTL_STAGE_STARTUP_RED;   
		TPhaseLampClrRunCounter tRunCounter;
		m_pLogicCtlStatus->GetPhaseLampClrRunCounter(tRunCounter);
		tRunCounter.m_nLampClrTime[0] = 0;
		tRunCounter.m_nLampClrStartCounter[0] = tRunCounter.m_nCurCounter;
		m_pLogicCtlStatus->SetPhaseLampClrRunCounter(tRunCounter);

		TLogicCtlStatus tCtlStatus;
		m_pLogicCtlStatus->GetLogicCtlStatus(tCtlStatus);
		tCtlStatus.m_nRunStage = CTL_STAGE_STARTUP_RED;
		m_pLogicCtlStatus->SetLogicCtlStatus(tCtlStatus);

		if (m_pLogicCtlMode != NULL)
        {
            m_pLogicCtlMode->Release();
            delete m_pLogicCtlMode;
            m_pLogicCtlMode = NULL;
        }
        m_pLogicCtlMode = CLogicCtlModeSimpleFactory::Create(CTL_MODE_ALLRED);
        m_pLogicCtlMode->Init(m_pLogicCtlParam,m_pLogicCtlStatus,m_pOpenATCLog,0);
        m_pLogicCtlMode->Run();

		m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "FaultDetBoard Lose of Control, COpenATCLogicCtlManager SetAllRedStage!");
	}
}

/*==================================================================== 
������ ��SetSystemControlStatus
���� ������ϵͳ��������״̬
�㷨ʵ�� �� 
����˵�� ������Ľ����ʧ��ԭ��
����ֵ˵������
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2020/05/14     V1.0                 �������� 
====================================================================*/ 
void COpenATCLogicCtlManager::SetSystemControlStatus(bool bSpecicalControlResult, int nSpecicalControlFailCode,
	bool bPatternControlResult, int nPatternControlFailCode, 
	bool bStageControlResult, int nStageControlFailCode, 
	bool bPhaseControlResult, int nPhaseControlFailCode, 
	bool bChannelLockResult, int nChannelLockFailCode)
{
	TSystemControlStatus tSystemControlStatus;
	memset(&tSystemControlStatus, 0, sizeof(tSystemControlStatus));
	m_pLogicCtlStatus->GetSystemControlStatus(tSystemControlStatus);
	tSystemControlStatus.m_nSpecicalControlResult = bSpecicalControlResult;
	tSystemControlStatus.m_nSpecicalControlFailCode = nSpecicalControlFailCode;
	tSystemControlStatus.m_nPatternControlResult = bPatternControlResult;
	tSystemControlStatus.m_nPatternControlFailCode = nPatternControlFailCode;
	tSystemControlStatus.m_nStageControlResult = bStageControlResult;
	tSystemControlStatus.m_nStageControlFailCode = nStageControlFailCode;
	tSystemControlStatus.m_nPhaseControlResult = bPhaseControlResult;
	tSystemControlStatus.m_nPhaseControlFailCode = nPhaseControlFailCode;
	tSystemControlStatus.m_nChannelLockResult = bChannelLockResult;
	tSystemControlStatus.m_nChannelLockFailCode = nChannelLockFailCode;
	m_pLogicCtlStatus->SetSystemControlStatus(tSystemControlStatus);
}

/*==================================================================== 
������ ��SetPreemptControlStatus
���� ���������ȿ�������״̬
�㷨ʵ�� �� 
����˵�� ������Ľ����ʧ��ԭ��
����ֵ˵������
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2020/05/14     V1.0                 �������� 
====================================================================*/ 
void COpenATCLogicCtlManager::SetPreemptControlStatus(bool bPreemptControlResult, int nPreemptControlFailCode)
{
	TPreemptControlStatus tPreemptControlStatus;
	memset(&tPreemptControlStatus, 0, sizeof(tPreemptControlStatus));
	m_pLogicCtlStatus->GetPreemptControlStatus(tPreemptControlStatus);
	tPreemptControlStatus.m_nPreemptControlResult = bPreemptControlResult;
	tPreemptControlStatus.m_nPreemptControlResultFailCode = nPreemptControlFailCode;
	m_pLogicCtlStatus->SetPreemptControlStatus(tPreemptControlStatus);
}
/*==================================================================== 
������ ��LockChannelTransToDirection
���� ������ͨ���ĵ�ɫ���������е�����
�㷨ʵ�� �� 
����˵�� ��tAscOnePlanChannelLockInfo������ͨ����ʱ�����Ϣ
           nChannelCount��ͨ������
����ֵ˵������
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ����Ƽ          �������� 
====================================================================*/
void COpenATCLogicCtlManager::LockChannelTransToDirection(TAscOnePlanChannelLockInfo tAscOnePlanChannelLockInfo, int nChannelCount)
{
	TLampClrStatus tLampClrStatus;
	memset(&tLampClrStatus,0,sizeof(TLampClrStatus)); 
    m_pLogicCtlStatus->GetLampClrStatus(tLampClrStatus);

	bool bLockChannelTrans = false;//���ɽ�����־

	static int nCount = 0;
	int  nLockChannelCount = 0;
	int  i = 0, j = 0;
	for (j = 0;j < nChannelCount;j++)
	{
		if (tAscOnePlanChannelLockInfo.m_stChannelLockStatus[j].m_byChannelLockStatus != LOCKCHANNEL_STATUS_DEFAULT)
		{
            nLockChannelCount += 1;
		}
	}

	TAscManualPanel tAscManualPanel;
	memset(&tAscManualPanel, 0, sizeof(TAscManualPanel)); 
	m_pLogicCtlParam->GetManualPanelInfo(tAscManualPanel);

	TManualCmd  tValidManualCmd;
	memset(&tValidManualCmd,0,sizeof(tValidManualCmd));
	m_pLogicCtlStatus->GetValidManualCmd(tValidManualCmd);

	for (i = 0;i < MAX_PANEL_KEY_CFG_COUNT;i++)
	{
		if (tAscManualPanel.m_stPanelKeyCfg[i].m_byKeyNum == tValidManualCmd.m_tDirectionCmd.m_nNextDirectionIndex)
		{    
			for (j = 0;j < nChannelCount;j++)
			{
				if (tAscManualPanel.m_stPanelKeyCfg[i].m_ChannelCfg[j].m_byChannelStatus == m_nLockChannelTransStatus[j])
				{
					 nCount += 1;//����ͨ���ĵ�ɫ�ͷ�����Ƶ�ͨ����ɫһ�£��򲻹���
				}
				else
				{
					if (LockChannelTrans(tAscOnePlanChannelLockInfo, j, false, tLampClrStatus))
					{
						nCount += 1;
					}
				}

				if (nCount == nLockChannelCount)
				{
					bLockChannelTrans = true;
				}
			}

            break;
		}
	}
	
	m_pLogicCtlStatus->SetLampClrStatus(tLampClrStatus);

	if (bLockChannelTrans)
	{
		nCount = 0;
		memset(&m_tOldAscOnePlanChannelLockInfo,0,sizeof(m_tOldAscOnePlanChannelLockInfo));

		TChannel atChannelInfo[MAX_CHANNEL_COUNT];
		memset(atChannelInfo,0,sizeof(atChannelInfo));
		TChannel atRealChannelInfo[MAX_CHANNEL_COUNT];
		memset(atRealChannelInfo, 0, sizeof(atRealChannelInfo));
		m_pLogicCtlParam->GetChannelTable(atRealChannelInfo);

		for (i = 0;i < MAX_CHANNEL_COUNT;i++)
		{
			atChannelInfo[i].m_byChannelNumber = i + 1;
		}

		for (i = 0;i < MAX_CHANNEL_COUNT;i++)
		{
			for (j = 0;j < MAX_CHANNEL_COUNT;j++)
			{
				if (atRealChannelInfo[j].m_byChannelNumber == 0)
				{
					continue;
				}

				if (atChannelInfo[i].m_byChannelNumber == atRealChannelInfo[j].m_byChannelNumber)
				{
					memcpy(&atChannelInfo[i], &atRealChannelInfo[j], sizeof(TChannel));
				}
			}

			if (atChannelInfo[i].m_byChannelControlSource == 0)
			{
				atChannelInfo[i].m_byChannelNumber = 0;
			}
		}
	
		//��ȡ��ǰͨ����Ҫ�л���Ŀ���ɫ
		for (i = 0;i < MAX_PANEL_KEY_CFG_COUNT;i++)
		{
			if (tAscManualPanel.m_stPanelKeyCfg[i].m_byKeyNum == tValidManualCmd.m_tDirectionCmd.m_nNextDirectionIndex)
			{    
				for (j = 0;j < nChannelCount;j++)
				{
					if (tAscManualPanel.m_stPanelKeyCfg[i].m_ChannelCfg[j].m_byChannelID == atChannelInfo[j].m_byChannelNumber &&
						tAscManualPanel.m_stPanelKeyCfg[i].m_ChannelCfg[j].m_byChannelStatus != CHANNEL_STATUS_DEFAULT)
					{
						char * pStartPos = NULL;
						pStartPos = tLampClrStatus.m_achLampClr + (tAscManualPanel.m_stPanelKeyCfg[i].m_ChannelCfg[j].m_byChannelID - 1) * 3;

						if (tAscManualPanel.m_stPanelKeyCfg[i].m_ChannelCfg[j].m_byChannelStatus == CHANNEL_STATUS_GREEN)
						{
							*pStartPos = (char)LAMP_CLR_OFF;
							*(pStartPos + 1) = (char)LAMP_CLR_OFF;
							*(pStartPos + 2) = (char)LAMP_CLR_ON; 
						}
						else if (tAscManualPanel.m_stPanelKeyCfg[i].m_ChannelCfg[j].m_byChannelStatus == CHANNEL_STATUS_RED)
						{
							*pStartPos = (char)LAMP_CLR_ON;
							*(pStartPos + 1) = (char)LAMP_CLR_OFF;
							*(pStartPos + 2) = (char)LAMP_CLR_OFF; 
						}
						else if (tAscManualPanel.m_stPanelKeyCfg[i].m_ChannelCfg[j].m_byChannelStatus == CHANNEL_STATUS_OFF)
						{
							*pStartPos = (char)LAMP_CLR_OFF;
							*(pStartPos + 1) = (char)LAMP_CLR_OFF;
							*(pStartPos + 2) = (char)LAMP_CLR_OFF; 
						}
						m_pLogicCtlStatus->SetLampClrStatus(tLampClrStatus);
					}
				}
  
				break;
			} 
		}
	}
}

/*==================================================================== 
������ ��SwitchManualControlPatternToSelf
���� ���ֶ����Ʒ�����ʱ�ص�����
�㷨ʵ�� �� 
����˵�� ����
����ֵ˵������
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ����Ƽ          �������� 
====================================================================*/
void COpenATCLogicCtlManager::SwitchManualControlPatternToSelf()
{
	//��ʱ�ֶ��������Ƴ���ʱ�䣬���ͻص���������
	if (m_nCurCtlMode == CTL_MODE_MANUAL_CONTROL_PATTERN && labs(time(NULL) - m_nManualControlPatternStartTime) > m_nManualControlPatternDurationTime &&
		m_nManualControlPatternDurationTime > 0)
	{
		m_nManualControlPatternStartTime = 0;
		m_nManualControlPatternDurationTime = 0;

		TManualCmd tManualCmd;
		memset(&tManualCmd, 0, sizeof(tManualCmd));

		tManualCmd.m_bNewCmd = true;
		tManualCmd.m_nCmdSource = CTL_SOURCE_SYSTEM;
		memcpy(tManualCmd.m_szPeerIp, "127.0.0.1", strlen( "127.0.0.1"));
		tManualCmd.m_bStepForwardCmd = false;
		memset(&tManualCmd.m_tStepForwardCmd,0,sizeof(TStepForwardCmd));
		tManualCmd.m_bDirectionCmd = false;
		memset(&tManualCmd.m_tDirectionCmd,0,sizeof(TDirectionCmd));
		tManualCmd.m_bPatternInterruptCmd = true;
		tManualCmd.m_tPatternInterruptCmd.m_nControlMode = CTL_MODE_SELFCTL;
		tManualCmd.m_tPatternInterruptCmd.m_nPatternNo = 0;
		tManualCmd.m_bChannelLockCmd = false;
		memset(&tManualCmd.m_tChannelLockCmd,0,sizeof(TChannelLockCmd));
		memset(&tManualCmd.m_tPhaseLockPara,0,sizeof(TPhaseLockPara));
		tManualCmd.m_bPhaseToChannelLock = false;
		m_pLogicCtlStatus->SetManualCmd(tManualCmd);

		m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "CreateManualCmd, ManualControlPattern Timeout Return To Self");
	}
}

/*==================================================================== 
������ ��SetPreemptStageIndex
���� ���������ȿ��ƵĽ׶α��
�㷨ʵ�� �� 
����˵�� �� byPreemptPhaseID,���ȿ�����λ
����ֵ˵����byStageIndexTarget�����ȿ�����λ��Ӧ�Ľ׶κ�
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ����Ƽ          �������� 
====================================================================*/
void COpenATCLogicCtlManager::SetPreemptStageIndex(BYTE byPreemptPhaseID, BYTE & byStageIndexTarget)
{
	bool bFlag = false;
	int nRunStageIndex = 0;
	int nPhaseIndex = 0;
	for (nRunStageIndex = 0;nRunStageIndex < m_tRunStageTable.m_nRunStageCount;nRunStageIndex++)
	{
		for (nPhaseIndex = 0;nPhaseIndex < m_tRunStageTable.m_PhaseRunstageInfo[nRunStageIndex].m_nConcurrencyPhaseCount;nPhaseIndex++)
		{
			if (m_tRunStageTable.m_PhaseRunstageInfo[nRunStageIndex].m_nConcurrencyPhase[nPhaseIndex] == byPreemptPhaseID)
			{
				bFlag = true;
				break;
			}
		}

		if (bFlag)
		{
			break;
		}
	}

	byStageIndexTarget = nRunStageIndex;

	if (!m_tRunStageTable.m_PhaseRunstageInfo[nRunStageIndex].m_bDirectTransit[nPhaseIndex])
	{
		TPhaseRunStatus tAscPhaseRunStatus;
		memset(&tAscPhaseRunStatus, 0x00, sizeof(tAscPhaseRunStatus));
		m_pLogicCtlStatus->GetPhaseRunStatus(tAscPhaseRunStatus);

		int  i = 0;
		for (i = 0;i < tAscPhaseRunStatus.m_atRingRunStatus[nPhaseIndex].m_nPhaseCount; i++)
		{
			if (tAscPhaseRunStatus.m_atRingRunStatus[nPhaseIndex].m_nCurRunPhaseIndex == i &&
				tAscPhaseRunStatus.m_atRingRunStatus[nPhaseIndex].m_atPhaseStatus[i].m_byPhaseID == byPreemptPhaseID)
			{
				if (tAscPhaseRunStatus.m_atRingRunStatus[nPhaseIndex].m_atPhaseStatus[i].m_chPhaseStatus == C_CH_PHASESTAGE_G)
				{
					byStageIndexTarget = tAscPhaseRunStatus.m_atRingRunStatus[nPhaseIndex].m_nCurStageIndex;
				}
			}
		}
	}

	m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "SetPreemptStageIndex:%d", byStageIndexTarget);
}

/*==================================================================== 
������ ��CreatePatternInterruptCmdInPreemptControl
���� �����Ż�����ʱ���ɷ�����Ԥָ��
�㷨ʵ�� �� 
����˵�� �� tManualCmd������ָ��
����ֵ˵������
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ����Ƽ          �������� 
====================================================================*/
void COpenATCLogicCtlManager::CreatePatternInterruptCmdInPreemptControl(TManualCmd  tManualCmd)
{
	TPreemptCtlCmd  tPreemptCtlCmd;
	memset(&tPreemptCtlCmd,0,sizeof(tPreemptCtlCmd));
	tPreemptCtlCmd.m_nCmdSource = CTL_SOURCE_SYSTEM;
	tPreemptCtlCmd.m_nCurCtlSource = CTL_SOURCE_PREEMPT;
	memcpy(tPreemptCtlCmd.m_szPeerIp,tManualCmd.m_szPeerIp,strlen(tManualCmd.m_szPeerIp));
	tPreemptCtlCmd.m_bNewCmd = true;
	tPreemptCtlCmd.m_byPreemptType = PREEMPT_TYPE_DEFAULT;
	tPreemptCtlCmd.m_bPatternInterruptCmd = true;
	tPreemptCtlCmd.m_tPatternInterruptCmd.m_nControlMode = tManualCmd.m_tPatternInterruptCmd.m_nControlMode;
	tPreemptCtlCmd.m_tPatternInterruptCmd.m_nPatternNo = tManualCmd.m_tPatternInterruptCmd.m_nPatternNo;
	m_pLogicCtlStatus->SetPreemptCtlCmd(tPreemptCtlCmd);

	m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "CreatePatternInterruptCmdInPreemptControl, ControlMode:%d  PatternNo:%d", tPreemptCtlCmd.m_tPatternInterruptCmd.m_nControlMode, tPreemptCtlCmd.m_tPatternInterruptCmd.m_nPatternNo);
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
void COpenATCLogicCtlManager::GetVirtualTimeByGlobalCount(int& nYear, int& nMon, int& nDay, int& nHour, int& nMin, int& nSec, int& nWeek)
{
	unsigned long nCounterDiff = 0;
	int diffTime = 0;
	int tSpeedyRunInfo = 0;
	//int TempSec = 0;
	//int TempMin = 0;
	//int TempHour = 0;
	//int TempDay = 0;
	int monthDays[13][2] = { {0,0},{31,31},{28,29},{31,31},{30,30},{31,31},{30,30},{31,31},{31,31},{30,30},{31,31},{30,30},{31,31} };
	unsigned long nGlobalCount;
	TVirtualRunTime mVirtualTimeData;
	memset(&mVirtualTimeData, 0, sizeof(mVirtualTimeData));
	tSpeedyRunInfo = m_pLogicCtlParam->GetSpeedyRunInfo();
	m_pLogicCtlStatus->GetVirtualTimeData(mVirtualTimeData);
	nGlobalCount = m_pLogicCtlStatus->GetGlobalCounter();
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
		if (mVirtualTimeData.VirtualDay == monthDays[mVirtualTimeData.VirtualMon][m_pLogicCtlParam->isLeap(mVirtualTimeData.VirtualYear)] + 1)
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
		m_pLogicCtlStatus->SetVirtualTimeData(mVirtualTimeData);
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