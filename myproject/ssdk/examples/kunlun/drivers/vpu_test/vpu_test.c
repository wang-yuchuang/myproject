#include <CLI_type.h>
#include <cmsis_os2.h>
#include <debug.h>
#include <getopt.h>
#include <malloc.h>
#include <param.h>
#include <reg.h>
#include <res_loader.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sdm_display.h>
#include <disp_data_type.h>
#include <driver/memmap.h>

#include <coda9/coda9_regdefine.h>
#include <coda9/coda9_vpuconfig.h>
#include <vpuapifunc.h>
#include <wave/common/common_regdefine.h>
#include <wave/common/common_vpuconfig.h>
#include <wave/wave4/wave4_regdefine.h>

#include "vpu.pb-c.h"
#include "vpu_instance.h"

#define VPU_CODA988_IDX 0
#define VPU_WAVE412_IDX 1
#define VPU_WAVE521_IDX 2

#define VPU_STREAM_BUF_SIZE 0x100000 // 1M

#define VPU_EXTRA_FRAME_BUFFER_NUM 1
#define VPU_PROTO_BUFFER_SIZE 4096
#define VPU_ENC_OUTPUT_BS_BUFFER_SIZE 0x200000 //2M

#define VPU_TEST_H264_RESOURCE_PATH "early_app/video/bbb_avc_176x144_15fps.h264"
#define VPU_TEST_H264_RESOURCE_INFO_PATH "early_app/video/bbb_avc_176x144_15fps.info"
#define VPU_TEST_H265_RESOURCE_PATH "early_app/video/bbb_hevc_176x144_15fps.hevc"
#define VPU_TEST_H265_RESOURCE_INFO_PATH "early_app/video/bbb_hevc_176x144_15fps.info"

#define VPU_TEST_ENC_WIDTH 176
#define VPU_TEST_ENC_HEIGHT 144
#define VPU_TEST_ENC_H264_RESOURCE_PATH "early_app/video/176x144_I420.yuv"
#define VPU_TEST_H265_ENC_WIDTH 352
#define VPU_TEST_H265_ENC_HEIGHT 288
#define VPU_TEST_ENC_H265_RESOURCE_PATH "early_app/video/352x288_5_frames.yuv"

typedef struct file_resource {
    const char *resource_path;
    int resource_size;
    void *resource_buffer;
} file_resource_t;

typedef struct file_resource_info {
    unsigned int first_offset;
    unsigned int second_offest;
    file_resource_t resource;
} file_resource_info_t;

typedef struct vpu_proto_info {
    // dec open
    Vpu__DecOpenParam dec_open_param;

    // seq init
    Vpu__VpuRect init_pic_crop_rect;
    Vpu__AvcVuiInfo init_avc_vui_info;
    Vpu__DecInitialInfo init_output_info;

    // frame buffer
    Vpu__FrameBufferAllocInfo dec_fb_alloc_info_dec;
    Vpu__FrameBufferAllocInfo dec_fb_alloc_info_disp;

    // dec output info
    Vpu__VpuRect dec_rc_display;
    Vpu__AvcVuiInfo dec_avc_vui_info;
    Vpu__DecHevcInfo__H265Info dec_hevc_h265_info;
    Vpu__FrameBuffer dec_disp_frame;
    Vpu__DecOutputInfo dec_output_info;
} vpu_proto_info_t;

typedef struct vpu_config {
    int core_idx;
    TiledMapType map_type;

    sdm_display_t *display;
    uint32_t clr_disp_flag;

    // common buffer
    vpu_buffer_t vb_stream;
    vpu_buffer_t vb_stream_wave_more;

    // dec buffer
    vpu_buffer_t dec_vb_frame_dec[MAX_REG_FRAME];
    vpu_buffer_t dec_vb_frame_disp[MAX_REG_FRAME];

    vpu_proto_info_t vpu_proto_info;

    file_resource_info_t dec_resource_info;
    file_resource_info_t dec_resource;

    vpu_ipcc_data_t pack;
    void *msg_buf;
    uint32_t vpu_handle;
} vpu_config_t; // for dec

typedef struct vpu_enc_config {
    int core_idx;
    TiledMapType map_type;

    sdm_display_t *display;

    // common buffer
    vpu_buffer_t vb_stream;

    // enc buffer
    vpu_buffer_t enc_vb_frame[MAX_REG_FRAME]; // for refer
    vpu_buffer_t enc_vb_src;

    // enc open proto
    Vpu__EncOpenParam enc_open_param;
    Vpu__EncStdParam enc_std_param;
    Vpu__EncStdParam__EncAvcParam enc_avc_param;
    Vpu__EncStdParam__EncWaveParam enc_wave_param;
    Vpu__AvcPpsParam enc_pps_param_1;

    // enc init proto
    Vpu__EncInitialInfo enc_init_info;

    // enc regist buffer proto
    Vpu__FrameBufferAllocInfo fb_alloc_info_src;
    Vpu__FrameBuffer fb_src;

    file_resource_info_t enc_output_bs_buf;
    file_resource_info_t enc_resource;

    vpu_ipcc_data_t pack;
    void *msg_buf;
    uint32_t vpu_handle;
} vpu_enc_config_t;

#define SEND_PROTO_AND_CHECK_RET_SUCCESS(proto_msg_head, msg, ret_msg, cmd, error_handle)          \
    do {                                                                                           \
        int ret = 0;                                                                               \
        int msg_len = 0;                                                                           \
        msg_len = proto_msg_head##__get_packed_size(&msg);                                         \
        proto_msg_head##__pack(&msg, vpu_config->msg_buf);                                         \
                                                                                                   \
        ret = vpu_send_msg_to_service(&vpu_config->pack, cmd, vpu_config->msg_buf, msg_len);       \
        if (ret < 0) {                                                                             \
            VLOG(ERR, "%s send failed\n", #msg);                                                   \
            goto msg##_error;                                                                      \
        }                                                                                          \
                                                                                                   \
        ret_msg = proto_msg_head##__unpack(NULL, vpu_config->pack.cmd_param_full_length,           \
                                           vpu_config->pack.cmd_param);                            \
        if (!ret_msg) {                                                                            \
            VLOG(ERR, "[%s][%d] %s__unpack failed\n", __func__, __LINE__, #proto_msg_head);        \
            goto msg##_error;                                                                      \
        }                                                                                          \
                                                                                                   \
        if (!ret_msg->has_ret_code) {                                                              \
            VLOG(ERR, "[%s][%d] %s invailed ret code\n", __func__, __LINE__, #cmd);                \
            proto_msg_head##__free_unpacked(ret_msg, NULL);                                        \
            ret_msg = NULL;                                                                        \
            goto msg##_error;                                                                      \
        }                                                                                          \
                                                                                                   \
        if (ret_msg->ret_code != RETCODE_SUCCESS) {                                                \
            VLOG(ERR, "[%s][%d] %s failed error code:0x%x\n", __func__, __LINE__, #cmd,            \
                 ret_msg->ret_code);                                                               \
            proto_msg_head##__free_unpacked(ret_msg, NULL);                                        \
            ret_msg = NULL;                                                                        \
            goto msg##_error;                                                                      \
        }                                                                                          \
        break;                                                                                     \
        msg##_error : error_handle;                                                                \
                                                                                                   \
    } while (0)

#define SEND_PROTO_AND_CHECK_HAS_RET_CODE(proto_msg_head, msg, ret_msg, cmd, error_handle)         \
    do {                                                                                           \
        int msg_len = 0;                                                                           \
        int ret = 0;                                                                               \
        msg_len = proto_msg_head##__get_packed_size(&msg);                                         \
        proto_msg_head##__pack(&msg, vpu_config->msg_buf);                                         \
                                                                                                   \
        ret = vpu_send_msg_to_service(&vpu_config->pack, cmd, vpu_config->msg_buf, msg_len);       \
        if (ret < 0) {                                                                             \
            VLOG(ERR, "%s send failed\n", #msg);                                                   \
            goto msg##_error;                                                                      \
        }                                                                                          \
                                                                                                   \
        /* pase cmd return msg */                                                                  \
        ret_msg = proto_msg_head##__unpack(NULL, vpu_config->pack.cmd_param_full_length,           \
                                           vpu_config->pack.cmd_param);                            \
        if (!ret_msg) {                                                                            \
            VLOG(ERR, "[%s][%d] %s__unpack failed\n", __func__, __LINE__, #proto_msg_head);        \
            goto msg##_error;                                                                      \
        }                                                                                          \
                                                                                                   \
        if (!ret_msg->has_ret_code) {                                                              \
            VLOG(ERR, "[%s][%d] %s invailed ret code\n", __func__, __LINE__, #cmd);                \
            proto_msg_head##__free_unpacked(ret_msg, NULL);                                        \
            ret_msg = NULL;                                                                        \
            goto msg##_error;                                                                      \
        }                                                                                          \
                                                                                                   \
        break;                                                                                     \
        msg##_error : error_handle;                                                                \
                                                                                                   \
    } while (0)

int get_next_frame_length(file_resource_info_t *resource_info)
{
    int ret = 0;
    char str[32] = {0};

    for (int i = resource_info->first_offset; i < resource_info->resource.resource_size; i++) {
        if (((char *)resource_info->resource.resource_buffer)[i] == ' ') {
            resource_info->second_offest = i;

            strncpy(str,
                    (char *)(resource_info->resource.resource_buffer + resource_info->first_offset),
                    resource_info->second_offest - resource_info->first_offset);

            str[resource_info->second_offest - resource_info->first_offset] = '\0';
            ret = atoi(str);

            break;
        }
    }

    for (int i = resource_info->second_offest; i < resource_info->resource.resource_size; i++) {
        if (((char *)resource_info->resource.resource_buffer)[i] == '\n') {
            resource_info->first_offset = i + 1;
            break;
        }
    }

    return ret;
}

sdm_display_t *vpu_get_screen_display(void)
{
    struct list_node *disp_node = sdm_get_display_list();
    sdm_display_t *disp = NULL;
    disp_node = disp_node->next;

    for (uint8_t i = 0; i < list_length(sdm_get_display_list()); i++) {
        disp = containerof(disp_node, sdm_display_t, node);
        if (disp->handle->display_id == INFOTAINMENT)
            return disp;
        disp_node = disp_node->next;
    }

    return NULL;
}

int vpu_display_to_screen(vpu_config_t *vpu_config, int size)
{
    int n_bufs = 1;
    struct sdm_buffer sdmbuf[1] = {0};
    struct sdm_post_config postConfig = {0};

    if (!vpu_config || vpu_config->vpu_proto_info.dec_output_info.index_frame_display < 0) {
        VLOG(ERR, "Invalid parameter\n");
        return -1;
    }

    if (!vpu_config->display) {
        vpu_config->display = vpu_get_screen_display();
    }

    postConfig.bufs = sdmbuf;
    postConfig.n_bufs = n_bufs;

    for (int i = 0; i < n_bufs; i++) {
        struct sdm_buffer *buf = &postConfig.bufs[i];

        buf->addr[0] =
            (unsigned long)ap2p((paddr_t)vpu_config->vpu_proto_info.dec_disp_frame.buf_y);
        buf->addr[1] =
            (unsigned long)ap2p((paddr_t)vpu_config->vpu_proto_info.dec_disp_frame.buf_cb);
        buf->addr[2] =
            (unsigned long)ap2p((paddr_t)vpu_config->vpu_proto_info.dec_disp_frame.buf_cr);

        buf->layer = 1;
        buf->layer_dirty = 1;
        buf->alpha = 0xff;
        buf->alpha_en = 1;
        buf->ckey = 0;
        buf->ckey_en = 0;
        buf->fmt = COLOR_YUV420P;
        buf->layer_en = 1;
        buf->src.x = 0;
        buf->src.y = 0;
        buf->src.w = vpu_config->vpu_proto_info.dec_fb_alloc_info_dec.stride;
        buf->src.h = vpu_config->vpu_proto_info.dec_fb_alloc_info_dec.height;

        buf->start.x = vpu_config->vpu_proto_info.dec_rc_display.left;
        buf->start.y = vpu_config->vpu_proto_info.dec_rc_display.top;
        buf->start.w = vpu_config->vpu_proto_info.dec_rc_display.right;
        buf->start.h = vpu_config->vpu_proto_info.dec_rc_display.bottom;

        // src_stride
        buf->src_stride[0] = vpu_config->vpu_proto_info.dec_fb_alloc_info_dec.stride;
        buf->src_stride[1] = vpu_config->vpu_proto_info.dec_fb_alloc_info_dec.stride / 2;
        buf->src_stride[2] = vpu_config->vpu_proto_info.dec_fb_alloc_info_dec.stride / 2;

        // 1920x720 is screen size
        buf->dst.x = 0;
        buf->dst.y = 0;
        buf->dst.w = 1920;
        buf->dst.h = 720;
        buf->z_order = 0;
    }

    if (vpu_config->display)
        return sdm_post(vpu_config->display->handle, &postConfig);

    return -1;
}

int vpu_send_msg_to_service(vpu_ipcc_data_t *pack, uint8_t cmd, void *msg, int msg_len)
{
    int ret;
    if (!pack) {
        printf("[%s][%d] null pack ptr\n", __func__, __LINE__);
        return -1;
    }

    osal_memset(pack, 0x0, sizeof(vpu_ipcc_data_t));

    pack->cmd = cmd;
    pack->ipcc_id = 0;
    pack->dev_id = VPU_DEVICES_CODA988; // jsut for test, wave coda use the same id

    printf("[%s][%d] cmd:%x msg_len:%d\n", __func__, __LINE__, cmd, msg_len);
    if (msg_len <= VPU_IPCC_DATA_LEN_MAX) {
        pack->cmd_param_full_length = msg_len;
        pack->current_packed_end_offset = msg_len;

        memcpy(pack->cmd_param, msg, msg_len);

        ret = vpu_instance_process_msg(pack);
        if (ret < 0)
            pack->return_value = VPU_IPC_RETURN_ERROR;
        else
            pack->return_value = VPU_IPC_RETUERN_OK;

    } else {
        printf("[%s][%d] TODO: support msg split\n", __func__, __LINE__);
        return -1;
    }

    if (pack->return_value == VPU_IPC_RETURN_ERROR)
        return -1;

    return 0;
}

// line buffer, write one frame at a time
int feed_bit_stream(vpu_config_t *vpu_config)
{
    int feed_size = 0;
    static uint32_t feed_count = 0;
    PhysicalAddress wr_ptr = 0;
    uint32_t core_idx = vpu_config->core_idx;

    if (vpu_config->vpu_proto_info.dec_open_param.bitstream_mode != BS_MODE_PIC_END) {
        VLOG(ERR, "[%s][%d] will only called by dec pic end mode\n", __func__, __LINE__);
        return 0;
    }

    feed_size = get_next_frame_length(&vpu_config->dec_resource_info);

    if (feed_size < 0) {
        VLOG(ERR, "[%s][%d] get_next_frame_length failed\n", __func__, __LINE__);
        return 0;
    }

    if ((uint32_t)feed_size > vpu_config->vb_stream.size) {
        VLOG(ERR, "The writable room:%u is smaller than the size of the data:%d to be written\n",
             vpu_config->vb_stream.size, feed_size);
        return 0;
    }

    if (core_idx == VPU_CODA988_IDX) {
        wr_ptr = vpu_config->vb_stream.phys_addr;
    } else if (core_idx == VPU_WAVE412_IDX) {
        if (feed_count % 2 == 0)
            wr_ptr = vpu_config->vb_stream.phys_addr;
        else
            wr_ptr = vpu_config->vb_stream_wave_more.phys_addr;
    } else
        return 0;

    if (feed_size > 0) {
        VLOG(INFO, "[%s][%d], wr_ptr(%x), feed_size(%d)\n", __FUNCTION__, __LINE__, wr_ptr,
             feed_size);
        VpuWriteMem(core_idx, wr_ptr,
                    (unsigned char *)vpu_config->dec_resource.resource.resource_buffer +
                        vpu_config->dec_resource.first_offset,
                    feed_size, vpu_config->vpu_proto_info.dec_open_param.stream_endian);
        vpu_config->dec_resource.first_offset += feed_size;

        feed_count++;
    }

    // Vpu__DecPicEndModeUpdateBSBuffer
    {
        Vpu__DecPicEndModeUpdateBSBuffer *pic_end_mode_update_bs_buf_msg_ret = NULL;
        Vpu__DecPicEndModeUpdateBSBuffer pic_end_mode_update_bs_buf_msg =
            VPU__DEC_PIC_END_MODE_UPDATE_BSBUFFER__INIT;
        Vpu__DecHandle dec_handle_msg = VPU__DEC_HANDLE__INIT;

        pic_end_mode_update_bs_buf_msg.handle = &dec_handle_msg;
        dec_handle_msg.handle = vpu_config->vpu_handle;

        pic_end_mode_update_bs_buf_msg.has_addr = TRUE;
        pic_end_mode_update_bs_buf_msg.addr = wr_ptr;

        pic_end_mode_update_bs_buf_msg.has_size = TRUE;
        pic_end_mode_update_bs_buf_msg.size = feed_size;

        pic_end_mode_update_bs_buf_msg.has_flag = TRUE;
        pic_end_mode_update_bs_buf_msg.flag = VPU__STREAM_UP_FLAG__STREAM_UPDATE;

        // proto VPU_IPC_DEC_PIC_END_MODE_UPDATE_BS_BUFFER msg
        SEND_PROTO_AND_CHECK_RET_SUCCESS(vpu__dec_pic_end_mode_update_bsbuffer,
                                         pic_end_mode_update_bs_buf_msg,
                                         pic_end_mode_update_bs_buf_msg_ret,
                                         VPU_IPC_DEC_PIC_END_MODE_UPDATE_BS_BUFFER, return -1);
        vpu__dec_pic_end_mode_update_bsbuffer__free_unpacked(pic_end_mode_update_bs_buf_msg_ret,
                                                             NULL);
    }
    return feed_size;
}

int vpu_test_seq_init(vpu_config_t *vpu_config)
{
    if (!vpu_config) {
        VLOG(ERR, "[%s][%d] null vpu_config\n", __FUNCTION__, __LINE__);
        return -1;
    }

    // VPU_IPC_DEC_SEQINIT

    Vpu__DecSeqInit *seq_init_msg_ret = NULL;
    Vpu__DecSeqInit seq_init_msg = VPU__DEC_SEQ_INIT__INIT;
    Vpu__DecHandle dec_handle_msg = VPU__DEC_HANDLE__INIT;

    seq_init_msg.handle = &dec_handle_msg;
    dec_handle_msg.handle = vpu_config->vpu_handle;

    // proto VPU_IPC_DEC_SEQINIT msg
    SEND_PROTO_AND_CHECK_HAS_RET_CODE(vpu__dec_seq_init, seq_init_msg, seq_init_msg_ret,
                                      VPU_IPC_DEC_SEQINIT, return -1);

    if (seq_init_msg_ret->ret_code != RETCODE_SUCCESS) {
        if (!seq_init_msg_ret->initial_info ||
            !seq_init_msg_ret->initial_info->has_seq_init_err_reason) {
            VLOG(ERR, "[%s][%d] VPU_IPC_DEC_SEQINIT failed error code:0x%x\n", __func__, __LINE__,
                 seq_init_msg_ret->ret_code);
        } else {
            VLOG(ERR, "[ERROR] Failed to SEQ_INIT(ERROR REASON: %d(0x%x) error code:0x%c\n",
                 seq_init_msg_ret->initial_info->seq_init_err_reason,
                 seq_init_msg_ret->initial_info->seq_init_err_reason, seq_init_msg_ret->ret_code);
        }

        vpu__dec_seq_init__free_unpacked(seq_init_msg_ret, NULL);

        return -1;
    }

    if (!seq_init_msg_ret->initial_info || !seq_init_msg_ret->initial_info->pic_crop_rect) {
        VLOG(ERR, "[%s][%d] VPU_IPC_DEC_SEQINIT invailed ret msg\n", __func__, __LINE__);
        vpu__dec_seq_init__free_unpacked(seq_init_msg_ret, NULL);
        return -1;
    }

    memcpy(&vpu_config->vpu_proto_info.init_output_info, seq_init_msg_ret->initial_info,
           sizeof(Vpu__DecInitialInfo));
    memcpy(&vpu_config->vpu_proto_info.init_pic_crop_rect,
           seq_init_msg_ret->initial_info->pic_crop_rect, sizeof(Vpu__VpuRect));

    if (vpu_config->vpu_proto_info.dec_open_param.bitstream_format == STD_AVC) {
        if (seq_init_msg_ret->initial_info->codec_mode_info_case !=
                VPU__DEC_INITIAL_INFO__CODEC_MODE_INFO_AVC_VUI_INFO ||
            !seq_init_msg_ret->initial_info->avc_vui_info) {
            VLOG(ERR, "[%s][%d] VPU_IPC_DEC_SEQINIT invailed avc ret msg\n", __func__, __LINE__);
            vpu__dec_seq_init__free_unpacked(seq_init_msg_ret, NULL);
            return -1;
        }

        memcpy(&vpu_config->vpu_proto_info.init_avc_vui_info,
               seq_init_msg_ret->initial_info->avc_vui_info, sizeof(Vpu__AvcVuiInfo));
    }

    VLOG(ERR,
         "VPU_IPC_DEC_COMPLETE_SEQINIT success, picWidth : %d,picHeight : %d, "
         "picCropRect.right : %d,"
         "picCropRect.bottom : %d, minFrameBufferCount : %d, RdPtr : 0x%x, "
         "WrPtr : 0x%x\n",
         vpu_config->vpu_proto_info.init_output_info.pic_width,
         vpu_config->vpu_proto_info.init_output_info.pic_height,
         vpu_config->vpu_proto_info.init_pic_crop_rect.right,
         vpu_config->vpu_proto_info.init_pic_crop_rect.bottom,
         vpu_config->vpu_proto_info.init_output_info.min_frame_buffer_count,
         vpu_config->vpu_proto_info.init_output_info.rd_ptr,
         vpu_config->vpu_proto_info.init_output_info.wr_ptr);

    vpu__dec_seq_init__free_unpacked(seq_init_msg_ret, NULL);
    return 0;
}

int vpu_test_register_frame_buffer(vpu_config_t *vpu_config)
{
    // VPU_IPC_DEC_REGISTER_DECODE_BUFFER
    Vpu__FrameBufferAllocInfo *dec_fb_alloc_info_msg = NULL;
    Vpu__FrameBufferAllocInfo *disp_fb_alloc_info_msg = NULL;
    Vpu__DecRegisterDecodeBuffer *register_dec_buf_msg_ret = NULL;
    Vpu__DecRegisterDecodeBuffer register_dec_buf_msg = VPU__DEC_REGISTER_DECODE_BUFFER__INIT;
    Vpu__DecHandle dec_handle_msg = VPU__DEC_HANDLE__INIT;

    dec_handle_msg.handle = vpu_config->vpu_handle;
    register_dec_buf_msg.handle = &dec_handle_msg;

    // vb frame buffer for dec
    dec_fb_alloc_info_msg = &vpu_config->vpu_proto_info.dec_fb_alloc_info_dec;
    *dec_fb_alloc_info_msg = (Vpu__FrameBufferAllocInfo)VPU__FRAME_BUFFER_ALLOC_INFO__INIT;
    register_dec_buf_msg.fb_alloc_info_dec = dec_fb_alloc_info_msg;

    dec_fb_alloc_info_msg->format = FORMAT_420;
    dec_fb_alloc_info_msg->cbcr_interleave =
        vpu_config->vpu_proto_info.dec_open_param.cbcr_interleave;
    dec_fb_alloc_info_msg->map_type = vpu_config->map_type;
    dec_fb_alloc_info_msg->height =
        VPU_ALIGN32(vpu_config->vpu_proto_info.init_output_info.pic_height);
    dec_fb_alloc_info_msg->stride = CalcStride(
        vpu_config->vpu_proto_info.init_output_info.pic_width,
        vpu_config->vpu_proto_info.init_output_info.pic_height, dec_fb_alloc_info_msg->format,
        vpu_config->vpu_proto_info.dec_open_param.cbcr_interleave, dec_fb_alloc_info_msg->map_type,
        FALSE);
    dec_fb_alloc_info_msg->size = VPU_GetFrameBufSize(
        (VpuHandle)vpu_config->vpu_handle,
        vpu_config->core_idx, dec_fb_alloc_info_msg->stride, dec_fb_alloc_info_msg->height,
        dec_fb_alloc_info_msg->map_type, dec_fb_alloc_info_msg->format,
        dec_fb_alloc_info_msg->cbcr_interleave, NULL); // dram cfg only needed by coda960
    dec_fb_alloc_info_msg->luma_bit_depth =
        vpu_config->vpu_proto_info.init_output_info.luma_bitdepth;
    dec_fb_alloc_info_msg->chroma_bit_depth =
        vpu_config->vpu_proto_info.init_output_info.chroma_bitdepth;
    dec_fb_alloc_info_msg->num =
        vpu_config->vpu_proto_info.init_output_info.min_frame_buffer_count +
        VPU_EXTRA_FRAME_BUFFER_NUM;
    dec_fb_alloc_info_msg->endian = vpu_config->vpu_proto_info.dec_open_param.frame_endian;
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

    VLOG(ERR, "[main] min_frame_buffer_count %d, stride %d, height %d, maptype %d\n",
         vpu_config->vpu_proto_info.init_output_info.min_frame_buffer_count,
         dec_fb_alloc_info_msg->stride, dec_fb_alloc_info_msg->height,
         dec_fb_alloc_info_msg->map_type);

    VLOG(ERR, "step 1.0 framebuffer count (%d)\n", dec_fb_alloc_info_msg->num);

    uint32_t vb_frame_dec_buf_ptr[dec_fb_alloc_info_msg->num];
    for (int i = 0; i < dec_fb_alloc_info_msg->num; i++) {
        vpu_config->dec_vb_frame_dec[i].size = dec_fb_alloc_info_msg->size;
        if (vdi_allocate_dma_memory(vpu_config->core_idx, &vpu_config->dec_vb_frame_dec[i]) < 0) {
            VLOG(ERR, "%s:%d fail to allocate frame buffer\n", __FUNCTION__, __LINE__);
            return -1;
        }

        vb_frame_dec_buf_ptr[i] = vpu_config->dec_vb_frame_dec[i].phys_addr;
        printf("[%s][%d] dec_addr[%d]:%x\n", __func__, __LINE__, i, vb_frame_dec_buf_ptr[i]);
    }

    register_dec_buf_msg.has_vb_frame_dec_size = TRUE;
    register_dec_buf_msg.vb_frame_dec_size = dec_fb_alloc_info_msg->size;
    register_dec_buf_msg.n_vb_frame_dec_phys_addr = dec_fb_alloc_info_msg->num;
    register_dec_buf_msg.vb_frame_dec_phys_addr = vb_frame_dec_buf_ptr;

    // vb frame buffer for display
    uint32_t vb_frame_wtl_buf_ptr[dec_fb_alloc_info_msg->num];
    if (vpu_config->core_idx == VPU_WAVE412_IDX) {
        disp_fb_alloc_info_msg = &vpu_config->vpu_proto_info.dec_fb_alloc_info_disp;
        *disp_fb_alloc_info_msg = (Vpu__FrameBufferAllocInfo)VPU__FRAME_BUFFER_ALLOC_INFO__INIT;
        register_dec_buf_msg.fb_alloc_info_wtl = disp_fb_alloc_info_msg;

        *disp_fb_alloc_info_msg = *dec_fb_alloc_info_msg;

        disp_fb_alloc_info_msg->map_type = LINEAR_FRAME_MAP;
        disp_fb_alloc_info_msg->stride = CalcStride(
            vpu_config->vpu_proto_info.init_output_info.pic_width,
            vpu_config->vpu_proto_info.init_output_info.pic_height, disp_fb_alloc_info_msg->format,
            vpu_config->vpu_proto_info.dec_open_param.cbcr_interleave,
            disp_fb_alloc_info_msg->map_type, FALSE);
        disp_fb_alloc_info_msg->size = VPU_GetFrameBufSize(
            (VpuHandle)vpu_config->vpu_handle,
            vpu_config->core_idx, disp_fb_alloc_info_msg->stride, disp_fb_alloc_info_msg->height,
            disp_fb_alloc_info_msg->map_type, disp_fb_alloc_info_msg->format,
            disp_fb_alloc_info_msg->cbcr_interleave, NULL); // dram cfg only needed by coda960

        for (int i = 0; i < disp_fb_alloc_info_msg->num; i++) {
            vpu_config->dec_vb_frame_disp[i].size = disp_fb_alloc_info_msg->size;
            if (vdi_allocate_dma_memory(vpu_config->core_idx, &vpu_config->dec_vb_frame_disp[i]) <
                0) {
                VLOG(ERR, "%s:%d fail to allocate frame buffer\n", __FUNCTION__, __LINE__);
                return -1;
            }

            vb_frame_wtl_buf_ptr[i] = vpu_config->dec_vb_frame_disp[i].phys_addr;
            printf("[%s][%d] disp_addr[%d]:%x\n", __func__, __LINE__, i, vb_frame_wtl_buf_ptr[i]);
        }

        register_dec_buf_msg.has_vb_frame_wtl_size = TRUE;
        register_dec_buf_msg.vb_frame_wtl_size = disp_fb_alloc_info_msg->size;
        register_dec_buf_msg.n_vb_frame_wtl_phys_addr = disp_fb_alloc_info_msg->num;
        register_dec_buf_msg.vb_frame_wtl_phys_addr = vb_frame_wtl_buf_ptr;

        for (size_t i = 0; i < register_dec_buf_msg.n_vb_frame_wtl_phys_addr; i++)
            printf("[%s][%d] vb_frame_wtl_phys_addr[%d]:%x\n", __func__, __LINE__, i,
                   register_dec_buf_msg.vb_frame_wtl_phys_addr[i]);
    }

    // proto VPU_IPC_DEC_REGISTER_DECODE_BUFFER msg
    SEND_PROTO_AND_CHECK_RET_SUCCESS(vpu__dec_register_decode_buffer, register_dec_buf_msg,
                                     register_dec_buf_msg_ret, VPU_IPC_DEC_REGISTER_DECODE_BUFFER,
                                     return -1);
    vpu__dec_register_decode_buffer__free_unpacked(register_dec_buf_msg_ret, NULL);
    return 0;
}

int vpu_test_decode(vpu_config_t *vpu_config)
{
    int ret = 0;
    int need_stream = FALSE;

    int disp_count = 0;
    int decoded_count = 0;

    int last_disp_index = -1;
    int clr_index = -1;

    while (TRUE) {
        if (need_stream == TRUE) {
            ret = feed_bit_stream(vpu_config);
            if (ret == 0) {
                VLOG(ERR, "read all input video\n");
                break;
            }

            need_stream = FALSE;
        }

        // Vpu__DecOneFrame
        {
            Vpu__DecOneFrame *dec_one_frame_ret = NULL;
            Vpu__DecOneFrame dec_one_frame = VPU__DEC_ONE_FRAME__INIT;
            Vpu__DecHandle dec_handle_msg = VPU__DEC_HANDLE__INIT;
            Vpu__DecParam dec_param_msg = (Vpu__DecParam)VPU__DEC_PARAM__INIT;

            dec_one_frame.handle = &dec_handle_msg;
            dec_handle_msg.handle = vpu_config->vpu_handle;

            dec_one_frame.dec_param = &dec_param_msg;
            dec_param_msg.clr_disp_flag = vpu_config->clr_disp_flag;
            dec_param_msg.has_clr_disp_flag = TRUE;

            vpu_config->clr_disp_flag = 0;

            // proto VPU_IPC_DEC_ONE_FRAMER msg
            SEND_PROTO_AND_CHECK_RET_SUCCESS(vpu__dec_one_frame, dec_one_frame, dec_one_frame_ret,
                                             VPU_IPC_DEC_ONE_FRAMER, return -1);

            if (!dec_one_frame_ret->has_interrupt_reaseon) {
                VLOG(ERR, "[%s][%d] invailed ret msg \n", __func__, __LINE__);
                vpu__dec_one_frame__free_unpacked(dec_one_frame_ret, NULL);
                return -1;
            }

            if (!(dec_one_frame_ret->interrupt_reaseon & (1 << INT_BIT_PIC_RUN))) {
                VLOG(ERR, "[%s][%d] invailed interrupt reaseon\n", __func__, __LINE__);
                vpu__dec_one_frame__free_unpacked(dec_one_frame_ret, NULL);
                return -1;
            }

            if (!dec_one_frame_ret->dec_output_info ||
                !dec_one_frame_ret->dec_output_info->rc_display ||
                !dec_one_frame_ret->dec_output_info->disp_frame) {
                VLOG(ERR, "[%s][%d] invailed ret msg \n", __func__, __LINE__);
                vpu__dec_one_frame__free_unpacked(dec_one_frame_ret, NULL);
                return -1;
            }

            memcpy(&vpu_config->vpu_proto_info.dec_output_info, dec_one_frame_ret->dec_output_info,
                   sizeof(Vpu__DecOutputInfo));
            memcpy(&vpu_config->vpu_proto_info.dec_rc_display,
                   dec_one_frame_ret->dec_output_info->rc_display, sizeof(Vpu__VpuRect));
            memcpy(&vpu_config->vpu_proto_info.dec_disp_frame,
                   dec_one_frame_ret->dec_output_info->disp_frame, sizeof(Vpu__FrameBuffer));

            if (vpu_config->vpu_proto_info.dec_open_param.bitstream_format == STD_AVC) {
                if (dec_one_frame_ret->dec_output_info->dec_codec_info_case !=
                        VPU__DEC_OUTPUT_INFO__DEC_CODEC_INFO_AVC_INFO ||
                    !dec_one_frame_ret->dec_output_info->avc_info ||
                    !dec_one_frame_ret->dec_output_info->avc_info->avc_vui_info) {
                    VLOG(ERR, "[%s][%d] dec get output info invailed avc ret msg\n", __func__,
                         __LINE__);
                    vpu__dec_one_frame__free_unpacked(dec_one_frame_ret, NULL);
                    return -1;
                }

                memcpy(&vpu_config->vpu_proto_info.dec_avc_vui_info,
                       dec_one_frame_ret->dec_output_info->avc_info->avc_vui_info,
                       sizeof(Vpu__AvcVuiInfo));
            } else if (vpu_config->vpu_proto_info.dec_open_param.bitstream_format == STD_HEVC) {
                if (dec_one_frame_ret->dec_output_info->dec_codec_info_case !=
                        VPU__DEC_OUTPUT_INFO__DEC_CODEC_INFO_HEVC_INFO ||
                    !dec_one_frame_ret->dec_output_info->hevc_info ||
                    !dec_one_frame_ret->dec_output_info->hevc_info->h265_info) {
                    VLOG(ERR, "[%s][%d] dec get output info invailed hevc ret msg\n", __func__,
                         __LINE__);
                    vpu__dec_one_frame__free_unpacked(dec_one_frame_ret, NULL);
                    return -1;
                }

                memcpy(&vpu_config->vpu_proto_info.dec_hevc_h265_info,
                       dec_one_frame_ret->dec_output_info->hevc_info->h265_info,
                       sizeof(Vpu__DecHevcInfo__H265Info));
            }

            vpu__dec_one_frame__free_unpacked(dec_one_frame_ret, NULL);
        }

        if (vpu_config->vpu_proto_info.dec_output_info.decoding_success) {
            if (vpu_config->vpu_proto_info.dec_output_info.pic_type == PIC_TYPE_I ||
                vpu_config->vpu_proto_info.dec_output_info.pic_type == PIC_TYPE_P ||
                vpu_config->vpu_proto_info.dec_output_info.pic_type == PIC_TYPE_B ||
                vpu_config->vpu_proto_info.dec_output_info.pic_type == PIC_TYPE_IDR) {
                VLOG(TRACE, "decoding success\n");
            }
        } else {
            VLOG(ERR, "dec failed, error:0x%x, error_ext:0x%x\n",
                 vpu_config->vpu_proto_info.dec_output_info.error_reason,
                 vpu_config->vpu_proto_info.dec_output_info.error_reason_ext);
            return -1;
        }

        VLOG(ERR,
             "disIdx=%d, decIdx=%d, prescanIdx=%d, picType=%d,"
             "reuseReuired:%d, dispFlag=0x%x,RdPtr : 0x%x, WrPtr : 0x%x,"
             "display left=%d, top=%d, right=%d, bottom=%d, %dx%d\n",
             (int)vpu_config->vpu_proto_info.dec_output_info.index_frame_display,
             (int)vpu_config->vpu_proto_info.dec_output_info.index_frame_decoded,
             (int)vpu_config->vpu_proto_info.dec_output_info.index_frame_prescan,
             (int)vpu_config->vpu_proto_info.dec_output_info.pic_type,
             (int)vpu_config->vpu_proto_info.dec_output_info.chunk_reuse_required,
             (int)vpu_config->vpu_proto_info.dec_output_info.frame_display_flag,
             vpu_config->vpu_proto_info.dec_output_info.rd_ptr,
             vpu_config->vpu_proto_info.dec_output_info.wr_ptr,
             (int)vpu_config->vpu_proto_info.dec_rc_display.left,
             (int)vpu_config->vpu_proto_info.dec_rc_display.top,
             (int)vpu_config->vpu_proto_info.dec_rc_display.right,
             (int)vpu_config->vpu_proto_info.dec_rc_display.bottom,
             (int)vpu_config->vpu_proto_info.dec_output_info.disp_pic_width,
             (int)vpu_config->vpu_proto_info.dec_output_info.disp_pic_height);

        if (vpu_config->vpu_proto_info.dec_output_info.sequence_changed == TRUE) {
            // TODO: support sequence changed
            VLOG(INFO, "---> SEQUENCE CHANGED <---\n");
        }

        if (vpu_config->vpu_proto_info.dec_output_info.index_frame_decoded >= 0)
            decoded_count++;

        if (vpu_config->vpu_proto_info.dec_output_info.index_frame_display >= 0) {
            // sync display & clear display flag
            int size;

            size = vpu_config->vpu_proto_info.dec_fb_alloc_info_dec.stride *
                   vpu_config->vpu_proto_info.dec_fb_alloc_info_dec.height * 3 / 2;
            VLOG(INFO,
                 "disaplay frame info:: display flag=%d bufY=0x%x bufCb=0x%x, bufCr=0x%x, "
                 "size:%d decoded_count:%d disp_count:%d\n",
                 vpu_config->vpu_proto_info.dec_disp_frame.my_index,
                 vpu_config->vpu_proto_info.dec_disp_frame.buf_y,
                 vpu_config->vpu_proto_info.dec_disp_frame.buf_cb,
                 vpu_config->vpu_proto_info.dec_disp_frame.buf_cr, size, decoded_count, disp_count);
            vpu_display_to_screen(vpu_config, size);

            if (vpu_config->core_idx == VPU_WAVE412_IDX) {
                clr_index = last_disp_index;
            } else {
                clr_index = vpu_config->vpu_proto_info.dec_disp_frame.my_index;
            }

            last_disp_index = vpu_config->vpu_proto_info.dec_disp_frame.my_index;

            if (clr_index >= 0)
                vpu_config->clr_disp_flag |= 1 << clr_index;

            disp_count++;
        }

        if (vpu_config->vpu_proto_info.dec_output_info.index_frame_display ==
            DISPLAY_IDX_FLAG_SEQ_END) {
            VLOG(INFO,
                 "vpu_config->vpu_proto_info.dec_output_info.index_frame_display is seq end\n");
            break;
        }

        if (vpu_config->vpu_proto_info.dec_output_info.index_frame_decoded ==
            DECODED_IDX_FLAG_NO_FB) {
            need_stream = FALSE;
        } else {
            need_stream = TRUE;
        }

        if (vpu_config->vpu_proto_info.dec_output_info.chunk_reuse_required == TRUE) {
            need_stream = FALSE;
        }
    }

    // VLOG(INFO,
    //      "\nDec core_idx:%d End. Tot Frame %d, decNum %d, time consumed %u ms(%.2f sec), "
    //      "%.2f/%.2f\n",
    //      vpu_config->core_idx, disp_count, decoded_count, total_ts / 1000.0,
    //      total_ts / 1000.0 / 1000.0, (double)disp_count / (total_ts / 1000.0 / 1000.0),
    //      (double)decoded_count / (total_ts / 1000.0 / 1000.0));

    return 0;
}

int vpu_test_decoder_h264(void)
{
    int ret = 0;
    int msg_len = 0;

    vpu_config_t *vpu_config = osal_malloc(sizeof(vpu_config_t));

    VLOG(ERR, "%s: %d\n", __FUNCTION__, __LINE__);
    osal_memset(vpu_config, 0x00, sizeof(vpu_config_t));

    {
        vpu_config->core_idx = VPU_CODA988_IDX;
        vpu_config->map_type = LINEAR_FRAME_MAP;
        vpu_config->msg_buf = osal_malloc(VPU_PROTO_BUFFER_SIZE);

        // vbstream
        vpu_config->vb_stream.size = VPU_STREAM_BUF_SIZE;
        if (vdi_allocate_dma_memory(vpu_config->core_idx, &vpu_config->vb_stream) < 0) {
            VLOG(ERR, "fail to allocate bitstream buffer\n");
            ret = -1;
            goto ERR_DEC_INIT;
        }

        VLOG(INFO, "[Main] vbStream base 0x%llx, phy 0x%llx, virt 0x%llx\n",
             vpu_config->vb_stream.base, vpu_config->vb_stream.phys_addr,
             vpu_config->vb_stream.virt_addr);

        // load test h264 file resource
        vpu_config->dec_resource.resource.resource_path = VPU_TEST_H264_RESOURCE_PATH;
        vpu_config->dec_resource.resource.resource_size =
            res_size(vpu_config->dec_resource.resource.resource_path);
        vpu_config->dec_resource.resource.resource_buffer =
            memalign(1024, vpu_config->dec_resource.resource.resource_size);

        ret = res_load(vpu_config->dec_resource.resource.resource_path,
                       vpu_config->dec_resource.resource.resource_buffer,
                       ROUNDUP(vpu_config->dec_resource.resource.resource_size, 1024), 0);
        if (ret < 0) {
            VLOG(ERR, "[%s][%d] res_load failed\n", __func__, __LINE__);
            goto ERR_DEC_INIT;
        }

        // load test h264 resource info file
        vpu_config->dec_resource_info.resource.resource_path = VPU_TEST_H264_RESOURCE_INFO_PATH;
        vpu_config->dec_resource_info.resource.resource_size =
            res_size(vpu_config->dec_resource_info.resource.resource_path);
        vpu_config->dec_resource_info.resource.resource_buffer =
            memalign(1024, vpu_config->dec_resource_info.resource.resource_size);

        ret = res_load(vpu_config->dec_resource_info.resource.resource_path,
                       vpu_config->dec_resource_info.resource.resource_buffer,
                       ROUNDUP(vpu_config->dec_resource_info.resource.resource_size, 1024), 0);
        if (ret < 0) {
            VLOG(ERR, "[%s][%d] res_load failed\n", __func__, __LINE__);
            goto ERR_DEC_INIT;
        }
    }

    // VPU_DecOpen //set up decoder configurations
    {
        Vpu__DecOpen *dec_open_msg_ret = NULL;
        Vpu__DecOpen dec_open_msg = VPU__DEC_OPEN__INIT;
        Vpu__DecHandle dec_handle_msg = VPU__DEC_HANDLE__INIT;

        Vpu__DecOpenParam *dec_op_msg = &vpu_config->vpu_proto_info.dec_open_param;
        *dec_op_msg = (Vpu__DecOpenParam)VPU__DEC_OPEN_PARAM__INIT;

        dec_open_msg.handle = &dec_handle_msg;
        dec_open_msg.dec_op = dec_op_msg;

        // dec open param
        dec_op_msg->bitstream_format = STD_AVC;
        dec_op_msg->avc_extension = 0;
        dec_op_msg->core_idx = VPU_CODA988_IDX;
        dec_op_msg->bitstream_buffer = vpu_config->vb_stream.phys_addr;
        dec_op_msg->bitstream_buffer_size = vpu_config->vb_stream.size;
        dec_op_msg->bitstream_mode = BS_MODE_PIC_END;
        dec_op_msg->tiled2_linear_enable = 0;
        dec_op_msg->tiled2_linear_mode = FF_FRAME;
        dec_op_msg->wtl_enable = 0;
        dec_op_msg->wtl_mode = FF_FRAME;
        dec_op_msg->cbcr_interleave = 0; // yuv420
        dec_op_msg->nv21 = 0;
        dec_op_msg->stream_endian = VDI_LITTLE_ENDIAN;
        dec_op_msg->frame_endian = VDI_LITTLE_ENDIAN;
        dec_op_msg->bwb_enable = VPU_ENABLE_BWB;
        dec_op_msg->mp4_deblk_enable = 0;
        dec_op_msg->mp4_class =
            8; //(8 means that firmware try to find mp4class by searching user-data)
        dec_op_msg->sram_enable = TRUE;

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
        dec_op_msg->has_sram_enable = TRUE;

        // proto VPU_IPC_DEC_OPEN msg
        SEND_PROTO_AND_CHECK_RET_SUCCESS(vpu__dec_open, dec_open_msg, dec_open_msg_ret,
                                         VPU_IPC_DEC_OPEN, goto ERR_DEC_INIT);

        vpu_config->vpu_handle = dec_open_msg_ret->handle->handle;
        vpu__dec_open__free_unpacked(dec_open_msg_ret, NULL);

        VLOG(ERR, "VPU_DecOpen success, vpu handle:0x%x\n", vpu_config->vpu_handle);
    }

    VLOG(ERR,
         "%s: %d sizeof(vpu_config_t):%d sizeof(DecOpenParam):%d sizeof(_Vpu__DecOutputInfo):%d\n",
         __FUNCTION__, __LINE__, sizeof(vpu_config_t),
         sizeof(vpu_config->vpu_proto_info.dec_open_param), sizeof(struct _Vpu__DecOutputInfo));

    feed_bit_stream(vpu_config);

    ret = vpu_test_seq_init(vpu_config);
    if (ret < 0) {
        VLOG(ERR, "[%s][%d] vpu_test_seq_init failed\n", __func__, __LINE__);
        goto DECODE_CLOSE;
    }

    ret = vpu_test_register_frame_buffer(vpu_config);
    if (ret < 0) {
        VLOG(ERR, "[%s][%d] vpu_test_register_frame_buffer failed\n", __func__, __LINE__);
        goto DECODE_CLOSE;
    }

    // VPU_IPC_DEC_ONE_FRAMER
    ret = vpu_test_decode(vpu_config);
    if (ret < 0) {
        VLOG(ERR, "[%s][%d] vpu_test_decode failed\n", __func__, __LINE__);
        goto DECODE_END;
    }

DECODE_END:
    // Vpu__DecPicEndModeUpdateBSBuffer
    {
        Vpu__DecPicEndModeUpdateBSBuffer *pic_end_mode_update_bs_buf_msg_ret = NULL;
        Vpu__DecPicEndModeUpdateBSBuffer pic_end_mode_update_bs_buf_msg =
            VPU__DEC_PIC_END_MODE_UPDATE_BSBUFFER__INIT;
        Vpu__DecHandle dec_handle_msg = VPU__DEC_HANDLE__INIT;

        pic_end_mode_update_bs_buf_msg.handle = &dec_handle_msg;
        dec_handle_msg.handle = vpu_config->vpu_handle;

        pic_end_mode_update_bs_buf_msg.has_addr = TRUE;
        pic_end_mode_update_bs_buf_msg.addr = vpu_config->vb_stream.phys_addr;

        pic_end_mode_update_bs_buf_msg.has_size = TRUE;
        pic_end_mode_update_bs_buf_msg.size = STREAM_END_SIZE;

        // proto VPU_IPC_DEC_PIC_END_MODE_UPDATE_BS_BUFFER msg
        SEND_PROTO_AND_CHECK_RET_SUCCESS(
            vpu__dec_pic_end_mode_update_bsbuffer, pic_end_mode_update_bs_buf_msg,
            pic_end_mode_update_bs_buf_msg_ret, VPU_IPC_DEC_PIC_END_MODE_UPDATE_BS_BUFFER, break);
        vpu__dec_pic_end_mode_update_bsbuffer__free_unpacked(pic_end_mode_update_bs_buf_msg_ret,
                                                             NULL);
    }

DECODE_CLOSE:
    // VPU_DecClose
    {
        Vpu__DecClose dec_close_msg = VPU__DEC_CLOSE__INIT;
        Vpu__DecHandle dec_handle_msg = VPU__DEC_HANDLE__INIT;

        dec_close_msg.handle = &dec_handle_msg;
        dec_handle_msg.handle = vpu_config->vpu_handle;

        // proto msg
        msg_len = vpu__dec_close__get_packed_size(&dec_close_msg);
        vpu__dec_close__pack(&dec_close_msg, vpu_config->msg_buf);

        ret = vpu_send_msg_to_service(&vpu_config->pack, VPU_IPC_DEC_CLOSE, vpu_config->msg_buf,
                                      msg_len);
        if (ret < 0) {
            VLOG(ERR, "VPU_DecClose failed\n");
        }
    }

    for (int i = 0; i < MAX_REG_FRAME; i++) {
        if (vpu_config->dec_vb_frame_dec[i].size > 0)
            vdi_free_dma_memory(vpu_config->core_idx, &vpu_config->dec_vb_frame_dec[i]);

        if (vpu_config->dec_vb_frame_disp[i].size > 0)
            vdi_free_dma_memory(vpu_config->core_idx, &vpu_config->dec_vb_frame_disp[i]);
    }

ERR_DEC_INIT:
    if (vpu_config->vb_stream.size > 0) {
        vdi_free_dma_memory(vpu_config->core_idx, &vpu_config->vb_stream);
    }

    if (vpu_config->vb_stream_wave_more.size > 0) {
        vdi_free_dma_memory(vpu_config->core_idx, &vpu_config->vb_stream_wave_more);
    }

    if (vpu_config->dec_resource.resource.resource_buffer)
        free(vpu_config->dec_resource.resource.resource_buffer);

    if (vpu_config->dec_resource_info.resource.resource_buffer)
        free(vpu_config->dec_resource_info.resource.resource_buffer);

    if (vpu_config->display)
        sdm_clear_display(vpu_config->display->handle);

    if (vpu_config->msg_buf)
        free(vpu_config->msg_buf);

    if (vpu_config)
        free(vpu_config);

    return ret;
}

int vpu_test_decoder_h265(void)
{
    int ret = 0;
    int msg_len = 0;

    vpu_config_t *vpu_config = osal_malloc(sizeof(vpu_config_t));

    VLOG(ERR, "%s: %d\n", __FUNCTION__, __LINE__);
    osal_memset(vpu_config, 0x00, sizeof(vpu_config_t));

    {
        vpu_config->core_idx = VPU_WAVE412_IDX;
        vpu_config->map_type = COMPRESSED_FRAME_MAP; // dec map type
        vpu_config->msg_buf = osal_malloc(VPU_PROTO_BUFFER_SIZE);

        // vbstream
        vpu_config->vb_stream.size = VPU_STREAM_BUF_SIZE;
        if (vdi_allocate_dma_memory(vpu_config->core_idx, &vpu_config->vb_stream) < 0) {
            VLOG(ERR, "fail to allocate bitstream buffer\n");
            ret = -1;
            goto ERR_DEC_INIT;
        }

        VLOG(INFO, "[Main] vbStream base 0x%llx, phy 0x%llx, virt 0x%llx\n",
             vpu_config->vb_stream.base, vpu_config->vb_stream.phys_addr,
             vpu_config->vb_stream.virt_addr);

        // vb stream wave more
        vpu_config->vb_stream_wave_more.size = VPU_STREAM_BUF_SIZE;
        if (vdi_allocate_dma_memory(vpu_config->core_idx, &vpu_config->vb_stream_wave_more) < 0) {
            VLOG(ERR, "fail to allocate bitstream buffer\n");
            ret = -1;
            goto ERR_DEC_INIT;
        }

        VLOG(INFO, "[Main] vb_stream_wave_more base 0x%llx, phy 0x%llx, virt 0x%llx\n",
             vpu_config->vb_stream_wave_more.base, vpu_config->vb_stream_wave_more.phys_addr,
             vpu_config->vb_stream_wave_more.virt_addr);

        // load test h264 file resource
        vpu_config->dec_resource.resource.resource_path = VPU_TEST_H265_RESOURCE_PATH;
        vpu_config->dec_resource.resource.resource_size =
            res_size(vpu_config->dec_resource.resource.resource_path);
        vpu_config->dec_resource.resource.resource_buffer =
            memalign(1024, vpu_config->dec_resource.resource.resource_size);

        ret = res_load(vpu_config->dec_resource.resource.resource_path,
                       vpu_config->dec_resource.resource.resource_buffer,
                       ROUNDUP(vpu_config->dec_resource.resource.resource_size, 1024), 0);
        if (ret < 0) {
            VLOG(ERR, "[%s][%d] res_load failed\n", __func__, __LINE__);
            goto ERR_DEC_INIT;
        }

        // load test h264 resource info file
        vpu_config->dec_resource_info.resource.resource_path = VPU_TEST_H265_RESOURCE_INFO_PATH;
        vpu_config->dec_resource_info.resource.resource_size =
            res_size(vpu_config->dec_resource_info.resource.resource_path);
        vpu_config->dec_resource_info.resource.resource_buffer =
            memalign(1024, vpu_config->dec_resource_info.resource.resource_size);

        ret = res_load(vpu_config->dec_resource_info.resource.resource_path,
                       vpu_config->dec_resource_info.resource.resource_buffer,
                       ROUNDUP(vpu_config->dec_resource_info.resource.resource_size, 1024), 0);
        if (ret < 0) {
            VLOG(ERR, "[%s][%d] res_load failed\n", __func__, __LINE__);
            goto ERR_DEC_INIT;
        }
    }

    // VPU_DecOpen //set up decoder configurations
    {
        Vpu__DecOpen *dec_open_msg_ret = NULL;
        Vpu__DecOpen dec_open_msg = VPU__DEC_OPEN__INIT;
        Vpu__DecHandle dec_handle_msg = VPU__DEC_HANDLE__INIT;

        Vpu__DecOpenParam *dec_op_msg = &vpu_config->vpu_proto_info.dec_open_param;
        *dec_op_msg = (Vpu__DecOpenParam)VPU__DEC_OPEN_PARAM__INIT;

        dec_open_msg.handle = &dec_handle_msg;
        dec_open_msg.dec_op = dec_op_msg;

        // dec open param
        dec_op_msg->bitstream_format = STD_HEVC;
        dec_op_msg->core_idx = VPU_WAVE412_IDX;
        dec_op_msg->bitstream_buffer = vpu_config->vb_stream.phys_addr;
        dec_op_msg->bitstream_buffer_size = vpu_config->vb_stream.size;
        dec_op_msg->bitstream_mode = BS_MODE_PIC_END;
        dec_op_msg->wtl_enable = TRUE;
        dec_op_msg->pvric_fbc_enable = 0;
        dec_op_msg->pvric31_hw_support = 0;
        dec_op_msg->wtl_mode = FF_FRAME;
        dec_op_msg->cbcr_interleave = 0;
        dec_op_msg->nv21 = 0;
        dec_op_msg->stream_endian = VDI_128BIT_LITTLE_ENDIAN;
        dec_op_msg->frame_endian = VDI_128BIT_LITTLE_ENDIAN;
        dec_op_msg->fbc_mode = 0x0C;     // normal: 0x0C - Best prediction: 0x00 - Basic prediction:
                                         // 0x3C
        dec_op_msg->bw_optimization = 0; // feature only for wave

        dec_op_msg->has_bitstream_format = TRUE;
        dec_op_msg->has_core_idx = TRUE;
        dec_op_msg->has_bitstream_buffer = TRUE;
        dec_op_msg->has_bitstream_buffer_size = TRUE;
        dec_op_msg->has_bitstream_mode = TRUE;
        dec_op_msg->has_wtl_enable = TRUE;
        dec_op_msg->has_pvric_fbc_enable = TRUE;
        dec_op_msg->has_pvric31_hw_support = TRUE;
        dec_op_msg->has_wtl_mode = TRUE;
        dec_op_msg->has_cbcr_interleave = TRUE;
        dec_op_msg->has_nv21 = TRUE;
        dec_op_msg->has_stream_endian = TRUE;
        dec_op_msg->has_frame_endian = TRUE;
        dec_op_msg->has_fbc_mode = TRUE;
        dec_op_msg->has_bw_optimization = TRUE;

        // proto VPU_IPC_DEC_OPEN msg
        SEND_PROTO_AND_CHECK_RET_SUCCESS(vpu__dec_open, dec_open_msg, dec_open_msg_ret,
                                         VPU_IPC_DEC_OPEN, goto ERR_DEC_INIT);

        vpu_config->vpu_handle = dec_open_msg_ret->handle->handle;
        vpu__dec_open__free_unpacked(dec_open_msg_ret, NULL);

        VLOG(ERR, "VPU_DecOpen success, vpu handle:0x%x\n", vpu_config->vpu_handle);
    }

    // TODO: add thumbnail mode process

    feed_bit_stream(vpu_config);

    // seq init
    ret = vpu_test_seq_init(vpu_config);
    if (ret < 0) {
        VLOG(ERR, "[%s][%d] vpu_test_seq_init failed\n", __func__, __LINE__);
        goto DECODE_CLOSE;
    }

    // frame buffer
    ret = vpu_test_register_frame_buffer(vpu_config);
    if (ret < 0) {
        VLOG(ERR, "[%s][%d] vpu_test_register_frame_buffer failed\n", __func__, __LINE__);
        goto DECODE_CLOSE;
    }

    // decode
    ret = vpu_test_decode(vpu_config);
    if (ret < 0) {
        VLOG(ERR, "[%s][%d] vpu_test_seq_init vpu_test_register_frame_buffer\n", __func__,
             __LINE__);
        goto DECODE_END;
    }

DECODE_END:
    // Vpu__DecPicEndModeUpdateBSBuffer
    {
        Vpu__DecPicEndModeUpdateBSBuffer *pic_end_mode_update_bs_buf_msg_ret = NULL;
        Vpu__DecPicEndModeUpdateBSBuffer pic_end_mode_update_bs_buf_msg =
            VPU__DEC_PIC_END_MODE_UPDATE_BSBUFFER__INIT;
        Vpu__DecHandle dec_handle_msg = VPU__DEC_HANDLE__INIT;

        pic_end_mode_update_bs_buf_msg.handle = &dec_handle_msg;
        dec_handle_msg.handle = vpu_config->vpu_handle;

        pic_end_mode_update_bs_buf_msg.has_addr = TRUE;
        pic_end_mode_update_bs_buf_msg.addr = vpu_config->vb_stream.phys_addr;

        pic_end_mode_update_bs_buf_msg.has_size = TRUE;
        pic_end_mode_update_bs_buf_msg.size = STREAM_END_SIZE;

        // proto VPU_IPC_DEC_PIC_END_MODE_UPDATE_BS_BUFFER msg
        SEND_PROTO_AND_CHECK_RET_SUCCESS(
            vpu__dec_pic_end_mode_update_bsbuffer, pic_end_mode_update_bs_buf_msg,
            pic_end_mode_update_bs_buf_msg_ret, VPU_IPC_DEC_PIC_END_MODE_UPDATE_BS_BUFFER, break);
        vpu__dec_pic_end_mode_update_bsbuffer__free_unpacked(pic_end_mode_update_bs_buf_msg_ret,
                                                             NULL);
    }

DECODE_CLOSE:
    // VPU_DecClose
    {
        Vpu__DecClose dec_close_msg = VPU__DEC_CLOSE__INIT;
        Vpu__DecHandle dec_handle_msg = VPU__DEC_HANDLE__INIT;

        dec_close_msg.handle = &dec_handle_msg;
        dec_handle_msg.handle = vpu_config->vpu_handle;

        // proto msg
        msg_len = vpu__dec_close__get_packed_size(&dec_close_msg);
        vpu__dec_close__pack(&dec_close_msg, vpu_config->msg_buf);

        ret = vpu_send_msg_to_service(&vpu_config->pack, VPU_IPC_DEC_CLOSE, vpu_config->msg_buf,
                                      msg_len);
        if (ret < 0) {
            VLOG(ERR, "VPU_DecClose failed\n");
        }
    }

    for (int i = 0; i < MAX_REG_FRAME; i++) {
        if (vpu_config->dec_vb_frame_dec[i].size > 0)
            vdi_free_dma_memory(vpu_config->core_idx, &vpu_config->dec_vb_frame_dec[i]);

        if (vpu_config->dec_vb_frame_disp[i].size > 0)
            vdi_free_dma_memory(vpu_config->core_idx, &vpu_config->dec_vb_frame_disp[i]);
    }

ERR_DEC_INIT:
    if (vpu_config->vb_stream.size > 0) {
        vdi_free_dma_memory(vpu_config->core_idx, &vpu_config->vb_stream);
    }

    if (vpu_config->vb_stream_wave_more.size > 0) {
        vdi_free_dma_memory(vpu_config->core_idx, &vpu_config->vb_stream_wave_more);
    }

    if (vpu_config->dec_resource.resource.resource_buffer)
        free(vpu_config->dec_resource.resource.resource_buffer);

    if (vpu_config->dec_resource_info.resource.resource_buffer)
        free(vpu_config->dec_resource_info.resource.resource_buffer);

    if (vpu_config->display)
        sdm_clear_display(vpu_config->display->handle);

    if (vpu_config->msg_buf)
        free(vpu_config->msg_buf);

    if (vpu_config)
        free(vpu_config);

    return ret;
}

/********************************enc********************************/

int ppu_fb_alloc_info_to_fb(uint32_t handle, Vpu__FrameBufferAllocInfo *info, Vpu__FrameBuffer *fb, int fb_codec_num)
{
    if (!info || !fb || info->num <= 0 || info->stride <= 0 || info->height <= 0 ||
        info->type != FB_TYPE_PPU) {
        VLOG(ERR, "[%s][%d] invailed param\n", __func__, __LINE__);
        return -1;
    }

    if (info->map_type != LINEAR_FRAME_MAP || info->format != FORMAT_420 ||
        info->cbcr_interleave != FALSE) {
        VLOG(ERR, "[%s][%d] TODO: support more format, map_type:%d, format:%d, cbcr:%d\n",
             info->map_type, info->format, info->cbcr_interleave);
        return -1;
    }

    uint32_t size_luma;
    uint32_t size_chroma;

    size_luma = CalcLumaSize((CodecInst *)handle, PRODUCT_ID_980, info->stride, info->height, info->format,
                             info->cbcr_interleave, info->map_type, NULL);
    size_chroma = CalcChromaSize((CodecInst *)handle, PRODUCT_ID_980, info->stride, info->height, info->format,
                                 info->cbcr_interleave, info->map_type, NULL);

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
            fb[i].size_luma = size_luma;
            fb[i].size_chroma = size_chroma;
            fb[i].source_l_burst_en = FALSE;
            fb[i].buf_cb = fb[i].buf_y + size_luma;
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

int write_frame_data_to_src_buf(vpu_enc_config_t *vpu_config, vpu_buffer_t *src_buf,
                                Vpu__FrameBuffer *fb_src)
{
    int n_y, n_cb, n_cr;
    int luma_size, luma_stride, chroma_size, chroma_stride, chroma_width;

    int ret;
    uint32_t p_dst = src_buf->phys_addr;
    void *p_src =
        vpu_config->enc_resource.resource.resource_buffer + vpu_config->enc_resource.first_offset;
    int pic_width = vpu_config->enc_open_param.pic_width;
    int pic_height = vpu_config->enc_open_param.pic_height;
    uint32_t frame_size = pic_width * pic_height * 3 / 2;

    if (vpu_config->enc_resource.first_offset >=
        (unsigned int)vpu_config->enc_resource.resource.resource_size) {
        VLOG(ERR, "[%s][%d] read all input buffer\n", __func__, __LINE__);
        return 0;
    }

    if (frame_size == src_buf->size) {
        ret = vdi_write_memory(vpu_config->core_idx, p_dst, p_src, frame_size,
                               vpu_config->enc_open_param.stream_endian);
        if (ret < 0) {
            VLOG(ERR, "[%s][%d] vdi_write_memory error, ret:%d\n", __func__, __LINE__, ret);
            return ret;
        }
    } else {
        n_y = pic_height;
        luma_stride = fb_src->stride;
        luma_size = pic_width * pic_height;

        n_cb = n_cr = n_y / 2;
        chroma_size = luma_size / 4;
        chroma_stride = luma_stride / 2;
        chroma_width = pic_width / 2;

        // y
        for (int i = 0; i < n_y; i++) {
            ret = vdi_write_memory(vpu_config->core_idx, p_dst + luma_stride * i,
                                   p_src + pic_width * i, pic_width,
                                   vpu_config->enc_open_param.stream_endian);
            if (ret < 0) {
                VLOG(ERR, "[%s][%d] vdi_write_memory error, ret:%d\n", __func__, __LINE__, ret);
                return ret;
            }
        }

        p_src += luma_size;
        p_dst = fb_src->buf_cb;

        // u
        for (int i = 0; i < n_cb; i++) {
            ret = vdi_write_memory(vpu_config->core_idx, p_dst + chroma_stride * i,
                                   p_src + chroma_width * i, chroma_width,
                                   vpu_config->enc_open_param.stream_endian);
            if (ret < 0) {
                VLOG(ERR, "[%s][%d] vdi_write_memory error, ret:%d\n", __func__, __LINE__, ret);
                return ret;
            }
        }

        p_src += chroma_size;
        p_dst = fb_src->buf_cr;

        // v
        for (int i = 0; i < n_cr; i++) {
            ret = vdi_write_memory(vpu_config->core_idx, p_dst + chroma_stride * i,
                                   p_src + chroma_width * i, chroma_width,
                                   vpu_config->enc_open_param.stream_endian);
            if (ret < 0) {
                VLOG(ERR, "[%s][%d] vdi_write_memory error, ret:%d\n", __func__, __LINE__, ret);
                return ret;
            }
        }
    }

    vpu_config->enc_resource.first_offset += frame_size;

    return frame_size;
}

int disp_enc_input_buf(vpu_enc_config_t *vpu_config, Vpu__FrameBuffer *fb_src)
{
    int n_bufs = 1;
    struct sdm_buffer sdmbuf[1] = {0};
    struct sdm_post_config postConfig = {0};

    if (!vpu_config) {
        VLOG(ERR, "Invalid parameter\n");
        return -1;
    }

    if (!vpu_config->display) {
        vpu_config->display = vpu_get_screen_display();
    }

    postConfig.bufs = sdmbuf;
    postConfig.n_bufs = n_bufs;

    for (int i = 0; i < n_bufs; i++) {
        struct sdm_buffer *buf = &postConfig.bufs[i];

        buf->addr[0] = (unsigned long)ap2p((paddr_t)fb_src->buf_y);
        buf->addr[1] = (unsigned long)ap2p((paddr_t)fb_src->buf_cb);
        buf->addr[2] = (unsigned long)ap2p((paddr_t)fb_src->buf_cr);

        buf->layer = 1;
        buf->layer_dirty = 1;
        buf->alpha = 0xff;
        buf->alpha_en = 1;
        buf->ckey = 0;
        buf->ckey_en = 0;
        buf->fmt = COLOR_YUV420P;
        buf->layer_en = 1;
        buf->src.x = 0;
        buf->src.y = 0;
        buf->src.w = fb_src->stride;
        buf->src.h = fb_src->height;

        buf->start.x = 0;
        buf->start.y = 0;
        buf->start.w = vpu_config->enc_open_param.pic_width;
        buf->start.h = vpu_config->enc_open_param.pic_height;

        // src_stride
        buf->src_stride[0] = fb_src->stride;
        buf->src_stride[1] = fb_src->stride / 2;
        buf->src_stride[2] = fb_src->stride / 2;

        // 1920x720 is screen size
        buf->dst.x = 0;
        buf->dst.y = 0;
        buf->dst.w = 1920;
        buf->dst.h = 720;
        buf->z_order = 0;
    }

    if (vpu_config->display)
        return sdm_post(vpu_config->display->handle, &postConfig);

    return -1;
}

int vpu_test_encoder_h264(void)
{
    int ret = 0;
    int msg_len = 0;
    int frame_idx = 0;

    vpu_enc_config_t *vpu_config = osal_malloc(sizeof(vpu_enc_config_t));

    VLOG(ERR, "%s: %d\n", __FUNCTION__, __LINE__);
    osal_memset(vpu_config, 0x00, sizeof(vpu_enc_config_t));

    {
        vpu_config->core_idx = VPU_CODA988_IDX;
        vpu_config->map_type = LINEAR_FRAME_MAP;
        vpu_config->msg_buf = osal_malloc(VPU_PROTO_BUFFER_SIZE);

        // vbstream
        vpu_config->vb_stream.size = VPU_STREAM_BUF_SIZE;
        if (vdi_allocate_dma_memory(vpu_config->core_idx, &vpu_config->vb_stream) < 0) {
            VLOG(ERR, "fail to allocate bitstream buffer\n");
            ret = -1;
            goto ERR_ENC_INIT;
        }

        VLOG(INFO, "[Main] vbStream base 0x%llx, phy 0x%llx, virt 0x%llx\n",
             vpu_config->vb_stream.base, vpu_config->vb_stream.phys_addr,
             vpu_config->vb_stream.virt_addr);

        // load test enc h264 file resource
        vpu_config->enc_resource.resource.resource_path = VPU_TEST_ENC_H264_RESOURCE_PATH;
        vpu_config->enc_resource.resource.resource_size =
            res_size(vpu_config->enc_resource.resource.resource_path);
        vpu_config->enc_resource.resource.resource_buffer =
            memalign(1024, vpu_config->enc_resource.resource.resource_size);

        ret = res_load(vpu_config->enc_resource.resource.resource_path,
                       vpu_config->enc_resource.resource.resource_buffer,
                       ROUNDUP(vpu_config->enc_resource.resource.resource_size, 1024), 0);
        if (ret < 0) {
            VLOG(ERR, "[%s][%d] res_load failed\n", __func__, __LINE__);
            goto ERR_ENC_INIT;
        }

        // enc output buf
        vpu_config->enc_output_bs_buf.resource.resource_buffer =
            osal_malloc(VPU_ENC_OUTPUT_BS_BUFFER_SIZE);
    }

    // VPU_EncOpen
    /*
    FRAME_NUMBER_ENCODED  300   			                // number of frames to be encoded
    PICTURE_WIDTH         4096   			            // picture width
    PICTURE_HEIGHT        2304  			            // picture height
    FRAME_RATE            30   			                // frame rate
    LEVEL                 42                             // level in 14496-10 Annex A
    CONSTRAINED_INTRA     1   			                // constrained_intra_pred_flag
    INTERLACED_PIC        0
    ;-----------------------
    ; DEBLKING FILTER
    ;-----------------------
    DISABLE_DEBLK         1   			// disable_deblk (0 : enable, 1 : disable, 2 : disable at
    slice boundary) DEBLK_ALPHA           3   			// deblk_filter_offset_alpha (-6 ~ 6)
    DEBLK_BETA            -4   			// deblk_filter_offset_beta  (-6 ~ 6)
    CHROMA_QP_OFFSET      2   			// chroma_qp_offset (-12 ~ 12)
    PIC_QP_Y              10   			// pic_qp_y (0 ~ 51)
    GOP_PIC_NUMBER        0   			// GOP picture number (0 : only first I, 1 : all I, 3 :
    I,P,P,I,)
    ;-----------------------
    ; SLICE STRUCTURE
    ;-----------------------
    SLICE_MODE            1   			// slice mode (0 : one slice, 1 : multiple slice)
    SLICE_SIZE_MODE       0   			// slice size mode (0 : slice bit number, 1 : slice mb
    number) SLICE_SIZE_NUMBER     8000   	    // slice size number (bit count or mb number)
    ;-----------------------
    ; RATE CONTROL
    ;-----------------------
    RATE_CONTROL_ENABLE   1   			// rate control enable
    BIT_RATE_KBPS         40000   			// bit rate in kbps (ignored if rate control disable)
    (64 ~50000) DELAY_IN_MS           20   			// delay in ms (initial decoder buffer delay)
    (20 ~ 1000) VBV_BUFFER_SIZE       0   			// reference decoder buffer size in bits (0 :
    ignore)
    ;-----------------------
    ; ERROR RESILIENCE
    ;-----------------------
    INTRA_MB_REFRESH      1  			// Intra MB Refresh (0 - None, 1 ~ MbNum-1)
    ;-----------------------
    ; ADDITIONAL PARAMETER
    ;-----------------------
    ENTROPY_CODING_MODE     0           // 0 : CAVLC, 1 : CABAC
    TRANSFORM_8X8_MODE      0           // 0 : disable(BP), 1 : enable(HP)
    CHROMA_FORMAT_400       0           // 0 : 420, 1 : 400 (MONO mode allowed only on HP)
    SEARCH_RANGE_X          3           // 3: -16~15, 2:-32~31, 1:-48~47, 0:-64~63, H.263(Short
    Header : always 0) SEARCH_RANGE_Y          2           // 2: -16~15, 1:-32~31, 0:-48~47,
    H.263(Short Header : always 0) ME_USE_ZERO_PMV         0           // 0: PMV_ENABLE, 1:
    PMV_DISABLE WEIGHT_INTRA_COST       0         // Intra weight when compare Intra and Inter
    CABAC_INIT_IDC          0           // cabac init idc ( 0 ~ 2 )
    */
    {
        Vpu__EncOpen *enc_open_msg_ret = NULL;
        Vpu__EncOpen enc_open_msg = VPU__ENC_OPEN__INIT;
        Vpu__EncSliceMode slice_mode_msg = VPU__ENC_SLICE_MODE__INIT;
        Vpu__EncHandle enc_handle_msg = VPU__ENC_HANDLE__INIT;
        Vpu__EncOpenParam *enc_op_msg = &vpu_config->enc_open_param;

        *enc_op_msg = (Vpu__EncOpenParam)VPU__ENC_OPEN_PARAM__INIT;
        enc_open_msg.handle = &enc_handle_msg;
        enc_open_msg.enc_op = enc_op_msg;

        // enc open param
        enc_op_msg->bitstream_format = STD_AVC;
        enc_op_msg->bitstream_buffer = vpu_config->vb_stream.phys_addr;
        enc_op_msg->bitstream_buffer_size = vpu_config->vb_stream.size;
        enc_op_msg->ring_buffer_enable = 0;
        enc_op_msg->cbcr_interleave = 0;
        enc_op_msg->nv21 = 0;
        enc_op_msg->frame_endian = VDI_LITTLE_ENDIAN;
        enc_op_msg->stream_endian = VDI_LITTLE_ENDIAN;
        enc_op_msg->bwb_enable = TRUE;
        enc_op_msg->line_buf_int_en = 0;
        enc_op_msg->core_idx = vpu_config->core_idx;
        enc_op_msg->linear2_tiled_enable = FALSE;
        enc_op_msg->linear2_tiled_mode = FF_NONE;
        enc_op_msg->cbcr_order = CBCR_ORDER_NORMAL;
        enc_op_msg->pic_width = VPU_TEST_ENC_WIDTH;
        enc_op_msg->pic_height = VPU_TEST_ENC_HEIGHT;
        enc_op_msg->frame_rate_info = 30;
        enc_op_msg->bit_rate = 0;
        enc_op_msg->initial_delay = 1000;
        enc_op_msg->vbv_buffer_size = 0;
        enc_op_msg->frame_skip_disable = 1;
        enc_op_msg->me_blk_mode = 0;
        enc_op_msg->gop_size = 0;
        enc_op_msg->idr_interval = 0;
        enc_op_msg->intra_refresh = 0;
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
        enc_op_msg->rc_enable = 0;
        enc_op_msg->user_gamma = -1;
        enc_op_msg->me_use_zero_pmv = 0;
        enc_op_msg->user_qp_max = 31;
        enc_op_msg->rc_interval_mode = 0;
        enc_op_msg->mb_interval = 1;
        enc_op_msg->map_type = vpu_config->map_type;
        enc_op_msg->sram_enable = TRUE;

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
        slice_mode_msg.slice_size = 8000;
        slice_mode_msg.has_slice_mode = TRUE;
        slice_mode_msg.has_slice_size_mode = TRUE;
        slice_mode_msg.has_slice_size = TRUE;

        if (enc_op_msg->bitstream_format == STD_AVC) {
            enc_op_msg->user_max_delta_qp = -1;
            enc_op_msg->user_qp_min = -1;
            enc_op_msg->user_min_delta_qp = -1;
            enc_op_msg->has_user_max_delta_qp = TRUE;
            enc_op_msg->has_user_qp_min = TRUE;
            enc_op_msg->has_user_min_delta_qp = TRUE;

            // avc param
            enc_op_msg->enc_std_param = &vpu_config->enc_std_param;
            *enc_op_msg->enc_std_param = (Vpu__EncStdParam)VPU__ENC_STD_PARAM__INIT;

            enc_op_msg->enc_std_param->param_case = VPU__ENC_STD_PARAM__PARAM_AVC_PARAM;
            enc_op_msg->enc_std_param->avc_param = &vpu_config->enc_avc_param;
            *enc_op_msg->enc_std_param->avc_param =
                (Vpu__EncStdParam__EncAvcParam)VPU__ENC_STD_PARAM__ENC_AVC_PARAM__INIT;

            enc_op_msg->enc_std_param->avc_param->constrained_intra_pred_flag = 0;
            enc_op_msg->enc_std_param->avc_param->disable_deblk = 0;
            enc_op_msg->enc_std_param->avc_param->deblk_filter_offset_alpha = 3;
            enc_op_msg->enc_std_param->avc_param->deblk_filter_offset_beta = -4;
            enc_op_msg->enc_std_param->avc_param->chroma_qp_offset = 2;
            enc_op_msg->enc_std_param->avc_param->aud_enable = 0;
            enc_op_msg->enc_std_param->avc_param->frame_cropping_flag = 0;
            enc_op_msg->enc_std_param->avc_param->frame_crop_left = 0;
            enc_op_msg->enc_std_param->avc_param->frame_crop_right = 0;
            enc_op_msg->enc_std_param->avc_param->frame_crop_top = 0;
            enc_op_msg->enc_std_param->avc_param->frame_crop_bottom = 0;
            enc_op_msg->enc_std_param->avc_param->level = 42;
            enc_op_msg->enc_std_param->avc_param->profile = 0;
            enc_op_msg->enc_std_param->avc_param->field_flag = 0;
            enc_op_msg->enc_std_param->avc_param->field_ref_mode = 1;
            enc_op_msg->enc_std_param->avc_param->chroma_format400 = 0;
            enc_op_msg->enc_std_param->avc_param->pps_num = 1;

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

            enc_op_msg->enc_std_param->avc_param->pps_param_1 = &vpu_config->enc_pps_param_1;
            *enc_op_msg->enc_std_param->avc_param->pps_param_1 =
                (Vpu__AvcPpsParam)VPU__AVC_PPS_PARAM__INIT;

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
            enc_op_msg->enc_std_param->avc_param->pps_param_1->pps_id = 0;
            enc_op_msg->enc_std_param->avc_param->pps_param_1->cabac_init_idc = 0;

            enc_op_msg->enc_std_param->avc_param->pps_param_1->has_pps_id = TRUE;
            enc_op_msg->enc_std_param->avc_param->pps_param_1->has_cabac_init_idc = TRUE;
            enc_op_msg->enc_std_param->avc_param->pps_param_1->has_entropy_coding_mode = TRUE;
            enc_op_msg->enc_std_param->avc_param->pps_param_1->has_transform8x8_mode = TRUE;
        }

        // proto VPU_IPC_ENC_OPEN msg
        SEND_PROTO_AND_CHECK_RET_SUCCESS(vpu__enc_open, enc_open_msg, enc_open_msg_ret,
                                         VPU_IPC_ENC_OPEN, goto ERR_ENC_INIT);

        vpu_config->vpu_handle = enc_open_msg_ret->handle->handle;
        vpu__enc_open__free_unpacked(enc_open_msg_ret, NULL);

        VLOG(ERR, "VPU_EncOpen success, vpu handle:0x%x\n", vpu_config->vpu_handle);
    }

    // VPU_EncGetInitialInfo
    {
        Vpu__EncGetInitialInfo *get_init_info_msg_ret = NULL;
        Vpu__EncGetInitialInfo get_init_info_msg =
            (Vpu__EncGetInitialInfo)VPU__ENC_GET_INITIAL_INFO__INIT;
        Vpu__EncHandle enc_handle_msg = VPU__ENC_HANDLE__INIT;

        get_init_info_msg.handle = &enc_handle_msg;
        enc_handle_msg.handle = vpu_config->vpu_handle;

        // proto VPU_IPC_ENC_GET_INITIAL_INFO
        SEND_PROTO_AND_CHECK_RET_SUCCESS(vpu__enc_get_initial_info, get_init_info_msg,
                                         get_init_info_msg_ret, VPU_IPC_ENC_GET_INITIAL_INFO,
                                         goto ENCODE_CLOSE);

        if (!get_init_info_msg_ret->initial_info ||
            !get_init_info_msg_ret->initial_info->has_min_frame_buffer_count ||
            !get_init_info_msg_ret->initial_info->has_min_src_frame_count) {
            VLOG(ERR, "[%s][%d] VPU_IPC_ENC_GET_INITIAL_INFO invailed ret msg\n", __func__,
                 __LINE__);
            vpu__enc_get_initial_info__free_unpacked(get_init_info_msg_ret, NULL);
            goto ENCODE_CLOSE;
        }

        memcpy(&vpu_config->enc_init_info, get_init_info_msg_ret->initial_info,
               sizeof(Vpu__EncInitialInfo));

        vpu__enc_get_initial_info__free_unpacked(get_init_info_msg_ret, NULL);
    }

    Vpu__FrameBufferAllocInfo *fb_alloc_info_src = &vpu_config->fb_alloc_info_src;
    *fb_alloc_info_src = (Vpu__FrameBufferAllocInfo)VPU__FRAME_BUFFER_ALLOC_INFO__INIT;

    // VPU_IPC_ENC_REGISTER_ENCODE_BUFFER
    {
        Vpu__EncRegisterEncodeBuffer *register_enc_buf_msg_ret = NULL;
        Vpu__EncRegisterEncodeBuffer register_enc_buf_msg =
            (Vpu__EncRegisterEncodeBuffer)VPU__ENC_REGISTER_ENCODE_BUFFER__INIT;
        Vpu__FrameBufferAllocInfo fb_alloc_info_msg =
            (Vpu__FrameBufferAllocInfo)VPU__FRAME_BUFFER_ALLOC_INFO__INIT;
        Vpu__EncHandle enc_handle_msg = VPU__ENC_HANDLE__INIT;

        register_enc_buf_msg.handle = &enc_handle_msg;
        enc_handle_msg.handle = vpu_config->vpu_handle;

        // vb frame buffer for enc
        register_enc_buf_msg.fb_alloc_info = &fb_alloc_info_msg;

        fb_alloc_info_msg.format = FORMAT_420;
        fb_alloc_info_msg.cbcr_interleave = vpu_config->enc_open_param.cbcr_interleave;
        fb_alloc_info_msg.nv21 = vpu_config->enc_open_param.nv21;
        fb_alloc_info_msg.map_type = vpu_config->map_type;
        fb_alloc_info_msg.height = VPU_ALIGN32(vpu_config->enc_open_param.pic_height);
        fb_alloc_info_msg.stride =
            CalcStride(vpu_config->enc_open_param.pic_width, vpu_config->enc_open_param.pic_height,
                       fb_alloc_info_msg.format, vpu_config->enc_open_param.cbcr_interleave,
                       fb_alloc_info_msg.map_type, FALSE);
        fb_alloc_info_msg.size = VPU_GetFrameBufSize(
            (VpuHandle)vpu_config->vpu_handle,
            vpu_config->core_idx, fb_alloc_info_msg.stride, fb_alloc_info_msg.height,
            fb_alloc_info_msg.map_type, fb_alloc_info_msg.format, fb_alloc_info_msg.cbcr_interleave,
            NULL);                            // dram cfg only needed by coda960
        fb_alloc_info_msg.luma_bit_depth = 8; // 8-bit depth
        fb_alloc_info_msg.chroma_bit_depth = 8;
        fb_alloc_info_msg.num = vpu_config->enc_init_info.min_frame_buffer_count;
        fb_alloc_info_msg.endian = vpu_config->enc_open_param.frame_endian;
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

        VLOG(INFO, "[%s][%d] frame_buffer_count %d, stride %d, height %d, maptype %d size:%d\n",
             __func__, __LINE__, fb_alloc_info_msg.num, fb_alloc_info_msg.stride,
             fb_alloc_info_msg.height, fb_alloc_info_msg.map_type, fb_alloc_info_msg.size);

        uint32_t vb_frame_enc_buf_ptr[fb_alloc_info_msg.num];
        for (int i = 0; i < fb_alloc_info_msg.num; i++) {
            vpu_config->enc_vb_frame[i].size = fb_alloc_info_msg.size;
            if (vdi_allocate_dma_memory(vpu_config->core_idx, &vpu_config->enc_vb_frame[i]) < 0) {
                VLOG(ERR, "%s:%d fail to allocate frame buffer\n", __FUNCTION__, __LINE__);
                goto ENCODE_CLOSE;
            }

            vb_frame_enc_buf_ptr[i] = vpu_config->enc_vb_frame[i].phys_addr;
            VLOG(INFO, "[%s][%d] enc_addr[%d]:%x\n", __func__, __LINE__, i,
                 vb_frame_enc_buf_ptr[i]);
        }

        register_enc_buf_msg.has_vb_frame_buf_size = TRUE;
        register_enc_buf_msg.vb_frame_buf_size = fb_alloc_info_msg.size;
        register_enc_buf_msg.n_vb_frame_buf_phys_addr = fb_alloc_info_msg.num;
        register_enc_buf_msg.vb_frame_buf_phys_addr = vb_frame_enc_buf_ptr;

        // proto VPU_IPC_ENC_REGISTER_ENCODE_BUFFER msg
        SEND_PROTO_AND_CHECK_RET_SUCCESS(vpu__enc_register_encode_buffer, register_enc_buf_msg,
                                         register_enc_buf_msg_ret,
                                         VPU_IPC_ENC_REGISTER_ENCODE_BUFFER, goto ENCODE_CLOSE);
        vpu__enc_register_encode_buffer__free_unpacked(register_enc_buf_msg_ret, NULL);

        *fb_alloc_info_src = fb_alloc_info_msg;
    }

    {
        // vb buffer for input src
        fb_alloc_info_src->num = 1;
        fb_alloc_info_src->stride =
            CalcStride(vpu_config->enc_open_param.pic_width, vpu_config->enc_open_param.pic_height,
                       fb_alloc_info_src->format, vpu_config->enc_open_param.cbcr_interleave,
                       fb_alloc_info_src->map_type, FALSE);
        fb_alloc_info_src->height = VPU_ALIGN16(vpu_config->enc_open_param.pic_height);
        fb_alloc_info_src->type = FB_TYPE_PPU;
        fb_alloc_info_src->size = VPU_GetFrameBufSize(
            (VpuHandle)vpu_config->vpu_handle,
            vpu_config->core_idx, fb_alloc_info_src->stride, fb_alloc_info_src->height,
            fb_alloc_info_src->map_type, fb_alloc_info_src->format,
            fb_alloc_info_src->cbcr_interleave, NULL); // dram cfg only needed by coda960

        vpu_config->enc_vb_src.size = fb_alloc_info_src->size;
        if (vdi_allocate_dma_memory(vpu_config->core_idx, &vpu_config->enc_vb_src) < 0) {
            VLOG(ERR, "%s:%d fail to allocate frame buffer\n", __FUNCTION__, __LINE__);
            goto ENCODE_CLOSE;
        }

        VLOG(INFO, "[%s][%d] frame_buffer_count %d, stride %d, height %d, maptype %d size:%d\n",
             __func__, __LINE__, fb_alloc_info_src->num, fb_alloc_info_src->stride,
             fb_alloc_info_src->height, fb_alloc_info_src->map_type, fb_alloc_info_src->size);

        vpu_config->fb_src = (Vpu__FrameBuffer)VPU__FRAME_BUFFER__INIT;
        vpu_config->fb_src.buf_y = vpu_config->enc_vb_src.phys_addr;
        vpu_config->fb_src.buf_cb = -1;
        vpu_config->fb_src.buf_cr = -1;
        vpu_config->fb_src.update_fb_info = TRUE;

        ppu_fb_alloc_info_to_fb(vpu_config->vpu_handle, fb_alloc_info_src, &vpu_config->fb_src,
                                vpu_config->enc_init_info.min_frame_buffer_count);
    }

    // VPU_IPC_ENC_GET_HEAD
    {
        Vpu__EncGetHead *enc_get_head_msg_ret = NULL;
        Vpu__EncGetHead enc_get_head_msg = (Vpu__EncGetHead)VPU__ENC_GET_HEAD__INIT;
        Vpu__EncHandle enc_handle_msg = VPU__ENC_HANDLE__INIT;

        enc_get_head_msg.handle = &enc_handle_msg;
        enc_handle_msg.handle = vpu_config->vpu_handle;

        enc_get_head_msg.has_bitstream_format = TRUE;
        enc_get_head_msg.bitstream_format = vpu_config->enc_open_param.bitstream_format;
        enc_get_head_msg.has_input_buf_addr = TRUE;
        enc_get_head_msg.input_buf_addr = vpu_config->vb_stream.phys_addr;
        enc_get_head_msg.has_input_buf_size = TRUE;
        enc_get_head_msg.input_buf_size = vpu_config->vb_stream.size;

        // proto VPU_IPC_ENC_GET_HEAD msg
        SEND_PROTO_AND_CHECK_RET_SUCCESS(vpu__enc_get_head, enc_get_head_msg, enc_get_head_msg_ret,
                                         VPU_IPC_ENC_GET_HEAD, goto ENCODE_CLOSE);

        if (!enc_get_head_msg_ret->n_output_buf_addr || !enc_get_head_msg_ret->output_buf_addr ||
            !enc_get_head_msg_ret->n_output_buf_size || !enc_get_head_msg_ret->output_buf_size ||
            enc_get_head_msg_ret->n_output_buf_addr != enc_get_head_msg_ret->n_output_buf_size) {
            VLOG(ERR, "[%s][%d] VPU_IPC_ENC_GET_HEAD invailed ret msg\n", __func__, __LINE__);
            vpu__enc_get_head__free_unpacked(enc_get_head_msg_ret, NULL);
            goto ENCODE_CLOSE;
        }

        for (size_t i = 0; i < enc_get_head_msg_ret->n_output_buf_addr; i++) {
            ret = vdi_read_memory(vpu_config->core_idx, enc_get_head_msg_ret->output_buf_addr[i],
                                  vpu_config->enc_output_bs_buf.resource.resource_buffer +
                                      vpu_config->enc_output_bs_buf.resource.resource_size,
                                  enc_get_head_msg_ret->output_buf_size[i],
                                  vpu_config->enc_open_param.stream_endian);
            if (ret < 0) {
                VLOG(ERR, "[%s][%d] vdi_read_memory failed ret:%d\n", __func__, __LINE__, ret);
                vpu__enc_get_head__free_unpacked(enc_get_head_msg_ret, NULL);
                goto ENCODE_CLOSE;
            }

            vpu_config->enc_output_bs_buf.resource.resource_size +=
                enc_get_head_msg_ret->output_buf_size[i];
        }

        for (int i = 0; i < vpu_config->enc_output_bs_buf.resource.resource_size; i++) {
            printf("\t%x", ((char *)vpu_config->enc_output_bs_buf.resource.resource_buffer)[i]);
        }
        printf("\n");

        vpu__enc_get_head__free_unpacked(enc_get_head_msg_ret, NULL);
    }

    while (TRUE) {
        // write frame data to frame buf
        ret = write_frame_data_to_src_buf(vpu_config, &vpu_config->enc_vb_src, &vpu_config->fb_src);
        if (ret < 0) {
            VLOG(ERR, "[%s][%d] write_frame_data_to_src_buf error, ret:%d\n", __func__, __LINE__,
                 ret);
            goto ENCODE_CLOSE;
        } else if (ret == 0) {
            VLOG(ERR, "[%s][%d] read all input buf\n", __func__, __LINE__);
            break;
        } else {
            disp_enc_input_buf(vpu_config, &vpu_config->fb_src);

            // VPU_IPC_ENC_ONE_FRAMER
            Vpu__EncOneFrame *enc_one_frame_msg_ret = NULL;
            Vpu__EncOneFrame enc_one_frame_msg = (Vpu__EncOneFrame)VPU__ENC_ONE_FRAME__INIT;
            Vpu__EncParam enc_param_msg = (Vpu__EncParam)VPU__ENC_PARAM__INIT;
            Vpu__EncHandle enc_handle_msg = VPU__ENC_HANDLE__INIT;

            enc_one_frame_msg.handle = &enc_handle_msg;
            enc_handle_msg.handle = vpu_config->vpu_handle;

            enc_one_frame_msg.enc_param = &enc_param_msg;

            enc_param_msg.source_frame = &vpu_config->fb_src;
            enc_param_msg.quant_param = 10;
            enc_param_msg.force_i_picture = 0;
            enc_param_msg.skip_picture = 0;
            enc_param_msg.pic_stream_buffer_addr = vpu_config->vb_stream.phys_addr;
            enc_param_msg.pic_stream_buffer_size = vpu_config->vb_stream.size;
            enc_param_msg.src_idx = 0;

            enc_param_msg.has_quant_param = TRUE;
            enc_param_msg.has_force_i_picture = TRUE;
            enc_param_msg.has_skip_picture = TRUE;
            enc_param_msg.has_pic_stream_buffer_addr = TRUE;
            enc_param_msg.has_pic_stream_buffer_size = TRUE;
            enc_param_msg.has_src_idx = TRUE;

            if (!frame_idx) // set first frame to I frame
                enc_param_msg.force_i_picture = 1;

            // proto VPU_IPC_ENC_ONE_FRAMER msg
            SEND_PROTO_AND_CHECK_RET_SUCCESS(vpu__enc_one_frame, enc_one_frame_msg,
                                             enc_one_frame_msg_ret, VPU_IPC_ENC_ONE_FRAMER,
                                             goto ENCODE_CLOSE);

            if (!enc_one_frame_msg_ret->enc_output_info ||
                !enc_one_frame_msg_ret->enc_output_info->bitstream_size ||
                !enc_one_frame_msg_ret->enc_output_info->bitstream_buffer) {
                VLOG(ERR, "[%s][%d] VPU_IPC_ENC_ONE_FRAMER invailed ret msg\n", __func__, __LINE__);
                vpu__enc_one_frame__free_unpacked(enc_one_frame_msg_ret, NULL);
                goto ENCODE_CLOSE;
            }

            if (vpu_config->enc_output_bs_buf.resource.resource_size +
                    enc_one_frame_msg_ret->enc_output_info->bitstream_size >
                VPU_ENC_OUTPUT_BS_BUFFER_SIZE) {
                VLOG(ERR,
                     "[%s][%d] output bit streamer buffer is full, bs buffer size:%d, filled "
                     "size:%d, coming size:%d \n",
                     __func__, __LINE__, VPU_ENC_OUTPUT_BS_BUFFER_SIZE,
                     vpu_config->enc_output_bs_buf.resource.resource_size,
                     enc_one_frame_msg_ret->enc_output_info->bitstream_size);
                vpu__enc_one_frame__free_unpacked(enc_one_frame_msg_ret, NULL);
                goto ENCODE_CLOSE;
            }

            ret = vdi_read_memory(vpu_config->core_idx,
                                  enc_one_frame_msg_ret->enc_output_info->bitstream_buffer,
                                  vpu_config->enc_output_bs_buf.resource.resource_buffer +
                                      vpu_config->enc_output_bs_buf.resource.resource_size,
                                  enc_one_frame_msg_ret->enc_output_info->bitstream_size,
                                  vpu_config->enc_open_param.stream_endian);
            if (ret < 0) {
                VLOG(ERR, "[%s][%d] vdi_read_memory failed ret:%d\n", __func__, __LINE__, ret);
                vpu__enc_one_frame__free_unpacked(enc_one_frame_msg_ret, NULL);
                goto ENCODE_CLOSE;
            }

            vpu_config->enc_output_bs_buf.resource.resource_size +=
                enc_one_frame_msg_ret->enc_output_info->bitstream_size;
            VLOG(INFO, "[%s][%d] VPU_IPC_ENC_ONE_FRAMER enc size:%d pic_type:%d\n", __func__,
                 __LINE__, enc_one_frame_msg_ret->enc_output_info->bitstream_size,
                 enc_one_frame_msg_ret->enc_output_info->pic_type);
            vpu__enc_one_frame__free_unpacked(enc_one_frame_msg_ret, NULL);

            frame_idx++;
        }
    }

    VLOG(INFO, "[%s][%d] enc over, h264 stream size:%d\n", __func__, __LINE__,
         vpu_config->enc_output_bs_buf.resource.resource_size);

    // print enc data to serial
    // printf("\n******************start*****************\n");
    // for (int i = 0; i < vpu_config->enc_output_bs_buf.resource.resource_size; i++) {
    //     printf("%02x", ((char *)vpu_config->enc_output_bs_buf.resource.resource_buffer)[i]);
    // }
    // printf("\n******************over*****************\n");

ENCODE_CLOSE:
    // VPU_EncClose
    {
        Vpu__EncClose enc_close_msg = VPU__ENC_CLOSE__INIT;
        Vpu__EncHandle enc_handle_msg = VPU__ENC_HANDLE__INIT;

        enc_close_msg.handle = &enc_handle_msg;
        enc_handle_msg.handle = vpu_config->vpu_handle;

        // proto msg
        msg_len = vpu__enc_close__get_packed_size(&enc_close_msg);
        vpu__enc_close__pack(&enc_close_msg, vpu_config->msg_buf);

        ret = vpu_send_msg_to_service(&vpu_config->pack, VPU_IPC_ENC_CLOSE, vpu_config->msg_buf,
                                      msg_len);
        if (ret < 0) {
            VLOG(ERR, "VPU_EncClose failed\n");
        }
    }

    for (int i = 0; i < MAX_REG_FRAME; i++) {
        if (vpu_config->enc_vb_frame[i].size > 0)
            vdi_free_dma_memory(vpu_config->core_idx, &vpu_config->enc_vb_frame[i]);
    }

    if (vpu_config->enc_vb_src.size > 0)
        vdi_free_dma_memory(vpu_config->core_idx, &vpu_config->enc_vb_src);

ERR_ENC_INIT:
    if (vpu_config->vb_stream.size > 0) {
        vdi_free_dma_memory(vpu_config->core_idx, &vpu_config->vb_stream);
    }

    if (vpu_config->enc_resource.resource.resource_buffer)
        free(vpu_config->enc_resource.resource.resource_buffer);

    if (vpu_config->enc_output_bs_buf.resource.resource_buffer) {
        free(vpu_config->enc_output_bs_buf.resource.resource_buffer);
    }

    if (vpu_config->msg_buf)
        free(vpu_config->msg_buf);

    if (vpu_config->display)
        sdm_clear_display(vpu_config->display->handle);

    if (vpu_config)
        free(vpu_config);

    return ret;
}

int vpu_test_encoder_h265(void)
{
    int ret = 0;
    int msg_len = 0;
    int frame_idx = 0;

    vpu_enc_config_t *vpu_config = osal_malloc(sizeof(vpu_enc_config_t));

    VLOG(ERR, "%s: %d\n", __FUNCTION__, __LINE__);
    osal_memset(vpu_config, 0x00, sizeof(vpu_enc_config_t));

    {
        vpu_config->core_idx = VPU_WAVE521_IDX;
        vpu_config->map_type = LINEAR_FRAME_MAP;
        vpu_config->msg_buf = osal_malloc(VPU_PROTO_BUFFER_SIZE);

        // vbstream
        vpu_config->vb_stream.size = VPU_STREAM_BUF_SIZE;
        if (vdi_allocate_dma_memory(vpu_config->core_idx, &vpu_config->vb_stream) < 0) {
            VLOG(ERR, "fail to allocate bitstream buffer\n");
            ret = -1;
            goto ERR_ENC_INIT;
        }

        VLOG(ERR, "[Main] vbStream base 0x%llx, phy 0x%llx, virt 0x%llx\n",
             vpu_config->vb_stream.base, vpu_config->vb_stream.phys_addr,
             vpu_config->vb_stream.virt_addr);

        // load test enc h264 file resource
        vpu_config->enc_resource.resource.resource_path = VPU_TEST_ENC_H265_RESOURCE_PATH;
        vpu_config->enc_resource.resource.resource_size =
            res_size(vpu_config->enc_resource.resource.resource_path);
        vpu_config->enc_resource.resource.resource_buffer =
            memalign(1024, vpu_config->enc_resource.resource.resource_size);

        ret = res_load(vpu_config->enc_resource.resource.resource_path,
                       vpu_config->enc_resource.resource.resource_buffer,
                       ROUNDUP(vpu_config->enc_resource.resource.resource_size, 1024), 0);
        if (ret < 0) {
            VLOG(ERR, "[%s][%d] res_load failed\n", __func__, __LINE__);
            goto ERR_ENC_INIT;
        }

        // enc output buf
        vpu_config->enc_output_bs_buf.resource.resource_buffer =
            osal_malloc(VPU_ENC_OUTPUT_BS_BUFFER_SIZE);
    }

    VLOG(ERR, "load resource success!\n");

    // VPU ENC open
    {
        VLOG(ERR, "Enter VPU encode open!\n");

        Vpu__EncOpen *enc_open_msg_ret      = NULL;
        Vpu__EncOpen enc_open_msg           = VPU__ENC_OPEN__INIT;
        Vpu__EncSliceMode slice_mode_msg    = VPU__ENC_SLICE_MODE__INIT;
        Vpu__EncHandle enc_handle_msg       = VPU__ENC_HANDLE__INIT;
        Vpu__EncOpenParam *enc_op_msg       = &vpu_config->enc_open_param;

        int32_t fixed_bit_ratio[MAX_GOP_NUM] = {0};

        *enc_op_msg = (Vpu__EncOpenParam)VPU__ENC_OPEN_PARAM__INIT;
        enc_open_msg.handle = &enc_handle_msg;
        enc_open_msg.enc_op = enc_op_msg;

        // enc open param for coda & wave521
        enc_op_msg->bitstream_format    = STD_HEVC;
        enc_op_msg->cbcr_interleave     = 0;
        enc_op_msg->nv21                = 0;
        enc_op_msg->cbcr_order          = CBCR_ORDER_NORMAL;
        enc_op_msg->pic_width           = VPU_TEST_H265_ENC_WIDTH;
        enc_op_msg->pic_height          = VPU_TEST_H265_ENC_HEIGHT;
        enc_op_msg->frame_rate_info     = 30;
        enc_op_msg->bit_rate            = 0;
        enc_op_msg->frame_skip_disable  = 1;
        enc_op_msg->gop_size            = 0;
        enc_op_msg->intra_refresh       = 0;
        enc_op_msg->rc_enable           = 0;
        enc_op_msg->sram_enable         = TRUE;

        enc_op_msg->bitstream_buffer    = vpu_config->vb_stream.phys_addr;
        enc_op_msg->bitstream_buffer_size = vpu_config->vb_stream.size;
        enc_op_msg->ring_buffer_enable  = 0;
        enc_op_msg->core_idx            = vpu_config->core_idx;
        enc_op_msg->frame_endian        = VDI_LITTLE_ENDIAN;
        enc_op_msg->stream_endian       = VDI_LITTLE_ENDIAN;
        enc_op_msg->bwb_enable          = TRUE;
        enc_op_msg->line_buf_int_en     = 0;
        enc_op_msg->linear2_tiled_enable = FALSE;
        enc_op_msg->linear2_tiled_mode  = 0;
        enc_op_msg->initial_delay       = 1000;
        enc_op_msg->vbv_buffer_size     = 0;
        enc_op_msg->me_blk_mode         = 0;
        enc_op_msg->idr_interval        = 0;
        enc_op_msg->consc_intra_refresh_enable = 0;
        enc_op_msg->count_intra_mb_enable = 0;
        enc_op_msg->field_seq_intra_refresh_enable = 0;
        enc_op_msg->rc_intra_qp         = -1;
        enc_op_msg->intra_cost_weight   = 0;
        enc_op_msg->rc_gop_i_qp_offset_en = 0;
        enc_op_msg->rc_gop_i_qp_offset  = 0;
        enc_op_msg->me_search_range_x   = 3;
        enc_op_msg->me_search_range_y   = 2;
        enc_op_msg->max_intra_size      = 0;
        enc_op_msg->user_gamma          = -1;
        enc_op_msg->me_use_zero_pmv     = 0;
        enc_op_msg->rc_interval_mode    = 0;
        enc_op_msg->mb_interval         = 1;
        enc_op_msg->map_type            = LINEAR_FRAME_MAP;

        if (enc_op_msg->rc_enable == 2)
            enc_op_msg->user_qp_max = 32;
        else
            enc_op_msg->user_qp_max = 31;

        if (vpu_config->core_idx == VPU_WAVE521_IDX) {
            // if (enc_op_msg->bit_rate) {
            //     // It specifies the size of the VBV buffer in msec (10 ~ 3000)
            //     enc_op_msg->vbv_buffer_size = FSAT(enc_op_msg->vbv_buffer_size, 3000, 10);
            // }

            enc_op_msg->src_bit_depth = 8;
            enc_op_msg->frame_endian = VDI_128BIT_LITTLE_ENDIAN;
            enc_op_msg->stream_endian = VDI_128BIT_LITTLE_ENDIAN;
            enc_op_msg->source_endian = VDI_128BIT_LITTLE_ENDIAN;

            enc_op_msg->has_src_bit_depth = TRUE;
            enc_op_msg->has_source_endian = TRUE;
        }

        enc_op_msg->has_bitstream_format        = TRUE;
        enc_op_msg->has_bitstream_buffer        = TRUE;
        enc_op_msg->has_bitstream_buffer_size   = TRUE;
        enc_op_msg->has_ring_buffer_enable      = TRUE;
        enc_op_msg->has_cbcr_interleave         = TRUE;
        enc_op_msg->has_nv21                    = TRUE;
        enc_op_msg->has_frame_endian            = TRUE;
        enc_op_msg->has_stream_endian           = TRUE;
        enc_op_msg->has_bwb_enable              = TRUE;
        enc_op_msg->has_line_buf_int_en         = TRUE;
        enc_op_msg->has_core_idx                = TRUE;
        enc_op_msg->has_linear2_tiled_enable    = TRUE;
        enc_op_msg->has_linear2_tiled_mode      = TRUE;
        enc_op_msg->has_cbcr_order              = TRUE;
        enc_op_msg->has_pic_width               = TRUE;
        enc_op_msg->has_pic_height              = TRUE;
        enc_op_msg->has_frame_rate_info         = TRUE;
        enc_op_msg->has_bit_rate                = TRUE;
        enc_op_msg->has_initial_delay           = TRUE;
        enc_op_msg->has_vbv_buffer_size         = TRUE;
        enc_op_msg->has_frame_skip_disable      = TRUE;
        enc_op_msg->has_me_blk_mode             = TRUE;
        enc_op_msg->has_gop_size                = TRUE;
        enc_op_msg->has_idr_interval            = TRUE;
        enc_op_msg->has_intra_refresh           = TRUE;
        enc_op_msg->has_consc_intra_refresh_enable = TRUE;
        enc_op_msg->has_count_intra_mb_enable   = TRUE;
        enc_op_msg->has_field_seq_intra_refresh_enable = TRUE;
        enc_op_msg->has_rc_intra_qp             = TRUE;
        enc_op_msg->has_intra_cost_weight       = TRUE;
        enc_op_msg->has_rc_gop_i_qp_offset_en   = TRUE;
        enc_op_msg->has_rc_gop_i_qp_offset      = TRUE;
        enc_op_msg->has_me_search_range_x       = TRUE;
        enc_op_msg->has_me_search_range_y       = TRUE;
        enc_op_msg->has_max_intra_size          = TRUE;
        enc_op_msg->has_rc_enable               = TRUE;
        enc_op_msg->has_user_gamma              = TRUE;
        enc_op_msg->has_me_use_zero_pmv         = TRUE;
        enc_op_msg->has_user_qp_max             = TRUE;
        enc_op_msg->has_rc_interval_mode        = TRUE;
        enc_op_msg->has_mb_interval             = TRUE;
        enc_op_msg->has_map_type                = TRUE;
        enc_op_msg->has_sram_enable             = TRUE;

        // slice mode
        enc_op_msg->slice_mode                  = &slice_mode_msg;
        slice_mode_msg.slice_mode               = 0;
        slice_mode_msg.slice_size_mode          = 0;
        slice_mode_msg.slice_size               = 8192;
        slice_mode_msg.has_slice_mode           = TRUE;
        slice_mode_msg.has_slice_size_mode      = TRUE;
        slice_mode_msg.has_slice_size           = TRUE;

        // enc codc std param
        enc_op_msg->enc_std_param = &vpu_config->enc_std_param;
        *enc_op_msg->enc_std_param = (Vpu__EncStdParam)VPU__ENC_STD_PARAM__INIT;

        if (vpu_config->core_idx == VPU_WAVE521_IDX) {

            VLOG(ERR, "Enter VPU_WAVE521_IDX!\n");

            // h264 / h265 param for wave521
            enc_op_msg->enc_std_param->param_case = VPU__ENC_STD_PARAM__PARAM_WAVE_PARAM;
            enc_op_msg->enc_std_param->wave_param = &vpu_config->enc_wave_param;
            *enc_op_msg->enc_std_param->wave_param =
                (Vpu__EncStdParam__EncWaveParam)VPU__ENC_STD_PARAM__ENC_WAVE_PARAM__INIT;

            if (enc_op_msg->bitstream_format == STD_HEVC) {
                enc_op_msg->enc_std_param->wave_param->profile = HEVC_PROFILE_MAIN;
                enc_op_msg->enc_std_param->wave_param->level = 0;
                enc_op_msg->enc_std_param->wave_param->intra_period = 30;
            } else {
                enc_op_msg->enc_std_param->wave_param->profile = 0;
                enc_op_msg->enc_std_param->wave_param->level = 0;
                enc_op_msg->enc_std_param->wave_param->intra_period = enc_op_msg->gop_size;
            }

            enc_op_msg->enc_std_param->wave_param->tier = 0;
            enc_op_msg->enc_std_param->wave_param->internal_bit_depth = 8;
            enc_op_msg->enc_std_param->wave_param->lossless_enable = 0;
            enc_op_msg->enc_std_param->wave_param->const_intra_pred_flag = 0;
            enc_op_msg->enc_std_param->wave_param->use_long_term = 0;

            /* for CMD_ENC_SEQ_GOP_PARAM */
            enc_op_msg->enc_std_param->wave_param->gop_preset_idx = PRESET_IDX_IPPPP;

            /* for CMD_ENC_SEQ_INTRA_PARAM */
            enc_op_msg->enc_std_param->wave_param->decoding_refresh_type =
                ((enc_op_msg->enc_std_param->wave_param->gop_preset_idx == 1) &&
                 (enc_op_msg->rc_enable) && (enc_op_msg->bitstream_format == STD_HEVC))
                    ? 2
                    : 1;

            enc_op_msg->enc_std_param->wave_param->intra_qp = 30;
            enc_op_msg->enc_std_param->wave_param->forced_idr_header_enable = 1;

            /* for cmd_enc_seq_conf_win_top_bot/left_right */
            enc_op_msg->enc_std_param->wave_param->conf_win_top = 0;
            enc_op_msg->enc_std_param->wave_param->conf_win_bot = 0;
            enc_op_msg->enc_std_param->wave_param->conf_win_left = 0;
            enc_op_msg->enc_std_param->wave_param->conf_win_right = 0;

            /* for cmd_enc_seq_independent_slice */
            enc_op_msg->enc_std_param->wave_param->independ_slice_mode = 0;
            enc_op_msg->enc_std_param->wave_param->independ_slice_mode_arg = 0;

            /* for cmd_enc_seq_dependent_slice */
            enc_op_msg->enc_std_param->wave_param->depend_slice_mode = 0;
            enc_op_msg->enc_std_param->wave_param->depend_slice_mode_arg = 0;

            /* for cmd_enc_seq_intra_refresh_param */
            enc_op_msg->enc_std_param->wave_param->intra_refresh_mode = 0;
            enc_op_msg->enc_std_param->wave_param->intra_refresh_arg = 0;
            enc_op_msg->enc_std_param->wave_param->use_recommend_enc_param = 1;
            enc_op_msg->enc_std_param->wave_param->cu_size_mode = 0x3; // picwidth , picheight should be aligned with 16
            if (enc_op_msg->enc_std_param->wave_param->use_recommend_enc_param != 1) {
                // 0 : custom,  2 : boost mode (normal encoding speed, normal picture quality),
                // 3 : fast mode (high encoding speed, low picture quality)
                enc_op_msg->enc_std_param->wave_param->scaling_list_enable = 0;
                enc_op_msg->enc_std_param->wave_param->tmvp_enable = 1;
                enc_op_msg->enc_std_param->wave_param->wpp_enable = 0;
                enc_op_msg->enc_std_param->wave_param->max_num_merge = 2;
                enc_op_msg->enc_std_param->wave_param->disable_deblk = 0;
                enc_op_msg->enc_std_param->wave_param->lf_cross_slice_boundary_enable = 1;
                enc_op_msg->enc_std_param->wave_param->beta_offset_div2 = 0;
                enc_op_msg->enc_std_param->wave_param->tc_offset_div2 = 0;
                enc_op_msg->enc_std_param->wave_param->skip_intra_trans = 1;
                enc_op_msg->enc_std_param->wave_param->sao_enable = 1;
                enc_op_msg->enc_std_param->wave_param->intra_nx_n_enable = 1;
            }

            enc_op_msg->enc_std_param->wave_param->roi_enable = 0;
            enc_op_msg->enc_std_param->wave_param->bit_alloc_mode = 0;

            enc_op_msg->enc_std_param->wave_param->fixed_bit_ratio = fixed_bit_ratio;
            enc_op_msg->enc_std_param->wave_param->n_fixed_bit_ratio = MAX_GOP_NUM;
            for (int i = 0; i < MAX_GOP_NUM; i++) {
                enc_op_msg->enc_std_param->wave_param->fixed_bit_ratio[i] = 1;
            }
            enc_op_msg->enc_std_param->wave_param->cu_level_rc_enable = 0;
            enc_op_msg->enc_std_param->wave_param->hvs_qp_enable = 1;
            enc_op_msg->enc_std_param->wave_param->hvs_qp_scale = 2;

            /* for cmd_enc_rc_min_max_qp */
            enc_op_msg->enc_std_param->wave_param->min_qp_i = 8;
            enc_op_msg->enc_std_param->wave_param->max_qp_i = 51;
            enc_op_msg->enc_std_param->wave_param->min_qp_p = 8;
            enc_op_msg->enc_std_param->wave_param->max_qp_p = 51;
            enc_op_msg->enc_std_param->wave_param->min_qp_b = 8;
            enc_op_msg->enc_std_param->wave_param->max_qp_b = 51;

            enc_op_msg->enc_std_param->wave_param->hvs_max_delta_qp = 10;

            /* for cmd_enc_custom_gop_param */
            // enc_op_msg->enc_std_param->wave_param->gop_param.custom_gop_size = 0;
            // for (int i = 0; i < enc_op_msg->enc_std_param->wave_param->gop_param.custom_gop_size; i++) {
            //     enc_op_msg->enc_std_param->wave_param->gop_param.pic_param[i].pic_type = PIC_TYPE_I;
            //     enc_op_msg->enc_std_param->wave_param->gop_param.pic_param[i].poc_offset = 1;
            //     enc_op_msg->enc_std_param->wave_param->gop_param.pic_param[i].pic_qp = 30;
            //     enc_op_msg->enc_std_param->wave_param->gop_param.pic_param[i].ref_poc_l0 = 0;
            //     enc_op_msg->enc_std_param->wave_param->gop_param.pic_param[i].ref_poc_l1 = 0;
            //     enc_op_msg->enc_std_param->wave_param->gop_param.pic_param[i].temporal_id = 0;
            // }

            // for vui / time information.
            enc_op_msg->enc_std_param->wave_param->num_ticks_poc_diff_one = 0;
            enc_op_msg->enc_std_param->wave_param->time_scale = enc_op_msg->frame_rate_info * 1000;
            enc_op_msg->enc_std_param->wave_param->num_units_in_tick = 1000;

            enc_op_msg->enc_std_param->wave_param->chroma_cb_qp_offset = 0;
            enc_op_msg->enc_std_param->wave_param->chroma_cr_qp_offset = 0;
            enc_op_msg->enc_std_param->wave_param->initial_rc_qp = -1; // 63 is meaningless.
            enc_op_msg->enc_std_param->wave_param->nr_y_enable = 0;
            enc_op_msg->enc_std_param->wave_param->nr_cb_enable = 0;
            enc_op_msg->enc_std_param->wave_param->nr_cr_enable = 0;
            enc_op_msg->enc_std_param->wave_param->nr_noise_est_enable = 0;

            enc_op_msg->enc_std_param->wave_param->monochrome_enable = 0;
            enc_op_msg->enc_std_param->wave_param->strong_intra_smooth_enable = 1;
            enc_op_msg->enc_std_param->wave_param->weight_pred_enable = 0;
            enc_op_msg->enc_std_param->wave_param->bg_detect_enable = 0;
            enc_op_msg->enc_std_param->wave_param->bg_thr_diff = 8;
            enc_op_msg->enc_std_param->wave_param->bg_thr_mean_diff = 1;
            enc_op_msg->enc_std_param->wave_param->bg_lambda_qp = 32;
            enc_op_msg->enc_std_param->wave_param->bg_delta_qp = 3;
            enc_op_msg->enc_std_param->wave_param->rc_weight_param = 2;
            enc_op_msg->enc_std_param->wave_param->rc_weight_buf = 128;
            // for h.264 encoder
            enc_op_msg->enc_std_param->wave_param->avc_idr_period =
                ((enc_op_msg->enc_std_param->wave_param->gop_preset_idx == 1) &&
                 (enc_op_msg->rc_enable) && (enc_op_msg->bitstream_format == STD_HEVC))
                    ? 1
                    : 0;
            enc_op_msg->enc_std_param->wave_param->rdo_skip = 1;
            enc_op_msg->enc_std_param->wave_param->lambda_scaling_enable = 1;
            enc_op_msg->enc_std_param->wave_param->transform8x8_enable = 1;
            enc_op_msg->enc_std_param->wave_param->avc_slice_mode = 0;
            enc_op_msg->enc_std_param->wave_param->avc_slice_arg = 0;
            enc_op_msg->enc_std_param->wave_param->intra_mb_refresh_mode = 0;
            enc_op_msg->enc_std_param->wave_param->intra_mb_refresh_arg = 1;
            enc_op_msg->enc_std_param->wave_param->mb_level_rc_enable = 0;
            enc_op_msg->enc_std_param->wave_param->entropy_coding_mode = 1;
            enc_op_msg->enc_std_param->wave_param->disable_deblk = 0;


            enc_op_msg->enc_std_param->wave_param->has_profile = TRUE;
            enc_op_msg->enc_std_param->wave_param->has_level = TRUE;
            enc_op_msg->enc_std_param->wave_param->has_intra_period = TRUE;
            enc_op_msg->enc_std_param->wave_param->has_tier = TRUE;
            enc_op_msg->enc_std_param->wave_param->has_internal_bit_depth = TRUE;
            enc_op_msg->enc_std_param->wave_param->has_lossless_enable = TRUE;
            enc_op_msg->enc_std_param->wave_param->has_const_intra_pred_flag = TRUE;
            enc_op_msg->enc_std_param->wave_param->has_use_long_term = TRUE;
            enc_op_msg->enc_std_param->wave_param->has_gop_preset_idx = TRUE;
            enc_op_msg->enc_std_param->wave_param->has_decoding_refresh_type = TRUE;
            enc_op_msg->enc_std_param->wave_param->has_intra_qp = TRUE;
            enc_op_msg->enc_std_param->wave_param->has_forced_idr_header_enable = TRUE;
            enc_op_msg->enc_std_param->wave_param->has_conf_win_top = TRUE;
            enc_op_msg->enc_std_param->wave_param->has_conf_win_bot = TRUE;
            enc_op_msg->enc_std_param->wave_param->has_conf_win_left = TRUE;
            enc_op_msg->enc_std_param->wave_param->has_conf_win_right = TRUE;
            enc_op_msg->enc_std_param->wave_param->has_independ_slice_mode = TRUE;
            enc_op_msg->enc_std_param->wave_param->has_independ_slice_mode_arg = TRUE;
            enc_op_msg->enc_std_param->wave_param->has_depend_slice_mode = TRUE;
            enc_op_msg->enc_std_param->wave_param->has_depend_slice_mode_arg = TRUE;
            enc_op_msg->enc_std_param->wave_param->has_intra_refresh_mode = TRUE;
            enc_op_msg->enc_std_param->wave_param->has_intra_refresh_arg = TRUE;
            enc_op_msg->enc_std_param->wave_param->has_use_recommend_enc_param = TRUE;
            enc_op_msg->enc_std_param->wave_param->has_cu_size_mode = TRUE;

            enc_op_msg->enc_std_param->wave_param->has_scaling_list_enable = TRUE;
            enc_op_msg->enc_std_param->wave_param->has_tmvp_enable = TRUE;
            enc_op_msg->enc_std_param->wave_param->has_wpp_enable = TRUE;
            enc_op_msg->enc_std_param->wave_param->has_max_num_merge = TRUE;
            enc_op_msg->enc_std_param->wave_param->has_disable_deblk = TRUE;
            enc_op_msg->enc_std_param->wave_param->has_lf_cross_slice_boundary_enable = TRUE;
            enc_op_msg->enc_std_param->wave_param->has_beta_offset_div2 = TRUE;
            enc_op_msg->enc_std_param->wave_param->has_tc_offset_div2 = TRUE;
            enc_op_msg->enc_std_param->wave_param->has_skip_intra_trans = TRUE;
            enc_op_msg->enc_std_param->wave_param->has_sao_enable = TRUE;
            enc_op_msg->enc_std_param->wave_param->has_intra_nx_n_enable = TRUE;

            enc_op_msg->enc_std_param->wave_param->has_roi_enable = TRUE;
            enc_op_msg->enc_std_param->wave_param->has_bit_alloc_mode = TRUE;

            enc_op_msg->enc_std_param->wave_param->has_cu_level_rc_enable = TRUE;
            enc_op_msg->enc_std_param->wave_param->has_hvs_qp_enable = TRUE;
            enc_op_msg->enc_std_param->wave_param->has_hvs_qp_scale = TRUE;

            enc_op_msg->enc_std_param->wave_param->has_min_qp_i = TRUE;
            enc_op_msg->enc_std_param->wave_param->has_max_qp_i = TRUE;
            enc_op_msg->enc_std_param->wave_param->has_min_qp_p = TRUE;
            enc_op_msg->enc_std_param->wave_param->has_max_qp_p = TRUE;
            enc_op_msg->enc_std_param->wave_param->has_min_qp_b = TRUE;
            enc_op_msg->enc_std_param->wave_param->has_max_qp_b = TRUE;

            enc_op_msg->enc_std_param->wave_param->has_hvs_max_delta_qp = TRUE;

            enc_op_msg->enc_std_param->wave_param->has_num_ticks_poc_diff_one = TRUE;
            enc_op_msg->enc_std_param->wave_param->has_time_scale = TRUE;
            enc_op_msg->enc_std_param->wave_param->has_num_units_in_tick = TRUE;

            enc_op_msg->enc_std_param->wave_param->has_chroma_cb_qp_offset = TRUE;
            enc_op_msg->enc_std_param->wave_param->has_chroma_cr_qp_offset = TRUE;
            enc_op_msg->enc_std_param->wave_param->has_initial_rc_qp = TRUE;
            enc_op_msg->enc_std_param->wave_param->has_nr_y_enable = TRUE;
            enc_op_msg->enc_std_param->wave_param->has_nr_cb_enable = TRUE;
            enc_op_msg->enc_std_param->wave_param->has_nr_cr_enable = TRUE;
            enc_op_msg->enc_std_param->wave_param->has_nr_noise_est_enable = TRUE;

            enc_op_msg->enc_std_param->wave_param->has_monochrome_enable = TRUE;
            enc_op_msg->enc_std_param->wave_param->has_strong_intra_smooth_enable = TRUE;
            enc_op_msg->enc_std_param->wave_param->has_weight_pred_enable = TRUE;
            enc_op_msg->enc_std_param->wave_param->has_bg_detect_enable = TRUE;
            enc_op_msg->enc_std_param->wave_param->has_bg_thr_diff = TRUE;
            enc_op_msg->enc_std_param->wave_param->has_bg_thr_mean_diff = TRUE;
            enc_op_msg->enc_std_param->wave_param->has_bg_lambda_qp = TRUE;
            enc_op_msg->enc_std_param->wave_param->has_bg_delta_qp = TRUE;
            enc_op_msg->enc_std_param->wave_param->has_rc_weight_param = TRUE;
            enc_op_msg->enc_std_param->wave_param->has_rc_weight_buf = TRUE;

            enc_op_msg->enc_std_param->wave_param->has_avc_idr_period = TRUE;
            enc_op_msg->enc_std_param->wave_param->has_rdo_skip = TRUE;
            enc_op_msg->enc_std_param->wave_param->has_lambda_scaling_enable = TRUE;
            enc_op_msg->enc_std_param->wave_param->has_transform8x8_enable = TRUE;
            enc_op_msg->enc_std_param->wave_param->has_avc_slice_mode = TRUE;
            enc_op_msg->enc_std_param->wave_param->has_avc_slice_arg = TRUE;
            enc_op_msg->enc_std_param->wave_param->has_intra_mb_refresh_mode = TRUE;
            enc_op_msg->enc_std_param->wave_param->has_intra_mb_refresh_arg = TRUE;
            enc_op_msg->enc_std_param->wave_param->has_mb_level_rc_enable = TRUE;
            enc_op_msg->enc_std_param->wave_param->has_entropy_coding_mode = TRUE;
            enc_op_msg->enc_std_param->wave_param->has_disable_deblk = TRUE;
        }

        // proto VPU_IPC_ENC_OPEN msg
        SEND_PROTO_AND_CHECK_RET_SUCCESS(vpu__enc_open, enc_open_msg, enc_open_msg_ret,
                                         VPU_IPC_ENC_OPEN, goto ERR_ENC_INIT);

        vpu_config->vpu_handle = enc_open_msg_ret->handle->handle;
        vpu__enc_open__free_unpacked(enc_open_msg_ret, NULL);

        VLOG(ERR, "VPU_EncOpen success, vpu handle:0x%x\n", vpu_config->vpu_handle);
    }

    // VPU_EncGetInitialInfo
    {
        Vpu__EncGetInitialInfo *get_init_info_msg_ret = NULL;
        Vpu__EncGetInitialInfo get_init_info_msg =
            (Vpu__EncGetInitialInfo)VPU__ENC_GET_INITIAL_INFO__INIT;
        Vpu__EncHandle enc_handle_msg = VPU__ENC_HANDLE__INIT;

        get_init_info_msg.handle = &enc_handle_msg;
        enc_handle_msg.handle = vpu_config->vpu_handle;

        // proto VPU_IPC_ENC_GET_INITIAL_INFO
        SEND_PROTO_AND_CHECK_RET_SUCCESS(vpu__enc_get_initial_info, get_init_info_msg,
                                         get_init_info_msg_ret, VPU_IPC_ENC_GET_INITIAL_INFO,
                                         goto ENCODE_CLOSE);

        if (!get_init_info_msg_ret->initial_info ||
            !get_init_info_msg_ret->initial_info->has_min_frame_buffer_count ||
            !get_init_info_msg_ret->initial_info->has_min_src_frame_count) {
            VLOG(ERR, "[%s][%d] VPU_IPC_ENC_GET_INITIAL_INFO invailed ret msg\n", __func__,
                 __LINE__);
            vpu__enc_get_initial_info__free_unpacked(get_init_info_msg_ret, NULL);
            goto ENCODE_CLOSE;
        }

        memcpy(&vpu_config->enc_init_info, get_init_info_msg_ret->initial_info,
               sizeof(Vpu__EncInitialInfo));

        vpu__enc_get_initial_info__free_unpacked(get_init_info_msg_ret, NULL);

        VLOG(ERR, "VPU_EncGetInitialInfo success!\n");
    }


    Vpu__FrameBufferAllocInfo *fb_alloc_info_src = &vpu_config->fb_alloc_info_src;
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
        enc_handle_msg.handle = vpu_config->vpu_handle;

        // vb frame buffer for enc
        register_enc_buf_msg.fb_alloc_info = &fb_alloc_info_msg;

        fb_alloc_info_msg.format = FORMAT_420;

        fb_alloc_info_msg.cbcr_interleave = vpu_config->enc_open_param.cbcr_interleave;
        fb_alloc_info_msg.nv21 = vpu_config->enc_open_param.nv21;

        if (vpu_config->core_idx == VPU_CODA988_IDX)
            fb_alloc_info_msg.map_type = LINEAR_FRAME_MAP;
        else
            fb_alloc_info_msg.map_type = COMPRESSED_FRAME_MAP;

        fb_alloc_info_msg.height = VPU_ALIGN32(vpu_config->enc_open_param.pic_height);
        fb_alloc_info_msg.stride = CalcStride(
            vpu_config->enc_open_param.pic_width, vpu_config->enc_open_param.pic_height, fb_alloc_info_msg.format,
            vpu_config->enc_open_param.cbcr_interleave, fb_alloc_info_msg.map_type, FALSE);

        fb_alloc_info_msg.size = VPU_GetFrameBufSize(
            (VpuHandle)vpu_config->vpu_handle,
            vpu_config->core_idx, fb_alloc_info_msg.stride, fb_alloc_info_msg.height,
            fb_alloc_info_msg.map_type, fb_alloc_info_msg.format, fb_alloc_info_msg.cbcr_interleave,
            NULL);

        fb_alloc_info_msg.luma_bit_depth    = 8; // 8-bit depth
        fb_alloc_info_msg.chroma_bit_depth  = 8;
        fb_alloc_info_msg.num               = vpu_config->enc_init_info.min_frame_buffer_count;
        fb_alloc_info_msg.endian            = vpu_config->enc_open_param.frame_endian;
        fb_alloc_info_msg.type              = FB_TYPE_CODEC;

        fb_alloc_info_msg.has_format        = TRUE;
        fb_alloc_info_msg.has_cbcr_interleave = TRUE;
        fb_alloc_info_msg.has_nv21          = TRUE;
        fb_alloc_info_msg.has_map_type      = TRUE;
        fb_alloc_info_msg.has_height        = TRUE;
        fb_alloc_info_msg.has_stride        = TRUE;
        fb_alloc_info_msg.has_size          = TRUE;
        fb_alloc_info_msg.has_luma_bit_depth    = TRUE;
        fb_alloc_info_msg.has_chroma_bit_depth  = TRUE;
        fb_alloc_info_msg.has_num           = TRUE;
        fb_alloc_info_msg.has_endian        = TRUE;
        fb_alloc_info_msg.has_type          = TRUE;

        VLOG(ERR, "[%s][%d] frame_buffer_count %d, stride %d, height %d, maptype %d size:%d\n",
             __func__, __LINE__, fb_alloc_info_msg.num, fb_alloc_info_msg.stride,
             fb_alloc_info_msg.height, fb_alloc_info_msg.map_type, fb_alloc_info_msg.size);

        uint32_t vb_frame_enc_buf_ptr[fb_alloc_info_msg.num];
        for (int i = 0; i < fb_alloc_info_msg.num; i++) {
            vpu_config->enc_vb_frame[i].size = fb_alloc_info_msg.size;
            if (vdi_allocate_dma_memory(vpu_config->core_idx, &vpu_config->enc_vb_frame[i]) < 0) {
                VLOG(ERR, "%s:%d fail to allocate frame buffer\n", __FUNCTION__, __LINE__);
                goto ENCODE_CLOSE;
            }

            vb_frame_enc_buf_ptr[i] = vpu_config->enc_vb_frame[i].phys_addr;
            VLOG(ERR, "[%s][%d] enc_addr[%d]:%x\n", __func__, __LINE__, i,
                 vb_frame_enc_buf_ptr[i]);
        }

        register_enc_buf_msg.has_vb_frame_buf_size      = TRUE;
        register_enc_buf_msg.vb_frame_buf_size          = fb_alloc_info_msg.size;
        register_enc_buf_msg.n_vb_frame_buf_phys_addr   = fb_alloc_info_msg.num;
        register_enc_buf_msg.vb_frame_buf_phys_addr     = vb_frame_enc_buf_ptr;

        // proto VPU_IPC_ENC_REGISTER_ENCODE_BUFFER msg
        SEND_PROTO_AND_CHECK_RET_SUCCESS(vpu__enc_register_encode_buffer, register_enc_buf_msg,
                                         register_enc_buf_msg_ret,
                                         VPU_IPC_ENC_REGISTER_ENCODE_BUFFER, goto ENCODE_CLOSE);

        vpu__enc_register_encode_buffer__free_unpacked(register_enc_buf_msg_ret, NULL);

        *fb_alloc_info_src = fb_alloc_info_msg;

        VLOG(ERR, "VPU register buffer success!\n");
    }

    {
        // vb buffer for input src, vpu proto internal use
        fb_alloc_info_src->num = 1;
        fb_alloc_info_src->map_type = LINEAR_FRAME_MAP;
        fb_alloc_info_src->stride = CalcStride(
            vpu_config->enc_open_param.pic_width, vpu_config->enc_open_param.pic_height, fb_alloc_info_src->format,
            vpu_config->enc_open_param.cbcr_interleave, fb_alloc_info_src->map_type, FALSE);
        // note: input buffer height != reference buffer height
        fb_alloc_info_src->height = vpu_config->enc_open_param.pic_height;
        fb_alloc_info_src->type = FB_TYPE_PPU;

        fb_alloc_info_src->size = VPU_GetFrameBufSize(
                                    (VpuHandle)vpu_config->vpu_handle,
                                    vpu_config->core_idx, fb_alloc_info_src->stride, fb_alloc_info_src->height,
                                    fb_alloc_info_src->map_type, fb_alloc_info_src->format, fb_alloc_info_src->cbcr_interleave,
            NULL);

        vpu_config->enc_vb_src.size = fb_alloc_info_src->size;
        if (vdi_allocate_dma_memory(vpu_config->core_idx, &vpu_config->enc_vb_src) < 0) {
            VLOG(ERR, "%s:%d fail to allocate frame buffer\n", __FUNCTION__, __LINE__);
            goto ENCODE_CLOSE;
        }

        VLOG(ERR, "[%s][%d] frame_buffer_count %d, stride %d, height %d, maptype %d size:%d\n",
             __func__, __LINE__, fb_alloc_info_src->num, fb_alloc_info_src->stride,
             fb_alloc_info_src->height, fb_alloc_info_src->map_type, fb_alloc_info_src->size);

        vpu_config->fb_src = (Vpu__FrameBuffer)VPU__FRAME_BUFFER__INIT;
        vpu_config->fb_src.buf_y = vpu_config->enc_vb_src.phys_addr;
        vpu_config->fb_src.buf_cb = -1;
        vpu_config->fb_src.buf_cr = -1;
        vpu_config->fb_src.update_fb_info = TRUE;

        ppu_fb_alloc_info_to_fb(vpu_config->vpu_handle, fb_alloc_info_src, &vpu_config->fb_src,
                                vpu_config->enc_init_info.min_frame_buffer_count);
    }

    // VPU_IPC_ENC_GET_HEAD
    {
        Vpu__EncGetHead *enc_get_head_msg_ret   = NULL;
        Vpu__EncGetHead enc_get_head_msg        = (Vpu__EncGetHead)VPU__ENC_GET_HEAD__INIT;
        Vpu__EncHandle enc_handle_msg           = VPU__ENC_HANDLE__INIT;

        enc_get_head_msg.handle = &enc_handle_msg;
        enc_handle_msg.handle   = vpu_config->vpu_handle;

        enc_get_head_msg.has_bitstream_format   = TRUE;
        enc_get_head_msg.bitstream_format       = vpu_config->enc_open_param.bitstream_format;
        enc_get_head_msg.has_input_buf_addr     = TRUE;
        enc_get_head_msg.input_buf_addr         = vpu_config->vb_stream.phys_addr;
        enc_get_head_msg.has_input_buf_size     = TRUE;
        enc_get_head_msg.input_buf_size         = vpu_config->vb_stream.size;

        // proto VPU_IPC_ENC_GET_HEAD msg
        SEND_PROTO_AND_CHECK_RET_SUCCESS(vpu__enc_get_head, enc_get_head_msg,
                                         enc_get_head_msg_ret, VPU_IPC_ENC_GET_HEAD, goto ENCODE_CLOSE);

        if (!enc_get_head_msg_ret->n_output_buf_addr || !enc_get_head_msg_ret->output_buf_addr ||
            !enc_get_head_msg_ret->n_output_buf_size || !enc_get_head_msg_ret->output_buf_size ||
            enc_get_head_msg_ret->n_output_buf_addr != enc_get_head_msg_ret->n_output_buf_size) {
            VLOG(ERR, "[%s][%d] VPU_IPC_ENC_GET_HEAD invailed ret msg\n", __func__, __LINE__);
            vpu__enc_get_head__free_unpacked(enc_get_head_msg_ret, NULL);
            goto ENCODE_CLOSE;
        }

        for (size_t i = 0; i < enc_get_head_msg_ret->n_output_buf_addr; i++) {
            ret = vdi_read_memory(vpu_config->core_idx, enc_get_head_msg_ret->output_buf_addr[i],
                                  vpu_config->enc_output_bs_buf.resource.resource_buffer +
                                      vpu_config->enc_output_bs_buf.resource.resource_size,
                                  enc_get_head_msg_ret->output_buf_size[i],
                                  vpu_config->enc_open_param.stream_endian);
            if (ret < 0) {
                VLOG(ERR, "[%s][%d] vdi_read_memory failed ret:%d\n", __func__, __LINE__, ret);
                vpu__enc_get_head__free_unpacked(enc_get_head_msg_ret, NULL);
                goto ENCODE_CLOSE;
            }

            vpu_config->enc_output_bs_buf.resource.resource_size +=
                enc_get_head_msg_ret->output_buf_size[i];
        }

        for (int i = 0; i < vpu_config->enc_output_bs_buf.resource.resource_size; i++) {
            printf("\t%x", ((char *)vpu_config->enc_output_bs_buf.resource.resource_buffer)[i]);
        }
        printf("\n");
    }

    while (TRUE) {
        // write frame data to frame buf
        ret = write_frame_data_to_src_buf(vpu_config, &vpu_config->enc_vb_src, &vpu_config->fb_src);
        if (ret < 0) {
            VLOG(ERR, "[%s][%d] write_frame_data_to_src_buf error, ret:%d\n", __func__, __LINE__,
                 ret);
            goto ENCODE_CLOSE;
        } else if (ret == 0) {
            VLOG(ERR, "[%s][%d] read all input buf\n", __func__, __LINE__);
            break;
        } else {
            disp_enc_input_buf(vpu_config, &vpu_config->fb_src);

            // VPU_IPC_ENC_ONE_FRAMER
            Vpu__EncOneFrame *enc_one_frame_msg_ret = NULL;
            Vpu__EncOneFrame enc_one_frame_msg = (Vpu__EncOneFrame)VPU__ENC_ONE_FRAME__INIT;
            Vpu__EncParam enc_param_msg = (Vpu__EncParam)VPU__ENC_PARAM__INIT;
            Vpu__EncHandle enc_handle_msg = VPU__ENC_HANDLE__INIT;
            Vpu__EncCodeOpt enc_code_opt_msg = (Vpu__EncCodeOpt)VPU__ENC_CODE_OPT__INIT;

            enc_one_frame_msg.handle = &enc_handle_msg;
            enc_handle_msg.handle = vpu_config->vpu_handle;

            enc_one_frame_msg.enc_param = &enc_param_msg;

            enc_param_msg.source_frame = &vpu_config->fb_src;
            enc_param_msg.quant_param = 10;
            enc_param_msg.force_i_picture = 0;
            enc_param_msg.skip_picture = 0;
            enc_param_msg.pic_stream_buffer_addr = vpu_config->vb_stream.phys_addr;
            enc_param_msg.pic_stream_buffer_size = vpu_config->vb_stream.size;
            enc_param_msg.src_idx = 0;

            enc_param_msg.has_quant_param = TRUE;
            enc_param_msg.has_force_i_picture = TRUE;
            enc_param_msg.has_skip_picture = TRUE;
            enc_param_msg.has_pic_stream_buffer_addr = TRUE;
            enc_param_msg.has_pic_stream_buffer_size = TRUE;
            enc_param_msg.has_src_idx = TRUE;

            enc_param_msg.code_option = &enc_code_opt_msg;
            enc_code_opt_msg.implicit_header_encode = 1;
            enc_code_opt_msg.has_implicit_header_encode = TRUE;

            if (!frame_idx) // set first frame to I frame
                enc_param_msg.force_i_picture = 1;

            // proto VPU_IPC_ENC_ONE_FRAMER msg
            SEND_PROTO_AND_CHECK_RET_SUCCESS(vpu__enc_one_frame, enc_one_frame_msg,
                                             enc_one_frame_msg_ret, VPU_IPC_ENC_ONE_FRAMER,
                                             goto ENCODE_CLOSE);

            if (!enc_one_frame_msg_ret->enc_output_info ||
                !enc_one_frame_msg_ret->enc_output_info->bitstream_size ||
                !enc_one_frame_msg_ret->enc_output_info->bitstream_buffer) {
                VLOG(ERR, "[%s][%d] VPU_IPC_ENC_ONE_FRAMER invailed ret msg\n", __func__, __LINE__);
                VLOG(ERR, "======%d\n", enc_param_msg.pic_stream_buffer_size);
                VLOG(ERR, "%p %d %p\n", enc_one_frame_msg_ret->enc_output_info,
                    enc_one_frame_msg_ret->enc_output_info->bitstream_size,
                    enc_one_frame_msg_ret->enc_output_info->bitstream_buffer);

                vpu__enc_one_frame__free_unpacked(enc_one_frame_msg_ret, NULL);
                goto ENCODE_CLOSE;
            }

            if (vpu_config->enc_output_bs_buf.resource.resource_size +
                    enc_one_frame_msg_ret->enc_output_info->bitstream_size >
                VPU_ENC_OUTPUT_BS_BUFFER_SIZE) {
                VLOG(ERR,
                     "[%s][%d] output bit streamer buffer is full, bs buffer size:%d, filled "
                     "size:%d, coming size:%d \n",
                     __func__, __LINE__, VPU_ENC_OUTPUT_BS_BUFFER_SIZE,
                     vpu_config->enc_output_bs_buf.resource.resource_size,
                     enc_one_frame_msg_ret->enc_output_info->bitstream_size);
                vpu__enc_one_frame__free_unpacked(enc_one_frame_msg_ret, NULL);
                goto ENCODE_CLOSE;
            }

            ret = vdi_read_memory(vpu_config->core_idx,
                                  enc_one_frame_msg_ret->enc_output_info->bitstream_buffer,
                                  vpu_config->enc_output_bs_buf.resource.resource_buffer +
                                      vpu_config->enc_output_bs_buf.resource.resource_size,
                                  enc_one_frame_msg_ret->enc_output_info->bitstream_size,
                                  vpu_config->enc_open_param.stream_endian);
            if (ret < 0) {
                VLOG(ERR, "[%s][%d] vdi_read_memory failed ret:%d\n", __func__, __LINE__, ret);
                vpu__enc_one_frame__free_unpacked(enc_one_frame_msg_ret, NULL);
                goto ENCODE_CLOSE;
            }

            vpu_config->enc_output_bs_buf.resource.resource_size +=
                enc_one_frame_msg_ret->enc_output_info->bitstream_size;
            VLOG(ERR, "[%s][%d] VPU_IPC_ENC_ONE_FRAMER enc size:%d pic_type:%d\n", __func__,
                 __LINE__, enc_one_frame_msg_ret->enc_output_info->bitstream_size,
                 enc_one_frame_msg_ret->enc_output_info->pic_type);
            vpu__enc_one_frame__free_unpacked(enc_one_frame_msg_ret, NULL);

            frame_idx++;
        }
    }

    VLOG(INFO, "[%s][%d] enc over, h265 stream size:%d\n", __func__, __LINE__,
         vpu_config->enc_output_bs_buf.resource.resource_size);

ENCODE_CLOSE:
    // VPU_EncClose
    {
        Vpu__EncClose enc_close_msg = VPU__ENC_CLOSE__INIT;
        Vpu__EncHandle enc_handle_msg = VPU__ENC_HANDLE__INIT;

        enc_close_msg.handle = &enc_handle_msg;
        enc_handle_msg.handle = vpu_config->vpu_handle;

        // proto msg
        msg_len = vpu__enc_close__get_packed_size(&enc_close_msg);
        vpu__enc_close__pack(&enc_close_msg, vpu_config->msg_buf);

        ret = vpu_send_msg_to_service(&vpu_config->pack, VPU_IPC_ENC_CLOSE, vpu_config->msg_buf,
                                      msg_len);
        if (ret < 0) {
            VLOG(ERR, "VPU_EncClose failed\n");
        }
    }

    for (int i = 0; i < MAX_REG_FRAME; i++) {
        if (vpu_config->enc_vb_frame[i].size > 0)
            vdi_free_dma_memory(vpu_config->core_idx, &vpu_config->enc_vb_frame[i]);
    }

    if (vpu_config->enc_vb_src.size > 0)
        vdi_free_dma_memory(vpu_config->core_idx, &vpu_config->enc_vb_src);

ERR_ENC_INIT:
    if (vpu_config->vb_stream.size > 0) {
        vdi_free_dma_memory(vpu_config->core_idx, &vpu_config->vb_stream);
    }

    if (vpu_config->enc_resource.resource.resource_buffer)
        free(vpu_config->enc_resource.resource.resource_buffer);

    if (vpu_config->enc_output_bs_buf.resource.resource_buffer) {
        free(vpu_config->enc_output_bs_buf.resource.resource_buffer);
    }

    if (vpu_config->msg_buf)
        free(vpu_config->msg_buf);

    if (vpu_config->display)
        sdm_clear_display(vpu_config->display->handle);

    if (vpu_config)
        free(vpu_config);

    return ret;
}

int32_t vpu_test_task(int32_t argc, char *argv[])
{
    printf("vpu_test_task start\n");

    if (argc < 2) {
        vpu_test_decoder_h264();
    } else {
        if (argv[1][0] == '0') {
            vpu_test_decoder_h264();
        } else if (argv[1][0] == '1') {
            vpu_test_decoder_h265();
        } else if (argv[1][0] == '2') {
            vpu_test_encoder_h264();
        } else if (argv[1][0] == '3') {
            vpu_test_encoder_h265();
        } else {
            printf("Error: Invalid command!\n");
        }
    }

    printf("vpu_test_task end\n");
    return 0;
}

CLI_CMD("vpu_test", "vpu_test [0/1/2];\n\t\t0:h264_dec, 1:h265_dec, 2:h264_enc", vpu_test_task);
