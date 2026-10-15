/*
 * @File:sdrv_epwm_drv.c
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * @Descripttion: taishan epwm hld driver
 *
 * @Revision History:
 * ----------------
 */

#define LOG_TAG "sdrv_epwm_drv.c "
#include <driver/clk.h>
#include <errno.h>
#include <regs_base.h>
#include <string.h>
#include <types.h>

#include "driver/taishan/epwm/sdrv_epwm_drv.h"

#include "bits.h"
#include "debug.h"
#include "driver/irq.h"
#include "driver/pwm.h"
#include "reg.h"
#include "sdrv_epwm.h"

static void sdrv_epwm_start(pwm_dev_t *dev);
static void sdrv_epwm_stop(pwm_dev_t *dev);
static void sdrv_epwm_config(pwm_dev_t *dev, const pwm_state_t *state);

static pwm_ops_t sdrv_epwm_ops = {
    .start = sdrv_epwm_start,
    .stop = sdrv_epwm_stop,
    .config = sdrv_epwm_config,
};

/**
 * @brief Start ePWM channel output.
 *
 * @param[in] dev pwm common instance
 *
 */
static void sdrv_epwm_start(pwm_dev_t *dev)
{
    sdrv_epwm_cmp_en(dev, true);
    sdrv_epwm_cnt_en(dev, true);
}

/**
 * @brief Stop ePWM channel output.
 *
 * @param[in] dev pwm common instance
 *
 */
static void sdrv_epwm_stop(pwm_dev_t *dev)
{
    sdrv_epwm_cmp_en(dev, false);
    sdrv_epwm_cnt_en(dev, false);
}

/**
 * @brief ePWM channel config.
 *
 * @param[in] dev pwm common instance
 * @param[in] state pwm duty and period
 *
 */
static void sdrv_epwm_config(pwm_dev_t *dev, const pwm_state_t *state)
{
    sdrv_epwm_dev_t *epwm_dev = dev->priv;
    sdrv_epwm_controller_t *epwm_ctrl = epwm_dev->controller;
    sdrv_epwm_channel_e chnl = epwm_dev->id;
    const epwm_cmp_cfg_t *cmp_cfg = epwm_dev->cmp_cfg;

    uint32_t cmp_pulse_wid0_cmp00_cnt =
        EPWM_NS_TO_VAL_1(cmp_cfg->cmp_pulse_wid0.cmp00, epwm_ctrl->clk_div);
    // cmp_pulse_wid0.cmp00 SHOULD LESS THAN OR EQUAL TO 1/EPWM_SRC_CLK * 0xFF
    uint32_t cmp_pulse_wid0_cmp01_cnt =
        EPWM_NS_TO_VAL_1(cmp_cfg->cmp_pulse_wid0.cmp01, epwm_ctrl->clk_div);
    // cmp_pulse_wid0.cmp01 SHOULD LESS THAN OR EQUAL TO 1/EPWM_SRC_CLK * 0xFF
    uint32_t cmp_pulse_wid0_cmp10_cnt =
        EPWM_NS_TO_VAL_1(cmp_cfg->cmp_pulse_wid0.cmp10, epwm_ctrl->clk_div);
    // cmp_pulse_wid0.cmp10 SHOULD LESS THAN OR EQUAL TO 1/EPWM_SRC_CLK * 0xFF
    uint32_t cmp_pulse_wid0_cmp11_cnt =
        EPWM_NS_TO_VAL_1(cmp_cfg->cmp_pulse_wid0.cmp11, epwm_ctrl->clk_div);
    // cmp_pulse_wid0.cmp11 SHOULD LESS THAN OR EQUAL TO 1/EPWM_SRC_CLK * 0xFF
    uint32_t cmp_pulse_wid1_cmp0_ovf =
        EPWM_NS_TO_VAL_1(cmp_cfg->cmp_pulse_wid1.cmp0_ovf, epwm_ctrl->clk_div);
    // cmp_pulse_wid0.cmp0_ovf SHOULD LESS THAN OR EQUAL TO 1/EPWM_SRC_CLK *
    // 0xFF
    uint32_t cmp_pulse_wid1_cmp1_ovf =
        EPWM_NS_TO_VAL_1(cmp_cfg->cmp_pulse_wid1.cmp1_ovf, epwm_ctrl->clk_div);
    // cmp_pulse_wid0.cmp1_ovf SHOULD LESS THAN OR EQUAL TO 1/EPWM_SRC_CLK *
    // 0xFF

    sdrv_epwm_cmp_cfg_t cfg;
    memset(&cfg, 0, sizeof(cfg));
    cfg.con_mode = cmp_cfg->con_mode;
    cfg.cnt_sel = cmp_cfg->cmp_cnt_sel;
    cfg.cmp_x_out_mode = cmp_cfg->chnl_out_mode;
    cfg.cmp_x_mode = cmp_cfg->cmp_mode;
    cfg.refresh_intval = cmp_cfg->refresh_intval;
    cfg.out_mode.cmp00 = cmp_cfg->out_mode.cmp00;
    cfg.out_mode.cmp01 = cmp_cfg->out_mode.cmp01;
    cfg.out_mode.cmp10 = cmp_cfg->out_mode.cmp10;
    cfg.out_mode.cmp11 = cmp_cfg->out_mode.cmp11;
    cfg.out_mode.cmp0_ovf = cmp_cfg->out_mode.cmp0_ovf;
    cfg.out_mode.cmp1_ovf = cmp_cfg->out_mode.cmp1_ovf;
    cfg.wid0.cmp00 = cmp_pulse_wid0_cmp00_cnt;
    cfg.wid0.cmp01 = cmp_pulse_wid0_cmp01_cnt;
    cfg.wid0.cmp10 = cmp_pulse_wid0_cmp10_cnt;
    cfg.wid0.cmp11 = cmp_pulse_wid0_cmp11_cnt;
    cfg.wid1.cmp0_ovf = cmp_pulse_wid1_cmp0_ovf;
    cfg.wid1.cmp1_ovf = cmp_pulse_wid1_cmp1_ovf;
    cfg.sw_rld_mode = cmp_cfg->sw_rld_mode;

    // cnt cfg
    sdrv_epwm_cnt_ovf_upd(dev, state->period);
    // dma cfg
    if (cmp_cfg->dma_cfg.dma_enable) {
        sdrv_epwm_lld_dma_wml(epwm_ctrl->base, chnl, cmp_cfg->dma_cfg.fifo_wml);
        sdrv_epwm_lld_chn_dma_ctrl_chn_en(epwm_ctrl->base, chnl, true);
    } else
        sdrv_epwm_cmp_val_upd(dev, state);
#ifdef CONFIG_SDRV_EPWM_L
    if (cmp_cfg->center_align_mode)
        sdrv_epwm_center_align_mode_en(dev, true);
#endif
    sdrv_epwm_lld_cmp_config(epwm_ctrl->base, chnl, &cfg);
}

/**
 * @brief epwm interrupt handler
 *
 * @param[in] irq irq_num
 * @param[in] arg input argument
 *
 */
static int sdrv_epwm_irq_handle(uint32_t irq, void *arg)
{
    sdrv_epwm_controller_t *epwm_ctrl = arg;
    uint32_t int_sta = sdrv_epwm_lld_int_sta_get(epwm_ctrl->base);
    sdrv_epwm_lld_int_sta_clr(epwm_ctrl->base, int_sta);

    for (uint8_t id = SDRV_EPWM_CNT_G0; id < SDRV_EPWM_CNT_MAX_NUM; id++) {
        if (BIT_SET(int_sta, SDRV_EPWM_STA_CNT_OVF_SHIFT(id))) {
            sdrv_epwm_dev_t *epwm_dev = &epwm_ctrl->pwm[id];
            pwm_expiry(&epwm_dev->dev);
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
void sdrv_epwm_multi_cmp_mode(pwm_dev_t *dev, const pwm_state_t *state,
                              epwm_multi_cmp_val_t *multi_cmp)
{
    sdrv_epwm_dev_t *epwm_dev = dev->priv;
    sdrv_epwm_controller_t *epwm_ctrl = epwm_dev->controller;
    sdrv_epwm_channel_e chnl = epwm_dev->id;
    uint32_t cmp00_cnt =
        EPWM_NS_TO_CNTVAL(multi_cmp->cmp00_val, epwm_ctrl->clk_div);
    uint32_t cmp01_cnt =
        EPWM_NS_TO_CNTVAL(multi_cmp->cmp01_val, epwm_ctrl->clk_div);
    uint32_t cmp10_cnt =
        EPWM_NS_TO_CNTVAL(multi_cmp->cmp10_val, epwm_ctrl->clk_div);
    uint32_t cmp11_cnt =
        EPWM_NS_TO_CNTVAL(multi_cmp->cmp11_val, epwm_ctrl->clk_div);

    sdrv_epwm_lld_cmp_val(epwm_ctrl->base, chnl, SDRV_EPWM_CMP_00, cmp00_cnt);
    sdrv_epwm_lld_cmp_val(epwm_ctrl->base, chnl, SDRV_EPWM_CMP_01, cmp01_cnt);
    sdrv_epwm_lld_cmp_val(epwm_ctrl->base, chnl, SDRV_EPWM_CMP_10, cmp10_cnt);
    sdrv_epwm_lld_cmp_val(epwm_ctrl->base, chnl, SDRV_EPWM_CMP_11, cmp11_cnt);
}

/**
 * @brief configure epwm counter clear
 *
 * @param[in] dev pwm common instance
 * @param[in] cnt_clr_trig_sel sellect cnt_clr_trig_polarity
 * @param[in] clr_sel input_clr_sel_trig
 *
 */
void sdrv_epwm_cnt_clr_cfg(pwm_dev_t *dev,
                           sdrv_epwm_event_trigger_mode_e cnt_clr_trig_sel,
                           sdrv_epwm_input_sel_trig_e clr_sel)
{
    sdrv_epwm_dev_t *epwm_dev = dev->priv;
    sdrv_epwm_controller_t *epwm_ctrl = epwm_dev->controller;
    const epwm_cmp_cfg_t *cmp_cfg = epwm_dev->cmp_cfg;

    sdrv_epwm_lld_cnt_cfg_clr_trig_sel(epwm_ctrl->base, cmp_cfg->cmp_cnt_sel,
                                       cnt_clr_trig_sel);
    sdrv_epwm_lld_cnt_input_sel_clr(epwm_ctrl->base, cmp_cfg->cmp_cnt_sel,
                                    clr_sel);
}

/**
 * @brief configure epwm counter overflow upload cfg
 *
 * @param[in] dev pwm common instance
 * @param[in] set_mode_sel sellect set_mode
 * @param[in] cnt_set_trig_sel sellect cnt_set_trig_polarity
 * @param[in] set_sel input_set_sel_trig
 *
 */
void sdrv_epwm_cnt_ovf_upd_cfg(pwm_dev_t *dev,
                               sdrv_epwm_cnt_cfg_set_upd_sel_e set_mode_sel,
                               sdrv_epwm_event_trigger_mode_e cnt_set_trig_sel,
                               sdrv_epwm_input_sel_trig_e set_sel)
{
    sdrv_epwm_dev_t *epwm_dev = dev->priv;
    sdrv_epwm_controller_t *epwm_ctrl = epwm_dev->controller;
    const epwm_cmp_cfg_t *cmp_cfg = epwm_dev->cmp_cfg;

    sdrv_epwm_lld_cnt_cfg_set_trig_sel(epwm_ctrl->base, cmp_cfg->cmp_cnt_sel,
                                       set_mode_sel, cnt_set_trig_sel);
    sdrv_epwm_lld_cnt_input_sel_set(epwm_ctrl->base, cmp_cfg->cmp_cnt_sel,
                                    set_sel);
}

/**
 * @brief software trigger
 *
 * @param[in] dev pwm common instance
 * @param[in] tirg sellect trigger mode
 *
 */
void sdrv_epwm_sw_trig(pwm_dev_t *dev, sdrv_epwm_sw_trig_ctrl_e trig)
{

    sdrv_epwm_dev_t *epwm_dev = dev->priv;
    sdrv_epwm_controller_t *epwm_ctrl = epwm_dev->controller;
    sdrv_epwm_channel_e chnl = epwm_dev->id;

    sdrv_epwm_lld_sw_trig_ctrl(epwm_ctrl->base, chnl, trig);
}

/**
 * @brief configure epwm death time
 *
 * @param[in] dev pwm common instance
 * @param[in] prefin_pol prefinial polarity
 * @param[in] dti_val death time value
 *
 */
void sdrv_epwm_dti(pwm_dev_t *dev, sdrv_epwm_cmp_prefin_pol_t *prefin_pol,
                   const uint32_t dti_val)
{
    sdrv_epwm_dev_t *epwm_dev = dev->priv;
    sdrv_epwm_controller_t *epwm_ctrl = epwm_dev->controller;
    sdrv_epwm_channel_e chnl = epwm_dev->id;

    uint32_t dti_cnt = EPWM_NS_TO_VAL_1(dti_val, epwm_ctrl->clk_div);

    sdrv_epwm_lld_cmp_prefin_pol(epwm_ctrl->base, chnl, prefin_pol);

    sdrv_epwm_lld_cmp_dti_ctrl_inv(epwm_ctrl->base, chnl, true);

    sdrv_epwm_lld_cmp_dti_wid(epwm_ctrl->base, chnl, dti_cnt, dti_cnt);

    sdrv_epwm_lld_cmp_dti_ctrl_en(epwm_ctrl->base, chnl, true);
}

/**
 * @brief configure sse epwm output
 *
 * @param[in] dev pwm common instance
 * @param[in] sse_cfg sse_mode and edge sellect
 * @param[in] sse_reg_val sse register value
 *
 */
void sdrv_epwm_sse(pwm_dev_t *dev, sdrv_epwm_cmp_sse_ctrl_t *sse_cfg,
                   uint32_t sse_reg_val)
{

    sdrv_epwm_dev_t *epwm_dev = dev->priv;
    sdrv_epwm_controller_t *epwm_ctrl = epwm_dev->controller;
    sdrv_epwm_channel_e chnl = epwm_dev->id;

    sdrv_epwm_lld_cmp_sse_ctrl_cfg(epwm_ctrl->base, chnl, sse_cfg);
    sdrv_epwm_lld_cmp_sse_ctrl_cfg_so_det_fault(epwm_ctrl->base, chnl,
                                                sse_cfg->sse_so_det_fault);
    sdrv_epwm_lld_cmp_sse_reg(epwm_ctrl->base, chnl, sse_reg_val);
    sdrv_epwm_lld_cmp_sse_ctrl_sse_en(epwm_ctrl->base, chnl, true);
}

/**
 * @brief configure modulation frequency control
 *
 * @param[in] dev pwm common instance
 * @param[in] val mfc up times to 2^val times
 *
 */
void sdrv_epwm_mfc(pwm_dev_t *dev, uint8_t val)
{

    sdrv_epwm_dev_t *epwm_dev = dev->priv;
    sdrv_epwm_controller_t *epwm_ctrl = epwm_dev->controller;
    const epwm_cmp_cfg_t *cmp_cfg = epwm_dev->cmp_cfg;

    sdrv_epwm_lld_cnt_mfc(epwm_ctrl->base, cmp_cfg->cmp_cnt_sel, val);
}

/**
 * @brief configure compare data format
 *
 * @param[in] dev pwm common instance
 * @param[in] dat_format cmp data formate
 */
void sdrv_epwm_cmp_data_format(
    pwm_dev_t *dev, sdrv_epwm_chn_dma_ctrl_cmp_x_dat_format_e dat_format)
{
    sdrv_epwm_dev_t *epwm_dev = dev->priv;
    sdrv_epwm_controller_t *epwm_ctrl = epwm_dev->controller;
    sdrv_epwm_channel_e chnl = epwm_dev->id;

    sdrv_epwm_lld_chn_dma_ctrl_cmp_dat_format(epwm_ctrl->base, chnl,
                                              dat_format);
}

/**
 * @brief configure compare dither mode
 *
 * @param[in] dev pwm common instance
 * @param[in] clip_rslt dither clip_rslt val
 * @param[in] init_offset dither init_offset val
 */
void sdrv_epwm_cmp_dither(pwm_dev_t *dev, uint8_t clip_rslt,
                          uint16_t init_offset)
{
    sdrv_epwm_dev_t *epwm_dev = dev->priv;
    sdrv_epwm_controller_t *epwm_ctrl = epwm_dev->controller;
    sdrv_epwm_channel_e chnl = epwm_dev->id;

    sdrv_epwm_lld_cmp_dither(epwm_ctrl->base, chnl, clip_rslt, init_offset);
    if (init_offset)
        sdrv_epwm_lld_cmp_dither_init_offset_en(epwm_ctrl->base, chnl, true);

    sdrv_epwm_lld_cmp_dither_en(epwm_ctrl->base, chnl, true);
}

/**
 * @brief configure compare multi channels
 *
 * @param[in] dev pwm common instance
 * @param[in] full_en full channels enable
 */
void sdrv_epwm_cmp_multi_chnl(pwm_dev_t *dev, bool full_en)
{
    sdrv_epwm_dev_t *epwm_dev = dev->priv;
    sdrv_epwm_controller_t *epwm_ctrl = epwm_dev->controller;
    sdrv_epwm_channel_e chnl = epwm_dev->id;

    if (full_en)
        sdrv_epwm_lld_chn_dma_ctrl_four_chan_mode(epwm_ctrl->base, true);
    else
        sdrv_epwm_lld_chn_dma_ctrl_two_chn_xx_mode(epwm_ctrl->base, chnl, true);
}

/**
 * @brief configure compare event/cnt overflow event
 *
 * @param[in] dev pwm common instance
 * @param[in] src_sel choose cmp or cnt trigger source
 */
void sdrv_epwm_ce_ctrl(pwm_dev_t *dev, bool src_sel)
{
    sdrv_epwm_dev_t *epwm_dev = dev->priv;
    sdrv_epwm_controller_t *epwm_ctrl = epwm_dev->controller;
    const epwm_cmp_cfg_t *cmp_cfg = epwm_dev->cmp_cfg;
    sdrv_epwm_channel_e chnl = epwm_dev->id;

    if (src_sel) {
        sdrv_epwm_lld_cmp_config_ce_en(epwm_ctrl->base, chnl, true);
    }

    else {
        sdrv_epwm_lld_cnt_cfg_ce_en(epwm_ctrl->base, cmp_cfg->cmp_cnt_sel,
                                    true);
    }
}

/**
 * @brief configure dma request except fifo wml trig
 *
 * @param[in] dev pwm common instance
 * @param[in] sel_trig sellect trigger source
 */
void sdrv_epwm_ce_dma_trig(pwm_dev_t *dev, uint8_t sel_trig)
{

    sdrv_epwm_dev_t *epwm_dev = dev->priv;
    sdrv_epwm_controller_t *epwm_ctrl = epwm_dev->controller;

    sdrv_epwm_lld_config_dma_ctl_cfg(epwm_ctrl->base, false, sel_trig);
    sdrv_epwm_lld_config_dma_ctl_en(epwm_ctrl->base, true);
}

/**
 * @brief filter input fault source signals
 *
 * @param[in] dev pwm common instance
 * @param[in] flt_cfg config filter
 */
void sdrv_epwm_fault_flt(pwm_dev_t *dev, sdrv_epwm_fault_flt_t *flt_cfg)
{

    sdrv_epwm_dev_t *epwm_dev = dev->priv;
    sdrv_epwm_controller_t *epwm_ctrl = epwm_dev->controller;
    sdrv_epwm_channel_e chnl = epwm_dev->id;

    //  pos/neg_band_wid should equal or greater than 2 due to 0~15 indicate
    //  2~17 sample interval
    //  smpl_intval should equal or greater than 1 due to 0~255 indicate
    //  1~256 timer clock cycles
    sdrv_epwm_lld_fault_flt(epwm_ctrl->base, chnl, flt_cfg);

    sdrv_epwm_lld_fault_flt_en(epwm_ctrl->base, chnl, true);
}

/**
 * @brief configure input fault event
 *
 * @param[in] dev pwm common instance
 * @param[in] sub_chnl config which fault source to trigger
 * @param[in] fault_cfg config fault event
 * @param[in] fs_state cmp failsafe status
 */
void sdrv_epwm_fault_event(pwm_dev_t *dev, uint8_t sub_chnl,
                           sdrv_epwm_cmp_fault_event_ctrl_t *fault_cfg,
                           epwm_fs_state_t *fs_state)
{
    sdrv_epwm_dev_t *epwm_dev = dev->priv;
    sdrv_epwm_controller_t *epwm_ctrl = epwm_dev->controller;
    sdrv_epwm_channel_e chnl = epwm_dev->id;

    sdrv_epwm_lld_cmp_fault_event_ctrl_config(epwm_ctrl->base, chnl, sub_chnl,
                                              fault_cfg);

    sdrv_epwm_lld_cmp_fault_event_fs_sta_set(epwm_ctrl->base, chnl,
                                             fs_state->cmp0_fs_state,
                                             fs_state->cmp1_fs_state);

    sdrv_epwm_lld_cmp_fault_event_ctrl_en(epwm_ctrl->base, chnl, sub_chnl,
                                          true);
}

/**
 * @brief input fault event clr
 *
 * @param[in] dev pwm common instance
 * @param[in] sub_chnl config which fault source to trigger
 */
void sdrv_epwm_fault_event_clr(pwm_dev_t *dev, uint8_t sub_chnl)
{
    sdrv_epwm_dev_t *epwm_dev = dev->priv;
    sdrv_epwm_controller_t *epwm_ctrl = epwm_dev->controller;
    sdrv_epwm_channel_e chnl = epwm_dev->id;

    sdrv_epwm_lld_cmp_fault_event_ctrl_clr(epwm_ctrl->base, chnl, sub_chnl);
}

/**
 * @brief compare value upload
 *
 * @param[in] dev pwm common instance
 * @param[in] state pwm duty and period
 */
void sdrv_epwm_cmp_val_upd(pwm_dev_t *dev, const pwm_state_t *state)
{
    sdrv_epwm_dev_t *epwm_dev = dev->priv;
    sdrv_epwm_controller_t *epwm_ctrl = epwm_dev->controller;
    sdrv_epwm_channel_e chnl = epwm_dev->id;
    const epwm_cmp_cfg_t *cmp_cfg = epwm_dev->cmp_cfg;

    uint32_t period_cnt;
    uint32_t dual_period_cnt_left = 0;
    uint32_t dual_period_cnt_right = 0;
    period_cnt = EPWM_NS_TO_CNTVAL(state->period, epwm_ctrl->clk_div);
    uint32_t duty_cnt;
    if (cmp_cfg->center_align_mode) {
        duty_cnt = EPWM_NS_TO_CNTVAL(state->duty, epwm_ctrl->clk_div) / 2;
        // duty_cnt SHOULD LESS THAN OR EQUAL TO  period_cnt/2
        dual_period_cnt_left = (period_cnt) / 4 - duty_cnt / 2;
        dual_period_cnt_right = (period_cnt) / 4 + duty_cnt / 2;
    } else {
        duty_cnt = EPWM_NS_TO_CNTVAL(state->duty, epwm_ctrl->clk_div);
        dual_period_cnt_left = (period_cnt) / 2 - duty_cnt / 2;
        dual_period_cnt_right = (period_cnt) / 2 + duty_cnt / 2;
    }

    if (cmp_cfg->cmp_mode == SDRV_EPWM_SINGLE_COMPARE_MODE) {
        sdrv_epwm_lld_cmp_val(epwm_ctrl->base, chnl, SDRV_EPWM_CMP_00,
                              duty_cnt);
        if (cmp_cfg->chnl_out_mode == EPWM_BOTH_COMP0_AND_COMP1_OUTPUT)
            sdrv_epwm_lld_cmp_val(epwm_ctrl->base, chnl, SDRV_EPWM_CMP_10,
                                  duty_cnt);
    } else if (cmp_cfg->cmp_mode == SDRV_EPWM_DUAL_COMPARE_MODE) {
        sdrv_epwm_lld_cmp_val(epwm_ctrl->base, chnl, SDRV_EPWM_CMP_00,
                              dual_period_cnt_left);
        sdrv_epwm_lld_cmp_val(epwm_ctrl->base, chnl, SDRV_EPWM_CMP_01,
                              dual_period_cnt_right);
        if (cmp_cfg->chnl_out_mode == EPWM_BOTH_COMP0_AND_COMP1_OUTPUT) {
            sdrv_epwm_lld_cmp_val(epwm_ctrl->base, chnl, SDRV_EPWM_CMP_10,
                                  dual_period_cnt_left);
            sdrv_epwm_lld_cmp_val(epwm_ctrl->base, chnl, SDRV_EPWM_CMP_11,
                                  dual_period_cnt_right);
        }
    }

    sdrv_epwm_lld_cmp_ctrl_cmp_val_upd(epwm_ctrl->base, chnl);
}

/**
 * @brief cnt overflow value upload (unit is ns)
 *
 * @param[in] dev pwm common instance
 * @param[in] period cnt overflow value
 */
void sdrv_epwm_cnt_ovf_upd(pwm_dev_t *dev, uint32_t period)
{
    sdrv_epwm_dev_t *epwm_dev = dev->priv;
    sdrv_epwm_controller_t *epwm_ctrl = epwm_dev->controller;
    const epwm_cmp_cfg_t *cmp_cfg = epwm_dev->cmp_cfg;
    uint32_t period_cnt;
    if (cmp_cfg->audio_enable)
        period_cnt = EPWM_NS_TO_VAL_1(period, epwm_ctrl->clk_div) - 1;
    else {
        if (cmp_cfg->center_align_mode)
            period_cnt = EPWM_NS_TO_CNTVAL(period, epwm_ctrl->clk_div) / 2 - 1;
        else
            period_cnt = EPWM_NS_TO_VAL_1(period, epwm_ctrl->clk_div);
    }

    sdrv_epwm_lld_cnt_ovf_val(epwm_ctrl->base, cmp_cfg->cmp_cnt_sel,
                              period_cnt);
    sdrv_epwm_lld_cnt_cfg_ovf_upd(epwm_ctrl->base, cmp_cfg->cmp_cnt_sel);
}

/**
 * @brief cnt overflow value upload directly (unit is cnt val)
 *
 * @param[in] dev pwm common instance
 * @param[in] period_cnt counter overflow value
 */
void sdrv_epwm_cnt_ovf_dir_upd(pwm_dev_t *dev, uint32_t period_cnt)
{
    sdrv_epwm_dev_t *epwm_dev = dev->priv;
    sdrv_epwm_controller_t *epwm_ctrl = epwm_dev->controller;
    const epwm_cmp_cfg_t *cmp_cfg = epwm_dev->cmp_cfg;

    sdrv_epwm_lld_cnt_ovf_val(epwm_ctrl->base, cmp_cfg->cmp_cnt_sel,
                              period_cnt);
    sdrv_epwm_lld_cnt_cfg_ovf_upd(epwm_ctrl->base, cmp_cfg->cmp_cnt_sel);
}

/**
 * @brief upload chnl_b/c/d compare value with dual mode 100% duty check(unit is
 * cnt val)
 *
 * @param[in] dev pwm common instance
 * @param[in] left_point dual mode left cmp_val
 * @param[in] right_point dual mode right cmp_val
 */
void sdrv_epwm_cmp_bcd_update(pwm_dev_t *dev, uint32_t left_point,
                              uint32_t right_point)
{
    sdrv_epwm_dev_t *epwm_dev = dev->priv;
    sdrv_epwm_controller_t *epwm_ctrl = epwm_dev->controller;
    sdrv_epwm_channel_e chnl = epwm_dev->id;
    const epwm_cmp_cfg_t *cmp_cfg = epwm_dev->cmp_cfg;
    uint32_t cnt_ovf = 0;

    if (epwm_dev->pre_duty.left_point == 0 && left_point != 0) {
        cnt_ovf =
            sdrv_epwm_lld_cnt_ovf_val_rd(epwm_ctrl->base, cmp_cfg->cmp_cnt_sel);
        sdrv_epwm_lld_cmp_val(epwm_ctrl->base, chnl, SDRV_EPWM_CMP_01,
                              cnt_ovf + 1);
        sdrv_epwm_lld_cmp_ctrl_cmp_val_upd(epwm_ctrl->base, chnl);
        sdrv_epwm_cmp_sw_rld(dev);
    }

    if (left_point != 0)
        sdrv_epwm_cmp_val_upd_bcd(dev, left_point, right_point);
    else {
        sdrv_epwm_cmp_val_upd_bcd(dev, left_point, right_point + 1);
    }

    epwm_dev->pre_duty.left_point = left_point;
    epwm_dev->pre_duty.right_point = right_point;
}

/**
 * @brief compare value chnl_b/c/d upload (unit is cnt val)
 *
 * @param[in] dev pwm common instance
 * @param[in] left_point dual mode left cmp_val
 * @param[in] right_point dual mode right cmp_val
 */
void sdrv_epwm_cmp_val_upd_bcd(pwm_dev_t *dev, uint32_t left_point,
                               uint32_t right_point)
{
    sdrv_epwm_dev_t *epwm_dev = dev->priv;
    sdrv_epwm_controller_t *epwm_ctrl = epwm_dev->controller;
    sdrv_epwm_channel_e chnl = epwm_dev->id;

    sdrv_epwm_lld_cmp_val(epwm_ctrl->base, chnl, SDRV_EPWM_CMP_00, left_point);

    if (left_point != 0)
        sdrv_epwm_lld_cmp_val(epwm_ctrl->base, chnl, SDRV_EPWM_CMP_01,
                              right_point);
    else
        sdrv_epwm_lld_cmp_val(epwm_ctrl->base, chnl, SDRV_EPWM_CMP_01,
                              right_point - 1);

    sdrv_epwm_lld_cmp_ctrl_cmp_val_upd(epwm_ctrl->base, chnl);
}

/**
 * @brief compare value chnl_a upload (unit is cnt val)
 *
 * @param[in] dev pwm common instance
 * @param[in] trig_point set trig point to tmux
 */
void sdrv_epwm_cmp_val_upd_a(pwm_dev_t *dev, uint32_t trig_point)
{
    sdrv_epwm_dev_t *epwm_dev = dev->priv;
    sdrv_epwm_controller_t *epwm_ctrl = epwm_dev->controller;
    sdrv_epwm_channel_e chnl = epwm_dev->id;

    sdrv_epwm_lld_cmp_val(epwm_ctrl->base, chnl, SDRV_EPWM_CMP_00, trig_point);

    sdrv_epwm_lld_cmp_ctrl_cmp_val_upd(epwm_ctrl->base, chnl);
}

/**
 * @brief compare software reload
 *
 * @param[in] dev pwm common instance
 */
void sdrv_epwm_cmp_sw_rld(pwm_dev_t *dev)
{
    sdrv_epwm_dev_t *epwm_dev = dev->priv;
    sdrv_epwm_controller_t *epwm_ctrl = epwm_dev->controller;
    sdrv_epwm_channel_e chnl = epwm_dev->id;

    sdrv_epwm_lld_cmp_config_sw_rld_set(epwm_ctrl->base, chnl, true);
}

/**
 * @brief compare clr/set configure
 * @param[in] dev pwm common instance
 * @param[in] input_sel input sellect
 * @param[in] set_trig cmp set trigger sel
 * @param[in] clr_trig cmp clr trigger sel
 */
void sdrv_epwm_cmp_input(pwm_dev_t *dev, sdrv_epwm_input_sel_t *input_sel,
                         uint8_t set_trig, uint8_t clr_trig)
{
    sdrv_epwm_dev_t *epwm_dev = dev->priv;
    sdrv_epwm_controller_t *epwm_ctrl = epwm_dev->controller;
    sdrv_epwm_channel_e chnl = epwm_dev->id;

    sdrv_epwm_lld_cmp_config_clr_set(epwm_ctrl->base, chnl, set_trig, clr_trig);
    sdrv_epwm_lld_cmp_input_sel(epwm_ctrl->base, chnl, input_sel);
}

/**
 * @brief configure compare a event id
 *
 * @param[in] dev pwm common instance
 * @param[in] eid_val cmp00/01/10/11 eid
 */
void sdrv_epwm_cmp_a_eid(pwm_dev_t *dev, epwm_eid_val_t *eid_val)
{
    sdrv_epwm_dev_t *epwm_dev = dev->priv;
    sdrv_epwm_controller_t *epwm_ctrl = epwm_dev->controller;

    sdrv_epwm_lld_cmp_a_eid(epwm_ctrl->base, SDRV_EPWM_CMP_00, eid_val->eid00);
    sdrv_epwm_lld_cmp_a_eid(epwm_ctrl->base, SDRV_EPWM_CMP_01, eid_val->eid01);
    sdrv_epwm_lld_cmp_a_eid(epwm_ctrl->base, SDRV_EPWM_CMP_10, eid_val->eid10);
    sdrv_epwm_lld_cmp_a_eid(epwm_ctrl->base, SDRV_EPWM_CMP_11, eid_val->eid11);
    sdrv_epwm_lld_cmp_ctrl_cmp_eid_upd(epwm_ctrl->base);
}

/**
 * @brief compare channel enable.
 *
 * @param[in] dev pwm common instance
 *
 */
void sdrv_epwm_cmp_en(pwm_dev_t *dev, bool en)
{
    sdrv_epwm_dev_t *epwm_dev = dev->priv;
    sdrv_epwm_controller_t *epwm_ctrl = epwm_dev->controller;
    sdrv_epwm_channel_e chnl = epwm_dev->id;

    sdrv_epwm_lld_cmp_ctrl_cmp_en(epwm_ctrl->base, chnl, en);
}

/**
 * @brief counter enable.
 *
 * @param[in] dev pwm common instance
 *
 */
void sdrv_epwm_cnt_en(pwm_dev_t *dev, bool en)
{
    sdrv_epwm_dev_t *epwm_dev = dev->priv;
    sdrv_epwm_controller_t *epwm_ctrl = epwm_dev->controller;
    const epwm_cmp_cfg_t *cmp_cfg = epwm_dev->cmp_cfg;

    sdrv_epwm_lld_cnt_en(epwm_ctrl->base, cmp_cfg->cmp_cnt_sel, en);
}

/**
 * @brief epwm initial status.
 *
 * @param[in] dev pwm common instance
 * @param[in] cmp0_init cmp0 channel init status
 * @param[in] cmp1_init cmp1 channel init status
 *
 */
void sdrv_epwm_init_sta(pwm_dev_t *dev, bool cmp0_init, bool cmp1_init)
{
    sdrv_epwm_dev_t *epwm_dev = dev->priv;
    sdrv_epwm_controller_t *epwm_ctrl = epwm_dev->controller;
    sdrv_epwm_channel_e chnl = epwm_dev->id;

    sdrv_epwm_lld_cmp_ctrl_cmp_init_status(epwm_ctrl->base, chnl, cmp0_init,
                                           cmp1_init);

    sdrv_epwm_lld_cmp_ctrl_cmp_init_upd(epwm_ctrl->base, chnl, 0);
    sdrv_epwm_lld_cmp_ctrl_cmp_init_upd(epwm_ctrl->base, chnl, 1);
}

#ifdef CONFIG_SDRV_EPWM_L
/**
 * @brief center align mode enable.
 *
 * @param[in] dev pwm common instance
 * @param[in] en enable/diable center align mode
 *
 */
void sdrv_epwm_center_align_mode_en(pwm_dev_t *dev, bool en)
{

    sdrv_epwm_dev_t *epwm_dev = dev->priv;
    sdrv_epwm_controller_t *epwm_ctrl = epwm_dev->controller;
    sdrv_epwm_channel_e chnl = epwm_dev->id;
    const epwm_cmp_cfg_t *cmp_cfg = epwm_dev->cmp_cfg;

    sdrv_epwm_lld_cnt_cfg_ovf_rst_dis(epwm_ctrl->base, cmp_cfg->cmp_cnt_sel,
                                      en);

    sdrv_epwm_lld_cnt_center_align_mode_en(epwm_ctrl->base,
                                           cmp_cfg->cmp_cnt_sel, en);

    sdrv_epwm_lld_cmp_center_align_mode_en(epwm_ctrl->base, chnl, en);
}

/**
 * @brief compare value chnl_b/c/d upload in center align mode (unit is cnt val)
 *
 * @param[in] dev pwm common instance
 * @param[in] cmp_point center align mode cmp_val
 */
void sdrv_epwm_cmp_center_align_val_upd_bcd(pwm_dev_t *dev, uint32_t cmp_point)
{
    sdrv_epwm_dev_t *epwm_dev = dev->priv;
    sdrv_epwm_controller_t *epwm_ctrl = epwm_dev->controller;
    sdrv_epwm_channel_e chnl = epwm_dev->id;

    sdrv_epwm_lld_cmp_val(epwm_ctrl->base, chnl, SDRV_EPWM_CMP_00, cmp_point);

    sdrv_epwm_lld_cmp_ctrl_cmp_val_upd(epwm_ctrl->base, chnl);
}
#endif

/**
 * @brief epwm controller initital
 *
 * @param[in] epwm_ctrl  epwm controller instance
 * @param[in] cfg epwm controller configure
 *
 */
int sdrv_epwm_controller_init(sdrv_epwm_controller_t *epwm_ctrl,
                              const sdrv_epwm_cfg_t *cfg)
{
    epwm_ctrl->base = cfg->base;
    epwm_ctrl->clk_div = cfg->clk_div;

    sdrv_epwm_lld_clk_mon_en(epwm_ctrl->base, false, false);
    sdrv_epwm_lld_clk_config(epwm_ctrl->base, cfg->clk_src, cfg->clk_div);
    while (sdrv_epwm_lld_clk_config_div_num_upd_sta(epwm_ctrl->base))
        ;
    sdrv_epwm_lld_clk_mon_en(epwm_ctrl->base, true, true);

    if (cfg->irq > 0) {
        sdrv_epwm_lld_int_enable(epwm_ctrl->base, cfg->irq_id);
        irq_attach(cfg->irq, sdrv_epwm_irq_handle, epwm_ctrl);
        irq_enable(cfg->irq);
    }

    return 0;
}

/**
 * @brief Probe epwm channel device.
 *
 * Get epwm device instance with specified channel ID.
 *
 * @param[in] epwm epwm controller
 * @param[in] epwm_cfg epwm channel configure
 * @return dev pwm_dev_t type
 */
pwm_dev_t *sdrv_epwm_probe(sdrv_epwm_controller_t *epwm_ctrl,
                           const epwm_cfg_t *epwm_cfg)
{

    sdrv_epwm_channel_e chnl = epwm_cfg->chnl;
    sdrv_epwm_dev_t *epwm_dev = &epwm_ctrl->pwm[chnl];

    pwm_dev_t *dev = &epwm_dev->dev;
    epwm_dev->controller = epwm_ctrl;

    epwm_dev->id = chnl;
    epwm_dev->cmp_cfg = &epwm_cfg->cmp_cfg;
    dev->clk_div = epwm_ctrl->clk_div;

    dev->ops = &sdrv_epwm_ops;
    dev->priv = epwm_dev;

    return dev;
}

