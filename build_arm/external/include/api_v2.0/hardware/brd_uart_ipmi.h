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
#define CUART_IPMI_MSG_TYPE_LOCAL                   (0x00)   /*��lpc2368����*/
#define CUART_IPMI_MSG_TYPE_EVENT_REQ               (0x01)   /*event request ��Ϣ*/
#define CUART_IPMI_MSG_TYPE_RESPOND                 (0x02)   /*respond ��Ϣ*/

#define CUART_IPMI_MSG_MAGIC                        (0xBB)
#define IPMI_EVT_REQ_DATA_LEN                       (6)      /*��Ҫת����event request��Ϣ���ݳ���*/
#define IPMI_RESP_DATA_LEN                          (40)     /*��Ҫת����respond��Ϣ���ݳ���*/

typedef enum{
	TEMPERATURE_EVENT               = 0x01,     /**�¶��¼�**/
	VOLTAGE_EVENT,                               /**��ѹ�¼�**/
	FAN_EVENT                       = 0x04,     /**�����¼�**/
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
	HOT_SWAP_EVENT                  = 0xF0,     /**�Ȳ���¼�***/
	PHYSICAL_SENSOR_EVENT           = 0xF1      //
}TEventType;

typedef enum { /*��Ϣ����״̬*/
	STAT_OK = 0x00,  /*��Ϣ����ɹ�*/
	STAT_FAIL = 0xFF,/*��Ϣ����ʧ��*/
	STAT_ACK = 0xFE, /*ACK��Ϣ*/
	STAT_REQ = 0xFC, /*������Ϣ*/
}msg_proc_stat;

typedef union {
	struct {
		u8 bid;			/*�����Ȳ�εİ��ӵ�id*/
		u8 state;		/*�Ȳ�δ���֮���״̬��0Ϊ������0xFF��ʾ�쳣*/
					/*0xFE��ʾACK��Ϣ,0xFC��ʾ������Ϣ*/
		}hotplug_msg;			/*�Ȳ����Ϣ*/

	struct {
		u8 bid;            /*�ϵ�İ��ӵ�id*/
		u8 state;	   /*����֮���״̬��0Ϊ������0xFF��ʾ�쳣*/
			   /*0xFE��ʾACK��Ϣ,0xFC��ʾ������Ϣ*/
		}pwr_on_msg;               /*�����ϵ���Ϣ*/

	struct {
		u8 bid;             /*����λоƬGMPU���ӵ�id*/
		u8 dspnum_low;      /*dspnum_low��dspnum_highһ��16λ���ֱ����1��12��netra*/
		u8 dspnum_high;     /*13λΪPEX8749��14λΪPEX8749NT0��
				      15λΪPEX8749NT1��16λΪC6678��ֵΪ1��Ч*/
	}gmpu_unit_reset_msg;       /*gmpu�����оƬ��λ��������Ϣ������netra����оƬ*/

	struct {
		u8 bid;            /*����Ĳۺ�*/
		u8 state;	   /*����֮���״̬��0Ϊ������0xFF��ʾ�쳣*/
				   /*0xFE��ʾACK��Ϣ,0xFC��ʾ������Ϣ*/
	}ms_mode_msg;              /*MSU����֪ͨ�¼���Ϣ*/

	struct {
		u8 btype;          /*����Ϊ��BRD_TYPE_CEU��BRD_TYPE_XMPU��BRD_TYPE_ALL*/
		u8 low;            /*low��highһ��16λ����Ӧ��λΪ1��ʾ�ð�������*/
		u8 high;           /**/
	}brd_online_msg;           /*��ȡ��������״̬����Ϣ*/

	struct {
		u8 bid;		    /*������gmpu����id*/
		u8 state;	    /*����֮���״̬��0Ϊ������0xFF��ʾ�쳣*/
				    /*0xFE��ʾACK��Ϣ,0xFC��ʾ������Ϣ*/
	}gmpu_reboot_msg;           /*gmpu������������Ϣ*/

	struct {
		u8 time[3];	 /*ʱ��*/
	}time_sync_msg;          /*ʱ��ͬ����Ϣ*/

	struct {
		u8 bslot;          /*���Ӳۺ�*/
		u8 state;	   /*����֮���״̬��0Ϊ������0xFF��ʾ�쳣*/
				   /*0xFE��ʾACK��Ϣ,0xFC��ʾ������Ϣ*/
	}slot_num;		   /*��ȡ�ۺŵ���Ϣ*/

	struct {
		u8 slot_lsb;        /*�ۺţ�λΪ1��Ч*/
		u8 slot_msb;        /**/
	}pcie_warrn_msg;           /*pcieɨ��ʧ�ܵĸ澯��Ϣ*/

	u8 pri_data[3];
}comm_date_t;

typedef union {
	struct { /*respond ��Ϣ*/
		struct {
			u8 dvoid[4];
			u8 cmd;
			u8 uniq;           /*Ψһ��ʾ*/
			union {
				comm_date_t comm_data;
				u8 pri_data[10];/*pri_data[4]��дtime_sync_msg���ĸ�ʱ��ֵ
						  �������gmpu_unit_reset_msg��state*/
			} data;
			u8 p34;
			u8 dvoid2[2];
		} pcmd;
	}resp_data;

	struct { /*request ��Ϣ*/
		u8 cmd;            /*����*/
		u8 dvoid;          /*�������*/
		u8 uniq;           /*Ψһ��ʾ*/
		comm_date_t comm_data;    /*event request ��Ϣ����Ҫת����ipmi*/
	}req_data ;
}ipmi_core_data_t ;

typedef struct {
	u8 magic;  /*��Ч��Ϣ��ʾ*/
	u8 mtype;  /*��Ϣ����,���ش�����event����respond*/
	ipmi_core_data_t ipmi_core_data; /*����*/
} cuart_ipmi_msg_t ;
#endif
