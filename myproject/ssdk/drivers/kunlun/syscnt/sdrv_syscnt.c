/**
 * @file sdrv_syscnt.c
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: semidriver syscnt driver.
 *
 * Revision History:
 * -----------------
 */

#include <types.h>

#include "reg.h"
#include "sdrv_syscnt.h"

/**
 * @brief sdrv syscnt enable.
 *
 * @param[in] rw_base rw register base
 * @param[in] enable enable or disable
 */
void sdrv_syscnt_lld_enable(uint32_t rw_base, bool enable)
{
    uint32_t reg = rw_base + SDRV_SYSCNT_CNTCR;
    uint32_t val = readl(reg);

    if (enable) {
        val |= (1 << SDRV_SYSCNT_CNTCR_EN);
    }
    else {
        val &= ~(1 << SDRV_SYSCNT_CNTCR_EN);
    }
    writel(val, reg);
}

/**
 * @brief sdrv syscnt set val.
 *
 * @param[in] rw_base rw register base
 * @param[in] val cnt val
 */
void sdrv_syscnt_lld_set_val(uint32_t rw_base, uint64_t val)
{
    uint32_t l_reg = rw_base + SDRV_SYSCNT_CNTCVL;
    uint32_t h_reg = rw_base + SDRV_SYSCNT_CNTCVU;

    writel(val & 0xffffffff, l_reg);
    writel(val >> 32, h_reg);
}

/**
 * @brief sdrv syscnt get val.
 *
 * @param[in] ro_base ro register base
 * @return cnt val
 */
uint64_t sdrv_syscnt_lld_get_val(uint32_t ro_base)
{
    uint32_t l_reg, l_reg_pre, h_reg;

    l_reg = readl(ro_base + SDRV_SYSCNT_CNTCVLREAD);

    do {
        l_reg_pre = l_reg;
        h_reg = readl(ro_base + SDRV_SYSCNT_CNTCVUREAD);
        l_reg = readl(ro_base + SDRV_SYSCNT_CNTCVLREAD);
    } while(l_reg_pre > l_reg);

    return (((uint64_t)h_reg) << 32) + l_reg;
}
