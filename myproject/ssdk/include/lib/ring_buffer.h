/**
 * @file ring_buffer.h
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: ring buffer header.
 *
 * Revision History:
 * -----------------
 */

#ifndef RING_BUFFER
#define RING_BUFFER

#include <stdlib.h>

#define RING_BUFFER_USED(ring_buffer)       (ring_buffer->w_idx - ring_buffer->r_idx)
#define RING_BUFFER_UNUSED(ring_buffer)     (ring_buffer->size - RING_BUFFER_USED(ring_buffer))
#define RING_BUFFER_MASK(ring_buffer)       (ring_buffer->size - 1)

typedef struct ring_buffer {
    uint32_t r_idx;
    uint32_t w_idx;
    uint32_t size;
    uint8_t data[0];
} ring_buffer_t;

/**
 * @brief Create the ring buffer.
 *
 * @param[in] buffer_size The size of the buffer in bytes.
 * @return ring buffer object.
 */
ring_buffer_t *ring_buffer_create(uint32_t buffer_size);


/**
 * @brief Destroy the ring buffer.
 *
 * @param[in] ring_buffer ring buffer object.
 */
void ring_buffer_destroy(ring_buffer_t *ring_buffer);

/**
 * @brief Put data into the ring buffer.
 *
 * @param[in] ring_buffer ring buffer object.
 * @param[in] data data object.
 * @param[in] size data size.
 * @return data size we put into the ring buffer.
 */
uint32_t ring_buffer_put(ring_buffer_t *ring_buffer, uint8_t *data, uint32_t size);

/**
 * @brief Get data from the ring buffer.
 *
 * @param[in] ring_buffer ring buffer object.
 * @param[in] data data object.
 * @param[in] size data size.
 * @return data size we get from the ring buffer.
 */
uint32_t ring_buffer_get(ring_buffer_t *ring_buffer, uint8_t *data, uint32_t size);

#endif
