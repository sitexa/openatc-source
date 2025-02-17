/*====================================================================
ģ���� ��������ģ��
�ļ��� ��DataPackUnpack.cpp
����ļ���DataPackUnpack.h
ʵ�ֹ��ܣ��źŻ�����λ��ͨ�ŵ�ʱ������ݽ��д�������
���� ������Ƽ
��Ȩ ��<Copyright(C) 2019-2020 Suzhou Keda Technology Co., Ltd. All rights reserved.>
---------------------------------------------------------------------------------------------------------------------
�޸ļ�¼��
�� ��           �汾    �޸���             �߶���             �޸ļ�¼
2019/09/06      V1.0    ����Ƽ             ����Ƽ             ����ģ��
====================================================================*/

#include "OpenATCDataPackUnpack.h"
#include <string.h>


const  char CB_HEADTAIL = 0xC0;
const  char CB_ESCAPE1  = 0xDB;
const  char CB_ESCAPE2  = 0xDC;
const  char CB_ESCAPE3  = 0xDD;	

COpenATCDataPackUnpack::COpenATCDataPackUnpack()
{
	unpackCache_.clear();
}

COpenATCDataPackUnpack::~COpenATCDataPackUnpack()
{
	unpackCache_.clear();
}

void COpenATCDataPackUnpack::Write(unsigned char *pBuff, unsigned int dwCount)
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

void COpenATCDataPackUnpack::PackBuffer(unsigned char *pSource, unsigned int dwSrcCount, unsigned char *pDest, unsigned int & dwDstCount)
{
	pDest[0] = CB_HEADTAIL;
	dwDstCount = 1;

	for (unsigned int i = 0;i < dwSrcCount;i++)
	{
		switch ((char)pSource[i])
		{
			case CB_HEADTAIL:
				{
					pDest[dwDstCount++] = CB_ESCAPE1;
					pDest[dwDstCount++] = CB_ESCAPE2;
				}
				break;
			case CB_ESCAPE1:
				{
					pDest[dwDstCount++] = CB_ESCAPE1;
					pDest[dwDstCount++] = CB_ESCAPE3;
				}
				break;
			default:
				{
					pDest[dwDstCount++] = pSource[i];
				}
				break;
		}
	}
    pDest[dwDstCount++] = 0x00;
	pDest[dwDstCount++] = CB_HEADTAIL;
}

int COpenATCDataPackUnpack::Read(unsigned char *pBuff, unsigned int & dwCount)
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

	dwCount = 0;

	int nSize = endPos - startPos + 1;
    for (i = 0; i < nSize; ++i)
	{
		if (unpackCache_.at(i) == CB_ESCAPE1)
		{
			switch (unpackCache_.at(i + 1))
			{
			case CB_ESCAPE2:
				{
					pBuff[dwCount++] = CB_HEADTAIL;
				}
				break;
			case CB_ESCAPE3:
				{
					pBuff[dwCount++] = CB_ESCAPE1;
				}
				break;
			default:
				{
					dwCount = 0;
					return ReadWrongData;
				}
				break;
			}
		}
		else
		{
			if (i >= 1 && unpackCache_.at(i - 1) == CB_ESCAPE1 && (unpackCache_.at(i) == CB_ESCAPE2 || unpackCache_.at(i) == CB_ESCAPE3))
			{
				
			}
			else
			{
				pBuff[dwCount++] = unpackCache_.at(i);
			}
		}
	}

	//���������ݺͰ�β
	EraseHeadItem(nSize + 1);

	return ReadOk;
}

void COpenATCDataPackUnpack::EraseHeadItem(int cnt)
{
    if ((unsigned int)cnt >= unpackCache_.size())
    {
        unpackCache_.clear();
        return;
    }
	
    unpackCache_.erase(unpackCache_.begin(), unpackCache_.begin()+cnt);
}

bool COpenATCDataPackUnpack::FindFirstStartFlag(int & pos)
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

bool COpenATCDataPackUnpack::FindLastEndFlag(int & pos)
{
	if (unpackCache_.size() < 2)
	{
		return false;
	}
	
    for (unsigned int i = 0; i <= unpackCache_.size() - 2; ++i)
    {
		if (unpackCache_.at(i) == CB_HEADTAIL && unpackCache_.at(i + 1) == CB_HEADTAIL)
		{
			pos = i;
			return true;
		}
    }

	if (unpackCache_.at(unpackCache_.size() - 1) == CB_HEADTAIL)
	{
		pos = unpackCache_.size() - 1;
		return true;
	}
	
    return false;
}