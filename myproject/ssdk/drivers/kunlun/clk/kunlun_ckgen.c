/**
 * @file kunlun_ckgen.c
 * @brief
 *
 * @copyright Copyright (c) 2020  Semidrive Semiconductor.
 *            All rights reserved.
 */

#include <reg.h>
#include <types.h>

#include "kunlun_ckgen.h"

#define CKGEN_POLLING_CYCLE_MAX (10000)

#define DIV_ROUND_UP(n, d)  (((n) + (d)/2) / (d))

static bool wait_for_bit_times(addr_t base, uint32_t offset, uint32_t val,
                               uint32_t count)
{
    volatile uint32_t times_out = count;

    while ((times_out != 0u) &&
           (val != ((readl(base) & ((uint32_t)1u << offset)) >> offset))) {
        times_out--;
    }

    return times_out > 0u ? true : false;
}

static inline uint32_t my_abs(int32_t a)
{
    return (a > 0) ? (uint32_t)a : (uint32_t)(-a);
}

static uint32_t ckgen_get_best_div(uint32_t f_ref, uint32_t freq)
{
    uint32_t clk_div;

    if (0u == freq) {
        return UINT32_MAX;
    }

    clk_div = DIV_ROUND_UP(f_ref, freq);

    /* The clk_div greater than 1 and less than UINT32_MAX. */
    return clk_div;
}

static bool ckgen_calc_2_div(uint32_t f_ref, uint32_t freq, uint32_t div1_max,
                             uint32_t *div1, uint32_t div2_max, uint32_t *div2)
{
    uint32_t clk_div;
    uint32_t i;
    uint32_t j;
    bool clock_div_succ = false;
    uint32_t div_bias = f_ref;
    uint32_t curr_div_bias;
    uint32_t tmp_u32;
    uint64_t tmp_u64;

    clk_div = ckgen_get_best_div(f_ref, freq);
    if (clk_div == UINT32_MAX) {
        return false;
    }

    for (i = 1u; i <= div1_max; i++) {
        tmp_u64 = (uint64_t)i * (uint64_t)freq;

        /* Div2 can't less than 1, and prevent value greater-than UINT32_MAX */
        if (tmp_u64 > (uint64_t)f_ref) {
            break;
        }

        tmp_u32 = (uint32_t)tmp_u64;

        j = ckgen_get_best_div(clk_div, i);
        if (j == UINT32_MAX || j > div2_max) {
            continue;
        }

        /* divisible */
        if (0u == f_ref % tmp_u32) {
            *div1 = i;
            *div2 = j;
            clock_div_succ = true;
            break;
        }

        /* Calculate the nearest value of div1&div2 which the freq need less than ref freq */
        curr_div_bias =
            my_abs((int32_t)((int64_t)tmp_u32 * (int64_t)j - (int64_t)f_ref));
        if (curr_div_bias < div_bias) {
            div_bias = curr_div_bias;
            *div1 = i;
            *div2 = j;
            clock_div_succ = true;
        }
    }

    return clock_div_succ;
}

#ifdef CONFIG_CLK_MONITOR
static ckgen_rate_t ckgen_mon_get_rate(addr_t ckgen_base, uint32_t id,
                                       ckgen_rate_t src_rate,
                                       uint32_t clk_type)
{
    uint32_t reg;
    uint32_t f_ref;
    uint32_t freq_mul_num;
    uint32_t count = CKGEN_POLLING_CYCLE_MAX;
    addr_t ctrl_base = ckgen_base + CKGEN_MON_CTRL;
    addr_t sel_base = ckgen_base + CKGEN_IP_MON_CTRL + ((clk_type * 4u) << 10u);

    /* Disable clk monitor. */
    RMWREG32(ctrl_base, CKGEN_MON_CTRL_CLK_DIS, 1u, 1u);
    /* Wait disable status bit equal 1. */
    if (!wait_for_bit_times(ctrl_base, CKGEN_MON_CTRL_CLK_DIS_STA, 1u, count)) {
        return 0u;
    }

    /* Set slice with the clk type and id. */
    RMWREG32(sel_base, CKGEN_MON_CTRL_SLICE_LSB, 16u, id);

    /* Set the monitor type. */
    RMWREG32(ctrl_base, CKGEN_MON_CTRL_SEL_LSB, 2u, clk_type);

    /* Select fast source, the souce clock freq is 24Mhz. */
    RMWREG32(ctrl_base, CKGEN_MON_CTRL_CLK_SRC, 1u, 0u);
    RMWREG32(ctrl_base, CKGEN_MON_CTRL_DIV_LSB, 4u, 0u);
    /* Wait div busy bit equal 0. */
    if (!wait_for_bit_times(ctrl_base, CKGEN_MON_CTRL_DIV_BUSY, 0u, count)) {
        return 0u;
    }

    f_ref = 24000000u * 1u;

    /* Enable clk monitor. */
    RMWREG32(ctrl_base, CKGEN_MON_CTRL_CLK_DIS, 1u, 0u);
    /* Wait disable status bit equal 0. */
    if (!wait_for_bit_times(ctrl_base, CKGEN_MON_CTRL_CLK_DIS_STA, 0u, count)) {
        return 0u;
    }

    /* Wait freq update status bit equal 1. */
    if (!wait_for_bit_times(ctrl_base, CKGEN_MON_CTRL_FREQ_READY, 1u, count)) {
        return 0u;
    }

    /* Enable clk monitor val update. */
    RMWREG32(ctrl_base, CKGEN_MON_CTRL_VAL_UPDATE, 1u, 1u);
    /* Wait val update val bit equal 1. */
    if (!wait_for_bit_times(ctrl_base, CKGEN_MON_CTRL_VAL_UPDATE, 1u, count)) {
        return 0u;
    }

    /* Read the freq mul num average. */
    reg = readl(ckgen_base + CKGEN_MON_AVE_FREQ);
    freq_mul_num = (reg >> CKGEN_MON_AVE_FREQ_LSB) & CKGEN_MON_AVE_FREQ_MASK;

    return (ckgen_rate_t)(f_ref) * (ckgen_rate_t)freq_mul_num;
}

ckgen_rate_t ckgen_ip_mon_get_rate(addr_t ckgen_base, uint32_t id,
                                   ckgen_rate_t src_rate)
{
    return ckgen_mon_get_rate(ckgen_base, id, src_rate,
                              CKGEN_MON_CTRL_SEL_IP);
}

ckgen_rate_t ckgen_bus_mon_get_rate(addr_t ckgen_base, uint32_t id,
                                    ckgen_rate_t src_rate)
{
    return ckgen_mon_get_rate(ckgen_base, id, src_rate,
                              CKGEN_MON_CTRL_SEL_BUS);
}

ckgen_rate_t ckgen_core_mon_get_rate(addr_t ckgen_base, uint32_t id,
                                     ckgen_rate_t src_rate)
{
    return ckgen_mon_get_rate(ckgen_base, id, src_rate,
                              CKGEN_MON_CTRL_SEL_CORE);
}
#endif

static void ckgen_ip_pre_enable(paddr_t ckgen_base, uint32_t id)
{
    uint32_t count = CKGEN_POLLING_CYCLE_MAX;
    addr_t base = CKGEN_IP_CTRL_BASE(ckgen_base, id);

    if (0u != (readl(base) & ((uint32_t)1u << CKGEN_IP_CTRL_CGEN))) {
        return;
    }

    RMWREG32(base, CKGEN_IP_CTRL_CGEN, 1u, 1u);

    /* Wait clock gating status bit equal 1. */
    (void)wait_for_bit_times(base, CKGEN_IP_CTRL_CGEN_STATUS, 1u, count);

    /* Clear the cgen status bit. */
    RMWREG32(base, CKGEN_IP_CTRL_CGEN_STATUS, 1u, 0u);

    return;
}

static void ckgen_ip_pre_disable(paddr_t ckgen_base, uint32_t id)
{
    uint32_t count = CKGEN_POLLING_CYCLE_MAX;
    addr_t base = CKGEN_IP_CTRL_BASE(ckgen_base, id);

    if (0u == (readl(base) & ((uint32_t)1u << CKGEN_IP_CTRL_CGEN))) {
        return;
    }

    RMWREG32(base, CKGEN_IP_CTRL_CGEN, 1u, 0u);

    /* Wait clock gating status bit equal 1. */
    (void)wait_for_bit_times(base, CKGEN_IP_CTRL_CGEN_STATUS, 1u, count);

    /* Clear the cgen status bit. */
    RMWREG32(base, CKGEN_IP_CTRL_CGEN_STATUS, 1u, 0u);

    return;
}

int ckgen_ip_set_rate(addr_t ckgen_base, uint32_t id,
                      ckgen_rate_t src_rate, ckgen_rate_t rate)
{
    addr_t base = CKGEN_IP_CTRL_BASE(ckgen_base, id);
    uint32_t count = CKGEN_POLLING_CYCLE_MAX;
    uint32_t pre_div;
    uint32_t post_div;
    uint32_t pre_div_max = CKGEN_IP_CTRL_PRE_DIV_MASK + 1u;
    uint32_t post_div_max = CKGEN_IP_CTRL_POST_DIV_MASK + 1u;

    if (!ckgen_calc_2_div(src_rate, rate, pre_div_max, &pre_div, post_div_max,
                          &post_div)) {
        return -1;
    }

    RMWREG32(base, CKGEN_IP_CTRL_PRE_DIV_LSB, 3u, pre_div - 1u);
    /* Wait clock pre div busy bit equal 0. */
    if (!wait_for_bit_times(base, CKGEN_IP_CTRL_PRE_BUSY, 0u, count)) {
        return -1;
    }

    RMWREG32(base, CKGEN_IP_CTRL_POST_DIV_LSB, 6u, post_div - 1u);
    if (!wait_for_bit_times(base, CKGEN_IP_CTRL_POST_BUSY, 0u, count)) {
        return -1;
    }

    return 0;
}

ckgen_rate_t ckgen_ip_get_rate(addr_t ckgen_base, uint32_t id,
                               ckgen_rate_t src_rate)
{
    uint32_t reg;

    uint32_t pre_div, post_div;
    addr_t base = CKGEN_IP_CTRL_BASE(ckgen_base, id);

    reg = readl(base);
    pre_div = (reg >> CKGEN_IP_CTRL_PRE_DIV_LSB) & CKGEN_IP_CTRL_PRE_DIV_MASK;
    post_div =
        (reg >> CKGEN_IP_CTRL_POST_DIV_LSB) & CKGEN_IP_CTRL_POST_DIV_MASK;

    return (ckgen_rate_t)src_rate /
        (ckgen_rate_t)((pre_div + 1u) * (post_div + 1u));
}

int ckgen_ip_prepare(paddr_t ckgen_base, uint32_t id, uint32_t mux,
                     ckgen_rate_t src_rate, ckgen_rate_t rate)
{
    addr_t base = CKGEN_IP_CTRL_BASE(ckgen_base, id);

    /* Disable the gating. */
    ckgen_ip_pre_disable(ckgen_base, id);

    RMWREG32(base, CKGEN_IP_CTRL_SRC_SEL_LSB, 3u, mux);

    /* Enable the gating. */
    ckgen_ip_pre_enable(ckgen_base, id);

    return ckgen_ip_set_rate(ckgen_base, id, src_rate, rate);
}

static void ckgen_bus_pre_enable(paddr_t ckgen_base, uint32_t id,
                                 uint32_t ab_sel)
{
    uint32_t cgen_offset;
    uint32_t cgen_status_offset;
    uint32_t count = CKGEN_POLLING_CYCLE_MAX;
    addr_t base = CKGEN_BUS_CTRL_BASE(ckgen_base, id);

    if (0u != ab_sel) {
        cgen_offset = CKGEN_BUS_CTRL_CGEN_B;
        cgen_status_offset = CKGEN_BUS_CTRL_CGEN_STATUS_B;
    } else {
        cgen_offset = CKGEN_BUS_CTRL_CGEN_A;
        cgen_status_offset = CKGEN_BUS_CTRL_CGEN_STATUS_A;
    }

    if (0u != (readl(base) & ((uint32_t)1u << cgen_offset))) {
        return;
    }

    /* Enable pre gating. */
    RMWREG32(base, cgen_offset, 1u, 1u);

    /* Wait clock gating status bit equal 1. */
    (void)wait_for_bit_times(base, cgen_status_offset, 1u, count);

    /* Clear the cgen status bit. */
    RMWREG32(base, cgen_status_offset, 1u, 0u);

    return;
}

static void ckgen_bus_pre_disable(paddr_t ckgen_base, uint32_t id,
                                  uint32_t ab_sel)
{
    uint32_t cgen_offset;
    uint32_t cgen_status_offset;
    uint32_t count = CKGEN_POLLING_CYCLE_MAX;
    addr_t base = CKGEN_BUS_CTRL_BASE(ckgen_base, id);

    if (0u != ab_sel) {
        cgen_offset = CKGEN_BUS_CTRL_CGEN_B;
        cgen_status_offset = CKGEN_BUS_CTRL_CGEN_STATUS_B;
    } else {
        cgen_offset = CKGEN_BUS_CTRL_CGEN_A;
        cgen_status_offset = CKGEN_BUS_CTRL_CGEN_STATUS_A;
    }

    if (0u == (readl(base) & ((uint32_t)1u << cgen_offset))) {
        return;
    }

    /* Disable pre gating. */
    RMWREG32(base, cgen_offset, 1u, 0u);

    /* Wait clock gating status bit equal 1. */
    (void)wait_for_bit_times(base, cgen_status_offset, 1u, count);

    /* Clear the cgen status bit. */
    RMWREG32(base, cgen_status_offset, 1u, 0u);

    return;
}

static int ckgen_bus_root_set_rate(addr_t ckgen_base, uint32_t id,
                                   ckgen_rate_t src_rate, ckgen_rate_t rate,
                                   uint32_t ab_sel)
{
    addr_t base = CKGEN_BUS_CTRL_BASE(ckgen_base, id);
    uint32_t count = CKGEN_POLLING_CYCLE_MAX;
    uint32_t pre_div;
    uint32_t pre_div_offset;
    uint32_t pre_div_busy_offset;
    uint32_t post_div;
    uint32_t pre_div_max = CKGEN_BUS_CTRL_PRE_DIV_MASK + 1u;
    uint32_t post_div_max = CKGEN_BUS_CTRL_POST_DIV_MASK + 1u;

    if (0u != ab_sel) {
        pre_div_offset = CKGEN_BUS_CTRL_PRE_DIV_B_LSB;
        pre_div_busy_offset = CKGEN_BUS_CTRL_PRE_BUSY_B;
    } else {
        pre_div_offset = CKGEN_BUS_CTRL_PRE_DIV_A_LSB;
        pre_div_busy_offset = CKGEN_BUS_CTRL_PRE_BUSY_A;
    }

    if (!ckgen_calc_2_div(src_rate, rate, pre_div_max, &pre_div, post_div_max,
                          &post_div)) {
        return -1;
    }

    RMWREG32(base, pre_div_offset, 3u, pre_div - 1u);
    /* Wait clock pre div busy bit equal 0. */
    if (!wait_for_bit_times(base, pre_div_busy_offset, 0u, count)) {
        return -1;
    }

    RMWREG32(base, CKGEN_BUS_CTRL_AB_SEL, 1u, ab_sel);

    RMWREG32(base, CKGEN_BUS_CTRL_POST_DIV_LSB, 6u, post_div - 1u);
    if (!wait_for_bit_times(base, CKGEN_CORE_CTRL_POST_BUSY, 0u, count)) {
        return -1;
    }

    return 0;
}

static ckgen_rate_t ckgen_bus_root_get_rate(addr_t ckgen_base, uint32_t id,
                                            ckgen_rate_t src_rate)
{
    uint32_t reg;
    uint32_t pre_div;
    uint32_t pre_div_offset;
    uint32_t post_div;
    /* 0: a path, 1: b path. */
    uint32_t ab_sel;
    addr_t base = CKGEN_BUS_CTRL_BASE(ckgen_base, id);

    reg = readl(base);
    ab_sel = (reg >> CKGEN_BUS_CTRL_AB_SEL) & 0x1u;

    if (0u != ab_sel) {
        pre_div_offset = CKGEN_BUS_CTRL_PRE_DIV_B_LSB;
    } else {
        pre_div_offset = CKGEN_BUS_CTRL_PRE_DIV_A_LSB;
    }

    reg = readl(base);
    pre_div = (reg >> pre_div_offset) & CKGEN_BUS_CTRL_PRE_DIV_MASK;
    post_div =
        (reg >> CKGEN_BUS_CTRL_POST_DIV_LSB) & CKGEN_BUS_CTRL_POST_DIV_MASK;

    return (ckgen_rate_t)src_rate /
        (ckgen_rate_t)((pre_div + 1u) * (post_div + 1u));
}

static int ckgen_bus_root_prepare(paddr_t ckgen_base, uint32_t id, uint32_t mux,
                                  ckgen_rate_t src_rate, ckgen_rate_t rate)
{
    uint32_t reg;
    /* 0: a path, 1: b path. */
    uint32_t ab_sel;
    addr_t base = CKGEN_BUS_CTRL_BASE(ckgen_base, id);

    reg = readl(base);
    ab_sel = (reg >> CKGEN_BUS_CTRL_AB_SEL) & 0x1u;

    /* Disable the gating. */
    ckgen_bus_pre_disable(ckgen_base, id, 1u - ab_sel);

    RMWREG32(base, CKGEN_BUS_CTRL_SRC_SEL_A_LSB + 16u * (1u - ab_sel), 3u, mux);

    /* Enable the gating. */
    ckgen_bus_pre_enable(ckgen_base, id, 1u - ab_sel);

    return ckgen_bus_root_set_rate(ckgen_base, id, src_rate, rate, 1u - ab_sel);
}

static int ckgen_bus_gasket_set_rate(paddr_t ckgen_base, uint32_t id,
                                     ckgen_rate_t src_rate, ckgen_rate_t rate)
{
    int ret = -1;
    uint32_t clk_div;
    uint32_t div_offset;
    uint32_t div_busy_offset;
    uint32_t count = CKGEN_POLLING_CYCLE_MAX;
    addr_t base = CKGEN_BUS_GASKET_BASE(ckgen_base, id);
    uint32_t type = CKGEN_BUS_ID_TYPE(ckgen_base, id);

    clk_div = ckgen_get_best_div(src_rate, rate);
    if (clk_div == UINT32_MAX) {
        return ret;
    }

    if (clk_div <= CKGEN_BUS_GASKET_DIV_MASK) {
        div_offset = (type - CKGEN_BUS_ID_TYPE_DIV_Q) * 3u;
        div_offset += CKGEN_BUS_GASKET_Q_DIV_LSB;
        div_busy_offset = (type - CKGEN_BUS_ID_TYPE_DIV_Q);
        div_busy_offset += CKGEN_BUS_GASKET_Q_DIV_BUSY;

        RMWREG32(base, div_offset, 3u, clk_div - 1u);
        /* Wait div busy bit. */
        if (!wait_for_bit_times(base, div_busy_offset, 0u, count)) {
            return -1;
        }
        ret = 0;
    }

    return ret;
}

static ckgen_rate_t ckgen_bus_gasket_get_rate(paddr_t ckgen_base, uint32_t id,
                                              ckgen_rate_t src_rate)
{
    uint32_t reg;
    uint32_t clk_div;
    uint32_t div_offset;
    addr_t base = CKGEN_BUS_GASKET_BASE(ckgen_base, id);
    uint32_t type = CKGEN_BUS_ID_TYPE(ckgen_base, id);

    div_offset = (type - CKGEN_BUS_ID_TYPE_DIV_Q) * 3u;
    div_offset += CKGEN_BUS_GASKET_Q_DIV_LSB;

    reg = readl(base);
    clk_div = (reg >> div_offset) & CKGEN_BUS_GASKET_DIV_MASK;

    return src_rate / (clk_div + 1u);
}

static int ckgen_bus_gasket_prepare(paddr_t ckgen_base, uint32_t id,
                                    ckgen_rate_t src_rate, ckgen_rate_t rate)
{
    /* Gasket can set fly. */
    return ckgen_bus_gasket_set_rate(ckgen_base, id, src_rate, rate);
}

int ckgen_bus_set_rate(addr_t ckgen_base, uint32_t id,
                       ckgen_rate_t src_rate, ckgen_rate_t rate)
{
    uint32_t reg;
    uint32_t ab_sel;
    addr_t base = CKGEN_BUS_CTRL_BASE(ckgen_base, id);
    uint32_t type = CKGEN_BUS_ID_TYPE(ckgen_base, id);

    if (type == CKGEN_BUS_ID_TYPE_DIV_ROOT) {
        reg = readl(base);
        ab_sel = (reg >> CKGEN_BUS_CTRL_AB_SEL) & 0x1u;

        /* Invert pre a/b path select. */
        return ckgen_bus_root_set_rate(ckgen_base, id, src_rate, rate,
                                       1u - ab_sel);
    } else {
        return ckgen_bus_gasket_set_rate(ckgen_base, id, src_rate, rate);
    }
}

ckgen_rate_t ckgen_bus_get_rate(addr_t ckgen_base, uint32_t id,
                                ckgen_rate_t src_rate)
{
    uint32_t type = CKGEN_BUS_ID_TYPE(ckgen_base, id);

    if (type == CKGEN_BUS_ID_TYPE_DIV_ROOT) {
        return ckgen_bus_root_get_rate(ckgen_base, id, src_rate);
    } else {
        return ckgen_bus_gasket_get_rate(ckgen_base, id, src_rate);
    }
}

int ckgen_bus_prepare(paddr_t ckgen_base, uint32_t id, uint32_t mux,
                      ckgen_rate_t src_rate, ckgen_rate_t rate)
{
    uint32_t type = CKGEN_BUS_ID_TYPE(ckgen_base, id);

    if (type == CKGEN_BUS_ID_TYPE_DIV_ROOT) {
        return ckgen_bus_root_prepare(ckgen_base, id, mux, src_rate, rate);
    } else {
        return ckgen_bus_gasket_prepare(ckgen_base, id, src_rate, rate);
    }
}

static void ckgen_core_pre_enable(paddr_t ckgen_base, uint32_t id,
                                  uint32_t ab_sel)
{
    uint32_t cgen_offset;
    uint32_t cgen_status_offset;
    uint32_t count = CKGEN_POLLING_CYCLE_MAX;
    addr_t base = CKGEN_CORE_CTRL_BASE(ckgen_base, id);

    if (0u != ab_sel) {
        cgen_offset = CKGEN_CORE_CTRL_CGEN_B;
        cgen_status_offset = CKGEN_CORE_CTRL_CGEN_STATUS_B;
    } else {
        cgen_offset = CKGEN_CORE_CTRL_CGEN_A;
        cgen_status_offset = CKGEN_CORE_CTRL_CGEN_STATUS_A;
    }

    if (0u != (readl(base) & ((uint32_t)1u << cgen_offset))) {
        return;
    }

    /* Enable pre gating. */
    RMWREG32(base, cgen_offset, 1u, 1u);

    /* Wait clock gating status bit equal 1. */
    (void)wait_for_bit_times(base, cgen_status_offset, 1u, count);

    /* Clear the cgen status bit. */
    RMWREG32(base, cgen_status_offset, 1u, 0u);

    return;
}

static void ckgen_core_pre_disable(paddr_t ckgen_base, uint32_t id,
                                   uint32_t ab_sel)
{
    uint32_t cgen_offset;
    uint32_t cgen_status_offset;
    uint32_t count = CKGEN_POLLING_CYCLE_MAX;
    addr_t base = CKGEN_CORE_CTRL_BASE(ckgen_base, id);

    if (0u != ab_sel) {
        cgen_offset = CKGEN_CORE_CTRL_CGEN_B;
        cgen_status_offset = CKGEN_CORE_CTRL_CGEN_STATUS_B;
    } else {
        cgen_offset = CKGEN_CORE_CTRL_CGEN_A;
        cgen_status_offset = CKGEN_CORE_CTRL_CGEN_STATUS_A;
    }

    if (0u == (readl(base) & ((uint32_t)1u << cgen_offset))) {
        return;
    }

    /* Disable pre gating. */
    RMWREG32(base, cgen_offset, 1u, 0u);

    /* Wait clock gating status bit equal 1. */
    (void)wait_for_bit_times(base, cgen_status_offset, 1u, count);

    /* Clear the cgen status bit. */
    RMWREG32(base, cgen_status_offset, 1u, 0u);

    return;
}

int ckgen_core_set_rate(addr_t ckgen_base, uint32_t id,
                        ckgen_rate_t src_rate, ckgen_rate_t rate)
{
    int ret = -1;
    uint32_t reg;
    uint32_t count = CKGEN_POLLING_CYCLE_MAX;
    addr_t base = CKGEN_CORE_CTRL_BASE(ckgen_base, id);
    uint32_t post_div;
    uint32_t ab_sel;

    reg = readl(base);
    ab_sel = (reg >> CKGEN_CORE_CTRL_AB_SEL) & 0x1u;

    post_div = ckgen_get_best_div(src_rate, rate);
    if (post_div == UINT32_MAX) {
        return ret;
    }

    /* Invert pre a/b path select. */
    RMWREG32(base, CKGEN_CORE_CTRL_AB_SEL, 1u, 1u - ab_sel);

    RMWREG32(base, CKGEN_CORE_CTRL_POST_DIV_LSB, 6u, post_div - 1u);
    if (!wait_for_bit_times(base, CKGEN_CORE_CTRL_POST_BUSY, 0u, count)) {
        return -1;
    }

    return 0;
}

ckgen_rate_t ckgen_core_get_rate(addr_t ckgen_base, uint32_t id,
                                 ckgen_rate_t src_rate)
{
    uint32_t reg;
    uint32_t post_div;
    addr_t base = CKGEN_CORE_CTRL_BASE(ckgen_base, id);

    reg = readl(base);
    post_div =
        (reg >> CKGEN_CORE_CTRL_POST_DIV_LSB) & CKGEN_CORE_CTRL_POST_DIV_MASK;

    return (ckgen_rate_t)src_rate / (post_div + 1u);
}

int ckgen_core_prepare(paddr_t ckgen_base, uint32_t id, uint32_t mux,
                       ckgen_rate_t src_rate, ckgen_rate_t rate)
{
    uint32_t reg;
    /* 0: a path, 1: b path. */
    uint32_t ab_sel;
    addr_t base = CKGEN_CORE_CTRL_BASE(ckgen_base, id);

    reg = readl(base);
    ab_sel = (reg >> CKGEN_CORE_CTRL_AB_SEL) & 0x1u;
    /* Invert pre a/b path select. */
    ab_sel = 1u - ab_sel;

    /* Disable the gating. */
    ckgen_core_pre_disable(ckgen_base, id, ab_sel);

    RMWREG32(base, CKGEN_CORE_CTRL_SRC_SEL_A_LSB + 16u * ab_sel, 3u, mux);

    /* Enable the gating. */
    ckgen_core_pre_enable(ckgen_base, id, ab_sel);

    return ckgen_core_set_rate(ckgen_base, id, src_rate, rate);
}

int ckgen_lp_gating_enable(paddr_t ckgen_base, uint32_t id)
{
    addr_t base = CKGEN_LP_GATING_BASE(ckgen_base, id);

    /* Unlock the gating. */
    RMWREG32(base, CKGEN_LP_GATING_LOCK, 1u, 0);
    /* Enable the gating. */
    RMWREG32(base, CKGEN_LP_GATING_SW_DIS, 1u, 0);
    /* Lock the gating. */
    RMWREG32(base, CKGEN_LP_GATING_LOCK, 1u, 0);

    return 0;
}

int ckgen_lp_gating_disable(paddr_t ckgen_base, uint32_t id)
{
    addr_t base = CKGEN_LP_GATING_BASE(ckgen_base, id);

    /* Unlock the gating. */
    RMWREG32(base, CKGEN_LP_GATING_LOCK, 1u, 0);
    /* Disable the gating. */
    RMWREG32(base, CKGEN_LP_GATING_SW_DIS, 1u, 1);
    /* Lock the gating. */
    RMWREG32(base, CKGEN_LP_GATING_LOCK, 1u, 0);

    return 0;
}

int ckgen_uuu_set_rate(paddr_t ckgen_base, uint32_t id,
                       ckgen_rate_t src_rate, ckgen_rate_t rate)
{
    int ret = -1;
    uint32_t reg;
    uint32_t clk_div;
    uint32_t div_offset;
    uint32_t div_m;
    addr_t base = CKGEN_UUU_CTRL_BASE(ckgen_base, id);
    uint32_t type = CKGEN_UUU_ID_TYPE(ckgen_base, id);

    /* div n/p/q source clock is div_m out */
    if (type < CKGEN_UUU_ID_TYPE_DIV_M) {
        reg = readl(base);
        div_m = (reg >> CKGEN_UUU_CTRL_M_DIV_LSB) & CKGEN_UUU_CTRL_DIV_MASK;

        clk_div = ckgen_get_best_div(src_rate, rate * (div_m + 1u));
    } else {
        clk_div = ckgen_get_best_div(src_rate, rate);
    };

    if (clk_div == UINT32_MAX) {
        return ret;
    }

    if (clk_div <= CKGEN_UUU_CTRL_DIV_MASK) {
        div_offset = (type - CKGEN_UUU_ID_TYPE_DIV_Q) * 4u;
        div_offset += CKGEN_UUU_ID_TYPE_DIV_Q;
        RMWREG32(base, div_offset, 4u, clk_div - 1u);
        ret = 0;
    }

    return ret;
}

ckgen_rate_t ckgen_uuu_get_rate(paddr_t ckgen_base, uint32_t id,
                                ckgen_rate_t src_rate)
{
    uint32_t reg;
    uint32_t clk_div = 1;
    uint32_t div_offset;
    uint32_t div_m;
    addr_t base = CKGEN_UUU_CTRL_BASE(ckgen_base, id);
    uint32_t type = CKGEN_UUU_ID_TYPE(ckgen_base, id);

    reg = readl(base);
    div_m = ((reg >> CKGEN_UUU_CTRL_M_DIV_LSB) & CKGEN_UUU_CTRL_DIV_MASK) + 1u;

    if (type < CKGEN_UUU_ID_TYPE_DIV_M) {
        div_offset = (type - CKGEN_UUU_ID_TYPE_DIV_Q) * 4u;
        div_offset += CKGEN_UUU_ID_TYPE_DIV_Q;
        clk_div = ((reg >> div_offset) & CKGEN_UUU_CTRL_DIV_MASK) + 1u;
        clk_div = clk_div * div_m;
    } else if (type == CKGEN_UUU_ID_TYPE_DIV_M) {
        clk_div = div_m;
    }

    return src_rate / clk_div;
}

int ckgen_uuu_prepare(paddr_t ckgen_base, uint32_t id, uint32_t mux,
                      ckgen_rate_t src_rate, ckgen_rate_t rate)
{
    int ret = -1;
    uint32_t reg;
    addr_t base = CKGEN_UUU_CTRL_BASE(ckgen_base, id);

    /* Div m/n/p/q can set fly. */
    ret = ckgen_uuu_set_rate(ckgen_base, id, src_rate, rate);

    if (ret == 0) {
        reg = readl(base);
        if (mux != ((reg >> CKGEN_UUU_CTRL_SEL0) & 0x1u)) {
            /* Select the uuu source ref clock to ckgen or pll. */
            RMWREG32(base, CKGEN_UUU_CTRL_SEL0, 1u, mux);
        }
    }

    return ret;
}
