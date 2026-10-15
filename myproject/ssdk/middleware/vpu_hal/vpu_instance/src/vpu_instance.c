#include <arch/cache.h>
#include <assert.h>
#include <cmsis_os2.h>
#include <driver/irq.h>
#include <driver/memmap.h>
#include <getopt.h>
#include <irq_num.h>
#include <malloc.h>
#include <param.h>
#include <reg.h>
#include <regs_base.h>
#include <res_loader.h>
#include <scr_cfg.h>
#include <scr_hw.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>

#include "image_cfg.h"
#include "vpuapifunc.h"

#include "vpu.pb-c.h"
#include "vpu_instance.h"

// #include "coda980.h"
// #include "pissarro.h"

#define VPU_CODA988_IDX 0
#define VPU_WAVE412_IDX 1
#define VPU_WAVE521_IDX 2

#define VPU_CODA988_REG_RES RES_VPU_VPU2
#define VPU_WAVE412_REG_RES RES_VPU_VPU1

#define VPU_RES_LOAD_ALIGN_SIZE 32

#define VPU_REG_SIZE 0x4000
#define VPU_STREAM_BUF_SIZE 0x100000

#define VPU_PROTO_BUFFER_SIZE 4096

#define VPU_ENC_MAX_HEAD_COUNT 5
#define VPU_ENC_EVERY_HEAD_SIZE 2048

#define VPU_MAX_NUM_INSTANCE 8

// 4k 90fps decode
#define VPU_WAVE521_CODEC_ABILITY (4096 * 2304 * 90)
#define VPU_WAVE521_DEC_AVC_ABILITY_COMPENSATION_VALUE (1.25)
// 4K 70fps decode
#define VPU_WAVE412_CODEC_ABILITY (4096 * 2304 * 70)
// 4K 30fps decode & enable sram
#define VPU_CODA988_CODEC_ABILITY (4096 * 2304 * 30 * 1 * 1)
#define VPU_CODA988_ENABLE_ABILITY_COMPENSATION_VALUE (30 / 20)
#define VPU_CODA988_DISABLE_SRAM_ABILITY_COMPENSATION_VALUE (1.25)

// #define VPU_CODA988_FIRMWARE_PATH "early_app/video/coda980.out"
#define VPU_CODA988_FIRMWARE_PATH "early_app/video/coda988safety.bin"
#define VPU_WAVE412_FIRMWARE_PATH "early_app/video/pissarro.bin"
#define VPU_WAVE521_FIRMWARE_PATH "early_app/video/chagall.bin"

typedef enum {
    DEC_INT_STATUS_NONE,    // Interrupt not asserted yet
    DEC_INT_STATUS_EMPTY,   // Need more es
    DEC_INT_STATUS_DONE,    // Interrupt asserted
    DEC_INT_STATUS_TIMEOUT, // Interrupt not asserted during given time.
} DEC_INT_STATUS;           // for wave521

typedef enum {
    ENC_INT_STATUS_NONE, // Interrupt not asserted yet
    ENC_INT_STATUS_FULL, // Need more buffer
    ENC_INT_STATUS_DONE, // Interrupt asserted
    ENC_INT_STATUS_LOW_LATENCY,
    ENC_INT_STATUS_TIMEOUT, // Interrupt not asserted during given time.
    ENC_INT_STATUS_SRC_RELEASED,
} ENC_INT_STATUS; // for wave521

typedef struct file_resource {
    const char *resource_path;
    int resource_size;
    void *resource_buffer;
} file_resource_t;

typedef struct vpu_info {
    Uint32 product_id;
    Uint32 version;
    Uint32 revision;

    // dec
    DecHandle handle;
    DecParam dec_param;
    FrameBuffer frame_buffers[MAX_REG_FRAME];
    DecInitialInfo sequence_info;
    DecOutputInfo dec_output_info;

    // enc
    FrameBuffer fb_recon[MAX_REG_FRAME];
    EncParam enc_param;
    EncOutputInfo enc_output_info;
} vpu_info_t;

typedef struct vpu_proto_info {
    // seq init
    Vpu__VpuRect init_pic_crop_rect;
    Vpu__AvcVuiInfo init_avc_vui_info;
    Vpu__DecHevcInfo init_hevc_info;
    Vpu__DecHevcInfo__H265Info init_hevc_h265_info;
    Vpu__DecInitialInfo init_output_info;
    Vpu__DecSeqInit init_seq_msg_ret;

    // frame buffer

    // dec output info
    Vpu__VpuRect dec_rc_display;
    Vpu__DecAvcInfo dec_avc_info;
    Vpu__AvcVuiInfo dec_avc_vui_info;
    Vpu__DecHevcInfo dec_hevc_info;
    Vpu__DecHevcInfo__H265Info dec_hevc_h265_info;
    Vpu__FrameBuffer disp_frame;
    Vpu__DecOutputInfo dec_output_info;
    Vpu__DecOneFrame dec_frame_msg_ret;

    // enc get init info
    Vpu__EncGetInitialInfo enc_init_info_msg_ret;
    Vpu__EncInitialInfo enc_init_info;

    // enc get head
    uint32_t enc_head_output_buf_addr[VPU_ENC_MAX_HEAD_COUNT];
    uint32_t enc_head_output_buf_size[VPU_ENC_MAX_HEAD_COUNT];
    Vpu__EncGetHead enc_get_head_msg_ret;

    // enc one frame
    Vpu__EncOneFrame enc_frame_msg_ret;
    Vpu__EncOutputInfo enc_output_info;
} vpu_proto_info_t;

typedef struct vpu_instance_contrl {
    uint32_t inst_handle[VPU_MAX_NUM_INSTANCE];
    uint32_t inst_score[VPU_MAX_NUM_INSTANCE];
    uint32_t inst_enable_sram[VPU_MAX_NUM_INSTANCE];
    uint32_t codec_ability_score;
} vpu_instance_contrl_t;

typedef struct vpu_instance {
    int core_idx;
    unsigned long reg_phy_base;
    unsigned int reg_range;

    vpu_info_t vpu_info;
    vpu_proto_info_t vpu_proto_info;
    vpu_instance_contrl_t vpu_instance_contrl;

    file_resource_t fw_resource;
    void *msg_buf;
} vpu_instance_t;

vpu_instance_t *vpu_instance_coda988 = NULL;
vpu_instance_t *vpu_instance_wave412 = NULL;
vpu_instance_t *vpu_instance_wave521 = NULL;

int vpu_instance_init_coda988(void)
{
    int ret = 0;
#ifdef CONFIG_BOARD_X9SP
    scr_signal_t coda988_scr_phy = SCR_SEC__coda988_linebuffer_cfg_1_0;
#else  /* !CONFIG_BOARD_X9SP */
    scr_signal_t coda988_scr_phy = SCR_SEC__vpu2_i_linebuffer_cfg_1_0;
#endif /* end CONFIG_BOARD_X9SP */

    VLOG(ERR, "%s: %d\n", __func__, __LINE__);
    if (vpu_instance_coda988)
        return 0;

    vpu_instance_coda988 = osal_malloc(sizeof(vpu_instance_t));

    if (!vpu_instance_coda988) {
        VLOG(ERR, "%s: %d malloc failed\n", __func__, __LINE__);
        return -1;
    }

    osal_memset(vpu_instance_coda988, 0x00, sizeof(vpu_instance_t));

    // msg buf
    vpu_instance_coda988->msg_buf = osal_malloc(VPU_PROTO_BUFFER_SIZE);

    {
        // load fireware
        vpu_instance_coda988->fw_resource.resource_path = VPU_CODA988_FIRMWARE_PATH;
        vpu_instance_coda988->fw_resource.resource_size =
            res_size(vpu_instance_coda988->fw_resource.resource_path);
        vpu_instance_coda988->fw_resource.resource_buffer = memalign(
            VPU_RES_LOAD_ALIGN_SIZE,
            ROUNDUP(vpu_instance_coda988->fw_resource.resource_size, VPU_RES_LOAD_ALIGN_SIZE));

        ret = res_load(
            vpu_instance_coda988->fw_resource.resource_path,
            vpu_instance_coda988->fw_resource.resource_buffer,
            ROUNDUP(vpu_instance_coda988->fw_resource.resource_size, VPU_RES_LOAD_ALIGN_SIZE), 0);
        if (ret < 0) {
            VLOG(ERR, "%s: %d coda988 load firmware failed\n", __func__, __LINE__);
            free(vpu_instance_coda988);
            return -1;
        }

        // load form .h file
        // vpu_instance_coda988->fw_resource.resource_size = sizeof(coda988_bit_code);
        // vpu_instance_coda988->fw_resource.resource_buffer = (void *)coda988_bit_code;
    }

    {
        // get product id for test vpu is working
        vpu_instance_coda988->core_idx = VPU_CODA988_IDX;
        vpu_instance_coda988->reg_range = VPU_REG_SIZE;
        vpu_instance_coda988->reg_phy_base = APB_VPU2_BASE;
        vpu_instance_coda988->vpu_info.product_id =
            VPU_GetProductId(vpu_instance_coda988->reg_phy_base, vpu_instance_coda988->reg_range,
                             vpu_instance_coda988->core_idx);

        if ((int)vpu_instance_coda988->vpu_info.product_id < 0) {
            VLOG(ERR, "Failed to get product ID\n");
            goto ERR_DEC_INIT;
        }

        // load firmware to vpu
        ret = VPU_InitWithBitcode(
            vpu_instance_coda988->core_idx,
            (const uint16_t *)vpu_instance_coda988->fw_resource.resource_buffer,
            vpu_instance_coda988->fw_resource.resource_size / 2, vpu_instance_coda988->reg_phy_base,
            vpu_instance_coda988->reg_range);
        if (ret != RETCODE_CALLED_BEFORE && ret != RETCODE_SUCCESS) {
            VLOG(ERR,
                 "Failed to boot up VPU(coreIdx: %d, productId: %d, fw buffer:%p, fw size:%d)\n",
                 vpu_instance_coda988->core_idx, vpu_instance_coda988->vpu_info.product_id,
                 vpu_instance_coda988->fw_resource.resource_buffer,
                 vpu_instance_coda988->fw_resource.resource_size);

            goto ERR_DEC_INIT;
        }

        // print vpu version info
        VPU_GetVersionInfo(vpu_instance_coda988->core_idx, &vpu_instance_coda988->vpu_info.version,
                           &vpu_instance_coda988->vpu_info.revision,
                           &vpu_instance_coda988->vpu_info.product_id);
        VLOG(INFO, "VPU coreNum : [%d]\n", vpu_instance_coda988->core_idx);
        VLOG(INFO, "Firmware : CustomerCode: %04x | version : %d.%d.%d rev.%d\n",
             (Uint32)(vpu_instance_coda988->vpu_info.version >> 16),
             (Uint32)((vpu_instance_coda988->vpu_info.version >> (12)) & 0x0f),
             (Uint32)((vpu_instance_coda988->vpu_info.version >> (8)) & 0x0f),
             (Uint32)((vpu_instance_coda988->vpu_info.version) & 0xff),
             vpu_instance_coda988->vpu_info.revision);
        VLOG(INFO, "Hardware : %04x\n", vpu_instance_coda988->vpu_info.product_id);
    }

    // set coda988 sec mode to line buffer for enc
    ret = scr_set(&g_scr_sec, &coda988_scr_phy, MODE_SRAM_LINEBUFFER);
    if (0 == ret) {
        VLOG(ERR, "[%s][%d] set coda988 sram mode failed\n", __func__, __LINE__);
        goto ERR_DEC_INIT;
    }

    vpu_instance_coda988->vpu_instance_contrl.codec_ability_score = VPU_CODA988_CODEC_ABILITY;

    return 0;

ERR_DEC_INIT:
    VPU_DeInit(vpu_instance_coda988->core_idx);

    if (vpu_instance_coda988->fw_resource.resource_buffer)
        free(vpu_instance_coda988->fw_resource.resource_buffer);

    if (vpu_instance_coda988->msg_buf)
        free(vpu_instance_coda988->msg_buf);

    if (vpu_instance_coda988) {
        free(vpu_instance_coda988);
        vpu_instance_coda988 = NULL;
    }

    return -1;
}

void vpu_instance_deinit_coda988(void)
{
    if (!vpu_instance_coda988)
        return;

    VPU_DeInit(vpu_instance_coda988->core_idx);

    if (vpu_instance_coda988->fw_resource.resource_buffer)
        free(vpu_instance_coda988->fw_resource.resource_buffer);

    if (vpu_instance_coda988->msg_buf)
        free(vpu_instance_coda988->msg_buf);

    if (vpu_instance_coda988) {
        free(vpu_instance_coda988);
        vpu_instance_coda988 = NULL;
    }
}

int vpu_instance_init_wave412(void)
{
    int ret = 0;

    VLOG(ERR, "%s: %d\n", __func__, __LINE__);
    if (vpu_instance_wave412)
        return 0;

    vpu_instance_wave412 = osal_malloc(sizeof(vpu_instance_t));

    if (!vpu_instance_wave412) {
        VLOG(ERR, "%s: %d malloc failed\n", __func__, __LINE__);
        return -1;
    }

    osal_memset(vpu_instance_wave412, 0x00, sizeof(vpu_instance_t));

    // msg buf
    vpu_instance_wave412->msg_buf = osal_malloc(VPU_PROTO_BUFFER_SIZE);

    {
        // load fireware
        vpu_instance_wave412->fw_resource.resource_path = VPU_WAVE412_FIRMWARE_PATH;
        vpu_instance_wave412->fw_resource.resource_size =
            res_size(vpu_instance_wave412->fw_resource.resource_path);
        vpu_instance_wave412->fw_resource.resource_buffer = memalign(
            VPU_RES_LOAD_ALIGN_SIZE,
            ROUNDUP(vpu_instance_wave412->fw_resource.resource_size, VPU_RES_LOAD_ALIGN_SIZE));

        ret = res_load(
            vpu_instance_wave412->fw_resource.resource_path,
            vpu_instance_wave412->fw_resource.resource_buffer,
            ROUNDUP(vpu_instance_wave412->fw_resource.resource_size, VPU_RES_LOAD_ALIGN_SIZE), 0);
        if (ret < 0) {
            VLOG(ERR, "%s: %d wave412 load firmware failed\n", __func__, __LINE__);
            free(vpu_instance_wave412);
            return -1;
        }

        // load form .h file
        // vpu_instance_wave412->fw_resource.resource_size = sizeof(wave412_bit_code);
        // vpu_instance_wave412->fw_resource.resource_buffer = (void *)wave412_bit_code;
    }

    {
        // get product id for test vpu is working
        vpu_instance_wave412->core_idx = VPU_WAVE412_IDX;
        vpu_instance_wave412->reg_range = VPU_REG_SIZE;
        vpu_instance_wave412->reg_phy_base = APB_VPU1_BASE;
        vpu_instance_wave412->vpu_info.product_id =
            VPU_GetProductId(vpu_instance_wave412->reg_phy_base, vpu_instance_wave412->reg_range,
                             vpu_instance_wave412->core_idx);

        if ((int)vpu_instance_wave412->vpu_info.product_id < 0) {
            VLOG(ERR, "Failed to get product ID\n");
            goto ERR_DEC_INIT;
        }

        // load firmware to vpu
        ret = VPU_InitWithBitcode(
            vpu_instance_wave412->core_idx,
            (const uint16_t *)vpu_instance_wave412->fw_resource.resource_buffer,
            vpu_instance_wave412->fw_resource.resource_size / 2, vpu_instance_wave412->reg_phy_base,
            vpu_instance_wave412->reg_range);
        if (ret != RETCODE_CALLED_BEFORE && ret != RETCODE_SUCCESS) {
            VLOG(ERR,
                 "Failed to boot up VPU(coreIdx: %d, productId: %d, fw buffer:%p, fw size:%d)\n",
                 vpu_instance_wave412->core_idx, vpu_instance_wave412->vpu_info.product_id,
                 vpu_instance_wave412->fw_resource.resource_buffer,
                 vpu_instance_wave412->fw_resource.resource_size);

            goto ERR_DEC_INIT;
        }

        // print vpu version info
        VPU_GetVersionInfo(vpu_instance_wave412->core_idx, &vpu_instance_wave412->vpu_info.version,
                           &vpu_instance_wave412->vpu_info.revision,
                           &vpu_instance_wave412->vpu_info.product_id);
        VLOG(INFO, "VPU coreNum : [%d]\n", vpu_instance_wave412->core_idx);
        VLOG(INFO, "Firmware : CustomerCode: %04x | version : %d.%d.%d rev.%d\n",
             (Uint32)(vpu_instance_wave412->vpu_info.version >> 16),
             (Uint32)((vpu_instance_wave412->vpu_info.version >> (12)) & 0x0f),
             (Uint32)((vpu_instance_wave412->vpu_info.version >> (8)) & 0x0f),
             (Uint32)((vpu_instance_wave412->vpu_info.version) & 0xff),
             vpu_instance_wave412->vpu_info.revision);
        VLOG(INFO, "Hardware : %04x\n", vpu_instance_wave412->vpu_info.product_id);
    }

    vpu_instance_wave412->vpu_instance_contrl.codec_ability_score = VPU_WAVE412_CODEC_ABILITY;

    return 0;

ERR_DEC_INIT:
    VPU_DeInit(vpu_instance_wave412->core_idx);

    if (vpu_instance_wave412->fw_resource.resource_buffer)
        free(vpu_instance_wave412->fw_resource.resource_buffer);

    if (vpu_instance_wave412->msg_buf)
        free(vpu_instance_wave412->msg_buf);

    if (vpu_instance_wave412) {
        free(vpu_instance_wave412);
        vpu_instance_wave412 = NULL;
    }

    return -1;
}

void vpu_instance_deinit_wave412(void)
{
    if (!vpu_instance_wave412)
        return;

    VPU_DeInit(vpu_instance_wave412->core_idx);

    if (vpu_instance_wave412->fw_resource.resource_buffer)
        free(vpu_instance_wave412->fw_resource.resource_buffer);

    if (vpu_instance_wave412->msg_buf)
        free(vpu_instance_wave412->msg_buf);

    if (vpu_instance_wave412) {
        free(vpu_instance_wave412);
        vpu_instance_wave412 = NULL;
    }
}

#ifdef CONFIG_BOARD_X9SP
int vpu_instance_init_wave521(void)
{
    int ret = 0;

    VLOG(ERR, "%s: %d\n", __func__, __LINE__);
    if (vpu_instance_wave521)
        return 0;

    vpu_instance_wave521 = osal_malloc(sizeof(vpu_instance_t));

    if (!vpu_instance_wave521) {
        VLOG(ERR, "%s: %d malloc failed\n", __func__, __LINE__);
        return -1;
    }

    osal_memset(vpu_instance_wave521, 0x00, sizeof(vpu_instance_t));

    // msg buf
    vpu_instance_wave521->msg_buf = osal_malloc(VPU_PROTO_BUFFER_SIZE);

    {
        // load fireware
        vpu_instance_wave521->fw_resource.resource_path = VPU_WAVE521_FIRMWARE_PATH;
        vpu_instance_wave521->fw_resource.resource_size =
            res_size(vpu_instance_wave521->fw_resource.resource_path);
        vpu_instance_wave521->fw_resource.resource_buffer = memalign(
            VPU_RES_LOAD_ALIGN_SIZE,
            ROUNDUP(vpu_instance_wave521->fw_resource.resource_size, VPU_RES_LOAD_ALIGN_SIZE));

        ret = res_load(
            vpu_instance_wave521->fw_resource.resource_path,
            vpu_instance_wave521->fw_resource.resource_buffer,
            ROUNDUP(vpu_instance_wave521->fw_resource.resource_size, VPU_RES_LOAD_ALIGN_SIZE), 0);
        if (ret < 0) {
            VLOG(ERR, "%s: %d wave521 load firmware failed\n", __func__, __LINE__);
            free(vpu_instance_wave521);
            return -1;
        }

        // load form .h file
        // vpu_instance_wave521->fw_resource.resource_size = sizeof(wave521_bit_code);
        // vpu_instance_wave521->fw_resource.resource_buffer = (void *)wave521_bit_code;
    }

    {
        // get product id for test vpu is working
        vpu_instance_wave521->core_idx = VPU_WAVE521_IDX;
        vpu_instance_wave521->reg_range = VPU_REG_SIZE;
        vpu_instance_wave521->reg_phy_base = APB_VPU4_BASE;
        vpu_instance_wave521->vpu_info.product_id =
            VPU_GetProductId(vpu_instance_wave521->reg_phy_base, vpu_instance_wave521->reg_range,
                             vpu_instance_wave521->core_idx);

        if ((int)vpu_instance_wave521->vpu_info.product_id < 0) {
            VLOG(ERR, "Failed to get product ID\n");
            goto ERR_DEC_INIT;
        }

        // load firmware to vpu
        ret = VPU_InitWithBitcode(
            vpu_instance_wave521->core_idx,
            (const uint16_t *)vpu_instance_wave521->fw_resource.resource_buffer,
            vpu_instance_wave521->fw_resource.resource_size / 2, vpu_instance_wave521->reg_phy_base,
            vpu_instance_wave521->reg_range);
        if (ret != RETCODE_CALLED_BEFORE && ret != RETCODE_SUCCESS) {
            VLOG(ERR,
                 "Failed to boot up VPU(coreIdx: %d, productId: %d, fw buffer:%p, fw size:%d)\n",
                 vpu_instance_wave521->core_idx, vpu_instance_wave521->vpu_info.product_id,
                 vpu_instance_wave521->fw_resource.resource_buffer,
                 vpu_instance_wave521->fw_resource.resource_size);

            goto ERR_DEC_INIT;
        }

        // print vpu version info
        VPU_GetVersionInfo(vpu_instance_wave521->core_idx, &vpu_instance_wave521->vpu_info.version,
                           &vpu_instance_wave521->vpu_info.revision,
                           &vpu_instance_wave521->vpu_info.product_id);
        VLOG(INFO, "VPU coreNum : [%d]\n", vpu_instance_wave521->core_idx);
        VLOG(INFO, "Firmware : CustomerCode: %04x | version : %d.%d.%d rev.%d\n",
             (Uint32)(vpu_instance_wave521->vpu_info.version >> 16),
             (Uint32)((vpu_instance_wave521->vpu_info.version >> (12)) & 0x0f),
             (Uint32)((vpu_instance_wave521->vpu_info.version >> (8)) & 0x0f),
             (Uint32)((vpu_instance_wave521->vpu_info.version) & 0xff),
             vpu_instance_wave521->vpu_info.revision);
        VLOG(INFO, "Hardware : %04x\n", vpu_instance_wave521->vpu_info.product_id);
    }

    vpu_instance_wave521->vpu_instance_contrl.codec_ability_score = VPU_WAVE521_CODEC_ABILITY;

    return 0;

ERR_DEC_INIT:
    VPU_DeInit(vpu_instance_wave521->core_idx);

    if (vpu_instance_wave521->fw_resource.resource_buffer)
        free(vpu_instance_wave521->fw_resource.resource_buffer);

    if (vpu_instance_wave521->msg_buf)
        free(vpu_instance_wave521->msg_buf);

    if (vpu_instance_wave521) {
        free(vpu_instance_wave521);
        vpu_instance_wave521 = NULL;
    }

    return -1;
}

void vpu_instance_deinit_wave521(void)
{
    if (!vpu_instance_wave521)
        return;

    VPU_DeInit(vpu_instance_wave521->core_idx);

    if (vpu_instance_wave521->fw_resource.resource_buffer)
        free(vpu_instance_wave521->fw_resource.resource_buffer);

    if (vpu_instance_wave521->msg_buf)
        free(vpu_instance_wave521->msg_buf);

    if (vpu_instance_wave521) {
        free(vpu_instance_wave521);
        vpu_instance_wave521 = NULL;
    }
}
#else  /* !CONFIG_BOARD_X9SP */
int vpu_instance_init_wave521(void) { return 0; }
void vpu_instance_deinit_wave521(void) { return; }
#endif /* end CONFIG_BOARD_X9SP */

int vpu_instance_calculate_fps(DecHandle handle, Vpu__DecInitialInfo *output_info_msg)
{
    int fps = 0;

    // frame rate
    if (output_info_msg->f_rate_denominator <= 0)
        fps = 0;
    else {
        switch (handle->codecMode) {
        case AVC_DEC: // AVC_DEC & HEVC_DEC use the same enum
        case AVS_DEC:
        case MP2_DEC:
        case MP4_DEC:
        case RV_DEC:
        case VPX_DEC:
            fps = output_info_msg->f_rate_numerator / output_info_msg->f_rate_denominator;
            break;
        case VC1_DEC:
            if (output_info_msg->profile == 2 && output_info_msg->f_rate_numerator < 8) {
                const int TBL_FRAME_RATE_NR[8] = {0, 24, 25, 30, 50, 60, 48, 72};
                fps = (TBL_FRAME_RATE_NR[output_info_msg->f_rate_numerator] * 100) /
                      (output_info_msg->f_rate_denominator + 999);
            } else {
                fps = output_info_msg->f_rate_numerator + 1 / output_info_msg->f_rate_denominator;
            }
            break;
        default:
            fps = 0;
            break;
        }
    }

    if (fps > 120)
        fps = 0; // it means we bypass the timestamp of inputbuffer to outpubuffer
                 // without timestamp corretion logic.

    return fps;
}

static DEC_INT_STATUS wave5_dec_hand_interrupt(DecHandle handle)
{
    Int32 interruptFlag = 0;
    DEC_INT_STATUS status = DEC_INT_STATUS_NONE;

    do {
        interruptFlag = VPU_WaitInterruptEx(handle, VPU_DEC_TIMEOUT);
        if (INTERRUPT_TIMEOUT_VALUE == interruptFlag) {
            VLOG(ERR, "[%s][%d] VPU_WaitInterruptEx timeout\n", __func__, __LINE__);
            status = DEC_INT_STATUS_TIMEOUT;
            break;
        }

        if (interruptFlag < 0) {
            VLOG(ERR, "[%s][%d] interruptFlag is negative value! %08x\n", __func__, __LINE__,
                 interruptFlag);
        }

        if (interruptFlag > 0) {
            VPU_ClearInterruptEx(handle, interruptFlag);
            status = DEC_INT_STATUS_DONE;
            if (interruptFlag & (1 << INT_WAVE5_INIT_SEQ)) {
                break;
            }

            if (interruptFlag & (1 << INT_WAVE5_DEC_PIC)) {
                break;
            }

            if (interruptFlag & (1 << INT_WAVE5_BSBUF_EMPTY)) {
                status = DEC_INT_STATUS_EMPTY;
                break;
            }
        }
    } while (FALSE);

    return status;
}

static ENC_INT_STATUS wave5_enc_hand_interrupt(EncHandle handle)
{
    Int32 interruptFlag = 0;
    ENC_INT_STATUS status = ENC_INT_STATUS_NONE;

    do {
        interruptFlag = VPU_WaitInterruptEx(handle, VPU_ENC_TIMEOUT);
        if (INTERRUPT_TIMEOUT_VALUE == interruptFlag) {
            VLOG(ERR, "[%s][%d] VPU_WaitInterruptEx timeout\n", __func__, __LINE__);
            status = ENC_INT_STATUS_TIMEOUT;
            break;
        }

        if (interruptFlag < 0) {
            VLOG(ERR, "[%s][%d] interruptFlag is negative value! %08x\n", __func__, __LINE__,
                 interruptFlag);
        }

        if (interruptFlag > 0) {
            VPU_ClearInterruptEx(handle, interruptFlag);

            if (interruptFlag & (1 << INT_WAVE5_ENC_SET_PARAM)) {
                status = ENC_INT_STATUS_DONE;
                break;
            }

            if (interruptFlag & (1 << INT_WAVE5_ENC_PIC)) {
                status = ENC_INT_STATUS_DONE;
                break;
            }

            if (interruptFlag & (1 << INT_WAVE5_BSBUF_FULL)) {
                status = ENC_INT_STATUS_FULL;
                break;
            }
        }
    } while (FALSE);

    return status;
}

int vpu_instance_ipc_dec_open(vpu_instance_t *instance, Vpu__DecOpen *dec_open_msg)
{
    int ret = 0;
    int index = 0;
    DecHandle handle = NULL;
    DecOpenParam dec_open_param = {0};
    SecAxiUse sec_axi_use = {0};
    Vpu__DecOpenParam *dec_op = NULL;

    if (!dec_open_msg->handle || !dec_open_msg->dec_op) {
        VLOG(ERR, "[%s][%d] invailed handle\n", __func__, __LINE__);
        return -1;
    }

    for (; index < VPU_MAX_NUM_INSTANCE; index++) {
        if (instance->vpu_instance_contrl.inst_handle[index] == 0)
            break;
    }

    if (index >= VPU_MAX_NUM_INSTANCE) {
        VLOG(ERR, "[%s][%d] the maximum number:%d of open instances has been reached\n", __func__,
             __LINE__, VPU_MAX_NUM_INSTANCE);

        dec_open_msg->has_ret_code = TRUE;
        dec_open_msg->ret_code = RETCODE_RESOURECE_CONSTRAINTS;
        return 0;
    }

    dec_op = dec_open_msg->dec_op;

    {
        // dec open
        if (dec_op->has_bitstream_format)
            dec_open_param.bitstreamFormat = dec_op->bitstream_format;
        if (dec_op->has_avc_extension)
            dec_open_param.avcExtension = dec_op->avc_extension;
        if (dec_op->has_core_idx)
            dec_open_param.coreIdx = dec_op->core_idx;
        if (dec_op->has_bitstream_buffer)
            dec_open_param.bitstreamBuffer = dec_op->bitstream_buffer;
        if (dec_op->has_bitstream_buffer_size)
            dec_open_param.bitstreamBufferSize = dec_op->bitstream_buffer_size;
        if (dec_op->has_bitstream_mode)
            dec_open_param.bitstreamMode = dec_op->bitstream_mode;
        if (dec_op->has_tiled2_linear_enable)
            dec_open_param.tiled2LinearEnable = dec_op->tiled2_linear_enable;
        if (dec_op->has_tiled2_linear_mode)
            dec_open_param.tiled2LinearMode = dec_op->tiled2_linear_mode;
        if (dec_op->has_wtl_enable)
            dec_open_param.wtlEnable = dec_op->wtl_enable;
        if (dec_op->has_wtl_mode)
            dec_open_param.wtlMode = dec_op->wtl_mode;
        if (dec_op->has_cbcr_interleave)
            dec_open_param.cbcrInterleave = dec_op->cbcr_interleave;
        if (dec_op->has_nv21)
            dec_open_param.nv21 = dec_op->nv21;
        if (dec_op->has_stream_endian)
            dec_open_param.streamEndian = dec_op->stream_endian;
        if (dec_op->has_frame_endian)
            dec_open_param.frameEndian = dec_op->frame_endian;
        if (dec_op->has_bwb_enable)
            dec_open_param.bwbEnable = dec_op->bwb_enable;
        if (dec_op->has_mp4_deblk_enable)
            dec_open_param.mp4DeblkEnable = dec_op->mp4_deblk_enable;
        if (dec_op->has_mp4_class)
            dec_open_param.mp4Class = dec_op->mp4_class;
        if (dec_op->has_pvric_fbc_enable)
            dec_open_param.pvricFbcEnable = dec_op->pvric_fbc_enable;
        if (dec_op->has_fbc_mode)
            dec_open_param.fbc_mode = dec_op->fbc_mode;
        if (dec_op->has_bw_optimization)
            dec_open_param.bwOptimization = dec_op->bw_optimization;
        if (dec_op->has_pvric31_hw_support)
            dec_open_param.pvric31HwSupport = dec_op->pvric31_hw_support;
        if (dec_op->has_enable_non_ref_fbc_write)
            dec_open_param.enableNonRefFbcWrite = dec_op->enable_non_ref_fbc_write;
        if (dec_op->has_enable_pvric_v31)
            dec_open_param.enablePvricV31 = dec_op->enable_pvric_v31;
        if (dec_op->has_temp_id_select_mode)
            dec_open_param.tempIdSelectMode = dec_op->temp_id_select_mode;
        if (dec_op->has_av1_format)
            dec_open_param.av1Format = dec_op->av1_format;
        if (dec_op->has_error_conceal_unit)
            dec_open_param.errorConcealUnit = dec_op->error_conceal_unit;
        if (dec_op->has_error_conceal_mode)
            dec_open_param.errorConcealMode = dec_op->error_conceal_mode;

        VLOG(INFO, "---------------------------- DECODER OPTIONS ----------------------------\n");
        VLOG(INFO, "[coreIdx             ]: %d\n", dec_open_param.coreIdx);
        VLOG(INFO, "[wtlEnable           ]: %d\n", dec_open_param.wtlEnable);
        VLOG(INFO, "[wtlMode             ]: %d\n", dec_open_param.wtlMode);
        VLOG(INFO, "[bitstreamBuffer     ]: 0x%08x\n", dec_open_param.bitstreamBuffer);
        VLOG(INFO, "[bitstreamBufferSize ]: %d\n", dec_open_param.bitstreamBufferSize);
        VLOG(INFO, "[bitstreamMode       ]: %d\n", dec_open_param.bitstreamMode);
        VLOG(INFO, "[cbcrInterleave      ]: %d\n", dec_open_param.cbcrInterleave);
        VLOG(INFO, "[nv21                ]: %d\n", dec_open_param.nv21);
        VLOG(INFO, "[streamEndian        ]: %d\n", dec_open_param.streamEndian);
        VLOG(INFO, "[frameEndian         ]: %d\n", dec_open_param.frameEndian);
        VLOG(INFO, "[BWB                 ]: %d\n", dec_open_param.bwbEnable);
        VLOG(INFO, "[bitstreamFormat     ]: %d\n", dec_open_param.bitstreamFormat);
        VLOG(INFO, "[avcExtension        ]: %d\n", dec_open_param.avcExtension);
        VLOG(INFO, "[tiled2LinearEnable  ]: %d\n", dec_open_param.tiled2LinearEnable);
        VLOG(INFO, "[tiled2LinearMode    ]: %d\n", dec_open_param.tiled2LinearMode);
        VLOG(INFO, "[mp4DeblkEnable      ]: %d\n", dec_open_param.mp4DeblkEnable);
        VLOG(INFO, "[mp4Class            ]: %d\n", dec_open_param.mp4Class);
        VLOG(INFO, "[pvricFbcEnable      ]: %d\n", dec_open_param.pvricFbcEnable);
        VLOG(INFO, "[fbc_mode            ]: %d\n", dec_open_param.fbc_mode);
        VLOG(INFO, "[bwOptimization      ]: %d\n", dec_open_param.bwOptimization);
        VLOG(INFO, "[pvric31HwSupport    ]: %d\n", dec_open_param.pvric31HwSupport);
        VLOG(INFO, "[enableNonRefFbcWrite]: %d\n", dec_open_param.enableNonRefFbcWrite);
        VLOG(INFO, "[enablePvricV31      ]: %d\n", dec_open_param.enablePvricV31);
        VLOG(INFO, "[tempIdSelectMode    ]: %d\n", dec_open_param.tempIdSelectMode);
        VLOG(INFO, "[av1Format           ]: %d\n", dec_open_param.av1Format);
        VLOG(INFO, "[errorConcealUnit    ]: %d\n", dec_open_param.errorConcealUnit);
        VLOG(INFO, "[errorConcealMode    ]: %d\n", dec_open_param.errorConcealMode);
        VLOG(INFO, "-------------------------------------------------------------------------\n");

        /********************************************************************************
         * CREATE INSTANCE                                                              *
         ********************************************************************************/
        ret = VPU_DecOpen(&handle, &dec_open_param);
        dec_open_msg->has_ret_code = TRUE;
        dec_open_msg->ret_code = ret;

        if (ret != RETCODE_SUCCESS) {
            VLOG(ERR, "VPU_DecOpen failed Error code is 0x%x \n", ret);
            return 0;
        }
        VLOG(INFO, "[%s][%d] VPU_DecOpen success, handle:%p\n", __func__, __LINE__, handle);
    }

    // sec AXI
    if (dec_op->has_sram_enable && dec_op->sram_enable) {
        vpu_buffer_t vb = {0};

        if (!vdi_get_sram_memory(dec_open_param.coreIdx, &vb) && vb.size > 0) {
            if (dec_open_param.coreIdx == VPU_CODA988_IDX) {
                sec_axi_use.u.coda9.useBitEnable = USE_BIT_INTERNAL_BUF;
                sec_axi_use.u.coda9.useIpEnable = USE_IP_INTERNAL_BUF;
                sec_axi_use.u.coda9.useDbkYEnable = USE_DBKY_INTERNAL_BUF;
                sec_axi_use.u.coda9.useDbkCEnable = USE_DBKC_INTERNAL_BUF;
                sec_axi_use.u.coda9.useBtpEnable = USE_BTP_INTERNAL_BUF;
                sec_axi_use.u.coda9.useOvlEnable = USE_OVL_INTERNAL_BUF;

                ret = VPU_DecGiveCommand(handle, SET_SEC_AXI, &sec_axi_use);
                dec_open_msg->ret_code = ret;
                if (ret != RETCODE_SUCCESS) {
                    VLOG(ERR, "VPU_DecGiveCommand ( SET_SEC_AXI ) failed Error code is 0x%x \n",
                         ret);
                    return 0;
                }

                instance->vpu_instance_contrl.inst_enable_sram[index] = TRUE;
            } else if (dec_open_param.coreIdx == VPU_WAVE412_IDX) {
                sec_axi_use.u.wave4.useIpEnable = TRUE;
                sec_axi_use.u.wave4.useLfRowEnable = TRUE;
                sec_axi_use.u.wave4.useBitEnable = TRUE;
                sec_axi_use.u.wave4.useSclEnable = TRUE;
                sec_axi_use.u.wave4.useSclPackedModeEnable = TRUE;

                ret = VPU_DecGiveCommand(handle, SET_SEC_AXI, &sec_axi_use);
                dec_open_msg->ret_code = ret;
                if (ret != RETCODE_SUCCESS) {
                    VLOG(ERR, "VPU_DecGiveCommand ( SET_SEC_AXI ) failed Error code is 0x%x \n",
                         ret);
                    return 0;
                }
            } else if (dec_open_param.coreIdx == VPU_WAVE521_IDX) {
                sec_axi_use.u.wave5.useIpEnable = TRUE;
                sec_axi_use.u.wave5.useLfRowEnable = TRUE;
                sec_axi_use.u.wave5.useBitEnable = TRUE;
                sec_axi_use.u.wave5.useSclEnable = TRUE;

                ret = VPU_DecGiveCommand(handle, SET_SEC_AXI, &sec_axi_use);
                dec_open_msg->ret_code = ret;
                if (ret != RETCODE_SUCCESS) {
                    VLOG(ERR, "VPU_DecGiveCommand ( SET_SEC_AXI ) failed Error code is 0x%x \n",
                         ret);
                    return 0;
                }
            }
        }
    }

    dec_open_msg->handle->handle = (uint32_t)handle;
    dec_open_msg->ret_code = ret;
    instance->vpu_instance_contrl.inst_handle[index] = (uint32_t)handle;

    return 0;
}

int vpu_instance_ipc_dec_flush(Vpu__DecFlush *dec_flush_msg)
{
    if (!dec_flush_msg->handle || !dec_flush_msg->handle->handle) {
        VLOG(ERR, "[%s][%d] invailed handle\n", __func__, __LINE__);
        return -1;
    }

    int ret = 0;
    DecHandle handle = (DecHandle)dec_flush_msg->handle->handle;
    VLOG(INFO, "[%s][%d] handle:%p\n", __func__, __LINE__, handle);

    if (dec_flush_msg->has_clr_disp_flag) {
        for (int i = 0; i < MAX_GDI_IDX; i++) {
            if ((dec_flush_msg->clr_disp_flag >> i) & 0x01)
                VPU_DecClrDispFlag(handle, i);
        }
    }

    if (dec_flush_msg->has_set_disp_flag) {
        for (int i = 0; i < MAX_GDI_IDX; i++) {
            if ((dec_flush_msg->set_disp_flag >> i) & 0x01)
                VPU_DecGiveCommand(handle, DEC_SET_DISPLAY_FLAG, &i);
        }
    }

    ret = VPU_DecFrameBufferFlush(handle, NULL, NULL);

    if (ret != RETCODE_SUCCESS)
        VLOG(ERR, "handle:%p VPU_DecFrameBufferFlush failed Error code is 0x%x \n", handle, ret);

    dec_flush_msg->has_ret_code = TRUE;
    dec_flush_msg->ret_code = ret;

    return 0;
}

int vpu_instance_ipc_dec_close(vpu_instance_t *instance, Vpu__DecClose *dec_close_msg)
{
    if (!dec_close_msg->handle || !dec_close_msg->handle->handle) {
        VLOG(ERR, "[%s][%d] invailed handle\n", __func__, __LINE__);
        return -1;
    }

    int ret = 0;
    int index = 0;
    DecHandle handle = (DecHandle)dec_close_msg->handle->handle;
    VLOG(INFO, "[%s][%d] handle:%p\n", __func__, __LINE__, handle);

    ret = VPU_DecClose(handle);
    if (ret != RETCODE_SUCCESS)
        VLOG(ERR, "handle:%p VPU_DecClose failed Error code is 0x%x \n", handle, ret);

    dec_close_msg->has_ret_code = TRUE;
    dec_close_msg->ret_code = ret;

    for (; index < VPU_MAX_NUM_INSTANCE; index++) {
        if (instance->vpu_instance_contrl.inst_handle[index] == (uint32_t)handle)
            break;
    }

    if (index >= VPU_MAX_NUM_INSTANCE) {
        VLOG(WARN, "[%s][%d] invaild handle:%p\n", __func__, __LINE__, handle);
        return 0;
    }

    instance->vpu_instance_contrl.inst_handle[index] = 0;
    instance->vpu_instance_contrl.inst_score[index] = 0;
    instance->vpu_instance_contrl.inst_enable_sram[index] = 0;

    return 0;
}

int vpu_instance_ipc_get_bitstream_buffer(Vpu__DecGetBitstreamBuffer *get_bs_buf_msg)
{
    if (!get_bs_buf_msg->handle || !get_bs_buf_msg->handle->handle) {
        VLOG(ERR, "[%s][%d] invailed handle\n", __func__, __LINE__);
        return -1;
    }

    int ret = 0;
    uint32_t rd_ptr, wr_ptr, room;
    DecHandle handle = (DecHandle)get_bs_buf_msg->handle->handle;
    VLOG(INFO, "[%s][%d] handle:%p\n", __func__, __LINE__, handle);

    ret = VPU_DecGetBitstreamBuffer(handle, &rd_ptr, &wr_ptr, &room);
    if (ret != RETCODE_SUCCESS)
        VLOG(ERR, "handle:%p VPU_DecGetBitstreamBuffer failed Error code is 0x%x \n", handle, ret);

    get_bs_buf_msg->has_prd_ptr = TRUE;
    get_bs_buf_msg->prd_ptr = rd_ptr;
    get_bs_buf_msg->has_pwr_ptr = TRUE;
    get_bs_buf_msg->pwr_ptr = wr_ptr;
    get_bs_buf_msg->has_size = TRUE;
    get_bs_buf_msg->size = room;
    get_bs_buf_msg->has_ret_code = TRUE;
    get_bs_buf_msg->ret_code = ret;

    return 0;
}

int vpu_instance_ipc_update_bitstream_buffer(Vpu__DecUpdateBitstreamBuffer *update_bs_buf_msg)
{
    if (!update_bs_buf_msg->handle || !update_bs_buf_msg->handle->handle ||
        !update_bs_buf_msg->has_size) {
        VLOG(ERR, "[%s][%d] invailed param\n", __func__, __LINE__);
        return -1;
    }

    int ret = 0;
    DecHandle handle = (DecHandle)update_bs_buf_msg->handle->handle;
    VLOG(INFO, "[%s][%d] handle:%p\n", __func__, __LINE__, handle);

    ret = VPU_DecUpdateBitstreamBuffer(handle, update_bs_buf_msg->size);
    if (ret != RETCODE_SUCCESS)
        VLOG(ERR, "handle:%p VPU_DecUpdateBitstreamBuffer failed Error code is 0x%x \n", handle,
             ret);

    update_bs_buf_msg->has_ret_code = TRUE;
    update_bs_buf_msg->ret_code = ret;

    return 0;
}

int vpu_instance_ipc_pic_end_mode_update_bs_buffer(
    Vpu__DecPicEndModeUpdateBSBuffer *pic_end_mode_update_bs_buf_msg)
{
    if (!pic_end_mode_update_bs_buf_msg->handle ||
        !pic_end_mode_update_bs_buf_msg->handle->handle ||
        !pic_end_mode_update_bs_buf_msg->has_size || !pic_end_mode_update_bs_buf_msg->has_addr ||
        !pic_end_mode_update_bs_buf_msg->has_flag) {
        VLOG(ERR, "[%s][%d] invailed param\n", __func__, __LINE__);
        return -1;
    }

    int ret = 0;
    DecHandle handle = (DecHandle)pic_end_mode_update_bs_buf_msg->handle->handle;
    VLOG(INFO, "[%s][%d] handle:%p bs_buf_addr:%x write_size:%d flag:%d\n", __func__, __LINE__,
         handle, pic_end_mode_update_bs_buf_msg->addr, pic_end_mode_update_bs_buf_msg->size,
         pic_end_mode_update_bs_buf_msg->flag);

    // VPU_DecSetRdPtrEx for wave to update streamBufStartAddr
    ret = VPU_DecSetRdPtrEx(handle, pic_end_mode_update_bs_buf_msg->addr,
                            pic_end_mode_update_bs_buf_msg->addr, TRUE);
    pic_end_mode_update_bs_buf_msg->has_ret_code = TRUE;
    pic_end_mode_update_bs_buf_msg->ret_code = ret;

    if (ret != RETCODE_SUCCESS) {
        VLOG(ERR, "[%s][%d] handle:%p VPU_DecSetRdPtr failed Error code is 0x%x \n", __func__,
             __LINE__, handle, ret);
        return 0;
    }

    // VPU_DecUpdateBitstreamBuffer
    switch (pic_end_mode_update_bs_buf_msg->flag) {
    case VPU__STREAM_UP_FLAG__STREAM_UPDATE:
        ret = VPU_DecUpdateBitstreamBuffer(handle, pic_end_mode_update_bs_buf_msg->size);
        break;
    case VPU__STREAM_UP_FLAG__STREAM_END:
        ret = VPU_DecUpdateBitstreamBuffer(handle, STREAM_END_SIZE);
        break;
    case VPU__STREAM_UP_FLAG__STREAM_END_CLEAR:
        ret = VPU_DecUpdateBitstreamBuffer(handle, STREAM_END_CLEAR_FLAG);
        break;
    default:
        break;
    }

    pic_end_mode_update_bs_buf_msg->ret_code = ret;

    if (ret != RETCODE_SUCCESS) {
        VLOG(ERR, "[%s][%d] handle:%p VPU_DecUpdateBitstreamBuffer failed Error code is 0x%x \n",
             __func__, __LINE__, handle, ret);
    }

    return 0;
}

int vpu_instance_ipc_dec_seq_init(Vpu__DecSeqInit *seq_init_msg, Vpu__DecSeqInit *seq_init_msg_ret,
                                  vpu_instance_t *instance)
{
    if (!seq_init_msg->handle || !seq_init_msg->handle->handle || !instance) {
        VLOG(ERR, "[%s][%d] invailed handle\n", __func__, __LINE__);
        return -1;
    }

    int ret = 0;
    DecHandle handle = (DecHandle)seq_init_msg->handle->handle;
    DecInitialInfo *output_info = &instance->vpu_info.sequence_info;

    *seq_init_msg_ret = (Vpu__DecSeqInit)VPU__DEC_SEQ_INIT__INIT;
    seq_init_msg_ret->handle = seq_init_msg->handle;
    VLOG(INFO, "[%s][%d] handle:%p\n", __func__, __LINE__, handle);

    // VPU_DecIssueSeqInit
    ret = VPU_DecIssueSeqInit(handle);
    seq_init_msg_ret->has_ret_code = TRUE;
    seq_init_msg_ret->ret_code = ret;

    if (ret != RETCODE_SUCCESS) {
        VLOG(ERR, "handle:%p VPU_DecIssueSeqInit failed Error code is 0x%x \n", handle, ret);
        VPU_DecCompleteSeqInit(handle, output_info);
        return 0;
    }

    // VPU_WaitInterrupt
    if (handle->coreIdx == VPU_WAVE521_IDX) {
        ret = wave5_dec_hand_interrupt(handle);
        seq_init_msg_ret->has_interrupt_reaseon = TRUE;
        seq_init_msg_ret->interrupt_reaseon = ret;

        if (ret == DEC_INT_STATUS_TIMEOUT) {
            seq_init_msg_ret->ret_code = RETCODE_FAILURE;
            VPU_DecCompleteSeqInit(handle, output_info);
            // the third parameter of VPU_SWReset requires the pending instance
            VPU_SWReset(handle->coreIdx, SW_RESET_SAFETY, NULL);
            VLOG(ERR,
                 "[%s][%d] handle:%p VPU_WaitInterrupt timeout, ret is 0x%x rdPtr:%x wrPtr:%x\n",
                 __func__, __LINE__, handle, ret, output_info->rdPtr, output_info->wrPtr);
            return 0;
        } else if (ret != DEC_INT_STATUS_DONE) {
            seq_init_msg_ret->ret_code = RETCODE_FAILURE;
            VPU_DecCompleteSeqInit(handle, output_info);
            VLOG(ERR,
                 "[%s][%d] handle:%p wave5_dec_hand_interrupt failed, ret is 0x%x rdPtr:%x "
                 "wrPtr:%x\n",
                 __func__, __LINE__, handle, ret, output_info->rdPtr, output_info->wrPtr);
            return 0;
        }
    } else {
        ret = VPU_WaitInterrupt(handle->coreIdx, VPU_DEC_TIMEOUT);
        seq_init_msg_ret->has_interrupt_reaseon = TRUE;
        seq_init_msg_ret->interrupt_reaseon = ret;

        if (ret == -1) {
            seq_init_msg_ret->ret_code = RETCODE_FAILURE;
            VPU_DecCompleteSeqInit(handle, output_info);
            // the third parameter of VPU_SWReset requires the pending instance
            VPU_SWReset(handle->coreIdx, SW_RESET_SAFETY, NULL);
            VLOG(ERR, "handle:%p VPU_WaitInterrupt timeout, ret is 0x%x rdPtr:%x wrPtr:%x\n",
                 handle, ret, output_info->rdPtr, output_info->wrPtr);
            return 0;
        }

        if (ret) {
            VPU_ClearInterrupt(handle->coreIdx);
            if (ret & (1 << INT_BIT_SEQ_INIT))
                VLOG(INFO, "[%s][%d] handle:%p INT_BIT_SEQ_INIT\n", __func__, __LINE__, handle);
            else {
                seq_init_msg_ret->ret_code = RETCODE_FAILURE;
                VLOG(ERR, "[%s][%d] handle:%p Non INT_BIT_SEQ_INIT, ret:%d\n", __func__, __LINE__,
                     handle, ret);
                VPU_DecCompleteSeqInit(handle, output_info);
                return 0;
            }
        }
    }

    // VPU_DecCompleteSeqInit
    memset(output_info, 0x0, sizeof(DecInitialInfo));
    ret = VPU_DecCompleteSeqInit(handle, output_info);
    seq_init_msg_ret->ret_code = ret;

    if (ret != RETCODE_SUCCESS) {
        VLOG(ERR,
             "handle:%p VPU_DecCompleteSeqInit(SEQ_INIT) failed Error code is 0x%x "
             "REASON:%d(0x%x)\n",
             handle, ret, output_info->seqInitErrReason, output_info->seqInitErrReason);

        Vpu__DecInitialInfo *output_info_msg = &instance->vpu_proto_info.init_output_info;

        *output_info_msg = (Vpu__DecInitialInfo)VPU__DEC_INITIAL_INFO__INIT;
        seq_init_msg_ret->initial_info = output_info_msg;

        output_info_msg->has_seq_init_err_reason = TRUE;
        output_info_msg->seq_init_err_reason = output_info->seqInitErrReason;

        return 0;
    }

    // return msg
    Vpu__DecInitialInfo *output_info_msg = &instance->vpu_proto_info.init_output_info;

    *output_info_msg = (Vpu__DecInitialInfo)VPU__DEC_INITIAL_INFO__INIT;
    seq_init_msg_ret->initial_info = output_info_msg;

    output_info_msg->pic_crop_rect = &instance->vpu_proto_info.init_pic_crop_rect;
    *output_info_msg->pic_crop_rect = (Vpu__VpuRect)VPU__VPU_RECT__INIT;

    /*
    picWidth
    picHeight
    seqInitErrReason

    minFrameBufferCount
    rdPtr
    wrPtr
    sequenceNo
    fRateNumerator
    fRateDenominator
    profile

    lumaBitdepth
    chromaBitdepth

    mp2ColorDescPresentFlag
    mp2ColorPrimaries
    mp2TransferChar
    mp2MatrixCoeff

    picCropRect  //left top right bottom
    avcVuiInfo //struct
    h265Info
    */
    output_info_msg->pic_width = output_info->picWidth;
    output_info_msg->pic_height = output_info->picHeight;
    output_info_msg->seq_init_err_reason = output_info->seqInitErrReason;
    output_info_msg->min_frame_buffer_count = output_info->minFrameBufferCount;
    output_info_msg->rd_ptr = output_info->rdPtr;
    output_info_msg->wr_ptr = output_info->wrPtr;
    output_info_msg->sequence_no = output_info->sequenceNo;
    output_info_msg->f_rate_numerator = output_info->fRateNumerator;
    output_info_msg->f_rate_denominator = output_info->fRateDenominator;
    output_info_msg->profile = output_info->profile;
    output_info_msg->luma_bitdepth = output_info->lumaBitdepth;
    output_info_msg->chroma_bitdepth = output_info->chromaBitdepth;
    output_info_msg->mp2_color_desc_present_flag = output_info->mp2ColorDescPresentFlag;
    output_info_msg->mp2_color_primaries = output_info->mp2ColorPrimaries;
    output_info_msg->mp2_transfer_char = output_info->mp2TransferChar;
    output_info_msg->mp2_matrix_coeff = output_info->mp2MatrixCoeff;

    output_info_msg->pic_crop_rect->left = output_info->picCropRect.left;
    output_info_msg->pic_crop_rect->top = output_info->picCropRect.top;
    output_info_msg->pic_crop_rect->right = output_info->picCropRect.right;
    output_info_msg->pic_crop_rect->bottom = output_info->picCropRect.bottom;

    output_info_msg->has_pic_width = TRUE;
    output_info_msg->has_pic_height = TRUE;
    output_info_msg->has_seq_init_err_reason = TRUE;
    output_info_msg->has_min_frame_buffer_count = TRUE;
    output_info_msg->has_rd_ptr = TRUE;
    output_info_msg->has_wr_ptr = TRUE;
    output_info_msg->has_sequence_no = TRUE;
    output_info_msg->has_f_rate_numerator = TRUE;
    output_info_msg->has_f_rate_denominator = TRUE;
    output_info_msg->has_profile = TRUE;
    output_info_msg->has_luma_bitdepth = TRUE;
    output_info_msg->has_chroma_bitdepth = TRUE;
    output_info_msg->has_mp2_color_desc_present_flag = TRUE;
    output_info_msg->has_mp2_color_primaries = TRUE;
    output_info_msg->has_mp2_transfer_char = TRUE;
    output_info_msg->has_mp2_matrix_coeff = TRUE;

    output_info_msg->pic_crop_rect->has_left = TRUE;
    output_info_msg->pic_crop_rect->has_top = TRUE;
    output_info_msg->pic_crop_rect->has_right = TRUE;
    output_info_msg->pic_crop_rect->has_bottom = TRUE;

    if ((handle->coreIdx == VPU_CODA988_IDX && handle->codecMode == AVC_DEC) ||
        (handle->coreIdx == VPU_WAVE521_IDX && handle->codecMode == W_AVC_DEC)) {
        output_info_msg->codec_mode_info_case = VPU__DEC_INITIAL_INFO__CODEC_MODE_INFO_AVC_VUI_INFO;
        output_info_msg->avc_vui_info = &instance->vpu_proto_info.init_avc_vui_info;

        *output_info_msg->avc_vui_info = (Vpu__AvcVuiInfo)VPU__AVC_VUI_INFO__INIT;

        output_info_msg->avc_vui_info->fixed_frame_rate_flag =
            output_info->avcVuiInfo.fixedFrameRateFlag;
        output_info_msg->avc_vui_info->timing_info_present =
            output_info->avcVuiInfo.timingInfoPresent;
        output_info_msg->avc_vui_info->chroma_loc_bot_field =
            output_info->avcVuiInfo.chromaLocBotField;
        output_info_msg->avc_vui_info->chroma_loc_top_field =
            output_info->avcVuiInfo.chromaLocTopField;
        output_info_msg->avc_vui_info->chroma_loc_info_present =
            output_info->avcVuiInfo.chromaLocInfoPresent;
        output_info_msg->avc_vui_info->color_primaries = output_info->avcVuiInfo.colorPrimaries;
        output_info_msg->avc_vui_info->color_desc_present =
            output_info->avcVuiInfo.colorDescPresent;
        output_info_msg->avc_vui_info->is_ext_s_a_r = output_info->avcVuiInfo.isExtSAR;
        output_info_msg->avc_vui_info->vid_full_range = output_info->avcVuiInfo.vidFullRange;
        output_info_msg->avc_vui_info->vid_format = output_info->avcVuiInfo.vidFormat;
        output_info_msg->avc_vui_info->vid_sig_type_present =
            output_info->avcVuiInfo.vidSigTypePresent;
        output_info_msg->avc_vui_info->vui_param_present = output_info->avcVuiInfo.vuiParamPresent;
        output_info_msg->avc_vui_info->vui_pic_struct_present =
            output_info->avcVuiInfo.vuiPicStructPresent;
        output_info_msg->avc_vui_info->vui_pic_struct = output_info->avcVuiInfo.vuiPicStruct;
        output_info_msg->avc_vui_info->vui_transfer_characteristics =
            output_info->avcVuiInfo.vuiTransferCharacteristics;
        output_info_msg->avc_vui_info->vui_matrix_coefficients =
            output_info->avcVuiInfo.vuiMatrixCoefficients;

        output_info_msg->avc_vui_info->has_fixed_frame_rate_flag = TRUE;
        output_info_msg->avc_vui_info->has_timing_info_present = TRUE;
        output_info_msg->avc_vui_info->has_chroma_loc_bot_field = TRUE;
        output_info_msg->avc_vui_info->has_chroma_loc_top_field = TRUE;
        output_info_msg->avc_vui_info->has_chroma_loc_info_present = TRUE;
        output_info_msg->avc_vui_info->has_color_primaries = TRUE;
        output_info_msg->avc_vui_info->has_color_desc_present = TRUE;
        output_info_msg->avc_vui_info->has_is_ext_s_a_r = TRUE;
        output_info_msg->avc_vui_info->has_vid_full_range = TRUE;
        output_info_msg->avc_vui_info->has_vid_format = TRUE;
        output_info_msg->avc_vui_info->has_vid_sig_type_present = TRUE;
        output_info_msg->avc_vui_info->has_vui_param_present = TRUE;
        output_info_msg->avc_vui_info->has_vui_pic_struct_present = TRUE;
        output_info_msg->avc_vui_info->has_vui_pic_struct = TRUE;
        output_info_msg->avc_vui_info->has_vui_transfer_characteristics = TRUE;
        output_info_msg->avc_vui_info->has_vui_matrix_coefficients = TRUE;
    } else if ((handle->coreIdx == VPU_WAVE412_IDX && handle->codecMode == HEVC_DEC) ||
               (handle->coreIdx == VPU_WAVE521_IDX && handle->codecMode == W_HEVC_DEC)) {
        output_info_msg->codec_mode_info_case = VPU__DEC_INITIAL_INFO__CODEC_MODE_INFO_HEVC_INFO;

        output_info_msg->hevc_info = &instance->vpu_proto_info.init_hevc_info;
        *output_info_msg->hevc_info = (Vpu__DecHevcInfo)VPU__DEC_HEVC_INFO__INIT;

        output_info_msg->hevc_info->h265_info = &instance->vpu_proto_info.init_hevc_h265_info;
        *output_info_msg->hevc_info->h265_info =
            (Vpu__DecHevcInfo__H265Info)VPU__DEC_HEVC_INFO__H265_INFO__INIT;

        output_info_msg->hevc_info->h265_info->decoded_p_o_c = output_info->h265Info.decodedPOC;
        output_info_msg->hevc_info->h265_info->display_p_o_c = output_info->h265Info.displayPOC;
        output_info_msg->hevc_info->h265_info->ctu_size = output_info->h265Info.ctuSize;
        output_info_msg->hevc_info->h265_info->temporal_id = output_info->h265Info.temporalId;
        output_info_msg->hevc_info->h265_info->color_primaries =
            output_info->h265Info.colorPrimaries;
        output_info_msg->hevc_info->h265_info->color_desc_present =
            output_info->h265Info.colorDescPresent;
        output_info_msg->hevc_info->h265_info->vid_full_range = output_info->h265Info.vidFullRange;
        output_info_msg->hevc_info->h265_info->vui_param_present =
            output_info->h265Info.vuiParamPresent;
        output_info_msg->hevc_info->h265_info->vui_transfer_characteristics =
            output_info->h265Info.vuiTransferCharacteristics;
        output_info_msg->hevc_info->h265_info->vui_matrix_coefficients =
            output_info->h265Info.vuiMatrixCoefficients;
        output_info_msg->hevc_info->h265_info->master_meta_present =
            output_info->h265Info.masterMetaPresent;
        output_info_msg->hevc_info->h265_info->display_primaries_x_b =
            output_info->h265Info.display_primaries_x[0];
        output_info_msg->hevc_info->h265_info->display_primaries_x_g =
            output_info->h265Info.display_primaries_x[1];
        output_info_msg->hevc_info->h265_info->display_primaries_x_r =
            output_info->h265Info.display_primaries_x[2];
        output_info_msg->hevc_info->h265_info->display_primaries_y_b =
            output_info->h265Info.display_primaries_y[0];
        output_info_msg->hevc_info->h265_info->display_primaries_y_g =
            output_info->h265Info.display_primaries_y[1];
        output_info_msg->hevc_info->h265_info->display_primaries_y_r =
            output_info->h265Info.display_primaries_y[2];
        output_info_msg->hevc_info->h265_info->white_point_x = output_info->h265Info.white_point_x;
        output_info_msg->hevc_info->h265_info->white_point_y = output_info->h265Info.white_point_y;
        output_info_msg->hevc_info->h265_info->max_display_mastering_luminance =
            output_info->h265Info.max_display_mastering_luminance;
        output_info_msg->hevc_info->h265_info->min_display_mastering_luminance =
            output_info->h265Info.min_display_mastering_luminance;
        output_info_msg->hevc_info->h265_info->content_meta_present =
            output_info->h265Info.contentMetaPresent;
        output_info_msg->hevc_info->h265_info->max_content_light_level =
            output_info->h265Info.max_content_light_level;
        output_info_msg->hevc_info->h265_info->max_pic_average_light_level =
            output_info->h265Info.max_pic_average_light_level;

        output_info_msg->hevc_info->h265_info->has_decoded_p_o_c = TRUE;
        output_info_msg->hevc_info->h265_info->has_display_p_o_c = TRUE;
        output_info_msg->hevc_info->h265_info->has_ctu_size = TRUE;
        output_info_msg->hevc_info->h265_info->has_temporal_id = TRUE;
        output_info_msg->hevc_info->h265_info->has_color_primaries = TRUE;
        output_info_msg->hevc_info->h265_info->has_color_desc_present = TRUE;
        output_info_msg->hevc_info->h265_info->has_vid_full_range = TRUE;
        output_info_msg->hevc_info->h265_info->has_vui_param_present = TRUE;
        output_info_msg->hevc_info->h265_info->has_vui_transfer_characteristics = TRUE;
        output_info_msg->hevc_info->h265_info->has_vui_matrix_coefficients = TRUE;
        output_info_msg->hevc_info->h265_info->has_master_meta_present = TRUE;
        output_info_msg->hevc_info->h265_info->has_display_primaries_x_b = TRUE;
        output_info_msg->hevc_info->h265_info->has_display_primaries_x_g = TRUE;
        output_info_msg->hevc_info->h265_info->has_display_primaries_x_r = TRUE;
        output_info_msg->hevc_info->h265_info->has_display_primaries_y_b = TRUE;
        output_info_msg->hevc_info->h265_info->has_display_primaries_y_g = TRUE;
        output_info_msg->hevc_info->h265_info->has_display_primaries_y_r = TRUE;
        output_info_msg->hevc_info->h265_info->has_white_point_x = TRUE;
        output_info_msg->hevc_info->h265_info->has_white_point_y = TRUE;
        output_info_msg->hevc_info->h265_info->has_max_display_mastering_luminance = TRUE;
        output_info_msg->hevc_info->h265_info->has_min_display_mastering_luminance = TRUE;
        output_info_msg->hevc_info->h265_info->has_content_meta_present = TRUE;
        output_info_msg->hevc_info->h265_info->has_max_content_light_level = TRUE;
        output_info_msg->hevc_info->h265_info->has_max_pic_average_light_level = TRUE;
    }

    // codec resource control
    {
        int fps;
        int index = 0;
        uint32_t total_score = 0;

        for (; index < VPU_MAX_NUM_INSTANCE; index++) {
            if (instance->vpu_instance_contrl.inst_handle[index] == (uint32_t)handle)
                break;
        }

        if (index >= VPU_MAX_NUM_INSTANCE) {
            VLOG(ERR, "[%s][%d] invaild handle:%p\n", __func__, __LINE__, handle);
            seq_init_msg_ret->ret_code = RETCODE_RESOURECE_CONSTRAINTS;
            return 0;
        }

        fps = vpu_instance_calculate_fps(handle, output_info_msg);
        if (fps == 0)
            fps = 30; // defalut value for smooth playback

        // score = width * height * fps * sram_compensation_value + 1
        if (handle->coreIdx == VPU_CODA988_IDX) {
            instance->vpu_instance_contrl.inst_score[index] =
                VPU_ALIGN32(output_info_msg->pic_width) * VPU_ALIGN32(output_info_msg->pic_height) *
                    fps *
                    (instance->vpu_instance_contrl.inst_enable_sram[index] == TRUE
                         ? 1
                         : VPU_CODA988_DISABLE_SRAM_ABILITY_COMPENSATION_VALUE) +
                1;
        } else if (handle->coreIdx == VPU_WAVE521_IDX && handle->codecMode == W_AVC_DEC) {
            instance->vpu_instance_contrl.inst_score[index] =
                VPU_ALIGN32(output_info_msg->pic_width) * VPU_ALIGN32(output_info_msg->pic_height) *
                    fps * VPU_WAVE521_DEC_AVC_ABILITY_COMPENSATION_VALUE +
                1;
        } else {
            instance->vpu_instance_contrl.inst_score[index] =
                VPU_ALIGN32(output_info_msg->pic_width) * VPU_ALIGN32(output_info_msg->pic_height) *
                    fps +
                1;
        }

        for (int i = 0; i < VPU_MAX_NUM_INSTANCE; i++) {
            if (instance->vpu_instance_contrl.inst_handle[i])
                total_score += instance->vpu_instance_contrl.inst_score[i];
        }

        if (total_score == instance->vpu_instance_contrl.inst_score[index]) {
            // at least one instance is need supported
        } else if (total_score > instance->vpu_instance_contrl.codec_ability_score) {
            VLOG(ERR,
                 "[%s][%d] total score:%u > codec ability score:%u, current instance score:%d, "
                 "current handle:%p\n",
                 __func__, __LINE__, total_score, instance->vpu_instance_contrl.codec_ability_score,
                 instance->vpu_instance_contrl.inst_score[index], handle);

            instance->vpu_instance_contrl.inst_score[index] = 0;
            seq_init_msg_ret->ret_code = RETCODE_RESOURECE_CONSTRAINTS;
            return 0;
        }
    }

    return 0;
}

int vpu_instance_ipc_sw_reset(Vpu__SWReset *sw_reset_msg)
{
    if (!sw_reset_msg->has_handle || !sw_reset_msg->handle) {
        VLOG(ERR, "[%s][%d] invailed handle\n", __func__, __LINE__);
        return -1;
    }

    int ret = 0;
    SWResetMode reset_mode = SW_RESET_SAFETY;
    VLOG(INFO, "[%s][%d] handle:%x\n", __func__, __LINE__, sw_reset_msg->handle);

    reset_mode = sw_reset_msg->has_reset_mode ? sw_reset_msg->reset_mode : SW_RESET_SAFETY;

    // the third parameter of VPU_SWReset requires the pending instance
    ret = VPU_SWReset(sw_reset_msg->core_idx, reset_mode, NULL);
    if (ret != RETCODE_SUCCESS)
        VLOG(ERR, "handle:%x VPU_SWReset failed Error code is 0x%x \n", sw_reset_msg->handle, ret);

    sw_reset_msg->has_ret_code = TRUE;
    sw_reset_msg->ret_code = ret;

    return 0;
}

int vpu_instance_ipc_dec_clr_disp_flag(Vpu__DecClrDispFlag *clr_disp_flag_msg)
{
    if (!clr_disp_flag_msg->handle || !clr_disp_flag_msg->handle->handle ||
        !clr_disp_flag_msg->has_index) {
        VLOG(ERR, "[%s][%d] invailed param\n", __func__, __LINE__);
        return -1;
    }

    int ret = 0;
    DecHandle handle = (DecHandle)clr_disp_flag_msg->handle->handle;
    VLOG(INFO, "[%s][%d] handle:%p\n", __func__, __LINE__, handle);

    ret = VPU_DecClrDispFlag(handle, clr_disp_flag_msg->index);
    if (ret != RETCODE_SUCCESS)
        VLOG(ERR, "handle:%p VPU_DecClrDispFlag failed Error code is 0x%x \n", handle, ret);

    clr_disp_flag_msg->has_ret_code = TRUE;
    clr_disp_flag_msg->ret_code = ret;

    return 0;
}

int vpu_instance_ipc_dec_register_dec_buf(Vpu__DecRegisterDecodeBuffer *register_dec_buf_msg,
                                          vpu_instance_t *instance)
{
    if (!register_dec_buf_msg->handle || !register_dec_buf_msg->handle->handle ||
        !register_dec_buf_msg->fb_alloc_info_dec || !register_dec_buf_msg->has_vb_frame_dec_size ||
        !register_dec_buf_msg->n_vb_frame_dec_phys_addr ||
        !register_dec_buf_msg->vb_frame_dec_phys_addr || !instance) {
        VLOG(ERR, "[%s][%d] invailed param\n", __func__, __LINE__);
        return -1;
    }

    int ret = 0;
    FrameBufferAllocInfo fb_alloc_info_dec = {0};
    FrameBufferAllocInfo fb_alloc_info_wtl = {0};
    Vpu__FrameBufferAllocInfo *fb_alloc_info_dec_msg = register_dec_buf_msg->fb_alloc_info_dec;
    Vpu__FrameBufferAllocInfo *fb_alloc_info_wtl_msg = register_dec_buf_msg->fb_alloc_info_wtl;
    DecHandle handle = (DecHandle)register_dec_buf_msg->handle->handle;
    VLOG(INFO, "[%s][%d] handle:%p\n", __func__, __LINE__, handle);

    // FrameBufferAllocInfo
    {
        if (fb_alloc_info_dec_msg->has_map_type)
            fb_alloc_info_dec.mapType = fb_alloc_info_dec_msg->map_type;
        if (fb_alloc_info_dec_msg->has_cbcr_interleave)
            fb_alloc_info_dec.cbcrInterleave = fb_alloc_info_dec_msg->cbcr_interleave;
        if (fb_alloc_info_dec_msg->has_nv21)
            fb_alloc_info_dec.nv21 = fb_alloc_info_dec_msg->nv21;
        if (fb_alloc_info_dec_msg->has_format)
            fb_alloc_info_dec.format = fb_alloc_info_dec_msg->format;
        if (fb_alloc_info_dec_msg->has_stride)
            fb_alloc_info_dec.stride = fb_alloc_info_dec_msg->stride;
        if (fb_alloc_info_dec_msg->has_height)
            fb_alloc_info_dec.height = fb_alloc_info_dec_msg->height;
        if (fb_alloc_info_dec_msg->has_size)
            fb_alloc_info_dec.size = fb_alloc_info_dec_msg->size;
        if (fb_alloc_info_dec_msg->has_luma_bit_depth)
            fb_alloc_info_dec.lumaBitDepth = fb_alloc_info_dec_msg->luma_bit_depth;
        if (fb_alloc_info_dec_msg->has_chroma_bit_depth)
            fb_alloc_info_dec.chromaBitDepth = fb_alloc_info_dec_msg->chroma_bit_depth;
        if (fb_alloc_info_dec_msg->has_endian)
            fb_alloc_info_dec.endian = fb_alloc_info_dec_msg->endian;
        if (fb_alloc_info_dec_msg->has_num)
            fb_alloc_info_dec.num = fb_alloc_info_dec_msg->num;
        if (fb_alloc_info_dec_msg->has_type)
            fb_alloc_info_dec.type = fb_alloc_info_dec_msg->type;
    }

    // frame buffer for dec
    memset(instance->vpu_info.frame_buffers, 0x0, sizeof(instance->vpu_info.frame_buffers));
    for (size_t i = 0; i < register_dec_buf_msg->n_vb_frame_dec_phys_addr; i++) {
        instance->vpu_info.frame_buffers[i].bufY = register_dec_buf_msg->vb_frame_dec_phys_addr[i];
        instance->vpu_info.frame_buffers[i].bufCb = -1;
        instance->vpu_info.frame_buffers[i].bufCr = -1;
        instance->vpu_info.frame_buffers[i].updateFbInfo = TRUE;
    }

    ret =
        VPU_DecAllocateFrameBuffer(handle, fb_alloc_info_dec, &instance->vpu_info.frame_buffers[0]);
    register_dec_buf_msg->has_ret_code = TRUE;
    register_dec_buf_msg->ret_code = ret;

    if (ret != RETCODE_SUCCESS) {
        VLOG(ERR, "[%s][%d] handle:%p VPU_DecAllocateFrameBuffer failed Error code is 0x%x \n",
             __func__, __LINE__, handle, ret);
        return 0;
    }

    // frame buffer for wtl
    if (fb_alloc_info_wtl_msg && register_dec_buf_msg->has_vb_frame_wtl_size &&
        register_dec_buf_msg->n_vb_frame_wtl_phys_addr &&
        register_dec_buf_msg->vb_frame_wtl_phys_addr) {
        if (fb_alloc_info_wtl_msg->has_map_type)
            fb_alloc_info_wtl.mapType = fb_alloc_info_wtl_msg->map_type;
        if (fb_alloc_info_wtl_msg->has_cbcr_interleave)
            fb_alloc_info_wtl.cbcrInterleave = fb_alloc_info_wtl_msg->cbcr_interleave;
        if (fb_alloc_info_wtl_msg->has_nv21)
            fb_alloc_info_wtl.nv21 = fb_alloc_info_wtl_msg->nv21;
        if (fb_alloc_info_wtl_msg->has_format)
            fb_alloc_info_wtl.format = fb_alloc_info_wtl_msg->format;
        if (fb_alloc_info_wtl_msg->has_stride)
            fb_alloc_info_wtl.stride = fb_alloc_info_wtl_msg->stride;
        if (fb_alloc_info_wtl_msg->has_height)
            fb_alloc_info_wtl.height = fb_alloc_info_wtl_msg->height;
        if (fb_alloc_info_wtl_msg->has_size)
            fb_alloc_info_wtl.size = fb_alloc_info_wtl_msg->size;
        if (fb_alloc_info_wtl_msg->has_luma_bit_depth)
            fb_alloc_info_wtl.lumaBitDepth = fb_alloc_info_wtl_msg->luma_bit_depth;
        if (fb_alloc_info_wtl_msg->has_chroma_bit_depth)
            fb_alloc_info_wtl.chromaBitDepth = fb_alloc_info_wtl_msg->chroma_bit_depth;
        if (fb_alloc_info_wtl_msg->has_endian)
            fb_alloc_info_wtl.endian = fb_alloc_info_wtl_msg->endian;
        if (fb_alloc_info_wtl_msg->has_num)
            fb_alloc_info_wtl.num = fb_alloc_info_wtl_msg->num;
        if (fb_alloc_info_wtl_msg->has_type)
            fb_alloc_info_wtl.type = fb_alloc_info_wtl_msg->type;

        for (size_t i = 0; i < register_dec_buf_msg->n_vb_frame_wtl_phys_addr; i++) {
            instance->vpu_info.frame_buffers[i + register_dec_buf_msg->n_vb_frame_dec_phys_addr]
                .bufY = register_dec_buf_msg->vb_frame_wtl_phys_addr[i];
            instance->vpu_info.frame_buffers[i + register_dec_buf_msg->n_vb_frame_dec_phys_addr]
                .bufCb = -1;
            instance->vpu_info.frame_buffers[i + register_dec_buf_msg->n_vb_frame_dec_phys_addr]
                .bufCr = -1;
            instance->vpu_info.frame_buffers[i + register_dec_buf_msg->n_vb_frame_dec_phys_addr]
                .updateFbInfo = TRUE;
        }

        // check scaler info, only for vpu wave(wtl) care scaler info
        if (register_dec_buf_msg->has_enable_scaler && register_dec_buf_msg->enable_scaler &&
            register_dec_buf_msg->has_scale_width && register_dec_buf_msg->has_scale_height) {
            ScalerInfo scalerInfo;
            scalerInfo.enScaler = TRUE;
            scalerInfo.scaleWidth = register_dec_buf_msg->scale_width;
            scalerInfo.scaleHeight = register_dec_buf_msg->scale_height;

            if (scalerInfo.scaleWidth ==
                    (uint32_t)handle->CodecInfo->decInfo.initialInfo.picWidth &&
                scalerInfo.scaleHeight ==
                    (uint32_t)handle->CodecInfo->decInfo.initialInfo.picHeight) {
                scalerInfo.sameSize = TRUE;
            } else {
                scalerInfo.sameSize = FALSE;
            }

            ret = VPU_DecGiveCommand(handle, DEC_SET_SCALER_INFO, (void *)&scalerInfo);
            register_dec_buf_msg->ret_code = ret;

            if (ret != RETCODE_SUCCESS) {
                VLOG(ERR, "[%s][%d] handle:%p DEC_SET_SCALER_INFO failed Error code is 0x%x \n",
                     __func__, __LINE__, handle, ret);
                return 0;
            }
        } else if (fb_alloc_info_wtl.mapType == PVRIC_COMPRESSED_FRAME_MAP) {
            // defalut enable scaler info for ifbc mode
            ScalerInfo scalerInfo;
            scalerInfo.enScaler = TRUE;
            scalerInfo.sameSize = TRUE;
            scalerInfo.scaleWidth = handle->CodecInfo->decInfo.initialInfo.picWidth;
            scalerInfo.scaleHeight = handle->CodecInfo->decInfo.initialInfo.picHeight;

            ret = VPU_DecGiveCommand(handle, DEC_SET_SCALER_INFO, (void *)&scalerInfo);
            register_dec_buf_msg->ret_code = ret;

            if (ret != RETCODE_SUCCESS) {
                VLOG(ERR, "[%s][%d] handle:%p DEC_SET_SCALER_INFO failed Error code is 0x%x \n",
                     __func__, __LINE__, handle, ret);
                return 0;
            }
        }

        ret = VPU_DecAllocateFrameBuffer(
            handle, fb_alloc_info_wtl,
            &instance->vpu_info.frame_buffers[register_dec_buf_msg->n_vb_frame_dec_phys_addr]);
        register_dec_buf_msg->ret_code = ret;

        if (ret != RETCODE_SUCCESS) {
            VLOG(ERR, "[%s][%d] handle:%p VPU_DecAllocateFrameBuffer failed Error code is 0x%x \n",
                 __func__, __LINE__, handle, ret);
            return 0;
        }
    }

    if (handle->coreIdx == VPU_WAVE412_IDX) {
        if (register_dec_buf_msg->has_vp9_res_change && register_dec_buf_msg->vp9_res_change)
            VPU_DecGiveCommand(handle, DEC_SET_INTER_RES_INFO_ON, NULL);

        ret = VPU_DecRegisterFrameBufferEx(handle, instance->vpu_info.frame_buffers,
                                           register_dec_buf_msg->n_vb_frame_dec_phys_addr,
                                           register_dec_buf_msg->n_vb_frame_wtl_phys_addr,
                                           fb_alloc_info_dec.stride, fb_alloc_info_dec.height,
                                           fb_alloc_info_dec.mapType);

        if (register_dec_buf_msg->has_vp9_res_change && register_dec_buf_msg->vp9_res_change)
            VPU_DecGiveCommand(handle, DEC_SET_INTER_RES_INFO_OFF, NULL);

        uint32_t val = (handle->codecMode == HEVC_DEC) ? SEQ_CHANGE_ENABLE_ALL_HEVC
                                                       : SEQ_CHANGE_ENABLE_ALL_VP9;
        VPU_DecGiveCommand(handle, DEC_SET_SEQ_CHANGE_MASK, (void *)&val);
    } else {
        ret = VPU_DecRegisterFrameBufferEx(handle, instance->vpu_info.frame_buffers,
                                           register_dec_buf_msg->n_vb_frame_dec_phys_addr,
                                           register_dec_buf_msg->n_vb_frame_wtl_phys_addr,
                                           fb_alloc_info_dec.stride, fb_alloc_info_dec.height,
                                           fb_alloc_info_dec.mapType);
    }

    register_dec_buf_msg->ret_code = ret;
    if (ret != RETCODE_SUCCESS) {
        VLOG(ERR, "handle:%p VPU_DecRegisterFrameBufferEx failed Error code is 0x%x \n", handle,
             ret);
        return 0;
    }
    return 0;
}

int vpu_instance_ipc_dec_set_rd_ptr(Vpu__DecSetRdPtr *set_rd_ptr_msg)
{
    if (!set_rd_ptr_msg->handle || !set_rd_ptr_msg->handle->handle || !set_rd_ptr_msg->has_addr) {
        VLOG(ERR, "[%s][%d] invailed param\n", __func__, __LINE__);
        return -1;
    }

    int ret = 0;
    int update_wr_ptr = 0;
    DecHandle handle = (DecHandle)set_rd_ptr_msg->handle->handle;
    VLOG(INFO, "[%s][%d] handle:%p\n", __func__, __LINE__, handle);

    update_wr_ptr = set_rd_ptr_msg->has_update_wr_ptr ? set_rd_ptr_msg->update_wr_ptr : 0;

    ret = VPU_DecSetRdPtr(handle, set_rd_ptr_msg->addr, update_wr_ptr);
    if (ret != RETCODE_SUCCESS)
        VLOG(ERR, "handle:%p VPU_DecSetRdPtr failed Error code is 0x%x \n", handle, ret);

    set_rd_ptr_msg->has_ret_code = TRUE;
    set_rd_ptr_msg->ret_code = ret;

    return 0;
}

void vpu_instance_debug_dec_output_info(DecHandle handle, DecOutputInfo *decodedInfo)
{
#define PIC_TYPE(picType)                                                                          \
    ((picType == PIC_TYPE_I)     ? 'I'                                                             \
     : (picType == PIC_TYPE_P)   ? 'P'                                                             \
     : (picType == PIC_TYPE_B)   ? 'B'                                                             \
     : (picType == PIC_TYPE_IDR) ? 'I'                                                             \
                                 : 'N')

    static int frameNo = 0;

    if (handle->coreIdx == VPU_WAVE412_IDX && handle->codecMode == HEVC_DEC) {
        printf("instance[%d:%d] %5d %d %4d(%4d) %3d %2d(%2d) %2d(%2d) %7d %08x %08x %08x %08x %08x "
               "%4dx%-4d %4d  %4d, frame: %c, cycle: %d\n",
               handle->coreIdx, handle->instIndex, frameNo, decodedInfo->picType,
               decodedInfo->h265Info.decodedPOC, decodedInfo->h265Info.displayPOC,
               decodedInfo->nalType, decodedInfo->indexFrameDecoded,
               decodedInfo->indexFrameDecodedForTiled, decodedInfo->indexFrameDisplay,
               decodedInfo->indexFrameDisplayForTiled, decodedInfo->indexFramePrescan,
               decodedInfo->frameDisplayFlag, decodedInfo->rdPtr, decodedInfo->wrPtr,
               decodedInfo->bytePosFrameStart, decodedInfo->bytePosFrameEnd,
               decodedInfo->dispPicWidth, decodedInfo->dispPicHeight, decodedInfo->sequenceNo,
               decodedInfo->h265Info.temporalId, PIC_TYPE(decodedInfo->picType),
               decodedInfo->frameCycle);
    } else
        return;

    frameNo++;

#undef PIC_TYPE
    return;
}

int vpu_instance_private_copy_dec_output_info(DecHandle handle, Vpu__DecOutputInfo *output_info_msg,
                                              DecOutputInfo *output_info, vpu_instance_t *instance)
{
    if (!handle || !output_info_msg || !output_info || !instance) {
        VLOG(ERR, "[%s][%d] invailed param\n", __func__, __LINE__);
        return -1;
    }

    // vpu_instance_debug_dec_output_info(handle, output_info);

    output_info_msg->rc_display = &instance->vpu_proto_info.dec_rc_display;
    *output_info_msg->rc_display = (Vpu__VpuRect)VPU__VPU_RECT__INIT;

    output_info_msg->disp_frame = &instance->vpu_proto_info.disp_frame;
    *output_info_msg->disp_frame = (Vpu__FrameBuffer)VPU__FRAME_BUFFER__INIT;

    /*
    indexFrameDecoded
    indexFrameDisplay
    indexFramePrescan
    decodingSuccess
    picType
    sequenceChanged
    rdPtr
    wrPtr
    streamEndFlag
    chunkReuseRequired
    dispFrame
    errorReason
    errorReasonExt
    sequenceNo
    frameDisplayFlag
    indexInterFrameDecoded
    decPicWidth
    decPicHeight
    dispPicWidth
    dispPicHeight
    consumedByte
    numOfErrMBs

    mp2ColorDescPresentFlag
    mp2ColorPrimaries
    mp2TransferChar
    mp2MatrixCoeff

    dispFrame //buf_y buf_cb buf_cr map_type ...
    rcDisplay  //left, top, right, bottom

    avcVuiInfo
    h265Info
    */

    output_info_msg->index_frame_display = output_info->indexFrameDisplay;
    output_info_msg->index_frame_decoded = output_info->indexFrameDecoded;
    output_info_msg->index_inter_frame_decoded = output_info->indexInterFrameDecoded;
    output_info_msg->pic_type = output_info->picType;
    output_info_msg->num_of_err_m_bs = output_info->numOfErrMBs;
    output_info_msg->decoding_success = output_info->decodingSuccess;
    output_info_msg->chunk_reuse_required = output_info->chunkReuseRequired;
    output_info_msg->disp_pic_width = output_info->dispPicWidth;
    output_info_msg->disp_pic_height = output_info->dispPicHeight;
    output_info_msg->dec_pic_width = output_info->decPicWidth;
    output_info_msg->dec_pic_height = output_info->decPicHeight;
    output_info_msg->consumed_byte = output_info->consumedByte;
    output_info_msg->rd_ptr = output_info->rdPtr;
    output_info_msg->wr_ptr = output_info->wrPtr;
    output_info_msg->frame_display_flag = output_info->frameDisplayFlag;
    output_info_msg->sequence_changed = output_info->sequenceChanged;
    output_info_msg->stream_end_flag = output_info->streamEndFlag;
    output_info_msg->error_reason = output_info->errorReason;
    output_info_msg->error_reason_ext = output_info->errorReasonExt;
    output_info_msg->sequence_no = output_info->sequenceNo;
    output_info_msg->index_frame_prescan = output_info->indexFramePrescan;
    output_info_msg->mp2_color_desc_present_flag = output_info->mp2ColorDescPresentFlag;
    output_info_msg->mp2_color_primaries = output_info->mp2ColorPrimaries;
    output_info_msg->mp2_transfer_char = output_info->mp2TransferChar;
    output_info_msg->mp2_matrix_coeff = output_info->mp2MatrixCoeff;

    output_info_msg->has_index_frame_display = TRUE;
    output_info_msg->has_index_frame_decoded = TRUE;
    output_info_msg->has_index_inter_frame_decoded = TRUE;
    output_info_msg->has_pic_type = TRUE;
    output_info_msg->has_num_of_err_m_bs = TRUE;
    output_info_msg->has_decoding_success = TRUE;
    output_info_msg->has_chunk_reuse_required = TRUE;
    output_info_msg->has_disp_pic_width = TRUE;
    output_info_msg->has_disp_pic_height = TRUE;
    output_info_msg->has_dec_pic_width = TRUE;
    output_info_msg->has_dec_pic_height = TRUE;
    output_info_msg->has_consumed_byte = TRUE;
    output_info_msg->has_rd_ptr = TRUE;
    output_info_msg->has_wr_ptr = TRUE;
    output_info_msg->has_frame_display_flag = TRUE;
    output_info_msg->has_sequence_changed = TRUE;
    output_info_msg->has_stream_end_flag = TRUE;
    output_info_msg->has_error_reason = TRUE;
    output_info_msg->has_error_reason_ext = TRUE;
    output_info_msg->has_sequence_no = TRUE;
    output_info_msg->has_index_frame_prescan = TRUE;
    output_info_msg->has_mp2_color_desc_present_flag = TRUE;
    output_info_msg->has_mp2_color_primaries = TRUE;
    output_info_msg->has_mp2_transfer_char = TRUE;
    output_info_msg->has_mp2_matrix_coeff = TRUE;

    output_info_msg->rc_display->left = output_info->rcDisplay.left;
    output_info_msg->rc_display->top = output_info->rcDisplay.top;
    output_info_msg->rc_display->right = output_info->rcDisplay.right;
    output_info_msg->rc_display->bottom = output_info->rcDisplay.bottom;

    output_info_msg->rc_display->has_left = TRUE;
    output_info_msg->rc_display->has_top = TRUE;
    output_info_msg->rc_display->has_right = TRUE;
    output_info_msg->rc_display->has_bottom = TRUE;

    output_info_msg->disp_frame->buf_y = output_info->dispFrame.bufY;
    output_info_msg->disp_frame->buf_cb = output_info->dispFrame.bufCb;
    output_info_msg->disp_frame->buf_cr = output_info->dispFrame.bufCr;
    output_info_msg->disp_frame->buf_y_bot = output_info->dispFrame.bufYBot;
    output_info_msg->disp_frame->buf_cb_bot = output_info->dispFrame.bufCbBot;
    output_info_msg->disp_frame->buf_cr_bot = output_info->dispFrame.bufCrBot;
    output_info_msg->disp_frame->buf_y_size = output_info->dispFrame.bufYSize;
    output_info_msg->disp_frame->buf_cb_size = output_info->dispFrame.bufCbSize;
    output_info_msg->disp_frame->buf_cr_size = output_info->dispFrame.bufCrSize;
    output_info_msg->disp_frame->cbcr_interleave = output_info->dispFrame.cbcrInterleave;
    output_info_msg->disp_frame->nv21 = output_info->dispFrame.nv21;
    output_info_msg->disp_frame->endian = output_info->dispFrame.endian;
    output_info_msg->disp_frame->my_index = output_info->dispFrame.myIndex;
    output_info_msg->disp_frame->map_type = output_info->dispFrame.mapType;
    output_info_msg->disp_frame->stride = output_info->dispFrame.stride;
    output_info_msg->disp_frame->width = output_info->dispFrame.width;
    output_info_msg->disp_frame->height = output_info->dispFrame.height;
    output_info_msg->disp_frame->size = output_info->dispFrame.size;
    output_info_msg->disp_frame->size_luma = output_info->dispFrame.sizeLuma;
    output_info_msg->disp_frame->size_chroma = output_info->dispFrame.sizeChroma;
    output_info_msg->disp_frame->luma_bit_depth = output_info->dispFrame.lumaBitDepth;
    output_info_msg->disp_frame->chroma_bit_depth = output_info->dispFrame.chromaBitDepth;
    output_info_msg->disp_frame->format = output_info->dispFrame.format;
    output_info_msg->disp_frame->source_l_burst_en = output_info->dispFrame.sourceLBurstEn;
    output_info_msg->disp_frame->src_buf_state = output_info->dispFrame.srcBufState;
    output_info_msg->disp_frame->sequence_no = output_info->dispFrame.sequenceNo;
    output_info_msg->disp_frame->packed_enable = output_info->dispFrame.packedEnable;
    output_info_msg->disp_frame->packed_mode = output_info->dispFrame.packedMode;
    output_info_msg->disp_frame->update_fb_info = output_info->dispFrame.updateFbInfo;

    output_info_msg->disp_frame->has_buf_y = TRUE;
    output_info_msg->disp_frame->has_buf_cb = TRUE;
    output_info_msg->disp_frame->has_buf_cr = TRUE;
    output_info_msg->disp_frame->has_buf_y_bot = TRUE;
    output_info_msg->disp_frame->has_buf_cb_bot = TRUE;
    output_info_msg->disp_frame->has_buf_cr_bot = TRUE;
    output_info_msg->disp_frame->has_buf_y_size = TRUE;
    output_info_msg->disp_frame->has_buf_cb_size = TRUE;
    output_info_msg->disp_frame->has_buf_cr_size = TRUE;
    output_info_msg->disp_frame->has_cbcr_interleave = TRUE;
    output_info_msg->disp_frame->has_nv21 = TRUE;
    output_info_msg->disp_frame->has_endian = TRUE;
    output_info_msg->disp_frame->has_my_index = TRUE;
    output_info_msg->disp_frame->has_map_type = TRUE;
    output_info_msg->disp_frame->has_stride = TRUE;
    output_info_msg->disp_frame->has_width = TRUE;
    output_info_msg->disp_frame->has_height = TRUE;
    output_info_msg->disp_frame->has_size = TRUE;
    output_info_msg->disp_frame->has_size_luma = TRUE;
    output_info_msg->disp_frame->has_size_chroma = TRUE;
    output_info_msg->disp_frame->has_luma_bit_depth = TRUE;
    output_info_msg->disp_frame->has_chroma_bit_depth = TRUE;
    output_info_msg->disp_frame->has_format = TRUE;
    output_info_msg->disp_frame->has_source_l_burst_en = TRUE;
    output_info_msg->disp_frame->has_src_buf_state = TRUE;
    output_info_msg->disp_frame->has_sequence_no = TRUE;
    output_info_msg->disp_frame->has_packed_enable = TRUE;
    output_info_msg->disp_frame->has_packed_mode = TRUE;
    output_info_msg->disp_frame->has_update_fb_info = TRUE;

    if ((handle->coreIdx == VPU_CODA988_IDX && handle->codecMode == AVC_DEC) ||
        (handle->coreIdx == VPU_WAVE521_IDX && handle->codecMode == W_AVC_DEC)) {
        output_info_msg->dec_codec_info_case = VPU__DEC_OUTPUT_INFO__DEC_CODEC_INFO_AVC_INFO;

        output_info_msg->avc_info = &instance->vpu_proto_info.dec_avc_info;
        *output_info_msg->avc_info = (Vpu__DecAvcInfo)VPU__DEC_AVC_INFO__INIT;

        output_info_msg->avc_info->avc_vui_info = &instance->vpu_proto_info.dec_avc_vui_info;
        *output_info_msg->avc_info->avc_vui_info = (Vpu__AvcVuiInfo)VPU__AVC_VUI_INFO__INIT;

        output_info_msg->avc_info->avc_vui_info->fixed_frame_rate_flag =
            output_info->avcVuiInfo.fixedFrameRateFlag;
        output_info_msg->avc_info->avc_vui_info->timing_info_present =
            output_info->avcVuiInfo.timingInfoPresent;
        output_info_msg->avc_info->avc_vui_info->chroma_loc_bot_field =
            output_info->avcVuiInfo.chromaLocBotField;
        output_info_msg->avc_info->avc_vui_info->chroma_loc_top_field =
            output_info->avcVuiInfo.chromaLocTopField;
        output_info_msg->avc_info->avc_vui_info->chroma_loc_info_present =
            output_info->avcVuiInfo.chromaLocInfoPresent;
        output_info_msg->avc_info->avc_vui_info->color_primaries =
            output_info->avcVuiInfo.colorPrimaries;
        output_info_msg->avc_info->avc_vui_info->color_desc_present =
            output_info->avcVuiInfo.colorDescPresent;
        output_info_msg->avc_info->avc_vui_info->is_ext_s_a_r = output_info->avcVuiInfo.isExtSAR;
        output_info_msg->avc_info->avc_vui_info->vid_full_range =
            output_info->avcVuiInfo.vidFullRange;
        output_info_msg->avc_info->avc_vui_info->vid_format = output_info->avcVuiInfo.vidFormat;
        output_info_msg->avc_info->avc_vui_info->vid_sig_type_present =
            output_info->avcVuiInfo.vidSigTypePresent;
        output_info_msg->avc_info->avc_vui_info->vui_param_present =
            output_info->avcVuiInfo.vuiParamPresent;
        output_info_msg->avc_info->avc_vui_info->vui_pic_struct_present =
            output_info->avcVuiInfo.vuiPicStructPresent;
        output_info_msg->avc_info->avc_vui_info->vui_pic_struct =
            output_info->avcVuiInfo.vuiPicStruct;
        output_info_msg->avc_info->avc_vui_info->vui_transfer_characteristics =
            output_info->avcVuiInfo.vuiTransferCharacteristics;
        output_info_msg->avc_info->avc_vui_info->vui_matrix_coefficients =
            output_info->avcVuiInfo.vuiMatrixCoefficients;

        output_info_msg->avc_info->avc_vui_info->has_fixed_frame_rate_flag = TRUE;
        output_info_msg->avc_info->avc_vui_info->has_timing_info_present = TRUE;
        output_info_msg->avc_info->avc_vui_info->has_chroma_loc_bot_field = TRUE;
        output_info_msg->avc_info->avc_vui_info->has_chroma_loc_top_field = TRUE;
        output_info_msg->avc_info->avc_vui_info->has_chroma_loc_info_present = TRUE;
        output_info_msg->avc_info->avc_vui_info->has_color_primaries = TRUE;
        output_info_msg->avc_info->avc_vui_info->has_color_desc_present = TRUE;
        output_info_msg->avc_info->avc_vui_info->has_is_ext_s_a_r = TRUE;
        output_info_msg->avc_info->avc_vui_info->has_vid_full_range = TRUE;
        output_info_msg->avc_info->avc_vui_info->has_vid_format = TRUE;
        output_info_msg->avc_info->avc_vui_info->has_vid_sig_type_present = TRUE;
        output_info_msg->avc_info->avc_vui_info->has_vui_param_present = TRUE;
        output_info_msg->avc_info->avc_vui_info->has_vui_pic_struct_present = TRUE;
        output_info_msg->avc_info->avc_vui_info->has_vui_pic_struct = TRUE;
        output_info_msg->avc_info->avc_vui_info->has_vui_transfer_characteristics = TRUE;
        output_info_msg->avc_info->avc_vui_info->has_vui_matrix_coefficients = TRUE;
    } else if ((handle->coreIdx == VPU_WAVE412_IDX && handle->codecMode == HEVC_DEC) ||
               (handle->coreIdx == VPU_WAVE521_IDX && handle->codecMode == W_HEVC_DEC)) {
        output_info_msg->dec_codec_info_case = VPU__DEC_OUTPUT_INFO__DEC_CODEC_INFO_HEVC_INFO;

        output_info_msg->hevc_info = &instance->vpu_proto_info.dec_hevc_info;
        *output_info_msg->hevc_info = (Vpu__DecHevcInfo)VPU__DEC_HEVC_INFO__INIT;

        output_info_msg->hevc_info->h265_info = &instance->vpu_proto_info.dec_hevc_h265_info;
        *output_info_msg->hevc_info->h265_info =
            (Vpu__DecHevcInfo__H265Info)VPU__DEC_HEVC_INFO__H265_INFO__INIT;

        output_info_msg->hevc_info->h265_info->decoded_p_o_c = output_info->h265Info.decodedPOC;
        output_info_msg->hevc_info->h265_info->display_p_o_c = output_info->h265Info.displayPOC;
        output_info_msg->hevc_info->h265_info->ctu_size = output_info->h265Info.ctuSize;
        output_info_msg->hevc_info->h265_info->temporal_id = output_info->h265Info.temporalId;
        output_info_msg->hevc_info->h265_info->color_primaries =
            output_info->h265Info.colorPrimaries;
        output_info_msg->hevc_info->h265_info->color_desc_present =
            output_info->h265Info.colorDescPresent;
        output_info_msg->hevc_info->h265_info->vid_full_range = output_info->h265Info.vidFullRange;
        output_info_msg->hevc_info->h265_info->vui_param_present =
            output_info->h265Info.vuiParamPresent;
        output_info_msg->hevc_info->h265_info->vui_transfer_characteristics =
            output_info->h265Info.vuiTransferCharacteristics;
        output_info_msg->hevc_info->h265_info->vui_matrix_coefficients =
            output_info->h265Info.vuiMatrixCoefficients;
        output_info_msg->hevc_info->h265_info->master_meta_present =
            output_info->h265Info.masterMetaPresent;
        output_info_msg->hevc_info->h265_info->display_primaries_x_b =
            output_info->h265Info.display_primaries_x[0];
        output_info_msg->hevc_info->h265_info->display_primaries_x_g =
            output_info->h265Info.display_primaries_x[1];
        output_info_msg->hevc_info->h265_info->display_primaries_x_r =
            output_info->h265Info.display_primaries_x[2];
        output_info_msg->hevc_info->h265_info->display_primaries_y_b =
            output_info->h265Info.display_primaries_y[0];
        output_info_msg->hevc_info->h265_info->display_primaries_y_g =
            output_info->h265Info.display_primaries_y[1];
        output_info_msg->hevc_info->h265_info->display_primaries_y_r =
            output_info->h265Info.display_primaries_y[2];
        output_info_msg->hevc_info->h265_info->white_point_x = output_info->h265Info.white_point_x;
        output_info_msg->hevc_info->h265_info->white_point_y = output_info->h265Info.white_point_y;
        output_info_msg->hevc_info->h265_info->max_display_mastering_luminance =
            output_info->h265Info.max_display_mastering_luminance;
        output_info_msg->hevc_info->h265_info->min_display_mastering_luminance =
            output_info->h265Info.min_display_mastering_luminance;
        output_info_msg->hevc_info->h265_info->content_meta_present =
            output_info->h265Info.contentMetaPresent;
        output_info_msg->hevc_info->h265_info->max_content_light_level =
            output_info->h265Info.max_content_light_level;
        output_info_msg->hevc_info->h265_info->max_pic_average_light_level =
            output_info->h265Info.max_pic_average_light_level;

        output_info_msg->hevc_info->h265_info->has_decoded_p_o_c = TRUE;
        output_info_msg->hevc_info->h265_info->has_display_p_o_c = TRUE;
        output_info_msg->hevc_info->h265_info->has_ctu_size = TRUE;
        output_info_msg->hevc_info->h265_info->has_temporal_id = TRUE;
        output_info_msg->hevc_info->h265_info->has_color_primaries = TRUE;
        output_info_msg->hevc_info->h265_info->has_color_desc_present = TRUE;
        output_info_msg->hevc_info->h265_info->has_vid_full_range = TRUE;
        output_info_msg->hevc_info->h265_info->has_vui_param_present = TRUE;
        output_info_msg->hevc_info->h265_info->has_vui_transfer_characteristics = TRUE;
        output_info_msg->hevc_info->h265_info->has_vui_matrix_coefficients = TRUE;
        output_info_msg->hevc_info->h265_info->has_master_meta_present = TRUE;
        output_info_msg->hevc_info->h265_info->has_display_primaries_x_b = TRUE;
        output_info_msg->hevc_info->h265_info->has_display_primaries_x_g = TRUE;
        output_info_msg->hevc_info->h265_info->has_display_primaries_x_r = TRUE;
        output_info_msg->hevc_info->h265_info->has_display_primaries_y_b = TRUE;
        output_info_msg->hevc_info->h265_info->has_display_primaries_y_g = TRUE;
        output_info_msg->hevc_info->h265_info->has_display_primaries_y_r = TRUE;
        output_info_msg->hevc_info->h265_info->has_white_point_x = TRUE;
        output_info_msg->hevc_info->h265_info->has_white_point_y = TRUE;
        output_info_msg->hevc_info->h265_info->has_max_display_mastering_luminance = TRUE;
        output_info_msg->hevc_info->h265_info->has_min_display_mastering_luminance = TRUE;
        output_info_msg->hevc_info->h265_info->has_content_meta_present = TRUE;
        output_info_msg->hevc_info->h265_info->has_max_content_light_level = TRUE;
        output_info_msg->hevc_info->h265_info->has_max_pic_average_light_level = TRUE;
    }

    // seq change
    if (output_info_msg->sequence_changed) {
        VLOG(INFO, "[%s][%d] sequence_changed, idx:%d\n", __func__, __LINE__, handle->coreIdx);
        if (handle->coreIdx == VPU_WAVE412_IDX) {
            int resolution_change = (output_info_msg->sequence_changed >> 16) & 0x01;
            int bit_depth_change = (output_info_msg->sequence_changed >> 18) & 0x01;
            int num_dpb_change = (output_info_msg->sequence_changed >> 19) & 0x01;

            VLOG(INFO, "[%s][%d] resolution_change:%d, bit_depth_change:%d, num_dpb_change:%d\n",
                 __func__, __LINE__, resolution_change, bit_depth_change, num_dpb_change);

            if (resolution_change || bit_depth_change || num_dpb_change) {
                VPU_DecFrameBufferFlush(handle, NULL, NULL);

                // free buffers inside API
                VPU_DecGiveCommand(handle, DEC_FREE_FRAME_BUFFER, NULL);
                VPU_DecGiveCommand(handle, DEC_RESET_FRAMEBUF_INFO, NULL);
            }
        } else if (handle->coreIdx == VPU_WAVE521_IDX) {
            int dpb_changed =
                (output_info_msg->sequence_changed & SEQ_CHANGE_ENABLE_DPB_COUNT) ? TRUE : FALSE;
            int size_changed =
                (output_info_msg->sequence_changed & SEQ_CHANGE_ENABLE_SIZE) ? TRUE : FALSE;
            int bit_depth_changed =
                (output_info_msg->sequence_changed & SEQ_CHANGE_ENABLE_BITDEPTH) ? TRUE : FALSE;
            int chroma_idc_changed =
                (output_info_msg->sequence_changed & SEQ_CHANGE_CHROMA_FORMAT_IDC) ? TRUE : FALSE;

            VLOG(INFO, "[%s][%d] dpb_changed:%d, size_changed:%d, bit_depth_changed:%d, chroma_idc_changed:%d\n",
                 __func__, __LINE__, dpb_changed, size_changed, bit_depth_changed, chroma_idc_changed);

            if (dpb_changed || size_changed || bit_depth_changed || chroma_idc_changed) {
                VPU_DecFrameBufferFlush(handle, NULL, NULL);

                // free buffers inside API
                VPU_DecGiveCommand(handle, DEC_FREE_FRAME_BUFFER, NULL);
                VPU_DecGiveCommand(handle, DEC_RESET_FRAMEBUF_INFO, NULL);
            }
        } else {
            // free all internal buffer
            VPU_DecGiveCommand(handle, DEC_FREE_FRAME_BUFFER, NULL);
        }
    }

    // special handling for vp9
    if (handle->codecMode == C7_VP9_DEC) {
        // chunk_reuse_required
        if (output_info_msg->rd_ptr < output_info_msg->wr_ptr)
            output_info_msg->chunk_reuse_required = TRUE;
        else
            output_info_msg->chunk_reuse_required = FALSE;

        // inter resolution changed
        if (output_info_msg->sequence_changed & (1 << 17) ||
            ((output_info_msg->index_frame_decoded == -1) &&
             (output_info_msg->index_inter_frame_decoded != -1))) {
            int32_t fbc_cur_frame_idx = 0;
            int32_t bwb_cur_frame_idx = 0;

            VLOG(INFO, "[%s][%d]vp9 inter resolution change\n", __func__, __LINE__);
            fbc_cur_frame_idx = (short)(output_info_msg->index_inter_frame_decoded & 0xffff);
            bwb_cur_frame_idx =
                (short)((output_info_msg->index_inter_frame_decoded >> 16) & 0xffff);

            VPU_DecGiveCommand(handle, DEC_SET_BWB_CUR_FRAME_IDX, (void *)&bwb_cur_frame_idx);
            VPU_DecGiveCommand(handle, DEC_SET_FBC_CUR_FRAME_IDX, (void *)&fbc_cur_frame_idx);

            if (output_info_msg->index_inter_frame_decoded != -1)
                for (int i = 0; i < MAX_GDI_IDX; i++)
                    VPU_DecClrDispFlag(handle, i);

            if (fbc_cur_frame_idx >= 0) {
                VPU_DecGiveCommand(handle, DEC_FREE_MV_BUFFER, (void *)&fbc_cur_frame_idx);
                VPU_DecGiveCommand(handle, DEC_FREE_FBC_TABLE_BUFFER, (void *)&fbc_cur_frame_idx);

                VPU_DecGiveCommand(handle, DEC_ALLOC_MV_BUFFER, (void *)&fbc_cur_frame_idx);
                VPU_DecGiveCommand(handle, DEC_ALLOC_FBC_Y_TABLE_BUFFER,
                                   (void *)&fbc_cur_frame_idx);
                VPU_DecGiveCommand(handle, DEC_ALLOC_FBC_C_TABLE_BUFFER,
                                   (void *)&fbc_cur_frame_idx);
            }

            if ((fbc_cur_frame_idx >= 0) || (bwb_cur_frame_idx >= 0)) {
                output_info_msg->chunk_reuse_required = TRUE;
                output_info_msg->sequence_changed |= 1 << 17;

                output_info_msg->vp9_inter_res_change = TRUE;
                output_info_msg->has_vp9_inter_res_change = TRUE;
                output_info_msg->vp9_inter_res_change_frame_idx = fbc_cur_frame_idx;
                output_info_msg->has_vp9_inter_res_change_frame_idx = TRUE;
            }
        }
    }

    return 0;
}

int vpu_instance_ipc_dec_one_frame(Vpu__DecOneFrame *dec_one_frame_msg,
                                   Vpu__DecOneFrame *dec_one_frame_msg_ret,
                                   vpu_instance_t *instance)
{
    if (!dec_one_frame_msg->handle || !dec_one_frame_msg->handle->handle || !instance) {
        VLOG(ERR, "[%s][%d] invailed handle\n", __func__, __LINE__);
        return -1;
    }

    int ret = 0;
    DecParam *param = NULL;
    DecOutputInfo *output_info = &instance->vpu_info.dec_output_info;
    DecHandle handle = (DecHandle)dec_one_frame_msg->handle->handle;

    *dec_one_frame_msg_ret = (Vpu__DecOneFrame)VPU__DEC_ONE_FRAME__INIT;
    dec_one_frame_msg_ret->handle = dec_one_frame_msg->handle;
    VLOG(INFO, "[%s][%d] handle:%p\n", __func__, __LINE__, handle);

    // VPU_DecStartOneFrame
    param = &instance->vpu_info.dec_param;
    memset(param, 0x0, sizeof(DecParam));

    if (dec_one_frame_msg->dec_param) {
        Vpu__DecParam *dec_param_msg = dec_one_frame_msg->dec_param;

        if (dec_param_msg->has_iframe_search_enable)
            param->iframeSearchEnable = dec_param_msg->iframe_search_enable;
        if (dec_param_msg->has_skipframe_mode)
            param->skipframeMode = dec_param_msg->skipframe_mode;
        if (dec_param_msg->has_cra_as_bla_flag)
            param->craAsBlaFlag = dec_param_msg->cra_as_bla_flag;
        if (dec_param_msg->has_pixel_luma_pad)
            param->pixelLumaPad = dec_param_msg->pixel_luma_pad;
        if (dec_param_msg->has_pixel_chroma_pad)
            param->pixelChromaPad = dec_param_msg->pixel_chroma_pad;
        if (dec_param_msg->has_sram_mode)
            param->sramMode = dec_param_msg->sram_mode;

        if (dec_param_msg->has_thumbnail_mode && dec_param_msg->thumbnail_mode)
            VPU_DecGiveCommand(handle, ENABLE_DEC_THUMBNAIL_MODE, 0);

        if (dec_param_msg->has_clr_disp_flag) {
            for (int i = 0; i < MAX_GDI_IDX; i++) {
                if ((dec_param_msg->clr_disp_flag >> i) & 0x01)
                    VPU_DecClrDispFlag(handle, i);
            }
        }

        if (dec_param_msg->has_set_disp_flag) {
            for (int i = 0; i < MAX_GDI_IDX; i++) {
                if ((dec_param_msg->set_disp_flag >> i) & 0x01)
                    VPU_DecGiveCommand(handle, DEC_SET_DISPLAY_FLAG, &i);
            }
        }
    }

    ret = VPU_DecStartOneFrame(handle, param);
    dec_one_frame_msg_ret->has_ret_code = TRUE;
    dec_one_frame_msg_ret->ret_code = ret;

    if (ret != RETCODE_SUCCESS) {
        VPU_DecGetOutputInfo(handle, output_info);
        VLOG(ERR, "handle:%p VPU_DecStartOneFrame failed Error code is 0x%x \n", handle, ret);
        return 0;
    }

    // VPU_WaitInterrupt
    if (handle->coreIdx == VPU_WAVE521_IDX) {
        ret = wave5_dec_hand_interrupt(handle);
        dec_one_frame_msg_ret->has_interrupt_reaseon = TRUE;
        dec_one_frame_msg_ret->interrupt_reaseon = ret;

        if (ret == DEC_INT_STATUS_TIMEOUT) {
            dec_one_frame_msg_ret->ret_code = RETCODE_FAILURE;
            VPU_DecGetOutputInfo(handle, output_info);
            // the third parameter of VPU_SWReset requires the pending instance
            VPU_SWReset(handle->coreIdx, SW_RESET_SAFETY, NULL);
            VLOG(ERR,
                 "[%s][%d] handle:%p VPU_WaitInterrupt timeout, ret is 0x%x rdPtr:%x wrPtr:%x\n",
                 __func__, __LINE__, handle, ret, output_info->rdPtr, output_info->wrPtr);
            return 0;
        } else if (ret != DEC_INT_STATUS_DONE) {
            dec_one_frame_msg_ret->ret_code = RETCODE_FAILURE;
            VPU_DecGetOutputInfo(handle, output_info);
            VLOG(ERR,
                 "[%s][%d] handle:%p VPU_WaitInterrupt timeout, ret is 0x%x rdPtr:%x wrPtr:%x\n",
                 __func__, __LINE__, handle, ret, output_info->rdPtr, output_info->wrPtr);
            return 0;
        }
    } else {
        ret = VPU_WaitInterrupt(handle->coreIdx, VPU_DEC_TIMEOUT);
        dec_one_frame_msg_ret->has_interrupt_reaseon = TRUE;
        dec_one_frame_msg_ret->interrupt_reaseon = ret;

        if (ret == -1) {
            dec_one_frame_msg_ret->ret_code = RETCODE_FAILURE;
            VPU_DecGetOutputInfo(handle, output_info);
            // the third parameter of VPU_SWReset requires the pending instance
            VPU_SWReset(handle->coreIdx, SW_RESET_SAFETY, NULL);
            VLOG(ERR, "handle:%p VPU_WaitInterrupt timeout, ret is 0x%x rdPtr:%x wrPtr:%x\n",
                 handle, ret, output_info->rdPtr, output_info->wrPtr);
            return 0;
        }

        // Field mode is not supported yet
        if (ret & (1 << INT_BIT_DEC_FIELD)) {
            VLOG(ERR, "[%s][%d] Field mode is not supported\n", __func__, __LINE__);
            // if (handle->codecMode == C7_VP9_DEC) {
            //     VLOG(ERR, "[%s][%d] handle:%p Detect PRESCAN-ERROR\n", __func__, __LINE__,
            //     handle); dec_one_frame_msg_ret->ret_code = RETCODE_FAILURE;
            //     VPU_DecGetOutputInfo(handle, output_info);
            // } else {
            //     uint32_t rd_ptr, wr_ptr, room;
            //     VPU_DecGetBitstreamBuffer(handle, &rd_ptr, &wr_ptr, &room);
            //     VLOG(INFO, "[%s][%d] handle:%p rd_ptr:%x, wr_ptr:%x\n", __func__, __LINE__,
            //     handle,
            //          rd_ptr, wr_ptr);
            //     if (rd_ptr == wr_ptr) {
            //         // do not clear interrupt until feeding next field picture.
            //     } else {
            //         dec_one_frame_msg_ret->ret_code = RETCODE_FAILURE;
            //         VPU_DecGetOutputInfo(handle, output_info);
            //     }
            // }
            // return 0;
        }

        if (ret)
            VPU_ClearInterrupt(handle->coreIdx);

        if (ret & (1 << INT_BIT_PIC_RUN)) {
            // normal state
        } else {
            dec_one_frame_msg_ret->ret_code = RETCODE_FAILURE;
            VPU_DecGetOutputInfo(handle, output_info);
            VLOG(ERR,
                 "handle:%p VPU_WaitInterrupt flag is:%x, instead of PIC_RUN, rdPtr:%x wrPtr:%x\n",
                 handle, ret, output_info->rdPtr, output_info->wrPtr);
            return 0;
        }
    }

    // VPU_DecGetOutputInfo
    memset(output_info, 0x0, sizeof(DecOutputInfo));
    ret = VPU_DecGetOutputInfo(handle, output_info);
    dec_one_frame_msg_ret->ret_code = ret;

    if (ret != RETCODE_SUCCESS) {
        VLOG(ERR, "handle:%p VPU_DecGetOutputInfo failed Error code is 0x%x \n", handle, ret);
        return 0;
    }

    Vpu__DecOutputInfo *output_info_msg = &instance->vpu_proto_info.dec_output_info;

    *output_info_msg = (Vpu__DecOutputInfo)VPU__DEC_OUTPUT_INFO__INIT;
    dec_one_frame_msg_ret->dec_output_info = output_info_msg;

    vpu_instance_private_copy_dec_output_info(handle, output_info_msg, output_info, instance);

    return 0;
}

// kernel will tall us to clear isntance's resource when vpu client process crash
int vpu_instance_ipc_clear_instance(vpu_instance_t *instance, uint32_t handle)
{
    int ret = 0;

    if (!instance || !handle) {
        VLOG(ERR, "[%s][%d] invailed param\n", __func__, __LINE__);
        return -1;
    }

    int index = 0;
    VLOG(INFO, "[%s][%d] handle:%x\n", __func__, __LINE__, handle);

    for (; index < VPU_MAX_NUM_INSTANCE; index++) {
        if (instance->vpu_instance_contrl.inst_handle[index] == (uint32_t)handle)
            break;
    }

    if (index >= VPU_MAX_NUM_INSTANCE) {
        VLOG(WARN, "[%s][%d] invaild handle:%p\n", __func__, __LINE__, handle);
        return -1;
    }

    if (((VpuHandle)handle)->isDecoder)
        ret = VPU_DecClose((DecHandle)handle);
    else
        ret = VPU_EncClose((EncHandle)handle);

    if (ret != RETCODE_SUCCESS)
        VLOG(ERR, "[%s][%d] failed, handle:%p Error code is 0x%x \n", __func__, __LINE__,
             (VpuHandle)handle, ret);

    instance->vpu_instance_contrl.inst_handle[index] = 0;
    instance->vpu_instance_contrl.inst_score[index] = 0;
    instance->vpu_instance_contrl.inst_enable_sram[index] = 0;

    return 0;
}

int vpu_instance_ipc_enc_open(vpu_instance_t *instance, Vpu__EncOpen *enc_open_msg)
{
    int ret = 0;
    int index = 0;
    EncOpenParam enc_open_param = {0};
    SecAxiUse sec_axi_use = {0};
    EncHandle handle = NULL;
    Vpu__EncOpenParam *enc_op = NULL;

    if (!enc_open_msg->handle || !enc_open_msg->enc_op) {
        VLOG(ERR, "[%s][%d] invailed handle\n", __func__, __LINE__);
        return -1;
    }

    enc_op = enc_open_msg->enc_op;

    // codec resource control
    {
        int fps;
        uint32_t total_score = 0;
        int is_enable_sram = 0;

        for (; index < VPU_MAX_NUM_INSTANCE; index++) {
            if (instance->vpu_instance_contrl.inst_handle[index] == 0)
                break;
        }

        if (index >= VPU_MAX_NUM_INSTANCE) {
            VLOG(ERR, "[%s][%d] the maximum number:%d of open instances has been reached\n",
                 __func__, __LINE__, VPU_MAX_NUM_INSTANCE);

            enc_open_msg->has_ret_code = TRUE;
            enc_open_msg->ret_code = RETCODE_RESOURECE_CONSTRAINTS;
            return 0;
        }

        fps = enc_op->frame_rate_info;
        if (!fps)
            fps = 20; // defalut value for 4K encoder

        if (enc_op->has_sram_enable && enc_op->sram_enable && enc_op->core_idx == VPU_CODA988_IDX)
            is_enable_sram = TRUE;

        // score = width * height * fps * encode_compensation_value * sram_compensation_value + 1
        if (enc_op->core_idx == VPU_CODA988_IDX) {
            instance->vpu_instance_contrl.inst_score[index] =
                VPU_ALIGN32(enc_op->pic_width) * VPU_ALIGN32(enc_op->pic_height) * fps *
                    VPU_CODA988_ENABLE_ABILITY_COMPENSATION_VALUE *
                    (is_enable_sram ? 1 : VPU_CODA988_DISABLE_SRAM_ABILITY_COMPENSATION_VALUE) +
                1;
        } else {
            // wave521
            instance->vpu_instance_contrl.inst_score[index] =
                VPU_ALIGN32(enc_op->pic_width) * VPU_ALIGN32(enc_op->pic_height) * fps + 1;
        }

        total_score += instance->vpu_instance_contrl.inst_score[index];

        for (int i = 0; i < VPU_MAX_NUM_INSTANCE; i++) {
            if (instance->vpu_instance_contrl.inst_handle[i])
                total_score += instance->vpu_instance_contrl.inst_score[i];
        }

        if (total_score == instance->vpu_instance_contrl.inst_score[index]) {
            // at least one instance is need supported
        } else if (total_score > instance->vpu_instance_contrl.codec_ability_score) {
            VLOG(ERR,
                 "[%s][%d] total score:%u > codec ability score:%u, current instance score:%d\n",
                 __func__, __LINE__, total_score, instance->vpu_instance_contrl.codec_ability_score,
                 instance->vpu_instance_contrl.inst_score[index]);

            instance->vpu_instance_contrl.inst_score[index] = 0;
            enc_open_msg->has_ret_code = TRUE;
            enc_open_msg->ret_code = RETCODE_RESOURECE_CONSTRAINTS;
            return 0;
        }
    }

    // enc open
    if (enc_op->has_bitstream_buffer)
        enc_open_param.bitstreamBuffer = enc_op->bitstream_buffer;
    if (enc_op->has_bitstream_buffer_size)
        enc_open_param.bitstreamBufferSize = enc_op->bitstream_buffer_size;
    if (enc_op->has_bitstream_format)
        enc_open_param.bitstreamFormat = enc_op->bitstream_format;
    if (enc_op->has_ring_buffer_enable)
        enc_open_param.ringBufferEnable = enc_op->ring_buffer_enable;
    if (enc_op->has_pic_width)
        enc_open_param.picWidth = enc_op->pic_width;
    if (enc_op->has_pic_height)
        enc_open_param.picHeight = enc_op->pic_height;
    if (enc_op->has_linear2_tiled_enable)
        enc_open_param.linear2TiledEnable = enc_op->linear2_tiled_enable;
    if (enc_op->has_linear2_tiled_mode)
        enc_open_param.linear2TiledMode = enc_op->linear2_tiled_mode;
    if (enc_op->has_frame_rate_info)
        enc_open_param.frameRateInfo = enc_op->frame_rate_info;
    if (enc_op->has_me_search_range_x)
        enc_open_param.MESearchRangeX = enc_op->me_search_range_x;
    if (enc_op->has_me_search_range_y)
        enc_open_param.MESearchRangeY = enc_op->me_search_range_y;
    if (enc_op->has_rc_gop_i_qp_offset_en)
        enc_open_param.rcGopIQpOffsetEn = enc_op->rc_gop_i_qp_offset_en;
    if (enc_op->has_rc_gop_i_qp_offset)
        enc_open_param.rcGopIQpOffset = enc_op->rc_gop_i_qp_offset;
    if (enc_op->has_me_search_range)
        enc_open_param.MESearchRange = enc_op->me_search_range;
    if (enc_op->has_vbv_buffer_size)
        enc_open_param.vbvBufferSize = enc_op->vbv_buffer_size;
    if (enc_op->has_frame_skip_disable)
        enc_open_param.frameSkipDisable = enc_op->frame_skip_disable;
    if (enc_op->has_gop_size)
        enc_open_param.gopSize = enc_op->gop_size;
    if (enc_op->has_idr_interval)
        enc_open_param.idrInterval = enc_op->idr_interval;
    if (enc_op->has_me_blk_mode)
        enc_open_param.meBlkMode = enc_op->me_blk_mode;

    if (enc_op->has_intra_refresh)
        enc_open_param.intraRefresh = enc_op->intra_refresh;
    if (enc_op->has_consc_intra_refresh_enable)
        enc_open_param.ConscIntraRefreshEnable = enc_op->consc_intra_refresh_enable;
    if (enc_op->has_count_intra_mb_enable)
        enc_open_param.CountIntraMbEnable = enc_op->count_intra_mb_enable;
    if (enc_op->has_field_seq_intra_refresh_enable)
        enc_open_param.FieldSeqIntraRefreshEnable = enc_op->field_seq_intra_refresh_enable;
    if (enc_op->has_user_qp_max)
        enc_open_param.userQpMax = enc_op->user_qp_max;
    if (enc_op->has_max_intra_size)
        enc_open_param.maxIntraSize = enc_op->max_intra_size;
    if (enc_op->has_user_max_delta_qp)
        enc_open_param.userMaxDeltaQp = enc_op->user_max_delta_qp;
    if (enc_op->has_user_qp_min)
        enc_open_param.userQpMin = enc_op->user_qp_min;
    if (enc_op->has_user_min_delta_qp)
        enc_open_param.userMinDeltaQp = enc_op->user_min_delta_qp;
    if (enc_op->has_me_use_zero_pmv)
        enc_open_param.MEUseZeroPmv = enc_op->me_use_zero_pmv;
    if (enc_op->has_intra_cost_weight)
        enc_open_param.intraCostWeight = enc_op->intra_cost_weight;
    if (enc_op->has_rc_intra_qp)
        enc_open_param.rcIntraQp = enc_op->rc_intra_qp;
    if (enc_op->has_user_gamma)
        enc_open_param.userGamma = enc_op->user_gamma;
    if (enc_op->has_rc_interval_mode)
        enc_open_param.rcIntervalMode = enc_op->rc_interval_mode;
    if (enc_op->has_mb_interval)
        enc_open_param.mbInterval = enc_op->mb_interval;
    if (enc_op->has_bit_rate)
        enc_open_param.bitRate = enc_op->bit_rate;
    if (enc_op->has_bit_rate_bl)
        enc_open_param.bitRateBL = enc_op->bit_rate_bl;
    if (enc_op->has_initial_delay)
        enc_open_param.initialDelay = enc_op->initial_delay;
    if (enc_op->has_rc_enable)
        enc_open_param.rcEnable = enc_op->rc_enable;

    if (enc_op->has_frame_cache_bypass)
        enc_open_param.frameCacheBypass = enc_op->frame_cache_bypass;
    if (enc_op->has_frame_cache_burst)
        enc_open_param.frameCacheBurst = enc_op->frame_cache_burst;
    if (enc_op->has_frame_cache_merge)
        enc_open_param.frameCacheMerge = enc_op->frame_cache_merge;
    if (enc_op->has_frame_cache_way_shape)
        enc_open_param.frameCacheWayShape = enc_op->frame_cache_way_shape;
    if (enc_op->has_cache_bypass)
        enc_open_param.cacheBypass = enc_op->cache_bypass;
    if (enc_op->has_cbcr_interleave)
        enc_open_param.cbcrInterleave = enc_op->cbcr_interleave;
    if (enc_op->has_cbcr_order)
        enc_open_param.cbcrOrder = enc_op->cbcr_order;
    if (enc_op->has_frame_endian)
        enc_open_param.frameEndian = enc_op->frame_endian;
    if (enc_op->has_stream_endian)
        enc_open_param.streamEndian = enc_op->stream_endian;
    if (enc_op->has_source_endian)
        enc_open_param.sourceEndian = enc_op->source_endian;
    if (enc_op->has_bwb_enable)
        enc_open_param.bwbEnable = enc_op->bwb_enable;
    if (enc_op->has_line_buf_int_en)
        enc_open_param.lineBufIntEn = enc_op->line_buf_int_en;
    if (enc_op->has_packed_format)
        enc_open_param.packedFormat = enc_op->packed_format;
    if (enc_op->has_src_format)
        enc_open_param.srcFormat = enc_op->src_format;
    if (enc_op->has_output_format)
        enc_open_param.outputFormat = enc_op->output_format;
    if (enc_op->has_src_bit_depth)
        enc_open_param.srcBitDepth = enc_op->src_bit_depth;
    if (enc_op->has_core_idx)
        enc_open_param.coreIdx = enc_op->core_idx;
    if (enc_op->has_nv21)
        enc_open_param.nv21 = enc_op->nv21;
    if (enc_op->has_encode_hrd_rbsp_in_vps)
        enc_open_param.encodeHrdRbspInVPS = enc_op->encode_hrd_rbsp_in_vps;
    if (enc_op->has_encode_hrd_rbsp_in_vui)
        enc_open_param.encodeHrdRbspInVUI = enc_op->encode_hrd_rbsp_in_vui;
    if (enc_op->has_hrd_rbsp_data_size)
        enc_open_param.hrdRbspDataSize = enc_op->hrd_rbsp_data_size;
    if (enc_op->has_hrd_rbsp_data_addr)
        enc_open_param.hrdRbspDataAddr = enc_op->hrd_rbsp_data_addr;
    if (enc_op->has_encode_vui_rbsp)
        enc_open_param.encodeVuiRbsp = enc_op->encode_vui_rbsp;
    if (enc_op->has_vui_rbsp_data_size)
        enc_open_param.vuiRbspDataSize = enc_op->vui_rbsp_data_size;
    if (enc_op->has_vui_rbsp_data_addr)
        enc_open_param.vuiRbspDataAddr = enc_op->vui_rbsp_data_addr;
    if (enc_op->has_virt_axi_id)
        enc_open_param.virtAxiID = enc_op->virt_axi_id;
    if (enc_op->has_enable_pts)
        enc_open_param.enablePTS = enc_op->enable_pts;
    if (enc_op->has_enable_non_ref_fbc_write)
        enc_open_param.enableNonRefFbcWrite = enc_op->enable_non_ref_fbc_write;
    if (enc_op->has_pic_width_bl)
        enc_open_param.picWidthBL = enc_op->pic_width_bl;
    if (enc_op->has_pic_height_bl)
        enc_open_param.picHeightBL = enc_op->pic_height_bl;
    if (enc_op->has_source_buf_count)
        enc_open_param.sourceBufCount = enc_op->source_buf_count;
    if (enc_op->has_stream_buf_count)
        enc_open_param.streamBufCount = enc_op->stream_buf_count;
    if (enc_op->has_stream_buf_size)
        enc_open_param.streamBufSize = enc_op->stream_buf_size;
    if (enc_op->has_rc_weight_factor)
        enc_open_param.rcWeightFactor = enc_op->rc_weight_factor;
    // if (enc_op->has_src_release_int_enable)  // SUPPORT_SOURCE_RELEASE_INTERRUPT
    //     enc_open_param.srcReleaseIntEnable = enc_op->src_release_int_enable;
    if (enc_op->has_ring_buffer_wrap_enable)
        enc_open_param.ringBufferWrapEnable = enc_op->ring_buffer_wrap_enable;

    if (enc_op->slice_mode) {
        if (enc_op->slice_mode->has_slice_mode)
            enc_open_param.sliceMode.sliceMode = enc_op->slice_mode->slice_mode;
        if (enc_op->slice_mode->has_slice_size_mode)
            enc_open_param.sliceMode.sliceSizeMode = enc_op->slice_mode->slice_size_mode;
        if (enc_op->slice_mode->has_slice_size)
            enc_open_param.sliceMode.sliceSize = enc_op->slice_mode->slice_size;
    }

    if (enc_op->enc_std_param) {
        if (enc_op->enc_std_param->param_case == VPU__ENC_STD_PARAM__PARAM_MP4_PARAM &&
            enc_op->enc_std_param->mp4_param) {
            if (enc_op->enc_std_param->mp4_param->has_mp4_data_partition_enable)
                enc_open_param.EncStdParam.mp4Param.mp4DataPartitionEnable =
                    enc_op->enc_std_param->mp4_param->mp4_data_partition_enable;
            if (enc_op->enc_std_param->mp4_param->has_mp4_reversible_vlc_enable)
                enc_open_param.EncStdParam.mp4Param.mp4ReversibleVlcEnable =
                    enc_op->enc_std_param->mp4_param->mp4_reversible_vlc_enable;
            if (enc_op->enc_std_param->mp4_param->has_mp4_intra_dc_vlc_thr)
                enc_open_param.EncStdParam.mp4Param.mp4IntraDcVlcThr =
                    enc_op->enc_std_param->mp4_param->mp4_intra_dc_vlc_thr;
            if (enc_op->enc_std_param->mp4_param->has_mp4_hec_enable)
                enc_open_param.EncStdParam.mp4Param.mp4HecEnable =
                    enc_op->enc_std_param->mp4_param->mp4_hec_enable;
            if (enc_op->enc_std_param->mp4_param->has_mp4_verid)
                enc_open_param.EncStdParam.mp4Param.mp4Verid =
                    enc_op->enc_std_param->mp4_param->mp4_verid;
        } else if (enc_op->enc_std_param->param_case == VPU__ENC_STD_PARAM__PARAM_H263_PARAM &&
                   enc_op->enc_std_param->h263_param) {
            if (enc_op->enc_std_param->h263_param->has_h263_annex_i_enable)
                enc_open_param.EncStdParam.h263Param.h263AnnexIEnable =
                    enc_op->enc_std_param->h263_param->h263_annex_i_enable;
            if (enc_op->enc_std_param->h263_param->has_h263_annex_j_enable)
                enc_open_param.EncStdParam.h263Param.h263AnnexJEnable =
                    enc_op->enc_std_param->h263_param->h263_annex_j_enable;
            if (enc_op->enc_std_param->h263_param->has_h263_annex_k_enable)
                enc_open_param.EncStdParam.h263Param.h263AnnexKEnable =
                    enc_op->enc_std_param->h263_param->h263_annex_k_enable;
            if (enc_op->enc_std_param->h263_param->has_h263_annex_t_enable)
                enc_open_param.EncStdParam.h263Param.h263AnnexTEnable =
                    enc_op->enc_std_param->h263_param->h263_annex_t_enable;
        } else if (enc_op->enc_std_param->param_case == VPU__ENC_STD_PARAM__PARAM_AVC_PARAM &&
                   enc_op->enc_std_param->avc_param) {
            if (enc_op->enc_std_param->avc_param->has_constrained_intra_pred_flag)
                enc_open_param.EncStdParam.avcParam.constrainedIntraPredFlag =
                    enc_op->enc_std_param->avc_param->constrained_intra_pred_flag;
            if (enc_op->enc_std_param->avc_param->has_disable_deblk)
                enc_open_param.EncStdParam.avcParam.disableDeblk =
                    enc_op->enc_std_param->avc_param->disable_deblk;
            if (enc_op->enc_std_param->avc_param->has_deblk_filter_offset_alpha)
                enc_open_param.EncStdParam.avcParam.deblkFilterOffsetAlpha =
                    enc_op->enc_std_param->avc_param->deblk_filter_offset_alpha;
            if (enc_op->enc_std_param->avc_param->has_deblk_filter_offset_beta)
                enc_open_param.EncStdParam.avcParam.deblkFilterOffsetBeta =
                    enc_op->enc_std_param->avc_param->deblk_filter_offset_beta;
            if (enc_op->enc_std_param->avc_param->has_chroma_qp_offset)
                enc_open_param.EncStdParam.avcParam.chromaQpOffset =
                    enc_op->enc_std_param->avc_param->chroma_qp_offset;
            if (enc_op->enc_std_param->avc_param->has_aud_enable)
                enc_open_param.EncStdParam.avcParam.audEnable =
                    enc_op->enc_std_param->avc_param->aud_enable;
            if (enc_op->enc_std_param->avc_param->has_frame_cropping_flag)
                enc_open_param.EncStdParam.avcParam.frameCroppingFlag =
                    enc_op->enc_std_param->avc_param->frame_cropping_flag;
            if (enc_op->enc_std_param->avc_param->has_frame_crop_left)
                enc_open_param.EncStdParam.avcParam.frameCropLeft =
                    enc_op->enc_std_param->avc_param->frame_crop_left;
            if (enc_op->enc_std_param->avc_param->has_frame_crop_right)
                enc_open_param.EncStdParam.avcParam.frameCropRight =
                    enc_op->enc_std_param->avc_param->frame_crop_right;
            if (enc_op->enc_std_param->avc_param->has_frame_crop_top)
                enc_open_param.EncStdParam.avcParam.frameCropTop =
                    enc_op->enc_std_param->avc_param->frame_crop_top;
            if (enc_op->enc_std_param->avc_param->has_frame_crop_bottom)
                enc_open_param.EncStdParam.avcParam.frameCropBottom =
                    enc_op->enc_std_param->avc_param->frame_crop_bottom;
            if (enc_op->enc_std_param->avc_param->has_level)
                enc_open_param.EncStdParam.avcParam.level = enc_op->enc_std_param->avc_param->level;
            if (enc_op->enc_std_param->avc_param->has_profile)
                enc_open_param.EncStdParam.avcParam.profile =
                    enc_op->enc_std_param->avc_param->profile;
            if (enc_op->enc_std_param->avc_param->has_field_flag)
                enc_open_param.EncStdParam.avcParam.fieldFlag =
                    enc_op->enc_std_param->avc_param->field_flag;
            if (enc_op->enc_std_param->avc_param->has_field_ref_mode)
                enc_open_param.EncStdParam.avcParam.fieldRefMode =
                    enc_op->enc_std_param->avc_param->field_ref_mode;
            if (enc_op->enc_std_param->avc_param->has_chroma_format400)
                enc_open_param.EncStdParam.avcParam.chromaFormat400 =
                    enc_op->enc_std_param->avc_param->chroma_format400;
            if (enc_op->enc_std_param->avc_param->has_pps_num)
                enc_open_param.EncStdParam.avcParam.ppsNum =
                    enc_op->enc_std_param->avc_param->pps_num;
            if (enc_op->enc_std_param->avc_param->pps_param_1) {
                if (enc_op->enc_std_param->avc_param->pps_param_1->has_pps_id)
                    enc_open_param.EncStdParam.avcParam.ppsParam[0].ppsId =
                        enc_op->enc_std_param->avc_param->pps_param_1->pps_id;
                if (enc_op->enc_std_param->avc_param->pps_param_1->has_entropy_coding_mode)
                    enc_open_param.EncStdParam.avcParam.ppsParam[0].entropyCodingMode =
                        enc_op->enc_std_param->avc_param->pps_param_1->entropy_coding_mode;
                if (enc_op->enc_std_param->avc_param->pps_param_1->has_cabac_init_idc)
                    enc_open_param.EncStdParam.avcParam.ppsParam[0].cabacInitIdc =
                        enc_op->enc_std_param->avc_param->pps_param_1->cabac_init_idc;
                if (enc_op->enc_std_param->avc_param->pps_param_1->has_transform8x8_mode)
                    enc_open_param.EncStdParam.avcParam.ppsParam[0].transform8x8Mode =
                        enc_op->enc_std_param->avc_param->pps_param_1->transform8x8_mode;
            }
            if (enc_op->enc_std_param->avc_param->pps_param_2) {
                if (enc_op->enc_std_param->avc_param->pps_param_2->has_pps_id)
                    enc_open_param.EncStdParam.avcParam.ppsParam[1].ppsId =
                        enc_op->enc_std_param->avc_param->pps_param_2->pps_id;
                if (enc_op->enc_std_param->avc_param->pps_param_2->has_entropy_coding_mode)
                    enc_open_param.EncStdParam.avcParam.ppsParam[1].entropyCodingMode =
                        enc_op->enc_std_param->avc_param->pps_param_2->entropy_coding_mode;
                if (enc_op->enc_std_param->avc_param->pps_param_2->has_cabac_init_idc)
                    enc_open_param.EncStdParam.avcParam.ppsParam[1].cabacInitIdc =
                        enc_op->enc_std_param->avc_param->pps_param_2->cabac_init_idc;
                if (enc_op->enc_std_param->avc_param->pps_param_2->has_transform8x8_mode)
                    enc_open_param.EncStdParam.avcParam.ppsParam[1].transform8x8Mode =
                        enc_op->enc_std_param->avc_param->pps_param_2->transform8x8_mode;
            }
        } else if (enc_op->enc_std_param->param_case == VPU__ENC_STD_PARAM__PARAM_HEVC_PARAM &&
                   enc_op->enc_std_param->hevc_param) {
            /*
             *TODO: add hevc param proto & handler
             */
        } else if (enc_op->enc_std_param->param_case == VPU__ENC_STD_PARAM__PARAM_WAVE_PARAM &&
                   enc_op->enc_std_param->wave_param) {

            if (enc_op->enc_std_param->wave_param->has_profile)
                enc_open_param.EncStdParam.waveParam.profile =
                    enc_op->enc_std_param->wave_param->profile;
            if (enc_op->enc_std_param->wave_param->has_en_still_picture)
                enc_open_param.EncStdParam.waveParam.enStillPicture =
                    enc_op->enc_std_param->wave_param->en_still_picture;
            if (enc_op->enc_std_param->wave_param->has_level)
                enc_open_param.EncStdParam.waveParam.level =
                    enc_op->enc_std_param->wave_param->level;
            if (enc_op->enc_std_param->wave_param->has_tier)
                enc_open_param.EncStdParam.waveParam.tier = enc_op->enc_std_param->wave_param->tier;
            if (enc_op->enc_std_param->wave_param->has_internal_bit_depth)
                enc_open_param.EncStdParam.waveParam.internalBitDepth =
                    enc_op->enc_std_param->wave_param->internal_bit_depth;
            if (enc_op->enc_std_param->wave_param->has_lossless_enable)
                enc_open_param.EncStdParam.waveParam.losslessEnable =
                    enc_op->enc_std_param->wave_param->lossless_enable;
            if (enc_op->enc_std_param->wave_param->has_const_intra_pred_flag)
                enc_open_param.EncStdParam.waveParam.constIntraPredFlag =
                    enc_op->enc_std_param->wave_param->const_intra_pred_flag;
            if (enc_op->enc_std_param->wave_param->has_gop_preset_idx)
                enc_open_param.EncStdParam.waveParam.gopPresetIdx =
                    enc_op->enc_std_param->wave_param->gop_preset_idx;
            if (enc_op->enc_std_param->wave_param->has_decoding_refresh_type)
                enc_open_param.EncStdParam.waveParam.decodingRefreshType =
                    enc_op->enc_std_param->wave_param->decoding_refresh_type;
            if (enc_op->enc_std_param->wave_param->has_intra_qp)
                enc_open_param.EncStdParam.waveParam.intraQP =
                    enc_op->enc_std_param->wave_param->intra_qp;
            if (enc_op->enc_std_param->wave_param->has_intra_period)
                enc_open_param.EncStdParam.waveParam.intraPeriod =
                    enc_op->enc_std_param->wave_param->intra_period;
            if (enc_op->enc_std_param->wave_param->has_forced_idr_header_enable)
                enc_open_param.EncStdParam.waveParam.forcedIdrHeaderEnable =
                    enc_op->enc_std_param->wave_param->forced_idr_header_enable;
            if (enc_op->enc_std_param->wave_param->has_conf_win_top)
                enc_open_param.EncStdParam.waveParam.confWinTop =
                    enc_op->enc_std_param->wave_param->conf_win_top;
            if (enc_op->enc_std_param->wave_param->has_conf_win_bot)
                enc_open_param.EncStdParam.waveParam.confWinBot =
                    enc_op->enc_std_param->wave_param->conf_win_bot;
            if (enc_op->enc_std_param->wave_param->has_conf_win_left)
                enc_open_param.EncStdParam.waveParam.confWinLeft =
                    enc_op->enc_std_param->wave_param->conf_win_left;
            if (enc_op->enc_std_param->wave_param->has_conf_win_right)
                enc_open_param.EncStdParam.waveParam.confWinRight =
                    enc_op->enc_std_param->wave_param->conf_win_right;
            if (enc_op->enc_std_param->wave_param->has_independ_slice_mode)
                enc_open_param.EncStdParam.waveParam.independSliceMode =
                    enc_op->enc_std_param->wave_param->independ_slice_mode;
            if (enc_op->enc_std_param->wave_param->has_independ_slice_mode_arg)
                enc_open_param.EncStdParam.waveParam.independSliceModeArg =
                    enc_op->enc_std_param->wave_param->independ_slice_mode_arg;
            if (enc_op->enc_std_param->wave_param->has_depend_slice_mode)
                enc_open_param.EncStdParam.waveParam.dependSliceMode =
                    enc_op->enc_std_param->wave_param->depend_slice_mode;
            if (enc_op->enc_std_param->wave_param->has_depend_slice_mode_arg)
                enc_open_param.EncStdParam.waveParam.dependSliceModeArg =
                    enc_op->enc_std_param->wave_param->depend_slice_mode_arg;
            if (enc_op->enc_std_param->wave_param->has_intra_refresh_mode)
                enc_open_param.EncStdParam.waveParam.intraRefreshMode =
                    enc_op->enc_std_param->wave_param->intra_refresh_mode;
            if (enc_op->enc_std_param->wave_param->has_intra_refresh_arg)
                enc_open_param.EncStdParam.waveParam.intraRefreshArg =
                    enc_op->enc_std_param->wave_param->intra_refresh_arg;
            if (enc_op->enc_std_param->wave_param->has_use_recommend_enc_param)
                enc_open_param.EncStdParam.waveParam.useRecommendEncParam =
                    enc_op->enc_std_param->wave_param->use_recommend_enc_param;
            if (enc_op->enc_std_param->wave_param->has_scaling_list_enable)
                enc_open_param.EncStdParam.waveParam.scalingListEnable =
                    enc_op->enc_std_param->wave_param->scaling_list_enable;
            if (enc_op->enc_std_param->wave_param->has_cu_size_mode)
                enc_open_param.EncStdParam.waveParam.cuSizeMode =
                    enc_op->enc_std_param->wave_param->cu_size_mode;
            if (enc_op->enc_std_param->wave_param->has_tmvp_enable)
                enc_open_param.EncStdParam.waveParam.tmvpEnable =
                    enc_op->enc_std_param->wave_param->tmvp_enable;
            if (enc_op->enc_std_param->wave_param->has_wpp_enable)
                enc_open_param.EncStdParam.waveParam.wppEnable =
                    enc_op->enc_std_param->wave_param->wpp_enable;
            if (enc_op->enc_std_param->wave_param->has_max_num_merge)
                enc_open_param.EncStdParam.waveParam.maxNumMerge =
                    enc_op->enc_std_param->wave_param->max_num_merge;
            if (enc_op->enc_std_param->wave_param->has_disable_deblk)
                enc_open_param.EncStdParam.waveParam.disableDeblk =
                    enc_op->enc_std_param->wave_param->disable_deblk;
            if (enc_op->enc_std_param->wave_param->has_lf_cross_slice_boundary_enable)
                enc_open_param.EncStdParam.waveParam.lfCrossSliceBoundaryEnable =
                    enc_op->enc_std_param->wave_param->lf_cross_slice_boundary_enable;
            if (enc_op->enc_std_param->wave_param->has_beta_offset_div2)
                enc_open_param.EncStdParam.waveParam.betaOffsetDiv2 =
                    enc_op->enc_std_param->wave_param->beta_offset_div2;
            if (enc_op->enc_std_param->wave_param->has_tc_offset_div2)
                enc_open_param.EncStdParam.waveParam.tcOffsetDiv2 =
                    enc_op->enc_std_param->wave_param->tc_offset_div2;
            if (enc_op->enc_std_param->wave_param->has_skip_intra_trans)
                enc_open_param.EncStdParam.waveParam.skipIntraTrans =
                    enc_op->enc_std_param->wave_param->skip_intra_trans;
            if (enc_op->enc_std_param->wave_param->has_sao_enable)
                enc_open_param.EncStdParam.waveParam.saoEnable =
                    enc_op->enc_std_param->wave_param->sao_enable;
            if (enc_op->enc_std_param->wave_param->has_intra_nx_n_enable)
                enc_open_param.EncStdParam.waveParam.intraNxNEnable =
                    enc_op->enc_std_param->wave_param->intra_nx_n_enable;
            if (enc_op->enc_std_param->wave_param->has_bit_alloc_mode)
                enc_open_param.EncStdParam.waveParam.bitAllocMode =
                    enc_op->enc_std_param->wave_param->bit_alloc_mode;

            if (enc_op->enc_std_param->wave_param->has_cu_level_rc_enable)
                enc_open_param.EncStdParam.waveParam.cuLevelRCEnable =
                    enc_op->enc_std_param->wave_param->cu_level_rc_enable;
            if (enc_op->enc_std_param->wave_param->has_hvs_qp_enable)
                enc_open_param.EncStdParam.waveParam.hvsQPEnable =
                    enc_op->enc_std_param->wave_param->hvs_qp_enable;
            if (enc_op->enc_std_param->wave_param->has_hvs_qp_scale)
                enc_open_param.EncStdParam.waveParam.hvsQpScale =
                    enc_op->enc_std_param->wave_param->hvs_qp_scale;
            if (enc_op->enc_std_param->wave_param->has_hvs_max_delta_qp)
                enc_open_param.EncStdParam.waveParam.hvsMaxDeltaQp =
                    enc_op->enc_std_param->wave_param->hvs_max_delta_qp;

            if (enc_op->enc_std_param->wave_param->has_roi_enable)
                enc_open_param.EncStdParam.waveParam.roiEnable =
                    enc_op->enc_std_param->wave_param->roi_enable;
            if (enc_op->enc_std_param->wave_param->has_num_units_in_tick)
                enc_open_param.EncStdParam.waveParam.numUnitsInTick =
                    enc_op->enc_std_param->wave_param->num_units_in_tick;
            if (enc_op->enc_std_param->wave_param->has_time_scale)
                enc_open_param.EncStdParam.waveParam.timeScale =
                    enc_op->enc_std_param->wave_param->time_scale;
            if (enc_op->enc_std_param->wave_param->has_num_ticks_poc_diff_one)
                enc_open_param.EncStdParam.waveParam.numTicksPocDiffOne =
                    enc_op->enc_std_param->wave_param->num_ticks_poc_diff_one;
            if (enc_op->enc_std_param->wave_param->has_chroma_cb_qp_offset)
                enc_open_param.EncStdParam.waveParam.chromaCbQpOffset =
                    enc_op->enc_std_param->wave_param->chroma_cb_qp_offset;
            if (enc_op->enc_std_param->wave_param->has_chroma_cr_qp_offset)
                enc_open_param.EncStdParam.waveParam.chromaCrQpOffset =
                    enc_op->enc_std_param->wave_param->chroma_cr_qp_offset;
            if (enc_op->enc_std_param->wave_param->has_initial_rc_qp)
                enc_open_param.EncStdParam.waveParam.initialRcQp =
                    enc_op->enc_std_param->wave_param->initial_rc_qp;
            if (enc_op->enc_std_param->wave_param->has_nr_y_enable)
                enc_open_param.EncStdParam.waveParam.nrYEnable =
                    enc_op->enc_std_param->wave_param->nr_y_enable;
            if (enc_op->enc_std_param->wave_param->has_nr_cb_enable)
                enc_open_param.EncStdParam.waveParam.nrCbEnable =
                    enc_op->enc_std_param->wave_param->nr_cb_enable;
            if (enc_op->enc_std_param->wave_param->has_nr_cr_enable)
                enc_open_param.EncStdParam.waveParam.nrCrEnable =
                    enc_op->enc_std_param->wave_param->nr_cr_enable;
            if (enc_op->enc_std_param->wave_param->has_nr_intra_weight_y)
                enc_open_param.EncStdParam.waveParam.nrIntraWeightY =
                    enc_op->enc_std_param->wave_param->nr_intra_weight_y;
            if (enc_op->enc_std_param->wave_param->has_nr_intra_weight_cb)
                enc_open_param.EncStdParam.waveParam.nrIntraWeightCb =
                    enc_op->enc_std_param->wave_param->nr_intra_weight_cb;
            if (enc_op->enc_std_param->wave_param->has_nr_intra_weight_cr)
                enc_open_param.EncStdParam.waveParam.nrIntraWeightCr =
                    enc_op->enc_std_param->wave_param->nr_intra_weight_cr;
            if (enc_op->enc_std_param->wave_param->has_nr_inter_weight_y)
                enc_open_param.EncStdParam.waveParam.nrInterWeightY =
                    enc_op->enc_std_param->wave_param->nr_inter_weight_y;
            if (enc_op->enc_std_param->wave_param->has_nr_inter_weight_cb)
                enc_open_param.EncStdParam.waveParam.nrInterWeightCb =
                    enc_op->enc_std_param->wave_param->nr_inter_weight_cb;
            if (enc_op->enc_std_param->wave_param->has_nr_inter_weight_cr)
                enc_open_param.EncStdParam.waveParam.nrInterWeightCr =
                    enc_op->enc_std_param->wave_param->nr_inter_weight_cr;
            if (enc_op->enc_std_param->wave_param->has_nr_noise_est_enable)
                enc_open_param.EncStdParam.waveParam.nrNoiseEstEnable =
                    enc_op->enc_std_param->wave_param->nr_noise_est_enable;
            if (enc_op->enc_std_param->wave_param->has_nr_noise_sigma_y)
                enc_open_param.EncStdParam.waveParam.nrNoiseSigmaY =
                    enc_op->enc_std_param->wave_param->nr_noise_sigma_y;
            if (enc_op->enc_std_param->wave_param->has_nr_noise_sigma_cb)
                enc_open_param.EncStdParam.waveParam.nrNoiseSigmaCb =
                    enc_op->enc_std_param->wave_param->nr_noise_sigma_cb;
            if (enc_op->enc_std_param->wave_param->has_nr_noise_sigma_cr)
                enc_open_param.EncStdParam.waveParam.nrNoiseSigmaCr =
                    enc_op->enc_std_param->wave_param->nr_noise_sigma_cr;
            if (enc_op->enc_std_param->wave_param->has_use_long_term)
                enc_open_param.EncStdParam.waveParam.useLongTerm =
                    enc_op->enc_std_param->wave_param->use_long_term;
            if (enc_op->enc_std_param->wave_param->has_monochrome_enable)
                enc_open_param.EncStdParam.waveParam.monochromeEnable =
                    enc_op->enc_std_param->wave_param->monochrome_enable;
            if (enc_op->enc_std_param->wave_param->has_strong_intra_smooth_enable)
                enc_open_param.EncStdParam.waveParam.strongIntraSmoothEnable =
                    enc_op->enc_std_param->wave_param->strong_intra_smooth_enable;
            if (enc_op->enc_std_param->wave_param->has_weight_pred_enable)
                enc_open_param.EncStdParam.waveParam.weightPredEnable =
                    enc_op->enc_std_param->wave_param->weight_pred_enable;
            if (enc_op->enc_std_param->wave_param->has_bg_detect_enable)
                enc_open_param.EncStdParam.waveParam.bgDetectEnable =
                    enc_op->enc_std_param->wave_param->bg_detect_enable;
            if (enc_op->enc_std_param->wave_param->has_bg_thr_diff)
                enc_open_param.EncStdParam.waveParam.bgThrDiff =
                    enc_op->enc_std_param->wave_param->bg_thr_diff;
            if (enc_op->enc_std_param->wave_param->has_bg_thr_mean_diff)
                enc_open_param.EncStdParam.waveParam.bgThrMeanDiff =
                    enc_op->enc_std_param->wave_param->bg_thr_mean_diff;
            if (enc_op->enc_std_param->wave_param->has_bg_lambda_qp)
                enc_open_param.EncStdParam.waveParam.bgLambdaQp =
                    enc_op->enc_std_param->wave_param->bg_lambda_qp;
            if (enc_op->enc_std_param->wave_param->has_bg_delta_qp)
                enc_open_param.EncStdParam.waveParam.bgDeltaQp =
                    enc_op->enc_std_param->wave_param->bg_delta_qp;
            if (enc_op->enc_std_param->wave_param->has_custom_lambda_enable)
                enc_open_param.EncStdParam.waveParam.customLambdaEnable =
                    enc_op->enc_std_param->wave_param->custom_lambda_enable;
            if (enc_op->enc_std_param->wave_param->has_custom_md_enable)
                enc_open_param.EncStdParam.waveParam.customMDEnable =
                    enc_op->enc_std_param->wave_param->custom_md_enable;
            if (enc_op->enc_std_param->wave_param->has_pu04_delta_rate)
                enc_open_param.EncStdParam.waveParam.pu04DeltaRate =
                    enc_op->enc_std_param->wave_param->pu04_delta_rate;
            if (enc_op->enc_std_param->wave_param->has_pu08_delta_rate)
                enc_open_param.EncStdParam.waveParam.pu08DeltaRate =
                    enc_op->enc_std_param->wave_param->pu08_delta_rate;
            if (enc_op->enc_std_param->wave_param->has_pu16_delta_rate)
                enc_open_param.EncStdParam.waveParam.pu16DeltaRate =
                    enc_op->enc_std_param->wave_param->pu16_delta_rate;
            if (enc_op->enc_std_param->wave_param->has_pu32_delta_rate)
                enc_open_param.EncStdParam.waveParam.pu32DeltaRate =
                    enc_op->enc_std_param->wave_param->pu32_delta_rate;
            if (enc_op->enc_std_param->wave_param->has_pu04_intra_planar_delta_rate)
                enc_open_param.EncStdParam.waveParam.pu04IntraPlanarDeltaRate =
                    enc_op->enc_std_param->wave_param->pu04_intra_planar_delta_rate;
            if (enc_op->enc_std_param->wave_param->has_pu04_intra_dc_delta_rate)
                enc_open_param.EncStdParam.waveParam.pu04IntraDcDeltaRate =
                    enc_op->enc_std_param->wave_param->pu04_intra_dc_delta_rate;
            if (enc_op->enc_std_param->wave_param->has_pu04_intra_angle_delta_rate)
                enc_open_param.EncStdParam.waveParam.pu04IntraAngleDeltaRate =
                    enc_op->enc_std_param->wave_param->pu04_intra_angle_delta_rate;
            if (enc_op->enc_std_param->wave_param->has_pu08_intra_planar_delta_rate)
                enc_open_param.EncStdParam.waveParam.pu08IntraPlanarDeltaRate =
                    enc_op->enc_std_param->wave_param->pu08_intra_planar_delta_rate;
            if (enc_op->enc_std_param->wave_param->has_pu08_intra_dc_delta_rate)
                enc_open_param.EncStdParam.waveParam.pu08IntraDcDeltaRate =
                    enc_op->enc_std_param->wave_param->pu08_intra_dc_delta_rate;
            if (enc_op->enc_std_param->wave_param->has_pu08_intra_angle_delta_rate)
                enc_open_param.EncStdParam.waveParam.pu08IntraAngleDeltaRate =
                    enc_op->enc_std_param->wave_param->pu08_intra_angle_delta_rate;
            if (enc_op->enc_std_param->wave_param->has_pu16_intra_planar_delta_rate)
                enc_open_param.EncStdParam.waveParam.pu16IntraPlanarDeltaRate =
                    enc_op->enc_std_param->wave_param->pu16_intra_planar_delta_rate;
            if (enc_op->enc_std_param->wave_param->has_pu16_intra_dc_delta_rate)
                enc_open_param.EncStdParam.waveParam.pu16IntraDcDeltaRate =
                    enc_op->enc_std_param->wave_param->pu16_intra_dc_delta_rate;
            if (enc_op->enc_std_param->wave_param->has_pu16_intra_angle_delta_rate)
                enc_open_param.EncStdParam.waveParam.pu16IntraAngleDeltaRate =
                    enc_op->enc_std_param->wave_param->pu16_intra_angle_delta_rate;
            if (enc_op->enc_std_param->wave_param->has_pu32_intra_planar_delta_rate)
                enc_open_param.EncStdParam.waveParam.pu32IntraPlanarDeltaRate =
                    enc_op->enc_std_param->wave_param->pu32_intra_planar_delta_rate;
            if (enc_op->enc_std_param->wave_param->has_pu32_intra_dc_delta_rate)
                enc_open_param.EncStdParam.waveParam.pu32IntraDcDeltaRate =
                    enc_op->enc_std_param->wave_param->pu32_intra_dc_delta_rate;
            if (enc_op->enc_std_param->wave_param->has_pu32_intra_angle_delta_rate)
                enc_open_param.EncStdParam.waveParam.pu32IntraAngleDeltaRate =
                    enc_op->enc_std_param->wave_param->pu32_intra_angle_delta_rate;
            if (enc_op->enc_std_param->wave_param->has_cu08_intra_delta_rate)
                enc_open_param.EncStdParam.waveParam.cu08IntraDeltaRate =
                    enc_op->enc_std_param->wave_param->cu08_intra_delta_rate;
            if (enc_op->enc_std_param->wave_param->has_cu08_inter_delta_rate)
                enc_open_param.EncStdParam.waveParam.cu08InterDeltaRate =
                    enc_op->enc_std_param->wave_param->cu08_inter_delta_rate;
            if (enc_op->enc_std_param->wave_param->has_cu08_merge_delta_rate)
                enc_open_param.EncStdParam.waveParam.cu08MergeDeltaRate =
                    enc_op->enc_std_param->wave_param->cu08_merge_delta_rate;
            if (enc_op->enc_std_param->wave_param->has_cu16_intra_delta_rate)
                enc_open_param.EncStdParam.waveParam.cu16IntraDeltaRate =
                    enc_op->enc_std_param->wave_param->cu16_intra_delta_rate;
            if (enc_op->enc_std_param->wave_param->has_cu16_inter_delta_rate)
                enc_open_param.EncStdParam.waveParam.cu16InterDeltaRate =
                    enc_op->enc_std_param->wave_param->cu16_inter_delta_rate;
            if (enc_op->enc_std_param->wave_param->has_cu16_merge_delta_rate)
                enc_open_param.EncStdParam.waveParam.cu16MergeDeltaRate =
                    enc_op->enc_std_param->wave_param->cu16_merge_delta_rate;
            if (enc_op->enc_std_param->wave_param->has_cu32_intra_delta_rate)
                enc_open_param.EncStdParam.waveParam.cu32IntraDeltaRate =
                    enc_op->enc_std_param->wave_param->cu32_intra_delta_rate;
            if (enc_op->enc_std_param->wave_param->has_cu32_inter_delta_rate)
                enc_open_param.EncStdParam.waveParam.cu32InterDeltaRate =
                    enc_op->enc_std_param->wave_param->cu32_inter_delta_rate;
            if (enc_op->enc_std_param->wave_param->has_cu32_merge_delta_rate)
                enc_open_param.EncStdParam.waveParam.cu32MergeDeltaRate =
                    enc_op->enc_std_param->wave_param->cu32_merge_delta_rate;
            if (enc_op->enc_std_param->wave_param->has_coef_clear_disable)
                enc_open_param.EncStdParam.waveParam.coefClearDisable =
                    enc_op->enc_std_param->wave_param->coef_clear_disable;
            if (enc_op->enc_std_param->wave_param->has_min_qp_i)
                enc_open_param.EncStdParam.waveParam.minQpI =
                    enc_op->enc_std_param->wave_param->min_qp_i;
            if (enc_op->enc_std_param->wave_param->has_max_qp_i)
                enc_open_param.EncStdParam.waveParam.maxQpI =
                    enc_op->enc_std_param->wave_param->max_qp_i;
            if (enc_op->enc_std_param->wave_param->has_min_qp_p)
                enc_open_param.EncStdParam.waveParam.minQpP =
                    enc_op->enc_std_param->wave_param->min_qp_p;
            if (enc_op->enc_std_param->wave_param->has_max_qp_p)
                enc_open_param.EncStdParam.waveParam.maxQpP =
                    enc_op->enc_std_param->wave_param->max_qp_p;
            if (enc_op->enc_std_param->wave_param->has_min_qp_b)
                enc_open_param.EncStdParam.waveParam.minQpB =
                    enc_op->enc_std_param->wave_param->min_qp_b;
            if (enc_op->enc_std_param->wave_param->has_max_qp_b)
                enc_open_param.EncStdParam.waveParam.maxQpB =
                    enc_op->enc_std_param->wave_param->max_qp_b;
            if (enc_op->enc_std_param->wave_param->has_custom_lambda_addr)
                enc_open_param.EncStdParam.waveParam.customLambdaAddr =
                    enc_op->enc_std_param->wave_param->custom_lambda_addr;
            if (enc_op->enc_std_param->wave_param->has_user_scaling_list_addr)
                enc_open_param.EncStdParam.waveParam.userScalingListAddr =
                    enc_op->enc_std_param->wave_param->user_scaling_list_addr;
            if (enc_op->enc_std_param->wave_param->has_svc_enable)
                enc_open_param.EncStdParam.waveParam.svcEnable =
                    enc_op->enc_std_param->wave_param->svc_enable;
            if (enc_op->enc_std_param->wave_param->has_svc_mode)
                enc_open_param.EncStdParam.waveParam.svcMode =
                    enc_op->enc_std_param->wave_param->svc_mode;
            if (enc_op->enc_std_param->wave_param->has_luma_dc_qp_offset)
                enc_open_param.EncStdParam.waveParam.lumaDcQpOffset =
                    enc_op->enc_std_param->wave_param->luma_dc_qp_offset;
            if (enc_op->enc_std_param->wave_param->has_chroma_dc_qp_offset)
                enc_open_param.EncStdParam.waveParam.chromaDcQpOffset =
                    enc_op->enc_std_param->wave_param->chroma_dc_qp_offset;
            if (enc_op->enc_std_param->wave_param->has_chroma_ac_qp_offset)
                enc_open_param.EncStdParam.waveParam.chromaAcQpOffset =
                    enc_op->enc_std_param->wave_param->chroma_ac_qp_offset;
            if (enc_op->enc_std_param->wave_param->has_avc_idr_period)
                enc_open_param.EncStdParam.waveParam.avcIdrPeriod =
                    enc_op->enc_std_param->wave_param->avc_idr_period;
            if (enc_op->enc_std_param->wave_param->has_rdo_skip)
                enc_open_param.EncStdParam.waveParam.rdoSkip =
                    enc_op->enc_std_param->wave_param->rdo_skip;
            if (enc_op->enc_std_param->wave_param->has_lambda_scaling_enable)
                enc_open_param.EncStdParam.waveParam.lambdaScalingEnable =
                    enc_op->enc_std_param->wave_param->lambda_scaling_enable;
            if (enc_op->enc_std_param->wave_param->has_transform8x8_enable)
                enc_open_param.EncStdParam.waveParam.transform8x8Enable =
                    enc_op->enc_std_param->wave_param->transform8x8_enable;
            if (enc_op->enc_std_param->wave_param->has_avc_slice_mode)
                enc_open_param.EncStdParam.waveParam.avcSliceMode =
                    enc_op->enc_std_param->wave_param->avc_slice_mode;
            if (enc_op->enc_std_param->wave_param->has_avc_slice_arg)
                enc_open_param.EncStdParam.waveParam.avcSliceArg =
                    enc_op->enc_std_param->wave_param->avc_slice_arg;
            if (enc_op->enc_std_param->wave_param->has_intra_mb_refresh_mode)
                enc_open_param.EncStdParam.waveParam.intraMbRefreshMode =
                    enc_op->enc_std_param->wave_param->intra_mb_refresh_mode;
            if (enc_op->enc_std_param->wave_param->has_intra_mb_refresh_arg)
                enc_open_param.EncStdParam.waveParam.intraMbRefreshArg =
                    enc_op->enc_std_param->wave_param->intra_mb_refresh_arg;
            if (enc_op->enc_std_param->wave_param->has_mb_level_rc_enable)
                enc_open_param.EncStdParam.waveParam.mbLevelRcEnable =
                    enc_op->enc_std_param->wave_param->mb_level_rc_enable;
            if (enc_op->enc_std_param->wave_param->has_entropy_coding_mode)
                enc_open_param.EncStdParam.waveParam.entropyCodingMode =
                    enc_op->enc_std_param->wave_param->entropy_coding_mode;
            if (enc_op->enc_std_param->wave_param->has_s2fme_disable)
                enc_open_param.EncStdParam.waveParam.s2fmeDisable =
                    enc_op->enc_std_param->wave_param->s2fme_disable;
            if (enc_op->enc_std_param->wave_param->has_rc_weight_param)
                enc_open_param.EncStdParam.waveParam.rcWeightParam =
                    enc_op->enc_std_param->wave_param->rc_weight_param;
            if (enc_op->enc_std_param->wave_param->has_rc_weight_buf)
                enc_open_param.EncStdParam.waveParam.rcWeightBuf =
                    enc_op->enc_std_param->wave_param->rc_weight_buf;

            if (enc_op->enc_std_param->wave_param->vui_param) {
                if (enc_op->enc_std_param->wave_param->vui_param->has_vui_param_flags)
                    enc_open_param.EncStdParam.waveParam.vuiParam.vuiParamFlags =
                        enc_op->enc_std_param->wave_param->vui_param->vui_param_flags;
                if (enc_op->enc_std_param->wave_param->vui_param->has_vui_aspect_ratio_idc)
                    enc_open_param.EncStdParam.waveParam.vuiParam.vuiAspectRatioIdc =
                        enc_op->enc_std_param->wave_param->vui_param->vui_aspect_ratio_idc;
                if (enc_op->enc_std_param->wave_param->vui_param->has_vui_sar_size)
                    enc_open_param.EncStdParam.waveParam.vuiParam.vuiSarSize =
                        enc_op->enc_std_param->wave_param->vui_param->vui_sar_size;
                if (enc_op->enc_std_param->wave_param->vui_param->has_vui_over_scan_appropriate)
                    enc_open_param.EncStdParam.waveParam.vuiParam.vuiOverScanAppropriate =
                        enc_op->enc_std_param->wave_param->vui_param->vui_over_scan_appropriate;
                if (enc_op->enc_std_param->wave_param->vui_param->has_video_signal)
                    enc_open_param.EncStdParam.waveParam.vuiParam.videoSignal =
                        enc_op->enc_std_param->wave_param->vui_param->video_signal;
                if (enc_op->enc_std_param->wave_param->vui_param->has_vui_chroma_sample_loc)
                    enc_open_param.EncStdParam.waveParam.vuiParam.vuiChromaSampleLoc =
                        enc_op->enc_std_param->wave_param->vui_param->vui_chroma_sample_loc;
                if (enc_op->enc_std_param->wave_param->vui_param->has_vui_disp_win_left_right)
                    enc_open_param.EncStdParam.waveParam.vuiParam.vuiDispWinLeftRight =
                        enc_op->enc_std_param->wave_param->vui_param->vui_disp_win_left_right;
                if (enc_op->enc_std_param->wave_param->vui_param->has_vui_disp_win_top_bottom)
                    enc_open_param.EncStdParam.waveParam.vuiParam.vuiDispWinTopBottom =
                        enc_op->enc_std_param->wave_param->vui_param->vui_disp_win_top_bottom;
            }

            if (enc_op->enc_std_param->wave_param->fixed_bit_ratio) {
                for (int i = 0; i < enc_op->enc_std_param->wave_param->n_fixed_bit_ratio; i++)
                    enc_open_param.EncStdParam.waveParam.fixedBitRatio[i] =
                        enc_op->enc_std_param->wave_param->fixed_bit_ratio[i];
            }

            // CustomGopParam
            if (enc_op->enc_std_param->wave_param->gop_param) {
                if (enc_op->enc_std_param->wave_param->gop_param->has_custom_gop_size)
                    enc_open_param.EncStdParam.waveParam.gopParam.customGopSize =
                        enc_op->enc_std_param->wave_param->gop_param->custom_gop_size;
                if (enc_op->enc_std_param->wave_param->gop_param->has_use_derive_lambda_weight)
                    enc_open_param.EncStdParam.waveParam.gopParam.useDeriveLambdaWeight =
                        enc_op->enc_std_param->wave_param->gop_param->use_derive_lambda_weight;

                if (enc_op->enc_std_param->wave_param->gop_param->gop_pic_lambda) {
                    for (int i = 0;
                         i < enc_op->enc_std_param->wave_param->gop_param->n_gop_pic_lambda; i++)
                        enc_open_param.EncStdParam.waveParam.gopParam.gopPicLambda[i] =
                            enc_op->enc_std_param->wave_param->gop_param->gop_pic_lambda[i];
                }

                if (enc_op->enc_std_param->wave_param->gop_param->pic_param) {
                    for (int i = 0; i < enc_op->enc_std_param->wave_param->gop_param->n_pic_param;
                         i++) {
                        if (enc_op->enc_std_param->wave_param->gop_param->pic_param[i]) {
                            if (enc_op->enc_std_param->wave_param->gop_param->pic_param[i]
                                    ->has_pic_type)
                                enc_open_param.EncStdParam.waveParam.gopParam.picParam[i].picType =
                                    enc_op->enc_std_param->wave_param->gop_param->pic_param[i]
                                        ->pic_type;

                            if (enc_op->enc_std_param->wave_param->gop_param->pic_param[i]
                                    ->has_poc_offset)
                                enc_open_param.EncStdParam.waveParam.gopParam.picParam[i]
                                    .pocOffset =
                                    enc_op->enc_std_param->wave_param->gop_param->pic_param[i]
                                        ->poc_offset;
                            if (enc_op->enc_std_param->wave_param->gop_param->pic_param[i]
                                    ->has_pic_qp)
                                enc_open_param.EncStdParam.waveParam.gopParam.picParam[i].picQp =
                                    enc_op->enc_std_param->wave_param->gop_param->pic_param[i]
                                        ->pic_qp;
                            if (enc_op->enc_std_param->wave_param->gop_param->pic_param[i]
                                    ->has_use_multi_ref_p)
                                enc_open_param.EncStdParam.waveParam.gopParam.picParam[i]
                                    .useMultiRefP =
                                    enc_op->enc_std_param->wave_param->gop_param->pic_param[i]
                                        ->use_multi_ref_p;
                            if (enc_op->enc_std_param->wave_param->gop_param->pic_param[i]
                                    ->has_ref_poc_l0)
                                enc_open_param.EncStdParam.waveParam.gopParam.picParam[i].refPocL0 =
                                    enc_op->enc_std_param->wave_param->gop_param->pic_param[i]
                                        ->ref_poc_l0;
                            if (enc_op->enc_std_param->wave_param->gop_param->pic_param[i]
                                    ->has_ref_poc_l1)
                                enc_open_param.EncStdParam.waveParam.gopParam.picParam[i].refPocL1 =
                                    enc_op->enc_std_param->wave_param->gop_param->pic_param[i]
                                        ->ref_poc_l1;
                            if (enc_op->enc_std_param->wave_param->gop_param->pic_param[i]
                                    ->has_temporal_id)
                                enc_open_param.EncStdParam.waveParam.gopParam.picParam[i]
                                    .temporalId =
                                    enc_op->enc_std_param->wave_param->gop_param->pic_param[i]
                                        ->temporal_id;
                        }
                    }
                }
            }
        }
    }

    VLOG(INFO, "[VPU] VPU_EncOpen\n");
    VLOG(INFO, "------------------------------ ENCODER OPTIONS ------------------------------\n");
    VLOG(INFO, "[coreIdx            ]: %d\n", enc_open_param.coreIdx);
    VLOG(INFO, "[bitstreamBufferSize]: %d\n", enc_open_param.bitstreamBufferSize);
    VLOG(INFO, "[bitstreamFormat    ]: %d\n", enc_open_param.bitstreamFormat);
    VLOG(INFO, "[ringBufferEnable   ]: %d\n", enc_open_param.ringBufferEnable);
    VLOG(INFO, "[picWidth           ]: %d\n", enc_open_param.picWidth);
    VLOG(INFO, "[picHeight          ]: %d\n", enc_open_param.picHeight);
    VLOG(INFO, "[linear2TiledEnable ]: %d\n", enc_open_param.linear2TiledEnable);
    VLOG(INFO, "[linear2TiledMode   ]: %d\n", enc_open_param.linear2TiledMode);
    VLOG(INFO, "[frameRateInfo      ]: %d\n", enc_open_param.frameRateInfo);
    VLOG(INFO, "[idrInterval        ]: %d\n", enc_open_param.idrInterval);
    VLOG(INFO, "[meBlkMode          ]: %d\n", enc_open_param.meBlkMode);
    VLOG(INFO, "[sliceMode          ]: %d\n", enc_open_param.sliceMode);
    VLOG(INFO, "[bitRate            ]: %d\n", enc_open_param.bitRate);
    VLOG(INFO, "[cbcrInterleave     ]: %d\n", enc_open_param.cbcrInterleave);
    VLOG(INFO, "[frameEndian        ]: %d\n", enc_open_param.frameEndian);
    VLOG(INFO, "[streamEndian       ]: %d\n", enc_open_param.streamEndian);
    VLOG(INFO, "[sourceEndian       ]: %d\n", enc_open_param.sourceEndian);
    VLOG(INFO, "[bwbEnable          ]: %d\n", enc_open_param.bwbEnable);
    VLOG(INFO, "[packedFormat       ]: %d\n", enc_open_param.packedFormat);
    VLOG(INFO, "[srcFormat          ]: %d\n", enc_open_param.srcFormat);
    VLOG(INFO, "[srcBitDepth        ]: %d\n", enc_open_param.srcBitDepth);
    VLOG(INFO, "[nv21               ]: %d\n", enc_open_param.nv21);
    VLOG(INFO, "[enablePTS          ]: %d\n", enc_open_param.enablePTS);
    VLOG(INFO, "[rcGopIQpOffsetEn   ]: %d\n", enc_open_param.rcGopIQpOffsetEn);
    VLOG(INFO, "[rcGopIQpOffset     ]: %d\n", enc_open_param.rcGopIQpOffset);
    VLOG(INFO, "[gopSize            ]: %d\n", enc_open_param.gopSize);
    VLOG(INFO, "[meBlkMode          ]: %d\n", enc_open_param.meBlkMode);
    VLOG(INFO, "[sliceMode          ]: %d\n", enc_open_param.sliceMode.sliceMode);
    VLOG(INFO, "[intraRefresh       ]: %d\n", enc_open_param.intraRefresh);
    VLOG(INFO, "[ConscIntraRefreshEnable]: %d\n", enc_open_param.ConscIntraRefreshEnable);
    VLOG(INFO, "[CountIntraMbEnable ]: %d\n", enc_open_param.CountIntraMbEnable);
    VLOG(INFO, "[userQpMax          ]: %d\n", enc_open_param.userQpMax);
    VLOG(INFO, "[maxIntraSize       ]: %d\n", enc_open_param.maxIntraSize);
    VLOG(INFO, "[userMaxDeltaQp     ]: %d\n", enc_open_param.userMaxDeltaQp);
    VLOG(INFO, "[userQpMin          ]: %d\n", enc_open_param.userQpMin);
    VLOG(INFO, "[userMinDeltaQp     ]: %d\n", enc_open_param.userMinDeltaQp);
    VLOG(INFO, "[MEUseZeroPmv       ]: %d\n", enc_open_param.MEUseZeroPmv);
    VLOG(INFO, "[userGamma          ]: %d\n", enc_open_param.userGamma);
    VLOG(INFO, "[rcIntervalMode     ]: %d\n", enc_open_param.rcIntervalMode);
    VLOG(INFO, "[mbInterval         ]: %d\n", enc_open_param.mbInterval);
    VLOG(INFO, "[rcEnable           ]: %d\n", enc_open_param.rcEnable);
    VLOG(INFO, "[cbcrOrder          ]: %d\n", enc_open_param.cbcrOrder);
    VLOG(INFO, "[encodeVuiRbsp      ]: %d\n", enc_open_param.encodeVuiRbsp);
    VLOG(INFO, "[avcParam.level     ]: %d\n", enc_open_param.EncStdParam.avcParam.level);
    VLOG(INFO, "[avcParam.profile   ]: %d\n", enc_open_param.EncStdParam.avcParam.profile);
    VLOG(INFO, "[avcParam.chroma    ]: %d\n", enc_open_param.EncStdParam.avcParam.chromaFormat400);
    VLOG(INFO, "[avcParam.ppsNum    ]: %d\n", enc_open_param.EncStdParam.avcParam.ppsNum);
    VLOG(INFO, "-----------------------------------------------------------------------------\n");

    ret = VPU_EncOpen(&handle, &enc_open_param);
    enc_open_msg->has_ret_code = TRUE;
    enc_open_msg->ret_code = ret;

    if (ret != RETCODE_SUCCESS) {
        VLOG(ERR, "handle:%p VPU_EncOpen failed Error code is 0x%x \n", handle, ret);
        return 0;
    }

    // sec AXI
    if (enc_op->has_sram_enable && enc_op->sram_enable) {
        vpu_buffer_t vb = {0};

        if (!vdi_get_sram_memory(enc_open_param.coreIdx, &vb) && vb.size > 0) {
            if (enc_open_param.coreIdx == VPU_CODA988_IDX) {
                sec_axi_use.u.coda9.useBitEnable = USE_BIT_INTERNAL_BUF;
                sec_axi_use.u.coda9.useIpEnable = USE_IP_INTERNAL_BUF;
                sec_axi_use.u.coda9.useDbkYEnable = USE_DBKY_INTERNAL_BUF;
                sec_axi_use.u.coda9.useDbkCEnable = USE_DBKC_INTERNAL_BUF;

                ret = VPU_EncGiveCommand(handle, SET_SEC_AXI, &sec_axi_use);
                enc_open_msg->ret_code = ret;
                if (ret != RETCODE_SUCCESS) {
                    VLOG(ERR, "VPU_EncGiveCommand ( SET_SEC_AXI ) failed Error code is 0x%x \n",
                         ret);
                    return 0;
                }

                instance->vpu_instance_contrl.inst_enable_sram[index] = TRUE;

            } else if (enc_open_param.coreIdx == VPU_WAVE521_IDX) {
                sec_axi_use.u.wave5.useEncRdoEnable = TRUE;
                sec_axi_use.u.wave5.useEncLfEnable = TRUE;

                ret = VPU_EncGiveCommand(handle, SET_SEC_AXI, &sec_axi_use);
                enc_open_msg->ret_code = ret;
                if (ret != RETCODE_SUCCESS) {
                    VLOG(ERR, "VPU_EncGiveCommand ( SET_SEC_AXI ) failed Error code is 0x%x \n",
                         ret);
                    return 0;
                }
            }
        }
    }

    if (enc_open_param.coreIdx == VPU_CODA988_IDX) {
        // SET_CACHE_CONFIG
        if (enc_op->has_map_type && enc_op->has_cbcr_interleave) {
            MaverickCacheConfig encCacheConfig = {0};
            MaverickCache2Config(&encCacheConfig,
                                 0,                       // encoder
                                 enc_op->cbcr_interleave, // cb cr interleave
                                 0,                       /* bypass */
                                 0,                       /* burst  */
                                 3,                       /* merge mode */
                                 enc_op->map_type, 15 /* shape */);
            VPU_EncGiveCommand(handle, SET_CACHE_CONFIG, &encCacheConfig);
        }
    }

    enc_open_msg->handle->handle = (uint32_t)handle;
    instance->vpu_instance_contrl.inst_handle[index] = (uint32_t)handle;

    return 0;
}

int vpu_instance_ipc_enc_get_init_info(Vpu__EncGetInitialInfo *enc_get_init_info_msg,
                                       Vpu__EncGetInitialInfo *enc_get_init_info_msg_ret,
                                       vpu_instance_t *instance)
{
    if (!enc_get_init_info_msg->handle || !enc_get_init_info_msg->handle->handle) {
        VLOG(ERR, "[%s][%d] invailed handle\n", __func__, __LINE__);
        return -1;
    }

    int ret = 0;
    EncInitialInfo initialInfo = {0};
    EncHandle handle = (EncHandle)enc_get_init_info_msg->handle->handle;
    VLOG(INFO, "[%s][%d] handle:%p\n", __func__, __LINE__, handle);

    *enc_get_init_info_msg_ret = (Vpu__EncGetInitialInfo)VPU__ENC_GET_INITIAL_INFO__INIT;
    enc_get_init_info_msg_ret->handle = enc_get_init_info_msg->handle;

    if (handle->coreIdx == VPU_WAVE521_IDX) {
        do {
            ret = VPU_EncIssueSeqInit(handle);
        } while (ret == RETCODE_QUEUEING_FAILURE);

        if (ret != RETCODE_SUCCESS) {
            VLOG(ERR, "[%s][%d] VPU_EncIssueSeqInit failed, ret:%d\n", __func__, __LINE__, ret);
            goto OUT;
        }

        ret = wave5_enc_hand_interrupt(handle);
        if (ret != ENC_INT_STATUS_DONE) {
            VLOG(ERR, "[%d][%d] wave5_enc_hand_interrupt error, ret:%d\n", __func__, __LINE__, ret);
            VPU_EncCompleteSeqInit(handle, &initialInfo);
            ret = RETCODE_FAILURE;
            goto OUT;
        }

        if ((ret = VPU_EncCompleteSeqInit(handle, &initialInfo)) != RETCODE_SUCCESS) {
            VLOG(ERR, "[%s][%d] VPU_EncCompleteSeqInit failed, ret(%d), SEQERR(%08x)\n", __func__,
                 __LINE__, ret, initialInfo.seqInitErrReason);
        }

    } else
        ret = VPU_EncGetInitialInfo(handle, &initialInfo);

OUT:
    enc_get_init_info_msg_ret->has_ret_code = TRUE;
    enc_get_init_info_msg_ret->ret_code = ret;

    if (ret != RETCODE_SUCCESS) {
        VLOG(ERR, "handle:%p VPU_EncGetInitialInfo failed Error code is 0x%x \n", handle, ret);
        return 0;
    }

    enc_get_init_info_msg_ret->initial_info = &instance->vpu_proto_info.enc_init_info;
    *enc_get_init_info_msg_ret->initial_info = (Vpu__EncInitialInfo)VPU__ENC_INITIAL_INFO__INIT;

    enc_get_init_info_msg_ret->initial_info->min_frame_buffer_count =
        initialInfo.minFrameBufferCount;
    enc_get_init_info_msg_ret->initial_info->min_src_frame_count = initialInfo.minSrcFrameCount;
    enc_get_init_info_msg_ret->initial_info->max_latency_pictures = initialInfo.maxLatencyPictures;
    enc_get_init_info_msg_ret->initial_info->seq_init_err_reason = initialInfo.seqInitErrReason;
    enc_get_init_info_msg_ret->initial_info->warn_info = initialInfo.warnInfo;
    enc_get_init_info_msg_ret->initial_info->vlc_buf_size = initialInfo.vlcBufSize;
    enc_get_init_info_msg_ret->initial_info->param_buf_size = initialInfo.paramBufSize;

    enc_get_init_info_msg_ret->initial_info->has_min_frame_buffer_count = TRUE;
    enc_get_init_info_msg_ret->initial_info->has_min_src_frame_count = TRUE;
    enc_get_init_info_msg_ret->initial_info->has_max_latency_pictures = TRUE;
    enc_get_init_info_msg_ret->initial_info->has_seq_init_err_reason = TRUE;
    enc_get_init_info_msg_ret->initial_info->has_warn_info = TRUE;
    enc_get_init_info_msg_ret->initial_info->has_vlc_buf_size = TRUE;
    enc_get_init_info_msg_ret->initial_info->has_param_buf_size = TRUE;

    return 0;
}

int vpu_instance_ipc_enc_register_enc_buf(Vpu__EncRegisterEncodeBuffer *enc_register_enc_buf_msg,
                                          vpu_instance_t *instance)
{
    if (!enc_register_enc_buf_msg->handle || !enc_register_enc_buf_msg->handle->handle ||
        !enc_register_enc_buf_msg->fb_alloc_info ||
        !enc_register_enc_buf_msg->has_vb_frame_buf_size ||
        !enc_register_enc_buf_msg->n_vb_frame_buf_phys_addr ||
        !enc_register_enc_buf_msg->vb_frame_buf_phys_addr || !instance) {
        VLOG(ERR, "[%s][%d] invailed param\n", __func__, __LINE__);
        return -1;
    }

    int ret = 0;
    FrameBufferAllocInfo fb_alloc_info = {0};
    Vpu__FrameBufferAllocInfo *fb_alloc_info_msg = enc_register_enc_buf_msg->fb_alloc_info;
    EncHandle handle = (EncHandle)enc_register_enc_buf_msg->handle->handle;
    VLOG(INFO, "[%s][%d] handle:%p\n", __func__, __LINE__, handle);

    // FrameBufferAllocInfo
    {
        if (fb_alloc_info_msg->has_map_type)
            fb_alloc_info.mapType = fb_alloc_info_msg->map_type;
        if (fb_alloc_info_msg->has_cbcr_interleave)
            fb_alloc_info.cbcrInterleave = fb_alloc_info_msg->cbcr_interleave;
        if (fb_alloc_info_msg->has_nv21)
            fb_alloc_info.nv21 = fb_alloc_info_msg->nv21;
        if (fb_alloc_info_msg->has_format)
            fb_alloc_info.format = fb_alloc_info_msg->format;
        if (fb_alloc_info_msg->has_stride)
            fb_alloc_info.stride = fb_alloc_info_msg->stride;
        if (fb_alloc_info_msg->has_height)
            fb_alloc_info.height = fb_alloc_info_msg->height;
        if (fb_alloc_info_msg->has_size)
            fb_alloc_info.size = fb_alloc_info_msg->size;
        if (fb_alloc_info_msg->has_luma_bit_depth)
            fb_alloc_info.lumaBitDepth = fb_alloc_info_msg->luma_bit_depth;
        if (fb_alloc_info_msg->has_chroma_bit_depth)
            fb_alloc_info.chromaBitDepth = fb_alloc_info_msg->chroma_bit_depth;
        if (fb_alloc_info_msg->has_endian)
            fb_alloc_info.endian = fb_alloc_info_msg->endian;
        if (fb_alloc_info_msg->has_num)
            fb_alloc_info.num = fb_alloc_info_msg->num;
        if (fb_alloc_info_msg->has_type)
            fb_alloc_info.type = fb_alloc_info_msg->type;
    }

    // frame buf for enc
    memset(instance->vpu_info.fb_recon, 0x0, sizeof(instance->vpu_info.fb_recon));
    for (size_t i = 0; i < enc_register_enc_buf_msg->n_vb_frame_buf_phys_addr; i++) {
        instance->vpu_info.fb_recon[i].bufY = enc_register_enc_buf_msg->vb_frame_buf_phys_addr[i];
        instance->vpu_info.fb_recon[i].bufCb = -1;
        instance->vpu_info.fb_recon[i].bufCr = -1;
        instance->vpu_info.fb_recon[i].updateFbInfo = TRUE;
    }

    ret = VPU_EncAllocateFrameBuffer(handle, fb_alloc_info, instance->vpu_info.fb_recon);
    enc_register_enc_buf_msg->has_ret_code = TRUE;
    enc_register_enc_buf_msg->ret_code = ret;
    if (ret != RETCODE_SUCCESS) {
        VLOG(ERR, "handle:%p VPU_EncAllocateFrameBuffer failed Error code is 0x%x \n", handle, ret);
        return 0;
    }

    ret = VPU_EncRegisterFrameBuffer(
        handle, instance->vpu_info.fb_recon, enc_register_enc_buf_msg->n_vb_frame_buf_phys_addr,
        fb_alloc_info.stride, fb_alloc_info.height, fb_alloc_info.mapType);
    enc_register_enc_buf_msg->ret_code = ret;
    if (ret != RETCODE_SUCCESS) {
        VLOG(ERR, "handle:%p VPU_EncRegisterEncBuffer failed Error code is 0x%x \n", handle, ret);
        return 0;
    }

    return 0;
}

int vpu_instance_ipc_enc_get_head(Vpu__EncGetHead *enc_get_head_msg,
                                  Vpu__EncGetHead *enc_get_head_msg_ret, vpu_instance_t *instance)
{
    if (!enc_get_head_msg->handle || !enc_get_head_msg->handle->handle || !instance ||
        !enc_get_head_msg->has_bitstream_format || !enc_get_head_msg->has_input_buf_addr ||
        !enc_get_head_msg->has_input_buf_size ||
        enc_get_head_msg->input_buf_size < VPU_ENC_MAX_HEAD_COUNT * VPU_ENC_EVERY_HEAD_SIZE) {
        VLOG(ERR, "[%s][%d] invailed param\n", __func__, __LINE__);
        return -1;
    }

    int ret;
    int head_count = 0;
    EncHeaderParam head_param = {0};
    EncHandle handle = (EncHandle)enc_get_head_msg->handle->handle;

    *enc_get_head_msg_ret = (Vpu__EncGetHead)VPU__ENC_GET_HEAD__INIT;
    enc_get_head_msg_ret->handle = enc_get_head_msg->handle;
    VLOG(INFO, "[%s][%d] handle:%p\n", __func__, __LINE__, handle);

    if (enc_get_head_msg->bitstream_format == VPU__COD_STD__STD_MPEG4) {
        // vos
        head_param.headerType = VOS_HEADER;
        head_param.buf = enc_get_head_msg->input_buf_addr + head_count * VPU_ENC_EVERY_HEAD_SIZE;
        head_param.size = VPU_ENC_EVERY_HEAD_SIZE;

        ret = VPU_EncGiveCommand(handle, ENC_PUT_VIDEO_HEADER, &head_param);
        enc_get_head_msg_ret->has_ret_code = TRUE;
        enc_get_head_msg_ret->ret_code = ret;

        if (ret != RETCODE_SUCCESS) {
            VLOG(ERR, "[%s][%d] handle:%p ENC_PUT_VIDEO_HEADER failed Error code is 0x%x \n",
                 __func__, __LINE__, handle, ret);
            return 0;
        }

        if (head_param.size > 0) {
            instance->vpu_proto_info.enc_head_output_buf_addr[head_count] = head_param.buf;
            instance->vpu_proto_info.enc_head_output_buf_size[head_count] = head_param.size;
            head_count++;
        }

        // vis
        head_param.headerType = VIS_HEADER;
        head_param.buf = enc_get_head_msg->input_buf_addr + head_count * VPU_ENC_EVERY_HEAD_SIZE;
        head_param.size = VPU_ENC_EVERY_HEAD_SIZE;

        ret = VPU_EncGiveCommand(handle, ENC_PUT_VIDEO_HEADER, &head_param);
        enc_get_head_msg_ret->ret_code = ret;

        if (ret != RETCODE_SUCCESS) {
            VLOG(ERR, "[%s][%d] handle:%p ENC_PUT_VIDEO_HEADER failed Error code is 0x%x \n",
                 __func__, __LINE__, handle, ret);
            return 0;
        }

        if (head_param.size > 0) {
            instance->vpu_proto_info.enc_head_output_buf_addr[head_count] = head_param.buf;
            instance->vpu_proto_info.enc_head_output_buf_size[head_count] = head_param.size;
            head_count++;
        }

        // vol
        head_param.headerType = VOL_HEADER;
        head_param.buf = enc_get_head_msg->input_buf_addr + head_count * VPU_ENC_EVERY_HEAD_SIZE;
        head_param.size = VPU_ENC_EVERY_HEAD_SIZE;

        ret = VPU_EncGiveCommand(handle, ENC_PUT_VIDEO_HEADER, &head_param);
        enc_get_head_msg_ret->ret_code = ret;

        if (ret != RETCODE_SUCCESS) {
            VLOG(ERR, "[%s][%d] handle:%p ENC_PUT_VIDEO_HEADER failed Error code is 0x%x \n",
                 __func__, __LINE__, handle, ret);
            return 0;
        }

        if (head_param.size > 0) {
            instance->vpu_proto_info.enc_head_output_buf_addr[head_count] = head_param.buf;
            instance->vpu_proto_info.enc_head_output_buf_size[head_count] = head_param.size;
            head_count++;
        }

    } else if (handle->coreIdx == VPU_CODA988_IDX &&
               enc_get_head_msg->bitstream_format == VPU__COD_STD__STD_AVC) {
        int active_pps_idx = 0;

        // sps
        head_param.headerType = SPS_RBSP;
        head_param.buf = enc_get_head_msg->input_buf_addr + head_count * VPU_ENC_EVERY_HEAD_SIZE;
        head_param.size = VPU_ENC_EVERY_HEAD_SIZE;

        ret = VPU_EncGiveCommand(handle, ENC_PUT_VIDEO_HEADER, &head_param);
        enc_get_head_msg_ret->has_ret_code = TRUE;
        enc_get_head_msg_ret->ret_code = ret;

        if (ret != RETCODE_SUCCESS) {
            VLOG(ERR, "[%s][%d] handle:%p ENC_PUT_VIDEO_HEADER failed Error code is 0x%x \n",
                 __func__, __LINE__, handle, ret);
            return 0;
        }

        if (head_param.size > 0) {
            instance->vpu_proto_info.enc_head_output_buf_addr[head_count] = head_param.buf;
            instance->vpu_proto_info.enc_head_output_buf_size[head_count] = head_param.size;
            head_count++;
        }

        // pps
        VPU_EncGiveCommand(handle, ENC_GET_ACTIVE_PPS, &active_pps_idx);
        for (int i = 0; i < handle->CodecInfo->encInfo.openParam.EncStdParam.avcParam.ppsNum; i++) {
            head_param.headerType = PPS_RBSP;
            head_param.buf =
                enc_get_head_msg->input_buf_addr + head_count * VPU_ENC_EVERY_HEAD_SIZE;
            head_param.size = VPU_ENC_EVERY_HEAD_SIZE;

            ret = VPU_EncGiveCommand(handle, ENC_SET_ACTIVE_PPS, &i);
            enc_get_head_msg_ret->ret_code = ret;
            if (ret != RETCODE_SUCCESS) {
                VLOG(ERR, "[%s][%d] handle:%p ENC_SET_ACTIVE_PPS failed Error code is 0x%x \n",
                     __func__, __LINE__, handle, ret);
                return 0;
            }

            ret = VPU_EncGiveCommand(handle, ENC_PUT_VIDEO_HEADER, &head_param);
            enc_get_head_msg_ret->ret_code = ret;

            if (ret != RETCODE_SUCCESS) {
                VLOG(ERR, "[%s][%d] handle:%p ENC_PUT_VIDEO_HEADER failed Error code is 0x%x \n",
                     __func__, __LINE__, handle, ret);
                return 0;
            }

            if (head_param.size > 0) {
                instance->vpu_proto_info.enc_head_output_buf_addr[head_count] = head_param.buf;
                instance->vpu_proto_info.enc_head_output_buf_size[head_count] = head_param.size;
                head_count++;
            }
        }
        VPU_EncGiveCommand(handle, ENC_SET_ACTIVE_PPS, &active_pps_idx);
    } else if (handle->coreIdx == VPU_WAVE521_IDX &&
               enc_get_head_msg->bitstream_format == VPU__COD_STD__STD_AVC) {

        EncOutputInfo *output_info = &instance->vpu_info.enc_output_info;

        head_param.buf = enc_get_head_msg->input_buf_addr;
        head_param.size = enc_get_head_msg->input_buf_size;
        head_param.headerType = CODEOPT_ENC_SPS | CODEOPT_ENC_PPS;

        do {
            ret = VPU_EncGiveCommand(handle, ENC_PUT_VIDEO_HEADER, &head_param);
        } while (ret == RETCODE_QUEUEING_FAILURE);
        enc_get_head_msg_ret->has_ret_code = TRUE;
        enc_get_head_msg_ret->ret_code = ret;

        if (ret != RETCODE_SUCCESS) {
            VLOG(ERR, "[%s][%d] handle:%p ENC_PUT_VIDEO_HEADER failed Error code is 0x%x \n",
                 __func__, __LINE__, handle, ret);
            return 0;
        }

        ret = wave5_enc_hand_interrupt(handle);
        if (ret != ENC_INT_STATUS_DONE) {
            enc_get_head_msg_ret->ret_code = RETCODE_FAILURE;
            VPU_EncGetOutputInfo(handle, output_info);
            VLOG(ERR, "[%s][%d] handle:%p wave5_enc_hand_interrupt failed Error code is 0x%x \n",
                 __func__, __LINE__, handle, ret);
            return 0;
        }

        VPU_EncGetOutputInfo(handle, output_info);

        if (output_info->bitstreamSize > 0) {
            instance->vpu_proto_info.enc_head_output_buf_addr[head_count] = head_param.buf;
            instance->vpu_proto_info.enc_head_output_buf_size[head_count] = output_info->bitstreamSize;
            head_count++;
        }
    } else if (handle->coreIdx == VPU_WAVE521_IDX &&
               enc_get_head_msg->bitstream_format == VPU__COD_STD__STD_HEVC) {

        EncOutputInfo *output_info = &instance->vpu_info.enc_output_info;

        head_param.buf = enc_get_head_msg->input_buf_addr;
        head_param.size = enc_get_head_msg->input_buf_size;
        head_param.headerType = CODEOPT_ENC_VPS | CODEOPT_ENC_SPS | CODEOPT_ENC_PPS;

        do {
            ret = VPU_EncGiveCommand(handle, ENC_PUT_VIDEO_HEADER, &head_param);
        } while (ret == RETCODE_QUEUEING_FAILURE);
        enc_get_head_msg_ret->has_ret_code = TRUE;
        enc_get_head_msg_ret->ret_code = ret;

        if (ret != RETCODE_SUCCESS) {
            VLOG(ERR, "[%s][%d] handle:%p ENC_PUT_VIDEO_HEADER failed Error code is 0x%x \n",
                 __func__, __LINE__, handle, ret);
            return 0;
        }

        ret = wave5_enc_hand_interrupt(handle);
        if (ret != ENC_INT_STATUS_DONE) {
            enc_get_head_msg_ret->ret_code = RETCODE_FAILURE;
            VPU_EncGetOutputInfo(handle, output_info);
            VLOG(ERR, "[%s][%d] handle:%p wave5_enc_hand_interrupt failed Error code is 0x%x \n",
                 __func__, __LINE__, handle, ret);
            return 0;
        }

        VPU_EncGetOutputInfo(handle, output_info);

        if (output_info->bitstreamSize > 0) {
            instance->vpu_proto_info.enc_head_output_buf_addr[head_count] = head_param.buf;
            instance->vpu_proto_info.enc_head_output_buf_size[head_count] = output_info->bitstreamSize;
            head_count++;
        }
    } else {
        VLOG(ERR, "[%s][%d] invailed bitstream_format:%d\n", __func__, __LINE__,
             enc_get_head_msg->bitstream_format);
        return -1;
    }

    if (head_count > VPU_ENC_MAX_HEAD_COUNT) {
        VLOG(ERR, "[%s][%d] error count:%d\n", __func__, __LINE__, head_count);
        return -1;
    }

    enc_get_head_msg_ret->n_output_buf_addr = head_count;
    enc_get_head_msg_ret->output_buf_addr = instance->vpu_proto_info.enc_head_output_buf_addr;
    enc_get_head_msg_ret->n_output_buf_size = head_count;
    enc_get_head_msg_ret->output_buf_size = instance->vpu_proto_info.enc_head_output_buf_size;

    return 0;
}

int vpu_instance_ipc_enc_one_frame(Vpu__EncOneFrame *enc_one_frame_msg,
                                   Vpu__EncOneFrame *enc_one_frame_msg_ret,
                                   vpu_instance_t *instance)
{
    if (!enc_one_frame_msg->handle || !enc_one_frame_msg->handle->handle || !instance) {
        VLOG(ERR, "[%s][%d] invailed handle\n", __func__, __LINE__);
        return -1;
    }

    int ret = 0;
    FrameBuffer source_frame = {0};
    FrameBuffer offset_tbl_buffer = {0};
    EncParam *enc_param = &instance->vpu_info.enc_param;
    EncOutputInfo *output_info = &instance->vpu_info.enc_output_info;
    EncHandle handle = (EncHandle)enc_one_frame_msg->handle->handle;

    *enc_one_frame_msg_ret = (Vpu__EncOneFrame)VPU__ENC_ONE_FRAME__INIT;
    enc_one_frame_msg_ret->handle = enc_one_frame_msg->handle;
    VLOG(INFO, "[%s][%d] handle:%p\n", __func__, __LINE__, handle);

    // VPU_EncStartOneFrame
    memset(enc_param, 0x0, sizeof(EncParam));
    if (enc_one_frame_msg->enc_param) {
        Vpu__EncParam *enc_param_msg = enc_one_frame_msg->enc_param;

        if (enc_param_msg->source_frame) {
            enc_param->sourceFrame = &source_frame;

            if (enc_param_msg->source_frame->has_buf_y)
                source_frame.bufY = enc_param_msg->source_frame->buf_y;
            if (enc_param_msg->source_frame->has_buf_cb)
                source_frame.bufCb = enc_param_msg->source_frame->buf_cb;
            if (enc_param_msg->source_frame->has_buf_cr)
                source_frame.bufCr = enc_param_msg->source_frame->buf_cr;
            if (enc_param_msg->source_frame->has_buf_y_bot)
                source_frame.bufYBot = enc_param_msg->source_frame->buf_y_bot;
            if (enc_param_msg->source_frame->has_buf_cb_bot)
                source_frame.bufCbBot = enc_param_msg->source_frame->buf_cb_bot;
            if (enc_param_msg->source_frame->has_buf_cr_bot)
                source_frame.bufCrBot = enc_param_msg->source_frame->buf_cr_bot;
            if (enc_param_msg->source_frame->has_buf_y_size)
                source_frame.bufYSize = enc_param_msg->source_frame->buf_y_size;
            if (enc_param_msg->source_frame->has_buf_cb_size)
                source_frame.bufCbSize = enc_param_msg->source_frame->buf_cb_size;
            if (enc_param_msg->source_frame->has_buf_cr_size)
                source_frame.bufCrSize = enc_param_msg->source_frame->buf_cr_size;
            if (enc_param_msg->source_frame->has_cbcr_interleave)
                source_frame.cbcrInterleave = enc_param_msg->source_frame->cbcr_interleave;
            if (enc_param_msg->source_frame->has_nv21)
                source_frame.nv21 = enc_param_msg->source_frame->nv21;
            if (enc_param_msg->source_frame->has_endian)
                source_frame.endian = enc_param_msg->source_frame->endian;
            if (enc_param_msg->source_frame->has_my_index)
                source_frame.myIndex = enc_param_msg->source_frame->my_index;
            if (enc_param_msg->source_frame->has_map_type)
                source_frame.mapType = enc_param_msg->source_frame->map_type;
            if (enc_param_msg->source_frame->has_stride)
                source_frame.stride = enc_param_msg->source_frame->stride;
            if (enc_param_msg->source_frame->has_width)
                source_frame.width = enc_param_msg->source_frame->width;
            if (enc_param_msg->source_frame->has_height)
                source_frame.height = enc_param_msg->source_frame->height;
            if (enc_param_msg->source_frame->has_size)
                source_frame.size = enc_param_msg->source_frame->size;
            if (enc_param_msg->source_frame->has_size_luma)
                source_frame.sizeLuma = enc_param_msg->source_frame->size_luma;
            if (enc_param_msg->source_frame->has_size_chroma)
                source_frame.sizeChroma = enc_param_msg->source_frame->size_chroma;
            if (enc_param_msg->source_frame->has_luma_bit_depth)
                source_frame.lumaBitDepth = enc_param_msg->source_frame->luma_bit_depth;
            if (enc_param_msg->source_frame->has_chroma_bit_depth)
                source_frame.chromaBitDepth = enc_param_msg->source_frame->chroma_bit_depth;
            if (enc_param_msg->source_frame->has_format)
                source_frame.format = enc_param_msg->source_frame->format;
            if (enc_param_msg->source_frame->has_source_l_burst_en)
                source_frame.sourceLBurstEn = enc_param_msg->source_frame->source_l_burst_en;
            if (enc_param_msg->source_frame->has_src_buf_state)
                source_frame.srcBufState = enc_param_msg->source_frame->src_buf_state;
            if (enc_param_msg->source_frame->has_sequence_no)
                source_frame.sequenceNo = enc_param_msg->source_frame->sequence_no;
            if (enc_param_msg->source_frame->has_packed_enable)
                source_frame.packedEnable = enc_param_msg->source_frame->packed_enable;
            if (enc_param_msg->source_frame->has_packed_mode)
                source_frame.packedMode = enc_param_msg->source_frame->packed_mode;
            if (enc_param_msg->source_frame->has_update_fb_info)
                source_frame.updateFbInfo = enc_param_msg->source_frame->update_fb_info;
        }

        if (enc_param_msg->offset_tbl_buffer) {
            enc_param->OffsetTblBuffer = &offset_tbl_buffer;

            if (enc_param_msg->offset_tbl_buffer->has_buf_y)
                offset_tbl_buffer.bufY = enc_param_msg->offset_tbl_buffer->buf_y;
            if (enc_param_msg->offset_tbl_buffer->has_buf_cb)
                offset_tbl_buffer.bufCb = enc_param_msg->offset_tbl_buffer->buf_cb;
            if (enc_param_msg->offset_tbl_buffer->has_buf_cr)
                offset_tbl_buffer.bufCr = enc_param_msg->offset_tbl_buffer->buf_cr;
            if (enc_param_msg->offset_tbl_buffer->has_buf_y_bot)
                offset_tbl_buffer.bufYBot = enc_param_msg->offset_tbl_buffer->buf_y_bot;
            if (enc_param_msg->offset_tbl_buffer->has_buf_cb_bot)
                offset_tbl_buffer.bufCbBot = enc_param_msg->offset_tbl_buffer->buf_cb_bot;
            if (enc_param_msg->offset_tbl_buffer->has_buf_cr_bot)
                offset_tbl_buffer.bufCrBot = enc_param_msg->offset_tbl_buffer->buf_cr_bot;
            if (enc_param_msg->offset_tbl_buffer->has_buf_y_size)
                offset_tbl_buffer.bufYSize = enc_param_msg->offset_tbl_buffer->buf_y_size;
            if (enc_param_msg->offset_tbl_buffer->has_buf_cb_size)
                offset_tbl_buffer.bufCbSize = enc_param_msg->offset_tbl_buffer->buf_cb_size;
            if (enc_param_msg->offset_tbl_buffer->has_buf_cr_size)
                offset_tbl_buffer.bufCrSize = enc_param_msg->offset_tbl_buffer->buf_cr_size;
            if (enc_param_msg->offset_tbl_buffer->has_cbcr_interleave)
                offset_tbl_buffer.cbcrInterleave = enc_param_msg->offset_tbl_buffer->cbcr_interleave;
            if (enc_param_msg->offset_tbl_buffer->has_nv21)
                offset_tbl_buffer.nv21 = enc_param_msg->offset_tbl_buffer->nv21;
            if (enc_param_msg->offset_tbl_buffer->has_endian)
                offset_tbl_buffer.endian = enc_param_msg->offset_tbl_buffer->endian;
            if (enc_param_msg->offset_tbl_buffer->has_my_index)
                offset_tbl_buffer.myIndex = enc_param_msg->offset_tbl_buffer->my_index;
            if (enc_param_msg->offset_tbl_buffer->has_map_type)
                offset_tbl_buffer.mapType = enc_param_msg->offset_tbl_buffer->map_type;
            if (enc_param_msg->offset_tbl_buffer->has_stride)
                offset_tbl_buffer.stride = enc_param_msg->offset_tbl_buffer->stride;
            if (enc_param_msg->offset_tbl_buffer->has_width)
                offset_tbl_buffer.width = enc_param_msg->offset_tbl_buffer->width;
            if (enc_param_msg->offset_tbl_buffer->has_height)
                offset_tbl_buffer.height = enc_param_msg->offset_tbl_buffer->height;
            if (enc_param_msg->offset_tbl_buffer->has_size)
                offset_tbl_buffer.size = enc_param_msg->offset_tbl_buffer->size;
            if (enc_param_msg->offset_tbl_buffer->has_size_luma)
                offset_tbl_buffer.sizeLuma = enc_param_msg->offset_tbl_buffer->size_luma;
            if (enc_param_msg->offset_tbl_buffer->has_size_chroma)
                offset_tbl_buffer.sizeChroma = enc_param_msg->offset_tbl_buffer->size_chroma;
            if (enc_param_msg->offset_tbl_buffer->has_luma_bit_depth)
                offset_tbl_buffer.lumaBitDepth = enc_param_msg->offset_tbl_buffer->luma_bit_depth;
            if (enc_param_msg->offset_tbl_buffer->has_chroma_bit_depth)
                offset_tbl_buffer.chromaBitDepth = enc_param_msg->offset_tbl_buffer->chroma_bit_depth;
            if (enc_param_msg->offset_tbl_buffer->has_format)
                offset_tbl_buffer.format = enc_param_msg->offset_tbl_buffer->format;
            if (enc_param_msg->offset_tbl_buffer->has_source_l_burst_en)
                offset_tbl_buffer.sourceLBurstEn = enc_param_msg->offset_tbl_buffer->source_l_burst_en;
            if (enc_param_msg->offset_tbl_buffer->has_src_buf_state)
                offset_tbl_buffer.srcBufState = enc_param_msg->offset_tbl_buffer->src_buf_state;
            if (enc_param_msg->offset_tbl_buffer->has_sequence_no)
                offset_tbl_buffer.sequenceNo = enc_param_msg->offset_tbl_buffer->sequence_no;
            if (enc_param_msg->offset_tbl_buffer->has_packed_enable)
                offset_tbl_buffer.packedEnable = enc_param_msg->offset_tbl_buffer->packed_enable;
            if (enc_param_msg->offset_tbl_buffer->has_packed_mode)
                offset_tbl_buffer.packedMode = enc_param_msg->offset_tbl_buffer->packed_mode;
            if (enc_param_msg->offset_tbl_buffer->has_update_fb_info)
                offset_tbl_buffer.updateFbInfo = enc_param_msg->offset_tbl_buffer->update_fb_info;
        }

        if (enc_param_msg->has_force_i_picture)
            enc_param->forceIPicture = enc_param_msg->force_i_picture;
        if (enc_param_msg->has_skip_picture)
            enc_param->skipPicture = enc_param_msg->skip_picture;
        if (enc_param_msg->has_quant_param)
            enc_param->quantParam = enc_param_msg->quant_param;
        if (enc_param_msg->has_pic_stream_buffer_addr)
            enc_param->picStreamBufferAddr = enc_param_msg->pic_stream_buffer_addr;
        if (enc_param_msg->has_pic_stream_buffer_size)
            enc_param->picStreamBufferSize = enc_param_msg->pic_stream_buffer_size;
        if (enc_param_msg->has_field_run)
            enc_param->fieldRun = enc_param_msg->field_run;
        if (enc_param_msg->has_force_pic_qp_enable)
            enc_param->forcePicQpEnable = enc_param_msg->force_pic_qp_enable;
        if (enc_param_msg->has_force_pic_qp_i)
            enc_param->forcePicQpI = enc_param_msg->force_pic_qp_i;
        if (enc_param_msg->has_force_pic_qp_p)
            enc_param->forcePicQpP = enc_param_msg->force_pic_qp_p;
        if (enc_param_msg->has_force_pic_qp_b)
            enc_param->forcePicQpB = enc_param_msg->force_pic_qp_b;
        if (enc_param_msg->has_force_pic_type_enable)
            enc_param->forcePicTypeEnable = enc_param_msg->force_pic_type_enable;
        if (enc_param_msg->has_force_pic_type)
            enc_param->forcePicType = enc_param_msg->force_pic_type;
        if (enc_param_msg->has_src_idx)
            enc_param->srcIdx = enc_param_msg->src_idx;
        if (enc_param_msg->has_src_end_flag)
            enc_param->srcEndFlag = enc_param_msg->src_end_flag;
        if (enc_param_msg->has_use_cur_src_as_longterm_pic)
            enc_param->useCurSrcAsLongtermPic = enc_param_msg->use_cur_src_as_longterm_pic;
        if (enc_param_msg->has_use_longterm_ref)
            enc_param->useLongtermRef = enc_param_msg->use_longterm_ref;
        if (enc_param_msg->has_pts)
            enc_param->pts = enc_param_msg->pts;
        if (enc_param_msg->has_sram_mode)
            enc_param->sramMode = enc_param_msg->sram_mode;
        if (enc_param_msg->has_coda9_roi_enable)
            enc_param->coda9RoiEnable = enc_param_msg->coda9_roi_enable;
        if (enc_param_msg->has_coda9_roi_pic_avg_qp)
            enc_param->coda9RoiPicAvgQp = enc_param_msg->coda9_roi_pic_avg_qp;
        if (enc_param_msg->has_roi_qp_map_addr)
            enc_param->roiQpMapAddr = enc_param_msg->roi_qp_map_addr;
        if (enc_param_msg->has_non_roi_qp)
            enc_param->nonRoiQp = enc_param_msg->non_roi_qp;
        if (enc_param_msg->has_wp_pix_sigma_y)
            enc_param->wpPixSigmaY = enc_param_msg->wp_pix_sigma_y;
        if (enc_param_msg->has_wp_pix_sigma_cb)
            enc_param->wpPixSigmaCb = enc_param_msg->wp_pix_sigma_cb;
        if (enc_param_msg->has_wp_pix_sigma_cr)
            enc_param->wpPixSigmaCr = enc_param_msg->wp_pix_sigma_cr;
        if (enc_param_msg->has_wp_pix_mean_y)
            enc_param->wpPixMeanY = enc_param_msg->wp_pix_mean_y;
        if (enc_param_msg->has_wp_pix_mean_cb)
            enc_param->wpPixMeanCb = enc_param_msg->wp_pix_mean_cb;
        if (enc_param_msg->has_wp_pix_mean_cr)
            enc_param->wpPixMeanCr = enc_param_msg->wp_pix_mean_cr;
        if (enc_param_msg->has_force_all_ctu_coef_drop_enable)
            enc_param->forceAllCtuCoefDropEnable = enc_param_msg->force_all_ctu_coef_drop_enable;
        if (enc_param_msg->has_user_filter_level_enable)
            enc_param->userFilterLevelEnable = enc_param_msg->user_filter_level_enable;
        if (enc_param_msg->has_lf_filter_level)
            enc_param->lfFilterLevel = enc_param_msg->lf_filter_level;
        if (enc_param_msg->has_sharp_level)
            enc_param->sharpLevel = enc_param_msg->sharp_level;
        if (enc_param_msg->has_lf_ref_delta_intra)
            enc_param->lfRefDeltaIntra = enc_param_msg->lf_ref_delta_intra;
        if (enc_param_msg->has_lf_ref_delta_ref0)
            enc_param->lfRefDeltaRef0 = enc_param_msg->lf_ref_delta_ref0;
        if (enc_param_msg->has_lf_ref_delta_ref1)
            enc_param->lfRefDeltaRef1 = enc_param_msg->lf_ref_delta_ref1;
        if (enc_param_msg->has_lf_mode_delta)
            enc_param->lfModeDelta = enc_param_msg->lf_mode_delta;
        if (enc_param_msg->has_svc_layer_flag)
            enc_param->svcLayerFlag = enc_param_msg->svc_layer_flag;

        if (enc_param_msg->set_roi) {
            if (enc_param_msg->set_roi->has_mode)
                enc_param->setROI.mode = enc_param_msg->set_roi->mode;
            if (enc_param_msg->set_roi->has_number)
                enc_param->setROI.number = enc_param_msg->set_roi->number;
            if (enc_param_msg->set_roi->n_region && enc_param_msg->set_roi->region) {
                for (size_t i = 0; i < enc_param_msg->set_roi->n_region; i++) {
                    if (enc_param_msg->set_roi->region[i]->has_left)
                        enc_param->setROI.region[i].left = enc_param_msg->set_roi->region[i]->left;
                    if (enc_param_msg->set_roi->region[i]->has_top)
                        enc_param->setROI.region[i].top = enc_param_msg->set_roi->region[i]->top;
                    if (enc_param_msg->set_roi->region[i]->has_right)
                        enc_param->setROI.region[i].right =
                            enc_param_msg->set_roi->region[i]->right;
                    if (enc_param_msg->set_roi->region[i]->has_bottom)
                        enc_param->setROI.region[i].bottom =
                            enc_param_msg->set_roi->region[i]->bottom;
                }
            }
            if (enc_param_msg->set_roi->n_qp && enc_param_msg->set_roi->qp) {
                for (size_t i = 0; i < enc_param_msg->set_roi->n_qp; i++) {
                    enc_param->setROI.qp[i] = enc_param_msg->set_roi->qp[i];
                }
            }
        }
        if (enc_param_msg->ctu_opt_param) {
            if (enc_param_msg->ctu_opt_param->has_roi_enable)
                enc_param->ctuOptParam.roiEnable = enc_param_msg->ctu_opt_param->roi_enable;
            if (enc_param_msg->ctu_opt_param->has_roi_delta_qp)
                enc_param->ctuOptParam.roiDeltaQp = enc_param_msg->ctu_opt_param->roi_delta_qp;
            if (enc_param_msg->ctu_opt_param->has_map_endian)
                enc_param->ctuOptParam.mapEndian = enc_param_msg->ctu_opt_param->map_endian;
            if (enc_param_msg->ctu_opt_param->has_map_stride)
                enc_param->ctuOptParam.mapStride = enc_param_msg->ctu_opt_param->map_stride;
            if (enc_param_msg->ctu_opt_param->has_addr_roi_ctu_map)
                enc_param->ctuOptParam.addrRoiCtuMap =
                    enc_param_msg->ctu_opt_param->addr_roi_ctu_map;
        }
        if (enc_param_msg->code_option) {
            if (enc_param_msg->code_option->has_implicit_header_encode)
                enc_param->codeOption.implicitHeaderEncode =
                    enc_param_msg->code_option->implicit_header_encode;
            if (enc_param_msg->code_option->has_encode_vcl)
                enc_param->codeOption.encodeVCL = enc_param_msg->code_option->encode_vcl;
            if (enc_param_msg->code_option->has_encode_vps)
                enc_param->codeOption.encodeVPS = enc_param_msg->code_option->encode_vps;
            if (enc_param_msg->code_option->has_encode_sps)
                enc_param->codeOption.encodeSPS = enc_param_msg->code_option->encode_sps;
            if (enc_param_msg->code_option->has_encode_pps)
                enc_param->codeOption.encodePPS = enc_param_msg->code_option->encode_pps;
            if (enc_param_msg->code_option->has_encode_aud)
                enc_param->codeOption.encodeAUD = enc_param_msg->code_option->encode_aud;
            if (enc_param_msg->code_option->has_encode_eos)
                enc_param->codeOption.encodeEOS = enc_param_msg->code_option->encode_eos;
            if (enc_param_msg->code_option->has_encode_eob)
                enc_param->codeOption.encodeEOB = enc_param_msg->code_option->encode_eob;
            if (enc_param_msg->code_option->has_encode_vui)
                enc_param->codeOption.encodeVUI = enc_param_msg->code_option->encode_vui;
        }
        if (enc_param_msg->custom_map_opt) {
            if (enc_param_msg->custom_map_opt->has_roi_avg_qp)
                enc_param->customMapOpt.roiAvgQp = enc_param_msg->custom_map_opt->roi_avg_qp;
            if (enc_param_msg->custom_map_opt->has_custom_roi_map_enable)
                enc_param->customMapOpt.customRoiMapEnable =
                    enc_param_msg->custom_map_opt->custom_roi_map_enable;
            if (enc_param_msg->custom_map_opt->has_custom_lambda_map_enable)
                enc_param->customMapOpt.customLambdaMapEnable =
                    enc_param_msg->custom_map_opt->custom_lambda_map_enable;
            if (enc_param_msg->custom_map_opt->has_custom_mode_map_enable)
                enc_param->customMapOpt.customModeMapEnable =
                    enc_param_msg->custom_map_opt->custom_mode_map_enable;
            if (enc_param_msg->custom_map_opt->has_custom_coef_drop_enable)
                enc_param->customMapOpt.customCoefDropEnable =
                    enc_param_msg->custom_map_opt->custom_coef_drop_enable;
            if (enc_param_msg->custom_map_opt->has_addr_custom_map)
                enc_param->customMapOpt.addrCustomMap =
                    enc_param_msg->custom_map_opt->addr_custom_map;
        }

        if (enc_param_msg->has_new_bit_rate) {
            ret = VPU_EncGiveCommand(handle, ENC_SET_BITRATE, &enc_param_msg->new_bit_rate);
            if (ret != RETCODE_SUCCESS) {
                enc_one_frame_msg_ret->has_ret_code = TRUE;
                enc_one_frame_msg_ret->ret_code = ret;
                VLOG(ERR, "[%s][%d] ENC_SET_BITRATE failed, ret:%x", __func__, __LINE__, ret);
                return 0;
            }
        }

        if (enc_param_msg->has_new_frame_rate) {
            ret = VPU_EncGiveCommand(handle, ENC_SET_FRAME_RATE, &enc_param_msg->new_frame_rate);
            if (ret != RETCODE_SUCCESS) {
                enc_one_frame_msg_ret->has_ret_code = TRUE;
                enc_one_frame_msg_ret->ret_code = ret;
                VLOG(ERR, "[%s][%d] ENC_SET_FRAME_RATE failed, ret:%x", __func__, __LINE__, ret);
                return 0;
            }
        }

        if (enc_param_msg->has_new_gop_size) {
            ret = VPU_EncGiveCommand(handle, ENC_SET_GOP_NUMBER, &enc_param_msg->new_gop_size);
            if (ret != RETCODE_SUCCESS) {
                enc_one_frame_msg_ret->has_ret_code = TRUE;
                enc_one_frame_msg_ret->ret_code = ret;
                VLOG(ERR, "[%s][%d] ENC_SET_GOP_NUMBER failed, ret:%x", __func__, __LINE__, ret);
                return 0;
            }
        }
    }

    ret = VPU_EncStartOneFrame(handle, enc_param);
    enc_one_frame_msg_ret->has_ret_code = TRUE;
    enc_one_frame_msg_ret->ret_code = ret;
    if (ret != RETCODE_SUCCESS) {
        VLOG(ERR, "handle:%p VPU_EncStartOneFrame failed Error code is 0x%x \n", handle, ret);
        return 0;
    }

    // VPU_WaitInterrupt
    if (handle->coreIdx == VPU_WAVE521_IDX) {
        ret = wave5_enc_hand_interrupt(handle);
        enc_one_frame_msg_ret->has_interrupt_reaseon = TRUE;
        enc_one_frame_msg_ret->interrupt_reaseon = ret;

        if (ret == ENC_INT_STATUS_TIMEOUT) {
            enc_one_frame_msg_ret->ret_code = RETCODE_FAILURE;
            VPU_EncGetOutputInfo(handle, output_info);
            // the third parameter of VPU_SWReset requires the pending instance
            VPU_SWReset(handle->coreIdx, SW_RESET_SAFETY, NULL);
            VLOG(ERR, "handle:%p wave5_enc_hand_interrupt timeout, ret is 0x%x rdPtr:%x wrPtr:%x\n",
                 handle, ret, output_info->rdPtr, output_info->wrPtr);
            return 0;
        } else if (ret != ENC_INT_STATUS_DONE) {
            enc_one_frame_msg_ret->ret_code = RETCODE_FAILURE;
            VPU_EncGetOutputInfo(handle, output_info);
            VLOG(ERR,
                 "handle:%p VPU_WaitInterrupt flag is:%x, instead of ENC_INT_STATUS_DONE, rdPtr:%x "
                 "wrPtr:%x\n",
                 handle, ret, output_info->rdPtr, output_info->wrPtr);
            return 0;
        }
    } else {
        ret = VPU_WaitInterrupt(handle->coreIdx, VPU_ENC_TIMEOUT);
        enc_one_frame_msg_ret->has_interrupt_reaseon = TRUE;
        enc_one_frame_msg_ret->interrupt_reaseon = ret;

        if (ret == -1) {
            enc_one_frame_msg_ret->ret_code = RETCODE_FAILURE;
            VPU_EncGetOutputInfo(handle, output_info);
            // the third parameter of VPU_SWReset requires the pending instance
            VPU_SWReset(handle->coreIdx, SW_RESET_SAFETY, NULL);
            VLOG(ERR, "handle:%p VPU_WaitInterrupt timeout, ret is 0x%x rdPtr:%x wrPtr:%x\n",
                 handle, ret, output_info->rdPtr, output_info->wrPtr);
            return 0;
        }

        if (ret)
            VPU_ClearInterrupt(handle->coreIdx);

        if (ret & (1 << INT_BIT_PIC_RUN)) {
            // normal state
        } else {
            enc_one_frame_msg_ret->ret_code = RETCODE_FAILURE;
            VPU_EncGetOutputInfo(handle, output_info);
            VLOG(ERR,
                 "handle:%p VPU_WaitInterrupt flag is:%x, instead of PIC_RUN, rdPtr:%x wrPtr:%x\n",
                 handle, ret, output_info->rdPtr, output_info->wrPtr);
            return 0;
        }
    }

    // VPU_EncGetOutputInfo
    ret = VPU_EncGetOutputInfo(handle, output_info);
    enc_one_frame_msg_ret->ret_code = ret;
    if (ret != RETCODE_SUCCESS) {
        VLOG(ERR, "handle:%p VPU_EncGetOutputInfo error, ret is 0x%x \n", handle, ret);
        return 0;
    }

    Vpu__EncOutputInfo *output_info_msg = &instance->vpu_proto_info.enc_output_info;

    *output_info_msg = (Vpu__EncOutputInfo)VPU__ENC_OUTPUT_INFO__INIT;
    enc_one_frame_msg_ret->enc_output_info = output_info_msg;

    {
        output_info_msg->bitstream_buffer = output_info->bitstreamBuffer;
        output_info_msg->has_bitstream_buffer = TRUE;
        output_info_msg->bitstream_size = output_info->bitstreamSize;
        output_info_msg->has_bitstream_size = TRUE;
        output_info_msg->bitstream_wrap_around = output_info->bitstreamWrapAround;
        output_info_msg->has_bitstream_wrap_around = TRUE;
        output_info_msg->pic_type = output_info->picType;
        output_info_msg->has_pic_type = TRUE;
        output_info_msg->num_of_slices = output_info->numOfSlices;
        output_info_msg->has_num_of_slices = TRUE;
        output_info_msg->recon_frame_index = output_info->reconFrameIndex;
        output_info_msg->has_recon_frame_index = TRUE;
        output_info_msg->rd_ptr = output_info->rdPtr;
        output_info_msg->has_rd_ptr = TRUE;
        output_info_msg->wr_ptr = output_info->wrPtr;
        output_info_msg->has_wr_ptr = TRUE;
        output_info_msg->pic_skipped = output_info->picSkipped;
        output_info_msg->has_pic_skipped = TRUE;
        output_info_msg->num_of_intra = output_info->numOfIntra;
        output_info_msg->has_num_of_intra = TRUE;
        output_info_msg->num_of_merge = output_info->numOfMerge;
        output_info_msg->has_num_of_merge = TRUE;
        output_info_msg->num_of_skip_block = output_info->numOfSkipBlock;
        output_info_msg->has_num_of_skip_block = TRUE;
        output_info_msg->avg_ctu_qp = output_info->avgCtuQp;
        output_info_msg->has_avg_ctu_qp = TRUE;
        output_info_msg->enc_pic_byte = output_info->encPicByte;
        output_info_msg->has_enc_pic_byte = TRUE;
        output_info_msg->enc_gop_pic_idx = output_info->encGopPicIdx;
        output_info_msg->has_enc_gop_pic_idx = TRUE;
        output_info_msg->enc_pic_poc = output_info->encPicPoc;
        output_info_msg->has_enc_pic_poc = TRUE;
        output_info_msg->enc_src_idx = output_info->encSrcIdx;
        output_info_msg->has_enc_src_idx = TRUE;
        output_info_msg->enc_vcl_nal = output_info->encVclNal;
        output_info_msg->has_enc_vcl_nal = TRUE;
        output_info_msg->enc_pic_cnt = output_info->encPicCnt;
        output_info_msg->has_enc_pic_cnt = TRUE;
        output_info_msg->frame_cycle = output_info->frameCycle;
        output_info_msg->has_frame_cycle = TRUE;
        output_info_msg->pts = output_info->pts;
        output_info_msg->has_pts = TRUE;
        output_info_msg->enc_inst_idx = output_info->encInstIdx;
        output_info_msg->has_enc_inst_idx = TRUE;
        output_info_msg->release_src_flag = output_info->releaseSrcFlag;
        output_info_msg->has_release_src_flag = TRUE;
        output_info_msg->error_reason = output_info->errorReason;
        output_info_msg->has_error_reason = TRUE;
        output_info_msg->warn_info = output_info->warnInfo;
        output_info_msg->has_warn_info = TRUE;
        output_info_msg->cycle_per_tick = output_info->cyclePerTick;
        output_info_msg->has_cycle_per_tick = TRUE;
        output_info_msg->enc_host_cmd_tick = output_info->encHostCmdTick;
        output_info_msg->has_enc_host_cmd_tick = TRUE;
        output_info_msg->enc_prepare_start_tick = output_info->encPrepareStartTick;
        output_info_msg->has_enc_prepare_start_tick = TRUE;
        output_info_msg->enc_prepare_end_tick = output_info->encPrepareEndTick;
        output_info_msg->has_enc_prepare_end_tick = TRUE;
        output_info_msg->enc_processing_start_tick = output_info->encProcessingStartTick;
        output_info_msg->has_enc_processing_start_tick = TRUE;
        output_info_msg->enc_processing_end_tick = output_info->encProcessingEndTick;
        output_info_msg->has_enc_processing_end_tick = TRUE;
        output_info_msg->enc_encode_start_tick = output_info->encEncodeStartTick;
        output_info_msg->has_enc_encode_start_tick = TRUE;
        output_info_msg->enc_encode_end_tick = output_info->encEncodeEndTick;
        output_info_msg->has_enc_encode_end_tick = TRUE;
        output_info_msg->prepare_cycle = output_info->prepareCycle;
        output_info_msg->has_prepare_cycle = TRUE;
        output_info_msg->processing = output_info->processing;
        output_info_msg->has_processing = TRUE;
        output_info_msg->encoded_cycle = output_info->EncodedCycle;
        output_info_msg->has_encoded_cycle = TRUE;
        output_info_msg->pic_distortion_low = output_info->picDistortionLow;
        output_info_msg->has_pic_distortion_low = TRUE;
        output_info_msg->pic_distortion_high = output_info->picDistortionHigh;
        output_info_msg->has_pic_distortion_high = TRUE;
        output_info_msg->is_svc_layer_el = output_info->isSvcLayerEL;
        output_info_msg->has_is_svc_layer_el = TRUE;

        /* TODO: add reconFrame / mbInfo / mvInfo / sliceInfo
         *
         */
    }

    return 0;
}

int vpu_instance_ipc_enc_close(vpu_instance_t *instance, Vpu__EncClose *enc_close_msg)
{
    if (!enc_close_msg->handle || !enc_close_msg->handle->handle) {
        VLOG(ERR, "[%s][%d] invailed handle\n", __func__, __LINE__);
        return -1;
    }

    int ret = 0;
    int index = 0;
    EncHandle handle = (EncHandle)enc_close_msg->handle->handle;
    VLOG(INFO, "[%s][%d] handle:%p\n", __func__, __LINE__, handle);

    ret = VPU_EncClose(handle);
    if (ret != RETCODE_SUCCESS)
        VLOG(ERR, "handle:%p VPU_EncClose failed Error code is 0x%x \n", handle, ret);

    enc_close_msg->has_ret_code = TRUE;
    enc_close_msg->ret_code = ret;

    for (; index < VPU_MAX_NUM_INSTANCE; index++) {
        if (instance->vpu_instance_contrl.inst_handle[index] == (uint32_t)handle)
            break;
    }

    if (index >= VPU_MAX_NUM_INSTANCE) {
        VLOG(WARN, "[%s][%d] invaild handle:%p\n", __func__, __LINE__, handle);
        return 0;
    }

    instance->vpu_instance_contrl.inst_handle[index] = 0;
    instance->vpu_instance_contrl.inst_score[index] = 0;
    instance->vpu_instance_contrl.inst_enable_sram[index] = 0;

    return 0;
}

int _vpu_instance_process_msg(vpu_ipcc_data_t *pack, vpu_instance_t *vpu_instance)
{
    int ret = 0;
    int ret_msg_len = 0;

    if (!pack || !vpu_instance)
        return -1;

    switch (pack->cmd) {
    case VPU_IPC_DEC_OPEN: {
        Vpu__DecOpen *dec_open_msg =
            vpu__dec_open__unpack(NULL, pack->cmd_param_full_length, pack->cmd_param);
        if (!dec_open_msg) {
            VLOG(ERR, "[%s][%d] vpu__dec_open__unpack failed\n", __func__, __LINE__);
            pack->return_value = VPU_IPC_RETURN_ERROR;
            return -1;
        }

        ret = vpu_instance_ipc_dec_open(vpu_instance, dec_open_msg);
        if (ret >= 0) {
            ret_msg_len = vpu__dec_open__get_packed_size(dec_open_msg);
            vpu__dec_open__pack(dec_open_msg, pack->cmd_param);

            pack->cmd_param_full_length = ret_msg_len;
            pack->current_packed_start_offset = 0;
            pack->current_packed_end_offset = ret_msg_len;
        }

        vpu__dec_open__free_unpacked(dec_open_msg, NULL);
        break;
    }
    case VPU_IPC_DEC_GIVE_COMMAND:
        break;
    case VPU_IPC_DEC_GET_BITSTREAM_BUFFER: {
        Vpu__DecGetBitstreamBuffer *get_bs_buf_msg = vpu__dec_get_bitstream_buffer__unpack(
            NULL, pack->cmd_param_full_length, pack->cmd_param);
        if (!get_bs_buf_msg) {
            VLOG(ERR, "[%s][%d] vpu__dec_get_bitstream_buffer__unpack failed\n", __func__,
                 __LINE__);
            pack->return_value = VPU_IPC_RETURN_ERROR;
            return -1;
        }

        ret = vpu_instance_ipc_get_bitstream_buffer(get_bs_buf_msg);
        if (ret >= 0) {
            ret_msg_len = vpu__dec_get_bitstream_buffer__get_packed_size(get_bs_buf_msg);
            vpu__dec_get_bitstream_buffer__pack(get_bs_buf_msg, pack->cmd_param);

            pack->cmd_param_full_length = ret_msg_len;
            pack->current_packed_start_offset = 0;
            pack->current_packed_end_offset = ret_msg_len;
        }

        vpu__dec_get_bitstream_buffer__free_unpacked(get_bs_buf_msg, NULL);
        break;
    }
    case VPU_IPC_DEC_UPDATE_BITSTREAMER_BUFFER: {
        Vpu__DecUpdateBitstreamBuffer *update_bs_buf_msg = vpu__dec_update_bitstream_buffer__unpack(
            NULL, pack->cmd_param_full_length, pack->cmd_param);
        if (!update_bs_buf_msg) {
            VLOG(ERR, "[%s][%d] vpu__dec_update_bitstream_buffer__unpack failed\n", __func__,
                 __LINE__);
            pack->return_value = VPU_IPC_RETURN_ERROR;
            return -1;
        }

        ret = vpu_instance_ipc_update_bitstream_buffer(update_bs_buf_msg);
        if (ret >= 0) {
            ret_msg_len = vpu__dec_update_bitstream_buffer__get_packed_size(update_bs_buf_msg);
            vpu__dec_update_bitstream_buffer__pack(update_bs_buf_msg, pack->cmd_param);

            pack->cmd_param_full_length = ret_msg_len;
            pack->current_packed_start_offset = 0;
            pack->current_packed_end_offset = ret_msg_len;
        }

        vpu__dec_update_bitstream_buffer__free_unpacked(update_bs_buf_msg, NULL);
        break;
    }
    case VPU_IPC_DEC_PIC_END_MODE_UPDATE_BS_BUFFER: {
        Vpu__DecPicEndModeUpdateBSBuffer *pic_end_mode_update_bs_buf_msg =
            vpu__dec_pic_end_mode_update_bsbuffer__unpack(NULL, pack->cmd_param_full_length,
                                                          pack->cmd_param);
        if (!pic_end_mode_update_bs_buf_msg) {
            VLOG(ERR, "[%s][%d] vpu__dec_pic_end_mode_update_bsbuffer__unpack failed\n", __func__,
                 __LINE__);
            pack->return_value = VPU_IPC_RETURN_ERROR;
            return -1;
        }

        ret = vpu_instance_ipc_pic_end_mode_update_bs_buffer(pic_end_mode_update_bs_buf_msg);
        if (ret >= 0) {
            ret_msg_len = vpu__dec_pic_end_mode_update_bsbuffer__get_packed_size(
                pic_end_mode_update_bs_buf_msg);
            vpu__dec_pic_end_mode_update_bsbuffer__pack(pic_end_mode_update_bs_buf_msg,
                                                        pack->cmd_param);

            pack->cmd_param_full_length = ret_msg_len;
            pack->current_packed_start_offset = 0;
            pack->current_packed_end_offset = ret_msg_len;
        }

        vpu__dec_pic_end_mode_update_bsbuffer__free_unpacked(pic_end_mode_update_bs_buf_msg, NULL);
        break;
    }
    case VPU_IPC_DEC_SEQINIT: {
        Vpu__DecSeqInit *seq_init_msg =
            vpu__dec_seq_init__unpack(NULL, pack->cmd_param_full_length, pack->cmd_param);
        Vpu__DecSeqInit *seq_init_msg_ret = &vpu_instance->vpu_proto_info.init_seq_msg_ret;
        if (!seq_init_msg) {
            VLOG(ERR, "[%s][%d] vpu__dec_seq_init__unpack failed\n", __func__, __LINE__);
            pack->return_value = VPU_IPC_RETURN_ERROR;
            return -1;
        }

        ret = vpu_instance_ipc_dec_seq_init(seq_init_msg, seq_init_msg_ret, vpu_instance);
        if (ret >= 0) {
            ret_msg_len = vpu__dec_seq_init__get_packed_size(seq_init_msg_ret);
            vpu__dec_seq_init__pack(seq_init_msg_ret, pack->cmd_param);

            pack->cmd_param_full_length = ret_msg_len;
            pack->current_packed_start_offset = 0;
            pack->current_packed_end_offset = ret_msg_len;
        }
        vpu__dec_seq_init__free_unpacked(seq_init_msg, NULL);
        break;
    }
    case VPU_IPC_DEC_REGISTER_DECODE_BUFFER: {
        Vpu__DecRegisterDecodeBuffer *register_dec_buf_msg =
            vpu__dec_register_decode_buffer__unpack(NULL, pack->cmd_param_full_length,
                                                    pack->cmd_param);
        if (!register_dec_buf_msg) {
            VLOG(ERR, "[%s][%d] vpu__dec_register_decode_buffer__unpack failed\n", __func__,
                 __LINE__);
            pack->return_value = VPU_IPC_RETURN_ERROR;
            return -1;
        }

        ret = vpu_instance_ipc_dec_register_dec_buf(register_dec_buf_msg, vpu_instance);
        if (ret >= 0) {
            ret_msg_len = vpu__dec_register_decode_buffer__get_packed_size(register_dec_buf_msg);
            vpu__dec_register_decode_buffer__pack(register_dec_buf_msg, pack->cmd_param);

            pack->cmd_param_full_length = ret_msg_len;
            pack->current_packed_start_offset = 0;
            pack->current_packed_end_offset = ret_msg_len;
        }
        vpu__dec_register_decode_buffer__free_unpacked(register_dec_buf_msg, NULL);
        break;
    }
    case VPU_IPC_DEC_SET_RD_PTR: {
        Vpu__DecSetRdPtr *set_rd_ptr_msg =
            vpu__dec_set_rd_ptr__unpack(NULL, pack->cmd_param_full_length, pack->cmd_param);
        if (!set_rd_ptr_msg) {
            VLOG(ERR, "[%s][%d] vpu__dec_set_rd_ptr__unpack failed\n", __func__, __LINE__);
            pack->return_value = VPU_IPC_RETURN_ERROR;
            return -1;
        }

        ret = vpu_instance_ipc_dec_set_rd_ptr(set_rd_ptr_msg);
        if (ret >= 0) {
            ret_msg_len = vpu__dec_set_rd_ptr__get_packed_size(set_rd_ptr_msg);
            vpu__dec_set_rd_ptr__pack(set_rd_ptr_msg, pack->cmd_param);

            pack->cmd_param_full_length = ret_msg_len;
            pack->current_packed_start_offset = 0;
            pack->current_packed_end_offset = ret_msg_len;
        }
        vpu__dec_set_rd_ptr__free_unpacked(set_rd_ptr_msg, NULL);
        break;
    }
    case VPU_IPC_DEC_ONE_FRAMER: {
        Vpu__DecOneFrame *dec_one_frame_msg =
            vpu__dec_one_frame__unpack(NULL, pack->cmd_param_full_length, pack->cmd_param);
        Vpu__DecOneFrame *dec_one_frame_msg_ret = &vpu_instance->vpu_proto_info.dec_frame_msg_ret;
        if (!dec_one_frame_msg) {
            VLOG(ERR, "[%s][%d] vpu__dec_one_frame__unpack failed\n", __func__, __LINE__);
            pack->return_value = VPU_IPC_RETURN_ERROR;
            return -1;
        }

        ret =
            vpu_instance_ipc_dec_one_frame(dec_one_frame_msg, dec_one_frame_msg_ret, vpu_instance);
        if (ret >= 0) {
            ret_msg_len = vpu__dec_one_frame__get_packed_size(dec_one_frame_msg_ret);
            vpu__dec_one_frame__pack(dec_one_frame_msg_ret, pack->cmd_param);

            pack->cmd_param_full_length = ret_msg_len;
            pack->current_packed_start_offset = 0;
            pack->current_packed_end_offset = ret_msg_len;
        }
        vpu__dec_one_frame__free_unpacked(dec_one_frame_msg, NULL);
        break;
    }
    case VPU_IPC_DEC_FLUSH: {
        Vpu__DecFlush *dec_flush_msg =
            vpu__dec_flush__unpack(NULL, pack->cmd_param_full_length, pack->cmd_param);
        if (!dec_flush_msg) {
            VLOG(ERR, "[%s][%d] vpu__dec_flush__unpack failed\n", __func__, __LINE__);
            pack->return_value = VPU_IPC_RETURN_ERROR;
            return -1;
        }

        ret = vpu_instance_ipc_dec_flush(dec_flush_msg);
        if (ret >= 0) {
            ret_msg_len = vpu__dec_flush__get_packed_size(dec_flush_msg);
            vpu__dec_flush__pack(dec_flush_msg, pack->cmd_param);

            pack->cmd_param_full_length = ret_msg_len;
            pack->current_packed_start_offset = 0;
            pack->current_packed_end_offset = ret_msg_len;
        }
        vpu__dec_flush__free_unpacked(dec_flush_msg, NULL);
        break;
    }
    case VPU_IPC_DEC_CLOSE: {
        Vpu__DecClose *dec_close_msg =
            vpu__dec_close__unpack(NULL, pack->cmd_param_full_length, pack->cmd_param);
        if (!dec_close_msg) {
            VLOG(ERR, "[%s][%d] vpu__dec_close__unpack failed\n", __func__, __LINE__);
            pack->return_value = VPU_IPC_RETURN_ERROR;
            return -1;
        }

        ret = vpu_instance_ipc_dec_close(vpu_instance, dec_close_msg);
        if (ret >= 0) {
            ret_msg_len = vpu__dec_close__get_packed_size(dec_close_msg);
            vpu__dec_close__pack(dec_close_msg, pack->cmd_param);

            pack->cmd_param_full_length = ret_msg_len;
            pack->current_packed_start_offset = 0;
            pack->current_packed_end_offset = ret_msg_len;
        }
        vpu__dec_close__free_unpacked(dec_close_msg, NULL);
        break;
    }
    case VPU_IPC_SWRESET: {
        Vpu__SWReset *sw_reset_msg =
            vpu__swreset__unpack(NULL, pack->cmd_param_full_length, pack->cmd_param);
        if (!sw_reset_msg) {
            VLOG(ERR, "[%s][%d] vpu__swreset__unpack failed\n", __func__, __LINE__);
            pack->return_value = VPU_IPC_RETURN_ERROR;
            return -1;
        }

        ret = vpu_instance_ipc_sw_reset(sw_reset_msg);
        if (ret >= 0) {
            ret_msg_len = vpu__swreset__get_packed_size(sw_reset_msg);
            vpu__swreset__pack(sw_reset_msg, pack->cmd_param);

            pack->cmd_param_full_length = ret_msg_len;
            pack->current_packed_start_offset = 0;
            pack->current_packed_end_offset = ret_msg_len;
        }
        vpu__swreset__free_unpacked(sw_reset_msg, NULL);
        break;
    }
    case VPU_IPC_ENC_OPEN: {
        Vpu__EncOpen *enc_open_msg =
            vpu__enc_open__unpack(NULL, pack->cmd_param_full_length, pack->cmd_param);
        if (!enc_open_msg) {
            VLOG(ERR, "[%s][%d] vpu__enc_open__unpack failed\n", __func__, __LINE__);
            pack->return_value = VPU_IPC_RETURN_ERROR;
            return -1;
        }

        ret = vpu_instance_ipc_enc_open(vpu_instance, enc_open_msg);
        if (ret >= 0) {
            ret_msg_len = vpu__enc_open__get_packed_size(enc_open_msg);
            vpu__enc_open__pack(enc_open_msg, pack->cmd_param);

            pack->cmd_param_full_length = ret_msg_len;
            pack->current_packed_start_offset = 0;
            pack->current_packed_end_offset = ret_msg_len;
        }

        vpu__enc_open__free_unpacked(enc_open_msg, NULL);
        break;
    }
    case VPU_IPC_ENC_GIVE_COMMAND: {
        break;
    }
    case VPU_IPC_ENC_GET_INITIAL_INFO: {
        Vpu__EncGetInitialInfo *enc_get_init_info_msg =
            vpu__enc_get_initial_info__unpack(NULL, pack->cmd_param_full_length, pack->cmd_param);
        Vpu__EncGetInitialInfo *enc_get_init_info_msg_ret =
            &vpu_instance->vpu_proto_info.enc_init_info_msg_ret;
        if (!enc_get_init_info_msg) {
            VLOG(ERR, "[%s][%d] vpu__enc_get_initial_info__unpack failed\n", __func__, __LINE__);
            pack->return_value = VPU_IPC_RETURN_ERROR;
            return -1;
        }

        ret = vpu_instance_ipc_enc_get_init_info(enc_get_init_info_msg, enc_get_init_info_msg_ret,
                                                 vpu_instance);
        if (ret >= 0) {
            ret_msg_len = vpu__enc_get_initial_info__get_packed_size(enc_get_init_info_msg_ret);
            ret = vpu__enc_get_initial_info__pack(enc_get_init_info_msg_ret, pack->cmd_param);

            pack->cmd_param_full_length = ret_msg_len;
            pack->current_packed_start_offset = 0;
            pack->current_packed_end_offset = ret_msg_len;
        }
        vpu__enc_get_initial_info__free_unpacked(enc_get_init_info_msg, NULL);
        break;
    }
    case VPU_IPC_ENC_REGISTER_ENCODE_BUFFER: {
        Vpu__EncRegisterEncodeBuffer *enc_register_enc_buf_msg =
            vpu__enc_register_encode_buffer__unpack(NULL, pack->cmd_param_full_length,
                                                    pack->cmd_param);
        if (!enc_register_enc_buf_msg) {
            VLOG(ERR, "[%s][%d] vpu__enc_register_encode_buffer__unpack failed\n", __func__,
                 __LINE__);
            pack->return_value = VPU_IPC_RETURN_ERROR;
            return -1;
        }

        ret = vpu_instance_ipc_enc_register_enc_buf(enc_register_enc_buf_msg, vpu_instance);
        if (ret >= 0) {
            ret_msg_len =
                vpu__enc_register_encode_buffer__get_packed_size(enc_register_enc_buf_msg);
            vpu__enc_register_encode_buffer__pack(enc_register_enc_buf_msg, pack->cmd_param);

            pack->cmd_param_full_length = ret_msg_len;
            pack->current_packed_start_offset = 0;
            pack->current_packed_end_offset = ret_msg_len;
        }
        vpu__enc_register_encode_buffer__free_unpacked(enc_register_enc_buf_msg, NULL);
        break;
    }
    case VPU_IPC_ENC_GET_HEAD: {
        Vpu__EncGetHead *enc_get_head_msg =
            vpu__enc_get_head__unpack(NULL, pack->cmd_param_full_length, pack->cmd_param);
        Vpu__EncGetHead *enc_get_head_msg_ret = &vpu_instance->vpu_proto_info.enc_get_head_msg_ret;
        if (!enc_get_head_msg) {
            VLOG(ERR, "[%s][%d] vpu__enc_get_head__unpack failed\n", __func__, __LINE__);
            pack->return_value = VPU_IPC_RETURN_ERROR;
            return -1;
        }

        ret = vpu_instance_ipc_enc_get_head(enc_get_head_msg, enc_get_head_msg_ret, vpu_instance);
        if (ret >= 0) {
            ret_msg_len = vpu__enc_get_head__get_packed_size(enc_get_head_msg_ret);
            vpu__enc_get_head__pack(enc_get_head_msg_ret, pack->cmd_param);

            pack->cmd_param_full_length = ret_msg_len;
            pack->current_packed_start_offset = 0;
            pack->current_packed_end_offset = ret_msg_len;
        }
        vpu__enc_get_head__free_unpacked(enc_get_head_msg, NULL);
        break;
    }
    case VPU_IPC_ENC_ONE_FRAMER: {
        Vpu__EncOneFrame *enc_one_frame_msg =
            vpu__enc_one_frame__unpack(NULL, pack->cmd_param_full_length, pack->cmd_param);
        Vpu__EncOneFrame *enc_one_frame_msg_ret = &vpu_instance->vpu_proto_info.enc_frame_msg_ret;
        if (!enc_one_frame_msg) {
            VLOG(ERR, "[%s][%d] vpu__enc_one_frame__unpack failed\n", __func__, __LINE__);
            pack->return_value = VPU_IPC_RETURN_ERROR;
            return -1;
        }

        ret =
            vpu_instance_ipc_enc_one_frame(enc_one_frame_msg, enc_one_frame_msg_ret, vpu_instance);
        if (ret >= 0) {
            ret_msg_len = vpu__enc_one_frame__get_packed_size(enc_one_frame_msg_ret);
            vpu__enc_one_frame__pack(enc_one_frame_msg_ret, pack->cmd_param);

            pack->cmd_param_full_length = ret_msg_len;
            pack->current_packed_start_offset = 0;
            pack->current_packed_end_offset = ret_msg_len;
        }
        vpu__enc_one_frame__free_unpacked(enc_one_frame_msg, NULL);
        break;
    }
    case VPU_IPC_ENC_CLOSE: {
        Vpu__EncClose *enc_close_msg =
            vpu__enc_close__unpack(NULL, pack->cmd_param_full_length, pack->cmd_param);
        if (!enc_close_msg) {
            VLOG(ERR, "[%s][%d] vpu__enc_close__unpack failed\n", __func__, __LINE__);
            pack->return_value = VPU_IPC_RETURN_ERROR;
            return -1;
        }

        ret = vpu_instance_ipc_enc_close(vpu_instance, enc_close_msg);
        if (ret >= 0) {
            ret_msg_len = vpu__enc_close__get_packed_size(enc_close_msg);
            vpu__enc_close__pack(enc_close_msg, pack->cmd_param);

            pack->cmd_param_full_length = ret_msg_len;
            pack->current_packed_start_offset = 0;
            pack->current_packed_end_offset = ret_msg_len;
        }
        vpu__enc_close__free_unpacked(enc_close_msg, NULL);
        break;
    }
    case VPU_IPC_CLEAR_INSTANCE: {
        ret = vpu_instance_ipc_clear_instance(vpu_instance, *((uint32_t *)pack->cmd_param));
        break;
    }
    default:
        // invalid cmd id
        pack->return_value = VPU_IPC_RETURN_ERROR;
        break;
    }

    return ret;
}

int vpu_instance_process_msg(vpu_ipcc_data_t *pack)
{
    int ret = 0;

    if (!pack)
        return -1;

    switch (pack->dev_id) {
    case VPU_DEVICES_CODA988:
        ret = _vpu_instance_process_msg(pack, vpu_instance_coda988);
        break;
    case VPU_DEVICES_WAVE412:
        ret = _vpu_instance_process_msg(pack, vpu_instance_wave412);
        break;
    case VPU_DEVICES_WAVE521:
        ret = _vpu_instance_process_msg(pack, vpu_instance_wave521);
        break;
    default:
        // invalid dev id
        ret = -1;
        break;
    }

    return ret;
}