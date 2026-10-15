/*
 * irq.h
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: ARM irq interface.
 *
 * Revision History:
 * -----------------
 */

#ifndef INCLUDE_ARCH_IRQ_H
#define INCLUDE_ARCH_IRQ_H


#define REG_R0              (0)
#define REG_R1              (1)
#define REG_R2              (2)
#define REG_R3              (3)
#define REG_R4              (4)
#define REG_R5              (5)
#define REG_R6              (6)
#define REG_R7              (7)
#define REG_R8              (8)
#define REG_R9              (9)
#define REG_R10             (10)
#define REG_R11             (11)
#define REG_R12             (12)
#define REG_R13             (13)
#define REG_R14             (14)
#define REG_R15             (15)
#define REG_CPSR            (16)

#define ARM_CONTEXT_REGS    (17)

/* If the MCU supports a floating point unit, then it will be necessary
 * to save the state of the FPU status register and data registers on
 * each context switch.  These registers are not saved during interrupt
 * level processing, however. So, as a consequence, floating point
 * operations may NOT be performed in interrupt handlers.
 *
 * The FPU provides an extension register file containing 32 single-
 * precision registers. These can be viewed as:
 *
 * - Sixteen 64-bit double word registers, D0-D15
 * - Thirty-two 32-bit single-word registers, S0-S31
 *   S<2n> maps to the least significant half of D<n>
 *   S<2n+1> maps to the most significant half of D<n>.
 */

#ifdef CONFIG_ARCH_WITH_FPU
#  define REG_D0            (ARM_CONTEXT_REGS+0)  /* D0 */
#  define REG_S0            (ARM_CONTEXT_REGS+0)  /* S0 */
#  define REG_S1            (ARM_CONTEXT_REGS+1)  /* S1 */
#  define REG_D1            (ARM_CONTEXT_REGS+2)  /* D1 */
#  define REG_S2            (ARM_CONTEXT_REGS+2)  /* S2 */
#  define REG_S3            (ARM_CONTEXT_REGS+3)  /* S3 */
#  define REG_D2            (ARM_CONTEXT_REGS+4)  /* D2 */
#  define REG_S4            (ARM_CONTEXT_REGS+4)  /* S4 */
#  define REG_S5            (ARM_CONTEXT_REGS+5)  /* S5 */
#  define REG_D3            (ARM_CONTEXT_REGS+6)  /* D3 */
#  define REG_S6            (ARM_CONTEXT_REGS+6)  /* S6 */
#  define REG_S7            (ARM_CONTEXT_REGS+7)  /* S7 */
#  define REG_D4            (ARM_CONTEXT_REGS+8)  /* D4 */
#  define REG_S8            (ARM_CONTEXT_REGS+8)  /* S8 */
#  define REG_S9            (ARM_CONTEXT_REGS+9)  /* S9 */
#  define REG_D5            (ARM_CONTEXT_REGS+10) /* D5 */
#  define REG_S10           (ARM_CONTEXT_REGS+10) /* S10 */
#  define REG_S11           (ARM_CONTEXT_REGS+11) /* S11 */
#  define REG_D6            (ARM_CONTEXT_REGS+12) /* D6 */
#  define REG_S12           (ARM_CONTEXT_REGS+12) /* S12 */
#  define REG_S13           (ARM_CONTEXT_REGS+13) /* S13 */
#  define REG_D7            (ARM_CONTEXT_REGS+14) /* D7 */
#  define REG_S14           (ARM_CONTEXT_REGS+14) /* S14 */
#  define REG_S15           (ARM_CONTEXT_REGS+15) /* S15 */
#  define REG_D8            (ARM_CONTEXT_REGS+16) /* D8 */
#  define REG_S16           (ARM_CONTEXT_REGS+16) /* S16 */
#  define REG_S17           (ARM_CONTEXT_REGS+17) /* S17 */
#  define REG_D9            (ARM_CONTEXT_REGS+18) /* D9 */
#  define REG_S18           (ARM_CONTEXT_REGS+18) /* S18 */
#  define REG_S19           (ARM_CONTEXT_REGS+19) /* S19 */
#  define REG_D10           (ARM_CONTEXT_REGS+20) /* D10 */
#  define REG_S20           (ARM_CONTEXT_REGS+20) /* S20 */
#  define REG_S21           (ARM_CONTEXT_REGS+21) /* S21 */
#  define REG_D11           (ARM_CONTEXT_REGS+22) /* D11 */
#  define REG_S22           (ARM_CONTEXT_REGS+22) /* S22 */
#  define REG_S23           (ARM_CONTEXT_REGS+23) /* S23 */
#  define REG_D12           (ARM_CONTEXT_REGS+24) /* D12 */
#  define REG_S24           (ARM_CONTEXT_REGS+24) /* S24 */
#  define REG_S25           (ARM_CONTEXT_REGS+25) /* S25 */
#  define REG_D13           (ARM_CONTEXT_REGS+26) /* D13 */
#  define REG_S26           (ARM_CONTEXT_REGS+26) /* S26 */
#  define REG_S27           (ARM_CONTEXT_REGS+27) /* S27 */
#  define REG_D14           (ARM_CONTEXT_REGS+28) /* D14 */
#  define REG_S28           (ARM_CONTEXT_REGS+28) /* S28 */
#  define REG_S29           (ARM_CONTEXT_REGS+29) /* S29 */
#  define REG_D15           (ARM_CONTEXT_REGS+30) /* D15 */
#  define REG_S30           (ARM_CONTEXT_REGS+30) /* S30 */
#  define REG_S31           (ARM_CONTEXT_REGS+31) /* S31 */
#  define REG_FPSCR         (ARM_CONTEXT_REGS+32) /* Floating point status and control */
#  define FPU_CONTEXT_REGS  (33)
#else
#  define FPU_CONTEXT_REGS  (0)
#endif

/* The total number of registers saved by software */

#define XCPTCONTEXT_REGS    (ARM_CONTEXT_REGS + FPU_CONTEXT_REGS)
#define XCPTCONTEXT_SIZE    (4 * XCPTCONTEXT_REGS)


#ifndef ASSEMBLY

#include <compiler.h>
#include <stdbool.h>

__BEGIN_CDECLS

/*
 * irq state type.
 */
typedef unsigned int  irq_state_t;

/*
 * irq enable.
 */
static inline void arch_irq_enable(void)
{
    CF;
    __ASM volatile("cpsie i");
}

/*
 * irq disable.
 */
static inline void arch_irq_disable(void)
{
    __ASM volatile("cpsid i");
    CF;
}

/*
 * irq save.
 *
 * @return  old irq state.
 */
static inline irq_state_t arch_irq_save(void)
{
    unsigned int cpsr;

    __ASM volatile
    (
        "\tmrs    %0, cpsr\n"
        "\tcpsid  i\n"
        : "=r" (cpsr)
        :
        : "memory"
    );

    return cpsr;
}

/*
 * irq restore.
 *
 * @flags   old irq state.
 */
static inline void arch_irq_restore(irq_state_t flags)
{
    __ASM volatile
    (
        "msr    cpsr_c, %0"
        :
        : "r" (flags)
        : "memory"
    );
}

/*
 * is irq masked.
 *
 * @return   masked or not.
 */
bool arch_irq_is_masked(void);

/*
 * is irq mode.
 *
 * @return   irq mode or not.
 */
bool arch_in_irq_mode(void);

/**
 * @brief Enable vectored interrupt mode.
 */
void arch_vectored_irq_enable(bool en);

/**
 * @brief Whether core is in FIQ mode or not
 *
 * @return true FIQ mode
 * @return false otherwise
 */
bool arch_in_fiq_mode(void);

__END_CDECLS

#endif

#endif
