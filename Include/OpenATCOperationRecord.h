/*=====================================================================
模块名 ：主控板运行参数类
文件名 ：OpenATCOperationRecord.h
相关文件：OpenATCOperationRecord.cpp
实现功能：用于操作记录存储
作者 ：李永萍
版权 ：<Copyright(c) 2019-2020 Suzhou Keda Technology Co.,Ltd. All right reserved.>
---------------------------------------------------------------------------------------------------------------------
修改记录：
日 期          版本     修改人     走读人     修改记录
2019/08/03     V1.0     李永萍			      创建模块
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
	
	//保存一条操作记录日志
	void SaveOneOperationRecordMessage(TAscOperationRecord *pTAscOperationRecord, COpenATCRunStatus * pLogicCtlStatus);

private:
	//创建操作记录json文件
	void CreateOperationRecordFile();
	unsigned char* m_pOperationRecordBuffer;
	unsigned char* m_pOperationWriteBuffer;

	unsigned int m_nAscOperationRecordCount;      //当前日志已存操作记录数量
	unsigned int m_nAscOperationRecordCoverIndex; //覆盖下标
};


#endif// !ifndef OPENATCOPERATIONRECORD_H