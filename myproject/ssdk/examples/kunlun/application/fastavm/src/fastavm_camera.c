/**
 * @file fastavm_camera.c
 *
 * Copyright (c) 2022 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: sdrv fastavm application.
 *
 * Revision History:
 * -----------------
 */
#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "fastavm"

#include <driver/mbox.h>
#include <driver/kunlun/mbox/sdrv_mbox_drv.h>
#include <memmap.h>
#include <param.h>
#include <gpu_hal.h>
#include <malloc.h>
#include <mbox_cfg.h>
#include <driver/timer.h>
#include "disp_utility.h"
#include "fastavm_def.h"
#include "fastavm_cfg.h"
#include "fastavm.h"
#include "container.h"
#include "app_log.h"
#include "vdsp_utility.h"

#define YUV  (2)
#define RGB  (3)
#define RGBA (4)
#ifdef CONFIG_MP
#define VDSP_ELF_PATH "early_app/fastavm_mp/vdsp.elf"
#define MAP_TLB_PATH  "early_app/fastavm_mp/MappingTable.bin"
#else
#define VDSP_ELF_PATH "early_app/fastavm/vdsp.elf"
#define MAP_TLB_PATH  "early_app/fastavm/MappingTable.bin"
#endif

#define FASTAVM_OPS(o, f, args...) ({ \
        int ret = 0; \
        if ((o) && ((o)->f)) \
            ret = ((o)->f)(args); \
        ret; \
        })

typedef struct _vdsp_msg_t {
    paddr_t output_addr;
    paddr_t input_addr;
} vdsp_msg_t;

typedef struct _fastavm_camera_ctx {
    uint32_t disp_w;
    uint32_t disp_h;
    void* avm_input;
    void* avm_output;
    void* g2d_input;
    void* g2d_output;
    fastavm_csi_ops_t* csi_ops;
    fastavm_g2dlite_ops_t* g2d_ops;
    mbox_chan_t* vdsp_mb_chan;
    osEventFlagsId_t vdsp_sig;
    void* g2dlite_handle;
    void* vdsp_map_tbl;
} fastavm_camera_ctx;

extern fastavm_csi_ops_t fastavm_csi_ops;
extern fastavm_g2dlite_ops_t fastavm_g2dlite_ops;

static fastavm_camera_ctx camera_ctx = {
    .csi_ops = &fastavm_csi_ops,
    .g2d_ops = &fastavm_g2dlite_ops,
};

static int fastavm_camera_read_csi_frame(uint32_t* pin)
{
    if (pin) {
        uint32_t buf_idx;
        int ret = FASTAVM_OPS(camera_ctx.csi_ops, p_getbuf, 0, &buf_idx, NULL);
        if (ret < 0) {
            goto err;
        }

        *pin += buf_idx*IMG_COUNT*IMG_WIDTH*IMG_HEIGHT*2;
        ret = FASTAVM_OPS(camera_ctx.csi_ops, p_setbuf, 0, buf_idx);
        if (ret) {
            goto err;
        }

        return 0;
    }

err:
    return -1;
}

static int fastavm_camera_vdsp_wait_signal(uint32_t timeout)
{
    if (camera_ctx.vdsp_sig) {
        uint32_t sig = osEventFlagsWait(camera_ctx.vdsp_sig, 1, osFlagsWaitAny, timeout);
        APP_DBG("sig:%d", sig);
        osEventFlagsClear(camera_ctx.vdsp_sig, 1);
        if (1 == sig) {
            return 0;
        }
    }
    return -1;
}

static int fastavm_camera_vdsp_signal(void *arg, uint8_t *data, uint32_t len)
{
    if (arg) {
        osEventFlagsId_t sig = (osEventFlagsId_t)arg;
        return osOK == osEventFlagsSet(sig, 1) ? 0 : -1;
    }
    return -1;
}

static int fastavm_camera_vdsp_start_one_frame(uint32_t pin, uint8_t (*pout)[AVM_WIDTH*AVM_HEIGHT*RGB])
{
    if (camera_ctx.vdsp_mb_chan) {
        vdsp_msg_t msg;
        msg.output_addr = p2ap((uintptr_t)pout);
        msg.input_addr = p2ap((uintptr_t)pin);
        APP_DBG("input:%x, output:%x", msg.input_addr, msg.output_addr);
        int ret = mbox_send_nobuf(camera_ctx.vdsp_mb_chan, (uint8_t*)&msg, sizeof(msg), 1000);
        return ret < 0 ? -1 : 0;
    }
    return -1;
}

static int fastavm_camera_vdsp_set_mapping_table()
{
    if (camera_ctx.vdsp_mb_chan) {
        vdsp_msg_t msg;
        msg.output_addr = p2ap((paddr_t)camera_ctx.vdsp_map_tbl);
        msg.input_addr = p2ap(0x123FF);
        APP_INFO("input:%x, output:%x", msg.input_addr, msg.output_addr);
        int ret = mbox_send_nobuf(camera_ctx.vdsp_mb_chan, (uint8_t*)&msg, sizeof(msg), 1000);
        APP_INFO("send vdsp mapping table %s", !ret ? "success" : "fail");
        if (!ret) {
            ret = fastavm_camera_vdsp_wait_signal(3000);
            APP_INFO("vdsp mapping table load %s", !ret ? "success" : "fail");
        }
        return ret ? -1 : 0;
    }
    return -1;
}

static int fastavm_camera_g2dlite_blend(uint32_t avm_in, uint8_t* avm_out, unsigned long* g2d_blend_out)
{
    static int index = 0;
    if (camera_ctx.g2dlite_handle) {
        struct g2dlite_input* g2d_in = (struct g2dlite_input*)camera_ctx.g2d_input;
        uint8_t (*g2d_out)[camera_ctx.disp_w*camera_ctx.disp_h*RGB] = (uint8_t (*)[camera_ctx.disp_w*camera_ctx.disp_h*RGB])camera_ctx.g2d_output;
        uint32_t primary_cam_index = fastavm_get_primary_cam();
        uint32_t primary_cam_offset = CAM_WIDTH*CAM_HEIGHT*YUV*primary_cam_index;
        uint32_t cam_w = CAM_WIDTH;
        uint32_t cam_h = MAX((CAM_HEIGHT-camera_ctx.disp_h)/2, 0);
        uint32_t cam_w_offset = MAX(((CAM_WIDTH-(camera_ctx.disp_w-AVM_WIDTH))/2), 0)*YUV;
        uint32_t avm_w = AVM_WIDTH;
        uint32_t avm_h = MAX((AVM_HEIGHT-camera_ctx.disp_h)/2, 0);
        g2d_in->layer[0].addr[0] = (unsigned long)(avm_in+primary_cam_offset+cam_w*cam_h*YUV+cam_w_offset);
        g2d_in->layer[1].addr[0] = (unsigned long)(((uint8_t*)avm_out)+avm_w*avm_h*RGB);
        g2d_in->output.addr[0] = (unsigned long)(g2d_out+index);
        hal_g2dlite_blend(camera_ctx.g2dlite_handle, g2d_in);
        if (g2d_blend_out) *g2d_blend_out = (unsigned long)(g2d_out+index);
        index++;
        index%=3;
        return 0;
    }
    return -1;
}

static int fastavm_camera_csi_init()
{
    fastavm_csi_ops_t* ops = camera_ctx.csi_ops;
    int ret = 0;
    uint32_t cam_w;
    uint32_t cam_h;
    uint32_t cam_fmt;

    /* data init */
    camera_ctx.avm_input = malloc(IMG_COUNT*CAMERA_MAX_BUF*IMG_WIDTH*IMG_HEIGHT*YUV);
    camera_ctx.avm_output = memalign(0x1000, AVM_WIDTH*AVM_HEIGHT*RGB);
    camera_ctx.g2d_input = (void*)malloc(sizeof(struct g2dlite_input));
    camera_ctx.g2d_output = memalign(0x1000, 3*camera_ctx.disp_w*camera_ctx.disp_h*RGB);
    if (!(camera_ctx.avm_input && camera_ctx.avm_output && camera_ctx.g2d_input && camera_ctx.g2d_output)) {
        goto err;
    }
    memset(camera_ctx.g2d_input, 0, sizeof(struct g2dlite_input));
    //APP_LOG("camera_ctx.avm_input:%p", camera_ctx.avm_input);

    /* csi init */
    if (!ret) {
        ret |= FASTAVM_OPS(ops, p_open, 0);
    }
    if (!ret) {
        ret |= FASTAVM_OPS(ops, p_init, 0 , &cam_w, &cam_h, &cam_fmt);
    }
    if (!ret) {
        //memset(camera_ctx.avm_input, 0, ROUNDUP(IMG_COUNT*CAMERA_MAX_BUF*IMG_WIDTH*IMG_HEIGHT*YUV, 0x1000));
        ret |= FASTAVM_OPS(ops, p_config, 0, (uint8_t*)camera_ctx.avm_input, IMG_COUNT*IMG_WIDTH*IMG_HEIGHT*YUV, CAMERA_MAX_BUF);
    }
#if 0 /* stream on in entry not here */
    if (!ret) {
        ret |= FASTAVM_OPS(ops, p_start, 0);
    }
#endif
    if (ret) {
        goto err;
    }
    return 0;

err:
    return -1;
}

static void fastavm_camera_csi_deinit()
{
    /* data free */
    if (camera_ctx.avm_input) {
        free(camera_ctx.avm_input);
        camera_ctx.avm_input = NULL;
    }
    if (camera_ctx.avm_output) {
        free(camera_ctx.avm_output);
        camera_ctx.avm_output = NULL;
    }
    if (camera_ctx.g2d_input) {
        free(camera_ctx.g2d_input);
        camera_ctx.g2d_input = NULL;
    }
    if (camera_ctx.g2d_output) {
        free(camera_ctx.g2d_output);
        camera_ctx.g2d_output = NULL;
    }

    fastavm_csi_ops_t* ops = camera_ctx.csi_ops;
    FASTAVM_OPS(ops, p_stop, 0);
    FASTAVM_OPS(ops, p_close, 0);
}

static int fastavm_camera_g2dlite_init()
{
    fastavm_g2dlite_ops_t* ops = camera_ctx.g2d_ops;
    int ret = 0;

    hal_g2dlite_init(&camera_ctx.g2dlite_handle);
    if (!camera_ctx.g2dlite_handle) {
        goto err;
    }

    ret = FASTAVM_OPS(ops, p_inputInit, camera_ctx.g2d_input, camera_ctx.disp_w, camera_ctx.disp_h);
    if (ret) {
        goto err;
    }
    return 0;

err:
    return -1;
}

static int fastavm_camera_vdsp_init()
{
    int ret = 0;
    if (!ret) {
        ret |= utils_vdsp_stall();
    }
    if (!ret) {
        ret |= utils_vdsp_core_reset();
    }
    if (!ret) {
        ret |= utils_vdsp_load_mapping_tbl(MAP_TLB_PATH, &camera_ctx.vdsp_map_tbl);
    }
    if (!ret) {
        ret |= utils_vdsp_load_firmware(VDSP_ELF_PATH);
    }
    if (!ret) {
        ret |= utils_vdsp_go();
    }
    if (!ret) {
        sdrv_mbox_chan_req_t ch_req;
        ch_req.rproc = MBOX_RPROC_VDSP;
        ch_req.src_addr = SDRV_MBOX_ANY_ADDR;
        ch_req.dest_addr = MBOX_ADDR_LEGACY;
        camera_ctx.vdsp_mb_chan = mbox_request_channel(g_mbox_dev, (void*)&ch_req);
        //APP_LOG("camera_ctx.vdsp_mb_chan:%p, callback:%p", camera_ctx.vdsp_mb_chan, fastavm_camera_vdsp_signal);
        if (camera_ctx.vdsp_mb_chan) {
            camera_ctx.vdsp_sig = osEventFlagsNew(NULL);
            osEventFlagsClear(camera_ctx.vdsp_sig, 1);
            mbox_set_callback(camera_ctx.vdsp_mb_chan, fastavm_camera_vdsp_signal, (void*)camera_ctx.vdsp_sig);
        }
        ret = !camera_ctx.vdsp_mb_chan ? -1 : 0;
    }
    if (ret) {
        goto err;
    }
    return 0;

err:
    return -1;
}

static int fastavm_camera_init(fastavm_task_token_t* task)
{
    APP_INFO("");
    if (task) {
        sdm_display_t* disp = arb_get_slave_handle(arb_magic_video, INFOTAINMENT);
        if (!disp) {
            goto err;
        }

        utils_get_sdm_disp_rect(disp, &camera_ctx.disp_w, &camera_ctx.disp_h);
        int ret = fastavm_camera_csi_init();
        if (ret) {
            goto err;
        }

        ret = fastavm_camera_g2dlite_init();
        if (ret) {
            goto err;
        }

        ret = fastavm_camera_vdsp_init();
        if (ret) {
            goto err;
        }

        return 0;
    }

err:
    return -1;
}

static void fastavm_camera_entry(fastavm_task_token_t* task)
{
    APP_INFO("");
    int cnt = 0;
    int ret = 0;
    uint32_t pin = 0;
    uint8_t (*pout)[AVM_WIDTH*AVM_HEIGHT*RGB] = (uint8_t (*)[AVM_WIDTH*AVM_HEIGHT*RGB])camera_ctx.avm_output;
    unsigned long g2d_out;

    ret = fastavm_camera_vdsp_set_mapping_table();
    if (ret) {
        APP_ERR("vdsp set mappling table fail");
        return;
    }

    ret = FASTAVM_OPS(camera_ctx.csi_ops, p_start, 0);
    if (ret) {
        return;
    }

    while (task && task->running) {
        cnt = 0;
        ret = 0;
        do {
            pin = (uintptr_t)camera_ctx.avm_input;
            ret = fastavm_camera_read_csi_frame(&pin);
        } while (cnt++ < 25 && ret);
        if (ret) {
            APP_ERR("read csi timeout");
            break;
        }

        //APP_LOG("timestamp:%lld", timer_get_global_time(g_syscnt_timer));
        ret = fastavm_camera_vdsp_start_one_frame(pin, pout);
        if (ret) {
            APP_ERR("vdsp start one frame fail");
            continue;
        }

        ret = fastavm_camera_vdsp_wait_signal(1000);
        if (ret) {
            APP_ERR("wait vdsp signal timeout");
            break;
        }

        ret = fastavm_camera_g2dlite_blend(pin, (uint8_t*)pout, &g2d_out);
        if (ret) {
            break;
        }

        fastavm_vstream_t vstream = {
            .yuyv_addr = g2d_out,
            .yuyv_stride = camera_ctx.disp_w*RGB,
            .disp_w = camera_ctx.disp_w,
            .disp_h = camera_ctx.disp_h,
        };
        issue_token_send_sync(task->issue, &vstream, &task->running);
    }
}

static void fastavm_camera_exit(fastavm_task_token_t* task)
{
    APP_INFO("");
    if (task) {
        fastavm_camera_csi_deinit();
        osEventFlagsSet(task->terminate_sig, FASTAVM_MASK(task->id));
        msg_t msg = { .msgid = fastavm_evt_stop, .param = task->id };
        fastavm_set_evt(&msg);
    }
    osThreadExit();
}

void fastavm_camera_task(void* args)
{
    if (args) {
        fastavm_task_token_t* task = (fastavm_task_token_t*)args;
        if (!fastavm_camera_init(task)) {
            fastavm_camera_entry(task);
        }
        fastavm_camera_exit(task);
    }
}
/* EOF */
