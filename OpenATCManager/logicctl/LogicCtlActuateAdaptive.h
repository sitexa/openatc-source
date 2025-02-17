/*=====================================================================
ģ���� ��Τ��˹������Ӧʵ��ģ��
�ļ��� ��LogicCtlActuateAdaptive.h
����ļ���
ʵ�ֹ��ܣ����ڶ���Τ��˹������Ӧģʽʵ�ֽӿ�
���� ��������
��Ȩ ��<Copyright(c) 2019-2020 Suzhou Keda Technology Co.,Ltd. All right reserved.>
-----------------------------------------------------------------------
�޸ļ�¼��
�� ��           �汾     �޸���     �߶���     �޸ļ�¼
2019/9/26       V1.0     ������     ������     ����ģ��
=====================================================================*/

#ifndef LOGICCTLACTUATEADAPTIVE_H
#define LOGICCTLACTUATEADAPTIVE_H

#include "LogicCtlFixedTime.h"
#include "LogicCtlWebsterOptim.h"

class CLogicCtlActuateAdaptive : public CLogicCtlFixedTime
{
public:
	CLogicCtlActuateAdaptive();
	virtual ~CLogicCtlActuateAdaptive();

	//��ʼ������Ӧ���Ʒ�ʽ��Ҫ�Ĳ���
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

	//�������С��ͷʱ��ͳ�Ƽ���
	void ProcPhaseDetMinTimeDis(int nRingIndex, int nPhaseIndex);

	//������λ���Ͷ�
	bool CalcWebsterBestCycleLength(int & nIndex, int & nBestCycleTimeLength, int & nCycleLostTime, float & fFlowRatio);

	//���¼������λ���ű�
	void RecalcWebsterBestSplitTime(int nIndex, int nBestCycleLength, int nCycleLostTime, float & fFlowRatio);
	void RecalcWebsterBestSplitTimeByPlan();

	//������λ���ű�
	int  SetPhaseSplitTime(int nRingIndex, int nPhaseIndex, WORD wSplitTime, bool bFlag);

	//�жϵ�ǰ��λ�Ƿ�������ʹ�õļ����
	bool CheckPhaseIfHavaEffectiveDet(int nRingIndex, int nPhaseIndex);

	bool m_bCalcSplitTimeFlag;																	    //���űȼ����־

	bool m_bIsFirstCycle;																			//�ж��Ƿ��������Ӧ���Ƶĵ�һ������

	BYTE m_byDetectorTmpStatus[MAX_VEHICLEDETECTOR_COUNT];										    //��ʱ�洢�����״̬,���ڼ�����Чͨ��ʱ��
	int m_anDetectorTmpCounter[C_N_MAXDETBOARD_NUM * C_N_MAXDETINPUT_NUM];						    //��ʱ����������ʱ�������,���ڼ�����Чͨ��ʱ��

    //�����Ż�����ĳ���������Ϣ������״̬������ʱ�䣬�������ȵ�
	TWebsterOptimInfo   m_tWebsterOptimInfo;													    //�Ż���Ϣ

    //�������ڸ�����λ�ľ����ĳ�����Ϣ��Ŀǰ��ȡ��4��ֵ��0��1��2��3
	int					m_nPhaseVehNumInfo[MAX_RING_COUNT][MAX_PHASE_COUNT];               
	
    //ͳ�Ƽ�¼�������ڸ�����λ�����ĳ�������Ϣ 
    int					m_nPhaseValidVehNum[MAX_RING_COUNT][MAX_PHASE_COUNT];              
		
    //��¼�������ڸ�����λ����С��ͷʱ������Ϣ���и���ʼֵ������ᶯ̬����
    float				m_fPhaseValidMinVehPassTime[MAX_RING_COUNT][MAX_PHASE_COUNT];      

    //����������ڸ�����λ�������ȣ�����λ������������Ա�������	
    float				m_fPhaseFlowRatio[MAX_RING_COUNT][MAX_PHASE_COUNT];                
	   
    //����������ڸ�����λ��ƽ�������ȣ�ͨ��m_fPhaseFlowRatio������Ӧ���ò�������õ�
    float               m_fPhaseBalanceFlowRatio[MAX_RING_COUNT][MAX_PHASE_COUNT]; 

    //����������ڸ�����λ�����űȣ���ʼֵ�������������õ����ű�
	WORD				m_wOriPlanPhaseSplitTime[MAX_RING_COUNT][MAX_PHASE_COUNT];
	
    //���������õ����ű�
    WORD				m_wOriPlanPhaseSplitTimePlan[MAX_RING_COUNT][MAX_PHASE_COUNT];

    //��¼ÿ����λ�Ż�������ű�ʱ�䣬Ŀǰ��û���ô�����ע�͵�
//	int					m_nValidSplitTime[MAX_RING_COUNT][MAX_PHASE_COUNT];
						    
    //�������������������������С�Ŀ��ܵ����ڳ�
	int					m_nMinBestCycleLength[MAX_RING_COUNT];
    //��������������������������Ŀ��ܵ����ڳ�
	int					m_nMaxBestCycleLength[MAX_RING_COUNT]; 
    
    //������λ�����������
	float               m_fBalanceTotalFlowRatio;

    //�����Ż��������������
	TAscSingleOptim m_tAscSingleOptimInfo;

	//������ʵ�����е����ű�ʱ��
	WORD				m_wRealSplitTime[MAX_RING_COUNT][MAX_PHASE_COUNT];

	//��¼�ϸ�����ÿ������ʵ����������
	int					m_nRealCycleLength[MAX_RING_COUNT];

	//ÿ������Ӧ����λ��
	int					m_nPhaseNum[MAX_RING_COUNT];
};

#endif // !ifndef LOGICCTLACTUATEADAPTIVE_H



















