/**
 * @file cam_hal.h
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */
#ifndef __CAM_HAL_H__
#define __CAM_HAL_H__

#include <driver/kunlun/csi/cam_res.h>

#include "sdrv-cam-os-def.h"
#include "vdev_defs.h"

#define MAX_PIXFMT_COUNT 8

typedef enum e_cam_debug_cmd {
    CAM_SHOW = 0,
    CAM_LOG,
} cam_debug_cmd;

/**
 * @brief Camera instance for app user
 */
typedef struct camera_instance_t {
    uint32_t cam_id;
    uint32_t csi_id;
    uint32_t ch_id;
    void *res_cfg;
    const char *name;
    void *prid_data;
    void *vdev;
    void *csi_dev;
    atomic_t usr_cnt;
} camera_handle;

/**
 * @brief Get a camera instance by cam id.
 */
camera_handle *hal_cam_get_instance(enum camera_id cam_id);

int hal_cam_put_instance(camera_handle *cam_handle);

int hal_cam_init(camera_handle *cam_handle);

int hal_cam_get_size(camera_handle *cam_handle, uint32_t *width,
                     uint32_t *height);

int hal_cam_enum_format(camera_handle *cam_handle, uint32_t *format);

int hal_cam_set_format(camera_handle *cam_handle, uint32_t format);

int hal_cam_get_fps(camera_handle *cam_handle, uint32_t *fps);

int hal_cam_set_crop(camera_handle *cam_handle, uint32_t x[2], uint32_t y[2],
                     uint32_t w[2], uint32_t h[2]);

int hal_cam_set_memrange(camera_handle *cam_handle, uint32_t low[3],
                         uint32_t up[3]);

int hal_cam_qbuf(camera_handle *cam_handle, struct vdev_buffer *buf);

int hal_cam_dqbuf(camera_handle *cam_handle, struct vdev_buffer *buf);

int hal_cam_start(camera_handle *cam_handle);

int hal_cam_stop(camera_handle *cam_handle);

int hal_cam_deinit(camera_handle *cam_handle);

void hal_cam_debug(cam_debug_cmd cmd, int data);

#endif
