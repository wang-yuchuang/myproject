# -*- coding: utf-8 -*-

from const import Const

const_mk = Const()

# board arch config list
const_mk.board_list = {
    # board_core
    'x9h_safety' : {'ARCH':'arm', 'CPU':'cortex-r5'},
    'x9h_secure' : {'ARCH':'arm', 'CPU':'cortex-r5'}
}

const_mk.string_list = ['BOARD', 'PROJECT', 'ARCH', 'ARM_CPU', 'PLATFORM', 'SUBARCH', 'ENABLE_THUMB',
'VIC_OP_MODE']

const_mk.main_c = r"""#include <arch.h>
#include <cmsis_os2.h>
#include <board.h>
#include <driver/uart/uart.h>
#include "CLI.h"

extern uart_dev_t g_uart_console;

int main(void)
{
    arch_early_init();
    board_early_init();
    osKernelInitialize();
    arch_init();
    board_init();
    CLIConsoleStart(1024, osPriorityNormal,
                    &g_uart_console, false);

    if (osKernelGetState() == osKernelReady) {
        osKernelStart();
    }

    for (;;) {
    }
}
"""

const_mk.board_c = r"""#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <types.h>
#include <compiler.h>
#include "arch/armv7-r/mpu.h"
#include "driver/reset.h"
#include "driver/reset/sdrv_reset_drv.h"
#include "driver/irq.h"
#include "driver/scr.h"
#include "board.h"
#include "reg.h"
#include "uart_cfg.h"
#include "clock_cfg.h"

#define RSTGEN_GENERAL_REG(n)   ((0x50 + (n)*4) << 10)
#define RSTGEN_REMAP_STATUS_REG (APB_RSTGEN_BASE + RSTGEN_GENERAL_REG(1))
#define REMAP_DONE              (0x52454d50) /* 'REMP' */
#define ROMC_STICKY_REG         (APB_ROMC_BASE + 0x34)
#define ARRAY_NUMS(array)       (sizeof(array) / sizeof(array[0]))

uart_dev_t g_uart_console;
dw_uart_dev_t g_dw_uart;

#ifdef BOARD_KERNEL_TIMER
sdrv_tmr_dev_t g_sdrv_tmr;
timer_dev_t g_kernel_timer;
#endif

static void vector_remap(addr_t vector_base)
{
    /* base address must be 4 KB aligned */
    //writel(REMAP_DONE, RSTGEN_REMAP_STATUS_REG);
    /* Enable R5 remapping to vector base. The remap config doesn't
     * take effect until REMAP module detects R5 core reset.
     */
    scr_signal_t remap_addr_signal = SCR_REMAP_ADDR_OFFSET;
    scr_signal_t remap_en_signal = SCR_REMAP_OVRD_EN;
    writel(REMAP_DONE, RSTGEN_REMAP_STATUS_REG);
    scr_set(APB_SCR_BASE, &remap_addr_signal, vector_base >> 12);
    scr_set(APB_SCR_BASE, &remap_en_signal, 1);
    /* ROMC Stick Reg
     *
     * [0]: ROMC_STICKY_REMAP_EN. This bit is AND'ed with
     *      SCR remap_en bit, to enable remapping.
     */
    RMWREG32(ROMC_STICKY_REG, 0, 1, 1);
}

static void board_remap(void)
{
    if (readl((RSTGEN_REMAP_STATUS_REG)) != REMAP_DONE) {
        vector_remap(CONFIG_MEM_BASE);
        board_core_reset();
        while (1) {
            __ASM volatile("wfi");
        }
    } else {
        writel(0, (RSTGEN_REMAP_STATUS_REG));
    }
}

static void board_mpu_init(void)
{
    static const mpu_config_t mpu_config_array[] = {
        {0,                     0x40,       MPU_REGION_NORMAL_RO},
        {CONFIG_MEM_BASE,       0x40,       MPU_REGION_NORMAL_RO},
        /* ospi1 indirect·address space. */
        {0x7FFFE00,             0x200,       MPU_REGION_DEVICE},
        /* PCIe1 region address space. */
        {0x10000000,            0x10000000, MPU_REGION_DEVICE},
        /* PCIe2 region address space. */
        {0x20000000,            0x10000000, MPU_REGION_DEVICE},
    };

    mpu_config_t *config;

    mpu_enable(false);

    for (uint32_t i = 0; i < ARRAY_NUMS(mpu_config_array); i++) {
        config = (mpu_config_t *)&mpu_config_array[i];
        if (config->size > 0) {
            mpu_add_region(i, config->addr, config->size, config->type);
        }
    }

    mpu_enable(true);
}

static void kernel_timer_init(void)
{
#ifdef BOARD_KERNEL_TIMER
    sdrv_tmr_init(&g_sdrv_tmr, &kernel_timer_cfg);
    sdrv_tmr_timer_probe(&g_kernel_timer, &g_sdrv_tmr, SDRV_TMR_KERNEL_TIMER);
#endif
}

void board_early_init(void)
{
    board_reset_init();
    board_remap();
    clk_dev_init(&board_clk_config);
    dw_uart_probe(&g_uart_console, &g_dw_uart, &console_uart_config);
    board_mpu_init();
    irq_initialize();
    kernel_timer_init();
    board_port_init();
}

void board_init(void)
{
    uart_init(&g_uart_console);
}

char board_getc(void)
{
    return uart_getc(&g_uart_console);
}

void board_putc(char c)
{
    uart_putc(&g_uart_console, c);
    if (c == '\n')
        uart_putc(&g_uart_console, '\r');
}

void udelay(uint32_t usecs)
{
#ifdef BOARD_KERNEL_TIMER
    uint64_t start = timer_get_global_time(&g_kernel_timer);

    while ((timer_get_global_time(&g_kernel_timer) - start) < usecs)
        ;
#endif
}
"""

const_mk.board_h = r"""/*
 * board.h
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: board interface.
 *
 * Revision History:
 * -----------------
 */
#ifndef _INCLUDE_BOARD_H
#define _INCLUDE_BOARD_H

#include <compiler.h>
#include <regs_base.h>
#include <irq_num.h>
#include "scr_hw.h"

#define APB_RSTGEN_BASE             APB_RSTGEN_SAF_BASE
#define APB_SCR_BASE                APB_SCR_SAF_BASE
#define APB_ROMC_BASE               APB_ROMC1_BASE
#define RSTGEN_CORE_INDEX           2
#define RSTGEN_IDX_NUM              10
#define SCR_REMAP_ADDR_OFFSET       SCR_SAFETY__remap_cr5_saf_ar_addr_offset_19_0
#define SCR_REMAP_OVRD_EN           SCR_SAFETY__remap_cr5_saf_ar_remap_ovrd_en

#define DEVICE_BASE(dev) _DEVICE_BASE(dev)
#define _DEVICE_BASE(dev) APB_##dev##_BASE

#define INT_BASE __INT_BASE(BOARD_INTR_CTRL)
#define __INT_BASE(intr) __INT_BASE__(intr)
#define __INT_BASE__(intr) intr##_BASE

#define DEVICE_INTR(dev) _DEVICE_INTR(dev)
#define _DEVICE_INTR(dev) dev##_INTR_NUM


__BEGIN_CDECLS;

/* super early platform initialization, before almost everything */
void board_early_init(void);

/* later init, after the kernel has come up */
void board_init(void);

/* board put char */
void board_putc(char c);

/* board get char */
char board_getc(void);

/* delay inline for the requested number of microseconds */
void udelay(uint32_t usecs);

void board_port_init(void);
void board_reset_init(void);
void board_core_reset(void);

__END_CDECLS;

#endif
"""

const_mk.reset_c = r"""/**
 * @file board_reset.c
 * @brief
 *
 * @copyright Copyright (c) 2021  Semidrive Semiconductor.
 *            All rights reserved.
 */
#include <types.h>
#include <param.h>
#include "driver/reset.h"
#include "driver/reset/sdrv_reset_drv.h"
#include "kunlun/reset/sdrv_rstgen.h"
#include "board.h"

static sdrv_reset_ctl_t rstgen_safety_dev = {
    .base           = APB_RSTGEN_SAF_BASE,
    .self_rst_en    = true,
    .sem_rst_en     = true,
    .dbg_rst_en     = true,
    .wdg1_rst_en    = true,
};

reset_ctl_t g_rstgen_safety;

static sdrv_reset_ctl_t rstgen_sec_dev = {
    .base   = APB_RSTGEN_SEC_BASE,
};

reset_ctl_t g_rstgen_sec;

static const uint32_t default_reset_ids[] = {
    RSTSIG_SAF_GIC1,
    RSTSIG_SAF_OSPI1,
    RSTSIG_SAF_ENET1,
    RSTSIG_SAF_I2S_SC1,
    RSTSIG_SAF_I2S_SC2,
    RSTSIG_SAF_CANFD1,
    RSTSIG_SAF_CANFD2,
    RSTSIG_SAF_CANFD3,
    RSTSIG_SAF_CANFD4,
    RSTSIG_SAF_SEM1,
    RSTSIG_SAF_SEM2,
};

static void init_pcie_ss(void)
{
    reset_sig_t rst_sig = {
        .rst_ctl = &g_rstgen_sec,
    };

    uint32_t ids[] = {
        RSTSIG_AP_PCIEPHY,
        RSTSIG_AP_PCIE2,
        RSTSIG_AP_PCIE1,
    };

    /* PCIe iso disable */
    rst_sig.id = RSTSIG_AP_ISOLATE_PCIE;
    reset_ctl_deassert(&rst_sig);

    /* Reset PCIe subsystem */
    for (int i = 0; i < ARRAY_SIZE(ids); i++) {
        rst_sig.id = ids[i];
        reset_ctl_reset(&rst_sig);
    }
}

static void board_release_rst(void)
{
    reset_sig_t rst_sig = {
        .rst_ctl = &g_rstgen_safety,
    };

    for (uint32_t i = 0;
         i < ARRAY_SIZE(default_reset_ids); i++) {
        rst_sig.id = default_reset_ids[i];
        reset_ctl_reset(&rst_sig);
    }

    init_pcie_ss();
}

void board_core_reset(void)
{
    reset_sig_t rstsig_saf_r5 = {
        .rst_ctl = &g_rstgen_safety,
        .id      = RSTSIG_SAF_CR5_SAF,
    };

    reset_ctl_reset(&rstsig_saf_r5);
}

void board_reset_init(void)
{
    sdrv_reset_init(&g_rstgen_safety, &rstgen_safety_dev);
    sdrv_reset_init(&g_rstgen_sec,    &rstgen_sec_dev);

    board_release_rst();
}
"""

const_mk.pinmux_c = r"""/*
 * board_pinmux.h
 *
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: Board level pinmux configuration.
 *
 * Revision History:
 * -----------------
 */
#include <types.h>
#include <bits.h>
#include <param.h>
#include <reg.h>
#include <string.h>
#include <cmsis_os2.h>
#include "driver/pinctrl/pinctrl.h"
#include "driver/pinctrl/sdrv_pinctrl.h"
#include "driver/gpio/gpio.h"
#include "driver/gpio/sdrv_gpio.h"
#include "pinmux_cfg.h"
#include "board.h"

/*
 * Each domain in the SOC has one pinctrl controller:
 *   safety: 5 dedicated JTAG pins, GPIO 0 ~ 47, SEM_FAULT, GPIO_SAF
 *   ap: GPIO 48 ~ 155
 *   rtc: 12 dedicated sys/rtc pins
 *
 * To make life easier, we define following "virtual" pinctrl devices,
 * each controlling a continuous range of pins.
 */

/* pinctrl_safety [0:47]:      GPIO 0~47 */
#define PINCTRL_SAFETY_PIN_START    0
#define PINCTRL_SAFETY_PIN_NUM      48

/* pinctrl_ap [48:155]:        GPIO 48~155 */
#define PINCTRL_AP_PIN_START        48
#define PINCTRL_AP_PIN_NUM          108
#define PINCTRL_AP_EMMC_PIN_START   132
#define PINCTRL_AP_EMMC_PIN_NUM     24

/* Number of "normal" GPIO pins - GPIO [0:155] */
#define PINCTRL_GPIO_PIN_NR         156

/* pinctrl_safety1 [156:160]:  JTAG TMS/TCK/TDI/TDO/TRST_N */
#define PINCTRL_SAFETY1_PIN_START   156
#define PINCTRL_SAFETY1_PIN_NUM     5

/* pinctrl_safety2 [161:162]:  SEM_FAULT, GPIO_SAF */
#define PINCTRL_SAFETY2_PIN_START   161
#define PINCTRL_SAFETY2_PIN_NUM     2

/* pinctrl_rtc [163:174]:      sys/rtc pins */
#define PINCTRL_RTC_PIN_START       163
#define PINCTRL_RTC_PIN_NUM         12

static sdrv_pinctrl_t pinctrl_rtc_dev = {
    .base = APB_IOMUXC_RTC_BASE,
    .pad_reg_offset = 0,
    .mux_reg_offset = 0x200,
};

static sdrv_pinctrl_t pinctrl_safety_dev = {
    .base = APB_IOMUXC_SAF_BASE,
    .pad_reg_offset = 0x14,
    .mux_reg_offset = 0x200,
};

static sdrv_pinctrl_t pinctrl_ap_dev = {
    .base = APB_IOMUXC_SEC_BASE,
    .pad_reg_offset = 0,
    .mux_reg_offset = 0x200,
    .emmc_pad_start = PINCTRL_AP_EMMC_PIN_START,
    .emmc_pad_nr = PINCTRL_AP_EMMC_PIN_NUM,
};

pinctrl_t g_pinctrl_rtc = {
    .pin_start = PINCTRL_RTC_PIN_START,
    .pin_nr = PINCTRL_RTC_PIN_NUM,
};

pinctrl_t g_pinctrl_safety = {
    .pin_start = PINCTRL_SAFETY_PIN_START,
    .pin_nr = PINCTRL_SAFETY_PIN_NUM,
};

pinctrl_t g_pinctrl_ap = {
    .pin_start = PINCTRL_AP_PIN_START,
    .pin_nr = PINCTRL_AP_PIN_NUM,
};

static const uint32_t gpio_safety_irqs[] = {
    GPIO1_GPIO_INT0_INTR_NUM,
    GPIO1_GPIO_INT1_INTR_NUM,
    GPIO1_GPIO_INT2_INTR_NUM,
    GPIO1_GPIO_INT3_INTR_NUM,
    GPIO1_GPIO_INT4_INTR_NUM,
    GPIO1_GPIO_INT5_INTR_NUM,
};

gpio_dev_t g_gpio_dev;

static void pin_mux_init(void)
{
    for (int i = 0; i < NUM_OF_CONFIGURED_PINS; i++) {
        pin_settings_config_t *config = &g_pin_init_config[i];
        pinctrl_t *pinctrl = config->pin_index < PINCTRL_AP_PIN_START ?
                             &g_pinctrl_safety : &g_pinctrl_ap;
        uint32_t drive = config->open_drain == PIN_OPEN_DRAIN_ENABLED ?
                         PINCTRL_DRIVE_OPEN_DRAIN : PINCTRL_DRIVE_PUSH_PULL;

        pinctrl_config(pinctrl, config->pin_index, PINCTRL_CONFIG_FUNCTION, config->mux);
        pinctrl_config(pinctrl, config->pin_index, PINCTRL_CONFIG_DRIVE, drive);
        pinctrl_config(pinctrl, config->pin_index, PINCTRL_CONFIG_PULL, config->pull_config);
        pinctrl_config(pinctrl, config->pin_index, PINCTRL_CONFIG_DRIVE_STRENGTH, config->drive_strength);
        pinctrl_config(pinctrl, config->pin_index, PINCTRL_CONFIG_SLEW_RATE, config->slew_rate);
        pinctrl_config(pinctrl, config->pin_index, PINCTRL_CONFIG_INPUT_SCHMITT, config->input_select);
        pinctrl_config(pinctrl, config->pin_index, PINCTRL_CONFIG_FORCE_INPUT, config->force_input);

        if (config->mux == PAD_MUX_FUNC_GPIO) {
            sdrv_gpio_assign_controller(config->pin_index, config->controller_id);

            if ((uint32_t)config->controller_id == GPIO_SAFETY) {
                gpio_type_e gpio_type = GPIO_OUT;

                if (config->data_direction == PIN_INPUT_DIRECTION) {
                    switch (config->interrupt_config) {
                        case PIN_INTERRUPT_DISABLED:
                            gpio_type = GPIO_IN;
                            break;
                        case PIN_INTERRUPT_HIGH_LEVEL:
                            gpio_type = GPIO_INTR_HIGH_LEVEL;
                            break;
                        case PIN_INTERRUPT_LOW_LEVEL:
                            gpio_type = GPIO_INTR_LOW_LEVEL;
                            break;
                        case PIN_INTERRUPT_RISING_EDGE:
                            gpio_type = GPIO_INTR_RISING_EDGE;
                            break;
                        case PIN_INTERRUPT_FALLING_EDGE:
                            gpio_type = GPIO_INTR_FALLING_EDGE;
                            break;
                        case PIN_INTERRUPT_BOTH_EDGE:
                            gpio_type = GPIO_INTR_BOTH_EDGE;
                            break;
                        default:
                            gpio_type = GPIO_IN;
                            break;
                    }
                }

                gpio_config(&g_gpio_dev, config->pin_index, gpio_type);
                gpio_set(&g_gpio_dev, config->pin_index, config->initial_value);
            }
        }
    }
}

void board_port_init(void)
{
    /* Initialize pinctrl devices. */
    sdrv_pinctrl_init(&g_pinctrl_rtc, &pinctrl_rtc_dev);
    sdrv_pinctrl_init(&g_pinctrl_safety, &pinctrl_safety_dev);
    sdrv_pinctrl_init(&g_pinctrl_ap, &pinctrl_ap_dev);

    /* Initialize the GPIO controller. */
    sdrv_gpio_init(&g_gpio_dev, APB_GPIO1_BASE,
                   ARRAY_SIZE(gpio_safety_irqs), gpio_safety_irqs);

    pin_mux_init();
}
"""
