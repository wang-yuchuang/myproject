/**
 * @file: etimer_pwm_drv.h
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * @Descripttion:
 * @Revision History:
 * ----------------
 */
#ifndef INCLUDE_DRV_ETMR_PWM_H
#define INCLUDE_DRV_ETMR_PWM_H

#include <compiler.h>
#include <spinlock.h>
#include <types.h>
#include <sdrv_etimer.h>
#include "driver/pwm.h"
#ifndef ASSEMBLY

__BEGIN_CDECLS

#define ETMR_NS_TO_VAL(NS, DIV) \
    (NS * 1.0 / (1000 * 1000000)) / (1.0 / (400 * 1000000) * (DIV + 1))
#define ETMR_NS_TO_VAL_1(NS, DIV) \
    (NS * 1.0 / (1000 * 1000000)) / (1.0 / (400 * 1000000) * (DIV + 1)) - 1

#define ETMR_VAL_TO_NS_1(VAL, DIV) \
    (((VAL + 1) * 1.0) * (DIV + 1) * (1000)) / (250)

#define TOTAL_CHN_NUM_PER_ETMR 4

/**
 * @brief etimer_cmpxx_val NS.
 */
typedef struct etimer_multi_cmp_val
{
    uint32_t cmp00_val;
    uint32_t cmp01_val;
    uint32_t cmp10_val;
    uint32_t cmp11_val;
} etimer_multi_cmp_val_t;

/**
 * @brief etimer_event_trigger_mode .
 */
typedef enum etimer_event_trigger_mode
{
    ETMR_EVENT_IGNORED,
    ETMR_POSITIVE_EDGE,
    ETMR_NEGATIVE_EDGE,
    ETMR_LEVEL_HIGH,
} etimer_event_trigger_mode_t;

/**
 * @brief etimer_input_sel_trig.
 */
typedef enum etimer_input_sel_trig
{
    ETMR_INPUT_SEL_TRIG_EXT_SET,
    ETMR_INPUT_SEL_TRIG_EXT_CLR,
    ETMR_INPUT_SEL_TRIG_CPT_A_CE,
    ETMR_INPUT_SEL_TRIG_CPT_B_CE,
    ETMR_INPUT_SEL_TRIG_CPT_C_CE,
    ETMR_INPUT_SEL_TRIG_CPT_D_CE,
    ETMR_INPUT_SEL_TRIG_CNT_G0_CE,
    ETMR_INPUT_SEL_TRIG_CNT_G1_CE,
    ETMR_INPUT_SEL_TRIG_LEVEL_LOW = 9,
    ETMR_INPUT_SEL_TRIG_LEVEL_HIGH,
    ETMR_INPUT_SEL_TRIG_SW_PROGRAM_PULSE,
    ETMR_INPUT_SEL_TRIG_SW_TRIG0,
    ETMR_INPUT_SEL_TRIG_SW_TRIG1,
    ETMR_INPUT_SEL_TRIG_SW_TRIG2,
    ETMR_INPUT_SEL_TRIG_SW_TRIG3,
} etimer_input_sel_trig_t;

/**
 * @brief etimer_cmp_cfg.
 */
typedef struct etimer_cmp_cfg
{
    // common
    sdrv_etimer_cmp_out_e chnl_out_mode;
    sdrv_etimer_cmp_con_mode_e con_mode;
    sdrv_etimer_cmp_mode_e cmp_mode;
    // extern
    etimer_event_trigger_mode_t cnt_clr_trig_sel;
    etimer_input_sel_trig_t clr_sel;
} etimer_cmp_cfg_t;

/**
 * @brief etimer_edge_sel  .
 */
typedef enum etimer_edge_sel
{
    ETMR_EDGE_SEL_RISING_EDGE,
    ETMR_EDGE_SEL_FAILING_EDGE,
    ETMR_EDGE_SEL_BOTH_EDGE,
    ETMR_EDGE_SEL_BOTH_EDGE_2,
} etimer_edge_sel_t;
/**
 * @brief etimer_cmp_sse_ctrl.
 */
typedef struct etimer_cmp_sse_ctrl
{
    uint32_t sse_mode;
    uint32_t sse_edge;
} etimer_sse_ctrl_t;

/**
 * @brief etimer_cmp_fault_ctrl.
 */
typedef struct
{
    sdrv_etimer_cmp_fault_e fault_idx;
    cmp_fault_ctrl_t fault_ctrl;
} etimer_pwm_fault_ctrl_t;

/**
 * @brief etimer_dma.
 */
typedef struct
{
    uint16_t wml;
    uint32_t chn_sig_mask;
    sdrv_etimer_bit_mode_e bit_mode; /* compare mode only */
} etimer_pwm_dma_t;

/**
 * @brief etimer_cfg.
 */
typedef struct etimer_cfg
{
    sdrv_etimer_chn_id_e cpt_chn;
    sdrv_etimer_chn_id_e cmp_chn;
    etimer_cmp_cfg_t cmp_cfg;
} etimer_cfg_t;

struct etimer_pwm_controller;

/**
 * @brief eTMR channel device.
 *
 * Each PWM device maps to one eTMR channel in the controller.
 */
typedef struct etimer_pwm_dev
{
    /* Base PWM device structure. */
    pwm_dev_t dev;

    /* eTMR specific parameters. */
    struct etimer_pwm_controller *controller;
    sdrv_etimer_chn_id_e cpt_chn;
    sdrv_etimer_chn_id_e cmp_chn;
    etimer_cmp_cfg_t *cmp_cfg;
} etimer_pwm_dev_t;

/**
 * @brief eTMR hardware controller.
 */
typedef struct etimer_pwm_controller
{
    uint32_t base;
    uint8_t irq;
    uint32_t irq_id;
    uint16_t clk_div;
    /* eTMR channels. */
    etimer_pwm_dev_t pwm[TOTAL_CHN_NUM_PER_ETMR];
} etimer_pwm_controller_t;

/**
 * @brief Taishan eTMR controller configuration.
 */
typedef struct etimer_pwm_cfg
{
    uint32_t base;
    uint8_t irq;
    uint32_t irq_id;
    sdrv_etimer_clk_src_e clk_src;
    uint16_t clk_div;
} etimer_pwm_cfg_t;

/**
 * @brief configure multi compare mode
 *
 * @param[in] dev pwm common instance
 * @param[in] state pwm duty and periods
 * @param[in] multi cmp00/01/10/11 val
 *
 */
void etimer_pwm_multi_cmp_mode(pwm_dev_t *dev, const pwm_state_t *state,
                               etimer_multi_cmp_val_t multi_cmp);

/**
 * @brief configure etimer death time
 *
 * @param[in] dev pwm common instance
 * @param[in] dti_val death time value
 *
 */
void etimer_pwm_dti(pwm_dev_t *dev, const uint32_t dti_val);

/**
 * @brief software trigger
 *
 * @param[in] dev pwm common instance
 * @param[in] trig_sel sellect trigger index
 * @param[in] trig_type sellect trigger type
 *
 */
void etimer_pwm_sw_trig(pwm_dev_t *dev, sdrv_etimer_sw_trig_sel_e trig_sel,
                        sdrv_etimer_sw_trig_typr_e trig_type);


/**
 * @brief configure etimer death time
 *
 * @param[in] dev pwm common instance
 * @param[in] fault fault_cfg
 *
 */
void etimer_pwm_fault_ctrl(pwm_dev_t *dev, etimer_pwm_fault_ctrl_t *fault);

/**
 * @brief configure etimer filter config
 *
 * @param[in] dev pwm common instance
 * @param[in] filter_cfg filter_cfg
 *
 */
void etimer_pwm_filter(pwm_dev_t *dev, flt_config_t *filter_cfg);

/**
 * @brief configure sse etimer output
 *
 * @param[in] dev pwm common instance
 * @param[in] sse_cfg sse_mode and edge sellect
 * @param[in] sse_reg_val sse register value
 *
 */
void etimer_pwm_cmp_sse(pwm_dev_t *dev, const sdrv_etimer_sse_ctrl_t sse_cfg,
                        const uint32_t sse_reg_val);

/**
 * @brief configure sse etimer input
 *
 * @param[in] dev pwm common instance
 * @param[in] sse_cfg sse_mode and edge sellect
 * @param[in] sse_reg_val sse register value
 *
 */
void etimer_pwm_cpt_sse(pwm_dev_t *dev, const sdrv_etimer_sse_ctrl_t sse_cfg,
                        const uint32_t sse_reg_val);

/**
 * @brief configure quad etimer input
 *
 * @param[in] dev pwm common instance
 * @param[in] quad_ctrl quad config
 *
 */
void etimer_pwm_cpt_quad(pwm_dev_t *dev, quad_ctrl_t *quad_ctrl);

/**
 * @brief configure etimer cpt dma etimer
 *
 * @param[in] dev pwm common instance
 * @param[in] dma_cfg dma config
 *
 */
void etimer_pwm_cpt_dma(pwm_dev_t *dev, etimer_pwm_dma_t *dma_cfg);

/**
 * @brief configure etimer cmp dma etimer
 *
 * @param[in] dev pwm common instance
 * @param[in] dma_cfg dma config
 *
 */
void etimer_pwm_cmp_dma(pwm_dev_t *dev, etimer_pwm_dma_t *dma_cfg);

/**
 * @brief compare clr/set configure
 * @param[in] dev pwm common instance
 * @param[in] input_sel input sellect
 * @param[in] trig_val cmp set trigger sel
 * @param[in] trig_mode cmp trigger mode sel
 */
void etimer_pwm_cmp_input(pwm_dev_t *dev, sdrv_etimer_trig_sel_e input_sel,
                          sdrv_etimer_trig_set_clr_e trig_val, sdrv_etimer_cmp_event_trig_e trig_mode);

/**
 * @brief lcnt init val set
 * @param[in] dev pwm common instance
 * @param[in] init_val cnt init value
 */
void etimer_pwm_lcnt_init_val_set(pwm_dev_t *dev, uint32_t init_val);

/**
 * @brief capture clr/set configure
 * @param[in] dev pwm common instance
 * @param[in] input_sel input sellect
 * @param[in] trig_val capture set trigger sel
 * @param[in] trig_mode lcnt trigger mode sel
 */
void etimer_pwm_cpt_input(pwm_dev_t *dev, sdrv_etimer_trig_sel_e input_sel,
                          uint16_t trig_val, sdrv_etimer_cmp_event_trig_e trig_mode, 
                          sdrv_etimer_set_upd_sel_e set_upd);

/**
 * @brief capture clr/set configure
 * @param[in] dev pwm common instance
 * @param[in] input_sel input sellect
 * @param[in] trig_val capture set trigger sel
 * @param[in] trig_mode lcnt trigger mode sel
 */
void etimer_pwm_ext_output(pwm_dev_t *dev, sdrv_etimer_trig_sel_e output_sel,
                           uint16_t trig_val);

/**
 * @brief get signal status
 * @param[in] dev pwm common instance
 */
uint32_t etimer_pwm_get_signal_status(pwm_dev_t *dev);

/**
 * @brief get cpt_a dir signal status
 * @param[in] dev pwm common instance
 */
bool etimer_pwm_get_cpt_a_dir_up(pwm_dev_t *dev);

/**
 * @brief get current cnt value
 * @param[in] dev pwm common instance
 * @param[in] cnt_x cnt id
 */
uint32_t etimer_pwm_get_cnt_value(pwm_dev_t *dev, sdrv_etimer_cnt_id_e cnt_x);

/**
 * @brief set cmp eid
 * @param[in] dev pwm common instance
 * @param[in] cmp_event compare event
 * @param[in] val eid val
 */
void etimer_pwm_set_eid(pwm_dev_t *dev, sdrv_etimer_cmp_event_e cmp_event, uint32_t val);

/**
 * @brief etimer controller initital
 *
 * @param[in] etimer_ctrl  etimer controller instance
 * @param[in] cfg etimer controller configure
 *
 */
void etimer_pwm_controller_init(etimer_pwm_controller_t *etimer_ctrl,
                                const etimer_pwm_cfg_t *cfg);

/**
 * @brief Probe etimer channel device.
 *
 * Get etimer device instance with specified channel ID.
 *
 * @param[in] etimer etimer controller
 * @param[in] etimer_cfg etimer channel configure
 */
pwm_dev_t *etimer_pwm_probe(etimer_pwm_controller_t *etimer_ctrl,
                            const etimer_cfg_t *etimer_cfg);

__END_CDECLS

#endif

#endif
