/*
 * arm_fullcontextrestore.S
 *
 * Copyright (c) 2022 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: ARM full context restore.
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
 * Name: arm_fullcontextrestore
 ****************************************************************************/

	.globl	arm_fullcontextrestore
	.type	arm_fullcontextrestore, function

arm_fullcontextrestore:

	cps		#0x1F						/* Enter sys mode */

	/* Perform the System call with R0=1 and R1=regs */

	mov		r1, r0						/* R1: regs */
	mov		r0, #SYS_restore_context	/* R0: restore context */
	svc		0							/* Force synchronous SVCall (or Hard Fault) */

	/* This call should not return */

	bx		lr							/* Unnecessary ... will not return */
	.size	arm_fullcontextrestore, .-arm_fullcontextrestore
	.end
