/*=====================================================================
ģ���� ��ϵͳ����������ģ��
�ļ��� ��OpenATCMainCtlManager.h
����ļ���OpenATCParameter.h OpenATCRunStatus.h
ʵ�ֹ��ܣ�����ģ����������࣬���ڹ���ϵͳ�Ĳ���������״̬��
���� ��������
��Ȩ ��<Copyright(c) 2019-2020 Suzhou Keda Technology Co.,Ltd. All right reserved.>
----------------------------------------------------------------------
�޸ļ�¼��
�� ��           �汾     �޸���     �߶���     �޸ļ�¼
2019/9/14       V1.0     ������     �� ��      ����ģ��
=====================================================================*/

#ifndef OPENATCMAINCTLMANAGER_H
#define OPENATCMAINCTLMANAGER_H

#include "../Include/OpenATCParameter.h"
#include "../Include/OpenATCRunStatus.h"
#include "../Include/OpenATCLog.h"

const int C_N_FEEDDOG_INTERVAL = 5;
const int C_N_FEEDDOG_TIMEOUT = 20;

const int C_N_GPSADJUSTTIME_INTERVAL = 2;

/*=====================================================================
���� ��COpenATCMainCtlManager
���� ��������������࣬���ڲ����ͱ�����ʼ����ͨѶ�����ʼ������ʱ����ʼ����ȫ�ֲ����ͱ����Ĺ���
��Ҫ�ӿڣ�
��ע ��
-----------------------------------------------------------------------
�޸ļ�¼��
�� ��          �汾     �޸���     �߶���       �޸ļ�¼
2019/09/14     V1.0     ������     ����         ������
=====================================================================*/

#ifdef _WIN32
    #ifdef DLL_FILE
    class _declspec(dllexport) COpenATCMainCtlManager
    #else
    class _declspec(dllimport) COpenATCMainCtlManager
    #endif
#else
    class COpenATCMainCtlManager
#endif
{
public:
    //�ඨ��Ϊ����
    static COpenATCMainCtlManager * getInstance();

    //�����ʼ��
    void Init(COpenATCParameter * pParameter, COpenATCRunStatus * pRunStatus, COpenATCLog * pOpenATCLog);

    //����ֹͣ
    void Stop();  

    //ģ��������
    void Work();
	
    //��ȡ״̬��ָ��
	COpenATCRunStatus * GetOpenATCRunStatus();

	//��ȡ��־��ָ��
	COpenATCLog * GetOpenATCLog();

    //�����˳�ͣ��״̬
    void SetStopDogFlag(bool bFlag);

private:
	COpenATCMainCtlManager();
	~COpenATCMainCtlManager();

    //����linuxϵͳ���ź�
    void ProcSystemSignal();

    //��ʼ�����Ź�
    bool InitWatchDog();

    //ι��
    bool FeedWatchDog();

    //ͣ��
    bool StopWatchDog();

	//����ϵͳʱ��
	bool SetSysTime(const long nTime);

    static COpenATCMainCtlManager * s_pData;

	COpenATCRunStatus * m_pLogicCtlStatus;              //����״̬��ָ��
    bool m_bStopDogFlag;                                //�˳�ʱ��ͣ����־

	COpenATCLog       * m_pOpenATCLog;                  //��־��ָ��
};

#endif // !ifndef OPENATCMAINCTLMANAGER_H
