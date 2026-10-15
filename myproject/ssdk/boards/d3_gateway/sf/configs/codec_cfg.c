/**
 * @file codec_cfg.c
 * @version 0.1
 * @date 2022-03-16
 *
 * @copyright Copyright (c) 2021  Semidrive Semiconductor.
 *            All rights reserved.
 *
 */

#ifdef CONFIG_CODEC

#include <board.h>
#include <types.h>

#include "codec_cfg.h"

#include "driver/i2c.h"
#include "driver/taishan/pinctrl/taishan_pinctrl.h"
#include "taishan/snd/codecs/tlv320.h"

codec_dev_t *codec_tlv320;
tlv320_dev_t tlv320;

extern i2c_adap_dev_t *i2c6_adap_dev;

static tlv320_cfg_t tlv320_cfg = {
    .clk_rate = 12288000,
    .codec_addr = 0x1a,
};

/**
 * @brief
 *
 */
void board_codec_init(void)
{
    tlv320_cfg.i2c_handle = i2c6_adap_dev;

    codec_tlv320 = tlv320_probe(&tlv320, &tlv320_cfg);
}

#endif
