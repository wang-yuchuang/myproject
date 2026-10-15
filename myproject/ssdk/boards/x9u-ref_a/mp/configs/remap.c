
//*****************************************************************************
//
// WARNING: Automatically generated file, don't modify anymore!!!
//
// Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
// Software License Agreement
//
//*****************************************************************************

#ifdef CONFIG_BOARD_REMAP

#include <types.h>
#include <compiler.h>
#include <reg.h>

#include <driver/kunlun/reset/sdrv_reset_drv.h>
#include <driver/scr.h>
#include <scr_hw.h>
#include <rstgen_cfg.h>
#include <scr_cfg.h>
#include <regs_base.h>

#include "remap.h"

#define APB_RSTGEN_BASE         APB_RSTGEN_SEC_BASE
#define APB_ROMC_BASE           APB_SYS_CNT_RW_BASE

#define SCR_REMAP_ADDR_OFFSET   SCR_SEC__remap_cr5_mp_ar_addr_offset_19_0
#define SCR_REMAP_OVRD_EN       SCR_SEC__remap_cr5_mp_ar_remap_en

#define RSTGEN_CORE             &rstsig_cr5_mp

#define RSTGEN_GENERAL_REG(n)   ((0x50 + (n)*4) << 10)
#define RSTGEN_REMAP_STATUS_REG (APB_RSTGEN_BASE + RSTGEN_GENERAL_REG(2))
#define REMAP_DONE              (0x52454d50) /* 'REMP' */
#define ROMC_STICKY_REG         (APB_ROMC_BASE + 0x34)

static void vector_remap(addr_t vector_base)
{
    /* base address must be 4 KB aligned */
    //writel(REMAP_DONE, RSTGEN_REMAP_STATUS_REG);
    /* Enable R5 remapping to vector base. The remap config doesn't
     * take effect until REMAP module detects R5 core reset.
     */
    scr_signal_t remap_addr_signal = SCR_REMAP_ADDR_OFFSET;
    scr_signal_t remap_en_signal = SCR_REMAP_OVRD_EN;
    writel(REMAP_DONE, RSTGEN_REMAP_STATUS_REG);
    scr_set(&g_scr_sec, &remap_addr_signal, vector_base >> 12);
    scr_set(&g_scr_sec, &remap_en_signal, 1);
    /* ROMC Stick Reg
     *
     * [0]: ROMC_STICKY_REMAP_EN. This bit is AND'ed with
     *      SCR remap_en bit, to enable remapping.
     */
    RMWREG32(ROMC_STICKY_REG, 0, 1, 1);
}

void board_remap(void)
{
    if (readl((RSTGEN_REMAP_STATUS_REG)) != REMAP_DONE) {
        /* If configuring TCMA base as 0, no remap is needed. */
    #if CONFIG_ARMV7R_TCMA_BASE != 0
        vector_remap(CONFIG_MEM_BASE);
        reset_ctl_reset(RSTGEN_CORE);
        while (1) {
            __ASM volatile("wfi");
        }
    #endif
    } else {
        writel(0, (RSTGEN_REMAP_STATUS_REG));
    }
}

#endif