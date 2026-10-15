/**
 * @file: sdrv_etimer.c
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * @Author:
 * @Descripttion: taishan smc lld driver
 * @Revision History:
 * ----------------
 */
#include <regs_base.h>
#include <stdio.h>
#include <string.h>

#include "sdrv_smc.h"

#include "reg.h"
#include "types.h"
/******************global variable definition******************/
static bool ap_dom_off = false;
uint8_t saf_dom_lp_mode = SLP_MODE;
uint8_t ap_dom_lp_mode = SLP_MODE;

/******************function definition******************/
/**
 * @brief sdrv smc write reg
 *
 * @param[in] base reg base
 * @param[in] offset reg offset
 * @param[in] val value written into reg
 */
static void sdrv_smc_lld_writel(paddr_t base, uint32_t offset, uint32_t val)
{
    writel(val, base + offset);
}

/**
 * @brief sdrv smc read reg
 *
 * @param[in] base reg base
 * @param[in] offset reg offset
 * @return value read from reg
 */
static volatile uint32_t sdrv_smc_lld_readl(paddr_t base, uint32_t offset)
{
    return readl(base + offset);
}

/**********************************
 * rs domain register configration *
 **********************************/
/**
 * @brief rule space permission configuration for each domain
 *
 * @param[in] base reg base
 * @param[in] rs_idx rule space index, range 0~7
 * @param[in] dom_idx domain index, range 0~7
 * @param[in] usr_per user access permission
 * @param[in] pri_per privileged access permission
 * @param[in] non_sec_per non-secure access permission
 * @param[in] sec_per secure access permission
 *
 * @return -1 if rs_idx,dom_idx out of range, otherwise 0
 */
#define GET_DOM_PRE_OFF(m, n) (DOM_PER##m##_OFF(n))

#define GET_FM_PER_DOM_PER(m, n)                                               \
    (FM_DOM_PER##m##_DOM##n##_USE_PER | FM_DOM_PER##m##_DOM##n##_PRI_PER |     \
     FM_DOM_PER##m##_DOM##n##_NSE_PER | FM_DOM_PER##m##_DOM##n##_SEC_PER)

#define GET_FV_PER_DOM_PER(m, n, usr_p, pri_p, nse_p, sec_p)                   \
    (FV_DOM_PER##m##_DOM##n##_USE_PER(usr_p) |                                 \
     FV_DOM_PER##m##_DOM##n##_PRI_PER(pri_p) |                                 \
     FV_DOM_PER##m##_DOM##n##_NSE_PER(nse_p) |                                 \
     FV_DOM_PER##m##_DOM##n##_SEC_PER(sec_p))

static int rs_dom_permission_config(paddr_t base, uint32_t rs_idx,
                                    uint32_t dom_idx, uint8_t usr_per,
                                    uint8_t pri_per, uint8_t non_sec_per,
                                    uint8_t sec_per)
{
    int ret = -1;

    if ((rs_idx < 8) && (dom_idx < 8)) {
        uint32_t dom_per_off = 0;

        if (dom_idx < 4) {
            dom_per_off = GET_DOM_PRE_OFF(0, rs_idx);
        } else {
            dom_per_off = GET_DOM_PRE_OFF(1, rs_idx);
        }

        uint32_t dom_per = sdrv_smc_lld_readl(base, dom_per_off);

        if (dom_idx < 4) {
            if (dom_idx == 0) {
                dom_per &= ~GET_FM_PER_DOM_PER(0, 0);
                dom_per |= GET_FV_PER_DOM_PER(0, 0, usr_per, pri_per,
                                              non_sec_per, sec_per);
            } else if (dom_idx == 1) {
                dom_per &= ~GET_FM_PER_DOM_PER(0, 1);
                dom_per |= GET_FV_PER_DOM_PER(0, 1, usr_per, pri_per,
                                              non_sec_per, sec_per);
            } else if (dom_idx == 2) {
                dom_per &= ~GET_FM_PER_DOM_PER(0, 2);
                dom_per |= GET_FV_PER_DOM_PER(0, 2, usr_per, pri_per,
                                              non_sec_per, sec_per);
            } else {
                dom_per &= ~GET_FM_PER_DOM_PER(0, 3);
                dom_per |= GET_FV_PER_DOM_PER(0, 3, usr_per, pri_per,
                                              non_sec_per, sec_per);
            }
        } else {
            if (dom_idx == 4) {
                dom_per &= ~GET_FM_PER_DOM_PER(1, 4);
                dom_per |= GET_FV_PER_DOM_PER(1, 4, usr_per, pri_per,
                                              non_sec_per, sec_per);
            } else if (dom_idx == 5) {
                dom_per &= ~GET_FM_PER_DOM_PER(1, 5);
                dom_per |= GET_FV_PER_DOM_PER(1, 5, usr_per, pri_per,
                                              non_sec_per, sec_per);
            } else if (dom_idx == 6) {
                dom_per &= ~GET_FM_PER_DOM_PER(1, 6);
                dom_per |= GET_FV_PER_DOM_PER(1, 6, usr_per, pri_per,
                                              non_sec_per, sec_per);
            } else {
                dom_per &= ~GET_FM_PER_DOM_PER(1, 7);
                dom_per |= GET_FV_PER_DOM_PER(1, 7, usr_per, pri_per,
                                              non_sec_per, sec_per);
            }
        }

        sdrv_smc_lld_writel(base, dom_per_off, dom_per);

        ret = 0;
    }

    return ret;
}

/**
 * @brief safe domain rule space configuration
 *
 * @param[in] base reg base
 * @param[in] rs_idx rule space index, range 0~7
 * @param[in] did_en domain id assign enable
 * @param[in] lock lock domain register
 *
 * @return -1 if rs_idx out of range, otherwise 0
 */
static int saf_dom_rs_config(paddr_t base, uint8_t rs_idx, bool did_en,
                             bool lock)
{
    int ret = -1;

    if (rs_idx < 8) {
        uint32_t v = sdrv_smc_lld_readl(base, SAF_DOM_RS_OFF);

        v &= ~FM_SAF_DOM_RS_RS;
        v |= FV_SAF_DOM_RS_RS(rs_idx);

        if (did_en) {
            v |= BM_SAF_DOM_RS_EN;
        } else {
            v &= ~BM_SAF_DOM_RS_EN;
        }

        if (lock) {
            v |= BM_SAF_DOM_RS_LOCK;
        } else {
            v &= ~BM_SAF_DOM_RS_LOCK;
        }

        sdrv_smc_lld_writel(base, SAF_DOM_RS_OFF, v);

        ret = 0;
    }

    return ret;
}

/**
 * @brief ap domain rule space configuration
 *
 * @param[in] base reg base
 * @param[in] rs_idx rule space index, range 0~7
 * @param[in] did_en domain id assign enable
 * @param[in] lock lock domain register
 *
 * @return -1 if rs_idx out of range, otherwise 0
 */
static int ap_dom_rs_config(paddr_t base, uint8_t rs_idx, bool did_en,
                            bool lock)
{
    int ret = -1;

    if (rs_idx < 8) {
        uint32_t v = sdrv_smc_lld_readl(base, AP_DOM_RS_OFF);

        v &= ~FM_AP_DOM_RS_RS;
        v |= FV_AP_DOM_RS_RS(rs_idx);

        if (did_en) {
            v |= BM_AP_DOM_RS_EN;
        } else {
            v &= ~BM_AP_DOM_RS_EN;
        }

        if (lock) {
            v |= BM_AP_DOM_RS_LOCK;
        } else {
            v &= ~BM_AP_DOM_RS_LOCK;
        }

        sdrv_smc_lld_writel(base, AP_DOM_RS_OFF, v);

        ret = 0;
    }

    return ret;
}

/**
 * @brief soc domain rule space configuration
 *
 * @param[in] base reg base
 * @param[in] rs_idx rule space index, range 0~7
 * @param[in] did_en domain id assign enable
 * @param[in] lock lock domain register
 *
 * @return -1 if rs_idx out of range, otherwise 0
 */
static int soc_dom_rs_config(paddr_t base, uint8_t rs_idx, bool did_en,
                             bool lock)
{
    int ret = -1;

    if (rs_idx < 8) {
        uint32_t v = sdrv_smc_lld_readl(base, SOC_DOM_RS_OFF);

        v &= ~FM_SOC_DOM_RS_RS;
        v |= FV_SOC_DOM_RS_RS(rs_idx);

        if (did_en) {
            v |= BM_SOC_DOM_RS_EN;
        } else {
            v &= ~BM_SOC_DOM_RS_EN;
        }

        if (lock) {
            v |= BM_SOC_DOM_RS_LOCK;
        } else {
            v &= ~BM_SOC_DOM_RS_LOCK;
        }

        sdrv_smc_lld_writel(base, SOC_DOM_RS_OFF, v);

        ret = 0;
    }

    return ret;
}

/**
 * @brief soc clock monitor rule space configuration
 *
 * @param[in] base reg base
 * @param[in] rs_idx rule space index, range 0~7
 * @param[in] did_en domain id assign enable
 * @param[in] lock lock domain register
 *
 * @return -1 if rs_idx out of range, otherwise 0
 */

static int soc_cmom_rs_config(paddr_t base, uint8_t rs_idx, bool did_en,
                              bool lock)
{
    int ret = -1;

    if (rs_idx < 8) {
        uint32_t v = sdrv_smc_lld_readl(base, SOC_CMON_RS_OFF);

        v &= ~FM_SOC_CMON_RS_RS;
        v |= FV_SOC_CMON_RS_RS(rs_idx);

        if (did_en) {
            v |= BM_SOC_CMON_RS_EN;
        } else {
            v &= ~BM_SOC_CMON_RS_EN;
        }

        if (lock) {
            v |= BM_SOC_CMON_RS_LOCK;
        } else {
            v &= ~BM_SOC_CMON_RS_LOCK;
        }

        sdrv_smc_lld_writel(base, SOC_CMON_RS_OFF, v);

        ret = 0;
    }

    return ret;
}

/**
 * @brief core irq mask rule space configuration
 *
 * @param[in] base reg base
 * @param[in] core_idx core index, range 0~4
 * @param[in] rs_idx rule space index, range 0~7
 * @param[in] did_en domain id assign enable
 * @param[in] lock lock domain register
 *
 * @return -1 if rs_idx,core_idx out of range, otherwise 0
 */
static int core_irq_mask_rs_config(paddr_t base, uint8_t core_idx,
                                   uint8_t rs_idx, bool did_en, bool lock)
{
    int ret = -1;

    if ((core_idx < 5) && (rs_idx < 8)) {
        uint32_t v = sdrv_smc_lld_readl(base, CORE_IRQ_MASK_RS_OFF(core_idx));

        v &= ~FM_CORE_IRQ_MASK_RS_RS;
        v |= FV_CORE_IRQ_MASK_RS_RS(rs_idx);

        if (did_en) {
            v |= BM_CORE_IRQ_MASK_RS_EN;
        } else {
            v &= ~BM_CORE_IRQ_MASK_RS_EN;
        }

        if (lock) {
            v |= BM_CORE_IRQ_MASK_RS_LOCK;
        } else {
            v &= ~BM_CORE_IRQ_MASK_RS_LOCK;
        }

        sdrv_smc_lld_writel(base, CORE_IRQ_MASK_RS_OFF(core_idx), v);

        ret = 0;
    }

    return ret;
}

/**
 * @brief common irq mask rule space configuration
 *
 * @param[in] base reg base
 * @param[in] irq_grp_idx common irq group index, range 0~7
 * @param[in] rs_idx rule space index, range 0~7
 * @param[in] did_en domain id assign enable
 * @param[in] lock lock domain register
 *
 * @return -1 if rs_idx,irq_grp_idx out of range, otherwise 0
 */
static int com_irq_mask_rs_config(paddr_t base, uint8_t irq_grp_idx,
                                  uint8_t rs_idx, bool did_en, bool lock)
{
    int ret = -1;

    if ((irq_grp_idx < 8) && (rs_idx < 8)) {
        uint32_t v = sdrv_smc_lld_readl(base, COM_IRQ_MASK_RS_OFF(irq_grp_idx));

        v &= ~FM_COM_IRQ_MASK_RS_RS;
        v |= FV_COM_IRQ_MASK_RS_RS(rs_idx);

        if (did_en) {
            v |= BM_COM_IRQ_MASK_RS_EN;
        } else {
            v &= ~BM_COM_IRQ_MASK_RS_EN;
        }

        if (lock) {
            v |= BM_COM_IRQ_MASK_RS_LOCK;
        } else {
            v &= ~BM_COM_IRQ_MASK_RS_LOCK;
        }

        sdrv_smc_lld_writel(base, COM_IRQ_MASK_RS_OFF(irq_grp_idx), v);

        ret = 0;
    }

    return ret;
}

/**
 * @brief irq monitor mask rule space configuration
 *
 * @param[in] base reg base
 * @param[in] rs_idx rule space index, range 0~7
 * @param[in] did_en domain id assign enable
 * @param[in] lock lock domain register
 *
 * @return -1 if rs_idx out of range, otherwise 0
 */
static int irq_mon_rs_config(paddr_t base, uint8_t rs_idx, bool did_en,
                             bool lock)
{
    int ret = -1;

    if (rs_idx < 8) {
        uint32_t v = sdrv_smc_lld_readl(base, IRQ_MON_RS_OFF);

        v &= ~FM_IRQ_MON_RS_RS;
        v |= FV_IRQ_MON_RS_RS(rs_idx);

        if (did_en) {
            v |= BM_IRQ_MON_RS_EN;
        } else {
            v &= ~BM_IRQ_MON_RS_EN;
        }

        if (lock) {
            v |= BM_IRQ_MON_RS_LOCK;
        } else {
            v &= ~BM_IRQ_MON_RS_LOCK;
        }

        sdrv_smc_lld_writel(base, IRQ_MON_RS_OFF, v);

        ret = 0;
    }

    return ret;
}

/**
 * @brief smc reserve rule space configuration
 *
 * @param[in] base reg base
 * @param[in] rs_idx rule space index, range 0~7
 * @param[in] did_en domain id assign enable
 * @param[in] lock lock domain register
 *
 * @return -1 if rs_idx out of range, otherwise 0
 */
static int smc_res_rs_config(paddr_t base, uint8_t rs_idx, bool did_en,
                             bool lock)
{
    int ret = -1;

    if (rs_idx < 8) {
        uint32_t v = sdrv_smc_lld_readl(base, SMC_RES_RS_OFF);

        v &= ~FM_SMC_RES_RS_RS;
        v |= FV_SMC_RES_RS_RS(rs_idx);

        if (did_en) {
            v |= BM_SMC_RES_RS_EN;
        } else {
            v &= ~BM_SMC_RES_RS_EN;
        }

        if (lock) {
            v |= BM_SMC_RES_RS_LOCK;
        } else {
            v &= ~BM_SMC_RES_RS_LOCK;
        }

        sdrv_smc_lld_writel(base, SMC_RES_RS_OFF, v);

        ret = 0;
    }

    return ret;
}

/**
 * @brief smc misc rule space configuration
 *
 * @param[in] base reg base
 * @param[in] rs_idx rule space index, range 0~7
 * @param[in] did_en domain id assign enable
 * @param[in] lock lock domain register
 *
 * @return -1 if rs_idx out of range, otherwise 0
 */

static int smc_misc_rs_config(paddr_t base, uint8_t rs_idx, bool did_en,
                              bool lock)
{
    int ret = -1;

    if (rs_idx < 8) {
        uint32_t v = sdrv_smc_lld_readl(base, SMC_MISC_RS_OFF);

        v &= ~FM_SMC_MISC_RS_RS;
        v |= FV_SMC_MISC_RS_RS(rs_idx);

        if (did_en) {
            v |= BM_SMC_MISC_RS_EN;
        } else {
            v &= ~BM_SMC_MISC_RS_EN;
        }

        if (lock) {
            v |= BM_SMC_MISC_RS_LOCK;
        } else {
            v &= ~BM_SMC_MISC_RS_LOCK;
        }

        sdrv_smc_lld_writel(base, SMC_MISC_RS_OFF, v);

        ret = 0;
    }

    return ret;
}

/**
 * @brief smc fusa rule space configuration
 *
 * @param[in] base reg base
 * @param[in] rs_idx rule space index, range 0~7
 * @param[in] did_en domain id assign enable
 * @param[in] lock lock domain register
 *
 * @return -1 if rs_idx out of range, otherwise 0
 */
static int smc_fusa_rs_config(paddr_t base, uint8_t rs_idx, bool did_en,
                              bool lock)
{
    int ret = -1;

    if (rs_idx < 8) {
        uint32_t v = sdrv_smc_lld_readl(base, SMC_FUSA_RS_OFF);

        v &= ~FM_SMC_FUSA_RS_RS;
        v |= FV_SMC_FUSA_RS_RS(rs_idx);

        if (did_en) {
            v |= BM_SMC_FUSA_RS_EN;
        } else {
            v &= ~BM_SMC_FUSA_RS_EN;
        }

        if (lock) {
            v |= BM_SMC_FUSA_RS_LOCK;
        } else {
            v &= ~BM_SMC_FUSA_RS_LOCK;
        }

        sdrv_smc_lld_writel(base, SMC_FUSA_RS_OFF, v);

        ret = 0;
    }

    return ret;
}

/**
 * @brief smc func interrupt space configuration
 *
 * @param[in] base reg base
 * @param[in] rs_idx rule space index, range 0~7
 * @param[in] did_en domain id assign enable
 * @param[in] lock lock domain register
 *
 * @return -1 if rs_idx out of range, otherwise 0
 */
static int smc_func_int_rs_config(paddr_t base, uint8_t rs_idx, bool did_en,
                                  bool lock)
{
    int ret = -1;

    if (rs_idx < 8) {
        uint32_t v = sdrv_smc_lld_readl(base, SMC_FUNC_INT_RS_OFF);

        v &= ~FM_SMC_FUNC_INT_RS_RS;
        v |= FV_SMC_FUNC_INT_RS_RS(rs_idx);

        if (did_en) {
            v |= BM_SMC_FUNC_INT_RS_EN;
        } else {
            v &= ~BM_SMC_FUNC_INT_RS_EN;
        }

        if (lock) {
            v |= BM_SMC_FUNC_INT_RS_LOCK;
        } else {
            v &= ~BM_SMC_FUNC_INT_RS_LOCK;
        }

        sdrv_smc_lld_writel(base, SMC_FUNC_INT_RS_OFF, v);

        ret = 0;
    }

    return ret;
}
/**********************************
 * rs domain register configration *
 ***********************************/

/***********************************
 * saf domain register configration *
 ***********************************/

/**
 * @brief safe domain wfi configuration
 *
 * @param[in] base reg base
 * @param[in] wfi_en wfi enable or not
 * @param[in] lpm saf dom low power mode, slp or hib
 */
static void saf_wfi_config(paddr_t base, bool wfi_en, domain_lp_mode lpm)
{
    saf_dom_lp_mode = lpm;
    uint32_t v = sdrv_smc_lld_readl(base, SAF_GLB_CTL_OFF);

    if (wfi_en) {
        v &= ~BM_SAF_GLB_CTL_WFI_DIS;
    } else {
        v |= BM_SAF_GLB_CTL_WFI_DIS;
    }

    if (SLP_MODE == lpm) {
        v &= ~BM_SAF_GLB_CTL_LP_MODE;
    } else {
        v |= BM_SAF_GLB_CTL_LP_MODE;
    }

    sdrv_smc_lld_writel(base, SAF_GLB_CTL_OFF, v);
}

/**
 * @brief safe domain wfi configuration
 *
 * @param[in] base reg base
 * @param[in] pri_core safe domain primary core
 */
static void saf_pri_core_config(paddr_t base, uint8_t pri_core)
{
    uint32_t v = sdrv_smc_lld_readl(base, SAF_GLB_CTL_OFF);

    v &= ~FM_SAF_GLB_CTL_PRI_CORE;
    v |= FV_SAF_GLB_CTL_PRI_CORE(pri_core);

    sdrv_smc_lld_writel(base, SAF_GLB_CTL_OFF, v);
}

/**
 * @brief safe domain low power configuration
 *
 * @param[in] base reg base
 * @param[in] sub_dom_idx saf power domain index,range 0~3
 * @param[in] pd_en power down enable
 *
 * @return -1 if sub_dom_idx out of range, otherwise 0
 */
static int saf_lp_config(paddr_t base, uint8_t sub_dom_idx, bool pd_en)
{
    int ret = -1;

    if (sub_dom_idx < 4) {
        uint32_t v = sdrv_smc_lld_readl(base, SAF_LP_CTL_OFF(sub_dom_idx));

        if (pd_en) {
            if (HIB_MODE == saf_dom_lp_mode) {
                v |= BM_SAF_LP_CTL_HIB_PD_EN;
            } else {
                v |= BM_SAF_LP_CTL_SLP_PD_EN;
            }
        } else {
            if (HIB_MODE == saf_dom_lp_mode) {
                v &= ~BM_SAF_LP_CTL_HIB_PD_EN;
            } else {
                v &= ~BM_SAF_LP_CTL_SLP_PD_EN;
            }
        }

        sdrv_smc_lld_writel(base, SAF_LP_CTL_OFF(sub_dom_idx), v);

        ret = 0;
    }

    return ret;
}

/**
 * @brief safe domain low power delay configuration
 *
 * @param[in] base reg base
 * @param[in] sub_dom_idx saf power domain index,range 0~3
 * @param[in] iso_dis_dly isolation disable delay
 * @param[in] po_dly power on delay
 * @param[in] pg_dly power gate delay
 * @param[in] iso_en_dly isolation enable delay
 *
 * @return -1 if sub_dom_idx out of range, otherwise 0
 */
static int saf_lp_dly_config(paddr_t base, uint8_t sub_dom_idx,
                             uint8_t iso_dis_dly, uint8_t po_dly,
                             uint8_t pg_dly, uint8_t iso_en_dly)
{
    int ret = -1;

    if (sub_dom_idx < 4) {
        uint32_t v = sdrv_smc_lld_readl(base, SAF_LP_DLY_CTL_OFF(sub_dom_idx));

        v &= ~FM_SAF_LP_DLY_CTL_ISO_DIS;
        v &= ~FM_SAF_LP_DLY_CTL_PO;
        v &= ~FM_SAF_LP_DLY_CTL_PG;
        v &= ~FM_SAF_LP_DLY_CTL_ISO_EN;

        v |= FV_SAF_LP_DLY_CTL_ISO_DIS(iso_dis_dly);
        v |= FV_SAF_LP_DLY_CTL_PO(po_dly);
        v |= FV_SAF_LP_DLY_CTL_PG(pg_dly);
        v |= FV_SAF_LP_DLY_CTL_ISO_EN(iso_en_dly);

        sdrv_smc_lld_writel(base, SAF_LP_DLY_CTL_OFF(0), v);

        ret = 0;
    }

    return ret;
}

/**
 * @brief safe domain ram low power configuration under hib mode
 *
 * @param[in] base reg base
 * @param[in] mem_dom_idx saf power domain index,range 0~4
 * @param[in] ram_pd_en ram power down enable
 * @param[in] ram_setting ram low power setting
 *
 * @return -1 if mem_dom_idx out of range, otherwise 0
 */
static int saf_ram_pd_config(paddr_t base, bool ram_pd_en, uint8_t mem_dom_idx)
{
    int ret = -1;

    if (mem_dom_idx < 5) {
        uint32_t v = sdrv_smc_lld_readl(base, SAF_RAM_LP_CTL_OFF(mem_dom_idx));

        if (ram_pd_en) {
            if (HIB_MODE == saf_dom_lp_mode) {
                v |= BM_SAF_RAM_LP_CTL_HIB_RAM_PD_EN;
            } else {
                v |= BM_SAF_RAM_LP_CTL_SLP_RAM_PD_EN;
            }
        } else {
            if (HIB_MODE == saf_dom_lp_mode) {
                v &= ~BM_SAF_RAM_LP_CTL_HIB_RAM_PD_EN;
            } else {
                v &= ~BM_SAF_RAM_LP_CTL_SLP_RAM_PD_EN;
            }
        }

        sdrv_smc_lld_writel(base, SAF_RAM_LP_CTL_OFF(mem_dom_idx), v);

        ret = 0;
    }

    return ret;
}

/**
 * @brief safe domain ram low power configuration under hib mode
 *
 * @param[in] base reg base
 * @param[in] mem_dom_idx saf power domain index,range 0~4
 * @param[in] ram_setting ram low power setting
 *
 * @return -1 if mem_dom_idx out of range, otherwise 0
 */
static int saf_ram_pd_setting_config(paddr_t base, uint8_t ram_setting,
                                     uint8_t mem_dom_idx)
{
    int ret = -1;

    if (mem_dom_idx < 5) {
        uint32_t v = sdrv_smc_lld_readl(base, SAF_RAM_LP_CTL_OFF(mem_dom_idx));

        if (HIB_MODE == saf_dom_lp_mode) {
            v &= ~FM_SAF_RAM_LP_CTL_HIB_RAM_LP_SETTING;
            v |= FV_SAF_RAM_LP_CTL_HIB_RAM_LP_SETTING(ram_setting);
        } else {
            v &= ~FM_SAF_RAM_LP_CTL_SLP_RAM_LP_SETTING;
            v |= FV_SAF_RAM_LP_CTL_SLP_RAM_LP_SETTING(ram_setting);
        }

        sdrv_smc_lld_writel(base, SAF_RAM_LP_CTL_OFF(mem_dom_idx), v);

        ret = 0;
    }

    return ret;
}

/**
 * @brief safe domain timeout configuration
 *
 * @param[in] base reg base
 * @param[in] wdt_en saf domain handshake watchdog enable
 * @param[in] tout_stage timeout monitor stage, ckgen, rstgen or mode req trans
 * @param[in] tout_val timeout value
 *
 * @return -1 if tout_stage out of range, otherwise 0
 */
static int saf_timeout_config(paddr_t base, bool wdt_en,
                              handshake_stage tout_stage, uint8_t tout_val)
{
    int ret = 0;

    uint32_t v = sdrv_smc_lld_readl(base, SAF_TIMEOUT_OFF);

    if (wdt_en) {
        v |= BM_SAF_TIMEOUT_WDT_EN;

        if (CKGEN_HK == tout_stage) {
            v &= ~FM_SAF_TIMEOUT_CKGEN_HK;
            v |= FV_SAF_TIMEOUT_CKGEN_HK(tout_val);
        } else if (RSTGEN_HK == tout_stage) {
            v &= ~FM_SAF_TIMEOUT_RSTGEN_HK;
            v |= FV_SAF_TIMEOUT_RSTGEN_HK(tout_val);
        } else if (MODE_REQ_TRANS == tout_stage) {
            v &= ~FM_SAF_TIMEOUT_MODE_REQ_TRANS;
            v |= FV_SAF_TIMEOUT_MODE_REQ_TRANS(tout_val);
        } else {
            ret = -1;
        }
    } else {
        v &= ~BM_SAF_TIMEOUT_WDT_EN;
    }

    sdrv_smc_lld_writel(base, SAF_TIMEOUT_OFF, v);

    return ret;
}

/**
 * @brief safe domain timeout wakeup enable configuration
 *
 * @param[in] base reg base
 * @param[in] wkup_en safe domain timeout wakeup enable
 *
 */
static void saf_timeout_wkup_config(paddr_t base, bool wkup_en)
{
    uint32_t v = sdrv_smc_lld_readl(base, SAF_TIMEOUT_OFF);

    if (wkup_en) {
        v |= BM_SAF_TIMEOUT_WKUP_EN;
    } else {
        v &= ~BM_SAF_TIMEOUT_WKUP_EN;
    }

    sdrv_smc_lld_writel(base, SAF_TIMEOUT_OFF, v);
}

/***********************************
 * saf domain register configration *
 ***********************************/

/**********************************
 * ap domain register configration *
 **********************************/

/**
 * @brief ap domain wfi configuration
 *
 * @param[in] base reg base
 * @param[in] wfi_en ap domain wfi enable
 * @param[in] lpm ap domain low power mode
 *
 */
static void ap_wfi_config(paddr_t base, bool wfi_en, domain_lp_mode lpm)
{
    ap_dom_lp_mode = lpm;
    uint32_t v = sdrv_smc_lld_readl(base, AP_GLB_CTL_OFF);

    if (wfi_en) {
        v &= ~BM_AP_GLB_CTL_WFI_DIS;
    } else {
        v |= BM_AP_GLB_CTL_WFI_DIS;
    }

    if (SLP_MODE == lpm) {
        v &= ~BM_AP_GLB_CTL_LP_MODE;
    } else {
        v |= BM_AP_GLB_CTL_LP_MODE;
    }

    sdrv_smc_lld_writel(base, AP_GLB_CTL_OFF, v);
}

/**
 * @brief ap domain lp align with saf domain configuration
 *
 * @param[in] base reg base
 * @param[in] lp_align2sf ap domain lp align with saf domain
 *
 */
static void ap_lp_align_saf_config(paddr_t base, bool lp_align2sf)
{
    uint32_t v = sdrv_smc_lld_readl(base, AP_GLB_CTL_OFF);

    if (lp_align2sf) {
        v |= BM_AP_GLB_CTL_LP_ALIGN2SAF;
    } else {
        v &= ~BM_AP_GLB_CTL_LP_ALIGN2SAF;
    }

    sdrv_smc_lld_writel(base, AP_GLB_CTL_OFF, v);
}

/**
 * @brief ap domain wakeup align with saf domain configuration
 *
 * @param[in] base reg base
 * @param[in] wkup_align2sf ap domain wakeup align with saf domain enable
 *
 */
static void ap_wkup_align_saf_config(paddr_t base, bool wkup_align2sf)
{
    uint32_t v = sdrv_smc_lld_readl(base, AP_GLB_CTL_OFF);

    if (wkup_align2sf) {
        v |= BM_AP_GLB_CTL_WKUP_ALIGN2SAF;
    } else {
        v &= ~BM_AP_GLB_CTL_WKUP_ALIGN2SAF;
    }

    sdrv_smc_lld_writel(base, AP_GLB_CTL_OFF, v);
}

/**
 * @brief ap domain primary core configuration
 *
 * @param[in] base reg base
 * @param[in] pri_core ap domain primary core
 *
 */
static void ap_pri_core_config(paddr_t base, uint8_t pri_core)
{
    uint32_t v = sdrv_smc_lld_readl(base, AP_GLB_CTL_OFF);

    v &= ~FM_AP_GLB_CTL_PRI_CORE;
    v |= FV_AP_GLB_CTL_PRI_CORE(pri_core);

    sdrv_smc_lld_writel(base, AP_GLB_CTL_OFF, v);
}

/**
 * @brief ap domain low power configuration
 *
 * @param[in] base reg base
 * @param[in] pd_en power down enable
 *
 */
static void ap_lp_config(paddr_t base, bool pd_en)
{
    uint32_t v = sdrv_smc_lld_readl(base, AP_LP_CTL_OFF(0));

    if (pd_en) {
        if (HIB_MODE == ap_dom_lp_mode) {
            v |= BM_AP_LP_CTL_HIB_PD_EN;
        } else {
            v |= BM_AP_LP_CTL_SLP_PD_EN;
        }
    } else {
        if (HIB_MODE == ap_dom_lp_mode) {
            v &= ~BM_AP_LP_CTL_HIB_PD_EN;
        } else {
            v &= ~BM_AP_LP_CTL_SLP_PD_EN;
        }
    }

    sdrv_smc_lld_writel(base, AP_LP_CTL_OFF(0), v);
}

/**
 * @brief safe domain low power delay configuration
 *
 * @param[in] base reg base
 * @param[in] iso_dis_dly isolation disable delay
 * @param[in] po_dly power on delay
 * @param[in] pg_dly power gate delay
 * @param[in] iso_en_dly isolation enable delay
 *
 */
static void ap_lp_dly_config(paddr_t base, uint8_t iso_dis_dly, uint8_t po_dly,
                             uint8_t pg_dly, uint8_t iso_en_dly)
{
    uint8_t v = sdrv_smc_lld_readl(base, AP_LP_DLY_CTL_OFF(0));

    v &= ~FM_AP_LP_DLY_CTL_ISO_DIS;
    v &= ~FM_AP_LP_DLY_CTL_PO;
    v &= ~FM_AP_LP_DLY_CTL_PG;
    v &= ~FM_AP_LP_DLY_CTL_ISO_EN;

    v |= FV_AP_LP_DLY_CTL_ISO_DIS(iso_dis_dly);
    v |= FV_AP_LP_DLY_CTL_PO(po_dly);
    v |= FV_AP_LP_DLY_CTL_PG(pg_dly);
    v |= FV_AP_LP_DLY_CTL_ISO_EN(iso_en_dly);

    sdrv_smc_lld_writel(base, AP_LP_DLY_CTL_OFF(0), v);
}

/**
 * @brief ap domain ram low power configuration
 *
 * @param[in] base reg base
 * @param[in] mem_dom_idx saf power domain index,range 0~1
 * @param[in] ram_pd_en ram power down enable
 *
 * @return -1 if mem_dom_idx out of range, otherwise 0
 */
static int ap_ram_pd_config(paddr_t base, bool ram_pd_en, uint8_t mem_dom_idx)
{
    int ret = -1;

    if (mem_dom_idx < 2) {
        uint32_t v = sdrv_smc_lld_readl(base, AP_RAM_LP_CTL_OFF(mem_dom_idx));

        if (ram_pd_en) {
            if (HIB_MODE == ap_dom_lp_mode) {
                v |= BM_AP_RAM_LP_CTL_HIB_RAM_PD_EN;
            } else {
                v |= BM_AP_RAM_LP_CTL_SLP_RAM_PD_EN;
            }
        } else {
            if (HIB_MODE == ap_dom_lp_mode) {
                v &= ~BM_AP_RAM_LP_CTL_HIB_RAM_PD_EN;
            } else {
                v &= ~BM_AP_RAM_LP_CTL_SLP_RAM_PD_EN;
            }
        }

        sdrv_smc_lld_writel(base, AP_RAM_LP_CTL_OFF(mem_dom_idx), v);

        ret = 0;
    }

    return ret;
}

/**
 * @brief ap domain ram low power configuration
 *
 * @param[in] base reg base
 * @param[in] mem_dom_idx saf power domain index,range 0~1
 * @param[in] ram_setting ram low power setting
 *
 * @return -1 if mem_dom_idx out of range, otherwise 0
 */
static int ap_ram_pd_setting_config(paddr_t base, uint8_t ram_setting,
                                    uint8_t mem_dom_idx)
{
    int ret = -1;

    if (mem_dom_idx < 2) {
        uint32_t v = sdrv_smc_lld_readl(base, AP_RAM_LP_CTL_OFF(mem_dom_idx));

        if (HIB_MODE == ap_dom_lp_mode) {
            v &= ~FM_AP_RAM_LP_CTL_HIB_RAM_LP_SETTING;
            v |= FV_AP_RAM_LP_CTL_HIB_RAM_LP_SETTING(ram_setting);
        } else {
            v &= ~FM_AP_RAM_LP_CTL_SLP_RAM_LP_SETTING;
            v |= FV_AP_RAM_LP_CTL_SLP_RAM_LP_SETTING(ram_setting);
        }

        sdrv_smc_lld_writel(base, AP_RAM_LP_CTL_OFF(mem_dom_idx), v);

        ret = 0;
    }

    return ret;
}

/**
 * @brief ap domain timeout configuration
 *
 * @param[in] base reg base
 * @param[in] wdt_en ap domain handshake watchdog enable
 * @param[in] tout_stage timeout monitor stage, ckgen, rstgen or mode req trans
 * @param[in] tout_val timeout value
 *
 * @return -1 if tout_stage out of range, otherwise 0
 */
static int ap_timeout_config(paddr_t base, bool wdt_en,
                             handshake_stage tout_stage, uint8_t tout_val)
{
    int ret = 0;

    uint32_t v = sdrv_smc_lld_readl(base, AP_TIMEOUT_OFF);

    if (wdt_en) {
        v |= BM_AP_TIMEOUT_WDT_EN;

        if (CKGEN_HK == tout_stage) {
            v &= ~FM_AP_TIMEOUT_CKGEN_HK;
            v |= FV_AP_TIMEOUT_CKGEN_HK(tout_val);
        } else if (RSTGEN_HK == tout_stage) {
            v &= ~FM_AP_TIMEOUT_RSTGEN_HK;
            v |= FV_AP_TIMEOUT_RSTGEN_HK(tout_val);
        } else if (MODE_REQ_TRANS == tout_stage) {
            v &= ~FM_AP_TIMEOUT_MODE_REQ_TRANS;
            v |= FV_AP_TIMEOUT_MODE_REQ_TRANS(tout_val);
        } else {
            ret = -1;
        }
    } else {
        v &= ~BM_AP_TIMEOUT_WDT_EN;
    }

    sdrv_smc_lld_writel(base, AP_TIMEOUT_OFF, v);

    return ret;
}

/**
 * @brief ap domain timeout wakeup enable configuration
 *
 * @param[in] base reg base
 * @param[in] wkup_en ap domain timeout wakeup enable
 *
 */
static void ap_timeout_wkup_config(paddr_t base, bool wkup_en)
{
    uint32_t v = sdrv_smc_lld_readl(base, AP_TIMEOUT_OFF);

    if (wkup_en) {
        v |= BM_AP_TIMEOUT_WKUP_EN;
    } else {
        v &= ~BM_AP_TIMEOUT_WKUP_EN;
    }

    sdrv_smc_lld_writel(base, AP_TIMEOUT_OFF, v);
}

/**********************************
 * ap domain register configration *
 **********************************/

/***********************************
 *  irq mask register configration  *
 ***********************************/

/**
 * @brief core irq mask configuration
 *
 * @param[in] base reg base
 * @param[in] core_idx core index, range 0~4
 * @param[in] mask_val irq mask value
 *
 * @return -1 if core_idx out of range, otherwise 0
 */
static int core_irq_mask_config(paddr_t base, uint32_t core_idx,
                                uint16_t mask_val)
{
    int ret = -1;
    if (core_idx < 5u) {
        sdrv_smc_lld_writel(base, CORE_IRQ_MASK_OFF(core_idx), mask_val);
        ret = 0;
    }
    return ret;
}

/**
 * @brief common irq mask configuration
 *
 * @param[in] base reg base
 * @param[in] core_idx core index, range 0~4
 * @param[in] irq_grp_idx core index, range 0~7
 * @param[in] mask_val irq mask value
 *
 * @return -1 if core_idx,irq_grp_idx out of range, otherwise 0
 */

#define GET_COM_IRQ_MASK_BASE(m, n) (COM_IRQ_MASK_##m##_OFF(n))

static int com_irq_mask_config(paddr_t base, uint32_t core_idx,
                               uint32_t irq_grp_idx, uint32_t mask_val)
{
    int ret = -1;

    if ((core_idx < 5u) && (irq_grp_idx < 8u)) {
        uint32_t addr;

        if (irq_grp_idx == 0) {
            addr = base + GET_COM_IRQ_MASK_BASE(0, core_idx);
        } else if (irq_grp_idx == 1) {
            addr = base + GET_COM_IRQ_MASK_BASE(1, core_idx);
        } else if (irq_grp_idx == 2) {
            addr = base + GET_COM_IRQ_MASK_BASE(2, core_idx);
        } else if (irq_grp_idx == 3) {
            addr = base + GET_COM_IRQ_MASK_BASE(3, core_idx);
        } else if (irq_grp_idx == 4) {
            addr = base + GET_COM_IRQ_MASK_BASE(4, core_idx);
        } else if (irq_grp_idx == 5) {
            addr = base + GET_COM_IRQ_MASK_BASE(5, core_idx);
        } else if (irq_grp_idx == 6) {
            addr = base + GET_COM_IRQ_MASK_BASE(6, core_idx);
        } else if (irq_grp_idx == 7) {
            addr = base + GET_COM_IRQ_MASK_BASE(7, core_idx);
        }

        sdrv_smc_lld_writel(addr, 0, mask_val);
        ret = 0;
    }

    return ret;
}

/**
 * @brief core irq monitor configuration
 *
 * @param[in] base reg base
 * @param[in] core_idx core index, range 0~4
 * @param[in] irq_state pointer to core irq state
 *
 * @return -1 if core_idx out of range, otherwise 0
 */
static int core_irq_monitor(paddr_t base, uint32_t core_idx,
                            uint32_t *irq_state)
{
    int ret = -1;

    if ((core_idx < 5) && (irq_state != NULL)) {
        *irq_state = sdrv_smc_lld_readl(base, CORE_IRQ_MON_OFF(core_idx));
        ret = 0;
    }

    return ret;
}

/**
 * @brief core irq clear configuration
 *
 * @param[in] base reg base
 * @param[in] core_idx core index, range 0~4
 * @param[in] clr_bit_mask for each bit, 1 means not clear the corresponding bit
 * and 0 means clear
 *
 * @return -1 if core_idx out of range, otherwise 0
 */
static int core_irq_clear(paddr_t base, uint32_t core_idx,
                          uint32_t clr_bit_mask)
{
    int ret = -1;

    if (core_idx < 5) {
        uint32_t irq_state = 0;
        core_irq_monitor(base, core_idx, &irq_state);
        sdrv_smc_lld_writel(base, CORE_IRQ_MON_OFF(core_idx),
                            (clr_bit_mask & irq_state));
        ret = 0;
    }

    return ret;
}

/**
 * @brief common irq monitor configuration
 *
 * @param[in] base reg base
 * @param[in] irq_grp_idx irq group index, range 0~7
 * @param[in] irq_state pointer to common irq state
 *
 * @return -1 if irq_grp_idx out of range, otherwise 0
 */

#define GET_COM_IRQ_MON_BASE(n) (COM_IRQ_MON_##n##_OFF)

static int com_irq_monitor(paddr_t base, uint32_t irq_grp_idx,
                           uint32_t *irq_state)
{
    int ret = -1;

    if ((irq_grp_idx < 8) && (irq_state != NULL)) {
        if (irq_grp_idx == 0) {
            *irq_state = sdrv_smc_lld_readl(base, GET_COM_IRQ_MON_BASE(0));
        } else if (irq_grp_idx == 1) {
            *irq_state = sdrv_smc_lld_readl(base, GET_COM_IRQ_MON_BASE(1));
        } else if (irq_grp_idx == 2) {
            *irq_state = sdrv_smc_lld_readl(base, GET_COM_IRQ_MON_BASE(2));
        } else if (irq_grp_idx == 3) {
            *irq_state = sdrv_smc_lld_readl(base, GET_COM_IRQ_MON_BASE(3));
        } else if (irq_grp_idx == 4) {
            *irq_state = sdrv_smc_lld_readl(base, GET_COM_IRQ_MON_BASE(4));
        } else if (irq_grp_idx == 5) {
            *irq_state = sdrv_smc_lld_readl(base, GET_COM_IRQ_MON_BASE(5));
        } else if (irq_grp_idx == 6) {
            *irq_state = sdrv_smc_lld_readl(base, GET_COM_IRQ_MON_BASE(6));
        } else if (irq_grp_idx == 7) {
            *irq_state = sdrv_smc_lld_readl(base, GET_COM_IRQ_MON_BASE(7));
        }

        ret = 0;
    }

    return ret;
}

/**
 * @brief common irq clear configuration
 *
 * @param[in] base reg base
 * @param[in] irq_grp_idx core index, range 0~7
 * @param[in] clr_bit_mask for each bit, 1 means not clear the corresponding bit
 * and 0 means clear
 *
 * @return -1 if irq_grp_idx out of range, otherwise 0
 */
static int com_irq_clear(paddr_t base, uint32_t irq_grp_idx,
                         uint32_t clr_bit_mask)
{
    int ret = -1;

    if (irq_grp_idx < 8) {
        uint32_t irq_state = 0;
        com_irq_monitor(base, irq_grp_idx, &irq_state);

        if (irq_grp_idx == 0) {
            sdrv_smc_lld_writel(base, GET_COM_IRQ_MON_BASE(0),
                                (clr_bit_mask & irq_state));
        } else if (irq_grp_idx == 1) {
            sdrv_smc_lld_writel(base, GET_COM_IRQ_MON_BASE(1),
                                (clr_bit_mask & irq_state));
        } else if (irq_grp_idx == 2) {
            sdrv_smc_lld_writel(base, GET_COM_IRQ_MON_BASE(2),
                                (clr_bit_mask & irq_state));
        } else if (irq_grp_idx == 3) {
            sdrv_smc_lld_writel(base, GET_COM_IRQ_MON_BASE(3),
                                (clr_bit_mask & irq_state));
        } else if (irq_grp_idx == 4) {
            sdrv_smc_lld_writel(base, GET_COM_IRQ_MON_BASE(4),
                                (clr_bit_mask & irq_state));
        } else if (irq_grp_idx == 5) {
            sdrv_smc_lld_writel(base, GET_COM_IRQ_MON_BASE(5),
                                (clr_bit_mask & irq_state));
        } else if (irq_grp_idx == 6) {
            sdrv_smc_lld_writel(base, GET_COM_IRQ_MON_BASE(6),
                                (clr_bit_mask & irq_state));
        } else if (irq_grp_idx == 7) {
            sdrv_smc_lld_writel(base, GET_COM_IRQ_MON_BASE(7),
                                (clr_bit_mask & irq_state));
        }

        ret = 0;
    }

    return ret;
}
/***********************************
 *  irq mask register configration  *
 ***********************************/

/******************************
 *  soc register configration  *
 ******************************/
/**
 * @brief soc global configuration
 *
 * @param[in] base reg base
 * @param[in] ap_off ap domain power off
 *
 */
static void soc_global_config(paddr_t base, bool ap_off)
{
    ap_dom_off = ap_off;

    uint32_t v = sdrv_smc_lld_readl(base, SOC_GLB_CTL_OFF);

    if (ap_off) {
        v |= BM_SOC_GLB_CTL_AP_OFF;
    } else {
        v &= ~BM_SOC_GLB_CTL_AP_OFF;
    }

    sdrv_smc_lld_writel(base, SOC_GLB_CTL_OFF, v);
}

/**
 * @brief soc hib mode rc24m disable configuration
 *
 * @param[in] base reg base
 * @param[in] rc_dis_en rc24m disable in hib mode
 *
 */
static void soc_hib_rc_dis_config(paddr_t base, bool rc_dis_en)
{
    uint32_t v = sdrv_smc_lld_readl(base, SOC_HIB_RC_DIS_OFF);

    if (rc_dis_en) {
        v |= BM_SOC_HIB_RC_DIS_RC_DIS_EN;
    } else {
        v &= ~BM_SOC_HIB_RC_DIS_RC_DIS_EN;
    }

    sdrv_smc_lld_writel(base, SOC_HIB_RC_DIS_OFF, v);
}

/**
 * @brief soc pre divider number for 32k configuration
 *
 * @param[in] base reg base
 * @param[in] div_num pre div number for clk32k
 *
 */
static void soc_32k_div_num_config(paddr_t base, uint8_t div_num)
{
    uint32_t v = sdrv_smc_lld_readl(base, SOC_PRE_DIV_OFF);

    v &= ~FM_SOC_PRE_DIV_DIV_NUM_32K;
    v |= FV_SOC_PRE_DIV_DIV_NUM_32K(div_num);

    sdrv_smc_lld_writel(base, SOC_PRE_DIV_OFF, v);
}

/**
 * @brief soc pre divider number for 24m configuration
 *
 * @param[in] base reg base
 * @param[in] div_num pre div number for clk24k
 *
 */
static void soc_24m_div_num_config(paddr_t base, uint8_t div_num)
{
    uint32_t v = sdrv_smc_lld_readl(base, SOC_PRE_DIV_OFF);

    v &= ~FM_SOC_PRE_DIV_DIV_NUM_24M;
    v |= FV_SOC_PRE_DIV_DIV_NUM_24M(div_num);

    sdrv_smc_lld_writel(base, SOC_PRE_DIV_OFF, v);
}

/**
 * @brief soc swm timeout configuration
 *
 * @param[in] base reg base
 * @param[in] wdt_en pmu handshake watchdog enable
 * @param[in] pmu_hk timeout value for pmu handshake
 *
 */
static void soc_swm_timeout_config(paddr_t base, bool wdt_en, uint8_t pmu_hk)
{
    uint32_t v = sdrv_smc_lld_readl(base, SOC_SWM_TIMEOUT_OFF);

    if (wdt_en) {
        v |= BM_SOC_SWM_TIMEOUT_WDT_EN;
    } else {
        v &= ~BM_SOC_SWM_TIMEOUT_WDT_EN;
    }

    v &= ~FM_SOC_SWM_TIMEOUT_PMU_HK;
    v |= FV_SOC_SWM_TIMEOUT_PMU_HK(pmu_hk);

    sdrv_smc_lld_writel(base, SOC_SWM_TIMEOUT_OFF, v);
}

/**
 * @brief soc swm timeout wakeup configuration
 *
 * @param[in] base reg base
 * @param[in] wkup_en pmu handshake timeout wakeup enable
 *
 */
static void soc_swm_timeout_wkup_config(paddr_t base, bool wkup_en)
{
    uint32_t v = sdrv_smc_lld_readl(base, SOC_SWM_TIMEOUT_OFF);

    if (wkup_en) {
        v |= BM_SOC_SWM_TIMEOUT_WKUP_EN;
    } else {
        v &= ~BM_SOC_SWM_TIMEOUT_WKUP_EN;
    }

    sdrv_smc_lld_writel(base, SOC_SWM_TIMEOUT_OFF, v);
}

/**
 * @brief soc wakeup acknowledge tiomeout configuration
 *
 * @param[in] base reg base
 * @param[in] tout_val timeout value for wakeup event receive to all active cpu
 * ack done
 *
 */
static void soc_wkup_ack_timeout_config(paddr_t base, uint16_t tout_val)
{
    uint32_t v = sdrv_smc_lld_readl(base, SOC_WKUP_CTL_OFF);

    v |= BM_SOC_WKUP_CTL_WKUP_DONE_SRC_SEL;
    v |= BM_SOC_WKUP_CTL_WKUP_WDT_EN;

    sdrv_smc_lld_writel(base, SOC_WKUP_CTL_OFF, v);

    v = sdrv_smc_lld_readl(base, SOC_WKUP_TIMEOUT_OFF);

    v &= ~FM_SOC_WKUP_TIMEOUT_WKUP_ACK;
    v |= FV_SOC_WKUP_TIMEOUT_WKUP_ACK(tout_val);

    sdrv_smc_lld_writel(base, SOC_WKUP_TIMEOUT_OFF, v);
}

/**
 * @brief soc wakeup to run state tiomeout configuration
 *
 * @param[in] base reg base
 * @param[in] tout_val timeout value for wakeup event receive to soc goes into
 * RUN state
 *
 */
static void soc_to_run_timeout_config(paddr_t base, uint16_t tout_val)
{
    uint32_t v = sdrv_smc_lld_readl(base, SOC_WKUP_CTL_OFF);

    v &= ~BM_SOC_WKUP_CTL_WKUP_DONE_SRC_SEL;
    v |= BM_SOC_WKUP_CTL_WKUP_WDT_EN;

    sdrv_smc_lld_writel(base, SOC_WKUP_CTL_OFF, v);

    v = sdrv_smc_lld_readl(base, SOC_WKUP_TIMEOUT_OFF);

    v &= ~FM_SOC_WKUP_TIMEOUT_SOC_RUN;
    v |= FV_SOC_WKUP_TIMEOUT_SOC_RUN(tout_val);

    sdrv_smc_lld_writel(base, SOC_WKUP_TIMEOUT_OFF, v);
}

/**
 * @brief soc low power to wakeup timeout configuration
 *
 * @param[in] base reg base
 * @param[in] tout_val timeout value between low power to wakeup event receive
 * @param[in] tout_wkup_en soc low power to wakeup timeout wakeup enable
 *
 */
static void soc_lp2wkup_timeout_config(paddr_t base, uint16_t tout_val,
                                       bool tout_wkup_en)
{
    uint32_t v = sdrv_smc_lld_readl(base, SOC_WKUP_CTL_OFF);

    v |= BM_SOC_WKUP_CTL_LP2WKUP_WDT_EN;

    if (tout_wkup_en) {
        v |= BM_SOC_WKUP_CTL_LP2WKUP_TOUT_WKUP_EN;
    } else {
        v &= ~BM_SOC_WKUP_CTL_LP2WKUP_TOUT_WKUP_EN;
    }

    v &= ~FM_SOC_WKUP_CTL_LP2WKUP_VAL;
    v |= FV_SOC_WKUP_CTL_LP2WKUP_VAL(tout_val);

    sdrv_smc_lld_writel(base, SOC_WKUP_CTL_OFF, v);
}

/**
 * @brief soc wakeup irq configuration
 *
 * @param[in] base reg base
 * @param[in] err_wkup_en smc error wakeup core irq enable
 * @param[in] bk_wkup_en soc wakeup core irq enable
 *
 */
static void soc_wkup_irq_wkup_config(paddr_t base, uint8_t err_wkup_en,
                                     uint8_t bk_wkup_en)
{
    uint32_t v = sdrv_smc_lld_readl(base, SMC_WKUP_IRQ_OFF);

    if (err_wkup_en) {
        v &= ~FM_SMC_WKUP_IRQ_ERR_WKUP_EN;
        v |= FV_SMC_WKUP_IRQ_ERR_WKUP_EN(err_wkup_en);
    }

    if (bk_wkup_en) {
        v &= ~FM_SMC_WKUP_IRQ_BK_WKUP_EN;
        v |= FV_SMC_WKUP_IRQ_BK_WKUP_EN(bk_wkup_en);
    }

    sdrv_smc_lld_writel(base, SMC_WKUP_IRQ_OFF, v);
}

/**
 * @brief soc wakeup irq clear configuration
 *
 * @param[in] base reg base
 * @param[in] wkup_irq_state soc wakeup irq state before clear
 *
 */
static void soc_wkup_irq_clr(paddr_t base, uint8_t *wkup_irq_state)
{
    uint32_t v = sdrv_smc_lld_readl(base, SMC_WKUP_IRQ_OFF);

    uint8_t irq_state = GFV_SMC_WKUP_IRQ_WKUP_IRQ_STA(v);

    *(uint8_t *)wkup_irq_state = irq_state;

    if (irq_state) {
        v |= FV_SMC_WKUP_IRQ_WKUP_CLR(irq_state);
        sdrv_smc_lld_writel(base, SMC_WKUP_IRQ_OFF, v);
    }
}

/******************************
 *  soc register configration  *
 ******************************/

/*********************************
 *  sw smc register configration  *
 *********************************/
/**
 * @brief safe domain low power software configuration
 *
 * @param[in] base reg base
 * @param[in] sub_dom_idx saf power domain index,range 0~3
 * @param[in] stage which stage to configured, power gate, isolation enable or
 * power down enable
 * @param[in] pd_en power down or power on
 *
 * @return -1 if sub_dom_idx out of range, otherwise 0
 */
static int sw_saf_lp_config(paddr_t base, uint8_t sub_dom_idx,
                            sw_pd_stage stage, bool pd_en)
{
    int ret = -1;

    if (sub_dom_idx < 4) {

        uint32_t v = sdrv_smc_lld_readl(base, SAF_LP_CTL_OFF(sub_dom_idx));

        if (pd_en) {
            if (SW_PWR_GATE == stage) {
                v |= BM_SAF_LP_CTL_SW_PWR_GATE;
            } else if (SW_ISO_EN == stage) {
                v |= BM_SAF_LP_CTL_SW_ISO_EN;
            } else if (SW_PD_EN == stage) {
                v |= BM_SAF_LP_CTL_SW_PD_EN;
            }
        } else {
            if (SW_PWR_GATE == stage) {
                v &= ~BM_SAF_LP_CTL_SW_PWR_GATE;
            } else if (SW_ISO_EN == stage) {
                v &= ~BM_SAF_LP_CTL_SW_ISO_EN;
            } else if (SW_PD_EN == stage) {
                v &= ~BM_SAF_LP_CTL_SW_PD_EN;
            }
        }

        sdrv_smc_lld_writel(base, SAF_LP_CTL_OFF(sub_dom_idx), v);

        ret = 0;
    }

    return ret;
}

/**
 * @brief ap domain low power software configuration
 *
 * @param[in] base reg base
 * @param[in] stage which stage to configured, power gate, isolation enable or
 * power down enable
 * @param[in] pd_en power down or power on
 *
 */
static void sw_ap_lp_config(paddr_t base, sw_pd_stage stage, bool pd_en)
{
    uint32_t v = sdrv_smc_lld_readl(base, AP_LP_CTL_OFF(0));

    if (pd_en) {
        if (SW_PWR_GATE == stage) {
            v |= BM_AP_LP_CTL_SW_PWR_GATE;
        } else if (SW_ISO_EN == stage) {
            v |= BM_AP_LP_CTL_SW_ISO_EN;
        } else if (SW_PD_EN == stage) {
            v |= BM_AP_LP_CTL_SW_PD_EN;
        }
    } else {
        if (SW_PWR_GATE == stage) {
            v &= ~BM_AP_LP_CTL_SW_PWR_GATE;
        } else if (SW_ISO_EN == stage) {
            v &= ~BM_AP_LP_CTL_SW_ISO_EN;
        } else if (SW_PD_EN == stage) {
            v &= ~BM_AP_LP_CTL_SW_PD_EN;
        }
    }

    sdrv_smc_lld_writel(base, AP_LP_CTL_OFF(0), v);
}

/**
 * @brief saf domain ram power down software configuration
 *
 * @param[in] base reg base
 * @param[in] mem_dom_idx memory region index
 * @param[in] ret1n,ret2n for ram setting
 * @param[in] pd_en power down or power on
 *
 * @return -1 if mem_dom_idx out of range, otherwise 0
 */
static int sw_saf_ram_lp_config(paddr_t base, uint8_t mem_dom_idx, bool pd_en,
                                uint8_t ret1n, uint8_t ret2n)
{
    int ret = -1;

    if (mem_dom_idx < 5) {
        uint32_t v = sdrv_smc_lld_readl(base, SAF_RAM_LP_CTL_OFF(mem_dom_idx));

        if (pd_en) {
            v |= BM_SAF_RAM_LP_CTL_SW_RAM_PD_EN;

            v &= ~FM_SAF_RAM_LP_CTL_SW_RAM_LP_SETTING;
            v |= FV_SAF_RAM_LP_CTL_SW_RAM_LP_SETTING((0x1 << 2) | (ret1n << 1) |
                                                     (ret2n << 0));
        } else {
            v &= ~BM_SAF_RAM_LP_CTL_SW_RAM_PD_EN;
            v &= ~(0x1 << 11);
        }

        sdrv_smc_lld_writel(base, SAF_RAM_LP_CTL_OFF(mem_dom_idx), v);

        ret = 0;
    }

    return ret;
}

/**
 * @brief ap domain ram power down software configuration
 *
 * @param[in] base reg base
 * @param[in] mem_dom_idx memory region index
 * @param[in] ret1n,ret2n for ram setting
 * @param[in] pd_en power down or power on
 *
 * @return -1 if mem_dom_idx out of range, otherwise 0
 */
static int sw_ap_ram_lp_config(paddr_t base, uint8_t mem_dom_idx, bool pd_en,
                               uint8_t ret1n, uint8_t ret2n)
{
    int ret = -1;

    if (mem_dom_idx < 2) {
        uint32_t v = sdrv_smc_lld_readl(base, AP_RAM_LP_CTL_OFF(mem_dom_idx));

        if (pd_en) {
            v |= BM_AP_RAM_LP_CTL_SW_RAM_PD_EN;

            v &= ~FM_AP_RAM_LP_CTL_SW_RAM_LP_SETTING;
            v |= FV_AP_RAM_LP_CTL_SW_RAM_LP_SETTING((0x1 << 2) | (ret1n << 1) |
                                                    (ret2n << 0));
        } else {
            v &= ~BM_AP_RAM_LP_CTL_SW_RAM_PD_EN;
            v &= ~(0x1 << 11);
        }

        sdrv_smc_lld_writel(base, AP_RAM_LP_CTL_OFF(mem_dom_idx), v);

        ret = 0;
    }

    return ret;
}

/**
 * @brief saf domain ckgen handshake software configuration
 *
 * @param[in] base reg base
 * @param[in] sw_mode mode info handshake with ckgen, run/slp/hib/rtc
 *
 */
static void sw_saf_ckgen_swm_config(paddr_t base, sw_swm_mode sw_mode)
{
    uint32_t v = 0;

    sdrv_smc_lld_writel(base, SMC_SW_SWM_OFF, v);

    v &= ~FM_SMC_SW_SWM_SAF_CKGEN_SWM_O;
    v |= FV_SMC_SW_SWM_SAF_CKGEN_SWM_O(sw_mode);

    v |= BM_SMC_SW_SWM_SAF_CKGEN_SWM_EN;

    v |= BM_SMC_SW_SWM_SAF_CKGEN_SWM_TRIGGER;

    sdrv_smc_lld_writel(base, SMC_SW_SWM_OFF, v);

    v = sdrv_smc_lld_readl(base, SMC_SW_SWM_OFF);
    while (0 == (v & BM_SMC_SW_SWM_SAF_CKGEN_SWM_OK)) {
        v = sdrv_smc_lld_readl(base, SMC_SW_SWM_OFF);
    }

    v = 0;

    sdrv_smc_lld_writel(base, SMC_SW_SWM_OFF, v);
}

/**
 * @brief saf domain rstgen handshake software configuration
 *
 * @param[in] base reg base
 * @param[in] sw_mode mode info handshake with rstgen, run/slp/hib/rtc
 *
 */
static void sw_saf_rstgen_swm_config(paddr_t base, sw_swm_mode sw_mode)
{
    uint32_t v = 0;

    sdrv_smc_lld_writel(base, SMC_SW_SWM_OFF, v);

    v &= ~FM_SMC_SW_SWM_SAF_RSTGEN_SWM_O;
    v |= FV_SMC_SW_SWM_SAF_RSTGEN_SWM_O(sw_mode);

    v |= BM_SMC_SW_SWM_SAF_RSTGEN_SWM_EN;

    v |= BM_SMC_SW_SWM_SAF_RSTGEN_SWM_TRIGGER;

    sdrv_smc_lld_writel(base, SMC_SW_SWM_OFF, v);

    v = sdrv_smc_lld_readl(base, SMC_SW_SWM_OFF);
    while (0 == (v & BM_SMC_SW_SWM_SAF_RSTGEN_SWM_OK)) {
        v = sdrv_smc_lld_readl(base, SMC_SW_SWM_OFF);
    }

    v = 0;

    sdrv_smc_lld_writel(base, SMC_SW_SWM_OFF, v);
}

/**
 * @brief ap domain ckgen handshake software configuration
 *
 * @param[in] base reg base
 * @param[in] sw_mode mode info handshake with ckgen, run/slp/hib/rtc
 *
 */
static void sw_ap_ckgen_swm_config(paddr_t base, sw_swm_mode sw_mode)
{
    uint32_t v = 0;

    sdrv_smc_lld_writel(base, SMC_SW_SWM_OFF, v);

    v &= ~FM_SMC_SW_SWM_AP_CKGEN_SWM_O;
    v |= FV_SMC_SW_SWM_AP_CKGEN_SWM_O(sw_mode);

    v |= BM_SMC_SW_SWM_AP_CKGEN_SWM_EN;

    v |= BM_SMC_SW_SWM_AP_CKGEN_SWM_TRIGGER;

    sdrv_smc_lld_writel(base, SMC_SW_SWM_OFF, v);

    v = sdrv_smc_lld_readl(base, SMC_SW_SWM_OFF);
    while (0 == (v & BM_SMC_SW_SWM_AP_CKGEN_SWM_OK)) {
        v = sdrv_smc_lld_readl(base, SMC_SW_SWM_OFF);
    }

    v = 0;

    sdrv_smc_lld_writel(base, SMC_SW_SWM_OFF, v);
}

/**
 * @brief ap domain rstgen handshake software configuration
 *
 * @param[in] base reg base
 * @param[in] sw_mode mode info handshake with rstgen, run/slp/hib/rtc
 *
 */
static void sw_ap_rstgen_swm_config(paddr_t base, sw_swm_mode sw_mode)
{
    uint32_t v = 0;

    sdrv_smc_lld_writel(base, SMC_SW_SWM_OFF, v);

    v &= ~FM_SMC_SW_SWM_AP_RSTGEN_SWM_O;
    v |= FV_SMC_SW_SWM_AP_RSTGEN_SWM_O(sw_mode);

    v |= BM_SMC_SW_SWM_AP_RSTGEN_SWM_EN;

    v |= BM_SMC_SW_SWM_AP_RSTGEN_SWM_TRIGGER;

    sdrv_smc_lld_writel(base, SMC_SW_SWM_OFF, v);

    v = sdrv_smc_lld_readl(base, SMC_SW_SWM_OFF);
    while (0 == (v & BM_SMC_SW_SWM_AP_RSTGEN_SWM_OK)) {
        v = sdrv_smc_lld_readl(base, SMC_SW_SWM_OFF);
    }

    v = 0;

    sdrv_smc_lld_writel(base, SMC_SW_SWM_OFF, v);
}

/**
 * @brief soc pmu handshake software configuration
 *
 * @param[in] base reg base
 * @param[in] sw_mode mode info handshake with pmu, run/slp/hib/rtc
 *
 */
static void sw_soc_pmu_swm_config(paddr_t base, sw_swm_mode sw_mode)
{
    uint32_t v = 0;

    sdrv_smc_lld_writel(base, SMC_SW_SWM_OFF, v);

    v &= ~FM_SMC_SW_SWM_PMU_SWM_O;
    v |= FV_SMC_SW_SWM_PMU_SWM_O(sw_mode);

    v |= BM_SMC_SW_SWM_PMU_SWM_EN;

    v |= BM_SMC_SW_SWM_PMU_SWM_TRIGGER;

    sdrv_smc_lld_writel(base, SMC_SW_SWM_OFF, v);

    v = sdrv_smc_lld_readl(base, SMC_SW_SWM_OFF);
    while (0 == (v & BM_SMC_SW_SWM_PMU_SWM_OK)) {
        v = sdrv_smc_lld_readl(base, SMC_SW_SWM_OFF);
    }

    v = 0;

    sdrv_smc_lld_writel(base, SMC_SW_SWM_OFF, v);
}

/*********************************
 *  sw smc register configration  *
 *********************************/

/*******************************************
 *   status monitor register configuration  *
 *******************************************/

/**
 * @brief swm status monitor
 *
 * @param[in] base reg base
 * @param[in] type type of status to monitor
 * @param[in] status pointer to the status of "type"
 *
 * @return -1 if type out of range or status is NULL, otherwise the status of
 * the specified type
 */
static int swm_status_monitor(paddr_t base, swm_st_type type)
{
    int ret = -1;

    if (type < 5) {
        uint32_t v = sdrv_smc_lld_readl(base, SWM_MON_OFF);

        switch (type) {
        case AP_INTER_SWM:
            ret = (uint8_t)GFV_SWM_MON_AP_INTER_SWM(v);
            break;
        case SAF_INTER_SWM:
            ret = (uint8_t)GFV_SWM_MON_SAF_INTER_SWM(v);
            break;
        case SOC_SWM:
            ret = (uint8_t)GFV_SWM_MON_SOC_SWM(v);
            break;
        case AP_SWM:
            ret = (uint8_t)GFV_SWM_MON_AP_SWM(v);
            break;
        case SAF_SWM:
            ret = (uint8_t)GFV_SWM_MON_SAF_SWM(v);
            break;
        default:
            break;
        }
    }

    return ret;
}

/**
 * @brief apb error and int enable configuration
 *
 * @param[in] base reg base
 * @param[in] int_en apb error interrupt enable
 *     int_en bitmap:
 *         bit0: pwdata fatal error
 *         bit1: pwdata uncorrectable error
 *         bit2: pwdata correctable error
 *         bit3: pctrl0 uncorrectable error
 *         bit4: pctrl1 uncorrectable error
 *         bit5: puser uncorrectable error
 *         bit6: paddr uncorrectable error
 *
 */
static void apb_err_int_en_config(paddr_t base, bool int_en)
{
    uint32_t v = sdrv_smc_lld_readl(base, SMC_APB_ERR_INT_OFF);

    if (int_en) {
        v |= BM_APB_ERR_INT_PWDAT_F_INT_EN;
        v |= BM_APB_ERR_INT_PWDAT_U_INT_EN;
        v |= BM_APB_ERR_INT_PWDAT_C_INT_EN;
        v |= BM_APB_ERR_INT_PCTRL0_INT_EN;
        v |= BM_APB_ERR_INT_PCTRL1_INT_EN;
        v |= BM_APB_ERR_INT_PUSER_INT_EN;
        v |= BM_APB_ERR_INT_PADDR_INT_EN;
    } else {
        v &= ~BM_APB_ERR_INT_PWDAT_F_INT_EN;
        v &= ~BM_APB_ERR_INT_PWDAT_U_INT_EN;
        v &= ~BM_APB_ERR_INT_PWDAT_C_INT_EN;
        v &= ~BM_APB_ERR_INT_PCTRL0_INT_EN;
        v &= ~BM_APB_ERR_INT_PCTRL1_INT_EN;
        v &= ~BM_APB_ERR_INT_PUSER_INT_EN;
        v &= ~BM_APB_ERR_INT_PADDR_INT_EN;
    }

    sdrv_smc_lld_writel(base, SMC_APB_ERR_INT_OFF, v);
}

/**
 * @brief apb error and int monitor configuration
 *
 * @param[in] base reg base
 *
 * @return apb error and int status
 */
static uint8_t apb_err_int_mon_config(paddr_t base)
{
    uint32_t v = sdrv_smc_lld_readl(base, SMC_APB_ERR_INT_OFF);

    return (uint8_t)((v >> 9) & 0x7Fu);
}

/**
 * @brief apb error and int clear configuration
 *
 * @param[in] base reg base
 */
static void apb_err_int_clr_config(paddr_t base)
{
    uint32_t v = sdrv_smc_lld_readl(base, SMC_APB_ERR_INT_OFF);
    uint8_t int_sta = apb_err_int_mon_config(base);

    if (int_sta) {
        v |= (int_sta << 17);
        sdrv_smc_lld_writel(base, SMC_APB_ERR_INT_OFF, v);
    }
}

/**
 * @brief apb lockstep int configuration
 *
 * @param[in] base reg base
 * @param[in] int_en apb lockstep interrupt enable
 *     int_en bitmap:
 *         bit0: lockstep compare error
 *         bit1: request parity error
 *         bit2: response parity error
 *         bit3: sync error
 */
static void apb_lkstep_int_en_config(paddr_t base, bool int_en)
{
    uint32_t v = sdrv_smc_lld_readl(base, SMC_APB_LKSTEP_INT_OFF);

    if (int_en) {
        v |= BM_APB_LKSTEP_INT_CMP_INT_EN;
        v |= BM_APB_LKSTEP_INT_REQ_ERR_INT_EN;
        v |= BM_APB_LKSTEP_INT_RESP_ERR_INT_EN;
        v |= BM_APB_LKSTEP_INT_SYNC_ERR_INT_EN;
    } else {
        v &= ~BM_APB_LKSTEP_INT_CMP_INT_EN;
        v &= ~BM_APB_LKSTEP_INT_REQ_ERR_INT_EN;
        v &= ~BM_APB_LKSTEP_INT_RESP_ERR_INT_EN;
        v &= ~BM_APB_LKSTEP_INT_SYNC_ERR_INT_EN;
    }

    sdrv_smc_lld_writel(base, SMC_APB_LKSTEP_INT_OFF, v);
}

/**
 * @brief apb lockstep int configuration
 *
 * @param[in] base reg base
 *
 * @return  apb lockstep int status
 */
static uint8_t apb_lkstep_int_mon_config(paddr_t base)
{
    uint32_t v = sdrv_smc_lld_readl(base, SMC_APB_LKSTEP_INT_OFF);

    return (uint8_t)((v >> 8U) & 0xFU);
}

/**
 * @brief apb lockstep int configuration
 *
 * @param[in] base reg base
 */
static void apb_lkstep_int_clr_config(paddr_t base)
{
    uint32_t v = sdrv_smc_lld_readl(base, SMC_APB_LKSTEP_INT_OFF);
    uint8_t int_sta = apb_lkstep_int_mon_config(base);

    if (int_sta) {
        v |= (int_sta << 16);
        sdrv_smc_lld_writel(base, SMC_APB_LKSTEP_INT_OFF, v);
    }
}

/**
 * @brief smc internal fusa int enable configuration
 *
 * @param[in] base reg base
 * @param[in] int_en fusa interrupt enable
 *     int_en bitmap:
 *         bit0: swm check error
 *         bit1: timeout error
 *         bit2: swm global config error
 *         bit3: soc 24m error
 *         bit4: internal sync check error
 *         bit5: rstgen core lockstep compare error
 *
 */
static void smc_fusa_int_en_config(paddr_t base, bool int_en)
{
    uint32_t v = sdrv_smc_lld_readl(base, SMC_FUSA_INT_OFF);

    if (int_en) {
        v |= BM_SMC_FUSA_INT_SWM_CHK_ERR_EN;
        v |= BM_SMC_FUSA_INT_TOUT_ERR_EN;
        v |= BM_SMC_FUSA_INT_SMC_CFG_ERR_EN;
        v |= BM_SMC_FUSA_INT_SOC_24M_ERR_EN;
        v |= BM_SMC_FUSA_INT_SYNC_ERR_EN;
        v |= BM_SMC_FUSA_INT_LKSTEP_CMP_ERR_EN;
    } else {
        v &= ~BM_SMC_FUSA_INT_SWM_CHK_ERR_EN;
        v &= ~BM_SMC_FUSA_INT_TOUT_ERR_EN;
        v &= ~BM_SMC_FUSA_INT_SMC_CFG_ERR_EN;
        v &= ~BM_SMC_FUSA_INT_SOC_24M_ERR_EN;
        v &= ~BM_SMC_FUSA_INT_SYNC_ERR_EN;
        v &= ~BM_SMC_FUSA_INT_LKSTEP_CMP_ERR_EN;
    }

    sdrv_smc_lld_writel(base, SMC_FUSA_INT_OFF, v);
}

/**
 * @brief smc internal fusa int configuration
 *
 * @param[in] base reg base
 *
 * @return fusa interrupt status
 */
static uint8_t smc_fusa_int_mon_config(paddr_t base)
{
    uint32_t v = sdrv_smc_lld_readl(base, SMC_FUSA_INT_OFF);

    return (uint8_t)((v >> 8U) & 0x3FU);
}

/**
 * @brief smc internal fusa int configuration
 *
 * @param[in] base reg base
 *
 */
static void smc_fusa_int_clr_config(paddr_t base)
{
    uint32_t v = sdrv_smc_lld_readl(base, SMC_FUSA_INT_OFF);
    uint8_t int_sta = smc_fusa_int_mon_config(base);

    if (int_sta) {
        v |= (int_sta << 16);
        sdrv_smc_lld_writel(base, SMC_FUSA_INT_OFF, v);
    }
}

/**
 * @brief smc timeout status configuration
 *
 * @param[in] base reg base
 *
 * @return  timeout status
 */
static uint16_t smc_tout_sta_config(paddr_t base)
{
    uint32_t v = sdrv_smc_lld_readl(base, SMC_TOUT_STA_OFF);

    return (uint16_t)(v & 0xFFFFU);
}

/**
 * @brief smc illegal transfer event monitor status configuration
 *
 * @param[in] base reg base
 * @param[in] ill_trans_sta pointer to ill transfer status
 *
 * @return -1 if ill_trans_sta is NULL, otherwise 0
 */
static uint16_t smc_ill_trans_sta_config(paddr_t base)
{

    uint32_t v = sdrv_smc_lld_readl(base, SMC_ILL_TRANS_STA_OFF);

    return (uint16_t)(v & 0x3FFU);
}

/**
 * @brief watchdog lockstep int enable configuration
 *
 * @param[in] base reg base
 * @param[in] int_en wdt lockstep interrupt enable
 *     int_en bitmap:
 *         bit0: timeout value check error
 *         bit1: sync check error
 *         bit2: cmp error
 *
 */
static void wdt_lkstep_int_en_config(paddr_t base, bool int_en)
{
    uint32_t v = sdrv_smc_lld_readl(base, SMC_WDT_LKSTEP_INT_OFF);

    if (int_en) {
        v |= BM_WDT_LKSTEP_INT_TOUT_CHK_ERR_EN;
        v |= BM_WDT_LKSTEP_INT_SYNC_ERR_EN;
        v |= BM_WDT_LKSTEP_INT_CMP_ERR_EN;
    } else {
        v &= ~BM_WDT_LKSTEP_INT_TOUT_CHK_ERR_EN;
        v &= ~BM_WDT_LKSTEP_INT_SYNC_ERR_EN;
        v &= ~BM_WDT_LKSTEP_INT_CMP_ERR_EN;
    }

    sdrv_smc_lld_writel(base, SMC_WDT_LKSTEP_INT_OFF, v);
}

/**
 * @brief watchdog lockstep int monitor configuration
 *
 * @param[in] base reg base
 *
 * return wdt lockstep interrupt status
 *     return value bitmap:
 *         bit0: timeout value check error
 *         bit1: sync check error
 *         bit2: cmp error
 */
static uint8_t wdt_lkstep_int_mon_config(paddr_t base)
{
    uint32_t v = sdrv_smc_lld_readl(base, SMC_WDT_LKSTEP_INT_OFF);

    return (uint8_t)((v >> 8U) & 0x7U);
}

/**
 * @brief watchdog lockstep int clear configuration
 *
 * @param[in] base reg base
 */
static void wdt_lkstep_int_clr_config(paddr_t base)
{
    uint32_t v = sdrv_smc_lld_readl(base, SMC_WDT_LKSTEP_INT_OFF);
    uint8_t int_sta = wdt_lkstep_int_mon_config(base);

    if (int_sta) {
        v |= (int_sta << 16);
        sdrv_smc_lld_writel(base, SMC_WDT_LKSTEP_INT_OFF, v);
    }
}

/**
 * @brief smc func int enable configuration
 *
 * @param[in] base reg base
 * @param[in] int_en smc func interrupt enable
 *     int_en bitmap:
 *         bit0: access permission check error
 */
static void smc_func_int_en_config(paddr_t base, bool int_en)
{
    uint32_t v = sdrv_smc_lld_readl(base, SMC_FUNC_INT_OFF);

    if (int_en) {
        v |= BM_SMC_FUNC_INT_ACCESS_PER_ERR_EN;
    } else {
        v &= ~BM_SMC_FUNC_INT_ACCESS_PER_ERR_EN;
    }

    sdrv_smc_lld_writel(base, SMC_FUNC_INT_OFF, v);
}

/**
 * @brief smc func int monitor configuration
 *
 * @param[in] base reg base
 *
 * @return func int status
 */
static uint8_t smc_func_int_mon_config(paddr_t base)
{
    uint32_t v = sdrv_smc_lld_readl(base, SMC_FUNC_INT_OFF);

    return (uint8_t)((v >> 8U) & 0x1U);
}

/**
 * @brief smc func int clear configuration
 *
 * @param[in] base reg base
 */
static void smc_func_int_clr_config(paddr_t base)
{
    uint32_t v = sdrv_smc_lld_readl(base, SMC_FUNC_INT_OFF);
    uint8_t int_st = smc_func_int_mon_config(base);

    if (int_st) {
        v |= BM_SMC_FUNC_INT_ACCESS_PER_ERR_CLR;
        sdrv_smc_lld_writel(base, SMC_FUNC_INT_OFF, v);
    }
}

/*******************************************
 *   status monitor register configuration  *
 *******************************************/

/*******************************************
 *   clock  monitor register configuration  *
 *******************************************/
/**
 * @brief soc clock monitor configuration
 *
 * @param[in] base reg base
 * @param[in] hw_mon clk32k monitor clk24m hw enable
 * @param[in] force_mon clk32k monitor clk24m force enable
 *
 */
static void soc_clk_mon_config(paddr_t base, bool hw_mon, bool force_mon)
{
    uint32_t v = sdrv_smc_lld_readl(base, SOC_CMON_OFF);

    if (force_mon) {
        v |= BM_SOC_CMON_FORCE_MON_EN;
    } else {
        v &= ~BM_SOC_CMON_FORCE_MON_EN;
    }

    if (hw_mon) {
        v |= BM_SOC_CMON_HW_MON_EN;
    } else {
        v &= ~BM_SOC_CMON_HW_MON_EN;
    }

    sdrv_smc_lld_writel(base, SOC_CMON_OFF, v);
}

/**
 * @brief soc clock monitor configuration
 *
 * @param[in] base reg base
 * @param[in] loss_dis active loss event disable
 *
 */
static void soc_clk24m_active_loss_config(paddr_t base, bool loss_dis)
{
    uint32_t v = sdrv_smc_lld_readl(base, SOC_CMON_OFF);

    if (loss_dis) {
        v |= BM_SOC_CMON_ACTIVE_LOSS_DIS;
    } else {
        v &= ~BM_SOC_CMON_ACTIVE_LOSS_DIS;
    }

    sdrv_smc_lld_writel(base, SOC_CMON_OFF, v);
}

/**
 * @brief soc clock frequency monitor configuration
 *
 * @param[in] base reg base
 * @param[in] freq pointer to the updated frequency
 *
 * return whether the frequency update or not
 */
static bool soc_cmon_freq_update_config(paddr_t base, uint16_t *freq)
{
    bool ret = false;
    if (freq != NULL) {
        uint32_t v = sdrv_smc_lld_readl(base, SOC_CMON_OFF);

        if (v & BM_SOC_CMON_MON_FREQ_UPD) {
            *(uint16_t *)freq = GFV_SOC_CMON_MON_FREQ(v);
            ret = true;
        }
    }

    return ret;
}

/**
 * @brief clear the soc clk mon update frequency
 *
 * @param[in] base reg base
 */
static void soc_cmon_freq_clear_config(paddr_t base)
{
    uint32_t v = sdrv_smc_lld_readl(base, SOC_CMON_OFF);

    v &= ~BM_SOC_CMON_MON_FREQ_UPD;
    v &= ~FM_SOC_CMON_MON_FREQ;

    sdrv_smc_lld_writel(base, SOC_CMON_OFF, v);
}

/**
 * @brief soc clock monitor rc check setting
 *
 * @param[in] base reg base
 * @param[in] high_thrd high threshold
 * @param[in] low_thrd low threshold
 *
 */
static void soc_cmon_rc_config(paddr_t base, uint16_t high_thrd,
                               uint16_t low_thrd)
{
    uint32_t v = sdrv_smc_lld_readl(base, SOC_CMON_RC_OFF);

    v &= ~FM_SOC_CMON_RC_HIGH_THRD;
    v |= FV_SOC_CMON_RC_HIGH_THRD(high_thrd);

    v &= ~FM_SOC_CMON_RC_LOW_THRD;
    v |= FV_SOC_CMON_RC_LOW_THRD(low_thrd);

    sdrv_smc_lld_writel(base, SOC_CMON_RC_OFF, v);
}

/**
 * @brief soc clock monitor xtal check setting
 *
 * @param[in] base reg base
 * @param[in] high_thrd high threshold
 * @param[in] low_thrd low threshold
 *
 */
static void soc_cmon_xtal_config(paddr_t base, uint16_t high_thrd,
                                 uint16_t low_thrd)
{
    uint32_t v = sdrv_smc_lld_readl(base, SOC_CMON_XTAL_OFF);

    v &= ~FM_SOC_CMON_XTAL_HIGH_THRD;
    v |= FV_SOC_CMON_XTAL_HIGH_THRD(high_thrd);

    v &= ~FM_SOC_CMON_XTAL_LOW_THRD;
    v |= FV_SOC_CMON_XTAL_LOW_THRD(low_thrd);

    sdrv_smc_lld_writel(base, SOC_CMON_XTAL_OFF, v);
}

/*******************************************
 *   clock  monitor register configuration  *
 *******************************************/

/******************************
 * rs and domain configuration *
 ******************************/
int sdrv_smc_lld_rs_did_perm_config(paddr_t base, uint32_t rs_idx,
                                    uint32_t dom_idx, uint8_t usr_per,
                                    uint8_t pri_per, uint8_t non_sec_per,
                                    uint8_t sec_per)
{
    int ret = -1;

    ret = rs_dom_permission_config(base, rs_idx, dom_idx, usr_per, pri_per,
                                   non_sec_per, sec_per);

    return ret;
}

int sdrv_smc_lld_rs_config(paddr_t base, smc_rs_type type, uint8_t rs_idx,
                           bool did_en, bool lock, uint8_t core_idx,
                           uint8_t irq_grp_idx)
{
    int ret = -1;
    switch (type) {
    case SAF_DOMAIN:
        ret = saf_dom_rs_config(base, rs_idx, did_en, lock);
        break;
    case AP_DOMAIN:
        ret = ap_dom_rs_config(base, rs_idx, did_en, lock);
        break;
    case SOC_DOMAIN:
        ret = soc_dom_rs_config(base, rs_idx, did_en, lock);
        break;
    case SOC_CLK_MON:
        ret = soc_cmom_rs_config(base, rs_idx, did_en, lock);
        break;
    case CORE_IRQ_MSK:
        ret = core_irq_mask_rs_config(base, core_idx, rs_idx, did_en, lock);
        break;
    case COM_IRQ_MSK:
        ret = com_irq_mask_rs_config(base, irq_grp_idx, rs_idx, did_en, lock);
        break;
    case IRQ_MONITOR:
        ret = irq_mon_rs_config(base, rs_idx, did_en, lock);
        break;
    case SMC_RES:
        ret = smc_res_rs_config(base, rs_idx, did_en, lock);
        break;
    case SMC_MISC:
        ret = smc_misc_rs_config(base, rs_idx, did_en, lock);
        break;
    case SMC_FUSA:
        ret = smc_fusa_rs_config(base, rs_idx, did_en, lock);
        break;
    case SMC_FUNC_INT:
        ret = smc_func_int_rs_config(base, rs_idx, did_en, lock);
        break;
    default:
        break;
    }

    return ret;
}
/******************************
 * rs and domain configuration *
 ******************************/

/****************************
 * soc lp mode configuration *
 ****************************/
/**
 * @brief config the saf or ap domain pri core
 *
 * @param[in] base reg base
 * @param[in] dom domain type, ap or saf
 * @param[in] pri_core pri_core configured for this domain
 *
 */
void sdrv_smc_lld_pri_core_config(paddr_t base, smc_rs_type dom,
                                  uint8_t pri_core)
{
    if (AP_DOMAIN == dom) {
        ap_pri_core_config(base, pri_core);
    } else if (SAF_DOMAIN == dom) {
        saf_pri_core_config(base, pri_core);
    }
}

/**
 * @brief ap power off, saf wfi disable
 *
 * @param[in] base reg base
 *
 */
void sdrv_smc_lld_saf_lp_dis_ap_off(paddr_t base)
{
    soc_global_config(base, 1);

    saf_wfi_config(base, 0, 0);
}

/**
 * @brief ap power off, saf wfi enable and lp mode is slp
 *
 * @param[in] base reg base
 *
 */
void sdrv_smc_lld_saf_slp_ap_off(paddr_t base)
{
    soc_global_config(base, 1);

    saf_wfi_config(base, 1, SLP_MODE);
}

/**
 * @brief ap power off, saf wfi enable and lp mode is hib
 *
 * @param[in] base reg base
 *
 */
void sdrv_smc_lld_saf_hib_ap_off(paddr_t base)
{
    soc_global_config(base, 1);

    saf_wfi_config(base, 1, HIB_MODE);
}

/**
 * @brief ap power on, saf domain and ap domain wfi disable
 *
 * @param[in] base reg base
 *
 */
void sdrv_smc_lld_saf_ap_lp_dis(paddr_t base)
{
    saf_wfi_config(base, 0, 0);

    ap_wfi_config(base, 0, 0);
}

/**
 * @brief ap power on, saf domain wfi disable, ap domain wfi enable and lp mode
 * is slp
 *
 * @param[in] base reg base
 *
 */
void sdrv_smc_lld_saf_lp_dis_ap_slp(paddr_t base)
{
    saf_wfi_config(base, 0, 0);

    ap_wfi_config(base, 1, SLP_MODE);
}

/**
 * @brief ap power on, saf domain wfi disable, ap domain wfi enable and lp mode
 * is hib
 *
 * @param[in] base reg base
 *
 */
void sdrv_smc_lld_saf_lp_dis_ap_hib(paddr_t base)
{
    saf_wfi_config(base, 0, 0);

    ap_wfi_config(base, 1, HIB_MODE);
}

/**
 * @brief ap power on, saf domain wfi enable and lp mode is slp, ap domain wfi
 * enable and lp mode is slp note: ap domain not align with saf domain
 *
 * @param[in] base reg base
 *
 */
void sdrv_smc_lld_saf_slp_ap_slp(paddr_t base)
{
    saf_wfi_config(base, 1, SLP_MODE);

    ap_wfi_config(base, 1, SLP_MODE);
}

/**
 * @brief ap power on, saf domain wfi enable and lp mode is slp, ap domain wfi
 * enable and lp mode is hib note: ap domain not align with saf domain
 *
 * @param[in] base reg base
 *
 */
void sdrv_smc_lld_saf_slp_ap_hib(paddr_t base)
{
    saf_wfi_config(base, 1, SLP_MODE);

    ap_wfi_config(base, 1, HIB_MODE);
}

/**
 * @brief ap power on, saf domain wfi enable and lp mode is hib, ap domain wfi
 * enable and lp mode is hib note: ap domain not align with saf domain
 *
 * @param[in] base reg base
 *
 */
void sdrv_smc_lld_saf_hib_ap_hib(paddr_t base)
{
    saf_wfi_config(base, 1, HIB_MODE);

    ap_wfi_config(base, 1, HIB_MODE);
}

/**
 * @brief saf&ap domain power down
 * note: power down delay default
 *
 * @param[in] base reg base
 * @param[in] saf_pd_msk 4 bit valid,means bit 0-3. if 1, the corresponding core
 * pd enable, otherwise pd disable
 * @param[in] ap_pd_msk 1 bit valid,means bit 0. if 1, the corresponding core pd
 * enable, otherwise pd disable
 *
 */
void sdrv_smc_lld_saf_ap_pd(paddr_t base, uint8_t saf_pd_msk, uint8_t ap_pd_msk)
{
    for (uint8_t i = 0; i < 4u; i++) {
        if (saf_pd_msk & (0x01u << i)) {
            saf_lp_config(base, i, 1);
        } else {
            saf_lp_config(base, i, 0);
        }
    }

    if (ap_pd_msk & 0x01u) {
        ap_lp_config(base, 1);
    } else {
        ap_lp_config(base, 0);
    }
}

/**
 * @brief AP domain power off
 * note: SMC only handshake with saf domain
 *
 * @param[in] base reg base
 *
 */
void sdrv_smc_lld_ap_off(paddr_t base) { soc_global_config(base, 1); }

/**
 * @brief Safety domain LP power ctrl
 *
 * @param[in] base reg base
 * @param[in] index reg index
 * @param[in] lp LP mode
 * @param[in] pd_en power down enable
 *
 */
void sdrv_smc_lld_saf_lp_ctrl(paddr_t base, uint32_t index, domain_lp_mode lp,
                              bool pd_en)
{
    if (index < 4) {
        uint32_t v = sdrv_smc_lld_readl(base, SAF_LP_CTL_OFF(index));

        if (pd_en) {
            if (HIB_MODE == lp) {
                v |= BM_SAF_LP_CTL_HIB_PD_EN;
            } else {
                v |= BM_SAF_LP_CTL_SLP_PD_EN;
            }
        } else {
            if (HIB_MODE == lp) {
                v &= ~BM_SAF_LP_CTL_HIB_PD_EN;
            } else {
                v &= ~BM_SAF_LP_CTL_SLP_PD_EN;
            }
        }

        sdrv_smc_lld_writel(base, SAF_LP_CTL_OFF(index), v);

        // RMWREG32(base + SAF_LP_CTL_OFF(index) + 0x4u, 12, 8, 0x4);
    }
}

/**
 * @brief AP domain LP power ctrl
 *
 * @param[in] base reg base
 * @param[in] index reg index
 * @param[in] lp LP mode
 * @param[in] pd_en power down enable
 *
 */
void sdrv_smc_lld_ap_lp_ctrl(paddr_t base, uint32_t index, domain_lp_mode lp,
                             bool pd_en)
{
    if (index < 1) {
        uint32_t v = sdrv_smc_lld_readl(base, AP_LP_CTL_OFF(index));

        if (pd_en) {
            if (HIB_MODE == lp) {
                v |= BM_AP_LP_CTL_HIB_PD_EN;
            } else {
                v |= BM_AP_LP_CTL_SLP_PD_EN;
            }
        } else {
            if (HIB_MODE == lp) {
                v &= ~BM_AP_LP_CTL_HIB_PD_EN;
            } else {
                v &= ~BM_AP_LP_CTL_SLP_PD_EN;
            }
        }

        sdrv_smc_lld_writel(base, AP_LP_CTL_OFF(index), v);

        // RMWREG32(base + AP_LP_CTL_OFF(index) + 0x4u, 12, 8, 0x4);
    }
}

/**
 * @brief Safety domain LP ram ctrl
 *
 * @param[in] base reg base
 * @param[in] index reg index
 * @param[in] lp LP mode
 * @param[in] pd_en power down enable
 * @param[in] lp_setting ram lp setting
 *
 */
void sdrv_smc_lld_saf_ram_lp_ctrl(paddr_t base, uint32_t index,
                                  domain_lp_mode lp, bool pd_en,
                                  uint32_t lp_setting)
{
    if (index < 5) {
        uint32_t v = sdrv_smc_lld_readl(base, SAF_RAM_LP_CTL_OFF(index));

        if (pd_en) {
            if (HIB_MODE == lp) {
                v |= BM_SAF_RAM_LP_CTL_HIB_RAM_PD_EN;
            } else {
                v |= BM_SAF_RAM_LP_CTL_SLP_RAM_PD_EN;
            }
        } else {
            if (HIB_MODE == lp) {
                v &= ~BM_SAF_RAM_LP_CTL_HIB_RAM_PD_EN;
            } else {
                v &= ~BM_SAF_RAM_LP_CTL_SLP_RAM_PD_EN;
            }
        }

        if (HIB_MODE == lp) {
            v &= ~FM_SAF_RAM_LP_CTL_HIB_RAM_LP_SETTING;
            v |= FV_SAF_RAM_LP_CTL_HIB_RAM_LP_SETTING(lp_setting);
        } else {
            v &= ~FM_SAF_RAM_LP_CTL_SLP_RAM_LP_SETTING;
            v |= FV_SAF_RAM_LP_CTL_SLP_RAM_LP_SETTING(lp_setting);
        }

        sdrv_smc_lld_writel(base, SAF_RAM_LP_CTL_OFF(index), v);
    }
}

/**
 * @brief AP domain LP ram ctrl
 *
 * @param[in] base reg base
 * @param[in] index reg index
 * @param[in] lp LP mode
 * @param[in] pd_en power down enable
 * @param[in] lp_setting ram lp setting
 *
 */
void sdrv_smc_lld_ap_ram_lp_ctrl(paddr_t base, uint32_t index,
                                 domain_lp_mode lp, bool pd_en,
                                 uint32_t lp_setting)
{
    if (index < 2) {
        uint32_t v = sdrv_smc_lld_readl(base, AP_RAM_LP_CTL_OFF(index));

        if (pd_en) {
            if (HIB_MODE == lp) {
                v |= BM_AP_RAM_LP_CTL_HIB_RAM_PD_EN;
            } else {
                v |= BM_AP_RAM_LP_CTL_SLP_RAM_PD_EN;
            }
        } else {
            if (HIB_MODE == lp) {
                v &= ~BM_AP_RAM_LP_CTL_HIB_RAM_PD_EN;
            } else {
                v &= ~BM_AP_RAM_LP_CTL_SLP_RAM_PD_EN;
            }
        }

        if (HIB_MODE == lp) {
            v &= ~FM_AP_RAM_LP_CTL_HIB_RAM_LP_SETTING;
            v |= FV_AP_RAM_LP_CTL_HIB_RAM_LP_SETTING(lp_setting);
        } else {
            v &= ~FM_AP_RAM_LP_CTL_SLP_RAM_LP_SETTING;
            v |= FV_AP_RAM_LP_CTL_SLP_RAM_LP_SETTING(lp_setting);
        }

        sdrv_smc_lld_writel(base, AP_RAM_LP_CTL_OFF(index), v);
    }
}

/**
 * @brief Safety domain request LP
 *
 * @param[in] base reg base
 * @param[in] lp lower power mode
 *
 */
void sdrv_smc_lld_saf_request_lp(paddr_t base, domain_lp_mode lp)
{
    uint32_t v = sdrv_smc_lld_readl(base, SAF_GLB_CTL_OFF);

    if (SLP_MODE == lp) {
        v &= ~BM_SAF_GLB_CTL_LP_MODE;
    } else {
        v |= BM_SAF_GLB_CTL_LP_MODE;
    }

    v &= ~BM_SAF_GLB_CTL_WFI_DIS;

    sdrv_smc_lld_writel(base, SAF_GLB_CTL_OFF, v);
}

/**
 * @brief Safety domain disable LP
 *
 * @param[in] base reg base
 *
 */
void sdrv_smc_lld_saf_disable_lp(paddr_t base)
{
    uint32_t v = sdrv_smc_lld_readl(base, SAF_GLB_CTL_OFF);

    v &= ~BM_SAF_GLB_CTL_LP_MODE;
    v |= BM_SAF_GLB_CTL_WFI_DIS;

    sdrv_smc_lld_writel(base, SAF_GLB_CTL_OFF, v);
}

/**
 * @brief AP domain request LP
 *
 * @param[in] base reg base
 * @param[in] lp lower power mode
 *
 */
void sdrv_smc_lld_ap_request_lp(paddr_t base, domain_lp_mode lp)
{
    uint32_t v = sdrv_smc_lld_readl(base, AP_GLB_CTL_OFF);

    if (SLP_MODE == lp) {
        v &= ~BM_AP_GLB_CTL_LP_MODE;
    } else {
        v |= BM_AP_GLB_CTL_LP_MODE;
    }

    v &= ~BM_AP_GLB_CTL_WFI_DIS;

    sdrv_smc_lld_writel(base, AP_GLB_CTL_OFF, v);
}

/**
 * @brief AP domain disable LP
 *
 * @param[in] base reg base
 *
 */
void sdrv_smc_lld_ap_disable_lp(paddr_t base)
{
    uint32_t v = sdrv_smc_lld_readl(base, AP_GLB_CTL_OFF);

    v &= ~BM_AP_GLB_CTL_LP_MODE;
    v |= BM_AP_GLB_CTL_WFI_DIS;

    sdrv_smc_lld_writel(base, AP_GLB_CTL_OFF, v);
}

/**
 * @brief saf&ap domain ram power down
 * note: ram low power setting default
 *
 * @param[in] base reg base
 * @param[in] saf_pd_msk 4 bit valid,means bit 0-3. if 1, the corresponding core
 * pd enable, otherwise pd disable
 * @param[in] ap_pd_msk 1 bit valid,means bit 0. if 1, the corresponding core pd
 * enable, otherwise pd disable
 * @param[in] ret1n,ret2n ram configuration params
 *
 */
void sdrv_smc_lld_saf_ap_ram_pd(paddr_t base, uint8_t saf_ram_msk,
                                uint8_t ap_ram_msk, uint8_t ret1n,
                                uint8_t ret2n)
{
    for (uint8_t i = 0; i < 5u; i++) {
        if (saf_ram_msk & (0x01u << i)) {
            saf_ram_pd_config(base, 1, i);
            saf_ram_pd_setting_config(
                base, ((0x1 << 2) | (ret1n << 1) | (ret2n << 0)), i);
        } else {
            saf_ram_pd_config(base, 0, i);
        }
    }

    for (uint8_t i = 0; i < 2u; i++) {
        if (ap_ram_msk & (0x01u << i)) {
            ap_ram_pd_config(base, 1, i);
            ap_ram_pd_setting_config(
                base, ((0x1 << 2) | (ret1n << 1) | (ret2n << 0)), i);
        } else {
            ap_ram_pd_config(base, 0, i);
        }
    }
}

/* TODO : ret1n and ret2n different value for each core */

/**
 * @brief ap domain align with saf domain
 *
 * @param[in] base reg base
 *
 */
void sdrv_smc_lld_ap_lp_align_saf_config(paddr_t base)
{
    ap_lp_align_saf_config(base, 1);
}

/****************************
 * soc lp mode configuration *
 ****************************/

/***************************
 * soc wakeup configuration *
 ***************************/
/**
 * @brief soc wakeup by lp2wkup timeout irq, need to clear the irq in irq
 * handler
 *
 * @param[in] base reg base
 * @param[in] tout_val timeout value to receive wakeup event, otherwise wakeup
 * by self
 *
 */
void sdrv_smc_lld_soc_lp2wkup_config(paddr_t base, uint16_t tout_val)
{
    soc_lp2wkup_timeout_config(base, tout_val, 1);

    /* TODO: which core to wakeup if lp2wkup happen */
    soc_wkup_irq_wkup_config(base, 0x1F, 0x1F);
}

/**
 * @brief ap domain align with saf domain
 *
 * @param[in] base reg base
 *
 */
void sdrv_smc_lld_ap_wakeup_align_saf_config(paddr_t base)
{
    ap_wkup_align_saf_config(base, 1);
}

/**
 * @brief monitor the soc wakeup timeout from wakeup event received to soc
 * wakeup
 *
 * @param[in] base reg base
 * @param[in] wkup_src sakeup done source select, soc_to_run or all active cpu
 * ack done
 * @param[in] tout_val timeout value for wakeup event receive to wakeup done
 *
 */
void sdrv_smc_lld_soc_wkup_timeout_config(paddr_t base, uint8_t wkup_src,
                                          uint16_t tout_val)
{
    if (wkup_src) {
        soc_to_run_timeout_config(base, tout_val);
    } else {
        soc_wkup_ack_timeout_config(base, tout_val);
    }
}
/***************************
 * soc wakeup configuration *
 ***************************/

/*********************************
 * watchdog monitor configuration *
 ********************************/
/**
 * @brief watchdog monitor for soc swm, including ckgen/rstgen/pmu handshake and
 * mode transaction note:  not including soc wakeup timeout, refer to function
 * soc_wkup_timeout_setting
 *
 * @param[in] base reg base
 * @param[in] dom timeout monitor domain, saf or ap or soc level
 * @param[in] handshake_stage timeout monitor stage, ckgen handshake, rstgen
 * handshake, pmu handshake, mode transaction
 * @param[in] tout_val timeout value
 * @param[in] tout_wkup_en tiomeout wakeup enable
 *
 */
int sdrv_smc_lld_soc_swm_timeout_config(paddr_t base, smc_rs_type dom,
                                        handshake_stage tout_stage,
                                        uint8_t tout_val, bool tout_wkup_en)
{
    int ret = -1;

    if (SAF_DOMAIN == dom) {
        if (tout_stage <= MODE_REQ_TRANS) {
            ret = saf_timeout_config(base, 1, tout_stage, tout_val);
            saf_timeout_wkup_config(base, tout_wkup_en);
        }
    } else if (AP_DOMAIN == dom) {
        if (tout_stage <= MODE_REQ_TRANS) {
            ret = ap_timeout_config(base, 1, tout_stage, tout_val);
            ap_timeout_wkup_config(base, tout_wkup_en);
        }
    } else if (SOC_DOMAIN == dom) {
        if (tout_stage == PMU_HK) {
            soc_swm_timeout_config(base, 1, tout_val);
            soc_swm_timeout_wkup_config(base, tout_wkup_en);
            ret = 0;
        }
    }

    if (tout_wkup_en) {
        /* TODO: which core to wakeup when watchdog timeout */
        soc_wkup_irq_wkup_config(base, 0x1f, 0x1f);
    }

    return ret;
}
/*********************************
 * watchdog monitor configuration *
 *********************************/

/*******************************
 * irq management configuration *
 *******************************/
/**
 * @brief core&common irq mask configuration
 *
 * @param[in] base reg base
 * @param[in] smc_irq_type irq type, core irq or common irq
 * @param[in] core_idx core index
 * @param[in] irq_grp idx irq group index
 * @param[in] mask_val irq mask value
 *
 * @return -1 if irq mask fail
 */
int sdrv_smc_lld_irq_mask_config(paddr_t base, smc_irq_type type,
                                 uint32_t core_idx, uint32_t irq_grp_idx,
                                 uint32_t mask_val)
{
    int ret = -1;

    if (CORE_IRQ_TYPE == type) {
        ret = core_irq_mask_config(base, core_idx, mask_val);
    } else if (COMMON_IRQ_TYPE == type) {
        ret = com_irq_mask_config(base, core_idx, irq_grp_idx, mask_val);
    }

    return ret;
}

/**
 * @brief Wakeup irq unmask config
 *
 * @param[in] base reg base
 * @param[in] core core id
 * @param[in] irq irq number
 *
 */
void sdrv_smc_lld_irq_unmask(paddr_t base, uint32_t core, uint32_t irq)
{
    if (core < 5 && irq < 256) {
        if (irq < 16) {
            uint32_t v = sdrv_smc_lld_readl(base, CORE_IRQ_MASK_OFF(core));
            v &= ~(0x1u << irq);
            sdrv_smc_lld_writel(base, CORE_IRQ_MASK_OFF(core), v);
        } else {
            uint32_t offset, v;

            irq -= 16;

            switch (irq / 32) {
            case 0:
                offset = GET_COM_IRQ_MASK_BASE(0, core);
                break;

            case 1:
                offset = GET_COM_IRQ_MASK_BASE(1, core);
                break;

            case 2:
                offset = GET_COM_IRQ_MASK_BASE(2, core);
                break;

            case 3:
                offset = GET_COM_IRQ_MASK_BASE(3, core);
                break;

            case 4:
                offset = GET_COM_IRQ_MASK_BASE(4, core);
                break;

            case 5:
                offset = GET_COM_IRQ_MASK_BASE(5, core);
                break;

            case 6:
                offset = GET_COM_IRQ_MASK_BASE(6, core);
                break;

            case 7:
                offset = GET_COM_IRQ_MASK_BASE(7, core);
                break;

            default:
                return;
            }

            v = sdrv_smc_lld_readl(base, offset);
            v &= ~(0x1u << (irq % 32));
            sdrv_smc_lld_writel(base, offset, v);
        }
    }
}

/**
 * @brief Get core irq status
 *
 * @param[in] base reg base
 * @param[in] core core id
 *
 * @return irq status
 */
uint16_t sdrv_smc_lld_get_core_irq_mon(paddr_t base, uint32_t core)
{
    uint32_t stat = 0;

    if (core < 5) {
        stat = sdrv_smc_lld_readl(base, CORE_IRQ_MON_OFF(core));
    }

    return (uint16_t)stat;
}

/**
 * @brief Clear core irq status
 *
 * @param[in] base reg base
 * @param[in] core core id
 * @param[in] clear clear bit field
 *
 */
void sdrv_smc_lld_clear_core_irq_mon(paddr_t base, uint32_t core,
                                     uint16_t clear)
{
    uint32_t stat;

    if (core < 5) {
        stat = sdrv_smc_lld_readl(base, CORE_IRQ_MON_OFF(core));
        stat &= ~((uint32_t)clear);
        sdrv_smc_lld_writel(base, CORE_IRQ_MON_OFF(core), stat);
    }
}

/**
 * @brief Get comm irq status
 *
 * @param[in] base reg base
 * @param[in] grp group
 *
 * @return irq status
 */
uint32_t sdrv_smc_lld_get_comm_irq_mon(paddr_t base, uint32_t grp)
{
    uint32_t stat = 0;
    uint32_t offset;

    if (grp < 8) {
        offset = COM_IRQ_MON_0_OFF + grp * 4u;
        stat = sdrv_smc_lld_readl(base, offset);
    }

    return stat;
}

/**
 * @brief Clear comm irq status
 *
 * @param[in] base reg base
 * @param[in] grp group
 * @param[in] clear clear bit field
 *
 */
void sdrv_smc_lld_clear_comm_irq_mon(paddr_t base, uint32_t grp, uint32_t clear)
{
    uint32_t stat;
    uint32_t offset;

    if (grp < 8) {
        offset = COM_IRQ_MON_0_OFF + grp * 4u;
        stat = sdrv_smc_lld_readl(base, offset);
        stat &= ~clear;
        sdrv_smc_lld_writel(base, offset, stat);
    }
}

/**
 * @brief core&common irq monitor configuration
 *
 * @param[in] base reg base
 * @param[in] smc_irq_type irq type, core irq or common irq
 * @param[in] index core index if core irq, group index if common irq
 * @param[in] irq_state pointer to irq state
 *
 * @return -1 if get irq state fail, the irq state in pointer irq_state not
 * valid
 */
int sdrv_smc_lld_get_irq_state(paddr_t base, smc_irq_type type, uint32_t index,
                               uint32_t *irq_state)
{
    int ret = -1;

    if (CORE_IRQ_TYPE == type) {
        ret = core_irq_monitor(base, index, irq_state);
    } else if (COMMON_IRQ_TYPE == type) {
        ret = com_irq_monitor(base, index, irq_state);
    }

    return ret;
}

/**
 * @brief core&common irq clear configuration
 *
 * @param[in] base reg base
 * @param[in] smc_irq_type irq type, core irq or common irq
 * @param[in] index core index if core irq, group index if common irq
 * @param[in] clr_bit_mask 1 means not clear and 0 means clear
 *
 * @return -1 if clear irq fail
 */
int sdrv_smc_lld_clr_irq_state(paddr_t base, smc_irq_type type, uint32_t index,
                               uint32_t clr_bit_mask)
{
    int ret = -1;

    if (CORE_IRQ_TYPE == type) {
        ret = core_irq_clear(base, index, clr_bit_mask);
    } else if (COMMON_IRQ_TYPE == type) {
        ret = com_irq_clear(base, index, clr_bit_mask);
    }

    return ret;
}

/**
 * @brief get and clear smc irq state which cause wakeup
 * note: should be called in the irq handler if the wakeup irq is smc irq
 *
 * @param[in] base reg base
 *
 * @return the smc irq state before clear
 */
uint8_t sdrv_smc_lld_clr_wkup_irq_state(paddr_t base)
{
    uint8_t wkup_irq = 0;

    soc_wkup_irq_clr(base, &wkup_irq);

    return wkup_irq;
}
/*******************************
 * irq management configuration *
 *******************************/

/************************************
 * status and interrupt configuration *
 ************************************/
/**
 * @brief smc interrupt configuration
 *
 * @param[in] base reg base
 * @param[in] int_en interrupt enable
 * @param[in] type which interrupt to be moniter,enable or clear
 *
 */
void sdrv_smc_lld_interrupt_enable_config(paddr_t base, smc_int_type type,
                                          bool int_en)
{
    switch (type) {
    case WDT_LKSTEP_INT:
        wdt_lkstep_int_en_config(base, int_en);
        break;

    case SMC_FUSA_INT:
        smc_fusa_int_en_config(base, int_en);
        break;

    case APB_LKSTEP_INT:
        apb_lkstep_int_en_config(base, int_en);
        break;

    case APB_ERR_INT:
        apb_err_int_en_config(base, int_en);
        break;

    case SMC_PERM_INT:
        smc_func_int_en_config(base, int_en);
        break;

    default:
        break;
    }
}

/**
 * @brief smc interrupt monitor configuration
 *
 * @param[in] base reg base
 * @param[in] type which interrupt to be moniter,enable or clear
 *
 */
uint8_t sdrv_smc_lld_interrupt_monitor_config(paddr_t base, smc_int_type type)
{
    uint8_t int_sta = 0xFF;

    switch (type) {
    case WDT_LKSTEP_INT:
        int_sta = wdt_lkstep_int_mon_config(base);
        break;

    case SMC_FUSA_INT:
        int_sta = smc_fusa_int_mon_config(base);
        break;

    case APB_LKSTEP_INT:
        int_sta = apb_lkstep_int_mon_config(base);
        break;

    case APB_ERR_INT:
        int_sta = apb_err_int_mon_config(base);
        break;

    case SMC_PERM_INT:
        int_sta = smc_func_int_mon_config(base);
        break;

    default:
        break;
    }

    return int_sta;
}

/**
 * @brief smc interrupt clear configuration
 *
 * @param[in] base reg base
 * @param[in] type which interrupt to be moniter,enable or clear
 *
 */
void sdrv_smc_lld_interrupt_clear_config(paddr_t base, smc_int_type type)
{
    switch (type) {
    case WDT_LKSTEP_INT:
        wdt_lkstep_int_clr_config(base);
        break;

    case SMC_FUSA_INT:
        smc_fusa_int_clr_config(base);
        break;

    case APB_LKSTEP_INT:
        apb_lkstep_int_clr_config(base);
        break;

    case APB_ERR_INT:
        apb_err_int_clr_config(base);
        break;

    case SMC_PERM_INT:
        smc_func_int_clr_config(base);
        break;

    default:
        break;
    }
}

/**
 * @brief monitor the status of saf/ap/soc domain and saf/ap inter status
 *
 * @param[in] base reg base
 * @param[in] type which status to monitor
 *
 */
int8_t sdrv_smc_lld_swm_status_monitor(paddr_t base, swm_st_type type)
{
    return swm_status_monitor(base, type);
}

/**
 * @brief monitor the status of smc timeout
 *
 * @param[in] base reg base
 *
 */
uint16_t sdrv_smc_lld_timeout_status_monitor(paddr_t base)
{
    return smc_tout_sta_config(base);
}

/**
 * @brief monitor the illegal transaction
 *
 * @param[in] base reg base
 *
 */
uint16_t sdrv_smc_lld_ill_trans_status_monitor(paddr_t base)
{
    return smc_ill_trans_sta_config(base);
}
/************************************
 * status and interrupt configuration *
 ************************************/

/******************************
 * clock monitor configuration *
 ******************************/
/**
 * @brief clock pre divider for either 32K or 24M clk
 *
 * @param[in] base reg base
 * @param[in] type which clk, 32K or 24M
 * @param[in] div_num divider number for the clk
 *
 */
void sdrv_smc_lld_soc_clk_pre_div_config(paddr_t base, clk_type type,
                                         uint8_t div_num)
{
    if (CLK32K == type) {
        soc_32k_div_num_config(base, div_num);
    } else if (CLK24M == type) {
        soc_24m_div_num_config(base, div_num);
    }
}

/**
 * @brief RC24M disable when soc in hib mode
 *
 * @param[in] base reg base
 *
 */
void sdrv_smc_lld_soc_hib_rc_dis_config(paddr_t base)
{
    soc_hib_rc_dis_config(base, 1);
}

/**
 * @brief clk32k monitor clk24m
 *
 * @param[in] base reg base
 * @param[in] force_mon force enable, monitor active ignore the 32k/24m clk
 * status
 * @param[in] hw_mon monitor active when 32k/24m both active
 * @param[in] clk_src_type monitor which 24m clk, rc or xtal
 * @param[in] l_thrd, h_thrd low and high threshold
 *
 */
void sdrv_smc_lld_clk32k_mon_clk24k_config(paddr_t base, bool force_mon,
                                           bool hw_mon, clk_src_type type,
                                           uint8_t l_thrd, uint8_t h_thrd)
{
    if (RC_CLK == type) {
        soc_cmon_rc_config(base, h_thrd, l_thrd);
    } else if (XTAL_CLK == type) {
        soc_cmon_xtal_config(base, h_thrd, l_thrd);
    }

    soc_clk_mon_config(base, hw_mon, force_mon);
}

/**
 * @brief soc clock frequency monitor configuration
 *
 * @param[in] base reg base
 * @param[in] freq pointer to the updated frequency
 *
 * return whether the frequency update or not
 */
bool sdrv_smc_lld_cmon_freq_update_config(paddr_t base, uint16_t *freq)
{
    bool ret = false;

    ret = soc_cmon_freq_update_config(base, freq);

    return ret;
}

/**
 * @brief clear the soc clk mon update frequency
 *
 * @param[in] base reg base
 */
void sdrv_smc_lld_soc_cmon_freq_clear_config(paddr_t base)
{
    soc_cmon_freq_clear_config(base);
}

/******************************
 * clock monitor configuration *
 ******************************/

/*****************************
 * software swm configuration *
 *****************************/
/**
 * @brief software setting for handshake with rstgen/ckgen/pmu
 *
 * @param[in] base reg base
 * @param[in] dom_type domain type, ap or saf or soc level
 * @param[in] mode mode info for handshake, run/slp/hib/rtc
 * @param[in] stage handshake with which module, ckgen/rstgen/pmu
 *
 */
void sdrv_smc_lld_sw_swm_config(paddr_t base, smc_rs_type dom_type,
                                sw_swm_mode mode, handshake_stage stage)
{
    if (AP_DOMAIN == dom_type) {
        if (CKGEN_HK == stage) {
            sw_ap_ckgen_swm_config(base, mode);
        } else if (RSTGEN_HK == stage) {
            sw_ap_rstgen_swm_config(base, mode);
        }
    } else if (SAF_DOMAIN == dom_type) {
        if (CKGEN_HK == stage) {
            sw_saf_ckgen_swm_config(base, mode);
        } else if (RSTGEN_HK == stage) {
            sw_saf_rstgen_swm_config(base, mode);
        }
    } else if (SOC_DOMAIN == dom_type) {
        if (PMU_HK == stage) {
            sw_soc_pmu_swm_config(base, mode);
        }
    }
}

/**
 * @brief software setting for ap&saf domain power down
 *
 * @param[in] base reg base
 * @param[in] dom_type domain type, ap or saf
 * @param[in] sub_dom_idx which part in this domain power down
 * @param[in] pd_en power down or power on
 * @param[in] stage power gate or isolation enable
 *
 * @return -1 if sub_dom_idx out of range
 */
int sdrv_smc_lld_sw_domain_lp_config(paddr_t base, smc_rs_type dom_type,
                                     uint8_t sub_dom_idx, sw_pd_stage stage,
                                     bool pd_en)
{
    int ret = -1;
    if (SAF_DOMAIN == dom_type) {
        ret = sw_saf_lp_config(base, sub_dom_idx, stage, pd_en);
    } else if (AP_DOMAIN == dom_type) {
        sw_ap_lp_config(base, stage, pd_en);
        ret = 0;
    }

    return ret;
}

/**
 * @brief software setting for ap&saf domain ram power down
 *
 * @param[in] base reg base
 * @param[in] dom_type domain type, ap or saf
 * @param[in] mem_dom_idx ram domain for each domain
 * @param[in] pd_en power down or power on
 * @param[in] ret1n,ret2n ram configuration params
 *
 * @return -1 if mem_dom_idx out of range
 */
int sdrv_smc_lld_sw_domain_ram_lp_config(paddr_t base, smc_rs_type dom_type,
                                         uint8_t mem_dom_idx, bool pd_en,
                                         uint8_t ret1n, uint8_t ret2n)
{
    int ret = -1;

    if (SAF_DOMAIN == dom_type) {
        ret = sw_saf_ram_lp_config(base, mem_dom_idx, pd_en, ret1n, ret2n);
    } else if (AP_DOMAIN == dom_type) {
        ret = sw_ap_ram_lp_config(base, mem_dom_idx, pd_en, ret1n, ret2n);
    }

    return ret;
}

void sdrv_smc_lld_sw_saf_ram_lp_ctrl(paddr_t base, uint32_t index,
                                     bool pd_en, uint32_t lp_setting)
{
    if (index < 5) {
        uint32_t v = sdrv_smc_lld_readl(base, SAF_RAM_LP_CTL_OFF(index));

        if (pd_en) {
            v |= BM_SAF_RAM_LP_CTL_SW_RAM_PD_EN;
            v &= ~FM_SAF_RAM_LP_CTL_SW_RAM_LP_SETTING;
            v |= FV_SAF_RAM_LP_CTL_SW_RAM_LP_SETTING(lp_setting);
        }
        else {
            v &= ~BM_SAF_RAM_LP_CTL_SW_RAM_PD_EN;
            v &= ~FM_SAF_RAM_LP_CTL_SW_RAM_LP_SETTING;
            v |= FV_SAF_RAM_LP_CTL_SW_RAM_LP_SETTING(0x3);
        }

        sdrv_smc_lld_writel(base, SAF_RAM_LP_CTL_OFF(index), v);
    }
}

void sdrv_smc_lld_sw_ap_ram_lp_ctrl(paddr_t base, uint32_t index,
                                    bool pd_en, uint32_t lp_setting)
{
    if (index < 2) {
        uint32_t v = sdrv_smc_lld_readl(base, AP_RAM_LP_CTL_OFF(index));

        if (pd_en) {
            v |= BM_AP_RAM_LP_CTL_SW_RAM_PD_EN;
            v &= ~FM_AP_RAM_LP_CTL_SW_RAM_LP_SETTING;
            v |= FV_AP_RAM_LP_CTL_SW_RAM_LP_SETTING(lp_setting);
        }
        else {
            v &= ~BM_AP_RAM_LP_CTL_SW_RAM_PD_EN;
            v &= ~FM_AP_RAM_LP_CTL_SW_RAM_LP_SETTING;
            v |= FV_AP_RAM_LP_CTL_SW_RAM_LP_SETTING(0x3);
        }

        sdrv_smc_lld_writel(base, AP_RAM_LP_CTL_OFF(index), v);
    }
}
/*****************************
 * software swm configuration *
 *****************************/
