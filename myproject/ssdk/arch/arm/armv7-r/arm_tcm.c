/*
 * arm_tcm.c
 *
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: Cortex V7R TCM driver.
 *
 * Revision History:
 * -----------------
 */

#ifdef CONFIG_ARM_WITH_TCM

#include <types.h>
#include <stdbool.h>
#include <compiler.h>
#include <bits.h>
#include <param.h>
#include <debug.h>
#include <arch/armv7-r/tcm.h>
#include <arch/register.h>

/**
 * @brief Clear the TCM.
 */
static void tcm_clear(uint32_t base, size_t size)
{
    volatile uint32_t a = 0;
    volatile uint32_t b = 0;
    uint32_t end = base + size;

    /* Each store must be 64 bits aligned. */
    __ASM volatile(
        "clr_loop:      \n"
        "cmp %2, %3     \n"
        "bhs clr_end    \n"
        "stmia %2!, {%0, %1} \n"
        "b clr_loop     \n"
        "clr_end:       \n"
        : : "r" (a), "r" (b), "r" (base), "r" (end)
    );
}

/**
 * @brief Get TCMA size in bytes.
 */
static size_t tcma_get_size(void)
{
    uint32_t atcm_size = BITS_SHIFT(arm_read_atcmrgn(), 6, 2);
    return (1 << (atcm_size + 9));
}

/**
 * @brief Get TCMB size in bytes.
 */
static size_t tcmb_get_size(void)
{
    uint32_t btcm_size = BITS_SHIFT(arm_read_btcmrgn(), 6, 2);
    return (1 << (btcm_size + 9));
}

/**
 * @brief Enable the TCMA.
 */
void tcma_enable(uint32_t atcm_base, bool enable_ecc)
{
    size_t      atcm_size;
    uint32_t    actlr = arm_read_actlr();

    /* Disable ECC by default */
    actlr &= ~ATCMPCEN;
    arm_write_actlr(actlr);

    /* Get TCM size. */
    atcm_size = tcma_get_size();

    /* Update TCM base address. */
    if (atcm_size != 0) {
        ASSERT(IS_ALIGNED(atcm_base, atcm_size));
        arm_write_atcmrgn(atcm_base | 1);
    }

    if (enable_ecc) {
        /* Write to the TCM in 64-bit aligned quantities to initialize
         * ECC codes.
         */
        tcm_clear(atcm_base, atcm_size);

        /* Enable ECC.  */
        actlr |= ATCMPCEN;
        arm_write_actlr(actlr);
    }
}

/**
 * @brief Enable the TCMB.
 */
void tcmb_enable(uint32_t btcm_base, bool enable_ecc)
{
    size_t      btcm_size;
    uint32_t    actlr = arm_read_actlr();

    /* Disable ECC by default */
    actlr &= ~(B0TCMPCEN | B1TCMPCEN);
    arm_write_actlr(actlr);

    /* Get TCM size. */
    btcm_size = tcmb_get_size();

    /* Update TCM base address. */
    if (btcm_size != 0) {
        ASSERT(IS_ALIGNED(btcm_base, btcm_size));
        arm_write_btcmrgn(btcm_base | 1);
    }

    if (enable_ecc) {
        /* Write to the TCM in 64-bit aligned quantities to initialize
         * ECC codes.
         */
        tcm_clear(btcm_base, btcm_size);

        /* Enable ECC.  */
        actlr |= B0TCMPCEN | B1TCMPCEN;
        arm_write_actlr(actlr);
    }
}

#endif