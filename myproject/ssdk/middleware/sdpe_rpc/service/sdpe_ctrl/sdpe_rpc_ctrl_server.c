/*
 * sdpe_rpc_ctrl_server.c
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

#include "sdpe_rpc_ctrl.h"

static struct sdpe_ctrl_instance g_sdpe_ctrl_server;

#ifndef CONFIG_SDPE_RPC_MULTI_TASK
static sdpe_rpc_dev_cb g_sdpe_service_handler[SDPE_RPC_MAX_SERVICE];
#endif

static int sdpe_rpc_sync_handle(void *arg, uint8_t *data, uint32_t len);
static int sdpe_start_routing_handle(void *arg, uint8_t *data, uint32_t len);
static int sdpe_stop_routing_handle(void *arg, uint8_t *data, uint32_t len);
static int sdpe_monitor_event_handle(void *arg, uint8_t *data, uint32_t len);

static const sdpe_rpc_service_hdl g_sdpe_ctrl_handler[] =
{
    [SDPE_CTRL_SYNC_REQ]    = sdpe_rpc_sync_handle,
    [SDPE_CTRL_START_ROUTE] = sdpe_start_routing_handle,
    [SDPE_CTRL_STOP_ROUTE]  = sdpe_stop_routing_handle,
    [SDPE_CTRL_MONITOR]     = sdpe_monitor_event_handle,
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

static int sdpe_rpc_sync_handle(void *arg, uint8_t *data, uint32_t len)
{
    struct sdpe_rpc_dev *rpc_dev = &g_sdpe_ctrl_server.rpc_dev;
    sdpe_rpc_message_t message;

    message.msg_id = SDPE_RPC_CTRL_GET_MSGID(SDPE_CTRL_SYNC_CNF);
    message.msg_type = SDPE_RPC_IND;
    message.data_num = 0;

    return sdpe_rpc_dev_send(rpc_dev, &message, SDPE_RPC_WAIT_FOREVER);
}

static int sdpe_start_routing_handle(void *arg, uint8_t *data, uint32_t len)
{
    struct sdpe_ctrl_start_route *start_route = (struct sdpe_ctrl_start_route *)data;
    struct sdpe_ctrl_start_route_req *req = &start_route->req;
    struct sdpe_ctrl_start_route_resp *resp = &start_route->resp;

    sdpe_start_routing(req->route_table_addr, req->route_table_size);
    resp->result = 0;
    return 0;
}

static int sdpe_stop_routing_handle(void *arg, uint8_t *data, uint32_t len)
{
    struct sdpe_ctrl_stop_route *stop_route = (struct sdpe_ctrl_stop_route *)data;
    struct sdpe_ctrl_stop_route_resp *resp = &stop_route->resp;

    sdpe_stop_routing();
    resp->result = 0;
    return 0;
}

static int sdpe_monitor_event_handle(void *arg, uint8_t *data, uint32_t len)
{
    struct sdpe_ctrl_set_monitor *set_monitor = (struct sdpe_ctrl_set_monitor *)data;
    struct sdpe_ctrl_set_monitor_req *req = &set_monitor->req;
    struct sdpe_ctrl_set_monitor_resp *resp = &set_monitor->resp;

    sdpe_monitor_event(req->event_id, req->enable_flag,
                       req->arg_len, req->arg);
    resp->result = 0;
    return 0;
}

void sdpe_rpc_event_cb(uint32_t event_id, uint32_t arg)
{
    struct sdpe_rpc_dev *rpc_dev = &g_sdpe_ctrl_server.rpc_dev;
    struct sdpe_ctrl_event_cb *event_cb;
    sdpe_rpc_message_t message;

    if (!sdpe_rpc_is_ready()) {
        SDPE_RPC_ERR("sdpe rpc is not ready\n");
        return;
    }

    event_cb = SDPE_RPC_GET_SHARE_BUF(struct sdpe_ctrl_event_cb);
    event_cb->req.event_id = event_id;
    event_cb->req.arg = arg;

    message.msg_id = SDPE_RPC_CTRL_GET_MSGID(SDPE_CTRL_EVENT_CB);
    message.msg_type = SDPE_RPC_REQ;
    message.data_num = 1;
    message.data_len[0] = sizeof(uint32_t);
    message.data[0] = (uint8_t*)&event_cb;

    sdpe_rpc_dev_send(rpc_dev, &message, SDPE_RPC_WAIT_FOREVER);
}

#ifndef CONFIG_SDPE_RPC_MULTI_TASK
void *sdpe_rpc_service_cb_register(uint8_t services, void *usr_cb)
{
    g_sdpe_service_handler[services] = (sdpe_rpc_dev_cb)usr_cb;
    return &g_sdpe_ctrl_server.rpc_dev;
}
#endif

bool sdpe_rpc_is_ready(void)
{
    return g_sdpe_ctrl_server.ready;
}

int sdpe_rpc_ctrl_service_init(sdpe_rpc_service_cfg_t *cfg)
{
    sdpe_rpc_dev_cfg_t rpc_dev_cfg;
    int ret;

    rpc_dev_cfg.name = "SDPE_CTRL_SERVER";
    rpc_dev_cfg.rproc = cfg->rproc;
    rpc_dev_cfg.addr = cfg->addr;
    rpc_dev_cfg.max_msg_len = 32;
    rpc_dev_cfg.support_pack = false;
    rpc_dev_cfg.support_workq = true;

    rpc_dev_cfg.type = SDPE_RPC_MBOX_TRNASPORT;
    rpc_dev_cfg.lower_half = cfg->lower_half;

    ret = sdpe_rpc_dev_init(&g_sdpe_ctrl_server.rpc_dev, &rpc_dev_cfg);
    if (ret < 0) {
        SDPE_RPC_ERR("rpc dev init fail\n");
        return ret;
    }

    ret = sdpe_rpc_dev_open(&g_sdpe_ctrl_server.rpc_dev,
                            sdpe_ctrl_rpc_cb, NULL);
    if (ret < 0) {
        SDPE_RPC_ERR("rpc dev open fail\n");
        return ret;
    }

    g_sdpe_ctrl_server.ready = true;

    return ret;
}
