/*
 * sd_vic.c
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: SemiDrive VIC driver.
 *
 * Revision History:
 * -----------------
 */

#include <string.h>
#include "compiler.h"
#include "param.h"
#include "reg.h"
#include "arch/cache.h"
#include "arch/irq.h"
#include "bits.h"
#include "core_id.h"
#include "driver/taishan/vic/sdrv_vic.h"

#define OFFSET32(v)  (((v) >> 5) << 2)
#define SHIFT32(v)  ((v) & 31U)

#define VICINTENABLE(v)         (g_vic_data[core_id].base + 0x100U + OFFSET32(v))
#define VICINTENCLEAR(v)        (g_vic_data[core_id].base + 0x1C0U + OFFSET32(v))
#define VICSWMASK(v)            (g_vic_data[core_id].base + 0x180U + OFFSET32(v))
#define VICVECTPRIORITY(v)      (g_vic_data[core_id].base + 0x400U + ((v) << 2))
#define VIC_WDT_EN              (g_vic_data[core_id].base + 0x380U)
#define VIC_WDT_TH              (g_vic_data[core_id].base + 0x3C0U)
#define VIC_ERRINT_CLR          (g_vic_data[core_id].base + 0x3CCU)
#define VIC_SFINT_MASK          (g_vic_data[core_id].base + 0x1F00U)
#define VICVECADDR(v)           (g_vic_data[core_id].base + 0xF000U + ((v) << 2))
#define VICADDRESS              (g_vic_data[core_id].base + 0xF00U)
#define VICINTSELECT(v)         (g_vic_data[core_id].base + 0xC0U + OFFSET32(v))
#define VICSOFTINT(v)           (g_vic_data[core_id].base + 0x140U + OFFSET32(v))
#define VICSOFTINTCLEAR(v)      (g_vic_data[core_id].base + 0x200U + OFFSET32(v))
#define VICFIQSTATUS(v)         (g_vic_data[core_id].base + 0x40U + OFFSET32(v))
#define VICSWPRIORITYMASK       (g_vic_data[core_id].base + 0x3C8U)
#define VICSWMASK_REG(n)        (g_vic_data[core_id].base + 0x180U + ((n) << 2))

#define MAX_PRI 15

#define SWMASK_REG_NUM  8

/* Timeout configuration */
#define WDT_EN  0U
#define WDT_DIV 1U
#define WDT_TH  0x80U
#define MAKE_WDT_TH(en, div, th)    (((en) << 31) | (((div) & 0x3FFU) << 16) | \
                                     ((th) & 0xFFU))

struct vic_data {
    uint32_t base;
    uint32_t int_num;
    uint32_t cur_rp;
    uint32_t irq_nest_cnt;
    uint32_t pri_masked;

#ifndef CONFIG_VIC_IRQ_INTERRUPT_MODE
    #ifndef CONFIG_VIC_INT_NEST_MAX_CNT
    #define CONFIG_VIC_INT_NEST_MAX_CNT 16
    #endif

    struct stack {
        uint32_t buf[CONFIG_VIC_INT_NEST_MAX_CNT];
        uint32_t head;
    } old_pri;
#endif

#ifdef CONFIG_VIC_1_0
    struct sw_mask {
        /* Bitmap that record registered interrupt of each priority
         * and each sw mask register value.
         */
        struct pri_mask {
            /* Is each priority masked?
            * Each priority is arranged linealy to get higher access performance.
            */
            bool masked[MAX_PRI + 1] __CACHE_ALIGN;
            /* Registered interrupt of each priority. */
            unsigned long int_mask[MAX_PRI + 1][SWMASK_REG_NUM] __CACHE_ALIGN;
            /* Registerd interrupt which has a equal or lower priority than
            * each given priority specified by array index.
            */
            unsigned long low_pri_mask[MAX_PRI + 1][SWMASK_REG_NUM] __CACHE_ALIGN;
        } pmsk;
        uint32_t cur_sw_mask[SWMASK_REG_NUM] __CACHE_ALIGN;
    } swmsk;

#ifdef CONFIG_VIC_INT_NEST_ERRATA
    uint32_t cur_sw_mask;
#endif
#endif
} __CACHE_ALIGN;

static struct vic_data g_vic_data[CORE_NUM];

static inline void sdrv_vic_lld_int_disable_all(void)
{
    int core_id = get_core_id();
    uint32_t int_num = g_vic_data[core_id].int_num;

    for (uint32_t i = 0U; i < int_num; i += 32U) {
        writel(0xFFFFFFFFU, VICINTENCLEAR(i));
    }
}

static void sdrv_vic_lld_func_safety_en(void)
{
    int core_id = get_core_id();
    /* Enable timeout dection. */
    writel(MAKE_WDT_TH(WDT_EN, WDT_DIV, WDT_TH), VIC_WDT_TH);
    writel(WDT_EN, VIC_WDT_EN);

    /* Unmask error interrupts. */
    writel(0U, VIC_SFINT_MASK);
}

static void sdrv_vic_lld_all_ints_trigger_irq(void)
{
    int core_id = get_core_id();
    uint32_t int_num = g_vic_data[core_id].int_num;

    for (uint32_t i = 0U; i < int_num; i += 32U) {
        writel(0U, VICINTSELECT(i));
    }
}

static void sdrv_vic_lld_set_vector(void)
{
    int core_id = get_core_id();
    uint32_t int_num = g_vic_data[core_id].int_num;

#if defined(CONFIG_VIC_IRQ_INTERRUPT_MODE)
    extern uint8_t vectored_irq0_handler, vectored_irq1_handler;

    uint32_t vector_start_addr = (uint32_t)&vectored_irq0_handler;
    uint32_t each_vector_size = (uint32_t)(&vectored_irq1_handler -
                                           &vectored_irq0_handler);

    for (uint32_t i = 0U; i < int_num; i++) {
        writel(vector_start_addr + i * each_vector_size,
               VICVECADDR(i));
    }
#else
    for (uint32_t i = 0U; i < int_num; i++) {
        writel(i, VICVECADDR(i));
    }
#endif
}

#if defined(CONFIG_VIC_IRQ_INTERRUPT_MODE)
static int sdrv_vic_dummy_isr(uint32_t vector)
{
    return -1;
}

/* IRQ handler may be allowed not to use per cpu variable?
 * Because this variable value is the same on all cores.
 */
int (*g_vic_user_isr)(uint32_t vector) = sdrv_vic_dummy_isr;

static inline void sdrv_vic_lld_set_user_isr(int (*isr)(uint32_t))
{
    if (isr) {
        g_vic_user_isr = isr;
    }
}
#else
static inline void sdrv_vic_inc_int_nest_cnt(void)
{
    g_vic_data[get_core_id()].irq_nest_cnt++;
}

static inline int sdrv_vic_dec_int_nest_cnt(void)
{
    return (--g_vic_data[get_core_id()].irq_nest_cnt);
}
#endif

static inline void sdrv_vic_lld_clr_int_nest_cnt(void)
{
    int core_id = get_core_id();
    g_vic_data[core_id].irq_nest_cnt = 0U;
}

static void sdrv_vic_lld_unmask_interrupt(uint32_t vector)
{
    int core_id = get_core_id();
    RMWREG32(VICSWMASK(vector), SHIFT32(vector), 1U, 0U);
}

/**
 * @brief Mask specified interrupt
 * @param vector priority to be masked
 */
static void sdrv_vic_lld_mask_interrupt(uint32_t vector)
{
    int core_id = get_core_id();
    RMWREG32(VICSWMASK(vector), SHIFT32(vector), 1U, 1U);
}

#ifndef CONFIG_VIC_1_0
static int local_clz(int x)
{
#if __GNUC__ || __ICCARM__
    if (x)
        return __builtin_clz(x);
    else
        return 32;
#else
    unsigned y;
    int n = 32;
    y = x >> 16;
    if (y != 0) {
        n = n - 16;
        x = y;
    }
    y = x >> 8;
    if (y != 0) {
        n = n - 8;
        x = y;
    }
    y = x >> 4;
    if (y != 0) {
        n = n - 4;
        x = y;
    }
    y = x >> 2;
    if (y != 0) {
        n = n - 2;
        x = y;
    }
    y = x >> 1;
    if (y != 0)
        return n - 2;
    return n - x;
#endif
}
#endif

/**
 * @brief Mask all low priority level
 * @param pri_threshold interrupts with priority equals
 *        to or lower than pri_threshold are masked.
 * @param user if this api called by user or interrupt
 *        framework.
 * @return Previous masked priority value
 */
#ifdef CONFIG_VIC_INT_NEST_ERRATA
uint32_t sdrv_vic_lld_mask_low_priority(uint32_t pri_threshold, bool user)
#else
uint32_t sdrv_vic_lld_mask_low_priority(uint32_t pri_threshold)
#endif
{
    int core_id = get_core_id();
#ifdef CONFIG_VIC_1_0
    static uint32_t old_mask = ~0U;
    uint32_t ret = old_mask;
    uint32_t swmask[SWMASK_REG_NUM] = {0};
    struct vic_data *vicdata = &g_vic_data[core_id];
    struct pri_mask *pmsk = &vicdata->swmsk.pmsk;
    uint32_t *curmask = vicdata->swmsk.cur_sw_mask;

    pri_threshold = MIN(pri_threshold, MAX_PRI + 1);

    irq_state_t state = arch_irq_save();
#ifdef CONFIG_VIC_INT_NEST_ERRATA
    if (user)
#endif
        old_mask = pri_threshold;

#ifdef CONFIG_VIC_INT_NEST_ERRATA
    /* If called by user, update current masked priority. */
    if (user)
        vicdata->cur_sw_mask = pri_threshold;

    /* When user sets priority mask, always no unmasking any
     * priority masked by interrupt framework.
     */
    if (user && (vicdata->cur_rp < pri_threshold))
        pri_threshold = vicdata->cur_rp;
    /* When interrupt framework sets priority mask, always no
     * unmasking any priority masked by user.
     */
    else if (!user && (vicdata->cur_sw_mask < pri_threshold))
        pri_threshold = vicdata->cur_sw_mask;
#endif

    /* Larger priority value, lower priority. */
    for (int i = 0; i <= MAX_PRI; i++) {
        /* Update mask value of each priroity. */
        if (i < pri_threshold)
            pmsk->masked[i] = false;
        else
            pmsk->masked[i] = true;
    }
    if (pri_threshold <= MAX_PRI) {
        /* Update swmask register value to low pri masked bitmap. */
        memcpy(swmask, pmsk->low_pri_mask[pri_threshold], sizeof(swmask));
    }

    for (int i = 0; i < SWMASK_REG_NUM; i++) {
        if (curmask[i] != swmask[i]) {
            writel(swmask[i], VICSWMASK_REG(i));
            curmask[i] = swmask[i];
        }
    }

    vicdata->pri_masked = pri_threshold;

out:
    arch_irq_restore(state);
#else
    uint32_t ret;

    irq_state_t state = arch_irq_save();

    ret = readl(VICSWPRIORITYMASK);
    if (ret != ~0) {
        ret &= 0xFFFF;
        ret = 32 - local_clz(ret);
    }

    pri_threshold = MIN(pri_threshold, MAX_PRI + 1);
    uint16_t temp = 0xFFFF;
    temp = temp >> (16 - pri_threshold);
    writel(temp, VICSWPRIORITYMASK);

    g_vic_data[core_id].pri_masked = pri_threshold;

    arch_irq_restore(state);
#endif

    return ret;
}

/**
 * @brief Unmask all interrupt
 */
void sdrv_vic_lld_unmask_all_interrupt(void)
{
#ifdef CONFIG_VIC_1_0
#ifdef CONFIG_VIC_INT_NEST_ERRATA
    sdrv_vic_lld_mask_low_priority(0xFF, true);
#else
    sdrv_vic_lld_mask_low_priority(0xFF);
#endif
#else
    int core_id = get_core_id();
    irq_state_t state = arch_irq_save();

    for (int i = 0; i < SWMASK_REG_NUM; i++)
        writel(0, VICSWMASK_REG(i));

    arch_irq_restore(state);
#endif
}

#ifndef CONFIG_VIC_1_0
/**
 * @brief Unmask all priority
 */
void sdrv_vic_lld_unmask_all_priority(void)
{
    int core_id = get_core_id();
    irq_state_t state = arch_irq_save();
    writel(~0, VICSWPRIORITYMASK);
    g_vic_data[core_id].pri_masked = 0xFF;
    arch_irq_restore(state);
}
#endif

/**
 * @brief Initialize VIC
 * @param isr user ISR
 */
void sdrv_vic_lld_init(uint32_t base, uint32_t intr_num, int (*isr)(uint32_t))
{
    int core_id = get_core_id();

    g_vic_data[core_id].base = base;
    g_vic_data[core_id].int_num = intr_num;
    g_vic_data[core_id].cur_rp = 0xFF;
    g_vic_data[core_id].pri_masked = 0xFF;
#ifdef CONFIG_VIC_1_0
    memset(g_vic_data[core_id].swmsk.cur_sw_mask, 0xFF,
           sizeof(g_vic_data[core_id].swmsk.cur_sw_mask));
#endif

    sdrv_vic_lld_int_disable_all();
    sdrv_vic_lld_all_ints_trigger_irq();
    sdrv_vic_lld_unmask_all_interrupt();
#ifndef CONFIG_VIC_1_0
    sdrv_vic_lld_unmask_all_priority();
#endif

    sdrv_vic_lld_func_safety_en();

    sdrv_vic_lld_set_vector();

#if defined(CONFIG_VIC_IRQ_INTERRUPT_MODE)
    sdrv_vic_lld_set_user_isr(isr);
    arch_vectored_irq_enable(1U);
#else
    arch_vectored_irq_enable(0U);
#endif

    sdrv_vic_lld_clr_int_nest_cnt();
}

#ifdef CONFIG_VIC_1_0
static inline void
sdrv_vic_lld_clear_int_mask(uint32_t vector)
{
    int core_id = get_core_id();
    struct sw_mask *swmsk = &g_vic_data[core_id].swmsk;

    for (int i = 0; i <= MAX_PRI; i++) {
        unsigned long *intmask = swmsk->pmsk.int_mask[i];
        unsigned long *low_pir_mask = swmsk->pmsk.low_pri_mask[i];
        if (bitmap_test(intmask, vector))
            bitmap_clear(intmask, vector);
        if (bitmap_test(low_pir_mask, vector))
            bitmap_clear(low_pir_mask, vector);
    }
}
#endif

/**
 * @brief Set priority
 */
void sdrv_vic_lld_set_priority(uint32_t vector,
                               uint32_t pri)
{
    int core_id = get_core_id();
    irq_state_t state = arch_irq_save();
    writel(pri & 15U, VICVECTPRIORITY(vector));

#ifdef CONFIG_VIC_1_0
    struct pri_mask *pmsk = &g_vic_data[core_id].swmsk.pmsk;
    unsigned long *intmask = pmsk->int_mask[pri];

    /* In case of modifying priority dynamically, clear
     * previous int bitmap first.
     */
    sdrv_vic_lld_clear_int_mask(vector);
    bitmap_set(intmask, vector);
    /* If masking a equal or higher priority, this
     * vector should be masked. So, set this vector
     * in each low pir mask bitmap with a equal or
     * higher priority.
     */
    for (int i = 0; i <= pri; i++) {
        bitmap_set(pmsk->low_pri_mask[i], vector);
    }

    if (pmsk->masked[pri])
        sdrv_vic_lld_mask_interrupt(vector);
    else
        sdrv_vic_lld_unmask_interrupt(vector);
#endif
    arch_irq_restore(state);
}

/**
 * @brief Enable interrupt
 * @param vector interrupt vector number
 */
void sdrv_vic_lld_int_enable(uint32_t vector)
{
    int core_id = get_core_id();
    irq_state_t state = arch_irq_save();
    RMWREG32(VICINTENABLE(vector), SHIFT32(vector), 1U, 1U);
    arch_irq_restore(state);
}

/**
 * @brief Disable interrupt
 * @param vector interrupt vector number
 */
void sdrv_vic_lld_int_disable(uint32_t vector)
{
    int core_id = get_core_id();
    irq_state_t state = arch_irq_save();
    RMWREG32(VICINTENCLEAR(vector), SHIFT32(vector), 1U, 1U);
    arch_irq_restore(state);
}

/**
 * @brief Get priority
 * @param vector interrupt vector number
 * @return interrupt priority
 */
uint32_t sdrv_vic_lld_get_priority(uint32_t vector)
{
    int core_id = get_core_id();
    return readl(VICVECTPRIORITY(vector)) & 15U;
}

/**
 * @brief Get current running priority
 * @return uint32_t current running priority
 */
uint32_t sdrv_vic_lld_current_running_priority(void)
{
    return g_vic_data[get_core_id()].cur_rp;
}

/**
 * @brief Set one vector to trigger FIQ requeset
 * @param vector vector number
 */
void sdrv_vic_lld_set_fiq_req_src(uint32_t vector)
{
    int core_id = get_core_id();
    uint32_t int_num = g_vic_data[core_id].int_num;
    irq_state_t state = arch_irq_save();

    /* The system should have at most one FIQ source. */
    for (uint32_t i = 0U; i < int_num; i += 32U) {
        if ((i >> 5) == (vector >> 5)) {
            writel(1U << SHIFT32(vector), VICINTSELECT(vector));
        }
        else {
            writel(0U, VICINTSELECT(i));
        }
    }

    arch_irq_restore(state);
}

static inline int local_fls(int x)
{
    int r = 32;

    if (!x)
        return 0;
    if (!(x & 0xffff0000u)) {
        x <<= 16;
        r -= 16;
    }
    if (!(x & 0xff000000u)) {
        x <<= 8;
        r -= 8;
    }
    if (!(x & 0xf0000000u)) {
        x <<= 4;
        r -= 4;
    }
    if (!(x & 0xc0000000u)) {
        x <<= 2;
        r -= 2;
    }
    if (!(x & 0x80000000u)) {
        x <<= 1;
        r -= 1;
    }
    return r;
}

/**
 * @brief Get current serviced fiq vector number
 * @return uint32_t vector number
 */
uint32_t sdrv_vic_lld_get_active_fiq_src(void)
{
    int core_id = get_core_id();
    uint32_t int_num = g_vic_data[core_id].int_num;
    uint32_t i = 0U;
    uint32_t fiq_status = 0U;
    uint32_t ret = ~0;

    irq_state_t state = arch_irq_save();

    for (i = 0U; i < int_num; i += 32U) {
        fiq_status = readl(VICFIQSTATUS(i));
        if (fiq_status) {
            ret = i + local_fls(fiq_status) - 1;
            break;
        }
    }

    arch_irq_restore(state);
    return ret;
}

#ifndef CONFIG_VIC_IRQ_INTERRUPT_MODE
/**
 * No lock needed, because the folling function
 * is always called in IRQ disabled interrupt context.
 */
static void sdrv_vic_push_stack(struct stack *s, uint32_t val)
{
    uint32_t head = s->head;

    s->buf[head] = val;
    s->head = head + 1;
}

static uint32_t sdrv_vic_pop_stack(struct stack *s)
{
    uint32_t top = s->head - 1;
    uint32_t ret = s->buf[top];
    s->head = top;

    return ret;
}

static inline void sdrv_vic_push_old_pri(void)
{
    int core_id = get_core_id();
    struct stack *old_pri = &g_vic_data[core_id].old_pri;

    sdrv_vic_push_stack(old_pri, g_vic_data[core_id].cur_rp);
}

static inline void sdrv_vic_pop_old_pri(void)
{
    int core_id = get_core_id();
    struct stack *old_pri = &g_vic_data[core_id].old_pri;
    g_vic_data[core_id].cur_rp = sdrv_vic_pop_stack(old_pri);
}
#endif

/**
 * @brief Interrupt acknowledge
 * @return uint32_t interrupt number
 */
uint32_t sdrv_vic_lld_ack(void)
{
#ifndef CONFIG_VIC_IRQ_INTERRUPT_MODE
    int core_id = get_core_id();
    if (!arch_in_fiq_mode()) {
        sdrv_vic_inc_int_nest_cnt();

        uint32_t vector_num =  readl(VICADDRESS);

        /* Save previous running priority. */
        sdrv_vic_push_old_pri();
        /* Update current running priority. */
        uint32_t cur_rp = sdrv_vic_lld_get_priority(vector_num);
        g_vic_data[core_id].cur_rp = cur_rp;
        /* Save priority mask before this interrupt. */
        uint32_t pri_mask = g_vic_data[core_id].pri_masked;

    #ifdef CONFIG_VIC_INT_NEST_ERRATA
        /* Mask all interrupt with the same and lower priority
         * to prevent:
         * 1) same or lower priority interrupt preempting after
         *    writing VICADDRESS
         * 2) same sw priority but higher hw priority interrupt
         *    preempting
         */
        sdrv_vic_lld_mask_low_priority(cur_rp, false);

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
        (void)readl(VICADDRESS);
        /* A dummy write to VICADDRESS
         * finish the current interrupt.
         */
        writel(vector_num, VICADDRESS);
    #endif

    /* Make sure only unmasked interrupt can be taken. */
    if (cur_rp >= pri_mask)
        return g_vic_data[core_id].int_num;

    #ifdef CONFIG_VIC_INT_NEST_AUTO_ENABLE
        if (g_vic_data[core_id].irq_nest_cnt < CONFIG_VIC_INT_NEST_MAX_CNT)
            arch_irq_enable();
    #endif

        return vector_num;
    }
    else
#endif
    {
        return sdrv_vic_lld_get_active_fiq_src();
    }
}

#ifndef CONFIG_VIC_IRQ_INTERRUPT_MODE
/**
 * @brief Interrupt completed
 */
void sdrv_vic_lld_eoi(uint32_t vector)
{
    /* No need to write vicaddress register
     * in FIQ service routine.
     */
    if (!arch_in_fiq_mode()) {
        int core_id = get_core_id();
        arch_irq_disable();
    #ifndef CONFIG_VIC_INT_NEST_ERRATA
        /* A dummy write to VICADDRESS
        * finish the current interrupt.
        */
        writel(vector, VICADDRESS);
    #endif

        sdrv_vic_dec_int_nest_cnt();
        /* Restore previous running priority. */
        sdrv_vic_pop_old_pri();

    #ifdef CONFIG_VIC_INT_NEST_ERRATA
        /* Unmask priority to previous state. */
        sdrv_vic_lld_mask_low_priority(g_vic_data[core_id].cur_rp,
                                       false);
    #endif
    }
}
#endif

/**
 * @brief Trigger software interrupt
 * @param vector interrupt number
 */
void sdrv_vic_lld_trigger_soft_int(uint32_t vector)
{
    int core_id = get_core_id();
    irq_state_t state = arch_irq_save();
    writel(1U << SHIFT32(vector), VICSOFTINT(vector));
    arch_irq_restore(state);
}

/**
 * @brief Clear software interrupt
 * @param vector interrupt number
 */
void sdrv_vic_lld_clear_soft_int(uint32_t vector)
{
    int core_id = get_core_id();
    irq_state_t state = arch_irq_save();
    writel(1U << SHIFT32(vector), VICSOFTINTCLEAR(vector));
    arch_irq_restore(state);
}

void *get_vic_data_base(void)
{
    return &g_vic_data[get_core_id()];
}
