/*
 * sdpe_rpc_eth_server.c
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
#include <sdpe_rpc_eth_service.h>

#include "sdpe_rpc_eth.h"

static struct veth_instance g_veth_server[SDPE_RPC_MAX_ETH_NUM];

static int veth_send_frame_handle(void *arg, uint8_t *data, uint32_t len);

static const sdpe_rpc_service_hdl g_veth_server_handler[] =
{
    [VETH_SEND_FRAME] = veth_send_frame_handle,
};

static int veth_rpc_cb(void *arg, uint16_t msg_id, uint8_t *data, uint32_t len)
{
    struct veth_instance *eth_server = (struct veth_instance *)arg;
    uint8_t service_id = SDPE_RPC_GET_SERVICE_ID(msg_id);
    uint8_t sub_msg_id = SDPE_RPC_GET_MSG_ID(msg_id);

    ASSERT(eth_server != NULL);
    ASSERT(service_id == eth_server->service_id);

    if (sub_msg_id > VETH_MSG_START && sub_msg_id < VETH_MSG_END) {
        SDPE_RPC_INFO("eth server recv msg:%d\n", sub_msg_id);
        if (g_veth_server_handler[sub_msg_id])
            return g_veth_server_handler[sub_msg_id](arg, data, len);
    }
    SDPE_RPC_ERR("eth server recv wrong msg:%d\n", msg_id);
    return -1;
}

static int veth_send_frame_handle(void *arg, uint8_t *data, uint32_t len)
{
    struct veth_instance *eth_server = (struct veth_instance *)arg;
    int ret = 0;

    ASSERT(eth_server != NULL);

    if (eth_server->usr_cb) {
        ret = eth_server->usr_cb(eth_server->arg, data, len);
    }

    return ret;
}

static int veth_get_eth_id(int eth_service_id)
{
    ASSERT(eth_service_id >= SDPE_RPC_VETH1_SERVICE);
    return eth_service_id - SDPE_RPC_VETH1_SERVICE;
}

int veth_send_frame(uint8_t eth_id, uint8_t *data, uint32_t len)
{
    struct veth_instance *eth_server;
    sdpe_rpc_message_t message;

    ASSERT(eth_id < SDPE_RPC_MAX_ETH_NUM);

    eth_server = &g_veth_server[eth_id];

    message.msg_id = SDPE_RPC_CREATE_MSGID(veth_get_eth_service_id(eth_id),
                                           VETH_SEND_FRAME);
    message.msg_type = SDPE_RPC_IND;
    message.data_num = 1;
    message.data_len[0] = len;
    message.data[0] = data;

    return sdpe_rpc_dev_send(&eth_server->priv, &message, SDPE_RPC_WAIT_FOREVER);
}

int veth_service_open(uint8_t eth_id, veth_usr_cb usr_cb, void *arg)
{
    struct veth_instance *eth_server;
    int ret;

    ASSERT(eth_id < SDPE_RPC_MAX_ETH_NUM);

    eth_server = &g_veth_server[eth_id];
    ret = sdpe_rpc_dev_open(&eth_server->priv);
    if (ret < 0) {
        SDPE_RPC_ERR("rpc transport open fail\n");
        return ret;
    }

    eth_server->usr_cb = usr_cb;
    eth_server->arg = arg;

    return 0;
}

int veth_service_close(uint8_t eth_id)
{
    struct veth_instance *eth_server;

    ASSERT(eth_id < SDPE_RPC_MAX_ETH_NUM);

    eth_server = &g_veth_server[eth_id];
    return sdpe_rpc_dev_close(&eth_server->priv);
}

int veth_service_init(sdpe_rpc_service_cfg_t *cfg)
{
    sdpe_rpc_dev_cfg_t rpc_dev_cfg;
    struct veth_instance *eth_server;
    char name[32];
    int eth_id;
    int ret;

    eth_id = veth_get_eth_id(cfg->service);
    if (eth_id >= SDPE_RPC_MAX_ETH_NUM) {
        SDPE_RPC_ERR("error eth_id:%d\n", eth_id);
        return -1;
    }

    eth_server = &g_veth_server[eth_id];

    sprintf(name, "sdpe-eth%d", eth_id);
    rpc_dev_cfg.name = name;
    rpc_dev_cfg.rproc = cfg->rproc;
    rpc_dev_cfg.addr = cfg->addr;
    rpc_dev_cfg.max_msg_len = SDPE_RPC_ETH_PACKET_LEN;
    rpc_dev_cfg.support_pack = true;
    rpc_dev_cfg.support_workq = false;

    rpc_dev_cfg.type = SDPE_RPC_RPMSG_TRNASPORT;
    rpc_dev_cfg.lower_half = cfg->lower_half;

    ret = sdpe_rpc_dev_init(&eth_server->priv, &rpc_dev_cfg);
    if (ret < 0) {
        SDPE_RPC_ERR("rpc dev init fail\n");
        return ret;
    }

    eth_server->service_id = cfg->service;

    return ret;
}
