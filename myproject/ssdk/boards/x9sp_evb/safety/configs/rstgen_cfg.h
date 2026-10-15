/*
 * Copyright (c) 2021-2029 Semidrive Incorporated.
 * All rights reserved.
 * Software License Agreement.
 */
#ifndef BOARDS_RSTGEN_CFG_H
#define BOARDS_RSTGEN_CFG_H

#ifdef CONFIG_RESETGEN

#include <driver/reset.h>

extern reset_sig_t rstsig_sw_self_rst;
extern reset_sig_t rstsig_cr5_saf;
extern reset_sig_t rstsig_gic1;
extern reset_sig_t rstsig_ospi1;
extern reset_sig_t rstsig_eth1;
extern reset_sig_t rstsig_i2s_sc1;
extern reset_sig_t rstsig_i2s_sc2;
extern reset_sig_t rstsig_canfd1;
extern reset_sig_t rstsig_canfd2;
extern reset_sig_t rstsig_canfd3;
extern reset_sig_t rstsig_canfd4;
extern reset_sig_t rstsig_sem1;
extern reset_sig_t rstsig_sem2;
extern reset_sig_t rstsig_adc2;
extern reset_sig_t rstsig_adc3;
extern reset_sig_t rstsig_xspi_slv;
extern reset_sig_t rstsig_xspi1_mst_core0;
extern reset_sig_t rstsig_xspi1_mst_core1;
extern reset_sig_t rstsig_ospi2;
extern reset_sig_t rstsig_i2s_sc3;
extern reset_sig_t rstsig_i2s_sc4;
extern reset_sig_t rstsig_i2s_sc5;
extern reset_sig_t rstsig_i2s_sc6;
extern reset_sig_t rstsig_i2s_sc7;
extern reset_sig_t rstsig_i2s_sc8;
extern reset_sig_t rstsig_i2s_mc1;
extern reset_sig_t rstsig_i2s_mc2;
extern reset_sig_t rstsig_canfd5;
extern reset_sig_t rstsig_canfd6;
extern reset_sig_t rstsig_canfd7;
extern reset_sig_t rstsig_canfd8;
extern reset_sig_t rstsig_canfd9;
extern reset_sig_t rstsig_canfd10;
extern reset_sig_t rstsig_canfd11;
extern reset_sig_t rstsig_canfd12;
extern reset_sig_t rstsig_canfd13;
extern reset_sig_t rstsig_canfd14;
extern reset_sig_t rstsig_canfd15;
extern reset_sig_t rstsig_canfd16;
extern reset_sig_t rstsig_canfd17;
extern reset_sig_t rstsig_canfd18;
extern reset_sig_t rstsig_canfd19;
extern reset_sig_t rstsig_canfd20;
extern reset_sig_t rstsig_eth2;
extern reset_sig_t rstsig_mshc1;
extern reset_sig_t rstsig_mshc2;
extern reset_sig_t rstsig_mshc3;
extern reset_sig_t rstsig_mshc4;
extern reset_sig_t rstsig_ce2;
extern reset_sig_t rstsig_adc1;
extern reset_sig_t rstsig_gic2;
extern reset_sig_t rstsig_gic3;
extern reset_sig_t rstsig_gic4;
extern reset_sig_t rstsig_gic5;
extern reset_sig_t rstsig_dbg_req_ack;
extern reset_sig_t rstsig_dci;
extern reset_sig_t rstsig_cpu1_core0_warm;
extern reset_sig_t rstsig_cpu1_core1_warm;
extern reset_sig_t rstsig_cpu1_core2_warm;
extern reset_sig_t rstsig_cpu1_core3_warm;
extern reset_sig_t rstsig_cpu1_core4_warm;
extern reset_sig_t rstsig_cpu1_core5_warm;
extern reset_sig_t rstsig_cpu1_core6_warm;
extern reset_sig_t rstsig_cpu1_core7_warm;
extern reset_sig_t rstsig_cpu1_scu_warm;
extern reset_sig_t rstsig_cpu2_core0_warm;
extern reset_sig_t rstsig_cpu2_core1_warm;
extern reset_sig_t rstsig_cpu2_core2_warm;
extern reset_sig_t rstsig_cpu2_core3_warm;
extern reset_sig_t rstsig_cpu2_scu_warm;
extern reset_sig_t rstsig_vpu1;
extern reset_sig_t rstsig_vpu2;
extern reset_sig_t rstsig_vpu3;
extern reset_sig_t rstsig_vpu4;
extern reset_sig_t rstsig_mipi_csi1;
extern reset_sig_t rstsig_mipi_csi2;
extern reset_sig_t rstsig_mipi_csi3;
extern reset_sig_t rstsig_csi1;
extern reset_sig_t rstsig_csi2;
extern reset_sig_t rstsig_csi3;
extern reset_sig_t rstsig_csi4;
extern reset_sig_t rstsig_g2d1;
extern reset_sig_t rstsig_g2d2;
extern reset_sig_t rstsig_mipi_dsi1;
extern reset_sig_t rstsig_mipi_dsi2;
extern reset_sig_t rstsig_dc1;
extern reset_sig_t rstsig_dc2;
extern reset_sig_t rstsig_dc3;
extern reset_sig_t rstsig_dc4;
extern reset_sig_t rstsig_dc5;
extern reset_sig_t rstsig_dp1;
extern reset_sig_t rstsig_dp2;
extern reset_sig_t rstsig_dp3;
extern reset_sig_t rstsig_lvds_ss;
extern reset_sig_t rstsig_isp;
extern reset_sig_t rstsig_vdsp_dreset;
extern reset_sig_t rstsig_pcie1_pciex4;
extern reset_sig_t rstsig_pcie2_pciex1a;
extern reset_sig_t rstsig_pcie2_pciex1b;
extern reset_sig_t rstsig_pciex4phy;
extern reset_sig_t rstsig_usb1;
extern reset_sig_t rstsig_usb2;
extern reset_sig_t rstsig_pciex2phy;
extern reset_sig_t rstsig_ddr_secb;
extern reset_sig_t rstsig_ddr_axi;
extern reset_sig_t rstsig_ddr_core;
extern reset_sig_t rstsig_usb1_aon;
extern reset_sig_t rstsig_usb2_aon;
extern reset_sig_t rstsig_cpu1_ss;
extern reset_sig_t rstsig_cpu2_ss;
extern reset_sig_t rstsig_gpu1_ss;
extern reset_sig_t rstsig_gpu2_ss;
extern reset_sig_t rstsig_vpu_ss;
extern reset_sig_t rstsig_img_ss;
extern reset_sig_t rstsig_disp_ss;
extern reset_sig_t rstsig_vsn_ss;
extern reset_sig_t rstsig_his_ss;
extern reset_sig_t rstsig_ddr_ss;
extern reset_sig_t rstsig_nna_ss;
extern reset_sig_t rstsig_cpu1_ss_aon;
extern reset_sig_t rstsig_cpu2_ss_aon;
extern reset_sig_t rstsig_vpu_ss_aon;
extern reset_sig_t rstsig_img_ss_aon;
extern reset_sig_t rstsig_disp_ss_aon;
extern reset_sig_t rstsig_his_ss_aon;
extern reset_sig_t rstsig_nna_ss_aon;
extern reset_sig_t rstsig_isolate_cpu;
extern reset_sig_t rstsig_isolate_gpu;
extern reset_sig_t rstsig_isolate_ddr;
extern reset_sig_t rstsig_isolate_nna_vsn;
extern reset_sig_t rstsig_isolate_pcie1;
extern reset_sig_t rstsig_isolate_pcie2;
extern reset_sig_t rstsig_isolate_usb1;
extern reset_sig_t rstsig_isolate_usb2;
extern reset_sig_t rstsig_vdsp;
extern reset_sig_t rstsig_cr5_sec;
extern reset_sig_t rstsig_cr5_mp;
extern reset_sig_t rstsig_cpu1_core_all;
extern reset_sig_t rstsig_cpu2_core_all;
extern reset_sig_t rstsig_gpu1;
extern reset_sig_t rstsig_gpu2;
extern reset_sig_t rstsig_vsn2_ss;
extern reset_sig_t rstsig_iram9;
extern reset_sig_t rstsig_iram10;
extern reset_sig_t rstsig_iram11;
extern reset_sig_t rstsig_iram12;
extern reset_sig_t rstsig_nna_por_rst_b;
extern reset_sig_t rstsig_vdsp2;
extern reset_sig_t rstsig_nna_core_rst_b;

extern void board_reset_init(void);

#endif

#endif /* BOARDS_RSTGEN_CFG_H */
