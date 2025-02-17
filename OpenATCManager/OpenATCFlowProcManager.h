/*=====================================================================
ģ���� ����������ģ��
�ļ��� ��OpenATCFlowProcManager.h
����ļ���OpenATCParameter.h OpenATCRunStatus.h
ʵ�ֹ��ܣ���������ģ������࣬��������ͳ�ƺ�ʵʱ�������ɡ�
���� ��������
��Ȩ ��<Copyright(c) 2019-2020 Suzhou Keda Technology Co.,Ltd. All right reserved.>
------------------------------------------------------------------------
�޸ļ�¼��
�� ��           �汾     �޸���     �߶���     �޸ļ�¼
2019/9/26       V1.0     ������     ������      ����ģ��
=====================================================================*/

#ifndef OPENATCFLOWPROCMANAGER_H
#define OPENATCFLOWPROCMANAGER_H

#include "OpenATCFlowProcLog.h"
#include "../Include/OpenATCParameter.h"
#include "../Include/OpenATCRunStatus.h"
#include "../Include/OpenATCLog.h"
#include "../Include/OneWayQueue.h"
#include "../Include/OpenATCFlowProcDefine.h"

#ifdef _WIN32
#define FLOWPROCMANAGER_CALLBACK WINAPI
typedef HANDLE               FLOWPROCMANAGERHANDLE;
#else
#define FLOWPROCMANAGER_CALLBACK
typedef pthread_t            FLOWPROCMANAGERHANDLE;
#endif

class COpenATCFlowProcManager  
{
public:
    //�ඨ��Ϊ����
    static COpenATCFlowProcManager * getInstance();

    //��ĳ�ʼ������
    void Init(COpenATCParameter * pParameter,COpenATCRunStatus * pRunStatus, COpenATCLog * pOpenATCLog);

    //���������
    void Work();

    //���ֹͣ���ͷ�
    void Stop();

	FLOWPROCMANAGERHANDLE  GetHandle();

	virtual int Run();

	//��ȡ��Ҫ�洢����������
    inline COneWayQueue<TStatisticVehDetData> & GetFlowDataQueue()
    {
        return m_FlowDataQueue;
    }
    //������Ҫ�洢����������
    inline void SetFlowDataQueue(COneWayQueue<TStatisticVehDetData> & oneWayQueue)
    {
        memcpy(&m_FlowDataQueue,&oneWayQueue,sizeof(m_FlowDataQueue));     
    }

	//��������������д��U��
	inline void BackUpLogFile(const char* pdir)
	{
        m_OpenATCFlowProcLog.BackUpLogFile(FLOW_FILE_DISK_PATH, m_pLogicCtlStatus, m_pOpenATCLog);
	}

private:
	COpenATCFlowProcManager();
	~COpenATCFlowProcManager();

    int  ReadConfig(char szXmlFile[]);

    int  SetLogFileName(const char *szLogFilePath, const char *szLogFileName);

    void CreateNewLogFile();	

    void LogOneMessage(const char *szFormat, ...);

    static COpenATCFlowProcManager * s_pData;               //������ָ��
    COpenATCParameter * m_pLogicCtlParam;                   //����������ָ��
    COpenATCRunStatus * m_pLogicCtlStatus;                  //����״̬��ָ��
	COpenATCLog       * m_pOpenATCLog;                      //��־��ָ��

	FLOWPROCMANAGERHANDLE         m_hThread;
    unsigned long                 m_dwThreadRet;
	static void *FLOWPROCMANAGER_CALLBACK FlowDataThread(void *pParam);


    //����ʵʱ��������
    void ProcRTVehDetData();

    //������ͳ������
    void ProcSTVehDetData();

    //���ݳ�������ʱ���жϳ���
    int GetVehType(long nExistTimeMs);

	//�ӳ�
	void OpenATCSleep(long nMsec);

	//��������ĺ���
	unsigned long CalcCounter(unsigned long nStart, unsigned long nEnd, unsigned long nMax);  

	void  SetDetectorStatusCounter();//�̵ƿ���ͳ������ʱ�����¼����״̬������

    TStatisticVehDetData m_tStatisticFlowData;                              //����ͳ������

	TStatisticVehDetData m_tOldStatisticFlowData;                           //���һ�ε�����ͳ������

	ofstream        m_outPutStream;
	size_t          m_nFileMaxSize;      //��־�ļ�����С �����ô�С����ɾ�� Ĭ��Ϊ10MB
	int				m_nFileMaxNum;		 //��¼����־�ļ�����

	char			m_szLogFilePath[FRM_MAX_LOGFILENAME_LENGTH];			//��־�ļ�����Ŀ¼
	char			m_szLogFileName[FRM_MAX_LOGFILENAME_LENGTH];			//��־���ĳ����� 
	char			m_szCurrLogFileName[FRM_MAX_LOGFILENAME_LENGTH];	    //��ǰ��־�ļ�����

    int             m_nLastMin;

	bool           m_bGreenStartFlag[MAX_VEHICLEDETECTOR_COUNT];            
	unsigned long  m_nGreenStartCounter[MAX_VEHICLEDETECTOR_COUNT];

    COneWayQueue<TStatisticVehDetData>  m_FlowDataQueue; 

	COpenATCFlowProcLog                 m_OpenATCFlowProcLog;

public:
    TStatisticVehDetData  &  GetCurrentStatisticVehDetData();

};

#endif //ifndef OPENATCFLOWPROCMANAGER_H
