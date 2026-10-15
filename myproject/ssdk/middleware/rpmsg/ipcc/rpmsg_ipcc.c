/*
 * Copyright (c) 2014, Mentor Graphics Corporation
 * Copyright (c) 2015 Xilinx, Inc.
 * Copyright (c) 2016 Freescale Semiconductor, Inc.
 * Copyright 2016-2019 NXP
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#include <debug.h>
#include <stdlib.h>
#include <param.h>
#include <string.h>

#include <rpmsg.h>
#include <rpmsg_ipcc.h>
#include <driver/mbox.h>
#include <sd_errcode.h>

#include "../rpmsg_internal.h"

static int ipcc_tx(struct rpmsg_instance *ins, void *buffer, unsigned long len, unsigned short idx)
{
    struct rpmsg_ipcc_instance *ipcc;
    uint32_t timeout = 100;

    ipcc = (struct rpmsg_ipcc_instance *)ins->priv;

    return mbox_send_nocopy(ipcc->chan, (uint8_t *)buffer, (uint32_t)len, timeout);
}

static void *ipcc_tx_alloc(struct rpmsg_instance *ins, unsigned long *len, unsigned short *idx)
{
    struct rpmsg_ipcc_instance *ipcc;

    ipcc = (struct rpmsg_ipcc_instance *)ins->priv;

    if (*len > CONFIG_IPCC_PAYLOAD_SIZE) {
        *len = CONFIG_IPCC_PAYLOAD_SIZE;
    }

    *idx = 0;
    return (void *)mbox_alloc_buf(ipcc->chan, (uint32_t)(*len));
}

static int ipcc_rx_free(struct rpmsg_instance *ins, void *buffer, unsigned long len, unsigned short idx)
{
    struct rpmsg_ipcc_instance *ipcc;

    ipcc = (struct rpmsg_ipcc_instance *)ins->priv;
    buffqueue_free_buffer(ipcc->recv_queue, idx);
    return RPMSG_SUCCESS;
}

static int ipcc_tx_direct(struct rpmsg_instance *ins, unsigned long src, unsigned long dst,
                          char *data, unsigned long len, unsigned long timeout)
{
    int ret;
    unsigned long total;
    struct rpmsg_std_msg *rpmsg_msg;
    struct rpmsg_ipcc_instance *ipcc;

    total = len + sizeof(struct rpmsg_std_hdr);
    if (total > CONFIG_IPCC_PAYLOAD_SIZE) {
        total = CONFIG_IPCC_PAYLOAD_SIZE;
        len = CONFIG_IPCC_PAYLOAD_SIZE - sizeof(struct rpmsg_std_hdr);
    }

    rpmsg_msg = (struct rpmsg_std_msg *)osAlloc(total);
    if (rpmsg_msg == NULL) {
        ssdk_printf(SSDK_WARNING, "%s: alloc memory failed.\n", __func__);
        ERRCODE_PRINT(MOD_ID_SYS_SERVICE_COMM,
                ins->remote_proc, COM_IPCC_MALLOC_FAIL, 0);
        return RPMSG_ERR_NO_MEM;
    }

    memset(rpmsg_msg, 0x00, total);
    rpmsg_msg->hdr.dst = (uint32_t)dst;
    rpmsg_msg->hdr.src = (uint32_t)src;
    rpmsg_msg->hdr.len = (uint16_t)len;
    memcpy(rpmsg_msg->data, data, len);
    ipcc = (struct rpmsg_ipcc_instance *)ins->priv;
    ret = mbox_send(ipcc->chan, (uint8_t *)rpmsg_msg,
                    (uint32_t)total, (uint32_t)timeout);
    osFree(rpmsg_msg);
    return ret;
}

static struct rpmsg_lower_ops lower_ops = {
    .tx        = ipcc_tx,
    .tx_alloc  = ipcc_tx_alloc,
    .rx_free   = ipcc_rx_free,
    .tx_direct = ipcc_tx_direct,
};

static int ipcc_get_notified(void *arg, uint8_t *mssg, uint32_t len)
{
    void *buff;
    uint32_t size;
    uint16_t index;
    struct rpmsg_ipcc_instance *ipcc;

    ipcc = (struct rpmsg_ipcc_instance *)arg;

    if (mssg != NULL) {
        buff = buffqueue_alloc_buffer(ipcc->recv_queue, &size, &index);
        if (buff != NULL) {
            if (len > size) {
                len = size;
                ssdk_printf(SSDK_WARNING, "%s: len %d, size %d, trimed.\n",
                            __func__, len, size);
            }
            memcpy(buff, mssg, len);
            buffqueue_enqueue(ipcc->recv_queue, index);
            osEventFlagsSet(ipcc->event, 0x1);
        }
        else {
            /* 中断上下文: 不 printf(逐帧打印会占满 ISR, 饿死所有任务, 形成
             * 丢帧->打印->更多丢帧 的死亡螺旋), 直接丢帧。 */
        }
    }

    return 0;
}

static void ipcc_notified_thread(void *arg)
{
    struct rpmsg_instance *rpmsg;
    struct rpmsg_ipcc_instance *ipcc;
    struct rpmsg_std_msg *rpmsg_msg;
    struct rpmsg_hdr_reserved *rsvd;
    struct rpmsg_endpoint *ept;
    uint32_t len;
    uint16_t index;
    int ret;

    rpmsg = (struct rpmsg_instance *)arg;
    ipcc = (struct rpmsg_ipcc_instance *)rpmsg->priv;

    while (1) {
        if (osEventFlagsWait(ipcc->event, 0x1, osFlagsWaitAny, osWaitForever) == 0x1) {
            rpmsg_msg = (struct rpmsg_std_msg *)buffqueue_dequeue(ipcc->recv_queue, &len, &index);
            while (rpmsg_msg != NULL) {
                ret = RPMSG_RELEASE;
                osMutexAcquire(rpmsg->mutex, osWaitForever);
                ept = rpmsg_get_ept(rpmsg, rpmsg_msg->hdr.dst);
                osMutexRelease(rpmsg->mutex);

                //@NOTE ensure index assignment and rx_free invoke timing sequence
                rsvd = (struct rpmsg_hdr_reserved *)&rpmsg_msg->hdr.reserved;
                rsvd->idx = index;

                if (ept != NULL) {
                    ret = ept->rx_cb(rpmsg_msg->data, rpmsg_msg->hdr.len, rpmsg_msg->hdr.src, ept->rx_cb_data);
                }

                if (ret == RPMSG_HOLD) {
                    // do nothing
                }
                else {
                    osMutexAcquire(rpmsg->mutex, osWaitForever);
                    rpmsg->lower_ops->rx_free(rpmsg, rpmsg_msg, len, index);
                    osMutexRelease(rpmsg->mutex);
                }

                rpmsg_msg = (struct rpmsg_std_msg *)buffqueue_dequeue(ipcc->recv_queue, &len, &index);
            }
        }
    }
}

static int rpmsg_ipcc_init(struct rpmsg_instance *ins, void *cfg)
{
    struct rpmsg_ipcc_instance *ipcc;
    struct rpmsg_ipcc_config *config;
    struct sdrv_mbox_chan_req chan_req;

    ipcc = (struct rpmsg_ipcc_instance *)ins->priv;
    config = (struct rpmsg_ipcc_config *)cfg;

    memset(ipcc, 0x00, sizeof(struct rpmsg_ipcc_instance));

    ipcc->event = osEventFlagsNew(NULL);
    if (ipcc->event == NULL) {
        ssdk_printf(SSDK_WARNING, "%s: event create failed.\n", __func__);
        return RPMSG_ERR_NO_MEM;
    }

    ipcc->recv_queue = buffqueue_create(CONFIG_IPCC_PAYLOAD_SIZE,
                                        CONFIG_IPCC_BUFF_NUM);
    if (ipcc->recv_queue == NULL) {
        ssdk_printf(SSDK_WARNING, "%s: buffer queue create failed.\n", __func__);
        osEventFlagsDelete(ipcc->event);
        return RPMSG_ERR_NO_MEM;
    }

    chan_req.rproc = config->remote_proc;

#ifdef CONFIG_KUNLUN_MBOX
    chan_req.src_addr = (int)config->src_addr;
    chan_req.dest_addr = (int)config->dst_addr;
#endif

#ifdef CONFIG_TAISHAN_MBOX
    chan_req.src_addr = MBOX_ADDR_IPCC;
    chan_req.dest_addr = MBOX_ADDR_IPCC;
#endif

    ipcc->chan = mbox_request_channel(config->mbox_ins, &chan_req);
    if (ipcc->chan == NULL) {
        ssdk_printf(SSDK_WARNING, "%s: mbox channel create failed.\n", __func__);
        osEventFlagsDelete(ipcc->event);
        buffqueue_destroy(ipcc->recv_queue);
        return RPMSG_ERR_NO_MEM;
    }

    mbox_set_callback(ipcc->chan, ipcc_get_notified, (void *)ipcc);
    osThreadAttr_t attr = {
        .name = "ipcc_notify",
        .priority = 28,   /* 高于 CAN 收报(26): 保证 ICC 分发/缓冲归还及时, 否则握手被饿死 */
    };
    ipcc->thread = osThreadNew(ipcc_notified_thread, (void *)ins, &attr);
    ipcc->is_master = config->is_master;
    ipcc->link_state = RPMSG_LINKSTATE_UP;

    return RPMSG_SUCCESS;
}

static int rpmsg_ipcc_deinit(struct rpmsg_instance *ins)
{
    struct rpmsg_ipcc_instance *ipcc;

    ipcc = (struct rpmsg_ipcc_instance *)ins->priv;

    ipcc->link_state = RPMSG_LINKSTATE_DOWN;
    osThreadTerminate(ipcc->thread);
    mbox_release_channel(ipcc->chan);
    buffqueue_destroy(ipcc->recv_queue);
    osEventFlagsDelete(ipcc->event);

    return RPMSG_SUCCESS;
}

static int rpmsg_ipcc_is_master(struct rpmsg_instance *ins)
{
    struct rpmsg_ipcc_instance *ipcc;

    ipcc = (struct rpmsg_ipcc_instance *)ins->priv;
    return (int)(ipcc->is_master);
}

static int rpmsg_ipcc_get_buffer_num(struct rpmsg_instance *ins)
{
    return CONFIG_IPCC_BUFF_NUM;
}

static int rpmsg_ipcc_get_payload_size(struct rpmsg_instance *ins)
{
    struct rpmsg_ipcc_instance *ipcc;
    int size = CONFIG_IPCC_PAYLOAD_SIZE;

    ipcc = (struct rpmsg_ipcc_instance *)ins->priv;
    size = MIN(size, ipcc->chan->dev->mtu);

    return size - sizeof(struct rpmsg_std_hdr);
}

static int rpmsg_ipcc_get_link_status(struct rpmsg_instance *ins)
{
    struct rpmsg_ipcc_instance *ipcc;

    ipcc = (struct rpmsg_ipcc_instance *)ins->priv;
    return ipcc ? (int)(ipcc->link_state) : RPMSG_LINKSTATE_ERRSTATE;
}

static struct rpmsg_instance_ops ins_ops = {
    .master_init      = rpmsg_ipcc_init,
    .remote_init      = rpmsg_ipcc_init,
    .deinit           = rpmsg_ipcc_deinit,
    .is_master        = rpmsg_ipcc_is_master,
    .get_buffer_num   = rpmsg_ipcc_get_buffer_num,
    .get_payload_size = rpmsg_ipcc_get_payload_size,
    .get_link_status  = rpmsg_ipcc_get_link_status,
};

int rpmsg_ipcc_probe(struct rpmsg_instance *rpmsg,
                     struct rpmsg_ipcc_instance *ipcc,
                     struct rpmsg_ipcc_config *cfg)
{
    rpmsg->ins_ops = &ins_ops;
    rpmsg->lower_ops = &lower_ops;
    rpmsg->priv = (void *)ipcc;
    rpmsg->rpmsg_type = RPMSG_TYPE_IPCC;
    rpmsg->remote_proc = cfg->remote_proc;

    return rpmsg_master_init(rpmsg, (void *)cfg);
}
