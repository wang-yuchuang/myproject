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

#include <types.h>
#include <arch/register.h>

#include "board.h"
#include "mbox_cfg.h"

sdrv_mbox_dev_t  g_sdrv_mbox_dev;
mbox_dev_t *g_mbox_dev;

static const mbox_config_t sdrv_mbox_cfg = {
    .base = MB_REG_BASE,
    .mem_base = MB_MEM_BASE,
    .irq = MB_MU_MESSAGE_READY_INTR_NUM,
    .set_masterid_num = 5,
};

void board_mbox_init(void)
{
    g_mbox_dev = sdrv_mbox_probe(&g_sdrv_mbox_dev, &sdrv_mbox_cfg);
}

unsigned int board_mbox_get_coreid(void)
{
    uint32_t reg_val;
    uint32_t grp_id;
    uint32_t core_id;

    reg_val = arm_read_mpidr();
    grp_id = (reg_val >> 8) & 0xffu;
    core_id = reg_val & 0xffu;

    if (!grp_id) {
        return MBOX_RPROC_SF;
    }
    else if(grp_id == 1) {
        if (!core_id) {
            return MBOX_RPROC_SP0;
        }
        else {
            return MBOX_RPROC_SP1;
        }
    }
    else if (!core_id) {
        return MBOX_RPROC_SX0;
    }
    else {
        return MBOX_RPROC_SX1;
    }
}

#endif
