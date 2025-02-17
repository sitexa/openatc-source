/*====================================================================
ģ���� ��������ģ��
�ļ��� ��ITS300DataPackUnpack.cpp
����ļ���ITS300DataPackUnpack.h
ʵ�ֹ��ܣ��źŻ���ITS300ͨ�ŵ�ʱ������ݽ��д�������
���� ������Ƽ
��Ȩ ��<Copyright(C) 2019-2020 Suzhou Keda Technology Co., Ltd. All rights reserved.>
---------------------------------------------------------------------------------------------------------------------
�޸ļ�¼��
�� ��           �汾    �޸���             �߶���             �޸ļ�¼
2019/09/06      V1.0    ����Ƽ             ����Ƽ             ����ģ��
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

    //��黺������������
    if (unpackCache_.size() == 0)
    {
        return ReadNoComplete;
    }
	
    //���������ȿ�ʼ��־λ
    if (!FindFirstStartFlag(startPos))
    {
        EraseHeadItem(unpackCache_.size());
        return ReadNoComplete;
    }
	
    //����ǰ�����õ����ݡ�
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

		//���������ݺͰ�β
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