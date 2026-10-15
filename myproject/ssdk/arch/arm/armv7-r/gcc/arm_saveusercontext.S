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

#include <compiler.h>
#include "svcall.h"

/****************************************************************************
 * Public Functions
 ****************************************************************************/

	.text

/****************************************************************************
 * Name: arm_saveusercontext
 ****************************************************************************/

	.globl	arm_saveusercontext
	.type	arm_saveusercontext, function

arm_saveusercontext:

	/* Perform the System call with R0=0 and R1=regs */

	mov		r1, r0					/* R1: regs */
	mov		r0, #SYS_save_context	/* R0: save context (also return value) */
	svc		0						/* Force synchronous SVCall (or Hard Fault) */

	bx		lr

	.size	arm_saveusercontext, .-arm_saveusercontext
	.end
