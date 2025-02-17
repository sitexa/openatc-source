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
#ifndef OPENATCNEWPARAMSTRUCTDEFINE_H
#define OPENATCNEWPARAMSTRUCTDEFINE_H

#include "../Include/OpenATCParamConstDefine.h"

#define MAX_PHASE_COUNT_20999    64
#define MAX_DAY_PLAN_SIZE 128
#define MAX_VEC_DET_SIZE 128

#pragma warning (disable:4819)
//#pragma pack(push)//�������״̬
//#pragma pack(1)  //����Ϊ1�ֽڶ���

//�źŻ�IPV4����������
typedef struct tagAscIPV4NetCard
{
	BYTE m_chAscIPV4IP[4];			//�źŻ�IPV4��IP��ַ
	BYTE m_chAscIPV4SubNet[4];		//�źŻ�IPV4����������
	BYTE m_chAscIPV4GateWay[4];		//�źŻ�IPV4������
}TAscIPV4NetCard, * PTAscIPV4NetCard;

//��λ����IPV4����������
typedef struct tagCerterIPV4NetCard
{
	BYTE m_chCerterIPV4IP[4];		//��λ��IPV4��IP��ַ
	WORD m_chCerterIPV4Port;		//��λ��IPV4ͨѶ�˿�
	BYTE m_chCerterIPV4Type;		//��λ��IPV4ͨѶ���� 1.TCP 2.UDP 3.RS232
}TCerterIPV4NetCard, * PTCerterIPV4NetCard;

//�źŻ�IPV6����������
typedef struct tagAscIPV6NetCard
{
	BYTE m_chAscIPV6IP[16];			//�źŻ�IPV4��IP��ַ
	BYTE m_chAscIPV6SubNet[16];		//�źŻ�IPV4����������
	BYTE m_chAscIPV6GateWay[16];		//�źŻ�IPV4������
}TAscIPV6NetCard, * PTAscIPV6NetCard;

//��λ����IPV6����������
typedef struct tagCerterIPV6NetCard
{
	BYTE m_chCerterIPV6IP[16];		//��λ��IPV6��IP��ַ
	WORD m_chCerterIPV6Port;		//��λ��IPV6ͨѶ�˿�
	BYTE m_chCerterIPV6Type;		//��λ��IPV6ͨѶ���� 1.TCP 2.UDP 3.RS232
}TCerterIPV6NetCard, * PTCerterIPV6NetCard;

//�豸��Ϣ
typedef struct tagDeviceInfo
{
	char m_chManufacturer[128];		//���쳧��
	BYTE m_byDeviceVersion[4];		//�豸�汾
	char m_chDeviceNum[16];			//�豸���
	BYTE m_byDateOfProduction[7];	//��������
	BYTE m_byDateOfConfig[7];		//��������
}TDeviceInfo, * PTDeviceInfo;

//������Ϣ
typedef struct tagBaseInfo
{
	BYTE m_bySiteRoadID[128];			//�źŻ���װ·��
	TAscIPV4NetCard m_stAscIPV4;		//�źŻ�IPV4��������
	TCerterIPV4NetCard m_stCerterIPV4;	//��λ��IPV4��������
	long m_wTimeZone;					//�źŻ�����ʱ��
	DWORD m_wATCCode;					//�źŻ����
	BYTE m_byCrossRoadNum;				//�źŻ�����·������
	bool m_bGpsClockFlag;				//GPSʱ�ӱ�־
	TAscIPV6NetCard m_stAscIPV6;		//�źŻ�IPV6��������
	TCerterIPV6NetCard m_stCerterIPV6;	//��λ��IPV6��������
}TBaseInfo, *PTBaseInfo;

//���鶨��
typedef struct tag20999Channel
{
	BYTE m_byChannelNumber;		/*2.9.2.1ͨ���ţ����ܴ���maxChannels��(1..32)*/
	BYTE m_byLightControlType;	//1 ������ 2 �ǻ����� 3 ���� 4 ���� 5 �ɱ佻ͨ��־ 6 ����ר�õƾ� 7 �й�糵ר�õƾ� 8 ���⹫��
	BYTE m_byForbiddenFlag;		//ͨ����ֹ��־
	BYTE m_byScreenFlag;		//ͨ�����α�־
	BYTE m_byRoutineFlag;		//ͨ������ʹ�ñ�־��0��ʾ��������ʹ�ã�1��ʾ��������ʹ��
}T20999Channel, * PT20999Channel;

//��λ����
typedef struct tag20999Phase
{
	BYTE m_byPhaseNumber;			/*2.2.2.1 ��λ�ţ����ܳ���maxPhases�������ֵ��(1..32)*/
	WORD m_wPhaseMinimumGreen;		/* 2.2.2.4��С�̵�ʱ�䣬�����С�̡�*/
	WORD m_wPhaseMaximum1;			/* 2.2.2.6����̵�ʱ��1����������1��(0..255)*/
	WORD m_wPhaseMaximum2;			/* 2.2.2.7����̵�ʱ��2����������2��(0..255)*/
	//20999
	BYTE m_byLightGroup[8];         //��λ�ĵ���
	BYTE m_byPhaseCall[8];          //��λ����
	//BYTE m_byConflictPhase[8];      //��ͻ��λ����
	BYTE m_byForbiddenFlag;         //��λ��ֹ��־
	BYTE m_byScreenFlag;			//��λ���α�־
	BYTE m_nLoseControlLampOneType;
	BYTE m_nLoseControlLampOneTime;
	BYTE m_nLoseControlLampTwoType;
	BYTE m_nLoseControlLampTwoTime;
	BYTE m_nLoseControlLampThreeType;
	BYTE m_nLoseControlLampThreeTime;
	BYTE m_nGetControlLampOneType;
	BYTE m_nGetControlLampOneTime;
	BYTE m_nGetControlLampTwoType;
	BYTE m_nGetControlLampTwoTime;
	BYTE m_nGetControlLampThreeType;
	BYTE m_nGetControlLampThreeTime;
	BYTE m_nPowerOnGetControlLampOneType;
	BYTE m_nPowerOnGetControlLampOneTime;
	BYTE m_nPowerOnGetControlLampTwoType;
	BYTE m_nPowerOnGetControlLampTwoTime;
	BYTE m_nPowerOnGetControlLampThreeType;
	BYTE m_nPowerOnGetControlLampThreeTime;
	BYTE m_nPowerOnLossControlLampOneType;
	BYTE m_nPowerOnLossControlLampOneTime;
	BYTE m_nPowerOnLossControlLampTwoType;
	BYTE m_nPowerOnLossControlLampTwoTime;
	BYTE m_nPowerOnLossControlLampThreeType;
	BYTE m_nPowerOnLossControlLampThreeTime;
	BYTE m_byPhaseExtend;				//��λ�ӳ���ʱ�䣬20999��λΪ0.1��
}T20999Phase, * PT20999Phase;

//�������������
typedef struct tag20999VehicleDetector
{
	BYTE m_byVehicleDetectorNumber;     /*2.3.2.1������������кš�(1..48)*/
	BYTE m_byDetectorType;		//20999����
	WORD m_wFlowGatherCycle;//�����ɼ�����
	WORD m_wOccupancyGatherCycle;//ռ���ʲɼ�����
	char m_chFixPosition[MAX_MODULE_STRING_LENGTH];//��װλ��
}T20999VehicleDetector, * PT20999VehicleDetector;

//20999 ��λ�׶�
typedef struct tag20999PhaseStage
{
	BYTE m_byPhaseStageNumber;						//��λ�׶α��
	BYTE m_byPhase[8];								//��λ�׶ε���λ
	BYTE m_byLaterStartTime[MAX_PHASE_COUNT_20999]; //�׶�����λ������ʱ��
	BYTE m_byEarlyEndTime[MAX_PHASE_COUNT_20999];   //�׶�����λ�����ʱ��

	bool m_bSoftCall;                              //�������
	bool m_bScreen;                                //��λ�׶�����                        
	bool m_bForbidden;                             //��λ�׶ν�ֹ
}T20999PhaseStage, * PT20999PhaseStage;

//��λ��ȫ��Ϣ20999  ��λ��ͻ��
typedef struct tagPhaseConflictInfo
{
	BYTE m_byPhaseNumber;
	BYTE m_byConflictSequenceInfo[8];	//��ͻ����
}TPhaseConflictInfo, * PTPhaseConflictInfo;

//��λ�̼�����ñ�
typedef struct tagPhaseGreenGap
{
	BYTE m_byPhaseNumber;
	BYTE m_byGreenGapSequenceInfo[64];	//�̼������
}TPhaseGreenGapInfo, * PTPhaseGreenGapInfo;

//������Ϣ20999
typedef struct tagPriorityInfo
{
	BYTE m_byPrioritySignalNumber;		//�����źű��
	BYTE m_byPrioritySignalPhaseStage;	//�����ź�������λ�׶�
	BYTE m_byPrioritySignalGrade;		//�����ź��������ȼ�
	BYTE m_blPrioritySignalScreen;		//�����ź����α�־
	BYTE m_byPrioritySignalSource;      //�����ź���Դ(��������)
}TPriorityInfo, * PTPriorityInfo;

//������Ϣ20999
typedef struct tagEmergyInfo
{
	BYTE m_byEmergySignalID;			//�����źű��	
	BYTE m_byEmergySignalPhaseStage;	//�����ź�������λ�׶�
	BYTE m_byEmergySignalGrade;			//�����ź��������ȼ�
	BYTE m_bEmergySignalScreen;			//�����ź����α�־
	BYTE m_byEmergySignalSource;        //�����ź���Դ(��������)
}TEmergyInfo, * PTEmergyInfo;

//��������
typedef struct tag20999Pattern
{
	BYTE m_byPatternNumber;		/*2.5.7.1 �������и��������ĸ�����*/
	BYTE m_byRoadID;				   //��������·�����
	BYTE m_byCoorditionStage;		   //������Э�����
	DWORD m_wPatternCycleTime;		   //����������
	WORD m_byPatternOffsetTime;		   //������λ��ʱ��
	BYTE m_byPatternStage[16];		   //������λ�׶���
	BYTE m_byPatternStageTime[32];	   //������λ�׶���ʱ��
	BYTE m_byPatternStageOccurType[16];//������λ�׶γ�������
	//BYTE m_byForbiddenStage[16];       //�����б���ֹ����λ�׶�
	//BYTE m_byScreenStage[16];          //�����б����ε���λ�׶�
}T20999Pattern, * PT20999Pattern;

//����Լ��20999
typedef struct tagTransitBound
{
	BYTE m_byPhaseStageNumber;
	BYTE m_byTransitBound[MAX_PHASE_COUNT_20999];
}TTransitBound, * PTTransitBound;

//�ռƻ�20999
typedef struct tagDayPlanInfo
{
	BYTE m_byDayPlanID;					//�ռƻ����
	BYTE m_byRoadID;					//�ռƻ�����·�ڱ��
	BYTE m_byDayPlanStartTime[96];		//ʱ�ο�ʼʱ����
	BYTE m_byDayPlanActionPattern[48];	//ʱ�ο�ʼ������
	BYTE m_byDayPlanRunMode[48];		//ʱ������ģʽ��
	BYTE m_byActionChainOne[96];		//����1
	BYTE m_byActionChainTwo[96];		//����2
	BYTE m_byActionChainThree[96];		//����3
	BYTE m_byActionChainFour[96];		//����4
	BYTE m_byActionChainFive[96];		//����5
	BYTE m_byActionChainSix[96];		//����6
	BYTE m_byActionChainSeven[96];		//����7
	BYTE m_byActionChainEight[96];		//����8
}TDayPlanInfo, * PTDayPlanInfo;

//���ȼƻ�20999
typedef struct tagSchedulePlanInfo
{
	BYTE m_SchedulePlanID;				//���ȼƻ����
	BYTE m_byRoadID;					//���ȼƻ�����·��
	BYTE m_byPriority;					//���ȱ����ȼ�
	BYTE m_byWeek;						//����ֵ
	WORD m_byMonth;						//�·�ֵ
	DWORD m_byDate;						//����ֵ
	BYTE m_byScheduleOfDayPlanID;	    //�ռƻ����
}TSchedulePlanInfo, * PTSchedulePlanInfo;

//���Ŀ�����Ϣ20999
typedef struct tagCenterCtrlInfo
{
	BYTE m_byIntersectionID;//·�����
	BYTE m_byPhaseStage;	//ָ����λ�׶�
	BYTE m_byPattern;		//ָ������
	BYTE m_byRunMode;		//ָ������ģʽ
}TCenterCtrlInfo, * PTCenterCtrlInfo;

typedef struct tagAsc20999Param
{
	TDeviceInfo m_stDeviceInfo;											//�豸��Ϣ

	TBaseInfo m_stBaseInfo;												//�źŻ�������Ϣ

	int m_stChannelTableValidSize;										//��Ч��ͨ��������
	T20999Channel m_stAscChannelTable[MAX_PHASE_COUNT_20999];			//ͨ����(����)

	int m_stPhaseTableValidSize;										//��Ч����λ������
	T20999Phase m_stAscPhaseTable[MAX_PHASE_COUNT_20999];				//��λ��

	int m_stVehicleDetectorTableValidSize;								//��Ч�ĳ������������
	T20999VehicleDetector m_stAscVehicleDetectorTable[MAX_VEC_DET_SIZE];//�����������

	int m_stPhaseStageValidSize;										//��Ч����λ�׶α�����
	T20999PhaseStage m_stPhaseStage[MAX_PHASE_COUNT_20999];				//��λ�׶�

	int m_stPhaseConflictValidSize;
	TPhaseConflictInfo m_stPhaseConflictInfo[MAX_PHASE_COUNT_20999];	//��λ�̳�ͻ��Ϣ

	int m_stPhaseGreenGapValidSize;
	TPhaseGreenGapInfo m_stPhaseGreenGapInfo[MAX_PHASE_COUNT_20999];	//��λ�̼����Ϣ

	int m_stPriorityValidSize;
	TPriorityInfo m_stPriorityInfo[MAX_PHASE_COUNT_20999];				//������Ϣ

	int m_stEmergyValidSize;
	TEmergyInfo m_stEmergyInfo[MAX_PHASE_COUNT_20999];					//������Ϣ

	int m_stPatternTableValidSize;
	T20999Pattern m_stAscPatternTable[MAX_DAY_PLAN_SIZE];				//������

	int m_stTransitBoundValidSize;
	TTransitBound m_stTransitBound[MAX_PHASE_COUNT_20999];				//����Լ��

	int m_stDayPlanValidSize;
	TDayPlanInfo m_stDayPlanInfo[MAX_DAY_PLAN_SIZE];					//�ռƻ�

	int m_stSchedulePlanValidSize;
	TSchedulePlanInfo m_stSchedulePlanInfo[MAX_DAY_PLAN_SIZE];			//���ȼƻ�

	TCenterCtrlInfo m_stCenterCtrlInfo;									//���Ŀ�����Ϣ
}TAsc20999Param, * PTAsc20999Param;
#endif 
