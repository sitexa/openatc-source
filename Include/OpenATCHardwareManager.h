/*=====================================================================
模块名 ：硬件资源相关处理模块
文件名 ：OpenATCHardwareManager.h
相关文件：OpenATCParameter.h OpenATCRunStatus.h
实现功能：硬件资源相关处理模块调度类。
作者 ：刘黎明
版权 ：<Copyright(c) 2019-2020 Suzhou Keda Technology Co.,Ltd. All right reserved.>
------------------------------------------------------------------------
修改记录：
日 期           版本     修改人     走读人     修改记录
2019/9/26       V1.0     刘黎明     刘黎明      创建模块
=====================================================================*/

#ifndef OPENATCHARDWAREMANAGER_H
#define OPENATCHARDWAREMANAGER_H

#include "../Include/OpenATCParameter.h"
#include "../Include/OpenATCRunStatus.h"
#include "../Include/OpenATCRunStatusDefine.h"
#include <vector>

#ifdef _WIN32
#include <windows.h>
#else
/* Standard Include Files */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include "mxcfb.h"
// #include <linux/fb.h>
#include <sys/mman.h>
#include <math.h>
#include <string.h>
// #include <malloc.h>
#endif

#include "PackUnpackBase.h"

#ifdef _WIN32
#define HARDWAREMANAGER_CALLBACK WINAPI
typedef HANDLE               HARDWAREMANAGERHANDLE;
#else
#define HARDWAREMANAGER_CALLBACK
typedef pthread_t            HARDWAREMANAGERHANDLE;
#endif

#define     FRM_MAX_FILENAME_LENGTH             255                //最大文件长度
#define     FRM_MAX_TRIGGERSTATUS_COUNTER       3
#define     FRM_MAX_LCDSCREEN_WIDTH		        800
#define     FRM_MAX_LCDSCREEN_HEIGHT	        480

// control button list

enum
{
	LED_ERR		= 0,
	LED_RUN		= 1,
	LED_GPS		= 2,
	LED_CAN1	= 3,
	LED_CAN2	= 4,
};

enum
{
	LED_STATUS_OFF				= 0,
	LED_STATUS_ON				= 1,
	LED_STATUS_FLICKER_FAST		= 2,
	LED_STATUS_FLICKER_NORMAL	= 3,
	LED_STATUS_FLICKER_SLOW		= 4,
};

enum
{
	SOURCE_MANUAL_PANEL	= 0,
	SOURCE_GPS			= 1,
	SOURCE_REMOTE_CONTROL = 4,
};

enum
{
	READ_YELLOWFALSH_COUNTER		= 1,
	MAX_BRDHWMON_DEVIVE				= 11,
    YELLOWFALSHRUNSTATUS_DWNO		= 8,
    YELLOWFALSHTRIGGERSTATUS_DWNO	= 9,
    LEDSCREEN_DWNO					= 12,
};

enum
{
	LEDSCREEN_OFF	= 0,
	LEDSCREEN_ON	= 100,
};

enum
{
	SERIAL_COM1	= 1,
	SERIAL_COM2	= 2,
	SERIAL_COM3	= 3,
	SERIAL_COM4	= 4,
	SERIAL_COM5	= 5,
};

enum
{
	SERIAL_BANDRATE_9600	= 9600,
	SERIAL_BANDRATE_115200	= 115200,
};

enum
{
	SERIAL_DATABIT_5	= 5,
	SERIAL_DATABIT_6	= 6,
	SERIAL_DATABIT_7	= 7,
	SERIAL_DATABIT_8	= 8,
};

enum
{
	SERIAL_PARITY_NONE	= 0,
	SERIAL_PARITY_ODD	= 1,
	SERIAL_PARITY_EVEN	= 2,
};

enum
{
	SERIAL_STOPBIT_1	= 0,
	SERIAL_STOPBIT_2	= 1,
	SERIAL_STOPBIT_3	= 2,
};

enum
{
	TEXT_SIZE_1	= 1,
	TEXT_SIZE_2	= 2,
	TEXT_SIZE_3	= 3,
	TEXT_SIZE_4	= 4,
	TEXT_SIZE_5	= 5,
};

enum
{
	LANGUAGE_CHINESE	= 0,
	LANGUAGE_ENGLISH	= 1,
};

typedef struct
{	
	int nCom;
	int nBandrate;
	int nStopBit;
	int nDataBit;
	int nParity;
	int nTimeOut;
} SERIAL_INFO;  

typedef struct
{	
	float fTemp;
	float fHum;
} TEMPANDHUM_INFO; 

typedef struct
{	
	int                      m_nMainBoardFaultCount;								//主控板运行故障数量
	TRunFaultInfo            m_tMainBoardFaultInfo[C_N_MAX_FAULT_COUNT];			//运行故障
	int                      m_nFaultBoardFaultCount;								//故障检测板运行故障数量
	TRunFaultInfo            m_tFaultBoardFaultInfo[C_N_MAX_FAULT_COUNT];			//运行故障
	int                      m_nLampBoardFaultCount;								//灯控板运行故障数量
	TRunFaultInfo            m_tLampBoardFaultInfo[C_N_MAX_FAULT_COUNT];			//运行故障
	int                      m_nVehDetBoardFaultCount;								//车检板运行故障数量
	TRunFaultInfo            m_tVehDetBoardFaultInfo[C_N_MAX_FAULT_COUNT];			//运行故障
	int                      m_nIOBoardFaultCount;									//IO板运行故障数量
	TRunFaultInfo            m_tIOBoardFaultInfo[C_N_MAX_FAULT_COUNT];				//运行故障
	int                      m_nOtherFaultCount;									//其他故障数量
	TRunFaultInfo            m_tOtherFaultInfo[C_N_MAX_FAULT_COUNT];				//其他故障
} FAULT_INFO_SORTED; 

typedef struct tagCpuOccupy
{
	char name[20];
	unsigned int user;
	unsigned int nice;
	unsigned int system;
	unsigned int idle;
	unsigned int iowait;
	unsigned int irq;
	unsigned int softirq;
	unsigned int reserve;
}CPU_OCCUPY;

class CHardwareInterface;

#ifdef _WIN32
    #ifdef DLL_FILE
    class _declspec(dllexport) COpenATCHardwareManager
    #else
    class _declspec(dllimport) COpenATCHardwareManager
    #endif
#else
    class COpenATCHardwareManager
#endif
{
public:
    //类定义为单件
    static COpenATCHardwareManager * getInstance();

    //类的初始化操作
    void Init(COpenATCParameter * pParameter, COpenATCRunStatus * pRunStatus, COpenATCLog * pOpenATCLog, const char * pSoftwareVer, int nLanguageType);

    //类的主流程
    void Work();

    //类的停止与释放
    void Stop();

    void HardwareInit();

    //读取地址板地址
    int ReadSiteID();

    //读取地址板版本
    int ReadSiteRev(int nArray[]);

    //读取硬件版本
    int ReadBomVersion();

	//读取门状态
    void ReadDoorStatus(bool & bFrontDoorOpen, bool & bBackDoorOpen);

    //检查主继电器状态
	bool CheckRelayStatus();

	//网络设置
	void NetConfig(COpenATCParameter * pParameter, char * ipAddr0, char * ipAddr1);

	HARDWAREMANAGERHANDLE  GetHandle();
	
	virtual int PanelSignalRun();

	virtual int LcdScreenShowRun();

	//显示屏显示处理
	void ProcCtlLcdScreen();

	/// 显示信号机自检信息到led屏幕
	void ShowSelfDetectInfoInLedScreen(const char * strPasteInfo, unsigned int startX, unsigned int startY, int nTextSize);
	void ShowSelfDetectFaultInfoInLedScreen(COpenATCRunStatus * pRunStatus, TSelfDetectInfo & tSelfDetectInfo, unsigned int startX, unsigned int startY, int nTextSize, int nLanguage);

protected:
    //主继电器控制
    bool CtlMainRelay(int nEnable);

    //控制面板信号处理
	void ProcCtlPanelSignal();

private:
	COpenATCHardwareManager();
	~COpenATCHardwareManager();

    static COpenATCHardwareManager * s_pData;              //单件类指针

    COpenATCParameter * m_pLogicCtlParam;                   //特征参数类指针
    COpenATCRunStatus * m_pLogicCtlStatus;                  //运行状态类指针
	COpenATCLog       * m_pOpenATCLog;                      //日志类指针

    CHardwareInterface* m_cHardwareInterface;                //硬件相关资源处理类

    int m_anPanelSignal[5];                                  //分别是全红,步进,手动,黄闪,自动5个按钮的状态

    TAscNetCard m_atNetConfig[MAX_NETCARD_TABLE_COUNT];      //网络配置信息

	HARDWAREMANAGERHANDLE         m_hPanelSignalThread;
    unsigned long				  m_dwPanelSignalThreadRet;
	static void *HARDWAREMANAGER_CALLBACK HardwarePanelSignalReceiveAndSendThread(void *pParam);

	HARDWAREMANAGERHANDLE         m_hLcdScreenShowThread;
	unsigned long				  m_dwLcdScreenShowThreadRet;
	static void *HARDWAREMANAGER_CALLBACK HardwareLcdScreenShowThread(void *pParam);
	
	/// 解释串口数据包
    void                ParserPack(const char* packBuffer, const int packLength);

	/// 设置指示灯状态
	bool				SetLedStatus(int nLedNo, int nMode);

	/// 控制指示灯
	void				CheckAndControlLedStatus();

	/// 打开并设置串口参数
	int					OpenAndSetSerialParam(int & nFd, SERIAL_INFO serialInfo);

	/// 打包并发送数据给指示灯板
	int					PackAndSendInformationsToIndicatorBoard(TOpenATCStatusInfo tRunStatus);

	/// 屏幕亮度控制（开关屏）
	void				ControlLedScreen(int on_off);

	/// 加载图片，并显示到led屏幕
	void				LoadPicAndShowInLedScreen(bool bFlag, int nTextSize);

	///延时
	void                OpenATCSleep(long nMsec);

	///根据索引获取控制方式
	const char *		GetControlModeByIndex(int nCtlMode, int nSubCtlMode, int nLanguageType);
	
	/// 与温湿度传感器进行通信
	void				CommWithSensor();

	void ModbusCrc16(unsigned char *ptr,unsigned int len, unsigned char *pRet);

	void				PasteFaultInfoToLcdScreen(unsigned int nStartX, int nTextSize, bool bFlag);

	const char *		GetRunCommonFaultInfoDescribe(int nFaultType, int nSubFaultType, char * cValue, int nLanguage);

	void				SortFaultInfo();
	
	bool				get_cpuoccupy (CPU_OCCUPY *cpust);
	int					cal_cpuoccupy (CPU_OCCUPY *o, CPU_OCCUPY *n);
	enum
    {
        /// 消息队列的超时时间
        WAIT_MESSAGE_TIME			= 200,
			
		/// 串口接收缓冲区大小
		SERIAL_RECV_BUFF_SIZE		= 100,
		
		/// 串口发送缓冲区大小
		SERIAL_SEND_BUFF_SIZE		= 150,
		
		/// 串口原始包缓冲区大小
		SERIAL_PACK_BUFF_SIZE		= 150,

		/// 超时时长
		HARDWARE_TIME_OUT			= 2,

		HARDWARE_SCREEN_TIME_OUT	= 1,

		HARDWARE_GPS_TIME_OUT		= 10,

		HARDWARE_CAN_TIME_OUT		= 1,
		
		HARDWARE_TIME_OUT_TIMES		= 3,

		SOFTWARE_VERSION_SIZE		= 128,

		JAVA_HEART_TIME_OUT         = 20,

		HARDWARE_RECVDAT_TIME_OUT	= 20,
    };

	char		serialRecvBuff_[SERIAL_RECV_BUFF_SIZE];
    char		serialUnPackBuff_[SERIAL_PACK_BUFF_SIZE];
	char		serialSendBuff_[SERIAL_SEND_BUFF_SIZE];
    char		serialPackBuff_[SERIAL_PACK_BUFF_SIZE];
	char        serialSensorRecvBuff_[SERIAL_RECV_BUFF_SIZE];
    char		serialSensorUnPackBuff_[SERIAL_PACK_BUFF_SIZE];

	/// 打包解包
    CPackUnpackBase*    BoardPackUnpacker_;
    CPackUnpackBase*    SensorPackUnpacker_;

	long				m_nCAN1LastTime;
	int					m_nCAN1Count;
	bool				m_bCAN1LedOn;

	long				m_nCAN2LastTime;
	int					m_nCAN2Count;
	bool				m_bCAN2LedOn;
	
	long				m_nGPSLastTime;
	bool				m_bGPSLedOn;
	
	long				m_nHWPanelLastTime;

	THWPanelBtnStatus	m_lastPanelBtnStatus;

    long                m_nYellowFlashLastTime;

#ifdef _WIN32
	HANDLE				m_hGPSAndHWPanelCom;
#endif
	int					m_serialGPSAndHWPanel_fd;
	int					m_serialIndicatorBoard_fd;
	int					m_serialTemAndHumSensor_fd;

	TLedScreenShowInfo	m_tScreenShowInfo;
    long                m_nCommWithSensorLastTime;

	TOpenATCStatusInfo	m_tLastOpenATCStatusInfo;
	
	TGpsData			m_tGpsData;
	TEMPANDHUM_INFO		m_tTempAndHum;

	char				m_cSoftwareVer[SOFTWARE_VERSION_SIZE];

	int					m_nLanguageType;

	char				m_cSysFaultInfoLine1[1024 * 1024];
	char				m_cSysFaultInfoLine2[1024 * 1024];
	
	string				m_strSysFaultInfoLine1;
	string				m_strSysFaultInfoLine2;

	vector<string>   	m_strSysSelfDetectInfo;

	TLedScreenShowInfo	m_tOldLedScreenShowFaultInfo;

	FAULT_INFO_SORTED	m_tFaultInfoSorted;

	bool				m_bIfFrontDoorOpen;
	bool				m_bLastFrontDoorOpenStatus;
	bool				m_bIfBackDoorOpen;

    bool                m_bManualBtn;//手动按钮按下标志

};

#endif //ifndef OPENATCHARDWAREMANAGER_H

/*=============================================
硬件管理器,是一个复杂的硬件抽象层，负责管理和控制交通信号机的各种硬件设备。
头文件的主要结构和功能：

1. 主要功能模块：
- 硬件初始化和控制
- 设备状态监控
- LED显示控制
- 串口通信管理
- 网络配置
- 故障检测和处理
- 温湿度传感器通信
- LCD屏幕显示管理

2.关键数据结构：
struct FAULT_INFO_SORTED    // 故障信息分类
struct CPU_OCCUPY          // CPU占用信息
struct SERIAL_INFO         // 串口配置信息
struct TEMPANDHUM_INFO     // 温湿度信息

3. 重要功能接口：
- 设备初始化和配置
- 状态监控和故障处理
- 通信接口管理
- 显示控制
- 系统参数读取

4. 硬件控制特性：
- LED指示灯控制
- 继电器控制
- 串口通信
- 门禁状态监控
- 温湿度监控
- LCD屏显示

5. 安全特性：
- 故障检测和处理
- 状态监控
- 硬件自检
- 日志记录

===============================================*/