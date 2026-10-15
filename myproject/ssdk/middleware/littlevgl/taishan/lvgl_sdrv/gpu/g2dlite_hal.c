/*
* g2dlite_hal.c
*
* Copyright (c) 2019 Semidrive Semiconductor.
* All rights reserved.
*
* Description:
*
* Revision History:
* -----------------
* 011, 04/27/2020 BI create this file
*/
#include <g2dlite_api.h>
#include <g2dlite_hal.h>
#include <g2dlite_common.h>

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
