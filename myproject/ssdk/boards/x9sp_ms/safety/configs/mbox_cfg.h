/*
 * mbox_cfg.h
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#ifndef BOARD_MBOX_CFG_H
#define BOARD_MBOX_CFG_H

#ifdef CONFIG_MBOX

#include <driver/kunlun/mbox/sdrv_mbox_drv.h>

extern mbox_dev_t *g_mbox_dev;

extern void board_mbox_init(void);

extern unsigned int board_mbox_get_coreid(void);

#endif

#endif