/*=====================================================================
ģ���� ����GB20999Э���������������߳�ģ��
�ļ��� OpenATCCommWithGB20999Thread.h
����ļ���OpenATCDataPackUnpack.h OpenATCComDef.h
ʵ�ֹ��ܣ���GB20999Э���������Ľ���
���� ������Ƽ
��Ȩ ��<Copyright(c) 2019-2020 Suzhou Keda Technology Co.,Ltd. All right reserved.>
--------------------------------------------------------------------------------------------------------------------
�޸ļ�¼��
�� ��           �汾    �޸���             �߶���             �޸ļ�¼
2019/09/06      V1.0    ����Ƽ             ����Ƽ             ����ģ��
====================================================================*/

#ifndef OPENATCCOMMWITHGB20999THREAD_H
#define OPENATCCOMMWITHGB20999THREAD_H

#include "../../Include/OpenATCParameter.h"
#include "../../Include/OpenATCRunStatus.h"
#include "../../Include/CanBusManager.h"
#include "../logicctl/LogicCtlFixedTime.h"
#include "../../Include/OpenATC20999ParamStructDefine.h"
#include "../../Include/OpenATCOperationRecord.h"
#include "OpenATCComDef.h"
#include "OpenATCITS300DataPackUnpack.h"
#include "OpenATCSocket.h"
#include "OpenATCCfgCommHelper.h"

class COpenATCSocket;

#ifdef _WIN32
	#define COMMWITHGB20999_CALLBACK WINAPI
	typedef HANDLE               COMMWITHGB20999HANDLE;
#else
	#define COMMWITHGB20999_CALLBACK
	typedef pthread_t            COMMWITHGB20999HANDLE;
#endif

class COpenATCParameter;
class COpenATCRunStatus;

const short C_N_AGREEMENT_VERSION       = 0x0101;            //Э��汾

const BYTE  C_N_HOST_ID                 = 0x01;              //��λ��ID
const BYTE  C_N_ROAD_ID                 = 0;                 //·��ID

const int   C_N_FRAME_ID_POS            = 10;                //֡��ˮ��
const int   C_N_FRAME_TYPE_POS          = 11;                //֡����
const int   C_N_DATAVALUE_COUNT_POS     = 12;                //����ֵ����
const int   C_N_DATAVALUE_INDEX_POS     = 13;                //����ֵ����
const int   C_N_DATAVALUE_LENGTH_POS    = 14;                //����ֵ����
const int   C_N_DATACLASS_ID_POS        = 15;                //������ID
const int   C_N_OBJECT_ID_POS           = 16;                //����ID
const int   C_N_ATTRIBUTE_ID_POS        = 17;                //����ID
const int   C_N_ELEMENT_ID_POS          = 18;                //Ԫ��ID
const int   C_N_DATAVALUE_POS           = 19;                //����ֵ

const int   C_N_MAX_PLATE_LENGTH        = 16;                //�����Ƴ���
const int   C_N_MAX_PHASE_LIGHTGROUP    = 8;                 //��λ�ĵ��鳤��
const int   C_N_MAX_PHASE_DETECTOR      = 8;                 //��λ�ļ��������
//const int   C_N_MAX_PHASE_RUNSTAGE      = 8;                 //��λ�Ľ׶γ���
const int   C_N_MAX_PHASE_CONFLICT      = 8;                 //��λ�ĳ�ͻ���г���
const int   C_N_MAX_PHASE_GREENINTERVAL = 64;                //��λ���̼��ʱ�����г���
//const int   C_N_MAX_PATTERN_STAGE_CHAIN = 16;                //�����Ľ׶γ���
const int   C_N_MAX_DEVICE_MODULE       = 8;                 //�豸ģ�鳤��
const int   C_N_MAX_TRANSIT_RETAIN      = 64;                //��λ�Ľ׶ι���Լ��ֵ����
const int   C_N_MAX_LIGHTGROUP_COUNT    = 64;                //����������
const int   C_N_MAX_PHASE_COUNT         = 64;                //�����λ����
const int   C_N_MAX_DETECTOR_COUNT      = 128;               //�����������
const int   C_N_MAX_RUNSTAGE_COUNT      = 64;                //������н׶�����
const int   C_N_MAX_PRIORITY_COUNT      = 64;                //��������ź�����
const int   C_N_MAX_EMERGENCY_COUNT     = 64;                //�������ź�����
const int   C_N_MAX_PATTERN_COUNT       = 128;               //��󷽰�����
const int   C_N_MAX_DAYPLAN_COUNT       = 128;               //����ռƻ�����
const int   C_N_MAX_TIMECHAIN_COUNT     = 96;                //���ʱ��������
const int   C_N_MAX_PATTERNCHAIN_COUNT  = 48;                //��󷽰�������
const int   C_N_MAX_RUNMODECHAIN_COUNT  = 48;                //���ģʽ������
const int   C_N_MAX_ACTCHAIN_COUNT      = 96;                //�����������
const int   C_N_MAX_SCHEDULE_COUNT      = 128;               //�����ȱ�����

typedef struct tagDataConfig
{
	BYTE        m_byIndex;                                  //����
	BYTE        m_byDataLength;                             //����ֵ����
	BYTE        m_byDataClassID;                            //������ID    
	BYTE        m_byObjectID;                               //����ID
	BYTE        m_byAttributeID;                            //����ID
	BYTE        m_byElementID;                              //Ԫ��ID
	BYTE        m_byDataValueLength;                        //Ԫ��ֵ����
	BYTE        m_byDataValue[256];                         //Ԫ��ֵ
}TDataConfig,*PTDataConfig;

typedef struct tagReturnData
{
	BYTE        m_byReturnCount;                            //����ֵ����
	TDataConfig m_tDataConfig[256];                     
}TReturnData,*PTReturnData;

typedef struct tagDBManagement
{
	BYTE m_byDBCreateTransaction;		//normal(1)��transaction(2)��verify(3)��done(6)
	BYTE m_byDBVerifyStatus;			//notDone(1)��doneWithError(2)��doneWithNoError(3)
	BYTE m_byDBVerifyError;				//����֤�����ֵĴ�����ı����������ҽ���DBCreateTransaction������Done״̬��dbVerifyStatus������doneWithError״̬ʱ��Ч����������ֵ��Ч����������ʾ����
}TDBManagement, * PTDBManagement;

#ifdef _WIN32
#define GB20999_CALLBACK WINAPI
typedef HANDLE               GB20999HANDLE;
#else
#define GB20999_CALLBACK
typedef pthread_t            GB20999HANDLE;
#endif

#define NORMAL      1
#define TRANSACTION 2
#define VERIFYING    3
#define DONE        6

#define NOTDONE			1
#define DONEWITHERROR	2
#define DONEWITHNOERROR	3

/*=====================================================================
���� ��COpenATCCommWithIGB20999Thread
���� ����GB20999Э���������Ľ���
��Ҫ�ӿڣ�void Init����ʼ����������һ������yΪ���ò������ڶ�������Ϊ״̬
��ע ��
--------------------------------------------------------------------------------------------------------------------
�޸ļ�¼��
�� ��           �汾    �޸���             �߶���             �޸ļ�¼
2019/09/06      V1.0    ����Ƽ             ����Ƽ             ������
====================================================================*/
class COpenATCCommWithGB20999Thread
{
public:
    COpenATCCommWithGB20999Thread();
    virtual ~COpenATCCommWithGB20999Thread();

    virtual int Run();

	/****************************************************
	��������Init
    ���ܣ���ʼ������
	�㷨ʵ��:
    ����˵�� �� pParameter������
	            pRunStatus��״̬
    ����ֵ˵������
    --------------------------------------------------------------------------------------------------------------------
	�޸ļ�¼��
	�� ��           �汾    �޸���             �߶���             �޸ļ�¼
	2019/09/06      V1.0    ����Ƽ             ����Ƽ             ����
	====================================================================*/
	void        Init(COpenATCParameter *pParameter, COpenATCRunStatus *pRunStatus, COpenATCLog * pOpenATCLog);

    int         Start();
    int         Join();
    int         Detach();


private:
	enum
	{
		RECV_BUFFER_SIZE		= 1024,
		UNPACKED_BUFFER_SIZE	= 1024,  
		SEND_BUFFER_SIZE        = 1024,
		PACKED_BUFFER_SIZE      = 1024,

		HEART_INTERVAL_TIME     = 15,
		ALARM_INTERVAL_TIME     = 60,
		FAULT_INTERVAL_TIME     = 60,
	};
    static void *COMMWITHGB20999_CALLBACK RunThread(void *pParam);

	void  OpenATCSleep(long nMsec);

	unsigned short Crc16(const unsigned char *buffer, int buffer_length);

    void  ParserPack(char* chPeerIp);//�������ݰ�

    void  ParserPack_QueryLink();//������ѯ��

	void  ParserPack_SetLink(char* chPeerIp);//�������ð�

	int   SendAckToPeer(int nPackSize, int & nRet);//��������

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void  GetPatternParam(BYTE byPatternSplitNumber, BYTE byPatternSequenceNumber, TRunStageInfo & tRunStageInfo);//��ȡ��������

	void  GetRunStageTable(TFixTimeCtlInfo tFixTimeCtlInfo, TRunStageInfo & tRunStageInfo);//��ȡ�׶α� 

	BYTE  TransAlarmTypeToHost(BYTE byFaultValue, char cFaultInfo1, char cFaultInfo2, BYTE & byAlarmValue); //ת���������͵�ƽ̨

	BYTE  TransFaultTypeToHost(BYTE byFaultValue); //ת���������͵�ƽ̨

	BYTE  TransRunModeToHost(BYTE byModeValue, BYTE byControlSource); //ת������ģʽ��ƽ̨

	BYTE  TransRunModeToASC(BYTE byModeValue); //ת������ģʽ���źŻ�

	BYTE  TransStageStatus(BYTE byStageStatus);	//ת���׶�����״̬

	void  OpenATCGetCurTime(int & nYear, int & nMon, int & nDay, int & nHour, int & nMin, int & nSec, int & nWeek);//��ȡ��ǰϵͳʱ��

	bool  SetSysTime(const long nTime);

	char  GetBit(unsigned int nInput, char chNum); //��һ��ֵ��ȡ������λ

	void  RemoveDuplates(BYTE byData[], int & nCnt);//ɾ���������ظ�����

	void  SetTSequenceInfo(BYTE byPatternSequenceNumber, TRunStageInfo tRunStageInfo);//�����������

	void  SetTSplitInfo(BYTE byPatternSplitNumber, TRunStageInfo tRunStageInfo, bool bSetOrder, bool bSetSplitTime);//�������űȲ���

	bool  SetStartAndEndTime(BYTE byPatternSplitNumber, BYTE byRunStageIndex, TRunStageInfo tRunStageInfo, bool bStartTime, bool bEndTime, BYTE byDataValuePos);//����������ʱ��������ʱ��

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void  QueryLightGroupCount(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//��ȡ��������

	void  QueryAllElementLightGroupConfig(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//��ȡ����Ԫ�صĵ�������

	void  QueryAllElementLightGroupStatus(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//��ȡ����Ԫ�صĵ���״̬

	void  QueryAllElementLightGroupControl(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//��ȡ����Ԫ�صĵ������

	void  QueryLightGroupConfig(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//��ȡָ��Ԫ�صĵ�������

	void  QueryLightGroupStatus(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//��ȡָ��Ԫ�صĵ���״̬

	void  QueryLightGroupControl(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//��ȡָ��Ԫ�صĵ������

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void  QueryPhaseCount(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//��ȡ��λ����

	void  QueryAllElementPhaseConfig(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//��ȡ����Ԫ�ص���λ����

	void  QueryAllElementPhaseControl(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//��ȡ����Ԫ�ص���λ����

	void  QueryPhaseConfig(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//��ȡָ��Ԫ�ص���λ����

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void  QueryPhaseControl(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//��ȡָ��Ԫ�ص���λ����

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void  QueryDetectorCount(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//��ȡ���������

	void  QueryAllElementDetectorConfig(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//��ȡ����Ԫ�صļ��������

	void  QueryAllElementDetectorData(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//��ȡ����Ԫ�صļ��������

	void  QueryDetectorConfig(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//��ȡָ��Ԫ�صļ��������

	void  QueryDetectorData(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//��ȡָ��Ԫ�صļ��������

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void  QueryPhaseStageCount(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//��ȡ�׶�����

	void  QueryAllElementPhaseStageConfig(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//��ȡ����Ԫ�صĽ׶�����

	void  QueryAllElementPhaseStageStatus(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//��ȡ����Ԫ�صĽ׶�״̬

	void  QueryAllElementPhaseStageControl(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//��ȡ����Ԫ�صĽ׶ο���

	void  QueryPhaseStageConfig(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//��ȡָ��Ԫ�ص���λ�׶�����

	void  QueryPhaseStageStatus(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//��ȡָ��Ԫ�ص���λ�׶�״̬����

	void  QueryPhaseStageControl(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//��ȡָ��Ԫ�ص���λ�׶ο�������

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void  QueryAllElementPhaseeConflictInfo(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//��ȡ����Ԫ�ص���λ��ͻ��

	void  QueryAllElementPhasGreenInterval(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//��ȡ����Ԫ�ص��̼����

	void  QueryPhaseConflict(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//��ȡָ��Ԫ�ص���λ��ͻ

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void  QueryPhaseGreenInterval(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//��ȡָ��Ԫ�ص���λ�̼������

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void  QueryPriorityCount(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//��ȡ��������

	void  QueryEmergencyCount(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//��ȡ��������

	void  QueryAllElementPriorityConfig(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//��ȡ����Ԫ�ص���������

	void  QueryAllElementPriorityStatus(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//��ȡ����Ԫ�ص�����״̬

	void  QueryAllElementEmergencyConfig(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//��ȡ����Ԫ�صĽ�������

	void  QueryAllElementEmergencyStatus(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//��ȡ����Ԫ�صĽ���״̬

	void  QueryPriorityConfig(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//��ȡ��Ԫ�ص���������

	void  QueryPriorityStatus(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//��ȡ��Ԫ�ص�����״̬

	void  QueryEmergencyConfig(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//��ȡ��Ԫ�صĽ�������

	void  QueryEmergencyStatus(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//��ȡ��Ԫ�صĽ���״̬

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void  QueryPatternCount(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//��ȡ��������

	void  QueryAllElementPatternConfig(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//��ȡ����Ԫ�صķ���

	void  QueryPatternConfig(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//��ȡָ��Ԫ�صķ�������

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void  QueryAllElementTransitionRetainConfig(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//��ȡ����Ԫ�صĹ���Լ��

	void  QueryTransitionRetainConfig(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//��ȡָ��Ԫ�صĹ�������

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void  QueryDayPlanCount(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//��ȡ�ռƻ�����

	void  QueryAllElementDayPlanConfig(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//��ȡ����Ԫ�ص��ռƻ�

	void  QueryDayPlanConfig(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//��ȡָ��Ԫ�ص��ռƻ�����

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void  QueryScheduleTableCount(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//��ȡ��������

	void  QueryAllElementScheduleTableConfig(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//��ȡ����Ԫ�صĵ���

	void  QueryScheduleTableConfig(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//��ȡָ��Ԫ�ص��ռƻ�����

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void  QueryDeveiceStatus(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//��ȡ�豸״̬

	void  QueryControlStatus(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//��ȡ����״̬

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void  QueryAllElementStatisticData(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//��ѯ����Ԫ�ص�ͳ������

	void  QueryRealTimeData(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//��ȡʵʱ����

	void  QueryStatisticData(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//��ȡͳ������

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void  QueryAlarmDataCount(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//��ȡ��������

	void  QueryAllAlarmData(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//��ȡ���б�������

	void  QueryAllElementAlarmData(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//��ȡ����Ԫ�صı�������

	void  QueryAlarmData(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//��ȡָ��Ԫ�صı�������

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void  QueryFaultDataCount(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//��ȡ��������

	void  QueryAllFaultData(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//��ȡ���й�������

	void  QueryAllElementFaultData(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//��ȡ����Ԫ�صĹ�������

	void  QueryFaultData(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//��ȡָ��Ԫ�صĹ�������

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void  QueryCentreControlTable(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//��ȡ���Ŀ��Ʊ�

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void  QueryOrderPipeTable(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//��ȡ����ܵ�

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void  QueryOverlapCount(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//��ȡ���и�����λ�� --HPH 2021.12.07

	void  QueryAllElementOverlapConfig(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//��ȡ����Ԫ�صĸ�����λ --HPH 2021.12.07

	void  QueryOverlapConfig(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//��ȡ������λ --HPH 2021.12.07

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void  CreateSetReturnData(BYTE byRet, BYTE byIndex, BYTE byDataClassID, BYTE byObjectID, BYTE byAttributeID, BYTE byElementID, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);//�������ò������ؽṹ��

	void  SetDeviceInfo(BYTE byIndex, BYTE byObjectID, BYTE byAttributeID, BYTE byElementID, BYTE byDataValuePos, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData, BYTE byDataValueLength);//������������

	void  SetBaseInfo(BYTE byIndex, BYTE byObjectID, BYTE byAttributeID, BYTE byElementID, BYTE byDataValuePos, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData, BYTE byDataValueLength);//���û�����Ϣ

	void  SetLightGroupInfo(BYTE byIndex, BYTE byObjectID, BYTE byAttributeID, BYTE byElementID, BYTE byDataValuePos, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData, BYTE byDataValueLength);//���õ�����Ϣ

	void  SetLightGroupConfigInfo(BYTE byIndex, BYTE byObjectID, BYTE byAttributeID, BYTE byElementID, BYTE byDataValuePos, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData, BYTE byDataValueLength);//���õ���������Ϣ

	void  SetLightGroupControlInfo(BYTE byIndex, BYTE byObjectID, BYTE byAttributeID, BYTE byElementID, BYTE byDataValuePos, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData, BYTE byDataValueLength);//���õ��������Ϣ

	void  SetPhaseInfo(BYTE byIndex, BYTE byObjectID, BYTE byAttributeID, BYTE byElementID, BYTE byDataValuePos, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData, BYTE byDataValueLength);//������λ��Ϣ

	void  SetPhaseConfigInfo(BYTE byIndex, BYTE byObjectID, BYTE byAttributeID, BYTE byElementID, BYTE byDataValuePos, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData, BYTE byDataValueLength);//������λ������Ϣ

	void  SetPhaseControlInfo(BYTE byIndex, BYTE byObjectID, BYTE byAttributeID, BYTE byElementID, BYTE byDataValuePos, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData, BYTE byDataValueLength);//������λ������Ϣ

	void  SetDetectorInfo(BYTE byIndex, BYTE byObjectID, BYTE byAttributeID, BYTE byElementID, BYTE byDataValuePos, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData, BYTE byDataValueLength);//���ü������Ϣ

	void  SetDetectorConfigInfo(BYTE byIndex, BYTE byObjectID, BYTE byAttributeID, BYTE byElementID, BYTE byDataValuePos, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData, BYTE byDataValueLength);//���ü����������Ϣ

	void  SetPhaseStageInfo(BYTE byIndex, BYTE byObjectID, BYTE byAttributeID, BYTE byElementID, BYTE byDataValuePos, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData, BYTE byDataValueLength);//������λ�׶���Ϣ

	void  SetPhaseStageConfigInfo(BYTE byIndex, BYTE byObjectID, BYTE byAttributeID, BYTE byElementID, BYTE byDataValuePos, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData, BYTE byDataValueLength);//������λ�׶�������Ϣ

	void  SetPhaseStageControlInfo(BYTE byIndex, BYTE byObjectID, BYTE byAttributeID, BYTE byElementID, BYTE byDataValuePos, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData, BYTE byDataValueLength);//������λ�׶ο�����Ϣ

	void  SetPhaseSafetyInfo(BYTE byIndex, BYTE byObjectID, BYTE byAttributeID, BYTE byElementID, BYTE byDataValuePos, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData, BYTE byDataValueLength);//������λ��ȫ��Ϣ

	void  SetPhaseConflictInfo(BYTE byIndex, BYTE byObjectID, BYTE byAttributeID, BYTE byElementID, BYTE byDataValuePos, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData, BYTE byDataValueLength);//������λ�̳�ͻ������Ϣ

	void  SetPhaseGreenIntervalInfo(BYTE byIndex, BYTE byObjectID, BYTE byAttributeID, BYTE byElementID, BYTE byDataValuePos, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData, BYTE byDataValueLength);//������λ�̼��������Ϣ

	void  SetEmergencyAndPriorityInfo(BYTE byIndex, BYTE byObjectID, BYTE byAttributeID, BYTE byElementID, BYTE byDataValuePos, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData, BYTE byDataValueLength);//���ý���������Ϣ

	void  SetPriorityConfigInfo(BYTE byIndex, BYTE byObjectID, BYTE byAttributeID, BYTE byElementID, BYTE byDataValuePos, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData, BYTE byDataValueLength);//��������������Ϣ

	void  SetEmergencyConfigInfo(BYTE byIndex, BYTE byObjectID, BYTE byAttributeID, BYTE byElementID, BYTE byDataValuePos, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData, BYTE byDataValueLength);//���ý���������Ϣ

	void  SetPatternInfo(BYTE byIndex, BYTE byObjectID, BYTE byAttributeID, BYTE byElementID, BYTE byDataValuePos, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData, BYTE byDataValueLength);//���÷�����Ϣ

	void  SetPatternConfigInfo(BYTE byIndex, BYTE byObjectID, BYTE byAttributeID, BYTE byElementID, BYTE byDataValuePos, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData, BYTE byDataValueLength);//���÷���������Ϣ

	void  SetTransitionRetain(BYTE byIndex, BYTE byObjectID, BYTE byAttributeID, BYTE byElementID, BYTE byDataValuePos, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData, BYTE byDataValueLength);//���ù���Լ����Ϣ

	void  SetDayPlanInfo(BYTE byIndex, BYTE byObjectID, BYTE byAttributeID, BYTE byElementID, BYTE byDataValuePos, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData, BYTE byDataValueLength);//�����ռƻ���Ϣ

	void  SetDayPlanConfigInfo(BYTE byIndex, BYTE byObjectID, BYTE byAttributeID, BYTE byElementID, BYTE byDataValuePos, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData, BYTE byDataValueLength);//�����ռƻ�������Ϣ

	void  SetScheduleTable(BYTE byIndex, BYTE byObjectID, BYTE byAttributeID, BYTE byElementID, BYTE byDataValuePos, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData, BYTE byDataValueLength);//���õ��ȱ���Ϣ

	void  SetScheduleTableConfig(BYTE byIndex, BYTE byObjectID, BYTE byAttributeID, BYTE byElementID, BYTE byDataValuePos, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData, BYTE byDataValueLength);//���õ��ȱ�������Ϣ

	void  SetRunStatusInfo(BYTE byIndex, BYTE byObjectID, BYTE byAttributeID, BYTE byElementID, BYTE byDataValuePos, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData, BYTE byDataValueLength);//��������״̬��Ϣ

	void  SetATCStandardTime(BYTE byDataValuePos);//���ñ�׼ʱ��

	void  SetATCLocalTime(BYTE byDataValuePos);//���ñ���ʱ��

	void  SetCenterControl(BYTE byIndex, BYTE byObjectID, BYTE byAttributeID, BYTE byElementID, BYTE byDataValuePos, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData, BYTE byDataValueLength);//�������Ŀ�����Ϣ

	void  SetCenterControlConfig(BYTE byIndex, BYTE byObjectID, BYTE byAttributeID, BYTE byElementID, BYTE byDataValuePos, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData, BYTE byDataValueLength);//�������Ŀ�����Ϣ

	void  SetOrderPipe(BYTE byIndex, BYTE byObjectID, BYTE byAttributeID, BYTE byElementID, BYTE byDataValuePos, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData, BYTE byDataValueLength);//��������ܵ���Ϣ

	//void  SetPrivateData(BYTE byIndex, BYTE byObjectID, BYTE byAttributeID, BYTE byElementID, BYTE byDataValuePos, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData, BYTE byDataValueLength);//����˽�������� --HPH 2021.12.06

	void  SetOverlapConfig(BYTE byIndex, BYTE byObjectID, BYTE byAttributeID, BYTE byElementID, BYTE byDataValuePos, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData, BYTE byDataValueLength);//����˽��������:������λ --HPH 2021.12.06

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void  CreateWrongQueryReturnData(BYTE byIndex, TReturnData tWrongReturnData);//���ɲ�ѯ���󷵻ؽṹ��

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void  AckCtl_ReturnData(bool bCorrect, TReturnData & tReturnData);//Ӧ���ѯ����

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////
	int   AckCtl_AskHeart();//Ӧ������

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void  AckCtl_AskDeviceInfo(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void  AckCtl_AskBaseInfo(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void  AckCtl_AskLightGroupInfo(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void  AckCtl_AskPhaseInfo(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void  AckCtl_AskDetectorInfo(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void  AckCtl_AskPhaseStageInfo(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void  AckCtl_AskPhaseSafetyInfo(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void  AckCtl_AskEmergencyPriorityInfo(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void  AckCtl_AskPatternInfo(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void  AckCtl_AskTransitionRetain(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void  AckCtl_AskDayPlanInfo(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void  AckCtl_AskScheduleTable(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void  AckCtl_AskDeviceStatus(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);

	void  AckCtl_AskRunStatus(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void  AckCtl_AskTrafficData(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void  AckCtl_AskAlarmData(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void  AckCtl_AskFaultData(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void  AckCtl_AskCenterControl(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void  AckCtl_AskOrderPipe(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void  AckCtl_SetParamInfo(bool bCorrect, TReturnData & tReturnData);//Ӧ���������

	////////////////////////////////////////////////////////////////////////////////////////////////////////////HPH 2021.12.06
	void  AckCtl_AskPrivateData(BYTE byIndex, TReturnData & tCorrectReturnData, TReturnData & tWrongReturnData);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//��ȡ�׶�����ʱ���Լ�ʣ��ʱ��
	void GetStageRunTime(short &tRunTime, short &tRemainTime, int StageIndex, TPhaseRunStatus tPhaseRunStatus);

	void GetRoadTwoStageRunTime(short &tRunTime, short &tRemainTime, unsigned char & StageStatus, int StageIndex);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//���ɹ�����Ϣ
	void  AddFaultInfo(BYTE FalutType, BYTE FalutAction);

	void  GetSystemTimeZone(int & nTimeZoneHour, int & nTimeZoneMinute);

	void  SetSystemTimeZone();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////

    COMMWITHGB20999HANDLE            m_hThread;
    unsigned long                    m_dwThreadRet;
	bool                             m_bExitFlag;
    int                              m_nDetachState;

	COpenATCParameter         *      m_pOpenATCParameter;

	COpenATCRunStatus         *      m_pOpenATCRunStatus;

	COpenATCLog               *      m_pOpenATCLog;

	COpenATCPackUnpackBase   *		 m_pDataPackUnpackMode;

	COpenATCOperationRecord  m_tOpenATCOperationRecord;  //������¼����

	//-COpenATCParamCheck       m_openATCParamCheck;

	unsigned char            *		 m_chRecvBuff;

	unsigned char            *       m_chSendBuff;

	unsigned char            *       m_chPackedBuff;

	unsigned char            *		 m_chUnPackedBuff;

	time_t				             m_lastReadOkTime;

	int                              m_nSendTimeOut;

	int                              m_nRecvTimeOut;

	char                             m_szPeerIp[20];

	BYTE                             m_byFrameID;       

	TRunFaultInfo                    m_tRunAlarmInfo[C_N_MAX_FAULT_COUNT]; //������Ϣ

	TRunFaultInfo                    m_tRunFaultInfo[C_N_MAX_FAULT_COUNT]; //������Ϣ

	TAsc20999Param					 m_tAscParamInfo;						//20999�źŻ�����

	TAsc20999Param					 m_tVerifyParamInfo;					//����������Ʋ������õ��źŻ�����

	TAscParam                        m_tParamInfo;	//���ڻ�ȡ���źŻ���������

	//TAscParam                        m_tVerifyParamInfo;//����������Ʋ������õ��źŻ�����

	bool							 m_bIsNeedSave;		//�ж��Ƿ���Ҫ������������ڷ�������ƣ�

	COpenATCCfgCommHelper			 m_commHelper;

#ifndef _WIN32
	struct tm						 Alarm_tm;	//linux����ʱ��--HPH

	struct tm						 Utc_tm;	//UTCʱ��--HPH

	struct tm						 Local_tm;	//Localʱ��--HPH

#else
	SYSTEMTIME                       m_stAlarmTime;

	SYSTEMTIME                       m_stUTCTime;

	SYSTEMTIME                       m_stLocalTime;
#endif

	GB20999HANDLE                    m_hTDataProesshread;

    unsigned long				     m_dwDataProessThreadRet;

	static void *GB20999_CALLBACK    DBDataProessThread(void *pParam);

	int                              RunDBDataProessThread();

	TDBManagement					 m_tDBManagement;

	int                              m_nFaultDataIndex;

	int                              m_nFaultDataCount;

	BYTE m_nPipeInfo[16];

	bool m_bPhaseControlChange;
};

#endif // !ifndef OPENATCCOMMWITHGB20999THREAD_H
