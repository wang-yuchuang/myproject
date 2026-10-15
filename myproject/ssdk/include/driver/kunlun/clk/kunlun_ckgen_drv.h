/**
 * @file kunlun_ckgen_drv.h
 * @brief kunlun ckgen hld header
 * 
 * @copyright Copyright (c) 2021  Semidrive Semiconductor.
 *            All rights reserved.
 */

#ifndef KUNLUN_CKGEN_DRV_H_
#define KUNLUN_CKGEN_DRV_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <driver/clk.h>

extern struct clk_ops ckgen_ip_ops;

extern struct clk_ops ckgen_bus_ops;

extern struct clk_ops ckgen_core_ops;

extern struct clk_ops ckgen_uuu_ops;

extern struct clk_ops ckgen_lp_gating_ops;

#define CLK_NODE(node) &(node)

#ifdef __cplusplus
}
#endif

#endif /* KUNLUN_CKGEN_DRV_H__ */
