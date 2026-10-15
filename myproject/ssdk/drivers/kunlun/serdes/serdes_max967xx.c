/**
 * @file serdes_max967xx.c
 *
 * Copyright (c) 2023 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: serdes driver for max96789/max96752
 *
 * Revision History:
 * -----------------
 */
/* =========================== include file session ======================== */
#include <types.h>
#include <debug.h>
#include <string.h>
#include <stdlib.h>

#include <driver/kunlun/serdes/serdes.h>
#include <driver/kunlun/serdes/serdes_vendor.h>
/* =========================== marco define session ======================== */
#define STRUCT_MEMBER_OFFSET(x, y)         (size_t)(&((x *)(0))->y)

#define SDRV_MAX967XX_DEBUG_LVL      SSDK_DEBUG
#define EXTRA_FMT   "sdrv_max967xx: "
#if SDRV_MAX967XX_DEBUG_LVL >= SSDK_ERR
#define SDRV_MAX967XX_ERR(format, ...) \
    ssdk_printf(SSDK_EMERG, EXTRA_FMT format, ##__VA_ARGS__)
#else
#define SDRV_MAX967XX_ERR(format, ...)
#endif

#if SDRV_MAX967XX_DEBUG_LVL >= SSDK_WARNING
#define SDRV_MAX967XX_WARN(format, ...) \
    ssdk_printf(SSDK_WARNING, EXTRA_FMT format, ##__VA_ARGS__)
#else
#define SDRV_MAX967XX_WARN(format, ...)
#endif

#if SDRV_MAX967XX_DEBUG_LVL >= SSDK_INFO
#define SDRV_MAX967XX_INFO(format, ...) \
    ssdk_printf(SSDK_INFO, EXTRA_FMT format, ##__VA_ARGS__)
#else
#define SDRV_MAX967XX_INFO(format, ...)
#endif

#define TRUNC_HIGH_BYTE(x)      (x >> 8)
#define TRUNC_LOW_BYTE(x)       (x & 0xff)
/* =========================== enumeration define session ================== */
/* =========================== structure define session ==================== */
/** i2c trans data structure */
typedef struct i2c_transfer {
    union {
        struct {
            uint8_t reg_h;
            uint8_t reg_l;
        };
        uint16_t reg;
    };
    uint8_t val[10];
} __PACKED i2c_transfer_t;

/** sdrv serdes driver structure */
typedef struct sdrv_serdes_drv {
    serdes_dev_t dev;
}sdrv_serdes_drv_t;
/* =========================== function declare session ==================== */
static int32_t max967xx_init(serdes_dev_t* dev);
static int32_t max967xx_deinit(serdes_dev_t* dev);
static int32_t max967xx_check_link(serdes_dev_t* dev);
static int32_t max967xx_device_control(serdes_dev_t* dev, int32_t ctlType, void* data, uint8_t* dataLen);
static int32_t max967xx_ser_reg_read(serdes_dev_t* dev, uint32_t reg, uint32_t* regVal);
static int32_t max967xx_ser_reg_write(serdes_dev_t* dev, uint32_t reg, uint32_t regVal);
static int32_t max967xx_des_reg_read(serdes_dev_t* dev, uint32_t reg, uint32_t* regVal);
static int32_t max967xx_des_reg_write(serdes_dev_t* dev, uint32_t reg, uint32_t regVal);

/* =========================== global value define session ================= */
/* =========================== static value define session ================= */
static serdes_ops_t s_serdes_ops = {
    .init = max967xx_init,
    .deinit = max967xx_deinit,
    .check_serdes_link = max967xx_check_link,
    .device_control = max967xx_device_control,
    .ser_reg_read = max967xx_ser_reg_read,
    .ser_reg_write = max967xx_ser_reg_write,
    .des_reg_read = max967xx_des_reg_read,
    .des_reg_write = max967xx_des_reg_write,
};
/* =========================== function implementation session ============= */
#define SERDES_I2C_POLLING
#ifndef SERDES_I2C_POLLING
static int serdes_i2c_read(i2c_adap_dev_t *adap, uint8_t addr, i2c_transfer_t* buf, int len)
{
    int ret = 0;
    ret = i2c_write_read(adap, addr
                         , (uint8_t *)buf, STRUCT_MEMBER_OFFSET(i2c_transfer_t, val)
                         , buf->val, len);
    if (ret < 0) {
        ssdk_printf(SSDK_ERR, "%s: read error, ret=%d\n", __func__, ret);
        return ret;
    }

    return 0;
}

static int serdes_i2c_write(i2c_adap_dev_t *adap, uint8_t addr, i2c_transfer_t* buf, int len)
{
    int ret = 0;
    ret = i2c_write(adap, addr, (uint8_t *)buf, len + STRUCT_MEMBER_OFFSET(i2c_transfer_t, val));
    if (ret < 0) {
        ssdk_printf(SSDK_ERR, "%s: write error, ret=%d.\n", __func__, ret);
        return ret;
    }

    return ret;
}
#else
static int serdes_i2c_read(i2c_adap_dev_t* adap, uint8_t addr, i2c_transfer_t* buf, int len)
{
    int ret;
    struct i2c_msg msgs[2] = {
        {
            .addr = addr,
            .buf = (uint8_t *)buf,
            .len = STRUCT_MEMBER_OFFSET(i2c_transfer_t, val),
            .addr_flag = I2C_ADDR7B,
            .rw_flag = I2C_M_W,
            .polling = 1,
        },
        {
            .addr = addr,
            .buf = buf->val,
            .len = len,
            .addr_flag = I2C_ADDR7B,
            .rw_flag = I2C_M_R,
            .polling = 1,
        },
    };
    ret = i2c_transfer(adap, msgs, 2);
    return ret;
}

static int serdes_i2c_write(i2c_adap_dev_t *adap, uint8_t addr, i2c_transfer_t* buf, int len)
{
    int ret;
    struct i2c_msg msgs[1] = {
        {
            .addr = addr,
            .buf = (uint8_t *)buf,
            .len = len + STRUCT_MEMBER_OFFSET(i2c_transfer_t, val),
            .addr_flag = I2C_ADDR7B,
            .rw_flag = I2C_M_W,
            .polling = 1,
        },
    };
    ret = i2c_transfer(adap, msgs, 1);
    return ret;
}
#endif

static int32_t max967xx_init(serdes_dev_t* dev)
{
    return 0;
}

static int32_t max967xx_deinit(serdes_dev_t* dev)
{
    return 0;
}

static int32_t max967xx_check_link(serdes_dev_t* dev)
{
    int32_t ret = -1;

    if (dev == NULL || dev->priv == NULL) {
        SDRV_MAX967XX_ERR("invalid parameter for %s \n", __func__);
        return ret;
    }

    i2c_adap_dev_t* i2c_dev = dev->cfg_ptr->i2c_dev;
    uint16_t des_addr = dev->cfg_ptr->des_addr;

    i2c_transfer_t trans_buf;
    memset(&trans_buf, 0x00, sizeof(trans_buf));

    // read remote des device i2c address
    trans_buf.reg_h = TRUNC_HIGH_BYTE(0x00);
    trans_buf.reg_l = TRUNC_LOW_BYTE(0x00);

    ret = serdes_i2c_read(i2c_dev, des_addr, &trans_buf, 1);
    if (ret) {
        // read target timeout
        SDRV_MAX967XX_ERR("%s serdes check link status timeout\n", __func__);
        return SERDES_RET_I2C_RW_TMO;
    }
    else if (trans_buf.val[0] >> 1 != des_addr) {
        SDRV_MAX967XX_ERR("%s serdes check link read invalid address: cfg(0x%x) read(0x%x)\n", __func__, des_addr, trans_buf.val[0] >> 1);
        return SERDES_RET_DES_OFFLINE;
    }
    else {
        SDRV_MAX967XX_INFO("%s serdes check link status ok\n", __func__);
        ret = 0;
    }

    return ret;
}

static int32_t enable_12c_passthrough(serdes_dev_t* dev)
{
    i2c_transfer_t write_data = {
        .reg_h = TRUNC_HIGH_BYTE(0x17),
        .reg_l = TRUNC_LOW_BYTE(0x17),
    };

    i2c_transfer_t read_data = {
        .reg_h = TRUNC_HIGH_BYTE(0x17),
        .reg_l = TRUNC_LOW_BYTE(0x17),
    };

    int32_t ret = 0;

    i2c_adap_dev_t* i2c_dev = dev->cfg_ptr->i2c_dev;
    uint16_t ser_addr = dev->cfg_ptr->ser_addr;

    ret = serdes_i2c_read(i2c_dev, ser_addr, &read_data, 1);
    if (ret !=0) {
        SDRV_MAX967XX_ERR("%s failed, read register 0x%x%x error\n", __func__, read_data.reg_h, read_data.reg_l);
        return -1;
    }

    if ((read_data.val[0] & 0xC0) == 0xC0) {
        SDRV_MAX967XX_INFO("%s do nothing, i2c passtrough already enable\n", __func__);
        return 0;
    }

    return 0;
    write_data.val[0] = read_data.val[0] | 0xC0;
    ret = serdes_i2c_write(i2c_dev, ser_addr, &write_data, 1);
    if (ret) {
        SDRV_MAX967XX_ERR("%s failed, write register 0x%x%x error\n", __func__, write_data.reg_h, write_data.reg_l);
        return -1;
    }

    return 0;
}

static int32_t enable_port(serdes_dev_t* dev)
{
    //@TODO do nothing
    return 0;
}

// // input mode, only transmit io status to ser
// static int32_t max96752_gpio_input(serdes_dev_t* dev, int32_t gpio)
// {
//     int32_t ret = 0;
//     uint16_t dreg_a = 0;

//     // confirm gpio control register address
//     dreg_a = 0x200 + gpio * 3;

//     i2c_transfer_t gpio_data = {
//         .reg_h = TRUNC_HIGH_BYTE(dreg_a),
//         .reg_l = TRUNC_LOW_BYTE(dreg_a),
//         .val = {0},
//     };

//     ret = serdes_i2c_read(dev->cfg_ptr->i2c_dev
//                           , dev->cfg_ptr->des_addr
//                           , &gpio_data
//                           , 1);
//     if (ret) {
//         SDRV_MAX967XX_ERR("%s failed for gpio %d read, slave addr: 0x%x, register addr is: 0x%x\n"
//                          , __func__
//                          , gpio
//                          , dev->cfg_ptr->des_addr
//                          , dreg);
//         return -1;
//     }

//     gpio_data.val[0] = 0x83;
//     ret = serdes_i2c_write(dev->cfg_ptr->i2c_dev
//                           , dev->cfg_ptr->des_addr
//                           , &gpio_data
//                           , 1);
//     if (ret) {
//         SDRV_MAX967XX_ERR("%s failed for gpio %d write, slave addr: 0x%x, register addr is: 0x%x\n"
//                          , __func__
//                          , gpio
//                          , dev->cfg_ptr->des_addr
//                          , dreg);
//         return -1;
//     }

//     return 0;
// }

static int32_t max967xx_gpio_output(serdes_dev_t* dev, int32_t gpio, int32_t val)
{
    int32_t ret = 0;
    uint16_t dreg_a = 0;
    if (dev->cfg_ptr->des_type == MAX96752) {
        dreg_a = 0x200 + gpio * 3;
    }
    else if (dev->cfg_ptr->des_type == MAX96772_74) {
        dreg_a = 0x2b0 + gpio * 3;
    }
    else {
        SDRV_MAX967XX_ERR("invalid des type %d\n", dev->cfg_ptr->des_type);
        return -1;
    }

    i2c_transfer_t gpio_data = {
        .reg_h = TRUNC_HIGH_BYTE(dreg_a),
        .reg_l = TRUNC_LOW_BYTE(dreg_a),
    };
    val == 1 ? (gpio_data.val[0] = 0x90) : (gpio_data.val[0] = 0x80);

    ret = serdes_i2c_write(dev->cfg_ptr->i2c_dev
                           , dev->cfg_ptr->des_addr
                           , &gpio_data
                           , 1);
    if (ret) {
        SDRV_MAX967XX_ERR("%s failed for gpio %d write, slave addr: 0x%x, register addr is: 0x%x\n"
                         , __func__
                         , gpio
                         , dev->cfg_ptr->des_addr
                         , dreg_a);
        return -1;
    }

    return 0;
}

static int32_t des_gpio_ctl(serdes_dev_t* dev, serdes_gpio_ctrl_t* ctrl)
{
    switch (ctrl->dir) {
        case 0:
            max967xx_gpio_output(dev, ctrl->io_num, ctrl->val);
            break;
        case 1:
            // max96752_gpio_input(dev, ctrl->io_num);
            break;
        default:
            break;
    }
    return 0;
}

// // output mode, only receive io status from des
// static int32_t max96789_gpio_output(serdes_dev_t* dev, int32_t gpio)
// {
//     int32_t ret = 0;
//     uint16_t sreg_a = 0;

//     sreg_a = 0x2be + 3 * gpio;

//     i2c_transfer_t gpio_data = {
//         .reg_h = TRUNC_HIGH_BYTE(sreg_a),
//         .reg_l = TRUNC_LOW_BYTE(sreg_a),
//     };

//     gpio_data.val[0] = 0x84;
//     ret = serdes_i2c_write(dev->cfg_ptr->i2c_dev
//                             , dev->cfg_ptr->ser_addr
//                             , &gpio_data
//                             , 1);
//     if (ret) {
//         SDRV_MAX967XX_ERR("%s failed for gpio %d write, slave addr: 0x%x, register addr is: 0x%x\n"
//                          , __func__
//                          , gpio
//                          , dev->cfg_ptr->ser_addr
//                          , sreg);
//         return -1;
//     }

//     return 0;
// }

// static int32_t max96735_gpio_output(serdes_dev_t* dev, int32_t gpio)
// {
//     int32_t ret = 0;
//     uint16_t sreg_a = 0;

//     sreg_a = 0x2be + 3 * gpio;

//     i2c_transfer_t gpio_data = {
//         .reg_h = TRUNC_HIGH_BYTE(sreg_a),
//         .reg_l = TRUNC_LOW_BYTE(sreg_a),
//     };

//     gpio_data.val[0] = 0x04;
//     ret = serdes_i2c_write(dev->cfg_ptr->i2c_dev
//                             , dev->cfg_ptr->ser_addr
//                             , &gpio_data
//                             , 1);
//     if (ret) {
//         SDRV_MAX967XX_ERR("%s failed for gpio %d write, slave addr: 0x%x, register addr is: 0x%x\n"
//                          , __func__
//                          , gpio
//                          , dev->cfg_ptr->ser_addr
//                          , sreg);
//         return -1;
//     }

//     return 0;
// }

typedef struct {
    int32_t base_addr;
    char reg_type;
    uint8_t reg_val;
}gpio_grp_t;

/** 96735 port config */
static gpio_grp_t s_96735_d2s[] = {
    {0x2be, 'A', 0x80},
    {0x2bf, 'B', 0x20},     /**< reg val | sport */
    {0x2c0, 'C', 0x60},     /**< reg val | sport */
    {0x2be, 'A', 0x84},
    {-1, 0, 0},     /**< reg val | dport */
};

/** 96789 port config */
static gpio_grp_t s_96789_d2s[] = {
    {0x2be, 'A', 0x80},
    {0x2bf, 'B', 0xa0},     /**< reg val | sport */
    {0x2c0, 'C', 0x20},     /**< reg val | sport */
    {0x2be, 'A', 0x84},
    {-1, 0, 0},     /**< reg val | dport */
};

/** 96752 port config */
static gpio_grp_t s_96752_d2s[] = {
    {0x200, 'A', 0x83},
    {0x201, 'B', 0x20},     /**< reg val | sport */
    {-1, 0, 0},     /**< reg val | dport */
};

/** 96772 port config */
static gpio_grp_t s_96772_d2s[] = {
    {0x2b0, 'A', 0x83},
    {0x2b1, 'B', 0x20},     /**< reg val | sport */
    {-1, 0, 0},     /**< reg val | dport */
};

static int32_t serdes_port_passthrough(serdes_dev_t* dev, serdes_port_passthrough_ctl_t* ctlData)
{
    uint16_t dreg = 0, sreg = 0;
    i2c_transfer_t gpio_data;
    int32_t ret = 0;

    gpio_grp_t* sgpio_grp_ptr = NULL;
    gpio_grp_t* dgpio_grp_ptr = NULL;

    int32_t dir = ctlData->dir;
    uint8_t sPort = ctlData->s_port;
    uint8_t dPort = ctlData->d_port;

    if (dir == 1) {
        if (dev->cfg_ptr->ser_type == MAX96789) {
            sgpio_grp_ptr = s_96789_d2s;
        }
        else if (dev->cfg_ptr->ser_type == MAX96735) {
            sgpio_grp_ptr = s_96735_d2s;
        }
        else {
            SDRV_MAX967XX_ERR("invalid ser type: %d\n", dev->cfg_ptr->ser_type);
            return -1;
        }

        if (dev->cfg_ptr->des_type == MAX96752) {
            dgpio_grp_ptr = s_96752_d2s;
        }
        else if (dev->cfg_ptr->des_type == MAX96772_74) {
            dgpio_grp_ptr = s_96772_d2s;
        }
        else {
            SDRV_MAX967XX_ERR("invalid ser type: %d\n", dev->cfg_ptr->ser_type);
            return -1;
        }

        int32_t i = 0;
        // setup ser register
        while (sgpio_grp_ptr[i].base_addr != -1) {
            sreg = sgpio_grp_ptr[i].base_addr + sPort * 3;

            if (sgpio_grp_ptr[i].reg_type == 'A') {
                gpio_data.val[0] = sgpio_grp_ptr[i].reg_val;
            }
            else {
                gpio_data.val[0] = sgpio_grp_ptr[i].reg_val | sPort;
            }

            gpio_data.reg_h = TRUNC_HIGH_BYTE(sreg),
            gpio_data.reg_l = TRUNC_LOW_BYTE(sreg),
            ret = serdes_i2c_write(dev->cfg_ptr->i2c_dev
                                , dev->cfg_ptr->ser_addr
                                , &gpio_data
                                , 1);
            if (ret) {
                SDRV_MAX967XX_ERR("%s failed for sPort %d write, slave addr: 0x%x, register addr is: 0x%x\n"
                                , __func__
                                , sPort
                                , dev->cfg_ptr->ser_addr
                                , sreg);
                return -1;
            }
            i++;
        }

        // setup des register
        i = 0;
        while (dgpio_grp_ptr[i].base_addr != -1) {
            dreg = dgpio_grp_ptr[i].base_addr + dPort * 3;

            if (dgpio_grp_ptr[i].reg_type == 'A') {
                gpio_data.val[0] = dgpio_grp_ptr[i].reg_val;
            }
            else {
                gpio_data.val[0] = dgpio_grp_ptr[i].reg_val | sPort;
            }

            gpio_data.reg_h = TRUNC_HIGH_BYTE(dreg),
            gpio_data.reg_l = TRUNC_LOW_BYTE(dreg),
            ret = serdes_i2c_write(dev->cfg_ptr->i2c_dev
                                , dev->cfg_ptr->des_addr
                                , &gpio_data
                                , 1);
            if (ret) {
                SDRV_MAX967XX_ERR("%s failed for dPort %d write, slave addr: 0x%x, register addr is: 0x%x\n"
                                , __func__
                                , dPort
                                , dev->cfg_ptr->des_addr
                                , dreg);
                return -1;
            }
            i++;
        }
    }
    else {
        //@TODO donot support
        return -1;
    }

    return 0;
}


static int32_t max967xx_device_control(serdes_dev_t* dev, int32_t ctlType, void* data, uint8_t* dataLen)
{
    if (dev == NULL && data == NULL && dataLen == NULL) {
        SDRV_MAX967XX_ERR("invlaid parameter for %s.\n", __func__);
        return -1;
    }

    switch (ctlType) {
        case SER_CONTROL_ENABLE_I2C_PASSTROUGH:
            enable_12c_passthrough(dev);
            break;
        case SER_CONTROL_GPIO_CTRL:
            // do not support
            break;
        case SER_CONTROL_PORT_ENABLE:
            // do not support
            break;
        case DES_CONTROL_GPIO_CTRL:
            des_gpio_ctl(dev, (serdes_gpio_ctrl_t *)data);
            break;
        case SERDES_CONTROL_GET_I2C_ADDR:
            if (data != NULL) {
                ((serdesCtl_i2cAddr_t *)(data))->ser_addr = dev->cfg_ptr->ser_addr;
                ((serdesCtl_i2cAddr_t *)(data))->des_addr = dev->cfg_ptr->des_addr;

                if (dataLen != NULL)
                    *dataLen = sizeof(serdesCtl_i2cAddr_t);
            }
            else {
                return -1;
            }
            break;
        case SERDES_PORT_PASSTHROUGH_CTL:
            serdes_port_passthrough(dev, (serdes_port_passthrough_ctl_t *)data);
            break;
        default:
            break;
    }

    return 0;
}

int32_t max967xx_ser_reg_read(serdes_dev_t* dev, uint32_t reg, uint32_t* regVal)
{
    if (dev == NULL || regVal == NULL) {
        SDRV_MAX967XX_ERR("invalid parameter for %s. \n", __func__);
        return -1;
    }

    i2c_transfer_t read_data = {
        .reg_h = TRUNC_HIGH_BYTE(reg),
        .reg_l = TRUNC_LOW_BYTE(reg),
        .val = {0}
    };

    serdes_i2c_read(dev->cfg_ptr->i2c_dev
                    , dev->cfg_ptr->ser_addr
                    , &read_data, 1);
    *regVal = read_data.val[0];

    // SDRV_TI9XX_INFO("[debug]test read ser register: 0x%x and val: 0x%x\n", reg, *regVal);
    return 0;
}

int32_t max967xx_ser_reg_write(serdes_dev_t* dev, uint32_t reg, uint32_t regVal)
{
    if (dev == NULL) {
        SDRV_MAX967XX_ERR("invalid parameter for %s. \n", __func__);
        return -1;
    }

    i2c_transfer_t write_data = {
        .reg_h = TRUNC_HIGH_BYTE(reg),
        .reg_l = TRUNC_LOW_BYTE(reg),
        .val = {regVal}
    };

    serdes_i2c_write(dev->cfg_ptr->i2c_dev
                    , dev->cfg_ptr->ser_addr
                    , &write_data, 1);

    return 0;
}

int32_t max967xx_des_reg_read(serdes_dev_t* dev, uint32_t reg, uint32_t* regVal)
{
    if (dev == NULL || regVal == NULL) {
        SDRV_MAX967XX_ERR("invalid parameter for %s. \n", __func__);
        return -1;
    }

    i2c_transfer_t read_data = {
        .reg_h = TRUNC_HIGH_BYTE(reg),
        .reg_l = TRUNC_LOW_BYTE(reg),
        .val = {0}
    };

    serdes_i2c_read(dev->cfg_ptr->i2c_dev
                    , dev->cfg_ptr->des_addr
                    , &read_data, 1);
    *regVal = read_data.val[0];

    return 0;
}

int32_t max967xx_des_reg_write(serdes_dev_t* dev, uint32_t reg, uint32_t regVal)
{
    if (dev == NULL) {
        SDRV_MAX967XX_ERR("invalid parameter for %s. \n", __func__);
        return -1;
    }

    i2c_transfer_t write_data = {
        .reg_h = TRUNC_HIGH_BYTE(reg),
        .reg_l = TRUNC_LOW_BYTE(reg),
        .val = {regVal}
    };

    serdes_i2c_write(dev->cfg_ptr->i2c_dev
                    , dev->cfg_ptr->des_addr
                    , &write_data, 1);

    return 0;
}

serdes_dev_t* max967xx_serdes_probe(sdrv_serdes_handle* serdesSdrvHandle, const serdes_config_t* cfgPtr)
{
    serdes_dev_t* dev = NULL;
    if (cfgPtr == NULL) {
        SDRV_MAX967XX_ERR("invalid parameter for %s\n", __func__);
        return dev;
    }

    if (*serdesSdrvHandle != NULL) {
        SDRV_MAX967XX_INFO("already probed serdes device for %s.\n", cfgPtr->name);
        return &((sdrv_serdes_drv_t *)(*serdesSdrvHandle))->dev;
    }

    *serdesSdrvHandle = malloc(sizeof(sdrv_serdes_drv_t));
    if (*serdesSdrvHandle == NULL) {
        SDRV_MAX967XX_ERR("malloc memory for %s device error \n", cfgPtr->name);
        return NULL;
    }

    memset(*serdesSdrvHandle, 0x00, sizeof(sdrv_serdes_drv_t));

    sdrv_serdes_drv_t* serdes_drv = (sdrv_serdes_drv_t *)(*serdesSdrvHandle);
    serdes_drv->dev.cfg_ptr = cfgPtr;
    serdes_drv->dev.name = cfgPtr->name;
    serdes_drv->dev.priv = serdes_drv;

    int32_t ret = max967xx_check_link(&serdes_drv->dev);
    if (ret != SERDES_RET_SUCESS &&
        ret != SERDES_RET_DES_OFFLINE) {
        SDRV_MAX967XX_WARN("serdes connect status is disconnect \n");
        free(*serdesSdrvHandle);
        *serdesSdrvHandle = NULL;
        return dev;
    }

    dev = &serdes_drv->dev;
    dev->ops = &s_serdes_ops;
    serdes_init(dev);

    return dev;
}
/* end of file serdes_max967xx.c */