/*=====================================================================
ģ���� ��ȫ����Ʒ�ʽʵ��ģ��
�ļ��� ��LogicCtlAllRed.cpp
����ļ���LogicCtlAllRed.h
ʵ�ֹ��ܣ�ȫ����Ʒ�ʽ�ӿ�ʵ��
���� ��������
��Ȩ ��<Copyright(c) 2019-2020 Suzhou Keda Technology Co.,Ltd. All right reserved.>
-----------------------------------------------------------------------
�޸ļ�¼��
�� ��           �汾     �޸���     �߶���     �޸ļ�¼
2019/9/14       V1.0     ������     ������     ����ģ��
=====================================================================*/

#include <memory.h> 
#include "LogicCtlAllRed.h"

CLogicCtlAllRed::CLogicCtlAllRed()
{

}

CLogicCtlAllRed::~CLogicCtlAllRed()
{

}

/*==================================================================== 
������ ��Init 
���� ��ȫ����Ʒ�ʽ������Դ��ʼ��
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
void CLogicCtlAllRed::Init(COpenATCParameter * pParameter, COpenATCRunStatus * pRunStatus, COpenATCLog * pOpenATCLog, int nPlanNo)
{
    m_pOpenATCParameter = pParameter;
    m_pOpenATCRunStatus = pRunStatus;
	m_pOpenATCLog       = pOpenATCLog;

	memset(&m_tRunStageInfo, 0x00, sizeof(m_tRunStageInfo));
    memset(m_nChannelSplitMode,0x00,sizeof(m_nChannelSplitMode)); 

    //��ǰ����ģʽΪȫ��
    m_nCurRunMode = CTL_MODE_ALLRED;

    TLogicCtlStatus tCtlStatus;
    m_pOpenATCRunStatus->GetLogicCtlStatus(tCtlStatus);
    tCtlStatus.m_nCurCtlMode = CTL_MODE_ALLRED;
    tCtlStatus.m_nCurPlanNo = 0;						//ȫ��ʱ���صķ�������0
    m_pOpenATCRunStatus->SetLogicCtlStatus(tCtlStatus);  

    InitChannelParam();  

    memset(m_achLampClr,LAMP_CLR_UNDEF,sizeof(m_achLampClr)); 
}

/*==================================================================== 
������ ��Run 
���� ��ȫ����Ʒ�ʽ������
�㷨ʵ�� �� 
����˵�� ��
����ֵ˵����
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ������          �������� 
====================================================================*/ 
void CLogicCtlAllRed::Run()
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
        SetOneChannelOutput(pStartPos,C_CH_PHASESTAGE_R);
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

        //���õ�ɫ
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
