/*=====================================================================
ģ���� ��ͨ�������Ʒ�ʽʵ��ģ��
�ļ��� ��LogicCtlChannelCheck.h
����ļ���
ʵ�ֹ��ܣ�����ͨ�������Ʒ�ʽʵ�ֽӿ�
���� ��������
��Ȩ ��<Copyright(c) 2019-2020 Suzhou Keda Technology Co.,Ltd. All right reserved.>
-----------------------------------------------------------------------
�޸ļ�¼��
�� ��           �汾     �޸���     �߶���     �޸ļ�¼
2019/9/26       V1.0     ������     �� ��      ����ģ��
=====================================================================*/

#ifndef LOGICCHANNELCHECK_H
#define LOGICCHANNELCHECK_H

#include "LogicCtlFixedTime.h"

class CLogicCtlChannelCheck : public CLogicCtlMode
{
public:
	CLogicCtlChannelCheck();
	virtual ~CLogicCtlChannelCheck();

	//��ʼ��ͨ�������Ʒ�ʽ��Ҫ�Ĳ�����״̬,��Ҫ���ڳ�ʼ��״̬,������֮ǰ�Ŀ��Ʒ�ʽ�̳ж���
	virtual void Init(COpenATCParameter * pParameter, COpenATCRunStatus * pRunStatus, COpenATCLog * pOpenATCLog, int nPlanNo);

	//ͨ�������Ʒ�ʽ������
	virtual void Run();

	
protected:



	
};

#endif // ifndef LOGICCHANNELCHECK_H
