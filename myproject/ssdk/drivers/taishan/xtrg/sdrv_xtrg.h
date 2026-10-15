/**
 * @file sdrv_xtrg.h
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#ifndef SDRV_XTRG_H
#define SDRV_XTRG_H

#include <types.h>

#define FUNC_INT_STA (0x0)
#define FUNC_INT_STA_EN (0x4)
#define FUNC_INT_SIG_EN (0x8)
#define FUNC_INT_STA_SHIFT(int_id) (int_id)
#define FUNC_INT_STA_MASK(int_id) (0x1 << FUNC_INT_STA_SHIFT(int_id))

#define FUNC_ERR_INT_STA (0x10)
#define FUNC_ERR_INT_STA_EN (0x14)

#define FUNC_COR_ERR_INT_SIG_EN (0x18)
#define FUNC_UNC_ERR_INT_SIG_EN (0x1C)

#define FUNC_IO_ERR0_INT_STA (0x20)
#define FUNC_IO_ERR0_INT_STA_EN (0x24)

#define FUNC_IO_COR_ERR0_INT_SIG_EN (0x28)
#define FUNC_IO_UNC_ERR0_INT_SIG_EN (0x2C)

#define FUNC_IO_ERR1_INT_STA (0x30)
#define FUNC_IO_ERR1_INT_STA_EN (0x34)

#define FUNC_IO_COR_ERR1_INT_SIG_EN (0x38)
#define FUNC_IO_UNC_ERR1_INT_SIG_EN (0x3C)

#define FUNC_SMON_CMP_ERR_INT_STA (0x40)
#define FUNC_SMON_CMP_ERR_INT_STA_EN (0x44)

#define FUNC_SMON_CMP_COR_ERR_INT_SIG_EN (0x48)
#define FUNC_SMON_CMP_UNC_ERR_INT_SIG_EN (0x4C)

#define SWDT_ERR_INT_STA (0x50)
#define SWDT_ERR_INT_STA_EN (0x54)

#define SWDT_COR_ERR_INT_SIG_EN (0x58)
#define SWDT_UNC_ERR_INT_SIG_EN (0x5C)

#define FUNC_SEQ_ERR_INT_STA (0x60)
#define FUNC_SEQ_ERR_INT_STA_EN (0x64)

#define FUNC_SEQ_COR_ERR_INT_SIG_EN (0x68)
#define FUNC_SEQ_UNC_ERR_INT_SIG_EN (0x6C)

#define FAULT_SOURCE_EN0 (0xD00)
#define FAULT_SOURCE_EN1 (0xD04)
#define FAULT_SOURCE_EN2 (0xD08)
#define FAULT_SOURCE_EN3 (0xD0C)
#define FAULT_SOURCE_EN4 (0xD10)
#define FAULT_EVENT_CLR (0xD14)

#define TRG_SYNC_DIS (0x100)
#define SIG_SYNC_DIS (0x104)
#define SYNC_SYNC_DIS (0x110)
#define IO_SYNC_DIS (0x200)
#define IO_FLAT(io) (0x210 + (io)*4)

#define SW_TRG_CTRL (0xc00)
#define SW_TRG_STATUS (0xc04)
#define SW_TRG_PULSE_WIDTH(swt) (0xc10 + (swt) / 2 * 4)

#define SSE_IN_0_3_SEL(sse) (0x310 + 0x10 * (sse))
#define SSE_IN_4_SEL(sse) (SSE_IN_0_3_SEL(sse) + 0x4)
#define SSE_REG(sse) (SSE_IN_0_3_SEL(sse) + 0x8)
#define SSE_CTRL(sse) (SSE_IN_0_3_SEL(sse) + 0xc)

#define SIG_O_SEL(sig_o) (0x410 + (sig_o)*4)
#define IO_O_SEL(io_o) (0x540 + (io_o)*4)

#define IO_OUT_EN(io_o) (0x670 + (io_o) / 32 * 4)
#define SYNC_O_SEL(sync_o) (0x680 + (sync_o)*4)

#define TMUX_DEC_EN (0x8D0)
#define TMUX0_DRT_CTRL(drt_id) (0x6d0 + (drt_id)*0x18)
#define TMUX0_DRT_DECODE_SEL(drt_id, sel) (TMUX0_DRT_CTRL(drt_id) + (sel)*4)

#define TMUX0_INDRT_CTRL(drt_id) (0x790 + (drt_id)*0x28)
#define TMUX0_INDRT_DECODE_SEL(drt_id, sel) (TMUX0_INDRT_CTRL(drt_id) + (sel)*4)
#define TMUX0_INDRT_TID_POOL(drt_id, sel)                                      \
    (TMUX0_INDRT_CTRL(drt_id) + (sel)*4 + 0x8)

#define SMON_EN (0x8FC)
#define SMON_FIRST_MUX_SELECT(smon_id) (0x900 + smon_id * 8)
#define SMON_SECOND_MUX_SELECT(smon_id) (0x904 + smon_id * 8)

#define SWDT_IN_SEL(swdt_id) (0xA00 + swdt_id * 0x10)
#define SWDT_EVENT_CHECK(swdt_id) (SWDT_IN_SEL(swdt_id) + 4)
#define SWDT_PULSE_CHECK(swdt_id) (SWDT_IN_SEL(swdt_id) + 8)
#define SWDT_PULSE_CHECK_WINDOW(swdt_id) (SWDT_IN_SEL(swdt_id) + 0xC)

#define DMAC0_CTRL (0xB00)
#define DMAC1_CTRL (0xB04)

#define TMUX_ADC_RSLT(adc_id, tmux_id) (0xE00 + (adc_id)*4 + (tmux_id)*0xC)
#define TMUX_ACMP_RSLT(acmp_id, tmux_id) (0xE80 + (acmp_id)*4 + (tmux_id)*0x10)

typedef uint32_t xtrg_sig_t;

/**
 * @brief xTRG signal groups, including input (i) and output (o) signals.
 */
typedef enum sdrv_xtrg_i_group {
    /* Input signal groups. */
    XTRG_SWT_i_GROUP,
    XTRG_TRG_i_GROUP,
    XTRG_SIG_i_GROUP,
    XTRG_SYNC_i_GROUP,
    XTRG_IO_i_GROUP,
    XTRG_TID_i_GROUP,

    /* Output signal groups. */
    XTRG_SIG_o_GROUP,
    XTRG_IO_o_GROUP,
    XTRG_SYNC_o_GROUP,

    /* Logical groups for convenience. */
    XTRG_SSIG_GROUP,

} sdrv_xtrg_i_group_e;

/**
 * @brief xTRG signals.
 */
#define XTRG_i_GROUP_SHIFT 28
#define XTRG_i_GROUP(SIG) (sdrv_xtrg_i_group_e)((SIG) >> XTRG_i_GROUP_SHIFT)
#define XTRG_i_SIG(SIG) ((SIG)&0x0FFFFFFFul)

#define SWT_i_START (XTRG_SWT_i_GROUP << XTRG_i_GROUP_SHIFT)
#define SWT_i_NR 8

#define TRG_i_START (XTRG_TRG_i_GROUP << XTRG_i_GROUP_SHIFT)
#define TRG_i_NR 8

#define SIG_i_START (XTRG_SIG_i_GROUP << XTRG_i_GROUP_SHIFT)
#define SIG_i_NR 68

#define SYNC_i_START (XTRG_SYNC_i_GROUP << XTRG_i_GROUP_SHIFT)
#define SYNC_i_NR 20

#define IO_i_START (XTRG_IO_i_GROUP << XTRG_i_GROUP_SHIFT)
#define IO_i_NR 64

#define TID_i_START (XTRG_TID_i_GROUP << XTRG_i_GROUP_SHIFT)
#define TID_i_NR 64

#define SIG_o_START (XTRG_SIG_o_GROUP << XTRG_i_GROUP_SHIFT)
#define SIG_o_NR 76

#define IO_o_START (XTRG_IO_o_GROUP << XTRG_i_GROUP_SHIFT)
#define IO_o_NR 64

#define SYNC_o_START (XTRG_SYNC_o_GROUP << XTRG_i_GROUP_SHIFT)
#define SYNC_o_NR 20

#define SSIG_START (XTRG_SSIG_GROUP << XTRG_i_GROUP_SHIFT)
#define SSIG_NR 16

/* SWT_i group. */
#define XTRG_SWT(swt) (SWT_i_START + (swt))

/* SIG_i group. */
#define ETIMER_CMP_A1(ETMR_ID) (SIG_i_START + 7 * (ETMR_ID) + 0)
#define ETIMER_CMP_B0(ETMR_ID) (SIG_i_START + 7 * (ETMR_ID) + 1)
#define ETIMER_CMP_B1(ETMR_ID) (SIG_i_START + 7 * (ETMR_ID) + 2)
#define ETIMER_CMP_C0(ETMR_ID) (SIG_i_START + 7 * (ETMR_ID) + 3)
#define ETIMER_CMP_C1(ETMR_ID) (SIG_i_START + 7 * (ETMR_ID) + 4)
#define ETIMER_CMP_D0(ETMR_ID) (SIG_i_START + 7 * (ETMR_ID) + 5)
#define ETIMER_CMP_D1(ETMR_ID) (SIG_i_START + 7 * (ETMR_ID) + 6)
#define ETIMER_DIR(ETMR_ID) (SIG_i_START + 28 + (ETMR_ID))
#define EPWM_CMP_A1(EPWM_ID) (SIG_i_START + 32 + 7 * (EPWM_ID) + 0)
#define EPWM_CMP_B0(EPWM_ID) (SIG_i_START + 32 + 7 * (EPWM_ID) + 1)
#define EPWM_CMP_B1(EPWM_ID) (SIG_i_START + 32 + 7 * (EPWM_ID) + 2)
#define EPWM_CMP_C0(EPWM_ID) (SIG_i_START + 32 + 7 * (EPWM_ID) + 3)
#define EPWM_CMP_C1(EPWM_ID) (SIG_i_START + 32 + 7 * (EPWM_ID) + 4)
#define EPWM_CMP_D0(EPWM_ID) (SIG_i_START + 32 + 7 * (EPWM_ID) + 5)
#define EPWM_CMP_D1(EPWM_ID) (SIG_i_START + 32 + 7 * (EPWM_ID) + 6)
#define ADC_DONE(ADC_ID) (SIG_i_START + 60 + (ADC_ID))
#define ACMPO(ACMP_ID) (SIG_i_START + 63 + (ACMP_ID))
#define SEM_FAULT (SIG_i_START + 67)

/* SYNC_i group. */
#define ETIMER_SNAP_SHOT_I(ETMR_ID) (SYNC_i_START + (ETMR_ID))
#define ETIMER_SET_I(ETMR_ID) (SYNC_i_START + 4 + (ETMR_ID))
#define ETIMER_CLR_I(ETMR_ID) (SYNC_i_START + 8 + (ETMR_ID))
#define EPWM_SET_I(EPWM_ID) (SYNC_i_START + 12 + (EPWM_ID))
#define EPWM_CLR_I(EPWM_ID) (SYNC_i_START + 16 + (EPWM_ID))

/* TRG_i group. */
#define ETIMER_CMP0(ETMR_ID) (TRG_i_START + (ETMR_ID))
#define EPWM_CMP0(EPWM_ID) (TRG_i_START + 4 + (EPWM_ID))

/* IO_i group. */
#define IO_i(IO) (IO_i_START + (IO))

/* TID_i group. */
#define ETIMER_EID(ETMR) (TID_i_START + (ETMR)*8)
#define EPWM_EID(EPWM) (TID_i_START + 32 + (EPWM)*8)

/* SIG_o group */
#define ETIMER_CPT0(ETMR, CHNL) (SIG_o_START + ETMR * 8 + (CHNL))
#define ETIMER_FAULT(ETMR, CHNL) (SIG_o_START + 32 + (ETMR)*4 + (CHNL))
#define ETIMER_CLK(ETMR) (SIG_o_START + 48 + (ETMR))
#define EPWM_FAULT(EPWM, CHNL) (SIG_o_START + 52 + (EPWM)*4 + (CHNL))
#define EPWM_CLK(EPWM) (SIG_o_START + 68 + (EPWM))
#define ACMP_SAMPLE(ACMP) (SIG_o_START + 72 + (ACMP))

/* IO_o group. */
#define IO_o(IO) (IO_o_START + (IO))

/* SYNC_o group. */
#define ETIMER_SNAP_SHOT_O(ETMR) (SYNC_o_START + (ETMR))
#define ETIMER_SET_O(ETMR) (SYNC_o_START + 4 + (ETMR))
#define ETIMER_CLR_O(ETMR) (SYNC_o_START + 8 + (ETMR))
#define EPWM_SET_O(EPWM) (SYNC_o_START + 12 + (EPWM))
#define EPWM_CLR_O(EPWM) (SYNC_o_START + 16 + (EPWM))

/* SSIG group. */
#define SSIG(SSIG) (SSIG_START + (SSIG))

/**
 * @brief Edge detect mode for I/O filter and SSE5.
 */
typedef enum xtrg_edge {
    XTRG_POSEDGE,
    XTRG_NEGEDGE,
    XTRG_BOTHEDGE
} xtrg_edge_e;

/**
 * @brief Software Triggers sel.
 */
typedef enum xtrg_swt_sel {
    XTRG_SWT0,
    XTRG_SWT1,
    XTRG_SWT2,
    XTRG_SWT3,
    XTRG_SWT4,
    XTRG_SWT5,
    XTRG_SWT6,
    XTRG_SWT7,
} xtrg_swt_sel_e;

/**
 * @brief Software Triggers events.
 */
typedef enum xtrg_swt_event {
    XTRG_SWT_EVENT_SET,
    XTRG_SWT_EVENT_CLEAR,
    XTRG_SWT_EVENT_TRIGGER,
    XTRG_SWT_EVENT_PULSE,
} xtrg_swt_event_e;

/**
 * @brief Software Triggers config.
 */
typedef struct xtrg_swt_config {
    xtrg_swt_sel_e swt;
    xtrg_swt_event_e event;
    uint32_t width;
} xtrg_swt_config_t;

/**
 * @brief ssm sel.
 */
typedef enum xtrg_ssm_sel {
    XTRG_SSM0,
    XTRG_SSM1,
    XTRG_SSM2,
    XTRG_SSM3,
    XTRG_SSM4,
    XTRG_SSM5,
    XTRG_SSM6,
    XTRG_SSM7,
    XTRG_SSM8,
    XTRG_SSM9,
    XTRG_SSM10,
    XTRG_SSM11,
    XTRG_SSM12,
    XTRG_SSM13,
    XTRG_SSM14,
    XTRG_SSM15,
} xtrg_ssm_sel_e;

/**
 * @brief SSE5 configuration structure.
 */
typedef struct xtrg_sse5_config {
    /* 5 input signals. */
    struct {
        uint32_t sig;

        /* Detect mode. False for level detect. True for Edge detect. */
        bool edge_detect;

        /* Edge select, when edge_detect is True. */
        xtrg_edge_e edge;
    } in[5];

    /*
     * Synthesis Engine register (sse_reg) value. The output SSIG
     * is ssig=sse_reg[in], where "in" is the 5 bit input signal value.
     */
    uint32_t synthesis_val;

    xtrg_ssm_sel_e ssm_sel;

} xtrg_sse5_config_t;

/**
 * @brief SMON sel.
 */
typedef enum xtrg_smon_sel {
    XTRG_SMON0,
    XTRG_SMON1,
    XTRG_SMON2,
    XTRG_SMON3,
    XTRG_SMON4,
    XTRG_SMON5,
    XTRG_SMON6,
    XTRG_SMON7,
    XTRG_SMON8,
    XTRG_SMON9,
    XTRG_SMON10,
    XTRG_SMON11,
    XTRG_SMON12,
    XTRG_SMON13,
    XTRG_SMON14,
    XTRG_SMON15,
    XTRG_SMON16,
    XTRG_SMON17,
    XTRG_SMON18,
    XTRG_SMON19,
    XTRG_SMON20,
    XTRG_SMON21,
    XTRG_SMON22,
    XTRG_SMON23,
    XTRG_SMON24,
    XTRG_SMON25,
    XTRG_SMON26,
    XTRG_SMON27,
    XTRG_SMON28,
    XTRG_SMON29,
    XTRG_SMON30,
    XTRG_SMON31,
} xtrg_smon_sel_e;

/**
 * @brief SMON configuration structure.
 */
typedef struct xtrg_smon_config {
    /* First level mux select
        sig_out_sel from SYNC_o, SIG_o and IO_o*/
    uint32_t sig_out_sel;

    /* sig_in_sel  from IO_i, TRG_i, SIG_i, SYNC_i, SSIG and SWT */
    uint32_t sig_in_sel;

    /* Second level mux select Compare input0/1 mux select from first level mux
     * output */
    uint32_t sig_input1_sel;
    uint32_t sig_input0_sel;

    /* Change polarity of input0 of comparator */
    bool change_pol_in0;

    xtrg_smon_sel_e smon_sel;
} xtrg_smon_config_t;

/**
 * @brief event trigger mode.
 */
typedef enum xtrg_event_trigger_mode {
    XTRG_EVENT_TRIGGER_MODE_POSEDGE,
    XTRG_EVENT_TRIGGER_MODE_NEGEDGE,
    XTRG_EVENT_TRIGGER_MODE_TOGGLE,
    XTRG_EVENT_TRIGGER_MODE_HIGH,
    XTRG_EVENT_TRIGGER_MODE_LOW,
} xtrg_event_trigger_mode_e;

/**
 * @brief swdt sel.
 */
typedef enum xtrg_swdt_sel {
    XTRG_SWDT0,
    XTRG_SWDT1,
    XTRG_SWDT2,
    XTRG_SWDT3,
} xtrg_swdt_sel_e;

/**
 * @brief tmux index sel.
 */
typedef enum xtrg_tmux_sel {
    XTRG_TMUX0,
    XTRG_TMUX1,
    XTRG_TMUX2,
    XTRG_TMUX3,
    XTRG_TMUX4,
    XTRG_TMUX5,
    XTRG_TMUX6,
    XTRG_TMUX7,
} xtrg_tmux_sel_e;

/**
 * @brief tmux adc sel.
 */
typedef enum xtrg_adc_sel {
    XTRG_ADC0,
    XTRG_ADC1,
    XTRG_ADC2,
} xtrg_adc_sel_e;

/**
 * @brief tmux acmp sel.
 */
typedef enum xtrg_acmp_sel {
    XTRG_ACMP0,
    XTRG_ACMP1,
    XTRG_ACMP2,
    XTRG_ACMP3,
} xtrg_acmp_sel_e;

/**
 * @brief SWDT event check configuration structure.
 */
typedef struct xtrg_swdt_event_check_config {
    /* SWDT input select from SMON first level mux output */
    uint32_t swdt_sig_sel;

    /* Preconfigured time window for check event happen */
    uint32_t swdt_event_check_window;

    /* event trigger mode */
    xtrg_event_trigger_mode_e event_trg_mode;

    xtrg_swdt_sel_e swdt_sel;
} xtrg_swdt_event_check_config_t;

/**
 * @brief Pulse signal width check.
 */
typedef enum xtrg_pulse_check_mode {
    XTRG_PULSE_CHECK_MODE_BETWEEN_WINDOW,
    XTRG_PULSE_CHECK_MODE_MORE_OR_LESS_WINDOW,
} xtrg_pulse_check_mode_e;

/**
 * @brief Pulse signal width edge.
 */
typedef enum xtrg_pulse_check_edge {
    XTRG_PULSE_CHECK_POSITIVE_PULSE,
    XTRG_PULSE_CHECK_NEGATIVE_PULSE,
} xtrg_pulse_check_edge_e;

/**
 * @brief SWDT pulse check configuration structure.
 */
typedef struct xtrg_swdt_pulse_check_config {
    /* SWDT input select from SMON first level mux output */
    uint32_t swdt_sig_sel;

    /* Pulse check mode */
    xtrg_pulse_check_mode_e swdt_pulse_check_mode;

    /* Pulse check edge */
    xtrg_pulse_check_edge_e swdt_pulse_check_edge;

    /* Max value of check window */
    uint32_t swdt_max_check_window;

    /* Min value of check window */
    uint32_t swdt_min_check_window;

    /* SWDT select */
    xtrg_swdt_sel_e swdt_sel;
} xtrg_swdt_pulse_check_config_t;

/**
 * @brief swdt check mode.
 */
typedef enum xtrg_swdt_check_mode {
    XTRG_SWDT_PULSE_CHECK,
    XTRG_SWDT_EVENT_CHECK,
} xtrg_swdt_check_mode_e;

/**
 * @brief swdt conifg.
 */
typedef struct xtrg_swdt_config {
    xtrg_swdt_pulse_check_config_t xtrg_swdt_pulse_check_config;
    xtrg_swdt_event_check_config_t xtrg_swdt_event_check_config;
    xtrg_swdt_check_mode_e xtrg_swdt_check_mode;
} xtrg_swdt_config_t;

/**
 * @brief I/O Filter configuration.
 */
typedef struct xtrg_io_filter_config {
    /* Debouncing filter edge. */
    xtrg_edge_e edge;

    /*
     * Filter intervals for positive and negative edges, in units of
     * sampling intervals. Range: 2 ~ 17.
     */
    uint32_t pos_edge_intvl;
    uint32_t neg_edge_intvl;

    /* Sampling interval, in timer clock cycles. Range 1~256. */
    uint32_t sampling_intvl;

} xtrg_io_filter_config_t;

/**
 * @brief tmux0 drt sel.
 */
typedef enum xtrg_tmux0_drt_sel {
    XTRG_TMUX0_DRT0,
    XTRG_TMUX0_DRT1,
    XTRG_TMUX0_DRT2,
    XTRG_TMUX0_DRT3,
    XTRG_TMUX0_DRT4,
    XTRG_TMUX0_DRT5,
    XTRG_TMUX0_DRT6,
    XTRG_TMUX0_DRT7,
} xtrg_tmux0_drt_sel_e;

/**
 * @brief tmux0 indrt sel.
 */
typedef enum xtrg_tmux0_indrt_sel {
    XTRG_TMUX0_INDRT0,
    XTRG_TMUX0_INDRT1,
    XTRG_TMUX0_INDRT2,
    XTRG_TMUX0_INDRT3,
    XTRG_TMUX0_INDRT4,
    XTRG_TMUX0_INDRT5,
    XTRG_TMUX0_INDRT6,
    XTRG_TMUX0_INDRT7,
} xtrg_tmux0_indrt_sel_e;

/**
 * @brief Trigger Mux (tmux) configuration.
 */
typedef struct xtrg_tmux_drt_config {
    /* Trigger signal from TRG_i or SSIG groups. */
    uint32_t trigger_sig;

    /* TID_i signal, one of etimer_eid, or epwm_eid. */
    uint32_t tid;

    /* The LUT is used to select which ADC or ACMP should be triggered
     * according to tid[7:5].
     */
    uint8_t target_lut[8];

    /* TMUX drt select */
    xtrg_tmux0_drt_sel_e tmux0_drt_sel;
} xtrg_tmux_drt_config_t;

/**
 * @brief Trigger Mux (tmux) configuration.
 */
typedef struct xtrg_tmux_indrt_config {
    /* Trigger signal from TRG_i or SSIG groups. */
    uint32_t trigger_sig;

    /* TERM_VAL, when read address of ID pool reaches TERM_VAL, it goes back to
     * 0. */
    uint32_t term_val;

    /* TERM_TID, when current read TID of ID pools matches TERM_TID, read
     * address of ID pool can be reset to 0. */
    uint32_t term_tid;

    /* The LUT is used to select which ADC or ACMP should be triggered
     * according to tid[7:5].
     */
    uint8_t target_lut[8];

    /* TMUX indrt input mux TID pool
    pool0 pool1 8(bit) * 8(entries) */
    uint32_t tid_pool[2];

    /*TMUX indrt select*/
    xtrg_tmux0_indrt_sel_e tmux0_indrt_sel;

} xtrg_tmux_indrt_config_t;

/**
 * @brief Edge detect mode for dma.
 */
typedef enum xtrg_dma_ssig_mode {
    XTRG_SSIG_POSEDGE,
    XTRG_SSIG_NEGEDGE
} xtrg_dma_ssig_mode_e;

/**
 * @brief DMA sel.
 */
typedef enum xtrg_dma_sel { XTRG_DMA0, XTRG_DMA1 } xtrg_dma_sel_e;

typedef struct xtrg_dma_config {
    /* Select one of SSIG signals as DMA request trigger */
    xtrg_ssm_sel_e ssm_sel;

    /* Generate DMA request on pos/neg edge of selcted SSIG */
    xtrg_dma_ssig_mode_e ssig_mode;

    /* DMA select */
    xtrg_dma_sel_e dma_sel;
} xtrg_dma_config_t;

/**
 * @brief Enable or disable input signal synchronization.
 *
 * SIG_i, TRG_i, SYNC_i and IO_i signals are synchronized to timer clock domain
 * by default. This function enable or disable signal synchronizations.
 *
 * @param[in] base   xTRG base.
 * @param[in] sig    The input signal to enable or disable synchronization for.
 * @param[in] enable Enable or disable synchronization.
 */
void sdrv_xtrg_lld_sync_enable(paddr_t base, xtrg_sig_t sig, bool enable);

/**
 * @brief Configure I/O filters.
 *
 * @param[in] base   xTRG base.
 * @param[in] sig    The input signal to enable or disable debouncing for.
 * @param[in] config I/O filter config.
 */
void sdrv_xtrg_lld_io_filter_config(paddr_t base, xtrg_sig_t sig,
                                    const xtrg_io_filter_config_t *config,
                                    bool enable);

/**
 * @brief Trigger SWT (software trigger) status.
 *
 * @param[in] base   xTRG base.
 * @param[in] swt    xTRG swt index.
 * @param[in] config swt config.
 */
void sdrv_xtrg_lld_swt_trigger(paddr_t base, xtrg_swt_sel_e swt,
                               xtrg_swt_event_e event);

/**
 * @brief get swt status.
 *
 * @param[in] base   xTRG base.
 * @param[in] swt    xTRG swt index.
 */
bool sdrv_xtrg_lld_swt_status(paddr_t base, xtrg_swt_sel_e swt);

/**
 * @brief Set SWT pulse width.
 *
 * @param[in] base   xTRG base.
 * @param[in] swt    xTRG swt index.
 * @param[in] width  xTRG swt pulse width.
 */
void sdrv_xtrg_lld_swt_set_pulse_width(paddr_t base, xtrg_swt_sel_e swt,
                                       uint32_t width);

/**
 * @brief Configure SSE (Signal Synthesis Engine).
 * Each SSE has 5 input signals, in[4:0], and one value register SSE_REG. The
 * output ssig value is ssig = sse_reg[in].
 *
 * @param[in] base    xTRG base.
 * @param[in] config  xTRG sse config.
 */
void sdrv_xtrg_lld_sse5_config(paddr_t base, const xtrg_sse5_config_t *config);

/**
 * @brief Enable or disable SSE5.
 *
 * @param[in] base    xTRG base.
 * @param[in] enable  Enable or disable sse5.
 * @param[in] config  xTRG sse config.
 */
void sdrv_xtrg_lld_sse5_enable(paddr_t base, bool enable,
                               const xtrg_sse5_config_t *config);

/**
 * @brief Configure signal mux, muxing input signals to SIG_o or IO_o.
 *
 * The smux module muxes SSIG, TRG_i, SIG_i, and I/O inputs to SIG_o
 * and I/O outputs.
 *
 * @param[in] base    xTRG base.
 * @param[in] sig_i   xTRG input signal.
 * @param[in] sig_o   xTRG output signal.
 */
void sdrv_xtrg_lld_smux_config(paddr_t base, xtrg_sig_t sig_i,
                               xtrg_sig_t sig_o);

/**
 * @brief Enable or disable I/O output.
 *
 * @param[in] base    xTRG base.
 * @param[in] sig_o   xTRG output signal.
 * @param[in] enable  Enable or disable io out.
 */
void sdrv_xtrg_lld_io_out_enable(paddr_t base, xtrg_sig_t sig_o, bool enable);

/**
 * @brief Configure IO output, muxing SSIG_i,TRG_i,SIG_i,IO_i to IO_o.
 *
 * @param[in] base    xTRG base.
 * @param[in] sig_i   xTRG input signal.
 * @param[in] sig_o   xTRG output signal.
 */
void sdrv_xtrg_lld_io_out_config(paddr_t base, xtrg_sig_t sig_i,
                                 xtrg_sig_t sig_o);

/**
 * @brief Configure syncmux, muxing input sync signals to SYNC_o.
 *
 * The syncmux module muxes SWT_i, SSIG and SYNC_i inputs to SYNC_o outputs.
 *
 * @param[in] base    xTRG base.
 * @param[in] sig_i   xTRG input signal.
 * @param[in] sig_o   xTRG output signal.
 */
void sdrv_xtrg_lld_syncmux_config(paddr_t base, xtrg_sig_t sig_i,
                                  xtrg_sig_t sig_o);

/**
 * @brief Configure the Trigger Mux (tmux) drt modules..
 *
 * The tmux muxes TRG_i, SSIG signals to ADC and ACMP modules, with specified
 * TID. There are 8 "drt" decoders, whose TID comes from TID_i, and 8
 * "indrt" decoders, whose TID comes from xTRG internel TID pool.
 *
 * @param[in] base     xTRG base.
 * @param[in] config   tmux drt configuratin.
 */
void sdrv_xtrg_lld_tmux0_drt_config(paddr_t base,
                                    const xtrg_tmux_drt_config_t *config);

/**
 * @brief Configure the Trigger Mux (tmux) indrt modules..
 *
 * The tmux muxes TRG_i, SSIG signals to ADC and ACMP modules, with specified
 * TID. There are 8 "drt" decoders, whose TID comes from TID_i, and 8
 * "indrt" decoders, whose TID comes from xTRG internel TID pool.
 *
 * @param[in] base        xTRG base.
 * @param[in] config      tmux indrt configuration.
 */
void sdrv_xtrg_lld_tmux0_indrt_config(paddr_t base,
                                      const xtrg_tmux_indrt_config_t *config);

/**
 * @brief Configure SMON (Signal Monitor).
 * The SMON monitors the input and output signal groups
 * First level select 32 input signals and 32 output signals.
 * Second level select inputs from output of 1st level
 *
 * @param[in] base     xTRG base.
 * @param[in] config   smon configuration.
 */
void sdrv_xtrg_lld_smon_config(paddr_t base, const xtrg_smon_config_t *config);

/**
 * @brief Enable or disable SMON.
 *
 * @param[in] base     xTRG base.
 * @param[in] smon_id  smon id (0~31).
 * @param[in] enable   Enable or disable smon.
 * @param[in] config   smon configuration.
 */
void sdrv_xtrg_lld_smon_enable(paddr_t base, bool enable,
                               const xtrg_smon_config_t *config);

/**
 * @brief Configure SWDT Event Check.(Signal Watch Dog Timer).
 *
 * @param[in] base     xTRG base.
 * @param[in] config   swdt configuration.
 */
void sdrv_xtrg_lld_swdt_event_check_config(
    paddr_t base, const xtrg_swdt_event_check_config_t *config);

/**
 * @brief Enable or disable SWDT Event Check.
 *
 * @param[in] base     xTRG base.
 * @param[in] enable   Enable or disable swdt.
 * @param[in] config   swdt configuration.
 */
void sdrv_xtrg_lld_swdt_event_check_enable(
    paddr_t base, bool enable, const xtrg_swdt_event_check_config_t *config);

/**
 * @brief Configure SWDT Pulse Check.(Signal Watch Dog Timer).
 *
 * @param[in] base     xTRG base.
 * @param[in] config   swdt configuration.
 */
void sdrv_xtrg_lld_swdt_pulse_check_config(
    paddr_t base, const xtrg_swdt_pulse_check_config_t *config);

/**
 * @brief Enable or disable SWDT Pulse Check.
 *
 * @param[in] base     xTRG base.
 * @param[in] swdt_id  swdt id (0~3).
 * @param[in] enable   Enable or disable swdt.
 * @param[in] config   swdt configuration.
 */
void sdrv_xtrg_lld_swdt_pulse_check_enable(
    paddr_t base, bool enable, const xtrg_swdt_pulse_check_config_t *config);

/**
 * @brief sdrv xtrg get int status.
 *
 * @param[in] base xtrg base
 * @return FUNC_INT_STA status bitmap
 */
uint32_t sdrv_xtrg_lld_handle_irq(paddr_t base);

/**
 * @brief sdrv xtrg clear int status.
 *
 * @param[in] base xtrg base
 * @param[in] state func int state
 */
void sdrv_xtrg_lld_clear_irq(paddr_t base, uint32_t state);

/**
 * @brief sdrv xtrg int enable.
 *
 * @param[in] xtrg dev base
 * @param[in] int_id int id
 */
void sdrv_xtrg_lld_int_enable(paddr_t base, uint32_t int_id);

/**
 * @brief sdrv xtrg int disable.
 *
 * @param[in] xtrg dev base
 * @param[in] int_id int id
 */
void sdrv_xtrg_lld_int_disable(paddr_t base, uint32_t int_id);

/**
 * @brief Configure DMA0/1.
 *
 * @param[in] base    xTRG base.
 * @param[in] config  xTRG dma configuration.
 */
void sdrv_xtrg_lld_dma_config(paddr_t base, const xtrg_dma_config_t *config);

/**
 * @brief Enable or disable DMA0/1.
 *
 * @param[in] base     xTRG base.
 * @param[in] config   xTRG dma configuration.
 * @param[in] enable   Enable or disable dma.
 */
void sdrv_xtrg_lld_dma_enable(paddr_t base, const xtrg_dma_config_t *config,
                              bool enable);

/**
 * @brief Read tmux adc result.
 *
 * @param[in] base     xTRG base.
 * @param[in] tmux_id  xTRG tmux index.
 * @param[in] adc_id   xTRG adc  index.
 *
 * @return adc result
 */
uint32_t sdrv_xtrg_lld_adc_rslt(paddr_t base, xtrg_tmux_sel_e tmux_id,
                                xtrg_adc_sel_e adc_id);

/**
 * @brief Read tmux acmp result.
 *
 * @param[in] base     xTRG base.
 * @param[in] tmux_id  xTRG tmux index.
 * @param[in] acmp_id  xTRG acmp index.
 *
 * @return acmp result
 */
uint32_t sdrv_xtrg_lld_acmp_rslt(paddr_t base, xtrg_tmux_sel_e tmux_id,
                                 xtrg_acmp_sel_e acmp_id);

#endif // SDRV_XTRG_H
