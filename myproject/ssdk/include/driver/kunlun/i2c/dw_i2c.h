/*
 * dw_i2c.h
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: I2C driver header.
 *
 * Revision History:
 * ----------------------------------------------------------------
 */
#ifndef __DW_I2C_H__
#define __DW_I2C_H__

#include <driver/i2c.h>
#include <types.h>

#define DW_I2C_RETRY 3
#define DW_I2C_EVENT_TIMEOUT 3000 //ms
#define DW_I2C_STUCK_TIMEOUT 1 //s
#define DW_I2C_EVENT_COMPLETE 0x1u

#define DW_I2C_EPARAM    1
#define DW_I2C_EBUSY     2
#define DW_I2C_ETIMEOUT  3
#define DW_I2C_ENOACK    4
#define DW_I2C_ESCLSTUCK 5
#define DW_I2C_ESDASTUCK 6
#define DW_I2C_EARBLOST  7
#define DW_I2C_EIO       8
#define DW_I2C_EMSG      9
#define DW_I2C_EINVAL    10

/* Registers offset */
#define DW_IC_CON               0x0
#define DW_IC_TAR               0x4
#define DW_IC_SAR               0x8
#define DW_IC_DATA_CMD          0x10
#define DW_IC_SS_SCL_HCNT       0x14
#define DW_IC_SS_SCL_LCNT       0x18
#define DW_IC_FS_SCL_HCNT       0x1c
#define DW_IC_FS_SCL_LCNT       0x20
#define DW_IC_HS_SCL_HCNT       0x24
#define DW_IC_HS_SCL_LCNT       0x28
#define DW_IC_INTR_STAT         0x2c
#define DW_IC_INTR_MASK         0x30
#define DW_IC_RAW_INTR_STAT     0x34
#define DW_IC_RX_TL             0x38
#define DW_IC_TX_TL             0x3c
#define DW_IC_CLR_INTR          0x40
#define DW_IC_CLR_RX_UNDER      0x44
#define DW_IC_CLR_RX_OVER       0x48
#define DW_IC_CLR_TX_OVER       0x4c
#define DW_IC_CLR_RD_REQ        0x50
#define DW_IC_CLR_TX_ABRT       0x54
#define DW_IC_CLR_RX_DONE       0x58
#define DW_IC_CLR_ACTIVITY      0x5c
#define DW_IC_CLR_STOP_DET      0x60
#define DW_IC_CLR_START_DET     0x64
#define DW_IC_CLR_GEN_CALL      0x68
#define DW_IC_ENABLE            0x6c
#define DW_IC_STATUS            0x70
#define DW_IC_TXFLR             0x74
#define DW_IC_RXFLR             0x78
#define DW_IC_SDA_HOLD          0x7c
#define DW_IC_TX_ABRT_SOURCE    0x80
#define DW_IC_ENABLE_STATUS     0x9c
#define DW_IC_CLR_RESTART_DET   0xa8
#define DW_IC_SCL_STUCK_TIMEOUT 0xac
#define DW_IC_SDA_STUCK_TIMEOUT 0xb0
#define DW_IC_CLR_SCL_STUCK_DET 0xb4
#define DW_IC_COMP_PARAM_1      0xf4
#define DW_IC_COMP_VERSION      0xf8
#define DW_IC_SDA_HOLD_MIN_VERS 0x3131312A
#define DW_IC_COMP_TYPE         0xfc
#define DW_IC_COMP_TYPE_VALUE   0x44570140

#define DW_IC_CON_MASTER                 0x1
#define DW_IC_CON_SPEED_STD              0x2
#define DW_IC_CON_SPEED_FAST             0x4
#define DW_IC_CON_SPEED_HIGH             0x6
#define DW_IC_CON_SPEED_MASK             0x6
#define DW_IC_CON_10BITADDR_SLAVE        0x8
#define DW_IC_CON_10BITADDR_MASTER       0x10
#define DW_IC_CON_RESTART_EN             0x20
#define DW_IC_CON_SLAVE_DISABLE          0x40
#define DW_IC_CON_STOP_DET_IFADDRESSED   0x80
#define DW_IC_CON_TX_EMPTY_CTRL          0x100
#define DW_IC_CON_RX_FIFO_FULL_HLD_CTRL  0x200
#define DW_IC_CON_BUS_CLEAR_FEATURE_CTRL 0x800

#define DW_IC_INTR_RX_UNDER     0x001
#define DW_IC_INTR_RX_OVER      0x002
#define DW_IC_INTR_RX_FULL      0x004
#define DW_IC_INTR_TX_OVER      0x008
#define DW_IC_INTR_TX_EMPTY     0x010
#define DW_IC_INTR_RD_REQ       0x020
#define DW_IC_INTR_TX_ABRT      0x040
#define DW_IC_INTR_RX_DONE      0x080
#define DW_IC_INTR_ACTIVITY     0x100
#define DW_IC_INTR_STOP_DET     0x200
#define DW_IC_INTR_START_DET    0x400
#define DW_IC_INTR_GEN_CALL     0x800
#define DW_IC_INTR_RESTART_DET  0x1000
#define DW_IC_INTR_SCL_STUCK    0x4000

#define DW_IC_INTR_DEFAULT_MASK     (DW_IC_INTR_RX_FULL | \
                     DW_IC_INTR_TX_ABRT | \
                     DW_IC_INTR_STOP_DET)
#define DW_IC_INTR_MASTER_MASK      (DW_IC_INTR_DEFAULT_MASK | \
                     DW_IC_INTR_TX_EMPTY | \
                     DW_IC_INTR_SCL_STUCK)

/* status codes */
#define DW_STATUS_IDLE          0x0
#define DW_STATUS_W_IN_PROGRESS 0x1
#define DW_STATUS_R_IN_PROGRESS 0x2

/*
 * Hardware abort codes from the DW_IC_TX_ABRT_SOURCE register
 *
 * Only expected abort codes are listed here
 * refer to the datasheet for the full list
 */
#define ABRT_7B_ADDR_NOACK      0
#define ABRT_10ADDR1_NOACK      1
#define ABRT_10ADDR2_NOACK      2
#define ABRT_TXDATA_NOACK       3
#define ABRT_GCALL_NOACK        4
#define ABRT_GCALL_READ         5
#define ABRT_SBYTE_ACKDET       7
#define ABRT_SBYTE_NORSTRT      9
#define ABRT_10B_RD_NORSTRT     10
#define ABRT_MASTER_DIS         11
#define ARB_LOST                12
#define ABRT_SLAVE_FLUSH_TXFIFO 13
#define ABRT_SLAVE_ARBLOST      14
#define ABRT_SLAVE_RD_INTX      15
#define ABRT_SDA_STUCK_AT_LOW   17

#define DW_IC_TX_ABRT_7B_ADDR_NOACK (1UL << ABRT_7B_ADDR_NOACK)
#define DW_IC_TX_ABRT_10ADDR1_NOACK (1UL << ABRT_10ADDR1_NOACK)
#define DW_IC_TX_ABRT_10ADDR2_NOACK (1UL << ABRT_10ADDR2_NOACK)
#define DW_IC_TX_ABRT_TXDATA_NOACK  (1UL << ABRT_TXDATA_NOACK)
#define DW_IC_TX_ABRT_GCALL_NOACK   (1UL << ABRT_GCALL_NOACK)
#define DW_IC_TX_ABRT_GCALL_READ    (1UL << ABRT_GCALL_READ)
#define DW_IC_TX_ABRT_SBYTE_ACKDET  (1UL << ABRT_SBYTE_ACKDET)
#define DW_IC_TX_ABRT_SBYTE_NORSTRT (1UL << ABRT_SBYTE_NORSTRT)
#define DW_IC_TX_ABRT_10B_RD_NORSTRT    (1UL << ABRT_10B_RD_NORSTRT)
#define DW_IC_TX_ABRT_MASTER_DIS    (1UL << ABRT_MASTER_DIS)
#define DW_IC_TX_ARB_LOST           (1UL << ARB_LOST)
#define DW_IC_RX_ABRT_SLAVE_RD_INTX (1UL << ABRT_SLAVE_RD_INTX)
#define DW_IC_RX_ABRT_SLAVE_ARBLOST (1UL << ABRT_SLAVE_ARBLOST)
#define DW_IC_RX_ABRT_SLAVE_FLUSH_TXFIFO    (1UL << ABRT_SLAVE_FLUSH_TXFIFO)
#define DW_IC_ABRT_SDA_STUCK_AT_LOW (1UL << ABRT_SDA_STUCK_AT_LOW)

#define DW_IC_TX_ABRT_NOACK     (DW_IC_TX_ABRT_7B_ADDR_NOACK | \
                     DW_IC_TX_ABRT_10ADDR1_NOACK | \
                     DW_IC_TX_ABRT_10ADDR2_NOACK | \
                     DW_IC_TX_ABRT_TXDATA_NOACK | \
                     DW_IC_TX_ABRT_GCALL_NOACK)

#define DW_IC_SDA_HOLD_RX_SHIFT     16
#define DW_IC_STATUS_ACTIVITY       0x1
#define DW_IC_ERR_TX_ABRT   0x1
#define DW_IC_ERR_SCL_STUCK 0x2

typedef struct {
    uint32_t master_cfg;
    uint32_t sda_hold_time;
    uint32_t scl_timeout;
    uint32_t sda_timeout;
    uint16_t ss_hcnt;
    uint16_t ss_lcnt;
    uint16_t fs_hcnt;
    uint16_t fs_lcnt;
    uint32_t tx_fifo_depth;
    uint32_t rx_fifo_depth;
    osEventFlagsId_t event;
    int msgs_num;
    struct i2c_msg *msgs;
    int msg_err;
    int cmd_err;
    int scl_stuck_err;
    int sda_stuck_err;
    uint32_t abort_source;
    int msg_write_idx;
    uint32_t tx_buf_len;
    uint8_t *tx_buf;
    int msg_read_idx;
    uint32_t rx_buf_len;
    uint8_t *rx_buf;
    uint32_t status;
    int rx_outstanding;
} dw_i2c_context;

extern void dw_i2c_adap_probe(i2c_adap_dev_t *dev, i2c_adap_cfg_t *cfg,
                              dw_i2c_context *i2c_cxt, int id);

#endif //__DW_I2C_H__
