/**
 * @file dw_rtc.c
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: SDRV RTC low level driver.
 *
 * Revision History:
 * -----------------
 */
#include <bits.h>
#include <reg.h>
#include <types.h>
#include "dw_rtc.h"

/* RTC Control Register */
#define SEC 0x00
#define SEC_LOCAL_ENABLE 0 /* RTC enable. */
#define SEC_SECURE_ENABLE 1 /* secure enable. */
#define SEC_PRIBILEGE_ENABLE 2 /* privilege enable. */
#define SEC_LOCK                                                               \
    31 /* once lock bit is set, other enable bits can't be changed. */

#define RTC_H 0x04
#define RTC_L 0x08

/* RTC Auto Adjust Function Control Register */
#define AUTO_ADJUST 0x0c
#define AUTO_ADJUST_ENABLE 0 /* auto adjust enable. */
#define AUTO_ADJUST_ADJUST_DIR 1 /* auto adjust direction. */
#define AUTO_ADJUST_CYCLE 8 /* num of auto adjust cycles. */

#define TIMER_H 0x10
#define TIMER_L 0x14

/* Wake-up Control Register */
#define WAKEUP_CTRL 0x18
#define WAKEUP_CTRL_REQ_EN 0 /* req enable used in pmu wake up. */
#define WAKEUP_CTRL_IRQ_EN 1 /* generate IRQ used in RTC compare. */
#define WAKEUP_CTRL_ENABLE 2 /* wake up enable. */
#define WAKEUP_CTRL_STATUS                                                     \
    3 /* wake up status, this bit will be set when timer value arrive. */
#define WAKEUP_CTRL_CLEAR 4 /* write 1 to clear IRQ status. */
#define WAKEUP_CTRL_OVERFLOW 5 /* counter overflow violation clear. */
#define WAKEUP_CTRL_DISABLE 6 /* counter disable  violation clear. */

/* Periodical IRQ Control Register */
#define PERIODIC_CTRL 0x1c
#define PERIODIC_CTRL_IRQ_EN 0 /* period irq enable. */
#define PERIODIC_CTRL_FREQ 3 /* periodical irq frequence select. */

/* rtc sync tickets */
#define RTC_WRITE_SYNC_CLK 7

/**
 * @brief All APB writes to the RTC domain need to wait several RTC clocks
 *        for the write transaction to complete.
 *
 * @param base
 * @param rtc_clk
 */
static void dw_rtc_clk_sync(paddr_t base, uint32_t rtc_clk)
{
    uint64_t dw_rtc_point = dw_rtc_lld_get_tick(base) + rtc_clk;

    while (dw_rtc_lld_get_tick(base) <= dw_rtc_point)
        ;
}

/**
 * @brief read rtc register value
 *
 * @param base
 * @param reg
 * @return uint32_t
 */
static inline uint32_t dw_rtc_readl(paddr_t base, uint32_t reg)
{
    return readl(base + reg);
}

/**
 * @brief rtc write register value
 *
 * @param base
 * @param reg
 * @param value
 */
static inline void dw_rtc_writel(paddr_t base, uint32_t reg, uint32_t value)
{
    writel(value, base + reg);
    dw_rtc_clk_sync(base, RTC_WRITE_SYNC_CLK);
}

/**
 * @brief write rtc register whitout sync
 *
 * @param base
 * @param reg
 * @param value
 */
static inline void dw_rtc_writel_no_sync(paddr_t base, uint32_t reg,
        uint32_t value)
{
    writel(value, base + reg);
}

/**
 * @brief modify rtc register value
 *
 * @param base
 * @param reg
 * @param bit
 * @param width
 * @param val
 */
static inline void dw_rtc_modifyl(paddr_t base, uint32_t reg, uint32_t bit,
                                  uint32_t width, uint32_t val)
{
    uint32_t v = dw_rtc_readl(base, reg);

    v &= ~(BIT_MASK(width) << bit);
    v |= (val & BIT_MASK(width)) << bit;
    dw_rtc_writel(base, reg, v);
}

/**
 * @brief lock rtc
 *
 * @param base
 */
void dw_rtc_lld_lock(paddr_t base)
{
    dw_rtc_modifyl(base, SEC, SEC_LOCK, 1, 1);
}

/**
 * @brief unlock rtc
 *
 * @param base
 */
void dw_rtc_lld_unlock(paddr_t base)
{
    dw_rtc_modifyl(base, SEC, SEC_LOCK, 1, 0);
}

/**
 * @brief check rtc lock status
 *
 * @param base
 * @return true
 * @return false
 */
bool dw_rtc_lld_is_locked(paddr_t base)
{
    return !!(dw_rtc_readl(base, SEC) & (1 << SEC_LOCK));
}

/**
 * @brief clear rtc alarm status
 *
 * @param base
 */
void dw_rtc_lld_clear_alarm_status(paddr_t base)
{
    dw_rtc_modifyl(base, WAKEUP_CTRL, WAKEUP_CTRL_CLEAR, 1, 1);
    dw_rtc_modifyl(base, WAKEUP_CTRL, WAKEUP_CTRL_CLEAR, 1, 0);
}

/**
 * @brief enable rtc alarm
 *
 * @param base
 * @param en
 * @return int
 */
int dw_rtc_lld_set_alarm_enable(addr_t base, bool en)
{
    if (dw_rtc_lld_enable(base) < 0) {
        return -1;
    }

    dw_rtc_lld_clear_alarm_status(base);

    if (en) {
        /* set wake up enable */
        dw_rtc_modifyl(base, WAKEUP_CTRL, WAKEUP_CTRL_ENABLE, 1, 1);

        /* generate IRQ used in RTC compare */
        dw_rtc_modifyl(base, WAKEUP_CTRL, WAKEUP_CTRL_IRQ_EN, 1, 1);

        /* req enable used in pmu wake up */
        dw_rtc_modifyl(base, WAKEUP_CTRL, WAKEUP_CTRL_REQ_EN, 1, 1);
    }
    else {
        dw_rtc_modifyl(base, WAKEUP_CTRL, WAKEUP_CTRL_ENABLE, 1, 0);
        dw_rtc_modifyl(base, WAKEUP_CTRL, WAKEUP_CTRL_IRQ_EN, 1, 0);
        dw_rtc_modifyl(base, WAKEUP_CTRL, WAKEUP_CTRL_REQ_EN, 1, 0);
    }

    return 0;
}

/**
 * @brief get rtc alarm enable status
 *
 * @param base
 * @return true
 * @return false
 */
bool dw_rtc_lld_get_alarm_enable(paddr_t base)
{
    uint32_t val = dw_rtc_readl(base, WAKEUP_CTRL);
    return !!(val & ((1 << WAKEUP_CTRL_ENABLE) | (1 << WAKEUP_CTRL_IRQ_EN) |
                     (1 << WAKEUP_CTRL_REQ_EN)));
}

/**
 * @brief get rtc alarm status
 *
 * @param base
 * @return true
 * @return false
 */
bool dw_rtc_lld_get_alarm_status(paddr_t base)
{
    return !!(dw_rtc_readl(base, WAKEUP_CTRL) & (1 << WAKEUP_CTRL_STATUS));
}

/**
 * @brief get rtc alarm tick
 *
 * @param base
 * @return uint64_t
 */
uint64_t dw_rtc_lld_get_alarm_tick(paddr_t base)
{
    return ((uint64_t)dw_rtc_readl(base, TIMER_H) << 32) |
           (uint64_t)dw_rtc_readl(base, TIMER_L);
}

/**
 * @brief rtc enable
 *
 * @param base
 * @return int
 */
int dw_rtc_lld_enable(paddr_t base)
{
    uint32_t var = readl(base + SEC);

    if (!dw_rtc_lld_is_locked(base)) {
        var |= (1ul << SEC_LOCAL_ENABLE);
        dw_rtc_writel(base, SEC, var);
        return 0;
    }

    return -1;
}

/**
 * @brief rtc disable
 *
 * @param base
 * @return int
 */
int dw_rtc_lld_disable(paddr_t base)
{
    if (!dw_rtc_lld_is_locked(base)) {
        RMWREG32(base + SEC, 0, 1, 0);
        return 0;
    }

    return -1;
}

/**
 * @brief set rtc alarm value
 *
 * @param base
 * @param v
 * @return int
 */
int dw_rtc_lld_alarm_compare_value(paddr_t base, uint64_t v)
{
    dw_rtc_writel(base, TIMER_L, (uint32_t)(v & 0xfffffffful));
    dw_rtc_writel(base, TIMER_H, (uint32_t)((v >> 32) & 0xfffffffful));

    return 0;
}

/**
 * @brief get rtc tickets
 *
 * @param base
 * @return uint64_t
 */
uint64_t dw_rtc_lld_get_tick(paddr_t base)
{
    return ((uint64_t)dw_rtc_readl(base, RTC_H) << 32) |
           (uint64_t)dw_rtc_readl(base, RTC_L);
}

/**
 * @brief set rtc tickets
 *
 * @param base
 * @param v
 */
void dw_rtc_lld_set_tick(paddr_t base, uint64_t v)
{
    /* if user want to modify ticks, please disable RTC_ENABLE bit.
     * so in this case ,do not do rtc clock synchronization .
     */
    dw_rtc_writel_no_sync(base, RTC_H, (uint32_t)((v >> 32) & 0xfffffffful));
    dw_rtc_writel_no_sync(base, RTC_L, (uint32_t)(v & 0xfffffffful));
}
