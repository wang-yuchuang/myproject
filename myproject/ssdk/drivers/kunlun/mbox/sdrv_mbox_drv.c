/**
 * @file sdrv_mbox_drv.c
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: mbox driver.
 *
 * Revision History:
 * -----------------
 */

#include <types.h>
#include <string.h>

#include <arch/barriers.h>
#include <arch/cache.h>
#include "driver/kunlun/mbox/sdrv_mbox_drv.h"
#include "sdrv_mbox.h"
#include <driver/irq.h>
#include <bits.h>
#include <spinlock.h>
#include <debug.h>
#include <reg.h>

#define SDRV_MBOX_DEBUG_LVL      SSDK_WARNING

#define EXTRA_FMT   "sdrv_mbox:"

#if SDRV_MBOX_DEBUG_LVL >= SSDK_ERR
#define SDRV_MBOX_ERR(format, ...) \
    ssdk_printf(SSDK_ERR, EXTRA_FMT format, ##__VA_ARGS__)
#else
#define SDRV_MBOX_ERR(format, ...)
#endif

#if SDRV_MBOX_DEBUG_LVL >= SSDK_WARNING
#define SDRV_MBOX_WARN(format, ...) \
    ssdk_printf(SSDK_WARNING, EXTRA_FMT format, ##__VA_ARGS__)
#else
#define SDRV_MBOX_WARN(format, ...)
#endif

#if SDRV_MBOX_DEBUG_LVL >= SSDK_INFO
#define SDRV_MBOX_INFO(format, ...) \
    ssdk_printf(SSDK_INFO, EXTRA_FMT format, ##__VA_ARGS__)
#else
#define SDRV_MBOX_INFO(format, ...)
#endif

static void sdrv_mbox_init(mbox_dev_t *dev);
static bool sdrv_mbox_startup(mbox_chan_t *chan, void *priv_data);
static void sdrv_mbox_shutdown(mbox_chan_t *chan);
static int sdrv_mbox_send(mbox_chan_t *chan, uint8_t *data, uint32_t len);
static uint8_t *sdrv_mbox_alloc_buffer(mbox_chan_t *chan, uint32_t len);
static int sdrv_mbox_send_nocopy(mbox_chan_t *chan, uint8_t *data, uint32_t len);
static int sdrv_mbox_send_nobuf(mbox_chan_t *chan, uint8_t *data, uint32_t len);
static void sdrv_mbox_cancel_send(mbox_chan_t *chan);
static bool sdrv_mbox_tx_done(mbox_chan_t *chan);

static mbox_ops_t g_sdrv_mbox_ops = {
    .init = sdrv_mbox_init,
    .startup = sdrv_mbox_startup,
    .shutdown = sdrv_mbox_shutdown,
    .send = sdrv_mbox_send,
    .alloc_buf = sdrv_mbox_alloc_buffer,
    .send_nocopy = sdrv_mbox_send_nocopy,
    .send_nobuf = sdrv_mbox_send_nobuf,
    .cancel_send = sdrv_mbox_cancel_send,
    .tx_done = sdrv_mbox_tx_done,
};

/**
 * @brief sdrv mbox alloc message
 *
 * @param[in] sdrv_mbox sdrv mbox
 * return alloc msg id
 */
static int sdrv_mbox_alloc_message(sdrv_mbox_dev_t *sdrv_mbox)
{
    uint8_t msg_id;

    for (msg_id = 0; msg_id < SDRV_MBOX_MSG_NUM; msg_id++) {
        if (!BIT_SET(sdrv_mbox->msg_bitmap, msg_id)) {
            sdrv_mbox->msg_bitmap |= (1 << msg_id);
            return msg_id;
        }
    }

    return -1;
}

/**
 * @brief sdrv mbox free message
 *
 * @param[in] sdrv_mbox sdrv mbox
 * @param[in] msg_id msg id
 */
static void sdrv_mbox_free_message(sdrv_mbox_dev_t *sdrv_mbox, uint8_t msg_id)
{
    sdrv_mbox->msg_bitmap &= ~(1 << msg_id);
}

/**
 * @brief sdrv mbox receive data
 *
 * @param[in] dev mbox dev
 * @param[in] rproc rproc id
 * @param[in] msg_id msg id
 */
static void sdrv_mbox_receive(mbox_dev_t *dev, uint8_t rproc, uint8_t msg_id)
{
    sdrv_mbox_dev_t *sdrv_mbox = dev->priv;
    sdrv_mbox_chan_t *sdrv_chan;
    uint64_t short_data;
    uint16_t mb_len;
    uint8_t mbm;
    uint8_t *rx_buf = NULL;
    int chan_id;

    short_data = sdrv_mbox_lld_get_short_msg(dev->base, rproc, msg_id);

    mb_len = sdrv_mbox_lld_get_msg_head(dev->base, rproc, msg_id, &mbm);

    if (mbm != 0) {

#if CONFIG_MBOX_FLUSH_CACHE
        rx_buf = sdrv_mbox_lld_get_rxbuf(dev->mem_base, rproc, msg_id);
        arch_invalidate_cache_range((addr_t)rx_buf, mb_len);
#else
        rx_buf = sdrv_mbox_lld_get_rxbuf(dev->base, rproc, msg_id);
#endif

    }

    for (chan_id = 0; chan_id < SDRV_MBOX_CHN_NUM; chan_id++) {
        sdrv_chan = &sdrv_mbox->priv[chan_id];
        if (sdrv_chan->used == true && sdrv_chan->rproc == rproc &&
            (sdrv_chan->src_addr == (uint16_t)SDRV_MBOX_ANY_ADDR ||
             sdrv_chan->src_addr == (short_data >> 32))) {
            if (rx_buf) {
                sdrv_mbox_hdr_t *header = (sdrv_mbox_hdr_t *)rx_buf;
                ASSERT(header->data_len >= sizeof(sdrv_mbox_hdr_t));
                mbox_received(&dev->chan[chan_id], header->data,
                              header->data_len - sizeof(sdrv_mbox_hdr_t));
            }
            else {
                mbox_received(&dev->chan[chan_id], (uint8_t *)&short_data, mb_len);
            }
        }
    }
}

/**
 * @brief sdrv mbox channel irq handler
 *
 * @param[in] irq irq number
 * @param[in] arg irq arg
 * return irq handler result
 */
static int sdrv_mbox_irq_handler(uint32_t irq, void *arg)
{
    mbox_dev_t *dev = arg;
    sdrv_mbox_dev_t *sdrv_mbox;
    irq_state_t flag;
    uint32_t state;
    uint8_t rproc, msg_mask, msg_id;

    ASSERT(dev);

    sdrv_mbox = dev->priv;

    flag = spin_lock_irqsave(&sdrv_mbox->lock);

    state = sdrv_mbox_lld_get_status(dev->base);

    for (rproc = 0; rproc < SDRV_MBOX_RPROC_NUM; rproc++) {
        msg_mask = 0xf & (state >> (rproc << 2));

        while (msg_mask) {
            msg_id = __builtin_ffs(msg_mask) - 1;
            msg_mask &= (msg_mask - 1);

            SDRV_MBOX_INFO("recv: %d msg: %d\n", rproc, msg_id);
            sdrv_mbox_receive(dev, rproc, msg_id);
            sdrv_mbox_lld_ack(dev->base, rproc, msg_id);
            SDRV_MBOX_INFO("ack: %d msg: %d\n", rproc, msg_id);
        }
    }

    spin_unlock_irqrestore(&sdrv_mbox->lock, flag);

    return 0;
}

/**
 * @brief sdrv mbox channel send short data
 *
 * @param[in] chan mbox channel
 * @param[in] data data ptr
 * @param[in] len data len
 * return send result
 */
static int sdrv_mbox_send_nobuf(mbox_chan_t *chan, uint8_t *data, uint32_t len)
{
    mbox_dev_t *dev = chan->dev;
    sdrv_mbox_dev_t *sdrv_mbox = dev->priv;
    sdrv_mbox_chan_t *sdrv_chan = chan->priv;
    uint64_t short_msg = 0;
    irq_state_t flag;
    int msg_id;

    if (len > SDRV_MBOX_SHORT_BUF_LEN) {
        SDRV_MBOX_WARN("send short message length too large\n");
        return -1;
    }

    flag = spin_lock_irqsave(&sdrv_mbox->lock);

    msg_id = sdrv_mbox_alloc_message(sdrv_mbox);
    if (msg_id < 0) {
        spin_unlock_irqrestore(&sdrv_mbox->lock, flag);
        SDRV_MBOX_WARN("no message buffer\n");
        return -1;
    }

    sdrv_chan->msg_id = (uint8_t)msg_id;

    sdrv_mbox_lld_fill_msg_head(dev->base, sdrv_chan->msg_id,
                                (1 << sdrv_chan->rproc), 0, len);

    if ((data != NULL) && (len != 0)) {
        for (uint8_t i = 0; i < len; i++) {
            short_msg |= ((uint64_t) data[i] << (i * 8));
        }
    }

    sdrv_mbox_lld_fill_short_msg(dev->base, short_msg);

    sdrv_mbox_lld_send(dev->base, sdrv_chan->msg_id);

    spin_unlock_irqrestore(&sdrv_mbox->lock, flag);

    return 0;
}

/**
 * @brief sdrv mbox channel send data use message buf
 *
 * @param[in] chan mbox channel
 * @param[in] data data ptr
 * @param[in] len data len
 * return send result
 */
static int sdrv_mbox_send(mbox_chan_t *chan, uint8_t *data, uint32_t len)
{
    mbox_dev_t *dev = chan->dev;
    sdrv_mbox_dev_t *sdrv_mbox = dev->priv;
    sdrv_mbox_chan_t *sdrv_chan = chan->priv;
    uint32_t actual_len = len + sizeof(sdrv_mbox_hdr_t);
    sdrv_mbox_hdr_t *header;
    irq_state_t flag;
    int msg_id;

    if (actual_len > SDRV_MBOX_BANK_LEN) {
        SDRV_MBOX_WARN("send message length too large\n");
        return -1;
    }

    flag = spin_lock_irqsave(&sdrv_mbox->lock);

    msg_id = sdrv_mbox_alloc_message(sdrv_mbox);
    if (msg_id < 0) {
        spin_unlock_irqrestore(&sdrv_mbox->lock, flag);
        SDRV_MBOX_WARN("no message buffer\n");
        return ERR_NO_HW_CHAN;
    }

    sdrv_chan->msg_id = (uint8_t)msg_id;

    sdrv_mbox_lld_fill_msg_head(dev->base, sdrv_chan->msg_id,
                                (1 << sdrv_chan->rproc),
                                (1 << sdrv_chan->msg_id), actual_len);

    sdrv_mbox_lld_fill_short_msg(dev->base,
            (((uint64_t)sdrv_chan->dest_addr << 32) + sdrv_chan->src_addr));

#if CONFIG_MBOX_FLUSH_CACHE
    header = (sdrv_mbox_hdr_t *)sdrv_mbox_lld_get_txbuf(dev->mem_base, sdrv_chan->msg_id);
#else
    header = (sdrv_mbox_hdr_t *)sdrv_mbox_lld_get_txbuf(dev->base, sdrv_chan->msg_id);
#endif
    /* ssdk only support PRMSG protocol */
    header->protocol = SDRV_MBOX_PROTO_RPMSG;
    header->rproc = sdrv_chan->rproc;
    header->addr = sdrv_chan->dest_addr;
    header->data_len = actual_len;
    if ((data != NULL) && (len != 0))
        memcpy_toio(header->data, data, len);

#if CONFIG_MBOX_FLUSH_CACHE
    arch_clean_cache_range((addr_t)header, actual_len);
#endif

    sdrv_mbox_lld_send(dev->base, sdrv_chan->msg_id);

    spin_unlock_irqrestore(&sdrv_mbox->lock, flag);

    return 0;
}

/**
 * @brief sdrv mbox channel alloc buffer
 *
 * @param[in] chan mbox channel
 * @param[in] len data len
 * return buffer address
 */
static uint8_t *sdrv_mbox_alloc_buffer(mbox_chan_t *chan, uint32_t len)
{
    mbox_dev_t *dev = chan->dev;
    sdrv_mbox_dev_t *sdrv_mbox = dev->priv;
    sdrv_mbox_chan_t *sdrv_chan = chan->priv;
    uint32_t actual_len = len + sizeof(sdrv_mbox_hdr_t);
    sdrv_mbox_hdr_t *header;
    irq_state_t flag;
    int msg_id;

    if (actual_len > SDRV_MBOX_BANK_LEN) {
        SDRV_MBOX_WARN("send message length too large\n");
        return NULL;
    }

    flag = spin_lock_irqsave(&sdrv_mbox->lock);

    msg_id = sdrv_mbox_alloc_message(sdrv_mbox);
    if (msg_id < 0) {
        spin_unlock_irqrestore(&sdrv_mbox->lock, flag);
        SDRV_MBOX_WARN("no message buffer\n");
        return NULL;
    }

    sdrv_chan->msg_id = (uint8_t)msg_id;

    sdrv_mbox_lld_fill_msg_head(dev->base, sdrv_chan->msg_id,
                                (1 << sdrv_chan->rproc),
                                (1 << sdrv_chan->msg_id), actual_len);

    sdrv_mbox_lld_fill_short_msg(dev->base,
            (((uint64_t)sdrv_chan->dest_addr << 32) + sdrv_chan->src_addr));

#if CONFIG_MBOX_FLUSH_CACHE
    header = (sdrv_mbox_hdr_t *)sdrv_mbox_lld_get_txbuf(dev->mem_base, sdrv_chan->msg_id);
#else
    header = (sdrv_mbox_hdr_t *)sdrv_mbox_lld_get_txbuf(dev->base, sdrv_chan->msg_id);
#endif

    header->rproc = sdrv_chan->rproc;
    header->addr = sdrv_chan->dest_addr;
    header->data_len = actual_len;

#if CONFIG_MBOX_FLUSH_CACHE
    arch_clean_cache_range(header, sizeof(sdrv_mbox_hdr_t));
#endif

    spin_unlock_irqrestore(&sdrv_mbox->lock, flag);

    return header->data;
}

/**
 * @brief sdrv mbox channel send data no copy
 *
 * @param[in] chan mbox channel
 * @param[in] data data ptr
 * @param[in] len data len
 * return send result
 */
static int sdrv_mbox_send_nocopy(mbox_chan_t *chan, uint8_t *data, uint32_t len)
{
    mbox_dev_t *dev = chan->dev;
    sdrv_mbox_dev_t *sdrv_mbox = dev->priv;
    sdrv_mbox_chan_t *sdrv_chan = chan->priv;
    irq_state_t flag;

    flag = spin_lock_irqsave(&sdrv_mbox->lock);

#if CONFIG_MBOX_FLUSH_CACHE
    if ((data != NULL) && (len != 0))
        arch_clean_cache_range((addr_t)data, len);
#endif

    sdrv_mbox_lld_send(dev->base, sdrv_chan->msg_id);

    spin_unlock_irqrestore(&sdrv_mbox->lock, flag);

    return 0;
}

/**
 * @brief check sdrv mbox channel tx done
 *
 * @param[in] chan mbox channel
 * return tx done result
 */
static bool sdrv_mbox_tx_done(mbox_chan_t *chan)
{
    mbox_dev_t *dev = chan->dev;
    sdrv_mbox_dev_t *sdrv_mbox = dev->priv;
    sdrv_mbox_chan_t *sdrv_chan = chan->priv;
    irq_state_t flag;
    bool ret;

    ret = sdrv_mbox_lld_tx_done(dev->base, sdrv_chan->msg_id);
    if (ret) {
        flag = spin_lock_irqsave(&sdrv_mbox->lock);
        sdrv_mbox_free_message(sdrv_mbox, sdrv_chan->msg_id);
        spin_unlock_irqrestore(&sdrv_mbox->lock, flag);
    }

    return ret;
}

/**
 * @brief sdrv mbox channel cancel send
 *
 * @param[in] chan mbox channel
 */
static void sdrv_mbox_cancel_send(mbox_chan_t *chan)
{
    mbox_dev_t *dev = chan->dev;
    sdrv_mbox_dev_t *sdrv_mbox = dev->priv;
    sdrv_mbox_chan_t *sdrv_chan = chan->priv;
    irq_state_t flag;

    sdrv_mbox_lld_cancel_send(dev->base, sdrv_chan->msg_id);

    flag = spin_lock_irqsave(&sdrv_mbox->lock);
    sdrv_mbox_free_message(sdrv_mbox, sdrv_chan->msg_id);
    spin_unlock_irqrestore(&sdrv_mbox->lock, flag);
}

/**
 * @brief sdrv mbox channel startup
 *
 * @param[in] chan mbox channel
 * @param[in] priv_data startup data
 * return startup result
 */
static bool sdrv_mbox_startup(mbox_chan_t *chan, void *priv_data)
{
    sdrv_mbox_chan_req_t *chan_req = priv_data;
    sdrv_mbox_chan_t *sdrv_chan = chan->priv;

    if (chan_req->rproc >= SDRV_MBOX_RPROC_NUM || sdrv_chan == NULL) {
        return false;
    }

    sdrv_chan->rproc = chan_req->rproc;
    sdrv_chan->src_addr = chan_req->src_addr;
    sdrv_chan->dest_addr = chan_req->dest_addr;
    sdrv_chan->used = true;

    return true;
}

/**
 * @brief sdrv mbox channel shutdown
 *
 * @param[in] chan mbox channel
 */
static void sdrv_mbox_shutdown(mbox_chan_t *chan)
{
    sdrv_mbox_chan_t *sdrv_chan = chan->priv;

    sdrv_chan->used = false;
}

/**
 * @brief sdrv mbox init
 *
 * @param[in] dev mbox dev
 */
static void sdrv_mbox_init(mbox_dev_t *dev)
{
    sdrv_mbox_dev_t *sdrv_mbox = dev->priv;

    spin_lock_init(&sdrv_mbox->lock);
    sdrv_mbox->msg_bitmap = 0;

    sdrv_mbox_lld_init(dev->base, 0);

    sdrv_mbox_lld_buff_clr(dev->base + SDRV_MBOX_TX_BUF_DIFF, SDRV_MBOX_BUF_LEN);

    if (dev->irq >= 0) {
        irq_attach(dev->irq, sdrv_mbox_irq_handler, dev);
        irq_enable(dev->irq);
    }
}

/**
 * @brief sdrv mbox probe
 *
 * @param[in] sdrv_mbox sdrv mbox dev
 * @param[in] cfg mbox config
 * return mbox dev
 */
mbox_dev_t *sdrv_mbox_probe(sdrv_mbox_dev_t *sdrv_mbox,
                            const mbox_config_t *cfg)
{
    mbox_dev_t *dev;
    uint8_t cnt;

    dev = &sdrv_mbox->dev;

    dev->priv = sdrv_mbox;
    dev->ops = &g_sdrv_mbox_ops;

    dev->chan_num = SDRV_MBOX_CHN_NUM;
    dev->chan = sdrv_mbox->chan;
    for (cnt = 0; cnt < dev->chan_num; cnt++) {
        dev->chan[cnt].priv = &sdrv_mbox->priv[cnt];
    }

    dev->mtu = SDRV_MBOX_MTU;

    mbox_init(dev, cfg);

    return dev;
}
