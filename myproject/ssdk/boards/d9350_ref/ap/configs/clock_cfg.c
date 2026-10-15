
//*****************************************************************************
//
// WARNING: Automatically generated file, don't modify anymore!!!
//
// Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
// Software License Agreement
//
//*****************************************************************************

#ifdef CONFIG_CLK

#include <types.h>
#include <param.h>
#include <regs_base.h>

// #include <driver/kunlun/clk/sc_pfpll.h>
// #include <driver/kunlun/clk/kunlun_ckgen.h>
#include <driver/kunlun/clk/sc_pfpll_drv.h>
#include <driver/kunlun/clk/kunlun_ckgen_drv.h>

#include "clock_cfg.h"


#ifdef CONFIG_ARCH_WITH_SMP
static clk_core_t g_soc_gating_core = {
    .base = APB_CKGEN_SOC_BASE,
    .ops = &ckgen_lp_gating_ops,
};

/* cpu core gating */
clk_node_t g_ckgen_gating_cpu1_clk0 = {
    .core = &g_soc_gating_core,
    .id = 0x36,
};
clk_node_t g_ckgen_gating_cpu1_clk1 = {
    .core = &g_soc_gating_core,
    .id = 0x37,
};
clk_node_t g_ckgen_gating_cpu1_clk2 = {
    .core = &g_soc_gating_core,
    .id = 0x38,
};
clk_node_t g_ckgen_gating_cpu1_clk3 = {
    .core = &g_soc_gating_core,
    .id = 0x39,
};
#endif

void board_clk_init(void)
{
}

#endif
