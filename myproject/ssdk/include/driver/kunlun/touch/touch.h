/**
 * @file touch.h
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:touch implement file
 *
 * Revision History:
 * -----------------
 */

#ifndef INCLUDE_TOUCH_H
#define INCLUDE_TOUCH_H

/* =========================== include file session ======================== */
#include <types.h>
#include <driver/pinctrl.h>
#include <driver/gpio.h>
#include <driver/i2c.h>
#include <driver/kunlun/serdes/serdes.h>

/**
* @defgroup touch
* @brief  touch driver common api
* @{
*/
/* =========================== marco define session ======================== */
#define TOUCH_POINT_MAX_NUM     10
#define TOUCH_NOTICER_MAX_NUM   6
#define TS_ENABLE true
#define TS_DISABLE false
/* USE_INT_ROUTE:serdes use int not gpio for interrupt to soc */
#define USE_INT_ROUTE (0xff)

/* =========================== enumeration define session ================== */
/** screen instance enumeration */
enum DISPLAY_SCREEN {
    NOT_FOUND = -1,
    CLUSTER,
    INFOTAINMENT,
    ENTERTAINMENT,
    CONTROLPANEL,
    HUD,
    SCREEN_MAX
};

/** touch driver notice sensor data type */
typedef enum {
    TS_NOTICE_EVT_COORD,
    TS_NOTICE_EVT_GESTURE,

    TS_NOTICE_EVT_INVALID,
}touch_notice_type_enum_t;


typedef enum {
    TS_CONTROL_RESET_DEV,
    TS_CONTROL_UPDATE_FIRMWARE,
    TS_CONTROL_CHANGE_MODE,

    /** get data */
    TS_CONTROL_GET_COORD_CFG,   /**< data structure ref@tsCtrl_configInfo_t  */
    TS_CONTROL_GET_SCREEN_ID,
    TS_CONTROL_GET_VENDOR_INFO, /**< data structure ref@tsCtrl_vendorInfo_t  */

}touch_ctrl_type_enum_t;

/* =========================== typedef define session ====================== */
/** touch driver handle typedef */
typedef void* sdrv_touch_handle;

/** touch device typedef */
typedef struct touch_device touch_dev_t;

/** touch sensor notice data typedef */
typedef struct touch_notice touch_notice_t;

/** touch sensor data notice callback function type declaration */
typedef int32_t (*touch_event_notice)(int32_t scId
                                      , touch_notice_t* notice
                                      , void* usrCtx);

/* =========================== structure define session ==================== */
/** touch control vendor info structure */
typedef struct {
    const char* name;
    uint16_t version;
    uint16_t id;
    uint16_t vendor;
    uint16_t ser_addr;
    uint16_t des_addr;
    uint16_t ts_addr;
}tsCtrl_vendorInfo_t;

/** touch control config info structure */
typedef struct {
    uint8_t swapped_x_y;
    uint8_t inverted_x;
    uint8_t inverted_y;
    uint8_t max_touch_num;
}tsCtrl_configInfo_t;

/** describe coordinate point structure */
typedef struct ts_coord_point {
    uint8_t id;
    uint16_t x;
    uint16_t y;
    uint16_t w;
}ts_coord_point_t;

/** touch coordinate point notice information structure
 * notice service with @TS_NOTICE_EVT_COORD enum
 */
typedef struct ts_coord_info {
    uint8_t touch_num;
    uint8_t have_key;
    uint16_t key_value;
    ts_coord_point_t point[TOUCH_POINT_MAX_NUM];
} ts_coord_info_t;

/** touch sensor data notice structure, evt_type reference for
 * @touch_notice_type_enum_t enumeration
 */
struct touch_notice {
    int32_t evt_type;
    union {
        ts_coord_info_t coord;
    };
};

/** touch callback structure */
typedef struct touch_cb_chan {
    bool is_valid;
    touch_event_notice cb;
    void* usrCtx;
}touch_cb_chan_t;

/** coordinate config */
typedef struct ts_coord_config {
    uint16_t x_max;
    uint16_t y_max;
    uint8_t max_touch_num;
    uint8_t swapped_x_y;
    uint8_t inverted_x;
    uint8_t inverted_y;
}ts_coord_cfg_t;

/** serdes config */
typedef struct ts_serdes_config {
    bool enable;
    serdes_dev_t** serdes;
    uint8_t reset_pin;
    uint8_t irq_pin;
    uint8_t sirq_pin;
    uint8_t srst_pin;
}ts_serdes_cfg_t;

/** pin config */
typedef struct ts_pin_config {
    int32_t pin_num;
    gpio_dev_t* pin_dev;
}ts_pin_cfg_t;

// /**
//  * @brief prot expand config
//  *
//  */
// typedef struct port_expand_config {
//     bool enable;
//     uint32_t port_i2c_bus;
//     uint32_t port_addr;
//     uint32_t port_id;
// };

/** touch config structure */
typedef struct touch_config {
    enum DISPLAY_SCREEN screen_id;
    bool                enable;
    const char*         name;
    uint8_t             i2c_addr;
    i2c_adap_dev_t*     i2c_dev;
    ts_coord_cfg_t      coord_config;
    ts_serdes_cfg_t     serdes_config;
    ts_pin_cfg_t        reset_pin;
    ts_pin_cfg_t        irq_pin;

    touch_dev_t* (*probe)(sdrv_touch_handle* touchDrv
                          , const struct touch_config* cfgPtr);
}touch_config_t;

/** touch operation function structure */
typedef struct touch_ops {
    int32_t (*init)(touch_dev_t* dev);
    int32_t (*deinit)(touch_dev_t* dev);
    int32_t (*online_check)(touch_dev_t* dev);
    int32_t (*register_callback)(touch_dev_t* dev
                                , touch_event_notice cb
                                , void* usrCtx);
    int32_t (*device_control)(touch_dev_t* dev
                              , touch_ctrl_type_enum_t type
                              , void* data
                              , int32_t* len);
    int32_t (*reg_read)(touch_dev_t* dev, uint32_t reg, uint32_t* regVal);
    int32_t (*reg_write)(touch_dev_t* dev, uint32_t reg, uint32_t regVal);
}touch_ops_t;

/** touch device structure define */
struct touch_device {
    char*           name;
    bool            is_init;
    touch_ops_t*    ops;
    touch_config_t* cfg_ptr;
    void*           priv;
};

/* =========================== function declare session ==================== */
/**
 * @brief touch device init
 *
 * @param dev[in] touch device pointer
 * @return int32_t
 * @retval 0 sucess
 * @retval -1 failed
 */
int32_t touch_init(touch_dev_t* dev);

/**
 * @brief touch device deinit
 *
 * @param dev[in] touch device deinit
 * @return int32_t
 * @retval 0 sucess
 * @retval -1 failed
 */
int32_t touch_deinit(touch_dev_t* dev);

/**
 * @brief touch online check
 *
 * @param dev
 * @return int32_t
 */
int32_t touch_online_check(touch_dev_t* dev);

/**
 * @brief register callback, touch sensor data will be notified to the service
 * through the callback.
 *
 * @param dev[in] touch device
 * @param cb[in] callback function pointer
 * @param usrCtx[in] user set callback parameters
 * @return int32_t
 * @retval 0 sucess
 * @retval -1 failed
 */
int32_t touch_register_callback(touch_dev_t* dev, touch_event_notice cb, void* usrCtx);

/**
 * @brief touch device control
 *
 * @param dev[in] touch device
 * @param type[in] control type, reference for @touch_ctrl_type_enum_t enumeration
 * @param data[in/out] control data pointer, if donot need, specify as NULL
 * @param len[in/out] control data len pointer, if donot need, specify as NULL
 * @return int32_t
 * @retval 0 sucess
 * @retval -1 failed
 */
int32_t touch_device_control(touch_dev_t* dev
                             , touch_ctrl_type_enum_t type
                             , void* data
                             , int32_t* len);

/**
 * @brief read touch register value, for debug
 *
 * @param dev[in] touch device
 * @param reg[in] touch register address
 * @param regVal[out] touch register read value
 * @return int32_t
 * @retval 0 sucess
 * @retval -1 failed
 */
int32_t touch_reg_read(touch_dev_t* dev, uint32_t reg, uint32_t* regVal);

/**
 * @brief write touch register value, for debug
 *
 * @param dev[in] touch device
 * @param reg[in] touch register address
 * @param regVal[in] touch register write value
 * @return int32_t
 * @retval 0 sucess
 * @retval -1 failed
 */
int32_t touch_reg_write(touch_dev_t* dev, uint32_t reg, uint32_t regVal);

void touch_test_send_flg(touch_dev_t* dev);
/** @} */
#endif /* INCLUDE_TOUCH_H */
/* end of header file touch.h */