/**
 * @file stream_source.c
 * @brief stream source file.
 *
 * @copyright Copyright (c) 2021  Semidrive Semiconductor.
 *            All rights reserved.
 */

#include <debug.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stream_source.h"

int stream_source_open(source_handle_t source_handle)
{
    stream_source_t *handle;

    if (!source_handle->request_data_call_back) {
        ssdk_printf(SSDK_ERR, "%s:request_data_call_back is null \n", __func__);
        source_handle->private_data = NULL;
        return -1;
    }

    handle = (stream_source_t *)malloc(sizeof(stream_source_t));

    if (!handle) {
        ssdk_printf(SSDK_ERR, "%s:handle is null \n", __func__);
        source_handle->private_data = NULL;
        return -1;
    }

    memset(handle, 0, sizeof(stream_source_t));
    handle->request_data_callback = source_handle->request_data_call_back;
    handle->callback_param = source_handle->callback_param;
    source_handle->private_data = handle;

    return 0;
}

int stream_source_close(source_handle_t source_handle)
{
    int ret = 0;
    stream_source_t *handle = (stream_source_t *)source_handle->private_data;

    if (!handle) {
        ssdk_printf(SSDK_ERR, "%s:handle is null \n", __func__);
        ret = -1;
        return ret;
    }

    free(handle);
    source_handle->private_data = NULL;

    return ret;
}

int stream_source_seek(source_handle_t source_handle, uint32_t offset)
{
    int ret = 0;
    stream_source_t *handle = (stream_source_t *)source_handle->private_data;

    if (!handle) {
        ssdk_printf(SSDK_ERR, "%s:handle is null \n", __func__);
        ret = -1;
        return ret;
    }

    handle->offset = offset;
    return ret;
}

bool stream_source_eof(source_handle_t source_handle)
{
    bool ret = false;
    stream_source_t *handle = (stream_source_t *)source_handle->private_data;

    if (!handle) {
        ssdk_printf(SSDK_ERR, "%s:handle is null \n", __func__);
        ret = true;
        return ret;
    }

    if (handle->offset >= handle->fsz) {
        ret = true;
    }

    return false;
}

static source_operations_t source_ops_stream = {
    .read = NULL,
    .open = stream_source_open,
    .close = stream_source_close,
    .is_eof = stream_source_eof,
    .seek = stream_source_seek,
};

source_operations_t *au_get_source_stream_ops(void)
{
    return &source_ops_stream;
}
