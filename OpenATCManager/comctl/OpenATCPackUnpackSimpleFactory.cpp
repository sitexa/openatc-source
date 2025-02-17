/*=====================================================================
ģ���� �����ݽ����������ʽ�����๤��
�ļ��� ��OpenATCPackUnpackSimpleFactory.cpp
����ļ���OpenATCPackUnpackSimpleFactory.h
          OpenATCPackUnpackBase.h
ʵ�ֹ��ܣ����ݿ��Ʒ�ʽ�����������ݽ����������ʽʵ�ֶ���
���� ��������
��Ȩ ��<Copyright(c) 2019-2020 Suzhou Keda Technology Co.,Ltd. All right reserved.>
-----------------------------------------------------------------------
�޸ļ�¼��
�� ��           �汾     �޸���     �߶���     �޸ļ�¼
2019/9/14       V1.0     ������     ������     ����ģ��
=====================================================================*/

#include "OpenATCPackUnpackSimpleFactory.h"
#include "OpenATCPackUnpackBase.h"
#include "OpenATCDataPackUnpack.h"
#include "OpenATCITS300DataPackUnpack.h"
#include "OpenATCCameraDataPackUnpack.h"
#include "OpenATCGB20999DataPackUnpack.h"
#include <stdio.h>

COpenATCPackUnpackSimpleFactory::COpenATCPackUnpackSimpleFactory()
{

}

COpenATCPackUnpackSimpleFactory::~COpenATCPackUnpackSimpleFactory()
{

}

/*==================================================================== 
������ ��Create 
���� �����ؾ�����Ʒ�ʽ����ָ��
�㷨ʵ�� �� 
����˵�� ��nCtlMode�����Ʒ�ʽ 
����ֵ˵����nCtlMode��ʾ�Ŀ��Ʒ�ʽʵ�ֶ����ָ��
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ������          �������� 
====================================================================*/ 
COpenATCPackUnpackBase * COpenATCPackUnpackSimpleFactory::Create(int nCtlMode)
{
    COpenATCPackUnpackBase * pBase = NULL;
    if (nCtlMode == PACK_UNPACK_MODE_ITS300) 
    {
        pBase = new COpenATCITS300DataPackUnpack();
    }
	else if (nCtlMode == PACK_UNPACK_MODE_CAMERA) 
    {
        pBase = new COpenATCCameraDataPackUnpack();
    }
    else if (nCtlMode == PACK_UNPACK_MODE_GB20999)
    {
        pBase = new COpenATCGB20999DataPackUnpack();
    }
    else
    {
        pBase = new COpenATCDataPackUnpack();
    }
    
    return pBase; 
}
