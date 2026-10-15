/**
 * @file sdrv_jpu.h
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: semidriver jpu header file
 *
 * Revision History:
 * -----------------
 */
#ifndef __INCLUDE_SDRV_JPU_H__
#define __INCLUDE_SDRV_JPU_H__
/* =========================== include file session ======================== */
#include <cmsis_os2.h>

#include "jdi.h"
#include "jpuapi.h"
#include "jpuapifunc.h"

#include "vpu.h"

#ifdef __cplusplus
extern "C"{
#endif /* __cplusplus */
/**
* @defgroup XXX XXX
* @brief  XXX service
* @{
*/


#define MAX_FRAME_BUFFER_NUM 19
#define MAX_JPU_CHAN_INS_NUM 4

typedef void (*fill_input_data)(void *private_ptr, int expect_size, void *input_buffer,
                                int *fill_input_size);
typedef void (*read_output_data)(void *private_ptr, void *output_buffer, int output_size);

typedef enum {
    PACK_FORMAT_NONE, // planar format
    PACK_FORMAT_NV12, // semi-planar format
    PACK_FORMAT_NV21, // semi-planar format
    PACK_FORMAT_YUYV, // packed format
    PACK_FORMAT_YVYU, // packed format
    PACK_FORMAT_UYVY, // packed format
    PACK_FORMAT_VYUY, // packed format
    PACK_FORMAT_AYUV  // packed format
} hal_jpu_pack_format_t;

typedef enum {
    FEED_METHOD_FIXED_SIZE, // read a fixed data every time
    FEED_METHOD_FRAME_SIZE, // read a frame every time
    FEED_METHOD_MAX
} hal_jpu_dec_feed_method_t;

typedef struct dec_output_info {
    int pic_width;
    int pic_height;
    FrameFormat pic_format;

    int output_width;  // hw 32-bit align
    int output_height; // hw 32-bit align
    int output_cbcrInterleave;
    int output_packed_format;
    FrameFormat output_format;
    int output_frame_size;

    int bit_depth;
    int current_output_index;
    int decode_finish;

    Uint8 *y_ptr;
    Uint8 *u_ptr;
    Uint8 *v_ptr;
    int y_size;
    int u_size;
    int v_size;
} hal_jpu_dec_output_info_t;

typedef struct enc_input_info {
    // write by user
    int pic_width;
    int pic_height;

    // Source Format (0 : 4:2:0, 1 : 4:2:2, 2 : 4:4:0, 3 : 4:4:4, 4 : 4:0:0)
    FrameFormat frame_format;
    /*
        0-planar, 1-NV12,NV16(CbCr interleave) 2-NV21,NV61(CbCr alternative)
        3-YUYV, 4-UYVY, 5-YVYU, 6-VYUY, 7-YUV packed (444 only)
    */
    int packageFormat;

    // write by jpu
    int frameSize;
    int lumaSize;
    int chromaSize;
    int lumaLineWidth;
    int lumaHeight;
    int chromaLineWidth;
    int chromaHeight;
} hal_jpu_enc_input_info_t;

typedef struct hal_jpu_hw_instance_t {
    jpu_buffer_t bs_stream;
    fill_input_data fill_buffer_callback;
    read_output_data read_buffer_callback;

    hal_jpu_dec_output_info_t dec_output;
    hal_jpu_dec_feed_method_t dec_feedong_method;
    int jpu_dec_init_success;
    jpu_buffer_t dec_frame_buffer_stream;
    FrameBuffer dec_frameBuf[MAX_FRAME_BUFFER_NUM];

    hal_jpu_enc_input_info_t enc_input;
    int jpu_enc_init_success;
    jpu_buffer_t enc_frame_buffer_stream;
    FrameBuffer enc_frameBuf[MAX_FRAME_BUFFER_NUM];

    JpgInst *handle;
    void *user_private_ptr; // for user
} hal_jpu_hw_instance_t;

hal_jpu_hw_instance_t *hal_jpu_hw_init(void);
int hal_jpu_decompress(hal_jpu_hw_instance_t *jpu);
int hal_jpu_encompress(hal_jpu_hw_instance_t *jpu);
int hal_jpu_hw_release(hal_jpu_hw_instance_t *jpu);

/* =========================== marco define session ======================== */
/* =========================== enumeration define session ================== */
/* =========================== structure define session ==================== */
/** jpu channel instance structure */
typedef struct jpu_chan_ins {
    JpgInst *handle;
    jpu_buffer_t bs_stream;
    hal_jpu_dec_feed_method_t dec_feedong_method;
    int32_t type;                                       /**< 0: dec; 1: enc */
    osEventFlagsId_t chan_event;
    uint16_t chan_id;
    bool is_working;
    /* frame bs for dec or dec */
    union {
        jpu_buffer_t dec_frame_buffer_stream;
        jpu_buffer_t enc_frame_buffer_stream;
    };

    /* frame buf for dec or enc */
    union {
        FrameBuffer dec_frameBuf[MAX_FRAME_BUFFER_NUM];
        FrameBuffer enc_frameBuf[MAX_FRAME_BUFFER_NUM];
    };

    /* output info for dec or input info for enc */
    union {
        hal_jpu_dec_output_info_t dec_output;
        hal_jpu_enc_input_info_t enc_input;
    };

    // async function notify callback function
    void (*sync_callback)(uint16_t chanId, int32_t codecRet, void* usrCtx);
    void* sync_args;

    fill_input_data fill_buffer_callback;
    read_output_data read_buffer_callback;
    void* user_private_ptr;
}jpu_chan_ins_t;


/** jpu sdrv structure */
typedef struct sdrv_jpu_drv {
    vpu_dev_t dev;
    osEventFlagsId_t jpu_event;
    osEventFlagsId_t work_event;
    osThreadId_t thread_id;
    jpu_chan_ins_t* chan[4];
    osMutexId_t mutex;
    uint16_t chan_id;
    osMessageQueueId_t queue;
    uint32_t api_version;
    uint32_t hw_version;
    uint32_t product_id;
    bool is_running;
}sdrv_jpu_drv_t;

/* =========================== function declare session ==================== */
/* =========================== global value define session ================= */
/* =========================== static value define session ================= */
/* =========================== function implementation session ============= */
vpu_dev_t *jpu_probe(sdrv_jpu_drv_t* jpuDrv, vpu_config_t* cfg);
/** @} */
#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __INCLUDE_SDRV_JPU_H__ */
/* end of header file sdrv_jpu.h */