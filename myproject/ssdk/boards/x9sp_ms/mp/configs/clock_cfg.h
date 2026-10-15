/*
 * Copyright (c) 2021-2029 Semidrive Incorporated.
 * All rights reserved.
 * Software License Agreement.
 */
#ifndef CLOCK_CFG_H
#define CLOCK_CFG_H

#ifdef CONFIG_CLK

#include <driver/kunlun/clk/sc_pfpll_drv.h>
#include <driver/kunlun/clk/kunlun_ckgen_drv.h>

extern clk_node_t g_rc_24m;

extern clk_node_t g_rc_rtc;

extern clk_node_t g_xtal_24m;

extern clk_node_t g_xtal_rtc;

extern clk_node_t g_pll1_vco;

extern clk_node_t g_pll1_root;

extern clk_node_t g_pll1_div_a;

extern clk_node_t g_pll1_div_b;

extern clk_node_t g_pll1_div_c;

extern clk_node_t g_pll1_div_d;

extern clk_node_t g_pll2_vco;

extern clk_node_t g_pll2_root;

extern clk_node_t g_pll2_div_a;

extern clk_node_t g_pll2_div_b;

extern clk_node_t g_pll2_div_c;

extern clk_node_t g_pll2_div_d;

extern clk_node_t g_ext_aud3;

extern clk_node_t g_ext_aud4;

extern clk_node_t g_ckgen_bus_saf_plat_root;

extern clk_node_t g_ckgen_bus_saf_plat_div_m;

extern clk_node_t g_ckgen_bus_saf_plat_div_n;

extern clk_node_t g_ckgen_bus_saf_plat_div_p;

extern clk_node_t g_ckgen_bus_saf_plat_div_q;

extern clk_node_t g_ckgen_ip_xspi1a;

extern clk_node_t g_ckgen_ip_xspi1b;

extern clk_node_t g_ckgen_ip_reserved;

extern clk_node_t g_ckgen_ip_i2c_saf;

extern clk_node_t g_ckgen_ip_spi_saf;

extern clk_node_t g_ckgen_ip_uart_saf;

extern clk_node_t g_ckgen_ip_i2s_mclk1;

extern clk_node_t g_ckgen_ip_i2s_sc1;

extern clk_node_t g_ckgen_ip_i2s_sc2;

extern clk_node_t g_ckgen_ip_enet1_tx;

extern clk_node_t g_ckgen_ip_enet1_rmii;

extern clk_node_t g_ckgen_ip_enet1_phy_ref;

extern clk_node_t g_ckgen_ip_enet1_timer_sec;

extern clk_node_t g_ckgen_ip_ospi1;

extern clk_node_t g_ckgen_ip_timer1;

extern clk_node_t g_ckgen_ip_timer2;

extern clk_node_t g_ckgen_ip_pwm1;

extern clk_node_t g_ckgen_ip_pwm2;

extern clk_node_t g_ckgen_ip_can_1_to_4;

extern clk_node_t g_ckgen_ip_lin;

extern clk_node_t g_ckgen_ip_adc2;

extern clk_node_t g_ckgen_ip_adc3;

extern clk_node_t g_pll3_vco;

extern clk_node_t g_pll3_root;

extern clk_node_t g_pll3_div_a;

extern clk_node_t g_pll3_div_b;

extern clk_node_t g_pll3_div_c;

extern clk_node_t g_pll3_div_d;

extern clk_node_t g_pll4_vco;

extern clk_node_t g_pll4_root;

extern clk_node_t g_pll4_div_a;

extern clk_node_t g_pll4_div_b;

extern clk_node_t g_pll4_div_c;

extern clk_node_t g_pll4_div_d;

extern clk_node_t g_pll5_vco;

extern clk_node_t g_pll5_root;

extern clk_node_t g_pll5_div_a;

extern clk_node_t g_pll5_div_b;

extern clk_node_t g_pll5_div_c;

extern clk_node_t g_pll5_div_d;

extern clk_node_t g_pll6_vco;

extern clk_node_t g_pll6_root;

extern clk_node_t g_pll6_div_a;

extern clk_node_t g_pll6_div_b;

extern clk_node_t g_pll6_div_c;

extern clk_node_t g_pll6_div_d;

extern clk_node_t g_pll7_vco;

extern clk_node_t g_pll7_root;

extern clk_node_t g_pll7_div_a;

extern clk_node_t g_pll7_div_b;

extern clk_node_t g_pll7_div_c;

extern clk_node_t g_pll7_div_d;

extern clk_node_t g_ext_aud1;

extern clk_node_t g_ext_aud2;

extern clk_node_t g_ckgen_bus_sec_plat_root;

extern clk_node_t g_ckgen_bus_sec_plat_div_m;

extern clk_node_t g_ckgen_bus_sec_plat_div_n;

extern clk_node_t g_ckgen_bus_sec_plat_div_p;

extern clk_node_t g_ckgen_bus_sec_plat_div_q;

extern clk_node_t g_ckgen_core_mp_plat;

extern clk_node_t g_ckgen_bus_spare_0_root;

extern clk_node_t g_ckgen_bus_spare_0_div_m;

extern clk_node_t g_ckgen_bus_spare_0_div_n;

extern clk_node_t g_ckgen_bus_spare_0_div_p;

extern clk_node_t g_ckgen_bus_spare_0_div_q;

extern clk_node_t g_ckgen_bus_spare_1_root;

extern clk_node_t g_ckgen_bus_spare_1_div_m;

extern clk_node_t g_ckgen_bus_spare_1_div_n;

extern clk_node_t g_ckgen_bus_spare_1_div_p;

extern clk_node_t g_ckgen_bus_spare_1_div_q;

extern clk_node_t g_ckgen_ip_adc1;

extern clk_node_t g_ckgen_ip_spare_2;

extern clk_node_t g_ckgen_ip_i2c_sec0;

extern clk_node_t g_ckgen_ip_i2c_sec1;

extern clk_node_t g_ckgen_ip_spi_sec0;

extern clk_node_t g_ckgen_ip_spi_sec1;

extern clk_node_t g_ckgen_ip_uart_sec0;

extern clk_node_t g_ckgen_ip_uart_sec1;

extern clk_node_t g_ckgen_ip_emmc1;

extern clk_node_t g_ckgen_ip_emmc2;

extern clk_node_t g_ckgen_ip_emmc3;

extern clk_node_t g_ckgen_ip_emmc4;

extern clk_node_t g_ckgen_ip_enet2_tx;

extern clk_node_t g_ckgen_ip_enet2_rmii;

extern clk_node_t g_ckgen_ip_enet2_phy_ref;

extern clk_node_t g_ckgen_ip_enet2_timer_sec;

extern clk_node_t g_ckgen_ip_spdif1;

extern clk_node_t g_ckgen_ip_spdif2;

extern clk_node_t g_ckgen_ip_spdif3;

extern clk_node_t g_ckgen_ip_spdif4;

extern clk_node_t g_ckgen_ip_ospi2;

extern clk_node_t g_ckgen_ip_timer3;

extern clk_node_t g_ckgen_ip_timer4;

extern clk_node_t g_ckgen_ip_timer5;

extern clk_node_t g_ckgen_ip_timer6;

extern clk_node_t g_ckgen_ip_timer7;

extern clk_node_t g_ckgen_ip_timer8;

extern clk_node_t g_ckgen_ip_pwm3;

extern clk_node_t g_ckgen_ip_pwm4;

extern clk_node_t g_ckgen_ip_pwm5;

extern clk_node_t g_ckgen_ip_pwm6;

extern clk_node_t g_ckgen_ip_pwm7;

extern clk_node_t g_ckgen_ip_pwm8;

extern clk_node_t g_ckgen_ip_i2s_mclk2;

extern clk_node_t g_ckgen_ip_i2s_mclk3;

extern clk_node_t g_ckgen_ip_i2s_mc1;

extern clk_node_t g_ckgen_ip_i2s_mc2;

extern clk_node_t g_ckgen_ip_i2s_sc3;

extern clk_node_t g_ckgen_ip_i2s_sc4;

extern clk_node_t g_ckgen_ip_i2s_sc5;

extern clk_node_t g_ckgen_ip_i2s_sc6;

extern clk_node_t g_ckgen_ip_i2s_sc7;

extern clk_node_t g_ckgen_ip_i2s_sc8;

extern clk_node_t g_ckgen_ip_csi_mclk1;

extern clk_node_t g_ckgen_ip_csi_mclk2;

extern clk_node_t g_ckgen_ip_gic_4_5;

extern clk_node_t g_ckgen_ip_can_5_to_20;

extern clk_node_t g_ckgen_ip_trace;

extern clk_node_t g_ckgen_ip_sys_cnt;

extern clk_node_t g_ckgen_ip_mshc_timer;

extern clk_node_t g_ckgen_ip_hpi_clk600;

extern clk_node_t g_ckgen_ip_hpi_clk800;

extern clk_node_t g_pll_disp1_vco;

extern clk_node_t g_pll_disp1_root;

extern clk_node_t g_pll_disp1_div_a;

extern clk_node_t g_pll_disp1_div_b;

extern clk_node_t g_pll_disp2_vco;

extern clk_node_t g_pll_disp2_root;

extern clk_node_t g_pll_disp2_div_a;

extern clk_node_t g_pll_disp2_div_b;

extern clk_node_t g_pll_lvds1_vco;

extern clk_node_t g_pll_lvds1_root;

extern clk_node_t g_pll_lvds1_div_a;

extern clk_node_t g_pll_lvds1_div_b;

extern clk_node_t g_pll_lvds2_vco;

extern clk_node_t g_pll_lvds2_root;

extern clk_node_t g_pll_lvds2_div_a;

extern clk_node_t g_pll_lvds2_div_b;

extern clk_node_t g_pll_lvds3_vco;

extern clk_node_t g_pll_lvds3_root;

extern clk_node_t g_pll_lvds3_div_a;

extern clk_node_t g_pll_lvds3_div_b;

extern clk_node_t g_pll_lvds4_vco;

extern clk_node_t g_pll_lvds4_root;

extern clk_node_t g_pll_lvds4_div_a;

extern clk_node_t g_pll_lvds4_div_b;

extern clk_node_t g_ckgen_ip_mipi_csi1_pix;

extern clk_node_t g_ckgen_ip_mipi_csi2_pix;

extern clk_node_t g_ckgen_ip_mipi_csi3_pix;

extern clk_node_t g_ckgen_ip_csi4;

extern clk_node_t g_ckgen_ip_dp1;

extern clk_node_t g_ckgen_ip_dp2;

extern clk_node_t g_ckgen_ip_dp3;

extern clk_node_t g_ckgen_ip_dc1;

extern clk_node_t g_ckgen_ip_dc2;

extern clk_node_t g_ckgen_ip_dc3;

extern clk_node_t g_ckgen_ip_dc4;

extern clk_node_t g_ckgen_ip_dc5;

extern clk_node_t g_ckgen_ip_isp;

extern clk_node_t g_ckgen_ip_spare1;

extern clk_node_t g_ckgen_ip_dc_alt_dsp_clk0;

extern clk_node_t g_ckgen_ip_dc_alt_dsp_clk1;

extern clk_node_t g_ckgen_bus_disp_bus_root;

extern clk_node_t g_ckgen_bus_disp_bus_div_m;

extern clk_node_t g_ckgen_bus_disp_bus_div_n;

extern clk_node_t g_ckgen_bus_disp_bus_div_p;

extern clk_node_t g_ckgen_bus_disp_bus_div_q;

extern clk_node_t g_ckgen_ip_ext_aud1;

extern clk_node_t g_ckgen_ip_ext_aud2;

extern clk_node_t g_ckgen_ip_ext_aud3;

extern clk_node_t g_ckgen_ip_ext_aud4;

extern clk_node_t g_ckgen_core_cpu1a_pre;

extern clk_node_t g_ckgen_uuu_cpu1a_div_m_pre;

extern clk_node_t g_pll_cpu1a_vco;

extern clk_node_t g_pll_cpu1a_root;

extern clk_node_t g_pll_cpu1a_div_a;

extern clk_node_t g_pll_cpu1a_div_b;

extern clk_node_t g_ckgen_core_cpu1b_pre;

extern clk_node_t g_ckgen_uuu_cpu1b_div_m_pre;

extern clk_node_t g_pll_cpu1b_vco;

extern clk_node_t g_pll_cpu1b_root;

extern clk_node_t g_pll_cpu1b_div_a;

extern clk_node_t g_pll_cpu1b_div_b;

extern clk_node_t g_pll_cpu1c_vco;

extern clk_node_t g_pll_cpu1c_root;

extern clk_node_t g_pll_cpu1c_div_a;

extern clk_node_t g_pll_cpu1c_div_b;

extern clk_node_t g_pll_cpu2a_vco;

extern clk_node_t g_pll_cpu2a_root;

extern clk_node_t g_pll_cpu2a_div_a;

extern clk_node_t g_pll_cpu2a_div_b;

extern clk_node_t g_pll_cpu2b_vco;

extern clk_node_t g_pll_cpu2b_root;

extern clk_node_t g_pll_cpu2b_div_a;

extern clk_node_t g_pll_cpu2b_div_b;

extern clk_node_t g_ckgen_core_gpu1_pre;

extern clk_node_t g_ckgen_uuu_gpu1_div_m_pre;

extern clk_node_t g_ckgen_uuu_gpu1_div_n_pre;

extern clk_node_t g_ckgen_uuu_gpu1_div_p_pre;

extern clk_node_t g_pll_gpu1_vco;

extern clk_node_t g_pll_gpu1_root;

extern clk_node_t g_pll_gpu1_div_a;

extern clk_node_t g_pll_gpu1_div_b;

extern clk_node_t g_ckgen_core_gpu2_pre;

extern clk_node_t g_ckgen_uuu_gpu2_div_m_pre;

extern clk_node_t g_ckgen_uuu_gpu2_div_n_pre;

extern clk_node_t g_ckgen_uuu_gpu2_div_p_pre;

extern clk_node_t g_pll_gpu2_vco;

extern clk_node_t g_pll_gpu2_root;

extern clk_node_t g_pll_gpu2_div_a;

extern clk_node_t g_pll_gpu2_div_b;

extern clk_node_t g_ckgen_ip_vpu1_pre;

extern clk_node_t g_ckgen_uuu_vpu1_div_m_pre;

extern clk_node_t g_pll_vpu1_vco;

extern clk_node_t g_pll_vpu1_root;

extern clk_node_t g_pll_vpu1_div_a;

extern clk_node_t g_pll_vpu1_div_b;

extern clk_node_t g_pll_vpu2_vco;

extern clk_node_t g_pll_vpu2_root;

extern clk_node_t g_pll_vpu2_div_a;

extern clk_node_t g_pll_vpu2_div_b;

extern clk_node_t g_pll_vsn_vco;

extern clk_node_t g_pll_vsn_root;

extern clk_node_t g_pll_vsn_div_a;

extern clk_node_t g_pll_vsn_div_b;

extern clk_node_t g_pll_hpie_vco;

extern clk_node_t g_pll_hpie_root;

extern clk_node_t g_pll_hpie_div_a;

extern clk_node_t g_pll_hpie_div_b;

extern clk_node_t g_pll_hpiw_vco;

extern clk_node_t g_pll_hpiw_root;

extern clk_node_t g_pll_hpiw_div_a;

extern clk_node_t g_pll_hpiw_div_b;

extern clk_node_t g_pll_his_vco;

extern clk_node_t g_pll_his_root;

extern clk_node_t g_pll_his_div_a;

extern clk_node_t g_pll_his_div_b;

extern clk_node_t g_pll_ddr_vco;

extern clk_node_t g_pll_ddr_root;

extern clk_node_t g_pll_ddr_div_a;

extern clk_node_t g_pll_ddr_div_b;

extern clk_node_t g_pll_vsn2_vco;

extern clk_node_t g_pll_vsn2_root;

extern clk_node_t g_pll_vsn2_div_a;

extern clk_node_t g_pll_vsn2_div_b;

extern clk_node_t g_ckgen_core_cpu1a;

extern clk_node_t g_ckgen_core_cpu1b;

extern clk_node_t g_ckgen_core_cpu1c;

extern clk_node_t g_ckgen_core_cpu2a;

extern clk_node_t g_ckgen_core_cpu2b;

extern clk_node_t g_ckgen_core_gpu1;

extern clk_node_t g_ckgen_core_gpu2;

extern clk_node_t g_ckgen_ip_vpu1;

extern clk_node_t g_ckgen_ip_vpu2;

extern clk_node_t g_ckgen_ip_vpu3;

extern clk_node_t g_ckgen_ip_vpu4;

extern clk_node_t g_ckgen_bus_vpu_bus_root;

extern clk_node_t g_ckgen_bus_vpu_bus_div_m;

extern clk_node_t g_ckgen_bus_vpu_bus_div_n;

extern clk_node_t g_ckgen_bus_vpu_bus_div_p;

extern clk_node_t g_ckgen_bus_vpu_bus_div_q;

extern clk_node_t g_ckgen_bus_vsn_bus_root;

extern clk_node_t g_ckgen_bus_vsn_bus_div_m;

extern clk_node_t g_ckgen_bus_vsn_bus_div_n;

extern clk_node_t g_ckgen_bus_vsn_bus_div_p;

extern clk_node_t g_ckgen_bus_vsn_bus_div_q;

extern clk_node_t g_ckgen_ip_vsn2_bus;

extern clk_node_t g_ckgen_core_ddr;

extern clk_node_t g_ckgen_bus_his_bus_root;

extern clk_node_t g_ckgen_bus_his_bus_div_m;

extern clk_node_t g_ckgen_bus_his_bus_div_n;

extern clk_node_t g_ckgen_bus_his_bus_div_p;

extern clk_node_t g_ckgen_bus_his_bus_div_q;

extern clk_node_t g_ckgen_bus_noce_main_root;

extern clk_node_t g_ckgen_bus_noce_main_div_m;

extern clk_node_t g_ckgen_bus_noce_main_div_n;

extern clk_node_t g_ckgen_bus_noce_main_div_p;

extern clk_node_t g_ckgen_bus_noce_main_div_q;

extern clk_node_t g_ckgen_bus_noce_mainp_root;

extern clk_node_t g_ckgen_bus_noce_mainp_div_m;

extern clk_node_t g_ckgen_bus_noce_mainp_div_n;

extern clk_node_t g_ckgen_bus_noce_mainp_div_p;

extern clk_node_t g_ckgen_bus_noce_mainp_div_q;

extern clk_node_t g_ckgen_bus_nocw_main_root;

extern clk_node_t g_ckgen_bus_nocw_main_div_m;

extern clk_node_t g_ckgen_bus_nocw_main_div_n;

extern clk_node_t g_ckgen_bus_nocw_main_div_p;

extern clk_node_t g_ckgen_bus_nocw_main_div_q;

extern clk_node_t g_ckgen_bus_nocw_mainp_root;

extern clk_node_t g_ckgen_bus_nocw_mainp_div_m;

extern clk_node_t g_ckgen_bus_nocw_mainp_div_n;

extern clk_node_t g_ckgen_bus_nocw_mainp_div_p;

extern clk_node_t g_ckgen_bus_nocw_mainp_div_q;

extern clk_node_t g_ckgen_ip_dci;

extern clk_node_t g_ckgen_uuu_cpu1a_div_m;

extern clk_node_t g_ckgen_uuu_cpu1b_div_m;

extern clk_node_t g_ckgen_uuu_cpu1c_div_m;

extern clk_node_t g_ckgen_uuu_cpu1c_div_n;

extern clk_node_t g_ckgen_uuu_cpu1c_div_p;

extern clk_node_t g_ckgen_uuu_cpu2a_div_m;

extern clk_node_t g_ckgen_uuu_cpu2b_div_m;

extern clk_node_t g_ckgen_uuu_cpu2b_div_n;

extern clk_node_t g_ckgen_uuu_cpu2b_div_p;

extern clk_node_t g_ckgen_uuu_gpu1_div_m;

extern clk_node_t g_ckgen_uuu_gpu1_div_n;

extern clk_node_t g_ckgen_uuu_gpu1_div_p;

extern clk_node_t g_ckgen_uuu_gpu2_div_m;

extern clk_node_t g_ckgen_uuu_gpu2_div_n;

extern clk_node_t g_ckgen_uuu_gpu2_div_p;

extern clk_node_t g_ckgen_uuu_vpu1_div_m;

extern clk_node_t g_ckgen_uuu_vpu2_div_m;

extern clk_node_t g_ckgen_uuu_vpu3_div_m;

extern clk_node_t g_ckgen_uuu_vpu4_div_m;

extern clk_node_t g_ckgen_uuu_vpu_bus_div_m;

extern clk_node_t g_ckgen_uuu_vpu_bus_div_n;

extern clk_node_t g_ckgen_uuu_vsn_bus_div_m;

extern clk_node_t g_ckgen_uuu_vsn_bus_div_n;

extern clk_node_t g_ckgen_uuu_vsn2_bus_div_m;

extern clk_node_t g_ckgen_uuu_vsn2_bus_div_n;

extern clk_node_t g_ckgen_uuu_ddr_div_m;

extern clk_node_t g_ckgen_uuu_ddr_div_n;

extern clk_node_t g_ckgen_uuu_ddr_div_p;

extern clk_node_t g_ckgen_uuu_ddr_div_q;

extern clk_node_t g_ckgen_uuu_his_bus_div_m;

extern clk_node_t g_ckgen_uuu_his_bus_div_n;

extern clk_node_t g_ckgen_uuu_his_bus_div_p;

extern clk_node_t g_ckgen_uuu_his_bus_div_q;

extern clk_node_t g_ckgen_uuu_noce_main_div_m;

extern clk_node_t g_ckgen_uuu_noce_main_div_n;

extern clk_node_t g_ckgen_uuu_noce_mainp_div_m;

extern clk_node_t g_ckgen_uuu_noce_mainp_div_n;

extern clk_node_t g_ckgen_gating_cr5_safe;

extern clk_node_t g_ckgen_gating_noc_safe_main;

extern clk_node_t g_ckgen_gating_noc_mainb_bus;

extern clk_node_t g_ckgen_gating_gic1;

extern clk_node_t g_ckgen_gating_dma1;

extern clk_node_t g_ckgen_gating_dma9;

extern clk_node_t g_ckgen_gating_enet1;

extern clk_node_t g_ckgen_gating_ospi1;

extern clk_node_t g_ckgen_gating_rdc_safe;

extern clk_node_t g_ckgen_gating_iram1;

extern clk_node_t g_ckgen_gating_romc1;

extern clk_node_t g_ckgen_gating_mmu_switch_safe;

extern clk_node_t g_ckgen_gating_tbu20;

extern clk_node_t g_ckgen_gating_tbu21;

extern clk_node_t g_ckgen_gating_tbu22;

extern clk_node_t g_ckgen_gating_tbu23;

extern clk_node_t g_ckgen_gating_tbu24;

extern clk_node_t g_ckgen_gating_ce1;

extern clk_node_t g_ckgen_gating_xspi_slv;

extern clk_node_t g_ckgen_gating_noc_safe;

extern clk_node_t g_ckgen_gating_noc_mainb_busdiv;

extern clk_node_t g_ckgen_gating_i2c1;

extern clk_node_t g_ckgen_gating_i2c2;

extern clk_node_t g_ckgen_gating_i2c3;

extern clk_node_t g_ckgen_gating_i2c4;

extern clk_node_t g_ckgen_gating_i2c17;

extern clk_node_t g_ckgen_gating_i2c18;

extern clk_node_t g_ckgen_gating_i2c19;

extern clk_node_t g_ckgen_gating_i2c20;

extern clk_node_t g_ckgen_gating_spi1;

extern clk_node_t g_ckgen_gating_spi2;

extern clk_node_t g_ckgen_gating_spi3;

extern clk_node_t g_ckgen_gating_spi4;

extern clk_node_t g_ckgen_gating_spi9;

extern clk_node_t g_ckgen_gating_spi10;

extern clk_node_t g_ckgen_gating_spi11;

extern clk_node_t g_ckgen_gating_spi12;

extern clk_node_t g_ckgen_gating_uart1;

extern clk_node_t g_ckgen_gating_uart2;

extern clk_node_t g_ckgen_gating_uart3;

extern clk_node_t g_ckgen_gating_uart4;

extern clk_node_t g_ckgen_gating_uart5;

extern clk_node_t g_ckgen_gating_uart6;

extern clk_node_t g_ckgen_gating_uart7;

extern clk_node_t g_ckgen_gating_uart8;

extern clk_node_t g_ckgen_gating_i2s_sc1;

extern clk_node_t g_ckgen_gating_i2s_sc2;

extern clk_node_t g_ckgen_gating_wdt1;

extern clk_node_t g_ckgen_gating_wdt2;

extern clk_node_t g_ckgen_gating_gpio1;

extern clk_node_t g_ckgen_gating_scr_safe;

extern clk_node_t g_ckgen_gating_pvt_sns_safe;

extern clk_node_t g_ckgen_gating_pll1;

extern clk_node_t g_ckgen_gating_pll2;

extern clk_node_t g_ckgen_gating_iomux_ctrl_safe;

extern clk_node_t g_ckgen_gating_rpc_safe;

extern clk_node_t g_ckgen_gating_xtal_safe;

extern clk_node_t g_ckgen_gating_efuse;

extern clk_node_t g_ckgen_gating_timer1;

extern clk_node_t g_ckgen_gating_pwm1;

extern clk_node_t g_ckgen_gating_pwm2;

extern clk_node_t g_ckgen_gating_canfd1;

extern clk_node_t g_ckgen_gating_canfd2;

extern clk_node_t g_ckgen_gating_canfd3;

extern clk_node_t g_ckgen_gating_canfd4;

extern clk_node_t g_ckgen_gating_cr5_safe_atclk;

extern clk_node_t g_ckgen_gating_cr5_safe_pclk_dbg;

extern clk_node_t g_ckgen_gating_sem1;

extern clk_node_t g_ckgen_gating_sem2;

extern clk_node_t g_ckgen_gating_rc_24m;

extern clk_node_t g_ckgen_gating_eic_safe;

extern clk_node_t g_ckgen_gating_lin1;

extern clk_node_t g_ckgen_gating_lin2;

extern clk_node_t g_ckgen_gating_lin3;

extern clk_node_t g_ckgen_gating_lin4;

extern clk_node_t g_ckgen_gating_lin5;

extern clk_node_t g_ckgen_gating_lin6;

extern clk_node_t g_ckgen_gating_lin7;

extern clk_node_t g_ckgen_gating_lin8;

extern clk_node_t g_ckgen_gating_adc3;

extern clk_node_t g_ckgen_gating_adc2;

extern clk_node_t g_ckgen_gating_noc_tp;

extern clk_node_t g_ckgen_gating_axislvmux_ce1;

extern clk_node_t g_ckgen_gating_xspi1a_apb_sync;

extern clk_node_t g_ckgen_gating_xspi1b_apb_sync;

extern clk_node_t g_ckgen_gating_timer2;

extern clk_node_t g_ckgen_gating_xspi1_split;

extern clk_node_t g_ckgen_gating_rtc;

extern clk_node_t g_ckgen_gating_cr5_mp;

extern clk_node_t g_ckgen_gating_cr5_sec;

extern clk_node_t g_ckgen_gating_noc_sec_main;

extern clk_node_t g_ckgen_gating_noc_sec_m_main;

extern clk_node_t g_ckgen_gating_gic2;

extern clk_node_t g_ckgen_gating_gic3;

extern clk_node_t g_ckgen_gating_mshc1;

extern clk_node_t g_ckgen_gating_mshc2;

extern clk_node_t g_ckgen_gating_mshc3;

extern clk_node_t g_ckgen_gating_mshc4;

extern clk_node_t g_ckgen_gating_enet2;

extern clk_node_t g_ckgen_gating_ospi2;

extern clk_node_t g_ckgen_gating_dma2;

extern clk_node_t g_ckgen_gating_dma3;

extern clk_node_t g_ckgen_gating_dma4;

extern clk_node_t g_ckgen_gating_dma5;

extern clk_node_t g_ckgen_gating_dma6;

extern clk_node_t g_ckgen_gating_dma7;

extern clk_node_t g_ckgen_gating_dma8;

extern clk_node_t g_ckgen_gating_axi2ahb_mb;

extern clk_node_t g_ckgen_gating_mb;

extern clk_node_t g_ckgen_gating_iram2;

extern clk_node_t g_ckgen_gating_iram3;

extern clk_node_t g_ckgen_gating_iram4;

extern clk_node_t g_ckgen_gating_romc2;

extern clk_node_t g_ckgen_gating_tbu18;

extern clk_node_t g_ckgen_gating_tbu19;

extern clk_node_t g_ckgen_gating_ce2;

extern clk_node_t g_ckgen_gating_adti_whpi2sec_mst;

extern clk_node_t g_ckgen_gating_mmu_switch_sec;

extern clk_node_t g_ckgen_gating_noc_sec_per;

extern clk_node_t g_ckgen_gating_noc_sec_m_per;

extern clk_node_t g_ckgen_gating_i2c5;

extern clk_node_t g_ckgen_gating_i2c6;

extern clk_node_t g_ckgen_gating_i2c7;

extern clk_node_t g_ckgen_gating_i2c8;

extern clk_node_t g_ckgen_gating_i2c9;

extern clk_node_t g_ckgen_gating_i2c10;

extern clk_node_t g_ckgen_gating_i2c11;

extern clk_node_t g_ckgen_gating_i2c12;

extern clk_node_t g_ckgen_gating_i2c13;

extern clk_node_t g_ckgen_gating_i2c14;

extern clk_node_t g_ckgen_gating_i2c15;

extern clk_node_t g_ckgen_gating_i2c16;

extern clk_node_t g_ckgen_gating_uart9;

extern clk_node_t g_ckgen_gating_uart10;

extern clk_node_t g_ckgen_gating_uart11;

extern clk_node_t g_ckgen_gating_uart12;

extern clk_node_t g_ckgen_gating_uart13;

extern clk_node_t g_ckgen_gating_uart14;

extern clk_node_t g_ckgen_gating_uart15;

extern clk_node_t g_ckgen_gating_uart16;

extern clk_node_t g_ckgen_gating_timer3;

extern clk_node_t g_ckgen_gating_timer4;

extern clk_node_t g_ckgen_gating_timer5;

extern clk_node_t g_ckgen_gating_timer6;

extern clk_node_t g_ckgen_gating_timer7;

extern clk_node_t g_ckgen_gating_timer8;

extern clk_node_t g_ckgen_gating_pwm3;

extern clk_node_t g_ckgen_gating_pwm4;

extern clk_node_t g_ckgen_gating_pwm5;

extern clk_node_t g_ckgen_gating_pwm6;

extern clk_node_t g_ckgen_gating_pwm7;

extern clk_node_t g_ckgen_gating_pwm8;

extern clk_node_t g_ckgen_gating_i2s_mc1;

extern clk_node_t g_ckgen_gating_i2s_mc2;

extern clk_node_t g_ckgen_gating_i2s_sc3;

extern clk_node_t g_ckgen_gating_i2s_sc4;

extern clk_node_t g_ckgen_gating_i2s_sc5;

extern clk_node_t g_ckgen_gating_i2s_sc6;

extern clk_node_t g_ckgen_gating_i2s_sc7;

extern clk_node_t g_ckgen_gating_i2s_sc8;

extern clk_node_t g_ckgen_gating_gpio2;

extern clk_node_t g_ckgen_gating_gpio3;

extern clk_node_t g_ckgen_gating_gpio4;

extern clk_node_t g_ckgen_gating_gpio5;

extern clk_node_t g_ckgen_gating_adc1;

extern clk_node_t g_ckgen_gating_wdt3;

extern clk_node_t g_ckgen_gating_wdt4;

extern clk_node_t g_ckgen_gating_wdt5;

extern clk_node_t g_ckgen_gating_wdt6;

extern clk_node_t g_ckgen_gating_wdt7;

extern clk_node_t g_ckgen_gating_wdt8;

extern clk_node_t g_ckgen_gating_pll3;

extern clk_node_t g_ckgen_gating_pll4;

extern clk_node_t g_ckgen_gating_pll5;

extern clk_node_t g_ckgen_gating_pll6;

extern clk_node_t g_ckgen_gating_pll7;

extern clk_node_t g_ckgen_gating_iomux_ctrl_ap;

extern clk_node_t g_ckgen_gating_scr_sec;

extern clk_node_t g_ckgen_gating_pvt_sns_sec;

extern clk_node_t g_ckgen_gating_mac;

extern clk_node_t g_ckgen_gating_rpc_sec;

extern clk_node_t g_ckgen_gating_spdif1;

extern clk_node_t g_ckgen_gating_spdif2;

extern clk_node_t g_ckgen_gating_spdif3;

extern clk_node_t g_ckgen_gating_spdif4;

extern clk_node_t g_ckgen_gating_spi5;

extern clk_node_t g_ckgen_gating_spi6;

extern clk_node_t g_ckgen_gating_spi7;

extern clk_node_t g_ckgen_gating_spi8;

extern clk_node_t g_ckgen_gating_canfd5;

extern clk_node_t g_ckgen_gating_canfd6;

extern clk_node_t g_ckgen_gating_canfd7;

extern clk_node_t g_ckgen_gating_canfd8;

extern clk_node_t g_ckgen_gating_canfd9;

extern clk_node_t g_ckgen_gating_canfd10;

extern clk_node_t g_ckgen_gating_canfd11;

extern clk_node_t g_ckgen_gating_canfd12;

extern clk_node_t g_ckgen_gating_canfd13;

extern clk_node_t g_ckgen_gating_canfd14;

extern clk_node_t g_ckgen_gating_canfd15;

extern clk_node_t g_ckgen_gating_canfd16;

extern clk_node_t g_ckgen_gating_canfd17;

extern clk_node_t g_ckgen_gating_canfd18;

extern clk_node_t g_ckgen_gating_canfd19;

extern clk_node_t g_ckgen_gating_canfd20;

extern clk_node_t g_ckgen_gating_eic_sec;

extern clk_node_t g_ckgen_gating_gic4;

extern clk_node_t g_ckgen_gating_gic5;

extern clk_node_t g_ckgen_gating_cssys_trace;

extern clk_node_t g_ckgen_gating_sys_cnt;

extern clk_node_t g_ckgen_gating_apb_mac_disp_slv_;

extern clk_node_t g_ckgen_gating_apb_mac_his_slv_;

extern clk_node_t g_ckgen_gating_aaxi_dispa_img_clk;

extern clk_node_t g_ckgen_gating_xtal_ap_pclk;

extern clk_node_t g_ckgen_gating_pll_disp1_pclk;

extern clk_node_t g_ckgen_gating_pll_disp2_pclk;

extern clk_node_t g_ckgen_gating_csi1_axiclk;

extern clk_node_t g_ckgen_gating_csi2_axiclk;

extern clk_node_t g_ckgen_gating_csi3_axiclk;

extern clk_node_t g_ckgen_gating_csi4_axiclk;

extern clk_node_t g_ckgen_gating_imux_pclk;

extern clk_node_t g_ckgen_gating_mipi_csi1_pclk;

extern clk_node_t g_ckgen_gating_mipi_csi2_pclk;

extern clk_node_t g_ckgen_gating_mipi_csi3_pclk;

extern clk_node_t g_ckgen_gating_g2d1_axiclk;

extern clk_node_t g_ckgen_gating_g2d2_axiclk;

extern clk_node_t g_ckgen_gating_dc1_axiclk;

extern clk_node_t g_ckgen_gating_dc2_axiclk;

extern clk_node_t g_ckgen_gating_dc3_axiclk;

extern clk_node_t g_ckgen_gating_dc4_axiclk;

extern clk_node_t g_ckgen_gating_dc5_axiclk;

extern clk_node_t g_ckgen_gating_dp1_axiclk;

extern clk_node_t g_ckgen_gating_dp2_axiclk;

extern clk_node_t g_ckgen_gating_dp3_axiclk;

extern clk_node_t g_ckgen_gating_iram_disp_axiclk;

extern clk_node_t g_ckgen_gating_pll_lvds1_pclk;

extern clk_node_t g_ckgen_gating_pll_lvds2_pclk;

extern clk_node_t g_ckgen_gating_pll_lvds3_pclk;

extern clk_node_t g_ckgen_gating_pll_lvds4_pclk;

extern clk_node_t g_ckgen_gating_mipi_dsi1_pclk;

extern clk_node_t g_ckgen_gating_mipi_dsi2_pclk;

extern clk_node_t g_ckgen_gating_lvds_ss_pclk;

extern clk_node_t g_ckgen_gating_disp_mux_pclk;

extern clk_node_t g_ckgen_gating_isp_pix_clk;

extern clk_node_t g_ckgen_gating_lvds_ch0_clk_x7;

extern clk_node_t g_ckgen_gating_mipi_dsi1_clkext;

extern clk_node_t g_ckgen_gating_mipi_dsi2_clkext;

extern clk_node_t g_ckgen_gating_csi4_mipi_ipi0_pclk;

extern clk_node_t g_ckgen_gating_csi4_mipi_ipi1_pclk;

extern clk_node_t g_ckgen_gating_csi4_mipi_ipi2_pclk;

extern clk_node_t g_ckgen_gating_csi4_mipi_ipi3_pclk;

extern clk_node_t g_ckgen_gating_lvds_clk_wrap1_pclk;

extern clk_node_t g_ckgen_gating_lvds_clk_wrap2_pclk;

extern clk_node_t g_ckgen_gating_lvds_clk_wrap3_pclk;

extern clk_node_t g_ckgen_gating_lvds_clk_wrap4_pclk;

extern clk_node_t g_ckgen_gating_xpu_cpu1a_clk_ckgen;

extern clk_node_t g_ckgen_gating_xpu_cpu2a_clk_ckgen;

extern clk_node_t g_ckgen_gating_adb_gpu1_memif_aclk;

extern clk_node_t g_ckgen_gating_adb_gpu2_memif_aclk;

extern clk_node_t g_ckgen_gating_xpu_ddr_clk_ckgen;

extern clk_node_t g_ckgen_gating_xpu_vpu_bus_clk_ckgen;

extern clk_node_t g_ckgen_gating_bti_vsn_m12_axiclk;

extern clk_node_t g_ckgen_gating_xpu_his_clk_ckgen;

extern clk_node_t g_ckgen_gating_xpu_vpu1_clk_ckgen;

extern clk_node_t g_ckgen_gating_xpu_nna1_clk_ckgen;

extern clk_node_t g_ckgen_gating_xpu_nna2_clk_ckgen;

extern clk_node_t g_ckgen_gating_aaxi_imgs0_mainwm0;

extern clk_node_t g_ckgen_gating_aapb_hpi2disp;

extern clk_node_t g_ckgen_gating_noc_mainw_sysclk;

extern clk_node_t g_ckgen_gating_noc_mainw_pu600clk;

extern clk_node_t g_ckgen_gating_noc_mainw_perclk;

extern clk_node_t g_ckgen_gating_pll_hpiw_pclk;

extern clk_node_t g_ckgen_gating_iram7_aclk;

extern clk_node_t g_ckgen_gating_iram8_aclk;

extern clk_node_t g_ckgen_gating_cssys_clk;

extern clk_node_t g_ckgen_gating_scr4k_sid_pclk;

extern clk_node_t g_ckgen_gating_scr4k_ssid_pclk;

extern clk_node_t g_ckgen_gating_eic_hpi_pclk;

extern clk_node_t g_ckgen_gating_scr_hpi_pclk;

extern clk_node_t g_ckgen_gating_ftbu0_pclk;

extern clk_node_t g_ckgen_gating_ftbu1_pclk;

extern clk_node_t g_ckgen_gating_ftbu0_aclk;

extern clk_node_t g_ckgen_gating_ftbu1_aclk;

extern clk_node_t g_ckgen_gating_tbu_16_aclk;

extern clk_node_t g_ckgen_gating_cpu1_coreclk0;

extern clk_node_t g_ckgen_gating_cpu1_coreclk1;

extern clk_node_t g_ckgen_gating_cpu1_coreclk2;

extern clk_node_t g_ckgen_gating_cpu1_coreclk3;

extern clk_node_t g_ckgen_gating_cpu1_coreclk4;

extern clk_node_t g_ckgen_gating_cpu1_coreclk5;

extern clk_node_t g_ckgen_gating_cpu1_coreclk6;

extern clk_node_t g_ckgen_gating_cpu1_coreclk7;

extern clk_node_t g_ckgen_gating_cpu1_periphclk;

extern clk_node_t g_ckgen_gating_cpu1_sys_cnt_sync;

extern clk_node_t g_ckgen_gating_cpu1_pclk;

extern clk_node_t g_ckgen_gating_cpu1_gicclk;

extern clk_node_t g_ckgen_gating_cpu1_atclk;

extern clk_node_t g_ckgen_gating_cpu1_sclk;

extern clk_node_t g_ckgen_gating_pll_cpu1a_pclk;

extern clk_node_t g_ckgen_gating_pll_cpu1b_pclk;

extern clk_node_t g_ckgen_gating_pll_cpu1c_pclk;

extern clk_node_t g_ckgen_gating_tbu_17_aclk;

extern clk_node_t g_ckgen_gating_cpu2_coreclk0;

extern clk_node_t g_ckgen_gating_cpu2_coreclk1;

extern clk_node_t g_ckgen_gating_cpu2_coreclk2;

extern clk_node_t g_ckgen_gating_cpu2_coreclk3;

extern clk_node_t g_ckgen_gating_cpu2_periphclk;

extern clk_node_t g_ckgen_gating_cpu2_pclk;

extern clk_node_t g_ckgen_gating_cpu2_gicclk;

extern clk_node_t g_ckgen_gating_cpu2_atclk;

extern clk_node_t g_ckgen_gating_cpu2_sclk;

extern clk_node_t g_ckgen_gating_pll_cpu2a_pclk;

extern clk_node_t g_ckgen_gating_pll_cpu2b_pclk;

extern clk_node_t g_ckgen_gating_tbu_13_aclk;

extern clk_node_t g_ckgen_gating_tbu_14_aclk;

extern clk_node_t g_ckgen_gating_tbu9_aclk;

extern clk_node_t g_ckgen_gating_tbu10_aclk;

extern clk_node_t g_ckgen_gating_vpu1_bclk;

extern clk_node_t g_ckgen_gating_vpu2_aclk;

extern clk_node_t g_ckgen_gating_vpu3_aclk;

extern clk_node_t g_ckgen_gating_vpu4_aclk;

extern clk_node_t g_ckgen_gating_pll_vpu1_pclk;

extern clk_node_t g_ckgen_gating_pll_vpu2_pclk;

extern clk_node_t g_ckgen_gating_tbu11_aclk;

extern clk_node_t g_ckgen_gating_tbu12_aclk;

extern clk_node_t g_ckgen_gating_tbu15_aclk;

extern clk_node_t g_ckgen_gating_tbu8_aclk;

extern clk_node_t g_ckgen_gating_dciclk;

extern clk_node_t g_ckgen_gating_tbu3_aclk;

extern clk_node_t g_ckgen_gating_pciex4_axiclk;

extern clk_node_t g_ckgen_gating_pciex1a_axiaclk;

extern clk_node_t g_ckgen_gating_pciex1b_axiclk;

extern clk_node_t g_ckgen_gating_tbu7_aclk;

extern clk_node_t g_ckgen_gating_usb1_axiclk;

extern clk_node_t g_ckgen_gating_usb2_axiclk;

extern clk_node_t g_ckgen_gating_usb3_axiclk;

extern clk_node_t g_ckgen_gating_pll_his_pclk;

extern clk_node_t g_ckgen_gating_pciephyx4_pclk;

extern clk_node_t g_ckgen_gating_pciephyx2_pclk;

extern clk_node_t g_ckgen_gating_nna1_sysclk;

extern clk_node_t g_ckgen_gating_nna1_dbgclk;

extern clk_node_t g_ckgen_gating_pll_nna1_pclk;

extern clk_node_t g_ckgen_gating_tbu1_aclk;

extern clk_node_t g_ckgen_gating_tcu_aclk;

extern clk_node_t g_ckgen_gating_noc_maine_sysclk;

extern clk_node_t g_ckgen_gating_noc_maine_perclk;

extern clk_node_t g_ckgen_gating_noc_maine_pu800clk;

extern clk_node_t g_ckgen_gating_iram5_aclk;

extern clk_node_t g_ckgen_gating_iram6_aclk;

extern clk_node_t g_ckgen_gating_caf_aclk;

extern clk_node_t g_ckgen_gating_pll_hpie_pclk;

extern clk_node_t g_ckgen_gating_bti_noc_maine_cpu1;

extern clk_node_t g_ckgen_gating_bti_noc_maine_cpu2;

extern clk_node_t g_ckgen_gating_bti_noc_maine_nna1;

extern clk_node_t g_ckgen_gating_bti_noc_maine_nna2;

extern clk_node_t g_ckgen_gating_adb_ddr_clk;

extern clk_node_t g_ckgen_gating_aapb_vpu_clk;

extern clk_node_t g_ckgen_gating_nna2_ceva_clk;

extern clk_node_t g_ckgen_gating_pll_nna2_pclk;

extern clk_node_t g_ckgen_gating_iop_nna2_clk;

extern clk_node_t g_ckgen_gating_nna2_etm_atb_clk;

extern clk_node_t g_ckgen_gating_nna2_ceva_clk_timer;


extern void board_clk_init(void);


#endif

#endif
