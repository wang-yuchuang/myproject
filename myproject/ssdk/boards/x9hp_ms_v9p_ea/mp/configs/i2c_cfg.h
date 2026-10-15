
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
extern i2c_adap_dev_t g_i2c9_adap_dev;
extern i2c_adap_dev_t g_i2c10_adap_dev;
extern i2c_adap_dev_t g_i2c14_adap_dev;
extern i2c_adap_dev_t g_i2c15_adap_dev;
extern i2c_adap_dev_t g_i2c16_adap_dev;

/* i2c init functions */
extern void board_i2c_early_init(void);
extern void board_i2c_init(void);

#endif

#endif /* BOARDS_I2C_CFG_H */
