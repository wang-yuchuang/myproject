
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
scr_dev_t g_scr_sf = {
    .base = APB_SCR_SF_BASE,
    .lock = SPIN_LOCK_INITIAL_VALUE,
    .reg_shift = 0,
    .reg_off = {
        [TYPE_RW] = 0x1000,
        [TYPE_RO] = 0x1100,
        [TYPE_L16] = 0x1200,
        [TYPE_L31] = 0x1400,
        [TYPE_R16W16] = 0x1600,
    },
};

scr_dev_t g_scr_ap = {
    .base = APB_SCR_AP_BASE,
    .lock = SPIN_LOCK_INITIAL_VALUE,
    .reg_shift = 0,
    .reg_off = {
        [TYPE_RW] = 0x1000,
        [TYPE_RO] = 0x1100,
        [TYPE_L16] = 0x1200,
        [TYPE_L31] = 0x1400,
        [TYPE_R16W16] = 0x1600,
    },
};


#endif
