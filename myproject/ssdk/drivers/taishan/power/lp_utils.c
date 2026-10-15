#ifdef CONFIG_SDRV_SMC
/**
 * @file lp_utils.c
 * @brief low power utils function
 *
 * @copyright Copyright (c) 2020  Semidrive Semiconductor.
 *            All rights reserved.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <cmsis_os2.h>
#include <debug.h>
#include <board.h>
#include <driver/taishan/smc/sdrv_smc_drv.h>
#include <driver/taishan/rtc/sec_rtcd.h>
#include <driver/pm.h>
#include <driver/irq.h>
#include <types.h>
#include <param.h>
#include <reg.h>
#include "clock_cfg.h"
#include "rstgen_cfg.h"
#include "rtc_cfg.h"
#include "irq_num.h"

void lp_clean_uart_tx_fifo(uint32_t base)
{
    RMWREG32(base + 0x50, 16, 1, 1);
}

void lp_ckgen_node_switch_rc24m(taishan_ckgen_node_t *ckgen_node)
{
    taishan_ckgen_node_t oper_node;

    oper_node = *ckgen_node;
    oper_node.clk_node.mux = 0;
    oper_node.clk_node.rate = 24000000;

    clk_prepare(CLK_NODE(oper_node));
}

void lp_ckgen_node_switch_fs24m(taishan_ckgen_node_t *ckgen_node)
{
    taishan_ckgen_node_t oper_node;

    oper_node = *ckgen_node;
    oper_node.clk_node.mux = 1;
    oper_node.clk_node.rate = 24000000;

    clk_prepare(CLK_NODE(oper_node));
}

void lp_ckgen_node_switch_config(taishan_ckgen_node_t *ckgen_node)
{
    clk_prepare(CLK_NODE((*ckgen_node)));
}

void lp_rtc_wakeup_after_seconds(uint64_t sec)
{
    struct rtc_wkalrm alarm;

    rtc_enable_alarm(&g_sec_rtc_dev, false);
    rtc_get_time(&g_sec_rtc_dev, &alarm.tm);
    sec += rtc_to_epoch(&alarm.tm);
    epoch_to_rtc(&alarm.tm, sec);
    rtc_set_alarm(&g_sec_rtc_dev, &alarm, NULL);
    rtc_enable_alarm(&g_sec_rtc_dev, true);
}

#endif /* CONFIG_SDRV_SMC */
