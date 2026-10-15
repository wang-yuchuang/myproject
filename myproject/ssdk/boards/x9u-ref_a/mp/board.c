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
#include <debug.h>
#include <assert.h>

#ifdef CONFIG_IRQ
#include <driver/irq.h>
#endif

#ifdef CONFIG_ARCH_USE_MPU
#include "mpu_cfg.h"
#endif

#ifdef CONFIG_UART
#include "uart_cfg.h"
#endif

#ifdef CONFIG_I2C
#include "i2c_cfg.h"
#endif

#ifdef CONFIG_SPI
#include "spi_cfg.h"
#endif

#ifdef CONFIG_CLK
#include "clock_cfg.h"
#endif

#ifdef CONFIG_TIMER
#include "timer_cfg.h"
#endif

#ifdef CONFIG_RTC
#include "rtc_cfg.h"
#endif /* end CONFIG_RTC */

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

#ifdef CONFIG_KUNLUN_MBOX
#include "mbox_cfg.h"
#endif /* end CONFIG_KUNLUN_MBOX */

#ifdef CONFIG_REBOOT
#include "reboot_cfg.h"
#endif /* end CONFIG_REBOOT */

#ifdef CONFIG_SYS_DIAGNOSIS
#include "sys_diagnosis.h"
#endif /* end CONFIG_SYS_DIAGNOSIS */

#ifdef CONFIG_DISPSS
#include "disp_cfg.h"
#endif

#ifdef CONFIG_G2DLITE
#include "g2dlite_cfg.h"
#endif

#ifdef CONFIG_KUNLUN_PWM
#include "pwm_cfg.h"
#endif

#if defined(CONFIG_TCA6408) || defined(CONFIG_TCA9539)
#include "tca_gpio_cfg.h"
#endif

#ifdef CONFIG_CSI
#include "cam_cfg.h"
#endif

#ifdef CONFIG_SDRV_KUNLUN_PMU
#include "pmu_cfg.h"
#endif /* end CONFIG_SDRV_KUNLUN_PMU */

#ifdef CONFIG_VPU
#include "vpu_cfg.h"
#endif /* end CONFIG_VPU */

#ifdef CONFIG_SERDES
#include "serdes_cfg.h"
#endif /* end CONFIG_SERDES */

#ifdef CONFIG_TOUCH
#include "touch_cfg.h"
#endif /* end CONFIG_TOUCH */

#ifdef CONFIG_PVT
#include "pvt_cfg.h"
#endif /* end CONFIG_PVT */

#ifdef CONFIG_SDRV_VIRT_UART
#include "driver/kunlun/virt_uart/virt_uart.h"
#endif

#ifdef CONFIG_ERRCODE
#include <sd_errcode.h>
#endif

void board_early_init(void)
{
#ifdef CONFIG_ERRCODE
    system_errcode_early_init();
#endif

#ifdef CONFIG_UART
    board_uart_early_init();
#endif

#ifdef CONFIG_SDRV_VIRT_UART
    virt_uart_init(MPC_R5);
#endif

    printf("MP core early init start\n");

#ifdef CONFIG_RESETGEN
    board_reset_init();
#endif

#ifdef CONFIG_BOARD_REMAP
    board_remap();
#endif

#ifdef CONFIG_CLK
    board_clk_init();
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

#ifdef CONFIG_I2C
    board_i2c_early_init();
#endif

    printf("MP core early init end\n");
}

void board_init(void)
{
    printf("MP board init start\n");

#ifdef CONFIG_UART
    board_uart_init();
#endif

#ifdef CONFIG_SDRV_VIRT_UART
    virt_uart_init(MPC_R5);
#endif

#ifdef CONFIG_TIMER
    board_timer_init();
#endif

#ifdef CONFIG_RTC
    board_rtc_init();
#endif /* end CONFIG_RTC */

#if defined(CONFIG_DW_DMA) || defined(CONFIG_DW_PCIE_DMA)
    board_dma_init();
#endif

#ifdef CONFIG_KUNLUN_MBOX
    board_mbox_init();
#endif /* end CONFIG_KUNLUN_MBOX */

#ifdef CONFIG_DWC_PCIE
   board_pcie_init();
#endif

#ifdef CONFIG_I2C
    board_i2c_init();
#endif

#ifdef CONFIG_SPI
    board_spi_init();
#endif

#ifdef CONFIG_DISPSS
    board_disp_init();
#endif

#ifdef CONFIG_SERDES
    board_serdes_init();
#endif /* end CONFIG_SERDES */

#ifdef CONFIG_TOUCH
    board_touch_init();
#endif /* end CONFIG_TOUCH */

#ifdef CONFIG_G2DLITE
    board_g2dlite_init();
#endif

#ifdef CONFIG_KUNLUN_PWM
    //board_pwm_init();
#endif

#ifdef CONFIG_TCA6408
    board_tca6408_init();
#endif

#ifdef CONFIG_CSI
    board_camera_init();
#endif

#ifdef CONFIG_VPU
    board_vpu_init();
#endif /* end CONFIG_VPU */
}

