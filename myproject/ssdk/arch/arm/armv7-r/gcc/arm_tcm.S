/*
 * arm_tcm.S
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: Cortex V7R TCM driver.
 *
 * Revision History:
 * -----------------
 */

#include <compiler.h>
#include <arch/armv7-r/tcm.h>

#ifdef CONFIG_ARM_WITH_TCM

.text

.macro tcm_enable, idx, addr_reg, temp_reg
    orr     \temp_reg, \addr_reg, #1
    mcr     p15, 0, \temp_reg, c9, c1, \idx
.endm

.macro tcm_enable_ecc, temp_reg, val
    mrc     p15, 0, \temp_reg, c1, c0, 1
    orr     \temp_reg, \temp_reg, #(\val)
    mcr     p15, 0, \temp_reg, c1, c0, 1
.endm

.macro tcm_disable_ecc, temp_reg, val
    mrc     p15, 0, \temp_reg, c1, c0, 1
    bic     \temp_reg, \temp_reg, #(\val)
    mcr     p15, 0, \temp_reg, c1, c0, 1
.endm

.macro tcm_get_size, idx, size_reg, temp_reg
    mrc     p15, 0, \size_reg, c9, c1, \idx
    ubfx    \size_reg, \size_reg, #2, #5
    adds    \size_reg, #9
    movs    \temp_reg, #1
    lsls    \size_reg, \temp_reg, \size_reg
.endm

.macro tcm_clear, addr_reg, size_reg, temp_reg1, temp_reg2
    mov     \temp_reg1, #0
    mov     \temp_reg2, #0
    add     \size_reg, \addr_reg, \size_reg
.Ltcm_loop\@:
    cmp     \addr_reg, \size_reg
    bhs     .Ltcm_loop_end\@
    stmia   \addr_reg!, {\temp_reg1-\temp_reg2}
    b       .Ltcm_loop\@
 .Ltcm_loop_end\@:
.endm

/* This function not store registers,
 * only used before stack setup.
 */
FUNCTION(tcma_enable_early)

    /* disable ecc */
    tcm_disable_ecc r4, ATCMPCEN

    /* read tcma size, r2 save size */
    tcm_get_size 1, r2, r4

    /* enable tcma, r0 save addr */
    tcm_enable 1, r0, r4

    /* check enable ecc */
    cmp     r1, #1
    bne     .Lout

    /* clear tcma */
    tcm_clear r0, r2, r6, r7

    /* enalbe ecc */
    tcm_enable_ecc r4, ATCMPCEN

   b       .Lout

/* This function not store registers,
 * only used before stack setup.
 */
FUNCTION(tcmb_enable_early)

    /* disable ecc */
    tcm_disable_ecc r4, (B0TCMPCEN | B1TCMPCEN)

    /* read tcma size, r2 save size */
    tcm_get_size 0, r2, r4

    /* enable tcma, r0 save addr */
    tcm_enable 0, r0, r4

    /* check enable ecc */
    cmp     r1, #1
    bne     .Lout

    /* clear tcma */
    tcm_clear r0, r2, r6, r7

    /* enalbe ecc */
    tcm_enable_ecc r4, (B0TCMPCEN | B1TCMPCEN)

.Lout:
    bx      lr

#endif
