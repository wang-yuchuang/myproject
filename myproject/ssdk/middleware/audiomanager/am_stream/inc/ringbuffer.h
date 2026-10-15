/**
 * @file ringbuffer.h
 * @brief ringbuffer header file.
 *
 * @copyright Copyright (c) 2021  Semidrive Semiconductor.
 *            All rights reserved.
 */

#ifndef __RINGBUFFER_H__
#define __RINGBUFFER_H__
#include <cmsis_os2.h>
#include <stdint.h>

struct ringbuf {
    uint8_t *buf;
    volatile uint32_t head; /* read ptr */
    volatile uint32_t tail; /* write ptr */
    uint32_t size;
    volatile uint32_t used;
    osMutexId_t lock;
};

typedef struct ringbuf ringbuf_t;


void ringbuf_create(ringbuf_t *rb, uint32_t size);
void ringbuf_destroy(ringbuf_t *rb);
/* create by ringbuf_initialize_etc don't distroy */
void ringbuf_initialize_etc(ringbuf_t *rb, uint32_t len, void *buf);
void ringbuf_destroy_etc(ringbuf_t *rb);
void ringbuf_clear(ringbuf_t *rb);
uint32_t ringbuf_get_used(ringbuf_t *rb);
uint32_t ringbuf_get_available(ringbuf_t *rb);
uint32_t ringbuf_write(ringbuf_t *rb, uint8_t *data, uint32_t size);
uint32_t ringbuf_read(ringbuf_t *rb, uint8_t *data, uint32_t size);
uint32_t ringbuf_peek(ringbuf_t *rb, uint8_t *data, uint32_t size);

#endif /* _RINGBUFFER_H_ */
