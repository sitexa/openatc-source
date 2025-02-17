/*=====================================================================
ģ���� ����Ӧʽ�̲�����ģ��
�ļ��� ��LogicCtlInductiveGreenWave.h
����ļ���
ʵ�ֹ��ܣ����ڶ����Ӧʽ�̲�����ʵ�ֽӿ�
���� ������
��Ȩ ��<Copyright(c) 2019-2020 Suzhou Keda Technology Co.,Ltd. All right reserved.>
-----------------------------------------------------------------------
�޸ļ�¼��
�� ��           �汾     �޸���     �߶���     �޸ļ�¼
2019/9/26       V1.0     ����       ����       ����ģ��
=====================================================================*/

#ifndef LOGICCTL_INDUCTIVE_GREEN_WAVE_H
#define LOGICCTL_INDUCTIVE_GREEN_WAVE_H

#include "LogicCtlFixedTime.h"

const float C_F_ADJ_KEY_MODE = (float)0.3;
class CLogicCtlInductiveGreenWave :public CLogicCtlFixedTime
{
public:
	CLogicCtlInductiveGreenWave();
	virtual ~CLogicCtlInductiveGreenWave();

	//��ʼ����Ӧ���Ʒ�ʽ��Ҫ�Ĳ���
	virtual void Init(COpenATCParameter * pParameter, COpenATCRunStatus * pRunStatus, COpenATCLog * pOpenATCLog, int nPlanNo);

protected:
	//���ڻ�������λ������״̬����
	virtual void OnePhaseRun(int nRingIndex);

	//����������λ������״̬����
	virtual void OnePedPhaseRun(int nRingIndex);

	//���ڽ������ÿ���״̬
	virtual void ReSetCtlStatus();

	//������λ�ӳ���
	virtual void ProcExtendGreen(int nRingIndex,int nPhaseIndex,int nCurRunTime);

	//������λ�ӳ���
	virtual void LastConcurrencyPhaseBeforeBarrierExtendGreen(int nRingIndex,int nPhaseIndex,int nCurRunTime);

	//�����̲���λ���̵�ʱ��
	virtual void CalcGreenWavePhaseTime();

private:
	//����ʵ����λ��,������λ�̵�ʱ��
	short CalcRealOffset();

	//������λ��,�̵�ʱ�����С�̣������������ʵ�ʵ�������
	short CalcRealAdjVal(short nOffset, WORD wGreenTime, WORD wMinGreen, WORD wMaxGreen);

	//short m_nRealOffset;									//ʵ����λ��

	short m_nGreenStageChgLen;								//��ǰ�̵ƽ׶οɵ����ĳ���

	bool m_bCalcOffsetFlag;									//������λ���־

	bool m_bOffsetCoordinateCloseFlag;						//��λ��Э��ֹͣ��־
	bool m_bCycleCoordinateCloseFlag;						//����Э��������־
	int m_nGreenPhaseSplitTime[MAX_RING_COUNT];
	int CurRunTotalCounter[MAX_RING_COUNT][MAX_PHASE_COUNT];//����λ�����ۻ�ʱ��
};
#endif //#!LOGICCTL_INDUCTIVE_GREEN_WAVE_H