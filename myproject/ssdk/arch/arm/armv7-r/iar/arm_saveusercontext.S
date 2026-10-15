/*
 * arm_saveusercontext.S
 *
 * Copyright (c) 2022 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: ARM save user context.
 *
 * Revision History:
 * -----------------
 */

    INCLUDE config.h
    INCLUDE compiler.h
    INCLUDE svcall.h

/****************************************************************************
 * Public Functions
 ****************************************************************************/

    PUBLIC arm_saveusercontext

    SECTION .text:CODE(2)

/****************************************************************************
 * Name: arm_saveusercontext
 ****************************************************************************/

arm_saveusercontext:

	/* Perform the System call with R0=0 and R1=regs */

	mov		r1, r0					/* R1: regs */
	mov		r0, #SYS_save_context	/* R0: save context (also return value) */
	svc		0						/* Force synchronous SVCall (or Hard Fault) */

	bx		lr

	END
