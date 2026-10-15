/**
 * @file sdrv_i2c_demo.c
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#include <debug.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <CLI_type.h>
#include <driver/i2c.h>

static int i2c_pwrite(i2c_adap_dev_t *adap, uint16_t addr,
                     const uint8_t *wbuf, int wlen)
{
    int ret;
    struct i2c_msg msgs[1] = {
        {
            .addr = addr,
            .buf = (uint8_t *)wbuf,
            .len = wlen,
            .addr_flag = I2C_ADDR7B,
            .rw_flag = I2C_M_W,
            .polling = 1,
        },
    };
    ret = i2c_transfer(adap, msgs, 1);
    return ret;
}

static int i2c_pread(i2c_adap_dev_t *adap, uint16_t addr,
                      uint8_t *rbuf, int rlen)
{
    int ret;
    struct i2c_msg msgs[1] = {
        {
            .addr = addr,
            .buf = rbuf,
            .len = rlen,
            .addr_flag = I2C_ADDR7B,
            .rw_flag = I2C_M_R,
            .polling = 1,
        },
    };
    ret = i2c_transfer(adap, msgs, 1);
    return ret;
}

static int i2c_pwrite_pread(i2c_adap_dev_t *adap, uint16_t addr,
                    const uint8_t *wbuf, int wlen,  uint8_t *rbuf, int rlen)
{
    int ret;
    struct i2c_msg msgs[2] = {
        {
            .addr = addr,
            .buf = (uint8_t *)wbuf,
            .len = wlen,
            .addr_flag = I2C_ADDR7B,
            .rw_flag = I2C_M_W,
            .polling = 1,
        },
        {
            .addr = addr,
            .buf = rbuf,
            .len = rlen,
            .addr_flag = I2C_ADDR7B,
            .rw_flag = I2C_M_R,
            .polling = 1,
        },
    };
    ret = i2c_transfer(adap, msgs, 2);
    return ret;
}

static int i2c_demo(int argc, char *argv[])
{
    int ret = 0;
    uint8_t addr = 0;
    uint32_t id = 0;
    i2c_adap_dev_t *adap = NULL;

    if (argc == 0) {
        printf("Pls choose i2c adapter num\n");
        printf("i2c_demo [1~8]\n");
        return CLI_FALSE;
    }

    id = strtoul(argv[0], NULL, 16);

    adap = i2c_get_adap_dev(id);
    if (!adap) {
        printf("i2c adap %u offline\n", id);
        return CLI_FALSE;
    }

    /* auto detect first available slv for demo */
    {
        for (addr = 0x8; addr <= 0x77; addr++) {
            ret = i2c_write(adap, addr, NULL, 0);
            if (!ret)
                break;
        }

        if (ret) {
            printf("no slave detect\n");
            return CLI_FALSE;
        }
    }

    printf("\nslave %#x detect for demo test start\n", addr);

    /******************************************
    ** uapi i2c function start
    ** use adapter interrupt mode
    ******************************************/
    //read slave default reg 0x0 value
    uint8_t reg = 0x0;
    uint8_t val = 0x0;
    ret = i2c_write_read(adap, addr, &reg, 1, &val, 1);
    if (!ret)
        printf("write + read pass\n");

    //write back value to slave default reg 0x0
    uint8_t wbuf[2];
    wbuf[0] = reg;
    wbuf[1] = val;
    ret = i2c_write(adap, addr, wbuf, 2);
    if (!ret)
        printf("write back pass\n");

    //random read, no assigned reg
    ret = i2c_read(adap, addr, &val, 1);
    if (!ret)
        printf("random read pass\n");

    /******************************************
    ** uapi i2c function start
    ** use adapter polling mode
    ** only when interrupt mode can not work
    ******************************************/
    //read slave default reg 0x0 value
    ret = i2c_pwrite_pread(adap, addr, &reg, 1, &val, 1);
    if (!ret)
        printf("pwrite + pread pass\n");

    //write back value to slave default reg 0x0
    wbuf[0] = reg;
    wbuf[1] = val;
    ret = i2c_pwrite(adap, addr, wbuf, 2);
    if (!ret)
        printf("pwrite back pass\n");

    //random read, no assigned reg
    ret = i2c_pread(adap, addr, &val, 1);
    if (!ret)
        printf("random pread pass\n");

    printf("slave %#x detect for demo test end\n", addr);

    return CLI_FALSE;
}

CLI_CMD("i2c_demo", "\r\ni2c_demo:\r\n i2c demo\r\n", i2c_demo);

