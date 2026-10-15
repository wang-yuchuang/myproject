/**
 * @file vpu.h
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: vpu header file
 *
 * Revision History:
 * -----------------
 */
#ifndef __INCLUDE_VPU_H__
#define __INCLUDE_VPU_H__
/* =========================== include file session ======================== */
#include <types.h>

#include <jpuapi.h>

#ifdef __cplusplus
extern "C"{
#endif /* __cplusplus */
/**
* @defgroup XXX XXX
* @brief  XXX service
* @{
*/
/* =========================== marco define session ======================== */
#define VPU_REQ_DEC_CHAN_TYPE    0
#define VPU_REQ_ENC_CHAN_TYPE    1

/* =========================== typedef define session ====================== */
typedef struct vpu_device vpu_dev_t;

typedef void (*fill_input_data)(void *private_ptr, int expect_size, void *input_buffer,
                                int *fill_input_size);
typedef void (*read_output_data)(void *private_ptr, void *output_buffer, int output_size);

/** async interface callback notify */
typedef void (*cb_t)(uint16_t chanId, int32_t codecRet, void* args);
/* =========================== enumeration define session ================== */
/** codec return value */
typedef enum {
    VPU_CODEC_RET_ERROR = -1,
    VPU_CODEC_RET_FRAME_DONE = 0,
    VPU_CODEC_RET_BITSTREAM_DONE = 1,
}vpu_codec_ret_enum_t;

/** codec control cmd */
typedef enum {
    VPU_CODEC_CTLCMD_GET_DECOUT_DATA,
    VPU_CODEC_CTLCMD_DEC_RESET,
}vpu_codec_ctlCmd_enum_t;
/* =========================== global value define session ================= */
/* =========================== static value define session ================= */

/* =========================== structure define session ==================== */
typedef struct vpu_config {
    char* dev_name;
    int32_t irq_num;
    addr_t base;
}vpu_config_t;

typedef struct encoder_opt {
    int32_t pic_width;
    int32_t pic_height;
    FrameFormat frame_format;
    int32_t package_format;

    fill_input_data fill_buffer_callback;
    read_output_data read_buffer_callback;
    void* user_private_ptr;
}encoder_opt_t;

typedef struct decoder_opt {
    int32_t dec_feeding_method;
    int32_t output_cbcrInterleave;
    int32_t output_packed_format;
    FrameFormat output_format;

    fill_input_data fill_buffer_callback;
    void* user_private_ptr;
}decoder_opt_t;

typedef struct decoder_output {
    bool is_valid;
    uint8_t* y_ptr;
    uint8_t* u_ptr;
    uint8_t* v_ptr;
    int32_t output_width;
    int32_t output_height;
    int32_t pic_width;
    int32_t pic_height;
}decoder_output_t;

typedef struct vpu_ops {
    int32_t (*init)(vpu_dev_t* dev);
    int32_t (*acquire_chan)(vpu_dev_t* dev, int32_t chanType, void* optData);
    int32_t (*release_chan)(vpu_dev_t* dev, uint16_t chanId);
    int32_t (*chan_codec_sync)(vpu_dev_t* dev, uint16_t chanId);
    int32_t (*chan_codec_async)(vpu_dev_t* dev, uint16_t chanId, cb_t cb, void* usrCtx);
    int32_t (*device_ctrl)(vpu_dev_t* dev, uint16_t chanId, uint16_t ctlCmd, void* ctlData);
    int32_t (*deinit)(vpu_dev_t* dev);
}vpu_ops_t;

typedef struct vpu_device {
    char* dev_name;
    uint32_t is_init;
    vpu_config_t* cfg_ptr;
    vpu_ops_t* ops;
    void* priv;
}vpu_dev_t;


/* =========================== function declare session ==================== */
extern int32_t vpu_init(vpu_dev_t* dev);
extern int32_t vpu_acquire_chan(vpu_dev_t* dev, int32_t chanType, void* optData);
extern int32_t vpu_release_chan(vpu_dev_t* dev, uint16_t chanId);
extern int32_t vpu_chan_codec_sync(vpu_dev_t* dev, uint16_t chanId);
extern int32_t vpu_chan_codec_async(vpu_dev_t* dev, uint16_t chanId, cb_t cb, void* usrCtx);
extern int32_t vpu_deinit(vpu_dev_t* dev);
extern int32_t vpu_device_ctrl(vpu_dev_t* dev, uint16_t chanId, uint16_t ctlCmd, void* ctlData);
/** @} */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __INCLUDE_VPU_H__ */
/* end of header file vpu.h */
