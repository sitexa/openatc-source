/*=====================================================================
ģ���� ��ͨ�������Ʒ�ʽʵ��ģ��
�ļ��� ��LogicCtlChannelCheck.cpp
����ļ���LogicCtlChannelCheck.h
ʵ�ֹ��ܣ�ͨ�����ӿ�ʵ��
���� ��������
��Ȩ ��<Copyright(c) 2019-2020 Suzhou Keda Technology Co.,Ltd. All right reserved.>
-----------------------------------------------------------------------
�޸ļ�¼��
�� ��           �汾     �޸���     �߶���     �޸ļ�¼
2019/9/14       V1.0     ������     �� ��      ����ģ��
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
������ ��Init
���� ���صƿ��Ʒ�ʽ����Դ��ʼ��
�㷨ʵ�� ��
����˵�� ��pParameter����������ָ��
           pRunStatus��ȫ������״̬��ָ��
		   pOpenATCLog����־ָ��
           nPlanNo��ʹ�õķ����ţ�0��ʾ����ʱ��ȡ���� 
����ֵ˵����
----------------------------------------------------------------------
�޸ļ�¼ ��
�� ��          �汾 �޸���  �߶���  �޸ļ�¼
2019/09/14     V1.0 ������          ��������
====================================================================*/
void CLogicCtlChannelCheck::Init(COpenATCParameter * pParameter, COpenATCRunStatus * pRunStatus, COpenATCLog * pOpenATCLog, int nPlanNo)
{
	m_pOpenATCParameter = pParameter;
	m_pOpenATCRunStatus = pRunStatus;

	TLogicCtlStatus tCtlStatus;
	m_pOpenATCRunStatus->GetLogicCtlStatus(tCtlStatus);
	tCtlStatus.m_nCurCtlMode = CTL_MODE_CHANNEL_CHECK;
	tCtlStatus.m_nCurPlanNo = 0;//ͨ�����ʱ���صķ�������0
	m_pOpenATCRunStatus->SetLogicCtlStatus(tCtlStatus);

	memset(m_achLampClr, LAMP_CLR_UNDEF, sizeof(m_achLampClr));

	memset(m_bKeepGreenChannelBeforeControlChannelFlag, false, sizeof(m_bKeepGreenChannelBeforeControlChannelFlag));
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
void CLogicCtlChannelCheck::Run()
{
	//��ȡ��ͨ���������
	TAscChannelVerifyInfo	atChannelCheckInfo;	
	memset(&atChannelCheckInfo, 0, sizeof(atChannelCheckInfo));
	m_pOpenATCRunStatus->GetChannelCheckInfo(atChannelCheckInfo);
		
	//��ȡ��ɫ״̬��Ϣ
	bool bRefresh = false;
	char achLampClr[C_N_MAXLAMPBOARD_NUM * C_N_LAMPBORAD_OUTPUTNUM];
	memcpy(&achLampClr, atChannelCheckInfo.m_achLampClr, sizeof(achLampClr));

	//�ص�ָ��
	char achLampOff[C_N_MAXLAMPBOARD_NUM * C_N_LAMPBORAD_OUTPUTNUM];
	memset(achLampOff, LAMP_CLR_OFF, sizeof(achLampOff));

	if (memcmp(m_achLampClr, achLampClr, sizeof(achLampClr)) != 0)
	{
		if (!bRefresh)
		{
			bRefresh = true;	//����ͨ���������
		}
	}

	if (bRefresh)
	{
		TLampClrStatus tLampClrStatus;
		memset(&tLampClrStatus, 0, sizeof(TLampClrStatus));
		m_pOpenATCRunStatus->GetLampClrStatus(tLampClrStatus);
		if (memcmp(tLampClrStatus.m_achLampClr, achLampOff, sizeof(achLampOff)) != 0)	//�����ǰ״̬δ�صƽ��йص�
		{
			memcpy(m_achLampClr, achLampOff, sizeof(achLampOff));
			SetLampClr(tLampClrStatus);
			tLampClrStatus.m_bIsRefreshClr = true;
			m_pOpenATCRunStatus->SetLampClrStatus(tLampClrStatus);
		}
		//ͨ��������õ�ɫ���
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
