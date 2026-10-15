
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


#include <driver/taishan/reset/taishan_reset_drv.h>
#include <regs_base.h>

#include "rstgen_cfg.h"

static taishan_reset_ctl_t taishan_rstgen_sf_dev = {
    .base = APB_RSTGEN_SF_BASE,
};
static reset_ctl_t g_rstgen_saf;

static taishan_reset_ctl_t taishan_rstgen_ap_dev = {
    .base = APB_RSTGEN_AP_BASE,
};
static reset_ctl_t g_rstgen_ap;

reset_sig_t rstsig_rstgen_ap = {
    .rst_ctl = &g_rstgen_saf,
    .id      = RSTSIG_SAF_RSTGEN_AP,
};

reset_sig_t rstsig_cr5_saf = {
    .rst_ctl = &g_rstgen_saf,
    .id      = RSTSIG_SAF_CR5_SAF,
};

reset_sig_t rstsig_cr5_sp0 = {
    .rst_ctl = &g_rstgen_saf,
    .id      = RSTSIG_SAF_CR5_SP0,
};

reset_sig_t rstsig_cr5_sp1 = {
    .rst_ctl = &g_rstgen_saf,
    .id      = RSTSIG_SAF_CR5_SP1,
};

reset_sig_t rstsig_cr5_sx0 = {
    .rst_ctl = &g_rstgen_saf,
    .id      = RSTSIG_SAF_CR5_SX0,
};

reset_sig_t rstsig_cr5_sx1 = {
    .rst_ctl = &g_rstgen_saf,
    .id      = RSTSIG_SAF_CR5_SX1,
};

reset_sig_t rstsig_mac = {
    .rst_ctl = &g_rstgen_saf,
    .id      = RSTSIG_SAF_MAC,
};

reset_sig_t rstsig_scr_boot = {
    .rst_ctl = &g_rstgen_saf,
    .id      = RSTSIG_SAF_SCR_BOOT,
};

reset_sig_t rstsig_gpio_sf = {
    .rst_ctl = &g_rstgen_saf,
    .id      = RSTSIG_SAF_GPIO_SF,
};

reset_sig_t rstsig_wdt1_2 = {
    .rst_ctl = &g_rstgen_saf,
    .id      = RSTSIG_SAF_WDT1_2,
};

reset_sig_t rstsig_sem = {
    .rst_ctl = &g_rstgen_saf,
    .id      = RSTSIG_SAF_SEM,
};

reset_sig_t rstsig_ana_sf = {
    .rst_ctl = &g_rstgen_saf,
    .id      = RSTSIG_SAF_ANA_SF,
};

reset_sig_t rstsig_scr_sf = {
    .rst_ctl = &g_rstgen_saf,
    .id      = RSTSIG_SAF_SCR_SF,
};

reset_sig_t rstsig_uart = {
    .rst_ctl = &g_rstgen_saf,
    .id      = RSTSIG_SAF_UART,
};

reset_sig_t rstsig_spi = {
    .rst_ctl = &g_rstgen_saf,
    .id      = RSTSIG_SAF_SPI,
};

reset_sig_t rstsig_i2c = {
    .rst_ctl = &g_rstgen_saf,
    .id      = RSTSIG_SAF_I2C,
};

reset_sig_t rstsig_wdt5_6 = {
    .rst_ctl = &g_rstgen_saf,
    .id      = RSTSIG_SAF_WDT5_6,
};

reset_sig_t rstsig_wdt3_4 = {
    .rst_ctl = &g_rstgen_saf,
    .id      = RSTSIG_SAF_WDT3_4,
};

reset_sig_t rstsig_canfd1 = {
    .rst_ctl = &g_rstgen_saf,
    .id      = RSTSIG_SAF_CANFD1,
};

reset_sig_t rstsig_canfd2 = {
    .rst_ctl = &g_rstgen_saf,
    .id      = RSTSIG_SAF_CANFD2,
};

reset_sig_t rstsig_canfd3_4 = {
    .rst_ctl = &g_rstgen_saf,
    .id      = RSTSIG_SAF_CANFD3_4,
};

reset_sig_t rstsig_canfd5_8 = {
    .rst_ctl = &g_rstgen_saf,
    .id      = RSTSIG_SAF_CANFD5_8,
};

reset_sig_t rstsig_canfd9_16 = {
    .rst_ctl = &g_rstgen_saf,
    .id      = RSTSIG_SAF_CANFD9_16,
};

reset_sig_t rstsig_canfd17_24 = {
    .rst_ctl = &g_rstgen_saf,
    .id      = RSTSIG_SAF_CANFD17_24,
};

reset_sig_t rstsig_xspi1a = {
    .rst_ctl = &g_rstgen_saf,
    .id      = RSTSIG_SAF_XSPI1A,
};

reset_sig_t rstsig_xspi1b = {
    .rst_ctl = &g_rstgen_saf,
    .id      = RSTSIG_SAF_XSPI1B,
};

reset_sig_t rstsig_xspi2a = {
    .rst_ctl = &g_rstgen_saf,
    .id      = RSTSIG_SAF_XSPI2A,
};

reset_sig_t rstsig_xspi2b = {
    .rst_ctl = &g_rstgen_saf,
    .id      = RSTSIG_SAF_XSPI2B,
};

reset_sig_t rstsig_dma_rst0 = {
    .rst_ctl = &g_rstgen_saf,
    .id      = RSTSIG_SAF_DMA_RST0,
};

reset_sig_t rstsig_dma_rst1 = {
    .rst_ctl = &g_rstgen_saf,
    .id      = RSTSIG_SAF_DMA_RST1,
};

reset_sig_t rstsig_ahb_syncup_gama1 = {
    .rst_ctl = &g_rstgen_saf,
    .id      = RSTSIG_SAF_AHB_SYNCUP_GAMA1,
};

reset_sig_t rstsig_enet1 = {
    .rst_ctl = &g_rstgen_saf,
    .id      = RSTSIG_SAF_ENET1,
};

reset_sig_t rstsig_enet2 = {
    .rst_ctl = &g_rstgen_saf,
    .id      = RSTSIG_SAF_ENET2,
};

reset_sig_t rstsig_gama1 = {
    .rst_ctl = &g_rstgen_saf,
    .id      = RSTSIG_SAF_GAMA1,
};

reset_sig_t rstsig_gama2 = {
    .rst_ctl = &g_rstgen_saf,
    .id      = RSTSIG_SAF_GAMA2,
};

reset_sig_t rstsig_ahb_syncup_gama2 = {
    .rst_ctl = &g_rstgen_saf,
    .id      = RSTSIG_SAF_AHB_SYNCUP_GAMA2,
};

reset_sig_t rstsig_vic1 = {
    .rst_ctl = &g_rstgen_saf,
    .id      = RSTSIG_SAF_VIC1,
};

reset_sig_t rstsig_vic2_porta = {
    .rst_ctl = &g_rstgen_saf,
    .id      = RSTSIG_SAF_VIC2_PORTA,
};

reset_sig_t rstsig_vic2_portb = {
    .rst_ctl = &g_rstgen_saf,
    .id      = RSTSIG_SAF_VIC2_PORTB,
};

reset_sig_t rstsig_vic3_porta = {
    .rst_ctl = &g_rstgen_saf,
    .id      = RSTSIG_SAF_VIC3_PORTA,
};

reset_sig_t rstsig_vic3_portb = {
    .rst_ctl = &g_rstgen_saf,
    .id      = RSTSIG_SAF_VIC3_PORTB,
};

reset_sig_t rstsig_xspi_slv = {
    .rst_ctl = &g_rstgen_saf,
    .id      = RSTSIG_SAF_XSPI_SLV,
};

reset_sig_t rstsig_mb = {
    .rst_ctl = &g_rstgen_saf,
    .id      = RSTSIG_SAF_MB,
};

reset_sig_t rstsig_xtrg = {
    .rst_ctl = &g_rstgen_saf,
    .id      = RSTSIG_SAF_XTRG,
};

reset_sig_t rstsig_disp_mux = {
    .rst_ctl = &g_rstgen_ap,
    .id      = RSTSIG_AP_DISP_MUX,
};

reset_sig_t rstsig_gpio_ap = {
    .rst_ctl = &g_rstgen_ap,
    .id      = RSTSIG_AP_GPIO_AP,
};

reset_sig_t rstsig_wdt8 = {
    .rst_ctl = &g_rstgen_ap,
    .id      = RSTSIG_AP_WDT8,
};

reset_sig_t rstsig_scr_ap = {
    .rst_ctl = &g_rstgen_ap,
    .id      = RSTSIG_AP_SCR_AP,
};

reset_sig_t rstsig_csi = {
    .rst_ctl = &g_rstgen_ap,
    .id      = RSTSIG_AP_CSI,
};

reset_sig_t rstsig_dc = {
    .rst_ctl = &g_rstgen_ap,
    .id      = RSTSIG_AP_DC,
};

reset_sig_t rstsig_g2d = {
    .rst_ctl = &g_rstgen_ap,
    .id      = RSTSIG_AP_G2D,
};

reset_sig_t rstsig_sdramc = {
    .rst_ctl = &g_rstgen_ap,
    .id      = RSTSIG_AP_SDRAMC,
};

reset_sig_t rstsig_saci1 = {
    .rst_ctl = &g_rstgen_ap,
    .id      = RSTSIG_AP_SACI1,
};

reset_sig_t rstsig_saci2 = {
    .rst_ctl = &g_rstgen_ap,
    .id      = RSTSIG_AP_SACI2,
};

reset_sig_t rstsig_dma_ap = {
    .rst_ctl = &g_rstgen_ap,
    .id      = RSTSIG_AP_DMA_AP,
};

reset_sig_t rstsig_sehc1 = {
    .rst_ctl = &g_rstgen_ap,
    .id      = RSTSIG_AP_SEHC1,
};

reset_sig_t rstsig_sehc2 = {
    .rst_ctl = &g_rstgen_ap,
    .id      = RSTSIG_AP_SEHC2,
};

reset_sig_t rstsig_usb = {
    .rst_ctl = &g_rstgen_ap,
    .id      = RSTSIG_AP_USB,
};

reset_sig_t rstsig_seip = {
    .rst_ctl = &g_rstgen_ap,
    .id      = RSTSIG_AP_SEIP,
};

reset_sig_t rstsig_lvds_ss = {
    .rst_ctl = &g_rstgen_ap,
    .id      = RSTSIG_AP_LVDS_SS,
};

static reset_sig_t *board_reset_array[] = {
    &rstsig_xspi1a,
    &rstsig_vic1,
};

void board_reset_init(void)
{

    taishan_reset_init(&g_rstgen_saf, &taishan_rstgen_sf_dev);

    taishan_reset_init(&g_rstgen_ap, &taishan_rstgen_ap_dev);

    for (uint32_t i = 0; i < ARRAY_SIZE(board_reset_array); i++) {
        reset_ctl_reset(board_reset_array[i]);
    }
}
#endif