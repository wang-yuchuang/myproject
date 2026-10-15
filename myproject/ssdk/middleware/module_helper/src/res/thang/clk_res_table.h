#ifndef CONFIG_MP
#define PLL_RES_ITEMS   \
    CLK_ITEM_PLL(CLK_ID_PLL1, 0, "PLL1", &g_pll1_vco), \
    CLK_ITEM_PLL(CLK_ID_PLL2, 0, "PLL2", &g_pll2_vco), \
    CLK_ITEM_PLL(CLK_ID_PLL3, 0, "PLL3", &g_pll3_vco), \
    CLK_ITEM_PLL(CLK_ID_PLL4, 0, "PLL4", &g_pll4_vco), \
    CLK_ITEM_PLL(CLK_ID_PLL5, 0, "PLL5", &g_pll5_vco), \
    CLK_ITEM_PLL(CLK_ID_PLL6, 0, "PLL6", &g_pll6_vco), \
    CLK_ITEM_PLL(CLK_ID_PLL7, 0, "PLL7", &g_pll7_vco), \
    CLK_ITEM_PLL_ABCD(CLK_ID_PLL1, 0, "PLL1", &g_pll1),   \
    CLK_ITEM_PLL_ABCD(CLK_ID_PLL2, 0, "PLL2", &g_pll2),   \
    CLK_ITEM_PLL_ABCD(CLK_ID_PLL3, 0, "PLL3", &g_pll3),   \
    CLK_ITEM_PLL_ABCD(CLK_ID_PLL4, 0, "PLL4", &g_pll4),   \
    CLK_ITEM_PLL_ABCD(CLK_ID_PLL5, 0, "PLL5", &g_pll5),    \
    CLK_ITEM_PLL_ABCD(CLK_ID_PLL6, 0, "PLL6", &g_pll6),   \
    CLK_ITEM_PLL_ABCD(CLK_ID_PLL7, 0, "PLL7", &g_pll7),   \
    CLK_ITEM_PLL(CLK_ID_PLL_DISP1, 0, "PLL_DISP1", &g_pll_disp1_vco), \
    CLK_ITEM_PLL(CLK_ID_PLL_DISP2, 0, "PLL_DISP2", &g_pll_disp2_vco), \
    CLK_ITEM_PLL(CLK_ID_PLL_LVDS1, 0, "PLL_LVDS1", &g_pll_lvds1_vco), \
    CLK_ITEM_PLL(CLK_ID_PLL_LVDS2, 0, "PLL_LVDS2", &g_pll_lvds2_vco), \
    CLK_ITEM_PLL(CLK_ID_PLL_LVDS3, 0, "PLL_LVDS3", &g_pll_lvds3_vco), \
    CLK_ITEM_PLL(CLK_ID_PLL_LVDS4, 0, "PLL_LVDS4", &g_pll_lvds4_vco), \
    CLK_ITEM_PLL(CLK_ID_PLL_CPU1A, 0, "PLL_CPU1A", &g_pll_cpu1a_vco), \
    CLK_ITEM_PLL(CLK_ID_PLL_CPU1B, 0, "PLL_CPU1B", &g_pll_cpu1b_vco), \
    CLK_ITEM_PLL(CLK_ID_PLL_CPU2A, 0, "PLL_CPU2A", &g_pll_cpu2A_vco), \
    CLK_ITEM_PLL(CLK_ID_PLL_CPU2B, 0, "PLL_CPU2B", &g_pll_cpu2B_vco), \
    CLK_ITEM_PLL(CLK_ID_PLL_GPU1, 0, "PLL_GPU1", &g_pll_gpu1_vco), \
    CLK_ITEM_PLL(CLK_ID_PLL_GPU2, 0, "PLL_GPU2", &g_pll_gpu2_vco), \
    CLK_ITEM_PLL(CLK_ID_PLL_VPU1, 0, "PLL_VPU1", &g_pll_vpu1_vco), \
    CLK_ITEM_PLL(CLK_ID_PLL_VPU2, 0, "PLL_VPU2", &g_pll_vpu2_vco), \
    CLK_ITEM_PLL(CLK_ID_PLL_VSN, 0, "PLL_VSN", &g_pll_vsn_vco), \
    CLK_ITEM_PLL(CLK_ID_PLL_HPIW, 0, "PLL_HPIW", &g_pll_hpiw_vco), \
    CLK_ITEM_PLL(CLK_ID_PLL_HPIE, 0, "PLL_HPIE", &g_pll_hpie_vco), \
    CLK_ITEM_PLL(CLK_ID_PLL_HIS, 0, "PLL_HIS", &g_pll_his_vco), \
    CLK_ITEM_PLL(CLK_ID_PLL_DDR, 0, "PLL_DDR", &g_pll_ddr_vco), \
    CLK_ITEM_PLL_AB(CLK_ID_PLL_DISP1, 0, "PLL_DISP1", &g_pll_disp1), \
    CLK_ITEM_PLL_AB(CLK_ID_PLL_DISP2, 0, "PLL_DISP2", &g_pll_disp2), \
    CLK_ITEM_PLL_AB(CLK_ID_PLL_LVDS1, 0, "PLL_LVDS1", &g_pll_lvds1),  \
    CLK_ITEM_PLL_AB(CLK_ID_PLL_LVDS2, 0, "PLL_LVDS2", &g_pll_lvds2),  \
    CLK_ITEM_PLL_AB(CLK_ID_PLL_LVDS3, 0, "PLL_LVDS3", &g_pll_lvds3),  \
    CLK_ITEM_PLL_AB(CLK_ID_PLL_LVDS4, 0, "PLL_LVDS4", &g_pll_lvds4),  \
    CLK_ITEM_PLL_AB(CLK_ID_PLL_CPU1A, 0, "PLL_CPU1A", &g_pll_cpu1a),  \
    CLK_ITEM_PLL_AB(CLK_ID_PLL_CPU1B, 0, "PLL_CPU1B", &g_pll_cpu1b),  \
    CLK_ITEM_PLL_AB(CLK_ID_PLL_CPU2A, 0, "PLL_CPU2A", &g_pll_cpu2a), \
    CLK_ITEM_PLL_AB(CLK_ID_PLL_CPU2B, 0, "PLL_CPU2B", &g_pll_cpu2b), \
    CLK_ITEM_PLL_AB(CLK_ID_PLL_GPU1, 0, "PLL_GPU1", &g_pll_gpu1), \
    CLK_ITEM_PLL_AB(CLK_ID_PLL_GPU2, 0, "PLL_GPU2", &g_pll_gpu2), \
    CLK_ITEM_PLL_AB(CLK_ID_PLL_VPU1, 0, "PLL_VPU1", &g_pll_vpu1),    \
    CLK_ITEM_PLL_AB(CLK_ID_PLL_VPU2, 0, "PLL_VPU2", &g_pll_vpu2),    \
    CLK_ITEM_PLL_AB(CLK_ID_PLL_VSN, 0, "PLL_VSN", &g_pll_vsn),   \
    CLK_ITEM_PLL_AB(CLK_ID_PLL_HPIW, 0, "PLL_HPIW", &g_pll_hpiw),    \
    CLK_ITEM_PLL_AB(CLK_ID_PLL_HPIE, 0, "PLL_HPIE", &g_pll_hpie),    \
    CLK_ITEM_PLL_AB(CLK_ID_PLL_HIS, 0, "PLL_HIS", &g_pll_his),    \
    CLK_ITEM_PLL_AB(CLK_ID_PLL_DDR, 0, "PLL_DDR", &g_pll_ddr),
#else
#define PLL_RES_ITEMS   \
    CLK_ITEM_PLL(CLK_ID_PLL3, 0, "PLL3", &g_pll3_vco), \
    CLK_ITEM_PLL(CLK_ID_PLL4, 0, "PLL4", &g_pll4_vco), \
    CLK_ITEM_PLL(CLK_ID_PLL5, 0, "PLL5", &g_pll5_vco), \
    CLK_ITEM_PLL(CLK_ID_PLL6, 0, "PLL6", &g_pll6_vco), \
    CLK_ITEM_PLL(CLK_ID_PLL7, 0, "PLL7", &g_pll7_vco), \
    CLK_ITEM_PLL_ABCD(CLK_ID_PLL3, 0, "PLL3", &g_pll3),   \
    CLK_ITEM_PLL_ABCD(CLK_ID_PLL4, 0, "PLL4", &g_pll4),   \
    CLK_ITEM_PLL_ABCD(CLK_ID_PLL5, 0, "PLL5", &g_pll5),    \
    CLK_ITEM_PLL_ABCD(CLK_ID_PLL6, 0, "PLL6", &g_pll6),   \
    CLK_ITEM_PLL_ABCD(CLK_ID_PLL7, 0, "PLL7", &g_pll7),   \
    CLK_ITEM_PLL(CLK_ID_PLL_DISP1, 0, "PLL_DISP1", &g_pll_disp1_vco), \
    CLK_ITEM_PLL(CLK_ID_PLL_DISP2, 0, "PLL_DISP2", &g_pll_disp2_vco), \
    CLK_ITEM_PLL(CLK_ID_PLL_LVDS1, 0, "PLL_LVDS1", &g_pll_lvds1_vco), \
    CLK_ITEM_PLL(CLK_ID_PLL_LVDS2, 0, "PLL_LVDS2", &g_pll_lvds2_vco), \
    CLK_ITEM_PLL(CLK_ID_PLL_LVDS3, 0, "PLL_LVDS3", &g_pll_lvds3_vco), \
    CLK_ITEM_PLL(CLK_ID_PLL_LVDS4, 0, "PLL_LVDS4", &g_pll_lvds4_vco), \
    CLK_ITEM_PLL(CLK_ID_PLL_CPU1A, 0, "PLL_CPU1A", &g_pll_cpu1a_vco), \
    CLK_ITEM_PLL(CLK_ID_PLL_CPU1B, 0, "PLL_CPU1B", &g_pll_cpu1b_vco), \
    CLK_ITEM_PLL(CLK_ID_PLL_GPU1, 0, "PLL_GPU1", &g_pll_gpu1_vco), \
    CLK_ITEM_PLL(CLK_ID_PLL_GPU2, 0, "PLL_GPU2", &g_pll_gpu2_vco), \
    CLK_ITEM_PLL(CLK_ID_PLL_VPU1, 0, "PLL_VPU2", &g_pll_vpu1_vco), \
    CLK_ITEM_PLL(CLK_ID_PLL_VPU2, 0, "PLL_VPU1", &g_pll_vpu2_vco), \
    CLK_ITEM_PLL(CLK_ID_PLL_VSN, 0, "PLL_VSN", &g_pll_vsn_vco), \
    CLK_ITEM_PLL(CLK_ID_PLL_HPIW, 0, "PLL_HPIW", &g_pll_hpiw_vco), \
    CLK_ITEM_PLL(CLK_ID_PLL_HPIE, 0, "PLL_HPIE", &g_pll_hpie_vco), \
    CLK_ITEM_PLL(CLK_ID_PLL_HIS, 0, "PLL_HIS", &g_pll_his_vco), \
    CLK_ITEM_PLL(CLK_ID_PLL_DDR, 0, "PLL_DDR", &g_pll_ddr_vco), \
    CLK_ITEM_PLL_AB(CLK_ID_PLL_DISP1, 0, "PLL_DISP1", &g_pll_disp1), \
    CLK_ITEM_PLL_AB(CLK_ID_PLL_DISP2, 0, "PLL_DISP2", &g_pll_disp2), \
    CLK_ITEM_PLL_AB(CLK_ID_PLL_LVDS1, 0, "PLL_LVDS1", &g_pll_lvds1),  \
    CLK_ITEM_PLL_AB(CLK_ID_PLL_LVDS2, 0, "PLL_LVDS2", &g_pll_lvds2),  \
    CLK_ITEM_PLL_AB(CLK_ID_PLL_LVDS3, 0, "PLL_LVDS3", &g_pll_lvds3),  \
    CLK_ITEM_PLL_AB(CLK_ID_PLL_LVDS4, 0, "PLL_LVDS4", &g_pll_lvds4),  \
    CLK_ITEM_PLL_AB(CLK_ID_PLL_CPU1A, 0, "PLL_CPU1A", &g_pll_cpu1a),  \
    CLK_ITEM_PLL_AB(CLK_ID_PLL_CPU1B, 0, "PLL_CPU1B", &g_pll_cpu1b),  \
    CLK_ITEM_PLL_AB(CLK_ID_PLL_GPU1, 0, "PLL_GPU1", &g_pll_gpu1), \
    CLK_ITEM_PLL_AB(CLK_ID_PLL_GPU2, 0, "PLL_GPU2", &g_pll_gpu2), \
    CLK_ITEM_PLL_AB(CLK_ID_PLL_VPU1, 0, "PLL_VPU1", &g_pll_vpu1),    \
    CLK_ITEM_PLL_AB(CLK_ID_PLL_VPU2, 0, "PLL_VPU2", &g_pll_vpu2),    \
    CLK_ITEM_PLL_AB(CLK_ID_PLL_VSN, 0, "PLL_VSN", &g_pll_vsn),   \
    CLK_ITEM_PLL_AB(CLK_ID_PLL_HPIW, 0, "PLL_HPIW", &g_pll_hpiw),    \
    CLK_ITEM_PLL_AB(CLK_ID_PLL_HPIE, 0, "PLL_HPIE", &g_pll_hpie),    \
    CLK_ITEM_PLL_AB(CLK_ID_PLL_HIS, 0, "PLL_HIS", &g_pll_his),    \
    CLK_ITEM_PLL_AB(CLK_ID_PLL_DDR, 0, "PLL_DDR", &g_pll_ddr),
#endif

    // CLK_ITEM_WITH_TABLE_AND_GATE(CLK_ID_CE2, 0, "CE2", &g_ckgen_ip_ce2, &g_ckgen_gating_ce2),
#define CKGEN_SEC_ITEMS \
    CLK_ITEM_WITH_TABLE_AND_GATE(CLK_ID_SEC_PLAT, 0,"SEC_PLAT", &g_ckgen_bus_sec_plat_root, &g_ckgen_gating_cr5_sec),   \
    CLK_ITEM_WITH_TABLE_AND_GATE(CLK_ID_MP_PLAT, 0,"MP_PLAT", &g_ckgen_core_mp_plat, &g_ckgen_gating_cr5_mp), \
    CLK_ITEM_WITH_TABLE_AND_GATE(CLK_ID_I2C_SEC0, 0, "I2C_SEC0", &g_ckgen_ip_i2c_sec0, &g_ckgen_gating_i2c5),  \
    CLK_ITEM_WITH_TABLE_AND_GATE(CLK_ID_I2C_SEC1, 0, "I2C_SEC1", &g_ckgen_ip_i2c_sec1, &g_ckgen_gating_i2c5),  \
    CLK_ITEM_WITH_TABLE_AND_GATE(CLK_ID_SPI_SEC0, 0, "SPI_SEC0", &g_ckgen_ip_spi_sec0, &g_ckgen_gating_spi5),  \
    CLK_ITEM_WITH_TABLE_AND_GATE(CLK_ID_SPI_SEC1, 0, "SPI_SEC1", &g_ckgen_ip_spi_sec1, &g_ckgen_gating_spi5),  \
    CLK_ITEM_WITH_TABLE_AND_GATE(CLK_ID_UART_SEC0, 0, "UART_SEC0", &g_ckgen_ip_uart_sec0, &g_ckgen_gating_uart10),   \
    CLK_ITEM_WITH_TABLE_AND_GATE(CLK_ID_UART_SEC1, 0, "UART_SEC1", &g_ckgen_ip_uart_sec1, &g_ckgen_gating_uart10),   \
    CLK_ITEM_WITH_TABLE_AND_GATE(CLK_ID_EMMC1, 0, "EMMC1", &g_ckgen_ip_emmc1, &g_ckgen_gating_mshc1),   \
    CLK_ITEM_WITH_TABLE_AND_GATE(CLK_ID_EMMC2, 0, "EMMC2", &g_ckgen_ip_emmc2, &g_ckgen_gating_mshc2),   \
    CLK_ITEM_WITH_TABLE_AND_GATE(CLK_ID_EMMC3, 0, "EMMC3", &g_ckgen_ip_emmc3, &g_ckgen_gating_mshc3),   \
    CLK_ITEM_WITH_TABLE_AND_GATE(CLK_ID_EMMC4, 0, "EMMC4", &g_ckgen_ip_emmc4, &g_ckgen_gating_mshc4),   \
    CLK_ITEM_WITH_TABLE_AND_GATE(CLK_ID_ETH2_TX, 0, "ETH2_TX", &g_ckgen_ip_enet2_tx, &g_ckgen_gating_enet2), \
    CLK_ITEM_WITH_TABLE_AND_GATE(CLK_ID_ETH2_RMII, 0, "ETH2_RMII", &g_ckgen_ip_enet2_rmii, &g_ckgen_gating_enet2),   \
    CLK_ITEM_WITH_TABLE_AND_GATE(CLK_ID_ETH2_PHY_REF, 0, "ETH2_PHY_REF", &g_ckgen_ip_enet2_phy_ref, &g_ckgen_gating_enet2),  \
    CLK_ITEM_WITH_TABLE_AND_GATE(CLK_ID_ETH2_TIMER_SEC, 0, "ETH2_TIMER", &g_ckgen_ip_enet2_timer_sec, &g_ckgen_gating_enet2),  \
    CLK_ITEM_WITH_TABLE_AND_GATE(CLK_ID_SPDIF1, 0, "SPDIF1", &g_ckgen_ip_spdif1, &g_ckgen_gating_spdif1),   \
    CLK_ITEM_WITH_TABLE_AND_GATE(CLK_ID_SPDIF2, 0, "SPDIF2", &g_ckgen_ip_spdif2, &g_ckgen_gating_spdif2),   \
    CLK_ITEM_WITH_TABLE_AND_GATE(CLK_ID_SPDIF3, 0, "SPDIF3", &g_ckgen_ip_spdif3, &g_ckgen_gating_spdif3),   \
    CLK_ITEM_WITH_TABLE_AND_GATE(CLK_ID_SPDIF4, 0, "SPDIF4", &g_ckgen_ip_spdif4, &g_ckgen_gating_spdif4),   \
    CLK_ITEM_WITH_TABLE_AND_GATE(CLK_ID_OSPI2, 0, "OSPI2", &g_ckgen_ip_ospi2, &g_ckgen_gating_ospi2),    \
    CLK_ITEM_WITH_TABLE_AND_GATE(CLK_ID_TIMER3, 0, "TIMER3", &g_ckgen_ip_timer3, &g_ckgen_gating_timer3),   \
    CLK_ITEM_WITH_TABLE_AND_GATE(CLK_ID_TIMER4, 0, "TIMER4", &g_ckgen_ip_timer4, &g_ckgen_gating_timer4),   \
    CLK_ITEM_WITH_TABLE_AND_GATE(CLK_ID_TIMER5, 0, "TIMER5", &g_ckgen_ip_timer5, &g_ckgen_gating_timer5),   \
    CLK_ITEM_WITH_TABLE_AND_GATE(CLK_ID_TIMER6, 0, "TIMER6", &g_ckgen_ip_timer6, &g_ckgen_gating_timer6),   \
    CLK_ITEM_WITH_TABLE_AND_GATE(CLK_ID_TIMER7, 0, "TIMER7", &g_ckgen_ip_timer7, &g_ckgen_gating_timer7),   \
    CLK_ITEM_WITH_TABLE_AND_GATE(CLK_ID_TIMER8, 0, "TIMER8", &g_ckgen_ip_timer8, &g_ckgen_gating_timer8),   \
    CLK_ITEM_WITH_TABLE_AND_GATE(CLK_ID_PWM3, 0, "PWM3", &g_ckgen_ip_pwm3, &g_ckgen_gating_pwm3), \
    CLK_ITEM_WITH_TABLE_AND_GATE(CLK_ID_PWM4, 0, "PWM4", &g_ckgen_ip_pwm4, &g_ckgen_gating_pwm4), \
    CLK_ITEM_WITH_TABLE_AND_GATE(CLK_ID_PWM5, 0, "PWM5", &g_ckgen_ip_pwm5, &g_ckgen_gating_pwm5), \
    CLK_ITEM_WITH_TABLE_AND_GATE(CLK_ID_PWM6, 0, "PWM6", &g_ckgen_ip_pwm6, &g_ckgen_gating_pwm6), \
    CLK_ITEM_WITH_TABLE_AND_GATE(CLK_ID_PWM7, 0, "PWM7", &g_ckgen_ip_pwm7, &g_ckgen_gating_pwm7), \
    CLK_ITEM_WITH_TABLE_AND_GATE(CLK_ID_PWM8, 0, "PWM8", &g_ckgen_ip_pwm8, &g_ckgen_gating_pwm8), \
    CLK_ITEM_WITH_TABLE_AND_GATE(CLK_ID_I2S_MCLK2, 0, "I2S_MCLK2", &g_ckgen_ip_i2s_mclk2, NULL),  \
    CLK_ITEM_WITH_TABLE_AND_GATE(CLK_ID_I2S_MC1, 0, "I2S_MC1", &g_ckgen_ip_i2s_mc1, &g_ckgen_gating_i2s_mc1),    \
    CLK_ITEM_WITH_TABLE_AND_GATE(CLK_ID_I2S_SC3, 0, "I2S_SC3", &g_ckgen_ip_i2s_sc3, &g_ckgen_gating_i2s_sc3),    \
    CLK_ITEM_WITH_TABLE_AND_GATE(CLK_ID_I2S_MCLK3, 0, "I2S_MCLK3", &g_ckgen_ip_i2s_mclk3, NULL),  \
    CLK_ITEM_WITH_TABLE_AND_GATE(CLK_ID_I2S_MC2, 0, "I2S_MC2", &g_ckgen_ip_i2s_mc2, &g_ckgen_gating_i2s_mc2),    \
    CLK_ITEM_WITH_TABLE_AND_GATE(CLK_ID_I2S_SC5, 0, "I2S_SC5", &g_ckgen_ip_i2s_sc5, &g_ckgen_gating_i2s_sc5),    \
    CLK_ITEM_WITH_TABLE_AND_GATE(CLK_ID_I2S_SC4, 0, "I2S_SC4", &g_ckgen_ip_i2s_sc4, &g_ckgen_gating_i2s_sc4),  \
    CLK_ITEM_WITH_TABLE_AND_GATE(CLK_ID_I2S_SC7, 0, "I2S_SC7", &g_ckgen_ip_i2s_sc7, &g_ckgen_gating_i2s_sc7),  \
    CLK_ITEM_WITH_TABLE_AND_GATE(CLK_ID_I2S_SC6, 0, "I2S_SC6", &g_ckgen_ip_i2s_sc6, &g_ckgen_gating_i2s_sc6),  \
    CLK_ITEM_WITH_TABLE_AND_GATE(CLK_ID_I2S_SC8, 0, "I2S_SC8", &g_ckgen_ip_i2s_sc8, &g_ckgen_gating_i2s_sc8),  \
    CLK_ITEM_WITH_TABLE_AND_GATE(CLK_ID_CSI_MCLK1, 0, "CSI_MCLK1", &g_ckgen_ip_csi_mclk1, &g_ckgen_gating_csi1_axiclk), \
    CLK_ITEM_WITH_TABLE_AND_GATE(CLK_ID_CSI_MCLK2, 0, "CSI_MCLK2", &g_ckgen_ip_csi_mclk2, &g_ckgen_gating_csi2_axiclk), \
    CLK_ITEM_WITH_TABLE_AND_GATE(CLK_ID_GIC4_GIC5, 0, "GIC4_GIC5", &g_ckgen_ip_gic_4_5, &g_ckgen_gating_gic2),    \
    CLK_ITEM_WITH_TABLE_AND_GATE(CLK_ID_TRACE, 0, "TRACE", &g_ckgen_ip_trace, &g_ckgen_gating_cssys_trace),    \
    CLK_ITEM_WITH_TABLE_AND_GATE(CLK_ID_SYS_CNT, 0, "SYS_CNT", &g_ckgen_ip_sys_cnt, &g_ckgen_gating_sys_cnt),  \
    CLK_ITEM_WITH_TABLE_AND_GATE(CLK_ID_MSHC_TIMER, 0, "MSHC_TIMER", &g_ckgen_ip_mshc_timer, NULL), \
    CLK_ITEM_WITH_TABLE_AND_GATE(CLK_ID_HPI_CLK600, 0, "HPI_CLK600", &g_ckgen_ip_hpi_clk600, NULL), \
    CLK_ITEM_WITH_TABLE_AND_GATE(CLK_ID_HPI_CLK800, 0, "HPI_CLK800", &g_ckgen_ip_hpi_clk800, NULL), \


#define CKGEN_DISP_ITEMS \
    CLK_ITEM_WITH_TABLE_AND_GATE(CLK_ID_MIPI_CSI1_PIX, 0, "MIPI_CSI1_PIX", &g_ckgen_ip_mipi_csi1_pix, &g_ckgen_gating_mipi_csi1_pclk),  \
    CLK_ITEM_WITH_TABLE_AND_GATE(CLK_ID_MIPI_CSI2_PIX, 0, "MIPI_CSI2_PIX", &g_ckgen_ip_mipi_csi2_pix, &g_ckgen_gating_mipi_csi2_pclk),  \
    CLK_ITEM_WITH_TABLE_AND_GATE(CLK_ID_DISP_BUS, 0, "DISP_BUS", &g_ckgen_bus_disp_bus_root, &g_ckgen_gating_disp_mux_pclk),    \
    CLK_ITEM_WITH_TABLE_AND_GATE(CLK_ID_DP1, 0, "DP1", &g_ckgen_ip_dp1, &g_ckgen_gating_dp1_axiclk),  \
    CLK_ITEM_WITH_TABLE_AND_GATE(CLK_ID_DP2, 0, "DP2", &g_ckgen_ip_dp2, &g_ckgen_gating_dp2_axiclk),  \
    CLK_ITEM_WITH_TABLE_AND_GATE(CLK_ID_DP3, 0, "DP3", &g_ckgen_ip_dp3, &g_ckgen_gating_dp3_axiclk),  \
    CLK_ITEM_WITH_TABLE_AND_GATE(CLK_ID_DC1, 0, "DC1", &g_ckgen_ip_dc1, &g_ckgen_gating_dc1_axiclk),  \
    CLK_ITEM_WITH_TABLE_AND_GATE(CLK_ID_DC2, 0, "DC2", &g_ckgen_ip_dc2, &g_ckgen_gating_dc2_axiclk),  \
    CLK_ITEM_WITH_TABLE_AND_GATE(CLK_ID_DC3, 0, "DC3", &g_ckgen_ip_dc3, &g_ckgen_gating_dc3_axiclk),  \
    CLK_ITEM_WITH_TABLE_AND_GATE(CLK_ID_DC4, 0, "DC4", &g_ckgen_ip_dc4, &g_ckgen_gating_dc4_axiclk),  \
    CLK_ITEM_WITH_TABLE_AND_GATE(CLK_ID_DC5, 0, "DC5", &g_ckgen_ip_dc5, &g_ckgen_gating_dc5_axiclk),  \
    CLK_ITEM_WITH_TABLE_AND_GATE(CLK_ID_EXT_AUD1, 0, "EXT_AUD1", &g_ckgen_ip_ext_aud1, NULL), \
    CLK_ITEM_WITH_TABLE_AND_GATE(CLK_ID_EXT_AUD2, 0, "EXT_AUD2", &g_ckgen_ip_ext_aud2, NULL), \
    CLK_ITEM_WITH_TABLE_AND_GATE(CLK_ID_EXT_AUD3, 0, "EXT_AUD3", &g_ckgen_ip_ext_aud3, NULL), \
    CLK_ITEM_WITH_TABLE_AND_GATE(CLK_ID_EXT_AUD4, 0, "EXT_AUD4", &g_ckgen_ip_ext_aud4, NULL), \


#define CKGEN_SAF_ITEMS \
    CLK_ITEM_WITH_TABLE_AND_GATE(CLK_ID_SAF_PLAT, 0, "SAF_PLAT", &g_ckgen_bus_saf_plat_root, NULL),  \
    CLK_ITEM_WITH_TABLE_AND_GATE(CLK_ID_CE1, 0, "CE1", &g_ckgen_ip_ce1, &g_ckgen_gating_ce1),  \
    CLK_ITEM_WITH_TABLE_AND_GATE(CLK_ID_I2C_SAF, 0, "I2C_SAF", &g_ckgen_ip_i2c_saf, &g_ckgen_gating_i2c1), \
    CLK_ITEM_WITH_TABLE_AND_GATE(CLK_ID_UART_SAF, 0, "UART_SAF", &g_ckgen_ip_uart_saf, &g_ckgen_gating_uart1),  \
    CLK_ITEM_WITH_TABLE_AND_GATE(CLK_ID_SPI_SAF, 0, "SPI_SAF", &g_ckgen_ip_spi_saf, &g_ckgen_gating_spi1),  \
    CLK_ITEM_WITH_TABLE_AND_GATE(CLK_ID_I2S_MCLK1, 0, "I2S_MCLK1", &g_ckgen_ip_i2s_mclk1, NULL),    \
    CLK_ITEM_WITH_TABLE_AND_GATE(CLK_ID_I2S_SC1, 0, "I2S_SC1", &g_ckgen_ip_i2s_sc1, &g_ckgen_gating_i2s_sc1),  \
    CLK_ITEM_WITH_TABLE_AND_GATE(CLK_ID_I2S_SC2, 0, "I2S_SC2", &g_ckgen_ip_i2s_sc2, &g_ckgen_gating_i2s_sc2),  \
    CLK_ITEM_WITH_TABLE_AND_GATE(CLK_ID_ETH1_TX, 0, "ETH1_TX", &g_ckgen_ip_eth1_tx, &g_ckgen_gating_eth1), \
    CLK_ITEM_WITH_TABLE_AND_GATE(CLK_ID_ETH1_RMII, 0, "ETH1_RMII", &g_ckgen_ip_eth1_rmii, &g_ckgen_gating_eth1),   \
    CLK_ITEM_WITH_TABLE_AND_GATE(CLK_ID_ETH1_PHY_REF, 0, "ETH1_PHY_REF", &g_ckgen_ip_eth1_phy_ref, &g_ckgen_gating_eth1),  \
    CLK_ITEM_WITH_TABLE_AND_GATE(CLK_ID_ETH1_TIMER_SEC, 0, "ETH1_TIMER_SEC", &g_ckgen_ip_eth1_timer_sec, &g_ckgen_gating_eth1),  \
    CLK_ITEM_WITH_TABLE_AND_GATE(CLK_ID_OSPI1, 0, "OSPI1", &g_ckgen_ip_ospi1, &g_ckgen_gating_ospi1),    \
    CLK_ITEM_WITH_TABLE_AND_GATE(CLK_ID_TIMER1, 0, "TIMER1", &g_ckgen_ip_timer1, &g_ckgen_gating_timer1),   \
    CLK_ITEM_WITH_TABLE_AND_GATE(CLK_ID_TIMER2, 0, "TIMER2", &g_ckgen_ip_timer2, &g_ckgen_gating_timer2),   \
    CLK_ITEM_WITH_TABLE_AND_GATE(CLK_ID_PWM1, 0, "PWM1", &g_ckgen_ip_pwm1, &g_ckgen_gating_pwm1), \
    CLK_ITEM_WITH_TABLE_AND_GATE(CLK_ID_PWM2, 0, "PWM2", &g_ckgen_ip_pwm2, &g_ckgen_gating_pwm2), \
    CLK_ITEM_WITH_TABLE_AND_GATE(CLK_ID_CAN1_TO_4, 0, "CAN1_TO_4", &g_ckgen_ip_can_1_to_4, &g_ckgen_gating_can1),   \


#define CKGEN_SOC_ITEMS \
    CLK_ITEM_WITH_TABLE_AND_GATE(CLK_ID_CPU1A, 0, "CPU1A", &g_ckgen_core_cpu1a, &g_ckgen_gating_pll_cpu1a_pclk),    \
    CLK_ITEM_WITH_TABLE_AND_GATE(CLK_ID_CPU1B, 0, "CPU1B", &g_ckgen_core_cpu1b, &g_ckgen_gating_pll_cpu1b_pclk),    \
    CLK_ITEM_WITH_TABLE_AND_GATE(CLK_ID_CPU2A, 0, "CPU2A", &g_ckgen_core_cpu2a, &g_ckgen_gating_pll_cpu2a_pclk),   \
    CLK_ITEM_WITH_TABLE_AND_GATE(CLK_ID_CPU2B, 0, "CPU2B", &g_ckgen_core_cpu2b, &g_ckgen_gating_pll_cpu2b_pclk),   \
    CLK_ITEM_WITH_TABLE_AND_GATE(CLK_ID_GPU1, 0, "GPU1", &g_ckgen_core_gpu1, &g_ckgen_gating_adb_gpu1_memif_aclk),   \
    CLK_ITEM_WITH_TABLE_AND_GATE(CLK_ID_GPU2, 0, "GPU2", &g_ckgen_core_gpu2, &g_ckgen_gating_adb_gpu2_memif_aclk),   \
    CLK_ITEM_WITH_TABLE_AND_GATE(CLK_ID_VPU1, 0, "VPU1", &g_ckgen_ip_vpu1, &g_ckgen_gating_vpu1_bclk), \
    CLK_ITEM_WITH_TABLE_AND_GATE(CLK_ID_VPU3, 0, "MJPEG", &g_ckgen_ip_vpu3, &g_ckgen_gating_vpu3_aclk),  \
    CLK_ITEM_WITH_TABLE_AND_GATE(CLK_ID_VPU_BUS, 0, "VPU_BUS", &g_ckgen_bus_vpu_bus_root, &g_ckgen_gating_xpu_vpu_bus_clk_ckgen),   \
    CLK_ITEM_WITH_TABLE_AND_GATE(CLK_ID_VSN_BUS, 0, "VSN_BUS", &g_ckgen_bus_vsn_bus_root, &g_ckgen_gating_bti_vsn_m12_axiclk),   \
    CLK_ITEM_WITH_TABLE_AND_GATE(CLK_ID_DDR, 0, "DDR", &g_ckgen_core_ddr, &g_ckgen_gating_xpu_ddr_clk_ckgen),  \
    CLK_ITEM_WITH_TABLE_AND_GATE(CLK_ID_HIS_BUS, 0, "HIS_BUS", &g_ckgen_bus_his_bus_root, &g_ckgen_gating_pll_his_pclk),   \

// CLK_ITEM_WITH_TABLE_AND_GATE(CLK_ID_NOC_BUS, 0, "NOC_BUS", &g_ckgen_bus_noce_main_root, &g_ckgen_gating_noc_mainb_bus),

/*CKGEN UUU*/
/*
sample
static int CLK_ID_CPU1A_uuu_sel0_tab[] = {
    CLK_ID_CPU1A, CLK_ID_PLL_CPU1A_ROOT
};
static int CLK_ID_CPU1A_uuu_mnpq_tab[] = {
    CLK_ID_CPU1A_SEL0
};
static int CLK_ID_CPU1A_0_tab[] = {//sel1
    CLK_ID_CPU1A_M, CLK_ID_PLL_CPU1A_ROOT
};
*/
#define UUU_ITEM_TABLE(_id, _pll_id)    \
    static int _id ##_uuu_sel0_tab[] = {    \
        _id, _pll_id    \
    };  \
    static int _id ##_uuu_mnpq_tab[] = {    \
        _id ##_SEL0 \
    };  \
    static int _id ##_0_tab[] = {/*sel1*/   \
        _id ##_M, _pll_id   \
    };

#define CKGEN_UUU_COMPOSITE_ITEM(_id, _resid, _name, _clk)    \
    CLK_UUU_ITEM_WITH_TABLE(_id ##_M, _resid, _name"M", _clk ##_div_m)


//@TODO some uuu clock disable clock m p q
// CLK_UUU_ITEM_WITH_TABLE(_id ##_1, _resid, _name"1", _clk ##_div_n),
// CLK_UUU_ITEM_WITH_TABLE(_id ##_2, _resid, _name"2", _clk ##_div_p),
// CLK_UUU_ITEM_WITH_TABLE(_id ##_3, _resid, _name"3", _clk ##_div_q)


#define CKGEN_UUU_ITEMS \
    CKGEN_UUU_COMPOSITE_ITEM(CLK_ID_CPU1A, 0, "CPU1A_", &g_ckgen_uuu_cpu1a), \
    CKGEN_UUU_COMPOSITE_ITEM(CLK_ID_CPU1B, 0, "CPU1B_", &g_ckgen_uuu_cpu1b), \
    CKGEN_UUU_COMPOSITE_ITEM(CLK_ID_CPU2A, 0, "CPU2A_", &g_ckgen_uuu_cpu2a), \
    CKGEN_UUU_COMPOSITE_ITEM(CLK_ID_CPU2B, 0, "CPU2B_", &g_ckgen_uuu_cpu2b), \
    CKGEN_UUU_COMPOSITE_ITEM(CLK_ID_GPU1, 0, "GPU1_",  &g_ckgen_uuu_gpu1), \
    CKGEN_UUU_COMPOSITE_ITEM(CLK_ID_GPU2, 0, "GPU2_", & g_ckgen_uuu_gpu2), \
    CKGEN_UUU_COMPOSITE_ITEM(CLK_ID_VPU1, 0, "VPU1_", &g_ckgen_uuu_vpu1), \
    CKGEN_UUU_COMPOSITE_ITEM(CLK_ID_VPU3, 0, "MJPEG_", &g_ckgen_uuu_vpu3), \
    CKGEN_UUU_COMPOSITE_ITEM(CLK_ID_VPU_BUS, 0, "VPU_BUS_", &g_ckgen_uuu_vpu_bus), \
    CKGEN_UUU_COMPOSITE_ITEM(CLK_ID_VSN_BUS, 0, "VSN_BUS_", &g_ckgen_uuu_vsn_bus), \
    CKGEN_UUU_COMPOSITE_ITEM(CLK_ID_DDR, 0, "DDR_", &g_ckgen_uuu_ddr), \
    CKGEN_UUU_COMPOSITE_ITEM(CLK_ID_HIS_BUS, 0, "HIS_BUS_", &g_ckgen_uuu_his_bus), \


#define CLK_UUU_RATIO_LIMIT_LIST \
    CLK_UUU_RATIO_LIMIT(CLK_ID_CPU1A, 1, 2, 4, 8) \
    CLK_UUU_RATIO_LIMIT(CLK_ID_CPU1B, 1, 2, 4, 8) \
    CLK_UUU_RATIO_LIMIT(CLK_ID_CPU2A, 1, 2, 4, 8) \
    CLK_UUU_RATIO_LIMIT(CLK_ID_CPU2B, 1, 2, 4, 8) \
    CLK_UUU_RATIO_LIMIT(CLK_ID_GPU1, 1, 1, 4, 1) \
    CLK_UUU_RATIO_LIMIT(CLK_ID_GPU2, 1, 1, 4, 1) \
    CLK_UUU_RATIO_LIMIT(CLK_ID_VPU_BUS, 1, 4, 1, 1) \
    CLK_UUU_RATIO_LIMIT(CLK_ID_VSN_BUS, 1, 4, 1, 1) \
    CLK_UUU_RATIO_LIMIT(CLK_ID_DDR, 1, 2, 4, 1) \
    CLK_UUU_RATIO_LIMIT(CLK_ID_HIS_BUS, 1, 10, 2, 4) \


#define SET_CLK_PLL_DEPENDS_AB_LIST \
    SET_CLK_PLL_DEPENDS_AB(CLK_ID_PLL_CPU1A, CLK_ID_CPU1A_2) \
    SET_CLK_PLL_DEPENDS_AB(CLK_ID_PLL_CPU1B, CLK_ID_CPU1A_2) \
    SET_CLK_PLL_DEPENDS_AB(CLK_ID_PLL_CPU2A, CLK_ID_CPU2A_2) \
    SET_CLK_PLL_DEPENDS_AB(CLK_ID_PLL_CPU2B, CLK_ID_CPU2B_2) \
    SET_CLK_PLL_DEPENDS_AB(CLK_ID_PLL_GPU1, CLK_ID_GPU1_2) \
    SET_CLK_PLL_DEPENDS_AB(CLK_ID_PLL_GPU2, CLK_ID_GPU2_2) \
    SET_CLK_PLL_DEPENDS_AB(CLK_ID_PLL_VPU1, CLK_ID_VPU_BUS_1) \
    SET_CLK_PLL_DEPENDS_AB(CLK_ID_PLL_VPU2, CLK_ID_VPU_BUS_2) \
    SET_CLK_PLL_DEPENDS_AB(CLK_ID_PLL_DDR, CLK_ID_DDR_2) \
    SET_CLK_PLL_DEPENDS_AB(CLK_ID_PLL_VSN, CLK_ID_VSN_BUS_1) \

#define SET_MOREPROCISE_LIST \
    clk_set_moreprecise(CLK_ID_PLL_DDR, true, 0); \
    clk_set_moreprecise(CLK_ID_PLL_DISP1, true, 0); \
    clk_set_moreprecise(CLK_ID_PLL_DISP2, true, 0); \
    clk_set_moreprecise(CLK_ID_PLL_LVDS1, true, 0); \
    clk_set_moreprecise(CLK_ID_PLL_LVDS2, true, 0); \
    clk_set_moreprecise(CLK_ID_PLL_LVDS3, true, 0); \
    clk_set_moreprecise(CLK_ID_PLL_LVDS4, true, 0); \
    clk_set_moreprecise(CLK_ID_PLL_CPU1A, true, 0); \
    clk_set_moreprecise(CLK_ID_PLL_CPU1B, true, 0); \
    clk_set_moreprecise(CLK_ID_PLL_CPU2A, true, 0); \
    clk_set_moreprecise(CLK_ID_PLL_CPU2B, true, 0); \
    clk_set_moreprecise(CLK_ID_PLL_GPU1, true, 0); \
    clk_set_moreprecise(CLK_ID_PLL_GPU2, true, 0); \
    clk_set_moreprecise(CLK_ID_PLL_VPU1, true, 0); \
    clk_set_moreprecise(CLK_ID_PLL_VPU2, true, 0); \
    clk_set_moreprecise(CLK_ID_PLL_VSN, true, 0); \
    clk_set_moreprecise(CLK_ID_PLL_HPIW, true, 0); \
    clk_set_moreprecise(CLK_ID_PLL_HPIE, true, 0); \
    clk_set_moreprecise(CLK_ID_PLL_HIS, true, 0); \
    clk_set_moreprecise(CLK_ID_PLL1, true, 0); \
    clk_set_moreprecise(CLK_ID_PLL2, true, 0); \
    clk_set_moreprecise(CLK_ID_PLL3, true, 0); \
    clk_set_moreprecise(CLK_ID_PLL4, true, 0); \
    clk_set_moreprecise(CLK_ID_PLL5, true, 0); \
    clk_set_moreprecise(CLK_ID_PLL6, true, 0); \
    clk_set_moreprecise(CLK_ID_PLL7, true, 0); \

