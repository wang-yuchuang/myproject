/**
 * @file lin_cfg.h
 *
 * Copyright (c) 2022 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#ifndef BOARDS_LIN_CFG_H
#define BOARDS_LIN_CFG_H

#include <driver/lin.h>

#define LIN_DEV_NUM 8

/* exported lin devices */
extern lin_dev_t *g_lin_dev[LIN_DEV_NUM];

/* lin init functions */
extern void board_lin_init(void);

#endif