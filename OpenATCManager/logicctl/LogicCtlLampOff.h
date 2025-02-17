/*=====================================================================
ģ���� ���صƿ��Ʒ�ʽʵ��ģ��
�ļ��� ��LogicCtlLampOff.h
����ļ���
ʵ�ֹ��ܣ����ڶ���صƿ���ģʽʵ�ֽӿ�
���� ��������
��Ȩ ��<Copyright(c) 2019-2020 Suzhou Keda Technology Co.,Ltd. All right reserved.>
-----------------------------------------------------------------------
�޸ļ�¼��
�� ��           �汾     �޸���     �߶���     �޸ļ�¼
2019/9/26       V1.0     ������     ������     ����ģ��
=====================================================================*/

#ifndef LOGICCTLLAMPOFF_H
#define LOGICCTLLAMPOFF_H

#include "LogicCtlMode.h"

/*=====================================================================
���� ��CLogicCtlLampOff
���� ���صƿ��Ʒ�ʽʵ����
��Ҫ�ӿڣ�Run
��ע ��
-----------------------------------------------------------------------
�޸ļ�¼��
�� ��          �汾     �޸���     �߶���       �޸ļ�¼
2019/09/14     V1.0     ������     ������       ������
=====================================================================*/
class CLogicCtlLampOff : public CLogicCtlMode 
{
public:
	CLogicCtlLampOff();
	virtual ~CLogicCtlLampOff();

    //��ʼ�������ڿ��Ʒ�ʽ��Ҫ�Ĳ���
    virtual void Init(COpenATCParameter * pParameter, COpenATCRunStatus * pRunStatus, COpenATCLog * pOpenATCLog, int nPlanNo);

    //�صƿ��Ʒ�ʽ��������
    virtual void Run();

protected:

};

#endif // !ifndef LOGICCTLLAMPOFF_H
