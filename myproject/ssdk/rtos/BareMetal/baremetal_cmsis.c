/*
 * baremetal_cmsis.c
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: bare metal cmsis handler.
 *
 * Revision History:
 * -----------------
 */


#ifdef __ICCARM__
#include <stdlib.h>
#else
#include <malloc.h>
#endif

#include <string.h>
#include <stdio.h>

#include <compiler.h>
#include <cmsis_os2.h>
#include <debug.h>
#include <lib/list.h>
#include <arch/barriers.h>
#include <driver/irq.h>
#include <driver/arch_timer.h>
#include <board.h>

#define configTICK_RATE_HZ                      (CONFIG_OS_TICK * 1000 )
#define configTICK_PERIOD_MS                    CONFIG_OS_TICK

#define BARE_METAL_TIME_STOP                    0
#define BARE_METAL_TIME_RUNNINT                 1

#define BARE_METAL_IDLE_STACK_SIZE              1024

#ifdef CONFIG_BAREMETAL_WORK_IN_IRQ
#define BARE_METAL_IRQ_MODE()                   0
#else
#define BARE_METAL_IRQ_MODE()                   arch_in_irq_mode()
#endif

typedef struct bare_metal_thread {
    struct list_node        node;
    struct list_node        running_node;
    osThreadAttr_t          attr;
    osThreadFunc_t          func;
    void                    *argument;
    osThreadState_t         thread_state;
} bare_metal_thread_t;

typedef struct bare_metal_sema {
    osSemaphoreAttr_t       attr;
    uint32_t                max_count;
    uint32_t                initial_count;
    uint32_t                current_count;
    osThreadId_t            thread_wait;
} bare_metal_sema_t;

typedef struct bare_metal_mutex {
    osMutexAttr_t           attr;
} bare_metal_mutex_t;

typedef struct bare_metal_event {
    osEventFlagsAttr_t      attr;
    uint32_t                event_bitmap;
    osThreadId_t            thread_wait;
} bare_metal_event_t;

typedef struct bare_metal_msgq {
    osMessageQueueAttr_t    attr;
    uint32_t                msg_count;
    uint32_t                msg_size;
    uint8_t                 *msg_q;
    uint32_t                msg_head;
    uint32_t                msg_tail;
    osThreadId_t            thread_wait;
} bare_metal_msgq_t;

typedef struct bare_metal_timer {
    struct list_node        node;
    osTimerAttr_t           attr;
    osTimerType_t           type;
    osTimerFunc_t           func;
    void                    *argument;
    uint32_t                time_len;   //tick
    uint32_t                time_out;   //tick
    uint32_t                state;
} bare_metal_timer_t;

static struct list_node g_thread_list;
static struct list_node g_active_list;
static osThreadId_t g_idle_thread;
static osThreadId_t g_current_thread;

static osKernelState_t g_kernel_state = osKernelInactive;

static uint32_t g_tick_count;
static struct list_node g_timer_list;

//  ==== BareMetal Internal Functions ====
static void BareMetalAddRunningThread(bare_metal_thread_t *pcurr_thread)
{
    bare_metal_thread_t *pthread;
    struct list_node *pre_node = NULL;

    IRQ_SAVE

    list_for_every_entry(&g_active_list, pthread, bare_metal_thread_t, running_node) {
        if (pthread->attr.priority < pcurr_thread->attr.priority) {
            pre_node = &pthread->running_node;
            break;
        }
    }
    if (pre_node != NULL) {
        list_add_before(pre_node, &pcurr_thread->running_node);
    }
    else {
        list_add_tail(&g_active_list, &pcurr_thread->running_node);
    }
    pcurr_thread->thread_state = osThreadReady;

    IRQ_RESTORE
}

static void BareMetalThreadSched(void)
{
    bare_metal_thread_t *pthread;
    irq_state_t irq_flags;

    irq_flags = arch_irq_save();

    while (!list_is_empty(&g_active_list)) {

        pthread = list_remove_head_type(&g_active_list, bare_metal_thread_t, running_node);

        arch_irq_restore(irq_flags);

        ASSERT(pthread != NULL);
        ASSERT(pthread->thread_state == osThreadReady);

        pthread->thread_state = osThreadRunning;

        g_current_thread = (osThreadId_t)pthread;
        pthread->func(pthread->argument);
        g_current_thread = NULL;

        if (pthread->thread_state == osThreadRunning) {
            pthread->thread_state = osThreadReady;
        }
        else if (pthread->thread_state == osThreadTerminated) {
            list_delete(&pthread->node);
            free(pthread);
        }

        irq_flags = arch_irq_save();
    }

    BareMetalAddRunningThread((bare_metal_thread_t *)g_idle_thread);

    arch_irq_restore(irq_flags);
}

static void BareMetalTickHandle(void *arg)
{
    bare_metal_timer_t *ptimer;

    IRQ_SAVE

    g_tick_count++;

    list_for_every_entry(&g_timer_list, ptimer, bare_metal_timer_t, node) {
        if (ptimer->time_out <= g_tick_count) {
            ptimer->func(ptimer->argument);
            if (ptimer->type == osTimerPeriodic) {
                osTimerStart((osTimerId_t)ptimer, ptimer->time_len);
            }
            else {
                ptimer->state = BARE_METAL_TIME_STOP;
            }
        }
    }

    IRQ_RESTORE
}

static void BareMetalEnableTick(void)
{
#ifdef CONFIG_TIMER
    arch_timer_register((arch_timer_cb)BareMetalTickHandle, NULL);
#endif
}

static void BareMetalIdleFunc(void *arg)
{
    arch_idle();
}

static osStatus_t BareMetalInitIdleThread(void)
{
    osThreadAttr_t attr = {
        .name = "idle",
        .stack_size = BARE_METAL_IDLE_STACK_SIZE,
        .priority = osPriorityIdle,
    };

    g_idle_thread = osThreadNew(BareMetalIdleFunc, NULL, &attr);
    if (!g_idle_thread) {
        return osError;
    }

    return osOK;
}

//  ==== Kernel Management Functions ====

/// Initialize the RTOS Kernel.
/// \return status code that indicates the execution status of the function.
osStatus_t osKernelInitialize (void)
{
    list_initialize(&g_thread_list);
    list_initialize(&g_active_list);
    list_initialize(&g_timer_list);
    g_kernel_state = osKernelReady;
    return osOK;
}

///  Get RTOS Kernel Information.
/// \param[out]    version       pointer to buffer for retrieving version information.
/// \param[out]    id_buf        pointer to buffer for retrieving kernel identification string.
/// \param[in]     id_size       size of buffer for kernel identification string.
/// \return status code that indicates the execution status of the function.
osStatus_t osKernelGetInfo (osVersion_t *version, char *id_buf, uint32_t id_size)
{
    return osOK;
}

/// Get the current RTOS Kernel state.
/// \return current RTOS Kernel state.
osKernelState_t osKernelGetState (void)
{
    return g_kernel_state;
}

/// Start the RTOS Kernel scheduler.
/// \return status code that indicates the execution status of the function.
osStatus_t osKernelStart (void)
{
    for (;;) {
        switch (g_kernel_state) {
            case osKernelReady:
                g_kernel_state = osKernelRunning;
                BareMetalInitIdleThread();
                BareMetalEnableTick();
                break;
            case osKernelRunning:
                BareMetalThreadSched();
                break;
            default:
                break;
        }
    }

    return osOK;
}

/// Lock the RTOS Kernel scheduler.
/// \return previous lock state (1 - locked, 0 - not locked, error code if negative).
int32_t osKernelLock (void)
{
    if (g_kernel_state == osKernelRunning) {
        g_kernel_state = osKernelLocked;
    }
    return 0;
}

/// Unlock the RTOS Kernel scheduler.
/// \return previous lock state (1 - locked, 0 - not locked, error code if negative).
int32_t osKernelUnlock (void)
{
    if (g_kernel_state == osKernelLocked) {
        g_kernel_state = osKernelRunning;
    }
    return 1;
}

/// Restore the RTOS Kernel scheduler lock state.
/// \param[in]     lock          lock state obtained by \ref osKernelLock or \ref osKernelUnlock.
/// \return new lock state (1 - locked, 0 - not locked, error code if negative).
int32_t osKernelRestoreLock (int32_t lock)
{
    osKernelState_t old_state = g_kernel_state;

    if (lock) {
        osKernelLock();
    }
    else {
        osKernelUnlock();
    }

    return (int32_t)old_state;
}

/// Suspend the RTOS Kernel scheduler.
/// \return time in ticks, for how long the system can sleep or power-down.
uint32_t osKernelSuspend (void)
{
    if (g_kernel_state == osKernelRunning) {
        g_kernel_state = osKernelSuspended;
    }
    return 0;
}

/// Resume the RTOS Kernel scheduler.
/// \param[in]     sleep_ticks   time in ticks for how long the system was in sleep or power-down mode.
void osKernelResume (uint32_t sleep_ticks)
{
    if (g_kernel_state == osKernelSuspended) {
        g_kernel_state = osKernelRunning;
    }
}

/// Get the RTOS kernel tick count.
/// \return RTOS kernel current tick count.
uint32_t osKernelGetTickCount (void)
{
    return g_tick_count;
}

/// Get the RTOS kernel tick frequency.
/// \return frequency of the kernel tick in hertz, i.e. kernel ticks per second.
uint32_t osKernelGetTickFreq (void)
{
    return configTICK_RATE_HZ;
}

/// Get the RTOS kernel system timer count.
/// \return RTOS kernel current system timer count as 32-bit value.
uint32_t osKernelGetSysTimerCount (void)
{
    return 0;
}

/// Get the RTOS kernel system timer frequency.
/// \return frequency of the system timer in hertz, i.e. timer ticks per second.
uint32_t osKernelGetSysTimerFreq (void)
{
    return 0;
}


//  ==== Thread Management Functions ====

/// Create a thread and add it to Active Threads.
/// \param[in]     func          thread function.
/// \param[in]     argument      pointer that is passed to the thread function as start argument.
/// \param[in]     attr          thread attributes; NULL: default values.
/// \return thread ID for reference by other functions or NULL in case of error.
osThreadId_t osThreadNew (osThreadFunc_t func, void *argument, const osThreadAttr_t *attr)
{
    bare_metal_thread_t *pcurr_thread;

    ASSERT(!BARE_METAL_IRQ_MODE());

    pcurr_thread = (bare_metal_thread_t *)calloc(1, sizeof(bare_metal_thread_t));
    if (pcurr_thread) {
        pcurr_thread->func = func;
        pcurr_thread->argument = argument;
        if (attr) {
            memcpy(&pcurr_thread->attr, attr, sizeof(osThreadAttr_t));
        }
        list_add_tail(&g_thread_list, &pcurr_thread->node);

        BareMetalAddRunningThread(pcurr_thread);
    }

    return (osThreadId_t)pcurr_thread;
}

/// Get name of a thread.
/// \param[in]     thread_id     thread ID obtained by \ref osThreadNew or \ref osThreadGetId.
/// \return name as null-terminated string.
const char *osThreadGetName (osThreadId_t thread_id)
{
    bare_metal_thread_t *pthread = (bare_metal_thread_t *)thread_id;

    if (pthread) {
        return pthread->attr.name;
    }
    return NULL;
}

/// Return the thread ID of the current running thread.
/// \return thread ID for reference by other functions or NULL in case of error.
osThreadId_t osThreadGetId (void)
{
    return g_current_thread;
}

/// Get current thread state of a thread.
/// \param[in]     thread_id     thread ID obtained by \ref osThreadNew or \ref osThreadGetId.
/// \return current thread state of the specified thread.
osThreadState_t osThreadGetState (osThreadId_t thread_id)
{
    bare_metal_thread_t *pthread = (bare_metal_thread_t *)thread_id;

    if (pthread) {
        return pthread->thread_state;
    }
    return osThreadError;
}

/// Get stack size of a thread.
/// \param[in]     thread_id     thread ID obtained by \ref osThreadNew or \ref osThreadGetId.
/// \return stack size in bytes.
uint32_t osThreadGetStackSize (osThreadId_t thread_id)
{
    return 0;
}

/// Get available stack space of a thread based on stack watermark recording during execution.
/// \param[in]     thread_id     thread ID obtained by \ref osThreadNew or \ref osThreadGetId.
/// \return remaining stack space in bytes.
uint32_t osThreadGetStackSpace (osThreadId_t thread_id)
{
    return 0;
}

/// Change priority of a thread.
/// \param[in]     thread_id     thread ID obtained by \ref osThreadNew or \ref osThreadGetId.
/// \param[in]     priority      new priority value for the thread function.
/// \return status code that indicates the execution status of the function.
osStatus_t osThreadSetPriority (osThreadId_t thread_id, osPriority_t priority)
{
    bare_metal_thread_t *pcurr_thread = (bare_metal_thread_t *)thread_id;

    if (!pcurr_thread) {
        return osError;
    }

    if (pcurr_thread->attr.priority != priority) {
        pcurr_thread->attr.priority = priority;
    }

    return osOK;
}

/// Get current priority of a thread.
/// \param[in]     thread_id     thread ID obtained by \ref osThreadNew or \ref osThreadGetId.
/// \return current priority value of the specified thread.
osPriority_t osThreadGetPriority (osThreadId_t thread_id)
{
    bare_metal_thread_t *pcurr_thread = (bare_metal_thread_t *)thread_id;

    if (pcurr_thread) {
        return pcurr_thread->attr.priority;
    }

    return osPriorityNone;
}

/// Pass control to next thread that is in state \b READY.
/// \return status code that indicates the execution status of the function.
osStatus_t osThreadYield (void)
{
    /* not supported */
    return osError;
}

/// Suspend execution of a thread.
/// \param[in]     thread_id     thread ID obtained by \ref osThreadNew or \ref osThreadGetId.
/// \return status code that indicates the execution status of the function.
osStatus_t osThreadSuspend (osThreadId_t thread_id)
{
    bare_metal_thread_t *pcurr_thread = (bare_metal_thread_t *)thread_id;

    if (pcurr_thread) {
        pcurr_thread->thread_state = osThreadBlocked;
        return osOK;
    }

    return osError;
}

/// Resume execution of a thread.
/// \param[in]     thread_id     thread ID obtained by \ref osThreadNew or \ref osThreadGetId.
/// \return status code that indicates the execution status of the function.
osStatus_t osThreadResume (osThreadId_t thread_id)
{
    bare_metal_thread_t *pcurr_thread = (bare_metal_thread_t *)thread_id;

    if (pcurr_thread) {
        pcurr_thread->thread_state = osThreadReady;
        return osOK;
    }

    return osError;
}

/// Detach a thread (thread storage can be reclaimed when thread terminates).
/// \param[in]     thread_id     thread ID obtained by \ref osThreadNew or \ref osThreadGetId.
/// \return status code that indicates the execution status of the function.
osStatus_t osThreadDetach (osThreadId_t thread_id)
{
    /* not supported */
    return osError;
}

/// Wait for specified thread to terminate.
/// \param[in]     thread_id     thread ID obtained by \ref osThreadNew or \ref osThreadGetId.
/// \return status code that indicates the execution status of the function.
osStatus_t osThreadJoin (osThreadId_t thread_id)
{
    /* not supported */
    return osError;
}

/// Terminate execution of current running thread.
__NO_RETURN void osThreadExit (void) {
#ifndef USE_FreeRTOS_HEAP_1
  //vTaskDelete (NULL);
#endif
  for (;;);
}

/// Terminate execution of a thread.
/// \param[in]     thread_id     thread ID obtained by \ref osThreadNew or \ref osThreadGetId.
/// \return status code that indicates the execution status of the function.
osStatus_t osThreadTerminate (osThreadId_t thread_id)
{
    bare_metal_thread_t *pcurr_thread = (bare_metal_thread_t *)thread_id;

    ASSERT(!BARE_METAL_IRQ_MODE());

    if (!pcurr_thread) {
        return osError;
    }

    pcurr_thread->thread_state = osThreadTerminated;
    return osOK;
}

/// Get number of active threads.
/// \return number of active threads.
uint32_t osThreadGetCount (void)
{
    return list_length(&g_thread_list);
}

/// Enumerate active threads.
/// \param[out]    thread_array  pointer to array for retrieving thread IDs.
/// \param[in]     array_items   maximum number of items in array for retrieving thread IDs.
/// \return number of enumerated threads.
uint32_t osThreadEnumerate (osThreadId_t *thread_array, uint32_t array_items)
{
    bare_metal_thread_t *pthread;
    uint32_t thread_num = 0;

    list_for_every_entry(&g_thread_list, pthread, bare_metal_thread_t, node) {
        thread_array[thread_num] = (osThreadId_t *)pthread;
        thread_num++;
    }

    return thread_num;
}

/*
  Enumerate active threads and get threads info.
*/
uint32_t osThreadInfoEnumerate (osThreadInfo_t *threadInfo_array, uint32_t array_items, uint32_t *total_runtime) {
    bare_metal_thread_t *pthread;
    uint32_t thread_num = 0;

    list_for_every_entry(&g_thread_list, pthread, bare_metal_thread_t, node) {
        threadInfo_array[thread_num].base_priority = 0;
        threadInfo_array[thread_num].current_priority = pthread->attr.priority;
        threadInfo_array[thread_num].current_state = pthread->thread_state;
        threadInfo_array[thread_num].thread_id = 0;
        threadInfo_array[thread_num].runtime_counter = 0;
        threadInfo_array[thread_num].thread_handle = (osThreadId_t)pthread;
        thread_num++;
    }

    return thread_num;
}
//  ==== Thread Flags Functions ====

/// Set the specified Thread Flags of a thread.
/// \param[in]     thread_id     thread ID obtained by \ref osThreadNew or \ref osThreadGetId.
/// \param[in]     flags         specifies the flags of the thread that shall be set.
/// \return thread flags after setting or error code if highest bit set.
uint32_t osThreadFlagsSet (osThreadId_t thread_id, uint32_t flags)
{
    /* not supported */
    return (uint32_t)osError;
}

/// Clear the specified Thread Flags of current running thread.
/// \param[in]     flags         specifies the flags of the thread that shall be cleared.
/// \return thread flags before clearing or error code if highest bit set.
uint32_t osThreadFlagsClear (uint32_t flags)
{
    /* not supported */
    return (uint32_t)osError;
}

/// Get the current Thread Flags of current running thread.
/// \return current thread flags.
uint32_t osThreadFlagsGet (void)
{
    /* not supported */
    return (uint32_t)osError;
}

/// Wait for one or more Thread Flags of the current running thread to become signaled.
/// \param[in]     flags         specifies the flags to wait for.
/// \param[in]     options       specifies flags options (osFlagsXxxx).
/// \param[in]     timeout       \ref CMSIS_RTOS_TimeOutValue or 0 in case of no time-out.
/// \return thread flags before clearing or error code if highest bit set.
uint32_t osThreadFlagsWait (uint32_t flags, uint32_t options, uint32_t timeout)
{
    /* not supported */
    return (uint32_t)osError;
}

//  ==== Generic Wait Functions ====

/// Wait for Timeout (Time Delay).
/// \param[in]     ticks         \ref CMSIS_RTOS_TimeOutValue "time ticks" value
/// \return status code that indicates the execution status of the function.
osStatus_t osDelay (uint32_t ticks)
{
    volatile uint32_t *tick_ptr = &g_tick_count;
    uint32_t start = *tick_ptr;

    while ((*tick_ptr - start) < ticks)
        ;
    return osOK;
}

/// Wait until specified time.
/// \param[in]     ticks         absolute time in ticks
/// \return status code that indicates the execution status of the function.
osStatus_t osDelayUntil (uint32_t ticks)
{
    volatile uint32_t *tick_ptr = &g_tick_count;
    uint32_t delta;
    uint32_t start = *tick_ptr;

    delta = (ticks > start) ? (ticks - start) : 1;
    while ((*tick_ptr - start) < delta)
        ;
    return osOK;
}

//  ==== Timer Management Functions ====

/// Create and Initialize a timer.
/// \param[in]     func          function pointer to callback function.
/// \param[in]     type          \ref osTimerOnce for one-shot or \ref osTimerPeriodic for periodic behavior.
/// \param[in]     argument      argument to the timer callback function.
/// \param[in]     attr          timer attributes; NULL: default values.
/// \return timer ID for reference by other functions or NULL in case of error.
osTimerId_t osTimerNew (osTimerFunc_t func, osTimerType_t type, void *argument, const osTimerAttr_t *attr)
{
    bare_metal_timer_t *timer;

    ASSERT(!BARE_METAL_IRQ_MODE());

    timer = (bare_metal_timer_t *)calloc(1, sizeof(bare_metal_timer_t));
    if (timer) {
        timer->type = type;
        timer->func = func;
        timer->argument = argument;
        timer->state = BARE_METAL_TIME_STOP;
        if (attr) {
            memcpy(&timer->attr, attr, sizeof(osTimerAttr_t));
        }
    }

    return timer;
}

/// Get name of a timer.
/// \param[in]     timer_id      timer ID obtained by \ref osTimerNew.
/// \return name as null-terminated string.
const char *osTimerGetName (osTimerId_t timer_id)
{
    bare_metal_timer_t *timer = (bare_metal_timer_t *)timer_id;
    const char *name = NULL;

    IRQ_SAVE

    if (timer) {
        name = timer->attr.name;
    }

    IRQ_RESTORE

    return name;
}

/// Start or restart a timer.
/// \param[in]     timer_id      timer ID obtained by \ref osTimerNew.
/// \param[in]     ticks         \ref CMSIS_RTOS_TimeOutValue "time ticks" value of the timer.
/// \return status code that indicates the execution status of the function.
osStatus_t osTimerStart (osTimerId_t timer_id, uint32_t ticks)
{
    bare_metal_timer_t *pcurr_timer = (bare_metal_timer_t *)timer_id;
    bare_metal_timer_t *ptimer;

    if (!pcurr_timer) {
        return osError;
    }

    if (ticks == 0) {
        ticks = 1;
    }

    IRQ_SAVE

    if (pcurr_timer->state == BARE_METAL_TIME_RUNNINT) {
        list_delete(&pcurr_timer->node);
    }

    pcurr_timer->time_len = ticks;
    pcurr_timer->time_out = g_tick_count + ticks;

    list_for_every_entry(&g_timer_list, ptimer, bare_metal_timer_t, node) {
        if (ptimer->time_out > pcurr_timer->time_out) {
            list_add_before(&ptimer->node, &pcurr_timer->node);
            goto start_out;
        }
    }

    list_add_tail(&g_timer_list, &pcurr_timer->node);

    pcurr_timer->state = BARE_METAL_TIME_RUNNINT;

start_out:

    IRQ_RESTORE

    return osOK;
}

/// Stop a timer.
/// \param[in]     timer_id      timer ID obtained by \ref osTimerNew.
/// \return status code that indicates the execution status of the function.
osStatus_t osTimerStop (osTimerId_t timer_id)
{
    bare_metal_timer_t *timer = (bare_metal_timer_t *)timer_id;

    IRQ_SAVE

    if (timer && timer->state == BARE_METAL_TIME_RUNNINT) {
        list_delete(&timer->node);
        timer->state = BARE_METAL_TIME_STOP;
    }

    IRQ_RESTORE

    return osOK;
}

/// Check if a timer is running.
/// \param[in]     timer_id      timer ID obtained by \ref osTimerNew.
/// \return 0 not running, 1 running.
uint32_t osTimerIsRunning (osTimerId_t timer_id)
{
    bare_metal_timer_t *timer = (bare_metal_timer_t *)timer_id;
    uint32_t state = 0;

    IRQ_SAVE

    if (timer) {
        state = (uint32_t)(timer->state == BARE_METAL_TIME_RUNNINT);
    }

    IRQ_RESTORE

    return state;
}

/// Delete a timer.
/// \param[in]     timer_id      timer ID obtained by \ref osTimerNew.
/// \return status code that indicates the execution status of the function.
osStatus_t osTimerDelete (osTimerId_t timer_id)
{
    bare_metal_timer_t *timer = (bare_metal_timer_t *)timer_id;
    osStatus_t ret = osError;

    ASSERT(!BARE_METAL_IRQ_MODE());

    IRQ_SAVE

    if (timer) {
        free(timer);
        ret = osOK;
    }

    IRQ_RESTORE

    return ret;
}


//  ==== Event Flags Management Functions ====

/// Create and Initialize an Event Flags object.
/// \param[in]     attr          event flags attributes; NULL: default values.
/// \return event flags ID for reference by other functions or NULL in case of error.
osEventFlagsId_t osEventFlagsNew (const osEventFlagsAttr_t *attr)
{
    bare_metal_event_t *event;

    ASSERT(!BARE_METAL_IRQ_MODE());

    event = (bare_metal_event_t *)calloc(1, sizeof(bare_metal_event_t));
    if (event) {
        if (attr) {
            memcpy(&event->attr, attr, sizeof(osEventFlagsAttr_t));
        }
        event->thread_wait = NULL;
    }

    return (osEventFlagsId_t)event;
}

/// Get name of an Event Flags object.
/// \param[in]     ef_id         event flags ID obtained by \ref osEventFlagsNew.
/// \return name as null-terminated string.
const char *osEventFlagsGetName (osEventFlagsId_t ef_id)
{
    bare_metal_event_t *event = (bare_metal_event_t *)ef_id;
    char *name = NULL;

    IRQ_SAVE

    if (event) {
        name = (char *)event->attr.name;
    }

    IRQ_RESTORE

    return name;
}

/// Set the specified Event Flags.
/// \param[in]     ef_id         event flags ID obtained by \ref osEventFlagsNew.
/// \param[in]     flags         specifies the flags that shall be set.
/// \return event flags after setting or error code if highest bit set.
uint32_t osEventFlagsSet (osEventFlagsId_t ef_id, uint32_t flags)
{
    bare_metal_event_t *event = (bare_metal_event_t *)ef_id;
    uint32_t new_bitmap = (uint32_t)osError;

    IRQ_SAVE

    if (event) {
        event->event_bitmap |= flags;
        new_bitmap = event->event_bitmap;

        if (new_bitmap != 0 && event->thread_wait != NULL) {
            BareMetalAddRunningThread((bare_metal_thread_t *)event->thread_wait);
            event->thread_wait = NULL;
        }
    }

    IRQ_RESTORE

    return new_bitmap;
}

/// Clear the specified Event Flags.
/// \param[in]     ef_id         event flags ID obtained by \ref osEventFlagsNew.
/// \param[in]     flags         specifies the flags that shall be cleared.
/// \return event flags before clearing or error code if highest bit set.
uint32_t osEventFlagsClear (osEventFlagsId_t ef_id, uint32_t flags)
{
    bare_metal_event_t *event = (bare_metal_event_t *)ef_id;
    uint32_t pre_bitmap = (uint32_t)osError;

    IRQ_SAVE

    if (event) {
        pre_bitmap = event->event_bitmap;
        event->event_bitmap &= ~flags;
    }

    IRQ_RESTORE

    return pre_bitmap;
}

/// Get the current Event Flags.
/// \param[in]     ef_id         event flags ID obtained by \ref osEventFlagsNew.
/// \return current event flags.
uint32_t osEventFlagsGet (osEventFlagsId_t ef_id)
{
    bare_metal_event_t *event = (bare_metal_event_t *)ef_id;
    uint32_t bitmap = (uint32_t)osError;

    IRQ_SAVE

    if (event) {
        bitmap = event->event_bitmap;
    }

    IRQ_RESTORE

    return bitmap;
}

/// Wait for one or more Event Flags to become signaled.
/// \param[in]     ef_id         event flags ID obtained by \ref osEventFlagsNew.
/// \param[in]     flags         specifies the flags to wait for.
/// \param[in]     options       specifies flags options (osFlagsXxxx).
/// \param[in]     timeout       \ref CMSIS_RTOS_TimeOutValue or 0 in case of no time-out.
/// \return event flags before clearing or error code if highest bit set.
uint32_t osEventFlagsWait (osEventFlagsId_t ef_id, uint32_t flags, uint32_t options, uint32_t timeout)
{
    bare_metal_event_t *event = (bare_metal_event_t *)ef_id;
    uint32_t bitmap = (uint32_t)osError;

    IRQ_SAVE

    if (event) {
        bitmap = event->event_bitmap & flags;
        if (bitmap) {
            event->event_bitmap &= ~flags;
        }

        if (options & osFlagsWaitAll) {
            if (bitmap != flags) {
                event->thread_wait = g_current_thread;
            }
        }
        else {
            if (bitmap == 0) {
                event->thread_wait = g_current_thread;
            }
        }
    }

    IRQ_RESTORE

    return bitmap;
}

/// Delete an Event Flags object.
/// \param[in]     ef_id         event flags ID obtained by \ref osEventFlagsNew.
/// \return status code that indicates the execution status of the function.
osStatus_t osEventFlagsDelete (osEventFlagsId_t ef_id)
{
    bare_metal_event_t *event = (bare_metal_event_t *)ef_id;
    osStatus_t ret = osError;

    ASSERT(!BARE_METAL_IRQ_MODE());

    IRQ_SAVE

    if (event) {
        free(event);
        ret = osOK;
    }

    IRQ_RESTORE

    return ret;
}

//  ==== Mutex Management Functions ====

/// Create and Initialize a Mutex object.
/// \param[in]     attr          mutex attributes; NULL: default values.
/// \return mutex ID for reference by other functions or NULL in case of error.
osMutexId_t osMutexNew (const osMutexAttr_t *attr)
{
    bare_metal_mutex_t *mutex;

    ASSERT(!BARE_METAL_IRQ_MODE());

    mutex = (bare_metal_mutex_t *)calloc(1, sizeof(bare_metal_mutex_t));
    if (mutex && attr) {
        memcpy(&mutex->attr, attr, sizeof(osMutexAttr_t));
    }

    return (osMutexId_t)mutex;
}

/// Get name of a Mutex object.
/// \param[in]     mutex_id      mutex ID obtained by \ref osMutexNew.
/// \return name as null-terminated string.
const char *osMutexGetName (osMutexId_t mutex_id)
{
    bare_metal_mutex_t *mutex = (bare_metal_mutex_t *)mutex_id;

    if (mutex) {
        return mutex->attr.name;
    }
    return NULL;
}

/// Acquire a Mutex or timeout if it is locked.
/// \param[in]     mutex_id      mutex ID obtained by \ref osMutexNew.
/// \param[in]     timeout       \ref CMSIS_RTOS_TimeOutValue or 0 in case of no time-out.
/// \return status code that indicates the execution status of the function.
osStatus_t osMutexAcquire (osMutexId_t mutex_id, uint32_t timeout)
{
    ASSERT(!BARE_METAL_IRQ_MODE());

    /* BareMetal always get mutex */
    return osOK;
}

/// Release a Mutex that was acquired by \ref osMutexAcquire.
/// \param[in]     mutex_id      mutex ID obtained by \ref osMutexNew.
/// \return status code that indicates the execution status of the function.
osStatus_t osMutexRelease (osMutexId_t mutex_id)
{
    ASSERT(!BARE_METAL_IRQ_MODE());

    /* BareMetal always get mutex */
    return osOK;
}

/// Get Thread which owns a Mutex object.
/// \param[in]     mutex_id      mutex ID obtained by \ref osMutexNew.
/// \return thread ID of owner thread or NULL when mutex was not acquired.
osThreadId_t osMutexGetOwner (osMutexId_t mutex_id)
{
    /* not supported */
    return NULL;
}

/// Delete a Mutex object.
/// \param[in]     mutex_id      mutex ID obtained by \ref osMutexNew.
/// \return status code that indicates the execution status of the function.
osStatus_t osMutexDelete (osMutexId_t mutex_id)
{
    bare_metal_mutex_t *mutex = (bare_metal_mutex_t *)mutex_id;

    ASSERT(!BARE_METAL_IRQ_MODE());

    if (mutex) {
        free(mutex);
    }
    return osOK;
}

//  ==== Semaphore Management Functions ====

/// Create and Initialize a Semaphore object.
/// \param[in]     max_count     maximum number of available tokens.
/// \param[in]     initial_count initial number of available tokens.
/// \param[in]     attr          semaphore attributes; NULL: default values.
/// \return semaphore ID for reference by other functions or NULL in case of error.
osSemaphoreId_t osSemaphoreNew (uint32_t max_count, uint32_t initial_count, const osSemaphoreAttr_t *attr)
{
    bare_metal_sema_t *sema;

    ASSERT(!BARE_METAL_IRQ_MODE());

    sema = (bare_metal_sema_t *)calloc(1, sizeof(bare_metal_sema_t));
    if (sema) {
        sema->max_count = max_count;
        sema->initial_count = initial_count;
        sema->current_count = initial_count;
        sema->thread_wait = NULL;
        if (attr) {
            memcpy(&sema->attr, attr, sizeof(osSemaphoreAttr_t));
        }
    }
    return (osSemaphoreId_t)sema;
}

/// Get name of a Semaphore object.
/// \param[in]     semaphore_id  semaphore ID obtained by \ref osSemaphoreNew.
/// \return name as null-terminated string.
const char *osSemaphoreGetName (osSemaphoreId_t semaphore_id)
{
    bare_metal_sema_t *sema = (bare_metal_sema_t*)semaphore_id;
    char *name = NULL;

    IRQ_SAVE

    if (sema) {
        name = (char *)sema->attr.name;
    }

    IRQ_RESTORE

    return name;
}

/// Acquire a Semaphore token or timeout if no tokens are available.
/// \param[in]     semaphore_id  semaphore ID obtained by \ref osSemaphoreNew.
/// \param[in]     timeout       \ref CMSIS_RTOS_TimeOutValue or 0 in case of no time-out.
/// \return status code that indicates the execution status of the function.
osStatus_t osSemaphoreAcquire (osSemaphoreId_t semaphore_id, uint32_t timeout)
{
    bare_metal_sema_t *sema = (bare_metal_sema_t*)semaphore_id;
    osStatus_t ret = osError;

    if (!sema) {
        return ret;
    }

    IRQ_SAVE

    if (sema->current_count > 0) {
        sema->current_count--;
        ret = osOK;
    }
    else {
        sema->thread_wait = g_current_thread;
    }

    IRQ_RESTORE

    /* no wait */
    return ret;
}

/// Release a Semaphore token up to the initial maximum count.
/// \param[in]     semaphore_id  semaphore ID obtained by \ref osSemaphoreNew.
/// \return status code that indicates the execution status of the function.
osStatus_t osSemaphoreRelease (osSemaphoreId_t semaphore_id)
{
    bare_metal_sema_t *sema = (bare_metal_sema_t*)semaphore_id;

    if (!sema) {
        return osError;
    }

    IRQ_SAVE

    if (sema->current_count < sema->max_count) {
        sema->current_count++;
    }

    if (sema->current_count > 0 && sema->thread_wait != NULL) {
        BareMetalAddRunningThread((bare_metal_thread_t *)sema->thread_wait);
        sema->thread_wait = NULL;
    }

    IRQ_RESTORE

    return osOK;
}

/// Get current Semaphore token count.
/// \param[in]     semaphore_id  semaphore ID obtained by \ref osSemaphoreNew.
/// \return number of tokens available.
uint32_t osSemaphoreGetCount (osSemaphoreId_t semaphore_id)
{
    bare_metal_sema_t *sema = (bare_metal_sema_t*)semaphore_id;
    uint32_t count = 0;

    IRQ_SAVE

    if (sema) {
        count = sema->current_count;
    }

    IRQ_RESTORE

    return count;
}

/// Delete a Semaphore object.
/// \param[in]     semaphore_id  semaphore ID obtained by \ref osSemaphoreNew.
/// \return status code that indicates the execution status of the function.
osStatus_t osSemaphoreDelete (osSemaphoreId_t semaphore_id)
{
    bare_metal_sema_t *sema = (bare_metal_sema_t*)semaphore_id;

    IRQ_SAVE

    if (sema) {
        free(sema);
    }

    IRQ_RESTORE

    return osOK;
}

//  ==== Memory Pool Management Functions ====

/// Create and Initialize a Memory Pool object.
/// \param[in]     block_count   maximum number of memory blocks in memory pool.
/// \param[in]     block_size    memory block size in bytes.
/// \param[in]     attr          memory pool attributes; NULL: default values.
/// \return memory pool ID for reference by other functions or NULL in case of error.
osMemoryPoolId_t osMemoryPoolNew (uint32_t block_count, uint32_t block_size, const osMemoryPoolAttr_t *attr)
{
    /* not supported */
    return NULL;
}

/// Get name of a Memory Pool object.
/// \param[in]     mp_id         memory pool ID obtained by \ref osMemoryPoolNew.
/// \return name as null-terminated string.
const char *osMemoryPoolGetName (osMemoryPoolId_t mp_id)
{
    /* not supported */
    return NULL;
}

/// Allocate a memory block from a Memory Pool.
/// \param[in]     mp_id         memory pool ID obtained by \ref osMemoryPoolNew.
/// \param[in]     timeout       \ref CMSIS_RTOS_TimeOutValue or 0 in case of no time-out.
/// \return address of the allocated memory block or NULL in case of no memory is available.
void *osMemoryPoolAlloc (osMemoryPoolId_t mp_id, uint32_t timeout)
{
    /* not supported */
    return NULL;
}

/// Return an allocated memory block back to a Memory Pool.
/// \param[in]     mp_id         memory pool ID obtained by \ref osMemoryPoolNew.
/// \param[in]     block         address of the allocated memory block to be returned to the memory pool.
/// \return status code that indicates the execution status of the function.
osStatus_t osMemoryPoolFree (osMemoryPoolId_t mp_id, void *block)
{
    /* not supported */
    return osOK;
}

/// Get maximum number of memory blocks in a Memory Pool.
/// \param[in]     mp_id         memory pool ID obtained by \ref osMemoryPoolNew.
/// \return maximum number of memory blocks.
uint32_t osMemoryPoolGetCapacity (osMemoryPoolId_t mp_id)
{
    /* not supported */
    return 0;
}

/// Get memory block size in a Memory Pool.
/// \param[in]     mp_id         memory pool ID obtained by \ref osMemoryPoolNew.
/// \return memory block size in bytes.
uint32_t osMemoryPoolGetBlockSize (osMemoryPoolId_t mp_id)
{
    /* not supported */
    return 0;
}

/// Get number of memory blocks used in a Memory Pool.
/// \param[in]     mp_id         memory pool ID obtained by \ref osMemoryPoolNew.
/// \return number of memory blocks used.
uint32_t osMemoryPoolGetCount (osMemoryPoolId_t mp_id)
{
    /* not supported */
    return 0;
}

/// Get number of memory blocks available in a Memory Pool.
/// \param[in]     mp_id         memory pool ID obtained by \ref osMemoryPoolNew.
/// \return number of memory blocks available.
uint32_t osMemoryPoolGetSpace (osMemoryPoolId_t mp_id)
{
    /* not supported */
    return 0;
}

/// Delete a Memory Pool object.
/// \param[in]     mp_id         memory pool ID obtained by \ref osMemoryPoolNew.
/// \return status code that indicates the execution status of the function.
osStatus_t osMemoryPoolDelete (osMemoryPoolId_t mp_id)
{
    /* not supported */
    return osOK;
}

//  ==== memory block Management Functions ====

/// Alloc a Memory object.
/// \param[in]     size         required memory block size.
/// \return address of the allocated memory block or NULL in case of no memory available.
void *osAlloc (uint32_t size)
{
    ASSERT(!BARE_METAL_IRQ_MODE());
    if (size > 0)
        return malloc(size);
    return NULL;
}

/// Alloc a Align Memory object.
/// \param[in]     align        required alignment size.
/// \param[in]     size         required memory block size.
/// \return address of the allocated memory block or NULL in case of no memory available.
void *osAllocAlign (uint32_t align, uint32_t size)
{
    ASSERT(!BARE_METAL_IRQ_MODE());
    if (size > 0)
#ifdef __ICCARM__
        return aligned_alloc(align, size);
#else
        return memalign(align, size);
#endif
    return NULL;
}

/// Delete a Memory object.
/// \param[in]     block        address of the allocated memory block that is returned to system.
void osFree (void *block)
{
    ASSERT(!BARE_METAL_IRQ_MODE());
    if (block)
        free(block);
}

//  ==== Message Queue Management Functions ====

/// Create and Initialize a Message Queue object.
/// \param[in]     msg_count     maximum number of messages in queue.
/// \param[in]     msg_size      maximum message size in bytes.
/// \param[in]     attr          message queue attributes; NULL: default values.
/// \return message queue ID for reference by other functions or NULL in case of error.
osMessageQueueId_t osMessageQueueNew (uint32_t msg_count, uint32_t msg_size, const osMessageQueueAttr_t *attr)
{
    bare_metal_msgq_t *msgq;

    ASSERT(!BARE_METAL_IRQ_MODE());

    msgq = (bare_metal_msgq_t *)calloc(1, sizeof(bare_metal_msgq_t));
    if (msgq) {
        msgq->msg_count = msg_count;
        msgq->msg_size = msg_size;
        if (attr) {
            memcpy(&msgq->attr, attr, sizeof(osMessageQueueAttr_t));
        }

        msgq->msg_q = malloc(msg_count * msg_size);
        msgq->msg_head = 0;
        msgq->msg_tail = 0;
        msgq->thread_wait = NULL;
    }

    return msgq;
}

/// Get name of a Message Queue object.
/// \param[in]     mq_id         message queue ID obtained by \ref osMessageQueueNew.
/// \return name as null-terminated string.
const char *osMessageQueueGetName (osMessageQueueId_t mq_id)
{
    bare_metal_msgq_t *msgq = (bare_metal_msgq_t *)mq_id;
    char *name = NULL;

    IRQ_SAVE

    if (msgq) {
        name = (char *)msgq->attr.name;
    }

    IRQ_RESTORE

    return name;
}

/// Put a Message into a Queue or timeout if Queue is full.
/// \param[in]     mq_id         message queue ID obtained by \ref osMessageQueueNew.
/// \param[in]     msg_ptr       pointer to buffer with message to put into a queue.
/// \param[in]     msg_prio      message priority.
/// \param[in]     timeout       \ref CMSIS_RTOS_TimeOutValue or 0 in case of no time-out.
/// \return status code that indicates the execution status of the function.
osStatus_t osMessageQueuePut (osMessageQueueId_t mq_id, const void *msg_ptr, uint8_t msg_prio, uint32_t timeout)
{
    bare_metal_msgq_t *msgq = (bare_metal_msgq_t *)mq_id;
    uint32_t tail_idx;
    osStatus_t ret = osError;

    if (!msgq) {
        return ret;
    }

    IRQ_SAVE

    if ((msgq->msg_tail - msgq->msg_head) >= msgq->msg_count) {
        ssdk_printf(SSDK_WARNING, "osMessageQueue full\n");
        goto qput_out;
    }

    tail_idx = msgq->msg_tail & (msgq->msg_count - 1);
    memcpy(msgq->msg_q + tail_idx * msgq->msg_size, msg_ptr, msgq->msg_size);
    DSB;
    msgq->msg_tail++;
    ret = osOK;

qput_out:

    if (msgq->thread_wait != NULL) {
        BareMetalAddRunningThread((bare_metal_thread_t *)msgq->thread_wait);
        msgq->thread_wait = NULL;
    }

    IRQ_RESTORE

    return ret;
}

/// Get a Message from a Queue or timeout if Queue is empty.
/// \param[in]     mq_id         message queue ID obtained by \ref osMessageQueueNew.
/// \param[out]    msg_ptr       pointer to buffer for message to get from a queue.
/// \param[out]    msg_prio      pointer to buffer for message priority or NULL.
/// \param[in]     timeout       \ref CMSIS_RTOS_TimeOutValue or 0 in case of no time-out.
/// \return status code that indicates the execution status of the function.
osStatus_t osMessageQueueGet (osMessageQueueId_t mq_id, void *msg_ptr, uint8_t *msg_prio, uint32_t timeout)
{
    bare_metal_msgq_t *msgq = (bare_metal_msgq_t *)mq_id;
    uint32_t head_idx;
    osStatus_t ret = osError;

    if (!msgq) {
        return ret;
    }

    IRQ_SAVE

    if (msgq->msg_tail == msgq->msg_head) {
        msgq->thread_wait = g_current_thread;
        goto qget_out;
    }

    head_idx = msgq->msg_head & (msgq->msg_count - 1);
    memcpy(msg_ptr, msgq->msg_q + head_idx * msgq->msg_size, msgq->msg_size);
    DSB;
    msgq->msg_head++;
    ret = osOK;

qget_out:

    IRQ_RESTORE

    return ret;
}

/// Get maximum number of messages in a Message Queue.
/// \param[in]     mq_id         message queue ID obtained by \ref osMessageQueueNew.
/// \return maximum number of messages.
uint32_t osMessageQueueGetCapacity (osMessageQueueId_t mq_id)
{
    bare_metal_msgq_t *msgq = (bare_metal_msgq_t *)mq_id;
    uint32_t count = 0;

    IRQ_SAVE

    if (msgq) {
        count = msgq->msg_count;
    }

    IRQ_RESTORE

    return count;
}

/// Get maximum message size in a Message Queue.
/// \param[in]     mq_id         message queue ID obtained by \ref osMessageQueueNew.
/// \return maximum message size in bytes.
uint32_t osMessageQueueGetMsgSize (osMessageQueueId_t mq_id)
{
    bare_metal_msgq_t *msgq = (bare_metal_msgq_t *)mq_id;
    uint32_t size = 0;

    IRQ_SAVE

    if (msgq) {
        size = msgq->msg_size;
    }

    IRQ_RESTORE

    return size;
}

/// Get number of queued messages in a Message Queue.
/// \param[in]     mq_id         message queue ID obtained by \ref osMessageQueueNew.
/// \return number of queued messages.
uint32_t osMessageQueueGetCount (osMessageQueueId_t mq_id)
{
    bare_metal_msgq_t *msgq = (bare_metal_msgq_t *)mq_id;
    uint32_t count = 0;

    IRQ_SAVE

    if (msgq) {
        count = msgq->msg_tail - msgq->msg_head;
    }

    IRQ_RESTORE

    return count;
}

/// Get number of available slots for messages in a Message Queue.
/// \param[in]     mq_id         message queue ID obtained by \ref osMessageQueueNew.
/// \return number of available slots for messages.
uint32_t osMessageQueueGetSpace (osMessageQueueId_t mq_id)
{
    bare_metal_msgq_t *msgq = (bare_metal_msgq_t *)mq_id;
    uint32_t count = 0;

    IRQ_SAVE

    if (msgq) {
        count = (msgq->msg_count - (msgq->msg_tail - msgq->msg_head));
    }

    IRQ_RESTORE

    return count;
}

/// Reset a Message Queue to initial empty state.
/// \param[in]     mq_id         message queue ID obtained by \ref osMessageQueueNew.
/// \return status code that indicates the execution status of the function.
osStatus_t osMessageQueueReset (osMessageQueueId_t mq_id)
{
    bare_metal_msgq_t *msgq = (bare_metal_msgq_t *)mq_id;
    osStatus_t ret = osError;

    IRQ_SAVE

    if (msgq) {
        msgq->msg_head = 0;
        msgq->msg_tail = 0;
        ret = osOK;
    }

    IRQ_RESTORE

    return ret;
}

/// Delete a Message Queue object.
/// \param[in]     mq_id         message queue ID obtained by \ref osMessageQueueNew.
/// \return status code that indicates the execution status of the function.
osStatus_t osMessageQueueDelete (osMessageQueueId_t mq_id)
{
    bare_metal_msgq_t *msgq = (bare_metal_msgq_t *)mq_id;
    osStatus_t ret = osError;

    ASSERT(!BARE_METAL_IRQ_MODE());

    IRQ_SAVE

    if (msgq) {
        free(msgq->msg_q);
        free(msgq);
        ret = osOK;
    }

    IRQ_RESTORE

    return ret;
}
