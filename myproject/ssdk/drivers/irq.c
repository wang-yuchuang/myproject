/*
 * interrupt.c
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: interrupt driver.
 *
 * Revision History:
 * -----------------
 */

#include <types.h>

#include "arch/cache.h"
#include "core_id.h"
#include "driver/irq.h"
#ifdef CONFIG_ARM_GIC
#include "driver/kunlun/gic/arm_gic.h"
#endif
#ifdef CONFIG_SDRV_VIC
#include "driver/taishan/vic/sdrv_vic.h"
#endif
#if defined(CONFIG_ARCH_WITH_SMP) && defined(CONFIG_ARM_GIC)
#include <arch/smp.h>
#endif

#include "irq_num.h"

/* irq info type */
struct irq_info_s {
  irq_handler handler;      /* Address of the interrupt handler */
  void *arg;                /* The argument provided to the interrupt handler. */
};

struct irq_table {
    struct irq_info_s irq_info[IRQ_MAX_INTR_NUM];
} __CACHE_ALIGN;

/* int table */
static struct irq_table g_irq_table[CORE_NUM] OS_CRITICAL_DATA;

static int irq_handle(uint32_t irq)
{
    if (irq < IRQ_MAX_INTR_NUM) {
        int core_id = get_core_id();
        struct irq_info_s *irq_info = &g_irq_table[core_id].irq_info[irq];
        if (irq_info->handler) {
            return irq_info->handler(irq, irq_info->arg);
        }
    }

    return -1;
}

/*
 * interrupt initialize.
 */
void irq_initialize(uint32_t base, uint32_t intr_num)
{
#ifdef CONFIG_ARM_GIC
    irq_lld_initialize(base, intr_num);
#endif
#ifdef CONFIG_SDRV_VIC
    irq_lld_initialize(base, intr_num, irq_handle);
#endif
    arch_irq_enable();
}

/*
 * interrupt attach.
 */
int irq_attach(uint32_t irq, irq_handler handler, void *arg)
{
    int ret = -1;

    if (irq < IRQ_MAX_INTR_NUM) {
        int core_id = get_core_id();
        struct irq_info_s *irq_info = &g_irq_table[core_id].irq_info[irq];

        IRQ_SAVE

        irq_info->handler = handler;
        irq_info->arg = arg;

        irq_lld_set_priority(irq, DEFAULT_IRQ_PRIORITY);

        IRQ_RESTORE

        ret = 0;
    }

    return ret;
}

/*
 * interrupt detach.
 */
int irq_detach(uint32_t irq)
{
    int ret = -1;

    if (irq < IRQ_MAX_INTR_NUM) {
        int core_id = get_core_id();
        struct irq_info_s *irq_info = &g_irq_table[core_id].irq_info[irq];

        IRQ_SAVE

        irq_lld_disable(irq);

        irq_info->handler = NULL;
        irq_info->arg = NULL;

        IRQ_RESTORE

        ret = 0;
    }

    return ret;
}

/*
 * interrupt dispatch.
 */
int irq_dispatch(void)
{
    int ret = -1;

    uint32_t irq = irq_lld_acknowledge();

    /* We should don't handle the interrupts
     * which hardware don't support and spurious interrupt
     */
    if (irq < IRQ_MAX_INTR_NUM)
        ret = irq_handle(irq);
    else
        ret = 0;

    irq_lld_complete(irq);

    return ret;
}

/*
 * irq enable.
 *
 * @irq   irq number.
 */
void irq_enable(uint32_t irq)
{
    irq_lld_enable(irq);
}

/*
 * irq disable.
 *
 * @irq   irq number.
 */
void irq_disable(uint32_t irq)
{
    irq_lld_disable(irq);
}

/*
 * interrupt set priority mask.
 *
 * @mask     int priority mask.
 * @return   old int priority mask.
 */
uint32_t irq_mask(uint32_t mask)
{
    return irq_lld_mask(mask);
}

/*
 * interrupt unmask all int.
 */
void irq_unmask(void)
{
    irq_lld_unmask();
}

/*
 * set irq priority.
 *
 * @irq         irq number.
 * @priority    irq priority.
 */
void irq_set_priority(uint32_t irq, uint32_t priority)
{
    irq_lld_set_priority(irq, priority);
}

/*
 * get irq priority.
 *
 * @irq         irq number.
 * @return      irq priority.
 */
int irq_get_priority(uint32_t irq)
{
    return irq_lld_get_priority(irq);
}

/*
 * set irq trigger mode.
 *
 * @irq         irq number.
 * @mode        irq trigger mode.
 */
void irq_set_trigger_mode(uint32_t irq, uint8_t mode)
{
    uint8_t lld_mode;

#ifdef CONFIG_ARM_GIC
    if (mode == TRIGGER_MODE_LEVEL) {
        lld_mode = GIC_LEVEL_SENSITIVE;
    }
    else if (mode == TRIGGER_MODE_EDGE) {
        lld_mode = GIC_EDGE_TRIGGERED;
    }
    else {
        return;
    }
#endif
    irq_lld_set_trigger_mode(irq, lld_mode);
}

/*
 * interrupt get current int priority.
 *
 * @return      int priority.
 */
uint32_t irq_get_current_priority(void)
{
    return irq_lld_get_current_priority();
}

#if defined(CONFIG_ARCH_WITH_SMP) && defined(CONFIG_ARM_GIC)
void smp_gic_cpu_init()
{
	irq_lld_cpu_init();
}

void smp_cross_call(uint32_t cpumask, uint32_t ipinr)
{
	uint32_t gic_ipi_num = ipinr + GIC_IPI_BASE;

	IRQ_SAVE
	irq_lld_raise_ipi(cpumask, gic_ipi_num);
	IRQ_RESTORE
}

void gic_set_affinity(uint32_t cpumask, uint32_t irq)
{
	if (irq >= IRQ_MAX_INTR_NUM)
		return;

	IRQ_SAVE
	irq_lld_set_affinity(cpumask, irq);
	IRQ_RESTORE
}
#endif
