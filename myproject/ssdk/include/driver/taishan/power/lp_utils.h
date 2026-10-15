/**
 * @file lp_utils.h
 * @brief low power utils function
 *
 * @copyright Copyright (c) 2020  Semidrive Semiconductor.
 *            All rights reserved.
 */

#ifdef CONFIG_SDRV_SMC

#ifndef LP_UTILS_H_
#define LP_UTILS_H_

#include <types.h>
#include <driver/taishan/clk/taishan_ckgen_drv.h>

void lp_clean_uart_tx_fifo(uint32_t base);

void lp_ckgen_node_switch_rc24m(taishan_ckgen_node_t *ckgen_node);

void lp_ckgen_node_switch_fs24m(taishan_ckgen_node_t *ckgen_node);

void lp_ckgen_node_switch_config(taishan_ckgen_node_t *ckgen_node);

void lp_rtc_wakeup_after_seconds(uint64_t sec);

#endif /* LP_UTILS_H_ */

#endif /* CONFIG_SDRV_SMC */
