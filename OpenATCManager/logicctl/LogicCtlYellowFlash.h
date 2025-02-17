/*=====================================================================
ģ���� ���������Ʒ�ʽʵ��ģ��
�ļ��� ��LogicCtlYellowFlash.h
����ļ���
ʵ�ֹ��ܣ����ڶ���صƿ���ģʽʵ�ֽӿ�
���� ��������
��Ȩ ��<Copyright(c) 2019-2020 Suzhou Keda Technology Co.,Ltd. All right reserved.>
-----------------------------------------------------------------------
�޸ļ�¼��
�� ��           �汾     �޸���     �߶���     �޸ļ�¼
2019/9/26       V1.0     ������     ������     ����ģ��
=====================================================================*/

#ifndef LOGICCTLYELLOWFLASH_H
#define LOGICCTLYELLOWFLASH_H

#include "LogicCtlMode.h"

/*=====================================================================
���� ��CLogicCtlYellowFlash
���� ���������Ʒ�ʽʵ����
��Ҫ�ӿڣ�Run
��ע ��
-----------------------------------------------------------------------
�޸ļ�¼��
�� ��          �汾     �޸���     �߶���       �޸ļ�¼
2019/09/14     V1.0     ������     ����         ������
=====================================================================*/
class CLogicCtlYellowFlash : public CLogicCtlMode  
{
public:
	CLogicCtlYellowFlash();
	virtual ~CLogicCtlYellowFlash();

    //��ʼ���������Ʒ�ʽ��Ҫ�Ĳ���
    virtual void Init(COpenATCParameter * pParameter, COpenATCRunStatus * pRunStatus, COpenATCLog * pOpenATCLog, int nPlanNo);

    //�������Ʒ�ʽ������
    virtual void Run();
protected:
};

#endif // !ifndef LOGICCTLYELLOWFLASH_H
