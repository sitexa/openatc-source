/*=====================================================================
ģ���� �����ϼ��ģ��
�ļ��� ��OpenATCFaultProcManager.h
����ļ���OpenATCParameter.h OpenATCRunStatus.h
ʵ�ֹ��ܣ����ϼ��ģ������࣬���ڹ��ϼ�⡣
���� ��������
��Ȩ ��<Copyright(c) 2019-2020 Suzhou Keda Technology Co.,Ltd. All right reserved.>
------------------------------------------------------------------------
�޸ļ�¼��
�� ��           �汾     �޸���     �߶���     �޸ļ�¼
2019/9/26       V1.0     ������     ������     ����ģ��
=====================================================================*/

#ifndef OPENATCFAULTPROCMANAGER_H
#define OPENATCFAULTPROCMANAGER_H

#include "../Include/OpenATCParameter.h"
#include "../Include/OpenATCRunStatus.h"
#include "../Include/OpenATCRunStatusDefine.h"
#include "../Include/CanBusManager.h"

#include <mutex>

#define LAMP_DRV_ERROR_ON_VOLT_NONE    0xffffff00
#define LAMP_DRV_ERROR_ON_VOLT_FAIL    0x00000001 //δ�����Ч��ѹ
#define LAMP_DRV_ERROR_ON_VOLT_LOW     0x00000002 //�����ѹ���������ѹ����
#define LAMP_DRV_ERROR_ON_VOLT_HIGH    0x00000004 //�����ѹ���������ѹ
#define LAMP_DRV_ERROR_OFF_VOLT_NONE   0xffff00ff
#define LAMP_DRV_ERROR_OFF_VOLT_HIGH   0x00000100 //�ر������ʵ����Ȼ���
#define LAMP_DRV_ERROR_OFF_VOLT_LOW    0x00000200 //�ر������ʵ�ʲ������
#define LAMP_DRV_ERROR_LINE_AC_RESIDUE 0x00000400 //��·������ѹ����

#define LAMP_DRV_ERROR_ON_POWER_NONE   0xff00ffff
#define LAMP_DRV_ERROR_ON_POWER_UP     0x00010000 //�ƹ����쳣����
#define LAMP_DRV_ERROR_ON_POWER_DOWN   0x00020000 //�ƹ����쳣��С
#define LAMP_DRV_ERROR_ON_POWER_ZERO   0x00800000 //�ƹ��������
#define LAMP_DRV_ERROR_OFF_POWER_NONE  0x00ffffff
#define LAMP_DRV_ERROR_OFF_POWER_HIGH  0x01000000 //�ر�״̬�й������

typedef struct tagOnePhaseConcurInfo
{
    BYTE m_byPhaseNum;
    char m_achPhaseConcurInfo[MAX_PHASE_CONCURRENCY_COUNT];
}TOnePhaseConcurInfo,*PTOnePhaseConcurInfo;

typedef struct tagPhaseConcurInfo
{
    TOnePhaseConcurInfo m_atPhaseConcurInfo[MAX_PHASE_COUNT];
    int m_nPhaseCount;    
}TPhaseConcurInfo,*PTPhaseConcurInfo;

typedef struct tagOneOverlap
{
    BYTE m_byOverlapNumber;
    BYTE m_byArrOverlapIncludedPhases[MAX_PHASE_COUNT_IN_OVERLAP];
}TOneOverlap, *PTOneOverlap;

typedef struct tagOverlapInfo
{
    TOneOverlap m_atOverlapInfo[MAX_OVERLAP_COUNT];
    int m_nOverlapCount;
}TOverlapInfo, *PTOverlapInfo;

typedef struct tagOneLampCtlBoardFaultStatus
{
    char m_chLampCtlBoardOnline;                                                //�忨���߹���,1Ϊ����,0Ϊ������
    char m_chRedAndGreenOnFault;                                                //�������ͬ������
    char m_achLampPower[C_N_LAMPBORAD_OUTPUTNUM];                               //�ƹ��ʹ���,0Ϊ����,1Ϊ����
    char m_achLampVoltage[C_N_LAMPBORAD_OUTPUTNUM];                             //�Ƶ�ѹ����,0Ϊ����,1Ϊ����    
}TOneLampCtlBoardFaultStatus,*PTOneLampCtlBoardFaultStatus;                     //��¼�ƿذ����״̬,����ģ������ݽ���

typedef struct tagLampCtlBoardFaultStatus
{
    int m_nLampCtlBoardCount;                                                   //�忨����
    char m_chRedLampOutFault;                                                   //���ȫ�����
    char m_chGreenConflictFault;                                                //�̳�ͻ����
    char m_chLampCtlBoardNumFault;                                              //�ƿذ���������
    char m_achLampCtlBoardUseStatus[C_N_MAXLAMPBOARD_NUM];                      //�忨ʹ��״̬,������������ȡ 
    TOneLampCtlBoardFaultStatus m_atLampCtlBoardFault[C_N_MAXLAMPBOARD_NUM];    //�忨����״̬��Ϣ����
}TLampCtlBoardFaultStatus,*PTLampCtlBoardFaultStatus;

typedef struct tagOneVehDetBoardFaultStatus
{
    char m_chDetBoardOnline;                                                    //�忨���߹���,1Ϊ����,0Ϊ������
    char m_achDetectorUseStatus[C_N_MAXDETINPUT_NUM];                           //�����ʹ��״̬,1Ϊʹ��,0Ϊ��ʹ��
    char m_achDetectorFault[C_N_MAXDETINPUT_NUM];                               //���������״̬,0Ϊ����,1Ϊ����
}TOneVehDetBoardFaultStatus,*PTOneVehDetBoardFaultStatus;

typedef struct tagVehDetBoardFaultStatus
{
    int m_nDetBoardCount;                                                       //�忨����
    char m_achVehDetBoardUseStatus[C_N_MAXDETBOARD_NUM];                        //�忨ʹ��״̬,������������ȡ                                                       
    TOneVehDetBoardFaultStatus m_atDetBoardFault[C_N_MAXDETBOARD_NUM];          //�忨����״̬��Ϣ����
}TVehDetBoardFaultStatus,*PTVehDetBoardFaultStatus;

typedef struct tagOneIOBoardFaultStatus
{
    char m_chIOBoardOnline;                                                    //�忨���߹���,1Ϊ����,0Ϊ������
}TOneIOBoardFaultStatus,*PTOneIOBoardFaultStatus;

typedef struct tagIOBoardFaultStatus
{
    int m_nIOBoardCount;                                                       //�忨����                                                       
    char m_achIOBoardUseStatus[C_N_MAXIOBOARD_NUM];                            //�忨ʹ��״̬,������������ȡ                                                       
    TOneIOBoardFaultStatus m_atIOBoardFault[C_N_MAXIOBOARD_NUM];               //�忨����״̬��Ϣ����
}TIOBoardFaultStatus,*PTIOBoardFaultStatus;

typedef struct tagFaultDetBoardFaultStatus
{
    char m_chFaultDetBoardOnline;                                              //�忨���߹���,1Ϊ����,0Ϊ������
}TFaultDetBoardFaultStatus,*PTFaultDetBoardFaultStatus;

typedef struct tagMainCtlBoardFaultStatus
{
    char m_achCanBusFault[C_N_MAXCANBUS_NUM];                                  //can���߹���,0Ϊ����,1Ϊ����
    char m_chTZParamFault;                                                     //������������,0Ϊ����,��0Ϊ����
    char m_chYellowFlashFault;                                                 //����������,0Ϊ����,��0Ϊ����
    TFaultDetBoardFaultStatus m_tFaultDetBoardStatus;                          //���ϼ��������Ϣ
}TMainCtlBoardFaultStatus,*PTMainCtlBoardFaultStatus;

typedef struct tagOpenATCFaultStatus
{
    TMainCtlBoardFaultStatus m_tMainCtlBoardStatus;
    TLampCtlBoardFaultStatus m_tLampCtlBoardStatus;
    TVehDetBoardFaultStatus m_tVehDetBoardStatus;
    TIOBoardFaultStatus m_tIOBoardStatus;    
}TOpenATCFaultStatus,*PTpenATCFaultStatus;

#ifdef _WIN32
	#define OPENATCFAULTLOG_CALLBACK WINAPI
	typedef HANDLE               OPENATCFAULTLOGHANDLE;
#else
	#define OPENATCFAULTLOG_CALLBACK
	typedef pthread_t            OPENATCFAULTLOGHANDLE;
#endif

#ifdef _WIN32
    #ifdef OpenATCFaultProc_EXPORTS
    class _declspec(dllexport) COpenATCFaultProcManager
    #else
    class _declspec(dllimport) COpenATCFaultProcManager
    #endif
#else
    class COpenATCFaultProcManager
#endif
{
public:
    //�ඨ��Ϊ����
    static COpenATCFaultProcManager * getInstance();

    //��ĳ�ʼ������
    void Init(COpenATCParameter * pParameter,COpenATCRunStatus * pRunStatus, COpenATCLog * pOpenATCLog);

    //���������
    void Work();

    //�Լ�����
    void SelfDetect();    

    //���ֹͣ���ͷ�
    void Stop();

    //��ȡ�̳�ͻͨ����                 
    void GetGreenConflictTable(int nChannelIndex, unsigned char chSendBuff[]);

    //�ֶ�ģʽ�£��������ʱ��ͨ����ͻ�ж�                   
    bool ConflictProcByChannel(int nKeyIndex);

	const char* GenDetailedFaultInfo(int nBoardType);

	//�������
	void AddOneFaultMessage(char cBoardType, char cFaultLevel, DWORD wFaultType, DWORD wFaultSubType, char cFaultInfo1, char cFaultInfo2);

	//�������
	void EraseOneFaultMessage(char cBoardType, char cFaultLevel, DWORD wFaultType, DWORD wFaultSubType, char cFaultInfo1, char cFaultInfo2);

	//�����Լ����
	void SaveParamInitFault(TParamRunStatus tParamStatus, COpenATCRunStatus * pRunStatus, COpenATCLog * pOpenATCLog);

	//�����Լ����
	void ClearParamInitFault(TParamRunStatus tParamStatus, COpenATCRunStatus * pRunStatus, COpenATCLog * pOpenATCLog);

private:
	COpenATCFaultProcManager();
	~COpenATCFaultProcManager();

    //����������������ʼ�����ϼ�����
    void InitFaultProcParam(bool bParamChg);

    //���ݲ�����λ,ͨ��,������λ�����̳�ͻ��Ϣ��
    void InitGreenConflictInfo();

    //���ݲ����ж�����ͨ���Ƿ��ͻ
    bool IsTwoChannelConflict(BYTE bySrcChannelIndex,BYTE byDstChannelIndex,bool bAllChannel);

    //��ȡͨ����Ӧ����λ���
    void GetChannelSrcPhase(BYTE byChannelIndex,int & nPhaseCount,BYTE * pPhaseInfo,bool bAllChannel);

    //�忨�Լ�
    bool AllBoardSelfDetect();

    //�ƿع��ϼ��
    void LampCtlFaultProc();
    //�ƿذ������������
    bool LampNumFaultProc(TLampCltBoardData & tLampCtlBoardInfo);
    //�̳�ͻ���
    bool GreenConflictProcByPhase(TLampCltBoardData & tLampCtlBoardInfo);
    bool GreenConflictProcByChannel(TLampCltBoardData & tLampCtlBoardInfo);
    //�������ͬ�����
    bool GreenAndRedOnProc(TLampCltBoardData & tLampCtlBoardInfo);
    //�޺��������
    bool NoRedProc(TLampCltBoardData & tLampCtlBoardInfo);
    
    //������ϼ��
    void VehDetFaultProc();
    //����������������
    bool VehDetNumFaultProc(TVehDetBoardData & tVehDetBoardInfo);
    //�������Ȧ���ϼ��
    bool VehDetectorFaultProc(TVehDetBoardData & tVehDetBoardInfo);

    //���ع��ϼ��
    void MainCtlFaultProc();
    //CAN���߹��ϼ��
    bool CanBusFaultProc();
    //���ϼ�����ϼ��    
    bool FaultDetFaultProc(TFaultDetBoardData & tFaultDetBoardInfo);
    //���������ϼ��
    bool YellowFlashFaultProc();
    //�����������ϼ��
    bool TZParamFaultProc(TParamRunStatus & tParamStatus);

    //������ϼ��
    void IOFaultProc();

    void LampCtlGroupFault(TLampCltBoardData & tLampCtlBoardInfo);

    WORD TransTZParamFaultValue(unsigned int nTZParamFault);

    int  TransGroupFaultValue(int nBoardIndex, int nChannelIndex, int nLampClrIndex, unsigned int nGroupFault, unsigned char chGroupFault[], WORD wFaultType[], WORD wSubFaultType[]);

	bool IsLockChannel(int nChannelIndex);

	void InitLampBoardCtrParamByIndex();

	void InitDetectorParamByIndex();

	void SetLampBoardCtrParamFlag();

	void SetDetectorParamFlag();

	void ResetLampBoardCtrParam();

	void ResetDetectorParam();

	void AddLedScreenShowFaultInfo(char cBoardType, char cFaultLevel, DWORD wFaultType, DWORD wSubFaultType, char cFaultInfo1, char cFaultInfo2);

	void EraseLedScreenShowFaultInfo(char cBoardType, char cFaultLevel, DWORD wFaultType, DWORD wSubFaultType, char cFaultInfo1, char cFaultInfo2);

	void SetVetDetBoardRunStatus();

	void SetIOBoardRunStatus();

	bool ClearLampGroupFault(unsigned int nBoardIndex, unsigned int nChannelIndex, unsigned int nLightIndex);
	
	//��������ĺ���
    unsigned long CalcCounter(unsigned long nStart,unsigned long nEnd,unsigned long nMax); 

	//CANͨ�ŷ��͵ĵ���״̬�ֽ�ɺ�ƻƵ��̵�״̬
    void GetRYGStatusByGroup(char chGroup,char & chR,char & chY,char & chG);

	void OpenATCGetCurTime(int & nYear, int & nMon, int & nDay, int & nHour, int & nMin, int & nSec, int & nWeek);

	static COpenATCFaultProcManager * s_pData;              //������ָ��

    COpenATCParameter * m_pLogicCtlParam;                   //����������ָ��

    COpenATCRunStatus * m_pLogicCtlStatus;                  //����״̬��ָ��

	COpenATCLog       * m_pOpenATCLog;                      //��־ָ��

    TPhaseConcurInfo m_tPhaseConcurInfo;                    //������λ��Ϣ��

    TChannel m_atChannelInfo[MAX_CHANNEL_COUNT];            //ͨ����Ϣ��
    BYTE m_abyChannelNumToIndex[MAX_CHANNEL_COUNT];         //ͨ�������Ϊ�±��Ӧͨ������

    TOverlapInfo m_tOverlapInfo;                            //������λ��Ϣ��

    char m_achGreenConflictInfo[MAX_CHANNEL_COUNT][MAX_CHANNEL_COUNT];  //ͨ���̳�ͻ��Ϣ��,0��ʾ����ͬʱ���̵�,1��ʾ����ͬʱ���̵�

    char m_achGreenConflictInfoToFaultDetBoard[MAX_CHANNEL_COUNT][MAX_CHANNEL_COUNT];  //ͨ���̳�ͻ��Ϣ��,0��ʾ����ͬʱ���̵�,1��ʾ����ͬʱ���̵�
    
    char m_achLampCtlBoardStatus[C_N_MAXLAMPBOARD_NUM];     //�ƿذ�ʹ����Ϣ����,�����������ж�ȡ.
    char m_achVehDetBoardStatus[C_N_MAXDETBOARD_NUM];       //�����ʹ����Ϣ����,�����������ж�ȡ.
    char m_achIOBoardStatus[C_N_MAXIOBOARD_NUM];            //IO��ʹ����Ϣ����,�����������ж�ȡ.
    char m_achDetectorUseStatus[C_N_MAXDETBOARD_NUM * C_N_MAXDETINPUT_NUM];//������ʹ����Ϣ����

    TOpenATCFaultStatus m_tOpenATCFaultStatus;              //�����źŻ��Ĺ�����Ϣ

    bool m_bFaultDetectBoardOnLineStatus;//�жϹ��ϰ��Ƿ�����

    bool m_bLampCtlBoardOnlineStatus[C_N_MAXLAMPBOARD_NUM];//�жϵƿذ��Ƿ�����

    bool m_bNoRedOn;//�����ж��޺���������

    bool m_bFaultDetOffLine;//�����жϹ��ϰ���߹���

    bool m_bLampBoardOffLine[C_N_MAXLAMPBOARD_NUM];//�����жϵƿذ���߹���

    bool m_bGreenAndRedOn[C_N_MAXLAMPBOARD_NUM][C_N_CHANNELNUM_PER_BOARD];//�����жϺ���ͬ������

    bool m_bGreenConflict[MAX_CHANNEL_COUNT][MAX_CHANNEL_COUNT];//�����ж��̳�ͻ����

	TAscFault m_pTAscFault;//�źŻ�����

    int       m_nLampNumFault[C_N_MAXLAMPBOARD_NUM];//�ƿذ��������ϴ���

    int       m_nLampGreenAndRedOnFault[C_N_MAXLAMPBOARD_NUM][C_N_CHANNELNUM_PER_BOARD];//�ƿذ����ͬ�����ϴ��� 

    int       m_nGreenConflictFaultByChannel[MAX_CHANNEL_COUNT][MAX_CHANNEL_COUNT];//�ƿذ��̳�ͻ���ϴ��� 

    int       m_nGreenConflictFaultByPhase[MAX_PHASE_COUNT][MAX_PHASE_COUNT];//�ƿذ��̳�ͻ���ϴ���

    int       m_nVehDetNumFault[C_N_MAXDETBOARD_NUM];//������������� ����

	bool      m_bOldVehDetectorFaultStatus[C_N_MAXDETBOARD_NUM][C_N_MAXDETINPUT_NUM];//���������һ�γ������������״̬

    int       m_nVehDetectorFault[C_N_MAXDETBOARD_NUM][C_N_MAXDETINPUT_NUM];//������������ϴ��� 

    int       m_nIONumFault[C_N_MAXIOBOARD_NUM];//IO���������� ����

	bool      m_bOldLampGroupFaultStatus[C_N_MAXLAMPBOARD_NUM][C_N_CHANNELNUM_PER_BOARD][C_N_CHANNEL_OUTPUTNUM];//���������һ�εƿذ�������״̬

    unsigned int  m_nLampGroupFault[C_N_MAXLAMPBOARD_NUM][C_N_CHANNELNUM_PER_BOARD][C_N_CHANNEL_OUTPUTNUM];//�ƿذ������ϴ���

    int       m_nLampBoardUseMaxIndex;

	char             m_achOldGreenConflictInfoToFaultDetBoard[MAX_CHANNEL_COUNT][MAX_CHANNEL_COUNT];  //ͨ���̳�ͻ��Ϣ��,0��ʾ����ͬʱ���̵�,1��ʾ����ͬʱ���̵�

	TChannel         m_atOldChannelInfo[MAX_CHANNEL_COUNT]; //���������һ�ε�ͨ������

	TVehicleDetector m_atOldVehDetector[MAX_VEHICLEDETECTOR_COUNT];//���������һ�εļ��������

	bool             m_bLampBoardCtrParamChg[C_N_MAXLAMPBOARD_NUM][C_N_CHANNELNUM_PER_BOARD];//�ƿذ����Ĳ����Ƿ����仯

	bool             m_bDetectorChg[C_N_MAXDETBOARD_NUM][C_N_MAXDETINPUT_NUM];//��������������Ƿ����仯

	bool             m_bDeviceParamOtherInfoFaultFlagChg;//��siteid����豸��Ϣ�쳣�Ƿ����仯

private:
	int  Start(float fSaveFps = 0.01);

	int  Join();

	int  Detach();

	void Run();

	static void *OPENATCFAULTLOG_CALLBACK RunThread(void *pParam);

	void OpenATCSleep(long nMsec);

	void SaveOneFaultMessage(TAscFault *pTAscFault);

	bool IsConflictTableChanged();

    std::mutex m_hMutex;          // ���� g_OpenATCFaultLog ȫ�ֱ���

	OPENATCFAULTLOGHANDLE   m_hThread;

	unsigned long           m_dwThreadRet;

	bool                    m_bExitFlag;

	int                     m_nDetachState;

	bool                    m_bIsExit;				//�Ƿ����¹�������б�־

	float                   m_fSaveFps;				//ÿ���ӹ��ϱ���Ƶ��

};

#endif // !ifndef OPENATCFAULTPROCMANAGER_H
