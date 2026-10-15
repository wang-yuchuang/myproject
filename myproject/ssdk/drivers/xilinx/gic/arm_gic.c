/*
 * arm_gic.c
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: Cortex V7R GIC driver.
 *
 * Revision History:
 * -----------------
 */

#include <types.h>

#include "reg.h"
#include "driver/xilinx/gic/arm_gic.h"

#define GIC_OFFSET4(n)              ((n >> 2) << 2)
#define GIC_SHIFT4(n)               (((n) & 3) << 3)
#define GIC_MASK4(n)                (0xff << GIC_SHIFT4(n))

#define GIC_OFFSET16(n)             ((n >> 4) << 2)
#define GIC_SHIFT16(n)              (((n) & 15) << 1)
#define GIC_MASK16(n)               (3 << GIC_SHIFT16(n))

#define GIC_OFFSET32(n)             ((n >> 5) << 2)
#define GIC_SHIFT32(n)              ((n) & 31)
#define GIC_MASK32(n)               (1U << GIC_SHIFT32(n))

/* GIC regs offset */
#define GICC_OFFSET                 (g_gic_base + 0x2000)
#define GICD_OFFSET                 (g_gic_base + 0x1000)

/* Interrupt Interface registers */
#define GICC_CTLR                   (GICC_OFFSET + 0x0000)
#define GICC_PMR                    (GICC_OFFSET + 0x0004)
#define GICC_BPR                    (GICC_OFFSET + 0x0008)
#define GICC_IAR                    (GICC_OFFSET + 0x000c)
#define GICC_EOIR                   (GICC_OFFSET + 0x0010)
#define GICC_RPR                    (GICC_OFFSET + 0x0014)
#define GICC_HPPIR                  (GICC_OFFSET + 0x0018)
#define GICC_ABPR                   (GICC_OFFSET + 0x001c)
#define GICC_IIDR                   (GICC_OFFSET + 0x00fc)
#define GICC_DIR                    (GICC_OFFSET + 0x1000)

/* Distributor Registers */
#define GICD_CTLR                   (GICD_OFFSET + 0x0000)
#define GICD_TYPER                  (GICD_OFFSET + 0x0004)
#define GICD_IIDR                   (GICD_OFFSET + 0x0008)
#define GICD_IGROUPR(n)             (GICD_OFFSET + (0x0080 + GIC_OFFSET32(n)))
#define GICD_ISENABLER(n)           (GICD_OFFSET + (0x0100 + GIC_OFFSET32(n)))
#define GICD_ICENABLER(n)           (GICD_OFFSET + (0x0180 + GIC_OFFSET32(n)))
#define GICD_ISPENDR(n)             (GICD_OFFSET + (0x0200 + GIC_OFFSET32(n)))
#define GICD_ICPENDR(n)             (GICD_OFFSET + (0x0280 + GIC_OFFSET32(n)))
#define GICD_ISACTIVER(n)           (GICD_OFFSET + (0x0300 + GIC_OFFSET32(n)))
#define GICD_ICACTIVER(n)           (GICD_OFFSET + (0x0380 + GIC_OFFSET32(n)))
#define GICD_IPRIORITYR(n)          (GICD_OFFSET + (0x0400 + GIC_OFFSET4(n)))
#define GICD_ITARGETSR(n)           (GICD_OFFSET + (0x0800 + GIC_OFFSET4(n)))
#define GICD_ICFGR(n)               (GICD_OFFSET + (0x0c00 + GIC_OFFSET16(n)))
#define GICD_NSACR(n)               (GICD_OFFSET + (0x0e00 + GIC_OFFSET16(n)))
#define GICD_SGIR                   (GICD_OFFSET + 0x0f00)
#define GICD_CPENDSGIR(n)           (GICD_OFFSET + (0x0f10 + GIC_OFFSET4(n)))
#define GICD_SPENDSGIR(n)           (GICD_OFFSET + (0x0f20 + GIC_OFFSET4(n)))

/* interrupt priority number */
static uint8_t g_priority_shift = 1;

/* interrupt base */
static uint32_t g_gic_base;

/* interrupt base */
static uint32_t g_gic_intr_num;

/*
 * gic get priority bit implemented.
 */
static uint8_t arm_gic_get_priority_shift(uint32_t vector)
{
    uint32_t regaddr, regval, testval;
    uint32_t priority;
    uint8_t shift = 0;

    regaddr = GICD_IPRIORITYR(vector);

    regval = readl(regaddr);

    irq_lld_disable(vector);

    testval = (regval & ~GIC_MASK4(vector)) | (0xff << GIC_SHIFT4(vector));
    writel(testval, regaddr);

    priority = (readl(regaddr) & GIC_MASK4(vector)) >> GIC_SHIFT4(vector);

    writel(regval, regaddr);

    irq_lld_enable(vector);

    while ((priority & 0x1) != 0x1) {
        shift++;
        priority >>= 1;
    }

    return shift;
}

/*
 * gic initialize.
 */
void irq_lld_initialize(uint32_t base, uint32_t intr_num)
{
    uint32_t i;

    g_gic_base = base;
    g_gic_intr_num = intr_num;

    /* enable signaling of interrupts for Group0 */
    writel(1, GICD_CTLR);

    /* disable int */
    for (i = 0; i < intr_num; i += 32) {
        writel(0xffffffff, GICD_ICENABLER(i));
        writel(0xffffffff, GICD_ICPENDR(i));
    }

    /* set SPI cpu to cpu 0 and Non-secure view */
    for (i = 32; i < intr_num; i += 4) {
        writel(0x80808080, GICD_IPRIORITYR(i));
        writel(0x01010101, GICD_ITARGETSR(i));
    }

    /* unmask PMR */
    irq_lld_unmask();

    /* enable GIC0 */
#ifdef CONFIG_INTERRUPT_NESTING
    writel((1 << 9) + 1, GICC_CTLR);
#else
    writel(1, GICC_CTLR);
#endif

    g_priority_shift = arm_gic_get_priority_shift(32);
}

/*
 * interrupt set lld mask.
 */
uint32_t irq_lld_mask(uint32_t mask)
{
    uint32_t old_mask = (readl(GICC_PMR) >> g_priority_shift);

    writel((mask << g_priority_shift), GICC_PMR);

    return old_mask;
}

/*
 * interrupt lld unmask all int.
 */
void irq_lld_unmask(void)
{
    writel(0xff, GICC_PMR);
}

/*
 * gic set trigger mode.
 */
void irq_lld_set_trigger_mode(uint32_t vector, uint8_t trigger_mode)
{
    uint32_t regaddr, regval;

    if (vector < g_gic_intr_num) {
        regaddr = GICD_ICFGR(vector);
        regval = readl(regaddr);
        regval &= ~GIC_MASK16(vector);
        regval |= (trigger_mode << GIC_SHIFT16(vector));
        writel(regval, regaddr);
    }
}

/*
 * gic set priority.
 */
void irq_lld_set_priority(uint32_t vector, uint32_t priority)
{
    uint32_t regaddr, regval;

    if (vector < g_gic_intr_num) {
        regaddr = GICD_IPRIORITYR(vector);
        regval = readl(regaddr);
        regval &= ~GIC_MASK4(vector);
        priority <<= g_priority_shift;
        regval |= (priority << GIC_SHIFT4(vector));
        writel(regval, regaddr);
    }
}

/*
 * gic get priority.
 */
int irq_lld_get_priority(uint32_t vector)
{
    uint32_t regaddr, regval;
    uint8_t priority;

    if (vector < g_gic_intr_num) {
        regaddr = GICD_IPRIORITYR(vector);
        regval = readl(regaddr);
        regval &= GIC_MASK4(vector);
        priority = (uint8_t)(regval >> GIC_SHIFT4(vector));
        priority >>= g_priority_shift;
        return priority;
    }
    return -1;
}

/*
 * interrupt lld get current int priority.
 */
uint32_t irq_lld_get_current_priority(void)
{
    return (uint32_t)(readl(GICC_RPR) >> g_priority_shift);
}

/*
 * gic set enable.
 */
void irq_lld_enable(uint32_t vector)
{
    if (vector < g_gic_intr_num) {
        writel(GIC_MASK32(vector), GICD_ISENABLER(vector));
    }
}

/*
 * gic set disable.
 */
void irq_lld_disable(uint32_t vector)
{
    if (vector < g_gic_intr_num) {
        writel(GIC_MASK32(vector), GICD_ICENABLER(vector));
    }
}

/*
 * gic get signaled interrupt id.
 */
uint32_t irq_lld_acknowledge(void)
{
    uint32_t vector = readl(GICC_IAR);

#ifdef CONFIG_INTERRUPT_NESTING
    writel(vector, GICC_EOIR);
#endif

    return vector;
}

/*
 * gic complete signaled interrupt id.
 */
void irq_lld_complete(uint32_t vector)
{
#ifdef CONFIG_INTERRUPT_NESTING
    writel(vector, GICC_DIR);
#else
    writel(vector, GICC_EOIR);
#endif
}
