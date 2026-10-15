/**
 * @file saci_pdm_regs.h
 * @version 0.1
 * @date 2022-03-16
 *
 * @copyright Copyright (c) 2022  Semidrive Semiconductor.
 *            All rights reserved.
 *
 */

#ifndef SCAI_PDM_REGS_H_
#define SCAI_PDM_REGS_H_

#include <taishan/snd/pdm/sdrv_saci_pdm.h>
#include <types.h>

#include "../i2s/saci_regs.h"

#define PDM_0000_SACI_CTRL SACI_0000_SACI_CTRL

#define PDM_CHX_FIFO_FLUSH_POS(id)                                             \
    (id ? SACI_0000_PDM_CH1_FIFO_FLUSH_POS : SACI_0000_PDM_CH0_FIFO_FLUSH_POS)
#define PDM_CHX_FIFO_FLUSH_MASK(id)                                            \
    (id ? SACI_0000_PDM_CH1_FIFO_FLUSH_MASK : SACI_0000_PDM_CH0_FIFO_FLUSH_MASK)

#define PDM_CHX_RST_POS(id)                                                    \
    (id ? SACI_0000_PDM_CH1_RST_POS : SACI_0000_PDM_CH0_RST_POS)
#define PDM_CHX_RST_MASK(id)                                                   \
    (id ? SACI_0000_PDM_CH1_RST_MASK : SACI_0000_PDM_CH0_RST_MASK)

#define PDM_0004_IRQ_MASK SACI_0004_SACI_IRQ_MASK

#define PDM_CHX_FIFO_PRE_FULL_POS(id)                                          \
    (id ? SACI_0004_PDM_CH1_FIFO_PRE_FULL_POS                                  \
        : SACI_0004_PDM_CH0_FIFO_PRE_FULL_POS)
#define PDM_CHX_FIFO_PRE_FULL_MASK(id)                                         \
    (id ? SACI_0004_PDM_CH1_FIFO_PRE_FULL_MASK                                 \
        : SACI_0004_PDM_CH0_FIFO_PRE_FULL_MASK)
#define PDM_CHX_FIFO_EMPTY_POS(id)                                             \
    (id ? SACI_0004_PDM_CH1_FIFO_EMPTY_POS : SACI_0004_PDM_CH0_FIFO_EMPTY_POS)
#define PDM_CHX_FIFO_EMPTY_MASK(id)                                            \
    (id ? SACI_0004_PDM_CH1_FIFO_EMPTY_MASK : SACI_0004_PDM_CH0_FIFO_EMPTY_MASK)
#define PDM_CHX_FIFO_FULL_POS(id)                                              \
    (id ? SACI_0004_PDM_CH1_FIFO_FULL_POS : SACI_0004_PDM_CH0_FIFO_FULL_POS)
#define PDM_CHX_FIFO_FULL_MASK(id)                                             \
    (id ? SACI_0004_PDM_CH1_FIFO_FULL_MASK : SACI_0004_PDM_CH0_FIFO_FULL_MASK)
#define PDM_CHX_FIFO_OVR_POS(id)                                               \
    (id ? SACI_0004_PDM_CH1_FIFO_OVR_POS : SACI_0004_PDM_CH0_FIFO_OVR_POS)
#define PDM_CHX_FIFO_OVR_MASK(id)                                              \
    (id ? SACI_0004_PDM_CH1_FIFO_OVR_MASK : SACI_0004_PDM_CH0_FIFO_OVR_MASK)
#define PDM_CHX_WID_MISM_POS(id)                                               \
    (id ? SACI_0004_PDM_CH1_WID_MISM_POS : SACI_0004_PDM_CH0_WID_MISM_POS)
#define PDM_CHX_WID_MISM_MASK(id)                                              \
    (id ? SACI_0004_PDM_CH1_WID_MISM_MASK : SACI_0004_PDM_CH0_WID_MISM_MASK)

#define PDM_0008_IRQ_STAT SACI_0008_SACI_IRQ_STAT
#define PDM_IRQ_STAT_CHX_FIFO_PRE_FULL_POS(id)                                 \
    (id ? SACI_0008_PDM_CH1_FIFO_PRE_FULL_POS                                  \
        : SACI_0008_PDM_CH0_FIFO_PRE_FULL_POS)
#define PDM_IRQ_STAT_CHX_FIFO_PRE_FULL_MASK(id)                                \
    (id ? SACI_0008_PDM_CH1_FIFO_PRE_FULL_MASK                                 \
        : SACI_0008_PDM_CH0_FIFO_PRE_FULL_MASK)
#define PDM_IRQ_STAT_CHX_FIFO_EMPTY_POS(id)                                    \
    (id ? SACI_0008_PDM_CH1_FIFO_EMPTY_POS : SACI_0008_PDM_CH0_FIFO_EMPTY_POS)
#define PDM_IRQ_STAT_CHX_FIFO_EMPTY_MASK(id)                                   \
    (id ? SACI_0008_PDM_CH1_FIFO_EMPTY_MASK : SACI_0008_PDM_CH0_FIFO_EMPTY_MASK)
#define PDM_IRQ_STAT_CHX_FIFO_FULL_POS(id)                                     \
    (id ? SACI_0008_PDM_CH1_FIFO_FULL_POS : SACI_0008_PDM_CH0_FIFO_FULL_POS)
#define PDM_IRQ_STAT_CHX_FIFO_FULL_MASK(id)                                    \
    (id ? SACI_0008_PDM_CH1_FIFO_FULL_MASK : SACI_0008_PDM_CH0_FIFO_FULL_MASK)
#define PDM_IRQ_STAT_CHX_FIFO_OVR_POS(id)                                      \
    (id ? SACI_0008_PDM_CH1_FIFO_OVR_POS : SACI_0008_PDM_CH0_FIFO_OVR_POS)
#define PDM_IRQ_STAT_CHX_FIFO_OVR_MASK(id)                                     \
    (id ? SACI_0008_PDM_CH1_FIFO_OVR_MASK : SACI_0008_PDM_CH0_FIFO_OVR_MASK)

#define PDM_CLK_X_CTRL(id)                                                     \
    (id ? SACI_5004_SACI_PDM_CLK_1_CTRL : SACI_5000_SACI_PDM_CLK_0_CTRL)
#define PDM_CLK_DIV_POS (0x10)
#define PDM_CLK_DIV_MASK (0xfff0000)
#define PDM_DIV_CLK_EN_POS (0x5)
#define PDM_DIV_CLK_EN_MASK (0x20)
#define PDM_CLK_O_EN_POS (0x4)
#define PDM_CLK_O_EN_MASK (0x10)
#define PDM_CLK_POL_POS (0x3)
#define PDM_CLK_POL_MASK (0x8)
#define PDM_CLK_SEL_POS (0x0)
#define PDM_CLK_SEL_MASK (0x7)

#define PDM_CTRL_X(id)                                                         \
    (id ? SACI_500C_SACI_PDM_CTRL_1 : SACI_5008_SACI_PDM_CTRL_0)

#define PDM_GAIN_EN_POS (0x1f)
#define PDM_GAIN_EN_MASK (0x80000000)
#define PDM_GAIN_VALUE_POS (0x18)
#define PDM_GAIN_VALUE_MASK (0x1f000000)
#define PDM_DMA_EN_POS (0x10)
#define PDM_DMA_EN_MASK (0x10000)
#define PDM_MINUS_MID_EN_POS (0x2)
#define PDM_MINUS_MID_EN_MASK (0x4)
#define PDM_MODE_POS (0x0)
#define PDM_MODE_MASK (0x3)

#define PDM_FIFO_CTRL_X(id)                                                    \
    (id ? SACI_5014_SACI_PDM_FIFO_CTRL_1 : SACI_5010_SACI_PDM_FIFO_CTRL_0)
#define PDM_FIFO_PACK_WIDTH_POS (0x1c)
#define PDM_FIFO_PACK_WIDTH_MASK (0x70000000)
#define PDM_THRD_POS (0x10)
#define PDM_THRD_MASK (0xfff0000)
#define PDM_DATA_WITH_POS (0x9)
#define PDM_DATA_WITH_MASK (0x7e00)
#define PDM_TEST_EN_POS (0x8)
#define PDM_TEST_EN_MASK (0x100)
#define PDM_FIFO16BIT_VLD_POS (0x4)
#define PDM_FIFO16BIT_VLD_MASK (0x10)
#define PDM_DATA_ALIGN_POS (0x1)
#define PDM_DATA_ALIGN_MASK (0x2)
#define PDM_DATA_PACK_MODE_POS (0x0)
#define PDM_DATA_PACK_MODE_MASK (0x1)

#define PDM_FIFO_STA_X(id)                                                     \
    (id ? SACI_501C_SACI_PDM_FIFO_STA_1 : SACI_5018_SACI_PDM_FIFO_STA_0)
#define PDM_FIFO_DPT_POS (0x10)
#define PDM_FIFO_DPT_MASK (0xfff0000)
#define PDM_VLD_DATA_WID_POS (0x8)
#define PDM_VLD_DATA_WID_MASK (0x3f00)
#define PDM_FULL_POS (0x1)
#define PDM_FULL_MASK (0x2)
#define PDM_EMPTY_POS (0x0)
#define PDM_EMPTY_MASK (0x1)

#define PDM_FILTER_CTRL_X(id)                                                  \
    (id ? SACI_5024_SACI_PDM_FILTER_CTRL_1 : SACI_5020_SACI_PDM_FILTER_CTRL_0)
#define PDM_DEC_RATE_POS (0x10)
#define PDM_DEC_RATE_MASK (0x3ff0000)
#define PDM_FLT_ORDER_POS (0x0)
#define PDM_FLT_ORDER_MASK (0x7)

#define PDM_EN_X(id) (id ? SACI_5034_SACI_PDM_EN_1 : SACI_5030_SACI_PDM_EN_0)
#define PDM_FIFO_EN_POS (0x1)
#define PDM_FIFO_EN_MASK (0x2)

#define PDM_EN_POS (0x0)
#define PDM_EN_MASK (0x1)

#define PDM_MID_VALUE SACI_5040_SACI_PDM_MID
#define PDM_MID_VALUE_POS SACI_5040_SACI_PDM_MID_VALUE_POS
#define PDM_MID_VALUE_MASK SACI_5040_SACI_PDM_MID_VALUE_MASK

#define PDM_CHX_FIFO_DATA(id)                                                  \
    (id ? SACI_6800_SACI_PDM_CH1_FIFO_DATA : SACI_6000_SACI_PDM_CH0_FIFO_DATA)
#define PDM_CHX_FIFO_DATA_POS (0x0)
#define PDM_CHX_FIFO_DATA_MASK (0xffffffff)

#define CLK_DIVIDER_SOURCE_PDM_PER_CLK 4

/* filter order  */
#define PDM_FILTER_BYPASSORDER 0x00
#define PDM_FILTER_1ORDER 0x01
#define PDM_FILTER_2ORDER 0x02
#define PDM_FILTER_3ORDER 0x03
#define PDM_FILTER_4ORDER 0x04
#define PDM_FILTER_5ORDER 0x05

/* decimation rate  */
#define PDM_DEC_RATE_64 64
#define PDM_DEC_RATE_128 128
#define PDM_DEC_RATE_256 256

/* pdm data with  */
#define PDM_DATA_WITH_22BITS 22
#define PDM_DATA_WITH_25BITS 25
#define PDM_DATA_WITH_29BITS 29
#define PDM_DATA_WITH_31BITS 31

/* pdm gain value */
#define PDM_GAIN_VALUE_N3DB 0x01
#define PDM_GAIN_VALUE_N6DB 0x02
#define PDM_GAIN_VALUE_N9DB 0x03
#define PDM_GAIN_VALUE_N12DB 0x04
#define PDM_GAIN_VALUE_0DB 0x10
#define PDM_GAIN_VALUE_P3DB 0x11
#define PDM_GAIN_VALUE_P6DB 0x12
#define PDM_GAIN_VALUE_P9DB 0x13
#define PDM_GAIN_VALUE_P12DB 0x14

#endif
