/*
 * arm_start.S
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: ARM start function.
 *
 * Revision History:
 * -----------------
 */
    INCLUDE config.h
    INCLUDE compiler.h
    INCLUDE iar/lnk_sym.h
    INCLUDE arm.h
    INCLUDE arch.h
    INCLUDE arch/cache.h

    PUBLIC _start

    EXTERN __data_start, __data_start_rom, __data_end, __bss_start, __bss_end
    EXTERN irq_stack, fiq_stack, abt_stack, und_stack, sys_stack, svc_stack
    EXTERN __except_stack_start, __except_stack_end

    EXTERN tcma_enable_early
    EXTERN tcmb_enable_early
    EXTERN arch_enable_cache
    EXTERN osSysCallHandler
    EXTERN osIrqHandler
    EXTERN osFiqHandler
    EXTERN board_startup
    EXTERN arm_undefined
    EXTERN arm_prefetch_abort
    EXTERN arm_data_abort
    EXTERN arm_reserved

#ifdef CONFIG_PM
    EXTERN arm_context_restore
#endif

    PUBLIC __iar_program_start
    EXTWEAK __iar_data_init3
    EXTWEAK __iar_init_core
    EXTWEAK __iar_init_vfp
    EXTWEAK __iar_argc_argv

    SECTION .vectab:CODE(2)

_start:
    ldr pc,   arm_reset_const
    ldr pc,   arm_undefined_const
    ldr pc,   arm_syscall_const
    ldr pc,   arm_prefetch_abort_const
    ldr pc,   arm_data_abort_const
    ldr pc,   arm_reserved_const
    ldr pc,   arm_irq_const
    ldr pc,   arm_fiq_const

arm_reset_const                 DC32 __iar_program_start
arm_undefined_const             DC32 arm_undefined
arm_syscall_const               DC32 osSysCallHandler
arm_prefetch_abort_const        DC32 arm_prefetch_abort
arm_data_abort_const            DC32 arm_data_abort
arm_reserved_const              DC32 arm_reserved
arm_irq_const                   DC32 osIrqHandler
arm_fiq_const                   DC32 osFiqHandler

    SECTION .boot:CODE(2)

__iar_program_start:
    /* initialize ARM registers recommended by ARM*/
    mov r0, #0 /* initialise r0-r12 */
    mov r1, #0
    mov r2, #0
    mov r3, #0
    mov r4, #0
    mov r5, #0
    mov r6, #0
    mov r7, #0
    mov r8, #0
    mov r9, #0
    mov r10, #0
    mov r11, #0
    mov r12, #0

    /* do some early cpu setup */
    mov     r0, #(PSR_MODE_SVC | PSR_I_BIT | PSR_F_BIT)
    msr     cpsr_c, r0

    mrc     p15, 0, r12, c1, c0, 0
    bic     r12, r12, #(SCTLR_M | SCTLR_A | SCTLR_C)
    bic     r12, r12, #(SCTLR_SW | SCTLR_I | SCTLR_RR)
    bic     r12, r12, #(SCTLR_EE | SCTLR_TE)
    orr     r12, r12, #(SCTLR_Z)
#ifdef CONFIG_ARM_HIGHVECTORS
    orr     r12, r12, #(SCTLR_V)
#else
    bic     r12, r12, #(SCTLR_V)
#endif
    mcr     p15, 0, r12, c1, c0, 0

    /* Configure peripheral ports */
    mrc     p15, 0, r0, c15, c0, 1
    tst     r0, #(0x1F << 2)
    beq     .Lno_normal_axi_pp
    /* Enable LLPP normal AXI interface */
    orr     r0, r0, #1
    mcr     p15, 0, r0, c15, c0, 1
.Lno_normal_axi_pp:
    mrc     p15, 0, r0, c15, c0, 2
    tst     r0, #(0x1F << 2)
    beq     .Lno_virtual_axi_pp
    /* Enable LLPP virtual AXI interface */
    orr     r0, r0, #1
    mcr     p15, 0, r0, c15, c0, 2
.Lno_virtual_axi_pp:
    mrc     p15, 0, r0, c15, c0, 3
    tst     r0, #(0x1F << 2)
    beq     .Lno_ahb_pp
    /* Enable AHB peripheral interface */
    orr     r0, r0, #1
    mcr     p15, 0, r0, c15, c0, 3
.Lno_ahb_pp:

    /* enable tcm before use stacks */
#ifdef CONFIG_ARMV7R_USE_TCMA
    ldr     r0, =CONFIG_ARMV7R_TCMA_BASE
    mov     r1, #0
#ifdef CONFIG_ARMV7R_TCMA_ECC
    add     r1, #1
#endif
    bl      tcma_enable_early
#endif

#ifdef CONFIG_ARMV7R_USE_TCMB
    ldr     r0, =CONFIG_ARMV7R_TCMB_BASE
    mov     r1, #0
#ifdef CONFIG_ARMV7R_TCMB_ECC
    add     r1, #1
#endif
    bl      tcmb_enable_early
#endif

#ifdef CONFIG_ARCH_STACK_DYNAMIC
.Lstack_setup:
    /* set up the stack for irq, fiq, abort, undefined, system/user, and lastly supervisor mode */
    mov     r12, #0

    /* Setup normal interrupt stack */
    cpsid   i, #MODE_IRQ
    mov     sp, r0 /* initialise R13_irq */
    mov     lr, #0 /* initialise R14_irq */
    ldr     r12, =irq_stack
    add     r12, #CONFIG_ARCH_IRQ_STACK_SIZE
    mov     sp, r12

    /* Setup fast interrupt stack */
    cpsid   i, #MODE_FIQ
    mov     r8, #0 /* initialise R8_fiq */
    mov     r9, #0 /* initialise R9_fiq */
    mov     r10, #0 /* initialise R10_fiq */
    mov     r11, #0 /* initialise R11_fiq */
    mov     r12, #0 /* initialise R12_fiq */
    mov     sp, r0 /* initialise R13_fiq */
    mov     lr, #0 /* initialise R14_fiq */
    ldr     r12, =fiq_stack
    add     r12, #CONFIG_ARCH_FIQ_STACK_SIZE
    mov     sp, r12

    /* Setup data abort stack */
    cpsid   i, #MODE_ABT
    mov     sp, r0 /* initialise R13_abt */
    mov     lr, #0 /* initialise R14_abt */
    ldr     r12, =abt_stack
    add     r12, #CONFIG_ARCH_ABT_STACK_SIZE
    mov     sp, r12

    /* Setup undefined instruction stack */
    cpsid   i, #MODE_UND
    mov     sp, r0 /* initialise R13_und */
    mov     lr, #0 /* initialise R14_und */
    ldr     r12, =und_stack
    add     r12, #CONFIG_ARCH_UND_STACK_SIZE
    mov     sp, r12

    /* Setup system/user stack */
    cpsid   i, #MODE_SYS
    mov     sp, r0 /* initialise R13_system/user */
    mov     lr, #0 /* initialise R14_system/user */
    ldr     r12, =sys_stack
    add     r12, #CONFIG_ARCH_SYS_STACK_SIZE
    mov     sp, r12

    /* Setup supervisor stack */
    cpsid   i, #MODE_SVC
    mov     sp, r0 /* initialise R13_svc */
    mov     lr, #0 /* initialise R14_svc */
    ldr     r12, =svc_stack
    add     r12, #CONFIG_ARCH_SVC_STACK_SIZE
    mov     sp, r12

#ifdef CONFIG_PM
    ldr     r0, arm_context_restore_const
    blx     r0
#endif

    /* Init stack with stack magic */
#ifdef CONFIG_ARCH_STACK_COLORATION
.Lstack_init:
    ldr     r4, =__except_stack_start
    ldr     r5, =__except_stack_end
    ldr     r6, =ARCH_STACK_COLOR
.Lstack_loop:
    cmp     r4, r5
    strlt   r6, [r4], #4
    blt     .Lstack_loop

#if CONFIG_ARMV7R_TCMA_BASE == 0
    ldr     r4, =svc_stack
    add     r5, r4, #CONFIG_ARCH_SVC_STACK_SIZE
.Ltcm_stack_loop:
    cmp     r4, r5
    strlt   r6, [r4], #4
    blt     .Ltcm_stack_loop
#endif
#endif
#endif

#ifdef CONFIG_ARCH_USE_FPU
    /* init FPU register */
    ldr     r0, =0x00F00000
    mcr     p15, 0, r0, c1, c0, 2	/* Enable coprocessor access in CPACR */
    ldr     r2, =0x40000000
    vmsr    fpexc, r2	/* Enable FPU access so that VFP registers can be initialized */
    vmov    D0, r5, r6	/* r5/6 is 0 now */
    vmov    D1, r5, r6
    vmov    D2, r5, r6
    vmov    D3, r5, r6
    vmov    D4, r5, r6
    vmov    D5, r5, r6
    vmov    D6, r5, r6
    vmov    D7, r5, r6
    vmov    D8, r5, r6
    vmov    D9, r5, r6
    vmov    D10, r5, r6
    vmov    D11, r5, r6
    vmov    D12, r5, r6
    vmov    D13, r5, r6
    vmov    D14, r5, r6
    vmov    D15, r5, r6
#endif

#if CONFIG_ARMV7R_TCMA_BASE == 0
    /* copy vector table */
    ldr r0, =sfb(.vectab)
    ldr r1, =CONFIG_ARMV7R_TCMA_BASE
    ldr r2, =sfe(.vectab)
.Lcpy_vectab:
    ldmia r0!, {r3-r10}
    stmia r1!, {r3-r10}
    cmp r0, r2
    blt .Lcpy_vectab
#endif

    /* enable cache, use stack enable after stack init */
#ifdef CONFIG_ARCH_EARLY_ENABLE_ICACHE
    ldr     r0, =ICACHE
    bl      arch_enable_cache
#endif

#ifdef CONFIG_ARCH_EARLY_ENABLE_DCACHE
    ldr     r0, =DCACHE
    bl      arch_enable_cache
#endif

    /* Execute relocations & zero BSS */
    FUNCALL __iar_program_start, __iar_data_init3
    bl      __iar_data_init3

    /* Turn on core features assumed to be enabled */
    FUNCALL __iar_program_start, __iar_init_core
    bl      __iar_init_core

    /* Initialize VFP (if needed) */
    FUNCALL __iar_program_start, __iar_init_vfp
    bl      __iar_init_vfp

    /* Setup command line */
    mov     r0, #0
    FUNCALL __iar_program_start, __iar_argc_argv
    bl      __iar_argc_argv

    /* change to system mode */
    cpsid   i, #MODE_SYS

    FUNCALL __iar_program_start, board_startup
    bl      board_startup
    b       .

#ifdef CONFIG_PM
arm_context_restore_const       DC32 arm_context_restore
#endif

    END
