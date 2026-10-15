/*
 * sd_vic_baremetal.S
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: SemiDrive VIC interrupt handlers for baremetal.
 *
 * Revision History:
 * -----------------
 */

#if __ICCARM__
    extern get_vic_data_base
    extern sdrv_vic_lld_get_priority
#ifdef CONFIG_VIC_INT_NEST_ERRATA
    extern sdrv_vic_lld_mask_low_priority
#endif
    extern g_vic_user_isr
#endif

#if __GNUC__
.macro save_fpu_context
#elif __ICCARM__
save_fpu_context macro
#endif
    vmrs r1, fpexc
    tst r1, #(1 << 30)
    beq save_exit
    /* Save callee corrupted fpu registers */
    vpush {d0-d7}
    /* Save fpscr and fpexc */
    vmrs r1, fpscr
    push {r1}
    vmrs r1, fpexc
save_exit:
    push {r1}
#if __GNUC__
.endm
#elif __ICCARM__
    endm
#endif

#if __GNUC__
.macro restore_fpu_context
#elif __ICCARM__
restore_fpu_context macro
#endif
    pop {r1}
    vmsr fpexc, r1
    tst r1, #(1 << 30)
    beq restore_exit
    pop {r1}
    vmsr fpscr, r1
    vpop {d0-d7}
restore_exit:
#if __GNUC__
.endm
#elif __ICCARM__
    endm
#endif

#if __GNUC__
.macro vectored_irq_handler vector_num
    .globl vectored_irq\vector_num\()_handler
vectored_irq\vector_num\()_handler:
    sub lr, lr, #4
    srsdb sp!, #SVC_MODE

    cps #SVC_MODE

    /* Save callee and common_irq_handler corrupted registers */
    push {r0-r4, r12}

    mov r0, #\vector_num
    b .Lcommon_irq_handler
.endm
#elif __ICCARM__
vectored_irq_handler macro vector_num
    public vectored_irq\1_handler
vectored_irq\1_handler:
    sub lr, lr, #4
    srsdb sp!, #SVC_MODE

    cps #SVC_MODE

    /* Save callee and common_irq_handler corrupted registers */
    push {r0-r4, r12}

    mov r0, #vector_num
    b .Lcommon_irq_handler
    endm
#endif

.Lcommon_irq_handler:
    /* Save fpu context, will corrupt r1 */
    save_fpu_context

    /* Force sp to align to 8 bytes */
    mov r1, sp
    bic sp, sp, #7
    /* Save old sp and supervisor lr */
    push {r1, lr}

    /* Save vector number into callee-saved register */
    mov r4, r0

    /* Save old running priority */
    blx get_vic_data_base
    add r1, r0, #8 /* r1 now holds address of {struct vic_data}.cur_rp*/
    ldr r2, [r1]
    push {r1, r2}
    /* Update current running priority */
    mov r0, r4
    blx sdrv_vic_lld_get_priority
    ldr r1, [sp]
    str r0, [r1]

    ldr r1, [r1, #8]

#ifdef CONFIG_VIC_INT_NEST_ERRATA
    push {r0, r1}
    mov r1, #0
    /* Mask all interrupt with the same and lower priority
     * to prevent:
     * 1) same or lower priority interrupt preempting after
     *    writing VICADDRESS
     * 2) same sw priority but higher hw priority interrupt
     *    preempting
     */
    blx sdrv_vic_lld_mask_low_priority

    /* Because irq_b given by vic can only be cleared by CPU
     * acknowledge to vic, so if vic outputs another valid
     * irq_b signal triggered by interrupt with the same sw
     * priority but lower interrupt number before masking it,
     * CPU will take it after IRQ re-enabled.
     * To avoid this, here we should read vicaddress register
     * first to clear the additional irq_b signal, and then
     * writing vicaddress register to clear hwmask to make all
     * interrupts, including this additional interrupt, can be
     * sent to CPU at the right time.
     */
    ldr r1, [sp, #8]
    ldr r1, [r1, #-8]
    add r1, r1, #0xF00
    ldr r0, [r1]
    /* Perform a dummy write to VICADDRESS register to clear hwmask */
    str r4, [r1]
    pop {r0, r1}
#endif

    /* Make sure only unmasked interrupt can be taken. */
    cmp r0, r1
    bge .Lirq_out

    /* Increment interrupt nesting counter */
    ldr r1, [sp]
    ldr r2, [r1, #4]!   /* r1 now holds address of {struct vic_data}.irq_nest_cnt */
    add r0, r2, #1
    str r0, [r1]
    push {r1, r2}

#ifdef CONFIG_VIC_INT_NEST_AUTO_ENABLE
    cmp r0, #CONFIG_VIC_INT_NEST_MAX_CNT
    bge .Lskip_reenable_irq
    cpsie i
.Lskip_reenable_irq:
#endif

    /* Call user ISR */
    mov r0, r4
    ldr r1, =g_vic_user_isr
    ldr r1, [r1]
    blx r1

    cpsid i

#ifndef CONFIG_VIC_INT_NEST_ERRATA
    ldr r1, [sp]
    ldr r1, [r1, #-12]
    add r1, r1, #0xF00
    /* Perform a dummy write to VICADDRESS register to clear hwmask */
    str r4, [r1]
#endif

    /* Decrement interrupt nesting counter */
    pop {r1, r2}
    str r2, [r1]

.Lirq_out:
    /* Restore current running priority */
    pop {r1, r2}
    str r2, [r1]

#ifdef CONFIG_VIC_INT_NEST_ERRATA
    /* Unmask priority to previous state. */
    mov r0, r2
    mov r1, #0
    blx sdrv_vic_lld_mask_low_priority
#endif

    /* Restore sp and supervisor lr */
    pop {r1, lr}
    mov sp, r1

    restore_fpu_context

    /* Restore context */
    pop {r0-r4, r12}
    rfeia sp!

#if __GNUC__
    .ltorg
#elif __ICCARM__
    ltorg
#endif
