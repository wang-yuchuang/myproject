
//*****************************************************************************
//
// WARNING: Automatically generated file, don't modify anymore!!!
//
// Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
// Software License Agreement
//
//*****************************************************************************

#ifdef CONFIG_I2C

#include <stdio.h>
#include <types.h>
#include <board.h>
#include <driver/i2c/dw_i2c.h>
#include "i2c_cfg.h"


/* i2c device drivers */
i2c_adap_dev_t g_i2c3_adap_dev;
dw_i2c_context g_dw_i2c3_cxt;

/* i2c config structures for TCA9539 Safety IO-3.3V*/
i2c_adap_cfg_t i2c3_adap_cfg = {
    .base = DEVICE_BASE(I2C3),
    .irq = DEVICE_INTR(I2C3_IC),
    .clk = 24000000,
    .enable = true,
    .opmode = I2C_MASTER,
    .speed = I2C_SSPEED,
};

i2c_adap_dev_t g_i2c4_adap_dev;
dw_i2c_context g_dw_i2c4_cxt;

/* i2c config structures for TCA9539 Safety IO-3.3V*/
i2c_adap_cfg_t i2c4_adap_cfg = {
    .base = DEVICE_BASE(I2C4),
    .irq = DEVICE_INTR(I2C4_IC),
    .clk = 24000000,
    .enable = true,
    .opmode = I2C_MASTER,
    .speed = I2C_SSPEED,
};

/* i2c early initialization */
void  board_i2c_early_init(void)
{
    dw_i2c_adap_probe(&g_i2c3_adap_dev, &i2c3_adap_cfg, &g_dw_i2c3_cxt, 0);
    dw_i2c_adap_probe(&g_i2c4_adap_dev, &i2c4_adap_cfg, &g_dw_i2c4_cxt, 1);
}

static int res_vaild_check(uint32_t i2c_bus_num){
    int ret = -1;

    if( i2c_bus_num <=4 ){
        uint32_t res_location = *(uint32_t *)0xfc280000;
        printf("res_location=0x%x off=0x%x\n", res_location , res_location>>8);
        res_location = res_location>>(8+i2c_bus_num-1);
        res_location &= 0x01;
        printf("I2C_%d is in %s\n", i2c_bus_num, (res_location == 0)? "saf":"ap");
        if(res_location != 0){
            ret = -1;  //res is in ap,can't use in saf
        }else{
            ret = 0;
        }
    }

    return ret;
}

/* i2c initialization */
void  board_i2c_init(void)
{
    i2c_adap_init(&g_i2c3_adap_dev);
    i2c_adap_init(&g_i2c4_adap_dev);
}
#endif
