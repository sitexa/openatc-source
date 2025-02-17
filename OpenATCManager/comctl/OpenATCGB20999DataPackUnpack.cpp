/*====================================================================
ģ���� ��������ģ��
�ļ��� ��GB20999DataPackUnpack.cpp
����ļ���GB20999DataPackUnpack.h
ʵ�ֹ��ܣ��źŻ���GB20999���������ʱ������ݽ��д�������
���� ������Ƽ
��Ȩ ��<Copyright(C) 2019-2020 Suzhou Keda Technology Co., Ltd. All rights reserved.>
---------------------------------------------------------------------------------------------------------------------
�޸ļ�¼��
�� ��           �汾    �޸���             �߶���             �޸ļ�¼
2019/09/06      V1.0    ����Ƽ             ����Ƽ             ����ģ��
====================================================================*/

#include "OpenATCGB20999DataPackUnpack.h"
#include <string.h>

const  char CB_HEAD = 0x7E;
const  char CB_TAIL = 0x7D;

COpenATCGB20999DataPackUnpack::COpenATCGB20999DataPackUnpack()
{
	unpackCache_.clear();
}

COpenATCGB20999DataPackUnpack::~COpenATCGB20999DataPackUnpack()
{
	unpackCache_.clear();
}

void COpenATCGB20999DataPackUnpack::Write(unsigned char *pBuff, unsigned int dwCount)
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

void COpenATCGB20999DataPackUnpack::PackBuffer(unsigned char *pSource, unsigned int dwSrcCount, unsigned char *pDest, unsigned int & dwDstCount)
{
	pDest[0] = CB_HEAD;
	dwDstCount = 1;

	int nSum = 0;
	for (unsigned int i = 0;i < dwSrcCount;i++)
	{
		pDest[dwDstCount++] = pSource[i];
		nSum += pSource[i];
	}

	pDest[dwDstCount++] = CB_TAIL;
}

int COpenATCGB20999DataPackUnpack::Read(unsigned char *pBuff, unsigned int & dwCount)
{
	unsigned int i = 0;
	int startPos = 0;
	int endPos = 0;

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
	}

	short nLength = 0;
	char chLength[2] = {0, 0};
	chLength[1] = pBuff[0];
	chLength[0] = pBuff[1];
	memcpy(&nLength, chLength, 2);
	nLength += 2;
	if (nLength != nSize)
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

void COpenATCGB20999DataPackUnpack::EraseHeadItem(int cnt)
{
    if ((unsigned int)cnt >= unpackCache_.size())
    {
        unpackCache_.clear();
        return;
    }
	
    unpackCache_.erase(unpackCache_.begin(), unpackCache_.begin()+cnt);
}

bool COpenATCGB20999DataPackUnpack::FindFirstStartFlag(int & pos)
{
	if (unpackCache_.size() < 2)
	{
		return false;
	}
	
    for (unsigned int i = 0; i <= unpackCache_.size() - 2; ++i)
    {
		if (unpackCache_.at(i) == CB_HEAD && unpackCache_.at(i + 1) != CB_TAIL)
		{
			pos = i + 1;
			return true;
		}
    }
	
    return false;
}

bool COpenATCGB20999DataPackUnpack::FindLastEndFlag(int & pos)
{
	if (unpackCache_.size() < 2)
	{
		return false;
	}
	
    for (unsigned int i = 0; i <= unpackCache_.size() - 2; ++i)
    {
		if (unpackCache_.at(i) == CB_TAIL && unpackCache_.at(i + 1) == CB_HEAD)
		{
			pos = i;
			return true;
		}
    }

	if (unpackCache_.at(unpackCache_.size() - 1) == CB_TAIL)
	{
		pos = unpackCache_.size() - 1;
		return true;
	}
	
    return false;
}

unsigned short COpenATCGB20999DataPackUnpack::Crc16(const unsigned char *buffer, int buffer_length)
{
    unsigned char c, treat, bcrc;
    unsigned short wcrc = 0;
    int i, j;
    for (i = 0; i < buffer_length; i++)
    {
        c = buffer[i];
        for (j = 0; j < 8; j++)
        {
            treat = c & 0x80;
            c <<= 1;
            bcrc = (wcrc >> 8) & 0x80;
            wcrc <<= 1;
            if (treat != bcrc)
                    wcrc ^= 0x1021;
        }
    }
    return wcrc;
}
