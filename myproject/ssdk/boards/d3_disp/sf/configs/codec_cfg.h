/**
 * @file codec_cfg.h
 * @version 0.1
 * @date 2022-03-16
 *
 * @copyright Copyright (c) 2021  Semidrive Semiconductor.
 *            All rights reserved.
 *
 */
#ifndef BOARDS_CODEC_CFG_H
#define BOARDS_CODEC_CFG_H

#include <driver/taishan/snd/codec_driver.h>

extern codec_dev_t *codec_ak4619;

extern void board_codec_init(void);

#endif /* BOARDS_CODEC_CFG_H */
