/*
 * sdpe_rpc_rpmsg_transport.c
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
#include <kernel/event.h>
#include <kernel/thread.h>
#include <assert.h>

#include <rpmsg_channel.h>
#include <sdpe_rpc_common.h>
#include <sdpe_rpc_transport.h>

typedef struct sdpe_rpc_rpmsg_instance {
    struct rpmsg_instance *rpmsg_dev;
    rpmsg_channel_t rpmsg_channel;
    uint32_t rproc;
    uint32_t addr;
} sdpe_rpc_rpmsg_instance_t;

static int sdpe_rpc_rpmsg_open(sdpe_rpc_transport_dev_t *dev);
static int sdpe_rpc_rpmsg_close(sdpe_rpc_transport_dev_t *dev);
static void *sdpe_rpc_rpmsg_tx_alloc(sdpe_rpc_transport_dev_t *dev, uint32_t *size,
                                     uint32_t wait_time);
static int sdpe_rpc_rpmsg_send(sdpe_rpc_transport_dev_t *dev, uint8_t *data,
                               uint32_t len, uint32_t wait_time);
static int sdpe_rpc_rpmsg_send_nocopy(sdpe_rpc_transport_dev_t *dev, uint8_t *data,
                                      uint32_t len);
static int sdpe_rpc_rpmsg_recv(sdpe_rpc_transport_dev_t *dev, uint8_t *data,
                               uint32_t *len, bool block);

struct sdpe_rpc_transport_ops g_rpc_rpmsg_ops = {
    .open                   = sdpe_rpc_rpmsg_open,
    .close                  = sdpe_rpc_rpmsg_close,
    .tx_alloc               = sdpe_rpc_rpmsg_tx_alloc,
    .send                   = sdpe_rpc_rpmsg_send,
    .send_nocopy            = sdpe_rpc_rpmsg_send_nocopy,
    .recv                   = sdpe_rpc_rpmsg_recv,
};

static int sdpe_rpc_rpmsg_open(sdpe_rpc_transport_dev_t *dev)
{
    sdpe_rpc_rpmsg_instance_t *rpmsg = dev->priv;

    ASSERT(rpmsg != NULL);

    rpmsg->rpmsg_channel = rpmsg_channel_create(rpmsg->rpmsg_dev, rpmsg->addr, dev->name);
    if (!rpmsg->rpmsg_channel)
        goto err_out;

    dev->mtu = rpmsg_channel_max_payload(rpmsg->rpmsg_channel);

    if (rpmsg_channel_start(rpmsg->rpmsg_channel, NULL, NULL) < 0) {
        SDPE_RPC_ERR("rpmsg_channel_start fail\n");
        goto err_out_with_ept;
    }

    return 0;

err_out_with_ept:
    rpmsg_channel_destroy(rpmsg->rpmsg_channel);

err_out:
    return -1;
}

static int sdpe_rpc_rpmsg_close(sdpe_rpc_transport_dev_t *dev)
{
    sdpe_rpc_rpmsg_instance_t *rpmsg = dev->priv;

    ASSERT(rpmsg != NULL);

    if (rpmsg->rpmsg_channel) {
        rpmsg_channel_destroy(rpmsg->rpmsg_channel);
    }

    return 0;
}

static void *sdpe_rpc_rpmsg_tx_alloc(sdpe_rpc_transport_dev_t *dev, uint32_t *size, uint32_t wait_time)
{
    sdpe_rpc_rpmsg_instance_t *rpmsg = dev->priv;
    uint32_t remain_size = 0;
    void *buffer;

    buffer = rpmsg_channel_alloc_send_buffer(rpmsg->rpmsg_channel, (unsigned long *)&remain_size, wait_time);
    *size = remain_size;
    return buffer;
}

static int sdpe_rpc_rpmsg_send(sdpe_rpc_transport_dev_t *dev, uint8_t *data,
                               uint32_t len, uint32_t wait_time)
{
    sdpe_rpc_rpmsg_instance_t *rpmsg = dev->priv;
    int ret;

    ASSERT(rpmsg != NULL && data != NULL);

    ret = rpmsg_channel_send(rpmsg->rpmsg_channel, rpmsg->addr,
                             (char *)data, len, wait_time);

    return ret;
}

static int sdpe_rpc_rpmsg_send_nocopy(sdpe_rpc_transport_dev_t *dev, uint8_t *data, uint32_t len)
{
    sdpe_rpc_rpmsg_instance_t *rpmsg = dev->priv;
    int ret;

    ASSERT(rpmsg != NULL && data != NULL);

    ret = rpmsg_channel_send_nocopy(rpmsg->rpmsg_channel, rpmsg->addr, (char *)data, len);

    return ret;
}

static int sdpe_rpc_rpmsg_recv(sdpe_rpc_transport_dev_t *dev, uint8_t *data,
                               uint32_t *len, bool block)
{
    sdpe_rpc_rpmsg_instance_t *rpmsg = dev->priv;
    unsigned long src;
    unsigned long wait_time = 0;

    ASSERT(rpmsg != NULL && data != NULL);

    if (block) {
        wait_time = SDPE_RPC_WAIT_FOREVER;
    }

    rpmsg_channel_recv(rpmsg->rpmsg_channel, &src, (char *)data,
                       dev->mtu, (int *)len, wait_time);

    return 0;
}

int sdpe_rpc_rpmsg_init(sdpe_rpc_transport_dev_t *dev,
                        sdpe_rpc_transport_cfg *cfg)
{
    sdpe_rpc_rpmsg_instance_t *rpmsg;

    rpmsg = osAlloc(sizeof(sdpe_rpc_rpmsg_instance_t));
    if (rpmsg == NULL) {
        SDPE_RPC_ERR("sdpe_rpc_rpmsg_init malloc fail\n");
        return -1;
    }
    memset(rpmsg, 0x00, sizeof(sdpe_rpc_rpmsg_instance_t));

    rpmsg->rproc = cfg->rproc;
    rpmsg->addr = cfg->addr;
    rpmsg->rpmsg_dev = (struct rpmsg_instance *)cfg->lower_half;

    dev->name = cfg->name;
    dev->priv = rpmsg;
    dev->ops = &g_rpc_rpmsg_ops;

    return 0;
}
