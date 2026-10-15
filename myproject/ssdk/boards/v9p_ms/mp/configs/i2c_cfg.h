/*
 * Copyright (c) 2021-2029 Semidrive Incorporated.
 * All rights reserved.
 * Software License Agreement.
 */
#ifndef BOARDS_I2C_CFG_H_
#define BOARDS_I2C_CFG_H_

#ifdef CONFIG_I2C

#include <driver/i2c.h>

//#define I2C1_CFG_ENABLE
//#define I2C2_CFG_ENABLE
//#define I2C3_CFG_ENABLE
//#define I2C4_CFG_ENABLE
//#define I2C5_CFG_ENABLE
//#define I2C6_CFG_ENABLE
//#define I2C7_CFG_ENABLE
//#define I2C8_CFG_ENABLE
#define I2C9_CFG_ENABLE
#define I2C10_CFG_ENABLE
//#define I2C11_CFG_ENABLE
//#define I2C12_CFG_ENABLE
//#define I2C13_CFG_ENABLE
#define I2C14_CFG_ENABLE
#define I2C15_CFG_ENABLE
#define I2C16_CFG_ENABLE
//#define I2C17_CFG_ENABLE
//#define I2C18_CFG_ENABLE
//#define I2C19_CFG_ENABLE
//#define I2C20_CFG_ENABLE

/* exported i2c devices */
#ifdef I2C1_CFG_ENABLE
extern i2c_adap_dev_t g_i2c1_adap_dev;
#endif

#ifdef I2C2_CFG_ENABLE
extern i2c_adap_dev_t g_i2c2_adap_dev;
#endif

#ifdef I2C3_CFG_ENABLE
extern i2c_adap_dev_t g_i2c3_adap_dev;
#endif

#ifdef I2C4_CFG_ENABLE
extern i2c_adap_dev_t g_i2c4_adap_dev;
#endif

#ifdef I2C5_CFG_ENABLE
extern i2c_adap_dev_t g_i2c5_adap_dev;
#endif

#ifdef I2C6_CFG_ENABLE
extern i2c_adap_dev_t g_i2c6_adap_dev;
#endif

#ifdef I2C7_CFG_ENABLE
extern i2c_adap_dev_t g_i2c7_adap_dev;
#endif

#ifdef I2C8_CFG_ENABLE
extern i2c_adap_dev_t g_i2c8_adap_dev;
#endif

#ifdef I2C9_CFG_ENABLE
extern i2c_adap_dev_t g_i2c9_adap_dev;
#endif

#ifdef I2C10_CFG_ENABLE
extern i2c_adap_dev_t g_i2c10_adap_dev;
#endif

#ifdef I2C11_CFG_ENABLE
extern i2c_adap_dev_t g_i2c11_adap_dev;
#endif

#ifdef I2C12_CFG_ENABLE
extern i2c_adap_dev_t g_i2c12_adap_dev;
#endif

#ifdef I2C13_CFG_ENABLE
extern i2c_adap_dev_t g_i2c13_adap_dev;
#endif

#ifdef I2C14_CFG_ENABLE
extern i2c_adap_dev_t g_i2c14_adap_dev;
#endif

#ifdef I2C15_CFG_ENABLE
extern i2c_adap_dev_t g_i2c15_adap_dev;
#endif

#ifdef I2C16_CFG_ENABLE
extern i2c_adap_dev_t g_i2c16_adap_dev;
#endif

#ifdef I2C17_CFG_ENABLE
extern i2c_adap_dev_t g_i2c17_adap_dev;
#endif

#ifdef I2C18_CFG_ENABLE
extern i2c_adap_dev_t g_i2c18_adap_dev;
#endif

#ifdef I2C19_CFG_ENABLE
extern i2c_adap_dev_t g_i2c19_adap_dev;
#endif

#ifdef I2C20_CFG_ENABLE
extern i2c_adap_dev_t g_i2c20_adap_dev;
#endif

/* i2c init functions */
extern void board_i2c_early_init(void);
extern void board_i2c_init(void);

#endif

#endif /* BOARDS_I2C_CFG_H */

