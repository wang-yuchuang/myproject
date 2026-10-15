/**
 * @file sdrv_xtrg.c
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#include <reg.h>
#include <string.h>
#include <types.h>

#include "sdrv_xtrg.h"

typedef struct sdrv_xtrg_sig_group {
    /* Signal number. */
    uint32_t sig_nr;

    /* Start index of SSE5 input select mux. */
    uint32_t sse_idx;

    /* Start index of smux input. */
    uint32_t smux_idx;

    /* Start index of syncmux input. */
    uint32_t syncmux_idx;

    /* Start index of smon input. */
    uint32_t smon_idx;

    /* Synchronization disable register offset. */
    uint32_t sync_dis_off;

} sdrv_xtrg_sig_group_t;

static sdrv_xtrg_sig_group_t sdrv_xtrg_sig_groups[] = {
    [XTRG_SWT_i_GROUP] =
        {
            .sig_nr = SWT_i_NR,
            .sse_idx = 0,
            .smon_idx = 0,
            .syncmux_idx = 0,
        },
    [XTRG_TRG_i_GROUP] =
        {
            .sig_nr = TRG_i_NR,
            .sse_idx = 8,
            .smux_idx = 84,
            .smon_idx = 112,
            .sync_dis_off = TRG_SYNC_DIS,
        },
    [XTRG_SIG_i_GROUP] =
        {
            .sig_nr = SIG_i_NR,
            .sse_idx = 16,
            .smux_idx = 16,
            .smon_idx = 44,
            .sync_dis_off = SIG_SYNC_DIS,
        },
    [XTRG_SYNC_i_GROUP] =
        {
            .sig_nr = SYNC_i_NR,
            .sse_idx = 84,
            .smon_idx = 24,
            .syncmux_idx = 24,
            .sync_dis_off = SYNC_SYNC_DIS,
        },
    [XTRG_IO_i_GROUP] =
        {
            .sig_nr = IO_i_NR,
            .sse_idx = 104,
            .smux_idx = 92,
            .smon_idx = 120,
            .sync_dis_off = IO_SYNC_DIS,
        },
    [XTRG_TID_i_GROUP] =
        {
            .sig_nr = TID_i_NR,
        },
    [XTRG_SIG_o_GROUP] =
        {
            .sig_nr = SIG_o_NR,
            .smon_idx = 20,
        },
    [XTRG_IO_o_GROUP] =
        {
            .sig_nr = IO_o_NR,
            .smon_idx = 96,
        },
    [XTRG_SYNC_o_GROUP] =
        {
            .sig_nr = SYNC_o_NR,
            .smon_idx = 0,
        },
    [XTRG_SSIG_GROUP] =
        {
            .sig_nr = SSIG_NR,
            .smux_idx = 0,
            .smon_idx = 8,
            .syncmux_idx = 8,
        },
};

/**
 * @brief sdrv xtrg write reg.
 *
 * @param[in] base reg base
 * @param[in] val reg val
 * @param[in] reg reg offset
 */
static inline void sdrv_xtrg_lld_writel(paddr_t base, uint32_t val,
                                        uint32_t reg)
{
    writel(val, base + reg);
}

/**
 * @brief sdrv xtrg read reg.
 *
 * @param[in] base reg base
 * @param[in] reg  reg offset
 */
static inline uint32_t sdrv_xtrg_lld_readl(paddr_t base, uint32_t reg)
{
    return readl(base + reg);
}

/**
 * @brief sdrv xtrg modify reg.
 *
 * @param[in] base reg base
 * @param[in] reg  reg offset
 * @param[in] start  start bit
 * @param[in] width  width
 * @param[in] val   reg val
 */
static inline void sdrv_xtrg_lld_modifyl(paddr_t base, uint32_t reg,
                                         uint32_t start, uint32_t width,
                                         uint32_t val)
{
    RMWREG32(base + reg, start, width, val);
}

/**
 * @brief sdrv xtrg int enable.
 *
 * @param[in] xtrg dev base
 * @param[in] int_id int id
 */
void sdrv_xtrg_lld_int_enable(paddr_t base, uint32_t int_id)
{
    uint32_t sta_en, sig_en;

    sta_en = sdrv_xtrg_lld_readl(base, FUNC_INT_STA_EN);
    sta_en |= int_id;
    sdrv_xtrg_lld_writel(base, sta_en, FUNC_INT_STA_EN);

    sig_en = sdrv_xtrg_lld_readl(base, FUNC_INT_SIG_EN);
    sig_en |= int_id;
    sdrv_xtrg_lld_writel(base, sig_en, FUNC_INT_SIG_EN);
}

/**
 * @brief sdrv xtrg int disable.
 *
 * @param[in] xtrg dev base
 * @param[in] int_id int id
 */
void sdrv_xtrg_lld_int_disable(paddr_t base, uint32_t int_id)
{
    uint32_t sta_en, sig_en;

    sta_en = sdrv_xtrg_lld_readl(base, FUNC_INT_STA_EN);
    sta_en &= ~(int_id);
    sdrv_xtrg_lld_writel(base, sta_en, FUNC_INT_STA_EN);

    sig_en = sdrv_xtrg_lld_readl(base, FUNC_INT_SIG_EN);
    sig_en &= ~(int_id);
    sdrv_xtrg_lld_writel(base, sig_en, FUNC_INT_SIG_EN);
}

/**
 * @brief sdrv xtrg get int status.
 *
 * @param[in] base xtrg base
 * @return FUNC_INT_STA status bitmap
 */
uint32_t sdrv_xtrg_lld_handle_irq(paddr_t base)
{
    return sdrv_xtrg_lld_readl(base, FUNC_INT_STA);
}

/**
 * @brief sdrv xtrg clear int status.
 *
 * @param[in] base xtrg base
 * @param[in] state func int state
 */
void sdrv_xtrg_lld_clear_irq(paddr_t base, uint32_t state)
{
    sdrv_xtrg_lld_writel(base, state, FUNC_INT_STA);
}

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
void sdrv_xtrg_lld_sync_enable(paddr_t base, xtrg_sig_t sig, bool enable)
{
    sdrv_xtrg_i_group_e group_id = XTRG_i_GROUP(sig);
    sdrv_xtrg_sig_group_t *group = &sdrv_xtrg_sig_groups[group_id];
    uint32_t i = XTRG_i_SIG(sig);

    sdrv_xtrg_lld_modifyl(base, group->sync_dis_off + i / 32, i % 32, 1,
                          !enable);
}

/**
 * @brief Configure I/O filters.
 *
 * @param[in] base   xTRG base.
 * @param[in] sig    The input signal to enable or disable debouncing for.
 * @param[in] config I/O filter config.
 */
void sdrv_xtrg_lld_io_filter_config(paddr_t base, xtrg_sig_t sig,
                                    const xtrg_io_filter_config_t *config,
                                    bool enable)
{
    uint32_t val = 0;

    val = ((config->sampling_intvl - 1) << 12) |
          ((config->neg_edge_intvl - 2) << 8) |
          ((config->pos_edge_intvl - 2) << 4) | (config->edge << 1) | enable;
    sdrv_xtrg_lld_writel(base, val, IO_FLAT(XTRG_i_SIG(sig)));
}

/**
 * @brief Trigger SWT (software trigger) status.
 *
 * @param[in] base   xTRG base.
 * @param[in] swt    xTRG swt index.
 * @param[in] config swt config.
 */
void sdrv_xtrg_lld_swt_trigger(paddr_t base, xtrg_swt_sel_e swt,
                               xtrg_swt_event_e event)
{
    /* {S,C,T,P} values */
    static const uint32_t sctp[] = {
        [XTRG_SWT_EVENT_SET] = 0x8,
        [XTRG_SWT_EVENT_CLEAR] = 0x4,
        [XTRG_SWT_EVENT_TRIGGER] = 0x2,
        [XTRG_SWT_EVENT_PULSE] = 0x1,
    };

    sdrv_xtrg_lld_modifyl(base, SW_TRG_CTRL, 4 * XTRG_i_SIG(swt), 4,
                          sctp[event]);
}

/**
 * @brief get swt status.
 *
 * @param[in] base   xTRG base.
 * @param[in] swt    xTRG swt index.
 */
bool sdrv_xtrg_lld_swt_status(paddr_t base, xtrg_swt_sel_e swt)
{
    return !!(sdrv_xtrg_lld_readl(base, SW_TRG_STATUS) &
              (1 << XTRG_i_SIG(swt)));
}

/**
 * @brief Set SWT pulse width.
 *
 * @param[in] base   xTRG base.
 * @param[in] swt    xTRG swt index.
 * @param[in] width  xTRG swt pulse width.
 */
void sdrv_xtrg_lld_swt_set_pulse_width(paddr_t base, xtrg_swt_sel_e swt,
                                       uint32_t width)
{
    sdrv_xtrg_lld_modifyl(base, SW_TRG_PULSE_WIDTH(XTRG_i_SIG(swt)),
                          16 * (XTRG_i_SIG(swt) % 2), 16, width & 0x0000FFFFul);
}

/**
 * @brief Configure SSE (Signal Synthesis Engine).
 * Each SSE has 5 input signals, in[4:0], and one value register SSE_REG. The
 * output ssig value is ssig = sse_reg[in].
 *
 * @param[in] base    xTRG base.
 * @param[in] config  xTRG sse config.
 */
void sdrv_xtrg_lld_sse5_config(paddr_t base, const xtrg_sse5_config_t *config)
{
    uint32_t sel03 = 0, sel4 = 0;
    uint32_t ctrl = 0;

    for (int i = 0; i < 5; i++) {
        sdrv_xtrg_i_group_e group_id = XTRG_i_GROUP(config->in[i].sig);
        uint32_t idx = XTRG_i_SIG(config->in[i].sig);

        if (i < 4) {
            sel03 |= (sdrv_xtrg_sig_groups[group_id].sse_idx + idx) << (i * 8);
        } else {
            sel4 = sdrv_xtrg_sig_groups[group_id].sse_idx + idx;
        }

        ctrl |= (config->in[i].edge_detect << (1 + i)) |
                (config->in[i].edge << (6 + 2 * i));
    }
    sdrv_xtrg_lld_writel(base, sel03, SSE_IN_0_3_SEL(config->ssm_sel));
    sdrv_xtrg_lld_writel(base, sel4, SSE_IN_4_SEL(config->ssm_sel));
    sdrv_xtrg_lld_writel(base, config->synthesis_val, SSE_REG(config->ssm_sel));
    sdrv_xtrg_lld_modifyl(base, SSE_CTRL(config->ssm_sel), 0, 16, ctrl);
}

/**
 * @brief Enable or disable SSE5.
 *
 * @param[in] base    xTRG base.
 * @param[in] enable  Enable or disable sse5.
 * @param[in] config  xTRG sse config.
 */
void sdrv_xtrg_lld_sse5_enable(paddr_t base, bool enable,
                               const xtrg_sse5_config_t *config)
{
    if (enable && config != NULL) {
        sdrv_xtrg_lld_sse5_config(base, config);
    }
    sdrv_xtrg_lld_modifyl(base, SSE_CTRL(config->ssm_sel), 0, 1,
                          (uint32_t)enable);
}

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
void sdrv_xtrg_lld_smux_config(paddr_t base, xtrg_sig_t sig_i, xtrg_sig_t sig_o)
{
    if (XTRG_SIG_o_GROUP == XTRG_i_GROUP(sig_o)) {
        sdrv_xtrg_lld_writel(
            base,
            sdrv_xtrg_sig_groups[XTRG_i_GROUP(sig_i)].smux_idx +
                XTRG_i_SIG(sig_i),
            SIG_O_SEL(XTRG_i_SIG(sig_o)));
    } else if (XTRG_IO_o_GROUP == XTRG_i_GROUP(sig_o)) {
        sdrv_xtrg_lld_writel(
            base,
            sdrv_xtrg_sig_groups[XTRG_i_GROUP(sig_i)].smux_idx +
                XTRG_i_SIG(sig_i),
            IO_O_SEL(XTRG_i_SIG(sig_o)));
    }
}

/**
 * @brief Enable or disable I/O output.
 *
 * @param[in] base    xTRG base.
 * @param[in] sig_o   xTRG output signal.
 * @param[in] enable  Enable or disable io out.
 */
void sdrv_xtrg_lld_io_out_enable(paddr_t base, xtrg_sig_t sig_o, bool enable)
{
    sdrv_xtrg_lld_modifyl(base, IO_OUT_EN(XTRG_i_SIG(sig_o)),
                          XTRG_i_SIG(sig_o) % 32, 1, (uint32_t)enable);
}

/**
 * @brief Configure IO output, muxing SSIG_i,TRG_i,SIG_i,IO_i to IO_o.
 *
 * @param[in] base    xTRG base.
 * @param[in] sig_i   xTRG input signal.
 * @param[in] sig_o   xTRG output signal.
 */
void sdrv_xtrg_lld_io_out_config(paddr_t base, xtrg_sig_t sig_i,
                                 xtrg_sig_t sig_o)
{
    sdrv_xtrg_lld_writel(base,
                         sdrv_xtrg_sig_groups[XTRG_i_GROUP(sig_i)].smux_idx +
                             XTRG_i_SIG(sig_i),
                         IO_O_SEL(XTRG_i_SIG(sig_o)));
}

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
                                  xtrg_sig_t sig_o)
{
    sdrv_xtrg_lld_writel(base,
                         sdrv_xtrg_sig_groups[XTRG_i_GROUP(sig_i)].syncmux_idx +
                             XTRG_i_SIG(sig_i),
                         SYNC_O_SEL(XTRG_i_SIG(sig_o)));
}

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
                                    const xtrg_tmux_drt_config_t *config)
{
    uint32_t trg_sel, tid_sel;
    uint32_t decode_sel[2] = {0};

    /* Input order is TRG_i, then SSIG. */
    if (XTRG_TRG_i_GROUP == XTRG_i_GROUP(config->trigger_sig))
        trg_sel = XTRG_i_SIG(config->trigger_sig);
    else
        trg_sel = TRG_i_NR + XTRG_i_SIG(config->trigger_sig);

    tid_sel = XTRG_i_SIG(config->tid);

    sdrv_xtrg_lld_writel(base, (trg_sel << 3) + tid_sel,
                         TMUX0_DRT_CTRL(config->tmux0_drt_sel));

    for (int i = 0; i < 8; i++) {
        if (i < 4) {
            decode_sel[0] |= config->target_lut[i] << (i * 8);
        } else {
            decode_sel[1] |= config->target_lut[i] << ((i - 4) * 8);
        }
    }
    sdrv_xtrg_lld_writel(base, decode_sel[0],
                         TMUX0_DRT_DECODE_SEL(config->tmux0_drt_sel, 1));
    sdrv_xtrg_lld_writel(base, decode_sel[1],
                         TMUX0_DRT_DECODE_SEL(config->tmux0_drt_sel, 2));
    sdrv_xtrg_lld_modifyl(base, TMUX_DEC_EN, config->tmux0_drt_sel, 1,
                          1); // TMUX_DEC_EN
}

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
                                      const xtrg_tmux_indrt_config_t *config)
{
    uint32_t trg_sel;
    uint32_t decode_sel[2] = {0};

    /* Input order is TRG_i, then SSIG. */
    if (XTRG_TRG_i_GROUP == XTRG_i_GROUP(config->trigger_sig))
        trg_sel = XTRG_i_SIG(config->trigger_sig);
    else
        trg_sel = TRG_i_NR + XTRG_i_SIG(config->trigger_sig);

    sdrv_xtrg_lld_writel(
        base, (config->term_tid << 16) | (trg_sel << 3) | config->term_val,
        TMUX0_INDRT_CTRL(config->tmux0_indrt_sel));

    for (int i = 0; i < 8; i++) {
        if (i < 4) {
            decode_sel[0] |= config->target_lut[i] << (i * 8);
        } else {
            decode_sel[1] |= config->target_lut[i] << ((i - 4) * 8);
        }
    }
    sdrv_xtrg_lld_writel(base, decode_sel[0],
                         TMUX0_INDRT_DECODE_SEL(config->tmux0_indrt_sel, 1));
    sdrv_xtrg_lld_writel(base, decode_sel[1],
                         TMUX0_INDRT_DECODE_SEL(config->tmux0_indrt_sel, 2));

    sdrv_xtrg_lld_writel(base, config->tid_pool[0],
                         TMUX0_INDRT_TID_POOL(config->tmux0_indrt_sel, 1));
    sdrv_xtrg_lld_writel(base, config->tid_pool[1],
                         TMUX0_INDRT_TID_POOL(config->tmux0_indrt_sel, 2));
    sdrv_xtrg_lld_modifyl(base, TMUX_DEC_EN, config->tmux0_indrt_sel + 8, 1,
                          1); // TMUX_DEC_EN
}

/**
 * @brief Configure SMON (Signal Monitor).
 * The SMON monitors the input and output signal groups
 * First level select 32 input signals and 32 output signals.
 * Second level select inputs from output of 1st level
 *
 * @param[in] base     xTRG base.
 * @param[in] config   smon configuration.
 */
void sdrv_xtrg_lld_smon_config(paddr_t base, const xtrg_smon_config_t *config)
{
    sdrv_xtrg_i_group_e sig_in_group = XTRG_i_GROUP(config->sig_in_sel);
    sdrv_xtrg_i_group_e sig_out_group = XTRG_i_GROUP(config->sig_out_sel);

    uint32_t sig_in_idx = sdrv_xtrg_sig_groups[sig_in_group].smon_idx +
                          XTRG_i_SIG(config->sig_in_sel);
    uint32_t sig_out_idx = sdrv_xtrg_sig_groups[sig_out_group].smon_idx +
                           XTRG_i_SIG(config->sig_out_sel);

    sdrv_xtrg_lld_writel(base, (sig_out_idx << 8) | sig_in_idx,
                         SMON_FIRST_MUX_SELECT(config->smon_sel));
    sdrv_xtrg_lld_writel(base,
                         (config->change_pol_in0 << 15) |
                             (config->sig_input1_sel << 8) |
                             config->sig_input0_sel,
                         SMON_SECOND_MUX_SELECT(config->smon_sel));
}

/**
 * @brief Enable or disable SMON.
 *
 * @param[in] base     xTRG base.
 * @param[in] enable   Enable or disable smon.
 * @param[in] config   smon configuration.
 */
void sdrv_xtrg_lld_smon_enable(paddr_t base, bool enable,
                               const xtrg_smon_config_t *config)
{
    if (enable && config != NULL) {
        sdrv_xtrg_lld_smon_config(base, config);
    }

    sdrv_xtrg_lld_modifyl(base, SMON_EN, config->smon_sel, 1, (uint32_t)enable);
}

/**
 * @brief Configure SWDT Event Check.(Signal Watch Dog Timer).
 *
 * @param[in] base     xTRG base.
 * @param[in] config   swdt configuration.
 */
void sdrv_xtrg_lld_swdt_event_check_config(
    paddr_t base, const xtrg_swdt_event_check_config_t *config)
{
    sdrv_xtrg_lld_writel(base, config->swdt_sig_sel,
                         SWDT_IN_SEL(config->swdt_sel));
    sdrv_xtrg_lld_modifyl(base, SWDT_EVENT_CHECK(config->swdt_sel), 16, 16,
                          config->swdt_event_check_window);
    sdrv_xtrg_lld_modifyl(base, SWDT_EVENT_CHECK(config->swdt_sel), 1, 3,
                          config->event_trg_mode);
}

/**
 * @brief Enable or disable SWDT Event Check.
 *
 * @param[in] base     xTRG base.
 * @param[in] enable   Enable or disable swdt.
 * @param[in] config   swdt configuration.
 */
void sdrv_xtrg_lld_swdt_event_check_enable(
    paddr_t base, bool enable, const xtrg_swdt_event_check_config_t *config)
{
    if (enable && config != NULL) {
        sdrv_xtrg_lld_swdt_event_check_config(base, config);
    }
    sdrv_xtrg_lld_modifyl(base, SWDT_EVENT_CHECK(config->swdt_sel), 0, 1,
                          (uint32_t)enable);
}

/**
 * @brief Configure SWDT Pulse Check.(Signal Watch Dog Timer).
 *
 * @param[in] base     xTRG base.
 * @param[in] config   swdt configuration.
 */
void sdrv_xtrg_lld_swdt_pulse_check_config(
    paddr_t base, const xtrg_swdt_pulse_check_config_t *config)
{
    sdrv_xtrg_lld_writel(base, config->swdt_sig_sel,
                         SWDT_IN_SEL(config->swdt_sel));
    sdrv_xtrg_lld_modifyl(base, SWDT_PULSE_CHECK(config->swdt_sel), 1, 1,
                          config->swdt_pulse_check_edge);
    sdrv_xtrg_lld_modifyl(base, SWDT_PULSE_CHECK(config->swdt_sel), 2, 1,
                          config->swdt_pulse_check_mode);
    sdrv_xtrg_lld_modifyl(base, SWDT_PULSE_CHECK_WINDOW(config->swdt_sel), 16,
                          16, config->swdt_max_check_window);
    sdrv_xtrg_lld_modifyl(base, SWDT_PULSE_CHECK_WINDOW(config->swdt_sel), 0,
                          16, config->swdt_min_check_window);
}

/**
 * @brief Enable or disable SWDT Pulse Check.
 *
 * @param[in] base     xTRG base.
 * @param[in] swdt_id  swdt id (0~3).
 * @param[in] enable   Enable or disable swdt.
 * @param[in] config   swdt configuration.
 */
void sdrv_xtrg_lld_swdt_pulse_check_enable(
    paddr_t base, bool enable, const xtrg_swdt_pulse_check_config_t *config)
{
    if (enable && config != NULL) {
        sdrv_xtrg_lld_swdt_pulse_check_config(base, config);
    }
    sdrv_xtrg_lld_modifyl(base, SWDT_PULSE_CHECK(config->swdt_sel), 0, 1,
                          (uint32_t)enable);
}

/**
 * @brief Configure DMA0/1.
 *
 * @param[in] base    xTRG base.
 * @param[in] config  xTRG dma configuration.
 */
void sdrv_xtrg_lld_dma_config(paddr_t base, const xtrg_dma_config_t *config)
{
    if (XTRG_DMA0 == config->dma_sel) {
        sdrv_xtrg_lld_writel(base,
                             (config->ssm_sel << 2) | (config->ssig_mode << 1),
                             DMAC0_CTRL);
    } else {
        sdrv_xtrg_lld_writel(base,
                             (config->ssm_sel << 2) | (config->ssig_mode << 1),
                             DMAC1_CTRL);
    }
}

/**
 * @brief Enable or disable DMA0/1.
 *
 * @param[in] base     xTRG base.
 * @param[in] config   xTRG dma configuration.
 * @param[in] enable   Enable or disable dma.
 */
void sdrv_xtrg_lld_dma_enable(paddr_t base, const xtrg_dma_config_t *config,
                              bool enable)
{
    if (enable && config != NULL) {
        sdrv_xtrg_lld_dma_config(base, config);
    }
    if (XTRG_DMA0 == config->dma_sel) {
        sdrv_xtrg_lld_modifyl(base, DMAC0_CTRL, 0, 1, (uint32_t)enable);
    } else {
        sdrv_xtrg_lld_modifyl(base, DMAC1_CTRL, 0, 1, (uint32_t)enable);
    }
}

/**
 * @brief Read tmux adc result.
 *
 * @param[in] base     xTRG base.
 * @param[in] tmux_id  xTRG tmux index.
 * @param[in] adc_id   xTRG adc index.
 *
 * @return adc result
 */
uint32_t sdrv_xtrg_lld_adc_rslt(paddr_t base, xtrg_tmux_sel_e tmux_id,
                                xtrg_adc_sel_e adc_id)
{
    return sdrv_xtrg_lld_readl(base, TMUX_ADC_RSLT(adc_id, tmux_id));
}

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
                                 xtrg_acmp_sel_e acmp_id)
{
    return sdrv_xtrg_lld_readl(base, TMUX_ACMP_RSLT(acmp_id, tmux_id));
}
