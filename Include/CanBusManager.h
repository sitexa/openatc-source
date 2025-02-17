/*=====================================================================
ģ���� ��CANͨ�Ź���ģ��
�ļ��� ��CanBusManager.h
����ļ���OpenATCParameter.h OpenATCRunStatus.h
ʵ�ֹ��ܣ��������ذ�͵ƿذ壬I0�壬�����͹��ϰ�֮ǰ��CANͨ�š�
���� ��������
��Ȩ ��<Copyright(c) 2019-2020 Suzhou Keda Technology Co.,Ltd. All right reserved.>
-----------------------------------------------------------------------
�޸ļ�¼��
�� ��           �汾     �޸���     �߶���     �޸ļ�¼
2019/9/26       V1.0     ����Ƽ     ����Ƽ     ����ģ��
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

typedef struct tagIOBoardCardParam
{
    int            m_nRunStatus[C_N_MAXIOBOARD_NUM];     //����״̬
    uint32_t         m_nIOBoardCardID[C_N_MAXIOBOARD_NUM]; //IO�忨ID����
}TIOBoardCardParam,*PTIOBoardCardParam;

typedef struct tagLampBoardCardParam
{
    int            m_nRunStatus[C_N_MAXLAMPBOARD_NUM];       //����״̬
    uint32_t         m_nLampBoardCardID[C_N_MAXLAMPBOARD_NUM]; //�ƿذ忨ID����
}TLampBoardCardParam,*PTLampBoardCardParam;

typedef struct tagVehDetBoardCardParam
{
    int            m_nRunStatus[C_N_MAXDETBOARD_NUM];         //����״̬
    uint32_t         m_nVehDetBoardCardID[C_N_MAXDETBOARD_NUM]; //����忨ID����
}TVehDetBoardCardParam,*PTVehDetBoardCardParam;

typedef struct tagFaultBoardCardParam
{
    int            m_nRunStatus;           //����״̬
    uint32_t         m_nFaultDetBoardCardID; //���ϰ忨ID����
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
    //�ඨ��Ϊ����
    static CCanBusManager * getInstance();

    //��ĳ�ʼ������
    void Init(COpenATCParameter * pParameter, COpenATCRunStatus * pRunStatus, COpenATCLog * pOpenATCLog, TFaultProcCallBacks *pFaultcallback);

    //���������
    void Work();

    //���ֹͣ���ͷ�
    void Stop();

private:
	CCanBusManager();
	~CCanBusManager();

	bool OpenCanDevice();                               //��Can�豸
 
	void CloseCanDevice();                              //�ر�Can�豸

	void ReceiveBoardCardData();                        //���հ忨����

    void ReceiveLampBoardCardData(unsigned int nID, int nBoardIndex, unsigned char * pDataBuff, int nDataType);//���յƿذ忨����
  
    void ReceiveVehDetBoardCardData(unsigned int nID, int nBoardIndex, unsigned char * pDataBuff, int nDataType);//���ճ���忨����

    void ReceiveFaultDetBoardCardData(unsigned int nID);//���չ��ϰ忨����

    void ProcessFlashLampClrStatus(int nBoardIndex, int nIndex, char chColor, unsigned long nGlobalCounter, bool bReFresh, TCanData tCanData[], bool & bSync);  //��������״̬
	
    bool GetCanData(int nBoardIndex, bool bReFresh, bool bGreenLampPulse[], bool bRedLampPulse[], unsigned long nGlobalCounter, unsigned char chLampClr[], TCanData tCanData[]);//��ȡ�ƿ�����

    void SendDataToBoardCard(bool bReFresh, char chLampClr[], bool bGreenLampPulse[], bool bRedLampPulse[]);//�������ݵ��ƿذ�          

	int  SendBoardCardLampStatus();                     //���͵ƿذ忨״̬

    inline void GetFlashLampClrStatus(TFlashLampClrStatus & tFlashLampClrStatus)
    {
        memcpy(&tFlashLampClrStatus,&m_tFlashLampClrStatus,sizeof(TFlashLampClrStatus));
    }
    //��������״̬��¼��Ϣ
    inline void SetFlashLampClrStatus(const TFlashLampClrStatus & tFlashLampClrStatus)
    {
        memcpy(&m_tFlashLampClrStatus,&tFlashLampClrStatus,sizeof(TFlashLampClrStatus));
    }
	inline void GetLampPulseStatus(TLampPulseStatus & tLampPulseStatus)
    {
        memcpy(&tLampPulseStatus,&m_tLampPulseStatus,sizeof(TLampPulseStatus));
    }
    //���������¼��Ϣ
    inline void SetLampPulseStatus(const TLampPulseStatus & tLampPulseStatus)
    {
        memcpy(&m_tLampPulseStatus,&tLampPulseStatus,sizeof(TLampPulseStatus));
    }

    void   ReadBoardCardInfo(unsigned int nID);//����ID���忨��Ϣ

    void   ReceiveBoardCardHeart(unsigned int nID);//����ID���忨����

    bool   AllBoardCardDetect();//�����а忨�����Լ죬�жϰ忨�Ƿ�����

	bool   ReadLampBoardFaultType(unsigned int nID, int nBoardIndex, unsigned int nSubIndex, unsigned char chVoltage, unsigned char chPower, unsigned char chFault[]);//���ƿذ��������

	void   ReadLampBoardFault(unsigned int nID, int nBoardIndex, unsigned char * pDataBuff, unsigned long nGlobalCounter);//���ƿذ����

    bool   ProcessLampPulse(int nBoardIndex, int nIndex, char chColor, unsigned long nGlobalCounter, int & nLampStatus, bool & bLampPulse);//������������

	void   SetLampOn(int nIndex, char chColor, TCanData tCanData[]);//����

    void   SetLampOff(int nIndex, char chColor, TCanData tCanData[]);//���

	void   WriteCycleChgToFaultDetBoardCard();//��֪���ϼ������ѧϰ���� 

    int    GetBoardIndex(int nBoardType, unsigned int nID, unsigned char Buf[]);//��ȡ�忨���

	int    GetMasterBoardIndex(int nBoardType, unsigned int nID, unsigned char Buf[], bool bHaveSlave);//��ȡ�����忨���

    int    GetSlaveBoardIndex(int nBoardType, unsigned int nID, unsigned char Buf[]);//��ȡ�����忨���

    bool   GetBoardStartStatus(unsigned int nID);//��ȡ�忨����״̬

	void   ReadDetectorFault(unsigned int nID, int nBoardIndex, unsigned int nSubIndex);//���������������

    char   GetBit(unsigned int nInput, char chNum); //��һ��ֵ��ȡ������λ

	unsigned long CalcCounter(unsigned long nStart, unsigned long nEnd, unsigned long nMax);//�������  

	void   OpenATCSleep(long nMsec);//��ʱ
            
    void   GetGreenConflictTable(int nChannelIndex, char chGreenConflictInfo[MAX_CHANNEL_COUNT][MAX_CHANNEL_COUNT], unsigned char chSendBuff[]);//��ȡ�̳�ͻ���ϱ�

	void   InitLampBoardCtrParam();

	void   InitDetectorParam();

public:
	void   ReceiveIOBoardCardData(unsigned int nID, int nBoardIndex, unsigned char * pDataBuff, int nDataType);//����IO�忨����

	bool   WriteLampBoardCtrParam(int nBoardIndex, int nIndex);//����ͨ����ƿذ�д���Ʋ���
	
    bool   StartDetVedetector(int nBoardIndex, int nIndex);//���������
	
    void   SelfDetect();//�������ߵĵƿذ忨״̬

    void   PullInRelay(int nTimer);//����ָ����ϼ��壬���ϼ���ȥ�������̵���

    bool   ReadFaultDetBoardCardStatus();//�����ϰ�����״̬

    void   StopHardWareResource();//����ָ����ϼ��壬���ϼ���ֹͣ�������̵���

    void   ClearFaultDetBoardFaultStatus();//������ϰ����״̬

    void   SendGreenConflicitTableToFaultDetBoard();//�����̳�ͻ������ϼ���

	void   ClearFaultDetBoardFaultStudyStatus();//������ϼ����ѧϰ״̬

	void   ReadFaultCodeFromFaultDetBoard();

	void   ReadBoardVersion(BYTE byBoardType, unsigned int nID);

	void   ReadChannelVoltageAndPower(unsigned int nID, int nBoardIndex, int nIndex);

	bool   WriteCriticalFaultParamToFaultDetBoard();

    CCanOpen*       GetCanOpen();

    COpenATCCan*    GetOpenATCCan();

    CCanProtocol*   GetMainQueue();

    CCanProtocol*   GetSlaveQueue();

	void SetCAN1LedStatus(bool bStatus);//����can1����ָʾ��״̬

private:

    static CCanBusManager * s_pData;                    //������ָ��

    COpenATCParameter * m_pLogicCtlParam;               //����������ָ��

    COpenATCRunStatus * m_pLogicCtlStatus;              //����״̬��ָ��

	COpenATCLog       * m_pOpenATCLog;                  //��־��ָ��

    TIOBoardCardParam         m_tIOBoardCardParam;    //IO�忨

    TLampBoardCardParam       m_tLampBoardCardParam;  //�ƿذ忨

    TVehDetBoardCardParam     m_tVehDetBoardCardParam;//����忨
   
    TFaultDetBoardCardParam   m_tFaultDetBoardCardParam; //���ϰ忨

	TIOBoardData              m_tIOBoardData;           //IO�忨����

	TLampCltBoardData         m_tLampCltBoardData;      //�ƿذ忨����

	TVehDetBoardData          m_tVehDetBoardData;       //����忨����

    TFaultDetBoardData        m_tFaultDetBoardData;     //���ϼ��忨����

	bool                      m_bOpenCanDevice[CAN_MAX_NUM];//Can�Ƿ�򿪳ɹ�
 
    unsigned long             m_nSendBoardDataTime[C_N_MAXLAMPBOARD_NUM];     //���һ�η������ݵ�Can��ʱ��   
   
    TFlashLampClrStatus       m_tFlashLampClrStatus;    //����״̬��¼ 
 
    COpenATCCommWithCanReceiveThread   * m_openATCCommWithCanReceiveThread;//Can���������߳�

    COpenATCCommWithCanSendThread   * m_openATCCommWithCanSendThread;//Can���������߳�

    unsigned long             m_nStartHardWareResourceCounter; //�����������̵���ָ����ϼ����ʱ��

	unsigned long             m_nReadFaultCodeFromFaultDetBoardCounter; //�ӹ��ϼ�������ϴ����ʱ��

	void                      *m_SDOHandle[C_N_MAXLAMPBOARD_NUM];  //����SDO���ݵ��ƿذ�ľ��
	
	TLampPulseStatus          m_tLampPulseStatus;      //����״̬��¼   

	int                       m_nYellowFlashCount[C_N_MAXLAMPOUTPUT_NUM];  //��������

    int                       m_nLampFlashCount[C_N_MAXLAMPOUTPUT_NUM];  //��������

    bool                      m_bFaultDetBoardDriveStatus;//���ϰ�����״̬

	bool                      m_bLampBoardCtrParamInit[C_N_MAXLAMPBOARD_NUM][C_N_CHANNELNUM_PER_BOARD];//��������Ƿ��ʼ���ɹ�

	bool                      m_bDetectorInit[C_N_MAXDETBOARD_NUM][C_N_MAXDETINPUT_NUM];//����������Ƿ��ʼ���ɹ�

	bool                      m_bPlugStatus[C_N_MAXLAMPBOARD_NUM]; 

    TFaultProcCallBacks* m_faultcallback;

public:
	 CCanOpen*                 m_CanOpen;
 
};

#endif // !ifndef CANBUSMANAGER_H
