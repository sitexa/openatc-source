/*=====================================================================
ģ���� ��Ӳ����Դ��ش���ģ��
�ļ��� ��OpenATCHardwareManager.h
����ļ���OpenATCParameter.h OpenATCRunStatus.h
ʵ�ֹ��ܣ�Ӳ����Դ��ش���ģ������ࡣ
���� ��������
��Ȩ ��<Copyright(c) 2019-2020 Suzhou Keda Technology Co.,Ltd. All right reserved.>
------------------------------------------------------------------------
�޸ļ�¼��
�� ��           �汾     �޸���     �߶���     �޸ļ�¼
2019/9/26       V1.0     ������     ������      ����ģ��
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

#define     FRM_MAX_FILENAME_LENGTH             255                //����ļ�����
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
	int                      m_nMainBoardFaultCount;								//���ذ����й�������
	TRunFaultInfo            m_tMainBoardFaultInfo[C_N_MAX_FAULT_COUNT];			//���й���
	int                      m_nFaultBoardFaultCount;								//���ϼ������й�������
	TRunFaultInfo            m_tFaultBoardFaultInfo[C_N_MAX_FAULT_COUNT];			//���й���
	int                      m_nLampBoardFaultCount;								//�ƿذ����й�������
	TRunFaultInfo            m_tLampBoardFaultInfo[C_N_MAX_FAULT_COUNT];			//���й���
	int                      m_nVehDetBoardFaultCount;								//��������й�������
	TRunFaultInfo            m_tVehDetBoardFaultInfo[C_N_MAX_FAULT_COUNT];			//���й���
	int                      m_nIOBoardFaultCount;									//IO�����й�������
	TRunFaultInfo            m_tIOBoardFaultInfo[C_N_MAX_FAULT_COUNT];				//���й���
	int                      m_nOtherFaultCount;									//������������
	TRunFaultInfo            m_tOtherFaultInfo[C_N_MAX_FAULT_COUNT];				//��������
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
    //�ඨ��Ϊ����
    static COpenATCHardwareManager * getInstance();

    //��ĳ�ʼ������
    void Init(COpenATCParameter * pParameter, COpenATCRunStatus * pRunStatus, COpenATCLog * pOpenATCLog, const char * pSoftwareVer, int nLanguageType);

    //���������
    void Work();

    //���ֹͣ���ͷ�
    void Stop();

    void HardwareInit();

    //��ȡ��ַ���ַ
    int ReadSiteID();

    //��ȡ��ַ��汾
    int ReadSiteRev(int nArray[]);

    //��ȡӲ���汾
    int ReadBomVersion();

	//��ȡ��״̬
    void ReadDoorStatus(bool & bFrontDoorOpen, bool & bBackDoorOpen);

    //������̵���״̬
	bool CheckRelayStatus();

	//��������
	void NetConfig(COpenATCParameter * pParameter, char * ipAddr0, char * ipAddr1);

	HARDWAREMANAGERHANDLE  GetHandle();
	
	virtual int PanelSignalRun();

	virtual int LcdScreenShowRun();

	//��ʾ����ʾ����
	void ProcCtlLcdScreen();

	/// ��ʾ�źŻ��Լ���Ϣ��led��Ļ
	void ShowSelfDetectInfoInLedScreen(const char * strPasteInfo, unsigned int startX, unsigned int startY, int nTextSize);
	void ShowSelfDetectFaultInfoInLedScreen(COpenATCRunStatus * pRunStatus, TSelfDetectInfo & tSelfDetectInfo, unsigned int startX, unsigned int startY, int nTextSize, int nLanguage);

protected:
    //���̵�������
    bool CtlMainRelay(int nEnable);

    //��������źŴ���
	void ProcCtlPanelSignal();

private:
	COpenATCHardwareManager();
	~COpenATCHardwareManager();

    static COpenATCHardwareManager * s_pData;              //������ָ��

    COpenATCParameter * m_pLogicCtlParam;                   //����������ָ��
    COpenATCRunStatus * m_pLogicCtlStatus;                  //����״̬��ָ��
	COpenATCLog       * m_pOpenATCLog;                      //��־��ָ��

    CHardwareInterface* m_cHardwareInterface;                //Ӳ�������Դ������

    int m_anPanelSignal[5];                                  //�ֱ���ȫ��,����,�ֶ�,����,�Զ�5����ť��״̬

    TAscNetCard m_atNetConfig[MAX_NETCARD_TABLE_COUNT];      //����������Ϣ

	HARDWAREMANAGERHANDLE         m_hPanelSignalThread;
    unsigned long				  m_dwPanelSignalThreadRet;
	static void *HARDWAREMANAGER_CALLBACK HardwarePanelSignalReceiveAndSendThread(void *pParam);

	HARDWAREMANAGERHANDLE         m_hLcdScreenShowThread;
	unsigned long				  m_dwLcdScreenShowThreadRet;
	static void *HARDWAREMANAGER_CALLBACK HardwareLcdScreenShowThread(void *pParam);
	
	/// ���ʹ������ݰ�
    void                ParserPack(const char* packBuffer, const int packLength);

	/// ����ָʾ��״̬
	bool				SetLedStatus(int nLedNo, int nMode);

	/// ����ָʾ��
	void				CheckAndControlLedStatus();

	/// �򿪲����ô��ڲ���
	int					OpenAndSetSerialParam(int & nFd, SERIAL_INFO serialInfo);

	/// ������������ݸ�ָʾ�ư�
	int					PackAndSendInformationsToIndicatorBoard(TOpenATCStatusInfo tRunStatus);

	/// ��Ļ���ȿ��ƣ���������
	void				ControlLedScreen(int on_off);

	/// ����ͼƬ������ʾ��led��Ļ
	void				LoadPicAndShowInLedScreen(bool bFlag, int nTextSize);

	///��ʱ
	void                OpenATCSleep(long nMsec);

	///����������ȡ���Ʒ�ʽ
	const char *		GetControlModeByIndex(int nCtlMode, int nSubCtlMode, int nLanguageType);
	
	/// ����ʪ�ȴ���������ͨ��
	void				CommWithSensor();

	void ModbusCrc16(unsigned char *ptr,unsigned int len, unsigned char *pRet);

	void				PasteFaultInfoToLcdScreen(unsigned int nStartX, int nTextSize, bool bFlag);

	const char *		GetRunCommonFaultInfoDescribe(int nFaultType, int nSubFaultType, char * cValue, int nLanguage);

	void				SortFaultInfo();
	
	bool				get_cpuoccupy (CPU_OCCUPY *cpust);
	int					cal_cpuoccupy (CPU_OCCUPY *o, CPU_OCCUPY *n);
	enum
    {
        /// ��Ϣ���еĳ�ʱʱ��
        WAIT_MESSAGE_TIME			= 200,
			
		/// ���ڽ��ջ�������С
		SERIAL_RECV_BUFF_SIZE		= 100,
		
		/// ���ڷ��ͻ�������С
		SERIAL_SEND_BUFF_SIZE		= 150,
		
		/// ����ԭʼ����������С
		SERIAL_PACK_BUFF_SIZE		= 150,

		/// ��ʱʱ��
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

	/// ������
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

    bool                m_bManualBtn;//�ֶ���ť���±�־

};

#endif //ifndef OPENATCHARDWAREMANAGER_H

/*=============================================
Ӳ��������,��һ�����ӵ�Ӳ������㣬�������Ϳ��ƽ�ͨ�źŻ��ĸ���Ӳ���豸��
ͷ�ļ�����Ҫ�ṹ�͹��ܣ�

1. ��Ҫ����ģ�飺
- Ӳ����ʼ���Ϳ���
- �豸״̬���
- LED��ʾ����
- ����ͨ�Ź���
- ��������
- ���ϼ��ʹ���
- ��ʪ�ȴ�����ͨ��
- LCD��Ļ��ʾ����

2.�ؼ����ݽṹ��
struct FAULT_INFO_SORTED    // ������Ϣ����
struct CPU_OCCUPY          // CPUռ����Ϣ
struct SERIAL_INFO         // ����������Ϣ
struct TEMPANDHUM_INFO     // ��ʪ����Ϣ

3. ��Ҫ���ܽӿڣ�
- �豸��ʼ��������
- ״̬��غ͹��ϴ���
- ͨ�Žӿڹ���
- ��ʾ����
- ϵͳ������ȡ

4. Ӳ���������ԣ�
- LEDָʾ�ƿ���
- �̵�������
- ����ͨ��
- �Ž�״̬���
- ��ʪ�ȼ��
- LCD����ʾ

5. ��ȫ���ԣ�
- ���ϼ��ʹ���
- ״̬���
- Ӳ���Լ�
- ��־��¼

===============================================*/