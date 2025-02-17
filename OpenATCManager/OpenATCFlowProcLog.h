/*=====================================================================
ģ���� �����ذ����в�����
�ļ��� ��OpenATCFlowProLog.h
����ļ���OpenATCFlowProLog.cpp
ʵ�ֹ��ܣ����ڴ洢��ͨ������־ģ��
���� ������
��Ȩ ��<Copyright(c) 2019-2020 Suzhou Keda Technology Co.,Ltd. All right reserved.>
---------------------------------------------------------------------------------------------------------------------
�޸ļ�¼��
�� ��           �汾     �޸���     �߶���     �޸ļ�¼
2019/9/14       V1.0     ����       ����      ����ģ��
=====================================================================*/
#ifndef OPENATCFLOWPROLOG_H 
#define OPENATCFLOWPROLOG_H

#include "cJSON.h"
#include "../Include/OpenATCRunStatus.h"
#include "../Include/OpenATCLog.h"
#include "../Include/OpenATCFlowProcDefine.h"

#ifdef _WIN32
#include <time.h>
#define FLOW_FILE_LOCAL_PATH "./TRAFFICFLOW.json"
#define FLOW_FILE_DISK_PATH  ".//TrafficFlowLog"  //U���ļ���
#else
#define FLOW_FILE_LOCAL_PATH "/usr/log/TRAFFICFLOW.json"
#define FLOW_FILE_DISK_PATH  "/mnt/TrafficFlowLog"  //U���ļ���
#include <sys/time.h>
#endif

const int C_N_MAX_VEHICLEDETECTOR_COUNT = 72;
const int C_N_MAXFLOWTBUFFER_SIZE		= 1024 * 1024 * 50;//buffer ���ߴ�
const int C_N_MAX_FLOW_FILE_SIZE		= 1024 * 1024 * 25; //�ļ����մ洢���ߴ�
const int C_N_MAX_FILEPATH_SIZE			= 128;
const int C_N_MAX_TIME_CHAR_SIZE		= 12;
const int C_N_MAX_TIME_BUFF_SIZE		= 30;
const int FLOW_DATA_FILE_RETENTION_TIME = 1;	// �����ļ�����ʱ������λ����


class COpenATCFlowProcLog
{
public:
	COpenATCFlowProcLog();
	virtual ~COpenATCFlowProcLog();

	//��¼��ͨ����Ϣ
	void SaveTrafficFlowInfo(PTStatisticVehDetData, COpenATCRunStatus * pRunStatus, COpenATCLog * pOpenATCLog);
		
	//��������������д��U��
	void BackUpLogFile(const char* pdir, COpenATCRunStatus * pRunStatus, COpenATCLog * pOpenATCLog);

	//U���ϵ������ļ�ά��
	void FlowDataFilesMaintenance(COpenATCLog * pOpenATCLog);

private:
	//������ͨ����־json�ļ�
	void CreateFlowFile();

	//�ж�ָ��Ŀ¼�ļ����Ƿ���ڣ������ڴ����ļ���
	void NewDirCreate(const char* pdir, int& nResult, COpenATCLog * pOpenATCLog);

	// ����U��
	int MountUSBDevice(COpenATCRunStatus * pRunStatus, COpenATCLog * pOpenATCLog);

	// ж��U��
	int UnmountUSBDevice(COpenATCRunStatus * pRunStatus, COpenATCLog * pOpenATCLog);

	char *m_pFaultBuffer;

	unsigned long m_nAscFlowFileSize;  //��ǰ��־ά����С,Ĭ�����1G

};


#endif// !ifndef OPENATCFAULTLOG_H