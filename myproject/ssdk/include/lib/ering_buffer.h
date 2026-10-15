/**
 * @file ering_buffer.h
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: ering buffer header.
 *
 * Revision History:
 * -----------------
 */

#ifndef ERING_BUFFER
#define ERING_BUFFER

#include <types.h>
#include <stdlib.h>
#include <lib/ring_buffer.h>

#define ERING_BUFFER_MAGIC          0xfdfdfdfd

typedef ring_buffer_t ering_buffer_t;

typedef struct ering_buffer_header {
    uint32_t magic;
    uint32_t data_len;
} ering_buffer_header_t;

/**
 * @brief Create the ring buffer.
 *
 * @param[in] buffer_size The size of the buffer in bytes.
 * @return ring buffer object.
 */
ering_buffer_t *ering_buffer_create(uint32_t buffer_size);


/**
 * @brief Destroy the ring buffer.
 *
 * @param[in] ring_buffer ring buffer object.
 */
void ering_buffer_destroy(ering_buffer_t *ering_buffer);

/**
 * @brief Put data into the ring buffer.
 *
 * @param[in] ring_buffer ring buffer object.
 * @param[in] data data object.
 * @param[in] size data size.
 * @return result.
 */
int ering_buffer_put(ering_buffer_t *ering_buffer, uint8_t *data, uint32_t size);

/**
 * @brief Get data from the ring buffer.
 *
 * @param[in] ring_buffer ring buffer object.
 * @param[in] data data object.
 * @param[out] size data size.
 * @return result.
 */
int ering_buffer_get(ering_buffer_t *ering_buffer, uint8_t *data, uint32_t *size);

#endif
