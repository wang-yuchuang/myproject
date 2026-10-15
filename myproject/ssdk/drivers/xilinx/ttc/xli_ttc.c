/**
 * @file xli_ttc.c
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: Xilinx ttc driver interface.
 *
 * Revision History:
 * -----------------
 */

#include <types.h>

#include "reg.h"
#include "xli_ttc.h"

/**
 * @brief xli ttc read reg.
 *
 * @param[in] base reg base
 * @param[in] offset reg offset
 * @param[in] return reg val
 */
static inline uint32_t xli_ttc_lld_readl(uint32_t base, uint32_t offset)
{
    return readl(base + offset);
}

/**
 * @brief xli ttc write reg.
 *
 * @param[in] base reg base
 * @param[in] offset reg offset
 * @param[in] val reg val
 */
static inline void xli_ttc_lld_writel(uint32_t base, uint32_t offset, uint32_t val)
{
    writel(val, (base + offset));
}

/**
 * @brief xli ttc enable int.
 *
 * @param[in] base reg base
 * @param[in] mask int mask
 */
static void xli_ttc_lld_enable_int(uint32_t base, uint32_t mask)
{
    uint32_t ier;
    ier = xli_ttc_lld_readl(base, XLI_TTC_IER_OFFSET);
    xli_ttc_lld_writel(base, XLI_TTC_IER_OFFSET, ier | mask);
}

/**
 * @brief xli ttc disable int.
 *
 * @param[in] base reg base
 * @param[in] mask int mask
 */
static void xli_ttc_lld_disable_int(uint32_t base, uint32_t mask)
{
    uint32_t ier;
    ier = xli_ttc_lld_readl(base, XLI_TTC_IER_OFFSET);
    xli_ttc_lld_writel(base, XLI_TTC_IER_OFFSET, ier & ~mask);
}

/**
 * @brief xli ttc clear int status.
 *
 * @param[in] base xli ttc address
 */
void xli_ttc_lld_clear_int(uint32_t base)
{
    xli_ttc_lld_readl(base, XLI_TTC_ISR_OFFSET);
}

/**
 * @brief xli ttc init.
 *
 * @param[in] base xli ttc address
 * @param[in] clk clk src
 * @param[in] div clk div
 */
void xli_ttc_lld_init(uint32_t base)
{
    uint32_t cnt_ctrl, clk_ctrl;

    /*
     * Reset the count control register to it's default value.
     */
    xli_ttc_lld_writel(base, XLI_TTC_CNT_CNTRL_OFFSET,
                       XLI_TTC_CNT_CNTRL_RESET_VALUE);

    /*
     * Reset the rest of the registers to the default values.
     */
    xli_ttc_lld_writel(base, XLI_TTC_CLK_CNTRL_OFFSET, 0x00U);
    xli_ttc_lld_writel(base, XLI_TTC_INTERVAL_VAL_OFFSET, 0x00U);
    xli_ttc_lld_writel(base, XLI_TTC_MATCH_0_OFFSET, 0x00U);
    xli_ttc_lld_writel(base, XLI_TTC_MATCH_1_OFFSET, 0x00U);
    xli_ttc_lld_writel(base, XLI_TTC_MATCH_2_OFFSET, 0x00U);
    xli_ttc_lld_writel(base, XLI_TTC_IER_OFFSET, 0x00U);
    xli_ttc_lld_writel(base, XLI_TTC_ISR_OFFSET, XLI_TTC_IXR_ALL_MASK);

    cnt_ctrl = xli_ttc_lld_readl(base, XLI_TTC_CNT_CNTRL_OFFSET);
    cnt_ctrl |= XLI_TTC_CNT_CNTRL_RST_MASK;
    xli_ttc_lld_writel(base, XLI_TTC_CNT_CNTRL_OFFSET, cnt_ctrl);

    /*
     * Set option.
     */
    clk_ctrl = xli_ttc_lld_readl(base, XLI_TTC_CLK_CNTRL_OFFSET);
    clk_ctrl &= ~(XLI_TTC_CLK_CNTRL_SRC_MASK |
                  XLI_TTC_CLK_CNTRL_EXT_EDGE_MASK);
    xli_ttc_lld_writel(base, XLI_TTC_CLK_CNTRL_OFFSET, clk_ctrl);

    cnt_ctrl = xli_ttc_lld_readl(base, XLI_TTC_CNT_CNTRL_OFFSET);
    cnt_ctrl |= (XLI_TTC_CNT_CNTRL_INT_MASK |
                 XLI_TTC_CNT_CNTRL_EN_WAVE_MASK);
    cnt_ctrl &= ~(XLI_TTC_CNT_CNTRL_DECR_MASK |
                  XLI_TTC_CNT_CNTRL_MATCH_MASK |
                  XLI_TTC_CNT_CNTRL_POL_WAVE_MASK);
    xli_ttc_lld_writel(base, XLI_TTC_CNT_CNTRL_OFFSET, cnt_ctrl);
}

/**
 * @brief xli ttc set timer start.
 *
 * @param[in] base xli ttc address
 * @param[in] count count value
 */
void xli_ttc_lld_timer_start(uint32_t base, uint32_t count)
{
    uint32_t clk_ctrl, cnt_ctrl;

    xli_ttc_lld_writel(base, XLI_TTC_INTERVAL_VAL_OFFSET, count);
    clk_ctrl = xli_ttc_lld_readl(base, XLI_TTC_CLK_CNTRL_OFFSET);
    clk_ctrl &= ~(XLI_TTC_CLK_CNTRL_PS_VAL_MASK | XLI_TTC_CLK_CNTRL_PS_EN_MASK);
    xli_ttc_lld_writel(base, XLI_TTC_CLK_CNTRL_OFFSET, clk_ctrl);

    xli_ttc_lld_enable_int(base, XLI_TTC_IXR_INTERVAL_MASK);

    cnt_ctrl = xli_ttc_lld_readl(base, XLI_TTC_CNT_CNTRL_OFFSET);
    xli_ttc_lld_writel(base, XLI_TTC_CNT_CNTRL_OFFSET,
                       (cnt_ctrl & ~XLI_TTC_CNT_CNTRL_DIS_MASK));
}

/**
 * @brief xli ttc stop.
 *
 * @param[in] base xli ttc address
 */
void xli_ttc_lld_timer_stop(uint32_t base)
{
    uint32_t cnt_ctrl;

    xli_ttc_lld_disable_int(base, XLI_TTC_IXR_INTERVAL_MASK);

    cnt_ctrl = xli_ttc_lld_readl(base, XLI_TTC_CNT_CNTRL_OFFSET);
    xli_ttc_lld_writel(base, XLI_TTC_CNT_CNTRL_OFFSET,
                       (cnt_ctrl | XLI_TTC_CNT_CNTRL_DIS_MASK));
}

/**
 * @brief xli ttc get current cnt value.
 *
 * @param[in] base xli ttc address
 * @return current time
 */
uint32_t xli_ttc_lld_get_cur_cnt(uint32_t base)
{
    return xli_ttc_lld_readl(base, XLI_TTC_COUNT_VALUE_OFFSET);
}
