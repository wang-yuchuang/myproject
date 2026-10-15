/**
 * @file ering_buffer.c
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: ering buffer function.
 *
 * Revision History:
 * -----------------
 */

#include <string.h>

#include <lib/ering_buffer.h>

/**
 * @brief Create the ring buffer.
 *
 * @param[in] buffer_size The size of the buffer in bytes.
 * @return ring buffer object.
 */
ering_buffer_t *ering_buffer_create(uint32_t buffer_size)
{
    return ring_buffer_create(buffer_size);
}

/**
 * @brief Destroy the ring buffer.
 *
 * @param[in] ring_buffer ring buffer object.
 */
void ering_buffer_destroy(ering_buffer_t *ering_buffer)
{
    ring_buffer_destroy(ering_buffer);
}

/**
 * @brief Put data into the ring buffer.
 *
 * @param[in] ring_buffer ring buffer object.
 * @param[in] data data object.
 * @param[in] size data size.
 * @return data size we put into the ring buffer.
 */
int ering_buffer_put(ering_buffer_t *ering_buffer, uint8_t *data, uint32_t size)
{
    ering_buffer_header_t header;

    if ((size + sizeof(ering_buffer_header_t)) > RING_BUFFER_UNUSED(ering_buffer)) {
        return -1;
    }

    header.magic = ERING_BUFFER_MAGIC;
    header.data_len = size;

    ring_buffer_put(ering_buffer, (uint8_t *)&header,
                    sizeof(ering_buffer_header_t));
    ring_buffer_put(ering_buffer, data, size);
    return 0;
}

/**
 * @brief Get data from the ring buffer.
 *
 * @param[in] ring_buffer ring buffer object.
 * @param[in] data data object.
 * @param[in] size data size.
 * @return data size we get from the ring buffer.
 */
int ering_buffer_get(ering_buffer_t *ering_buffer, uint8_t *data, uint32_t *size)
{
    ering_buffer_header_t header;
    uint32_t len;

    len = ring_buffer_get(ering_buffer, (uint8_t *)&header,
                          sizeof(ering_buffer_header_t));

    if ((len == 0) || (header.magic != ERING_BUFFER_MAGIC)) {
        return -1;
    }

    ring_buffer_get(ering_buffer, data, header.data_len);
    *size = header.data_len;
    return 0;
}
