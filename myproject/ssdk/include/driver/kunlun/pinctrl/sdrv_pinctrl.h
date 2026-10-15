/**
 * @file sdrv_pinctrl.h
 * @brief Kunlun pinctrl device driver header.
 *
 * @copyright Copyright (c) 2020  Semidrive Semiconductor.
 *            All rights reserved.
 */
#ifndef _SDRV_PINCTRL_H
#define _SDRV_PINCTRL_H

#include <types.h>
#include "driver/pinctrl.h"

/*
 * The pinctrl driver controls configuration and muliplexing of digital pins
 * in the SOC. There are 2 types of pads:
 *  - GPIO pad, used by Safety and RTC domain, as well as non-eMMC pins
 *    in AP domain.
 *  - EMMC pad, used by eMMC pins in AP domain. The pads are controlled by eMMC
 *    controller in eMMC mode, by PINMUX_AP in other modes.
 */
#define SDRV_PINCTRL_GPIO_PAD           0
#define SDRV_PINCTRL_EMMC_PAD           1

/*
 * GPIO PAD REGISTERS.
 */
#ifdef CONFIG_BOARD_X9SP
/* GPIO IO_PAD_CONFIG */
#define PAD_PULL_SHIFT                  0
#define PAD_PULL_WIDTH                  2
#define PAD_DRIVE_STRENGTH_SHIFT        2
#define PAD_DRIVE_STRENGTH_WIDTH        2
#define PAD_SLEW_RATE_SHIFT             4
#define PAD_SLEW_RATE_WIDTH             1
#define PAD_INPUT_SCHMITT_SHIFT         5
#define PAD_INPUT_SCHMITT_WIDTH         1
#define PAD_PARAMETRIC_OUTPUT_SHIFT     6
#define PAD_PARAMETRIC_OUTPUT_WIDTH     1

/* GPIO PIN_MUX_CONFIG */
#define PAD_MUX_FUNC_SHIFT              0
#define PAD_MUX_FUNC_WIDTH              4
#define PAD_MUX_OPEN_DRAIN_SHIFT        4
#define PAD_MUX_OPEN_DRAIN_WIDTH        1
#define PAD_MUX_FORCE_INPUT_SHIFT       8
#define PAD_MUX_FORCE_INPUT_WIDTH       2
#define PAD_MUX_FORCE_INPUT_VALUE_SHIFT 12
#define PAD_MUX_FORCE_INPUT_VALUE_WIDTH 1
#else
/* GPIO IO_PAD_CONFIG */
#define PAD_PULL_SHIFT                  0
#define PAD_PULL_WIDTH                  2
#define PAD_DRIVE_STRENGTH_SHIFT        4
#define PAD_DRIVE_STRENGTH_WIDTH        2
#define PAD_SLEW_RATE_SHIFT             8
#define PAD_SLEW_RATE_WIDTH             1
#define PAD_INPUT_SCHMITT_SHIFT         12
#define PAD_INPUT_SCHMITT_WIDTH         1
#define PAD_PARAMETRIC_OUTPUT_SHIFT     16
#define PAD_PARAMETRIC_OUTPUT_WIDTH     1

/* GPIO PIN_MUX_CONFIG */
#define PAD_MUX_FUNC_SHIFT              0
#define PAD_MUX_FUNC_WIDTH              3
#define PAD_MUX_OPEN_DRAIN_SHIFT        4
#define PAD_MUX_OPEN_DRAIN_WIDTH        1
#define PAD_MUX_FORCE_INPUT_SHIFT       8
#define PAD_MUX_FORCE_INPUT_WIDTH       2
#define PAD_MUX_FORCE_INPUT_VALUE_SHIFT 12
#define PAD_MUX_FORCE_INPUT_VALUE_WIDTH 1
#endif

/* Pulls. */
#define PAD_NOPULL                      (0x0)
#define PAD_PULLDOWN                    (0x1)
#define PAD_PULLUP                      (0x3)

/* Output drive strength. */
#define PAD_DRIVE_STRENGTH_2mA          (0x0)
#define PAD_DRIVE_STRENGTH_8mA          (0x1)
#define PAD_DRIVE_STRENGTH_4mA          (0x2)
#define PAD_DRIVE_STRENGTH_12mA         (0x3)

/* Slew rate. */
#define PAD_SLEW_RATE_FAST              (0x0)
#define PAD_SLEW_RATE_SLOW              (0x1)

/* Input Schmitt Select. Schmitt trigger mode provides input
 * glitch filtering (default is normal mode).
 */
#define PAD_INPUT_SCHMITT               (0x0)
#define PAD_INPUT_NORMAL                (0x1)

/* Parametric output. Not used. */
#define PAD_PARAMETRIC_OUTPUT_DISABLE   (0x0)
#define PAD_PARAMETRIC_OUTPUT_ENABLE    (0x1)

/* Alternative function selection. */
#define PAD_MUX_FUNC_GPIO               (0x0)
#define PAD_MUX_FUNC_ALT1               (0x1)
#define PAD_MUX_FUNC_ALT2               (0x2)
#define PAD_MUX_FUNC_ALT3               (0x3)
#define PAD_MUX_FUNC_ALT4               (0x4)
#define PAD_MUX_FUNC_ALT5               (0x5)
#define PAD_MUX_FUNC_ALT6               (0x6)
#define PAD_MUX_FUNC_ALT7               (0x7)
#define PAD_MUX_FUNC_ALT8               (0x8)

/* Open drain or push pull, only for output pads. */
#define PAD_PUSHPULL                    (0x0)
#define PAD_OPENDRAIN                   (0x1)

/* Force input ON/OFF.
 *  - no force:  Pad input/output is selected by ALT function.
 *  - force enable: Enable pad input, regardless of ALT function.
 *  - force disable: Disable pad input. Internal IP read the value of
 *    FORCE_INPUT bit.
 */
#define PAD_INPUT_NO_FORCE              (0x0)
#define PAD_INPUT_FORCE_ENABLE          (0x1)
#define PAD_INPUT_FORCE_DISABLE         (0x2)

/* Force input value. Used in PAD_INPUT_FORCE_DISABLE mode. */
#define PAD_FORCE_INPUT_VALUE_0         (0)
#define PAD_FORCE_INPUT_VALUE_1         (1)

/*
 * EMMC PAD REGISTERS.
 */

/* EMMC PAD_CONFIG */
#define EMMC_PAD_PULL_SHIFT                 0
#define EMMC_PAD_PULL_WIDTH                 2
#define EMMC_PAD_SLEW_RATE_NMOS_SHIFT       4
#define EMMC_PAD_SLEW_RATE_NMOS_WIDTH       4
#define EMMC_PAD_SLEW_RATE_PMOS_SHIFT       8
#define EMMC_PAD_SLEW_RATE_PMOS_WIDTH       4
#define EMMC_PAD_RXSEL_SHIFT                12
#define EMMC_PAD_RXSEL_WIDTH                3
#define EMMC_PAD_DRIVE_STRENGTH_NMOS_SHIFT  16
#define EMMC_PAD_DRIVE_STRENGTH_NMOS_WIDTH  4
#define EMMC_PAD_DRIVE_STRENGTH_PMOS_SHIFT  20
#define EMMC_PAD_DRIVE_STRENGTH_PMOS_WIDTH  4

/* Pulls. */
#define EMMC_PAD_NOPULL                     (0x0)
#define EMMC_PAD_PULLUP                     (0x1)
#define EMMC_PAD_PULLDOWN                   (0x2)

/* Slew rate for NMOS and PMOS. */
#define EMMC_PAD_SLEW_RATE_MIN              (0x0)
#define EMMC_PAD_SLEW_RATE_MAX              (0xF)
// #define EMMC_PAD_SLEW_RATE_1V8_DEFAULT      ((0x1 << 4) | (0x3 << 8))
// #define EMMC_PAD_SLEW_RATE_3V3_DEFAULT      EMMC_PAD_SLEW_RATE_MAX

/* Input select. */
#define EMMC_PAD_RXSEL_RECEIVER_DISABLED    (0x0)
#define EMMC_PAD_RXSEL_1V8_SCHMITT          (0x1)
#define EMMC_PAD_RXSEL_3V3_SCHMITT          (0x2)
#define EMMC_PAD_RXSEL_INTERNAL_LOOPBACK    (0x7)

/* Drive strength for NMOS and PMOS. */
#define EMMC_PAD_DRIVE_STRENGTH_100ohm      0x0
#define EMMC_PAD_DRIVE_STRENGTH_66ohm       0x4
#define EMMC_PAD_DRIVE_STRENGTH_50ohm       0x8
#define EMMC_PAD_DRIVE_STRENGTH_40ohm       0xc
#define EMMC_PAD_DRIVE_STRENGTH_33ohm       0xe

/* EMMC MUX_CONFIG register is same as GPIO MUX_CONFIG. */

/**
 * @brief Kunlun pinctrl module.
 *
 * We have 3 PINCTRL modules in the SOC, iomux_ctrl_rtc, iomux_ctrl_safety,
 * and iomux_ctrl_ap, which control pins from RTC domain, safety domain
 * and non-safety domain.
 */
typedef struct sdrv_pinctrl {
    /* Controller address. */
    paddr_t     base;

    /* Range of EMMC pads. */
    uint32_t    emmc_pad_start;
    uint32_t    emmc_pad_nr;

    /* Offset of PAD and MUX config registers. */
    uint32_t    pad_reg_offset;
    uint32_t    mux_reg_offset;
} sdrv_pinctrl_t;

void sdrv_pinctrl_init(pinctrl_t *pinctrl, sdrv_pinctrl_t *dev);
void sdrv_pinctrl_enable_canfd(int can_ctrl, bool enable);

#endif /* _SDRV_PINCTRL_H */
