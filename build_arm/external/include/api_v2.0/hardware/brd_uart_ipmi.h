/*
 * lpc_cuart_ipmi.h
 * communiction between smu&xmpu&ceu using ipmi for cuart
 *
 * ver:1.0.0.0
 * author:zhangzhuan@kedacom.com
 * date:2014/11/12
 */

#ifndef _LPC_CUART_IPMI_H
#define _LPC_CUART_IPMI_H

#define IPMI_NETFN_MSU_PROC_REQUEST                 (0x0e)
#define IPMI_NETFN_MSU_PROC_RESPONSE                (0x0f)

#define IPMI_NETFN_LPC2368_PROC_REQUEST             (0x10)
#define IPMI_NETFN_LPC2368_PROC_RESPONSE            (0x11)

/*MSU PROCESS CMD*/
#define IPMI_GMPU_POWER_ON_STATE_CMD                (0x01)
#define IPMI_BOARD_HOT_PLUG_STATE_CMD               (0x02)
#define IPMI_MSU_TIME_SET_CMD                       (0x03)
#define IPMI_MSU_MS_NOTIFY_CMD                      (0x04)

#define IPMI_MSU_TIME_REQUEST_ACK_CMD               (0x10)
#define IPMI_MSU_RST_GMPU_UNIT_REQ_ACK_CMD          (0x11)
#define IPMI_MSU_REBOOT_CMPU_REQ_ACK_CMD            (0x12)
#define IPMI_MSU_GET_ONLINE_BRD_REQ_ACK_CMD         (0x13)
#define IPMI_MSU_MAIN_MODE_NOTIFY_ACK_CMD           (0x14)
#define IPMI_MSU_HOTPLUG_PROC_FINISH_ACK_CMD        (0x15)
#define IPMI_MSU_SLAVE_MODE_NOTIFY_ACK_CMD          (0x16)
#define IPMI_MSU_REBOOT_ALL_GMPU_ACK_CMD            (0x17)


/*LPC2368 PROCESS CMD*/
#define IPMI_RESET_GMPU_UNIT_CMD                    (0x01)
#define IPMI_GET_BOARD_ACTIVE_STATE_CMD             (0x02)
#define IPMI_GET_SLOT_NUM                           (0x03)

/*cuart impi message type*/
#define CUART_IPMI_MSG_TYPE_LOCAL                   (0x00)   /*本lpc2368处理*/
#define CUART_IPMI_MSG_TYPE_EVENT_REQ               (0x01)   /*event request 消息*/
#define CUART_IPMI_MSG_TYPE_RESPOND                 (0x02)   /*respond 消息*/

#define CUART_IPMI_MSG_MAGIC                        (0xBB)
#define IPMI_EVT_REQ_DATA_LEN                       (6)      /*需要转发的event request消息数据长度*/
#define IPMI_RESP_DATA_LEN                          (40)     /*需要转发的respond消息数据长度*/

typedef enum{
	TEMPERATURE_EVENT               = 0x01,     /**温度事件**/
	VOLTAGE_EVENT,                               /**电压事件**/
	FAN_EVENT                       = 0x04,     /**风扇事件**/
	PROCESSOR_EVENT                 = 0x07,     //
	MSU_TIME_REQUEST_EVENT          = 0x30,
	MSU_RST_GMPU_UNIT_REQ_EVENT     = 0x31,
	MSU_REBOOT_CMPU_REQ_EVENT       = 0x32,
	MSU_GET_ONLINE_BRD_REQ_EVENT    = 0x33,
	MSU_MAIN_MODE_NOTIFY_EVENT      = 0x34,
	MSU_HOTPLUG_PROC_FINISH_EVENT   = 0x35,
	MSU_GET_SLOT_NUM_EVENT          = 0x36,
	MSU_PCIE_WARRN_EVENT            = 0x37,
	GET_MACHINE_TYPE_EVENT          = 0x38,
	MSU_SLAVE_MODE_NOTIFY_EVENT     = 0x39,
	MSU_REBOOT_ALL_GMPU_EVENT       = 0x40,
	GET_SDR_INFO_EVENT              = 0x60,
	HOT_SWAP_EVENT                  = 0xF0,     /**热插拔事件***/
	PHYSICAL_SENSOR_EVENT           = 0xF1      //
}TEventType;

typedef enum { /*消息处理状态*/
	STAT_OK = 0x00,  /*消息处理成功*/
	STAT_FAIL = 0xFF,/*消息处理失败*/
	STAT_ACK = 0xFE, /*ACK消息*/
	STAT_REQ = 0xFC, /*请求消息*/
}msg_proc_stat;

typedef union {
	struct {
		u8 bid;			/*发生热插拔的板子的id*/
		u8 state;		/*热插拔处理之后的状态，0为正常，0xFF表示异常*/
					/*0xFE表示ACK消息,0xFC表示请求消息*/
		}hotplug_msg;			/*热插拔消息*/

	struct {
		u8 bid;            /*上电的板子的id*/
		u8 state;	   /*处理之后的状态，0为正常，0xFF表示异常*/
			   /*0xFE表示ACK消息,0xFC表示请求消息*/
		}pwr_on_msg;               /*板子上电消息*/

	struct {
		u8 bid;             /*请求复位芯片GMPU板子的id*/
		u8 dspnum_low;      /*dspnum_low和dspnum_high一共16位，分别代表1～12号netra*/
		u8 dspnum_high;     /*13位为PEX8749，14位为PEX8749NT0，
				      15位为PEX8749NT1，16位为C6678，值为1有效*/
	}gmpu_unit_reset_msg;       /*gmpu单板各芯片复位的请求消息，包括netra，桥芯片*/

	struct {
		u8 bid;            /*主板的槽号*/
		u8 state;	   /*处理之后的状态，0为正常，0xFF表示异常*/
				   /*0xFE表示ACK消息,0xFC表示请求消息*/
	}ms_mode_msg;              /*MSU主备通知事件消息*/

	struct {
		u8 btype;          /*类型为：BRD_TYPE_CEU，BRD_TYPE_XMPU，BRD_TYPE_ALL*/
		u8 low;            /*low和high一共16位，相应的位为1表示该板子在线*/
		u8 high;           /**/
	}brd_online_msg;           /*获取板子在线状态的消息*/

	struct {
		u8 bid;		    /*重启的gmpu板子id*/
		u8 state;	    /*处理之后的状态，0为正常，0xFF表示异常*/
				    /*0xFE表示ACK消息,0xFC表示请求消息*/
	}gmpu_reboot_msg;           /*gmpu整版重启的消息*/

	struct {
		u8 time[3];	 /*时间*/
	}time_sync_msg;          /*时间同步消息*/

	struct {
		u8 bslot;          /*板子槽号*/
		u8 state;	   /*处理之后的状态，0为正常，0xFF表示异常*/
				   /*0xFE表示ACK消息,0xFC表示请求消息*/
	}slot_num;		   /*获取槽号的消息*/

	struct {
		u8 slot_lsb;        /*槽号，位为1有效*/
		u8 slot_msb;        /**/
	}pcie_warrn_msg;           /*pcie扫描失败的告警消息*/

	u8 pri_data[3];
}comm_date_t;

typedef union {
	struct { /*respond 消息*/
		struct {
			u8 dvoid[4];
			u8 cmd;
			u8 uniq;           /*唯一标示*/
			union {
				comm_date_t comm_data;
				u8 pri_data[10];/*pri_data[4]填写time_sync_msg第四个时间值
						  或者填充gmpu_unit_reset_msg的state*/
			} data;
			u8 p34;
			u8 dvoid2[2];
		} pcmd;
	}resp_data;

	struct { /*request 消息*/
		u8 cmd;            /*命令*/
		u8 dvoid;          /*填充数据*/
		u8 uniq;           /*唯一标示*/
		comm_date_t comm_data;    /*event request 消息，需要转发给ipmi*/
	}req_data ;
}ipmi_core_data_t ;

typedef struct {
	u8 magic;  /*有效消息标示*/
	u8 mtype;  /*消息类型,本地处处理，event或者respond*/
	ipmi_core_data_t ipmi_core_data; /*数据*/
} cuart_ipmi_msg_t ;
#endif
