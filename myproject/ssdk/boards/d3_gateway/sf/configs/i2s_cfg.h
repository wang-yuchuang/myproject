/**
 * @file i2s_cfg.h
 * @brief
 * @version 0.1
 * @date 2021-12-04
 *
 * @copyright Copyright (c) 2021  Semidrive Semiconductor.
 *            All rights reserved.
 *
 */

#ifndef BOARDS_I2S_CFG_H
#define BOARDS_I2S_CFG_H

#ifdef CONFIG_SACI_I2S
#include <taishan/snd/i2s/sdrv_saci_i2s.h>

extern snd_dev_t *i2s1_dev;
extern snd_dev_t *i2s2_dev;

extern void board_i2s_init(void);

#endif

#endif /* BOARDS_I2S_CFG_H */
