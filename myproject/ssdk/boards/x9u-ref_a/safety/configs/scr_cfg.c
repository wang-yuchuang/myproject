
//*****************************************************************************
//
// WARNING: Automatically generated file, don't modify anymore!!!
//
// Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
// Software License Agreement
//
//*****************************************************************************

#ifdef CONFIG_SCR

#include <regs_base.h>
#include "scr_cfg.h"
/* scr device driver headers */
#include <driver/scr.h>

/* scr device */
scr_dev_t g_scr_safety = {
    .base = APB_SCR_SAF_BASE,
    .lock = SPIN_LOCK_INITIAL_VALUE,
    .reg_shift = 10,
    .reg_off = {
        [TYPE_RW] = 0,
        [TYPE_RO] = 0x100,
        [TYPE_L16] = 0x200,
        [TYPE_L31] = 0x400,
        [TYPE_R16W16] = 0x600,
    },
};

scr_dev_t g_scr_sec = {
    .base = APB_SCR_SEC_BASE,
    .lock = SPIN_LOCK_INITIAL_VALUE,
    .reg_shift = 10,
    .reg_off = {
        [TYPE_RW] = 0,
        [TYPE_RO] = 0x100,
        [TYPE_L16] = 0x200,
        [TYPE_L31] = 0x400,
        [TYPE_R16W16] = 0x600,
    },
};

scr_dev_t g_scr_hpi = {
    .base = APB_SCR_HPI_BASE,
    .lock = SPIN_LOCK_INITIAL_VALUE,
    .reg_shift = 0,
    .reg_off = {
        [TYPE_RW] = 0,
        [TYPE_RO] = 0x100,
        [TYPE_L16] = 0x200,
        [TYPE_L31] = 0x400,
        [TYPE_R16W16] = 0x600,
    },
};


#endif
