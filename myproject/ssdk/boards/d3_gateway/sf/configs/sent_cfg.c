
//*****************************************************************************
//
// WARNING: Automatically generated file, don't modify anymore!!!
//
// Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
// Software License Agreement
//
//*****************************************************************************

#ifdef CONFIG_SENT

#include "sent_cfg.h"

#include <types.h>
#include <param.h>
#include <board.h>

#include <driver/sent.h>

/* sent device driver headers */
#include <driver/taishan/sent/sdrv_sent_drv.h>

/* sent device drivers */
sdrv_sent_dev_t g_sdrv_sent[SENT_DEV_NUM];
sent_dev_t *g_sent[SENT_DEV_NUM] = {NULL};


/* sent config structures */
static const sdrv_sent_config_t g_sent_cfg[] = {
#ifdef CONFIG_ETMR1
    {
    .base = APB_ETMR1_BASE,
    .irq = ETMR1_CHN_A_INTR_NUM, //A channel's irq num is same as B C D
    .hwid = 1,
    .chan_cfg_num = 4,

    .chan_cfg[0].chan_id = SENT_HW_CPT_A,
    .chan_cfg[0].tickfreq = 50000,
    .chan_cfg[0].fast_chan = SENT_STANDARD_FORMAT,
    .chan_cfg[0].data_bitnum = SENT_DATA_WIDTH_24_BIT,
    .chan_cfg[0].lowpulse_width = 5,
    .chan_cfg[0].fifowml = 7,
    .chan_cfg[0].pause_pulse = true,

    .chan_cfg[1].chan_id = SENT_HW_CPT_B,
    .chan_cfg[1].tickfreq = 50000,
    .chan_cfg[1].fast_chan = SENT_STANDARD_FORMAT,
    .chan_cfg[1].data_bitnum = SENT_DATA_WIDTH_24_BIT,
    .chan_cfg[1].lowpulse_width = 5,
    .chan_cfg[1].fifowml = 7,
    .chan_cfg[1].pause_pulse = true,

    .chan_cfg[2].chan_id = SENT_HW_CPT_C,
    .chan_cfg[2].tickfreq = 50000,
    .chan_cfg[2].fast_chan = SENT_STANDARD_FORMAT,
    .chan_cfg[2].data_bitnum = SENT_DATA_WIDTH_24_BIT,
    .chan_cfg[2].lowpulse_width = 5,
    .chan_cfg[2].fifowml = 7,
    .chan_cfg[2].pause_pulse = true,

    .chan_cfg[3].chan_id = SENT_HW_CPT_D,
    .chan_cfg[3].tickfreq = 50000,
    .chan_cfg[3].fast_chan = SENT_STANDARD_FORMAT,
    .chan_cfg[3].data_bitnum = SENT_DATA_WIDTH_24_BIT,
    .chan_cfg[3].lowpulse_width = 5,
    .chan_cfg[3].fifowml = 7,
    .chan_cfg[3].pause_pulse = true,
    },
#endif /* CONFIG_ETMR1 */
#ifdef CONFIG_ETMR2
    {
    .base = APB_ETMR2_BASE,
    .irq = ETMR2_CHN_A_INTR_NUM, //A channel's irq num is same as B C D
    .hwid = 2,
    .chan_cfg_num = 4,

    .chan_cfg[0].chan_id = SENT_HW_CPT_A,
    .chan_cfg[0].tickfreq = 50000,
    .chan_cfg[0].fast_chan = SENT_STANDARD_FORMAT,
    .chan_cfg[0].data_bitnum = SENT_DATA_WIDTH_24_BIT,
    .chan_cfg[0].lowpulse_width = 5,
    .chan_cfg[0].fifowml = 7,
    .chan_cfg[0].pause_pulse = true,

    .chan_cfg[1].chan_id = SENT_HW_CPT_B,
    .chan_cfg[1].tickfreq = 50000,
    .chan_cfg[1].fast_chan = SENT_STANDARD_FORMAT,
    .chan_cfg[1].data_bitnum = SENT_DATA_WIDTH_24_BIT,
    .chan_cfg[1].lowpulse_width = 5,
    .chan_cfg[1].fifowml = 7,
    .chan_cfg[1].pause_pulse = true,

    .chan_cfg[2].chan_id = SENT_HW_CPT_C,
    .chan_cfg[2].tickfreq = 50000,
    .chan_cfg[2].fast_chan = SENT_STANDARD_FORMAT,
    .chan_cfg[2].data_bitnum = SENT_DATA_WIDTH_24_BIT,
    .chan_cfg[2].lowpulse_width = 5,
    .chan_cfg[2].fifowml = 7,
    .chan_cfg[2].pause_pulse = true,

    .chan_cfg[3].chan_id = SENT_HW_CPT_D,
    .chan_cfg[3].tickfreq = 50000,
    .chan_cfg[3].fast_chan = SENT_STANDARD_FORMAT,
    .chan_cfg[3].data_bitnum = SENT_DATA_WIDTH_24_BIT,
    .chan_cfg[3].lowpulse_width = 5,
    .chan_cfg[3].fifowml = 7,
    .chan_cfg[3].pause_pulse = true,
    },
#endif /* CONFIG_ETMR2 */
};


/* sent initialization */
void board_sent_init(void)
{
    for (uint8_t i = 0; i < ARRAY_SIZE(g_sent_cfg); i++) {
        g_sent[i] = sdrv_sent_probe(&g_sdrv_sent[i], (sdrv_sent_config_t *)&g_sent_cfg[i]);
    }
}


#endif
