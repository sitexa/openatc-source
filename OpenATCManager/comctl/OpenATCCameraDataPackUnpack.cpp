/*====================================================================
ģ���� ��������ģ��
�ļ��� OpenATCCameraDataPackUnpack.cpp
����ļ���OpenATCIOBoxDataPackUnpack.h
ʵ�ֹ��ܣ��źŻ���IOBoxͨ�ŵ�ʱ������ݽ��д�������
���� ������Ƽ
��Ȩ ��<Copyright(C) 2019-2020 Suzhou Keda Technology Co., Ltd. All rights reserved.>
---------------------------------------------------------------------------------------------------------------------
�޸ļ�¼��
�� ��           �汾    �޸���             �߶���             �޸ļ�¼
2019/09/06      V1.0    ����Ƽ             ����Ƽ             ����ģ��
====================================================================*/

#include "OpenATCCameraDataPackUnpack.h"
#include <string.h>

const unsigned char PACK_HEAD_1 = 0xfd;
const unsigned char PACK_HEAD_2 = 0xfd;
const unsigned char PACK_TAIL_1 = 0x55;

COpenATCCameraDataPackUnpack::COpenATCCameraDataPackUnpack()
{
	unpackBuff_.clear();
}

COpenATCCameraDataPackUnpack::~COpenATCCameraDataPackUnpack()
{
	unpackBuff_.clear();
}

void COpenATCCameraDataPackUnpack::Write(unsigned char *pBuff, unsigned int dwCount)
{
	if (unpackBuff_.size() > MAX_CACHE_SIZE)
	{
		unpackBuff_.erase(unpackBuff_.begin(), unpackBuff_.begin()+unpackBuff_.size()-MAX_CACHE_SIZE);
	}

	for (unsigned int i=0; i<dwCount; ++i)
	{
		unpackBuff_.push_back(pBuff[i]);
	}
}

int COpenATCCameraDataPackUnpack::Read(unsigned char *pBuff, unsigned int & dwCount)
{
	/// Raw message's header and tailer cur.
	int rawHead = 0;
	int rawTail = 0;
	int ret = 0;

	/// If has a protocol package.
	if (HasPack(rawHead, rawTail) == false)
	{
		return ReadNoComplete;
	}

	/// Empty pack(AA AA AA AB)
	if ((rawHead+1) == rawTail)
	{
		ClearBuff(rawTail);
		return ReadNoData;
	}

	/// Copy raw message to rawBuff
	for (int i=0; i<rawTail-rawHead-1; ++i)
	{
		pBuff[i] = unpackBuff_.at(rawHead+1+i);
	}
	dwCount = rawTail-rawHead-1;

	/// Clear
	ClearBuff(rawTail);
	return ReadOk;
}

void COpenATCCameraDataPackUnpack::ClearBuff(int rawTail)
{
	unpackBuff_.erase(unpackBuff_.begin(), unpackBuff_.begin()+rawTail+2);
}

bool COpenATCCameraDataPackUnpack::HasPack(int& rawHead, int& rawTail)
{
	rawHead = 0;
	rawTail = 0;

	///pack length check.
	if (unpackBuff_.size() < 4)
	{
		return false;
	}

	/// Get Header.
	unsigned int i = 2;
	for (; i<=unpackBuff_.size(); ++i)
	{
		if ((unpackBuff_.at(i-2)==PACK_HEAD_1) && 
			(unpackBuff_.at(i-1)==PACK_HEAD_2))
		{
			rawHead = i-1;
			break;
		}
	}
	if (rawHead == 0)
	{
		return false;
	}

	/// Get tailer
	i = rawHead;
	for (; i<unpackBuff_.size()-1; ++i)
	{
		if ((unpackBuff_.at(i)==PACK_TAIL_1))
		{
			rawTail = i;
			break;
		}
	}
	if (rawTail == 0)
	{
		return false;
	}

	return true;
}