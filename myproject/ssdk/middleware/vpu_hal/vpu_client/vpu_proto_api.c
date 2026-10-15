/*
 * Copyright (c) 2022 Semidrive Semiconductor, Inc.
 * All rights reserved.
 */

#include <res_loader.h>
#include <stdlib.h>
#include <string.h>
#include <debug.h>
#include <malloc.h>
#include <driver/memmap.h>
#include <arch/cache.h>

#include "vpu_config.h"
#include "vpu_proto_api.h"
#include "vpu_proto_client.h"

#define INITIAL_DELAY 1000

#define VPU_DEFAULT_EXTRA_FRAME_BUFFER_NUM 1
#define VPU_PROTO_BUFFER_SIZE 4096

// #define VPU_PROTO_DEBUG

#ifdef VPU_PROTO_DEBUG
#define VPU_PROTO_DEBUG_LEVEL CRITICAL
#else
#define VPU_PROTO_DEBUG_LEVEL INFO
#endif

#define SEND_PROTO_AND_CHECK_RET_SUCCESS(INSTANCE, PROTO_MSG_HEAD, MSG, RET_MSG, CMD,              \
                                         ERROR_HANDLE)                                             \
    do {                                                                                           \
        int ret = 0;                                                                               \
        int msg_len = 0;                                                                           \
        msg_len = PROTO_MSG_HEAD##__get_packed_size(&MSG);                                         \
        PROTO_MSG_HEAD##__pack(&MSG, INSTANCE->msg_buf);                                           \
                                                                                                   \
        memset(&INSTANCE->pack, 0x0, sizeof(vpu_ipcc_data_t));                                     \
        INSTANCE->pack.cmd = CMD;                                                                  \
                                                                                                   \
        if (msg_len <= VPU_IPCC_DATA_LEN_MAX) {                                                    \
            INSTANCE->pack.cmd_param_full_length = msg_len;                                        \
            INSTANCE->pack.current_packed_end_offset = msg_len;                                    \
                                                                                                   \
            memcpy(INSTANCE->pack.cmd_param, INSTANCE->msg_buf, msg_len);                          \
                                                                                                   \
            ret = vpu_proto_send_msg_to_service(INSTANCE->core_idx, &INSTANCE->pack);              \
            if (ret < 0) {                                                                         \
                ssdk_printf(SSDK_CRIT, "%s send failed\n", #MSG);                                       \
                goto MSG##_ERROR;                                                                  \
            }                                                                                      \
        } else {                                                                                   \
            ssdk_printf(SSDK_CRIT, "[%s][%d] TODO: support msg split\n", __func__, __LINE__);           \
            goto MSG##_ERROR;                                                                      \
        }                                                                                          \
                                                                                                   \
        RET_MSG = PROTO_MSG_HEAD##__unpack(NULL, INSTANCE->pack.cmd_param_full_length,             \
                                           INSTANCE->pack.cmd_param);                              \
        if (!RET_MSG) {                                                                            \
            ssdk_printf(SSDK_CRIT, "[%s][%d] %s__unpack failed\n", __func__, __LINE__,                  \
                    #PROTO_MSG_HEAD);                                                              \
            goto MSG##_ERROR;                                                                      \
        }                                                                                          \
                                                                                                   \
        if (!RET_MSG->has_ret_code) {                                                              \
            ssdk_printf(SSDK_CRIT, "[%s][%d] %s invailed ret code\n", __func__, __LINE__, #CMD);        \
            PROTO_MSG_HEAD##__free_unpacked(RET_MSG, NULL);                                        \
            RET_MSG = NULL;                                                                        \
            goto MSG##_ERROR;                                                                      \
        }                                                                                          \
                                                                                                   \
        if (RET_MSG->ret_code != (int)RETCODE_SUCCESS) {                                                \
            ssdk_printf(SSDK_CRIT, "[%s][%d] %s failed error code:0x%x\n", __func__, __LINE__, #CMD,    \
                    RET_MSG->ret_code);                                                            \
            PROTO_MSG_HEAD##__free_unpacked(RET_MSG, NULL);                                        \
            RET_MSG = NULL;                                                                        \
            goto MSG##_ERROR;                                                                      \
        }                                                                                          \
        break;                                                                                     \
        MSG##_ERROR : ERROR_HANDLE;                                                                \
                                                                                                   \
    } while (0)

#define SEND_PROTO_AND_CHECK_HAS_RET_CODE(INSTANCE, PROTO_MSG_HEAD, MSG, RET_MSG, CMD,             \
                                          ERROR_HANDLE)                                            \
    do {                                                                                           \
        int ret = 0;                                                                               \
        int msg_len = 0;                                                                           \
        msg_len = PROTO_MSG_HEAD##__get_packed_size(&MSG);                                         \
        PROTO_MSG_HEAD##__pack(&MSG, INSTANCE->msg_buf);                                           \
                                                                                                   \
        memset(&INSTANCE->pack, 0x0, sizeof(vpu_ipcc_data_t));                                     \
        INSTANCE->pack.cmd = CMD;                                                                  \
                                                                                                   \
        if (msg_len <= VPU_IPCC_DATA_LEN_MAX) {                                                    \
            INSTANCE->pack.cmd_param_full_length = msg_len;                                        \
            INSTANCE->pack.current_packed_end_offset = msg_len;                                    \
                                                                                                   \
            memcpy(INSTANCE->pack.cmd_param, INSTANCE->msg_buf, msg_len);                          \
                                                                                                   \
            ret = vpu_proto_send_msg_to_service(INSTANCE->core_idx, &INSTANCE->pack);              \
            if (ret < 0) {                                                                         \
                ssdk_printf(SSDK_CRIT, "%s send failed\n", #MSG);                                       \
                goto MSG##_ERROR;                                                                  \
            }                                                                                      \
        } else {                                                                                   \
            ssdk_printf(SSDK_CRIT, "[%s][%d] TODO: support msg split\n", __func__, __LINE__);           \
            goto MSG##_ERROR;                                                                      \
        }                                                                                          \
                                                                                                   \
        RET_MSG = PROTO_MSG_HEAD##__unpack(NULL, INSTANCE->pack.cmd_param_full_length,             \
                                           INSTANCE->pack.cmd_param);                              \
        if (!RET_MSG) {                                                                            \
            ssdk_printf(SSDK_CRIT, "[%s][%d] %s__unpack failed\n", __func__, __LINE__,                  \
                    #PROTO_MSG_HEAD);                                                              \
            goto MSG##_ERROR;                                                                      \
        }                                                                                          \
                                                                                                   \
        if (!RET_MSG->has_ret_code) {                                                              \
            ssdk_printf(SSDK_CRIT, "[%s][%d] %s invailed ret code\n", __func__, __LINE__, #CMD);        \
            PROTO_MSG_HEAD##__free_unpacked(RET_MSG, NULL);                                        \
            RET_MSG = NULL;                                                                        \
            goto MSG##_ERROR;                                                                      \
        }                                                                                          \
                                                                                                   \
        break;                                                                                     \
        MSG##_ERROR : ERROR_HANDLE;                                                                \
                                                                                                   \
    } while (0)

int _vpu_proto_allocate_dma_memory(vpu_buffer_t *vb)
{
    if (!vb) {
        ssdk_printf(SSDK_CRIT, "[%s][%d] invailed vb:%p", __func__, __LINE__, vb);
        return -1;
    }

    vb->virt_addr = (unsigned long)memalign(1024, vb->size);
    if (!vb->virt_addr) {
        ssdk_printf(SSDK_CRIT, "[%s][%d] fail to allocate vb mem! \n", __func__, __LINE__);
        return -1;
    }

    vb->phys_addr = (unsigned long)p2ap((paddr_t)vb->virt_addr);
    vb->base = vb->phys_addr;

    ssdk_printf(SSDK_INFO, "[%s][%d] base %llx phy addr=%llx, virt addr=%llx, size=%d \n",
            __func__, __LINE__, vb->base, vb->phys_addr, vb->virt_addr, vb->size);

    return 0;
}

void _vpu_proto_free_dma_memory(vpu_buffer_t *vb)
{
    if (!vb || vb->size == 0)
        return;

    ssdk_printf(SSDK_INFO, "[%s][%d] base %llx phy addr=%llx, virt addr=%llx, size=%d\n",
            __func__, __LINE__, vb->base, vb->phys_addr, vb->virt_addr, vb->size);

    free(((void *)((uint32_t)vb->virt_addr)));
    memset(vb, 0x0, sizeof(vpu_buffer_t));

    return;
}

void *_vpu_proto_memcpy_form_cpu_to_device(void *dst, const void *src, int count)
{
    void *ret = memcpy(dst, src, count);
    arch_clean_cache_range((addr_t)dst, count);
    return ret;
}

void *_vpu_proto_memcpy_form_device_to_cpu(void *dst, const void *src, int count)
{
    arch_invalidate_cache_range((addr_t)src, count);
    void *ret = memcpy(dst, src, count);
    return ret;
}

vpu_instance_t *vpu_proto_init(int core_idx, uint32_t bs_buf_size)
{
    if (core_idx != VPU_CODA988_IDX && core_idx != VPU_WAVE412_IDX) {
        ssdk_printf(SSDK_CRIT, "%s: %d invailed core idx:%d\n", __func__, __LINE__, core_idx);
        return NULL;
    }

    ssdk_printf(SSDK_INFO, "%s: %d\n", __func__, __LINE__);

    vpu_instance_t *vpu = malloc(sizeof(vpu_instance_t));
    if (!vpu) {
        ssdk_printf(SSDK_CRIT, "[%s][%d] malloc failed\n", __func__, __LINE__);
        return NULL;
    }

    memset(vpu, 0x00, sizeof(vpu_instance_t));

    vpu->core_idx = core_idx;

    if (vpu->core_idx == VPU_CODA988_IDX) {
        vpu->product_id = PRODUCT_ID_980;
    } else if (vpu->core_idx == VPU_WAVE412_IDX) {
        vpu->product_id = PRODUCT_ID_412;
    }

    vpu->msg_buf = malloc(VPU_PROTO_BUFFER_SIZE);
    if (!vpu->msg_buf) {
        ssdk_printf(SSDK_CRIT, "[%s][%d] malloc failed\n", __func__, __LINE__);
        goto INIT_ERR;
    }

    // vbstream
    vpu->vb_stream.size = VPU_ALIGN4096(bs_buf_size);
    if (_vpu_proto_allocate_dma_memory(&vpu->vb_stream) < 0) {
        ssdk_printf(SSDK_CRIT, "fail to allocate bitstream buffer\n");
        goto INIT_ERR;
    }

    ssdk_printf(SSDK_INFO, "[Main] vbStream base 0x%llx, phy 0x%llx, virt 0x%llx\n",
            vpu->vb_stream.base, vpu->vb_stream.phys_addr, vpu->vb_stream.virt_addr);

    // vb stream wave more
    if (vpu->core_idx == VPU_WAVE412_IDX) {
        vpu->vb_stream_wave_more.size = VPU_ALIGN4096(bs_buf_size);
        if (_vpu_proto_allocate_dma_memory(&vpu->vb_stream_wave_more) < 0) {
            ssdk_printf(SSDK_CRIT, "fail to allocate bitstream buffer\n");
            goto INIT_ERR;
        }

        ssdk_printf(SSDK_INFO,
                "[Main] vb_stream_wave_more base 0x%llx, phy 0x%llx, virt 0x%llx\n",
                vpu->vb_stream_wave_more.base, vpu->vb_stream_wave_more.phys_addr,
                vpu->vb_stream_wave_more.virt_addr);
    }

    vpu->dec_disp_flag.disp_flag_mutex = osMutexNew(NULL);

    return vpu;

INIT_ERR:
    if (vpu->vb_stream.size)
        _vpu_proto_free_dma_memory(&vpu->vb_stream);

    if (vpu->vb_stream_wave_more.size)
        _vpu_proto_free_dma_memory(&vpu->vb_stream_wave_more);

    if (vpu->msg_buf)
        free(vpu->msg_buf);

    free(vpu);

    return NULL;
}

void vpu_proto_dec_close(vpu_instance_t *vpu)
{
    int ret;
    int msg_len;

    if (!vpu || !vpu->vpu_handle)
        return;

    Vpu__DecClose dec_close_msg = VPU__DEC_CLOSE__INIT;
    Vpu__DecHandle dec_handle_msg = VPU__DEC_HANDLE__INIT;

    dec_close_msg.handle = &dec_handle_msg;
    dec_handle_msg.handle = vpu->vpu_handle;

    // proto msg
    msg_len = vpu__dec_close__get_packed_size(&dec_close_msg);
    vpu__dec_close__pack(&dec_close_msg, vpu->msg_buf);

    memset(&vpu->pack, 0x0, sizeof(vpu_ipcc_data_t));
    vpu->pack.cmd = VPU_IPC_DEC_CLOSE;
    vpu->pack.cmd_param_full_length = msg_len;
    vpu->pack.current_packed_end_offset = msg_len;

    memcpy(vpu->pack.cmd_param, vpu->msg_buf, msg_len);

    ret = vpu_proto_send_msg_to_service(vpu->core_idx, &vpu->pack);
    if (ret < 0) {
        ssdk_printf(SSDK_CRIT, "VPU_DecClose failed\n");
    }

    for (int i = 0; i < MAX_GDI_IDX; i++) {
        if (vpu->dec_vb_frame_dec[i].size) {
            _vpu_proto_free_dma_memory(&vpu->dec_vb_frame_dec[i]);
            vpu->dec_vb_frame_dec[i].size = 0;
        }

        if (vpu->dec_vb_frame_disp[i].size) {
            _vpu_proto_free_dma_memory(&vpu->dec_vb_frame_disp[i]);
            vpu->dec_vb_frame_disp[i].size = 0;
        }
    }

    return;
}

void vpu_proto_enc_close(vpu_instance_t *vpu)
{
    int ret;
    int msg_len;

    if (!vpu || !vpu->vpu_handle)
        return;

    Vpu__EncClose enc_close_msg = VPU__ENC_CLOSE__INIT;
    Vpu__EncHandle enc_handle_msg = VPU__ENC_HANDLE__INIT;

    enc_close_msg.handle = &enc_handle_msg;
    enc_handle_msg.handle = vpu->vpu_handle;

    // proto msg
    msg_len = vpu__enc_close__get_packed_size(&enc_close_msg);
    vpu__enc_close__pack(&enc_close_msg, vpu->msg_buf);

    memset(&vpu->pack, 0x0, sizeof(vpu_ipcc_data_t));
    vpu->pack.cmd = VPU_IPC_ENC_CLOSE;
    vpu->pack.cmd_param_full_length = msg_len;
    vpu->pack.current_packed_end_offset = msg_len;

    memcpy(vpu->pack.cmd_param, vpu->msg_buf, msg_len);

    ret = vpu_proto_send_msg_to_service(vpu->core_idx, &vpu->pack);
    if (ret < 0) {
        ssdk_printf(SSDK_CRIT, "VPU_EncClose failed\n");
    }

    for (int i = 0; i < MAX_GDI_IDX; i++) {
        if (vpu->enc_vb_frame[i].size > 0) {
            _vpu_proto_free_dma_memory(&vpu->enc_vb_frame[i]);
            vpu->enc_vb_frame[i].size = 0;
        }
    }

    if (vpu->enc_vb_src.size > 0) {
        _vpu_proto_free_dma_memory(&vpu->enc_vb_src);
        vpu->enc_vb_src.size = 0;
    }
}

int vpu_proto_release(vpu_instance_t *vpu)
{
    if (!vpu)
        return 0;

    if (vpu->dec_param) {
        vpu_proto_dec_close(vpu);
        free(vpu->dec_param);
        vpu->dec_param = NULL;
    } else if (vpu->enc_param) {
        vpu_proto_enc_close(vpu);
        free(vpu->enc_param);
        vpu->enc_param = NULL;
    }

    for (int i = 0; i < MAX_GDI_IDX; i++) {
        if (vpu->dec_vb_frame_dec[i].size)
            _vpu_proto_free_dma_memory(&vpu->dec_vb_frame_dec[i]);
        if (vpu->dec_vb_frame_disp[i].size)
            _vpu_proto_free_dma_memory(&vpu->dec_vb_frame_disp[i]);
        if (vpu->enc_vb_frame[i].size)
            _vpu_proto_free_dma_memory(&vpu->enc_vb_frame[i]);
    }

    osMutexDelete(vpu->dec_disp_flag.disp_flag_mutex);

    if (vpu->enc_vb_src.size)
        _vpu_proto_free_dma_memory(&vpu->enc_vb_src);

    if (vpu->vb_stream.size)
        _vpu_proto_free_dma_memory(&vpu->vb_stream);

    if (vpu->vb_stream_wave_more.size)
        _vpu_proto_free_dma_memory(&vpu->vb_stream_wave_more);

    if (vpu->msg_buf)
        free(vpu->msg_buf);

    free(vpu);

    return 0;
}

int vpu_proto_dec_seq_init(vpu_instance_t *vpu, vpu_dec_seq_init_info_t *init_info)
{
    if (!vpu) {
        ssdk_printf(SSDK_CRIT, "[%s][%d] invailed input param, vpu:%p\n", __FUNCTION__, __LINE__, vpu);
        return -1;
    }

    if (!vpu->vpu_handle || !vpu->dec_param || !init_info) {
        ssdk_printf(SSDK_CRIT,
                    "[%s][%d] invailed input param, vpu:%p, vpu_handle:%x, dec_param:%p, init_info:%p\n",
                    __FUNCTION__, __LINE__, vpu, vpu->vpu_handle, vpu->dec_param, init_info);
        return -1;
    }

    // VPU_IPC_DEC_SEQINIT
    Vpu__DecSeqInit *seq_init_msg_ret = NULL;
    Vpu__DecSeqInit seq_init_msg = VPU__DEC_SEQ_INIT__INIT;
    Vpu__DecHandle dec_handle_msg = VPU__DEC_HANDLE__INIT;

    seq_init_msg.handle = &dec_handle_msg;
    dec_handle_msg.handle = vpu->vpu_handle;

    // proto VPU_IPC_DEC_SEQINIT msg
    SEND_PROTO_AND_CHECK_HAS_RET_CODE(vpu, vpu__dec_seq_init, seq_init_msg, seq_init_msg_ret,
                                      VPU_IPC_DEC_SEQINIT, return -1);

    if (seq_init_msg_ret->ret_code != (int)RETCODE_SUCCESS) {
        if (!seq_init_msg_ret->initial_info ||
            !seq_init_msg_ret->initial_info->has_seq_init_err_reason) {
            ssdk_printf(SSDK_CRIT, "[%s][%d] VPU_IPC_DEC_SEQINIT failed error code:0x%x\n", __func__,
                    __LINE__, seq_init_msg_ret->ret_code);
        } else {
            ssdk_printf(SSDK_CRIT, "[ERROR] Failed to SEQ_INIT(ERROR REASON: %d(0x%x) error code:0x%c\n",
                    seq_init_msg_ret->initial_info->seq_init_err_reason,
                    seq_init_msg_ret->initial_info->seq_init_err_reason,
                    seq_init_msg_ret->ret_code);
        }

        vpu__dec_seq_init__free_unpacked(seq_init_msg_ret, NULL);

        return -1;
    }

    if (!seq_init_msg_ret->initial_info || !seq_init_msg_ret->initial_info->pic_crop_rect) {
        ssdk_printf(SSDK_CRIT, "[%s][%d] VPU_IPC_DEC_SEQINIT invailed ret msg\n", __func__, __LINE__);
        vpu__dec_seq_init__free_unpacked(seq_init_msg_ret, NULL);
        return -1;
    }

    memcpy(&vpu->vpu_dec_proto_info.init_output_info, seq_init_msg_ret->initial_info,
           sizeof(Vpu__DecInitialInfo));

    if (vpu->vpu_dec_proto_info.dec_open_param.bitstream_format == (int)STD_AVC) {
        if (seq_init_msg_ret->initial_info->codec_mode_info_case !=
                VPU__DEC_INITIAL_INFO__CODEC_MODE_INFO_AVC_VUI_INFO ||
            !seq_init_msg_ret->initial_info->avc_vui_info) {
            ssdk_printf(SSDK_CRIT, "[%s][%d] VPU_IPC_DEC_SEQINIT invailed avc ret msg\n", __func__,
                    __LINE__);
            vpu__dec_seq_init__free_unpacked(seq_init_msg_ret, NULL);
            return -1;
        }
    } else if (vpu->vpu_dec_proto_info.dec_open_param.bitstream_format == (int)STD_HEVC) {
        if (seq_init_msg_ret->initial_info->codec_mode_info_case !=
                VPU__DEC_INITIAL_INFO__CODEC_MODE_INFO_HEVC_INFO ||
            !seq_init_msg_ret->initial_info->hevc_info) {
            ssdk_printf(SSDK_CRIT, "[%s][%d] VPU_IPC_DEC_SEQINIT invailed hevc ret msg\n", __func__,
                    __LINE__);
            vpu__dec_seq_init__free_unpacked(seq_init_msg_ret, NULL);
            return -1;
        }
    }

    ssdk_printf(SSDK_CRIT,
            "VPU_IPC_DEC_COMPLETE_SEQINIT success, picWidth : %d,picHeight : %d, "
            "picCropRect.right : %d,"
            "picCropRect.bottom : %d, minFrameBufferCount : %d, RdPtr : 0x%x, "
            "WrPtr : 0x%x\n",
            vpu->vpu_dec_proto_info.init_output_info.pic_width,
            vpu->vpu_dec_proto_info.init_output_info.pic_height,
            seq_init_msg_ret->initial_info->pic_crop_rect->right,
            seq_init_msg_ret->initial_info->pic_crop_rect->bottom,
            vpu->vpu_dec_proto_info.init_output_info.min_frame_buffer_count,
            vpu->vpu_dec_proto_info.init_output_info.rd_ptr,
            vpu->vpu_dec_proto_info.init_output_info.wr_ptr);

    // output info
    init_info->pic_width = vpu->vpu_dec_proto_info.init_output_info.pic_width;
    init_info->pic_height = vpu->vpu_dec_proto_info.init_output_info.pic_height;
    init_info->min_frame_buffer_count =
        vpu->vpu_dec_proto_info.init_output_info.min_frame_buffer_count;
    init_info->crop_right = seq_init_msg_ret->initial_info->pic_crop_rect->right;
    init_info->crop_bottom = seq_init_msg_ret->initial_info->pic_crop_rect->bottom;
    init_info->crop_left = seq_init_msg_ret->initial_info->pic_crop_rect->left;
    init_info->crop_top = seq_init_msg_ret->initial_info->pic_crop_rect->top;

    // frame rate
    if (vpu->vpu_dec_proto_info.init_output_info.f_rate_denominator <= 0)
        init_info->frame_rate = 0;
    else {
        switch (vpu->dec_param->bitstream_format) {
        case STD_AVC:
        case STD_HEVC:
        case STD_AVS:
        case STD_MPEG2:
        case STD_MPEG4:
        case STD_H263:
        case STD_RV:
        case STD_VP8:
            init_info->frame_rate = vpu->vpu_dec_proto_info.init_output_info.f_rate_numerator /
                                    vpu->vpu_dec_proto_info.init_output_info.f_rate_denominator;
            break;
        case STD_VC1:
            if (vpu->vpu_dec_proto_info.init_output_info.profile == 2 &&
                vpu->vpu_dec_proto_info.init_output_info.f_rate_numerator < 8) {
                const int TBL_FRAME_RATE_NR[8] = {0, 24, 25, 30, 50, 60, 48, 72};
                init_info->frame_rate =
                    (TBL_FRAME_RATE_NR[vpu->vpu_dec_proto_info.init_output_info.f_rate_numerator] *
                     100) /
                    (vpu->vpu_dec_proto_info.init_output_info.f_rate_denominator + 999);
            } else {
                init_info->frame_rate =
                    vpu->vpu_dec_proto_info.init_output_info.f_rate_numerator +
                    1 / vpu->vpu_dec_proto_info.init_output_info.f_rate_denominator;
            }
            break;
        default:
            init_info->frame_rate = 0;
            break;
        }
    }

    if (init_info->frame_rate > 120)
        init_info->frame_rate = 0; // it means we bypass the timestamp of inputbuffer to outpubuffer
                                   // without timestamp corretion logic.

    // color aspect & hdr info
    switch (vpu->dec_param->bitstream_format) {
    case STD_AVC:
        init_info->color_aspect.color_desc_present =
            seq_init_msg_ret->initial_info->avc_vui_info->color_desc_present;
        init_info->color_aspect.primaries =
            seq_init_msg_ret->initial_info->avc_vui_info->color_primaries;
        init_info->color_aspect.transfer =
            seq_init_msg_ret->initial_info->avc_vui_info->vui_transfer_characteristics;
        init_info->color_aspect.coeffs =
            seq_init_msg_ret->initial_info->avc_vui_info->vui_matrix_coefficients;
        init_info->color_aspect.full_range =
            seq_init_msg_ret->initial_info->avc_vui_info->vid_full_range;
        break;
    case STD_MPEG2:
        init_info->color_aspect.color_desc_present =
            seq_init_msg_ret->initial_info->mp2_color_desc_present_flag;
        init_info->color_aspect.primaries = seq_init_msg_ret->initial_info->mp2_color_primaries;
        init_info->color_aspect.transfer = seq_init_msg_ret->initial_info->mp2_transfer_char;
        init_info->color_aspect.coeffs = seq_init_msg_ret->initial_info->mp2_matrix_coeff;
        init_info->color_aspect.full_range = 0;
        break;
    case STD_HEVC:
        // hdr info
        init_info->h265_hdr.master_meta_present =
            seq_init_msg_ret->initial_info->hevc_info->h265_info->master_meta_present;
        init_info->h265_hdr.display_primaries_x[0] =
            seq_init_msg_ret->initial_info->hevc_info->h265_info->display_primaries_x_b;
        init_info->h265_hdr.display_primaries_x[1] =
            seq_init_msg_ret->initial_info->hevc_info->h265_info->display_primaries_x_g;
        init_info->h265_hdr.display_primaries_x[2] =
            seq_init_msg_ret->initial_info->hevc_info->h265_info->display_primaries_x_r;
        init_info->h265_hdr.display_primaries_y[0] =
            seq_init_msg_ret->initial_info->hevc_info->h265_info->display_primaries_y_b;
        init_info->h265_hdr.display_primaries_y[1] =
            seq_init_msg_ret->initial_info->hevc_info->h265_info->display_primaries_y_g;
        init_info->h265_hdr.display_primaries_y[2] =
            seq_init_msg_ret->initial_info->hevc_info->h265_info->display_primaries_y_r;
        init_info->h265_hdr.white_point_x =
            seq_init_msg_ret->initial_info->hevc_info->h265_info->white_point_x;
        init_info->h265_hdr.white_point_y =
            seq_init_msg_ret->initial_info->hevc_info->h265_info->white_point_y;
        init_info->h265_hdr.max_display_mastering_luminance =
            seq_init_msg_ret->initial_info->hevc_info->h265_info->max_display_mastering_luminance;
        init_info->h265_hdr.min_display_mastering_luminance =
            seq_init_msg_ret->initial_info->hevc_info->h265_info->min_display_mastering_luminance;
        init_info->h265_hdr.content_meta_present =
            seq_init_msg_ret->initial_info->hevc_info->h265_info->content_meta_present;
        init_info->h265_hdr.max_content_light_level =
            seq_init_msg_ret->initial_info->hevc_info->h265_info->max_content_light_level;
        init_info->h265_hdr.max_frame_ave_light_level =
            seq_init_msg_ret->initial_info->hevc_info->h265_info->max_pic_average_light_level;
        break;
    default:
        break;
    }

    vpu__dec_seq_init__free_unpacked(seq_init_msg_ret, NULL);
    return 0;
}

int vpu_proto_dec_open(vpu_instance_t *vpu, vpu_dec_param_t *dec_param)
{
    if (!vpu || !dec_param) {
        ssdk_printf(SSDK_CRIT, "[%s][%d] invailed input param, vpu:%p, dec_param:%p\n", __func__,
                __LINE__, vpu, dec_param);
        return -1;
    }

    if (!vpu->dec_param)
        vpu->dec_param = (vpu_dec_param_t *)malloc(sizeof(vpu_dec_param_t));
    else {
        // already open vpu, close it
        vpu_proto_dec_close(vpu);
    }

    *vpu->dec_param = *dec_param;

    // VPU_DecOpen //set up decoder configurations
    {
        Vpu__DecOpen *dec_open_msg_ret = NULL;
        Vpu__DecOpen dec_open_msg = VPU__DEC_OPEN__INIT;
        Vpu__DecHandle dec_handle_msg = VPU__DEC_HANDLE__INIT;

        Vpu__DecOpenParam *dec_op_msg = &vpu->vpu_dec_proto_info.dec_open_param;
        *dec_op_msg = (Vpu__DecOpenParam)VPU__DEC_OPEN_PARAM__INIT;

        dec_open_msg.handle = &dec_handle_msg;
        dec_open_msg.dec_op = dec_op_msg;

        dec_op_msg->core_idx = vpu->core_idx;
        dec_op_msg->wtl_mode = FF_FRAME;
        dec_op_msg->bitstream_buffer = vpu->vb_stream.phys_addr;
        dec_op_msg->bitstream_buffer_size = vpu->vb_stream.size;
        dec_op_msg->bitstream_mode = BS_MODE_PIC_END;
        dec_op_msg->bwb_enable = VPU_ENABLE_BWB;
        dec_op_msg->nv21 = 0;
        dec_op_msg->avc_extension = 0;
        dec_op_msg->tiled2_linear_enable = 0;
        dec_op_msg->mp4_deblk_enable = 0;

        dec_op_msg->cbcr_interleave = vpu->dec_param->cbcr_interleave;
        dec_op_msg->cbcr_order = vpu->dec_param->cbcr_order;
        dec_op_msg->bitstream_format = vpu->dec_param->bitstream_format;
        dec_op_msg->mp4_class = vpu->dec_param->mp4_class;

        if (dec_op_msg->core_idx == VPU_CODA988_IDX) {
            dec_op_msg->wtl_enable = FALSE;
            dec_op_msg->stream_endian = VDI_LITTLE_ENDIAN;
            dec_op_msg->frame_endian = VDI_LITTLE_ENDIAN;
            dec_op_msg->tiled2_linear_mode = FF_FRAME;
            dec_op_msg->sram_enable = vpu->dec_param->sram_enable;
        } else if (dec_op_msg->core_idx == VPU_WAVE412_IDX) {
            dec_op_msg->wtl_enable = TRUE;
            dec_op_msg->stream_endian = VDI_128BIT_LITTLE_ENDIAN;
            dec_op_msg->frame_endian = VDI_128BIT_LITTLE_ENDIAN;
            dec_op_msg->tiled2_linear_mode = FF_NONE;
            // normal: 0x0C - Best prediction: 0x00 - Basic prediction: 0x3C
            dec_op_msg->fbc_mode = 0x0C;
            dec_op_msg->bw_optimization = TRUE;

            if (vpu->dec_param->map_type == PVRIC_COMPRESSED_FRAME_MAP)
                dec_op_msg->pvric_fbc_enable = TRUE;
        }

        dec_op_msg->has_bitstream_format = TRUE;
        dec_op_msg->has_avc_extension = TRUE;
        dec_op_msg->has_core_idx = TRUE;
        dec_op_msg->has_bitstream_buffer = TRUE;
        dec_op_msg->has_bitstream_buffer_size = TRUE;
        dec_op_msg->has_bitstream_mode = TRUE;
        dec_op_msg->has_tiled2_linear_enable = TRUE;
        dec_op_msg->has_tiled2_linear_mode = TRUE;
        dec_op_msg->has_wtl_enable = TRUE;
        dec_op_msg->has_wtl_mode = TRUE;
        dec_op_msg->has_cbcr_interleave = TRUE;
        dec_op_msg->has_nv21 = TRUE;
        dec_op_msg->has_stream_endian = TRUE;
        dec_op_msg->has_frame_endian = TRUE;
        dec_op_msg->has_bwb_enable = TRUE;
        dec_op_msg->has_mp4_deblk_enable = TRUE;
        dec_op_msg->has_mp4_class = TRUE;
        dec_op_msg->has_fbc_mode = TRUE;
        dec_op_msg->has_bw_optimization = TRUE;
        dec_op_msg->has_pvric_fbc_enable = TRUE;
        dec_op_msg->has_cbcr_order = TRUE;
        dec_op_msg->has_sram_enable = TRUE;

        ssdk_printf(SSDK_INFO,
                "------------------------------ DECODER OPTIONS ------------------------------\n");
        ssdk_printf(SSDK_INFO, "[coreIdx            ]: %d\n", dec_op_msg->core_idx);
        ssdk_printf(SSDK_INFO, "[wtlEnable          ]: %d\n", dec_op_msg->wtl_enable);
        ssdk_printf(SSDK_INFO, "[wtlMode            ]: %d\n", dec_op_msg->wtl_mode);
        ssdk_printf(SSDK_INFO, "[bitstreamBuffer    ]: 0x%08x\n",
                dec_op_msg->bitstream_buffer);
        ssdk_printf(SSDK_INFO, "[bitstreamBufferSize]: %d\n",
                dec_op_msg->bitstream_buffer_size);
        ssdk_printf(SSDK_INFO, "[bitstreamMode      ]: %d\n", dec_op_msg->bitstream_mode);
        ssdk_printf(SSDK_INFO, "[cbcrInterleave     ]: %d\n", dec_op_msg->cbcr_interleave);
        ssdk_printf(SSDK_INFO, "[cbcrOrder          ]: %d\n", dec_op_msg->cbcr_order);
        ssdk_printf(SSDK_INFO, "[nv21               ]: %d\n", dec_op_msg->nv21);
        ssdk_printf(SSDK_INFO, "[streamEndian       ]: %d\n", dec_op_msg->stream_endian);
        ssdk_printf(SSDK_INFO, "[frameEndian        ]: %d\n", dec_op_msg->frame_endian);
        ssdk_printf(SSDK_INFO, "[BWB                ]: %d\n", dec_op_msg->bwb_enable);
        ssdk_printf(SSDK_INFO, "[bitstreamFormat    ]: %d\n", dec_op_msg->bitstream_format);
        ssdk_printf(SSDK_INFO, "[avcExtension       ]: %d\n", dec_op_msg->avc_extension);
        ssdk_printf(SSDK_INFO, "[tiled2LinearEnable ]: %d\n",
                dec_op_msg->tiled2_linear_enable);
        ssdk_printf(SSDK_INFO, "[tiled2LinearMode   ]: %d\n",
                dec_op_msg->tiled2_linear_mode);
        ssdk_printf(SSDK_INFO, "[mp4DeblkEnable     ]: %d\n", dec_op_msg->mp4_deblk_enable);
        ssdk_printf(SSDK_INFO, "[mp4Class           ]: %d\n", dec_op_msg->mp4_class);
        ssdk_printf(SSDK_INFO, "[pvricFbcEnable     ]: %d\n", dec_op_msg->pvric_fbc_enable);
        ssdk_printf(SSDK_INFO, "[fbc_mode           ]: %d\n", dec_op_msg->fbc_mode);
        ssdk_printf(SSDK_INFO, "[bwOptimization     ]: %d\n", dec_op_msg->bw_optimization);
        ssdk_printf(SSDK_INFO,
                "--------------------------------------------------------------------------\n");

        // proto VPU_IPC_DEC_OPEN msg
        SEND_PROTO_AND_CHECK_RET_SUCCESS(vpu, vpu__dec_open, dec_open_msg, dec_open_msg_ret,
                                         VPU_IPC_DEC_OPEN, return -1);

        vpu->vpu_handle = dec_open_msg_ret->handle->handle;
        vpu__dec_open__free_unpacked(dec_open_msg_ret, NULL);

        ssdk_printf(SSDK_CRIT, "VPU_DecOpen success, vpu handle:0x%x\n", vpu->vpu_handle);
    }

    return 0;
}

int vpu_proto_dec_one_frame(vpu_instance_t *vpu, int is_thumbnail_mode,
                            vpu_dec_frame_output_info_t *output_info)
{
    if (!vpu) {
        ssdk_printf(SSDK_CRIT, "[%s][%d] invailed input param, vpu:%p\n", __FUNCTION__, __LINE__, vpu);
        return -1;
    }

    if (!vpu->vpu_handle || !vpu->dec_param || !output_info) {
        ssdk_printf(SSDK_CRIT,
                    "[%s][%d] invailed input param, vpu:%p, vpu_handle:%x, dec_param:%p, output_info:%p\n",
                    __FUNCTION__, __LINE__, vpu, vpu->vpu_handle, vpu->dec_param, output_info);
        return -1;
    }

    // Vpu__DecOneFrame
    Vpu__DecOneFrame *dec_one_frame_ret = NULL;
    Vpu__DecOneFrame dec_one_frame = VPU__DEC_ONE_FRAME__INIT;
    Vpu__DecHandle dec_handle_msg = VPU__DEC_HANDLE__INIT;
    Vpu__DecParam dec_param_msg = (Vpu__DecParam)VPU__DEC_PARAM__INIT;

    dec_one_frame.handle = &dec_handle_msg;
    dec_handle_msg.handle = vpu->vpu_handle;

    dec_one_frame.dec_param = &dec_param_msg;

    osMutexAcquire (vpu->dec_disp_flag.disp_flag_mutex, osWaitForever);

    dec_param_msg.clr_disp_flag = vpu->dec_disp_flag.clr_disp_flag;
    dec_param_msg.set_disp_flag = vpu->dec_disp_flag.set_disp_flag;
    vpu->dec_disp_flag.clr_disp_flag = 0;
    vpu->dec_disp_flag.set_disp_flag = 0;

    osMutexRelease (vpu->dec_disp_flag.disp_flag_mutex);

    dec_param_msg.has_clr_disp_flag = TRUE;
    dec_param_msg.has_set_disp_flag = TRUE;

    if (is_thumbnail_mode) {
        dec_param_msg.skipframe_mode = TRUE;
        dec_param_msg.has_skipframe_mode = TRUE;
        dec_param_msg.thumbnail_mode = TRUE;
        dec_param_msg.has_thumbnail_mode = TRUE;
    }

    // proto VPU_IPC_DEC_ONE_FRAMER msg
    SEND_PROTO_AND_CHECK_RET_SUCCESS(vpu, vpu__dec_one_frame, dec_one_frame, dec_one_frame_ret,
                                     VPU_IPC_DEC_ONE_FRAMER, return -1);

    if (!dec_one_frame_ret->has_interrupt_reaseon) {
        ssdk_printf(SSDK_CRIT, "[%s][%d] invailed ret msg \n", __func__, __LINE__);
        vpu__dec_one_frame__free_unpacked(dec_one_frame_ret, NULL);
        return -1;
    }

    if (!(dec_one_frame_ret->interrupt_reaseon & (1 << INT_BIT_PIC_RUN))) {
        ssdk_printf(SSDK_CRIT, "[%s][%d] invailed interrupt reaseon\n", __func__, __LINE__);
        vpu__dec_one_frame__free_unpacked(dec_one_frame_ret, NULL);
        return -1;
    }

    if (!dec_one_frame_ret->dec_output_info || !dec_one_frame_ret->dec_output_info->rc_display ||
        !dec_one_frame_ret->dec_output_info->disp_frame) {
        ssdk_printf(SSDK_CRIT, "[%s][%d] invailed ret msg \n", __func__, __LINE__);
        vpu__dec_one_frame__free_unpacked(dec_one_frame_ret, NULL);
        return -1;
    }

    if (vpu->vpu_dec_proto_info.dec_open_param.bitstream_format == (int)STD_AVC) {
        if (dec_one_frame_ret->dec_output_info->dec_codec_info_case !=
                VPU__DEC_OUTPUT_INFO__DEC_CODEC_INFO_AVC_INFO ||
            !dec_one_frame_ret->dec_output_info->avc_info ||
            !dec_one_frame_ret->dec_output_info->avc_info->avc_vui_info) {
            ssdk_printf(SSDK_CRIT, "[%s][%d] dec get output info invailed avc ret msg\n", __func__,
                    __LINE__);
            vpu__dec_one_frame__free_unpacked(dec_one_frame_ret, NULL);
            return -1;
        }
    } else if (vpu->vpu_dec_proto_info.dec_open_param.bitstream_format == (int)STD_HEVC) {
        if (dec_one_frame_ret->dec_output_info->dec_codec_info_case !=
                VPU__DEC_OUTPUT_INFO__DEC_CODEC_INFO_HEVC_INFO ||
            !dec_one_frame_ret->dec_output_info->hevc_info ||
            !dec_one_frame_ret->dec_output_info->hevc_info->h265_info) {
            ssdk_printf(SSDK_CRIT, "[%s][%d] dec get output info invailed hevc ret msg\n", __func__,
                    __LINE__);
            vpu__dec_one_frame__free_unpacked(dec_one_frame_ret, NULL);
            return -1;
        }
    }

    if (!dec_one_frame_ret->dec_output_info->decoding_success) {
        ssdk_printf(SSDK_INFO,
                "[%s][%d] decoding_success failed, error:0x%x, error_ext:0x%x\n", __func__,
                __LINE__, dec_one_frame_ret->dec_output_info->error_reason,
                dec_one_frame_ret->dec_output_info->error_reason_ext);
    }

    output_info->pic_type = dec_one_frame_ret->dec_output_info->pic_type;
    output_info->index_frame_display = dec_one_frame_ret->dec_output_info->index_frame_display;
    output_info->index_frame_decoded = dec_one_frame_ret->dec_output_info->index_frame_decoded;
    output_info->index_frame_prescan = dec_one_frame_ret->dec_output_info->index_frame_prescan;
    output_info->chunk_reuse_required = dec_one_frame_ret->dec_output_info->chunk_reuse_required;
    output_info->frame_display_flag = dec_one_frame_ret->dec_output_info->frame_display_flag;
    output_info->disp_pic_width = dec_one_frame_ret->dec_output_info->disp_pic_width;
    output_info->disp_pic_height = dec_one_frame_ret->dec_output_info->disp_pic_height;
    output_info->sequence_changed = dec_one_frame_ret->dec_output_info->sequence_changed;

    output_info->disp_frame_index = dec_one_frame_ret->dec_output_info->disp_frame->my_index;

    output_info->crop_left = dec_one_frame_ret->dec_output_info->rc_display->left;
    output_info->crop_top = dec_one_frame_ret->dec_output_info->rc_display->top;
    output_info->crop_right = dec_one_frame_ret->dec_output_info->rc_display->right;
    output_info->crop_bottom = dec_one_frame_ret->dec_output_info->rc_display->bottom;

    output_info->disp_frame_addr =
        (void *)ap2p((paddr_t)dec_one_frame_ret->dec_output_info->disp_frame->buf_y);

    if (vpu->core_idx == VPU_CODA988_IDX) {
        output_info->disp_frame_size = vpu->vpu_dec_proto_info.dec_fb_alloc_info_dec.size;
    } else if (vpu->core_idx == VPU_WAVE412_IDX) {
        output_info->disp_frame_size = vpu->vpu_dec_proto_info.dec_fb_alloc_info_disp.size;
    }

    // color aspect & hdr info & vp9 seq change
    switch (vpu->dec_param->bitstream_format) {
    case STD_AVC:
        output_info->color_aspect.color_desc_present =
            dec_one_frame_ret->dec_output_info->avc_info->avc_vui_info->color_desc_present;
        output_info->color_aspect.primaries =
            dec_one_frame_ret->dec_output_info->avc_info->avc_vui_info->color_primaries;
        output_info->color_aspect.transfer = dec_one_frame_ret->dec_output_info->avc_info
                                                 ->avc_vui_info->vui_transfer_characteristics;
        output_info->color_aspect.coeffs =
            dec_one_frame_ret->dec_output_info->avc_info->avc_vui_info->vui_matrix_coefficients;
        output_info->color_aspect.full_range =
            dec_one_frame_ret->dec_output_info->avc_info->avc_vui_info->vid_full_range;
        break;
    case STD_MPEG2:
        output_info->color_aspect.color_desc_present =
            dec_one_frame_ret->dec_output_info->mp2_color_desc_present_flag;
        output_info->color_aspect.primaries =
            dec_one_frame_ret->dec_output_info->mp2_color_primaries;
        output_info->color_aspect.transfer = dec_one_frame_ret->dec_output_info->mp2_transfer_char;
        output_info->color_aspect.coeffs = dec_one_frame_ret->dec_output_info->mp2_matrix_coeff;
        output_info->color_aspect.full_range = 0;
        break;
    case STD_HEVC:
        output_info->color_aspect.color_desc_present =
            dec_one_frame_ret->dec_output_info->hevc_info->h265_info->color_desc_present;
        output_info->color_aspect.primaries =
            dec_one_frame_ret->dec_output_info->hevc_info->h265_info->color_primaries;
        output_info->color_aspect.transfer =
            dec_one_frame_ret->dec_output_info->hevc_info->h265_info->vui_transfer_characteristics;
        output_info->color_aspect.coeffs =
            dec_one_frame_ret->dec_output_info->hevc_info->h265_info->vui_matrix_coefficients;
        output_info->color_aspect.full_range =
            dec_one_frame_ret->dec_output_info->hevc_info->h265_info->vid_full_range;

        // hdr info
        output_info->h265_hdr.master_meta_present =
            dec_one_frame_ret->dec_output_info->hevc_info->h265_info->master_meta_present;
        output_info->h265_hdr.display_primaries_x[0] =
            dec_one_frame_ret->dec_output_info->hevc_info->h265_info->display_primaries_x_b;
        output_info->h265_hdr.display_primaries_x[1] =
            dec_one_frame_ret->dec_output_info->hevc_info->h265_info->display_primaries_x_g;
        output_info->h265_hdr.display_primaries_x[2] =
            dec_one_frame_ret->dec_output_info->hevc_info->h265_info->display_primaries_x_r;
        output_info->h265_hdr.display_primaries_y[0] =
            dec_one_frame_ret->dec_output_info->hevc_info->h265_info->display_primaries_y_b;
        output_info->h265_hdr.display_primaries_y[1] =
            dec_one_frame_ret->dec_output_info->hevc_info->h265_info->display_primaries_y_g;
        output_info->h265_hdr.display_primaries_y[2] =
            dec_one_frame_ret->dec_output_info->hevc_info->h265_info->display_primaries_y_r;
        output_info->h265_hdr.white_point_x =
            dec_one_frame_ret->dec_output_info->hevc_info->h265_info->white_point_x;
        output_info->h265_hdr.white_point_y =
            dec_one_frame_ret->dec_output_info->hevc_info->h265_info->white_point_y;
        output_info->h265_hdr.max_display_mastering_luminance =
            dec_one_frame_ret->dec_output_info->hevc_info->h265_info
                ->max_display_mastering_luminance;
        output_info->h265_hdr.min_display_mastering_luminance =
            dec_one_frame_ret->dec_output_info->hevc_info->h265_info
                ->min_display_mastering_luminance;
        output_info->h265_hdr.content_meta_present =
            dec_one_frame_ret->dec_output_info->hevc_info->h265_info->content_meta_present;
        output_info->h265_hdr.max_content_light_level =
            dec_one_frame_ret->dec_output_info->hevc_info->h265_info->max_content_light_level;
        output_info->h265_hdr.max_frame_ave_light_level =
            dec_one_frame_ret->dec_output_info->hevc_info->h265_info->max_pic_average_light_level;

        break;
    case STD_VP9:
        if (dec_one_frame_ret->dec_output_info->has_vp9_inter_res_change)
            output_info->vp9_res_change = dec_one_frame_ret->dec_output_info->vp9_inter_res_change;
        if (dec_one_frame_ret->dec_output_info->has_vp9_inter_res_change_frame_idx)
            output_info->vp9_res_change_frame_idx =
                dec_one_frame_ret->dec_output_info->vp9_inter_res_change_frame_idx;
        break;
    default:
        break;
    }

    vpu__dec_one_frame__free_unpacked(dec_one_frame_ret, NULL);
    return 0;
}

int vpu_proto_dec_fill_bit_stream_buffer(vpu_instance_t *vpu, StreamUpFlag flag,
                                         unsigned char *data, int data_size)
{
    if (!vpu) {
        ssdk_printf(SSDK_CRIT, "[%s][%d] invailed input param, vpu:%p\n", __FUNCTION__, __LINE__, vpu);
        return -1;
    }

    if (!vpu->vpu_handle || !vpu->dec_param) {
        ssdk_printf(SSDK_CRIT, "[%s][%d] invailed input param, vpu:%p, vpu_handle:%x, dec_param:%p\n",
                __FUNCTION__, __LINE__, vpu, vpu->vpu_handle, vpu->dec_param);
        return -1;
    }

    if (data_size < 0 || data_size > vpu->vb_stream.size) {
        ssdk_printf(SSDK_CRIT, "[%s][%d] invailed data size:%d\n", __func__, __LINE__, data_size);
        return -1;
    }

    if (!vpu->rd_ptr) {
        if (vpu->core_idx == VPU_CODA988_IDX) {
            vpu->rd_ptr = vpu->vb_stream.phys_addr;
        } else if (vpu->core_idx == VPU_WAVE412_IDX) {
            if (vpu->wave_stream_count % 2 == 0)
                vpu->rd_ptr = vpu->vb_stream.phys_addr;
            else
                vpu->rd_ptr = vpu->vb_stream_wave_more.phys_addr;
        } else {
            ssdk_printf(SSDK_CRIT, "[%s][%d] core_idx:%d\n", __func__, __LINE__, vpu->core_idx);
            return -1;
        }

        vpu->wr_ptr = vpu->rd_ptr;
    }

    ssdk_printf(SSDK_INFO, "[%s][%d], wr_ptr(%x), feed_size(%d) flag:%d\n", __func__,
            __LINE__, vpu->wr_ptr, data_size, flag);

    switch (flag) {
    case STREAM_FILL:
        _vpu_proto_memcpy_form_cpu_to_device((void *)ap2p((paddr_t)vpu->wr_ptr), data, data_size);
        vpu->wr_ptr += data_size;
        break;
    case STREAM_UPDATE:
        _vpu_proto_memcpy_form_cpu_to_device((void *)ap2p((paddr_t)vpu->wr_ptr), data, data_size);
        vpu->wr_ptr += data_size;
        break;
    case STREAM_END:
        vpu->rd_ptr = vpu->vb_stream.phys_addr;
        vpu->wr_ptr = vpu->rd_ptr;
        break;
    case STREAM_END_CLEAR:
        vpu->rd_ptr = vpu->vb_stream.phys_addr;
        vpu->wr_ptr = vpu->rd_ptr;
        break;
    default:
        break;
    }

    // Vpu__DecPicEndModeUpdateBSBuffer
    if (flag != STREAM_FILL) {
        Vpu__DecPicEndModeUpdateBSBuffer *pic_end_mode_update_bs_buf_msg_ret = NULL;
        Vpu__DecPicEndModeUpdateBSBuffer pic_end_mode_update_bs_buf_msg =
            VPU__DEC_PIC_END_MODE_UPDATE_BSBUFFER__INIT;
        Vpu__DecHandle dec_handle_msg = VPU__DEC_HANDLE__INIT;

        pic_end_mode_update_bs_buf_msg.handle = &dec_handle_msg;
        dec_handle_msg.handle = vpu->vpu_handle;

        pic_end_mode_update_bs_buf_msg.has_addr = TRUE;
        pic_end_mode_update_bs_buf_msg.addr = vpu->rd_ptr;

        pic_end_mode_update_bs_buf_msg.has_size = TRUE;
        pic_end_mode_update_bs_buf_msg.size = vpu->wr_ptr - vpu->rd_ptr;

        pic_end_mode_update_bs_buf_msg.has_flag = TRUE;
        pic_end_mode_update_bs_buf_msg.flag = flag;

        // reset
        vpu->rd_ptr = 0;
        vpu->wr_ptr = vpu->rd_ptr;
        if (flag == STREAM_UPDATE && (pic_end_mode_update_bs_buf_msg.size > 0))
            vpu->wave_stream_count++;

        // proto VPU_IPC_DEC_PIC_END_MODE_UPDATE_BS_BUFFER msg
        SEND_PROTO_AND_CHECK_RET_SUCCESS(vpu, vpu__dec_pic_end_mode_update_bsbuffer,
                                         pic_end_mode_update_bs_buf_msg,
                                         pic_end_mode_update_bs_buf_msg_ret,
                                         VPU_IPC_DEC_PIC_END_MODE_UPDATE_BS_BUFFER, return -1);
        vpu__dec_pic_end_mode_update_bsbuffer__free_unpacked(pic_end_mode_update_bs_buf_msg_ret,
                                                             NULL);
    }

    return data_size;
}

int vpu_proto_dec_register_frame_buffer(vpu_instance_t *vpu, vpu_buffer_t *vb, int buf_count,
                                        int is_vp9_res_change, int vp9_res_change_idx,
                                        vpu_dec_scaler_info_t *scaler_info)
{
    if (!vpu) {
        ssdk_printf(SSDK_CRIT, "[%s][%d] invailed input param, vpu:%p\n", __FUNCTION__, __LINE__, vpu);
        return -1;
    }

    if (!vpu->vpu_handle || !vpu->dec_param) {
        ssdk_printf(SSDK_CRIT, "[%s][%d] invailed input param, vpu:%p, vpu_handle:%x, dec_param:%p\n",
                __FUNCTION__, __LINE__, vpu, vpu->vpu_handle, vpu->dec_param);
        return -1;
    }

    // buf_count equals 0 means automatic calculation
    if (buf_count && buf_count < vpu->vpu_dec_proto_info.init_output_info.min_frame_buffer_count) {
        ssdk_printf(SSDK_CRIT, "[%s][%d] invailed buf_count:%d vb:%p\n", __func__, __LINE__, buf_count,
                vb);
        return -1;
    }

    // VPU_IPC_DEC_REGISTER_DECODE_BUFFER
    Vpu__FrameBufferAllocInfo *dec_fb_alloc_info_msg = NULL;
    Vpu__FrameBufferAllocInfo *disp_fb_alloc_info_msg = NULL;
    Vpu__DecRegisterDecodeBuffer *register_dec_buf_msg_ret = NULL;
    Vpu__DecRegisterDecodeBuffer register_dec_buf_msg = VPU__DEC_REGISTER_DECODE_BUFFER__INIT;
    Vpu__DecHandle dec_handle_msg = VPU__DEC_HANDLE__INIT;

    dec_handle_msg.handle = vpu->vpu_handle;
    register_dec_buf_msg.handle = &dec_handle_msg;

    // vb frame buffer for dec // for coda, dec & display use same buffer
    dec_fb_alloc_info_msg = &vpu->vpu_dec_proto_info.dec_fb_alloc_info_dec;
    *dec_fb_alloc_info_msg = (Vpu__FrameBufferAllocInfo)VPU__FRAME_BUFFER_ALLOC_INFO__INIT;
    register_dec_buf_msg.fb_alloc_info_dec = dec_fb_alloc_info_msg;

    if (vpu->core_idx == VPU_WAVE412_IDX &&
        (vpu->vpu_dec_proto_info.init_output_info.luma_bitdepth > 8 ||
         vpu->vpu_dec_proto_info.init_output_info.chroma_bitdepth > 8))
        dec_fb_alloc_info_msg->format = FORMAT_420_P10_16BIT_LSB;
    else
        dec_fb_alloc_info_msg->format = FORMAT_420; // defalut value

    if (vpu->core_idx == VPU_CODA988_IDX)
        dec_fb_alloc_info_msg->map_type = vpu->dec_param->map_type;
    else if (vpu->core_idx == VPU_WAVE412_IDX)
        dec_fb_alloc_info_msg->map_type = COMPRESSED_FRAME_MAP; // for wave internal refer

    dec_fb_alloc_info_msg->cbcr_interleave = vpu->dec_param->cbcr_interleave;

    if (vpu->vpu_dec_proto_info.dec_open_param.bitstream_format == (int)STD_VP9) {
        dec_fb_alloc_info_msg->height =
            VPU_ALIGN64(vpu->vpu_dec_proto_info.init_output_info.pic_height);
        dec_fb_alloc_info_msg->stride = vpu_proto_calc_stride(
            VPU_ALIGN64(vpu->vpu_dec_proto_info.init_output_info.pic_width),
            vpu->vpu_dec_proto_info.init_output_info.pic_height, dec_fb_alloc_info_msg->format,
            dec_fb_alloc_info_msg->cbcr_interleave, dec_fb_alloc_info_msg->map_type, TRUE);
    } else {
        dec_fb_alloc_info_msg->height =
            VPU_ALIGN32(vpu->vpu_dec_proto_info.init_output_info.pic_height);
        dec_fb_alloc_info_msg->stride = vpu_proto_calc_stride(
            vpu->vpu_dec_proto_info.init_output_info.pic_width,
            vpu->vpu_dec_proto_info.init_output_info.pic_height, dec_fb_alloc_info_msg->format,
            dec_fb_alloc_info_msg->cbcr_interleave, dec_fb_alloc_info_msg->map_type, FALSE);
    }

    dec_fb_alloc_info_msg->size = vpu_proto_calc_frame_size(
        vpu->product_id, dec_fb_alloc_info_msg->stride, dec_fb_alloc_info_msg->height,
        dec_fb_alloc_info_msg->map_type, dec_fb_alloc_info_msg->format,
        dec_fb_alloc_info_msg->cbcr_interleave);
    dec_fb_alloc_info_msg->luma_bit_depth = vpu->vpu_dec_proto_info.init_output_info.luma_bitdepth;
    dec_fb_alloc_info_msg->chroma_bit_depth =
        vpu->vpu_dec_proto_info.init_output_info.chroma_bitdepth;
    if (buf_count) {
        dec_fb_alloc_info_msg->num = buf_count;
    } else {
        dec_fb_alloc_info_msg->num =
            vpu->vpu_dec_proto_info.init_output_info.min_frame_buffer_count +
            VPU_DEFAULT_EXTRA_FRAME_BUFFER_NUM;
    }
    dec_fb_alloc_info_msg->endian = vpu->vpu_dec_proto_info.dec_open_param.frame_endian;
    dec_fb_alloc_info_msg->type = FB_TYPE_CODEC;

    dec_fb_alloc_info_msg->has_format = TRUE;
    dec_fb_alloc_info_msg->has_cbcr_interleave = TRUE;
    dec_fb_alloc_info_msg->has_map_type = TRUE;
    dec_fb_alloc_info_msg->has_height = TRUE;
    dec_fb_alloc_info_msg->has_stride = TRUE;
    dec_fb_alloc_info_msg->has_size = TRUE;
    dec_fb_alloc_info_msg->has_luma_bit_depth = TRUE;
    dec_fb_alloc_info_msg->has_chroma_bit_depth = TRUE;
    dec_fb_alloc_info_msg->has_num = TRUE;
    dec_fb_alloc_info_msg->has_endian = TRUE;
    dec_fb_alloc_info_msg->has_type = TRUE;

    ssdk_printf(SSDK_INFO, "dec buf count %d, stride %d, height %d, maptype %d\n",
            dec_fb_alloc_info_msg->num, dec_fb_alloc_info_msg->stride,
            dec_fb_alloc_info_msg->height, dec_fb_alloc_info_msg->map_type);

    uint32_t vb_frame_dec_buf_ptr[dec_fb_alloc_info_msg->num];
    if (vb && vpu->core_idx == VPU_CODA988_IDX) {
        for (int i = 0; i < dec_fb_alloc_info_msg->num; i++) {
            vb_frame_dec_buf_ptr[i] = vb[i].phys_addr;
            ssdk_printf(SSDK_INFO, "[%s][%d] dec_addr[%d]:%x\n", __func__, __LINE__, i,
                    vb_frame_dec_buf_ptr[i]);
        }
        register_dec_buf_msg.vb_frame_dec_size = vb[0].size;
    } else {
        for (int i = 0; i < dec_fb_alloc_info_msg->num; i++) {
            if (is_vp9_res_change) {
                if (i == vp9_res_change_idx) {
                    if (vpu->dec_vb_frame_dec[i].size > 0)
                        _vpu_proto_free_dma_memory(&vpu->dec_vb_frame_dec[i]);

                    vpu->dec_vb_frame_dec[i].size = dec_fb_alloc_info_msg->size;
                    if (_vpu_proto_allocate_dma_memory(&vpu->dec_vb_frame_dec[i]) < 0) {
                        ssdk_printf(SSDK_CRIT, "%s:%d fail to allocate frame buffer\n", __func__,
                                __LINE__);
                        return -1;
                    }
                }

                register_dec_buf_msg.has_vp9_res_change = TRUE;
                register_dec_buf_msg.vp9_res_change = TRUE;
            } else {
                if (vpu->dec_vb_frame_dec[i].size > 0)
                    _vpu_proto_free_dma_memory(&vpu->dec_vb_frame_dec[i]);

                vpu->dec_vb_frame_dec[i].size = dec_fb_alloc_info_msg->size;
                if (_vpu_proto_allocate_dma_memory(&vpu->dec_vb_frame_dec[i]) < 0) {
                    ssdk_printf(SSDK_CRIT, "%s:%d fail to allocate frame buffer\n", __func__, __LINE__);
                    return -1;
                }
            }

            vb_frame_dec_buf_ptr[i] = vpu->dec_vb_frame_dec[i].phys_addr;
            ssdk_printf(SSDK_INFO, "[%s][%d] dec_addr[%d]:%x\n", __func__, __LINE__, i,
                    vb_frame_dec_buf_ptr[i]);
        }
        register_dec_buf_msg.vb_frame_dec_size = dec_fb_alloc_info_msg->size;
    }

    register_dec_buf_msg.has_vb_frame_dec_size = TRUE;
    register_dec_buf_msg.n_vb_frame_dec_phys_addr = dec_fb_alloc_info_msg->num;
    register_dec_buf_msg.vb_frame_dec_phys_addr = vb_frame_dec_buf_ptr;

    // independent display vb frame buffer for wave
    uint32_t vb_frame_wtl_buf_ptr[dec_fb_alloc_info_msg->num];
    if (vpu->core_idx == VPU_WAVE412_IDX) {
        disp_fb_alloc_info_msg = &vpu->vpu_dec_proto_info.dec_fb_alloc_info_disp;
        *disp_fb_alloc_info_msg = (Vpu__FrameBufferAllocInfo)VPU__FRAME_BUFFER_ALLOC_INFO__INIT;
        register_dec_buf_msg.fb_alloc_info_wtl = disp_fb_alloc_info_msg;

        disp_fb_alloc_info_msg->format = FORMAT_420;
        disp_fb_alloc_info_msg->cbcr_interleave = vpu->dec_param->cbcr_interleave;
        disp_fb_alloc_info_msg->map_type = vpu->dec_param->map_type;

        if (scaler_info && scaler_info->enable_scaler) {
            register_dec_buf_msg.enable_scaler = TRUE;
            register_dec_buf_msg.scale_width = scaler_info->scale_width;
            register_dec_buf_msg.scale_height = scaler_info->scale_height;
            register_dec_buf_msg.has_enable_scaler = TRUE;
            register_dec_buf_msg.has_scale_width = TRUE;
            register_dec_buf_msg.has_scale_height = TRUE;
        }

        if (vpu->vpu_dec_proto_info.dec_open_param.bitstream_format == (int)STD_VP9) {
            if (scaler_info && scaler_info->enable_scaler) {
                disp_fb_alloc_info_msg->height = VPU_ALIGN64(scaler_info->scale_height);
                disp_fb_alloc_info_msg->stride = vpu_proto_calc_stride(
                    VPU_ALIGN64(scaler_info->scale_width), scaler_info->scale_height,
                    disp_fb_alloc_info_msg->format, disp_fb_alloc_info_msg->cbcr_interleave,
                    disp_fb_alloc_info_msg->map_type, TRUE);
            } else {
                disp_fb_alloc_info_msg->height =
                    VPU_ALIGN64(vpu->vpu_dec_proto_info.init_output_info.pic_height);
                disp_fb_alloc_info_msg->stride = vpu_proto_calc_stride(
                    VPU_ALIGN64(vpu->vpu_dec_proto_info.init_output_info.pic_width),
                    vpu->vpu_dec_proto_info.init_output_info.pic_height,
                    disp_fb_alloc_info_msg->format, disp_fb_alloc_info_msg->cbcr_interleave,
                    disp_fb_alloc_info_msg->map_type, TRUE);
            }
        } else {
            if (scaler_info && scaler_info->enable_scaler) {
                disp_fb_alloc_info_msg->height = VPU_ALIGN32(scaler_info->scale_height);
                disp_fb_alloc_info_msg->stride = vpu_proto_calc_stride(
                    scaler_info->scale_width, scaler_info->scale_height,
                    disp_fb_alloc_info_msg->format, disp_fb_alloc_info_msg->cbcr_interleave,
                    disp_fb_alloc_info_msg->map_type, FALSE);
            } else {
                disp_fb_alloc_info_msg->height =
                    VPU_ALIGN32(vpu->vpu_dec_proto_info.init_output_info.pic_height);
                disp_fb_alloc_info_msg->stride = vpu_proto_calc_stride(
                    vpu->vpu_dec_proto_info.init_output_info.pic_width,
                    vpu->vpu_dec_proto_info.init_output_info.pic_height,
                    disp_fb_alloc_info_msg->format, disp_fb_alloc_info_msg->cbcr_interleave,
                    disp_fb_alloc_info_msg->map_type, FALSE);
            }
        }

        disp_fb_alloc_info_msg->size = vpu_proto_calc_frame_size(
            vpu->product_id, disp_fb_alloc_info_msg->stride, disp_fb_alloc_info_msg->height,
            disp_fb_alloc_info_msg->map_type, disp_fb_alloc_info_msg->format,
            disp_fb_alloc_info_msg->cbcr_interleave);

        disp_fb_alloc_info_msg->luma_bit_depth =
            vpu->vpu_dec_proto_info.init_output_info.luma_bitdepth;
        disp_fb_alloc_info_msg->chroma_bit_depth =
            vpu->vpu_dec_proto_info.init_output_info.chroma_bitdepth;
        if (buf_count) {
            disp_fb_alloc_info_msg->num = buf_count;
        } else {
            disp_fb_alloc_info_msg->num =
                vpu->vpu_dec_proto_info.init_output_info.min_frame_buffer_count +
                VPU_DEFAULT_EXTRA_FRAME_BUFFER_NUM;
        }
        disp_fb_alloc_info_msg->endian = vpu->vpu_dec_proto_info.dec_open_param.frame_endian;
        disp_fb_alloc_info_msg->type = FB_TYPE_CODEC;

        disp_fb_alloc_info_msg->has_format = TRUE;
        disp_fb_alloc_info_msg->has_cbcr_interleave = TRUE;
        disp_fb_alloc_info_msg->has_map_type = TRUE;
        disp_fb_alloc_info_msg->has_height = TRUE;
        disp_fb_alloc_info_msg->has_stride = TRUE;
        disp_fb_alloc_info_msg->has_size = TRUE;
        disp_fb_alloc_info_msg->has_luma_bit_depth = TRUE;
        disp_fb_alloc_info_msg->has_chroma_bit_depth = TRUE;
        disp_fb_alloc_info_msg->has_num = TRUE;
        disp_fb_alloc_info_msg->has_endian = TRUE;
        disp_fb_alloc_info_msg->has_type = TRUE;

        ssdk_printf(SSDK_CRIT, "WAVE: disp buf count %d, stride %d, height %d, maptype %d\n",
                disp_fb_alloc_info_msg->num, disp_fb_alloc_info_msg->stride,
                disp_fb_alloc_info_msg->height, disp_fb_alloc_info_msg->map_type);

        if (vb) {
            for (int i = 0; i < disp_fb_alloc_info_msg->num; i++) {
                vb_frame_wtl_buf_ptr[i] = vb[i].phys_addr;
                ssdk_printf(SSDK_INFO, "[%s][%d] disp_addr[%d]:%x\n", __func__, __LINE__, i,
                        vb_frame_wtl_buf_ptr[i]);
            }
            register_dec_buf_msg.vb_frame_wtl_size = vb[0].size;
        } else {
            for (int i = 0; i < disp_fb_alloc_info_msg->num; i++) {
                if (vpu->dec_vb_frame_disp[i].size > 0)
                    _vpu_proto_free_dma_memory(&vpu->dec_vb_frame_disp[i]);

                vpu->dec_vb_frame_disp[i].size = disp_fb_alloc_info_msg->size;
                if (_vpu_proto_allocate_dma_memory(&vpu->dec_vb_frame_disp[i]) < 0) {
                    ssdk_printf(SSDK_CRIT, "%s:%d fail to allocate frame buffer\n", __func__, __LINE__);
                    return -1;
                }

                vb_frame_wtl_buf_ptr[i] = vpu->dec_vb_frame_disp[i].phys_addr;
                ssdk_printf(SSDK_INFO, "[%s][%d] disp_addr[%d]:%x\n", __func__, __LINE__, i,
                        vb_frame_wtl_buf_ptr[i]);
            }
            register_dec_buf_msg.vb_frame_wtl_size = disp_fb_alloc_info_msg->size;
        }

        register_dec_buf_msg.has_vb_frame_wtl_size = TRUE;
        register_dec_buf_msg.n_vb_frame_wtl_phys_addr = disp_fb_alloc_info_msg->num;
        register_dec_buf_msg.vb_frame_wtl_phys_addr = vb_frame_wtl_buf_ptr;
    }

    // proto VPU_IPC_DEC_REGISTER_DECODE_BUFFER msg
    SEND_PROTO_AND_CHECK_RET_SUCCESS(vpu, vpu__dec_register_decode_buffer, register_dec_buf_msg,
                                     register_dec_buf_msg_ret, VPU_IPC_DEC_REGISTER_DECODE_BUFFER,
                                     return -1);
    vpu__dec_register_decode_buffer__free_unpacked(register_dec_buf_msg_ret, NULL);
    return 0;
}

int vpu_proto_dec_clr_all_disp_flag(vpu_instance_t *vpu)
{
    if (!vpu) {
        ssdk_printf(SSDK_CRIT, "[%s][%d] invailed input param, vpu:%p\n", __FUNCTION__, __LINE__, vpu);
        return -1;
    }

    if (!vpu->vpu_handle) {
        ssdk_printf(SSDK_CRIT, "[%s][%d] invailed input param, vpu:%p, vpu_handle:%x\n", __FUNCTION__,
                __LINE__, vpu, vpu->vpu_handle);
        return -1;
    }

    osMutexAcquire (vpu->dec_disp_flag.disp_flag_mutex, osWaitForever);
    
    vpu->dec_disp_flag.clr_disp_flag = 0;
    vpu->dec_disp_flag.set_disp_flag = 0;

    osMutexRelease (vpu->dec_disp_flag.disp_flag_mutex);

    return 0;
}

int vpu_proto_dec_set_disp_flag(vpu_instance_t *vpu, int index, int is_clear)
{
    if (!vpu) {
        ssdk_printf(SSDK_CRIT, "[%s][%d] invailed input param, vpu:%p\n", __FUNCTION__, __LINE__, vpu);
        return -1;
    }

    if (!vpu->vpu_handle || index < 0 || index > MAX_GDI_IDX) {
        ssdk_printf(SSDK_CRIT, "[%s][%d] invailed input param, vpu:%p, vpu_handle:%x, index:%d\n",
                __FUNCTION__, __LINE__, vpu, vpu->vpu_handle, index);
        return -1;
    }

    osMutexAcquire (vpu->dec_disp_flag.disp_flag_mutex, osWaitForever);
    if (is_clear)
        vpu->dec_disp_flag.clr_disp_flag |= 1 << index;
    else
        vpu->dec_disp_flag.set_disp_flag |= 1 << index;
    osMutexRelease (vpu->dec_disp_flag.disp_flag_mutex);

    return 0;
}

int vpu_proto_dec_flush(vpu_instance_t *vpu)
{
    if (!vpu) {
        ssdk_printf(SSDK_CRIT, "[%s][%d] invailed input param, vpu:%p\n", __FUNCTION__, __LINE__, vpu);
        return -1;
    }

    if (!vpu->vpu_handle) {
        ssdk_printf(SSDK_CRIT, "[%s][%d] invailed input param, vpu:%p, vpu_handle:%x\n", __FUNCTION__,
                __LINE__, vpu, vpu->vpu_handle);
        return -1;
    }

    Vpu__DecFlush *dec_flush_msg_ret = NULL;
    Vpu__DecFlush dec_flush_msg = VPU__DEC_FLUSH__INIT;
    Vpu__DecHandle dec_handle_msg = VPU__DEC_HANDLE__INIT;

    dec_flush_msg.handle = &dec_handle_msg;
    dec_handle_msg.handle = vpu->vpu_handle;

    osMutexAcquire (vpu->dec_disp_flag.disp_flag_mutex, osWaitForever);
    dec_flush_msg.clr_disp_flag = vpu->dec_disp_flag.clr_disp_flag;
    dec_flush_msg.set_disp_flag = vpu->dec_disp_flag.set_disp_flag;
    vpu->dec_disp_flag.clr_disp_flag = 0;
    vpu->dec_disp_flag.set_disp_flag = 0;
    osMutexRelease (vpu->dec_disp_flag.disp_flag_mutex);

    dec_flush_msg.has_clr_disp_flag = TRUE;
    dec_flush_msg.has_set_disp_flag = TRUE;

    // proto VPU_IPC_DEC_FLUSH msg
    SEND_PROTO_AND_CHECK_RET_SUCCESS(vpu, vpu__dec_flush, dec_flush_msg, dec_flush_msg_ret,
                                     VPU_IPC_DEC_FLUSH, return -1);

    vpu__dec_flush__free_unpacked(dec_flush_msg_ret, NULL);

    return 0;
}

int vpu_proto_alloc_dma_buffer(vpu_instance_t *vpu, vpu_buffer_t *vb)
{
    if (!vpu || !vb) {
        ssdk_printf(SSDK_CRIT, "[%s][%d] invailed input param, vpu:%p, vb:%p,\n", __func__, __LINE__,
                vpu, vb);
        return -1;
    }

    return _vpu_proto_allocate_dma_memory(vb);
}

void vpu_proto_free_dma_buffer(vpu_instance_t *vpu, vpu_buffer_t *vb)
{
    if (!vpu || !vb) {
        ssdk_printf(SSDK_CRIT, "[%s][%d] invailed input param, vpu:%p, vb:%p,\n", __func__, __LINE__,
                vpu, vb);
        return;
    }

    return _vpu_proto_free_dma_memory(vb);
}

/*******************************enc*************************************/

int vpu_proto_ppu_fb_alloc_info_to_fb(int product_id, Vpu__FrameBufferAllocInfo *info,
                                      Vpu__FrameBuffer *fb, int fb_codec_num)
{
    if (!info || !fb || info->num <= 0 || info->stride <= 0 || info->height <= 0 ||
        info->type != FB_TYPE_PPU) {
        ssdk_printf(SSDK_CRIT, "[%s][%d] invailed param\n", __func__, __LINE__);
        return -1;
    }

    if (info->map_type != LINEAR_FRAME_MAP) {
        ssdk_printf(SSDK_CRIT, "[%s][%d] TODO: support more map_type:%d\n", __func__, __LINE__,
                info->map_type);
        return -1;
    }

    uint32_t size_luma;
    uint32_t size_chroma;

    size_luma = vpu_proto_calc_luma_size(product_id, info->stride, info->height, info->format,
                                         info->cbcr_interleave, info->map_type);
    size_chroma = vpu_proto_calc_chroma_size(product_id, info->stride, info->height, info->format,
                                             info->cbcr_interleave, info->map_type);

    for (int i = 0; i < info->num; i++) {
        if (fb[i].update_fb_info == TRUE) {
            fb[i].update_fb_info = FALSE;
            fb[i].my_index = i + fb_codec_num;
            fb[i].stride = info->stride;
            fb[i].height = info->height;
            fb[i].map_type = info->map_type;
            fb[i].format = info->format;
            fb[i].cbcr_interleave = info->cbcr_interleave;
            fb[i].nv21 = info->nv21;
            fb[i].endian = info->endian;
            fb[i].luma_bit_depth = info->luma_bit_depth;
            fb[i].chroma_bit_depth = info->chroma_bit_depth;
            fb[i].size = info->size;
            fb[i].size_luma = size_luma;
            fb[i].size_chroma = size_chroma;
            fb[i].source_l_burst_en = FALSE;
            fb[i].buf_cb = fb[i].buf_y + size_luma;

            if (info->cbcr_interleave == TRUE)
                fb[i].buf_cr = (uint32_t)-1;
            else
                fb[i].buf_cr = fb[i].buf_cb + size_chroma;

            fb[i].has_update_fb_info = TRUE;
            fb[i].has_my_index = TRUE;
            fb[i].has_stride = TRUE;
            fb[i].has_height = TRUE;
            fb[i].has_map_type = TRUE;
            fb[i].has_format = TRUE;
            fb[i].has_cbcr_interleave = TRUE;
            fb[i].has_nv21 = TRUE;
            fb[i].has_endian = TRUE;
            fb[i].has_luma_bit_depth = TRUE;
            fb[i].has_chroma_bit_depth = TRUE;
            fb[i].has_size = TRUE;
            fb[i].has_size_luma = TRUE;
            fb[i].has_size_chroma = TRUE;
            fb[i].has_source_l_burst_en = TRUE;
            fb[i].has_buf_y = TRUE;
            fb[i].has_buf_cb = TRUE;
            fb[i].has_buf_cr = TRUE;
        }
    }

    return 0;
}

int vpu_proto_enc_open(vpu_instance_t *vpu, vpu_enc_param_t *enc_param)
{
    if (!vpu || !enc_param) {
        ssdk_printf(SSDK_CRIT, "[%s][%d] invailed input param, vpu:%p, enc_param:%p,\n", __func__,
                __LINE__, vpu, enc_param);
        return -1;
    }

    if (vpu->core_idx != VPU_CODA988_IDX) {
        ssdk_printf(SSDK_CRIT, "[%s][%d] invailed core idx:%d\n", __func__, __LINE__, vpu->core_idx);
        return -1;
    }

    if (!vpu->enc_param)
        vpu->enc_param = (vpu_enc_param_t *)malloc(sizeof(vpu_enc_param_t));
    else {
        // already open vpu, close it
        vpu_proto_enc_close(vpu);
    }

    *vpu->enc_param = *enc_param;

    // VPU ENC open
    {
        Vpu__EncOpen *enc_open_msg_ret = NULL;
        Vpu__EncOpen enc_open_msg = VPU__ENC_OPEN__INIT;
        Vpu__EncSliceMode slice_mode_msg = VPU__ENC_SLICE_MODE__INIT;
        Vpu__EncHandle enc_handle_msg = VPU__ENC_HANDLE__INIT;
        Vpu__EncOpenParam *enc_op_msg = &vpu->vpu_enc_proto_info.enc_open_param;

        *enc_op_msg = (Vpu__EncOpenParam)VPU__ENC_OPEN_PARAM__INIT;
        enc_open_msg.handle = &enc_handle_msg;
        enc_open_msg.enc_op = enc_op_msg;

        // enc open param for coda
        enc_op_msg->bitstream_format = vpu->enc_param->bitstream_format;
        enc_op_msg->cbcr_interleave = vpu->enc_param->cbcr_interleave;
        enc_op_msg->nv21 = vpu->enc_param->nv21;
        enc_op_msg->cbcr_order = vpu->enc_param->cbcr_order;
        enc_op_msg->pic_width = vpu->enc_param->pic_width;
        enc_op_msg->pic_height = vpu->enc_param->pic_height;
        enc_op_msg->frame_rate_info = vpu->enc_param->frame_rate_info;
        enc_op_msg->bit_rate = vpu->enc_param->bit_rate;
        enc_op_msg->frame_skip_disable = vpu->enc_param->frame_skip_disable;
        enc_op_msg->gop_size = vpu->enc_param->gop_size;
        enc_op_msg->intra_refresh = vpu->enc_param->intra_refresh;
        enc_op_msg->rc_enable = vpu->enc_param->rc_enable;
        enc_op_msg->sram_enable = vpu->enc_param->sram_enable;

        enc_op_msg->bitstream_buffer = vpu->vb_stream.phys_addr;
        enc_op_msg->bitstream_buffer_size = vpu->vb_stream.size;
        enc_op_msg->ring_buffer_enable = 0;
        enc_op_msg->core_idx = vpu->core_idx;
        enc_op_msg->frame_endian = VDI_LITTLE_ENDIAN;
        enc_op_msg->stream_endian = VDI_LITTLE_ENDIAN;
        enc_op_msg->bwb_enable = TRUE;
        enc_op_msg->line_buf_int_en = 0;
        enc_op_msg->linear2_tiled_enable = FALSE;
        enc_op_msg->linear2_tiled_mode = 0;
        enc_op_msg->initial_delay = INITIAL_DELAY;
        enc_op_msg->vbv_buffer_size = INITIAL_DELAY * vpu->enc_param->bit_rate / 1000;
        enc_op_msg->me_blk_mode = 0;
        enc_op_msg->idr_interval = 0;
        enc_op_msg->consc_intra_refresh_enable = 0;
        enc_op_msg->count_intra_mb_enable = 0;
        enc_op_msg->field_seq_intra_refresh_enable = 0;
        enc_op_msg->rc_intra_qp = -1;
        enc_op_msg->intra_cost_weight = 0;
        enc_op_msg->rc_gop_i_qp_offset_en = 0;
        enc_op_msg->rc_gop_i_qp_offset = 0;
        enc_op_msg->me_search_range_x = 3;
        enc_op_msg->me_search_range_y = 2;
        enc_op_msg->max_intra_size = 0;
        enc_op_msg->user_gamma = -1;
        enc_op_msg->me_use_zero_pmv = 0;
        enc_op_msg->rc_interval_mode = 0;
        enc_op_msg->mb_interval = 1;
        enc_op_msg->map_type = LINEAR_FRAME_MAP;

        if (vpu->enc_param->rc_enable == 2)
            enc_op_msg->user_qp_max = 32;
        else
            enc_op_msg->user_qp_max = 31;

        enc_op_msg->has_bitstream_format = TRUE;
        enc_op_msg->has_bitstream_buffer = TRUE;
        enc_op_msg->has_bitstream_buffer_size = TRUE;
        enc_op_msg->has_ring_buffer_enable = TRUE;
        enc_op_msg->has_cbcr_interleave = TRUE;
        enc_op_msg->has_nv21 = TRUE;
        enc_op_msg->has_frame_endian = TRUE;
        enc_op_msg->has_stream_endian = TRUE;
        enc_op_msg->has_bwb_enable = TRUE;
        enc_op_msg->has_line_buf_int_en = TRUE;
        enc_op_msg->has_core_idx = TRUE;
        enc_op_msg->has_linear2_tiled_enable = TRUE;
        enc_op_msg->has_linear2_tiled_mode = TRUE;
        enc_op_msg->has_cbcr_order = TRUE;
        enc_op_msg->has_pic_width = TRUE;
        enc_op_msg->has_pic_height = TRUE;
        enc_op_msg->has_frame_rate_info = TRUE;
        enc_op_msg->has_bit_rate = TRUE;
        enc_op_msg->has_initial_delay = TRUE;
        enc_op_msg->has_vbv_buffer_size = TRUE;
        enc_op_msg->has_frame_skip_disable = TRUE;
        enc_op_msg->has_me_blk_mode = TRUE;
        enc_op_msg->has_gop_size = TRUE;
        enc_op_msg->has_idr_interval = TRUE;
        enc_op_msg->has_intra_refresh = TRUE;
        enc_op_msg->has_consc_intra_refresh_enable = TRUE;
        enc_op_msg->has_count_intra_mb_enable = TRUE;
        enc_op_msg->has_field_seq_intra_refresh_enable = TRUE;
        enc_op_msg->has_rc_intra_qp = TRUE;
        enc_op_msg->has_intra_cost_weight = TRUE;
        enc_op_msg->has_rc_gop_i_qp_offset_en = TRUE;
        enc_op_msg->has_rc_gop_i_qp_offset = TRUE;
        enc_op_msg->has_me_search_range_x = TRUE;
        enc_op_msg->has_me_search_range_y = TRUE;
        enc_op_msg->has_max_intra_size = TRUE;
        enc_op_msg->has_rc_enable = TRUE;
        enc_op_msg->has_user_gamma = TRUE;
        enc_op_msg->has_me_use_zero_pmv = TRUE;
        enc_op_msg->has_user_qp_max = TRUE;
        enc_op_msg->has_rc_interval_mode = TRUE;
        enc_op_msg->has_mb_interval = TRUE;
        enc_op_msg->has_map_type = TRUE;
        enc_op_msg->has_sram_enable = TRUE;

        // slice mode
        enc_op_msg->slice_mode = &slice_mode_msg;
        slice_mode_msg.slice_mode = 0;
        slice_mode_msg.slice_size_mode = 0;
        slice_mode_msg.slice_size = 8192;
        slice_mode_msg.has_slice_mode = TRUE;
        slice_mode_msg.has_slice_size_mode = TRUE;
        slice_mode_msg.has_slice_size = TRUE;

        // enc codc std param
        enc_op_msg->enc_std_param = &vpu->vpu_enc_proto_info.enc_std_param;
        *enc_op_msg->enc_std_param = (Vpu__EncStdParam)VPU__ENC_STD_PARAM__INIT;
        if (enc_op_msg->bitstream_format == (int)STD_AVC) {
            enc_op_msg->user_max_delta_qp = -1;
            enc_op_msg->user_qp_min = -1;
            enc_op_msg->user_min_delta_qp = -1;
            enc_op_msg->has_user_max_delta_qp = TRUE;
            enc_op_msg->has_user_qp_min = TRUE;
            enc_op_msg->has_user_min_delta_qp = TRUE;

            // avc param
            enc_op_msg->enc_std_param->param_case = VPU__ENC_STD_PARAM__PARAM_AVC_PARAM;
            enc_op_msg->enc_std_param->avc_param = &vpu->vpu_enc_proto_info.enc_avc_param;
            *enc_op_msg->enc_std_param->avc_param =
                (Vpu__EncStdParam__EncAvcParam)VPU__ENC_STD_PARAM__ENC_AVC_PARAM__INIT;

            enc_op_msg->enc_std_param->avc_param->constrained_intra_pred_flag = 0;
            enc_op_msg->enc_std_param->avc_param->disable_deblk = 0;
            enc_op_msg->enc_std_param->avc_param->deblk_filter_offset_alpha = 3;
            enc_op_msg->enc_std_param->avc_param->deblk_filter_offset_beta = -4;
            enc_op_msg->enc_std_param->avc_param->chroma_qp_offset = 2;
            enc_op_msg->enc_std_param->avc_param->aud_enable = 0;
            enc_op_msg->enc_std_param->avc_param->field_flag = 0;
            enc_op_msg->enc_std_param->avc_param->field_ref_mode = 1;
            enc_op_msg->enc_std_param->avc_param->pps_num = 1;

            enc_op_msg->enc_std_param->avc_param->frame_cropping_flag =
                vpu->enc_param->enc_std_param.avc_param.frame_cropping_flag;
            enc_op_msg->enc_std_param->avc_param->frame_crop_left =
                vpu->enc_param->enc_std_param.avc_param.frame_crop_left;
            enc_op_msg->enc_std_param->avc_param->frame_crop_right =
                vpu->enc_param->enc_std_param.avc_param.frame_crop_right;
            enc_op_msg->enc_std_param->avc_param->frame_crop_top =
                vpu->enc_param->enc_std_param.avc_param.frame_crop_top;
            enc_op_msg->enc_std_param->avc_param->frame_crop_bottom =
                vpu->enc_param->enc_std_param.avc_param.frame_crop_bottom;
            enc_op_msg->enc_std_param->avc_param->profile =
                vpu->enc_param->enc_std_param.avc_param.profile;
            enc_op_msg->enc_std_param->avc_param->level =
                vpu->enc_param->enc_std_param.avc_param.level;
            enc_op_msg->enc_std_param->avc_param->chroma_format400 =
                vpu->enc_param->chroma_format400;

            enc_op_msg->enc_std_param->avc_param->has_constrained_intra_pred_flag = TRUE;
            enc_op_msg->enc_std_param->avc_param->has_disable_deblk = TRUE;
            enc_op_msg->enc_std_param->avc_param->has_deblk_filter_offset_alpha = TRUE;
            enc_op_msg->enc_std_param->avc_param->has_deblk_filter_offset_beta = TRUE;
            enc_op_msg->enc_std_param->avc_param->has_chroma_qp_offset = TRUE;
            enc_op_msg->enc_std_param->avc_param->has_aud_enable = TRUE;
            enc_op_msg->enc_std_param->avc_param->has_frame_cropping_flag = TRUE;
            enc_op_msg->enc_std_param->avc_param->has_frame_crop_left = TRUE;
            enc_op_msg->enc_std_param->avc_param->has_frame_crop_right = TRUE;
            enc_op_msg->enc_std_param->avc_param->has_frame_crop_top = TRUE;
            enc_op_msg->enc_std_param->avc_param->has_frame_crop_bottom = TRUE;
            enc_op_msg->enc_std_param->avc_param->has_level = TRUE;
            enc_op_msg->enc_std_param->avc_param->has_profile = TRUE;
            enc_op_msg->enc_std_param->avc_param->has_field_flag = TRUE;
            enc_op_msg->enc_std_param->avc_param->has_field_ref_mode = TRUE;
            enc_op_msg->enc_std_param->avc_param->has_chroma_format400 = TRUE;
            enc_op_msg->enc_std_param->avc_param->has_pps_num = TRUE;

            enc_op_msg->enc_std_param->avc_param->pps_param_1 =
                &vpu->vpu_enc_proto_info.enc_pps_param_1;
            *enc_op_msg->enc_std_param->avc_param->pps_param_1 =
                (Vpu__AvcPpsParam)VPU__AVC_PPS_PARAM__INIT;

            enc_op_msg->enc_std_param->avc_param->pps_param_1->pps_id = 0;
            enc_op_msg->enc_std_param->avc_param->pps_param_1->cabac_init_idc = 0;

            if (enc_op_msg->enc_std_param->avc_param->profile == 0) {
                enc_op_msg->enc_std_param->avc_param->pps_param_1->entropy_coding_mode = 0;
                enc_op_msg->enc_std_param->avc_param->pps_param_1->transform8x8_mode = 0;
            } else if (enc_op_msg->enc_std_param->avc_param->profile == 1) {
                enc_op_msg->enc_std_param->avc_param->pps_param_1->entropy_coding_mode = 1;
                enc_op_msg->enc_std_param->avc_param->pps_param_1->transform8x8_mode = 0;

            } else if (enc_op_msg->enc_std_param->avc_param->profile == 2) {
                enc_op_msg->enc_std_param->avc_param->pps_param_1->entropy_coding_mode = 1;
                enc_op_msg->enc_std_param->avc_param->pps_param_1->transform8x8_mode = 1;
            }

            enc_op_msg->enc_std_param->avc_param->pps_param_1->has_pps_id = TRUE;
            enc_op_msg->enc_std_param->avc_param->pps_param_1->has_cabac_init_idc = TRUE;
            enc_op_msg->enc_std_param->avc_param->pps_param_1->has_entropy_coding_mode = TRUE;
            enc_op_msg->enc_std_param->avc_param->pps_param_1->has_transform8x8_mode = TRUE;
        } else if (enc_op_msg->bitstream_format == (int)STD_MPEG4) {
            // mpeg4 param
            enc_op_msg->enc_std_param->param_case = VPU__ENC_STD_PARAM__PARAM_MP4_PARAM;
            enc_op_msg->enc_std_param->mp4_param = &vpu->vpu_enc_proto_info.enc_mpeg4_param;
            *enc_op_msg->enc_std_param->mp4_param =
                (Vpu__EncStdParam__EncMp4Param)VPU__ENC_STD_PARAM__ENC_MP4_PARAM__INIT;

            enc_op_msg->enc_std_param->mp4_param->mp4_data_partition_enable =
                vpu->enc_param->enc_std_param.mpeg4_param.mp4_data_partition_enable;
            enc_op_msg->enc_std_param->mp4_param->mp4_reversible_vlc_enable =
                vpu->enc_param->enc_std_param.mpeg4_param.mp4_reversible_vlc_enable;
            enc_op_msg->enc_std_param->mp4_param->mp4_hec_enable =
                vpu->enc_param->enc_std_param.mpeg4_param.mp4_hec_enable;
            enc_op_msg->enc_std_param->mp4_param->mp4_intra_dc_vlc_thr =
                vpu->enc_param->enc_std_param.mpeg4_param.mp4_intra_dc_vlc_thr;
            enc_op_msg->enc_std_param->mp4_param->mp4_verid =
                vpu->enc_param->enc_std_param.mpeg4_param.mp4_verid;

            enc_op_msg->enc_std_param->mp4_param->has_mp4_data_partition_enable = TRUE;
            enc_op_msg->enc_std_param->mp4_param->has_mp4_reversible_vlc_enable = TRUE;
            enc_op_msg->enc_std_param->mp4_param->has_mp4_intra_dc_vlc_thr = TRUE;
            enc_op_msg->enc_std_param->mp4_param->has_mp4_hec_enable = TRUE;
            enc_op_msg->enc_std_param->mp4_param->has_mp4_verid = TRUE;

        } else if (enc_op_msg->bitstream_format == (int)STD_H263) {
            // h263 param
            enc_op_msg->enc_std_param->param_case = VPU__ENC_STD_PARAM__PARAM_H263_PARAM;
            enc_op_msg->enc_std_param->h263_param = &vpu->vpu_enc_proto_info.enc_h263_param;
            *enc_op_msg->enc_std_param->h263_param =
                (Vpu__EncStdParam__EncH263Param)VPU__ENC_STD_PARAM__ENC_H263_PARAM__INIT;

            enc_op_msg->enc_std_param->h263_param->h263_annex_i_enable =
                vpu->enc_param->enc_std_param.h263_param.h263_annex_i_enable;
            enc_op_msg->enc_std_param->h263_param->h263_annex_j_enable =
                vpu->enc_param->enc_std_param.h263_param.h263_annex_j_enable;
            enc_op_msg->enc_std_param->h263_param->h263_annex_k_enable =
                vpu->enc_param->enc_std_param.h263_param.h263_annex_k_enable;
            enc_op_msg->enc_std_param->h263_param->h263_annex_t_enable =
                vpu->enc_param->enc_std_param.h263_param.h263_annex_t_enable;

            enc_op_msg->enc_std_param->h263_param->has_h263_annex_i_enable = TRUE;
            enc_op_msg->enc_std_param->h263_param->has_h263_annex_j_enable = TRUE;
            enc_op_msg->enc_std_param->h263_param->has_h263_annex_k_enable = TRUE;
            enc_op_msg->enc_std_param->h263_param->has_h263_annex_t_enable = TRUE;
        }

        // proto VPU_IPC_ENC_OPEN msg
        SEND_PROTO_AND_CHECK_RET_SUCCESS(vpu, vpu__enc_open, enc_open_msg, enc_open_msg_ret,
                                         VPU_IPC_ENC_OPEN, return -1);

        vpu->vpu_handle = enc_open_msg_ret->handle->handle;
        vpu__enc_open__free_unpacked(enc_open_msg_ret, NULL);

        ssdk_printf(SSDK_INFO, "VPU_EncOpen success, vpu handle:0x%x\n", vpu->vpu_handle);
    }

    // VPU_EncGetInitialInfo
    {
        Vpu__EncGetInitialInfo *get_init_info_msg_ret = NULL;
        Vpu__EncGetInitialInfo get_init_info_msg =
            (Vpu__EncGetInitialInfo)VPU__ENC_GET_INITIAL_INFO__INIT;
        Vpu__EncHandle enc_handle_msg = VPU__ENC_HANDLE__INIT;

        get_init_info_msg.handle = &enc_handle_msg;
        enc_handle_msg.handle = vpu->vpu_handle;

        // proto VPU_IPC_ENC_GET_INITIAL_INFO
        SEND_PROTO_AND_CHECK_RET_SUCCESS(vpu, vpu__enc_get_initial_info, get_init_info_msg,
                                         get_init_info_msg_ret, VPU_IPC_ENC_GET_INITIAL_INFO,
                                         return -1);

        if (!get_init_info_msg_ret->initial_info ||
            !get_init_info_msg_ret->initial_info->has_min_frame_buffer_count ||
            !get_init_info_msg_ret->initial_info->has_min_src_frame_count) {
            ssdk_printf(SSDK_CRIT, "[%s][%d] VPU_IPC_ENC_GET_INITIAL_INFO invailed ret msg\n", __func__,
                    __LINE__);
            vpu__enc_get_initial_info__free_unpacked(get_init_info_msg_ret, NULL);
            return -1;
        }

        memcpy(&vpu->vpu_enc_proto_info.enc_init_info, get_init_info_msg_ret->initial_info,
               sizeof(Vpu__EncInitialInfo));

        vpu__enc_get_initial_info__free_unpacked(get_init_info_msg_ret, NULL);
    }

    return 0;
}

int vpu_proto_enc_register_frame_buffer(vpu_instance_t *vpu)
{
    if (!vpu) {
        ssdk_printf(SSDK_CRIT, "[%s][%d] invailed input param, vpu:%p\n", __FUNCTION__, __LINE__, vpu);
        return -1;
    }

    if (!vpu->enc_param || !vpu->vpu_handle) {
        ssdk_printf(SSDK_CRIT, "[%s][%d] invailed input param, vpu:%p, enc_param:%p, vpu_handle:%x\n",
                __FUNCTION__, __LINE__, vpu, vpu->enc_param, vpu->vpu_handle);
        return -1;
    }

    Vpu__FrameBufferAllocInfo *fb_alloc_info_src = &vpu->vpu_enc_proto_info.fb_alloc_info_src;
    *fb_alloc_info_src = (Vpu__FrameBufferAllocInfo)VPU__FRAME_BUFFER_ALLOC_INFO__INIT;

    // VPU_IPC_ENC_REGISTER_ENCODE_BUFFER
    {
        // vpu hw internal stores reference frames
        Vpu__EncRegisterEncodeBuffer *register_enc_buf_msg_ret = NULL;
        Vpu__EncRegisterEncodeBuffer register_enc_buf_msg =
            (Vpu__EncRegisterEncodeBuffer)VPU__ENC_REGISTER_ENCODE_BUFFER__INIT;
        Vpu__FrameBufferAllocInfo fb_alloc_info_msg =
            (Vpu__FrameBufferAllocInfo)VPU__FRAME_BUFFER_ALLOC_INFO__INIT;
        Vpu__EncHandle enc_handle_msg = VPU__ENC_HANDLE__INIT;

        register_enc_buf_msg.handle = &enc_handle_msg;
        enc_handle_msg.handle = vpu->vpu_handle;

        // vb frame buffer for enc
        register_enc_buf_msg.fb_alloc_info = &fb_alloc_info_msg;

        if (vpu->enc_param->chroma_format400)
            fb_alloc_info_msg.format = FORMAT_400;
        else
            fb_alloc_info_msg.format = FORMAT_420;

        fb_alloc_info_msg.cbcr_interleave = vpu->enc_param->cbcr_interleave;
        fb_alloc_info_msg.nv21 = vpu->enc_param->nv21;
        fb_alloc_info_msg.map_type = LINEAR_FRAME_MAP;
        fb_alloc_info_msg.height = VPU_ALIGN32(vpu->enc_param->pic_height);
        fb_alloc_info_msg.stride = vpu_proto_calc_stride(
            vpu->enc_param->pic_width, vpu->enc_param->pic_height, fb_alloc_info_msg.format,
            vpu->enc_param->cbcr_interleave, fb_alloc_info_msg.map_type, FALSE);
        fb_alloc_info_msg.size =
            vpu_proto_calc_frame_size(vpu->product_id, fb_alloc_info_msg.stride,
                                      fb_alloc_info_msg.height, fb_alloc_info_msg.map_type,
                                      fb_alloc_info_msg.format, fb_alloc_info_msg.cbcr_interleave);
        fb_alloc_info_msg.luma_bit_depth = 8; // 8-bit depth
        fb_alloc_info_msg.chroma_bit_depth = 8;
        fb_alloc_info_msg.num = vpu->vpu_enc_proto_info.enc_init_info.min_frame_buffer_count;
        fb_alloc_info_msg.endian = vpu->vpu_enc_proto_info.enc_open_param.frame_endian;
        fb_alloc_info_msg.type = FB_TYPE_CODEC;

        fb_alloc_info_msg.has_format = TRUE;
        fb_alloc_info_msg.has_cbcr_interleave = TRUE;
        fb_alloc_info_msg.has_nv21 = TRUE;
        fb_alloc_info_msg.has_map_type = TRUE;
        fb_alloc_info_msg.has_height = TRUE;
        fb_alloc_info_msg.has_stride = TRUE;
        fb_alloc_info_msg.has_size = TRUE;
        fb_alloc_info_msg.has_luma_bit_depth = TRUE;
        fb_alloc_info_msg.has_chroma_bit_depth = TRUE;
        fb_alloc_info_msg.has_num = TRUE;
        fb_alloc_info_msg.has_endian = TRUE;
        fb_alloc_info_msg.has_type = TRUE;

        ssdk_printf(SSDK_INFO,
                "[%s][%d] frame_buffer_count %d, stride %d, height %d, maptype %d size:%d\n",
                __func__, __LINE__, fb_alloc_info_msg.num, fb_alloc_info_msg.stride,
                fb_alloc_info_msg.height, fb_alloc_info_msg.map_type, fb_alloc_info_msg.size);

        uint32_t vb_frame_enc_buf_ptr[fb_alloc_info_msg.num];
        for (int i = 0; i < fb_alloc_info_msg.num; i++) {
            vpu->enc_vb_frame[i].size = fb_alloc_info_msg.size;
            if (_vpu_proto_allocate_dma_memory(&vpu->enc_vb_frame[i]) < 0) {
                ssdk_printf(SSDK_CRIT, "%s:%d fail to allocate frame buffer\n", __func__, __LINE__);
                return -1;
            }

            vb_frame_enc_buf_ptr[i] = vpu->enc_vb_frame[i].phys_addr;
            ssdk_printf(SSDK_INFO, "[%s][%d] enc_addr[%d]:%x\n", __func__, __LINE__, i,
                    vb_frame_enc_buf_ptr[i]);
        }

        register_enc_buf_msg.has_vb_frame_buf_size = TRUE;
        register_enc_buf_msg.vb_frame_buf_size = fb_alloc_info_msg.size;
        register_enc_buf_msg.n_vb_frame_buf_phys_addr = fb_alloc_info_msg.num;
        register_enc_buf_msg.vb_frame_buf_phys_addr = vb_frame_enc_buf_ptr;

        // proto VPU_IPC_ENC_REGISTER_ENCODE_BUFFER msg
        SEND_PROTO_AND_CHECK_RET_SUCCESS(vpu, vpu__enc_register_encode_buffer, register_enc_buf_msg,
                                         register_enc_buf_msg_ret,
                                         VPU_IPC_ENC_REGISTER_ENCODE_BUFFER, return -1);
        vpu__enc_register_encode_buffer__free_unpacked(register_enc_buf_msg_ret, NULL);

        *fb_alloc_info_src = fb_alloc_info_msg;
    }

    {
        // vb buffer for input src, vpu proto internal use
        fb_alloc_info_src->num = 1;
        fb_alloc_info_src->stride = vpu_proto_calc_stride(
            vpu->enc_param->pic_width, vpu->enc_param->pic_height, fb_alloc_info_src->format,
            vpu->enc_param->cbcr_interleave, fb_alloc_info_src->map_type, FALSE);
        // note: input buffer height != reference buffer height
        fb_alloc_info_src->height = vpu->enc_param->pic_height;
        fb_alloc_info_src->type = FB_TYPE_PPU;
        fb_alloc_info_src->size = vpu_proto_calc_frame_size(
            vpu->product_id, fb_alloc_info_src->stride, fb_alloc_info_src->height,
            fb_alloc_info_src->map_type, fb_alloc_info_src->format,
            fb_alloc_info_src->cbcr_interleave);

        vpu->enc_vb_src.size = fb_alloc_info_src->size;
        if (_vpu_proto_allocate_dma_memory(&vpu->enc_vb_src) < 0) {
            ssdk_printf(SSDK_CRIT, "%s:%d fail to allocate frame buffer\n", __func__, __LINE__);
            return -1;
        }

        ssdk_printf(SSDK_INFO,
                "[%s][%d] frame_buffer_count %d, stride %d, height %d, maptype %d size:%d\n",
                __func__, __LINE__, fb_alloc_info_src->num, fb_alloc_info_src->stride,
                fb_alloc_info_src->height, fb_alloc_info_src->map_type, fb_alloc_info_src->size);

        vpu->vpu_enc_proto_info.fb_src = (Vpu__FrameBuffer)VPU__FRAME_BUFFER__INIT;
        vpu->vpu_enc_proto_info.fb_src.buf_y = vpu->enc_vb_src.phys_addr;
        vpu->vpu_enc_proto_info.fb_src.buf_cb = -1;
        vpu->vpu_enc_proto_info.fb_src.buf_cr = -1;
        vpu->vpu_enc_proto_info.fb_src.update_fb_info = TRUE;

        vpu_proto_ppu_fb_alloc_info_to_fb(
            vpu->product_id, fb_alloc_info_src, &vpu->vpu_enc_proto_info.fb_src,
            vpu->vpu_enc_proto_info.enc_init_info.min_frame_buffer_count);
    }

    return 0;
}

int vpu_proto_enc_get_cfg_frame(vpu_instance_t *vpu, unsigned char *data, int data_size)
{
    int feed_size = 0;

    if (!vpu) {
        ssdk_printf(SSDK_CRIT, "[%s][%d] invailed input param, vpu:%p\n", __FUNCTION__, __LINE__, vpu);
        return -1;
    }

    if (!vpu->enc_param || !vpu->vpu_handle || !data || data_size < vpu->vb_stream.size) {
        ssdk_printf(SSDK_CRIT,
                "[%s][%d] invailed input param, vpu:%p, enc_param:%p, vpu_handle:%x, data:%p, "
                "data_size:%d\n",
                __FUNCTION__, __LINE__, vpu, vpu->enc_param, vpu->vpu_handle, data, data_size);
        return -1;
    }

    if (vpu->enc_param->bitstream_format != STD_AVC &&
        vpu->enc_param->bitstream_format != STD_MPEG4) {
        ssdk_printf(SSDK_CRIT, "[%s][%d] don't support bitstream format:%d\n", __func__, __LINE__,
                vpu->enc_param->bitstream_format);
        return -1;
    }

    // VPU_IPC_ENC_GET_HEAD
    {
        Vpu__EncGetHead *enc_get_head_msg_ret = NULL;
        Vpu__EncGetHead enc_get_head_msg = (Vpu__EncGetHead)VPU__ENC_GET_HEAD__INIT;
        Vpu__EncHandle enc_handle_msg = VPU__ENC_HANDLE__INIT;

        enc_get_head_msg.handle = &enc_handle_msg;
        enc_handle_msg.handle = vpu->vpu_handle;

        enc_get_head_msg.has_bitstream_format = TRUE;
        enc_get_head_msg.bitstream_format = vpu->enc_param->bitstream_format;
        enc_get_head_msg.has_input_buf_addr = TRUE;
        enc_get_head_msg.input_buf_addr = vpu->vb_stream.phys_addr;
        enc_get_head_msg.has_input_buf_size = TRUE;
        enc_get_head_msg.input_buf_size = vpu->vb_stream.size;

        // proto VPU_IPC_ENC_GET_HEAD msg
        SEND_PROTO_AND_CHECK_RET_SUCCESS(vpu, vpu__enc_get_head, enc_get_head_msg,
                                         enc_get_head_msg_ret, VPU_IPC_ENC_GET_HEAD, return -1);

        if (!enc_get_head_msg_ret->n_output_buf_addr || !enc_get_head_msg_ret->output_buf_addr ||
            !enc_get_head_msg_ret->n_output_buf_size || !enc_get_head_msg_ret->output_buf_size ||
            enc_get_head_msg_ret->n_output_buf_addr != enc_get_head_msg_ret->n_output_buf_size) {
            ssdk_printf(SSDK_CRIT, "[%s][%d] VPU_IPC_ENC_GET_HEAD invailed ret msg\n", __func__,
                    __LINE__);
            vpu__enc_get_head__free_unpacked(enc_get_head_msg_ret, NULL);
            return -1;
        }

        for (size_t i = 0; i < enc_get_head_msg_ret->n_output_buf_addr; i++) {
            _vpu_proto_memcpy_form_device_to_cpu(
                data + feed_size, (void *)ap2p((paddr_t)enc_get_head_msg_ret->output_buf_addr[i]),
                enc_get_head_msg_ret->output_buf_size[i]);

            feed_size += enc_get_head_msg_ret->output_buf_size[i];
        }

        vpu__enc_get_head__free_unpacked(enc_get_head_msg_ret, NULL);
    }

    return feed_size;
}

int vpu_proto_enc_fill_frame_buffer(vpu_instance_t *vpu, vpu_enc_frame_param_t *enc_frame_param,
                                    Vpu__FrameBuffer *fb_src)
{
    if (!vpu || !enc_frame_param || !fb_src) {
        ssdk_printf(SSDK_CRIT, "[%s][%d] invailed input param, vpu:%p, enc_frame_param:%p, fb_src:%p\n",
                __func__, __LINE__, vpu, enc_frame_param, fb_src);
        return -1;
    }

    if (enc_frame_param->input_buf_phys_addr) {
        if (enc_frame_param->input_buf_size != fb_src->size) {
            ssdk_printf(SSDK_CRIT, "[%s][%d] invailed input_buf_size:%d\n", __func__, __LINE__,
                    enc_frame_param->input_buf_size);
            return -1;
        }

        fb_src->buf_y = enc_frame_param->input_buf_phys_addr;
        fb_src->buf_cb = enc_frame_param->input_buf_phys_addr + fb_src->size_luma;

        if (!fb_src->cbcr_interleave)
            fb_src->buf_cr = fb_src->buf_cb + fb_src->size_chroma;

        ssdk_printf(SSDK_INFO, "usr buf phys addr, y:%x, u:%x, v:%x\n", fb_src->buf_y,
                fb_src->buf_cb, fb_src->buf_cr);
    } else if (enc_frame_param->input_buf_virtual_addr) {
        if (enc_frame_param->input_buf_size == fb_src->size) {
            _vpu_proto_memcpy_form_cpu_to_device((void *)ap2p((paddr_t)fb_src->buf_y),
                                                 enc_frame_param->input_buf_virtual_addr,
                                                 fb_src->size);

        } else {
            int pic_width = vpu->enc_param->pic_width;
            int pic_height = vpu->enc_param->pic_height;

            unsigned char *p_src = enc_frame_param->input_buf_virtual_addr;
            uint32_t p_dst = fb_src->buf_y;

            // y
            for (int i = 0; i < pic_height; i++) {
                _vpu_proto_memcpy_form_cpu_to_device(
                    (void *)ap2p((paddr_t)(p_dst + fb_src->stride * i)), p_src + pic_width * i,
                    pic_width);
            }

            if (fb_src->format == (int)FORMAT_400)
                return 0;

            p_src += pic_width * pic_height;
            p_dst = fb_src->buf_cb;

            // cb
            if (!fb_src->cbcr_interleave) {
                for (int i = 0; i < pic_height / 2; i++) {
                    _vpu_proto_memcpy_form_cpu_to_device(
                        (void *)ap2p((paddr_t)(p_dst + fb_src->stride / 2 * i)),
                        p_src + pic_width / 2 * i, pic_width / 2);
                }
            } else {
                for (int i = 0; i < pic_height / 2; i++) {
                    _vpu_proto_memcpy_form_cpu_to_device(
                        (void *)ap2p((paddr_t)(p_dst + fb_src->stride * i)), p_src + pic_width * i,
                        pic_width);
                }
            }

            // cr
            if (!fb_src->cbcr_interleave) {
                p_src += pic_width * pic_height / 4;
                p_dst = fb_src->buf_cr;

                for (int i = 0; i < pic_height / 2; i++) {
                    _vpu_proto_memcpy_form_cpu_to_device(
                        (void *)ap2p((paddr_t)(p_dst + fb_src->stride / 2 * i)),
                        p_src + pic_width / 2 * i, pic_width / 2);
                }
            }
        }
    }

    return 0;
}

int vpu_proto_enc_one_frame(vpu_instance_t *vpu, vpu_enc_frame_param_t *enc_frame_param,
                            vpu_enc_frame_output_info_t *output_info)
{
    int ret = 0;

    if (!vpu) {
        ssdk_printf(SSDK_CRIT, "[%s][%d] invailed input param, vpu:%p\n", __FUNCTION__, __LINE__, vpu);
        return -1;
    }

    if (!vpu->enc_param || !vpu->vpu_handle || !enc_frame_param || !output_info) {
        ssdk_printf(SSDK_CRIT,
                "[%s][%d] invailed input param, vpu:%p, enc_param:%p, vpu_handle:%x, "
                "enc_frame_param:%p, output_info:%p\n",
                __FUNCTION__, __LINE__, vpu, vpu->enc_param, vpu->vpu_handle, enc_frame_param,
                output_info);
        return -1;
    }

    if (enc_frame_param->output_buf_phys_addr &&
        enc_frame_param->output_buf_size < vpu->vb_stream.size) {
        ssdk_printf(SSDK_CRIT, "[%s][%d] invailed output buf size :%d\n", __func__, __LINE__,
                enc_frame_param->output_buf_size);
        return -1;
    }

    ret = vpu_proto_enc_fill_frame_buffer(vpu, enc_frame_param, &vpu->vpu_enc_proto_info.fb_src);
    if (ret < 0) {
        ssdk_printf(SSDK_CRIT, "[%s][%d] vpu_proto_enc_fill_frame_buffer failed\n", __func__, __LINE__);
        return -1;
    }

    // VPU_IPC_ENC_ONE_FRAMER
    Vpu__EncOneFrame *enc_one_frame_msg_ret = NULL;
    Vpu__EncOneFrame enc_one_frame_msg = (Vpu__EncOneFrame)VPU__ENC_ONE_FRAME__INIT;
    Vpu__EncParam enc_param_msg = (Vpu__EncParam)VPU__ENC_PARAM__INIT;
    Vpu__EncHandle enc_handle_msg = VPU__ENC_HANDLE__INIT;

    enc_one_frame_msg.handle = &enc_handle_msg;
    enc_handle_msg.handle = vpu->vpu_handle;

    enc_one_frame_msg.enc_param = &enc_param_msg;

    enc_param_msg.source_frame = &vpu->vpu_enc_proto_info.fb_src;
    enc_param_msg.quant_param = enc_frame_param->quant_param;
    enc_param_msg.force_i_picture = enc_frame_param->is_key_frame;
    enc_param_msg.skip_picture = 0;
    enc_param_msg.src_idx = 0;

    if (enc_frame_param->output_buf_phys_addr) {
        enc_param_msg.pic_stream_buffer_addr = enc_frame_param->output_buf_phys_addr;
        enc_param_msg.pic_stream_buffer_size = enc_frame_param->output_buf_size;
    } else {
        enc_param_msg.pic_stream_buffer_addr = vpu->vb_stream.phys_addr;
        enc_param_msg.pic_stream_buffer_size = vpu->vb_stream.size;
    }

    enc_param_msg.has_quant_param = TRUE;
    enc_param_msg.has_force_i_picture = TRUE;
    enc_param_msg.has_skip_picture = TRUE;
    enc_param_msg.has_pic_stream_buffer_addr = TRUE;
    enc_param_msg.has_pic_stream_buffer_size = TRUE;
    enc_param_msg.has_src_idx = TRUE;

    if (enc_frame_param->new_bit_rate) {
        enc_param_msg.new_bit_rate = enc_frame_param->new_bit_rate;
        enc_param_msg.has_new_bit_rate = TRUE;
    }

    if (enc_frame_param->new_frame_rate) {
        enc_param_msg.new_frame_rate = enc_frame_param->new_frame_rate;
        enc_param_msg.has_new_frame_rate = TRUE;
    }

    if (enc_frame_param->new_gop_size) {
        enc_param_msg.new_gop_size = enc_frame_param->new_gop_size;
        enc_param_msg.has_new_gop_size = TRUE;
    }

    // proto VPU_IPC_ENC_ONE_FRAMER msg
    SEND_PROTO_AND_CHECK_RET_SUCCESS(vpu, vpu__enc_one_frame, enc_one_frame_msg,
                                     enc_one_frame_msg_ret, VPU_IPC_ENC_ONE_FRAMER, return -1);

    if (!enc_one_frame_msg_ret->enc_output_info ||
        !enc_one_frame_msg_ret->enc_output_info->has_bitstream_size ||
        !enc_one_frame_msg_ret->enc_output_info->has_bitstream_buffer ||
        !enc_one_frame_msg_ret->enc_output_info->has_rd_ptr ||
        !enc_one_frame_msg_ret->enc_output_info->has_wr_ptr ||
        !enc_one_frame_msg_ret->enc_output_info->bitstream_size ||
        !enc_one_frame_msg_ret->enc_output_info->bitstream_buffer) {
        ssdk_printf(SSDK_CRIT, "[%s][%d] VPU_IPC_ENC_ONE_FRAMER invailed ret msg\n", __func__, __LINE__);
        vpu__enc_one_frame__free_unpacked(enc_one_frame_msg_ret, NULL);
        return -1;
    }

    if (!enc_frame_param->output_buf_phys_addr) {
        output_info->output_buf =
            (void *)ap2p((paddr_t)enc_one_frame_msg_ret->enc_output_info->bitstream_buffer);
    }

    output_info->output_buf_size = enc_one_frame_msg_ret->enc_output_info->bitstream_size;
    output_info->output_pic_type = enc_one_frame_msg_ret->enc_output_info->pic_type;

    ssdk_printf(SSDK_INFO,
            "[%s][%d] VPU_IPC_ENC_ONE_FRAMER enc size:%d pic_type:%d rd_ptr:%x wr_ptr:%x\n",
            __func__, __LINE__, enc_one_frame_msg_ret->enc_output_info->bitstream_size,
            enc_one_frame_msg_ret->enc_output_info->pic_type,
            enc_one_frame_msg_ret->enc_output_info->rd_ptr,
            enc_one_frame_msg_ret->enc_output_info->wr_ptr);
    vpu__enc_one_frame__free_unpacked(enc_one_frame_msg_ret, NULL);

    return output_info->output_buf_size;
}

int vpu_proto_sw_reset(vpu_instance_t *vpu)
{
    if (!vpu) {
        ssdk_printf(SSDK_CRIT, "[%s][%d] invailed input param, vpu:%p\n", __FUNCTION__, __LINE__, vpu);
        return -1;
    }

    if (!vpu->vpu_handle) {
        ssdk_printf(SSDK_CRIT, "[%s][%d] invailed input param, vpu:%p, vpu_handle:%x,\n", __FUNCTION__,
                __LINE__, vpu, vpu->vpu_handle);
        return -1;
    }

    Vpu__SWReset *sw_reset_msg_ret = NULL;
    Vpu__SWReset sw_reset_msg = (Vpu__SWReset)VPU__SWRESET__INIT;

    sw_reset_msg.core_idx = vpu->core_idx;
    sw_reset_msg.reset_mode = VPU__SWRESET_MODE__SW_RESET_SAFETY;
    sw_reset_msg.handle = vpu->vpu_handle;

    sw_reset_msg.has_reset_mode = TRUE;
    sw_reset_msg.has_handle = TRUE;

    // proto VPU_IPC_SWRESET msg
    SEND_PROTO_AND_CHECK_RET_SUCCESS(vpu, vpu__swreset, sw_reset_msg, sw_reset_msg_ret,
                                     VPU_IPC_SWRESET, return -1);

    vpu__swreset__free_unpacked(sw_reset_msg_ret, NULL);

    return 0;
}