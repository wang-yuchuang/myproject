/**
 * @file maxim_rw.h
 * @brief main source file.
 *
 * @copyright Copyright (c) 2021  Semidrive Semiconductor.
 *            All rights reserved.
 */

#ifndef __MAXIM_RW_H
#define __MAXIM_RW_H

#include <driver/i2c.h>

#define MAXIM_DEBUG 0

int maxim_read_reg(i2c_adap_dev_t *adap, uint16_t slave_addr,
    uint16_t reg, uint8_t *val);
int maxim_write_reg(i2c_adap_dev_t *adap, uint16_t slave_addr,
    uint16_t reg, uint8_t val);

#endif

