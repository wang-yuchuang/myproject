/*
 * arm_arch.c
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: ARMV7R arch function.
 *
 * Revision History:
 * -----------------
 */

#include <types.h>
#include <compiler.h>
#include <arch.h>
#include <arch/cache.h>
#include <arch/fpu.h>
#include <arch/pmu.h>

#ifdef CONFIG_ARCH_STACK_DYNAMIC
/* exception handler stacks */
#define __EXCEPT_STACK  __SECTION(".except.stack")
uint8_t abt_stack[CONFIG_ARCH_ABT_STACK_SIZE] __CACHE_ALIGN __EXCEPT_STACK;
uint8_t sys_stack[CONFIG_ARCH_SYS_STACK_SIZE] __CACHE_ALIGN __EXCEPT_STACK;
uint8_t und_stack[CONFIG_ARCH_UND_STACK_SIZE] __CACHE_ALIGN __EXCEPT_STACK;
uint8_t fiq_stack[CONFIG_ARCH_FIQ_STACK_SIZE] __CACHE_ALIGN __EXCEPT_STACK;
uint8_t irq_stack[CONFIG_ARCH_IRQ_STACK_SIZE] __CACHE_ALIGN __EXCEPT_STACK;
#if CONFIG_ARMV7R_TCMA_BASE == 0
uint8_t svc_stack[CONFIG_ARCH_SVC_STACK_SIZE] __CACHE_ALIGN __SECTION(".tcm.except.stack");
#else
uint8_t svc_stack[CONFIG_ARCH_SVC_STACK_SIZE] __CACHE_ALIGN __EXCEPT_STACK;
#endif
#endif

/*
 * arch early init function.
 * call before kernel init.
 */
void arch_early_init(void)
{
#ifdef CONFIG_ENABLE_CYCLE_COUNTER
    pmu_enable();
    pmu_start_cycle_cntr(false);
#endif

    /* fpu config */
#ifdef CONFIG_ARCH_USE_FPU
    arm_fpu_enable();
#endif

    /* enable i&d cache */
#if !(defined (CONFIG_ARCH_EARLY_ENABLE_ICACHE) || \
      defined (CONFIG_ARCH_DISABLE_ICACHE))
    arch_enable_cache(ICACHE);
#endif

#if !(defined (CONFIG_ARCH_EARLY_ENABLE_DCACHE) || \
      defined (CONFIG_ARCH_DISABLE_DCACHE))
    arch_enable_cache(DCACHE);
#endif
}

/*
 * arch init function.
 * call after kernel init.
 */
void arch_init(void)
{
}

/*
 * arch idle function.
 */
void arch_idle(void)
{
    __ASM volatile("wfi");
}

/*
 * arch check exception stack.
 */
bool arch_check_exception_stack(void)
{
#ifdef CONFIG_ARCH_STACK_COLORATION
#ifdef CONFIG_ARCH_STACK_DYNAMIC
    if ((*(uint32_t *)abt_stack != ARCH_STACK_COLOR) || \
        (*(uint32_t *)sys_stack != ARCH_STACK_COLOR) || \
        (*(uint32_t *)und_stack != ARCH_STACK_COLOR) || \
        (*(uint32_t *)fiq_stack != ARCH_STACK_COLOR) || \
        (*(uint32_t *)irq_stack != ARCH_STACK_COLOR) || \
        (*(uint32_t *)svc_stack != ARCH_STACK_COLOR)) {
        return false;
    }
#endif
#endif
    return true;
}

void arm_prefetch_burst_enable(void)
{
    uint32_t actlr = arm_read_actlr();

    // ARM_CPU_CORTEX_R5 close branch prediction not use sctlr |= (1 << 11);
    // /*BP:This field controls the branch prediction policy:
    // b00 = Normal operation. This is the reset value.
    // b01 = Branch always taken and history table updates disabled.
    // b10 = Branch always not taken and history table updates disabled.
    // b11 = Reserved. Behavior is Unpredictable if this field is set to b11.*/
    actlr &= ~(0x3 << 15);
    actlr &= ~(1 << 14);
    actlr &= ~(1 << 17);
    arm_write_actlr(actlr);
}

void arm_prefetch_burst_disable(void)
{
    uint32_t actlr = arm_read_actlr();
    // ARM_CPU_CORTEX_R5 close branch prediction not use sctlr |= (1 << 11);
    // /*BP:This field controls the branch prediction policy:
    // b00 = Normal operation. This is the reset value.
    // b01 = Branch always taken and history table updates disabled.
    // b10 = Branch always not taken and history table updates disabled.
    // b11 = Reserved. Behavior is Unpredictable if this field is set to b11.*/
    actlr |= (0x2 << 15);
    actlr |= (1 << 14);
    actlr |= (1 << 17);
    arm_write_actlr(actlr);
}