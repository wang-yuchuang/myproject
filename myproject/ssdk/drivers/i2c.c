/**
 * @file i2c.c
 * @common i2c interface file
 *
 * @Copyright (c) 2020  Semidrive Semiconductor.
 * @All rights reserved.
 *
 **/

#include <types.h>
#include <debug.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <driver/irq.h>

#include "driver/i2c.h"
#include <sd_errcode.h>

#ifndef CONFIG_PART_I2C_NUM
#define CONFIG_PART_I2C_NUM 32
#endif

static i2c_adap_dev_t *i2c_adap_register_table[CONFIG_PART_I2C_NUM];

int i2c_transfer(i2c_adap_dev_t *adap, struct i2c_msg *msgs, int num)
{
    int ret = 0, try = 0;

    osStatus_t stat = osMutexAcquire(adap->lock, osWaitForever);
    if (stat == osErrorISR && adap->busy) {
        //printf("i2c%u call err in irq\n", adap->id);
        ret = -ECANCELED;
        return ret;
    }

    adap->busy = true;

    if (!adap->ops->i2c_xfer || adap->status < 0) {
        printf("i2c%u adapter offline\n", adap->id);
        ret = -EOPNOTSUPP;
        ERRCODE_PRINT(MOD_ID_I2C, adap->id, EI2C_XFER, 0);
    }
    else {
        do {
            ret = adap->ops->i2c_xfer(adap, msgs, num);
            //ZQD808更改，解决连续操作eeprom失败问题
            #if(1)
            if(ret == -4)
            {
                osDelay(10);  // EEPROM可能正在写入，等待更长时间
            }
            #endif
            if (!ret)
                break;
        } while (++try < adap->retry);
    }

    adap->busy = false;
    osMutexRelease(adap->lock);

    return ret;
}

/*
 * [S] + [addr+w] + [wbuf1...wbufN] + [P]
 * easy wrapper api support for common write
*/
int i2c_write(i2c_adap_dev_t *adap, uint16_t addr,
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
        },
    };
    ret = i2c_transfer(adap, msgs, 1);
    return ret;
}

/*
 * [S] + [addr+r] + [rbuf1...rbufN] + [P]
 * easy wrapper api support for common read
*/
int i2c_read(i2c_adap_dev_t *adap, uint16_t addr,
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
        },
    };
    ret = i2c_transfer(adap, msgs, 1);
    return ret;
}

/*
 * [S] + [addr+w] + [wbuf1...wbufN] +
 * [Sr] + [addr+r] + [rbuf1...rbufN] + [P]
 * easy wrapper api support for common write then read
*/
int i2c_write_read(i2c_adap_dev_t *adap, uint16_t addr,
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
        },
        {
            .addr = addr,
            .buf = rbuf,
            .len = rlen,
            .addr_flag = I2C_ADDR7B,
            .rw_flag = I2C_M_R,
        },
    };
    ret = i2c_transfer(adap, msgs, 2);
    return ret;
}

/*
 * [S] + [addr+w] + [reg8] + [data1...dataN] + [P]
 * easy wrapper api support only data len < 64 bytes
*/
int i2c_write_reg8(i2c_adap_dev_t *adap, uint16_t addr, uint8_t reg,
                   uint8_t *data, int data_len)
{
    uint8_t buf[1 + 64] = {reg};

    if (data_len > 64)
        return -ENOBUFS;

    for (int i = 0; i < data_len; i++)
        buf[i + 1] = data[i];

    return i2c_write(adap, addr, (const uint8_t *)buf, 1 + data_len);
}

/*
 * [S] + [addr+w] + [reg8] +
 * [Sr] + [addr+r] + [data1...dataN] + [P]
 * easy wrapper api support only data len < 64 bytes
*/
int i2c_read_reg8(i2c_adap_dev_t *adap, uint16_t addr, uint8_t reg,
                  uint8_t *data, int data_len)
{
    if (data_len > 64)
        return -ENOBUFS;

    return i2c_write_read(adap, addr, &reg, 1, data, data_len);
}

/*
 * [S] + [addr+w] + [reg16_high_byte]+ [reg16_low_byte] + [data1...dataN] + [P]
 * easy wrapper api support only data len < 64 bytes
*/
int i2c_write_reg16(i2c_adap_dev_t *adap, uint16_t addr, uint16_t reg,
                    uint8_t *data, int data_len)
{
    uint8_t buf[2 + 64] = {reg >> 8, reg & 0xff};

    if (data_len > 64)
        return -ENOBUFS;

    for (int i = 0; i < data_len; i++)
        buf[i + 2] = data[i];

    return i2c_write(adap, addr, (const uint8_t *)buf, 2 + data_len);
}

/*
 * [S] + [addr+w] + [reg16_high_byte] + [reg16_low_byte]
 * [Sr] + [addr+r] + [data1...dataN] + [P]
 * easy wrapper api support only data len < 64 bytes
*/
int i2c_read_reg16(i2c_adap_dev_t *adap, uint16_t addr, uint16_t reg,
                   uint8_t *data, int data_len)
{
    uint8_t reg_buf[2] = {reg >> 8, reg & 0xff};

    if (data_len > 64)
        return -ENOBUFS;

    return i2c_write_read(adap, addr, reg_buf, 2, data, data_len);
}

/*
 * [S] + [addr+w] + [reg16_low_byte]+ [reg16_high_byte] + [data1...dataN] + [P]
 * easy wrapper api support only data len < 64 bytes
*/
int i2c_write_reg16s(i2c_adap_dev_t *adap, uint16_t addr, uint16_t reg,
                     uint8_t *data, int data_len)
{
    uint8_t buf[2 + 64] = {reg & 0xff, reg >> 8};

    if (data_len > 64)
        return -ENOBUFS;

    for (int i = 0; i < data_len; i++)
        buf[i + 2] = data[i];

    return i2c_write(adap, addr, (const uint8_t *)buf, 2 + data_len);
}

/*
 * [S] + [addr+w] + [reg16_low_byte] + [reg16_high_byte]
 * [Sr] + [addr+r] + [data1...dataN] + [P]
 * easy wrapper api support only data len < 64 bytes
*/
int i2c_read_reg16s(i2c_adap_dev_t *adap, uint16_t addr, uint16_t reg,
                    uint8_t *data, int data_len)
{
    uint8_t reg_buf[2] = {reg & 0xff, reg >> 8};

    if (data_len > 64)
        return -ENOBUFS;

    return i2c_write_read(adap, addr, reg_buf, 2, data, data_len);
}

i2c_adap_dev_t *i2c_get_adap_dev(int id)
{
    for (int i = 0; i < CONFIG_PART_I2C_NUM; i++) {
        if (i2c_adap_register_table[i] && i2c_adap_register_table[i]->id == id)
            return i2c_adap_register_table[i];
    }

    return NULL;
}

void i2c_adap_earlyinit(i2c_adap_dev_t *adap, i2c_adap_cfg_t *cfg)
{
    adap->base = cfg->base;
    adap->irq = cfg->irq;
    adap->clk = cfg->clk;
    adap->enable = cfg->enable;
    adap->opmode = cfg->opmode;
    adap->speed = cfg->speed;
}

void i2c_adap_init(i2c_adap_dev_t *adap)
{
    adap->lock = osMutexNew(NULL);

    if (!adap->enable) {
        adap->status = -1;
        printf("i2c%u not enable\n", adap->id);
        ERRCODE_PRINT(MOD_ID_I2C, adap->id, EI2C_INSTANCE, 0);
        return;
    }

    if (adap->ops->i2c_init) {
        adap->status = adap->ops->i2c_init(adap);

        if (adap->status) {
            ssdk_printf(SSDK_ERR, "%s-%u init failed\n", __func__, adap->id);
        }
        else {
            for (int i = 0; i < CONFIG_PART_I2C_NUM; i++) {
                if (!i2c_adap_register_table[i]) {
                    i2c_adap_register_table[i] = adap;
                    break;
                }
            }
        }
    }
    else {
        printf("i2c%u not implement\n", adap->id);
        adap->status = -1;
        ERRCODE_PRINT(MOD_ID_I2C, adap->id, EI2C_INSTANCE, 0);
    }
}

