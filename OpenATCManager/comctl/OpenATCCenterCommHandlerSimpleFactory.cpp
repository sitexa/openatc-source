/*====================================================================
ģ���� ��ͨ�Ź���ģ��
�ļ��� ��OpenATCCenterCommHandlerSimpleFactory.cpp
����ļ���OpenATCCenterCommHandlerSimpleFactory.h
ʵ�ֹ��ܣ�����ͨ�Ź���������ͨ�Ŷ���
���� ������Ƽ
��Ȩ ��<Copyright(C) 2019-2020 Suzhou Keda Technology Co., Ltd. All rights reserved.>
---------------------------------------------------------------------------------------------------------------------
�޸ļ�¼��
�� ��           �汾    �޸���             �߶���             �޸ļ�¼
2019/09/06      V1.0    ����Ƽ             ����Ƽ             ����ģ��
====================================================================*/

#include "OpenATCCenterCommHandlerSimpleFactory.h"
#include "OpenATCComWithControlCenterImpl.h"

COpenATCCenterCommHandlerBase* COpenATCCenterCommHandlerSimpleFactory::Create(COpenATCParameter *pParameter, COpenATCRunStatus *pRunStatus, COpenATCLog * pOpenATCLog, const char * pOpenATCVersion)
{
    COpenATCCenterCommHandlerBase* centerHandler;

	centerHandler = new COpenATCComWithControlCenterImpl(pParameter, pRunStatus, pOpenATCLog, pOpenATCVersion);

    return centerHandler;
	
}

