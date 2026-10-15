/*
* sdrv_asw.c
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

#include <string.h>
#include <stdlib.h>
#include <arch/cache.h>
#include <driver/taishan/asw/asw.h>
#include <driver/taishan/asw/asw_log.h>
#include <driver/irq.h>
#include <driver/taishan/g2dlite/g2dlite.h>
#include <driver/taishan/dispss/disp_data_type.h>

extern struct g2dlite_device g2dlite_dev;
extern struct asw_operations sdrv_asw_ops;
void sdrv_asw_probe(struct asw_dev *dev, const struct asw_config *asw_cfg)
{
    dev->base = asw_cfg->base;
    dev->irq = asw_cfg->irq;
    dev->ops = &sdrv_asw_ops;
    dev->wdone_sema = osSemaphoreNew(1, 0, NULL);
    dev->app.id = 0;
    irq_attach(dev->irq, dev->ops->irq_handler, dev);
    irq_enable(dev->irq);
}

void sdrv_asw_cubic(struct asw_input *input)
{
    struct asw_dev *dev = &aswdev;

    dev->input = input;
    dev->ops->cubic_cfg(dev);
}

void sdrv_asw_cubic_with_hvkt(struct asw_input *input, struct hvkt_param *hvkt)
{
    struct asw_dev *dev = &aswdev;

    dev->input = input;
    dev->ops->cubic_cfg_with_hvkt(dev, hvkt);
}

void sdrv_asw_bilinear(struct asw_input *input)
{
    struct asw_dev *dev = &aswdev;

    dev->input = input;
    dev->ops->bilinear_cfg(dev);
}

void sdrv_asw_bilinear_with_hvkt(struct asw_input *input, struct hvkt_param *hvkt)
{
    struct asw_dev *dev = &aswdev;

    dev->input = input;
    dev->ops->bilinear_cfg_with_hvkt(dev, hvkt);
}

void sdrv_asw_build_input(struct asw_dev *dev, struct asw_input *input, int work_mode, int lut_mode,
    int dst_fmt, int dst_hsize, int dst_vsize, int dst_ba, int dst_stride, int dst_bg_color, int dst_flip,
    int src_fmt, int src_hsize, int src_vsize, int src_ry_ba, int src_gu_ba, int src_bv_ba, int src_ry_stride, int src_gu_stride, int src_bv_stride, int src_flip,
    int asw_lut_hsize, int asw_lut_vsize, int cache_entry_type, int cache_way_n)
{
    input->work_mode = work_mode;
    input->lut_mode = lut_mode;

    input->dst.dst_fmt = dst_fmt;
    input->dst.dst_hsize = dst_hsize;
    input->dst.dst_vsize = dst_vsize;
    input->dst.dst_ba = dst_ba;
    input->dst.dst_stride = dst_stride;
    input->dst.dst_bg_color = dst_bg_color;
    input->dst.dst_flip = dst_flip;

    input->src.src_fmt = src_fmt;
    input->src.src_hsize = src_hsize;
    input->src.src_vsize = src_vsize;
    input->src.src_ry_ba = src_ry_ba;
    input->src.src_gu_ba = src_gu_ba;
    input->src.src_bv_ba = src_bv_ba;
    input->src.src_ry_stride = src_ry_stride;
    input->src.src_gu_stride = src_gu_stride;
    input->src.src_bv_stride = src_bv_stride;
    input->src.src_flip = src_flip;

    input->lut.asw_lut_hsize = asw_lut_hsize;
    input->lut.asw_lut_vsize = asw_lut_vsize;
    input->cache_entry_type = cache_entry_type;
    input->cache_way_n = cache_way_n;

    dev->input = input;

}

void sdrv_asw_bilinear_ftob32(const float *faddr, int *b32, int len)
{
    for (int i = 0; i < len; i++) {
        int tmp = (int)(faddr[i] * 256);
        tmp = (tmp > 524287) ? 524287 : (tmp < -524288) ? -524288 : tmp;
        b32[i] = tmp;
    }
}

void sdrv_asw_bilinear_ftob16(const float * faddr, short *b16, int len)
{
    for (int i = 0; i < len; i++) {
        int tmp16 = (int)(faddr[i] * 16);
        tmp16 = (tmp16 > 32767) ? 32767 : (tmp16 < -32768) ? -32768 : tmp16;
        b16[i] = (short)tmp16;
    }
}

static int is_g2d_tp_en(double angle)
{
    if ((angle > 45 && angle <= 135) || (angle > 225 && angle < 315))
        return 1;

    return 0;
}

static int get_g2d_fmt(int fmt)
{
    /*DES FMT support: */
    /*   FMT_MONOTONIC_8BIT   */
    /*   FMT_RGB565           */
    /*   FMT_RGBA5551         */
    /*   FMT_ARGB1555         */
    /*   FMT_RGB_YUV888       */
    /*   FMT_YUV422_UYVY */
    /*   FMT_YUV422_VYUY */
    /*   FMT_RGBA8888         */
    /*   FMT_ARGB8888         */
    /*   FMT_YUV422_YUYV */
    /*   FMT_YUV422_YVYU */

    switch (fmt) {
        case FMT_RGB565:
            return COLOR_RGB565;
        case FMT_ARGB1555:
            return COLOR_ARGB1555;
        case FMT_ARGB8888:
            return COLOR_ARGB8888;
        default:
            ASW_LOG_ERR("can't support this fmt\n");
            return -1;
    }
}

static int get_fmt_bpp(int fmt)
{
    switch (fmt) {
        case FMT_RGB565:
        case FMT_ARGB1555:
        case FMT_RGBA5551:
            return 2;
        case FMT_RGB_YUV888:
            return 3;
        case FMT_RGBA8888:
        case FMT_ARGB8888:
            return 4;
        default:
            ASW_LOG_ERR("can't support this fmt\n");
            return -1;
    }
}

void sdrv_asw_cubic_rotation(struct asw_rot *rot)
{
    struct asw_dev *dev = &aswdev;
    struct asw_input input;
    struct hvkt_param hvkt;
    struct g2dlite_input ginput;
    void *g2d_in_buf = NULL;
    int g2d_tp;

    rot->angle = (int)rot->angle % 360 + (rot->angle - (int)rot->angle);

    memset(&input, 0, sizeof(struct asw_input));
    input.work_mode = WORK_MODE_M_TO_M;
    input.lut_mode = LUT_MODE_CUBIC_SPLINE;
    input.dst.dst_fmt = rot->dst_fmt;
    input.dst.dst_hsize = rot->hsize;
    input.dst.dst_vsize = rot->vsize;

    g2d_tp = is_g2d_tp_en(rot->angle);
    if (g2d_tp) {
        g2d_in_buf = malloc(rot->dst_stride * input.dst.dst_vsize);
        if (!g2d_in_buf) {
            ASW_LOG_ERR("g2d in buf malloc failed \n");
            return;
        }
        input.dst.dst_ba = (int)g2d_in_buf;
    } else {
        input.dst.dst_ba = rot->dst_ba;
    }
    input.dst.dst_stride = rot->dst_stride;
    input.dst.dst_bg_color = 0x0;

    input.src.src_fmt = rot->src_fmt;
    input.src.src_hsize = rot->hsize;
    input.src.src_vsize = rot->vsize;
    input.src.src_ry_ba = rot->src_ba[0];
    input.src.src_gu_ba = rot->src_ba[1];
    input.src.src_bv_ba = rot->src_ba[2];
    input.src.src_ry_stride = rot->src_stride[0];
    input.src.src_gu_stride = rot->src_stride[1];
    input.src.src_bv_stride = rot->src_stride[2];

    input.lut.asw_lut_hsize = 35;
    input.lut.asw_lut_vsize = 35;

    input.cache_entry_type = C_ENTRY_TYPE_16X4;
    input.cache_way_n = 6;

    /*hvkt*/
    hvkt.chn = 1;
    hvkt.is_external_lut = false;
    hvkt.fw_map = false;

    hvkt.hsize = input.dst.dst_hsize;
    hvkt.vsize = input.dst.dst_vsize;

    hvkt.hsize_in = input.src.src_hsize;
    hvkt.vsize_in = input.src.src_vsize;

    hvkt.lut_h_size = input.lut.asw_lut_hsize;
    hvkt.lut_v_size = input.lut.asw_lut_vsize;

    hvkt.extend = 1;

    dev->input = &input;
    dev->ops->cubic_rotation(dev, rot->angle, &hvkt);

//g2d
    if (g2d_tp) {
        memset(&ginput, 0, sizeof(struct g2dlite_input));
        int fmt = get_g2d_fmt(rot->dst_fmt);
        ginput.layer_num = 1;
        for (int i = 0; i < ginput.layer_num; i++) {
            ginput.layer[i].layer = i;
            ginput.layer[i].layer_en = 1;

            ginput.layer[i].fmt = fmt;
            ginput.layer[i].zorder = i;
            ginput.layer[i].src.w = rot->hsize;
            ginput.layer[i].src.h = rot->vsize;
            ginput.layer[i].addr[0] = (unsigned long)g2d_in_buf;
            ginput.layer[i].src_stride[0] = rot->dst_stride;
            ginput.layer[i].dst.w = rot->hsize;
            ginput.layer[i].dst.h = rot->vsize;
            ginput.layer[i].blend = BLEND_PIXEL_NONE;
            ginput.layer[i].alpha = 0xFF;
        }

        ginput.output.fmt = fmt;
        ginput.output.width = rot->hsize;
        ginput.output.height = rot->vsize;
        ginput.output.addr[0] = (unsigned long)rot->dst_ba;
        ginput.output.stride[0] = rot->dst_stride;
        ginput.output.rotation = ROTATION_TYPE_ROT_90;
        sdrv_g2dlite_rotaion(&g2dlite_dev, &ginput);

        free(g2d_in_buf);
    }
}

void sdrv_asw_bilinear_rotation(struct asw_rot *rot)
{
    struct asw_dev *dev = &aswdev;
    struct asw_input input;
    struct hvkt_param hvkt;
    struct g2dlite_input ginput;
    void *g2d_in_buf = NULL;
    int g2d_tp, offstep, idx;
    static void *ext_addr[2];
    static int times = 0;
    static bool inited = false;

    rot->angle = (int)rot->angle % 360 + (rot->angle - (int)rot->angle);

    memset(&input, 0, sizeof(struct asw_input));
    input.work_mode = WORK_MODE_M_TO_M;
    input.hdsk_mode = BUF_SIZE_16LINES;
    input.lut_mode = LUT_MODE_BILINEAR;
    input.ext_lut_fmt = EXT_LUT_FORMAT_32BIT;
    input.dst.dst_fmt = rot->dst_fmt;
    input.dst.dst_hsize = rot->hsize;
    input.dst.dst_vsize = rot->vsize;

    g2d_tp = is_g2d_tp_en(rot->angle);
    if (g2d_tp) {
        g2d_in_buf = malloc(rot->dst_stride * input.dst.dst_vsize);
        if (!g2d_in_buf) {
            ASW_LOG_ERR("g2d in buf malloc failed \n");
            return;
        }
        input.dst.dst_ba = (int)g2d_in_buf;
    } else {
        input.dst.dst_ba = rot->dst_ba;
    }
    input.dst.dst_stride = rot->dst_stride;
    input.dst.dst_bg_color = 0x0;

    input.src.src_fmt = rot->src_fmt;
    input.src.src_hsize = rot->hsize;
    input.src.src_vsize = rot->vsize;
    input.src.src_ry_ba = rot->src_ba[0];
    input.src.src_gu_ba = rot->src_ba[1];
    input.src.src_bv_ba = rot->src_ba[2];
    input.src.src_ry_stride = rot->src_stride[0];
    input.src.src_gu_stride = rot->src_stride[1];
    input.src.src_bv_stride = rot->src_stride[2];

    input.lut.asw_lut_hsize = 35;
    input.lut.asw_lut_vsize = 35;

    offstep = 35 * 35 * 4;
    if (!inited) {
        ext_addr[0] = malloc(offstep * 6);
        ext_addr[1] = ext_addr[0];//malloc(offstep * 6);
        inited = true;
    }

    times++;
    if ( times % 2 == 1)
        idx = 0;
    else
        idx = 1;
    if (times == 1000)
        times = 0;

    input.ext.ext_mlut_rx_ba = (int)ext_addr[idx];
    input.ext.ext_mlut_ry_ba = (int)ext_addr[idx] + offstep;// when set :input->ext_mlut_rx_ba + input->asw_lut_hsize * 4, stride = input->asw_lut_hsize * 4 * 2;
    input.ext.ext_mlut_gx_ba = (int)ext_addr[idx] + offstep * 2;
    input.ext.ext_mlut_gy_ba = (int)ext_addr[idx] + offstep * 3;
    input.ext.ext_mlut_bx_ba = (int)ext_addr[idx] + offstep * 4;
    input.ext.ext_mlut_by_ba = (int)ext_addr[idx] + offstep * 5;
    input.ext.ext_mlut_rx_stride = input.lut.asw_lut_hsize * 4;
    input.ext.ext_mlut_ry_stride = input.lut.asw_lut_hsize * 4;
    input.ext.ext_mlut_gx_stride = input.lut.asw_lut_hsize * 4,
    input.ext.ext_mlut_gy_stride = input.lut.asw_lut_hsize * 4;
    input.ext.ext_mlut_bx_stride = input.lut.asw_lut_hsize * 4;
    input.ext.ext_mlut_by_stride = input.lut.asw_lut_hsize * 4;

    input.cache_entry_type = C_ENTRY_TYPE_16X4;
    input.cache_way_n = 6;

    /*hvkt*/
    hvkt.chn = 1;
    hvkt.is_external_lut = true;
    hvkt.fw_map = false;
    hvkt.external_lut_format = input.ext_lut_fmt;//32bits

    hvkt.hsize = input.dst.dst_hsize;
    hvkt.vsize = input.dst.dst_vsize;

    hvkt.hsize_in = input.src.src_hsize;
    hvkt.vsize_in = input.src.src_vsize;

    hvkt.lut_h_size = input.lut.asw_lut_hsize;
    hvkt.lut_v_size = input.lut.asw_lut_vsize;

    hvkt.extend = 1;

    dev->input = &input;
    dev->ops->bilinear_rotation(dev, rot->angle, &hvkt);

//g2d
    if (g2d_tp) {
        memset(&ginput, 0, sizeof(struct g2dlite_input));
        int fmt = get_g2d_fmt(rot->dst_fmt);
        ginput.layer_num = 1;
        for (int i = 0; i < ginput.layer_num; i++) {
            ginput.layer[i].layer = i;
            ginput.layer[i].layer_en = 1;

            ginput.layer[i].fmt = fmt;
            ginput.layer[i].zorder = i;
            ginput.layer[i].src.w = rot->hsize;
            ginput.layer[i].src.h = rot->vsize;
            ginput.layer[i].addr[0] = (unsigned long)g2d_in_buf;
            ginput.layer[i].src_stride[0] = rot->dst_stride;
            ginput.layer[i].dst.w = rot->hsize;
            ginput.layer[i].dst.h = rot->vsize;
            ginput.layer[i].blend = BLEND_PIXEL_NONE;
            ginput.layer[i].alpha = 0xFF;
        }

        ginput.output.fmt = fmt;
        ginput.output.width = rot->hsize;
        ginput.output.height = rot->vsize;
        ginput.output.addr[0] = (unsigned long)rot->dst_ba;
        ginput.output.stride[0] = rot->dst_stride;
        ginput.output.rotation = ROTATION_TYPE_ROT_90;
        sdrv_g2dlite_rotaion(&g2dlite_dev, &ginput);

        free(g2d_in_buf);
    }
}

//right angle triangle
void sdrv_asw_ra_triangle(struct asw_ratria *info)
{
    struct asw_input input;
    float lutx[4], luty[4];
    int s_w, s_h, size;
    int *src = NULL;
    int offstep;
    int exaddr = 0;
    int bpp;

    s_w = 1024;
    s_h = 2;
    size = s_w * s_h;
    src = malloc(size * 4);
    if (!src) {
        ASW_LOG_ERR("triangle src malloc failed \n");
        return;
    }
    for (int i = 0; i < size; i++)
        src[i] = info->color;
    arch_clean_cache_range((addr_t)src, size*4);

    memset(&input, 0, sizeof(struct asw_input));

    bpp = get_fmt_bpp(info->fmt);
    input.work_mode = WORK_MODE_M_TO_M;
    input.hdsk_mode = BUF_SIZE_16LINES;
    input.lut_mode = LUT_MODE_BILINEAR;
    input.ext_lut_fmt = EXT_LUT_FORMAT_32BIT;
    input.dst.dst_fmt = info->fmt;
    input.dst.dst_hsize = info->hsize;
    input.dst.dst_vsize = info->vsize;
    input.dst.dst_ba = info->addr + info->pos_y * info->stride + info->pos_x * bpp;
    input.dst.dst_stride = info->stride;
    input.dst.dst_bg_color = 0x0;

    input.src.src_fmt = FMT_ARGB8888;
    input.src.src_hsize = s_w;
    input.src.src_vsize = s_h;
    input.src.src_ry_ba = (int)src;
    input.src.src_gu_ba = 0;
    input.src.src_bv_ba = 0;
    input.src.src_ry_stride = s_w*4;
    input.src.src_gu_stride = 0;
    input.src.src_bv_stride = 0;

    input.lut.asw_lut_hsize = 2;
    input.lut.asw_lut_vsize = 2;

    offstep = input.lut.asw_lut_hsize * input.lut.asw_lut_vsize * 4;

    exaddr = (int)malloc(offstep*6);
    if (!exaddr) {
        ASW_LOG_ERR("g2d exaddr malloc failed \n");
        goto done;
    }

    input.ext.ext_mlut_rx_ba = exaddr;
    input.ext.ext_mlut_ry_ba = exaddr + offstep;
    input.ext.ext_mlut_gx_ba = exaddr + offstep * 2;
    input.ext.ext_mlut_gy_ba = exaddr + offstep * 3;
    input.ext.ext_mlut_bx_ba = exaddr + offstep * 4;
    input.ext.ext_mlut_by_ba = exaddr + offstep * 5;
    input.ext.ext_mlut_rx_stride = input.lut.asw_lut_hsize * 4;
    input.ext.ext_mlut_ry_stride = input.lut.asw_lut_hsize * 4;
    input.ext.ext_mlut_gx_stride = input.lut.asw_lut_hsize * 4,
    input.ext.ext_mlut_gy_stride = input.lut.asw_lut_hsize * 4;
    input.ext.ext_mlut_bx_stride = input.lut.asw_lut_hsize * 4;
    input.ext.ext_mlut_by_stride = input.lut.asw_lut_hsize * 4;

    input.cache_entry_type = C_ENTRY_TYPE_16X4;
    input.cache_way_n = 6;

    switch (info->type) {
        case TRIAG_LEFT_TOP:
            lutx[0] = 0;
            lutx[1] = s_w - 1;
            lutx[2] = s_w - 1;
            lutx[3] = 2 * s_w - 1;

            luty[0] = 0;
            luty[1] = 0;
            luty[2] = 1;
            luty[3] = 1;
            break;
        case TRIAG_LEFT_BOTTOM:
            lutx[0] = s_w - 1;
            lutx[1] = 2 * s_w - 1;
            lutx[2] = 0;
            lutx[3] = s_w - 1;

            luty[0] = 0;
            luty[1] = 0;
            luty[2] = 1;
            luty[3] = 1;
            break;
        case TRIAG_TYPE_RIGHT_TOP:
            lutx[0] = 0;
            lutx[1] = s_w - 1;
            lutx[2] =  1 - s_w;
            lutx[3] = 0;

            luty[0] = 0;
            luty[1] = 0;
            luty[2] = 1;
            luty[3] = 1;
            break;
        case TRIAG_TYPE_RIGHT_BOTTOM:
            lutx[0] = 1 - s_w;
            lutx[1] = 0;
            lutx[2] = 0;
            lutx[3] = s_w - 1;

            luty[0] = 0;
            luty[1] = 0;
            luty[2] = 1;
            luty[3] = 1;
            break;
        default:
            ASW_LOG_ERR("Not Support This TYPE(%d)\n", info->type);
            return;
    }

    sdrv_asw_bilinear_ftob32(lutx, (int *)input.ext.ext_mlut_rx_ba, 4);
    sdrv_asw_bilinear_ftob32(luty, (int *)input.ext.ext_mlut_ry_ba, 4);
    arch_clean_cache_range((addr_t)input.ext.ext_mlut_rx_ba, offstep*6);
    sdrv_asw_bilinear(&input);

done:
    free((void *)exaddr);
    free((void *)src);
}
