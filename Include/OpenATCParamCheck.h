/*=====================================================================
模块名 ：信号机配置参数合法性校验类
文件名 ：OpenATCParamCheck.h
相关文件：OpenATCParamCheck.cpp
实现功能：用于信号机特征参数配置的校验
作者 ：梁厅
版权 ：<Copyright(c) 2019-2020 Suzhou Keda Technology Co.,Ltd. All right reserved.>
---------------------------------------------------------------------------------------------------------------------
修改记录：
日 期           版本     修改人     走读人     修改记录
2019/9/14       V1.0     梁厅        王五      创建模块
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

	//信号机特征参数校验
	int CheckAscParam(unsigned char* pData);

	//信号机设备参数校验
	int CheckSystemDeviceInfo_SiteID(unsigned char* pData);

	//信号机设备参数校验
	int CheckSystemDeviceInfo_Other(unsigned char* pData, bool bClearErrFlag);

	//手动面板按键通道绿冲突校验
	int CheckGreenConflictByPanel(unsigned char *pData, int nSize);

	//MD5码校验
	bool CheckMd5Code(unsigned char * pdataJsonBuffer, int nDataJsonBufferSize);

	//地址码检验
	bool CheckSiteID(unsigned char* pdataJsonBuffer, int nDataJsonBufferSize, int nSiteID, bool bDeviceParamFlag);

	//获取错误码
	void GetErrCode(TErrInfo(&atErrCodeTable)[C_N_MAX_ERR_SIZE]);

	//通道锁定下发命令绿冲突校验
	int  CheckGreenConflictByChannelLock(int(&nChannelStatus)[MAX_CHANNEL_COUNT], unsigned char* pData, int nSize, char tChannelGreenConflictTable[MAX_CHANNEL_COUNT][MAX_CHANNEL_COUNT],BYTE* tChannelNum);

    //校验部分下载的参数是否正确
	int  CheckPartParam(BYTE byParamType, unsigned char* pData, unsigned char* pParamBuffer);

	//设置校验结果
	void SetCheckTranErrCode(int nDataValue, int nSubDataValue);

	void SetCheckTranErrCode(int nDataValue, int nSubDataValue, int nSubDataValueTwo);

private:

	static COpenATCParamCheck *m_pParamCheck;

	//相位信息参数校验
	void CheckPhaseInfo(cJSON* cPhaseObj);

	//环信息参数校验
	void CheckRingInfo();

	//检验跟随相位信息
	void CheckOverlapInfo(cJSON* cOverlapObj);

	//方案信息参数校验
	void CheckPatternInfo(cJSON* cPatternObj, int nPatternSize);

	//时段信息参数校验
	void CheckDayPlanInfo(cJSON* cDayPlanObj);

	//调度计划参数校验
	void CheckScheduleInfo(cJSON* cScheduleObj);

	//通道信息参数校验
	void CheckChannelInfo(cJSON* cChannelObj);

	//通道锁定信息参数校验
	void CheckChannelLockInfo(cJSON* cChannelObj, int nTimeSegIndex);

	//车辆检测器参数校验
	void CheckVechDetectorInfo(cJSON* cVechDetectorObj);

	//行人检测器参数校验
	void CheckPedDetectorInfo(cJSON* cPedDetectorObj);

	//自适应参数校验 2021.12.13
	void CheckSingleOptimInfo(cJSON* cSingleOpeimObj);

	//手动面板参数校验
	void CheckManualPanelInfo(cJSON* cManualPanelObj);

	//设备信息(siteid)校验
	void CheckCustomInfo_SiteID(cJSON* cCustomInfoObj);

	//设备信息(除siteid外的信息)校验
	void CheckCustomInfo_Other(cJSON* cCustomInfoObj);

	//并发相位校验
	int CheckConcurrentPhase();

	//对所有方案进行相位并发冲突校验
	void CheckConcurrentConflictByPhase();

	//初始化并发矩阵表、跟随相位表 、通道表
	void InitCheckInfo(unsigned char *pData, int nSize);

	//根据通道ID反查通道编号
	int GetChannelNum(int nChannelID);

	int GetChannelNum(int nChannelID, BYTE* tChannelNum);

	//根据并发 跟随 通道信息初始换通道绿冲突表
	void InitGreenConflictbyChannel();

	//生成所有方案的阶段表
	void GetPatternStageTable(int nPatternIndex);

	//根据时间段获得该时段内该执行的方案号
	void GetPatternByDayPlan(int nStartTime, int nEndTime, BYTE(&abyPattern)[MAX_PATTERN_COUNT]);

	//值域校验
	int CheckValueRange(int nDataValue, int nMinValue, int nMaxValue);

	//判断相位是否绿冲突
	bool CheckConcurrentConflictByPhaseConcurInfo(int nPatternIndex);

	//判断是否为有效ip
	bool is_valid_ip(char *ip);

	//如果有下发冲突表，则提取相关的通道冲突信息来进行校验
	void GetChannelGreenConflictInfo(cJSON* cGreenConflictObj);

	//根据下发的相位，跟随相位等信息生成默认的通道冲突表，用于校验
	void CreateGreenConflictbyChannel(char tChannelGreenConflictTable[MAX_CHANNEL_COUNT][MAX_CHANNEL_COUNT]);

	//获取已配相位编号和相位并发表 --用于单独下发方案表校验所需参数
	void GetCheckPatternRequiredParameters(unsigned char* pParamBuffer);

	//获取已配方案编号 --用于单独下发计划表校验所需参数
	void GetCheckplanListRequiredParameters(unsigned char* pParamBuffer);

	//根据新的计划表信息对日期表进行校验
	void CheckDateListByNewPlanList(unsigned char* pParamBuffer);

	//获取已配计划编号 --用于单独下发日期表校验所需参数
	void GetCheckdateListRequiredParameters(unsigned char* pParamBuffer);

	int m_nPhaseCfgTable[MAX_PHASE_COUNT];										//已配相位编号

	int m_n0verlapPhaseTable[MAX_OVERLAP_COUNT];								//已配跟随相位编号

	int m_nPatternCfgTable[MAX_PATTERN_COUNT];									//已配方案编号

	int m_nPlanCfgTable[MAX_DAYPLAN_TABLE_COUNT];								//已配置的计划编号

	BYTE m_byCoorFlag[MAX_DAYPLAN_TABLE_COUNT];									//对应的计划是否为协调控制计划，1为是协调计划，0为非协调计划

	BYTE m_byMonthWeek[12][7];													//非协调计划已配月周

	BYTE m_byMonthDay[12][31];													//非协调计划已配月日

	int m_nPatternSize;															//已配方案数量

	int m_nDayPlanCfgSize;														//已配时段数量

	int m_nSplitLimitTable[MAX_PHASE_COUNT];									//绿信比下限值表

	int m_nSplitTopLimitTable[MAX_PHASE_COUNT];									//绿信比上限值表

	int m_nSplitMode[MAX_PHASE_COUNT];											//绿信比模式

	int m_nCurChannelTable[MAX_CHANNEL_COUNT];									//已配通道编号

	int m_nErrCodeIndex;				

	int m_nCheckPhaseConcurInfo[MAX_PHASE_COUNT][MAX_PHASE_COUNT];				//待校验并发

	char m_nCheckChannelGreenConflict[MAX_CHANNEL_COUNT][MAX_CHANNEL_COUNT];	//待校验通道绿冲突,0表示不能同时亮绿灯,1表示可以同时亮绿灯

	char m_achChannelGreenConflictInfo[MAX_CHANNEL_COUNT][MAX_CHANNEL_COUNT];	//通道绿冲突信息表,0表示不能同时亮绿灯,1表示可以同时亮绿灯

	TAscParam m_atAscParamInfo;

	int m_nChannelSize;															//已配置通道数

	TChannel m_atChannelInfo[MAX_CHANNEL_COUNT];								//通道信息表

	int		 m_atChannelType[MAX_CHANNEL_COUNT];								//通道属性表

	int		 m_atChannelSource[MAX_CHANNEL_COUNT];								//通道控制源

	int m_nPhaseConcurInfo[MAX_PHASE_COUNT][MAX_PHASE_COUNT];					//并发矩阵

	TOverlapTable m_atAscOverlapTable[MAX_OVERLAP_COUNT];						//跟随相位表

	int m_atOverlapIncludePhases[MAX_OVERLAP_COUNT][MAX_PHASE_COUNT_IN_OVERLAP];//跟随相位包含的相位号

	TPatternInfo  m_atPatternInfo[MAX_PATTERN_COUNT];							//所有方案的信息

	TPatternStageInfo m_tAllPaternStageInfo[MAX_PATTERN_COUNT];					//所有方案的阶段信息 

	TErrInfo m_nCheckTranErrCode[C_N_MAX_ERR_SIZE];								//校验结果汇总

	int m_nParamRingNum[MAX_PHASE_COUNT];										//用于判断配置中的环索引是否从1开始，并且连续
};
#endif// !ifndef  OPENATCPARAMCHECK_H
