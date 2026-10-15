/**
 * @file cpufreq.c
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <cmsis_os2.h>
#include <debug.h>
#include <param.h>
#include <board.h>
#include <reg.h>
#include <cpufreq.h>
#include <driver/clk.h>
#include <clock_cfg.h>

#define CPUFREQ_MODIFY_SF        0
#define CPUFREQ_MODIFY_SP        1
#define CPUFREQ_MODIFY_AP_BUS    2
#define CPUFREQ_MODIFY_DISP_BUS  3
#define CPUFREQ_MODIFY_SEIP_BUS  4

#define CKGEN_MAX_PRE_DIV        32
#define CKGEN_MAX_DIV_M          16

#define DIV_ROUND_CLOSEST(n, d)  ((((n) < 0) ^ ((d) < 0)) ? (((n) - (d)/2)/(d)) : (((n) + (d)/2)/(d)))
#define abs(x)  (((int)(x) < 0) ? -(int)(x) : (int)(x))
#define RET_CHECK(r)  do { if ((r)) { return -1; } } while(0)

static void cpufreq_print_clk_node_info(struct clk_node *node)
{
    ssdk_printf(SSDK_DEBUG, "clk_node: id=0x%x, rate=%d, mux=%d\n",
                node->id, node->rate, node->mux);
}

static uint32_t cpufreq_get_best_div(uint32_t f_ref, uint32_t freq)
{
    uint32_t clk_div;

    if (0u == freq) {
        return UINT32_MAX;
    }

    clk_div = DIV_ROUND_CLOSEST(f_ref, freq);

    return clk_div;
}

static bool cpufreq_is_better_rate(uint32_t rate, uint32_t now, uint32_t best)
{
    return abs(now - rate) <= abs(best - rate);
}

static int cpufreq_modify_ckgen_node(taishan_ckgen_node_t *ckgen_node, uint32_t rate, uint8_t mux)
{
    ASSERT((ckgen_node) && (mux < ckgen_node->clk_node.parents_num));

    ckgen_node->clk_node.rate = rate;
    ckgen_node->clk_node.mux = mux;

    if (!clk_prepare(CLK_NODE((*ckgen_node)))) {
        cpufreq_print_clk_node_info(CLK_NODE((*ckgen_node)));
        return 0;
    }
    else {
        ssdk_printf(SSDK_CRIT, "%s: ckgen id 0x%x, set rate %d, mux %d, prepare fail\n",
                    __func__, ckgen_node->clk_node.id, rate, mux);
        return -1;
    }
}

static int cpufreq_search_parent_div(taishan_ckgen_node_t *div_root, uint32_t expect_rate,
                                     uint8_t *mux, uint32_t *div_pre, uint32_t *div_m)
{
    uint32_t best_rate = 0;

    ASSERT(mux && div_pre && div_m);
    ASSERT((div_root) && (div_root->clk_node.parents_num == 5));

    /* RC24M exclude */
    for (uint8_t i = 1; i < div_root->clk_node.parents_num; i++) {
        clk_node_t *parent;
        uint32_t div, calc_rate;

        parent = div_root->clk_node.parents[i];

        if ((!parent) || (parent->rate < expect_rate)) {
            continue;
        }

        div = cpufreq_get_best_div(parent->rate, expect_rate);

        if (div == UINT32_MAX ||
            (i == 4 && div > CKGEN_MAX_DIV_M) ||
            (i != 4 && div > (CKGEN_MAX_PRE_DIV * CKGEN_MAX_DIV_M))) {
            continue;
        }

        calc_rate = parent->rate / div;

        if (cpufreq_is_better_rate(expect_rate, calc_rate, best_rate)) {
            best_rate = calc_rate;
            *mux = i;

            if (i == 4) {
                *div_pre = 1;
                *div_m = div;
            }
            else if (div <= CKGEN_MAX_PRE_DIV) {
                *div_pre = div;
                *div_m = 1;
            }
            else {
                *div_pre = CKGEN_MAX_PRE_DIV;
                *div_m = div / CKGEN_MAX_PRE_DIV;
            }
        }
    }

    return (best_rate != 0) ? 0 : 1;
}

static int cpufreq_modify_sf_sp_ap_bus(uint32_t freq, uint32_t type)
{
    taishan_ckgen_node_t *div_root = NULL;
    taishan_ckgen_node_t *div_m = NULL;
    taishan_ckgen_node_t *div_n = NULL;
    taishan_ckgen_node_t *div_p = NULL;
    taishan_ckgen_node_t *div_q = NULL;
    taishan_ckgen_node_t *root = NULL;
    uint32_t d_pre = 1;
    uint32_t d_m = 1;
    uint8_t mux = 0;

    ASSERT(type <= CPUFREQ_MODIFY_SEIP_BUS);

    switch (type) {
#ifdef CONFIG_PART_SF
        case CPUFREQ_MODIFY_SF:
            div_root = &g_ckgen_bus_cr5_sf_div_root;
            div_m = &g_ckgen_bus_cr5_sf_div_m;
            div_n = &g_ckgen_bus_cr5_sf_div_n;
            div_p = &g_ckgen_bus_cr5_sf_div_p;
            div_q = &g_ckgen_bus_cr5_sf_div_q;
            root = &g_ckgen_bus_cr5_sf_root;
        break;
#endif

#ifdef CONFIG_PART_SP
        case CPUFREQ_MODIFY_SP:
            div_root = &g_ckgen_bus_cr5_sp_div_root;
            div_m = &g_ckgen_bus_cr5_sp_div_m;
            div_n = &g_ckgen_bus_cr5_sp_div_n;
            div_p = &g_ckgen_bus_cr5_sp_div_p;
            div_q = &g_ckgen_bus_cr5_sp_div_q;
            root = &g_ckgen_bus_cr5_sp_root;
        break;
#endif

        case CPUFREQ_MODIFY_AP_BUS:
            div_root = &g_ckgen_bus_ap_bus_div_root;
            div_m = &g_ckgen_bus_ap_bus_div_m;
            div_n = &g_ckgen_bus_ap_bus_div_n;
            div_p = &g_ckgen_bus_ap_bus_div_p;
            div_q = &g_ckgen_bus_ap_bus_div_q;
            root = &g_ckgen_bus_ap_bus_root;
        break;

        case CPUFREQ_MODIFY_DISP_BUS:
            div_root = &g_ckgen_bus_disp_bus_div_root;
            div_m = &g_ckgen_bus_disp_bus_div_m;
            div_n = &g_ckgen_bus_disp_bus_div_n;
            div_p = &g_ckgen_bus_disp_bus_div_p;
            div_q = &g_ckgen_bus_disp_bus_div_q;
            root = &g_ckgen_bus_disp_bus_root;
        break;

        case CPUFREQ_MODIFY_SEIP_BUS:
            div_root = &g_ckgen_bus_seip_div_root;
            div_m = &g_ckgen_bus_seip_div_m;
            div_n = &g_ckgen_bus_seip_div_n;
            div_p = &g_ckgen_bus_seip_div_p;
            div_q = &g_ckgen_bus_seip_div_q;
            root = &g_ckgen_bus_seip_root;
        break;

        default:
            ASSERT(0);
        break;
    }

    if (cpufreq_search_parent_div(div_root, freq, &mux, &d_pre, &d_m)) {
        ssdk_printf(SSDK_CRIT, "%s: type %d, frequency %d search parent fail\n",
                    __func__, type, freq);
        return -1;
    }

    /* cpu/axi/apb default ratio 4/2/1 */
    RET_CHECK(cpufreq_modify_ckgen_node(div_root, div_root->clk_node.parents[mux]->rate / d_pre, mux));
    RET_CHECK(cpufreq_modify_ckgen_node(div_m, div_root->clk_node.rate / d_m, div_m->clk_node.mux));
    RET_CHECK(cpufreq_modify_ckgen_node(div_n, div_m->clk_node.rate / 2, div_n->clk_node.mux));
    RET_CHECK(cpufreq_modify_ckgen_node(div_p, div_m->clk_node.rate / 4, div_p->clk_node.mux));
    RET_CHECK(cpufreq_modify_ckgen_node(div_q, div_m->clk_node.rate / 4, div_q->clk_node.mux));
    RET_CHECK(cpufreq_modify_ckgen_node(root, div_m->clk_node.rate, root->clk_node.mux));

    return 0;
}

#ifdef CONFIG_PART_SX
static int cpufreq_modify_sx_core(uint32_t expect_rate)
{
    uint32_t best_rate = 0;
    uint8_t mux;
    taishan_ckgen_node_t *sx_root = &g_ckgen_core_cr5_sx;

    ASSERT(sx_root->clk_node.parents_num == 5);

    /* RC24M exclude */
    for (uint8_t i = 1; i < sx_root->clk_node.parents_num; i++) {
        clk_node_t *parent;
        uint32_t div, calc_rate;

        parent = sx_root->clk_node.parents[i];

        if ((!parent) || (parent->rate < expect_rate)) {
            continue;
        }

        if (i != 4) {
            div = cpufreq_get_best_div(parent->rate, expect_rate);

            if (div == UINT32_MAX || (div > CKGEN_MAX_PRE_DIV)) {
                continue;
            }
        }
        else {
            div = 1;
        }

        calc_rate = parent->rate / div;

        if (cpufreq_is_better_rate(expect_rate, calc_rate, best_rate)) {
            best_rate = calc_rate;
            mux = i;
        }
    }

    if (best_rate) {
        return cpufreq_modify_ckgen_node(sx_root, best_rate, mux);
    }
    else {
        ssdk_printf(SSDK_CRIT, "%s: sx expect frequency %d search parent fail\n",
                    __func__, expect_rate);
        return -1;
    }
}
#endif

int cpufreq_set(struct cpufreq *cpu_f)
{
    int ret = 0;

    ASSERT(cpu_f);

#ifdef CONFIG_PART_SF
    ret |= cpufreq_modify_sf_sp_ap_bus(cpu_f->sf_freq, CPUFREQ_MODIFY_SF);
#endif

#ifdef CONFIG_PART_SP
    ret |= cpufreq_modify_sf_sp_ap_bus(cpu_f->sp_freq, CPUFREQ_MODIFY_SP);
#endif

#ifdef CONFIG_PART_SX
    ret |= cpufreq_modify_sx_core(cpu_f->sx_freq);
#endif

    ret |= cpufreq_modify_sf_sp_ap_bus(cpu_f->ap_bus, CPUFREQ_MODIFY_AP_BUS);
    ret |= cpufreq_modify_sf_sp_ap_bus(cpu_f->disp_bus, CPUFREQ_MODIFY_DISP_BUS);
    ret |= cpufreq_modify_sf_sp_ap_bus(cpu_f->seip_bus, CPUFREQ_MODIFY_SEIP_BUS);

    return ret;
}

int cpufreq_get(struct cpufreq *cpu_f)
{
    ASSERT(cpu_f);

#ifdef CONFIG_PART_SF
    cpu_f->sf_freq = clk_get_rate(CLK_NODE(g_ckgen_bus_cr5_sf_root));
#endif

#ifdef CONFIG_PART_SP
    cpu_f->sp_freq = clk_get_rate(CLK_NODE(g_ckgen_bus_cr5_sp_root));
#endif

#ifdef CONFIG_PART_SX
    cpu_f->sx_freq = clk_get_rate(CLK_NODE(g_ckgen_core_cr5_sx));
#endif

    cpu_f->ap_bus = clk_get_rate(CLK_NODE(g_ckgen_bus_ap_bus_root));
    cpu_f->disp_bus = clk_get_rate(CLK_NODE(g_ckgen_bus_disp_bus_root));
    cpu_f->seip_bus = clk_get_rate(CLK_NODE(g_ckgen_bus_seip_root));

    return 0;
}
