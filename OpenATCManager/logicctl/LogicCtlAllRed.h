/*=====================================================================
ģ���� ��ȫ����Ʒ�ʽʵ��ģ��
�ļ��� ��LogicCtlAllRed.h
����ļ���
ʵ�ֹ��ܣ����ڶ���ȫ�����ģʽʵ�ֽӿ�
���� ��������
��Ȩ ��<Copyright(c) 2019-2020 Suzhou Keda Technology Co.,Ltd. All right reserved.>
-----------------------------------------------------------------------
�޸ļ�¼��
�� ��           �汾     �޸���     �߶���     �޸ļ�¼
2019/9/26       V1.0     ������     ������     ����ģ��
=====================================================================*/

#ifndef LOGICCTLALLRED_H
#define LOGICCTLALLRED_H

#include "LogicCtlMode.h"

class CLogicCtlAllRed : public CLogicCtlMode 
{
public:
	CLogicCtlAllRed();
	virtual ~CLogicCtlAllRed();

    //��ʼ��ȫ����Ʒ�ʽ��Ҫ�Ĳ���
    virtual void Init(COpenATCParameter * pParameter, COpenATCRunStatus * pRunStatus, COpenATCLog * pOpenATCLog, int nPlanNo);

    //ȫ����Ʒ�ʽ������
    virtual void Run();


};

#endif // !ifndef LOGICCTLALLRED_H
