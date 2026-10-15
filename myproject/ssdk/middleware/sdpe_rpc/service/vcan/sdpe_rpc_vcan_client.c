/*
 * sdpe_rpc_vcan_client.c
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
#include <sdpe_rpc_vcan_service.h>

#include "sdpe_rpc_vcan.h"

static struct vcan_instance g_vcan_client;

static int vcan_busoff_handle(void *arg, uint8_t *data, uint32_t len);
static int vcan_set_wakeup_event_handle(void *arg, uint8_t *data, uint32_t len);
static int vcan_rx_indication_handle(void *arg, uint8_t *data, uint32_t len);
static int vcan_tx_confirmation_handle(void *arg, uint8_t *data, uint32_t len);
static int vcan_ctrl_wakeup_handle(void *arg, uint8_t *data, uint32_t len);

static const sdpe_rpc_service_hdl g_vcan_client_handler[] =
{
    [VCAN_BUSOFF]             = vcan_busoff_handle,
    [VCAN_SET_WAKEUP_EVENT]   = vcan_set_wakeup_event_handle,
    [VCAN_RX_INDICATIOPN]     = vcan_rx_indication_handle,
    [VCAN_TX_CONFIRMATION]    = vcan_tx_confirmation_handle,
    [VCAN_CTRL_WAKEUP]        = vcan_ctrl_wakeup_handle,
};

static osMutexId_t vcan_write_mutex[SDPE_RPC_MAX_HTH_NUM];

static void vcan_mutex_init(void)
{
    for (uint32_t i = 0; i < SDPE_RPC_MAX_HTH_NUM; i++) {
        vcan_write_mutex[i] = osMutexNew(NULL);
    }
}

static int vcan_busoff_handle(void *arg, uint8_t *data, uint32_t len)
{
    struct vcan_busoff_s *busoff = (struct vcan_busoff_s *)data;
    struct vcan_busoff_req *req = &busoff->req;
    struct vcan_busoff_resp *resp = &busoff->resp;

    virCan_ControllerBusOff(req->controller);
    resp->result = 0;
    return 0;
}

static int vcan_set_wakeup_event_handle(void *arg, uint8_t *data, uint32_t len)
{
    struct vcan_set_wakeup_event_s *set_wakeup = (struct vcan_set_wakeup_event_s *)data;
    struct vcan_set_wakeup_event_req *req = &set_wakeup->req;
    struct vcan_set_wakeup_event_resp *resp = &set_wakeup->resp;

    virCan_SetWakeupEvent(req->controller);
    resp->result = 0;
    return 0;
}

static int vcan_rx_indication_handle(void *arg, uint8_t *data, uint32_t len)
{
    struct vcan_rx_indicate_s *rx_ind = (struct vcan_rx_indicate_s *)data;
    struct vcan_rx_indicate_req *req = &rx_ind->req;
    struct vcan_rx_indicate_resp *resp = &rx_ind->resp;

    virCan_RxIndication(req->hrh, req->id, req->dlc, req->data);
    resp->result = 0;
    return 0;
}

static int vcan_tx_confirmation_handle(void *arg, uint8_t *data, uint32_t len)
{
    struct vcan_tx_confirmation_s *tx_cnf = (struct vcan_tx_confirmation_s *)data;
    struct vcan_tx_confirmation_req *req = &tx_cnf->req;
    struct vcan_tx_confirmation_resp *resp = &tx_cnf->resp;

    virCan_TxConfirmation(req->pdu_id);
    resp->result = 0;
    return 0;
}

static int vcan_ctrl_wakeup_handle(void *arg, uint8_t *data, uint32_t len)
{
    struct vcan_ctrl_wakeup_s *ctrl_wakeup = (struct vcan_ctrl_wakeup_s *)data;
    struct vcan_ctrl_wakeup_req *req = &ctrl_wakeup->req;
    struct vcan_ctrl_wakeup_resp *resp = &ctrl_wakeup->resp;

    virCan_ControllerWakeUp(req->controller);
    resp->result = 0;
    return 0;
}

static int vcan_rpc_cb(void *arg, uint16_t msg_id, uint8_t *data, uint32_t len)
{
    uint8_t *message_data = (uint8_t *)(*(uint32_t*)data);
    uint8_t service_id = SDPE_RPC_GET_SERVICE_ID(msg_id);
    uint8_t sub_msg_id = SDPE_RPC_GET_MSG_ID(msg_id);

    ASSERT(service_id == SDPE_RPC_VCAN_SERVICE);

    if (sub_msg_id > VCAN_MSG_START && sub_msg_id < VCAN_MSG_END) {
        SDPE_RPC_INFO("vcan client recv msg:%d\n", sub_msg_id);
        if (g_vcan_client_handler[sub_msg_id])
            return g_vcan_client_handler[sub_msg_id](arg, message_data, len);
    }
    SDPE_RPC_ERR("vcan client recv wrong msg:%d\n", msg_id);
    return -1;
}

static int vcan_construct_init_msg(Can_ConfigType *new_config,
                                   const Can_ConfigType *config,
                                   uint8_t *config_buf)
{
    uint32_t size;
    uint8_t *addr = config_buf;

    if (config->controllerCount > 0) {
        size = sizeof(Can_ControllerConfig)*config->controllerCount;
        if (((addr - config_buf) + size) > SDPE_RPC_VCAN_CFG_LEN) {
            return -1;
        }
        SDPE_RPC_COPY_MSG_DATA(ctrllerCfg, size, addr);
    }

    if (config->rxCount > 0) {
        size = sizeof(Can_MBConfig)*config->rxCount;
        if (((addr - config_buf) + size) > SDPE_RPC_VCAN_CFG_LEN) {
            return -1;
        }
        SDPE_RPC_COPY_MSG_DATA(rxMBCfg, size, addr);
    }

    if (config->txCount > 0) {
        size = sizeof(Can_MBConfig)*config->txCount;
        if (((addr - config_buf) + size) > SDPE_RPC_VCAN_CFG_LEN) {
            return -1;
        }
        SDPE_RPC_COPY_MSG_DATA(txMBCfg, size, addr);
    }

    if (config->baudRateCfgCount > 0) {
        size = sizeof(Can_BaudRateConfig)*config->baudRateCfgCount;
        if (((addr - config_buf) + size) > SDPE_RPC_VCAN_CFG_LEN) {
            return -1;
        }
        SDPE_RPC_COPY_MSG_DATA(baudRateCfg, size, addr);
    }

    if (config->rxFifoCount > 0) {
        size = sizeof(Can_RxFIFOConfig)*config->rxFifoCount;
        if (((addr - config_buf) + size) > SDPE_RPC_VCAN_CFG_LEN) {
            return -1;
        }
        SDPE_RPC_COPY_MSG_DATA(rxFIFOCfg, size, addr);

        if (config->rxFIFOCfg->flexcanRxFIFOCfg.idFilterNum > 0) {
            size = sizeof(flexcan_rx_fifo_filter_table_t)*config->rxFIFOCfg->flexcanRxFIFOCfg.idFilterNum;
            if (((addr - config_buf) + size) > SDPE_RPC_VCAN_CFG_LEN) {
                return -1;
            }
            SDPE_RPC_COPY_MSG_DATA(rxFIFOCfg->flexcanRxFIFOCfg.filter_tab, size, addr);
        }
    }

    return 0;
}

void virCan_Init(const Can_ConfigType *config)
{
    struct sdpe_rpc_dev *rpc_dev = g_vcan_client.rpc_dev;
    struct vcan_init_s *vcan_init;
    struct vcan_init_req *req;
    sdpe_rpc_message_t message;

    if (!sdpe_rpc_is_ready()) {
        SDPE_RPC_ERR("sdpe rpc is not ready\n");
        return;
    }

    vcan_init = SDPE_RPC_GET_SHARE_BUF(struct vcan_init_s);

    req = &vcan_init->req;
    memcpy(&req->config, config, sizeof(Can_ConfigType));
    if (vcan_construct_init_msg(&req->config, config, req->config_buf) < 0) {
        SDPE_RPC_ERR("construct vcan init msg fail, no enough memory\n");
        return;
    }

    message.msg_id = SDPE_RPC_VCAN_GET_MSGID(VCAN_INIT);
    message.msg_type = SDPE_RPC_REQ;
    message.data_num = 1;
    message.data_len[0] = sizeof(uint32_t);
    message.data[0] = (uint8_t*)&vcan_init;

    sdpe_rpc_dev_send(rpc_dev, &message, SDPE_RPC_WAIT_FOREVER);
}

void virCan_DeInit(void)
{
    struct sdpe_rpc_dev *rpc_dev = g_vcan_client.rpc_dev;
    struct vcan_deinit_s *vcan_deinit;
    sdpe_rpc_message_t message;

    if (!sdpe_rpc_is_ready()) {
        SDPE_RPC_ERR("sdpe rpc is not ready\n");
        return;
    }

    vcan_deinit = SDPE_RPC_GET_SHARE_BUF(struct vcan_deinit_s);

    message.msg_id = SDPE_RPC_VCAN_GET_MSGID(VCAN_DEINIT);
    message.msg_type = SDPE_RPC_REQ;
    message.data_num = 1;
    message.data_len[0] = sizeof(uint32_t);
    message.data[0] = (uint8_t*)&vcan_deinit;

    sdpe_rpc_dev_send(rpc_dev, &message, SDPE_RPC_WAIT_FOREVER);
}

int virCan_SetBaudrate(uint8_t controller, uint16_t baudrate)
{
    struct sdpe_rpc_dev *rpc_dev = g_vcan_client.rpc_dev;
    struct vcan_set_baudrate_s *set_baud;
    struct vcan_set_baudrate_req *req;
    struct vcan_set_baudrate_resp *resp;
    sdpe_rpc_message_t message;

    if (!sdpe_rpc_is_ready()) {
        SDPE_RPC_ERR("sdpe rpc is not ready\n");
        return -1;
    }

    if (controller >= SDPE_RPC_MAX_VCAN_NUM) {
        SDPE_RPC_ERR("controller id error:%d\n", controller);
        return -1;
    }

    set_baud = SDPE_RPC_GET_MULTI_SHARE_BUF(struct vcan_set_baudrate_s,
                                            SDPE_RPC_MAX_VCAN_NUM, controller);
    req = &set_baud->req;
    req->controller = controller;
    req->baudrate = baudrate;

    resp = &set_baud->resp;
    resp->result = -1;

    message.msg_id = SDPE_RPC_VCAN_GET_MSGID(VCAN_SET_BAUDRATE);
    message.msg_type = SDPE_RPC_REQ;
    message.data_num = 1;
    message.data_len[0] = sizeof(uint32_t);
    message.data[0] = (uint8_t*)&set_baud;

    sdpe_rpc_dev_send(rpc_dev, &message, SDPE_RPC_WAIT_FOREVER);

    return resp->result;
}

int virCan_SetControllerMode(uint8_t controller, uint8_t mode)
{
    struct sdpe_rpc_dev *rpc_dev = g_vcan_client.rpc_dev;
    struct vcan_set_mode_s *set_mode;
    struct vcan_set_mode_req *req;
    struct vcan_set_mode_resp *resp;
    sdpe_rpc_message_t message;

    if (!sdpe_rpc_is_ready()) {
        SDPE_RPC_ERR("sdpe rpc is not ready\n");
        return -1;
    }

    if (controller >= SDPE_RPC_MAX_VCAN_NUM) {
        SDPE_RPC_ERR("controller id error:%d\n", controller);
        return -1;
    }

    set_mode = SDPE_RPC_GET_MULTI_SHARE_BUF(struct vcan_set_mode_s,
                                            SDPE_RPC_MAX_VCAN_NUM, controller);

    req = &set_mode->req;
    req->controller = controller;
    req->mode = mode;

    resp = &set_mode->resp;
    resp->result = -1;

    message.msg_id = SDPE_RPC_VCAN_GET_MSGID(VCAN_SET_MODE);
    message.msg_type = SDPE_RPC_REQ;
    message.data_num = 1;
    message.data_len[0] = sizeof(uint32_t);
    message.data[0] = (uint8_t*)&set_mode;

    sdpe_rpc_dev_send(rpc_dev, &message, SDPE_RPC_WAIT_FOREVER);

    return resp->result;
}

int virCan_GetControllerMode(uint8_t controller, uint8_t *mode)
{
    struct sdpe_rpc_dev *rpc_dev = g_vcan_client.rpc_dev;
    struct vcan_get_mode_s *get_mode;
    struct vcan_get_mode_req *req;
    struct vcan_get_mode_resp *resp;
    sdpe_rpc_message_t message;

    if (!sdpe_rpc_is_ready()) {
        SDPE_RPC_ERR("sdpe rpc is not ready\n");
        return -1;
    }

    if (controller >= SDPE_RPC_MAX_VCAN_NUM) {
        SDPE_RPC_ERR("controller id error:%d\n", controller);
        return -1;
    }

    get_mode = SDPE_RPC_GET_MULTI_SHARE_BUF(struct vcan_get_mode_s,
                                            SDPE_RPC_MAX_VCAN_NUM, controller);

    req = &get_mode->req;
    req->controller = controller;

    resp = &get_mode->resp;
    resp->result = -1;

    message.msg_id = SDPE_RPC_VCAN_GET_MSGID(VCAN_GET_MODE);
    message.msg_type = SDPE_RPC_REQ;
    message.data_num = 1;
    message.data_len[0] = sizeof(uint32_t);
    message.data[0] = (uint8_t*)&get_mode;

    sdpe_rpc_dev_send(rpc_dev, &message, SDPE_RPC_WAIT_FOREVER);

    *mode = resp->mode;
    return resp->result;
}

int virCan_EnableControllerInterrupts(uint8_t controller)
{
    struct sdpe_rpc_dev *rpc_dev = g_vcan_client.rpc_dev;
    struct vcan_enable_interrupts_s *enable_int;
    struct vcan_enable_interrupts_req *req;
    struct vcan_enable_interrupts_resp *resp;
    sdpe_rpc_message_t message;

    if (!sdpe_rpc_is_ready()) {
        SDPE_RPC_ERR("sdpe rpc is not ready\n");
        return -1;
    }

    if (controller >= SDPE_RPC_MAX_VCAN_NUM) {
        SDPE_RPC_ERR("controller id error:%d\n", controller);
        return -1;
    }

    enable_int = SDPE_RPC_GET_MULTI_SHARE_BUF(struct vcan_enable_interrupts_s,
                                              SDPE_RPC_MAX_VCAN_NUM, controller);

    req = &enable_int->req;
    req->controller = controller;

    resp = &enable_int->resp;
    resp->result = -1;

    message.msg_id = SDPE_RPC_VCAN_GET_MSGID(VCAN_ENABLE_INT);
    message.msg_type = SDPE_RPC_REQ;
    message.data_num = 1;
    message.data_len[0] = sizeof(uint32_t);
    message.data[0] = (uint8_t*)&enable_int;

    sdpe_rpc_dev_send(rpc_dev, &message, SDPE_RPC_WAIT_FOREVER);

    return resp->result;
}

int virCan_DisableControllerInterrupts(uint8_t controller)
{
    struct sdpe_rpc_dev *rpc_dev = g_vcan_client.rpc_dev;
    struct vcan_disable_interrupts_s *disable_int;
    struct vcan_disable_interrupts_req *req;
    struct vcan_disable_interrupts_resp *resp;
    sdpe_rpc_message_t message;

    if (!sdpe_rpc_is_ready()) {
        SDPE_RPC_ERR("sdpe rpc is not ready\n");
        return -1;
    }

    if (controller >= SDPE_RPC_MAX_VCAN_NUM) {
        SDPE_RPC_ERR("controller id error:%d\n", controller);
        return -1;
    }

    disable_int = SDPE_RPC_GET_MULTI_SHARE_BUF(struct vcan_disable_interrupts_s,
                                               SDPE_RPC_MAX_VCAN_NUM, controller);

    req = &disable_int->req;
    req->controller = controller;

    resp = &disable_int->resp;
    resp->result = -1;

    message.msg_id = SDPE_RPC_VCAN_GET_MSGID(VCAN_DISABLE_INT);
    message.msg_type = SDPE_RPC_REQ;
    message.data_num = 1;
    message.data_len[0] = sizeof(uint32_t);
    message.data[0] = (uint8_t*)&disable_int;

    sdpe_rpc_dev_send(rpc_dev, &message, SDPE_RPC_WAIT_FOREVER);

    return resp->result;
}

int virCan_CheckWakeup(uint8_t controller)
{
    struct sdpe_rpc_dev *rpc_dev = g_vcan_client.rpc_dev;
    struct vcan_check_wakeup_s *check_wakup;
    struct vcan_check_wakeup_req *req;
    struct vcan_check_wakeup_resp *resp;
    sdpe_rpc_message_t message;

    if (!sdpe_rpc_is_ready()) {
        SDPE_RPC_ERR("sdpe rpc is not ready\n");
        return -1;
    }

    if (controller >= SDPE_RPC_MAX_VCAN_NUM) {
        SDPE_RPC_ERR("controller id error:%d\n", controller);
        return -1;
    }

    check_wakup = SDPE_RPC_GET_MULTI_SHARE_BUF(struct vcan_check_wakeup_s,
                                               SDPE_RPC_MAX_VCAN_NUM, controller);

    req = &check_wakup->req;
    req->controller = controller;

    resp = &check_wakup->resp;
    resp->result = -1;

    message.msg_id = SDPE_RPC_VCAN_GET_MSGID(VCAN_CHECK_WAKEUP);
    message.msg_type = SDPE_RPC_REQ;
    message.data_num = 1;
    message.data_len[0] = sizeof(uint32_t);
    message.data[0] = (uint8_t*)&check_wakup;

    sdpe_rpc_dev_send(rpc_dev, &message, SDPE_RPC_WAIT_FOREVER);

    return resp->result;
}

int virCan_GetControllerErrorState(uint8_t controller, uint8_t *state)
{
    struct sdpe_rpc_dev *rpc_dev = g_vcan_client.rpc_dev;
    struct vcan_get_err_state_s *get_state;
    struct vcan_get_err_state_req *req;
    struct vcan_get_err_state_resp *resp;
    sdpe_rpc_message_t message;

    if (!sdpe_rpc_is_ready()) {
        SDPE_RPC_ERR("sdpe rpc is not ready\n");
        return -1;
    }

    if (controller >= SDPE_RPC_MAX_VCAN_NUM) {
        SDPE_RPC_ERR("controller id error:%d\n", controller);
        return -1;
    }

    get_state = SDPE_RPC_GET_MULTI_SHARE_BUF(struct vcan_get_err_state_s,
                                             SDPE_RPC_MAX_VCAN_NUM, controller);

    req = &get_state->req;
    req->controller = controller;

    resp = &get_state->resp;
    resp->result = -1;

    message.msg_id = SDPE_RPC_VCAN_GET_MSGID(VCAN_GET_ERR_STATE);
    message.msg_type = SDPE_RPC_REQ;
    message.data_num = 1;
    message.data_len[0] = sizeof(uint32_t);
    message.data[0] = (uint8_t*)&get_state;

    sdpe_rpc_dev_send(rpc_dev, &message, SDPE_RPC_WAIT_FOREVER);

    *state = resp->state;
    return resp->result;
}

int virCan_Write(uint16_t hth, const Can_PduType *pdu)
{
    struct sdpe_rpc_dev *rpc_dev = g_vcan_client.rpc_dev;
    struct vcan_write_s *write;
    struct vcan_write_req *req;
    struct vcan_write_resp *resp;
    sdpe_rpc_message_t message;
    int result;

    if (!sdpe_rpc_is_ready()) {
        SDPE_RPC_ERR("sdpe rpc is not ready\n");
        return -1;
    }

    if (hth >= SDPE_RPC_MAX_HTH_NUM) {
        SDPE_RPC_ERR("hth id error:%d\n", hth);
        return -1;
    }

    osMutexAcquire(vcan_write_mutex[hth], osWaitForever);

    write = SDPE_RPC_GET_MULTI_SHARE_BUF(struct vcan_write_s,
                                         SDPE_RPC_MAX_HTH_NUM, hth);

    req = &write->req;
    req->hth = hth;
    req->length = pdu->length;
    req->swPduHandle = pdu->swPduHandle;
    req->id = pdu->id;
    memcpy(req->data, pdu->sdu, pdu->length);

    resp = &write->resp;
    resp->result = -1;

    message.msg_id = SDPE_RPC_VCAN_GET_MSGID(VCAN_WRITE);
    message.msg_type = SDPE_RPC_REQ;
    message.data_num = 1;
    message.data_len[0] = sizeof(uint32_t);
    message.data[0] = (uint8_t*)&write;

    sdpe_rpc_dev_send(rpc_dev, &message, SDPE_RPC_WAIT_FOREVER);

    result = resp->result;

    osMutexRelease(vcan_write_mutex[hth]);

    return result;
}

void vircan_MainFunction_Write(void)
{
    struct sdpe_rpc_dev *rpc_dev = g_vcan_client.rpc_dev;
    struct vcan_mf_write_s *vcan_mf_write;
    sdpe_rpc_message_t message;

    if (!sdpe_rpc_is_ready()) {
        SDPE_RPC_ERR("sdpe rpc is not ready\n");
        return;
    }

    vcan_mf_write = SDPE_RPC_GET_SHARE_BUF(struct vcan_mf_write_s);

    message.msg_id = SDPE_RPC_VCAN_GET_MSGID(VCAN_MF_WRITE);
    message.msg_type = SDPE_RPC_REQ;
    message.data_num = 1;
    message.data_len[0] = sizeof(uint32_t);
    message.data[0] = (uint8_t*)&vcan_mf_write;

    sdpe_rpc_dev_send(rpc_dev, &message, SDPE_RPC_WAIT_FOREVER);
}

void vircan_MainFunction_Read(void)
{
    struct sdpe_rpc_dev *rpc_dev = g_vcan_client.rpc_dev;
    struct vcan_mf_read_s *vcan_mf_read;
    sdpe_rpc_message_t message;

    if (!sdpe_rpc_is_ready()) {
        SDPE_RPC_ERR("sdpe rpc is not ready\n");
        return;
    }

    vcan_mf_read = SDPE_RPC_GET_SHARE_BUF(struct vcan_mf_read_s);

    message.msg_id = SDPE_RPC_VCAN_GET_MSGID(VCAN_MF_READ);
    message.msg_type = SDPE_RPC_REQ;
    message.data_num = 1;
    message.data_len[0] = sizeof(uint32_t);
    message.data[0] = (uint8_t*)&vcan_mf_read;

    sdpe_rpc_dev_send(rpc_dev, &message, SDPE_RPC_WAIT_FOREVER);
}

int vcan_service_init(sdpe_rpc_service_cfg_t *cfg)
{
    int ret = 0;

#ifdef CONFIG_SDPE_RPC_MULTI_TASK
    sdpe_rpc_dev_cfg_t rpc_dev_cfg;

    rpc_dev_cfg.name = "VCAN_CLIENT";
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

    ret = sdpe_rpc_dev_open(&g_vcan_client.priv, vcan_rpc_cb, NULL);
    if (ret < 0) {
        SDPE_RPC_ERR("rpc dev open fail\n");
        return ret;
    }

    g_vcan_client.rpc_dev = &g_vcan_client.priv;
#else
    g_vcan_client.rpc_dev = \
        sdpe_rpc_service_cb_register(SDPE_RPC_VCAN_SERVICE, (void *)vcan_rpc_cb);
#endif

    vcan_mutex_init();
    return ret;
}
