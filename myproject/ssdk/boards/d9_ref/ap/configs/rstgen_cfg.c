
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

static sdrv_reset_ctl_t sdrv_rstgen_sec_dev = {
    .base = APB_RSTGEN_SEC_BASE,
};
static reset_ctl_t g_rstgen_sec;

//reset_sig_t rstsig_cr5_saf_self = {
//    .rst_ctl = &g_rstgen_saf,
//    .id      = SDRV_RESET_SIG_ID(SDRV_RESET_SELF, 0),
//};

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

reset_sig_t rstsig_i2s_mc1 = {
    .rst_ctl = &g_rstgen_sec,
    .id      = RSTSIG_AP_I2S_MC1,
};

reset_sig_t rstsig_i2s_mc2 = {
    .rst_ctl = &g_rstgen_sec,
    .id      = RSTSIG_AP_I2S_MC2,
};

reset_sig_t rstsig_enet2 = {
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

reset_sig_t rstsig_mipi_dsi1 = {
    .rst_ctl = &g_rstgen_sec,
    .id      = RSTSIG_AP_MIPI_DSI1,
};

reset_sig_t rstsig_dc1 = {
    .rst_ctl = &g_rstgen_sec,
    .id      = RSTSIG_AP_DC1,
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

void board_reset_init(void)
{

    sdrv_reset_init(&g_rstgen_sec, &sdrv_rstgen_sec_dev);

}
#endif
