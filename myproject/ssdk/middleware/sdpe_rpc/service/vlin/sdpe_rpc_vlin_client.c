/*
 * sdpe_rpc_vlin_client.c
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#include <types.h>
#include <string.h>

#include <cmsis_os2.h>
#include <sdpe_rpc_device.h>
#include <sdpe_rpc_common.h>
#include <sdpe_rpc_ctrl_service.h>
#include <sdpe_rpc_vlin_service.h>

#include "sdpe_rpc_vlin.h"

static struct vlin_instance g_vlin_client;

static int vlin_rpc_cb(void *arg, uint16_t msg_id, uint8_t *data, uint32_t len)
{
    SDPE_RPC_ERR("vlin client recv wrong msg:%d\n", msg_id);
    return -1;
}

static int vlin_construct_init_msg(Lin_ConfigType *new_config,
                                   const Lin_ConfigType *config,
                                   uint8_t *config_buf)
{
    uint32_t size;
    uint8_t *addr = config_buf;

    if (config->Count > 0) {
        size = sizeof(Lin_ControllerConfigType)*config->Count;
        if (((addr - config_buf) + size) > SDPE_RPC_VLIN_CFG_LEN) {
            return -1;
        }
        SDPE_RPC_COPY_MSG_DATA(Config, size, addr);
    }
    return 0;
}

int virLin_Init(const Lin_ConfigType *config)
{
    struct sdpe_rpc_dev *rpc_dev = g_vlin_client.rpc_dev;
    struct vlin_init_s *vlin_init;
    struct vlin_init_req *req;
    struct vlin_init_resp *resp;
    sdpe_rpc_message_t message;

    if (!sdpe_rpc_is_ready()) {
        SDPE_RPC_ERR("sdpe rpc is not ready\n");
        return -1;
    }

    vlin_init = SDPE_RPC_GET_SHARE_BUF(struct vlin_init_s);

    req = &vlin_init->req;
    req->config.Count = config->Count;
    if (vlin_construct_init_msg(&req->config, config, req->config_buf) < 0) {
        SDPE_RPC_ERR("construct vlin init msg fail, no enough memory\n");
        return -1;
    }

    resp = &vlin_init->resp;
    resp->result = -1;

    message.msg_id = SDPE_RPC_VLIN_GET_MSGID(VLIN_INIT);
    message.msg_type = SDPE_RPC_REQ;
    message.data_num = 1;
    message.data_len[0] = sizeof(uint32_t);
    message.data[0] = (uint8_t*)&vlin_init;

    sdpe_rpc_dev_send(rpc_dev, &message, SDPE_RPC_WAIT_FOREVER);

    return resp->result;
}

int virLin_CheckWakeup(uint8_t channel)
{
    struct sdpe_rpc_dev *rpc_dev = g_vlin_client.rpc_dev;
    struct vlin_check_wakeup_s *check_wakeup;
    struct vlin_check_wakeup_req *req;
    struct vlin_check_wakeup_resp *resp;
    sdpe_rpc_message_t message;

    if (!sdpe_rpc_is_ready()) {
        SDPE_RPC_ERR("sdpe rpc is not ready\n");
        return -1;
    }

    check_wakeup = SDPE_RPC_GET_MULTI_SHARE_BUF(struct vlin_check_wakeup_s,
                                                SDPE_RPC_MAX_VLIN_NUM, channel);

    req = &check_wakeup->req;
    req->channel = channel;

    resp = &check_wakeup->resp;
    resp->result = -1;

    message.msg_id = SDPE_RPC_VLIN_GET_MSGID(VLIN_CHECK_WAKEUP);
    message.msg_type = SDPE_RPC_REQ;
    message.data_num = 1;
    message.data_len[0] = sizeof(uint32_t);
    message.data[0] = (uint8_t*)&check_wakeup;

    sdpe_rpc_dev_send(rpc_dev, &message, SDPE_RPC_WAIT_FOREVER);

    return resp->result;
}

int virLin_GoToSleep(uint8_t channel)
{
    struct sdpe_rpc_dev *rpc_dev = g_vlin_client.rpc_dev;
    struct vlin_goto_sleep_s *goto_sleep;
    struct vlin_goto_sleep_req *req;
    struct vlin_goto_sleep_resp *resp;
    sdpe_rpc_message_t message;

    if (!sdpe_rpc_is_ready()) {
        SDPE_RPC_ERR("sdpe rpc is not ready\n");
        return -1;
    }

    goto_sleep = SDPE_RPC_GET_MULTI_SHARE_BUF(struct vlin_goto_sleep_s,
                                              SDPE_RPC_MAX_VLIN_NUM, channel);

    req = &goto_sleep->req;
    req->channel = channel;

    resp = &goto_sleep->resp;
    resp->result = -1;

    message.msg_id = SDPE_RPC_VLIN_GET_MSGID(VLIN_GOTO_SLEEP);
    message.msg_type = SDPE_RPC_REQ;
    message.data_num = 1;
    message.data_len[0] = sizeof(uint32_t);
    message.data[0] = (uint8_t*)&goto_sleep;

    sdpe_rpc_dev_send(rpc_dev, &message, SDPE_RPC_WAIT_FOREVER);

    return resp->result;
}

int virLin_GoToSleepInternal(uint8_t channel)
{
    struct sdpe_rpc_dev *rpc_dev = g_vlin_client.rpc_dev;
    struct vlin_goto_sleep_internal_s *goto_sleep;
    struct vlin_goto_sleep_internal_req *req;
    struct vlin_goto_sleep_internal_resp *resp;
    sdpe_rpc_message_t message;

    if (!sdpe_rpc_is_ready()) {
        SDPE_RPC_ERR("sdpe rpc is not ready\n");
        return -1;
    }

    goto_sleep = SDPE_RPC_GET_MULTI_SHARE_BUF(struct vlin_goto_sleep_internal_s,
                                              SDPE_RPC_MAX_VLIN_NUM, channel);

    req = &goto_sleep->req;
    req->channel = channel;

    resp = &goto_sleep->resp;
    resp->result = -1;

    message.msg_id = SDPE_RPC_VLIN_GET_MSGID(VLIN_GOTO_SLEEP_INTERNAL);
    message.msg_type = SDPE_RPC_REQ;
    message.data_num = 1;
    message.data_len[0] = sizeof(uint32_t);
    message.data[0] = (uint8_t*)&goto_sleep;

    sdpe_rpc_dev_send(rpc_dev, &message, SDPE_RPC_WAIT_FOREVER);

    return resp->result;
}

int virLin_Wakeup(uint8_t channel)
{
    struct sdpe_rpc_dev *rpc_dev = g_vlin_client.rpc_dev;
    struct vlin_wakeup_s *wakeup;
    struct vlin_wakeup_req *req;
    struct vlin_wakeup_resp *resp;
    sdpe_rpc_message_t message;

    if (!sdpe_rpc_is_ready()) {
        SDPE_RPC_ERR("sdpe rpc is not ready\n");
        return -1;
    }

    wakeup = SDPE_RPC_GET_MULTI_SHARE_BUF(struct vlin_wakeup_s,
                                          SDPE_RPC_MAX_VLIN_NUM, channel);

    req = &wakeup->req;
    req->channel = channel;

    resp = &wakeup->resp;
    resp->result = -1;

    message.msg_id = SDPE_RPC_VLIN_GET_MSGID(VLIN_WAKEUP);
    message.msg_type = SDPE_RPC_REQ;
    message.data_num = 1;
    message.data_len[0] = sizeof(uint32_t);
    message.data[0] = (uint8_t*)&wakeup;

    sdpe_rpc_dev_send(rpc_dev, &message, SDPE_RPC_WAIT_FOREVER);

    return resp->result;
}

int virLin_WakeupInternal(uint8_t channel)
{
    struct sdpe_rpc_dev *rpc_dev = g_vlin_client.rpc_dev;
    struct vlin_wakeup_internal_s *wakeup;
    struct vlin_wakeup_internal_req *req;
    struct vlin_wakeup_internal_resp *resp;
    sdpe_rpc_message_t message;

    if (!sdpe_rpc_is_ready()) {
        SDPE_RPC_ERR("sdpe rpc is not ready\n");
        return -1;
    }

    wakeup = SDPE_RPC_GET_MULTI_SHARE_BUF(struct vlin_wakeup_internal_s,
                                          SDPE_RPC_MAX_VLIN_NUM, channel);

    req = &wakeup->req;
    req->channel = channel;

    resp = &wakeup->resp;
    resp->result = -1;

    message.msg_id = SDPE_RPC_VLIN_GET_MSGID(VLIN_WAKEUP_INTERNAL);
    message.msg_type = SDPE_RPC_REQ;
    message.data_num = 1;
    message.data_len[0] = sizeof(uint32_t);
    message.data[0] = (uint8_t*)&wakeup;

    sdpe_rpc_dev_send(rpc_dev, &message, SDPE_RPC_WAIT_FOREVER);

    return resp->result;
}

int virLin_GetStatus(uint8_t channel, uint8_t *lin_sdu)
{
    struct sdpe_rpc_dev *rpc_dev = g_vlin_client.rpc_dev;
    struct vlin_get_status_s *get_status;
    struct vlin_get_status_req *req;
    struct vlin_get_status_resp *resp;
    sdpe_rpc_message_t message;

    if (!sdpe_rpc_is_ready()) {
        SDPE_RPC_ERR("sdpe rpc is not ready\n");
        return -1;
    }

    get_status = SDPE_RPC_GET_MULTI_SHARE_BUF(struct vlin_get_status_s,
                                              SDPE_RPC_MAX_VLIN_NUM, channel);

    req = &get_status->req;
    req->channel = channel;

    resp = &get_status->resp;
    resp->result = -1;

    message.msg_id = SDPE_RPC_VLIN_GET_MSGID(VLIN_GET_STATUS);
    message.msg_type = SDPE_RPC_REQ;
    message.data_num = 1;
    message.data_len[0] = sizeof(uint32_t);
    message.data[0] = (uint8_t*)&get_status;

    sdpe_rpc_dev_send(rpc_dev, &message, SDPE_RPC_WAIT_FOREVER);

    memcpy(lin_sdu, resp->pdu, SDPE_RPC_VLIN_PDU_LEN);
    return resp->result;
}

int virLin_SendFrame(uint8_t channel,
                     const Lin_PduType *pdu)
{
    struct sdpe_rpc_dev *rpc_dev = g_vlin_client.rpc_dev;
    struct vlin_send_frame_s *send_frame;
    struct vlin_send_frame_req *req;
    struct vlin_send_frame_resp *resp;
    sdpe_rpc_message_t message;

    if (!sdpe_rpc_is_ready()) {
        SDPE_RPC_ERR("sdpe rpc is not ready\n");
        return -1;
    }

    send_frame = SDPE_RPC_GET_MULTI_SHARE_BUF(struct vlin_send_frame_s,
                                              SDPE_RPC_MAX_VLIN_NUM, channel);

    req = &send_frame->req;
    req->channel = channel;
    req->pid = pdu->Pid;
    req->cs = pdu->Cs;
    req->drc = pdu->Drc;
    req->dl = pdu->Dl;
    memcpy(req->data, pdu->SduPtr, pdu->Dl);

    resp = &send_frame->resp;
    resp->result = -1;

    message.msg_id = SDPE_RPC_VLIN_GET_MSGID(VLIN_SEND_FRAME);
    message.msg_type = SDPE_RPC_REQ;
    message.data_num = 1;
    message.data_len[0] = sizeof(uint32_t);
    message.data[0] = (uint8_t*)&send_frame;

    sdpe_rpc_dev_send(rpc_dev, &message, SDPE_RPC_WAIT_FOREVER);

    return resp->result;
}

int vlin_service_init(sdpe_rpc_service_cfg_t *cfg)
{
    int ret = 0;

#ifdef CONFIG_SDPE_RPC_MULTI_TASK
    sdpe_rpc_dev_cfg_t rpc_dev_cfg;

    rpc_dev_cfg.name = "VLIN_CLIENT";
    rpc_dev_cfg.rproc = cfg->rproc;
    rpc_dev_cfg.addr = cfg->addr;
    rpc_dev_cfg.max_msg_len = 32;
    rpc_dev_cfg.support_pack = false;
    rpc_dev_cfg.support_workq = true;

    rpc_dev_cfg.type = SDPE_RPC_MBOX_TRNASPORT;
    rpc_dev_cfg.lower_half = cfg->lower_half;

    ret = sdpe_rpc_dev_init(&g_vcan_client.priv, &rpc_dev_cfg);
    if (ret < 0) {
        SDPE_RPC_ERR("rpc dev init fail\n");
        return ret;
    }

    ret = sdpe_rpc_dev_open(&g_vcan_client.priv, vlin_rpc_cb, NULL);
    if (ret < 0) {
        SDPE_RPC_ERR("rpc dev open fail\n");
        return ret;
    }

    g_vlin_client.rpc_dev = &g_vlin_client.priv;
#else
    g_vlin_client.rpc_dev = \
        sdpe_rpc_service_cb_register(SDPE_RPC_VLIN_SERVICE, (void *)vlin_rpc_cb);
#endif

    return ret;
}
