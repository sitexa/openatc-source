/*=====================================================================
模块名 ：CAN通信管理模块
文件名 ：CanBusManager.h
相关文件：OpenATCParameter.h OpenATCRunStatus.h
实现功能：用于主控板和灯控板，I0板，车检板和故障板之前的CAN通信。
作者 ：刘黎明
版权 ：<Copyright(c) 2019-2020 Suzhou Keda Technology Co.,Ltd. All right reserved.>
-----------------------------------------------------------------------
修改记录：
日 期           版本     修改人     走读人     修改记录
2019/9/26       V1.0     李永萍     李永萍     创建模块
=====================================================================*/

#ifndef CANBUSMANAGER_H
#define CANBUSMANAGER_H

#include "../Include/OpenATCParameter.h"
#include "../Include/OpenATCRunStatus.h"
#include "../Include/OpenATCParamStructDefine.h"
#include "../Include/OpenATCLog.h"
#include "Common.h"
#include <list>
#include <string>

class COpenATCCommWithCanReceiveThread;
class COpenATCCommWithCanSendThread;
////////////////////////////////////////////////////////////////////////////////////

#define		             MAX_CANOBJ_COUNT     200		
#define		             MAX_CANINDEX_COUNT   2	

#define                  BOARD_IO_PLUG        8
#define                  BOARD_VEHDET_PLUG    6
#define                  MASTER_BOARD_STARTID 0x11   
#define                  MASTER_BOARD_ENDID   0x1A   

#define DEVICETYPE_INTABLE          0x1000
#define IDINFO_INTABLE              0x2000
#define READOUTPUTSTATE_INTABLE     0x3000
#define LAMPFAULTTYPE_INTABLE       0x3010
#define CRITICALFAULTPARAM_INTABLE  0x3020
#define FAULTDETBOARDSTATUS_TABLE   0x4002
#define FAULTDETBOARDFAULT_TABLE    0x4001
#define FAULTDETBOARDCONFLICT_TABLE 0x4000
#define BOARD_HARD_VERSION          0x1009
#define BOARD_SOFT_VERSION          0x100A

#define MANUFACTURE_KEDACOM			0x11220000
#define DEVTYPE_OPENATC_MAINCTL		0x11220010
#define DEVTYPE_OPENATC_FMU			0x11220011
#define DEVTYPE_OPENATC_LAMPCTL		0x11220020
#define DEVTYPE_OPENATC_IO			0x11220021
#define DEVTYPE_OPENATC_LDT			0x11220022

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

typedef struct tagIOBoardCardParam
{
    int            m_nRunStatus[C_N_MAXIOBOARD_NUM];     //运行状态
    uint32_t         m_nIOBoardCardID[C_N_MAXIOBOARD_NUM]; //IO板卡ID数组
}TIOBoardCardParam,*PTIOBoardCardParam;

typedef struct tagLampBoardCardParam
{
    int            m_nRunStatus[C_N_MAXLAMPBOARD_NUM];       //运行状态
    uint32_t         m_nLampBoardCardID[C_N_MAXLAMPBOARD_NUM]; //灯控板卡ID数组
}TLampBoardCardParam,*PTLampBoardCardParam;

typedef struct tagVehDetBoardCardParam
{
    int            m_nRunStatus[C_N_MAXDETBOARD_NUM];         //运行状态
    uint32_t         m_nVehDetBoardCardID[C_N_MAXDETBOARD_NUM]; //车检板卡ID数组
}TVehDetBoardCardParam,*PTVehDetBoardCardParam;

typedef struct tagFaultBoardCardParam
{
    int            m_nRunStatus;           //运行状态
    uint32_t         m_nFaultDetBoardCardID; //故障板卡ID数组
}TFaultDetBoardCardParam,*PTFaultDetBoardCardParam;

typedef list<int> LISTINT; 

typedef void(* PFN_AddOrEraseOneFaultMessage)(char, char, DWORD, DWORD, char, char);

typedef struct tagFaultProcCallBacks
{
    PFN_AddOrEraseOneFaultMessage pfnAddOneFaultMessage;
	PFN_AddOrEraseOneFaultMessage pfnEraseOneFaultMessage;
}TFaultProcCallBacks;

class CCanOpen;
class COpenATCCan;
class CCanProtocol;

#ifdef _WIN32
    #ifdef OpenATCCanBusProc_EXPORTS
    class _declspec(dllexport) CCanBusManager
    #else
    class _declspec(dllimport) CCanBusManager
    #endif
#else
    class CCanBusManager
#endif
{
public:
    //类定义为单件
    static CCanBusManager * getInstance();

    //类的初始化操作
    void Init(COpenATCParameter * pParameter, COpenATCRunStatus * pRunStatus, COpenATCLog * pOpenATCLog, TFaultProcCallBacks *pFaultcallback);

    //类的主流程
    void Work();

    //类的停止与释放
    void Stop();

private:
	CCanBusManager();
	~CCanBusManager();

	bool OpenCanDevice();                               //打开Can设备
 
	void CloseCanDevice();                              //关闭Can设备

	void ReceiveBoardCardData();                        //接收板卡数据

    void ReceiveLampBoardCardData(unsigned int nID, int nBoardIndex, unsigned char * pDataBuff, int nDataType);//接收灯控板卡数据
  
    void ReceiveVehDetBoardCardData(unsigned int nID, int nBoardIndex, unsigned char * pDataBuff, int nDataType);//接收车检板卡数据

    void ReceiveFaultDetBoardCardData(unsigned int nID);//接收故障板卡数据

    void ProcessFlashLampClrStatus(int nBoardIndex, int nIndex, char chColor, unsigned long nGlobalCounter, bool bReFresh, TCanData tCanData[], bool & bSync);  //处理闪灯状态
	
    bool GetCanData(int nBoardIndex, bool bReFresh, bool bGreenLampPulse[], bool bRedLampPulse[], unsigned long nGlobalCounter, unsigned char chLampClr[], TCanData tCanData[]);//获取灯控数据

    void SendDataToBoardCard(bool bReFresh, char chLampClr[], bool bGreenLampPulse[], bool bRedLampPulse[]);//发送数据到灯控板          

	int  SendBoardCardLampStatus();                     //发送灯控板卡状态

    inline void GetFlashLampClrStatus(TFlashLampClrStatus & tFlashLampClrStatus)
    {
        memcpy(&tFlashLampClrStatus,&m_tFlashLampClrStatus,sizeof(TFlashLampClrStatus));
    }
    //设置闪灯状态记录信息
    inline void SetFlashLampClrStatus(const TFlashLampClrStatus & tFlashLampClrStatus)
    {
        memcpy(&m_tFlashLampClrStatus,&tFlashLampClrStatus,sizeof(TFlashLampClrStatus));
    }
	inline void GetLampPulseStatus(TLampPulseStatus & tLampPulseStatus)
    {
        memcpy(&tLampPulseStatus,&m_tLampPulseStatus,sizeof(TLampPulseStatus));
    }
    //设置脉冲记录信息
    inline void SetLampPulseStatus(const TLampPulseStatus & tLampPulseStatus)
    {
        memcpy(&m_tLampPulseStatus,&tLampPulseStatus,sizeof(TLampPulseStatus));
    }

    void   ReadBoardCardInfo(unsigned int nID);//根据ID读板卡信息

    void   ReceiveBoardCardHeart(unsigned int nID);//根据ID读板卡心跳

    bool   AllBoardCardDetect();//对所有板卡进行自检，判断板卡是否都上线

	bool   ReadLampBoardFaultType(unsigned int nID, int nBoardIndex, unsigned int nSubIndex, unsigned char chVoltage, unsigned char chPower, unsigned char chFault[]);//读灯控板故障类型

	void   ReadLampBoardFault(unsigned int nID, int nBoardIndex, unsigned char * pDataBuff, unsigned long nGlobalCounter);//读灯控板故障

    bool   ProcessLampPulse(int nBoardIndex, int nIndex, char chColor, unsigned long nGlobalCounter, int & nLampStatus, bool & bLampPulse);//处理脉冲数据

	void   SetLampOn(int nIndex, char chColor, TCanData tCanData[]);//亮灯

    void   SetLampOff(int nIndex, char chColor, TCanData tCanData[]);//灭灯

	void   WriteCycleChgToFaultDetBoardCard();//告知故障检测板可以学习周期 

    int    GetBoardIndex(int nBoardType, unsigned int nID, unsigned char Buf[]);//获取板卡编号

	int    GetMasterBoardIndex(int nBoardType, unsigned int nID, unsigned char Buf[], bool bHaveSlave);//获取本机板卡编号

    int    GetSlaveBoardIndex(int nBoardType, unsigned int nID, unsigned char Buf[]);//获取级联板卡编号

    bool   GetBoardStartStatus(unsigned int nID);//获取板卡启动状态

	void   ReadDetectorFault(unsigned int nID, int nBoardIndex, unsigned int nSubIndex);//读检测器故障类型

    char   GetBit(unsigned int nInput, char chNum); //从一个值中取出任意位

	unsigned long CalcCounter(unsigned long nStart, unsigned long nEnd, unsigned long nMax);//计算计数  

	void   OpenATCSleep(long nMsec);//延时
            
    void   GetGreenConflictTable(int nChannelIndex, char chGreenConflictInfo[MAX_CHANNEL_COUNT][MAX_CHANNEL_COUNT], unsigned char chSendBuff[]);//获取绿冲突故障表

	void   InitLampBoardCtrParam();

	void   InitDetectorParam();

public:
	void   ReceiveIOBoardCardData(unsigned int nID, int nBoardIndex, unsigned char * pDataBuff, int nDataType);//接收IO板卡数据

	bool   WriteLampBoardCtrParam(int nBoardIndex, int nIndex);//按照通道向灯控板写控制参数
	
    bool   StartDetVedetector(int nBoardIndex, int nIndex);//启动检测器
	
    void   SelfDetect();//设置上线的灯控板卡状态

    void   PullInRelay(int nTimer);//发送指令到故障检测板，故障检测板去驱动主继电器

    bool   ReadFaultDetBoardCardStatus();//读故障版驱动状态

    void   StopHardWareResource();//发送指令到故障检测板，故障检测板停止驱动主继电器

    void   ClearFaultDetBoardFaultStatus();//清除故障板故障状态

    void   SendGreenConflicitTableToFaultDetBoard();//发送绿冲突表给故障检测板

	void   ClearFaultDetBoardFaultStudyStatus();//清除故障检测板的学习状态

	void   ReadFaultCodeFromFaultDetBoard();

	void   ReadBoardVersion(BYTE byBoardType, unsigned int nID);

	void   ReadChannelVoltageAndPower(unsigned int nID, int nBoardIndex, int nIndex);

	bool   WriteCriticalFaultParamToFaultDetBoard();

    CCanOpen*       GetCanOpen();

    COpenATCCan*    GetOpenATCCan();

    CCanProtocol*   GetMainQueue();

    CCanProtocol*   GetSlaveQueue();

	void SetCAN1LedStatus(bool bStatus);//设置can1总线指示灯状态

private:

    static CCanBusManager * s_pData;                    //单件类指针

    COpenATCParameter * m_pLogicCtlParam;               //特征参数类指针

    COpenATCRunStatus * m_pLogicCtlStatus;              //运行状态类指针

	COpenATCLog       * m_pOpenATCLog;                  //日志类指针

    TIOBoardCardParam         m_tIOBoardCardParam;    //IO板卡

    TLampBoardCardParam       m_tLampBoardCardParam;  //灯控板卡

    TVehDetBoardCardParam     m_tVehDetBoardCardParam;//车检板卡
   
    TFaultDetBoardCardParam   m_tFaultDetBoardCardParam; //故障板卡

	TIOBoardData              m_tIOBoardData;           //IO板卡数据

	TLampCltBoardData         m_tLampCltBoardData;      //灯控板卡数据

	TVehDetBoardData          m_tVehDetBoardData;       //车检板卡数据

    TFaultDetBoardData        m_tFaultDetBoardData;     //故障检测板卡数据

	bool                      m_bOpenCanDevice[CAN_MAX_NUM];//Can是否打开成功
 
    unsigned long             m_nSendBoardDataTime[C_N_MAXLAMPBOARD_NUM];     //最近一次发生数据到Can的时间   
   
    TFlashLampClrStatus       m_tFlashLampClrStatus;    //闪灯状态记录 
 
    COpenATCCommWithCanReceiveThread   * m_openATCCommWithCanReceiveThread;//Can接收数据线程

    COpenATCCommWithCanSendThread   * m_openATCCommWithCanSendThread;//Can发送数据线程

    unsigned long             m_nStartHardWareResourceCounter; //发送驱动主继电器指令到故障检测板的时间

	unsigned long             m_nReadFaultCodeFromFaultDetBoardCounter; //从故障检测板读故障代码的时间

	void                      *m_SDOHandle[C_N_MAXLAMPBOARD_NUM];  //发送SDO数据到灯控板的句柄
	
	TLampPulseStatus          m_tLampPulseStatus;      //脉冲状态记录   

	int                       m_nYellowFlashCount[C_N_MAXLAMPOUTPUT_NUM];  //黄闪计数

    int                       m_nLampFlashCount[C_N_MAXLAMPOUTPUT_NUM];  //绿闪计数

    bool                      m_bFaultDetBoardDriveStatus;//故障板驱动状态

	bool                      m_bLampBoardCtrParamInit[C_N_MAXLAMPBOARD_NUM][C_N_CHANNELNUM_PER_BOARD];//灯组参数是否初始化成功

	bool                      m_bDetectorInit[C_N_MAXDETBOARD_NUM][C_N_MAXDETINPUT_NUM];//检测器参数是否初始化成功

	bool                      m_bPlugStatus[C_N_MAXLAMPBOARD_NUM]; 

    TFaultProcCallBacks* m_faultcallback;

public:
	 CCanOpen*                 m_CanOpen;
 
};

#endif // !ifndef CANBUSMANAGER_H
