/*
 * Kedacom Hardware Abstract Level
 *
 * brd_wrapper.h
 *
 * Copyright (C) 2013-2020, Kedacom, Inc.
 *
 * History:
 *   2017/08/30 - [yuhui] Create
 *
 */

#ifndef _BRD_WRAPPER_H
#define _BRD_WRAPPER_H
#include <drvlib_def.h>
#ifdef __cplusplus
extern "C" {
#endif

//所有板子主备状态定义
#define BRD_RUN_MASTER		((u8)0x00)  /*板子为主板*/
#define BRD_RUN_SLAVE		((u8)0x01)  /*板子为备板*/
//For JD10000CEU板主备附加状态定义
#define BRD_CEU_MS_WAIT		((u8)0xFF)  /*CEU板主备等待确定*/

//不同设备对应的硬件ID（eeprom_hwid）
#define JD4000_EEPROM_HWID          0x032B
#define JD6000_EEPROM_HWID          0x028F
#define JD10000_EEPROM_HWID         0x028E

/*====================================================================
 函数名: BrdQueryMSMode
 功能  : 获取板子的主备状态
 输入参数: msMode -- 用于获取板子主备的指针
          #define BRD_RUN_MASTER          ((u8)0x00)  板子为主板
	   #define BRD_RUN_SLAVE           ((u8)0x01)  板子为备板

 For JD10000 CEU板子：
	   #define BRD_CEU_MS_WAIT         ((u8)0xFF)   CEU板主备等待确定
 返回值说明: 0为正常，-1为异常
 ====================================================================*/
int BrdQueryMSMode(u8 *msMode);

/*====================================================================
 * 函数名：BrdSetMSMode
 * 功能: 设置当前板卡的主备状态
 *		<0> master status
 *		<1> slave status
 * 算法实现：（可选项）
 * 引用全局变量：
 * 返回值说明： 成功返回OK
 *              失败返回ERROR
 ====================================================================*/
s32 BrdSetMSMode(u8 msMode);

/*====================================================================
 * 函数名： BrdE2promSetMsMode
 * 功能: 设置eeprom用户数据区域
 *		<1> 主备模式
 *		<0> 非主备模式
 * 算法实现：（可选项）
 * 引用全局变量：
 * 返回值说明： 成功返回 0
 *              失败返回 小于零的错误码
 ====================================================================*/
int BrdE2promSetMsMode(u8 udata);

/*====================================================================
 * 函数名： BrdE2promGetMsMode
 * 功能: 获取eeprom用户数据区域
 * 算法实现：（可选项）
 * 引用全局变量：
 * 返回值说明： <1>  主备模式
 *              <0>  非主备模式
 *              <-1> fail
 ====================================================================*/
int BrdE2promGetMsMode();

/*====================================================================
 * 函数名：BrdReInitAllGMPU
 * 功能: 使所有EP侧重启，并加载内核镜像和文件系统。
 * 算法实现：（可选项）
 * 引用全局变量：
 * 返回值说明： 成功返回OK
 *				失败返回ERROR
 ====================================================================*/
s32 BrdReInitAllGMPU();


#ifdef __cplusplus
}
#endif

#endif
