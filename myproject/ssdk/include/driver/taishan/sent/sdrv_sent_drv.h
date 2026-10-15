/**
 * @file sdrv_sent_drv.h
 * @brief Taishan SENT head source.
 *
 * @copyright Copyright (c) 2022  Semidrive Semiconductor.
 *            All rights reserved.
 */

#ifndef SDRV_SENT_DRV_H_
#define SDRV_SENT_DRV_H_

#include <types.h>
#include <compiler.h>

#include "driver/sent.h"

#define SDRV_SENT_MOUDLE_NUM         2
#define SDRV_SENT_CHAN_NUM           4

/* sdrv sent source */
typedef enum
{
    SDRV_SENT_HF_CLK = 0,
    SDRV_SENT_AHF_CLK,
    SDRV_SENT_EXT_CLK,
    SDRV_SENT_LP_CLK,
} sdrv_sent_clk_src_e;

/* sdrv sent channel id */
typedef enum {
    SENT_HW_CPT_A  = 0U,
    SENT_HW_CPT_B  = 1U,
    SENT_HW_CPT_C  = 2U,
    SENT_HW_CPT_D  = 3U
} sdrv_sent_chan_num_e;

typedef struct sdrv_sent_chan {
    sdrv_sent_chan_num_e     chan_id;

    uint32_t                 tickfreq;
    uint8_t                  fifowml;

    void                     *priv;
} sdrv_sent_chan_t;

typedef struct sdrv_sent_dev {
    sent_dev_t              dev;
    uint8_t                 hwid;

    sdrv_sent_clk_src_e     clk_sel;
    uint8_t                 clk_div;
    uint32_t                clk_freq;

    sent_chan_t             sent_chan[SDRV_SENT_CHAN_NUM];
    sdrv_sent_chan_t        sdrv_sent_chan[SDRV_SENT_CHAN_NUM];
} sdrv_sent_dev_t;

typedef struct sdrv_sent_chan_config {
    sdrv_sent_chan_num_e    chan_id;

    uint32_t                tickfreq;  /* range : 12500 ~ 200000, match tick 5us ~ 80us */
    sent_fastchan_e         fast_chan;  /* default value is 0 */
    sent_bitnum_e           data_bitnum;  /* range : 1 ~ 6 */
    uint8_t                 lowpulse_width;
    uint8_t                 fifowml;  /* range : 1 ~ 16 */
    bool                    pause_pulse;  /* default value is false */
} sdrv_sent_chan_config_t;

typedef struct sdrv_sent_config {
    paddr_t                 base;
    uint32_t                irq;
    uint8_t                 hwid;

    uint8_t                 clk_sel;
    uint8_t                 clk_div;
    uint32_t                clk_freq;

    uint8_t                 chan_cfg_num;
    sdrv_sent_chan_config_t chan_cfg[SDRV_SENT_CHAN_NUM];
} sdrv_sent_config_t;

sent_dev_t *sdrv_sent_probe(sdrv_sent_dev_t *sdrv_sent, sdrv_sent_config_t *sent_chan_cfg);


#endif /* SDRV_SENT_DRV_H_ */
