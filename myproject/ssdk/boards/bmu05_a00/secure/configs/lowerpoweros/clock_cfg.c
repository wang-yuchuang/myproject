
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
#include "stdio.h"
#include "clock_cfg.h"

static clk_core_t g_pll1_core = {
    .base = APB_PLL1_BASE,
    .ops = &sc_pfpll_ops,
};

static clk_core_t g_pll5_core = {
    .base = APB_PLL5_BASE,
    .ops = &sc_pfpll_ops,
};

static clk_core_t g_saf_ip_core = {
    .base = APB_CKGEN_SAF_BASE,
    .ops = &ckgen_ip_ops,
};

static clk_core_t g_sec_ip_core = {
    .base = APB_CKGEN_SEC_BASE,
    .ops = &ckgen_ip_ops,
};

static clk_core_t g_saf_gating_core = {
    .base = APB_CKGEN_SAF_BASE,
    .ops = &ckgen_lp_gating_ops,
};

static clk_core_t g_sec_gating_core = {
    .base = APB_CKGEN_SEC_BASE,
    .ops = &ckgen_lp_gating_ops,
};

clk_node_t g_rc_24m = {
    .core = NULL,
    .id = 0x0,
    .rate = 24000000,
    .mux = 0,
    .parent = NULL,
};

clk_node_t g_pll1_vco = {
    .core = &g_pll1_core,
    .id = 0x0,
    .rate = 2000000000,
    .mux = 0,
    .parent = CLK_NODE(g_rc_24m),
};

clk_node_t g_pll5_vco = {
    .core = &g_pll5_core,
    .id = 0x0,
    .rate = 1592000000,
    .mux = 0,
    .parent = CLK_NODE(g_rc_24m),
};

clk_node_t g_pll1_div_b = {
    .core = &g_pll1_core,
    .id = 0x3,
    .rate = 333333300,
    .mux = 0,
    .parent = CLK_NODE(g_pll1_vco),
};

clk_node_t g_pll5_div_a = {
    .core = &g_pll5_core,
    .id = 0x2,
    .rate = 398000000,
    .mux = 0,
    .parent = CLK_NODE(g_pll5_vco),
};

clk_node_t g_ckgen_ip_ospi1 = {
    .core = &g_saf_ip_core,
    .id = 0xb,
    .rate = 333333000,
    .mux = 4,
    .parent = CLK_NODE(g_pll1_div_b),
};

clk_node_t g_ckgen_ip_ospi2 = {
    .core = &g_sec_ip_core,
    .id = 0x15,
    .rate = 398000000,
    .mux = 7,
    .parent = CLK_NODE(g_pll5_div_a),
};

clk_node_t g_ckgen_ip_emmc1 = {
    .core = &g_sec_ip_core,
    .id = 0x9,
    .rate = 398000000,
    .mux = 4,
    .parent = CLK_NODE(g_pll5_div_a),
};

clk_node_t g_ckgen_ip_emmc2 = {
    .core = &g_sec_ip_core,
    .id = 0xa,
    .rate = 398000000,
    .mux = 4,
    .parent = CLK_NODE(g_pll5_div_a),
};

clk_node_t g_ckgen_ip_emmc3 = {
    .core = &g_sec_ip_core,
    .id = 0xb,
    .rate = 398000000,
    .mux = 4,
    .parent = CLK_NODE(g_pll5_div_a),
};

clk_node_t g_ckgen_ip_emmc4 = {
    .core = &g_sec_ip_core,
    .id = 0xc,
    .rate = 199000000,
    .mux = 4,
    .parent = CLK_NODE(g_pll5_div_a),
};

clk_node_t g_ckgen_gating_ospi1 = {
    .core = &g_saf_gating_core,
    .id = 0x9,
};

clk_node_t g_ckgen_gating_ospi2 = {
    .core = &g_sec_gating_core,
    .id = 0x11,
};

clk_node_t g_ckgen_gating_emmc1 = {
    .core = &g_sec_gating_core,
    .id = 0xc,
};

clk_node_t g_ckgen_gating_emmc2 = {
    .core = &g_sec_gating_core,
    .id = 0xd,
};

clk_node_t g_ckgen_gating_emmc3 = {
    .core = &g_sec_gating_core,
    .id = 0xe,
};

clk_node_t g_ckgen_gating_emmc4 = {
    .core = &g_sec_gating_core,
    .id = 0xf,
};

void board_clk_init(void)
{

}

#endif
