/*
 * sdpe_rpc_transport.h
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#ifndef SDPE_RPC_TRANSPORT_H
#define SDPE_RPC_TRANSPORT_H

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define SDPE_RPC_WAIT_FOREVER       (0xFFFFFFFF)
#define SDPE_RPC_MAX_WAIT           (100)  //ms

typedef enum sdpe_rpc_transport_type_e
{
    SDPE_RPC_MBOX_TRNASPORT = 0,
    SDPE_RPC_RPMSG_TRNASPORT,
    SDPE_RPC_MAX_TRNASPORT
} sdpe_rpc_transport_type;

typedef struct sdpe_rpc_transport_dev sdpe_rpc_transport_dev_t;

typedef struct sdpe_rpc_transport_ops {
    int (*open)(sdpe_rpc_transport_dev_t *dev);
    int (*close)(sdpe_rpc_transport_dev_t *dev);
    void *(*tx_alloc)(sdpe_rpc_transport_dev_t *dev, uint32_t *size, uint32_t wait_time);
    int (*send)(sdpe_rpc_transport_dev_t *dev, uint8_t *data, uint32_t len, uint32_t wait_time);
    int (*send_nocopy)(sdpe_rpc_transport_dev_t *dev, uint8_t *data, uint32_t len);
    int (*recv)(sdpe_rpc_transport_dev_t *dev, uint8_t *data, uint32_t *len, bool block);
} sdpe_rpc_transport_ops_t;

typedef struct sdpe_rpc_transport_cfg {
    const char *name;
    sdpe_rpc_transport_type type;
    uint8_t rproc;
    uint32_t addr;
    void *lower_half;
} sdpe_rpc_transport_cfg_t;

typedef struct sdpe_rpc_transport_dev {
    const char *name;
    void *priv;
    struct sdpe_rpc_transport_ops *ops;
    uint32_t mtu;
} sdpe_rpc_transport_dev_t;

extern int sdpe_rpc_mbox_init(sdpe_rpc_transport_dev_t *dev,
                              sdpe_rpc_transport_cfg_t *cfg);
extern int sdpe_rpc_rpmsg_init(sdpe_rpc_transport_dev_t *dev,
                               sdpe_rpc_transport_cfg_t *cfg);

/*******************************************************************************
 * API
 ******************************************************************************/

/*
 * @brief Initializes rpc transport.
 * @param dev  rpc transport instance
 * @return init result
 */
static inline int sdpe_rpc_transport_init(sdpe_rpc_transport_dev_t *dev,
                                          sdpe_rpc_transport_cfg_t *cfg)
{
    int ret;

#ifdef CONFIG_SDPE_RPC_MBOX_TRANSPORT
    if (cfg->type == SDPE_RPC_MBOX_TRNASPORT) {
        ret = sdpe_rpc_mbox_init(dev, cfg);
    }
    else
#endif
#ifdef CONFIG_SDPE_RPC_RPMSG_TRANSPORT
    if (cfg->type == SDPE_RPC_RPMSG_TRNASPORT) {
        ret = sdpe_rpc_rpmsg_init(dev, cfg);
    }
    else
#endif
    {
        ret = -1;
    }

    return ret;
}

/*
 * @brief get rpc transport mtu.
 * @param dev  rpc transport instance
 * @return mtu
 */
#define sdpe_rpc_get_mtu(dev)   (dev)->mtu

/*
 * @brief open rpc transport.
 * @param dev  rpc transport instance
 * @return open result
 */
#define sdpe_rpc_transport_open(dev)  (dev)->ops->open(dev)

/*
 * @brief close rpc transport.
 * @param dev  rpc transport instance
 * @return close result
 */
#define sdpe_rpc_transport_close(dev)  (dev)->ops->close(dev)

/*
 * @brief alloc tx buffer.
 * @param dev  rpc transport instance
 * @param size  alloc buffer size
 * @param wait_time wait time for alloc
 * @return tx buffer addr
 */
#define sdpe_rpc_transport_tx_alloc(dev, size, wait_time)  (dev)->ops->tx_alloc(dev, size, wait_time)

/*
 * @brief send tx buffer.
 * @param dev  rpc transport instance
 * @param data send data
 * @param len send data size
 * @return send result
 */
#define sdpe_rpc_transport_send(dev, data, len, wait)  (dev)->ops->send(dev, data, len, wait)

/*
 * @brief send tx buffer.
 * @param dev  rpc transport instance
 * @param data send data
 * @param len send data size
 * @return send result
 */
#define sdpe_rpc_transport_send_nocopy(dev, data, len)  (dev)->ops->send_nocopy(dev, data, len)

/**
 * @brief send tx buffer and wait for the handle response.
 * @param dev  rpc transport instance
 * @param data send data
 * @param len send data size
 * @return send result
 */
#define sdpe_rpc_transport_recv(dev, data, len, block)  (dev)->ops->recv(dev, data, len, block)

#endif
