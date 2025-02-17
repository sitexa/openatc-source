/*=====================================================================
ģ���� ����Ӧʽ���˹��ֿ��Ʒ�ʽʵ��ģ��
�ļ��� ��LogicCtlActuatePedCrossStreet.h
����ļ���
ʵ�ֹ��ܣ����ڶ����Ӧʽ���˹��ֿ���ģʽʵ�ֽӿ�
���� ���º���
��Ȩ ��<Copyright(c) 2019-2021 Suzhou Keda Technology Co.,Ltd. All right reserved.>
-----------------------------------------------------------------------
�޸ļ�¼��
�� ��           �汾     �޸���     �߶���     �޸ļ�¼
2020/11/24      V1.0     �º���      	       ����ģ��
=====================================================================*/

#ifndef LOGICCTLACTUATEPEDCROSSSTREET_H
#define LOGICCTLACTUATEPEDCROSSSTREET_H

#include "LogicCtlFixedTime.h"

class CLogicCtlActuatePedCrossStreet : public CLogicCtlFixedTime  
{
public:
	CLogicCtlActuatePedCrossStreet();
	virtual ~CLogicCtlActuatePedCrossStreet();

    //��ʼ�����˹��ֿ��Ʒ�ʽ��Ҫ�Ĳ���
    virtual void Init(COpenATCParameter * pParameter, COpenATCRunStatus * pRunStatus, COpenATCLog * pOpenATCLog,  int nPlanNo);

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
	TVehicleQueueUpInfo m_tVehicleQueueUpInfo[MAX_VEHICLEDETECTOR_COUNT];//�����Ŷ���Ϣ

	TPedDetectInfo m_tPedDetectInfo[MAX_VEHICLEDETECTOR_COUNT];//���˼����Ϣ

	enum
	{
		PED_DETECTOR_WAIT_AREA = 1,
		PED_DETECTOR_CROSS_AREA = 2,
	};

	int m_nPedDetecetorType[MAX_VEHICLEDETECTOR_COUNT];
	int m_nPhaseThresholdVehicle[MAX_VEHICLEDETECTOR_COUNT];
	int m_nPhaseThresholdPedWait[MAX_VEHICLEDETECTOR_COUNT];
};

#endif // ifndef LOGICCTLACTUATEPEDCROSSSTREET_H
