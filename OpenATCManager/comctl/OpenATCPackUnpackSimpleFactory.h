/*=====================================================================
ģ���� �����ݽ����������ʽ�����๤��
�ļ��� ��OpenATCPackUnpackSimpleFactory.h
����ļ���
ʵ�ֹ��ܣ�ʵ�־������ݽ����������ʽ�����ӿ�
���� ��������
��Ȩ ��<Copyright(c) 2019-2020 Suzhou Keda Technology Co.,Ltd. All right reserved.>
------------------------------------------------------------------------
�޸ļ�¼��
�� ��           �汾     �޸���     �߶���     �޸ļ�¼
2019/9/25       V1.0     ������     ������     ����ģ��
=====================================================================*/

#ifndef LOGICCTLMODESIMPLEFACTORY_H
#define LOGICCTLMODESIMPLEFACTORY_H

class COpenATCPackUnpackBase;

/*=====================================================================
���� ��COpenATCPackUnpackSimpleFactory
���� �����ݽ���������๤�����������ݽ����������������������ݽ��������ʵ������
��Ҫ�ӿڣ�Create
��ע ��
-----------------------------------------------------------------------
�޸ļ�¼��
�� ��          �汾     �޸���     �߶���       �޸ļ�¼
2019/09/14     V1.0     ������     ������       ������
=====================================================================*/
class COpenATCPackUnpackSimpleFactory  
{
public:
	COpenATCPackUnpackSimpleFactory();
	virtual ~COpenATCPackUnpackSimpleFactory();

    static COpenATCPackUnpackBase * Create(int nCtlMode);

};

#endif // ifndef LOGICCTLMODESIMPLEFACTORY_H
