/*=====================================================================
ģ���� �����ذ����в�����
�ļ��� ��OpenATCOperationRecord.h
����ļ���OpenATCOperationRecord.cpp
ʵ�ֹ��ܣ����ڲ�����¼�洢
���� ������Ƽ
��Ȩ ��<Copyright(c) 2019-2020 Suzhou Keda Technology Co.,Ltd. All right reserved.>
---------------------------------------------------------------------------------------------------------------------
�޸ļ�¼��
�� ��          �汾     �޸���     �߶���     �޸ļ�¼
2019/08/03     V1.0     ����Ƽ			      ����ģ��
=====================================================================*/

#ifndef OPENATCOPERATIONRECORD_H 
#define OPENATCOPERATIONRECORD_H
#include "../Include/OpenATCOperationRecordBase.h"
#include "../Include/OpenATCOperationLogBase.h"
#include "../Include/OpenATCParamConstDefine.h"
#include "../Include/OpenATCParamStructDefine.h"
#include <time.h>
#include "../Include/OpenATCRunStatus.h"

using namespace PBLIB::RapidJsonHelper;

#ifdef _WIN32
#define OPERATIONRECORD_FILE_PATH "./OPERATIONRECORD.json"
#else
//#define OPERATIONRECORD_FILE_PATH "../../log/OPERATIONRECORD.json"
#define OPERATIONRECORD_FILE_PATH "/usr/log/OPERATIONRECORD.json"
#endif

const int C_N_MAX_OPERATIONRECORD_MESSAGE_COUNT = 3000;
const int C_N_MAXOPERATIONRECORDBUFFER_SIZE = 100 * 1024;

class COpenATCOperationRecord
{
public:
    COpenATCOperationRecord();
	virtual ~COpenATCOperationRecord();
	
	//����һ��������¼��־
	void SaveOneOperationRecordMessage(TAscOperationRecord *pTAscOperationRecord, COpenATCRunStatus * pLogicCtlStatus);

private:
	//����������¼json�ļ�
	void CreateOperationRecordFile();
	unsigned char* m_pOperationRecordBuffer;
	unsigned char* m_pOperationWriteBuffer;

	unsigned int m_nAscOperationRecordCount;      //��ǰ��־�Ѵ������¼����
	unsigned int m_nAscOperationRecordCoverIndex; //�����±�
};


#endif// !ifndef OPENATCOPERATIONRECORD_H