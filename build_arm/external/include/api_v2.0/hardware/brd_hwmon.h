/*
 * Kedacom Hardware Abstract Level
 *
 * brd_hwmon.h
 *
 * Copyright (C) 2013-2020, Kedacom, Inc.
 *
 * History:
 *   2013/09/22 - [xuliqin] Create
 *
 */

#ifndef _BRD_MONITOR_H
#define _BRD_MONITOR_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../drvlib_def.h"

#ifndef USE_KLSP_DEFINES

/*
 * Hardware monitor definition
 * use u32 type variable
 *   D[31:16]: hwmon type
 *   D[ 7: 0]: index for each hwmon type
 */
#define HWMON_NO_SHIFT          0
#define HWMON_NO_MASK           0xff
#define HWMON_NO(v)             (((v) >> HWMON_NO_SHIFT) & HWMON_NO_MASK)

#define HWMON_TYPE_SHIFT        16
#define HWMON_TYPE_MASK         0xffff
#define HWMON_TYPE(v)           (((v) >> HWMON_TYPE_SHIFT) & HWMON_TYPE_MASK)

#define HWMON_TYPE_LINE_IN      0 /* line input  on/off */
#define HWMON_TYPE_LINE_OUT     1 /* line output on/off */
#define HWMON_TYPE_FAN_SPEED    2 /* fan speed */
#define HWMON_TYPE_TEMP         3 /* temperature */
#define HWMON_TYPE_VOLT         4 /* voltage */
#define HWMON_TYPE_BRIGHT       5 /* brightness */
#define HWMON_TYPE_HEATER       6 /* heater */
#define HWMON_TYPE_MAGNETIC     7 /* 3-axis digital magnetometer */
#define HWMON_TYPE_ACCELE       8 /* 3-axis digital accelerometer */
#define HWMON_TYPE_PRESSURE     9 /* buzzer */
#define HWMON_TYPE_HUMIDITY     10 /* humidity */
#define HWMON_TYPE_DOOR_OPEN    11 /* open door input*/
#define HWMON_TYPE_DOOR_DETECT  12 /* door open detect input*/
#define HWMON_TYPE_DOOR_CTL     13 /* door ctl output*/
#define HWMON_TYPE_GYRO         14 /* mpu6500*/
#define HWMON_TYPE_LEVEL        15/* board IO level,it should be period,not peak */
#define HWMON_TYPE_BUZZER       16 /* buzzer */
#define HWMON_TYPE_BATTERY      17 /* battery */

#define HWMON_ID(type, no) \
	(((type) << HWMON_TYPE_SHIFT) | \
	 ((no)   << HWMON_NO_SHIFT))

#define HWMON_ID_LINE_IN(n)     HWMON_ID(HWMON_TYPE_LINE_IN,   n)
#define HWMON_ID_LINE_OUT(n)    HWMON_ID(HWMON_TYPE_LINE_OUT,  n)
#define HWMON_ID_FAN_SPEED(n)   HWMON_ID(HWMON_TYPE_FAN_SPEED, n)
#define HWMON_ID_TEMP(n)        HWMON_ID(HWMON_TYPE_TEMP,      n)
#define HWMON_ID_VOLT(n)        HWMON_ID(HWMON_TYPE_VOLT,      n)
#define HWMON_ID_BRIGHT(n)      HWMON_ID(HWMON_TYPE_BRIGHT,    n)
#define HWMON_ID_HEATER(n)      HWMON_ID(HWMON_TYPE_HEATER,    n)
#define HWMON_ID_MAGNETIC(n)    HWMON_ID(HWMON_TYPE_MAGNETIC,  n)
#define HWMON_ID_ACCELE(n)      HWMON_ID(HWMON_TYPE_ACCELE,    n)
#define HWMON_ID_PRESSURE(n)    HWMON_ID(HWMON_TYPE_PRESSURE,  n)
#define HWMON_ID_HUMIDITY(n)    HWMON_ID(HWMON_TYPE_HUMIDITY , n)
#define HWMON_ID_DOOR_OPEN(n)   HWMON_ID(HWMON_TYPE_DOOR_OPEN ,n)
#define HWMON_ID_DOOR_DETECT(n) HWMON_ID(HWMON_TYPE_DOOR_DETECT ,n)
#define HWMON_ID_DOOR_CTL(n)    HWMON_ID(HWMON_TYPE_DOOR_CTL , n)
#define HWMON_ID_GYRO(n)        HWMON_ID(HWMON_TYPE_GYRO,  n)
#define HWMON_ID_LEVEL(n)       HWMON_ID(HWMON_TYPE_LEVEL,    n)
#define HWMON_ID_BUZZER(n)      HWMON_ID(HWMON_TYPE_BUZZER,  n)
#define HWMON_ID_BATTERY(n)     HWMON_ID(HWMON_TYPE_BATTERY,  n)


#endif /* USE_KLSP_DEFINES */

/* fan info: globle <struct fan_info board_fans[CFG_FAN_NUM];> */
typedef struct
{
	u32   dwCab;        /* see also: FAN_CAB_SPEED_SET */
	u32   dwSpeedMax;  /*  rotate speed, in RPM or 0-100(%) */
	u32   dwSpeedCur;  /* cuttent rotate speed, in RPM or 0-100(%) */
	u32   dwSpeedAlarm;/* if (speed_cur < speed_alarm) report alarm */
} TFanInfo;

/* temperature info */
typedef struct
{
	s32   dwCur;        /* cuttent temperature, in 0.001 degree C */
	s32   dwAlarmMin;  /* if (cur < alarm_min) report alarm */
	s32   dwAlarmMax;  /* if (cur > alarm_max) report alarm */
} TTempInfo;

/* voltage info */
typedef struct
{
	s32   dwCur;        /* cuttent voltage, in mVolt */
	s32   dwAlarmMin;  /* if (cur < alarm_min) report alarm */
	s32   dwAlarmMax;  /* if (cur > alarm_max) report alarm */
} TVoltInfo;

/* brightness info */
typedef struct {
        u32   dwCur;        /* cuttent brightness, in LM */
        u32   dwAlarmMin;  /* if (cur < alarm_min) report alarm */
        u32   dwAlarmMax;  /* if (cur > alarm_max) report alarm */
}TBrightInfo;

/* magnetometer info */
typedef struct {
	s32   x; /* X-axis 16-bit output sample data of the magnetic field
	            strength expressed as signed 2's complement numbers */
	s32   y; /* Y-axis 16-bit output sample data of the magnetic field
	            strength expressed as signed 2's complement numbers */
	s32   z; /* Z-axis 16-bit output sample data of the magnetic field
	            strength expressed as signed 2's complement numbers */
}TMagneticInfo;

/* accelerometer info */
typedef struct {
	s32   x; /* X-axis measured acceleration data */
	s32   y; /* Y-axis measured acceleration data */
	s32   z; /* Z-axis measured acceleration data */
}TAcceleInfo;

/* heater info */
typedef struct {
	u32   id;         /* device id */
	u32   on_off;     /* switch for heater 1:on 0:off */
	u32   temper;     /* heater temperature, in 0.001 degree C */
}THeatInfo;

typedef struct
{
	u32   dwNo;    /* Input: 0 ~ hwmon_num-1 */

	u32   dwId;    /* see also: HWMON_ID_TEMP */
	char  achName[DRVLIB_NAME_MAX_LEN]; /* hwmon obj name */
} THwmonInfo;

/* buzzer info */
typedef struct {
	u32   id;         /* device id */
	u32   on_off;     /* buzzer 0:close,1:open */
}TBuzzerInfo;

/* battery info */
typedef struct {
	u32   id;         /* device id */
	u32   cap_ratio;  /* remain capacity ratio */
}TBatteryInfo;

/* level info */
typedef struct {
	u32   id;         /* device id */
	u8  level;      /* IO status:  0 or 1 */
}TLevelInfo;

/* humidity info */
typedef struct {
	u32   cur;        /* cuttent humidity, in % */
	u32   alarm_min;  /* if (cur < alarm_min) report alarm */
	u32   alarm_max;  /* if (cur > alarm_max) report alarm */
}ThumidityInfo;

typedef struct
{
	u32   dwId;    /* see also: HWMON_ID_TEMP */
	int   nAlarm; /* hwmon state: 0 = normal; 1 = alarm on */

	union
	{
		u8 abyParam[256];
		TFanInfo    tFan;
		TTempInfo   tTemp;
		TVoltInfo   tVolt;
		TBrightInfo tBright;
		TMagneticInfo tMagnetic;
		TAcceleInfo tAccele;
		THeatInfo tHeater;
		TBuzzerInfo tBuzzer;
		TBatteryInfo tBattery;
		TLevelInfo tLevel;
		ThumidityInfo tHumidity;
	} tEntry;           /* append param for each HWMON_TYPE(id) */
} THwmonStat;

int BrdHwmonQueryInfo(THwmonInfo *ptInfo);
int BrdHwmonGetStatus(THwmonStat *ptStat);
int BrdHwmonSetStatus(THwmonStat *ptStat);

#ifdef __cplusplus
}
#endif

#endif
