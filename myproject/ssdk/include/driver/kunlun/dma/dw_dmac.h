/**
 * @file  dw_dmac.h
 * @brief dw_dmac api header.
 *
 * @copyright Copyright (c) 2021  Semidrive Semiconductor.
 *            All rights reserved.
 */
#ifndef __DW_DMAC_H__
#define __DW_DMAC_H__

#include <types.h>
#include <bits.h>
#include <board.h>

#include "driver/kunlun/dma/dw_dma.h"

#define DMAC_CFG_REG_OFFSET 0x10
#define DMAC_CH_EN_OFFSET 0x18
#define DMAC_INT_STAT_OFFSET 0x30
#define DMAC_INT_CLR_OFFSET 0x38
#define DMAC_INT_EN_OFFSET 0x40
#define DMAC_INT_SIG_EN_OFFSET 0x48
#define DMAC_ERR_STAT_OFFSET 0x50
#define DMAC_RESET_OFFSET 0x58
#define DMAC_CH_BLK_TS_OFFSET 0x110

#define DMAC_CH_CTRL_OFFSET 0x118
#define DMAC_CH_CTRL_L_OFFSET 0x00
#define DMAC_CH_CTRL_H_OFFSET 0x04
#define DMAC_CH_CTRL_BLK_TR_SHIFT (26)
#define DMAC_CH_CTRL_BLK_TR_MASK(val) ((val) << DMAC_CH_CTRL_BLK_TR_SHIFT)
#define DMAC_CH_CTRL_LLI_VALID_SHIFT (31)
#define DMAC_CH_CTRL_LLI_VALID_MASK(val) ((val) << DMAC_CH_CTRL_LLI_VALID_SHIFT)

#define DMAC_CH_CFG_OFFSET 0x120
#define DMAC_CH_CFG_L_OFFSET 0x00
#define DMAC_CH_CFG_H_OFFSET 0x04

#define DMAC_CH_LLP_OFFSET 0x128
#define DMAC_CH_LLP_L_OFFSET 0x00
#define DMAC_CH_LLP_H_OFFSET 0x04

#define DMAC_CH_STAT_OFFSET 0x130
#define DMAC_CH_AXI_QOS_OFFSET 0x158
#define DMAC_CH_AXI_QOS_L_OFFSET 0x00
#define DMAC_CH_AXI_QOS_H_OFFSET 0x04

#define DMAC_CH_STAT_EN_OFFSET 0x180
#define DMAC_CH_STAT_EN_L_OFFSET 0x00
#define DMAC_CH_STAT_EN_H_OFFSET 0x04
#define DMAC_CH_STAT_EN_TR_DONE_SHIFT (0)
#define DMAC_CH_STAT_EN_TR_DONE_MASK(val)                                      \
    ((val) << DMAC_CH_STAT_EN_TR_DONE_SHIFT)

#define DMAC_CH_INT_STAT_OFFSET 0x188
#define DMAC_CH_INT_STAT_BLK_TR_DONE_SHIFT (0)
#define DMAC_CH_INT_STAT_BLK_TR_DONE_MASK(val)                                 \
    ((val) << DMAC_CH_INT_STAT_BLK_TR_DONE_SHIFT)
#define DMAC_CH_INT_STAT_TR_DONE_SHIFT (1)
#define DMAC_CH_INT_STAT_TR_DONE_MASK(val)                                     \
    ((val) << DMAC_CH_INT_STAT_TR_DONE_SHIFT)
#define DMAC_CH_INT_STAT_ABORT_SHIFT (31)
#define DMAC_CH_INT_STAT_ABORT_MASK(val) ((val) << DMAC_CH_INT_STAT_ABORT_SHIFT)
#define DMAC_CH_INT_STAT_DISABLED_SHIFT (30)
#define DMAC_CH_INT_STAT_DISABLED_MASK(val)                                    \
    ((val) << DMAC_CH_INT_STAT_DISABLED_SHIFT)
#define DMAC_CH_INT_STAT_SUSPEND_SHIFT (29)
#define DMAC_CH_INT_STAT_SUSPEND_MASK(val)                                     \
    ((val) << DMAC_CH_INT_STAT_SUSPEND_SHIFT)
#define DMAC_CH_INT_STAT_SRC_SUSPEND_SHIFT (28)
#define DMAC_CH_INT_STAT_SRC_SUSPEND_MASK(val)                                 \
    ((val) << DMAC_CH_INT_STAT_SRC_SUSPEND_SHIFT)

#define DMAC_CH_INT_SIG_EN_OFFSET 0x190
#define DMAC_CH_INT_CLR_OFFSET 0x198

#define DMAC_CH_REG_OFFSET 0x100
#define DMAC_CH_REG_SAR_L_OFFSET 0x00
#define DMAC_CH_REG_SAR_H_OFFSET 0x04
#define DMAC_CH_REG_DAR_L_OFFSET 0x08
#define DMAC_CH_REG_DAR_H_OFFSET 0x0C
#define DMAC_CH_REG_BLK_TS_OFFSET 0x10

#define DMAC_CH_REG_STEP 0x100

#define DW_DMAC_CHANNEL_NUMB (8)
#define DW_DMAC_MAX_BLK_SIZE (0x10000) /* Semidrive version size */
#define DW_DMAC_LLI_LAST_OFFSET (0x4000000000000000)

#if IS_64BIT
#define GEN_MASK(h, l) (((~0UL) - (1UL << (l)) + 1) & (~0UL >> (64 - 1 - (h))))
#else
#define GEN_MASK(h, l) (((~0UL) - (1UL << (l)) + 1) & (~0UL >> (32 - 1 - (h))))
#endif

typedef enum {
    DMAC_CHx_PRIORITY0 = 0,
    DMAC_CHx_PRIORITY1 = 1,
    DMAC_CHx_PRIORITY2 = 2,
    DMAC_CHx_PRIORITY3 = 3,
    DMAC_CHx_PRIORITY4 = 4,
    DMAC_CHx_PRIORITY5 = 5,
    DMAC_CHx_PRIORITY6 = 6,
    DMAC_CHx_PRIORITY7 = 7,
} dmac_ch_priority;

typedef enum {
    DMA_TRANSFER_TYPE_CONTINUOUS =
        0, /*both src and dst set continuous result single type*/
    DMA_TRANSFER_TYPE_RELOAD,
    DMA_TRANSFER_TYPE_SHADOW,
    DMA_TRANSFER_TYPE_LLI,
} dma_transfer_type;

typedef enum {
    DMA_TRANSFER_ADDR_INCREMENT =
        0, /*both src and dst set continuous result single type*/
    DMA_TRANSFER_ADDR_NOCHANGE = 1,
} dma_transfer_addr;

typedef enum {
    DMA_TRANSFER_DIR_MEM2MEM_DMAC = 0,
    DMA_TRANSFER_DIR_MEM2PER_DMAC = 1,
    DMA_TRANSFER_DIR_PER2MEM_DMAC = 2,
    DMA_TRANSFER_DIR_PER2PER_DMAC = 3,
    DMA_TRANSFER_DIR_PER2MEM_SRC = 4,
    DMA_TRANSFER_DIR_PER2PER_SRC = 5,
    DMA_TRANSFER_DIR_MEM2PER_DST = 6,
    DMA_TRANSFER_DIR_PER2PER_DST = 7,
} dma_transfer_direction;

typedef enum {
    DMA_TRANSFER_WIDTH_8BITS = 0,   /* 1 */
    DMA_TRANSFER_WIDTH_16BITS = 1,  /* 2 */
    DMA_TRANSFER_WIDTH_32BITS = 2,  /* 4 */
    DMA_TRANSFER_WIDTH_64BITS = 3,  /* 8 */
    DMA_TRANSFER_WIDTH_128BITS = 4, /* 16 */
    DMA_TRANSFER_WIDTH_256BITS = 5, /* 32 */
    DMA_TRANSFER_WIDTH_512BITS = 6, /* 64 */
    DMA_TRANSFER_WIDTH_MAX = DMA_TRANSFER_WIDTH_512BITS
} dma_transfer_width;

/**
 * @brief dma burst transfer length.
 */
typedef enum {
    DMA_BURST_TR_1ITEM = 0,
    DMA_BURST_TR_4ITEMS = 1,
    DMA_BURST_TR_8ITEMS = 2,
    DMA_BURST_TR_16ITEMS = 3,
    DMA_BURST_TR_32ITEMS = 4,
    DMA_BURST_TR_64ITEMS = 5,
    DMA_BURST_TR_128ITEMS = 6,
    DMA_BURST_TR_256ITEMS = 7,
    DMA_BURST_TR_512ITEMS = 8,
    DMA_BURST_TR_1024ITEMS = 9,
} dma_burst_tr_len;

/* Int status */
typedef enum {
    DMA_BLOCK_TR_DONE = BIT(0xffffffff, 0),
    DMA_DMA_TR_DONE = BIT(0xffffffff, 1),
    DMA_SRC_COMP = BIT(0xffffffff, 3),
    DMA_DST_COMP = BIT(0xffffffff, 4),
    DMA_SRC_DEC_ERR = BIT(0xffffffff, 5),
    DMA_DST_DEC_ERR = BIT(0xffffffff, 6),
    DMA_SRC_SLV_ERR = BIT(0xffffffff, 7),
    DMA_DST_SLV_ERR = BIT(0xffffffff, 8),
    DMA_LLI_RD_DEC_ERR = BIT(0xffffffff, 9),
    DMA_LLI_WR_DEC_ERR = BIT(0xffffffff, 10),
    DMA_LLI_RD_SLV_ERR = BIT(0xffffffff, 11),
    DMA_LLI_WR_SLV_ERR = BIT(0xffffffff, 12),
    DMA_INVALID_ERR = BIT(0xffffffff, 13),
    DMA_MULTIBLKTYPE_ERR = BIT(0xffffffff, 14),
    DMA_DEC_ERR = BIT(0xffffffff, 16),
    DMA_WR2RO_ERR = BIT(0xffffffff, 17),
    DMA_RD2RWO_ERR = BIT(0xffffffff, 18),
    DMA_WR_ON_CHEN_ERR = BIT(0xffffffff, 19),
    DMA_SHADOW_WR_VALID_ERR = BIT(0xffffffff, 20),
    DMA_WR_ONHOLD_ERR = BIT(0xffffffff, 21),
    DMA_LOCK_CLEARED = BIT(0xffffffff, 27),
    DMA_SRC_SUSPENDED = BIT(0xffffffff, 28),
    DMA_SUSPENDED = BIT(0xffffffff, 29),
    DMA_DISABLED = BIT(0xffffffff, 30),
    DMA_ABORTED = BIT(0xffffffff, 31),
    DMA_ALL_ERR = (GEN_MASK(21, 16) | GEN_MASK(14, 5)),
    DMA_ALL = GEN_MASK(31, 0)
} dma_int_status;

/* 0x10 DMAC_CFGREG */
typedef union {
    struct {
        volatile uint32_t
            dmac_en : 1; /* This bit is used to enable the DW_axi_dmac. */
        volatile uint32_t int_en : 1; /* This bit is used to globally enable the
                                         interrupt generation. */
        volatile uint32_t rsvd : 30;
        volatile uint32_t rsvd1 : 32;
    };
    struct {
        volatile uint32_t vl;
        volatile uint32_t vh;
    };
} dmac_cfg_t;

/* 0x18 This is DW_axi_dmac Channel Enable Register. */
typedef union {
    struct {
        volatile uint32_t ch1_en : 1;    /* enable the DW_axi_dmac channel 1. */
        volatile uint32_t ch2_en : 1;    /* enable the DW_axi_dmac channel 2. */
        volatile uint32_t ch3_en : 1;    /* enable the DW_axi_dmac channel 3. */
        volatile uint32_t ch4_en : 1;    /* enable the DW_axi_dmac channel 4. */
        volatile uint32_t ch5_en : 1;    /* enable the DW_axi_dmac channel 5. */
        volatile uint32_t ch6_en : 1;    /* enable the DW_axi_dmac channel 6. */
        volatile uint32_t ch7_en : 1;    /* enable the DW_axi_dmac channel 7. */
        volatile uint32_t ch8_en : 1;    /* enable the DW_axi_dmac channel 8. */
        volatile uint32_t ch1_en_we : 1; /* enable the DW_axi_dmac channel 1
                                            Write Enable bit. */
        volatile uint32_t ch2_en_we : 1; /* enable the DW_axi_dmac channel 2
                                            Write Enable bit. */
        volatile uint32_t ch3_en_we : 1; /* enable the DW_axi_dmac channel 3
                                            Write Enable bit. */
        volatile uint32_t ch4_en_we : 1; /* enable the DW_axi_dmac channel 4
                                            Write Enable bit. */
        volatile uint32_t ch5_en_we : 1; /* enable the DW_axi_dmac channel 5
                                            Write Enable bit. */
        volatile uint32_t ch6_en_we : 1; /* enable the DW_axi_dmac channel 6
                                            Write Enable bit. */
        volatile uint32_t ch7_en_we : 1; /* enable the DW_axi_dmac channel 7
                                            Write Enable bit. */
        volatile uint32_t ch8_en_we : 1; /* enable the DW_axi_dmac channel 8
                                            Write Enable bit. */
        volatile uint32_t
            ch1_susp : 1; /* the DW_axi_dmac channel 1 Suspend Request. */
        volatile uint32_t
            ch2_susp : 1; /* the DW_axi_dmac channel 2 Suspend Request. */
        volatile uint32_t
            ch3_susp : 1; /* the DW_axi_dmac channel 3 Suspend Request. */
        volatile uint32_t
            ch4_susp : 1; /* the DW_axi_dmac channel 4 Suspend Request. */
        volatile uint32_t
            ch5_susp : 1; /* the DW_axi_dmac channel 5 Suspend Request. */
        volatile uint32_t
            ch6_susp : 1; /* the DW_axi_dmac channel 6 Suspend Request. */
        volatile uint32_t
            ch7_susp : 1; /* the DW_axi_dmac channel 7 Suspend Request. */
        volatile uint32_t
            ch8_susp : 1; /* the DW_axi_dmac channel 8 Suspend Request. */
        volatile uint32_t ch1_susp_we : 1; /* the DW_axi_dmac channel 1 Suspend
                                              Request Write Enable bit. */
        volatile uint32_t ch2_susp_we : 1; /* the DW_axi_dmac channel 2 Suspend
                                              Request Write Enable bit. */
        volatile uint32_t ch3_susp_we : 1; /* the DW_axi_dmac channel 3 Suspend
                                              Request Write Enable bit. */
        volatile uint32_t ch4_susp_we : 1; /* the DW_axi_dmac channel 4 Suspend
                                              Request Write Enable bit. */
        volatile uint32_t ch5_susp_we : 1; /* the DW_axi_dmac channel 5 Suspend
                                              Request Write Enable bit. */
        volatile uint32_t ch6_susp_we : 1; /* the DW_axi_dmac channel 6 Suspend
                                              Request Write Enable bit. */
        volatile uint32_t ch7_susp_we : 1; /* the DW_axi_dmac channel 7 Suspend
                                              Request Write Enable bit. */
        volatile uint32_t ch8_susp_we : 1; /* the DW_axi_dmac channel 8 Suspend
                                              Request Write Enable bit. */

        volatile uint32_t
            ch1_abort : 1; /* The DW_axi_dmac channel 1 abort request. */
        volatile uint32_t
            ch2_abort : 1; /* The DW_axi_dmac channel 2 abort request. */
        volatile uint32_t
            ch3_abort : 1; /* The DW_axi_dmac channel 3 abort request. */
        volatile uint32_t
            ch4_abort : 1; /* The DW_axi_dmac channel 4 abort request. */
        volatile uint32_t
            ch5_abort : 1; /* The DW_axi_dmac channel 5 abort request. */
        volatile uint32_t
            ch6_abort : 1; /* The DW_axi_dmac channel 6 abort request. */
        volatile uint32_t
            ch7_abort : 1; /* The DW_axi_dmac channel 7 abort request. */
        volatile uint32_t
            ch8_abort : 1; /* The DW_axi_dmac channel 8 abort request. */
        volatile uint32_t ch1_abort_we : 1; /* The DW_axi_dmac channel 1 abort
                                               request Write Enable bit. */
        volatile uint32_t ch2_abort_we : 1; /* The DW_axi_dmac channel 2 abort
                                               request Write Enable bit. */
        volatile uint32_t ch3_abort_we : 1; /* The DW_axi_dmac channel 3 abort
                                               request Write Enable bit. */
        volatile uint32_t ch4_abort_we : 1; /* The DW_axi_dmac channel 4 abort
                                               request Write Enable bit. */
        volatile uint32_t ch5_abort_we : 1; /* The DW_axi_dmac channel 5 abort
                                               request Write Enable bit. */
        volatile uint32_t ch6_abort_we : 1; /* The DW_axi_dmac channel 6 abort
                                               request Write Enable bit. */
        volatile uint32_t ch7_abort_we : 1; /* The DW_axi_dmac channel 7 abort
                                               request Write Enable bit. */
        volatile uint32_t ch8_abort_we : 1; /* The DW_axi_dmac channel 8 abort
                                               request Write Enable bit. */
        volatile uint32_t rvsd_48 : 16;
    };
    struct {
        volatile uint32_t vl;
        volatile uint32_t vh;
    };
} dmac_ch_en_t;

/* 0x30 DMAC Interrupt Status Register captures the combined channel interrupt
 * for each channel and Combined common register block interrupt. */
typedef union {
    struct {
        volatile uint32_t
            ch1_int_stat : 1; /* Channel 1 Interrupt Status Bit. */
        volatile uint32_t
            ch2_int_stat : 1; /* Channel 2 Interrupt Status Bit. */
        volatile uint32_t
            ch3_int_stat : 1; /* Channel 3 Interrupt Status Bit. */
        volatile uint32_t
            ch4_int_stat : 1; /* Channel 4 Interrupt Status Bit. */
        volatile uint32_t
            ch5_int_stat : 1; /* Channel 5 Interrupt Status Bit. */
        volatile uint32_t
            ch6_int_stat : 1; /* Channel 6 Interrupt Status Bit. */
        volatile uint32_t
            ch7_int_stat : 1; /* Channel 7 Interrupt Status Bit. */
        volatile uint32_t
            ch8_int_stat : 1;         /* Channel 8 Interrupt Status Bit. */
        volatile uint32_t rsvd_8 : 8; /* Channel 1 Interrupt Status Bit. */
        volatile uint32_t
            comm_reg_int_Stat : 1; /* Common Register Interrupt Status Bit. */
        volatile uint32_t rsvd_17 : 15;
        volatile uint32_t rsvd_32 : 32;
    };
    struct {
        volatile uint32_t vl;
        volatile uint32_t vh;
    };
} dmac_int_stat_t;

/* 0x38 Writing 1 to specific field clears the corresponding field in DMAC
 * Common register Interrupt Status Register (DMAC_CommonReg_IntStatusReg). */
typedef union {
    struct {
        volatile uint32_t
            slvif_dec_err : 1; /* Slave Interface Common Register Decode Error
                                  Interrupt clear Bit. */
        volatile uint32_t
            slvif_wr2ro_err : 1; /* Slave Interface Common Register Write to
                                    Read only Error Interrupt clear Bit. */
        volatile uint32_t
            slvif_rd2wo_err : 1; /* Slave Interface Common Register Read to
                                    Write only Error Interrupt clear Bit. */
        volatile uint32_t
            slvif_wr_onhold_err : 1; /* Slave Interface Common Register Write On
                                        Hold Error Interrupt clear Bit. */
        volatile uint32_t rsvd_4 : 4;
        volatile uint32_t
            slvif_undef_dec_err : 1; /* Slave Interface Undefined register
                                        Decode Error Interrupt clear Bit. */
        volatile uint32_t rsvd_9 : 23;
        volatile uint32_t rsvd_32 : 32;
    };
    struct {
        volatile uint32_t vl;
        volatile uint32_t vh;
    };
} dmac_comm_int_clr_t;

/* 0x40 Writing 1 to specific field enables the corresponding interrupt status
 * generation in DMAC Common register Interrupt Status Register
 * (DMAC_CommonReg_IntStatusReg). */
typedef union {
    struct {
        volatile uint32_t
            slvif_dec_err : 1; /* Slave Interface Common Register Decode Error
                                  Interrupt enable Bit. */
        volatile uint32_t
            slvif_wr2ro_err : 1; /* Slave Interface Common Register Write to
                                    Read only Error Interrupt enable Bit. */
        volatile uint32_t
            slvif_rd2wo_err : 1; /* Slave Interface Common Register Read to
                                    Write only Error Interrupt enable Bit. */
        volatile uint32_t
            slvif_wr_onhold_err : 1; /* Slave Interface Common Register Write On
                                        Hold Error Interrupt enable Bit. */
        volatile uint32_t rsvd_4 : 4;
        volatile uint32_t
            slvif_undef_dec_err : 1; /* Slave Interface Undefined register
                                        Decode Error Interrupt enable Bit. */
        volatile uint32_t rsvd_9 : 23;
        volatile uint32_t rsvd_32 : 32;
    };
    struct {
        volatile uint32_t vl;
        volatile uint32_t vh;
    };
} dmac_comm_int_stat_en_t;

/* 0x48 Writing 1 to specific field will propagate the corresponding interrupt
 * status in DMAC Common register Interrupt Status Register
 * (DMAC_CommonReg_IntStatusReg) to generate an port level interrupt. */
typedef union {
    struct {
        volatile uint32_t
            slvif_dec_err : 1; /* Slave Interface Common Register Decode Error
                                  Interrupt clear Bit. */
        volatile uint32_t
            slvif_wr2ro_err : 1; /* Slave Interface Common Register Write to
                                    Read only Error Interrupt clear Bit. */
        volatile uint32_t
            slvif_rd2wo_err : 1; /* Slave Interface Common Register Read to
                                    Write only Error Interrupt clear Bit. */
        volatile uint32_t
            slvif_wr_onhold_err : 1; /* Slave Interface Common Register Write On
                                        Hold Error Interrupt clear Bit. */
        volatile uint32_t rsvd_4 : 4;
        volatile uint32_t
            slvif_undef_dec_err : 1; /* Slave Interface Undefined register
                                        Decode Error Interrupt clear Bit. */
        volatile uint32_t rsvd_9 : 23;
        volatile uint32_t rsvd_32 : 32;
    };
    struct {
        volatile uint32_t vl;
        volatile uint32_t vh;
    };
} dmac_comm_int_sig_en_t;

/* 0x50 This Register captures Slave interface access errors. */
typedef union {
    struct {
        volatile uint32_t
            slvif_dec_err : 1; /* Slave Interface Common Register Decode Error
                                  Interrupt clear Bit. */
        volatile uint32_t
            slvif_wr2ro_err : 1; /* Slave Interface Common Register Write to
                                    Read only Error Interrupt clear Bit. */
        volatile uint32_t
            slvif_rd2wo_err : 1; /* Slave Interface Common Register Read to
                                    Write only Error Interrupt clear Bit. */
        volatile uint32_t
            slvif_wr_onhold_err : 1; /* Slave Interface Common Register Write On
                                        Hold Error Interrupt clear Bit. */
        volatile uint32_t rsvd_4 : 4;
        volatile uint32_t
            slvif_undef_dec_err : 1; /* Slave Interface Undefined register
                                        Decode Error Interrupt clear Bit. */
        volatile uint32_t rsvd_9 : 23;
        volatile uint32_t rsvd_32 : 32;
    };
    struct {
        volatile uint32_t vl;
        volatile uint32_t vh;
    };
} dmac_comm_int_stat_t;

/* 0x58 This register is used to initiate the Software Reset to DW_axi_dmac. */
typedef union {
    struct {
        volatile uint32_t reset : 1; /* DMAC Reset Request bit Software writes 1
                                        to this bit */
        volatile uint32_t rsvd : 31;
        volatile uint32_t rsvd2 : 32;
    };
    struct {
        volatile uint32_t vl;
        volatile uint32_t vh;
    };
} dmac_reset_t;

/* 0x110 + x(x-1)*0x100 */
typedef union {
    struct {
        volatile uint32_t blk_ts : 22; /* Block Transfer Size = BLOCK_TS+1 */
        volatile uint32_t rsvd : 10;
        volatile uint32_t rsvd_32 : 32;
    };
    struct {
        volatile uint32_t vl;
        volatile uint32_t vh;
    };
} ch_blk_ts_t;

/* ctl structure 0x118 + x(x-1)*0x100 */
typedef union {
    struct {
        volatile uint32_t sms : 1;    /* default to 0 */
        volatile uint32_t rsvd_1 : 1; /* reserved chan x0 1 */
        volatile uint32_t dms : 1;    /* default to 0 */
        volatile uint32_t rsvd_3 : 1; /* reserved chan x0 3 */
        volatile uint32_t sinc : 1;   /*  Source Address Increment. */
        volatile uint32_t rsvd_5 : 1; /* reserved chan x0 5 */
        volatile uint32_t dinc : 1;   /*  Destination Address Increment. */
        volatile uint32_t rsvd_7 : 1; /* reserved chan x0 7 */
        volatile uint32_t src_tr_width : 3; /* Source Transfer Width. */
        volatile uint32_t dst_tr_width : 3; /* Destination Transfer Width */
        volatile uint32_t src_msize : 4; /* Source Burst Transaction Length. */
        volatile uint32_t
            dst_msize : 4; /* Destination Burst Transaction Length. */
        volatile uint32_t ar_cache : 4; /* default 0 AXI 'ar_cache' signal */
        volatile uint32_t aw_cache : 4; /* default 0 AXI 'aw_cache' signal */
        volatile uint32_t
            nonposted_last_write_en : 1;   /*  0 1 : no transfer 0 :transfer */
        volatile uint32_t rsvd_31 : 1;     /* reserved chan x0 7 */
        volatile uint32_t ar_prot : 3;     /* AXI 'ar_prot' signal */
        volatile uint32_t aw_prot : 3;     /* AXI 'aw_prot' signal */
        volatile uint32_t arlen_en : 1;    /* Source Burst Length Enable */
        volatile uint32_t arlen : 8;       /* Source Burst Length */
        volatile uint32_t awlen_en : 1;    /* Destination Burst Length Enable */
        volatile uint32_t awlen : 8;       /* Destination Burst Length */
        volatile uint32_t src_stat_en : 1; /* Source Status Enable */
        volatile uint32_t dst_stat_en : 1; /* Destination Status Enable */
        volatile uint32_t
            ioc_blk_tr : 1; /* Interrupt On completion of Block Transfer 1
                               enable interrupt 0 disable */
        volatile uint32_t rsvd_59 : 3; /* reserved chan x0 59-61 */
        volatile uint32_t
            shadow_lli_last : 1; /* Last Shadow Register/Linked List Item. 0:
                                    Not last Shadow Register/LLI 1:Last Shadow
                                    Register/LLI */
        volatile uint32_t
            shadow_lli_valid : 1; /* 0: Shadow Register content/LLI is
                                     invalid.1: Last Shadow Register/LLI is
                                     valid. */
    };
    struct {
        volatile uint32_t vl;
        volatile uint32_t vh;
    };
} ch_ctl_t;

/* 0x120 + x(x-1)*0x100 */
typedef union {
    struct {
        volatile uint32_t
            src_multblk_type : 2; /* Source Multi Block Transfer Type. */
        volatile uint32_t
            dst_multblk_type : 2; /* Destination Multi Block Transfer Type. */
        volatile uint32_t rsvd_4 : 28;
        volatile uint32_t tt_fc : 3;      /* Transfer Type and Flow Control. */
        volatile uint32_t hs_sel_src : 1; /* Source Software or Hardware
                                             Handshaking Select. */
        volatile uint32_t hs_sel_dst : 1; /* Destination Software or Hardware
                                             Handshaking Select. */
        volatile uint32_t src_hwhs_pol : 1; /* Source Hardware Handshaking
                                               Interface Polarity. */
        volatile uint32_t dst_hwhs_pol : 1; /* Destination Hardware Handshaking
                                               Interface Polarity. */
        volatile uint32_t src_per : 4;
        volatile uint32_t rsvd_42 : 1;
        volatile uint32_t dst_per : 4;
        volatile uint32_t rsvd_48 : 1;
        volatile uint32_t ch_pri : 3;    /* Channel Priority 0-7 */
        volatile uint32_t lock_ch : 1;   /* Channel Lock bit */
        volatile uint32_t lock_ch_l : 2; /* Channel Lock Level */
        volatile uint32_t
            src_osr_lmt : 4; /* Source Outstanding Request Limit */
        volatile uint32_t dst_osr_lmt : 4; /* Channel Lock Level */
        volatile uint32_t
            rsvd_63 : 1; /* DMAC Channel_x Transfer Configuration Register */
    };
    struct {
        volatile uint32_t vl;
        volatile uint32_t vh;
    };
} ch_cfg_t;

/* 0x128 + x(x-1)*0x100 */
typedef union {
    struct {
        volatile uint64_t
            lms : 1; /* DMAX_CH(x)_LMS == 2) ? "read-write" : "read-only"} */
        volatile uint64_t rsvd : 5;
        volatile uint64_t loc : 58; /* Starting Address Memory of LLI block */
    };
    struct {
        volatile uint32_t vl;
        volatile uint32_t vh;
    };
} ch_llp_t;

/* 0x130 + x(x-1)*0x100 Channel_x Status Register contains fields that indicate
 * the status of DMA transfers for Channelx. */
typedef union {
    struct {
        volatile uint32_t cmp_blk_tr : 22; /* Completed Block Transfer Size. */
        volatile uint32_t rsvd : 10;
        volatile uint32_t data_in_fifo : 15; /* Data Left in FIFO. */
        volatile uint32_t rsvd_47 : 17;
    };
    struct {
        volatile uint32_t vl;
        volatile uint32_t vh;
    };
} ch_stat_t;

/* 0x180 Writing 1 to specific field enables the corresponding interrupt status
 * generation in Channelx Interrupt Status Register(CH1_IntStatusReg). */
typedef union {
    struct {
        /* data */
        volatile uint32_t block_tr_done : 1; /* Block Transfer Done Interrupt
                                                Status Enable. */
        volatile uint32_t
            dma_tr_done : 1; /* DMA Transfer Done Interrupt Status Enable. */
        volatile uint32_t rvsd_2 : 1;
        volatile uint32_t
            src_tr_comp : 1; /* Source Transaction Completed Status Enable. */
        volatile uint32_t dst_tr_comp : 1; /* Destination Transaction Completed
                                              Status Enable. */
        volatile uint32_t
            src_dec_err : 1; /* Source Decode Error Status Enable. */
        volatile uint32_t
            dst_dec_err : 1; /* Destination Decode Error Status Enable. */
        volatile uint32_t
            src_slv_err : 1; /* Source Slave Error Status Enable. */
        volatile uint32_t
            dst_slv_err : 1; /* Destination Slave Error Status Enable. */
        volatile uint32_t
            lli_rd_dec_err : 1; /* LLI Read Decode Error Status Enable. */
        volatile uint32_t
            lli_wr_dec_err : 1; /* LLI WRITE Decode Error Status Enable. */
        volatile uint32_t
            lli_rd_slv_err : 1; /* LLI Read Slave Error Status Enable. */
        volatile uint32_t
            lli_wr_slv_err : 1; /* LLI WRITE Slave Error Status Enable. */
        volatile uint32_t
            shadow_lli_invalid_err : 1; /* Shadow register or LLI Invalid Error
                                           Status Enable. */
        volatile uint32_t
            slvif_multiblk_type_err : 1; /* Slave Interface Multi Block type
                                            Error Signal Enable. */
        volatile uint32_t rsvd_15 : 1;
        volatile uint32_t
            slvif_dec_err : 1; /* Slave Interface Decode Error Signal Enable. */
        volatile uint32_t slvif_wr2ro_err : 1; /* Slave Interface Write to Read
                                                  Only Error Signal Enable. */
        volatile uint32_t slvif_rd2wo_err : 1; /* Slave Interface Read to write
                                                  Only Error Signal Enable. */
        volatile uint32_t
            slvif_wr_on_chan_err : 1; /* Slave Interface Write On Channel
                                         Enabled Error Signal Enable. */
        volatile uint32_t
            slvif_shadow_wr_valid_err : 1; /* Shadow Register Write On Valid
                                              Error Signal Enable. */
        volatile uint32_t
            slvif_wr_onhold_err : 1; /* Slave Interface Write On Hold Error
                                        Signal Enable. */
        volatile uint32_t rsvd_22 : 5;
        volatile uint32_t
            ch_lock_cleared : 1; /* Channel Lock Cleared Signal Enable. */
        volatile uint32_t
            ch_src_suspended : 1; /* Channel Source Suspended Signal Enable. */
        volatile uint32_t
            ch_suspended : 1; /* Channel Suspended Signal Enable. */
        volatile uint32_t ch_disabled : 1; /* Channel Disabled Signal Enable. */
        volatile uint32_t ch_aborted : 1;  /* Channel Aborted Signal Enable. */
        volatile uint32_t rsvd_32 : 32;
    };
    struct {
        volatile uint32_t vl;
        volatile uint32_t vh;
    };
} ch_int_stat_en_t;

typedef union {
    struct {
        /* data */
        volatile uint32_t block_tr_done : 1; /* Block Transfer Done Interrupt
                                                Status Enable. */
        volatile uint32_t
            dma_tr_done : 1; /* DMA Transfer Done Interrupt Status Enable. */
        volatile uint32_t rvsd_2 : 1;
        volatile uint32_t
            src_tr_comp : 1; /* Source Transaction Completed Status Enable. */
        volatile uint32_t dst_tr_comp : 1; /* Destination Transaction Completed
                                              Status Enable. */
        volatile uint32_t
            src_dec_err : 1; /* Source Decode Error Status Enable. */
        volatile uint32_t
            dst_dec_err : 1; /* Destination Decode Error Status Enable. */
        volatile uint32_t
            src_slv_err : 1; /* Source Slave Error Status Enable. */
        volatile uint32_t
            dst_slv_err : 1; /* Destination Slave Error Status Enable. */
        volatile uint32_t
            lli_rd_dec_err : 1; /* LLI Read Decode Error Status Enable. */
        volatile uint32_t
            lli_wr_dec_err : 1; /* LLI WRITE Decode Error Status Enable. */
        volatile uint32_t
            lli_rd_slv_err : 1; /* LLI Read Slave Error Status Enable. */
        volatile uint32_t
            lli_wr_slv_err : 1; /* LLI WRITE Slave Error Status Enable. */
        volatile uint32_t
            shadow_lli_invalid_err : 1; /* Shadow register or LLI Invalid Error
                                           Status Enable. */
        volatile uint32_t
            slvif_multiblk_type_err : 1; /* Slave Interface Multi Block type
                                            Error Signal Enable. */
        volatile uint32_t rsvd_15 : 1;
        volatile uint32_t
            slvif_dec_err : 1; /* Slave Interface Decode Error Signal Enable. */
        volatile uint32_t slvif_wr2ro_err : 1; /* Slave Interface Write to Read
                                                  Only Error Signal Enable. */
        volatile uint32_t slvif_rd2wo_err : 1; /* Slave Interface Read to write
                                                  Only Error Signal Enable. */
        volatile uint32_t
            slvif_wr_on_chan_err : 1; /* Slave Interface Write On Channel
                                         Enabled Error Signal Enable. */
        volatile uint32_t
            slvif_shadow_wr_valid_err : 1; /* Shadow Register Write On Valid
                                              Error Signal Enable. */
        volatile uint32_t
            slvif_wr_onhold_err : 1; /* Slave Interface Write On Hold Error
                                        Signal Enable. */
        volatile uint32_t rsvd_22 : 5;
        volatile uint32_t
            ch_lock_cleared : 1; /* Channel Lock Cleared Signal Enable. */
        volatile uint32_t
            ch_src_suspended : 1; /* Channel Source Suspended Signal Enable. */
        volatile uint32_t
            ch_suspended : 1; /* Channel Suspended Signal Enable. */
        volatile uint32_t ch_disabled : 1; /* Channel Disabled Signal Enable. */
        volatile uint32_t ch_aborted : 1;  /* Channel Aborted Signal Enable. */
        volatile uint32_t rsvd_32 : 32;
    };
    struct {
        volatile uint32_t vl;
        volatile uint32_t vh;
    };

} ch_int_stat_t;

/* 0x190 This register contains fields that are used to enable the generation of
 * port level interrupt at the channel level. */
typedef union {
    struct {
        /* data */
        volatile uint32_t block_tr_done : 1; /* Block Transfer Done Interrupt
                                                Status Enable. */
        volatile uint32_t
            dma_tr_done : 1; /* DMA Transfer Done Interrupt Status Enable. */
        volatile uint32_t rvsd_2 : 1;
        volatile uint32_t
            src_tr_comp : 1; /* Source Transaction Completed Status Enable. */
        volatile uint32_t dst_tr_comp : 1; /* Destination Transaction Completed
                                              Status Enable. */
        volatile uint32_t
            src_dec_err : 1; /* Source Decode Error Status Enable. */
        volatile uint32_t
            dst_dec_err : 1; /* Destination Decode Error Status Enable. */
        volatile uint32_t
            src_slv_err : 1; /* Source Slave Error Status Enable. */
        volatile uint32_t
            dst_slv_err : 1; /* Destination Slave Error Status Enable. */
        volatile uint32_t
            lli_rd_dec_err : 1; /* LLI Read Decode Error Status Enable. */
        volatile uint32_t
            lli_wr_dec_err : 1; /* LLI WRITE Decode Error Status Enable. */
        volatile uint32_t
            lli_rd_slv_err : 1; /* LLI Read Slave Error Status Enable. */
        volatile uint32_t
            lli_wr_slv_err : 1; /* LLI WRITE Slave Error Status Enable. */
        volatile uint32_t
            shadow_lli_invalid_err : 1; /* Shadow register or LLI Invalid Error
                                           Status Enable. */
        volatile uint32_t
            slvif_multiblk_type_err : 1; /* Slave Interface Multi Block type
                                            Error Signal Enable. */
        volatile uint32_t rsvd_15 : 1;
        volatile uint32_t
            slvif_dec_err : 1; /* Slave Interface Decode Error Signal Enable. */
        volatile uint32_t slvif_wr2ro_err : 1; /* Slave Interface Write to Read
                                                  Only Error Signal Enable. */
        volatile uint32_t slvif_rd2wo_err : 1; /* Slave Interface Read to write
                                                  Only Error Signal Enable. */
        volatile uint32_t
            slvif_wr_on_chan_err : 1; /* Slave Interface Write On Channel
                                         Enabled Error Signal Enable. */
        volatile uint32_t
            slvif_shadow_wr_valid_err : 1; /* Shadow Register Write On Valid
                                              Error Signal Enable. */
        volatile uint32_t
            slvif_wr_onhold_err : 1; /* Slave Interface Write On Hold Error
                                        Signal Enable. */
        volatile uint32_t rsvd_22 : 5;
        volatile uint32_t
            ch_lock_cleared : 1; /* Channel Lock Cleared Signal Enable. */
        volatile uint32_t
            ch_src_suspended : 1; /* Channel Source Suspended Signal Enable. */
        volatile uint32_t
            ch_suspended : 1; /* Channel Suspended Signal Enable. */
        volatile uint32_t ch_disabled : 1; /* Channel Disabled Signal Enable. */
        volatile uint32_t ch_aborted : 1;  /* Channel Aborted Signal Enable. */
        volatile uint32_t rsvd_32 : 32;
    };
    struct {
        volatile uint32_t vl;
        volatile uint32_t vh;
    };
} dmac_ch_int_sig_en_t;

/* 0x198 */
typedef union {
    struct {
        /* data */
        volatile uint32_t block_tr_done : 1; /* Block Transfer Done Interrupt
                                                Status Enable. */
        volatile uint32_t
            dma_tr_done : 1; /* DMA Transfer Done Interrupt Status Enable. */
        volatile uint32_t rvsd_2 : 1;
        volatile uint32_t
            src_tr_comp : 1; /* Source Transaction Completed Status Enable. */
        volatile uint32_t dst_tr_comp : 1; /* Destination Transaction Completed
                                              Status Enable. */
        volatile uint32_t
            src_dec_err : 1; /* Source Decode Error Status Enable. */
        volatile uint32_t
            dst_dec_err : 1; /* Destination Decode Error Status Enable. */
        volatile uint32_t
            src_slv_err : 1; /* Source Slave Error Status Enable. */
        volatile uint32_t
            dst_slv_err : 1; /* Destination Slave Error Status Enable. */
        volatile uint32_t
            lli_rd_dec_err : 1; /* LLI Read Decode Error Status Enable. */
        volatile uint32_t
            lli_wr_dec_err : 1; /* LLI WRITE Decode Error Status Enable. */
        volatile uint32_t
            lli_rd_slv_err : 1; /* LLI Read Slave Error Status Enable. */
        volatile uint32_t
            lli_wr_slv_err : 1; /* LLI WRITE Slave Error Status Enable. */
        volatile uint32_t
            shadow_lli_invalid_err : 1; /* Shadow register or LLI Invalid Error
                                           Status Enable. */
        volatile uint32_t
            slvif_multiblk_type_err : 1; /* Slave Interface Multi Block type
                                            Error Signal Enable. */
        volatile uint32_t rsvd_15 : 1;
        volatile uint32_t
            slvif_dec_err : 1; /* Slave Interface Decode Error Signal Enable. */
        volatile uint32_t slvif_wr2ro_err : 1; /* Slave Interface Write to Read
                                                  Only Error Signal Enable. */
        volatile uint32_t slvif_rd2wo_err : 1; /* Slave Interface Read to write
                                                  Only Error Signal Enable. */
        volatile uint32_t
            slvif_wr_on_chan_err : 1; /* Slave Interface Write On Channel
                                         Enabled Error Signal Enable. */
        volatile uint32_t
            slvif_shadow_wr_valid_err : 1; /* Shadow Register Write On Valid
                                              Error Signal Enable. */
        volatile uint32_t
            slvif_wr_onhold_err : 1; /* Slave Interface Write On Hold Error
                                        Signal Enable. */
        volatile uint32_t rsvd_22 : 5;
        volatile uint32_t
            ch_lock_cleared : 1; /* Channel Lock Cleared Signal Enable. */
        volatile uint32_t
            ch_src_suspended : 1; /* Channel Source Suspended Signal Enable. */
        volatile uint32_t
            ch_suspended : 1; /* Channel Suspended Signal Enable. */
        volatile uint32_t ch_disabled : 1; /* Channel Disabled Signal Enable. */
        volatile uint32_t ch_aborted : 1;  /* Channel Aborted Signal Enable. */
        volatile uint32_t rsvd_32 : 32;
    };
    struct {
        volatile uint32_t vl;
        volatile uint32_t vh;
    };
} ch_int_clr_t;

/* dmac channel */
typedef struct dmac_ch {
    volatile uint32_t
        sar_l; // Source Address Register      0x100 + x(x-1)*0x100
    volatile uint32_t sar_h;
    volatile uint32_t
        dar_l; // Destination Address Register 0x108 + x(x-1)*0x100
    volatile uint32_t dar_h;
    volatile ch_blk_ts_t blk_ts; /* block-size. 0x110 + x(x-1)*0x100 */
    volatile ch_ctl_t ctl;       /* Control Register   0x118 + x(x-1)*0x100 */
    volatile ch_cfg_t cfg;       /* Configuration Register */
    volatile ch_llp_t llp;       /*  Linked List Pointer register. */
    volatile ch_stat_t status;   /* Channelx Status Register 0x130 */
    volatile uint64_t
        swhssrc; /* Channelx Software handshake Source Register. */
    volatile uint64_t
        swhsdst; /* Channelx Software handshake Destination Register. */
    volatile uint64_t blk_tr_resume_req; /* Channelx Block Transfer Resume
                                            Request Register. */
    volatile uint64_t axi_id;            /* Channelx AXI ID Register. */
    volatile uint64_t axi_qos;           /* Channelx AXI QOS Register. */
    volatile uint64_t sstat;
    volatile uint64_t dstat;
    volatile uint64_t sstatar; /* Channelx Source Status Fetch Register. */
    volatile uint64_t dstatar; /* Channelx Destination Status Fetch Register. */
    volatile ch_int_stat_en_t int_stat_en; /* CHx_INTSTATUS_ENABLEREG */
    volatile ch_int_stat_t
        int_stat; /* Channelx Interrupt Status Register captures the Channelx
                     specific interrupts */
    volatile dmac_ch_int_sig_en_t
        int_sig_en; /* This register contains fields that are used to enable the
                       generation of port level interrupt at the channel level.
                     */
    volatile ch_int_clr_t
        int_clr; /* Writing 1 to specific field will clear the corresponding
                    field in Channelx Interrupt Status Register */
    volatile uint64_t rsvd[12];
} dmac_ch_t;

typedef struct {
    volatile uint64_t dmac_id;        /* 0x0 */
    volatile uint32_t dmac_comp_ver;  /* 0x8 Component Version Register. */
    volatile uint32_t rsvd_0;         /* 0xc */
    volatile dmac_cfg_t dmac_cfg;     /* 0x10 */
    volatile dmac_ch_en_t dmac_ch_en; /* 0x18 */
    volatile uint64_t
        dmac_ch_susp; /* 0x20 This is DW_axi_dmac Channel Suspend Register. */
    volatile uint64_t dmac_ch_abort; /* 0x28 Channel Abort Register. */
    volatile dmac_int_stat_t
        dmac_int_stat; /* 0x30 DMAC Interrupt Status Register */
    volatile dmac_comm_int_clr_t
        dmac_comm_int_clr; /* 0x38 DMAC_COMMONREG_INTCLEARREG */
    volatile dmac_comm_int_stat_en_t
        dmac_comm_int_stat_en; /* 0x40 DMAC_COMMONREG_INTSTATUS_ENABLEREG */
    volatile dmac_comm_int_sig_en_t
        dmac_comm_int_sig_en; /* 0x48 MAC_COMMONREG_INTSIGNAL_ENABLEREG */
    volatile dmac_comm_int_stat_t
        dmac_comm_int_stat;            /* 0x50 DMAC_COMMONREG_INTSTATUSREG */
    volatile dmac_reset_t dmac_reset;  /* 0x58 DMAC_RESETREG */
    volatile uint64_t dmac_low_pm_cfg; /* 0x60 DMAC_LOWPOWER_CFGREG */
    volatile uint64_t rsvd_2[19];      /* 0x68 */
    volatile dmac_ch_t ch[DW_DMAC_CHANNEL_NUMB];
} dw_dmac_t;

typedef union {
    struct {
        volatile uint32_t wr_hdsk : 8;         /* RW */
        volatile uint32_t updated_wr_hdsk : 8; /* RO write */
        volatile uint32_t rd_hdsk : 8;         /* RW */
        volatile uint32_t updated_rd_hdsk : 8; /* RO wrtie */
    };
    volatile uint32_t v;
} dmac_mux_ch_t;

typedef union {
    struct {
        volatile uint32_t en : 1; /* DMA mux enable mux. */
        volatile uint32_t rsvd : 31;
    };
    volatile uint32_t v;
} dmac_mux_en_t;

typedef struct {
    volatile dmac_mux_ch_t ch[DW_DMAC_CHANNEL_NUMB];
    volatile dmac_mux_en_t en_reg;
} dmac_mux_t;

void dw_dmac_init_mux(paddr_t mux_base, uint32_t chan);
void dw_dmac_set_cfg(dmac_coeff_t cfg);
void dw_dmac_enable_ch(paddr_t dma_base, uint32_t chan);
bool dw_dmac_disable_ch(dmac_coeff_t cfg);
void dw_dmac_reset(paddr_t dma_base);
void dw_dmac_enable_irq(paddr_t dma_base);
void dw_dmac_disable_irq(paddr_t dma_base);
uint32_t dw_dmac_get_ch_int_stat(paddr_t dma_base, uint32_t chan);
bool dw_dmac_ch_has_int_status(paddr_t dma_base, uint32_t chan);
void dw_dmac_clr_ch_int_stat(paddr_t dma_base, uint32_t chan, uint32_t status);
bool dw_dmac_ch_tr_done(paddr_t dma_base, uint32_t chan);
dma_status_e dw_dmac_irq_status_handle(uint32_t status);
void dw_dmac_get_ctrl_cfg(dmac_coeff_t *cfg, ch_ctl_t *ctrl_cfg);
void dw_dmac_set_int_stat(dmac_coeff_t *cfg, bool en, uint8_t type);

#endif