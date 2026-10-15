/**
 * @file mjpeg.h
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */
#ifndef MJPEG_H
#define MJPEG_H

#include "tjpgd.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MJPEG_WORKBUFF_SIZE (4096 * 4) // Recommended by TJPGD libray

typedef enum {
    JPEG_IO_SOURCE_C_ARRAY = 0,
    JPEG_IO_SOURCE_DISK,
} io_source_type;

typedef enum {
    JDF_444 = 0,
    JDF_422,
    JDF_420,
} jformat;

/* session identifier for input/output functions (name, members and usage are as
 * user defined) */
typedef struct {
    io_source_type src_type;
    uint8_t *bit_stream; /* input bs buffer*/
    uint32_t bit_size;
    uint32_t bits_offset;
    uint8_t first_parse;

    void *work_buf;
    JDEC jdec;
} jpg_session;

typedef struct {
    int x;
    int y;
    int w;
    int h;
} mjpeg_rect;

typedef struct {
    uint32_t width;
    uint32_t height;
    jformat format;
    mjpeg_rect crop;
} jpg_out_info;

jpg_session *mjpeg_dec_init(io_source_type type, const void *data,
                            uint32_t size);
JRESULT mjpeg_dec_get_header_info(jpg_session *dec_session,
                                  jpg_out_info *out_info);
JRESULT mjpeg_dec_start_decompress(jpg_session *dec_session);
JRESULT mjpeg_dec_parser_next_frame(jpg_session *dec_session);
JRESULT mjpeg_dec_process_frame(jpg_session *dec_session, uint8_t *buf,
                                uint32_t y_size, uint32_t u_size);
void mjpeg_dec_deinit(jpg_session *dec_session);

#ifdef __cplusplus
}
#endif

#endif /*MJPEG_H*/
