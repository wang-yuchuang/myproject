/**
 * @file sdrv_tmr.c
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: semidriver tmr driver.
 *
 * Revision History:
 * -----------------
 */

#include <types.h>

#include "reg.h"
#include "sdrv_tmr.h"

/**
 * @brief sdrv tmr read reg.
 *
 * @param[in] base reg base
 * @param[in] offset reg offset
 * @param[in] return reg val
 */
static inline uint32_t sdrv_tmr_lld_readl(uint32_t base, uint32_t offset)
{
    return readl(base + offset);
}

/**
 * @brief sdrv tmr write reg.
 *
 * @param[in] base reg base
 * @param[in] offset reg offset
 * @param[in] val reg val
 */
static inline void sdrv_tmr_lld_writel(uint32_t base, uint32_t offset, uint32_t val)
{
    writel(val, (base + offset));
}

/**
 * @brief sdrv tmr set ovf cnt.
 *
 * @param[in] tmr dev base
 * @param[in] tmr_id tmr id
 * @param[in] time_out ovf cnt value
 */
static void sdrv_tmr_lld_set_ovf_cnt(uint32_t base, uint8_t id, uint32_t ovf_cnt)
{
    sdrv_tmr_lld_writel(base, SDRV_TMR_CNT_OVF(id), ovf_cnt);
}

/**
 * @brief sdrv tmr start.
 *
 * @param[in] tmr dev base
 * @param[in] tmr_id tmr id
 */
static void sdrv_tmr_lld_start_cnt(uint32_t base, uint8_t id)
{
    uint32_t cnt_cfg;

    cnt_cfg = sdrv_tmr_lld_readl(base, SDRV_TMR_CNT_CFG);
    cnt_cfg |= SDRV_TMR_CNT_CFG_RLD_MASK(id);
    sdrv_tmr_lld_writel(base, SDRV_TMR_CNT_CFG, cnt_cfg);
}

/**
 * @brief sdrv tmr int enable.
 *
 * @param[in] tmr dev base
 * @param[in] int_id int id
 */
static void sdrv_tmr_lld_int_enable(uint32_t base, uint32_t int_id)
{
    uint32_t sta_en, sig_en;

    sta_en = sdrv_tmr_lld_readl(base, SDRV_TMR_STA_EN);
    sta_en |= SDRV_TMR_STA_MASK(int_id);
    sdrv_tmr_lld_writel(base, SDRV_TMR_STA_EN, sta_en);

    sig_en = sdrv_tmr_lld_readl(base, SDRV_TMR_SIG_EN);
    sig_en |= SDRV_TMR_STA_MASK(int_id);
    sdrv_tmr_lld_writel(base, SDRV_TMR_SIG_EN, sig_en);
}

/**
 * @brief sdrv tmr int disable.
 *
 * @param[in] tmr dev base
 * @param[in] int_id int id
 */
static void sdrv_tmr_lld_int_disable(uint32_t base, uint32_t int_id)
{
    uint32_t sta_en, sig_en;

    sta_en = sdrv_tmr_lld_readl(base, SDRV_TMR_STA_EN);
    sta_en &= ~(SDRV_TMR_STA_MASK(int_id));
    sdrv_tmr_lld_writel(base, SDRV_TMR_STA_EN, sta_en);

    sig_en = sdrv_tmr_lld_readl(base, SDRV_TMR_SIG_EN);
    sig_en &= ~(SDRV_TMR_STA_MASK(int_id));
    sdrv_tmr_lld_writel(base, SDRV_TMR_SIG_EN, sig_en);
}

/**
 * @brief sdrv tmr int status clear.
 *
 * @param[in] tmr dev base
 * @param[in] int_id int id
 */
static void sdrv_tmr_lld_int_clear(uint32_t base, uint32_t int_id)
{
    uint32_t sta_val;

    sta_val = sdrv_tmr_lld_readl(base, SDRV_TMR_STA);
    sta_val |= SDRV_TMR_STA_MASK(int_id);
    sdrv_tmr_lld_writel(base, SDRV_TMR_STA, sta_val);
}

/**
 * @brief sdrv tmr get ovf status and clear.
 *
 * @param[in] tmr dev base
 * @return ovf status bitmap
 */
uint32_t sdrv_tmr_lld_get_int_status(uint32_t base)
{
    uint32_t sta;

    sta = sdrv_tmr_lld_readl(base, SDRV_TMR_STA);
    sdrv_tmr_lld_writel(base, SDRV_TMR_STA, sta);
    return sta;
}

/**
 * @brief sdrv tmr init.
 *
 * @param[in] base sdrv tmr address
 * @param[in] clk clk src
 * @param[in] div clk div
 */
void sdrv_tmr_lld_init(uint32_t base, uint32_t clk, uint32_t div)
{
    uint32_t clk_cfg;

    /* set clk */
    clk_cfg = sdrv_tmr_lld_readl(base, SDRV_TMR_CLK_CFG);
    clk_cfg &= ~(SDRV_TMR_CLK_CFG_DIV_NUM_MASK | SDRV_TMR_CLK_CFG_SRC_CLK_SEL_MASK);
    clk_cfg |= ((clk << SDRV_TMR_CLK_CFG_SRC_CLK_SEL_SHIFT) |
                (div << SDRV_TMR_CLK_CFG_DIV_NUM_SHIFT));
    sdrv_tmr_lld_writel(base, SDRV_TMR_CLK_CFG, clk_cfg);
}

/**
 * @brief sdrv tmr set cascase mode.
 *
 * @param[in] base sdrv tmr address
 * @param[in] cascase_mode cascase mode
 */
void sdrv_tmr_lld_set_cascase_mode(uint32_t base, bool cascase_mode)
{
    uint32_t cnt_cfg;

    /* set cascase mode */
    if (cascase_mode) {
        cnt_cfg = sdrv_tmr_lld_readl(base, SDRV_TMR_CNT_CFG);
        cnt_cfg |= SDRV_TMR_CNT_CFG_CASCADE_MODE_MASK;
        sdrv_tmr_lld_writel(base, SDRV_TMR_CNT_CFG, cnt_cfg);

        /* start G0 and G1 */
        sdrv_tmr_lld_set_ovf_cnt(base, SDRV_TMR_G0, SDRV_TMR_MAX_OVF_VAL);
        sdrv_tmr_lld_set_ovf_cnt(base, SDRV_TMR_G1, SDRV_TMR_MAX_OVF_VAL);
        sdrv_tmr_lld_start_cnt(base, SDRV_TMR_G0);
        sdrv_tmr_lld_start_cnt(base, SDRV_TMR_G1);
    }
    else {
        cnt_cfg = sdrv_tmr_lld_readl(base, SDRV_TMR_CNT_CFG);
        cnt_cfg &= ~(SDRV_TMR_CNT_CFG_CASCADE_MODE_MASK);
        sdrv_tmr_lld_writel(base, SDRV_TMR_CNT_CFG, cnt_cfg);
    }
}

/**
 * @brief sdrv tmr set timer start.
 *
 * @param[in] base sdrv tmr address
 * @param[in] id tmr id
 * @param[in] count time_out time out value(ms)
 */
void sdrv_tmr_lld_timer_start(uint32_t base, uint8_t id, uint32_t count)
{
    sdrv_tmr_lld_set_ovf_cnt(base, id, count);
    sdrv_tmr_lld_start_cnt(base, id);
    sdrv_tmr_lld_int_enable(base, SDRV_TMR_STA_CNT_OVF_SHIFT(id));
}

/**
 * @brief sdrv tmr stop.
 *
 * @param[in] base sdrv tmr address
 * @param[in] id tmr id
 */
void sdrv_tmr_lld_timer_stop(uint32_t base, uint8_t id)
{
    sdrv_tmr_lld_int_disable(base, SDRV_TMR_STA_CNT_OVF_SHIFT(id));
    sdrv_tmr_lld_int_clear(base, SDRV_TMR_STA_CNT_OVF_SHIFT(id));
    sdrv_tmr_lld_set_ovf_cnt(base, id, SDRV_TMR_MAX_OVF_VAL);
}

/**
 * @brief sdrv tmr get current cnt value.
 *
 * @param[in] base sdrv tmr address
 * @param[in] id tmr id
 * @return current time
 */
uint32_t sdrv_tmr_lld_get_cur_cnt(uint32_t base, uint8_t id)
{
    return sdrv_tmr_lld_readl(base, SDRV_TMR_CNT_VAL(id));
}

/**
 * @brief sdrv tmr get ptp time.
 *
 * @param[in] base ptp tmr address
 * @param[in] ptp reg offset
 * @return ptp time
 */
 
uint32_t sdrv_tmr_lld_get_ptp_time(uint32_t base, uint8_t offset)
{
    return sdrv_tmr_lld_readl(base, offset);
}

/**
 * @brief sdrv tmr get ptp  sta.
 *
 * @param[in] base ptp tmr address
 * @param[in] ptp reg offset
 * @return 1:ptp enable  0:ptp disable
 */
uint8_t sdrv_tmr_lld_get_ptp_enable(uint32_t base, uint8_t offset)
{
    return (sdrv_tmr_lld_readl(base, offset) & 0x01);
}

/**
 * @brief sdrv tmr get glb cnt value.
 *
 * @param[in] dev tmr dev
 * @return glb cnt count(count)
 */
uint64_t sdrv_tmr_lld_get_glb_cnt(uint32_t base)
{
    uint32_t g0_cnt, g0_cnt_pre, g1_cnt;

    g0_cnt = sdrv_tmr_lld_readl(base, SDRV_TMR_CNT_VAL(SDRV_TMR_G0));
    do {
        g0_cnt_pre = g0_cnt;
        g1_cnt = sdrv_tmr_lld_readl(base, SDRV_TMR_CNT_VAL(SDRV_TMR_G1));
        g0_cnt = sdrv_tmr_lld_readl(base, SDRV_TMR_CNT_VAL(SDRV_TMR_G0));
    } while (g0_cnt_pre > g0_cnt);

    return (((uint64_t)g1_cnt) << 32) + g0_cnt;
}

/**
 * @brief sdrv tmr func cmp config.
 *
 * @param[in] tmr dev base
 * @param[in] chn_id func channel id
 * @param[in] cmp_cfg func cmp config
 */
void sdrv_tmr_lld_cmp_init(uint32_t base, uint8_t chn_id,
                           const sdrv_tmr_lld_cmp_cfg_t *cmp_cfg)
{
    uint32_t cfg_val;
    uint32_t cnt_cfg;

    cfg_val = sdrv_tmr_lld_readl(base, SDRV_TMR_CMP_CFG(chn_id));

    cfg_val &= ~(SDRV_TMR_CMP_CFG_CNT_SEL_MASK | SDRV_TMR_CMP_CFG_DUAL_MODE_MASK |
               SDRV_TMR_CMP_CFG_SINGLE_MODE_MASK |
               SDRV_TMR_CMP_CFG_CMP0_OUT_MODE_MASK | SDRV_TMR_CMP_CFG_CMP0_PULSE_WID_MASK |
               SDRV_TMR_CMP_CFG_CMP1_OUT_MODE_MASK | SDRV_TMR_CMP_CFG_CMP1_PULSE_WID_MASK);

    cfg_val |= ((cmp_cfg->cmp_cnt_sel << SDRV_TMR_CMP_CFG_CNT_SEL_SHIFT) |
              (cmp_cfg->dual_cmp_mode << SDRV_TMR_CMP_CFG_DUAL_MODE_SHIFT) |
              (cmp_cfg->oneshot_mode << SDRV_TMR_CMP_CFG_SINGLE_MODE_SHIFT) |
              (cmp_cfg->cmp0_out_mode << SDRV_TMR_CMP_CFG_CMP0_OUT_MODE_SHIFT) |
              (cmp_cfg->cmp0_pulse_width << SDRV_TMR_CMP_CFG_CMP0_PULSE_WID_SHIFT) |
              (cmp_cfg->cmp1_out_mode << SDRV_TMR_CMP_CFG_CMP1_OUT_MODE_SHIFT) |
              (cmp_cfg->cmp1_pulse_width << SDRV_TMR_CMP_CFG_CMP1_PULSE_WID_SHIFT));

    sdrv_tmr_lld_writel(base, SDRV_TMR_CMP_CFG(chn_id), cfg_val);

    cnt_cfg = sdrv_tmr_lld_readl(base, SDRV_TMR_CNT_CFG);
    if (cmp_cfg->cmp_rst_en) {
        cnt_cfg |= SDRV_TMR_CNT_CFG_RST_EN_MASK(chn_id);
    }
    else {
        cnt_cfg &= ~(SDRV_TMR_CNT_CFG_RST_EN_MASK(chn_id));
    }
    sdrv_tmr_lld_writel(base, SDRV_TMR_CNT_CFG, cnt_cfg);

    sdrv_tmr_lld_sse_cmp_init(base, chn_id, cmp_cfg->sse_enable);

    sdrv_tmr_lld_cmp_forceout(base, chn_id, cmp_cfg->force_out,
                              cmp_cfg->force_level);

    if (cmp_cfg->dma_cfg.dma_enable) {
        sdrv_tmr_lld_func_dma_init(base, chn_id, SDRV_TMR_DMA_CTRL_CHN_SEL_CMP,
                                   cmp_cfg->dma_cfg.fifo_wml);
    }
}

/**
 * @brief sdrv tmr func compare set val.
 *
 * @param[in] tmr dev base
 * @param[in] chn_id func channel id
 * @param[in] cmp_val0 func cmp val0
 * @param[in] cmp_val1 func cmp val1
 */
void sdrv_tmr_lld_set_cmp_val(uint32_t base, uint8_t chn_id,
                              uint32_t cmp_val0, uint32_t cmp_val1)
{
    sdrv_tmr_lld_writel(base, SDRV_TMR_CMP0_VAL(chn_id), cmp_val0);
    sdrv_tmr_lld_writel(base, SDRV_TMR_CMP1_VAL(chn_id), cmp_val1);
    sdrv_tmr_lld_writel(base, SDRV_TMR_CMP_UPT(chn_id), 1);
}

/**
 * @brief sdrv tmr func cmp start.
 *
 * @param[in] tmr dev base
 * @param[in] chn_id func channel id
 * @param[in] start_fg start or stop cmp(0-stop, 1-start)
 */
void sdrv_tmr_lld_cmp_ctrl(uint32_t base, uint8_t chn_id,
                           bool start_fg)
{
    uint32_t cfg_val;

    cfg_val = sdrv_tmr_lld_readl(base, SDRV_TMR_CMP_CFG(chn_id));
    if (start_fg) {
        cfg_val |= SDRV_TMR_CMP_CFG_EN_MASK;
    }
    else {
        cfg_val &= ~SDRV_TMR_CMP_CFG_EN_MASK;
    }
    sdrv_tmr_lld_writel(base, SDRV_TMR_CMP_CFG(chn_id), cfg_val);
}

/**
 * @brief sdrv tmr func cmp force out.
 *
 * @param[in] tmr dev base
 * @param[in] chn_id func channel id
 * @param[in] enable enable force out(0-disable, 1-enable)
 * @param[in] level out level(0-low, 1-high)
 */
void sdrv_tmr_lld_cmp_forceout(uint32_t base, uint8_t chn_id,
                               bool enable, bool level)
{
    uint32_t cfg_val;

    cfg_val = sdrv_tmr_lld_readl(base, SDRV_TMR_CMP_CFG(chn_id));

    cfg_val &= ~(SDRV_TMR_CMP_CFG_FRC_MASK);

    if (enable) {
        if (level) {
            cfg_val |= (1 << SDRV_TMR_CMP_CFG_FRC_HIGH_SHIFT);
        }
        else {
            cfg_val |= (1 << SDRV_TMR_CMP_CFG_FRC_LOW_SHIFT);
        }
    }

    sdrv_tmr_lld_writel(base, SDRV_TMR_CMP_CFG(chn_id), cfg_val);
}

/**
 * @brief sdrv tmr func cpt config.
 *
 * @param[in] tmr dev base
 * @param[in] chn_id func channel id
 * @param[in] cpt_cfg func cpt config
 */
void sdrv_tmr_lld_cpt_init(uint32_t base, uint8_t chn_id,
                           const sdrv_tmr_lld_cpt_cfg_t *cpt_cfg)
{
    uint32_t cfg_val;
    uint32_t cnt_cfg;

    cfg_val = sdrv_tmr_lld_readl(base, SDRV_TMR_CPT_CFG(chn_id));

    cfg_val &= ~(SDRV_TMR_CPT_CFG_CNT_SEL_MASK | SDRV_TMR_CPT_CFG_DUAL_MODE_MASK |
                 SDRV_TMR_CPT_CFG_SINGLE_MODE_MASK | SDRV_TMR_CPT_CFG_TRIG_MODE_MASK);

    cfg_val |= ((cpt_cfg->cpt_cnt_sel << SDRV_TMR_CPT_CFG_CNT_SEL_SHIFT) |
                (cpt_cfg->dual_cpt_mode << SDRV_TMR_CPT_CFG_DUAL_MODE_SHIFT) |
                (cpt_cfg->oneshot_mode << SDRV_TMR_CPT_CFG_SINGLE_MODE_SHIFT) |
                (cpt_cfg->trig_mode << SDRV_TMR_CPT_CFG_TRIG_MODE_SHIFT));

    sdrv_tmr_lld_writel(base, SDRV_TMR_CPT_CFG(chn_id), cfg_val);

    cnt_cfg = sdrv_tmr_lld_readl(base, SDRV_TMR_CNT_CFG);
    if (cpt_cfg->first_cpt_rst_en) {
        cnt_cfg |= SDRV_TMR_CNT_CFG_RST_EN_MASK(chn_id);
    }
    else {
        cnt_cfg &= ~(SDRV_TMR_CNT_CFG_RST_EN_MASK(chn_id));
    }
    sdrv_tmr_lld_writel(base, SDRV_TMR_CNT_CFG, cnt_cfg);

    sdrv_tmr_lld_sse_cpt_init(base, chn_id, cpt_cfg->sse_enable,
                              cpt_cfg->sse_cmp_chn);

    sdrv_tmr_lld_cpt_filter_init(base, chn_id, cpt_cfg->filter_dis,
                                 cpt_cfg->filter_width);

    if (cpt_cfg->dma_cfg.dma_enable) {
        sdrv_tmr_lld_func_dma_init(base, chn_id, SDRV_TMR_DMA_CTRL_CHN_SEL_CPT,
                                   cpt_cfg->dma_cfg.fifo_wml);
    }
}

/**
 * @brief sdrv tmr func cpt start.
 *
 * @param[in] tmr dev base
 * @param[in] chn_id func channel id
 * @param[in] start_fg start or stop cpt(0-stop, 1-start)
 */
void sdrv_tmr_lld_cpt_ctrl(uint32_t base, uint8_t chn_id,
                           bool start_fg)
{
    uint32_t cfg_val;

    cfg_val = sdrv_tmr_lld_readl(base, SDRV_TMR_CPT_CFG(chn_id));
    if (start_fg) {
        cfg_val |= SDRV_TMR_CPT_CFG_EN_MASK;
    }
    else {
        cfg_val &= ~SDRV_TMR_CPT_CFG_EN_MASK;
    }
    sdrv_tmr_lld_writel(base, SDRV_TMR_CPT_CFG(chn_id), cfg_val);
}

/**
 * @brief sdrv tmr cpt filter init.
 *
 * @param[in] tmr dev base
 * @param[in] chn_id func channel id
 * @param[in] flt_enable flt enalbe(1-enable, 0-disable)
 * @param[in] flt_width flt width
 */
void sdrv_tmr_lld_cpt_filter_init(uint32_t base, uint8_t chn_id,
                                  uint8_t flt_dis, uint8_t flt_width)
{
    uint32_t cpt_flt;

    cpt_flt = sdrv_tmr_lld_readl(base, SDRV_TMR_CPT_FLT);
    if (flt_dis) {
        cpt_flt |= SDRV_TMR_CPT_FLT_DIS_MASK(chn_id);
    }
    else {
        cpt_flt &= ~(SDRV_TMR_CPT_FLT_DIS_MASK(chn_id));
        cpt_flt |= (flt_width << SDRV_TMR_CPT_FLT_WID_SHIFT(chn_id));
    }

    sdrv_tmr_lld_writel(base, SDRV_TMR_CPT_FLT, cpt_flt);
}

/**
 * @brief sdrv tmr get fifo value.
 *
 * @param[in] tmr dev base
 * @param[in] chn_id func channel id
 * @return fifo val
 */
uint32_t sdrv_tmr_lld_get_fifo_val(uint32_t base, uint8_t chn_id)
{
    return sdrv_tmr_lld_readl(base, SDRV_TMR_FIFO_VAL(chn_id));
}

/**
 * @brief sdrv tmr set fifo value.
 *
 * @param[in] tmr dev base
 * @param[in] chn_id func channel id
 * @param[in] val set value
 */
void sdrv_tmr_lld_set_fifo_val(uint32_t base, uint8_t chn_id, uint32_t val)
{
    sdrv_tmr_lld_writel(base, SDRV_TMR_FIFO_VAL(chn_id), val);
}

/**
 * @brief sdrv tmr sse cmp init.
 *
 * @param[in] tmr dev base
 * @param[in] chn_id cmp channel id
 * @param[in] cmp_enable enable cmp sse(0-disable, 1-enable)
 */
void sdrv_tmr_lld_sse_cmp_init(uint32_t base, uint8_t chn_id,
                               bool cmp_enable)
{
    uint32_t sse_ctrl;

    sse_ctrl = sdrv_tmr_lld_readl(base, SDRV_TMR_SSE_CTRL);
    if (cmp_enable) {
        sse_ctrl |= SDRV_TMR_SSE_CTRL_CMP_EN_MASK(chn_id);
    }
    else {
        sse_ctrl &= ~(SDRV_TMR_SSE_CTRL_CMP_EN_MASK(chn_id));
    }

    sdrv_tmr_lld_writel(base, SDRV_TMR_SSE_CTRL, sse_ctrl);
}

/**
 * @brief sdrv tmr sse cpt init.
 *
 * @param[in] tmr dev base
 * @param[in] chn_id cpt channel id
 * @param[in] cpt_enable enable cpt sse(0-disable, 1-enable)
 * @param[in] cmp_chn cmp channel id
 */
void sdrv_tmr_lld_sse_cpt_init(uint32_t base, uint8_t chn_id,
                               bool cpt_enable, uint8_t cmp_chn)
{
    uint32_t sse_ctrl;

    sse_ctrl = sdrv_tmr_lld_readl(base, SDRV_TMR_SSE_CTRL);

    if (cpt_enable) {
        sse_ctrl &= ~(SDRV_TMR_SSE_CTRL_CMP_SEL_MASK(chn_id));
        sse_ctrl |= (cmp_chn << SDRV_TMR_SSE_CTRL_CMP_SEL_SHIFT(chn_id));
        sse_ctrl |= SDRV_TMR_SSE_CTRL_CPT_EN_MASK(chn_id);
    }
    else {
        sse_ctrl &= ~(SDRV_TMR_SSE_CTRL_CPT_EN_MASK(chn_id));
    }

    sdrv_tmr_lld_writel(base, SDRV_TMR_SSE_CTRL, sse_ctrl);
}

/**
 * @brief sdrv tmr ovf dma init.
 *
 * @param[in] tmr dev base
 * @param[in] ovf_sel ovf sel
 */
void sdrv_tmr_lld_glb_dma_init(uint32_t base, uint8_t ovf_sel)
{
    uint32_t dma_ctrl;

    dma_ctrl = sdrv_tmr_lld_readl(base, SDRV_TMR_DMA_CTRL);

    dma_ctrl |= (ovf_sel << SDRV_TMR_DMA_CTRL_OVF_SEL_SHIFT) | \
                (1 << SDRV_TMR_DMA_CTRL_OVF_EN_SHIFT);

    sdrv_tmr_lld_writel(base, SDRV_TMR_DMA_CTRL, dma_ctrl);
}

/**
 * @brief sdrv tmr func dma init.
 *
 * @param[in] tmr dev base
 * @param[in] chn_id func channel id
 * @param[in] sel func sel
 * @param[in] fifo_wml fifo wml
 */
void sdrv_tmr_lld_func_dma_init(uint32_t base, uint8_t chn_id,
                                uint8_t sel, uint8_t fifo_wml)
{
    uint32_t dma_ctrl;

    dma_ctrl = sdrv_tmr_lld_readl(base, SDRV_TMR_DMA_CTRL);

    dma_ctrl &= ~(SDRV_TMR_DMA_CTRL_CHN_SEL_MASK(chn_id) | \
                  SDRV_TMR_DMA_CTRL_WML_MASK(chn_id));
    dma_ctrl |= (sel << SDRV_TMR_DMA_CTRL_CHN_SEL_SHIFT(chn_id)) | \
                (fifo_wml << SDRV_TMR_DMA_CTRL_WML_SHIFT(chn_id)) | \
                (1 << SDRV_TMR_DMA_CTRL_CHN_EN_SHIFT(chn_id));

    sdrv_tmr_lld_writel(base, SDRV_TMR_DMA_CTRL, dma_ctrl);
}

/**
 * @brief sdrv tmr get fifo num.
 *
 * @param[in] tmr dev base
 * @param[in] chn_id func channel id
 * @return fifo num
 */
uint8_t sdrv_tmr_lld_get_fifo_num(uint32_t base, uint8_t chn_id)
{
    uint32_t fifo_sta;

    fifo_sta = sdrv_tmr_lld_readl(base, SDRV_TMR_FIFO_STA);
    fifo_sta &= SDRV_TMR_FIFO_STA_ENTRIES_MASK(chn_id);

    return (uint8_t)(fifo_sta >> SDRV_TMR_FIFO_STA_ENTRIES_SHIFT(chn_id));
}

/**
 * @brief sdrv tmr func soft reset.
 *
 * @param[in] tmr dev base
 * @param[in] chn_id channel id
 */
void sdrv_tmr_lld_func_reset(uint32_t base, uint8_t chn_id)
{
    sdrv_tmr_lld_writel(base, SDRV_TMR_SOFT_RESET,
                        SDRV_TMR_SOFT_RESET_CHN_MASK(chn_id));
}
