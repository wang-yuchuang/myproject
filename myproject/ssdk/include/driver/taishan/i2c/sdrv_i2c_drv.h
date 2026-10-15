/**
 * @file sdrv_i2c_drv.h
 * @brief sdrv i2c driver header file.
 *
 * @Copyright (c) 2020  Semidrive Semiconductor.
 * @All rights reserved.
 *
 */

#ifndef SDRV_I2C_DRV_H_
#define SDRV_I2C_DRV_H_

#include "driver/i2c.h"

#define SDRV_I2C_FUSA

#define SDRV_I2C_RETRY 3
#define SDRV_I2C_EVENT_COMPLETE 0x1u
#define SDRV_I2C_TIMEOUT 3000
#define SDRV_I2C_MSG_IDLE 0
#define SDRV_I2C_MSG_RW 1
#define SDRV_I2C_SLV_ADDR 0x34

#define min(a, b)   (a) < (b) ? a : b

#define CAL_BASE(value, clk_khz)     (value * clk_khz / 1000000)
#define CAL_SCLL(value, clk_khz)     (CAL_BASE(value, clk_khz))
#define CAL_TA(value, clk_khz)       (CAL_BASE(value, clk_khz))
#define CAL_DVDAT(value, clk_khz)    (CAL_BASE(value, clk_khz) / 2 - 1)
#define CAL_UNIT(value)              (value)
#define CAL_MEXT(value, clk_khz)     (CAL_BASE(value, clk_khz))
#define CAL_SEXT(value, clk_khz)     (CAL_BASE(value, clk_khz))
#define CAL_STUCK(value, clk_khz)    (value * clk_khz / 65536)
#define CAL_SDARXHLD(value, clk_khz) (CAL_BASE(value, clk_khz))
#define CAL_DATSPL(value)            (value)
#define CAL_BUF(value, clk_khz)      (log2(CAL_BASE(value, clk_khz) / 4))
#define CAL_IDLE(value, clk_khz)     (CAL_BASE(value, clk_khz))

typedef struct sdrv_i2c_timing_cfg {
    uint32_t timscll;
    uint32_t timtasclh;
    uint32_t timtascll;
    uint32_t timtasdal;
    uint32_t timdvdat;
    uint32_t timscll_h;
    uint32_t timtasclh_h;
    uint32_t timtascll_h;
    uint32_t timtasdal_h;
    uint32_t timdvdat_h;
    uint32_t timunit;
    uint32_t timmext;
    uint32_t timsext;
    uint32_t timstuck;
    uint32_t timsdarxhld;
    uint32_t timdatspl;
    uint32_t timbuf;
    uint32_t timidle;
} sdrv_i2c_timing_cfg_t;

typedef struct sdrv_i2c_timing {
    uint32_t divs;
    uint32_t pcr2;
    uint32_t pcr3;
    uint32_t pcr4;
    uint32_t pcr5;
} sdrv_i2c_timing_t;

typedef struct sdrv_i2c_mdev {
    uint32_t timeout;
    osEventFlagsId_t event;
    int status;
    uint32_t int0_stat;
    struct i2c_msg *msgs;
    int num;
    struct i2c_msg *cur_msg;
    int cur_msg_idx;
    int cur_msg_stat;
    int cur_cmd_done;
    uint16_t cur_msg_len;
} sdrv_i2c_mdev_t;

typedef struct sdrv_i2c_sdev {
    bool ctlbyte_det;
    enum i2c_slv_event rw_event;
    bool first_read_req;
    uint32_t rw_req_len;
    uint8_t slv_buf_pos;
    uint8_t slv_buf[255];
} sdrv_i2c_sdev_t;

typedef union sdrv_i2c_dev_type {
    sdrv_i2c_mdev_t mdev;
    sdrv_i2c_sdev_t sdev;
} sdrv_i2c_dev_type_t;

typedef struct sdrv_i2c_dev {
    i2c_adap_dev_t adap;
    sdrv_i2c_dev_type_t dev_type;
} sdrv_i2c_dev_t;

i2c_adap_dev_t *sdrv_i2c_adap_probe(sdrv_i2c_dev_t *dev, i2c_adap_cfg_t *cfg, int id);

#endif
