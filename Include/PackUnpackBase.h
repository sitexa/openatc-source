
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
		/// ��ѹ���ɹ�
		UNPACK_DONE = 0,
			
		/// ����δ���
		NO_PACK     = 1,
		
		/// У��ʧ��
		CHK_FAILED  = 2, 
		
		/// �а�������
		EMPTY_PACK  = 3,
		
		/// ������У��ʧ��
		LENGTH_ERR  = 4,

        /// ������������
        EMPTY_DATA  = 5,

        /// ��������Э������
        ERR_DATA    = 6,
    };

	/**
	 *  WriteToBuff() : ���յ�����Ϣ�ŵ����������
	 *
	 *  @param:	
	 *		[in] recvBuff -- ���յ�����
	 *		[in] buffSize -- ���յ������ݳ���
	 *
	 *  @return:
	 *		FRM_RTN_
	 */
	virtual int WriteToBuff(const char* recvBuff, const int buffSize) = 0;

	/**
	 *  UnPack() :  �ӽ��ܻ�������ȡһ����ѹ��
	 *   
	 *  @param : 
	 *		[out] rawBuff -- ��ѹ��Ļ�����
	 *      [out] rawBuffSize -- ��ѹ��Ļ������Ĵ�С
	 *	
	 *	@return :
	 *		FRM_RTN_
	 */
	virtual int UnPack(char* rawBuff, int rawBuffSize, int& infoSize) = 0;
	virtual int UnPack(char* rawBuff) = 0;


	/**
	 *  Pack() : �����ݰ�����Э����  
	 *
	 *  @param :
	 *		[in] rawBuff -- ԭʼ���ݰ�
	 *		[in] rawBuffSize -- ԭʼ���ݰ���С
	 *      [out] packBuff -- ������Э��
	 *	    [in] packBuffSize -- ����󻺴�Ĵ�С
	 *
	 *  @return :
	 *		�����Ĵ�С
	 */
	virtual int Pack(const char* rawBuff, const int rawBuffSize,
		char* packBuff, int packBuffSize) = 0;

	/**
	 *  Pack() : �����ݰ�����Э����  
	 *
	 *  @param :
	 *		[in] nBtnIndex -- ������ť����
	 *      [out] packBuff -- ������Э��
	 *
	 *  @return :
	 *		�����Ĵ�С
	 */
	virtual int Pack(const int nBtnIndex, char* packBuff, const int nDataSrc) = 0;
};

#endif//HL__PACKUNPACK_BASE__H 


