/*
 * sdpe_rpc_device.h
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#ifndef SDPE_RPC_DEVICE_H
#define SDPE_RPC_DEVICE_H

#include <types.h>
#include <cmsis_os2.h>

#include <lib/ering_buffer.h>
#include <sdpe_rpc_common.h>
#include <sdpe_rpc_transport.h>

#define SDPE_RPC_NAME_LEN       32
#define SDPE_RPC_MAX_DATA_NUM   2

/* sdpe rpc message type */

#define SDPE_RPC_REQ            0
#define SDPE_RPC_RSP            1
#define SDPE_RPC_IND            2

typedef int (*sdpe_rpc_dev_cb)(void *arg, uint16_t msg_id, uint8_t *msg, uint32_t len);

SDPE_RPC_PACKED_BEGIN
struct sdpe_rpc_message_head {
    uint16_t msg_id;
    uint16_t msg_type;
    uint32_t cookie;
    uint8_t data[0];
} SDPE_RPC_PACKED_END;

#define SDPE_RPC_MESSAGE_HEAD sizeof(struct sdpe_rpc_message_head)

typedef struct sdpe_rpc_dev_cfg {
    const char *name;
    uint8_t rproc;
    uint32_t addr;
    sdpe_rpc_transport_type type;
    uint32_t max_msg_len;
    bool support_workq;
    bool support_pack;
    void *lower_half;
} sdpe_rpc_dev_cfg_t;

typedef struct sdpe_rpc_pack_info {
    uint16_t pack_buf_len;
    uint16_t pack_offset;
    uint16_t pack_sn;
    uint16_t reserve;
    uint8_t  *pack_buffer;
} sdpe_rpc_pack_info_t;

typedef struct sdpe_rpc_dev {
    char name[SDPE_RPC_NAME_LEN];
    bool support_workq;
    bool support_pack;
    uint8_t status;
    sdpe_rpc_transport_dev_t transport;
    sdpe_rpc_pack_info_t pack;
    osMutexId_t tx_lock;
    uint32_t rx_buf_len;
    uint8_t *rx_buf;
    ering_buffer_t *wq_buffer;
    osSemaphoreId_t wq_sem;
    osThreadId_t rx_thread;
    osThreadId_t wq_thread;
    sdpe_rpc_dev_cb usr_cb;
    void *arg;
} sdpe_rpc_dev_t;

typedef struct sdpe_rpc_message {
    uint16_t msg_id;
    uint8_t msg_type;
    uint8_t data_num;
    uint32_t data_len[SDPE_RPC_MAX_DATA_NUM];
    uint8_t *data[SDPE_RPC_MAX_DATA_NUM];
} sdpe_rpc_message_t;

/**
 * @brief sdpe rpc device init
 *
 * @param[in] dev sdpe rpc dev
 * @param[in] cfg sdpe rpc dev config
 * @return result
 */
int sdpe_rpc_dev_init(sdpe_rpc_dev_t *dev, sdpe_rpc_dev_cfg_t *cfg);

/**
 * @brief open sdpe rpc device
 *
 * @param[in] dev sdpe rpc dev
 * @param[in] usr_cb rx callback
 * @param[in] arg rx callback arg
 * @return result
 */
int sdpe_rpc_dev_open(sdpe_rpc_dev_t *dev, sdpe_rpc_dev_cb usr_cb, void *arg);

/**
 * @brief close sdpe rpc device
 *
 * @param[in] dev sdpe rpc dev
 * @return result
 */
int sdpe_rpc_dev_close(sdpe_rpc_dev_t *dev);

/**
 * @brief send sdpe rpc message
 *
 * @param[in] message rpc message need to send
 * @param[in] wait_time wait time for sending message
 * @return result
 */
int sdpe_rpc_dev_send(sdpe_rpc_dev_t *dev, sdpe_rpc_message_t *message, uint32_t wait_time);

#endif
