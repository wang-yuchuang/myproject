/**
 * @file arch_timer.h
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: arch timer.
 *
 * Revision History:
 * -----------------
 */

#ifndef INCLUDE_DRV_ARCH_TIMER_H
#define INCLUDE_DRV_ARCH_TIMER_H

#include <driver/timer.h>

/* arch timer callback */
typedef void (*arch_timer_cb)(void *arg);

typedef struct arch_timer {
  timer_dev_t *timer_dev;
  uint64_t time_base;
#ifdef CONFIG_USE_TICKLESS_IDLE
  uint32_t count_adjust;
#endif
  arch_timer_cb callback;
  void * arg;
} arch_timer_t;

/**
 * @brief arch timer init
 *
 * @param[in] timer_dev timer dev
 */
void arch_timer_init(timer_dev_t *timer_dev);

/**
 * @brief arch timer start
 *
 * @param[in] callback callback function
 * @param[in] arg callback arg
 */
void arch_timer_start(arch_timer_cb callback, void *arg);

/**
 * @brief arch timer set start
 *
 * @param[in] callback callback function
 * @param[in] arg callback arg
 */
void arch_timer_restart(arch_timer_cb callback, void *arg);

/**
 * @brief arch timer register callback
 *
 * @param callback callback function
 * @param arg callback arg
 */
void arch_timer_register(arch_timer_cb callback, void* arg);

/**
 * @brief arch get current timer
 *
 * @return currentt timer(us)
 */
uint64_t arch_timer_get_current(void);

#ifdef CONFIG_USE_TICKLESS_IDLE

void arch_timer_frozen(void);

void arch_timer_continue(void);

uint32_t arch_timer_get_counter_register(void);

void arch_timer_set_terminal_register(uint32_t val);

void arch_timer_force_reload(void);

bool arch_timer_overflow_int_occur(void);

void arch_timer_clear_overflow_int(void);

void arch_timer_update_time_base(uint32_t val, uint32_t cnt);

uint32_t arch_timer_get_count_adjust(void);

#endif /* CONFIG_USE_TICKLESS_IDLE */

/**
 * @brief arch timer udelay
 *
 * @param[in] usecs delay us
 */
void arch_udelay(uint32_t usecs);

#endif
