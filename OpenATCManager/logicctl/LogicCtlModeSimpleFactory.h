/*=====================================================================
ģ���� �����Ʒ�ʽ��������
�ļ��� ��LogicCtlModeSimpleFactory.h
����ļ���
ʵ�ֹ��ܣ�ʵ�־������ģʽ�����ӿ�
���� ��������
��Ȩ ��<Copyright(c) 2019-2020 Suzhou Keda Technology Co.,Ltd. All right reserved.>
------------------------------------------------------------------------
�޸ļ�¼��
�� ��           �汾     �޸���     �߶���     �޸ļ�¼
2019/9/25       V1.0     ������     ������     ����ģ��
=====================================================================*/

#ifndef LOGICCTLMODESIMPLEFACTORY_H
#define LOGICCTLMODESIMPLEFACTORY_H

class CLogicCtlMode;

/*=====================================================================
���� ��CLogicCtlModeSimpleFactory
���� �����Ʒ�ʽ�๤�������ݿ��Ʒ�ʽ����������Ŀ��Ʒ�ʽʵ������
��Ҫ�ӿڣ�Create
��ע ��
-----------------------------------------------------------------------
�޸ļ�¼��
�� ��          �汾     �޸���     �߶���       �޸ļ�¼
2019/09/14     V1.0     ������     ������       ������
=====================================================================*/
class CLogicCtlModeSimpleFactory  
{
public:
	CLogicCtlModeSimpleFactory();
	virtual ~CLogicCtlModeSimpleFactory();

    static CLogicCtlMode * Create(int nCtlMode);

};

#endif // ifndef LOGICCTLMODESIMPLEFACTORY_H
