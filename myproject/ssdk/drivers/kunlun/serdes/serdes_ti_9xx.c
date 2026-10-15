/**
 * @file serdes_ti_9xx.c
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: ti serdes implement file
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


/* =========================== enumeration define session ================== */
/* =========================== global value define session ================= */

/* =========================== marco define session ======================== */
#define STRUCT_MEMBER_OFFSET(x, y)         (size_t)(&((x *)(0))->y)

#define SDRV_TI9XX_DEBUG_LVL      SSDK_DEBUG
#define EXTRA_FMT   "sdrv_ti9xx: "
#if SDRV_TI9XX_DEBUG_LVL >= SSDK_ERR
#define SDRV_TI9XX_ERR(format, ...) \
    ssdk_printf(SSDK_ERR, EXTRA_FMT format, ##__VA_ARGS__)
#else
#define SDRV_TI9XX_ERR(format, ...)
#endif

#if SDRV_TI9XX_DEBUG_LVL >= SSDK_WARNING
#define SDRV_TI9XX_WARN(format, ...) \
    ssdk_printf(SSDK_WARNING, EXTRA_FMT format, ##__VA_ARGS__)
#else
#define SDRV_TI9XX_WARN(format, ...)
#endif

#if SDRV_TI9XX_DEBUG_LVL >= SSDK_INFO
#define SDRV_TI9XX_INFO(format, ...) \
    ssdk_printf(SSDK_INFO, EXTRA_FMT format, ##__VA_ARGS__)
#else
#define SDRV_TI9XX_INFO(format, ...)
#endif

/* =========================== structure define session ==================== */
/** i2c trans data structure */
typedef struct i2c_transfer {
    uint8_t reg_address;
    uint8_t val[10];
} __PACKED i2c_transfer_t;

/** sdrv serdes driver structure */
typedef struct sdrv_serdes_drv {
    serdes_dev_t dev;
}sdrv_serdes_drv_t;

/* =========================== function declare session ==================== */
int32_t ti9xx_init(serdes_dev_t* dev);
int32_t ti9xx_deinit(serdes_dev_t* dev);
int32_t ti9xx_check_serdes_link(serdes_dev_t* dev);
int32_t ti9xx_device_control(serdes_dev_t* dev, int32_t ctlType, void* data, uint8_t* dataLen);
int32_t ti9xx_ser_reg_read(serdes_dev_t* dev, uint32_t reg, uint32_t* regVal);
int32_t ti9xx_ser_reg_write(serdes_dev_t* dev, uint32_t reg, uint32_t regVal);
int32_t ti9xx_des_reg_read(serdes_dev_t* dev, uint32_t reg, uint32_t* regVal);
int32_t ti9xx_des_reg_write(serdes_dev_t* dev, uint32_t reg, uint32_t regVal);

/* =========================== static value define session ================= */
static serdes_ops_t s_serdes_ops = {
    .init = ti9xx_init,
    .deinit = ti9xx_deinit,
    .check_serdes_link = ti9xx_check_serdes_link,
    .device_control = ti9xx_device_control,
    .ser_reg_read = ti9xx_ser_reg_read,
    .ser_reg_write = ti9xx_ser_reg_write,
    .des_reg_read = ti9xx_des_reg_read,
    .des_reg_write = ti9xx_des_reg_write,
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
int serdes_i2c_read(i2c_adap_dev_t* adap, uint8_t addr, i2c_transfer_t* buf, int len)
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

int serdes_i2c_write(i2c_adap_dev_t *adap, uint8_t addr, i2c_transfer_t* buf, int len)
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

int32_t ti9xx_init(serdes_dev_t* dev)
{
    return 0;
}

int32_t ti9xx_deinit(serdes_dev_t* dev)
{
    // do nothing
    return 0;
}

int32_t ti9xx_check_serdes_link(serdes_dev_t* dev)
{
    int32_t ret = -1;
    if (dev == NULL || dev->priv == NULL) {
        SDRV_TI9XX_ERR("invlaid parameter for %s\n", __func__);
        return ret;
    }

    i2c_adap_dev_t* i2c_dev = dev->cfg_ptr->i2c_dev;
    uint8_t ser_addr = dev->cfg_ptr->ser_addr;
    uint8_t des_addr = dev->cfg_ptr->des_addr;

    i2c_transfer_t trans_buf;
    memset(&trans_buf, 0x00, sizeof(trans_buf));
    trans_buf.reg_address = 0x6;

    ret = serdes_i2c_read(i2c_dev, ser_addr, &trans_buf, 1);
    if (ret) {
        SDRV_TI9XX_ERR("%s: ti ser_type=%d, ser_addr=%x, address invalid\n",
                        __func__, dev->cfg_ptr->ser_type, ser_addr);
        ret = SERDES_RET_I2C_RW_TMO;
    }
    else if (trans_buf.val[0] >> 1 != des_addr) {
        SDRV_TI9XX_ERR("%s: ti des_type=%d, cfg des_addr=%x, read des_addr=%x, offline\n",
            __func__, dev->cfg_ptr->des_type, des_addr, trans_buf.val[0] >> 1);
        ret = SERDES_RET_DES_OFFLINE;
    }
    else {
        // on line
        SDRV_TI9XX_INFO("%s: ti des_type=%d, cfg des_addr=%x, read des_addr=%x, online\n",
            __func__, dev->cfg_ptr->des_type, des_addr, trans_buf.val[0] >> 1);
        ret = SERDES_RET_SUCESS;
    }

    return ret;

}

static int32_t enable_port(serdes_dev_t* dev)
{
    if (dev == NULL) {
        SDRV_TI9XX_ERR("invalid parameter for %s.\n", __func__);
        return -1;
    }

    uint8_t sval = 0;

    int32_t port_flag = dev->cfg_ptr->ser_type;

    if (port_flag == TI941_SINGLE || port_flag == TI947_SINGLE)
        sval = 0x1;
    else if (port_flag == TI941_SECOND || port_flag == TI947_SECOND)
        sval = 0x2;
    else if (port_flag == TI941_DUAL || port_flag == TI947_DUAL)
        sval = 0x4;
    else {
        SDRV_TI9XX_ERR("%s, port_flag=%d invalid, addr=%#x\n",
                __func__, port_flag, dev->cfg_ptr->ser_addr);
        return -1;
    }

    i2c_transfer_t write_data = {
        .reg_address = 0x1e,
        .val = {sval}
    };

    return serdes_i2c_write(dev->cfg_ptr->i2c_dev, dev->cfg_ptr->ser_addr, &write_data, 1);
}

static int32_t enable_i2c_passthrough(serdes_dev_t* dev)
{
    i2c_transfer_t write_data = {
        .reg_address = 0x17,
        .val = {0x9e}
    };

    return serdes_i2c_write(dev->cfg_ptr->i2c_dev, dev->cfg_ptr->ser_addr, &write_data, 1);
}

static int32_t du90ub948_gpio_output(serdes_dev_t* dev, int32_t gpio, int32_t val)
{
    int ret;
    uint8_t dreg = 0, dval = 0;

    switch (gpio) {
        case 0:
            dreg = 0x1d;
            break;
        case 1:
        case 2:
            dreg = 0x1e;
            break;
        case 3:
            dreg = 0x1f;
            break;
        case 5:
        case 6:
            dreg = 0x20;
            break;
        case 7:
        case 8:
            dreg = 0x21;
            break;
        default:
            SDRV_TI9XX_ERR("%s, 948(0) gpio num %d invalid\n", __func__, gpio);
            return -1;
    }

    i2c_transfer_t gpio_data = {
        .reg_address = dreg,
        .val = {0}
    };
    // ret = hal_i2c_read_reg_data(i2c_handle, addr, &dreg, 1, &dval, 1);
    ret = serdes_i2c_read(dev->cfg_ptr->i2c_dev, dev->cfg_ptr->des_addr, &gpio_data, 1);
    dval = gpio_data.val[0];

    switch (gpio) {
        case 0:
        case 1:
        case 3:
        case 5:
        case 7:
            dval &= 0xf0;

            if (val == 1)
                dval |= 0x09;
            else
                dval |= 0x01;
            break;

        case 2:
        case 6:
        case 8:
            dval &= 0x0f;

            if (val == 1)
                dval |= 0x90;
            else
                dval |= 0x10;
            break;

        default:
            SDRV_TI9XX_ERR("%s, 948(1) gpio num %d invalid\n", __func__, gpio);
            return -1;
    }

    // ret = hal_i2c_write_reg_data(i2c_handle, addr, &dreg, 1, &dval, 1);
    gpio_data.val[0] = dval;
    ret = serdes_i2c_write(dev->cfg_ptr->i2c_dev, dev->cfg_ptr->des_addr, &gpio_data, 1);
    // if (ret) {
    //     dprintf(ALWAYS, "%s: addr=%#x, i2c write failed\n", __func__, addr);
    //     return -1;
    // }

    return ret;
}

static int du90ub948_gpio_input(serdes_dev_t* dev, int32_t gpio)
{
    int ret;
    uint8_t dreg = 0, dval = 0;

    switch (gpio) {
        case 0:
            dreg = 0x1d;
            break;
        case 1:
        case 2:
            dreg = 0x1e;
            break;
        case 3:
            dreg = 0x1f;
            break;
        case 5:
        case 6:
            dreg = 0x20;
            break;
        case 7:
        case 8:
            dreg = 0x21;
            break;
        default:
            SDRV_TI9XX_ERR("%s, 948(0) gpio num %d invalid\n", __func__, gpio);
            return -1;
    }

    i2c_transfer_t gpio_data = {
        .reg_address = dreg,
        .val = {0}
    };

    ret = serdes_i2c_read(dev->cfg_ptr->i2c_dev, dev->cfg_ptr->des_addr, &gpio_data, 1);
    if (ret) {
        SDRV_TI9XX_ERR("%s: addr=%#x, dreg=%#x, i2c read failed\n",
                __func__, dev->cfg_ptr->des_addr, dreg);
    }
    dval = gpio_data.val[0];

    switch (gpio) {
        case 0:
        case 1:
        case 3:
        case 5:
        case 7:
            dval &= 0xf0;
            dval |= 0x03;
            break;
        case 2:
        case 6:
        case 8:
            dval &= 0x0f;
            dval |= 0x30;
            break;
        default:
            SDRV_TI9XX_ERR("%s, 948(1) gpio num %d invalid\n", __func__, gpio);
            return -1;
    }

    gpio_data.val[0] = dval;
    ret = serdes_i2c_write(dev->cfg_ptr->i2c_dev, dev->cfg_ptr->des_addr, &gpio_data, 1);
    if (ret) {
        SDRV_TI9XX_ERR("%s: addr=%#x, i2c write failed\n", __func__, dev->cfg_ptr->des_addr);
        return -1;
    }

    return 0;
}

static int du90ub941_or_947_gpio_input(serdes_dev_t* dev, int32_t gpio)
{
    int ret;
    uint8_t sreg = 0, sval = 0;

    switch (gpio) {
        case 0:
            sreg = 0x0d;
            break;
        case 1:
        case 2:
            sreg = 0x0e;
            break;
        case 3:
            sreg = 0x0f;
            break;
        case 5:
        case 6:
            sreg = 0x10;
            break;
        case 7:
        case 8:
            sreg = 0x11;
            break;
        default:
            SDRV_TI9XX_ERR("%s, --gpio num %d invalid\n", __func__, gpio);
            return -1;
    }

    i2c_transfer_t gpio_data = {
        .reg_address = sreg,
        .val = {0}
    };
    ret = serdes_i2c_read(dev->cfg_ptr->i2c_dev, dev->cfg_ptr->ser_addr, &gpio_data, 1);
    if (ret) {
        SDRV_TI9XX_ERR("%s: addr=%#x, sreg=%#x, i2c read failed\n",
                __func__, dev->cfg_ptr->des_addr, sreg);
    }
    sval = gpio_data.val[0];

    switch (gpio) {
        case 0:
        case 1:
        case 3:
            sval &= 0xf0;
            sval |= 0x05;
            break;
        case 2:
            sval &= 0x0f;
            sval |= 0x50;
            break;
        case 5:
        case 7:
            sval &= 0xf0;
            sval |= 0x03;
            break;
        case 6:
        case 8:
            sval &= 0x0f;
            sval |= 0x30;
            break;
        default:
            SDRV_TI9XX_ERR("%s, ++gpio num %d invalid\n", __func__, gpio);
            return -1;
    }

    gpio_data.val[0] = sval;
    ret = serdes_i2c_write(dev->cfg_ptr->i2c_dev, dev->cfg_ptr->ser_addr, &gpio_data, 1);
    if (ret) {
        SDRV_TI9XX_ERR("%s: addr=%#x, i2c write failed\n", __func__, dev->cfg_ptr->des_addr);
        return -1;
    }

    return ret;
}

static int32_t ser_gpio_ctrl(serdes_dev_t* dev, serdes_gpio_ctrl_t* ctrl)
{
    switch (ctrl->dir) {
        case 0:
            break;
        case 1:
            du90ub941_or_947_gpio_input(dev, ctrl->io_num);
            break;
        default:
            break;
    }

    return 0;
}

static int32_t des_gpio_ctrl(serdes_dev_t* dev, serdes_gpio_ctrl_t* ctrl)
{
    switch (ctrl->dir) {
        case 0:
            du90ub948_gpio_output(dev, ctrl->io_num, ctrl->val);
            break;
        case 1:
            du90ub948_gpio_input(dev, ctrl->io_num);
            break;
        default:
            break;
    }

    return 0;
}

int32_t ti9xx_device_control(serdes_dev_t* dev, int32_t ctlType, void* data, uint8_t* dataLen)
{
    if (dev == NULL && data == NULL && dataLen == NULL) {
        SDRV_TI9XX_ERR("invlaid parameter for %s.\n", __func__);
        return -1;
    }

    switch (ctlType) {
        case SER_CONTROL_ENABLE_I2C_PASSTROUGH:
            enable_i2c_passthrough(dev);
            break;
        case SER_CONTROL_GPIO_CTRL:
            ser_gpio_ctrl(dev, (serdes_gpio_ctrl_t *)data);
            break;
        case SER_CONTROL_PORT_ENABLE:
            enable_port(dev);
            break;
        case DES_CONTROL_GPIO_CTRL:
            des_gpio_ctrl(dev, (serdes_gpio_ctrl_t *)data);
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
        default:
            break;
    }
    return 0;
}

int32_t ti9xx_ser_reg_read(serdes_dev_t* dev, uint32_t reg, uint32_t* regVal)
{
    if (dev == NULL || regVal == NULL ||
        reg > 0xFF) {
        SDRV_TI9XX_ERR("invalid parameter for %s. \n", __func__);
        return -1;
    }

    i2c_transfer_t read_data = {
        .reg_address = reg,
        .val = {0}
    };

    serdes_i2c_read(dev->cfg_ptr->i2c_dev
                    , dev->cfg_ptr->ser_addr
                    , &read_data, 1);
    *regVal = read_data.val[0];

    // SDRV_TI9XX_INFO("[debug]test read ser register: 0x%x and val: 0x%x\n", reg, *regVal);
    return 0;
}

int32_t ti9xx_ser_reg_write(serdes_dev_t* dev, uint32_t reg, uint32_t regVal)
{
    if (dev == NULL || reg > 0xFF) {
        SDRV_TI9XX_ERR("invalid parameter for %s. \n", __func__);
        return -1;
    }

    i2c_transfer_t write_data = {
        .reg_address = reg,
        .val = {regVal}
    };

    serdes_i2c_write(dev->cfg_ptr->i2c_dev
                    , dev->cfg_ptr->ser_addr
                    , &write_data, 1);

    return 0;
}

int32_t ti9xx_des_reg_read(serdes_dev_t* dev, uint32_t reg, uint32_t* regVal)
{
    if (dev == NULL || regVal == NULL ||
        reg > 0xFF) {
        SDRV_TI9XX_ERR("invalid parameter for %s. \n", __func__);
        return -1;
    }

    i2c_transfer_t read_data = {
        .reg_address = reg,
        .val = {0}
    };

    serdes_i2c_read(dev->cfg_ptr->i2c_dev
                    , dev->cfg_ptr->des_addr
                    , &read_data, 1);
    *regVal = read_data.val[0];

    return 0;
}

int32_t ti9xx_des_reg_write(serdes_dev_t* dev, uint32_t reg, uint32_t regVal)
{
    if (dev == NULL || reg > 0xFF) {
        SDRV_TI9XX_ERR("invalid parameter for %s. \n", __func__);
        return -1;
    }

    i2c_transfer_t write_data = {
        .reg_address = reg,
        .val = {regVal}
    };

    serdes_i2c_write(dev->cfg_ptr->i2c_dev
                    , dev->cfg_ptr->des_addr
                    , &write_data, 1);

    return 0;
}

serdes_dev_t* ti9xx_serdes_probe(sdrv_serdes_handle* serdesSdrvHandle, const serdes_config_t* cfgPtr)
{
    serdes_dev_t* dev = NULL;
    if (cfgPtr == NULL) {
        SDRV_TI9XX_ERR("invalid parameter for %s\n", __func__);
        return dev;
    }

    if (*serdesSdrvHandle != NULL) {
        SDRV_TI9XX_INFO("already probed serdes device for %s.\n", cfgPtr->name);
        return &((sdrv_serdes_drv_t *)(*serdesSdrvHandle))->dev;
    }

    *serdesSdrvHandle = malloc(sizeof(sdrv_serdes_drv_t));
    if (*serdesSdrvHandle == NULL) {
        SDRV_TI9XX_ERR("malloc memory for %s device error \n", cfgPtr->name);
        return NULL;
    }

    memset(*serdesSdrvHandle, 0x00, sizeof(sdrv_serdes_drv_t));

    sdrv_serdes_drv_t* serdes_drv = (sdrv_serdes_drv_t *)(*serdesSdrvHandle);
    serdes_drv->dev.cfg_ptr = cfgPtr;
    serdes_drv->dev.name = cfgPtr->name;
    serdes_drv->dev.priv = serdes_drv;

    int32_t ret = ti9xx_check_serdes_link(&serdes_drv->dev);
    if (ret != SERDES_RET_SUCESS &&
        ret != SERDES_RET_DES_OFFLINE) {
        SDRV_TI9XX_WARN("serdes connect status is disconnect \n");
        free(*serdesSdrvHandle);
        *serdesSdrvHandle = NULL;
        return dev;
    }

    dev = &serdes_drv->dev;
    dev->ops = &s_serdes_ops;
    serdes_init(dev);

    return dev;
}
/* end of file serdes_ti_9xx.c */