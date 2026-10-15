/**
 * @file sdrv_pvt.h
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:pvt lld driver header file
 *
 * Revision History:
 * -----------------
 */

#ifndef INCLUDE_SDRV_PVT_H
#define INCLUDE_SDRV_PVT_H

/** macro define section */
#define PVT_REG_DISABLE 0
#define PVT_REG_ENABLE  1

/** enum define section */
/**
 * @brief sensor sample mode
 *
 */
typedef enum {
    PVT_SENSOR_SAMPLE_MODE_TEMP,
    PVT_SENSOR_SAMPLE_MODE_VOLTAGE,
    PVT_SENSOR_SAMPLE_MODE_PROCESS,

    PVT_SENSOR_SAMPLE_MODE_INVALID,
}pvt_sample_mode_enum_t;

/**
 * @brief sensor wrapper alarm chan
 *
 */
typedef enum {
    PVT_SENSOR_ALARM_CHAN0,
    PVT_SENSOR_ALARM_CHAN1,

    PVT_SENSOR_ALARM_CHANNUM,
}pvt_alarm_chan_emum_t;

/**
 * @brief pvt sensor alarm type
 *
 */
typedef enum {
    PVT_SENSOR_ALARM_TYPE_HYST_H = 1 << 0,
    PVT_SENSOR_ALARM_TYPE_HYST_L = 1 << 1,
    PVT_SENSOR_ALARM_TYPE_HYST_R = 1 << 2,
    PVT_SENSOR_ALARM_TYPE_HYST_F = 1 << 3,
}pvt_sensor_alarm_type_enum_t;

/**
 * @brief sensor pn type
 *
 */
typedef enum {
    PVT_SENSOR_TYPE_LVT,
    PVT_SENSOR_TYPE_ULVT,
    PVT_SENSOR_TYPE_SVT,

    PVT_SENSOR_TYPE_NUM,
}pvt_sensor_type_enum_t;

int32_t sdrv_pvt_lld_set_ctrl_ctlMode(paddr_t base, int32_t mode);
int32_t sdrv_pvt_lld_set_ctrl_enable(paddr_t base, int32_t enable);
int32_t sdrv_pvt_lld_set_ctrl_sample_mode(paddr_t base, pvt_sensor_type_enum_t devType, pvt_sample_mode_enum_t mode);
int32_t sdrv_pvt_lld_set_ctrl_alarm_mode(paddr_t base, pvt_alarm_chan_emum_t chan, pvt_sensor_alarm_type_enum_t type, int32_t enable);
int32_t sdrv_pvt_lld_set_ctrl_trimo(paddr_t base, int32_t trimo);
int32_t sdrv_pvt_lld_set_ctrl_trimg(paddr_t base, int32_t trimg);

int32_t sdrv_pvt_lld_get_dout_data(paddr_t base, int32_t* dataOut);
int32_t sdrv_pvt_lld_get_dout_valid(paddr_t base, int32_t* valid);

int32_t sdrv_pvt_lld_set_hyst_high(paddr_t base, pvt_alarm_chan_emum_t chan, uint16_t high, uint16_t low);
int32_t sdrv_pvt_lld_set_hyst_low(paddr_t base, pvt_alarm_chan_emum_t chan, uint16_t high, uint16_t low);
int32_t sdrv_pvt_lld_set_hyst_raising(paddr_t base, pvt_alarm_chan_emum_t chan, uint16_t high, uint16_t low);
int32_t sdrv_pvt_lld_set_hyst_falling(paddr_t base, pvt_alarm_chan_emum_t chan, uint16_t high, uint16_t low);
int32_t sdrv_pvt_lld_get_hyst_high(paddr_t base, pvt_alarm_chan_emum_t chan, uint16_t* high, uint16_t* low);
int32_t sdrv_pvt_lld_get_hyst_low(paddr_t base, pvt_alarm_chan_emum_t chan, uint16_t* high, uint16_t* low);
int32_t sdrv_pvt_lld_get_hyst_raising(paddr_t base, pvt_alarm_chan_emum_t chan, uint16_t* high, uint16_t* low);
int32_t sdrv_pvt_lld_get_hyst_falling(paddr_t base, pvt_alarm_chan_emum_t chan, uint16_t* high, uint16_t* low);



int32_t sdrv_pvt_lld_get_hyst_timer_f(paddr_t base, pvt_alarm_chan_emum_t chan, uint32_t* time);
int32_t sdrv_pvt_lld_get_hyst_timer_r(paddr_t base, pvt_alarm_chan_emum_t chan, uint32_t* time);

int32_t sdrv_pvt_lld_set_int(paddr_t base, pvt_alarm_chan_emum_t chan, pvt_sensor_alarm_type_enum_t alarmType, int32_t enable);
int32_t sdrv_pvt_lld_get_int(paddr_t base, pvt_alarm_chan_emum_t chan, pvt_sensor_alarm_type_enum_t alarmType, int32_t* enableSta);
int32_t sdrv_pvt_lld_get_int_status(paddr_t base, pvt_alarm_chan_emum_t chan, pvt_sensor_alarm_type_enum_t alarmType, int32_t* status);
int32_t sdrv_pvt_lld_clr_int_sta(paddr_t base, pvt_alarm_chan_emum_t chan, pvt_sensor_alarm_type_enum_t alarmType);
#endif /* INCLUDE_SDRV_PVT_H */
/* end of header file sdrv_pvt.h */