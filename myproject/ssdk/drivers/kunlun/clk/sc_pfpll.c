/**
 * @file sc_pfpll.c
 * @brief
 *
 * @copyright Copyright (c) 2020  Semidrive Semiconductor.
 *            All rights reserved.
 */

#include <reg.h>
#include <types.h>

#include "sc_pfpll.h"

#define DIV_ROUND_UP(n, d) (((n) + (d)-1u) / (d))

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

static uint32_t pll_get_best_div(uint32_t f_ref, uint32_t freq)
{
    uint32_t pll_div;

    if (0u == freq) {
        return UINT32_MAX;
    }

    pll_div = DIV_ROUND_UP(f_ref, freq);

    if (pll_div > 1u) {
        if ((pll_div * freq - f_ref) > (f_ref - (pll_div - 1u) * freq)) {
            pll_div--;
        }
    }

    /* The pll_div greater than 1 and less than UINT32_MAX. */
    return pll_div;
}

static bool pll_calc_2_div(uint32_t f_ref, uint32_t freq, uint32_t div1_max,
                           uint32_t *div1, uint32_t div2_max, uint32_t *div2)
{
    uint32_t pll_div;
    uint32_t i;
    uint32_t j;
    bool clock_div_succ = false;
    uint32_t div_bias = f_ref;
    uint32_t curr_div_bias;
    uint32_t tmp_u32;
    uint64_t tmp_u64;

    pll_div = pll_get_best_div(f_ref, freq);
    if (pll_div == UINT32_MAX) {
        return false;
    }

    for (i = 1u; i <= div2_max; i++) {
        tmp_u64 = (uint64_t)i * (uint64_t)freq;

        /* Div2 can't less than 1, and prevent value greater-than UINT32_MAX */
        if (tmp_u64 > (uint64_t)f_ref) {
            break;
        }

        tmp_u32 = (uint32_t)tmp_u64;

        j = pll_get_best_div(pll_div, i);
        if (j == UINT32_MAX) {
            continue;
        }

        /* Div2 should less than or equal to div1. */
        if (j < i || j > div1_max) {
            continue;
        }

        /* divisible */
        if (0u == f_ref % tmp_u32) {
            *div1 = j;
            *div2 = i;
            clock_div_succ = true;
            break;
        }

        /* Calculate the nearest value of div1&div2 which the freq need less than ref freq */
        curr_div_bias =
            my_abs((int32_t)((int64_t)tmp_u32 * (int64_t)j - (int64_t)f_ref));
        if (curr_div_bias < div_bias) {
            div_bias = curr_div_bias;
            *div1 = j;
            *div2 = i;
            clock_div_succ = true;
        }
    }

    return clock_div_succ;
}

static uint32_t pll_get_fvco(paddr_t pll_base, pll_rate_t src_rate)
{
    uint32_t reg;
    uint32_t f_vco;
    uint32_t ref_div;
    uint32_t fb_div;
    uint32_t frac_div;
    bool is_int_mode;
    uint32_t tmp_u32;
    uint64_t tmp_u64;

    addr_t base = pll_base;

    reg = readl(base + PLL_CTRL);

    if (0u == (reg >> PLL_CTRL_INT_MODE & 0x1u)) {
        is_int_mode = false;
    } else {
        is_int_mode = true;
    }

    reg = readl(base + PLL_DIV);
    ref_div = (reg >> PLL_DIV_REFDIV_LSB) & PLL_DIV_REFDIV_MASK;

    reg = readl(base + PLL_FBDIV);
    fb_div = (reg >> PLL_FBDIV_LSB) & PLL_FBDIV_MASK;

    tmp_u32 = src_rate / ref_div;
    /* Integer part. */
    f_vco = tmp_u32 * fb_div;

    if (!is_int_mode) {
        reg = readl(base + PLL_FRAC);
        frac_div = (reg >> PLL_FRAC_LSB) & PLL_FRAC_MASK;
        /* Add the fractional part. */
        tmp_u64 = (uint64_t)tmp_u32;
        f_vco = f_vco + (uint32_t)((tmp_u64 * (uint64_t)frac_div) >> 24u);
    }

    return f_vco;
}

static void pll_vco_enable(paddr_t pll_base)
{
    uint32_t reg;
    addr_t base = pll_base;
    /* Enable PLL */
    RMWREG32(base + PLL_CTRL, PLL_CTRL_PLLEN, 1u, 1u);

    /* Wait until PLL is locked. */
    (void)wait_for_bit_times(base + PLL_CTRL, 1u, PLL_CTRL_LOCK, 10000u);

    reg = readl(base + PLL_CTRL);
    reg |= (1u << PLL_CTRL_FOUTPOSTDIVEN) + (1u << PLL_CTRL_FOUT4PHASEEN);
    writel(reg, base + PLL_CTRL);
}

static void pll_out_enable(paddr_t pll_base, uint32_t id)
{
    uint32_t reg;
    addr_t base = pll_base;

    reg = readl(base + PLL_CTRL);
    if (id == SC_PFPLL_ID_NUM_ROOT) {
        reg |= (uint32_t)1u << PLL_CTRL_PLLPOSTCG_EN;
    } else {
        reg |= (uint32_t)1u
            << ((id - SC_PFPLL_ID_NUM_DIVA) + PLL_CTRL_PLL_DIVA_CG_EN);
    }

    /* Enable PLL out enable. */
    writel(reg, base + PLL_CTRL);
}

static void pll_vco_disable(paddr_t pll_base)
{
    uint32_t reg;
    addr_t base = pll_base;

    reg = readl(base + PLL_CTRL);
    reg &= ~(1 << PLL_CTRL_PLLPOSTCG_EN | 1 << PLL_CTRL_PLL_DIVA_CG_EN | 1 << PLL_CTRL_PLL_DIVB_CG_EN
            | 1 << PLL_CTRL_PLL_DIVC_CG_EN | 1 << PLL_CTRL_PLL_DIVD_CG_EN);
    reg &= ~((1u << PLL_CTRL_FOUTPOSTDIVEN) + (1u << PLL_CTRL_FOUT4PHASEEN));
    writel(reg, base + PLL_CTRL);

    /* Disable PLL. */
    RMWREG32(base + PLL_CTRL, PLL_CTRL_PLLEN, 1u, 0u);
}

static void pll_out_disable(paddr_t pll_base, uint32_t id)
{
    uint32_t reg;
    addr_t base = pll_base;

    reg = readl(base + PLL_CTRL);
    if (id == SC_PFPLL_ID_NUM_ROOT) {
        reg &= ~((uint32_t)1u << PLL_CTRL_PLLPOSTCG_EN);
    } else {
        reg &= ~((uint32_t)1u
                 << (PLL_CTRL_PLL_DIVA_CG_EN + (id - SC_PFPLL_ID_NUM_DIVA)));
    }

    /* Disable PLL out enable. */
    writel(reg, base + PLL_CTRL);
}

static int pll_vco_prepare(paddr_t pll_base, pll_rate_t src_rate, pll_rate_t rate)
{
    int ret = -1;
    uint32_t frac_div = ((uint32_t)1u << 24u) - 1u;
    uint32_t ref_div, ref_div_result;
    uint32_t fb_div;
    uint32_t fb_div_result;
    uint32_t freq_bias = rate;
    uint32_t curr_freq_bias;
    uint32_t tmp_u32;
    uint64_t tmp_u64;
    addr_t base = pll_base;
    uint32_t val;

    for (ref_div = PLL_REFDIV_MIN; ref_div <= PLL_REFDIV_MAX; ref_div++) {
        /* Not deal with cases which not divisible. */
        if (0u != src_rate % ref_div) {
            continue;
        }

        tmp_u32 = src_rate / ref_div;

        /* PLL integer mode limit. */
        if (tmp_u32 < 5000000u) {
            break;
        }

        /* If can't be dvivided exactly by integer, use fractional mode, so use round down here. */
        fb_div = rate / tmp_u32;

        if (fb_div >= 16u && fb_div <= 640u) {
            /* PLL integer mode. */
            if (0u == rate % tmp_u32) {
                ref_div_result = ref_div;
                fb_div_result = fb_div;
                frac_div = 0u;
                ret = 0;
                break;
            }

            /* PLL fractional mode limit. */
            if (tmp_u32 < 10000000u) {
                continue;
            }

            /* PLL fractional mode. */
            curr_freq_bias = my_abs((int32_t)(
                (int64_t)tmp_u32 * (int64_t)fb_div - (int64_t)rate));

            if (curr_freq_bias < freq_bias && fb_div >= 20u && fb_div <= 320u) {
                freq_bias = curr_freq_bias;
                ref_div_result = ref_div;
                fb_div_result = fb_div;
                ret = 0;
            }
        }
    }

    if (0 != ret) {
        return ret;
    }

    /* disable PLL */
    val = readl(base + PLL_CTRL);
    if((val & (1 << PLL_CTRL_PLLEN)) == (1 << PLL_CTRL_PLLEN))
    {
        val &= ~(1 << PLL_CTRL_PLLPOSTCG_EN | 1 << PLL_CTRL_PLL_DIVA_CG_EN | 1 << PLL_CTRL_PLL_DIVB_CG_EN
            | 1 << PLL_CTRL_PLL_DIVC_CG_EN | 1 << PLL_CTRL_PLL_DIVD_CG_EN);
        writel(val, base + PLL_CTRL);
        val = readl(base + PLL_CTRL);
        val &= ~(1 << PLL_CTRL_PLLEN);
        writel(val, base + PLL_CTRL);
    }

    /* Don't enable bypass mode */
    RMWREG32(base + PLL_CTRL, PLL_CTRL_BAPASS, 1u, 0u);

    if (frac_div != 0u) {
        RMWREG32(base + PLL_CTRL, PLL_CTRL_INT_MODE, 1u, 0u);
        RMWREG32(base + PLL_CTRL, PLL_CTRL_DACEN, 1u, 1u);

        /* Calculate fractional div. */
        tmp_u32 = src_rate / ref_div_result;
        tmp_u64 = ((uint64_t)rate << 24u) / (uint64_t)tmp_u32;
        tmp_u64 = tmp_u64 - ((uint64_t)fb_div << 24u);
        frac_div = (uint32_t)tmp_u64;
        writel(frac_div, base + PLL_FRAC);
    } else {
        RMWREG32(base + PLL_CTRL, PLL_CTRL_INT_MODE, 1u, 1u);
    }

    writel(fb_div_result, base + PLL_FBDIV);
    RMWREG32(base + PLL_DIV, PLL_DIV_REFDIV_LSB, 24u, ref_div_result);

    /* Enable the VCO. */
    pll_vco_enable(pll_base);

    return ret;
}

static bool pll_set_vco_rate(paddr_t pll_base, pll_rate_t src_rate, pll_rate_t rate)
{
    int ret = -1;
    uint32_t frac_div = ((uint32_t)1u << 24u) - 1u;
    uint32_t ref_div, ref_div_result;
    uint32_t fb_div;
    uint32_t fb_div_result;
    uint32_t freq_bias = rate;
    uint32_t curr_freq_bias;
    uint32_t tmp_u32;
    uint64_t tmp_u64;
    addr_t base = pll_base;

    for (ref_div = PLL_REFDIV_MIN; ref_div <= PLL_REFDIV_MAX; ref_div++) {
        /* Not deal with cases which not divisible. */
        if (0u != src_rate % ref_div) {
            continue;
        }

        tmp_u32 = src_rate / ref_div;

        /* PLL integer mode limit. */
        if (tmp_u32 < 5000000u) {
            break;
        }

        /* If can't be dvivided exactly by integer, use fractional mode, so use round down here. */
        fb_div = rate / tmp_u32;

        if (fb_div >= 16u && fb_div <= 640u) {
            /* PLL integer mode. */
            if (0u == rate % tmp_u32) {
                ref_div_result = ref_div;
                fb_div_result = fb_div;
                frac_div = 0u;
                ret = 0;
                break;
            }

            /* PLL fractional mode limit. */
            if (tmp_u32 < 10000000u) {
                continue;
            }

            /* PLL fractional mode. */
            curr_freq_bias = my_abs((int32_t)(
                (int64_t)tmp_u32 * (int64_t)fb_div - (int64_t)rate));

            if (curr_freq_bias < freq_bias && fb_div >= 20u && fb_div <= 320u) {
                freq_bias = curr_freq_bias;
                ref_div_result = ref_div;
                fb_div_result = fb_div;
                ret = 0;
            }
        }
    }

    if (0 != ret) {
        return false;
    }

    /* Disable the VCO. */
    pll_vco_disable(pll_base);

    /* Don't enable bypass mode */
    RMWREG32(base + PLL_CTRL, PLL_CTRL_BAPASS, 1u, 0u);

    if (frac_div != 0u) {
        RMWREG32(base + PLL_CTRL, PLL_CTRL_INT_MODE, 1u, 0u);
        RMWREG32(base + PLL_CTRL, PLL_CTRL_DACEN, 1u, 1u);

        /* Calculate fractional div. */
        tmp_u32 = src_rate / ref_div_result;
        tmp_u64 = ((uint64_t)rate << 24u) / (uint64_t)tmp_u32;
        tmp_u64 = tmp_u64 - ((uint64_t)fb_div << 24u);
        frac_div = (uint32_t)tmp_u64 + 1;
        writel(frac_div, base + PLL_FRAC);
    } else {
        RMWREG32(base + PLL_CTRL, PLL_CTRL_INT_MODE, 1u, 1u);
    }

    writel(fb_div_result, base + PLL_FBDIV);
    RMWREG32(base + PLL_DIV, PLL_DIV_REFDIV_LSB, 24u, ref_div_result);

    /* Enable the VCO. */
    pll_vco_enable(pll_base);

    return true;
}

static bool pll_set_root_rate(addr_t base, uint32_t src_rate, uint32_t rate)
{
    uint32_t post_div1;
    uint32_t post_div2;

    if (!pll_calc_2_div(src_rate, rate, 7u, &post_div1, 7u, &post_div2)) {
        return false;
    }

    RMWREG32(base + PLL_DIV, PLL_DIV_POSTDIV1_LSB, 3u, post_div1);
    RMWREG32(base + PLL_DIV, PLL_DIV_POSTDIV2_LSB, 3u, post_div2);

    return true;
}

static bool pll_set_out_rate(addr_t base, uint32_t id, uint32_t src_rate,
                             uint32_t rate)
{
    uint32_t pll_div;
    addr_t div_base;
    uint32_t div_offset;

    pll_div = pll_get_best_div(src_rate, rate);
    if (pll_div != UINT32_MAX) {
        pll_div--;
    } else {
        return false;
    }

    if (pll_div >= PLL_OUT_DIV_MIN && pll_div <= PLL_OUT_DIV_MAX) {
        /* Set PLL divider. */
        div_base = base + PLL_OUT_DIV_1;
        div_base += (id - SC_PFPLL_ID_NUM_DIVA) / 2u * 4u;
        div_offset = (id - SC_PFPLL_ID_NUM_DIVA) % 2u * 16u;

        RMWREG32(div_base, div_offset, 4u, pll_div);

        return true;
    }

    return false;
}

static int pll_out_prepare(paddr_t pll_base, uint32_t id,
                           pll_rate_t src_rate, pll_rate_t rate)
{
    int ret = -1;
    uint32_t f_vco;

    addr_t base = pll_base;

    f_vco = src_rate;

    /* Enable the pll out. */
    /* Should enable CG_EN first, then set div freq and wait the corresponding busy bit. */
    pll_out_enable(pll_base, id);

    if (id == SC_PFPLL_ID_NUM_ROOT) {
        if (pll_set_root_rate(base, f_vco, rate)) {
            ret = 0;
        }
    } else {
        if (pll_set_out_rate(base, id, f_vco, rate)) {
            ret = 0;
        }
    }

    return ret;
}

int pll_enable(paddr_t pll_base, uint32_t id)
{
    if (id == SC_PFPLL_ID_NUM_VCO) {
        pll_vco_enable(pll_base);
    } else {
        pll_out_enable(pll_base, id);
    }

    return 0;
}

int pll_disable(paddr_t pll_base, uint32_t id)
{
    if (id == SC_PFPLL_ID_NUM_VCO) {
        pll_vco_disable(pll_base);
    } else {
        pll_out_disable(pll_base, id);
    }

    return 0;
}

int pll_prepare(paddr_t pll_base, uint32_t id,
                pll_rate_t src_rate, pll_rate_t rate)
{
    if (id == SC_PFPLL_ID_NUM_VCO) {
        return pll_vco_prepare(pll_base, src_rate, rate);
    } else {
        return pll_out_prepare(pll_base, id, src_rate, rate);
    }
}

pll_rate_t pll_get_rate(paddr_t pll_base, uint32_t id,
                        pll_rate_t src_rate)
{
    uint32_t reg;
    uint32_t pll_div;
    uint32_t post_div1, post_div2;
    addr_t div_base;
    addr_t div_offset;
    addr_t base = pll_base;

    if (id == SC_PFPLL_ID_NUM_VCO) {
        return pll_get_fvco(pll_base, src_rate);
    } else if (id == SC_PFPLL_ID_NUM_ROOT) {
        reg = readl(base + PLL_DIV);
        post_div1 = (reg >> PLL_DIV_POSTDIV1_LSB) & PLL_DIV_POSTDIV1_MASK;
        post_div2 = (reg >> PLL_DIV_POSTDIV2_LSB) & PLL_DIV_POSTDIV2_MASK;

        return src_rate / (post_div1 * post_div2);
    } else {
        /* Set PLL divider */
        div_base = base + PLL_OUT_DIV_1;
        div_base += (id - SC_PFPLL_ID_NUM_DIVA) / 2u * 4u;
        div_offset = (id - SC_PFPLL_ID_NUM_DIVA) % 2u * 16u;

        reg = readl(div_base);
        pll_div =
            (reg >> (PLL_OUT_DIV_NUM_LSB + div_offset)) & PLL_OUT_DIV_NUM_MASK;

        return src_rate / (pll_div + 1u);
    }
}

int pll_set_rate(paddr_t pll_base, uint32_t id,
                 pll_rate_t src_rate, pll_rate_t rate)
{
    int ret = -1;
    addr_t base = pll_base;

    if (id == SC_PFPLL_ID_NUM_VCO) {
        if (pll_set_vco_rate(base, src_rate, rate)) {
            ret = 0;
        }
    } else if (id == SC_PFPLL_ID_NUM_ROOT) {
        if (pll_set_root_rate(base, src_rate, rate)) {
            ret = 0;
        }
    } else {
        if (pll_set_out_rate(base, id, src_rate, rate)) {
            ret = 0;
        }
    }

    return ret;
}
