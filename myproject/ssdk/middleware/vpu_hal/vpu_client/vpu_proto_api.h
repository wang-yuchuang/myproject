/*
 * Copyright (c) 2022 Semidrive Semiconductor, Inc.
 * All rights reserved.
 */

#ifndef __VPU_PROTO_API_H__
#define __VPU_PROTO_API_H__

#include "../../vpu_service/vpu_svc_cmd.h"
#include "vpu_proto_tool.h"
#include <cmsis_os2.h>

typedef enum {
    STREAM_FILL,
    STREAM_UPDATE,
    STREAM_END,
    STREAM_END_CLEAR, // for end loop decoding
} StreamUpFlag;

typedef struct vpu_buffer_t {
    uint32_t size;
    uint64_t phys_addr;
    uint64_t base;
    uint64_t virt_addr;
    uint64_t dma_addr;
    uint64_t attachment;
    uint64_t sgt;
    int32_t buf_handle;
    int32_t data_direction;
} vpu_buffer_t;

typedef struct vpu_dec_proto_info {
    // dec open
    Vpu__DecOpenParam dec_open_param;

    // seq init
    Vpu__DecInitialInfo init_output_info;

    // frame buffer
    Vpu__FrameBufferAllocInfo dec_fb_alloc_info_dec;
    Vpu__FrameBufferAllocInfo dec_fb_alloc_info_disp;
} vpu_dec_proto_info_t;

typedef struct vpu_enc_proto_info {
    // enc open proto
    Vpu__EncOpenParam enc_open_param;
    Vpu__EncStdParam enc_std_param;
    Vpu__EncStdParam__EncAvcParam enc_avc_param;
    Vpu__EncStdParam__EncMp4Param enc_mpeg4_param;
    Vpu__EncStdParam__EncH263Param enc_h263_param;
    Vpu__AvcPpsParam enc_pps_param_1;

    // enc init proto
    Vpu__EncInitialInfo enc_init_info;

    // enc regist buffer proto
    Vpu__FrameBufferAllocInfo fb_alloc_info_src;
    Vpu__FrameBuffer fb_src;
} vpu_enc_proto_info_t;

typedef struct vpu_dec_param {
    int cbcr_interleave;
    int cbcr_order;
    int bitstream_format;
    int mp4_class;
    int map_type; // for output
    int sram_enable;
} vpu_dec_param_t;

typedef struct vpu_dec_scaler_info {
    int enable_scaler;
    uint32_t scale_width;
    uint32_t scale_height;
} vpu_dec_scaler_info_t;

typedef struct vpu_dec_disp_flag {
    uint32_t set_disp_flag;
    uint32_t clr_disp_flag;
    osMutexId_t disp_flag_mutex;
} vpu_dec_disp_flag_t;

typedef struct vpu_dec_color_aspect_info {
    int color_desc_present;
    int primaries;
    int transfer;
    int coeffs;
    int full_range;
} vpu_dec_color_aspect_info_t;

typedef struct vpu_dec_hdr_info {
    int master_meta_present;
    int display_primaries_x[3];
    int display_primaries_y[3];
    int white_point_x;
    int white_point_y;
    int max_display_mastering_luminance;
    int min_display_mastering_luminance;
    int content_meta_present;
    int max_content_light_level;
    int max_frame_ave_light_level;
} vpu_dec_hdr_info_t;

typedef struct vpu_dec_seq_init_info {
    int pic_width;
    int pic_height;
    int crop_left;
    int crop_top;
    int crop_right;
    int crop_bottom;
    int frame_rate;
    int min_frame_buffer_count;
    vpu_dec_color_aspect_info_t color_aspect;
    vpu_dec_hdr_info_t h265_hdr;
} vpu_dec_seq_init_info_t;

typedef struct vpu_dec_frame_output_info {
    int pic_type;
    int index_frame_display;
    int index_frame_decoded;
    int index_frame_prescan;
    int chunk_reuse_required;
    int frame_display_flag;
    int disp_pic_width;
    int disp_pic_height;
    int crop_left;
    int crop_top;
    int crop_right;
    int crop_bottom;
    int sequence_changed;
    int disp_frame_index;
    int disp_frame_size;
    void *disp_frame_addr;
    int vp9_res_change;
    int vp9_res_change_frame_idx;
    vpu_dec_color_aspect_info_t color_aspect;
    vpu_dec_hdr_info_t h265_hdr;
} vpu_dec_frame_output_info_t;

typedef struct vpu_enc_mpeg4_param {
    int mp4_data_partition_enable;
    int mp4_reversible_vlc_enable;
    int mp4_hec_enable;
    int mp4_intra_dc_vlc_thr;
    int mp4_verid;
} vpu_enc_mpeg4_param_t;

typedef struct vpu_enc_h263_param {
    int h263_annex_i_enable;
    int h263_annex_j_enable;
    int h263_annex_k_enable;
    int h263_annex_t_enable;
} vpu_enc_h263_param_t;

typedef struct vpu_enc_avc_param {
    int frame_cropping_flag;
    int frame_crop_left;
    int frame_crop_right;
    int frame_crop_top;
    int frame_crop_bottom;
    int profile;
    int level;
} vpu_enc_avc_param_t;

typedef struct vpu_enc_param {
    int bitstream_format;
    int cbcr_interleave;
    int nv21;
    int cbcr_order;
    int pic_width;
    int pic_height;
    int frame_rate_info;
    int bit_rate;
    int frame_skip_disable;
    int gop_size;
    int chroma_format400; // yuv420 or 400
    int intra_refresh;
    int rc_enable;
    int sram_enable;
    union {
        vpu_enc_mpeg4_param_t mpeg4_param;
        vpu_enc_h263_param_t h263_param;
        vpu_enc_avc_param_t avc_param;
    } enc_std_param;
} vpu_enc_param_t;

typedef struct vpu_enc_frame_param {
    int quant_param;
    int is_key_frame;
    uint32_t new_bit_rate;
    uint32_t new_frame_rate;
    uint32_t new_gop_size;

    uint32_t input_buf_phys_addr;
    unsigned char *input_buf_virtual_addr;
    uint32_t input_buf_size;

    uint32_t output_buf_phys_addr;
    uint32_t output_buf_size;
} vpu_enc_frame_param_t;

typedef struct vpu_enc_frame_output_info {
    int output_pic_type;
    unsigned char *output_buf;
    int output_buf_size;
} vpu_enc_frame_output_info_t;

typedef struct vpu_instance {
    // isntance
    int core_idx;
    int product_id;
    uint32_t vpu_handle;

    vpu_dec_param_t *dec_param;
    vpu_enc_param_t *enc_param;
    vpu_dec_disp_flag_t dec_disp_flag;

    // common buffer
    uint32_t rd_ptr;
    uint32_t wr_ptr;
    uint32_t wave_stream_count;
    vpu_buffer_t vb_stream;
    vpu_buffer_t vb_stream_wave_more;

    // frame buffer
    vpu_buffer_t dec_vb_frame_dec[MAX_GDI_IDX];
    vpu_buffer_t dec_vb_frame_disp[MAX_GDI_IDX]; // for wave
    vpu_buffer_t enc_vb_frame[MAX_GDI_IDX];      // for refer
    vpu_buffer_t enc_vb_src;

    // proto info
    vpu_dec_proto_info_t vpu_dec_proto_info;
    vpu_enc_proto_info_t vpu_enc_proto_info;

    // proto msg
    vpu_ipcc_data_t pack;
    void *msg_buf;

    // private ptr
    void *private;
} vpu_instance_t;

// instance
vpu_instance_t *vpu_proto_init(int core_idx, uint32_t bs_buf_size);
int vpu_proto_release(vpu_instance_t *vpu);

// dec
int vpu_proto_dec_open(vpu_instance_t *vpu, vpu_dec_param_t *dec_param);
int vpu_proto_dec_fill_bit_stream_buffer(vpu_instance_t *vpu, StreamUpFlag flag,
                                         unsigned char *data, int data_size);
int vpu_proto_dec_seq_init(vpu_instance_t *vpu, vpu_dec_seq_init_info_t *init_info);
/*
 *buf_count equals 0 means automatic calculation,
 *scaler_info only for vpu wave
 */
int vpu_proto_dec_register_frame_buffer(vpu_instance_t *vpu, vpu_buffer_t *vb, int buf_count,
                                        int is_vp9_res_change, int vp9_res_change_idx,
                                        vpu_dec_scaler_info_t *scaler_info);
int vpu_proto_dec_one_frame(vpu_instance_t *vpu, int is_thumbnail_mode,
                            vpu_dec_frame_output_info_t *output_info);
int vpu_proto_dec_flush(vpu_instance_t *vpu);
int vpu_proto_dec_clr_all_disp_flag(vpu_instance_t *vpu);
int vpu_proto_dec_set_disp_flag(vpu_instance_t *vpu, int index, int is_clear);

// dma buffer
int vpu_proto_alloc_dma_buffer(vpu_instance_t *vpu, vpu_buffer_t *vb);
void vpu_proto_free_dma_buffer(vpu_instance_t *vpu, vpu_buffer_t *vb);

// enc
int vpu_proto_enc_open(vpu_instance_t *vpu, vpu_enc_param_t *enc_param);
int vpu_proto_enc_register_frame_buffer(vpu_instance_t *vpu);
// only for avc & mpeg4
int vpu_proto_enc_get_cfg_frame(vpu_instance_t *vpu, unsigned char *data, int data_size);
int vpu_proto_enc_one_frame(vpu_instance_t *vpu, vpu_enc_frame_param_t *enc_frame_param,
                            vpu_enc_frame_output_info_t *output_info);

// common func
int vpu_proto_sw_reset(vpu_instance_t *vpu);
#endif //__VPU_PROTO_API_H__