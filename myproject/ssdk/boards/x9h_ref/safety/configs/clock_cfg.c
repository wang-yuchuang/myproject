
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
    .ops = &sc_pfpll_ops,
};

static clk_core_t g_pll2_core = {
    .base = APB_PLL2_BASE,
    .ops = &sc_pfpll_ops,
};

static clk_core_t g_saf_bus_core = {
    .base = APB_CKGEN_SAF_BASE,
    .ops = &ckgen_bus_ops,
};

static clk_core_t g_saf_ip_core = {
    .base = APB_CKGEN_SAF_BASE,
    .ops = &ckgen_ip_ops,
};

static clk_core_t g_saf_gating_core = {
    .base = APB_CKGEN_SAF_BASE,
    .ops = &ckgen_lp_gating_ops,
};

static clk_core_t g_pll3_core = {
    .base = APB_PLL3_BASE,
    .ops = &sc_pfpll_ops,
};

static clk_core_t g_pll4_core = {
    .base = APB_PLL4_BASE,
    .ops = &sc_pfpll_ops,
};

static clk_core_t g_pll5_core = {
    .base = APB_PLL5_BASE,
    .ops = &sc_pfpll_ops,
};

static clk_core_t g_pll6_core = {
    .base = APB_PLL6_BASE,
    .ops = &sc_pfpll_ops,
};

static clk_core_t g_pll7_core = {
    .base = APB_PLL7_BASE,
    .ops = &sc_pfpll_ops,
};

static clk_core_t g_sec_bus_core = {
    .base = APB_CKGEN_SEC_BASE,
    .ops = &ckgen_bus_ops,
};

static clk_core_t g_sec_ip_core = {
    .base = APB_CKGEN_SEC_BASE,
    .ops = &ckgen_ip_ops,
};

static clk_core_t g_sec_gating_core = {
    .base = APB_CKGEN_SEC_BASE,
    .ops = &ckgen_lp_gating_ops,
};

static clk_core_t g_pll_disp_core = {
    .base = APB_PLL_DISP_BASE,
    .ops = &sc_pfpll_ops,
};

static clk_core_t g_pll_lvds1_core = {
    .base = APB_PLL_LVDS1_BASE,
    .ops = &sc_pfpll_ops,
};

static clk_core_t g_pll_lvds2_core = {
    .base = APB_PLL_LVDS2_BASE,
    .ops = &sc_pfpll_ops,
};

static clk_core_t g_pll_lvds3_core = {
    .base = APB_PLL_LVDS3_BASE,
    .ops = &sc_pfpll_ops,
};

static clk_core_t g_pll_lvds4_core = {
    .base = APB_PLL_LVDS4_BASE,
    .ops = &sc_pfpll_ops,
};

static clk_core_t g_disp_ip_core = {
    .base = APB_CKGEN_DISP_BASE,
    .ops = &ckgen_ip_ops,
};

static clk_core_t g_disp_bus_core = {
    .base = APB_CKGEN_DISP_BASE,
    .ops = &ckgen_bus_ops,
};

static clk_core_t g_disp_gating_core = {
    .base = APB_CKGEN_DISP_BASE,
    .ops = &ckgen_lp_gating_ops,
};

static clk_core_t g_pll_cpu1a_core = {
    .base = APB_PLL_CPU1A_BASE,
    .ops = &sc_pfpll_ops,
};

static clk_core_t g_pll_cpu1b_core = {
    .base = APB_PLL_CPU1B_BASE,
    .ops = &sc_pfpll_ops,
};

static clk_core_t g_pll_gpu1_core = {
    .base = APB_PLL_GPU1_BASE,
    .ops = &sc_pfpll_ops,
};

static clk_core_t g_pll_gpu2_core = {
    .base = APB_PLL_GPU2_BASE,
    .ops = &sc_pfpll_ops,
};

static clk_core_t g_pll_vpu_core = {
    .base = APB_PLL_VPU_BASE,
    .ops = &sc_pfpll_ops,
};

static clk_core_t g_pll_vsn_core = {
    .base = APB_PLL_VSN_BASE,
    .ops = &sc_pfpll_ops,
};

static clk_core_t g_pll_hpi_core = {
    .base = APB_PLL_HPI_BASE,
    .ops = &sc_pfpll_ops,
};

static clk_core_t g_pll_his_core = {
    .base = APB_PLL_HIS_BASE,
    .ops = &sc_pfpll_ops,
};

static clk_core_t g_pll_ddr_core = {
    .base = APB_PLL_DDR_BASE,
    .ops = &sc_pfpll_ops,
};

static clk_core_t g_soc_core_core = {
    .base = APB_CKGEN_SOC_BASE,
    .ops = &ckgen_core_ops,
};

static clk_core_t g_soc_ip_core = {
    .base = APB_CKGEN_SOC_BASE,
    .ops = &ckgen_ip_ops,
};

static clk_core_t g_soc_bus_core = {
    .base = APB_CKGEN_SOC_BASE,
    .ops = &ckgen_bus_ops,
};

static clk_core_t g_soc_uuu_core = {
    .base = APB_CKGEN_SOC_BASE,
    .ops = &ckgen_uuu_ops,
};

static clk_core_t g_soc_gating_core = {
    .base = APB_CKGEN_SOC_BASE,
    .ops = &ckgen_lp_gating_ops,
};

clk_node_t g_rc_24m = {
    .core = NULL,
    .id = 0x0,
    .rate = 24000000,
    .mux = 0,
    .parent = NULL,
};
clk_node_t g_rc_rtc = {
    .core = NULL,
    .id = 0x0,
    .rate = 2000000,
    .mux = 0,
    .parent = NULL,
};
clk_node_t g_xtal_24m = {
    .core = NULL,
    .id = 0x0,
    .rate = 24000000,
    .mux = 0,
    .parent = NULL,
};
clk_node_t g_xtal_rtc = {
    .core = NULL,
    .id = 0x0,
    .rate = 32000,
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
clk_node_t g_pll1_root = {
    .core = &g_pll1_core,
    .id = 0x1,
    .rate = 1000000000,
    .mux = 0,
    .parent = CLK_NODE(g_pll1_vco),
};
clk_node_t g_pll1_div_a = {
    .core = &g_pll1_core,
    .id = 0x2,
    .rate = 500000000,
    .mux = 0,
    .parent = CLK_NODE(g_pll1_vco),
};
clk_node_t g_pll1_div_b = {
    .core = &g_pll1_core,
    .id = 0x3,
    .rate = 333333300,
    .mux = 0,
    .parent = CLK_NODE(g_pll1_vco),
};
clk_node_t g_pll1_div_c = {
    .core = &g_pll1_core,
    .id = 0x4,
    .rate = 250000000,
    .mux = 0,
    .parent = CLK_NODE(g_pll1_vco),
};
clk_node_t g_pll1_div_d = {
    .core = &g_pll1_core,
    .id = 0x5,
    .rate = 200000000,
    .mux = 0,
    .parent = CLK_NODE(g_pll1_vco),
};
clk_node_t g_pll2_vco = {
    .core = &g_pll2_core,
    .id = 0x0,
    .rate = 1600000000,
    .mux = 0,
    .parent = CLK_NODE(g_rc_24m),
};
clk_node_t g_pll2_root = {
    .core = &g_pll2_core,
    .id = 0x1,
    .rate = 800000000,
    .mux = 0,
    .parent = CLK_NODE(g_pll2_vco),
};
clk_node_t g_pll2_div_a = {
    .core = &g_pll2_core,
    .id = 0x2,
    .rate = 400000000,
    .mux = 0,
    .parent = CLK_NODE(g_pll2_vco),
};
clk_node_t g_pll2_div_b = {
    .core = &g_pll2_core,
    .id = 0x3,
    .rate = 266666700,
    .mux = 0,
    .parent = CLK_NODE(g_pll2_vco),
};
clk_node_t g_pll2_div_c = {
    .core = &g_pll2_core,
    .id = 0x4,
    .rate = 200000000,
    .mux = 0,
    .parent = CLK_NODE(g_pll2_vco),
};
clk_node_t g_pll2_div_d = {
    .core = &g_pll2_core,
    .id = 0x5,
    .rate = 160000000,
    .mux = 0,
    .parent = CLK_NODE(g_pll2_vco),
};
clk_node_t g_ext_aud3 = {
    .core = NULL,
    .id = 0x0,
    .rate = 199500000,
    .mux = 0,
    .parent = NULL,
};
clk_node_t g_ext_aud4 = {
    .core = NULL,
    .id = 0x0,
    .rate = 199500000,
    .mux = 0,
    .parent = NULL,
};
clk_node_t g_ckgen_bus_saf_plat_root = {
    .core = &g_saf_bus_core,
    .id = 0x0,
    .rate = 800000000,
    .mux = 5,
    .parent = CLK_NODE(g_pll2_root),
};
clk_node_t g_ckgen_bus_saf_plat_div_m = {
    .core = &g_saf_bus_core,
    .id = 0x4000000,
    .rate = 400000000,
    .mux = 5,
    .parent = CLK_NODE(g_pll2_root),
};
clk_node_t g_ckgen_bus_saf_plat_div_n = {
    .core = &g_saf_bus_core,
    .id = 0x3000000,
    .rate = 200000000,
    .mux = 5,
    .parent = CLK_NODE(g_pll2_root),
};
clk_node_t g_ckgen_bus_saf_plat_div_p = {
    .core = &g_saf_bus_core,
    .id = 0x2000000,
    .rate = 800000000,
    .mux = 5,
    .parent = CLK_NODE(g_pll2_root),
};
clk_node_t g_ckgen_bus_saf_plat_div_q = {
    .core = &g_saf_bus_core,
    .id = 0x1000000,
    .rate = 800000000,
    .mux = 5,
    .parent = CLK_NODE(g_pll2_root),
};
clk_node_t g_ckgen_ip_ce1 = {
    .core = &g_saf_ip_core,
    .id = 0x0,
    .rate = 400000000,
    .mux = 4,
    .parent = CLK_NODE(g_pll2_root),
};
clk_node_t g_ckgen_ip_i2c_saf = {
    .core = &g_saf_ip_core,
    .id = 0x1,
    .rate = 24000000,
    .mux = 2,
    .parent = CLK_NODE(g_xtal_24m),
};
clk_node_t g_ckgen_ip_spi_saf = {
    .core = &g_saf_ip_core,
    .id = 0x2,
    .rate = 133330000,
    .mux = 5,
    .parent = CLK_NODE(g_pll2_div_b),
};
clk_node_t g_ckgen_ip_uart_saf = {
    .core = &g_saf_ip_core,
    .id = 0x3,
    .rate = 80000000,
    .mux = 4,
    .parent = CLK_NODE(g_pll2_div_d),
};
clk_node_t g_ckgen_ip_i2s_mclk1 = {
    .core = &g_saf_ip_core,
    .id = 0x4,
    .rate = 12000000,
    .mux = 2,
    .parent = CLK_NODE(g_xtal_24m),
};
clk_node_t g_ckgen_ip_i2s_sc1 = {
    .core = &g_saf_ip_core,
    .id = 0x5,
    .rate = 400000000,
    .mux = 5,
    .parent = CLK_NODE(g_pll2_div_a),
};
clk_node_t g_ckgen_ip_i2s_sc2 = {
    .core = &g_saf_ip_core,
    .id = 0x6,
    .rate = 400000000,
    .mux = 5,
    .parent = CLK_NODE(g_pll2_div_a),
};
clk_node_t g_ckgen_ip_eth1_tx = {
    .core = &g_saf_ip_core,
    .id = 0x7,
    .rate = 250000000,
    .mux = 4,
    .parent = CLK_NODE(g_pll1_div_c),
};
clk_node_t g_ckgen_ip_eth1_rmii = {
    .core = &g_saf_ip_core,
    .id = 0x8,
    .rate = 50000000,
    .mux = 4,
    .parent = CLK_NODE(g_pll1_div_c),
};
clk_node_t g_ckgen_ip_eth1_phy_ref = {
    .core = &g_saf_ip_core,
    .id = 0x9,
    .rate = 125000000,
    .mux = 4,
    .parent = CLK_NODE(g_pll1_div_c),
};
clk_node_t g_ckgen_ip_eth1_timer_sec = {
    .core = &g_saf_ip_core,
    .id = 0xa,
    .rate = 500000000,
    .mux = 4,
    .parent = CLK_NODE(g_pll1_div_a),
};
clk_node_t g_ckgen_ip_ospi1 = {
    .core = &g_saf_ip_core,
    .id = 0xb,
    .rate = 333333000,
    .mux = 4,
    .parent = CLK_NODE(g_pll1_div_b),
};
clk_node_t g_ckgen_ip_timer1 = {
    .core = &g_saf_ip_core,
    .id = 0xc,
    .rate = 400000000,
    .mux = 4,
    .parent = CLK_NODE(g_pll2_div_a),
};
clk_node_t g_ckgen_ip_timer2 = {
    .core = &g_saf_ip_core,
    .id = 0xd,
    .rate = 400000000,
    .mux = 4,
    .parent = CLK_NODE(g_pll2_div_a),
};
clk_node_t g_ckgen_ip_pwm1 = {
    .core = &g_saf_ip_core,
    .id = 0xe,
    .rate = 400000000,
    .mux = 4,
    .parent = CLK_NODE(g_pll2_div_a),
};
clk_node_t g_ckgen_ip_pwm2 = {
    .core = &g_saf_ip_core,
    .id = 0xf,
    .rate = 400000000,
    .mux = 4,
    .parent = CLK_NODE(g_pll2_div_a),
};
clk_node_t g_ckgen_ip_can_1_to_4 = {
    .core = &g_saf_ip_core,
    .id = 0x10,
    .rate = 80000000,
    .mux = 4,
    .parent = CLK_NODE(g_pll2_div_d),
};
clk_node_t g_pll3_vco = {
    .core = &g_pll3_core,
    .id = 0x0,
    .rate = 1800000000,
    .mux = 0,
    .parent = CLK_NODE(g_rc_24m),
};
clk_node_t g_pll3_root = {
    .core = &g_pll3_core,
    .id = 0x1,
    .rate = 900000000,
    .mux = 0,
    .parent = CLK_NODE(g_pll3_vco),
};
clk_node_t g_pll3_div_a = {
    .core = &g_pll3_core,
    .id = 0x2,
    .rate = 600000000,
    .mux = 0,
    .parent = CLK_NODE(g_pll3_vco),
};
clk_node_t g_pll3_div_b = {
    .core = &g_pll3_core,
    .id = 0x3,
    .rate = 150000000,
    .mux = 0,
    .parent = CLK_NODE(g_pll3_vco),
};
clk_node_t g_pll3_div_c = {
    .core = &g_pll3_core,
    .id = 0x4,
    .rate = 120000000,
    .mux = 0,
    .parent = CLK_NODE(g_pll3_vco),
};
clk_node_t g_pll3_div_d = {
    .core = &g_pll3_core,
    .id = 0x5,
    .rate = 360000000,
    .mux = 0,
    .parent = CLK_NODE(g_pll3_vco),
};
clk_node_t g_pll4_vco = {
    .core = &g_pll4_core,
    .id = 0x0,
    .rate = 2000000000,
    .mux = 0,
    .parent = CLK_NODE(g_rc_24m),
};
clk_node_t g_pll4_root = {
    .core = &g_pll4_core,
    .id = 0x1,
    .rate = 1000000000,
    .mux = 0,
    .parent = CLK_NODE(g_pll4_vco),
};
clk_node_t g_pll4_div_a = {
    .core = &g_pll4_core,
    .id = 0x2,
    .rate = 500000000,
    .mux = 0,
    .parent = CLK_NODE(g_pll4_vco),
};
clk_node_t g_pll4_div_b = {
    .core = &g_pll4_core,
    .id = 0x3,
    .rate = 333333300,
    .mux = 0,
    .parent = CLK_NODE(g_pll4_vco),
};
clk_node_t g_pll4_div_c = {
    .core = &g_pll4_core,
    .id = 0x4,
    .rate = 250000000,
    .mux = 0,
    .parent = CLK_NODE(g_pll4_vco),
};
clk_node_t g_pll4_div_d = {
    .core = &g_pll4_core,
    .id = 0x5,
    .rate = 200000000,
    .mux = 0,
    .parent = CLK_NODE(g_pll4_vco),
};
clk_node_t g_pll5_vco = {
    .core = &g_pll5_core,
    .id = 0x0,
    .rate = 1592000000,
    .mux = 0,
    .parent = CLK_NODE(g_rc_24m),
};
clk_node_t g_pll5_root = {
    .core = &g_pll5_core,
    .id = 0x1,
    .rate = 796000000,
    .mux = 0,
    .parent = CLK_NODE(g_pll5_vco),
};
clk_node_t g_pll5_div_a = {
    .core = &g_pll5_core,
    .id = 0x2,
    .rate = 398000000,
    .mux = 0,
    .parent = CLK_NODE(g_pll5_vco),
};
clk_node_t g_pll5_div_b = {
    .core = &g_pll5_core,
    .id = 0x3,
    .rate = 265333300,
    .mux = 0,
    .parent = CLK_NODE(g_pll5_vco),
};
clk_node_t g_pll5_div_c = {
    .core = &g_pll5_core,
    .id = 0x4,
    .rate = 199000000,
    .mux = 0,
    .parent = CLK_NODE(g_pll5_vco),
};
clk_node_t g_pll5_div_d = {
    .core = &g_pll5_core,
    .id = 0x5,
    .rate = 159200000,
    .mux = 0,
    .parent = CLK_NODE(g_pll5_vco),
};
clk_node_t g_pll6_vco = {
    .core = &g_pll6_core,
    .id = 0x0,
    .rate = 1474560000,
    .mux = 0,
    .parent = CLK_NODE(g_rc_24m),
};
clk_node_t g_pll6_root = {
    .core = &g_pll6_core,
    .id = 0x1,
    .rate = 294912000,
    .mux = 0,
    .parent = CLK_NODE(g_pll6_vco),
};
clk_node_t g_pll6_div_a = {
    .core = &g_pll6_core,
    .id = 0x2,
    .rate = 491520000,
    .mux = 0,
    .parent = CLK_NODE(g_pll6_vco),
};
clk_node_t g_pll6_div_b = {
    .core = &g_pll6_core,
    .id = 0x3,
    .rate = 368640000,
    .mux = 0,
    .parent = CLK_NODE(g_pll6_vco),
};
clk_node_t g_pll6_div_c = {
    .core = &g_pll6_core,
    .id = 0x4,
    .rate = 294912000,
    .mux = 0,
    .parent = CLK_NODE(g_pll6_vco),
};
clk_node_t g_pll6_div_d = {
    .core = &g_pll6_core,
    .id = 0x5,
    .rate = 245760000,
    .mux = 0,
    .parent = CLK_NODE(g_pll6_vco),
};
clk_node_t g_pll7_vco = {
    .core = &g_pll7_core,
    .id = 0x0,
    .rate = 1083801600,
    .mux = 0,
    .parent = CLK_NODE(g_rc_24m),
};
clk_node_t g_pll7_root = {
    .core = &g_pll7_core,
    .id = 0x1,
    .rate = 270950400,
    .mux = 0,
    .parent = CLK_NODE(g_pll7_vco),
};
clk_node_t g_pll7_div_a = {
    .core = &g_pll7_core,
    .id = 0x2,
    .rate = 361267200,
    .mux = 0,
    .parent = CLK_NODE(g_pll7_vco),
};
clk_node_t g_pll7_div_b = {
    .core = &g_pll7_core,
    .id = 0x3,
    .rate = 270950400,
    .mux = 0,
    .parent = CLK_NODE(g_pll7_vco),
};
clk_node_t g_pll7_div_c = {
    .core = &g_pll7_core,
    .id = 0x4,
    .rate = 216760300,
    .mux = 0,
    .parent = CLK_NODE(g_pll7_vco),
};
clk_node_t g_pll7_div_d = {
    .core = &g_pll7_core,
    .id = 0x5,
    .rate = 180633600,
    .mux = 0,
    .parent = CLK_NODE(g_pll7_vco),
};
clk_node_t g_ext_aud1 = {
    .core = NULL,
    .id = 0x0,
    .rate = 199500000,
    .mux = 0,
    .parent = NULL,
};
clk_node_t g_ext_aud2 = {
    .core = NULL,
    .id = 0x0,
    .rate = 199500000,
    .mux = 0,
    .parent = NULL,
};
clk_node_t g_ckgen_bus_sec_plat_root = {
    .core = &g_sec_bus_core,
    .id = 0x0,
    .rate = 796000000,
    .mux = 5,
    .parent = CLK_NODE(g_pll5_root),
};
clk_node_t g_ckgen_bus_sec_plat_div_m = {
    .core = &g_sec_bus_core,
    .id = 0x4000000,
    .rate = 398000000,
    .mux = 5,
    .parent = CLK_NODE(g_pll5_root),
};
clk_node_t g_ckgen_bus_sec_plat_div_n = {
    .core = &g_sec_bus_core,
    .id = 0x3000000,
    .rate = 99500000,
    .mux = 5,
    .parent = CLK_NODE(g_pll5_root),
};
clk_node_t g_ckgen_bus_sec_plat_div_p = {
    .core = &g_sec_bus_core,
    .id = 0x2000000,
    .rate = 796000000,
    .mux = 5,
    .parent = CLK_NODE(g_pll5_root),
};
clk_node_t g_ckgen_bus_sec_plat_div_q = {
    .core = &g_sec_bus_core,
    .id = 0x1000000,
    .rate = 796000000,
    .mux = 5,
    .parent = CLK_NODE(g_pll5_root),
};
clk_node_t g_ckgen_bus_spare_0_root = {
    .core = &g_sec_bus_core,
    .id = 0x1,
    .rate = 500000000,
    .mux = 4,
    .parent = CLK_NODE(g_pll4_root),
};
clk_node_t g_ckgen_bus_spare_0_div_m = {
    .core = &g_sec_bus_core,
    .id = 0x4000001,
    .rate = 250000000,
    .mux = 4,
    .parent = CLK_NODE(g_pll4_root),
};
clk_node_t g_ckgen_bus_spare_0_div_n = {
    .core = &g_sec_bus_core,
    .id = 0x3000001,
    .rate = 62500000,
    .mux = 4,
    .parent = CLK_NODE(g_pll4_root),
};
clk_node_t g_ckgen_bus_spare_0_div_p = {
    .core = &g_sec_bus_core,
    .id = 0x2000001,
    .rate = 500000000,
    .mux = 4,
    .parent = CLK_NODE(g_pll4_root),
};
clk_node_t g_ckgen_bus_spare_0_div_q = {
    .core = &g_sec_bus_core,
    .id = 0x1000001,
    .rate = 500000000,
    .mux = 4,
    .parent = CLK_NODE(g_pll4_root),
};
clk_node_t g_ckgen_bus_spare_1_root = {
    .core = &g_sec_bus_core,
    .id = 0x2,
    .rate = 500000000,
    .mux = 4,
    .parent = CLK_NODE(g_pll4_root),
};
clk_node_t g_ckgen_bus_spare_1_div_m = {
    .core = &g_sec_bus_core,
    .id = 0x4000002,
    .rate = 250000000,
    .mux = 4,
    .parent = CLK_NODE(g_pll4_root),
};
clk_node_t g_ckgen_bus_spare_1_div_n = {
    .core = &g_sec_bus_core,
    .id = 0x3000002,
    .rate = 62500000,
    .mux = 4,
    .parent = CLK_NODE(g_pll4_root),
};
clk_node_t g_ckgen_bus_spare_1_div_p = {
    .core = &g_sec_bus_core,
    .id = 0x2000002,
    .rate = 500000000,
    .mux = 4,
    .parent = CLK_NODE(g_pll4_root),
};
clk_node_t g_ckgen_bus_spare_1_div_q = {
    .core = &g_sec_bus_core,
    .id = 0x1000002,
    .rate = 500000000,
    .mux = 4,
    .parent = CLK_NODE(g_pll4_root),
};
clk_node_t g_ckgen_ip_ce2 = {
    .core = &g_sec_ip_core,
    .id = 0x0,
    .rate = 398000000,
    .mux = 4,
    .parent = CLK_NODE(g_pll5_root),
};
clk_node_t g_ckgen_ip_adc = {
    .core = &g_sec_ip_core,
    .id = 0x1,
    .rate = 398000000,
    .mux = 4,
    .parent = CLK_NODE(g_pll5_root),
};
clk_node_t g_ckgen_ip_spare_2 = {
    .core = &g_sec_ip_core,
    .id = 0x2,
    .rate = 796000000,
    .mux = 4,
    .parent = CLK_NODE(g_pll5_root),
};
clk_node_t g_ckgen_ip_i2c_sec0 = {
    .core = &g_sec_ip_core,
    .id = 0x3,
    .rate = 24000000,
    .mux = 2,
    .parent = CLK_NODE(g_xtal_24m),
};
clk_node_t g_ckgen_ip_i2c_sec1 = {
    .core = &g_sec_ip_core,
    .id = 0x4,
    .rate = 24000000,
    .mux = 2,
    .parent = CLK_NODE(g_xtal_24m),
};
clk_node_t g_ckgen_ip_spi_sec0 = {
    .core = &g_sec_ip_core,
    .id = 0x5,
    .rate = 120000000,
    .mux = 4,
    .parent = CLK_NODE(g_pll3_div_c),
};
clk_node_t g_ckgen_ip_spi_sec1 = {
    .core = &g_sec_ip_core,
    .id = 0x6,
    .rate = 120000000,
    .mux = 4,
    .parent = CLK_NODE(g_pll3_div_c),
};
clk_node_t g_ckgen_ip_uart_sec0 = {
    .core = &g_sec_ip_core,
    .id = 0x7,
    .rate = 60000000,
    .mux = 4,
    .parent = CLK_NODE(g_pll3_div_c),
};
clk_node_t g_ckgen_ip_uart_sec1 = {
    .core = &g_sec_ip_core,
    .id = 0x8,
    .rate = 60000000,
    .mux = 4,
    .parent = CLK_NODE(g_pll3_div_c),
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
clk_node_t g_ckgen_ip_eth2_tx = {
    .core = &g_sec_ip_core,
    .id = 0xd,
    .rate = 250000000,
    .mux = 4,
    .parent = CLK_NODE(g_pll4_div_c),
};
clk_node_t g_ckgen_ip_eth2_rmii = {
    .core = &g_sec_ip_core,
    .id = 0xe,
    .rate = 50000000,
    .mux = 4,
    .parent = CLK_NODE(g_pll4_div_c),
};
clk_node_t g_ckgen_ip_eth2_phy_ref = {
    .core = &g_sec_ip_core,
    .id = 0xf,
    .rate = 125000000,
    .mux = 4,
    .parent = CLK_NODE(g_pll4_div_c),
};
clk_node_t g_ckgen_ip_eth2_timer_sec = {
    .core = &g_sec_ip_core,
    .id = 0x10,
    .rate = 500000000,
    .mux = 4,
    .parent = CLK_NODE(g_pll4_div_a),
};
clk_node_t g_ckgen_ip_spdif1 = {
    .core = &g_sec_ip_core,
    .id = 0x11,
    .rate = 58980000,
    .mux = 4,
    .parent = CLK_NODE(g_pll6_root),
};
clk_node_t g_ckgen_ip_spdif2 = {
    .core = &g_sec_ip_core,
    .id = 0x12,
    .rate = 58980000,
    .mux = 4,
    .parent = CLK_NODE(g_pll6_root),
};
clk_node_t g_ckgen_ip_spdif3 = {
    .core = &g_sec_ip_core,
    .id = 0x13,
    .rate = 58980000,
    .mux = 4,
    .parent = CLK_NODE(g_pll6_root),
};
clk_node_t g_ckgen_ip_spdif4 = {
    .core = &g_sec_ip_core,
    .id = 0x14,
    .rate = 58980000,
    .mux = 4,
    .parent = CLK_NODE(g_pll6_root),
};
clk_node_t g_ckgen_ip_ospi2 = {
    .core = &g_sec_ip_core,
    .id = 0x15,
    .rate = 398000000,
    .mux = 7,
    .parent = CLK_NODE(g_pll5_div_a),
};
clk_node_t g_ckgen_ip_timer3 = {
    .core = &g_sec_ip_core,
    .id = 0x16,
    .rate = 398000000,
    .mux = 4,
    .parent = CLK_NODE(g_pll5_div_a),
};
clk_node_t g_ckgen_ip_timer4 = {
    .core = &g_sec_ip_core,
    .id = 0x17,
    .rate = 398000000,
    .mux = 4,
    .parent = CLK_NODE(g_pll5_div_a),
};
clk_node_t g_ckgen_ip_timer5 = {
    .core = &g_sec_ip_core,
    .id = 0x18,
    .rate = 398000000,
    .mux = 4,
    .parent = CLK_NODE(g_pll5_div_a),
};
clk_node_t g_ckgen_ip_timer6 = {
    .core = &g_sec_ip_core,
    .id = 0x19,
    .rate = 398000000,
    .mux = 4,
    .parent = CLK_NODE(g_pll5_div_a),
};
clk_node_t g_ckgen_ip_timer7 = {
    .core = &g_sec_ip_core,
    .id = 0x1a,
    .rate = 398000000,
    .mux = 4,
    .parent = CLK_NODE(g_pll5_div_a),
};
clk_node_t g_ckgen_ip_timer8 = {
    .core = &g_sec_ip_core,
    .id = 0x1b,
    .rate = 398000000,
    .mux = 4,
    .parent = CLK_NODE(g_pll5_div_a),
};
clk_node_t g_ckgen_ip_pwm3 = {
    .core = &g_sec_ip_core,
    .id = 0x1c,
    .rate = 398000000,
    .mux = 4,
    .parent = CLK_NODE(g_pll5_div_a),
};
clk_node_t g_ckgen_ip_pwm4 = {
    .core = &g_sec_ip_core,
    .id = 0x1d,
    .rate = 398000000,
    .mux = 4,
    .parent = CLK_NODE(g_pll5_div_a),
};
clk_node_t g_ckgen_ip_pwm5 = {
    .core = &g_sec_ip_core,
    .id = 0x1e,
    .rate = 398000000,
    .mux = 4,
    .parent = CLK_NODE(g_pll5_div_a),
};
clk_node_t g_ckgen_ip_pwm6 = {
    .core = &g_sec_ip_core,
    .id = 0x1f,
    .rate = 398000000,
    .mux = 4,
    .parent = CLK_NODE(g_pll5_div_a),
};
clk_node_t g_ckgen_ip_pwm7 = {
    .core = &g_sec_ip_core,
    .id = 0x20,
    .rate = 398000000,
    .mux = 4,
    .parent = CLK_NODE(g_pll5_div_a),
};
clk_node_t g_ckgen_ip_pwm8 = {
    .core = &g_sec_ip_core,
    .id = 0x21,
    .rate = 398000000,
    .mux = 4,
    .parent = CLK_NODE(g_pll5_div_a),
};
clk_node_t g_ckgen_ip_i2s_mclk2 = {
    .core = &g_sec_ip_core,
    .id = 0x22,
    .rate = 58980000,
    .mux = 4,
    .parent = CLK_NODE(g_pll6_root),
};
clk_node_t g_ckgen_ip_i2s_mclk3 = {
    .core = &g_sec_ip_core,
    .id = 0x23,
    .rate = 58980000,
    .mux = 4,
    .parent = CLK_NODE(g_pll6_root),
};
clk_node_t g_ckgen_ip_i2s_mc1 = {
    .core = &g_sec_ip_core,
    .id = 0x24,
    .rate = 398000000,
    .mux = 6,
    .parent = CLK_NODE(g_pll5_div_a),
};
clk_node_t g_ckgen_ip_i2s_mc2 = {
    .core = &g_sec_ip_core,
    .id = 0x25,
    .rate = 398000000,
    .mux = 6,
    .parent = CLK_NODE(g_pll5_div_a),
};
clk_node_t g_ckgen_ip_i2s_sc3 = {
    .core = &g_sec_ip_core,
    .id = 0x26,
    .rate = 294912000,
    .mux = 4,
    .parent = CLK_NODE(g_pll6_div_c),
};
clk_node_t g_ckgen_ip_i2s_sc4 = {
    .core = &g_sec_ip_core,
    .id = 0x27,
    .rate = 294912000,
    .mux = 4,
    .parent = CLK_NODE(g_pll6_div_c),
};
clk_node_t g_ckgen_ip_i2s_sc5 = {
    .core = &g_sec_ip_core,
    .id = 0x28,
    .rate = 294912000,
    .mux = 4,
    .parent = CLK_NODE(g_pll6_div_c),
};
clk_node_t g_ckgen_ip_i2s_sc6 = {
    .core = &g_sec_ip_core,
    .id = 0x29,
    .rate = 294912000,
    .mux = 4,
    .parent = CLK_NODE(g_pll6_div_c),
};
clk_node_t g_ckgen_ip_i2s_sc7 = {
    .core = &g_sec_ip_core,
    .id = 0x2a,
    .rate = 294912000,
    .mux = 4,
    .parent = CLK_NODE(g_pll6_div_c),
};
clk_node_t g_ckgen_ip_i2s_sc8 = {
    .core = &g_sec_ip_core,
    .id = 0x2b,
    .rate = 294912000,
    .mux = 4,
    .parent = CLK_NODE(g_pll6_div_c),
};
clk_node_t g_ckgen_ip_csi_mclk1 = {
    .core = &g_sec_ip_core,
    .id = 0x2c,
    .rate = 150000000,
    .mux = 4,
    .parent = CLK_NODE(g_pll3_div_b),
};
clk_node_t g_ckgen_ip_csi_mclk2 = {
    .core = &g_sec_ip_core,
    .id = 0x2d,
    .rate = 150000000,
    .mux = 4,
    .parent = CLK_NODE(g_pll3_div_b),
};
clk_node_t g_ckgen_ip_gic_4_5 = {
    .core = &g_sec_ip_core,
    .id = 0x2e,
    .rate = 500000000,
    .mux = 4,
    .parent = CLK_NODE(g_pll4_div_a),
};
clk_node_t g_ckgen_ip_trace = {
    .core = &g_sec_ip_core,
    .id = 0x30,
    .rate = 166670000,
    .mux = 4,
    .parent = CLK_NODE(g_pll4_div_b),
};
clk_node_t g_ckgen_ip_sys_cnt = {
    .core = &g_sec_ip_core,
    .id = 0x31,
    .rate = 3000000,
    .mux = 2,
    .parent = CLK_NODE(g_xtal_24m),
};
clk_node_t g_ckgen_ip_mshc_timer = {
    .core = &g_sec_ip_core,
    .id = 0x32,
    .rate = 1000000,
    .mux = 0,
    .parent = CLK_NODE(g_rc_24m),
};
clk_node_t g_ckgen_ip_hpi_clk600 = {
    .core = &g_sec_ip_core,
    .id = 0x33,
    .rate = 600000000,
    .mux = 4,
    .parent = CLK_NODE(g_pll3_div_a),
};
clk_node_t g_ckgen_ip_hpi_clk800 = {
    .core = &g_sec_ip_core,
    .id = 0x34,
    .rate = 796000000,
    .mux = 4,
    .parent = CLK_NODE(g_pll5_root),
};
clk_node_t g_pll_disp_vco = {
    .core = &g_pll_disp_core,
    .id = 0x0,
    .rate = 2079000000,
    .mux = 0,
    .parent = CLK_NODE(g_rc_24m),
};
clk_node_t g_pll_disp_root = {
    .core = &g_pll_disp_core,
    .id = 0x1,
    .rate = 2079000000,
    .mux = 0,
    .parent = CLK_NODE(g_pll_disp_vco),
};
clk_node_t g_pll_disp_div_a = {
    .core = &g_pll_disp_core,
    .id = 0x2,
    .rate = 415800000,
    .mux = 0,
    .parent = CLK_NODE(g_pll_disp_vco),
};
clk_node_t g_pll_disp_div_b = {
    .core = &g_pll_disp_core,
    .id = 0x3,
    .rate = 1039500000,
    .mux = 0,
    .parent = CLK_NODE(g_pll_disp_vco),
};
clk_node_t g_pll_lvds1_vco = {
    .core = &g_pll_lvds1_core,
    .id = 0x0,
    .rate = 1995000000,
    .mux = 0,
    .parent = CLK_NODE(g_rc_24m),
};
clk_node_t g_pll_lvds1_root = {
    .core = &g_pll_lvds1_core,
    .id = 0x1,
    .rate = 1995000000,
    .mux = 0,
    .parent = CLK_NODE(g_pll_lvds1_vco),
};
clk_node_t g_pll_lvds1_div_a = {
    .core = &g_pll_lvds1_core,
    .id = 0x2,
    .rate = 498750000,
    .mux = 0,
    .parent = CLK_NODE(g_pll_lvds1_vco),
};
clk_node_t g_pll_lvds1_div_b = {
    .core = &g_pll_lvds1_core,
    .id = 0x3,
    .rate = 399000000,
    .mux = 0,
    .parent = CLK_NODE(g_pll_lvds1_vco),
};
clk_node_t g_pll_lvds2_vco = {
    .core = &g_pll_lvds2_core,
    .id = 0x0,
    .rate = 1659000000,
    .mux = 0,
    .parent = CLK_NODE(g_rc_24m),
};
clk_node_t g_pll_lvds2_root = {
    .core = &g_pll_lvds2_core,
    .id = 0x1,
    .rate = 1659000000,
    .mux = 0,
    .parent = CLK_NODE(g_pll_lvds2_vco),
};
clk_node_t g_pll_lvds2_div_a = {
    .core = &g_pll_lvds2_core,
    .id = 0x2,
    .rate = 331800000,
    .mux = 0,
    .parent = CLK_NODE(g_pll_lvds2_vco),
};
clk_node_t g_pll_lvds2_div_b = {
    .core = &g_pll_lvds2_core,
    .id = 0x3,
    .rate = 331800000,
    .mux = 0,
    .parent = CLK_NODE(g_pll_lvds2_vco),
};
clk_node_t g_pll_lvds3_vco = {
    .core = &g_pll_lvds3_core,
    .id = 0x0,
    .rate = 1995000000,
    .mux = 0,
    .parent = CLK_NODE(g_rc_24m),
};
clk_node_t g_pll_lvds3_root = {
    .core = &g_pll_lvds3_core,
    .id = 0x1,
    .rate = 1995000000,
    .mux = 0,
    .parent = CLK_NODE(g_pll_lvds3_vco),
};
clk_node_t g_pll_lvds3_div_a = {
    .core = &g_pll_lvds3_core,
    .id = 0x2,
    .rate = 498750000,
    .mux = 0,
    .parent = CLK_NODE(g_pll_lvds3_vco),
};
clk_node_t g_pll_lvds3_div_b = {
    .core = &g_pll_lvds3_core,
    .id = 0x3,
    .rate = 399000000,
    .mux = 0,
    .parent = CLK_NODE(g_pll_lvds3_vco),
};
clk_node_t g_pll_lvds4_vco = {
    .core = &g_pll_lvds4_core,
    .id = 0x0,
    .rate = 1995000000,
    .mux = 0,
    .parent = CLK_NODE(g_rc_24m),
};
clk_node_t g_pll_lvds4_root = {
    .core = &g_pll_lvds4_core,
    .id = 0x1,
    .rate = 1995000000,
    .mux = 0,
    .parent = CLK_NODE(g_pll_lvds4_vco),
};
clk_node_t g_pll_lvds4_div_a = {
    .core = &g_pll_lvds4_core,
    .id = 0x2,
    .rate = 498750000,
    .mux = 0,
    .parent = CLK_NODE(g_pll_lvds4_vco),
};
clk_node_t g_pll_lvds4_div_b = {
    .core = &g_pll_lvds4_core,
    .id = 0x3,
    .rate = 399000000,
    .mux = 0,
    .parent = CLK_NODE(g_pll_lvds4_vco),
};
clk_node_t g_ckgen_ip_mipi_csi1_pix = {
    .core = &g_disp_ip_core,
    .id = 0x0,
    .rate = 693000000,
    .mux = 5,
    .parent = CLK_NODE(g_pll_disp_root),
};
clk_node_t g_ckgen_ip_mipi_csi2_pix = {
    .core = &g_disp_ip_core,
    .id = 0x1,
    .rate = 693000000,
    .mux = 5,
    .parent = CLK_NODE(g_pll_disp_root),
};
clk_node_t g_ckgen_ip_csi3 = {
    .core = &g_disp_ip_core,
    .id = 0x2,
    .rate = 693000000,
    .mux = 5,
    .parent = CLK_NODE(g_pll_disp_root),
};
clk_node_t g_ckgen_ip_dp1 = {
    .core = &g_disp_ip_core,
    .id = 0x3,
    .rate = 415800000,
    .mux = 4,
    .parent = CLK_NODE(g_pll_disp_div_a),
};
clk_node_t g_ckgen_ip_dp2 = {
    .core = &g_disp_ip_core,
    .id = 0x4,
    .rate = 415800000,
    .mux = 4,
    .parent = CLK_NODE(g_pll_disp_div_a),
};
clk_node_t g_ckgen_ip_dp3 = {
    .core = &g_disp_ip_core,
    .id = 0x5,
    .rate = 415800000,
    .mux = 4,
    .parent = CLK_NODE(g_pll_disp_div_a),
};
clk_node_t g_ckgen_ip_dc5 = {
    .core = &g_disp_ip_core,
    .id = 0x6,
    .rate = 415800000,
    .mux = 4,
    .parent = CLK_NODE(g_pll_disp_div_a),
};
clk_node_t g_ckgen_ip_dc1 = {
    .core = &g_disp_ip_core,
    .id = 0x7,
    .rate = 415800000,
    .mux = 4,
    .parent = CLK_NODE(g_pll_disp_div_a),
};
clk_node_t g_ckgen_ip_dc2 = {
    .core = &g_disp_ip_core,
    .id = 0x8,
    .rate = 415800000,
    .mux = 4,
    .parent = CLK_NODE(g_pll_disp_div_a),
};
clk_node_t g_ckgen_ip_dc3 = {
    .core = &g_disp_ip_core,
    .id = 0x9,
    .rate = 415800000,
    .mux = 4,
    .parent = CLK_NODE(g_pll_disp_div_a),
};
clk_node_t g_ckgen_ip_dc4 = {
    .core = &g_disp_ip_core,
    .id = 0xa,
    .rate = 415800000,
    .mux = 4,
    .parent = CLK_NODE(g_pll_disp_div_a),
};
clk_node_t g_ckgen_ip_spare1 = {
    .core = &g_disp_ip_core,
    .id = 0xb,
    .rate = 103950000,
    .mux = 4,
    .parent = CLK_NODE(g_pll_disp_div_b),
};
clk_node_t g_ckgen_ip_dc5_alt_dsp_clk = {
    .core = &g_disp_ip_core,
    .id = 0xc,
    .rate = 148500000,
    .mux = 5,
    .parent = CLK_NODE(g_pll_disp_root),
};
clk_node_t g_ckgen_bus_disp_bus_root = {
    .core = &g_disp_bus_core,
    .id = 0x0,
    .rate = 519750000,
    .mux = 4,
    .parent = CLK_NODE(g_pll_disp_div_b),
};
clk_node_t g_ckgen_bus_disp_bus_div_m = {
    .core = &g_disp_bus_core,
    .id = 0x4000000,
    .rate = 519750000,
    .mux = 4,
    .parent = CLK_NODE(g_pll_disp_div_b),
};
clk_node_t g_ckgen_bus_disp_bus_div_n = {
    .core = &g_disp_bus_core,
    .id = 0x3000000,
    .rate = 259875000,
    .mux = 4,
    .parent = CLK_NODE(g_pll_disp_div_b),
};
clk_node_t g_ckgen_bus_disp_bus_div_p = {
    .core = &g_disp_bus_core,
    .id = 0x2000000,
    .rate = 519750000,
    .mux = 4,
    .parent = CLK_NODE(g_pll_disp_div_b),
};
clk_node_t g_ckgen_bus_disp_bus_div_q = {
    .core = &g_disp_bus_core,
    .id = 0x1000000,
    .rate = 519750000,
    .mux = 4,
    .parent = CLK_NODE(g_pll_disp_div_b),
};
clk_node_t g_ckgen_ip_ext_aud1 = {
    .core = &g_disp_ip_core,
    .id = 0xd,
    .rate = 199500000,
    .mux = 4,
    .parent = CLK_NODE(g_pll_lvds1_root),
};
clk_node_t g_ckgen_ip_ext_aud2 = {
    .core = &g_disp_ip_core,
    .id = 0xe,
    .rate = 199500000,
    .mux = 4,
    .parent = CLK_NODE(g_pll_lvds1_root),
};
clk_node_t g_ckgen_ip_ext_aud3 = {
    .core = &g_disp_ip_core,
    .id = 0xf,
    .rate = 199500000,
    .mux = 4,
    .parent = CLK_NODE(g_pll_lvds1_root),
};
clk_node_t g_ckgen_ip_ext_aud4 = {
    .core = &g_disp_ip_core,
    .id = 0x10,
    .rate = 199500000,
    .mux = 4,
    .parent = CLK_NODE(g_pll_lvds1_root),
};
clk_node_t g_ckgen_core_cpu1a_pre = {
    .core = &g_soc_core_core,
    .id = 0x0,
    .rate = 24000000,
    .mux = 0,
    .parent = CLK_NODE(g_rc_24m),
};
clk_node_t g_ckgen_uuu_cpu1a_div_m_pre = {
    .core = &g_soc_uuu_core,
    .id = 0x3000000,
    .rate = 24000000,
    .mux = 0,
    .parent = CLK_NODE(g_ckgen_core_cpu1a_pre),
};
clk_node_t g_ckgen_uuu_cpu1a_div_n_pre = {
    .core = &g_soc_uuu_core,
    .id = 0x2000000,
    .rate = 24000000,
    .mux = 0,
    .parent = CLK_NODE(g_ckgen_core_cpu1a_pre),
};
clk_node_t g_ckgen_uuu_cpu1a_div_p_pre = {
    .core = &g_soc_uuu_core,
    .id = 0x1000000,
    .rate = 24000000,
    .mux = 0,
    .parent = CLK_NODE(g_ckgen_core_cpu1a_pre),
};
clk_node_t g_ckgen_uuu_cpu1a_div_q_pre = {
    .core = &g_soc_uuu_core,
    .id = 0x0,
    .rate = 24000000,
    .mux = 0,
    .parent = CLK_NODE(g_ckgen_core_cpu1a_pre),
};
clk_node_t g_pll_cpu1a_vco = {
    .core = &g_pll_cpu1a_core,
    .id = 0x0,
    .rate = 1600000000,
    .mux = 0,
    .parent = CLK_NODE(g_rc_24m),
};
clk_node_t g_pll_cpu1a_root = {
    .core = &g_pll_cpu1a_core,
    .id = 0x1,
    .rate = 1600000000,
    .mux = 0,
    .parent = CLK_NODE(g_pll_cpu1a_vco),
};
clk_node_t g_pll_cpu1a_div_a = {
    .core = &g_pll_cpu1a_core,
    .id = 0x2,
    .rate = 800000000,
    .mux = 0,
    .parent = CLK_NODE(g_pll_cpu1a_vco),
};
clk_node_t g_pll_cpu1a_div_b = {
    .core = &g_pll_cpu1a_core,
    .id = 0x3,
    .rate = 400000000,
    .mux = 0,
    .parent = CLK_NODE(g_pll_cpu1a_vco),
};
clk_node_t g_ckgen_core_cpu1b_pre = {
    .core = &g_soc_core_core,
    .id = 0x1,
    .rate = 24000000,
    .mux = 0,
    .parent = CLK_NODE(g_rc_24m),
};
clk_node_t g_ckgen_uuu_cpu1b_div_m_pre = {
    .core = &g_soc_uuu_core,
    .id = 0x3000001,
    .rate = 24000000,
    .mux = 0,
    .parent = CLK_NODE(g_ckgen_core_cpu1b_pre),
};
clk_node_t g_ckgen_uuu_cpu1b_div_n_pre = {
    .core = &g_soc_uuu_core,
    .id = 0x2000001,
    .rate = 24000000,
    .mux = 0,
    .parent = CLK_NODE(g_ckgen_core_cpu1b_pre),
};
clk_node_t g_ckgen_uuu_cpu1b_div_p_pre = {
    .core = &g_soc_uuu_core,
    .id = 0x1000001,
    .rate = 24000000,
    .mux = 0,
    .parent = CLK_NODE(g_ckgen_core_cpu1b_pre),
};
clk_node_t g_ckgen_uuu_cpu1b_div_q_pre = {
    .core = &g_soc_uuu_core,
    .id = 0x1,
    .rate = 24000000,
    .mux = 0,
    .parent = CLK_NODE(g_ckgen_core_cpu1b_pre),
};
clk_node_t g_pll_cpu1b_vco = {
    .core = &g_pll_cpu1b_core,
    .id = 0x0,
    .rate = 1280000000,
    .mux = 0,
    .parent = CLK_NODE(g_rc_24m),
};
clk_node_t g_pll_cpu1b_root = {
    .core = &g_pll_cpu1b_core,
    .id = 0x1,
    .rate = 1280000000,
    .mux = 0,
    .parent = CLK_NODE(g_pll_cpu1b_vco),
};
clk_node_t g_pll_cpu1b_div_a = {
    .core = &g_pll_cpu1b_core,
    .id = 0x2,
    .rate = 640000000,
    .mux = 0,
    .parent = CLK_NODE(g_pll_cpu1b_vco),
};
clk_node_t g_pll_cpu1b_div_b = {
    .core = &g_pll_cpu1b_core,
    .id = 0x3,
    .rate = 320000000,
    .mux = 0,
    .parent = CLK_NODE(g_pll_cpu1b_vco),
};
clk_node_t g_ckgen_core_gpu1_pre = {
    .core = &g_soc_core_core,
    .id = 0x3,
    .rate = 24000000,
    .mux = 0,
    .parent = CLK_NODE(g_rc_24m),
};
clk_node_t g_ckgen_uuu_gpu1_div_m_pre = {
    .core = &g_soc_uuu_core,
    .id = 0x3000003,
    .rate = 24000000,
    .mux = 0,
    .parent = CLK_NODE(g_ckgen_core_gpu1_pre),
};
clk_node_t g_ckgen_uuu_gpu1_div_n_pre = {
    .core = &g_soc_uuu_core,
    .id = 0x2000003,
    .rate = 24000000,
    .mux = 0,
    .parent = CLK_NODE(g_ckgen_core_gpu1_pre),
};
clk_node_t g_ckgen_uuu_gpu1_div_p_pre = {
    .core = &g_soc_uuu_core,
    .id = 0x1000003,
    .rate = 24000000,
    .mux = 0,
    .parent = CLK_NODE(g_ckgen_core_gpu1_pre),
};
clk_node_t g_ckgen_uuu_gpu1_div_q_pre = {
    .core = &g_soc_uuu_core,
    .id = 0x3,
    .rate = 24000000,
    .mux = 0,
    .parent = CLK_NODE(g_ckgen_core_gpu1_pre),
};
clk_node_t g_pll_gpu1_vco = {
    .core = &g_pll_gpu1_core,
    .id = 0x0,
    .rate = 1504000000,
    .mux = 0,
    .parent = CLK_NODE(g_rc_24m),
};
clk_node_t g_pll_gpu1_root = {
    .core = &g_pll_gpu1_core,
    .id = 0x1,
    .rate = 752000000,
    .mux = 0,
    .parent = CLK_NODE(g_pll_gpu1_vco),
};
clk_node_t g_pll_gpu1_div_a = {
    .core = &g_pll_gpu1_core,
    .id = 0x2,
    .rate = 376000000,
    .mux = 0,
    .parent = CLK_NODE(g_pll_gpu1_vco),
};
clk_node_t g_pll_gpu1_div_b = {
    .core = &g_pll_gpu1_core,
    .id = 0x3,
    .rate = 250666700,
    .mux = 0,
    .parent = CLK_NODE(g_pll_gpu1_vco),
};
clk_node_t g_ckgen_core_gpu2_pre = {
    .core = &g_soc_core_core,
    .id = 0x4,
    .rate = 24000000,
    .mux = 0,
    .parent = CLK_NODE(g_rc_24m),
};
clk_node_t g_ckgen_uuu_gpu2_div_m_pre = {
    .core = &g_soc_uuu_core,
    .id = 0x3000004,
    .rate = 24000000,
    .mux = 0,
    .parent = CLK_NODE(g_ckgen_core_gpu2_pre),
};
clk_node_t g_ckgen_uuu_gpu2_div_n_pre = {
    .core = &g_soc_uuu_core,
    .id = 0x2000004,
    .rate = 24000000,
    .mux = 0,
    .parent = CLK_NODE(g_ckgen_core_gpu2_pre),
};
clk_node_t g_ckgen_uuu_gpu2_div_p_pre = {
    .core = &g_soc_uuu_core,
    .id = 0x1000004,
    .rate = 24000000,
    .mux = 0,
    .parent = CLK_NODE(g_ckgen_core_gpu2_pre),
};
clk_node_t g_ckgen_uuu_gpu2_div_q_pre = {
    .core = &g_soc_uuu_core,
    .id = 0x4,
    .rate = 24000000,
    .mux = 0,
    .parent = CLK_NODE(g_ckgen_core_gpu2_pre),
};
clk_node_t g_pll_gpu2_vco = {
    .core = &g_pll_gpu2_core,
    .id = 0x0,
    .rate = 1400000000,
    .mux = 0,
    .parent = CLK_NODE(g_rc_24m),
};
clk_node_t g_pll_gpu2_root = {
    .core = &g_pll_gpu2_core,
    .id = 0x1,
    .rate = 700000000,
    .mux = 0,
    .parent = CLK_NODE(g_pll_gpu2_vco),
};
clk_node_t g_pll_gpu2_div_a = {
    .core = &g_pll_gpu2_core,
    .id = 0x2,
    .rate = 350000000,
    .mux = 0,
    .parent = CLK_NODE(g_pll_gpu2_vco),
};
clk_node_t g_pll_gpu2_div_b = {
    .core = &g_pll_gpu2_core,
    .id = 0x3,
    .rate = 233333300,
    .mux = 0,
    .parent = CLK_NODE(g_pll_gpu2_vco),
};
clk_node_t g_ckgen_ip_vpu1_pre = {
    .core = &g_soc_ip_core,
    .id = 0x0,
    .rate = 24000000,
    .mux = 0,
    .parent = CLK_NODE(g_rc_24m),
};
clk_node_t g_ckgen_ip_mjpeg_pre = {
    .core = &g_soc_ip_core,
    .id = 0x1,
    .rate = 24000000,
    .mux = 0,
    .parent = CLK_NODE(g_rc_24m),
};
clk_node_t g_ckgen_bus_vpu_root_pre = {
    .core = &g_soc_bus_core,
    .id = 0x0,
    .rate = 24000000,
    .mux = 0,
    .parent = CLK_NODE(g_rc_24m),
};
clk_node_t g_ckgen_bus_vpu_div_m_pre = {
    .core = &g_soc_bus_core,
    .id = 0x4000000,
    .rate = 24000000,
    .mux = 0,
    .parent = CLK_NODE(g_rc_24m),
};
clk_node_t g_ckgen_bus_vpu_div_n_pre = {
    .core = &g_soc_bus_core,
    .id = 0x3000000,
    .rate = 24000000,
    .mux = 0,
    .parent = CLK_NODE(g_rc_24m),
};
clk_node_t g_ckgen_bus_vpu_div_p_pre = {
    .core = &g_soc_bus_core,
    .id = 0x2000000,
    .rate = 24000000,
    .mux = 0,
    .parent = CLK_NODE(g_rc_24m),
};
clk_node_t g_ckgen_bus_vpu_div_q_pre = {
    .core = &g_soc_bus_core,
    .id = 0x1000000,
    .rate = 24000000,
    .mux = 0,
    .parent = CLK_NODE(g_rc_24m),
};
clk_node_t g_ckgen_uuu_vpu1_div_m_pre = {
    .core = &g_soc_uuu_core,
    .id = 0x3000005,
    .rate = 24000000,
    .mux = 0,
    .parent = CLK_NODE(g_ckgen_ip_vpu1_pre),
};
clk_node_t g_ckgen_uuu_vpu1_div_n_pre = {
    .core = &g_soc_uuu_core,
    .id = 0x2000005,
    .rate = 24000000,
    .mux = 0,
    .parent = CLK_NODE(g_ckgen_ip_vpu1_pre),
};
clk_node_t g_ckgen_uuu_vpu1_div_p_pre = {
    .core = &g_soc_uuu_core,
    .id = 0x1000005,
    .rate = 24000000,
    .mux = 0,
    .parent = CLK_NODE(g_ckgen_ip_vpu1_pre),
};
clk_node_t g_ckgen_uuu_vpu1_div_q_pre = {
    .core = &g_soc_uuu_core,
    .id = 0x5,
    .rate = 24000000,
    .mux = 0,
    .parent = CLK_NODE(g_ckgen_ip_vpu1_pre),
};
clk_node_t g_ckgen_uuu_mjpeg_div_m_pre = {
    .core = &g_soc_uuu_core,
    .id = 0x3000006,
    .rate = 24000000,
    .mux = 0,
    .parent = CLK_NODE(g_ckgen_ip_mjpeg_pre),
};
clk_node_t g_ckgen_uuu_mjpeg_div_n_pre = {
    .core = &g_soc_uuu_core,
    .id = 0x2000006,
    .rate = 24000000,
    .mux = 0,
    .parent = CLK_NODE(g_ckgen_ip_mjpeg_pre),
};
clk_node_t g_ckgen_uuu_mjpeg_div_p_pre = {
    .core = &g_soc_uuu_core,
    .id = 0x1000006,
    .rate = 24000000,
    .mux = 0,
    .parent = CLK_NODE(g_ckgen_ip_mjpeg_pre),
};
clk_node_t g_ckgen_uuu_mjpeg_div_q_pre = {
    .core = &g_soc_uuu_core,
    .id = 0x6,
    .rate = 24000000,
    .mux = 0,
    .parent = CLK_NODE(g_ckgen_ip_mjpeg_pre),
};
clk_node_t g_ckgen_uuu_vpu_div_m_pre = {
    .core = &g_soc_uuu_core,
    .id = 0x3000007,
    .rate = 24000000,
    .mux = 0,
    .parent = CLK_NODE(g_ckgen_bus_vpu_root_pre),
};
clk_node_t g_ckgen_uuu_vpu_div_n_pre = {
    .core = &g_soc_uuu_core,
    .id = 0x2000007,
    .rate = 24000000,
    .mux = 0,
    .parent = CLK_NODE(g_ckgen_bus_vpu_root_pre),
};
clk_node_t g_ckgen_uuu_vpu_div_p_pre = {
    .core = &g_soc_uuu_core,
    .id = 0x1000007,
    .rate = 24000000,
    .mux = 0,
    .parent = CLK_NODE(g_ckgen_bus_vpu_root_pre),
};
clk_node_t g_ckgen_uuu_vpu_div_q_pre = {
    .core = &g_soc_uuu_core,
    .id = 0x7,
    .rate = 24000000,
    .mux = 0,
    .parent = CLK_NODE(g_ckgen_bus_vpu_root_pre),
};
clk_node_t g_pll_vpu_vco = {
    .core = &g_pll_vpu_core,
    .id = 0x0,
    .rate = 2664000000,
    .mux = 0,
    .parent = CLK_NODE(g_rc_24m),
};
clk_node_t g_pll_vpu_root = {
    .core = &g_pll_vpu_core,
    .id = 0x1,
    .rate = 2664000000,
    .mux = 0,
    .parent = CLK_NODE(g_pll_vpu_vco),
};
clk_node_t g_pll_vpu_div_a = {
    .core = &g_pll_vpu_core,
    .id = 0x2,
    .rate = 666000000,
    .mux = 0,
    .parent = CLK_NODE(g_pll_vpu_vco),
};
clk_node_t g_pll_vpu_div_b = {
    .core = &g_pll_vpu_core,
    .id = 0x3,
    .rate = 532799999,
    .mux = 0,
    .parent = CLK_NODE(g_pll_vpu_vco),
};
clk_node_t g_ckgen_bus_vsn_root_pre = {
    .core = &g_soc_bus_core,
    .id = 0x1,
    .rate = 24000000,
    .mux = 0,
    .parent = CLK_NODE(g_rc_24m),
};
clk_node_t g_ckgen_bus_vsn_div_m_pre = {
    .core = &g_soc_bus_core,
    .id = 0x4000001,
    .rate = 24000000,
    .mux = 0,
    .parent = CLK_NODE(g_rc_24m),
};
clk_node_t g_ckgen_bus_vsn_div_n_pre = {
    .core = &g_soc_bus_core,
    .id = 0x3000001,
    .rate = 24000000,
    .mux = 0,
    .parent = CLK_NODE(g_rc_24m),
};
clk_node_t g_ckgen_bus_vsn_div_p_pre = {
    .core = &g_soc_bus_core,
    .id = 0x2000001,
    .rate = 24000000,
    .mux = 0,
    .parent = CLK_NODE(g_rc_24m),
};
clk_node_t g_ckgen_bus_vsn_div_q_pre = {
    .core = &g_soc_bus_core,
    .id = 0x1000001,
    .rate = 24000000,
    .mux = 0,
    .parent = CLK_NODE(g_rc_24m),
};
clk_node_t g_ckgen_uuu_vsn_div_m_pre = {
    .core = &g_soc_uuu_core,
    .id = 0x3000008,
    .rate = 24000000,
    .mux = 0,
    .parent = CLK_NODE(g_ckgen_bus_vsn_root_pre),
};
clk_node_t g_ckgen_uuu_vsn_div_n_pre = {
    .core = &g_soc_uuu_core,
    .id = 0x2000008,
    .rate = 24000000,
    .mux = 0,
    .parent = CLK_NODE(g_ckgen_bus_vsn_root_pre),
};
clk_node_t g_ckgen_uuu_vsn_div_p_pre = {
    .core = &g_soc_uuu_core,
    .id = 0x1000008,
    .rate = 24000000,
    .mux = 0,
    .parent = CLK_NODE(g_ckgen_bus_vsn_root_pre),
};
clk_node_t g_ckgen_uuu_vsn_div_q_pre = {
    .core = &g_soc_uuu_core,
    .id = 0x8,
    .rate = 24000000,
    .mux = 0,
    .parent = CLK_NODE(g_ckgen_bus_vsn_root_pre),
};
clk_node_t g_pll_vsn_vco = {
    .core = &g_pll_vsn_core,
    .id = 0x0,
    .rate = 1496000000,
    .mux = 0,
    .parent = CLK_NODE(g_rc_24m),
};
clk_node_t g_pll_vsn_root = {
    .core = &g_pll_vsn_core,
    .id = 0x1,
    .rate = 748000000,
    .mux = 0,
    .parent = CLK_NODE(g_pll_vsn_vco),
};
clk_node_t g_pll_vsn_div_a = {
    .core = &g_pll_vsn_core,
    .id = 0x2,
    .rate = 374000000,
    .mux = 0,
    .parent = CLK_NODE(g_pll_vsn_vco),
};
clk_node_t g_pll_vsn_div_b = {
    .core = &g_pll_vsn_core,
    .id = 0x3,
    .rate = 249333300,
    .mux = 0,
    .parent = CLK_NODE(g_pll_vsn_vco),
};
clk_node_t g_pll_hpi_vco = {
    .core = &g_pll_hpi_core,
    .id = 0x0,
    .rate = 1896000000,
    .mux = 0,
    .parent = CLK_NODE(g_rc_24m),
};
clk_node_t g_pll_hpi_root = {
    .core = &g_pll_hpi_core,
    .id = 0x1,
    .rate = 1896000000,
    .mux = 0,
    .parent = CLK_NODE(g_pll_hpi_vco),
};
clk_node_t g_pll_hpi_div_a = {
    .core = &g_pll_hpi_core,
    .id = 0x2,
    .rate = 474000000,
    .mux = 0,
    .parent = CLK_NODE(g_pll_hpi_vco),
};
clk_node_t g_pll_hpi_div_b = {
    .core = &g_pll_hpi_core,
    .id = 0x3,
    .rate = 316000000,
    .mux = 0,
    .parent = CLK_NODE(g_pll_hpi_vco),
};
clk_node_t g_pll_his_vco = {
    .core = &g_pll_his_core,
    .id = 0x0,
    .rate = 2000000000,
    .mux = 0,
    .parent = CLK_NODE(g_rc_24m),
};
clk_node_t g_pll_his_root = {
    .core = &g_pll_his_core,
    .id = 0x1,
    .rate = 1000000000,
    .mux = 0,
    .parent = CLK_NODE(g_pll_his_vco),
};
clk_node_t g_pll_his_div_a = {
    .core = &g_pll_his_core,
    .id = 0x2,
    .rate = 500000000,
    .mux = 0,
    .parent = CLK_NODE(g_pll_his_vco),
};
clk_node_t g_pll_his_div_b = {
    .core = &g_pll_his_core,
    .id = 0x3,
    .rate = 333333300,
    .mux = 0,
    .parent = CLK_NODE(g_pll_his_vco),
};
clk_node_t g_pll_ddr_vco = {
    .core = &g_pll_ddr_core,
    .id = 0x0,
    .rate = 2128000000,
    .mux = 0,
    .parent = CLK_NODE(g_rc_24m),
};
clk_node_t g_pll_ddr_root = {
    .core = &g_pll_ddr_core,
    .id = 0x1,
    .rate = 1064000000,
    .mux = 0,
    .parent = CLK_NODE(g_pll_ddr_vco),
};
clk_node_t g_pll_ddr_div_a = {
    .core = &g_pll_ddr_core,
    .id = 0x2,
    .rate = 532000000,
    .mux = 0,
    .parent = CLK_NODE(g_pll_ddr_vco),
};
clk_node_t g_pll_ddr_div_b = {
    .core = &g_pll_ddr_core,
    .id = 0x3,
    .rate = 354666700,
    .mux = 0,
    .parent = CLK_NODE(g_pll_ddr_vco),
};
clk_node_t g_ckgen_core_cpu1a = {
    .core = &g_soc_core_core,
    .id = 0x0,
    .rate = 400000000,
    .mux = 3,
    .parent = CLK_NODE(g_pll2_div_a),
};
clk_node_t g_ckgen_core_cpu1b = {
    .core = &g_soc_core_core,
    .id = 0x1,
    .rate = 400000000,
    .mux = 3,
    .parent = CLK_NODE(g_pll2_div_a),
};
clk_node_t g_ckgen_core_gpu1 = {
    .core = &g_soc_core_core,
    .id = 0x3,
    .rate = 400000000,
    .mux = 3,
    .parent = CLK_NODE(g_pll2_div_a),
};
clk_node_t g_ckgen_core_gpu2 = {
    .core = &g_soc_core_core,
    .id = 0x4,
    .rate = 400000000,
    .mux = 3,
    .parent = CLK_NODE(g_pll2_div_a),
};
clk_node_t g_ckgen_ip_vpu1 = {
    .core = &g_soc_ip_core,
    .id = 0x0,
    .rate = 400000000,
    .mux = 3,
    .parent = CLK_NODE(g_pll2_div_a),
};
clk_node_t g_ckgen_ip_mjpeg = {
    .core = &g_soc_ip_core,
    .id = 0x1,
    .rate = 400000000,
    .mux = 3,
    .parent = CLK_NODE(g_pll2_div_a),
};
clk_node_t g_ckgen_bus_vpu_root = {
    .core = &g_soc_bus_core,
    .id = 0x0,
    .rate = 400000000,
    .mux = 3,
    .parent = CLK_NODE(g_pll2_div_a),
};
clk_node_t g_ckgen_bus_vpu_div_m = {
    .core = &g_soc_bus_core,
    .id = 0x4000000,
    .rate = 400000000,
    .mux = 3,
    .parent = CLK_NODE(g_pll2_div_a),
};
clk_node_t g_ckgen_bus_vpu_div_n = {
    .core = &g_soc_bus_core,
    .id = 0x3000000,
    .rate = 400000000,
    .mux = 3,
    .parent = CLK_NODE(g_pll2_div_a),
};
clk_node_t g_ckgen_bus_vpu_div_p = {
    .core = &g_soc_bus_core,
    .id = 0x2000000,
    .rate = 400000000,
    .mux = 3,
    .parent = CLK_NODE(g_pll2_div_a),
};
clk_node_t g_ckgen_bus_vpu_div_q = {
    .core = &g_soc_bus_core,
    .id = 0x1000000,
    .rate = 400000000,
    .mux = 3,
    .parent = CLK_NODE(g_pll2_div_a),
};
clk_node_t g_ckgen_bus_vsn_root = {
    .core = &g_soc_bus_core,
    .id = 0x1,
    .rate = 400000000,
    .mux = 3,
    .parent = CLK_NODE(g_pll2_div_a),
};
clk_node_t g_ckgen_bus_vsn_div_m = {
    .core = &g_soc_bus_core,
    .id = 0x4000001,
    .rate = 400000000,
    .mux = 3,
    .parent = CLK_NODE(g_pll2_div_a),
};
clk_node_t g_ckgen_bus_vsn_div_n = {
    .core = &g_soc_bus_core,
    .id = 0x3000001,
    .rate = 400000000,
    .mux = 3,
    .parent = CLK_NODE(g_pll2_div_a),
};
clk_node_t g_ckgen_bus_vsn_div_p = {
    .core = &g_soc_bus_core,
    .id = 0x2000001,
    .rate = 400000000,
    .mux = 3,
    .parent = CLK_NODE(g_pll2_div_a),
};
clk_node_t g_ckgen_bus_vsn_div_q = {
    .core = &g_soc_bus_core,
    .id = 0x1000001,
    .rate = 400000000,
    .mux = 3,
    .parent = CLK_NODE(g_pll2_div_a),
};
clk_node_t g_ckgen_core_ddr = {
    .core = &g_soc_core_core,
    .id = 0x5,
    .rate = 400000000,
    .mux = 3,
    .parent = CLK_NODE(g_pll2_div_a),
};
clk_node_t g_ckgen_bus_noc_root = {
    .core = &g_soc_bus_core,
    .id = 0x2,
    .rate = 948000000,
    .mux = 4,
    .parent = CLK_NODE(g_pll_hpi_root),
};
clk_node_t g_ckgen_bus_noc_div_m = {
    .core = &g_soc_bus_core,
    .id = 0x4000002,
    .rate = 474000000,
    .mux = 4,
    .parent = CLK_NODE(g_pll_hpi_root),
};
clk_node_t g_ckgen_bus_noc_div_n = {
    .core = &g_soc_bus_core,
    .id = 0x3000002,
    .rate = 158000000,
    .mux = 4,
    .parent = CLK_NODE(g_pll_hpi_root),
};
clk_node_t g_ckgen_bus_noc_div_p = {
    .core = &g_soc_bus_core,
    .id = 0x2000002,
    .rate = 158000000,
    .mux = 4,
    .parent = CLK_NODE(g_pll_hpi_root),
};
clk_node_t g_ckgen_bus_noc_div_q = {
    .core = &g_soc_bus_core,
    .id = 0x1000002,
    .rate = 118500000,
    .mux = 4,
    .parent = CLK_NODE(g_pll_hpi_root),
};
clk_node_t g_ckgen_bus_his_root = {
    .core = &g_soc_bus_core,
    .id = 0x3,
    .rate = 400000000,
    .mux = 3,
    .parent = CLK_NODE(g_pll2_div_a),
};
clk_node_t g_ckgen_bus_his_div_m = {
    .core = &g_soc_bus_core,
    .id = 0x4000003,
    .rate = 400000000,
    .mux = 3,
    .parent = CLK_NODE(g_pll2_div_a),
};
clk_node_t g_ckgen_bus_his_div_n = {
    .core = &g_soc_bus_core,
    .id = 0x3000003,
    .rate = 400000000,
    .mux = 3,
    .parent = CLK_NODE(g_pll2_div_a),
};
clk_node_t g_ckgen_bus_his_div_p = {
    .core = &g_soc_bus_core,
    .id = 0x2000003,
    .rate = 400000000,
    .mux = 3,
    .parent = CLK_NODE(g_pll2_div_a),
};
clk_node_t g_ckgen_bus_his_div_q = {
    .core = &g_soc_bus_core,
    .id = 0x1000003,
    .rate = 400000000,
    .mux = 3,
    .parent = CLK_NODE(g_pll2_div_a),
};
clk_node_t g_ckgen_uuu_cpu1a_div_m = {
    .core = &g_soc_uuu_core,
    .id = 0x3000000,
    .rate = 1600000000,
    .mux = 1,
    .parent = CLK_NODE(g_pll_cpu1a_root),
};
clk_node_t g_ckgen_uuu_cpu1a_div_n = {
    .core = &g_soc_uuu_core,
    .id = 0x2000000,
    .rate = 800000000,
    .mux = 1,
    .parent = CLK_NODE(g_pll_cpu1a_root),
};
clk_node_t g_ckgen_uuu_cpu1a_div_p = {
    .core = &g_soc_uuu_core,
    .id = 0x1000000,
    .rate = 400000000,
    .mux = 1,
    .parent = CLK_NODE(g_pll_cpu1a_root),
};
clk_node_t g_ckgen_uuu_cpu1a_div_q = {
    .core = &g_soc_uuu_core,
    .id = 0x0,
    .rate = 200000000,
    .mux = 1,
    .parent = CLK_NODE(g_pll_cpu1a_root),
};
clk_node_t g_ckgen_uuu_cpu1b_div_m = {
    .core = &g_soc_uuu_core,
    .id = 0x3000001,
    .rate = 1280000000,
    .mux = 1,
    .parent = CLK_NODE(g_pll_cpu1b_root),
};
clk_node_t g_ckgen_uuu_cpu1b_div_n = {
    .core = &g_soc_uuu_core,
    .id = 0x2000001,
    .rate = 640000000,
    .mux = 1,
    .parent = CLK_NODE(g_pll_cpu1b_root),
};
clk_node_t g_ckgen_uuu_cpu1b_div_p = {
    .core = &g_soc_uuu_core,
    .id = 0x1000001,
    .rate = 320000000,
    .mux = 1,
    .parent = CLK_NODE(g_pll_cpu1b_root),
};
clk_node_t g_ckgen_uuu_cpu1b_div_q = {
    .core = &g_soc_uuu_core,
    .id = 0x1,
    .rate = 160000000,
    .mux = 1,
    .parent = CLK_NODE(g_pll_cpu1b_root),
};
clk_node_t g_ckgen_uuu_gpu1_div_m = {
    .core = &g_soc_uuu_core,
    .id = 0x3000003,
    .rate = 752000000,
    .mux = 1,
    .parent = CLK_NODE(g_pll_gpu1_root),
};
clk_node_t g_ckgen_uuu_gpu1_div_n = {
    .core = &g_soc_uuu_core,
    .id = 0x2000003,
    .rate = 752000000,
    .mux = 1,
    .parent = CLK_NODE(g_pll_gpu1_root),
};
clk_node_t g_ckgen_uuu_gpu1_div_p = {
    .core = &g_soc_uuu_core,
    .id = 0x1000003,
    .rate = 188000000,
    .mux = 1,
    .parent = CLK_NODE(g_pll_gpu1_root),
};
clk_node_t g_ckgen_uuu_gpu1_div_q = {
    .core = &g_soc_uuu_core,
    .id = 0x3,
    .rate = 752000000,
    .mux = 1,
    .parent = CLK_NODE(g_pll_gpu1_root),
};
clk_node_t g_ckgen_uuu_gpu2_div_m = {
    .core = &g_soc_uuu_core,
    .id = 0x3000004,
    .rate = 700000000,
    .mux = 1,
    .parent = CLK_NODE(g_pll_gpu2_root),
};
clk_node_t g_ckgen_uuu_gpu2_div_n = {
    .core = &g_soc_uuu_core,
    .id = 0x2000004,
    .rate = 700000000,
    .mux = 1,
    .parent = CLK_NODE(g_pll_gpu2_root),
};
clk_node_t g_ckgen_uuu_gpu2_div_p = {
    .core = &g_soc_uuu_core,
    .id = 0x1000004,
    .rate = 175000000,
    .mux = 1,
    .parent = CLK_NODE(g_pll_gpu2_root),
};
clk_node_t g_ckgen_uuu_gpu2_div_q = {
    .core = &g_soc_uuu_core,
    .id = 0x4,
    .rate = 700000000,
    .mux = 1,
    .parent = CLK_NODE(g_pll_gpu2_root),
};
clk_node_t g_ckgen_uuu_vpu1_div_m = {
    .core = &g_soc_uuu_core,
    .id = 0x3000005,
    .rate = 532799999,
    .mux = 1,
    .parent = CLK_NODE(g_pll_vpu_root),
};
clk_node_t g_ckgen_uuu_vpu1_div_n = {
    .core = &g_soc_uuu_core,
    .id = 0x2000005,
    .rate = 532799999,
    .mux = 1,
    .parent = CLK_NODE(g_pll_vpu_root),
};
clk_node_t g_ckgen_uuu_vpu1_div_p = {
    .core = &g_soc_uuu_core,
    .id = 0x1000005,
    .rate = 532799999,
    .mux = 1,
    .parent = CLK_NODE(g_pll_vpu_root),
};
clk_node_t g_ckgen_uuu_vpu1_div_q = {
    .core = &g_soc_uuu_core,
    .id = 0x5,
    .rate = 532799999,
    .mux = 1,
    .parent = CLK_NODE(g_pll_vpu_root),
};
clk_node_t g_ckgen_uuu_mjpeg_div_m = {
    .core = &g_soc_uuu_core,
    .id = 0x3000006,
    .rate = 532799999,
    .mux = 1,
    .parent = CLK_NODE(g_pll_vpu_root),
};
clk_node_t g_ckgen_uuu_mjpeg_div_n = {
    .core = &g_soc_uuu_core,
    .id = 0x2000006,
    .rate = 532799999,
    .mux = 1,
    .parent = CLK_NODE(g_pll_vpu_root),
};
clk_node_t g_ckgen_uuu_mjpeg_div_p = {
    .core = &g_soc_uuu_core,
    .id = 0x1000006,
    .rate = 532799999,
    .mux = 1,
    .parent = CLK_NODE(g_pll_vpu_root),
};
clk_node_t g_ckgen_uuu_mjpeg_div_q = {
    .core = &g_soc_uuu_core,
    .id = 0x6,
    .rate = 532799999,
    .mux = 1,
    .parent = CLK_NODE(g_pll_vpu_root),
};
clk_node_t g_ckgen_uuu_vpu_div_m = {
    .core = &g_soc_uuu_core,
    .id = 0x3000007,
    .rate = 666000000,
    .mux = 1,
    .parent = CLK_NODE(g_pll_vpu_root),
};
clk_node_t g_ckgen_uuu_vpu_div_n = {
    .core = &g_soc_uuu_core,
    .id = 0x2000007,
    .rate = 166500000,
    .mux = 1,
    .parent = CLK_NODE(g_pll_vpu_root),
};
clk_node_t g_ckgen_uuu_vpu_div_p = {
    .core = &g_soc_uuu_core,
    .id = 0x1000007,
    .rate = 666000000,
    .mux = 1,
    .parent = CLK_NODE(g_pll_vpu_root),
};
clk_node_t g_ckgen_uuu_vpu_div_q = {
    .core = &g_soc_uuu_core,
    .id = 0x7,
    .rate = 666000000,
    .mux = 1,
    .parent = CLK_NODE(g_pll_vpu_root),
};
clk_node_t g_ckgen_uuu_vsn_div_m = {
    .core = &g_soc_uuu_core,
    .id = 0x3000008,
    .rate = 748000000,
    .mux = 1,
    .parent = CLK_NODE(g_pll_vsn_root),
};
clk_node_t g_ckgen_uuu_vsn_div_n = {
    .core = &g_soc_uuu_core,
    .id = 0x2000008,
    .rate = 187000000,
    .mux = 1,
    .parent = CLK_NODE(g_pll_vsn_root),
};
clk_node_t g_ckgen_uuu_vsn_div_p = {
    .core = &g_soc_uuu_core,
    .id = 0x1000008,
    .rate = 748000000,
    .mux = 1,
    .parent = CLK_NODE(g_pll_vsn_root),
};
clk_node_t g_ckgen_uuu_vsn_div_q = {
    .core = &g_soc_uuu_core,
    .id = 0x8,
    .rate = 748000000,
    .mux = 1,
    .parent = CLK_NODE(g_pll_vsn_root),
};
clk_node_t g_ckgen_uuu_ddr_div_m = {
    .core = &g_soc_uuu_core,
    .id = 0x3000009,
    .rate = 1064000000,
    .mux = 1,
    .parent = CLK_NODE(g_pll_ddr_root),
};
clk_node_t g_ckgen_uuu_ddr_div_n = {
    .core = &g_soc_uuu_core,
    .id = 0x2000009,
    .rate = 532000000,
    .mux = 1,
    .parent = CLK_NODE(g_pll_ddr_root),
};
clk_node_t g_ckgen_uuu_ddr_div_p = {
    .core = &g_soc_uuu_core,
    .id = 0x1000009,
    .rate = 266000000,
    .mux = 1,
    .parent = CLK_NODE(g_pll_ddr_root),
};
clk_node_t g_ckgen_uuu_ddr_div_q = {
    .core = &g_soc_uuu_core,
    .id = 0x9,
    .rate = 1064000000,
    .mux = 1,
    .parent = CLK_NODE(g_pll_ddr_root),
};
clk_node_t g_ckgen_uuu_his_div_m = {
    .core = &g_soc_uuu_core,
    .id = 0x300000a,
    .rate = 1000000000,
    .mux = 1,
    .parent = CLK_NODE(g_pll_his_root),
};
clk_node_t g_ckgen_uuu_his_div_n = {
    .core = &g_soc_uuu_core,
    .id = 0x200000a,
    .rate = 100000000,
    .mux = 1,
    .parent = CLK_NODE(g_pll_his_root),
};
clk_node_t g_ckgen_uuu_his_div_p = {
    .core = &g_soc_uuu_core,
    .id = 0x100000a,
    .rate = 500000000,
    .mux = 1,
    .parent = CLK_NODE(g_pll_his_root),
};
clk_node_t g_ckgen_uuu_his_div_q = {
    .core = &g_soc_uuu_core,
    .id = 0xa,
    .rate = 250000000,
    .mux = 1,
    .parent = CLK_NODE(g_pll_his_root),
};
clk_node_t g_ckgen_gating_cr5 = {
    .core = &g_saf_gating_core,
    .id = 0x0,
};
clk_node_t g_ckgen_gating_noc_saf = {
    .core = &g_saf_gating_core,
    .id = 0x1,
};
clk_node_t g_ckgen_gating_noc_mainb = {
    .core = &g_saf_gating_core,
    .id = 0x2,
};
clk_node_t g_ckgen_gating_mac_rdc = {
    .core = &g_saf_gating_core,
    .id = 0x3,
};
clk_node_t g_ckgen_gating_rtc_ss = {
    .core = &g_saf_gating_core,
    .id = 0x4,
};
clk_node_t g_ckgen_gating_gic1 = {
    .core = &g_saf_gating_core,
    .id = 0x6,
};
clk_node_t g_ckgen_gating_dma1 = {
    .core = &g_saf_gating_core,
    .id = 0x7,
};
clk_node_t g_ckgen_gating_eth1 = {
    .core = &g_saf_gating_core,
    .id = 0x8,
};
clk_node_t g_ckgen_gating_ospi1 = {
    .core = &g_saf_gating_core,
    .id = 0x9,
};
clk_node_t g_ckgen_gating_iram1 = {
    .core = &g_saf_gating_core,
    .id = 0xa,
};
clk_node_t g_ckgen_gating_romc1 = {
    .core = &g_saf_gating_core,
    .id = 0xb,
};
clk_node_t g_ckgen_gating_efusec = {
    .core = &g_saf_gating_core,
    .id = 0xc,
};
clk_node_t g_ckgen_gating_ce1 = {
    .core = &g_saf_gating_core,
    .id = 0xd,
};
clk_node_t g_ckgen_gating_i2c1 = {
    .core = &g_saf_gating_core,
    .id = 0xe,
};
clk_node_t g_ckgen_gating_i2c2 = {
    .core = &g_saf_gating_core,
    .id = 0xf,
};
clk_node_t g_ckgen_gating_i2c3 = {
    .core = &g_saf_gating_core,
    .id = 0x10,
};
clk_node_t g_ckgen_gating_i2c4 = {
    .core = &g_saf_gating_core,
    .id = 0x11,
};
clk_node_t g_ckgen_gating_spi1 = {
    .core = &g_saf_gating_core,
    .id = 0x12,
};
clk_node_t g_ckgen_gating_spi2 = {
    .core = &g_saf_gating_core,
    .id = 0x13,
};
clk_node_t g_ckgen_gating_spi3 = {
    .core = &g_saf_gating_core,
    .id = 0x14,
};
clk_node_t g_ckgen_gating_spi4 = {
    .core = &g_saf_gating_core,
    .id = 0x15,
};
clk_node_t g_ckgen_gating_uart1 = {
    .core = &g_saf_gating_core,
    .id = 0x16,
};
clk_node_t g_ckgen_gating_uart2 = {
    .core = &g_saf_gating_core,
    .id = 0x17,
};
clk_node_t g_ckgen_gating_uart3 = {
    .core = &g_saf_gating_core,
    .id = 0x18,
};
clk_node_t g_ckgen_gating_uart4 = {
    .core = &g_saf_gating_core,
    .id = 0x19,
};
clk_node_t g_ckgen_gating_uart5 = {
    .core = &g_saf_gating_core,
    .id = 0x1a,
};
clk_node_t g_ckgen_gating_uart6 = {
    .core = &g_saf_gating_core,
    .id = 0x1b,
};
clk_node_t g_ckgen_gating_uart7 = {
    .core = &g_saf_gating_core,
    .id = 0x1c,
};
clk_node_t g_ckgen_gating_uart8 = {
    .core = &g_saf_gating_core,
    .id = 0x1d,
};
clk_node_t g_ckgen_gating_i2s_sc1 = {
    .core = &g_saf_gating_core,
    .id = 0x1e,
};
clk_node_t g_ckgen_gating_i2s_sc2 = {
    .core = &g_saf_gating_core,
    .id = 0x1f,
};
clk_node_t g_ckgen_gating_tbu11 = {
    .core = &g_saf_gating_core,
    .id = 0x20,
};
clk_node_t g_ckgen_gating_tbu12 = {
    .core = &g_saf_gating_core,
    .id = 0x21,
};
clk_node_t g_ckgen_gating_tbu13 = {
    .core = &g_saf_gating_core,
    .id = 0x22,
};
clk_node_t g_ckgen_gating_timer1 = {
    .core = &g_saf_gating_core,
    .id = 0x25,
};
clk_node_t g_ckgen_gating_timer2 = {
    .core = &g_saf_gating_core,
    .id = 0x26,
};
clk_node_t g_ckgen_gating_pwm1 = {
    .core = &g_saf_gating_core,
    .id = 0x27,
};
clk_node_t g_ckgen_gating_pwm2 = {
    .core = &g_saf_gating_core,
    .id = 0x28,
};
clk_node_t g_ckgen_gating_can1 = {
    .core = &g_saf_gating_core,
    .id = 0x29,
};
clk_node_t g_ckgen_gating_can2 = {
    .core = &g_saf_gating_core,
    .id = 0x2a,
};
clk_node_t g_ckgen_gating_can3 = {
    .core = &g_saf_gating_core,
    .id = 0x2b,
};
clk_node_t g_ckgen_gating_can4 = {
    .core = &g_saf_gating_core,
    .id = 0x2c,
};
clk_node_t g_ckgen_gating_gpio1 = {
    .core = &g_saf_gating_core,
    .id = 0x2d,
};
clk_node_t g_ckgen_gating_scr_saf = {
    .core = &g_saf_gating_core,
    .id = 0x2f,
};
clk_node_t g_ckgen_gating_pvt_sens_saf = {
    .core = &g_saf_gating_core,
    .id = 0x30,
};
clk_node_t g_ckgen_gating_pll1 = {
    .core = &g_saf_gating_core,
    .id = 0x31,
};
clk_node_t g_ckgen_gating_pll2 = {
    .core = &g_saf_gating_core,
    .id = 0x32,
};
clk_node_t g_ckgen_gating_ckgen_saf = {
    .core = &g_saf_gating_core,
    .id = 0x33,
};
clk_node_t g_ckgen_gating_rstgen_saf = {
    .core = &g_saf_gating_core,
    .id = 0x34,
};
clk_node_t g_ckgen_gating_sem1 = {
    .core = &g_saf_gating_core,
    .id = 0x35,
};
clk_node_t g_ckgen_gating_sem2 = {
    .core = &g_saf_gating_core,
    .id = 0x36,
};
clk_node_t g_ckgen_gating_eic_saf = {
    .core = &g_saf_gating_core,
    .id = 0x37,
};
clk_node_t g_ckgen_gating_rpc_saf = {
    .core = &g_saf_gating_core,
    .id = 0x38,
};
clk_node_t g_ckgen_gating_xtal_saf = {
    .core = &g_saf_gating_core,
    .id = 0x39,
};
clk_node_t g_ckgen_gating_bipc_saf = {
    .core = &g_saf_gating_core,
    .id = 0x3a,
};
clk_node_t g_ckgen_gating_sec_plat = {
    .core = &g_sec_gating_core,
    .id = 0x6,
};
clk_node_t g_ckgen_gating_mp_plat = {
    .core = &g_sec_gating_core,
    .id = 0x7,
};
clk_node_t g_ckgen_gating_ce2 = {
    .core = &g_sec_gating_core,
    .id = 0x9,
};
clk_node_t g_ckgen_gating_gic2 = {
    .core = &g_sec_gating_core,
    .id = 0xa,
};
clk_node_t g_ckgen_gating_gic3 = {
    .core = &g_sec_gating_core,
    .id = 0xb,
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
clk_node_t g_ckgen_gating_eth2 = {
    .core = &g_sec_gating_core,
    .id = 0x10,
};
clk_node_t g_ckgen_gating_ospi2 = {
    .core = &g_sec_gating_core,
    .id = 0x11,
};
clk_node_t g_ckgen_gating_dma2 = {
    .core = &g_sec_gating_core,
    .id = 0x12,
};
clk_node_t g_ckgen_gating_dma3 = {
    .core = &g_sec_gating_core,
    .id = 0x13,
};
clk_node_t g_ckgen_gating_dma4 = {
    .core = &g_sec_gating_core,
    .id = 0x14,
};
clk_node_t g_ckgen_gating_dma5 = {
    .core = &g_sec_gating_core,
    .id = 0x15,
};
clk_node_t g_ckgen_gating_dma6 = {
    .core = &g_sec_gating_core,
    .id = 0x16,
};
clk_node_t g_ckgen_gating_dma7 = {
    .core = &g_sec_gating_core,
    .id = 0x17,
};
clk_node_t g_ckgen_gating_dma8 = {
    .core = &g_sec_gating_core,
    .id = 0x18,
};
clk_node_t g_ckgen_gating_mb = {
    .core = &g_sec_gating_core,
    .id = 0x19,
};
clk_node_t g_ckgen_gating_iram2 = {
    .core = &g_sec_gating_core,
    .id = 0x1a,
};
clk_node_t g_ckgen_gating_iram3 = {
    .core = &g_sec_gating_core,
    .id = 0x1b,
};
clk_node_t g_ckgen_gating_iram4 = {
    .core = &g_sec_gating_core,
    .id = 0x1c,
};
clk_node_t g_ckgen_gating_romc2 = {
    .core = &g_sec_gating_core,
    .id = 0x1d,
};
clk_node_t g_ckgen_gating_i2c5 = {
    .core = &g_sec_gating_core,
    .id = 0x1e,
};
clk_node_t g_ckgen_gating_i2c6 = {
    .core = &g_sec_gating_core,
    .id = 0x1f,
};
clk_node_t g_ckgen_gating_i2c7 = {
    .core = &g_sec_gating_core,
    .id = 0x20,
};
clk_node_t g_ckgen_gating_i2c8 = {
    .core = &g_sec_gating_core,
    .id = 0x21,
};
clk_node_t g_ckgen_gating_i2c9 = {
    .core = &g_sec_gating_core,
    .id = 0x22,
};
clk_node_t g_ckgen_gating_i2c10 = {
    .core = &g_sec_gating_core,
    .id = 0x23,
};
clk_node_t g_ckgen_gating_i2c11 = {
    .core = &g_sec_gating_core,
    .id = 0x24,
};
clk_node_t g_ckgen_gating_i2c12 = {
    .core = &g_sec_gating_core,
    .id = 0x25,
};
clk_node_t g_ckgen_gating_i2c13 = {
    .core = &g_sec_gating_core,
    .id = 0x26,
};
clk_node_t g_ckgen_gating_i2c14 = {
    .core = &g_sec_gating_core,
    .id = 0x27,
};
clk_node_t g_ckgen_gating_i2c15 = {
    .core = &g_sec_gating_core,
    .id = 0x28,
};
clk_node_t g_ckgen_gating_i2c16 = {
    .core = &g_sec_gating_core,
    .id = 0x29,
};
clk_node_t g_ckgen_gating_spi5 = {
    .core = &g_sec_gating_core,
    .id = 0x2a,
};
clk_node_t g_ckgen_gating_spi6 = {
    .core = &g_sec_gating_core,
    .id = 0x2b,
};
clk_node_t g_ckgen_gating_spi7 = {
    .core = &g_sec_gating_core,
    .id = 0x2c,
};
clk_node_t g_ckgen_gating_spi8 = {
    .core = &g_sec_gating_core,
    .id = 0x2d,
};
clk_node_t g_ckgen_gating_uart9 = {
    .core = &g_sec_gating_core,
    .id = 0x2e,
};
clk_node_t g_ckgen_gating_uart10 = {
    .core = &g_sec_gating_core,
    .id = 0x2f,
};
clk_node_t g_ckgen_gating_uart11 = {
    .core = &g_sec_gating_core,
    .id = 0x30,
};
clk_node_t g_ckgen_gating_uart12 = {
    .core = &g_sec_gating_core,
    .id = 0x31,
};
clk_node_t g_ckgen_gating_uart13 = {
    .core = &g_sec_gating_core,
    .id = 0x32,
};
clk_node_t g_ckgen_gating_uart14 = {
    .core = &g_sec_gating_core,
    .id = 0x33,
};
clk_node_t g_ckgen_gating_uart15 = {
    .core = &g_sec_gating_core,
    .id = 0x34,
};
clk_node_t g_ckgen_gating_uart16 = {
    .core = &g_sec_gating_core,
    .id = 0x35,
};
clk_node_t g_ckgen_gating_spdif1 = {
    .core = &g_sec_gating_core,
    .id = 0x36,
};
clk_node_t g_ckgen_gating_spdif2 = {
    .core = &g_sec_gating_core,
    .id = 0x37,
};
clk_node_t g_ckgen_gating_spdif3 = {
    .core = &g_sec_gating_core,
    .id = 0x38,
};
clk_node_t g_ckgen_gating_spdif4 = {
    .core = &g_sec_gating_core,
    .id = 0x39,
};
clk_node_t g_ckgen_gating_timer3 = {
    .core = &g_sec_gating_core,
    .id = 0x3a,
};
clk_node_t g_ckgen_gating_timer4 = {
    .core = &g_sec_gating_core,
    .id = 0x3b,
};
clk_node_t g_ckgen_gating_timer5 = {
    .core = &g_sec_gating_core,
    .id = 0x3c,
};
clk_node_t g_ckgen_gating_timer6 = {
    .core = &g_sec_gating_core,
    .id = 0x3d,
};
clk_node_t g_ckgen_gating_timer7 = {
    .core = &g_sec_gating_core,
    .id = 0x3e,
};
clk_node_t g_ckgen_gating_timer8 = {
    .core = &g_sec_gating_core,
    .id = 0x3f,
};
clk_node_t g_ckgen_gating_pwm3 = {
    .core = &g_sec_gating_core,
    .id = 0x40,
};
clk_node_t g_ckgen_gating_pwm4 = {
    .core = &g_sec_gating_core,
    .id = 0x41,
};
clk_node_t g_ckgen_gating_pwm5 = {
    .core = &g_sec_gating_core,
    .id = 0x42,
};
clk_node_t g_ckgen_gating_pwm6 = {
    .core = &g_sec_gating_core,
    .id = 0x43,
};
clk_node_t g_ckgen_gating_pwm7 = {
    .core = &g_sec_gating_core,
    .id = 0x44,
};
clk_node_t g_ckgen_gating_pwm8 = {
    .core = &g_sec_gating_core,
    .id = 0x45,
};
clk_node_t g_ckgen_gating_i2s_mc1 = {
    .core = &g_sec_gating_core,
    .id = 0x46,
};
clk_node_t g_ckgen_gating_i2s_mc2 = {
    .core = &g_sec_gating_core,
    .id = 0x47,
};
clk_node_t g_ckgen_gating_i2s_sc3 = {
    .core = &g_sec_gating_core,
    .id = 0x48,
};
clk_node_t g_ckgen_gating_i2s_sc4 = {
    .core = &g_sec_gating_core,
    .id = 0x49,
};
clk_node_t g_ckgen_gating_i2s_sc5 = {
    .core = &g_sec_gating_core,
    .id = 0x4a,
};
clk_node_t g_ckgen_gating_i2s_sc6 = {
    .core = &g_sec_gating_core,
    .id = 0x4b,
};
clk_node_t g_ckgen_gating_i2s_sc7 = {
    .core = &g_sec_gating_core,
    .id = 0x4c,
};
clk_node_t g_ckgen_gating_i2s_sc8 = {
    .core = &g_sec_gating_core,
    .id = 0x4d,
};
clk_node_t g_ckgen_gating_can5 = {
    .core = &g_sec_gating_core,
    .id = 0x4e,
};
clk_node_t g_ckgen_gating_can6 = {
    .core = &g_sec_gating_core,
    .id = 0x4f,
};
clk_node_t g_ckgen_gating_can7 = {
    .core = &g_sec_gating_core,
    .id = 0x50,
};
clk_node_t g_ckgen_gating_can8 = {
    .core = &g_sec_gating_core,
    .id = 0x51,
};
clk_node_t g_ckgen_gating_trace = {
    .core = &g_sec_gating_core,
    .id = 0x52,
};
clk_node_t g_ckgen_gating_sys_cnt = {
    .core = &g_sec_gating_core,
    .id = 0x53,
};
clk_node_t g_ckgen_gating_tbu14 = {
    .core = &g_sec_gating_core,
    .id = 0x54,
};
clk_node_t g_ckgen_gating_tbu15 = {
    .core = &g_sec_gating_core,
    .id = 0x55,
};
clk_node_t g_ckgen_gating_gpio2 = {
    .core = &g_sec_gating_core,
    .id = 0x56,
};
clk_node_t g_ckgen_gating_gpio3 = {
    .core = &g_sec_gating_core,
    .id = 0x57,
};
clk_node_t g_ckgen_gating_gpio4 = {
    .core = &g_sec_gating_core,
    .id = 0x58,
};
clk_node_t g_ckgen_gating_gpio5 = {
    .core = &g_sec_gating_core,
    .id = 0x59,
};
clk_node_t g_ckgen_gating_wdt3 = {
    .core = &g_sec_gating_core,
    .id = 0x5a,
};
clk_node_t g_ckgen_gating_wdt4 = {
    .core = &g_sec_gating_core,
    .id = 0x5b,
};
clk_node_t g_ckgen_gating_wdt5 = {
    .core = &g_sec_gating_core,
    .id = 0x5c,
};
clk_node_t g_ckgen_gating_wdt6 = {
    .core = &g_sec_gating_core,
    .id = 0x5d,
};
clk_node_t g_ckgen_gating_wdt7 = {
    .core = &g_sec_gating_core,
    .id = 0x5e,
};
clk_node_t g_ckgen_gating_wdt8 = {
    .core = &g_sec_gating_core,
    .id = 0x5f,
};
clk_node_t g_ckgen_gating_pll3 = {
    .core = &g_sec_gating_core,
    .id = 0x60,
};
clk_node_t g_ckgen_gating_pll4 = {
    .core = &g_sec_gating_core,
    .id = 0x61,
};
clk_node_t g_ckgen_gating_pll5 = {
    .core = &g_sec_gating_core,
    .id = 0x62,
};
clk_node_t g_ckgen_gating_pll6 = {
    .core = &g_sec_gating_core,
    .id = 0x63,
};
clk_node_t g_ckgen_gating_pll7 = {
    .core = &g_sec_gating_core,
    .id = 0x64,
};
clk_node_t g_ckgen_gating_can9 = {
    .core = &g_sec_gating_core,
    .id = 0x65,
};
clk_node_t g_ckgen_gating_can10 = {
    .core = &g_sec_gating_core,
    .id = 0x66,
};
clk_node_t g_ckgen_gating_can11 = {
    .core = &g_sec_gating_core,
    .id = 0x67,
};
clk_node_t g_ckgen_gating_can12 = {
    .core = &g_sec_gating_core,
    .id = 0x68,
};
clk_node_t g_ckgen_gating_can13 = {
    .core = &g_sec_gating_core,
    .id = 0x69,
};
clk_node_t g_ckgen_gating_can14 = {
    .core = &g_sec_gating_core,
    .id = 0x6a,
};
clk_node_t g_ckgen_gating_can15 = {
    .core = &g_sec_gating_core,
    .id = 0x6b,
};
clk_node_t g_ckgen_gating_can16 = {
    .core = &g_sec_gating_core,
    .id = 0x6c,
};
clk_node_t g_ckgen_gating_can17 = {
    .core = &g_sec_gating_core,
    .id = 0x6d,
};
clk_node_t g_ckgen_gating_can18 = {
    .core = &g_sec_gating_core,
    .id = 0x6e,
};
clk_node_t g_ckgen_gating_can19 = {
    .core = &g_sec_gating_core,
    .id = 0x6f,
};
clk_node_t g_ckgen_gating_can20 = {
    .core = &g_sec_gating_core,
    .id = 0x70,
};
clk_node_t g_ckgen_gating_adc = {
    .core = &g_sec_gating_core,
    .id = 0x71,
};
clk_node_t g_ckgen_gating_noc_disp = {
    .core = &g_disp_gating_core,
    .id = 0x0,
};
clk_node_t g_ckgen_gating_mipi_dsi1_ext = {
    .core = &g_disp_gating_core,
    .id = 0x1,
};
clk_node_t g_ckgen_gating_mipi_dsi2_ext = {
    .core = &g_disp_gating_core,
    .id = 0x2,
};
clk_node_t g_ckgen_gating_iram5 = {
    .core = &g_disp_gating_core,
    .id = 0x5,
};
clk_node_t g_ckgen_gating_xtal_ap = {
    .core = &g_disp_gating_core,
    .id = 0x6,
};
clk_node_t g_ckgen_gating_pll_lvds1 = {
    .core = &g_disp_gating_core,
    .id = 0x7,
};
clk_node_t g_ckgen_gating_pll_lvds2 = {
    .core = &g_disp_gating_core,
    .id = 0x8,
};
clk_node_t g_ckgen_gating_pll_lvds3 = {
    .core = &g_disp_gating_core,
    .id = 0x9,
};
clk_node_t g_ckgen_gating_pll_lvds4 = {
    .core = &g_disp_gating_core,
    .id = 0xa,
};
clk_node_t g_ckgen_gating_disp = {
    .core = &g_disp_gating_core,
    .id = 0xb,
};
clk_node_t g_ckgen_gating_dc1 = {
    .core = &g_disp_gating_core,
    .id = 0xc,
};
clk_node_t g_ckgen_gating_dc2 = {
    .core = &g_disp_gating_core,
    .id = 0xd,
};
clk_node_t g_ckgen_gating_dc3 = {
    .core = &g_disp_gating_core,
    .id = 0xe,
};
clk_node_t g_ckgen_gating_dc4 = {
    .core = &g_disp_gating_core,
    .id = 0xf,
};
clk_node_t g_ckgen_gating_dc5 = {
    .core = &g_disp_gating_core,
    .id = 0x10,
};
clk_node_t g_ckgen_gating_dp1 = {
    .core = &g_disp_gating_core,
    .id = 0x11,
};
clk_node_t g_ckgen_gating_dp2 = {
    .core = &g_disp_gating_core,
    .id = 0x12,
};
clk_node_t g_ckgen_gating_dp3 = {
    .core = &g_disp_gating_core,
    .id = 0x13,
};
clk_node_t g_ckgen_gating_g2d1 = {
    .core = &g_disp_gating_core,
    .id = 0x14,
};
clk_node_t g_ckgen_gating_g2d2 = {
    .core = &g_disp_gating_core,
    .id = 0x15,
};
clk_node_t g_ckgen_gating_mipi_dsi1 = {
    .core = &g_disp_gating_core,
    .id = 0x16,
};
clk_node_t g_ckgen_gating_mipi_dsi2 = {
    .core = &g_disp_gating_core,
    .id = 0x17,
};
clk_node_t g_ckgen_gating_lvds_ss = {
    .core = &g_disp_gating_core,
    .id = 0x18,
};
clk_node_t g_ckgen_gating_csi1 = {
    .core = &g_disp_gating_core,
    .id = 0x19,
};
clk_node_t g_ckgen_gating_csi2 = {
    .core = &g_disp_gating_core,
    .id = 0x1a,
};
clk_node_t g_ckgen_gating_csi3 = {
    .core = &g_disp_gating_core,
    .id = 0x1b,
};
clk_node_t g_ckgen_gating_disp_mux = {
    .core = &g_disp_gating_core,
    .id = 0x1c,
};
clk_node_t g_ckgen_gating_mipi_csi1 = {
    .core = &g_disp_gating_core,
    .id = 0x1d,
};
clk_node_t g_ckgen_gating_mipi_csi2 = {
    .core = &g_disp_gating_core,
    .id = 0x1e,
};
clk_node_t g_ckgen_gating_tcu = {
    .core = &g_soc_gating_core,
    .id = 0x6,
};
clk_node_t g_ckgen_gating_tbu0 = {
    .core = &g_soc_gating_core,
    .id = 0x7,
};
clk_node_t g_ckgen_gating_tbu1 = {
    .core = &g_soc_gating_core,
    .id = 0x8,
};
clk_node_t g_ckgen_gating_tbu2 = {
    .core = &g_soc_gating_core,
    .id = 0x9,
};
clk_node_t g_ckgen_gating_tbu3 = {
    .core = &g_soc_gating_core,
    .id = 0xa,
};
clk_node_t g_ckgen_gating_tbu4 = {
    .core = &g_soc_gating_core,
    .id = 0xb,
};
clk_node_t g_ckgen_gating_tbu5 = {
    .core = &g_soc_gating_core,
    .id = 0xc,
};
clk_node_t g_ckgen_gating_tbu6 = {
    .core = &g_soc_gating_core,
    .id = 0xd,
};
clk_node_t g_ckgen_gating_tbu7 = {
    .core = &g_soc_gating_core,
    .id = 0xe,
};
clk_node_t g_ckgen_gating_tbu8 = {
    .core = &g_soc_gating_core,
    .id = 0xf,
};
clk_node_t g_ckgen_gating_tbu9 = {
    .core = &g_soc_gating_core,
    .id = 0x10,
};
clk_node_t g_ckgen_gating_tbu10 = {
    .core = &g_soc_gating_core,
    .id = 0x11,
};
clk_node_t g_ckgen_gating_scr4k_sid = {
    .core = &g_soc_gating_core,
    .id = 0x13,
};
clk_node_t g_ckgen_gating_scr4k_ssid = {
    .core = &g_soc_gating_core,
    .id = 0x14,
};
clk_node_t g_ckgen_gating_bipc_ddr = {
    .core = &g_soc_gating_core,
    .id = 0x15,
};
clk_node_t g_ckgen_gating_eic_hpi = {
    .core = &g_soc_gating_core,
    .id = 0x16,
};
clk_node_t g_ckgen_gating_noc_main = {
    .core = &g_soc_gating_core,
    .id = 0x17,
};
clk_node_t g_ckgen_gating_gic4 = {
    .core = &g_soc_gating_core,
    .id = 0x18,
};
clk_node_t g_ckgen_gating_gic5 = {
    .core = &g_soc_gating_core,
    .id = 0x19,
};
clk_node_t g_ckgen_gating_pll_cpu1 = {
    .core = &g_soc_gating_core,
    .id = 0x1b,
};
clk_node_t g_ckgen_gating_cpu2 = {
    .core = &g_soc_gating_core,
    .id = 0x1c,
};
clk_node_t g_ckgen_gating_pll_cpu2 = {
    .core = &g_soc_gating_core,
    .id = 0x1d,
};
clk_node_t g_ckgen_gating_gpu1 = {
    .core = &g_soc_gating_core,
    .id = 0x1e,
};
clk_node_t g_ckgen_gating_pll_gpu1 = {
    .core = &g_soc_gating_core,
    .id = 0x1f,
};
clk_node_t g_ckgen_gating_gpu2 = {
    .core = &g_soc_gating_core,
    .id = 0x20,
};
clk_node_t g_ckgen_gating_pll_gpu2 = {
    .core = &g_soc_gating_core,
    .id = 0x21,
};
clk_node_t g_ckgen_gating_vpu1 = {
    .core = &g_soc_gating_core,
    .id = 0x22,
};
clk_node_t g_ckgen_gating_vpu2 = {
    .core = &g_soc_gating_core,
    .id = 0x23,
};
clk_node_t g_ckgen_gating_mjpeg = {
    .core = &g_soc_gating_core,
    .id = 0x24,
};
clk_node_t g_ckgen_gating_pll_vpu = {
    .core = &g_soc_gating_core,
    .id = 0x26,
};
clk_node_t g_ckgen_gating_vdsp = {
    .core = &g_soc_gating_core,
    .id = 0x27,
};
clk_node_t g_ckgen_gating_noc_vsn = {
    .core = &g_soc_gating_core,
    .id = 0x28,
};
clk_node_t g_ckgen_gating_pll_vsn = {
    .core = &g_soc_gating_core,
    .id = 0x29,
};
clk_node_t g_ckgen_gating_bipc_vdsp = {
    .core = &g_soc_gating_core,
    .id = 0x2a,
};
clk_node_t g_ckgen_gating_eic_vsn = {
    .core = &g_soc_gating_core,
    .id = 0x2b,
};
clk_node_t g_ckgen_gating_noc_his = {
    .core = &g_soc_gating_core,
    .id = 0x2c,
};
clk_node_t g_ckgen_gating_pll_his = {
    .core = &g_soc_gating_core,
    .id = 0x2d,
};
clk_node_t g_ckgen_gating_pcie2_mstr = {
    .core = &g_soc_gating_core,
    .id = 0x2e,
};
clk_node_t g_ckgen_gating_pcie1_mstr = {
    .core = &g_soc_gating_core,
    .id = 0x2f,
};
clk_node_t g_ckgen_gating_pcie_phy = {
    .core = &g_soc_gating_core,
    .id = 0x30,
};
clk_node_t g_ckgen_gating_pcie_phy_ref = {
    .core = &g_soc_gating_core,
    .id = 0x31,
};
clk_node_t g_ckgen_gating_usb1 = {
    .core = &g_soc_gating_core,
    .id = 0x32,
};
clk_node_t g_ckgen_gating_usb2 = {
    .core = &g_soc_gating_core,
    .id = 0x33,
};
clk_node_t g_ckgen_gating_usb1_ref = {
    .core = &g_soc_gating_core,
    .id = 0x34,
};
clk_node_t g_ckgen_gating_usb2_ref = {
    .core = &g_soc_gating_core,
    .id = 0x35,
};
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
clk_node_t g_ckgen_gating_cpu1_clk4 = {
    .core = &g_soc_gating_core,
    .id = 0x3a,
};
clk_node_t g_ckgen_gating_cpu1_clk5 = {
    .core = &g_soc_gating_core,
    .id = 0x3b,
};
const clk_node_t *board_clk_prepare_array[] = {
    CLK_NODE(g_ckgen_bus_saf_plat_div_q),
    CLK_NODE(g_ckgen_bus_saf_plat_div_p),
    CLK_NODE(g_ckgen_bus_saf_plat_div_n),
    CLK_NODE(g_ckgen_bus_saf_plat_div_m),
    CLK_NODE(g_ckgen_bus_saf_plat_root),
    CLK_NODE(g_pll1_vco),
    CLK_NODE(g_pll1_root),
    CLK_NODE(g_pll1_div_a),
    CLK_NODE(g_pll1_div_b),
    CLK_NODE(g_pll1_div_c),
    CLK_NODE(g_pll1_div_d),
    CLK_NODE(g_ckgen_ip_ce1),
    CLK_NODE(g_ckgen_ip_i2c_saf),
    CLK_NODE(g_ckgen_ip_spi_saf),
    CLK_NODE(g_ckgen_ip_uart_saf),
    CLK_NODE(g_ckgen_ip_i2s_mclk1),
    CLK_NODE(g_ckgen_ip_i2s_sc1),
    CLK_NODE(g_ckgen_ip_i2s_sc2),
    CLK_NODE(g_ckgen_ip_eth1_tx),
    CLK_NODE(g_ckgen_ip_eth1_rmii),
    CLK_NODE(g_ckgen_ip_eth1_phy_ref),
    CLK_NODE(g_ckgen_ip_eth1_timer_sec),
    CLK_NODE(g_ckgen_ip_ospi1),
    CLK_NODE(g_ckgen_ip_timer1),
    CLK_NODE(g_ckgen_ip_timer2),
    CLK_NODE(g_ckgen_ip_pwm1),
    CLK_NODE(g_ckgen_ip_pwm2),
    CLK_NODE(g_ckgen_ip_can_1_to_4),
    CLK_NODE(g_pll6_vco),
    CLK_NODE(g_pll6_root),
    CLK_NODE(g_pll6_div_a),
    CLK_NODE(g_pll6_div_b),
    CLK_NODE(g_pll6_div_c),
    CLK_NODE(g_pll6_div_d),
    CLK_NODE(g_pll7_vco),
    CLK_NODE(g_pll7_root),
    CLK_NODE(g_pll7_div_a),
    CLK_NODE(g_pll7_div_b),
    CLK_NODE(g_pll7_div_c),
    CLK_NODE(g_pll7_div_d),
    CLK_NODE(g_ckgen_ip_i2c_sec0),
    CLK_NODE(g_ckgen_ip_i2c_sec1),
    CLK_NODE(g_ckgen_ip_spi_sec0),
    CLK_NODE(g_ckgen_ip_spi_sec1),
    CLK_NODE(g_ckgen_ip_uart_sec0),
    CLK_NODE(g_ckgen_ip_uart_sec1),
    CLK_NODE(g_ckgen_ip_emmc1),
    CLK_NODE(g_ckgen_ip_emmc2),
    CLK_NODE(g_ckgen_ip_emmc3),
    CLK_NODE(g_ckgen_ip_emmc4),
    CLK_NODE(g_ckgen_ip_eth2_tx),
    CLK_NODE(g_ckgen_ip_eth2_rmii),
    CLK_NODE(g_ckgen_ip_eth2_phy_ref),
    CLK_NODE(g_ckgen_ip_eth2_timer_sec),
    CLK_NODE(g_ckgen_ip_spdif1),
    CLK_NODE(g_ckgen_ip_spdif2),
    CLK_NODE(g_ckgen_ip_spdif3),
    CLK_NODE(g_ckgen_ip_spdif4),
    CLK_NODE(g_ckgen_ip_ospi2),
    CLK_NODE(g_ckgen_ip_timer3),
    CLK_NODE(g_ckgen_ip_timer4),
    CLK_NODE(g_ckgen_ip_timer5),
    CLK_NODE(g_ckgen_ip_timer6),
    CLK_NODE(g_ckgen_ip_timer7),
    CLK_NODE(g_ckgen_ip_timer8),
    CLK_NODE(g_ckgen_ip_pwm3),
    CLK_NODE(g_ckgen_ip_pwm4),
    CLK_NODE(g_ckgen_ip_pwm5),
    CLK_NODE(g_ckgen_ip_pwm6),
    CLK_NODE(g_ckgen_ip_pwm7),
    CLK_NODE(g_ckgen_ip_pwm8),
    CLK_NODE(g_ckgen_ip_i2s_mclk2),
    CLK_NODE(g_ckgen_ip_i2s_mclk3),
    CLK_NODE(g_ckgen_ip_i2s_mc1),
    CLK_NODE(g_ckgen_ip_i2s_mc2),
    CLK_NODE(g_ckgen_ip_i2s_sc3),
    CLK_NODE(g_ckgen_ip_i2s_sc4),
    CLK_NODE(g_ckgen_ip_i2s_sc5),
    CLK_NODE(g_ckgen_ip_i2s_sc6),
    CLK_NODE(g_ckgen_ip_i2s_sc7),
    CLK_NODE(g_ckgen_ip_i2s_sc8),
    CLK_NODE(g_ckgen_ip_csi_mclk1),
    CLK_NODE(g_ckgen_ip_csi_mclk2),
    CLK_NODE(g_ckgen_ip_gic_4_5),
    CLK_NODE(g_ckgen_ip_trace),
    CLK_NODE(g_ckgen_ip_sys_cnt),
    CLK_NODE(g_ckgen_ip_mshc_timer),
    CLK_NODE(g_ckgen_ip_hpi_clk600),
    CLK_NODE(g_ckgen_ip_hpi_clk800),
    CLK_NODE(g_pll_disp_vco),
    CLK_NODE(g_pll_disp_root),
    CLK_NODE(g_pll_disp_div_a),
    CLK_NODE(g_pll_disp_div_b),
    CLK_NODE(g_pll_lvds1_vco),
    CLK_NODE(g_pll_lvds1_root),
    CLK_NODE(g_pll_lvds1_div_a),
    CLK_NODE(g_pll_lvds1_div_b),
    CLK_NODE(g_pll_lvds2_vco),
    CLK_NODE(g_pll_lvds2_root),
    CLK_NODE(g_pll_lvds2_div_a),
    CLK_NODE(g_pll_lvds2_div_b),
    CLK_NODE(g_pll_lvds3_vco),
    CLK_NODE(g_pll_lvds3_root),
    CLK_NODE(g_pll_lvds3_div_a),
    CLK_NODE(g_pll_lvds3_div_b),
    CLK_NODE(g_pll_lvds4_vco),
    CLK_NODE(g_pll_lvds4_root),
    CLK_NODE(g_pll_lvds4_div_a),
    CLK_NODE(g_pll_lvds4_div_b),
    CLK_NODE(g_ckgen_ip_mipi_csi1_pix),
    CLK_NODE(g_ckgen_ip_mipi_csi2_pix),
    CLK_NODE(g_ckgen_ip_csi3),
    CLK_NODE(g_ckgen_ip_dp1),
    CLK_NODE(g_ckgen_ip_dp2),
    CLK_NODE(g_ckgen_ip_dp3),
    CLK_NODE(g_ckgen_ip_dc5),
    CLK_NODE(g_ckgen_ip_dc1),
    CLK_NODE(g_ckgen_ip_dc2),
    CLK_NODE(g_ckgen_ip_dc3),
    CLK_NODE(g_ckgen_ip_dc4),
    CLK_NODE(g_ckgen_ip_spare1),
    CLK_NODE(g_ckgen_ip_dc5_alt_dsp_clk),
    CLK_NODE(g_ckgen_bus_disp_bus_root),
    CLK_NODE(g_ckgen_bus_disp_bus_div_m),
    CLK_NODE(g_ckgen_bus_disp_bus_div_n),
    CLK_NODE(g_ckgen_bus_disp_bus_div_p),
    CLK_NODE(g_ckgen_bus_disp_bus_div_q),
    CLK_NODE(g_ckgen_ip_ext_aud1),
    CLK_NODE(g_ckgen_ip_ext_aud2),
    CLK_NODE(g_ckgen_ip_ext_aud3),
    CLK_NODE(g_ckgen_ip_ext_aud4),
    CLK_NODE(g_ckgen_core_cpu1a_pre),
    CLK_NODE(g_ckgen_uuu_cpu1a_div_m_pre),
    CLK_NODE(g_ckgen_uuu_cpu1a_div_n_pre),
    CLK_NODE(g_ckgen_uuu_cpu1a_div_p_pre),
    CLK_NODE(g_ckgen_uuu_cpu1a_div_q_pre),
    CLK_NODE(g_pll_cpu1a_vco),
    CLK_NODE(g_pll_cpu1a_root),
    CLK_NODE(g_pll_cpu1a_div_a),
    CLK_NODE(g_pll_cpu1a_div_b),
    CLK_NODE(g_ckgen_core_cpu1b_pre),
    CLK_NODE(g_ckgen_uuu_cpu1b_div_m_pre),
    CLK_NODE(g_ckgen_uuu_cpu1b_div_n_pre),
    CLK_NODE(g_ckgen_uuu_cpu1b_div_p_pre),
    CLK_NODE(g_ckgen_uuu_cpu1b_div_q_pre),
    CLK_NODE(g_pll_cpu1b_vco),
    CLK_NODE(g_pll_cpu1b_root),
    CLK_NODE(g_pll_cpu1b_div_a),
    CLK_NODE(g_pll_cpu1b_div_b),
    CLK_NODE(g_ckgen_core_gpu1_pre),
    CLK_NODE(g_ckgen_uuu_gpu1_div_m_pre),
    CLK_NODE(g_ckgen_uuu_gpu1_div_n_pre),
    CLK_NODE(g_ckgen_uuu_gpu1_div_p_pre),
    CLK_NODE(g_ckgen_uuu_gpu1_div_q_pre),
    CLK_NODE(g_pll_gpu1_vco),
    CLK_NODE(g_pll_gpu1_root),
    CLK_NODE(g_pll_gpu1_div_a),
    CLK_NODE(g_pll_gpu1_div_b),
    CLK_NODE(g_ckgen_core_gpu2_pre),
    CLK_NODE(g_ckgen_uuu_gpu2_div_m_pre),
    CLK_NODE(g_ckgen_uuu_gpu2_div_n_pre),
    CLK_NODE(g_ckgen_uuu_gpu2_div_p_pre),
    CLK_NODE(g_ckgen_uuu_gpu2_div_q_pre),
    CLK_NODE(g_pll_gpu2_vco),
    CLK_NODE(g_pll_gpu2_root),
    CLK_NODE(g_pll_gpu2_div_a),
    CLK_NODE(g_pll_gpu2_div_b),
    CLK_NODE(g_ckgen_ip_vpu1_pre),
    CLK_NODE(g_ckgen_ip_mjpeg_pre),
    CLK_NODE(g_ckgen_bus_vpu_root_pre),
    CLK_NODE(g_ckgen_bus_vpu_div_m_pre),
    CLK_NODE(g_ckgen_bus_vpu_div_n_pre),
    CLK_NODE(g_ckgen_bus_vpu_div_p_pre),
    CLK_NODE(g_ckgen_bus_vpu_div_q_pre),
    CLK_NODE(g_ckgen_uuu_vpu1_div_m_pre),
    CLK_NODE(g_ckgen_uuu_vpu1_div_n_pre),
    CLK_NODE(g_ckgen_uuu_vpu1_div_p_pre),
    CLK_NODE(g_ckgen_uuu_vpu1_div_q_pre),
    CLK_NODE(g_ckgen_uuu_mjpeg_div_m_pre),
    CLK_NODE(g_ckgen_uuu_mjpeg_div_n_pre),
    CLK_NODE(g_ckgen_uuu_mjpeg_div_p_pre),
    CLK_NODE(g_ckgen_uuu_mjpeg_div_q_pre),
    CLK_NODE(g_ckgen_uuu_vpu_div_m_pre),
    CLK_NODE(g_ckgen_uuu_vpu_div_n_pre),
    CLK_NODE(g_ckgen_uuu_vpu_div_p_pre),
    CLK_NODE(g_ckgen_uuu_vpu_div_q_pre),
    CLK_NODE(g_pll_vpu_vco),
    CLK_NODE(g_pll_vpu_root),
    CLK_NODE(g_pll_vpu_div_a),
    CLK_NODE(g_pll_vpu_div_b),
    CLK_NODE(g_ckgen_bus_vsn_root_pre),
    CLK_NODE(g_ckgen_bus_vsn_div_m_pre),
    CLK_NODE(g_ckgen_bus_vsn_div_n_pre),
    CLK_NODE(g_ckgen_bus_vsn_div_p_pre),
    CLK_NODE(g_ckgen_bus_vsn_div_q_pre),
    CLK_NODE(g_ckgen_uuu_vsn_div_m_pre),
    CLK_NODE(g_ckgen_uuu_vsn_div_n_pre),
    CLK_NODE(g_ckgen_uuu_vsn_div_p_pre),
    CLK_NODE(g_ckgen_uuu_vsn_div_q_pre),
    CLK_NODE(g_pll_vsn_vco),
    CLK_NODE(g_pll_vsn_root),
    CLK_NODE(g_pll_vsn_div_a),
    CLK_NODE(g_pll_vsn_div_b),
    CLK_NODE(g_pll_hpi_vco),
    CLK_NODE(g_pll_hpi_root),
    CLK_NODE(g_pll_hpi_div_a),
    CLK_NODE(g_pll_hpi_div_b),
    CLK_NODE(g_ckgen_core_cpu1a),
    CLK_NODE(g_ckgen_core_cpu1b),
    CLK_NODE(g_ckgen_core_gpu1),
    CLK_NODE(g_ckgen_core_gpu2),
    CLK_NODE(g_ckgen_ip_vpu1),
    CLK_NODE(g_ckgen_ip_mjpeg),
    CLK_NODE(g_ckgen_bus_vpu_root),
    CLK_NODE(g_ckgen_bus_vpu_div_m),
    CLK_NODE(g_ckgen_bus_vpu_div_n),
    CLK_NODE(g_ckgen_bus_vpu_div_p),
    CLK_NODE(g_ckgen_bus_vpu_div_q),
    CLK_NODE(g_ckgen_bus_vsn_root),
    CLK_NODE(g_ckgen_bus_vsn_div_m),
    CLK_NODE(g_ckgen_bus_vsn_div_n),
    CLK_NODE(g_ckgen_bus_vsn_div_p),
    CLK_NODE(g_ckgen_bus_vsn_div_q),
    CLK_NODE(g_ckgen_bus_noc_root),
    CLK_NODE(g_ckgen_bus_noc_div_m),
    CLK_NODE(g_ckgen_bus_noc_div_n),
    CLK_NODE(g_ckgen_bus_noc_div_p),
    CLK_NODE(g_ckgen_bus_noc_div_q),
    CLK_NODE(g_ckgen_bus_his_root),
    CLK_NODE(g_ckgen_bus_his_div_m),
    CLK_NODE(g_ckgen_bus_his_div_n),
    CLK_NODE(g_ckgen_bus_his_div_p),
    CLK_NODE(g_ckgen_bus_his_div_q),
    CLK_NODE(g_ckgen_uuu_cpu1a_div_m),
    CLK_NODE(g_ckgen_uuu_cpu1a_div_n),
    CLK_NODE(g_ckgen_uuu_cpu1a_div_p),
    CLK_NODE(g_ckgen_uuu_cpu1a_div_q),
    CLK_NODE(g_ckgen_uuu_cpu1b_div_m),
    CLK_NODE(g_ckgen_uuu_cpu1b_div_n),
    CLK_NODE(g_ckgen_uuu_cpu1b_div_p),
    CLK_NODE(g_ckgen_uuu_cpu1b_div_q),
    CLK_NODE(g_ckgen_uuu_gpu1_div_m),
    CLK_NODE(g_ckgen_uuu_gpu1_div_n),
    CLK_NODE(g_ckgen_uuu_gpu1_div_p),
    CLK_NODE(g_ckgen_uuu_gpu1_div_q),
    CLK_NODE(g_ckgen_uuu_gpu2_div_m),
    CLK_NODE(g_ckgen_uuu_gpu2_div_n),
    CLK_NODE(g_ckgen_uuu_gpu2_div_p),
    CLK_NODE(g_ckgen_uuu_gpu2_div_q),
    CLK_NODE(g_ckgen_uuu_vpu1_div_m),
    CLK_NODE(g_ckgen_uuu_vpu1_div_n),
    CLK_NODE(g_ckgen_uuu_vpu1_div_p),
    CLK_NODE(g_ckgen_uuu_vpu1_div_q),
    CLK_NODE(g_ckgen_uuu_mjpeg_div_m),
    CLK_NODE(g_ckgen_uuu_mjpeg_div_n),
    CLK_NODE(g_ckgen_uuu_mjpeg_div_p),
    CLK_NODE(g_ckgen_uuu_mjpeg_div_q),
    CLK_NODE(g_ckgen_uuu_vpu_div_m),
    CLK_NODE(g_ckgen_uuu_vpu_div_n),
    CLK_NODE(g_ckgen_uuu_vpu_div_p),
    CLK_NODE(g_ckgen_uuu_vpu_div_q),
    CLK_NODE(g_ckgen_uuu_vsn_div_m),
    CLK_NODE(g_ckgen_uuu_vsn_div_n),
    CLK_NODE(g_ckgen_uuu_vsn_div_p),
    CLK_NODE(g_ckgen_uuu_vsn_div_q),
    CLK_NODE(g_ckgen_uuu_his_div_m),
    CLK_NODE(g_ckgen_uuu_his_div_n),
    CLK_NODE(g_ckgen_uuu_his_div_p),
    CLK_NODE(g_ckgen_uuu_his_div_q),
};

const clk_node_t *board_clk_enable_array[] = {
    CLK_NODE(g_ckgen_gating_cr5),
    CLK_NODE(g_ckgen_gating_noc_saf),
    CLK_NODE(g_ckgen_gating_noc_mainb),
    CLK_NODE(g_ckgen_gating_mac_rdc),
    CLK_NODE(g_ckgen_gating_rtc_ss),
    CLK_NODE(g_ckgen_gating_gic1),
    CLK_NODE(g_ckgen_gating_dma1),
    CLK_NODE(g_ckgen_gating_eth1),
    CLK_NODE(g_ckgen_gating_ospi1),
    CLK_NODE(g_ckgen_gating_iram1),
    CLK_NODE(g_ckgen_gating_romc1),
    CLK_NODE(g_ckgen_gating_efusec),
    CLK_NODE(g_ckgen_gating_ce1),
    CLK_NODE(g_ckgen_gating_i2c1),
    CLK_NODE(g_ckgen_gating_i2c2),
    CLK_NODE(g_ckgen_gating_i2c3),
    CLK_NODE(g_ckgen_gating_i2c4),
    CLK_NODE(g_ckgen_gating_spi1),
    CLK_NODE(g_ckgen_gating_spi2),
    CLK_NODE(g_ckgen_gating_spi3),
    CLK_NODE(g_ckgen_gating_spi4),
    CLK_NODE(g_ckgen_gating_uart3),
    CLK_NODE(g_ckgen_gating_uart4),
    CLK_NODE(g_ckgen_gating_uart5),
    CLK_NODE(g_ckgen_gating_uart6),
    CLK_NODE(g_ckgen_gating_uart7),
    CLK_NODE(g_ckgen_gating_uart8),
    CLK_NODE(g_ckgen_gating_i2s_sc1),
    CLK_NODE(g_ckgen_gating_i2s_sc2),
    CLK_NODE(g_ckgen_gating_tbu11),
    CLK_NODE(g_ckgen_gating_tbu12),
    CLK_NODE(g_ckgen_gating_tbu13),
    CLK_NODE(g_ckgen_gating_timer1),
    CLK_NODE(g_ckgen_gating_timer2),
    CLK_NODE(g_ckgen_gating_pwm1),
    CLK_NODE(g_ckgen_gating_pwm2),
    CLK_NODE(g_ckgen_gating_can1),
    CLK_NODE(g_ckgen_gating_can2),
    CLK_NODE(g_ckgen_gating_can3),
    CLK_NODE(g_ckgen_gating_can4),
    CLK_NODE(g_ckgen_gating_gpio1),
    CLK_NODE(g_ckgen_gating_scr_saf),
    CLK_NODE(g_ckgen_gating_pvt_sens_saf),
    CLK_NODE(g_ckgen_gating_pll1),
    CLK_NODE(g_ckgen_gating_pll2),
    CLK_NODE(g_ckgen_gating_ckgen_saf),
    CLK_NODE(g_ckgen_gating_rstgen_saf),
    CLK_NODE(g_ckgen_gating_sem1),
    CLK_NODE(g_ckgen_gating_sem2),
    CLK_NODE(g_ckgen_gating_eic_saf),
    CLK_NODE(g_ckgen_gating_rpc_saf),
    CLK_NODE(g_ckgen_gating_xtal_saf),
    CLK_NODE(g_ckgen_gating_bipc_saf),
    CLK_NODE(g_ckgen_gating_sec_plat),
    CLK_NODE(g_ckgen_gating_mp_plat),
    CLK_NODE(g_ckgen_gating_ce2),
    CLK_NODE(g_ckgen_gating_gic2),
    CLK_NODE(g_ckgen_gating_gic3),
    CLK_NODE(g_ckgen_gating_emmc1),
    CLK_NODE(g_ckgen_gating_emmc2),
    CLK_NODE(g_ckgen_gating_emmc3),
    CLK_NODE(g_ckgen_gating_emmc4),
    CLK_NODE(g_ckgen_gating_eth2),
    CLK_NODE(g_ckgen_gating_ospi2),
    CLK_NODE(g_ckgen_gating_dma2),
    CLK_NODE(g_ckgen_gating_dma3),
    CLK_NODE(g_ckgen_gating_dma4),
    CLK_NODE(g_ckgen_gating_dma5),
    CLK_NODE(g_ckgen_gating_dma6),
    CLK_NODE(g_ckgen_gating_dma7),
    CLK_NODE(g_ckgen_gating_dma8),
    CLK_NODE(g_ckgen_gating_mb),
    CLK_NODE(g_ckgen_gating_iram2),
    CLK_NODE(g_ckgen_gating_iram3),
    CLK_NODE(g_ckgen_gating_iram4),
    CLK_NODE(g_ckgen_gating_romc2),
    CLK_NODE(g_ckgen_gating_i2c5),
    CLK_NODE(g_ckgen_gating_i2c6),
    CLK_NODE(g_ckgen_gating_i2c7),
    CLK_NODE(g_ckgen_gating_i2c8),
    CLK_NODE(g_ckgen_gating_i2c9),
    CLK_NODE(g_ckgen_gating_i2c10),
    CLK_NODE(g_ckgen_gating_i2c11),
    CLK_NODE(g_ckgen_gating_i2c12),
    CLK_NODE(g_ckgen_gating_i2c13),
    CLK_NODE(g_ckgen_gating_i2c14),
    CLK_NODE(g_ckgen_gating_i2c15),
    CLK_NODE(g_ckgen_gating_i2c16),
    CLK_NODE(g_ckgen_gating_spi5),
    CLK_NODE(g_ckgen_gating_spi6),
    CLK_NODE(g_ckgen_gating_spi7),
    CLK_NODE(g_ckgen_gating_spi8),
    CLK_NODE(g_ckgen_gating_uart9),
    CLK_NODE(g_ckgen_gating_uart10),
    CLK_NODE(g_ckgen_gating_uart11),
    CLK_NODE(g_ckgen_gating_uart12),
    CLK_NODE(g_ckgen_gating_uart13),
    CLK_NODE(g_ckgen_gating_uart14),
    CLK_NODE(g_ckgen_gating_uart15),
    CLK_NODE(g_ckgen_gating_uart16),
    CLK_NODE(g_ckgen_gating_spdif1),
    CLK_NODE(g_ckgen_gating_spdif2),
    CLK_NODE(g_ckgen_gating_spdif3),
    CLK_NODE(g_ckgen_gating_spdif4),
    CLK_NODE(g_ckgen_gating_timer3),
    CLK_NODE(g_ckgen_gating_timer4),
    CLK_NODE(g_ckgen_gating_timer5),
    CLK_NODE(g_ckgen_gating_timer6),
    CLK_NODE(g_ckgen_gating_timer7),
    CLK_NODE(g_ckgen_gating_timer8),
    CLK_NODE(g_ckgen_gating_pwm3),
    CLK_NODE(g_ckgen_gating_pwm4),
    CLK_NODE(g_ckgen_gating_pwm5),
    CLK_NODE(g_ckgen_gating_pwm6),
    CLK_NODE(g_ckgen_gating_pwm7),
    CLK_NODE(g_ckgen_gating_pwm8),
    CLK_NODE(g_ckgen_gating_i2s_mc1),
    CLK_NODE(g_ckgen_gating_i2s_mc2),
    CLK_NODE(g_ckgen_gating_i2s_sc3),
    CLK_NODE(g_ckgen_gating_i2s_sc4),
    CLK_NODE(g_ckgen_gating_i2s_sc5),
    CLK_NODE(g_ckgen_gating_i2s_sc6),
    CLK_NODE(g_ckgen_gating_i2s_sc7),
    CLK_NODE(g_ckgen_gating_i2s_sc8),
    CLK_NODE(g_ckgen_gating_trace),
    CLK_NODE(g_ckgen_gating_sys_cnt),
    CLK_NODE(g_ckgen_gating_tbu14),
    CLK_NODE(g_ckgen_gating_tbu15),
    CLK_NODE(g_ckgen_gating_gpio2),
    CLK_NODE(g_ckgen_gating_gpio3),
    CLK_NODE(g_ckgen_gating_gpio4),
    CLK_NODE(g_ckgen_gating_gpio5),
    CLK_NODE(g_ckgen_gating_wdt3),
    CLK_NODE(g_ckgen_gating_wdt4),
    CLK_NODE(g_ckgen_gating_wdt5),
    CLK_NODE(g_ckgen_gating_wdt6),
    CLK_NODE(g_ckgen_gating_wdt7),
    CLK_NODE(g_ckgen_gating_wdt8),
    CLK_NODE(g_ckgen_gating_pll3),
    CLK_NODE(g_ckgen_gating_pll4),
    CLK_NODE(g_ckgen_gating_pll5),
    CLK_NODE(g_ckgen_gating_pll6),
    CLK_NODE(g_ckgen_gating_pll7),
    CLK_NODE(g_ckgen_gating_adc),
    CLK_NODE(g_ckgen_gating_noc_disp),
    CLK_NODE(g_ckgen_gating_mipi_dsi1_ext),
    CLK_NODE(g_ckgen_gating_mipi_dsi2_ext),
    CLK_NODE(g_ckgen_gating_iram5),
    CLK_NODE(g_ckgen_gating_xtal_ap),
    CLK_NODE(g_ckgen_gating_pll_lvds1),
    CLK_NODE(g_ckgen_gating_pll_lvds2),
    CLK_NODE(g_ckgen_gating_pll_lvds3),
    CLK_NODE(g_ckgen_gating_pll_lvds4),
    CLK_NODE(g_ckgen_gating_disp),
    CLK_NODE(g_ckgen_gating_dc1),
    CLK_NODE(g_ckgen_gating_dc2),
    CLK_NODE(g_ckgen_gating_dc3),
    CLK_NODE(g_ckgen_gating_dc4),
    CLK_NODE(g_ckgen_gating_dc5),
    CLK_NODE(g_ckgen_gating_dp1),
    CLK_NODE(g_ckgen_gating_dp2),
    CLK_NODE(g_ckgen_gating_dp3),
    CLK_NODE(g_ckgen_gating_g2d1),
    CLK_NODE(g_ckgen_gating_g2d2),
    CLK_NODE(g_ckgen_gating_mipi_dsi1),
    CLK_NODE(g_ckgen_gating_mipi_dsi2),
    CLK_NODE(g_ckgen_gating_lvds_ss),
    CLK_NODE(g_ckgen_gating_csi1),
    CLK_NODE(g_ckgen_gating_csi2),
    CLK_NODE(g_ckgen_gating_csi3),
    CLK_NODE(g_ckgen_gating_disp_mux),
    CLK_NODE(g_ckgen_gating_mipi_csi1),
    CLK_NODE(g_ckgen_gating_mipi_csi2),
    CLK_NODE(g_ckgen_gating_tcu),
    CLK_NODE(g_ckgen_gating_tbu0),
    CLK_NODE(g_ckgen_gating_tbu1),
    CLK_NODE(g_ckgen_gating_tbu2),
    CLK_NODE(g_ckgen_gating_tbu3),
    CLK_NODE(g_ckgen_gating_tbu4),
    CLK_NODE(g_ckgen_gating_tbu5),
    CLK_NODE(g_ckgen_gating_tbu6),
    CLK_NODE(g_ckgen_gating_tbu7),
    CLK_NODE(g_ckgen_gating_tbu8),
    CLK_NODE(g_ckgen_gating_tbu9),
    CLK_NODE(g_ckgen_gating_tbu10),
    CLK_NODE(g_ckgen_gating_scr4k_sid),
    CLK_NODE(g_ckgen_gating_scr4k_ssid),
    CLK_NODE(g_ckgen_gating_bipc_ddr),
    CLK_NODE(g_ckgen_gating_eic_hpi),
    CLK_NODE(g_ckgen_gating_noc_main),
    CLK_NODE(g_ckgen_gating_gic4),
    CLK_NODE(g_ckgen_gating_gic5),
    CLK_NODE(g_ckgen_gating_pll_cpu1),
    CLK_NODE(g_ckgen_gating_gpu1),
    CLK_NODE(g_ckgen_gating_pll_gpu1),
    CLK_NODE(g_ckgen_gating_gpu2),
    CLK_NODE(g_ckgen_gating_pll_gpu2),
    CLK_NODE(g_ckgen_gating_vpu1),
    CLK_NODE(g_ckgen_gating_vpu2),
    CLK_NODE(g_ckgen_gating_mjpeg),
    CLK_NODE(g_ckgen_gating_pll_vpu),
    CLK_NODE(g_ckgen_gating_vdsp),
    CLK_NODE(g_ckgen_gating_noc_vsn),
    CLK_NODE(g_ckgen_gating_pll_vsn),
    CLK_NODE(g_ckgen_gating_bipc_vdsp),
    CLK_NODE(g_ckgen_gating_eic_vsn),
    CLK_NODE(g_ckgen_gating_noc_his),
    CLK_NODE(g_ckgen_gating_pll_his),
    CLK_NODE(g_ckgen_gating_pcie2_mstr),
    CLK_NODE(g_ckgen_gating_pcie1_mstr),
    CLK_NODE(g_ckgen_gating_pcie_phy),
    CLK_NODE(g_ckgen_gating_pcie_phy_ref),
    CLK_NODE(g_ckgen_gating_usb1),
    CLK_NODE(g_ckgen_gating_usb2),
    CLK_NODE(g_ckgen_gating_usb1_ref),
    CLK_NODE(g_ckgen_gating_usb2_ref),
    CLK_NODE(g_ckgen_gating_cpu1_clk0),
    CLK_NODE(g_ckgen_gating_cpu1_clk1),
    CLK_NODE(g_ckgen_gating_cpu1_clk2),
    CLK_NODE(g_ckgen_gating_cpu1_clk3),
    CLK_NODE(g_ckgen_gating_cpu1_clk4),
    CLK_NODE(g_ckgen_gating_cpu1_clk5),
};

const clk_node_t *board_clk_disable_array[] = {
    CLK_NODE(g_ckgen_gating_uart1),
    CLK_NODE(g_ckgen_gating_uart2),
    CLK_NODE(g_ckgen_gating_can5),
    CLK_NODE(g_ckgen_gating_can6),
    CLK_NODE(g_ckgen_gating_can7),
    CLK_NODE(g_ckgen_gating_can8),
    CLK_NODE(g_ckgen_gating_can9),
    CLK_NODE(g_ckgen_gating_can10),
    CLK_NODE(g_ckgen_gating_can11),
    CLK_NODE(g_ckgen_gating_can12),
    CLK_NODE(g_ckgen_gating_can13),
    CLK_NODE(g_ckgen_gating_can14),
    CLK_NODE(g_ckgen_gating_can15),
    CLK_NODE(g_ckgen_gating_can16),
    CLK_NODE(g_ckgen_gating_can17),
    CLK_NODE(g_ckgen_gating_can18),
    CLK_NODE(g_ckgen_gating_can19),
    CLK_NODE(g_ckgen_gating_can20),
    CLK_NODE(g_ckgen_gating_cpu2),
    CLK_NODE(g_ckgen_gating_pll_cpu2),
};

void board_clk_init(void)
{
    /* Disable unused clock. */
    for (uint32_t i = 0; i < ARRAY_SIZE(board_clk_disable_array); i++) {
        clk_disable((clk_node_t*)board_clk_disable_array[i]);
    }

    /* Enable used clock. */
    for (uint32_t i = 0; i < ARRAY_SIZE(board_clk_enable_array); i++) {
        clk_enable((clk_node_t*)board_clk_enable_array[i]);
    }
    /* Prepare clock tree. */
    for (uint32_t i = 0; i < ARRAY_SIZE(board_clk_prepare_array); i++) {
        clk_prepare((clk_node_t*)board_clk_prepare_array[i]);
    }
}

#endif