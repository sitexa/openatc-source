/*
 * Kedacom Hardware Abstract Level
 *
 * src/video/vid_sen.h
 *
 * Copyright (C) 2013-2020, Kedacom, Inc.
 *
 * History:
 *   2014/03/20 - [lijun] Create
 *
 */

#ifndef __VID_SENSOR_H
#define __VID_SENSOR_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef USE_KLSP_DEFINES
/*
 * Sensor identify defines
 * use u32 type variable
 *   D[31:24]: sensor vender
 *   D[23:16]: sensor device
 *   D[15:8]: mode ,reserved
 *   D[ 7: 0]: index for each Sensor obj
 */
#define SENSOR_NO_SHIFT            0
#define SENSOR_NO_MASK             0xff
#define SENSOR_NO(v)               (((v) >> SENSOR_NO_SHIFT) & SENSOR_NO_MASK)

#define SENSOR_TYPE_SHIFT          16
#define SENSOR_TYPE_MASK           0xffff
#define SENSOR_TYPE(v)             (((v) >> SENSOR_TYPE_SHIFT) & SENSOR_TYPE_MASK)

#define SENSOR_VEND_SHIFT          24
#define SENSOR_VEND_MASK           0xff
#define SENSOR_VEND(v)            (((v) >> SENSOR_VEND_SHIFT) & SENSOR_VEND_MASK)

#define SENSOR_VEND_NONE           0 /* UNKNOWN*/
#define SENSOR_VEND_PANASONIC      1 /* Panasonic sensor */
#define SENSOR_VEND_SONY           2 /* SONY sensor */
#define SENSOR_VEND_APTINA         3 /* Aptina sensor */
#define SENSOR_VEND_OMNIVISION     4 /* OmniVision sensor */
#define SENSOR_VEND_ALTASENS       5 /* ALTASENS sensor */
#define SENSOR_VEND_KODAK          6 /* Kodak sensor */
#define SENSOR_VEND_SOI            7 /* SOI sensor */
#define SENSOR_VEND_HIMAX          8 /* Himax sensor */
#define SENSOR_VEND_AD             9 /* Extern Chip Input*/
#define SENSOR_VEND_FPGA          10 /* FPGA Input */
#define SENSOR_VEND_SMARTSENS     11 /* smartsens sensor */
#define SENSOR_VEND_E2V           12 /* teledyne e2v */

#define SENSOR_TYPE_PANASONIC_MN34041  (0x01 | (SENSOR_VEND_PANASONIC << \
                         (SENSOR_VEND_SHIFT - SENSOR_TYPE_SHIFT)))
#define SENSOR_TYPE_PANASONIC_MN34031  (0x02 | (SENSOR_VEND_PANASONIC << \
                         (SENSOR_VEND_SHIFT - SENSOR_TYPE_SHIFT)))
#define SENSOR_TYPE_PANASONIC_MN34210  (0x03 | (SENSOR_VEND_PANASONIC << \
                         (SENSOR_VEND_SHIFT - SENSOR_TYPE_SHIFT)))
#define SENSOR_TYPE_PANASONIC_MN34220  (0x04 | (SENSOR_VEND_PANASONIC << \
                         (SENSOR_VEND_SHIFT - SENSOR_TYPE_SHIFT)))

#define SENSOR_TYPE_SONY_ICX274        (0x01 | (SENSOR_VEND_SONY << \
                         (SENSOR_VEND_SHIFT - SENSOR_TYPE_SHIFT)))
#define SENSOR_TYPE_SONY_IMX036        (0x02 | (SENSOR_VEND_SONY << \
                         (SENSOR_VEND_SHIFT - SENSOR_TYPE_SHIFT)))
#define SENSOR_TYPE_SONY_IMX236        (0x03 | (SENSOR_VEND_SONY << \
                         (SENSOR_VEND_SHIFT - SENSOR_TYPE_SHIFT)))
#define SENSOR_TYPE_SONY_IMX122        (0x04 | (SENSOR_VEND_SONY << \
                         (SENSOR_VEND_SHIFT - SENSOR_TYPE_SHIFT)))
#define SENSOR_TYPE_SONY_ICX692        (0x05 | (SENSOR_VEND_SONY << \
                         (SENSOR_VEND_SHIFT - SENSOR_TYPE_SHIFT)))
#define SENSOR_TYPE_SONY_IMX104        (0x06 | (SENSOR_VEND_SONY << \
                         (SENSOR_VEND_SHIFT - SENSOR_TYPE_SHIFT)))
#define SENSOR_TYPE_SONY_IMX138        (0x07 | (SENSOR_VEND_SONY << \
                         (SENSOR_VEND_SHIFT - SENSOR_TYPE_SHIFT)))
#define SENSOR_TYPE_SONY_IMX185        (0x08 | (SENSOR_VEND_SONY << \
                         (SENSOR_VEND_SHIFT - SENSOR_TYPE_SHIFT)))
#define SENSOR_TYPE_SONY_IMX178        (0x09 | (SENSOR_VEND_SONY << \
                         (SENSOR_VEND_SHIFT - SENSOR_TYPE_SHIFT)))
#define SENSOR_TYPE_SONY_IMX226        (0x0A | (SENSOR_VEND_SONY << \
                         (SENSOR_VEND_SHIFT - SENSOR_TYPE_SHIFT)))
#define SENSOR_TYPE_SONY_IMX290        (0x0B | (SENSOR_VEND_SONY << \
                         (SENSOR_VEND_SHIFT - SENSOR_TYPE_SHIFT)))
#define SENSOR_TYPE_SONY_IMX290_LVDS   (0x0C | (SENSOR_VEND_SONY << \
                         (SENSOR_VEND_SHIFT - SENSOR_TYPE_SHIFT)))
#define SENSOR_TYPE_SONY_IMX323        (0x0D | (SENSOR_VEND_SONY << \
                         (SENSOR_VEND_SHIFT - SENSOR_TYPE_SHIFT)))
#define SENSOR_TYPE_SONY_IMX477        (0x0E | (SENSOR_VEND_SONY << \
                         (SENSOR_VEND_SHIFT - SENSOR_TYPE_SHIFT)))
#define SENSOR_TYPE_SONY_IMX299        (0x0F | (SENSOR_VEND_SONY << \
                         (SENSOR_VEND_SHIFT - SENSOR_TYPE_SHIFT)))
#define SENSOR_TYPE_SONY_IMX334        (0x10 | (SENSOR_VEND_SONY << \
                         (SENSOR_VEND_SHIFT - SENSOR_TYPE_SHIFT)))
#define SENSOR_TYPE_SONY_IMX334_SLAVE  (0x11 | (SENSOR_VEND_SONY << \
                         (SENSOR_VEND_SHIFT - SENSOR_TYPE_SHIFT)))
#define SENSOR_TYPE_SONY_IMX327        (0x12 | (SENSOR_VEND_SONY << \
                         (SENSOR_VEND_SHIFT - SENSOR_TYPE_SHIFT)))
#define SENSOR_TYPE_SONY_IMX294        (0x13 | (SENSOR_VEND_SONY << \
                         (SENSOR_VEND_SHIFT - SENSOR_TYPE_SHIFT)))
#define SENSOR_TYPE_SONY_IMX305        (0x14 | (SENSOR_VEND_SONY << \
                         (SENSOR_VEND_SHIFT - SENSOR_TYPE_SHIFT)))
#define SENSOR_TYPE_SONY_IMX385        (0x15 | (SENSOR_VEND_SONY << \
                         (SENSOR_VEND_SHIFT - SENSOR_TYPE_SHIFT)))
#define SENSOR_TYPE_SONY_IMX265        (0x16 | (SENSOR_VEND_SONY << \
                         (SENSOR_VEND_SHIFT - SENSOR_TYPE_SHIFT)))
#define SENSOR_TYPE_SONY_IMX307        (0x17 | (SENSOR_VEND_SONY << \
                         (SENSOR_VEND_SHIFT - SENSOR_TYPE_SHIFT)))
#define SENSOR_TYPE_SONY_IMX335        (0x18 | (SENSOR_VEND_SONY << \
                         (SENSOR_VEND_SHIFT - SENSOR_TYPE_SHIFT)))
#define SENSOR_TYPE_SONY_IMX458        (0x19 | (SENSOR_VEND_SONY << \
                         (SENSOR_VEND_SHIFT - SENSOR_TYPE_SHIFT)))


#define SENSOR_TYPE_APTINA_AR0330      (0x01 | (SENSOR_VEND_APTINA << \
                         (SENSOR_VEND_SHIFT - SENSOR_TYPE_SHIFT)))
#define SENSOR_TYPE_APTINA_AR0331      (0x02 | (SENSOR_VEND_APTINA << \
                         (SENSOR_VEND_SHIFT - SENSOR_TYPE_SHIFT)))
#define SENSOR_TYPE_APTINA_AR0130      (0x03 | (SENSOR_VEND_APTINA << \
                         (SENSOR_VEND_SHIFT - SENSOR_TYPE_SHIFT)))
#define SENSOR_TYPE_APTINA_MT9M034     (0x04 | (SENSOR_VEND_APTINA << \
                         (SENSOR_VEND_SHIFT - SENSOR_TYPE_SHIFT)))
#define SENSOR_TYPE_APTINA_MT9P006     (0x05 | (SENSOR_VEND_APTINA << \
                         (SENSOR_VEND_SHIFT - SENSOR_TYPE_SHIFT)))
#define SENSOR_TYPE_APTINA_MT9P031     (0x06 | (SENSOR_VEND_APTINA << \
                         (SENSOR_VEND_SHIFT - SENSOR_TYPE_SHIFT)))
#define SENSOR_TYPE_APTINA_AR0140     (0x07 | (SENSOR_VEND_APTINA << \
                         (SENSOR_VEND_SHIFT - SENSOR_TYPE_SHIFT)))
#define SENSOR_TYPE_APTINA_AR0141      (0x08 | (SENSOR_VEND_APTINA << \
                         (SENSOR_VEND_SHIFT - SENSOR_TYPE_SHIFT)))

#define SENSOR_TYPE_SMARTSENS_MARS400W (0x01 | (SENSOR_VEND_SMARTSENS << \
                         (SENSOR_VEND_SHIFT - SENSOR_TYPE_SHIFT)))

#define SENSOR_TYPE_OMNIVISION_OV2715  (0x01 | (SENSOR_VEND_OMNIVISION << \
                         (SENSOR_VEND_SHIFT - SENSOR_TYPE_SHIFT)))
#define SENSOR_TYPE_OMNIVISION_OV5653  (0x02 | (SENSOR_VEND_OMNIVISION << \
                         (SENSOR_VEND_SHIFT - SENSOR_TYPE_SHIFT)))
#define SENSOR_TYPE_OMNIVISION_OV2710  (0x03 | (SENSOR_VEND_OMNIVISION << \
                         (SENSOR_VEND_SHIFT - SENSOR_TYPE_SHIFT)))
#define SENSOR_TYPE_OMNIVISION_OV9712  (0x04 | (SENSOR_VEND_OMNIVISION << \
                         (SENSOR_VEND_SHIFT - SENSOR_TYPE_SHIFT)))

#define SENSOR_TYPE_OMNIVISION_OV4689  (0x05 | (SENSOR_VEND_OMNIVISION << \
                         (SENSOR_VEND_SHIFT - SENSOR_TYPE_SHIFT)))
#define SENSOR_TYPE_OMNIVISION_OS04A10 (0x06 | (SENSOR_VEND_OMNIVISION << \
                         (SENSOR_VEND_SHIFT - SENSOR_TYPE_SHIFT)))
#define SENSOR_TYPE_OMNIVISION_OS05A   (0x07 | (SENSOR_VEND_OMNIVISION << \
                         (SENSOR_VEND_SHIFT - SENSOR_TYPE_SHIFT)))

#define SENSOR_TYPE_E2V_EV2S8M9_CLV0150 (0x01 | (SENSOR_VEND_E2V << \
                         (SENSOR_VEND_SHIFT - SENSOR_TYPE_SHIFT)))

#define SENSOR_TYPE_ALTASENS_AS3372    (0x01 | (SENSOR_VEND_ALTASENS << \
                         (SENSOR_VEND_SHIFT - SENSOR_TYPE_SHIFT)))

#define SENSOR_TYPE_KODAK_KAI02150     (0x01 | (SENSOR_VEND_KODAK << \
                         (SENSOR_VEND_SHIFT - SENSOR_TYPE_SHIFT)))

#define SENSOR_TYPE_SOI_H22            (0x01 | (SENSOR_VEND_SOI << \
                         (SENSOR_VEND_SHIFT - SENSOR_TYPE_SHIFT)))

#define SENSOR_TYPE_HIMAX_1375         (0x01 | (SENSOR_VEND_HIMAX << \
                         (SENSOR_VEND_SHIFT - SENSOR_TYPE_SHIFT)))

#define SENSOR_TYPE_AD                 (0x01 | (SENSOR_VEND_AD << \
                         (SENSOR_VEND_SHIFT - SENSOR_TYPE_SHIFT)))

#define SENSOR_TYPE_FPGA               (0x01 | (SENSOR_VEND_FPGA << \
                         (SENSOR_VEND_SHIFT - SENSOR_TYPE_SHIFT)))

#define SENSOR_ID(type, no) \
	(((type)  << SENSOR_TYPE_SHIFT) | \
	 ((no)    << SENSOR_NO_SHIFT))

/* predefined sensor identify code */
#define SENSOR_ID_NONE  0

#define SENSOR_ID_PANASONIC_MN34041(n)   SENSOR_ID \
                                            (SENSOR_TYPE_PANASONIC_MN34041, n)
#define SENSOR_ID_PANASONIC_MN34031(n)   SENSOR_ID \
                                            (SENSOR_TYPE_PANASONIC_MN34031, n)
#define SENSOR_ID_PANASONIC_MN34210(n)   SENSOR_ID \
                                            (SENSOR_TYPE_PANASONIC_MN34210, n)
#define SENSOR_ID_PANASONIC_MN34220(n)   SENSOR_ID \
                                            (SENSOR_TYPE_PANASONIC_MN34220, n)

#define SENSOR_ID_SONY_IMX036(n)         SENSOR_ID \
                                            (SENSOR_TYPE_SONY_IMX036, n)
#define SENSOR_ID_SONY_IMX236(n)         SENSOR_ID \
                                            (SENSOR_TYPE_SONY_IMX236, n)
#define SENSOR_ID_SONY_IMX122(n)         SENSOR_ID \
                                            (SENSOR_TYPE_SONY_IMX122, n)
#define SENSOR_ID_SONY_ICX692(n)         SENSOR_ID \
                                            (SENSOR_TYPE_SONY_ICX692, n)
#define SENSOR_ID_SONY_IMX104(n)         SENSOR_ID \
                                            (SENSOR_TYPE_SONY_IMX104, n)
#define SENSOR_ID_SONY_IMX138(n)         SENSOR_ID \
                                            (SENSOR_TYPE_SONY_IMX138, n)
#define SENSOR_ID_SONY_IMX185(n)         SENSOR_ID \
                                            (SENSOR_TYPE_SONY_IMX185, n)
#define SENSOR_ID_SONY_IMX178(n)         SENSOR_ID \
                                            (SENSOR_TYPE_SONY_IMX178, n)
#define SENSOR_ID_SONY_IMX226(n)         SENSOR_ID \
                                            (SENSOR_TYPE_SONY_IMX226, n)
#define SENSOR_ID_SONY_IMX290(n)         SENSOR_ID \
                                            (SENSOR_TYPE_SONY_IMX290, n)
#define SENSOR_ID_SONY_IMX290_LVDS(n)    SENSOR_ID \
                                            (SENSOR_TYPE_SONY_IMX290_LVDS, n)
#define SENSOR_ID_SONY_IMX323(n)         SENSOR_ID \
                                            (SENSOR_TYPE_SONY_IMX323, n)
#define SENSOR_ID_SONY_IMX327(n)         SENSOR_ID \
                                            (SENSOR_TYPE_SONY_IMX327, n)
#define SENSOR_ID_SONY_IMX477(n)         SENSOR_ID \
                                            (SENSOR_TYPE_SONY_IMX477, n)
#define SENSOR_ID_SONY_IMX299(n)         SENSOR_ID \
                                            (SENSOR_TYPE_SONY_IMX299, n)
#define SENSOR_ID_SONY_IMX334(n)         SENSOR_ID \
                                            (SENSOR_TYPE_SONY_IMX334, n)
#define SENSOR_ID_SONY_IMX334_SLAVE(n)   SENSOR_ID \
                                            (SENSOR_TYPE_SONY_IMX334_SLAVE, n)
#define SENSOR_ID_SONY_IMX294(n)         SENSOR_ID \
                                            (SENSOR_TYPE_SONY_IMX294, n)
#define SENSOR_ID_SONY_IMX305(n)         SENSOR_ID \
                                            (SENSOR_TYPE_SONY_IMX305, n)
#define SENSOR_ID_SONY_IMX385(n)         SENSOR_ID \
                                            (SENSOR_TYPE_SONY_IMX385, n)
#define SENSOR_ID_SONY_IMX265(n)         SENSOR_ID \
                                            (SENSOR_TYPE_SONY_IMX265, n)
#define SENSOR_ID_SONY_IMX307(n)         SENSOR_ID \
                                            (SENSOR_TYPE_SONY_IMX307, n)
#define SENSOR_ID_SONY_IMX335(n)         SENSOR_ID \
                                            (SENSOR_TYPE_SONY_IMX335, n)
#define SENSOR_ID_SONY_IMX458(n)         SENSOR_ID \
                                            (SENSOR_TYPE_SONY_IMX458, n)


#define SENSOR_ID_SMARTSENS_MARS400W(n)  SENSOR_ID \
                                            (SENSOR_TYPE_SMARTSENS_MARS400W, n)

#define SENSOR_ID_APTINA_AR0330(n)       SENSOR_ID \
                                            (SENSOR_TYPE_APTINA_AR0330, n)
#define SENSOR_ID_APTINA_AR0331(n)       SENSOR_ID \
                                            (SENSOR_TYPE_APTINA_AR0331, n)
#define SENSOR_ID_APTINA_AR0130(n)       SENSOR_ID \
                                            (SENSOR_TYPE_APTINA_AR0130, n)
#define SENSOR_ID_APTINA_MT9M034(n)      SENSOR_ID \
                                            (SENSOR_TYPE_APTINA_MT9M034, n)
#define SENSOR_ID_APTINA_MT9P006(n)      SENSOR_ID \
                                            (SENSOR_TYPE_APTINA_MT9P006, n)
#define SENSOR_ID_APTINA_AR0140(n)       SENSOR_ID \
                                            (SENSOR_TYPE_APTINA_AR0140, n)
#define SENSOR_ID_APTINA_AR0141(n)       SENSOR_ID \
                                            (SENSOR_TYPE_APTINA_AR0141, n)

#define SENSOR_ID_OMNI_OV2710(n)         SENSOR_ID \
                                            (SENSOR_TYPE_OMNIVISION_OV2710, n)
#define SENSOR_ID_OMNI_OV9712(n)         SENSOR_ID \
                                            (SENSOR_TYPE_OMNIVISION_OV9712, n)
#define SENSOR_ID_OMNI_OV4689(n)         SENSOR_ID \
                                             (SENSOR_TYPE_OMNIVISION_OV4689, n)
#define SENSOR_ID_OMNI_OS04A10(n)        SENSOR_ID \
                                              (SENSOR_TYPE_OMNIVISION_OS04A10, n)
#define SENSOR_ID_OMNI_OS05A(n)          SENSOR_ID \
                                              (SENSOR_TYPE_OMNIVISION_OS05A, n)

#define SENSOR_ID_E2V_EV2S8M9_CLV0150(n) SENSOR_ID \
                                              (SENSOR_TYPE_E2V_EV2S8M9_CLV0150, n)

#define SENSOR_ID_SOI_H22(n)             SENSOR_ID \
                                            (SENSOR_TYPE_SOI_H22, n)

#define SENSOR_ID_HIMAX_1375(n)          SENSOR_ID \
                                            (SENSOR_TYPE_HIMAX_1375, n)

#define SENSOR_ID_AD(n)                  SENSOR_ID \
                                            (SENSOR_TYPE_AD, n)

#define SENSOR_ID_FPGA(n)                SENSOR_ID \
                                            (SENSOR_TYPE_FPGA, n)
/* sensor capability mask defines */
#define SENSOR_CAB_NONE           0x00000000 /* no support */

/* 0: Mipi 1: LVDS 2:MIPI-AD 3.MIPI-DC 4:SLVS 5:BT1120 6:BT656 7:HiSPI 8:ViCap (0~7 input to MIPI Controller, 8 input to VICAP  ) */
#define SENSOR_INTF_MIPI           0
#define SENSOR_INTF_LVDS           1
#define SENSOR_INTF_MIPI_AD        2
#define SENSOR_INTF_MIPI_DC        3
#define SENSOR_INTF_SLVS           4
#define SENSOR_INTF_BT1120         5
#define SENSOR_INTF_BT656          6
#define SENSOR_INTF_HiSPI          7
#define SENSOR_INTF_VICAP          8

struct sensor_info {
	u32 no;           /* input: -1 = return sensor_registed_num;
	                       input: 0 ~ sensor_registed_num-1, return detail */

	u32 sensor_id;      /* see also: SENSOR_ID_APTINA_AR0130 */
	u32 cab;          /* capability mask, see also: SENSOR_CAB_IRCUT */
	u32 intf;          /* 0: Mipi 1: LVDS 2:MIPI-AD 3.MIPI-DC 4:SLVS 5:BT1120 6:BT656 7:HiSPI 8:ViCap (0~7 input to MIPI Controller, 8 input to VICAP  ) */
};

struct sensor_params {
	u32   sensor_id;     /* see also: SENSOR_ID_APTINA_AR0130 */
	u32   val32[16];   /* params */
};


#define SENSOR_PROC_DEV               "sensor"
#define SENSOR_IOC                    'L'
#define SENSOR_MOD_VER                0x00000001
#define SENSOR_MAX_NUM                10

#else
/* refer to kernel/driver/klsp */
#include <video/sensor/sensor_drv.h>
#endif /* USE_KLSP_DEFINES */


enum eVidSensorCmd {
	SENSOR_CTRL_GET_MOD_VER = 0,
	SENSOR_CTRL_QUERY = 1,
};

/* APIs */
/*
 * request sensor type
 */
int VidSensorCtrl(int idx, int cmd, void *args);


#ifdef __cplusplus
}
#endif

#endif /* end __VID_SENSOR_H */
