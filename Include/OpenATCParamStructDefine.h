/*=====================================================================
ģ���� �����ĵ���Ϊ�ѻ��ĵ��еĽṹ����
�ļ��� ��OpenATCParamStructDefine.h
����ļ���
ʵ�ֹ��ܣ��źŻ������ṹ�嶨��
���� ��
��Ȩ ��<Copyright(c) 2019-2020 Suzhou Keda Technology Co.,Ltd. All right reserved.>
-----------------------------------------------------------------------
�޸ļ�¼��
�� ��           �汾     �޸���     �߶���     �޸ļ�¼
2019/9/14       V1.0               
=====================================================================*/
#ifndef OPENATCPARAMSTRUCTDEFINE_H
#define OPENATCPARAMSTRUCTDEFINE_H

#include "../Include/OpenATCParamConstDefine.h"

#pragma warning (disable:4819)
//#pragma pack(push)//�������״̬
//#pragma pack(1)  //����Ϊ1�ֽڶ���

typedef struct tagAscStepCfg
{
	BYTE m_byStepType;			//Ĭ��0 ������1 ɫ��
}TAscStepCfg, *PTAscStepCfg;

typedef struct tagAscFaultCfg
{
	BYTE m_byIsCloseGreenAndRedOn;			//Ĭ��1 Ϊ������0 Ϊ�ر�
	WORD m_wDetectGapTimeByGreenAndRedOn;	//����ͬ�����ʱ����
	BYTE m_byIsCloseNoRedOn;
	WORD m_byDetectGapTimeByNoRedOn;		//�޺��������ʱ����
	WORD m_byDetectGapTimeByGreenConflict;	//�̳�ͻ���ʱ����
}TAscFaultCfg, *PTAscFaultCfg;

//��ťͨ������
typedef struct tagAscChannelCfg
{
	BYTE m_byChannelID;           //ͨ�����  (Ϊ����ͨ�����)1-32
	BYTE m_byChannelStatus;       //ͨ��״̬   1--���  2--�Ƶ�  3--�̵�  4--����  5--���     
}TAscChannelCfg, *PAscChannelCfg;

typedef struct tagAscRemoteControl
{
	BYTE m_byControlType;
	WORD m_wControlTime;
}TAscRemoteControl, *PAscRemoteControl;
//����ť������Ϣ
typedef struct tagAscPanelOneKeyCfg
{
	BYTE m_byKeyNum;         /*    1--����ֱ��
							 2--����ͨ��
							 3--������ת
							 4--����ͨ��
							 5--����ͨ��
							 6--�ϱ�ֱ��
							 7--����ͨ��
							 8--�ϱ���ת
							 9--Y1�Զ��尴ť
							 10--Y2�Զ��尴ť
							 11--Y3�Զ��尴ť
							 12--Y4�Զ��尴ť*/
	char m_cPanelOneKeyDesc[MAX_DESC_LENGTH];		   /*����*/
	TAscChannelCfg m_ChannelCfg[MAX_CHANNEL_COUNT];    //��������ť��Ӧ��ͨ��������Ϣ
}TAscPanelOneKeyCfg, *PAscPanelOneKeyCfg;

//�ֶ��������
typedef struct tagAscManualPanel
{
	WORD m_wDuration;                  //�������������ʱ��  0--������ʱ��   1-600�� 
	BYTE m_byGreenFlash;               //��������ʱ��    ��λ��   ��Χ0-100����
	BYTE m_byYellow;                   //���ɻƵ�ʱ��   ��λ��   ��Χ0-100����
	BYTE m_byRedClear;                 //����ȫ��ʱ��   ��λ��   ��Χ0-100����
	BYTE m_byMinGreen;                 //��С��ʱ��    ��λ��   ��Χ0-255����
	BYTE m_byMinGreenIgnore;           //��С���Ƿ���� 0 ������ 1 ����
	TAscPanelOneKeyCfg m_stPanelKeyCfg[MAX_PANEL_KEY_CFG_COUNT];
}TAscManualPanel, *PAscManualPanel;

//�źŻ�ʶ���루��ַ�ĵ�ַ�룩�ṹ�嶨��
typedef struct tagAscAddressCode
{
	char m_chAscAddressCode[MAX_SITEID_COUNT];
}TAscAddressCode, *PTAscAddressCode;

//�źŻ�������Ϣ���� �ṹ�嶨��
typedef struct tagAscNetCard
{
	char m_chAscNetCardIp[MAX_IPSTR_LEN];		//ip
	char m_chAscSubnet[MAX_IPSTR_LEN];			//��������
	char m_chGateway[MAX_IPSTR_LEN];			//����
}TAscNetCard, *PTAscNetCard;

//�źŻ�ʱ����Ϣ���� �ṹ�嶨��
typedef struct tagAscTimeZoneInfo
{
	int		m_nAscHour;							//ʱ(��Ϊ������Ϊ��)
	int		m_nAscMinute;						//��
}TAscTimeZoneInfo, *PTAscTimeZoneInfo;

//ƽ̨ip��port����
typedef struct tagAscCenter
{
	char m_chAscCenterIp[MAX_IPSTR_LEN];		//ip
	int m_chAscAscCenterPort;					//port
}TAscCenter, *PTAscCenter;

//����ƽ̨����
typedef struct tagAscSimulate
{
	char m_chSimulateIP[MAX_IPSTR_LEN];			//ip
	int  m_nSimulatePort;						//port
	int  m_nCfgPort;
    int  m_nDetectorPort;
	int  m_nVideoDetectorPort;
}TAscSimulate, *PTAscSimulate;

//��������Ӧ���Ʋ���
typedef struct tagAscSingleOptim
{
	BYTE m_byPattern;												//��������Ӧ���ƹ���������
	BYTE m_bySelfLearn;												//����������ѧϰ����
	int	m_nMaxSaturatedFlow;										//��󱥺�����
	float m_fGreenStartUpLoss;										//�̵�������ʧʱ��
	float m_fYellowEndLoss;											//�Ƶƽ�����ʧʱ��
	int m_fCycleAdjustFactor;										//���ڵ�������
	float m_fStaticWeight;											//��̬Ȩ��
	float m_fPhaseStaticWeight[MAX_RING_COUNT][MAX_PHASE_COUNT];	//��λ��̬Ȩ������
}TAscSingleOptim, *PTAscSingleOptim;

//�źŻ����Ͻṹ�嶨��
typedef struct tagAscFault
{
	WORD m_wFaultID;            //����ID,���ڱ�ʶ���ϣ����ں��ڹ���ɾ�������ϻָ������ϲ�ѯ��ʶ���Ƿ�Ϊͬһ���ϵȹ��� ֵ1000��
	BYTE m_byBoardType;         //���ϰ忨����    ���ذ�--1���ƿذ�--2��   �����--3��   I/O��--4
	long m_unFaultOccurTime;    //���Ϸ�����ʱ��   ���������ʱ�������
	long m_unFaultRenewTime;    //���ϻָ���ʱ��   ���������ʱ�������
	BYTE m_byFaultLevel;        //���������ȼ�   һ�����--1����������--2�����ع���--3
	WORD m_wFaultType;          //����������
	WORD m_wSubFaultType;		  //����������	������������ֵΪ0��
	                           /* m_wFaultType
									101          CanBus Fault              can����ͨ�Ź���
									102           Yellow Flasher Fault      ����������
									------------------------------------------------------------------
									103       TZParam Fault             ������������
									                m_wSubFaultType     
									          ----->      1    Non-existent           ��������������
									          ----->      2    File Is Unreadable     ���������ļ����ɶ�
										     ----->      3    File Artificial Changes   ����������Ϊ�޸�
										     ----->      4    File Open Fail         ���������ļ���ʧ��
										     ----->      5    File Update Fail       ���������ļ�����ʧ��
										     ----->      6    File Check SiteID Fail   �źŻ���ַ��У��ʧ��
										     ----->      7    Format Error           �����������ݸ�ʽ����
									------------------------------------------------------------------
									104     FaultDet Offline          ���ϼ��岻����
									201     LampBoard ID Fault        �ƿذ�ID����
									202     LampBoard Offline         �ƿذ��ѻ�
									203     No Red Lamp Is On         �޺������
									204     Red And Green Conflict    ����ͬ��
									205     Green Conflict            �̳�ͻ
									-----------------------------------------------------------------
									206    Red Lamp Voltage  Fault     ��ƵƵ�ѹ����
									207    Yellow Lamp Voltage  Fault  �ƵƵƵ�ѹ����
									208    Green Lamp Voltage  Fault   �̵ƵƵ�ѹ����
									               m_wSubFaultType     
									             ---->  1    Output Volatage Is Fail      δ�����Ч��ѹ
									             ---->  2    Output Volatage Is Low     �����ѹ���������ѹ����
									             ---->  3    Output Volatage Is High      �����ѹ���������ѹ
											    ---->  4    Off Output Volatage Is high      �ر������ʵ�ʵ�ѹ��Ȼ���
											    ---->  5    Off Output Volatage Is low      �ر������ʵ�ʵ�ѹ�������
								                 ---->  6    Residual Voltage Is Over-High      ��·������ѹ����

									------------------------------------------------------------------

									209   Red Lamp Lamp Power Fault       ��Ƶƹ��ʹ���
									210   Yellow Lamp Lamp Power Fault    �ƵƵƹ��ʹ���
									211   Green Lamp Lamp Power Fault     �̵Ƶƹ��ʹ���
									               m_wSubFaultType     
									           ---->    1   Output Power Is Up           �����쳣����
									           ---->    2   Output Power Is Down         �����쳣����
									           ---->    3   Output Power Is Zero         ���������
									           ---->    4   Off Output Power Is High     �ر�״̬�й������
									-----------------------------------------------------------------
									301      VehDetBoard Is Not Init         �����δ��ʼ��
									302      VehDetBoard Is Offline          ������ѻ�
									303      VehDetector Short Circiut       �����������·
									304      VehDetector Open  Circiut       �����������·
																	*/
								
	BYTE m_byFaultSupple[MAX_CHANNEL_COUNT]; //���ϲ���˵��ֵ
							/*
							  ֵ��ʾ�忨��š�ͨ�����
							  ��m_wFaultDescValueΪ����ֵʱ����ʾ��Ӧͨ�����߰忨���
							*/
	

}TAscFault, *PAscFault;

//�źŻ�������¼�ṹ�嶨��
typedef struct tagAscOperationRecord
{
    WORD m_wRecordID;										//������¼ID�����ڱ�ʶ������¼
	long m_unStartTime;										//������¼����ʼʱ��   ���������ʱ�������
    long m_unEndTime;										//������¼�Ľ���ʱ��   ���������ʱ�������
	BYTE m_bySubject;										//�����ʶ  ƽ̨-0���������-1����ȡ�ֶ����-2
    BYTE m_byObject;										//�����ʶ  һ��ָ�������Ķ���
    int  m_nInfoType;										//�¼����� 
    bool m_bStatus;											//����״̬ true-�ɹ� false-ʧ��
 	BYTE m_byFailureValue[MAX_OPERATIONRECORD_DESC_LEN];	//����ʧ������                           							
}TAscOperationRecord, *PTAscOperationRecord;

//��������
typedef struct tagAscCasCadeInfo
{
	BYTE m_byLocalLampBoardNum;								//�����ƿذ�����
	BYTE m_byLocalVehDetBoardNum;							//�������������
	BYTE m_byLocalIOBoardNum;								//����IO������  
	BYTE m_byJoinOffset;									//����ƫ����
}TAscCasCadeInfo, *PTAscCasCadeInfo;

typedef struct tagAscChannelLockInfo
{
	BYTE m_byChannelID;										//ͨ�����
	BYTE m_byChannelLockStatus;								//ͨ������״̬   0 Ϊ��ָ��״̬     1--���  2--�Ƶ�  3--�̵�  4--����  5--���   6--����
}TAscChannelLockStatus, *PAscChannelLockStatus;

//ͨ����������
typedef struct tagAscOnePlanChannelLockInfo
{
	BYTE m_byStartHour;     //��ʼʱ��ʱ
	BYTE m_byStartMin;      //��ʼʱ���
	BYTE m_byStartSec;      //��ʼʱ����

	BYTE m_byEndHour;       //����ʱ��ʱ
	BYTE m_byEndMin;        //����ʱ���
	BYTE m_byEndSec;        //����ʱ����

	BYTE m_byGreenFlash;    //��������ʱ��
	BYTE m_byYellow;       //���ɻƵ�ʱ��
	
	TAscChannelLockStatus m_stChannelLockStatus[MAX_CHANNEL_COUNT];   

}TAscOnePlanChannelLockInfo, *PTAscOnePlanChannelLockInfo;



//�����������Ʋ���
typedef struct tagVehDetCtrParam
{
	BYTE m_byEnable;    //ʹ��
	BYTE m_byMode;       //ģʽ  0��Ĭ�ϣ�1��Boostģʽ
	WORD m_wPresentTime;/*����ʱ��  ��λ�룬
						�������ʱ�����õļ��Ŀ����Զ������
						�������Ϊ0����ʵ�ʶ�Ӧ240��*/
	BYTE m_bySense;     /*������  Sense��0~9Ԥ�������ȣ���ֵԽ��������Խ�ߣ�
						һ���ʼ����3��Ԥ�������Ⱥ���ļ���������������9����Ϊ�Զ��壬
						�Զ����ʱ����漸�������Ż��������*/
	WORD m_wSampleTime;  /*����ʱ�䣬��λ΢�룬ʱ��Խ���ۻ������Խ�ã����ǻᵼ�²��پ��ȱ���Ӧ����*/
	WORD m_wInThresh;    /*������ֵ ԽСԽ��������Ȼ������Ҳ�ߣ�һ�㷶Χ50~100*/
	WORD m_wOutThresh;   /*�뿪��ֵ  �뿪��ֵ��ԽСԽ������һ��20~30��һ����InThreshС��������������*/
	WORD m_wInFilter;      /*�����˲� ����������������ٴβ���Ϊ�����������ȷ��һ��3~5*/
	WORD m_wOutFilter;     /*�뿪�˲� ,����������ٴ��뿪����Ϊ�뿪��һ��3~5*/

}TVehDetCtrParam, *PVehDetCtrParam;

//�ƿذ���Ʋ���
typedef struct tagLampBoardCtrParam
{
	WORD m_wVoltThresh;   /*�Ƶ�ѹ�����ֵ ��λV��ĿǰĬ��50V��
						   ���������ѹ��Ϊ�Ƶ�ѹ�������0��������1���������Ŀǰ����Ϣ���ֳ����п���Ҫ��*/
	WORD m_wPacThresh;     /*�ƹ��ʼ����ֵ ��λ0.1W��ĿǰĬ��30��Ҳ���ǹ��ʳ���3W���*/
	WORD m_wPeakHThresh;   /*�߹��ʹ�����ֵ �ٷֱȣ�Ҳ���ǵƿؼ��˲ʱ���ʳ���ƽ�����ʶ�����Ϊ���ϣ�ĿǰĬ��400������400%��
						   �൱�ڳ���ƽ������4���ű���*/
	WORD m_wPeakLThresh;   /*�͹��ʹ�����ֵ ˲ʱ���ʵ���ƽ�����ʵ�100/LThresh������ĿǰĬ��130��
						   Ҳ���ǵ��ڵ�ǰƽ�����ʵ�100/130����*/
}TLampBoardCtrParam, *PLampBoardCtrParam;

//�豸����������
typedef struct tagAscArea
{
	unsigned char m_chAscRegionNo;					//�����
	unsigned short m_usAscRoadNo;					//·�ں�
}TAscArea, *PTAscArea;

//�汾�Žṹ����
typedef struct tagVersionNumber
{
    DWORD m_dwMajorVersion;							//���汾�ţ����ڵ���1����������С��DWORD�����Χ
    DWORD m_dwMinorVersion;							//�ΰ汾�ţ����ڵ���0����������С��DWORD�����Χ
}TVersionNumber, *PVersionNumber;


//�źŻ����������汾�ṹ����
typedef struct tagAscParamFileHeader
{
	TVersionNumber m_stAscParamFileVersion;			//�źŻ����������ѻ��ļ��汾��
	BYTE m_byCtrlSpotAscType;						//���Ƶ��źŻ����ͣ�1-����ڡ�2-����·���ڡ�3-����·��ڡ�4-�����ơ���
	BYTE m_byAscParamFileAuthor;					//�źŻ��ѻ��ļ������ߣ�1-���ϵ��ԡ�2-�źŻ�ά�����ߡ�3-����ϵͳ�ͻ��ˡ���
	TVersionNumber m_stAscParamFileAuthorVer;		//�źŻ��ѻ��ļ����ߵİ汾��
	DWORD m_dwFileHeaderReserve;					//�ļ�ͷ�ı����ֽڣ������Ժ����չ	
}TAscParamFileHeader, *PTAscParamFileHeader;


//��λ����
typedef struct tagPhase
{
	BYTE m_byPhaseNumber;			/*2.2.2.1 ��λ�ţ����ܳ���maxPhases�������ֵ��(1..32)*/
	WORD m_wPhaseWalk;				/*2.2.2.2���˷���ʱ�䣬�������˹����̵Ƶ�������(0..255),second*/
	BYTE m_byPhasePedestrianClear;	/* 2.2.2.3 �������ʱ�䣨��������ʱ�䣩(0..255)*/
	WORD m_wPhaseMinimumGreen;		/* 2.2.2.4��С�̵�ʱ�䣬�����С�̡�															һ�������ڼ������ͣ���߼�ĳ����Ŷ����ȷ����(0..255)*/
	BYTE m_byPhasePassage;			/*2.2.2.5��λ�̵��ӳ�ʱ��(0-25.5 sec)����Ƶ�λ�ӳ��̡�(0..255)*/
	WORD m_wPhaseMaximum1;			/* 2.2.2.6����̵�ʱ��1����������1��(0..255)*/
	WORD m_wPhaseMaximum2;			/* 2.2.2.7����̵�ʱ��2����������2��(0..255)*/
	BYTE m_byPhaseYellowChange;		/*2.2.2.8��λ�Ƶ�ʱ��*/
	BYTE m_byPhaseRedClear;			/*2.2.2.9��λ������ʱ��*/
	BYTE m_byPhaseRedRevert;			/*2.2.2.10��֤�Ƶ��Ժ���ֱ�ӷ����̵ƿ��Ƶĺ��ʱ�䱣��*/
	BYTE m_byPhaseAddedInitial;		/*2.2.2.11��λ���ӳ�ʼֵ*/
	BYTE m_byPhaseMaximumInitial;	/*2.2.2.12��λ����ʼֵ*/
	BYTE m_byPhaseTimeBeforeReduction;/*2.2.2.13 gap�ݼ�֮ǰ��ʱ����2.2.2.14�ۺ�ʹ��*/
	BYTE m_byPhaseCarsBeforeReduction;/*2.2.2.14 gap�ݼ�֮ǰͨ���ĳ�����*/
	BYTE m_byPhaseTimeToReduce;		/*2.2.2.15 gap�ݼ���minigap��ʱ��*/
	BYTE m_byPhaseReduceBy;			/*2.2.2.16��λ�ݼ��ʣ���2.2.2.15��16��ѡһ��*/
	BYTE m_byPhaseMinimumGap;		/*2.2.2.17���Եݼ�������Сgap��Ӧ�ú�phaseTimeToReduce�ۺ�ʹ��*/
	BYTE m_byPhaseDynamicMaxLimit;	/*2.2.2.18����MAX���޶�ֵ����С�ڴ�ֵʱ������MAX1����֮����MAX2*/
	BYTE m_byPhaseDynamicMaxStep;	/*2.2.2.19��̬��������*/
	BYTE m_byPhaseStartup;			/*2.2.2.20��λ��ʼ������
									other(1)��λ��ʹ�ܣ��Ƕ��壩��־λ������phaseOption��bit0=0����phaseRing=0��
									phaseNotON(2)��λ��ʼΪ�죨�ǻ��
									greenWalk(3)��λ��ʼΪ��С�̺�����ʱ��
									greenNoWalk(4)��λ��ʼΪ��С�̵Ŀ�ʼ
									yellowChange(5)��λ��ʼΪ�Ƶƿ�ʼ
									redClear(6)��λ��ʼ��Ϊ��ƿ�ʼ*/
	WORD m_wPhaseOptions;			/*2.2.2.21��λѡ��
									Bit 0 - Enabled Phase��λʹ��
									Bit 1 �C �������Զ��������ʱ������Ϊ1ʱ����λ�����������������ǰ�Ƚ��к�Ʋ���
									Bit 2 �C ���������ʱ������Ϊ1ʱ�������д���λ��
									Bit 3 �C �Ǹ�Ӧ1
									Bit 4 -  �Ǹ�Ӧ2
									Bit 5 �C ����Ϊ0ʱ��������������λ�Ƶƿ�ʼ��¼������Ϊ1ʱ��������������������detectorOptions����
									Bit 6 - Min Vehicle Recall��Ϊ����Ϊ��С������
									Bit 7 - Max Vehicle Recall��Ϊ����Ϊ���������
									Bit 8 - Ped Recall��Ϊ����Ϊ��������
									Bit 9 - Soft Vehicle Recall��������λû��ʵ����������еĳ�ͻ��λ��MAX RECALLʱ������Ҳû������פ�����̵�ʱ��������λһ��soft recall��ʹ��λת��������λ��
									Bit 10 - Dual Entry Phase������Ϊ1ʱʹһ��û�м�����������λ������һ������ͬʱ���е���λһ����С�
									Bit 11 - Simultaneous Gap Disable������Ϊ1ʹ������Gap����
									Bit 12 - Guaranteed Passage������Ϊ1ʱ��֤��Ӧ��λ���ȫ�̣�passage��
									Bit 13 - Actuated Rest In Walk��1�����Ӧ��λ�ڳ�ͻ��û������ʱ�������˷���
									Bit 14 - Conditional Service Enable����λʱ��û���꣬��ʣ���ʱ�����ͬһ��barrier����λ��
									Bit 15 - Added Initial Calculation������Ϊ1ʱ��ѡ�����������ֵ������Ϊ0ʱ��ѡ�������ļӺ͡�*/
	BYTE m_byPhaseRing;				/*2.2.2.22�õ�����λ��ring���*/
	BYTE m_byPhaseConcurrency[MAX_PHASE_CONCURRENCY_COUNT];		/*2.2.2.23���Բ�������λ��ÿ���ֽڴ���һ���ɲ�������λ��*/
	BYTE m_byRedYellow;				//���ʱ��(BYTE)		
	BYTE m_byGreenFlash;				//����ʱ��(BYTE)		
	BYTE m_bySafeRed;				//��ȫ���ʱ��(BYTE)	
	BYTE m_byRedPulse;				//������ʱ��
	BYTE m_byGreenPulse;			//������ʱ��
	WORD m_wVehicleQueueThresh;		//�����Ŷ���ֵ
	WORD m_wPedestrianWaitThresh;	//���˵ȴ���ֵ
	BYTE m_byPulseType;				// ��������   0���ر����˼����������壻 1�����ͻ��������壻2�������������� ��3 ���������˼�����������
	BYTE m_byForbiddenFlag;         //��λ��ֹ��־ -GB20999
	BYTE m_byScreenFlag;			//��λ���α�־ -GB20999
}TPhase, *PTPhase;


//������λ����
typedef struct tagOverlapTable
{
	BYTE m_byOverlapNumber;			/*2.10.2.1 overlapNumber��overlap�ţ�	������maxOverlaps��1 = Overlap A, 2 = Overlap B etc */
	BYTE m_byOverlapType;				/*2.10.2.2 overlap�������ͣ�ö�����£�
									other(1) ��δ�ڴ������Ĳ������͡�
									normal(2)�����ֲ�������ʱ��overlap�������overlapIncludedPhases�������ơ�����������ʱoverlap����̵ƣ�
									��overlap��������λ���̵�ʱ��
									��overlap��������λ�ǻƵƣ�����ȫ��red clearance����overlap������һ��λ��included phase is next��ʱ��
									���overlap��������λ�ǻƵ���overlap��������һ��λ��included phase is not next����overlap����Ƶơ����overlap���̵ƺͻƵ���Ч���������ơ�
									minusGreenYellow(3)�����ֲ�������ʱ��overlap�������overlapIncludedPhases��overlapModifierPhases�������ơ�����������ʱoverlap����̵ƣ�
									��overlap������λ���̵���overlap��������λ�����̵�ʱ��NOT green��
									��overlap��������λ�ǻƵƣ�����ȫ��red clearance����overlap������һ��λ��included phase is next����overlap��������λ�����̵�ʱ��
									���overlap��������λ�ǻƵ���overlap��������λ���ǻƵ���overlap��������һ��λ��included phase is not next����overlap����Ƶơ����overlap���̵ƺͻƵ���Ч���������ơ�*/
	BYTE m_byArrOverlapIncludedPhases[MAX_PHASE_COUNT_IN_OVERLAP];	/*2.10.2.3 overlap��������λ��ÿ�ֽڱ�ʾһ����λ�š�*/
	BYTE m_byArrOverlapModifierPhases[MAX_PHASE_COUNT_MO_OVERLAP];	/*2.10.2.4 overlap��������λ��ÿ�ֽڱ�ʾһ����λ�š�	���Ϊ��ֵ��null����overlapTypeΪnormal�����Ϊ�ǿ�ֵ��non-null����overlapTypeΪminusGreenYellow��*/
	BYTE m_byOverlapTrailGreen;		/*2.10.2.5  0-255�룬	����˲�������0��overlap���̵�����������overlap�̵ƽ��ӳ��˲����趨��������*/
	BYTE m_byOverlapTrailYellow;		/*2.10.2.6  3-25.5�롣���overlap���̵Ʊ��ӳ�	��Trailing Green�����㣩���˲���������overlap Yellow Change��ʱ�������ȡ�*/
	BYTE m_byOverlapTrailRed;			/*2.10.2.7  0-25.5�롣���overlap���̵Ʊ��ӳ�	��Trailing Green�����˲���������overlap Red Clearance��ʱ�������ȡ�*/
	BYTE m_byPulseType;				// ��������   0���ر����˼����������壻 1�����ͻ��������壻2�������������� ��3 ���������˼�����������
}TOverlapTable, *PTOverlapTable;


//��������
typedef struct tagPattern
{
	BYTE m_byPatternNumber;						/*2.5.7.1 �������и��������ĸ�����*/
	WORD m_wPatternCycleTime;					/*2.5.7.2 �������ڳ�,һ���޸�Ӧ��������λ��С���������
													miniGreen��Walk��PedClear��yellow��Red
													һ����Ӧ��������λ��С���������miniGreen+Yellow+Red */
	WORD m_byPatternOffsetTime;					/*2.5.7.3 ��λ���С*/
	BYTE m_byPatternSplitNumber;				/*2.5.7.4������Ӧ�����űȱ��*/
	BYTE m_byPatternSequenceNumber;				/*2.5.7.5������Ӧ��sequence���*/
	char m_cPatternDesc[MAX_DESC_LENGTH];		/*����*/
}TPattern, *PTPattern;


//���űȶ���
typedef struct tagSplit
{
	BYTE m_bySplitNumber;			/* 2.5.9.1�������ű���ţ�һ�����ڵĸ��ֶ���ͬ*/
	BYTE m_bySplitPhase;			/*2.5.9.2���ж�Ӧ����λ��*/
	WORD m_wSplitTime;			/*2.5.9.3��Ӧ���ű���λ��ʱ��*/
	BYTE m_bySplitMode;			/*2.5.9.4���ڸ���λӦ��β���
								    1---����
  								    2---δ���壬�����űȿ���ģʽ
								    3---��С������Ӧ����Ӧ����ʱ����������λ��ǿ��ֱ����С�̡����������ȼ�������λ�����еġ��������Զ���������
									4---�������Ӧ����Ӧ����ʱ����������λ��ǿ��ֱ������̡����������ȼ�������λ�����еġ��������Զ���������
									5---������Ӧ����Ӧ����ʱ��������λ��ǿ�ƻ�ȡ����Ȩ�����������ȼ�������λ�����еġ������Զ���������
									6---�����/������Ӧ����Ӧ����ʱ����������λ��ǿ��ֱ������̣�������λ��ǿ�ƻ�ȡ����Ȩ�����������ȼ�������λ�����еġ��������Զ��������Ժ͡������Զ���������
									7---������λ������λ�����ű�ģʽ�£��ӷ�����ȥ����
									8---�ض���λ������λ�����Ա�ģʽ�£�һֱ���ں�ơ�
									*/
	BYTE m_bySplitCoordPhase;		/*2.5.9.5�����Ƿ���ΪЭ����λ����*/
}TSplit, *PTSplit;


//������
typedef struct tagSequence
{
	BYTE m_bySequenceNumber;			/*2.8.3.1ʱ�򷽰���*/
	BYTE m_bySequenceRingNumber;		/* 2.8.3.2ʱ�򷽰���Ӧ��ring��*/
	BYTE m_bySequenceData[MAX_SEQUENCE_DATA_LENGTH];		/*2.8.3.3 ���֧�ֵ�ring����*/
}TSequence, *PTSequence;

//�׶�������
typedef struct tagStagesList
{
	WORD m_wSplitTimeInStage;			                       /*�׶���������ű�ʱ��*/
	BYTE m_byPhaseInStage[MAX_RING_COUNT];		               /*�׶��������λ��Ϣ*/
}TStagesList, *PTStagesList;

//�׶ζ���
typedef struct tagStages
{
	BYTE m_byPhaseInStage[MAX_RING_COUNT];		               /*�׶��������λ��Ϣ*/
}TStages, *PTStages;

//���϶���
typedef struct tagBarrier
{
	BYTE m_byBarrierNumber;			                           /*���ϱ��*/
	BYTE m_byBarrierLength;		                               /*����ʱ��*/
	BYTE m_byRingCount;                                        /*��������Ļ�����*/
	BYTE m_byRingNumber[MAX_RING_COUNT];                       /*��������Ļ���Ϣ*/
	BYTE m_byPhaseInBarrier[MAX_RING_COUNT][MAX_PHASE_COUNT];  /*��������Ļ���Ӧ����λ��Ϣ*/
}TBarrier, *PTBarrier;

//ʱ����������
typedef struct tagTimeBaseVariable
{
	WORD m_wTimebaseAscPatternSync;	/*2.6.1�ھ�����ҹ�ܶ�ʱ���ڵķ���ͬ���ο�������Ϊ0XFFFFʱ���źſ��Ƶ�Ԫ����ACTION TIME��Ϊ������ͬ���ο���*/
	BYTE m_byTimebaseAscActionStatus;/*2.6.4 ������������ǰ�õ���ʱ����š�*/
	DWORD m_dwGlobalTime;				/*2.4.1 UTC����GMT��ʱ�䣬��1970/1/1 00:00:00���������*/
	BYTE m_byGlobalDaylightSaving;	/*2.4.2����ʱ
									other (1)��DST�������û���ڱ���׼�С�
									disableDST (2)����ʹ��DST
									enableUSDST (3)��DSTʹ������ϰ��
									enableEuropeDST (4)��DSTʹ��ŷ��ϰ��
									enableAustraliaDST (5)��DSTʹ�ðĴ�����ϰ��
									enableTasmaniaDST (6) DSTʹ����˹������ϰ��*/
	BYTE m_byDayPlanStatus;			/*2��4��4��4��ʾ�ʱ�α�ı�š�0��ʾû�л��ʱ�α�*/
	int m_iGlobalLocalTimeDifferential;	/*2��4��5 ʱ��*/
	int m_iGcontrollerStandardTimeZone;	/*2��4��6 ���ر�׼ʱ����GMT��ʱ��룩����ֵ��ʾ����ʱ���ڶ����򣬸�ֵ��ʾ����ʱ����������*/
	DWORD m_dwControllerLocalTime;	/*2��4��7 ����ʱ�䣬����1970/1/1 00:00:00����������*/
}TTimeBaseVariable, *PTTimeBaseVariable;


//��������
typedef struct tagTimeBaseAscAction
{
	BYTE m_byTimebaseAscActionNumber;	/* 2.6.3.1�к�*/
	BYTE m_byTimebaseAscPattern;		/* 2.6.3.2��Ӧ�����š�����������ܳ���maxPatterns, flash,����free��ֵ������Ϊ0����û�з�����ѡ��*/
	BYTE m_byTimebaseAscAuxillaryFunction;/* 2.6.3.3��λ���*/
	BYTE m_byTimebaseAscSpecialFunction;/* 2.6.3.4ͨ�����*/
}TTimeBaseAscAction, *PTTimeBaseAscAction;


//���ȼƻ�����
typedef struct tagTimeBaseSchedule
{
	WORD m_wTimeBaseScheduleNumber;	    /*2.4.3.2.1 ���ȼƻ��ţ���timeBaseScheduleMonth��timeBaseScheduleDate��timeBaseScheduleDate��timeBaseScheduleDayPlan�ĸ�������ͬ�����ƻ��Ƿ����ִ�С�(1..40)*/
	WORD m_wTimeBaseScheduleMonth;		/*2.4.3.2.2 bit1-bit12��ÿλ��ʾһ���¡���1��ʾ�����Ӧ�ƻ��ڸ���ִ�С�(0..65535)*/
	BYTE m_byTimeBaseScheduleDay;		/*2.4.3.2.3 bit1-bit7��ÿλ��ʾһ���е�һ�졣��1��ʾ�����Ӧ�ƻ��ڸ���ִ�С�(0..255)*/
	DWORD m_dwTimeBaseScheduleDate;		/* 2.4.3.2.4 bit1-bit31��ÿλ��ʾһ���е�һ�졣��1��ʾ�����Ӧ�ƻ��ڸ���ִ�С�(0..4294967295)*/
	BYTE m_byTimeBaseScheduleDayPlan;	/*2.4.3.2.5ʱ�α�ţ�ָ��timeBaseScheduleDayPlan��0��ʾ������Ч��(0..255)*/
	char m_cTimeBaseScheduleDesc[MAX_DESC_LENGTH];		/*����*/
}TTimeBaseSchedule, *PTTimeBaseSchedule;


//ʱ�ζ���
typedef struct tagTimeBaseDayPlan
	/*�������������maxDayPlans��maxDayPlanEvents object�ĳ˻���Day Plan Number��Day Plan Event Number����1��ʼ������
	���źŻ��У�action�����źſ��Ʒ���pattern����action����λϵͳ�������������ͻʱ���źŻ�Ӧ�ø���Ԥ�ȶ�������ȼ�ѡ��ִ���ĸ����ֻ��DayPlan��Ч�Ҵ��ڻʱ�䣬����action�����ȼ������豸��ǰ���������ȼ�ʱ��ʱ�α����action�ſ��Ա�ִ�С�
	�������action��ͬһʱ��ִ�У���ִ�н�����豸��صġ�������������ı�globalTimeʱ��Ӧ������ʹ��ǰ24Сʱ���ڵ���һ��������ִ�е�event��������ʹ�õ�ǰʱ���event��*/
{
	BYTE m_byDayPlanNumber;			/*ʱ�α��(1..16)������*/
	BYTE m_byDayPlanEventNumber;		/*ʱ�Σ��¼����š�(1..48)������
										������ͬ���¼�������һ���еĲ�ͬʱ��ִ�С���������¼����ֵ�ʱ����ͬ����ʱ�κ�С����ִ��*/
	BYTE m_byDayPlanHour;			    /*��ʼִ��ʱ�̵�����������ʱ�䣨24ʱ�ƣ���*/
	BYTE m_byDayPlanMinute;			/*��ʼִ�е�������*/
	BYTE m_byDayPlanControl;       /*      0 �C ��������
      									1 �C ����
 										2 �C ȫ��
 										3 �C �ص�
										4 �C ����
										5  - �����ڿ���		
										6 �C �����Ӧ����
										7 �C Э����Ӧ���� 		
										8 �C ����ѡ�����
										9 �C ����Ӧ����
										10 �C �޵��¿���
										11 �C �е��¿��ƣ���λ�����ƣ�
										12  - ���˹���*/
	BYTE m_byDayPlanActionNumberOID;      /*��ʱ������*/
	char m_cDayPlanDesc[MAX_DESC_LENGTH]; /*����*/
	BYTE m_byCoorDination;				//Э���ƻ���־ 0������Э���ƻ� 1����Э���ƻ�
}TTimeBaseDayPlan, *PTTimeBaseDayPlan;


//�������������
typedef struct tagVehicleDetector
{
	BYTE m_byVehicleDetectorNumber;     /*2.3.2.1������������кš�(1..48)*/
	BYTE m_byVehicleDetectorOptions;	/*2.3.2.2���������ѡ��
			Bit 0 �C ���������
			Bit 1 �C ռ���ʼ����
			Bit 2 -  ������λ�ڻƵ�ʱ���¼������
			Bit 3 -  ������λ�ں��ʱ���¼������
			Bit 4 �C��Ӧ��λ���ӵ�λ�ӳ��̣�passage��
			Bit 5 �C�������ȼ����
			Bit 6 �C �ŶӼ����
			Bit 7 �C ��������*/
	BYTE m_byVehicleDetectorCallPhase;/*2.3.2.3�ü������Ӧ��������λ*/
	BYTE m_byVehicleDetectorSwitchPhase;/*2.3.2.4�Ǹ���λ��,����λ�ɽ��ոó�������������󣬵�assigned phase Ϊ��ƻ�Ƶ�ʱ����the Switch Phaseʱ�̵�ʱ������λ��ת��*/
	WORD m_wVehicleDetectorDelay;	/*2.3.2.5��assigned phase�����̵�ʱ������������뽫���ӳ�һ��ʱ�䣨00-999�룩��һ���������ӳ٣�����֮�������ӳٽ����ۼ�*/
	BYTE m_byVehicleDetectorExtend;/*2.3.2.6��assigned phase���̵�ʱ���������ÿ�����룬��λ�������̵Ƶ���ֹ�㱻�ӳ�һ��ʱ�䣨00-999�룩*/
	BYTE m_byVehicleDetectorQueueLimit;/*2.3.2.7������Ŷӳ������ƣ���������һ����ʱ������ĳ���������Ч*/
	BYTE m_byVehicleDetectorNoActivity;/*2.3.2.8 0-255���ӣ������ָ����ʱ���м����û�з�����Ӧ��Ϣ���ж�Ϊһ���������ֵΪ0���򲻻ᱻ�ж�*/
	BYTE m_byVehicleDetectorMaxPresence;/*2.3.2.9 0-255���ӣ������ʱ���ڣ����������������Ӧ��Ϣ�����ж�Ϊһ���������ֵΪ0���򲻻ᱻ�ж�*/
	BYTE m_byVehicleDetectorErraticCounts;	/*2.3.2.10ÿ����0-255�Σ��������������ĸ�Ӧ��Ϣ��Ƶ�ʳ������ֵ�����ж�Ϊһ���������ֵΪ0���򲻻ᱻ�ж�*/
	BYTE m_byVehicleDetectorFailTime;/*2.3.2.11 �����ʧ��ʱ�䣬��λ����*/
	BYTE m_byVehicleDetectorAlarms;	/*2.3.2.12 ������澯
					Bit 7: Other Fault �C ��������
					Bit 6: Reserved.
					Bit 5: Reserved.
					Bit 4: Configuration Fault �C ���õļ����û��ʹ�û��һ�������ڵ���λ��ϵ
					Bit 3: Communications Fault �C�����ͨ�Ŵ���
					Bit 2: Erratic Output Fault �C �������������(��������)
					Bit 1: Max Presence Fault �C �����һֱ�г�
					Bit 0��No Activity Fault - �����һֱû��*/
	BYTE m_byVehicleDetectorReportedAlarms; /*2.3.2.12���������
					Bit 7: Reserved.
					Bit 6: Reserved.
					Bit 5: Reserved.
					Bit 4: Excessive Change Fault - ������������ࡣ
					Bit 3: Shorted Loop Fault - ������ջ���
					Bit 2: Open Loop Fault �C ���������
					Bit 1: Watchdog Fault -  watchdog ����
					Bit 0: Other �C ��������*/
	BYTE m_byVehicleDetectorReset;	/*2.3.2.13 ���ö�������Ϊ����ʱ��������cu����������������cu������������֮��ö����Զ�����Ϊ0*/

	TVehDetCtrParam m_stVehDetCtrParam;  //�������Ʋ���
	BYTE m_byDetectorType;//���������
	WORD m_wSaturatedFlow;//�������������
	
}TVehicleDetector, *PTVehicleDetector;


//���˼��������
typedef struct tagPedestrianDetector
{
	BYTE m_byPedestrianDetectorNumber;		/*2.3.7.1 ���˼�����к�*/
	BYTE m_byDetectorType;					/*0��Ȧ,1�״�,2��Ƶ*/
	BYTE m_byPedestrianDetectorCallPhase;	/*2.3.7.2 ���˼������Ӧ��������λ*/
	BYTE m_byPedestrianDetectorNoActivity;	/*2.3.7.3 0-255���ӣ������ָ����ʱ�������˼����û�з�����Ӧ��Ϣ���ж�Ϊһ���������ֵΪ0���򲻻ᱻ�ж�*/
	BYTE m_byPedestrianDetectorMaxPresence;	/*2.3.7.4  0-255���ӣ������ʱ���ڣ����˼��������������Ӧ��Ϣ�����ж�Ϊһ���������ֵΪ0���򲻻ᱻ�ж�*/
	BYTE m_byPedestrianDetectorErraticCounts;/*2.3.7.5 ÿ����0-255�Σ�������˼���������ĸ�Ӧ��Ϣ��Ƶ�ʳ������ֵ�����ж�Ϊһ���������ֵΪ0���򲻻ᱻ�ж�*/
	//BYTE  byPedestrianDetectorAlarms;		/*2.3.7.6 ���˼����������Ϣ��
	//		Bit 7: Other Fault �C ��������
	//		Bit 6: Reserved.
	//		Bit 5: Reserved.
	//		Bit 4: Configuration Fault �C ���ô���
	//		Bit 3: Communications Fault �C ͨ�Ŵ���
	//		Bit 2: Erratic Output Fault �C ��������
	//		Bit 1: Max Presence Fault �C �����ֳ�
	//		Bit 0: No Activity Fault �C ����û��*/
	BYTE m_byDetectorRegion;	/*0Ĭ��,1�ȴ����� ,2��������*/
}TPedestrianDetector, *PTPedestrianDetector;


//��Ԫ��������
typedef struct tagGeneralParam
{
	BYTE m_byUnitStartUpFlash;					// 2.4.1 ����ʱ���������ʱ��(��)������ʱ����������ڵ���ָ�����֡�����ָ����������Щ������豸���塣�����ڼ䣬Ӳ���������źŵƼ����ǲ���ģ�����еĻ�����
	BYTE m_byUnitAutoPedestrianClear;	       // 2.4.2�����Զ���ղ�����1 = False/Disable 2=True/Enable����	������Ϊ1�����ֶ�������Чʱ���źŻ�����������Զ����ʱ�䣬�Է�ֹ�������ʱ�䱻Ԥ�����õ�ʱ����ֹ��
	WORD m_wUnitBackupTime;						// 2.4.3�źŻ����ߺ󵽽���ǰ��ʱ�䡣
	BYTE m_byUnitRedRevert;						// 2.4.4��С���ʱ�䡣�˲���Ϊ���е���λ�ṩ��С���ʱ�䣨�磺�����ֵ����һ����λ�ĺ��ʱ�䣬�������λ�ĺ��ʱ����������������棩���������Ϊ�Ƶ�֮�����һ���̵�֮ǰ�ṩ���ʱ���ṩһ����Сָʾ��
	BYTE m_byUnitControl;							// .10 ����Զ�̿���ʵ�弤���źŻ���ĳЩ����( 0 = False / Disabled, 1 = True / Enabled)��
	//Bit 7: �Ҷ�ʹ�ܡ�����Ϊ1ʱ����ʾ����ͨ���ҶȲ�����Ϊ��ʵ��������ܣ�timebaseAscAuxillaryFunction������������Ϊtrue��
	//Bit 6: ����Э�� - ����Ϊ1ʱ����ʾ��Ϊ����Э���ķ������
	//Bit 5��Walk Rest Modifier - ������Ϊ1ʱ�������ͻ��λû�з����������Ӧ��λͣ���ڷ���״̬
	//Bit 4��Call to Non-Actuated 2 - ������Ϊ1ʱ��ʹ��phaseOptions�ֶ�������Non-Actuated 1����λ�����ڷǸ�Ӧ״̬��
	//Bit3: Call to Non-Actuated 1 - ������Ϊ1ʱ��ʹ��phaseOptions�ֶ�������Non-Actuated 2����λ�����ڷǸ�Ӧ״̬��
	//Bit2:External Minimum Recall -������Ϊ1ʱ��ʹ������λ��������С����״̬
	//Bit 1��0: Reserved��
	BYTE m_byCoordOperationalMode; // 2.5.1Э���Ĳ�����ʽAutomatic��0�����Զ���ʽ������ΪЭ������Ӧ������ȿ���
	//ManualPattern��1~253���������ֶ��趨�ķ���
	//ManualFree��254������Э���Զ���Ӧ
	//ManualFlash(255)����Э���Զ�����
	BYTE m_byCoordCorrectionMode;// 2.5.2Э����ʽ
	//other(1)Э��������һ��û���ڱ������ж�����µ���λ��
	//dwell(2)Э��ͨ��פ��Э����λ�����仯�ﵽ��λ��
	//shortway(3)Э��ͨ��ĳ���������ڱ仯�������ٺ�����ʱ��ﵽ��λ���ƽ������
	//addOnly(4)Э��ͨ��ĳ���������ڱ仯��ϰ��������ʱ�����ﵽ��λ��
	BYTE m_byCoordMaximumMode;	// 2.5.3 Э�������ķ�ʽ
	//other(1)�����ڴ��������δ֪��ʽ
	//maximum1(2)��Max1��Ч��Э��
	//maximum2(3)��Max2��Ч��Э��
	//maxinhibit(4)��������Э��ģʽʱ����ֹ���������
	BYTE m_byCoordForceMode;		// 2.5.4 Patternǿ��ģʽ
	//other(1)���źŻ�ʹ���ڴ�û�ж����ģʽ
	//floating(2)��ÿ����λ�����ǿ�Ƶ��̵�ʱ�䣬�����õ���λʱ��ת����Э����λ
	//fixed(3)��ÿ����λ��ǿ�Ƶ����ڹ̶�λ�ã����õ���λʱ��ӵ�����������λ��

	BYTE m_byFlashFrequency;			//����Ƶ��(BYTE)			
	BYTE m_byThroughStreetTimeGap;		//���ι���ʱ��(BYTE)
	BYTE m_byFluxCollectCycle;			//�����ɼ�����(BYTE)
	BYTE m_bySecondTimeDiff;			//���ι�������Э������
	BYTE m_byStartAllRedTime;			//����ȫ��ʱ��
	BYTE m_byCollectCycleUnit;			//�����ɼ���λ���� / ���� ( 0/1)
	BYTE m_byUseStartOrder;			//������������
	BYTE m_byCommOutTime;				//ͨ�ų�ʱʱ��
	WORD m_wSpeedCoef;					//�ٶȼ�������
	//�Զ��岿��                         
	WORD m_wDelayTime;       //���˷����ӳ���Чʱ�䣬�յ�������ӳ����ֵ��Ȼ��ʼ���У���λΪ��
	WORD m_wWaitTime;		   //������ǿ�Ʒ���ʱ�䣬һֱû���������������ô��ʱ��ͻ�ǿ�Ʒ���һ�Σ���λΪ�֣�0��ʾû����Ͳ�����
	WORD m_wPassGap;			//��ť���δ�����Сʱ������ÿ�η���֮���ٴδ��������������̵ȴ�ʱ�䣬��λΪ��
	unsigned char m_acGatwayIp[7];     //Ԥ��

	BYTE m_byTransCycle;				//ƽ����������
	BYTE m_byOption;		    //ѡ���������λȡֵ
	//BIT 7---------��ѹ������
	//BIT 6---------����
	//BIT 5---------����
	//BIT 4---------����
	//BIT 3---------����
	//BIT 2---------��һ��������
	//BIT 1---------�����������
	//BIT 0---------���ð�����
	BYTE m_byUnitTransIntensityCalCo; //��ͨǿ�ȼ���ϵ��
}TGeneralParam, *PTGeneralParam;


//Э����������
typedef struct tagCoordinationVariable
{
	BYTE m_byCoordOperationalMode;		/*2.5.1Э���Ĳ�����ʽ
					Automatic��0�����Զ���ʽ������ΪЭ������Ӧ������ȿ���
					ManualPattern��1~253���������ֶ��趨�ķ���
					ManualFree��254������Э���Զ���Ӧ
					ManualFlash(255)����Э���Զ�����*/
	BYTE m_byCoordCorrectionMode;	/*2.5.2Э����ʽ
					other(1)Э��������һ��û���ڱ������ж�����µ���λ��
					dwell(2)Э��ͨ��פ��Э����λ�����仯�ﵽ��λ��
					shortway(3)Э��ͨ��ĳ���������ڱ仯�������ٺ�����ʱ��ﵽ��λ���ƽ������
					addOnly(4)Э��ͨ��ĳ���������ڱ仯��ϰ��������ʱ�����ﵽ��λ��*/
	BYTE m_byCoordMaximumMode;		/*2.5.3 Э�������ķ�ʽ
					other(1)�����ڴ��������δ֪��ʽ
					maximum1(2)��Max1��Ч��Э��
					maximum2(3)��Max2��Ч��Э��
					maxinhibit(4)��������Э��ģʽʱ����ֹ���������*/
	BYTE m_byCoordForceMode;			/*2.5.4 Patternǿ��ģʽ
					other(1)���źŻ�ʹ���ڴ�û�ж����ģʽ
					floating(2)��ÿ����λ�����ǿ�Ƶ��̵�ʱ�䣬�����õ���λʱ��ת����Э����λ
					fixed(3)��ÿ����λ��ǿ�Ƶ����ڹ̶�λ�ã����õ���λʱ��ӵ�����������λ��*/
	BYTE m_byPatternTableType;		/*2.5.6���巽��������Ҫ��������֯�ṹ
					other(1)���˴�û�ж����
					patterns(2)���������ÿһ�д���Ψһ��һ�����������Ҳ�����������
					offset3(3)��ÿ��������3����λ�ռ�������3��
					offset5(4)��ÿ��������5����λ�ռ5��*/
	BYTE m_byCoordPatternStatus;		/*2.5.10 Э������״̬
				Not used��0��
				Pattern -��1-253����ǰ���з�����
				Free - (254)��Ӧ
				Flash - (255)����*/
	BYTE m_byLocalFreeStatus;		/*2.5.11 Free����״̬
				other: ����״̬
				notFree: û�н���free����
				commandFree: 
				transitionFree: ����free��������Э��
				inputFree: �źŻ����뵼��free������ӦЭ��
				coordFree: the CU programming for the called pattern is to run Free.
				badPlan: ���õķ������Ϸ����Խ���free
				badCycleTime: ���ڲ��Ϸ�����������С�������Խ���free
				splitOverrun: ʱ��Խ��free
				invalidOffset: ��������
				failed: ������ϵ���free*/
	WORD m_wCoordCycleStatus;		/* 2.5.12 Э������������״̬��0-510sec���������ڳ�һֱ���ٵ�0*/
	WORD m_wCoordSyncStatus;			/*2.5.13Э��ͬ��״̬��0��510����Э����׼�㵽Ŀǰ���ڵ�ʱ�䣬��0��¼���¸����ڻ�׼�㡣���Գ������ڳ�*/
	BYTE m_bySystemPatternControl;	/*2.5.14ϵͳ��������
				Standby(0)ϵͳ��������
				Pattern(1-253)ϵͳ���Ʒ�����
				Free(254)call free 
				Flash(255)�Զ�Flash */
	BYTE m_bySystemSyncControl;		/*2.5.14 ����ϵͳͬ����׼��*/
}TCoordinationVariable, *PTCoordinationVariable;


//ͨ������
typedef struct tagChannel
{
	BYTE m_byChannelNumber;		/*2.9.2.1ͨ���ţ����ܴ���maxChannels��(1..32)*/
	BYTE m_byChannelControlSource;	/*ͨ������Դ[��λ(phase)�����ص�(overlap)]��
				��channelControlType��������λ�����ص������ܴ��������λ��������ص�����*/
	BYTE m_byChannelControlType;	/*2.9.2.2ͨ���������ͣ�
																			other(1), ����
																			phaseVehicle (2), ��������λ
																			phasePedestrian (3), ������λ
																			overlap (4)��������λ
																			overlapPedestrian(5)��������
																			Lane lamp(5), �ɱ䳵����*/

	BYTE m_byChannelFlash;			/*2.9.2.3�Զ�����״̬��
			Bit 7: Reserved
			Bit 6: Reserved
			Bit 5: Reserved
			Bit 4: Reserved
			Bit 3: ��������
				Bit=0: Off/Disabled & Bit=1: On/Enabled
			Bit 2: ����
				Bit=0: Off/Red Dark & Bit=1: On/Flash Red
			Bit 1: ����
				Bit=0: Off/Yellow Dark & Bit=1: On/Flash Yellow
			Bit 0: Reserved
				Bit 1 �� Bit 2 ͬʱΪ1��Ч����Bit 1 = 0��Bit 2 = 1��Reservedλ����Ϊ0�����򷵻�badValue(3)����*/
	BYTE m_byChannelDim;		/*2.9.2.4ͨ���Ҷ�״̬
			Bit 7: Reserved
			Bit 6: Reserved
			Bit 5: Reserved
			Bit 4: Reserved
			Bit 3: Dim Alternate Half Line Cycle
				Bit=0: Off/+ half cycle &
				Bit=1: On/- half cycle
			Bit 2: Dim Red
				Bit=0: Off/Red Not Dimmed &
				Bit=1: On/Dimmed Red
			Bit 1: Dim Yellow
				Bit=0: Off / Yellow Not Dimmed &
				Bit=1: On / Dimmed Yellow
			Bit 0: Dim Green
				Bit=0: Off / Green Not Dimmed &
				Bit=1: On / Dimmed Green*/

	TLampBoardCtrParam m_stLampBoardCtrParam;   //�ƿذ���Ʋ���
}TChannel, *PTChannel;


//�������ö���
typedef struct tagPreempt
{
	BYTE m_byPreemptID;        //���ȼ���
	BYTE m_byPreemptLevel;     //���ȼ� 1-5���ȼ�Խ�ߣ����ȼ�Խ��
	BYTE m_byPreemptType;      /*�������� 0���������ȣ��յ���������ʱ�����������λ�ǵ�ǰ��λ��������λ������ִ����С��
										  1���������ȣ��յ���������ʱ�����������λ�ǵ�ǰ��λ��ֱ������������λ*/
	WORD m_wPreemptDelay;      //�ӳ�ʱ��-����Ӽ����λ�õ�·����Ҫ��ʻ��������ʱ��
	WORD m_wPreemptDuration;   //����ʱ��-�������ȳ���ͨ��·����������ʱ��
	BYTE m_byPreemptPhaseID;   //������λ
	BYTE m_byPreemptOmit;      //���� 0������ 1������
	BYTE m_byPreemptSource;	   //������ź�Դ

	//BYTE m_byPreemptNumber;
	//BYTE m_byPreemptControl;
	//BYTE m_byPreemptLink;
	//WORD m_wPreemptDelay;
	//WORD m_wPreemptMinimumDuration;
	//BYTE m_byPreemptMinimumGreen;
	//BYTE m_byPreemptMinimumWalk;
	//BYTE m_byPreemptEnterPedClear;
	//BYTE m_byPreemptTrackGreen;
	//BYTE m_byPreemptDwellGreen;
	//WORD m_wPreemptMaximumPresence;
	//BYTE m_abyPreemptTrackPhase[MAX_PHASE_COUNT];
	//BYTE m_byPreemptTrackPhaseLen;
	//BYTE m_abyPreemptDwellPhase[MAX_PHASE_COUNT];
	//BYTE m_byPreemptDwellPhaseLen;
	//BYTE m_abyPreemptDwellPed[MAX_PEDESTRIANPHASE_COUNT];
	//BYTE m_byPreemptDwellPedLen;
	//BYTE m_abyPreemptExitPhase[MAX_PHASE_COUNT];
	//BYTE m_byPreemptExitPhaseLen;
	//BYTE m_byPreemptState;
	//BYTE m_abyPreemptTrackOverlap[MAX_OVERLAP_COUNT];
	//BYTE m_byPreemptTrackOverlapLen;
	//BYTE m_abyPreemptDwellOverlap[MAX_OVERLAP_COUNT];
	//BYTE m_byPreemptDwellOverlapLen;
	//BYTE m_abyPreemptCyclingPhase[MAX_PHASE_COUNT];
	//BYTE m_byPreemptCyclingPhaseLen;
	//BYTE m_abyPreemptCyclingPed[MAX_PEDESTRIANPHASE_COUNT];
	//BYTE m_byPreemptCyclingPedLen;
	//BYTE m_abyPreemptCyclingOverlap[MAX_OVERLAP_COUNT];
	//BYTE m_byPreemptCyclingOverlapLen;
	//BYTE m_byPreemptEnterYellowChange;
	//BYTE m_byPreemptEnterRedClear;
	//BYTE m_byPreemptTrackYellowChange;
	//BYTE m_byPreemptTrackRedClear;
}TPreempt, *PTPreempt;


typedef struct tagAddress
{
    WORD m_wSerialAddr;        //���ڵ�ַ
    BYTE m_abyIpAddr[4];       //IP��ַ
    DWORD m_dwSubNetMask;        //��������
    BYTE m_abyGatwayIp[4];     //����
    BYTE m_byPort;            //�˿ں�
    BYTE m_abyucMacAddr[6];    //MAC��ַ
    BYTE m_byreserved1;	    //����Ϊ�����ֶ�
    BYTE m_byreserved2;
    BYTE m_byreserved3;
    BYTE m_byreserved4;
    BYTE m_byreserved5;
    BYTE m_byreserved6;
    BYTE m_byreserved7;
    BYTE m_byreserved8;
}TAddress, *PTAddress;


typedef struct tagUnitParamEx
{
	BYTE m_byReserved1;		//��ͨǿ�ȼ���ϵ��
	BYTE m_byReserved2;
	BYTE m_byReserved3;
	BYTE m_byReserved4;
	BYTE m_byReserved5;
	BYTE m_byReserved6;
	BYTE m_byReserved7;
	BYTE m_byReserved8;
	BYTE m_byReserved9;
	BYTE m_byReserved10;
	BYTE m_byReserved11;
	BYTE m_byReserved12;
	BYTE m_byReserved13;
	BYTE m_byReserved14;
}TUnitParamEX, *PTUnitParamEX;


typedef struct tagExtraParamCountDown
{
	int m_iCountDownMode;					// ����ʱģʽ
										//	0:��ѧϰ(ȱʡ)
										//	1:����ȫ�̵���ʱģʽ
										//	2:�����̵���ʱģʽ
										//	485Э��
										//	3: ���ұ�׼
										//      4:��˹��׼
										//      5:���ű�׼
	int m_iFreeGreenTime;                     //��Ӧ���ʱ�䣬ȱʡֵΪ3
	int m_iPulseGreenTime;				//�����̵Ƶ���ʱʱ��
	int m_iPulseRedTime;				       //�����Ƶ���ʱʱ��
	TPhase m_iPhaseOfChannel[32];                    //ͨ����Ӧ����λ(Ŀǰ���֧��32��ͨ��)
	int m_iOption;								//bit[0]:����Ƶ�ʱ�Ƿ���˸
	int m_iRedFlashTime;						//��Ƶ���ʱ��˸ʱ��
}TExtraParamCountDown, *PTExtraParamCountDown;

//����ʱ�����
typedef struct tagAscStartSequenceInfo
{
	BYTE m_byStartYellowFlash;   //��������
	BYTE m_byStartAllRed;        //����ȫ��
	BYTE m_byGreenWaveCycle;     //�����̲���������
}TAscStartSequenceInfo, *PTAscStartSequenceInfo;

//ͨ���ɼ��
typedef struct tagAscChannelVerifyInfo
{
	int m_byControl;											//���Ʒ�ʽ
	char m_achLampClr[C_N_MAX_LAMP_OUTPUT_NUM];                  //�ƿذ���������״̬��Ϣ����
}TAscChannelVerifyInfo, *PTAscChannelVerifyInfo;

//��ȡU����������ʱƽָ̨��
typedef struct tagAscGainTrafficFlowCmd
{
	int m_SetUDiskStatus;                
	int m_GainTrafficFlowStatus;
}TAscGainTafficFlowCmd, *PTAscGainTafficFlowCmd;

//������Ԥ��Ϣ
typedef struct tagInterruptPatternInfo
{
	int m_nDurationTime;					    //����ʱ��
	int m_nDelayTime;						    //�ӳ�ʱ��
	int m_nOffset;
	int m_nCycleTime;
	int m_nRingNum;
	TSplit InterruptSplit[MAX_PHASE_COUNT];     //��Ԥ�����ű���Ϣ
	TSequence InterruptSequence[MAX_RING_COUNT]; //��Ԥ��ʱ����Ϣ
}TInterruptPatternInfo, *PTInterruptPatternInfo;

//�Զ����Ӧ����Ĺ�����ʽ
typedef struct tagWorkModeParam
{
	BYTE m_byControlMode;
	BYTE m_byControlType;
	WORD m_wControlNumber;
	WORD m_wControlValue;
	int  m_nDelay;                                                      //�ӳ�ʱ�䣬��λ��
	int  m_nDuration;                                                   //����ʱ�䣬��λ��
	TInterruptPatternInfo    m_atManualControlPattern;                  //�ֶ����Ʒ���             
}TWorkModeParam, *PTWorkModeParam;

typedef struct tagChannelStatusInfo
{
	BYTE m_byChannelID;
	float m_fRedResVolt;				//��ͨ����Ʋ�����ѹ
	float m_fRedOutputVolt;				//��ͨ����������ѹ
	float m_fRedOffResPower;			//��ͨ����ƹرպ��������
	float m_fRedOnOutputPower;			//��ͨ����������������

	float m_fYellowResVolt;				//��ͨ����Ʋ�����ѹ
	float m_fYellowOutputVolt;			//��ͨ����������ѹ
	float m_fYellowOffResPower;		    //��ͨ����ƹرպ��������
	float m_fYellowOnOutputPower;		//��ͨ����������������

	float m_fGreenResVolt;				//��ͨ����Ʋ�����ѹ
	float m_fGreenOutputVolt;			//��ͨ����������ѹ
	float m_fGreenOffResPower;			//��ͨ����ƹرպ��������
	float m_fGreenOnOutputPower;		//��ͨ����������������

	int   m_nInVolt;					//ͨ�������ѹ

}TChannelStatusInfo, *PTChannelStatusInfo;

//ͨ���̳�ͻ���ñ�
typedef struct tagAscChannelGreenConflictInfo
{
	BYTE m_byChannelNum;										//ͨ����
	BYTE m_byGreenConflict[MAX_CHANNEL_COUNT];					//ͨ�����̳�ͻ
}TAscChannelGreenConflictInfo, * PTAscChannelGreenConflictInfo;

typedef struct tagAscParam
{
	TAscStepCfg m_stAscStepCfg;

	TAscAddressCode m_stAscAddressCode;															//�źŻ�ʶ����

	TAscNetCard m_stAscNetCardTable[MAX_NETCARD_TABLE_COUNT];									//�źŻ���������������Ϣ��0������1. 1����2

	TAscTimeZoneInfo m_stAscTimeZoneInfo;														//�źŻ�ʱ��������Ϣ����ʱ������ʱ����Ϣ��
	
	TAscCenter m_stCenterVariable;																//ƽ̨����ip�Ͷ˿�����

	TAscManualPanel m_stAscManualPanel;															//�ֶ��������

	TAscArea m_stAscArea;																		//·�ں������

	TAscFaultCfg m_stAscFaultCfg;

	int m_stSingleOptimTableValidSize;
	TAscSingleOptim m_stAscSingleOptimTable[MAX_PATTERN_COUNT];									//��������Ӧ���Ʋ���
	
	int m_stPhaseTableValidSize;																//��λ����Ч����
	TPhase m_stAscPhaseTable[MAX_PHASE_COUNT];													//��λ��	

	int m_stVehicleDetectorTableValidSize;														//�������������Ч����
	TVehicleDetector m_stAscVehicleDetectorTable[MAX_VEHICLEDETECTOR_COUNT];					//�����������

	int m_stPedestrianDetectorTableValidSize;													//���˼������Ч����
	TPedestrianDetector m_stAscPedestrianDetectorTable[MAX_PEDESTRIANDETECTOR_COUNT];

	TGeneralParam m_stAscUnitVariable;															//�źŻ���Ԫ������	

	TCoordinationVariable m_stAscCoordinationVariable;											//Э������

	int m_stPatternTableValidSize;
	TPattern m_stAscPatternTable[MAX_PATTERN_COUNT];											//������

	int m_stSplitTableValidHeight;																//���űȱ���Ч�߶�
	int m_stSplitTableValidWidth[MAX_SPLIT_COUNT];												//���űȱ�ÿ����Ч���
	TSplit m_stAscSplitTable[MAX_SPLIT_COUNT][MAX_PHASE_COUNT];									//���űȱ�	

	TTimeBaseVariable m_stAscTimeBaseVariable;													//ʱ������
	
	int m_stTimeActionTableValidSize;
	TTimeBaseAscAction m_stAscTimeBaseActionTable[MAX_TIMEBASE_ACTION_COUNT];					//������

	int m_stScheduleTableValidSize;
	TTimeBaseSchedule m_stAscScheduleTable[MAX_SCHEDULE_COUNT];									//���ȼƻ���

	int m_stDayPlanTableValidHeight;															//��ά����Ч�߶�
	int m_stDayPlanTableValidWidth[MAX_DAYPLAN_TABLE_COUNT];									//ʱ�α�ÿ����Ч���ݳ���
	TTimeBaseDayPlan m_stAscDayPlanTable[MAX_DAYPLAN_TABLE_COUNT][MAX_SINGLE_DAYPLAN_COUNT];	//ʱ�α�

	int m_stSequenceTableValidHeight;															//ʱ�����Ч�߶�
	int m_stSequenceTableValidWidth[MAX_SEQUENCE_TABLE_COUNT];
	TSequence m_stAscSequenceTable[MAX_SEQUENCE_TABLE_COUNT][MAX_RING_COUNT];					//ʱ���	

	int m_stChannelTableValidSize;
	TChannel m_stAscChannelTable[MAX_CHANNEL_COUNT];											//ͨ����	

	int m_stOverlapTableValidSize;
	TOverlapTable m_stAscOverlapTable[MAX_OVERLAP_COUNT];										//������λ��
	
	int m_stPreemptTableValidSize;
	TPreempt m_stAscPreemptTable[MAX_PREEMPT_COUNT];											//���ȱ�

	TUnitParamEX m_stAscUnitParamEx;															//���ӵĵ�Ԫ��������

	TExtraParamCountDown m_stAscCountDownParam;													//����ʱ��

	TAscCasCadeInfo m_stAscCasCadeInfo;															//��������

	int m_stChannelLockTableValidSize;
	TAscOnePlanChannelLockInfo m_stAscChannelLockInfo[MAX_SINGLE_DAYPLAN_COUNT];				// ͨ��������Ϣ

	TAscStartSequenceInfo m_stAscStartSequenceInfo;												//����ʱ����Ϣ

	int m_stStagesListTableValidHeight;															
	int m_stStagesListTableValidWidth[MAX_PATTERN_COUNT];
	TStagesList m_stAscStagesList[MAX_PATTERN_COUNT][MAX_STAGE_COUNT];                          //�׶�������Ϣ

	int m_stStagesTableValidHeight;															
	int m_stStagesTableValidWidth[MAX_PATTERN_COUNT];
	TStages m_stAscStages[MAX_PATTERN_COUNT][MAX_STAGE_COUNT];                                  //�׶α���Ϣ

	int m_stBarrierTableValidHeight;															
	int m_stBarrierTableValidWidth[MAX_PATTERN_COUNT];
	TBarrier m_stAscBarrier[MAX_PATTERN_COUNT][MAX_SEQUENCE_TABLE_COUNT];                       //������Ϣ

	int m_stChannelConflictValidSize;
	TAscChannelGreenConflictInfo m_stAscChannelConflictTable[MAX_CHANNEL_COUNT];				//ͨ���̳�ͻ��

	char m_stChannelGreenConflictInfo[MAX_CHANNEL_COUNT][MAX_CHANNEL_COUNT];						//ͨ���̳�ͻ��Ϣ��,0��ʾ����ͬʱ���̵�,1��ʾ����ͬʱ���̵�
}TAscParam, *PTAscParam;


typedef struct tagPhaseInfo
{
	int nPhaseID;											//��λid
	int nPhaseSplitTime;									//��λ��Ӧ�����ű�ֵ
	int nPhaseSplitMode;                  					//���ű�ģʽ
}TPhaseInfo, * PTPhaseInfo;

typedef struct tagOneStageInfo
{
	int  nConcurrencyPhaseCount;
	int  nConcurrencyPhase[MAX_RING_COUNT];
}TOneStageInfo, * PTOneStageInfo;

typedef struct tagPatternStageInfo
{
	int  nStageCount;
	TOneStageInfo  atPhaseRunStageInfo[MAX_STAGE_COUNT];
}TPatternStageInfo, * PTPatternStageInfo;

typedef struct tagRingInfo
{
	int nPhaseCount;										//������λ����
	TPhaseInfo atPhaseInfo[MAX_PHASE_COUNT];				//������λ��Ϣ��
}TRingInfo, * PTRingInfo;

//����������Ϣ
typedef struct tagPatternInfo
{
	int nRingCount;											//һ�������л�����
	TRingInfo atRingInfo[MAX_RING_COUNT];					//���������л��Ļ�����λ��Ϣ
}TPatternInfo, * PTPatternInfo;

typedef struct tagErrInfo
{
	int nCheckErr;
	int nSubCheckErr;
	int nSubTwoCheckErr;
}TErrInfo, * PTErrInfo;
//#pragma pack(pop)//�ָ�����״̬
#endif 
