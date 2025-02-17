/*=====================================================================
ģ���� ���ֶ����Ʒ�ʽʵ��ģ��
�ļ��� ��LogicCtlManual.h
����ļ���
ʵ�ֹ��ܣ����ڶ����ֶ�����ģʽʵ�ֽӿ�
���� ��������
��Ȩ ��<Copyright(c) 2019-2020 Suzhou Keda Technology Co.,Ltd. All right reserved.>
-----------------------------------------------------------------------
�޸ļ�¼��
�� ��           �汾     �޸���     �߶���     �޸ļ�¼
2019/9/26       V1.0     ������     ������      ����ģ��
2019/2/28       V1.0     ����Ƽ     ������      �޸�ϵͳ�û������߼�
2019/3/28       V1.0     ����Ƽ     ������      �����ֶ���尴ť�л�����
=====================================================================*/

#ifndef LOGICCTLMANUAL_H
#define LOGICCTLMANUAL_H

#include "LogicCtlFixedTime.h"

const WORD MAX_GREEN_TIME = 32000;

typedef struct tagLockPhaseData
{
	int   nCurLockRingIndex;                    //��ǰ������λ����
	int   nCurLockPhaseID;                      //��ǰ������λID
	int   nCurLockStageIndex;                   //��ǰ�����׶α��
	int   nTargetPhaseIDInCurRing;              //��ǰ����Ŀ����λID���ͷǵ�ǰ����Ŀ����λ����ͬһ���׶�
	int   nTargetStageIndex;                    //Ŀ��׶α�ţ��ǵ�ǰ����Ŀ����λ��Ӧ�Ľ׶α��
	int   nLockChannelCount;                    //��ǰ������λ��Ӧͨ������           
	int   nLockChannelID[MAX_CHANNEL_COUNT];    //��ǰ������λ��Ӧͨ�����   
	bool  bNeedTransFlag;                       //��Ҫ��Ŀ����λ�л���־
	bool  bSwitchLockChannelToNextPhase;        //������λ���ɽ����Ժ���Ŀ����λ�л�����ʼ����һ����λ��־
	bool  bSwitchSuccessFlag;                   //�ɹ��л���λ������һ������������λ��ͬ�׶α�־
	bool  bLockPhaseInSameStageFlag;            //������λ��ͬһ���׶α�־
}TLockPhaseData,PLockPhaseData;

typedef struct tagLockPhaseStage
{
	int   nLockPhaseCount;                      //���ڼ�¼������λ����
	int   nLockPhaseID[MAX_PHASE_COUNT];        //���ڼ�¼��ǰ������λID
	char  chLockPhaseStage[MAX_PHASE_COUNT];    //���ڼ�¼��ǰ������λ״̬
	int   nLockPhaseCounter[MAX_PHASE_COUNT];	//���ڼ�¼��ǰ������λ����ʱ��
}TLockPhaseStage,PLockPhaseStage;

class CLogicCtlManual : public CLogicCtlFixedTime  
{
public:
	CLogicCtlManual();
	virtual ~CLogicCtlManual();

    //��ʼ���ֶ����Ʒ�ʽ��Ҫ�Ĳ�����״̬,��Ҫ���ڳ�ʼ��״̬,�������ֶ�֮ǰ�Ŀ��Ʒ�ʽ�̳ж���
    virtual void Init(COpenATCParameter * pParameter, COpenATCRunStatus * pRunStatus, COpenATCLog * pOpenATCLog, int nPlanNo);

    //�ֶ����Ʒ�ʽ������
    virtual void Run();

    //�����ֶ�����ʱ�Ĳ���
    virtual void SetCtlDerivedParam(void * pParam,void * pLampClr,void * pStageTable,void * pChannelSplitMode);

protected:
    //�ֶ����ƻ��ڻ�������λ������״̬����
    virtual void ManualOnePhaseRun(int nRingIndex,TManualCmd tValidManualCmd);

    //ϵͳ��Ԥ���ڻ�������λ������״̬����
    virtual void SysCtlOnePhaseRun(TManualCmd  & tValidManualCmd);

    //�ֶ����ƻ���������λ������״̬����
    virtual void ManualOnePedPhaseRun(int nRingIndex,TManualCmd tValidManualCmd);

    //�໷�������н����жϸ���
    virtual void ManualCycleChg(bool bIsAutoCtl);

	//���÷����е�������������״̬
	virtual void GetTransRunStatusBeforeLockPhase(TPhaseRunStatus & tRunStatus);

	//������������״̬
	virtual void GetLockPhaseRunStatus(TPhaseRunStatus & tRunStatus, bool bInvalidPhaseCmd, TPhaseLockPara tInvalidPhaseLockPara);

private:
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
		LOCKCHANNEL_TO_DIRECTION    = 9,
	};

    bool  ManualSwitchStage(TManualCmd  tValidManualCmd, int & nStageIndex);

	void  ChangeChannelClr();

    bool  TransitChannelClr(BYTE byChannelType, bool bLockCmd);

    void  InitParamBeforeDirection(int nNextDirectionIndex);

	bool  IsTimeInSpan(int nCurHour, int nCurMin, int nCurSec, int nStartHour, int nStartMin, int nStartSec, int nEndHour, int nEndMin, int nEndSec);

	bool  CheckIfNeedToRunToNextStage(TManualCmd & tValidManualCmd);

	bool  CheckIfNeedToStepForwardToNextStage(TManualCmd & tValidManualCmd);

	void  CheckIfStageHavePhaseInGreen(int & nGreenPhaseCount, int & nClosePhaseCount);

	void  SendCountDownPulse(int nRingIndex, bool bNeedSendRedPulseFlag);

	void  SendPedCountDownPulse(int nRingIndex, bool bNeedSendRedPulseFlag);

    void  ProcessDirection(TManualCmd & tValidManualCmd);
	
	void  GetChannelData(int nNextDirectionIndex);

	void  ProcessStepward(TManualCmd  & tValidManualCmd);

	int   GetPhaseRemainTime(int nRingIndex, TPhaseCtlInfo * pPhaseCtlInfo, TPhaseLampClrRunCounter tRunCounter, DWORD wPhaseTime);

	int   GetPedPhaseRemainTime(int nRingIndex, TPhaseCtlInfo * pPhaseCtlInfo, TPhaseLampClrRunCounter tRunCounter, DWORD wPhaseTime);

	void  SendRedPulse(int nPhaseType, PTRingCtlInfo ptRingRunInfo, int nRingIndex, int nIndex, int nNextIndex, bool bFlag);

	void  ResetNextStagePhaseGreenTime(int nRingIndex, int nPhaseIndex, int nNextStageIndex, bool bChangePhaseFlag, int nDuration);

	void  RecalculteStageTimeByDelayTimeAndDuration(TManualCmd  tValidManualCmd);

	void  RecalculteCurPhaseGreenTimeAndSetStatus(TManualCmd  & tValidManualCmd);

	void  InitNextStagePhase(int nRingIndex, int nPhaseIndex, int nNextStageIndex, bool bChangePhaseFlag);

	void  CreateManualCmdReturnToSelf(TManualCmd tValidManualCmd);

	void  SetTransStatus(TManualCmd  & tValidManualCmd);

	void  ProcessFirstManual(TManualCmd  & tValidManualCmd);

	void  ProcessPhasePassControlStatus(TManualCmd  & tValidManualCmd);

	void  ClearPulse();

	void  PanelFirstStepWard(int nStageIndexTarget);

	void  NonStepWardToStepWard(int nStageIndexTarget);

	void  FirstPanelManualSwitchToDirectionOrPattern(TManualCmd  & tValidManualCmd);

	void  BackUpPhaseTime(int nRefreshStageTime);

	bool  AdjustPhaseGreenTime(int nRingIndex, int nIndex, int nStageIndexTarget, int nGreenTimeFlag);

	bool  AdjustPedPhaseGreenTime(int nRingIndex, int nIndex, int nStageIndexTarget, int nGreenTimeFlag);

	//���ø�����λ��ɫ
	void  SetOverlapPhaseLampClr(int nNextStageIndex);

	void  InitParamBeforeChannelLock(int nChannelLockStatus[]);

	void  ProcessChannelLock(TManualCmd & tValidManualCmd);

	void  PatternSwitchToChannelLock(TManualCmd  & tValidManualCmd);

	void  SetKeepGreenChannelBeforeControlChannel(BYTE byGreenStageVehPhaseID, BYTE byGreenStagePedPhaseID, TManualCmd tValidManualCmd);

	bool  CheckSendGreenPulse(BYTE byPhaseNumber);

	void  PhaseTransBasedOnControlChannelFlag();

	bool  IsChannelNeedTranClr(BYTE byChannelIndex);

	void  ProcessAllClosePhaseInCurStage(TManualCmd  tValidManualCmd, int nNextStageIndex);

	bool  IsNeedTransBeforeControlChannel(int nPhaseID);

	void  GetPhaseGreenTimeAfterLockPhase(int nRingIndex, int nPhaseIndex, int nNextStageIndex, bool bChangePhaseFlag);

	void  GetLockPhaseData(TManualCmd  & tValidManualCmd);

	void  GetNextPhaseAfterLockEnd(bool bInSameStage, int nLockPhaseCount);

	bool  IsTransChannel(BYTE byChannelType, int nChannelIndex);

	void  SwitchLockChannelPhaseToNext();

	bool  IsTargetPhaseChannel(int nChannelIndex);

	bool  IsOverlapNeedKeepGreen(BYTE byOverlapIndex, int nOverlapPhaseType);

	bool  IsHasTargetPhaseIncludedOldLockPhase(BYTE byOverlapIndex, int nOverlapPhaseType, int & nLockPhaseIndex);

	void  GetOldLockPhaseStatus(BYTE byOverlapIndex, int nOverlapPhaseType, char & chStatus);

	bool  LockChannelTransClr();

	int   ProcessLockChannelToPanel(TManualCmd  tValidManualCmd);

	bool  IsNeglectPhase(int nPhaseID);

	int   SetNeglectPhaseRunTime(int nRingIndex, bool bPedPhase);

	void  ReSetNeglectPhaseStageRunTime();

	void  ReSetNeglectPhasetBackUpTime();

	void  SetNeglectChannelBoforePhaseLock();

	void  SetLockPhaseList(TManualCmd  tValidManualCmd);

	void  SetLockPhaseStage(TManualCmd  tValidManualCmd);

 private:
	TManualCmd           m_tOldValidManualCmd;								//�������һ����Ч���ֶ���������

    TFixTimeCtlInfo      m_tOldFixTimeCtlInfo;								//�����ֶ�����л�ʱ��������������Ϳ���״̬��Ϣ

    bool                 m_bChangeChannelClr[MAX_CHANNEL_COUNT];			//���ڼ�¼�Ƿ�ı�ͨ����ɫ
   
    char                 m_chChannelStage[MAX_CHANNEL_COUNT];				//���ڼ�¼�������ʱ�Ļ�����ͨ����ɫ�׶�

	char                 m_chChannelStatus[MAX_CHANNEL_COUNT];				//���ڼ�¼�л��������ʱ���õĻ�����ͨ����ɫ�׶�

	int                  m_nChannelCounter[MAX_CHANNEL_COUNT];				//���ڼ�¼���뷽�����ʱͨ����ɫ����ʱ��

    int                  m_nChannelDurationCounter[MAX_CHANNEL_COUNT];		//���ڼ�¼���뷽�����ʱͨ����ɫ����ʱ��

	bool                 m_bChannelTran[MAX_CHANNEL_COUNT];					//�����жϼ�¼�������ʱ�Ļ�����ͨ���Ƿ�������

	bool                 m_bClearPulseFlag;									//���ڼ�¼�Ƿ��������

	bool                 m_bSendRedPulse[MAX_RING_COUNT];					//���ڼ�¼�������������Ƿ�������

	bool                 m_bSendPedRedPulse[MAX_RING_COUNT];				//���ڼ�¼���˺������Ƿ�������

	int                  m_nManualCurStatus;								//���ڹ���״̬

	int                  m_nNextStageIndex;									//���ڼ�¼�¸��׶ε�����

	bool                 m_bCycleEndFlag;									//���ڼ�¼�Ƿ��������������

	int					 m_nReturnAutoCtrlStageIndex;						//�����û���Ԥ��ʱ���Զ�������������

	bool                 m_bTransToAutoFlag;								//���ڼ�¼�Ƿ��л�����

	bool                 m_bIsDirectionChannelClrChg;						//����ͨ����ɫ�Ƿ����仯

	TPhasePassCmdPhaseStatus m_tPhasePassCmdPhaseStatus;					//���ڼ�¼��ǰ��λ����״̬

	TPhasePassCmdPhaseStatus m_tPhasePassCmdPhaseStatusFromUser;			//���ڼ�¼��λ����״̬����ָ��

	int					m_nStageRunTime[MAX_RING_COUNT];					//���ڼ�¼�׶�����ʱ��

	int					m_nStageTimeForPhasePass[MAX_RING_COUNT];			//���ڼ�¼�׶�ʱ��

	bool				m_bPhaseColorChgToYellowFlag[MAX_RING_COUNT];		//���ڼ�¼��������λ�Ƿ������Ƶ�	

	bool				m_bIfPhasePassCmdPhaseStatusHaveRefreshedToInit;	//���ڼ�¼�ֶ�����ģʽ�£���λ����״̬���Ʊ��Ƿ���ˢ��Ϊȫ������

	int                 m_nDirectionGreenTime[MAX_CHANNEL_COUNT];           //���ڼ�¼�������ʱ���̵Ƴ���ʱ��

	int                 m_nChannelLockGreenTime[MAX_CHANNEL_COUNT];         //����ͨ������ʱ���̵Ƴ���ʱ��

	bool                m_bChannelKeepGreenFlag[MAX_CHANNEL_COUNT];         //����ͨ�������л�ʱ�ļ��������̵Ʊ�־

	bool                m_bNextClrStageIsFInColorStep;                      //���ڼ�¼ɫ��ģʽ��һ����ɫ״̬Ϊ�ɽ����ı�־

	bool                m_bStageStepwardInColorStep;                        //���ڼ�¼ɫ��ģʽ�½׶β����ı�־

	TLockPhaseData      m_tLockPhaseData[MAX_PHASE_COUNT];                  //���ڼ�¼��λ��������

	bool                m_bTargetLockPhaseChannelFlag[MAX_CHANNEL_COUNT];   //���ڼ�¼Ŀ��������λͨ���̵Ʊ�־

	bool                m_bNonTargetLockPhaseEndFlag[MAX_CHANNEL_COUNT];    //���ڼ�¼��Ŀ��������λͨ������������־

	TPhaseLockPara      m_tNewPhaseLockPara;                                //���ڼ�¼�µ���λ����ָ�����

	bool                m_bNeglectChannelBoforePhaseLock[MAX_CHANNEL_COUNT];//���ڼ�¼����λ����֮ǰ������ͨ��(���Ի�ض�)

	bool                m_bOldLockChannelCmdEndFlag;                        //���ڼ�¼�ϵ�ͨ������ָ�������־

	TLockPhaseStage     m_tLockPhaseStage;                                  //���ڼ�¼������λ�ĵ�ɫ�׶�
};

#endif // ifndef LOGICCTLMANUAL_H
