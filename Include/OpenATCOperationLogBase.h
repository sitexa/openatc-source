#ifndef OPENATCOPERATIONLOG_H
#define OPENATCOPERATIONLOG_H

#include "RapidjsonHelper.h"
#include "../Include/OpenATCParamConstDefine.h"
#include "OpenATCOperationRecordBase.h"
using namespace PBLIB::RapidJsonHelper;


class COpenATCOperationLogBase :public JsonBase
{
public:
	COpenATCOperationLogBase(){};

	~COpenATCOperationLogBase(){};

	JsonArray<COpenATCOperationRecordBase> operationrecord;	

	void ToWrite(Writer<StringBuffer> &writer)
	{
		RapidjsonWriteBegin(writer);
		RapidjsonWriteClass(operationrecord);
		RapidjsonWriteEnd();
	}

	void ParseJson(const Value& val)
	{
		RapidjsonParseBegin(val);
		RapidjsonParseToClass(operationrecord);
		RapidjsonParseEnd();
	}
};

#endif