/*
 * sd_vic.h
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: SemiDrive VIC header file.
 *
 * Revision History:
 * -----------------
 */

#ifndef SDRV_VIC_H_
#define SDRV_VIC_H_

#define DEFAULT_IRQ_PRIORITY    10U

#define irq_lld_initialize  sdrv_vic_lld_init
#define irq_lld_set_priority    sdrv_vic_lld_set_priority
#define irq_lld_disable sdrv_vic_lld_int_disable
#define irq_lld_enable  sdrv_vic_lld_int_enable
#ifdef CONFIG_VIC_INT_NEST_ERRATA
#define irq_lld_mask(x)    sdrv_vic_lld_mask_low_priority(x, true)
#else
#define irq_lld_mask    sdrv_vic_lld_mask_low_priority
#endif
#define irq_lld_get_priority    sdrv_vic_lld_get_priority
#define irq_lld_get_current_priority    sdrv_vic_lld_current_running_priority
#ifdef CONFIG_VIC_1_0
#define irq_lld_unmask  sdrv_vic_lld_unmask_all_interrupt
#else
#define irq_lld_unmask  sdrv_vic_lld_unmask_all_priority
#endif
#define irq_lld_acknowledge sdrv_vic_lld_ack

#ifdef CONFIG_VIC_IRQ_INTERRUPT_MODE
#define irq_lld_complete(irq)
#else
#define irq_lld_complete(irq)   sdrv_vic_lld_eoi(irq)
#endif

/* Not supported interface by VIC. */
#define irq_lld_set_trigger_mode(irq, mode) (void)mode

#ifndef ASSEMBLY

#include <compiler.h>
#include <stdint.h>

__BEGIN_CDECLS

#ifdef CONFIG_VIC_INT_NEST_ERRATA
uint32_t sdrv_vic_lld_mask_low_priority(uint32_t pri_threshold, bool user);
#else
uint32_t sdrv_vic_lld_mask_low_priority(uint32_t pri_threshold);
#endif
#ifdef CONFIG_VIC_1_0
void sdrv_vic_lld_unmask_all_interrupt(void);
#else
void sdrv_vic_lld_unmask_all_priority(void);
#endif
void sdrv_vic_lld_init(uint32_t base, uint32_t intr_num, int (*isr)(uint32_t));
void sdrv_vic_lld_set_priority(uint32_t vector,
                               uint32_t pri);
void sdrv_vic_lld_int_enable(uint32_t vector);
void sdrv_vic_lld_int_disable(uint32_t vector);
uint32_t sdrv_vic_lld_get_priority(uint32_t vector);
uint32_t sdrv_vic_lld_current_running_priority(void);
void sdrv_vic_lld_set_fiq_req_src(uint32_t vector);
uint32_t sdrv_vic_lld_get_active_fiq_src(void);
uint32_t sdrv_vic_lld_ack(void);
#ifndef CONFIG_VIC_IRQ_INTERRUPT_MODE
void sdrv_vic_lld_eoi(uint32_t vector);
#endif
void sdrv_vic_lld_trigger_soft_int(uint32_t vector);
void sdrv_vic_lld_clear_soft_int(uint32_t vector);

__END_CDECLS

#endif // !ASSEMBLY

#endif // !SDRV_VIC_H_