/**
 * @file mjpeg_test.c
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:MJPEG Quick Evaluation Program for SSDK
 *
 * Revision History:
 * -----------------
 */

#include <arch/cache.h>
#include <arch/irq.h>
#include <cmsis_os2.h>
#include <driver/taishan/asw/asw.h>
#include <driver/taishan/dispss/disp.h>
#include <driver/taishan/dispss/disp_data_type.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "CLI_type.h"
#include "arch/pmu.h"
#include "board.h"
#include "ff.h"
#include "mjpeg.h"

#define DISKD "mmc1:" //"mmc0:"
#define MJPEG_TEST_FILE DISKD "early_app/cluster/as32.mjpeg"
#define BUFFER_COUNT 2

#define OUT_BUFFER_ON_IRAM // better performance

typedef struct {
    uint32_t w;
    uint32_t h;
    mjpeg_rect crop;
    uint32_t pix_bytes;
    uint32_t stride[3];
    uint32_t buf_size[3];
    DISP_COLOR_ENUM format;
} out_buffer_info;

#if (CONFIG_MJPEG_LOAD_METHOD == 1)
extern uint8_t cluster[1225462];
#endif

#ifdef CONFIG_DISPSS
extern struct dc_dev g_dc_dev;
static osThreadId_t flush_thread;
static uint8_t *front_buffer;
static osSemaphoreId_t flush_sem;
static uint8_t exit_flush_thread = 0;

static void disp_post_init(struct post_cfg *post, struct surface *layer,
                           int width, int height, int fmt, unsigned long addr_y,
                           unsigned long addr_u, unsigned long addr_v,
                           int stride_y, int stride_u, int stride_v)
{
    memset(post, 0, sizeof(struct post_cfg));
    memset(layer, 0, sizeof(struct surface));

    layer->id = 1;
    layer->dirty = 1;
    layer->en = 1;
    layer->fmt = fmt;

    layer->src.x = 0;
    layer->src.y = 0;
    layer->src.w = width;
    layer->src.h = height;

    layer->start.x = 0;
    layer->start.y = 0;
    layer->start.w = width;
    layer->start.h = height;

    layer->addr[0] = addr_y;
    layer->src_stride[0] = stride_y;

    layer->addr[1] = addr_u;
    layer->src_stride[1] = stride_u;

    layer->addr[2] = addr_v;
    layer->src_stride[2] = stride_v;

    layer->dst.x = 0;
    layer->dst.y = 0;
    layer->dst.w = width;
    layer->dst.h = height;

    layer->z_order = 0;
    layer->alpha_en = 1;
    layer->alpha = 0xff;
    layer->ckey_en = 0;

    post->num_layers = 1;
    post->layers = layer;
}

static void flush_thread_func(void *data)
{
    struct post_cfg post;
    struct surface layer;
    out_buffer_info *buf_info = (out_buffer_info *)data;

    do {
        if (osSemaphoreAcquire(flush_sem, osWaitForever) == osOK) {
            if (exit_flush_thread)
                break;

            disp_post_init(
                &post, &layer, buf_info->crop.w, buf_info->crop.h,
                buf_info->format, (unsigned long)front_buffer,
                (unsigned long)(front_buffer + buf_info->buf_size[0]),
                (unsigned long)(front_buffer + buf_info->buf_size[0] +
                                buf_info->buf_size[1]),
                buf_info->stride[0], buf_info->stride[1], buf_info->stride[2]);
            arch_clean_cache_range((addr_t)(front_buffer), buf_info->pix_bytes);

            sdrv_display_post(&g_dc_dev, &post);
        } else {
            printf("%s: fail\n", __func__);
        }
    } while (1);

    osThreadExit();
}
#endif

static int decode_get_buffer_info(out_buffer_info *buf_info,
                                  jpg_out_info *jpg_info)
{
    buf_info->w = jpg_info->width;
    buf_info->h = jpg_info->height;
    buf_info->crop = jpg_info->crop;

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
        printf("%s: not support format %d\n", __func__, jpg_info->format);
        return -1;
    }

    buf_info->pix_bytes =
        buf_info->buf_size[0] + buf_info->buf_size[1] + buf_info->buf_size[2];

    return 0;
}

static int mjpeg_test(int argc, char *argv[])
{
    uint8_t buf_index = 0;
    uint32_t frame_count = 0;

    FATFS fatfs = {0};
    jpg_out_info out_info = {0};     /* JPG opt info*/
    out_buffer_info buf_info = {0};  /* Decoded buf info*/
    jpg_session *dec_session = NULL; /* Session identifier */
    uint8_t *frame[BUFFER_COUNT] = {0};
    uint64_t start_time, end_time;

#ifdef CONFIG_DISPSS
    exit_flush_thread = 0;
    osThreadAttr_t attr = {
        .priority = osPriorityRealtime,
    };

    flush_sem = osSemaphoreNew(1, 0, NULL);
    flush_thread = osThreadNew(flush_thread_func, &buf_info, &attr);
#endif

    f_mount(&fatfs, DISKD, 1);

    start_time = current_time_hires();

#if (CONFIG_MJPEG_LOAD_METHOD == 1)
    dec_session = mjpeg_dec_init(JPEG_IO_SOURCE_C_ARRAY, cluster, 1225462);
#else
    dec_session = mjpeg_dec_init(JPEG_IO_SOURCE_DISK, MJPEG_TEST_FILE, 0);
#endif

    end_time = current_time_hires();

    if (dec_session == NULL)
        return 0;

    printf("\rmjpeg_dec_init succeeded, elapsed time %lld\n",
           end_time - start_time);

    mjpeg_dec_get_header_info(dec_session, &out_info);
    decode_get_buffer_info(&buf_info, &out_info);

    for (int i = 0; i < BUFFER_COUNT; i++) {
#ifndef OUT_BUFFER_ON_IRAM
        frame[i] = (uint8_t *)osAllocAlign(32, buf_info.pix_bytes);
#else
        frame[i] = (uint8_t *)0x600000 + buf_info.pix_bytes * i; // use iram
#endif
    }

    start_time = current_time_hires();
    while (!mjpeg_dec_parser_next_frame(dec_session)) {
        if (JDR_OK == mjpeg_dec_process_frame(dec_session, frame[buf_index],
                                              buf_info.buf_size[0],
                                              buf_info.buf_size[1])) {
            end_time = current_time_hires();
            printf("\rDecompression succeeded., elapsed time %lld\n",
                   end_time - start_time);
            frame_count++;

#ifdef CONFIG_DISPSS
            front_buffer = frame[buf_index];
            buf_index = (buf_index + 1) % BUFFER_COUNT;
            osSemaphoreRelease(flush_sem);
#endif
        }
    }

#ifdef CONFIG_DISPSS
    exit_flush_thread = 1;
    osSemaphoreRelease(flush_sem);
    osDelay(20);
    osSemaphoreDelete(flush_sem);
#endif

    mjpeg_dec_deinit(dec_session);

#ifndef OUT_BUFFER_ON_IRAM
    for (int i = 0; i < BUFFER_COUNT; i++) {
        osFree(frame[i]);
    }
#endif

    f_mount(NULL, DISKD, 1);

    printf("\rmjpeg decode elapsed time %lld, frame count %d, fps %lld\n",
           (end_time - start_time), frame_count,
           ((frame_count * 1000 * 1000) / (end_time - start_time)));
    return CLI_FALSE;
}

CLI_CMD("mjpeg_demo", "\r\nmjpeg_demo:\r\n mjpeg decode demo\n", mjpeg_test);
