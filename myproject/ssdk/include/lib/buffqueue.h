/**
 * @file buffqueue.h
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#ifndef BUFFQUEUE_H_
#define BUFFQUEUE_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include <types.h>

typedef void *buffqueue_t;

buffqueue_t buffqueue_create(uint32_t size, uint32_t num);

void *buffqueue_alloc_buffer(buffqueue_t bq, uint32_t *len, uint16_t *idx);

void buffqueue_enqueue(buffqueue_t bq, uint16_t idx);

void *buffqueue_dequeue(buffqueue_t bq, uint32_t *len, uint16_t *idx);

void buffqueue_free_buffer(buffqueue_t bq, uint16_t idx);

void buffqueue_destroy(buffqueue_t bq);

#if defined(__cplusplus)
}
#endif

#endif /* BUFFQUEUE_H_ */
