/*=====================================================================
ģ���� ����Ӧ���Ʒ�ʽʵ��ģ��
�ļ��� ��LogicCtlActuate.h
����ļ���
ʵ�ֹ��ܣ����ڶ����Ӧ����ģʽʵ�ֽӿ�
���� ��������
��Ȩ ��<Copyright(c) 2019-2020 Suzhou Keda Technology Co.,Ltd. All right reserved.>
-----------------------------------------------------------------------
�޸ļ�¼��
�� ��           �汾     �޸���     �߶���     �޸ļ�¼
2019/9/26       V1.0     ������     ������     ����ģ��
=====================================================================*/

#ifndef LOGICCTLACTUATE_H
#define LOGICCTLACTUATE_H

#include "LogicCtlFixedTime.h"

class CLogicCtlActuate : public CLogicCtlFixedTime
{
public:
	CLogicCtlActuate();
	virtual ~CLogicCtlActuate();

    //��ʼ����Ӧ���Ʒ�ʽ��Ҫ�Ĳ���
    virtual void Init(COpenATCParameter * pParameter, COpenATCRunStatus * pRunStatus, COpenATCLog * pOpenATCLog, int nPlanNo);

protected:
    //���ڻ�������λ������״̬����
    virtual void OnePhaseRun(int nRingIndex);

    //����������λ������״̬����
    virtual void OnePedPhaseRun(int nRingIndex);
    
    //������λ�ӳ���
    virtual void ProcExtendGreen(int nRingIndex,int nPhaseIndex,int nCurRunTime);

	//������λ�ӳ���
    virtual void LastConcurrencyPhaseBeforeBarrierExtendGreen(int nRingIndex,int nPhaseIndex,int nCurRunTime);
    
private:
	bool				m_bChangeToYellow[MAX_RING_COUNT];										//����ͨ��������Ҫ��������ͨ��������ͬʱ����
};

#endif // !ifndef LOGICCTLACTUATE_H





















