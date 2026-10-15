/*
 * ext_rst_req.c
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Revision History:
 * -----------------
 */
#include <reg.h>
#include <board.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <debug.h>
#include <cmsis_os2.h>
#include <driver/kunlun/pinctrl/sdrv_pinctrl.h>

#include "CLI_type.h"
#include "regs_base.h"
#include "driver/watchdog.h"
#include "driver/kunlun/watchdog/sdrv_watchdog_drv.h"
#include "irq_num.h"
#include "pinmux_cfg.h"

extern pinctrl_t g_pinctrl_safety;
extern pinctrl_t g_pinctrl_rtc;
#define PIN_MUX_SYS_CTRL3_OFFSET 0x32C

static watchdog_device_t g_wdt;
static sdrv_watchdog_device_t g_sdrv_wdt = {
	.base  =  APB_WDT1_BASE,
	.wdt_id = WATCHDOG1,
	.int_rst = false,
	.ext_rst_mode = EXT_RST_LEVEL,
	.ext_rst_pol = EXT_RST_LOW_ACTIVE,
	.ext_rst = true,
};

int wdt_req_ext_rst(int argc, char *argv[])
{
	uint32_t val;
	paddr_t iomux_rtc_base;
	paddr_t pmu_base;

	ssdk_printf(SSDK_CRIT, "wdt_req_ext_rst start\n");
	ssdk_printf(SSDK_CRIT, "config sys_ctrl3 pinmux\n");
	iomux_rtc_base = APB_IOMUXC_RTC_BASE;
	ssdk_printf(SSDK_CRIT, "sys_ctrl3 pinmux original val : 0x%x\n",
		readl(iomux_rtc_base + PIN_MUX_SYS_CTRL3_OFFSET));
	writel(4, iomux_rtc_base + PIN_MUX_SYS_CTRL3_OFFSET);
	ssdk_printf(SSDK_CRIT, "sys_ctrl3 pinmux modified val : 0x%x\n",
		readl(iomux_rtc_base + PIN_MUX_SYS_CTRL3_OFFSET));

	ssdk_printf(SSDK_CRIT, "config pmu ext_ctrl\n");
	pmu_base = APB_PMU_BASE;
	val = readl(pmu_base);
	ssdk_printf(SSDK_CRIT, "ext_ctrl original val : 0x%x\n", val);
	val |= (1 << 7);
	writel(val, pmu_base);
	ssdk_printf(SSDK_CRIT, "ext_ctrl modified val : 0x%x\n", readl(pmu_base));

    if (sdrv_watchdog_init(&g_wdt, &g_sdrv_wdt)) {
        printf("sdrv_watchdog_init fail.\n");
        goto ret;
    }

    /* set timeout 5 second */
    if (watchdog_set_timeout(&g_wdt, 5)) {
        printf("watchdog_set_timeout fail.\n");
        goto ret;
    }

    if (watchdog_start(&g_wdt)) {
        printf("watchdog_start fail.\n");
        goto ret;
    }

	return 0;

ret:
	return CLI_FALSE;
}

CLI_CMD("wdt_req_ext_rst", "wdt1.reset_n test", wdt_req_ext_rst);
