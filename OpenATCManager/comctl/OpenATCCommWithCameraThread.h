/*=====================================================================
模块名 ：和相机交互的线程模块
文件名 OpenATCCommWithCameraThread.h
相关文件：OpenATCDataPackUnpack.h OpenATCComDef.h
实现功能：和相机的交互
作者 ：李永萍
版权 ：<Copyright(c) 2019-2020 Suzhou Keda Technology Co.,Ltd. All right reserved.>
--------------------------------------------------------------------------------------------------------------------
修改记录：
日 期           版本    修改人             走读人             修改记录
2019/09/06      V1.0    李永萍）           李永萍             创建模块
====================================================================*/

#ifndef OPENATCCOMMWITHCAMERATHREAD_H
#define OPENATCCOMMWITHCAMERATHREAD_H

#include "../../Include/OpenATCParameter.h"
#include "../../Include/OpenATCRunStatus.h"
#include "../../Include/CanBusManager.h"
#include "OpenATCCameraDataPackUnpack.h"
#include "OpenATCCfgCommHelper.h"
#include "OpenATCComDef.h"

class COpenATCSocket;

#ifdef _WIN32
	#define COMMWITHCAMERA_CALLBACK WINAPI
	typedef HANDLE               COMMWITHCAMERAHANDLE;
#else
	#define COMMWITHCAMERA_CALLBACK
	typedef pthread_t            COMMWITHCAMERAHANDLE;
#endif

class COpenATCParameter;
class COpenATCRunStatus;

/*=====================================================================
类名 ：COpenATCCommWithCameraThread
功能 ：和相机的交互
主要接口：void Init：初始化参数，第一个参数为配置参数，第二个参数为状态
备注 ：
--------------------------------------------------------------------------------------------------------------------
修改记录：
日 期           版本    修改人             走读人             修改记录
2019/09/06      V1.0    李永萍             李永萍             创建类
====================================================================*/
class COpenATCCommWithCameraThread
{
public:
    COpenATCCommWithCameraThread();
    virtual ~COpenATCCommWithCameraThread();

    virtual int Run();

	/****************************************************
	函数名：Init
    功能：初始化参数
	算法实现:
    参数说明 ： pParameter，参数
	            pRunStatus，状态
    返回值说明：无
    --------------------------------------------------------------------------------------------------------------------
	修改记录：
	日 期           版本    修改人             走读人             修改记录
	2019/09/06      V1.0    李永萍             李永萍             创建
	====================================================================*/
	void        Init(COpenATCParameter *pParameter, COpenATCRunStatus *pRunStatus, COpenATCLog * pOpenATCLog);

    int         Start();
    int         Join();
    int         Detach();

	/****************************************************
	函数名：SetClientSocket
    功能：设置相机信息
	算法实现:
    参数说明 ： clientSock，相机socket
    返回值说明：无
    --------------------------------------------------------------------------------------------------------------------
	修改记录：
	日 期           版本    修改人             走读人             修改记录
	2019/09/06      V1.0    李永萍             李永萍             创建
	====================================================================*/
	void        SetClientSocket(SOCKET & socket);

	/****************************************************
	函数名：SetCameraInfo
    功能：设置相机信息
	算法实现:
    参数说明 ： chCameraIp，相机IP
	            nCameraPort，相机端口
    返回值说明：无
    --------------------------------------------------------------------------------------------------------------------
	修改记录：
	日 期           版本    修改人             走读人             修改记录
	2019/09/06      V1.0    李永萍             李永萍             创建
	====================================================================*/
	void        SetCameraInfo(char *chCameraIp, int nCameraPort);

	/****************************************************
	函数名：GetCameraInfo
    功能：设置相机信息
	算法实现:
    参数说明 ： 无
    返回值说明：相机IP
    --------------------------------------------------------------------------------------------------------------------
	修改记录：
	日 期           版本    修改人             走读人             修改记录
	2019/09/06      V1.0    李永萍             李永萍             创建
	====================================================================*/
	char *     GetCameraInfo();

private:
	enum
	{
		MAX_HEARTDATA_SIZE      = 4,
		MAX_LAMPCLRDATA_SIZE    = 16,
		MAX_DATA_SIZE           = 19,

		HEART_INTERVAL_TIME     = 5,

		PACK_HEART_HEAD         = 0xfd,
		PACK_LAMPCLR_HEAD       = 0xfe,
		PACK_TAIL               = 0x55,

		MAX_HEART_INDEX         = 255,
	};
    static void *COMMWITHCAMERA_CALLBACK RunThread(void *pParam);

	void            SendLampClrDataToCamera();

	void            SendHeartToCamera();

	void            OpenATCSleep(long nMsec);

	//CAN通信发送的灯组状态分解成红灯黄灯绿灯状态
	void			GetRYGStatusByGroup(char chGroup,char & chR,char & chY,char & chG);

    COMMWITHCAMERAHANDLE             m_hThread;
    unsigned long                    m_dwThreadRet;
	bool                             m_bExitFlag;
    int                              m_nDetachState;

	COpenATCParameter                *m_pOpenATCParameter;

	COpenATCRunStatus                *m_pOpenATCRunStatus;

	COpenATCLog                      *m_pOpenATCLog;

	int                              m_nSendTimeOut;

	int                              m_nRecvTimeOut;
    
	COpenATCPackUnpackBase           *m_pDataPackUnpackMode;

	//从socket中接收的数据缓冲区.
	unsigned char                    m_chRecvBuff[MAX_HEARTDATA_SIZE + 1];

	//解包后协议数据的缓冲区.
	unsigned char                    m_chUnPackedBuff[MAX_HEARTDATA_SIZE + 1];

	unsigned char                    m_chOldSendBuff[MAX_LAMPCLRDATA_SIZE + 1];

	short                            m_sCameraCmdType;

	char	                         m_szCameraIp[20];

    int                              m_nCameraPort;

	bool                             m_bConnectStatus;

	unsigned char                    m_chHeartIndex;

	time_t                           m_lastSendHeartTime;

	time_t                           m_lastReadOkTime;

	COpenATCSocket                   m_clientSock;
          
};

#endif // !ifndef OPENATCCOMMWITHCAMERATHREAD_H
