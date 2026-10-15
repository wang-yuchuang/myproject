/**
 * @file minimp3.c
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#include <cmsis_os2.h>
#include <stdio.h>
#include <string.h>

#include "ff.h"

/*
    the follow 3 MACROS must be defined before incude minimp3.h,
    and the these MACROS can be defined only one time
*/
#define MINIMP3_IMPLEMENTATION
#define MINIMP3_NO_SIMD
#define MINIMP3_ALLOW_MONO_STEREO_TRANSITION
#include "mp3.h"

#define SAMPLE_PER_FRAME 1152 /* the max samples of per frame */

static inline size_t mp3_read_cb(void *buff, size_t nbyte, void *user_data)
{
    size_t ret = 0;
    mp3_dec_context *context = (mp3_dec_context *)user_data;

    if (context->src_type == MP3_IO_SOURCE_DISK) {
        uint8_t *temp_buff = osAllocAlign(32, nbyte);
        /*
           wrap the f_read here, a direct read may failed, because
           the decoder may read at any address not aligned with 32
        */
        f_read((FIL *)context->bit_stream, temp_buff, nbyte, &ret);
        memcpy(buff, temp_buff, ret);
        osFree(temp_buff);
    } else {
        if (context->bits_offset + nbyte > context->bit_size) {
            nbyte = context->bit_size - context->bits_offset;
        }

        memcpy(buff, (uint8_t *)context->bit_stream + context->bits_offset, nbyte);
        context->bits_offset += nbyte;
        ret = nbyte;
    }

    return ret;
}

static inline int mp3_seek_cb(uint64_t position, void *user_data)
{
    int ret = 0;
    mp3_dec_context *context = (mp3_dec_context *)user_data;

    if (position > context->bit_size) {
        position = context->bit_size;
    }

    if (context->src_type == MP3_IO_SOURCE_DISK) {
        ret = f_lseek((FIL *)context->bit_stream, position);
    } else {
        context->bits_offset = position;
    }

    return ret;
}

mp3_dec_context *mp3_init_dec(io_source_type type, void *data, uint32_t size)
{
    mp3_dec_context *context = NULL;
    if (!data)
        return NULL;

    context = (mp3_dec_context *)osAlloc(sizeof(mp3_dec_context));
    if (!context)
        return NULL;

    memset(context, 0x00, sizeof(mp3_dec_context));
    context->src_type = type;
    context->bits_offset = 0;

    if (context->src_type == MP3_IO_SOURCE_C_ARRAY) {
        // input data is stored in the ram already
        context->bit_stream = (uint8_t *)data;
        context->bit_size = size;
    } else if (context->src_type == MP3_IO_SOURCE_DISK_PRELOAD) {
        FIL fp = {0};
        size_t count = 0;
        FRESULT rc = FR_OK;
        char file_name[128] = {0};

        strcpy(file_name, (char *)data);

        rc = f_open(&fp, file_name, FA_READ);
        if (rc != FR_OK) {
            osFree(context);
            return NULL;
        }

        context->bit_size = f_size(&fp);
        context->bit_stream = (uint8_t *)osAllocAlign(32, context->bit_size);
        f_read(&fp, context->bit_stream, context->bit_size, &count);
        f_close(&fp);

        if (count != context->bit_size) {
            osFree(context->bit_stream);
            osFree(context);
            return NULL;
        }
    } else if (context->src_type == MP3_IO_SOURCE_DISK) {
        FRESULT rc = FR_OK;
        char file_name[128] = {0};
        strcpy(file_name, (char *)data);

        context->bit_stream = osAlloc(sizeof(FIL));
        rc = f_open((FIL *)context->bit_stream, file_name, FA_READ);
        if (rc != FR_OK) {
            osFree(context->bit_stream);
            osFree(context);
            return NULL;
        }

        context->bit_size = f_size((FIL *)context->bit_stream);
    }

    return context;
}

int mp3_open_dec(mp3_dec_context *context)
{
    if (!context)
        return -1;

    context->io_context.read = mp3_read_cb;
    context->io_context.seek = mp3_seek_cb;
    context->io_context.read_data = context;
    context->io_context.seek_data = context;

    mp3dec_ex_open_cb(&context->dec_context, &context->io_context,
                      MP3D_SEEK_TO_SAMPLE);

    context->duration =
        (context->dec_context.samples * 1000) /
        (context->dec_context.info.channels * context->dec_context.info.hz);

    return 0;
}

int mp3_close_dec(mp3_dec_context *context)
{
    if (!context)
        return -1;

    if (context->src_type == MP3_IO_SOURCE_DISK) {
        f_close((FIL *)context->bit_stream);
        osFree(context->bit_stream);
    } else if (context->src_type == MP3_IO_SOURCE_DISK_PRELOAD)
        osFree(context->bit_stream);

    mp3dec_ex_close(&context->dec_context);
    osFree(context);

    return 0;
}

int mp3_dec_get_frame_info(mp3_dec_context *context, mp3_frame_info *frame_info)
{
    if (!context || !frame_info)
        return -1;

    frame_info->channels = context->dec_context.info.channels;
    frame_info->bitrate = context->dec_context.info.bitrate_kbps;
    frame_info->samples = frame_info->channels * SAMPLE_PER_FRAME;
    frame_info->duration = context->duration;

#ifndef MINIMP3_FLOAT_OUTPUT
    frame_info->format = MP3_SAMPLE_FMT_S16;
#else
    frame_info->format = MP3_SAMPLE_FMT_FLT;
#endif

    return 0;
}

int mp3_dec_seek_frame(mp3_dec_context *context, uint64_t position)
{
    int ret = 0;
    uint64_t sample_position = 0;

    if (!context)
        return -1;

    // convert timestamp to sample position
    sample_position =
        context->dec_context.samples * position / context->duration;
    ret = mp3dec_ex_seek(&context->dec_context, sample_position);

    return ret;
}

int mp3_dec_receive_frame(mp3_dec_context *context, mp3d_sample_t *buf,
                          uint32_t samples)
{
    size_t readed = 0;

    if (!context)
        return -1;

    readed = mp3dec_ex_read(&context->dec_context, buf, samples);

    return readed;
}
