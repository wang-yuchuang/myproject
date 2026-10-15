/**
 * @file sdrv_pvt_drv.h
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:pvt driver header file
 *
 * Revision History:
 * -----------------
 */

#ifndef INCLUDE_SDRV_PVT_DRV_H
#define INCLUDE_SDRV_PVT_DRV_H

#include <types.h>
#include <spinlock.h>

#include <sdrv_pvt.h>
#include "pvt.h"


/** macro define section */
#define PVT_SENSOR_CONVERT_PARA_MAX     5
#define PVT_SENSOR_PN_TYPE_NUM          PVT_SENSOR_TYPE_NUM
#define PVT_SENSOR_PN_TYPE_LVT          PVT_SENSOR_TYPE_LVT
#define PVT_SENSOR_PN_TYPE_ULVT         PVT_SENSOR_TYPE_ULVT
#define PVT_SENSOR_PN_TYPE_SVT          PVT_SENSOR_TYPE_SVT

#define PVT_SENSOR_SAMPLE_MODE_TEMP     PVT_SENSOR_SAMPLE_MODE_TEMP
#define PVT_SENSOR_SAMPLE_MODE_VOLTAGE  PVT_SENSOR_SAMPLE_MODE_VOLTAGE
#define PVT_SENSOR_SAMPLE_MODE_PROCESS  PVT_SENSOR_SAMPLE_MODE_PROCESS
/** enumeration define section */
/**
 * @brief pvt alarm MODE
 *
 */
enum pvt_alarm_mode {
    PVT_ALARM_MODE_TEMP     = PVT_SENSOR_SAMPLE_MODE_TEMP,
    PVT_ALARM_MODE_VOLTAGE  = PVT_SENSOR_SAMPLE_MODE_VOLTAGE,

    PVT_ALARM_MODE_INVALID,
};

/**
 * @brief pvt alarm type
 * 0) hyst_high
 *   v/t^
 *      |
 *      |
 *      |                      alarm
 * high +-----------.....................
 *      |          .         .
 *      |         .         .
 *      |        .         .
 *      |       .         .
 * low  +--...............
 *      | noalarm
 *      |
 *      +-------------------------------------------->
 *                                                time
 *
 * 1) hyst_low
 *   v/t^
 *      |
 *      |
 *      |   noalarm
 * high +--................
 *      |     .            .
 *      |      .            .
 *      |       .            .
 *      |        .            .
 * low  +---------...................
 *      |                       alarm
 *      |
 *      +-------------------------------------------->
 *                                                  time
 * 2) hyst_raising
 * t2 moment will trigger alarm interrupt, and raising up
 * sustain time (t2 - t1) will be recoder; and it should
 * fall down to low again to clear alarm.
 *  v/t ^
 *      |
 *      |                        .
 *      |                       . t2 alarm
 * high +----------------------.-------------------
 *      |                     .
 *      |                    .
 *      |         ..        .
 *      |        .  .      . t1
 * low  +-------.----.----.-------------------------
 *      |      .      .  .
 *      |     .        ..
 *      +-------------------------------------------->
 *                                                  time
 *
 * 3) hyst_down
 * t2 moment will trigger alarm interrupt, and falling
 * down sustain time (t2 - t1) will be recoder; and it
 * should raise up to high again to clear alarm.
 *  v/t^
 *     |
 *     |        .       ..
 *     |         .     .  . t1
 *high +----------.---.----.----------------------
 *     |           . .      .
 *     |            .        .
 *     |                      .
 *     |                       .t2 alarm
 *low  +------------------------.------------------
 *     |                         ..
 *     |
 *     |
 *     +-------------------------------------------->time
 *
 */
enum pvt_alarm_type {
    PVT_ALARM_TYPE_HYST_HIGH,
    PVT_ALARM_TYPE_HYST_LOW,
    PVT_ALARM_TYPE_HYST_RAISING,
    PVT_ALARM_TYPE_HYST_FALLING,

    PVT_ALARM_TYPE_INVALID,
};

/**
 * @brief get integer data type, get some data need provide
 * the channel id whitch obtainging by @pvt_set_alarm return.
 *
 */
enum pvt_integer_data_get {
    PVT_GET_INT_PROCESS,
    PVT_GET_INT_SENSOR_SAMPLE_MODE,

    /* integer data below this line need appoint channel id */
    PVT_GET_INT_ALARM_SUSTAIN_TIME,     /**< falling down or raising up sustain time */
    PVT_GET_INT_ALARM_TYPE,             /**< hyst high or hyst low or hsyt raising or hyst falling */
    PVT_GET_INT_ALARM_STATUS,           /**< current channel is keeping alarm or not */
    PVT_GET_INT_ALARM_ENABLE_STA,       /**< current channel alarm is enable or not */

    PVT_GET_INT_INVALID,
};

enum pvt_integer_data_set {
    PVT_SET_INT_SENSOR_PNTYPE,          /**< set data, sensor pn type, only for debug */
    PVT_SET_INT_SAMPLE_MODE,

    PVT_SET_INT_INVALID,
};

/**
 * @brief get flaot data type, get some data need provide
 * the channel id whitch obtaining by @pvt_set_alarm return.
 *
 */
enum pvt_float_data_get {
    PVT_GET_FLOAT_CUR_TEMP,
    PVT_GET_FLOAT_CUR_VOLTAGE,

    /* float data below this line need appoint channel id */
    PVT_GET_FLOAT_ALARM_PHYVAL_HIGH,
    PVT_GET_FLOAT_ALARM_PHYVAL_LOW,

    PVT_GET_FLOAT_INVALID,
};

/** structure define section */
typedef void (*alarm_callback)(uint16_t chanId, void* usrData);
/**
 * @brief set alarm config struct
 *
 */
struct pvt_alarm_para {
    float high_val;
    float low_val;
    alarm_callback cb_fun;
    void* args;
};

/**
 * @brief pvt data structure
 *
 */
struct pvt_data {
    float temp;
    float voltage;
    uint16_t process;
};

typedef struct sdrv_sensor_process_cfg {
    uint16_t ss_process;
    uint16_t ff_process;
} sdrv_sensor_process_cfg_t;

typedef struct sdrv_sensor_conv_cfg {
    uint8_t par_num;
    float   para_arry[PVT_SENSOR_CONVERT_PARA_MAX];
} sdrv_sensor_conv_cfg_t;

/**
 * @brief sdrv pvt config
 *
 */
typedef struct sdrv_pvt_config {
    paddr_t base;
    uint32_t irq_num[PVT_SENSOR_ALARM_CHANNUM];
    int32_t sensor_type;
    int32_t sensor_mode;
    int32_t sensor_clk;
    int32_t conversion_time;    /**< us, if configed as 0, check data valid */
    sdrv_sensor_process_cfg_t process_cfg[PVT_SENSOR_PN_TYPE_NUM];
    sdrv_sensor_conv_cfg_t conv_temp2code;
    sdrv_sensor_conv_cfg_t conv_code2temp;
    sdrv_sensor_conv_cfg_t conv_vol2code;
    sdrv_sensor_conv_cfg_t conv_code2vol;
}sdrv_pvt_config_t;

/**
 * @brief sdrv pvt channel
 *
 */
typedef struct sdrv_pvt_chan {
    int32_t chan;
    int32_t chanId;
    uint32_t irq;
    uint32_t sustain_rtime;
    uint32_t sustain_ftime;
    pvt_dev_t* dev_ptr;
    pvt_alarm_mode_enum_t alarm_mode;   /**< temp or voltage */
    pvt_alarm_type_enum_t alarm_type;
    uint16_t high_code;
    uint16_t low_code;
    alarm_callback alarm_cb;
    void* usrData;
}sdrv_pvt_chan_t;

/**
 * @brief sdrv pvt device
 *
 */
typedef struct sdrv_pvt_dev {
    pvt_dev_t dev;
    spin_lock_t lock;
    uint8_t sensor_mode;
    uint8_t sensor_type;
    sdrv_pvt_chan_t chan[PVT_SENSOR_ALARM_CHANNUM];
    uint8_t chan_bit;
    uint16_t gen_id;
    const sdrv_pvt_config_t* cfg_ptr;
} sdrv_pvt_dev_t;


/**
 * @brief pvt device probe
 *
 * @param sdrv_pvt_ptr[in] sdrv pvt instace pointer
 * @param cfg_ptr[in] pvt config pointer
 * @return pvt_dev_t* pvt device pointer
 */
pvt_dev_t* sdrv_pvt_probe(sdrv_pvt_dev_t* sdrv_pvt_ptr
                            , const sdrv_pvt_config_t* cfg_ptr);
#endif /* INCLUDE_SDRV_PVT_DRV_H */
/* end of header file sdrv_pvt_drv.h */