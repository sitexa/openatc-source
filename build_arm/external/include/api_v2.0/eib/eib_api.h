/******************************************************************************
 * BIM M 13x Serial Communication
 *
 * File: main.c
 *
 * Date: 2018/3/14
 *
 * Version: 001
 *
 * Author: Li Yaqiang
 *****************************************************************************/
#ifndef EIB_API_H_
#define EIB_API_H_

#ifdef __cplusplus
extern "C" {
#endif

enum ERROR_CODE {
	EIB_OK          = 0,
	EIB_TIMEOUT     = 1,
	EIB_IO          = 2,
	EIB_NODEV       = 3,
	EIB_INVAL       = 4,
	EIB_NOMEM       = 5,
	EIB_DISCONNECT  = 6,
	EIB_NO_INIT     = 7,
};

/* Command */
#define GRPADDR_WRITE_REQUEST         0xB1
#define GRPADDR_WRITE_CONFIRM         0xB2
#define GRPADDR_WRITE_INDICATION      0xB3
#define GRPADDR_READ_REQUEST          0xB5
#define GRPADDR_READ_CONFIRM          0xB6
#define GRPADDR_READ_INDICATION       0xB7
#define GRPADDR_RESPONSE_REQUEST      0xB9
#define GRPADDR_RESPONSE_CONFIRM      0xBA
#define GRPADDR_RESPONSE_INDICATION   0xBB

typedef struct {
	u8 byCmd; /* 0xB1~0xBB */
	u16 wGroupAddr; /* 0~15.0~7.0~255 */
	u8 byDataLenght;
	u8 abyData[15];
} TMessage;

/* timeout: msec */
int EibReadMsg(TMessage *ptMsg, u32 dwTimeOut);
int EibWriteMsg(TMessage *ptMsg);
int EibInit(char *szDevName, u32 dwBaudrate, u8 byDataBit, u8 byStopBit, u8 byParity);
void EibRelease();
void EibSetDebugFlag(BOOL32 bEnDbg);

#ifdef __cplusplus
}
#endif

#endif
