/*
 * Copyright (c) 2022, Semidriver Semiconductor
 */
#include <stdlib.h>
#include <debug.h>
#include <rpc/rpmsg_rpc.h>
#include <rpmsg_service.h>
#include <can_proxy.h>
#include <driver/can.h>
#include <malloc.h>
#include <board.h>
#include <rpc/dcf_rpc.h>
#include <vcan_client.h>
#include <CLI.h>

struct dcf_front_device g_can_dev[4];

static ssize_t vircan_write(struct dcf_front_device *fdev, can_frame_t *frame)
{
    int ret = 0;

    if (frame->len > CANFD_MAX_DLEN) {
        ssdk_printf(SSDK_ERR, "invalid data length=%d\n", frame->len);
        goto free_kbuf;
    }

    osMutexAcquire(fdev->ept_lock, osWaitForever);

    ret = rpmsg_channel_send(g_can_dev[0].chan, g_can_dev[0].ept_id, (char *)frame, CP_CAN_MTU, 1000);
    if (ret < 0) {
        ssdk_printf(SSDK_ERR, "%s : write error %d\n", __func__, ret);
    } else {
        ssdk_printf(SSDK_INFO, "%s : data to ept %d\n", __func__, g_can_dev[0].ept_id);
    }

    osMutexRelease(fdev->ept_lock);
    ssdk_printf(SSDK_INFO, "%s sent %d bytes\n", __func__, ret);

free_kbuf:
    return ret;
}

static int vircan_if_control(struct dcf_front_device *candev, int command,
    struct canctl_result *data)
{
    rpc_call_result_t result = {0,};
    rpc_call_request_t request;
    struct canctl_cmd *ctl = DCF_RPC_PARAM(request, struct canctl_cmd);
    struct canctl_result *rs = (struct canctl_result *) &result.result[0];
    int ret = 0;

    DCF_INIT_RPC_REQ(request, MOD_RPC_REQ_CP_IOCTL);
    ctl->op = command;
    ctl->tag = candev->srv_id;
    ret = dcf_call(DP_CR5_SAF, &request, &result, 100);
    if (ret < 0 || result.retcode < 0) {
        ssdk_printf(SSDK_EMERG, "rpcall failed=%d %d\n", ret, result.retcode);
        goto fail_call;
    }

    memcpy(data, rs, sizeof(*rs));

fail_call:
    return ret;
}

static int vircan_if_start(struct dcf_front_device *candev)
{
    struct canctl_result op_ret;

    return vircan_if_control(candev, CP_OP_START, &op_ret);
}

static int vircan_if_stop(struct dcf_front_device *candev)
{
    struct canctl_result op_ret;

    return vircan_if_control(candev, CP_OP_STOP, &op_ret);
}

static int vircan_if_getconfig(struct dcf_front_device *candev)
{
    struct canctl_result op_ret;
    int ret;

    ret = vircan_if_control(candev, CP_OP_GET_CONFIG, &op_ret);
    if (ret == 0) {
        ssdk_printf(SSDK_ERR, "baud: %d, canfd: %d, if#: %d\n",
            op_ret.msg.cfg.baudrate, op_ret.msg.cfg.canfd_support,
            op_ret.msg.cfg.instances);
    }

    return ret;
}

static int vircan_if_getversion(struct dcf_front_device *candev)
{
    struct canctl_result op_ret;
    int retry = 0;
    int ret;

    /* in case remote is not ready to complete initialization */
    do {
        ret = vircan_if_control(candev, CP_OP_GET_VERSION, &op_ret);
        if (ret < 0) {
            if (retry++ >= 10)
                return ret;

            udelay(500 * 1000);
        }
    } while (ret < 0);

    if (ret == 0)
        ssdk_printf(SSDK_ERR, "id: %d, version: %04x, vendor: %d\n",
                op_ret.msg.v.id, op_ret.msg.v.version, op_ret.msg.v.vendor);

    return ret;
}

static int vircan_if_suspend(struct dcf_front_device *candev)
{
    ssdk_printf(SSDK_INFO, "candev suspend ept:%d hook\n", candev->ept_id);

    return 0;
}
static int vircan_if_resume(struct dcf_front_device *candev)
{
    ssdk_printf(SSDK_INFO, "candev resume ept:%d hook\n", candev->ept_id);

    return 0;
}

static int find_id(const char *name)
{
    int channel_id = -1;
    int i;

    for (i = 0; i < 4; i++) {
        if (strchr(name, (i+'0'))) {
            channel_id = i;
            return channel_id;
        }
    }
    return channel_id;
}

static int vircan_create_channel(struct dcf_front_device *candev, int cmd,
    struct canctl_result *data)
{
    rpc_call_result_t result = {0,};
    rpc_call_request_t request;
    struct canctl_cmd *ctl = DCF_RPC_PARAM(request, struct canctl_cmd);
    struct canctl_result *rs = (struct canctl_result *) &result.result[0];
    int ret = 0;

    DCF_INIT_RPC_REQ(request, MOD_RPC_REQ_CP_IOCTL);
    ctl->op = cmd;
    ctl->tag = candev->srv_id;
    ctl->msg.s.endpoint = candev->srv_id;

    if (candev->can_bind != 0) {
        // cause the can if begin from 0
        ctl->msg.s.canif_bind = candev->can_bind - 1;
    } else
        ctl->msg.s.canif_bind = find_id(candev->name);

    if (ctl->msg.s.canif_bind < 0) {
        ssdk_printf(SSDK_ERR, "wrong id number%d\n", ctl->msg.s.canif_bind);
        return -1;
    }

    ret = dcf_call(DP_CR5_SAF, &request, &result, 100);
    if (ret < 0 || result.retcode < 0) {
        ssdk_printf(SSDK_ERR, "rpcall failed=%d %d\n", ret,
            result.retcode);
        return ret;
    }
    memcpy(data, rs, sizeof(*rs));
    /* TODO: wait for remote channel complete */
    udelay(100 * 1000);

    return ret;
}

static int vircan_set_filter(struct dcf_front_device *candev, int cmd,
    struct vircan_setfilter *setfilter, struct canctl_result *data)
{
    rpc_call_result_t result = {0,};
    rpc_call_request_t request;
    struct canctl_cmd *ctl = DCF_RPC_PARAM(request, struct canctl_cmd);
    struct canctl_result *rs = (struct canctl_result *) &result.result[0];
    int ret = 0;
    int i;

    DCF_INIT_RPC_REQ(request, MOD_RPC_REQ_CP_IOCTL);
    ctl->op = cmd;
    ctl->tag = candev->srv_id;
    ctl->msg.f.mode = setfilter->mode;
    ctl->msg.f.id_range_high = setfilter->id_range_high;
    ctl->msg.f.id_range_low = setfilter->id_range_low;
    ctl->msg.f.batch_id = setfilter->batch_id;
    ctl->msg.f.id_count = setfilter->id_count;
    for (i = 0; i < ctl->msg.f.id_count; i++)
        ctl->msg.f.can_id[i] = setfilter->can_id[i];

    ret = dcf_call(DP_CR5_SAF, &request, &result, 100);
    if (ret < 0 || result.retcode < 0) {
        ssdk_printf(SSDK_ERR, "rpcall failed=%d %d\n", ret,
            result.retcode);
        return ret;
    }

    memcpy(data, rs, sizeof(*rs));

    return ret;
}

static int rpmsg_common_cb(void *data, int len, unsigned long src, void *arg)
{
    cp_if_frame_t *frame = malloc(sizeof(cp_if_frame_t) + len);
    ASSERT(frame);
    frame->bus = cpifBusRPMSG;
    frame->len = len;
    memcpy(frame->data, data, len);
    CPDBG("==== RPMSG recv data %d ==== ", len);

    for (int i = 0; i < len; i++) {
        CPDBG("0x%x ", frame->data[i]);
    }

    CPDBG("\n");
    free(frame);
    return 0;
}

static int rpmsg_init(struct dcf_front_device *fdev)
{
    struct dcf_front_device *handle = malloc(sizeof(struct dcf_front_device));
    struct rpmsg_instance *dev;
    int ret = 0;

    if (!handle) {
        CPDBG("rpmsg interface malloc fail\n");
        free(handle);
        return ERR_NO_MEMORY;
    }

    handle->ept_id = fdev->ept_id;
    handle->name = fdev->name;
    handle->rproc = DP_CR5_SAF;

    /* get a rpmsg instance */
    dev = rpmsg_get_instance(RPMSG_TYPE_IPCC, handle->rproc);
    if (!dev) {
        ssdk_printf(SSDK_CRIT,
                    "rpmsg_init: failed to create an rpmsg device\n");
        return ERR_NOT_READY;
    }

    /* create a rpmsg channel */
    handle->chan = rpmsg_channel_create(dev, handle->name,
                           handle->ept_id);
    if (handle->chan == NULL) {
        ssdk_printf(SSDK_CRIT, "rpmsg_init: failed to create rpmsg channel\n");
        return ERR_NOT_VALID;
    }

    /* start the rpmsg channel */
    ret = rpmsg_channel_start(handle->chan, rpmsg_common_cb,
                              NULL);
    if (ret != 0) {
        ssdk_printf(SSDK_CRIT, "rpmsg_init: start rpmsg channel failed.\n");
        rpmsg_channel_destroy(handle->chan);
        return ERR_FAULT;
    }
    fdev->chan = handle->chan;

    return ret;
}

int vcan_init(struct dcf_front_device *fdev, int id)
{
    struct canctl_result op_ret;
    int ret;

    fdev->ept_lock = osMutexNew(NULL);

    ret = vircan_if_getversion(fdev);
    if (ret < 0) {
        ssdk_printf(SSDK_EMERG, "dev:%d no response\n", fdev->srv_id);
        return ret;
    }

    ret = vircan_create_channel(fdev, CREATE_CHANNEL, &op_ret);
    if (ret < 0) {
        ssdk_printf(SSDK_EMERG, "channel:%d create failed\n", fdev->srv_id);
        return ret;
    }

    ret = rpmsg_init(fdev);
    if (ret < 0)
        goto out;

    g_can_dev[id] = *fdev;


    vircan_if_getconfig(fdev);
    vircan_if_start(fdev);
    ssdk_printf(SSDK_EMERG, "device open ept:%d\n", fdev->ept_id);

    return 0;

out:
    ssdk_printf(SSDK_EMERG, "init ept:%d fail:%d\n", fdev->ept_id, ret);

    return ret;
}

static int can_send(int argc, char *argv[])
{
    int ret = 0;
    can_frame_t frame;

    frame.id = 1;

    for (int i = 0; i < CANFD_MAX_DLEN; i++) {
        if (i < argc)
            frame.data[i] = strtoul(argv[i], NULL, 0);
        else
            frame.data[i] = 0;
    }
    frame.len = argc;

    ret = rpmsg_channel_send(g_can_dev[0].chan, g_can_dev[0].ept_id, (char *)&frame, CP_CAN_MTU, 1000);
    if (ret < 0) {
        printf("%s : write error %d\n", __func__, ret);
    } else {
        printf("%s : data len [%d] ept %d\n", __func__, argc, g_can_dev[0].ept_id);
    }

    return CLI_FALSE;
}

CLI_CMD("cansend", "can device send", can_send);
