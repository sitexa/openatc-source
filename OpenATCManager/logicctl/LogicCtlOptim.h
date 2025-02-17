/*=====================================================================
ģ���� �������Ż����Ʒ�ʽʵ��ģ��
�ļ��� ��LogicCtlActuate.h
����ļ���
ʵ�ֹ��ܣ����ڶ��嵥���Ż�����ģʽʵ�ֽӿ�
���� ��������
��Ȩ ��<Copyright(c) 2019-2020 Suzhou Keda Technology Co.,Ltd. All right reserved.>
-----------------------------------------------------------------------
�޸ļ�¼��
�� ��           �汾     �޸���     �߶���     �޸ļ�¼
2019/9/26       V1.0     ������     ������     ����ģ��
=====================================================================*/

#ifndef LOGICCTLOPTIM_H
#define LOGICCTLOPTIM_H

#include "LogicCtlFixedTime.h"

typedef struct tagOnePhaseOptimInfo
{
    int m_nDetectorCount;                                   //��λ���������
    BYTE m_byDetectorID[MAX_VEHICLEDETECTOR_COUNT];         //��������
    BYTE m_byDetectorStatus[MAX_VEHICLEDETECTOR_COUNT];     //�����״̬,0Ϊ����,1Ϊ����
    int m_anValidPassTime[MAX_VEHICLEDETECTOR_COUNT];      //���������Чͨ��ʱ�䣬����Ϊ��λ
    int m_nTotalPassTime;                                 //��λ��ͨ��ʱ��,����Ϊ��λ
}TOnePhaseOptimInfo,*PTOnePhaseOptimInfo;

typedef struct tagOneRingOptimInfo
{
    int m_nPhaseCount;                                                      //��λ����
    TOnePhaseOptimInfo m_atPhaseOptimInfo[MAX_SEQUENCE_TABLE_COUNT];        //������λ�Ż���Ϣ
}TOneRingOptimInfo,*PTOneRingOptimInfo;

typedef struct tagOptimInfo
{
    int m_nRingCount;                                                       //������
    TOneRingOptimInfo m_atRingOptimInfo[MAX_RING_COUNT];                    //���Ż���Ϣ
}TOptimInfo,*PTOptimInfo;

class CLogicCtlOptim : public CLogicCtlFixedTime
{
public:
	CLogicCtlOptim();
	virtual ~CLogicCtlOptim();

    //��ʼ����Ӧ���Ʒ�ʽ��Ҫ�Ĳ���
    virtual void Init(COpenATCParameter * pParameter, COpenATCRunStatus * pRunStatus, COpenATCLog * pOpenATCLog, int nPlanNo);

protected:
    //���ڻ�������λ������״̬����
    virtual void OnePhaseRun(int nRingIndex);

    //����������λ������״̬����
    virtual void OnePedPhaseRun(int nRingIndex);

    //���ڽ������ÿ���״̬
    virtual void ReSetCtlStatus();
private:
    //���ݵ�ǰ������Ϣ������һ���ڵ����ű�
    void CalcPhaseTime(); 

    //��ʼ���Ż���ز���
    void InitOptimParam();

    //���ó�ʼ����λ����
    void ResetPhaseOptimInfo(int nRingIndex,int nPhaseIndex);

    //������λ�Ż�����
    void ProcPhaseOptimInfo(int nRingIndex,int nPhaseIndex);

    //������λ���Ͷ�
    bool CalcPhaseDSInfo(int nRingIndex,int nPhaseIndex,int & nDS);

    bool m_bCalcSplitTimeFlag;          //���űȼ����־

    BYTE m_byDetectorTmpStatus[MAX_VEHICLEDETECTOR_COUNT];                      //��ʱ�洢�����״̬,���ڼ�����Чͨ��ʱ��
    int m_anDetectorTmpCounter[C_N_MAXDETBOARD_NUM * C_N_MAXDETINPUT_NUM];      //��ʱ����������ʱ�������,���ڼ�����Чͨ��ʱ��

    TOptimInfo m_tOptimInfo;            //�Ż���Ϣ
};

#endif // !ifndef LOGICCTLOPTIM_H





















