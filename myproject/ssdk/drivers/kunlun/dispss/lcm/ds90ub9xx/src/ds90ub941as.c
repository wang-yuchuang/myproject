/**
 * @file ds90ub941as.c
 * @brief display subsystem source file.
 *
 * @copyright Copyright (c) 2019-2029 Semidrive Semiconductor.
 *            All rights reserved.
 */
#include <string.h>
#include "ds90ub9xx.h"

static int ds90ub941as_bl_init(i2c_adap_dev_t *adap, uint32_t ser_addr, uint32_t des_addr)
{
    uint8_t value = 0;

    LOGD("%s enter\n", __func__);
    ds90ub9xx_write_reg(adap, ser_addr, 0x1E, 0x04);
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
    LOGD("%s end\n", __func__);

    return 0;
}

void ds90ub941as_init(i2c_adap_dev_t *adap, uint32_t slave_addr)
{
    int ret = 0;
    uint8_t value = 0;

    LOGD("%s enter\n", __func__);
    ret = ds90ub9xx_read_reg(adap, slave_addr, 0x1E, &value);
    if (ret < 0) {
        LOGE("[err] : screen offline !\n");
        return;
    }

    //ds90ub9xx_write_reg(adap, slave_addr, 0x01,0x08); //Disable DSI
#if defined(MIPI_HSD101_SERDES_1280X800_LCD)
    ds90ub9xx_write_reg(adap, slave_addr, 0x12, 0x04); //set 18-bit per pixel
#endif
    ds90ub9xx_write_reg(adap, slave_addr, 0x1E, 0x01);//Select FPD-Link III Port 0
    ds90ub9xx_write_reg(adap, slave_addr, 0x1E, 0x04);//Use I2D ID+1 for FPD-Link III Port 1 register access
    ds90ub9xx_write_reg(adap, slave_addr, 0x1E, 0x01);//Select FPD-Link III Port 0
    ds90ub9xx_write_reg(adap, slave_addr, 0x03, 0x9A);//Enable I2C_PASSTHROUGH, FPD-Link III Port 0
    ds90ub9xx_write_reg(adap, slave_addr, 0x1E, 0x02);//Select FPD-Link III Port 1
    ds90ub9xx_write_reg(adap, slave_addr, 0x03, 0x9A);//Enable I2C_PASSTHROUGH, FPD-Link III Port 1

    ds90ub9xx_write_reg(adap, slave_addr, 0x1E, 0x01);//Select FPD-Link III Port 0
    ds90ub9xx_write_reg(adap, slave_addr, 0x40, 0x05);//Select DSI Port 0 digital registers
    ds90ub9xx_write_reg(adap, slave_addr, 0x41, 0x21);//Select DSI_CONFIG_1 register
    ds90ub9xx_write_reg(adap, slave_addr, 0x42, 0x60);//Set DSI_VS_POLARITY=DSI_HS_POLARITY=1(active low)

    ds90ub9xx_write_reg(adap, slave_addr, 0x1E, 0x02);//Select FPD-Link III Port 1
    ds90ub9xx_write_reg(adap, slave_addr, 0x40, 0x09);//Select DSI Port 1 digital registers
    ds90ub9xx_write_reg(adap, slave_addr, 0x41, 0x21);//Select DSI_CONFIG_1 register
    ds90ub9xx_write_reg(adap, slave_addr, 0x42, 0x60);//Set DSI_VS_POLARITY=DSI_HS_POLARITY=1(active low)

    ds90ub9xx_write_reg(adap, slave_addr, 0x1E, 0x01);//Select FPD-Link III Port 0
    ds90ub9xx_write_reg(adap, slave_addr, 0x5B, 0x05);//Force Independent 2:2 mode
    //ds90ub9xx_write_reg(adap, slave_addr, 0x5B, 0x01);//Forced Single FPD-Link III Transmitter mode (Port 1 disabled)
    ds90ub9xx_write_reg(adap, slave_addr, 0x4F, 0x8C);//Set DSI_CONTINUOUS_CLOCK, 4 lanes, DSI Port 0

    ds90ub9xx_write_reg(adap, slave_addr, 0x1E, 0x01);//Select FPD-Link III Port 0
    ds90ub9xx_write_reg(adap, slave_addr, 0x40, 0x04);//Select DSI Port 0 digital registers
    ds90ub9xx_write_reg(adap, slave_addr, 0x41, 0x05);//Select DPHY_SKIP_TIMING register
#if defined(MIPI_KD070_SERDES_1024X600_LCD)
    ds90ub9xx_write_reg(adap, slave_addr, 0x42, 0x10);//Write TSKIP_CNT value for 200 MHz DSI clock frequency (1024x600, Round(65x0.2) -5)
#elif defined(MIPI_HSD101_SERDES_1280X800_LCD)
    ds90ub9xx_write_reg(adap, slave_addr, 0x42, 0x16);//Write TSKIP_CNT value for 250 MHz DSI clock frequency (1280x800, Round(65x0.25) -5)
#else
    ds90ub9xx_write_reg(adap, slave_addr, 0x42, 0x15);//Write TSKIP_CNT value for 300 MHz DSI clock frequency (1920x720, Round(65x0.3) -5)
#endif

    ds90ub9xx_write_reg(adap, slave_addr, 0x1E, 0x02);//Select FPD-Link III Port 1
    ds90ub9xx_write_reg(adap, slave_addr, 0x4F, 0x8C);//Set DSI_CONTINUOUS_CLOCK, 4 lanes, DSI Port 1

    ds90ub9xx_write_reg(adap, slave_addr, 0x1E, 0x01);//Select FPD-Link III Port 0
    ds90ub9xx_write_reg(adap, slave_addr, 0x40, 0x08);//Select DSI Port 1 digital registers
    ds90ub9xx_write_reg(adap, slave_addr, 0x41, 0x05);//Select DPHY_SKIP_TIMING register
#if defined(MIPI_KD070_SERDES_1024X600_LCD)
    ds90ub9xx_write_reg(adap, slave_addr, 0x42, 0x10);//Write TSKIP_CNT value for 200 MHz DSI clock frequency (1920x720, Round(65x0.2) -5)
#elif defined(MIPI_HSD101_SERDES_1280X800_LCD)
    ds90ub9xx_write_reg(adap, slave_addr, 0x42, 0x16);//Write TSKIP_CNT value for 250 MHz DSI clock frequency (1280x800, Round(65x0.25) -5)
#else
    ds90ub9xx_write_reg(adap, slave_addr, 0x42, 0x15);//Write TSKIP_CNT value for 300 MHz DSI clock frequency (1920x720, Round(65x0.3) -5)
#endif
    ds90ub9xx_write_reg(adap, slave_addr, 0x01, 0x00);//Enable DSI

    LOGD("%s done\n", __func__);
}

void ds90ub948_init(i2c_adap_dev_t *adap, uint32_t slave_addr)
{
    LOGD("%s enter\n", __func__);
#if defined(MIPI_HSD123_SERDES_1920X720_LCD)
    uint8_t val;

    ds90ub9xx_write_reg(adap, slave_addr, 0x49, 0x60);//set MAPSEL high
    ds90ub9xx_read_reg(adap, slave_addr, 0x49, &val);
    LOGD("%s: read: reg=0x49, val=0x%x\n", __func__, val);
#endif

#if defined(MIPI_KD070_SERDES_1024X600_LCD)
    uint8_t val;

    if (0) {
        ds90ub9xx_write_reg(adap, slave_addr, 0x1E, 0x9);
        ds90ub9xx_read_reg(adap, slave_addr, 0x1E, &val);
        LOGD("%s: read: reg=0x1D, val=0x%x\n", __func__, val);

        ds90ub9xx_write_reg(adap, slave_addr, 0x20, 0x9);
        ds90ub9xx_read_reg(adap, slave_addr, 0x20, &val);
        LOGD("%s: read: reg=0x20, val=0x%x\n", __func__, val);
    }
#endif

#if defined(MIPI_HSD101_SERDES_1280X800_LCD)
    uint8_t val;

    if (0) {
        ds90ub9xx_write_reg(adap, slave_addr, 0x49, 0x42);//set MAPSEL low
        ds90ub9xx_read_reg(adap, slave_addr, 0x49, &val);

        LOGD("%s: read: reg=0x49, val=0x%x\n", __func__, val);
    }
#endif
    LOGD("%s done\n", __func__);
}

void ds90ub941as_Enable_Dualdsi_DualPort(i2c_adap_dev_t *adap, uint32_t slave_addr)
{
    LOGD("%s enter\n", __func__);
    ds90ub9xx_write_reg(adap, slave_addr, 0x1E, 0x01);//Select FPD-Link III Port 0
    ds90ub9xx_write_reg(adap, slave_addr, 0x03, 0x9A);//Enable I2C_PASSTHROUGH, FPD-Link III Port 0
    ds90ub9xx_write_reg(adap, slave_addr, 0x4F, 0x8C);//Set DSI_CONTINUOUS_CLOCK, 4 lanes, DSI Port 0
    ds90ub9xx_write_reg(adap, slave_addr, 0x5B, 0x00);//auto detect FPD-Link
    ds90ub9xx_write_reg(adap, slave_addr, 0x01, 0x00);//Enable DSI
    LOGD("%s done\n", __func__);
}

bool ub941_get_serdes_link(i2c_adap_dev_t *adap, uint32_t slave_addr)
{
    int ret = 0;
    uint8_t val = 0;

    ret = ds90ub9xx_read_reg(adap, slave_addr, 0x5A, &val);
    if (ret < 0)
        return false;

    LOGD("read 0x5A -> 0x%x\n", val);

    if (val & UB941_TX_STS_MASK)
        if (((val & UB941_FPD3_PORT_MASK) >> UB941_FPD3_PORT_SHIFT) == DUAL_LINK)
            return true;

    return false;
}