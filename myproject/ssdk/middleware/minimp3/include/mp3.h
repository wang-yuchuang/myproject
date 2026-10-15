/**
 * @file mp3.h
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */
#ifndef MP3_H
#define MP3_H

#include "minimp3_ex.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    MP3_IO_SOURCE_C_ARRAY = 0,  /* the input stream is in the ram already*/
    MP3_IO_SOURCE_DISK_PRELOAD, /* the input stream is in file system, but we
                                   will load the whole file to ram once time */
    MP3_IO_SOURCE_DISK /* the input stream is in file system, and we will load
                          the file when needed */
} io_source_type;

typedef enum {
    MP3_SAMPLE_FMT_S16 = 0, /* signed 16 bits */
    MP3_SAMPLE_FMT_FLT,     /* float */
} sample_format;

typedef struct {
    io_source_type src_type;
    void *bit_stream; /* input bs buffer*/
    uint32_t bit_size;
    uint32_t bits_offset;
    uint64_t duration; /* duration with ms */

    mp3dec_ex_t dec_context;
    mp3dec_io_t io_context;
} mp3_dec_context;

typedef struct {
    uint8_t channels;
    uint32_t bitrate;  /* kbps */
    uint32_t samples;  /* samples per frame*/
    uint64_t duration; /* duration with ms */

    sample_format format; /* sample format */
} mp3_frame_info;

int mp3_open_dec(mp3_dec_context *context);
int mp3_close_dec(mp3_dec_context *context);
int mp3_dec_seek_frame(mp3_dec_context *context, uint64_t position);
mp3_dec_context *mp3_init_dec(io_source_type type, void *data, uint32_t size);
int mp3_dec_get_frame_info(mp3_dec_context *context,
                           mp3_frame_info *frame_info);
int mp3_dec_receive_frame(mp3_dec_context *context, mp3d_sample_t *buf,
                          uint32_t samples);

#ifdef __cplusplus
}
#endif

#endif /*MP3_H*/