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
	//unsigned short b_wRecordID;				//������¼ID
	long starttime;						//������¼��ʼʱ��
	long endtime;						//������¼����ʱ��
	unsigned char subject;				//�����ʾ
	unsigned char object;				//�����ʾ
	int infotype;						//�¼�����
	bool status;							//����״̬
	char desc[MAX_OPERATIONRECORD_DESC_LEN]; 	//����ʧ������
};
