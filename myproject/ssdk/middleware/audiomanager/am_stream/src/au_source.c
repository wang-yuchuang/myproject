/**
 * @file au_source.c
 * @brief audio source file.
 *
 * @copyright Copyright (c) 2021  Semidrive Semiconductor.
 *            All rights reserved.
 */

#include <debug.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "au_source.h"

#include "stream_source.h"

#define AM_SOURCE_DBG_PRT SSDK_DEBUG

int au_source_set(au_source_t *source, source_callback call_back,
                  const char *path, au_source_type_t stype)
{
    int ret = 0;

    if (!source) {
        ret = -1;
        ssdk_printf(SSDK_ERR, "%s: source is null\n", __func__);
        return ret;
    }

    switch (stype) {
    case AU_SOURCE_FILE:
        strncpy(source->path, path, AU_SOURCE_PATH_SIZE);
        break;
    case AU_SOURCE_STREAM:
        break;
    default:
        ssdk_printf(SSDK_ERR, "%s: can't support  type(%d)\n", __func__, stype);
        ret = -1;
    }

    source->source_type = stype;

    if (call_back) {
        source->request_data_call_back = call_back;
    }

    return ret;
}

int au_source_open(source_handle_t source_handle)
{
    int ret = 0;
    source_operations_t *source_ops = NULL;

    if (!source_handle) {
        ret = -1;
        ssdk_printf(SSDK_ERR, "%s: source_handle is null\n", __func__);
        return ret;
    }

    switch (source_handle->source_type) {
    case AU_SOURCE_FILE:
        //        source_handle->source_ops = au_get_source_f_ops();
        break;
    case AU_SOURCE_STREAM:
        source_handle->source_ops = au_get_source_stream_ops();
        break;
    default:
        ssdk_printf(SSDK_ERR, "%s: error: not support source_type=%d\n",
                    __func__, source_handle->source_type);
        ret = -1;
    }

    source_ops = source_handle->source_ops;

    if (!source_ops) {
        ret = -1;
        ssdk_printf(SSDK_ERR, "%s: source ops is null\n", __func__);
        return ret;
    }

    if (source_ops->open) {
        ret = source_ops->open(source_handle);
    }

    return ret;
}

int au_source_close(source_handle_t source_handle)
{
    int ret = 0;

    if (!source_handle) {
        ssdk_printf(SSDK_ERR, "%s: source_handle is null\n", __func__);
        ret = -1;
        return ret;
    }

    source_operations_t *source_ops = NULL;
    source_ops = source_handle->source_ops;

    if (!source_ops) {
        ssdk_printf(SSDK_ERR, "%s: source_ops is null\n", __func__);
        ret = -1;
        return ret;
    }

    if (source_ops->close) {
        ret = source_ops->close(source_handle);
    }

    return ret;
}

int au_source_read(source_handle_t source_handle, uint8_t *out, uint16_t len)
{
    int ret = 0;
    source_operations_t *source_ops = NULL;

    if ((!source_handle) || (!out)) {
        ssdk_printf(SSDK_ERR, "%s: source_handle out is null\n", __func__);
        ret = -1;
        return ret;
    }

    source_ops = source_handle->source_ops;

    if (!source_ops) {
        ssdk_printf(SSDK_ERR, "%s: source_ops is null\n", __func__);
        ret = -1;
        return ret;
    }

    if (source_ops->read) {
        ret = source_ops->read(source_handle, out, len);
    }

    return ret;
}

int au_source_seek(source_handle_t source_handle, uint32_t offset)
{
    int ret = 0;
    source_operations_t *source_ops = NULL;

    if (!source_handle) {
        ssdk_printf(SSDK_ERR, "%s: source_handle is null\n", __func__);
        ret = -1;
        return ret;
    }

    source_ops = source_handle->source_ops;

    if (!source_ops) {
        ssdk_printf(SSDK_ERR, "%s: source_ops is null\n", __func__);
        ret = -1;
        return ret;
    }

    if (source_ops->seek) {
        ret = source_ops->seek(source_handle, offset);
    }

    return ret;
}

bool au_source_eof(source_handle_t source_handle)
{
    bool ret = false;
    source_operations_t *source_ops = NULL;

    if (!source_handle) {
        ssdk_printf(SSDK_ERR, "%s: source_handle is null\n", __func__);
        ret = true;
        return ret;
    }

    source_ops = source_handle->source_ops;

    if (!source_ops) {
        ssdk_printf(SSDK_ERR, "%s: source_ops is null\n", __func__);
        ret = true;
        return ret;
    }

    if (source_ops->is_eof) {
        ret = source_ops->is_eof(source_handle);
    }

    return ret;
}
