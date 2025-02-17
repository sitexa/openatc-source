/*
 * brd_uart_ceu.h
 * costom uart communiction between CPU and lpc2368
 *
 * ver:1.0.0.0
 * author:zhangzhuan@kedacom.com
 * date:2014/11/12
 */
#ifndef _CUART_H
#define _CUART_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stdio.h>
#include <assert.h>
//msu cmd
#define CUSTOM_CMD_GMPU_OR_MSU_HOTPULG                          (0x00)
#define CUSTOM_CMD_RST_GMPU_UNIT                                (0x01)    /*重启GMPU上的netra，DSP，桥芯片*/
#define CUSTOM_CMD_MS_MODE_NOTIFY                               (0x02)    /*MSU向SMM发出主板消息*/
#define CUSTOM_CMD_GMPU_PWR_ON_STATE                            (0x03)
#define CUSTOM_CMD_GET_SLOT_NUM                                 (0x04)
#define CUSTOM_CMD_GET_ONLINE_BRD                               (0x05)
#define CUSTOM_CMD_TIME_SYNC_REQ                                (0x06)    /*MSU向SMM发出时间同步请求*/
#define CUSTOM_CMD_TIME_SYNC_RESP                               (0x07)    /*MSU响应SMM发出的时间同步事件*/
#define CUSTOM_CMD_GMPU_REBOOT_REQ                              (0x08)    /*GMPU整板重启*/
#define CUSTOM_CMD_MS_NOTIFY_REQ                                (0x09)    /*smu请求重新通知ceu主备情况*/
#define CUSTOM_CMD_PCIE_WARRN_REQ                               (0x0A)    /*PCIE告警*/
#define CUSTOM_CMD_UNDEFINE                                     (0xFF)    /*命令未定义*/
#define CUSTOM_CMD_SLAVE_MODE_NOTIFY                            (0x12)    /*MSU向SMM发出备板消息*/
#define CUSTOM_CMD_REBOOT_ALL_GMPU                              (0x13)    /*MSU通知SMU重启所有GMPU*/


//for MSU ms notify
#define MCA_MS_NOTIFY_REQUEST          0    /* msu send ms notify to smu */
#define MCA_MS_NOTIFY_REPLY            1    /* msu receive a reply from smu */

//dir
#define MSU_SEND_TO_LPC2368 							(0x01)
#define LPC2368_SEND_TO_MSU 							(0x00)
#define MSG_PROTOCOL_VER                                			(0x01)
#define MSG_PROTOCOL_TYPE                               			(0x01)

/*板子类型定义*/
#define BRD_TYPE_CEU    									(0x01)      /*CEU板*/
#define BRD_TYPE_XMPU   								(0x02)      /*XMPU板*/
#define BRD_TYPE_XMPU5  								(0x03)	 /*XMPU5板*/
#define BRD_TYPE_ALL    									(0x04)      /*所有板子*/
#define BRD_TYPE_XMPU5_2  								(0x06)	 /*XMPU5 3519av100板*/


#define OK                           								(0)
#define ERROR                       								(-1)

#define MESSAGE_RESP                                            (0x0)    /* 应答 */
#define BRD_REINIT_ALL_GMPU                                     (0x1)    /* 通过管道，通知重启所有MGPU */
#define BRD_XMPUx_UNIT_RESET										(0x2)    /* 通过管道，通知重启xmpu、xmpu5的netra，3536，3519芯片*/
#define BRD_SINGLE_GMPU_REBOOT                                     (0x3)    /* 通过管道，通知重启单个xmpu、xmpu5板卡 */

#define PRERROR(fmt,args...) ({  \
		printf("[CUART ERROR] %s(%d):", __FUNCTION__, __LINE__);  \
		printf(fmt, ##args); \
		syslog(LOG_ERR | LOG_LOCAL6, "[CUART ERROR] %s(%d):\n", __FUNCTION__, __LINE__);\
		syslog(LOG_ERR | LOG_LOCAL6, fmt, ##args); \
	})

#if defined(NDEBUG)
#define PRDEBUG(fmt,args...) ({  \
	if (0) {  \
			printf("[CUART DEBUG] %s(%d):", __FUNCTION__, __LINE__); \
			 printf(fmt, ##args);\
		} \
	})
#else
#define PRDEBUG(fmt,args...) ({  \
		 printf("[CUART DEBUG] %s(%d):", __FUNCTION__, __LINE__); \
		 printf(fmt, ##args); \
	 })
#endif
#define PRLOG(fmt,args...) ({  \
	printf("[CUART LOG] %s(%d):", __FUNCTION__, __LINE__);  \
	printf(fmt, ##args); \
	syslog(LOG_NOTICE | LOG_LOCAL6, "[CUART LOG] %s(%d):", __FUNCTION__, __LINE__);\
	syslog(LOG_NOTICE | LOG_LOCAL6, fmt, ##args); \
	})

#define CUASSERT(exp) ({  \
		if (!exp) { \
			printf("[CUART ASSERT] %s(%d):", __FUNCTION__, __LINE__);\
			assert(exp); \
			printf("\n"); \
			} \
	 })

typedef union {
	struct {
		u8 bid;      /*发生热插拔的板子的id*/
		u8 state;    /*热插拔处理之后的状态，0为正常，0xFF表示异常*/
	}hotplug_msg;        /*热插拔消息*/

	struct {
		u8 bid;            /*请求复位芯片GMPU板子的id*/
		u8 dspnum_low;     /*dspnum_low和dspnum_high一共16位，分别代表1～12号netra*/
		u8 dspnum_high;    /*13位为PEX8749，14位为PEX8749NT0，15位为PEX8749NT1，16位为C6678，值为1有效*/
		u8 state;          /*各芯片复位之后的状态，0为正常，0xFF表示异常*/
	}gmpu_unit_reset_msg;     /*gmpu单板各芯片复位的请求消息，包括netra，桥芯片*/

	struct {
		u8 bid;            /*主板的槽号*/
		u8 state;          /*消息处理结果，0为正常，0xFF表示异常*/
	}ms_mode_msg;              /*MSU主备通知事件消息*/

	struct {
		u8 bid;            /*上电的板子的id*/
		u8 state;          /*板子上电之后处理的状态，0为正常，0xFF表示异常*/
	}pwr_on_msg;               /*板子上电消息*/

	struct {
		u8 bslot;          /*板子槽号*/
		u8 state;          /*获取槽号的的状态，0为正常，0xFF表示异常*/
	}slot_num;                 /*获取槽号的消息*/

	struct {
		u8 btype;          /*类型为：BRD_TYPE_CEU，BRD_TYPE_XMPU，BRD_TYPE_ALL*/
		u8 low;            /*low和high一共16位，相应的位为1表示该板子在线*/
		u8 high;           /**/
	}brd_online_msg;               /*获取板子在线状态的消息*/

	struct {
		u8 bid;           /*重启的gmpu板子id*/
		u8 state;         /*整版重启状态，0为正常，0xFF表示异常*/
	}gmpu_reboot_msg;        /*gmpu整版重启的消息*/

	struct {
		u8 time[4];      /*时间值*/
		u8 state;        /*时间同步状态，0为正常，0xFF表示异常*/
	}time_sync_msg;          /*时间同步消息*/

	struct {
		u8 slot_lsb;    /*槽号，位为1有效*/
		u8 slot_msb;    /**/
	}pcie_warrn_msg;        /*pcie扫描失败的告警消息*/

	u8 pri_data[14];
}msg_data_t;

typedef struct msg_cmd_t{
	u8 cmd;
	u8 dir;
	msg_data_t msg_data;
}msg_cmd_t;

typedef struct {
	u8 ver; /*版本信息*/
	u8 type; /*版本类型*/
	u8 ssrc; /*唯一标示符*/
	msg_cmd_t msg_cmd;
} cuart_msg_t ;

/*
 * 参数说明：
 *           msg_data：相应事件传给处理函数信息
 */
typedef s32(*cuart_msg_callback)(msg_data_t *msg_data, void *para);

typedef enum { /*事件类型*/
	EV_HOTPLUG,                  /*热插拔事件*/
	EV_GMPU_REBOOT_UNIT,         /*GMPU板子相应模块重启请求*/
	EV_MSU_MS_MOD_NOTIFY,        /*MSU主备通知请求*/
	EV_GMPU_PWR_ON_STAT,         /*GMPU上电状态事件*/
	EV_MSU_SLOT_NUM,             /*MSU获取自身槽号请求*/
	EV_BRD_ON_LINE_STAT,         /*GMPU在位状态请求*/
	EV_TIME_SYNC,                /*时间同步事件*/
	EV_GMPU_REBOOT,              /*GMPU整板重启请求*/
	EV_MSU_MS_NOTIFY_REQ,        /*CEU主备重新通知请求，以备SMU实时获取CEU主备情况*/
	EV_MSU_SLAVE_MODE_NOTIFY,    /*MSU备板通知请求*/
	EV_MSU_REBOOT_ALL_GMPU,      /*MSU重启所有GMPU请求*/
	EV_ACK,                      /*消息回应事件*/
	EV_UNDEFINE,                 /*事件未定义*/
}brd_event;

typedef enum {
	MSG_TYPE_REQ,                 /*MSU发出请求后，收到的确认消息*/
	MSG_TYPE_RESP,                /*MSU未发出请求，收到的事件消息*/
}msg_type;

typedef struct brdUartFifoMsg
{
	u32  pid;                     /* pid: 对端进程pid */
	u8   bMsgCmd;                 /* bMsgCmd: fifo 消息类型 */
	u8   bId;                     /* bId: 热插拔板子的槽号 */
	u8   bSlot;                   /* bslot：业务板所处槽位号 */
	u8   bType;                   /* bType: 板子类型：BRD_TYPE_CEU，BRD_TYPE_XMPU，BRD_TYPE_ALL */
	u16  nBrd;                    /* nBrd：每一位为1表示响应槽号的板子在位 */
	u16  nChipId;                 /* nChipId: 重启EP对应的芯片号 */
	u16  nUnit;                   /* nUnit：表示复位的芯片,每一位为1表示要复位的芯片 */
	u32  ack;                     /* ack: 表示消息是否确认成功发送接收 */
} TbrdUartFifoMsg;

/*===============================
函数名：BrdCUartMsgInit
功能：lpc2368到msu串口通信初始化
算法实现：（可选项）
引用全局变量：
返回值说明： 成功返回OK
	     失败返回ERROR
==================================*/
s32 BrdCUartMsgInit(void);

/*===============================
函数名：BrdRegCUEventHandler
功能：注册事件处理
算法实现：（可选项）
引用全局变量：
	     event事件:
	           EV_HOTPLUG,                  热插拔事件
		   EV_GMPU_REBOOT_UNIT,         GMPU板子相应模块重启事件
		   EV_MSU_MS_MOD_NOTIFY,        MSU主备通知事件
		   EV_GMPU_PWR_ON_STAT,         获取GMPU上电状态事件
		   EV_MSU_SLOT_NUM,             MSU获取自身槽号事件
		   EV_BRD_ON_LINE_STAT,         板子在位状态事件
		   EV_TIME_SYNC,                时间同步事件
		   EV_GMPU_REBOOT,              GMPU整板重启事件

	     handler:函数指针
	     para：传给handler的参数
返回值说明： 成功返回OK
	     失败返回ERROR
==================================*/
s32 BrdRegCUEventHandler(brd_event event, cuart_msg_callback handler,void *para);

/*===============================
函数名：BrdHotplugProcDone
功能：通知SMU板子热插拔处理完成，执行去激活操作
算法实现：（可选项）
引用全局变量：bId:热插拔板子的槽号
返回值说明： 成功返回OK
	     失败返回ERROR
==================================*/
s32 BrdHotplugProcDone(u8 bId);

/*===============================
函数名：BrdMSStateNotify
功能：通知ipmb系统当前MSU主板槽号
算法实现：（可选项）
引用全局变量：bId:当前为主板MSU的槽号
返回值说明： 成功返回OK
	     失败返回ERROR
==================================*/
s32 BrdMSStateNotify(u8 bId);

/*===============================
函数名：BrdSlaveStateNotify
功能：通知ipmb系统当前MSU备板槽号
算法实现：（可选项）
引用全局变量：bId:当前为备板MSU的槽号
返回值说明： 成功返回OK
	     失败返回ERROR
==================================*/
s32 BrdSlaveStateNotify(u8 bId);

/*===============================
函数名：BrdGMPUUnitReset
功能：GMPU板子各芯片单元复位
算法实现：（可选项）
引用全局变量：bId:表示GMPU板子号
	      nUnit：表示复位的芯片,每一位为1表示要复位的芯片
	      一共有16位，高四位分别别是：C6678,PEX8749NT1,PEX8749NT0,PEX8749
	      低12位分别表示1～12号netra芯片
              如：要复位第1、2、5号netra芯片，nUnit值的二进值为10011，即：nUnit=0x13
返回值说明： 成功返回OK
	     失败返回ERROR
==================================*/
s32 BrdGMPUUnitReset(u8 bId,u16 nUnit);

/*===============================
函数名：BrdGetOnlineBoard
功能：获取机箱内板子的在位信息
算法实现：（可选项）
引用全局变量：
	      bType:板子类型：BRD_TYPE_CEU，BRD_TYPE_XMPU，BRD_TYPE_ALL
	      nBrd：每一位为1表示响应槽号的板子在位
	            如：1、2、5号板子在位，nBrd在函数运行后的值为10011，即：nBrd=0x13

返回值说明： 成功返回OK
	     失败返回ERROR
==================================*/
s32 BrdGetOnlineBoard(u8 bType, u16* nBrd);

/*===============================
 * 函数名：BrdGetSlotNumByFile
 * 功能;通过 "/dev/slot" 获取当前板子的槽号 (需要初始化并设置 "/dev/slot" )
 * 算法实现：避免两个进程同时使用串口导致无法获取板卡槽位号
 * 引用全局变量：bSlot表示板子槽号
 * 返回值说明： 成功返回OK
 *              失败返回ERROR
 ==================================*/
s32 BrdGetSlotNumByFile(u8* bSlot);

/*===============================
 * 函数名：BrdGetSlotNum
 * 功能;获取当前板子的槽号
 * 算法实现：（可选项）
 * 引用全局变量：bSlot表示板子槽号
 * 返回值说明： 成功返回OK
 *              失败返回ERROR
 ==================================*/
s32 BrdGetSlotNum(u8* bSlot);

/*===============================
 * 函数名：BrdTimeSync
 * 功能;时间同步请求
 * 算法实现：（可选项）
 * 引用全局变量：
 * 返回值说明： 成功返回OK
 *              失败返回ERROR
 ==================================*/
s32 BrdTimeSync(void);

/*===============================
 * 函数名：BrdGMPUReboot
 * 功能;GMPU整板重启
 * 算法实现：（可选项）
 * 引用全局变量：
 * 返回值说明： 成功返回OK
 *              失败返回ERROR
 ==================================*/
s32 BrdGMPUReboot(u8 bId);

/*===============================
 * 函数名：BrdPcieWarrn(u16 slot);
 * 功能;pcie告警
 * 算法实现：（可选项）
 * 引用全局变量：
 * 返回值说明： 成功返回OK
 *              失败返回ERROR
 ==================================*/
s32 BrdPcieWarrn(u16 slot);

/*===============================
函数名：BrdCUartMsgInit
功能：lpc2368到msu串口通信初始化,专用于initbrd处理
算法实现：（可选项）
引用全局变量：
返回值说明： 成功返回OK
	     失败返回ERROR
==================================*/
s32 BrdCUartMsgInit2(void);

/*===============================
函数名：BrdChipFileReload
功能：单个EP重启加载内核和文件系统
算法实现：（可选项）
引用全局变量：
		slot 业务板所处槽位号
		type 业务板类型xmpu/xmpu5(2/3)
		chipId 重启EP对应的芯片号
返回值说明： 成功返回OK
	     失败返回ERROR
==================================*/
s32 BrdChipFileReload(u8 slot,  u8 type, u16 chipId);

/*===============================
 函数名： UartRebootAllGMPU
 功能: 发送通知给SMU，使其重启所有的XMPU/XMPU5
 算法实现：（可选项）
 引用全局变量：
 返回值说明： 成功返回OK
              失败返回ERROR
 ==================================*/
s32 UartRebootAllGMPU(void);

/*===============================
 函数名： BrdRebootAllGMPU
 功能: 使所有EP侧重启
 算法实现：（可选项）
 引用全局变量：
 返回值说明： 成功返回OK
              失败返回ERROR
 ==================================*/
s32 BrdRebootAllGMPU(void);

/*===============================
函数名：BrdMsuUartInit
功能：ceu 串口功能单元模块初始化
算法实现：（可选项）
引用全局变量：无
返回值说明： 成功返回OK
	     失败返回ERROR
==================================*/
s32 BrdMsuUartInit(void);

/*===============================
函数名：BrdMsuReloadXmpuInit
功能：确认autoload是否是首次初始化（主板autoload首次初始化时，重启所有xmpu（5））
算法实现：（可选项）
引用全局变量：无
返回值说明： 成功返回OK
	     失败返回ERROR
==================================*/
s32 BrdMsuReloadXmpuInit(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
