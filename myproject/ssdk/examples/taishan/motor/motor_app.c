/*
 * @File: motor_app.c
 *
 * Copyright (c) 2022 Semidrive Semiconductor.
 * All rights reserved.
 *
 * @Descripttion:
 *
 * @Revision History:
 * ----------------
 */

#define LOG_TAG "motor_app.c "
#include <arch/cache.h>
#include <board.h>
#include <cmsis_os2.h>
#include <debug.h>
#include <irq_num.h>
#include <param.h>
#include <regs_base.h>
#include <stdlib.h>
#include <string.h>
#include <types.h>

#include "motor_app.h"

#include "CLI.h"
#include "reg.h"

#define ADC_CLK_NODE CLK_NODE(g_ckgen_ip_adc1)
#define DUTY_VAL_TOTAL 13
static pwm_state_t hf = {
    .period = PWM_PERIOD_CYCLES_NS,
    .duty = PWM_HALF_PERIOD_CYCLES_NS,
};

static pwm_state_t m1_trig_point = {
    .period = PWM_PERIOD_CYCLES_NS, .duty = 0, /*trig adc at cnt_zero*/
};

static pwm_state_t m1_trig_point2 = {
    .period = PWM_PERIOD_CYCLES_NS,
    .duty = PWM_HALF_PERIOD_CYCLES_NS,
};

static adc_config_t adc1_cfg = {
    .base = APB_ADC1_BASE,
    .resolution = 12,
    .inter_ref_volt = false,
    .sample_freq = 1.3 * 1000000,
    .is_diff = false,
    .clk = ADC_CLK_NODE,
};

static adc_config_t adc2_3_cfg = {
    .resolution = 12,
    .inter_ref_volt = false,
    .sample_freq = 1.3 * 1000000,
    .is_diff = false,
    .clk = ADC_CLK_NODE,
};

static xtrg_dev_t xtrg_dev_hfre = {
    .base = APB_XTRG_BASE,
    .irq = XTRG_FUNC_INTR_NUM,
};

#ifdef TEST_SW_TRIG_ADC
static adc_buff_t adc_buff[1];
static adc_buff_t adc_buff2[1];
#endif
static adc_dev_t dev;
extern gpio_dev_t g_gpio_safety;
extern gpio_dev_t g_gpio_ap;
static gpio_dev_t *gpio_dev;
static uint32_t pin = 101;
/*static uint32_t test1 = PWM_PERIOD_CYCLES / 8;
static uint32_t test2 = PWM_PERIOD_CYCLES / 8;
static uint32_t test3 = PWM_PERIOD_CYCLES / 8;*/
#define GPIO_INSTANCE(pin)                                                     \
    (((pin) < TAISHAN_SAFETY_PIN_NUM) ? &g_gpio_safety : &g_gpio_ap)

#ifdef TEST_DUTY_CHANGE_CYCLE_BY_CYCLE
static const uint32_t pwm_cnt_array[DUTY_VAL_TOTAL] = {
    62500, 62500, 56250, 50000, 43750, 37500, 31250,
    25000, 18750, 12500, 6250,  0,     0};
#endif

void cnt_gx_handle(void *arg)
{
    /*update pwm duty*/
    /*sdrv_epwm_cmp_val_upd_bcd(mc_chnl2, test1, PWM_PERIOD_CYCLES - test1);
    sdrv_epwm_cmp_val_upd_bcd(mc_chnl3, test2, PWM_PERIOD_CYCLES - test2);
    sdrv_epwm_cmp_val_upd_bcd(mc_chnl4, test3, PWM_PERIOD_CYCLES - test3);*/

#ifdef TEST_PERIOD_CHANGE
    /*update pwm period*/
    static uint16_t loop_cnt = 0;
    static bool flag = true;
    if (flag)
        loop_cnt++;
    if (loop_cnt >= 10) {
        sdrv_epwm_cmp_val_upd_bcd(mc_chnl2, PWM_PERIOD_CYCLES_8K / 4,
                                  PWM_PERIOD_CYCLES_8K -
                                      PWM_PERIOD_CYCLES_8K / 4);
        sdrv_epwm_cmp_val_upd_bcd(mc_chnl3, PWM_PERIOD_CYCLES_8K / 4,
                                  PWM_PERIOD_CYCLES_8K -
                                      PWM_PERIOD_CYCLES_8K / 4);
        sdrv_epwm_cmp_val_upd_bcd(mc_chnl4, PWM_PERIOD_CYCLES_8K / 4,
                                  PWM_PERIOD_CYCLES_8K -
                                      PWM_PERIOD_CYCLES_8K / 4);
        sdrv_epwm_cnt_ovf_dir_upd(mc_chnl2, PWM_PERIOD_CYCLES_8K);
        loop_cnt = 0;
        flag = false;
    }
#endif

#ifdef TEST_CHANGED_PERIOD_ADC_TRIG
    /*update pwm period*/
    static uint16_t loop_cnt = 0;
    static bool flag = true;
    if (flag)
        loop_cnt++;
    if (loop_cnt >= 10) {
        sdrv_epwm_cmp_val_upd_a(mc_chnl1, 0);
        sdrv_epwm_cmp_val_upd_bcd(mc_chnl2, PWM_PERIOD_CYCLES_8K / 4,
                                  PWM_PERIOD_CYCLES_8K -
                                      PWM_PERIOD_CYCLES_8K / 4);
        sdrv_epwm_cmp_val_upd_bcd(mc_chnl3, PWM_PERIOD_CYCLES_8K / 4,
                                  PWM_PERIOD_CYCLES_8K -
                                      PWM_PERIOD_CYCLES_8K / 4);
        sdrv_epwm_cmp_val_upd_bcd(mc_chnl4, PWM_PERIOD_CYCLES_8K / 4,
                                  PWM_PERIOD_CYCLES_8K -
                                      PWM_PERIOD_CYCLES_8K / 4);
        sdrv_epwm_cnt_ovf_dir_upd(mc_chnl2, PWM_PERIOD_CYCLES_8K);
        sdrv_epwm_cmp_val_upd_a(mc_adc_trig2, PWM_HALF_PERIOD_CYCLES_8K);
        sdrv_epwm_cnt_ovf_dir_upd(mc_adc_trig2, PWM_PERIOD_CYCLES_8K);
        loop_cnt = 0;
        flag = false;
    }
#endif

    /*after reload to shadow reg then update pwm duty immedaitely*/
    /*sdrv_epwm_cmp_sw_rld(mc_chnl2);
    sdrv_epwm_cmp_sw_rld(mc_chnl3);
    sdrv_epwm_cmp_sw_rld(mc_chnl4);*/

    /*after reload to shadow reg then update pwm period immedaitely*/
    /*sdrv_epwm_sw_trig(mc_chnl1, SDRV_EPWM_SW_TRIG_PULSE);*/
}

void mc_sem_cfg(void)
{
    sem_dev_t sem_dev = {
        .base = APB_SEM1_BASE,
    };

    sem_int_input_config_t input_cfg = {
        .int_out = SEM_OUTPUT_CPU,
        .int_in = SEM_EPWM1_COR_ERR_INT_NUM,
    };

    sdrv_sem_glb_err_inj_enable(&sem_dev, true);
    sdrv_sem_int_output_enable(&sem_dev, SEM_OUTPUT_CPU, true);
    sdrv_sem_int_input_enable(&sem_dev, &input_cfg, true);
}

int mc_breakin_handle(uint32_t irq, void *arg)
{
    uint32_t sta = sdrv_epwm_lld_cor_err_int_sta_get(APB_EPWM1_BASE);
    sdrv_epwm_lld_cor_err_int_sta_clr(APB_EPWM1_BASE, sta);

    sdrv_epwm_fault_event_clr(mc_chnl1, 1);
    sdrv_epwm_fault_event_clr(mc_chnl2, 1);
    sdrv_epwm_fault_event_clr(mc_chnl3, 1);
    sdrv_epwm_fault_event_clr(mc_chnl4, 1);

    return 0;
}

void pwm_init(void)
{
    sdrv_epwm_cmp_fault_event_ctrl_t fault_idle_cfg = {
        .fault_pol = PWM_IDLE_FAULT_POL_HIGH,
        .fault_clr_mode = 0,
    };

    epwm_eid_val_t eid_val = {
        .eid00 = PWM_EVENT_ID,
    };

    epwm_eid_val_t eid_val_2 = {
        .eid00 = PWM_EVENT_ID_2,
    };

    epwm_fs_state_t fs_sta = {
        .cmp0_fs_state = 0,
        .cmp1_fs_state = 0,
    };

    sdrv_epwm_cmp_prefin_pol_t prefin_pol = {
        .cmp0_pol = SDRV_EPWM_POL_KEEP,
        .cmp1_pol = SDRV_EPWM_POL_KEEP,
    };

    /*SET SYNC START SOURCE FOR M1 EPWM CNT*/
    sdrv_epwm_cnt_clr_cfg(mc_chnl1, SDRV_EPWM_LEVEL_HIGH,
                          SDRV_EPWM_INPUT_SEL_TRIG_EXT_CLR);
    /*ADC TRIGGER SOURCE*/
    pwm_config(mc_chnl1, &m1_trig_point);
    sdrv_epwm_cmp_a_eid(mc_chnl1, &eid_val);
    sdrv_epwm_cmp_en(mc_chnl1, true);
    /*PWM CHANNEL B*/
    pwm_config(mc_chnl2, &hf);
    sdrv_epwm_cnt_ovf_upd_cfg(mc_chnl2, SDRV_EPWM_ONLY_RELOAD_OVERFLOW_VALUE,
                              SDRV_EPWM_POSITIVE_EDGE,
                              SDRV_EPWM_INPUT_SEL_TRIG_SW_TRIG0);
    sdrv_epwm_dti(mc_chnl2, &prefin_pol, DTI_NS_VAL);
    /*CONFIG FAULT_EVENT*/
    sdrv_epwm_fault_event(mc_chnl2, PWM_IDLE_FAULT_SOURCE, &fault_idle_cfg,
                          &fs_sta);
    sdrv_epwm_init_sta(mc_chnl2, 1, 0);
    sdrv_epwm_cmp_en(mc_chnl2, true);
    /*PWM CHANNEL C*/
    pwm_config(mc_chnl3, &hf);
    sdrv_epwm_cnt_ovf_upd_cfg(mc_chnl3, SDRV_EPWM_ONLY_RELOAD_OVERFLOW_VALUE,
                              SDRV_EPWM_POSITIVE_EDGE,
                              SDRV_EPWM_INPUT_SEL_TRIG_SW_TRIG0);
    sdrv_epwm_dti(mc_chnl3, &prefin_pol, DTI_NS_VAL);
    sdrv_epwm_fault_event(mc_chnl3, PWM_IDLE_FAULT_SOURCE, &fault_idle_cfg,
                          &fs_sta);
    sdrv_epwm_init_sta(mc_chnl3, 1, 0);
    sdrv_epwm_cmp_en(mc_chnl3, true);
    /*PWM CHANNEL D*/
    pwm_config(mc_chnl4, &hf);
    sdrv_epwm_cnt_ovf_upd_cfg(mc_chnl4, SDRV_EPWM_ONLY_RELOAD_OVERFLOW_VALUE,
                              SDRV_EPWM_POSITIVE_EDGE,
                              SDRV_EPWM_INPUT_SEL_TRIG_SW_TRIG0);
    sdrv_epwm_dti(mc_chnl4, &prefin_pol, DTI_NS_VAL);
    sdrv_epwm_fault_event(mc_chnl4, PWM_IDLE_FAULT_SOURCE, &fault_idle_cfg,
                          &fs_sta);
    sdrv_epwm_init_sta(mc_chnl4, 1, 0);
    sdrv_epwm_cmp_en(mc_chnl4, true);

    /*SET SYNC START SOURCE FOR EPWM2 CNT*/
    sdrv_epwm_cnt_clr_cfg(mc_adc_trig2, SDRV_EPWM_LEVEL_HIGH,
                          SDRV_EPWM_INPUT_SEL_TRIG_EXT_CLR);
    /*ADC TRIGGER SOURCE_2*/
    pwm_config(mc_adc_trig2, &m1_trig_point2);
    sdrv_epwm_cmp_a_eid(mc_adc_trig2, &eid_val_2);
    sdrv_epwm_cmp_en(mc_adc_trig2, true);

    /*PWM OVF*/
    pwm_callback_t cnt_gx_cb = cnt_gx_handle;
    pwm_set_cb(mc_chnl1, cnt_gx_cb, NULL);
}

void pwm_breakin(void)
{
    sdrv_epwm_cmp_fault_event_ctrl_t fault_cfg = {
        .fault_pol = PWM_IDLE_FAULT_POL_LOW,
        .fault_clr_mode = 1,
    };

    epwm_fs_state_t fs_sta = {
        .cmp0_fs_state = 0,
        .cmp1_fs_state = 0,
    };

    /*BREADKIN*/
    mc_sem_cfg();
    irq_attach(SEM1_O_SEM_CPU_INTR_NUM, mc_breakin_handle, NULL);
    irq_enable(SEM1_O_SEM_CPU_INTR_NUM);
    sdrv_epwm_lld_cor_err_int_enable(APB_EPWM1_BASE,
                                     PWM_OVRCUR_FAULT_SOURCE_BIT);
    sdrv_epwm_fault_event(mc_chnl1, PWM_OVRCUR_FAULT_SOURCE, &fault_cfg,
                          &fs_sta);

    sdrv_epwm_fault_event(mc_chnl2, PWM_OVRCUR_FAULT_SOURCE, &fault_cfg,
                          &fs_sta);

    sdrv_epwm_fault_event(mc_chnl3, PWM_OVRCUR_FAULT_SOURCE, &fault_cfg,
                          &fs_sta);

    sdrv_epwm_fault_event(mc_chnl4, PWM_OVRCUR_FAULT_SOURCE, &fault_cfg,
                          &fs_sta);
}

void TurnOffPWM(void)
{
    xtrg_dev_t xtrg_dev = {.base = APB_XTRG_BASE};
    xtrg_swt_config_t swt_cfg;
    memset(&swt_cfg, 0, sizeof(swt_cfg));

    swt_cfg.swt = XTRG_IDLE_SSM_SEL;
    swt_cfg.event = XTRG_SWT_EVENT_SET;
    sdrv_xtrg_swt_config(&xtrg_dev, &swt_cfg);
}

void TurnOnPWM(void)
{
    xtrg_dev_t xtrg_dev = {.base = APB_XTRG_BASE};
    xtrg_swt_config_t swt_cfg;
    memset(&swt_cfg, 0, sizeof(swt_cfg));

    /*CLEAR IDLE by XTRG_SW_TRIG*/
    swt_cfg.swt = XTRG_IDLE_SSM_SEL;
    swt_cfg.event = XTRG_SWT_EVENT_CLEAR;
    sdrv_xtrg_swt_config(&xtrg_dev, &swt_cfg);
}

/*point 0 trig adc done*/
void xtrg_tmux0_done_handler(void)
{
#ifdef TEST_DUTY_CHANGE_CYCLE_BY_CYCLE
    static uint16_t loop = 0;
    sdrv_epwm_cmp_val_upd_bcd(mc_chnl2, pwm_cnt_array[loop],
                              PWM_PERIOD_CYCLES - pwm_cnt_array[loop]);
    sdrv_epwm_cmp_val_upd_bcd(mc_chnl3, pwm_cnt_array[loop],
                              PWM_PERIOD_CYCLES - pwm_cnt_array[loop]);
    sdrv_epwm_cmp_val_upd_bcd(mc_chnl4, pwm_cnt_array[loop],
                              PWM_PERIOD_CYCLES - pwm_cnt_array[loop]);
    loop++;
    if (loop >= DUTY_VAL_TOTAL)
        loop = 0;
#endif
}

/*point half_period trig adc done*/
void xtrg_tmux1_done_handler(void) {}

void xtrg_tmux_done_handler(void *arg, sdrv_xtrg_int_event_e event)
{
    if (event == TMUX0_DONE_EVENT)
        xtrg_tmux0_done_handler();
    if (event == TMUX1_DONE_EVENT)
        xtrg_tmux1_done_handler();

    /*xtrg_dev_t xtrg_dev = {.base = APB_XTRG_BASE};
    uint16_t M1_ADCDataReg2;
    uint16_t M1_ADCDataReg3;
    uint16_t M1_ADCDataReg2_sec;
    uint16_t M1_ADCDataReg3_sec;

    M1_ADCDataReg2 = sdrv_xtrg_adc_rslt(&xtrg_dev, M1_TMUX_SEL, TMUX_ADC2_RST);
    M1_ADCDataReg3 = sdrv_xtrg_adc_rslt(&xtrg_dev, M1_TMUX_SEL, TMUX_ADC3_RST);
    M1_ADCDataReg2_sec =
        sdrv_xtrg_adc_rslt(&xtrg_dev, M1_TMUX1_SEL, TMUX_ADC2_RST);
    M1_ADCDataReg3_sec =
        sdrv_xtrg_adc_rslt(&xtrg_dev, M1_TMUX1_SEL, TMUX_ADC3_RST);
                */
}

void xtrg_init(void)
{
    xtrg_swt_config_t swt_cfg;
    xtrg_sig_t EPWM_CMP0 = EPWM_CMP0(0);
    xtrg_sig_t EPWM2_CMP0 = EPWM_CMP0(1);
    xtrg_tmux_drt_config_t tmux0_dir_config;
    xtrg_tmux_drt_config_t tmux1_dir_config;
    xtrg_config_t xtrg_config;
    memset(&xtrg_config, 0, sizeof(xtrg_config));

    /*EPWM1/2 CNT IS HOLDED, NO PWM OUTPUT*/
    swt_cfg.swt = XTRG_TWO_MOTOR_HOLD_SW_TRIG_SEL;
    swt_cfg.width = 0xff;
    swt_cfg.event = XTRG_SWT_EVENT_SET;
    sdrv_xtrg_swt_config(&xtrg_dev_hfre, &swt_cfg);

    xtrg_config.sig_i = XTRG_SWT(XTRG_TWO_MOTOR_HOLD_SW_TRIG_SEL);
    xtrg_config.sig_o = EPWM_CLR_O(XTRG_EPWM_SEL);
    sdrv_xtrg_syncmux_config(&xtrg_dev_hfre, &xtrg_config);

    xtrg_config.sig_i = XTRG_SWT(XTRG_TWO_MOTOR_HOLD_SW_TRIG_SEL);
    xtrg_config.sig_o = EPWM_CLR_O(XTRG_EPWM2_SEL);
    sdrv_xtrg_syncmux_config(&xtrg_dev_hfre, &xtrg_config);

    /*ADC TRIGGER CONFIGURE*/
    tmux0_dir_config.trigger_sig = EPWM_CMP0;
    tmux0_dir_config.tid = 4;
    tmux0_dir_config.target_lut[0] = 0x06;
    tmux0_dir_config.tmux0_drt_sel = XTRG_TMUX0_DRT0;
    sdrv_xtrg_tmux0_drt_config(&xtrg_dev_hfre, &tmux0_dir_config);

    /*ADC TRIGGER CONFIGURE*/
    tmux1_dir_config.trigger_sig = EPWM2_CMP0;
    tmux1_dir_config.tid = 5;
    tmux1_dir_config.target_lut[0] = 0x06;
    tmux1_dir_config.tmux0_drt_sel = XTRG_TMUX0_DRT1;
    sdrv_xtrg_tmux0_drt_config(&xtrg_dev_hfre, &tmux1_dir_config);

    /* PWM CHANNEL TO IO*/
    xtrg_config.sig_i = EPWM_CMP_B0(XTRG_EPWM_SEL);
    xtrg_config.sig_o = IO_o(M1_CHNLB_0);
    sdrv_xtrg_io_out_config(&xtrg_dev_hfre, &xtrg_config, true);
    xtrg_config.sig_i = EPWM_CMP_B1(XTRG_EPWM_SEL);
    xtrg_config.sig_o = IO_o(M1_CHNLB_1);
    sdrv_xtrg_io_out_config(&xtrg_dev_hfre, &xtrg_config, true);
    xtrg_config.sig_i = EPWM_CMP_C0(XTRG_EPWM_SEL);
    xtrg_config.sig_o = IO_o(M1_CHNLC_0);
    sdrv_xtrg_io_out_config(&xtrg_dev_hfre, &xtrg_config, true);
    xtrg_config.sig_i = EPWM_CMP_C1(XTRG_EPWM_SEL);
    xtrg_config.sig_o = IO_o(M1_CHNLC_1);
    sdrv_xtrg_io_out_config(&xtrg_dev_hfre, &xtrg_config, true);
    xtrg_config.sig_i = EPWM_CMP_D0(XTRG_EPWM_SEL);
    xtrg_config.sig_o = IO_o(M1_CHNLD_0);
    sdrv_xtrg_io_out_config(&xtrg_dev_hfre, &xtrg_config, true);
    xtrg_config.sig_i = EPWM_CMP_D1(XTRG_EPWM_SEL);
    xtrg_config.sig_o = IO_o(M1_CHNLD_1);
    sdrv_xtrg_io_out_config(&xtrg_dev_hfre, &xtrg_config, true);

    xtrg_config.sig_i = EPWM_CMP0(XTRG_EPWM_SEL);
    xtrg_config.sig_o = IO_o(M1_TMUX_TRIG);
    sdrv_xtrg_io_out_config(&xtrg_dev_hfre, &xtrg_config, true);

    xtrg_config.sig_i = EPWM_CMP0(XTRG_EPWM2_SEL);
    xtrg_config.sig_o = IO_o(M1_TMUX_TRIG_2);
    sdrv_xtrg_io_out_config(&xtrg_dev_hfre, &xtrg_config, true);

    /* XTRG SW_TRIG TO PWM IDLE STATE*/
    xtrg_sse5_config_t sse5_config_1 = {0};
    sse5_config_1.ssm_sel = XTRG_IDLE_SSM_SEL;
    sse5_config_1.in[0].sig = (uint32_t)XTRG_SWT(XTRG_MOTOR1_TURN_ON_OFF);
    sse5_config_1.synthesis_val = 0xAAAAAAAA;
    sdrv_xtrg_sse5_enable(&xtrg_dev_hfre, &sse5_config_1, true);

    /*SWT(1) -> SSIG(1)->EPWM_FAULT0*/
    xtrg_config.sig_i = (uint32_t)SSIG(XTRG_IDLE_SSM_SEL);
    xtrg_config.sig_o = EPWM_FAULT(XTRG_EPWM_SEL, PWM_IDLE_FAULT_SOURCE);
    sdrv_xtrg_smux_config(&xtrg_dev_hfre, &xtrg_config);

    if (IDLE_HANG == 1) {
        /*XTRG SW TRIG HIGH*/
        swt_cfg.swt = XTRG_MOTOR1_TURN_ON_OFF;
        swt_cfg.width = 0xff;
        swt_cfg.event = XTRG_SWT_EVENT_SET;
        sdrv_xtrg_swt_config(&xtrg_dev_hfre, &swt_cfg);
    }

    /* OVFCUR FAULT SOURCE INPUT*/
    xtrg_config.sig_i = IO_i(M1_FAULT_INPUT);
    xtrg_config.sig_o = EPWM_FAULT(XTRG_EPWM_SEL, PWM_OVRCUR_FAULT_SOURCE);
    sdrv_xtrg_smux_config(&xtrg_dev_hfre, &xtrg_config);

    xtrg_callback_t xtrg_tmux0_done_handler_cb = xtrg_tmux_done_handler;
    xtrg_set_cb(&xtrg_dev_hfre, xtrg_tmux0_done_handler_cb, NULL);
    sdrv_xtrg_init(&xtrg_dev_hfre);
    sdrv_xtrg_int_enable(&xtrg_dev_hfre, 1 << 16);
    sdrv_xtrg_int_enable(&xtrg_dev_hfre, 1 << 17);
}

int32_t adc_htrc_init(addr_t base, const adc_config_t *cfg)
{
    int32_t ret = 0;
    uint32_t div;
    uint32_t cov_clks;
    clk_rate_t adc_clk;
    int i;

    ASSERT(cfg);

    sadc_lld_soft_rst(base);
    sadc_lld_adc_init(base, 0);

    sadc_lld_set_ana_para_sdsel(base, 0, cfg->is_diff);

    sadc_lld_clr_int_sta(base, 0xffffffff);
    sadc_lld_set_int_sta_en(base, 0);
    sadc_lld_set_int_sta_en(base,
                            BIT(SADC_INT_STA_EN_WATER_MARK_SUB_0_FIELD_OFFSET));
    sadc_lld_set_int_sig_en(base, 0);
    sadc_lld_set_int_sig_en(base,
                            BIT(SADC_INT_SIG_EN_WATER_MARK_SUB_0_FIELD_OFFSET));

    sadc_lld_set_sch_cfg_ts_vld(base, 0);
    sadc_lld_set_sch_cfg_ts_rst(base);
    sadc_lld_set_sch_cfg_sync_mode(base, SADC_ASYNC_MODE);
    sadc_lld_set_sch_cfg_mst_mode(base, SADC_MASTER_MODE);
    sadc_lld_set_sch_cfg_slot_mode(base, SADC_INTERVAL_NOCONST);

    sadc_lld_set_clk_ctrl_div_bypass(base, 0);

    adc_clk = clk_get_rate(cfg->clk);
    div = adc_clk / SADC_REF_CLK_MAX;
    div = ROUNDUP(div, 2);
    while (1) {
        cov_clks = adc_clk / (div * cfg->sample_freq);
        for (i = 0; i < ARRAY_SIZE(samctrl_to_clk_cnt); i++) {
            if (cov_clks <= samctrl_to_clk_cnt[i])
                break;
        }
        if (i < ARRAY_SIZE(samctrl_to_clk_cnt)) {
            break;
        } else if (div < SADC_RCLK_DIV_MAX) {
            div += 2;
        } else {
            i = ARRAY_SIZE(samctrl_to_clk_cnt) - 1;
            break;
        }
    }
    sadc_lld_set_clk_ctrl_ref_low(base, (div / 2) - 1);
    sadc_lld_set_clk_ctrl_ref_high(base, (div / 2) - 1);
    sadc_lld_set_ana_para_samctrl(base, 0, i);
    sadc_lld_set_ana_para_refsel(base, 0, SADC_REFP1);

    sadc_lld_set_fifo_cfg_bypass(base, 1);

    if (base == APB_ADC2_BASE) {
        /*GPIO_C7*/
        sadc_lld_set_rcht_entry_amsel(
            base, SADC_ENTRY_SEL, ADC_CH_SEL_TAISHAN(5, ADC_CH_POLAR_POS, 3));
        /*GPIO_C14*/
        sadc_lld_set_rcht_entry_amsel(
            base, SADC_ENTRY_SEL_2, ADC_CH_SEL_TAISHAN(5, ADC_CH_POLAR_NEG, 1));
    }

    if (base == APB_ADC3_BASE) {
        /*GPIO_C3*/
        sadc_lld_set_rcht_entry_amsel(
            base, SADC_ENTRY_SEL, ADC_CH_SEL_TAISHAN(5, ADC_CH_POLAR_POS, 1));
        /*GPIO_C4*/
        sadc_lld_set_rcht_entry_amsel(
            base, SADC_ENTRY_SEL_2, ADC_CH_SEL_TAISHAN(5, ADC_CH_POLAR_NEG, 2));
    }

    sadc_lld_set_rcht_entry_repeat_times(base, SADC_ENTRY_SEL, 0);
    sadc_lld_set_rcht_entry_repeat_mode(base, SADC_ENTRY_SEL,
                                        SADC_TRANSFER_ONCE_MODE);
    sadc_lld_set_rcht_entry_csel(base, SADC_ENTRY_SEL, 0);

    sadc_lld_set_rcht_entry_repeat_times(base, SADC_ENTRY_SEL_2, 0);
    sadc_lld_set_rcht_entry_repeat_mode(base, SADC_ENTRY_SEL_2,
                                        SADC_TRANSFER_ONCE_MODE);
    sadc_lld_set_rcht_entry_csel(base, SADC_ENTRY_SEL_2, 0);

    sadc_lld_set_htc_done_len(base, 0xF);
    sadc_lld_set_htc_ready_len(base, 0xF);
    sadc_lld_set_htc_ready(base, 1);

    return ret;
}

void mc_adc_common_init(void)
{
    const scr_signal_t adc_scr[] = {
        SCR_SF_ANA_SF_CFG_APD_A_C0_CTRL_3_0,
        SCR_SF_ANA_SF_CFG_APD_A_C7_CTRL_3_0,
        SCR_SF_ANA_SF_CFG_APD_A_C3_CTRL_3_0,
        SCR_SF_ANA_SF_CFG_APD_A_C1_CTRL_3_0,
        SCR_SF_ANA_SF_CFG_APD_A_C4_CTRL_3_0,
        SCR_SF_ANA_SF_CFG_APD_A_C2_CTRL_3_0,
    };

    for (int i = 0; i < ARRAY_SIZE(adc_scr); i++) {
        /* APD_A ctrl */
        scr_set(&g_scr_sf, &adc_scr[i], 1);
    }

    writel(0xFFFF, APB_ADC2_BASE + 0x244);
}

void sync_start(void)
{
    /*PWM1 CNT ENABLE*/
    sdrv_epwm_cnt_en(mc_chnl1, true);
    /*PWM2 CNT ENABLE*/
    sdrv_epwm_cnt_en(mc_adc_trig2, true);
    /*SYNC START*/
    xtrg_swt_config_t swt_cfg;
    swt_cfg.swt = XTRG_TWO_MOTOR_HOLD_SW_TRIG_SEL;
    swt_cfg.event = XTRG_SWT_EVENT_CLEAR;
    sdrv_xtrg_swt_config(&xtrg_dev_hfre, &swt_cfg);
}

int mc_sw_trig_init(void)
{
    int32_t ret;

    ret = taishan_adc_probe(&dev, &adc1_cfg);
    if (ret != 0) {
        printf("adc1_init error\n");
        return 0;
    }

    return 0;
}

void motor_app_main_start(void)
{
    gpio_dev = GPIO_INSTANCE(pin);
    mc_adc_common_init();
    adc_htrc_init(APB_ADC2_BASE, &adc2_3_cfg);
    adc_htrc_init(APB_ADC3_BASE, &adc2_3_cfg);
    pwm_init();
    xtrg_init();
    pwm_breakin();
    sync_start();

#ifdef TEST_SW_TRIG_ADC
    int32_t ret;
    uint32_t ch;
    uint32_t ch2;
    DEFINE_CHANNEL_MASK(channel);
    ch = ADC_CH_SEL_TAISHAN(4, ADC_CH_POLAR_NEG, 0);
    ch2 = ADC_CH_SEL_TAISHAN(4, ADC_CH_POLAR_POS, 0);
    mc_sw_trig_init();
    while (1) {
        adc_clear_channel_mask(&channel, 1, ch2);
        adc_add_channel_mask(&channel, 1, ch);
        ret = adc_read_sample(&dev, &channel, adc_buff, ARRAY_SIZE(adc_buff));
        if (ret != 0) {
            printf("adc_read_sample error\n");
            return;
        }
        adc_clear_channel_mask(&channel, 1, ch);
        adc_add_channel_mask(&channel, 1, ch2);
        ret = adc_read_sample(&dev, &channel, adc_buff2, ARRAY_SIZE(adc_buff));
        if (ret != 0) {
            printf("adc_read_sample error\n");
            return;
        }
    }
#endif
}

static int motor_app_main(int argc, char *argv[])
{
    motor_app_main_start();
    return 0;
}

CLI_CMD("mca", "\r\nmotor_app:\r\n Start motor app\r\n", motor_app_main);
