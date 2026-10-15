/**
 * @file sdrv_i2s_mc.h
 * @sdrv i2s mc header file.
 *
 * @Copyright (c) 2022  Semidrive Semiconductor.
 * @All rights reserved.
 *
 **/
#ifndef _SDRV_I2S_MC_H_
#define _SDRV_I2S_MC_H_

#include <stdbool.h>
#include <types.h>

#include "driver/snd_driver.h"
#include "bits.h"

#ifndef _BITS_MASK_
#define _BITS_MASK_(h_bit, l_bit)                                              \
    (((~0UL) - (1UL << (l_bit)) + 1) & (~0UL >> (32 - 1 - (h_bit))))
#endif

#ifndef VALUE_MASK
#define VALUE_MASK(val, shift) ((val) << (shift))
#endif

#ifndef _BIT_
#define _BIT_(n) (1 << (n))
#endif

#define I2S_MC_SAMPLE_RATE_CALC(fclk, fsample, chn_width)                      \
    (((fclk) + (fsample) * (chn_width)) / (2 * (fsample) * (chn_width)))

#define I2S_MC_SAMPLE_RESOLUTION_TO_CONFIG(bit_format) ((bit_format)-1)

#define I2S_MC_FIFO_OFFSET 0x3c

#define I2S_MC_TX_DEPTH 512
#define I2S_MC_RX_DEPTH 512

#define I2S_MC_TFIFO_TAEMPTY_THRESHOLD (I2S_MC_TX_DEPTH - 128)
#define I2S_MC_TFIFO_TAFULL_THRESHOLD (I2S_MC_TX_DEPTH - 1)
#define I2S_MC_RFIFO_RAEMPTY_THRESHOLD (0)
#define I2S_MC_RFIFO_RAFULL_THRESHOLD (I2S_MC_RX_DEPTH - 32)

#define I2S_MC_DATA_WIDTH 32

typedef enum {
    CLK_SYNC_NO_LOOP = 0,
    CLK_SYNC_TX_LOOP = 1,
    CLK_SYNC_RX_LOOP = 2,
} sdrv_clk_sync_loop_mode_t;

typedef enum {
    STREAM_STATUS_STOP = 0,
    STREAM_STATUS_START = 1,
} sdrv_stream_status_t;

/* I2S_MC REGS I2S_CTRL */
#define MC_I2S_CTRL (0x000)
#define CTRL_EN_SHIFT 0
#define CTRL_TR_CFG_SHIFT 8
#define CTRL_LOOP_BACK_0_1_SHIFT 16
#define CTRL_LOOP_BACK_2_3_SHIFT 17
#define CTRL_LOOP_BACK_4_5_SHIFT 18
#define CTRL_LOOP_BACK_7_8_SHIFT 19
#define CTRL_SFR_RST_SHIFT 20
#define CTRL_T_MS_SHIFT 21
#define CTRL_R_MS_SHIFT 22
#define CTRL_TFIFO_RST_SHIFT 23
#define CTRL_RFIFO_RST_SHIFT 24
#define CTRL_TSYNC_RST_SHIFT 25
#define CTRL_RSYNC_RST_SHIFT 26
#define CTRL_TSYNC_LOOP_BACK_SHIFT 27
#define CTRL_RSYNC_LOOP_BACK_SHIFT 28

#define CTRL_EN_MASK _BITS_MASK_(7, 0)
#define CTRL_TR_CFG_MASK _BITS_MASK_(15, 8)

#define CTRL_SFR_RST_MASK _BIT_(20)
#define CTRL_T_MS_MASK _BIT_(21)
#define CTRL_R_MS_MASK _BIT_(22)
#define CTRL_TFIFO_RST_MASK _BIT_(23)
#define CTRL_RFIFO_RST_MASK _BIT_(24)
#define CTRL_TSYNC_RST_MASK _BIT_(25)
#define CTRL_RSYNC_RST_MASK _BIT_(26)
#define CTRL_TSYNC_LOOP_BACK_MASK _BIT_(27)
#define CTRL_RSYNC_LOOP_BACK_MASK _BIT_(28)

/* I2S_MC REGS I2S_INTR_STAT */
#define MC_I2S_INTR_STAT (0x004)
#define INTR_TDATA_UNDERR_SHIFT 0
/* The code is a binary notation of the channels's number. 1~3 */
#define INTR_UNDERR_CODE_SHIFT 1
#define INTR_RDATA_OVRERR_SHIFT 4
/* The code is a binary notation of the channels's number. 5~7 */
#define INTR_OVRERR_CODE_SHIFT 5
#define INTR_TFIFO_EMPTY_SHIFT 8
#define INTR_TFIFO_AEMPTY_SHIFT 9
#define INTR_TFIFO_FULL_SHIFT 10
#define INTR_TFIFO_AFULL_SHIFT 11
#define INTR_RFIFO_EMPTY_SHIFT 12
#define INTR_RFIFO_AEMPTY_SHIFT 13
#define INTR_RFIFO_FULL_SHIFT 14
#define INTR_RFIFO_AFULL_SHIFT 15

#define INTR_TDATA_UNDERR_MASK _BIT_(0)
/* The code is a binary notation of the channels's number. 1~3 */
#define INTR_UNDERR_CODE_MASK _BITS_MASK_(3, 1)
#define INTR_RDATA_OVRERR_MASK _BIT_(4)
/* The code is a binary notation of the channels's number. 5~7 */
#define INTR_OVRERR_CODE_MASK _BITS_MASK_(7, 5)
#define INTR_TFIFO_EMPTY_MASK _BIT_(8)
#define INTR_TFIFO_AEMPTY_MASK _BIT_(9)
#define INTR_TFIFO_FULL_MASK _BIT_(10)
#define INTR_TFIFO_AFULL_MASK _BIT_(11)
#define INTR_RFIFO_EMPTY_MASK _BIT_(12)
#define INTR_RFIFO_AEMPTY_MASK _BIT_(13)
#define INTR_RFIFO_FULL_MASK _BIT_(14)
#define INTR_RFIFO_AFULL_MASK _BIT_(15)

/* I2S_MC REGS I2S_SRR */
#define MC_I2S_SRR (0x008)
#define TSAMPLE_RATE_SHIFT 0
#define TRESOLUTION_SHIFT 11
#define RSAMPLE_RATE_SHIFT 16
#define RRESOLUTION_SHIFT 27

#define TSAMPLE_RATE_MASK _BITS_MASK_(10, 0)
#define TRESOLUTION_MASK _BITS_MASK_(15, 11)
#define RSAMPLE_RATE_MASK _BITS_MASK_(26, 16)
#define RRESOLUTION_MASK _BITS_MASK_(31, 27)

/* I2S_MC REGS CID_CTRL */
#define MC_I2S_CID_CTRL (0x00c)
/* i2s clock strobe from 0 to 7 */
#define CLK_STROBE_SHIFT 0
#define STROBE_TS_SHIFT 8
#define STROBE_RS_SHIFT 9
#define INTREQ_MASK_SHIFT 15
#define I2S_MASK_SHIFT 16
#define TFIFO_EMPTY_MASK_SHIFT 24
#define TFIFO_AEMPTY_MASK_SHIFT 25
#define TFIFO_FULL_MASK_SHIFT 26
#define TFIFO_AFULL_MASK_SHIFT 27
#define RFIFO_EMPTY_MASK_SHIFT 28
#define RFIFO_AEMPTY_MASK_SHIFT 29
#define RFIFO_FULL_MASK_SHIFT 30
#define RFIFO_AFULL_MASK_SHIFT 31

#define CLK_STROBE_MASK _BITS_MASK_(7, 0)
#define STROBE_TS_MASK _BIT_(8)
#define STROBE_RS_MASK _BIT_(9)
#define INTREQ_MASK_MASK _BIT_(15)
/* Bit masking for interrupt request generation after underrun /overrun. from 0
 * to 7 */
#define I2S_MASK_MASK _BITS_MASK_(23, 16)
#define TFIFO_EMPTY_MASK_MASK _BIT_(24)
#define TFIFO_AEMPTY_MASK_MASK _BIT_(25)
#define TFIFO_FULL_MASK_MASK _BIT_(26)
#define TFIFO_AFULL_MASK_MASK _BIT_(27)
#define RFIFO_EMPTY_MASK_MASK _BIT_(28)
#define RFIFO_AEMPTY_MASK_MASK _BIT_(29)
#define RFIFO_FULL_MASK_MASK _BIT_(30)
#define RFIFO_AFULL_MASK_MASK _BIT_(31)

/* I2S_MC REGS TFIFO_STAT */
#define MC_I2S_TFIFO_STAT (0x010)
#define I2S_TFIFO_SHIFT 0
#define I2S_TFIFO_MASK _BITS_MASK_(8, 0)

/* I2S_MC REGS RFIFO_STAT */
#define MC_I2S_RFIFO_STAT (0x014)
#define I2S_RFIFO_SHIFT 0
#define I2S_RFIFO_MASK _BITS_MASK_(8, 0)

/* I2S_MC REGS TFIFO_CTRL */
#define MC_I2S_TFIFO_CTRL (0x018)
#define TAEMPTY_THRESHOLD_SHIFT 0
#define TAFULL_THRESHOLD_SHIFT 16
#define TAEMPTY_THRESHOLD_MASK _BITS_MASK_(7, 0)
#define TAFULL_THRESHOLD_MASK _BITS_MASK_(23, 16)

/* I2S_MC REGS RFIFO_CTRL */
#define MC_I2S_RFIFO_CTRL (0x01c)
#define RAEMPTY_THRESHOLD_SHIFT 0
#define RAFULL_THRESHOLD_SHIFT 16
#define RAEMPTY_THRESHOLD_MASK _BITS_MASK_(7, 0)
#define RAFULL_THRESHOLD_MASK _BITS_MASK_(23, 16)

/* I2S_MC REGS DEV_CONF */
#define MC_I2S_DEV_CONF (0x020)
#define TRAN_SCK_POLAR_SHIFT 0
#define TRAN_WS_POLAR_SHIFT 1
#define TRAN_APB_ALIGN_LR_SHIFT 2
#define TRAN_I2S_ALIGN_LR_SHIFT 3
#define TRAN_DATA_WS_DEL_SHIFT 4
#define TRAN_WS_DSP_MODE_SHIFT 5
#define REC_SCK_POLAR_SHIFT 6
#define REC_WS_POLAR_SHIFT 7
#define REC_APB_ALIGN_LR_SHIFT 8
#define I2S_ALIGN_LR_SHIFT 9
#define REC_DATA_WS_DEL_SHIFT 10
#define REC_WS_DSP_MODE_SHIFT 11

#define FMT_VAL_STANDARD_PHILLIPS (0x208)
#define FMT_VAL_LEFT_JUSTIFIED (0x69a)
#define FMT_VAL_RIGHT_JUSTIFIED (0x492)
#define FMT_VAL_DSP_A (0xaaa)
#define FMT_VAL_DSP_B (0xeba)

#define TRAN_SCK_POLAR_MASK _BIT_(0)
#define TRAN_WS_POLAR_MASK _BIT_(1)
#define TRAN_APB_ALIGN_LR_MASK _BIT_(2)
#define TRAN_I2S_ALIGN_LR_MASK _BIT_(3)
#define TRAN_DATA_WS_DEL_MASK _BIT_(4)
#define TRAN_WS_DSP_MODE_MASK _BIT_(5)
#define REC_SCK_POLAR_MASK _BIT_(6)
#define REC_WS_POLAR_MASK _BIT_(7)
#define REC_APB_ALIGN_LR_MASK _BIT_(8)
#define I2S_ALIGN_LR_MASK _BIT_(9)
#define REC_DATA_WS_DEL_MASK _BIT_(10)
#define REC_WS_DSP_MODE_MASK _BIT_(11)

/* I2S_MC REGS POLL_STAT */
#define MC_I2S_POLL_STAT (0x024)
#define STAT_TX_EMPTY_SHIFT 0
#define STAT_TX_AEMPTY_SHIFT 1
#define STAT_TX_UNDERRUN_SHIFT 2
#define STAT_RX_FULL_SHIFT 4
#define STAT_RX_AFULL_SHIFT 5
#define STAT_RX_OVERRUN_SHIFT 6

#define STAT_TX_EMPTY_MASK _BIT_(0)
#define STAT_TX_AEMPTY_MASK _BIT_(1)
#define STAT_TX_UNDERRUN_MASK _BIT_(2)
#define STAT_RX_FULL_MASK _BIT_(4)
#define STAT_RX_AFULL_MASK _BIT_(5)
#define STAT_RX_OVERRUN_MASK _BIT_(6)

typedef struct sdrv_i2s_mc_cfg {
    paddr_t base;
    uint32_t irq;
    clk_node_t *clk;
    dma_dev_t *dma_dev;
    uint8_t loop_mode;  /* clk sync loop */
    uint8_t clk_polar;  /* rising or falling edge sample */
    uint8_t tx_ch_mask; /* tx use which channels */
    uint8_t rx_ch_mask; /* rx use which channels */
} sdrv_i2s_mc_cfg_t;

typedef struct sdrv_i2s_mc {
    snd_dev_t dev;
    bool is_slave;              /* slave or master */
    uint8_t clk_sync_loop_mode; /* sync loop */
    uint8_t clk_sample_edge;    /* sample edge */
    uint8_t tx_channels_mask;   /* tx use which channels */
    uint8_t rx_channels_mask;   /* rx use which channels */
    uint8_t playback_status;    /* playback stream is in use */
    uint8_t capture_status;     /* capture stream is in use */
} sdrv_i2s_mc_t;

snd_dev_t *sdrv_i2s_mc_init(sdrv_i2s_mc_t *i2s_mc, sdrv_i2s_mc_cfg_t *cfg);

#endif