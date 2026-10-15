/**
 * @file scs_com.h
 * @brief common defines for both service sys and client sys
 *
 * @copyright Copyright (c) 2022  Semidrive Semiconductor.
 *			  All rights reserved.
 */

#ifndef __SCS_COM_H__
#define __SCS_COM_H__

/*  Four-character-code (FOURCC) */
#define v4l2_fourcc(a, b, c, d)                                                \
    ((uint32_t)(a) | ((uint32_t)(b) << 8) | ((uint32_t)(c) << 16) |            \
     ((uint32_t)(d) << 24))

#define print_falgs \
	dprintf(0, "%s:%d=======\n", __func__,__LINE__)

#define V4L2_PIX_FMT_YUYV                                                      \
    v4l2_fourcc('Y', 'U', 'Y', 'V') /* 16  YUV 4:2:2     */
#define V4L2_PIX_FMT_UYVY                                                      \
    v4l2_fourcc('U', 'Y', 'V', 'Y') /* 16  YUV 4:2:2     */
#define V4L2_PIX_FMT_NV16                                                      \
    v4l2_fourcc('N', 'V', '1', '6') /* 16  Y/CbCr 4:2:2  */
#define V4L2_PIX_FMT_NV61                                                      \
    v4l2_fourcc('N', 'V', '6', '1') /* 16  Y/CrCb 4:2:2  */
#define V4L2_PIX_FMT_YUV422P                                                   \
    v4l2_fourcc('4', '2', '2', 'P') /* 16  YVU422 planar */
#define V4L2_PIX_FMT_BGR24                                                     \
    v4l2_fourcc('B', 'G', 'R', '3') /* 24  BGR-8-8-8     */
#define V4L2_PIX_FMT_RGB24                                                     \
    v4l2_fourcc('R', 'G', 'B', '3') /* 24  RGB-8-8-8     */
#define V4L2_PIX_FMT_RGB565                                                    \
    v4l2_fourcc('R', 'G', 'B', 'P') /* 16  RGB-5-6-5     */
#define V4L2_PIX_FMT_NV12                                                      \
    v4l2_fourcc('N', 'V', '1', '2') /* 12  Y/CbCr 4:2:0  */
#define V4L2_PIX_FMT_NV21                                                      \
    v4l2_fourcc('N', 'V', '2', '1') /* 12  Y/CrCb 4:2:0  */

/*
 *  F R A M E   S I Z E   E N U M E R A T I O N
 */
enum v4l2_frmsizetypes {
    V4L2_FRMSIZE_TYPE_DISCRETE = 1,
    V4L2_FRMSIZE_TYPE_CONTINUOUS = 2,
    V4L2_FRMSIZE_TYPE_STEPWISE = 3,
};
enum v4l2_buf_type {
    V4L2_BUF_TYPE_VIDEO_CAPTURE = 1,
    V4L2_BUF_TYPE_VIDEO_OUTPUT = 2,
    V4L2_BUF_TYPE_VIDEO_OVERLAY = 3,
    V4L2_BUF_TYPE_VBI_CAPTURE = 4,
    V4L2_BUF_TYPE_VBI_OUTPUT = 5,
    V4L2_BUF_TYPE_SLICED_VBI_CAPTURE = 6,
    V4L2_BUF_TYPE_SLICED_VBI_OUTPUT = 7,
    V4L2_BUF_TYPE_VIDEO_OUTPUT_OVERLAY = 8,
    V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE = 9,
    V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE = 10,
    V4L2_BUF_TYPE_SDR_CAPTURE = 11,
    V4L2_BUF_TYPE_SDR_OUTPUT = 12,
    V4L2_BUF_TYPE_META_CAPTURE = 13,
    /* Deprecated, do not use */
    V4L2_BUF_TYPE_PRIVATE = 0x80,
};

enum scs_inst {
    SCS_CSI0_SYNC,
    SCS_CSI0_INST0,
    SCS_CSI0_INST1,
    SCS_CSI0_INST2,
    SCS_CSI0_INST3,
    SCS_CSI1_SYNC,
    SCS_CSI1_INST0,
    SCS_CSI1_INST1,
    SCS_CSI1_INST2,
    SCS_CSI1_INST3,
    SCS_CSI2_SYNC,
    SCS_CSI2_INST0,
    SCS_CSI2_INST1,
    SCS_CSI2_INST2,
    SCS_CSI2_INST3,
    SCS_INST_COUNT,
};

enum scs_memory {
	SCS_MEMORY_USERPTR,
	SCS_MEMORY_MMAP,
	SCS_MEMORY_OTHER,
};
/* Let these definition compatiable with another OS side */
enum scs_op_type {
    SCS_OP_DEV_OPEN = 0,
    SCS_OP_ENUM_FORMAT = 1,
    SCS_OP_ENUM_FRAMESIZE = 2,
    SCS_OP_GET_FORMAT = 3,
    SCS_OP_SET_FORMAT = 4,
    SCS_OP_QUEUE_SETUP = 5,
    SCS_OP_GET_BUFINFO = 6,
	//SCS_OP_GET_BUFADDR = 7,
    SCS_OP_SET_BUFINFO = 7,
    SCS_OP_QBUF = 8,
    SCS_OP_STREAM_ON = 9,
    SCS_OP_DQBUF = 10,
    SCS_OP_STREAM_OFF = 11,
    SCS_OP_DEV_CLOSE = 12,
    SCS_OP_ENUM_FRAMEINTERVALS = 13,
};

/* Do not exceed 16 bytes so far */
struct scs_ioctl_cmd {
    u16 op;
    u16 instance;/*bit0~7:csi id, 8~15:rproc*/
    union {
        struct {
            u16 index;
            u32 fmt;
        } fmt;
        struct {
            u16 type;
            u16 width;
            u16 height;
            u32 pixelformat;
        } s_fmt;
        struct {
            u16 type;
            u16 width;
            u16 height;
            u16 index;
        } fsz;
        struct {
            u16 type;
            u16 index;
        } fitv;
        struct {
            u8 num_bufs;
            u8 num_planes;
            u8 m_type;
            uint32_t buf_size;
        } queue;
        struct {
            u32 index;
            u32 addr[2];
        } s_bufinfo;

        u8 data[12];
    } msg;
};

/* Do not exceed 22 bytes so far */
struct scs_ioctl_result {
    u16 op;
    u16 instance;/*bit0~7:csi id, 8~15:rproc*/
    union {
        /** used for get_version */
        struct {
            u16 index;
            u32 fmt;
        } fmt;
        /** used for get_config */
        struct {
            u16 type;
            u16 width;
            u16 height;
            u16 index;
        } fsz;
        struct {
            u16 type;
            u16 width;
            u16 height;
            u32 pixelformat;
        } g_fmt;
        struct {
            u16 type;
            u16 index;
            u32 numerator;
            u32 denominator;
        } fitv;
        struct {
            u32 index;
            u32 timestamp;
            u32 timestamp_h;
			u32 addr[2];
        } g_bufinfo;

        struct {
            u8 bufs;
            u8 planes;
            u32 size[2];
        } queue;
        u8 data[18];
    } msg;
};
#endif /* __SCS_COM_H__ */
