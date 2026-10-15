
//*****************************************************************************
//
// WARNING: Automatically generated file, don't modify anymore!!!
//
// Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
// Software License Agreement
//
//*****************************************************************************

#ifdef CONFIG_BOARD_REMAP
#include <stdlib.h>
#include <stdio.h>
#include <types.h>
#include <param.h>
#include <reg.h>
#include <arch/barriers.h>
#include <regs_base.h>
#include "rstgen_cfg.h"
#include "scr_hw.h"
#include "scr_cfg.h"
#include "remap_cfg.h"

static scr_signal_t sf_remap_scr[] = {
    SCR_SF_REMAP_CR5_SF_AR_ADDR_OFFSET_19_0,
    SCR_SF_REMAP_CR5_SF_AR_REMAP_EN,
    SCR_SF_REMAP_CR5_SF_AW_ADDR_OFFSET_19_0,
    SCR_SF_SCR_REMAP_CR5_SF_REMAP_EN,
};

static void remap_sf_core(paddr_t addr)
{
    scr_set(&g_scr_sf, &sf_remap_scr[0], addr >> 12);
    scr_set(&g_scr_sf, &sf_remap_scr[1], 1);
    scr_set(&g_scr_sf, &sf_remap_scr[2], addr >> 12);
    scr_set(&g_scr_sf, &sf_remap_scr[3], 1);
}

static void sf_remap(paddr_t addr)
{
    arch_irq_disable();
    remap_sf_core(addr);
    writel((readl(0xf0670004) | 0x1), 0xf0670004);
    DSB;
    ISB;
    reset_ctl_reset(&rstsig_cr5_saf);
    while(1);
}

void board_remap()
{
    if(!reset_ctl_general_read(&reset_general_reg_sf_remap)){
        reset_ctl_general_write(&reset_general_reg_sf_remap, REMAP_DONE);
        sf_remap(CONFIG_MEM_BASE);
    }
    else {
        reset_ctl_general_write(&reset_general_reg_sf_remap, 0);
    }
}

#endif
