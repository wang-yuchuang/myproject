/**
 * @file dw_i2c_validation.c
 * @simple i2c validation file
 *
 * @Copyright (c) 2021  Semidrive Semiconductor.
 * @All rights reserved.
 *
 **/

#include <stdio.h>
#include <stdlib.h>

#include <debug.h>
#include <errno.h>
#include <types.h>

#include "CLI_type.h"
#include "driver/i2c.h"
#include <driver/kunlun/i2c/dw_i2c.h>

/* the i2c res list */
/*
I2C1            is in saf domain
I2C2            is in saf domain
I2C3            is in ap  domain
I2C4            is in ap  domain
*/

#ifndef APB_I2C1_BASE
#if defined(CONFIG_MP) || defined(CONFIG_BOARD_D9_AP1)
#define APB_I2C1_BASE              (0x300B0000)
#define APB_I2C2_BASE              (0x300C0000)
#define APB_I2C3_BASE              (0x300D0000)
#define APB_I2C4_BASE              (0x300E0000)
#define APB_I2C16_BASE             (0x30B70000)
#define APB_I2C15_BASE             (0x30B60000)
#define APB_I2C14_BASE             (0x30B50000)
#define APB_I2C13_BASE             (0x30B40000)
#define APB_I2C12_BASE             (0x30B30000)
#define APB_I2C11_BASE             (0x30B20000)
#define APB_I2C10_BASE             (0x30B10000)
#define APB_I2C9_BASE              (0x30B00000)
#define APB_I2C8_BASE              (0x30AF0000)
#define APB_I2C7_BASE              (0x30AE0000)
#define APB_I2C6_BASE              (0x30AD0000)
#define APB_I2C5_BASE              (0x30AC0000)
#else
#define APB_I2C1_BASE              (0xF00B0000)
#define APB_I2C2_BASE              (0xF00C0000)
#define APB_I2C3_BASE              (0xF00D0000)
#define APB_I2C4_BASE              (0xF00E0000)
#define APB_I2C16_BASE             (0xF0B70000)
#define APB_I2C15_BASE             (0xF0B60000)
#define APB_I2C14_BASE             (0xF0B50000)
#define APB_I2C13_BASE             (0xF0B40000)
#define APB_I2C12_BASE             (0xF0B30000)
#define APB_I2C11_BASE             (0xF0B20000)
#define APB_I2C10_BASE             (0xF0B10000)
#define APB_I2C9_BASE              (0xF0B00000)
#define APB_I2C8_BASE              (0xF0AF0000)
#define APB_I2C7_BASE              (0xF0AE0000)
#define APB_I2C6_BASE              (0xF0AD0000)
#define APB_I2C5_BASE              (0xF0AC0000)
#endif
#define I2C1_IC_INTR_NUM            64
#define I2C2_IC_INTR_NUM            65
#define I2C3_IC_INTR_NUM            66
#define I2C4_IC_INTR_NUM            67
#define I2C5_IC_INTR_NUM            68
#define I2C6_IC_INTR_NUM            69
#define I2C7_IC_INTR_NUM            70
#define I2C8_IC_INTR_NUM            71
#define I2C9_IC_INTR_NUM            72
#define I2C10_IC_INTR_NUM           73
#define I2C11_IC_INTR_NUM           74
#define I2C12_IC_INTR_NUM           75
#define I2C13_IC_INTR_NUM           76
#define I2C14_IC_INTR_NUM           77
#define I2C15_IC_INTR_NUM           78
#define I2C16_IC_INTR_NUM           79
#endif

#ifndef  DEVICE_BASE
#define DEVICE_BASE(dev) _DEVICE_BASE(dev)
#define _DEVICE_BASE(dev) APB_##dev##_BASE

#define DEVICE_INTR(dev) _DEVICE_INTR(dev)
#define _DEVICE_INTR(dev) dev##_INTR_NUM
#endif

#define I2C_NUM_MAX 12

i2c_adap_dev_t g_i2c_dev;
dw_i2c_context g_i2c_cxt;

/* i2c config structures for TCA9539 Safety IO-3.3V*/
i2c_adap_cfg_t g_i2c_cfg[] = {
{ DEVICE_BASE(I2C1),   DEVICE_INTR(I2C1_IC),  24000000, true, I2C_MASTER, I2C_SSPEED},
{ DEVICE_BASE(I2C2),   DEVICE_INTR(I2C2_IC),  24000000, true, I2C_MASTER, I2C_SSPEED},
{ DEVICE_BASE(I2C3),   DEVICE_INTR(I2C3_IC),  24000000, true, I2C_MASTER, I2C_SSPEED},
{ DEVICE_BASE(I2C4),   DEVICE_INTR(I2C4_IC),  24000000, true, I2C_MASTER, I2C_SSPEED},
{ DEVICE_BASE(I2C5),   DEVICE_INTR(I2C5_IC),  24000000, true, I2C_MASTER, I2C_SSPEED},
{ DEVICE_BASE(I2C6),   DEVICE_INTR(I2C6_IC),  24000000, true, I2C_MASTER, I2C_SSPEED},
{ DEVICE_BASE(I2C7),   DEVICE_INTR(I2C7_IC),  24000000, true, I2C_MASTER, I2C_SSPEED},
{ DEVICE_BASE(I2C8),   DEVICE_INTR(I2C8_IC),  24000000, true, I2C_MASTER, I2C_SSPEED},
{ DEVICE_BASE(I2C9),   DEVICE_INTR(I2C9_IC),  24000000, true, I2C_MASTER, I2C_SSPEED},
{ DEVICE_BASE(I2C10),  DEVICE_INTR(I2C10_IC), 24000000, true, I2C_MASTER, I2C_SSPEED},
{ DEVICE_BASE(I2C11),  DEVICE_INTR(I2C11_IC), 24000000, true, I2C_MASTER, I2C_SSPEED},
{ DEVICE_BASE(I2C12),  DEVICE_INTR(I2C12_IC), 24000000, true, I2C_MASTER, I2C_SSPEED},
{ DEVICE_BASE(I2C13),  DEVICE_INTR(I2C13_IC), 24000000, true, I2C_MASTER, I2C_SSPEED},
{ DEVICE_BASE(I2C14),  DEVICE_INTR(I2C14_IC), 24000000, true, I2C_MASTER, I2C_SSPEED},
{ DEVICE_BASE(I2C15),  DEVICE_INTR(I2C15_IC), 24000000, true, I2C_MASTER, I2C_SSPEED},
{ DEVICE_BASE(I2C16),  DEVICE_INTR(I2C16_IC), 24000000, true, I2C_MASTER, I2C_SSPEED},
};

static int res_vaild_check(uint32_t i2c_bus_num){
    int ret = 0;

    if( i2c_bus_num <= 4 ){
        uint32_t res_location = *(uint32_t *)0xfc280000;
        //printf("res_location=0x%x off=0x%x\n", res_location , res_location>>8);
        res_location = res_location>>(8+i2c_bus_num-1);
        res_location &= 0x01;
        //printf("I2C_%d is in %s\n", i2c_bus_num, (res_location == 0)? "saf":"ap");
        if(res_location != 0){
            ret = -1;  //res is in ap,can't use in saf
        }else{
            ret = 0;
        }
    }

    return ret;
}

static i2c_adap_dev_t *i2ctools_init_bus(int bus)
{
    int i = 0;
    int ret = -1;

    if(bus > I2C_NUM_MAX){
        printf("i2ctools_init_bus bus is out of range bus_num=%d\n",bus);
        return NULL;
    }

    dw_i2c_adap_probe(&g_i2c_dev, &g_i2c_cfg[bus-1], &g_i2c_cxt, i);
    ret = res_vaild_check(bus);
    if(ret == 0){
        i2c_adap_init(&g_i2c_dev);
        return &g_i2c_dev;
    }

    return NULL;
}

static int cmd_i2cset(int argc, char *argv[])
{
    i2c_adap_dev_t *adap = NULL;
    uint8_t i2c_bus = 0;
    uint16_t i2c_addr = 0;
    uint8_t i2c_reg = 0;
    uint8_t i2c_val = 0;
    int ret = -1;

    i2c_bus = (uint8_t )strtoul(argv[1], NULL, 10);
    i2c_addr = (uint16_t )strtoul(argv[2],NULL, 16);
    i2c_reg = (uint8_t )strtoul(argv[3], NULL, 16);
    i2c_val = (uint8_t )strtoul(argv[4], NULL, 16);
    i2c_addr >>= 1;  //make addr as 7bit format

    adap = i2ctools_init_bus(i2c_bus);
    if(!adap){
        printf("i2c_bus=%d error.\n", i2c_bus);
        return CLI_FALSE;
    }

    ret = i2c_write_reg8(adap, i2c_addr, i2c_reg, &i2c_val, 1);
    if(ret < 0){
        printf("i2c_write_reg error. i2c_bus=%d reg=%d\n", i2c_bus, i2c_reg);
    }else{
        printf("i2c-%d write\ndev_addr(0x%x)\nreg_addr=%x val=%x\n", i2c_bus, i2c_addr, i2c_reg, i2c_val);
    }

    return CLI_FALSE;
}

static int cmd_i2cget(int argc, char *argv[])
{
    i2c_adap_dev_t *adap = NULL;
    uint8_t i2c_bus = 0;
    uint16_t i2c_addr = 0;
    uint8_t i2c_reg = 0;
    uint8_t i2c_val = 0;
    int ret = -1;

    i2c_bus = (uint8_t )strtoul(argv[1], NULL, 10);
    i2c_addr = (uint16_t )strtoul(argv[2],NULL, 16);
    i2c_reg = (uint8_t )strtoul(argv[3], NULL, 16);
    i2c_val = (uint8_t )strtoul(argv[4], NULL, 16);

    i2c_addr >>= 1;  //make addr as 7bit format

    adap = i2ctools_init_bus(i2c_bus);
    if(!adap){
        printf("i2c_bus=%d error.\n", i2c_bus);
        return CLI_FALSE;
    }

    ret = i2c_read_reg8(adap, i2c_addr, i2c_reg, &i2c_val, 1);
    if(ret < 0){
        printf("i2c_read_reg error. i2c_bus=%d reg=%d\n", i2c_bus, i2c_reg);
    }else{
        printf(" i2c-%d read\ndev_addr(0x%x)\nreg_addr=%x val=%x\n", i2c_bus, i2c_addr<<1, i2c_reg, i2c_val);
    }

    return CLI_FALSE;
}

static int cmd_i2cdump(int argc, char *argv[])
{
    i2c_adap_dev_t *adap = NULL;
    uint8_t i2c_bus = 0;
    uint16_t i2c_addr = 0;
    uint32_t i;
    uint8_t val = 0;
    uint8_t len = 64;
    int ret = -1;

    i2c_bus = (uint8_t )strtoul(argv[1], NULL, 10);
    i2c_addr = (uint16_t )strtoul(argv[2],NULL, 16);
    i2c_addr >>= 1;  //make addr as 7bit format

    adap = i2ctools_init_bus(i2c_bus);
    if(!adap){
        printf("i2c_bus=%d error.\n", i2c_bus);
        return CLI_FALSE;
    }

    printf(" i2c-%d read device addr=0x%x reg[0~%d]\n", i2c_bus, i2c_addr, len);
    for(i = 0; i < len; i++){
        if(i%16 == 0){
            printf("\n");
        }

        if(i%8 == 0){
            printf(" ");
        }

        ret = i2c_read_reg8(adap, i2c_addr, i, &val, 1);
        if(ret < 0){
            printf(" --");
        }else{
            printf(" %.02x", val);
        }
    }

    return CLI_FALSE;
}

static int cmd_i2cdetect(int argc, char *argv[])
{
    i2c_adap_dev_t *adap = NULL;
    uint8_t i2c_bus = 0;
    uint32_t i;
    uint8_t val = 0;
    uint16_t len = 128;
    int ret = -1;

    i2c_bus = (uint8_t )strtoul(argv[1], NULL, 10);

    adap = i2ctools_init_bus(i2c_bus);
    if(!adap){
        printf("i2c_bus=%d error.\n", i2c_bus);
        return CLI_FALSE;
    }

    printf("i2c_bus=%d device detect:\n", i2c_bus);

    for(i = 0; i < len; i++){
        if(i%16 == 0){
            printf("\n");
        }

        if(i%8 == 0){
            printf(" ");
        }

        ret = i2c_read_reg8(adap, i, 0x01, &val, 1);
        if(ret < 0){
            printf(" --");
        }else{
            printf(" %.02x", i<<1);
        }
    }

    return CLI_FALSE;
}

static int _cli_inter_i2cset(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString)
{
    int argc = 0;
    char *argv[CLI_MAX_ARGS];

    argc = CLITokenizeCommand(pcCommandString, argv + 1);
    argv[0] = (char *)pcCommandString;
    argc++;

    return cmd_i2cset(argc, argv);
}

static int _cli_inter_i2cget(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString)
{
    int argc = 0;
    char *argv[CLI_MAX_ARGS];

    argc = CLITokenizeCommand(pcCommandString, argv + 1);
    argv[0] = (char *)pcCommandString;
    argc++;

    return cmd_i2cget(argc, argv);
}

static int _cli_inter_i2cdump(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString)
{
    int argc = 0;
    char *argv[CLI_MAX_ARGS];

    argc = CLITokenizeCommand(pcCommandString, argv + 1);
    argv[0] = (char *)pcCommandString;
    argc++;

    return cmd_i2cdump(argc, argv);
}

static int _cli_inter_i2cdetect(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString)
{
    int argc = 0;
    char *argv[CLI_MAX_ARGS];

    argc = CLITokenizeCommand(pcCommandString, argv + 1);
    argv[0] = (char *)pcCommandString;
    argc++;

    return cmd_i2cdetect(argc, argv);
}

static const char _cli_help_i2cset[] =
                "usage:   i2cset bus_num dev_addr reg_addr val\r\n"
                "example: i2cset 1 0xA0 0x77 0x3f\r\n"
                "    set i2c-1(addr=0xA0 reg_addr=0x77)'s val=0x3F\r\n";

static const char _cli_help_i2cget[] =
                "usage:   i2cget bus_num dev_addr reg_addr\r\n"
                "example: i2cget 1 0xA0 0x77\r\n"
                "     get i2c-1(addr=0xA0 reg_addr=0x77)'s val\r\n";

static const char _cli_help_i2cdump[] =
                "usage:   i2cdump bus_num dev_addr\r\n"
                "example: i2cdump 1 0xA0\r\n"
                "    dump i2c-1(addr=0xA0)'s val\r\n";

static const char _cli_help_i2cdetect[] =
                "usage:   i2cdetect bus_num dev_addr\r\n"
                "example: i2cdetect 1 0xA0\r\n"
                "    detect i2c-1(addr=0xA0)'s all reg_addr val\r\n";

__USED static const CLI_Command_Definition_t _cli_cmd_i2ctest[] __SECTION(".commands") = {
    {"i2cset", _cli_help_i2cset, _cli_inter_i2cset},
    {"i2cget", _cli_help_i2cget, _cli_inter_i2cget},
    {"i2cdump", _cli_help_i2cdump, _cli_inter_i2cdump},
    {"i2cdetect", _cli_help_i2cdetect, _cli_inter_i2cdetect},
};
