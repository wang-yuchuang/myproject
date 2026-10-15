/**
 * @file stream_source.h
 * @brief stream source header file.
 *
 * @copyright Copyright (c) 2021  Semidrive Semiconductor.
 *            All rights reserved.
 */

#ifndef __AU_STREAM_SOURCE_H__
#define __AU_STREAM_SOURCE_H__

#include <stdint.h>
#include <stdlib.h>

#include "au_source.h"
#include "ringbuffer.h"

typedef struct stream_source *stream_source_handle_t;

typedef struct stream_source {
    source_callback request_data_callback;
    void *callback_param;
    uint32_t fsz;
    uint32_t offset;
} stream_source_t;

source_operations_t *au_get_source_stream_ops(void);

#endif
