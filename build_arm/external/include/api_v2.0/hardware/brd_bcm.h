/*
 * Kedacom Hardware Abstract Level
 *
 * brd_bcm.h
 *
 * Copyright (C) 2013-2020, Kedacom, Inc.
 *
 * History:
 *   2019/10/11 - [zhugeyifan] Create
 *
 */

#ifndef _BRD_BCM_H
#define _BRD_BCM_H

#ifdef __cplusplus
extern "C" {
#endif

#include <drvlib_def.h>

#define JD10000_MAX_SLOT_NUM     16  /* JD10000 slot num(include smu) */
#define JD6000_MAX_SLOT_NUM      8   /* JD6000 slot num(include smu)  */
#define MAX_SLOT_NUM             16  /* Current maximum slot number */


typedef struct mac_slot_data {
	unsigned char macaddr[6];    /* Eth MAC Address */
	unsigned char slot;          /* Slot Number */
	int           linkstatus;    /* Link Status */
} TBcmMacSlotData;

typedef struct bcm_info {
	struct mac_slot_data data[MAX_SLOT_NUM];
} TBcmInfo;


/*===============================
函数名：BrdGetRemoteBoardMacSlotInfo
功能：获取机箱内远端在线板卡的Mac地址和SLOT槽位信息
      Ps：通过机框内部交换芯片获取，
          因为SMU其中一个内网网口会经过CEU2再转发到另一个CEU2
          因此通过BCM交换芯片获取远端的板卡信息不包括CEU2
算法实现：（可选项）
引用全局变量：
返回值说明： 成功返回OK
             失败返回ERROR
==================================*/
int BrdGetRemoteBoardMacSlotInfo(struct bcm_info *api_info);

/*===============================
函数名：BrdGetLocalBoardMacSlotInfo
功能：获取机箱内本地在线板卡的Mac地址和SLOT槽位信息
      Ps：只获取当前板卡的eth0 mac地址和slot信息
算法实现：（可选项）
引用全局变量：
返回值说明： 成功返回OK
             失败返回ERROR
==================================*/
int BrdGetLocalBoardMacSlotInfo(struct bcm_info *api_info);

/*
 * 通过Bcm交换芯片获取远端板卡的Mac与Slot槽位信息
 * api_info：用于返回给用户的检索信息结构体
 * return: success 0
 *         fail    -1
 */
int BrdGetBcmMacSlotInfo(struct bcm_info *api_info);


#ifdef __cplusplus
}
#endif

#endif
