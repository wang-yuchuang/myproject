/*
 * hwmutex_cfg.c
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#ifdef CONFIG_HWMUTEX

#include <types.h>

#include "board.h"
#include "hwmutex_cfg.h"

struct hwmutex_device g_hwmutex_dev;

void board_hwmutex_init(void)
{
    taishan_hwmutex_probe(&g_hwmutex_dev,
        MB_REG_BASE, MB_SEMAPHORE_LOCK_STATUS_CHANGE_INTR_NUM, 0, 1);
}

#endif
