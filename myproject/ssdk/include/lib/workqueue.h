/**
 * @file workqueue.h
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#ifndef WORKQUEUE_H_
#define WORKQUEUE_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include <types.h>
#include <cmsis_os2.h>
#include <lib/list.h>

typedef void (*worker_t)(void *arg);

struct work_s {
    struct list_node node;
    worker_t worker;
    void *arg;
};

struct workqueue_s {
    struct list_node list;
    osThreadId_t tid;
    osSemaphoreId_t sem;
};

void start_workqueue_service(void);

int work_queue(struct work_s *work, worker_t worker, void *arg);

#if defined(__cplusplus)
}
#endif

#endif /* WORKQUEUE_H_ */
