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
#include "taishan/snd/codecs/ak4619.h"

codec_dev_t *codec_ak4619;
ak4619_dev_t ak4619;

extern i2c_adap_dev_t *i2c7_adap_dev;
extern gpio_dev_t g_gpio_safety;

static ak4619_cfg_t ak4619_cfg = {
    .gpio_dev = &g_gpio_safety,
    .gpio_pdn = GPIO_M1,
    .clk_rate = 12288000,
    .codec_addr = 0x10,
};

/**
 * @brief
 *
 */
void board_codec_init(void)
{
    ak4619_cfg.i2c_handle = i2c7_adap_dev;

    codec_ak4619 = ak4619_probe(&ak4619, &ak4619_cfg);
}

#endif
