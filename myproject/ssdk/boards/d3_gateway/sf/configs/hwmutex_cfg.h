/*
 * hwmutex_cfg.h
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#ifndef BOARD_HWMUTEX_CFG_H
#define BOARD_HWMUTEX_CFG_H

#ifdef CONFIG_HWMUTEX

#include <driver/taishan/hwmutex/taishan_hwmutex.h>

extern struct hwmutex_device g_hwmutex_dev;

extern void board_hwmutex_init(void);

#endif

#endif