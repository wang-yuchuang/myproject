
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
#include "scr_hw.h"

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

typedef struct {
    scr_dev_t* scr_dev;
    scr_signal_t signal;
    uint32_t access_mode;
}res_access_ctl_t;

#define AP_ACCESS   1
#define SAF_ACCESS  0

res_access_ctl_t s_access_ctl_tbl[] = {
    {&g_scr_safety, SCR_SAFETY__apbmux2_rtc2_select,           AP_ACCESS},
    {&g_scr_safety, SCR_SAFETY__apbmux2_sec_storage2_select,   AP_ACCESS},
    {NULL}
};


void board_scr_init()
{
    int32_t i = 0;

    while (s_access_ctl_tbl[i].scr_dev != NULL) {
        res_access_ctl_t* access_ctl_ptr = &s_access_ctl_tbl[i++];

        scr_set(access_ctl_ptr->scr_dev
                , &access_ctl_ptr->signal
                , access_ctl_ptr->access_mode);
    }
}

#endif
