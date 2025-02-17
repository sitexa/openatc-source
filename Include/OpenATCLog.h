/************************************************************************
/*=====================================================================
模块名 ：日志模块
文件名 ：OpenATCLog.h
相关文件：
实现功能：基础通信
作者 ：李永萍
版权 ：<Copyright(c) 2019-2020 Suzhou Keda Technology Co.,Ltd. All right reserved.>
--------------------------------------------------------------------------------------------------------------------
修改记录：
日 期           版本    修改人             走读人             修改记录
2019/09/06      V1.0    李永萍             李永萍             创建模块
====================================================================*/

#ifndef OPENATCLOG_H 
#define OPENATCLOG_H

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <assert.h>

#ifdef _WIN32
#include <direct.h>
#include <Windows.h>
#endif

#include <mutex>
#include <string>

using namespace std;

#define TIMESTR_LENGTH      23
#define MAX_LINE_LEN        1024
#define MAX_LOG_FILE_SIZE   4096		// 按名称生成日志的最大容量(1M)

// 日志等级
#define LEVEL_INFO           1				
#define LEVEL_CRITICAL       2
#define LEVEL_ERROR          3

#define FRM_MAX_LOGFILENAME_LENGTH        255          //最大文件名长度


#if (defined VIRTUAL_DEVICE) || (defined _WIN32)
#define LOGCFG_NAME	"./config/LocalConfig.xml"
#else
#define LOGCFG_NAME	"/usr/config/LocalConfig.xml"
#endif

/*=====================================================================
类名 ：COpenATCLog
功能 ：通信类
主要接口：long WriteLog：写日志文件
备注 ：
--------------------------------------------------------------------------------------------------------------------
修改记录：
日 期           版本    修改人             走读人             修改记录
2019/09/06      V1.0    李永萍             李永萍             创建类
====================================================================*/

#ifdef _WIN32
    #ifdef CommonLog_EXPORTS
    class _declspec(dllexport) COpenATCLog
    #else
    class _declspec(dllimport) COpenATCLog
    #endif
#else
    class COpenATCLog
#endif
{
public:
	COpenATCLog();
	virtual ~COpenATCLog();
	
	/****************************************************
	函数名：SetLogFileName
    功能：设置日志文件名
	算法实现:
    参数说明 ： szLogFilePath，日志文件路径
	            szLogFileName，日志文件名称
    返回值说明：OPENATC_RTN_OK，连接成功
	            OPENATC_RTN_FAILED，连接失败
    --------------------------------------------------------------------------------------------------------------------
	修改记录：
	日 期           版本    修改人             走读人             修改记录
	2019/09/06      V1.0    李永萍             李永萍             创建
	====================================================================*/
	int    SetLogFileName(const char *szLogFilePath, const char *szLogFileName);

	/****************************************************
	函数名：LogOneMessage
    功能：记录一条日志
	算法实现:
    参数说明 ： nLogLevel，日志等级
	            szFormat，日志文件格式
    返回值说明：无
    --------------------------------------------------------------------------------------------------------------------
	修改记录：
	日 期           版本    修改人             走读人             修改记录
	2019/09/06      V1.0    李永萍             李永萍             创建
	====================================================================*/
	void    LogOneMessage(int nLogLevel, const char *szFormat, ...);

	/****************************************************
	函数名：CreateNewLogFile
    功能：创建新的日志文件
	算法实现:
    参数说明 ： 无
    返回值说明：无
    --------------------------------------------------------------------------------------------------------------------
	修改记录：
	日 期           版本    修改人             走读人             修改记录
	2019/09/06      V1.0    李永萍             李永萍             创建
	====================================================================*/
	void     CreateNewLogFile();		
	
	/****************************************************
	函数名：ReadConfig
    功能：读取日志配置信息
	算法实现:
    参数说明 ： 无
    返回值说明：ATC_OK，操作成功
	            ATC_FAIL，操作失败
    --------------------------------------------------------------------------------------------------------------------
	修改记录：
	日 期           版本    修改人             走读人             修改记录
	2019/09/06      V1.0    李永萍             李永萍             创建
	====================================================================*/
	int 	ReadConfig();

	/****************************************************
	函数名：SetConfig
    功能：设置配置文件名
	算法实现:
    参数说明 ： szPath，配置文件名
    返回值说明：无
    --------------------------------------------------------------------------------------------------------------------
	修改记录：
	日 期           版本    修改人             走读人             修改记录
	2019/09/06      V1.0    李永萍             李永萍             创建
	====================================================================*/
	void      SetConfig(char szXmlFile[]);

	/****************************************************
	函数名：ReadScreenLanguage
    功能：获取显示屏显示语言
	算法实现:
    参数说明 ： 
    返回值说明：无
    --------------------------------------------------------------------------------------------------------------------
	修改记录：
	日 期           版本    修改人             走读人             修改记录
	2019/09/06      V1.0    李永萍             李永萍             创建
	====================================================================*/
	int      ReadScreenLanguage();

private:
	/****************************************************
	函数名：LevelToMap
    功能：设置日志级别
	算法实现:
    参数说明 ： nLevel，日志等级
    返回值说明：日志级别
    --------------------------------------------------------------------------------------------------------------------
	修改记录：
	日 期           版本    修改人             走读人             修改记录
	2019/09/06      V1.0    李永萍             李永萍             创建
	====================================================================*/
	int     LevelToMap(int nLevel);

	/****************************************************
	函数名：ChangeLevelString
    功能：转换日志级别字符到自定义日志级别
	算法实现:
    参数说明 ： szLogLevelStr，日志等级
    返回值说明：日志级别
    --------------------------------------------------------------------------------------------------------------------
	修改记录：
	日 期           版本    修改人             走读人             修改记录
	2019/09/06      V1.0    李永萍             李永萍             创建
	====================================================================*/
	int		ChangeLevelString(const char *szLogLevelStr);
    
    /****************************************************
	函数名：GetCurrLogFileName
    功能：获取当前日志文件名
	算法实现:
    参数说明 ： 无
    返回值说明：当前日志文件名
    --------------------------------------------------------------------------------------------------------------------
	修改记录：
	日 期           版本    修改人             走读人             修改记录
	2019/09/06      V1.0    李永萍             李永萍             创建
	====================================================================*/
	char        *GetCurrLogFileName();

private:
	unsigned  int   m_dwLogLevel;		 //存储日志级别

	ofstream *      m_pOutPutStream;     //输出流指针
	size_t          m_nFileMaxSize;      //日志文件最大大小 超过该大小将被删除 默认为10MB
	int				m_nFileMaxNum;		 //记录的日志文件数量
	bool		    m_bPrintToScreen;	 //是否显示
	int				m_nLanguageType;

	char			m_szLogFilePath[FRM_MAX_LOGFILENAME_LENGTH];			//日志文件所在目录
	char			m_szLogFileName[FRM_MAX_LOGFILENAME_LENGTH];			//日志类别的程序名 
	char			m_szCurrLogFileName[FRM_MAX_LOGFILENAME_LENGTH];	    //当前日志文件名称

    char            m_szXmlFile[FRM_MAX_LOGFILENAME_LENGTH];                //日志等级配置文件名

	std::mutex		m_mutex;	// 保护m_pOutPutStream

};

#endif //!ifndef OPENATCLOG_H
