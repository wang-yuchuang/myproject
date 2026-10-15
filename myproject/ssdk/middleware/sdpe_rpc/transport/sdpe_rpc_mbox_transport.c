/*
 * sdpe_rpc_mbox_transport.c
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
#include <cmsis_os2.h>
#include <lib/ering_buffer.h>

#include <driver/kunlun/mbox/sdrv_mbox_drv.h>
#include <sdpe_rpc_common.h>
#include <sdpe_rpc_transport.h>

typedef struct sdpe_rpc_mbox_instance {
    uint8_t rproc;
    uint32_t addr;
    mbox_dev_t *mb_dev;
    mbox_chan_t *mb_chan;
    ering_buffer_t *rx_buf;
    osSemaphoreId_t rx_sem;
} sdpe_rpc_mbox_instance_t;

static int sdpe_rpc_mbox_open(sdpe_rpc_transport_dev_t *dev);
static int sdpe_rpc_mbox_close(sdpe_rpc_transport_dev_t *dev);
static void *sdpe_rpc_mbox_tx_alloc(sdpe_rpc_transport_dev_t *dev, uint32_t *size,
                                    uint32_t wait_time);
static int sdpe_rpc_mbox_send(sdpe_rpc_transport_dev_t *dev, uint8_t *data,
                              uint32_t len, uint32_t wait_time);
static int sdpe_rpc_mbox_send_nocopy(sdpe_rpc_transport_dev_t *dev, uint8_t *data,
                                     uint32_t len);
static int sdpe_rpc_mbox_recv(sdpe_rpc_transport_dev_t *dev, uint8_t *data,
                              uint32_t *len, bool block);

struct sdpe_rpc_transport_ops g_rpc_mbox_ops = {
    .open                   = sdpe_rpc_mbox_open,
    .close                  = sdpe_rpc_mbox_close,
    .tx_alloc               = sdpe_rpc_mbox_tx_alloc,
    .send                   = sdpe_rpc_mbox_send,
    .send_nocopy            = sdpe_rpc_mbox_send_nocopy,
    .recv                   = sdpe_rpc_mbox_recv,
};

static int sdpe_rpc_mb_rx_callback(void *arg, uint8_t *data, uint32_t len)
{
    sdpe_rpc_mbox_instance_t *mbox = (sdpe_rpc_mbox_instance_t *)arg;

    ASSERT(mbox != NULL);
    ASSERT(data != NULL);

    if (ering_buffer_put(mbox->rx_buf, data, len) < 0) {
        SDPE_RPC_WARN("mbox rx buffer full\n");
        return -1;
    }

    osSemaphoreRelease(mbox->rx_sem);
    return 0;
}

static int sdpe_rpc_mbox_open(sdpe_rpc_transport_dev_t *dev)
{
    sdpe_rpc_mbox_instance_t *mbox = dev->priv;
    sdrv_mbox_chan_req_t chan_req;

    ASSERT(mbox != NULL);

    mbox->rx_sem = osSemaphoreNew(64, 0, NULL);
    if (!mbox->rx_sem) {
        SDPE_RPC_ERR("sem create fail\n");
        goto open_err_out;
    }
    
    mbox->rx_buf = ering_buffer_create(CONFIG_SDPE_RPC_MBOX_RX_BUF);
    if (!mbox->rx_buf) {
        SDPE_RPC_ERR("rx buffer create fail\n");
        goto open_err_out;
    }

    chan_req.rproc = mbox->rproc;
    chan_req.src_addr = mbox->addr;
    chan_req.dest_addr = mbox->addr;

    mbox->mb_chan = mbox_request_channel(mbox->mb_dev, &chan_req);
    if (!mbox->mb_chan) {
        SDPE_RPC_ERR("mbox channel get fail\n");
        goto open_err_out;
    }

    mbox_set_callback(mbox->mb_chan, sdpe_rpc_mb_rx_callback, mbox);

    return 0;

open_err_out:
    if (mbox->rx_sem) {
        osSemaphoreDelete(mbox->rx_sem);
        mbox->rx_sem = NULL;
    }

    if (mbox->rx_buf) {
        ering_buffer_destroy(mbox->rx_buf);
        mbox->rx_buf = NULL;
    }

    return -1;
}

static int sdpe_rpc_mbox_close(sdpe_rpc_transport_dev_t *dev)
{
    sdpe_rpc_mbox_instance_t *mbox = dev->priv;

    ASSERT(mbox != NULL);

    mbox_release_channel(mbox->mb_chan);
    mbox->mb_chan = NULL;

    ering_buffer_destroy(mbox->rx_buf);
    mbox->rx_buf = NULL;

    osSemaphoreDelete(mbox->rx_sem);
    mbox->rx_sem = NULL;

    return 0;
}

static void *sdpe_rpc_mbox_tx_alloc(sdpe_rpc_transport_dev_t *dev, uint32_t *size, uint32_t wait_time)
{
    sdpe_rpc_mbox_instance_t *mbox = dev->priv;
    uint8_t *buffer;

    buffer = mbox_alloc_buf(mbox->mb_chan, *size);
    if (!buffer) {
        SDPE_RPC_ERR("mbox channel alloc buffer fail\n");
    }

    return buffer;
}

static int sdpe_rpc_mbox_send(sdpe_rpc_transport_dev_t *dev, uint8_t *data,
                              uint32_t len, uint32_t wait_time)
{
    sdpe_rpc_mbox_instance_t *mbox = dev->priv;
    int ret;

    ASSERT(mbox != NULL && data != NULL);
    ret = mbox_send(mbox->mb_chan, data, len, wait_time);
    if (ret < 0) {
        SDPE_RPC_ERR("mbox send error\n");
    }

    return ret;
}

static int sdpe_rpc_mbox_send_nocopy(sdpe_rpc_transport_dev_t *dev, uint8_t *data, uint32_t len)
{
    sdpe_rpc_mbox_instance_t *mbox = dev->priv;
    int ret;

    ASSERT(mbox != NULL && data != NULL);
    ret = mbox_send_nocopy(mbox->mb_chan, data, len, SDPE_RPC_MAX_WAIT);
    if (ret < 0) {
        SDPE_RPC_ERR("mbox send error\n");
    }

    return ret;
}

static int sdpe_rpc_mbox_recv(sdpe_rpc_transport_dev_t *dev, uint8_t *data,
                              uint32_t *len, bool block)
{
    sdpe_rpc_mbox_instance_t *mbox = dev->priv;
    uint32_t timeout = 0;
    uint32_t data_len;

    if (block) {
        timeout = osWaitForever;
    }

    if (osSemaphoreAcquire(mbox->rx_sem, timeout) != osOK) {
        SDPE_RPC_ERR("get semaphore fail\n");
        return -1;
    }

    if (ering_buffer_get(mbox->rx_buf, data, &data_len) < 0) {
        SDPE_RPC_ERR("mbox buf read error\n");
        return -1;
    }

    if (data_len > *len) {
        SDPE_RPC_ERR("mbox buf read len error:%d\n", data_len);
        return -1;
    }

    *len = data_len;
    return 0;
}

int sdpe_rpc_mbox_init(sdpe_rpc_transport_dev_t *dev,
                       sdpe_rpc_transport_cfg_t *cfg)
{
    sdpe_rpc_mbox_instance_t *mbox;

    mbox = osAlloc(sizeof(sdpe_rpc_mbox_instance_t));
    if (mbox == NULL) {
        SDPE_RPC_ERR("mbox instance alloc fail\n");
        return -1;
    }
    memset(mbox, 0x00, sizeof(sdpe_rpc_mbox_instance_t));

    mbox->rproc = cfg->rproc;
    mbox->addr = cfg->addr;
    mbox->mb_dev = (mbox_dev_t *)cfg->lower_half;

    dev->name = cfg->name;
    dev->priv = mbox;
    dev->mtu = mbox_mtu(mbox->mb_dev);
    dev->ops = &g_rpc_mbox_ops;

    return 0;
}
