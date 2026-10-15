/**
 * @file clk.c
 * @brief Clk subsystem core source.
 *
 * @copyright Copyright (c) 2021  Semidrive Semiconductor.
 *            All rights reserved.
 */

#include <debug.h>
#include <driver/clk.h>

#define LVDS_PLL_ROOT        (1u)
#define LVDS_PLL_DIVA        (2u)
#define LVDS_PLL_DIVB        (3u)
#define PLL_FBDIV_MIN        (16u)
#define PLL_FBDIV_MAX        (640u)
#define PLL_POSTDIV1_MAX     (0x7u)
#define PLL_POSTDIV2_MAX     (0x7u)
#define PLL_DIVABCD_MAX      (0xfu)

int clk_prepare(clk_node_t *clk)
{
    clk_core_t *core = clk->core;

    if (core == NULL || core->ops == NULL) {
        ssdk_printf(SSDK_CRIT, "%s: core point or ops is NULL!\n", __func__);
        return -1;
    }

    if (core->ops->prepare == NULL) {
        ssdk_printf(SSDK_WARNING, "%s: no prepare function!\n", __func__);
        return 0;
    }

    return core->ops->prepare(clk);
}

int clk_set_rate(clk_node_t *clk, clk_rate_t rate)
{
    clk_core_t *core = clk->core;
    clk_node_t *parent = NULL;
    int ret = -1;

    if (core == NULL) {
        ssdk_printf(SSDK_CRIT, "%s: core point is NULL!\n", __func__);
        return -1;
    }

    if (core->ops->select_parent != NULL) {
        core->ops->select_parent(clk, rate);
    }

    if (clk->parent == NULL) {
        ssdk_printf(SSDK_CRIT, "%s: clk node parent is NULL!\n", __func__);
        return -1;
    }

    parent = clk->parent;

    if (core->ops != NULL && core->ops->set_rate != NULL) {
        ret = core->ops->set_rate(clk, rate);
    }
    else if (parent->core->ops != NULL &&
             parent->core->ops->set_rate != NULL) {
        ret = parent->core->ops->set_rate(parent, rate);
    }

    if (ret >= 0) {
        clk->rate = rate;
    }

    return ret;
}

int lvds_clk_set_rate(clk_node_t *clk, clk_rate_t rate)
{
    clk_node_t *parent = NULL;
    int id = clk->id;
    int div_max = 15;
    int ret = -1;

    parent = clk->parent;
    parent->rate = rate;
    parent->parent->rate = 24000000;

    if (id == LVDS_PLL_ROOT) {
        div_max = PLL_POSTDIV1_MAX * PLL_POSTDIV2_MAX;
    } else {
        div_max = PLL_DIVABCD_MAX;
    }

    for (int i = 2; i <= div_max; i++) {
        if (parent->rate < 800000000) {
            parent->rate = rate * i;
            continue;
        } else if (parent->rate > 800000000 && parent->rate < 3200000000) {
            break;
        } else {
            return ret;
        }
    }

    if (clk_set_rate(parent, parent->rate)) {
        ssdk_printf(SSDK_CRIT, "set parent pll vco clock failed!\n");
        return ret;
    }

    for (int i = LVDS_PLL_ROOT; i <= LVDS_PLL_DIVB; i++) {
        if (i == id) {
            clk->id = i;
            ret = clk_set_rate(clk, rate);
            clk_enable(clk);
            continue;
        }

        switch (i) {
            case LVDS_PLL_ROOT:
                clk->id = i;
                ret = clk_set_rate(clk, parent->rate);
                clk_enable(clk);
                break;
            case LVDS_PLL_DIVA:
                clk->id = i;
                ret = clk_set_rate(clk, (parent->rate)/4);
                clk_enable(clk);
                break;
            case LVDS_PLL_DIVB:
                clk->id = i;
                ret = clk_set_rate(clk, (parent->rate)/5);
                clk_enable(clk);
                break;
            default:
                break;
        }
    }
    clk->id = id;
    return ret;
}

clk_rate_t clk_get_rate(clk_node_t *clk)
{
    clk_core_t *core = clk->core;
    clk_node_t *parent = NULL;
    clk_rate_t rate = 0;

    if (core == NULL) {
        ssdk_printf(SSDK_CRIT, "%s: core point is NULL!\n", __func__);
        return 0;
    }

    if (clk->parent == NULL) {
        ssdk_printf(SSDK_CRIT, "%s: clk node parent is NULL!\n", __func__);
        return 0;
    }

    parent = clk->parent;

    if (core->ops != NULL && core->ops->get_rate != NULL) {
        rate = core->ops->get_rate(clk);
    }
    else if (parent->core->ops != NULL &&
             parent->core->ops->get_rate != NULL) {
        rate = parent->core->ops->get_rate(parent);
    }

    return rate;
}

int clk_disable(clk_node_t *clk)
{
    clk_core_t *core = clk->core;

    if (core == NULL) {
        ssdk_printf(SSDK_CRIT, "%s: core point is NULL!\n", __func__);
        return -1;
    }

    if (core->ops->disable == NULL) {
        ssdk_printf(SSDK_WARNING, "%s: no disable function!\n", __func__);
        return -1;
    }

    return core->ops->disable(clk);
}

int clk_enable(clk_node_t *clk)
{
    clk_core_t *core = clk->core;

    if (core == NULL) {
        ssdk_printf(SSDK_CRIT, "%s: core point is NULL!\n", __func__);
        return -1;
    }

    if (core->ops->enable == NULL) {
        ssdk_printf(SSDK_WARNING, "%s: no enable function!\n", __func__);
        return -1;
    }

    return core->ops->enable(clk);
}

bool clk_is_gated(clk_node_t *clk)
{
    clk_core_t *core = clk->core;

    if (core == NULL) {
        ssdk_printf(SSDK_CRIT, "%s: core point is NULL!\n", __func__);
        return false;
    }

    if (core->ops->is_gated == NULL) {
        ssdk_printf(SSDK_WARNING, "%s: no is_gated function!\n", __func__);
        return false;
    }

    return core->ops->is_gated(clk);
}

bool clk_is_locked(clk_node_t *clk)
{
    clk_core_t *core = clk->core;

    if (core == NULL) {
        ssdk_printf(SSDK_CRIT, "%s: core point is NULL!\n", __func__);
        return false;
    }

    if (core->ops->is_locked == NULL) {
        ssdk_printf(SSDK_WARNING, "%s: no is_locked function!\n", __func__);
        return false;
    }

    return core->ops->is_locked(clk);
}