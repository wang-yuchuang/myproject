/**
 * @file sc_pfpll_drv.c
 * @brief
 *
 * @copyright Copyright (c) 2020  Semidrive Semiconductor.
 *            All rights reserved.
 */

#include <types.h>
#include <driver/clk.h>

#include "driver/kunlun/clk/kunlun_ckgen_drv.h"
#include "kunlun_ckgen.h"

static int ckgen_drv_ip_prepare(clk_node_t *clk)
{
    return ckgen_ip_prepare(clk->core->base, clk->id, clk->mux,
                            clk->parent->rate, clk->rate);
}

static clk_rate_t ckgen_drv_ip_get_rate(clk_node_t *clk)
{
    return ckgen_ip_get_rate(clk->core->base, clk->id,
                             clk->parent->rate);
}

static int ckgen_drv_ip_set_rate(clk_node_t *clk, clk_rate_t rate)
{
    return ckgen_ip_set_rate(clk->core->base, clk->id,
                             clk->parent->rate, rate);
}

static int ckgen_drv_core_prepare(clk_node_t *clk)
{
    return ckgen_core_prepare(clk->core->base, clk->id, clk->mux,
                              clk->parent->rate, clk->rate);
}

static clk_rate_t ckgen_drv_core_get_rate(clk_node_t *clk)
{
    return ckgen_core_get_rate(clk->core->base, clk->id,
                               clk->parent->rate);
}

static int ckgen_drv_core_set_rate(clk_node_t *clk, clk_rate_t rate)
{
    return ckgen_core_set_rate(clk->core->base, clk->id,
                               clk->parent->rate, rate);
}

static int ckgen_drv_bus_prepare(clk_node_t *clk)
{
    return ckgen_bus_prepare(clk->core->base, clk->id, clk->mux,
                             clk->parent->rate, clk->rate);
}

static clk_rate_t ckgen_drv_bus_get_rate(clk_node_t *clk)
{
    return ckgen_bus_get_rate(clk->core->base, clk->id,
                              clk->parent->rate);
}

static int ckgen_drv_bus_set_rate(clk_node_t *clk, clk_rate_t rate)
{
    return ckgen_bus_set_rate(clk->core->base, clk->id,
                              clk->parent->rate, rate);
}

static int ckgen_drv_uuu_prepare(clk_node_t *clk)
{
    return ckgen_uuu_prepare(clk->core->base, clk->id, clk->mux,
                             clk->parent->rate, clk->rate);
}

static clk_rate_t ckgen_drv_uuu_get_rate(clk_node_t *clk)
{
    return ckgen_uuu_get_rate(clk->core->base, clk->id,
                              clk->parent->rate);
}

static int ckgen_drv_uuu_set_rate(clk_node_t *clk, clk_rate_t rate)
{
    return ckgen_uuu_set_rate(clk->core->base, clk->id,
                              clk->parent->rate, rate);
}

static int ckgen_drv_lp_gating_enable(clk_node_t *clk)
{
    return ckgen_lp_gating_enable(clk->core->base, clk->id);
}

static int ckgen_drv_lp_gating_disable(clk_node_t *clk)
{
    return ckgen_lp_gating_disable(clk->core->base, clk->id);
}

#ifdef CONFIG_CLK_MONITOR
static clk_rate_t ckgen_drv_ip_mon_get_rate(clk_node_t *clk)
{
    return ckgen_ip_mon_get_rate(clk->core->base, clk->id, clk->parent->rate);
}

static clk_rate_t ckgen_drv_bus_mon_get_rate(clk_node_t *clk)
{
    return ckgen_bus_mon_get_rate(clk->core->base, clk->id, clk->parent->rate);
}

static clk_rate_t ckgen_drv_core_mon_get_rate(clk_node_t *clk)
{
    return ckgen_core_mon_get_rate(clk->core->base, clk->id, clk->parent->rate);
}
#endif

struct clk_ops ckgen_ip_ops = {
    .prepare = ckgen_drv_ip_prepare,
#ifdef CONFIG_CLK_MONITOR
    .get_rate = ckgen_drv_ip_mon_get_rate,
#else
    .get_rate = ckgen_drv_ip_get_rate,
#endif
    .set_rate = ckgen_drv_ip_set_rate,
};

struct clk_ops ckgen_bus_ops = {
    .prepare = ckgen_drv_bus_prepare,
#ifdef CONFIG_CLK_MONITOR
    .get_rate = ckgen_drv_bus_mon_get_rate,
#else
    .get_rate = ckgen_drv_bus_get_rate,
#endif
    .set_rate = ckgen_drv_bus_set_rate,
};

struct clk_ops ckgen_core_ops = {
    .prepare = ckgen_drv_core_prepare,
#ifdef CONFIG_CLK_MONITOR
    .get_rate = ckgen_drv_core_mon_get_rate,
#else
    .get_rate = ckgen_drv_core_get_rate,
#endif
    .set_rate = ckgen_drv_core_set_rate,
};

struct clk_ops ckgen_uuu_ops = {
    .prepare = ckgen_drv_uuu_prepare,
    .get_rate = ckgen_drv_uuu_get_rate,
    .set_rate = ckgen_drv_uuu_set_rate,
};

struct clk_ops ckgen_lp_gating_ops = {
    .enable = ckgen_drv_lp_gating_enable,
    .disable = ckgen_drv_lp_gating_disable,
};
