/**
 * @file sdrv_watchdog_drv.c
 * @brief semidriver watchdog driver
 *
 * @copyright Copyright (c) 2021  Semidrive Semiconductor.
 *            All rights reserved.
 */
#include <debug.h>
#include <stdlib.h>
#include <regs_base.h>
#include <reg.h>

#include "driver/taishan/watchdog/sdrv_watchdog_drv.h"
#include "sdrv_watchdog.h"

int32_t sdrv_watchdog_start(watchdog_device_t *wdd)
{
    sdrv_watchdog_device_t *sdrv_wdd = (sdrv_watchdog_device_t *)wdd->priv;

    sdrv_watchdog_lld_wdt_module_enable((wdg_reg_type_t *)sdrv_wdd->base);

    if (!sdrv_watchdog_lld_reg_poll_value((wdg_reg_type_t *)sdrv_wdd->base,
                                          WDG_CTRL, WDG_CTRL_WDG_EN_STA, 1, 1,
                                          1000))
        return -1;

    sdrv_watchdog_lld_wdt_soft_reset((wdg_reg_type_t *)sdrv_wdd->base);

    if (!sdrv_watchdog_lld_reg_poll_value((wdg_reg_type_t *)sdrv_wdd->base,
                                          WDG_CTRL, WDG_CTRL_SOFT_RST, 1, 0,
                                          1000))
        return -1;

    return 0;
}

int32_t sdrv_watchdog_stop(watchdog_device_t *wdd)
{
    sdrv_watchdog_device_t *sdrv_wdd = (sdrv_watchdog_device_t *)wdd->priv;

    if (!sdrv_watchdog_lld_reg_poll_value((wdg_reg_type_t *)sdrv_wdd->base,
                                          WDG_CTRL, WDG_CTRL_WDG_EN_STA, 1, 0,
                                          1))
    {
        sdrv_watchdog_lld_wdt_soft_reset((wdg_reg_type_t *)sdrv_wdd->base);

        if (!sdrv_watchdog_lld_reg_poll_value((wdg_reg_type_t *)sdrv_wdd->base,
                                            WDG_CTRL, WDG_CTRL_SOFT_RST, 1, 0,
                                            100))
            return -1;

        sdrv_watchdog_lld_wdt_module_disable((wdg_reg_type_t *)sdrv_wdd->base);
    }

    return 0;
}

int32_t sdrv_watchdog_ping(watchdog_device_t *wdd)
{
    sdrv_watchdog_device_t *sdrv_wdd = (sdrv_watchdog_device_t *)wdd->priv;

    sdrv_watchdog_lld_refresh_trigger((wdg_reg_type_t *)sdrv_wdd->base);

    return 0;
}

int32_t sdrv_watchdog_set_timeout(watchdog_device_t *wdd, uint32_t timeout)
{
    sdrv_watchdog_device_t *sdrv_wdd = (sdrv_watchdog_device_t *)wdd->priv;

    sdrv_watchdog_lld_config_terminal_count((wdg_reg_type_t *)sdrv_wdd->base,
                                            timeout);

    return 0;
}

static struct watchdog_ops wdd_ops = {
    .start = sdrv_watchdog_start,
    .stop = sdrv_watchdog_stop,
    .ping = sdrv_watchdog_ping,
    .set_timeout = sdrv_watchdog_set_timeout,
};

static int32_t sdrv_watchdog_irq_handler(uint32_t irq, void *arg)
{
    uint32_t int_status;
    sdrv_watchdog_device_t *wdd = (sdrv_watchdog_device_t *)arg;

    int_status = WDG_READ_INT(wdd->base);

    if ((int_status >> WDG_INT_OVERFLOW_INT_STA) & 0x1u) {
        WDG_WRITE_INT(wdd->base,
                      int_status | (0x1u << WDG_INT_OVERFLOW_INT_CLR));

        if (wdd->isr)
            wdd->isr(irq, wdd->isr_arg);
    }

    return 0;
}

int32_t sdrv_watchdog_init(struct watchdog_device *wdd,
                           struct sdrv_watchdog_device *sdrv_wdd)
{
    int32_t ret = -1;

    wdd->ops = &wdd_ops;
    wdd->priv = sdrv_wdd;

    WDG_WRITE_LOCK(sdrv_wdd->base, WDG_READ_LOCK(sdrv_wdd->base) &
                                       ~(0x1u << WDG_LOCK_CTL_LOCK));

    WDG_WRITE_CTRL(sdrv_wdd->base, 0x0u);

    sdrv_watchdog_lld_source_select((wdg_reg_type_t *)sdrv_wdd->base, true,
                                    true);

    if (!sdrv_watchdog_lld_reg_poll_value((wdg_reg_type_t *)sdrv_wdd->base,
                                          WDG_CTRL, WDG_CTRL_WDG_EN_STA, 1, 0,
                                          100))
        goto fail;

    sdrv_watchdog_lld_config_clock_source((wdg_reg_type_t *)sdrv_wdd->base,
                                          WDG_CLOCK_MAIN);
    sdrv_watchdog_lld_config_refresh_mechanism((wdg_reg_type_t *)sdrv_wdd->base,
                                               WDG_MECHANISM_MODE1);

    WDG_WRITE_LOCK(sdrv_wdd->base, WDG_READ_LOCK(sdrv_wdd->base) &
                                       ~(0x1u << WDG_LOCK_RST_LOCK));

    WDG_WRITE_RST_CTL(sdrv_wdd->base, WDG_READ_RST_CTL(sdrv_wdd->base) &
                                          ~(0x1u << WDG_RST_CTRL_INT_RST_EN));

    if (sdrv_wdd->int_rst) {
        uint32_t int_rst_val = WDG_READ_RST_CTL(sdrv_wdd->base);

        int_rst_val &= ~(WDG_RST_CTRL_RST_WIN_MASK << WDG_RST_CTRL_RST_WIN_LSB);
        int_rst_val |= (0xfu << WDG_RST_CTRL_RST_WIN_LSB);
        int_rst_val |= (0x1u << WDG_RST_CTRL_INT_RST_EN);

        WDG_WRITE_RST_CTL(sdrv_wdd->base, int_rst_val);

        switch(sdrv_wdd->wdt_id) {
            case WATCHDOG1:
                RMWREG32(APB_RSTGEN_SAF_BASE + 0x204, 4, 1, 1);
            break;

            case WATCHDOG2:
                RMWREG32(APB_RSTGEN_SAF_BASE + 0x204, 12, 1, 1);
            break;

            case WATCHDOG3:
                RMWREG32(APB_RSTGEN_SAF_BASE + 0x204, 5, 2, 0x3);
            break;

            case WATCHDOG4:
                RMWREG32(APB_RSTGEN_SAF_BASE + 0x204, 13, 2, 0x3);
            break;

            case WATCHDOG5:
                RMWREG32(APB_RSTGEN_SAF_BASE + 0x204, 7, 2, 0x3);
            break;

            case WATCHDOG6:
                RMWREG32(APB_RSTGEN_SAF_BASE + 0x204, 15, 2, 0x3);
            break;

            default:
            break;
        }
    }

    WDG_WRITE_LOCK(sdrv_wdd->base, WDG_READ_LOCK(sdrv_wdd->base) &
                   ~(0x1u << WDG_LOCK_EXT_RST_LOCK));

    WDG_WRITE_EXT_RST_CTL(sdrv_wdd->base, WDG_READ_EXT_RST_CTL(sdrv_wdd->base) &
                          ~(0x1u << WDG_EXT_RST_CTRL_EXT_RST_EN));

    if (sdrv_wdd->ext_rst) {
        uint32_t ext_rst_val = WDG_READ_EXT_RST_CTL(sdrv_wdd->base);

        ext_rst_val &= ~(WDG_EXT_RST_CTRL_RST_WIN_MASK << WDG_EXT_RST_CTRL_RST_WIN_LSB);
        ext_rst_val |= (0xfu << WDG_EXT_RST_CTRL_RST_WIN_LSB);
        ext_rst_val |= (0x1u << WDG_EXT_RST_CTRL_EXT_RST_EN);

        ext_rst_val &= ~(0x1u << WDG_EXT_RST_CTRL_EXT_RST_MODE);
        if (sdrv_wdd->ext_rst_mode == EXT_RST_PULSE) {
            ext_rst_val |= (0x1u << WDG_EXT_RST_CTRL_EXT_RST_MODE);
        }

        ext_rst_val &= ~(0x1u << WDG_EXT_RST_CTRL_EXT_RST_REQ_POL);
        if (sdrv_wdd->ext_rst_pol == EXT_RST_HIGH_ACTIVE) {
            ext_rst_val |= (0x1u << WDG_EXT_RST_CTRL_EXT_RST_REQ_POL);
        }

        WDG_WRITE_EXT_RST_CTL(sdrv_wdd->base, ext_rst_val);
    }

    if (sdrv_wdd->irq >= 0) {
        WDG_WRITE_LOCK(sdrv_wdd->base, WDG_READ_LOCK(sdrv_wdd->base) &
                                           ~(0x1u << WDG_LOCK_INT_LOCK));

        WDG_WRITE_INT(sdrv_wdd->base, (0x1u << WDG_INT_OVERFLOW_INT_EN) |
                                          (0x1u << WDG_INT_OVERFLOW_INT_CLR));

        irq_attach(sdrv_wdd->irq, sdrv_watchdog_irq_handler, sdrv_wdd);
        irq_enable(sdrv_wdd->irq);
    }

    ret = 0;
fail:
    return ret;
}
