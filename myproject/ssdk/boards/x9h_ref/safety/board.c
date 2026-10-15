/**
 * @file board.c
 * @brief board config source file.
 *
 * @copyright Copyright (c) 2021  Semidrive Semiconductor.
 *            All rights reserved.
 */

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <types.h>
#include <compiler.h>
#include "board.h"

#ifdef CONFIG_IRQ
#include <driver/irq.h>
#endif

#ifdef CONFIG_ARCH_USE_MPU
#include "mpu_cfg.h"
#endif

#ifdef CONFIG_UART
#include "uart_cfg.h"
#endif

#ifdef CONFIG_CLK
#include "clock_cfg.h"
#endif

#ifdef CONFIG_TIMER
#include "timer_cfg.h"
#endif

#ifdef CONFIG_BOARD_REMAP
#include "remap.h"
#endif

#ifdef CONFIG_RESETGEN
#include "rstgen_cfg.h"
#endif

#ifdef CONFIG_PINCTRL
#include "pinmux_cfg.h"
#endif

#if defined(CONFIG_DW_DMA) || defined(CONFIG_DW_PCIE_DMA)
#include "dma_cfg.h"
#endif

#ifdef CONFIG_DWC_PCIE
#include "pcie_cfg.h"
#endif

#ifdef CONFIG_SDRV_I2S_MC
#include "i2s_mc_cfg.h"
#endif

#ifdef CONFIG_SDRV_SPDIF
#include "spdif_cfg.h"
#endif

#ifdef CONFIG_CDNS_I2S_SC
#include "i2s_sc_cfg.h"
#endif

#ifdef CONFIG_DISK
#include <lib/disk/disk.h>
#endif

#ifdef CONFIG_FEE_EEPROM
#include <lib/fee/fee_eeprom.h>
#endif

#ifdef CONFIG_FEE_BLOCK
#include <lib/fee/fee_block.h>
#endif

#ifdef CONFIG_ERRCODE
#include <sd_errcode.h>
#endif

void board_early_init(void)
{
#ifdef CONFIG_ERRCODE
    system_errcode_early_init();
#endif

#ifdef CONFIG_RESETGEN
    board_reset_init();
#endif

#ifdef CONFIG_BOARD_REMAP
    board_remap();
#endif

#ifdef CONFIG_CLK
    board_clk_init();
#endif

#ifdef CONFIG_UART
    board_uart_early_init();
#endif

#ifdef CONFIG_ARCH_USE_MPU
    board_mpu_init();
#endif

#ifdef CONFIG_IRQ
    irq_initialize(INT_BASE, IRQ_MAX_INTR_NUM);
#endif

#ifdef CONFIG_PINCTRL
    board_port_init();
#endif
}

void board_init(void)
{
#ifdef CONFIG_UART
    board_uart_init();
#endif

#ifdef CONFIG_TIMER
    board_timer_init();
#endif

#if defined(CONFIG_DW_DMA) || defined(CONFIG_DW_PCIE_DMA)
    board_dma_init();
#endif

#ifdef CONFIG_DWC_PCIE
    board_pcie_init();
#endif

#ifdef CONFIG_SDRV_I2S_MC
    board_i2s_mc_init();
#endif

#ifdef CONFIG_SDRV_SPDIF
    board_spdif_init();
#endif

#ifdef CONFIG_CDNS_I2S_SC
    board_i2s_sc_init();
#endif

#ifdef CONFIG_DISK
    disk_init();
#endif

#if defined(CONFIG_FEE_EEPROM)
    fee_init_eeprom();
#endif

#if defined(CONFIG_FEE_BLOCK)
    fee_init_block();
#endif

}
