/*
 * mbox_cfg.c
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#ifdef CONFIG_MBOX
#include <debug.h>
#include <types.h>
#include <arch/register.h>
#include <regs_base.h>

#include "board.h"
#include "mbox_cfg.h"

sdrv_mbox_dev_t  g_sdrv_mbox_dev;
mbox_dev_t *g_mbox_dev;

static const mbox_config_t sdrv_mbox_cfg = {
    .base = MB_REG_BASE,
    .mem_base = MB_MEM_MBMESSAGEBUFFER_BASE,
    .irq = MU_MU_MESSAGE_READY_INTR_NUM,
    .set_masterid_num = 8,
};

void board_mbox_init(void)
{
    g_mbox_dev = sdrv_mbox_probe(&g_sdrv_mbox_dev, &sdrv_mbox_cfg);
}

unsigned int board_mbox_get_coreid(void)
{
    return MBOX_RPROC_SEC;
}

#endif
