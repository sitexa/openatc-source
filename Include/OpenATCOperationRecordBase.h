#pragma once
#include "RapidjsonHelper.h"
#include "../Include/OpenATCParamConstDefine.h"
using namespace PBLIB::RapidJsonHelper;


class COpenATCOperationRecordBase :public JsonBase
{
public:
	COpenATCOperationRecordBase()
	{
		memset(desc,0,MAX_OPERATIONRECORD_DESC_LEN);
	};
	~COpenATCOperationRecordBase()
	{};

	void ToWrite(Writer<StringBuffer> &writer)
	{
		RapidjsonWriteBegin(writer);
		//RapidjsonWriteInt(b_wRecordID);
		RapidjsonWriteInt(starttime);
		RapidjsonWriteInt(endtime);
		RapidjsonWriteInt(subject);
		RapidjsonWriteInt(object);
		RapidjsonWriteInt(infotype);
		RapidjsonWriteInt(status);
		RapidjsonWriteChar(desc);
		RapidjsonWriteEnd();
	}

	void ParseJson(const Value &val)
	{
		RapidjsonParseBegin(val);
		//RapidjsonParseToInt(b_wRecordID);
		RapidjsonParseToInt(starttime);
		RapidjsonParseToInt(endtime);
		RapidjsonParseToInt(subject);
		RapidjsonParseToInt(object);
		RapidjsonParseToInt(infotype);
		RapidjsonParseToInt(status);
		//TODO: RapidjsonParseToChar(desc);
		RapidjsonParseEnd();
	}

public:
	//unsigned short b_wRecordID;				//操作记录ID
	long starttime;						//操作记录起始时间
	long endtime;						//操作记录结束时间
	unsigned char subject;				//主体表示
	unsigned char object;				//客体表示
	int infotype;						//事件类型
	bool status;							//操作状态
	char desc[MAX_OPERATIONRECORD_DESC_LEN]; 	//操作失败描述
};
