/**
 * @file g2dlite.c
 * @brief display subsystem source file.
 *
 * @copyright Copyright (c) 2019-2029 Semidrive Semiconductor.
 *            All rights reserved.
 */
#include <debug.h>
#include <stdio.h>
#include <stdlib.h>
#include <sdm_display.h>
#include <disp_data_type.h>
#include <CLI_type.h>
#include <malloc.h>
#include <gpu_hal.h>
#include <string.h>
#include <arch/cache.h>
#include "g2dlite.h"

#define ROUNDUP(x,y)	((((x)+(y)-1)/(y))*(y))

void *drmem = NULL;
void *dwmem = NULL;
void *domem = NULL;
void *G2D = NULL;

static int clamp(int value, int min, int max) {
    if (value < min) {
        value = min;
    }
    if (value > max) {
        value = max;
    }
    return value;
}

unsigned long blend_gpipe_bypass(u32 fmt, u32 w, u32 h, void *addr)
{
    struct g2dlite_input input;
    hal_g2dlite_init(&G2D);
    if (domem == NULL) {
        domem = memalign(32, ROUNDUP(w * h *4, 32));
    }

    memset(&input, 0, sizeof(struct g2dlite_input));
    input.layer_num = 1;
    for (int i = 0; i < input.layer_num; i++) {
        input.layer[i].layer = i;
        input.layer[i].layer_en = 1;

        input.layer[i].fmt = fmt;
        input.layer[i].zorder = i;
        input.layer[i].src.x = 0;
        input.layer[i].src.y = 0;
        input.layer[i].src.w = w;
        input.layer[i].src.h = h;
        input.layer[i].addr[0] = (unsigned long)addr;
        input.layer[i].src_stride[0] = w * 2;
        input.layer[i].dst.x = 0;
        input.layer[i].dst.y = 0;
        input.layer[i].dst.w = w;
        input.layer[i].dst.h = h;
        input.layer[i].ckey.en = 0;
        input.layer[i].blend = BLEND_PIXEL_NONE;
        input.layer[i].alpha = 0xFF;
    }

    input.output.width = w;
    input.output.height = h;
    input.output.fmt = fmt;
    input.output.addr[0] = (unsigned long)domem;
    input.output.stride[0] = w * 2;
    input.output.rotation = 0;
    hal_g2dlite_blend(G2D, &input);
    printf("fmt:%u, width:%u, height:%u, addr:0x%lx, out addr:0x%lx\n",
            fmt, w, h, (unsigned long)addr, (unsigned long)domem);
    return (unsigned long)domem;
}

unsigned long yuv_to_rgb(u32 fmt, u32 w, u32 h, void *addr)
{
    struct g2dlite_input input;
    hal_g2dlite_init(&G2D);
    if (domem == NULL) {
        domem = memalign(32, ROUNDUP(w * h *4, 32));
    }

    memset(&input, 0, sizeof(struct g2dlite_input));
    input.layer_num = 1;
    for (int i = 0; i < input.layer_num; i++) {
        input.layer[i].layer = i;
        input.layer[i].layer_en = 1;

        input.layer[i].fmt = fmt;
        input.layer[i].zorder = i;
        input.layer[i].src.x = 0;
        input.layer[i].src.y = 0;
        input.layer[i].src.w = w;
        input.layer[i].src.h = h;
        input.layer[i].addr[0] = (unsigned long)addr;
        input.layer[i].src_stride[0] = w * 2;
        input.layer[i].dst.x = 0;
        input.layer[i].dst.y = 0;
        input.layer[i].dst.w = w;
        input.layer[i].dst.h = h;
        input.layer[i].ckey.en = 0;
        input.layer[i].blend = BLEND_PIXEL_NONE;
        input.layer[i].alpha = 0xFF;
    }

    input.output.width = w;
    input.output.height = h;
    input.output.fmt = COLOR_ARGB8888;
    input.output.addr[0] = (unsigned long)domem;
    input.output.stride[0] = w * 4;
    input.output.rotation = 0;
    hal_g2dlite_blend(G2D, &input);
    printf("fmt:%u, width:%u, height:%u, addr:0x%lx, out addr:0x%lx\n",
            fmt, w, h, (unsigned long)addr, (unsigned long)domem);
    return (unsigned long)domem;
}

unsigned long yuv_to_rgb_cpu(u32 fmt, u32 w, u32 h, void *addr)
{
    int color_r0, color_g0, color_b0;
    int color_r1, color_g1, color_b1;
    char *cdrmem, *cdomem;
    hal_g2dlite_init(&G2D);
    if (domem == NULL) {
        domem = memalign(32, ROUNDUP(w * h *4, 32));
    }

    cdrmem = (char*)addr;
    cdomem = (char*)domem;

    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            int color_y0 = cdrmem[y * w*2 + 4 * x + 0];
            int color_u  = cdrmem[y * w*2 + 4 * x + 1];
            int color_y1 = cdrmem[y * w*2 + 4 * x + 2];
            int color_v  = cdrmem[y * w*2 + 4 * x + 3];

            color_r0 = (int)(color_y0 + 1.5748f * (color_v - 128.0f) + 0.5f);
            color_g0 = (int)(color_y0 - 0.1873f * (color_u - 128.0f) - 0.4681f * (color_v - 128.0f) + 0.5f);
            color_b0 = (int)(color_y0 + 1.8556f * (color_u - 128.0f) + 0.5f);

            color_r0 = clamp(color_r0, 0, 255); // make sure 0 <= color <= 255
            color_g0 = clamp(color_g0, 0, 255);
            color_b0 = clamp(color_b0, 0, 255);

            color_r1 = (int)(color_y1 + 1.5748f * (color_v - 128.0f) + 0.5f);
            color_g1 = (int)(color_y1 - 0.1873f * (color_u - 128.0f) - 0.4681f * (color_v - 128.0f) + 0.5f);
            color_b1 = (int)(color_y1 + 1.8556f * (color_u - 128.0f) + 0.5f);

            color_r1 = clamp(color_r1, 0, 255);
            color_g1 = clamp(color_g1, 0, 255);
            color_b1 = clamp(color_b1, 0, 255);
            cdomem[y * w*4 + 8 * x + 0] = color_b0;
            cdomem[y * w*4 + 8 * x + 1] = color_g0;
            cdomem[y * w*4 + 8 * x + 2] = color_r0;
            cdomem[y * w*4 + 8 * x + 3] = 0;

            cdomem[y * w*4 + 8 * x + 4] = color_b1;
            cdomem[y * w*4 + 8 * x + 5] = color_g1;
            cdomem[y * w*4 + 8 * x + 6] = color_r1;
            cdomem[y * w*4 + 8 * x + 7] = 0;
        }
    }

    arch_sync_cache_range((addr_t)cdomem, w*h*4);

    printf("fmt:%u, width:%u, height:%u, addr:0x%lx, out addr:0x%lx\n",
            fmt, w, h, (unsigned long)addr, (unsigned long)domem);
    return (unsigned long)domem;
}

unsigned long blend_2_pipe(u32 fmt, u32 w, u32 h, void *addr)
{
    struct g2dlite_input input;
    hal_g2dlite_init(&G2D);
    if (domem == NULL) {
        domem = memalign(32, ROUNDUP(w * h *4, 32));
    }

    memset(&input, 0, sizeof(struct g2dlite_input));
    input.layer_num = 2;
    for (int i = 0; i < input.layer_num; i++) {
        input.layer[i].layer = i;
        input.layer[i].layer_en = 1;

        input.layer[i].fmt = fmt;
        input.layer[i].zorder = i;
        input.layer[i].src.x = 0;
        input.layer[i].src.y = 0;
        input.layer[i].src.w = w;
        input.layer[i].src.h = h;

        if (i == 0)
            input.layer[i].addr[0] = (unsigned long)addr;
        else
            input.layer[i].addr[0] = (unsigned long)addr;

        input.layer[i].src_stride[0] = w * 2;
        input.layer[i].dst.x = 0;
        input.layer[i].dst.y = 0;
        input.layer[i].dst.w = w;
        input.layer[i].dst.h = h;
        input.layer[i].ckey.en = 0;
        input.layer[i].blend = BLEND_PIXEL_NONE;
        input.layer[i].alpha = 0x80;
    }

    input.output.width = w;
    input.output.height = h;
    input.output.fmt = fmt;
    input.output.addr[0] = (unsigned long)domem;
    input.output.stride[0] = w*2;
    input.output.rotation = 0;
    hal_g2dlite_blend(G2D, &input);
    printf("fmt:%u, width:%u, height:%u, addr:0x%lx, out addr:0x%lx\n",
            fmt, w, h, (unsigned long)addr, (unsigned long)domem);
    return (unsigned long)domem;
}

unsigned long scaler_gpipe(u32 fmt, u32 w, u32 h, void *addr)
{
    struct g2dlite_input input;
    hal_g2dlite_init(&G2D);
    if (domem == NULL) {
        domem = memalign(32, ROUNDUP(w * h *4, 32));
    }
    memset(&input, 0, sizeof(struct g2dlite_input));
    input.layer_num = 1;
    for (int i = 0; i < input.layer_num; i++) {
        input.layer[i].layer = i;
        input.layer[i].layer_en = 1;

        input.layer[i].fmt = fmt;
        input.layer[i].zorder = i;
        input.layer[i].src.x = 0;
        input.layer[i].src.y = 0;
        input.layer[i].src.w = w;
        input.layer[i].src.h = h;
        input.layer[i].addr[0] = (unsigned long)addr;
        input.layer[i].src_stride[0] = w*2;
        input.layer[i].dst.x = 0;
        input.layer[i].dst.y = 0;
        input.layer[i].dst.w = 2*w;
        input.layer[i].dst.h = 2*h;
        input.layer[i].ckey.en = 0;
        input.layer[i].blend = BLEND_PIXEL_NONE;
        input.layer[i].alpha = 0xFF;
    }

    input.output.width = 2*w;
    input.output.height = 2*h;
    input.output.fmt = fmt;
    input.output.addr[0] = (unsigned long)domem;
    input.output.stride[0] = 2*w*2;
    input.output.rotation = 0;
    hal_g2dlite_blend(G2D, &input);
    printf("fmt:%u, width:%u, height:%u, addr:0x%lx, out addr:0x%lx\n",
            fmt, w, h, (unsigned long)addr, (unsigned long)domem);
    return (unsigned long)domem;
}

unsigned long rotation_test(u32 fmt, u32 w, u32 h, void *addr)
{
    struct g2dlite_input input;
    hal_g2dlite_init(&G2D);
    if (domem == NULL) {
        domem = memalign(32, ROUNDUP(w*h*4, 32));
    }

    memset(&input, 0, sizeof(struct g2dlite_input));
    input.layer_num = 1;
    for (int i = 0; i < input.layer_num; i++) {
        input.layer[i].layer = i;
        input.layer[i].layer_en = 1;

        input.layer[i].fmt = fmt;
        input.layer[i].zorder = i;
        input.layer[i].src.x = 0;
        input.layer[i].src.y = 0;
        input.layer[i].src.w = w;
        input.layer[i].src.h = h;
        input.layer[i].addr[0] = (unsigned long)addr;
        input.layer[i].src_stride[0] = w * 2;
        input.layer[i].dst.x = 0;
        input.layer[i].dst.y = 0;
        input.layer[i].dst.w = w;
        input.layer[i].dst.h = h;
        input.layer[i].ckey.en = 0;
        input.layer[i].blend = BLEND_PIXEL_NONE;
        input.layer[i].alpha = 0xFF;
    }
    input.output.rotation = ROTATION_TYPE_VF_90;
    input.output.width = h;
    input.output.stride[0] = h * 2;
    input.output.height = w;
    input.output.fmt = fmt;
    input.output.addr[0] = (unsigned long)domem;
    hal_g2dlite_rotaion(G2D, &input);
    printf("fmt:%u, width:%u, height:%u, addr:0x%lx, out addr:0x%lx\n",
            fmt, w, h, (unsigned long)addr, (unsigned long)domem);
    return (unsigned long)domem;
}

unsigned long fill_rect(u32 fmt, u32 w, u32 h, void *addr)
{
    struct g2dlite_output_cfg output;
    hal_g2dlite_init(&G2D);

    memset(&output, 0, sizeof(struct g2dlite_output_cfg));

    int count = 0;

    output.width = w;
    output.height = h;
    output.fmt = fmt;
    output.addr[0] = (unsigned long)addr;
    output.stride[0] = 2*w;
    output.rotation = 0;
    switch (count) {
        case 0:
            hal_g2dlite_fill_rect(G2D, 0xFFFFFFFF, 0xFF, 0, 0, 0, &output);
            count++;
            break;
        case 1:
            hal_g2dlite_fill_rect(G2D, 0x3ff, 0xFF, 0, 0, 0, &output);
            count++;
            break;
        case 2:
            hal_g2dlite_fill_rect(G2D, 0xFFC00, 0xFF, 0, 0, 0, &output);
            count++;
        case 3:
            hal_g2dlite_fill_rect(G2D, 0x0, 0xFF, 0, 0, 0, &output);
            count = 0;
            break;
    }
    printf("fmt:%u, width:%u, height:%u, addr:0x%lx, out addr:0x%lx\n",
            fmt, w, h, (unsigned long)addr, (unsigned long)domem);
    return output.addr[0];
}

unsigned long fastcopy(u32 fmt, u32 w, u32 h, void *addr)
{
    struct fcopy_t in, out;
    hal_g2dlite_init(&G2D);
    if (domem == NULL) {
        domem = memalign(32, ROUNDUP(w * h *4, 32));
    }

    memset(&in, 0, sizeof(struct fcopy_t));
    memset(&out, 0, sizeof(struct fcopy_t));

    in.addr = (unsigned long)addr;
    in.width = w;
    in.height = h;
    in.stride = 4*w;

    out.addr = (unsigned long)domem;
    out.width = w;
    out.height = h;
    out.stride = 4*w;

    hal_g2dlite_fastcopy(G2D, &in, &out);
    printf("fmt:%u, width:%u, height:%u, addr:0x%lx, out addr:0x%lx\n",
            fmt, w, h, (unsigned long)addr, (unsigned long)domem);
    return (unsigned long)domem;
}

unsigned long porter_duff(u32 fmt, u32 w, u32 h, void *addr)
{
    struct g2dlite_input input;
    int count = 0;
    hal_g2dlite_init(&G2D);
    memset(&input, 0, sizeof(struct g2dlite_input));
    input.layer_num = 2;
    input.pd_info.en = 1;
    input.pd_info.zorder = 0;
    input.pd_info.mode = CLEAR + count++;
    input.pd_info.alpha_need = 1;
    printf("porter-duff mode = %d\n", input.pd_info.mode);

    if (count == DES_SUB)
        count = CLEAR;

    for (int i = 0; i < input.layer_num; i++) {
        input.layer[i].layer = i;
        input.layer[i].layer_en = 1;

        input.layer[i].fmt = fmt;
        input.layer[i].src.x = 0;
        input.layer[i].src.y = 0;
        input.layer[i].src.w = w;
        input.layer[i].src.h = h;

        if (i == 0)
            input.layer[i].addr[0] = (unsigned long)0x76800000;
        else
            input.layer[i].addr[0] = (unsigned long)0x77800000;

        input.layer[i].src_stride[0] = w * 4;
        input.layer[i].dst.x = 0;
        input.layer[i].dst.y = 0;
        input.layer[i].dst.w = w;
        input.layer[i].dst.h = h;
        input.layer[i].ckey.en = 0;
        if (i == 0)
            input.layer[i].pd_type = PD_SRC;
        else
            input.layer[i].pd_type = PD_DST;
    }

    input.output.width = w;
    input.output.height = h;
    input.output.fmt = fmt;
    input.output.addr[0] = (unsigned long)0x78800000;
    input.output.stride[0] = w*4;
    input.output.rotation = 0;
    hal_g2dlite_blend(G2D, &input);
    printf("fmt:%u, width:%u, height:%u, addr:0x%lx, out addr:0x%lx\n",
            fmt, w, h, (unsigned long)addr, (unsigned long)domem);
    return (unsigned long)domem;
}

unsigned long yuv_to_yuv(u32 fmt, u32 w, u32 h, void *addr)
{
    struct g2dlite_input input;
    hal_g2dlite_init(&G2D);
    memset(&input, 0, sizeof(struct g2dlite_input));
    input.layer_num = 1;
    for (int i = 0; i < input.layer_num; i++) {
        input.layer[i].layer = i;
        input.layer[i].layer_en = 1;

        input.layer[i].fmt = fmt;
        input.layer[i].zorder = i;
        input.layer[i].src.x = 0;
        input.layer[i].src.y = 0;
        input.layer[i].src.w = w;
        input.layer[i].src.h = h;
        input.layer[i].addr[0] = (unsigned long)0x76800000;;
        input.layer[i].addr[1] = (unsigned long)0x76900000;
        input.layer[i].src_stride[0] = w;
        input.layer[i].src_stride[1] = w;
        input.layer[i].dst.x = 0;
        input.layer[i].dst.y = 0;
        input.layer[i].dst.w = w;
        input.layer[i].dst.h = h;
        input.layer[i].ckey.en = 0;
        input.layer[i].blend = BLEND_PIXEL_NONE;
        input.layer[i].alpha = 0xFF;
    }

    input.output.width = w;
    input.output.height = h;
    input.output.fmt = fmt;
    input.output.addr[0] = (unsigned long)0x77800000;
    input.output.addr[1] = (unsigned long)0x77900000;
    input.output.stride[0] = w;
    input.output.stride[1] = w;
    input.output.rotation = 0;
    hal_g2dlite_blend(G2D, &input);
    printf("fmt:%u, width:%u, height:%u, addr:0x%lx, out addr:0x%lx\n",
            fmt, w, h, (unsigned long)addr, (unsigned long)domem);
    return (unsigned long)domem;
}