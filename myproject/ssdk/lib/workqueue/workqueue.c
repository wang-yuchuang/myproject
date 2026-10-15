/**
 * @file workqueue.c
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#include <debug.h>
#include <lib/workqueue.h>
#include <arch/spinlock.h>
#include <arch/irq.h>

static bool g_inited;
static struct workqueue_s g_workqueue;

static void workqueue_handler_thread(void *arg)
{
    irq_state_t state;
    worker_t worker;
    void *worker_arg;
    struct work_s *work_s;

    while (1) {
        osSemaphoreAcquire(g_workqueue.sem, osWaitForever);

        state = arch_irq_save();
        work_s = list_remove_head_type(&g_workqueue.list, struct work_s, node);

        while (work_s) {
            if (work_s->worker) {
                worker = work_s->worker;
                worker_arg = work_s->arg;
                work_s->worker = NULL;
                arch_irq_restore(state);
                worker(worker_arg);
                state = arch_irq_save();
            }

            work_s = list_remove_head_type(&g_workqueue.list, struct work_s, node);
        }

        arch_irq_restore(state);
    }
}

int work_queue(struct work_s *work, worker_t worker, void *arg)
{
    irq_state_t state;

    ASSERT(work);

    state = arch_irq_save();

    if (work->worker) {
        list_delete(&work->node);
    }

    work->worker = worker;
    work->arg = arg;
    list_add_tail(&g_workqueue.list, &work->node);

    arch_irq_restore(state);
    osSemaphoreRelease(g_workqueue.sem);
    return 0;
}

void start_workqueue_service(void)
{
    osThreadAttr_t attr = {
        .name = "workqueue",
        .priority = 9,
    };

    if (!g_inited) {
        g_inited = true;
        list_initialize(&g_workqueue.list);
        g_workqueue.sem = osSemaphoreNew(10, 0, NULL);
        g_workqueue.tid = osThreadNew(workqueue_handler_thread, NULL, &attr);
    }
}
