
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

extern clk_node_t g_ckgen_ip_ce1;

extern clk_node_t g_ckgen_ip_i2c_saf;

extern clk_node_t g_ckgen_ip_spi_saf;

extern clk_node_t g_ckgen_ip_uart_saf;

extern clk_node_t g_ckgen_ip_i2s_mclk1;

extern clk_node_t g_ckgen_ip_i2s_sc1;

extern clk_node_t g_ckgen_ip_i2s_sc2;

extern clk_node_t g_ckgen_ip_eth1_tx;

extern clk_node_t g_ckgen_ip_eth1_rmii;

extern clk_node_t g_ckgen_ip_eth1_phy_ref;

extern clk_node_t g_ckgen_ip_eth1_timer_sec;

extern clk_node_t g_ckgen_ip_ospi1;

extern clk_node_t g_ckgen_ip_timer1;

extern clk_node_t g_ckgen_ip_timer2;

extern clk_node_t g_ckgen_ip_pwm1;

extern clk_node_t g_ckgen_ip_pwm2;

extern clk_node_t g_ckgen_ip_can_1_to_4;

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

extern clk_node_t g_ckgen_ip_ce2;

extern clk_node_t g_ckgen_ip_adc;

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

extern clk_node_t g_ckgen_ip_eth2_tx;

extern clk_node_t g_ckgen_ip_eth2_rmii;

extern clk_node_t g_ckgen_ip_eth2_phy_ref;

extern clk_node_t g_ckgen_ip_eth2_timer_sec;

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

extern clk_node_t g_ckgen_ip_trace;

extern clk_node_t g_ckgen_ip_sys_cnt;

extern clk_node_t g_ckgen_ip_mshc_timer;

extern clk_node_t g_ckgen_ip_hpi_clk600;

extern clk_node_t g_ckgen_ip_hpi_clk800;

extern clk_node_t g_pll_disp_vco;

extern clk_node_t g_pll_disp_root;

extern clk_node_t g_pll_disp_div_a;

extern clk_node_t g_pll_disp_div_b;

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

extern clk_node_t g_ckgen_ip_csi3;

extern clk_node_t g_ckgen_ip_dp1;

extern clk_node_t g_ckgen_ip_dp2;

extern clk_node_t g_ckgen_ip_dp3;

extern clk_node_t g_ckgen_ip_dc5;

extern clk_node_t g_ckgen_ip_dc1;

extern clk_node_t g_ckgen_ip_dc2;

extern clk_node_t g_ckgen_ip_dc3;

extern clk_node_t g_ckgen_ip_dc4;

extern clk_node_t g_ckgen_ip_spare1;

extern clk_node_t g_ckgen_ip_dc5_alt_dsp_clk;

extern clk_node_t g_ckgen_bus_disp_bus_root;

extern clk_node_t g_ckgen_bus_disp_bus_div_m;

extern clk_node_t g_ckgen_bus_disp_bus_div_n;

extern clk_node_t g_ckgen_bus_disp_bus_div_p;

extern clk_node_t g_ckgen_bus_disp_bus_div_q;

extern clk_node_t g_ckgen_ip_ext_aud1;

extern clk_node_t g_ckgen_ip_ext_aud2;

extern clk_node_t g_ckgen_ip_ext_aud3;

extern clk_node_t g_ckgen_ip_ext_aud4;

extern clk_node_t g_pll_cpu1a_vco;

extern clk_node_t g_pll_cpu1a_root;

extern clk_node_t g_pll_cpu1a_div_a;

extern clk_node_t g_pll_cpu1a_div_b;

extern clk_node_t g_pll_cpu1b_vco;

extern clk_node_t g_pll_cpu1b_root;

extern clk_node_t g_pll_cpu1b_div_a;

extern clk_node_t g_pll_cpu1b_div_b;

extern clk_node_t g_pll_gpu1_vco;

extern clk_node_t g_pll_gpu1_root;

extern clk_node_t g_pll_gpu1_div_a;

extern clk_node_t g_pll_gpu1_div_b;

extern clk_node_t g_pll_gpu2_vco;

extern clk_node_t g_pll_gpu2_root;

extern clk_node_t g_pll_gpu2_div_a;

extern clk_node_t g_pll_gpu2_div_b;

extern clk_node_t g_pll_vpu_vco;

extern clk_node_t g_pll_vpu_root;

extern clk_node_t g_pll_vpu_div_a;

extern clk_node_t g_pll_vpu_div_b;

extern clk_node_t g_pll_vsn_vco;

extern clk_node_t g_pll_vsn_root;

extern clk_node_t g_pll_vsn_div_a;

extern clk_node_t g_pll_vsn_div_b;

extern clk_node_t g_pll_hpi_vco;

extern clk_node_t g_pll_hpi_root;

extern clk_node_t g_pll_hpi_div_a;

extern clk_node_t g_pll_hpi_div_b;

extern clk_node_t g_pll_his_vco;

extern clk_node_t g_pll_his_root;

extern clk_node_t g_pll_his_div_a;

extern clk_node_t g_pll_his_div_b;

extern clk_node_t g_pll_ddr_vco;

extern clk_node_t g_pll_ddr_root;

extern clk_node_t g_pll_ddr_div_a;

extern clk_node_t g_pll_ddr_div_b;

extern clk_node_t g_ckgen_core_cpu1a;

extern clk_node_t g_ckgen_core_cpu1b;

extern clk_node_t g_ckgen_core_gpu1;

extern clk_node_t g_ckgen_core_gpu2;

extern clk_node_t g_ckgen_ip_vpu1;

extern clk_node_t g_ckgen_ip_mjpeg;

extern clk_node_t g_ckgen_bus_vpu_root;

extern clk_node_t g_ckgen_bus_vpu_div_m;

extern clk_node_t g_ckgen_bus_vpu_div_n;

extern clk_node_t g_ckgen_bus_vpu_div_p;

extern clk_node_t g_ckgen_bus_vpu_div_q;

extern clk_node_t g_ckgen_bus_vsn_root;

extern clk_node_t g_ckgen_bus_vsn_div_m;

extern clk_node_t g_ckgen_bus_vsn_div_n;

extern clk_node_t g_ckgen_bus_vsn_div_p;

extern clk_node_t g_ckgen_bus_vsn_div_q;

extern clk_node_t g_ckgen_core_ddr;

extern clk_node_t g_ckgen_bus_noc_root;

extern clk_node_t g_ckgen_bus_noc_div_m;

extern clk_node_t g_ckgen_bus_noc_div_n;

extern clk_node_t g_ckgen_bus_noc_div_p;

extern clk_node_t g_ckgen_bus_noc_div_q;

extern clk_node_t g_ckgen_bus_his_root;

extern clk_node_t g_ckgen_bus_his_div_m;

extern clk_node_t g_ckgen_bus_his_div_n;

extern clk_node_t g_ckgen_bus_his_div_p;

extern clk_node_t g_ckgen_bus_his_div_q;

extern clk_node_t g_ckgen_uuu_cpu1a_div_m;

extern clk_node_t g_ckgen_uuu_cpu1a_div_n;

extern clk_node_t g_ckgen_uuu_cpu1a_div_p;

extern clk_node_t g_ckgen_uuu_cpu1a_div_q;

extern clk_node_t g_ckgen_uuu_cpu1b_div_m;

extern clk_node_t g_ckgen_uuu_cpu1b_div_n;

extern clk_node_t g_ckgen_uuu_cpu1b_div_p;

extern clk_node_t g_ckgen_uuu_cpu1b_div_q;

extern clk_node_t g_ckgen_uuu_gpu1_div_m;

extern clk_node_t g_ckgen_uuu_gpu1_div_n;

extern clk_node_t g_ckgen_uuu_gpu1_div_p;

extern clk_node_t g_ckgen_uuu_gpu1_div_q;

extern clk_node_t g_ckgen_uuu_gpu2_div_m;

extern clk_node_t g_ckgen_uuu_gpu2_div_n;

extern clk_node_t g_ckgen_uuu_gpu2_div_p;

extern clk_node_t g_ckgen_uuu_gpu2_div_q;

extern clk_node_t g_ckgen_uuu_vpu1_div_m;

extern clk_node_t g_ckgen_uuu_vpu1_div_n;

extern clk_node_t g_ckgen_uuu_vpu1_div_p;

extern clk_node_t g_ckgen_uuu_vpu1_div_q;

extern clk_node_t g_ckgen_uuu_mjpeg_div_m;

extern clk_node_t g_ckgen_uuu_mjpeg_div_n;

extern clk_node_t g_ckgen_uuu_mjpeg_div_p;

extern clk_node_t g_ckgen_uuu_mjpeg_div_q;

extern clk_node_t g_ckgen_uuu_vpu_div_m;

extern clk_node_t g_ckgen_uuu_vpu_div_n;

extern clk_node_t g_ckgen_uuu_vpu_div_p;

extern clk_node_t g_ckgen_uuu_vpu_div_q;

extern clk_node_t g_ckgen_uuu_vsn_div_m;

extern clk_node_t g_ckgen_uuu_vsn_div_n;

extern clk_node_t g_ckgen_uuu_vsn_div_p;

extern clk_node_t g_ckgen_uuu_vsn_div_q;

extern clk_node_t g_ckgen_uuu_ddr_div_m;

extern clk_node_t g_ckgen_uuu_ddr_div_n;

extern clk_node_t g_ckgen_uuu_ddr_div_p;

extern clk_node_t g_ckgen_uuu_ddr_div_q;

extern clk_node_t g_ckgen_uuu_his_div_m;

extern clk_node_t g_ckgen_uuu_his_div_n;

extern clk_node_t g_ckgen_uuu_his_div_p;

extern clk_node_t g_ckgen_uuu_his_div_q;

extern clk_node_t g_ckgen_gating_sec_plat;

extern clk_node_t g_ckgen_gating_mp_plat;

extern clk_node_t g_ckgen_gating_ce2;

extern clk_node_t g_ckgen_gating_gic2;

extern clk_node_t g_ckgen_gating_gic3;

extern clk_node_t g_ckgen_gating_emmc1;

extern clk_node_t g_ckgen_gating_emmc2;

extern clk_node_t g_ckgen_gating_emmc3;

extern clk_node_t g_ckgen_gating_emmc4;

extern clk_node_t g_ckgen_gating_eth2;

extern clk_node_t g_ckgen_gating_ospi2;

extern clk_node_t g_ckgen_gating_dma2;

extern clk_node_t g_ckgen_gating_dma3;

extern clk_node_t g_ckgen_gating_dma4;

extern clk_node_t g_ckgen_gating_dma5;

extern clk_node_t g_ckgen_gating_dma6;

extern clk_node_t g_ckgen_gating_dma7;

extern clk_node_t g_ckgen_gating_dma8;

extern clk_node_t g_ckgen_gating_mb;

extern clk_node_t g_ckgen_gating_iram2;

extern clk_node_t g_ckgen_gating_iram3;

extern clk_node_t g_ckgen_gating_iram4;

extern clk_node_t g_ckgen_gating_romc2;

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

extern clk_node_t g_ckgen_gating_spi5;

extern clk_node_t g_ckgen_gating_spi6;

extern clk_node_t g_ckgen_gating_spi7;

extern clk_node_t g_ckgen_gating_spi8;

extern clk_node_t g_ckgen_gating_uart9;

extern clk_node_t g_ckgen_gating_uart10;

extern clk_node_t g_ckgen_gating_uart11;

extern clk_node_t g_ckgen_gating_uart12;

extern clk_node_t g_ckgen_gating_uart13;

extern clk_node_t g_ckgen_gating_uart14;

extern clk_node_t g_ckgen_gating_uart15;

extern clk_node_t g_ckgen_gating_uart16;

extern clk_node_t g_ckgen_gating_spdif1;

extern clk_node_t g_ckgen_gating_spdif2;

extern clk_node_t g_ckgen_gating_spdif3;

extern clk_node_t g_ckgen_gating_spdif4;

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

extern clk_node_t g_ckgen_gating_can5;

extern clk_node_t g_ckgen_gating_can6;

extern clk_node_t g_ckgen_gating_can7;

extern clk_node_t g_ckgen_gating_can8;

extern clk_node_t g_ckgen_gating_trace;

extern clk_node_t g_ckgen_gating_sys_cnt;

extern clk_node_t g_ckgen_gating_tbu14;

extern clk_node_t g_ckgen_gating_tbu15;

extern clk_node_t g_ckgen_gating_gpio2;

extern clk_node_t g_ckgen_gating_gpio3;

extern clk_node_t g_ckgen_gating_gpio4;

extern clk_node_t g_ckgen_gating_gpio5;

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

extern clk_node_t g_ckgen_gating_can9;

extern clk_node_t g_ckgen_gating_can10;

extern clk_node_t g_ckgen_gating_can11;

extern clk_node_t g_ckgen_gating_can12;

extern clk_node_t g_ckgen_gating_can13;

extern clk_node_t g_ckgen_gating_can14;

extern clk_node_t g_ckgen_gating_can15;

extern clk_node_t g_ckgen_gating_can16;

extern clk_node_t g_ckgen_gating_can17;

extern clk_node_t g_ckgen_gating_can18;

extern clk_node_t g_ckgen_gating_can19;

extern clk_node_t g_ckgen_gating_can20;

extern clk_node_t g_ckgen_gating_adc;


extern void board_clk_init(void);


#endif

#endif
