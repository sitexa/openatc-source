#pragma once

#ifdef _WIN32
#include <stdio.h>
//#include <winsock2.h>
//#include <WS2tcpip.h>
#include <Windows.h>
#include <process.h>
#include <string.h>
#include <time.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <dlfcn.h>
#include <semaphore.h>
#include <time.h>
#include <string>
#include <string.h>
// #include <malloc.h>
#include <iostream>
#include <cstring>
#include <errno.h>
#include <stdint.h> 
#include <cctype>
#include <algorithm>
#include <stdarg.h>
#include <pthread.h>
#include <sys/syscall.h>
#pragma comment (lib, "pthreadVC2.lib")
#endif

/**
 *  ����Linux���źŴ�����غ�
 */
#ifndef _WIN32
// #include <sys/prctl.h>
#include <signal.h>
#define SIGNAL(SignalID, SignalHandler)    signal((SignalID), (SignalHandler))
#define SIG_SEGMENT                        SIGSEGV 
#define SIG_ALARM						   SIGALRM
#define ALARM(Interval)                    alarm((Interval))
#define SHELL_MOVE		                   "mv"
#define SHELL_DELETE                       "rm -f"
#else 
#define SIGNAL(SignalID, SignalHandler)
#define SIG_SEGMENT
#define SIG_ALARM							 
#define ALARM(Interval)  
#define SHELL_MOVE		                  "move"
#define SHELL_DELETE                      "del"
#endif

#if _MSC_VER
#define snprintf _snprintf
#endif

#ifdef _WIN32
typedef int                 socklen_t;
typedef unsigned long       DWORD;
#else
typedef unsigned char       BYTE;
typedef unsigned short      WORD;
typedef int                 SOCKET;
typedef struct sockaddr     SOCKADDR;
typedef struct sockaddr_in  SOCKADDR_IN;

#define     INVALID_SOCKET                      -1
#define     SOCKET_ERROR                        -1
#define     FD_READ  	                        0x01
#define     FD_WRITE 	                        0x02
#define     FD_READWRITE                        0x03
#define     FD_NONE                             0x04
#endif

/////////////////////////////////////////////////////////////////////////////////
#define     FRM_MAX_FILENAME_LENGTH             255					//����ļ�����
#define     FRM_MAX_COMMAND_LENGTH              1024				//��������

/////////////////////////////////////////////////////////////////////////////////

#define     OPENATC_RTN_TIMEOUT                  -2
#define     UDP_CLIENT                           0
#define     UDP_SERVICE                          1
#define     TCP_CLIENT                           2
#define     TCP_SERVICE                          3

#define     CFG_SERVICE_LISTERN                  0
#define     CAMERA_SERVICE_LISTERN               1

/////////////////////////////////////////////////////////////////////////////////
const unsigned char      LINK_COM                   = 0x01;			//ͨ�Ź����·
const unsigned char      LINK_BASEINFO              = 0x02;			//������Ϣ��·
const unsigned char      LINK_ALTERNATEFEATUREPARA  = 0x03;			//��������һ�㽻����·
const unsigned char      LINK_INTERVENECOMMAND      = 0x04;			//��Ԥָ����·
const unsigned char      LINK_CONTROL               = 0x05;			//�����������������·
const unsigned char      LINK_CONFIG                = 0x06;			//����������Ĳ�����·
const unsigned char      LINK_OPTIMIZE_CONTROL      = 0x07;			//���Ż�ģ���������·

/////////////////////////////////////////////////////////////////////////////////
const unsigned char	     CTL_ONLINEMACHINE          = 0x01;			//����
const unsigned char	     CTL_TRAFFICFLOWINFO        = 0x02;			//��ͨ����Ϣ
const unsigned char	     CTL_WORKSTATUS             = 0x03;			//����״̬
const unsigned char      CTL_LAMPCOLORSTATUS        = 0x04;			//��ɫ״̬
const unsigned char	     CTL_CURRENTTIME            = 0x05;			//��ǰʱ��
const unsigned char	     CTL_SIGNALLIGHTGROUP       = 0x06;			//�źŵ����ѯ
const unsigned char	     CTL_PHASE                  = 0x07;			//��λ
const unsigned char	     CTL_SIGNALMATCHTIME        = 0x08;			//�ź���ʱ����
const unsigned char	     CTL_PROGRAMMESCHEDULEPLAN  = 0x09;			//�������ȼƻ�
const unsigned char	     CTL_WORKWAY                = 0x0A;			//������ʽ
const unsigned char		 CTL_FAULT                  = 0x0B;			//����
const unsigned char	     CTL_VERSION                = 0x0C;			//�汾
const unsigned char	     CTL_FEATUREPARAVERSION     = 0x0D;			//���������汾
const unsigned char	     CTL_INDENTIFYCODE          = 0x0E;			//ʶ����
const unsigned char		 CTL_REMOTECONTROL          = 0x0F;			//Զ�̿���
const unsigned char		 CTL_DETECTOR               = 0x10;			//�����
const unsigned char		 CTL_OVERLAP                = 0x11;			//������λ
const unsigned char		 CTL_SCHEDUL_DATE           = 0x12;			//����
const unsigned char		 CTL_HEART                  = 0xA1;			//����    
const unsigned char      CTL_UDISK                  = 0xA5;			//U��     
const unsigned char      CTL_SYSTEM_REMOTE          = 0xA6;			//ϵͳԶ�̵���  
const unsigned char      CTL_OPERATION_RECORD       = 0xA7;			//������¼��־  
const unsigned char      CTL_CHANNEL_CHECK          = 0xA8;			//ͨ���ɼ��
const unsigned char      CTL_VOLUME_LOG				= 0xA9;			//������־
const unsigned char		 CTL_PATTERN_INTERRUPT		= 0xAA;         //������Ԥ
const unsigned char		 CTL_CHANNEL_STATUS_INFO	= 0xAB;			//ͨ��״̬��Ϣ
const unsigned char      CTL_CHANNEK_LAMP_STATUS    = 0xAC;			//ͨ����ɫ״̬
const unsigned char      CTL_SYSTEM_CUSTOM			= 0xAD;			//�豸����
const unsigned char      CTL_SYSTEM_UPDATE			= 0xAF;			//FTP�ļ�����
const unsigned char      CTL_DETECTOR_STATUS        = 0xB0;         //�����״̬
const unsigned char      CTL_UPDATESECRETKEY_STATUS = 0xC0;         //�źŻ�������Կ����Ӧ�Ĳ���������0

const unsigned char      CFG_ASK_ASKSEND            = 0x17;			//�����������������
const unsigned char      CFG_ACK_ASKSEND            = 0x18;			//������Ӧ�����������������
const unsigned char      CFG_ASK_ASKREAD            = 0x19;			//����������������
const unsigned char      CFG_ACK_ASKREAD_OK         = 0x20;			//������Ӧ���������������ݳɹ�
const unsigned char      CFG_ACK_ASKREAD_FAILED     = 0x21;			//������Ӧ����������������ʧ��
const unsigned char      CFG_ASK_SENDDATA           = 0x22;			//���������������
const unsigned char      CFG_ACK_SENDDATA_OK        = 0x23;			//��������������ݳɹ�
const unsigned char      CFG_ACK_SENDDATA_FAILED    = 0x24;			//���������������ʧ��
const unsigned char      CFG_ACK_SENDDTA_END        = 0x25;			//��������������ݽ���

const unsigned char      CTL_ASK_LOGIN              = 0x50;			//������������½
const unsigned char      CTL_ACK_LOGIN              = 0x51;			//������Ӧ������������½
const unsigned char      CTL_HEART_BERAT			= 0x52;         //������
const unsigned char      CTL_ASK_VERSION            = 0x60;			//�����������汾
const unsigned char      CTL_ACK_VERSION            = 0x61;			//������Ӧ�����������汾
const unsigned char      CTL_ASK_UPDATEFILE         = 0x51;			//���������������ļ�
const unsigned char      CTL_ACK_UPDATEFILE         = 0x53;			//������Ӧ���������������ļ�
const unsigned char      CTL_ASK_SENDFILEBLOCK      = 0x54;			//�����������ȡ���ļ�����
const unsigned char      CTL_ASK_CANCELSENDFILE     = 0x62;			//������Ӧ�����������ȡ���ļ�����
const unsigned char      CTL_ACKED_SENDFILEEND      = 0x58;			//������������ļ��������ȷ��
const unsigned char      CTL_ASK_STARTUPDATE        = 0x55;			//������Ӧ������������ļ��������ȷ��
const unsigned char      CTL_ASK_UPDATEONE		    = 0x56;			//������������ļ��������
const unsigned char      CTL_ASK_ADJUSTTIME         = 0x12;			//������������ʱ
const unsigned char      CTL_ACK_ASKTIME	        = 0x13;			//������Ӧ������������ʱ
const unsigned char      CTL_ASK_REBOOT             = 0x14;			//���������������
const unsigned char      CTL_ASK_TIME	            = 0x18;			//�����������ʱ��

const unsigned char      CTL_TRAFFICFLOW_INFO       = 0x02;			//��ͨ����Ϣ
const unsigned char      CTL_REALDETECTOR_INFO      = 0x51;			//������ʵʱ�����Ϣ
const unsigned char      CTL_VEHILCEQUEUE_INFO      = 0x52;		    //�����Ŷ���Ϣ
const unsigned char      CTL_PEDDETECTOR_INFO       = 0x53;		    //���˼����Ϣ
const unsigned char      CTL_DETECTORFAULT_INFO     = 0x54;		    //����������״̬��Ϣ

const unsigned char      CTL_PREEMPT_INFO           = 0x5B;		    //������Ϣ

const unsigned char      CB_VERSION_FLAG            = 0x10;			//�汾��
const unsigned char      CB_ATC_FLAG                = 0x10;			//�źŻ���ݱ�ʶ
const unsigned char      CB_CONFIGSOFTWARE_FLAG     = 0x20;			//���շ���ʶ
const unsigned char      CB_FLOWCOLLECTDEVICE_FLAG  = 0x80;			//�����ɼ��豸��ݱ�ʶ
const unsigned char      CB_TRAFFICSIMULATE_FLAG    = 0x81;			//��ͨ������ݱ�ʶ
const unsigned char      CB_AIDEVICE_FLAG           = 0x82;			//AI�ſ���ݱ�ʶ

const unsigned char      CB_CONTENT_CHARACETER      = 0x01;         //�ֽ���
const unsigned char      CB_CONTENT_JSON            = 0x02;         //JSON
const unsigned char      CB_CONTENT_XML             = 0x03;         //XML

////////////////////////////////////////////////////////////////////////////////////
const unsigned char      LINKCODE_POS               = 3;
const unsigned char      CMDCODE_POS                = 8;
const unsigned char      DATALEN_POS                = 10;
const unsigned char      DATACONTENT_POS            = 14;

///////////////////////////////////////////////////////////////////////////////////
const unsigned char      ASK_QUERY                  = 0x80;
const unsigned char      ACK_QUERY                  = 0x83;
const unsigned char      ASK_SET                    = 0x81;
const unsigned char      ACK_SET                    = 0x84;
const unsigned char      REPORT_STATUS              = 0x82;
const unsigned char      ACK_WRONG                  = 0x85;

enum
{
    COM_WITH_CENTER      = 1,
    COM_WITH_SIMULATE    = 2,
	COM_WITH_ITS300      = 3,
};

///////////////////////////////////////////////////////////////////////////////////
//GB20999
typedef enum tagComType_GB20999
{
	COM_TCP = 1,							//TCP
	COM_UDP = 2,							//UDP
	COM_RS232 = 3,							//RS232
}EComType_GB20999;

typedef enum tagFrameType_GB20999
{
	FRAME_TYPE_QUERY = 0x10,				//��ѯ
	FRAME_TYPE_QUERY_REPLY = 0x20,			//��ѯӦ��
	FRAME_TYPE_QUERY_ERRORREPLY = 0x21,		//��ѯ����ظ�
	FRAME_TYPE_SET = 0x30,					//����
	FRAME_TYPE_SET_REPLY = 0x40,			//����Ӧ��
	FRAME_TYPE_SET_ERRORREPLY = 0x41,		//���ó���ظ�
	FRAME_TYPE_BROADCAST = 0x50,			//�㲥
	FRAME_TYPE_TRAP = 0x60,					//�����ϱ�
	FRAME_TYPE_HEART_QUERY = 0x70,			//������ѯ
	FRAME_TYPE_HEART_REPLY = 0x80,			//����Ӧ��
}EFrameType_GB20999;

typedef enum tagBadValue_GB20999
{
	BAD_VALUE_STATUS = 0x10,				//ֵ���� badValue
	BAD_VALUE_WRONGLENGTH = 0x11,			//ֵ���ȴ���
	BAD_VALUE_OVERFLOW = 0x12,				//ֵԽ��
	BAD_VALUE_READONLY = 0x20,				//ֵֻ��
	BAD_VALUE_NULL = 0x30,					//ֵ������
	BAD_VALUE_ERROR = 0x40,					//ֵһ�����
	BAD_VALUE_CONTROLFAIL = 0x50,			//����ʧ��
}EBadValue_GB20999;

typedef enum tagLightType_GB20999
{
	LIGHT_TYPE_VEHICLE = 0x01,				//����������
	LIGHT_TYPE_NONVEHICLE = 0x02,			//�ǻ���������
	LIGHT_TYPE_PEDESTRIAN = 0x03,			//���˵���
	LIGHT_TYPE_ROAD = 0x04,					//��������

}ELightType_GB20999;

typedef enum tagLightStatus_GB20999
{
	LIGHT_STATUS_OFF = 0x01,				//���
	LIGHT_STATUS_RED = 0x10,				//���
	LIGHT_STATUS_REDFLASH = 0x11,			//����
	LIGHT_STATUS_REDFASTFLASH = 0x12,		//�����
	LIGHT_STATUS_GREEN = 0x20,				//�̵�
	LIGHT_STATUS_GREENFLASH = 0x21,			//����
	LIGHT_STATUS_GREENFASTFLASH = 0x22,		//�̿���
	LIGHT_STATUS_YELLOW = 0x30,				//�Ƶ�
	LIGHT_STATUS_YELLOWFLASH = 0x31,		//����
	LIGHT_STATUS_YELLOWFASTFLASH = 0x32,	//�ƿ���
	LIGHT_STATUS_REDYELLOW = 0x40,			//��Ƶ�
}ELightStatus_GB20999;

typedef enum tagDetectorType_GB20999
{
	DETECTOR_TYPE_COIL = 0x01,				//��Ȧ
	DETECTOR_TYPE_VIDEO = 0x02,				//��Ƶ
	DETECTOR_TYPE_GEOMAGNETIC = 0x03,		//�ش�
	DETECTOR_TYPE_MICROWAVE = 0x04,			//΢�������
	DETECTOR_TYPE_ULTRASONIC = 0x05,		//�����������
	DETECTOR_TYPE_INFRARED = 0x06,			//��������
}EDetectorType_GB20999;

typedef enum tagPhaseStageType_GB20999
{
	PHASE_STAGE_TYPE_FIX = 0x10,			//��λ�׶ι̶�����
	PHASE_STAGE_TYPE_DEMAND = 0x20,			//��λ�׶ΰ��������
}EPhaseStageType_GB20999;

typedef enum tagPhaseStageStatus_GB20999
{
	PHASE_STAGE_STATUS_NOTOFWAY = 0x10,		//��λ�׶�δ����
	PHASE_STAGE_STATUS_ONTHEWAY = 0x20,		//��λ�׶����ڷ���
	PHASE_STAGE_STATUS_TRANSITON = 0x30,	//��λ�׶ι���
}EPhaseStageStatus_GB20999;

typedef enum tagControlMode_GB20999
{
	MODE_CENTER_Control = 0x10,				//���Ŀ���ģʽ
	MODE_LOCAL_Control = 0x20,				//���ؿ���ģʽ
	MODE_SPECIAL_Control = 0x30,			//�������
}EControlMode_GB20999;

typedef enum tagCenterControlMode_GB20999
{
	MODE_CENTER_TIMETABLE_CONTROL = 0x11,   //�����ռƻ�����
	MODE_CENTER_OPTIMIZATION_CONTROL = 0x12,//�����Ż�����
	MODE_CENTER_COORDINATION_CONTROL = 0x13,//����Э������
	MODE_CENTER_ADAPTIVE_CONTROL = 0x14,	//��������Ӧ����
	MODE_CENTER_MANUAL_CONTROL = 0x15,		//�����ֶ�����
}ECenterControlMode_GB20999;

typedef enum tagLocalControlMode_GB20999
{
	MODE_LOCAL_FIXCYCLE_CONTROL = 0x21,		//���ض����ڿ���
	MODE_LOCAL_VA_CONTROL = 0x22,			//���ظ�Ӧ����
	MODE_LOCAL_COORDINATION_CONTROL = 0x23, //����Э������
	MODE_LOCAL_ADAPTIVE_CONTROL = 0x24,		//��������Ӧ����
	MODE_LOCAL_MANUAL_CONTROL = 0x25,		//�����ֶ�����
}ELocalControlMode_GB20999;

typedef enum tagSpecialControlMode_GB20999
{
	MODE_SPECIAL_FLASH_CONTROL = 0x31,		//��������
	MODE_SPECIAL_ALLRED_CONTROL = 0x32,		//ȫ�����
	MODE_SPECIAL_ALLOFF_CONTROL = 0x33,		//�صƿ���
}ESpecialControlMode_GB20999;

typedef enum tagAlarmType_GB20999
{
	TYPE_ALARM_LIGHT = 0x10,				//�źŵƱ���
	TYPE_ALARM_DETECTOR = 0x30,				//���������
	TYPE_ALARM_DEVICE = 0x40,				//�豸���ϱ���
	TYPE_ALARM_ENVIRONMENT = 0x60,			//���������쳣����
}EAlarmType_GB20999;

typedef enum tagFaultType_GB20999
{
	TYPE_FAULT_GREENCONFLICT = 0x10,		//�̳�ͻ����
	TYPE_FAULT_GREENREDCONFLICT = 0x11,		//���̳�ͻ����
	TYPE_FAULT_REDLIGHT = 0x20,				//��ƹ���
	TYPE_FAULT_YELLOWLIGHT = 0x21,			//�Ƶƹ���
	TYPE_FAULT_COMMUNICATION = 0x30,		//ͨ�Ź���
	TYPE_FAULT_SELF = 0x40,					//�Լ����
	TYPE_FAULT_DETECTOR = 0x41,				//���������
	TYPE_FAULT_RELAY = 0x42,				//�̵�������
	TYPE_FAULT_MEMORY = 0x43,				//�洢������
	TYPE_FAULT_CLOCK = 0x44,				//ʱ�ӹ���
	TYPE_FAULT_MOTHERBOARD = 0x45,			//�������
	TYPE_FAULT_PHASEBOARD = 0x46,			//��λ�����
	TYPE_FAULT_DETECTORBOARD = 0x47,		//�������
	TYPE_FAULT_CONFIG = 0x50,				//���ù���
	TYPE_FAULT_RESPONSE = 0x70,				//������Ӧ����
}EFaultType_GB20999;

typedef enum tagSwitchOperation_GB20999
{
	SWITCH_NULL = 0x00,						//�޹��϶���
	SWITCH_TO_FLASH = 0x10,					//�л�������
	SWITCH_TO_OFF = 0x20,					//�л������
	SWITCH_TO_RED = 0x30,					//�л���ȫ��
	SWITCH_TO_LOCAL_FIXCYCLE = 0x40,		//�л������ض�����
	SWITCH_TO_COORDINATION = 0x50,			//�л�������Э��
}ESwitchOperation_GB20999;

typedef enum tagOrderValue_GB20999
{
	ORDER_FLASH = 0x01,						//����
	ORDER_RED = 0x02,						//ȫ��
	ORDER_ON = 0x03,						//����
	ORDER_OFF = 0x04,						//�ص�
	ORDER_RESET = 0x05,						//����
	ORDER_CANCEL = 0x00,					//ȡ������
}EOrderValue_GB20999;

typedef enum tagDataType_GB20999
{
	DEVICE_INFO = 1,						//�豸��Ϣ
	BASE_INFO = 2,							//������Ϣ
	LIGHTGROUP_INFO = 3,					//������Ϣ
	PHASE_INFO = 4,							//��λ��Ϣ
	DETECTOR_INFO = 5,						//�������Ϣ
	PHASESTAGE_INFO = 6,					//��λ�׶���Ϣ
	PHASESAFETY_INFO = 7,					//��λ��ȫ��Ϣ
	EMERGENCY_PRIORITY = 8,					//��������
	PATTERN_INFO = 9,						//������Ϣ
	TRANSITION_RETRAIN = 10,				//����Լ��
	DAY_PLAN = 11,							//�ռƻ�
	SCHEDULE_TABLE = 12,					//���ȱ�
	RUN_STATUS = 13,						//����״̬
	TRAFFIC_DATA = 14,						//��ͨ����
	ALARM_DATA = 15,						//��������
	FAULT_DATA = 16,						//��������
	CENTER_CONTROL = 17,					//���Ŀ���
	ORDER_PIPE = 18,						//����ܵ�
	PRIVATE_DATE = 128						//˽��������
}EDataType_GB20999;

typedef enum tagDeviceInfo_GB20999
{
	MANUFACTURER = 1,						//���쳧��
	DEVICE_VERSION = 2,						//�豸�汾
	DEVICE_ID = 3,							//�豸���
	MANUFACTUR_DATE = 4,					//��������
	CONFIG_DATE = 5,						//��������
}EDeviceInfo_GB20999;

typedef enum tagBaseInfo_GB20999
{
	INSTALLATION_ROAD = 1,					//��װ·��
	ATC_IPV4_NETCONFIG = 2,					//�źŻ�IPV4��������
	HOST_IPV4_NETCONFIG = 3,				//��λ��IPV4��������
	ATC_TIMEZONE = 4,						//�źŻ�����ʱ��
	ATC_ID = 5,								//�źŻ�ID
	ATC_ROADCOUNT = 6,						//�źŻ�����·������
	GPS_CLOCK_FLAG = 7,						//GPSʱ�ӱ�־
	ATC_IPV6_NETCONFIG = 8,					//�źŻ�IPV6��������
	HOST_IPV6_NETCONFIG = 9,				//��λ��IPV6��������
}EBaseInfo_GB20999;

typedef enum tagATCIPV4NetConfig_GB20999
{
	ATC_IP_ADDRESS = 1,						//IP��ַ
	SUB_NET = 2,							//��������
	GATE_WAY = 3,							//����
}EATCIPV4NetConfig_GB20999;

typedef enum tagHostIPV4NetConfig_GB20999
{
	HOST_IP_ADDRESS = 1,					//IP��ַ
	COM_PORT = 2,							//ͨ�Ŷ˿�
	COM_TYPE = 3,							//ͨ������
}EHostIPV4NetConfig_GB20999;

typedef enum tagLightGroupInfo_GB20999
{
	LIGHT_GROUP_COUNT = 1,					//ʵ�ʵ�����
	LIGHT_GROUP_CONFIG_TABLE = 2,			//�������ñ�
	LIGHT_GROUP_STATUS_TABLE = 3,			//����״̬��
	LIGHT_GROUP_CONTROL_TABLE = 4,			//������Ʊ�
}ELightGroupInfo_GB20999;

typedef enum tagLightGroupConfigTable_GB20999
{
	LIGHT_GROUP_TYPE_ID = 1,				//������
	LIGHT_GROUP_TYPE = 2,					//��������
}ELightGroupConfigTable_GB20999;

typedef enum tagLightGroupStatusTable_GB20999
{
	LIGHT_GROUP_STATUS_ID = 1,				//������
	LIGHT_GROUP_STATUS = 2,					//����״̬
}ELightGroupStatusTable_GB20999;

typedef enum tagControlTable_GB20999
{
	CONTROL_ID = 1,							//���
	CONTROL_SHIELD = 2,						//����
	CONTROL_PROHIBIT = 3,					//��ֹ
}EControlTable_GB20999;

typedef enum tagLightGroupType_GB20999
{
	LIGHT_GROUP_VEHICLE = 1,				//������
	LIGHT_GROUP_NOVEHICLE = 2,				//�ǻ�����
	LIGHT_GROUP_PED = 3,					//����
	LIGHT_GROUP_ROAD = 4,					//����
	LIGHT_GROUP_ALTERABLE_TRAFFIC = 5,		//�ɱ佻ͨ��־
	LIGHT_GROUP_BUS = 6,					//����ר�õƾ�
	LIGHT_GROUP_TRAM = 7,					//�й�糵ר�õƾ�
	LIGHT_GROUP_SPECIALBUS = 8,				//���⹫��
}ELightGroupType_GB20999;

typedef enum tagPhaseInfo_GB20999
{
	PHASE_COUNT = 1,						//ʵ����λ��
	PHASE_CONFIG_TABLE = 2,					//��λ���ñ�
	PHASE_CONTROL_TABLE = 3,				//��λ���Ʊ�
}EPhaseInfo_GB20999;

typedef enum tagPhaseConfigTable_GB20999
{
	PHASE_CONFIG_ID = 1,					//��λ���
	PHASE_LIGHTGROUP = 2,					//��λ�ĵ���
	PHASE_LOSETRANSITIONTYPE1 = 3,			//ʧȥ·Ȩ���ɵ�ɫ����1
	PHASE_LOSETRANSITIONTIME1 = 4,			//ʧȥ·Ȩ���ɵ�ɫʱ��1
	PHASE_LOSETRANSITIONTYPE2 = 5,			//ʧȥ·Ȩ���ɵ�ɫ����2
	PHASE_LOSETRANSITIONTIME2 = 6,			//ʧȥ·Ȩ���ɵ�ɫʱ��2
	PHASE_LOSETRANSITIONTYPE3 = 7,			//ʧȥ·Ȩ���ɵ�ɫ����3
	PHASE_LOSETRANSITIONTIME3 = 8,			//ʧȥ·Ȩ���ɵ�ɫʱ��3
	PHASE_GETTRANSITIONTYPE1 = 9,			//���·Ȩ���ɵ�ɫ����1
	PHASE_GETTRANSITIONTIME1 = 10,			//��ȡ·Ȩ���ɵ�ɫʱ��1
	PHASE_GETTRANSITIONTYPE2 = 11,			//���·Ȩ���ɵ�ɫ����2
	PHASE_GETTRANSITIONTIME2 = 12,			//��ȡ·Ȩ���ɵ�ɫʱ��2
	PHASE_GETTRANSITIONTYPE3 = 13,			//���·Ȩ���ɵ�ɫ����3
	PHASE_GETTRANSITIONTIME3 = 14,			//��ȡ·Ȩ���ɵ�ɫʱ��3
	PHASE_TURNONGETTRANSITIONTYPE1 = 15,	//�������·Ȩ���ɵ�ɫ����1
	PHASE_TURNONGETTRANSITIONTIME1 = 16,	//�������·Ȩ���ɵ�ɫʱ��1
	PHASE_TURNONGETTRANSITIONTYPE2 = 17,	//�������·Ȩ���ɵ�ɫ����2
	PHASE_TURNONGETTRANSITIONTIME2 = 18,	//�������·Ȩ���ɵ�ɫʱ��2
	PHASE_TURNONGETTRANSITIONTYPE3 = 19,	//�������·Ȩ���ɵ�ɫ����3
	PHASE_TURNONGETTRANSITIONTIME3 = 20,	//�������·Ȩ���ɵ�ɫʱ��3
	PHASE_TURNONLOSETRANSITIONTYPE1 = 21,	//����ʧȥ·Ȩ���ɵ�ɫ����1
	PHASE_TURNONLOSETRANSITIONTIME1 = 22,	//����ʧȥ·Ȩ���ɵ�ɫʱ��1
	PHASE_TURNONLOSETRANSITIONTYPE2 = 23,	//����ʧȥ·Ȩ���ɵ�ɫ����1
	PHASE_TURNONLOSETRANSITIONTIME2 = 24,	//����ʧȥ·Ȩ���ɵ�ɫʱ��2
	PHASE_TURNONLOSETRANSITIONTYPE3 = 25,	//����ʧȥ·Ȩ���ɵ�ɫ����3
	PHASE_TURNONLOSETRANSITIONTIME3 = 26,	//����ʧȥ·Ȩ���ɵ�ɫʱ��3
	PHASE_MIN_GREENTIME = 27,				//��С��ʱ��
	PHASE_MAX1_GREENTIME = 28,				//�����ʱ��1
	PHASE_MAX2_GREENTIME = 29,				//�����ʱ��2
	PHASE_PASSAGE_GREENTIME = 30,			//�ӳ���ʱ��
	PHASE_CALL = 31,						//��λ������
}EPhaseConfigTable_GB20999;

typedef enum tagPhaseControlTable_GB20999
{
	PHASE_CONTROL_ID = 1,					//��λID
	PHASE_CONTROL_SHIELD = 2,				//��λ����
	PHASE_CONTROL_PROHIBIT = 3,				//��λ��ֹ
}EPhaseControlTable_GB20999;

typedef enum tagDetectorInfo_GB20999
{
	DETECTOR_COUNT = 1,						//ʵ�ʼ������
	DETECTOR_CONFIG_TABLE = 2,				//��������ñ�
	DETECTOR_DATA_TABLE = 3,				//��������ݱ�
}EDetectorInfo_GB20999;

typedef enum tagDetectorConfigTable_GB20999
{
	DETECTOR_ID_CONFIG = 1,					//��������
	DETECTOR_TYPE = 2,						//���������
	FLOW_CYCLE = 3,							//�����ɼ�����
	OCCUPY_CYCLE = 4,						//ռ���ʲɼ�����
	INSTALL_POS = 5,						//��װλ��
}EDetectorConfigTable_GB20999;

typedef enum tagDetectorDataTable_GB20999
{
	DETECTOR_ID_DATA = 1,					//��������
	EXIST_STATUS = 2,						//�������������״̬
	VEHICLE_SPEED = 3,						//����������ٶ�
	VEHICLE_TYPE = 4,						//��⵽�ĳ�������
	VEHICLE_PLATE = 5,						//��⵽�ĳ�������
	QUEUE_LENGTH = 6,						//���ڳ����Ŷӳ���
}EDetectorDataTable_GB20999;

typedef enum tagVehiclType_GB20999
{
	VEHICLE_TYPE_UNDEF = 0,					//�޳�
	VEHICLE_TYPE_SMALL = 1,					//С�ͳ�
	VEHICLE_TYPE_MIDDLE = 2,				//���ͳ�
	VEHICLE_TYPE_LARGE = 3,					//���ͳ�
	VEHICLE_TYPE_BUS = 4,					//������
	VEHICLE_TYPE_TRAMCAR = 5,				//�й�糵
	VEHICLE_TYPE_SPEICAL = 6,				//���ֳ���
}EVehiclType_GB20999;

typedef enum tagPhaseStageInfo_GB20999
{
	PHASE_STAGE_COUNT = 1,					//ʵ����λ�׶���
	PHASE_STAGE_CONFIG_TABLE = 2,			//��λ�׶����ñ�
	PHASE_STAGE_STATUS_TABLE = 3,			//��λ�׶�״̬��
	PHASE_STAGE_CONTROL_TABLE = 4,			//��λ�׶ο��Ʊ�
}EPhaseStageInfo_GB20999;

typedef enum tagPhaseStageConfigTable_GB20999
{
	PHASE_STAGE_ID_CONFIG = 1,				//���
	PHASE_STAGE_PHASE = 2,					//��λ
	PHASE_STAGE_LATE_START = 3,				//������ʱ��
	PHASE_STAGE_EARLY_END = 4,				//�����ʱ��
}EPhaseStageConfigTable_GB20999;

typedef enum tagPhaseStageStatusTable_GB20999
{
	PHASE_STAGE_ID_STATUS = 1,				//���
	PHASE_STAGE_STATUS = 2,					//״̬
	PHASE_STAGE_RUN_TIME = 3,				//����ʱ��
	PHASE_STAGE_REMAIN_TIME = 4,			//ʣ��ʱ��
}EPhaseStageStatusTable_GB20999;

typedef enum tagPhaseStageControlTable_GB21999
{
	PHASE_STAGE_ID_CONTROL = 1,				//���
	PHASE_STAGE_SOFTWARECALL = 2,			//�������
	PHASE_STAGE_SHIELD = 3,					//��λ�׶�����
	PHASE_STAGE_PROHIBIT = 4,				//��λ�׶ν�ֹ
}EPhaseStageControlTable_GB21999;

typedef enum tagPhaseSafetyInfo_GB20999
{
	PHASE_CONFLICT_TABLE = 1,				//��λ��ͻ���ñ�
	PHASE_GREENINTERVAL_TABLE = 2,			//��λ�̼�����ñ�
}EPhaseSafetyInfo_GB20999;

typedef enum tagPhaseConflictTable_GB21999
{
	PHASE_ID_CONFLICT = 1,					//���
	PHASE_CONFLICT_ARRAY = 2,				//��ͻ��λ����
}EPhaseConflictTable_GB21999;

typedef enum tagPhaseGreenIntervalTable_GB21999
{
	PHASE_ID_GREENINTERVAL = 1,				//���
	PHASE_GREENINTERVAL_ARRAY = 2,			//��λ�̼������
}EPhaseGreenIntervalTable_GB21999;

typedef enum tagEmergencyPriority_GB20999
{
	PRIORITY_COUNT = 1,						//ʵ����������
	PRIORITY_CONFIG_TABLE = 2,				//�������ñ�
	PRIORITY_STATUS_TABLE = 3,				//����״̬��
	EMERGENCY_COUNT = 4,					//��������
	EMERGENCY_CONFIG_TABLE = 5,				//�������ñ�
	EMERGENCY_STATUS_TABLE = 6,				//����״̬��
}EEmergencyPriority_GB20999;

typedef enum tagPriorityConfig_GB20999
{
	PRIORITY_ID_CONFIG = 1,					//�����źű��
	PRIORITY_APPLY_PHASESTAGE = 2,			//�����ź�������λ�׶�
	PRIORITY_APPLY_GRADE = 3,				//�����ź��������ȼ�
	PRIORITY_SHIELD = 4,					//�����ź����α�־
	PRIORITY_SOURCE = 5,					//�����ź���Դ(��������)
}EPriorityConfig_GB20999;

typedef enum tagPriorityStatus_GB20999
{
	PRIORITY_ID_STATUS = 1,					//�����źű��
	PRIORITY_APPLY_STATUS = 2,				//�����ź�����״̬
	PRIORITY_EXECUTE_STATUS = 3,			//�����ź�ִ��״̬
}EPriorityStatus_GB20999;

typedef enum tagEmergencyConfig_GB20999
{
	EMERGENCY_ID_CONFIG = 1,				//�����źű��
	EMERGENCY_APPLY_PHASESTAGE = 2,			//�����ź�������λ�׶�
	EMERGENCY_APPLY_GRADE = 3,				//�����ź��������ȼ�
	EMERGENCY_SHIELD = 4,					//�����ź����α�־
	EMERGENCY_SOURCE = 5,					//�����ź���Դ(��������)
}EEmergencyConfig_GB20999;

typedef enum tagEmergencyStatus_GB20999
{
	EMERGENCY_ID_STATUS = 1,				//�����źű��
	EMERGENCY_APPLY_STATUS = 2,				//�����ź�����״̬
	EMERGENCY_EXECUTE_STATUS = 3,			//�����ź�ִ��״̬
}EEmergencyStatus_GB20999;

typedef enum tagPatternInfo_GB20999
{
	PATTERN_COUNT = 1,						//��������
	PATTERN_CONFIGTABLE = 2,				//�������ñ�
}EPatternInfo_GB20999;

typedef enum tagPatternConfigTable_GB20999
{
	PATTERN_ID = 1,							//�������
	PATTERN_ROADID = 2,						//·�����
	PATTERN_CYCLELEN = 3,					//��������
	PATTERN_ADJUST_STAGEID = 4,				//Э�����
	PATTERN_OFFSET = 5,						//��λ��ʱ��
	PATTERN_STAGE_CHAIN = 6,				//�׶���
	PATTERN_STAGE_CHAINTIME = 7,			//�׶���ʱ��
	PATTERN_STAGE_TYPE = 8,					//�׶�����
}EPatternConfigTable_GB20999;

typedef enum tagTransitionRetain_GB20999
{
	PHASE_TRANSITIONSTAGE_ID = 1,			//��λ�׶α��               
	PHASE_TRANSITIONSTAGE_RETAIN = 2,		//��λ�׶ι���Լ��ֵ   
}ETransitionRetain_GB20999;

typedef enum tagDayPlanInfo_GB20999
{
	DAYPLAN_COUNT = 1,						//ʵ���ռƻ�����               
	DAYPLAN_CONFIG = 2,						//�ռƻ�����   
}EDayPlanInfo_GB20999;

typedef enum tagDayPlanConfigTable_GB20999
{
	DAYPLAN_ID = 1,							//�ռƻ����               
	DAYPLAN_ROADID = 2,						//·�����   
	DAYPLAN_STARTTIMECHAIN = 3,				//��ʼʱ����   
	DAYPLAN_PATTERNCHAIN = 4,				//ִ�з�����   
	DAYPLAN_RUNMODECHAIN = 5,				//����ģʽ��   
	DAYPLAN_TIMESPANACTCHAIN1 = 6,			//ʱ�ζ�����1   
	DAYPLAN_TIMESPANACTCHAIN2 = 7,			//ʱ�ζ�����2  
	DAYPLAN_TIMESPANACTCHAIN3 = 8,			//ʱ�ζ�����3  
	DAYPLAN_TIMESPANACTCHAIN4 = 9,			//ʱ�ζ�����4  
	DAYPLAN_TIMESPANACTCHAIN5 = 10,			//ʱ�ζ�����5  
	DAYPLAN_TIMESPANACTCHAIN6 = 11,			//ʱ�ζ�����6  
	DAYPLAN_TIMESPANACTCHAIN7 = 12,			//ʱ�ζ�����7  
	DAYPLAN_TIMESPANACTCHAIN8 = 13,			//ʱ�ζ�����8  
}EDayPlanConfigTable_GB20999;

typedef enum tagScheduleTable_GB20999
{
	SCHEDULE_COUNT = 1,						//���ȱ�����              
	SCHEDULE_CONFIG_TABLE = 2,				//���ȱ����ñ�   
}EDayScheduleTable_GB20999;

typedef enum tagScheduleConfigTable_GB20999
{
	SCHEDULE_ID = 1,						//���ȱ���               
	SCHEDULE_ROADID = 2,					//·�����   
	SCHEDULE_PRIORITY = 3,					//���ȼ�   
	SCHEDULE_WEEK = 4,						//����ֵ   
	SCHEDULE_MONTH = 5,						//�·�ֵ   
	SCHEDULE_DAY = 6,						//����ֵ   
	SCHEDULE_DAYPLAN_ID = 7,				//�ռƻ����
}EDayScheduleeConfigTable_GB20999;

typedef enum tagRunStatus_GB20999
{
	DEVICE_STATUS = 1,						//�豸״̬               
	CONTROL_STATUS = 2,						//����״̬   
}ERunStatus_GB20999;

typedef enum tagDeviceStatus_GB20999
{
	DETECTOR_STATUS = 1,					//�����״̬               
	DEVICEMODULE_STATUS = 2,				//�豸ģ��״̬   
	ATC_DOOR_STATUS = 3,					//����״̬
	VOLTAGE_VALUE = 4,						//��ѹֵ
	CURRENT_VALUE = 5,						//����ֵ
	TEMPERATURE_VALUE = 6,					//�¶�ֵ
	HUMIDITY_VALUE = 7,						//ʪ��ֵ
	WATERLOGIN_VALUE = 8,					//ˮ��ֵ
	SMOKE_VALUE = 9,						//����ֵ
	STANDARD_TIME = 10,						//��׼ʱ��
	LOCAL_TIME = 11,						//����ʱ��
}EDeviceStatus_GB20999;

typedef enum tagControlStatus_GB20999
{
	CONTROL_ROADID = 1,						//·�����               
	ROAD_RUNMODE = 2,						//·������ģʽ   
	ROAD_PATTERN = 3,						//·�ڵ�ǰ����   
	ROAD_STAGE = 4,							//·�ڵ�ǰ�׶�   
}EControlStatus_GB20999;

typedef enum tagTrafficData_GB20999
{
	REALTIME_DATA = 1,						//ʵʱ����               
	STATISTIC_DATA = 2,						//ͳ�����ݱ�   
}ETrafficData_GB20999;

typedef enum tagStatisticData_GB20999
{
	DETECTOR_ID_STATISTIC = 1,				//��������               
	DETECTOR_FLOW = 2,						//��������� 
	DETECTOR_OCCUPY = 3,					//�����ռ���� 
	AVERAGE_SPEED = 4,						//ƽ������
}EStatisticData_GB20999;

typedef enum tagAlarmData_GB20999
{
	ALARM_COUNT = 1,						//��������               
	ALARMDATA_TABLE = 2,					//�������ݱ�   
}EAlarmData_GB20999;

typedef enum tagAlarmDataTable_GB20999
{
	ALARM_ID = 1,							//�������            
	ALARM_TYPE = 2,							//��������
	ALARM_VALUE = 3,						//����ֵ
	ALARM_TIME = 4,							//����ʱ��
}EAlarmDataTable_GB20999;

typedef enum tagFaultData_GB20999
{
	FAULT_COUNT = 1,						//��������               
	FAULT_RECORD = 2,						//���ϼ�¼��   
}EFaultData_GB20999;

typedef enum tagFaultRecordTable_GB20999
{
	FAULT_ID = 1,							//���ϼ�¼���            
	FAULT_TYPE = 2,							//��������
	FAULT_TIME = 3,							//����ʱ��
	FAULT_PARAM = 4,						//���϶�������
}EFaultRecordDataTable_GB20999;

typedef enum tagCenterControlTable_GB20999
{
	CENTERCONTROL_ROADID = 1,				//·�����               
	CENTERCONTROL_PHASESTAGE = 2,			//ָ����λ�׶�   
	CENTERCONTROL_PATTERN = 3,				//ָ������  
	CENTERCONTROL_RUNMODE = 4,				//ָ������ģʽ
}ECenterControlTable_GB20999;