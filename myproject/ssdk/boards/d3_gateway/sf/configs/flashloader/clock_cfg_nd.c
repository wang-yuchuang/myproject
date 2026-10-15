
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

#include "clock_cfg.h"

static clk_core_t g_pll1_core = {
    .base = APB_PLL1_BASE,
    .ops = &taishan_pll_ops,
};

static clk_core_t g_pll2_core = {
    .base = APB_PLL2_BASE,
    .ops = &taishan_pll_ops,
};

static clk_core_t g_pll3_core = {
    .base = APB_PLL3_BASE,
    .ops = &taishan_pll_ops,
};

static clk_core_t g_sf_ip_core = {
    .base = APB_CKGEN_SF_BASE,
    .ops = &ckgen_ip_ops,
};

taishan_pll_node_t g_rc_24m = {
    .clk_node = {
        .core = NULL,
        .id = 0x0,
        .rate = 24000000,
        .parent = NULL,
    },
};

taishan_pll_node_t g_fs_24m = {
    .clk_node = {
        .core = NULL,
        .id = 0x0,
        .rate = 24000000,
        .parent = NULL,
    },
};

taishan_pll_node_t g_pll1_root = {
    .clk_node = {
        .core = &g_pll1_core,
        .id = 0x0,
        .rate = 500000000,
        .parent = CLK_NODE(g_rc_24m),
    },
};

taishan_pll_node_t g_pll2_root = {
    .clk_node = {
        .core = &g_pll2_core,
        .id = 0x0,
        .rate = 600000000,
        .parent = CLK_NODE(g_rc_24m),
    },
};

taishan_pll_node_t g_pll3_root = {
    .clk_node = {
        .core = &g_pll3_core,
        .id = 0x0,
        .rate = 667000000,
        .parent = CLK_NODE(g_rc_24m),
    },
};

taishan_ckgen_node_t g_ckgen_ip_uart_sf_1_to_8 = {
    .clk_node = {
        .core = &g_sf_ip_core,
        .id = 0x4,
        .rate = 83380000,
        .mux = 3,
        .parents_num = 5,
        .parents[0] = CLK_NODE(g_rc_24m),
        .parents[1] = CLK_NODE(g_fs_24m),
        .parents[2] = CLK_NODE(g_pll1_root),
        .parents[3] = CLK_NODE(g_pll3_root),
        .parents[4] = CLK_NODE(g_pll2_root),
    },
};

taishan_ckgen_node_t g_ckgen_ip_xspi1a = {
    .clk_node = {
        .core = &g_sf_ip_core,
        .id = 0xd,
        .rate = 250000000,
        .mux = 2,
        .parents_num = 5,
        .parents[0] = CLK_NODE(g_rc_24m),
        .parents[1] = CLK_NODE(g_fs_24m),
        .parents[2] = CLK_NODE(g_pll1_root),
        .parents[3] = CLK_NODE(g_pll3_root),
        .parents[4] = CLK_NODE(g_pll2_root),
    },
};

const clk_node_t *board_clk_prepare_array[] = {
    CLK_NODE(g_pll1_root),
    CLK_NODE(g_pll3_root),
    CLK_NODE(g_ckgen_ip_uart_sf_1_to_8),
    CLK_NODE(g_ckgen_ip_xspi1a),
};

void board_clk_init(void)
{
    /* Prepare clock tree. */
    for (uint32_t i = 0; i < ARRAY_SIZE(board_clk_prepare_array); i++) {
        clk_prepare((clk_node_t*)board_clk_prepare_array[i]);
    }
}

#endif