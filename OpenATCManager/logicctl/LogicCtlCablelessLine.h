/*=====================================================================
ģ���� ���޵����߿ؿ��Ʒ�ʽʵ��ģ��
�ļ��� ��LogicCtlCablelessLine.h
����ļ���
ʵ�ֹ��ܣ����ڶ����޵����߿ؿ���ģʽʵ�ֽӿ�
���� ��������
��Ȩ ��<Copyright(c) 2019-2020 Suzhou Keda Technology Co.,Ltd. All right reserved.>
-----------------------------------------------------------------------
�޸ļ�¼��
�� ��           �汾     �޸���     �߶���     �޸ļ�¼
2019/9/26       V1.0     ������     ������     ����ģ��
2020/4/26       V1.0     ����Ƽ     ����Ƽ     �޸ĵ���ϵ��
=====================================================================*/

#ifndef LOGICCTLCABLELESSLINE_H
#define LOGICCTLCABLELESSLINE_H

#include "LogicCtlFixedTime.h"

const int C_N_SECONDS_PERHOUR = 3600;
const int C_N_SECONDS_PERMIN = 60;
const float C_F_ADJ_KEY = (float)0.3;

class CLogicCtlCablelessLine : public CLogicCtlFixedTime  
{
public:
	CLogicCtlCablelessLine();
	virtual ~CLogicCtlCablelessLine();

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
    //����ʵ����λ��,������λ�̵�ʱ��
    short CalcRealOffset();

    //������λ��,�̵�ʱ�����С�̣������������ʵ�ʵ�������
    short CalcRealAdjVal(short nOffset,WORD wGreenTime,WORD wMinGreen,WORD wMaxGreen);

	//����������λ���̵Ʊ仯����
	void  CalculateAllPhaseGreenChgLen();

	//����������λ���̵�ʱ��
	void  ResetAllPhaseGreenTime();

	//������λ���ű�
	int  SetPhaseSplitTime(int nRingIndex, int nPhaseIndex, WORD wSplitTime, bool bFlag);

	//����ԭ�������õ���λ���ű�
	void  SetPhaseSplitTimeFromParam();

	int   FloatToInt(float f);

    //short m_nRealOffset;														//ʵ����λ��

    //short m_nGreenStageChgLen;												//��ǰ�̵ƽ׶οɵ����ĳ���

    bool m_bCalcOffsetFlag;														//������λ���־

	int    m_nBarrierPhaseDiffArrCount;											//��������

	short  m_nAllGreenStageChgLen[MAX_RING_COUNT][MAX_SEQUENCE_TABLE_COUNT];	//ÿ����������ǰ���̵Ƶ������Ⱥ�
	
	short  m_nGreenStageChgLen[MAX_RING_COUNT][MAX_SEQUENCE_TABLE_COUNT];		//ÿ������ÿ����λ���̵Ƶ�������
	
	short  m_nSplitTimeBeforeBarrier[MAX_RING_COUNT][MAX_SEQUENCE_TABLE_COUNT];	//ÿ����������ǰ�����űȺ�

	bool   m_bChangeToYellow[MAX_RING_COUNT];									//����ͨ��������Ҫ��������ͨ��������ͬʱ����
};

#endif // ifndef LOGICCTLCABLELESSLINE_H
