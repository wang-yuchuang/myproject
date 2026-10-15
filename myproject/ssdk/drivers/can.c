/**
 * @file can.c
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */
#include <cmsis_os2.h>
#include <debug.h>
#include <lib/list.h>
#include <spinlock.h>
#include <stdlib.h>
#include <string.h>
#include <types.h>

#include "driver/can.h"

extern size_t ExtxPortGetFreeHeapSize( void );
extern void *ExtMalloc( size_t xWantedSize );
extern void ExtFree( void *pv );

#ifndef CONFIG_PART_CAN_NUM
#define CONFIG_PART_CAN_NUM 4
#endif

static can_device_t g_can_dev[CONFIG_PART_CAN_NUM];
static int g_can_dev_index = 0;
static uint32_t g_rx_drop_cnt[CONFIG_PART_CAN_NUM];

/**
 * @brief put buf back to tail of free_list after handle.
 *
 * @param[in] buf point to buffer node.
 */
static void can_release_buf(can_buf_t *buf)
{
    irq_state_t state;

    can_fifo_t *fifo = buf->fifo;
    state = spin_lock_irqsave(&fifo->lock);
    if (list_in_list(&buf->node))
        list_delete(&buf->node);
    list_add_tail(&fifo->free_list, &buf->node);
    spin_unlock_irqrestore(&fifo->lock, state);
}

/**
 * @brief alloc a buf from head of free_list, delete it from the list.
 *
 * @param[in] fifo point to rx/tx buffer fifo.
 * @return can_buf_t point to buffer node.
 */
static can_buf_t *can_require_buf(can_fifo_t *fifo)
{
    /* Assertion. */
    ASSERT(fifo);
    irq_state_t state;

    state = spin_lock_irqsave(&fifo->lock);
    can_buf_t *buf = list_remove_head_type(&fifo->free_list, can_buf_t, node);
    spin_unlock_irqrestore(&fifo->lock, state);

    return buf;
}

/**
 * @brief place buf at tail of used_list, notify tx/rx thread to handle.
 *
 * @param[in] buf point to buffer node.
 */
static void can_send_buf(can_buf_t *buf)
{
    irq_state_t state;

    can_fifo_t *fifo = buf->fifo;
    state = spin_lock_irqsave(&fifo->lock);
    list_add_tail(&fifo->used_list, &buf->node);
    spin_unlock_irqrestore(&fifo->lock, state);
    osSemaphoreRelease(fifo->buf_sem);
}

/**
 * @brief judge if usedlist of fifo is empty.
 *
 * @param[in] fifo point to rx/tx buffer fifo.
 */
static inline bool fifo_is_empty(can_fifo_t *fifo)
{
    return list_is_empty(&fifo->used_list);
}

/**
 * @brief convert frame to buf.
 *
 * @param[in] buf point to buffer node.
 * @param[in] frame data from user.
 * @param[in] mb_idx hardware bufffer index in specific arch.
 */
static void can_fill_buf(can_buf_t *buf, can_frame_t *frame, uint8_t mb_idx)
{
    buf->frame.id = frame->id;
    buf->frame.ide = frame->ide;
    buf->frame.rtr = frame->rtr;
    buf->frame.fd = frame->fd;
    memcpy(&buf->frame.data, &frame->data, frame->len);
    buf->frame.len = frame->len;
    buf->mb_idx = mb_idx;
}

/**
 * @brief after init, all available buffer attached to free_list.
 *
 * @param[in] fifo fifo's free_list can recv buf, free_list for recved buf
 * @param[in] num number of total buffers.
 */
static inline void can_init_fifo(can_fifo_t *fifo, uint32_t num)
{
    spin_lock_init(&fifo->lock);
    list_initialize(&fifo->free_list);
    list_initialize(&fifo->used_list);
    fifo->buf_sem = osSemaphoreNew(num, 0, NULL);
    for (int i = 0; i < num; i++) {
        list_add_tail(&fifo->free_list, &fifo->buf[i].node);
        fifo->buf[i].fifo = fifo;
    }
}

/**
 * @brief wait until mailbox is idle.
 * If using flow control, call this function in
 * sendmsg operation when sending frame failed.
 *
 * @param[in] can_dev specific can device.
 */
int can_tx_stop(can_device_t *device)
{
    /* Assertion. */
    ASSERT(device);

    /* 等待硬件邮箱发送完成, 上限 10 tick(约10ms @1kHz tick):
     * 总线异常(无ACK/bus-off)时邮箱永不完成, 无限等待会把
     * 发送泵任务永久睡死在 sendmsg 内部 */
    osEventFlagsWait(device->tx_event, FIFO_START,
                    osFlagsWaitAll, 10);

    return 0;
}

/**
 * @brief resume tx flow.
 *
 * @param[in] can_dev specific can device.
 */
void can_tx_resume(can_device_t *device)
{
    /* Assertion. */
    ASSERT(device);

    osEventFlagsSet(device->tx_event, FIFO_START);
}

/**
 * @brief transmits written can frame in buffer.
 *
 * @param[in] can_dev specific can device.
 */
void can_transmit_buf(can_device_t *can_dev)
{
    can_fifo_t *tx_fifo = can_dev->tx_fifo;

    osSemaphoreAcquire(tx_fifo->buf_sem, osWaitForever);

    irq_state_t state = spin_lock_irqsave(&tx_fifo->lock);
    can_buf_t *buf = list_remove_head_type(&tx_fifo->used_list,
                                            can_buf_t, node);
    spin_unlock_irqrestore(&tx_fifo->lock, state);

    if (buf) {
        can_dev->ops->sendmsg(can_dev, buf);
        /* 无条件释放软件 buf:
         * 成功 = 帧已写入硬件邮箱, 软件使命结束;
         * 失败 = 丢弃本帧, 周期帧由应用下个周期重发新数据。
         * 仅在失败时释放会把成功帧全部泄漏, ~50 帧耗尽 free_list,
         * can_write 永久 -1, 形成 "write fail" 1Hz 刷屏 */
        can_release_buf(buf);
    }
}

/**
 * @brief handles received can frame in buffer.
 *
 * @param[in] can_dev specific can device.
 */
int can_receive_buf(can_device_t *can_dev, can_frame_t *frame)
{
    ASSERT(frame);

    int ret = 0;
    can_fifo_t *rx_fifo = can_dev->rx_fifo;

    osSemaphoreAcquire(rx_fifo->buf_sem, osWaitForever);

    irq_state_t state = spin_lock_irqsave(&rx_fifo->lock);
    can_buf_t *buf = list_remove_head_type(&rx_fifo->used_list,
                                            can_buf_t, node);

    spin_unlock_irqrestore(&rx_fifo->lock, state);

    if (buf) {
        memcpy(frame, &buf->frame, sizeof(can_frame_t));
        can_release_buf(buf);
    }
    else
        ret = -1;

    return ret;
}

/**
 * @brief read can hardware if buf available.
 *
 * @param[in] can_dev specific can device.
 * @param[in] mb_idx hardware bufffer index in specific arch.
 */
int can_read(can_device_t *can_dev, uint8_t mb_idx)
{
    can_fifo_t *rx_fifo = can_dev->rx_fifo;
    int ret;
    can_frame_t frame;

    ret = can_dev->ops->recvmsg(can_dev, &frame, mb_idx);
    if(ret)
        return -1;

    can_buf_t *buf = can_require_buf(rx_fifo);
    if (buf) {
        can_fill_buf(buf, &frame, mb_idx);
        can_send_buf(buf);

        return 0;
    }
    else {
        /* 本函数运行于 CAN 中断上下文: 逐帧 printf(115200 下单行 3~4ms)
         * 会饿死接收任务, 形成 丢帧->打印->更多丢帧 的死亡螺旋。
         * 只累加计数, 不在中断里打印。 */
        if (can_dev->ch < CONFIG_PART_CAN_NUM)
            (void)++g_rx_drop_cnt[can_dev->ch];

        return -1;
    }
}

/**
 * @brief add can frame to tx buffer.
 *
 * @param[in] can_dev specific can device.
 * @param[in] frame message wrote by user.
 * @param[in] mb_idx hardware bufffer index in specific arch.
 */
int can_write(can_device_t *can_dev, can_frame_t *frame, uint8_t mb_idx)
{
    can_fifo_t *tx_fifo = can_dev->tx_fifo;
    int ret = 0;

    can_buf_t *buf = can_require_buf(tx_fifo);

    if (buf) {
        can_fill_buf(buf, frame, mb_idx);
        can_send_buf(buf);
    }
    else {
        ssdk_printf(SSDK_DEBUG, "CAN %d no availiable tx buf\n", can_dev->ch);
        ret = -1;
    }
    return ret;
}

/**
 * @brief read can frame from specific mailbox.
 *
 * @param[in] can_dev specific can device.
 * @param[in] mb_idx hardware bufffer index in specific arch.
 */
int can_read_polling(can_device_t *can_dev, uint8_t mb_idx)
{
    int result;

    osMutexAcquire(can_dev->mutex, osWaitForever);
    result = can_dev->ops->check_recvmsg(can_dev, mb_idx);
    if (result == true) {
        can_read(can_dev, mb_idx);
    }
    osMutexRelease(can_dev->mutex);

    return 0;
}

/**
 * @brief initialize can framework.
 *
 * @param[in] can_dev specific can device.
 * @param[in] cfg can configuration.
 */
int can_init(can_device_t *can_dev, struct can_buf_cfg *cfg)
{
    int ret = 0;

    can_dev->mutex = osMutexNew(NULL);
    can_dev->tx_event = osEventFlagsNew(NULL);
    #if(0)
    can_dev->rx_fifo =
        osAlloc(sizeof(can_fifo_t) + sizeof(can_buf_t) * cfg->rxfifo_size);
    can_dev->tx_fifo =
        osAlloc(sizeof(can_fifo_t) + sizeof(can_buf_t) * cfg->txfifo_size);
    #else
    can_dev->rx_fifo =
        ExtMalloc(sizeof(can_fifo_t) + sizeof(can_buf_t) * cfg->rxfifo_size);
    can_dev->tx_fifo =
        ExtMalloc(sizeof(can_fifo_t) + sizeof(can_buf_t) * cfg->txfifo_size);
    #endif

    if (!can_dev->rx_fifo || !can_dev->tx_fifo) {
        ret = 1;
        return ret;
    }

    memset(can_dev->rx_fifo, 0,
           sizeof(can_fifo_t) + sizeof(can_buf_t) * cfg->rxfifo_size);
    memset(can_dev->tx_fifo, 0,
           sizeof(can_fifo_t) + sizeof(can_buf_t) * cfg->txfifo_size);

    can_init_fifo(can_dev->rx_fifo, cfg->rxfifo_size);
    can_init_fifo(can_dev->tx_fifo, cfg->txfifo_size);

    return ret;
}

/**
 * @brief set can controller mode.
 *
 * @param[in] can_dev specific can device.
 * @param[in] mode the mode of the can controller.
 */
int can_set_mode(can_device_t *can_dev, enum can_mode mode)
{
    int ret;

    osMutexAcquire(can_dev->mutex, osWaitForever);
    ret = can_dev->ops->set_mode(can_dev, mode);
    osMutexRelease(can_dev->mutex);

    return ret;
}

/**
 * @brief request sleep operation.
 *
 * @param[in] can_dev specific can device.
 * @param[in] request true: enter sleep, false: exit sleep.
 */
void can_request_sleep(can_device_t *can_dev, bool request)
{
    osMutexAcquire(can_dev->mutex, osWaitForever);
    can_dev->ops->request_sleep(can_dev, request);
    osMutexRelease(can_dev->mutex);
}

void can_set_err_notify(can_device_t *can_dev, can_error_notify notify)
{
    can_dev->notify = notify;
}

void can_err_notify(can_device_t *can_dev, enum can_errcode code)
{
    if (can_dev->notify != NULL)
        can_dev->notify(can_dev, code);
}

/**
 * @brief alloc a can device instance for can pointer.
 *
 */
can_device_t * can_alloc(void)
{
    if(g_can_dev_index > CONFIG_PART_CAN_NUM){
        g_can_dev_index = 0;
        return NULL;
    }
    can_device_t * can_dev = &g_can_dev[g_can_dev_index];
    g_can_dev_index ++;
    return can_dev;
}

/**
 * @brief get probe instance number
 *
 */
int can_get_probe_count(void){
    return g_can_dev_index;
}

/**
 * @brief get can instance in instance arrary by probe function
 *
 * @param[in] can_id can controller id.
 * @return can_id contrller instance if it is not probe , it will return NULL.
 */
can_device_t * can_get_probe_device(int can_id)
{
    for(int i=0;i<g_can_dev_index;i++){
        if(g_can_dev[i].ch == can_id){
            return (can_device_t *)&g_can_dev[i];
        }
    }
    return NULL;
}
