/*
 * Copyright (c) 2021-2029 Semidrive Incorporated.
 * All rights reserved.
 * Software License Agreement.
 */
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

static clk_core_t g_sec_core_core = {
    .base = APB_CKGEN_SEC_BASE,
    .ops = &ckgen_core_ops,
};

static clk_core_t g_sec_ip_core = {
    .base = APB_CKGEN_SEC_BASE,
    .ops = &ckgen_ip_ops,
};

static clk_core_t g_sec_gating_core = {
    .base = APB_CKGEN_SEC_BASE,
    .ops = &ckgen_lp_gating_ops,
};

static clk_core_t g_pll_disp1_core = {
    .base = APB_PLL_DISP1_BASE,
    .ops = &sc_pfpll_ops,
};

static clk_core_t g_pll_disp2_core = {
    .base = APB_PLL_DISP2_BASE,
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

static clk_core_t g_img_ip_core = {
    .base = APB_CKGEN_DISP_BASE,
    .ops = &ckgen_ip_ops,
};

static clk_core_t g_img_bus_core = {
    .base = APB_CKGEN_DISP_BASE,
    .ops = &ckgen_bus_ops,
};

static clk_core_t g_img_gating_core = {
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

static clk_core_t g_pll_cpu1c_core = {
    .base = APB_PLL_CPU1C_BASE,
    .ops = &sc_pfpll_ops,
};

static clk_core_t g_pll_cpu2a_core = {
    .base = APB_PLL_CPU2A_BASE,
    .ops = &sc_pfpll_ops,
};

static clk_core_t g_pll_cpu2b_core = {
    .base = APB_PLL_CPU2B_BASE,
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

static clk_core_t g_pll_vpu1_core = {
    .base = APB_PLL_VPU1_BASE,
    .ops = &sc_pfpll_ops,
};

static clk_core_t g_pll_vpu2_core = {
    .base = APB_PLL_VPU2_BASE,
    .ops = &sc_pfpll_ops,
};

static clk_core_t g_pll_vsn_core = {
    .base = PLL_VSN_BASE,
    .ops = &sc_pfpll_ops,
};

static clk_core_t g_pll_hpie_core = {
    .base = APB_PLL_HPIE_BASE,
    .ops = &sc_pfpll_ops,
};

static clk_core_t g_pll_hpiw_core = {
    .base = APB_PLL_HPIW_BASE,
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

static clk_core_t g_pll_vsn2_core = {
    .base = PLL_VSN2_BASE,
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
    .rate = 210000000,
    .mux = 0,
    .parent = NULL,
};
clk_node_t g_ext_aud4 = {
    .core = NULL,
    .id = 0x0,
    .rate = 210000000,
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
clk_node_t g_ckgen_ip_xspi1a = {
    .core = &g_saf_ip_core,
    .id = 0x0,
    .rate = 400000000,
    .mux = 4,
    .parent = CLK_NODE(g_pll2_div_a),
};
clk_node_t g_ckgen_ip_xspi1b = {
    .core = &g_saf_ip_core,
    .id = 0x1,
    .rate = 400000000,
    .mux = 4,
    .parent = CLK_NODE(g_pll2_div_a),
};
clk_node_t g_ckgen_ip_reserved = {
    .core = &g_saf_ip_core,
    .id = 0x2,
    .rate = 500000000,
    .mux = 5,
    .parent = CLK_NODE(g_pll1_root),
};
clk_node_t g_ckgen_ip_i2c_saf = {
    .core = &g_saf_ip_core,
    .id = 0x3,
    .rate = 133330000,
    .mux = 5,
    .parent = CLK_NODE(g_pll2_div_b),
};
clk_node_t g_ckgen_ip_spi_saf = {
    .core = &g_saf_ip_core,
    .id = 0x4,
    .rate = 133330000,
    .mux = 5,
    .parent = CLK_NODE(g_pll2_div_b),
};
clk_node_t g_ckgen_ip_uart_saf = {
    .core = &g_saf_ip_core,
    .id = 0x5,
    .rate = 160000000,
    .mux = 4,
    .parent = CLK_NODE(g_pll2_div_d),
};
clk_node_t g_ckgen_ip_i2s_mclk1 = {
    .core = &g_saf_ip_core,
    .id = 0x6,
    .rate = 66670000,
    .mux = 7,
    .parent = CLK_NODE(g_pll2_div_b),
};
clk_node_t g_ckgen_ip_i2s_sc1 = {
    .core = &g_saf_ip_core,
    .id = 0x7,
    .rate = 400000000,
    .mux = 5,
    .parent = CLK_NODE(g_pll2_div_a),
};
clk_node_t g_ckgen_ip_i2s_sc2 = {
    .core = &g_saf_ip_core,
    .id = 0x8,
    .rate = 400000000,
    .mux = 5,
    .parent = CLK_NODE(g_pll2_div_a),
};
clk_node_t g_ckgen_ip_enet1_tx = {
    .core = &g_saf_ip_core,
    .id = 0x9,
    .rate = 250000000,
    .mux = 4,
    .parent = CLK_NODE(g_pll1_div_c),
};
clk_node_t g_ckgen_ip_enet1_rmii = {
    .core = &g_saf_ip_core,
    .id = 0xa,
    .rate = 50000000,
    .mux = 4,
    .parent = CLK_NODE(g_pll1_div_c),
};
clk_node_t g_ckgen_ip_enet1_phy_ref = {
    .core = &g_saf_ip_core,
    .id = 0xb,
    .rate = 125000000,
    .mux = 4,
    .parent = CLK_NODE(g_pll1_div_c),
};
clk_node_t g_ckgen_ip_enet1_timer_sec = {
    .core = &g_saf_ip_core,
    .id = 0xc,
    .rate = 500000000,
    .mux = 4,
    .parent = CLK_NODE(g_pll1_div_a),
};
clk_node_t g_ckgen_ip_ospi1 = {
    .core = &g_saf_ip_core,
    .id = 0xd,
    .rate = 400000000,
    .mux = 7,
    .parent = CLK_NODE(g_pll2_div_a),
};
clk_node_t g_ckgen_ip_timer1 = {
    .core = &g_saf_ip_core,
    .id = 0xe,
    .rate = 400000000,
    .mux = 4,
    .parent = CLK_NODE(g_pll2_div_a),
};
clk_node_t g_ckgen_ip_timer2 = {
    .core = &g_saf_ip_core,
    .id = 0xf,
    .rate = 400000000,
    .mux = 4,
    .parent = CLK_NODE(g_pll2_div_a),
};
clk_node_t g_ckgen_ip_pwm1 = {
    .core = &g_saf_ip_core,
    .id = 0x10,
    .rate = 400000000,
    .mux = 4,
    .parent = CLK_NODE(g_pll2_div_a),
};
clk_node_t g_ckgen_ip_pwm2 = {
    .core = &g_saf_ip_core,
    .id = 0x11,
    .rate = 400000000,
    .mux = 4,
    .parent = CLK_NODE(g_pll2_div_a),
};
clk_node_t g_ckgen_ip_can_1_to_4 = {
    .core = &g_saf_ip_core,
    .id = 0x12,
    .rate = 80000000,
    .mux = 4,
    .parent = CLK_NODE(g_pll2_div_d),
};
clk_node_t g_ckgen_ip_lin = {
    .core = &g_saf_ip_core,
    .id = 0x13,
    .rate = 80000000,
    .mux = 4,
    .parent = CLK_NODE(g_pll2_div_d),
};
clk_node_t g_ckgen_ip_adc2 = {
    .core = &g_saf_ip_core,
    .id = 0x14,
    .rate = 400000000,
    .mux = 5,
    .parent = CLK_NODE(g_pll2_div_a),
};
clk_node_t g_ckgen_ip_adc3 = {
    .core = &g_saf_ip_core,
    .id = 0x15,
    .rate = 400000000,
    .mux = 5,
    .parent = CLK_NODE(g_pll2_div_a),
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
    .rate = 541900800,
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
    .rate = 210000000,
    .mux = 0,
    .parent = NULL,
};
clk_node_t g_ext_aud2 = {
    .core = NULL,
    .id = 0x0,
    .rate = 210000000,
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
clk_node_t g_ckgen_core_mp_plat = {
    .core = &g_sec_core_core,
    .id = 0x0,
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
clk_node_t g_ckgen_ip_adc1 = {
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
    .rate = 120000000,
    .mux = 4,
    .parent = CLK_NODE(g_pll3_div_c),
};
clk_node_t g_ckgen_ip_i2c_sec1 = {
    .core = &g_sec_ip_core,
    .id = 0x4,
    .rate = 120000000,
    .mux = 4,
    .parent = CLK_NODE(g_pll3_div_c),
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
    .rate = 159200000,
    .mux = 6,
    .parent = CLK_NODE(g_pll5_div_d),
};
clk_node_t g_ckgen_ip_uart_sec1 = {
    .core = &g_sec_ip_core,
    .id = 0x8,
    .rate = 159200000,
    .mux = 6,
    .parent = CLK_NODE(g_pll5_div_d),
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
clk_node_t g_ckgen_ip_enet2_tx = {
    .core = &g_sec_ip_core,
    .id = 0xd,
    .rate = 250000000,
    .mux = 4,
    .parent = CLK_NODE(g_pll4_div_c),
};
clk_node_t g_ckgen_ip_enet2_rmii = {
    .core = &g_sec_ip_core,
    .id = 0xe,
    .rate = 50000000,
    .mux = 4,
    .parent = CLK_NODE(g_pll4_div_c),
};
clk_node_t g_ckgen_ip_enet2_phy_ref = {
    .core = &g_sec_ip_core,
    .id = 0xf,
    .rate = 125000000,
    .mux = 4,
    .parent = CLK_NODE(g_pll4_div_c),
};
clk_node_t g_ckgen_ip_enet2_timer_sec = {
    .core = &g_sec_ip_core,
    .id = 0x10,
    .rate = 500000000,
    .mux = 4,
    .parent = CLK_NODE(g_pll4_div_a),
};
clk_node_t g_ckgen_ip_spdif1 = {
    .core = &g_sec_ip_core,
    .id = 0x11,
    .rate = 66330000,
    .mux = 6,
    .parent = CLK_NODE(g_pll5_div_b),
};
clk_node_t g_ckgen_ip_spdif2 = {
    .core = &g_sec_ip_core,
    .id = 0x12,
    .rate = 66330000,
    .mux = 6,
    .parent = CLK_NODE(g_pll5_div_b),
};
clk_node_t g_ckgen_ip_spdif3 = {
    .core = &g_sec_ip_core,
    .id = 0x13,
    .rate = 66330000,
    .mux = 6,
    .parent = CLK_NODE(g_pll5_div_b),
};
clk_node_t g_ckgen_ip_spdif4 = {
    .core = &g_sec_ip_core,
    .id = 0x14,
    .rate = 66330000,
    .mux = 6,
    .parent = CLK_NODE(g_pll5_div_b),
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
    .rate = 12290000,
    .mux = 4,
    .parent = CLK_NODE(g_pll6_root),
};
clk_node_t g_ckgen_ip_i2s_mclk3 = {
    .core = &g_sec_ip_core,
    .id = 0x23,
    .rate = 12290000,
    .mux = 4,
    .parent = CLK_NODE(g_pll6_root),
};
clk_node_t g_ckgen_ip_i2s_mc1 = {
    .core = &g_sec_ip_core,
    .id = 0x24,
    .rate = 294910000,
    .mux = 4,
    .parent = CLK_NODE(g_pll6_root),
};
clk_node_t g_ckgen_ip_i2s_mc2 = {
    .core = &g_sec_ip_core,
    .id = 0x25,
    .rate = 294910000,
    .mux = 4,
    .parent = CLK_NODE(g_pll6_root),
};
clk_node_t g_ckgen_ip_i2s_sc3 = {
    .core = &g_sec_ip_core,
    .id = 0x26,
    .rate = 294910000,
    .mux = 4,
    .parent = CLK_NODE(g_pll6_root),
};
clk_node_t g_ckgen_ip_i2s_sc4 = {
    .core = &g_sec_ip_core,
    .id = 0x27,
    .rate = 294910000,
    .mux = 4,
    .parent = CLK_NODE(g_pll6_root),
};
clk_node_t g_ckgen_ip_i2s_sc5 = {
    .core = &g_sec_ip_core,
    .id = 0x28,
    .rate = 294910000,
    .mux = 4,
    .parent = CLK_NODE(g_pll6_root),
};
clk_node_t g_ckgen_ip_i2s_sc6 = {
    .core = &g_sec_ip_core,
    .id = 0x29,
    .rate = 294910000,
    .mux = 4,
    .parent = CLK_NODE(g_pll6_root),
};
clk_node_t g_ckgen_ip_i2s_sc7 = {
    .core = &g_sec_ip_core,
    .id = 0x2a,
    .rate = 294910000,
    .mux = 4,
    .parent = CLK_NODE(g_pll6_root),
};
clk_node_t g_ckgen_ip_i2s_sc8 = {
    .core = &g_sec_ip_core,
    .id = 0x2b,
    .rate = 294910000,
    .mux = 4,
    .parent = CLK_NODE(g_pll6_root),
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
clk_node_t g_ckgen_ip_can_5_to_20 = {
    .core = &g_sec_ip_core,
    .id = 0x2f,
    .rate = 79600000,
    .mux = 7,
    .parent = CLK_NODE(g_pll5_div_d),
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
    .mux = 2,
    .parent = CLK_NODE(g_xtal_24m),
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
clk_node_t g_pll_disp1_vco = {
    .core = &g_pll_disp1_core,
    .id = 0x0,
    .rate = 2079000000,
    .mux = 0,
    .parent = CLK_NODE(g_rc_24m),
};
clk_node_t g_pll_disp1_root = {
    .core = &g_pll_disp1_core,
    .id = 0x1,
    .rate = 2079000000,
    .mux = 0,
    .parent = CLK_NODE(g_pll_disp1_vco),
};
clk_node_t g_pll_disp1_div_a = {
    .core = &g_pll_disp1_core,
    .id = 0x2,
    .rate = 1039500000,
    .mux = 0,
    .parent = CLK_NODE(g_pll_disp1_vco),
};
clk_node_t g_pll_disp1_div_b = {
    .core = &g_pll_disp1_core,
    .id = 0x3,
    .rate = 519750000,
    .mux = 0,
    .parent = CLK_NODE(g_pll_disp1_vco),
};
clk_node_t g_pll_disp2_vco = {
    .core = &g_pll_disp2_core,
    .id = 0x0,
    .rate = 2000000000,
    .mux = 0,
    .parent = CLK_NODE(g_rc_24m),
};
clk_node_t g_pll_disp2_root = {
    .core = &g_pll_disp2_core,
    .id = 0x1,
    .rate = 2000000000,
    .mux = 0,
    .parent = CLK_NODE(g_pll_disp2_vco),
};
clk_node_t g_pll_disp2_div_a = {
    .core = &g_pll_disp2_core,
    .id = 0x2,
    .rate = 666666700,
    .mux = 0,
    .parent = CLK_NODE(g_pll_disp2_vco),
};
clk_node_t g_pll_disp2_div_b = {
    .core = &g_pll_disp2_core,
    .id = 0x3,
    .rate = 500000000,
    .mux = 0,
    .parent = CLK_NODE(g_pll_disp2_vco),
};
clk_node_t g_pll_lvds1_vco = {
    .core = &g_pll_lvds1_core,
    .id = 0x0,
    .rate = 2100000000,
    .mux = 0,
    .parent = CLK_NODE(g_rc_24m),
};
clk_node_t g_pll_lvds1_root = {
    .core = &g_pll_lvds1_core,
    .id = 0x1,
    .rate = 2100000000,
    .mux = 0,
    .parent = CLK_NODE(g_pll_lvds1_vco),
};
clk_node_t g_pll_lvds1_div_a = {
    .core = &g_pll_lvds1_core,
    .id = 0x2,
    .rate = 700000000,
    .mux = 0,
    .parent = CLK_NODE(g_pll_lvds1_vco),
};
clk_node_t g_pll_lvds1_div_b = {
    .core = &g_pll_lvds1_core,
    .id = 0x3,
    .rate = 210000000,
    .mux = 0,
    .parent = CLK_NODE(g_pll_lvds1_vco),
};
clk_node_t g_pll_lvds2_vco = {
    .core = &g_pll_lvds2_core,
    .id = 0x0,
    .rate = 2100000000,
    .mux = 0,
    .parent = CLK_NODE(g_rc_24m),
};
clk_node_t g_pll_lvds2_root = {
    .core = &g_pll_lvds2_core,
    .id = 0x1,
    .rate = 2100000000,
    .mux = 0,
    .parent = CLK_NODE(g_pll_lvds2_vco),
};
clk_node_t g_pll_lvds2_div_a = {
    .core = &g_pll_lvds2_core,
    .id = 0x2,
    .rate = 700000000,
    .mux = 0,
    .parent = CLK_NODE(g_pll_lvds2_vco),
};
clk_node_t g_pll_lvds2_div_b = {
    .core = &g_pll_lvds2_core,
    .id = 0x3,
    .rate = 210000000,
    .mux = 0,
    .parent = CLK_NODE(g_pll_lvds2_vco),
};
clk_node_t g_pll_lvds3_vco = {
    .core = &g_pll_lvds3_core,
    .id = 0x0,
    .rate = 2139000000,
    .mux = 0,
    .parent = CLK_NODE(g_rc_24m),
};
clk_node_t g_pll_lvds3_root = {
    .core = &g_pll_lvds3_core,
    .id = 0x1,
    .rate = 2139000000,
    .mux = 0,
    .parent = CLK_NODE(g_pll_lvds3_vco),
};
clk_node_t g_pll_lvds3_div_a = {
    .core = &g_pll_lvds3_core,
    .id = 0x2,
    .rate = 356500000,
    .mux = 0,
    .parent = CLK_NODE(g_pll_lvds3_vco),
};
clk_node_t g_pll_lvds3_div_b = {
    .core = &g_pll_lvds3_core,
    .id = 0x3,
    .rate = 213900000,
    .mux = 0,
    .parent = CLK_NODE(g_pll_lvds3_vco),
};
clk_node_t g_pll_lvds4_vco = {
    .core = &g_pll_lvds4_core,
    .id = 0x0,
    .rate = 2139000000,
    .mux = 0,
    .parent = CLK_NODE(g_rc_24m),
};
clk_node_t g_pll_lvds4_root = {
    .core = &g_pll_lvds4_core,
    .id = 0x1,
    .rate = 2139000000,
    .mux = 0,
    .parent = CLK_NODE(g_pll_lvds4_vco),
};
clk_node_t g_pll_lvds4_div_a = {
    .core = &g_pll_lvds4_core,
    .id = 0x2,
    .rate = 356500000,
    .mux = 0,
    .parent = CLK_NODE(g_pll_lvds4_vco),
};
clk_node_t g_pll_lvds4_div_b = {
    .core = &g_pll_lvds4_core,
    .id = 0x3,
    .rate = 213900000,
    .mux = 0,
    .parent = CLK_NODE(g_pll_lvds4_vco),
};
clk_node_t g_ckgen_ip_mipi_csi1_pix = {
    .core = &g_img_ip_core,
    .id = 0x0,
    .rate = 693000000,
    .mux = 5,
    .parent = CLK_NODE(g_pll_disp1_root),
};
clk_node_t g_ckgen_ip_mipi_csi2_pix = {
    .core = &g_img_ip_core,
    .id = 0x1,
    .rate = 693000000,
    .mux = 5,
    .parent = CLK_NODE(g_pll_disp1_root),
};
clk_node_t g_ckgen_ip_mipi_csi3_pix = {
    .core = &g_img_ip_core,
    .id = 0x2,
    .rate = 693000000,
    .mux = 5,
    .parent = CLK_NODE(g_pll_disp1_root),
};
clk_node_t g_ckgen_ip_csi4 = {
    .core = &g_img_ip_core,
    .id = 0x3,
    .rate = 693000000,
    .mux = 5,
    .parent = CLK_NODE(g_pll_disp1_root),
};
clk_node_t g_ckgen_ip_dp1 = {
    .core = &g_img_ip_core,
    .id = 0x4,
    .rate = 415800000,
    .mux = 4,
    .parent = CLK_NODE(g_pll_disp1_root),
};
clk_node_t g_ckgen_ip_dp2 = {
    .core = &g_img_ip_core,
    .id = 0x5,
    .rate = 415800000,
    .mux = 4,
    .parent = CLK_NODE(g_pll_disp1_root),
};
clk_node_t g_ckgen_ip_dp3 = {
    .core = &g_img_ip_core,
    .id = 0x6,
    .rate = 415800000,
    .mux = 4,
    .parent = CLK_NODE(g_pll_disp1_root),
};
clk_node_t g_ckgen_ip_dc1 = {
    .core = &g_img_ip_core,
    .id = 0x7,
    .rate = 415800000,
    .mux = 4,
    .parent = CLK_NODE(g_pll_disp1_root),
};
clk_node_t g_ckgen_ip_dc2 = {
    .core = &g_img_ip_core,
    .id = 0x8,
    .rate = 415800000,
    .mux = 4,
    .parent = CLK_NODE(g_pll_disp1_root),
};
clk_node_t g_ckgen_ip_dc3 = {
    .core = &g_img_ip_core,
    .id = 0x9,
    .rate = 415800000,
    .mux = 4,
    .parent = CLK_NODE(g_pll_disp1_root),
};
clk_node_t g_ckgen_ip_dc4 = {
    .core = &g_img_ip_core,
    .id = 0xa,
    .rate = 415800000,
    .mux = 4,
    .parent = CLK_NODE(g_pll_disp1_root),
};
clk_node_t g_ckgen_ip_dc5 = {
    .core = &g_img_ip_core,
    .id = 0xb,
    .rate = 415800000,
    .mux = 4,
    .parent = CLK_NODE(g_pll_disp1_root),
};
clk_node_t g_ckgen_ip_isp = {
    .core = &g_img_ip_core,
    .id = 0xc,
    .rate = 1000000000,
    .mux = 4,
    .parent = CLK_NODE(g_pll_disp2_root),
};
clk_node_t g_ckgen_ip_spare1 = {
    .core = &g_img_ip_core,
    .id = 0xd,
    .rate = 103950000,
    .mux = 4,
    .parent = CLK_NODE(g_pll_disp1_div_b),
};
clk_node_t g_ckgen_ip_dc_alt_dsp_clk0 = {
    .core = &g_img_ip_core,
    .id = 0xe,
    .rate = 346500000,
    .mux = 5,
    .parent = CLK_NODE(g_pll_disp1_root),
};
clk_node_t g_ckgen_ip_dc_alt_dsp_clk1 = {
    .core = &g_img_ip_core,
    .id = 0xf,
    .rate = 346500000,
    .mux = 5,
    .parent = CLK_NODE(g_pll_disp1_root),
};
clk_node_t g_ckgen_bus_disp_bus_root = {
    .core = &g_img_bus_core,
    .id = 0x0,
    .rate = 666670000,
    .mux = 4,
    .parent = CLK_NODE(g_pll_disp2_root),
};
clk_node_t g_ckgen_bus_disp_bus_div_m = {
    .core = &g_img_bus_core,
    .id = 0x4000000,
    .rate = 666670000,
    .mux = 4,
    .parent = CLK_NODE(g_pll_disp2_root),
};
clk_node_t g_ckgen_bus_disp_bus_div_n = {
    .core = &g_img_bus_core,
    .id = 0x3000000,
    .rate = 333335000,
    .mux = 4,
    .parent = CLK_NODE(g_pll_disp2_root),
};
clk_node_t g_ckgen_bus_disp_bus_div_p = {
    .core = &g_img_bus_core,
    .id = 0x2000000,
    .rate = 666670000,
    .mux = 4,
    .parent = CLK_NODE(g_pll_disp2_root),
};
clk_node_t g_ckgen_bus_disp_bus_div_q = {
    .core = &g_img_bus_core,
    .id = 0x1000000,
    .rate = 666670000,
    .mux = 4,
    .parent = CLK_NODE(g_pll_disp2_root),
};
clk_node_t g_ckgen_ip_ext_aud1 = {
    .core = &g_img_ip_core,
    .id = 0x10,
    .rate = 210000000,
    .mux = 4,
    .parent = CLK_NODE(g_pll_lvds1_div_b),
};
clk_node_t g_ckgen_ip_ext_aud2 = {
    .core = &g_img_ip_core,
    .id = 0x11,
    .rate = 210000000,
    .mux = 4,
    .parent = CLK_NODE(g_pll_lvds1_div_b),
};
clk_node_t g_ckgen_ip_ext_aud3 = {
    .core = &g_img_ip_core,
    .id = 0x12,
    .rate = 210000000,
    .mux = 4,
    .parent = CLK_NODE(g_pll_lvds1_div_b),
};
clk_node_t g_ckgen_ip_ext_aud4 = {
    .core = &g_img_ip_core,
    .id = 0x13,
    .rate = 210000000,
    .mux = 4,
    .parent = CLK_NODE(g_pll_lvds1_div_b),
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
clk_node_t g_pll_cpu1a_vco = {
    .core = &g_pll_cpu1a_core,
    .id = 0x0,
    .rate = 1800000000,
    .mux = 0,
    .parent = CLK_NODE(g_rc_24m),
};
clk_node_t g_pll_cpu1a_root = {
    .core = &g_pll_cpu1a_core,
    .id = 0x1,
    .rate = 1800000000,
    .mux = 0,
    .parent = CLK_NODE(g_pll_cpu1a_vco),
};
clk_node_t g_pll_cpu1a_div_a = {
    .core = &g_pll_cpu1a_core,
    .id = 0x2,
    .rate = 900000000,
    .mux = 0,
    .parent = CLK_NODE(g_pll_cpu1a_vco),
};
clk_node_t g_pll_cpu1a_div_b = {
    .core = &g_pll_cpu1a_core,
    .id = 0x3,
    .rate = 450000000,
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
clk_node_t g_pll_cpu1b_vco = {
    .core = &g_pll_cpu1b_core,
    .id = 0x0,
    .rate = 1800000000,
    .mux = 0,
    .parent = CLK_NODE(g_rc_24m),
};
clk_node_t g_pll_cpu1b_root = {
    .core = &g_pll_cpu1b_core,
    .id = 0x1,
    .rate = 1800000000,
    .mux = 0,
    .parent = CLK_NODE(g_pll_cpu1b_vco),
};
clk_node_t g_pll_cpu1b_div_a = {
    .core = &g_pll_cpu1b_core,
    .id = 0x2,
    .rate = 900000000,
    .mux = 0,
    .parent = CLK_NODE(g_pll_cpu1b_vco),
};
clk_node_t g_pll_cpu1b_div_b = {
    .core = &g_pll_cpu1b_core,
    .id = 0x3,
    .rate = 450000000,
    .mux = 0,
    .parent = CLK_NODE(g_pll_cpu1b_vco),
};
clk_node_t g_pll_cpu1c_vco = {
    .core = &g_pll_cpu1c_core,
    .id = 0x0,
    .rate = 1168000000,
    .mux = 0,
    .parent = CLK_NODE(g_rc_24m),
};
clk_node_t g_pll_cpu1c_root = {
    .core = &g_pll_cpu1c_core,
    .id = 0x1,
    .rate = 1168000000,
    .mux = 0,
    .parent = CLK_NODE(g_pll_cpu1c_vco),
};
clk_node_t g_pll_cpu1c_div_a = {
    .core = &g_pll_cpu1c_core,
    .id = 0x2,
    .rate = 584000000,
    .mux = 0,
    .parent = CLK_NODE(g_pll_cpu1c_vco),
};
clk_node_t g_pll_cpu1c_div_b = {
    .core = &g_pll_cpu1c_core,
    .id = 0x3,
    .rate = 292000000,
    .mux = 0,
    .parent = CLK_NODE(g_pll_cpu1c_vco),
};
clk_node_t g_pll_cpu2a_vco = {
    .core = &g_pll_cpu2a_core,
    .id = 0x0,
    .rate = 1800000000,
    .mux = 0,
    .parent = CLK_NODE(g_rc_24m),
};
clk_node_t g_pll_cpu2a_root = {
    .core = &g_pll_cpu2a_core,
    .id = 0x1,
    .rate = 900000000,
    .mux = 0,
    .parent = CLK_NODE(g_pll_cpu2a_vco),
};
clk_node_t g_pll_cpu2a_div_a = {
    .core = &g_pll_cpu2a_core,
    .id = 0x2,
    .rate = 900000000,
    .mux = 0,
    .parent = CLK_NODE(g_pll_cpu2a_vco),
};
clk_node_t g_pll_cpu2a_div_b = {
    .core = &g_pll_cpu2a_core,
    .id = 0x3,
    .rate = 450000000,
    .mux = 0,
    .parent = CLK_NODE(g_pll_cpu2a_vco),
};
clk_node_t g_pll_cpu2b_vco = {
    .core = &g_pll_cpu2b_core,
    .id = 0x0,
    .rate = 1400000000,
    .mux = 0,
    .parent = CLK_NODE(g_rc_24m),
};
clk_node_t g_pll_cpu2b_root = {
    .core = &g_pll_cpu2b_core,
    .id = 0x1,
    .rate = 700000000,
    .mux = 0,
    .parent = CLK_NODE(g_pll_cpu2b_vco),
};
clk_node_t g_pll_cpu2b_div_a = {
    .core = &g_pll_cpu2b_core,
    .id = 0x2,
    .rate = 700000000,
    .mux = 0,
    .parent = CLK_NODE(g_pll_cpu2b_vco),
};
clk_node_t g_pll_cpu2b_div_b = {
    .core = &g_pll_cpu2b_core,
    .id = 0x3,
    .rate = 350000000,
    .mux = 0,
    .parent = CLK_NODE(g_pll_cpu2b_vco),
};
clk_node_t g_ckgen_core_gpu1_pre = {
    .core = &g_soc_core_core,
    .id = 0x5,
    .rate = 24000000,
    .mux = 0,
    .parent = CLK_NODE(g_rc_24m),
};
clk_node_t g_ckgen_uuu_gpu1_div_m_pre = {
    .core = &g_soc_uuu_core,
    .id = 0x3000005,
    .rate = 24000000,
    .mux = 0,
    .parent = CLK_NODE(g_ckgen_core_gpu1_pre),
};
clk_node_t g_ckgen_uuu_gpu1_div_n_pre = {
    .core = &g_soc_uuu_core,
    .id = 0x2000005,
    .rate = 24000000,
    .mux = 0,
    .parent = CLK_NODE(g_ckgen_core_gpu1_pre),
};
clk_node_t g_ckgen_uuu_gpu1_div_p_pre = {
    .core = &g_soc_uuu_core,
    .id = 0x1000005,
    .rate = 24000000,
    .mux = 0,
    .parent = CLK_NODE(g_ckgen_core_gpu1_pre),
};
clk_node_t g_pll_gpu1_vco = {
    .core = &g_pll_gpu1_core,
    .id = 0x0,
    .rate = 1512000000,
    .mux = 0,
    .parent = CLK_NODE(g_rc_24m),
};
clk_node_t g_pll_gpu1_root = {
    .core = &g_pll_gpu1_core,
    .id = 0x1,
    .rate = 756000000,
    .mux = 0,
    .parent = CLK_NODE(g_pll_gpu1_vco),
};
clk_node_t g_pll_gpu1_div_a = {
    .core = &g_pll_gpu1_core,
    .id = 0x2,
    .rate = 378000000,
    .mux = 0,
    .parent = CLK_NODE(g_pll_gpu1_vco),
};
clk_node_t g_pll_gpu1_div_b = {
    .core = &g_pll_gpu1_core,
    .id = 0x3,
    .rate = 252000000,
    .mux = 0,
    .parent = CLK_NODE(g_pll_gpu1_vco),
};
clk_node_t g_ckgen_core_gpu2_pre = {
    .core = &g_soc_core_core,
    .id = 0x6,
    .rate = 24000000,
    .mux = 0,
    .parent = CLK_NODE(g_rc_24m),
};
clk_node_t g_ckgen_uuu_gpu2_div_m_pre = {
    .core = &g_soc_uuu_core,
    .id = 0x3000006,
    .rate = 24000000,
    .mux = 0,
    .parent = CLK_NODE(g_ckgen_core_gpu2_pre),
};
clk_node_t g_ckgen_uuu_gpu2_div_n_pre = {
    .core = &g_soc_uuu_core,
    .id = 0x2000006,
    .rate = 24000000,
    .mux = 0,
    .parent = CLK_NODE(g_ckgen_core_gpu2_pre),
};
clk_node_t g_ckgen_uuu_gpu2_div_p_pre = {
    .core = &g_soc_uuu_core,
    .id = 0x1000006,
    .rate = 24000000,
    .mux = 0,
    .parent = CLK_NODE(g_ckgen_core_gpu2_pre),
};
clk_node_t g_pll_gpu2_vco = {
    .core = &g_pll_gpu2_core,
    .id = 0x0,
    .rate = 1512000000,
    .mux = 0,
    .parent = CLK_NODE(g_rc_24m),
};
clk_node_t g_pll_gpu2_root = {
    .core = &g_pll_gpu2_core,
    .id = 0x1,
    .rate = 756000000,
    .mux = 0,
    .parent = CLK_NODE(g_pll_gpu2_vco),
};
clk_node_t g_pll_gpu2_div_a = {
    .core = &g_pll_gpu2_core,
    .id = 0x2,
    .rate = 378000000,
    .mux = 0,
    .parent = CLK_NODE(g_pll_gpu2_vco),
};
clk_node_t g_pll_gpu2_div_b = {
    .core = &g_pll_gpu2_core,
    .id = 0x3,
    .rate = 252000000,
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
clk_node_t g_ckgen_uuu_vpu1_div_m_pre = {
    .core = &g_soc_uuu_core,
    .id = 0x3000007,
    .rate = 24000000,
    .mux = 0,
    .parent = CLK_NODE(g_ckgen_ip_vpu1_pre),
};
clk_node_t g_pll_vpu1_vco = {
    .core = &g_pll_vpu1_core,
    .id = 0x0,
    .rate = 3000000000,
    .mux = 0,
    .parent = CLK_NODE(g_rc_24m),
};
clk_node_t g_pll_vpu1_root = {
    .core = &g_pll_vpu1_core,
    .id = 0x1,
    .rate = 3000000000,
    .mux = 0,
    .parent = CLK_NODE(g_pll_vpu1_vco),
};
clk_node_t g_pll_vpu1_div_a = {
    .core = &g_pll_vpu1_core,
    .id = 0x2,
    .rate = 750000000,
    .mux = 0,
    .parent = CLK_NODE(g_pll_vpu1_vco),
};
clk_node_t g_pll_vpu1_div_b = {
    .core = &g_pll_vpu1_core,
    .id = 0x3,
    .rate = 600000000,
    .mux = 0,
    .parent = CLK_NODE(g_pll_vpu1_vco),
};
clk_node_t g_pll_vpu2_vco = {
    .core = &g_pll_vpu2_core,
    .id = 0x0,
    .rate = 1440000000,
    .mux = 0,
    .parent = CLK_NODE(g_rc_24m),
};
clk_node_t g_pll_vpu2_root = {
    .core = &g_pll_vpu2_core,
    .id = 0x1,
    .rate = 720000000,
    .mux = 0,
    .parent = CLK_NODE(g_pll_vpu2_vco),
};
clk_node_t g_pll_vpu2_div_a = {
    .core = &g_pll_vpu2_core,
    .id = 0x2,
    .rate = 720000000,
    .mux = 0,
    .parent = CLK_NODE(g_pll_vpu2_vco),
};
clk_node_t g_pll_vpu2_div_b = {
    .core = &g_pll_vpu2_core,
    .id = 0x3,
    .rate = 480000000,
    .mux = 0,
    .parent = CLK_NODE(g_pll_vpu2_vco),
};
clk_node_t g_pll_vsn_vco = {
    .core = &g_pll_vsn_core,
    .id = 0x0,
    .rate = 1552000000,
    .mux = 0,
    .parent = CLK_NODE(g_rc_24m),
};
clk_node_t g_pll_vsn_root = {
    .core = &g_pll_vsn_core,
    .id = 0x1,
    .rate = 776000000,
    .mux = 0,
    .parent = CLK_NODE(g_pll_vsn_vco),
};
clk_node_t g_pll_vsn_div_a = {
    .core = &g_pll_vsn_core,
    .id = 0x2,
    .rate = 388000000,
    .mux = 0,
    .parent = CLK_NODE(g_pll_vsn_vco),
};
clk_node_t g_pll_vsn_div_b = {
    .core = &g_pll_vsn_core,
    .id = 0x3,
    .rate = 258666700,
    .mux = 0,
    .parent = CLK_NODE(g_pll_vsn_vco),
};
clk_node_t g_pll_hpie_vco = {
    .core = &g_pll_hpie_core,
    .id = 0x0,
    .rate = 3192000000,
    .mux = 0,
    .parent = CLK_NODE(g_rc_24m),
};
clk_node_t g_pll_hpie_root = {
    .core = &g_pll_hpie_core,
    .id = 0x1,
    .rate = 1064000000,
    .mux = 0,
    .parent = CLK_NODE(g_pll_hpie_vco),
};
clk_node_t g_pll_hpie_div_a = {
    .core = &g_pll_hpie_core,
    .id = 0x2,
    .rate = 798000000,
    .mux = 0,
    .parent = CLK_NODE(g_pll_hpie_vco),
};
clk_node_t g_pll_hpie_div_b = {
    .core = &g_pll_hpie_core,
    .id = 0x3,
    .rate = 266000000,
    .mux = 0,
    .parent = CLK_NODE(g_pll_hpie_vco),
};
clk_node_t g_pll_hpiw_vco = {
    .core = &g_pll_hpiw_core,
    .id = 0x0,
    .rate = 1600000000,
    .mux = 0,
    .parent = CLK_NODE(g_rc_24m),
};
clk_node_t g_pll_hpiw_root = {
    .core = &g_pll_hpiw_core,
    .id = 0x1,
    .rate = 800000000,
    .mux = 0,
    .parent = CLK_NODE(g_pll_hpiw_vco),
};
clk_node_t g_pll_hpiw_div_a = {
    .core = &g_pll_hpiw_core,
    .id = 0x2,
    .rate = 533333300,
    .mux = 0,
    .parent = CLK_NODE(g_pll_hpiw_vco),
};
clk_node_t g_pll_hpiw_div_b = {
    .core = &g_pll_hpiw_core,
    .id = 0x3,
    .rate = 266666700,
    .mux = 0,
    .parent = CLK_NODE(g_pll_hpiw_vco),
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
    .rate = 2000000000,
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
clk_node_t g_pll_vsn2_vco = {
    .core = &g_pll_vsn2_core,
    .id = 0x0,
    .rate = 1552000000,
    .mux = 0,
    .parent = CLK_NODE(g_rc_24m),
};
clk_node_t g_pll_vsn2_root = {
    .core = &g_pll_vsn2_core,
    .id = 0x1,
    .rate = 776000000,
    .mux = 0,
    .parent = CLK_NODE(g_pll_vsn2_vco),
};
clk_node_t g_pll_vsn2_div_a = {
    .core = &g_pll_vsn2_core,
    .id = 0x2,
    .rate = 388000000,
    .mux = 0,
    .parent = CLK_NODE(g_pll_vsn2_vco),
};
clk_node_t g_pll_vsn2_div_b = {
    .core = &g_pll_vsn2_core,
    .id = 0x3,
    .rate = 258666700,
    .mux = 0,
    .parent = CLK_NODE(g_pll_vsn2_vco),
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
clk_node_t g_ckgen_core_cpu1c = {
    .core = &g_soc_core_core,
    .id = 0x2,
    .rate = 400000000,
    .mux = 3,
    .parent = CLK_NODE(g_pll2_div_a),
};
clk_node_t g_ckgen_core_cpu2a = {
    .core = &g_soc_core_core,
    .id = 0x3,
    .rate = 400000000,
    .mux = 3,
    .parent = CLK_NODE(g_pll2_div_a),
};
clk_node_t g_ckgen_core_cpu2b = {
    .core = &g_soc_core_core,
    .id = 0x4,
    .rate = 400000000,
    .mux = 3,
    .parent = CLK_NODE(g_pll2_div_a),
};
clk_node_t g_ckgen_core_gpu1 = {
    .core = &g_soc_core_core,
    .id = 0x5,
    .rate = 400000000,
    .mux = 3,
    .parent = CLK_NODE(g_pll2_div_a),
};
clk_node_t g_ckgen_core_gpu2 = {
    .core = &g_soc_core_core,
    .id = 0x6,
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
clk_node_t g_ckgen_ip_vpu2 = {
    .core = &g_soc_ip_core,
    .id = 0x1,
    .rate = 400000000,
    .mux = 3,
    .parent = CLK_NODE(g_pll2_div_a),
};
clk_node_t g_ckgen_ip_vpu3 = {
    .core = &g_soc_ip_core,
    .id = 0x2,
    .rate = 400000000,
    .mux = 3,
    .parent = CLK_NODE(g_pll2_div_a),
};
clk_node_t g_ckgen_ip_vpu4 = {
    .core = &g_soc_ip_core,
    .id = 0x3,
    .rate = 400000000,
    .mux = 3,
    .parent = CLK_NODE(g_pll2_div_a),
};
clk_node_t g_ckgen_bus_vpu_bus_root = {
    .core = &g_soc_bus_core,
    .id = 0x0,
    .rate = 400000000,
    .mux = 3,
    .parent = CLK_NODE(g_pll2_div_a),
};
clk_node_t g_ckgen_bus_vpu_bus_div_m = {
    .core = &g_soc_bus_core,
    .id = 0x4000000,
    .rate = 400000000,
    .mux = 3,
    .parent = CLK_NODE(g_pll2_div_a),
};
clk_node_t g_ckgen_bus_vpu_bus_div_n = {
    .core = &g_soc_bus_core,
    .id = 0x3000000,
    .rate = 400000000,
    .mux = 3,
    .parent = CLK_NODE(g_pll2_div_a),
};
clk_node_t g_ckgen_bus_vpu_bus_div_p = {
    .core = &g_soc_bus_core,
    .id = 0x2000000,
    .rate = 400000000,
    .mux = 3,
    .parent = CLK_NODE(g_pll2_div_a),
};
clk_node_t g_ckgen_bus_vpu_bus_div_q = {
    .core = &g_soc_bus_core,
    .id = 0x1000000,
    .rate = 400000000,
    .mux = 3,
    .parent = CLK_NODE(g_pll2_div_a),
};
clk_node_t g_ckgen_bus_vsn_bus_root = {
    .core = &g_soc_bus_core,
    .id = 0x1,
    .rate = 400000000,
    .mux = 3,
    .parent = CLK_NODE(g_pll2_div_a),
};
clk_node_t g_ckgen_bus_vsn_bus_div_m = {
    .core = &g_soc_bus_core,
    .id = 0x4000001,
    .rate = 400000000,
    .mux = 3,
    .parent = CLK_NODE(g_pll2_div_a),
};
clk_node_t g_ckgen_bus_vsn_bus_div_n = {
    .core = &g_soc_bus_core,
    .id = 0x3000001,
    .rate = 400000000,
    .mux = 3,
    .parent = CLK_NODE(g_pll2_div_a),
};
clk_node_t g_ckgen_bus_vsn_bus_div_p = {
    .core = &g_soc_bus_core,
    .id = 0x2000001,
    .rate = 400000000,
    .mux = 3,
    .parent = CLK_NODE(g_pll2_div_a),
};
clk_node_t g_ckgen_bus_vsn_bus_div_q = {
    .core = &g_soc_bus_core,
    .id = 0x1000001,
    .rate = 400000000,
    .mux = 3,
    .parent = CLK_NODE(g_pll2_div_a),
};
clk_node_t g_ckgen_ip_vsn2_bus = {
    .core = &g_soc_ip_core,
    .id = 0x5,
    .rate = 400000000,
    .mux = 3,
    .parent = CLK_NODE(g_pll2_div_a),
};
clk_node_t g_ckgen_core_ddr = {
    .core = &g_soc_core_core,
    .id = 0x7,
    .rate = 400000000,
    .mux = 3,
    .parent = CLK_NODE(g_pll2_div_a),
};
clk_node_t g_ckgen_bus_his_bus_root = {
    .core = &g_soc_bus_core,
    .id = 0x2,
    .rate = 400000000,
    .mux = 3,
    .parent = CLK_NODE(g_pll2_div_a),
};
clk_node_t g_ckgen_bus_his_bus_div_m = {
    .core = &g_soc_bus_core,
    .id = 0x4000002,
    .rate = 400000000,
    .mux = 3,
    .parent = CLK_NODE(g_pll2_div_a),
};
clk_node_t g_ckgen_bus_his_bus_div_n = {
    .core = &g_soc_bus_core,
    .id = 0x3000002,
    .rate = 400000000,
    .mux = 3,
    .parent = CLK_NODE(g_pll2_div_a),
};
clk_node_t g_ckgen_bus_his_bus_div_p = {
    .core = &g_soc_bus_core,
    .id = 0x2000002,
    .rate = 400000000,
    .mux = 3,
    .parent = CLK_NODE(g_pll2_div_a),
};
clk_node_t g_ckgen_bus_his_bus_div_q = {
    .core = &g_soc_bus_core,
    .id = 0x1000002,
    .rate = 400000000,
    .mux = 3,
    .parent = CLK_NODE(g_pll2_div_a),
};
clk_node_t g_ckgen_bus_noce_main_root = {
    .core = &g_soc_bus_core,
    .id = 0x3,
    .rate = 400000000,
    .mux = 3,
    .parent = CLK_NODE(g_pll2_div_a),
};
clk_node_t g_ckgen_bus_noce_main_div_m = {
    .core = &g_soc_bus_core,
    .id = 0x4000003,
    .rate = 400000000,
    .mux = 3,
    .parent = CLK_NODE(g_pll2_div_a),
};
clk_node_t g_ckgen_bus_noce_main_div_n = {
    .core = &g_soc_bus_core,
    .id = 0x3000003,
    .rate = 400000000,
    .mux = 3,
    .parent = CLK_NODE(g_pll2_div_a),
};
clk_node_t g_ckgen_bus_noce_main_div_p = {
    .core = &g_soc_bus_core,
    .id = 0x2000003,
    .rate = 400000000,
    .mux = 3,
    .parent = CLK_NODE(g_pll2_div_a),
};
clk_node_t g_ckgen_bus_noce_main_div_q = {
    .core = &g_soc_bus_core,
    .id = 0x1000003,
    .rate = 400000000,
    .mux = 3,
    .parent = CLK_NODE(g_pll2_div_a),
};
clk_node_t g_ckgen_bus_noce_mainp_root = {
    .core = &g_soc_bus_core,
    .id = 0x4,
    .rate = 400000000,
    .mux = 3,
    .parent = CLK_NODE(g_pll2_div_a),
};
clk_node_t g_ckgen_bus_noce_mainp_div_m = {
    .core = &g_soc_bus_core,
    .id = 0x4000004,
    .rate = 400000000,
    .mux = 3,
    .parent = CLK_NODE(g_pll2_div_a),
};
clk_node_t g_ckgen_bus_noce_mainp_div_n = {
    .core = &g_soc_bus_core,
    .id = 0x3000004,
    .rate = 400000000,
    .mux = 3,
    .parent = CLK_NODE(g_pll2_div_a),
};
clk_node_t g_ckgen_bus_noce_mainp_div_p = {
    .core = &g_soc_bus_core,
    .id = 0x2000004,
    .rate = 400000000,
    .mux = 3,
    .parent = CLK_NODE(g_pll2_div_a),
};
clk_node_t g_ckgen_bus_noce_mainp_div_q = {
    .core = &g_soc_bus_core,
    .id = 0x1000004,
    .rate = 400000000,
    .mux = 3,
    .parent = CLK_NODE(g_pll2_div_a),
};
clk_node_t g_ckgen_bus_nocw_main_root = {
    .core = &g_soc_bus_core,
    .id = 0x5,
    .rate = 800000000,
    .mux = 4,
    .parent = CLK_NODE(g_pll_hpiw_root),
};
clk_node_t g_ckgen_bus_nocw_main_div_m = {
    .core = &g_soc_bus_core,
    .id = 0x4000005,
    .rate = 400000000,
    .mux = 4,
    .parent = CLK_NODE(g_pll_hpiw_root),
};
clk_node_t g_ckgen_bus_nocw_main_div_n = {
    .core = &g_soc_bus_core,
    .id = 0x3000005,
    .rate = 133333333,
    .mux = 4,
    .parent = CLK_NODE(g_pll_hpiw_root),
};
clk_node_t g_ckgen_bus_nocw_main_div_p = {
    .core = &g_soc_bus_core,
    .id = 0x2000005,
    .rate = 133333333,
    .mux = 4,
    .parent = CLK_NODE(g_pll_hpiw_root),
};
clk_node_t g_ckgen_bus_nocw_main_div_q = {
    .core = &g_soc_bus_core,
    .id = 0x1000005,
    .rate = 100000000,
    .mux = 4,
    .parent = CLK_NODE(g_pll_hpiw_root),
};
clk_node_t g_ckgen_bus_nocw_mainp_root = {
    .core = &g_soc_bus_core,
    .id = 0x6,
    .rate = 533330000,
    .mux = 4,
    .parent = CLK_NODE(g_pll_hpiw_div_a),
};
clk_node_t g_ckgen_bus_nocw_mainp_div_m = {
    .core = &g_soc_bus_core,
    .id = 0x4000006,
    .rate = 266665000,
    .mux = 4,
    .parent = CLK_NODE(g_pll_hpiw_div_a),
};
clk_node_t g_ckgen_bus_nocw_mainp_div_n = {
    .core = &g_soc_bus_core,
    .id = 0x3000006,
    .rate = 88888333,
    .mux = 4,
    .parent = CLK_NODE(g_pll_hpiw_div_a),
};
clk_node_t g_ckgen_bus_nocw_mainp_div_p = {
    .core = &g_soc_bus_core,
    .id = 0x2000006,
    .rate = 88888333,
    .mux = 4,
    .parent = CLK_NODE(g_pll_hpiw_div_a),
};
clk_node_t g_ckgen_bus_nocw_mainp_div_q = {
    .core = &g_soc_bus_core,
    .id = 0x1000006,
    .rate = 66666250,
    .mux = 4,
    .parent = CLK_NODE(g_pll_hpiw_div_a),
};
clk_node_t g_ckgen_ip_dci = {
    .core = &g_soc_ip_core,
    .id = 0x6,
    .rate = 400000000,
    .mux = 3,
    .parent = CLK_NODE(g_pll2_div_a),
};
clk_node_t g_ckgen_uuu_cpu1a_div_m = {
    .core = &g_soc_uuu_core,
    .id = 0x3000000,
    .rate = 1800000000,
    .mux = 1,
    .parent = CLK_NODE(g_pll_cpu1a_root),
};
clk_node_t g_ckgen_uuu_cpu1b_div_m = {
    .core = &g_soc_uuu_core,
    .id = 0x3000001,
    .rate = 1800000000,
    .mux = 1,
    .parent = CLK_NODE(g_pll_cpu1b_root),
};
clk_node_t g_ckgen_uuu_cpu1c_div_m = {
    .core = &g_soc_uuu_core,
    .id = 0x3000002,
    .rate = 1168000000,
    .mux = 1,
    .parent = CLK_NODE(g_pll_cpu1c_root),
};
clk_node_t g_ckgen_uuu_cpu1c_div_n = {
    .core = &g_soc_uuu_core,
    .id = 0x2000002,
    .rate = 584000000,
    .mux = 1,
    .parent = CLK_NODE(g_pll_cpu1c_root),
};
clk_node_t g_ckgen_uuu_cpu1c_div_p = {
    .core = &g_soc_uuu_core,
    .id = 0x1000002,
    .rate = 292000000,
    .mux = 1,
    .parent = CLK_NODE(g_pll_cpu1c_root),
};
clk_node_t g_ckgen_uuu_cpu2a_div_m = {
    .core = &g_soc_uuu_core,
    .id = 0x3000003,
    .rate = 900000000,
    .mux = 1,
    .parent = CLK_NODE(g_pll_cpu2a_root),
};
clk_node_t g_ckgen_uuu_cpu2b_div_m = {
    .core = &g_soc_uuu_core,
    .id = 0x3000004,
    .rate = 700000000,
    .mux = 1,
    .parent = CLK_NODE(g_pll_cpu2b_root),
};
clk_node_t g_ckgen_uuu_cpu2b_div_n = {
    .core = &g_soc_uuu_core,
    .id = 0x2000004,
    .rate = 350000000,
    .mux = 1,
    .parent = CLK_NODE(g_pll_cpu2b_root),
};
clk_node_t g_ckgen_uuu_cpu2b_div_p = {
    .core = &g_soc_uuu_core,
    .id = 0x1000004,
    .rate = 175000000,
    .mux = 1,
    .parent = CLK_NODE(g_pll_cpu2b_root),
};
clk_node_t g_ckgen_uuu_gpu1_div_m = {
    .core = &g_soc_uuu_core,
    .id = 0x3000005,
    .rate = 756000000,
    .mux = 1,
    .parent = CLK_NODE(g_pll_gpu1_root),
};
clk_node_t g_ckgen_uuu_gpu1_div_n = {
    .core = &g_soc_uuu_core,
    .id = 0x2000005,
    .rate = 756000000,
    .mux = 1,
    .parent = CLK_NODE(g_pll_gpu1_root),
};
clk_node_t g_ckgen_uuu_gpu1_div_p = {
    .core = &g_soc_uuu_core,
    .id = 0x1000005,
    .rate = 189000000,
    .mux = 1,
    .parent = CLK_NODE(g_pll_gpu1_root),
};
clk_node_t g_ckgen_uuu_gpu2_div_m = {
    .core = &g_soc_uuu_core,
    .id = 0x3000006,
    .rate = 756000000,
    .mux = 1,
    .parent = CLK_NODE(g_pll_gpu2_root),
};
clk_node_t g_ckgen_uuu_gpu2_div_n = {
    .core = &g_soc_uuu_core,
    .id = 0x2000006,
    .rate = 756000000,
    .mux = 1,
    .parent = CLK_NODE(g_pll_gpu2_root),
};
clk_node_t g_ckgen_uuu_gpu2_div_p = {
    .core = &g_soc_uuu_core,
    .id = 0x1000006,
    .rate = 189000000,
    .mux = 1,
    .parent = CLK_NODE(g_pll_gpu2_root),
};
clk_node_t g_ckgen_uuu_vpu1_div_m = {
    .core = &g_soc_uuu_core,
    .id = 0x3000007,
    .rate = 600000000,
    .mux = 1,
    .parent = CLK_NODE(g_pll_vpu1_root),
};
clk_node_t g_ckgen_uuu_vpu2_div_m = {
    .core = &g_soc_uuu_core,
    .id = 0x3000008,
    .rate = 600000000,
    .mux = 1,
    .parent = CLK_NODE(g_pll_vpu1_root),
};
clk_node_t g_ckgen_uuu_vpu3_div_m = {
    .core = &g_soc_uuu_core,
    .id = 0x3000009,
    .rate = 600000000,
    .mux = 1,
    .parent = CLK_NODE(g_pll_vpu1_root),
};
clk_node_t g_ckgen_uuu_vpu4_div_m = {
    .core = &g_soc_uuu_core,
    .id = 0x300000a,
    .rate = 720000000,
    .mux = 1,
    .parent = CLK_NODE(g_pll_vpu2_root),
};
clk_node_t g_ckgen_uuu_vpu_bus_div_m = {
    .core = &g_soc_uuu_core,
    .id = 0x300000b,
    .rate = 750000000,
    .mux = 1,
    .parent = CLK_NODE(g_pll_vpu1_root),
};
clk_node_t g_ckgen_uuu_vpu_bus_div_n = {
    .core = &g_soc_uuu_core,
    .id = 0x200000b,
    .rate = 375000000,
    .mux = 1,
    .parent = CLK_NODE(g_pll_vpu1_root),
};
clk_node_t g_ckgen_uuu_vsn_bus_div_m = {
    .core = &g_soc_uuu_core,
    .id = 0x300000c,
    .rate = 776000000,
    .mux = 1,
    .parent = CLK_NODE(g_pll_vsn_root),
};
clk_node_t g_ckgen_uuu_vsn_bus_div_n = {
    .core = &g_soc_uuu_core,
    .id = 0x200000c,
    .rate = 194000000,
    .mux = 1,
    .parent = CLK_NODE(g_pll_vsn_root),
};
clk_node_t g_ckgen_uuu_vsn2_bus_div_m = {
    .core = &g_soc_uuu_core,
    .id = 0x300000d,
    .rate = 776000000,
    .mux = 1,
    .parent = CLK_NODE(g_pll_vsn2_root),
};
clk_node_t g_ckgen_uuu_vsn2_bus_div_n = {
    .core = &g_soc_uuu_core,
    .id = 0x200000d,
    .rate = 194000000,
    .mux = 1,
    .parent = CLK_NODE(g_pll_vsn2_root),
};
clk_node_t g_ckgen_uuu_ddr_div_m = {
    .core = &g_soc_uuu_core,
    .id = 0x300000e,
    .rate = 1064000000,
    .mux = 1,
    .parent = CLK_NODE(g_pll_ddr_root),
};
clk_node_t g_ckgen_uuu_ddr_div_n = {
    .core = &g_soc_uuu_core,
    .id = 0x200000e,
    .rate = 532000000,
    .mux = 1,
    .parent = CLK_NODE(g_pll_ddr_root),
};
clk_node_t g_ckgen_uuu_ddr_div_p = {
    .core = &g_soc_uuu_core,
    .id = 0x100000e,
    .rate = 266000000,
    .mux = 1,
    .parent = CLK_NODE(g_pll_ddr_root),
};
clk_node_t g_ckgen_uuu_ddr_div_q = {
    .core = &g_soc_uuu_core,
    .id = 0xe,
    .rate = 133000000,
    .mux = 1,
    .parent = CLK_NODE(g_pll_ddr_root),
};
clk_node_t g_ckgen_uuu_his_bus_div_m = {
    .core = &g_soc_uuu_core,
    .id = 0x300000f,
    .rate = 1000000000,
    .mux = 1,
    .parent = CLK_NODE(g_pll_his_root),
};
clk_node_t g_ckgen_uuu_his_bus_div_n = {
    .core = &g_soc_uuu_core,
    .id = 0x200000f,
    .rate = 200000000,
    .mux = 1,
    .parent = CLK_NODE(g_pll_his_root),
};
clk_node_t g_ckgen_uuu_his_bus_div_p = {
    .core = &g_soc_uuu_core,
    .id = 0x100000f,
    .rate = 666666700,
    .mux = 1,
    .parent = CLK_NODE(g_pll_his_root),
};
clk_node_t g_ckgen_uuu_his_bus_div_q = {
    .core = &g_soc_uuu_core,
    .id = 0xf,
    .rate = 333333300,
    .mux = 1,
    .parent = CLK_NODE(g_pll_his_root),
};
clk_node_t g_ckgen_uuu_noce_main_div_m = {
    .core = &g_soc_uuu_core,
    .id = 0x3000010,
    .rate = 1064000000,
    .mux = 1,
    .parent = CLK_NODE(g_pll_hpie_root),
};
clk_node_t g_ckgen_uuu_noce_main_div_n = {
    .core = &g_soc_uuu_core,
    .id = 0x2000010,
    .rate = 266000000,
    .mux = 1,
    .parent = CLK_NODE(g_pll_hpie_root),
};
clk_node_t g_ckgen_uuu_noce_mainp_div_m = {
    .core = &g_soc_uuu_core,
    .id = 0x3000011,
    .rate = 798000000,
    .mux = 1,
    .parent = CLK_NODE(g_pll_hpie_root),
};
clk_node_t g_ckgen_uuu_noce_mainp_div_n = {
    .core = &g_soc_uuu_core,
    .id = 0x2000011,
    .rate = 399000000,
    .mux = 1,
    .parent = CLK_NODE(g_pll_hpie_root),
};
clk_node_t g_ckgen_gating_cr5_safe = {
    .core = &g_saf_gating_core,
    .id = 0x0,
};
clk_node_t g_ckgen_gating_noc_safe_main = {
    .core = &g_saf_gating_core,
    .id = 0x2,
};
clk_node_t g_ckgen_gating_noc_mainb_bus = {
    .core = &g_saf_gating_core,
    .id = 0x3,
};
clk_node_t g_ckgen_gating_gic1 = {
    .core = &g_saf_gating_core,
    .id = 0x4,
};
clk_node_t g_ckgen_gating_dma1 = {
    .core = &g_saf_gating_core,
    .id = 0x5,
};
clk_node_t g_ckgen_gating_dma9 = {
    .core = &g_saf_gating_core,
    .id = 0x6,
};
clk_node_t g_ckgen_gating_enet1 = {
    .core = &g_saf_gating_core,
    .id = 0x7,
};
clk_node_t g_ckgen_gating_ospi1 = {
    .core = &g_saf_gating_core,
    .id = 0x8,
};
clk_node_t g_ckgen_gating_rdc_safe = {
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
clk_node_t g_ckgen_gating_mmu_switch_safe = {
    .core = &g_saf_gating_core,
    .id = 0xc,
};
clk_node_t g_ckgen_gating_tbu20 = {
    .core = &g_saf_gating_core,
    .id = 0xd,
};
clk_node_t g_ckgen_gating_tbu21 = {
    .core = &g_saf_gating_core,
    .id = 0xe,
};
clk_node_t g_ckgen_gating_tbu22 = {
    .core = &g_saf_gating_core,
    .id = 0xf,
};
clk_node_t g_ckgen_gating_tbu23 = {
    .core = &g_saf_gating_core,
    .id = 0x10,
};
clk_node_t g_ckgen_gating_tbu24 = {
    .core = &g_saf_gating_core,
    .id = 0x11,
};
clk_node_t g_ckgen_gating_ce1 = {
    .core = &g_saf_gating_core,
    .id = 0x12,
};
clk_node_t g_ckgen_gating_xspi_slv = {
    .core = &g_saf_gating_core,
    .id = 0x13,
};
clk_node_t g_ckgen_gating_noc_safe = {
    .core = &g_saf_gating_core,
    .id = 0x14,
};
clk_node_t g_ckgen_gating_noc_mainb_busdiv = {
    .core = &g_saf_gating_core,
    .id = 0x15,
};
clk_node_t g_ckgen_gating_i2c1 = {
    .core = &g_saf_gating_core,
    .id = 0x17,
};
clk_node_t g_ckgen_gating_i2c2 = {
    .core = &g_saf_gating_core,
    .id = 0x18,
};
clk_node_t g_ckgen_gating_i2c3 = {
    .core = &g_saf_gating_core,
    .id = 0x19,
};
clk_node_t g_ckgen_gating_i2c4 = {
    .core = &g_saf_gating_core,
    .id = 0x1a,
};
clk_node_t g_ckgen_gating_i2c17 = {
    .core = &g_saf_gating_core,
    .id = 0x1b,
};
clk_node_t g_ckgen_gating_i2c18 = {
    .core = &g_saf_gating_core,
    .id = 0x1c,
};
clk_node_t g_ckgen_gating_i2c19 = {
    .core = &g_saf_gating_core,
    .id = 0x1d,
};
clk_node_t g_ckgen_gating_i2c20 = {
    .core = &g_saf_gating_core,
    .id = 0x1e,
};
clk_node_t g_ckgen_gating_spi1 = {
    .core = &g_saf_gating_core,
    .id = 0x1f,
};
clk_node_t g_ckgen_gating_spi2 = {
    .core = &g_saf_gating_core,
    .id = 0x20,
};
clk_node_t g_ckgen_gating_spi3 = {
    .core = &g_saf_gating_core,
    .id = 0x21,
};
clk_node_t g_ckgen_gating_spi4 = {
    .core = &g_saf_gating_core,
    .id = 0x22,
};
clk_node_t g_ckgen_gating_spi9 = {
    .core = &g_saf_gating_core,
    .id = 0x23,
};
clk_node_t g_ckgen_gating_spi10 = {
    .core = &g_saf_gating_core,
    .id = 0x24,
};
clk_node_t g_ckgen_gating_spi11 = {
    .core = &g_saf_gating_core,
    .id = 0x25,
};
clk_node_t g_ckgen_gating_spi12 = {
    .core = &g_saf_gating_core,
    .id = 0x26,
};
clk_node_t g_ckgen_gating_uart1 = {
    .core = &g_saf_gating_core,
    .id = 0x27,
};
clk_node_t g_ckgen_gating_uart2 = {
    .core = &g_saf_gating_core,
    .id = 0x28,
};
clk_node_t g_ckgen_gating_uart3 = {
    .core = &g_saf_gating_core,
    .id = 0x29,
};
clk_node_t g_ckgen_gating_uart4 = {
    .core = &g_saf_gating_core,
    .id = 0x2a,
};
clk_node_t g_ckgen_gating_uart5 = {
    .core = &g_saf_gating_core,
    .id = 0x2b,
};
clk_node_t g_ckgen_gating_uart6 = {
    .core = &g_saf_gating_core,
    .id = 0x2c,
};
clk_node_t g_ckgen_gating_uart7 = {
    .core = &g_saf_gating_core,
    .id = 0x2d,
};
clk_node_t g_ckgen_gating_uart8 = {
    .core = &g_saf_gating_core,
    .id = 0x2e,
};
clk_node_t g_ckgen_gating_i2s_sc1 = {
    .core = &g_saf_gating_core,
    .id = 0x2f,
};
clk_node_t g_ckgen_gating_i2s_sc2 = {
    .core = &g_saf_gating_core,
    .id = 0x30,
};
clk_node_t g_ckgen_gating_wdt1 = {
    .core = &g_saf_gating_core,
    .id = 0x31,
};
clk_node_t g_ckgen_gating_wdt2 = {
    .core = &g_saf_gating_core,
    .id = 0x32,
};
clk_node_t g_ckgen_gating_gpio1 = {
    .core = &g_saf_gating_core,
    .id = 0x33,
};
clk_node_t g_ckgen_gating_scr_safe = {
    .core = &g_saf_gating_core,
    .id = 0x34,
};
clk_node_t g_ckgen_gating_pvt_sns_safe = {
    .core = &g_saf_gating_core,
    .id = 0x35,
};
clk_node_t g_ckgen_gating_pll1 = {
    .core = &g_saf_gating_core,
    .id = 0x36,
};
clk_node_t g_ckgen_gating_pll2 = {
    .core = &g_saf_gating_core,
    .id = 0x37,
};
clk_node_t g_ckgen_gating_iomux_ctrl_safe = {
    .core = &g_saf_gating_core,
    .id = 0x38,
};
clk_node_t g_ckgen_gating_rpc_safe = {
    .core = &g_saf_gating_core,
    .id = 0x39,
};
clk_node_t g_ckgen_gating_xtal_safe = {
    .core = &g_saf_gating_core,
    .id = 0x3a,
};
clk_node_t g_ckgen_gating_efuse = {
    .core = &g_saf_gating_core,
    .id = 0x3b,
};
clk_node_t g_ckgen_gating_timer1 = {
    .core = &g_saf_gating_core,
    .id = 0x3c,
};
clk_node_t g_ckgen_gating_pwm1 = {
    .core = &g_saf_gating_core,
    .id = 0x3d,
};
clk_node_t g_ckgen_gating_pwm2 = {
    .core = &g_saf_gating_core,
    .id = 0x3e,
};
clk_node_t g_ckgen_gating_canfd1 = {
    .core = &g_saf_gating_core,
    .id = 0x3f,
};
clk_node_t g_ckgen_gating_canfd2 = {
    .core = &g_saf_gating_core,
    .id = 0x40,
};
clk_node_t g_ckgen_gating_canfd3 = {
    .core = &g_saf_gating_core,
    .id = 0x41,
};
clk_node_t g_ckgen_gating_canfd4 = {
    .core = &g_saf_gating_core,
    .id = 0x42,
};
clk_node_t g_ckgen_gating_cr5_safe_atclk = {
    .core = &g_saf_gating_core,
    .id = 0x43,
};
clk_node_t g_ckgen_gating_cr5_safe_pclk_dbg = {
    .core = &g_saf_gating_core,
    .id = 0x44,
};
clk_node_t g_ckgen_gating_sem1 = {
    .core = &g_saf_gating_core,
    .id = 0x45,
};
clk_node_t g_ckgen_gating_sem2 = {
    .core = &g_saf_gating_core,
    .id = 0x46,
};
clk_node_t g_ckgen_gating_rc_24m = {
    .core = &g_saf_gating_core,
    .id = 0x47,
};
clk_node_t g_ckgen_gating_eic_safe = {
    .core = &g_saf_gating_core,
    .id = 0x48,
};
clk_node_t g_ckgen_gating_lin1 = {
    .core = &g_saf_gating_core,
    .id = 0x49,
};
clk_node_t g_ckgen_gating_lin2 = {
    .core = &g_saf_gating_core,
    .id = 0x4a,
};
clk_node_t g_ckgen_gating_lin3 = {
    .core = &g_saf_gating_core,
    .id = 0x4b,
};
clk_node_t g_ckgen_gating_lin4 = {
    .core = &g_saf_gating_core,
    .id = 0x4c,
};
clk_node_t g_ckgen_gating_lin5 = {
    .core = &g_saf_gating_core,
    .id = 0x4d,
};
clk_node_t g_ckgen_gating_lin6 = {
    .core = &g_saf_gating_core,
    .id = 0x4e,
};
clk_node_t g_ckgen_gating_lin7 = {
    .core = &g_saf_gating_core,
    .id = 0x4f,
};
clk_node_t g_ckgen_gating_lin8 = {
    .core = &g_saf_gating_core,
    .id = 0x50,
};
clk_node_t g_ckgen_gating_adc3 = {
    .core = &g_saf_gating_core,
    .id = 0x51,
};
clk_node_t g_ckgen_gating_adc2 = {
    .core = &g_saf_gating_core,
    .id = 0x52,
};
clk_node_t g_ckgen_gating_noc_tp = {
    .core = &g_saf_gating_core,
    .id = 0x53,
};
clk_node_t g_ckgen_gating_axislvmux_ce1 = {
    .core = &g_saf_gating_core,
    .id = 0x54,
};
clk_node_t g_ckgen_gating_xspi1a_apb_sync = {
    .core = &g_saf_gating_core,
    .id = 0x55,
};
clk_node_t g_ckgen_gating_xspi1b_apb_sync = {
    .core = &g_saf_gating_core,
    .id = 0x56,
};
clk_node_t g_ckgen_gating_timer2 = {
    .core = &g_saf_gating_core,
    .id = 0x57,
};
clk_node_t g_ckgen_gating_xspi1_split = {
    .core = &g_saf_gating_core,
    .id = 0x58,
};
clk_node_t g_ckgen_gating_rtc = {
    .core = &g_saf_gating_core,
    .id = 0x61,
};
clk_node_t g_ckgen_gating_cr5_mp = {
    .core = &g_sec_gating_core,
    .id = 0x0,
};
clk_node_t g_ckgen_gating_cr5_sec = {
    .core = &g_sec_gating_core,
    .id = 0x1,
};
clk_node_t g_ckgen_gating_noc_sec_main = {
    .core = &g_sec_gating_core,
    .id = 0x3,
};
clk_node_t g_ckgen_gating_noc_sec_m_main = {
    .core = &g_sec_gating_core,
    .id = 0x4,
};
clk_node_t g_ckgen_gating_gic2 = {
    .core = &g_sec_gating_core,
    .id = 0x5,
};
clk_node_t g_ckgen_gating_gic3 = {
    .core = &g_sec_gating_core,
    .id = 0x6,
};
clk_node_t g_ckgen_gating_mshc1 = {
    .core = &g_sec_gating_core,
    .id = 0x7,
};
clk_node_t g_ckgen_gating_mshc2 = {
    .core = &g_sec_gating_core,
    .id = 0x8,
};
clk_node_t g_ckgen_gating_mshc3 = {
    .core = &g_sec_gating_core,
    .id = 0x9,
};
clk_node_t g_ckgen_gating_mshc4 = {
    .core = &g_sec_gating_core,
    .id = 0xa,
};
clk_node_t g_ckgen_gating_enet2 = {
    .core = &g_sec_gating_core,
    .id = 0xb,
};
clk_node_t g_ckgen_gating_ospi2 = {
    .core = &g_sec_gating_core,
    .id = 0xc,
};
clk_node_t g_ckgen_gating_dma2 = {
    .core = &g_sec_gating_core,
    .id = 0xd,
};
clk_node_t g_ckgen_gating_dma3 = {
    .core = &g_sec_gating_core,
    .id = 0xe,
};
clk_node_t g_ckgen_gating_dma4 = {
    .core = &g_sec_gating_core,
    .id = 0xf,
};
clk_node_t g_ckgen_gating_dma5 = {
    .core = &g_sec_gating_core,
    .id = 0x10,
};
clk_node_t g_ckgen_gating_dma6 = {
    .core = &g_sec_gating_core,
    .id = 0x11,
};
clk_node_t g_ckgen_gating_dma7 = {
    .core = &g_sec_gating_core,
    .id = 0x12,
};
clk_node_t g_ckgen_gating_dma8 = {
    .core = &g_sec_gating_core,
    .id = 0x13,
};
clk_node_t g_ckgen_gating_axi2ahb_mb = {
    .core = &g_sec_gating_core,
    .id = 0x14,
};
clk_node_t g_ckgen_gating_mb = {
    .core = &g_sec_gating_core,
    .id = 0x15,
};
clk_node_t g_ckgen_gating_iram2 = {
    .core = &g_sec_gating_core,
    .id = 0x16,
};
clk_node_t g_ckgen_gating_iram3 = {
    .core = &g_sec_gating_core,
    .id = 0x17,
};
clk_node_t g_ckgen_gating_iram4 = {
    .core = &g_sec_gating_core,
    .id = 0x18,
};
clk_node_t g_ckgen_gating_romc2 = {
    .core = &g_sec_gating_core,
    .id = 0x19,
};
clk_node_t g_ckgen_gating_tbu18 = {
    .core = &g_sec_gating_core,
    .id = 0x1a,
};
clk_node_t g_ckgen_gating_tbu19 = {
    .core = &g_sec_gating_core,
    .id = 0x1b,
};
clk_node_t g_ckgen_gating_ce2 = {
    .core = &g_sec_gating_core,
    .id = 0x1c,
};
clk_node_t g_ckgen_gating_adti_whpi2sec_mst = {
    .core = &g_sec_gating_core,
    .id = 0x1d,
};
clk_node_t g_ckgen_gating_mmu_switch_sec = {
    .core = &g_sec_gating_core,
    .id = 0x1f,
};
clk_node_t g_ckgen_gating_noc_sec_per = {
    .core = &g_sec_gating_core,
    .id = 0x20,
};
clk_node_t g_ckgen_gating_noc_sec_m_per = {
    .core = &g_sec_gating_core,
    .id = 0x21,
};
clk_node_t g_ckgen_gating_i2c5 = {
    .core = &g_sec_gating_core,
    .id = 0x22,
};
clk_node_t g_ckgen_gating_i2c6 = {
    .core = &g_sec_gating_core,
    .id = 0x23,
};
clk_node_t g_ckgen_gating_i2c7 = {
    .core = &g_sec_gating_core,
    .id = 0x24,
};
clk_node_t g_ckgen_gating_i2c8 = {
    .core = &g_sec_gating_core,
    .id = 0x25,
};
clk_node_t g_ckgen_gating_i2c9 = {
    .core = &g_sec_gating_core,
    .id = 0x26,
};
clk_node_t g_ckgen_gating_i2c10 = {
    .core = &g_sec_gating_core,
    .id = 0x27,
};
clk_node_t g_ckgen_gating_i2c11 = {
    .core = &g_sec_gating_core,
    .id = 0x28,
};
clk_node_t g_ckgen_gating_i2c12 = {
    .core = &g_sec_gating_core,
    .id = 0x29,
};
clk_node_t g_ckgen_gating_i2c13 = {
    .core = &g_sec_gating_core,
    .id = 0x2a,
};
clk_node_t g_ckgen_gating_i2c14 = {
    .core = &g_sec_gating_core,
    .id = 0x2b,
};
clk_node_t g_ckgen_gating_i2c15 = {
    .core = &g_sec_gating_core,
    .id = 0x2c,
};
clk_node_t g_ckgen_gating_i2c16 = {
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
clk_node_t g_ckgen_gating_timer3 = {
    .core = &g_sec_gating_core,
    .id = 0x36,
};
clk_node_t g_ckgen_gating_timer4 = {
    .core = &g_sec_gating_core,
    .id = 0x37,
};
clk_node_t g_ckgen_gating_timer5 = {
    .core = &g_sec_gating_core,
    .id = 0x38,
};
clk_node_t g_ckgen_gating_timer6 = {
    .core = &g_sec_gating_core,
    .id = 0x39,
};
clk_node_t g_ckgen_gating_timer7 = {
    .core = &g_sec_gating_core,
    .id = 0x3a,
};
clk_node_t g_ckgen_gating_timer8 = {
    .core = &g_sec_gating_core,
    .id = 0x3b,
};
clk_node_t g_ckgen_gating_pwm3 = {
    .core = &g_sec_gating_core,
    .id = 0x3c,
};
clk_node_t g_ckgen_gating_pwm4 = {
    .core = &g_sec_gating_core,
    .id = 0x3d,
};
clk_node_t g_ckgen_gating_pwm5 = {
    .core = &g_sec_gating_core,
    .id = 0x3e,
};
clk_node_t g_ckgen_gating_pwm6 = {
    .core = &g_sec_gating_core,
    .id = 0x3f,
};
clk_node_t g_ckgen_gating_pwm7 = {
    .core = &g_sec_gating_core,
    .id = 0x40,
};
clk_node_t g_ckgen_gating_pwm8 = {
    .core = &g_sec_gating_core,
    .id = 0x41,
};
clk_node_t g_ckgen_gating_i2s_mc1 = {
    .core = &g_sec_gating_core,
    .id = 0x42,
};
clk_node_t g_ckgen_gating_i2s_mc2 = {
    .core = &g_sec_gating_core,
    .id = 0x43,
};
clk_node_t g_ckgen_gating_i2s_sc3 = {
    .core = &g_sec_gating_core,
    .id = 0x44,
};
clk_node_t g_ckgen_gating_i2s_sc4 = {
    .core = &g_sec_gating_core,
    .id = 0x45,
};
clk_node_t g_ckgen_gating_i2s_sc5 = {
    .core = &g_sec_gating_core,
    .id = 0x46,
};
clk_node_t g_ckgen_gating_i2s_sc6 = {
    .core = &g_sec_gating_core,
    .id = 0x47,
};
clk_node_t g_ckgen_gating_i2s_sc7 = {
    .core = &g_sec_gating_core,
    .id = 0x48,
};
clk_node_t g_ckgen_gating_i2s_sc8 = {
    .core = &g_sec_gating_core,
    .id = 0x49,
};
clk_node_t g_ckgen_gating_gpio2 = {
    .core = &g_sec_gating_core,
    .id = 0x4a,
};
clk_node_t g_ckgen_gating_gpio3 = {
    .core = &g_sec_gating_core,
    .id = 0x4b,
};
clk_node_t g_ckgen_gating_gpio4 = {
    .core = &g_sec_gating_core,
    .id = 0x4c,
};
clk_node_t g_ckgen_gating_gpio5 = {
    .core = &g_sec_gating_core,
    .id = 0x4d,
};
clk_node_t g_ckgen_gating_adc1 = {
    .core = &g_sec_gating_core,
    .id = 0x4e,
};
clk_node_t g_ckgen_gating_wdt3 = {
    .core = &g_sec_gating_core,
    .id = 0x4f,
};
clk_node_t g_ckgen_gating_wdt4 = {
    .core = &g_sec_gating_core,
    .id = 0x50,
};
clk_node_t g_ckgen_gating_wdt5 = {
    .core = &g_sec_gating_core,
    .id = 0x51,
};
clk_node_t g_ckgen_gating_wdt6 = {
    .core = &g_sec_gating_core,
    .id = 0x52,
};
clk_node_t g_ckgen_gating_wdt7 = {
    .core = &g_sec_gating_core,
    .id = 0x53,
};
clk_node_t g_ckgen_gating_wdt8 = {
    .core = &g_sec_gating_core,
    .id = 0x54,
};
clk_node_t g_ckgen_gating_pll3 = {
    .core = &g_sec_gating_core,
    .id = 0x55,
};
clk_node_t g_ckgen_gating_pll4 = {
    .core = &g_sec_gating_core,
    .id = 0x56,
};
clk_node_t g_ckgen_gating_pll5 = {
    .core = &g_sec_gating_core,
    .id = 0x57,
};
clk_node_t g_ckgen_gating_pll6 = {
    .core = &g_sec_gating_core,
    .id = 0x58,
};
clk_node_t g_ckgen_gating_pll7 = {
    .core = &g_sec_gating_core,
    .id = 0x59,
};
clk_node_t g_ckgen_gating_iomux_ctrl_ap = {
    .core = &g_sec_gating_core,
    .id = 0x5a,
};
clk_node_t g_ckgen_gating_scr_sec = {
    .core = &g_sec_gating_core,
    .id = 0x5b,
};
clk_node_t g_ckgen_gating_pvt_sns_sec = {
    .core = &g_sec_gating_core,
    .id = 0x5c,
};
clk_node_t g_ckgen_gating_mac = {
    .core = &g_sec_gating_core,
    .id = 0x5d,
};
clk_node_t g_ckgen_gating_rpc_sec = {
    .core = &g_sec_gating_core,
    .id = 0x5f,
};
clk_node_t g_ckgen_gating_spdif1 = {
    .core = &g_sec_gating_core,
    .id = 0x60,
};
clk_node_t g_ckgen_gating_spdif2 = {
    .core = &g_sec_gating_core,
    .id = 0x61,
};
clk_node_t g_ckgen_gating_spdif3 = {
    .core = &g_sec_gating_core,
    .id = 0x62,
};
clk_node_t g_ckgen_gating_spdif4 = {
    .core = &g_sec_gating_core,
    .id = 0x63,
};
clk_node_t g_ckgen_gating_spi5 = {
    .core = &g_sec_gating_core,
    .id = 0x64,
};
clk_node_t g_ckgen_gating_spi6 = {
    .core = &g_sec_gating_core,
    .id = 0x65,
};
clk_node_t g_ckgen_gating_spi7 = {
    .core = &g_sec_gating_core,
    .id = 0x66,
};
clk_node_t g_ckgen_gating_spi8 = {
    .core = &g_sec_gating_core,
    .id = 0x67,
};
clk_node_t g_ckgen_gating_canfd5 = {
    .core = &g_sec_gating_core,
    .id = 0x68,
};
clk_node_t g_ckgen_gating_canfd6 = {
    .core = &g_sec_gating_core,
    .id = 0x69,
};
clk_node_t g_ckgen_gating_canfd7 = {
    .core = &g_sec_gating_core,
    .id = 0x6a,
};
clk_node_t g_ckgen_gating_canfd8 = {
    .core = &g_sec_gating_core,
    .id = 0x6b,
};
clk_node_t g_ckgen_gating_canfd9 = {
    .core = &g_sec_gating_core,
    .id = 0x6c,
};
clk_node_t g_ckgen_gating_canfd10 = {
    .core = &g_sec_gating_core,
    .id = 0x6d,
};
clk_node_t g_ckgen_gating_canfd11 = {
    .core = &g_sec_gating_core,
    .id = 0x6e,
};
clk_node_t g_ckgen_gating_canfd12 = {
    .core = &g_sec_gating_core,
    .id = 0x6f,
};
clk_node_t g_ckgen_gating_canfd13 = {
    .core = &g_sec_gating_core,
    .id = 0x70,
};
clk_node_t g_ckgen_gating_canfd14 = {
    .core = &g_sec_gating_core,
    .id = 0x71,
};
clk_node_t g_ckgen_gating_canfd15 = {
    .core = &g_sec_gating_core,
    .id = 0x72,
};
clk_node_t g_ckgen_gating_canfd16 = {
    .core = &g_sec_gating_core,
    .id = 0x73,
};
clk_node_t g_ckgen_gating_canfd17 = {
    .core = &g_sec_gating_core,
    .id = 0x74,
};
clk_node_t g_ckgen_gating_canfd18 = {
    .core = &g_sec_gating_core,
    .id = 0x75,
};
clk_node_t g_ckgen_gating_canfd19 = {
    .core = &g_sec_gating_core,
    .id = 0x76,
};
clk_node_t g_ckgen_gating_canfd20 = {
    .core = &g_sec_gating_core,
    .id = 0x77,
};
clk_node_t g_ckgen_gating_eic_sec = {
    .core = &g_sec_gating_core,
    .id = 0x78,
};
clk_node_t g_ckgen_gating_gic4 = {
    .core = &g_sec_gating_core,
    .id = 0x79,
};
clk_node_t g_ckgen_gating_gic5 = {
    .core = &g_sec_gating_core,
    .id = 0x7a,
};
clk_node_t g_ckgen_gating_cssys_trace = {
    .core = &g_sec_gating_core,
    .id = 0x7b,
};
clk_node_t g_ckgen_gating_sys_cnt = {
    .core = &g_sec_gating_core,
    .id = 0x7c,
};
clk_node_t g_ckgen_gating_apb_mac_disp_slv_ = {
    .core = &g_sec_gating_core,
    .id = 0x7d,
};
clk_node_t g_ckgen_gating_apb_mac_his_slv_ = {
    .core = &g_sec_gating_core,
    .id = 0x7e,
};
clk_node_t g_ckgen_gating_aaxi_dispa_img_clk = {
    .core = &g_img_gating_core,
    .id = 0x0,
};
clk_node_t g_ckgen_gating_xtal_ap_pclk = {
    .core = &g_img_gating_core,
    .id = 0x2,
};
clk_node_t g_ckgen_gating_pll_disp1_pclk = {
    .core = &g_img_gating_core,
    .id = 0x3,
};
clk_node_t g_ckgen_gating_pll_disp2_pclk = {
    .core = &g_img_gating_core,
    .id = 0x4,
};
clk_node_t g_ckgen_gating_csi1_axiclk = {
    .core = &g_img_gating_core,
    .id = 0x5,
};
clk_node_t g_ckgen_gating_csi2_axiclk = {
    .core = &g_img_gating_core,
    .id = 0x6,
};
clk_node_t g_ckgen_gating_csi3_axiclk = {
    .core = &g_img_gating_core,
    .id = 0x7,
};
clk_node_t g_ckgen_gating_csi4_axiclk = {
    .core = &g_img_gating_core,
    .id = 0x8,
};
clk_node_t g_ckgen_gating_imux_pclk = {
    .core = &g_img_gating_core,
    .id = 0x9,
};
clk_node_t g_ckgen_gating_mipi_csi1_pclk = {
    .core = &g_img_gating_core,
    .id = 0xa,
};
clk_node_t g_ckgen_gating_mipi_csi2_pclk = {
    .core = &g_img_gating_core,
    .id = 0xb,
};
clk_node_t g_ckgen_gating_mipi_csi3_pclk = {
    .core = &g_img_gating_core,
    .id = 0xc,
};
clk_node_t g_ckgen_gating_g2d1_axiclk = {
    .core = &g_img_gating_core,
    .id = 0xd,
};
clk_node_t g_ckgen_gating_g2d2_axiclk = {
    .core = &g_img_gating_core,
    .id = 0xe,
};
clk_node_t g_ckgen_gating_dc1_axiclk = {
    .core = &g_img_gating_core,
    .id = 0x12,
};
clk_node_t g_ckgen_gating_dc2_axiclk = {
    .core = &g_img_gating_core,
    .id = 0x13,
};
clk_node_t g_ckgen_gating_dc3_axiclk = {
    .core = &g_img_gating_core,
    .id = 0x14,
};
clk_node_t g_ckgen_gating_dc4_axiclk = {
    .core = &g_img_gating_core,
    .id = 0x15,
};
clk_node_t g_ckgen_gating_dc5_axiclk = {
    .core = &g_img_gating_core,
    .id = 0x16,
};
clk_node_t g_ckgen_gating_dp1_axiclk = {
    .core = &g_img_gating_core,
    .id = 0x17,
};
clk_node_t g_ckgen_gating_dp2_axiclk = {
    .core = &g_img_gating_core,
    .id = 0x18,
};
clk_node_t g_ckgen_gating_dp3_axiclk = {
    .core = &g_img_gating_core,
    .id = 0x19,
};
clk_node_t g_ckgen_gating_iram_disp_axiclk = {
    .core = &g_img_gating_core,
    .id = 0x1a,
};
clk_node_t g_ckgen_gating_pll_lvds1_pclk = {
    .core = &g_img_gating_core,
    .id = 0x1c,
};
clk_node_t g_ckgen_gating_pll_lvds2_pclk = {
    .core = &g_img_gating_core,
    .id = 0x1d,
};
clk_node_t g_ckgen_gating_pll_lvds3_pclk = {
    .core = &g_img_gating_core,
    .id = 0x1e,
};
clk_node_t g_ckgen_gating_pll_lvds4_pclk = {
    .core = &g_img_gating_core,
    .id = 0x1f,
};
clk_node_t g_ckgen_gating_mipi_dsi1_pclk = {
    .core = &g_img_gating_core,
    .id = 0x20,
};
clk_node_t g_ckgen_gating_mipi_dsi2_pclk = {
    .core = &g_img_gating_core,
    .id = 0x21,
};
clk_node_t g_ckgen_gating_lvds_ss_pclk = {
    .core = &g_img_gating_core,
    .id = 0x22,
};
clk_node_t g_ckgen_gating_disp_mux_pclk = {
    .core = &g_img_gating_core,
    .id = 0x23,
};
clk_node_t g_ckgen_gating_isp_pix_clk = {
    .core = &g_img_gating_core,
    .id = 0x24,
};
clk_node_t g_ckgen_gating_lvds_ch0_clk_x7 = {
    .core = &g_img_gating_core,
    .id = 0x25,
};
clk_node_t g_ckgen_gating_mipi_dsi1_clkext = {
    .core = &g_img_gating_core,
    .id = 0x26,
};
clk_node_t g_ckgen_gating_mipi_dsi2_clkext = {
    .core = &g_img_gating_core,
    .id = 0x27,
};
clk_node_t g_ckgen_gating_csi4_mipi_ipi0_pclk = {
    .core = &g_img_gating_core,
    .id = 0x28,
};
clk_node_t g_ckgen_gating_csi4_mipi_ipi1_pclk = {
    .core = &g_img_gating_core,
    .id = 0x29,
};
clk_node_t g_ckgen_gating_csi4_mipi_ipi2_pclk = {
    .core = &g_img_gating_core,
    .id = 0x2a,
};
clk_node_t g_ckgen_gating_csi4_mipi_ipi3_pclk = {
    .core = &g_img_gating_core,
    .id = 0x2b,
};
clk_node_t g_ckgen_gating_lvds_clk_wrap1_pclk = {
    .core = &g_img_gating_core,
    .id = 0x2c,
};
clk_node_t g_ckgen_gating_lvds_clk_wrap2_pclk = {
    .core = &g_img_gating_core,
    .id = 0x2d,
};
clk_node_t g_ckgen_gating_lvds_clk_wrap3_pclk = {
    .core = &g_img_gating_core,
    .id = 0x2e,
};
clk_node_t g_ckgen_gating_lvds_clk_wrap4_pclk = {
    .core = &g_img_gating_core,
    .id = 0x2f,
};
clk_node_t g_ckgen_gating_xpu_cpu1a_clk_ckgen = {
    .core = &g_soc_gating_core,
    .id = 0x0,
};
clk_node_t g_ckgen_gating_xpu_cpu2a_clk_ckgen = {
    .core = &g_soc_gating_core,
    .id = 0x1,
};
clk_node_t g_ckgen_gating_adb_gpu1_memif_aclk = {
    .core = &g_soc_gating_core,
    .id = 0x2,
};
clk_node_t g_ckgen_gating_adb_gpu2_memif_aclk = {
    .core = &g_soc_gating_core,
    .id = 0x3,
};
clk_node_t g_ckgen_gating_xpu_ddr_clk_ckgen = {
    .core = &g_soc_gating_core,
    .id = 0x4,
};
clk_node_t g_ckgen_gating_xpu_vpu_bus_clk_ckgen = {
    .core = &g_soc_gating_core,
    .id = 0x5,
};
clk_node_t g_ckgen_gating_bti_vsn_m12_axiclk = {
    .core = &g_soc_gating_core,
    .id = 0x6,
};
clk_node_t g_ckgen_gating_xpu_his_clk_ckgen = {
    .core = &g_soc_gating_core,
    .id = 0x9,
};
clk_node_t g_ckgen_gating_xpu_vpu1_clk_ckgen = {
    .core = &g_soc_gating_core,
    .id = 0xa,
};
clk_node_t g_ckgen_gating_xpu_nna1_clk_ckgen = {
    .core = &g_soc_gating_core,
    .id = 0xb,
};
clk_node_t g_ckgen_gating_xpu_nna2_clk_ckgen = {
    .core = &g_soc_gating_core,
    .id = 0xc,
};
clk_node_t g_ckgen_gating_aaxi_imgs0_mainwm0 = {
    .core = &g_soc_gating_core,
    .id = 0xd,
};
clk_node_t g_ckgen_gating_aapb_hpi2disp = {
    .core = &g_soc_gating_core,
    .id = 0xe,
};
clk_node_t g_ckgen_gating_noc_mainw_sysclk = {
    .core = &g_soc_gating_core,
    .id = 0x10,
};
clk_node_t g_ckgen_gating_noc_mainw_pu600clk = {
    .core = &g_soc_gating_core,
    .id = 0x11,
};
clk_node_t g_ckgen_gating_noc_mainw_perclk = {
    .core = &g_soc_gating_core,
    .id = 0x12,
};
clk_node_t g_ckgen_gating_pll_hpiw_pclk = {
    .core = &g_soc_gating_core,
    .id = 0x13,
};
clk_node_t g_ckgen_gating_iram7_aclk = {
    .core = &g_soc_gating_core,
    .id = 0x14,
};
clk_node_t g_ckgen_gating_iram8_aclk = {
    .core = &g_soc_gating_core,
    .id = 0x15,
};
clk_node_t g_ckgen_gating_cssys_clk = {
    .core = &g_soc_gating_core,
    .id = 0x16,
};
clk_node_t g_ckgen_gating_scr4k_sid_pclk = {
    .core = &g_soc_gating_core,
    .id = 0x17,
};
clk_node_t g_ckgen_gating_scr4k_ssid_pclk = {
    .core = &g_soc_gating_core,
    .id = 0x18,
};
clk_node_t g_ckgen_gating_eic_hpi_pclk = {
    .core = &g_soc_gating_core,
    .id = 0x19,
};
clk_node_t g_ckgen_gating_scr_hpi_pclk = {
    .core = &g_soc_gating_core,
    .id = 0x1a,
};
clk_node_t g_ckgen_gating_ftbu0_pclk = {
    .core = &g_soc_gating_core,
    .id = 0x1b,
};
clk_node_t g_ckgen_gating_ftbu1_pclk = {
    .core = &g_soc_gating_core,
    .id = 0x1c,
};
clk_node_t g_ckgen_gating_ftbu0_aclk = {
    .core = &g_soc_gating_core,
    .id = 0x1d,
};
clk_node_t g_ckgen_gating_ftbu1_aclk = {
    .core = &g_soc_gating_core,
    .id = 0x1e,
};
clk_node_t g_ckgen_gating_tbu_16_aclk = {
    .core = &g_soc_gating_core,
    .id = 0x1f,
};
clk_node_t g_ckgen_gating_cpu1_coreclk0 = {
    .core = &g_soc_gating_core,
    .id = 0x20,
};
clk_node_t g_ckgen_gating_cpu1_coreclk1 = {
    .core = &g_soc_gating_core,
    .id = 0x21,
};
clk_node_t g_ckgen_gating_cpu1_coreclk2 = {
    .core = &g_soc_gating_core,
    .id = 0x22,
};
clk_node_t g_ckgen_gating_cpu1_coreclk3 = {
    .core = &g_soc_gating_core,
    .id = 0x23,
};
clk_node_t g_ckgen_gating_cpu1_coreclk4 = {
    .core = &g_soc_gating_core,
    .id = 0x24,
};
clk_node_t g_ckgen_gating_cpu1_coreclk5 = {
    .core = &g_soc_gating_core,
    .id = 0x25,
};
clk_node_t g_ckgen_gating_cpu1_coreclk6 = {
    .core = &g_soc_gating_core,
    .id = 0x26,
};
clk_node_t g_ckgen_gating_cpu1_coreclk7 = {
    .core = &g_soc_gating_core,
    .id = 0x27,
};
clk_node_t g_ckgen_gating_cpu1_periphclk = {
    .core = &g_soc_gating_core,
    .id = 0x28,
};
clk_node_t g_ckgen_gating_cpu1_sys_cnt_sync = {
    .core = &g_soc_gating_core,
    .id = 0x29,
};
clk_node_t g_ckgen_gating_cpu1_pclk = {
    .core = &g_soc_gating_core,
    .id = 0x2a,
};
clk_node_t g_ckgen_gating_cpu1_gicclk = {
    .core = &g_soc_gating_core,
    .id = 0x2b,
};
clk_node_t g_ckgen_gating_cpu1_atclk = {
    .core = &g_soc_gating_core,
    .id = 0x2c,
};
clk_node_t g_ckgen_gating_cpu1_sclk = {
    .core = &g_soc_gating_core,
    .id = 0x2d,
};
clk_node_t g_ckgen_gating_pll_cpu1a_pclk = {
    .core = &g_soc_gating_core,
    .id = 0x2e,
};
clk_node_t g_ckgen_gating_pll_cpu1b_pclk = {
    .core = &g_soc_gating_core,
    .id = 0x2f,
};
clk_node_t g_ckgen_gating_pll_cpu1c_pclk = {
    .core = &g_soc_gating_core,
    .id = 0x30,
};
clk_node_t g_ckgen_gating_tbu_17_aclk = {
    .core = &g_soc_gating_core,
    .id = 0x31,
};
clk_node_t g_ckgen_gating_cpu2_coreclk0 = {
    .core = &g_soc_gating_core,
    .id = 0x33,
};
clk_node_t g_ckgen_gating_cpu2_coreclk1 = {
    .core = &g_soc_gating_core,
    .id = 0x34,
};
clk_node_t g_ckgen_gating_cpu2_coreclk2 = {
    .core = &g_soc_gating_core,
    .id = 0x35,
};
clk_node_t g_ckgen_gating_cpu2_coreclk3 = {
    .core = &g_soc_gating_core,
    .id = 0x36,
};
clk_node_t g_ckgen_gating_cpu2_periphclk = {
    .core = &g_soc_gating_core,
    .id = 0x37,
};
clk_node_t g_ckgen_gating_cpu2_pclk = {
    .core = &g_soc_gating_core,
    .id = 0x38,
};
clk_node_t g_ckgen_gating_cpu2_gicclk = {
    .core = &g_soc_gating_core,
    .id = 0x39,
};
clk_node_t g_ckgen_gating_cpu2_atclk = {
    .core = &g_soc_gating_core,
    .id = 0x3a,
};
clk_node_t g_ckgen_gating_cpu2_sclk = {
    .core = &g_soc_gating_core,
    .id = 0x3b,
};
clk_node_t g_ckgen_gating_pll_cpu2a_pclk = {
    .core = &g_soc_gating_core,
    .id = 0x3c,
};
clk_node_t g_ckgen_gating_pll_cpu2b_pclk = {
    .core = &g_soc_gating_core,
    .id = 0x3d,
};
clk_node_t g_ckgen_gating_tbu_13_aclk = {
    .core = &g_soc_gating_core,
    .id = 0x3e,
};
clk_node_t g_ckgen_gating_tbu_14_aclk = {
    .core = &g_soc_gating_core,
    .id = 0x3f,
};
clk_node_t g_ckgen_gating_tbu9_aclk = {
    .core = &g_soc_gating_core,
    .id = 0x40,
};
clk_node_t g_ckgen_gating_tbu10_aclk = {
    .core = &g_soc_gating_core,
    .id = 0x41,
};
clk_node_t g_ckgen_gating_vpu1_bclk = {
    .core = &g_soc_gating_core,
    .id = 0x44,
};
clk_node_t g_ckgen_gating_vpu2_aclk = {
    .core = &g_soc_gating_core,
    .id = 0x45,
};
clk_node_t g_ckgen_gating_vpu3_aclk = {
    .core = &g_soc_gating_core,
    .id = 0x46,
};
clk_node_t g_ckgen_gating_vpu4_aclk = {
    .core = &g_soc_gating_core,
    .id = 0x47,
};
clk_node_t g_ckgen_gating_pll_vpu1_pclk = {
    .core = &g_soc_gating_core,
    .id = 0x48,
};
clk_node_t g_ckgen_gating_pll_vpu2_pclk = {
    .core = &g_soc_gating_core,
    .id = 0x49,
};
clk_node_t g_ckgen_gating_tbu11_aclk = {
    .core = &g_soc_gating_core,
    .id = 0x4a,
};
clk_node_t g_ckgen_gating_tbu12_aclk = {
    .core = &g_soc_gating_core,
    .id = 0x4b,
};
clk_node_t g_ckgen_gating_tbu15_aclk = {
    .core = &g_soc_gating_core,
    .id = 0x4c,
};
clk_node_t g_ckgen_gating_tbu8_aclk = {
    .core = &g_soc_gating_core,
    .id = 0x4d,
};
clk_node_t g_ckgen_gating_dciclk = {
    .core = &g_soc_gating_core,
    .id = 0x4e,
};
clk_node_t g_ckgen_gating_tbu3_aclk = {
    .core = &g_soc_gating_core,
    .id = 0x50,
};
clk_node_t g_ckgen_gating_pciex4_axiclk = {
    .core = &g_soc_gating_core,
    .id = 0x51,
};
clk_node_t g_ckgen_gating_pciex1a_axiaclk = {
    .core = &g_soc_gating_core,
    .id = 0x52,
};
clk_node_t g_ckgen_gating_pciex1b_axiclk = {
    .core = &g_soc_gating_core,
    .id = 0x53,
};
clk_node_t g_ckgen_gating_tbu7_aclk = {
    .core = &g_soc_gating_core,
    .id = 0x54,
};
clk_node_t g_ckgen_gating_usb1_axiclk = {
    .core = &g_soc_gating_core,
    .id = 0x55,
};
clk_node_t g_ckgen_gating_usb2_axiclk = {
    .core = &g_soc_gating_core,
    .id = 0x56,
};
clk_node_t g_ckgen_gating_usb3_axiclk = {
    .core = &g_soc_gating_core,
    .id = 0x57,
};
clk_node_t g_ckgen_gating_pll_his_pclk = {
    .core = &g_soc_gating_core,
    .id = 0x58,
};
clk_node_t g_ckgen_gating_pciephyx4_pclk = {
    .core = &g_soc_gating_core,
    .id = 0x59,
};
clk_node_t g_ckgen_gating_pciephyx2_pclk = {
    .core = &g_soc_gating_core,
    .id = 0x5a,
};
clk_node_t g_ckgen_gating_nna1_sysclk = {
    .core = &g_soc_gating_core,
    .id = 0x5b,
};
clk_node_t g_ckgen_gating_nna1_dbgclk = {
    .core = &g_soc_gating_core,
    .id = 0x5c,
};
clk_node_t g_ckgen_gating_pll_nna1_pclk = {
    .core = &g_soc_gating_core,
    .id = 0x5d,
};
clk_node_t g_ckgen_gating_tbu1_aclk = {
    .core = &g_soc_gating_core,
    .id = 0x5e,
};
clk_node_t g_ckgen_gating_tcu_aclk = {
    .core = &g_soc_gating_core,
    .id = 0x5f,
};
clk_node_t g_ckgen_gating_noc_maine_sysclk = {
    .core = &g_soc_gating_core,
    .id = 0x64,
};
clk_node_t g_ckgen_gating_noc_maine_perclk = {
    .core = &g_soc_gating_core,
    .id = 0x65,
};
clk_node_t g_ckgen_gating_noc_maine_pu800clk = {
    .core = &g_soc_gating_core,
    .id = 0x66,
};
clk_node_t g_ckgen_gating_iram5_aclk = {
    .core = &g_soc_gating_core,
    .id = 0x67,
};
clk_node_t g_ckgen_gating_iram6_aclk = {
    .core = &g_soc_gating_core,
    .id = 0x68,
};
clk_node_t g_ckgen_gating_caf_aclk = {
    .core = &g_soc_gating_core,
    .id = 0x69,
};
clk_node_t g_ckgen_gating_pll_hpie_pclk = {
    .core = &g_soc_gating_core,
    .id = 0x6a,
};
clk_node_t g_ckgen_gating_bti_noc_maine_cpu1 = {
    .core = &g_soc_gating_core,
    .id = 0x6e,
};
clk_node_t g_ckgen_gating_bti_noc_maine_cpu2 = {
    .core = &g_soc_gating_core,
    .id = 0x6f,
};
clk_node_t g_ckgen_gating_bti_noc_maine_nna1 = {
    .core = &g_soc_gating_core,
    .id = 0x70,
};
clk_node_t g_ckgen_gating_bti_noc_maine_nna2 = {
    .core = &g_soc_gating_core,
    .id = 0x71,
};
clk_node_t g_ckgen_gating_adb_ddr_clk = {
    .core = &g_soc_gating_core,
    .id = 0x72,
};
clk_node_t g_ckgen_gating_aapb_vpu_clk = {
    .core = &g_soc_gating_core,
    .id = 0x73,
};
clk_node_t g_ckgen_gating_nna2_ceva_clk = {
    .core = &g_soc_gating_core,
    .id = 0x75,
};
clk_node_t g_ckgen_gating_pll_nna2_pclk = {
    .core = &g_soc_gating_core,
    .id = 0x76,
};
clk_node_t g_ckgen_gating_iop_nna2_clk = {
    .core = &g_soc_gating_core,
    .id = 0x77,
};
clk_node_t g_ckgen_gating_nna2_etm_atb_clk = {
    .core = &g_soc_gating_core,
    .id = 0x78,
};
clk_node_t g_ckgen_gating_nna2_ceva_clk_timer = {
    .core = &g_soc_gating_core,
    .id = 0x79,
};

const clk_node_t *board_clk_prepare_array[] = {
};

const clk_node_t *board_clk_enable_array[] = {
};

const clk_node_t *board_clk_disable_array[] = {
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
