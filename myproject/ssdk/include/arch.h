/*
 * arch.h
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: arch interface.
 *
 * Revision History:
 * -----------------
 */

#ifndef INCLUDE_ARCH_H
#define INCLUDE_ARCH_H

#ifdef CONFIG_ARCH_STACK_COLORATION
#define ARCH_STACK_COLOR       0x5A5A5A5A
#endif

#ifndef ASSEMBLY

#include <types.h>
#include <compiler.h>

__BEGIN_CDECLS

/*
 * arch early init before kernel start.
 */
void arch_early_init(void);

/*
 * arch init after kernel start.
 */
void arch_init(void);

/*
 * arch idle.
 */
void arch_idle(void);

/*
 * arch check exception stack.
 *
 * @return  check result.
 */
bool OS_CRITICAL_CODE arch_check_exception_stack(void);

/**
 * @brief enable branch prediction and write burst
 *
 */
void arm_prefetch_burst_enable(void);

/**
 * @brief disable branch prediction and write burst
 *
 */
void arm_prefetch_burst_disable(void);

__END_CDECLS

#endif

#endif
