/*=====================================================================
ģ���� ���˹����ܿ����Ż�ģ��
�ļ��� ��LogicCtlAIOptim.h
����ļ���
ʵ�ֹ��ܣ����ڶ����˹����ܿ����Ż�ģ��ʵ�ֽӿ�
���� ��������
��Ȩ ��<Copyright(c) 2019-2020 Suzhou Keda Technology Co.,Ltd. All right reserved.>
-----------------------------------------------------------------------
�޸ļ�¼��
�� ��           �汾     �޸���     �߶���     �޸ļ�¼
2019/9/26       V1.0     ������     ������     ����ģ��
=====================================================================*/

#ifndef LOGICCTLAIOPTIM_H
#define LOGICCTLAIOPTIM_H

#include "LogicCtlFixedTime.h"

typedef struct tagAIOptimInfo
{
	bool m_bPredetFlag;							//Ԥ���־
	BYTE m_byOptimPhase[MAX_PHASE_COUNT];		//�Ż���λ
	int  m_nRetainTime[MAX_PHASE_COUNT];		//��λ�̵Ƴ���ʱ��
}TAIOptimInfo, *PTAIOptimInfo;


class LogicCtlAIOptim : public CLogicCtlFixedTime
{
public:
	LogicCtlAIOptim();
	virtual ~LogicCtlAIOptim();

    //��ʼ���˹������Ż�������Ҫ�Ĳ���
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
};

#endif // !ifndef LOGICCTLAIOPTIM_H





















