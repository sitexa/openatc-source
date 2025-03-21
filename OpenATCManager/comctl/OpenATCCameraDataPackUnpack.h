/*=====================================================================
模块名 ：打包解包模块
文件名 ：OpenATCCameraDataPackUnpack.h
相关文件：
实现功能：信号机和IOBox通信的时候对数据进行打包、解包
作者 ：李永萍
版权 ：<Copyright(c) 2019-2020 Suzhou Keda Technology Co.,Ltd. All right reserved.>
--------------------------------------------------------------------------------------------------------------------
修改记录：
日 期           版本    修改人             走读人             修改记录
2019/09/06      V1.0    李永萍）           李永萍             创建模块
====================================================================*/

#ifndef OPENATCCAMERADATAPACKUNPACK_H
#define OPENATCCAMERADATAPACKUNPACK_H

#include "OpenATCPackUnpackBase.h"
#include <vector>

/*=====================================================================
类名 ：CBuffer
功能 ：队列类,可以进行数据打包、解包
主要接口：void PackBuffer：打包数据，第一个参数为源数据，第二个参数为源数据长度，第三个参数为打包好的数据，第四个参数为打包好的数据的长度
备注 ：
--------------------------------------------------------------------------------------------------------------------
修改记录：
日 期           版本    修改人             走读人             修改记录
2019/09/06      V1.0    李永萍             李永萍             创建类
====================================================================*/

class COpenATCCameraDataPackUnpack : public COpenATCPackUnpackBase    
{
public:
	COpenATCCameraDataPackUnpack();
	virtual ~COpenATCCameraDataPackUnpack();

    /****************************************************
	函数名：Write
    功能：把接收的数据写入接收缓冲区
	算法实现:
    参数说明 ： pBuff,需要写入的数据指针
	            dwCount,数据长度
    返回值说明：无
    --------------------------------------------------------------------------------------------------------------------
	修改记录：
	日 期           版本    修改人             走读人             修改记录
	2019/09/06      V1.0    李永萍             李永萍             创建
	====================================================================*/
	void Write(unsigned char *pBuff, unsigned int dwCount);

    /****************************************************
	函数名：PackBuffer
    功能：打包
	算法实现:
    参数：  pSource,打包源数据
	        dwSrcCount,源数据的数量
	        pDest,打包目标数据指针
	        dwDstCount,打包结果长度
    返回值：无
    --------------------------------------------------------------------------------------------------------------------
	修改记录：
	日 期           版本    修改人             走读人             修改记录
	2019/09/06      V1.0    李永萍             李永萍             创建
	====================================================================*/
	void PackBuffer(unsigned char *pSource, unsigned int dwSrcCount, unsigned char *pDest, unsigned int & dwDstCount){};
	
    /****************************************************
	函数名：Read
    功能：解包
	算法实现:
    参数：  pBuff,解出的数据包内容
	        dwCount,数据包的长度
    返回值：ReadNoData 数据长度为零
	        ReadNoComplete 数据不完整
			ReadWrongData 数据错误
			ReadOk 读数据成功
    --------------------------------------------------------------------------------------------------------------------
	修改记录：
	日 期           版本    修改人             走读人             修改记录
	2019/09/06      V1.0    李永萍             李永萍             创建
	====================================================================*/
    int Read(unsigned char *pBuff, unsigned int & dwCount);

    /****************************************************
	函数名：WriteSend
    功能：向发送缓冲区写入数据
	算法实现:
    参数：  pBuff,需要写入的数据指针
	        dwCount,需要写入的数据量
    返回值：无
    --------------------------------------------------------------------------------------------------------------------
	修改记录：
	日 期           版本    修改人             走读人             修改记录
	2019/09/06      V1.0    李永萍             李永萍             创建
	====================================================================*/
	void WriteSend(unsigned char *pBuff, unsigned int dwCount){};
	
    /****************************************************
	函数名：GetSendBuffPtr
    功能：获取发送缓冲区的头指针与数据量
	算法实现:
    参数：  ppBuff,发送缓冲区的头指针的指针
	        dwCount,发送缓冲区的数据量
    返回值：无
    --------------------------------------------------------------------------------------------------------------------
	修改记录：
	日 期           版本    修改人             走读人             修改记录
	2019/09/06      V1.0    李永萍             李永萍             创建
	====================================================================*/
	void GetSendBuffPtr(unsigned char **ppBuff, unsigned int & dwCount){};
	
    /****************************************************
	函数名：OnSend
    功能：数据发送完成时调用,移动头指针
	算法实现:
    参数：  dwCount,发送成功的数据量
    返回值：无
    --------------------------------------------------------------------------------------------------------------------
	修改记录：
	日 期           版本    修改人             走读人             修改记录
	2019/09/06      V1.0    李永萍             李永萍             创建
	====================================================================*/
	void OnSend(unsigned int dwCount){};

    /****************************************************
	函数名：ClearBuff
    功能：清空缓冲区,重置缓冲区头尾指针。
	算法实现:
    参数：  无。
    返回值：无。
    --------------------------------------------------------------------------------------------------------------------
	修改记录：
	日 期           版本    修改人             走读人             修改记录
	2019/09/06      V1.0    李永萍             李永萍             创建
	====================================================================*/
	void ClearBuff(void){};
	void ClearBuff(int rawTail);

	/****************************************************
	函数名：HasPack
    功能：判断是否包含包头包尾。
	算法实现:
    参数：  无。
    返回值：无。
    --------------------------------------------------------------------------------------------------------------------
	修改记录：
	日 期           版本    修改人             走读人             修改记录
	2019/09/06      V1.0    李永萍								  创建
	====================================================================*/
	bool HasPack(int& rawHead, int& rawTail);

protected:
	std::vector<unsigned char>	unpackBuff_;			//接收数据缓冲区
	unsigned int	m_dwHead;							//接受缓冲区的头位置
	unsigned int	m_dwTail;							//接受缓冲区的尾位置
private:
	/// 接受数据缓冲区
	enum
	{
		MAX_CACHE_SIZE = 2 * 1024,
	};
};

#endif  //!ifndef OPENATCCAMERADATAPACKUNPACK_H




