#pragma once

typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned long DWORD;
//�źŻ������������ó��������ļ�
#define     MAX_PANEL_KEY_CFG_COUNT             12      //�ֶ��������ð�������
#define		MAX_SITEID_COUNT					60		//��ַ���С
#define		MAX_ROADNO_LENGTH					2		//·��id��С
#define		MAX_IPSTR_LEN						25		//����ip
#define		MAX_NETCARD_TABLE_COUNT			    2		//��������
#define     MAX_FAULT_TABLE_COUNT				500     //���ϱ��С
#define     MAX_FAULT_DESC_LEN				    64      //������������
#define	    MAX_OPERATIONRECORD_DESC_LEN		256	    //������¼��������
#define		MAX_PHASE_COUNT					    40		//��λ���������
#define		MAX_LPHASE_COUNT					12		//��������λ���������
#define		MAX_PHASE_CONCURRENCY_COUNT		    40		//��󲢷���λ��
#define		MAX_PHASE_STATUS_COUNT				2		//��λ״̬����������
#define		MAX_PHASE_CONTROL_COUNT			    4		//��λ��������������
#define		GREEN_TIME_CALCULATE_TIME			2		//��λ����ʹ�������1����

#define		MAX_VEHICLEDETECTOR_COUNT			64	    //������������������ trap
#define		MAX_VEHICLEDETECTOR_COUNT_S		    48 		//������������������ snmp
#define		MAX_VEHICLEDETECTOR_STATUS_COUNT	8		//���������״̬����������
#define		MAX_PEDESTRIANDETECTOR_COUNT		8		//���˼�������������

#define		MAX_DESC_LENGTH						32		//������󳤶�

#define		MAX_VOLUMEOCCUPANCY_COUNT			MAX_VEHICLEDETECTOR_COUNT		//����ռ���ʱ��������
#define		MAX_PATTERN_COUNT					109		//��������
#define		MAX_SPLIT_COUNT					    36		//���űȱ��������

#define		MAX_OFFSET_COUNT					3		//���űȱ��Ӧ����λ������

#define		MAX_TIMEBASE_ACTION_COUNT		    255		//ʱ�����������
#define		MAX_SCHEDULE_COUNT			        255		//���ȼƻ����������

#define		MAX_DAYPLAN_TABLE_COUNT		  	    16		//���ʱ�α���
#define		MAX_SINGLE_DAYPLAN_COUNT			48		//ÿ��ʱ�α������(�������DAYPLANEVNET)

#define		MAX_SEQUENCE_TABLE_COUNT			32		//ʱ�򷽰�����
#define		MAX_RING_COUNT					    4		//ʱ�򷽰������������������ring����
#define		MAX_SEQUENCE_DATA_LENGTH			32		//ʱ�����ݵ���󳤶�

#define		MAX_CHANNEL_COUNT					80		//ͨ�����������
#define		MAX_CHANNEL_STATUS_COUNT			4		//ͨ��״̬����������

#define		MAX_CHANNEL_LOCK_COUNT				48		//ͨ���������������

#define		MAX_OVERLAP_COUNT					16		//�ص����������
#define		MAX_PHASE_COUNT_IN_OVERLAP			40		//�ص��е����include��λ��
#define		MAX_PHASE_COUNT_MO_OVERLAP			16		//�ص��е����Modifier��λ��
#define		MAX_OVERLAP_STATUS_COUNT			2		//�ص�״̬���������
#define		MAX_MODULE_COUNT					255		//ģ����������
#define		MAX_MODULE_STRING_LENGTH			128		//ģ�����ַ�������

#define		MAX_EVENTLOG_COUNT				    255		//�¼����������
#define		MAX_EVENTCLASS_COUNT				1		//�¼��������ֵ��ʵ��Ϊ3
#define		MAX_EVENTCONFIG_COUNT				50		//�¼����ñ������

#define		MAX_PREEMPT_COUNT					8		//����һ�������
#define		MAX_PEDESTRIANPHASE_COUNT			8  		//������λ������

#define		MAX_COUNTDOWNBOARD_COUNT			24		//��󵹼�ʱ�Ƹ���
#define		MAX_COUNTDOWNBOARD_INCLUDEDPHASES	16		//ÿ������ʱ�����ܰ����������λ��

#define		MAX_SPECIALFUNCOUTPUT_COUNT		    8		//���֧�ֵ����⹦������
#define		MAX_DYNPATTERNSEL_COUNT			    8		//���̬����ѡ�����ñ�����
#define		MAX_RS232PORT_COUNT			 	    3		//���֧�ֵĶ˿����ñ�����
#define     MAX_SIGNALTRANS_LIMIT	            20		//�ź�ת����������ʱ������
#define     MAX_ALLSTOPPHASE_LIMIT              32      //������λ������ 
#define		MAX_BYTE_VALUE			            255
#define		MAX_2BYTE_VALUE			            65535

#define		MAX_TRANSINTENSITYCHOICE_COUNT		90		//��ͨǿ������ѡ����������
#define     MAX_PHASE_TYPE_COUNT                3       //��λ����   

#define     MAX_ITSCONSERI_COUNT                6       //���ڱ���������    
#define     MAX_ITSCONIO_COUNT                  24      //IO ���ñ��������   
#define     MAX_ITSCONCHANNEL_COUNT             32      //��ͻͨ�����������  
#define	    MAX_STAGE_COUNT					    40	    //�׶������
#define     C_N_MAX_LAMP_OUTPUT_NUM				240     //���ƿ�������Ӹ���

typedef enum tagPhaseSrcType
{
    PHASE_SRC = 2,
    OVERLAP_SRC = 4,
}EPhaseSrcType;

typedef enum tagLogicCtlMode
{
    CTL_MODE_SELFCTL		        = 0,						//��������
    CTL_MODE_FLASH			        = 1,						//����
    CTL_MODE_ALLRED			        = 2,						//ȫ��
    CTL_MODE_OFF			        = 3,						//�ص�
    CTL_MODE_MANUAL			        = 4,						//�ֶ�����
    CTL_MODE_FIXTIME		        = 5,						//�����ڿ���
    CTL_MODE_ACTUATE		        = 6,						//�����Ӧ����
    CTL_MODE_ADVACTUATE		        = 7,						//Э����Ӧ����
    CTL_MODE_SELPLAN		        = 8,						//����ѡ�����
    CTL_MODE_SINGLEOPTIM	        = 9,						//����Ӧ����
    CTL_MODE_CABLELESS		        = 10,						//�޵��¿���
    CTL_MODE_CABLEL			        = 11,						//�е��¿���
    CTL_MODE_PEDCROSTREET	        = 12,						//���˹���
	CTL_MODE_MANUAL_RECOVER			= 13,				        //�����ָ�����
	CTL_MODE_PHASE_STAY	            = 14,						//��λפ��
	CTL_MODE_CHANNEL_CHECK 	        = 15,						//ͨ�����
	CTL_MODE_CHANNEL_LOCK 	        = 16,						//ͨ������
	CTL_MODE_WEBSTER_OPTIM	        = 17,						//Webster�������
	CTL_MODE_ACTUATE_PEDCROSTREET   = 19,                       //��Ӧʽ���˹���
	CTL_MODE_PHASE_LOCK 	        = 22,						//��λ����
	CTL_MODE_PHASE_PASS_CONTROL		= 23,                       //��λ���п���
	CTL_MODE_PREEMPT		        = 24,                       //���ȿ���

	CTL_MODE_PANEL_TRAN             = 51,				        //�ֶ���岽������ʱ�Ŀ���ģʽֵ
	CTL_MODE_SYSTEM_TRAN            = 52,				        //ϵͳ�ֶ�������

	CTL_MODE_SYS_INTERRUPT          = 20,                       //������Ԥ
	CTL_MODE_MANUAL_CONTROL_PATTERN = 100,                      //�ֶ����Ʒ���

    CTL_MODE_UNDEFINE		        = 255,
}ELogicCtlMode;

typedef enum tagChannelSrc
{
	DISABLE_CHA			= 0,							//������	
    OTHER_CHA			= 1,							//����ͨ��	
    VEH_CHA				= 2,							//������ͨ��
    PED_CHA				= 3,							//����ͨ��
    OVERLAP_CHA 		= 4,							//����������ͨ��
	OVERLAP_PED_CHA		= 5,							//���˸���ͨ��
    LANEWAY_LIGHT_CHA	= 6,							//������
}EChannelSrc;

#define     OPENATC_RTN_OK		1
#define     OPENATC_RTN_FAILED	0

typedef enum tagSaveParameterFailedVal
{
	OPENATC_SAVE_PARAM_FAILED_USB_NOT_FIND				= 2,
	OPENATC_SAVE_PARAM_FAILED_USB_MOUNT_FAILED			= 3,
	OPENATC_SAVE_PARAM_FAILED_PARAM_UPDATE_FAILED		= 4,
	OPENATC_SAVE_PARAM_FAILED_DEVPARAM_UPDATE_FAILED	= 5,
}ESaveParameterFailedVal;

enum{
	HWPANEL_BTN_AUTO						= 0x00,
	HWPANEL_BTN_MANUAL						= 0x01,
	HWPANEL_BTN_STEP						= 0x02,
	HWPANEL_BTN_ALL_RED						= 0x03,
	HWPANEL_BTN_YELLOW_FLASH				= 0x04,
	HWPANEL_BTN_DIR_EAST_WEST_STRAIGHT		= 0x05,
	HWPANEL_BTN_DIR_EAST_WEST_TURN_LEFT		= 0x06,
	HWPANEL_BTN_DIR_SOUTH_NORTH_STRAIGHT	= 0x07,
	HWPANEL_BTN_DIR_SOUTH_NORTH_TURN_LEFT	= 0x08,
	HWPANEL_BTN_DIR_EAST					= 0x09,
	HWPANEL_BTN_DIR_WEST					= 0x0A,
	HWPANEL_BTN_DIR_SOUTH					= 0x0B,
	HWPANEL_BTN_DIR_NORTH					= 0x0C,
	HWPANEL_BTN_REMOTE_CTRL					= 0x0D,
	HWPANEL_BTN_Y1							= 0x0E,
	HWPANEL_BTN_Y2							= 0x0F,
	HWPANEL_BTN_Y3							= 0x10,
	HWPANEL_BTN_Y4							= 0x11,
};

enum{
	HWPANEL_DIR_EAST_WEST_STRAIGHT_INDEX	= 1,
    HWPANEL_DIR_NORTH_INDEX					= 2,
	HWPANEL_DIR_EAST_WEST_TURN_LEFT_INDEX	= 3,
    HWPANEL_DIR_WEST_INDEX                  = 4,
    HWPANEL_DIR_EAST_INDEX					= 5,
	HWPANEL_DIR_SOUTH_NORTH_STRAIGHT_INDEX	= 6,
    HWPANEL_DIR_SOUTH_INDEX					= 7,
	HWPANEL_DIR_SOUTH_NORTH_TURN_LEFT_INDEX	= 8,
	HWPANEL_Y1_INDEX						= 9,
	HWPANEL_Y2_INDEX						= 10,
	HWPANEL_Y3_INDEX						= 11,
	HWPANEL_Y4_INDEX						= 12,
	HWPANEL_REMOTE_CTRL_INDEX				= 13,
};
enum
{
	REMOTE_CONTROL_PAIR_REQUEST		= 0X00,
};

enum
{
	REMOTE_CONTROL_PAIR_FAIL		= 0X00,
	REMOTE_CONTROL_PAIR_SUCCESS		= 0X01,
};

typedef enum tagSplitMode
{
    OTHER_MODE				= 1,
    UNDEFINE_MODE			= 2,
    MINVEHICLE_MODE			= 3,
    MAXVEHICLE_MODE			= 4,
    PED_MODE				= 5,
    MAXVEHICLEANDPED_MODE	= 6,
    NEGLECT_MODE			= 7,
	SHIELD_MODE				= 8,
}ESplitMode;

typedef enum tagDetectorType
{
    LOOP_DETECTOR 	= 0,
    VIDEO_DETECTOR 	= 1,
}EDetectorType;

#define     OPENATC_PARAM_OUTLIERS			-1
#define     OPENATC_PARAM_CHECK_OK			1
#define     OPENATC_PARAM_CHECK_FAILED		0
#define     MAX_UP_LIMIT_VALUE				255
#define     MAX_UP_LIMIT_HOUR				23
#define     MAX_UP_LIMIT_MINUTE				59
#define     MAX_UP_LIMIT_MON				12
#define     MAX_UP_LIMIT_WEEK				6
#define     MAX_UP_LIMIT_DATE				31
#define     MAX_UP_LIMIT_CONTROL_NUM		12
#define     MAX_UP_LIMIT_LAMP_TIME			25


const int C_N_MAX_MD5BUFF_SIZE = 33;
const int C_N_MAX_ERR_SIZE = 50;
const int C_N_NUM1 = 1;
const int C_N_NUM2 = 2;
const int C_N_NUM3 = 3;
const int C_N_NUM4 = 4;
const int SPLITMODE_IGNORE = 7;
const int SPLITMODE_OTHER = 1;
const int HOUR_TO_MIN = 60;

typedef enum tagCheckPhaseCode
{
	CheckPhase_ID = 101,      //��λ��ų�����ֵ
	CheckPhase_PedClear = 102,      //��������ʱ�䳬����ֵ
	CheckPhase_MinGreen = 103,      //��С��Ӧ���������̵�ʱ��
	CheckPhase_Max1 = 104,      //�����1Ӧ������С��ʱ��
	CheckPhase_Max2 = 105,      //�����2Ӧ���������1ʱ��
	CheckPhase_Passage = 106,      //��λ�ӳ��̵�ʱ�䳬����ֵ
	CheckPhase_Yellow = 107,      //�Ƶ�ʱ�䳬����ֵ
	CheckPhase_RedClear = 108,      //ȫ��ʱ�䳬����ֵ
	CheckPhase_FlashGreen = 109,      //����ʱ��ӦС����С��
	CheckPhase_Ring = 110,      //������������ֵ
	CheckPhase_ConCurConflict = 111,		//��λ�������ó�ͻ
	CheckPhase_OneRingOnePhase = 112,		//���л�����ֻ��һ����λ
	CheckPhase_RingIndexStartFromFaultValue = 113,		//������Ӧ��1��ʼ����
	CheckPhase_RingIndexDiscontinuous = 114,		//������Ӧ��������
}ECheckPhaseCode;

typedef enum tagCheckOverlapCode
{
	CheckOverlap_ID = 201,		//������λ����������ֵ
	CheckOverlap_IncludePhaseNull = 202,		//������λ��ĸ��λΪ��
	CheckOverlap_IncludePhase = 203,		//������λ����δ֪ĸ��λ
}ECheckOverlapCode;

typedef enum tagCheckPatternCode
{
	CheckPattern_ID = 301,		//��������������ֵ
	CheckPattern_Offset = 302,		//��λ��ӦС������ʱ��
	CheckPattern_RingNoPhaseIndex = 303,		//��������δ֪��λ
	CheckPattern_Split = 304,		//���ű�Ӧ������λ����С��+�Ƶ�+ȫ��
	CheckPattern_PhaseConCurConflict = 305,		//�����д��ڻ�����λ������ͻ��������1��Ӧ����1��2��Ӧ����2��һ�����ƣ�108������108
	CheckPattern_InconsistentCycleTime = 306,		//�����д��ڸ���������ʱ����һ�£�������1��Ӧ����1��2��Ӧ����2��һ�����ƣ�108������108
}ECheckPatternCode;

typedef enum tagCheckDayPlanCode
{
	CheckDayPlan_ID = 401,		//�ƻ�����������ֵ
	CheckDayPlan_Control = 402,		//���Ʒ�ʽ������
	CheckDayPlan_DayPlanID = 403,		//ʱ������������ֵ
	CheckDayPlan_Minute = 404,		//���ӳ�����ֵ
	CheckDayPlan_Hour = 405,		//Сʱ������ֵ
	CheckDayPlan_TimeOrder = 406,		//ʱ��˳�����ô���
	CheckDayPlan_PatternID = 407,		//�ƻ�������δ֪����
	CheckDayPlan_PatternNull = 408,		//�ƻ��з���δ����
	CheckDayPlan_Null = 409,		//����δ���õļƻ�
}ECheckDayPlanCode;

typedef enum tagCheckScheduleCode
{
	CheckSchedule_ID = 501,		//���ȼƻ�����������ֵ
	CheckSchedule_Month = 502,		//�·ݳ�����ֵ
	CheckSchedule_Week = 503,		//���ڳ�����ֵ
	CheckSchedule_Date = 504,		//����ֵ������ֵ
	CheckSchedule_PlanFlag = 505,		//����δ֪�ƻ���
	CheckSchedule_AllYear = 506,	//������ͨ�ƻ�������δ����ȫ��
	CheckSchedule_NoMonth = 507,	//���ȼƻ�δ�����·�
}ECheckScheduleCode;

typedef enum tagCheckChannelCode
{
	CheckChannel_ID = 601,		//ͨ����������ֵ
	CheckChannel_ControlSource = 602,		//ͨ������δ֪����Դ
	CheckChannel_ControlSourceNull = 603,		//ͨ������Դδ����
	CheckChannel_ControlTypeNull = 604,		//ͨ����������δ����
	CheckChannel_ControlType = 605,		//ͨ��δ֪��������
}ECheckChannelCode;

typedef enum tagCheckVecDetetorCode
{
	CheckVecDetetor_ID = 701,		//�������������������ֵ
	CheckVecDetetor_NoActivity = 702,		//�������������Ӧʱ�䳬����ֵ
	CheckVecDetetor_MaxPresence = 703,		//���������������ʱ�䳬����ֵ
	CheckVecDetetor_ErraticCounts = 704,		//����������������������ֵ
	CheckVecDetetor_FailTime = 705,		//���������ʧ��ʱ�䳬����ֵ
	CheckVecDetetor_CallPhase = 706,		//�������������δ֪������λ
}ECheckVecDetetorCode;

typedef enum tagCheckPedDetetorCode
{
	CheckPedDetetor_ID = 801,		//���˼��������������ֵ
	CheckPedDetetor_NoActivity = 802,		//���˼��������Ӧʱ�䳬����ֵ
	CheckPedDetetor_MaxPresence = 803,		//���˼����������ʱ�䳬����ֵ
	CheckPedDetetor_ErraticCounts = 804,		//���˼�����������������ֵ
	CheckPedDetetor_FailTime = 805,		//���˼����ʧ��ʱ�䳬����ֵ
	CheckPedDetetor_CallPhase = 806,		//���˼��������δ֪������λ
}ECheckPedDetetorCode;

typedef enum tagCheckManualPanelCode
{
	CheckManualPanel_ID = 901,		//�ֶ��������δ֪ͨ��
	CheckManualPanel_NULL = 902,		//�ֶ�������δ����
	CheckManualPanel_GreenConflictKey1 = 903,		//�ֶ���嶫��ֱ�а���ͨ���̳�ͻ
	CheckManualPanel_GreenConflictKey2 = 904,		//�ֶ���山��ͨ�а���ͨ���̳�ͻ
	CheckManualPanel_GreenConflictKey3 = 905,		//�ֶ���嶫����ת����ͨ���̳�ͻ
	CheckManualPanel_GreenConflictKey4 = 906,		//�ֶ��������ͨ�а���ͨ���̳�ͻ
	CheckManualPanel_GreenConflictKey5 = 907,		//�ֶ���嶫��ͨ�а���ͨ���̳�ͻ
	CheckManualPanel_GreenConflictKey6 = 908,		//�ֶ�����ϱ�ֱ�а���ͨ���̳�ͻ
	CheckManualPanel_GreenConflictKey7 = 909,		//�ֶ��������ͨ�а���ͨ���̳�ͻ
	CheckManualPanel_GreenConflictKey8 = 910,		//�ֶ�����ϱ���ת����ͨ���̳�ͻ
	CheckManualPanel_GreenConflictKey9 = 911,		//�ֶ����Y1�Զ��尴��ͨ���̳�ͻ
	CheckManualPanel_GreenConflictKey10 = 912,      //�ֶ����Y2�Զ��尴��ͨ���̳�ͻ
	CheckManualPanel_GreenConflictKey11 = 913,      //�ֶ����Y3�Զ��尴��ͨ���̳�ͻ
	CheckManualPanel_GreenConflictKey12 = 914,      //�ֶ����Y4�Զ��尴��ͨ���̳�ͻ
	CheckManualPanel_DurationErr = 915,		//�ֶ�����������ʱ��Ӧ��С����С��ʱ��
}ECheckManualPanelCode;

typedef enum tagCheckChannelLockInfo
{
	CheckChannelLockInfo_Conflict = 1001,		//ʱ��%ͨ��״̬������ͻ   %����ʱ�α��
	CheckChannelLockInfo_Source = 1002,		//ʱ��%����ͨ���Ŀ���Դδ������   %����ʱ�α��
	CheckChannelLockInfo_ID = 1003,		//ͨ������ʱ������������ֵ
	CheckChannelLockInfo_IllegalCtrlSrc = 1004,		//�����������ͨ��������ͨ������״̬
}ECheckChannelLockInfo;

typedef enum tagCheckParamBaseInfo
{
	CheckParamBaseInfo_Check_MD5_Fail = 1101,		//MD5��ֵУ��ʧ��
	CheckParamBaseInfo_Update_Fail = 1102,		//��������ͬ��ʧ��
	CheckParamBaseInfo_JSON_Parse_Fail = 1103,		//����JSON��ʽ����
}ECheckParamBaseInfo;

typedef enum tagCheckDeviceParamInfo
{
	CheckDeviceParamInfo_SiteID_NULL = 2001,		//�źŻ���ַ��δ����
	CheckDeviceParamInfo_NetCards = 2002,		//�źŻ�����������δ����
	CheckDeviceParamInfo_Check_SiteID_Fail = 2003,		//�źŻ���ַ�����ô���
	CheckDeviceParamInfo_JSON_Parse_Fail = 2004,		//����JSON��ʽ����
	CheckDeviceParamInfo_Update_Fail = 2005,		//�豸����ͬ��ʧ��
	CheckDeviceParamInfo_NetCards_Illegal = 2006,		//�źŻ����÷Ƿ�������Ϣ
	CheckDeviceParamInfo_RoadID_Illegal = 2007,		//�źŻ�·��ID����0-65535֮��
}ECheckDeviceParamInfo;

typedef enum tagCheckUSBStatus
{
	CheckUSBStatus_USB_Mount_Fail = 3001,		//U�̹���ʧ��
	CheckUSBStatus_USB_Not_Find = 3002,		//δ�ҵ�U��
}ECheckUSBStatus;

typedef enum tagCheckSingleOptim
{
	CheckCheckSingleOptim_StaticWeight = 1201,		//Ȩ������ֵ��ӦΪ[0,100]
	CheckCheckSingleOptim_SumOfPhaseWeight = 1202,		//����λȨ�����ӺͲ�Ϊ100
	CheckCheckSingleOptim_Null = 1203,		//����λȨ����������Ϊ��
}ECheckSingleOptim;

typedef enum tagCheckChannelGreenConflict
{
	CheckChannelGreenConflict_Concurrency = 1301,		//��������λ��ͨ���������ó�ͨ���̳�ͻ
}ECheckChannelGreenConflict;

typedef enum tagCheckPartParam
{
	CheckParam_Pattern				            = 1,		//����
	CheckParam_Plan  					        = 2,		//�ƻ�
	CheckParam_Date  					        = 3,		//����
	CheckParam_OverLap 					        = 4,		//��λ
}ECheckPartParam;
