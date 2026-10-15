
//*****************************************************************************
//
// WARNING: Automatically generated file, don't modify anymore!!!
//
// Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
// Software License Agreement
//
//*****************************************************************************

#ifndef CLOCK_CFG_ND_H
#define CLOCK_CFG_ND_H

#ifdef CONFIG_CLK

#include <driver/taishan/clk/taishan_pll_drv.h>
#include <driver/taishan/clk/taishan_ckgen_drv.h>

extern taishan_pll_node_t g_rc_24m;
extern taishan_pll_node_t g_fs_24m;
extern taishan_pll_node_t g_pll1_root;
extern taishan_pll_node_t g_pll2_root;
extern taishan_pll_node_t g_pll3_root;
extern taishan_ckgen_node_t g_ckgen_ip_uart_sf_1_to_8;
extern taishan_ckgen_node_t g_ckgen_ip_xspi1a;

extern void board_clk_init(void);


#endif

#endif
