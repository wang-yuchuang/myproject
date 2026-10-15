/**
 * @file sdrv_smc_drv.c
 * @brief Taishan SMC driver source.
 *
 * @copyright Copyright (c) 2022  Semidrive Semiconductor.
 *            All rights reserved.
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <driver/taishan/smc/sdrv_smc_drv.h>
#include <driver/clk.h>
#include <driver/taishan/clk/taishan_ckgen_drv.h>
#include <driver/irq.h>
#include <pmu_cfg.h>
#include <sdrv_pmu.h>
#include "sdrv_smc.h"
#include <regs_base.h>
#include <irq_num.h>
#include <reg.h>

extern uint32_t taishan_rstgen_lld_get_ss_rdy(uint32_t base);

static const paddr_t g_smc_base = APB_SMC_BASE;
static const uint32_t g_smc_int = SMC_SMC_INTR_NUM;

static int smc_irq_handle(uint32_t irq, void *arg)
{
    printf("%s: irq enter. \n", __func__);

    return 0;
}

static void sdrv_smc_lp_pwr_ctrl(uint32_t domain, uint32_t index, struct sdrv_smc_lp_ctrl *lp_ctrl)
{
    if (domain == SAF_DOMAIN) {
        if (lp_ctrl->hib_pd_enable) {
            sdrv_smc_lld_saf_lp_ctrl(g_smc_base, index, HIB_MODE, true);
        }

        if (lp_ctrl->sleep_pd_enable) {
            sdrv_smc_lld_saf_lp_ctrl(g_smc_base, index, SLP_MODE, true);
        }
    }
    else {
        if (lp_ctrl->hib_pd_enable) {
            sdrv_smc_lld_ap_lp_ctrl(g_smc_base, index, HIB_MODE, true);
        }

        if (lp_ctrl->sleep_pd_enable) {
            sdrv_smc_lld_ap_lp_ctrl(g_smc_base, index, SLP_MODE, true);
        }
    }
}

static void sdrv_smc_lp_ram_ctrl(uint32_t domain, uint32_t index, struct sdrv_smc_ram_lp_ctrl *ram_ctrl)
{
    if (domain == SAF_DOMAIN) {
        if (ram_ctrl->hib_ram_pd_enable) {
            sdrv_smc_lld_saf_ram_lp_ctrl(g_smc_base, index, HIB_MODE, true, ram_ctrl->hib_ram_lp_setting);
        }

        if (ram_ctrl->sleep_ram_pd_enable) {
            sdrv_smc_lld_saf_ram_lp_ctrl(g_smc_base, index, SLP_MODE, true, ram_ctrl->sleep_ram_lp_setting);
        }
    }
    else {
        if (ram_ctrl->hib_ram_pd_enable) {
            sdrv_smc_lld_ap_ram_lp_ctrl(g_smc_base, index, HIB_MODE, true, ram_ctrl->hib_ram_lp_setting);
        }

        if (ram_ctrl->sleep_ram_pd_enable) {
            sdrv_smc_lld_ap_ram_lp_ctrl(g_smc_base, index, SLP_MODE, true, ram_ctrl->sleep_ram_lp_setting);
        }
    }
}

static void sdrv_smc_ckgen_lp_disable_xcg(paddr_t base, uint32_t pcg_num, uint32_t bcg_num, uint32_t ccg_num)
{
    for (uint32_t i = 0; i < pcg_num; i++) {
        ckgen_xcg_set_gating_by_mode(base, CKGEN_PCG_ID(i), CKGEN_CG_CTL_SLP_MODE, 0);
        ckgen_xcg_set_gating_by_mode(base, CKGEN_PCG_ID(i), CKGEN_CG_CTL_HIB_MODE, 0);
    }

    for (uint32_t i = 0; i < bcg_num; i++) {
        ckgen_xcg_set_gating_by_mode(base, CKGEN_BCG_ID(i), CKGEN_CG_CTL_SLP_MODE, 0);
        ckgen_xcg_set_gating_by_mode(base, CKGEN_BCG_ID(i), CKGEN_CG_CTL_HIB_MODE, 0);
    }

    for (uint32_t i = 0; i < ccg_num; i++) {
        ckgen_xcg_set_gating_by_mode(base, CKGEN_CCG_ID(i), CKGEN_CG_CTL_SLP_MODE, 0);
        ckgen_xcg_set_gating_by_mode(base, CKGEN_CCG_ID(i), CKGEN_CG_CTL_HIB_MODE, 0);
    }
}

static void sdrv_smc_ckgen_lp_pll_config(paddr_t base, uint32_t index, struct sdrv_smc_pll *pll_config)
{
    if (pll_config->sleep_mode_disable) {
        ckgen_pll_set_gating_by_mode(base, index, CKGEN_PLL_CTL_SLP_MODE, 0);
    }

    if (pll_config->hib_mode_disable) {
        ckgen_pll_set_gating_by_mode(base, index, CKGEN_PLL_CTL_HIB_MODE, 0);
    }

    if (pll_config->sleep_mode_pd) {
        ckgen_pll_set_pd_state_by_mode(base, index, CKGEN_PLL_PD_SLP_MODE, 1);
    }

    if (pll_config->hib_mode_pd) {
        ckgen_pll_set_pd_state_by_mode(base, index, CKGEN_PLL_PD_HIB_MODE, 1);
    }
}

static void sdrv_smc_ckgen_lp_xtal_config(paddr_t base, struct sdrv_smc_xtal *xtal_config)
{
    if (xtal_config->sleep_mode_disable) {
        ckgen_xtal_set_gating_by_mode(base, CKGEN_XTAL_CTL_SLP_MODE, 0);
    }

    if (xtal_config->hib_mode_disable) {
        ckgen_xtal_set_gating_by_mode(base, CKGEN_XTAL_CTL_HIB_MODE, 0);
    }
}

static void sdrv_smc_pmu_lp_config(struct sdrv_smc_pmu *pmu_config, sdrv_pmu_mode_t lp_mode)
{
    sdrv_pmu_pwrctrl_enable(&g_sdrv_pmu_dev, lp_mode, PMU_PWR_CTRL_0, pmu_config->pwr_ctrl0);
    sdrv_pmu_pwrctrl_enable(&g_sdrv_pmu_dev, lp_mode, PMU_PWR_CTRL_1, pmu_config->pwr_ctrl1);
    sdrv_pmu_pwrctrl_enable(&g_sdrv_pmu_dev, lp_mode, PMU_PWR_CTRL_2, pmu_config->pwr_ctrl2);
    sdrv_pmu_pwrctrl_enable(&g_sdrv_pmu_dev, lp_mode, PMU_PWR_CTRL_3, pmu_config->pwr_ctrl3);

    sdrv_pmu_pwron_enable(&g_sdrv_pmu_dev, lp_mode, 0, pmu_config->pwr_on0);
    sdrv_pmu_pwron_enable(&g_sdrv_pmu_dev, lp_mode, 1, pmu_config->pwr_on1);
    sdrv_pmu_pwron_enable(&g_sdrv_pmu_dev, lp_mode, 2, pmu_config->pwr_on2);
    sdrv_pmu_pwron_enable(&g_sdrv_pmu_dev, lp_mode, 3, pmu_config->pwr_on3);

    sdrv_pmu_pwron_target(&g_sdrv_pmu_dev, lp_mode, 0, pmu_config->pwron0_ctrl_target);
    sdrv_pmu_pwron_target(&g_sdrv_pmu_dev, lp_mode, 1, pmu_config->pwron1_ctrl_target);
    sdrv_pmu_pwron_target(&g_sdrv_pmu_dev, lp_mode, 2, pmu_config->pwron2_ctrl_target);
    sdrv_pmu_pwron_target(&g_sdrv_pmu_dev, lp_mode, 3, pmu_config->pwron3_ctrl_target);

    sdrv_pmu_power_on_off(&g_sdrv_pmu_dev, lp_mode, 0, pmu_config->saf_power_on);
    sdrv_pmu_power_on_off(&g_sdrv_pmu_dev, lp_mode, 1, pmu_config->ap_power_on);
}

/**
 * @brief software trigger SWM change
 *
 * @param module see macro SMC_HK_**
 * @param domain see macro SMC_HK_**
 * @param lp_mode see macro SMC_HK_**
 */
void sdrv_smc_sw_trigger_handshake(uint32_t module, uint32_t domain, uint32_t lp_mode)
{
    smc_rs_type dom_type;
    sw_swm_mode mode;
    handshake_stage stage;

    dom_type = (domain == SMC_HK_SAF) ? SAF_DOMAIN : AP_DOMAIN;
    mode = lp_mode;

    if (SMC_HK_CKGEN == module) {
        stage = CKGEN_HK;
    }
    else if (SMC_HK_RSTGEN == module) {
        stage = RSTGEN_HK;
    }
    else if (SMC_HK_PMU == module) {
        stage = PMU_HK;
        dom_type = SOC_DOMAIN;
    }
    else {
        return;
    }

    sdrv_smc_lld_sw_swm_config(g_smc_base, dom_type, mode, stage);
}

void sdrv_smc_sw_power_switch_gate(smc_domain domain, uint32_t lp_ctrl, bool gate)
{
    ASSERT((domain == SMC_SAF_DOMAIN && lp_ctrl <= SMC_SAF_CR5_GAMA1) ||
           (domain == SMC_AP_DOMAIN && lp_ctrl <= SMC_AP_SS));

    if (gate) {
        sdrv_smc_lld_sw_domain_lp_config(g_smc_base, domain, (uint8_t)lp_ctrl, SW_PD_EN, true);
        sdrv_smc_lld_sw_domain_lp_config(g_smc_base, domain, (uint8_t)lp_ctrl, SW_ISO_EN, true);
        sdrv_smc_lld_sw_domain_lp_config(g_smc_base, domain, (uint8_t)lp_ctrl, SW_PWR_GATE, true);
    }
    else {
        sdrv_smc_lld_sw_domain_lp_config(g_smc_base, domain, (uint8_t)lp_ctrl, SW_PWR_GATE, false);
        sdrv_smc_lld_sw_domain_lp_config(g_smc_base, domain, (uint8_t)lp_ctrl, SW_PD_EN, false);
        sdrv_smc_lld_sw_domain_lp_config(g_smc_base, domain, (uint8_t)lp_ctrl, SW_ISO_EN, false);
    }
}

void sdrv_smc_sw_ram_lp_ctrl(smc_domain domain, uint32_t lp_ctrl, bool pd, uint32_t ram_mode)
{
    ASSERT((domain == SMC_SAF_DOMAIN && lp_ctrl <= SMC_SAF_MISC) ||
           (domain == SMC_AP_DOMAIN && lp_ctrl <= SMC_DISP_SS));

    if (domain == SMC_SAF_DOMAIN) {
        sdrv_smc_lld_sw_saf_ram_lp_ctrl(g_smc_base, lp_ctrl, pd, ram_mode);
    }
    else {
        sdrv_smc_lld_sw_ap_ram_lp_ctrl(g_smc_base, lp_ctrl, pd, ram_mode);
    }
}

void sdrv_smc_wait_soc_swm_run(void)
{
    while (sdrv_smc_lld_swm_status_monitor(g_smc_base, SOC_SWM) != 0x1) {
    }
}

void sdrv_smc_wait_ap_sys_ready(void)
{
    while (taishan_rstgen_lld_get_ss_rdy(APB_RSTGEN_SF_BASE) != 0x1) {
    }
}

void sdrv_smc_clear_all_irq_mon(void)
{
    for (uint32_t i = 0; i < 5; i++) {
        sdrv_smc_lld_clear_core_irq_mon(g_smc_base, i, 0xffffu);
    }

    for (uint32_t i = 0; i < 8; i++) {
        sdrv_smc_lld_clear_comm_irq_mon(g_smc_base, i, 0xffffffffu);
    }
}

void sdrv_smc_lp_request(uint32_t saf_mode, uint32_t ap_mode)
{
    sdrv_smc_lld_saf_request_lp(g_smc_base, saf_mode);
    sdrv_smc_lld_ap_request_lp(g_smc_base, ap_mode);
}

void sdrv_smc_lp_disable(void)
{
    sdrv_smc_lld_saf_disable_lp(g_smc_base);
    sdrv_smc_lld_ap_disable_lp(g_smc_base);
}

void sdrv_smc_ckgen_config(struct sdrv_smc_ckgen_config *ckgen_config)
{
    /* Config all PCG/BCG/CCG disable in SLEEP&HIB mode */
    sdrv_smc_ckgen_lp_disable_xcg(APB_CKGEN_SF_BASE, 334, 15, 5);
    sdrv_smc_ckgen_lp_disable_xcg(APB_CKGEN_AP_BASE, 60, 7, 0);

    /* Use enable clk config */
    for (uint32_t i = 0; i < ckgen_config->enable_clk_num; i++) {
        struct sdrv_smc_clock *enable_clk;

        enable_clk = &ckgen_config->enable_clk[i];

        if (enable_clk->peripheral) {
            if (enable_clk->sleep_enable) {
                ckgen_xcg_set_gating_by_mode(enable_clk->peripheral->clk_node.core->base,
                    enable_clk->peripheral->clk_node.id, CKGEN_CG_CTL_SLP_MODE, 1);
            }

            if (enable_clk->hib_enable) {
                ckgen_xcg_set_gating_by_mode(enable_clk->peripheral->clk_node.core->base,
                    enable_clk->peripheral->clk_node.id, CKGEN_CG_CTL_HIB_MODE, 1);
            }
        }
    }

    /* Default PLL & XTAL enable in LP */
    sdrv_smc_ckgen_lp_pll_config(APB_CKGEN_SF_BASE, 0, &ckgen_config->pll1);
    sdrv_smc_ckgen_lp_pll_config(APB_CKGEN_SF_BASE, 1, &ckgen_config->pll2);
    sdrv_smc_ckgen_lp_pll_config(APB_CKGEN_SF_BASE, 2, &ckgen_config->pll3);
    sdrv_smc_ckgen_lp_xtal_config(APB_CKGEN_SF_BASE, &ckgen_config->saf);

    sdrv_smc_ckgen_lp_pll_config(APB_CKGEN_AP_BASE, 0, &ckgen_config->pll4);
    sdrv_smc_ckgen_lp_pll_config(APB_CKGEN_AP_BASE, 1, &ckgen_config->pll5);
    sdrv_smc_ckgen_lp_pll_config(APB_CKGEN_AP_BASE, 2, &ckgen_config->lvds);
    sdrv_smc_ckgen_lp_xtal_config(APB_CKGEN_AP_BASE, &ckgen_config->ap);

    /* Bus slice 0&1 disable HW_DIS_EN */
    RMWREG32(CKGEN_BUS_CTL_BASE(APB_CKGEN_SF_BASE, 0), CKGEN_BUS_CTL_HW_DIS_EN, 1, 0);
    RMWREG32(CKGEN_BUS_CTL_BASE(APB_CKGEN_SF_BASE, 1), CKGEN_BUS_CTL_HW_DIS_EN, 1, 0);
    RMWREG32(CKGEN_BUS_CTL_BASE(APB_CKGEN_AP_BASE, 0), CKGEN_BUS_CTL_HW_DIS_EN, 1, 0);
    RMWREG32(CKGEN_BUS_CTL_BASE(APB_CKGEN_AP_BASE, 1), CKGEN_BUS_CTL_HW_DIS_EN, 1, 0);
}

void sdrv_smc_rstgen_config(struct sdrv_smc_rstgen_config *rstgen_config)
{
    for (uint32_t i = 0; i < rstgen_config->smc_reset_num; i++) {
        struct sdrv_smc_reset *smc_reset;

        smc_reset = &rstgen_config->smc_reset[i];

        if (smc_reset->sig) {
            if (smc_reset->sleep_mode_reset) {
                reset_ctl_lowpower_set(smc_reset->sig, RESET_LP_SLEEP, 0);
            }
            else {
                reset_ctl_lowpower_set(smc_reset->sig, RESET_LP_SLEEP, 1);
            }

            if (smc_reset->hib_mode_reset) {
                reset_ctl_lowpower_set(smc_reset->sig, RESET_LP_HIB, 0);
            }
            else {
                reset_ctl_lowpower_set(smc_reset->sig, RESET_LP_HIB, 1);
            }
        }
    }
}

void sdrv_smc_pmu_config(struct sdrv_smc_pmu_config *pmu_config)
{
    sdrv_smc_pmu_lp_config(&pmu_config->sleep, PMU_SLEEP);
    sdrv_smc_pmu_lp_config(&pmu_config->hib, PMU_HIBERNATE);
}

void sdrv_smc_init(struct sdrv_smc_config *smc_config)
{
    sdrv_smc_lp_disable();

    sdrv_smc_lld_pri_core_config(g_smc_base, SAF_DOMAIN, smc_config->saf_primary_core);

    sdrv_smc_lp_pwr_ctrl(SAF_DOMAIN, 0, &smc_config->sf_mix);
    sdrv_smc_lp_pwr_ctrl(SAF_DOMAIN, 1, &smc_config->sp_mix);
    sdrv_smc_lp_pwr_ctrl(SAF_DOMAIN, 2, &smc_config->sx_mix);
    sdrv_smc_lp_pwr_ctrl(SAF_DOMAIN, 3, &smc_config->gama1_pwr);

    sdrv_smc_lp_ram_ctrl(SAF_DOMAIN, 0, &smc_config->cr5_sf);
    sdrv_smc_lp_ram_ctrl(SAF_DOMAIN, 1, &smc_config->cr5_sp);
    sdrv_smc_lp_ram_ctrl(SAF_DOMAIN, 2, &smc_config->cr5_sx);
    sdrv_smc_lp_ram_ctrl(SAF_DOMAIN, 3, &smc_config->gama1_ram);
    sdrv_smc_lp_ram_ctrl(SAF_DOMAIN, 4, &smc_config->saf_misc);

    sdrv_smc_lld_pri_core_config(g_smc_base, AP_DOMAIN, smc_config->ap_primary_core);

    if (smc_config->ap_lp_align_saf) {
        sdrv_smc_lld_ap_lp_align_saf_config(g_smc_base);
    }

    if (smc_config->ap_wk_align_saf) {
        sdrv_smc_lld_ap_wakeup_align_saf_config(g_smc_base);
    }

    sdrv_smc_lp_pwr_ctrl(AP_DOMAIN, 0, &smc_config->ap_disp_ss);
    sdrv_smc_lp_ram_ctrl(AP_DOMAIN, 0, &smc_config->ap_ram);
    sdrv_smc_lp_ram_ctrl(AP_DOMAIN, 1, &smc_config->disp_ram);

    if (smc_config->ap_off) {
        sdrv_smc_lld_ap_off(g_smc_base);
    }

    if (smc_config->hib_disable_rc24m) {
        sdrv_smc_lld_soc_hib_rc_dis_config(g_smc_base);
    }

    /* default mask all interrupt */
    for (int core = 0; core < 5; core++) {
        sdrv_smc_lld_irq_mask_config(g_smc_base, CORE_IRQ_TYPE, core, 0, 0xffffu);

        for (int grp = 0; grp < 8; grp++) {
            sdrv_smc_lld_irq_mask_config(g_smc_base, COMMON_IRQ_TYPE, core, grp, 0xffffffffu);
        }
    }

    for (uint32_t i = 0; i < smc_config->wk_irq_num; i++) {
        if (smc_config->wk_irq[i].core < SMC_CORE_MAX) {
            sdrv_smc_lld_irq_unmask(g_smc_base, smc_config->wk_irq[i].core, smc_config->wk_irq[i].irq);
        }
    }

    irq_attach(g_smc_int, smc_irq_handle, NULL);
    irq_enable(g_smc_int);
}
