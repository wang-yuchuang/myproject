/**
 * @file arch_timer.c
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: arch timer.
 *
 * Revision History:
 * -----------------
 */

#include <types.h>
#include <string.h>

#include "driver/arch_timer.h"

#define TICK_TO_US(tick)            (tick * 1000)

static arch_timer_t g_arch_timer;

static void arch_timer_callback(void *arg)
{
    g_arch_timer.time_base += TICK_TO_US(CONFIG_OS_TICK);
#ifdef CONFIG_USE_TICKLESS_IDLE
    g_arch_timer.count_adjust = 0;
#endif

    if (g_arch_timer.callback) {
        g_arch_timer.callback(g_arch_timer.arg);
    }
}

/**
 * @brief arch timer init
 *
 * @param[in] timer_dev timer dev
 */
void arch_timer_init(timer_dev_t *timer_dev)
{
    g_arch_timer.time_base = 0;
    g_arch_timer.timer_dev = timer_dev;

    /*
     * start arch timer at board_time_init,
     *  make sure udelay can use before os start.
     */
    arch_timer_start(NULL, NULL);
}

/**
 * @brief arch timer set start
 *
 * @param[in] callback callback function
 * @param[in] arg callback arg
 */
void arch_timer_start(arch_timer_cb callback, void *arg)
{
    timer_dev_t *timer_dev = g_arch_timer.timer_dev;
    if (timer_dev) {
        g_arch_timer.callback = callback;
        g_arch_timer.arg = arg;
        timer_set_callback(timer_dev, arch_timer_callback, NULL);
        timer_start(timer_dev, TMR_TYPE_PERIOD, CONFIG_OS_TICK);
    }
}

/**
 * @brief arch timer set start
 *
 * @param[in] callback callback function
 * @param[in] arg callback arg
 */
void arch_timer_restart(arch_timer_cb callback, void *arg)
{
    timer_dev_t *timer_dev = g_arch_timer.timer_dev;
    if (timer_dev) {
        uint64_t current_time = arch_timer_get_current();
        timer_stop(timer_dev);
        g_arch_timer.time_base = current_time;
#ifdef CONFIG_USE_TICKLESS_IDLE
        g_arch_timer.count_adjust = 0;
#endif
        g_arch_timer.callback = callback;
        g_arch_timer.arg = arg;
        timer_start(timer_dev, TMR_TYPE_PERIOD, CONFIG_OS_TICK);
    }
}

/**
 * @brief arch timer register callback
 *
 * @param callback callback function
 * @param arg callback arg
 */
void arch_timer_register(arch_timer_cb callback, void* arg)
{
    timer_dev_t* timer_dev = g_arch_timer.timer_dev;

    if (timer_dev) {
        uint64_t current_time = arch_timer_get_current();
        g_arch_timer.time_base = current_time;

#ifdef CONFIG_USE_TICKLESS_IDLE
    g_arch_timer.count_adjust = 0;
#endif /* end CONFIG_USE_TICKLESS_IDLE */

        g_arch_timer.callback = callback;
        g_arch_timer.arg = arg;
    }
}

/**
 * @brief arch get current timer
 *
 * @return currentt timer(us)
 */
uint64_t arch_timer_get_current(void)
{
    timer_dev_t *timer_dev = g_arch_timer.timer_dev;
    uint64_t time_base;
    uint32_t current_time;

    if (!timer_dev) {
        return 0;
    }

    do {
        time_base = g_arch_timer.time_base;
        current_time = timer_get_current_time(timer_dev);
#ifdef CONFIG_USE_TICKLESS_IDLE
        current_time += timer_count_to_us(timer_dev,
                            g_arch_timer.count_adjust);
#endif
    } while (time_base != g_arch_timer.time_base);

    return time_base + current_time;
}

#ifdef CONFIG_USE_TICKLESS_IDLE

void arch_timer_frozen(void)
{
    if (g_arch_timer.timer_dev) {
        timer_frozen(g_arch_timer.timer_dev);
    }
}

void arch_timer_continue(void)
{
    if (g_arch_timer.timer_dev) {
        timer_continue(g_arch_timer.timer_dev);
    }
}

uint32_t arch_timer_get_counter_register(void)
{
    if (g_arch_timer.timer_dev) {
        return timer_get_counter_register(g_arch_timer.timer_dev);
    }
    else {
        return 0;
    }
}

void arch_timer_set_terminal_register(uint32_t val)
{
    if (g_arch_timer.timer_dev) {
        timer_set_terminal_register(g_arch_timer.timer_dev, val);
    }
}

void arch_timer_force_reload(void)
{
    if (g_arch_timer.timer_dev) {
        timer_force_reload(g_arch_timer.timer_dev);
    }
}

bool arch_timer_overflow_int_occur(void)
{
    if (g_arch_timer.timer_dev) {
        return timer_overflow_int_occur(g_arch_timer.timer_dev);
    }
    else {
        return false;
    }
}

void arch_timer_clear_overflow_int(void)
{
    if (g_arch_timer.timer_dev) {
        timer_clear_overflow_int(g_arch_timer.timer_dev);
    }
}

void arch_timer_update_time_base(uint32_t val, uint32_t cnt)
{
    g_arch_timer.time_base += TICK_TO_US(val);
    g_arch_timer.count_adjust = cnt;
}

uint32_t arch_timer_get_count_adjust(void)
{
    return g_arch_timer.count_adjust;
}

#endif /* CONFIG_USE_TICKLESS_IDLE */

/**
 * @brief arch timer udelay
 *
 * @param[in] usecs delay us
 */
 void arch_udelay(uint32_t usecs)
{
    timer_dev_t *timer_dev = g_arch_timer.timer_dev;
    uint32_t last, now;
    uint32_t current_time, exp_time;

#ifdef CONFIG_USE_TICKLESS_IDLE
    uint32_t adjust_time;

    adjust_time = timer_count_to_us(timer_dev,
        g_arch_timer.count_adjust);
#endif

    last = timer_get_current_time(timer_dev);
    current_time = last;
    exp_time = current_time + usecs;

#ifdef CONFIG_USE_TICKLESS_IDLE
    now = timer_get_current_time(timer_dev);
    if (now >= last && !g_arch_timer.count_adjust) {
        adjust_time = 0;
    }
#endif

    while (current_time < exp_time) {
        now = timer_get_current_time(timer_dev);
        if (now >= last) {
            current_time += now - last;
        }
        else {
            current_time += TICK_TO_US(CONFIG_OS_TICK) - last + now;
#ifdef CONFIG_USE_TICKLESS_IDLE
            current_time -= adjust_time;
            adjust_time = 0;
#endif
        }
        last = now;
    }
}
