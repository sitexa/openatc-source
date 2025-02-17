/*=====================================================================
ģ���� �������ڿ��Ʒ�ʽʵ��ģ��
�ļ��� ��LogicCtlFixedTime.h
����ļ���
ʵ�ֹ��ܣ����ڶ��嶨���ڿ���ģʽʵ�ֽӿ�
���� ��������
��Ȩ ��<Copyright(c) 2019-2020 Suzhou Keda Technology Co.,Ltd. All right reserved.>
-----------------------------------------------------------------------
�޸ļ�¼��
�� ��           �汾     �޸���     �߶���     �޸ļ�¼
2019/9/26       V1.0     ������     ������     ����ģ��
=====================================================================*/

#ifndef LOGICCTLFIXEDTIME_H
#define LOGICCTLFIXEDTIME_H

#include "LogicCtlMode.h"

typedef struct tagPhaseCtlInfo
{
    TPhase m_tPhaseParam;                                               //��λ��Ϣ
    WORD m_wPhaseTime;                                                  //��λ���ű�
    char m_chPhaseMode;                                                 //��λ���ű�ģʽ

    WORD m_wPhaseGreenTime;                                             //��λ�̵�ʱ��(�۳��������Ƶƺͺ��,������)
    char m_chPhaseStage;                                                //��λ��ǰ���н׶�(������)
    WORD m_wPhaseStageRunTime;                                          //��λ��ǰ�׶ε�����ʱ��(������)
    char m_chPhaseStatus;                                               //��λ��ǰ״̬(������:�ȴ�̬,ת��̬,����̬,����̬)

    WORD m_wPedPhaseGreenTime;                                          //��λ�̵�ʱ��(�۳��Ƶƺͺ��,����)
    char m_chPedPhaseStage;                                             //��λ��ǰ���н׶�(����)
    WORD m_wPedPhaseStageRunTime;                                       //��λ��ǰ�׶ε�����ʱ��(����)
    char m_chPedPhaseStatus;                                            //��λ��ǰ״̬(����:ת��̬������̬������̬)

    bool m_bIsPedAskPhase;                                              //�Ƿ������˹���������λ
}TPhaseCtlInfo,*PTPhaseCtlInfo;

typedef struct tagRingParam
{
    int m_nPhaseCount;                                                  //������λ����
    TPhaseCtlInfo m_atPhaseInfo[MAX_SEQUENCE_TABLE_COUNT];              //����λ����
}TRingCtlInfo,*PTRingCtlInfo;

typedef struct tagOverlapCtlInfo
{
    TOverlapTable m_tOverlapParam;                                      //������λ����
    char m_chOverlapStage;                                              //������λ�����н׶�
	char m_chPedOverlapStage;                                           //����������λ�����н׶�                                    
}TOverlapCtlInfo,*PTOverlapCtlInfo;

typedef struct tagFixTimeCtlParam
{
    int m_nRingCount;                                                   //������
    TRingCtlInfo m_atPhaseSeq[MAX_RING_COUNT];                          //�����Ʋ�������
    int m_nCurPhaseIndex[MAX_RING_COUNT];                               //���ڵ�ǰ���е���λ����
    int m_nCurStageIndex;                                               //���ڵ�ǰ���еĽ׶�����
//    int m_bIsChgPhase[MAX_RING_COUNT];                                //�����Ƿ�Ҫ�л���λ
    bool m_bIsChgStage[MAX_RING_COUNT];                                 //��λ���Ƿ��л���ɫ
    bool m_bIsChgPedStage[MAX_RING_COUNT];                              //������λ���Ƿ��л���ɫ
    int m_nOverlapCount;                                                //��˭��λ����
    TOverlapCtlInfo m_atOverlapInfo[MAX_OVERLAP_COUNT];                 //������λ��
//    int m_nChannelCount;                                              //ͨ������
//    TChannel m_atChannelInfo[MAX_CHANNEL_COUNT];                      //ͨ������ 
    int m_nVehDetCount;                                                 //���������
    TVehicleDetector m_atVehDetector[MAX_VEHICLEDETECTOR_COUNT];        //�����������
    int m_nPedDetCount;                                                 //���˼��������
	TPedestrianDetector m_atPedDetector[MAX_PEDESTRIANDETECTOR_COUNT];  //���˼������
    WORD m_wCycleLen;                                                   //���ڳ� 
    WORD m_wPhaseOffset;                                                //��λ��
    WORD m_wCycleRunTime;                                               //��ǰ��������ʱ��
	WORD m_wPatternNumber;                                              //������
}TFixTimeCtlInfo,*PTFixTimeCtlInfo;

#define  SEND_INIT     0
#define  SEND_SRART    1
#define  SEND_END      2

typedef struct tagPhasePulseStatus
{
	int  m_nPhaseIndex;                                                //��λ���
	int  m_nPhaseNum;                                                  //��λID
	bool m_bGreenPulseStatus;                                          //������״̬
	bool m_bRedPulseStatus;                                            //������״̬
	int  m_nGreenPulseSendStatus;                                      //�����巢�͹���״̬
	int  m_nRedPulseSendStatus;                                        //�����巢�͹���״̬
}TPhasePulseStatus,*PTPhasePulseStatus;

/*=====================================================================
���� ��CLogicCtlFixedTime
���� �������ڿ��Ʒ�ʽʵ����
��Ҫ�ӿڣ�Run
          Init
��ע ��
-----------------------------------------------------------------------
�޸ļ�¼��
�� ��          �汾     �޸���     �߶���       �޸ļ�¼
2019/09/14     V1.0     ������     ������       ������
2020/2/26      V1.0     ����Ƽ     ����Ƽ       �������崦��
2020/3/26      V1.0     ����Ƽ     ����Ƽ       ������λ���н׶α�ͻ��ڽ׶��л�״̬�жϸ���
2020/3/26      V1.0     ����Ƽ     ����Ƽ       �ҵ�ÿ����������ǰ�����һ����λ
=====================================================================*/
class CLogicCtlFixedTime : public CLogicCtlMode  
{
public:
	CLogicCtlFixedTime();
	virtual ~CLogicCtlFixedTime();

    //��ʼ�������ڿ��Ʒ�ʽ��Ҫ�Ĳ���
    virtual void Init(COpenATCParameter * pParameter, COpenATCRunStatus * pRunStatus, COpenATCLog * pOpenATCLog, int nPlanNo);
    //�����ڿ��Ʒ�ʽ������
    virtual void Run();

    //��ȡ��ǰʹ�õĿ��Ʋ���������״̬
    virtual void * GetCurCtlParam(); 

    //��������״̬
    virtual void GetPhaseRunStatus(TPhaseRunStatus & tRunStatus);

    //������ʾ����ʾ��Ϣ״̬
    virtual void GetLedScreenShowInfo(TLedScreenShowInfo & tLedScreenShowInfo);

	//�жϵ�ǰͨ���ĵ�ɫ�Ƿ�����ɫ
    virtual bool IsChannelGreen(BYTE byChannelType, int nDirectionIndex, int nChannelLockStatus[]);

public:
	bool				m_bGreenTimeExceedMax1Time[MAX_PHASE_COUNT];							//������λ�̵�ʱ���Ƿ�ﵽ�����1
	int					m_nGreenTimeIsMax1Time[MAX_PHASE_COUNT];								//������λ�̵�ʱ���Ƿ��������δﵽ�����1
	int					m_nGreenTimeNotExceedMax1Time[MAX_PHASE_COUNT];							//������λ�̵�ʱ���Ƿ���������δ�ﵽ�����1

protected:
    //��ȡʱ�β���
    virtual void InitByTimeSeg(BYTE & byPlanID);

    //��ȡָ����������
    virtual void InitByPlan(BYTE byPlanID);

    //��ʼ��������λ����
    virtual void InitOverlapParam();

    //��ʼ�����������
    virtual void InitDetectorParam();

    //��ʼ�����˼��������
    virtual void InitPedDetectorParam();

    //���ڻ�������λ������״̬����
    virtual void OnePhaseRun(int nRingIndex);

    //����������λ������״̬����
    virtual void OnePedPhaseRun(int nRingIndex);

    //������λ�л�״̬�жϸ���
    virtual void PhaseChg(int nRingIndex);

    //�໷�������н����жϸ���
    virtual void CycleChg();

    //���ڽ������ÿ���״̬
    virtual void ReSetCtlStatus();

    //���ݵ�ǰ�Ļ�������λ���н׶Σ��ж���һ����0�����н׶κͽ׶�ʱ��
    virtual char GetNextPhaseStageInfo(char chCurStage,const PTPhaseCtlInfo pPhaseInfo,WORD & nStageTime);

    //���ݵ�ǰ��������λ���н׶Σ��ж���һ����0�����н׶κͽ׶�ʱ��
    virtual char GetPedNextPhaseStageInfo(char chCurStage,const PTPhaseCtlInfo pPhaseInfo,WORD & nStageTime);

    //��ȡ��λʣ������ʱ��
    virtual int  GetPhaseRemainTime(TPhaseCtlInfo * pPhaseCtlInfo);

    virtual void SetGreenLampPulse(int nPhaseType, BYTE byPhaseNumber, bool bState);

    virtual void SetRedLampPulse(int nPhaseType, BYTE byNextPhaseNumber, bool bState);

	virtual void SetOverLapGreenPulse(int nRingIndex, int nIndex, bool bManual, int nNextStage, bool bState);

	virtual void SetOverLapRedPulse(int nRingIndex, int nIndex, bool bManual, int nNextStage, bool bState);

	virtual void SetOverLapPulse(bool bManual, int nNextStageIndex);

	virtual void GetGreenFalshCount(int nPhaseType, TPhaseCtlInfo * pPhaseCtlInfo, int nSecond);

    virtual int  GetPhaseCycleRemainTime(int nRingIndex, TPhaseCtlInfo * pPhaseCtlInfo, TPhaseLampClrRunCounter tRunCounter);

    virtual int  GetPedPhaseCycleRemainTime(int nRingIndex, TPhaseCtlInfo * pPhaseCtlInfo, TPhaseLampClrRunCounter tRunCounter);

    //������λ�Ų��Ҹ���λ���ڵĻ��źͻ���˳���
    bool GetPhaseIndexByPhaseNumber(int & nRingIndex,int & nPhaseIndex,int nPhaseNum);

    //����һ��Ring����λ��ɫ
    void SetLampClrByRing(int nRingIndex, int nPhaseIndex, char chPhaseStage, char chPedPhaseStage);

    //������λ��Ӧ��ͨ����״̬
    void SetChannelStatus(int nPhaseNum, int nPhaseSrc, char chPhaseStage, char chPedPhaseStage);

    //���ݽ׶����ø�����λ��ɫ
    void SetOverlapPhaseLampClr(int nNextStageIndex);

    //����ʱ�����ø�����λ��ɫ
    void SetOverlapPhaseLampClr();

    //��ǰ��λ�Ƿ��Ǹ�����λ��Ŀ��λ
    bool IsPhaseNumInOverlap(int nPhaseNum,int nOverlapIndex);
    //��ȡָ����λ��ǰ��״̬ 
    char GetPhaseStatus(int nPhaseNum, bool bPedPhase);

    //����ͨ��������ʼ��ͨ��״̬
    void RetsetAllChannelStatus();

    //�ж���λ��Ӧ�ĳ������Ƿ�ȫ������
    bool ProcPhaseDetStatus(int nRingIndex,int nPhaseIndex); 

    //��ȡ��λ���н׶α�
    void GetRunStageTable();

    //���ڽ׶��л�״̬�жϸ���
    bool StageChg(int nRingIndex);

    //�ҵ�ÿ����������ǰ�����һ����λ
    void GetLastPhaseBeforeBarrier();

    //��������λ��������
    void SendOverlapPhasePulse(int nRingIndex, int nCurPhaseNum, int nPhaseNum, bool bRedPulse, BYTE byOverlapNum[], bool bSendPulse[], bool bManual, int nNextStage);

    //����ͨ����Ӧ����λ����ͨ����Ӧ�����ű�ģʽ
    void SetChannelSplitMode();

	//��ȡ������λ��״̬ 
    char GetOverlapType(char chOverlapStatus);

    //����ͨ����Ӧ����λ����ͨ����Ӧ�����κͽ�ֹ״̬
    void SetChannelShieldAndProhibitStatus();

    //��λ��ֹ��������������ó���
    void SetLampByShieldAndProhibitStatus(int nRingNum, int nPhaseCount, int nChannelCount);

	bool IsNeglectPhase(int nPhaseID);

    //�жϣ���Ӧ����λ�Ƿ񲢷�
    bool IsConcurrencyPhase(int nRingIndex, int nDstRingIndex);

    //����m_bySplitPhaseMode��m_nSplitPhaseTime������������
    void SetSeqByStageInfo();

    //�ж�������λ�Ƿ񲢷�
    void InitPhaseConcurrencyTable();

    TFixTimeCtlInfo m_tFixTimeCtlInfo;												//���㶨���ڿ�����Ҫ�����������Ϳ���״̬��Ϣ

    bool m_bLastPhaseBeforeBarrier[MAX_RING_COUNT][MAX_SEQUENCE_TABLE_COUNT];		//ÿ��������λ�ǲ�������ǰ�����һ����λ

    unsigned long   m_nChannelDurationTime[MAX_CHANNEL_COUNT];						//�ֶ�����ʱÿ��ͨ����ɫ����ʱ��

    int             m_nSplitTime[MAX_RING_COUNT][MAX_SEQUENCE_TABLE_COUNT];			//���ڼ�¼����Ӧ���޵��º����˹���ģʽ�µ����ű�ʱ��

    short           m_nRealOffset;													//ʵ����λ��

    TSplit          m_atSplitInfo[MAX_PHASE_COUNT];									//���űȱ�

	TPhasePulseStatus m_tPhasePulseStatus[MAX_RING_COUNT];                          //ÿ�����Ļ�������λ���������־

	BYTE            m_byPlanID;                                                     //������

    BYTE            m_bySplitPhaseMode[MAX_RING_COUNT][MAX_PHASE_COUNT];            //��λ�Ƿ���� 0��������λ��1����λ���ԣ�2����λ���Բ���

    int             m_nSplitPhaseTime[MAX_RING_COUNT][MAX_PHASE_COUNT];             //����λ���Բ���ʱ�����Ե���λʱ��

    int m_nPhaseConcurInfo[MAX_PHASE_COUNT][MAX_PHASE_COUNT];			            //��λ������
};

#endif // !ifndef LOGICCTLFIXEDTIME_H
