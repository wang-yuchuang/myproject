/**
 * @file ringbuffer.c
 * @brief ring buffer file.
 *
 * @copyright Copyright (c) 2021  Semidrive Semiconductor.
 *            All rights reserved.
 */
#include <malloc.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "ringbuffer.h"

void ringbuf_create(ringbuf_t *rb, uint32_t size)
{
    void *buf = memalign(32, size);
    ringbuf_initialize_etc(rb, size, buf);
}

void ringbuf_initialize_etc(ringbuf_t *rb, uint32_t len, void *buf)
{

    if (!rb)
        return;

    rb->head = 0;
    rb->tail = 0;
    rb->used = 0;
    rb->size = len;
    rb->buf = buf;
    if (!rb->buf) {
        //free(rb);
        return ;
    }
    rb->lock = osMutexNew(NULL);
    return;
}

void ringbuf_destroy(ringbuf_t *rb)
{
    if (rb->buf) {
        free(rb->buf);
    }
    osMutexDelete(rb->lock);
    memset(rb, 0, sizeof(ringbuf_t));
}

void ringbuf_destroy_etc(ringbuf_t *rb)
{
    osMutexDelete(rb->lock);
    memset(rb, 0, sizeof(ringbuf_t));
}
void ringbuf_clear(ringbuf_t *rb)
{
    rb->head = 0;
    rb->tail = 0;
    rb->used = 0;
}

uint32_t ringbuf_get_available(ringbuf_t *rb)
{
    uint32_t available = 0;
    available = rb->size - rb->used;
    return available;
}

uint32_t ringbuf_get_used(ringbuf_t *rb) { return rb->used; }

uint32_t ringbuf_write(ringbuf_t *rb, uint8_t *data, uint32_t size)
{
    osMutexAcquire(rb->lock, osWaitForever);
    uint32_t remaining = rb->size - rb->tail;

    if (size > (rb->size - rb->used)) {
        /* Overflow - cap write size to available space. */
        size = rb->size - rb->used;
    }

    /* Split write into two if it will overflow. */
    if (size > remaining) {
        /* First copy to end of buffer. */
        memcpy(rb->buf + rb->tail, data, remaining);
        /* Second copy from start of buffer, remaining size. */
        memcpy(rb->buf, data + remaining, size - remaining);
        /* Set write point to 0 + <second copy size>. */
        rb->tail = size - remaining;
    } else {
        memcpy(rb->buf + rb->tail, data, size);
        rb->tail += size;
    }

    rb->used += size;
    osMutexRelease(rb->lock);
    return size;
}

static uint32_t _ringbuf_read_internal(ringbuf_t *rb, uint8_t *data,
                                       uint32_t size, bool peek)
{
    osMutexAcquire(rb->lock, osWaitForever);
    uint32_t remaining = rb->size - rb->head;

    if (size > rb->used) {
        /* Underrun - cap read at max available. */
        size = rb->used;
    }

    if (size > remaining) {
        if (data) {
            /* First copy to end of buffer. */
            memcpy(data, rb->buf + rb->head, remaining);
            /* Second copy from start of buffer, remaining size. */
            memcpy(data + remaining, rb->buf, size - remaining);
        }

        if (!peek) {
            rb->head = size - remaining;
        }
    } else {
        if (data) {
            memcpy(data, rb->buf + rb->head, size);
        }

        if (!peek) {
            rb->head += size;
        }
    }

    if (!peek) {
        rb->used -= size;
    }
    osMutexRelease(rb->lock);
    return size;
}

uint32_t ringbuf_read(ringbuf_t *rb, uint8_t *data, uint32_t size)
{
    return _ringbuf_read_internal(rb, data, size, false);
}

uint32_t ringbuf_peek(ringbuf_t *rb, uint8_t *data, uint32_t size)
{
    return _ringbuf_read_internal(rb, data, size, true);
}
