/**
 * @file fastavm_csi.c
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

#include <stdlib.h>
#include <lib/list.h>
#include "fastavm_def.h"
#include "fastavm_cfg.h"
#include "fastavm.h"
#include "app_log.h"
#include "cam_service.h"
#include <driver/memmap.h>

static void* scs_handle = NULL;

static int fastavm_csi_open(int id)
{
    APP_INFO("");
    if (!scs_handle) {
        scs_handle = scs_open(id);
    }
    return scs_handle ? 0 : -1;
}

static int fastavm_csi_close(int id)
{
    APP_INFO("");
    if (scs_handle) {
        scs_close(scs_handle);
        scs_handle = NULL;
    }
    return 0;
}

static int fastavm_csi_init(int id, uint32_t *w, uint32_t *h, uint32_t *fmt)
{
    APP_INFO("");
    if (!scs_handle || !w || !h || !fmt)
        return -1;

    struct scs_ioctl_cmd ctrl;
    struct scs_ioctl_result result;
    ctrl.op = SCS_OP_GET_FORMAT;
    ctrl.instance = (uint8_t)id;

    int ret = scs_ioctl(scs_handle, &ctrl, &result);
    if (ret)
        return -1;

    *fmt = result.msg.g_fmt.pixelformat;
    *w = result.msg.g_fmt.width;
    *h = result.msg.g_fmt.height;
    *fmt = V4L2_PIX_FMT_YUYV;
    APP_INFO("w:%d, h:%d, fmt:0x%08x, 0x%08x\n", *w, *h, result.msg.g_fmt.pixelformat, *fmt);

    ctrl.op = SCS_OP_SET_FORMAT;
    ctrl.msg.s_fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    ctrl.msg.s_fmt.width = result.msg.g_fmt.width;
    ctrl.msg.s_fmt.height = result.msg.g_fmt.height;
    ctrl.msg.s_fmt.pixelformat = *fmt;
    ret = scs_ioctl(scs_handle, &ctrl, &result);
    return ret ? -1 : 0;
}

static int fastavm_csi_set_buf(int id, uint32_t index)
{
    APP_DBG("");
    if (!scs_handle)
        return -1;

    struct scs_ioctl_cmd ctrl;
    struct scs_ioctl_result result;
    ctrl.op = SCS_OP_QBUF;
    ctrl.instance = (uint8_t)id;
    ctrl.msg.s_bufinfo.index = (uint8_t)index;
    int ret = scs_ioctl(scs_handle, &ctrl, &result);
    return ret ? -1 : 0;
}

static int fastavm_csi_config(int id, uint8_t *buf, uint32_t buf_size, uint32_t buf_cnt)
{
    APP_INFO("");
    if (!scs_handle || !buf)
        return -1;

    struct scs_ioctl_cmd ctrl;
    struct scs_ioctl_result result;
    ctrl.op = SCS_OP_QUEUE_SETUP;
    ctrl.instance = (uint8_t)id;
    ctrl.msg.queue.num_bufs = (uint8_t)buf_cnt;
    int ret = scs_ioctl(scs_handle, &ctrl, &result);
    if (ret)
        return -1;

    for (int i = 0; i < buf_cnt; i++) {
        ctrl.op = SCS_OP_SET_BUFINFO;
        ctrl.instance = (uint8_t)id;
        ctrl.msg.s_bufinfo.index = (uint8_t)i;
        ctrl.msg.s_bufinfo.addr[0] = (uint32_t)p2ap((paddr_t)(buf+buf_size*i));
        int ret = scs_ioctl(scs_handle, &ctrl, &result);
        if (ret)
            return -1;
    }
    for (uint32_t i = 0; i < buf_cnt; i++) {
        ret = fastavm_csi_set_buf(0, i);
        if (ret)
            return -1;
    }

    return 0;
}

static int fastavm_csi_get_buf(int id, uint32_t *index, uint64_t *timestamp)
{
    APP_DBG("");
    if (!scs_handle || !index)
        return -1;

    struct scs_ioctl_cmd ctrl;
    struct scs_ioctl_result result;
    ctrl.op = SCS_OP_DQBUF;
    ctrl.instance = (uint8_t)id;
    int ret = scs_ioctl(scs_handle, &ctrl, &result);
    if (ret)
        return -1;

    *index = result.msg.g_bufinfo.index;
    if (timestamp) {
        *timestamp = ((uint64_t)result.msg.g_bufinfo.timestamp_h << 32);
        *timestamp |= (uint64_t)result.msg.g_bufinfo.timestamp;
    }
    return 0;
}

static int fastavm_csi_start(int id)
{
    APP_INFO("");
    if (!scs_handle)
        return -1;

    struct scs_ioctl_cmd ctrl;
    struct scs_ioctl_result result;
    ctrl.op = SCS_OP_STREAM_ON;
    ctrl.instance = (uint8_t)id;
    int ret = scs_ioctl(scs_handle, &ctrl, &result);
    return ret ? -1 : 0;
}

static int fastavm_csi_stop(int id)
{
    APP_INFO("");
    if (!scs_handle)
        return -1;

    struct scs_ioctl_cmd ctrl;
    struct scs_ioctl_result result;
    ctrl.op = SCS_OP_STREAM_OFF;
    ctrl.instance = (uint8_t)id;
    int ret = scs_ioctl(scs_handle, &ctrl, &result);
    return ret ? -1 : 0;
}

fastavm_csi_ops_t fastavm_csi_ops = {
    .p_open = fastavm_csi_open,
    .p_close = fastavm_csi_close,
    .p_init = fastavm_csi_init,
    .p_config = fastavm_csi_config,
    .p_setbuf = fastavm_csi_set_buf,
    .p_getbuf = fastavm_csi_get_buf,
    .p_start = fastavm_csi_start,
    .p_stop = fastavm_csi_stop,
};
/* EOF */
