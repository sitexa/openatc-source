/*=====================================================================
ģ���� �����ȿ��Ʒ�ʽʵ��ģ��
�ļ��� ��LogicCtlPreempt.h
����ļ���
ʵ�ֹ��ܣ����ڶ������ȿ���ģʽʵ�ֽӿ�
���� ������Ƽ
��Ȩ ��<Copyright(c) 2019-2020 Suzhou Keda Technology Co.,Ltd. All right reserved.>
-----------------------------------------------------------------------
�޸ļ�¼��
�� ��           �汾     �޸���     �߶���     �޸ļ�¼
2019/9/26       V1.0     ����Ƽ     ����Ƽ      ����ģ��
=====================================================================*/

#ifndef LOGICCTLPREEMPT_H
#define LOGICCTLPREEMPT_H

#include "LogicCtlFixedTime.h"

class CLogicCtlPreempt : public CLogicCtlFixedTime  
{
public:
	CLogicCtlPreempt();
	virtual ~CLogicCtlPreempt();

    //��ʼ���ֶ����Ʒ�ʽ��Ҫ�Ĳ�����״̬,��Ҫ���ڳ�ʼ��״̬,�������ֶ�֮ǰ�Ŀ��Ʒ�ʽ�̳ж���
    virtual void Init(COpenATCParameter * pParameter, COpenATCRunStatus * pRunStatus, COpenATCLog * pOpenATCLog, int nPlanNo);

    //�Ż����Ʒ�ʽ������
    virtual void Run();

    //�����Ż�����ʱ�Ĳ���
    virtual void SetCtlDerivedParam(void * pParam,void * pLampClr,void * pStageTable,void * pChannelSplitMode);

protected:
	//�Ż����ƻ��ڻ�������λ������״̬����
    void PreemptCtlOnePhaseRun(int nRingIndex);

	//�Ż����ƻ���������λ������״̬����
    void PreemptCtlOnePedPhaseRun(int nRingIndex);

    //�Ż�����ʱ��λ����״̬����
    void PreemptCtlPhaseRun();

    //�໷�������н����жϸ���
    void ManualCycleChg(bool bIsAutoCtl);

private:
	enum
	{
		MAX_GREEN_TIME = 32000,
	};

	enum
	{
		MANUAL_CONTROL_STATUS = 1,
		MANUAL_STAGE_TRANS = 2,
	};

	enum
	{
		MANUAL_MINI_GREEN			= 1,
		MANUAL_PHASE_RUN_GREEN      = 2,
		MANUAL_PHASE_SPLIT_GREEN    = 3,
		MANUAL_PHASE_TOPLIMIT_GREEN = 4,
	};

	enum
	{
		STEPWARD_TO_NONSTEPWARD = 1,
		STEPWARD_TO_STEPWARD    = 2,
		NONSTEPWARD_TO_STEPWARD = 3,
		FIRSTPANEL_TO_DIRECTION = 4,
		FIRSTPANEL_TO_PATTERN   = 5,
		PATTERN_TO_CHANNELLOCK  = 6,
		PATTERN_TO_PATTERN      = 7,
		FIRSTPANEL_TO_FIRSTSTEPWARD = 8,
	};

    bool  ManualSwitchStage(int & nStageIndex);

	bool  TransCurStageToNextStage();

	void  CheckIfStageHavePhaseInGreen(int & nGreenPhaseCount);

	void  SendCountDownPulse(int nRingIndex, bool bNeedSendRedPulseFlag);

	void  SendPedCountDownPulse(int nRingIndex, bool bNeedSendRedPulseFlag);

	int   GetPhaseRemainTime(int nRingIndex, TPhaseCtlInfo * pPhaseCtlInfo, TPhaseLampClrRunCounter tRunCounter, DWORD wPhaseTime);

	int   GetPedPhaseRemainTime(int nRingIndex, TPhaseCtlInfo * pPhaseCtlInfo, TPhaseLampClrRunCounter tRunCounter, DWORD wPhaseTime);

	void  SendRedPulse(int nPhaseType, PTRingCtlInfo ptRingRunInfo, int nRingIndex, int nIndex, int nNextIndex, bool bFlag);

	void  ResetNextStagePhaseGreenTime(int nRingIndex, int nPhaseIndex, int nNextStageIndex, bool bChangePhaseFlag, int nDuration);

	void  RecalculteStageTimeByDelayTimeAndDuration();

	void  InitNextStagePhase(int nRingIndex, int nPhaseIndex, int nNextStageIndex, bool bChangePhaseFlag);

	void  CreatePreemptCtlCmdReturnToSelf();

	void  SetTransStatus();

	void  ClearPulse();

	bool  AdjustPhaseGreenTime(int nRingIndex, int nIndex, int nStageIndexTarget, int nGreenTimeFlag);

	bool  AdjustPedPhaseGreenTime(int nRingIndex, int nIndex, int nStageIndexTarget, int nGreenTimeFlag);

	void  SetOverlapPhaseLampClr(int nNextStageIndex);

	bool  GetPreemptCtlCmdFromList();

	bool  IsHaveUrgentCtlCmd();

	int   GetRingIndex(int nPhaseID);

	int   GetNextStageIndex();

	void  SetPreemptCtlCmd(int nStageIndex);

	char  GetPreemptCtlPhaseStage(int nPhaseID);

	void  SetOverlapCurPreemptPhaseLampClrChgFlag();

	void  SetOverlapCurPreemptPhaseLampClr(int nOverlapIndex, BYTE byOverlapNum, bool bPedOverlapPhase);

	bool  IsNeglectPhase(int nPhaseID);

	int   SetNeglectPhaseRunTime(int nRingIndex, bool bPedPhase);

	void  ReSetNeglectPhaseStageRunTime();

	void  ReSetNeglectPhasetBackUpTime();

 private:
    TFixTimeCtlInfo      m_tOldFixTimeCtlInfo;								//�����ֶ�����л�ʱ��������������Ϳ���״̬��Ϣ

	bool                 m_bClearPulseFlag;									//���ڼ�¼�Ƿ��������

	bool                 m_bSendRedPulse[MAX_RING_COUNT];					//���ڼ�¼�������������Ƿ�������

	bool                 m_bSendPedRedPulse[MAX_RING_COUNT];				//���ڼ�¼���˺������Ƿ�������

	int                  m_nManualCurStatus;								//���ڹ���״̬

	int                  m_nNextStageIndex;									//���ڼ�¼�¸��׶ε�����

	bool                 m_bCycleEndFlag;									//���ڼ�¼�Ƿ��������������

	int					 m_nReturnAutoCtrlStageIndex;						//�����û���Ԥ��ʱ���Զ�������������

	int					 m_nStageRunTime[MAX_RING_COUNT];					//���ڼ�¼�׶�����ʱ��

	int					 m_nStageTimeForPhasePass[MAX_RING_COUNT];			//���ڼ�¼�׶�ʱ��

	bool				 m_bPhaseColorChgToYellowFlag[MAX_RING_COUNT];		//���ڼ�¼��������λ�Ƿ������Ƶ�	

	bool                 m_bPreemptCtlCmdEndFlag;						    //���ڼ�¼���ȿ���ָ��ִ�н�����־

	bool                 m_bPreemptCtlStageProcessFlag[MAX_STAGE_COUNT];    //���ڼ�¼���Ƚ׶δ����־

	TPreemptCtlCmd       m_tPreemptCtlCmd;                                  //δ��ʼ�����ȿ���֮ǰ�����ȿ��ƻ���

	TPreemptCtlCmd       m_tNextPreemptCtlCmd;                              //��ʼ�����ȿ���֮�����һ�����ȿ��ƻ���

	bool                 m_bOverlapChgFlag[MAX_OVERLAP_COUNT];              //������������λ��ĸ��λ��������λ�ĵ�ɫ�仯��־

	bool                 m_bPedOverlapChgFlag[MAX_OVERLAP_COUNT];           //���˸�����λ��ĸ��λ��������λ�ĵ�ɫ�仯��־

	bool                 m_bCurAndNextCmdInSameStage[MAX_RING_COUNT];       //��ǰ���ȿ��ƺ���һ�����ȿ�����ͬһ���׶�
};

#endif // ifndef LOGICCTLPREEMPT_H
