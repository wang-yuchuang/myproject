/**
 * @file can.h
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#ifndef CAN_H_
#define CAN_H_

#include <cmsis_os2.h>
#include <lib/list.h>
#include <spinlock.h>
#include <stdint.h>

/**
 * @brief Defines the mode of the can controller
 */
enum can_mode {
    /* Normal mode */
    CAN_NORMAL_MODE,
    /* Controller is not allowed to send dominant bits */
    CAN_LISTEN_MODE,
    /* Controller is in loopback mode (receive own messages) */
    CAN_LOOPBACK_MODE,
    /* Combination of loopback and silent */
    CAN_LOOPBACKANLISTEN_MODE
};

/**
 * @brief Defines the error code.
 */
enum can_errcode {
    CAN_BUSOFF,
    CAN_WAKEUP,
    CAN_NO_ACK,
    CAN_BIT_ERROR,
    CAN_RX_OVERFLOW
};

#define FIFO_START (1 << 0)

#define CAN_STDID 0
#define CAN_EXTID 1
#define CAN_DTR 0
#define CAN_RTR 1
#define CAN_WITH_FD 1

typedef struct can_device can_device_t;
typedef struct can_buf can_buf_t;
typedef struct can_frame can_frame_t;
typedef struct can_fifo can_fifo_t;

typedef void (*can_error_notify)(can_device_t *can_dev, enum can_errcode code);

struct can_buf_cfg {
    uint32_t txfifo_size;
    uint32_t rxfifo_size;
};

struct can_ops {
    /* send one message on the can */
    int (*sendmsg)(can_device_t *can_dev, can_buf_t *buf);
    /* Return true by poll if there exits one message on the can */
    bool (*check_recvmsg)(can_device_t *can_dev, uint8_t mb_idx);
    /* rec one message from the can */
    int (*recvmsg)(can_device_t *can_dev, can_frame_t *frame, uint8_t index);
    /* modify can work mode */
    int (*set_mode)(can_device_t *can_dev, enum can_mode mode);
    /* request sleep operation */
    void (*request_sleep)(can_device_t *can_dev, bool request);
};

struct can_frame {
    /* message identifier */
    uint32_t id : 29;
    /* Indicates the identifier type (standard or extended) */
    uint32_t ide : 1;
    /* indication the frame type(data or remote) */
    uint32_t rtr : 1;
    /* frame is in the CAN-FD frame format */
    uint32_t fd : 1;
    /* length of the message (max. 64 if fd enabled) in byte */
    uint32_t len;
    /* frame payload data */
    uint8_t data[64];
} __attribute__((aligned(4)));

struct can_buf {
    struct list_node node;
    struct can_frame frame;
    uint8_t mb_idx;
    can_fifo_t *fifo;
} __attribute__((aligned(4)));

struct can_fifo {
    spin_lock_t lock;
    struct list_node free_list;
    struct list_node used_list;
    osSemaphoreId_t buf_sem;
    can_buf_t buf[];
} __attribute__((aligned(4)));

typedef struct can_device {
    uint8_t ch;
    int irq_nr;
    addr_t base;
    const struct can_ops *ops;
    osMutexId_t mutex;
    osEventFlagsId_t tx_event;
    /* sw buffer */
    can_fifo_t *rx_fifo;
    can_fifo_t *tx_fifo;
    can_error_notify notify;
    void *priv;
} can_device_t;

int can_tx_stop(can_device_t *device);
void can_tx_resume(can_device_t *device);

void can_transmit_buf(can_device_t *can_dev);
void can_transmit_buf_noblock(can_device_t *can_dev);
int can_receive_buf(can_device_t *can_dev, can_frame_t *frame);

int can_init(can_device_t *can_dev, struct can_buf_cfg *cfg);
int can_write(can_device_t *can_dev, can_frame_t *frame, uint8_t mb_idx);
int can_read(can_device_t *can_dev, uint8_t mb_idx);
int can_read_polling(can_device_t *can_dev, uint8_t mb_idx);

void can_request_sleep(can_device_t *can_dev, bool request);
int can_set_mode(can_device_t *can_dev, enum can_mode mode);
void can_set_err_notify(can_device_t *can_dev, can_error_notify ind);
void can_err_notify(can_device_t *can_dev, enum can_errcode code);

can_device_t * can_alloc(void);
int can_get_probe_count(void);
can_device_t * can_get_probe_device(int can_id);
#endif /*_CAN_H*/
