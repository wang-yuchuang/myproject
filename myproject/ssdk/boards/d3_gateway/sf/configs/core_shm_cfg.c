/*
 * core_shm_cfg.c
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#ifdef CONFIG_CORE_SHM

#include <stdlib.h>
#include <string.h>
#include "core_shm_cfg.h"

struct core_shm_param *g_coreshm;

void board_coreshm_init(void)
{
    if (sizeof(struct core_shm_param) <= CONFIG_CORE_SHM_SIZE) {
        g_coreshm = (struct core_shm_param *)CONFIG_CORE_SHM_BASE;
        memset(g_coreshm, 0x0, sizeof(struct core_shm_param));
    }
}

#endif
