/*=====================================================================
ģ���� ������Ӧʽ�̲����Ʒ�ʽʵ��ģ��
�ļ��� ��LogicCtlAdaptiveGreenWave.h
����ļ���
ʵ�ֹ��ܣ���������Ӧʽ�̲����Ʒ�ʽʵ�ֽӿ�
���� ��������
��Ȩ ��<Copyright(c) 2019-2020 Suzhou Keda Technology Co.,Ltd. All right reserved.>
-----------------------------------------------------------------------
�޸ļ�¼��
�� ��           �汾     �޸���     �߶���     �޸ļ�¼
2019/9/26       V1.0     ������     ������     ����ģ��
=====================================================================*/

#ifndef LOGICCTLADAPTIVEGREENWAVE_H
#define LOGICCTLADAPTIVEGREENWAVE_H

#include "LogicCtlFixedTime.h"
#include "LogicCtlWebsterOptim.h"

class CLogicCtlAdaptiveGreenWave : public CLogicCtlFixedTime
{
public:
	CLogicCtlAdaptiveGreenWave();
	virtual ~CLogicCtlAdaptiveGreenWave();

	//��ʼ����Ӧ���Ʒ�ʽ��Ҫ�Ĳ���
	virtual void Init(COpenATCParameter * pParameter, COpenATCRunStatus * pRunStatus, COpenATCLog * pOpenATCLog, int nPlanNo);

protected:
	//���ڻ�������λ������״̬����
	virtual void OnePhaseRun(int nRingIndex);

	//����������λ������״̬����
	virtual void OnePedPhaseRun(int nRingIndex);

	//������λ�ӳ���
	virtual void ProcExtendGreen(int nRingIndex, int nPhaseIndex, int nCurRunTime);

	//������λ�ӳ���
	virtual void LastConcurrencyPhaseBeforeBarrierExtendGreen(int nRingIndex, int nPhaseIndex, int nCurRunTime);

	//���ڽ������ÿ���״̬
	virtual void ReSetCtlStatus();
private:
	enum
	{
		WEBSTER_NO_VEH_PASS = 0,
		WEBSTER_MIN_SPLIT_TIME = 1,
		WEBSTER_MAX_SPLIT_TIME = 2,
		WEBSTER_MULTI_VEH_PASS = 3,
	};
	//���ݵ�ǰ������Ϣ������һ���ڵ����ű�
	void CalcPhaseTime();

	//��ʼ���Ż���ز���
	void InitWebsterOptimParam();

	//���ó�ʼ����λ����
	void ResetPhaseWebsterOptimInfo(int nRingIndex, int nPhaseIndex);

	//������λ�Ż�����
	void ProcPhaseWebsterOptimInfo(int nRingIndex, int nPhaseIndex);

	//������λ���Ͷ�
	bool CalcWebsterBestCycleLength(int & nIndex, int & nBestCycleTimeLength, int & nCycleLostTime, float & fFlowRatio);

	//���¼������λ���ű�
	void RecalcWebsterBestSplitTime(int nIndex, int nBestCycleLength, int nCycleLostTime, float & fFlowRatio);
	void RecalcWebsterBestSplitTimeByPlan();

	//������λ���ű�
	int  SetPhaseSplitTime(int nRingIndex, int nPhaseIndex, WORD wSplitTime, bool bFlag);

	//�жϵ�ǰ��λ�Ƿ�������ʹ�õļ����
	bool CheckPhaseIfHavaEffectiveDet(int nRingIndex, int nPhaseIndex);

	bool m_bCalcSplitTimeFlag;																	//���űȼ����־

	BYTE m_byDetectorTmpStatus[MAX_VEHICLEDETECTOR_COUNT];										//��ʱ�洢�����״̬,���ڼ�����Чͨ��ʱ��
	int m_anDetectorTmpCounter[C_N_MAXDETBOARD_NUM * C_N_MAXDETINPUT_NUM];						//��ʱ����������ʱ�������,���ڼ�����Чͨ��ʱ��

	TWebsterOptimInfo   m_tWebsterOptimInfo;													//�Ż���Ϣ
	int					m_nPhaseVehNumInfo[MAX_RING_COUNT][MAX_SEQUENCE_TABLE_COUNT];
	int					m_nPhaseValidVehNum[MAX_RING_COUNT][MAX_SEQUENCE_TABLE_COUNT];
	float				m_fPhaseValidVehPassTime[MAX_RING_COUNT][MAX_SEQUENCE_TABLE_COUNT];
	float				m_fPhaseValidMinVehPassTime[MAX_RING_COUNT][MAX_SEQUENCE_TABLE_COUNT]; //��¼·����λ����С��ͷʱ��
	float				m_fPhaseFlowRatio[MAX_RING_COUNT][MAX_SEQUENCE_TABLE_COUNT];
	float               m_fPhaseBalanceFlowRatio[MAX_RING_COUNT][MAX_SEQUENCE_TABLE_COUNT];
	WORD				m_wOriPlanPhaseSplitTime[MAX_RING_COUNT][MAX_SEQUENCE_TABLE_COUNT];
	WORD				m_wOriPlanPhaseSplitTimePlan[MAX_RING_COUNT][MAX_SEQUENCE_TABLE_COUNT];
	int					m_nMinBestCycleLength[MAX_RING_COUNT];
	int					m_nMaxBestCycleLength[MAX_RING_COUNT];
	float               m_fBalanceTotalFlowRatio;
	int					m_nValidSplitTime[MAX_RING_COUNT][MAX_PHASE_COUNT];						//��¼ÿ����λ�Ż�������ű�ʱ��

	TAscSingleOptim m_tAscSingleOptimInfo;														//��������Ӧ���Ʋ���

	//ÿ������Ӧ����λ��
	int					m_nPhaseNum[MAX_RING_COUNT];
};

#endif // !ifndef LOGICCTLACTUATE_H



















