/*
 * arm_switchcontext.S
 *
 * Copyright (c) 2022 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: ARM switch context.
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

    PUBLIC arm_switchcontext

    SECTION .text:CODE(2)

/****************************************************************************
 * Name: arm_switchcontext
 ****************************************************************************/

arm_switchcontext:

	/* Perform the System call with R0=2 */

	mov		r0, #SYS_switch_context	/* R0: context switch */
	svc		0						/* Force synchronous SVCall (or Hard Fault) */

	/* We will get here only after the rerturn from the context switch */

	bx		lr

	END
