/*=====================================================================
ģ���� ���߼����ƹ���ģ��
�ļ��� ��OpenATCLogicCtlManager.h
����ļ���OpenATCParameter.h OpenATCRunStatus.h
ʵ�ֹ��ܣ��߼�����ģ����������࣬��������ʱ����ƣ����������������п��ơ�
���� ��������
��Ȩ ��<Copyright(c) 2019-2020 Suzhou Keda Technology Co.,Ltd. All right reserved.>
------------------------------------------------------------------------
�޸ļ�¼��
�� ��           �汾     �޸���     �߶���     �޸ļ�¼
2019/9/26       V1.0     ������     �� ��      ����ģ��
=====================================================================*/

#ifndef OPENATCLOGICCTLMANAGER_H
#define OPENATCLOGICCTLMANAGER_H

#include "../Include/OpenATCParameter.h"
#include "../Include/OpenATCRunStatus.h"
#include "../Include/OpenATCRunStatusDefine.h"
#include "../Include/OpenATCLog.h"
#include "./logicctl/LogicCtlMode.h"
#include "OpenATCOperationRecord.h"

/*=====================================================================
���� ��COpenATCLogicCtlManager
���� ���߼����Ƶ����࣬��������ʱ����ƣ����������������п���
��Ҫ�ӿڣ�
��ע ��
-----------------------------------------------------------------------
�޸ļ�¼��
�� ��          �汾     �޸���     �߶���       �޸ļ�¼
2019/09/14     V1.0     ������     ����         ������
=====================================================================*/
class COpenATCLogicCtlManager  
{
public:
    //�ඨ��Ϊ����
    static COpenATCLogicCtlManager * getInstance();

    //��ĳ�ʼ������
    void Init(COpenATCParameter * pParameter,COpenATCRunStatus * pRunStatus, COpenATCLog * pOpenATCLog);

    //���������
    void Work();

    //���ֹͣ���ͷ�
    void Stop();

private:
	COpenATCLogicCtlManager();
	~COpenATCLogicCtlManager();

    typedef enum tagStartUpSequenceTimeVal
    {
        TIME_STARTUP_FLASH = 10,
        TIME_STARTUP_RED = 5,
    }TStartUpSequenceTimeVal;

    //����ʱ�����
    void StartUpTimeSequenceCtl();

    //��������                  
    void StartUpFlashCtl(); 

    //����ȫ��                        
    void StartUpAllRedCtl();  

    //����ʱ�������Ŀ���
    void AfterStartUpTimeSequenceCtl();

    //׼����ǰʱ��ε���������                
    void PrepareParam();    

	//�����û���Ԥ�Ŀ��Ʒ�ʽ�ͷ�����׼������
    void PrepareParamForSystemAsk(int nCtlMode,int nPlanNo); 

    //������ǰ���ϵȼ���������Ӧ�Ĵ���
    int ProcFault();  

    //��������ʱ��
    void ProcLampClrRunCounter();  

	//�������������仯ʱ����,�����жϵƿذ�ʹ���Ƿ����仯
    bool IsLampCtlBoardChg();   

    //���ع�������
    void CriticalFaultRun(); 

    //�������к��û���������
    void SelfAndUsrCtlRun(); 

    //�����û���Ԥ����
    bool LocalUsrCtlRun();

    //ϵͳ�û���Ԥ����
    bool SystemUsrCtlRun();

	//���ȿ���
    bool PreemptCtlRun();

    //��������
    void SelfRun();

    //ϵͳ��Ԥ��������
    void SystemAskPlanRun();

	//���ȸ�Ԥ��������
    void PreemptAskPlanRun();
                        
    //��������״̬��Ϣ
    void ProcGlobalRunStatus(); 

	//�����ֶ�����ʱ��ʼ���ֶ����ƶ���
	void InitUsrCtlLogicObj();

	//��������ģʽ
	void CreateCtlMode(int nCtlMode, int nCtlSource);

	//������Ч�Ŀ���ָ��
	void CreateValidManualCmd(TManualCmd tManualCmd, TManualCmd & tValidManualCmd);

    //�ֶ�����������      
    void UsrCtlYellowFalsh(TManualCmd tValidManualCmd);

	 //�ֶ����ȫ�촦��      
    void UsrCtlAllRed(TManualCmd tValidManualCmd);

	 //�ֶ���岽������                    
    void UsrCtlStepForward(TManualCmd tManualCmd, TManualCmd & tValidManualCmd);

    //�ֶ���巽�������      
    void UsrCtlDirectionKey(TManualCmd tManualCmd, TManualCmd & tValidManualCmd);

    //�ж���尴ť�ܷ�ʹ��
    bool CheckPanelBtnUseStatus(TManualCmd tManualCmd, TManualCmd tValidManualCmd);

	//�ж��������ָ���ܷ�ʹ��
    bool CheckSystemCmdUseStatus(TManualCmd tManualCmd, TManualCmd tValidManualCmd);

	//�ж����ȿ���ָ���ܷ�ʹ��
    bool CheckPreemptCmdUseStatus(TPreemptCtlCmd tPreemptCtlCmd);

    //���ð�ť�ظ�״̬
    void SetPanelBtnStatusReply(TManualCmd tValidManualCmd, int nHWPanelBtnIndex);

    //д������־
    void WriteOperationRecord(int nCtlSource, int nOperationType, bool bStatus, char szPeerIp[]);

	//��������ͨ����ɫ
	void ProcLockChannelLampClr();

	//����ͨ����ɫ����
	void LockChannelCtl(TAscOnePlanChannelLockInfo tAscOnePlanChannelLockInfo, int nChannelCount);

	//����ͨ���ص�
	void LockChannelOff(TAscOnePlanChannelLockInfo tAscOnePlanChannelLockInfo, int nChannelCount);

	//����ͨ���ĵ�ɫ����
	bool LockChannelTrans(TAscOnePlanChannelLockInfo tAscOnePlanChannelLockInfo, int nChannelIndex, bool bLockChannelOff, TLampClrStatus & tLampClrStatus);

	//��ȡ����ʱ��ε�����ͨ������
	int  GetAllLockChannelCount(TAscOnePlanChannelLockInfo tAscOnePlanChannelLockInfo[], int nChannelCount);

	//�жϵ�ǰʱ���Ƿ���ʱ�����
	bool IsTimeInSpan(int nCurHour, int nCurMin, int nCurSec, int nStartHour, int nStartMin, int nStartSec, int nEndHour, int nEndMin, int nEndSec);

	//��ȡʱ���(����)
	int  GetDiffTime(int nStartHour, int nStartMin, int nStartSec, int nEndHour, int nEndMin, int nEndSec);

	//��������ĺ���
	unsigned long CalcCounter(unsigned long nStart, unsigned long nEnd, unsigned long nMax);  

	//��ȡ��ǰʱ��
	void OpenATCGetCurTime(int & nYear, int & nMon, int & nDay, int & nHour, int & nMin, int & nSec, int & nWeek);

	//����������ȡ��尴ť���
	int GetManualBtnByIndex(int nIndex);

	//���ý��������׶�
	void SetAllRedStage();

	//����ϵͳ��������״̬
	void SetSystemControlStatus(bool bSpecicalControlResult, int nSpecicalControlFailCode, bool bPatternControlResult, int nPatternControlFailCode,
		bool bStageControlResult, int nStageControlFailCode, bool bPhaseControlResult, int nPhaseControlFailCode,
		bool bChannelLockResult, int nChannelLockFailCode);

	//�������ȿ�������״̬
	void SetPreemptControlStatus(bool bPreemptControlResult, int nPreemptControlFailCode);

	//����ͨ�������е��������
	void LockChannelTransToDirection(TAscOnePlanChannelLockInfo tAscOnePlanChannelLockInfo, int nChannelCount);

	void SwitchManualControlPatternToSelf();

	void SetPreemptStageIndex(BYTE byPreemptPhaseID, BYTE & byStageIndexTarget);

	void CreatePatternInterruptCmdInPreemptControl(TManualCmd  tManualCmd);
    
#ifdef VIRTUAL_DEVICE
    //��������ʱ����ȫ�ּ����������ǰ��ʱ��//Virtual_Test2022
    void GetVirtualTimeByGlobalCount(int& nYear, int& nMon, int& nDay, int& nHour, int& nMin, int& nSec, int& nWeek);
#endif // VIRTUAL_DEVICE
//Virtual_Test2022

private:
    static COpenATCLogicCtlManager * s_pData;								//������ָ��

    COpenATCParameter * m_pLogicCtlParam;									//����������ָ��

    COpenATCRunStatus * m_pLogicCtlStatus;									//����״̬��ָ��

	COpenATCLog       * m_pOpenATCLog;										//��־��ָ��

    int m_nLogicCtlStage;													//��ǰ���ƽ׶Ρ�1��ʾ����������2��ʾ����ȫ�죬3��ʾ��������

    int m_nCtlSource;														//��Ԥָ����Դ

    int m_nCurPlanNo;														//��ǰ������
    int m_nCurCtlMode;														//��ǰ�Ŀ��Ʒ�ʽ    
    bool m_bIsCtlModeChg;													//���Ʒ�ʽ�Ƿ����仯

    bool m_bFirstInitFlag;													//���ڱ�ʾ�Ƿ��һ�γ�ʼ������

    CLogicCtlMode *		 m_pLogicCtlMode;									//���Ʒ�ʽʵ����ָ��   

    TChannel             m_atOldChannelInfo[MAX_CHANNEL_COUNT];				//���������ͨ����Ϣ

    COpenATCOperationRecord m_tOpenATCOperationRecord;  					//������¼����

    TAscNetCard             m_atNetConfig[MAX_NETCARD_TABLE_COUNT];			//����������Ϣ

	TAscStartSequenceInfo   m_tAscStartSequenceInfo;						//����ʱ��

	int                     m_nLockChannelTransStatus[MAX_CHANNEL_COUNT];   //���ڼ�¼����ͨ���Ĺ���״̬

	int                     m_nLockChannelCounter[MAX_CHANNEL_COUNT];		//��������ͨ����ɫ����ʱ��

	TAscOnePlanChannelLockInfo  m_tOldAscOnePlanChannelLockInfo;			//���ڻ������һ�ε�����ͨ��ʱ���

	TAscChannelVerifyInfo   m_tOldChannelCheckInfo;                         //���ڻ������һ�ε�ͨ���������

	TManualCmd              m_tOldValidManualCmd;                           //���ڻ������һ����Ч���ֶ���������

	long					m_nManualControlPatternStartTime;               //���ڼ�¼�ֶ����Ʒ����Ŀ�ʼʱ��

	long					m_nManualControlPatternDurationTime;            //���ڼ�¼�ֶ����Ʒ����ĳ���ʱ��

	TRunStageInfo           m_tRunStageTable;                               //�׶α�

	TPreemptCtlCmd          m_tOldPreemptCtlCmd;                            //���ڻ������һ�ε����ȿ�������

	bool                    m_bInvalidPhaseCmd;                             //������λ��Ӧ��ͬ��ͨ����ָ���Ч�����Ƿ���ƽ̨����λ����״̬Ҫ�ú��淢����λ

	TPhaseLockPara          m_tInvalidPhaseLockPara;                        //������λ��Ӧ��ͬ��ͨ��������Ч��ָ��

	time_t                  m_tPatternInterruptCmdTime;                     //������Ԥָ���·�ʱ��

};

#endif // !ifndef OPENATCLOGICCTLMANAGER_H
