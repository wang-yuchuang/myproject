/*
 * cam_hal.c
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: implement the camera interface for application user
 *
 * Revision History:
 * -----------------
 */

#include "cam_hal.h"

#include "csi_drv.h"
#include "exvdev.h"
#include "mipicsi-os.h"

/* board init should reconfig this  */
struct camera_res_cfg g_cam_cfg[CAMERA_NUM] = {
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, NULL, NULL, NULL, NULL, NULL},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, NULL, NULL, NULL, NULL, NULL},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, NULL, NULL, NULL, NULL, NULL},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, NULL, NULL, NULL, NULL, NULL},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, NULL, NULL, NULL, NULL, NULL},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 1, NULL, NULL, NULL, NULL, NULL},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 1, NULL, NULL, NULL, NULL, NULL},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, NULL, NULL, NULL, NULL, NULL},
};

struct mipi_config g_mipi_cfg[MIPI_CSI_NUM] = {
    {0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0},
};

static camera_handle camera_handles[CAMERA_NUM] = {
    [CAMERA_0] =
        {
            .cam_id = CAMERA_0,
            .csi_id = 0,
            .ch_id = 0,
            .name = "camera0",
            .res_cfg = (void *)&g_cam_cfg[CAMERA_0],
        },
    [CAMERA_1] =
        {
            .cam_id = CAMERA_1,
            .csi_id = 0,
            .ch_id = 1,
            .name = "camera1",
            .res_cfg = (void *)&g_cam_cfg[CAMERA_1],
        },
    [CAMERA_2] =
        {
            .cam_id = CAMERA_2,
            .csi_id = 0,
            .ch_id = 2,
            .name = "camera2",
            .res_cfg = (void *)&g_cam_cfg[CAMERA_2],
        },
    [CAMERA_3] =
        {
            .cam_id = CAMERA_3,
            .csi_id = 0,
            .ch_id = 3,
            .name = "camera3",
            .res_cfg = (void *)&g_cam_cfg[CAMERA_3],
        },
    [CAMERA_4] =
        {
            .cam_id = CAMERA_4,
            .csi_id = 1,
            .ch_id = 0,
            .name = "camera4",
            .res_cfg = (void *)&g_cam_cfg[CAMERA_4],
        },
    [CAMERA_5] =
        {
            .cam_id = CAMERA_5,
            .csi_id = 1,
            .ch_id = 1,
            .name = "camera5",
            .res_cfg = (void *)&g_cam_cfg[CAMERA_5],
        },
    [CAMERA_6] =
        {
            .cam_id = CAMERA_6,
            .csi_id = 1,
            .ch_id = 2,
            .name = "camera6",
            .res_cfg = (void *)&g_cam_cfg[CAMERA_6],
        },
    [CAMERA_7] =
        {
            .cam_id = CAMERA_7,
            .csi_id = 1,
            .ch_id = 3,
            .name = "camera7",
            .res_cfg = (void *)&g_cam_cfg[CAMERA_7],
        },
    [CAMERA_8] =
        {
            .cam_id = CAMERA_8,
            .csi_id = 2,
            .ch_id = 0,
            .name = "camera8",
            .res_cfg = (void *)&g_cam_cfg[CAMERA_8],
        },
};

/**
 * @brief Get a camera instance by cam id.
 */
camera_handle *hal_cam_get_instance(enum camera_id cam_id)
{
    camera_handle *cur;
    if (cam_id >= CAMERA_NUM) {
        HAL_LOGE("cam_hal: error cam id %d\n", cam_id);
        return NULL;
    }
    cur = &camera_handles[cam_id];
    HAL_LOGD("cam_hal: get cam%d, ptr %p, before usrs %d\n", cam_id, cur,
             cur->usr_cnt);
    if (atomic_add(&cur->usr_cnt, 1) > 0) {
        atomic_add(&cur->usr_cnt, -1);
        HAL_LOGE("cam_hal: fail to get cam id %d, already in use\n", cam_id);
        return NULL;
    }

    HAL_LOGI("cam_hal: get cam%d, ptr %p, usrs %d\n", cam_id, cur,
             cur->usr_cnt);
    return cur;
}

int hal_cam_put_instance(camera_handle *cam_handle)
{
    if (cam_handle == NULL || cam_handle->cam_id >= CAMERA_NUM) {
        HAL_LOGE("cam_hal: put invalid camera %p\n", cam_handle);
        return 0;
    }

    HAL_LOGD("cam_hal: release usrs %d\n", cam_handle->usr_cnt);

    atomic_add(&cam_handle->usr_cnt, -1);
    HAL_LOGI("cam_hal: release cam%d, ptr %p, usrs %d\n", cam_handle->cam_id,
             cam_handle, cam_handle->usr_cnt);

    return 0;
}

int hal_cam_init(camera_handle *cam_handle)
{
    int ret = 0;
    const struct external_camera_api *excam_api = NULL;
    struct vdev_device *vdev = NULL;
    struct csi_device *csi_dev = NULL;
    struct csi_ch_params param;
    struct camera_res_cfg *cfg;

    if (cam_handle == NULL || cam_handle->cam_id >= CAMERA_NUM) {
        HAL_LOGE("cam_hal: fatal, null cam_handle\n");
        return -1;
    }

    /* camera_dev_init is global for csi dev. DO NOT need deinit */
    csi_dev = camera_dev_init(cam_handle->csi_id);
    if (csi_dev == NULL) {
        HAL_LOGE("cam_hal: cam%d fail to init csi %d\n", cam_handle->cam_id,
                 cam_handle->csi_id);
        return ret;
    }

    cfg = (struct camera_res_cfg *)cam_handle->res_cfg;
    HAL_LOGI("cam_hal: cam%d i2c_res 0x%08x handle %p, name %p\n",
             cam_handle->cam_id, cfg->i2c_res, cfg->i2c_handle, cfg->name);
    if (cfg->name == NULL) {
        HAL_LOGE("cam_hal: cam%d fail to get cam name\n", cam_handle->cam_id);
        return -1;
    }

    excam_api = get_excam_api(cfg->name);
    if (excam_api == NULL) {
        HAL_LOGE("cam_hal: cam%d fail to get external camera api for %s\n",
                 cam_handle->cam_id, cfg->name);
        return -1;
    }

    vdev = excam_api->init(cfg);
    if (vdev == NULL) {
        HAL_LOGE("cam_hal: fail to init vdev, name %s\n", cfg->name);
        return -1;
    }

    ret = vdev->ops.s_power(vdev, 1);
    if (ret) {
        HAL_LOGE("cam_hal: fail to power on vdev %s\n", cfg->name);
        goto pw_err;
    }

    ret = csi_dev->ops.channel_open(cam_handle->csi_id, cam_handle->ch_id);
    if (ret < 0) {
        HAL_LOGE("cam_hal: fail to open csi %d ch %d\n", cam_handle->csi_id,
                 cam_handle->ch_id);
        goto open_err;
    }

    param.bus_type = vdev->ep.bus_type;
    ret = csi_dev->ops.channel_cfg_bus_type(cam_handle->csi_id,
                                            cam_handle->ch_id, &param);
    if (ret < 0) {
        HAL_LOGE("cam_hal: fail to cfg bus_type, csi %d ch %d\n",
                 cam_handle->csi_id, cam_handle->ch_id);
        goto cfg_err;
    }

    if (vdev->ep.bus_type == VDEV_MBUS_CSI2) {
        if (cfg->mipi_cfg == NULL) {
            HAL_LOGE("fail to get mipi config for cam%d\n", cam_handle->cam_id);
            goto cfg_err;
        }
        mipicsi_init(cam_handle->csi_id, (void *)cfg->mipi_cfg);
        camera_dev_sync(cam_handle->csi_id, cfg->mipi_cfg->sync);
    }

    cam_handle->vdev = (void *)vdev;
    cam_handle->csi_dev = (void *)csi_dev;

#ifdef CONFIG_CSI_IOMUX
    /* for E3_BGA324 display board only */
    if ((cam_handle->cam_id == CAMERA_0) || (cam_handle->cam_id == CAMERA_1))
        csi_iomux_cfg(cam_handle->cam_id);
#endif

    HAL_LOGI("cam_hal: cam%d init done. vdev %s\n", cam_handle->cam_id,
             cfg->name);
    return 0;

cfg_err:
    csi_dev->ops.channel_close(cam_handle->csi_id, cam_handle->ch_id);
open_err:
    vdev->ops.s_power(vdev, 0);
pw_err:
    vdev->ops.close(vdev);
    cam_handle->vdev = NULL;
    HAL_LOGE("cam_hal: cam%d init failed. vdev %s\n", cam_handle->cam_id,
             cfg->name);
    return -1;
}

int hal_cam_deinit(camera_handle *cam_handle)
{
    struct vdev_device *vdev = (struct vdev_device *)cam_handle->vdev;
    struct csi_device *csi_dev = (struct csi_device *)cam_handle->csi_dev;

    vdev->ops.s_power(vdev, 0);
    vdev->ops.close(vdev);
    cam_handle->vdev = NULL;

    csi_dev->ops.channel_close(cam_handle->csi_id, cam_handle->ch_id);
    cam_handle->csi_dev = NULL;

    HAL_LOGI("cam_hal: cam%d deinit done\n", cam_handle->cam_id);
    return 0;
}

int hal_cam_get_size(camera_handle *cam_handle, uint32_t *width,
                     uint32_t *height)
{
    int ret = 0;
    struct vdev_device *vdev = (struct vdev_device *)cam_handle->vdev;
    struct csi_device *csi_dev = (struct csi_device *)cam_handle->csi_dev;
    struct csi_ch_params param;
    struct vdev_mbus_framefmt fmt;
    struct camera_res_cfg *cfg;
    cfg = (struct camera_res_cfg *)cam_handle->res_cfg;

    memset(&fmt, 0, sizeof(struct vdev_mbus_framefmt));

    ret = vdev->ops.get_fmt(vdev, &fmt);
    param.field_type = fmt.field;
    if (fmt.field != VDEV_FIELD_INTERLACED) {
        param.width = fmt.width;
        param.height = fmt.height;
    } else {
        param.width = fmt.field_w[0];
        param.height = fmt.field_h[0];
        param.width_even = fmt.field_w[1];
        param.height_even = fmt.field_h[1];
    }
    *width = fmt.width;
    *height = fmt.height;
    if (cfg->mipi_cfg && cfg->mipi_cfg->sync)
        *height *= 4;
    HAL_LOGI("cam_hal: cam %d get size %d %d\n", cam_handle->cam_id, *width,
             *height);

    ret = csi_dev->ops.channel_cfg_size(cam_handle->csi_id, cam_handle->ch_id,
                                        &param);

    return ret;
}

int hal_cam_enum_format(camera_handle *cam_handle, uint32_t *format)
{
    int ret = 0;
    struct vdev_device *vdev = (struct vdev_device *)cam_handle->vdev;
    struct csi_device *csi_dev = (struct csi_device *)cam_handle->csi_dev;
    struct csi_ch_params param;
    struct vdev_mbus_framefmt fmt;

    ret = vdev->ops.get_fmt(vdev, &fmt);
    param.bus_fmt = fmt.code;

    ret = csi_dev->ops.channel_cfg_bus_fmt(cam_handle->csi_id,
                                           cam_handle->ch_id, &param);
    ret = csi_dev->ops.channel_enum_pixfmt(cam_handle->csi_id,
                                           cam_handle->ch_id, &param);
    if (param.pix_fmt_num == 0) {
        HAL_LOGE("cam_hal: cam %d fail to get fmt\n", cam_handle->cam_id);
        return -1;
    } else if (param.pix_fmt_num > MAX_PIXFMT_COUNT) {
        param.pix_fmt_num = MAX_PIXFMT_COUNT;
    }
    param.set_pix_fmt = param.pix_fmt[0];
    for (uint32_t i = 0; i < param.pix_fmt_num; i++) {
        format[i] = param.pix_fmt[i];
    }
    ret = csi_dev->ops.channel_cfg_pixfmt(cam_handle->csi_id, cam_handle->ch_id,
                                          &param);

    HAL_LOGI("%s: set fmt %d\n", __func__, param.set_pix_fmt);
    *format = param.set_pix_fmt;
    return ret;
}

int hal_cam_set_format(camera_handle *cam_handle, uint32_t format)
{
    int ret = 0, support = 0;
    struct vdev_device *vdev = (struct vdev_device *)cam_handle->vdev;
    struct csi_device *csi_dev = (struct csi_device *)cam_handle->csi_dev;
    struct csi_ch_params param;
    struct vdev_mbus_framefmt fmt;

    ret = vdev->ops.get_fmt(vdev, &fmt);
    param.bus_fmt = fmt.code;

    ret = csi_dev->ops.channel_cfg_bus_fmt(cam_handle->csi_id,
                                           cam_handle->ch_id, &param);
    ret = csi_dev->ops.channel_enum_pixfmt(cam_handle->csi_id,
                                           cam_handle->ch_id, &param);
    if (param.pix_fmt_num == 0) {
        HAL_LOGE("cam_hal: cam %d fail to get fmt\n", cam_handle->cam_id);
        return -1;
    }
    for (uint32_t i = 0; i < param.pix_fmt_num; i++) {
        if (format == param.pix_fmt[i]) {
            support = 1;
            break;
        }
    }
    if (support == 0) {
        HAL_LOGE("cam_hal: cam %d does not support fmt %d\n",
                 cam_handle->cam_id, format);
        return -1;
    }
    param.set_pix_fmt = format;
    ret = csi_dev->ops.channel_cfg_pixfmt(cam_handle->csi_id, cam_handle->ch_id,
                                          &param);

    HAL_LOGI("%s: set fmt %d done\n", __func__, param.set_pix_fmt);
    return ret;
}

int hal_cam_get_fps(camera_handle *cam_handle, uint32_t *fps)
{
    int ret = 0;
    struct vdev_device *vdev = (struct vdev_device *)cam_handle->vdev;
    struct vdev_fract fract;

    *fps = 0;
    if (vdev->ops.g_frame_interval) {
        ret = vdev->ops.g_frame_interval(vdev, &fract);
        *fps = fract.denominator;
    }
    return ret;
}

int hal_cam_set_crop(camera_handle *cam_handle, uint32_t x[2], uint32_t y[2],
                     uint32_t w[2], uint32_t h[2])
{
    int ret = 0;
    struct csi_device *csi_dev = (struct csi_device *)cam_handle->csi_dev;
    struct csi_ch_params param;

    param.crop_x = x[0];
    param.crop_y = y[0];
    param.crop_w = w[0];
    param.crop_h = h[0];
    ret = csi_dev->ops.channel_cfg_crop(cam_handle->csi_id, cam_handle->ch_id,
                                        &param);
    if (ret) {
        HAL_LOGE("cam_hal: cam %d fail to cfg crop\n", cam_handle->cam_id);
    }
    return ret;
}

int hal_cam_set_memrange(camera_handle *cam_handle, uint32_t low[3],
                         uint32_t up[3])
{
    int ret = 0;
    struct csi_device *csi_dev = (struct csi_device *)cam_handle->csi_dev;

    ret = csi_dev->ops.channel_cfg_memrange(cam_handle->csi_id,
                                            cam_handle->ch_id, low, up);
    if (ret) {
        HAL_LOGE("cam_hal: cam %d fail to cfg memrange\n", cam_handle->cam_id);
    }
    return ret;
}

int hal_cam_qbuf(camera_handle *cam_handle, struct vdev_buffer *buf)
{
    int ret = 0;
    struct csi_device *csi_dev = (struct csi_device *)cam_handle->csi_dev;

    ret = csi_dev->ops.channel_qbuf(cam_handle->csi_id, cam_handle->ch_id, buf);
    if (ret) {
        HAL_LOGE("cam_hal: cam %d fail to push buffer\n", cam_handle->cam_id);
    }

    return ret;
}

int hal_cam_dqbuf(camera_handle *cam_handle, struct vdev_buffer *buf)
{
    int ret = 0;
    struct csi_device *csi_dev = (struct csi_device *)cam_handle->csi_dev;

    ret =
        csi_dev->ops.channel_dqbuf(cam_handle->csi_id, cam_handle->ch_id, buf);
    if (ret) {
        HAL_LOGE("cam_hal: cam %d fail to pop buffer\n", cam_handle->cam_id);
    }

    return ret;
}

int hal_cam_start(camera_handle *cam_handle)
{
    int ret = 0;
    struct vdev_device *vdev = (struct vdev_device *)cam_handle->vdev;
    struct csi_device *csi_dev = (struct csi_device *)cam_handle->csi_dev;
    struct camera_res_cfg *cfg = (struct camera_res_cfg *)cam_handle->res_cfg;

    ret = csi_dev->ops.channel_stream(cam_handle->csi_id, cam_handle->ch_id, 1);
    if (ret) {
        HAL_LOGE("cam_hal: cam %d fail to stream on csi\n", cam_handle->cam_id);
        return -1;
    }

    if (cfg->mipi_cfg)
        mipicsi_stream(cam_handle->csi_id, 1);

    ret = vdev->ops.s_stream(vdev, 1);
    if (ret) {
        HAL_LOGE("cam_hal: cam %d fail to stream on vdev\n",
                 cam_handle->cam_id);
        csi_dev->ops.channel_stream(cam_handle->csi_id, cam_handle->ch_id, 0);
        return -1;
    }

    HAL_LOGI("cam_hal: cam %d start done\n", cam_handle->cam_id);
    return ret;
}

int hal_cam_stop(camera_handle *cam_handle)
{
    struct vdev_device *vdev = (struct vdev_device *)cam_handle->vdev;
    struct csi_device *csi_dev = (struct csi_device *)cam_handle->csi_dev;
    struct camera_res_cfg *cfg = (struct camera_res_cfg *)cam_handle->res_cfg;

    vdev->ops.s_stream(vdev, 0);

    if (cfg->mipi_cfg)
        mipicsi_stream(cam_handle->csi_id, 0);

    csi_dev->ops.channel_stream(cam_handle->csi_id, cam_handle->ch_id, 0);

    HAL_LOGI("cam_hal: cam %d stop done\n", cam_handle->cam_id);
    return 0;
}

void hal_cam_debug(cam_debug_cmd cmd, int data)
{
    struct csi_device *p_csi_dev = NULL;
    int i;

    if (cmd == CAM_SHOW) {
        HAL_LOGI("--------------------camera info begin--------------------\n");
        for (i = 0; i < CAMERA_NUM; i++) {
            if (camera_handles[i].usr_cnt) {
                p_csi_dev = camera_handles[i].csi_dev;
                p_csi_dev->ops.channel_show_info(camera_handles[i].csi_id,
                                                 camera_handles[i].ch_id);
                HAL_LOGI("\n");
            }
        }
        HAL_LOGI("--------------------camera info end--------------------\n");
    }

    if (cmd == CAM_LOG) {
        switch (data) {
        case CAM_EMERG:
            cam_err_loglevel = CAM_OS_EMERG;
            cam_warn_loglevel = CAM_OS_EMERG;
            cam_info_loglevel = CAM_OS_EMERG;
            cam_debug_loglevel = CAM_OS_EMERG;
            break;
        case CAM_ERR:
            cam_err_loglevel = CAM_OS_ERR;
            cam_warn_loglevel = CAM_OS_EMERG;
            cam_info_loglevel = CAM_OS_EMERG;
            cam_debug_loglevel = CAM_OS_EMERG;
            break;
        case CAM_WARNING:
            cam_err_loglevel = CAM_OS_ERR;
            cam_warn_loglevel = CAM_OS_WARNING;
            cam_info_loglevel = CAM_OS_EMERG;
            cam_debug_loglevel = CAM_OS_EMERG;
            break;
        case CAM_INFO:
            cam_err_loglevel = CAM_OS_ERR;
            cam_warn_loglevel = CAM_OS_WARNING;
            cam_info_loglevel = CAM_OS_INFO;
            cam_debug_loglevel = CAM_OS_EMERG;
            break;
        case CAM_DEBUG:
            cam_err_loglevel = CAM_OS_ERR;
            cam_warn_loglevel = CAM_OS_WARNING;
            cam_info_loglevel = CAM_OS_INFO;
            cam_debug_loglevel = CAM_OS_DEBUG;
            break;
        default:
            HAL_LOGE("Parameter error\n");
            return;
        }
        HAL_LOGE("LOGE\n");
        HAL_LOGI("LOGI\n");
        HAL_LOGD("LOGD\n");
    }

    return;
}
