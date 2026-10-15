/**
 * @file emirror_csi.c
 *
 * Copyright (c) 2022 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: sdrv emirror application.
 *
 * Revision History:
 * -----------------
 */
#include <stdlib.h>
#include <lib/list.h>
#include <cam_service.h>
#include <driver/memmap.h>
#include <cam_hal.h>
#include "emirror_def.h"
#include "emirror_cfg.h"
#include "emirror.h"
#include "app_log.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "emirror"

static camera_handle* cam_handle = NULL;

static int emirror_csi_init(int id, uint32_t* w, uint32_t* h, uint32_t* fmt, uint32_t* f)
{
    cam_handle = hal_cam_get_instance(id);
    if (!cam_handle)
        return -1;

    APP_INFO("cam_handle->ch_id:%d", cam_handle->ch_id);
    int ret = hal_cam_init(cam_handle);
    if (ret)
        return -1;

    uint32_t fps, width, height;
    uint32_t fmt_list[MAX_PIXFMT_COUNT];
    hal_cam_get_fps(cam_handle, &fps);
    hal_cam_get_size(cam_handle, &width, &height);
    hal_cam_enum_format(cam_handle, fmt_list);
    ret = hal_cam_set_format(cam_handle, VDEV_PIX_FMT_YUYV);
    APP_INFO("fps:%d, width:%d, height:%d", fps, width, height);
    if (ret)
        return -1;

    if (w) *w = width;
    if (h) *h = height;
    if (fmt) *fmt = VDEV_PIX_FMT_YUYV;
    if (f) *f = fps;

    return 0;
}

static int emirror_csi_deinit(int id)
{
    APP_INFO("");
    if (cam_handle) {
        hal_cam_deinit(cam_handle);
        cam_handle = NULL;
    }
    return 0;
}

static int emirror_csi_config(int id, emi_csi_buff_t* buf, uint32_t cnt)
{
    APP_INFO("");
    if (!cam_handle || !buf)
        return -1;

    struct vdev_buffer cam_buf;
    for (int i = 0; i < cnt; i++) {
        cam_buf.buffer_idx = i;
        cam_buf.paddr[0] = (void *)p2ap((paddr_t)buf->buffers[i][0]);
        cam_buf.paddr[1] = (void *)p2ap((paddr_t)buf->buffers[i][1]);
        cam_buf.paddr[2] = (void *)p2ap((paddr_t)buf->buffers[i][2]);
        int ret = hal_cam_qbuf(cam_handle, &cam_buf);
        if (ret)
            return -1;
    }

    return 0;
}

static int emirror_csi_set_buf(int id, struct vdev_buffer* buf)
{
    APP_DBG("");
    if (!cam_handle || !buf)
        return -1;
    return hal_cam_qbuf(cam_handle, buf);
}

static int emirror_csi_get_buf(int id, struct vdev_buffer* buf)
{
    APP_DBG("");
    if (!cam_handle || !buf)
        return -1;
    buf->timeout_ms = 500;
    return hal_cam_dqbuf(cam_handle, buf);
}

static int emirror_csi_start(int id)
{
    APP_INFO("");
    if (!cam_handle)
        return -1;
    return hal_cam_start(cam_handle);
}

static int emirror_csi_stop(int id)
{
    APP_INFO("");
    if (!cam_handle)
        return -1;
    return hal_cam_stop(cam_handle);
}

static int emirror_csi_alloc_buff(int id, emi_csi_buff_t* buff)
{
    if (!cam_handle || !buff) {
        return -1;
    }

    switch (buff->fmt) {
    case VDEV_PIX_FMT_UYVY:
    case VDEV_PIX_FMT_YUYV:
    case VDEV_PIX_FMT_RGB16:
        buff->planes = 1;
        buff->pix_bytes = 2;
        buff->stride[0] = buff->w*2;
        buff->buf_size[0] = buff->w*buff->h*2;
        break;
    case VDEV_PIX_FMT_NV16:
    case VDEV_PIX_FMT_NV61:
        buff->planes = 2;
        buff->pix_bytes = 2;
        buff->stride[0] = buff->w;
        buff->stride[1] = buff->w;
        buff->buf_size[0] = buff->w*buff->h;
        buff->buf_size[1] = buff->w*buff->h;
        break;
    case VDEV_PIX_FMT_YUV422P:
        buff->planes = 3;
        buff->pix_bytes = 2;
        buff->stride[0] = buff->w;
        buff->stride[1] = buff->w>>1;
        buff->stride[2] = buff->w>>1;
        buff->buf_size[0] = buff->w*buff->h;
        buff->buf_size[1] = buff->w*buff->h>>1;
        buff->buf_size[2] = buff->w*buff->h>>1;
        break;
    case VDEV_PIX_FMT_RGB24:
    case VDEV_PIX_FMT_BGR24:
    default:
        buff->planes = 1;
        buff->pix_bytes = 3;
        buff->stride[0] = buff->w*3;
        buff->buf_size[0] = buff->w*buff->h*3;
        break;
    }

    void* base;
    for (int i = 0; i < buff->planes; i++) {
        buff->mem_size[i] = buff->buf_size[i]*CAM_MAX_BUF;
        buff->mem_ptr[i] = malloc(buff->mem_size[i]);
        for (int j = 0; j < CAM_MAX_BUF; j++) {
            base = buff->mem_ptr[i]+j*buff->buf_size[i];
            buff->buffers[j][i] = base;
            memset(base, (0x30*j)&0xFF, buff->buf_size[i]);
            arch_sync_cache_range((addr_t)base, buff->mem_size[i]);
            APP_INFO("base:%p", base);
        }
    }
    return 0;
}

emirror_csi_ops_t emi_csi_ops = {
    .p_init = emirror_csi_init,
    .p_deinit = emirror_csi_deinit,
    .p_config = emirror_csi_config,
    .p_setbuf = emirror_csi_set_buf,
    .p_getbuf = emirror_csi_get_buf,
    .p_start = emirror_csi_start,
    .p_stop = emirror_csi_stop,
    .p_alloc = emirror_csi_alloc_buff,
};
/* EOF */
