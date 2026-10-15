/**
 * @file vdev_defs.h
 * @brief externel video device type header file, similar to linux videodev2.h
 *
 * @copyright Copyright (c) 2021  Semidrive Semiconductor.
 *			  All rights reserved.
 */

#ifndef __VDEV_DEFS_H__
#define __VDEV_DEFS_H__

#ifdef __cplusplus
extern "C" {
#endif

enum vdev_colorspace {
    /*
     * Default colorspace, i.e. let the driver figure it out.
     * Can only be used with video capture.
     */
    VDEV_COLORSPACE_DEFAULT = 0,

    VDEV_COLORSPACE_JPEG = 7,
    /* For RGB colorspaces such as produces by most webcams. */
    VDEV_COLORSPACE_SRGB = 8,
};

enum vdev_mbus_type {
    VDEV_MBUS_PARALLEL,
    VDEV_MBUS_PARALLEL_GATE2,
    VDEV_MBUS_PARALLEL_NONGATE,
    VDEV_MBUS_BT656,
    VDEV_MBUS_CSI1,
    VDEV_MBUS_CCP2,
    VDEV_MBUS_CSI2, /* MIPI-CSI2 */
    VDEV_MBUS_DC_PARALLEL,
    VDEV_MBUS_DC_PARALLEL2,
    VDEV_MBUS_PARALLEL2,
};

enum vdev_mbus_format {
    VDEV_MBUS_FMT_UYVY = 0,
    VDEV_MBUS_FMT_YUYV,
    VDEV_MBUS_FMT_UYVYSP,
    VDEV_MBUS_FMT_YUYVSP,
    VDEV_MBUS_FMT_YUV444,
    VDEV_MBUS_FMT_YUV420SP,
    VDEV_MBUS_FMT_YUV420XP,
    VDEV_MBUS_FMT_RGB888,
    VDEV_MBUS_FMT_RGB565,
    VDEV_MBUS_FMT_RGB565X,
    VDEV_MBUS_FMT_RGB24,
    VDEV_MBUS_FMT_BGR24,
    VDEV_MBUS_FMT_JPEG,
};

enum vdev_pix_format {
    VDEV_PIX_FMT_UYVY,    /* YUV422, 1plane: UYVY */
    VDEV_PIX_FMT_YUYV,    /* YUV422, 1plane: YVYV */
    VDEV_PIX_FMT_NV16,    /* YUV422, 2plane: Y, UV */
    VDEV_PIX_FMT_NV61,    /* YUV422, 2plane: Y, VU */
    VDEV_PIX_FMT_YUV422P, /* YUV422, 3plane: Y, U, V */
    VDEV_PIX_FMT_NV12,    /* YUV420, 2plane: Y, UV */
    VDEV_PIX_FMT_NV21,    /* YUV420, 2plane: Y, VU */
    VDEV_PIX_FMT_RGB24,   /* RGB888, 1plane */
    VDEV_PIX_FMT_BGR24,   /* BGR888, 1plane */
    VDEV_PIX_FMT_RGB16,   /* RGB565, 1plane */
    VDEV_PIX_FMT_NUM,
};

enum vdev_field_fmt {
    VDEV_FIELD_ANY = 0,
    VDEV_FIELD_NONE,
    VDEV_FIELD_INTERLACED,
    VDEV_FIELD_MAX,
};

#define VDEV_MBUS_HSYNC_ACTIVE_HIGH (1 << 2)
#define VDEV_MBUS_HSYNC_ACTIVE_LOW (1 << 3)
#define VDEV_MBUS_VSYNC_ACTIVE_HIGH (1 << 4)
#define VDEV_MBUS_VSYNC_ACTIVE_LOW (1 << 5)
#define VDEV_MBUS_PCLK_SAMPLE_RISING (1 << 6)
#define VDEV_MBUS_PCLK_SAMPLE_FALLING (1 << 7)
#define VDEV_MBUS_DATA_ACTIVE_HIGH (1 << 8)
#define VDEV_MBUS_DATA_ACTIVE_LOW (1 << 9)

struct vdev_buffer {
    uint32_t buffer_idx;
    uint32_t frm_cnt;
    uint32_t timestamp;
    uint32_t timeout_ms;
    void *paddr[3];
};

#ifdef __cplusplus
extern "C" {
#endif

#endif