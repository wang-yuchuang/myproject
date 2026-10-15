/**
 * @file sdrv_pinctrl_lld.h
 * @brief pinctrl LLD driver
 *
 * @copyright Copyright (c) 2020  Semidrive Semiconductor.
 *            All rights reserved.
 */
#include <types.h>
#include <bits.h>
#include <reg.h>

#include "driver/kunlun/pinctrl/sdrv_pinctrl.h"

static inline paddr_t pin_reg(paddr_t base, uint32_t offset, uint32_t pin)
{
    return base + ((offset + pin * 4) << 10);
}

static inline void write_pin_reg(paddr_t base, uint32_t offset,
                                 uint32_t pin, uint32_t val)
{
    writel(val, pin_reg(base, offset, pin));
}

static inline uint32_t read_pin_reg(paddr_t base, uint32_t offset, uint32_t pin)
{
    return readl(pin_reg(base, offset, pin));
}

static inline void modify_pin_reg(paddr_t base, uint32_t offset, uint32_t pin,
                                  uint32_t bit, uint32_t width, uint32_t val)
{
    uint32_t v = read_pin_reg(base, offset, pin);
    v &= ~(BIT_MASK(width) << bit);
    v |= (val & BIT_MASK(width)) << bit;
    write_pin_reg(base, offset, pin, v);
}

static inline void
sdrv_pinctrl_set_function(paddr_t base, uint32_t offset, unsigned int pin,
                          unsigned int value)
{
#ifdef CONFIG_BOARD_X9SP
    if (value >= PAD_MUX_FUNC_GPIO && value <= PAD_MUX_FUNC_ALT8) {
#else
    if (value >= PAD_MUX_FUNC_GPIO && value <= PAD_MUX_FUNC_ALT7) {
#endif
        modify_pin_reg(base, offset, pin, PAD_MUX_FUNC_SHIFT,
                       PAD_MUX_FUNC_WIDTH, value);
    }
}

static inline void
sdrv_pinctrl_set_drive(paddr_t base, uint32_t offset, unsigned int pin,
                       unsigned int value)
{
    modify_pin_reg(base, offset, pin, PAD_MUX_OPEN_DRAIN_SHIFT,
                   PAD_MUX_OPEN_DRAIN_WIDTH, value);
}

static inline void
sdrv_pinctrl_set_pull_gpio(paddr_t base, uint32_t offset, unsigned int pin,
                           unsigned int value)
{
    modify_pin_reg(base, offset, pin, PAD_PULL_SHIFT, PAD_PULL_WIDTH, value);
}

static inline void
sdrv_pinctrl_set_drive_strength_gpio(paddr_t base, uint32_t offset,
                                     unsigned int pin, unsigned int value)
{
    modify_pin_reg(base, offset, pin, PAD_DRIVE_STRENGTH_SHIFT,
                   PAD_DRIVE_STRENGTH_WIDTH, value);
}

static inline void
sdrv_pinctrl_set_input_schmitt_gpio(paddr_t base, uint32_t offset,
                                    unsigned int pin, unsigned int value)
{
    modify_pin_reg(base, offset, pin, PAD_INPUT_SCHMITT_SHIFT,
                   PAD_INPUT_SCHMITT_WIDTH, value);
}

static inline void
sdrv_pinctrl_set_slew_rate_gpio(paddr_t base, uint32_t offset,
                                unsigned int pin, unsigned int value)
{
    modify_pin_reg(base, offset, pin, PAD_SLEW_RATE_SHIFT,
                   PAD_SLEW_RATE_WIDTH, value);
}

static inline void
sdrv_pinctrl_set_pull_emmc(paddr_t base, uint32_t offset, unsigned int pin,
                           unsigned int value)
{
    modify_pin_reg(base, offset, pin, EMMC_PAD_PULL_SHIFT,
                   EMMC_PAD_PULL_WIDTH, value);
}

static inline void
sdrv_pinctrl_set_drive_strength_emmc(paddr_t base, uint32_t offset,
                                     unsigned int pin, unsigned int value)
{
    modify_pin_reg(base, offset, pin, EMMC_PAD_DRIVE_STRENGTH_NMOS_SHIFT,
                   EMMC_PAD_DRIVE_STRENGTH_NMOS_WIDTH, value);
    modify_pin_reg(base, offset, pin, EMMC_PAD_DRIVE_STRENGTH_PMOS_SHIFT,
                   EMMC_PAD_DRIVE_STRENGTH_PMOS_WIDTH, value);
}

static inline void
sdrv_pinctrl_set_input_schmitt_emmc(paddr_t base, uint32_t offset,
                                    unsigned int pin, unsigned int value)
{
    modify_pin_reg(base, offset, pin, EMMC_PAD_RXSEL_SHIFT,
                   EMMC_PAD_RXSEL_WIDTH, value);
}

static inline void
sdrv_pinctrl_set_slew_rate_emmc(paddr_t base, uint32_t offset,
                                unsigned int pin, unsigned int value)
{
    modify_pin_reg(base, offset, pin, EMMC_PAD_SLEW_RATE_NMOS_SHIFT,
                   EMMC_PAD_SLEW_RATE_NMOS_WIDTH, value);
    modify_pin_reg(base, offset,  pin, EMMC_PAD_SLEW_RATE_PMOS_SHIFT,
                   EMMC_PAD_SLEW_RATE_PMOS_WIDTH, value);
}

static inline void
sdrv_pinctrl_set_force_input(paddr_t base, uint32_t offset, unsigned int pin,
                             unsigned int fin, unsigned int fv)
{
    modify_pin_reg(base, offset, pin, PAD_MUX_FORCE_INPUT_SHIFT,
                   PAD_MUX_FORCE_INPUT_WIDTH, fin);
    modify_pin_reg(base, offset, pin, PAD_MUX_FORCE_INPUT_VALUE_SHIFT,
                   PAD_MUX_FORCE_INPUT_VALUE_WIDTH, fv);
}
