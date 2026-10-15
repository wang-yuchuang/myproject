/**
 * @file mbox.c
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: mailbox devices.
 *
 * Revision History:
 * -----------------
 */
#include <types.h>

#include <reg.h>
#include <board.h>
#include <driver/irq.h>
#include <driver/mbox.h>
#include <debug.h>

/**
 * @brief mbox wait tx complete
 *
 * @param[in] chan mbox channel
 * @param[in] timeout wait timeout
 * @return complete result
 */
static int mbox_wait_tx_complete(mbox_chan_t *chan, uint32_t timeout)
{
    if (chan == NULL) {
        return -1;
    }
    mbox_dev_t *dev = chan->dev;
    uint32_t wait_time = 0;
    uint32_t usec_cnt = 1000;

    do {
        if (usec_cnt >= CONFIG_MBOX_TX_POLL) {
            usec_cnt -= CONFIG_MBOX_TX_POLL;
            udelay(CONFIG_MBOX_TX_POLL);
            wait_time += CONFIG_MBOX_TX_POLL;
        }
        else {
            osDelay(1);
            wait_time += 1000;
        }
        if (wait_time >= timeout * 1000) {
            return -1;
        }
    }  while (!dev->ops->tx_done(chan));

    return 0;
}

/**
 * @brief mbox register
 *
 * @param[in] dev mbox dev
 * @param[in] cfg mbox config
 */
void mbox_init(mbox_dev_t *dev, const mbox_config_t *cfg)
{
    uint8_t cnt;

    dev->base = cfg->base;
    dev->mem_base = cfg->mem_base;
    dev->irq = cfg->irq;
    dev->set_masterid_num = cfg->set_masterid_num;

    dev->mutex = osMutexNew(NULL);

    for (cnt = 0; cnt < dev->chan_num; cnt++) {
        mbox_chan_t *chan = &dev->chan[cnt];
        chan->dev = dev;
        chan->used = false;
    }

    if (dev->ops->init)
        dev->ops->init(dev);
}

/**
 * @brief mbox get mtu
 *
 * @param[in] dev mbox dev
 * @return mtu
 */
uint32_t mbox_mtu(mbox_dev_t *dev)
{
    if (dev == NULL)
	return 0;
    return dev->mtu;
}

/**
 * @brief mbox client request channel
 *
 * @param[in] dev mbox dev
 * @param[in] priv_data priv data for special driver
 * @return mbox channel
 */
mbox_chan_t *mbox_request_channel(mbox_dev_t *dev, void *priv_data)
{
    if (dev == NULL || priv_data == NULL) {
        return NULL;
    }

    mbox_chan_t *find_chan = NULL;
    uint8_t cnt;

    osMutexAcquire(dev->mutex, osWaitForever);

    for (cnt = 0; cnt < dev->chan_num; cnt++) {
        mbox_chan_t *chan = &dev->chan[cnt];
        if (!chan->used) {
            if (dev->ops->startup)
                dev->ops->startup(chan, priv_data);

            chan->callback = NULL;
            chan->arg = NULL;
            chan->mutex = osMutexNew(NULL);
            chan->used = true;
            find_chan = chan;
            break;
        }
    }

    osMutexRelease(dev->mutex);
    return find_chan;
}

/**
 * @brief mbox client release channel
 *
 * @param[in] client mbox client
 */
void mbox_release_channel(mbox_chan_t *chan)
{
    if (chan == NULL) {
        return;
    }

    mbox_dev_t *dev = chan->dev;

    osMutexAcquire(dev->mutex, osWaitForever);

    if (dev->ops->shutdown)
        dev->ops->shutdown(chan);

    chan->callback = NULL;
    chan->arg = NULL;
    osMutexDelete(chan->mutex);
    chan->used = false;

    osMutexRelease(dev->mutex);
}

/**
 * @brief mbox send message , used by irq handler
 *
 * @param[in] chan mbox channel
 * @param[in] data msg data
 * @param[in] msg len
 * @param[in] timer_out timerout value(ms)
 * @return send result
 */
int mbox_send_isr(mbox_chan_t *chan, uint8_t *data,
              uint32_t len, uint32_t timeout)
{
    if (chan == NULL) {
        return -1;
    }

    mbox_dev_t *dev = chan->dev;
    int ret;

    ret = dev->ops->send(chan, data, len);

    if (ret < 0 || timeout == MBOX_NO_WAIT) {
        return ret;
    }

    ret = mbox_wait_tx_complete(chan, timeout);
    if (ret < 0) {
        dev->ops->cancel_send(chan);
    }

    return ret;
}

/**
 * @brief mbox send message
 *
 * @param[in] chan mbox channel
 * @param[in] data msg data
 * @param[in] msg len
 * @param[in] timer_out timerout value(ms)
 * @return send result
 */
int mbox_send(mbox_chan_t *chan, uint8_t *data,
              uint32_t len, uint32_t timeout)
{
    if (chan == NULL) {
        return -1;
    }

    mbox_dev_t *dev = chan->dev;
    int ret;

    osMutexAcquire(chan->mutex, osWaitForever);

    ret = dev->ops->send(chan, data, len);

    if (ret < 0 || timeout == MBOX_NO_WAIT) {
        osMutexRelease(chan->mutex);
        return ret;
    }

    ret = mbox_wait_tx_complete(chan, timeout);
    if (ret < 0) {
        dev->ops->cancel_send(chan);
    }

    osMutexRelease(chan->mutex);

    return ret;
}

/**
 * @brief mbox alloc message
 *
 * @param[in] chan mbox channel
 * @param[in] msg len
 * @return message address
 */
uint8_t *mbox_alloc_buf(mbox_chan_t *chan, uint32_t len)
{
    if (chan == NULL) {
        return NULL;
    }

    mbox_dev_t *dev = chan->dev;
    uint8_t *buf;

    osMutexAcquire(chan->mutex, osWaitForever);

    buf = dev->ops->alloc_buf(chan, len);

    osMutexRelease(chan->mutex);

    return buf;
}

/**
 * @brief mbox send no copy message
 *
 * @param[in] chan mbox channel
 * @param[in] data msg data
 * @param[in] msg len
 * @param[in] timer_out timerout value(ms)
 * @return send result
 */
int mbox_send_nocopy(mbox_chan_t *chan, uint8_t *data,
                     uint32_t len, uint32_t timeout)
{
    if (chan == NULL) {
        return -1;
    }

    mbox_dev_t *dev = chan->dev;
    int ret;

    osMutexAcquire(chan->mutex, osWaitForever);

    ret = dev->ops->send_nocopy(chan, data, len);

    if (ret < 0 || timeout == MBOX_NO_WAIT) {
        osMutexRelease(chan->mutex);
        return ret;
    }

    ret = mbox_wait_tx_complete(chan, timeout);
    if (ret < 0) {
        dev->ops->cancel_send(chan);
    }

    osMutexRelease(chan->mutex);

    return ret;
}

/**
 * @brief mbox send with message buffer
 *
 * @param[in] chan mbox channel
 * @param[in] data msg data
 * @param[in] msg len
 * @param[in] timer_out timerout value(ms)
 * @return send result
 */
int mbox_send_nobuf(mbox_chan_t *chan, uint8_t *data,
                    uint32_t len, uint32_t timeout)
{
    if (chan == NULL) {
        return -1;
    }

    mbox_dev_t *dev = chan->dev;
    int ret;

    osMutexAcquire(chan->mutex, osWaitForever);

    ret = dev->ops->send_nobuf(chan, data, len);

    if (ret < 0 || timeout == MBOX_NO_WAIT) {
        osMutexRelease(chan->mutex);
        return ret;
    }

    ret = mbox_wait_tx_complete(chan, timeout);
    if (ret < 0) {
        dev->ops->cancel_send(chan);
    }

    osMutexRelease(chan->mutex);

    return ret;
}

/**
 * @brief mbox set callback
 *
 * @param[in] chan mbox channel
 * @param[in] callback callback function
 * @param[in] arg callback arg
 */
void mbox_set_callback(mbox_chan_t *chan, mbox_rxcallback callback, void *arg)
{
    if (chan == NULL) {
        return;
    }
    chan->arg = arg;
    chan->callback = callback;
}

/**
 * @brief mbox receive message, used by mbox driver
 *
 * @param[in] chan mbox channel
 * @param[in] data msg data
 * @param[in] msg len
 * @return receive result
 */
int mbox_received(mbox_chan_t *chan, uint8_t *data, uint32_t len)
{
    if (chan == NULL) {
        return -1;
    }
    int ret = 0;

    if (chan->callback) {
        ret = chan->callback(chan->arg, data, len);
    }
    return ret;
}
