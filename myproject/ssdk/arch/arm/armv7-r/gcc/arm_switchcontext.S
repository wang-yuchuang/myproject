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

#include <compiler.h>
#include "svcall.h"

/****************************************************************************
 * Public Functions
 ****************************************************************************/

	.text

/****************************************************************************
 * Name: arm_switchcontext
 ****************************************************************************/

	.globl	arm_switchcontext
	.type	arm_switchcontext, function

arm_switchcontext:

	/* Perform the System call with R0=2 */

	mov		r0, #SYS_switch_context	/* R0: context switch */
	svc		0						/* Force synchronous SVCall (or Hard Fault) */

	/* We will get here only after the rerturn from the context switch */

	bx		lr

	.size	arm_switchcontext, .-arm_switchcontext
	.end
