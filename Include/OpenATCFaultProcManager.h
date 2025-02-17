/*=====================================================================
模块名 ：故障监测模块
文件名 ：OpenATCFaultProcManager.h
相关文件：OpenATCParameter.h OpenATCRunStatus.h
实现功能：故障监测模块调度类，用于故障监测。
作者 ：刘黎明
版权 ：<Copyright(c) 2019-2020 Suzhou Keda Technology Co.,Ltd. All right reserved.>
------------------------------------------------------------------------
修改记录：
日 期           版本     修改人     走读人     修改记录
2019/9/26       V1.0     刘黎明     刘黎明     创建模块
=====================================================================*/

#ifndef OPENATCFAULTPROCMANAGER_H
#define OPENATCFAULTPROCMANAGER_H

#include "../Include/OpenATCParameter.h"
#include "../Include/OpenATCRunStatus.h"
#include "../Include/OpenATCRunStatusDefine.h"
#include "../Include/CanBusManager.h"

#include <mutex>

#define LAMP_DRV_ERROR_ON_VOLT_NONE    0xffffff00
#define LAMP_DRV_ERROR_ON_VOLT_FAIL    0x00000001 //未输出有效电压
#define LAMP_DRV_ERROR_ON_VOLT_LOW     0x00000002 //输出电压低于输入电压过多
#define LAMP_DRV_ERROR_ON_VOLT_HIGH    0x00000004 //输出电压高于输入电压
#define LAMP_DRV_ERROR_OFF_VOLT_NONE   0xffff00ff
#define LAMP_DRV_ERROR_OFF_VOLT_HIGH   0x00000100 //关闭输出但实际仍然输出
#define LAMP_DRV_ERROR_OFF_VOLT_LOW    0x00000200 //关闭输出但实际部分输出
#define LAMP_DRV_ERROR_LINE_AC_RESIDUE 0x00000400 //线路残留电压过高

#define LAMP_DRV_ERROR_ON_POWER_NONE   0xff00ffff
#define LAMP_DRV_ERROR_ON_POWER_UP     0x00010000 //灯功率异常增加
#define LAMP_DRV_ERROR_ON_POWER_DOWN   0x00020000 //灯功率异常减小
#define LAMP_DRV_ERROR_ON_POWER_ZERO   0x00800000 //灯功率无输出
#define LAMP_DRV_ERROR_OFF_POWER_NONE  0x00ffffff
#define LAMP_DRV_ERROR_OFF_POWER_HIGH  0x01000000 //关闭状态有功率输出

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
    char m_chLampCtlBoardOnline;                                                //板卡在线故障,1为在线,0为不在线
    char m_chRedAndGreenOnFault;                                                //灯组红绿同亮故障
    char m_achLampPower[C_N_LAMPBORAD_OUTPUTNUM];                               //灯功率故障,0为正常,1为故障
    char m_achLampVoltage[C_N_LAMPBORAD_OUTPUTNUM];                             //灯电压故障,0为正常,1为故障    
}TOneLampCtlBoardFaultStatus,*PTOneLampCtlBoardFaultStatus;                     //记录灯控板故障状态,用于模块间数据交互

typedef struct tagLampCtlBoardFaultStatus
{
    int m_nLampCtlBoardCount;                                                   //板卡数量
    char m_chRedLampOutFault;                                                   //红灯全灭故障
    char m_chGreenConflictFault;                                                //绿冲突故障
    char m_chLampCtlBoardNumFault;                                              //灯控板数量故障
    char m_achLampCtlBoardUseStatus[C_N_MAXLAMPBOARD_NUM];                      //板卡使用状态,从特征参数读取 
    TOneLampCtlBoardFaultStatus m_atLampCtlBoardFault[C_N_MAXLAMPBOARD_NUM];    //板卡故障状态信息数组
}TLampCtlBoardFaultStatus,*PTLampCtlBoardFaultStatus;

typedef struct tagOneVehDetBoardFaultStatus
{
    char m_chDetBoardOnline;                                                    //板卡在线故障,1为在线,0为不在线
    char m_achDetectorUseStatus[C_N_MAXDETINPUT_NUM];                           //检测器使用状态,1为使用,0为不使用
    char m_achDetectorFault[C_N_MAXDETINPUT_NUM];                               //检测器故障状态,0为正常,1为故障
}TOneVehDetBoardFaultStatus,*PTOneVehDetBoardFaultStatus;

typedef struct tagVehDetBoardFaultStatus
{
    int m_nDetBoardCount;                                                       //板卡数量
    char m_achVehDetBoardUseStatus[C_N_MAXDETBOARD_NUM];                        //板卡使用状态,从特征参数读取                                                       
    TOneVehDetBoardFaultStatus m_atDetBoardFault[C_N_MAXDETBOARD_NUM];          //板卡故障状态信息数组
}TVehDetBoardFaultStatus,*PTVehDetBoardFaultStatus;

typedef struct tagOneIOBoardFaultStatus
{
    char m_chIOBoardOnline;                                                    //板卡在线故障,1为在线,0为不在线
}TOneIOBoardFaultStatus,*PTOneIOBoardFaultStatus;

typedef struct tagIOBoardFaultStatus
{
    int m_nIOBoardCount;                                                       //板卡数量                                                       
    char m_achIOBoardUseStatus[C_N_MAXIOBOARD_NUM];                            //板卡使用状态,从特征参数读取                                                       
    TOneIOBoardFaultStatus m_atIOBoardFault[C_N_MAXIOBOARD_NUM];               //板卡故障状态信息数组
}TIOBoardFaultStatus,*PTIOBoardFaultStatus;

typedef struct tagFaultDetBoardFaultStatus
{
    char m_chFaultDetBoardOnline;                                              //板卡在线故障,1为在线,0为不在线
}TFaultDetBoardFaultStatus,*PTFaultDetBoardFaultStatus;

typedef struct tagMainCtlBoardFaultStatus
{
    char m_achCanBusFault[C_N_MAXCANBUS_NUM];                                  //can总线故障,0为正常,1为故障
    char m_chTZParamFault;                                                     //特征参数故障,0为正常,非0为故障
    char m_chYellowFlashFault;                                                 //黄闪器故障,0为正常,非0为故障
    TFaultDetBoardFaultStatus m_tFaultDetBoardStatus;                          //故障检测板故障信息
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
    //类定义为单件
    static COpenATCFaultProcManager * getInstance();

    //类的初始化操作
    void Init(COpenATCParameter * pParameter,COpenATCRunStatus * pRunStatus, COpenATCLog * pOpenATCLog);

    //类的主流程
    void Work();

    //自检流程
    void SelfDetect();    

    //类的停止与释放
    void Stop();

    //获取绿冲突通道表                 
    void GetGreenConflictTable(int nChannelIndex, unsigned char chSendBuff[]);

    //手动模式下，方向控制时的通道冲突判断                   
    bool ConflictProcByChannel(int nKeyIndex);

	const char* GenDetailedFaultInfo(int nBoardType);

	//保存故障
	void AddOneFaultMessage(char cBoardType, char cFaultLevel, DWORD wFaultType, DWORD wFaultSubType, char cFaultInfo1, char cFaultInfo2);

	//清除故障
	void EraseOneFaultMessage(char cBoardType, char cFaultLevel, DWORD wFaultType, DWORD wFaultSubType, char cFaultInfo1, char cFaultInfo2);

	//保存自检故障
	void SaveParamInitFault(TParamRunStatus tParamStatus, COpenATCRunStatus * pRunStatus, COpenATCLog * pOpenATCLog);

	//保存自检故障
	void ClearParamInitFault(TParamRunStatus tParamStatus, COpenATCRunStatus * pRunStatus, COpenATCLog * pOpenATCLog);

private:
	COpenATCFaultProcManager();
	~COpenATCFaultProcManager();

    //根据特征参数，初始化故障监测参数
    void InitFaultProcParam(bool bParamChg);

    //根据并发相位,通道,跟随相位生成绿冲突信息表
    void InitGreenConflictInfo();

    //根据参数判断两个通道是否冲突
    bool IsTwoChannelConflict(BYTE bySrcChannelIndex,BYTE byDstChannelIndex,bool bAllChannel);

    //获取通道对应的相位编号
    void GetChannelSrcPhase(BYTE byChannelIndex,int & nPhaseCount,BYTE * pPhaseInfo,bool bAllChannel);

    //板卡自检
    bool AllBoardSelfDetect();

    //灯控故障监测
    void LampCtlFaultProc();
    //灯控板在线数量检测
    bool LampNumFaultProc(TLampCltBoardData & tLampCtlBoardInfo);
    //绿冲突监测
    bool GreenConflictProcByPhase(TLampCltBoardData & tLampCtlBoardInfo);
    bool GreenConflictProcByChannel(TLampCltBoardData & tLampCtlBoardInfo);
    //灯组红绿同亮监测
    bool GreenAndRedOnProc(TLampCltBoardData & tLampCtlBoardInfo);
    //无红灯亮起监测
    bool NoRedProc(TLampCltBoardData & tLampCtlBoardInfo);
    
    //车检故障监测
    void VehDetFaultProc();
    //车检板在线数量检测
    bool VehDetNumFaultProc(TVehDetBoardData & tVehDetBoardInfo);
    //车检板线圈故障检测
    bool VehDetectorFaultProc(TVehDetBoardData & tVehDetBoardInfo);

    //主控故障监测
    void MainCtlFaultProc();
    //CAN总线故障检测
    bool CanBusFaultProc();
    //故障监测板故障检测    
    bool FaultDetFaultProc(TFaultDetBoardData & tFaultDetBoardInfo);
    //黄闪器故障检测
    bool YellowFlashFaultProc();
    //特征参数故障检测
    bool TZParamFaultProc(TParamRunStatus & tParamStatus);

    //车检故障监测
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
	
	//计算计数的函数
    unsigned long CalcCounter(unsigned long nStart,unsigned long nEnd,unsigned long nMax); 

	//CAN通信发送的灯组状态分解成红灯黄灯绿灯状态
    void GetRYGStatusByGroup(char chGroup,char & chR,char & chY,char & chG);

	void OpenATCGetCurTime(int & nYear, int & nMon, int & nDay, int & nHour, int & nMin, int & nSec, int & nWeek);

	static COpenATCFaultProcManager * s_pData;              //单件类指针

    COpenATCParameter * m_pLogicCtlParam;                   //特征参数类指针

    COpenATCRunStatus * m_pLogicCtlStatus;                  //运行状态类指针

	COpenATCLog       * m_pOpenATCLog;                      //日志指针

    TPhaseConcurInfo m_tPhaseConcurInfo;                    //并发相位信息表

    TChannel m_atChannelInfo[MAX_CHANNEL_COUNT];            //通道信息表
    BYTE m_abyChannelNumToIndex[MAX_CHANNEL_COUNT];         //通道编号作为下标对应通道索引

    TOverlapInfo m_tOverlapInfo;                            //跟随相位信息表

    char m_achGreenConflictInfo[MAX_CHANNEL_COUNT][MAX_CHANNEL_COUNT];  //通道绿冲突信息表,0表示不能同时亮绿灯,1表示可以同时亮绿灯

    char m_achGreenConflictInfoToFaultDetBoard[MAX_CHANNEL_COUNT][MAX_CHANNEL_COUNT];  //通道绿冲突信息表,0表示不能同时亮绿灯,1表示可以同时亮绿灯
    
    char m_achLampCtlBoardStatus[C_N_MAXLAMPBOARD_NUM];     //灯控板使用信息数组,从特征参数中读取.
    char m_achVehDetBoardStatus[C_N_MAXDETBOARD_NUM];       //车检板使用信息数组,从特征参数中读取.
    char m_achIOBoardStatus[C_N_MAXIOBOARD_NUM];            //IO板使用信息数组,从特征参数中读取.
    char m_achDetectorUseStatus[C_N_MAXDETBOARD_NUM * C_N_MAXDETINPUT_NUM];//车检器使用信息数组

    TOpenATCFaultStatus m_tOpenATCFaultStatus;              //整个信号机的故障信息

    bool m_bFaultDetectBoardOnLineStatus;//判断故障板是否上线

    bool m_bLampCtlBoardOnlineStatus[C_N_MAXLAMPBOARD_NUM];//判断灯控板是否上线

    bool m_bNoRedOn;//用于判断无红灯亮起故障

    bool m_bFaultDetOffLine;//用于判断故障板掉线故障

    bool m_bLampBoardOffLine[C_N_MAXLAMPBOARD_NUM];//用于判断灯控板掉线故障

    bool m_bGreenAndRedOn[C_N_MAXLAMPBOARD_NUM][C_N_CHANNELNUM_PER_BOARD];//用于判断红绿同亮故障

    bool m_bGreenConflict[MAX_CHANNEL_COUNT][MAX_CHANNEL_COUNT];//用于判断绿冲突故障

	TAscFault m_pTAscFault;//信号机故障

    int       m_nLampNumFault[C_N_MAXLAMPBOARD_NUM];//灯控板数量故障代码

    int       m_nLampGreenAndRedOnFault[C_N_MAXLAMPBOARD_NUM][C_N_CHANNELNUM_PER_BOARD];//灯控板红绿同亮故障代码 

    int       m_nGreenConflictFaultByChannel[MAX_CHANNEL_COUNT][MAX_CHANNEL_COUNT];//灯控板绿冲突故障代码 

    int       m_nGreenConflictFaultByPhase[MAX_PHASE_COUNT][MAX_PHASE_COUNT];//灯控板绿冲突故障代码

    int       m_nVehDetNumFault[C_N_MAXDETBOARD_NUM];//车检板数量故障 代码

	bool      m_bOldVehDetectorFaultStatus[C_N_MAXDETBOARD_NUM][C_N_MAXDETINPUT_NUM];//保存的最新一次车检板检测器故障状态

    int       m_nVehDetectorFault[C_N_MAXDETBOARD_NUM][C_N_MAXDETINPUT_NUM];//车检板检测器故障代码 

    int       m_nIONumFault[C_N_MAXIOBOARD_NUM];//IO板数量故障 代码

	bool      m_bOldLampGroupFaultStatus[C_N_MAXLAMPBOARD_NUM][C_N_CHANNELNUM_PER_BOARD][C_N_CHANNEL_OUTPUTNUM];//保存的最新一次灯控板灯组故障状态

    unsigned int  m_nLampGroupFault[C_N_MAXLAMPBOARD_NUM][C_N_CHANNELNUM_PER_BOARD][C_N_CHANNEL_OUTPUTNUM];//灯控板灯组故障代码

    int       m_nLampBoardUseMaxIndex;

	char             m_achOldGreenConflictInfoToFaultDetBoard[MAX_CHANNEL_COUNT][MAX_CHANNEL_COUNT];  //通道绿冲突信息表,0表示不能同时亮绿灯,1表示可以同时亮绿灯

	TChannel         m_atOldChannelInfo[MAX_CHANNEL_COUNT]; //保存的最新一次的通道参数

	TVehicleDetector m_atOldVehDetector[MAX_VEHICLEDETECTOR_COUNT];//保存的最新一次的检测器参数

	bool             m_bLampBoardCtrParamChg[C_N_MAXLAMPBOARD_NUM][C_N_CHANNELNUM_PER_BOARD];//灯控板灯组的参数是否发生变化

	bool             m_bDetectorChg[C_N_MAXDETBOARD_NUM][C_N_MAXDETINPUT_NUM];//车检板检测器参数是否发生变化

	bool             m_bDeviceParamOtherInfoFaultFlagChg;//除siteid外的设备信息异常是否发生变化

private:
	int  Start(float fSaveFps = 0.01);

	int  Join();

	int  Detach();

	void Run();

	static void *OPENATCFAULTLOG_CALLBACK RunThread(void *pParam);

	void OpenATCSleep(long nMsec);

	void SaveOneFaultMessage(TAscFault *pTAscFault);

	bool IsConflictTableChanged();

    std::mutex m_hMutex;          // 保护 g_OpenATCFaultLog 全局变量

	OPENATCFAULTLOGHANDLE   m_hThread;

	unsigned long           m_dwThreadRet;

	bool                    m_bExitFlag;

	int                     m_nDetachState;

	bool                    m_bIsExit;				//是否有新故障入队列标志

	float                   m_fSaveFps;				//每秒钟故障保存频率

};

#endif // !ifndef OPENATCFAULTPROCMANAGER_H
