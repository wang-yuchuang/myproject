
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
#include <driver/kunlun/i2c/dw_i2c.h>
#include "i2c_cfg.h"


/* i2c device drivers */
i2c_adap_dev_t g_i2c10_adap_dev;
dw_i2c_context g_dw_i2c10_cxt;

i2c_adap_cfg_t i2c10_adap_cfg = {
    .base = DEVICE_BASE(I2C10),
    .irq = DEVICE_INTR(I2C10_IC),
    .clk = 24000000,
    .enable = true,
    .opmode = I2C_MASTER,
    .speed = I2C_FSPEED,
};
/* i2c early initialization */
void  board_i2c_early_init(void)
{
    dw_i2c_adap_probe(&g_i2c10_adap_dev, &i2c10_adap_cfg, &g_dw_i2c10_cxt, 10);
}

static int res_vaild_check(uint32_t i2c_bus_num){
    uint32_t res_location = *(uint32_t *)0xfc280000;
    int ret = 0;

    if( i2c_bus_num <=4 ){
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
    int ret = -1;
    uint32_t i2c_bus;
    i2c_bus = 10;
    ret = res_vaild_check(i2c_bus);
    if(ret < 0){
        printf("error:board_i2c_init I2C_%d resourece is't in saf domain\n", i2c_bus);
    }else{
        i2c_adap_init(&g_i2c10_adap_dev);
    }
}
#endif
