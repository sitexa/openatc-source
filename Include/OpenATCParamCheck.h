/*=====================================================================
ģ���� ���źŻ����ò����Ϸ���У����
�ļ��� ��OpenATCParamCheck.h
����ļ���OpenATCParamCheck.cpp
ʵ�ֹ��ܣ������źŻ������������õ�У��
���� ������
��Ȩ ��<Copyright(c) 2019-2020 Suzhou Keda Technology Co.,Ltd. All right reserved.>
---------------------------------------------------------------------------------------------------------------------
�޸ļ�¼��
�� ��           �汾     �޸���     �߶���     �޸ļ�¼
2019/9/14       V1.0     ����        ����      ����ģ��
=====================================================================*/
#ifndef OPENATCPARAMCHECK_H
#define OPENATCPARAMCHECK_H

#include "../Include/OpenATCParamConstDefine.h"
#include "../Include/OpenATCParamStructDefine.h"

struct cJSON;

#ifdef _WIN32
    #ifdef DLL_FILE
    class _declspec(dllexport) COpenATCParamCheck
    #else
    class _declspec(dllimport) COpenATCParamCheck
    #endif
#else
    class COpenATCParamCheck
#endif
{
public:
	COpenATCParamCheck();
	virtual ~COpenATCParamCheck();

	//�źŻ���������У��
	int CheckAscParam(unsigned char* pData);

	//�źŻ��豸����У��
	int CheckSystemDeviceInfo_SiteID(unsigned char* pData);

	//�źŻ��豸����У��
	int CheckSystemDeviceInfo_Other(unsigned char* pData, bool bClearErrFlag);

	//�ֶ���尴��ͨ���̳�ͻУ��
	int CheckGreenConflictByPanel(unsigned char *pData, int nSize);

	//MD5��У��
	bool CheckMd5Code(unsigned char * pdataJsonBuffer, int nDataJsonBufferSize);

	//��ַ�����
	bool CheckSiteID(unsigned char* pdataJsonBuffer, int nDataJsonBufferSize, int nSiteID, bool bDeviceParamFlag);

	//��ȡ������
	void GetErrCode(TErrInfo(&atErrCodeTable)[C_N_MAX_ERR_SIZE]);

	//ͨ�������·������̳�ͻУ��
	int  CheckGreenConflictByChannelLock(int(&nChannelStatus)[MAX_CHANNEL_COUNT], unsigned char* pData, int nSize, char tChannelGreenConflictTable[MAX_CHANNEL_COUNT][MAX_CHANNEL_COUNT],BYTE* tChannelNum);

    //У�鲿�����صĲ����Ƿ���ȷ
	int  CheckPartParam(BYTE byParamType, unsigned char* pData, unsigned char* pParamBuffer);

	//����У����
	void SetCheckTranErrCode(int nDataValue, int nSubDataValue);

	void SetCheckTranErrCode(int nDataValue, int nSubDataValue, int nSubDataValueTwo);

private:

	static COpenATCParamCheck *m_pParamCheck;

	//��λ��Ϣ����У��
	void CheckPhaseInfo(cJSON* cPhaseObj);

	//����Ϣ����У��
	void CheckRingInfo();

	//���������λ��Ϣ
	void CheckOverlapInfo(cJSON* cOverlapObj);

	//������Ϣ����У��
	void CheckPatternInfo(cJSON* cPatternObj, int nPatternSize);

	//ʱ����Ϣ����У��
	void CheckDayPlanInfo(cJSON* cDayPlanObj);

	//���ȼƻ�����У��
	void CheckScheduleInfo(cJSON* cScheduleObj);

	//ͨ����Ϣ����У��
	void CheckChannelInfo(cJSON* cChannelObj);

	//ͨ��������Ϣ����У��
	void CheckChannelLockInfo(cJSON* cChannelObj, int nTimeSegIndex);

	//�������������У��
	void CheckVechDetectorInfo(cJSON* cVechDetectorObj);

	//���˼��������У��
	void CheckPedDetectorInfo(cJSON* cPedDetectorObj);

	//����Ӧ����У�� 2021.12.13
	void CheckSingleOptimInfo(cJSON* cSingleOpeimObj);

	//�ֶ�������У��
	void CheckManualPanelInfo(cJSON* cManualPanelObj);

	//�豸��Ϣ(siteid)У��
	void CheckCustomInfo_SiteID(cJSON* cCustomInfoObj);

	//�豸��Ϣ(��siteid�����Ϣ)У��
	void CheckCustomInfo_Other(cJSON* cCustomInfoObj);

	//������λУ��
	int CheckConcurrentPhase();

	//�����з���������λ������ͻУ��
	void CheckConcurrentConflictByPhase();

	//��ʼ�����������������λ�� ��ͨ����
	void InitCheckInfo(unsigned char *pData, int nSize);

	//����ͨ��ID����ͨ�����
	int GetChannelNum(int nChannelID);

	int GetChannelNum(int nChannelID, BYTE* tChannelNum);

	//���ݲ��� ���� ͨ����Ϣ��ʼ��ͨ���̳�ͻ��
	void InitGreenConflictbyChannel();

	//�������з����Ľ׶α�
	void GetPatternStageTable(int nPatternIndex);

	//����ʱ��λ�ø�ʱ���ڸ�ִ�еķ�����
	void GetPatternByDayPlan(int nStartTime, int nEndTime, BYTE(&abyPattern)[MAX_PATTERN_COUNT]);

	//ֵ��У��
	int CheckValueRange(int nDataValue, int nMinValue, int nMaxValue);

	//�ж���λ�Ƿ��̳�ͻ
	bool CheckConcurrentConflictByPhaseConcurInfo(int nPatternIndex);

	//�ж��Ƿ�Ϊ��Чip
	bool is_valid_ip(char *ip);

	//������·���ͻ������ȡ��ص�ͨ����ͻ��Ϣ������У��
	void GetChannelGreenConflictInfo(cJSON* cGreenConflictObj);

	//�����·�����λ��������λ����Ϣ����Ĭ�ϵ�ͨ����ͻ������У��
	void CreateGreenConflictbyChannel(char tChannelGreenConflictTable[MAX_CHANNEL_COUNT][MAX_CHANNEL_COUNT]);

	//��ȡ������λ��ź���λ������ --���ڵ����·�������У���������
	void GetCheckPatternRequiredParameters(unsigned char* pParamBuffer);

	//��ȡ���䷽����� --���ڵ����·��ƻ���У���������
	void GetCheckplanListRequiredParameters(unsigned char* pParamBuffer);

	//�����µļƻ�����Ϣ�����ڱ����У��
	void CheckDateListByNewPlanList(unsigned char* pParamBuffer);

	//��ȡ����ƻ���� --���ڵ����·����ڱ�У���������
	void GetCheckdateListRequiredParameters(unsigned char* pParamBuffer);

	int m_nPhaseCfgTable[MAX_PHASE_COUNT];										//������λ���

	int m_n0verlapPhaseTable[MAX_OVERLAP_COUNT];								//���������λ���

	int m_nPatternCfgTable[MAX_PATTERN_COUNT];									//���䷽�����

	int m_nPlanCfgTable[MAX_DAYPLAN_TABLE_COUNT];								//�����õļƻ����

	BYTE m_byCoorFlag[MAX_DAYPLAN_TABLE_COUNT];									//��Ӧ�ļƻ��Ƿ�ΪЭ�����Ƽƻ���1Ϊ��Э���ƻ���0Ϊ��Э���ƻ�

	BYTE m_byMonthWeek[12][7];													//��Э���ƻ���������

	BYTE m_byMonthDay[12][31];													//��Э���ƻ���������

	int m_nPatternSize;															//���䷽������

	int m_nDayPlanCfgSize;														//����ʱ������

	int m_nSplitLimitTable[MAX_PHASE_COUNT];									//���ű�����ֵ��

	int m_nSplitTopLimitTable[MAX_PHASE_COUNT];									//���ű�����ֵ��

	int m_nSplitMode[MAX_PHASE_COUNT];											//���ű�ģʽ

	int m_nCurChannelTable[MAX_CHANNEL_COUNT];									//����ͨ�����

	int m_nErrCodeIndex;				

	int m_nCheckPhaseConcurInfo[MAX_PHASE_COUNT][MAX_PHASE_COUNT];				//��У�鲢��

	char m_nCheckChannelGreenConflict[MAX_CHANNEL_COUNT][MAX_CHANNEL_COUNT];	//��У��ͨ���̳�ͻ,0��ʾ����ͬʱ���̵�,1��ʾ����ͬʱ���̵�

	char m_achChannelGreenConflictInfo[MAX_CHANNEL_COUNT][MAX_CHANNEL_COUNT];	//ͨ���̳�ͻ��Ϣ��,0��ʾ����ͬʱ���̵�,1��ʾ����ͬʱ���̵�

	TAscParam m_atAscParamInfo;

	int m_nChannelSize;															//������ͨ����

	TChannel m_atChannelInfo[MAX_CHANNEL_COUNT];								//ͨ����Ϣ��

	int		 m_atChannelType[MAX_CHANNEL_COUNT];								//ͨ�����Ա�

	int		 m_atChannelSource[MAX_CHANNEL_COUNT];								//ͨ������Դ

	int m_nPhaseConcurInfo[MAX_PHASE_COUNT][MAX_PHASE_COUNT];					//��������

	TOverlapTable m_atAscOverlapTable[MAX_OVERLAP_COUNT];						//������λ��

	int m_atOverlapIncludePhases[MAX_OVERLAP_COUNT][MAX_PHASE_COUNT_IN_OVERLAP];//������λ��������λ��

	TPatternInfo  m_atPatternInfo[MAX_PATTERN_COUNT];							//���з�������Ϣ

	TPatternStageInfo m_tAllPaternStageInfo[MAX_PATTERN_COUNT];					//���з����Ľ׶���Ϣ 

	TErrInfo m_nCheckTranErrCode[C_N_MAX_ERR_SIZE];								//У��������

	int m_nParamRingNum[MAX_PHASE_COUNT];										//�����ж������еĻ������Ƿ��1��ʼ����������
};
#endif// !ifndef  OPENATCPARAMCHECK_H
