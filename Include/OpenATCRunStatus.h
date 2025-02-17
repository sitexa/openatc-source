/*=====================================================================
模块名 ：系统运行状态记录模块
文件名 ：OpenATCRunStatus.h
相关文件：
实现功能：用于记录整个主控模块的运行状态
作者 ：刘黎明
版权 ：<Copyright(c) 2019-2020 Suzhou Keda Technology Co.,Ltd. All right reserved.>
-----------------------------------------------------------------------
修改记录：
日 期           版本     修改人     走读人     修改记录
2019/9/14       V1.0     刘黎明     刘黎明     创建模块
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
类名 ：COpenATCRunStatus
功能 ：用于记录主控软件运行时的各种状态，涵盖了板卡状态，板卡实时通信数据，特征参数运行状态等各类数据。
主要接口：
备注 ：
-----------------------------------------------------------------------
修改记录：
日 期          版本     修改人     走读人       修改记录
2019/09/14     V1.0     刘黎明     刘黎明       创建类
2019/12/18     V1.0     李永萍     李永萍       添加故障队列和信号机故障信息
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

    //初始化状态参数
    void Init();

    //获得主控板运行状态信息
    inline void GetMainCtlBoardRunStatus(TMainCtlBoardRunStatus & tRunStatus)
    {
        memcpy(&tRunStatus,&m_tMainCtlBoardRunStatus,sizeof(TMainCtlBoardRunStatus));
    }
    //设置主控板运行状态信息
    inline void SetMainCtlBoardRunStatus(const TMainCtlBoardRunStatus & tRunStatus)
    {
        memcpy(&m_tMainCtlBoardRunStatus,&tRunStatus,sizeof(TMainCtlBoardRunStatus));
    }

    //获得特征参数运行状态信息
    inline void GetParamRunStatus(TParamRunStatus & tRunStatus)
    {
        memcpy(&tRunStatus,&m_tParamRunStatus,sizeof(TParamRunStatus));
    }
    //设置特征参数运行状态信息
    inline void SetParamRunStatus(const TParamRunStatus & tRunStatus)
    {
        memcpy(&m_tParamRunStatus,&tRunStatus,sizeof(TParamRunStatus));
    }

    //获得灯色运行状态信息
    inline void GetLampClrStatus(TLampClrStatus & tLampClrStatus)
    {
        memcpy(&tLampClrStatus,&m_tAllLampClrStatus,sizeof(TLampClrStatus));
    }
    //设置灯色运行状态信息
    inline void SetLampClrStatus(const TLampClrStatus & tLampClrStatus)
    {
        memcpy(&m_tAllLampClrStatus,&tLampClrStatus,sizeof(TLampClrStatus));
    }

    //获得逻辑控制运行状态信息
    inline void GetLogicCtlStatus(TLogicCtlStatus & tRunStatus)
    {
        memcpy(&tRunStatus,&m_tLogicCtlStatus,sizeof(TLogicCtlStatus));
    }
    //设置逻辑控制运行状态信息
    inline void SetLogicCtlStatus(const TLogicCtlStatus & tRunStatus)
    {
        memcpy(&m_tLogicCtlStatus,&tRunStatus,sizeof(TLogicCtlStatus));
    }

    //获得灯控板点灯数据
    inline void GetLampCtlBoardData(TLampCltBoardData & tLampCtlBoardInfo)
    {
        memcpy(&tLampCtlBoardInfo,&m_tLampCtlBoardData,sizeof(TLampCltBoardData));
    }
    //设置灯控板点灯数据
    inline void SetLampCtlBoardData(const TLampCltBoardData & tLampCtlBoardInfo)
    {
        memcpy(&m_tLampCtlBoardData,&tLampCtlBoardInfo,sizeof(TLampCltBoardData));
    }
 
    //获取灯控故障状态
    inline void GetLampFault(TLampFaultType & tLampFault)
    {
        memcpy(&tLampFault,&m_tSysFault.m_tLampFault,sizeof(TLampFaultType));        
    }
    //设置灯控故障状态
    inline void SetLampFault(const TLampFaultType & tLampFault)
    {
        memcpy(&m_tSysFault.m_tLampFault,&tLampFault,sizeof(TLampFaultType));        
    }

    //获取车检板数据
    inline void GetVehDetBoardData(TVehDetBoardData & tVehDetData)
    {
        memcpy(&tVehDetData,&m_tVehDetBoardData,sizeof(TVehDetBoardData));        
    }
    //设置车检板数据
    inline void SetVehDetBoardData(const TVehDetBoardData & tVehDetData)
    {
        memcpy(&m_tVehDetBoardData,&tVehDetData,sizeof(TVehDetBoardData));        
    }

    //获取实时车检数据
    inline void GetRTVehDetData(TRealTimeVehDetData & tVehDetData)
    {
        memcpy(&tVehDetData,&m_tRealTimeDetData,sizeof(TRealTimeVehDetData));        
    }
    //设置实时车检数据
    inline void SetRTVehDetData(const TRealTimeVehDetData & tVehDetData)
    {
        memcpy(&m_tRealTimeDetData,&tVehDetData,sizeof(TRealTimeVehDetData));        
    }

    //获取全局计数器
    inline unsigned long GetGlobalCounter()
    {
        return m_nGlobalCounter;
    }
    //设置全局计数器
    inline void SetGlobalCounter(unsigned long nCounter)
    {
        m_nGlobalCounter = nCounter;
    }

    //获取灯色计数器
    inline unsigned long GetLampClrCounter()
    {
        return m_nLampClrCounter;
    }
    //设置灯色计数器
    inline void SetLampClrCounter(unsigned long nCounter)
    {
        m_nLampClrCounter = nCounter;
    }

    //获取灯色计时信息
    inline void GetPhaseLampClrRunCounter(TPhaseLampClrRunCounter & tRunCounter)
    {
        memcpy(&tRunCounter,&m_tPhaseLampClrRunCounter,sizeof(TPhaseLampClrRunCounter));        
    }
    //设置灯色计时信息
    inline void SetPhaseLampClrRunCounter(const TPhaseLampClrRunCounter & tRunCounter)
    {
        memcpy(&m_tPhaseLampClrRunCounter,&tRunCounter,sizeof(TPhaseLampClrRunCounter));        
    }

    //获取板卡使用状态
    inline void GetAllBoardUseStatus(TAllBoardUseStatus & tUseStatus)
    {
        memcpy(&tUseStatus,&m_tAllBoardUseStatus,sizeof(TAllBoardUseStatus));        
    }
    //设置板卡使用状态
    inline void SetAllBoardUseStatus(const TAllBoardUseStatus & tUseStatus)
    {
        memcpy(&m_tAllBoardUseStatus,&tUseStatus,sizeof(TAllBoardUseStatus));        
    }

    //获取板卡在线状态
    inline void GetAllBoardOnlineStatus(TAllBoardOnlineStatus & tOnlineStatus)
    {
        memcpy(&tOnlineStatus,&m_tAllBoardOnlineStatus,sizeof(TAllBoardOnlineStatus));        
    }
    //设置板卡在线状态
    inline void SetAllBoardOnlineStatus(const TAllBoardOnlineStatus & tOnlineStatus)
    {
        memcpy(&m_tAllBoardOnlineStatus,&tOnlineStatus,sizeof(TAllBoardOnlineStatus));        
    }

    //获取相位运行状态写标记
    inline bool GetPhaseRunStatusWriteFlag()
    {
        return m_bPhaseRunStatusWriteFlag;
    }
    //设置相位运行状态写标记
    inline void SetPhaseRunStatusWriteFlag(bool bFlag)
    {
        m_bPhaseRunStatusWriteFlag = bFlag;
    }    

    //获取相位运行状态读标记
    inline bool GetPhaseRunStatusReadFlag()
    {
        return m_bPhaseRunStatusReadFlag;
    }
    //设置相位运行状态读标记
    inline void SetPhaseRunStatusReadFlag(bool bFlag)
    {
        m_bPhaseRunStatusReadFlag = bFlag;
    } 

    //获取相位运行状态
    inline void GetPhaseRunStatus(TPhaseRunStatus & tRunStatus)
    {
        memcpy(&tRunStatus,&m_tPhaseRunStatus,sizeof(TPhaseRunStatus));        
    }
    //设置相位运行状态
    inline void SetPhaseRunStatus(const TPhaseRunStatus & tRunStatus)
    {
        memcpy(&m_tPhaseRunStatus,&tRunStatus,sizeof(TPhaseRunStatus));        
    }

    //获取IO板通信数据
    inline void GetIOBoardData(TIOBoardData & tBoardData)
    {
        memcpy(&tBoardData,&m_tIOBoardData,sizeof(TIOBoardData));        
    }
    //设置IO板通信数据
    inline void SetIOBoardData(const TIOBoardData & tBoardData)
    {
        memcpy(&m_tIOBoardData,&tBoardData,sizeof(TIOBoardData));        
    }

    //获取故障检测板通信数据
    inline void GetFaultDetBoardData(TFaultDetBoardData & tBoardData)
    {
        memcpy(&tBoardData,&m_tFaultDetBoardData,sizeof(TFaultDetBoardData));        
    }
    //设置故障检测板通信数据
    inline void SetFaultDetBoardData(const TFaultDetBoardData & tBoardData)
    {
        memcpy(&m_tFaultDetBoardData,&tBoardData,sizeof(TFaultDetBoardData));        
    }

    //获取自检状态
    inline bool GetSelfDetectStatus()
    {
        return m_bSelfDetect;
    }
    //设置自检状态
    inline void SetSelfDetectStatus(bool bStatus)
    {
        m_bSelfDetect = bStatus;
    }

    //获取是否停止运行
    inline bool GetStopWorkStatus()
    {
        return m_bIsStopWork;
    }
    //设置是否停止运行
    inline void SetStopWorkStatus(bool bStatus)
    {
        m_bIsStopWork = bStatus;
    }

    //获取重启状态
    inline bool GetRebootStatus()
    {
        return m_bIsRebootATC;
    }
    //设置重启状态
    inline void SetRebootStatus(bool bStatus)
    {
        m_bIsRebootATC = bStatus;
    }

    //推送故障到故障队列
    inline bool PushFaultToQueue(TAscFault tAscFault)
    {
        return m_FaultQueueToCenter.Push(tAscFault);
    }

    //从故障队列获取故障数据
    inline bool PopFaultFromQueue(TAscFault & tAscFault)
    {
        return m_FaultQueueToCenter.Pop(tAscFault);
    }

	//获取逻辑控制一个周期是否结束的状态
    inline bool GetCycleChgStatus()
    {
        return m_bIsCycleChgForFault;
    }
    //设置逻辑控制一个周期是否结束的状态
    inline void SetCycleChgStatus(bool bStatus)
    {
        m_bIsCycleChgForFault = bStatus;
    }

	//获取绿闪计数
    inline int GetGreenFlashCount(int nIndex)
    {
        return m_nGreenFlashCount[nIndex];
    }
    //设置绿闪计数
    inline void SetGreenFlashCount(int nIndex, int nCount)
    {
        m_nGreenFlashCount[nIndex] = nCount;
    }
	
	//获取红闪计数
    inline int GetRedFlashCount(int nIndex)
    {
        return m_nRedFlashCount[nIndex];
    }
    //设置红闪计数
    inline void SetRedFlashCount(int nIndex, int nCount)
    {
        m_nRedFlashCount[nIndex] = nCount;
    }

	// 设置U盘挂载标志
	inline void SetUSBMountFlag(bool bFlag)
	{
		m_bIsUSBMounted = bFlag;
	}
	
	//获取U盘挂载标志
	inline bool GetUSBMountFlag()
	{
		return m_bIsUSBMounted;
	}

	//设置can1总线指示灯状态
	inline void SetCAN1LedStatus(bool bFlag)
	{
		m_bCAN1LedStatus = bFlag;
	}

	//获取can1总线指示灯状态
	inline bool GetCAN1LedStatus()
	{
		return m_bCAN1LedStatus;
	}

	//设置can2总线指示灯状态
	inline void SetCAN2LedStatus(bool bFlag)
	{
		m_bCAN2LedStatus = bFlag;
	}
	
	// 获取can2总线指示灯状态
	inline bool GetCAN2LedStatus()
	{
		return m_bCAN2LedStatus;
	}

	//设置GPS指示灯状态
	inline void SetGPSLedStatus(bool bFlag)
	{
		m_bGPSLedStatus = bFlag;
	}
	
	// 获取GPS指示灯状态
	inline bool GetGPSLedStatus()
	{
		return m_bGPSLedStatus;
	}

	//设置ERR指示灯状态
	inline void SetErrLedStatus(bool bFlag)
	{
		m_bErrLedStatus = bFlag;
	}
	
	//获取ERR指示灯状态
	inline bool GetErrLedStatus()
	{
		return m_bErrLedStatus;
	}

	//设置需要存储的手动面板按钮响应状态队列
	inline void SetPanelBtnStatusList(THWPanelBtnStatus tStatus)
	{
		m_HWPanelBtnStatusList.push_back(tStatus);
	}

	//获取需要存储的手动面板按钮响应状态队列
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

    //设置需要存储的GPS数据队列
	inline void SetGpsDataList(TGpsData tData)
	{
		m_tGpsDataList.push_back(tData);
	}

	//获取需要存储的GPS数据队列
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

    //设置整机各个设备状态数据
	inline void SetOpenATCStatusInfo(TOpenATCStatusInfo tOpenATCStatusInfo)
	{
		memcpy(&m_tOpenATCStatusInfo, &tOpenATCStatusInfo, sizeof(TOpenATCStatusInfo)); 
	}
	
	//获取整机各个设备状态数据
	inline void GetOpenATCStatusInfo(TOpenATCStatusInfo & tOpenATCStatusInfo)
	{
		memcpy(&tOpenATCStatusInfo, &m_tOpenATCStatusInfo, sizeof(TOpenATCStatusInfo)); 
    }
	
	//获取显示屏显示的信息
    inline void GetLedScreenShowInfo(TLedScreenShowInfo & tInfo)
    {
        memcpy(&tInfo,&m_tLedScreenShowInfo,sizeof(TLedScreenShowInfo));        
    }
    //设置显示屏显示的信息
    inline void SetLedScreenShowInfo(const TLedScreenShowInfo & tInfo)
    {
        memcpy(&m_tLedScreenShowInfo,&tInfo,sizeof(TLedScreenShowInfo));        
    }

    //设置地址板信息
    inline void SetSiteRev(int nArray[])
    {
        for (int i = 0;i < sizeof(m_tOpenATCStatusInfo.m_tWholeDevStatusInfo.m_cHardwareVer);i++)
        {
            m_tOpenATCStatusInfo.m_tWholeDevStatusInfo.m_cHardwareVer[i] = nArray[i];
        }
    }

    //获取灯控板掉线状态
    inline bool GetLampCtlBoardOffLine(int nBoardIndex)
    {
        return m_bLampCtlBoardOffLine[nBoardIndex];  
    }
    //设置灯控板掉线状态
    inline void SetLampCtlBoardOffLine(int nBoardIndex, bool bOffLine)
    {
        m_bLampCtlBoardOffLine[nBoardIndex] = bOffLine;      
    }

    //获取IO板掉线状态
    inline bool GetIOBoardOffLine(int nBoardIndex)
    {
        return m_bIOBoardOffLine[nBoardIndex];  
    }
    //设置IO板掉线状态
    inline void SetIOBoardOffLine(int nBoardIndex, bool bOffLine)
    {
        m_bIOBoardOffLine[nBoardIndex] = bOffLine;      
    }
  
    //获取车检板掉线状态
    inline bool GetDetBoardOffLine(int nBoardIndex)
    {
        return m_bDetBoardOffLine[nBoardIndex];  
    }
    //设置车检板掉线状态
    inline void SetDetBoardOffLine(int nBoardIndex, bool bOffLine)
    {
        m_bDetBoardOffLine[nBoardIndex] = bOffLine;      
    }

    //获取故障板掉线状态
    inline bool GetFaultDetBoardOffLine()
    {
        return m_bFaultDetBoardOffLine;  
    }
    //设置故障板掉线状态
    inline void SetFaultDetBoardOffLine(bool bOffLine)
    {
        m_bFaultDetBoardOffLine = bOffLine;      
    }

    //获取通道绿冲突信息表
    inline void GetGreenConflictInfo(char chGreenConflictInfo[MAX_CHANNEL_COUNT][MAX_CHANNEL_COUNT])
    {
        memcpy(chGreenConflictInfo, m_achGreenConflictInfo, sizeof(m_achGreenConflictInfo));
    }
    //设置通道绿冲突信息表
    inline void SetGreenConflictInfo(char chGreenConflictInfo[MAX_CHANNEL_COUNT][MAX_CHANNEL_COUNT])
    {
        memcpy(m_achGreenConflictInfo, chGreenConflictInfo, sizeof(m_achGreenConflictInfo));
    }

	//获取通道检测信息表
	inline void GetChannelCheckInfo(TAscChannelVerifyInfo &tChannelInfo)
	{
		memcpy(&tChannelInfo, &m_tChannelVerifyInfo, sizeof(m_tChannelVerifyInfo));
	}
	//设置通道检测信息表
	inline void SetChannelCheckInfo(TAscChannelVerifyInfo &tChannelInfo)
	{
		memcpy(&m_tChannelVerifyInfo, &tChannelInfo, sizeof(tChannelInfo));
	}

	//获取自检信息
	inline void GetSelfDetectInfo(TSelfDetectInfo &tSelfDetectInfo)
	{
		memcpy(&tSelfDetectInfo, &m_tSelfDetectInfo, sizeof(m_tSelfDetectInfo));
	}
	//设置自检信息
	inline void SetSelfDetectInfo(TSelfDetectInfo &tSelfDetectInfo)
	{
		memcpy(&m_tSelfDetectInfo, &tSelfDetectInfo, sizeof(tSelfDetectInfo));
	}

	//获取Java是否停止运行
    inline time_t GetJavaHeartTime()
    {
        return m_tJavaHeartTime;
    }
    //设置是否停止运行
    inline void SetJavaHeartTime(time_t javaHeartTime)
    {
        m_tJavaHeartTime = javaHeartTime;
    }

	//获取通道实时电压和电流
    inline void GetChannelStatusInfo(int nChannelIndex, TChannelStatusInfo & tChannelStatusInfo)
    {
        memcpy(&tChannelStatusInfo, &m_tChannelStatusInfo[nChannelIndex], sizeof(tChannelStatusInfo));
    }
    //设置通道实时电压和电流
    inline void SeChannelStatusInfo(int nChannelIndex, TChannelStatusInfo tChannelStatusInfo)
    {
        memcpy(&m_tChannelStatusInfo[nChannelIndex], &tChannelStatusInfo, sizeof(tChannelStatusInfo));
    }

	//获取车辆排队信息
    inline void GetVehicleQueueUpInfo(TVehicleQueueUpInfo tVehicleQueueUpInfo[])
    {
        memcpy(tVehicleQueueUpInfo, m_tVehicleQueueUpInfo, sizeof(m_tVehicleQueueUpInfo));
    }
    //设置车辆排队信息
    inline void SetVehicleQueueUpInfo(TVehicleQueueUpInfo tVehicleQueueUpInfo[])
    {
        memcpy(m_tVehicleQueueUpInfo, tVehicleQueueUpInfo, sizeof(m_tVehicleQueueUpInfo));
    }

	//获取行人检测信息
	inline void GetPedDetectInfo(TPedDetectInfo tPedDetectInfo[])
	{
		memcpy(tPedDetectInfo, m_tPedDetectInfo, sizeof(m_tPedDetectInfo));
	}
	//设置行人检测信息
	inline void SetPedDetectInfo(TPedDetectInfo tPedDetectInfo[])
	{
		memcpy(m_tPedDetectInfo, tPedDetectInfo, sizeof(m_tPedDetectInfo));
	}

	//获取故障板控制状态
	inline bool GeFaultDetBoardControlStatus()
	{
		return m_bFaultDetBoardControlStatus;
	}
	//设置故障板控制状态
	inline void SetFaultDetBoardControlStatus(bool bFlag)
	{
		m_bFaultDetBoardControlStatus = bFlag;
	}

	//获取配置软件的通信状态
	inline void GetComStatusWithCfg(bool & bUDPComStatus, bool & bTCPComStatus)
	{
		bUDPComStatus = m_bUDPComStatusWithCfg;
		bTCPComStatus = m_bTCPComStatusWithCfg;
	}
	//设置配置软件的通信状态
	inline void SetComStatusWithCfg(bool bUDPComStatus, bool bTCPComStatus)
	{
		m_bUDPComStatusWithCfg = bUDPComStatus;
		m_bTCPComStatusWithCfg = bTCPComStatus;
	}

	//获取设置显示屏显示消息队列标记
    inline bool GetLedScreenShowInfoFlag()
    {
        return m_bLedScreenShowInfoFlag;
    }
    //设置显示屏显示消息队列标记
    inline void SetLedScreenShowInfoFlag(bool bFlag)
    {
        m_bLedScreenShowInfoFlag = bFlag;
    }   

	//获取系统控制状态
	inline void GetSystemControlStatus(TSystemControlStatus &tSystemControlStatus)
	{
		memcpy(&tSystemControlStatus, &m_tSystemControlStatus, sizeof(TSystemControlStatus));
	}
	//设置系统控制状态
	inline void SetSystemControlStatus(TSystemControlStatus &tSystemControlStatus)
	{
		memcpy(&m_tSystemControlStatus, &tSystemControlStatus, sizeof(TSystemControlStatus));
	}

	//获取来自用户的手动指令
	inline void GetManualCmd(TManualCmd &tManualCmd)
	{
		memcpy(&tManualCmd, &m_tManualCmd, sizeof(tManualCmd));
	}
	//设置来自用户的手动指令
	inline void SetManualCmd(TManualCmd &tManualCmd)
	{
		memcpy(&m_tManualCmd, &tManualCmd, sizeof(tManualCmd));
	}

	//获取有效指令
	inline void GetValidManualCmd(TManualCmd &tManualCmd)
	{
		memcpy(&tManualCmd, &m_tValidManualCmd, sizeof(tManualCmd));
	}
	//设置有效指令
	inline void SetValidManualCmd(TManualCmd &tManualCmd)
	{
		memcpy(&m_tValidManualCmd, &tManualCmd, sizeof(tManualCmd));
	}
	
	//获取除siteid外的设备信息异常标记
	inline bool GetDeviceParamOtherInfoFaultFlag()
	{
		return m_bIfDeviceParamOtherInfoFault;
	}
	//设置除siteid外的设备信息异常标记
	inline void SetDeviceParamOtherInfoFaultFlag(bool bFlag)
	{
		m_bIfDeviceParamOtherInfoFault = bFlag;
	}   

	//获取用户控制相位放行控制状态
	inline void GetPhasePassCmdPhaseStatus(TPhasePassCmdPhaseStatus &tPhasePassCmdPhaseStatus)
	{
		memcpy(&tPhasePassCmdPhaseStatus, &m_tPhasePassCmdPhaseStatus, sizeof(TPhasePassCmdPhaseStatus));
	}
	//设置用户控制相位放行控制状态
	inline void SetPhasePassCmdPhaseStatus(TPhasePassCmdPhaseStatus tPhasePassCmdPhaseStatus)
	{
		memcpy(&m_tPhasePassCmdPhaseStatus, &tPhasePassCmdPhaseStatus, sizeof(TPhasePassCmdPhaseStatus));
	}

	//获取本地相位放行控制状态
	inline void GetLocalPhasePassStatus(TPhasePassCmdPhaseStatus &tPhasePassCmdPhaseStatus)
	{
		memcpy(&tPhasePassCmdPhaseStatus, &m_tLocalPhasePassStatus, sizeof(TPhasePassCmdPhaseStatus));
	}
	//设置本地相位放行控制状态
	inline void SetLocalPhasePassStatus(TPhasePassCmdPhaseStatus tPhasePassCmdPhaseStatus)
	{
		memcpy(&m_tLocalPhasePassStatus, &tPhasePassCmdPhaseStatus, sizeof(TPhasePassCmdPhaseStatus));
	}

	//获取色步模式下下发的关断指令里包含当前运行阶段的相位标记
	inline bool GetIncludedCurPhaseInPhaseStatusCmdInStepColorFlag()
	{
		return m_bIncludedCurPhaseInPhaseStatusCmdInStepColor;
	}
	//设置色步模式下下发的关断指令里包含当前运行阶段的相位标记
	inline void SetIncludedCurPhaseInPhaseStatusCmdInStepColorFlag(bool bFlag)
	{
		m_bIncludedCurPhaseInPhaseStatusCmdInStepColor = bFlag;
	}

#ifdef VIRTUAL_DEVICE
    //获取虚拟信号机运行的虚拟时间
    inline void GetVirtualTimeData(TVirtualRunTime& tVirtualTimeData)
    {
        memcpy(&tVirtualTimeData, &m_tVirtualTime, sizeof(TVirtualRunTime));
    }
    //设置虚拟信号机运行的虚拟时间
    inline void SetVirtualTimeData(const TVirtualRunTime& tVirtualTimeData)
    {
        memcpy(&m_tVirtualTime, &tVirtualTimeData, sizeof(TVirtualRunTime));
    }
    //获取是否加速运行状态
    inline bool GetIsSpeedyRunStatus()
    {
        return m_bIsSpeedyRun;
    }
    //设置是否加速运行状态
    inline void SetIsSpeedyRunStatus(bool bIsSpeedyRun)
    {
        m_bIsSpeedyRun = bIsSpeedyRun;
    }
#endif // VIRTUAL_DEVICE
    //Virtual_Test2022
    
    //获取优先控制状态
	inline void GetPreemptControlStatus(TPreemptControlStatus &tPreemptControlStatus)
	{
		memcpy(&tPreemptControlStatus, &m_tPreemptControlStatus, sizeof(TPreemptControlStatus));
	}
	//设置优先控制状态
	inline void SetPreemptControlStatus(TPreemptControlStatus &tPreemptControlStatus)
	{
		memcpy(&m_tPreemptControlStatus, &tPreemptControlStatus, sizeof(TPreemptControlStatus));
	}

	//获取优先控制指令
	inline void GetPreemptCtlCmd(TPreemptCtlCmd &tPreemptCtlCmd)
	{
		memcpy(&tPreemptCtlCmd, &m_tPreemptCtlCmd, sizeof(tPreemptCtlCmd));
	}
	//设置优先控制指令
	inline void SetPreemptCtlCmd(TPreemptCtlCmd &tPreemptCtlCmd)
	{
		memcpy(&m_tPreemptCtlCmd, &tPreemptCtlCmd, sizeof(tPreemptCtlCmd));
	}

	//设置需要优先控制命令队列
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

	//设置优先相位开始切换标志
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

	//从优先控制命令队列中获取优先控制命令
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

	//从优先控制命令队列中删除优先控制命令
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
						m_tPreemptCtlCmdList.clear();//本地或系统干预时，队列剩余指令全部删除
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
	//从优先控制命令队列中删除所有优先控制命令
	inline void DelteAllPreemptCtlCmdFromList()
	{
		m_tPreemptCtlCmdList.clear();
	}
	
    //20999预留
    //获取设备状态
    inline void GetDeviceStatus(TDeviceStatus& tDeviceStatus)
    {
        memcpy(&tDeviceStatus, &m_tDeviceStatus, sizeof(m_tDeviceStatus));
    }
    //设置设备状态
    inline void SetDeviceStatus(TDeviceStatus tDeviceStatus)
    {
        memcpy(&m_tDeviceStatus, &tDeviceStatus, sizeof(m_tDeviceStatus));
    }

    //获取选择何种通讯线程
    inline int GetCommFlagStatus()
    {
        return CommFlag;
    }
    //设置选择何种通讯线程
    inline void SetCommFlagStatus(int iCommFlag)
    {
        CommFlag = iCommFlag;
    }

    inline bool GetPhaseControlChange()
    {
        return m_bPhaseControlChange;
    }
    //设置选择何种通讯线程
    inline void SetPhaseControlChange(bool bChange)
    {
        m_bPhaseControlChange = bChange;
    }

private:
    TParamRunStatus m_tParamRunStatus;                      //特征参数当前状态信息

    TMainCtlBoardRunStatus m_tMainCtlBoardRunStatus;        //主控板当前运行状态信息

    TLampClrStatus m_tAllLampClrStatus;                     //灯控板灯色状态信息

    TLogicCtlStatus m_tLogicCtlStatus;                      //逻辑控制状态信息   

    TLampCltBoardData m_tLampCtlBoardData;                  //从灯控板采集的灯控板信息

  	TCommonFault m_tSysFault;                               //全局故障状态,目前使用灯控状态,用于和逻辑控制模块交换数据

    TVehDetBoardData m_tVehDetBoardData;                    //从车检板采集到的车检信息

    TRealTimeVehDetData m_tRealTimeDetData;                 //实时车检信息，用于感应控制

    unsigned long m_nGlobalCounter;                         //全局计数器

    unsigned long m_nLampClrCounter;                        //灯色运行计数器

    TPhaseLampClrRunCounter m_tPhaseLampClrRunCounter;      //相位灯色运行计时信息

    TAllBoardUseStatus m_tAllBoardUseStatus;                //所有板卡使用状态,从参数中获得

    TAllBoardOnlineStatus m_tAllBoardOnlineStatus;          //所有板卡在线状态,通过CAN通信获取信息

    bool m_bPhaseRunStatusWriteFlag;                        //相位运行状态写标记,当查询状态时通信线程把状态置为可写

    bool m_bPhaseRunStatusReadFlag;                         //相位运行状态读标记,当主循环把运行状态更新完毕后把状态置为可读

    TPhaseRunStatus m_tPhaseRunStatus;                      //相位运行状态,用于向通信线程传送相位运行状态数据

    TIOBoardData m_tIOBoardData;                            //从IO板采集到的IO数据

    TFaultDetBoardData m_tFaultDetBoardData;                //从故障检测板采集到的数据

	TAscChannelVerifyInfo m_tChannelVerifyInfo;				//通道检测数据

    bool m_bSelfDetect;                                     //信号机自检状态

    bool m_bIsStopWork;                                     //信号机是否结束运行

    bool m_bIsRebootATC;                                    //是否重启信号机

    COneWayQueue<TAscFault>  m_FaultQueueToCenter;          //故障队列    
	
	bool m_bIsCycleChgForFault;                             //逻辑控制是否一个周期结束，通知故障检测模块

	int  m_nGreenFlashCount[C_N_MAXLAMPOUTPUT_NUM];         //绿闪亮灭计数

	int  m_nRedFlashCount[C_N_MAXLAMPOUTPUT_NUM];           //红闪亮灭计数
	
	bool m_bIsUSBMounted;									//是否已挂载U盘
	bool m_bCAN1LedStatus;									//can1总线指示灯状态
	bool m_bCAN2LedStatus;									//can2总线指示灯状态
	bool m_bGPSLedStatus;									//GPS指示灯状态
	bool m_bErrLedStatus;									//ERR指示灯状态

	std::list<THWPanelBtnStatus> m_HWPanelBtnStatusList;    //手动面板按钮响应状态队列

    TGpsData		   m_tGpsData;				            //GPS数据 
    TOpenATCStatusInfo m_tOpenATCStatusInfo;                //传给指示灯板的整机状态数据
	
    TLedScreenShowInfo m_tLedScreenShowInfo;                //Lcd显示屏显示的信息
	
    std::list<TGpsData> m_tGpsDataList;                     //GPS数据队列

    bool m_bLampCtlBoardOffLine[C_N_MAXLAMPBOARD_NUM];      //灯控板掉线状态

    bool m_bIOBoardOffLine[C_N_MAXIOBOARD_NUM];             //IO板掉线状态

    bool m_bDetBoardOffLine[C_N_MAXDETBOARD_NUM];           //车检板掉线状态

    bool m_bFaultDetBoardOffLine;                           //故障板掉线状态

    char m_achGreenConflictInfo[MAX_CHANNEL_COUNT][MAX_CHANNEL_COUNT];  //通道绿冲突信息表,0表示不能同时亮绿灯,1表示可以同时亮绿灯

	TSelfDetectInfo m_tSelfDetectInfo;						//信号机自检信息

	time_t m_tJavaHeartTime;                                //Java服务发送心跳时间

	TChannelStatusInfo m_tChannelStatusInfo[MAX_CHANNEL_COUNT];//通道实时电压和电流

	TVehicleQueueUpInfo m_tVehicleQueueUpInfo[MAX_VEHICLEDETECTOR_COUNT];//车辆排队信息

	TPedDetectInfo m_tPedDetectInfo[MAX_PEDESTRIANDETECTOR_COUNT];//行人检测信息

	bool m_bFaultDetBoardControlStatus;

	bool m_bUDPComStatusWithCfg;                            //和配置软件的UDP通信状态

	bool m_bTCPComStatusWithCfg;                            //和配置软件的TCP通信状态

	bool m_bLedScreenShowInfoFlag;                          //显示屏显示消息标记

	TSystemControlStatus m_tSystemControlStatus;            //系统控制状态

	TManualCmd m_tManualCmd;                                //控制指令

	TManualCmd m_tValidManualCmd;                           //有效的控制指令
	
	bool m_bIfDeviceParamOtherInfoFault;				    //除siteid外的设备信息是否异常

	TPhasePassCmdPhaseStatus m_tPhasePassCmdPhaseStatus;	//将配置工具下发的相位放行状态记录下来

	TPhasePassCmdPhaseStatus m_tLocalPhasePassStatus;	    //将本地的相位放行状态记录下来

	bool m_bIncludedCurPhaseInPhaseStatusCmdInStepColor;    //色步模式下下发的关断指令里包含当前运行阶段的相位标记

#ifdef VIRTUAL_DEVICE
    TVirtualRunTime m_tVirtualTime;                         //虚拟信号机运行的虚拟时间

    bool m_bIsUseVirtualTime;                               //使用了虚拟的时间

    bool m_bIsSpeedyRun;                                    //虚拟信号机是否加速运行
#endif // VIRTUAL_DEVICE
    //Virtual_Test2022

    TPreemptControlStatus m_tPreemptControlStatus;          //优先控制状态

	TPreemptCtlCmd        m_tPreemptCtlCmd;                 //优先控制命令
	
	std::list<TPreemptCtlCmd> m_tPreemptCtlCmdList;         //优先控制命令队列
	
    //20999预留
    TDeviceStatus  m_tDeviceStatus;                         //设备状态

    int CommFlag;                                           //配置工具选择信号机通讯线程

    bool m_bPhaseControlChange;                             //相位禁止与屏蔽是否有
};

#endif // !ifndef OPENATCRUNSTATUS_H
