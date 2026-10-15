/**
 * @file mbox.h
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: mailbox interface.
 *
 * Revision History:
 * -----------------
 */

#ifndef INCLUDE_DRV_MBOX_H
#define INCLUDE_DRV_MBOX_H

#include <types.h>

#include <compiler.h>
#include <spinlock.h>
#include <cmsis_os2.h>

#define MBOX_NO_WAIT            0
#define MBOX_WAIT_FOREVER       0xffffffff

#define ERR_NO_HW_CHAN -2

#ifndef ASSEMBLY

__BEGIN_CDECLS

/* mbox config */
typedef struct mbox_config {
    paddr_t                 base;
    paddr_t                 mem_base;
    int                     irq;
    uint8_t                 set_masterid_num;
} mbox_config_t;

typedef struct mbox_ops mbox_ops_t;
typedef struct mbox_chan mbox_chan_t;

/* mbox driver */
typedef struct mbox_dev {
    const mbox_ops_t        *ops;
    paddr_t                 base;
    paddr_t                 mem_base;
    int                     irq;
    uint8_t                 set_masterid_num;
    uint8_t                 chan_num;
    mbox_chan_t             *chan;
    osMutexId_t             mutex;
    void                    *priv;
    uint32_t                mtu;
} mbox_dev_t;

/* mbox callback */
typedef int (*mbox_rxcallback)(void *arg, uint8_t *data, uint32_t len);

/* mbox channel */
typedef struct mbox_chan {
    mbox_dev_t              *dev;
    bool                    used;
    osMutexId_t             mutex;
    mbox_rxcallback         callback;
    void                    *arg;
    void                    *priv;
} mbox_chan_t;

/* mbox operation */
typedef struct mbox_ops {
    void (*init)(mbox_dev_t *dev);
    bool (*startup)(mbox_chan_t *chan, void *priv_data);
    void (*shutdown)(mbox_chan_t *chan);
    int (*send)(mbox_chan_t *chan, uint8_t *data, uint32_t len);
    uint8_t *(*alloc_buf)(mbox_chan_t *chan, uint32_t len);
    int (*send_nocopy)(mbox_chan_t *chan, uint8_t *data, uint32_t len);
    int (*send_nobuf)(mbox_chan_t *chan, uint8_t *data, uint32_t len);
    void (*cancel_send)(mbox_chan_t *chan);
    bool (*tx_done)(mbox_chan_t *chan);
} mbox_ops_t;

/**
 * @brief mbox register
 *
 * @param[in] dev mbox dev
 * @param[in] cfg mbox config
 */
void mbox_init(mbox_dev_t *dev, const mbox_config_t *cfg);

/**
 * @brief mbox get mtu
 *
 * @param[in] dev mbox dev
 * @return mtu
 */
uint32_t mbox_mtu(mbox_dev_t *dev);

/**
 * @brief mbox client request channel
 *
 * @param[in] dev mbox dev
 * @param[in] priv_data priv data for lower driver
 * @return mbox channel
 */
mbox_chan_t *mbox_request_channel(mbox_dev_t *dev, void *priv_data);

/**
 * @brief mbox client release channel
 *
 * @param[in] client mbox client
 */
void mbox_release_channel(mbox_chan_t *chan);

/**
 * @brief mbox set callback
 *
 * @param[in] chan mbox channel
 * @param[in] callback callback function
 * @param[in] arg callback arg
 */
void mbox_set_callback(mbox_chan_t *chan, mbox_rxcallback callback, void *arg);

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
              uint32_t len, uint32_t timeout);

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
              uint32_t len, uint32_t timeout);

/**
 * @brief mbox alloc message
 *
 * @param[in] chan mbox channel
 * @param[in] msg len
 * @return message address
 */
uint8_t *mbox_alloc_buf(mbox_chan_t *chan, uint32_t len);

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
                     uint32_t len, uint32_t timeout);


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
                    uint32_t len, uint32_t timeout);

/**
 * @brief mbox receive message, used by mbox driver
 *
 * @param[in] chan mbox channel
 * @param[in] data msg data
 * @param[in] msg len
 * @return receive result
 */
int mbox_received(mbox_chan_t *chan, uint8_t *data, uint32_t len);

__END_CDECLS

#endif

#endif
