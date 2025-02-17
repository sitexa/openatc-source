/*=====================================================================
模块名 ：定周期控制方式实现模块
文件名 ：LogicCtlFixedTime.h
相关文件：
实现功能：用于定义定周期控制模式实现接口
作者 ：刘黎明
版权 ：<Copyright(c) 2019-2020 Suzhou Keda Technology Co.,Ltd. All right reserved.>
-----------------------------------------------------------------------
修改记录：
日 期           版本     修改人     走读人     修改记录
2019/9/26       V1.0     刘黎明     刘黎明     创建模块
=====================================================================*/

#ifndef LOGICCTLFIXEDTIME_H
#define LOGICCTLFIXEDTIME_H

#include "LogicCtlMode.h"

typedef struct tagPhaseCtlInfo
{
    TPhase m_tPhaseParam;                                               //相位信息
    WORD m_wPhaseTime;                                                  //相位绿信比
    char m_chPhaseMode;                                                 //相位绿信比模式

    WORD m_wPhaseGreenTime;                                             //相位绿灯时间(扣除绿闪、黄灯和红灯,机动车)
    char m_chPhaseStage;                                                //相位当前运行阶段(机动车)
    WORD m_wPhaseStageRunTime;                                          //相位当前阶段的运行时间(机动车)
    char m_chPhaseStatus;                                               //相位当前状态(机动车:等待态,转换态,运行态,结束态)

    WORD m_wPedPhaseGreenTime;                                          //相位绿灯时间(扣除黄灯和红灯,行人)
    char m_chPedPhaseStage;                                             //相位当前运行阶段(行人)
    WORD m_wPedPhaseStageRunTime;                                       //相位当前阶段的运行时间(行人)
    char m_chPedPhaseStatus;                                            //相位当前状态(行人:转换态，运行态，结束态)

    bool m_bIsPedAskPhase;                                              //是否是行人过街请求相位
}TPhaseCtlInfo,*PTPhaseCtlInfo;

typedef struct tagRingParam
{
    int m_nPhaseCount;                                                  //环内相位数量
    TPhaseCtlInfo m_atPhaseInfo[MAX_SEQUENCE_TABLE_COUNT];              //环相位数组
}TRingCtlInfo,*PTRingCtlInfo;

typedef struct tagOverlapCtlInfo
{
    TOverlapTable m_tOverlapParam;                                      //跟随相位参数
    char m_chOverlapStage;                                              //跟随相位的运行阶段
	char m_chPedOverlapStage;                                           //跟随行人相位的运行阶段                                    
}TOverlapCtlInfo,*PTOverlapCtlInfo;

typedef struct tagFixTimeCtlParam
{
    int m_nRingCount;                                                   //环数量
    TRingCtlInfo m_atPhaseSeq[MAX_RING_COUNT];                          //环控制参数数组
    int m_nCurPhaseIndex[MAX_RING_COUNT];                               //环内当前运行的相位索引
    int m_nCurStageIndex;                                               //环内当前运行的阶段索引
//    int m_bIsChgPhase[MAX_RING_COUNT];                                //环内是否要切换相位
    bool m_bIsChgStage[MAX_RING_COUNT];                                 //相位内是否切换灯色
    bool m_bIsChgPedStage[MAX_RING_COUNT];                              //行人相位内是否切换灯色
    int m_nOverlapCount;                                                //跟谁相位数量
    TOverlapCtlInfo m_atOverlapInfo[MAX_OVERLAP_COUNT];                 //跟随相位表
//    int m_nChannelCount;                                              //通道数量
//    TChannel m_atChannelInfo[MAX_CHANNEL_COUNT];                      //通道数组 
    int m_nVehDetCount;                                                 //检测器数量
    TVehicleDetector m_atVehDetector[MAX_VEHICLEDETECTOR_COUNT];        //车辆检测器表
    int m_nPedDetCount;                                                 //行人检测器数量
	TPedestrianDetector m_atPedDetector[MAX_PEDESTRIANDETECTOR_COUNT];  //行人检测器表
    WORD m_wCycleLen;                                                   //周期长 
    WORD m_wPhaseOffset;                                                //相位差
    WORD m_wCycleRunTime;                                               //当前周期运行时间
	WORD m_wPatternNumber;                                              //方案号
}TFixTimeCtlInfo,*PTFixTimeCtlInfo;

#define  SEND_INIT     0
#define  SEND_SRART    1
#define  SEND_END      2

typedef struct tagPhasePulseStatus
{
	int  m_nPhaseIndex;                                                //相位编号
	int  m_nPhaseNum;                                                  //相位ID
	bool m_bGreenPulseStatus;                                          //绿脉冲状态
	bool m_bRedPulseStatus;                                            //红脉冲状态
	int  m_nGreenPulseSendStatus;                                      //绿脉冲发送过程状态
	int  m_nRedPulseSendStatus;                                        //红脉冲发送过程状态
}TPhasePulseStatus,*PTPhasePulseStatus;

/*=====================================================================
类名 ：CLogicCtlFixedTime
功能 ：定周期控制方式实现类
主要接口：Run
          Init
备注 ：
-----------------------------------------------------------------------
修改记录：
日 期          版本     修改人     走读人       修改记录
2019/09/14     V1.0     刘黎明     刘黎明       创建类
2020/2/26      V1.0     李永萍     李永萍       增加脉冲处理
2020/3/26      V1.0     李永萍     李永萍       生成相位运行阶段表和环内阶段切换状态判断更新
2020/3/26      V1.0     李永萍     李永萍       找到每个环的屏障前的最后一个相位
=====================================================================*/
class CLogicCtlFixedTime : public CLogicCtlMode  
{
public:
	CLogicCtlFixedTime();
	virtual ~CLogicCtlFixedTime();

    //初始化定周期控制方式需要的参数
    virtual void Init(COpenATCParameter * pParameter, COpenATCRunStatus * pRunStatus, COpenATCLog * pOpenATCLog, int nPlanNo);
    //定周期控制方式主流程
    virtual void Run();

    //获取当前使用的控制参数及控制状态
    virtual void * GetCurCtlParam(); 

    //设置运行状态
    virtual void GetPhaseRunStatus(TPhaseRunStatus & tRunStatus);

    //设置显示屏显示信息状态
    virtual void GetLedScreenShowInfo(TLedScreenShowInfo & tLedScreenShowInfo);

	//判断当前通道的灯色是否是绿色
    virtual bool IsChannelGreen(BYTE byChannelType, int nDirectionIndex, int nChannelLockStatus[]);

public:
	bool				m_bGreenTimeExceedMax1Time[MAX_PHASE_COUNT];							//用于相位绿灯时间是否达到最大绿1
	int					m_nGreenTimeIsMax1Time[MAX_PHASE_COUNT];								//用于相位绿灯时间是否连续两次达到最大绿1
	int					m_nGreenTimeNotExceedMax1Time[MAX_PHASE_COUNT];							//用于相位绿灯时间是否连续两次未达到最大绿1

protected:
    //获取时段参数
    virtual void InitByTimeSeg(BYTE & byPlanID);

    //获取指定方案参数
    virtual void InitByPlan(BYTE byPlanID);

    //初始化跟随相位参数
    virtual void InitOverlapParam();

    //初始化检测器参数
    virtual void InitDetectorParam();

    //初始化行人检测器参数
    virtual void InitPedDetectorParam();

    //环内机动车相位的运行状态更新
    virtual void OnePhaseRun(int nRingIndex);

    //环内行人相位的运行状态更新
    virtual void OnePedPhaseRun(int nRingIndex);

    //环内相位切换状态判断更新
    virtual void PhaseChg(int nRingIndex);

    //多环周期运行结束判断更新
    virtual void CycleChg();

    //周期结束重置控制状态
    virtual void ReSetCtlStatus();

    //根据当前的机动车相位运行阶段，判断下一个非0的运行阶段和阶段时间
    virtual char GetNextPhaseStageInfo(char chCurStage,const PTPhaseCtlInfo pPhaseInfo,WORD & nStageTime);

    //根据当前的行人相位运行阶段，判断下一个非0的运行阶段和阶段时间
    virtual char GetPedNextPhaseStageInfo(char chCurStage,const PTPhaseCtlInfo pPhaseInfo,WORD & nStageTime);

    //获取相位剩余运行时间
    virtual int  GetPhaseRemainTime(TPhaseCtlInfo * pPhaseCtlInfo);

    virtual void SetGreenLampPulse(int nPhaseType, BYTE byPhaseNumber, bool bState);

    virtual void SetRedLampPulse(int nPhaseType, BYTE byNextPhaseNumber, bool bState);

	virtual void SetOverLapGreenPulse(int nRingIndex, int nIndex, bool bManual, int nNextStage, bool bState);

	virtual void SetOverLapRedPulse(int nRingIndex, int nIndex, bool bManual, int nNextStage, bool bState);

	virtual void SetOverLapPulse(bool bManual, int nNextStageIndex);

	virtual void GetGreenFalshCount(int nPhaseType, TPhaseCtlInfo * pPhaseCtlInfo, int nSecond);

    virtual int  GetPhaseCycleRemainTime(int nRingIndex, TPhaseCtlInfo * pPhaseCtlInfo, TPhaseLampClrRunCounter tRunCounter);

    virtual int  GetPedPhaseCycleRemainTime(int nRingIndex, TPhaseCtlInfo * pPhaseCtlInfo, TPhaseLampClrRunCounter tRunCounter);

    //根据相位号查找该相位所在的环号和环内顺序号
    bool GetPhaseIndexByPhaseNumber(int & nRingIndex,int & nPhaseIndex,int nPhaseNum);

    //设置一个Ring的相位灯色
    void SetLampClrByRing(int nRingIndex, int nPhaseIndex, char chPhaseStage, char chPedPhaseStage);

    //设置相位对应的通道的状态
    void SetChannelStatus(int nPhaseNum, int nPhaseSrc, char chPhaseStage, char chPedPhaseStage);

    //依据阶段设置跟随相位灯色
    void SetOverlapPhaseLampClr(int nNextStageIndex);

    //依据时序设置跟随相位灯色
    void SetOverlapPhaseLampClr();

    //当前相位是否是跟随相位的目相位
    bool IsPhaseNumInOverlap(int nPhaseNum,int nOverlapIndex);
    //获取指定相位当前的状态 
    char GetPhaseStatus(int nPhaseNum, bool bPedPhase);

    //根据通道参数初始化通道状态
    void RetsetAllChannelStatus();

    //判断相位对应的车检器是否全部故障
    bool ProcPhaseDetStatus(int nRingIndex,int nPhaseIndex); 

    //获取相位运行阶段表
    void GetRunStageTable();

    //环内阶段切换状态判断更新
    bool StageChg(int nRingIndex);

    //找到每个环的屏障前的最后一个相位
    void GetLastPhaseBeforeBarrier();

    //给跟随相位发送脉冲
    void SendOverlapPhasePulse(int nRingIndex, int nCurPhaseNum, int nPhaseNum, bool bRedPulse, BYTE byOverlapNum[], bool bSendPulse[], bool bManual, int nNextStage);

    //根据通道对应的相位设置通道对应的绿信比模式
    void SetChannelSplitMode();

	//获取跟随相位的状态 
    char GetOverlapType(char chOverlapStatus);

    //根据通道对应的相位设置通道对应的屏蔽和禁止状态
    void SetChannelShieldAndProhibitStatus();

    //相位禁止屏蔽设置了立马得呈现
    void SetLampByShieldAndProhibitStatus(int nRingNum, int nPhaseCount, int nChannelCount);

	bool IsNeglectPhase(int nPhaseID);

    //判断，对应的相位是否并发
    bool IsConcurrencyPhase(int nRingIndex, int nDstRingIndex);

    //根据m_bySplitPhaseMode和m_nSplitPhaseTime重新生成相序
    void SetSeqByStageInfo();

    //判断两个相位是否并发
    void InitPhaseConcurrencyTable();

    TFixTimeCtlInfo m_tFixTimeCtlInfo;												//单点定周期控制需要的特征参数和控制状态信息

    bool m_bLastPhaseBeforeBarrier[MAX_RING_COUNT][MAX_SEQUENCE_TABLE_COUNT];		//每个环的相位是不是屏障前的最后一个相位

    unsigned long   m_nChannelDurationTime[MAX_CHANNEL_COUNT];						//手动控制时每个通道灯色持续时间

    int             m_nSplitTime[MAX_RING_COUNT][MAX_SEQUENCE_TABLE_COUNT];			//用于记录自适应，无电缆和行人过街模式下的绿信比时间

    short           m_nRealOffset;													//实际相位差

    TSplit          m_atSplitInfo[MAX_PHASE_COUNT];									//绿信比表

	TPhasePulseStatus m_tPhasePulseStatus[MAX_RING_COUNT];                          //每个环的机动车相位发送脉冲标志

	BYTE            m_byPlanID;                                                     //方案号

    BYTE            m_bySplitPhaseMode[MAX_RING_COUNT][MAX_PHASE_COUNT];            //相位是否忽略 0：正常相位，1：相位忽略，2：相位忽略部分

    int             m_nSplitPhaseTime[MAX_RING_COUNT][MAX_PHASE_COUNT];             //当相位忽略部分时，忽略的相位时间

    int m_nPhaseConcurInfo[MAX_PHASE_COUNT][MAX_PHASE_COUNT];			            //相位并发表
};

#endif // !ifndef LOGICCTLFIXEDTIME_H
