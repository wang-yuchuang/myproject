/*
* asw_demo.c
*
* Copyright (c) 2019 Semidrive Semiconductor.
* All rights reserved.
*
* Description:
*
* Revision History:
* -----------------
* 011, 12/14/2021 BILLY create this file
*/
#include "CLI_type.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <arch/cache.h>

#include <driver/taishan/asw/asw.h>
#include <driver/taishan/dispss/disp.h>
#include <driver/taishan/dispss/disp_data_type.h>

#include "ext_lut_data.h"
#include "ext_lut_data2.h"

#include "disp_demo_api.h"

static void disp_post_init(struct post_cfg *post, struct surface *layer,
    int width, int height, int fmt, unsigned long addr_y, unsigned long addr_u,
    unsigned long addr_v, int stride_y, int stride_u, int stride_v)
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

    layer->z_order = 2;
    layer->alpha_en = 1;
    layer->alpha = 0xff;
    layer->ckey_en = 0;

    post->num_layers = 1;
    post->layers = layer;

}

/******************api********************/
int asw_case_csi_dc_hsdk(int mode, struct csi_hsk_info *info)
{
    struct asw_input input = {0};
    int width = 1280;
    int height = 720;

    if (mode == 0) {
        input.work_mode = WORK_MODE_CSI_DC_HS;
        input.hdsk_mode = BUF_SIZE_8LINES;
        input.lut_mode = LUT_MODE_BILINEAR;
        input.ext_lut_fmt = EXT_LUT_FORMAT_32BIT;
        input.dst.dst_fmt = FMT_YUV422_UYVY;
        input.dst.dst_hsize = width;
        input.dst.dst_vsize = height;
        input.dst.dst_ba = 0x700000;
        input.dst.dst_stride = width * 2;
        input.dst.dst_bg_color = 0x808000;
        input.dst.dst_flip = FLIP_TYPE_HFLIP;

        input.src.src_fmt = info->format;
        input.src.src_hsize = width;
        input.src.src_vsize = height;
        input.src.src_ry_ba = info->src_ry_ba;//csi mem address.
        input.src.src_gu_ba = info->src_gu_ba;
        input.src.src_bv_ba = info->src_bv_ba;
        input.src.src_ry_stride = info->stride_ry;
        input.src.src_gu_stride = info->stride_gu;
        input.src.src_bv_stride = info->stride_bv;

        input.lut.asw_lut_hsize = 33;
        input.lut.asw_lut_vsize = 33;

        int size = input.lut.asw_lut_hsize  * input.lut.asw_lut_hsize;
        input.ext.ext_mlut_rx_ba = (int)malloc(size * 4);
        input.ext.ext_mlut_ry_ba = (int)malloc(size * 4);
        input.ext.ext_mlut_gx_ba = (int)malloc(size * 4);
        input.ext.ext_mlut_gy_ba = (int)malloc(size * 4);
        //printf("addr 0x%x, 0x%x\n", input.ext.ext_mlut_rx_ba, input.ext.ext_mlut_ry_ba);
        //input.ext.ext_mlut_rx_ba = (input.ext.ext_mlut_rx_ba % 256) ? ((input.ext.ext_mlut_rx_ba / 256) * 256 + 256) : input.ext.ext_mlut_rx_ba;
        //input.ext.ext_mlut_ry_ba = (input.ext.ext_mlut_ry_ba % 256) ? ((input.ext.ext_mlut_ry_ba / 256) * 256 + 256) : input.ext.ext_mlut_ry_ba;
        //printf("addr: 0x%x, 0x%x\n", input.ext.ext_mlut_rx_ba, input.ext.ext_mlut_ry_ba);

        sdrv_asw_bilinear_ftob32(ext_lut_data_info.lut_rx, (int *)input.ext.ext_mlut_rx_ba, size);
        sdrv_asw_bilinear_ftob32(ext_lut_data_info.lut_ry, (int *)input.ext.ext_mlut_ry_ba, size);
        arch_clean_cache_range(input.ext.ext_mlut_rx_ba, size * 4);
        arch_clean_cache_range(input.ext.ext_mlut_ry_ba, size * 4);
        memcpy((void *)input.ext.ext_mlut_gx_ba, (void *)input.ext.ext_mlut_rx_ba, size * 4);
        memcpy((void *)input.ext.ext_mlut_gy_ba, (void *)input.ext.ext_mlut_ry_ba, size * 4);
        arch_clean_cache_range(input.ext.ext_mlut_gx_ba, size * 4);
        arch_clean_cache_range(input.ext.ext_mlut_gy_ba, size * 4);

        input.ext.ext_mlut_rx_stride = input.lut.asw_lut_hsize * 4;
        input.ext.ext_mlut_ry_stride = input.lut.asw_lut_hsize * 4;
        input.ext.ext_mlut_gx_stride = input.lut.asw_lut_hsize * 4;
        input.ext.ext_mlut_gy_stride = input.lut.asw_lut_hsize * 4;

        input.cache_entry_type = C_ENTRY_TYPE_16X4;
        input.cache_way_n = 6;
        input.csi_start_ln = 10;
        sdrv_asw_bilinear(&input);
    } else {
    }

    return 0;
}

int dc_wait_csi_irq(struct dc_dev *dev)
{
    struct post_cfg post;
    struct surface layer;
    int width = 1280;
    int height = 720;
    memset(&layer, 0, sizeof(struct surface));

    {
        layer.id = 1;
        layer.dirty = 1;
        layer.en = 1;
        layer.fmt = COLOR_YUVY;

        layer.src.x = 0;
        layer.src.y = 0;
        layer.src.w = width;
        layer.src.h = height;

        layer.start.x = 0;
        layer.start.y = 0;
        layer.start.w = width;
        layer.start.h = height;

        layer.addr[0] = (int)aswdev.app.out_buf[0];
        layer.src_stride[0] = width * 2;

        layer.dst.x = 0;
        layer.dst.y = 24;
        layer.dst.w = width;
        layer.dst.h = height;

        layer.z_order = 0;
        layer.alpha_en = 1;
        layer.alpha = 0xff;
        layer.ckey_en = 0;
    }

    post.num_layers = 1;
    post.layers = &layer;

    sdrv_update_csi_detect(dev, &post, 3, CSI_DC_1_2, true);

    return 0;
}

int asw_case_csi_hsdk(int mode, struct csi_hsk_info *info)
{
    struct asw_dev *dev = &aswdev;
    struct dc_dev *dc = &g_dc_dev;
    struct asw_input input = {0};
    int width = 1280;
    int height = 720;

    dev->app.out_buf[0] = (int)malloc(width * height *2);
    dev->app.out_buf[1] = (int)malloc(width * height *2);
    dc->app.out_buf[0] = dev->app.out_buf[0];
    dc->app.out_buf[1] = dev->app.out_buf[1];
    if (mode == 0) {
        input.work_mode = WORK_MODE_CSI_HS;
        input.hdsk_mode = BUF_SIZE_8LINES;
        input.lut_mode = LUT_MODE_BILINEAR;
        input.ext_lut_fmt = EXT_LUT_FORMAT_32BIT;
        input.dst.dst_fmt = FMT_YUV422_UYVY;
        input.dst.dst_hsize = width;
        input.dst.dst_vsize = height;
        input.dst.dst_ba = dev->app.out_buf[0];
        input.dst.dst_stride = width * 2;
        input.dst.dst_bg_color = 0x808000;
        input.dst.dst_flip = FLIP_TYPE_HFLIP;

        input.src.src_fmt = info->format;
        input.src.src_hsize = width;
        input.src.src_vsize = height;
        input.src.src_ry_ba = info->src_ry_ba;//csi mem address.
        input.src.src_gu_ba = info->src_gu_ba;
        input.src.src_bv_ba = info->src_bv_ba;
        input.src.src_ry_stride = info->stride_ry;
        input.src.src_gu_stride = info->stride_gu;
        input.src.src_bv_stride = info->stride_bv;

        input.lut.asw_lut_hsize = 35;
        input.lut.asw_lut_vsize = 35;

        int size = input.lut.asw_lut_hsize  * input.lut.asw_lut_hsize;
        dev->app.ext_lut_x[0] = (int)malloc(size * 4);
        dev->app.ext_lut_y[0] = (int)malloc(size * 4);
        input.ext.ext_mlut_rx_ba = dev->app.ext_lut_x[0];
        input.ext.ext_mlut_ry_ba = dev->app.ext_lut_y[0];
        input.ext.ext_mlut_gx_ba = input.ext.ext_mlut_rx_ba;
        input.ext.ext_mlut_gy_ba = input.ext.ext_mlut_ry_ba;

        dev->app.ext_lut_x[1] = (int)malloc(size * 4);
        dev->app.ext_lut_y[1] = (int)malloc(size * 4);
        //input.ext.ext_mlut_gx_ba = (int)malloc(size * 4);
        //input.ext.ext_mlut_gy_ba = (int)malloc(size * 4);
        //printf("addr 0x%x, 0x%x\n", input.ext.ext_mlut_rx_ba, input.ext.ext_mlut_ry_ba);
        //input.ext.ext_mlut_rx_ba = (input.ext.ext_mlut_rx_ba % 256) ? ((input.ext.ext_mlut_rx_ba / 256) * 256 + 256) : input.ext.ext_mlut_rx_ba;
        //input.ext.ext_mlut_ry_ba = (input.ext.ext_mlut_ry_ba % 256) ? ((input.ext.ext_mlut_ry_ba / 256) * 256 + 256) : input.ext.ext_mlut_ry_ba;
        //printf("addr: 0x%x, 0x%x\n", input.ext.ext_mlut_rx_ba, input.ext.ext_mlut_ry_ba);

        sdrv_asw_bilinear_ftob32(ext_lut_data_info.lut_rx, (int *)input.ext.ext_mlut_rx_ba, size);
        sdrv_asw_bilinear_ftob32(ext_lut_data_info.lut_ry, (int *)input.ext.ext_mlut_ry_ba, size);
        arch_clean_cache_range(input.ext.ext_mlut_rx_ba, size * 4);
        arch_clean_cache_range(input.ext.ext_mlut_ry_ba, size * 4);

        sdrv_asw_bilinear_ftob32(ext_lut_data_info2.lut_rx, (int *)dev->app.ext_lut_x[1], size);
        sdrv_asw_bilinear_ftob32(ext_lut_data_info2.lut_ry, (int *)dev->app.ext_lut_y[1], size);
        arch_clean_cache_range(dev->app.ext_lut_x[1], size * 4);
        arch_clean_cache_range(dev->app.ext_lut_y[1], size * 4);

        //memcpy((void *)input.ext.ext_mlut_gx_ba, (void *)input.ext.ext_mlut_rx_ba, size * 4);
        //memcpy((void *)input.ext.ext_mlut_gy_ba, (void *)input.ext.ext_mlut_ry_ba, size * 4);
        //arch_clean_cache_range(input.ext.ext_mlut_gx_ba, size * 4);
        //arch_clean_cache_range(input.ext.ext_mlut_gy_ba, size * 4);

        input.ext.ext_mlut_rx_stride = input.lut.asw_lut_hsize * 4;
        input.ext.ext_mlut_ry_stride = input.lut.asw_lut_hsize * 4;
        input.ext.ext_mlut_gx_stride = input.lut.asw_lut_hsize * 4;
        input.ext.ext_mlut_gy_stride = input.lut.asw_lut_hsize * 4;

        input.cache_entry_type = C_ENTRY_TYPE_16X4;
        input.cache_way_n = 6;
        input.csi_start_ln = 10;
        sdrv_asw_bilinear(&input);
    } else {
    }

    return 0;
}


int dc_csi_display(struct dc_dev *dev)
{
    struct post_cfg post;
    struct surface layer;
    int width = 1280;
    int height = 720;
    memset(&layer, 0, sizeof(struct surface));

    {
        layer.id = 1;
        layer.dirty = 1;
        layer.en = 1;
        layer.fmt = COLOR_YUVY;

        layer.src.x = 0;
        layer.src.y = 0;
        layer.src.w = width;
        layer.src.h = height;

        layer.start.x = 0;
        layer.start.y = 0;
        layer.start.w = width;
        layer.start.h = height;

        layer.addr[0] = (int)aswdev.app.out_buf[0];
        layer.src_stride[0] = width * 2;

        layer.dst.x = 0;
        layer.dst.y = 24;
        layer.dst.w = width;
        layer.dst.h = height;

        layer.z_order = 0;
        layer.alpha_en = 1;
        layer.alpha = 0xff;
        layer.ckey_en = 0;
    }

    post.num_layers = 1;
    post.layers = &layer;

    sdrv_display_post(dev, &post);

    return 0;
}

