/**
 * @file: sdrv_sem.c
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description
 *
 * @Revision History:
 * ----------------
 */

#include <types.h>

#include "taishan/sem/sdrv_sem.h"

#include "reg.h"

/**
 * @brief sdrv sem error injection global lock enable.
 *
 * @param[in] base        reg base
 * @param[in] en          enable/disable
 */
void sdrv_sem_lld_glb_ei_lock(paddr_t base, bool en)
{
    uint32_t val = 0;
    uint32_t addr = base + SEM_COMMON_SET_OFF;

    val = readl(addr);
    en ? (val |= BM_SEM_COMMON_SET_EI_GLOBAL_EN_LOCK)
       : (val &= ~BM_SEM_COMMON_SET_EI_GLOBAL_EN_LOCK);
    writel(val, addr);
}

/**
 * @brief sdrv sem error injection global enable.
 *
 * @param[in] base reg base
 * @param[in] en   reg val
 */
void sdrv_sem_lld_error_inj_glb_en(paddr_t base, bool en)
{
    uint32_t val = readl(base + SEM_COMMON_SET_OFF);

    en ? (val |= BM_SEM_COMMON_SET_EI_GLOBAL_EN)
       : (val &= ~BM_SEM_COMMON_SET_EI_GLOBAL_EN);
    writel(val, base + SEM_COMMON_SET_OFF);
}

/**
 * @brief sdrv sem int to HW selection(rstgen/ap_ss.dc/iomux/xtrg/sf_pll).
 *
 * @param[in] base     reg base
 * @param[in] int_sel  hw int selection
 */
void sdrv_sem_lld_hw_int_sel(paddr_t base, sdrv_sem_int_sel_cfg_t *int_sel)
{
    uint32_t val = readl(base + SEM_COMMON_SET_OFF);

    val &= ~(
        FM_SEM_COMMON_SET_SEM_INT_DC0_SEL | FM_SEM_COMMON_SET_SEM_INT_DC1_SEL |
        FM_SEM_COMMON_SET_SEM_INT_DC2_SEL | FM_SEM_COMMON_SET_SEM_INT_DC3_SEL |
        FM_SEM_COMMON_SET_SEM_INT_DC4_SEL);
    val |= FV_SEM_COMMON_SET_SEM_INT_DC0_SEL(int_sel->rstgen) |
           FV_SEM_COMMON_SET_SEM_INT_DC1_SEL(int_sel->dc) |
           FV_SEM_COMMON_SET_SEM_INT_DC2_SEL(int_sel->iomux) |
           FV_SEM_COMMON_SET_SEM_INT_DC3_SEL(int_sel->xtrg) |
           FV_SEM_COMMON_SET_SEM_INT_DC4_SEL(int_sel->sf_pll);
    writel(val, base + SEM_COMMON_SET_OFF);
}

/**
 * @brief sdrv sem compare output enable.
 *
 * @param[in] base    reg base
 * @param[in] output  sem interrupt output
 * @param[in] en      reg val
 */
void sdrv_sem_lld_cmp_en(paddr_t base, sdrv_sem_int_out_e output, bool en)
{
    uint32_t val = readl(base + SEM_COMMON_SET_OFF);

    en ? (val |= FV_SEM_COMMON_SET_CMP_ENABLE(1 << output))
       : (val &= (~FV_SEM_COMMON_SET_CMP_ENABLE(1 << output)));
    writel(val, base + SEM_COMMON_SET_OFF);
}

/**
 * @brief sdrv sem signal monitor global enable.
 *
 * @param[in] base    reg base
 * @param[in] en      reg val
 */
void sdrv_sem_lld_glb_mon_en(paddr_t base, bool en)
{
    uint32_t val = readl(base + SEM_COMMON_SET_OFF);

    en ? (val |= BM_SEM_COMMON_SET_SIG_MON_GLB_EN)
       : (val &= ~BM_SEM_COMMON_SET_SIG_MON_GLB_EN);
    writel(val, base + SEM_COMMON_SET_OFF);
}

/**
 * @brief sdrv sem error configuration.
 *
 * @param[in] base    reg base
 * @param[in] config  sem error configuration(error divider ration/wave A/B
 * selection /fatal error)
 */
void sdrv_sem_lld_error_config(paddr_t base, const sdrv_sem_err_cfg_t *config)
{
    uint32_t val = readl(base + SEM_COMMON_SET_OFF);

    val &= ~FM_SEM_COMMON_SET_ERROR_DIV;
    val |= FV_SEM_COMMON_SET_ERROR_DIV(config->err_divider);
    /* 0 : nominal->waveA recoverable->waveB */
    (config->err_wave_sel & 0x01) ? (val |= BM_SEM_COMMON_SET_ERROR_WAVE_SEL)
                                  : (val &= ~BM_SEM_COMMON_SET_ERROR_WAVE_SEL);
    /* 0: fatal error */
    (config->fatal_highlow_sel & 0x01)
        ? (val |= BM_SEM_COMMON_SET_FATAL_HIGH_LOW)
        : (val &= ~BM_SEM_COMMON_SET_FATAL_HIGH_LOW);
    writel(val, base + SEM_COMMON_SET_OFF);
}

/**
 * @brief  The SEM internal monitor compares all sem_monitor_sig signals
 *         with the target value (0 or 1) specified in this function.
 *
 * @param[in] base    reg base
 * @param[in] en      reg val
 */
void sdrv_sem_lld_mon_compare_sig(paddr_t base, uint32_t sig, bool compare,
                                  bool target)
{
    /* enable or disable monitoring of specified signal */
    RMWREG32((base + MON_ENABLE(sig)), (sig % 32), 1U, compare);

    /* set target if comparision is enabled */
    if (compare) {
        RMWREG32((base + TGT_VALUE(sig)), (sig % 32), 1U, target);
    }
}

/**
 * @brief sdrv sem int0~7 trigger.
 *
 * @param[in] base    reg base
 * @param[in] output  sem interrupt output
 * @param[in] en      reg val
 */
void sdrv_sem_lld_trigger_en(paddr_t base, sdrv_sem_int_out_e output, bool en)
{
    uint32_t val = readl(base + INT_TRIG_OFF);

    en ? (val |= FV_INT_TRIG_INT_TRIG(1 << output))
       : (val &= ~FV_INT_TRIG_INT_TRIG(1 << output));
    writel(val, base + INT_TRIG_OFF);
}

/**
 * @brief sdrv sem interrupt0~7 enable override.
 *
 * @param[in] base    reg base
 * @param[in] output  sem interrupt output
 * @param[in] en      reg val
 */
void sdrv_sem_lld_override_en(paddr_t base, sdrv_sem_int_out_e output, bool en)
{
    uint32_t val = readl(base + INT_ENABLE_OVRD_OFF);

    val |= FV_INT_ENABLE_OVRD_INT_ENABLE_OVRD_EN(1 << output);
    en ? (val |= FV_INT_ENABLE_OVRD_INT_ENABLE_OVRD_VAL(1 << output))
       : (val &= ~FV_INT_ENABLE_OVRD_INT_ENABLE_OVRD_VAL(1 << output));
    writel(val, base + INT_ENABLE_OVRD_OFF);
}

/**
 * @brief sdrv sem parity error injection enable.
 *
 * @param[in] base reg base
 * @param[in] en   reg val
 */
void sdrv_sem_lld_parity_error_inj_en(paddr_t base, bool en)
{
    uint32_t val = readl(base + ERR_INJ_CTRL_OFF);

    /* unlock error injection ctrl */
    val &= ~BM_ERR_INJ_CTRL_LOCK;
    writel(val, base + ERR_INJ_CTRL_OFF);

    en ? (val |= BM_ERR_INJ_CTRL_REG_PARITY_ERR_INJ_EN)
       : (val &= ~BM_ERR_INJ_CTRL_REG_PARITY_ERR_INJ_EN);
    writel(val, base + ERR_INJ_CTRL_OFF);
}

/**
 * @brief sdrv sem output io wave control.
 *
 * @param[in] base    reg base
 * @param[in] output  sem interrupt output
 * @param[in] en      reg val
 */
void sdrv_sem_lld_output_io_ctl(paddr_t base, const sdrv_sem_io_wave_cfg_t *config)
{
    uint32_t val = readl(base + IO_WAVE_CTRL_OFF);

    /* unlock output io wave control */
    val &= ~BM_IO_WAVE_CTRL_LOCK;
    writel(val, base + IO_WAVE_CTRL_OFF);
    /* config the output io for the normal and recoverable error */
    val &= ~FM_IO_WAVE_CTRL_LEVEL_EN;
    val |= FV_IO_WAVE_CTRL_LEVEL_EN(config->level_en[0] |
                                    (config->level_en[1] << 1));
    /* config the level for the io for the normal and recoverable error */
    val &= ~FM_IO_WAVE_CTRL_LEVEL_CFG;
    val |= FV_IO_WAVE_CTRL_LEVEL_CFG(config->level_cfg[0] |
                                     (config->level_cfg[1] << 1));

    writel(val, base + IO_WAVE_CTRL_OFF);
}

/**
 * @brief sdrv sem int ctrl lock.
 *
 * @param[in] base reg base
 * @param[in] en   reg val
 */
void sdrv_sem_lld_int_ctrl_lock(paddr_t base, bool en)
{
    uint32_t val = readl(base + SEM_INT_CTRL_OFF);

    en ? (val |= BM_SEM_INT_CTRL_LOCK) : (val &= ~BM_SEM_INT_CTRL_LOCK);
    writel(val, base + SEM_INT_CTRL_OFF);
}

/**
 * @brief sdrv sem parity error injection.
 *
 * @param[in] base reg base
 * @param[in] en   reg val
 */
void sdrv_sem_lld_parity_error_inj(paddr_t base, bool en)
{
    uint32_t val = readl(base + SEM_INT_CTRL_OFF);

    en ? (val |= BM_SEM_INT_CTRL_REG_PARITY_EJ_ERR_INJ)
       : (val &= ~BM_SEM_INT_CTRL_REG_PARITY_EJ_ERR_INJ);
    writel(val, base + SEM_INT_CTRL_OFF);
}

/**
 * @brief sdrv sem global error injection.
 *
 * @param[in] base reg base
 * @param[in] en   reg val
 */
void sdrv_sem_lld_glb_error_inj(paddr_t base, bool en)
{
    uint32_t val = readl(base + SEM_INT_CTRL_OFF);

    en ? (val |= BM_SEM_INT_CTRL_GLOBAL_EJ_EN_ERR_INJ_EN)
       : (val &= ~BM_SEM_INT_CTRL_GLOBAL_EJ_EN_ERR_INJ_EN);
    en ? (val |= BM_SEM_INT_CTRL_GLOBAL_EJ_EN_ERR_INJ)
       : (val &= ~BM_SEM_INT_CTRL_GLOBAL_EJ_EN_ERR_INJ);
    writel(val, base + SEM_INT_CTRL_OFF);
}

/**
 * @brief sdrv sem HW int error injection.
 *
 * @param[in] base    reg base
 * @param[in] value   reg val
 */
void sdrv_sem_lld_hw_int_error_inj(paddr_t base, uint32_t value)
{
    uint32_t val = readl(base + SEM_INT_CTRL_OFF);

    val |= FV_SEM_INT_CTRL_HW_INT_ERR_INJ(value & 0x01f);
    writel(val, base + SEM_INT_CTRL_OFF);
}

/**
 * @brief sdrv sem PMU int error injection.
 *
 * @param[in] base    reg base
 * @param[in] en      pmu error injection enable
 */
void sdrv_sem_lld_pmu_int_error_inj(paddr_t base, bool en)
{
    uint32_t val = readl(base + SEM_INT_CTRL_OFF);

    en ? (val |= BM_SEM_INT_CTRL_PMU_INT_ERR_INJ)
       : (val &= ~BM_SEM_INT_CTRL_PMU_INT_ERR_INJ);
    writel(val, base + SEM_INT_CTRL_OFF);
}

/**
 * @brief sdrv sem CPU int error injection.
 *
 * @param[in] base    reg base
 * @param[in] en      cpu error injection enable
 */
void sdrv_sem_lld_cpu_int_error_inj(paddr_t base, bool en)
{
    uint32_t val = readl(base + SEM_INT_CTRL_OFF);

    en ? (val |= BM_SEM_INT_CTRL_CPU_INT_ERR_INJ)
       : (val &= ~BM_SEM_INT_CTRL_CPU_INT_ERR_INJ);
    writel(val, base + SEM_INT_CTRL_OFF);
}

/**
 * @brief sdrv sem PAD int error injection.
 *
 * @param[in] base    reg base
 * @param[in] en      pad error injection enable
 */
void sdrv_sem_lld_pad_int_error_inj(paddr_t base, bool en)
{
    uint32_t val = readl(base + SEM_INT_CTRL_OFF);

    en ? (val |= BM_SEM_INT_CTRL_PAD_INT_ERR_INJ)
       : (val &= ~BM_SEM_INT_CTRL_PAD_INT_ERR_INJ);
    writel(val, base + SEM_INT_CTRL_OFF);
}

/**
 * @brief sdrv sem corerr int error status.
 *
 * @param[in] base    reg base
 */
bool sdrv_sem_lld_get_corerr_int_error_sta(paddr_t base)
{
    uint32_t val = readl(base + SEM_INT_CTRL_OFF);

    return (bool)((val & BM_SEM_INT_CTRL_CORERR_INT_STAT) ? 1 : 0);
}

/**
 * @brief sdrv sem uncorerr int error status.
 *
 * @param[in] base    reg base
 */
bool sdrv_sem_lld_get_uncorerr_int_error_sta(paddr_t base)
{
    uint32_t val = readl(base + SEM_INT_CTRL_OFF);

    return (bool)((val & BM_SEM_INT_CTRL_UNCERR_INT_STAT) ? 1 : 0);
}

/**
 * @brief sdrv sem corerr int error injection.
 *
 * @param[in] base    reg base
 * @param[in] en      cor error int error injection
 */
void sdrv_sem_lld_corerr_int_error_inj(paddr_t base, bool en)
{
    uint32_t val = readl(base + SEM_INT_CTRL_OFF);

    en ? (val |= BM_SEM_INT_CTRL_CORERR_INT_ERR_INJ)
       : (val &= ~BM_SEM_INT_CTRL_CORERR_INT_ERR_INJ);
    writel(val, base + SEM_INT_CTRL_OFF);
}

/**
 * @brief sdrv sem uncorerr int error injection.
 *
 * @param[in] base    reg base
 * @param[in] en      uncor error int error injection
 */
void sdrv_sem_lld_uncorerr_int_error_inj(paddr_t base, bool en)
{
    uint32_t val = readl(base + SEM_INT_CTRL_OFF);

    en ? (val |= BM_SEM_INT_CTRL_UNCERR_INT_ERR_INJ)
       : (val &= ~BM_SEM_INT_CTRL_UNCERR_INT_ERR_INJ);
    writel(val, base + SEM_INT_CTRL_OFF);
}

/**
 * @brief sdrv sem corerr int error clear.
 *
 * @param[in] base    reg base
 * @param[in] en      clear corerr int error
 */
void sdrv_sem_lld_corerr_int_error_clr(paddr_t base, bool en)
{
    uint32_t val = readl(base + SEM_INT_CTRL_OFF);

    en ? (val |= BM_SEM_INT_CTRL_CORERR_INT_CLR)
       : (val &= ~BM_SEM_INT_CTRL_CORERR_INT_CLR);
    writel(val, base + SEM_INT_CTRL_OFF);
}

/**
 * @brief sdrv sem uncorerr int error injection.
 *
 * @param[in] base    reg base
 * @param[in] en      clear uncorerr int error
 */
void sdrv_sem_lld_uncorerr_int_error_clr(paddr_t base, bool en)
{
    uint32_t val = readl(base + SEM_INT_CTRL_OFF);

    en ? (val |= BM_SEM_INT_CTRL_UNCERR_INT_CLR)
       : (val &= ~BM_SEM_INT_CTRL_UNCERR_INT_CLR);
    writel(val, base + SEM_INT_CTRL_OFF);
}

/**
 * @brief sdrv sem uncorrectable interrupt status.
 *
 * @param[in] base   reg base
 * @param[in] input  sem uncor interrupt input
 * @param[in] en     reg val
 */
bool sdrv_sem_lld_uncorr_int_status(paddr_t base, sem_uncorr_int_input input)
{
    uint32_t val = readl(base + SEM_UNCORR_INT_STATUS(input));

    val &= (uint32_t)(1 << (input % 32));
    return (bool)(val ? 1 : 0);
}

/**
 * @brief sdrv sem uncorrectable interrupt input enable.
 *
 * @param[in] base    reg base
 * @param[in] output  sem interrupt output
 * @param[in] input   sem uncor interrupt input
 * @param[in] en      reg val
 */
void sdrv_sem_lld_uncorr_interrupt_en(paddr_t base, sdrv_sem_int_out_e output,
                                      sem_uncorr_int_input input, bool en)
{
    uint32_t val = readl(base + SEM_UNCORR_INT_ENABLE(output, input));

    en ? (val |= (1 << (input % 32))) : (val &= ~(1 << (input % 32)));
    writel(val, base + (SEM_UNCORR_INT_ENABLE(output, input)));
}

/**
 * @brief sdrv sem correctable interrupt status.
 *
 * @param[in] base   reg base
 * @param[in] input  sem cor interrupt input
 */
bool sdrv_sem_lld_corr_int_status(paddr_t base, sem_corr_int_input input)
{
    uint32_t val = readl(base + SEM_CORR_INT_STATUS(input));

    val &= (uint32_t)(1 << (input % 32));
    return (bool)(val ? 1 : 0);
}

/**
 * @brief sdrv sem correctable interrupt input enable.
 *
 * @param[in] base    reg base
 * @param[in] output  sem interrupt output
 * @param[in] input   sem cor interrupt input
 * @param[in] en      reg val
 */
void sdrv_sem_lld_corr_interrupt_en(paddr_t base, sdrv_sem_int_out_e output,
                                    sem_corr_int_input input, bool en)
{
    uint32_t val = readl(base + SEM_CORR_INT_ENABLE(output, input));

    en ? (val |= (1 << (input % 32))) : (val &= ~(1 << (input % 32)));
    writel(val, base + (SEM_CORR_INT_ENABLE(output, input)));
}
