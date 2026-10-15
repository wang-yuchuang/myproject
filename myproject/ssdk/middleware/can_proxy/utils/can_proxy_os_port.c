/*
 * can proxy os port layer.
 * Copyright (c) 2020 Semidrive Semiconductor, Inc.
 * All rights reserved.
 *
 * User should placed appropriate os implementation
 * here to adapt different os.
 */
#include <stdlib.h>
#include <stddef.h>
#include <malloc.h>
#include <cmsis_os2.h>

extern uint32_t ulPortInterruptNesting;

void *os_msg_buf_create(size_t buffer_count, size_t buffer_size)
{
    return osMessageQueueNew(buffer_count, buffer_size, NULL);
}

/* variable length msg post function. Should support interrupt context. */
size_t os_msg_buf_post(void *handle, void *data, size_t len)
{
    osStatus_t status;
    status = osMessageQueuePut(handle, data, 0, 0);
    return status == osOK ? 1 : 0;
}

size_t os_msg_buf_get(void *handle, void *data, size_t len)
{
    osStatus_t status;
    status = osMessageQueueGet(handle, data, NULL, osWaitForever);
    return status == osOK ? 1 : 0;
}