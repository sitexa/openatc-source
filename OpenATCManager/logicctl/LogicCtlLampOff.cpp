/*=====================================================================
ģ���� ���صƿ��Ʒ�ʽʵ��ģ��
�ļ��� ��LogicCtlLampOff.cpp
����ļ���LogicCtlLampOff.h
ʵ�ֹ��ܣ��صƿ��Ʒ�ʽ�ӿ�ʵ��
���� ��������
��Ȩ ��<Copyright(c) 2019-2020 Suzhou Keda Technology Co.,Ltd. All right reserved.>
-----------------------------------------------------------------------
�޸ļ�¼��
�� ��           �汾     �޸���     �߶���     �޸ļ�¼
2019/9/14       V1.0     ������     ������     ����ģ��
=====================================================================*/

#include "LogicCtlLampOff.h"
#include <memory.h> 

CLogicCtlLampOff::CLogicCtlLampOff()
{

}

CLogicCtlLampOff::~CLogicCtlLampOff()
{

}

/*==================================================================== 
������ ��Init 
���� ���صƿ��Ʒ�ʽ����Դ��ʼ��
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
void CLogicCtlLampOff::Init(COpenATCParameter * pParameter, COpenATCRunStatus * pRunStatus, COpenATCLog * pOpenATCLog, int nPlanNo)
{
    m_pOpenATCParameter = pParameter;
    m_pOpenATCRunStatus = pRunStatus;
	m_pOpenATCLog       = pOpenATCLog;

	memset(&m_tRunStageInfo, 0x00, sizeof(m_tRunStageInfo));
    memset(m_nChannelSplitMode,0x00,sizeof(m_nChannelSplitMode)); 

    //��ǰ����ģʽΪ�ص�
    m_nCurRunMode = CTL_MODE_OFF;

    TLogicCtlStatus tCtlStatus;
    m_pOpenATCRunStatus->GetLogicCtlStatus(tCtlStatus);
    tCtlStatus.m_nCurCtlMode = CTL_MODE_OFF;
    tCtlStatus.m_nCurPlanNo = 0;//�ص�ʱ���صķ�������0
    m_pOpenATCRunStatus->SetLogicCtlStatus(tCtlStatus);   

    memset(m_achLampClr,LAMP_CLR_UNDEF,sizeof(m_achLampClr)); 
}

/*==================================================================== 
������ ��Run 
���� ���صƿ��Ʒ�ʽ������
�㷨ʵ�� �� 
����˵�� ��
����ֵ˵����
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ������          �������� 
====================================================================*/ 
void CLogicCtlLampOff::Run()
{
    bool bRefresh = false;

    char achLampClr[C_N_MAXLAMPBOARD_NUM * C_N_LAMPBORAD_OUTPUTNUM];
    memset(achLampClr,LAMP_CLR_OFF,sizeof(achLampClr)); 


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
