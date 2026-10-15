/*
 * @File:etimer_pwm_drv.c
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * @Descripttion: taishan etimer hld driver
 *
 * @Revision History:
 * ----------------
 */

#define LOG_TAG "etimer_pwm_drv.c "
#include <driver/clk.h>
#include <errno.h>
#include <regs_base.h>
#include <string.h>
#include <types.h>
#include "driver/taishan/etimer/etimer_pwm_drv.h"
#include "bits.h"
#include "debug.h"
#include "driver/irq.h"
#include "driver/pwm.h"
#include "reg.h"
static void etimer_pwm_start(pwm_dev_t *dev);
static void etimer_pwm_stop(pwm_dev_t *dev);
static void etimer_pwm_config(pwm_dev_t *dev, const pwm_state_t *state);
static int etimer_pwm_capture(pwm_dev_t *dev, pwm_state_t *state);

static pwm_ops_t etimer_pwm_ops = {
    .start = etimer_pwm_start,
    .stop = etimer_pwm_stop,
    .config = etimer_pwm_config,
    .capture = etimer_pwm_capture,
};

/**
 * @brief Start eTMR channel output.
 *
 * @param[in] dev pwm common instance
 *
 */
static void etimer_pwm_start(pwm_dev_t *dev)
{
    etimer_pwm_dev_t *etimer_pwm = dev->priv;
    etimer_pwm_controller_t *etimer_ctrl = etimer_pwm->controller;
    sdrv_etimer_chn_id_e cmp_chn = etimer_pwm->cmp_chn;
    sdrv_etimer_chn_id_e cpt_chn = etimer_pwm->cpt_chn;
    sdrv_etimer_cnt_id_e cpt_cnt_sel = cpt_chn + 2;
    sdrv_etimer_cnt_id_e cmp_cnt_sel;
    cmp_cnt_sel = (cmp_chn == cpt_chn) ? SDRV_ETIMR_CNT_G0 : (cmp_chn + 2);
    sdrv_etimer_lld_cmp_en(etimer_ctrl->base, cmp_chn, true);
    sdrv_etimer_lld_cpt_en(etimer_ctrl->base, cpt_chn, true);
    sdrv_etimer_lld_cnt_en(etimer_ctrl->base, cmp_cnt_sel, true);
    sdrv_etimer_lld_cnt_en(etimer_ctrl->base, cpt_cnt_sel, true);
}

/**
 * @brief Stop eTMR channel output.
 *
 * @param[in] dev pwm common instance
 *
 */
static void etimer_pwm_stop(pwm_dev_t *dev)
{
    etimer_pwm_dev_t *etimer_pwm = dev->priv;
    etimer_pwm_controller_t *etimer_ctrl = etimer_pwm->controller;
    sdrv_etimer_chn_id_e cmp_chn = etimer_pwm->cmp_chn;
    sdrv_etimer_chn_id_e cpt_chn = etimer_pwm->cpt_chn;
    sdrv_etimer_cnt_id_e cpt_cnt_sel = cpt_chn + 2;
    sdrv_etimer_cnt_id_e cmp_cnt_sel;
    cmp_cnt_sel = (cmp_chn == cpt_chn) ? SDRV_ETIMR_CNT_G0 : (cmp_chn + 2);
    sdrv_etimer_lld_cmp_en(etimer_ctrl->base, cmp_chn, false);
    sdrv_etimer_lld_cpt_en(etimer_ctrl->base, cpt_chn, false);
    sdrv_etimer_lld_cnt_en(etimer_ctrl->base, cmp_cnt_sel, false);
    sdrv_etimer_lld_cnt_en(etimer_ctrl->base, cpt_cnt_sel, false);
}

/**
 * @brief eTMR channel config.
 *
 * @param[in] dev pwm common instance
 * @param[in] state pwm duty and period
 *
 */
static void etimer_pwm_config(pwm_dev_t *dev, const pwm_state_t *state)
{
    etimer_pwm_dev_t *etimer_pwm = dev->priv;
    etimer_pwm_controller_t *etimer_ctrl = etimer_pwm->controller;
    sdrv_etimer_chn_id_e cmp_chn = etimer_pwm->cmp_chn;
    sdrv_etimer_chn_id_e cpt_chn = etimer_pwm->cpt_chn;
    cmp_config_t cmp_config;
    cpt_config_t cpt_config;
    etimer_cmp_event_mode_t cmp_event_mode;
    local_cnt_config_t local_cnt_config;
    
    memset(&cmp_config, 0, sizeof(cmp_config));
    memset(&cpt_config, 0, sizeof(cpt_config));
    memset(&cmp_event_mode, 0, sizeof(cmp_event_mode));
    memset(&local_cnt_config, 0, sizeof(local_cnt_config));

    /* cnt select */
    sdrv_etimer_cnt_id_e cpt_cnt_sel = cpt_chn + 2;
    cpt_config.cnt_sel = SDRV_ETIMR_CPT_LCNT;
    sdrv_etimer_cnt_id_e cmp_cnt_sel;
    cmp_cnt_sel = (cmp_chn == cpt_chn) ? SDRV_ETIMR_CNT_G0 : (cmp_chn + 2);
    cmp_config.cnt_sel = (cmp_chn == cpt_chn) ? SDRV_ETIMR_CMP_CNT_G0 : SDRV_ETIMR_CMP_LCNT;

    etimer_cmp_cfg_t *cmp_cfg = etimer_pwm->cmp_cfg;
    uint32_t period_cnt = ETMR_NS_TO_VAL_1(state->period, etimer_ctrl->clk_div);
    uint32_t duty_cnt = ETMR_NS_TO_VAL(state->duty, etimer_ctrl->clk_div);
    uint32_t dual_period_cnt_left = (period_cnt + 1) / 2 - duty_cnt / 2;
    uint32_t dual_period_cnt_right = (period_cnt + 1) / 2 + duty_cnt / 2;

    cmp_config.out_mode = cmp_cfg->chnl_out_mode;
    cmp_config.con_mode = cmp_cfg->con_mode;
    cmp_config.cmp_mode = cmp_cfg->cmp_mode;

    if (cmp_chn < TOTAL_CHN_NUM_PER_ETMR) {
        /* cmp cnt cfg */
        sdrv_etimer_lld_cnt_ovf(etimer_ctrl->base, cmp_cnt_sel, period_cnt);

        /* pwm cfg -- cmp config */
        cmp_event_mode.cmp_0_ovf_out = SDRV_ETIMR_KEEP;
        cmp_event_mode.cmp_1_ovf_out = SDRV_ETIMR_KEEP;
        if (cmp_cfg->cmp_mode == SDRV_ETIMR_CMP_SINGLE)
        {
            sdrv_etimer_lld_cmp_val(etimer_ctrl->base, cmp_chn, SDRV_ETIMR_CMP_00, duty_cnt);
            cmp_event_mode.cmp_00_out = SDRV_ETIMR_LEVEL_LOW;
            cmp_event_mode.cmp_0_ovf_out = SDRV_ETIMR_LEVEL_HIGH;
            if (cmp_cfg->chnl_out_mode == SDRV_ETIMR_CMP_OUT_CMP0_CMP1)
            {
                sdrv_etimer_lld_cmp_val(etimer_ctrl->base, cmp_chn, SDRV_ETIMR_CMP_10, duty_cnt);
                cmp_event_mode.cmp_10_out = SDRV_ETIMR_LEVEL_LOW;
                cmp_event_mode.cmp_1_ovf_out = SDRV_ETIMR_LEVEL_HIGH;
            }
        }
        else if (cmp_cfg->cmp_mode == SDRV_ETIMR_CMP_DUAL)
        {
            sdrv_etimer_lld_cmp_val(etimer_ctrl->base, cmp_chn, SDRV_ETIMR_CMP_00,
                                    dual_period_cnt_left);
            sdrv_etimer_lld_cmp_val(etimer_ctrl->base, cmp_chn, SDRV_ETIMR_CMP_01,
                                    dual_period_cnt_right);
            cmp_event_mode.cmp_00_out = SDRV_ETIMR_LEVEL_HIGH;
            cmp_event_mode.cmp_01_out = SDRV_ETIMR_LEVEL_LOW;
            if (cmp_cfg->chnl_out_mode == SDRV_ETIMR_CMP_OUT_CMP0_CMP1)
            {
                sdrv_etimer_lld_cmp_val(etimer_ctrl->base, cmp_chn, SDRV_ETIMR_CMP_10,
                                        dual_period_cnt_left);
                sdrv_etimer_lld_cmp_val(etimer_ctrl->base, cmp_chn, SDRV_ETIMR_CMP_11,
                                        dual_period_cnt_right);
                cmp_event_mode.cmp_10_out = SDRV_ETIMR_LEVEL_HIGH;
                cmp_event_mode.cmp_11_out = SDRV_ETIMR_LEVEL_LOW;
            }
        }
        sdrv_etimer_lld_cmp_val_upd(etimer_ctrl->base, cmp_chn);
        sdrv_etimer_lld_cmp_out_mode(etimer_ctrl->base, cmp_chn, &cmp_event_mode);
        sdrv_etimer_lld_cmp_cfg(etimer_ctrl->base, cmp_chn, &cmp_config);
        sdrv_etimer_lld_cmp_config_set(etimer_ctrl->base, cmp_chn);
    }

    if (cpt_chn < TOTAL_CHN_NUM_PER_ETMR) {
        /* cpt cnt cfg */
        local_cnt_config.cpt1_clr_en = true;
        local_cnt_config.start_by_first_cpt = true;
        sdrv_etimer_lld_local_cnt_cfg(etimer_ctrl->base, cpt_cnt_sel, &local_cnt_config);

        /* cpt_config */
        cpt_config.con_mode = SDRV_ETIMR_CPT_CONSECTIVE;
        cpt_config.dual_mode = SDRV_ETIMR_CPT_CPT0_CPT1;
        cpt_config.cpt1_trig_mode = SDRV_ETIMR_CPT_RISE_EDGE;
        cpt_config.cpt0_trig_mode = SDRV_ETIMR_CPT_FALL_EDGE;
        sdrv_etimer_lld_cpt_cfg(etimer_ctrl->base, cpt_chn, &cpt_config);
        sdrv_etimer_lld_cpt_config_set(etimer_ctrl->base, cpt_chn);
    }
}

/**
 * @brief eTMR channel config.
 *
 * @param[in] dev pwm common instance
 * @param[in] state pwm duty and period
 *
 */
static int etimer_pwm_capture(pwm_dev_t *dev, pwm_state_t *state)
{
    etimer_pwm_dev_t *etimer_pwm = dev->priv;
    etimer_pwm_controller_t *etimer_ctrl = etimer_pwm->controller;
    sdrv_etimer_chn_id_e cpt_chn = etimer_pwm->cpt_chn;
    if (cpt_chn < TOTAL_CHN_NUM_PER_ETMR) {
        uint32_t duty_cnt = sdrv_etimer_lld_get_cpt0_cnt0(etimer_ctrl->base, cpt_chn);
        uint32_t period_cnt = sdrv_etimer_lld_get_cpt1_cnt0(etimer_ctrl->base, cpt_chn);
        uint32_t duty_val = ETMR_VAL_TO_NS_1(duty_cnt, etimer_ctrl->clk_div);
        uint32_t period_val = ETMR_VAL_TO_NS_1(period_cnt, etimer_ctrl->clk_div);
        state->duty = duty_val;
        state->period = period_val;
    }
    return 0;
}

/**
 * @brief etimer interrupt handler
 *
 * @param[in] irq irq_num
 * @param[in] arg input argument
 *
 */
static int etimer_pwm_irq_handle(uint32_t irq, void *arg)
{
    etimer_pwm_controller_t *etimer_ctrl = arg;
    uint32_t int_sta = sdrv_etimer_lld_get_int_status(etimer_ctrl->base);
    sdrv_etimer_lld_clear_int_status(etimer_ctrl->base, int_sta);

    for (uint8_t id = SDRV_ETIMR_CHN_A; id < TOTAL_CHN_NUM_PER_ETMR; id++)
    {
        if (int_sta & ETMR_BM_INT_STA_CNT_OVF(id + 2))
        {
            etimer_pwm_dev_t *etimer_pwm = &etimer_ctrl->pwm[id];
            pwm_expiry(&etimer_pwm->dev);
        }
    }

    return 0;
}

/**
 * @brief configure multi compare mode
 *
 * @param[in] dev pwm common instance
 * @param[in] state pwm duty and period
 * @param[in] multi cmp00/01/10/11 val
 *
 */
void etimer_pwm_multi_cmp_mode(pwm_dev_t *dev, const pwm_state_t *state,
                               etimer_multi_cmp_val_t multi_cmp)
{
    etimer_pwm_dev_t *etimer_pwm = dev->priv;
    etimer_pwm_controller_t *etimer_ctrl = etimer_pwm->controller;
    sdrv_etimer_chn_id_e cmp_chn = etimer_pwm->cmp_chn;
    if (cmp_chn < TOTAL_CHN_NUM_PER_ETMR) {
        uint32_t cmp00_cnt =
            ETMR_NS_TO_VAL(multi_cmp.cmp00_val, etimer_ctrl->clk_div);
        uint32_t cmp01_cnt =
            ETMR_NS_TO_VAL(multi_cmp.cmp01_val, etimer_ctrl->clk_div);
        uint32_t cmp10_cnt =
            ETMR_NS_TO_VAL(multi_cmp.cmp10_val, etimer_ctrl->clk_div);
        uint32_t cmp11_cnt =
            ETMR_NS_TO_VAL(multi_cmp.cmp11_val, etimer_ctrl->clk_div);

        sdrv_etimer_lld_cmp_val(etimer_ctrl->base, cmp_chn, SDRV_ETIMR_CMP_00, cmp00_cnt);
        sdrv_etimer_lld_cmp_val(etimer_ctrl->base, cmp_chn, SDRV_ETIMR_CMP_01, cmp01_cnt);
        sdrv_etimer_lld_cmp_val(etimer_ctrl->base, cmp_chn, SDRV_ETIMR_CMP_10, cmp10_cnt);
        sdrv_etimer_lld_cmp_val(etimer_ctrl->base, cmp_chn, SDRV_ETIMR_CMP_11, cmp11_cnt);
    }
}

/**
 * @brief software trigger
 *
 * @param[in] dev pwm common instance
 * @param[in] trig_sel sellect trigger index
 * @param[in] trig_type sellect trigger type
 *
 */
void etimer_pwm_sw_trig(pwm_dev_t *dev, sdrv_etimer_sw_trig_sel_e trig_sel,
                        sdrv_etimer_sw_trig_typr_e trig_type)
{
    etimer_pwm_dev_t *etimer_pwm = dev->priv;
    etimer_pwm_controller_t *etimer_ctrl = etimer_pwm->controller;
    sdrv_etimer_lld_sw_trig_ctrl(etimer_ctrl->base, trig_sel, trig_type);
}

/**
 * @brief configure etimer death time
 *
 * @param[in] dev pwm common instance
 * @param[in] dti_val death time value
 *
 */
void etimer_pwm_dti(pwm_dev_t *dev, const uint32_t dti_val)
{
    etimer_pwm_dev_t *etimer_pwm = dev->priv;
    etimer_pwm_controller_t *etimer_ctrl = etimer_pwm->controller;
    sdrv_etimer_chn_id_e cmp_chn = etimer_pwm->cmp_chn;
    if (cmp_chn < TOTAL_CHN_NUM_PER_ETMR) {
        uint32_t dti_cnt = ETMR_NS_TO_VAL_1(dti_val, etimer_ctrl->clk_div);
        sdrv_etimer_lld_dti_ctrl_cfg(etimer_ctrl->base, cmp_chn, SDRV_ETIMR_CMP_1_INVERT_0);
        sdrv_etimer_lld_dti_wid(etimer_ctrl->base, cmp_chn, dti_cnt / 2, dti_cnt / 2);
        sdrv_etimer_lld_dti_ctrl_en(etimer_ctrl->base, cmp_chn, true);
    }
}

/**
 * @brief configure etimer death time
 *
 * @param[in] dev pwm common instance
 * @param[in] fault fault_cfg
 *
 */
void etimer_pwm_fault_ctrl(pwm_dev_t *dev, etimer_pwm_fault_ctrl_t *fault)
{
    etimer_pwm_dev_t *etimer_pwm = dev->priv;
    etimer_pwm_controller_t *etimer_ctrl = etimer_pwm->controller;
    sdrv_etimer_chn_id_e cmp_chn = etimer_pwm->cmp_chn;
    if (cmp_chn < TOTAL_CHN_NUM_PER_ETMR) {
        cmp_fault_ctrl_t fault_ctrl = fault->fault_ctrl;
        sdrv_etimer_lld_cmp_fault_ctrl(etimer_ctrl->base, cmp_chn, fault->fault_idx,
                                    &fault_ctrl);
        sdrv_etimer_lld_cmp_fault_en(etimer_ctrl->base, cmp_chn, fault->fault_idx, true);
    }
}

/**
 * @brief configure etimer filter config
 *
 * @param[in] dev pwm common instance
 * @param[in] filter_cfg filter_cfg
 *
 */
void etimer_pwm_filter(pwm_dev_t *dev, flt_config_t *filter_cfg)
{
    etimer_pwm_dev_t *etimer_pwm = dev->priv;
    etimer_pwm_controller_t *etimer_ctrl = etimer_pwm->controller;
    sdrv_etimer_chn_id_e cpt_chn = etimer_pwm->cpt_chn;
    if (cpt_chn < TOTAL_CHN_NUM_PER_ETMR) {
        sdrv_etimer_lld_cpt_flt_cfg(etimer_ctrl->base, cpt_chn, filter_cfg);
        sdrv_etimer_lld_cpt_flt_en(etimer_ctrl->base, cpt_chn, true);
    }
}

/**
 * @brief configure sse etimer output
 *
 * @param[in] dev pwm common instance
 * @param[in] sse_cfg sse_mode and edge sellect
 * @param[in] sse_reg_val sse register value
 *
 */
void etimer_pwm_cmp_sse(pwm_dev_t *dev, const sdrv_etimer_sse_ctrl_t sse_cfg,
                        const uint32_t sse_reg_val)
{

    etimer_pwm_dev_t *etimer_pwm = dev->priv;
    etimer_pwm_controller_t *etimer_ctrl = etimer_pwm->controller;
    sdrv_etimer_chn_id_e cmp_chn = etimer_pwm->cmp_chn;
    if (cmp_chn < TOTAL_CHN_NUM_PER_ETMR) {
        sdrv_etimer_lld_cmp_sse_ctrl(etimer_ctrl->base, cmp_chn, &sse_cfg);
        sdrv_etimer_lld_cmp_sse_val(etimer_ctrl->base, cmp_chn, sse_reg_val);
        sdrv_etimer_lld_cmp_sse_en(etimer_ctrl->base, cmp_chn, true);
    }
}

/**
 * @brief configure sse etimer input
 *
 * @param[in] dev pwm common instance
 * @param[in] sse_cfg sse_mode and edge sellect
 * @param[in] sse_reg_val sse register value
 *
 */
void etimer_pwm_cpt_sse(pwm_dev_t *dev, const sdrv_etimer_sse_ctrl_t sse_cfg,
                        const uint32_t sse_reg_val)
{

    etimer_pwm_dev_t *etimer_pwm = dev->priv;
    etimer_pwm_controller_t *etimer_ctrl = etimer_pwm->controller;
    sdrv_etimer_chn_id_e cpt_chn = etimer_pwm->cpt_chn;
    if (cpt_chn < TOTAL_CHN_NUM_PER_ETMR) {
        sdrv_etimer_lld_cpt_sse_ctrl(etimer_ctrl->base, cpt_chn, &sse_cfg);
        sdrv_etimer_lld_cpt_sse_val(etimer_ctrl->base, cpt_chn, sse_reg_val);
        sdrv_etimer_lld_cpt_sse_en(etimer_ctrl->base, cpt_chn, true);
    }
}

/**
 * @brief configure quad etimer input
 * 
 *
 * @param[in] dev pwm common instance
 * @param[in] quad_ctrl quad config
 *
 */
void etimer_pwm_cpt_quad(pwm_dev_t *dev, quad_ctrl_t *quad_ctrl)
{
    etimer_pwm_dev_t *etimer_pwm = dev->priv;
    etimer_pwm_controller_t *etimer_ctrl = etimer_pwm->controller;
    sdrv_etimer_chn_id_e cpt_chn = etimer_pwm->cpt_chn;
    if (cpt_chn == SDRV_ETIMR_CHN_A)
    {
        /* cpt_a is working for pwm state capture now */
        return;
    }
    sdrv_etimer_lld_cpt_quad_ctrl(etimer_ctrl->base, quad_ctrl);
    sdrv_etimer_lld_lcnt_sig_en(etimer_ctrl->base, SDRV_ETIMR_LCNT_A, true);
    sdrv_etimer_lld_cpt_quad_en(etimer_ctrl->base, true);
}

/**
 * @brief configure quad etimer input
 * 
 *
 * @param[in] dev pwm common instance
 * @param[in] quad_ctrl quad config
 *
 */
void etimer_pwm_quad_cnt_trig_(pwm_dev_t *dev, quad_ctrl_t *quad_ctrl)
{
    etimer_pwm_dev_t *etimer_pwm = dev->priv;
    etimer_pwm_controller_t *etimer_ctrl = etimer_pwm->controller;
    sdrv_etimer_chn_id_e cpt_chn = etimer_pwm->cpt_chn;
    if (cpt_chn == SDRV_ETIMR_CHN_A)
    {
        /* cpt_a is working for pwm state capture now */
        return;
    }
    sdrv_etimer_lld_cpt_quad_ctrl(etimer_ctrl->base, quad_ctrl);
    sdrv_etimer_lld_lcnt_sig_en(etimer_ctrl->base, SDRV_ETIMR_LCNT_A, true);
    sdrv_etimer_lld_cpt_quad_en(etimer_ctrl->base, true);
}

/**
 * @brief configure quad clr set pulse
 * 
 *
 * @param[in] dev pwm common instance
 * @param[in] ovf quad config
 *
 */
void etimer_pwm_quad_clr_set(pwm_dev_t *dev, uint32_t ovf)
{
    etimer_pwm_dev_t *etimer_pwm = dev->priv;
    etimer_pwm_controller_t *etimer_ctrl = etimer_pwm->controller;
    sdrv_etimer_chn_id_e cpt_chn = etimer_pwm->cpt_chn;
    if (cpt_chn == SDRV_ETIMR_CHN_A)
    {
        /* cpt_a is working for pwm state capture now */
        return;
    }
    etimer_cmp_event_mode_t cmp_out_mode = {
        .cmp_00_out = SDRV_ETIMR_POSITIVE_PULSE,
        .cmp_01_out = SDRV_ETIMR_KEEP,
        .cmp_10_out = SDRV_ETIMR_POSITIVE_PULSE,
        .cmp_11_out = SDRV_ETIMR_KEEP,  
    };
    cmp_config_t cmp_config;
    memset(&cmp_config, 0, sizeof(cmp_config));
    cmp_config.cnt_sel = SDRV_ETIMR_CMP_LCNT;
    cmp_config.out_mode = SDRV_ETIMR_CMP_OUT_CMP0_CMP1;
    cmp_config.con_mode = SDRV_ETIMR_CMP_CONSECUTIVE;
    cmp_config.cmp_mode = SDRV_ETIMR_CMP_DUAL;
    sdrv_etimer_lld_cmp_val(etimer_ctrl->base, SDRV_ETIMR_CHN_A, SDRV_ETIMR_CMP_00, 1);
    sdrv_etimer_lld_cmp_val(etimer_ctrl->base, SDRV_ETIMR_CHN_A, SDRV_ETIMR_CMP_01, 3);
    sdrv_etimer_lld_cmp_val(etimer_ctrl->base, SDRV_ETIMR_CHN_A, SDRV_ETIMR_CMP_10, ovf);
    sdrv_etimer_lld_cmp_val(etimer_ctrl->base, SDRV_ETIMR_CHN_A, SDRV_ETIMR_CMP_11, 3);
    sdrv_etimer_lld_cmp_val_upd(etimer_ctrl->base, SDRV_ETIMR_CHN_A);
    sdrv_etimer_lld_cmp_out_mode(etimer_ctrl->base, SDRV_ETIMR_CHN_A, &cmp_out_mode);
    sdrv_etimer_lld_cmp_cfg(etimer_ctrl->base, SDRV_ETIMR_CHN_A, &cmp_config);
    sdrv_etimer_lld_cmp_config_set(etimer_ctrl->base, SDRV_ETIMR_CHN_A);
}


/**
 * @brief configure etimer cpt dma etimer
 *
 * @param[in] dev pwm common instance
 * @param[in] dma_cfg dma config
 *
 */
void etimer_pwm_cpt_dma(pwm_dev_t *dev, etimer_pwm_dma_t *dma_cfg)
{
    etimer_pwm_dev_t *etimer_pwm = dev->priv;
    etimer_pwm_controller_t *etimer_ctrl = etimer_pwm->controller;
    sdrv_etimer_chn_id_e cpt_chn = etimer_pwm->cpt_chn;
    if (cpt_chn < TOTAL_CHN_NUM_PER_ETMR) {
        uint32_t wml = dma_cfg->wml;
        chn_etimer_dma_ctrl_t dma_config;
        memset(&dma_config, 0, sizeof(dma_config));
        dma_config.chn_sig_mask = dma_cfg->chn_sig_mask;
        dma_config.bit_mode = 0;
        dma_config.block_sel = SDRV_ETIMR_CPT_BLOCK;
        dma_config.chn_en = 1;
        sdrv_etimer_lld_dma_wml(etimer_ctrl->base, cpt_chn, wml);
        sdrv_etimer_lld_dma_chn_cfg(etimer_ctrl->base, cpt_chn, &dma_config);
    }
}

/**
 * @brief configure etimer cmp dma etimer
 *
 * @param[in] dev pwm common instance
 * @param[in] dma_cfg dma config
 *
 */
void etimer_pwm_cmp_dma(pwm_dev_t *dev, etimer_pwm_dma_t *dma_cfg)
{
    etimer_pwm_dev_t *etimer_pwm = dev->priv;
    etimer_pwm_controller_t *etimer_ctrl = etimer_pwm->controller;
    sdrv_etimer_chn_id_e cmp_chn = etimer_pwm->cmp_chn;
    if (cmp_chn < TOTAL_CHN_NUM_PER_ETMR) {
        uint32_t wml = dma_cfg->wml;
        chn_etimer_dma_ctrl_t dma_config;
        memset(&dma_config, 0, sizeof(dma_config));
        dma_config.chn_sig_mask = dma_cfg->chn_sig_mask;
        dma_config.bit_mode = dma_cfg->bit_mode;
        dma_config.block_sel = SDRV_ETIMR_CMP_BLOCK;
        dma_config.chn_en = 1;
        sdrv_etimer_lld_dma_wml(etimer_ctrl->base, cmp_chn, wml);
        sdrv_etimer_lld_dma_chn_cfg(etimer_ctrl->base, cmp_chn, &dma_config);
    }
}

/**
 * @brief compare clr/set configure
 * @param[in] dev pwm common instance
 * @param[in] input_sel input sellect
 * @param[in] trig_val cmp set trigger sel
 * @param[in] trig_mode cmp trigger mode sel
 */
void etimer_pwm_cmp_input(pwm_dev_t *dev, sdrv_etimer_trig_sel_e input_sel,
                          sdrv_etimer_trig_set_clr_e trig_val, sdrv_etimer_cmp_event_trig_e trig_mode)
{
    etimer_pwm_dev_t *etimer_pwm = dev->priv;
    etimer_pwm_controller_t *etimer_ctrl = etimer_pwm->controller;
    sdrv_etimer_chn_id_e cmp_chn = etimer_pwm->cmp_chn;
    if (cmp_chn < TOTAL_CHN_NUM_PER_ETMR) {
        sdrv_etimer_lld_cmp_input_sel(etimer_ctrl->base, cmp_chn, input_sel, trig_val);
        sdrv_etimer_lld_cmp_trig_cfg(etimer_ctrl->base, cmp_chn, input_sel, trig_mode);
    }
}

/**
 * @brief lcnt init val set
 * @param[in] dev pwm common instance
 * @param[in] init_val cnt init value
 */
void etimer_pwm_lcnt_init_val_set(pwm_dev_t *dev, uint32_t init_val)
{
    etimer_pwm_dev_t *etimer_pwm = dev->priv;
    etimer_pwm_controller_t *etimer_ctrl = etimer_pwm->controller;
    sdrv_etimer_chn_id_e cpt_chn = etimer_pwm->cpt_chn;
    if (cpt_chn < TOTAL_CHN_NUM_PER_ETMR) {
        /* cnt select */
        sdrv_etimer_cnt_id_e cpt_cnt_sel = cpt_chn + 2;
        sdrv_etimer_lld_cnt_init(etimer_ctrl->base, cpt_cnt_sel, init_val);
    }
}

/**
 * @brief capture clr/set configure
 * @param[in] dev pwm common instance
 * @param[in] input_sel input sellect
 * @param[in] trig_val capture set trigger sel
 * @param[in] trig_mode lcnt trigger mode sel
 */
void etimer_pwm_cpt_input(pwm_dev_t *dev, sdrv_etimer_trig_sel_e input_sel,
                          uint16_t trig_val, sdrv_etimer_cmp_event_trig_e trig_mode, 
                          sdrv_etimer_set_upd_sel_e set_upd)
{
    etimer_pwm_dev_t *etimer_pwm = dev->priv;
    etimer_pwm_controller_t *etimer_ctrl = etimer_pwm->controller;
    sdrv_etimer_chn_id_e cpt_chn = etimer_pwm->cpt_chn;
    if (cpt_chn < TOTAL_CHN_NUM_PER_ETMR) {
        /* cnt select */
        sdrv_etimer_cnt_id_e cpt_cnt_sel = cpt_chn + 2;
        sdrv_etimer_lld_lcnt_trig_cfg(etimer_ctrl->base, cpt_cnt_sel, input_sel, trig_mode, set_upd);
        if ((input_sel == SDRV_ETIMR_TRIG_SEL_SET) || (input_sel == SDRV_ETIMR_TRIG_SEL_CLR))
        {
            sdrv_etimer_lld_cpt_input_set_clr_sel(etimer_ctrl->base, cpt_chn, input_sel, trig_val);
        }
        else if (input_sel == SDRV_ETIMR_TRIG_SEL_SIG)
        {
            sdrv_etimer_lld_cpt_input_sig_sel(etimer_ctrl->base, cpt_chn, input_sel, trig_val);
        }
        else if (input_sel == SDRV_ETIMR_TRIG_SEL_DIR)
        {
            sdrv_etimer_lld_cpt_input_dir_sel(etimer_ctrl->base, cpt_chn, input_sel, trig_val);
        }
        else
        {
            return;
        }
    }
}

/**
 * @brief capture clr/set configure
 * @param[in] dev pwm common instance
 * @param[in] input_sel input sellect
 * @param[in] trig_val capture set trigger sel
 * @param[in] trig_mode lcnt trigger mode sel
 */
void etimer_pwm_ext_output(pwm_dev_t *dev, sdrv_etimer_trig_sel_e output_sel,
                           uint16_t trig_val)
{
    etimer_pwm_dev_t *etimer_pwm = dev->priv;
    etimer_pwm_controller_t *etimer_ctrl = etimer_pwm->controller;
    if (output_sel == SDRV_ETIMR_TRIG_SEL_DIR)
    {
        sdrv_etimer_lld_ext_output_dir_sel(etimer_ctrl->base, output_sel, trig_val);
    }
    else if ((output_sel == SDRV_ETIMR_TRIG_SEL_SET) || (output_sel == SDRV_ETIMR_TRIG_SEL_CLR))
    {
        sdrv_etimer_lld_ext_output_set_clr_sel(etimer_ctrl->base, output_sel, trig_val);
    }
    else
    {
        return;
    }
}

/**
 * @brief get signal status
 * @param[in] dev pwm common instance
 */
uint32_t etimer_pwm_get_signal_status(pwm_dev_t *dev)
{
    etimer_pwm_dev_t *etimer_pwm = dev->priv;
    etimer_pwm_controller_t *etimer_ctrl = etimer_pwm->controller;
    uint32_t sig_sta = sdrv_etimer_lld_get_sig_status(etimer_ctrl->base);
    return sig_sta;
}

/**
 * @brief get cpt_a dir signal status
 * @param[in] dev pwm common instance
 */
bool etimer_pwm_get_cpt_a_dir_up(pwm_dev_t *dev)
{
    etimer_pwm_dev_t *etimer_pwm = dev->priv;
    etimer_pwm_controller_t *etimer_ctrl = etimer_pwm->controller;
    uint32_t sig_sta = sdrv_etimer_lld_get_sig_status(etimer_ctrl->base);
    bool dir_up = false;
    /* bit 17 : cpt_a_dir */
    if (sig_sta & (0x1U << 17))
    {
        dir_up = false;
    }
    else
    {
        dir_up = true;
    }
    return dir_up;
}

/**
 * @brief get current cnt value
 * @param[in] dev pwm common instance
 * @param[in] cnt_x cnt id
 */
uint32_t etimer_pwm_get_cnt_value(pwm_dev_t *dev, sdrv_etimer_cnt_id_e cnt_x)
{
    etimer_pwm_dev_t *etimer_pwm = dev->priv;
    etimer_pwm_controller_t *etimer_ctrl = etimer_pwm->controller;
    uint32_t cnt_val = sdrv_etimer_lld_get_cnt_value(etimer_ctrl->base, cnt_x);
    return cnt_val;
}

/**
 * @brief set cmp eid
 * @param[in] dev pwm common instance
 * @param[in] cmp_event compare event
 * @param[in] val eid val
 */
void etimer_pwm_set_eid(pwm_dev_t *dev, sdrv_etimer_cmp_event_e cmp_event, uint32_t val)
{
    etimer_pwm_dev_t *etimer_pwm = dev->priv;
    etimer_pwm_controller_t *etimer_ctrl = etimer_pwm->controller;
    sdrv_etimer_chn_id_e cmp_chn = etimer_pwm->cmp_chn;
    if (cmp_chn != SDRV_ETIMR_CHN_A)
    {
        return;
    }
    sdrv_etimer_lld_cmp_eid(etimer_ctrl->base, cmp_event, val);
    sdrv_etimer_lld_cmp_eid_upd(etimer_ctrl->base);
    return;
}

/**
 * @brief etimer controller initital
 *
 * @param[in] etimer_ctrl  etimer controller instance
 * @param[in] cfg etimer controller configure
 *
 */
void etimer_pwm_controller_init(etimer_pwm_controller_t *etimer_ctrl,
                                const etimer_pwm_cfg_t *cfg)
{
    etimer_ctrl->base = cfg->base;
    etimer_ctrl->clk_div = cfg->clk_div;

    clk_set_t clk_config;
    clk_config.src_clk_sel = cfg->clk_src;
    clk_config.div_num = cfg->clk_div;
    sdrv_etimer_lld_clk_mon_en(etimer_ctrl->base, false);
    sdrv_etimer_lld_clk_cfg(etimer_ctrl->base, &clk_config);
    sdrv_etimer_lld_clk_mon_en(etimer_ctrl->base, true);

    if (cfg->irq >= 0)
    {
        sdrv_etimer_lld_int_enable(etimer_ctrl->base, cfg->irq_id);
        irq_attach(cfg->irq, etimer_pwm_irq_handle, etimer_ctrl);
        irq_enable(cfg->irq);
    }
}

/**
 * @brief Probe etimer channel device.
 *
 * Get etimer device instance with specified channel ID.
 *
 * @param[in] etimer etimer controller
 * @param[in] etimer_cfg etimer channel configure
 * @return dev pwm_dev_t type
 */
pwm_dev_t *etimer_pwm_probe(etimer_pwm_controller_t *etimer_ctrl,
                            const etimer_cfg_t *etimer_cfg)
{
    sdrv_etimer_chn_id_e cpt_chn = etimer_cfg->cpt_chn;
    sdrv_etimer_chn_id_e cmp_chn = etimer_cfg->cmp_chn;
    etimer_pwm_dev_t *etimer_pwm;
    if (cmp_chn == SDRV_ETIMR_CHN_INVALID) {
        etimer_pwm = &etimer_ctrl->pwm[cpt_chn];
    } else {
        etimer_pwm = &etimer_ctrl->pwm[cmp_chn];
    }
    pwm_dev_t *dev = &etimer_pwm->dev;
    etimer_pwm->controller = etimer_ctrl;
    etimer_pwm->cpt_chn = cpt_chn;
    etimer_pwm->cmp_chn = cmp_chn;
    etimer_pwm->cmp_cfg = (etimer_cmp_cfg_t *)&etimer_cfg->cmp_cfg;
    dev->clk_div = etimer_ctrl->clk_div;
    dev->ops = &etimer_pwm_ops;
    dev->priv = etimer_pwm;
    return dev;
}
