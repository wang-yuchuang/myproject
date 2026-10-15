
//*****************************************************************************
//
// WARNING: Automatically generated file, don't modify anymore!!!
//
// Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
// Software License Agreement
//
//*****************************************************************************

#ifndef BOARDS_RSTGEN_CFG_H
#define BOARDS_RSTGEN_CFG_H

#ifdef CONFIG_RESETGEN

#include <driver/reset.h>


extern reset_glb_ctl_t rstctl_glb;
extern reset_sig_t rstsig_rstgen_ap;
extern reset_sig_t rstsig_cr5_saf;
extern reset_sig_t rstsig_cr5_sp0;
extern reset_sig_t rstsig_cr5_sp1;
extern reset_sig_t rstsig_cr5_sx0;
extern reset_sig_t rstsig_cr5_sx1;
extern reset_sig_t rstsig_mac;
extern reset_sig_t rstsig_scr_boot;
extern reset_sig_t rstsig_gpio_sf;
extern reset_sig_t rstsig_wdt1_2;
extern reset_sig_t rstsig_sem;
extern reset_sig_t rstsig_ana_sf;
extern reset_sig_t rstsig_scr_sf;
extern reset_sig_t rstsig_uart;
extern reset_sig_t rstsig_spi;
extern reset_sig_t rstsig_i2c;
extern reset_sig_t rstsig_wdt5_6;
extern reset_sig_t rstsig_wdt3_4;
extern reset_sig_t rstsig_canfd1;
extern reset_sig_t rstsig_canfd2;
extern reset_sig_t rstsig_canfd3_4;
extern reset_sig_t rstsig_canfd5_8;
extern reset_sig_t rstsig_canfd9_16;
extern reset_sig_t rstsig_canfd17_24;
extern reset_sig_t rstsig_xspi1a;
extern reset_sig_t rstsig_xspi1b;
extern reset_sig_t rstsig_xspi2a;
extern reset_sig_t rstsig_xspi2b;
extern reset_sig_t rstsig_dma_rst0;
extern reset_sig_t rstsig_dma_rst1;
extern reset_sig_t rstsig_ahb_syncup_gama1;
extern reset_sig_t rstsig_enet1;
extern reset_sig_t rstsig_enet2;
extern reset_sig_t rstsig_gama1;
extern reset_sig_t rstsig_gama2;
extern reset_sig_t rstsig_ahb_syncup_gama2;
extern reset_sig_t rstsig_vic1;
extern reset_sig_t rstsig_vic2_porta;
extern reset_sig_t rstsig_vic2_portb;
extern reset_sig_t rstsig_vic3_porta;
extern reset_sig_t rstsig_vic3_portb;
extern reset_sig_t rstsig_xspi_slv;
extern reset_sig_t rstsig_mb;
extern reset_sig_t rstsig_xtrg;
extern reset_sig_t rstsig_disp_mux;
extern reset_sig_t rstsig_gpio_ap;
extern reset_sig_t rstsig_wdt8;
extern reset_sig_t rstsig_scr_ap;
extern reset_sig_t rstsig_csi;
extern reset_sig_t rstsig_dc;
extern reset_sig_t rstsig_g2d;
extern reset_sig_t rstsig_sdramc;
extern reset_sig_t rstsig_saci1;
extern reset_sig_t rstsig_saci2;
extern reset_sig_t rstsig_dma_ap;
extern reset_sig_t rstsig_sehc1;
extern reset_sig_t rstsig_sehc2;
extern reset_sig_t rstsig_usb;
extern reset_sig_t rstsig_seip;
extern reset_sig_t rstsig_lvds_ss;

extern reset_sig_t rstsig_saf_mission6;
extern reset_sig_t rstsig_ap_mission1;
extern reset_sig_t rstsig_saf_debug;
extern reset_sig_t rstsig_ap_debug;

extern reset_general_reg_t reset_general_reg_sf_boot;


extern void board_reset_init(void);
extern reset_general_reg_t reset_general_reg_rom_ctrl;
extern reset_general_reg_t reset_general_reg_sf_remap;
#endif

#endif /* BOARDS_RSTGEN_CFG_H */
