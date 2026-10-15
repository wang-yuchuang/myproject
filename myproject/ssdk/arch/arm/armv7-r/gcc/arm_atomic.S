/*
 * arm_atomic.s
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: ARM atomic function.
 *
 * Revision History:
 * -----------------
 */

#include <compiler.h>

.text

FUNCTION(arch_atomic_swap)
.Lloop_atomic_swap:
    ldrex   r12, [r0]
    strex   r2, r1, [r0]
    cmp     r2, #0
    bne     .Lloop_atomic_swap
    mov     r0, r12
    bx      lr

FUNCTION(arch_atomic_add)
.Lloop_atomic_add:
    ldrex   r12, [r0]
    add     r2, r12, r1
    strex   r3, r2, [r0]
    cmp     r3, #0
    bne     .Lloop_atomic_add
    mov     r0, r12
    bx      lr

FUNCTION(arch_atomic_and)
.Lloop_atomic_and:
    ldrex   r12, [r0]
    and     r2, r12, r1
    strex   r3, r2, [r0]
    cmp     r3, #0
    bne     .Lloop_atomic_and
    mov     r0, r12
    bx      lr

FUNCTION(arch_atomic_or)
.Lloop_atomic_or:
    ldrex   r12, [r0]
    orr     r2, r12, r1
    strex   r3, r2, [r0]
    cmp     r3, #0
    bne     .Lloop_atomic_or
    mov     r0, r12
    bx      lr
