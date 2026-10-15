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
#include "reg.h"

#ifdef CONFIG_PM
#include <driver/taishan/power/taishan_pm.h>
#endif

#ifdef CONFIG_SMC
#include <driver/taishan/smc/sdrv_smc_drv.h>
#endif

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

#ifdef CONFIG_IRAM_NONE_ECC
#include <taishan_iram.h>
#endif

#ifdef CONFIG_PINCTRL
#include "pinmux_cfg.h"
#endif

#ifdef CONFIG_I2C
#include "i2c_cfg.h"
#endif

#ifdef CONFIG_DISK
#include <lib/disk/disk.h>
#endif

#ifdef CONFIG_DISK_RAM
#include "ram_cfg.h"
#endif

#ifdef CONFIG_ADC
#include "adc_cfg.h"
#endif

#ifdef CONFIG_BOARD_DEBUG
#include <coredbg.h>
#endif

#ifdef CONFIG_MBOX
#include "mbox_cfg.h"
#endif

#ifdef CONFIG_HWMUTEX
#include "hwmutex_cfg.h"
#endif

#ifdef CONFIG_CU
#include "cu.h"
#endif

#ifdef CONFIG_ETH
#include "eth_cfg.h"
#endif

#ifdef CONFIG_CAN
#include "can_cfg.h"
#endif

#ifdef CONFIG_SDRV_DMA
#include "dma_cfg.h"
#endif

#ifdef CONFIG_PWM
#include "pwm_cfg.h"
#endif

#ifdef CONFIG_SENT
#include "sent_cfg.h"
#endif

#ifdef CONFIG_SBC
#include "sbc_cfg.h"
#endif

#ifdef CONFIG_RTC
#include "rtc_cfg.h"
#endif

#ifdef CONFIG_PMU
#include "pmu_cfg.h"
#endif

#ifdef CONFIG_CORE_SHM
#include "core_shm_cfg.h"
#endif

#ifdef CONFIG_SACI_I2S
#include "i2s_cfg.h"
#endif

#ifdef CONFIG_SACI_PDM
#include "pdm_cfg.h"
#endif

#ifdef CONFIG_CODEC
#include "codec_cfg.h"
#endif

#ifdef CONFIG_FUSA_MONITOR
#include "fusa_monitor.h"
#endif

#if  defined ( CONFIG_USB_DEVICE ) || defined ( CONFIG_USB_HOST )
#include "usb_bsp.h"
#endif

#ifdef CONFIG_LIN
#include "lin_cfg.h"
#endif

unsigned int g_core_running;

static unsigned int board_get_running_core(void)
{
    unsigned int running_core = 0;

#if defined(CONFIG_SP_SPLIT_MODE)
    running_core |= (CORE_SP0_RUNNING | CORE_SP1_RUNNING);
#elif defined(CONFIG_SP_LOCKSTEP_MODE)
    running_core |= CORE_SP0_RUNNING;
#endif

#if defined(CONFIG_SX_SPLIT_MODE)
    running_core |= (CORE_SX0_RUNNING | CORE_SX1_RUNNING);
#elif defined(CONFIG_SX_LOCKSTEP_MODE)
    running_core |= CORE_SX0_RUNNING;
#endif

    if (!reset_ctl_status(&rstsig_cr5_sp0)) {
        running_core |= CORE_SP0_RUNNING;
    }

    if (!reset_ctl_status(&rstsig_cr5_sp1)) {
        running_core |= CORE_SP1_RUNNING;
    }

    if (!reset_ctl_status(&rstsig_cr5_sx0)) {
        running_core |= CORE_SX0_RUNNING;
    }

    if (!reset_ctl_status(&rstsig_cr5_sx1)) {
        running_core |= CORE_SX1_RUNNING;
    }

    g_core_running = running_core;

#ifdef CONFIG_CORE_SHM
    g_coreshm->running_core = running_core;
#endif

    return running_core;
}

#ifdef CONFIG_CU
bool g_console_occupid;

void board_putc(char c)
{
#ifdef CONFIG_UART
    if (g_console_uart) {
        if (g_console_occupid) {
            uart_putc(g_console_uart, c);
            if (c == '\n')
                uart_putc(g_console_uart, '\r');
        }
        else {
            cu_buffer_print_log(&c, 1);
            if (c == '\n') {
                c = '\r';
                cu_buffer_print_log(&c, 1);
            }
        }
    }
#endif /* CONFIG_UART */
}
#endif /* CONFIG_CU */

void board_early_init(void)
{
#ifdef CONFIG_SMC
    sdrv_smc_wait_soc_swm_run();
    sdrv_smc_wait_ap_sys_ready();
#endif

#ifdef CONFIG_FIX_LDO_1V8
    writel(0x0, APB_LDO_DIG_BASE + 0x10);
#endif

#ifdef CONFIG_RESETGEN
    board_reset_init();
#endif

#ifdef CONFIG_IRAM_NONE_ECC
    iram_ecc_disable();
#endif

#ifdef CONFIG_BOARD_REMAP
    board_remap();
#endif

#ifdef CONFIG_CORE_SHM
    board_coreshm_init();
#endif

    board_get_running_core();

#ifdef CONFIG_BOARD_DEBUG
    core_debug_enable(true);
#endif

#ifdef CONFIG_PINCTRL
    board_port_early_init();
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

#ifdef CONFIG_I2C
    board_i2c_early_init();
#endif

#ifdef CONFIG_IRQ
    irq_initialize(INT_BASE, IRQ_MAX_INTR_NUM);
#endif
}

void board_init(void)
{
#ifdef CONFIG_PINCTRL
    board_port_init();
#endif

#ifdef CONFIG_UART
    board_uart_init();
#endif

#ifdef CONFIG_TIMER
    board_timer_init();
#endif

#ifdef CONFIG_MBOX
    board_mbox_init();
#endif

#ifdef CONFIG_HWMUTEX
    board_hwmutex_init();
#endif

#ifdef CONFIG_I2C
    board_i2c_init();
#endif

#ifdef CONFIG_CU
    cu_console_init();
#endif

#ifdef CONFIG_DISK
    disk_init();
#endif

#ifdef CONFIG_ADC
    board_adc_init();
#endif

#ifdef CONFIG_SDRV_DMA
    board_dma_init();
#endif

#ifdef CONFIG_DISK_RAM
    board_ram_init();
#endif

#ifdef CONFIG_ETH
    board_eth_init();
#endif

#ifdef CONFIG_CAN
    board_can_init();
#endif

#ifdef CONFIG_PWM
    board_pwm_init();
#endif

#ifdef CONFIG_SENT
    board_sent_init();
#endif

#ifdef CONFIG_SBC
    board_sbc_init();
#endif

#ifdef CONFIG_RTC
    board_rtc_init();
#endif

#ifdef CONFIG_PMU
    board_pmu_init();
#endif

#ifdef CONFIG_PM
    taishan_pm_initialize();
#endif

#ifdef CONFIG_SACI_I2S
    board_i2s_init();
#endif

#ifdef CONFIG_SACI_PDM
    board_pdm_init();
#endif

#ifdef CONFIG_CODEC
    board_codec_init();
#endif

#ifdef CONFIG_FUSA_MONITOR
    sem_monitor_init();
#endif

#if  defined ( CONFIG_USB_DEVICE ) || defined ( CONFIG_USB_HOST )
    usb_irq_init();
#endif

#ifdef CONFIG_LIN
    board_lin_init();
#endif
}
