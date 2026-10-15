/**
 * @file au_source.h
 * @brief audio source header file.
 *
 * @copyright Copyright (c) 2021  Semidrive Semiconductor.
 *            All rights reserved.
 */

#ifndef __AU_SOURCE_H__
#define __AU_SOURCE_H__

#include <stdint.h>
#include <stdlib.h>

#define AU_SOURCE_PATH_SIZE 200

typedef enum au_source_type {
    AU_SOURCE_FILE = 0,
    AU_SOURCE_STREAM = 1,
} au_source_type_t;

typedef struct au_source *source_handle_t;
typedef struct au_pcm_stream *pcm_stream_handle_t;

typedef int (*source_callback)(pcm_stream_handle_t handle, int32_t size,
                               void *param);

typedef struct source_operations {
    int (*seek)(source_handle_t handle, uint32_t offset);
    int (*read)(source_handle_t handle, uint8_t *out, uint16_t len);
    int (*open)(source_handle_t handle);
    int (*close)(source_handle_t handle);
    bool (*is_eof)(source_handle_t handle);
} source_operations_t;

typedef struct au_source {
    au_source_type_t source_type;
    char path[AU_SOURCE_PATH_SIZE];
    pcm_stream_handle_t pcm_stream;
    source_callback request_data_call_back;
    void *callback_param;
    source_operations_t *source_ops;
    void *private_data;
} au_source_t;

int au_source_open(source_handle_t source_handle);
int au_source_close(source_handle_t source_handle);
int au_source_read(source_handle_t source_handle, uint8_t *out, uint16_t len);
int au_source_seek(source_handle_t source_handle, uint32_t offset);
bool au_source_eof(source_handle_t source_handle);

int au_source_set(au_source_t *source, source_callback call_back,
                  const char *path, au_source_type_t stype);

#endif
