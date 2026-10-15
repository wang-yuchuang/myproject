
//*****************************************************************************
//
// WARNING: Automatically generated file, don't modify anymore!!!
//
// Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
// Software License Agreement
//
//*****************************************************************************

#ifdef CONFIG_RESETGEN

#include <types.h>
#include <param.h>

#include <driver/kunlun/reset/sdrv_reset_drv.h>
#include <regs_base.h>

#include "rstgen_cfg.h"

static sdrv_reset_ctl_t sdrv_rstgen_saf_dev = {
    .base = APB_RSTGEN_SAF_BASE,
    .self_rst_en = 1,
    .sem_rst_en = 1,
    .dbg_rst_en = 1,
    .wdg1_rst_en = 1,
};
static reset_ctl_t g_rstgen_saf;

static reset_ctl_t g_rstgen_sec;

reset_sig_t rstsig_sw_self_rst = {
    .rst_ctl = &g_rstgen_saf,
    .id = RSTSIG_SW_SELF_RST,
};

reset_sig_t rstsig_cr5_saf = {
    .rst_ctl = &g_rstgen_saf,
    .id      = RSTSIG_SAF_CR5_SAF,
};

reset_sig_t rstsig_gic1 = {
    .rst_ctl = &g_rstgen_saf,
    .id      = RSTSIG_SAF_GIC1,
};

reset_sig_t rstsig_ospi1 = {
    .rst_ctl = &g_rstgen_saf,
    .id      = RSTSIG_SAF_OSPI1,
};

reset_sig_t rstsig_eth1 = {
    .rst_ctl = &g_rstgen_saf,
    .id      = RSTSIG_SAF_ETH1,
};

reset_sig_t rstsig_i2s_sc1 = {
    .rst_ctl = &g_rstgen_saf,
    .id      = RSTSIG_SAF_I2S_SC1,
};

reset_sig_t rstsig_i2s_sc2 = {
    .rst_ctl = &g_rstgen_saf,
    .id      = RSTSIG_SAF_I2S_SC2,
};

reset_sig_t rstsig_canfd1 = {
    .rst_ctl = &g_rstgen_saf,
    .id      = RSTSIG_SAF_CANFD1,
};

reset_sig_t rstsig_canfd2 = {
    .rst_ctl = &g_rstgen_saf,
    .id      = RSTSIG_SAF_CANFD2,
};

reset_sig_t rstsig_canfd3 = {
    .rst_ctl = &g_rstgen_saf,
    .id      = RSTSIG_SAF_CANFD3,
};

reset_sig_t rstsig_canfd4 = {
    .rst_ctl = &g_rstgen_saf,
    .id      = RSTSIG_SAF_CANFD4,
};

reset_sig_t rstsig_sem1 = {
    .rst_ctl = &g_rstgen_saf,
    .id      = RSTSIG_SAF_SEM1,
};

reset_sig_t rstsig_sem2 = {
    .rst_ctl = &g_rstgen_saf,
    .id      = RSTSIG_SAF_SEM2,
};

reset_sig_t rstsig_core_vdsp = {
    .rst_ctl = &g_rstgen_sec,
    .id      = RSTSIG_AP_CORE_VDSP,
};

reset_sig_t rstsig_cr5_sec = {
    .rst_ctl = &g_rstgen_sec,
    .id      = RSTSIG_AP_CR5_SEC,
};

reset_sig_t rstsig_cr5_mp = {
    .rst_ctl = &g_rstgen_sec,
    .id      = RSTSIG_AP_CR5_MP,
};

reset_sig_t rstsig_cpu1_all = {
    .rst_ctl = &g_rstgen_sec,
    .id      = RSTSIG_AP_CPU1_ALL,
};

reset_sig_t rstsig_cpu2 = {
    .rst_ctl = &g_rstgen_sec,
    .id      = RSTSIG_AP_CPU2,
};

reset_sig_t rstsig_core_adsp = {
    .rst_ctl = &g_rstgen_sec,
    .id      = RSTSIG_AP_CORE_ADSP,
};

reset_sig_t rstsig_ospi2 = {
    .rst_ctl = &g_rstgen_sec,
    .id      = RSTSIG_AP_OSPI2,
};

reset_sig_t rstsig_i2s_sc3 = {
    .rst_ctl = &g_rstgen_sec,
    .id      = RSTSIG_AP_I2S_SC3,
};

reset_sig_t rstsig_i2s_sc4 = {
    .rst_ctl = &g_rstgen_sec,
    .id      = RSTSIG_AP_I2S_SC4,
};

reset_sig_t rstsig_i2s_sc5 = {
    .rst_ctl = &g_rstgen_sec,
    .id      = RSTSIG_AP_I2S_SC5,
};

reset_sig_t rstsig_i2s_sc6 = {
    .rst_ctl = &g_rstgen_sec,
    .id      = RSTSIG_AP_I2S_SC6,
};

reset_sig_t rstsig_i2s_sc7 = {
    .rst_ctl = &g_rstgen_sec,
    .id      = RSTSIG_AP_I2S_SC7,
};

reset_sig_t rstsig_i2s_sc8 = {
    .rst_ctl = &g_rstgen_sec,
    .id      = RSTSIG_AP_I2S_SC8,
};

reset_sig_t rstsig_i2s_mc1 = {
    .rst_ctl = &g_rstgen_sec,
    .id      = RSTSIG_AP_I2S_MC1,
};

reset_sig_t rstsig_i2s_mc2 = {
    .rst_ctl = &g_rstgen_sec,
    .id      = RSTSIG_AP_I2S_MC2,
};

reset_sig_t rstsig_canfd5 = {
    .rst_ctl = &g_rstgen_sec,
    .id      = RSTSIG_AP_CANFD5,
};

reset_sig_t rstsig_canfd6 = {
    .rst_ctl = &g_rstgen_sec,
    .id      = RSTSIG_AP_CANFD6,
};

reset_sig_t rstsig_canfd7 = {
    .rst_ctl = &g_rstgen_sec,
    .id      = RSTSIG_AP_CANFD7,
};

reset_sig_t rstsig_canfd8 = {
    .rst_ctl = &g_rstgen_sec,
    .id      = RSTSIG_AP_CANFD8,
};

reset_sig_t rstsig_eth2 = {
    .rst_ctl = &g_rstgen_sec,
    .id      = RSTSIG_AP_ETH2,
};

reset_sig_t rstsig_mshc1 = {
    .rst_ctl = &g_rstgen_sec,
    .id      = RSTSIG_AP_MSHC1,
};

reset_sig_t rstsig_mshc2 = {
    .rst_ctl = &g_rstgen_sec,
    .id      = RSTSIG_AP_MSHC2,
};

reset_sig_t rstsig_mshc3 = {
    .rst_ctl = &g_rstgen_sec,
    .id      = RSTSIG_AP_MSHC3,
};

reset_sig_t rstsig_mshc4 = {
    .rst_ctl = &g_rstgen_sec,
    .id      = RSTSIG_AP_MSHC4,
};

reset_sig_t rstsig_module_adsp = {
    .rst_ctl = &g_rstgen_sec,
    .id      = RSTSIG_AP_MODULE_ADSP,
};

reset_sig_t rstsig_gic2 = {
    .rst_ctl = &g_rstgen_sec,
    .id      = RSTSIG_AP_GIC2,
};

reset_sig_t rstsig_gic3 = {
    .rst_ctl = &g_rstgen_sec,
    .id      = RSTSIG_AP_GIC3,
};

reset_sig_t rstsig_cpu1_core0_warm = {
    .rst_ctl = &g_rstgen_sec,
    .id      = RSTSIG_AP_CPU1_CORE0_WARM,
};

reset_sig_t rstsig_cpu1_core1_warm = {
    .rst_ctl = &g_rstgen_sec,
    .id      = RSTSIG_AP_CPU1_CORE1_WARM,
};

reset_sig_t rstsig_cpu1_core2_warm = {
    .rst_ctl = &g_rstgen_sec,
    .id      = RSTSIG_AP_CPU1_CORE2_WARM,
};

reset_sig_t rstsig_cpu1_core3_warm = {
    .rst_ctl = &g_rstgen_sec,
    .id      = RSTSIG_AP_CPU1_CORE3_WARM,
};

reset_sig_t rstsig_cpu1_core4_warm = {
    .rst_ctl = &g_rstgen_sec,
    .id      = RSTSIG_AP_CPU1_CORE4_WARM,
};

reset_sig_t rstsig_cpu1_core5_warm = {
    .rst_ctl = &g_rstgen_sec,
    .id      = RSTSIG_AP_CPU1_CORE5_WARM,
};

reset_sig_t rstsig_cpu1_scu_warm = {
    .rst_ctl = &g_rstgen_sec,
    .id      = RSTSIG_AP_CPU1_SCU_WARM,
};

reset_sig_t rstsig_ddr_ss = {
    .rst_ctl = &g_rstgen_sec,
    .id      = RSTSIG_AP_DDR_SS,
};

reset_sig_t rstsig_ddr_apb = {
    .rst_ctl = &g_rstgen_sec,
    .id      = RSTSIG_AP_DDR_APB,
};

reset_sig_t rstsig_ddr_axi = {
    .rst_ctl = &g_rstgen_sec,
    .id      = RSTSIG_AP_DDR_AXI,
};

reset_sig_t rstsig_ddr_core = {
    .rst_ctl = &g_rstgen_sec,
    .id      = RSTSIG_AP_DDR_CORE,
};

reset_sig_t rstsig_gic4 = {
    .rst_ctl = &g_rstgen_sec,
    .id      = RSTSIG_AP_GIC4,
};

reset_sig_t rstsig_gic5 = {
    .rst_ctl = &g_rstgen_sec,
    .id      = RSTSIG_AP_GIC5,
};

reset_sig_t rstsig_cssys_treset_n = {
    .rst_ctl = &g_rstgen_sec,
    .id      = RSTSIG_AP_CSSYS_TRESET_N,
};

reset_sig_t rstsig_nna = {
    .rst_ctl = &g_rstgen_sec,
    .id      = RSTSIG_AP_NNA,
};

reset_sig_t rstsig_vdsp_dreset = {
    .rst_ctl = &g_rstgen_sec,
    .id      = RSTSIG_AP_VDSP_DRESET,
};

reset_sig_t rstsig_vpu1 = {
    .rst_ctl = &g_rstgen_sec,
    .id      = RSTSIG_AP_VPU1,
};

reset_sig_t rstsig_vpu2 = {
    .rst_ctl = &g_rstgen_sec,
    .id      = RSTSIG_AP_VPU2,
};

reset_sig_t rstsig_mjpeg = {
    .rst_ctl = &g_rstgen_sec,
    .id      = RSTSIG_AP_MJPEG,
};

reset_sig_t rstsig_cpu1_ss = {
    .rst_ctl = &g_rstgen_sec,
    .id      = RSTSIG_AP_CPU1_SS,
};

reset_sig_t rstsig_cpu2_ss = {
    .rst_ctl = &g_rstgen_sec,
    .id      = RSTSIG_AP_CPU2_SS,
};

reset_sig_t rstsig_pcie1 = {
    .rst_ctl = &g_rstgen_sec,
    .id      = RSTSIG_AP_PCIE1,
};

reset_sig_t rstsig_pcie2 = {
    .rst_ctl = &g_rstgen_sec,
    .id      = RSTSIG_AP_PCIE2,
};

reset_sig_t rstsig_pciephy = {
    .rst_ctl = &g_rstgen_sec,
    .id      = RSTSIG_AP_PCIEPHY,
};

reset_sig_t rstsig_usb1 = {
    .rst_ctl = &g_rstgen_sec,
    .id      = RSTSIG_AP_USB1,
};

reset_sig_t rstsig_usb2 = {
    .rst_ctl = &g_rstgen_sec,
    .id      = RSTSIG_AP_USB2,
};

reset_sig_t rstsig_mipi_csi1 = {
    .rst_ctl = &g_rstgen_sec,
    .id      = RSTSIG_AP_MIPI_CSI1,
};

reset_sig_t rstsig_mipi_csi2 = {
    .rst_ctl = &g_rstgen_sec,
    .id      = RSTSIG_AP_MIPI_CSI2,
};

reset_sig_t rstsig_mipi_csi3 = {
    .rst_ctl = &g_rstgen_sec,
    .id      = RSTSIG_AP_MIPI_CSI3,
};

reset_sig_t rstsig_mipi_dsi1 = {
    .rst_ctl = &g_rstgen_sec,
    .id      = RSTSIG_AP_MIPI_DSI1,
};

reset_sig_t rstsig_mipi_dsi2 = {
    .rst_ctl = &g_rstgen_sec,
    .id      = RSTSIG_AP_MIPI_DSI2,
};

reset_sig_t rstsig_dc1 = {
    .rst_ctl = &g_rstgen_sec,
    .id      = RSTSIG_AP_DC1,
};

reset_sig_t rstsig_dc2 = {
    .rst_ctl = &g_rstgen_sec,
    .id      = RSTSIG_AP_DC2,
};

reset_sig_t rstsig_dc3 = {
    .rst_ctl = &g_rstgen_sec,
    .id      = RSTSIG_AP_DC3,
};

reset_sig_t rstsig_dc4 = {
    .rst_ctl = &g_rstgen_sec,
    .id      = RSTSIG_AP_DC4,
};

reset_sig_t rstsig_dc5 = {
    .rst_ctl = &g_rstgen_sec,
    .id      = RSTSIG_AP_DC5,
};

reset_sig_t rstsig_dp1 = {
    .rst_ctl = &g_rstgen_sec,
    .id      = RSTSIG_AP_DP1,
};

reset_sig_t rstsig_dp2 = {
    .rst_ctl = &g_rstgen_sec,
    .id      = RSTSIG_AP_DP2,
};

reset_sig_t rstsig_dp3 = {
    .rst_ctl = &g_rstgen_sec,
    .id      = RSTSIG_AP_DP3,
};

reset_sig_t rstsig_lvds_ss = {
    .rst_ctl = &g_rstgen_sec,
    .id      = RSTSIG_AP_LVDS_SS,
};

reset_sig_t rstsig_csi1 = {
    .rst_ctl = &g_rstgen_sec,
    .id      = RSTSIG_AP_CSI1,
};

reset_sig_t rstsig_csi2 = {
    .rst_ctl = &g_rstgen_sec,
    .id      = RSTSIG_AP_CSI2,
};

reset_sig_t rstsig_csi3 = {
    .rst_ctl = &g_rstgen_sec,
    .id      = RSTSIG_AP_CSI3,
};

reset_sig_t rstsig_disp_mux = {
    .rst_ctl = &g_rstgen_sec,
    .id      = RSTSIG_AP_DISP_MUX,
};

reset_sig_t rstsig_g2d1 = {
    .rst_ctl = &g_rstgen_sec,
    .id      = RSTSIG_AP_G2D1,
};

reset_sig_t rstsig_g2d2 = {
    .rst_ctl = &g_rstgen_sec,
    .id      = RSTSIG_AP_G2D2,
};

reset_sig_t rstsig_gpu1_core = {
    .rst_ctl = &g_rstgen_sec,
    .id      = RSTSIG_AP_GPU1_CORE,
};

reset_sig_t rstsig_gpu1_ss = {
    .rst_ctl = &g_rstgen_sec,
    .id      = RSTSIG_AP_GPU1_SS,
};

reset_sig_t rstsig_gpu2_core = {
    .rst_ctl = &g_rstgen_sec,
    .id      = RSTSIG_AP_GPU2_CORE,
};

reset_sig_t rstsig_gpu2_ss = {
    .rst_ctl = &g_rstgen_sec,
    .id      = RSTSIG_AP_GPU2_SS,
};

reset_sig_t rstsig_dbg_req = {
    .rst_ctl = &g_rstgen_sec,
    .id      = RSTSIG_AP_DBG_REQ,
};

reset_sig_t rstsig_canfd9 = {
    .rst_ctl = &g_rstgen_sec,
    .id      = RSTSIG_AP_CANFD9,
};

reset_sig_t rstsig_canfd10 = {
    .rst_ctl = &g_rstgen_sec,
    .id      = RSTSIG_AP_CANFD10,
};

reset_sig_t rstsig_canfd11 = {
    .rst_ctl = &g_rstgen_sec,
    .id      = RSTSIG_AP_CANFD11,
};

reset_sig_t rstsig_canfd12 = {
    .rst_ctl = &g_rstgen_sec,
    .id      = RSTSIG_AP_CANFD12,
};

reset_sig_t rstsig_canfd13 = {
    .rst_ctl = &g_rstgen_sec,
    .id      = RSTSIG_AP_CANFD13,
};

reset_sig_t rstsig_canfd14 = {
    .rst_ctl = &g_rstgen_sec,
    .id      = RSTSIG_AP_CANFD14,
};

reset_sig_t rstsig_canfd15 = {
    .rst_ctl = &g_rstgen_sec,
    .id      = RSTSIG_AP_CANFD15,
};

reset_sig_t rstsig_canfd16 = {
    .rst_ctl = &g_rstgen_sec,
    .id      = RSTSIG_AP_CANFD16,
};

reset_sig_t rstsig_canfd17 = {
    .rst_ctl = &g_rstgen_sec,
    .id      = RSTSIG_AP_CANFD17,
};

reset_sig_t rstsig_canfd18 = {
    .rst_ctl = &g_rstgen_sec,
    .id      = RSTSIG_AP_CANFD18,
};

reset_sig_t rstsig_canfd19 = {
    .rst_ctl = &g_rstgen_sec,
    .id      = RSTSIG_AP_CANFD19,
};

reset_sig_t rstsig_canfd20 = {
    .rst_ctl = &g_rstgen_sec,
    .id      = RSTSIG_AP_CANFD20,
};

reset_sig_t rstsig_ce2 = {
    .rst_ctl = &g_rstgen_sec,
    .id      = RSTSIG_AP_CE2,
};

reset_sig_t rstsig_adc = {
    .rst_ctl = &g_rstgen_sec,
    .id      = RSTSIG_AP_ADC,
};

reset_sig_t rstsig_isolate_pcie = {
    .rst_ctl = &g_rstgen_sec,
    .id      = RSTSIG_AP_ISOLATE_PCIE,
};

reset_sig_t rstsig_isolate_usb = {
    .rst_ctl = &g_rstgen_sec,
    .id      = RSTSIG_AP_ISOLATE_USB,
};

reset_sig_t rstsig_isolate_cpu1 = {
    .rst_ctl = &g_rstgen_sec,
    .id      = RSTSIG_AP_ISOLATE_CPU1,
};

reset_sig_t rstsig_isolate_gpu1 = {
    .rst_ctl = &g_rstgen_sec,
    .id      = RSTSIG_AP_ISOLATE_GPU1,
};

reset_sig_t rstsig_isolate_ddr = {
    .rst_ctl = &g_rstgen_sec,
    .id      = RSTSIG_AP_ISOLATE_DDR,
};

static reset_sig_t *board_reset_array[] = {
    &rstsig_gic1,
    &rstsig_sem1,
};

void board_reset_init(void)
{

    sdrv_reset_init(&g_rstgen_saf, &sdrv_rstgen_saf_dev);

    for (uint32_t i = 0; i < ARRAY_SIZE(board_reset_array); i++) {
        reset_ctl_reset(board_reset_array[i]);
    }
}
#endif
