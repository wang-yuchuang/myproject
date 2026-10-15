/**
 * @file sdrv_syscnt.h
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: semidriver syscnt driver.
 *
 * Revision History:
 * -----------------
 */

#ifndef DRV_LLD_SDRV_SYSCNT_H
#define DRV_LLD_SDRV_SYSCNT_H

#include <types.h>
#include <compiler.h>

/* SYS_CNT_RW Registers */
#define SDRV_SYSCNT_CNTCR           0x0u
#define SDRV_SYSCNT_CNTCR_EN        0u
#define SDRV_SYSCNT_CNTCVL          0x8u
#define SDRV_SYSCNT_CNTCVU          0xcu
#define SDRV_SYSCNT_CNTFIDO         0x20u

/* SYS_CNT_RO Registers */
#define SDRV_SYSCNT_CNTCVLREAD      0x0u
#define SDRV_SYSCNT_CNTCVUREAD      0x4u

#ifndef ASSEMBLY

__BEGIN_CDECLS

/**
 * @brief sdrv syscnt enable.
 *
 * @param[in] rw_base rw register base
 * @param[in] enable enable or disable
 */
void sdrv_syscnt_lld_enable(uint32_t rw_base, bool enable);

/**
 * @brief sdrv syscnt set val.
 *
 * @param[in] rw_base rw register base
 * @param[in] val cnt val
 */
void sdrv_syscnt_lld_set_val(uint32_t rw_base, uint64_t val);

/**
 * @brief sdrv syscnt get val.
 *
 * @param[in] ro_base ro register base
 * @return cnt val
 */
uint64_t sdrv_syscnt_lld_get_val(uint32_t ro_base);

__END_CDECLS

#endif

#endif /* DRV_LLD_SDRV_SYSCNT_H */
