/*
 * sdpe_rpc_doip_server.c
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
#include <sdpe_rpc_doip_service.h>

#include "sdpe_rpc_doip.h"

static struct vdoip_instance g_vdoip_server;

extern int doip_data_request(struct sdpe_doip_addr *t_addr, uint8_t *t_data, uint32_t t_len);

static int vdoip_data_request_handle(void *arg, uint8_t *data, uint32_t len);

static const sdpe_rpc_service_hdl g_vdoip_server_handler[] =
{
    [VDOIP_DATA_REQ] = vdoip_data_request_handle,
};

static int vdoip_rpc_cb(void *arg, uint16_t msg_id, uint8_t *data, uint32_t len)
{
    uint8_t service_id = SDPE_RPC_GET_SERVICE_ID(msg_id);
    uint8_t sub_msg_id = SDPE_RPC_GET_MSG_ID(msg_id);

    ASSERT(service_id == SDPE_RPC_VDOIP_SERVICE);

    if (sub_msg_id > VDOIP_MSG_START && sub_msg_id < VDOIP_MSG_END) {
        SDPE_RPC_INFO("doip server recv msg:%d\n", sub_msg_id);
        if (g_vdoip_server_handler[sub_msg_id])
            return g_vdoip_server_handler[sub_msg_id](arg, data, len);
    }
    SDPE_RPC_ERR("doip server recv wrong msg:%d\n", msg_id);
    return -1;
}

static int vdoip_data_request_handle(void *arg, uint8_t *data, uint32_t len)
{
    struct vdoip_data_req_s *data_req= (struct vdoip_data_req_s *)data;

    ASSERT(data != NULL);

    return doip_data_request(&data_req->t_addr, data_req->t_data, data_req->t_len);
}

int vdoip_data_confirm(struct sdpe_doip_addr *addr, uint32_t result)
{
    struct vdoip_data_cnf_s cnf;
    sdpe_rpc_message_t message;

    memcpy(&cnf.t_addr, addr, sizeof(struct sdpe_doip_addr));
    cnf.t_result = result;

    message.msg_id = SDPE_RPC_DOIP_GET_MSGID(VDOIP_DATA_CNF);
    message.msg_type = SDPE_RPC_IND;
    message.data_num = 1;
    message.data_len[0] = sizeof(struct vdoip_data_cnf_s);
    message.data[0] = &cnf;

    return sdpe_rpc_dev_send(&g_vdoip_server.priv, &message, SDPE_RPC_WAIT_FOREVER);
}

int vdoip_data_som_ind(struct sdpe_doip_addr *addr, uint32_t t_len)
{
    struct vdoip_data_som_ind_s som_ind;
    sdpe_rpc_message_t message;

    memcpy(&som_ind.t_addr, addr, sizeof(struct sdpe_doip_addr));
    som_ind.t_len = t_len;

    message.msg_id = SDPE_RPC_DOIP_GET_MSGID(VDOIP_DATA_SOM_IND);
    message.msg_type = SDPE_RPC_IND;
    message.data_num = 1;
    message.data_len[0] = sizeof(struct vdoip_data_som_ind_s);
    message.data[0] = &som_ind;

    return sdpe_rpc_dev_send(&g_vdoip_server.priv, &message, SDPE_RPC_WAIT_FOREVER);
}

int vdoip_data_ind(struct sdpe_doip_addr *addr, uint32_t t_result,
                   uint32_t t_len, uint8_t *data)
{
    struct vdoip_data_ind_s ind;
    sdpe_rpc_message_t message;

    memcpy(&ind.t_addr, addr, sizeof(struct sdpe_doip_addr));
    ind.t_result = t_result;
    ind.t_len = t_len;

    message.msg_id = SDPE_RPC_DOIP_GET_MSGID(VDOIP_DATA_IND);
    message.msg_type = SDPE_RPC_IND;
    message.data_num = 2;
    message.data_len[0] = sizeof(struct vdoip_data_ind_s);
    message.data[0] = &ind;
    message.data_len[1] = t_len;
    message.data[1] = data;

    return sdpe_rpc_dev_send(&g_vdoip_server.priv, &message, SDPE_RPC_WAIT_FOREVER);
}

int vdoip_service_open(void)
{
    int ret;

    ret = sdpe_rpc_dev_open(&g_vdoip_server.priv);
    if (ret < 0) {
        SDPE_RPC_ERR("rpc transport open fail\n");
        return ret;
    }

    return 0;
}

int vdoip_service_close(void)
{
    return sdpe_rpc_dev_close(&g_vdoip_server.priv);
}

int vdoip_service_init(sdpe_rpc_service_cfg_t *cfg)
{
    sdpe_rpc_dev_cfg_t rpc_dev_cfg;
    int ret;

    rpc_dev_cfg.name = "sdpe-doip";
    rpc_dev_cfg.rproc = cfg->rproc;
    rpc_dev_cfg.addr = cfg->addr;
    rpc_dev_cfg.max_msg_len = SDPE_RPC_DOIP_PACKET_LEN;
    rpc_dev_cfg.support_pack = true;
    rpc_dev_cfg.support_workq = false;

    rpc_dev_cfg.type = SDPE_RPC_RPMSG_TRNASPORT;
    rpc_dev_cfg.lower_half = cfg->lower_half;

    ret = sdpe_rpc_dev_init(&g_vdoip_server->priv, &rpc_dev_cfg);
    if (ret < 0) {
        SDPE_RPC_ERR("rpc dev init fail\n");
        return ret;
    }

    return ret;
}
