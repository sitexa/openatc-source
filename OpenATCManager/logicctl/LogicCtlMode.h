/*=====================================================================
ģ���� �����Ʒ�ʽʵ��ģ��
�ļ��� ��LogicCtlMode.h
����ļ���
ʵ�ֹ��ܣ����ڶ������ģʽʵ�ֽӿ�
���� ��������
��Ȩ ��<Copyright(c) 2019-2020 Suzhou Keda Technology Co.,Ltd. All right reserved.>
-----------------------------------------------------------------------
�޸ļ�¼��
�� ��           �汾     �޸���     �߶���     �޸ļ�¼
2019/9/25       V1.0     ������     �� ��      ����ģ��
=====================================================================*/

#ifndef LOGICCTLMODE_H
#define LOGICCTLMODE_H

#include "../../Include/OpenATCRunStatus.h"
#include "../../Include/OpenATCParameter.h"
#include "../../Include/OpenATCLog.h"

enum
{
	DIRECTION_TRAN_NOTEND               = 0,
	DIRECTION_SWITCH_TO_DIRECTION       = 1,
	DIRECTION_SWITCH_TO_AUTOCTL	        = 2,
	DIRECTION_END_RETURN_TO_STEPFORWARD = 3,
	DIRECTION_END_RETUEN_TO_AUTOCTL	    = 4,
};


/*=====================================================================
���� ��CLogicCtlMode
���� �����Ʒ�ʽʵ�ֽӿ�
��Ҫ�ӿڣ�Init
          Run
          Release
��ע ��
-----------------------------------------------------------------------
�޸ļ�¼��
�� ��          �汾     �޸���     �߶���       �޸ļ�¼
2019/09/14     V1.0     ������     ������       ������
2020/1/25      V1.0     ����Ƽ     ����Ƽ       �����ֶ���尴ť�л�ʱ������ͻ�ȡ���������ӿ�
2020/2/25      V1.0     ����Ƽ     ����Ƽ       �����жϷ������ʱ���Ƿ�����ĺ����͵�ǰ�׶ζ�Ӧ����λ�Ƿ����н����ĺ���
=====================================================================*/
class CLogicCtlMode  
{
public:
	CLogicCtlMode();
	virtual ~CLogicCtlMode();

    //��ʼ�����Ʒ�ʽ�ڲ�����,�����˹���Ԥ����ʱʹ��
    virtual void Init(COpenATCParameter * pParameter, COpenATCRunStatus * pRunStatus, COpenATCLog * pOpenATCLog, int nPlanNo);

    //���Ʒ�ʽ�����̽ӿ�
    virtual void Run() = 0;

    //���Ʒ�ʽ��Դ�ͷŽӿ�
    virtual void Release();

    //����m_atLampClr����ȫ�ֵ�ɫ״̬
    virtual void SetLampClr(TLampClrStatus & tLampClr);

    //����ȫ��ȡ����״̬
    virtual void SetGetParamFlag(bool bFlag);

    //��ȡ��ǰʹ�õĿ��Ʋ���������״̬
    virtual void * GetCurCtlParam(); 

    //��ȡ��ǰ��ɫ
    virtual void * GetCurLampClr();

    //��ȡ�׶α�
    virtual void * GetCurStageTable();

    //��ȡͨ�������ű�ģʽ
    virtual void * GetCurChannelSplitMode();

    //�����������Ʋ����Ϳ���״̬�������ֶ�����ʱ�Ĳ����̳�
    virtual void SetCtlDerivedParam(void * pParam,void * pLampClr,void * pStageTable,void * pChannelSplitMode);

    //�����û����Ʊ�־
    virtual void SetUsrCtlFlag(bool bFlag);

    //����ϵͳ�û����Ʊ�־
    virtual void SetSystemUsrCtlFlag(bool bFlag);

	//�������ȿ��Ʊ�־
    virtual void SetPreemptCtlFlag(bool bFlag);

    //��������״̬
    virtual void GetPhaseRunStatus(TPhaseRunStatus & tRunStatus);

    //������ʾ����ʾ��Ϣ״̬
    virtual void GetLedScreenShowInfo(TLedScreenShowInfo & tLedScreenShowInfo);

	//�жϵ�ǰͨ���ĵ�ɫ�Ƿ�����ɫ
    virtual bool IsChannelGreen(BYTE byChannelType, int nDirectionIndex, int nChannelLockStatus[]);

	//�����е�����֮ǰ�Ĺ�������״̬
	virtual void GetTransRunStatusBeforeLockPhase(TPhaseRunStatus & tRunStatus);

	//������������״̬
	virtual void GetLockPhaseRunStatus(TPhaseRunStatus & tRunStatus, bool bInvalidPhaseCmd, TPhaseLockPara tInvalidPhaseLockPara);

	TRunStageInfo GetRunStageTable();

protected:
    //��ʼ��ͨ������
    virtual void InitChannelParam();

    //���õ���ͨ���ĵ�ɫ���״̬
    void SetOneChannelOutput(char * pStart,char chStage);

	//��������ĺ���
	unsigned long CalcCounter(unsigned long nStart, unsigned long nEnd, unsigned long nMax);  

	//��ȡ��ǰʱ��
	void OpenATCGetCurTime(int & nYear, int & nMon, int & nDay, int & nHour, int & nMin, int & nSec, int & nWeek);

#ifdef VIRTUAL_DEVICE
    //��������ʱ����ȫ�ּ����������ǰ��ʱ��//Virtual_Test2022
    void GetVirtualTimeByGlobalCount(int& nYear, int& nMon, int& nDay, int& nHour, int& nMin, int& nSec, int& nWeek);
#endif // VIRTUAL_DEVICE
//Virtual_Test2022

    COpenATCRunStatus * m_pOpenATCRunStatus;
    COpenATCParameter * m_pOpenATCParameter;
	COpenATCLog       * m_pOpenATCLog;

    char m_achLampClr[C_N_MAXLAMPOUTPUT_NUM];                                   //�ƿذ���������״̬��Ϣ����

    bool m_bIsLampClrChg;                                                       //����ɫ�Ƿ����仯

    bool m_bIsUsrCtl;                                                           //�Ƿ����û���Ԥ 

    bool m_bIsSystemCtl;                                                        //�Ƿ���ƽ̨��Ԥ

	bool m_bIsPreemptCtl;                                                       //�Ƿ������ȿ���

    int m_nChannelCount;                                                        //ͨ������

    TChannel m_atChannelInfo[MAX_CHANNEL_COUNT];                                //ͨ������

    TRunStageInfo m_tRunStageInfo;                                              //�׶α�

    int m_nChannelSplitMode[MAX_CHANNEL_COUNT];                                 //ͨ����Ӧ�����ű�ģʽ

	bool m_bKeepGreenChannelBeforeControlChannelFlag[MAX_CHANNEL_COUNT];        //����ͨ����ʼǰ����������ɫ��ͨ��

    bool m_bShieldStatus[MAX_CHANNEL_COUNT];                                    //ͨ��������״̬

    bool m_bProhibitStatus[MAX_CHANNEL_COUNT];                                  //ͨ���Ľ�ֹ״̬

    bool m_bOldShieldStatus[MAX_CHANNEL_COUNT];                                 //ͨ������ʷ����״̬

    bool m_bOldProhibitStatus[MAX_CHANNEL_COUNT];                               //ͨ������ʷ��ֹ״̬

    int m_nCurRunMode;                                                          //��ǰ�źŻ�������ģʽ
};

#endif //ifndef LOGICCTLMODE_H
