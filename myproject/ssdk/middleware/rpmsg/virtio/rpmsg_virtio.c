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
#include <rpmsg.h>
#include <rpmsg_virtio.h>

#if defined(CONFIG_KUNLUN_MBOX)
#include <driver/kunlun/mbox/sdrv_mbox_drv.h>
#elif defined(CONFIG_TAISHAN_MBOX)
#include <driver/taishan/mbox/sdrv_taishan_mbox_drv.h>
#endif

#include "../rpmsg_internal.h"
#include "rpmsg_default_config.h"
#include "rpmsg_lite.h"
#include "rpmsg_platform.h"
#include "rpmsg_env.h"
#include "virtqueue.h"
#include "virtio_ring.h"

#if (!RL_BUFFER_COUNT) || (RL_BUFFER_COUNT & (RL_BUFFER_COUNT - 1))
#error "RL_BUFFER_COUNT must be power of two (2, 4, ...)"
#endif

/* Buffer is formed by payload and struct rpmsg_std_hdr */
#define RL_BUFFER_SIZE (RL_BUFFER_PAYLOAD_SIZE + 16)

#if (!RL_BUFFER_SIZE) || (RL_BUFFER_SIZE & (RL_BUFFER_SIZE - 1))
#error "RL_BUFFER_SIZE must be power of two (256, 512, ...)"\
       "RL_BUFFER_PAYLOAD_SIZE must be equal to (240, 496, 1008, ...) [2^n - 16]."
#endif

/****************************************************************************

 m    m  mmmm         m    m   mm   mm   m mmmm   m      mmmmm  mm   m   mmm
 "m  m" m"  "m        #    #   ##   #"m  # #   "m #        #    #"m  # m"   "
  #  #  #    #        #mmmm#  #  #  # #m # #    # #        #    # #m # #   mm
  "mm"  #    #        #    #  #mm#  #  # # #    # #        #    #  # # #    #
   ##    #mm#"        #    # #    # #   ## #mmm"  #mmmmm mm#mm  #   ##  "mmm"
            #
 In case this processor has the MASTER role
*****************************************************************************/

/*!
 * @brief
 * Places buffer on the virtqueue for consumption by the other side.
 *
 * @param vq      Virtqueue to use
 * @param buffer  Buffer pointer
 * @param len     Buffer length
 * @idx           Buffer index
 *
 * @return Status of function execution
 *
 */
static int vq_tx_master(struct rpmsg_instance *ins, void *buffer, unsigned long len, unsigned short idx)
{
    int status;
    struct rpmsg_lite_instance *lite;

    lite = (struct rpmsg_lite_instance *)ins->priv;
    status = virtqueue_add_buffer(lite->tvq, idx);
    RL_ASSERT(status == VQUEUE_SUCCESS); /* must success here */

    virtqueue_kick(lite->tvq);

    return RL_SUCCESS;

    /* As long as the length of the virtqueue ring buffer is not shorter
     * than the number of buffers in the pool, this function should not fail.
     * This condition is always met, so we don't need to return anything here */
}

/*!
 * @brief
 * Provides buffer to transmit messages.
 *
 * @param vq      Virtqueue to use
 * @param len     Length of returned buffer
 * @param idx     Buffer index
 *
 * return Pointer to buffer.
 */
static void *vq_tx_alloc_master(struct rpmsg_instance *ins, unsigned long *len, unsigned short *idx)
{
    struct rpmsg_lite_instance *lite;

    lite = (struct rpmsg_lite_instance *)ins->priv;
    return virtqueue_get_buffer(lite->tvq, (uint32_t *)len, idx);
}

/*!
 * @brief
 * Retrieves the received buffer from the virtqueue.
 *
 * @param vq   Virtqueue to use
 * @param len  Size of received buffer
 * @param idx  Index of buffer
 *
 * @return  Pointer to received buffer
 *
 */
static void *vq_rx_master(struct rpmsg_instance *ins, unsigned long *len, unsigned short *idx)
{
    struct rpmsg_lite_instance *lite;

    lite = (struct rpmsg_lite_instance *)ins->priv;
    return virtqueue_get_buffer(lite->rvq, (uint32_t *)len, idx);
}

/*!
 * @brief
 * Places the used buffer back on the virtqueue.
 *
 * @param vq   Virtqueue to use
 * @param len  Size of received buffer
 * @param idx  Index of buffer
 *
 */
static int vq_rx_free_master(struct rpmsg_instance *ins, void *buffer, unsigned long len, unsigned short idx)
{
    int status;
    struct rpmsg_lite_instance *lite;

    lite = (struct rpmsg_lite_instance *)ins->priv;
#if defined(RL_CLEAR_USED_BUFFERS) && (RL_CLEAR_USED_BUFFERS == 1)
    env_memset(buffer, 0x00, len);
#endif
    status = virtqueue_add_buffer(lite->rvq, idx);
    RL_ASSERT(status == VQUEUE_SUCCESS); /* must success here */

    return RL_SUCCESS;

    /* As long as the length of the virtqueue ring buffer is not shorter
     * than the number of buffers in the pool, this function should not fail.
     * This condition is always met, so we don't need to return anything here */
}

static struct rpmsg_lower_ops master_lower_ops = {
    .tx        = vq_tx_master,
    .tx_alloc  = vq_tx_alloc_master,
    .rx        = vq_rx_master,
    .rx_free   = vq_rx_free_master,
};

/****************************************************************************

 m    m  mmmm         m    m   mm   mm   m mmmm   m      mmmmm  mm   m   mmm
 "m  m" m"  "m        #    #   ##   #"m  # #   "m #        #    #"m  # m"   "
  #  #  #    #        #mmmm#  #  #  # #m # #    # #        #    # #m # #   mm
  "mm"  #    #        #    #  #mm#  #  # # #    # #        #    #  # # #    #
   ##    #mm#"        #    # #    # #   ## #mmm"  #mmmmm mm#mm  #   ##  "mmm"
            #
 In case this processor has the REMOTE role
*****************************************************************************/
/*!
 * @brief
 * Places buffer on the virtqueue for consumption by the other side.
 *
 * @param vq      Virtqueue to use
 * @param buffer  Buffer pointer
 * @param len     Buffer length
 * @idx           Buffer index
 *
 * @return Status of function execution
 *
 */
static int vq_tx_remote(struct rpmsg_instance *ins, void *buffer, unsigned long len, unsigned short idx)
{
    int status;
    struct rpmsg_lite_instance *lite;

    lite = (struct rpmsg_lite_instance *)ins->priv;
    len = (unsigned long)virtqueue_get_buffer_length(lite->tvq, idx);
    status = virtqueue_add_consumed_buffer(lite->tvq, idx, len);
    RL_ASSERT(status == VQUEUE_SUCCESS); /* must success here */

    virtqueue_kick(lite->tvq);

    return RL_SUCCESS;

    /* As long as the length of the virtqueue ring buffer is not shorter
     * than the number of buffers in the pool, this function should not fail.
     * This condition is always met, so we don't need to return anything here */
}

/*!
 * @brief
 * Provides buffer to transmit messages.
 *
 * @param vq      Virtqueue to use
 * @param len     Length of returned buffer
 * @param idx     Buffer index
 *
 * return Pointer to buffer.
 */
static void *vq_tx_alloc_remote(struct rpmsg_instance *ins, unsigned long *len, unsigned short *idx)
{
    struct rpmsg_lite_instance *lite;

    lite = (struct rpmsg_lite_instance *)ins->priv;
    return virtqueue_get_available_buffer(lite->tvq, idx, (uint32_t *)len);
}

/*!
 * @brief
 * Retrieves the received buffer from the virtqueue.
 *
 * @param vq   Virtqueue to use
 * @param len  Size of received buffer
 * @param idx  Index of buffer
 *
 * @return  Pointer to received buffer
 *
 */
static void *vq_rx_remote(struct rpmsg_instance *ins, unsigned long *len, unsigned short *idx)
{
    struct rpmsg_lite_instance *lite;

    lite = (struct rpmsg_lite_instance *)ins->priv;
    return virtqueue_get_available_buffer(lite->rvq, idx, (uint32_t *)len);
}

/*!
 * @brief
 * Places the used buffer back on the virtqueue.
 *
 * @param vq   Virtqueue to use
 * @param len  Size of received buffer
 * @param idx  Index of buffer
 *
 */
static int vq_rx_free_remote(struct rpmsg_instance *ins, void *buffer, unsigned long len, unsigned short idx)
{
    int status;
    struct rpmsg_lite_instance *lite;

    lite = (struct rpmsg_lite_instance *)ins->priv;
#if defined(RL_CLEAR_USED_BUFFERS) && (RL_CLEAR_USED_BUFFERS == 1)
    env_memset(buffer, 0x00, len);
#endif
    len = (unsigned long)virtqueue_get_buffer_length(lite->rvq, idx);
    status = virtqueue_add_consumed_buffer(lite->rvq, idx, len);
    RL_ASSERT(status == VQUEUE_SUCCESS); /* must success here */
                                         /* As long as the length of the virtqueue ring buffer is not shorter
                                          * than the number of buffers in the pool, this function should not fail.
                                          * This condition is always met, so we don't need to return anything here */
    return RL_SUCCESS;
}

static struct rpmsg_lower_ops remote_lower_ops = {
    .tx        = vq_tx_remote,
    .tx_alloc  = vq_tx_alloc_remote,
    .rx        = vq_rx_remote,
    .rx_free   = vq_rx_free_remote,
};

/***************************************************************
   mmm    mm   m      m      mmmmm    mm     mmm  m    m  mmmm
 m"   "   ##   #      #      #    #   ##   m"   " #  m"  #"   "
 #       #  #  #      #      #mmmm"  #  #  #      #m#    "#mmm
 #       #mm#  #      #      #    #  #mm#  #      #  #m      "#
  "mmm" #    # #mmmmm #mmmmm #mmmm" #    #  "mmm" #   "m "mmm#"
****************************************************************/

/*!
 * @brief
 * Called when remote side calls virtqueue_kick()
 * at its transmit virtqueue.
 * In this callback, the buffer is read-out
 * of the rvq and user callback is called.
 *
 * @param vq  Virtqueue affected by the kick
 *
 */
static void rpmsg_lite_rx_callback(struct virtqueue *vq)
{
    struct rpmsg_std_msg *rpmsg_msg;
    unsigned long len;
    unsigned short idx;
    struct rpmsg_endpoint *ept;
    int cb_ret;
    struct rpmsg_hdr_reserved *rsvd;
    struct rpmsg_instance *ins;

    ins = (struct rpmsg_instance *)vq->priv;

    RL_ASSERT(ins != NULL);

    /* Process the received data from remote node */
    rpmsg_msg = (struct rpmsg_std_msg *)ins->lower_ops->rx(ins, &len, &idx);

    while (rpmsg_msg)
    {
        ept = rpmsg_get_ept(ins, rpmsg_msg->hdr.dst);
        cb_ret = RL_RELEASE;

        if (ept != RL_NULL)
        {
            cb_ret = ept->rx_cb(rpmsg_msg->data, rpmsg_msg->hdr.len, rpmsg_msg->hdr.src, ept->rx_cb_data);
        }

        if (cb_ret == RL_HOLD)
        {
            rsvd = (struct rpmsg_hdr_reserved *)&rpmsg_msg->hdr.reserved;
            rsvd->idx = idx;
        }
        else
        {
            ins->lower_ops->rx_free(ins, rpmsg_msg, len, idx);
        }

        rpmsg_msg = (struct rpmsg_std_msg *)ins->lower_ops->rx(ins, &len, &idx);
    }
}

/*!
 * @brief
 * Called when remote side calls virtqueue_kick()
 * at its receive virtqueue.
 *
 * @param vq  Virtqueue affected by the kick
 *
 */
#ifdef CONFIG_RPMSG_WITH_LK
static void rpmsg_lite_kick_schedule_work(void *arg)
{
    struct rpmsg_lite_instance *rpmsg_lite_dev;

    rpmsg_lite_dev = (struct rpmsg_lite_instance *)arg;
    virtqueue_kick(rpmsg_lite_dev->rvq);
}

static void rpmsg_lite_tx_callback(struct virtqueue *vq)
{
    bool kick = true;
    struct rpmsg_instance *ins;
    struct rpmsg_lite_instance *rpmsg_lite_dev;

    ins = (struct rpmsg_instance *)vq->priv;
    RL_ASSERT(ins != NULL);
    rpmsg_lite_dev = (struct rpmsg_lite_instance *)ins->priv;
    RL_ASSERT(rpmsg_lite_dev != NULL);

    ssdk_printf(SSDK_DEBUG, "rpmsg_lite_tx_callback: remote %d, cur-stat %d\n",
        RL_GET_LINK_ID(rpmsg_lite_dev->tvq->vq_queue_index), rpmsg_lite_dev->link_state);

    switch(rpmsg_lite_dev->link_state) {
        case RPMSG_LINKSTATE_DOWN:
            rpmsg_lite_dev->link_state = RPMSG_LINKSTATE_SYNC;
            break;

        case RPMSG_LINKSTATE_SYNC:
            rpmsg_lite_dev->link_state = RPMSG_LINKSTATE_SYNA;
            break;

        case RPMSG_LINKSTATE_SYNA:
            rpmsg_lite_dev->link_state = RPMSG_LINKSTATE_UP;
            break;

        case RPMSG_LINKSTATE_UP:
            kick = false;
            break;

        default:
            break;
    }

    if (rpmsg_lite_dev->is_master && kick) {
        work_queue(&rpmsg_lite_dev->work, rpmsg_lite_kick_schedule_work, rpmsg_lite_dev);
    }
}
#else
static void rpmsg_lite_tx_callback(struct virtqueue *vq)
{
    struct rpmsg_instance *ins;
    struct rpmsg_lite_instance *lite_ins;

    ins = (struct rpmsg_instance *)vq->priv;
    RL_ASSERT(ins != NULL);
    lite_ins = (struct rpmsg_lite_instance *)ins->priv;
    lite_ins->link_state = RPMSG_LINKSTATE_UP;
}
#endif

struct mbox_notify_msg {
    uint8_t device_type;
    uint8_t cc;
    uint8_t vq_id;
    uint8_t flags;
} __PACKED;

static int virtqueue_get_notified(void *arg, uint8_t *mssg, uint32_t len)
{
    struct virtqueue *vq;
    struct mbox_notify_msg *notify_msg;
    struct rpmsg_lite_instance *rpmsg_lite_dev =
        (struct rpmsg_lite_instance *)arg;

    /* Process the received data from remote node */
    notify_msg = (struct mbox_notify_msg *)mssg;

    if (rpmsg_lite_dev->is_master) {
        if (notify_msg->vq_id == 0) {
            vq = rpmsg_lite_dev->rvq;
        } else {
            vq = rpmsg_lite_dev->tvq;
        }
    } else {
        /* this for remote device */
        if (notify_msg->vq_id == 0) {
            vq = rpmsg_lite_dev->tvq;
        } else {
            vq = rpmsg_lite_dev->rvq;
        }
    }

    if (notify_msg && len) {
        virtqueue_notification(vq);
    }

    return 0;
}

static void virtqueue_notify(struct virtqueue *vq)
{
    platform_notify(vq->vq_queue_index);

    struct rpmsg_instance *ins;
    struct rpmsg_lite_instance *inst;
    struct mbox_notify_msg msg;
    struct mbox_chan *chan;
    uint32_t timeout = 100;

    ins = (struct rpmsg_instance *)vq->priv;
    inst = (struct rpmsg_lite_instance *)ins->priv;
    chan = (struct mbox_chan *)inst->notifier;

    msg.vq_id = RL_GET_Q_ID(vq->vq_queue_index);
    msg.device_type = inst->is_master ? 0x80 : 0x81;
    msg.cc = inst->counter++;

    mbox_send(chan, (uint8_t *)&msg, sizeof(struct mbox_notify_msg), timeout);
}

static int rpmsg_lite_master_init(struct rpmsg_instance *ins, void *cfg)
{
    int status;
    void (*callback[2])(struct virtqueue *vq);
    const char *vq_names[2];
    struct vring_alloc_info ring_info;
    struct virtqueue *vqs[2];
    void *buffer;
    int idx, j;
    struct rpmsg_lite_instance *rpmsg_lite_dev;
    struct rpmsg_lite_config *rpmsg_lite_cfg;

    rpmsg_lite_dev = (struct rpmsg_lite_instance *)ins->priv;
    rpmsg_lite_cfg = (struct rpmsg_lite_config *)cfg;

    if ((2 * RL_BUFFER_COUNT) > ((RL_WORD_ALIGN_DOWN(rpmsg_lite_cfg->shm_size - RL_VRING_OVERHEAD)) / RL_BUFFER_SIZE))
    {
        return RL_ERR_PARAM;
    }

    if (rpmsg_lite_cfg->remote_proc > RL_PLATFORM_HIGHEST_LINK_ID
        || rpmsg_lite_cfg->self_proc > RL_PLATFORM_HIGHEST_LINK_ID)
    {
        return RL_ERR_PARAM;
    }

    if (!rpmsg_lite_cfg->shm_phys_base)
    {
        return RL_ERR_PARAM;
    }

    env_memset(rpmsg_lite_dev, 0, sizeof(struct rpmsg_lite_instance));
    env_memcpy(&(rpmsg_lite_dev->config), rpmsg_lite_cfg,
               sizeof(struct rpmsg_lite_config));
    status = env_init();
    if (status != RL_SUCCESS)
    {
        return RL_ERRORS_BASE;
    }

    /*
     * Since device is RPMSG Remote so we need to manage the
     * shared buffers. Create shared memory pool to handle buffers.
     */
    rpmsg_lite_dev->sh_mem_base = (char *)RL_WORD_ALIGN_UP(rpmsg_lite_cfg->shm_phys_base + RL_VRING_OVERHEAD);
    rpmsg_lite_dev->sh_mem_remaining = (RL_WORD_ALIGN_DOWN(rpmsg_lite_cfg->shm_size - RL_VRING_OVERHEAD)) / RL_BUFFER_SIZE;
    rpmsg_lite_dev->sh_mem_total = rpmsg_lite_dev->sh_mem_remaining;

    /* Initialize names and callbacks*/
    vq_names[0] = "rx_vq";
    vq_names[1] = "tx_vq";
    callback[0] = rpmsg_lite_rx_callback;
    callback[1] = rpmsg_lite_tx_callback;

    /* Create virtqueue for each vring. */
    for (idx = 0; idx < 2; idx++)
    {
        ring_info.phy_addr = (void *)((unsigned long)rpmsg_lite_cfg->shm_phys_base + (unsigned long)((idx == 0) ? (0) : (VRING_SIZE)));
        ring_info.align = VRING_ALIGN;
        ring_info.num_descs = RL_BUFFER_COUNT;

        ssdk_printf(SSDK_DEBUG, "%s: idx %d, vring_size 0x%x\n", __func__, idx, vring_size(ring_info.num_descs, ring_info.align));

        env_memset((void *)ring_info.phy_addr, 0x00, vring_size(ring_info.num_descs, ring_info.align));

        status = virtqueue_create(RL_GET_VQ_ID(rpmsg_lite_cfg->remote_proc, idx), (char *)vq_names[idx], &ring_info, callback[idx],
                                  virtqueue_notify, &vqs[idx]);

        if (status == RL_SUCCESS)
        {
            /* Initialize vring control block in virtqueue. */
            vq_ring_init(vqs[idx]);

            /* Disable callbacks - will be enabled by the application
            * once initialization is completed.
            */
            virtqueue_disable_cb(vqs[idx]);
        }
        else
        {
            return RL_ERRORS_BASE;
        }

        /* virtqueue has reference to the RPMsg Lite instance */
        vqs[idx]->priv = (void *)ins;
    }

    // FIXME - a better way to handle this , tx for master is rx for remote and vice versa.
    rpmsg_lite_dev->tvq = vqs[1];
    rpmsg_lite_dev->rvq = vqs[0];

    for (j = 0; j < 2; j++)
    {
        for (idx = 0; ((idx < vqs[j]->vq_nentries) && (idx < rpmsg_lite_dev->sh_mem_total)); idx++)
        {
            /* Initialize TX virtqueue buffers for remote device */
            buffer = rpmsg_lite_dev->sh_mem_remaining ?
                         (rpmsg_lite_dev->sh_mem_base +
                          RL_BUFFER_SIZE * (rpmsg_lite_dev->sh_mem_total - rpmsg_lite_dev->sh_mem_remaining--)) :
                         (RL_NULL);

            RL_ASSERT(buffer);

            env_memset(buffer, 0x00, RL_BUFFER_SIZE);

            if (vqs[j] == rpmsg_lite_dev->rvq)
            {
                status = virtqueue_fill_avail_buffers(vqs[j], buffer, RL_BUFFER_SIZE);
            }
            else if (vqs[j] == rpmsg_lite_dev->tvq)
            {
                status = virtqueue_fill_used_buffers(vqs[j], buffer, RL_BUFFER_SIZE);
            }
            else
            {
                RL_ASSERT(0); /* should not happen */
            }

            if (status != RL_SUCCESS)
            {
                return RL_ERRORS_BASE;
            }
        }
    }

#ifdef CONFIG_RPMSG_WITH_LK
    spin_lock_init(&rpmsg_lite_dev->lock);
    start_workqueue_service();
#endif

    /* Install ISRs */
    platform_init_interrupt(rpmsg_lite_dev->rvq->vq_queue_index, rpmsg_lite_dev->rvq);
    platform_init_interrupt(rpmsg_lite_dev->tvq->vq_queue_index, rpmsg_lite_dev->tvq);
    env_disable_interrupt(rpmsg_lite_dev->rvq->vq_queue_index);
    env_disable_interrupt(rpmsg_lite_dev->tvq->vq_queue_index);
#if defined(CONFIG_KUNLUN_MBOX)
    rpmsg_lite_dev->link_state = RPMSG_LINKSTATE_SYNC;
#elif defined(CONFIG_TAISHAN_MBOX)
    rpmsg_lite_dev->link_state = RPMSG_LINKSTATE_UP;
#else
#error "No specify mbox"
#endif
    env_enable_interrupt(rpmsg_lite_dev->rvq->vq_queue_index);
    env_enable_interrupt(rpmsg_lite_dev->tvq->vq_queue_index);

    {
        struct mbox_chan *chan;
        struct sdrv_mbox_chan_req chan_req;

        chan_req.rproc = rpmsg_lite_cfg->remote_proc;

#ifdef CONFIG_KUNLUN_MBOX
        chan_req.src_addr = rpmsg_lite_cfg->src_addr;
        chan_req.dest_addr = rpmsg_lite_cfg->dst_addr;
#endif

#ifdef CONFIG_TAISHAN_MBOX
        chan_req.src_addr = MBOX_ADDR_RPMSG;
        chan_req.dest_addr = MBOX_ADDR_RPMSG;
#endif

        chan = mbox_request_channel(rpmsg_lite_cfg->mbox_ins, &chan_req);
        mbox_set_callback(chan, virtqueue_get_notified, (void *)rpmsg_lite_dev);

        rpmsg_lite_dev->notifier = (void *)chan;
        rpmsg_lite_dev->is_master = true;
        rpmsg_lite_dev->counter = 0;
    }

#ifdef CONFIG_RPMSG_WITH_LK
    /*
     * Let the remote device know that Master is ready for
     * communication.
     */
    // virtqueue_kick(rpmsg_lite_dev->rvq);
#endif

    return RL_SUCCESS;
}

static int rpmsg_lite_remote_init(struct rpmsg_instance *ins, void *cfg)
{
    int status;
    void (*callback[2])(struct virtqueue *vq);
    const char *vq_names[2];
    struct vring_alloc_info ring_info;
    struct virtqueue *vqs[2];
    int idx;
    struct rpmsg_lite_instance *rpmsg_lite_dev;
    struct rpmsg_lite_config *rpmsg_lite_cfg;

    rpmsg_lite_dev = (struct rpmsg_lite_instance *)ins->priv;
    rpmsg_lite_cfg = (struct rpmsg_lite_config *)cfg;

    if (rpmsg_lite_cfg->remote_proc > RL_PLATFORM_HIGHEST_LINK_ID
        || rpmsg_lite_cfg->self_proc > RL_PLATFORM_HIGHEST_LINK_ID)
    {
        return RL_ERR_PARAM;
    }

    if (!rpmsg_lite_cfg->shm_phys_base)
    {
        return RL_ERR_PARAM;
    }

    env_memset(rpmsg_lite_dev, 0, sizeof(struct rpmsg_lite_instance));

    status = env_init();
    if (status != RL_SUCCESS)
    {
        return RL_ERRORS_BASE;
    }

    vq_names[0] = "tx_vq"; /* swapped in case of remote */
    vq_names[1] = "rx_vq";
    callback[0] = rpmsg_lite_tx_callback;
    callback[1] = rpmsg_lite_rx_callback;
    env_memcpy(&(rpmsg_lite_dev->config), rpmsg_lite_cfg,
               sizeof(struct rpmsg_lite_config));
    rpmsg_lite_dev->sh_mem_base = (char *)RL_WORD_ALIGN_UP(
        rpmsg_lite_cfg->shm_phys_base + RL_VRING_OVERHEAD);

    /* Create virtqueue for each vring. */
    for (idx = 0; idx < 2; idx++)
    {
        ring_info.phy_addr = (void *)((unsigned long)rpmsg_lite_cfg->shm_phys_base + (unsigned long)((idx == 0) ? (0) : (VRING_SIZE)));
        ring_info.align = VRING_ALIGN;
        ring_info.num_descs = RL_BUFFER_COUNT;

        status = virtqueue_create(RL_GET_VQ_ID(rpmsg_lite_cfg->remote_proc, idx), (char *)vq_names[idx], &ring_info, callback[idx],
                                  virtqueue_notify, &vqs[idx]);
        if (status != RL_SUCCESS)
        {
            return RL_ERRORS_BASE;
        }

        /* virtqueue has reference to the RPMsg Lite instance */
        vqs[idx]->priv = (void *)ins;
    }

    // FIXME - a better way to handle this , tx for master is rx for remote and vice versa.
    rpmsg_lite_dev->tvq = vqs[0];
    rpmsg_lite_dev->rvq = vqs[1];

    {
        struct mbox_chan *chan;
        struct sdrv_mbox_chan_req chan_req;

        chan_req.rproc = rpmsg_lite_cfg->remote_proc;
#ifdef CONFIG_KUNLUN_MBOX
        chan_req.src_addr = rpmsg_lite_cfg->src_addr;
        chan_req.dest_addr = rpmsg_lite_cfg->dst_addr;
#endif
#ifdef CONFIG_TAISHAN_MBOX
        chan_req.src_addr = MBOX_ADDR_RPMSG;
        chan_req.dest_addr = MBOX_ADDR_RPMSG;
#endif
        chan = mbox_request_channel(rpmsg_lite_cfg->mbox_ins, &chan_req);
        mbox_set_callback(chan, virtqueue_get_notified, (void *)rpmsg_lite_dev);

        rpmsg_lite_dev->notifier = (void *)chan;
        rpmsg_lite_dev->is_master = false;
        rpmsg_lite_dev->counter = 0;
    }

#ifdef CONFIG_RPMSG_WITH_LK
    spin_lock_init(&rpmsg_lite_dev->lock);
#endif

    /* Install ISRs */
    platform_init_interrupt(rpmsg_lite_dev->rvq->vq_queue_index, rpmsg_lite_dev->rvq);
    platform_init_interrupt(rpmsg_lite_dev->tvq->vq_queue_index, rpmsg_lite_dev->tvq);
    env_disable_interrupt(rpmsg_lite_dev->rvq->vq_queue_index);
    env_disable_interrupt(rpmsg_lite_dev->tvq->vq_queue_index);
#if defined(CONFIG_KUNLUN_MBOX)
    rpmsg_lite_dev->link_state = RPMSG_LINKSTATE_DOWN;
#elif defined(CONFIG_TAISHAN_MBOX)
    rpmsg_lite_dev->link_state = RPMSG_LINKSTATE_UP;
#else
#error "No specify mbox"
#endif
    env_enable_interrupt(rpmsg_lite_dev->rvq->vq_queue_index);
    env_enable_interrupt(rpmsg_lite_dev->tvq->vq_queue_index);

    return RL_SUCCESS;
}

static int rpmsg_lite_deinit(struct rpmsg_instance *ins)
{
    struct rpmsg_lite_instance *rpmsg_lite_dev;

    rpmsg_lite_dev = (struct rpmsg_lite_instance *)ins->priv;

    if (!((rpmsg_lite_dev->rvq != NULL) && (rpmsg_lite_dev->tvq != NULL)))
    {
        /* ERROR - trying to initialize uninitialized RPMSG? */
        RL_ASSERT((rpmsg_lite_dev->rvq != NULL) && (rpmsg_lite_dev->tvq != NULL));
        return RL_ERR_PARAM;
    }

    rpmsg_lite_dev->link_state = RPMSG_LINKSTATE_DOWN;

    env_disable_interrupt(rpmsg_lite_dev->rvq->vq_queue_index);
    env_disable_interrupt(rpmsg_lite_dev->tvq->vq_queue_index);
    env_enable_interrupt(rpmsg_lite_dev->rvq->vq_queue_index);
    env_enable_interrupt(rpmsg_lite_dev->tvq->vq_queue_index);
    platform_deinit_interrupt(rpmsg_lite_dev->rvq->vq_queue_index);
    platform_deinit_interrupt(rpmsg_lite_dev->tvq->vq_queue_index);

    mbox_release_channel((struct mbox_chan *)rpmsg_lite_dev->notifier);

    virtqueue_free(rpmsg_lite_dev->rvq);
    virtqueue_free(rpmsg_lite_dev->tvq);
    env_deinit();

    return RL_SUCCESS;
}

static int rpmsg_lite_is_master(struct rpmsg_instance *ins)
{
    struct rpmsg_lite_instance *rpmsg_lite_dev;

    rpmsg_lite_dev = (struct rpmsg_lite_instance *)ins->priv;
    return (int)(rpmsg_lite_dev->is_master);
}

static int rpmsg_lite_get_buffer_num(struct rpmsg_instance *ins)
{
    return RL_BUFFER_COUNT;
}

static int rpmsg_lite_get_payload_size(struct rpmsg_instance *ins)
{
    return RL_BUFFER_PAYLOAD_SIZE;
}

static int rpmsg_lite_get_link_status(struct rpmsg_instance *ins)
{
    struct rpmsg_lite_instance *rpmsg_lite_dev;

    rpmsg_lite_dev = (struct rpmsg_lite_instance *)ins->priv;
    return rpmsg_lite_dev ? (int)(rpmsg_lite_dev->link_state) : RPMSG_LINKSTATE_ERRSTATE;
}

#ifdef CONFIG_RPMSG_WITH_LK
static int rpmsg_lite_kick_remote(struct rpmsg_instance *ins)
{
    struct rpmsg_lite_instance *rpmsg_lite_dev;
    irq_state_t irq_state;

    rpmsg_lite_dev = (struct rpmsg_lite_instance *)ins->priv;
    irq_state = spin_lock_irqsave(&rpmsg_lite_dev->lock);

    ssdk_printf(SSDK_DEBUG, "rpmsg_lite_kick_remote: remote %d, cur-stat %d\n",
        RL_GET_LINK_ID(rpmsg_lite_dev->tvq->vq_queue_index), rpmsg_lite_dev->link_state);

    switch(rpmsg_lite_dev->link_state) {
        case RPMSG_LINKSTATE_DOWN:
            /* First kickoff from down */
            rpmsg_lite_dev->link_state = RPMSG_LINKSTATE_SYNC;
            break;

        case RPMSG_LINKSTATE_SYNA:
            /* Since we've already got sync ack, fall back to link up safely */
            rpmsg_lite_dev->link_state = RPMSG_LINKSTATE_UP;
            break;

        case RPMSG_LINKSTATE_SYNC:
            break;

        case RPMSG_LINKSTATE_UP:
            break;

        default:
            break;
    }

    spin_unlock_irqrestore(&rpmsg_lite_dev->lock, irq_state);
    virtqueue_kick(rpmsg_lite_dev->rvq);
    return 0;
}

static int rpmsg_lite_set_link_status(struct rpmsg_instance *ins, int32_t status)
{
    struct rpmsg_lite_instance *rpmsg_lite_dev;
    irq_state_t irq_state;

    rpmsg_lite_dev = (struct rpmsg_lite_instance *)ins->priv;
    irq_state = spin_lock_irqsave(&rpmsg_lite_dev->lock);
    rpmsg_lite_dev->link_state = status;
    spin_unlock_irqrestore(&rpmsg_lite_dev->lock, irq_state);

    return 0;
}
#endif

int rpmsg_lite_remote_reinit(struct rpmsg_instance *ins)
{
    int status;
    void (*callback[2])(struct virtqueue * vq);
    const char *vq_names[2];
    struct vring_alloc_info ring_info;
    struct virtqueue *vqs[2];
    int idx;
    int link_id;
    struct rpmsg_lite_instance *rpmsg_lite_dev;
    rpmsg_lite_dev = (struct rpmsg_lite_instance *)ins->priv;

    virtqueue_free(rpmsg_lite_dev->rvq);
    virtqueue_free(rpmsg_lite_dev->tvq);
    rpmsg_lite_dev->tvq = NULL;
    rpmsg_lite_dev->rvq = NULL;

    vq_names[0] = "tx_vq"; /* swapped in case of remote */
    vq_names[1] = "rx_vq";
    callback[0] = rpmsg_lite_tx_callback;
    callback[1] = rpmsg_lite_rx_callback;
    link_id = rpmsg_lite_dev->config.remote_proc;

    /* Create virtqueue for each vring. */
    for (idx = 0; idx < 2; idx++) {
        ring_info.phy_addr =
            (void *)((unsigned long)rpmsg_lite_dev->config.shm_phys_base +
                     (unsigned long)((idx == 0) ? (0) : (VRING_SIZE)));
        ring_info.align = VRING_ALIGN;
        ring_info.num_descs = RL_BUFFER_COUNT;

        status = virtqueue_create(RL_GET_VQ_ID(link_id, idx),
                                  (char *)vq_names[idx], &ring_info,
                                  callback[idx], virtqueue_notify, &vqs[idx]);

        if (status != RL_SUCCESS) {
            return status;
        }

        /* virtqueue has reference to the RPMsg Lite instance */
        vqs[idx]->priv = (void *)ins;
    }

    rpmsg_lite_dev->tvq = vqs[0];
    rpmsg_lite_dev->rvq = vqs[1];

#if defined(CONFIG_KUNLUN_MBOX)
    rpmsg_lite_dev->link_state = RPMSG_LINKSTATE_DOWN;
#elif defined(CONFIG_TAISHAN_MBOX)
    rpmsg_lite_dev->link_state = RPMSG_LINKSTATE_UP;
#else
#error "No specify mbox"
#endif

    return 0;
}

static struct rpmsg_instance_ops ins_ops = {
    .master_init      = rpmsg_lite_master_init,
    .remote_init      = rpmsg_lite_remote_init,
    .deinit           = rpmsg_lite_deinit,
    .is_master        = rpmsg_lite_is_master,
    .get_buffer_num   = rpmsg_lite_get_buffer_num,
    .get_payload_size = rpmsg_lite_get_payload_size,
    .get_link_status  = rpmsg_lite_get_link_status,
#ifdef CONFIG_RPMSG_WITH_LK
    .kick_remote      = rpmsg_lite_kick_remote,
    .set_link_status  = rpmsg_lite_set_link_status,
#endif
};

int rpmsg_lite_probe(struct rpmsg_instance *rpmsg,
                     struct rpmsg_lite_instance *lite,
                     struct rpmsg_lite_config *cfg)
{
    int ret;

    rpmsg->ins_ops = &ins_ops;
    rpmsg->priv = (void *)lite;
    rpmsg->rpmsg_type = RPMSG_TYPE_VIRTIO;
    rpmsg->remote_proc = cfg->remote_proc;

    if (cfg->is_master == RL_TRUE) {
        rpmsg->lower_ops = &master_lower_ops;
        ret = rpmsg_master_init(rpmsg, (void *)cfg);
    }
    else {
        rpmsg->lower_ops = &remote_lower_ops;
        ret = rpmsg_remote_init(rpmsg, (void *)cfg);
    }

    return ret;
}
