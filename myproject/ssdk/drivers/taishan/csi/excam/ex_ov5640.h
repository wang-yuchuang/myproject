/**
 * @file ex_ov5640.h
 * @brief export header for external camera sensor ov5640
 *
 * @copyright Copyright (c) 2021  Semidrive Semiconductor.
 *			  All rights reserved.
 */

#ifndef __EX_OV5640_H__
#define __EX_OV5640_H__

#include "exvdev.h"

enum ov5640_mode_id {
    OV5640_MODE_QCIF_176_144 = 0,
    OV5640_MODE_QVGA_320_240,
    OV5640_MODE_VGA_640_480,
    OV5640_MODE_NTSC_720_480,
    OV5640_MODE_PAL_720_576,
    OV5640_MODE_XGA_1024_768,
    OV5640_MODE_720P_1280_720,
    OV5640_MODE_1080P_1920_1080,
    OV5640_MODE_QSXGA_2592_1944,
    OV5640_NUM_MODES,
};

enum ov5640_frame_rate {
    OV5640_15_FPS = 0,
    OV5640_30_FPS,
    OV5640_45_FPS,
    OV5640_60_FPS,
    OV5640_NUM_FRAMERATES,
};

/*
 * size_mode: must be enum (ov5640_mode_id)
 * fps_mode: must be enum (ov5640_frame_rate)
 */
int ov5640_config_mode(struct vdev_device *vdev, int size_mode, int fps_mode);

struct vdev_device *ex_ov5640_init(void *init_data);

#endif /*__EX_OV5640_H__*/
