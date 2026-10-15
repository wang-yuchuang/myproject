/*
 * irq.h
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: interrupt interface.
 *
 * Revision History:
 * -----------------
 */

#ifndef INCLUDE_DRV_IRQ_H
#define INCLUDE_DRV_IRQ_H

#include <types.h>
#include <compiler.h>

#include "arch/irq.h"

/* trigger mode */
#define TRIGGER_MODE_LEVEL         0
#define TRIGGER_MODE_EDGE          1

#ifndef ASSEMBLY

__BEGIN_CDECLS

/* irq callback type */
typedef int (*irq_handler)(uint32_t irq, void *arg);

/*
 * irq initialize.
 *
 * @base   irq base
 * @intr_num  irq number
 */
#ifdef CONFIG_IRQ
void irq_initialize(uint32_t base, uint32_t intr_num);
#else
#define irq_initialize(base, intr_num)
#endif

/*
 * irq attach.
 *
 * @irq     irq id.
 * @handler irq callback handler.
 * @arg     irq callback arg.
 * @return  result.
 */
#ifdef CONFIG_IRQ
int irq_attach(uint32_t irq, irq_handler handler, void *arg);
#else
#define irq_attach(irq, handler, arg)
#endif

/*
 * irq detach.
 *
 * @irq     irq id.
 * @return  result.
 */
#ifdef CONFIG_IRQ
int irq_detach(uint32_t irq);
#else
#define irq_detach(irq)
#endif

/*
 * interrupt dispatch.
 *
 * @return  irq dispatch result.
 */
#ifdef CONFIG_IRQ
int OS_CRITICAL_CODE irq_dispatch(void);
#else
#define irq_dispatch() 0
#endif

/*
 * irq enable.
 *
 * @irq   irq number.
 */
#ifdef CONFIG_IRQ
void irq_enable(uint32_t irq);
#else
#define irq_enable(irq)
#endif

/*
 * irq disable.
 *
 * @irq   irq number.
 */
#ifdef CONFIG_IRQ
void irq_disable(uint32_t irq);
#else
#define irq_disable(irq)
#endif

/*
 * interrupt set priority mask.
 *
 * @mask     int priority mask.
 * @return   old int priority mask.
 */
#ifdef CONFIG_IRQ
uint32_t irq_mask(uint32_t mask);
#else
#define irq_mask(mask)
#endif

/*
 * interrupt unmask all int.
 */
#ifdef CONFIG_IRQ
void irq_unmask(void);
#else
#define irq_unmask()
#endif

/*
 * set irq priority.
 *
 * @irq         irq number.
 * @priority    irq priority.
 */
#ifdef CONFIG_IRQ
void irq_set_priority(uint32_t irq, uint32_t priority);
#else
#define irq_set_priority(irq, priority)
#endif

/*
 * get irq priority.
 *
 * @irq         irq number.
 * @return      irq priority.
 */
#ifdef CONFIG_IRQ
int irq_get_priority(uint32_t irq);
#else
#define irq_get_priority(irq)
#endif

/*
 * set irq trigger mode.
 *
 * @irq         irq number.
 * @mode        irq trigger mode.
 */
#ifdef CONFIG_IRQ
void irq_set_trigger_mode(uint32_t irq, uint8_t mode);
#else
#define irq_set_trigger_mode(irq, mode)
#endif

/*
 * interrupt get current int priority.
 *
 * @return      int priority.
 */
#ifdef CONFIG_IRQ
uint32_t irq_get_current_priority(void);
#else
#define irq_get_current_priority()
#endif

/*
 * irq save.
 *
 * @return  old irq state.
 */
#ifdef CONFIG_IRQ
#define IRQ_SAVE                \
    irq_state_t irq_flags;      \
    irq_flags = arch_irq_save();
#else
#define IRQ_SAVE
#endif

/*
 * irq restore.
 *
 * @flags   old irq state.
 */
#ifdef CONFIG_IRQ
#define IRQ_RESTORE             \
    arch_irq_restore(irq_flags);
#else
#define IRQ_RESTORE
#endif

#if defined(CONFIG_ARCH_WITH_SMP) && defined(CONFIG_ARM_GIC)
void smp_gic_cpu_init(void);
void smp_cross_call(uint32_t cpumask, uint32_t ipinr);
void gic_set_affinity(uint32_t cpumask, uint32_t irq);
#else
#define smp_cross_call(cpumask, ipinr) do {} while(0);
#endif

__END_CDECLS

#endif

#endif
