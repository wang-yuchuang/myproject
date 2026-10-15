#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <driver/taishan/pinctrl/taishan_pinctrl.h>

#include "cmsis_os2.h"
#include "CLI_type.h"

#include "driver/watchdog.h"
#include "driver/taishan/watchdog/sdrv_watchdog_drv.h"
#include "irq_num.h"
#include "regs_base.h"
#include "param.h"
#include "rstgen_cfg.h"
#include "pinmux_cfg.h"

extern pinctrl_t g_pinctrl_safety;
extern pinctrl_t g_pinctrl_ap;
#define PINCTRL_INSTANCE(pin)  \
    (((pin) < TAISHAN_SAFETY_PIN_NUM) ? &g_pinctrl_safety : &g_pinctrl_ap)

static int wdt_overflow_isr(uint32_t irq, void *arg);

static osSemaphoreId_t g_sem;
static watchdog_device_t g_wdt;

static sdrv_watchdog_device_t g_sdrv_wdt = {
    .base  =  APB_WDT1_BASE,
    #ifdef CONFIG_SDRV_WDOG_TAISHAN
    .irq   =  WDT1_WDT_INTR_NUM,
    #else
    .irq   =  WDT1_OVFLOW_INTR_NUM,
    #endif
    .isr   =  wdt_overflow_isr,
};

static void watchdog_int_rst_cfg(uint32_t base)
{
    reset_ctl_general_write(&reset_general_reg_sf_boot, 0);

    if (base == APB_WDT1_BASE) {
        g_sdrv_wdt.wdt_id = WATCHDOG1;
        g_sdrv_wdt.int_rst = true;
    } else if (base == APB_WDT2_BASE) {
        g_sdrv_wdt.wdt_id = WATCHDOG2;
        g_sdrv_wdt.int_rst = true;
    } else if (base == APB_WDT3_BASE) {
        g_sdrv_wdt.wdt_id = WATCHDOG3;
        g_sdrv_wdt.int_rst = true;
    } else if (base == APB_WDT4_BASE) {
        g_sdrv_wdt.wdt_id = WATCHDOG4;
        g_sdrv_wdt.int_rst = true;
    } else if (base == APB_WDT5_BASE) {
        g_sdrv_wdt.wdt_id = WATCHDOG5;
        g_sdrv_wdt.int_rst = true;
    } else if (base == APB_WDT6_BASE) {
        g_sdrv_wdt.wdt_id = WATCHDOG6;
        g_sdrv_wdt.int_rst = true;
    }
}

static void watchdog_ext_rst_cfg(ext_rst_mode_e mode, ext_rst_pol_e pol)
{
    static pin_settings_config_t wdt_reset_n = {
        /* evb */
        .pin_index         =  GPIO_X11,
        .mux               =  PIN_MUX_ALT3,

        /* gateway */
        // .pin_index         =  GPIO_H12,
        // .mux               =  PIN_MUX_ALT4,
        .open_drain        =  PIN_PUSH_PULL,
        .pull_config       =  PIN_NOPULL,
        .drive_strength    =  PIN_DS_8MA,
        .slew_rate         =  PIN_SR_SLOW,
        .input_select      =  PIN_IS_CMOS,
        .data_direction    =  PIN_OUTPUT_DIRECTION,
        .interrupt_config  =  PIN_INTERRUPT_DISABLED,
        .initial_value     =  PIN_LEVEL_LOW,
        .force_input       =  PIN_FORCE_INPUT_NORMAL,
        .mode_select       =  PIN_MODE_DIGITAL,
    };

    g_sdrv_wdt.ext_rst_mode = mode;
    g_sdrv_wdt.ext_rst_pol = pol;
    g_sdrv_wdt.ext_rst = true;

    pinctrl_t *pinctrl = PINCTRL_INSTANCE(wdt_reset_n.pin_index);

    pinctrl_config(pinctrl, wdt_reset_n.pin_index, PINCTRL_CONFIG_FUNCTION, wdt_reset_n.mux);
    pinctrl_config(pinctrl, wdt_reset_n.pin_index, PINCTRL_CONFIG_DRIVE, wdt_reset_n.open_drain);
    pinctrl_config(pinctrl, wdt_reset_n.pin_index, PINCTRL_CONFIG_PULL, wdt_reset_n.pull_config);
    pinctrl_config(pinctrl, wdt_reset_n.pin_index, PINCTRL_CONFIG_DRIVE_STRENGTH, wdt_reset_n.drive_strength);
    pinctrl_config(pinctrl, wdt_reset_n.pin_index, PINCTRL_CONFIG_SLEW_RATE, wdt_reset_n.slew_rate);
    pinctrl_config(pinctrl, wdt_reset_n.pin_index, PINCTRL_CONFIG_INPUT_SCHMITT, wdt_reset_n.input_select);
    pinctrl_config(pinctrl, wdt_reset_n.pin_index, PINCTRL_CONFIG_FORCE_INPUT, wdt_reset_n.force_input);
}

static int wdt_overflow_isr(uint32_t irq, void *arg)
{
    printf("wdt_overflow_isr\n");

    osSemaphoreRelease(g_sem);

    return 0;
}

static int watchdog_test_main(int argc, char *argv[])
{
    int refresh_cnt = 5;

    if (argc && !strcmp(argv[0], "int_rst")) {
        watchdog_int_rst_cfg(g_sdrv_wdt.base);
    }
    else if (argc && !strcmp(argv[0], "ext_rst")) {
        ext_rst_mode_e mode = EXT_RST_LEVEL;
        ext_rst_pol_e pol = EXT_RST_LOW_ACTIVE;

        if (!strcmp(argv[1], "pulse")) {
            mode = EXT_RST_PULSE;
        }

        if (!strcmp(argv[2], "high")) {
            pol = EXT_RST_HIGH_ACTIVE;
        }

        watchdog_ext_rst_cfg(mode, pol);
    }

    if (sdrv_watchdog_init(&g_wdt, &g_sdrv_wdt)) {
        printf("sdrv_watchdog_init fail.\n");
        goto ret;
    }

    /* set timeout 2second */
    if (watchdog_set_timeout(&g_wdt, 2)) {
        printf("watchdog_set_timeout fail.\n");
        goto ret;
    }

    if (watchdog_start(&g_wdt)) {
        printf("watchdog_start fail.\n");
        goto ret;
    }

    while (refresh_cnt > 0) {
        osDelay(1 * osKernelGetTickFreq());
        printf("ping dog at time: %d\n", refresh_cnt);
        if(watchdog_ping(&g_wdt)) {
            printf("watchdog_ping fail.\n");
            goto ret;
        }
        refresh_cnt--;
    }

    g_sem = osSemaphoreNew(1, 0, NULL);

    watchdog_ping(&g_wdt);

    printf("Test watchdog timeout begin.\n");

    if (osSemaphoreAcquire(g_sem, 3 * osKernelGetTickFreq()) == osOK)
        printf("Watchdog timeout success\n");
    else
        printf("osSemaphoreAcquire fail.\n");

    osDelay(5 * osKernelGetTickFreq());

    osSemaphoreDelete(g_sem);
ret:
    return CLI_FALSE;
}

CLI_CMD("wdg_test", "\r\nwdg_test:\r\n Watchdog test\r\n", watchdog_test_main);