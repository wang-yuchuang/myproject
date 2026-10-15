/**
 * @file sdrv_watchdog.c
 * @brief semidrive watchdog driver
 *
 * @copyright Copyright (c) 2021  Semidrive Semiconductor.
 *            All rights reserved.
 */
#include <types.h>

#include "sdrv_watchdog.h"

#include "reg.h"

static uint32_t g_clk_select[WDG_CLOCK_MAX] = {
    WDG_MAIN_CLK, WDG_BUS_CLK, WDG_EXT_CLK, WDG_TIE_OFF, WDG_LP_CLK};

uint32_t sdrv_watchdog_lld_reg_poll_value(wdg_reg_type_t *base, uint32_t reg,
                                          uint32_t start, uint32_t width,
                                          uint32_t value, uint32_t retrycount)
{
    uint32_t v;
    volatile uint32_t *p;

    p = (uint32_t *)((paddr_t)base + reg);

    do {
        v = *p;
        if (((v >> start) & ((1 << width) - 1)) == value)
            return retrycount;
    } while (--retrycount);

    return retrycount;
}

void sdrv_watchdog_lld_source_select(wdg_reg_type_t *regs, bool enable_from_reg,
                                     bool wtc_from_reg)
{
    uint32_t cfg_val;

    regs->wdg_lock &= ~(0x1u << WDG_LOCK_CTL_LOCK);

    cfg_val = regs->wdg_ctrl;
    cfg_val &= ~((0x1u << WDG_CTRL_WDG_EN_SRC) | (0x1u << WDG_CTRL_WTC_SRC));

    if (enable_from_reg)
        cfg_val |= (0x1u << WDG_CTRL_WDG_EN_SRC);

    if (wtc_from_reg)
        cfg_val |= (0x1u << WDG_CTRL_WTC_SRC);

    regs->wdg_ctrl = cfg_val;
}

void sdrv_watchdog_lld_config_clock_source(wdg_reg_type_t *regs,
                                           wdg_clock_source_e clk)
{
    regs->wdg_lock &=
        ~((0x1u << WDG_LOCK_CTL_LOCK) | (0x1u << WDG_LOCK_CLK_SRC_LOCK));

    RMWREG32(&regs->wdg_ctrl, WDG_CTRL_CLK_SRC_LSB, 3u, clk);
}

void sdrv_watchdog_lld_config_terminal_count(wdg_reg_type_t *regs,
                                             uint32_t timeout)
{
    uint32_t clk;
    uint32_t freq;
    uint32_t divisor;
    uint32_t wtc;

    clk = (regs->wdg_ctrl >> WDG_CTRL_CLK_SRC_LSB) & WDG_CTRL_CLK_SRC_MASK;
    divisor = 1 + ((regs->wdg_ctrl >> WDG_CTRL_PRE_DIV_NUM_LSB) &
                   WDG_CTRL_PRE_DIV_NUM_MASK);
    freq = g_clk_select[clk];
    wtc = timeout * (freq / divisor);

    regs->wdg_lock &= ~(0x1u << WDG_LOCK_WTC_LOCK);
    regs->wdg_wtc &= ~WDG_WTC_VAL_MASK;
    regs->wdg_wtc |= wtc;
}

void sdrv_watchdog_lld_config_refresh_mechanism(wdg_reg_type_t *regs,
                                                wdg_mechanism_mode_e mode)
{
    regs->wdg_lock &= ~(0x1u << WDG_LOCK_WRC_LOCK);

    switch (mode) {
    case WDG_MECHANISM_MODE1: {
        regs->wdg_wrc_ctl &=
            ~((0x1u << WDG_WRC_CTRL_MODEM0) | (0x1u << WDG_WRC_CTRL_MODEM1));
        regs->wdg_wrc_ctl |= (0x1u << WDG_WRC_CTRL_MODEM0);
        break;
    }

    case WDG_MECHANISM_MODE2: {
        regs->wdg_wrc_ctl &=
            ~((0x1u << WDG_WRC_CTRL_MODEM0) | (0x1u << WDG_WRC_CTRL_MODEM1));
        regs->wdg_wrc_ctl |=
            ((0x1u << WDG_WRC_CTRL_MODEM0) | (0x1u << WDG_WRC_CTRL_MODEM1));
        break;
    }

    case WDG_MECHANISM_MODE3: {
        regs->wdg_wrc_ctl &=
            ~((0x1u << WDG_WRC_CTRL_MODEM0) | (0x1u << WDG_WRC_CTRL_MODEM1));
        regs->wdg_wrc_ctl |= (0x1u << WDG_WRC_CTRL_SEQ_REFR);
        break;
    }

    default:
        break;
    }
}

void sdrv_watchdog_lld_wdt_module_enable(wdg_reg_type_t *regs)
{
    regs->wdg_lock &= ~(0x1u << WDG_LOCK_CTL_LOCK);
    regs->wdg_ctrl |= (0x1u << WDG_CTRL_WDG_EN);
}

void sdrv_watchdog_lld_wdt_module_disable(wdg_reg_type_t *regs)
{
    regs->wdg_lock &= ~(0x1u << WDG_LOCK_CTL_LOCK);
    regs->wdg_ctrl &= ~(0x1u << WDG_CTRL_WDG_EN);
}

void sdrv_watchdog_lld_wdt_soft_reset(wdg_reg_type_t *regs)
{
    regs->wdg_lock &= ~(0x1u << WDG_LOCK_CTL_LOCK);
    regs->wdg_ctrl |= (0x1u << WDG_CTRL_SOFT_RST);
}

void sdrv_watchdog_lld_refresh_trigger(wdg_reg_type_t *regs)
{
    regs->wdg_wrc_ctl |= (0x1u << WDG_WRC_CTRL_REFR_TRIG);
}
