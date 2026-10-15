/**
 * @file ring_buffer.c
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: ring buffer function.
 *
 * Revision History:
 * -----------------
 */

#include <string.h>
#include <param.h>
#include <cmsis_os2.h>
#include <arch/barriers.h>
#include <debug.h>

#include <lib/ring_buffer.h>

/**
 * @brief Create the ring buffer.
 *
 * @param[in] buffer_size The size of the buffer in bytes.
 * @return ring buffer object.
 */
ring_buffer_t *ring_buffer_create(uint32_t buffer_size)
{
    ring_buffer_t *ring_buffer;

    ring_buffer = (ring_buffer_t *)osAlloc(sizeof(ring_buffer_t) + buffer_size);
    if (ring_buffer) {
        ring_buffer->r_idx = 0;
        ring_buffer->w_idx = 0;
        ring_buffer->size = buffer_size;
    }

    return ring_buffer;
}

/**
 * @brief Destroy the ring buffer.
 *
 * @param[in] ring_buffer ring buffer object.
 */
void ring_buffer_destroy(ring_buffer_t *ring_buffer)
{
    if (ring_buffer) {
        osFree(ring_buffer);
    }
}

/**
 * @brief Put data into the ring buffer.
 *
 * @param[in] ring_buffer ring buffer object.
 * @param[in] data data object.
 * @param[in] size data size.
 * @return data size we put into the ring buffer.
 */
uint32_t ring_buffer_put(ring_buffer_t *ring_buffer, uint8_t *data, uint32_t size)
{
    uint32_t r_idx_mask, w_idx_mask;
    uint32_t first_len;

    ASSERT(ring_buffer);
    ASSERT(data);

    if (size > RING_BUFFER_UNUSED(ring_buffer)) {
        return 0;
    }

    r_idx_mask = ring_buffer->r_idx & RING_BUFFER_MASK(ring_buffer);
    w_idx_mask = ring_buffer->w_idx & RING_BUFFER_MASK(ring_buffer);

    if (r_idx_mask <= w_idx_mask) {
        first_len = ring_buffer->size - w_idx_mask;
    }
    else {
        first_len = r_idx_mask - w_idx_mask;
    }

    if (size <= first_len) {
        memcpy(&ring_buffer->data[w_idx_mask], data, size);
    }
    else {
        memcpy(&ring_buffer->data[w_idx_mask], data, first_len);
        memcpy(ring_buffer->data, data + first_len, size - first_len);
    }

    DSB;

    ring_buffer->w_idx += size;

    return size;
}

/**
 * @brief Get data from the ring buffer.
 *
 * @param[in] ring_buffer ring buffer object.
 * @param[in] data data object.
 * @param[in] size data size.
 * @return data size we get from the ring buffer.
 */
uint32_t ring_buffer_get(ring_buffer_t *ring_buffer, uint8_t *data, uint32_t size)
{
    uint32_t r_idx_mask, w_idx_mask;
    uint32_t actual_len;
    uint32_t first_len;

    ASSERT(ring_buffer);
    ASSERT(data);

    if (RING_BUFFER_USED(ring_buffer) == 0) {
        return 0;
    }

    actual_len = MIN(size, RING_BUFFER_USED(ring_buffer));

    r_idx_mask = ring_buffer->r_idx & RING_BUFFER_MASK(ring_buffer);
    w_idx_mask = ring_buffer->w_idx & RING_BUFFER_MASK(ring_buffer);

    if (w_idx_mask <= r_idx_mask) {
        first_len = ring_buffer->size - r_idx_mask;
    }
    else {
        first_len = w_idx_mask - r_idx_mask;
    }

    if (actual_len <= first_len) {
        memcpy(data, &ring_buffer->data[r_idx_mask], actual_len);
    }
    else {
        memcpy(data, &ring_buffer->data[r_idx_mask], first_len);
        memcpy(data + first_len, ring_buffer->data, actual_len - first_len);
    }

    DSB;

    ring_buffer->r_idx += actual_len;

    return actual_len;
}
