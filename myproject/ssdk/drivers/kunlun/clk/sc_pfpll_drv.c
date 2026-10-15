/**
 * @file sc_pfpll_drv.c
 * @brief
 *
 * @copyright Copyright (c) 2020  Semidrive Semiconductor.
 *            All rights reserved.
 */

#include <types.h>
#include <driver/clk.h>

#include "driver/kunlun/clk/sc_pfpll_drv.h"
#include "sc_pfpll.h"

static int pll_drv_enable(clk_node_t *clk)
{
    return pll_enable(clk->core->base, clk->id);
}

static int pll_drv_disable(clk_node_t *clk)
{
    return pll_disable(clk->core->base, clk->id);
}

static int pll_drv_prepare(clk_node_t *clk)
{
    return pll_prepare(clk->core->base, clk->id,
                       clk->parent->rate, clk->rate);
}

static clk_rate_t pll_drv_get_rate(clk_node_t *clk)
{
    return pll_get_rate(clk->core->base, clk->id,
                        clk->parent->rate);
}

static int pll_drv_set_rate(clk_node_t *clk, clk_rate_t rate)
{
    return pll_set_rate(clk->core->base, clk->id,
                        clk->parent->rate, rate);
}

struct clk_ops sc_pfpll_ops = {
    .prepare = pll_drv_prepare,
    .enable = pll_drv_enable,
    .disable = pll_drv_disable,
    .get_rate = pll_drv_get_rate,
    .set_rate = pll_drv_set_rate,
};
