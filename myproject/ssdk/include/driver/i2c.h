/**
 * @file i2c.h
 * @Common i2c driver header.
 *
 * @Copyright (c) 2020  Semidrive Semiconductor.
 * @All rights reserved.
 *
 **/

#ifndef I2C_UAPI_H_
#define I2C_UAPI_H_

#include <cmsis_os2.h>

enum i2c_slv_event {
    I2C_SLV_RW_NONE,
    I2C_SLV_RD_REQUEST,
    I2C_SLV_WR_RECEIVE,
};

enum i2c_speed {
    I2C_SSPEED,
    I2C_FSPEED,
    I2C_PSPEED,
    I2C_HSPEED,
    I2C_USPEED,
};

enum i2c_opmode {
    I2C_MASTER,
    I2C_SLAVE,
    SMB_MASTER,
    SMB_SLAVE,
    PM_MASTER,
    PM_SLAVE,
};

typedef struct i2c_adap_cfg i2c_adap_cfg_t;
typedef struct i2c_adap_dev i2c_adap_dev_t;
typedef struct i2c_adap_ops i2c_adap_ops_t;
typedef int (*i2c_slv_cb_t)(i2c_adap_dev_t *adap,
    enum i2c_slv_event slv_event, uint8_t *data, uint16_t len);

struct i2c_msg {
    uint16_t addr;     /* slave address       */
    uint16_t len;      /* msg length          */
    uint8_t *buf;      /* pointer to msg data */
#define I2C_ADDR7B  0x0
#define I2C_ADDR10B 0x1
#define I2C_M_W 0x0
#define I2C_M_R 0x1
    uint8_t addr_flag; /* I2C_ADDR7 / I2C_ADDR10 */
    uint32_t rw_flag;  /* I2C_M_W / I2C_M_R */
    uint8_t polling;   /* true for polling for special scene, otherwise ignore */
};

typedef struct i2c_adap_cfg {
    paddr_t base;
    uint32_t irq;
    uint32_t clk;
    bool enable;
    enum i2c_opmode opmode;
    enum i2c_speed speed;
} i2c_adap_cfg_t;

typedef struct i2c_adap_ops {
    int (*i2c_init)(i2c_adap_dev_t *adap);
    int (*i2c_xfer)(i2c_adap_dev_t *adap, struct i2c_msg *msgs, int num);
} i2c_adap_ops_t;

typedef struct i2c_adap_dev {
    paddr_t base;
    uint32_t irq;
    uint32_t clk;
    bool enable;
    enum i2c_opmode opmode;
    enum i2c_speed speed;
    int id;
    int retry;
    i2c_adap_ops_t *ops;
    osMutexId_t lock;
    bool busy;
    int status;
    void *priv;
    i2c_slv_cb_t slv_cb;
} i2c_adap_dev_t;

int i2c_transfer(i2c_adap_dev_t *adap, struct i2c_msg *msgs, int num);
int i2c_write(i2c_adap_dev_t *adap, uint16_t addr, const uint8_t *wbuf, int wlen);
int i2c_read(i2c_adap_dev_t *adap, uint16_t addr, uint8_t *rbuf, int rlen);
int i2c_write_read(i2c_adap_dev_t *adap, uint16_t addr,
                    const uint8_t *wbuf, int wlen,  uint8_t *rbuf, int rlen);
int i2c_write_reg8(i2c_adap_dev_t *adap, uint16_t addr, uint8_t reg, uint8_t *data, int data_len);
int i2c_read_reg8(i2c_adap_dev_t *adap, uint16_t addr, uint8_t reg, uint8_t *data, int data_len);
int i2c_write_reg16(i2c_adap_dev_t *adap, uint16_t addr, uint16_t reg, uint8_t *data, int data_len);
int i2c_read_reg16(i2c_adap_dev_t *adap, uint16_t addr, uint16_t reg, uint8_t *data, int data_len);
int i2c_write_reg16s(i2c_adap_dev_t *adap, uint16_t addr, uint16_t reg, uint8_t *data, int data_len);
int i2c_read_reg16s(i2c_adap_dev_t *adap, uint16_t addr, uint16_t reg, uint8_t *data, int data_len);
int i2c_register_slave(i2c_adap_dev_t *adap, i2c_slv_cb_t slv_cb);
i2c_adap_dev_t *i2c_get_adap_dev(int id);
void i2c_adap_earlyinit(i2c_adap_dev_t *adap, i2c_adap_cfg_t *cfg);
void i2c_adap_init(i2c_adap_dev_t *adap);

#endif
