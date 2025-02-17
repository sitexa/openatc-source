/*=====================================================================
模块名 ：MD5模块
文件名 ：OpenATCMd5.h
相关文件：OpenATCMd5.cpp
实现功能：用于JSON文件的完整性校验及避免手动修改
作者 ：梁厅
版权 ：<Copyright(c) 2019-2020 Suzhou Keda Technology Co.,Ltd. All right reserved.>
---------------------------------------------------------------------------------------------------------------------
修改记录：
日 期           版本     修改人     走读人     修改记录
2019/9/14       V1.0      梁厅       王五      创建模块
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
	
	//计算字符串的MD5码值
	void getMd5(unsigned char* str, char(&buff)[C_N_MAX_MD5BUFFER_SIZE]);

	void getMd5BaseLength(unsigned char* str, unsigned int nLength, char(&buff)[C_N_MAX_MD5BUFFER_SIZE]);

	//int ConvertGBK2UTF8(const char* szGBK, char* szUTF8, int nUTF8Size);
	
	//生成MD5码值

};
                        
#endif
