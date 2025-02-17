
//=============================================================================
/**
 * @file    PackUnpackBase.h
 *
 *
 * This file define a base class to pack/unpack system.
 *
 *
 * @author 
 */
//=============================================================================

#if     _MSC_VER > 1000
#  pragma once
#endif

#ifndef HL__PACKUNPACK_BASE__H
#define HL__PACKUNPACK_BASE__H

union UN_LONG
{
    unsigned long l_field;
    struct  
    {
        char    b0;
        char    b1;
        char    b2;
        char    b3;
    }b_field;
};

union UN_SHORT
{
    unsigned short s_field;
    struct  
    {
        char    b0;
        char    b1;
    }b_field;
};

class CPackUnpackBase
{
public:
    CPackUnpackBase(){}
    virtual ~CPackUnpackBase(){}

	enum EUnpackRetern
	{
		/// 解压缩成功
		UNPACK_DONE = 0,
			
		/// 接收未完成
		NO_PACK     = 1,
		
		/// 校验失败
		CHK_FAILED  = 2, 
		
		/// 有包无数据
		EMPTY_PACK  = 3,
		
		/// 包长度校验失败
		LENGTH_ERR  = 4,

        /// 缓冲区无数据
        EMPTY_DATA  = 5,

        /// 有数据无协议内容
        ERR_DATA    = 6,
    };

	/**
	 *  WriteToBuff() : 将收到的信息放到解包缓冲区
	 *
	 *  @param:	
	 *		[in] recvBuff -- 接收到数据
	 *		[in] buffSize -- 接收到的数据长度
	 *
	 *  @return:
	 *		FRM_RTN_
	 */
	virtual int WriteToBuff(const char* recvBuff, const int buffSize) = 0;

	/**
	 *  UnPack() :  从接受缓冲区获取一个解压包
	 *   
	 *  @param : 
	 *		[out] rawBuff -- 解压后的缓冲区
	 *      [out] rawBuffSize -- 解压后的缓冲区的大小
	 *	
	 *	@return :
	 *		FRM_RTN_
	 */
	virtual int UnPack(char* rawBuff, int rawBuffSize, int& infoSize) = 0;
	virtual int UnPack(char* rawBuff) = 0;


	/**
	 *  Pack() : 对数据包进行协议打包  
	 *
	 *  @param :
	 *		[in] rawBuff -- 原始数据包
	 *		[in] rawBuffSize -- 原始数据包大小
	 *      [out] packBuff -- 打包后的协议
	 *	    [in] packBuffSize -- 打包后缓存的大小
	 *
	 *  @return :
	 *		打包后的大小
	 */
	virtual int Pack(const char* rawBuff, const int rawBuffSize,
		char* packBuff, int packBuffSize) = 0;

	/**
	 *  Pack() : 对数据包进行协议打包  
	 *
	 *  @param :
	 *		[in] nBtnIndex -- 步进按钮索引
	 *      [out] packBuff -- 打包后的协议
	 *
	 *  @return :
	 *		打包后的大小
	 */
	virtual int Pack(const int nBtnIndex, char* packBuff, const int nDataSrc) = 0;
};

#endif//HL__PACKUNPACK_BASE__H 


