/*
 * Copyright (c) 2022 Semidrive Semiconductor.
 * All rights reserved.
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _CONFIG_OS_SAFETY_

#include <kernel/mutex.h>
#include <lib/console.h>
#include <platform.h>
#include <uapi/uapi/err.h>

#include "i2c_hal.h"

#else /* _CONFIG_OS_SSDK_ */

#endif

#include "ex_vCamera.h"

struct vCam_dev {
    struct vdev_device vdev;
    uint32_t streaming;
    int power_count;
};

int vCamera_exit(struct vdev_device *vdev);

static inline struct vCam_dev *to_vCamera(struct vdev_device *vdev)
{
    return containerof(vdev, struct vCam_dev, vdev);
}

static int vCam_s_power(struct vdev_device *vdev, int on)
{
    int ret = 0;
    struct vCam_dev *vCam = to_vCamera(vdev);

    vCam->power_count += on ? 1 : -1;

    return ret;
}

static int vCam_enum_mbus_fmt(struct vdev_device *vdev,
                              struct vdev_mbus_framefmt *fme)
{
    struct vCam_dev *vCam = to_vCamera(vdev);

    if ((vdev == NULL) || (fme == NULL)) {
        return -1;
    }

    if (fme->index > 0) {
        fme->index = 0;
        return -1;
    }

    fme->code = vCam->vdev.fmt.code;
    fme->field = vCam->vdev.fmt.field;
    fme->colorspace = vCam->vdev.fmt.colorspace;

    return 0;
}

static int vCam_enum_frame_size(struct vdev_device *vdev,
                                struct vdev_frame_size_enum *fse)
{
    fse->min_width = 320;
    fse->max_width = 3840;
    fse->min_height = 240;
    fse->max_height = 1080;

    return 0;
}

static int vCam_enum_frame_interval(struct vdev_device *vdev,
                                    struct vdev_frame_interval_enum *fie)
{
    return 0;
}

static int vCam_g_frame_interval(struct vdev_device *vdev,
                                 struct vdev_fract *fi)
{
    if (!vdev || !fi) {
        return -1;
    }

    *fi = vdev->frame_interval;
    return 0;
}

static int vCam_s_frame_interval(struct vdev_device *vdev,
                                 struct vdev_fract frame_interval)
{
    return 0;
}

static int vCam_get_fmt(struct vdev_device *vdev,
                        struct vdev_mbus_framefmt *fmt)
{
    if (!vdev) {
        return -1;
    }

    *fmt = vdev->fmt;
    return 0;
    ;
}

static int vCam_set_fmt(struct vdev_device *vdev,
                        struct vdev_mbus_framefmt format)
{
    struct vCam_dev *vCam = to_vCamera(vdev);

    if (vCam->streaming)
        return -1;

    vdev->fmt = format;
    return 0;
}

static int vCam_s_stream(struct vdev_device *vdev, int enable)
{
    struct vCam_dev *vCam = to_vCamera(vdev);

    if (vCam->streaming == !enable)
        vCam->streaming = enable;

    return 0;
}

static const struct vdev_dev_ops vCam_vdev_ops = {
    .s_power = vCam_s_power,
    .enum_mbus_fmt = vCam_enum_mbus_fmt,
    .enum_frame_size = vCam_enum_frame_size,
    .enum_frame_interval = vCam_enum_frame_interval,
    .g_frame_interval = vCam_g_frame_interval,
    .s_frame_interval = vCam_s_frame_interval,
    .get_fmt = vCam_get_fmt,
    .set_fmt = vCam_set_fmt,
    .s_stream = vCam_s_stream,
    .close = vCamera_exit,
};

struct vdev_device *ex_vCamera_init(void *init_data)
{
    struct vCam_dev *vCam;
    struct vdev_mbus_framefmt *fmt;

    vCam = malloc(sizeof(struct vCam_dev));
    if (!vCam)
        return NULL;

    memset(vCam, 0, sizeof(*vCam));
    fmt = &vCam->vdev.fmt;
    fmt->code = VDEV_MBUS_FMT_UYVY;
    fmt->colorspace = VDEV_COLORSPACE_SRGB;
    fmt->width = 640;
    fmt->height = 480;
    fmt->field = VDEV_FIELD_NONE;
    vCam->vdev.frame_interval.numerator = 1;
    vCam->vdev.frame_interval.denominator = 30;
    vCam->vdev.ep.bus_type = VDEV_MBUS_PARALLEL;
    vCam->vdev.ops = vCam_vdev_ops;

    printf("%s:\n", __func__);
    return &vCam->vdev;
}

int vCamera_exit(struct vdev_device *vdev)
{
    struct vCam_dev *vCam = to_vCamera(vdev);

    free(vCam);

    return 0;
}
