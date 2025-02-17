/*=====================================================================
ģ���� �����ذ����в�����
�ļ��� ��OpenATCParameter.h
����ļ���OpenATCParameter.cpp
ʵ�ֹ��ܣ��������ذ������������������
���� ��������
��Ȩ ��<Copyright(c) 2019-2020 Suzhou Keda Technology Co.,Ltd. All right reserved.>
---------------------------------------------------------------------------------------------------------------------
�޸ļ�¼��
�� ��           �汾     �޸���     �߶���     �޸ļ�¼
2019/9/14       V1.0     ������      ����       ����ģ��
=====================================================================*/

#ifndef OPENATCPARAMETER_H
#define OPENATCPARAMETER_H

#include "../Include/OpenATCParamConstDefine.h"
#include "../Include/OpenATCParamStructDefine.h"
#include "../Include/OpenATC20999ParamStructDefine.h"
#include "../Include/OpenATCRunStatus.h"
#include "../Include/OpenATCLog.h"
#include "../Include/OpenATCFlowProcDefine.h"
#include "../OpenATCManager/comctl/OpenATCComDef.h"

#ifdef VIRTUAL_DEVICE
#define JSON_FILE_TZ_PATH "./config/OpenATCTZParam.json"
#define JSON_FILE_HW_PATH "./config/OpenATCHWParam.json"
#define XML_FILE_CONFIG_PATH "./config/ConfigPort.xml"
#else
#define JSON_FILE_TZ_PATH "/mnt/OpenATCTZParam.json"
#define JSON_FILE_HW_PATH "/mnt/OpenATCHWParam.json"
#define XML_FILE_CONFIG_PATH "/usr/config/ConfigPort.xml"
#endif

#ifdef VIRTUAL_DEVICE
#define FAULT_FILE_PATH "./log/FAULT.json"
#else
#define FAULT_FILE_PATH "/usr/log/FAULT.json"
#endif

const int C_N_MAXJOSNBUFFER_SIZE			= 1024 * 1024;
const int C_N_MAX_DEVICE_PARAM_BUFFER_SIZE	= 1024;
const int C_N_MAX_TIMEBASESCHEDULE_MONTH	= 12;
const int C_N_MAX_TIMEBASESCHEDULE_WEEK		= 7;
const int C_N_MAX_TIMEBASESCHEDULE_DAY		= 31;
const int C_N_MAX_TIMEBASEDAYPLAN_HOUR		= 24;
const int C_N_MAX_TIMEBASEDAYPLAN_MINUTE	= 60;
const int C_N_MAXFAULTBUFFER_SIZE			= 1024 * 1024 * 2;
const int C_N_MAXSIDE_SIZE			        = 7;

class COpenATCParamCheck;

/*=====================================================================
���� COpenATCParameter
���� �����ڴ洢����ȡ���ذ���������Ҫ������������
��Ҫ�ӿڣ�
��ע ��
---------------------------------------------------------------------------------------------------------------------
�޸ļ�¼��
�� ��          �汾     �޸���     �߶���       �޸ļ�¼
2019/09/14     V1.0     ������      ����         ������
=====================================================================*/

#ifdef _WIN32
    #ifdef DLL_FILE
    class _declspec(dllexport) COpenATCParameter
    #else
    class _declspec(dllimport) COpenATCParameter
    #endif
#else
    class COpenATCParameter
#endif
{
public:
	COpenATCParameter();
	virtual ~COpenATCParameter();

	//��ʼ����������
	void Init(COpenATCRunStatus * pRunStatus, COpenATCLog * pOpenATCLog);

	//����ʱ����Ϣ����ϵͳʱ��
	void SetSystemTimeZone();

	//��ȡʱ����Ϣ
	void GetSystemTimeZone(int & nTimeZoneHour, int & nTimeZoneMinute);

	//��ȡ��������
	void GetAscParamInfo(TAscParam &tAscParamInfo);

    //���õ�ַ��ID
    void SetSiteID(int nSiteID);

	//��ȡ��ַ��ID
	int GetSiteID();

	//�������ڻ�ȡ��Ӧ�ĵ��ȼƻ���
	void GetTimeBaseScheduleByTime(int nMonth, int nWeek, int nDate, TTimeBaseSchedule & tScheduleInfo, int &nCount);

	//����ʱ�λ�ȡ��Ӧ��ʱ�α���Ϣ
	void  GetTimeBaseDayPlanByTime(int nHour, int nMinute, BYTE wTimeBaseScheduleNumber, TTimeBaseDayPlan & tTimeBaseDayPlanInfo);

	//���ݷ����Ż�ȡ������Ϣ
	void GetPatternByPlanNumber(int nPlanNumber, TPattern &tPatternInfo);

	//�������űȱ�Ż�ȡ�����ű���Ϣ��	
	void GetSplitBySplitNumber(int nSplitNumber, TSplit(&atSplitInfo)[MAX_PHASE_COUNT]);

	//����ʱ���Ż�ȡʱ���
	void GetSequenceBySequenceNumber(int nSequenceNumber, TSequence(&atSequenceInfo)[MAX_RING_COUNT]);

	//������λ�Ż�ȡ��Ӧ����λ��Ϣ
	void GetPhaseByPhaseNumber(int nPhaseNumber, TPhase & tPhaseInfo);

	//���źŻ������ṹ���л�ȡ������λ��
	void GetOverlapTable(TOverlapTable(&atOverlapTable)[MAX_OVERLAP_COUNT]);

	//���źŻ������ṹ���л�ȡͨ����
	void GetChannelTable(TChannel(&atChannelTable)[MAX_CHANNEL_COUNT]);

	//���źŻ������ṹ���л�ȡ��λ��
	void GetPhaseTable(TPhase(&atPhaseTable)[MAX_PHASE_COUNT]);
	
	//���źŻ������ṹ�л�ȡ�����������
	void GetVehicleDetectorTable(TVehicleDetector(&atVehicleDetectorTable)[MAX_VEHICLEDETECTOR_COUNT]);

    //���źŻ������ṹ�л�ȡ���˼������
    void GetPedDetectorTable(TPedestrianDetector(&atPedDetectorTable)[MAX_PEDESTRIANDETECTOR_COUNT]);

	//���źŻ������ṹ�л�ȡ�������ò���
	void GetNetCardsTable(TAscNetCard(&atNetCardsTable)[MAX_NETCARD_TABLE_COUNT]);

	//���źŻ������ṹ�л�ȡƽ̨�������ò�����Ϣ
	void GetCenterInfo(TAscCenter& tCenterInfo);

	//���źŻ������ṹ�л�ȡ����ƽ̨�������ò�����Ϣ
	void GetSimulateInfo(TAscSimulate& tSimulateInfo);

	//���źŻ������ṹ�л�ȡ·�����ò�����Ϣ��·�ںţ�����ţ�
	void GetAscAreaInfo(TAscArea& tAreaInfo);
	
	//���źŻ������ṹ���л�ȡ�ֶ��������
	void GetManualPanelInfo(TAscManualPanel& tManualPanel);

	//���ݷ����Ż�ȡ��������Ӧ���Ʋ���
	void GetSingleOptimInfo(int nPlanNumber, TAscSingleOptim& tSingleOptimInfo, int RingIndex, int *PhaseIndex);

	//��ȡ����������Ϣ
	void GetAscCasCadeInfo(TAscCasCadeInfo &tCasCade);

	//��ȡͨ������������Ϣ
	void GetChannelLockInfo(TAscOnePlanChannelLockInfo(&atChannelLockInfo)[MAX_SINGLE_DAYPLAN_COUNT]);

	//��ȡ����ʱ�������Ϣ
	void GetStartSequenceInfo(TAscStartSequenceInfo &tStartSequenceInfo);

	//��ȡ���ϼ���������
	void GetFaultDetectInfo(TAscFaultCfg &tFaultCfg);

	//��ȡ��������
	void GetStepInfo(TAscStepCfg &tStepCfg);

	//��ȡ����buffer������Ϣ
	unsigned char* GetParamData();

	//��ȡ��λ������Ϣ
	unsigned char* GetPhaseParamData();

	//��ȡ����������Ϣ
	unsigned char* GetPatternParamData();

	//��ȡ���ڲ�����Ϣ
	unsigned char* GetDateParamData();

	//��ȡ������λ������Ϣ
	unsigned char* GetOverLapParamData();

	//��ȡ�ƻ�������Ϣ
	unsigned char* GetPlanParamData();

	//��ȡ���������������Ϣ
	unsigned char* GetVecDetectorParamData();
    
	//��ȡ���˼����������Ϣ
	unsigned char* GetPedDetectorParamData();

	//��ȡͨ��������Ϣ
	unsigned char* GetChannelParamData();

	//��ȡ�źŻ���ǰϵͳʱ����Ϣ�����������ʱ���ѯ��
	unsigned char * GetATCLocalTime();

	//��ȡ�źŻ�ʶ����
	unsigned char * GetATCCode();

	//��ȡ���������汾
	unsigned char * GetATCParamVersion();

	//��ȡ�źŻ��汾
	unsigned char * GetATCVersion(char * pATCVersion);

	//��ȡ������ʽ
	unsigned char * GetWorkPattern();

	//��ȡ�豸������Ϣ
	unsigned char * GetSystemCustom();

	//��ȡ��������״̬��Ϣ
	unsigned char* GetPatternRunStatusData();

	//��ȡ�����ϱ���Ϣ
	unsigned char* GetATCFaultReportData(TAscFault *pTAscFault);
	
	//���ʵʱ�����ӿ�
	unsigned char* GetCurrentTrafficFlowData(TStatisticVehDetData* pTVehData);

	//��ȡ��ѯ������Ϣ(���й�����Ϣ)
	unsigned char* GetATCFaultQueryData(int & iMyJsonBufferSize);

	//��ȡͨ���Ƶ�ѹ�Ƶ�����Ϣ
	unsigned char* GetATCChannelStatus(TChannelStatusInfo(&atChannelStatusInfo)[MAX_CHANNEL_COUNT]);

	//��ȡͨ����ɫ״̬
	unsigned char* GetChannelLampStatusInfo();

	//ע��ƽ̨��������
	unsigned char* GetAscLoginCenterData();
	
	//��ȡԶ�̿���ָ��ֵ
	int GetRemoteValueValue(unsigned char* pRemote);

	//����������ʽ
	void GetWorkModeParam(unsigned char* pWorkMode, TWorkModeParam &atWorkModeParam, TPhasePassCmdPhaseStatus &atPhasePhassCmdPhaseStatus, TChannelLockCtrlCmd &atChannelLockCtrlCmd, TPhaseLockCtrlCmd &atPhaseLockCtrlCmd, TPreemptCtlCmd &atPreemptCtlCmd);
	
	//�����źŻ�ʱ������
	void GetAscTimeSetValue(unsigned char* pTimeSet,time_t &timeValue);

	//�����źŻ�Զ�̵�������
	void GetAscRemoteControlValue(unsigned char* pRemote, TAscRemoteControl& atRemote);

	//����ͨ���ɼ��������Ϣ
	void GetAscChannelVerifyValue(unsigned char* pChannelVerify, TAscChannelVerifyInfo& atChannelVerify);

	//����ƽ̨���ͻ�ȡ��ʷ��������ָ��
	void GetAscGainTrafficFlowCmd(unsigned char* pGainTrafficFlowCmd, TAscGainTafficFlowCmd& atGainTrafficFlowCmd);

	//����ƽ̨���͵ķ�����Ԥ������Ϣ
	void GetAscInterruptPatternInfo(unsigned char* pInterruptPattern);

	//�������ϲ�ѯָ��ֵ
	int GetAscQueryFaultValue(unsigned char* pFault);

	//��ȡ����buffer��Ч������С
	int GetParamDataSize();

	//������ż���
	int CheckPatternNum(TWorkModeParam *pTWorkMode);
	
	//������������
	int SaveParameter(unsigned char * pData, int nSize);

	//�����豸����
	int SavaSystemCustom(unsigned char * pData, int nSize);

	// ����U��
	int MountUSBDevice(COpenATCRunStatus * pRunStatus, char & chFailedReason);
	
	// ж��U��
	int UnmountUSBDevice(COpenATCRunStatus * pRunStatus);

	// ����GPS��Ϣ
	void SetGpsInfo(const TGpsData & tGpsData);

	// �����豸����
	bool GenDeviceParamFile(unsigned char *pJsonBuffer, const char* pFilePath);

    //ͨ������ָ���̳�ͻУ��
	int  CheckGreenConflictByChannelLock(int(&nChannelStatus)[MAX_CHANNEL_COUNT]);
	
	//У�鲿�����صĲ����Ƿ���ȷ,��ȷ�Ļ�,Ҫ����ȫ��buffer�������µĲ����ļ�
	int  CheckPartParamAndUpdateParam(BYTE byParamType, unsigned char *pData, TErrInfo(&atErrCodeTable)[C_N_MAX_ERR_SIZE]);
	
	//���÷�����Ԥ������Ϣ
	void SetAscInterruptPatternInfo(TInterruptPatternInfo tInterruptPatternInfo);

	//�����ֶ����Ʒ���
	void SetManualControlPattern(TInterruptPatternInfo tInterruptPatternInfo);

	//���źŻ������ṹ�л�ȡͨ���̳�ͻ
	void GetChannelGreenConflict(char tChannelGreenConflict[MAX_CHANNEL_COUNT][MAX_CHANNEL_COUNT]);

	//��ȡͨ���̳�ͻ����������
	int GetChannelGreenConflictCount();

	//�������ȱ�Ż�ȡ�������Ȳ���
	bool GetPreemptParam(int nPreemptIndex, TPreempt & tPreempt);
	
	//��ȡ��ַ��ID
    void GetSiteID(char * pSiteIDFromATC, char * pSiteIDFromParam);

	//GB20999���
	void Gb20999ToGb25280(TAsc20999Param tTempAsc20999Param, TAscParam& atTAscParam);

	//�����в�����ȡ20999��ʽ����
	void GetAscParamByRunParam(TAsc20999Param& tTempAsc20999Param);

	//��ȡ��λ�ĵ�����Ϣ
	void GetPhaseLightGroup(BYTE tPhaseNum, BYTE* tLightGroup);

	//��ȡ��λ������
	void GetPhaseCall(BYTE tPhaseNum, BYTE* tPhaseCall);

	//ͨ����λ������λ����ȡ��λ��ͻ��
	void GetPhaseConflict(BYTE tPhaseNum, BYTE* tPhaseConflict);

	//20999�����ṹת25280�����ṹ
	void Gb20999ToGb25280(TAscParam& atTAscParam, TAsc20999Param tTempAsc20999Param);

	//���û�����������
	void SetAscTempParamInfo(const TAscParam& tAscParamInfo);

	//20999�����ṹ���ݱ���
	int SaveGB20999ASCParam(bool isTransaction/*TAscParam* tAscParamInfo, TErrInfo(&atErrCodeTable)[C_N_MAX_ERR_SIZE]*/);

	//��ѯ���ҽ׶��Ƿ��Ѿ�����
	bool CheckStageExistence(BYTE* tStageTablePhase, BYTE* tNewStagePhase);

	//��ȡ��λ�׶���Ϣ����λ�׶ε���λ
	void GetPhaseInStage(BYTE* tPhaseStage, BYTE* tPhaseInPhaseStage);

	//���������Ѿ��ҵ������Ⱥ�
	bool HaveFindNum(int tPreempt, int* tPreemptNum);

	//�ɰ�������λ�����Ҷ�Ӧ�Ľ׶κ�
	int GetStageNumByPhaseInfo(BYTE* tPhaseInfo, int tPhaseStageCount, T20999PhaseStage* m_stPhaseStage);

	//�����ù��߻�ȡ�߳�ѡ������
	int GetCommThreadInfo();

	//��ȡ������
	int GetDatePlanNum();

	//����ʱ�α������ȡʱ�α�����
	int GetDayPlanIndexByNum(BYTE byDayPlanNum);

#ifdef VIRTUAL_DEVICE
	//��ȡ�źŻ����ڼ�������ѡ��
	int	GetSpeedyRunInfo();

	//�ж��źŻ��Ƿ���Ҫ����ָ�����õ�ʱ������ //Virtual_Test2022
	void SetStartTime();

	//���չ�ʽ�������� //Virtual_Test2022
	int GetWeek(int nYear, int nMonth, int nDay);

	//�ж�����//Virtual_Test2022
	int isLeap(int year);
#endif // VIRTUAL_DEVICE
//Virtual_Test2022

private:
	//md5��У��
	bool CheckMd5Code(unsigned char * pdataJsonBuffer, int nDataJsonBufferSize);

	//��ַ�����
	bool CheckSiteID(unsigned char* pdataJsonBuffer, int nDataJsonBufferSize, int nSiteID, bool bDeviceParamFlag);

	//��ϵͳ������־�м�¼У�����ԭ��
	void RecordVerifyErrorToSystemLogFile(TErrInfo *pCheckCode);

	//�ڹ�����־�м�¼У�����ԭ��
	void RecordVerifyErrorToFaultLogFile(TErrInfo *pCheckCode);

	//���÷�����Ԥ����
	void ReSetInterruptPatternInfo();

private:
	//��ջ�����
	void ClearBuffer();

	COpenATCRunStatus  *m_pOpenATCRunStatus;								//ȫ������״̬��ָ��
	 
	COpenATCLog        *m_pOpenATCLog;										//������־ָ��

	TAscParam  m_tAscParamInfo;												//��������

	TAscParam  m_tTempAscParamInfo;                                         //��ʱ��������		//20999�����洢����

	unsigned char m_chParamBuffer[C_N_MAXJOSNBUFFER_SIZE];					//��������buffer

	int m_nJsonBufferSize;													//����������Ч�ߴ�

	unsigned char m_chDeviceParamBuffer[C_N_MAX_DEVICE_PARAM_BUFFER_SIZE];	//�豸����buffer

	int m_nDeviceParamBufferSize;											//�豸������Ч�ߴ�
 
	unsigned char m_chFaultBuffer[C_N_MAXFAULTBUFFER_SIZE];					//����buffer

	int m_nSiteID;															//��ַ��

	TGpsData m_nGpsData;													//GPS��Ϣ

	COpenATCParamCheck*       m_openATCParamCheck;

	TInterruptPatternInfo    m_atInterruptPatternInfo;                      //���ⷽ����Ԥ��Ϣ

	bool m_bIsSetSimStartTime;												//�Ƿ������˷�������ʱ��
	
	char m_szSiteIDFromATC[C_N_MAXSIDE_SIZE];                               //�źŻ�ʵ�ʵ�ַ��

	char m_szSiteIDFromParam[C_N_MAXSIDE_SIZE];							    //�����еĵ�ַ��

};

#endif // !ifndef OPENATCPARAMETER_H
