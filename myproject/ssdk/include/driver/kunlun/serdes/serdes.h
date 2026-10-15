/**
 * @file serdes.h
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: serdes device driver
 *
 * Revision History:
 * -----------------
 */

#ifndef INCLUDE_SERDES_H
#define INCLUDE_SERDES_H

/* =========================== include file session ======================== */
#include <types.h>
#include <cmsis_os2.h>

#include <driver/i2c.h>
/**
* @defgroup serdes
* @brief  serdes common api header file
* @{
*/
/* =========================== marco define session ======================== */
#define SERDES_RET_SUCESS       0
#define SERDES_RET_FAIL         -1
#define SERDES_RET_I2C_RW_TMO   -2
#define SERDES_RET_DES_OFFLINE  -3

/* =========================== type define session ======================== */
typedef struct serdes_dev serdes_dev_t;
typedef void* sdrv_serdes_handle;
typedef struct serdes_config serdes_config_t;

/* =========================== enumeration define session ================== */
typedef enum {
    /* serializer control type */
    SER_CONTROL_ENABLE_I2C_PASSTROUGH,
    SER_CONTROL_PORT_ENABLE,
    SER_CONTROL_GPIO_CTRL,

    /* deserializer control type */
    DES_CONTROL_GPIO_CTRL,

    /* serdes port passthrough ctl */
    SERDES_PORT_PASSTHROUGH_CTL,

    /* get serdes data */
    SERDES_CONTROL_GET_I2C_ADDR,
}serdes_ctrl_type_enum_t;


/* =========================== structure define session ==================== */
/** serdes control get i2c addr structure */
typedef struct {
    uint16_t ser_addr;
    uint16_t des_addr;
}serdesCtl_i2cAddr_t;

/** serdes gpio control data structure */
typedef struct serdes_gpio_ctrl {
    uint8_t io_num;
    uint8_t dir;        /**< 1 input; 0 output */
    uint8_t val;        /**< set output val */
}serdes_gpio_ctrl_t;

/** serdes gpio passthrough ctl structure */
typedef struct serdes_port_passthrough {
    uint8_t dir;        /**< passthrough direction. 0: ser -> des; 1: des -> ser */
    uint8_t s_port;     /**< ser port number */
    uint8_t d_port;     /**< des port number */
}serdes_port_passthrough_ctl_t;

/** serdes device config */
struct serdes_config {
    char* name;
    int32_t ser_type;
    int32_t des_type;
    uint16_t ser_addr;
    uint16_t des_addr;
    i2c_adap_dev_t* i2c_dev;
    serdes_dev_t* (*serdes_probe)(sdrv_serdes_handle* sdrvHandle, const serdes_config_t* cfgPtr);
};

/** serdes dev option structure define */
typedef struct serdes_ops {
    int32_t (*init)(serdes_dev_t* dev);
    int32_t (*deinit)(serdes_dev_t* dev);
    int32_t (*check_serdes_link)(serdes_dev_t* dev);
    int32_t (*device_control)(serdes_dev_t* dev, int32_t ctlType, void* data, uint8_t* dataLen);
    int32_t (*ser_reg_read)(serdes_dev_t* dev, uint32_t reg, uint32_t* regVal);
    int32_t (*ser_reg_write)(serdes_dev_t* dev, uint32_t reg, uint32_t regVal);
    int32_t (*des_reg_read)(serdes_dev_t* dev, uint32_t reg, uint32_t* regVal);
    int32_t (*des_reg_write)(serdes_dev_t* dev, uint32_t reg, uint32_t regVal);
}serdes_ops_t;

/** serdes dev structure define */
struct serdes_dev {
    char* name;
    bool is_init;
    serdes_ops_t* ops;
    void* priv;
    const serdes_config_t* cfg_ptr;
};

/* =========================== function declare session ==================== */
/**
 * @brief serdes device init
 *
 * @param dev[in] serdes device
 * @return int32_t
 * @retval 0 sucess
 * @retval -1 failed
 */
int32_t serdes_init(serdes_dev_t* dev);

/**
 * @brief serdes device deinit
 *
 * @param dev[in] serdes device
 * @return int32_t
 * @retval 0 sucess
 * @retval -1 failed
 */
int32_t serdes_deinit(serdes_dev_t* dev);

/**
 * @brief serdes link status check
 *
 * @param dev[in] serdes device
 * @return int32_t
 * @retval 0 sucess
 * @retval -1 failed
 */
int32_t serdes_check_serdes_link(serdes_dev_t* dev);

/**
 * @brief serdes device control
 *
 * @param dev[in] device
 * @param ctlType[in] control type, reference for @serdes_ctrl_type_enum_t
 * @param data[in] control data, if not need, specify as null
 * @param dataLen[in] control data len
 * @return int32_t
 * @retval 0 sucess
 * @retval -1 failed
 */
int32_t serdes_control(serdes_dev_t* dev
                       , int32_t ctlType
                       , void* data
                       , uint8_t* dataLen);

/** api for debug */
int32_t serdes_ser_reg_read(serdes_dev_t* dev, uint32_t reg, uint32_t* regVal);
int32_t serdes_ser_reg_write(serdes_dev_t* dev, uint32_t reg, uint32_t regVal);
int32_t serdes_des_reg_read(serdes_dev_t* dev, uint32_t reg, uint32_t* regVal);
int32_t serdes_des_reg_write(serdes_dev_t* dev, uint32_t reg, uint32_t regVal);

/** @} */
#endif /* INCLUDE_SERDES_H */
/* end of header file serdes.h */