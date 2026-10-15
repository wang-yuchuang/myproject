/**
 * @file maxim_rw.c
 * @brief main source file.
 *
 * @copyright Copyright (c) 2021  Semidrive Semiconductor.
 *            All rights reserved.
 */
#include <debug.h>
#include "maxim_rw.h"

static int i2c_read16_reg(i2c_adap_dev_t *adap,
    uint16_t addr, uint16_t reg, uint8_t *val)
{
    uint8_t wreg[2];
    int ret = 0;

    wreg[0] = reg >> 8;
    wreg[1] = reg & 0xff;

    struct i2c_msg msgs[2] = {
        {
            .addr = addr,
            .buf = wreg,
            .len = 2,
            .addr_flag = I2C_ADDR7B,
            .rw_flag = I2C_M_W,
            .polling = 1,
        },
        {
            .addr = addr,
            .buf = val,
            .len = 1,
            .addr_flag = I2C_ADDR7B,
            .rw_flag = I2C_M_R,
            .polling = 1,
        },
    };
    ret = i2c_transfer(adap, msgs, 2);
    return ret;
}

static int i2c_write16_reg(i2c_adap_dev_t *adap,
    uint16_t addr, uint16_t reg, uint8_t val)
{
    uint8_t wreg[3];
    int ret = 0;

    wreg[0] = reg >> 8;
    wreg[1] = reg & 0xff;
    wreg[2] = val;

    struct i2c_msg msgs[1] = {
        {
            .addr = addr,
            .buf = wreg,
            .len = 3,
            .addr_flag = I2C_ADDR7B,
            .rw_flag = I2C_M_W,
        },
    };
    ret = i2c_transfer(adap, msgs, 1);
    return ret;
}

int maxim_read_reg(i2c_adap_dev_t *adap, uint16_t slave_addr,
    uint16_t reg, uint8_t *val)
{
    uint8_t buf = 0;
    int ret = 0;

    ret = i2c_read16_reg(adap, slave_addr, reg, &buf);
    if (ret < 0) {
        ssdk_printf(SSDK_ERR, "%s(),addr[0x%x] read reg=0x%x\n",
                __func__, slave_addr, reg);
        return ret;
    }

    *val = buf;
    ssdk_printf(MAXIM_DEBUG, "%s(),addr[0x%x] read reg=0x%x,value= 0x%x\n",
        __func__, slave_addr, reg, buf);
    return 0;
}

int maxim_write_reg(i2c_adap_dev_t *adap, uint16_t slave_addr,
    uint16_t reg, uint8_t val)
{
    int ret = 0;

    ret = i2c_write16_reg(adap, slave_addr, reg, val);
    if (ret < 0) {
        ssdk_printf(SSDK_ERR, "%s: slave 0x%x write reg 0x%x val 0x%x, ret=%d\n",
                __func__, slave_addr, reg, val, ret);
        return ret;
    }
#if MAXIM_DEBUG
    maxim_read_reg(adap, slave_addr, reg, &val);
#endif
    return ret;
}

