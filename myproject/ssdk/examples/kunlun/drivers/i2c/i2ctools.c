#include <compiler.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <types.h>
#include <driver/i2c.h>

#include "CLI_type.h"

static int cmd_i2cset(int argc, char *argv[])
{
    uint32_t i2c_bus = 0;
    uint16_t i2c_addr = 0;
    uint8_t i2c_reg = 0;
    uint8_t i2c_val = 0;
    int ret = -1;

    i2c_bus = strtoul(argv[0], 0, 10);
    i2c_addr = strtoul(argv[1], 0, 16);
    i2c_reg = strtoul(argv[2], 0, 16);
    i2c_val = strtoul(argv[3], 0, 16);

    i2c_adap_dev_t* i2c_handle = i2c_get_adap_dev(i2c_bus);
    if (NULL == i2c_handle){
        printf("%s i2c%d get handle failed\n", __func__, (int)i2c_bus);
        return 0;
    }

    ret = i2c_write_reg8(i2c_handle, i2c_addr, i2c_reg, &i2c_val, 1);
    if(ret < 0){
        printf("\ni2c-%d write reg=%d error\n", i2c_bus, i2c_reg);
    }else{
        printf("\ni2c-%d write device(0x%x) reg=%x val=%x\n",
                i2c_bus, i2c_addr<<1, i2c_reg, i2c_val);
    }

    return 0;
}

static int cmd_i2cget(int argc, char *argv[])
{
    uint32_t i2c_bus = 0;
    uint16_t i2c_addr = 0;
    uint8_t i2c_reg = 0;
    uint8_t i2c_val = 0;
    int ret = -1;

    i2c_bus = strtoul(argv[0], 0, 10);
    i2c_addr = strtoul(argv[1], 0, 16);
    i2c_reg = strtoul(argv[2], 0, 16);

    i2c_adap_dev_t* i2c_handle = i2c_get_adap_dev(i2c_bus);
    if (NULL == i2c_handle){
        printf("%s i2c%d get handle failed\n", __func__, (int)i2c_bus);
        return 0;
    }

    ret = i2c_read_reg8(i2c_handle, i2c_addr, i2c_reg, &i2c_val, 1);
    if(ret < 0) {
        printf("\ni2c-%d read reg=%d error\n", i2c_bus, i2c_reg);
    }
    else {
        printf("\ni2c-%d read device(0x%x) reg=0x%x val=0x%x\n",
                i2c_bus, i2c_addr, i2c_reg, i2c_val);
    }

    return 0;
}

static int cmd_i2cdump(int argc, char *argv[])
{
    uint32_t i2c_bus = 0;
    uint16_t i2c_addr = 0;
    uint8_t i2c_val = 0;
    uint32_t offset = 0;
    uint32_t len = 64;
    uint32_t i;
    int ret = -1;

    i2c_bus = strtoul(argv[0], 0, 10);
    i2c_addr = strtoul(argv[1], 0, 16);
    offset = strtoul(argv[2], 0, 10);
    len = strtoul(argv[3], 0, 10);

    i2c_adap_dev_t* i2c_handle = i2c_get_adap_dev(i2c_bus);
    if (NULL == i2c_handle){
        printf("%s i2c%d get handle failed\n", __func__, (int)i2c_bus);
        return 0;
    }

    printf(" i2c-%d read device addr=0x%x. offset=%d len=%d\n",
            i2c_bus, i2c_addr, offset, len);
    printf("       0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f\n");
    for(i = offset; i < (offset + len); i++) {
        if(i%16 == 0){
            if(i) printf("\n");
            printf("%02x: ", i);
        }

        if(i%8 == 0){
            printf(" ");
        }

        ret = i2c_read_reg8(i2c_handle, i2c_addr, i, &i2c_val, 1);
        if(ret < 0){
            printf(" --");
        }else{
            printf(" %.02x", i2c_val);
        }
    }
    printf("\n");

    return 0;
}

static int cmd_i2cdetect(int argc, char *argv[])
{
    uint32_t i2c_bus = 0;
    uint16_t i2c_addr = 0;
    uint8_t i2c_reg = 0;
    uint8_t i2c_val = 0;
    uint16_t len = 128;
    uint32_t i;
    int ret = -1;

    i2c_bus = strtoul(argv[0], 0, 10);
    i2c_addr >>= 1;  //make addr as 7bit format

    i2c_adap_dev_t* i2c_handle = i2c_get_adap_dev(i2c_bus);
    if(NULL == i2c_handle) {
        printf("%s i2c%d get handle failed\n", __func__, (int)i2c_bus);
        return 0;
    }

    printf("i2c_bus=%d device detect:\n", i2c_bus);
    i2c_reg = 0x01;
    printf("       0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f\n");
    for(i = 0; i < len; i++){
        if(i%16 == 0){
            if(i) printf("\n");
            printf("%02x: ", i);
        }

        if(i%8 == 0){
            printf(" ");
        }

        ret = i2c_read_reg8(i2c_handle, i, i2c_reg, &i2c_val, 1);
        if(ret < 0){
            printf(" --");
        }else{
            printf(" %.02x", i);
        }
    }
    printf("\n");

    return 0;
}

#define I2CSET_STR "i2cset bus addr(7bit) reg val." \
    "\r\n\texample: i2cset 4 0xA0 0x01 0xAF" \
    "\r\n\t---set i2c-4's A0 device's 01 reg = 0xAF"

#define I2CGET_STR "i2cget bus addr(7bit) reg." \
    "\r\n\texample: i2cget 4 0xA0 0x01" \
    "\r\n\t---get i2c-4's A0 device's 01 reg"

 #define I2CDUMP_STR "i2cdump bus addr(7bit) off len." \
    "\r\n\texample: i2cdump 4 0xA0 0 128" \
    "\r\n\t---dump i2c-4's A0 device's 01~128 reg"

 #define I2CDETECT_STR "i2cdetect bus" \
    "\r\n\tdetect i2c-4's devices."

CLI_CMD("i2cdetect", I2CDETECT_STR, cmd_i2cdetect);
CLI_CMD("i2cget", I2CGET_STR, cmd_i2cget);
CLI_CMD("i2cset", I2CSET_STR, cmd_i2cset);
CLI_CMD("i2cdump", I2CDUMP_STR, cmd_i2cdump);
