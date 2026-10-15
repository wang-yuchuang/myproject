/*
* g2d_hal.c
*
* Copyright (c) 2012 Semidrive Semiconductor.
* All rights reserved.
*
* Description:
*
* Revision History:
* -----------------
* 2022/06/01 create this file
*/
#include <gpu_hal.h>

extern struct g2dlite_device g2dlite_dev;

void hal_g2dlite_init(void **handle)
{
    *handle = (void *)&g2dlite_dev;
}

void hal_g2dlite_update(void *handle, struct g2dlite_input *input)
{
    sdrv_g2dlite_update(handle, input);
}

void hal_g2dlite_blend(void *handle, struct g2dlite_input *input)
{
    sdrv_g2dlite_update(handle, input);
}

void hal_aswlite_rotaion(void * buf_act,const uint8_t * img_map,int img_angle,int hsize,int vsize,uint8_t *map2){
#ifdef CONFIG_ASW
    struct asw_rot rot;
    rot.hsize = hsize;
    rot.vsize = vsize;
    rot.dst_fmt = FMT_ARGB8888;
    rot.dst_ba = (int)&map2;
    rot.dst_stride = rot.hsize * 4;

    rot.src_fmt = FMT_ARGB8888;
    rot.src_ba[0] = (int)&img_map;
    rot.src_stride[0] =  rot.hsize * 4;

    rot.angle = 10;//img_angle;
    // sdrv_asw_cubic_rotation(&rot);
    // sdrv_asw_bilinear_rotation(&rot);
#endif
}

void hal_g2dlite_scaler(void *handle, struct g2dlite_input *input)
{
    sdrv_g2dlite_update(handle, input);
}

void hal_g2dlite_csc(void *handle, struct g2dlite_input *input)
{
    sdrv_g2dlite_update(handle, input);
}

void hal_g2dlite_rotaion(void *handle, struct g2dlite_input *input)
{
    sdrv_g2dlite_rotaion(handle, input);
}

#ifdef CONFIG_ASW
void hal_asw_draw_triangle(struct asw_ratria *triangle_info)
{
    sdrv_asw_ra_triangle(triangle_info);
}
#endif

void hal_g2dlite_fill_rect(void *handle, u32 color, u8 g_alpha,
    addr_t aaddr, u8 bpa, u32 astride, struct g2dlite_output_cfg *output)
{
    struct g2dlite_bg_cfg bgcfg;

    bgcfg.en = 1;
    bgcfg.color = color;
    bgcfg.g_alpha = g_alpha;
    bgcfg.aaddr = aaddr;
    bgcfg.bpa = bpa;
    bgcfg.astride = astride;

    sdrv_g2dlite_fill_rect(handle, &bgcfg, output);
}

void hal_g2dlite_fastcopy(void *handle, struct fcopy_t *in,
    struct fcopy_t *out)
{
    sdrv_g2dlite_fastcopy(handle, in, out);
}

void hal_g2dlite_clut_setting(void *handle, char *clut_table)
{
    sdrv_g2dlite_clut_setting(handle, clut_table);
}
