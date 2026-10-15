/*
 * FreeRTOS Kernel V10.2.1
 * Copyright (C) 2019 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://www.FreeRTOS.org
 * http://aws.amazon.com/freertos
 *
 * 1 tab == 4 spaces!
 */

	INCLUDE config.h
	INCLUDE arch/irq.h

#if CONFIG_ARMV7R_TCMA_BASE == 0
	SECTION .tcmcode:CODE:ROOT(2)
#else
	SECTION .text:CODE:ROOT(2)
#endif
	arm

SYS_MODE			EQU     0x1f
SVC_MODE			EQU     0x13
IRQ_MODE			EQU     0x12
SYS_save_context			EQU    0x0
SYS_restore_context			EQU    0x1
SYS_switch_context			EQU    0x2
SCTLR_M						EQU    0x1
SCTLR_BR					EQU    0x20000

	/* Variables and functions. */
	EXTERN ulCriticalNesting
	EXTERN ulMaxAPIPriority
	EXTERN pxCurrentTCB
	EXTERN vTaskSwitchContext
	EXTERN vApplicationIRQHandler
	EXTERN ulPortInterruptNesting
	EXTERN ulPortTaskHasFPUContext
	EXTERN ulPortYieldRequired
	EXTERN vPortSetInterruptMaskAsm
	EXTERN arch_clean_invalidate_dcache_all

	PUBLIC vPortRestoreTaskContext
	PUBLIC osSysCallHandler
	PUBLIC osIrqHandler
	PUBLIC osFiqHandler

osSysCallHandler				EQU FreeRTOS_SWI_Handler
osIrqHandler 					EQU FreeRTOS_IRQ_Handler
osFiqHandler					EQU FreeRTOS_FIQ_Handler

portSAVE_CONTEXT MACRO

	/* Save the LR and SPSR onto the system mode stack before switching to
	system mode to save the remaining system mode registers. */
	SRSDB	sp!, #SYS_MODE
	CPS		#SYS_MODE
	PUSH	{R0-R12, R14}

	/* Push the critical nesting count. */
	LDR		R2, ulCriticalNestingConst
	LDR		R1, [R2]
	PUSH	{R1}

	/* Does the task have a floating point context that needs saving?  If
	ulPortTaskHasFPUContext is 0 then no. */
	LDR		R2, ulPortTaskHasFPUContextConst
	LDR		R3, [R2]
	CMP		R3, #0

	/* Save the floating point context, if any. */
	FMRXNE  R1,  FPSCR
	VPUSHNE {D0-D15}
	/*VPUSHNE	{D16-D31}*/
	PUSHNE	{R1}

	/* Save ulPortTaskHasFPUContext itself. */
	PUSH	{R3}

	/* Save the stack pointer in the TCB. */
	LDR		R0, pxCurrentTCBConst
	LDR		R1, [R0]
	STR		SP, [R1]

	ENDM

; /**********************************************************************/

portRESTORE_CONTEXT MACRO

	/* Set the SP to point to the stack of the task being restored. */
	LDR		R0, pxCurrentTCBConst
	LDR		R1, [R0]
	LDR		SP, [R1]

	/* Is there a floating point context to restore?  If the restored
	ulPortTaskHasFPUContext is zero then no. */
	LDR		R0, ulPortTaskHasFPUContextConst
	POP		{R1}
	STR		R1, [R0]
	CMP		R1, #0

	/* Restore the floating point context, if any. */
	POPNE 	{R0}
	/*VPOPNE	{D16-D31}*/
	VPOPNE	{D0-D15}
	VMSRNE  FPSCR, R0

	/* Restore the critical section nesting depth. */
	LDR		R0, ulCriticalNestingConst
	POP		{R1}
	STR		R1, [R0]

	/* Ensure the priority mask is correct for the critical nesting depth. */
	CMP		R1, #0
	MOVEQ	R0, #255
	LDRNE	R0, ulMaxAPIPriorityConst
	LDRNE	R0, [R0]
	LDR		R4, vPortSetInterruptMaskAsmConst
	BLX		R4

	/* Restore all system mode registers other than the SP (which is already
	being used). */
	POP		{R0-R12, R14}

	/* Return to the task code, loading CPSR on the way. */
	RFEIA	sp!

	ENDM


; /**********************************************************************/

portMPU_RegisterSave MACRO

	MOV		r0, #0
mpu_save_loop:
	MCR		p15, 0, r0, c6, c2, 0
	MRC		p15, 0, r2, c6, c1, 0
	STR		r2, [r1], #4
	MRC		p15, 0, r2, c6, c1, 4
	STR		r2, [r1], #4
	MRC		p15, 0, r2, c6, c1, 2
	STR		r2, [r1], #4
	ADD		r0, r0, #1
	CMP		r0, #16
	BNE		mpu_save_loop
	MRC		p15, 0, r2, c1, c0, 0
	STR		r2, [r1], #4

	ENDM

; /**********************************************************************/

portMPU_RegisterRestore MACRO

	MRC		p15, 0, r2, c1, c0, 0
	LDR		r0, =(SCTLR_BR | SCTLR_M)
	BIC		r2, r2, r0
	MCR		p15, 0, r2, c1, c0, 0

	MOV		r0, #0
mpu_restore_loop:
	MCR		p15, 0, r0, c6, c2, 0
	LDR		r2, [r1], #4
	MCR		p15, 0, r2, c6, c1, 0
	LDR		r2, [r1], #4
	MCR		p15, 0, r2, c6, c1, 4
	LDR		r2, [r1], #4
	MCR		p15, 0, r2, c6, c1, 2
	ADD		r0, r0, #1
	CMP		r0, #16
	BNE		mpu_restore_loop

	MRC		p15, 0, r0, c1, c0, 0
	LDR		r2, =(SCTLR_BR | SCTLR_M)
	ORR		r0, r0, r2
	LDR		r2, [r1], #4
	TST		r2, #SCTLR_M
	MCRNE	p15, 0, r0, c1, c0, 0

	ENDM

; /**********************************************************************/

portCPU_RegisterSave MACRO

	PUSH	{r0-r3, r12}

	STMIA	r1, {r0-r12}
	STR		lr, [r1, #(4*REG_R15)]
	MRS		r2, SPSR
	STR		r2, [r1, #(4*REG_CPSR)]

#ifdef CONFIG_ARCH_WITH_FPU
	ADD		r3, r1, #(4*REG_S0)
	VSTMIA	r3!, {s0-s31}
	VMRS 	r2, fpscr
	STR		r2, [r3], #4
#endif

	CPS		#SYS_MODE
	ADD		r3, r1, #(4*REG_R13)
	STMIA	r3!, {sp, lr}

	CPS		#SVC_MODE
	ADD		r1, r1, #XCPTCONTEXT_SIZE
	portMPU_RegisterSave

	PUSH	{lr}
	LDR		r0, arch_clean_invalidate_dcache_all_const
	BLX		r0
	POP		{lr}

	POP		{r0-r3, r12}
	MOVS	pc, lr

	ENDM

; /**********************************************************************/

portCPU_RegisterRestore MACRO

#ifdef CONFIG_ARCH_WITH_FPU
	ADD		r3, r1, #(4*REG_S0)
	VLDMIA	r3!, {s0-s31}
	LDR		r2, [r3], #4
	VMSR	fpscr, r2
#endif

	ADD		r4, r1, #(4*REG_R15)
	LDMIA	r4, {r2, r3}
	MOV		lr, r2
	MSR		SPSR_cxsf, r3

	CPS		#SYS_MODE
	ADD		r4, r1, #(4*REG_R13)
	LDMIA	r4, {r2, r3}
	MOV		sp, r2
	MOV		lr, r3

	PUSH	{lr}
	MOV		lr, r1

	ADD		r1, r1, #XCPTCONTEXT_SIZE
	portMPU_RegisterRestore

	LDMIA	lr, {r0-r12}
	POP		{lr}

	CPS		#SVC_MODE
	MOVS	pc, lr

	ENDM

/******************************************************************************
 * FIQ handler is used to start the scheduler.
 *****************************************************************************/

FreeRTOS_FIQ_Handler:
	PUSH	{r0-r3, r12, lr}
	BLX		vApplicationIRQHandler
	POP		{r0-r3, r12, lr}
	SUBS	pc, lr, #4

/******************************************************************************
 * SVC handler is used to start the scheduler.
 *****************************************************************************/

FreeRTOS_SWI_Handler:
	CMP		r0, #SYS_switch_context
	BNE		core_context_restore

	/* Save the context of the current task and select a new task to run. */
	portSAVE_CONTEXT
	LDR R0, vTaskSwitchContextConst
	BLX	R0
	portRESTORE_CONTEXT

core_context_restore:
	CMP		r0, #SYS_restore_context
	BNE		core_context_save

	/* CPU register restore from buffer, r1 */
	portCPU_RegisterRestore

core_context_save:
	CMP		r0, #SYS_save_context
	BNE		swi_handler_fault

	/* CPU register save to buffer, r1 */
	portCPU_RegisterSave

swi_handler_fault:
	B 		.

/******************************************************************************
 * vPortRestoreTaskContext is used to start the scheduler.
 *****************************************************************************/

vPortRestoreTaskContext:
	/* Switch to system mode. */
	CPS		#SYS_MODE
	portRESTORE_CONTEXT

FreeRTOS_IRQ_Handler:

	/* Return to the interrupted instruction. */
	SUB		lr, lr, #4

	/* Push the return address and SPSR. */
	PUSH	{lr}
	MRS		lr, SPSR
	PUSH	{lr}

	/* Change to supervisor mode to allow reentry. */
	CPS		#SVC_MODE

	/* Push used registers. */
	PUSH	{r0-r4, r12}

	/* Increment nesting count.  r3 holds the address of ulPortInterruptNesting
	for future use.  r1 holds the original ulPortInterruptNesting value for
	future use. */
	LDR		r3, ulPortInterruptNestingConst
	LDR		r1, [r3]
	ADD		r4, r1, #1
	STR		r4, [r3]

	/* Ensure bit 2 of the stack pointer is clear.  r2 holds the bit 2 value for
	future use.  _RB_ Is this ever needed provided the start of the stack is
	alligned on an 8-byte boundary? */
	MOV		r2, sp
	AND		r2, r2, #4
	SUB		sp, sp, r2

	/* Call the interrupt handler. */
	PUSH	{r0-r4, lr}
	LDR		r1, vApplicationIRQHandlerConst
	BLX		r1
	POP		{r0-r4, lr}
	ADD		sp, sp, r2

	CPSID	i
	DSB
	ISB

	/* Restore the old nesting count. */
	STR		r1, [r3]

	/* A context switch is never performed if the nesting count is not 0. */
	CMP		r1, #0
	BNE		exit_without_switch

	/* Did the interrupt request a context switch?  r1 holds the address of
	ulPortYieldRequired and r0 the value of ulPortYieldRequired for future
	use. */
	LDR		r1, =ulPortYieldRequired
	LDR		r0, [r1]
	CMP		r0, #0
	BNE		switch_before_exit

exit_without_switch:
	/* No context switch.  Restore used registers, LR_irq and SPSR before
	returning. */
	POP		{r0-r4, r12}
	CPS		#IRQ_MODE
	POP		{LR}
	MSR		SPSR_cxsf, LR
	POP		{LR}
	MOVS	PC, LR

switch_before_exit:
	/* A context swtich is to be performed.  Clear the context switch pending
	flag. */
	MOV		r0, #0
	STR		r0, [r1]

	/* Restore used registers, LR-irq and SPSR before saving the context
	to the task stack. */
	POP		{r0-r4, r12}
	CPS		#IRQ_MODE
	POP		{LR}
	MSR		SPSR_cxsf, LR
	POP		{LR}
	portSAVE_CONTEXT

	/* Call the function that selects the new task to execute.
	vTaskSwitchContext() if vTaskSwitchContext() uses LDRD or STRD
	instructions, or 8 byte aligned stack allocated data.  LR does not need
	saving as a new LR will be loaded by portRESTORE_CONTEXT anyway. */
	LDR		R0, vTaskSwitchContextConst
	BLX		R0

	/* Restore the context of, and branch to, the task selected to execute
	next. */
	portRESTORE_CONTEXT

pxCurrentTCBConst				DC32 pxCurrentTCB
ulCriticalNestingConst			DC32 ulCriticalNesting
ulPortTaskHasFPUContextConst	DC32 ulPortTaskHasFPUContext
ulMaxAPIPriorityConst			DC32 ulMaxAPIPriority
vTaskSwitchContextConst			DC32 vTaskSwitchContext
vApplicationIRQHandlerConst		DC32 vApplicationIRQHandler
ulPortInterruptNestingConst		DC32 ulPortInterruptNesting
vPortSetInterruptMaskAsmConst	DC32 vPortSetInterruptMaskAsm
arch_clean_invalidate_dcache_all_const		DC32 arch_clean_invalidate_dcache_all
	END
