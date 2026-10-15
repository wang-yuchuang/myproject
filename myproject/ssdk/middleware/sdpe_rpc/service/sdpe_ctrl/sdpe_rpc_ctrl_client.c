/*
 * sdpe_rpc_ctrl_client.c
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

#include <sdpe_rpc_device.h>
#include <sdpe_rpc_common.h>
#include <sdpe_rpc_service.h>
#include <sdpe_rpc_ctrl_service.h>
#include <sdpe_ctrl.h>

#include "sdpe_rpc_ctrl.h"

static struct sdpe_ctrl_instance g_sdpe_ctrl_client;

#ifndef CONFIG_SDPE_RPC_MULTI_TASK
static sdpe_rpc_dev_cb g_sdpe_service_handler[SDPE_RPC_MAX_SERVICE];
#endif

static int sdpe_rpc_sync_cnf_handle(void *arg, uint8_t *data, uint32_t len);
static int sdpe_rpc_event_cb_handle(void *arg, uint8_t *data, uint32_t len);

static const sdpe_rpc_service_hdl g_sdpe_ctrl_handler[] =
{
    [SDPE_CTRL_SYNC_CNF] = sdpe_rpc_sync_cnf_handle,
    [SDPE_CTRL_EVENT_CB] = sdpe_rpc_event_cb_handle,
};

static int sdpe_ctrl_rpc_cb(void *arg, uint16_t msg_id, uint8_t *data, uint32_t len)
{
    uint8_t *message_data = (uint8_t *)(*(uint32_t*)data);
    uint8_t service_id = SDPE_RPC_GET_SERVICE_ID(msg_id);
    uint8_t sub_msg_id = SDPE_RPC_GET_MSG_ID(msg_id);

    ASSERT(service_id < SDPE_RPC_MAX_SERVICE);

#ifndef CONFIG_SDPE_RPC_MULTI_TASK
    if (service_id != SDPE_RPC_SDPE_SERVICE) {
        sdpe_rpc_dev_cb cb = g_sdpe_service_handler[service_id];
        if (cb) {
            return cb(arg, msg_id, data, len);
        }
    }
#endif

    if (sub_msg_id > SDPE_CTRL_MSG_START && sub_msg_id < SDPE_CTRL_MSG_END) {
        if (g_sdpe_ctrl_handler[sub_msg_id])
            return g_sdpe_ctrl_handler[sub_msg_id](arg, message_data, len);
    }
    SDPE_RPC_ERR("sdpe ctrl recv wrong msg:%d\n", msg_id);
    return -1;
}

static int sdpe_rpc_sync_cnf_handle(void *arg, uint8_t *data, uint32_t len)
{
    g_sdpe_ctrl_client.ready = true;
    return 0;
}

static int sdpe_rpc_event_cb_handle(void *arg, uint8_t *data, uint32_t len)
{
    struct sdpe_ctrl_event_cb *event_cb = (struct sdpe_ctrl_event_cb *)data;
    struct sdpe_ctrl_event_cb_req *req = &event_cb->req;
    struct sdpe_ctrl_event_cb_resp *resp = &event_cb->resp;

    sdpe_rpc_event_cb(req->event_id, req->arg);
    resp->result = 0;
    return 0;
}

void sdpe_rpc_sync(void)
{
    struct sdpe_rpc_dev *rpc_dev = &g_sdpe_ctrl_client.rpc_dev;
    sdpe_rpc_message_t message;

    message.msg_id = SDPE_RPC_CTRL_GET_MSGID(SDPE_CTRL_SYNC_REQ);
    message.msg_type = SDPE_RPC_IND;
    message.data_num = 0;

    while (!g_sdpe_ctrl_client.ready) {

        sdpe_rpc_dev_send(rpc_dev, &message, SDPE_RPC_WAIT_FOREVER);

        osDelay(1);
    }
}

void sdpe_rpc_start_routing(uint32_t route_table, uint32_t size)
{
    struct sdpe_rpc_dev *rpc_dev = &g_sdpe_ctrl_client.rpc_dev;
    struct sdpe_ctrl_start_route *start_route;
    sdpe_rpc_message_t message;

    if (!sdpe_rpc_is_ready()) {
        SDPE_RPC_ERR("sdpe rpc is not ready\n");
        return;
    }

    start_route = SDPE_RPC_GET_SHARE_BUF(struct sdpe_ctrl_start_route);
    start_route->req.route_table_addr = route_table;
    start_route->req.route_table_size = size;

    message.msg_id = SDPE_RPC_CTRL_GET_MSGID(SDPE_CTRL_START_ROUTE);
    message.msg_type = SDPE_RPC_REQ;
    message.data_num = 1;
    message.data_len[0] = sizeof(uint32_t);
    message.data[0] = (uint8_t*)&start_route;

    sdpe_rpc_dev_send(rpc_dev, &message, SDPE_RPC_WAIT_FOREVER);
}

void sdpe_rpc_stop_routing(void)
{
    struct sdpe_rpc_dev *rpc_dev = &g_sdpe_ctrl_client.rpc_dev;
    struct sdpe_ctrl_stop_route *stop_route;
    sdpe_rpc_message_t message;

    if (!sdpe_rpc_is_ready()) {
        SDPE_RPC_ERR("sdpe rpc is not ready\n");
        return;
    }

    stop_route = SDPE_RPC_GET_SHARE_BUF(struct sdpe_ctrl_stop_route);

    message.msg_id = SDPE_RPC_CTRL_GET_MSGID(SDPE_CTRL_STOP_ROUTE);
    message.msg_type = SDPE_RPC_REQ;
    message.data_num = 1;
    message.data_len[0] = sizeof(uint32_t);
    message.data[0] = (uint8_t*)&stop_route;

    sdpe_rpc_dev_send(rpc_dev, &message, SDPE_RPC_WAIT_FOREVER);
}

void sdpe_rpc_monitor_event(uint32_t event_id, uint8_t enable,
                            uint32_t arg_len, uint8_t *arg)
{
    struct sdpe_rpc_dev *rpc_dev = &g_sdpe_ctrl_client.rpc_dev;
    struct sdpe_ctrl_set_monitor *set_monitor;
    struct sdpe_ctrl_set_monitor_req *req;
    sdpe_rpc_message_t message;

    if (!sdpe_rpc_is_ready()) {
        SDPE_RPC_ERR("sdpe rpc is not ready\n");
        return;
    }

    ASSERT(arg_len <= SDPE_RPC_EVENT_ARG_LEN);

    set_monitor = SDPE_RPC_GET_SHARE_BUF(struct sdpe_ctrl_set_monitor);
    req = &set_monitor->req;
    req->event_id = event_id;
    req->enable_flag = enable;
    req->arg_len = arg_len;
    memcpy(req->arg, arg, arg_len);

    message.msg_id = SDPE_RPC_CTRL_GET_MSGID(SDPE_CTRL_MONITOR);
    message.msg_type = SDPE_RPC_REQ;
    message.data_num = 1;
    message.data_len[0] = sizeof(uint32_t);
    message.data[0] = (uint8_t*)&set_monitor;

    sdpe_rpc_dev_send(rpc_dev, &message, SDPE_RPC_WAIT_FOREVER);
}

#ifndef CONFIG_SDPE_RPC_MULTI_TASK
void *sdpe_rpc_service_cb_register(uint8_t services, void *usr_cb)
{
    g_sdpe_service_handler[services] = (sdpe_rpc_dev_cb)usr_cb;
    return &g_sdpe_ctrl_client.rpc_dev;
}
#endif

bool sdpe_rpc_is_ready(void)
{
    return g_sdpe_ctrl_client.ready;
}

int sdpe_rpc_ctrl_service_init(sdpe_rpc_service_cfg_t *cfg)
{
    sdpe_rpc_dev_cfg_t rpc_dev_cfg;
    int ret;

    rpc_dev_cfg.name = "SDPE_CTRL_CLIENT";
    rpc_dev_cfg.rproc = cfg->rproc;
    rpc_dev_cfg.addr = cfg->addr;
    rpc_dev_cfg.max_msg_len = 32;
    rpc_dev_cfg.support_pack = false;
    rpc_dev_cfg.support_workq = true;

    rpc_dev_cfg.type = SDPE_RPC_MBOX_TRNASPORT;
    rpc_dev_cfg.lower_half = cfg->lower_half;

    ret = sdpe_rpc_dev_init(&g_sdpe_ctrl_client.rpc_dev, &rpc_dev_cfg);
    if (ret < 0) {
        SDPE_RPC_ERR("rpc dev init fail\n");
        return ret;
    }

    ret = sdpe_rpc_dev_open(&g_sdpe_ctrl_client.rpc_dev,
                            sdpe_ctrl_rpc_cb, NULL);
    if (ret < 0) {
        SDPE_RPC_ERR("rpc dev open fail\n");
        return ret;
    }

    return ret;
}
