/*=====================================================================
ģ���� ��ͨ�ſ��ƹ���ģ��
�ļ��� ��OpenATCComCtlManager.cpp
����ļ���OpenATCComCtlManager.h
          OpenATCParameter.h
          OpenATCRunStatus.h
ʵ�ֹ��ܣ����������ù��߼�ƽ̨��Ϣ����
���� ��������
��Ȩ ��<Copyright(c) 2019-2020 Suzhou Keda Technology Co.,Ltd. All right reserved.>
-----------------------------------------------------------------------
�޸ļ�¼��
�� ��           �汾     �޸���     �߶���     �޸ļ�¼
2019/9/14       V1.0     ������     ������      ����ģ��
=====================================================================*/

#include "OpenATCComCtlManager.h"

COpenATCComCtlManager * COpenATCComCtlManager::s_pData;

COpenATCComCtlManager::COpenATCComCtlManager()
{
	m_openATCCommWithCfgSWThread	= new COpenATCCommWithCfgSWThread();
	m_openATCCommWithCenterThread	= new COpenATCCommWithCenterThread();
	m_openATCCommWithITS300Thread	= new COpenATCCommWithITS300Thread();
	//m_openATCCommWithCameraListenThread = new COpenATCCommWithCameraListenThread();
	//m_openATCCommWithCfgListenThread    = new COpenATCCommWithCameraListenThread();
	m_openATCCommWithGB20999Thread = new COpenATCCommWithGB20999Thread();
	m_pLogicCtlStatus = NULL;
	m_pLogicCtlParam  = NULL;
	m_pOpenATCLog	  = NULL;

	m_openATCCommWithSimulateThread = new COpenATCCommWithCenterThread();
    m_openATCCommWithDetectorThread = new COpenATCCommWithITS300Thread();

}

COpenATCComCtlManager::~COpenATCComCtlManager()
{

}

/*==================================================================== 
������ ��getInstance 
���� �����ص�����COpenATCComCtlManager��ʵ��ָ�� 
�㷨ʵ�� �� 
����˵�� �� 
����ֵ˵����������COpenATCComCtlManager��ʵ��ָ��
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ������          �������� 
====================================================================*/ 
COpenATCComCtlManager * COpenATCComCtlManager::getInstance()
{
    if (s_pData == NULL)
    {
        s_pData = new COpenATCComCtlManager();
    }

    return s_pData;
}

/*==================================================================== 
������ ��Init 
���� �����ڶ���COpenATCComCtlManager�ĳ�ʼ������ 
�㷨ʵ�� �� 
����˵�� �� pParameter������������ָ��
            pRunStatus������״̬��ָ��
����ֵ˵������
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ������          �������� 
====================================================================*/ 
void COpenATCComCtlManager::Init(COpenATCParameter * pParameter,COpenATCRunStatus * pRunStatus, COpenATCLog * pOpenATCLog, const char * pOpenATCVersion)
{
    if (pParameter != NULL)
    {
        m_pLogicCtlParam = pParameter;
    }

    if (pRunStatus != NULL)
    {
        m_pLogicCtlStatus = pRunStatus;
    }

	if (pOpenATCLog != NULL)
	{
		m_pOpenATCLog = pOpenATCLog;
	}

	//��ȡCommѡ��
	int iComm = 0;
	iComm = m_pLogicCtlStatus->GetCommFlagStatus();
	switch (iComm)
	{
	case 1:
		m_openATCCommWithCfgSWThread->Init(pParameter, pRunStatus, pOpenATCLog, UDP_SERVICE, 0, pOpenATCVersion);
		m_openATCCommWithCfgSWThread->Start();
		delete m_openATCCommWithGB20999Thread;
		break;
	case 2:
		m_openATCCommWithGB20999Thread->Init(pParameter, pRunStatus, pOpenATCLog);
		m_openATCCommWithGB20999Thread->Start();
		delete m_openATCCommWithCfgSWThread;
		break;
	//case 3:
	//	//Ԥ��NTCIP��
	//	break;
	default:
		//Ĭ��25280
		m_openATCCommWithCfgSWThread->Init(pParameter, pRunStatus, pOpenATCLog, UDP_SERVICE, 0, pOpenATCVersion);
		m_openATCCommWithCfgSWThread->Start();
		delete m_openATCCommWithGB20999Thread;
		break;
	}
	m_openATCCommWithCenterThread->Init(pParameter, pRunStatus, pOpenATCLog, COM_WITH_CENTER, pOpenATCVersion);
	m_openATCCommWithCenterThread->Start();

	m_openATCCommWithITS300Thread->Init(pParameter, pRunStatus, pOpenATCLog, COM_WITH_ITS300);
	m_openATCCommWithITS300Thread->Start();

	//m_openATCCommWithCameraListenThread->Init(pParameter, pRunStatus, pOpenATCLog, CAMERA_SERVICE_LISTERN);
	//m_openATCCommWithCameraListenThread->Start();

	//m_openATCCommWithCfgListenThread->Init(pParameter, pRunStatus, pOpenATCLog, CFG_SERVICE_LISTERN);
	//m_openATCCommWithCfgListenThread->Start();

	TAscSimulate tSimulateInfo;
    m_pLogicCtlParam->GetSimulateInfo(tSimulateInfo);

	if (tSimulateInfo.m_nSimulatePort != 0)
	{
		//����ӿ�û������ʱ��ͨ���߳�����Ҫ����
		m_openATCCommWithSimulateThread->Init(pParameter, pRunStatus, m_pOpenATCLog, COM_WITH_SIMULATE, pOpenATCVersion);
		m_openATCCommWithSimulateThread->Start();
	}

	m_openATCCommWithDetectorThread->Init(pParameter, pRunStatus, pOpenATCLog, COM_WITH_SIMULATE);
	m_openATCCommWithDetectorThread->Start();

    m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "COpenATCComCtlManager Init!"); 
}

void COpenATCComCtlManager::Stop()
{
	//��ȡCommѡ��
	int iComm = 0;
	iComm = m_pLogicCtlStatus->GetCommFlagStatus();
	switch (iComm)
	{
	case 1:
		m_openATCCommWithCfgSWThread->Detach();
		m_openATCCommWithCfgSWThread->Join();
		break;
	case 2:
		m_openATCCommWithGB20999Thread->Detach();
		m_openATCCommWithGB20999Thread->Join();
		break;
	//case 3:
	//	//Ԥ��NTCIP��
	//	break;
	default:
		//Ĭ��25280
		m_openATCCommWithCfgSWThread->Detach();
		m_openATCCommWithCfgSWThread->Join();
		break;
	}

	m_openATCCommWithCenterThread->Detach();
	m_openATCCommWithCenterThread->Join();

	m_openATCCommWithITS300Thread->Detach();
	m_openATCCommWithITS300Thread->Join();

	//m_openATCCommWithCameraListenThread->Detach();
	//m_openATCCommWithCameraListenThread->Join();

	//m_openATCCommWithCfgListenThread->Detach();
	//m_openATCCommWithCfgListenThread->Join();

	TAscSimulate tSimulateInfo;
    m_pLogicCtlParam->GetSimulateInfo(tSimulateInfo);

	if (tSimulateInfo.m_nSimulatePort != 0)
	{
		m_openATCCommWithSimulateThread->Detach();
		m_openATCCommWithSimulateThread->Join();
	}

	m_openATCCommWithDetectorThread->Detach();
	m_openATCCommWithDetectorThread->Join();
}

/*=====================================================================
�ó�����һ��ͨ�ſ��ƹ���ģ���ʵ�֣���Ҫ���������ù��߼�ƽ̨��Ϣ���н����������ǶԳ���ķ�����

### ��Ҫ����
1. **����ģʽ**��`COpenATCComCtlManager` ��ʵ���˵���ģʽ��ͨ�� `getInstance` �������ظ����Ψһʵ����
2. **��ʼ������**��`Init` �������ڳ�ʼ��ͨ���̣߳�������������������״̬����־���󣬲�����ͨ��ѡ��������Ӧ���̡߳�
3. **ֹͣ����**��`Stop` ��������ֹͣ����ͨ���̣߳�ȷ���ڹر�ʱ��ȷ�ط���ͼ����̡߳�

### �ؼ����
- **�̹߳���**�������д����˶���̶߳����� `COpenATCCommWithCfgSWThread`��`COpenATCCommWithCenterThread` �ȣ�����ͬ��ͨ������
- **�����ж�**��ͨ�� `GetCommFlagStatus` ������ȡ��ǰ��ͨ��ѡ�񣬲����ݲ�ͬ��ѡ���ʼ����ֹͣ��Ӧ���̡߳�
- **��־��¼**���ڳ�ʼ����ɺ�ʹ�� `LogOneMessage` ������¼��ʼ����Ϣ�����ں����ĵ��Ժͼ�ء�

### ����ṹ
- **���캯��**���ڹ��캯���г�ʼ�������̶߳��󣬲���ָ������Ϊ `NULL`��
- **��������**����������Ϊ�գ�������Ҫ��δ��ʵ����Դ�ͷš�
- **ע��**�������а���������ע�ͣ�˵����ÿ�������Ĺ��ܺ��޸ļ�¼����������ά����

### �Ľ�����
1. **������**���ڴ����̺߳ͳ�ʼ�������У�ȱ�ٴ�������ƣ����������Ӧ���쳣����
2. **��Դ����**��������������Ӧ�ͷŶ�̬������ڴ棬�����ڴ�й©��
3. **����ɶ���**�����Կ��ǽ�һЩ���ӵ��߼���ֳɸ�С�ĺ���������ߴ���Ŀɶ��ԺͿ�ά���ԡ�

������˵���ó���ʵ���˻�����ͨ�ſ��ƹ����ܣ����ڴ��������Դ�����滹�������Ŀռ䡣

====================================================================*/

