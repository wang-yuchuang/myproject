/*
 * arm_irq.c
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: ARM irq function.
 *
 * Revision History:
 * -----------------
 */

#include <arm.h>
#include <arch/register.h>

/*
 * is irq masked.
 *
 * @return   masked or not.
 */
bool arch_irq_is_masked(void)
{
    uint32_t cpsr = read_cpsr();
    return (!!(cpsr & PSR_I_BIT));
}

/*
 * is irq mode.
 *
 * @return   irq mode or not.
 */
bool arch_in_irq_mode(void)
{
    uint32_t cpsr;
    uint8_t mode;

    cpsr = read_cpsr();
    mode = cpsr & MODE_MASK;
    return ((mode != MODE_USR) && (mode != MODE_SYS));
}

/**
 * @brief Enable vectored interrupt mode.
 */
void arch_vectored_irq_enable(bool en)
{
    uint32_t sctlr;

    sctlr = arm_read_sctlr();

    if (en) {
        sctlr |= 1U << 24;
    }
    else {
        sctlr &= ~(1U << 24);
    }

    arm_write_sctlr(sctlr);
}

/**
 * @brief Whether core is in FIQ mode or not
 *
 * @return true FIQ mode
 * @return false otherwise
 */
bool arch_in_fiq_mode(void)
{
    uint32_t cpsr = read_cpsr();

    return (cpsr & MODE_MASK) == MODE_FIQ;
}

