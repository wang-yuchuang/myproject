/**
 * @file timer_wrapper.c
 * @brief FreeRTOS timer port code.
 *
 * @copyright Copyright (c) 2020  Semidrive Semiconductor.
 *            All rights reserved.
 */

#include <compiler.h>
#include <types.h>

#include <board.h>
#include <FreeRTOS.h>
#include <driver/arch_timer.h>

void vPortTickTimerEnable(void (*Tick_Handler)(void))
{
#ifdef CONFIG_TIMER
    arch_timer_register((arch_timer_cb)Tick_Handler, NULL);
#endif
}

unsigned long long vPortGetCurrentTimeUs(void)
{
#ifdef CONFIG_TIMER
    return arch_timer_get_current();
#else
    return 0;
#endif
}

#ifdef CONFIG_USE_TICKLESS_IDLE

void vPortTickTimerStop(void)
{
    arch_timer_frozen();
}

void vPortTickTimerContinue(void)
{
    arch_timer_continue();
}

uint32_t vPortTickTimerGetCounter(void)
{
    return arch_timer_get_counter_register();
}

void vPortTickTimerSetTerminalValue(uint32_t terminal)
{
    arch_timer_set_terminal_register(terminal);
}

void vPortTickTimerForceReload(void)
{
    arch_timer_force_reload();
}

bool vPortTickTimerOverflowIntOccur(void)
{
    return arch_timer_overflow_int_occur();
}

void vPortTickTimerClearOverflowInt(void)
{
    arch_timer_clear_overflow_int();
}

void vPortTickTimerTimeBaseUpdate(uint32_t tickTime, uint32_t counter)
{
    arch_timer_update_time_base(tickTime, counter);
}

uint32_t vPortTickTimerGetCountAdjust(void)
{
    return arch_timer_get_count_adjust();
}

__WEAK void vPortSuppressTickAndSleepProcess(uint32_t idleTime)
{
    __ASM volatile ( "dsb" ::: "memory" );
    __ASM volatile ( "wfi" );
    __ASM volatile ( "isb" );
}

#endif /* CONFIG_USE_TICKLESS_IDLE */
