/*=====================================================================
ģ���� ��MD5ģ��
�ļ��� ��OpenATCMd5.h
����ļ���OpenATCMd5.cpp
ʵ�ֹ��ܣ�����JSON�ļ���������У�鼰�����ֶ��޸�
���� ������
��Ȩ ��<Copyright(c) 2019-2020 Suzhou Keda Technology Co.,Ltd. All right reserved.>
---------------------------------------------------------------------------------------------------------------------
�޸ļ�¼��
�� ��           �汾     �޸���     �߶���     �޸ļ�¼
2019/9/14       V1.0      ����       ����      ����ģ��
=====================================================================*/
#ifndef _OPENATCMD5_H
#define _OPENATCMD5_H

#include "OpenATCMd5ConstDefine.h"

const unsigned int C_N_MAX_MD5BUFFER_SIZE = 33;
const unsigned int C_N_MAX_MD5VALUE_SIZE = 16;
typedef struct
{
	unsigned int count[2];
	unsigned int state[4];
	unsigned char buffer[64];
}TMD5_CTX,*PTMD5_CTX;

class COpenATCMD5
{
public:
	COpenATCMD5();
	~COpenATCMD5();

	void MD5Init(TMD5_CTX *context);

	void MD5Update(TMD5_CTX *context, unsigned char *input, unsigned int inputlen);

	void MD5Final(TMD5_CTX *context, unsigned char digest[16]);

	void MD5Transform(unsigned int state[4], unsigned char block[64]);

	void MD5Encode(unsigned char *output, unsigned int *input, unsigned int len);

	void MD5Decode(unsigned int *output, unsigned char *input, unsigned int len);
	
	//�����ַ�����MD5��ֵ
	void getMd5(unsigned char* str, char(&buff)[C_N_MAX_MD5BUFFER_SIZE]);

	void getMd5BaseLength(unsigned char* str, unsigned int nLength, char(&buff)[C_N_MAX_MD5BUFFER_SIZE]);

	//int ConvertGBK2UTF8(const char* szGBK, char* szUTF8, int nUTF8Size);
	
	//����MD5��ֵ

};
                        
#endif
