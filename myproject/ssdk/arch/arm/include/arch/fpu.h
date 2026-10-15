/*
 * fpu.h
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: ARM fpu interface.
 *
 * Revision History:
 * -----------------
 */

#ifndef INCLUDE_ARCH_FPU_H
#define INCLUDE_ARCH_FPU_H

#ifndef ASSEMBLY

#include <compiler.h>

__BEGIN_CDECLS

/*
 * fpu enable.
 */
#ifdef CONFIG_ARCH_WITH_FPU
void arm_fpu_enable(void);
#else
#define arm_fpu_enable()
#endif

/*
 * fpu disable.
 */
#ifdef CONFIG_ARCH_WITH_FPU
void arm_fpu_disable(void);
#else
#define arm_fpu_disable()
#endif

__END_CDECLS

#endif

#endif
