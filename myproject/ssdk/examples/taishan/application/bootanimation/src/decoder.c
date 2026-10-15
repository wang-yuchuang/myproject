/**
 * @file decoder.c
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: sdrv animation application decoder.
 *
 * Revision History:
 * -----------------
 */

#include "decoder.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ff.h"
#include "mjpeg.h"


#define DISKD "mmc1:" //"mmc1:"

typedef struct {
    uint32_t w;
    uint32_t h;
    uint32_t pix_bytes;
    uint32_t stride[3];
    uint32_t buf_size[3];
    DISP_COLOR_ENUM format;
} out_buffer_info;

jpg_out_info out_info = {0};     /* JPG opt info*/
jpg_session *dec_session = NULL; /* Session identifier */
uint32_t frame_count = 0;

ANIM_COMPONENT_T g_anim_decoder = {
    .c_init = decoder_init,
    .c_work = decoder_work,
    .c_deinit = decoder_deinit,
};

#define MJPEG_ALIGN_16(_x)             (((_x)+0x0F)&~0x0F)
static int decode_get_buffer_info(ANIM_BUFFER_INFO_T *buf_info, jpg_out_info *jpg_info)
{
    buf_info->w = MJPEG_ALIGN_16(jpg_info->width);
    buf_info->h = MJPEG_ALIGN_16(jpg_info->height);

    switch (jpg_info->format) {
    case JDF_444:
        buf_info->format = COLOR_YUV444P;
        buf_info->stride[0] = buf_info->w;
        buf_info->stride[1] = buf_info->w;
        buf_info->stride[2] = buf_info->w;
        buf_info->buf_size[0] = buf_info->w * buf_info->h;
        buf_info->buf_size[1] = buf_info->buf_size[0];
        buf_info->buf_size[2] = buf_info->buf_size[0];
        break;
    case JDF_422:
        buf_info->format = COLOR_YUV422P;
        buf_info->stride[0] = buf_info->w;
        buf_info->stride[1] = buf_info->w >> 1;
        buf_info->stride[2] = buf_info->w >> 1;
        buf_info->buf_size[0] = buf_info->w * buf_info->h;
        buf_info->buf_size[1] = buf_info->buf_size[0] >> 1;
        buf_info->buf_size[2] = buf_info->buf_size[0] >> 1;
        break;
    case JDF_420:
        buf_info->format = COLOR_YUV420P;
        buf_info->stride[0] = buf_info->w;
        buf_info->stride[1] = buf_info->w >> 1;
        buf_info->stride[2] = buf_info->w >> 1;
        buf_info->buf_size[0] = buf_info->w * buf_info->h;
        buf_info->buf_size[1] = buf_info->buf_size[0] >> 2;
        buf_info->buf_size[2] = buf_info->buf_size[0] >> 2;
        break;
    default:
        return -1;
    }

    buf_info->pix_bytes = buf_info->buf_size[0] + buf_info->buf_size[1] + buf_info->buf_size[2];

    return 0;
}

int decoder_init(ANIM_INFO_T *anim_info_ptr) {

    FATFS fatfs = {0};
    jpg_out_info out_info = {0};     /* JPG opt info*/
    uint64_t start_time, end_time;
    start_time = current_time_hires();

    FRESULT rc = f_mount(&fatfs, DISKD, 1);
    if(rc)
    {
        ssdk_printf(SSDK_CRIT, "f_mount returned %d\r\n",rc);
        return -1;
    }

    dec_session = mjpeg_dec_init(JPEG_IO_SOURCE_DISK,
                                 "mmc1:early_app/animation/anim.mjpeg", 0);
    end_time = current_time_hires();

#ifdef DEBUG_TIME_PRINT
    ssdk_printf(SSDK_CRIT, "\rmjpeg_dec_init succeeded, elapsed time %lld\n",
           end_time - start_time);
#endif

    if (dec_session == NULL)
    {
        ssdk_printf(SSDK_CRIT, "dec_session failed\n");
        return -2;
    }

    mjpeg_dec_get_header_info(dec_session, &out_info);

    decode_get_buffer_info(anim_info_ptr->a_data->buf_info, &out_info);

    return 0;
}

int decoder_work(ANIM_INFO_T *anim_info_ptr) {

    if (!mjpeg_dec_parser_next_frame(dec_session)) {
        if (JDR_OK == mjpeg_dec_process_frame(dec_session, anim_info_ptr->a_data->buf_info->fb_addr[anim_info_ptr->a_data->buf_info->fb_decoder],
                                              anim_info_ptr->a_data->buf_info->buf_size[0],
                                              anim_info_ptr->a_data->buf_info->buf_size[1])) {
        } else {
            ssdk_printf(SSDK_CRIT, "decoder_work process failed\n");
            return -1;
        }
    } else {
        ssdk_printf(SSDK_CRIT, "decoder_work no next frame\n");
        return -2;
    }

    return 0;
}

int decoder_deinit(ANIM_INFO_T *anim_info_ptr) {

    f_mount(NULL, DISKD, 1);
    mjpeg_dec_deinit(dec_session);

    return 0;
}