/**
 * @file sdrv_acmp.h
 * @brief semidrive acmp driver header.
 *
 * @copyright Copyright (c) 2021  Semidrive Semiconductor.
 *            All rights reserved.
 */
#ifndef DRV_LLD_SDRV_ACMP_H
#define DRV_LLD_SDRV_ACMP_H

#include <types.h>
#include <compiler.h>

#define ACMP_MAX_CSEL_NUM              4
#define ACMP_MAX_INDEX_NUM             32

#define ACMP_SOFT                      (0x00)
#define ACMP_SOFT_RST                  (0)
#define ACMP_SOFT_RC_TMR_RST           (1)

#define ACMP_MODE                      (0x04)

#define ACMP_IT_MODE_CFG               (0x08)
#define ACMP_IT_MODE_CFG_TG_START      (0)
#define ACMP_IT_MODE_CFG_QSTART        (8)
#define ACMP_IT_MODE_CFG_QEND          (16)
#define ACMP_IT_MODE_CFG_CUR_ENTRY_PT  (24)

#define ACMP_TIMER                     (0x0C)
#define ACMP_TIMER_COMPARE_VAL         (0)
#define ACMP_TIMER_TERMINAL_VAL        (16)

#define ACMP_CFG_INDEX(n)              (0x10 + n * 4)
#define ACMP_CFG_INDEX_AMSEL           (0)
#define ACMP_CFG_INDEX_CSEL            (12)
#define ACMP_CFG_INDEX_IE              (16)
#define ACMP_CFG_INDEX_UIE             (17)

#define ACMP_ANA_CFG(n)                (0x90 + n * 4)
#define ACMP_ANA_CFG_REFSEL            (0)
#define ACMP_ANA_CFG_SDSEL             (1)
#define ACMP_ANA_CFG_HCFG              (4)
#define ACMP_ANA_CFG_DAIN              (8)

#define ACMP_CTRL_CFG(n)               (0xA0 + n * 4)
#define ACMP_CTRL_CFG_AMUX_SETUP       (0)
#define ACMP_CTRL_CFG_PROP_DLY         (8)
#define ACMP_CTRL_CFG_RBW              (16)
#define ACMP_CTRL_CFG_FBW              (20)
#define ACMP_CTRL_CFG_SAMPLE_EN        (24)
#define ACMP_CTRL_CFG_MASK_EN          (25)
#define ACMP_CTRL_CFG_POL_ADJ          (26)
#define ACMP_CTRL_CFG_SYNC_EN          (27)

#define ACMP_IRQ                       (0xB0)

#define ACMP_IRQ_UNCOR                 (0xB4)

#define ANA_EXT_MUX_ERR                (0xB8)
#define ANA_EXT_MUX_ERR_STATUS         (0)
#define ANA_EXT_MUX_ERR_SIG_EN         (16)

#define ACMP_PD_CTRL                   (0xBC)
#define ACMP_PD_CTRL_CMP               (0)
#define ACMP_PD_CTRL_DAC               (1)
#define ACMP_PD_CTRL_Q_CH_CMP          (8)
#define ACMP_PD_CTRL_Q_CH_DAC          (9)

#define ACMP_INIT                      (0xC0)
#define ACMP_INIT_CNT_THRD             (0)
#define ACMP_INIT_DONE                 (16)

#define ACMP_LOW_POWER                 (0xC4)

#define ACMP_SCH                       (0xC8)
#define ACMP_SCH_ACMP_EN               (0)
#define ACMP_SCH_ANA_INSEL_ON          (1)

#define REG_PARITY_ERR_INT_STAT        (0xD0)

#define REG_PARITY_ERR_INT_STAT_EN     (0xD4)

#define REG_PARITY_ERR_INT_SIG_EN      (0xD8)

#define FUSA_COR_ERR_INT_STAT          (0xE0)

#define FUSA_COR_ERR_INT_STAT_EN       (0xE4)

#define FUSA_COR_ERR_INT_SIG_EN        (0xE8)

#define FUSA_UNCOR_ERR_INT_STAT        (0xF0)
#define FUSA_UNCOR_ERR_INT_STAT_PWDATA_UNCOR_ERR         (0)
#define FUSA_UNCOR_ERR_INT_STAT_PWDATA_DATAL_ERR         (1)
#define FUSA_UNCOR_ERR_INT_STAT_PADDR_UNCOR_ERR          (2)
#define FUSA_UNCOR_ERR_INT_STAT_PCTL_UNCOR_ERR           (3)
#define FUSA_UNCOR_ERR_INT_STAT_INPUT_ERR                (4)

#define FUSA_UNCOR_ERR_INT_STAT_EN     (0xF4)
#define FUSA_UNCOR_ERR_INT_STAT_EN_PWDATA_UNCOR_ERR      (0)
#define FUSA_UNCOR_ERR_INT_STAT_EN_PWDATA_DATAL_ERR      (1)
#define FUSA_UNCOR_ERR_INT_STAT_EN_PADDR_UNCOR_ERR       (2)
#define FUSA_UNCOR_ERR_INT_STAT_EN_PCTL_UNCOR_ERR        (3)
#define FUSA_UNCOR_ERR_INT_STAT_EN_INPUT_ERR             (4)

#define FUSA_UNCOR_ERR_INT_SIG_EN      (0xF8)
#define FUSA_UNCOR_ERR_INT_SIG_EN_PWDATA_UNCOR_ERR       (0)
#define FUSA_UNCOR_ERR_INT_SIG_EN_PWDATA_DATAL_ERR       (1)
#define FUSA_UNCOR_ERR_INT_SIG_EN_PADDR_UNCOR_ERR        (2)
#define FUSA_UNCOR_ERR_INT_SIG_EN_PCTL_UNCOR_ERR         (3)
#define FUSA_UNCOR_ERR_INT_SIG_EN_INPUT_ERR              (4)

#define PWDATA_INJ                     (0x100)

#define PWECC_INJ                      (0x104)

#define SELFTEST_MODE                  (0x108)

#define PRDATAINJ                      (0x10C)

#define INT_ERR_INJ                    (0x110)
#define INT_ERR_INJ_IE_ERR             (0)
#define INT_ERR_INJ_COR_ERR            (1)
#define INT_ERR_INJ_UNC_ERR            (2)

#ifndef ASSEMBLY

__BEGIN_CDECLS

/**
 * @brief sdrv acmp soft reset
 *
 * @param[in] base sdrv acmp address
 */
static inline void sdrv_acmp_lld_soft_reset(paddr_t base)
{
    RMWREG32(base + ACMP_SOFT, ACMP_SOFT_RST, 1, 1);

    RMWREG32(base + ACMP_SOFT, ACMP_SOFT_RST, 1, 0);
}

/**
 * @brief sdrv acmp config mode.
 *
 * @param[in] base sdrv acmp address
 * @param[in] mode sdrv acmp mode
 */
static inline void sdrv_acmp_lld_config_mode(paddr_t base, uint8_t mode)
{
    writel(mode, base + ACMP_MODE);
}

/**
 * @brief sdrv acmp config it mode.
 *
 * @param[in] base sdrv acmp address
 * @param[in] qstart sdrv acmp trigger start id number for loop
 * @param[in] qend sdrv acmp trigger end id number for loop
 * @param[in] cur_entry_pt sdrv acmp trigger first access id number
 */
static inline void sdrv_acmp_lld_config_it_mode(paddr_t base, uint8_t qstart,
                                                uint8_t qend, uint8_t cur_entry_pt)
{
    RMWREG32(base + ACMP_IT_MODE_CFG, ACMP_IT_MODE_CFG_QSTART, 5, qstart & 0x1F);
    RMWREG32(base + ACMP_IT_MODE_CFG, ACMP_IT_MODE_CFG_QEND, 5, qend & 0x1F);
    RMWREG32(base + ACMP_IT_MODE_CFG, ACMP_IT_MODE_CFG_CUR_ENTRY_PT, 5,
             cur_entry_pt & 0x1F);
}

/**
 * @brief sdrv acmp config timer.
 *
 * @param[in] base sdrv acmp address
 * @compare_val acmp timer compare value
 * @terminal_val acmp timer terminal value
 */
static inline void sdrv_acmp_lld_config_timer(paddr_t base, uint16_t compare_val,
                                              uint16_t terminal_val)
{
    RMWREG32(base + ACMP_TIMER, ACMP_TIMER_COMPARE_VAL, 16, compare_val & 0xFFFF);
    RMWREG32(base + ACMP_TIMER, ACMP_TIMER_TERMINAL_VAL, 16, terminal_val & 0xFFFF);
}

/**
 * @brief sdrv acmp start it generate.
 *
 * @param[in] base sdrv acmp address
 * @param[in] tg_start acmp it generate start
 */
static inline void sdrv_acmp_lld_it_gen_start(paddr_t base, bool tg_start)
{
    RMWREG32(base + ACMP_IT_MODE_CFG, ACMP_IT_MODE_CFG_TG_START, 1, tg_start);
}

/**
 * @brief sdrv acmp config csel
 *
 * @param[in] base sdrv acmp address
 * @param[in] csel_cfg sdrv acmp csel cfg
 */
static inline void sdrv_acmp_lld_config_csel(paddr_t base,
                                             sdrv_acmp_csel_cfg_t *csel_cfg)
{
    uint32_t val = 0;

    /* acmp ctrl cfg*/
    val = (csel_cfg->sync_en << ACMP_CTRL_CFG_SYNC_EN) |
          (csel_cfg->pol_adj << ACMP_CTRL_CFG_POL_ADJ) |
          (csel_cfg->mask_en << ACMP_CTRL_CFG_MASK_EN) |
          (csel_cfg->sample_en << ACMP_CTRL_CFG_SAMPLE_EN) |
          (csel_cfg->fbw << ACMP_CTRL_CFG_FBW) |
          (csel_cfg->rbw << ACMP_CTRL_CFG_RBW) |
          (csel_cfg->prop_delay << ACMP_CTRL_CFG_PROP_DLY) |
          (csel_cfg->amux_setup << ACMP_CTRL_CFG_AMUX_SETUP);

    writel(val, base + ACMP_CTRL_CFG(csel_cfg->csel_idx));

    /* acmp ana cfg*/
    val = (csel_cfg->dain << ACMP_ANA_CFG_DAIN) |
          (csel_cfg->hcfg << ACMP_ANA_CFG_HCFG) |
          (csel_cfg->sdsel << ACMP_ANA_CFG_SDSEL) |
          (csel_cfg->refsel << ACMP_ANA_CFG_REFSEL);

    writel(val, base + ACMP_ANA_CFG(csel_cfg->csel_idx));
}

/**
 * @brief sdrv acmp config index
 *
 * @param[in] base sdrv acmp address
 * @param[in] index_cfg sdrv acmp index cfg
 */
static inline void sdrv_acmp_lld_config_index(paddr_t base,
                                              sdrv_acmp_index_cfg_t *index_cfg)
{
    uint32_t val = 0;

    val = (index_cfg->uie << ACMP_CFG_INDEX_UIE) |
          (index_cfg->ie << ACMP_CFG_INDEX_IE) |
          (index_cfg->csel << ACMP_CFG_INDEX_CSEL) |
          (index_cfg->amsel << ACMP_CFG_INDEX_AMSEL);

    writel(val, base + ACMP_CFG_INDEX(index_cfg->idx));
}

/**
 * @brief sdrv acmp get irq state.
 *
 * @param[in] base sdrv acmp address
 * @return irq status
 */
static inline uint32_t sdrv_acmp_lld_get_irq_state(paddr_t base)
{
    return readl(base + ACMP_IRQ);
}

/**
 * @brief sdrv acmp clear irq.
 *
 * @param[in] base sdrv acmp address
 * @param[in] state irq state
 */
static inline void sdrv_acmp_lld_clear_irq(paddr_t base, uint32_t state)
{
    writel(state, base + ACMP_IRQ);
}

/**
 * @brief sdrv acmp get irq status.
 *
 * @param[in] base sdrv acmp address
 * @return init done
 */
static inline bool sdrv_acmp_lld_get_init_done(paddr_t base)
{
    return ((readl(base + ACMP_INIT) >> ACMP_INIT_DONE) & 0x1);
}

/**
 * @brief sdrv acmp set low power req_n delay num.
 *
 * @param[in] base sdrv acmp address
 * @param[in] req_n_dly req_n delay number
 */
static inline void sdrv_acmp_lld_low_power(paddr_t base, uint16_t req_n_dly)
{
    writel(req_n_dly & 0xFFFF, base + ACMP_LOW_POWER);
}

/**
 * @brief sdrv acmp power down analog competitor.
 *
 * @param[in] base sdrv acmp address
 * @param[in] down 0 power on (default), 1 power down.
 */
static inline void sdrv_acmp_lld_power_down_cmp(paddr_t base, bool down)
{
    RMWREG32(base + ACMP_PD_CTRL, ACMP_PD_CTRL_CMP, 1, down);
}

/**
 * @brief sdrv acmp power down analog dac.
 *
 * @param[in] base sdrv acmp address
 * @param[in] down 0 power on (default), 1 power down.
 */
static inline void sdrv_acmp_lld_power_down_dac(paddr_t base, bool down)
{
    RMWREG32(base + ACMP_PD_CTRL, ACMP_PD_CTRL_DAC, 1, down);
}

/**
 * @brief sdrv acmp dac recieve q channel low power requset.
 *
 * @param[in] base sdrv acmp address
 * @param[in] enable cmp recieve q channel low power requset
 */
static inline void sdrv_acmp_lld_rcv_qch_cmp(paddr_t base, bool enable)
{
    RMWREG32(base + ACMP_PD_CTRL, ACMP_PD_CTRL_CMP, 1, enable);
}

/**
 * @brief sdrv acmp comperitor recieve q channel low power requset.
 *
 * @param[in] base sdrv acmp address
 * @param[in] enable dac recieve q channel low power requset
 */
static inline void sdrv_acmp_lld_rcv_qch_dac(paddr_t base, bool enable)
{
    RMWREG32(base + ACMP_PD_CTRL, ACMP_PD_CTRL_DAC, 1, enable);
}

/**
 * @brief sdrv acmp init
 *
 * @param[in] base sdrv acmp address
 */
static inline void sdrv_acmp_lld_init(paddr_t base)
{
    /*soft reset */
    sdrv_acmp_lld_soft_reset(base);

    /* wait init done */
    while (!sdrv_acmp_lld_get_init_done(base));
}

/**
 * @brief sdrv acmp sch start
 *
 * @param[in] base sdrv acmp address
 * @param[in] start sdrv acmp sch start
 */
static inline void sdrv_acmp_lld_start(paddr_t base, bool start)
{
    RMWREG32(base + ACMP_SCH, ACMP_SCH_ANA_INSEL_ON, 1, 0);
    RMWREG32(base + ACMP_SCH, ACMP_SCH_ACMP_EN, 1, start);
}

/**
 * @brief sdrv acmp deinit
 *
 * @param[in] base sdrv acmp address
 */
static inline void sdrv_acmp_lld_deinit(paddr_t base)
{
    sdrv_acmp_lld_start(base, false);
}

__END_CDECLS

#endif /* ASSEMBLY */

#endif /* DRV_LLD_SDRV_ACMP_H */
