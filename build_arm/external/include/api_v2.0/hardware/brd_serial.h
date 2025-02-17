/*
 * Kedacom Hardware Abstract Level
 *
 * brd_serial.h
 *
 * Copyright (C) 2013-2020, Kedacom, Inc.
 *
 * History:
 *   2013/09/22 - [xuliqin] Create
 *
 */

#ifndef _BRD_SERIAL_H
#define _BRD_SERIAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../drvlib_def.h"

#ifndef USE_KLSP_DEFINES
/*serial type*/
#define SERIAL_RS232       		0
#define SERIAL_RS422       		1
#define SERIAL_RS485       		2
#define SERIAL_RS232_422_485            3
#define SERIAL_BLUETOOTH                4
#define SERIAL_VS100                    5
#define SERIAL_UNKNOWN                  6

/* serial usage */
#define SERIAL_CONSOLE                  0
#define SERIAL_INFRARED                 1
#define SERIAL_VISCA                    2
#define SERIAL_COMMU                    3
#define SERIAL_DIGMIC                   4
#define SERIAL_UNUSED                   5

#endif /* USE_KLSP_DEFINES */

typedef struct
{
	u32   dwNo;       /* Input: 0 ~ serial_num-1 */
	u32   dwType;     /* see also: SERIAL_RS232 */
	u32   dwUsage;    /* see also: SERIAL_CONSOLE */
	u32   dwFixBaudrate; /* 0 = no limit; else fixed, such as 115200 */

	char  achName[DRVLIB_TTY_NAME_MAX_LEN]; /* name */
} TSerialInfo;

/*
 * Serial Defines
 */
#define SIO_SET_BAUDRATE     0x2000
#define SIO_GET_BAUDRATE     0x2001
#define SIO_SET_STOPBIT      0x2002
#define SIO_GET_STOPBIT      0x2003
#define SIO_SET_DATABIT      0x2004
#define SIO_GET_DATABIT      0x2005
#define SIO_SET_PARITY       0x2006
#define SIO_GET_PARITY       0x2007
/*set serial unblock use by BrdSerialIoctl function*/
#define SIO_SET_UNBLOCK      0x2008
/*set serial block use by BrdSerialIoctl function*/
#define SIO_SET_BLOCK        0x2009
/*set serial no wait use by BrdSerialIoctl function*/
#define SIO_SET_NOWAIT       0x200a

#define SIO_PARITY_NONE      0
#define SIO_PARITY_ODD       1
#define SIO_PARITY_EVEN      2
#define SIO_PARITY_MARK      3
#define SIO_PARITY_SPACE     4

#define SIO_STOPBIT_1        0
#define SIO_STOPBIT_2        1
/*
 * Features:Query basic information about the serial port, such as ID, type, device path name, and usage.
 * Parameter Description:ptInfo:
 * The dwNo field needs to be filled in. The index number ranges from 0 to the number of serial ports on the board. The number of indicators on the board can be obtained through TBrdInfo.wSerialNum.
 * Used to return the corresponding information retrieval structure
 * return:0 success,other failure
 */
int BrdSerialQueryInfo(TSerialInfo *ptInfo);
/*
 * Features:Open serial device
 * Parameter Description:To open the serial port information, you need to call BrdSerialQueryInfo to get
 * return:Successfully returns a handle of >=0; fails returns an error code less than 0
 */
int BrdSerialOpen(TSerialInfo *ptInfo);
/*
 * Features:close serial
 * Parameter Description:fd:The serial port handle to be closed, returned by BrdSerialOpen
 * return:0 success,other failure
 */
int BrdSerialClose(int nFd);
/*
 * Features:Control serial device
 * Parameter Description:nFd: serial port handle
 * return:0 success,other failure
 */
int BrdSerialIoctl(int nFd, int nFunc, void *pArgs);
/*
 * Features:Read data from a serial device
 * Parameter Description:
 * Fd:Serial port handle, obtained by BrdSerialOpen
 * Buff:Data cache
 * Length:Maximum read length
 * return:Read successfully returns the number of bytes read, 0 means no data; failure returns an error code less than 0
 */
int BrdSerialRead(int nFd, u8 *pbyBuff, int nLength);
/*
 * Features:Write data to the serial device
 * Parameter Description:
 * Fd:Serial port handle, obtained by BrdSerialOpen
 * Buff:Data cache
 * Length:Length to be written
 * return:Successfully returns the number of bytes written; if it fails, it returns an error code less than 0.
 */
int BrdSerialWrite(int nFd, const u8 *pbyBuff, int nLength);
int BrdSerialSetMode(int nMode);
int BrdSerialSelConsole(int nCpuId);

#ifdef __cplusplus
}
#endif

#endif
