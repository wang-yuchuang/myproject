/*
 * exceptions.h
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: ARMV7R exceptions interface.
 *
 * Revision History:
 * -----------------
 */

#ifndef INCLUDE_ARCH_ARMV7R_EXCEPTIONS_H
#define INCLUDE_ARCH_ARMV7R_EXCEPTIONS_H

struct arm_iframe {
#if CONFIG_ARCH_USE_FPU
    uint32_t fpexc;
#endif
    uint32_t usp;
    uint32_t ulr;
    uint32_t r0;
    uint32_t r1;
    uint32_t r2;
    uint32_t r3;
    uint32_t r4;
    uint32_t r12;
    uint32_t lr;
    uint32_t pc;
    uint32_t spsr;
};

struct arm_fault_frame {
#if CONFIG_ARCH_USE_FPU
    uint32_t fpexc;
#endif
    uint32_t usp;
    uint32_t ulr;
    uint32_t r[13];
    uint32_t lr;
    uint32_t pc;
    uint32_t spsr;
};

struct arm_mode_regs {
    uint32_t usr_r13, usr_r14;
    uint32_t fiq_r13, fiq_r14;
    uint32_t irq_r13, irq_r14;
    uint32_t svc_r13, svc_r14;
    uint32_t abt_r13, abt_r14;
    uint32_t und_r13, und_r14;
    uint32_t sys_r13, sys_r14;
};

void arm_save_mode_regs(struct arm_mode_regs *regs);
void arm_undefined(void);
void arm_prefetch_abort(void);
void arm_data_abort(void);
void arm_reserved(void);

#endif
