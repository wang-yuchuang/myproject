/**
 * @file: sdrv_epwm_drv.h
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * @Descripttion:
 * @Revision History:
 * ----------------
 */
#ifndef INCLUDE_DRV_SDRV_EPWM_H
#define INCLUDE_DRV_SDRV_EPWM_H

#include <compiler.h>
#include <spinlock.h>
#include <types.h>

#include "driver/pwm.h"
#include "sdrv_epwm.h"
#ifndef ASSEMBLY

__BEGIN_CDECLS

#ifdef CONFIG_SDRV_EPWM_L
#define EPWM_SOURCE_CLK 200
#else
#define EPWM_SOURCE_CLK 250
#endif

#define EPWM_NS_TO_CNTVAL(NS, DIV)                                             \
    (NS * (1.0 / (1000 * 1000000)) /                                           \
     (1.0 / (EPWM_SOURCE_CLK * 1000000) * (DIV + 1)))
#define EPWM_CNTVAL_TO_NS(CNT, DIV)                                            \
    (CNT * (1.0 / (EPWM_SOURCE_CLK * 1000000) * (DIV + 1)) /                   \
     (1.0 / (1000 * 1000000)))
#define EPWM_NS_TO_VAL_1(NS, DIV) (EPWM_NS_TO_CNTVAL(NS, DIV) - 1)

#define EPWM_FIFO_A_OFFSET (0xc0U)
#define EPWM_FIFO_B_OFFSET (0xc4U)
#define EPWM_FIFO_C_OFFSET (0xc8U)
#define EPWM_FIFO_D_OFFSET (0xccU)
#define EPWM_CNT_G0_OVF_OFFSET (0x104U)
#define EPWM_CNT_G1_OVF_OFFSET (0x124U)
#define EPWM_INT_STA_EN (0x3f << 0 | 0xf << 20)
#define EPWM_COR_ERR_INT_STA_EN (0xf << 0 | 0xffff << 8)
#define EPWM_UNC_ERR_INT_STA_EN (0xf << 0 | 0xffff << 8)

#define EPWM_BM_INT_EN_CHN_D_DMA_REQ_SHIFT (23U)
#define EPWM_BM_INT_EN_CHN_C_DMA_REQ_SHIFT (22U)
#define EPWM_BM_INT_EN_CHN_B_DMA_REQ_SHIFT (21U)
#define EPWM_BM_INT_EN_CHN_A_DMA_REQ_SHIFT (20U)
#define EPWM_BM_INT_EN_CNT_G1_OVF_SHIFT (5U)
#define EPWM_BM_INT_EN_CNT_G0_OVF_SHIFT (4U)
#define EPWM_BM_INT_EN_CMP_D_SHIFT (3U)
#define EPWM_BM_INT_EN_CMP_C_SHIFT (2U)
#define EPWM_BM_INT_EN_CMP_B_SHIFT (1U)
#define EPWM_BM_INT_EN_CMP_A_SHIFT (0U)

#define EPWM_BM_ERR_INT_STA_CMP_D_REG_UPD_TWICE_SHIFT (23U)
#define EPWM_BM_ERR_INT_STA_CMP_C_REG_UPD_TWICE_SHIFT (22U)
#define EPWM_BM_ERR_INT_STA_CMP_B_REG_UPD_TWICE_SHIFT (21U)
#define EPWM_BM_ERR_INT_STA_CMP_A_REG_UPD_TWICE_SHIFT (20U)
#define EPWM_BM_ERR_INT_STA_CMP_D_REG_NO_UPD_SHIFT (19U)
#define EPWM_BM_ERR_INT_STA_CMP_C_REG_NO_UPD_SHIFT (18U)
#define EPWM_BM_ERR_INT_STA_CMP_B_REG_NO_UPD_SHIFT (17U)
#define EPWM_BM_ERR_INT_STA_CMP_A_REG_NO_UPD_SHIFT (16U)
#define EPWM_BM_ERR_INT_STA_CMP_D_FAULT_EVENT_SHIFT (15U)
#define EPWM_BM_ERR_INT_STA_CMP_C_FAULT_EVENT_SHIFT (14U)
#define EPWM_BM_ERR_INT_STA_CMP_B_FAULT_EVENT_SHIFT (13U)
#define EPWM_BM_ERR_INT_STA_CMP_A_FAULT_EVENT_SHIFT (12U)
#define EPWM_BM_ERR_INT_STA_CMP_D0_FAULT_SHIFT (11U)
#define EPWM_BM_ERR_INT_STA_CMP_C0_FAULT_SHIFT (10U)
#define EPWM_BM_ERR_INT_STA_CMP_B0_FAULT_SHIFT (9U)
#define EPWM_BM_ERR_INT_STA_CMP_A0_FAULT_SHIFT (8U)
#define EPWM_BM_ERR_INT_STA_FIFO_D_UNDERRUN_SHIFT (3U)
#define EPWM_BM_ERR_INT_STA_FIFO_C_UNDERRUN_SHIFT (2U)
#define EPWM_BM_ERR_INT_STA_FIFO_B_UNDERRUN_SHIFT (1U)
#define EPWM_BM_ERR_INT_STA_FIFO_A_UNDERRUN_SHIFT (0U)

#define EPWM_BM_CHN_D_DMA_REQ (1 << EPWM_BM_INT_EN_CHN_D_DMA_REQ_SHIFT)
#define EPWM_BM_CHN_C_DMA_REQ (1 << EPWM_BM_INT_EN_CHN_C_DMA_REQ_SHIFT)
#define EPWM_BM_CHN_B_DMA_REQ (1 << EPWM_BM_INT_EN_CHN_B_DMA_REQ_SHIFT)
#define EPWM_BM_CHN_A_DMA_REQ (1 << EPWM_BM_INT_EN_CHN_A_DMA_REQ_SHIFT)
#define EPWM_BM_CNT_G0_OVF (1 << EPWM_BM_INT_EN_CNT_G0_OVF_SHIFT)
#define EPWM_BM_CNT_G1_OVF (1 << EPWM_BM_INT_EN_CNT_G1_OVF_SHIFT)
#define EPWM_BM_CMP_D (1 << EPWM_BM_INT_EN_CMP_D_SHIFT)
#define EPWM_BM_CMP_C (1 << EPWM_BM_INT_EN_CMP_C_SHIFT)
#define EPWM_BM_CMP_B (1 << EPWM_BM_INT_EN_CMP_B_SHIFT)
#define EPWM_BM_CMP_A (1 << EPWM_BM_INT_EN_CMP_A_SHIFT)

#define EPWM_BM_CMP_D_REG_UPD_TWICE                                            \
    (1 << EPWM_BM_ERR_INT_STA_CMP_D_REG_UPD_TWICE_SHIFT)
#define EPWM_BM_CMP_C_REG_UPD_TWICE                                            \
    (1 << EPWM_BM_ERR_INT_STA_CMP_C_REG_UPD_TWICE_SHIFT)
#define EPWM_BM_CMP_B_REG_UPD_TWICE                                            \
    (1 << EPWM_BM_ERR_INT_STA_CMP_B_REG_UPD_TWICE_SHIFT)
#define EPWM_BM_CMP_A_REG_UPD_TWICE                                            \
    (1 << EPWM_BM_ERR_INT_STA_CMP_A_REG_UPD_TWICE_SHIFT)
#define EPWM_BM_CMP_D_REG_NO_UPD                                               \
    (1 << EPWM_BM_ERR_INT_STA_CMP_D_REG_NO_UPD_SHIFT)
#define EPWM_BM_CMP_C_REG_NO_UPD                                               \
    (1 << EPWM_BM_ERR_INT_STA_CMP_C_REG_NO_UPD_SHIFT)
#define EPWM_BM_CMP_B_REG_NO_UPD                                               \
    (1 << EPWM_BM_ERR_INT_STA_CMP_B_REG_NO_UPD_SHIFT)
#define EPWM_BM_CMP_A_REG_NO_UPD                                               \
    (1 << EPWM_BM_ERR_INT_STA_CMP_A_REG_NO_UPD_SHIFT)
#define EPWM_BM_CMP_D_FAULT_EVENT                                              \
    (1 << EPWM_BM_ERR_INT_STA_CMP_D_FAULT_EVENT_SHIFT)
#define EPWM_BM_CMP_C_FAULT_EVENT                                              \
    (1 << EPWM_BM_ERR_INT_STA_CMP_C_FAULT_EVENT_SHIFT)
#define EPWM_BM_CMP_B_FAULT_EVENT                                              \
    (1 << EPWM_BM_ERR_INT_STA_CMP_B_FAULT_EVENT_SHIFT)
#define EPWM_BM_CMP_A_FAULT_EVENT                                              \
    (1 << EPWM_BM_ERR_INT_STA_CMP_A_FAULT_EVENT_SHIFT)
#define EPWM_BM_CMP_D0_FAULT (1 << EPWM_BM_ERR_INT_STA_CMP_D0_FAULT_SHIFT)
#define EPWM_BM_CMP_C0_FAULT (1 << EPWM_BM_ERR_INT_STA_CMP_C0_FAULT_SHIFT)
#define EPWM_BM_CMP_B0_FAULT (1 << EPWM_BM_ERR_INT_STA_CMP_B0_FAULT_SHIFT)
#define EPWM_BM_CMP_A0_FAULT (1 << EPWM_BM_ERR_INT_STA_CMP_A0_FAULT_SHIFT)
#define EPWM_BM_FIFO_D_UNDERRUN (1 << EPWM_BM_ERR_INT_STA_FIFO_D_UNDERRUN_SHIFT)
#define EPWM_BM_FIFO_C_UNDERRUN (1 << EPWM_BM_ERR_INT_STA_FIFO_C_UNDERRUN_SHIFT)
#define EPWM_BM_FIFO_B_UNDERRUN (1 << EPWM_BM_ERR_INT_STA_FIFO_B_UNDERRUN_SHIFT)
#define EPWM_BM_FIFO_A_UNDERRUN (1 << EPWM_BM_ERR_INT_STA_FIFO_A_UNDERRUN_SHIFT)

#ifdef CONFIG_SDRV_EPWM_L
#define EPWM_BM_CENTER_ALIGN_CMP_INT_STA_CMP_D_CMP11 (0x01U << 15U)
#define EPWM_BM_CENTER_ALIGN_CMP_INT_STA_CMP_D_CMP10 (0x01U << 14U)
#define EPWM_BM_CENTER_ALIGN_CMP_INT_STA_CMP_D_CMP01 (0x01U << 13U)
#define EPWM_BM_CENTER_ALIGN_CMP_INT_STA_CMP_D_CMP00 (0x01U << 12U)
#define EPWM_BM_CENTER_ALIGN_CMP_INT_STA_CMP_C_CMP11 (0x01U << 11U)
#define EPWM_BM_CENTER_ALIGN_CMP_INT_STA_CMP_C_CMP10 (0x01U << 10U)
#define EPWM_BM_CENTER_ALIGN_CMP_INT_STA_CMP_C_CMP01 (0x01U << 9U)
#define EPWM_BM_CENTER_ALIGN_CMP_INT_STA_CMP_C_CMP00 (0x01U << 8U)
#define EPWM_BM_CENTER_ALIGN_CMP_INT_STA_CMP_B_CMP11 (0x01U << 7U)
#define EPWM_BM_CENTER_ALIGN_CMP_INT_STA_CMP_B_CMP10 (0x01U << 6U)
#define EPWM_BM_CENTER_ALIGN_CMP_INT_STA_CMP_B_CMP01 (0x01U << 5U)
#define EPWM_BM_CENTER_ALIGN_CMP_INT_STA_CMP_B_CMP00 (0x01U << 4U)
#define EPWM_BM_CENTER_ALIGN_CMP_INT_STA_CMP_A_CMP11 (0x01U << 3U)
#define EPWM_BM_CENTER_ALIGN_CMP_INT_STA_CMP_A_CMP10 (0x01U << 2U)
#define EPWM_BM_CENTER_ALIGN_CMP_INT_STA_CMP_A_CMP01 (0x01U << 1U)
#define EPWM_BM_CENTER_ALIGN_CMP_INT_STA_CMP_A_CMP00 (0x01U << 0U)
#endif
/**
 * @brief epwm_cmp_cfg_con_mode.
 */
typedef enum epwm_cmp_cfg_con_mode {
    EPWM_CONSECUTIVE_MODE,
    EPWM_ONLY_EXECUTE_ONE_TIME,
} epwm_cmp_cfg_con_mode_e;

/**
 * @brief epwm_cmp_out_mode.
 */
typedef enum epwm_cmp_out_mode {
    EPWM_ONLY_COMP0_OUTPUT,
    EPWM_BOTH_COMP0_AND_COMP1_OUTPUT,
} epwm_cmp_out_mode_e;

/**
 * @brief epwm_cfg_ctrl_dma_trig_sel.
 */
typedef enum epwm_cfg_ctrl_dma_trig_sel {
    EPWM_CMP_A_CE = 1 << 0,
    EPWM_CMP_B_CE = 1 << 1,
    EPWM_CMP_C_CE = 1 << 2,
    EPWM_CMP_D_CE = 1 << 3,
    EPWM_CNT_G0_CE = 1 << 4,
    EPWM_CNT_G1_CE = 1 << 5,
} epwm_cfg_ctrl_dma_trig_sel_t;

/**
 * @brief epwm_cmpxx_val NS.
 */
typedef struct epwm_multi_cmp_val {
    uint32_t cmp00_val;
    uint32_t cmp01_val;
    uint32_t cmp10_val;
    uint32_t cmp11_val;
} epwm_multi_cmp_val_t;

/**
 * @brief epwm_eid_val.
 */
typedef struct epwm_eid_val {
    uint8_t eid00;
    uint8_t eid01;
    uint8_t eid10;
    uint8_t eid11;
} epwm_eid_val_t;

/**
 * @brief epwm_dual_duty_state.
 */
typedef struct epwm_dual_duty_state {
    uint32_t left_point;
    uint32_t right_point;
} epwm_dual_duty_state_t;

/**
 * @brief epwm_fs_sta.
 */
typedef struct epwm_fs_state {
    bool cmp0_fs_state;
    bool cmp1_fs_state;
} epwm_fs_state_t;

/**
 * @brief epwm_dma_cfg.
 */
typedef struct epwm_dma_cfg {
    bool dma_enable;
    uint8_t fifo_wml;
} epwm_dma_cfg_t;

/**
 * @brief epwm_cmp_cfg.
 */
typedef struct epwm_cmp_cfg {
    sdrv_epwm_cnt_gx_e cmp_cnt_sel;
    epwm_cmp_out_mode_e chnl_out_mode;
    epwm_cmp_cfg_con_mode_e con_mode;
    sdrv_epwm_cmp_mode_e cmp_mode;
    sdrv_epwm_cmp_event_out_t out_mode;
    sdrv_epwm_cmp_pulse_wid0_t cmp_pulse_wid0;
    sdrv_epwm_cmp_pulse_wid1_t cmp_pulse_wid1;
    uint8_t refresh_intval;
    bool audio_enable;
    epwm_dma_cfg_t dma_cfg;
    bool sw_rld_mode;
    bool center_align_mode;
} epwm_cmp_cfg_t;

/**
 * @brief epwm_cfg.
 */
typedef struct epwm_cfg {
    epwm_cmp_cfg_t cmp_cfg;
    sdrv_epwm_channel_e chnl;
} epwm_cfg_t;

struct sdrv_epwm_controller;

/**
 * @brief ePWM channel device.
 *
 * Each PWM device maps to one ePWM channel in the controller.
 */
typedef struct sdrv_epwm_dev {
    /* Base PWM device structure. */
    pwm_dev_t dev;

    /* ePWM specific parameters. */
    struct sdrv_epwm_controller *controller;
    sdrv_epwm_channel_e id;
    const epwm_cmp_cfg_t *cmp_cfg;
    epwm_dual_duty_state_t pre_duty;
} sdrv_epwm_dev_t;

/**
 * @brief ePWM hardware controller.
 */
typedef struct sdrv_epwm_controller {
    paddr_t base;
    uint8_t irq;
    uint32_t irq_id;
    uint16_t clk_div;

    /* ePWM channels. */
    sdrv_epwm_dev_t pwm[SDRV_EPWM_CHANNEL_NR];
} sdrv_epwm_controller_t;

/**
 * @brief Taishan ePWM controller configuration.
 */
typedef struct sdrv_epwm_cfg {
    paddr_t base;
    uint8_t irq;
    uint32_t irq_id;
    sdrv_epwm_clk_config_src_clk_sel_e clk_src;
    uint16_t clk_div;
} sdrv_epwm_cfg_t;

/**
 * @brief configure multi compare mode
 *
 * @param[in] dev pwm common instance
 * @param[in] state pwm duty and period
 * @param[in] multi cmp00/01/10/11 val
 *
 */
void sdrv_epwm_multi_cmp_mode(pwm_dev_t *dev, const pwm_state_t *state,
                              epwm_multi_cmp_val_t *multi_cmp);

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
                           sdrv_epwm_input_sel_trig_e clr_sel);

/**
 * @brief configure epwm counter overflow upload immediately
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
                               sdrv_epwm_input_sel_trig_e set_sel);

/**
 * @brief software trigger
 *
 * @param[in] dev pwm common instance
 * @param[in] tirg sellect trigger mode
 *
 */
void sdrv_epwm_sw_trig(pwm_dev_t *dev, sdrv_epwm_sw_trig_ctrl_e trig);

/**
 * @brief configure epwm death time
 *
 * @param[in] dev pwm common instance
 * @param[in] prefin_pol prefinial polarity
 * @param[in] dti_val death time value
 *
 */
void sdrv_epwm_dti(pwm_dev_t *dev, sdrv_epwm_cmp_prefin_pol_t *prefin_pol,
                   const uint32_t dti_val);

/**
 * @brief configure sse epwm output
 *
 * @param[in] dev pwm common instance
 * @param[in] sse_cfg sse_mode and edge sellect
 * @param[in] sse_reg_val sse register value
 *
 */
void sdrv_epwm_sse(pwm_dev_t *dev, sdrv_epwm_cmp_sse_ctrl_t *sse_cfg,
                   uint32_t sse_reg_val);

/**
 * @brief configure modulation frequency control
 *
 * @param[in] dev pwm common instance
 * @param[in] val mfc up times to 2^val times
 *
 */
void sdrv_epwm_mfc(pwm_dev_t *dev, uint8_t val);

/**
 * @brief configure compare data format
 *
 * @param[in] dev pwm common instance
 * @param[in] dat_format cmp data formate
 */
void sdrv_epwm_cmp_data_format(
    pwm_dev_t *dev, sdrv_epwm_chn_dma_ctrl_cmp_x_dat_format_e dat_format);

/**
 * @brief configure compare dither mode
 *
 * @param[in] dev pwm common instance
 * @param[in] clip_rslt dither clip_rslt val
 * @param[in] init_offset dither init_offset val
 */
void sdrv_epwm_cmp_dither(pwm_dev_t *dev, uint8_t clip_rslt,
                          uint16_t init_offset);

/**
 * @brief configure compare multi channels
 *
 * @param[in] dev pwm common instance
 * @param[in] full_en full channels enable
 */
void sdrv_epwm_cmp_multi_chnl(pwm_dev_t *dev, bool full_en);

/**
 * @brief configure compare event/cnt overflow event
 *
 * @param[in] dev pwm common instance
 */
void sdrv_epwm_ce_ctrl(pwm_dev_t *dev, bool src_sel);

/**
 * @brief configure dma request except fifo wml trig
 *
 * @param[in] dev pwm common instance
 * @param[in] sel_trig sellect trigger source
 */
void sdrv_epwm_ce_dma_trig(pwm_dev_t *dev, uint8_t sel_trig);

/**
 * @brief filter input fault source signals
 *
 * @param[in] dev pwm common instance
 * @param[in] flt_cfg config filter
 */
void sdrv_epwm_fault_flt(pwm_dev_t *dev, sdrv_epwm_fault_flt_t *flt_cfg);

/**
 * @brief configure input fault event
 *
 * @param[in] dev pwm common instance
 * @param[in] fault_cfg config fault event
 * @param[in] sub_chnl config which fault source to trigger
 */
void sdrv_epwm_fault_event(pwm_dev_t *dev, uint8_t sub_chnl,
                           sdrv_epwm_cmp_fault_event_ctrl_t *fault_cfg,
                           epwm_fs_state_t *fs_state);

/**
 * @brief input fault event clr
 *
 * @param[in] dev pwm common instance
 * @param[in] sub_chnl config which fault source to trigger
 */
void sdrv_epwm_fault_event_clr(pwm_dev_t *dev, uint8_t sub_chnl);

/**
 * @brief compare value upload
 *
 * @param[in] dev pwm common instance
 * @param[in] state pwm duty and period
 */
void sdrv_epwm_cmp_val_upd(pwm_dev_t *dev, const pwm_state_t *state);

/**
 * @brief cnt overflow value upload
 *
 * @param[in] dev pwm common instance
 * @param[in] state pwm duty and period
 */
void sdrv_epwm_cnt_ovf_upd(pwm_dev_t *dev, uint32_t period);

/**
 * @brief cnt overflow value upload directly (unit is cnt val)
 *
 * @param[in] dev pwm common instance
 * @param[in] period_cnt counter overflow value
 */
void sdrv_epwm_cnt_ovf_dir_upd(pwm_dev_t *dev, uint32_t period_cnt);

/**
 * @brief upload chnl_b/c/d compare value with dual mode 100% duty check(unit is
 * cnt val)
 *
 * @param[in] dev pwm common instance
 * @param[in] left_point dual mode left cmp_val
 * @param[in] right_point dual mode right cmp_val
 */
void sdrv_epwm_cmp_bcd_update(pwm_dev_t *dev, uint32_t left_point,
                              uint32_t right_point);

/**
 * @brief compare value chnl_b/c/d upload (unit is cnt val)
 *
 * @param[in] dev pwm common instance
 * @param[in] left_point dual mode left cmp_val
 * @param[in] right_point dual mode right cmp_val
 */
void sdrv_epwm_cmp_val_upd_bcd(pwm_dev_t *dev, uint32_t left_point,
                               uint32_t right_point);

/**
 * @brief compare value chnl_a upload (unit is cnt val)
 *
 * @param[in] dev pwm common instance
 * @param[in] trig_point set trig point to tmux
 */
void sdrv_epwm_cmp_val_upd_a(pwm_dev_t *dev, uint32_t trig_point);

/**
 * @brief compare software reload
 *
 * @param[in] dev pwm common instance
 */
void sdrv_epwm_cmp_sw_rld(pwm_dev_t *dev);

/**
 * @brief compare clr/set configure
 * @param[in] dev pwm common instance
 * @param[in] input_sel input sellect
 * @param[in] set_trig cmp set trigger sel
 * @param[in] clr_trig cmp clr trigger sel
 */
void sdrv_epwm_cmp_input(pwm_dev_t *dev, sdrv_epwm_input_sel_t *input_sel,
                         uint8_t set_trig, uint8_t clr_trig);

/**
 * @brief configure compare a event id
 *
 * @param[in] dev pwm common instance
 * @param[in] eid_val cmp00/01/10/11 eid
 */
void sdrv_epwm_cmp_a_eid(pwm_dev_t *dev, epwm_eid_val_t *eid_val);

/**
 * @brief compare channel enable.
 *
 * @param[in] dev pwm common instance
 *
 */
void sdrv_epwm_cmp_en(pwm_dev_t *dev, bool en);

/**
 * @brief counter enable.
 *
 * @param[in] dev pwm common instance
 *
 */
void sdrv_epwm_cnt_en(pwm_dev_t *dev, bool en);

/**
 * @brief epwm initial status.
 *
 * @param[in] dev pwm common instance
 * @param[in] cmp0_init cmp0 channel init status
 * @param[in] cmp1_init cmp1 channel init status
 *
 */
void sdrv_epwm_init_sta(pwm_dev_t *dev, bool cmp0_init, bool cmp1_init);

#ifdef CONFIG_SDRV_EPWM_L
/**
 * @brief center align mode enable.
 *
 * @param[in] dev pwm common instance
 * @param[in] en enable/diable center align mode
 *
 */
void sdrv_epwm_center_align_mode_en(pwm_dev_t *dev, bool en);

/**
 * @brief compare value chnl_b/c/d upload in center align mode (unit is cnt val)
 *
 * @param[in] dev pwm common instance
 * @param[in] cmp_point center align mode cmp_val
 */
void sdrv_epwm_cmp_center_align_val_upd_bcd(pwm_dev_t *dev, uint32_t cmp_point);
#endif

/**
 * @brief epwm controller initial
 *
 * @param[in] epwm_ctrl  epwm controller instance
 * @param[in] cfg epwm controller configure
 *
 */
int sdrv_epwm_controller_init(sdrv_epwm_controller_t *epwm_ctrl,
                              const sdrv_epwm_cfg_t *cfg);

/**
 * @brief Probe epwm channel device.
 *
 * Get epwm device instance with specified channel ID.
 *
 * @param[in] epwm epwm controller
 * @param[in] epwm_cfg epwm channel configure
 */
pwm_dev_t *sdrv_epwm_probe(sdrv_epwm_controller_t *epwm_ctrl,
                           const epwm_cfg_t *epwm_cfg);

__END_CDECLS

#endif

#endif
