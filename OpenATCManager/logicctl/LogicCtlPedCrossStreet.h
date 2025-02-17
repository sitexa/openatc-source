/*=====================================================================
ģ���� �����˹��ֿ��Ʒ�ʽʵ��ģ��
�ļ��� ��LogicCtlCablelessLine.h
����ļ���
ʵ�ֹ��ܣ����ڶ������˹��ֿ���ģʽʵ�ֽӿ�
���� ��������
��Ȩ ��<Copyright(c) 2019-2020 Suzhou Keda Technology Co.,Ltd. All right reserved.>
-----------------------------------------------------------------------
�޸ļ�¼��
�� ��           �汾     �޸���     �߶���     �޸ļ�¼
2019/9/26       V1.0     ������     ������     ����ģ��
=====================================================================*/

#ifndef LOGICCTLPEDCROSSSTREET_H
#define LOGICCTLPEDCROSSSTREET_H

#include "LogicCtlFixedTime.h"

class CLogicCtlPedCrossStreet : public CLogicCtlFixedTime  
{
public:
	CLogicCtlPedCrossStreet();
	virtual ~CLogicCtlPedCrossStreet();

    //��ʼ�����˹��ֿ��Ʒ�ʽ��Ҫ�Ĳ���
    virtual void Init(COpenATCParameter * pParameter, COpenATCRunStatus * pRunStatus, COpenATCLog * pOpenATCLog,  int nPlanNo);

protected:
    //���ڻ�������λ������״̬����
    virtual void OnePhaseRun(int nRingIndex);

    //����������λ������״̬����
    virtual void OnePedPhaseRun(int nRingIndex);

    //���˼�����Ƿ�����������
    virtual bool IsPedAskCrossStreet();

private:
    bool  m_bIsPedAskPhase;

	int   m_nGreenRunTime[MAX_RING_COUNT][MAX_SEQUENCE_TABLE_COUNT];
};

#endif // ifndef LOGICCTLPEDCROSSSTREET_H
