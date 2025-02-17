/*=====================================================================
ģ���� ��ͨ�Ź���ģ��
�ļ��� ��OpenATCCenterCommHandlerSimpleFactory.h
����ļ���OpenATCCenterCommHandlerBase.h
ʵ�ֹ��ܣ�����ͨ�Ź���������ͨ�Ŷ���
���� ������Ƽ
��Ȩ ��<Copyright(c) 2019-2020 Suzhou Keda Technology Co.,Ltd. All right reserved.>
--------------------------------------------------------------------------------------------------------------------
�޸ļ�¼��
�� ��           �汾    �޸���             �߶���             �޸ļ�¼
2019/09/06      V1.0    ����Ƽ��           ����Ƽ             ����ģ��
====================================================================*/

#ifndef OPENATCCENTERCOMMHANDLERSIMPLEFACTORY__H
#define OPENATCCENTERCOMMHANDLERSIMPLEFACTORY__H

#include "OpenATCCenterCommHandlerBase.h"

/*=====================================================================
���� ��COpenATCCenterCommHandlerSimpleFactory
���� ������ͨ�Ź���������ͨ�Ŷ���
��Ҫ�ӿڣ�COpenATCCenterCommHandlerBase Create������ͨ�Ŷ���
��ע ��
--------------------------------------------------------------------------------------------------------------------
�޸ļ�¼��
�� ��           �汾    �޸���             �߶���             �޸ļ�¼
2019/09/06      V1.0    ����Ƽ             ����Ƽ             ������
====================================================================*/
class COpenATCCenterCommHandlerSimpleFactory
{
public:

	/****************************************************
	��������Create
    ���ܣ�����ͨ�Ŷ���
	�㷨ʵ��:
    ����˵�� ����
    ����ֵ˵��������ͨ�Ŷ���
    --------------------------------------------------------------------------------------------------------------------
	�޸ļ�¼��
	�� ��           �汾    �޸���             �߶���             �޸ļ�¼
	2019/09/06      V1.0    ����Ƽ             ����Ƽ             ����
	====================================================================*/
    static COpenATCCenterCommHandlerBase* Create(COpenATCParameter *pParameter, COpenATCRunStatus *pRunStatus,COpenATCLog * pOpenATCLog, const char * pOpenATCVersion);
};

#endif// !ifndef OPENATCCENTERCOMMHANDLERSIMPLEFACTORY__H



