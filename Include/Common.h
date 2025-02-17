/*--------------File Info------------------------------------------------------------------------------------
** File name:			Common.h
** Last modified Date:  2007-07-14
** Last Version:		1.0
** Descriptions:		common head
**
**-----------------------------------------------------------------------------------------------------------
** Created by:			Jiangsongtao
** Created date:		2007-07-14
** Version:				1.0
** Descriptions:		The original version
**
**-----------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Version:
** Descriptions:
**
************************************************************************************************************/
#ifndef COMMON_H_
#define COMMON_H_

//#include <stdint.h>
//#include <stdbool.h>
#include <string.h>

//#include "Cpu.h"
/******************************************************************************
 * Definitions
 ******************************************************************************/
#ifndef TRUE
#define TRUE 				1
#endif

#ifndef FALSE
#define FALSE				0
#endif

#ifndef bool
//typedef unsigned char		bool;                    /* defined for bool variable 	布尔变量  */
#endif
typedef unsigned char		uint8;                   /* defined for unsigned 8-bits integer variable 	无符号8位整型变量  */
typedef signed   char		int8;                    /* defined for signed 8-bits integer variable		有符号8位整型变量  */
typedef unsigned short int	uint16;                  /* defined for unsigned 16-bits integer variable 	无符号16位整型变量 */
typedef signed   short int	int16;                   /* defined for signed 16-bits integer variable 		有符号16位整型变量 */
typedef unsigned long int	uint32;                  /* defined for unsigned 32-bits integer variable 	无符号32位整型变量 */
typedef signed   long int	int32;                   /* defined for signed 32-bits integer variable 		有符号32位整型变量 */
typedef float				fp32;                    /* single precision floating point variable (32bits) 单精度浮点数（32位长度） */
typedef double				fp64;                    /* double precision floating point variable (64bits) 双精度浮点数（64位长度） */

//typedef unsigned char		BOOL;
typedef unsigned char		INT8U;                   /* 无符号8位整型变量                        */
typedef signed   char		INT8S;                   /* 有符号8位整型变量                        */
typedef unsigned short int	INT16U;                  /* 无符号16位整型变量                       */
typedef signed   short int	INT16S;                  /* 有符号16位整型变量                       */
typedef unsigned long int	INT32U;                  /* 无符号32位整型变量                       */
typedef signed   long int	INT32S;                  /* 有符号32位整型变量                       */
typedef float				FP32;                    /* 单精度浮点数（32位长度）                 */
typedef double				FP64;                    /* 双精度浮点数（64位长度）                 */

typedef unsigned char		BYTE;
typedef unsigned short int	WORD;
typedef unsigned long int	DWORD;
//typedef unsigned char		BOOL;

#define CAN_MAX_NUM			2
/*****************************************************************************/
// IO board port define
/*****************************************************************************/
void 	Delay(uint32 time);
void 	DelayUs(uint32 time);
void 	MemSetValueUint8(uint8 *Mem, uint8 Value);
void 	MemSetValueUint16(uint8 *Mem, uint16 Value);
void 	MemSetValueUint32(uint8 *Mem, uint32 Value);
void 	MemSetValueFloat(uint8 *Mem, float Value);
uint8 	MemGetValueUint8(uint8 *Mem);
uint16 	MemGetValueUint16(uint8 *Mem);
uint32 	MemGetValueUint32(uint8 *Mem);
float 	MemGetValueFloat(uint8 *Mem);

#endif /* COMMON_H_ */
/************************************************************************************************************
**                            End Of File
************************************************************************************************************/
