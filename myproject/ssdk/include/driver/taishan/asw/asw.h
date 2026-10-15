/*
* asw.h
*
* Copyright (c) 2021 Semidrive Semiconductor.
* All rights reserved.
*
* Description:
*
* Revision History:
* -----------------
* 011, 12/08/2021 BILLY create this file
*/
#ifndef __ASW_H__
#define __ASW_H__
#include <types.h>
#include <cmsis_os2.h>
#include "asw_lut_hvk.h"
#include "asw_g_use.h"

/*-*-*fmt*-*-*/
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

/*SRC FMT support: */
/*   FMT_MONOTONIC_8BIT   */
/*   FMT_RGB_YUV888_PLANAR*/
/*   FMT_YUV422_PLANAR    */
/*   FMT_YUV420_PLANAR    */
/*   FMT_RGB565           */
/*   FMT_RGBA5551         */
/*   FMT_ARGB1555         */
/*   FMT_RGB_YUV888       */
/*   FMT_YUV422_SEMI_U0V0 */
/*   FMT_YUV422_SEMI_V0U0 */
/*   FMT_RGBA8888         */
/*   FMT_ARGB8888         */

enum {
    FMT_MONOTONIC_8BIT    = 0,
    FMT_RGB_YUV888_PLANAR = 1,
    FMT_YUV422_PLANAR     = 2,
    FMT_YUV420_PLANAR     = 3,
    FMT_RGB565            = 4,
    FMT_RGBA5551          = 5,
    FMT_ARGB1555          = 6,
    FMT_RGB_YUV888        = 8,
    FMT_YUV422_SEMI_U0V0  = 9,
    FMT_YUV422_SEMI_V0U0  = 10,
    FMT_RGBA8888          = 12,
    FMT_ARGB8888          = 13,
    FMT_YUV422_YUYV       = 14,
    FMT_YUV422_YVYU       = 15
};

#define FMT_YUV422_UYVY  9
#define FMT_YUV422_VYUY  10

enum {
    LB_LAYOUT_A = 0,
    LB_LAYOUT_B
};

enum {
    LUT_MODE_BILINEAR = 0,
    LUT_MODE_CUBIC_SPLINE
};

enum {
    EXT_LUT_FORMAT_16BIT = 0,
    EXT_LUT_FORMAT_32BIT
};

enum {
    CUBIC_MODE_INT_ASM = 0,
    CUBIC_MODE_EXT_ASM
};

enum {
    WORK_MODE_M_TO_M = 0,
    WORK_MODE_CSI_HS,
    WORK_MODE_DC_HS,
    WORK_MODE_CSI_DC_HS
};

/*cache entry type (hsize x vsize)*/
enum {
    C_ENTRY_TYPE_8X4 = 0,
    C_ENTRY_TYPE_8X8,
    C_ENTRY_TYPE_16X4,
    C_ENTRY_TYPE_16X8,
    C_ENTRY_TYPE_32X4
};

enum {
    BUF_SIZE_8LINES = 0,
    BUF_SIZE_16LINES,
    BUF_SIZE_32LINES,
    BUF_SIZE_64LINES
};

enum {
    FLIP_TYPE_NONE = 0,
    FLIP_TYPE_HFLIP = 1,
    FLIP_TYPE_VFLIP = 2,
    FLIP_TYPE_HVFLIP = 3
};

enum TRIAG_YTPE {
    TRIAG_LEFT_TOP = 0,
    TRIAG_LEFT_BOTTOM,
    TRIAG_TYPE_RIGHT_TOP,
    TRIAG_TYPE_RIGHT_BOTTOM
};

struct asw_config {
    paddr_t base;
    int irq;
};

struct dst_info {
    int dst_fmt;
    int dst_hsize;
    int dst_vsize;
    int dst_ba;
    int dst_stride;
    int dst_bg_color;
    int dst_flip;
};

struct src_info {
    int src_fmt;
    int src_hsize;
    int src_vsize;
    int src_ry_ba;
    int src_gu_ba;
    int src_bv_ba;
    int src_ry_stride;
    int src_gu_stride;
    int src_bv_stride;
    int src_flip;
};

struct lut_info {
    int asw_lut_hsize;
    int asw_lut_vsize;
    int asw_lut_hstep;
    int asw_lut_vstep;
};

struct cubic_info {
    int lut_src_ba;
    int alpha_src_ba;
};

struct ext_info {
    int ext_mlut_rx_ba;
    int ext_mlut_ry_ba;
    int ext_mlut_gx_ba;
    int ext_mlut_gy_ba;
    int ext_mlut_bx_ba;
    int ext_mlut_by_ba;
    int ext_mlut_rx_stride;
    int ext_mlut_ry_stride;
    int ext_mlut_gx_stride;
    int ext_mlut_gy_stride;
    int ext_mlut_bx_stride;
    int ext_mlut_by_stride;
};

struct asw_input {
    int work_mode;
    int hdsk_mode;
    int lut_mode;
    int ext_lut_fmt;

    struct dst_info dst;
    struct src_info src;
    struct lut_info lut;

    int cache_entry_type;
    int cache_way_n;

    /*cubic spline lut,alpha src*/
    struct cubic_info cubic;

    /*bilinear lut infomation*/
    struct ext_info ext;

    /*csi handshake*/
    int csi_start_ln;
};

struct asw_ba_info {
    int asw_pos_calc_pfile_ba;
    int asw_pos_lut_ba;
    int asw_m_v_ba;
    int asw_alpha_ba;
    int asw_y_stepy_ba;
    int asw_lut_h_ba;
    int asw_m_h_ba;
    int asw_frb_ba;
    int asw_cache_ry_ba;
    int asw_cache_gu_ba;
    int asw_cache_bv_ba;
    int asw_cache_a_ba;
};

struct asw_rot {
    int hsize;
    int vsize;
    double angle;

    int dst_fmt;
    int dst_ba;
    int dst_stride;

    int src_fmt;
    int src_ba[4];
    int src_stride[4];
};

/**
  *This struct include asw draw triangle input infomations
  *@type: triangle type, value in enum TRIAG_YTPE.
  *@fmt: buffer format, only support FMT_RGB565, FMT_RGBA5551, FMT_ARGB1555
  *FMT_RGB_YUV888,FMT_RGBA8888,FMT_ARGB8888.
  *@color: B:bit[31:24], G:bit[23:16], R:bit[15:8], A:bit[7:0].
  *@pos_x, pos_y: the position(x,y) in buffer.
  *@addr: buffer base address.
  *@stride: buffer pitch.
**/
struct asw_ratria {
    int type;
    int fmt;
    int color;
    int hsize;
    int vsize;
    int pos_x;
    int pos_y;
    int addr;
    int stride;
};

struct asw_operations;
struct asw_dev {
    paddr_t base;
    int irq;
    struct asw_ba_info bas;
    struct asw_operations *ops;
    struct asw_input *input;
    osSemaphoreId_t wdone_sema;

    struct asw_app_asw app;
};

struct asw_operations {
    void (*cubic_cfg)(struct asw_dev *dev);
    void (*cubic_cfg_with_hvkt)(struct asw_dev *dev, struct hvkt_param *hvkt);
    void (*cubic_rotation)(struct asw_dev *dev, double rotate_angle,struct hvkt_param *hvkt);
    void (*bilinear_cfg)(struct asw_dev *dev);
    void (*bilinear_cfg_with_hvkt)(struct asw_dev *dev, struct hvkt_param *hvkt);
    void (*bilinear_rotation)(struct asw_dev *dev, double rotate_angle,struct hvkt_param *hvkt);
    int (*irq_handler)(unsigned int irq, void *arg);
};

extern struct asw_dev aswdev;

void sdrv_asw_probe(struct asw_dev *dev, const struct asw_config *asw_cfg);
void sdrv_asw_cubic(struct asw_input *input);
void sdrv_asw_cubic_with_hvkt(struct asw_input *input, struct hvkt_param *hvkt);

void sdrv_asw_bilinear(struct asw_input *input);
void sdrv_asw_bilinear_with_hvkt(struct asw_input *input, struct hvkt_param *hvkt);
void sdrv_asw_bilinear_ftob32(const float *faddr, int *b32, int len);
void sdrv_asw_bilinear_ftob16(const float *faddr, short *b16, int len);
void sdrv_asw_cubic_rotation(struct asw_rot *rot);
void sdrv_asw_bilinear_rotation(struct asw_rot *rot);
void sdrv_asw_ra_triangle(struct asw_ratria *info);

#endif //__GAMA_H__
