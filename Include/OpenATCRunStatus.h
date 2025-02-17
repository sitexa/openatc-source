/*=====================================================================
ģ���� ��ϵͳ����״̬��¼ģ��
�ļ��� ��OpenATCRunStatus.h
����ļ���
ʵ�ֹ��ܣ����ڼ�¼��������ģ�������״̬
���� ��������
��Ȩ ��<Copyright(c) 2019-2020 Suzhou Keda Technology Co.,Ltd. All right reserved.>
-----------------------------------------------------------------------
�޸ļ�¼��
�� ��           �汾     �޸���     �߶���     �޸ļ�¼
2019/9/14       V1.0     ������     ������     ����ģ��
=====================================================================*/

#ifndef OPENATCRUNSTATUS_H
#define OPENATCRUNSTATUS_H

#include "OneWayQueue.h"
#include "../Include/OpenATCRunStatusDefine.h"
#include "../Include/OpenATCParamStructDefine.h"
#include <stdio.h>
#include <string.h>
#include <list>

/*=====================================================================
���� ��COpenATCRunStatus
���� �����ڼ�¼�����������ʱ�ĸ���״̬�������˰忨״̬���忨ʵʱͨ�����ݣ�������������״̬�ȸ������ݡ�
��Ҫ�ӿڣ�
��ע ��
-----------------------------------------------------------------------
�޸ļ�¼��
�� ��          �汾     �޸���     �߶���       �޸ļ�¼
2019/09/14     V1.0     ������     ������       ������
2019/12/18     V1.0     ����Ƽ     ����Ƽ       ��ӹ��϶��к��źŻ�������Ϣ
=====================================================================*/

#ifdef _WIN32
    #ifdef OpenATCGlobalStatus_EXPORTS
    class _declspec(dllexport) COpenATCRunStatus
    #else
    class _declspec(dllimport) COpenATCRunStatus
    #endif
#else
    class COpenATCRunStatus
#endif
{
public:
	COpenATCRunStatus();
    virtual ~COpenATCRunStatus();

    //��ʼ��״̬����
    void Init();

    //������ذ�����״̬��Ϣ
    inline void GetMainCtlBoardRunStatus(TMainCtlBoardRunStatus & tRunStatus)
    {
        memcpy(&tRunStatus,&m_tMainCtlBoardRunStatus,sizeof(TMainCtlBoardRunStatus));
    }
    //�������ذ�����״̬��Ϣ
    inline void SetMainCtlBoardRunStatus(const TMainCtlBoardRunStatus & tRunStatus)
    {
        memcpy(&m_tMainCtlBoardRunStatus,&tRunStatus,sizeof(TMainCtlBoardRunStatus));
    }

    //���������������״̬��Ϣ
    inline void GetParamRunStatus(TParamRunStatus & tRunStatus)
    {
        memcpy(&tRunStatus,&m_tParamRunStatus,sizeof(TParamRunStatus));
    }
    //����������������״̬��Ϣ
    inline void SetParamRunStatus(const TParamRunStatus & tRunStatus)
    {
        memcpy(&m_tParamRunStatus,&tRunStatus,sizeof(TParamRunStatus));
    }

    //��õ�ɫ����״̬��Ϣ
    inline void GetLampClrStatus(TLampClrStatus & tLampClrStatus)
    {
        memcpy(&tLampClrStatus,&m_tAllLampClrStatus,sizeof(TLampClrStatus));
    }
    //���õ�ɫ����״̬��Ϣ
    inline void SetLampClrStatus(const TLampClrStatus & tLampClrStatus)
    {
        memcpy(&m_tAllLampClrStatus,&tLampClrStatus,sizeof(TLampClrStatus));
    }

    //����߼���������״̬��Ϣ
    inline void GetLogicCtlStatus(TLogicCtlStatus & tRunStatus)
    {
        memcpy(&tRunStatus,&m_tLogicCtlStatus,sizeof(TLogicCtlStatus));
    }
    //�����߼���������״̬��Ϣ
    inline void SetLogicCtlStatus(const TLogicCtlStatus & tRunStatus)
    {
        memcpy(&m_tLogicCtlStatus,&tRunStatus,sizeof(TLogicCtlStatus));
    }

    //��õƿذ�������
    inline void GetLampCtlBoardData(TLampCltBoardData & tLampCtlBoardInfo)
    {
        memcpy(&tLampCtlBoardInfo,&m_tLampCtlBoardData,sizeof(TLampCltBoardData));
    }
    //���õƿذ�������
    inline void SetLampCtlBoardData(const TLampCltBoardData & tLampCtlBoardInfo)
    {
        memcpy(&m_tLampCtlBoardData,&tLampCtlBoardInfo,sizeof(TLampCltBoardData));
    }
 
    //��ȡ�ƿع���״̬
    inline void GetLampFault(TLampFaultType & tLampFault)
    {
        memcpy(&tLampFault,&m_tSysFault.m_tLampFault,sizeof(TLampFaultType));        
    }
    //���õƿع���״̬
    inline void SetLampFault(const TLampFaultType & tLampFault)
    {
        memcpy(&m_tSysFault.m_tLampFault,&tLampFault,sizeof(TLampFaultType));        
    }

    //��ȡ���������
    inline void GetVehDetBoardData(TVehDetBoardData & tVehDetData)
    {
        memcpy(&tVehDetData,&m_tVehDetBoardData,sizeof(TVehDetBoardData));        
    }
    //���ó��������
    inline void SetVehDetBoardData(const TVehDetBoardData & tVehDetData)
    {
        memcpy(&m_tVehDetBoardData,&tVehDetData,sizeof(TVehDetBoardData));        
    }

    //��ȡʵʱ��������
    inline void GetRTVehDetData(TRealTimeVehDetData & tVehDetData)
    {
        memcpy(&tVehDetData,&m_tRealTimeDetData,sizeof(TRealTimeVehDetData));        
    }
    //����ʵʱ��������
    inline void SetRTVehDetData(const TRealTimeVehDetData & tVehDetData)
    {
        memcpy(&m_tRealTimeDetData,&tVehDetData,sizeof(TRealTimeVehDetData));        
    }

    //��ȡȫ�ּ�����
    inline unsigned long GetGlobalCounter()
    {
        return m_nGlobalCounter;
    }
    //����ȫ�ּ�����
    inline void SetGlobalCounter(unsigned long nCounter)
    {
        m_nGlobalCounter = nCounter;
    }

    //��ȡ��ɫ������
    inline unsigned long GetLampClrCounter()
    {
        return m_nLampClrCounter;
    }
    //���õ�ɫ������
    inline void SetLampClrCounter(unsigned long nCounter)
    {
        m_nLampClrCounter = nCounter;
    }

    //��ȡ��ɫ��ʱ��Ϣ
    inline void GetPhaseLampClrRunCounter(TPhaseLampClrRunCounter & tRunCounter)
    {
        memcpy(&tRunCounter,&m_tPhaseLampClrRunCounter,sizeof(TPhaseLampClrRunCounter));        
    }
    //���õ�ɫ��ʱ��Ϣ
    inline void SetPhaseLampClrRunCounter(const TPhaseLampClrRunCounter & tRunCounter)
    {
        memcpy(&m_tPhaseLampClrRunCounter,&tRunCounter,sizeof(TPhaseLampClrRunCounter));        
    }

    //��ȡ�忨ʹ��״̬
    inline void GetAllBoardUseStatus(TAllBoardUseStatus & tUseStatus)
    {
        memcpy(&tUseStatus,&m_tAllBoardUseStatus,sizeof(TAllBoardUseStatus));        
    }
    //���ð忨ʹ��״̬
    inline void SetAllBoardUseStatus(const TAllBoardUseStatus & tUseStatus)
    {
        memcpy(&m_tAllBoardUseStatus,&tUseStatus,sizeof(TAllBoardUseStatus));        
    }

    //��ȡ�忨����״̬
    inline void GetAllBoardOnlineStatus(TAllBoardOnlineStatus & tOnlineStatus)
    {
        memcpy(&tOnlineStatus,&m_tAllBoardOnlineStatus,sizeof(TAllBoardOnlineStatus));        
    }
    //���ð忨����״̬
    inline void SetAllBoardOnlineStatus(const TAllBoardOnlineStatus & tOnlineStatus)
    {
        memcpy(&m_tAllBoardOnlineStatus,&tOnlineStatus,sizeof(TAllBoardOnlineStatus));        
    }

    //��ȡ��λ����״̬д���
    inline bool GetPhaseRunStatusWriteFlag()
    {
        return m_bPhaseRunStatusWriteFlag;
    }
    //������λ����״̬д���
    inline void SetPhaseRunStatusWriteFlag(bool bFlag)
    {
        m_bPhaseRunStatusWriteFlag = bFlag;
    }    

    //��ȡ��λ����״̬�����
    inline bool GetPhaseRunStatusReadFlag()
    {
        return m_bPhaseRunStatusReadFlag;
    }
    //������λ����״̬�����
    inline void SetPhaseRunStatusReadFlag(bool bFlag)
    {
        m_bPhaseRunStatusReadFlag = bFlag;
    } 

    //��ȡ��λ����״̬
    inline void GetPhaseRunStatus(TPhaseRunStatus & tRunStatus)
    {
        memcpy(&tRunStatus,&m_tPhaseRunStatus,sizeof(TPhaseRunStatus));        
    }
    //������λ����״̬
    inline void SetPhaseRunStatus(const TPhaseRunStatus & tRunStatus)
    {
        memcpy(&m_tPhaseRunStatus,&tRunStatus,sizeof(TPhaseRunStatus));        
    }

    //��ȡIO��ͨ������
    inline void GetIOBoardData(TIOBoardData & tBoardData)
    {
        memcpy(&tBoardData,&m_tIOBoardData,sizeof(TIOBoardData));        
    }
    //����IO��ͨ������
    inline void SetIOBoardData(const TIOBoardData & tBoardData)
    {
        memcpy(&m_tIOBoardData,&tBoardData,sizeof(TIOBoardData));        
    }

    //��ȡ���ϼ���ͨ������
    inline void GetFaultDetBoardData(TFaultDetBoardData & tBoardData)
    {
        memcpy(&tBoardData,&m_tFaultDetBoardData,sizeof(TFaultDetBoardData));        
    }
    //���ù��ϼ���ͨ������
    inline void SetFaultDetBoardData(const TFaultDetBoardData & tBoardData)
    {
        memcpy(&m_tFaultDetBoardData,&tBoardData,sizeof(TFaultDetBoardData));        
    }

    //��ȡ�Լ�״̬
    inline bool GetSelfDetectStatus()
    {
        return m_bSelfDetect;
    }
    //�����Լ�״̬
    inline void SetSelfDetectStatus(bool bStatus)
    {
        m_bSelfDetect = bStatus;
    }

    //��ȡ�Ƿ�ֹͣ����
    inline bool GetStopWorkStatus()
    {
        return m_bIsStopWork;
    }
    //�����Ƿ�ֹͣ����
    inline void SetStopWorkStatus(bool bStatus)
    {
        m_bIsStopWork = bStatus;
    }

    //��ȡ����״̬
    inline bool GetRebootStatus()
    {
        return m_bIsRebootATC;
    }
    //��������״̬
    inline void SetRebootStatus(bool bStatus)
    {
        m_bIsRebootATC = bStatus;
    }

    //���͹��ϵ����϶���
    inline bool PushFaultToQueue(TAscFault tAscFault)
    {
        return m_FaultQueueToCenter.Push(tAscFault);
    }

    //�ӹ��϶��л�ȡ��������
    inline bool PopFaultFromQueue(TAscFault & tAscFault)
    {
        return m_FaultQueueToCenter.Pop(tAscFault);
    }

	//��ȡ�߼�����һ�������Ƿ������״̬
    inline bool GetCycleChgStatus()
    {
        return m_bIsCycleChgForFault;
    }
    //�����߼�����һ�������Ƿ������״̬
    inline void SetCycleChgStatus(bool bStatus)
    {
        m_bIsCycleChgForFault = bStatus;
    }

	//��ȡ��������
    inline int GetGreenFlashCount(int nIndex)
    {
        return m_nGreenFlashCount[nIndex];
    }
    //������������
    inline void SetGreenFlashCount(int nIndex, int nCount)
    {
        m_nGreenFlashCount[nIndex] = nCount;
    }
	
	//��ȡ��������
    inline int GetRedFlashCount(int nIndex)
    {
        return m_nRedFlashCount[nIndex];
    }
    //���ú�������
    inline void SetRedFlashCount(int nIndex, int nCount)
    {
        m_nRedFlashCount[nIndex] = nCount;
    }

	// ����U�̹��ر�־
	inline void SetUSBMountFlag(bool bFlag)
	{
		m_bIsUSBMounted = bFlag;
	}
	
	//��ȡU�̹��ر�־
	inline bool GetUSBMountFlag()
	{
		return m_bIsUSBMounted;
	}

	//����can1����ָʾ��״̬
	inline void SetCAN1LedStatus(bool bFlag)
	{
		m_bCAN1LedStatus = bFlag;
	}

	//��ȡcan1����ָʾ��״̬
	inline bool GetCAN1LedStatus()
	{
		return m_bCAN1LedStatus;
	}

	//����can2����ָʾ��״̬
	inline void SetCAN2LedStatus(bool bFlag)
	{
		m_bCAN2LedStatus = bFlag;
	}
	
	// ��ȡcan2����ָʾ��״̬
	inline bool GetCAN2LedStatus()
	{
		return m_bCAN2LedStatus;
	}

	//����GPSָʾ��״̬
	inline void SetGPSLedStatus(bool bFlag)
	{
		m_bGPSLedStatus = bFlag;
	}
	
	// ��ȡGPSָʾ��״̬
	inline bool GetGPSLedStatus()
	{
		return m_bGPSLedStatus;
	}

	//����ERRָʾ��״̬
	inline void SetErrLedStatus(bool bFlag)
	{
		m_bErrLedStatus = bFlag;
	}
	
	//��ȡERRָʾ��״̬
	inline bool GetErrLedStatus()
	{
		return m_bErrLedStatus;
	}

	//������Ҫ�洢���ֶ���尴ť��Ӧ״̬����
	inline void SetPanelBtnStatusList(THWPanelBtnStatus tStatus)
	{
		m_HWPanelBtnStatusList.push_back(tStatus);
	}

	//��ȡ��Ҫ�洢���ֶ���尴ť��Ӧ״̬����
	inline THWPanelBtnStatus GetPanelBtnStatusListElement()
	{
		THWPanelBtnStatus tStatus;
		memset(&tStatus, 0, sizeof(tStatus));
		if (m_HWPanelBtnStatusList.size() > 0)
		{
			tStatus = (THWPanelBtnStatus)m_HWPanelBtnStatusList.front();
			m_HWPanelBtnStatusList.pop_front();
		}
		return tStatus;
	}

    //������Ҫ�洢��GPS���ݶ���
	inline void SetGpsDataList(TGpsData tData)
	{
		m_tGpsDataList.push_back(tData);
	}

	//��ȡ��Ҫ�洢��GPS���ݶ���
	inline TGpsData GetGpsDataListElement()
	{
		TGpsData tData;
		memset(&tData, 0, sizeof(tData));
		if (m_tGpsDataList.size() > 0)
		{
			tData = (TGpsData)m_tGpsDataList.front();
			m_tGpsDataList.pop_front();
		}
		return tData;
	}

    //�������������豸״̬����
	inline void SetOpenATCStatusInfo(TOpenATCStatusInfo tOpenATCStatusInfo)
	{
		memcpy(&m_tOpenATCStatusInfo, &tOpenATCStatusInfo, sizeof(TOpenATCStatusInfo)); 
	}
	
	//��ȡ���������豸״̬����
	inline void GetOpenATCStatusInfo(TOpenATCStatusInfo & tOpenATCStatusInfo)
	{
		memcpy(&tOpenATCStatusInfo, &m_tOpenATCStatusInfo, sizeof(TOpenATCStatusInfo)); 
    }
	
	//��ȡ��ʾ����ʾ����Ϣ
    inline void GetLedScreenShowInfo(TLedScreenShowInfo & tInfo)
    {
        memcpy(&tInfo,&m_tLedScreenShowInfo,sizeof(TLedScreenShowInfo));        
    }
    //������ʾ����ʾ����Ϣ
    inline void SetLedScreenShowInfo(const TLedScreenShowInfo & tInfo)
    {
        memcpy(&m_tLedScreenShowInfo,&tInfo,sizeof(TLedScreenShowInfo));        
    }

    //���õ�ַ����Ϣ
    inline void SetSiteRev(int nArray[])
    {
        for (int i = 0;i < sizeof(m_tOpenATCStatusInfo.m_tWholeDevStatusInfo.m_cHardwareVer);i++)
        {
            m_tOpenATCStatusInfo.m_tWholeDevStatusInfo.m_cHardwareVer[i] = nArray[i];
        }
    }

    //��ȡ�ƿذ����״̬
    inline bool GetLampCtlBoardOffLine(int nBoardIndex)
    {
        return m_bLampCtlBoardOffLine[nBoardIndex];  
    }
    //���õƿذ����״̬
    inline void SetLampCtlBoardOffLine(int nBoardIndex, bool bOffLine)
    {
        m_bLampCtlBoardOffLine[nBoardIndex] = bOffLine;      
    }

    //��ȡIO�����״̬
    inline bool GetIOBoardOffLine(int nBoardIndex)
    {
        return m_bIOBoardOffLine[nBoardIndex];  
    }
    //����IO�����״̬
    inline void SetIOBoardOffLine(int nBoardIndex, bool bOffLine)
    {
        m_bIOBoardOffLine[nBoardIndex] = bOffLine;      
    }
  
    //��ȡ��������״̬
    inline bool GetDetBoardOffLine(int nBoardIndex)
    {
        return m_bDetBoardOffLine[nBoardIndex];  
    }
    //���ó�������״̬
    inline void SetDetBoardOffLine(int nBoardIndex, bool bOffLine)
    {
        m_bDetBoardOffLine[nBoardIndex] = bOffLine;      
    }

    //��ȡ���ϰ����״̬
    inline bool GetFaultDetBoardOffLine()
    {
        return m_bFaultDetBoardOffLine;  
    }
    //���ù��ϰ����״̬
    inline void SetFaultDetBoardOffLine(bool bOffLine)
    {
        m_bFaultDetBoardOffLine = bOffLine;      
    }

    //��ȡͨ���̳�ͻ��Ϣ��
    inline void GetGreenConflictInfo(char chGreenConflictInfo[MAX_CHANNEL_COUNT][MAX_CHANNEL_COUNT])
    {
        memcpy(chGreenConflictInfo, m_achGreenConflictInfo, sizeof(m_achGreenConflictInfo));
    }
    //����ͨ���̳�ͻ��Ϣ��
    inline void SetGreenConflictInfo(char chGreenConflictInfo[MAX_CHANNEL_COUNT][MAX_CHANNEL_COUNT])
    {
        memcpy(m_achGreenConflictInfo, chGreenConflictInfo, sizeof(m_achGreenConflictInfo));
    }

	//��ȡͨ�������Ϣ��
	inline void GetChannelCheckInfo(TAscChannelVerifyInfo &tChannelInfo)
	{
		memcpy(&tChannelInfo, &m_tChannelVerifyInfo, sizeof(m_tChannelVerifyInfo));
	}
	//����ͨ�������Ϣ��
	inline void SetChannelCheckInfo(TAscChannelVerifyInfo &tChannelInfo)
	{
		memcpy(&m_tChannelVerifyInfo, &tChannelInfo, sizeof(tChannelInfo));
	}

	//��ȡ�Լ���Ϣ
	inline void GetSelfDetectInfo(TSelfDetectInfo &tSelfDetectInfo)
	{
		memcpy(&tSelfDetectInfo, &m_tSelfDetectInfo, sizeof(m_tSelfDetectInfo));
	}
	//�����Լ���Ϣ
	inline void SetSelfDetectInfo(TSelfDetectInfo &tSelfDetectInfo)
	{
		memcpy(&m_tSelfDetectInfo, &tSelfDetectInfo, sizeof(tSelfDetectInfo));
	}

	//��ȡJava�Ƿ�ֹͣ����
    inline time_t GetJavaHeartTime()
    {
        return m_tJavaHeartTime;
    }
    //�����Ƿ�ֹͣ����
    inline void SetJavaHeartTime(time_t javaHeartTime)
    {
        m_tJavaHeartTime = javaHeartTime;
    }

	//��ȡͨ��ʵʱ��ѹ�͵���
    inline void GetChannelStatusInfo(int nChannelIndex, TChannelStatusInfo & tChannelStatusInfo)
    {
        memcpy(&tChannelStatusInfo, &m_tChannelStatusInfo[nChannelIndex], sizeof(tChannelStatusInfo));
    }
    //����ͨ��ʵʱ��ѹ�͵���
    inline void SeChannelStatusInfo(int nChannelIndex, TChannelStatusInfo tChannelStatusInfo)
    {
        memcpy(&m_tChannelStatusInfo[nChannelIndex], &tChannelStatusInfo, sizeof(tChannelStatusInfo));
    }

	//��ȡ�����Ŷ���Ϣ
    inline void GetVehicleQueueUpInfo(TVehicleQueueUpInfo tVehicleQueueUpInfo[])
    {
        memcpy(tVehicleQueueUpInfo, m_tVehicleQueueUpInfo, sizeof(m_tVehicleQueueUpInfo));
    }
    //���ó����Ŷ���Ϣ
    inline void SetVehicleQueueUpInfo(TVehicleQueueUpInfo tVehicleQueueUpInfo[])
    {
        memcpy(m_tVehicleQueueUpInfo, tVehicleQueueUpInfo, sizeof(m_tVehicleQueueUpInfo));
    }

	//��ȡ���˼����Ϣ
	inline void GetPedDetectInfo(TPedDetectInfo tPedDetectInfo[])
	{
		memcpy(tPedDetectInfo, m_tPedDetectInfo, sizeof(m_tPedDetectInfo));
	}
	//�������˼����Ϣ
	inline void SetPedDetectInfo(TPedDetectInfo tPedDetectInfo[])
	{
		memcpy(m_tPedDetectInfo, tPedDetectInfo, sizeof(m_tPedDetectInfo));
	}

	//��ȡ���ϰ����״̬
	inline bool GeFaultDetBoardControlStatus()
	{
		return m_bFaultDetBoardControlStatus;
	}
	//���ù��ϰ����״̬
	inline void SetFaultDetBoardControlStatus(bool bFlag)
	{
		m_bFaultDetBoardControlStatus = bFlag;
	}

	//��ȡ���������ͨ��״̬
	inline void GetComStatusWithCfg(bool & bUDPComStatus, bool & bTCPComStatus)
	{
		bUDPComStatus = m_bUDPComStatusWithCfg;
		bTCPComStatus = m_bTCPComStatusWithCfg;
	}
	//�������������ͨ��״̬
	inline void SetComStatusWithCfg(bool bUDPComStatus, bool bTCPComStatus)
	{
		m_bUDPComStatusWithCfg = bUDPComStatus;
		m_bTCPComStatusWithCfg = bTCPComStatus;
	}

	//��ȡ������ʾ����ʾ��Ϣ���б��
    inline bool GetLedScreenShowInfoFlag()
    {
        return m_bLedScreenShowInfoFlag;
    }
    //������ʾ����ʾ��Ϣ���б��
    inline void SetLedScreenShowInfoFlag(bool bFlag)
    {
        m_bLedScreenShowInfoFlag = bFlag;
    }   

	//��ȡϵͳ����״̬
	inline void GetSystemControlStatus(TSystemControlStatus &tSystemControlStatus)
	{
		memcpy(&tSystemControlStatus, &m_tSystemControlStatus, sizeof(TSystemControlStatus));
	}
	//����ϵͳ����״̬
	inline void SetSystemControlStatus(TSystemControlStatus &tSystemControlStatus)
	{
		memcpy(&m_tSystemControlStatus, &tSystemControlStatus, sizeof(TSystemControlStatus));
	}

	//��ȡ�����û����ֶ�ָ��
	inline void GetManualCmd(TManualCmd &tManualCmd)
	{
		memcpy(&tManualCmd, &m_tManualCmd, sizeof(tManualCmd));
	}
	//���������û����ֶ�ָ��
	inline void SetManualCmd(TManualCmd &tManualCmd)
	{
		memcpy(&m_tManualCmd, &tManualCmd, sizeof(tManualCmd));
	}

	//��ȡ��Чָ��
	inline void GetValidManualCmd(TManualCmd &tManualCmd)
	{
		memcpy(&tManualCmd, &m_tValidManualCmd, sizeof(tManualCmd));
	}
	//������Чָ��
	inline void SetValidManualCmd(TManualCmd &tManualCmd)
	{
		memcpy(&m_tValidManualCmd, &tManualCmd, sizeof(tManualCmd));
	}
	
	//��ȡ��siteid����豸��Ϣ�쳣���
	inline bool GetDeviceParamOtherInfoFaultFlag()
	{
		return m_bIfDeviceParamOtherInfoFault;
	}
	//���ó�siteid����豸��Ϣ�쳣���
	inline void SetDeviceParamOtherInfoFaultFlag(bool bFlag)
	{
		m_bIfDeviceParamOtherInfoFault = bFlag;
	}   

	//��ȡ�û�������λ���п���״̬
	inline void GetPhasePassCmdPhaseStatus(TPhasePassCmdPhaseStatus &tPhasePassCmdPhaseStatus)
	{
		memcpy(&tPhasePassCmdPhaseStatus, &m_tPhasePassCmdPhaseStatus, sizeof(TPhasePassCmdPhaseStatus));
	}
	//�����û�������λ���п���״̬
	inline void SetPhasePassCmdPhaseStatus(TPhasePassCmdPhaseStatus tPhasePassCmdPhaseStatus)
	{
		memcpy(&m_tPhasePassCmdPhaseStatus, &tPhasePassCmdPhaseStatus, sizeof(TPhasePassCmdPhaseStatus));
	}

	//��ȡ������λ���п���״̬
	inline void GetLocalPhasePassStatus(TPhasePassCmdPhaseStatus &tPhasePassCmdPhaseStatus)
	{
		memcpy(&tPhasePassCmdPhaseStatus, &m_tLocalPhasePassStatus, sizeof(TPhasePassCmdPhaseStatus));
	}
	//���ñ�����λ���п���״̬
	inline void SetLocalPhasePassStatus(TPhasePassCmdPhaseStatus tPhasePassCmdPhaseStatus)
	{
		memcpy(&m_tLocalPhasePassStatus, &tPhasePassCmdPhaseStatus, sizeof(TPhasePassCmdPhaseStatus));
	}

	//��ȡɫ��ģʽ���·��Ĺض�ָ���������ǰ���н׶ε���λ���
	inline bool GetIncludedCurPhaseInPhaseStatusCmdInStepColorFlag()
	{
		return m_bIncludedCurPhaseInPhaseStatusCmdInStepColor;
	}
	//����ɫ��ģʽ���·��Ĺض�ָ���������ǰ���н׶ε���λ���
	inline void SetIncludedCurPhaseInPhaseStatusCmdInStepColorFlag(bool bFlag)
	{
		m_bIncludedCurPhaseInPhaseStatusCmdInStepColor = bFlag;
	}

#ifdef VIRTUAL_DEVICE
    //��ȡ�����źŻ����е�����ʱ��
    inline void GetVirtualTimeData(TVirtualRunTime& tVirtualTimeData)
    {
        memcpy(&tVirtualTimeData, &m_tVirtualTime, sizeof(TVirtualRunTime));
    }
    //���������źŻ����е�����ʱ��
    inline void SetVirtualTimeData(const TVirtualRunTime& tVirtualTimeData)
    {
        memcpy(&m_tVirtualTime, &tVirtualTimeData, sizeof(TVirtualRunTime));
    }
    //��ȡ�Ƿ��������״̬
    inline bool GetIsSpeedyRunStatus()
    {
        return m_bIsSpeedyRun;
    }
    //�����Ƿ��������״̬
    inline void SetIsSpeedyRunStatus(bool bIsSpeedyRun)
    {
        m_bIsSpeedyRun = bIsSpeedyRun;
    }
#endif // VIRTUAL_DEVICE
    //Virtual_Test2022
    
    //��ȡ���ȿ���״̬
	inline void GetPreemptControlStatus(TPreemptControlStatus &tPreemptControlStatus)
	{
		memcpy(&tPreemptControlStatus, &m_tPreemptControlStatus, sizeof(TPreemptControlStatus));
	}
	//�������ȿ���״̬
	inline void SetPreemptControlStatus(TPreemptControlStatus &tPreemptControlStatus)
	{
		memcpy(&m_tPreemptControlStatus, &tPreemptControlStatus, sizeof(TPreemptControlStatus));
	}

	//��ȡ���ȿ���ָ��
	inline void GetPreemptCtlCmd(TPreemptCtlCmd &tPreemptCtlCmd)
	{
		memcpy(&tPreemptCtlCmd, &m_tPreemptCtlCmd, sizeof(tPreemptCtlCmd));
	}
	//�������ȿ���ָ��
	inline void SetPreemptCtlCmd(TPreemptCtlCmd &tPreemptCtlCmd)
	{
		memcpy(&m_tPreemptCtlCmd, &tPreemptCtlCmd, sizeof(tPreemptCtlCmd));
	}

	//������Ҫ���ȿ����������
	inline void SePreemptCtlCmdList(TPreemptCtlCmd tPreemptCtlCmd)
	{
		std::list<TPreemptCtlCmd>::iterator it;

		bool bFlag = false;

		for (it = m_tPreemptCtlCmdList.begin(); it != m_tPreemptCtlCmdList.end(); it++)
		{
			if (it->m_nCmdSource == tPreemptCtlCmd.m_nCmdSource && it->m_byPreemptStageIndex == tPreemptCtlCmd.m_byPreemptStageIndex && it->m_byPreemptSwitchFlag == 0)
			{
				bFlag = true;
				if (it->m_byPreemptPhaseID != tPreemptCtlCmd.m_byPreemptPhaseID)
				{
					it->m_bIncludeConcurPhase = true;
					it->m_byPreemptConcurPhaseID = tPreemptCtlCmd.m_byPreemptPhaseID;
				}

				if (it->m_byPreemptType == PREEMPT_TYPE_NORMAL && tPreemptCtlCmd.m_byPreemptType == PREEMPT_TYPE_URGENT)
				{
					it->m_byPreemptType = PREEMPT_TYPE_URGENT;
				}
			}
		}

		if (!bFlag)
		{
			m_tPreemptCtlCmdList.push_back(tPreemptCtlCmd);
		}
	}

	//����������λ��ʼ�л���־
	inline void ModifyPreemptSwitchFlag(int nCmdSource, bool bPatternInterruptCmd, BYTE byPreemptType, BYTE byPreemptPhaseID, bool & bIncludeConcurPhase)
	{
		std::list<TPreemptCtlCmd>::iterator it;

		bool bFlag = false;

  	    for (it = m_tPreemptCtlCmdList.begin(); it != m_tPreemptCtlCmdList.end(); it++)
		{
			if (!bPatternInterruptCmd)
			{
				if (it->m_nCmdSource == nCmdSource && it->m_byPreemptType == byPreemptType && it->m_byPreemptPhaseID == byPreemptPhaseID && it->m_byPreemptSwitchFlag == 0)
				{
					bIncludeConcurPhase = it->m_bIncludeConcurPhase;
					it->m_byPreemptSwitchFlag = 1;
					break;
				}
			}
			else
			{
				if (it->m_nCmdSource == nCmdSource && it->m_bPatternInterruptCmd && it->m_byPreemptSwitchFlag == 0)
				{
					it->m_byPreemptSwitchFlag = 1;
					break;
				}
			}
		}
	}

	//�����ȿ�����������л�ȡ���ȿ�������
	inline bool GetPreemptCtlCmdListElement(int nCmdSource, bool bPatternInterruptCmd, BYTE byPreemptType, BYTE byPreemptStageIndex, TPreemptCtlCmd & tPreemptCtlCmd)
	{
		std::list<TPreemptCtlCmd>::iterator it;

		bool bFlag = false;

  	    for (it = m_tPreemptCtlCmdList.begin(); it != m_tPreemptCtlCmdList.end(); it++)
		{
			if (!bPatternInterruptCmd)
			{
				if (it->m_nCmdSource == nCmdSource && it->m_byPreemptType == byPreemptType && it->m_byPreemptStageIndex == byPreemptStageIndex && it->m_byPreemptSwitchFlag == 0)
				{
					bFlag = true;
					tPreemptCtlCmd = *it;
					break;
				}
			}
			else
			{
				if (it->m_nCmdSource == nCmdSource && it->m_bPatternInterruptCmd && it->m_byPreemptSwitchFlag == 0)
				{
					bFlag = true;
					tPreemptCtlCmd = *it;
					break;
				}
			}
		}

		return bFlag;
	}

	//�����ȿ������������ɾ�����ȿ�������
	inline void DelteFromPreemptCtlCmdList(int nCmdSource, bool bPatternInterruptCmd, BYTE byPreemptType, BYTE byPreemptPhaseID)
	{
		std::list<TPreemptCtlCmd>::iterator it = m_tPreemptCtlCmdList.begin();

		while (it != m_tPreemptCtlCmdList.end())
		{
			if (!bPatternInterruptCmd)
			{
				if (it->m_nCmdSource == nCmdSource && it->m_byPreemptType == byPreemptType && it->m_byPreemptPhaseID == byPreemptPhaseID && it->m_byPreemptSwitchFlag == 1)
				{
					it = m_tPreemptCtlCmdList.erase(it);
					break;
				}
				else
				{
					it++;
				}
			}
			else
			{
				if (it->m_nCmdSource == nCmdSource && it->m_bPatternInterruptCmd)
				{
					if (it->m_nCmdSource != CTL_SOURCE_PREEMPT)
					{
						m_tPreemptCtlCmdList.clear();//���ػ�ϵͳ��Ԥʱ������ʣ��ָ��ȫ��ɾ��
					}
					else
					{
						it = m_tPreemptCtlCmdList.erase(it);
					}
					break;
				}
				else
				{
					it++;
				}
			}
		}
	}
	//�����ȿ������������ɾ���������ȿ�������
	inline void DelteAllPreemptCtlCmdFromList()
	{
		m_tPreemptCtlCmdList.clear();
	}
	
    //20999Ԥ��
    //��ȡ�豸״̬
    inline void GetDeviceStatus(TDeviceStatus& tDeviceStatus)
    {
        memcpy(&tDeviceStatus, &m_tDeviceStatus, sizeof(m_tDeviceStatus));
    }
    //�����豸״̬
    inline void SetDeviceStatus(TDeviceStatus tDeviceStatus)
    {
        memcpy(&m_tDeviceStatus, &tDeviceStatus, sizeof(m_tDeviceStatus));
    }

    //��ȡѡ�����ͨѶ�߳�
    inline int GetCommFlagStatus()
    {
        return CommFlag;
    }
    //����ѡ�����ͨѶ�߳�
    inline void SetCommFlagStatus(int iCommFlag)
    {
        CommFlag = iCommFlag;
    }

    inline bool GetPhaseControlChange()
    {
        return m_bPhaseControlChange;
    }
    //����ѡ�����ͨѶ�߳�
    inline void SetPhaseControlChange(bool bChange)
    {
        m_bPhaseControlChange = bChange;
    }

private:
    TParamRunStatus m_tParamRunStatus;                      //����������ǰ״̬��Ϣ

    TMainCtlBoardRunStatus m_tMainCtlBoardRunStatus;        //���ذ嵱ǰ����״̬��Ϣ

    TLampClrStatus m_tAllLampClrStatus;                     //�ƿذ��ɫ״̬��Ϣ

    TLogicCtlStatus m_tLogicCtlStatus;                      //�߼�����״̬��Ϣ   

    TLampCltBoardData m_tLampCtlBoardData;                  //�ӵƿذ�ɼ��ĵƿذ���Ϣ

  	TCommonFault m_tSysFault;                               //ȫ�ֹ���״̬,Ŀǰʹ�õƿ�״̬,���ں��߼�����ģ�齻������

    TVehDetBoardData m_tVehDetBoardData;                    //�ӳ����ɼ����ĳ�����Ϣ

    TRealTimeVehDetData m_tRealTimeDetData;                 //ʵʱ������Ϣ�����ڸ�Ӧ����

    unsigned long m_nGlobalCounter;                         //ȫ�ּ�����

    unsigned long m_nLampClrCounter;                        //��ɫ���м�����

    TPhaseLampClrRunCounter m_tPhaseLampClrRunCounter;      //��λ��ɫ���м�ʱ��Ϣ

    TAllBoardUseStatus m_tAllBoardUseStatus;                //���а忨ʹ��״̬,�Ӳ����л��

    TAllBoardOnlineStatus m_tAllBoardOnlineStatus;          //���а忨����״̬,ͨ��CANͨ�Ż�ȡ��Ϣ

    bool m_bPhaseRunStatusWriteFlag;                        //��λ����״̬д���,����ѯ״̬ʱͨ���̰߳�״̬��Ϊ��д

    bool m_bPhaseRunStatusReadFlag;                         //��λ����״̬�����,����ѭ��������״̬������Ϻ��״̬��Ϊ�ɶ�

    TPhaseRunStatus m_tPhaseRunStatus;                      //��λ����״̬,������ͨ���̴߳�����λ����״̬����

    TIOBoardData m_tIOBoardData;                            //��IO��ɼ�����IO����

    TFaultDetBoardData m_tFaultDetBoardData;                //�ӹ��ϼ���ɼ���������

	TAscChannelVerifyInfo m_tChannelVerifyInfo;				//ͨ���������

    bool m_bSelfDetect;                                     //�źŻ��Լ�״̬

    bool m_bIsStopWork;                                     //�źŻ��Ƿ��������

    bool m_bIsRebootATC;                                    //�Ƿ������źŻ�

    COneWayQueue<TAscFault>  m_FaultQueueToCenter;          //���϶���    
	
	bool m_bIsCycleChgForFault;                             //�߼������Ƿ�һ�����ڽ�����֪ͨ���ϼ��ģ��

	int  m_nGreenFlashCount[C_N_MAXLAMPOUTPUT_NUM];         //�����������

	int  m_nRedFlashCount[C_N_MAXLAMPOUTPUT_NUM];           //�����������
	
	bool m_bIsUSBMounted;									//�Ƿ��ѹ���U��
	bool m_bCAN1LedStatus;									//can1����ָʾ��״̬
	bool m_bCAN2LedStatus;									//can2����ָʾ��״̬
	bool m_bGPSLedStatus;									//GPSָʾ��״̬
	bool m_bErrLedStatus;									//ERRָʾ��״̬

	std::list<THWPanelBtnStatus> m_HWPanelBtnStatusList;    //�ֶ���尴ť��Ӧ״̬����

    TGpsData		   m_tGpsData;				            //GPS���� 
    TOpenATCStatusInfo m_tOpenATCStatusInfo;                //����ָʾ�ư������״̬����
	
    TLedScreenShowInfo m_tLedScreenShowInfo;                //Lcd��ʾ����ʾ����Ϣ
	
    std::list<TGpsData> m_tGpsDataList;                     //GPS���ݶ���

    bool m_bLampCtlBoardOffLine[C_N_MAXLAMPBOARD_NUM];      //�ƿذ����״̬

    bool m_bIOBoardOffLine[C_N_MAXIOBOARD_NUM];             //IO�����״̬

    bool m_bDetBoardOffLine[C_N_MAXDETBOARD_NUM];           //��������״̬

    bool m_bFaultDetBoardOffLine;                           //���ϰ����״̬

    char m_achGreenConflictInfo[MAX_CHANNEL_COUNT][MAX_CHANNEL_COUNT];  //ͨ���̳�ͻ��Ϣ��,0��ʾ����ͬʱ���̵�,1��ʾ����ͬʱ���̵�

	TSelfDetectInfo m_tSelfDetectInfo;						//�źŻ��Լ���Ϣ

	time_t m_tJavaHeartTime;                                //Java����������ʱ��

	TChannelStatusInfo m_tChannelStatusInfo[MAX_CHANNEL_COUNT];//ͨ��ʵʱ��ѹ�͵���

	TVehicleQueueUpInfo m_tVehicleQueueUpInfo[MAX_VEHICLEDETECTOR_COUNT];//�����Ŷ���Ϣ

	TPedDetectInfo m_tPedDetectInfo[MAX_PEDESTRIANDETECTOR_COUNT];//���˼����Ϣ

	bool m_bFaultDetBoardControlStatus;

	bool m_bUDPComStatusWithCfg;                            //�����������UDPͨ��״̬

	bool m_bTCPComStatusWithCfg;                            //�����������TCPͨ��״̬

	bool m_bLedScreenShowInfoFlag;                          //��ʾ����ʾ��Ϣ���

	TSystemControlStatus m_tSystemControlStatus;            //ϵͳ����״̬

	TManualCmd m_tManualCmd;                                //����ָ��

	TManualCmd m_tValidManualCmd;                           //��Ч�Ŀ���ָ��
	
	bool m_bIfDeviceParamOtherInfoFault;				    //��siteid����豸��Ϣ�Ƿ��쳣

	TPhasePassCmdPhaseStatus m_tPhasePassCmdPhaseStatus;	//�����ù����·�����λ����״̬��¼����

	TPhasePassCmdPhaseStatus m_tLocalPhasePassStatus;	    //�����ص���λ����״̬��¼����

	bool m_bIncludedCurPhaseInPhaseStatusCmdInStepColor;    //ɫ��ģʽ���·��Ĺض�ָ���������ǰ���н׶ε���λ���

#ifdef VIRTUAL_DEVICE
    TVirtualRunTime m_tVirtualTime;                         //�����źŻ����е�����ʱ��

    bool m_bIsUseVirtualTime;                               //ʹ���������ʱ��

    bool m_bIsSpeedyRun;                                    //�����źŻ��Ƿ��������
#endif // VIRTUAL_DEVICE
    //Virtual_Test2022

    TPreemptControlStatus m_tPreemptControlStatus;          //���ȿ���״̬

	TPreemptCtlCmd        m_tPreemptCtlCmd;                 //���ȿ�������
	
	std::list<TPreemptCtlCmd> m_tPreemptCtlCmdList;         //���ȿ����������
	
    //20999Ԥ��
    TDeviceStatus  m_tDeviceStatus;                         //�豸״̬

    int CommFlag;                                           //���ù���ѡ���źŻ�ͨѶ�߳�

    bool m_bPhaseControlChange;                             //��λ��ֹ�������Ƿ���
};

#endif // !ifndef OPENATCRUNSTATUS_H
