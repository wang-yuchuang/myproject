/**
 * @file ds90ub927.c
 * @brief display subsystem source file.
 *
 * @copyright Copyright (c) 2019-2029 Semidrive Semiconductor.
 *            All rights reserved.
 */
#include <string.h>
#include "ds90ub9xx.h"

void ds90ub927_init(i2c_adap_dev_t *adap, uint32_t slave_addr)
{
    LOGD("%s enter\n", __func__);
    ds90ub9xx_write_reg(adap, slave_addr, 0x03, 0x9A);//Enable I2C_PASSTHROUGH
    //ds90ub9xx_write_reg(adap, slave_addr, 0x01, 0x01);
    ds90ub9xx_write_reg(adap, slave_addr, 0x0D, 0x03);//Enable I2C_PASSTHROUGH
    LOGD("%s done\n", __func__);
}

void ds90ub928_match_927_init(i2c_adap_dev_t *adap, uint32_t slave_addr)
{
    LOGD("%s enter\n", __func__);
    ds90ub9xx_write_reg(adap, slave_addr, 0x1D, 0x05);
    LOGD("%s done\n", __func__);
}