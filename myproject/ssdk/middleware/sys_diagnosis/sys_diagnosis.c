/**
 * @file sys_diagnosis.c
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: system error handle
 *
 * Revision History:
 * -----------------
 */
/* =========================== include file session ======================== */
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <cmsis_os2.h>
#include <stdarg.h>
#include <driver/irq.h>
#include <irq_num.h>
#include "sys_diagnosis.h"

#ifdef CONFIG_KUNLUN_SEM
#include <driver/kunlun/sem/sdrv_sem_drv.h>
#endif /* end CONFIG_KUNLUN_SEM */

/* =========================== marco define session ======================== */
#ifdef CONFIG_BOARD_X9SP
#define MAX_SYSD_SIGNALS    477
#else
#define MAX_SYSD_SIGNALS    319
#endif
#define SYSD_WORKER_STACK_SZ (4 * 1024)
#define SYS_ERR_MGR_DETEC_SIGNAL    0x1

/* =========================== enumeration define session ================== */
/* =========================== structure define session ==================== */
typedef struct sysd_handler_t {
    struct sysd_handler_t* next;
    struct sysd_handler_t* prev;
    sys_event_cb_t evt_cb;
    void* args;
}sysd_handler_t;

typedef struct {
    osEventFlagsId_t evt;
    osMutexId_t lock;
    enum sem sem;
    uint16_t irq;
}sem_handle_t;

typedef struct sysd_list_t {
    int chain_len;
    sysd_handler_t* head;
    sysd_handler_t* tail;
}sysd_list_t;

/* =========================== function declare session ==================== */
void* malloc(size_t size);

/* =========================== static value define session ================= */
static sysd_list_t total_signals[MAX_SYSD_SIGNALS];

static sem_handle_t sem_handle;

/* =========================== function implementation session ============= */
static void sysd_signal_walkthrough(int signal)
{
    assert(signal < MAX_SYSD_SIGNALS);

    sysd_handler_t* sysd = total_signals[signal].head;

    while (sysd) {
        if (sysd->evt_cb) {
            sysd->evt_cb(signal,sysd->args);
        }
        sysd = sysd->next;
    }
}

static void sysd_add_handler(int signal, sysd_handler_t* handler, int tail)
{
    assert(signal < MAX_SYSD_SIGNALS);

    sysd_list_t* list = (sysd_list_t*)&total_signals[signal];

    if (list->head == NULL && list->tail == NULL) {
        //first added
        list->head = list->tail = handler;
        handler->prev = handler->next = NULL;
    }
    else {
        if (!tail) {
            handler->prev = NULL;
            handler->next = list->head;
            list->head->prev = handler;
            list->head = handler;
        }
        else {
            handler->next = NULL;
            handler->prev = list->tail;
            list->tail->next = handler;
            list->tail = handler;
        }
    }

    list->chain_len++;
}

int sysd_register_handler(sys_event_cb_t cb, void* args, uint32_t n,...)
{
    va_list ap;
    int signal = NULL_SIGNAL;
    va_start(ap,n);

    sysd_handler_t* phandler = NULL;

    osMutexAcquire(sem_handle.lock, osWaitForever);

    for (int i = 0; i < n; i++) {
        signal = va_arg(ap,int);

        if (signal == NULL_SIGNAL) {
            continue;
        }
        else {
            phandler = malloc(sizeof(sysd_handler_t));
            if (!phandler) {
                va_end(ap);
                osMutexRelease(sem_handle.lock);
                return -1;
            }
            else {
                phandler->args = args;
                phandler->evt_cb = cb;
                sysd_add_handler(signal,phandler, 0);
                sem_map_signal(sem_handle.sem, signal, SEM_INTR_CPU, true);
            }
        }
    }

    va_end(ap);
    osMutexRelease(sem_handle.lock);

    return 0;
}

int sysd_register_daemon_handler(sys_event_cb_t cb, void* args, int signal)
{
    sysd_handler_t* phandler = NULL;

    osMutexAcquire(sem_handle.lock, osWaitForever);

    phandler = malloc(sizeof(sysd_handler_t));
    if (!phandler) {
        osMutexRelease(sem_handle.lock);
        return -1;
    }
    else {
        phandler->args = args;
        phandler->evt_cb = cb;
        sysd_add_handler(signal,phandler, 1);
        sem_map_signal(sem_handle.sem, signal, SEM_INTR_CPU, true);
        osMutexRelease(sem_handle.lock);
        return 0;
    }
}

static int32_t sysd_sem_handler(uint32_t irq, void *arg)
{
    sem_enable_intr(sem_handle.sem, SEM_INTR_CPU, false);
    osEventFlagsSet(sem_handle.evt, SYS_ERR_MGR_DETEC_SIGNAL);

    return 0;
}

#ifdef CONFIG_FuncSafetyPkg
extern void Sem_Irq(void);
extern unsigned char gFuncSafetyFinishedInit;
#endif
static void sysd_dispatcher(void)
{
    for (int s=0; s < MAX_SYSD_SIGNALS; s++) {
        if (sem_signal_status(sem_handle.sem, s)) {
            sysd_signal_walkthrough(s);
        }
    }
    #ifdef CONFIG_FuncSafetyPkg
    if (gFuncSafetyFinishedInit == 1) {
        Sem_Irq();
    }
    #endif
}

static void sysd_worker(enum sem sem)
{
    sem_handle.sem = sem;
#ifdef CONFIG_BOARD_X9SP
    sem_handle.irq = ((sem == SEM1) ? SEM1_O_SEM_INT_CPU_NUM : SEM2_O_SEM_INT_CPU_NUM);
#else
    sem_handle.irq = ((sem == SEM1) ? SEM1_O_SEM_INTR_NUM : SEM2_O_SEM_INTR_NUM);
#endif

    sem_handle.evt = osEventFlagsNew(NULL);
    sem_handle.lock = osMutexNew(NULL);

    osMutexAcquire(sem_handle.lock, osWaitForever);

    for(int s=0; s < MAX_SYSD_SIGNALS; s++) {
        sem_map_signal(sem_handle.sem, s, SEM_INTR_CPU, false);
    }

    irq_attach(sem_handle.irq, sysd_sem_handler, NULL);
    irq_enable(sem_handle.irq);
    sem_enable_intr(sem_handle.sem, SEM_INTR_CPU, true);

    osMutexRelease(sem_handle.lock);

    while (1) {
        osEventFlagsWait(sem_handle.evt, SYS_ERR_MGR_DETEC_SIGNAL, osFlagsWaitAny, osWaitForever);

        osMutexAcquire(sem_handle.lock, osWaitForever);
        sysd_dispatcher();
        sem_enable_intr(sem_handle.sem, SEM_INTR_CPU, true);
        osMutexRelease(sem_handle.lock);
    }
}

void sysd_start(enum sem sem)
{
    osThreadAttr_t attr;
    memset(&attr, 0x00, sizeof(attr));

    attr.stack_size = SYSD_WORKER_STACK_SZ;
    attr.name = "sysd";
    attr.priority = 9;

    osThreadNew((osThreadFunc_t)sysd_worker, (void *)sem, &attr);
}


/* end of file sys_diagnosis.c */