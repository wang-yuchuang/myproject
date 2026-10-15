/*
 * tcm.h
 *
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: Cortex V7R TCM driver.
 *
 * Revision History:
 * -----------------
 */
#ifndef INCLUDE_ARCH_ARMV7R_TCM_H
#define INCLUDE_ARCH_ARMV7R_TCM_H

#define ATCMPCEN        (1ul << 25)
#define B0TCMPCEN       (1ul << 26)
#define B1TCMPCEN       (1ul << 27)

#ifndef ASSEMBLY

#include <types.h>
#include <compiler.h>
#include <stdbool.h>

__BEGIN_CDECLS

/*
 * tcma enable.
 *
 * @atcm_base   tcma base address.
 * @enable_ecc  enable tcm ecc.
 */
#ifdef CONFIG_ARM_WITH_TCM
void tcma_enable(uint32_t tcm_base, bool enable_ecc);
#else
#define tcma_enable(tcm_base, enable_ecc)
#endif

/*
 * tcmb enable.
 *
 * @atcm_base   tcmb base address.
 * @enable_ecc  enable tcm ecc.
 */
#ifdef CONFIG_ARM_WITH_TCM
void tcmb_enable(uint32_t tcm_base, bool enable_ecc);
#else
#define tcmb_enable(tcm_base, enable_ecc)
#endif

__END_CDECLS

#endif

#endif
