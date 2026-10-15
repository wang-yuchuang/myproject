/*
 * arm_gic.h
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: ARMV7R gic interface.
 *
 * Revision History:
 * -----------------
 */

#ifndef DRV_LLD_ARM_GIC_H
#define DRV_LLD_ARM_GIC_H

#define DEFAULT_IRQ_PRIORITY         30

#ifndef ASSEMBLY

/* trigger mode */
#define GIC_LEVEL_SENSITIVE         0
#define GIC_EDGE_TRIGGERED          1

/*
 * interrupt lld initialize.
 */
void irq_lld_initialize(uint32_t base, uint32_t intr_num);

/*
 * interrupt set lld mask.
 *
 * @mask     int mask.
 * @return   old int mask.
 */
uint32_t irq_lld_mask(uint32_t mask);

/*
 * interrupt lld unmask all int.
 */
void irq_lld_unmask(void);

/*
 * interrupt lld int enable.
 *
 * @vector   int id.
 */
void irq_lld_enable(uint32_t vector);

/*
 * interrupt lld int disable.
 *
 * @vector   int id.
 */
void irq_lld_disable(uint32_t vector);

/*
 * interrupt lld set trigger mode.
 *
 * @ vector         int id
 * @ trigger_mode   int trigger mode
 */
void irq_lld_set_trigger_mode(uint32_t vector, uint8_t trigger_mode);

/*
 * interrupt lld set int priority.
 *
 * @vector      int id.
 * @priority    int priority.
 */
void irq_lld_set_priority(uint32_t vector, uint32_t priority);

/*
 * interrupt lld get int priority.
 *
 * @vector      int id.
 * @return      int priority.
 */
int irq_lld_get_priority(uint32_t vector);

/*
 * interrupt lld get current int priority.
 *
 * @vector      int id.
 * @return      int priority.
 */
uint32_t irq_lld_get_current_priority(void);

/*
 * interrupt lld get signaled interrupt id.
 *
 * @ return     signaled id
 */
uint32_t OS_CRITICAL_CODE irq_lld_acknowledge(void);

/*
 * interrupt lld complete signaled interrupt id.
 *
 * @ vector     signaled id
 */
void irq_lld_complete(uint32_t vector);

#if defined(CONFIG_ARCH_WITH_SMP)
void irq_lld_cpu_init(void);
void irq_lld_raise_ipi(uint32_t cpumask, uint32_t ipinr);
void irq_lld_set_affinity(uint32_t cpumask, uint32_t irq);
#endif

#endif

#endif
