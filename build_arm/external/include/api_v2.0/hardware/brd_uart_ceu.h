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
#define CUSTOM_CMD_RST_GMPU_UNIT                                (0x01)    /*����GMPU�ϵ�netra��DSP����оƬ*/
#define CUSTOM_CMD_MS_MODE_NOTIFY                               (0x02)    /*MSU��SMM����������Ϣ*/
#define CUSTOM_CMD_GMPU_PWR_ON_STATE                            (0x03)
#define CUSTOM_CMD_GET_SLOT_NUM                                 (0x04)
#define CUSTOM_CMD_GET_ONLINE_BRD                               (0x05)
#define CUSTOM_CMD_TIME_SYNC_REQ                                (0x06)    /*MSU��SMM����ʱ��ͬ������*/
#define CUSTOM_CMD_TIME_SYNC_RESP                               (0x07)    /*MSU��ӦSMM������ʱ��ͬ���¼�*/
#define CUSTOM_CMD_GMPU_REBOOT_REQ                              (0x08)    /*GMPU��������*/
#define CUSTOM_CMD_MS_NOTIFY_REQ                                (0x09)    /*smu��������֪ͨceu�������*/
#define CUSTOM_CMD_PCIE_WARRN_REQ                               (0x0A)    /*PCIE�澯*/
#define CUSTOM_CMD_UNDEFINE                                     (0xFF)    /*����δ����*/
#define CUSTOM_CMD_SLAVE_MODE_NOTIFY                            (0x12)    /*MSU��SMM����������Ϣ*/
#define CUSTOM_CMD_REBOOT_ALL_GMPU                              (0x13)    /*MSU֪ͨSMU��������GMPU*/


//for MSU ms notify
#define MCA_MS_NOTIFY_REQUEST          0    /* msu send ms notify to smu */
#define MCA_MS_NOTIFY_REPLY            1    /* msu receive a reply from smu */

//dir
#define MSU_SEND_TO_LPC2368 							(0x01)
#define LPC2368_SEND_TO_MSU 							(0x00)
#define MSG_PROTOCOL_VER                                			(0x01)
#define MSG_PROTOCOL_TYPE                               			(0x01)

/*�������Ͷ���*/
#define BRD_TYPE_CEU    									(0x01)      /*CEU��*/
#define BRD_TYPE_XMPU   								(0x02)      /*XMPU��*/
#define BRD_TYPE_XMPU5  								(0x03)	 /*XMPU5��*/
#define BRD_TYPE_ALL    									(0x04)      /*���а���*/
#define BRD_TYPE_XMPU5_2  								(0x06)	 /*XMPU5 3519av100��*/


#define OK                           								(0)
#define ERROR                       								(-1)

#define MESSAGE_RESP                                            (0x0)    /* Ӧ�� */
#define BRD_REINIT_ALL_GMPU                                     (0x1)    /* ͨ���ܵ���֪ͨ��������MGPU */
#define BRD_XMPUx_UNIT_RESET										(0x2)    /* ͨ���ܵ���֪ͨ����xmpu��xmpu5��netra��3536��3519оƬ*/
#define BRD_SINGLE_GMPU_REBOOT                                     (0x3)    /* ͨ���ܵ���֪ͨ��������xmpu��xmpu5�忨 */

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
		u8 bid;      /*�����Ȳ�εİ��ӵ�id*/
		u8 state;    /*�Ȳ�δ���֮���״̬��0Ϊ������0xFF��ʾ�쳣*/
	}hotplug_msg;        /*�Ȳ����Ϣ*/

	struct {
		u8 bid;            /*����λоƬGMPU���ӵ�id*/
		u8 dspnum_low;     /*dspnum_low��dspnum_highһ��16λ���ֱ����1��12��netra*/
		u8 dspnum_high;    /*13λΪPEX8749��14λΪPEX8749NT0��15λΪPEX8749NT1��16λΪC6678��ֵΪ1��Ч*/
		u8 state;          /*��оƬ��λ֮���״̬��0Ϊ������0xFF��ʾ�쳣*/
	}gmpu_unit_reset_msg;     /*gmpu�����оƬ��λ��������Ϣ������netra����оƬ*/

	struct {
		u8 bid;            /*����Ĳۺ�*/
		u8 state;          /*��Ϣ��������0Ϊ������0xFF��ʾ�쳣*/
	}ms_mode_msg;              /*MSU����֪ͨ�¼���Ϣ*/

	struct {
		u8 bid;            /*�ϵ�İ��ӵ�id*/
		u8 state;          /*�����ϵ�֮�����״̬��0Ϊ������0xFF��ʾ�쳣*/
	}pwr_on_msg;               /*�����ϵ���Ϣ*/

	struct {
		u8 bslot;          /*���Ӳۺ�*/
		u8 state;          /*��ȡ�ۺŵĵ�״̬��0Ϊ������0xFF��ʾ�쳣*/
	}slot_num;                 /*��ȡ�ۺŵ���Ϣ*/

	struct {
		u8 btype;          /*����Ϊ��BRD_TYPE_CEU��BRD_TYPE_XMPU��BRD_TYPE_ALL*/
		u8 low;            /*low��highһ��16λ����Ӧ��λΪ1��ʾ�ð�������*/
		u8 high;           /**/
	}brd_online_msg;               /*��ȡ��������״̬����Ϣ*/

	struct {
		u8 bid;           /*������gmpu����id*/
		u8 state;         /*��������״̬��0Ϊ������0xFF��ʾ�쳣*/
	}gmpu_reboot_msg;        /*gmpu������������Ϣ*/

	struct {
		u8 time[4];      /*ʱ��ֵ*/
		u8 state;        /*ʱ��ͬ��״̬��0Ϊ������0xFF��ʾ�쳣*/
	}time_sync_msg;          /*ʱ��ͬ����Ϣ*/

	struct {
		u8 slot_lsb;    /*�ۺţ�λΪ1��Ч*/
		u8 slot_msb;    /**/
	}pcie_warrn_msg;        /*pcieɨ��ʧ�ܵĸ澯��Ϣ*/

	u8 pri_data[14];
}msg_data_t;

typedef struct msg_cmd_t{
	u8 cmd;
	u8 dir;
	msg_data_t msg_data;
}msg_cmd_t;

typedef struct {
	u8 ver; /*�汾��Ϣ*/
	u8 type; /*�汾����*/
	u8 ssrc; /*Ψһ��ʾ��*/
	msg_cmd_t msg_cmd;
} cuart_msg_t ;

/*
 * ����˵����
 *           msg_data����Ӧ�¼�������������Ϣ
 */
typedef s32(*cuart_msg_callback)(msg_data_t *msg_data, void *para);

typedef enum { /*�¼�����*/
	EV_HOTPLUG,                  /*�Ȳ���¼�*/
	EV_GMPU_REBOOT_UNIT,         /*GMPU������Ӧģ����������*/
	EV_MSU_MS_MOD_NOTIFY,        /*MSU����֪ͨ����*/
	EV_GMPU_PWR_ON_STAT,         /*GMPU�ϵ�״̬�¼�*/
	EV_MSU_SLOT_NUM,             /*MSU��ȡ����ۺ�����*/
	EV_BRD_ON_LINE_STAT,         /*GMPU��λ״̬����*/
	EV_TIME_SYNC,                /*ʱ��ͬ���¼�*/
	EV_GMPU_REBOOT,              /*GMPU������������*/
	EV_MSU_MS_NOTIFY_REQ,        /*CEU��������֪ͨ�����Ա�SMUʵʱ��ȡCEU�������*/
	EV_MSU_SLAVE_MODE_NOTIFY,    /*MSU����֪ͨ����*/
	EV_MSU_REBOOT_ALL_GMPU,      /*MSU��������GMPU����*/
	EV_ACK,                      /*��Ϣ��Ӧ�¼�*/
	EV_UNDEFINE,                 /*�¼�δ����*/
}brd_event;

typedef enum {
	MSG_TYPE_REQ,                 /*MSU����������յ���ȷ����Ϣ*/
	MSG_TYPE_RESP,                /*MSUδ���������յ����¼���Ϣ*/
}msg_type;

typedef struct brdUartFifoMsg
{
	u32  pid;                     /* pid: �Զ˽���pid */
	u8   bMsgCmd;                 /* bMsgCmd: fifo ��Ϣ���� */
	u8   bId;                     /* bId: �Ȳ�ΰ��ӵĲۺ� */
	u8   bSlot;                   /* bslot��ҵ���������λ�� */
	u8   bType;                   /* bType: �������ͣ�BRD_TYPE_CEU��BRD_TYPE_XMPU��BRD_TYPE_ALL */
	u16  nBrd;                    /* nBrd��ÿһλΪ1��ʾ��Ӧ�ۺŵİ�����λ */
	u16  nChipId;                 /* nChipId: ����EP��Ӧ��оƬ�� */
	u16  nUnit;                   /* nUnit����ʾ��λ��оƬ,ÿһλΪ1��ʾҪ��λ��оƬ */
	u32  ack;                     /* ack: ��ʾ��Ϣ�Ƿ�ȷ�ϳɹ����ͽ��� */
} TbrdUartFifoMsg;

/*===============================
��������BrdCUartMsgInit
���ܣ�lpc2368��msu����ͨ�ų�ʼ��
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
����ֵ˵���� �ɹ�����OK
	     ʧ�ܷ���ERROR
==================================*/
s32 BrdCUartMsgInit(void);

/*===============================
��������BrdRegCUEventHandler
���ܣ�ע���¼�����
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
	     event�¼�:
	           EV_HOTPLUG,                  �Ȳ���¼�
		   EV_GMPU_REBOOT_UNIT,         GMPU������Ӧģ�������¼�
		   EV_MSU_MS_MOD_NOTIFY,        MSU����֪ͨ�¼�
		   EV_GMPU_PWR_ON_STAT,         ��ȡGMPU�ϵ�״̬�¼�
		   EV_MSU_SLOT_NUM,             MSU��ȡ����ۺ��¼�
		   EV_BRD_ON_LINE_STAT,         ������λ״̬�¼�
		   EV_TIME_SYNC,                ʱ��ͬ���¼�
		   EV_GMPU_REBOOT,              GMPU���������¼�

	     handler:����ָ��
	     para������handler�Ĳ���
����ֵ˵���� �ɹ�����OK
	     ʧ�ܷ���ERROR
==================================*/
s32 BrdRegCUEventHandler(brd_event event, cuart_msg_callback handler,void *para);

/*===============================
��������BrdHotplugProcDone
���ܣ�֪ͨSMU�����Ȳ�δ�����ɣ�ִ��ȥ�������
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����bId:�Ȳ�ΰ��ӵĲۺ�
����ֵ˵���� �ɹ�����OK
	     ʧ�ܷ���ERROR
==================================*/
s32 BrdHotplugProcDone(u8 bId);

/*===============================
��������BrdMSStateNotify
���ܣ�֪ͨipmbϵͳ��ǰMSU����ۺ�
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����bId:��ǰΪ����MSU�Ĳۺ�
����ֵ˵���� �ɹ�����OK
	     ʧ�ܷ���ERROR
==================================*/
s32 BrdMSStateNotify(u8 bId);

/*===============================
��������BrdSlaveStateNotify
���ܣ�֪ͨipmbϵͳ��ǰMSU����ۺ�
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����bId:��ǰΪ����MSU�Ĳۺ�
����ֵ˵���� �ɹ�����OK
	     ʧ�ܷ���ERROR
==================================*/
s32 BrdSlaveStateNotify(u8 bId);

/*===============================
��������BrdGMPUUnitReset
���ܣ�GMPU���Ӹ�оƬ��Ԫ��λ
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����bId:��ʾGMPU���Ӻ�
	      nUnit����ʾ��λ��оƬ,ÿһλΪ1��ʾҪ��λ��оƬ
	      һ����16λ������λ�ֱ���ǣ�C6678,PEX8749NT1,PEX8749NT0,PEX8749
	      ��12λ�ֱ��ʾ1��12��netraоƬ
              �磺Ҫ��λ��1��2��5��netraоƬ��nUnitֵ�Ķ���ֵΪ10011������nUnit=0x13
����ֵ˵���� �ɹ�����OK
	     ʧ�ܷ���ERROR
==================================*/
s32 BrdGMPUUnitReset(u8 bId,u16 nUnit);

/*===============================
��������BrdGetOnlineBoard
���ܣ���ȡ�����ڰ��ӵ���λ��Ϣ
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
	      bType:�������ͣ�BRD_TYPE_CEU��BRD_TYPE_XMPU��BRD_TYPE_ALL
	      nBrd��ÿһλΪ1��ʾ��Ӧ�ۺŵİ�����λ
	            �磺1��2��5�Ű�����λ��nBrd�ں������к��ֵΪ10011������nBrd=0x13

����ֵ˵���� �ɹ�����OK
	     ʧ�ܷ���ERROR
==================================*/
s32 BrdGetOnlineBoard(u8 bType, u16* nBrd);

/*===============================
 * ��������BrdGetSlotNumByFile
 * ����;ͨ�� "/dev/slot" ��ȡ��ǰ���ӵĲۺ� (��Ҫ��ʼ�������� "/dev/slot" )
 * �㷨ʵ�֣�������������ͬʱʹ�ô��ڵ����޷���ȡ�忨��λ��
 * ����ȫ�ֱ�����bSlot��ʾ���Ӳۺ�
 * ����ֵ˵���� �ɹ�����OK
 *              ʧ�ܷ���ERROR
 ==================================*/
s32 BrdGetSlotNumByFile(u8* bSlot);

/*===============================
 * ��������BrdGetSlotNum
 * ����;��ȡ��ǰ���ӵĲۺ�
 * �㷨ʵ�֣�����ѡ�
 * ����ȫ�ֱ�����bSlot��ʾ���Ӳۺ�
 * ����ֵ˵���� �ɹ�����OK
 *              ʧ�ܷ���ERROR
 ==================================*/
s32 BrdGetSlotNum(u8* bSlot);

/*===============================
 * ��������BrdTimeSync
 * ����;ʱ��ͬ������
 * �㷨ʵ�֣�����ѡ�
 * ����ȫ�ֱ�����
 * ����ֵ˵���� �ɹ�����OK
 *              ʧ�ܷ���ERROR
 ==================================*/
s32 BrdTimeSync(void);

/*===============================
 * ��������BrdGMPUReboot
 * ����;GMPU��������
 * �㷨ʵ�֣�����ѡ�
 * ����ȫ�ֱ�����
 * ����ֵ˵���� �ɹ�����OK
 *              ʧ�ܷ���ERROR
 ==================================*/
s32 BrdGMPUReboot(u8 bId);

/*===============================
 * ��������BrdPcieWarrn(u16 slot);
 * ����;pcie�澯
 * �㷨ʵ�֣�����ѡ�
 * ����ȫ�ֱ�����
 * ����ֵ˵���� �ɹ�����OK
 *              ʧ�ܷ���ERROR
 ==================================*/
s32 BrdPcieWarrn(u16 slot);

/*===============================
��������BrdCUartMsgInit
���ܣ�lpc2368��msu����ͨ�ų�ʼ��,ר����initbrd����
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
����ֵ˵���� �ɹ�����OK
	     ʧ�ܷ���ERROR
==================================*/
s32 BrdCUartMsgInit2(void);

/*===============================
��������BrdChipFileReload
���ܣ�����EP���������ں˺��ļ�ϵͳ
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
		slot ҵ���������λ��
		type ҵ�������xmpu/xmpu5(2/3)
		chipId ����EP��Ӧ��оƬ��
����ֵ˵���� �ɹ�����OK
	     ʧ�ܷ���ERROR
==================================*/
s32 BrdChipFileReload(u8 slot,  u8 type, u16 chipId);

/*===============================
 �������� UartRebootAllGMPU
 ����: ����֪ͨ��SMU��ʹ���������е�XMPU/XMPU5
 �㷨ʵ�֣�����ѡ�
 ����ȫ�ֱ�����
 ����ֵ˵���� �ɹ�����OK
              ʧ�ܷ���ERROR
 ==================================*/
s32 UartRebootAllGMPU(void);

/*===============================
 �������� BrdRebootAllGMPU
 ����: ʹ����EP������
 �㷨ʵ�֣�����ѡ�
 ����ȫ�ֱ�����
 ����ֵ˵���� �ɹ�����OK
              ʧ�ܷ���ERROR
 ==================================*/
s32 BrdRebootAllGMPU(void);

/*===============================
��������BrdMsuUartInit
���ܣ�ceu ���ڹ��ܵ�Ԫģ���ʼ��
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�������
����ֵ˵���� �ɹ�����OK
	     ʧ�ܷ���ERROR
==================================*/
s32 BrdMsuUartInit(void);

/*===============================
��������BrdMsuReloadXmpuInit
���ܣ�ȷ��autoload�Ƿ����״γ�ʼ��������autoload�״γ�ʼ��ʱ����������xmpu��5����
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�������
����ֵ˵���� �ɹ�����OK
	     ʧ�ܷ���ERROR
==================================*/
s32 BrdMsuReloadXmpuInit(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
