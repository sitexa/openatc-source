#pragma once

#include "../Include/OpenATCParamConstDefine.h"
#include "../Include/OpenATCParamStructDefine.h"

const int C_N_MAXLAMPOUTPUT_NUM		= 240;
const int C_N_MAXRING_NUM			= 4;
const int C_N_MAXLAMPBOARD_NUM		= 20;
const int C_N_MAXIOBOARD_NUM		= 4;
const int C_N_MAXCANBUS_NUM			= 2;
const int C_N_MAXDETBOARD_NUM		= 4;
const int C_N_MAXDETINPUT_NUM		= 16;
const int C_N_MAXIOINPUT_NUM		= 16;
const int C_N_LAMPBORAD_OUTPUTNUM	= 12;
const int C_N_CHANNEL_OUTPUTNUM		= 3;
const int C_N_CHANNELNUM_PER_BOARD	= 4;
const int C_N_MAX_FUNC_CARDSLOT_NUM = 15;
const int C_N_MAX_CTRL_CARDSLOT_NUM = 6;
const int C_N_MAX_SCREEN_RING_NUM	= 4;

const char C_CH_PHASESTAGE_RY	= 'a';									//���ͬ��
const char C_CH_PHASESTAGE_G	= 'g';									//��
const char C_CH_PHASESTAGE_GF	= 'h';									//����
const char C_CH_PHASESTAGE_Y	= 'y';									//��
const char C_CH_PHASESTAGE_YF	= 'z';									//����
const char C_CH_PHASESTAGE_R	= 'r';									//��
const char C_CH_PHASESTAGE_RF	= 's';									//����
const char C_CH_PHASESTAGE_U	= 'u';									//δ����
const char C_CH_PHASESTAGE_F	= 'f';									//�ɽ���
const char C_CH_PHASESTAGE_OF	= 'c';									//�ص�

typedef enum tagLampClrOutput
{
    LAMP_CLR_OFF	= 0,												//�ص�
    LAMP_CLR_FLASH	= 1,												//����
    LAMP_CLR_ON		= 2,                                                //����
    LAMP_CLR_UNDEF	= 3,												//δ����
}ELampClrOutput;

typedef enum tagFaultLevel
{
    NO_FAULT		= 0,												//�޹���
    COMMON_FAULT	= 1,												//��ͨ����
    CRITICAL_FAULT	= 2,												//���ع���
}EFaultLevel;

typedef enum tagLogicCtlStageVal
{
    CTL_STAGE_UNDEFINE		= 0,                                        //���ƽ׶�δ����
    CTL_STAGE_STARTUP_FLASH = 1,										//����ʱ�����
    CTL_STAGE_STARTUP_RED	= 2,										//����ʱ��ȫ��
    CTL_STAGE_SELFCTL		= 3,                                        //��������
    CTL_STAGE_FAULTFORCE	= 4,										//���ع��ϻ���
}ELogicCtlStageVal;

typedef enum tagCtlCmdSource
{
    CTL_SOURCE_SELF					= 0,                                 //��������
    CTL_SOURCE_LOCAL				= 1,                                 //�����ֶ�
    CTL_SOURCE_SYSTEM				= 2,                                 //ϵͳ����
    CTL_SOURCE_TZPARAM				= 3,                                 //�����������
    CTL_SOURCE_RCONTROL				= 4,                                 //ң��������
    CTL_SOURCE_YELLOWFLASHTRIGGER	= 5,								 //����������
	CTL_SOURCE_DEGRADE              = 6,                                 //��������
	CTL_SOURCE_PREEMPT				= 7,                                 //���ȿ���
}ECtlCmdSource;

typedef enum tagManualSubMode
{
	CTL_MANUAL_SUBMODE_DEFAULT	                = 0,                     //Ĭ��ֵ
	CTL_MANUAL_SUBMODE_PANEL_FITSTMANUAL	    = 1,                     //����һ�ΰ����ֶ�
    CTL_MANUAL_SUBMODE_PANEL_STEPFOWARD	        = 2,                     //��岽��
    CTL_MANUAL_SUBMODE_PANEL_DIRECTION          = 3,                     //��巽��
	CTL_MANUAL_SUBMODE_SYSTEM_STEPFOWARD        = 4,                     //ϵͳ����
    CTL_MANUAL_SUBMODE_SYSTEM_INTERRUPT_PATTERN = 5,                     //ϵͳ��Ԥ����
	CTL_MANUAL_SUBMODE_SYSTEM_CHANNEL_LOCK      = 6,                     //ϵͳͨ������
}EManualSubMode;

typedef enum tagChannelStatus
{
	CHANNEL_STATUS_DEFAULT	= 0,										//Ĭ��
    CHANNEL_STATUS_RED		= 1,										//���
    CHANNEL_STATUS_YELLOW	= 2,										//�Ƶ�
    CHANNEL_STATUS_GREEN	= 3,										//�̵�
    CHANNEL_STATUS_OFF		= 4,										//���
}EChannelStatus;

typedef enum tagLockChannelStatus
{
	LOCKCHANNEL_STATUS_DEFAULT	  = 0,                                     //Ĭ��
    LOCKCHANNEL_STATUS_RED		  = 1,                                     //���
    LOCKCHANNEL_STATUS_YELLOW	  = 2,                                     //�Ƶ�
    LOCKCHANNEL_STATUS_GREEN	  = 3,                                     //�̵�
    LOCKCHANNEL_STATUS_GREENFLASH = 4,                                     //����
	LOCKCHANNEL_STATUS_OFF  	  = 5,                                     //���
    LOCKCHANNEL_STATUS_REDFLASH   = 6,                                     //����
}ELOCKChannelStatus;

typedef enum tagChannelType
{
	CHANNEL_TYPE_DIRECTION	      = 0,                                  //����ͨ��
    CHANNEL_TYPE_LOCK		      = 1,                                  //����ͨ��
}EChannelType;

typedef enum tagPhaseLockType
{
	LOCK_TYPE_CANCEL	          = 0,                                  //ȡ������
    LOCK_TYPE_ALL		          = 1,                                  //�����������˶�����
	LOCK_TYPE_VEH		          = 2,                                  //��������������λ(����������λ)
	LOCK_TYPE_PED		          = 3,                                  //����������(����������λ)
}EPhaseLockType;

typedef enum tagBoardType
{
	BOARD_MAINCTL   = 1,												//���ذ�
	BOARD_LAMP		= 2,												//�ƿذ�
	BOARD_VEHDET	= 3,												//�����
	BOARD_IO		= 4,												//IO��
	BOARD_FAULT     = 5,												//���ϰ�
	TZPARAM_FAULT   = 6,												//��������
}EBoardType;

enum
{
	WORK_NORMAL   = 0,													//0����
	WORK_ABNORMAL = 1,													//1�쳣
};

enum
{
	STEP_STAGE   = 0,													//�׶β���
	STEP_COLOR   = 1,													//ɫ������
};

enum
{
	PREEMPT_TYPE_NORMAL  = 0,                                           //��������
	PREEMPT_TYPE_URGENT  = 1,                                           //��������
	PREEMPT_TYPE_DEFAULT = 2,                                           //Ĭ��
};


typedef enum tagInfoType
{
	LOCAL_MANUALPANEL       = 0,          //����������
	SYSTEM_MANUALCONTROL    = 1,          //ϵͳ�ֶ�����
	SYSTEM_DOWNLOAD_TZPARAM = 2,          //ϵͳ������������
	SYSTEM_DOWNLOAD_HWPARAM = 3,          //ϵͳ�����豸����
	SYSTEM_UPLOADPARAM      = 4,          //ϵͳ���ز���
	SYETEM_REBOOT           = 5,          //ϵͳ�����źŻ�
	SYETEM_DOWNLOADPATTERN  = 6,          //ϵͳ���ط���
	SYETEM_UPLOADPATTERN    = 7,          //ϵͳ���ط���
	SYETEM_DOWNLOADPLAN     = 8,          //ϵͳ���ص��ȼƻ�
	SYETEM_UPLOADPLAN       = 9,          //ϵͳ���ص��ȼƻ�
	SYETEM_DOWNLOADDATE     = 10,         //ϵͳ��������
	SYETEM_UPLOADDATE       = 11,         //ϵͳ��������
	SYETEM_CHANNELCHECK     = 12,         //ϵͳͨ�����
	SYETEM_PATTERNINTERRUPT = 13,         //ϵͳ������Ԥ
	SYETEM_SETTIME          = 14,         //ϵͳ����ʱ��
}EInfoType;

typedef enum tagSelDetectStatus
{
    SELF_DETECT_INING     = 0,			//�Լ���
    SELF_DETECT_SUCCESS   = 1,			//�Լ�ɹ�
    SELF_DETECT_FAILED    = 2,			//�Լ�ʧ��
}ESelDetectStatus;

typedef enum tagSelDetectDetail
{
	NO_WRONG_INFO					= 0,		//�޴�����Ϣ
	USB_MOUNT_FAILED				= 1,		//U��Mountʧ��
    NO_FILE							= 2,		//�����ļ�������
    MD5_CHECK_FAILED				= 3,		//MD5У��ʧ��
    SITEID_CHECK_FAILED				= 4,		//��ַ�벻ƥ��
    FAULT_BOARD_NOTONLINE			= 5,		//���ϰ�û������
	NOT_CONFIG_MASTER_COUNT			= 6,		//�����忨����û������
	LAMP_BOARD_NOTONLINE			= 7,		//�ƿذ�û������
	LAMP_BOARD_ID_WRONG				= 8,		//�ƿذ�ID��
	VETDET_BOARD_ID_WRONG			= 9,		//�����ID��
	IO_BOARD_ID_WRONG				= 10,		//IO��ID��
	WONG_SLOT						= 11,		//�ƿذ��ͷ�������
	WONG_PLUG						= 12,		//�ƿذ��۱������
	RELAY_NOT_WORK					= 13,		//�̵���û������
	USB_NOT_FIND					= 14,		//δ����U��
	OPEN_PARAM_FILE_FAILED			= 15,		//�򿪲����ļ�ʧ��
	PARAM_JSON_FORMAT_ERR			= 16,		//�����ļ�json��ʽ����
	NO_DEVICE_PARAM_FILE			= 17,		//�豸�����ļ�������
	OPEN_DEVICE_PARAM_FILE_FAILED	= 18,		//���豸�����ļ�ʧ��
	OPEN_DEVICE_PARAM_CHECK_FAILED	= 19,		//�豸����(��siteid�����Ϣ)У��ʧ��
}ESelDetectDetail;

typedef enum tagFaultValue
{
	FaultType_MainBoard                    = 101,     //Can����ͨ�Ź���
	FaultType_YellowFlash                  = 102,     //����������
	FaultType_Relay_Not_Work			   = 103,     //�̵���û������

	FaultType_LampBoardID                  = 201,     //�ƿذ�ID����
	FaultType_LampBoardNum                 = 202,     //�ƿذ��ѻ�
	FaultType_NoRedOn                      = 203,     //�޺������
	FaultType_GreenAndRedOn                = 204,     //����ͬ��
	FaultType_GreenConflict                = 205,     //�̳�ͻ

	FaultType_Red_Lamp_Volt_Fault			= 206,	  //��ƵƵ�ѹ���� 
	FaultType_Yellow_Lamp_Volt_Fault		= 207,	  //�ƵƵƵ�ѹ����
	FaultType_Green_Lamp_Volt_Fault			= 208,	  //�̵ƵƵ�ѹ����

	FaultType_Red_Lamp_Power_Fault			= 209,	  //��Ƶƹ��ʹ���
	FaultType_Yellow_Lamp_Power_Fault		= 210,	  //�ƵƵƹ��ʹ���
	FaultType_Green_Lamp_Power_Fault		= 211,	  //�̵Ƶƹ��ʹ���

	FaultType_Lamp_Fault                    = 212,    //�������
	FaultType_Detector_Fault                = 213,    //���������
	FaultType_Wong_Slot						= 214,	  //�ƿذ��۱������	
	FaultType_Wong_Plug						= 215,	  //�ƿذ��ͷ�������	
	FaultType_Config_Master_Count			= 216,	  //�����ƿذ�����û������
	

	FaultType_VetDetID						= 301,	  //�����δ��ʼ����ID����
	FaultType_VetDetNum						= 302,	  //������ѻ�
	FaultType_VetDetector_Short_Circuit		= 303,	  //�������·
	FaultType_VetDetector_Open_Circuit		= 304,	  //�������·

	FaultType_IOBoardID				    	= 401,	  //IO��δ��ʼ����ID����
	FaultType_IOBoardNum					= 402,	  //IO���ѻ�

	FaultType_FaultDetBoard                 = 501,    //���ϰ��ѻ�
	
	FaultType_TZParam                       = 601,     //������������
	FaultType_HWParam                       = 602,     //�豸�����쳣������siteid��
	
}EFaultValue;

//��������103 �Ĺ���������
typedef enum tagSubFaultValueTZParam
{
	FaultSubType_TZParam_NO_Exist			= 1,     //���������ļ�������
	FaultSubType_TZParam_File_NO_Read		= 2,     //���������ļ����ɶ�
	FaultSubType_TZParam_File_Changes		= 3,     //����������Ϊ�޸ģ�MD5У��ʧ�ܣ�
	FaultSubType_TZParam_File_Open_Fail		= 4,     //���������ļ���ʧ��
	FaultSubType_TZParam_Update_Fail		= 5,     //���������ļ�����ʧ��
	FaultSubType_TZParam_SiteID_Failt		= 6,     //�źŻ���ַ��У��ʧ��
	FaultSubType_TZParam_Format_Error		= 7,     //�����������ݸ�ʽ����
	FaultSubType_TZParam_USB_Mount_Fail		= 8,	 //U�̹���ʧ��
	FaultSubType_TZParam_USB_Not_Find		= 9,	 //U��û�ҵ�
	FaultSubType_HWParam_Check_Fault		= 10,	 //�豸�����쳣������siteid��
}ESubFaultValue;

//�Ƶ�ѹ���ϵĹ��������ͣ������졢�ơ��̵ƣ�
typedef enum tagSubFaultLampVolt
{
	FaultSubType_Lamp_Volt_Output_Fail		= 1,     //δ�����Ч��ѹ
	FaultSubType_Lamp_Volt_Output_Low		= 2,     //�����ѹ���������ѹ����
	FaultSubType_Lamp_Volt_Output_High		= 3,     //�����ѹ���������ѹ
	FaultSubType_Lamp_Volt_Off_Output_High	= 4,     //�ر������ʵ�ʵ�ѹ��Ȼ���
	FaultSubType_Lamp_Volt_Off_Output_Low	= 5,     //�ر������ʵ�ʵ�ѹ�������
	FaultSubType_Lamp_Volt_Residual_High	= 6,     //��·������ѹ����

}ESubFaultLampVolt;

//�ƹ��ʹ��ϵĹ��������ͣ������졢�ơ��̵ƣ�
typedef enum tagSubFaultLampPower
{
	FaultSubType_Lamp_Power_Output_Up		= 1,     //�����쳣����
	FaultSubType_Lamp_Power_Output_Down		= 2,     //�����쳣����
	FaultSubType_Lamp_Power_Output_Zero		= 3,     //���������
	FaultSubType_Lamp_Power_Output_High		= 4,     //�ر�״̬���й������

}ESubFaultLampPower;

//������ϵ�������
typedef enum tagSubLampGroupFault
{
	FaultSubType_Lamp_Group_Red		    = 1,		 //��ƹ���
	FaultSubType_Lamp_Group_Yellow		= 2,		 //�Ƶƹ���
	FaultSubType_Lamp_Group_Green		= 3,         //�̵ƹ���

}ESubLampGroupFault;

typedef enum tagSystemControlResultCode
{
    CONTROL_SUCCEED                     = 0,         //��ȷ
    CONTROL_FAILED                      = 1,         //ʧ��
}ESystemControlSuccessCode;

typedef enum tagSystemControlFailCode
{
    NO_SUPPORT_CONTROL_WAY                   = 1,     //��֧�ֵĿ��Ʒ�ʽ
    NO_EXIST_PATTERN_NO                      = 2,     //�����ڵķ������
    NO_SUPPORT_CONTROL_PARAM                 = 3,     //��֧�ֵĿ��Ʋ���
	DEVICE_INIT_NO_EXECUT                    = 4,     //�豸��ʼ���У��޷�ִ��
	HIGH_PRIORITY_PATTERN_CONTROL_NO_EXECUT  = 5,     //���ȼ����ߵķ��������У��޷�ִ��
	HIGH_PRIORITY_USER_CONTROL_NO_EXECUT     = 6,     //���ȼ����ߵ��û������У��޷�ִ��
	NULL_PATTERNNUM							 = 7,	  //������Ŷ�Ӧ�ķ���������
	CONFIG_INCLUDE_GREENCONFLICT             = 8,     //���ð����̳�ͻ
	ROAD_INDEX_NO_EXIST                      = 9,     //�����ڵ�·�ڱ��
	PHASE_INDEX_NO_EXIST                     = 10,    //�����ڵ���λ���
	USER_NO_PERMISSION                       = 11,    //���û�û��Ȩ��
	AREA_OR_ROAD_NO_EXIST                    = 12,    //�����·�ڲ�����
	ACTION_LIST_INDEX_INVALID                = 13,    //��Ч�Ĳ����б���
	READ_FILE_FAILED                         = 14,    //�ļ���ȡʧ��
	ACTION_LIST_INDEX_NO_EXIST               = 15,    //�����б��Ų�����
	ACTION_LIST_COMMAND_ERROR                = 16,    //�����б����������
	ROAD_ID_NO_EXIST                         = 17,    //·��ID������
	CYCLE_INCONSISTENT_IN_DIFFENT_RING       = 18,    //��ͬ�����ڲ�һ��
	SPLIT_TIME_LESS_THAN_MIN_GREEN           = 19,    //���ű�С����С��
	SPLIT_TIME_MORE_THAN_MAX_GREEN           = 20,    //���ű�С����С��
	NO_SUPPORT_PHASE_TYPE                    = 21,    //��֧�ֵ���λ����
	NO_SUPPORT_PHASE_COUNT                   = 22,    //��֧�ֵ���λ����
	CERTIFIED_FAILED                         = 23,    //��֤ʧ��
	KEYFILE_NO_EXIST                         = 24,    //У����Ϣ������
	INVALID_PROTOCOL                         = 25,    //Э�鲻����
}ESystemControlFailCode;


/*��������׼��ʧ��ԭ��*/
const unsigned char C_CH_ISPARAMETERRET_NO				= 0x00;         //�����������쳣
const unsigned char C_CH_ISPARAMETERRET_NOEXIST			= 0x01;			//�ļ�������
const unsigned char C_CH_ISPARAMETERRET_NOREAD			= 0x02;			//�ļ����ɶ�
const unsigned char C_CH_ISPARAMETERRET_CHKERR			= 0x03;			//�ļ�У��ʧ��
const unsigned char C_CH_ISPARAMETERRET_NO_OPEN			= 0x04;			//�ļ���ʧ��
const unsigned char C_CH_ISPARAMETERRET_NO_UPDATE		= 0x05;			//��������ʧ��
const unsigned char C_CH_ISPARAMETERRET_NO_CHKERRSITE	= 0x06;			//��ַ�����ʧ��
const unsigned char C_CH_ISPARAMETERRET_NO_FORMAT_ERR	= 0x07;			//json��ʽ����
const unsigned char C_CH_ISPARAMETERRET_PARAM_ERR		= 0x08;			//��������У��ʧ��
const unsigned char C_CH_ISPARAMETERRET_MOUNT_FAILED	= 0x09;         //USB����ʧ��
const unsigned char C_CH_ISPARAMETERRET_USB_NOT_FIND	= 0x0A;         //USBû�ҵ�

/*���������仯״̬*/
const unsigned char C_CH_ISPARAMETERCHG_NO = 0x00;						//���������ޱ仯
const unsigned char C_CH_ISPARAMETERCHG_OK = 0x01;						//���������б仯    

/*��������׼��״̬*/
const unsigned char C_CH_PARAMERREADY_NO	= 0x00;						//��������׼��ʧ��
const unsigned char C_CH_PARAMERREADY_OK	= 0x01;						//��������׼�����
const unsigned char C_CH_PARAMERREADY_NOT	= 0x02;						//��������δ��ʼ��

/*���������仯��Դ*/
const unsigned char C_CH_ISPARAMETERCHG_SRC_CHANNEL = 0x01;				//��������ͨ�������б仯
const unsigned char C_CH_ISPARAMETERCHG_SRC_OTHER	= 0x02;				//�������������仯

const unsigned long C_N_MAXGLOBALCOUNTER = 65536;						//ȫ�ּ������ֵ

const unsigned long C_N_BOARDCOUNTER_KEY = 120;							//�жϰ忨�Ƿ����ߵļ�������ֵ

const unsigned long C_N_LAMPCTLBOARDCOUNTER_KEY = 40;					//�жϵƿذ��Ƿ����ߵļ�������ֵ

const unsigned long C_N_FLASHLAMPCLRSTATUS_COUNTER = 10;				//�ж��������ļ�������ֵ

const unsigned long C_N_PULSELAMPCLRSTATUS_COUNTER = 4;					//�ж��������ļ�������ֵ

const unsigned long C_N_TIMER_MILLSECOND = 50;							//���붨ʱ��

const unsigned long C_N_TIMER_TIMER_COUNTER = 20;						//����λ��ɫ�仯��������ֵ

const unsigned long C_N_LAMPCTLBOARDFAULT = 400;						//�����ϰ���ϼ���ļ�������ֵ

const unsigned long C_N_MAXWRITECYCLECHGCOUNTER = 32768;				//�����ϼ���д�������ڴ������ֵ

const unsigned long C_N_HWPANEL_BTN_COUNT = 20;							//�ֶ���尴ť����  

const unsigned long C_N_FAULT_TIMER_COUNTER = 20;						//���ϼ���ļ�������ֵ         

const unsigned long C_N_MAX_FAULTQUEUE_SIZE = 3000;						//�������ĵĹ��϶��г���

const unsigned long C_N_MAX_FAULT_COUNT = 300;                          //����������

const int C_N_DETBOARDID_DEFAULT_START = 0x31;                          //�����IDĬ����ʼֵ

typedef struct tagParamRunStatus
{
    unsigned char m_chParameterReady;									//���������Ƿ�׼�����
    unsigned char m_chParameterRet;										//���������쳣ԭ��
    unsigned char m_chIsParameterChg;									//���������Ƿ�仯
    unsigned char m_chParameterChgSrc;									//���������仯��Դ
}TParamRunStatus,*PTParamRunStatus;

typedef struct tagOneLampCtlBoardData
{
    unsigned int  m_nID;												//�忨ID
    unsigned long  m_nCounter;											//����״̬������
	char m_achLampGroupStatus[C_N_CHANNELNUM_PER_BOARD];				//��ɫ״̬����
    char m_achLampGroupPower[C_N_CHANNELNUM_PER_BOARD];					//��������
	char m_achDeFault[2];												//����
}TOneLampCltBoardData,*PTOneLampCltBoardData;

typedef struct tagOneLampCtlBoardFault
{
    unsigned int   m_nID;														//�忨ID
	bool           m_bFaultStatus[C_N_CHANNELNUM_PER_BOARD][C_N_CHANNEL_OUTPUTNUM];//�������״̬,trueΪ����,falseΪ����
	unsigned int   m_nFault[C_N_CHANNELNUM_PER_BOARD][C_N_CHANNEL_OUTPUTNUM];	//�������
    unsigned long  m_nReadFaultTypeCounter[C_N_LAMPBORAD_OUTPUTNUM];			//���������ݼ�����
    unsigned long  m_bReadStatus[C_N_LAMPBORAD_OUTPUTNUM]; //������϶�ȡ״̬,trueΪ��Ҫ��,falseΪ����Ҫ��
}TOneLampCltBoardFault,*PTOneLampCltBoardFault;

typedef struct tagLampCtlBoardData
{
    int m_nLampCltBoardCount;                                       //�ƿذ�����
    TOneLampCltBoardData  m_atLampData[C_N_MAXLAMPBOARD_NUM];       //�ƿذ��ɫ����
	TOneLampCltBoardFault m_atLampFault[C_N_MAXLAMPBOARD_NUM];      //�ƿذ��������  
	bool                  m_bReadParaStatus;                        //����ʵʱ���ݶ�ȡ״̬,trueΪ��Ҫ��,falseΪ����Ҫ��
}TLampCltBoardData,*PTLampCltBoardData;

typedef struct tagOneVehDetBoardData
{
    int m_nVehDetBoardID;                               //�����ID��,0��ʾδ��ʼ��
    char m_achVehChgVal[C_N_MAXDETINPUT_NUM];           //���������Ϣ�仯������
    int m_anVehChgValCounter[C_N_MAXDETINPUT_NUM];      //�����仯��ʱ�������
    char m_achVehTimerVal[C_N_MAXDETINPUT_NUM];         //���������Ϣȫ������        
    int m_nVehTimerValCounter;                          //�����ȫ�������Ϣʱ���
    bool m_bDetFaultStatus[C_N_MAXDETINPUT_NUM];        //��Ȧ״̬,trueΪ����,falseΪ���� 
    unsigned int   m_nFault[C_N_MAXDETINPUT_NUM];       //��Ȧ����   
	unsigned long  m_nReadFaultTypeCounter[C_N_MAXDETINPUT_NUM];//���������ݼ�����
	bool m_bReadStatus[C_N_MAXDETINPUT_NUM];            //��Ȧ���϶�ȡ״̬,trueΪ��Ҫ��,falseΪ����Ҫ��
	bool m_bVehDetExist[C_N_MAXDETINPUT_NUM];			//��Ȧ�Ƿ���ڣ�trueΪ���ڣ�falseΪ������
}TOneVehDetBoardData,*PTOneVehDetBoardData;

typedef struct tagVehDetBoardData
{
    TOneVehDetBoardData m_atVehDetData[C_N_MAXDETBOARD_NUM];
}TVehDetBoardData,*PTVehDetBoardData;

typedef struct tagRealTimeVehDetData
{   
    int m_nDetNum;                                                              //���������
    bool m_bIsNewVehCome[C_N_MAXDETBOARD_NUM * C_N_MAXDETINPUT_NUM];            //�Ƿ��г�������Ȧ
    bool m_bDetFaultStatus[C_N_MAXDETBOARD_NUM * C_N_MAXDETINPUT_NUM];          //��Ȧ״̬,trueΪ����,falseΪ����
    char m_chDetStatus[C_N_MAXDETBOARD_NUM * C_N_MAXDETINPUT_NUM];              //���������µ�״̬,0Ϊû�г�ռ��,1Ϊ�г�ռ��
    int  m_anDetStatusCounter[C_N_MAXDETBOARD_NUM * C_N_MAXDETINPUT_NUM];       //����������״̬������
	bool m_bVehDetExist[C_N_MAXDETBOARD_NUM * C_N_MAXDETINPUT_NUM];				//��Ȧ�Ƿ���ڣ�trueΪ���ڣ�falseΪ������
	char m_chDetStatusInGreen[C_N_MAXDETBOARD_NUM * C_N_MAXDETINPUT_NUM];       //�̵�ʱ�����������µ�״̬,0Ϊû�г�ռ��,1Ϊ�г�ռ��
    int  m_anDetStatusCounterInGreen[C_N_MAXDETBOARD_NUM * C_N_MAXDETINPUT_NUM];//�̵�ʱ������������״̬������
}TRealTimeVehDetData,*PTRealTimeVehDetData;                                     //���ڸ�Ӧ���Ż����ơ�����ͳ��

typedef struct tagOneIOBoardData
{
    int m_nIOBoardID;                                               //IO��ID��,0��ʾδ��ʼ��
    unsigned long  m_nCounter;                                      //�������ݼ�����
    char m_achIOStatus[C_N_MAXIOINPUT_NUM];                         //IO��IO״̬��Ϣ    
}TOneIOBoardData,*PTOneIOBoardData;

typedef struct tagIOBoardData
{
    TOneIOBoardData m_atIOBoardData[C_N_MAXIOBOARD_NUM];
}TIOBoardData,*PTIOBoardData;

typedef struct tagFaultDetBoardData
{
    int m_nFaultDetBoardID;												//���ϼ���ID��,0��ʾδ��ʼ��
    unsigned long  m_nCounter;											//�������ݼ�����
}TFaultDetBoardData,*PTFaultDetBoardData;

typedef struct tagMainCtlBoardRunStatus
{
	bool m_bIsNeedGetParam;												//�߼�����ģ���Ƿ���Ҫ��ȡ��������
    bool m_bIsUseNewParamForFault;										//�߼������Ƿ�ʹ�����²�����֪ͨ���ϼ��ģ��
    bool m_bIsUseNewParamForHard;										//�߼������Ƿ�ʹ�����²�����֪ͨӲ��ģ��
}TMainCtlBoardRunStatus,*PTMainCtlBoardRunStatus;						//���ذ�����״̬��Ϣ�ṹ��

typedef struct tagPhaseLampClrRunCounter
{
    unsigned long m_nCurCounter;										//��ǰ��������ֵ
    unsigned long m_nLampClrStartCounter[C_N_MAXRING_NUM];				//��ɫ��ʼ������ÿ��һ��
    unsigned long m_nPedLampClrStartCounter[C_N_MAXRING_NUM];			//������λ��ɫ��ʼ������ÿ��һ��
    unsigned long m_nLampClrTime[C_N_MAXRING_NUM];                  //��ɫ��ʱ��ÿ��һ��������m_nLampClrCounter[0]��������ʱ���ʱ
    unsigned long m_nPedLampClrTime[C_N_MAXRING_NUM];					//������λ��ɫ��ʱ��ÿ��һ��������m_nLampClrCounter[0]��������ʱ���ʱ
}TPhaseLampClrRunCounter,*PTPhaseLampClrRunCounter;

typedef struct tagLampClrStatus
{
    char m_achLampClr[C_N_MAXLAMPOUTPUT_NUM];							//�ƿذ���ӵĵ�ɫ���
    bool m_bGreenLampPulse[C_N_MAXLAMPOUTPUT_NUM];						//��λ�Ƿ����̵�����
    bool m_bRedLampPulse[C_N_MAXLAMPOUTPUT_NUM];						//��λ�Ƿ��к������
    bool m_bIsRefreshClr;												//�Ƿ���Ҫˢ�µ�ɫ
}TLampClrStatus,*PTLampClrStatus;										//��ɫ��Ϣ�ṹ��

typedef struct tagLogicCtlStatus
{
    int m_nCurCtlMode;													//��ǰ�Ŀ��Ʒ�ʽ
    int m_nCurPlanNo;													//��ǰʹ�õķ�����
    int m_nRunStage;													//�źŻ���ǰ���н׶�
}TLogicCtlStatus,*PTLogicCtlStatus;										//�߼�����״̬��Ϣ�ṹ��

typedef struct tagFlashLampClrStatus
{
    bool m_bFlashLampClrStatus[C_N_MAXLAMPOUTPUT_NUM];					//����״̬ 
    unsigned long m_nFlashLampClrStatusCounter[C_N_MAXLAMPOUTPUT_NUM];	//����ļ�����                                                      
}TFlashLampClrStatus,*PTFlashLampClrStatus;								//����״̬��¼�ṹ��

typedef struct tagLampPulseStatus
{
    bool m_bGreenLampPulse[C_N_MAXLAMPOUTPUT_NUM];						//�̵�����״̬  
    bool m_bRedLampPulse[C_N_MAXLAMPOUTPUT_NUM];						//�������״̬  
    unsigned long m_nGreenLampPulseCounter[C_N_MAXLAMPOUTPUT_NUM];		//�̵�����ļ����� 
    unsigned long m_nRedLampPulseCounter[C_N_MAXLAMPOUTPUT_NUM];		//�������ļ�����                                    
}TLampPulseStatus,*PTLampPulseStatus;									//������״̬��¼�ṹ��

typedef enum
{
	DeviceOnLine                                = -1,                //�豸����
	DeviceOffLine                               = 1,                 //�豸���� 
	OpenTheDoor                                 = 2,                 //�źŻ����� 
	RedAndGreenConflict                         = 3,                 //���̳�ͻ 
	RedLightGoOut                               = 4,                 //���Ϩ�� 
	GreenLighConflict                           = 5,                 //�̵Ƴ�ͻ 
	HighVoltage                                 = 6,                 //��ѹ���� 
	LowVoltage                                  = 7,                 //��ѹ���� 
	GreenLightGoOut                             = 8,                 //�̵�Ϩ�� 
	FuseFailure                                 = 9,                 //����˿����
	PhasePlateReverseError                      = 10,                //��λ�巴�ϴ���
	MultipleJunctionParameteConfigurationError  = 11,                //��·�ڲ������ô���
	LightsOff                                   = 12,                //�ƿعر�
	PedestrianButtonOpeningFailure              = 13,                //���˰�ť��������
	PedestrianButtonClosingFailure              = 14,                //���˰�ť�ջ�����
	CountDownCardCommunicationFailure           = 15,                //����ʱ��ͨ�Ź���
	CommunicationFailureOfStrategicDetector     = 16,                //ս�Լ����ͨ�Ź���
	DetectorIsNotRespondingLongTime             = 17,                //�������ʱ�䲻��Ӧ
	DetectorClosedLoopError                     = 18,                //������ջ�����
	DetectorCountError                          = 19,                //�������������
	CabinetReportFrontDoorOpen                  = 20,                //���񱨸�ǰ�ſ�
	CabinetReportBackDoorOpen                   = 21,                //���񱨸���ſ�
	CabinetReportSmallDoorOpen                  = 22,                //���񱨸�С�ſ�
	CabinetReportHighTempture                   = 23,                //���񱨸��¶ȹ���
	CabinetReportHighWaterLevel                 = 24,                //���񱨸�ˮλ����
	CabinetReportBigHumidity                    = 25,                //���񱨸�ʪ�ȹ���
	CabinetReportPoint1Novoltage                = 26,                //���񱨸����1��ѹ��
	CabinetReportPoint2Novoltage                = 27,                //���񱨸����2��ѹ��
	CabinetReportPoint3Novoltage                = 28,                //���񱨸����3��ѹ��
	CabinetReportPoint4Novoltage                = 29,                //���񱨸����4��ѹ��
	CellReportOn                                = 30,                //���񱨸��ϵ�
	LockerReportIllegalOpen                     = 31,                //���񱨸�Ƿ�����
	LockerReportlegalOpen                       = 32,                //���񱨸�Ϸ�����
	CabinetReportsVibration                     = 33,                //���񱨸��𶯲���
	ManualControlNoOperation                    = 34,                //�ֶ�����ʱ����ʱ���޲���
	GreenLightTooShort                          = 35,                //�̵�ʱ����̣�����������
}DeviceStatus;

typedef enum
{
	PhasePassStatus_Normal						= 0,
	PhasePassStatus_Close						= 1,
	PhasePassStatus_VehClose					= 2,
	PhasePassStatus_PedClose					= 3,
}PhasePassStatus;

typedef union tagFaultType
{
	unsigned short Hex;                     
	struct
	{
		unsigned  bDevice_Fault             :1;
		unsigned  bRedLamp_Fault            :1;
		unsigned  bVoltage_Fault            :1;
		unsigned  bGreenLamp_Fault          :1;
		unsigned  bFuse_Fault               :1;
		unsigned  bPhaseBoard_Fault		    :1;     
		unsigned  bConfig_Fault             :1;
		unsigned  bRing_Fault		        :1;	
		unsigned  bCom_Fault		        :1;	
		unsigned  bDetector_Fault		    :1;	
		unsigned  bCabinet_Fault		    :1;	
		unsigned  bReserved                 :8;
	}Bin;                                                            
}TFaultType,*PTFaultType;

typedef union tagCabinetFaultType
{
	unsigned  long	lFault;
	struct
	{
		unsigned	bCabinetReportHighTempture_Fault:1;		//�¶ȹ���
		unsigned	bCabinetReportHighWaterLevel_Fault:1;	//ˮλ����
		unsigned	bCabinetReportBigHumidity_Fault:1;		//ʪ�ȹ���
		unsigned	bCabinetReportPoint1Novoltage_Fault:1;	//����1��ѹ����
		unsigned	bCabinetReportPoint2Novoltage_Fault:1;	//����2��ѹ����
		unsigned	bCabinetReportPoint3Novoltage_Fault:1;	//����3��ѹ����
		unsigned	bCabinetReportPoint4Novoltage_Fault:1;	//����4��ѹ����
		unsigned	bLockerReportIllegalOpen_Fault:1;		//���Ź���
		unsigned	bCabinetReportsVibration_Fault:1;		//�𶯹���
		unsigned	bManualControlNoOperation_Fault:1;	    //�ֶ�����ʱ����ʱ���޲�������
		unsigned	bGreenLightTooShort_Fault:1;            //�̵�ʱ�����
		unsigned	bReserved:21;
	}Bin;
}TCabinetFaultType,*PTCabinetFaultType;

typedef union tagDetectorFaultType
{
	unsigned char chFault;
	struct 
	{
		unsigned 	bCommunicationFailureOfStrategicDetector_Fault:1;//ͨ�Ź���
		unsigned 	bDetectorIsNotRespondingLongTimeFault:1;		 //����Ӧ����
		unsigned 	bDetectorClosedLoopError_Fault:1;		         //�ջ�����
		unsigned 	bDetectorCountError_Fault:1;		             //��������
		unsigned 	bReserved:4;
	}Bin;
}TDetectorFaultType,*PTDetectorFaultType;

typedef union tagLampFaultType
{
	unsigned char chFault;
	struct  
	{
		unsigned  bRedAndGreenConflict_Fault:1;//���̳�ͻ 
		unsigned  bRedLightGoOut_Fault:1;	   //���Ϩ��
		unsigned  bGreenLighConflict_Fault:1;  //�̵Ƴ�ͻ
		unsigned  bGreenLightGoOut_Fault:1;	   //�̵�Ϩ��
		unsigned  LightsOfft_Fault:1;          //�ƿعر�
        unsigned  bLampNumger_Fault:1;         //�ƿذ���������
        unsigned  bFaultBoardOffline_Fault:1;  //���ϰ����
		unsigned  bReserved:2;
	}Bin;
}TLampFaultType,*PTLampFaultType;

#pragma pack(1)
typedef struct tagCommonFault
{
	union tagCabinetFaultType	    m_tCabinetFault;
	union tagDetectorFaultType	    m_tDetectorFault;
	union tagLampFaultType  	    m_tLampFault;
}TCommonFault,*PTCommonFault; 
#pragma pack()

typedef union tagCanData
{
	unsigned  char	chCanData;
	struct    
	{   
		unsigned io1:1;   
		unsigned io2:1;   
		unsigned io3:1;   
		unsigned io4:1;   
		unsigned io5:1;   
		unsigned io6:1;   
		unsigned io7:1;   
		unsigned io8:1; 
	 }CanData;
}TCanData, *PTCanData;

typedef struct tagAllBoardUseStatus
{
    char m_achLampCtlBoardStatus[C_N_MAXLAMPBOARD_NUM];								//�ƿذ�ʹ����Ϣ����
    char m_achVehDetBoardStatus[C_N_MAXDETBOARD_NUM];								//�����ʹ����Ϣ����
    char m_achIOBoardStatus[C_N_MAXIOBOARD_NUM];									//IO��ʹ����Ϣ����
    char m_achDetectorUseStatus[C_N_MAXDETBOARD_NUM * C_N_MAXDETINPUT_NUM];			//������ʹ����Ϣ����
}TAllBoardUseStatus,*PTAllBoardUseStatus;											//�����������л��

typedef struct tagAllBoardOnlineStatus
{
    char m_achLampCtlBoardOnlineStatus[C_N_MAXLAMPBOARD_NUM];						//�ƿذ�������Ϣ����
    char m_achVehDetBoardOnlineStatus[C_N_MAXDETBOARD_NUM];							//�����������Ϣ����
    char m_achIOBoardOnlineStatus[C_N_MAXIOBOARD_NUM];								//IO��������Ϣ����
    char m_chFaultDetectBoardStatus;												//���ϼ���������Ϣ,1��ʾ����,0��ʾ����
}TAllBoardOnlineStatus,*PTAllBoardOnlineStatus;										//Canͨ��ģ���л��

typedef struct tagOnePhaseRunStatus
{
    BYTE m_byPhaseID;																//��λ���
    int m_nSplitTime;																//���ű�ʱ��
    char m_chPhaseStatus;															//��λ�Ļ�����״̬
    int m_nCurStageRemainTime;														//��ǰ�׶λ�����ʣ��ʱ��

	char m_chPedPhaseStatus;														//��λ������״̬
	int m_nCurPedRemainTime;														//��λ�����˵�ʣ��ʱ��

	BYTE m_byOverlapPhaseID;														//����λ��Ϊĸ��λ����Ӧ�ĸ�����λ�ı��(�����Ϊĸ��λ��Ĭ��Ϊ0)
	char m_chOverlapPhaseStatus;													//����λ��Ϊĸ��λ����Ӧ�ĸ�����λ��״̬
	int m_nOverlapPhaseRemainTime;													//����λ��Ϊĸ��λ����Ӧ�ĸ�����λ��ʣ��ʱ��

    BYTE m_abyPhaseConcurrency[MAX_PHASE_CONCURRENCY_COUNT];						//������λ
	char m_chPhaseCloseStatus;                                                      //��λ�ض�״̬  1���ر� 0��ȡ���ر�
	char m_chPhaseLockStatus;                                                       //��λ����״̬��1������ 0��ȡ������
	char m_chPhaseControlStatus;													//��λ���������ֹ״̬ 0�����������ֹ 1����λ���� 2����λ��ֹ
}TOnePhaseRunStatus,*PTOnePhaseRunStatus;

typedef struct tagOneOverlapPhaseRunStatus
{
	BYTE m_byOverlapPhaseID;														//������λ���
	int m_nSplitTime;																//�ø�����λ�����ű�ʱ��
	BYTE m_byOverlapType;															//�ø�����λ�Ļ�����״̬0�ص�  1�� 2 �� 3�� 4���� 5����
	int m_nOverlapRemainTime;														//�ø�����λ�Ļ���������ʱ
	BYTE m_byOverlapPedType;														//�ø�����λ������״̬0�ص�  1�� 2 �� 3�� 4���� 5����
	int m_nOverlapPedRemainTime;													//�ø�����λ�����˵���ʱ
	int m_nMotherPhase[MAX_PHASE_COUNT_IN_OVERLAP];									//��ǰ������λ��ĸ��λ
}TOneOverlapPhaseRunStatus, *PTOneOverlapPhaseRunStatus;

typedef struct tagOverlapRunStatus
{
	int m_nOverlapPhaseCount;														//������λ����
	TOneOverlapPhaseRunStatus m_atOverlapPhaseRunStatus[MAX_OVERLAP_COUNT];
}TOverlapRunStatus, *PTOverlapRunStatus;

typedef struct tagOneRingRunStatus
{
    int m_nPhaseCount;																//������λ����
    int m_nCurRunPhaseIndex;														//��ǰ���е���λ����,��0��ʼ
    int m_nCurStageIndex;															//��ǰ���е���λ��Ӧ�Ľ׶�����
    TOnePhaseRunStatus m_atPhaseStatus[MAX_SEQUENCE_TABLE_COUNT];					//��������״̬
}TOneRingRunStatus,*PTOneRingRunStatus;

typedef struct tagOneRunStageInfo
{
    int  m_nStageIndex;
    int  m_nStageStartTime;
    int  m_nStageEndTime;
    int  m_nConcurrencyPhaseCount;
    int  m_nConcurrencyPhase[MAX_RING_COUNT];
	bool m_bDirectTransit[MAX_RING_COUNT];
	bool m_bPhaseStartTransitInCurStage[MAX_RING_COUNT];
	bool m_bPedPhaseStartTransitInCurStage[MAX_RING_COUNT];
	int  m_nPhaseCurStageTransitTime[MAX_RING_COUNT];
	int  m_nPedPhaseCurStageTransitTime[MAX_RING_COUNT];
}TOneRunStageInfo,*PTOneRunStageInfo;

typedef struct tagRunStageInfo
{
    int  m_nRunStageCount;
    TOneRunStageInfo  m_PhaseRunstageInfo[MAX_STAGE_COUNT];
}TRunStageInfo,*PTRunStageInfo;

typedef struct tagDirectionKeyChannelClr
{      
    int            m_nChannelID;                                                   //ͨ��ID                                        
    char           m_achChannelClr;                                                //������ƶ�Ӧ��ͨ����ɫ                          
    unsigned long  m_nChannelDurationTime;					                       //������ƶ�Ӧ��ͨ����ɫ����ʱ��
}TDirectionKeyChannelClr,*PTDirectionKeyChannelClr;

typedef struct tagLockPhaseClr
{      
    BYTE           m_byPhaseID;                                                    //��λID                                        
    char           m_achPhaseClr;                                                  //��λ��ɫ                          
    unsigned long  m_nPhaseClrDurationTime;					                       //��λ����ʱ��
}TLockPhaseClr,*PTLockPhaseClr;

typedef struct tagPhaseRunStatus
{
    int m_nCurCtlPattern;															//����ģʽ
    int m_nCurCtlMode;																//���Ʒ�ʽ
    BYTE m_byPlanID;																//�������
    char m_achPlanName[12];															//��������
    int m_nCycleLen;																//���ڳ�
    int m_nOffset;																	//��λ��
	int m_nPatternOffset;                                                           //Э����λ��
    int m_nCycleRunTime;															//��������ʱ��
    int m_nCycleRemainTime;															//����ʣ��ʱ��
    int m_nRingCount;																//������
    TOneRingRunStatus m_atRingRunStatus[MAX_RING_COUNT];							//������״̬
    TRunStageInfo     m_tRunStageInfo;												//�׶α�
	TOverlapRunStatus m_atOverlapRunStatus;											//������λ����״̬
}TPhaseRunStatus,*PTPhaseRunStatus;													//��ǰ��������״̬��Ϣ

typedef struct tagHWPanelBtnStatus
{
    int  m_nHWPanelBtnIndex;														//��尴ť���
    bool m_bHWPanelBtnStatus;														//��尴ť״̬
}THWPanelBtnStatus,*PTHWPanelBtnStatus;              

typedef struct tagGpsData
{
    long	m_nGpsTime;                                                            //ʱ��
    double	m_nLongitude;                                                          //����
    double	m_nLatitude;                                                           //γ��
}TGpsData,*PTGpsData;    

typedef struct tagWholeDeviceStatusInfo
{
    char 	m_cSiteID[4];															//��������
	char	m_cHardwareVer[10];														//Ӳ���汾��
    char	m_cVoltage;																//��ѹֵ
    char	m_cRunStatus;															//����״̬
}TWholeDeviceStatusInfo,*PTWholeDeviceStatusInfo;            

typedef struct tagBoardStatusInfo
{
    char 	m_cID;																	//��λid
    char	m_cBoardType;															//�忨����
    char	m_cRunStatus;															//����״̬
}TBoardStatusInfo,*PTBoardStatusInfo;     

typedef struct tagYellowFlashStatusInfo
{
    char	m_cRunStatus;															//����״̬
    char	m_cTriggerStatus;														//Ӳ����״̬
}TYellowFlashStatusInfo,*PTYellowFlashStatusInfo;

typedef struct tagOpenATCStatusInfo
{
    TWholeDeviceStatusInfo	m_tWholeDevStatusInfo;									//����״̬
	char					m_cInUsedFuncBoardCount;
    TBoardStatusInfo		m_tFuncBoardStatusInfo[C_N_MAX_FUNC_CARDSLOT_NUM];		//���ܰ忨״̬
	char					m_cInUsedCtrlBoardCount;
    TBoardStatusInfo		m_tCtrlBoardStatusInfo[C_N_MAX_CTRL_CARDSLOT_NUM];		//���ذ忨״̬
	TYellowFlashStatusInfo	m_tYellowFlashStatusInfo;								//������״̬
}TOpenATCStatusInfo,*PTOpenATCStatusInfo;    

typedef struct tagLedScreenRingInfo
{
	int m_nCurRunPhaseIndex;														//��ǰ������λ����ֵ
	int m_nSplitTime;																//���ű�ʱ��
}TLedScreenRingInfo,*PTLedScreenRingInfo;

typedef struct  tagRunFaultInfo
{
	char                     m_cBoardType;                                          //�忨����
    char                     m_cFaultLevel;                                         //���ϼ���
	DWORD                    m_wFaultType;                                          //��������
	DWORD                    m_wSubFaultType;                                       //����������
	char		             m_faultInfo[10];                                       //��������
	long                     m_unFaultOccurTime;   			                        //���Ϸ���ʱ�� 
}TRunFaultInfo,*PTRunFaultInfo;   

typedef struct tagChannelLockCtrlCmd
{
	int  m_nDuration;									//����ʱ��
	int  m_nGreenFlash;									//��������ʱ��
	int  m_nYellow;										//�Ƶƹ���ʱ��
	int  m_nRedClear;									//��ƹ���ʱ��
	int	 m_nMinGreen;									//��С��ʱ��
	int  m_nChannelLockStatus[MAX_CHANNEL_COUNT];       //ͨ������״̬״̬��0Ϊ�ָ�Ĭ��״̬��1Ϊ��ƣ�2Ϊ�Ƶƣ�3Ϊ�̵ƣ�4Ϊ������5Ϊ��ƣ�6Ϊ����
}TChannelLockCtrlCmd,*PTChannelLockCtrlCmd;

typedef struct tagPhaseLockCtrlCmd
{
	int  m_nDuration;									//����ʱ��
	int  m_nGreenFlash;									//��������ʱ��
	int  m_nYellow;										//�Ƶƹ���ʱ��
	int  m_nRedClear;									//��ƹ���ʱ��
	int	 m_nMinGreen;									//��С��ʱ��
	int  m_nPhaseLockType[MAX_PHASE_COUNT];             //��λ�������ͣ�0Ϊȡ��������1�����������˶�������2��������������λ(����������λ)��3����������(����������λ)
}TPhaseLockCtrlCmd,*PTPhaseLockCtrlCmd;

typedef struct tagPhaseLockPara
{
	int  m_nPhaseLockCount;                             //������λ����
	int  m_nPhaseLockID[MAX_PHASE_COUNT];               //������λID
	int  m_nPhaseLockType[MAX_PHASE_COUNT];             //������λ����
	int  m_nOverlapPhaseLockCount;                      //����������λ����
	int  m_nOverlapPhaseLockID[MAX_PHASE_COUNT];        //����������λID
	int  m_nOverlapPhaseLockType[MAX_PHASE_COUNT];      //����������λ����
}TPhaseLockPara,*PTPhaseLockPara;

typedef struct tagLedScreenShowInfo
{
	int m_nPlanNo;                                                                  //������
	int m_nCurCtlMode;																//��ǰ�Ŀ��Ʒ�ʽ
	int m_nSubCtlMode;                                                              //��ǰ�Ŀ�����ģʽ
    int m_nCycleLen;																//���ڳ�
    int m_nOffset;																	//��λ��
    int m_nRingCount;																//������
	TLedScreenRingInfo m_tScreenRingInfo[C_N_MAX_SCREEN_RING_NUM];					//��ǰ������״̬

    bool                     m_bKeyDirectionControl;                                //�Ƿ��˷����
    int                      m_nDirectionKeyIndex;                                  //��������
    int                      m_nChannelCount;                                       //������ƶ�Ӧ��ͨ������   
    TDirectionKeyChannelClr  m_tDirectionKeyChannelClr[MAX_CHANNEL_COUNT];          //��������Ƶ�ͨ����ɫ

	bool                     m_bChannelCheck;                                       //�Ƿ���ͨ������
	TDirectionKeyChannelClr  m_tChannelCheckClr;                                    //ͨ�������Ƶ�ͨ����ɫ

	unsigned int             m_nRunFaultCount;                                      //���й�������
	TRunFaultInfo            m_tRunFaultInfo[C_N_MAX_FAULT_COUNT];                  //���й���

	bool                     m_bChannelLockCheck;                                   //�Ƿ��·�ͨ������ָ��
	TChannelLockCtrlCmd      m_tChannelLockCtrlCmd;                                 //ͨ����������
	int                      m_nChannelLockCount;                                   //ͨ��������Ӧ��ͨ������   
    TDirectionKeyChannelClr  m_tChannelLockChannelClr[MAX_CHANNEL_COUNT];           //ͨ��������ͨ����ɫ

	bool                     m_bPhaseToChannelLock;                                 //��λ����תͨ��������־
	int                      m_nPhaseLockCount;                                     //������λ����   
	TLockPhaseClr            m_tPhaseClr[MAX_PHASE_COUNT];                          //������λ��ɫ

}TLedScreenShowInfo,*PTLedScreenShowInfo;        

typedef struct  tagSelfDetectInfo
{
    char    m_cSelfDetectStatus; 
    char    m_cSelfDetectFailedReason;
	char    m_cSelfDetectInfo[10]; 
}TSelfDetectInfo,*PTSelfDetectInfo;   

typedef struct tagVehicleQueueUpInfo
{
    int  m_nDetectorID;									//�����ID
	BYTE m_byVehicleDetectorCallPhase;					//�������Ӧ��������λ
	int  m_nVehicleQueueUpLength;						//�Ŷӳ���
}TVehicleQueueUpInfo,*PTVehicleQueueUpInfo;

typedef struct tagPedDetectInfo
{
    int  m_nDetectorID;									//���˼����ID
	BYTE m_byVehicleDetectorCallPhase;					//���˼������Ӧ��������λ
	BYTE m_byDetectorType;								//���������
	int  m_nPedCount;									//���˼������
}TPedDetectInfo,*PTPedDetectInfo;

typedef struct tagChannelLampStatus
{
	int m_nChannelID;
	int m_nLampLight;
	int m_nCountDown;
}TChannelLampStatus,*PTChannelLampStatus; 

typedef struct tagSystemControlStatus
{
	int  m_nSpecicalControlResult;						//������ƽ�����磺������ȫ�죬�ص�
	int  m_nPatternControlResult;						//�������ƽ��
	int  m_nStageControlResult;							//�׶ο��ƽ��
	int  m_nPhaseControlResult;                         //��λ�ضϿ��ƽ��
	int  m_nChannelLockResult;                          //ͨ���������ƽ��
	int  m_nSpecicalControlFailCode;					//�������ʧ��ԭ��
	int  m_nPatternControlFailCode;						//��������ʧ��ԭ��
	int  m_nStageControlFailCode;						//�׶ο���ʧ��ԭ��
	int  m_nPhaseControlFailCode;                       //��λ�ضϿ���ʧ��ԭ��
	int  m_nChannelLockFailCode;                        //ͨ����������ʧ��ԭ��
}TSystemControlStatus,*PTSystemControlStatus; 

typedef struct tagStepForwardCmd
{
	int  m_byStepType;									//��ǰ�������� 0:�׶� 1:��ɫ
	int  m_nNextStageID;								//��һ���׶α�ţ���1��ʼ��0�ǲ���
	int  m_nDurationTime;								//����ʱ��
	int  m_nDelayTime;									//�ӳ�ʱ��
}TStepForwardCmd,*PTStepForwardCmd; 

typedef struct tagDirectionCmd
{
	bool m_bStepFowardToDirection;						//true��ʾ�Ӳ����й����ķ���false��ʾ�����й����ķ���
	int  m_nTargetStageIndex;							//�з���֮ǰ�Ľ׶ε���һ���׶α��
	int  m_nNextDirectionIndex;							//��һ��������
}TDirectionCmd,*PTDirectionCmd; 

typedef struct tagPatternInterruptCmd
{
	int  m_nControlMode;								//���Ʒ�ʽ
	int  m_nPatternNo;									//������
	TInterruptPatternInfo m_tManualControlPattern;      //�ֶ����Ʒ���
}TPatternInterruptCmd,*PTPatternInterruptCmd;

typedef struct tagChannelLockCmd
{
	bool m_bStepFowardToChannelLock;					//true��ʾ�Ӳ����й�����ͨ��������false��ʾ�����й�����ͨ������
	int  m_nTargetStageIndex;							//��ͨ������֮ǰ�Ľ׶ε���һ���׶α��
	int  m_nDelayTime;                                  //�Ӳ����е�ͨ������ʱ���ӳ�ʱ��
	int  m_nDurationTime;                               //�Ӳ����е�ͨ������ʱ�ĳ���ʱ��
	TChannelLockCtrlCmd  m_tNextChannelLockCtrlCmd;		//��һ��ͨ����������
}TChannelLockCmd,*PTChannelLockCmd;

typedef struct tagManualCmd
{
	int                  m_nCtlMode;                     //���Ʒ�ʽ
	int                  m_nSubCtlMode;                  //������ģʽ
	int                  m_nCmdSource;                   //ָ����Դ
	int                  m_nCurCtlSource;                //��ǰ����Դ
	char                 m_szPeerIp[20];                 //����ԴIP
	bool                 m_bNewCmd;                      //��ָ��
	bool                 m_bStepForwardCmd;              //��һ���Ƿ���Ҫ������
	TStepForwardCmd      m_tStepForwardCmd;              //��������      
    bool                 m_bDirectionCmd;                //��һ���Ƿ���Ҫ������
	TDirectionCmd        m_tDirectionCmd;                //��������
	bool                 m_bPatternInterruptCmd;         //��һ���Ƿ���Ҫ������Ԥ
	TPatternInterruptCmd m_tPatternInterruptCmd;         //������Ԥ����
	bool                 m_bChannelLockCmd;              //��һ���Ƿ���Ҫ����ͨ������
	TChannelLockCmd      m_tChannelLockCmd;              //ͨ������ָ��
	bool                 m_bPhaseToChannelLock;          //��λ����תͨ��������־
	TPhaseLockPara       m_tPhaseLockPara;               //��λ����תͨ������ָ�����λ��������
	bool                 m_bPreemptCtlCmd;               //��һ���Ƿ���Ҫ���ȿ���
}TManualCmd,*PTManualCmd; 

typedef struct tagPhasePassCmdPhaseStatus
{
	bool m_bNewCmd;										//�Ƿ�Ϊ��ָ�0Ϊ���账��ľ�ָ�1Ϊ��Ҫ�������ָ��
	bool m_bUpdatePhasePassStatus[MAX_PHASE_COUNT];     //��λ�Ƿ���Ҫ����״̬��0Ϊ����Ҫ��1Ϊ��Ҫ
	int  m_nPhasePassStatus[MAX_PHASE_COUNT];           //��λ����״̬��0Ϊ������1Ϊ�رշ���
}TPhasePassCmdPhaseStatus,*PTPhasePassCmdPhaseStatus;

#ifdef VIRTUAL_DEVICE
typedef struct tagVirtualRunTime
{
	int VirtualYear;			//�������е�ʱ�䡪����
	int VirtualMon;				//�������е�ʱ�䡪����[1,12]
	int VirtualDay;				//�������е�ʱ�䡪����[1,31]
	int VirtualHour;			//�������е�ʱ�䡪��ʱ[0,23]
	int VirtualMin;				//�������е�ʱ�䡪����[0,59]
	int VirtualSec;				//�������е�ʱ�䡪����[0.59]
	int VirtualWeek;			//�������е�ʱ�䡪������[0,6]
	DWORD TempGlobalCount;		//��¼ʱ��ʱ��ȫ�ּ�����������һ��ʱ������㣩
}TVirtualRunTime, * PTVirtualRunTime;
#endif // VIRTUAL_DEVICE
//Virtual_Test2022

typedef struct tagPreemptControlStatus
{
	int  m_nPreemptControlResult;					    //���ȿ��ƽ��
	int  m_nPreemptControlResultFailCode;               //���ȿ���ʧ��ԭ��
}TPreemptControlStatus,*PTPreemptControlStatus; 

typedef struct tagPreemptCtlCmd
{
	int                  m_nCmdSource;                   //ָ����Դ
	int                  m_nCurCtlSource;                //��ǰ����Դ
	char                 m_szPeerIp[20];                 //����ԴIP
	bool                 m_bNewCmd;						 //�Ƿ�Ϊ��ָ�0Ϊ���账��ľ�ָ�1Ϊ��Ҫ�������ָ��
	BYTE                 m_byPreemptType;                //�������� 0:�������� 1:�������� 0:�������� 2:Ĭ��
	BYTE                 m_byPreemptPhaseID;             //������λID
	BYTE                 m_byPreemptStageIndex;          //������λ��Ӧ�Ľ׶α��
	WORD                 m_wPreemptDelay;                //�ӳ�ʱ��
	WORD                 m_wPreemptDuration;             //����ʱ��
	BYTE                 m_byPreemptLevel;               //���ȼ� 1-5���ȼ�Խ�ߣ����ȼ�Խ��
	BYTE                 m_byPreemptSwitchFlag;          //������λ��ʼ�л���־��0δ��ʼ�л���1�ѿ�ʼ�л�
	bool                 m_bPatternInterruptCmd;         //�Ƿ��յ�������Ԥ����
	TPatternInterruptCmd m_tPatternInterruptCmd;         //������Ԥ����
	bool                 m_bIncludeConcurPhase;          //�Ƿ����������λ
	BYTE                 m_byPreemptConcurPhaseID;       //���Ȳ�����λID
}TPreemptCtlCmd,*PTPreemptCtlCmd; 

//20999Ԥ���ṹ��
typedef struct tagDeviceStatus
{
	BYTE  m_byDoorStatus;                //����״̬
	short m_nVoltage;                    //��ѹ
	short m_nCurrent;                    //����
	char  m_chTemperature;               //�¶�          
	BYTE  m_byHumidity;                  //ʪ��
}TDeviceStatus, * PTDeviceStatus;