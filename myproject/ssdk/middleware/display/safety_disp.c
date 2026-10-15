/*
 * Copyright (c) 2020 Semidrive Semiconductor Inc.
 * All rights reserved.
 *
 * Description: display service implementation
 *
 */

#include "rpc/dcf_rpc.h"
#include "sdm_display.h"
#include "disp_data_type.h"
#include "display_service.h"
#include "display_share.h"
#include "rp_ioctl.h"

#include <disp_service_defs.h>
#include "mbox_cfg.h"

static struct dc_share g_dc_share = {0};

static int vsync_callback(int display_id, int64_t timestamp);

static void inline screen_cast_err(const char *err_msg)
{
    printf("cast: failed to %s\n", err_msg);
}

/* display ioc callback entry */
static rpc_call_result_t disp_svc_ioctl_cb(rpc_server_handle_t hserver, rpc_call_request_t *request)
{
    rpc_call_result_t result = {0,};
    struct disp_ioctl_cmd *ctl = (struct disp_ioctl_cmd *) &request->param[0];
    struct display_server *disp;
    unsigned int size = RP_IOC_SIZE(ctl->op);

    result.ack = request->cmd;
    if (!request) {
        result.retcode = ERR_INVALID_ARGS;
        screen_cast_err("get rpc_call_request_t");
        return result;
    }

    disp = &g_dc_share.display_server;

    osMutexAcquire(disp->dev_lock, 0);

    display_handle *handle = hal_get_display_handle(DISPLAY_ID_RPC);
    if (handle == NULL)
        return result;
    int ret =  ((disp->on_screen)++) % MAX_BUFFERS;
    disp->on_screen = ret;
    struct sdm_post_config *post = &g_dc_share.post_configs[disp->on_screen];
    if (!post)
        return result;

    switch (ctl->op) {
    case DISP_CMD_START:
        break;
    case DISP_CMD_SET_FRAMEINFO:
        if (size != sizeof(struct disp_frame_info)) {
            result.retcode = ERR_NOT_VALID;
            return result;
        }

        disp_ioctl_set_frameinfo(&g_dc_share, RP_IOC_DATA(ctl));
        #if CONFIG_DISP_THREAD
        osEventFlagsSet(disp->disp_event, DISPLAY_COMPLETE_EVENT_FLAG);
        #else
        sdm_post(handle, post);
        #endif
        break;

    case DISP_CMD_SHARING_WITH_MASK:
        if (size != sizeof(struct disp_frame_info)) {
            result.retcode = ERR_NOT_VALID;
            return result;
        }

        /* blending sharing frame and mask frame with porter-duff*/
        disp_ioctl_set_frameinfo(&g_dc_share, RP_IOC_DATA(ctl));

#if CONFIG_DISP_THREAD
        osEventFlagsSet(disp->disp_event, DISPLAY_COMPLETE_EVENT_FLAG);
#else
        post->psot_no_wait_vsync = true;
        sdm_post(handle, post);
#endif
        break;

        case DISP_CMD_CLEAR:
            g_dc_share.blend_ready = true;
            g_dc_share.skip_post = true;

            sdm_clear_display(handle);

            break;
#if CONFIG_VSYNC_THREAD
        case DISP_CMD_ENABLE_VSYNC:
            // display_handle *handle = hal_get_display_handle(0);
            ssdk_printf(0, "==== vsync_callback DISP_CMD_ENABLE_VSYNC: %d ====\n", handle->display_id);
            sdm_callback_vsync_register(handle, vsync_callback);
            break;
        case DISP_CMD_DISABLE_VSYNC:
            ssdk_printf(0, "==== vsync_callback DISP_CMD_DISABLE_VSYNC: %d ====\n", handle->display_id);
            sdm_callback_vsync_register(handle, NULL);
            break;
#endif
        default: break;
    }

    result.retcode = 0;
    osMutexRelease(disp->dev_lock);
    //ssdk_printf(0, "%s ioctl cmd=0x%x, disp->binitialized = %d\n", __func__, ctl->op, disp->binitialized);

    return result;
}


static int safe_disp_notify_agent(struct display_server *disp, u8 *data, u16 len);

#if CONFIG_VSYNC_THREAD
static int mbox_registered = 0;
static int display_vsync_id;
static int vsync_callback(int display_id, int64_t timestamp) {
    struct display_server *disp = &g_dc_share.display_server;

    if (!disp->binitialized || !mbox_registered)
        return 0;

    display_vsync_id = display_id;
    osEventFlagsSet(disp->disp_vsync_event, DISPLAY_COMPLETE_EVENT_FLAG);
    return 0;
}
#endif

static int safe_disp_notify_agent(struct display_server *disp, u8 *data, u16 len)
{
    return mbox_send(disp->mchan, data, len, 1000);
    return 0;
}

int fps(void) {
    static lk_time_t last_time;
    static lk_time_t now;
    static int fps_value = 0;
    static int frame_count = 0;

    now = current_time();
    frame_count++;
    if (now - last_time > 1000) {
        fps_value = frame_count;
        frame_count = 0;
        last_time = now;
        // ssdk_printf(0, "disp[%d] fps = %d\n", display_vsync_id,  fps_value);
    }
    return fps_value;
}

#if CONFIG_VSYNC_THREAD
static void display_vsync_thread(void *arg)
{
    int ret;
    struct display_server *disp = arg;

    while(1) {
        ret = osEventFlagsWait(disp->disp_vsync_event, DISPLAY_COMPLETE_EVENT_FLAG, osFlagsWaitAny, 4000);
        if (ret != DISPLAY_COMPLETE_EVENT_FLAG) {
            continue;
        }

        if (display_vsync_id != DISPLAY_ID_RPC) {
            continue;
        }

        uint32_t msg[2] = { display_vsync_id };
        safe_disp_notify_agent(disp, (u8*) msg, 8);
        // thread_sleep(10);
    }
}

static int safe_display_add_notifier(struct display_server *disp)
{
    if (mbox_registered)
        return 0;

    disp->mbox_addr = DISP_MAILBOX_ADDR;
    disp->rproc = MBOX_RPROC_AP2; // IPCC_RRPOC_AP1

    sdrv_mbox_chan_req_t chan_req;
    chan_req.rproc = disp->rproc;
    chan_req.dest_addr = disp->mbox_addr;
    chan_req.src_addr = disp->mbox_addr;
    disp->mchan = mbox_request_channel(g_mbox_dev, &chan_req);
    if (!disp->mchan) {
        ssdk_printf(0,"request channel failed\n");
        return -1;
    }

    disp->disp_vsync_event = osEventFlagsNew(NULL);

    osThreadAttr_t attr = {
        .name = "vsyncd",
        .stack_size = 2048,
        .priority = osPriorityRealtime,
    };
    disp->disp_vsync_thread = osThreadNew(display_vsync_thread, disp, &attr);

    mbox_registered = 1;

    return 0;
}
#endif

static rpc_server_impl_t devfuncs[] = {
    { .command = MOD_RPC_REQ_DC_IOCTL, .func = disp_svc_ioctl_cb, .flags = IPCC_RPC_NO_FLAGS },
};

void display_service_init(void)
{
    struct display_server *disp;

    disp = &g_dc_share.display_server;

    if (disp->binitialized)
        return;

    disp->dev_lock = osMutexNew(NULL);
    if (!disp->dev_lock) {
        screen_cast_err("new mutex");
        return;
    }

    osMutexAcquire(disp->dev_lock, 0);

    sdm_post_init(&g_dc_share);
#if CONFIG_VSYNC_THREAD
    safe_display_add_notifier(disp);
#endif

    osMutexRelease(disp->dev_lock);
    start_ipcc_rpc_service(devfuncs, ARRAY_SIZE(devfuncs));
    disp->binitialized = true;

    ssdk_printf(0,"**** display service init success****\n");

    return;
err_release:
    osMutexRelease(disp->dev_lock);
}

void display_service_test(void* addr)
{
    rpc_call_request_t *request = (rpc_call_request_t *)malloc(sizeof(rpc_call_request_t));
    memset(request, 0x00, sizeof(rpc_call_request_t));

    struct disp_ioctl_cmd *ctl = (struct disp_ioctl_cmd *)&request->param[0];
    memset(ctl, 0x00, sizeof(struct disp_ioctl_cmd));

    struct disp_frame_info *fi = (struct disp_frame_info *)malloc(sizeof(struct disp_frame_info));
    memset(fi, 0x00, sizeof(struct disp_frame_info));

    ctl->op = DISP_CMD_SHARING_WITH_MASK;

    fi->addr_l = (int)addr + 0x10000000;
    fi->addr_h = 0;
    fi->format = DRM_FORMAT_ABGR8888;
    fi->width = 1920;
    fi->height = 720;
    fi->pos_x = 0;
    fi->pos_y = 0;
    fi->pitch = 1920 * 32 / 8;
    fi->mask_id = 0;
    fi->cmd = 0;

    memcpy(&ctl->u.data[0], fi, sizeof(struct disp_frame_info));

    disp_svc_ioctl_cb(NULL, request);
}
