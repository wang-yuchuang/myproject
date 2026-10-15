/**
 * @file file_source.h
 * @brief file source header file.
 *
 * @copyright Copyright (c) 2021  Semidrive Semiconductor.
 *            All rights reserved.
 */

#ifndef __AU_FILE_SOURCE_H__
#define __AU_FILE_SOURCE_H__

#include <ff.h>
#include <stdint.h>
#include <stdlib.h>

#include "au_source.h"
#include "ringbuffer.h"

typedef struct f_source *f_source_handle_t;

typedef struct f_source {
    char fpath[AU_SOURCE_PATH_SIZE];
    uint32_t fsz;
    uint32_t offset;
} f_source_t;

source_operations_t *au_get_source_f_ops(void);

#endif
