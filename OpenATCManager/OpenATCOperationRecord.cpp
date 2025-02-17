/*====================================================================
模块名 ：操作记录模块
文件名 ：OpenATCOperationRecord.cpp
相关文件：OpenATCOperationRecord.h
实现功能：实现操作记录相关的功能
作者 ：梁厅
版权 ：<Copyright(C) 2019-2020 Suzhou Keda Technology Co., Ltd. All rights reserved.>
---------------------------------------------------------------------------------------------------------------------
修改记录：
日 期           版本    修改人             走读人             修改记录
2019/08/03      V1.0    李永萍             李永萍             创建模块
====================================================================*/

#include "OpenATCOperationRecord.h"
#include "../Include/OpenATCLog.h"
#include "OpenATCOperationRecordBase.h"
#include "OpenATCOperationLogBase.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/filewritestream.h"
#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include <iostream>
#include <malloc.h> 
#ifdef _WIN32
#include <io.h>
#else
#include<unistd.h>
#include<stdio.h>
#include<string.h>
#include <sys/time.h>
#include <sys/reboot.h>
#include <stdlib.h>
#endif
#include <fstream>
#include <string>
#include <sstream>
#include <deque>

extern COpenATCLog g_CommonLog;
using namespace rapidjson;

COpenATCOperationRecord::COpenATCOperationRecord()
{
	m_pOperationRecordBuffer = new unsigned char[C_N_MAXOPERATIONRECORDBUFFER_SIZE];
	m_pOperationWriteBuffer = new unsigned char[C_N_MAXOPERATIONRECORDBUFFER_SIZE];
	m_nAscOperationRecordCount = 0;
	m_nAscOperationRecordCoverIndex = 0;
}

COpenATCOperationRecord::~COpenATCOperationRecord()
{
	delete[] m_pOperationRecordBuffer;
	delete[] m_pOperationWriteBuffer;
}



/*==================================================================== 
函数名 CreateOperationRecordFile 
功能 ：创建操作记录json文件 
算法实现 ： 
参数说明 ： 
返回值说明：无
----------------------------------------------------------------------
修改记录 ： 
日 期           版本   修改人  走读人  修改记录 
2019/08/03      V1.0   李永萍          创建函数 
====================================================================*/ 
void COpenATCOperationRecord::CreateOperationRecordFile()
{
	if (access(OPERATIONRECORD_FILE_PATH, 0) == 0)		//如果日志路径存在文件
	{
		FILE *pf = fopen(OPERATIONRECORD_FILE_PATH, "rb");
		if (pf)	
		{
			memset(m_pOperationRecordBuffer, 0x00, strlen((char *)m_pOperationRecordBuffer));
			Document doc;	//创建rapidjson的doc文件
			FileReadStream is(pf,(char *)m_pOperationRecordBuffer,sizeof((char *)m_pOperationRecordBuffer));
			doc.ParseStream<0>(is);
			if (doc.HasParseError())	//如果json文件内容为空
			{
				fclose(pf);
				doc.SetObject();
				doc.AddMember("operationrecord",Value().SetArray(),doc.GetAllocator());
				/*存入json*/
				FILE *pfile = fopen(OPERATIONRECORD_FILE_PATH, "wb");
				memset(m_pOperationWriteBuffer, 0x00, strlen((char *)m_pOperationWriteBuffer));
				FileWriteStream os(pfile,(char *)m_pOperationWriteBuffer,sizeof((char *)m_pOperationWriteBuffer));
				Writer<FileWriteStream> writer(os);
				doc.Accept(writer);
				fclose(pfile);
				m_nAscOperationRecordCount = 0;
			}
			else
			{
				fclose(pf);
				if(doc.HasMember("operationrecord") && doc["operationrecord"].IsArray())
				{

					Value& operationRecordArray = doc["operationrecord"];
					m_nAscOperationRecordCount = operationRecordArray.Size();
				}

			}
		}
	}
	else if (access(OPERATIONRECORD_FILE_PATH, 0) != 0)
	{
		Document doc;
		doc.SetObject();
		doc.AddMember("operationrecord",Value().SetArray(),doc.GetAllocator());
		FILE *pfile = fopen(OPERATIONRECORD_FILE_PATH, "wb");
		memset(m_pOperationWriteBuffer, 0x00, strlen((char *)m_pOperationWriteBuffer));
		FileWriteStream os(pfile,(char *)m_pOperationWriteBuffer,sizeof((char *)m_pOperationWriteBuffer));
		Writer<FileWriteStream> writer(os);
		doc.Accept(writer);
		fclose(pfile);
		m_nAscOperationRecordCount = 0;
	}
}


/*==================================================================== 
函数名 SaveOneOperationRecordMessage 
功能 ：保存一条操作记录
算法实现 ： 
参数说明 ： 
返回值说明：无
----------------------------------------------------------------------
修改记录 ： 
日 期          版本   修改人  走读人  修改记录 
2019/08/03     V1.0   李永萍          创建函数 
====================================================================*/ 
void COpenATCOperationRecord::SaveOneOperationRecordMessage(TAscOperationRecord *pTAscOperationRecord, COpenATCRunStatus * pLogicCtlStatus)
{	
	CreateOperationRecordFile();
	COpenATCOperationLogBase m_OperationLogBase;
	COpenATCOperationRecordBase m_OperationBase;
	ifstream of(OPERATIONRECORD_FILE_PATH);
	if (of.is_open())
	{
		ostringstream tmp;
		tmp<<of.rdbuf();
		string	str = tmp.str();
		if (!str.empty())
		{
			COpenATCOperationLogBase::FromJson(&m_OperationLogBase,str);
			if (m_nAscOperationRecordCount < C_N_MAX_OPERATIONRECORD_MESSAGE_COUNT)
			{
				//m_OperationBase.b_wRecordID = pTAscOperationRecord->m_wRecordID;
				if (pTAscOperationRecord->m_unStartTime > 0)
				{
					m_OperationBase.starttime = pTAscOperationRecord->m_unStartTime;
				}
				else
				{
					m_OperationBase.starttime = 0;
				}
				if (pTAscOperationRecord->m_unEndTime)
				{
					m_OperationBase.endtime = pTAscOperationRecord->m_unEndTime;
				}
				else
				{
					m_OperationBase.endtime = 0;
				}
				m_OperationBase.subject = pTAscOperationRecord->m_bySubject;
				m_OperationBase.object = pTAscOperationRecord->m_byObject;
				m_OperationBase.infotype = pTAscOperationRecord->m_nInfoType;
				m_OperationBase.status = pTAscOperationRecord->m_bStatus;
				strcpy(m_OperationBase.desc,(const char*)pTAscOperationRecord->m_byFailureValue);
				m_OperationLogBase.operationrecord.arr.push_back(m_OperationBase);
				m_nAscOperationRecordCount += 1;
				string str = m_OperationLogBase.ToJson();
				ofstream os;
				os.open(OPERATIONRECORD_FILE_PATH);
				os << str;
				os.close();
			}
			else if (m_nAscOperationRecordCount == C_N_MAX_OPERATIONRECORD_MESSAGE_COUNT)
			{
				//m_OperationBase.b_wRecordID = pTAscOperationRecord->m_wRecordID;
				if (pTAscOperationRecord->m_unStartTime > 0)
				{
					m_OperationBase.starttime = pTAscOperationRecord->m_unStartTime;
				}
				else
				{
					m_OperationBase.starttime = 0;
				}
				if (pTAscOperationRecord->m_unEndTime)
				{
					m_OperationBase.endtime = pTAscOperationRecord->m_unEndTime;
				}
				else
				{
					m_OperationBase.endtime = 0;
				}
				m_OperationBase.subject = pTAscOperationRecord->m_bySubject;
				m_OperationBase.object = pTAscOperationRecord->m_byObject;
				m_OperationBase.infotype = pTAscOperationRecord->m_nInfoType;
				m_OperationBase.status = pTAscOperationRecord->m_bStatus;
				strcpy(m_OperationBase.desc,(const char*)pTAscOperationRecord->m_byFailureValue);
				m_OperationLogBase.operationrecord.arr.pop_front();
				m_OperationLogBase.operationrecord.arr.push_back(m_OperationBase);
				m_nAscOperationRecordCount += 1;
				string str = m_OperationLogBase.ToJson();
				ofstream os;
				os.open(OPERATIONRECORD_FILE_PATH);
				os << str;
				os.close();
			}
		}
	}
}





