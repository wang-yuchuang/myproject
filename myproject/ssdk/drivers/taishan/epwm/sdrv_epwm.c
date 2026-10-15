/*
 * @file: sdrv_epwm.c
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description
 *
 * @Revision History:
 * ----------------
 */
#define LOG_TAG "sdrv_epwm.c "
#include <regs_base.h>
#include <stdio.h>
#include <string.h>
#include <types.h>

#include "sdrv_epwm.h"

#include "cmsis_os2.h"

void sdrv_epwm_lld_int_enable(paddr_t base, uint32_t val)
{
    sdrv_epwm_lld_int_sta_enable(base, val);
    sdrv_epwm_lld_int_sig_enable(base, val);
}

void sdrv_epwm_lld_int_sta_enable(paddr_t base, uint32_t val)
{
    uint32_t sta = sdrv_epwm_lld_int_sta_en(base);
    writel(sta | val, base + EPWM_INT_STA_EN_OFF);
}

void sdrv_epwm_lld_int_sta_dis(paddr_t base, uint32_t val)
{
    uint32_t sta = sdrv_epwm_lld_int_sta_en(base);
    writel(sta & (~val), base + EPWM_INT_STA_EN_OFF);
}

void sdrv_epwm_lld_int_sig_enable(paddr_t base, uint32_t val)
{
    uint32_t sta = sdrv_epwm_lld_int_sig_en(base);
    writel(sta | val, base + EPWM_INT_SIG_EN_OFF);
}

void sdrv_epwm_lld_int_sig_dis(paddr_t base, uint32_t val)
{
    uint32_t sta = sdrv_epwm_lld_int_sig_en(base);
    writel(sta & (~val), base + EPWM_INT_SIG_EN_OFF);
}

void sdrv_epwm_lld_cor_err_int_enable(paddr_t base, uint32_t val)
{
    sdrv_epwm_lld_cor_err_int_sta_enable(base, val);
    sdrv_epwm_lld_cor_err_int_sig_enable(base, val);
}

void sdrv_epwm_lld_cor_err_int_sta_enable(paddr_t base, uint32_t val)
{
    uint32_t sta = sdrv_epwm_lld_cor_err_int_sta_en(base);
    writel(sta | val, base + EPWM_COR_ERR_INT_STA_EN_OFF);
}

void sdrv_epwm_lld_cor_err_int_sta_dis(paddr_t base, uint32_t val)
{
    uint32_t sta = sdrv_epwm_lld_cor_err_int_sta_en(base);
    writel(sta & (~val), base + EPWM_COR_ERR_INT_STA_EN_OFF);
}

void sdrv_epwm_lld_cor_err_int_sig_enable(paddr_t base, uint32_t val)
{
    uint32_t sta = sdrv_epwm_lld_cor_err_int_sig_en(base);
    writel(sta | val, base + EPWM_COR_ERR_INT_SIG_EN_OFF);
}

void sdrv_epwm_lld_cor_err_int_sig_dis(paddr_t base, uint32_t val)
{
    uint32_t sta = sdrv_epwm_lld_cor_err_int_sig_en(base);
    writel(sta & (~val), base + EPWM_COR_ERR_INT_SIG_EN_OFF);
}

void sdrv_epwm_lld_unc_err_int_enable(paddr_t base, uint32_t val)
{
    sdrv_epwm_lld_unc_err_int_sta_enable(base, val);
    sdrv_epwm_lld_unc_err_int_sig_enable(base, val);
}

void sdrv_epwm_lld_unc_err_int_sta_enable(paddr_t base, uint32_t val)
{
    uint32_t sta = sdrv_epwm_lld_unc_err_int_sta_en(base);
    writel(sta | val, base + EPWM_UNC_ERR_INT_STA_EN_OFF);
}

void sdrv_epwm_lld_unc_err_int_sta_dis(paddr_t base, uint32_t val)
{
    uint32_t sta = sdrv_epwm_lld_unc_err_int_sta_en(base);
    writel(sta & (~val), base + EPWM_UNC_ERR_INT_STA_EN_OFF);
}

void sdrv_epwm_lld_unc_err_int_sig_enable(paddr_t base, uint32_t val)
{
    uint32_t sta = sdrv_epwm_lld_unc_err_int_sig_en(base);
    writel(sta | val, base + EPWM_UNC_ERR_INT_SIG_EN_OFF);
}

void sdrv_epwm_lld_unc_err_int_sig_dis(paddr_t base, uint32_t val)
{
    uint32_t sta = sdrv_epwm_lld_unc_err_int_sig_en(base);
    writel(sta & (~val), base + EPWM_UNC_ERR_INT_SIG_EN_OFF);
}

void sdrv_epwm_lld_unc_int_en(paddr_t base, uint32_t int_id)
{
    uint32_t sta_en, sig_en;
    sta_en = sdrv_epwm_lld_unc_err_int_sta_en(base);
    sta_en |= SDRV_EPWM_STA_MASK(int_id);
    writel(sta_en, base + EPWM_UNC_ERR_INT_STA_EN_OFF);

    sig_en = sdrv_epwm_lld_unc_err_int_sig_en(base);
    sig_en |= SDRV_EPWM_STA_MASK(int_id);
    writel(sig_en, base + EPWM_UNC_ERR_INT_SIG_EN_OFF);
}

void sdrv_epwm_lld_fusa_cor_err_int_sta_enable(paddr_t base, uint32_t val)
{
    uint32_t sta = sdrv_epwm_lld_fusa_cor_err_int_sta_en(base);
    writel(sta | val, base + EPWM_FUSA_COR_ERR_INT_STA_EN_OFF);
}

void sdrv_epwm_lld_fusa_cor_err_int_sta_dis(paddr_t base, uint32_t val)
{
    uint32_t sta = sdrv_epwm_lld_fusa_cor_err_int_sta_en(base);
    writel(sta & (~val), base + EPWM_FUSA_COR_ERR_INT_STA_EN_OFF);
}

void sdrv_epwm_lld_fusa_cor_err_int_sig_enable(paddr_t base, uint32_t val)
{
    uint32_t sta = sdrv_epwm_lld_fusa_cor_err_int_sig_en(base);
    writel(sta | val, base + EPWM_FUSA_COR_ERR_INT_SIG_EN_OFF);
}

void sdrv_epwm_lld_fusa_cor_err_int_sig_dis(paddr_t base, uint32_t val)
{
    uint32_t sta = sdrv_epwm_lld_fusa_cor_err_int_sig_en(base);
    writel(sta & (~val), base + EPWM_FUSA_COR_ERR_INT_SIG_EN_OFF);
}

void sdrv_epwm_lld_fusa_cor_err_int_enable(paddr_t base, uint32_t val)
{
    sdrv_epwm_lld_fusa_cor_err_int_sta_enable(base, val);
    sdrv_epwm_lld_fusa_cor_err_int_sig_enable(base, val);
}

void sdrv_epwm_lld_fusa_unc_err_int_sta_enable(paddr_t base, uint32_t val)
{
    uint32_t sta = sdrv_epwm_lld_fusa_unc_err_int_sta_en(base);
    writel(sta | val, base + EPWM_FUSA_UNC_ERR_INT_STA_EN_OFF);
}

void sdrv_epwm_lld_fusa_unc_err_int_sta_dis(paddr_t base, uint32_t val)
{
    uint32_t sta = sdrv_epwm_lld_fusa_unc_err_int_sta_en(base);
    writel(sta & (~val), base + EPWM_FUSA_UNC_ERR_INT_STA_EN_OFF);
}

void sdrv_epwm_lld_fusa_unc_err_int_sig_enable(paddr_t base, uint32_t val)
{
    uint32_t sta = sdrv_epwm_lld_fusa_unc_err_int_sig_en(base);
    writel(sta | val, base + EPWM_FUSA_UNC_ERR_INT_SIG_EN_OFF);
}

void sdrv_epwm_lld_fusa_unc_err_int_sig_dis(paddr_t base, uint32_t val)
{
    uint32_t sta = sdrv_epwm_lld_fusa_unc_err_int_sig_en(base);
    writel(sta & (~val), base + EPWM_FUSA_UNC_ERR_INT_SIG_EN_OFF);
}

void sdrv_epwm_lld_fusa_unc_err_int_enable(paddr_t base, uint32_t val)
{
    sdrv_epwm_lld_fusa_unc_err_int_sta_enable(base, val);
    sdrv_epwm_lld_fusa_unc_err_int_sig_enable(base, val);
}

void sdrv_epwm_lld_fusa_unc_err_int_sta_1_enable(paddr_t base, uint32_t val)
{
    uint32_t sta = sdrv_epwm_lld_fusa_unc_err_int_sta_1_en(base);
    writel(sta | val, base + EPWM_FUSA_UNC_ERR_INT_STA_EN_1_OFF);
}

void sdrv_epwm_lld_fusa_unc_err_int_sta_1_dis(paddr_t base, uint32_t val)
{
    uint32_t sta = sdrv_epwm_lld_fusa_unc_err_int_sta_1_en(base);
    writel(sta & (~val), base + EPWM_FUSA_UNC_ERR_INT_STA_EN_1_OFF);
}

void sdrv_epwm_lld_fusa_unc_err_int_sig_1_enable(paddr_t base, uint32_t val)
{
    uint32_t sta = sdrv_epwm_lld_fusa_unc_err_int_sig_1_en(base);
    writel(sta | val, base + EPWM_FUSA_UNC_ERR_INT_SIG_EN_1_OFF);
}

void sdrv_epwm_lld_fusa_unc_err_int_sig_1_dis(paddr_t base, uint32_t val)
{
    uint32_t sta = sdrv_epwm_lld_fusa_unc_err_int_sig_1_en(base);
    writel(sta & (~val), base + EPWM_FUSA_UNC_ERR_INT_SIG_EN_1_OFF);
}

void sdrv_epwm_lld_fusa_unc_err_int_1_enable(paddr_t base, uint32_t val)
{
    sdrv_epwm_lld_fusa_unc_err_int_sta_1_enable(base, val);
    sdrv_epwm_lld_fusa_unc_err_int_sig_1_enable(base, val);
}

void sdrv_epwm_lld_reg_parity_err_int_sta_enable(paddr_t base, uint32_t val)
{
    uint32_t sta = sdrv_epwm_lld_reg_parity_err_int_sta_en(base);
    writel(sta | val, base + EPWM_REG_PARITY_ERR_INT_STAT_EN_OFF);
}

void sdrv_epwm_lld_reg_parity_err_int_sta_dis(paddr_t base, uint32_t val)
{
    uint32_t sta = sdrv_epwm_lld_reg_parity_err_int_sta_en(base);
    writel(sta & (~val), base + EPWM_REG_PARITY_ERR_INT_STAT_EN_OFF);
}

void sdrv_epwm_lld_reg_parity_err_int_sig_enable(paddr_t base, uint32_t val)
{
    uint32_t sta = sdrv_epwm_lld_reg_parity_err_int_sig_en(base);
    writel(sta | val, base + EPWM_REG_PARITY_ERR_INT_SIG_EN_OFF);
}

void sdrv_epwm_lld_reg_parity_err_int_sig_dis(paddr_t base, uint32_t val)
{
    uint32_t sta = sdrv_epwm_lld_reg_parity_err_int_sig_en(base);
    writel(sta & (~val), base + EPWM_REG_PARITY_ERR_INT_SIG_EN_OFF);
}

void sdrv_epwm_lld_reg_parity_err_int_enable(paddr_t base, uint32_t val)
{
    sdrv_epwm_lld_reg_parity_err_int_sta_enable(base, val);
    sdrv_epwm_lld_reg_parity_err_int_sig_enable(base, val);
}

void sdrv_epwm_lld_clk_config_div_num(paddr_t base, uint16_t div_num)
{
    RMWREG32(base + EPWM_CLK_CONFIG_OFF, 0, 16, div_num);
    RMWREG32(base + EPWM_CLK_CONFIG_OFF, 31, 1, true);
}

void sdrv_epwm_lld_clk_config(paddr_t base,
                              sdrv_epwm_clk_config_src_clk_sel_e src_clk_sel,
                              uint16_t div_num)
{
    RMWREG32(base + EPWM_CLK_CONFIG_OFF, 16, 2, src_clk_sel);

    sdrv_epwm_lld_clk_config_div_num(base, div_num);
}

void sdrv_epwm_lld_clk_mon_en(paddr_t base, bool en_p, bool en_n)
{
    RMWREG32(base + EPWM_CLK_MON_EN_OFF, 0, 1, !en_p);
    RMWREG32(base + EPWM_CLK_MON_EN_OFF, 1, 1, en_n);
    RMWREG32(base + EPWM_CLK_MON_EN_OFF, 2, 1, true);
}

void sdrv_epwm_lld_chn_dma_ctrl_cmp(paddr_t base, sdrv_epwm_channel_e chnl,
                                    bool cmp_x_out_mode,
                                    sdrv_epwm_cmp_mode_e cmp_x_mode)
{
    RMWREG32(base + EPWM_CHN_DMA_CTRL_OFF, 16 + chnl, 1, cmp_x_out_mode);
    RMWREG32(base + EPWM_CHN_DMA_CTRL_OFF, 20 + chnl * 2, 2, cmp_x_mode);
}

void sdrv_epwm_lld_chn_dma_ctrl_two_chn_xx_mode(paddr_t base,
                                                sdrv_epwm_channel_e chnl,
                                                bool en)
{
    if (chnl == SDRV_EPWM_CHANNEL_A || chnl == SDRV_EPWM_CHANNEL_C)
        RMWREG32(base + EPWM_CHN_DMA_CTRL_OFF, 29, 1, en);
    else if (chnl == SDRV_EPWM_CHANNEL_B || chnl == SDRV_EPWM_CHANNEL_D)
        RMWREG32(base + EPWM_CHN_DMA_CTRL_OFF, 30, 1, en);
}

void sdrv_epwm_lld_dma_wml(paddr_t base, sdrv_epwm_channel_e chnl, uint8_t val)
{
    if (chnl == SDRV_EPWM_CHANNEL_A)
        RMWREG32(base + EPWM_DMA_WML_OFF, 0, 6, val);
    else if (chnl == SDRV_EPWM_CHANNEL_B)
        RMWREG32(base + EPWM_DMA_WML_OFF, 8, 5, val);
    else if (chnl == SDRV_EPWM_CHANNEL_C)
        RMWREG32(base + EPWM_DMA_WML_OFF, 16, 4, val);
    else if (chnl == SDRV_EPWM_CHANNEL_D)
        RMWREG32(base + EPWM_DMA_WML_OFF, 24, 4, val);
}

void sdrv_epwm_lld_config_dma_ctl_cfg(paddr_t base, bool sig_mask, uint8_t val)
{
    RMWREG32(base + EPWM_CONFIG_DMA_CTL_OFF, 1, 1, sig_mask);
    RMWREG32(base + EPWM_CONFIG_DMA_CTL_OFF, 2, 6, val & 0x3f);
}

uint32_t sdrv_epwm_lld_fifo_sta_fifo(paddr_t base, sdrv_epwm_channel_e chnl)
{
    if (chnl == SDRV_EPWM_CHANNEL_A || chnl == SDRV_EPWM_CHANNEL_B)
        return readl(base + EPWM_FIFO_STA0_OFF);
    else if (chnl == SDRV_EPWM_CHANNEL_C || chnl == SDRV_EPWM_CHANNEL_D) {
        return readl(base + EPWM_FIFO_STA1_OFF);
    }
    return 0;
}

void sdrv_epwm_lld_cnt_cfg_set_trig_sel(
    paddr_t base, sdrv_epwm_cnt_gx_e cnt_gx,
    sdrv_epwm_cnt_cfg_set_upd_sel_e set_mode_sel,
    sdrv_epwm_event_trigger_mode_e set_trig_sel)
{
    RMWREG32(base + CNT_CFG(cnt_gx), 3, 2, set_mode_sel);
    RMWREG32(base + CNT_CFG(cnt_gx), 1, 2, set_trig_sel);
}

void sdrv_epwm_lld_cmp_config_clr_set(
    paddr_t base, sdrv_epwm_channel_e chnl,
    sdrv_epwm_event_trigger_mode_e cmp_set_trig_sel,
    sdrv_epwm_event_trigger_mode_e cmp_clr_trig_sel)
{

    RMWREG32(base + CMP_CONFIG(chnl), 10, 2, cmp_set_trig_sel);
    RMWREG32(base + CMP_CONFIG(chnl), 12, 2, cmp_clr_trig_sel);
}

void sdrv_epwm_lld_cmp_config(paddr_t base, sdrv_epwm_channel_e chnl,
                              sdrv_epwm_cmp_cfg_t *cfg)
{
    uint32_t val = cfg->con_mode << 0 | cfg->cnt_sel << 2 |
                   cfg->rld_trig_mode << 4 | cfg->hw_rld_mode << 5;

    RMWREG32(base + CMP_CONFIG(chnl), 0, 6, val);
    RMWREG32(base + CMP_CONFIG(chnl), 15, 1, cfg->sw_rld_mode);
    RMWREG32(base + CMP_CONFIG(chnl), 24, 8, cfg->refresh_intval);

    sdrv_epwm_lld_chn_dma_ctrl_cmp(base, chnl, cfg->cmp_x_out_mode,
                                   cfg->cmp_x_mode);
    sdrv_epwm_lld_cmp_event_out_mode(base, chnl, &(cfg->out_mode));
    sdrv_epwm_lld_cmp_pulse_wid0(base, chnl, &(cfg->wid0));
    sdrv_epwm_lld_cmp_pulse_wid1(base, chnl, &(cfg->wid1));
}

void sdrv_epwm_lld_cmp_event_out_mode(paddr_t base, sdrv_epwm_channel_e chnl,
                                      sdrv_epwm_cmp_event_out_t *out_mode)
{
    uint32_t val = out_mode->cmp00 << 0 | out_mode->cmp01 << 4 |
                   out_mode->cmp10 << 8 | out_mode->cmp11 << 12 |
                   out_mode->cmp0_ovf << 16 | out_mode->cmp1_ovf << 20;

    writel(val, base + CMP_EVENT_OUT_MODE(chnl));
}

void sdrv_epwm_lld_cmp_pulse_wid0(paddr_t base, sdrv_epwm_channel_e chnl,
                                  sdrv_epwm_cmp_pulse_wid0_t *wid)
{
    uint32_t val = (wid->cmp00 & 0xff) << 0 | (wid->cmp01 & 0xff) << 8 |
                   (wid->cmp10 & 0xff) << 16 | (wid->cmp11 & 0xff) << 24;
    writel(val, base + CMP_PULSE_WID0(chnl));
}

void sdrv_epwm_lld_cmp_pulse_wid1(paddr_t base, sdrv_epwm_channel_e chnl,
                                  sdrv_epwm_cmp_pulse_wid1_t *wid)
{
    uint16_t val = (wid->cmp1_ovf & 0xff) << 0 | (wid->cmp0_ovf & 0xff) << 8;
    writel(val, base + CMP_PULSE_WID1(chnl));
}

void sdrv_epwm_lld_cmp_dither(paddr_t base, sdrv_epwm_channel_e chnl,
                              uint8_t clip_rslt, uint16_t init_offset)
{
    RMWREG32(base + CMP_DITHER(chnl), 2, 4, clip_rslt);
    RMWREG32(base + CMP_DITHER(chnl), 16, 16, init_offset);
}

void sdrv_epwm_lld_cmp_ctrl_cmp_init_status(paddr_t base,
                                            sdrv_epwm_channel_e chnl,
                                            bool cmp0_init, bool cmp1_init)
{

    RMWREG32(base + EPWM_CMP_CTRL_OFF, chnl * 2 + 12, 1, cmp0_init);
    RMWREG32(base + EPWM_CMP_CTRL_OFF, chnl * 2 + 13, 1, cmp1_init);
}

void sdrv_epwm_lld_cmp_sse_ctrl_cfg(paddr_t base, sdrv_epwm_channel_e chnl,
                                    sdrv_epwm_cmp_sse_ctrl_t *cfg)
{
    uint32_t val =
        (cfg->sse_mode[0] << 1 | cfg->sse_mode[1] << 2 | cfg->sse_mode[2] << 3 |
         cfg->sse_mode[3] << 4 | cfg->sse_mode[4] << 5 | cfg->edge_sel[0] << 6 |
         cfg->edge_sel[1] << 8 | cfg->edge_sel[2] << 10 |
         cfg->edge_sel[3] << 12 | cfg->edge_sel[4] << 14) >>
        1;
    RMWREG32(base + EPWM_SSE(chnl), 1, 15, val & 0x7fff);
}

void sdrv_epwm_lld_cmp_input_sel(paddr_t base, sdrv_epwm_channel_e chnl,
                                 sdrv_epwm_input_sel_t *cfg)
{
    uint8_t val = cfg->set_sel | cfg->clr_sel << 4;
    RMWREG32(base + CMP_INPUT_SEL(chnl), 0, 8, val);
}

void sdrv_epwm_lld_ext_output_sel(paddr_t base, sdrv_epwm_input_sel_t *cfg)
{
    uint8_t val = cfg->set_sel | cfg->clr_sel << 4;
    RMWREG32(base + EPWM_EXT_OUTPUT_SEL_OFF, 0, 8, val);
}

void sdrv_epwm_lld_trigc_pol_inv(paddr_t base, sdrv_epwm_trigc_pol_inv_t *cfg)
{
    uint16_t val =
        cfg->trigc_pol_inv_cmp_a_set << 0 | cfg->trigc_pol_inv_cmp_a_clr << 1 |
        cfg->trigc_pol_inv_cmp_b_set << 2 | cfg->trigc_pol_inv_cmp_b_clr << 3 |
        cfg->trigc_pol_inv_cmp_c_set << 4 | cfg->trigc_pol_inv_cmp_c_clr << 5 |
        cfg->trigc_pol_inv_cmp_d_set << 6 | cfg->trigc_pol_inv_cmp_d_clr << 7 |
        cfg->trigc_pol_inv_cnt_g0_set << 8 |
        cfg->trigc_pol_inv_cnt_g0_clr << 9 |
        cfg->trigc_pol_inv_cnt_g1_set << 10 |
        cfg->trigc_pol_inv_cnt_g1_clr << 11 | cfg->trigc_pol_inv_ext_set << 12 |
        cfg->trigc_pol_inv_ext_clr << 13;

    RMWREG32(base + EPWM_TRIGC_POL_INV_OFF, 0, 14, val & 0x3fff);
}

void sdrv_epwm_lld_cmp_sw_trig(paddr_t base, sdrv_epwm_cmp_sw_trig_t *cfg)
{
    uint8_t val =
        cfg->cmp_sw_trig_cmp_a_set << 0 | cfg->cmp_sw_trig_cmp_b_set << 1 |
        cfg->cmp_sw_trig_cmp_c_set << 2 | cfg->cmp_sw_trig_cmp_d_set << 3 |
        cfg->cmp_sw_trig_cmp_a_clr << 4 | cfg->cmp_sw_trig_cmp_b_clr << 5 |
        cfg->cmp_sw_trig_cmp_c_clr << 6 | cfg->cmp_sw_trig_cmp_d_clr << 7;

    RMWREG32(base + EPWM_CMP_SW_TRIG_OFF, 0, 8, val);
}

void sdrv_epwm_lld_cnt_ext_sw_trig(paddr_t base,
                                   sdrv_epwm_cnt_ext_sw_trig_t *cfg)
{

    uint8_t val = cfg->cnt_ext_sw_trig_cnt_g0_set << 0 |
                  cfg->cnt_ext_sw_trig_cnt_g0_clr << 1 |
                  cfg->cnt_ext_sw_trig_cnt_g1_set << 2 |
                  cfg->cnt_ext_sw_trig_cnt_g1_clr << 3 |
                  cfg->cnt_ext_sw_trig_ext_set << 4 |
                  cfg->cnt_ext_sw_trig_ext_clr << 5;
    RMWREG32(base + EPWM_CNT_EXT_SW_TRIG_OFF, 0, 6, val);
}

void sdrv_epwm_lld_sw_trig_pulse(paddr_t base, sdrv_epwm_channel_e chnl,
                                 sdrv_epwm_sw_trig_pulse_width_t *cfg)
{
    uint32_t val1 = cfg->trig0_pulse_width << 0 | cfg->trig1_pulse_width << 16;
    uint32_t val2 = cfg->trig2_pulse_width << 0 | cfg->trig3_pulse_width << 16;
    if (chnl == SDRV_EPWM_CHANNEL_A || chnl == SDRV_EPWM_CHANNEL_B)
        writel(val1, base + EPWM_SW_TRIG_PULSE0_OFF);
    else if (chnl == SDRV_EPWM_CHANNEL_C || chnl == SDRV_EPWM_CHANNEL_D)
        writel(val2, base + EPWM_SW_TRIG_PULSE1_OFF);
}

void sdrv_epwm_lld_fault_flt(paddr_t base, sdrv_epwm_channel_e chnl,
                             sdrv_epwm_fault_flt_t *cfg)
{
    uint32_t val =
        (cfg->flt_edge_sel << 1 | (cfg->pos_band_wid - 2) << 4 |
         (cfg->neg_band_wid - 2) << 8 | (cfg->smpl_intval - 1) << 12) >>
        1;

    RMWREG32(base + FAULT_FLT(chnl), 1, 19, val & 0x7ffff);
}

void sdrv_epwm_lld_sync_dis(paddr_t base, sdrv_epwm_sync_dis_t *cfg)
{

    uint8_t val = cfg->sdrv_epwm_sync_dis_ext_set << 0 |
                  cfg->sdrv_epwm_sync_dis_ext_clr << 1 |
                  cfg->sdrv_epwm_sync_dis_fault0 << 2 |
                  cfg->sdrv_epwm_sync_dis_fault1 << 3 |
                  cfg->sdrv_epwm_sync_dis_fault2 << 4 |
                  cfg->sdrv_epwm_sync_dis_fault3 << 5;

    RMWREG32(base + EPWM_SYNC_DIS_OFF, 0, 6, val & 0x3f);
}

void sdrv_epwm_lld_cmp_fault_event_ctrl_config(
    paddr_t base, sdrv_epwm_channel_e chnl, sdrv_epwm_sub_channel_e sub_chnl,
    sdrv_epwm_cmp_fault_event_ctrl_t *cfg)
{
    uint8_t val = (cfg->fault_pol << 1 | cfg->fault_clr_mode << 2) >> 1;

    RMWREG32(base + CMP_FAULT_EVENT_CTRL(chnl), 1 + sub_chnl * 4, 2, val & 0x3);
}

void sdrv_epwm_lld_cmp_fault_event_fs_sta_set(paddr_t base,
                                              sdrv_epwm_channel_e chnl,
                                              bool cmp0_sta, bool cmp1_sta)
{
    uint8_t val = cmp0_sta | cmp1_sta << 1;
    RMWREG32(base + CMP_DTI_CTRL(chnl), 3, 2, val);
}

void sdrv_epwm_lld_cmp_prefin_pol(paddr_t base, sdrv_epwm_channel_e chnl,
                                  sdrv_epwm_cmp_prefin_pol_t *prefin_pol)
{
    RMWREG32(base + CMP_DTI_CTRL(chnl), 1, 1, prefin_pol->cmp0_pol);
    RMWREG32(base + CMP_DTI_CTRL(chnl), 2, 1, prefin_pol->cmp1_pol);
}

void sdrv_epwm_lld_cmp_dti_wid(paddr_t base, sdrv_epwm_channel_e chnl,
                               uint16_t dt0_wid, uint16_t dt1_wid)
{
    uint32_t val = dt0_wid << 0 | dt1_wid << 16;

    writel(val, base + CMP_DTI_WID(chnl));
}

#ifdef CONFIG_SDRV_EPWM_L

void sdrv_epwm_lld_center_align_cmp_int_enable(paddr_t base, uint32_t val)
{
    sdrv_epwm_lld_center_align_cmp_int_sta_enable(base, val);
    sdrv_epwm_lld_center_align_cmp_int_sig_enable(base, val);
}

void sdrv_epwm_lld_center_align_cmp_int_sta_enable(paddr_t base, uint32_t val)
{
    uint32_t sta = sdrv_epwm_lld_center_align_cmp_int_sta_en(base);
    writel(sta | val, base + EPWM_CENTER_ALIGN_CMP_INT_STA_EN_OFF);
}

void sdrv_epwm_lld_center_align_cmp_int_sta_dis(paddr_t base, uint32_t val)
{
    uint32_t sta = sdrv_epwm_lld_center_align_cmp_int_sta_en(base);
    writel(sta & (~val), base + EPWM_CENTER_ALIGN_CMP_INT_STA_EN_OFF);
}

void sdrv_epwm_lld_center_align_cmp_int_sig_enable(paddr_t base, uint32_t val)
{
    uint32_t sta = sdrv_epwm_lld_center_align_cmp_int_sta_en(base);
    writel(sta | val, base + EPWM_CENTER_ALIGN_CMP_INT_SIG_EN_OFF);
}

void sdrv_epwm_lld_center_align_cmp_int_sig_dis(paddr_t base, uint32_t val)
{
    uint32_t sta = sdrv_epwm_lld_center_align_cmp_int_sig_en(base);
    writel(sta & (~val), base + EPWM_CENTER_ALIGN_CMP_INT_SIG_EN_OFF);
}

void sdrv_epwm_lld_cmp_dti_idle_sta(paddr_t base, sdrv_epwm_channel_e chnl,
                                    bool cmp0_idle_status,
                                    bool cmp1_idle_status)
{
    uint8_t val = cmp0_idle_status | cmp1_idle_status << 1;
    RMWREG32(base + CMP_DTI_CTRL(chnl), 8, 2, val);
}

#endif
