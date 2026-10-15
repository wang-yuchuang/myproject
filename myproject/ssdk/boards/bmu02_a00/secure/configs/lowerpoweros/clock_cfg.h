
//*****************************************************************************
//
// WARNING: Automatically generated file, don't modify anymore!!!
//
// Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
// Software License Agreement
//
//*****************************************************************************

#ifndef CLOCK_CFG_H
#define CLOCK_CFG_H

#ifdef CONFIG_CLK

#include <driver/kunlun/clk/sc_pfpll_drv.h>
#include <driver/kunlun/clk/kunlun_ckgen_drv.h>

extern clk_node_t g_rc_24m;

extern clk_node_t g_pll1_vco;

extern clk_node_t g_pll5_vco;

extern clk_node_t g_pll1_div_b;

extern clk_node_t g_pll5_div_a;

extern clk_node_t g_ckgen_ip_ospi1;

extern clk_node_t g_ckgen_ip_ospi2;

extern clk_node_t g_ckgen_ip_emmc1;

extern clk_node_t g_ckgen_ip_emmc2;

extern clk_node_t g_ckgen_ip_emmc3;

extern clk_node_t g_ckgen_ip_emmc4;

extern clk_node_t g_ckgen_gating_ospi1;

extern clk_node_t g_ckgen_gating_ospi2;

extern clk_node_t g_ckgen_gating_emmc1;

extern clk_node_t g_ckgen_gating_emmc2;

extern clk_node_t g_ckgen_gating_emmc3;

extern clk_node_t g_ckgen_gating_emmc4;

extern void board_clk_init(void);

#endif

#endif
