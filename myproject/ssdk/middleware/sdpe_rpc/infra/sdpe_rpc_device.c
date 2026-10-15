/*
 * sdpe_rpc_device.c
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#include <stdlib.h>
#include <string.h>
#include <debug.h>
#include <cmsis_os2.h>

#include <sdpe_rpc_device.h>

#include "sdpe_rpc_pack.h"

#define SDPE_RPC_STATUS_NULL        0
#define SDPE_RPC_STATUS_INIT        1
#define SDPE_RPC_STATUS_READY       2
#define SDPE_RPC_CHECK_STATUS(dev)  (dev->status == SDPE_RPC_STATUS_READY)

#define SDPE_RPC_WORKQ_NUM          64

typedef struct sdpe_rpc_cookie {
    osSemaphoreId_t sem;
} sdpe_rpc_cookie_t;

static void sdpe_rpc_send_resp(sdpe_rpc_dev_t *dev,
                               struct sdpe_rpc_message_head *message)
{
    struct sdpe_rpc_message_head resp;

    resp.msg_type = SDPE_RPC_RSP;
    resp.cookie = message->cookie;
    resp.msg_id = message->msg_id;

    osMutexAcquire(dev->tx_lock, osWaitForever);
    sdpe_rpc_transport_send(&dev->transport, (uint8_t *)&resp,
                            sizeof(struct sdpe_rpc_message_head),
                            SDPE_RPC_MAX_WAIT);
    osMutexRelease(dev->tx_lock);
}

static int sdpe_rpc_dev_msg_ind(sdpe_rpc_dev_t *dev,
                                struct sdpe_rpc_message_head *message, uint32_t len)
{
    int ret = 0;

    ret = dev->usr_cb(dev->arg, message->msg_id, message->data,
                      (len - SDPE_RPC_MESSAGE_HEAD));

    if (SDPE_RPC_REQ == message->msg_type) {
        sdpe_rpc_send_resp(dev, message);
    }

    return ret;
}

static void sdpe_rpc_dev_workq_thread(void *arg)
{
    sdpe_rpc_dev_t *dev = (sdpe_rpc_dev_t *)arg;
    uint8_t *recv_buf;
    uint32_t data_len;

    recv_buf = osAlloc(dev->rx_buf_len);
    if (recv_buf == NULL) {
        SDPE_RPC_ERR("rpc workq buffer osAlloc fail\n");
        return;
    }

    while (1) {
        osSemaphoreAcquire(dev->wq_sem, osWaitForever);
        while (ering_buffer_get(dev->wq_buffer, recv_buf, &data_len) >= 0) {
            sdpe_rpc_dev_msg_ind(dev, (struct sdpe_rpc_message_head *)recv_buf, data_len);
        }
    }

    osFree(recv_buf);
}

static int sdpe_rpc_dev_workq_init(sdpe_rpc_dev_t *dev, const char *name)
{
    int ret = -1;

    osThreadAttr_t workq_thread_attr = {
        .name = name,
        .stack_size = 1024,
        .priority = osPriorityNormal,
    };

    dev->wq_buffer = ering_buffer_create(SDPE_RPC_WORKQ_NUM * dev->rx_buf_len);
    if (!dev->wq_buffer) {
        SDPE_RPC_ERR("create workq buffer fail\n");
        return ret;
    }

    dev->wq_sem = osSemaphoreNew(1, 0, NULL);
    if (!dev->wq_sem) {
        SDPE_RPC_ERR("create workq sem fail\n");
        goto err_out_with_wq_buffer;
    }

    dev->wq_thread = osThreadNew(sdpe_rpc_dev_workq_thread,
                                 dev, &workq_thread_attr);
    if (!dev->wq_thread) {
        SDPE_RPC_ERR("create workq thread fail\n");
        goto err_out_with_wq_sem;
    }

    return 0;

err_out_with_wq_sem:
    osSemaphoreDelete(dev->wq_sem);

err_out_with_wq_buffer:
    ering_buffer_destroy(dev->wq_buffer);

    return ret;
}

static void sdpe_rpc_dev_workq_deinit(sdpe_rpc_dev_t *dev)
{
    ering_buffer_destroy(dev->wq_buffer);
    osSemaphoreDelete(dev->wq_sem);
    osThreadTerminate(dev->wq_thread);
}

static int sdpe_rpc_dev_workq_send(sdpe_rpc_dev_t *dev,
                                    struct sdpe_rpc_message_head *message, uint32_t len)
{
    int ret;

    ret = ering_buffer_put(dev->wq_buffer, (uint8_t *)message, len);
    if (ret >= 0) {
        osSemaphoreRelease(dev->wq_sem);
    }
    else if (message->msg_type == SDPE_RPC_REQ) {
        sdpe_rpc_send_resp(dev, message);
    }

    return ret;
}

static int sdpe_rpc_dev_data_proc(sdpe_rpc_dev_t *dev, uint8_t *data, uint32_t len)
{
    struct sdpe_rpc_message_head *message;
    int ret = 0;

    ASSERT(dev != NULL && data != NULL);
    ASSERT(len >= SDPE_RPC_MESSAGE_HEAD);

    message = (struct sdpe_rpc_message_head *)data;

    if (SDPE_RPC_RSP == message->msg_type) {
        sdpe_rpc_cookie_t *cookie = (sdpe_rpc_cookie_t *)message->cookie;
        osSemaphoreRelease(cookie->sem);
    }
    else {
        if (dev->support_workq) {
            ret = sdpe_rpc_dev_workq_send(dev, message, len);
        }
        else {
            ret = sdpe_rpc_dev_msg_ind(dev, message, len);
        }
    }
    return ret;
}

static void sdpe_rpc_dev_rx_thread(void *arg)
{
    sdpe_rpc_dev_t *dev = (sdpe_rpc_dev_t *)arg;
    uint32_t data_len;
    int ret;

    ASSERT(dev != NULL);

    while (1) {
        data_len = dev->rx_buf_len;

        if (dev->support_pack) {
            ret = sdpe_rpc_dev_unpack_recv(dev, dev->rx_buf, &data_len);
        }
        else {
            ret = sdpe_rpc_transport_recv(&dev->transport, dev->rx_buf,
                                          &data_len, true);
        }

        if (ret < 0) {
            SDPE_RPC_ERR("sdpe rpc dev recv message fail\n");
            continue;
        }

        sdpe_rpc_dev_data_proc(dev, dev->rx_buf, data_len);
    }
}

static uint8_t *sdpe_rpc_dev_alloc_msg(sdpe_rpc_dev_t *dev,
                                           uint32_t message_len, uint32_t wait_time)
{
    uint32_t len = message_len;

    len = dev->support_pack ? SDPE_RPC_MESSAGE_HEAD : message_len;

    return sdpe_rpc_transport_tx_alloc(&dev->transport, &len, wait_time);
}

static int sdpe_rpc_dev_send_msg(sdpe_rpc_dev_t *dev,
                                 struct sdpe_rpc_message_head *rpc_header,
                                 sdpe_rpc_message_t *message, uint8_t *buffer,
                                 uint32_t total_len)
{
    uint32_t offset = SDPE_RPC_MESSAGE_HEAD;
    int ret;

    if (!dev->support_pack) {
        memcpy(buffer, rpc_header, SDPE_RPC_MESSAGE_HEAD);
        for (uint8_t cnt = 0; cnt < message->data_num; cnt++) {
            memcpy(&buffer[offset], message->data[cnt], message->data_len[cnt]);
            offset += message->data_len[cnt];
        }
        ret = sdpe_rpc_transport_send_nocopy(&dev->transport, buffer, total_len);
    }
    else {
        ret = sdpe_rpc_dev_pack_send(dev, rpc_header, message, buffer);
    }

    return ret;
}

int sdpe_rpc_dev_send(sdpe_rpc_dev_t *dev, sdpe_rpc_message_t *message, uint32_t wait_time)
{
    struct sdpe_rpc_message_head rpc_header;
    sdpe_rpc_cookie_t cookie;
    uint32_t total_len = SDPE_RPC_MESSAGE_HEAD;
    uint8_t *buffer;
    int ret = -1;

    ASSERT(dev != NULL);

    if (!SDPE_RPC_CHECK_STATUS(dev)) {
        SDPE_RPC_WARN("rpc dev not ready\n");
        return ret;
    }

    for (uint8_t cnt = 0; cnt < message->data_num; cnt++) {
        total_len += message->data_len[cnt];
    }

    osMutexAcquire(dev->tx_lock, osWaitForever);

    buffer = sdpe_rpc_dev_alloc_msg(dev, total_len, wait_time);
    if (!buffer) {
        SDPE_RPC_ERR("transport tx alloc fail\n");
        goto err_out_with_lock;
    }

    rpc_header.msg_id = message->msg_id;
    rpc_header.msg_type = message->msg_type;

    if (message->msg_type == SDPE_RPC_REQ) {
        cookie.sem = osSemaphoreNew(1, 0, NULL);
        rpc_header.cookie = (uint32_t)(&cookie);
    }

    ret = sdpe_rpc_dev_send_msg(dev, &rpc_header, message, buffer, total_len);

    osMutexRelease(dev->tx_lock);

    if (ret < 0) {
        SDPE_RPC_ERR("rpc message send fail:%d\n", ret);
        goto err_out_with_sem;
    }

    if (message->msg_type == SDPE_RPC_REQ) {
        while (osSemaphoreAcquire(cookie.sem, osWaitForever) != osOK);
    }

err_out_with_sem:
    if (message->msg_type == SDPE_RPC_REQ) {
        osSemaphoreDelete(cookie.sem);
    }
    return ret;

err_out_with_lock:
    osMutexRelease(dev->tx_lock);
    return ret;
}

int sdpe_rpc_dev_open(sdpe_rpc_dev_t *dev, sdpe_rpc_dev_cb usr_cb, void *arg)
{
    char thread_name[64];
    int ret = -1;

    ASSERT(dev != NULL);

    dev->usr_cb = usr_cb;
    dev->arg = arg;

    dev->rx_buf = osAlloc(dev->rx_buf_len);
    if (dev->rx_buf == NULL) {
        SDPE_RPC_ERR("rpc transport malloc fail\n");
        goto err_out;
    }

    if (dev->support_pack) {
        ret = sdpe_rpc_dev_pack_init(dev);
        if (ret < 0) {
            SDPE_RPC_ERR("allocates pack buffer failed\n");
            goto err_out_with_rx_buf;
        }
    }

    if (dev->support_workq) {
        snprintf(thread_name, 64, "%s_WQ", dev->name);
        ret = sdpe_rpc_dev_workq_init(dev, thread_name);
        if (ret < 0) {
            SDPE_RPC_ERR("workq init failed\n");
            goto err_out_with_pack;
        }
    }

    dev->tx_lock = osMutexNew(NULL);
    if (!dev->tx_lock) {
        SDPE_RPC_ERR("create tx lock fail\n");
        goto err_out_with_workq;
    }

    ret = sdpe_rpc_transport_open(&dev->transport);
    if (ret < 0) {
        SDPE_RPC_ERR("rpc transport open fail:%s\n", dev->name);
        goto err_out_with_mutex;
    }

    dev->status = SDPE_RPC_STATUS_READY;

    snprintf(thread_name, 64, "%s_RX", dev->name);
    osThreadAttr_t rx_thread_attr = {
        .name = thread_name,
        .stack_size = 1024,
        .priority = osPriorityHigh,
    };
    dev->rx_thread = osThreadNew(sdpe_rpc_dev_rx_thread, dev, &rx_thread_attr);
    if (!dev->rx_thread) {
        SDPE_RPC_ERR("create rx thread fail\n");
        goto err_out_with_open;
    }

    return 0;

err_out_with_open:
    sdpe_rpc_transport_close(&dev->transport);

err_out_with_mutex:
    osMutexDelete(dev->tx_lock);

err_out_with_workq:
    if (dev->support_workq) {
        sdpe_rpc_dev_workq_deinit(dev);
    }

err_out_with_pack:
    if (dev->support_pack) {
        sdpe_rpc_dev_pack_deinit(dev);
    }

err_out_with_rx_buf:
    osFree(dev->rx_buf);

err_out:
    return -1;
}

int sdpe_rpc_dev_close(sdpe_rpc_dev_t *dev)
{
    ASSERT(dev != NULL);

    dev->status = SDPE_RPC_STATUS_INIT;

    sdpe_rpc_transport_close(&dev->transport);

    if (dev->support_pack) {
        sdpe_rpc_dev_pack_deinit(dev);
    }

    if (dev->support_workq) {
        sdpe_rpc_dev_workq_deinit(dev);
    }

    osMutexDelete(dev->tx_lock);
    osFree(dev->rx_buf);

    return 0;
}

int sdpe_rpc_dev_init(sdpe_rpc_dev_t *dev, sdpe_rpc_dev_cfg_t *cfg)
{
    sdpe_rpc_transport_cfg_t transport_cfg;

    ASSERT(dev != NULL);

    memset(dev, 0x00, sizeof(sdpe_rpc_dev_t));

    dev->support_pack = cfg->support_pack;
    dev->support_workq = cfg->support_workq;
    dev->rx_buf_len = cfg->max_msg_len + SDPE_RPC_MESSAGE_HEAD;

    strcpy(dev->name, cfg->name);

    transport_cfg.name = dev->name;
    transport_cfg.type = cfg->type;
    transport_cfg.rproc = cfg->rproc;
    transport_cfg.addr = cfg->addr;
    transport_cfg.lower_half = cfg->lower_half;

    if (sdpe_rpc_transport_init(&dev->transport, &transport_cfg) < 0) {
        SDPE_RPC_ERR("rpc transport init fail\n");
        return -1;
    }

    dev->status = SDPE_RPC_STATUS_INIT;

    return 0;
}
