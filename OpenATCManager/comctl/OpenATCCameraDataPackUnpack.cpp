/*====================================================================
模块名 ：打包解包模块
文件名 OpenATCCameraDataPackUnpack.cpp
相关文件：OpenATCIOBoxDataPackUnpack.h
实现功能：信号机和IOBox通信的时候对数据进行打包、解包
作者 ：李永萍
版权 ：<Copyright(C) 2019-2020 Suzhou Keda Technology Co., Ltd. All rights reserved.>
---------------------------------------------------------------------------------------------------------------------
修改记录：
日 期           版本    修改人             走读人             修改记录
2019/09/06      V1.0    李永萍             李永萍             创建模块
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