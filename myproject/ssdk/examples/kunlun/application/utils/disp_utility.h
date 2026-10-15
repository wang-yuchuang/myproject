/**
 * @file disp_utility.h
 *
 * Copyright (c) 2022 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: sdrv launcher application.
 *
 * Revision History:
 * -----------------
 */
#ifndef __SD_SDM_DISPLAY_UTILITY_H__
#define __SD_SDM_DISPLAY_UTILITY_H__
#include <sdm_display.h>
#include "app_log.h"

static inline int utils_get_sdm_disp_rect(sdm_display_t* disp, uint32_t* width, uint32_t* height)
{
    if (!disp || !disp->handle || !width || !height) return -1;
    *width = disp->handle->info.width;
    *height = disp->handle->info.height;
    return 0;
}

static inline void utils_fill_sdm_disp_buff_yuv(struct sdm_buffer* sdm_buff, uint8_t* y_ptr, uint8_t* u_ptr, uint8_t* v_ptr, int stride)
{
    if (sdm_buff && y_ptr && u_ptr && v_ptr) {
        sdm_buff->addr[0] = (unsigned long)y_ptr;
        sdm_buff->addr[1] = (unsigned long)u_ptr;
        sdm_buff->addr[2] = (unsigned long)v_ptr;
        sdm_buff->src_stride[0] = stride;
        sdm_buff->src_stride[1] = stride/2;
        sdm_buff->src_stride[2] = stride/2;
    }
}

static inline void utils_fill_sdm_disp_buff(struct sdm_buffer* sdm_buff, unsigned long addr, unsigned long stride)
{
    if (sdm_buff) {
        //APP_LOG("addr:%x, sride:%d", addr, stride);
        sdm_buff->addr[0] = addr;
        sdm_buff->src_stride[0] = stride;
    }
}

static inline void utils_crop_sdm_disp_buff(struct sdm_buffer* sdm_buff, int px, int py, int width, int height)
{
    if (sdm_buff) {
        sdm_buff->start.x = px;
        sdm_buff->start.y = py;
        sdm_buff->start.w = width;
        sdm_buff->start.h = height;
        sdm_buff->src.x = px;
        sdm_buff->src.y = py;
        sdm_buff->src.w = width;
        sdm_buff->src.h = height;
    }
}

static inline void utils_map_sdm_disp_buff(struct sdm_buffer* sdm_buff, int px, int py, int width, int height)
{
    if (sdm_buff) {
        sdm_buff->dst.x = px;
        sdm_buff->dst.y = py;
        sdm_buff->dst.w = width;
        sdm_buff->dst.h = height;
    }
}

#endif /* __SD_SDM_DISPLAY_UTILITY_H__ */
/* EOF */
