/**
 * @file render.h
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: sdrv animation application render.
 *
 * Revision History:
 * -----------------
 */

#include "render.h"
#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <driver/taishan/dispss/disp.h>
#include <driver/taishan/dispss/disp_data_type.h>
#include <arch/cache.h>
#include "driver/taishan/g2dlite/g2dlite.h"

#define MAX_YUV_FIFO 1280
#define MAX_YUV_FIFO_GAP 8

#define DC_SURFACE_NUM 2


extern struct dc_dev g_dc_dev;
extern struct g2dlite_device g2dlite_dev;

struct post_cfg *post = NULL;
struct surface *layers = NULL;

uint8_t *rgb_buffer = NULL;
bool has_post_screen = false;

ANIM_COMPONENT_T g_anim_render = {
    .c_init = render_init,
    .c_work = render_work,
    .c_deinit = render_deinit,
};

static void disp_post_init(struct post_cfg *post, struct surface *layers,
                           int width, int height, int dw, int dh, int fmt, unsigned long addr_y,
                           unsigned long addr_u, unsigned long addr_v,
                           int stride_y, int stride_u, int stride_v)
{
#if 0
    ssdk_printf(SSDK_CRIT, "disp_post_init %dx%d, %dx%d f=%d, stride_y=%d, stride_u=%d, stride_v=%d\n", width, height, dw, dh, fmt, stride_y, stride_u, stride_v);
#endif
    memset(post, 0, sizeof(struct post_cfg));
    memset(layers, 0, sizeof(struct surface) * DC_SURFACE_NUM);

    layers[0].id = 1;
    layers[0].dirty = 1;
    layers[0].en = 1;
    layers[0].fmt = fmt;

    layers[0].src.x = 0;
    layers[0].src.y = 0;
    layers[0].src.w = width;
    layers[0].src.h = 540;

    layers[0].start.x = (width - MAX_YUV_FIFO);
    layers[0].start.y = 0;
    layers[0].start.w = (width < MAX_YUV_FIFO)? width : MAX_YUV_FIFO;
    layers[0].start.h = 540;

    layers[0].addr[0] = addr_y;
    layers[0].src_stride[0] = stride_y;

    layers[0].addr[1] = addr_u;
    layers[0].src_stride[1] = stride_u;

    layers[0].addr[2] = addr_v;
    layers[0].src_stride[2] = stride_v;

    layers[0].dst.x = (width - MAX_YUV_FIFO);
    layers[0].dst.y = 0;
    layers[0].dst.w = (width < MAX_YUV_FIFO)? width : MAX_YUV_FIFO;
    layers[0].dst.h = 540;

    layers[0].z_order = 0;
    layers[0].alpha_en = 1;
    layers[0].alpha = 0xff;
    layers[0].ckey_en = 0;

    post->num_layers = 1;

    if (width > MAX_YUV_FIFO && DC_SURFACE_NUM >= 2) {
        layers[1].id = 0;
        layers[1].dirty = 1;
        layers[1].en = 1;
        layers[1].fmt = COLOR_ARGB8888;

        layers[1].src.x = 0;
        layers[1].src.y = 0;
        layers[1].src.w = (width - MAX_YUV_FIFO + MAX_YUV_FIFO_GAP);
        layers[1].src.h = 540;

        layers[1].start.x = 0;
        layers[1].start.y = 0;
        layers[1].start.w = (width - MAX_YUV_FIFO + MAX_YUV_FIFO_GAP/2);
        layers[1].start.h = 540;

        layers[1].addr[0] = (unsigned long)rgb_buffer;
        layers[1].src_stride[0] = (width - MAX_YUV_FIFO + MAX_YUV_FIFO_GAP) * 4;

        layers[1].dst.x = 0;
        layers[1].dst.y = 0;
        layers[1].dst.w = (width - MAX_YUV_FIFO + MAX_YUV_FIFO_GAP/2);
        layers[1].dst.h = 540;

        layers[1].z_order = 1;
        layers[1].alpha_en = 1;
        layers[1].alpha = 0xff;
        layers[1].ckey_en = 0;

        post->num_layers = 2;
    }

    post->layers = layers;
}

static void yuv_to_rgb_for_fifo(int width, int height, int fmt, unsigned long addr_y,
                           unsigned long addr_u, unsigned long addr_v,
                           int stride_y, int stride_u, int stride_v)
{
    struct g2dlite_input input;
    memset(&input, 0, sizeof(struct g2dlite_input));
    input.layer_num = 1;
    for (int i = 0; i < input.layer_num; i++) {
        input.layer[i].layer = i;
        input.layer[i].layer_en = 1;

        input.layer[i].fmt = COLOR_YUV420P;
        input.layer[i].zorder = i;
        input.layer[i].src.x = 0;
        input.layer[i].src.y = 0;
        input.layer[i].src.w = (width - MAX_YUV_FIFO + MAX_YUV_FIFO_GAP);
        input.layer[i].src.h = 540;

        input.layer[i].addr[0] = addr_y;
        input.layer[i].src_stride[0] = stride_y;

        input.layer[i].addr[1] = addr_u;
        input.layer[i].src_stride[1] = stride_u;

        input.layer[i].addr[2] = addr_v;
        input.layer[i].src_stride[2] = stride_v;

        input.layer[i].dst.x = 0;
        input.layer[i].dst.y = 0;
        input.layer[i].dst.w = (width - MAX_YUV_FIFO + MAX_YUV_FIFO_GAP);
        input.layer[i].dst.h = 540;
        input.layer[i].ckey.en = 0;
        input.layer[i].blend = BLEND_PIXEL_NONE;
        input.layer[i].alpha = 0xFF;
    }

    input.output.width = (width - MAX_YUV_FIFO + MAX_YUV_FIFO_GAP);
    input.output.height = 540;
    input.output.fmt = COLOR_ARGB8888;
    input.output.addr[0] = (unsigned long)rgb_buffer;
    input.output.stride[0] = (width - MAX_YUV_FIFO + MAX_YUV_FIFO_GAP) * 4;
    input.output.rotation = 0;
    sdrv_g2dlite_update(&g2dlite_dev, &input);
}

int render_init(ANIM_INFO_T *anim_info_ptr)
{
    post = (struct post_cfg *)malloc(sizeof(struct post_cfg));
    layers = (struct surface *)malloc(sizeof(struct surface) * DC_SURFACE_NUM);
#ifdef BOOTANIM_USE_IRAM
    rgb_buffer = (uint8_t *)0x790000;
#else
    rgb_buffer = (uint8_t *)osAllocAlign(32, (width-MAX_YUV_FIFO) * 544 * 4);
#endif
    return 0;
}

int render_work(ANIM_INFO_T *anim_info_ptr)
{
    uint8_t *front_buffer = anim_info_ptr->a_data->buf_info->fb_addr[anim_info_ptr->a_data->buf_info->fb_render];

    if (anim_info_ptr->a_data->buf_info->w > MAX_YUV_FIFO) {
        yuv_to_rgb_for_fifo(
            anim_info_ptr->a_data->buf_info->w,
            anim_info_ptr->a_data->buf_info->h,
            anim_info_ptr->a_data->buf_info->format,
            (unsigned long)front_buffer,
            (unsigned long)(front_buffer + anim_info_ptr->a_data->buf_info->buf_size[0]),
            (unsigned long)(front_buffer + anim_info_ptr->a_data->buf_info->buf_size[0] +
                            anim_info_ptr->a_data->buf_info->buf_size[1]),
            anim_info_ptr->a_data->buf_info->stride[0], anim_info_ptr->a_data->buf_info->stride[1], anim_info_ptr->a_data->buf_info->stride[2]
        );
    }

    disp_post_init(
        post, layers,
        anim_info_ptr->a_data->buf_info->w,
        anim_info_ptr->a_data->buf_info->h,
        anim_info_ptr->a_data->config_info->display_info[0].dw,
        anim_info_ptr->a_data->config_info->display_info[0].dh,
        anim_info_ptr->a_data->buf_info->format,
        (unsigned long)front_buffer,
        (unsigned long)(front_buffer + anim_info_ptr->a_data->buf_info->buf_size[0]),
        (unsigned long)(front_buffer + anim_info_ptr->a_data->buf_info->buf_size[0] +
                        anim_info_ptr->a_data->buf_info->buf_size[1]),
        anim_info_ptr->a_data->buf_info->stride[0], anim_info_ptr->a_data->buf_info->stride[1], anim_info_ptr->a_data->buf_info->stride[2]);
    arch_clean_cache_range((addr_t)(front_buffer), anim_info_ptr->a_data->buf_info->pix_bytes);

    sdrv_display_post(&g_dc_dev, post);
    has_post_screen = true;
    return 0;
}

int render_deinit(ANIM_INFO_T *anim_info_ptr)
{
    if (has_post_screen) {
        layers[0].en = 0;
        layers[1].en = 0;
        sdrv_display_post(&g_dc_dev, post);
        has_post_screen = false;
    }

    if (post) {
        free(post);
        post = NULL;
    }

    if (layers) {
        free(layers);
        layers = NULL;
    }
#ifndef BOOTANIM_USE_IRAM
    if (rgb_buffer) {
        osFree((void *)rgb_buffer);
        rgb_buffer = NULL;
    }
#endif
    return 0;
}
