/*
 * irq.h
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: ARM irq interface.
 *
 * Revision History:
 * -----------------
 */

#ifndef INCLUDE_ARCH_IRQ_H
#define INCLUDE_ARCH_IRQ_H

#ifndef ASSEMBLY

#include <compiler.h>
#include <stdbool.h>

__BEGIN_CDECLS

/*
 * irq state type.
 */
typedef unsigned long irq_state_t;

/*
 * irq enable.
 */
void arch_irq_enable(void);

/*
 * irq disable.
 */
void arch_irq_disable(void);

/*
 * irq save.
 *
 * @return  old irq state.
 */
static inline irq_state_t arch_irq_save(void)
{
    unsigned long flags;

    __ASM volatile(
	"mrs	%0, daif \n\t"
	"msr	daifset, #2"
	: "=r" (flags)
	:
	: "memory");

   return flags;
}

/*
 * irq restore.
 *
 * @flags   old irq state.
 */
static inline void arch_irq_restore(irq_state_t flags)
{
    __ASM volatile(
	"msr	daif, %0"
	:
	: "r" (flags)
	: "memory");
}

/*
 * is irq masked.
 *
 * @return   masked or not.
 */
bool arch_irq_is_masked(void);

/*
 * is irq mode.
 *
 * @return   irq mode or not.
 */
bool arch_in_irq_mode(void);

/**
 * @brief Enable vectored interrupt mode.
 */
void arch_vectored_irq_enable(bool en);

/**
 * @brief Whether core is in FIQ mode or not
 *
 * @return true FIQ mode
 * @return false otherwise
 */
bool arch_in_fiq_mode(void);

__END_CDECLS

#endif

#endif
