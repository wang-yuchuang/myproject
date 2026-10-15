
//*****************************************************************************
//
// WARNING: Automatically generated file, don't modify anymore!!!
//
// Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
// Software License Agreement
//
//*****************************************************************************

#ifndef BOARDS_I2C_CFG_H_
#define BOARDS_I2C_CFG_H_

#ifdef CONFIG_I2C

#include <driver/i2c.h>

/* exported i2c devices */
extern i2c_adap_dev_t *i2c3_adap_dev;
extern i2c_adap_dev_t *i2c6_adap_dev;
extern i2c_adap_dev_t *i2c7_adap_dev;
extern i2c_adap_dev_t *i2c8_adap_dev;

/* i2c init functions */
extern void board_i2c_early_init(void);
extern void board_i2c_init(void);

#endif

#endif /* BOARDS_I2C_CFG_H */

