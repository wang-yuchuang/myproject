/*
 *  @File: sdrv_sem.h
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights RESERVEDd.
 *
 * @Description:
 *
 * @Revision History:
 * ----------------
 */

#ifndef DRV_LLD_SDRV_SEM_H
#define DRV_LLD_SDRV_SEM_H

#include <compiler.h>
#include <types.h>
#include "sem_hw.h"

#define SEM_COMMON_SET_OFF	0x0U

#define FM_SEM_COMMON_SET_CMP_ENABLE	(0xffU << 24U)
#define FV_SEM_COMMON_SET_CMP_ENABLE(v) \
	(((v) << 24U) & FM_SEM_COMMON_SET_CMP_ENABLE)
#define GFV_SEM_COMMON_SET_CMP_ENABLE(v) \
	(((v) & FM_SEM_COMMON_SET_CMP_ENABLE) >> 24U)

#define BM_SEM_COMMON_SET_SIG_MON_GLB_EN	(0x01U << 23U)

#define BM_SEM_COMMON_SET_EI_GLOBAL_EN_LOCK	(0x01U << 17U)

#define BM_SEM_COMMON_SET_EI_GLOBAL_EN	(0x01U << 16U)

#define FM_SEM_COMMON_SET_ERROR_DIV	(0xfU << 12U)
#define FV_SEM_COMMON_SET_ERROR_DIV(v) \
	(((v) << 12U) & FM_SEM_COMMON_SET_ERROR_DIV)
#define GFV_SEM_COMMON_SET_ERROR_DIV(v) \
	(((v) & FM_SEM_COMMON_SET_ERROR_DIV) >> 12U)

#define BM_SEM_COMMON_SET_ERROR_WAVE_SEL	(0x01U << 11U)

#define BM_SEM_COMMON_SET_FATAL_HIGH_LOW	(0x01U << 10U)

#define FM_SEM_COMMON_SET_SEM_INT_DC4_SEL	(0x3U << 8U)
#define FV_SEM_COMMON_SET_SEM_INT_DC4_SEL(v) \
	(((v) << 8U) & FM_SEM_COMMON_SET_SEM_INT_DC4_SEL)
#define GFV_SEM_COMMON_SET_SEM_INT_DC4_SEL(v) \
	(((v) & FM_SEM_COMMON_SET_SEM_INT_DC4_SEL) >> 8U)

#define FM_SEM_COMMON_SET_SEM_INT_DC3_SEL	(0x3U << 6U)
#define FV_SEM_COMMON_SET_SEM_INT_DC3_SEL(v) \
	(((v) << 6U) & FM_SEM_COMMON_SET_SEM_INT_DC3_SEL)
#define GFV_SEM_COMMON_SET_SEM_INT_DC3_SEL(v) \
	(((v) & FM_SEM_COMMON_SET_SEM_INT_DC3_SEL) >> 6U)

#define FM_SEM_COMMON_SET_SEM_INT_DC2_SEL	(0x3U << 4U)
#define FV_SEM_COMMON_SET_SEM_INT_DC2_SEL(v) \
	(((v) << 4U) & FM_SEM_COMMON_SET_SEM_INT_DC2_SEL)
#define GFV_SEM_COMMON_SET_SEM_INT_DC2_SEL(v) \
	(((v) & FM_SEM_COMMON_SET_SEM_INT_DC2_SEL) >> 4U)

#define FM_SEM_COMMON_SET_SEM_INT_DC1_SEL	(0x3U << 2U)
#define FV_SEM_COMMON_SET_SEM_INT_DC1_SEL(v) \
	(((v) << 2U) & FM_SEM_COMMON_SET_SEM_INT_DC1_SEL)
#define GFV_SEM_COMMON_SET_SEM_INT_DC1_SEL(v) \
	(((v) & FM_SEM_COMMON_SET_SEM_INT_DC1_SEL) >> 2U)

#define FM_SEM_COMMON_SET_SEM_INT_DC0_SEL	(0x3U << 0U)
#define FV_SEM_COMMON_SET_SEM_INT_DC0_SEL(v) \
	(((v) << 0U) & FM_SEM_COMMON_SET_SEM_INT_DC0_SEL)
#define GFV_SEM_COMMON_SET_SEM_INT_DC0_SEL(v) \
	(((v) & FM_SEM_COMMON_SET_SEM_INT_DC0_SEL) >> 0U)

#define INT_TRIG_OFF	0x4U

#define FM_INT_TRIG_INT_TRIG	(0xffU << 0U)
#define FV_INT_TRIG_INT_TRIG(v) \
	(((v) << 0U) & FM_INT_TRIG_INT_TRIG)

#define INT_ENABLE_OVRD_OFF	0x8U

#define FM_INT_ENABLE_OVRD_INT_ENABLE_OVRD_VAL	(0xffU << 8U)
#define FV_INT_ENABLE_OVRD_INT_ENABLE_OVRD_VAL(v) \
	(((v) << 8U) & FM_INT_ENABLE_OVRD_INT_ENABLE_OVRD_VAL)

#define FM_INT_ENABLE_OVRD_INT_ENABLE_OVRD_EN	(0xffU << 0U)
#define FV_INT_ENABLE_OVRD_INT_ENABLE_OVRD_EN(v) \
	(((v) << 0U) & FM_INT_ENABLE_OVRD_INT_ENABLE_OVRD_EN)

#define ERR_INJ_CTRL_OFF	0xcU

#define BM_ERR_INJ_CTRL_LOCK	(0x01U << 8U)

#define BM_ERR_INJ_CTRL_REG_PARITY_ERR_INJ_EN	(0x01U << 0U)

#define IO_WAVE_CTRL_OFF	0x10U

#define BM_IO_WAVE_CTRL_LOCK	(0x01U << 31U)

#define FM_IO_WAVE_CTRL_LEVEL_CFG	(0x3U << 2U)
#define FV_IO_WAVE_CTRL_LEVEL_CFG(v) \
	(((v) << 2U) & FM_IO_WAVE_CTRL_LEVEL_CFG)

#define FM_IO_WAVE_CTRL_LEVEL_EN	(0x3U << 0U)
#define FV_IO_WAVE_CTRL_LEVEL_EN(v) \
	(((v) << 0U) & FM_IO_WAVE_CTRL_LEVEL_EN)

#define SEM_INT_CTRL_OFF	0x14U

#define BM_SEM_INT_CTRL_LOCK	(0x01U << 31U)

#define BM_SEM_INT_CTRL_REG_PARITY_EJ_ERR_INJ	(0x01U << 26U)

#define BM_SEM_INT_CTRL_GLOBAL_EJ_EN_ERR_INJ_EN	(0x01U << 25U)

#define BM_SEM_INT_CTRL_GLOBAL_EJ_EN_ERR_INJ	(0x01U << 24U)

#define FM_SEM_INT_CTRL_HW_INT_ERR_INJ	(0x1fU << 16U)
#define FV_SEM_INT_CTRL_HW_INT_ERR_INJ(v) \
	(((v) << 16U) & FM_SEM_INT_CTRL_HW_INT_ERR_INJ)

#define BM_SEM_INT_CTRL_PMU_INT_ERR_INJ	(0x01U << 10U)

#define BM_SEM_INT_CTRL_CPU_INT_ERR_INJ	(0x01U << 9U)

#define BM_SEM_INT_CTRL_PAD_INT_ERR_INJ	(0x01U << 8U)

#define BM_SEM_INT_CTRL_CORERR_INT_STAT	(0x01U << 5U)

#define BM_SEM_INT_CTRL_UNCERR_INT_STAT	(0x01U << 4U)

#define BM_SEM_INT_CTRL_CORERR_INT_ERR_INJ	(0x01U << 3U)

#define BM_SEM_INT_CTRL_UNCERR_INT_ERR_INJ	(0x01U << 2U)

#define BM_SEM_INT_CTRL_CORERR_INT_CLR	(0x01U << 1U)

#define BM_SEM_INT_CTRL_UNCERR_INT_CLR	(0x01U << 0U)

#define INT_STATUS_OFF(n)	(0x80U + 4U*(n))

#define TGT_VALUE_OFF(n)	(0x500U + 4U*(n))

#define SIG_MON_ENABLE_OFF(n)	(0x580U + 4U*(n))

#define CORERR_INT_STAT_OFF(n)	(0x780U + 4U*(n))

#define SEM_UNCORR_INT_STATUS(input) \
                               (INT_STATUS_OFF((input) / 32))

#define SEM_UNCORR_INT_ENABLE(output, input) \
                               (0x100 + ((output) * 0x80) + ((input) / 32 * 4))

#define SEM_CORR_INT_STATUS(input) \
                               (CORERR_INT_STAT_OFF((input) / 32))

#define SEM_CORR_INT_ENABLE(output, input) \
                               (0x600 + ((output) * 0x80) + ((input) / 32 * 4))

#define TGT_VALUE(sig)     TGT_VALUE_OFF((sig) / 32)
#define MON_ENABLE(sig)    SIG_MON_ENABLE_OFF((sig) / 32)

#define IO_LEVEL_CFG_NUM     2U

#ifndef ASSEMBLY

__BEGIN_CDECLS

/* sem interrupt output to target(PAD/CPU/PMU/HW(0~4)) */
typedef enum {
    SEM_OUTPUT_PAD0 = 0,
    SEM_OUTPUT_PAD1,
    SEM_OUTPUT_CPU,
    SEM_OUTPUT_PMU,
    SEM_OUTPUT4,
    SEM_OUTPUT5,
    SEM_OUTPUT6,
    SEM_OUTPUT7
} sdrv_sem_int_out_e;

/* sem error configuration(error divider ration/wave A/B selection /fatal error) */
typedef struct sdrv_sem_err_cfg {
    uint8_t err_divider;
    uint8_t err_wave_sel;
    uint8_t fatal_highlow_sel;
} sdrv_sem_err_cfg_t;

/* sem error configuration(error divider ration/wave A/B selection /fatal error) */
typedef struct sdrv_sem_io_wave_cfg {
    uint8_t level_cfg[IO_LEVEL_CFG_NUM];
    uint8_t level_en[IO_LEVEL_CFG_NUM];
} sdrv_sem_io_wave_cfg_t;

/* sem interrupt out selection for(HW0~4:rstgen/dc/iomux/xtrg/sf_pll) */
typedef struct sdrv_sem_int_sel_cfg {
    uint8_t rstgen;
    uint8_t dc;
    uint8_t iomux;
    uint8_t xtrg;
    uint8_t sf_pll;
} sdrv_sem_int_sel_cfg_t;

/**
 * @brief sdrv sem error injection global lock enable.
 *
 * @param[in] base        reg base
 * @param[in] en          enable/disable
 */
void sdrv_sem_lld_glb_ei_lock(paddr_t base, bool en);

/**
 * @brief sdrv sem error injection global enable.
 *
 * @param[in] base reg base
 * @param[in] en   reg val
 */
void sdrv_sem_lld_error_inj_glb_en(paddr_t base, bool en);

/**
 * @brief sdrv sem int to HW selection(rstgen/ap_ss.dc/iomux/xtrg/sf_pll).
 *
 * @param[in] base     reg base
 * @param[in] int_sel  hw int selection
 */
void sdrv_sem_lld_hw_int_sel(paddr_t base, sdrv_sem_int_sel_cfg_t *int_sel);

/**
 * @brief sdrv sem compare output enable.
 *
 * @param[in] base    reg base
 * @param[in] output  sem interrupt output
 * @param[in] en      reg val
 */
void sdrv_sem_lld_cmp_en(paddr_t base, sdrv_sem_int_out_e output, bool en);

/**
 * @brief sdrv sem signal monitor global enable.
 *
 * @param[in] base    reg base
 * @param[in] en      reg val
 */
void sdrv_sem_lld_glb_mon_en(paddr_t base, bool en);

/**
 * @brief  The SEM internal monitor compares all sem_monitor_sig signals
 *         with the target value (0 or 1) specified in this function.
 *
 * @param[in] base    reg base
 * @param[in] en      reg val
 */
void sdrv_sem_lld_mon_compare_sig(paddr_t base, uint32_t sig, bool compare, bool target);

/**
 * @brief sdrv sem interrupt0~7 enable override.
 *
 * @param[in] base    reg base
 * @param[in] output  sem interrupt output
 * @param[in] en      reg val
 */
void sdrv_sem_lld_override_en(paddr_t base, sdrv_sem_int_out_e output, bool en);

/**
 * @brief sdrv sem output io wave control.
 *
 * @param[in] base    reg base
 * @param[in] output  sem interrupt output
 * @param[in] en      reg val
 */
void sdrv_sem_lld_output_io_ctl(paddr_t base, const sdrv_sem_io_wave_cfg_t *config);

/**
 * @brief sdrv sem uncorrectable interrupt status.
 *
 * @param[in] base   reg base
 * @param[in] input  sem interrupt input
 * @param[in] en     reg val
 */
bool sdrv_sem_lld_uncorr_int_status(paddr_t base, sem_uncorr_int_input input);

/**
 * @brief sdrv sem uncorrectable interrupt input enable.
 *
 * @param[in] base    reg base
 * @param[in] output  sem interrupt output
 * @param[in] input   sem uncor interrupt input
 * @param[in] en      reg val
 */
void sdrv_sem_lld_uncorr_interrupt_en(paddr_t base, sdrv_sem_int_out_e output, sem_uncorr_int_input input, bool en);

/**
 * @brief sdrv sem correctable interrupt status.
 *
 * @param[in] base   reg base
 * @param[in] input  sem cor interrupt input
 */
bool sdrv_sem_lld_corr_int_status(paddr_t base, sem_corr_int_input input);

/**
 * @brief sdrv sem correctable interrupt input enable.
 *
 * @param[in] base    reg base
 * @param[in] output  sem interrupt output
 * @param[in] input   sem cor interrupt input
 * @param[in] en      reg val
 */
void sdrv_sem_lld_corr_interrupt_en(paddr_t base, sdrv_sem_int_out_e output, sem_corr_int_input input, bool en);

/**
 * @brief sdrv sem error configuration.
 *
 * @param[in] base    reg base
 * @param[in] config  sem error configuration(error divider ration/wave A/B selection /fatal error)
 */
void sdrv_sem_lld_error_config(paddr_t base, const sdrv_sem_err_cfg_t *config);

/**
 * @brief sdrv sem int0~7 trigger.
 *
 * @param[in] base    reg base
 * @param[in] output  sem interrupt output
 * @param[in] en      reg val
 */
void sdrv_sem_lld_trigger_en(paddr_t base, sdrv_sem_int_out_e output, bool en);

/**
 * @brief sdrv sem parity error injection enable.
 *
 * @param[in] base reg base
 * @param[in] en   reg val
 */
void sdrv_sem_lld_parity_error_inj_en(paddr_t base, bool en);

/**
 * @brief sdrv sem int ctrl lock.
 *
 * @param[in] base reg base
 * @param[in] en   reg val
 */
void sdrv_sem_lld_int_ctrl_lock(paddr_t base, bool en);

/**
 * @brief sdrv sem parity error injection.
 *
 * @param[in] base reg base
 * @param[in] en   reg val
 */
void sdrv_sem_lld_parity_error_inj(paddr_t base, bool en);

/**
 * @brief sdrv sem global error injection.
 *
 * @param[in] base reg base
 * @param[in] en   reg val
 */
void sdrv_sem_lld_glb_error_inj(paddr_t base, bool en);

/**
 * @brief sdrv sem HW int error injection.
 *
 * @param[in] base    reg base
 * @param[in] value   reg val
 */
void sdrv_sem_lld_hw_int_error_inj(paddr_t base, uint32_t value);

/**
 * @brief sdrv sem PMU int error injection.
 *
 * @param[in] base    reg base
 * @param[in] en      pmu error injection enable
 */
void sdrv_sem_lld_pmu_int_error_inj(paddr_t base, bool en);

/**
 * @brief sdrv sem CPU int error injection.
 *
 * @param[in] base    reg base
 * @param[in] en      cpu error injection enable
 */
void sdrv_sem_lld_cpu_int_error_inj(paddr_t base, bool en);

/**
 * @brief sdrv sem PAD int error injection.
 *
 * @param[in] base    reg base
 * @param[in] en      pad error injection enable
 */
void sdrv_sem_lld_pad_int_error_inj(paddr_t base, bool en);

/**
 * @brief sdrv sem corerr int error status.
 *
 * @param[in] base    reg base
 */
bool sdrv_sem_lld_get_corerr_int_error_sta(paddr_t base);

/**
 * @brief sdrv sem uncorerr int error status.
 *
 * @param[in] base    reg base
 */
bool sdrv_sem_lld_get_uncorerr_int_error_sta(paddr_t base);

/**
 * @brief sdrv sem corerr int error injection.
 *
 * @param[in] base    reg base
 * @param[in] en      cor error int error injection
 */
void sdrv_sem_lld_corerr_int_error_inj(paddr_t base, bool en);

/**
 * @brief sdrv sem uncorerr int error injection.
 *
 * @param[in] base    reg base
 * @param[in] en      uncor error int error injection
 */
void sdrv_sem_lld_uncorerr_int_error_inj(paddr_t base, bool en);

/**
 * @brief sdrv sem corerr int error clear.
 *
 * @param[in] base    reg base
 * @param[in] en      clear corerr int error
 */
void sdrv_sem_lld_corerr_int_error_clr(paddr_t base, bool en);

/**
 * @brief sdrv sem uncorerr int error injection.
 *
 * @param[in] base    reg base
 * @param[in] en      clear uncorerr int error
 */
void sdrv_sem_lld_uncorerr_int_error_clr(paddr_t base, bool en);
__END_CDECLS

#endif

#endif
