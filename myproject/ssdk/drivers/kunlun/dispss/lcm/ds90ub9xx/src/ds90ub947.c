/**
 * @file ds90ub947.c
 * @brief display subsystem source file.
 *
 * @copyright Copyright (c) 2019-2029 Semidrive Semiconductor.
 *            All rights reserved.
 */
#include <string.h>
#include "ds90ub9xx.h"

void ds90ub947_bl_init(i2c_adap_dev_t *adap, uint32_t ser_addr, uint32_t des_addr)
{
    uint8_t value = 0;

    LOGD("%s config lvds gpio 0\n", __func__);
    ds90ub9xx_write_reg(adap, ser_addr, 0x1E, 0x01);
    ds90ub9xx_write_reg(adap, ser_addr, 0x03, 0x08);
    value = 0x1e | 0x80;
    ds90ub9xx_write_reg(adap, ser_addr, 0x17, value);
    mdelay(10);
    value = 0;
    ds90ub9xx_read_reg(adap, ser_addr, 0x0D, &value);
    value &= ~0x0f;
    value |= 0x03;
    ds90ub9xx_write_reg(adap, ser_addr, 0x0D, value);
    value = 0;
    ds90ub9xx_read_reg(adap, des_addr, 0x1D, &value);
    value &= ~0x0f;
    value |= 0x05;
    ds90ub9xx_write_reg(adap, des_addr, 0x1D, value);
}

void ds90ub947_init(i2c_adap_dev_t *adap, uint32_t slave_addr)
{
    LOGD("%s enter\n", __func__);
#if defined(LVDS_HSD101_SERDES_1280X800_LCD)
    ds90ub9xx_write_reg(adap, slave_addr, 0x12, 0x04);

    ds90ub9xx_write_reg(adap, slave_addr, 0x1E, 0x01);//Select FPD-Link III Port 0
    ds90ub9xx_write_reg(adap, slave_addr, 0x1E, 0x04);//Use I2D ID+1 for FPD-Link III Port 1 register access
    ds90ub9xx_write_reg(adap, slave_addr, 0x1E, 0x01);//Select FPD-Link III Port 0
    ds90ub9xx_write_reg(adap, slave_addr, 0x03, 0x9A);//Enable I2C_PASSTHROUGH, FPD-Link III Port 0
    ds90ub9xx_write_reg(adap, slave_addr, 0x1E, 0x02);//Select FPD-Link III Port 1
    ds90ub9xx_write_reg(adap, slave_addr, 0x03, 0x9A);//Enable I2C_PASSTHROUGH, FPD-Link III Port 1

    //OLDI_DUAL:Single-pixel mode,MAPSEL: OpenLDI Bit Mapping
    ds90ub9xx_write_reg(adap, slave_addr, 0x4f, 0x40);

    //REPEATER:Disable repeater mode
    ds90ub9xx_write_reg(adap, slave_addr, 0xc2, 0x98);

    //COAX:Enable FPD-Link III for coaxial cabling
    ds90ub9xx_write_reg(adap, slave_addr, 0x5b, 0xa0);
#else
    ds90ub9xx_write_reg(adap, slave_addr, 0x1E, 0x01);//Select FPD-Link III Port 0
    ds90ub9xx_write_reg(adap, slave_addr, 0x1E, 0x04);//Use I2D ID+1 for FPD-Link III Port 1 register access
    ds90ub9xx_write_reg(adap, slave_addr, 0x1E, 0x01);//Select FPD-Link III Port 0
    ds90ub9xx_write_reg(adap, slave_addr, 0x03, 0x9A);//Enable I2C_PASSTHROUGH, FPD-Link III Port 0
    ds90ub9xx_write_reg(adap, slave_addr, 0x1E, 0x02);//Select FPD-Link III Port 1
    ds90ub9xx_write_reg(adap, slave_addr, 0x03, 0x9A);//Enable I2C_PASSTHROUGH, FPD-Link III Port 1

    ds90ub9xx_write_reg(adap, slave_addr, 0x1E, 0x01);//Select FPD-Link III Port 0
    ds90ub9xx_write_reg(adap, slave_addr, 0x5B, 0x02);//Force dual mode

    ds90ub9xx_write_reg(adap, slave_addr, 0x40, 0x10);// select OLDI register
    ds90ub9xx_write_reg(adap, slave_addr, 0x41, 0x49);// force PLL controller in PPM reset state
    ds90ub9xx_write_reg(adap, slave_addr, 0x42, 0x16);
    ds90ub9xx_write_reg(adap, slave_addr, 0x41, 0x47);// force PLL LOCK Low
    ds90ub9xx_write_reg(adap, slave_addr, 0x42, 0x20);
    ds90ub9xx_write_reg(adap, slave_addr, 0x42, 0xA0);// reset PLL divider
    ds90ub9xx_write_reg(adap, slave_addr, 0x42, 0x20);
    ds90ub9xx_write_reg(adap, slave_addr, 0x42, 0x00);// release PLL LOCK control
    ds90ub9xx_write_reg(adap, slave_addr, 0x41, 0x49);// release PLL state control
    ds90ub9xx_write_reg(adap, slave_addr, 0x42, 0x00);
#endif
    LOGD("%s done\n", __func__);
}

void ds90ub948_match_947_init(i2c_adap_dev_t *adap, uint32_t slave_addr)
{
    uint8_t val = 0;

    LOGD("%s enter\n", __func__);
#if defined(LVDS_HSD101_SERDES_1280X800_LCD)
    ds90ub9xx_write_reg(adap, slave_addr, 0x49, 0x42);//set MAPSEL low
#else
    ds90ub9xx_write_reg(adap, slave_addr, 0x49, 0x60);//set MAPSEL high
#endif
    ds90ub9xx_read_reg(adap, slave_addr, 0x49, &val);
    LOGD("%s: read: reg=0x49, val=0x%x\n", __func__, val);
}