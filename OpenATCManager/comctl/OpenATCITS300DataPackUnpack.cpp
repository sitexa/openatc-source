/*====================================================================
模块名 ：打包解包模块
文件名 ：ITS300DataPackUnpack.cpp
相关文件：ITS300DataPackUnpack.h
实现功能：信号机和ITS300通信的时候对数据进行打包、解包
作者 ：李永萍
版权 ：<Copyright(C) 2019-2020 Suzhou Keda Technology Co., Ltd. All rights reserved.>
---------------------------------------------------------------------------------------------------------------------
修改记录：
日 期           版本    修改人             走读人             修改记录
2019/09/06      V1.0    李永萍             李永萍             创建模块
====================================================================*/

#include "OpenATCITS300DataPackUnpack.h"
#include <string.h>

const  char CB_HEADTAIL = 0xC0;

COpenATCITS300DataPackUnpack::COpenATCITS300DataPackUnpack()
{
	unpackCache_.clear();
}

COpenATCITS300DataPackUnpack::~COpenATCITS300DataPackUnpack()
{
	unpackCache_.clear();
}

void COpenATCITS300DataPackUnpack::Write(unsigned char *pBuff, unsigned int dwCount)
{
	for (int i = 0; i < dwCount; ++i)
    {
        unpackCache_.push_back(pBuff[i]);
    }

    if (unpackCache_.size() > CN_MAX_RECVBUFFER_SIZE)
    {
        EraseHeadItem(unpackCache_.size() - CN_MAX_RECVBUFFER_SIZE);
    }
}

void COpenATCITS300DataPackUnpack::PackBuffer(unsigned char *pSource, unsigned int dwSrcCount, unsigned char *pDest, unsigned int & dwDstCount)
{
	pDest[0] = CB_HEADTAIL;
	dwDstCount = 1;

	int nSum = 0;
	for (unsigned int i = 0;i < dwSrcCount;i++)
	{
		pDest[dwDstCount++] = pSource[i];
		nSum += pSource[i];
	}

    pDest[dwDstCount++] = nSum % 256;
	pDest[dwDstCount++] = CB_HEADTAIL;
}

int COpenATCITS300DataPackUnpack::Read(unsigned char *pBuff, unsigned int & dwCount)
{
	unsigned int i = 0;
	int startPos = 0;
	int endPos = 0;
	int nSum = 0;

    //检查缓冲区有无内容
    if (unpackCache_.size() == 0)
    {
        return ReadNoComplete;
    }
	
    //遍历搜索先开始标志位
    if (!FindFirstStartFlag(startPos))
    {
        EraseHeadItem(unpackCache_.size());
        return ReadNoComplete;
    }
	
    //擦除前面无用的数据。
    EraseHeadItem(startPos);

	if (!FindLastEndFlag(endPos) || (endPos < startPos))
    {
        EraseHeadItem(unpackCache_.size());
        return ReadNoComplete;
    }

	int nSize = endPos - startPos + 1;
	
    for (i = 0; i < nSize - 1; ++i)
	{
		pBuff[i] = unpackCache_.at(i);
		nSum += pBuff[i];	
	}

	nSum = nSum % 256;
	if (unpackCache_.at(nSize - 1) != (char)nSum)
	{
		return ReadNoComplete;
    }
	else
	{
		dwCount = nSize - 1;

		//擦除包内容和包尾
		EraseHeadItem(nSize + 1);

		return ReadOk;
	}
}

void COpenATCITS300DataPackUnpack::EraseHeadItem(int cnt)
{
    if ((unsigned int)cnt >= unpackCache_.size())
    {
        unpackCache_.clear();
        return;
    }
	
    unpackCache_.erase(unpackCache_.begin(), unpackCache_.begin()+cnt);
}

bool COpenATCITS300DataPackUnpack::FindFirstStartFlag(int & pos)
{
	if (unpackCache_.size() < 2)
	{
		return false;
	}
	
    for (unsigned int i = 0; i <= unpackCache_.size() - 2; ++i)
    {
		if (unpackCache_.at(i) == CB_HEADTAIL && unpackCache_.at(i + 1) != CB_HEADTAIL)
		{
			pos = i + 1;
			return true;
		}
    }
	
    return false;
}

bool COpenATCITS300DataPackUnpack::FindLastEndFlag(int & pos)
{
	if (unpackCache_.size() < 2)
	{
		return false;
	}
	
    /*for (unsigned int i = 0; i <= unpackCache_.size() - 2; ++i)
    {
		if (unpackCache_.at(i) == CB_HEADTAIL && unpackCache_.at(i + 1) == CB_HEADTAIL)
		{
			pos = i;
			return true;
		}
    }*/

	if (unpackCache_.at(unpackCache_.size() - 1) == CB_HEADTAIL)
	{
		pos = unpackCache_.size() - 1;
		return true;
	}
	
    return false;
}