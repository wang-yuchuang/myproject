#include <debug.h>
#include <stdlib.h>
#include <string.h>
#include <rpmsg_channel.h>
#include <rpmsg_ns.h>
#include <sd_errcode.h>

#include "rpmsg_internal.h"

#define RPMSG_CHANNEL_NAME_LEN  32

struct rpmsg_channel {
    char name[RPMSG_CHANNEL_NAME_LEN];
    struct rpmsg_instance *rpmsg_dev;
    struct rpmsg_endpoint *rpmsg_ept;
    osMessageQueueId_t queue;
    osThreadId_t thread;
    osEventFlagsId_t event;
    rpmsg_channel_cb cb;
    void *cb_arg;
    bool need_announce;
    bool announced;
};

struct rpmsg_queue_element {
    void *data;
    int len;
    unsigned long src;
};

static int rpmsg_ept_cb(void *payload, int payload_len, unsigned long src, void *priv)
{
    struct rpmsg_channel *chan = (struct rpmsg_channel *)priv;
    struct rpmsg_queue_element element;
    osStatus_t ret;

    element.data = payload;
    element.len = payload_len;
    element.src = src;

    ret = osMessageQueuePut(chan->queue, &element, 0, 0);
    if (ret == osOK) {
        return RPMSG_HOLD;
    }
    else {
        ssdk_printf(SSDK_WARNING, "%s: message queue put failed.\n", __func__);
        ERRCODE_PRINT(MOD_ID_SYS_SERVICE_COMM,
                chan->rpmsg_dev->remote_proc,
                COM_IPCC_IPCC_RECV_FAIL, src);
        return RPMSG_RELEASE;
    }
}

static void rpmsg_channel_thread(void *ch)
{
    osStatus_t ret;
    struct rpmsg_channel *chan;
    struct rpmsg_queue_element element;

    chan = (struct rpmsg_channel *)ch;
    osEventFlagsSet(chan->event, 1);

    while (1) {
        ret = osMessageQueueGet(chan->queue, &element, NULL, osWaitForever);
        if (ret == osOK) {
            chan->cb(element.data, element.len, element.src, chan->cb_arg);
            rpmsg_release_rx_buffer(chan->rpmsg_dev, element.data);
        }
    }
}

int rpmsg_channel_reset(rpmsg_channel_t rpchn)
{
    struct rpmsg_channel *chan;
    chan = (struct rpmsg_channel *)rpchn;

    if ((rpmsg_is_master(chan->rpmsg_dev) == 0) ||
        (chan->need_announce == RPMSG_TRUE)) {
        rpmsg_ns_announce(chan->rpmsg_dev, chan->rpmsg_ept, chan->name,
                          RPMSG_NS_CREATE);
    }
    return 0;
}

rpmsg_channel_t rpmsg_channel_create(struct rpmsg_instance *dev, char *name, uint32_t addr)
{
    struct rpmsg_channel *chan;

    if (rpmsg_is_link_up(dev) == false) {
        ssdk_printf(SSDK_WARNING, "%s: device not ready.\n", __func__);
        return NULL;
    }

    chan = (struct rpmsg_channel *)osAlloc(sizeof(struct rpmsg_channel));
    if (chan == NULL) {
        ssdk_printf(SSDK_WARNING, "%s: alloc memory failed.\n", __func__);
        ERRCODE_PRINT(MOD_ID_SYS_SERVICE_COMM,
                dev->remote_proc, COM_IPCC_MALLOC_FAIL, addr);
        return NULL;
    }

    memset(chan, 0x00, sizeof(struct rpmsg_channel));

    strncpy(chan->name, name, RPMSG_CHANNEL_NAME_LEN);
    chan->rpmsg_dev = dev;

    chan->queue = osMessageQueueNew(rpmsg_get_buffer_num(dev),
                                    sizeof(struct rpmsg_queue_element), NULL);
    if (chan->queue == NULL) {
        ssdk_printf(SSDK_WARNING, "%s: create queue failed.\n", __func__);
        osFree(chan);
        return NULL;
    }

    chan->rpmsg_ept = rpmsg_create_ept(dev, (unsigned long)addr, rpmsg_ept_cb, (void *)chan);
    if (chan->rpmsg_ept == NULL) {
        ssdk_printf(SSDK_WARNING, "%s: create ept failed.\n", __func__);
        osMessageQueueDelete(chan->queue);
        osFree(chan);
        return NULL;
    }

    if (dev->rpmsg_type == RPMSG_TYPE_IPCC) {
        chan->need_announce = RPMSG_TRUE;
    }

    chan->rpmsg_ept->priv = (void *)chan;

    return (rpmsg_channel_t)chan;
}

status_t rpmsg_channel_start(rpmsg_channel_t ch, rpmsg_channel_cb cb, void *cb_arg)
{
    status_t ret;
    struct rpmsg_channel *chan;

    chan = (struct rpmsg_channel *)ch;

    if (cb != NULL) {
        chan->cb = cb;
        chan->cb_arg = cb_arg;
        chan->event = osEventFlagsNew(NULL);
        if (chan->event == NULL) {
            ssdk_printf(SSDK_WARNING, "%s: create event failed.\n", __func__);
            return -1;
        }
         osThreadAttr_t attr = {
            .name = "rpmsg_ch",
            .priority = 28,   /* 高于 CAN 收报(26): 保证 echo_cb/通道消费及时 */
        };
        chan->thread = osThreadNew(rpmsg_channel_thread, chan, &attr);
        if (chan->thread == NULL) {
            ssdk_printf(SSDK_WARNING, "%s: create thread failed.\n", __func__);
            ERRCODE_PRINT(MOD_ID_SYS_SERVICE_COMM,
                    chan->rpmsg_dev->remote_proc, COM_IPCC_THREAD_FAIL, 0);
            osEventFlagsDelete(chan->event);
            chan->event = NULL;
            return -1;
        }
        while (osEventFlagsWait(chan->event, 1, osFlagsWaitAny, osWaitForever) != 1) {
        }
        osEventFlagsDelete(chan->event);
        chan->event = NULL;
    }

    if ((rpmsg_is_master(chan->rpmsg_dev) == 0) ||
        (chan->need_announce == RPMSG_TRUE)) {
        ret = rpmsg_ns_announce(chan->rpmsg_dev, chan->rpmsg_ept,
                                chan->name, RPMSG_NS_CREATE);
        if (ret != RPMSG_SUCCESS) {
            ssdk_printf(SSDK_WARNING, "%s: announce chan %s failed.\n",
                __func__, chan->name);
        }
    }

    return 0;
}

int rpmsg_channel_max_payload(rpmsg_channel_t ch)
{
    struct rpmsg_channel *chan;

    chan = (struct rpmsg_channel *)ch;

    return rpmsg_get_payload_size(chan->rpmsg_dev);
}

status_t rpmsg_channel_destroy(rpmsg_channel_t ch)
{
    status_t ret;
    osStatus_t os_ret;
    struct rpmsg_channel *chan;
    struct rpmsg_queue_element element;

    chan = (struct rpmsg_channel *)ch;

    if ((rpmsg_is_master(chan->rpmsg_dev) == 0) ||
        (chan->need_announce == RPMSG_TRUE)) {
        ret = rpmsg_ns_announce(chan->rpmsg_dev, chan->rpmsg_ept,
                                chan->name, RPMSG_NS_DESTROY);
        if (ret != RPMSG_SUCCESS) {
            ssdk_printf(SSDK_WARNING, "%s: announce chan %s failed.\n",
                __func__, chan->name);
        }
    }

    rpmsg_destroy_ept(chan->rpmsg_dev, chan->rpmsg_ept);

    if (chan->thread != NULL) {
        osThreadTerminate(chan->thread);
    }

    os_ret = osMessageQueueGet(chan->queue, &element, NULL, 0);
    while (os_ret == osOK) {
        rpmsg_release_rx_buffer(chan->rpmsg_dev, element.data);
        os_ret = osMessageQueueGet(chan->queue, &element, NULL, 0);
    }

    osMessageQueueDelete(chan->queue);
    osFree(chan);

    return 0;
}

status_t rpmsg_channel_send(rpmsg_channel_t ch, uint32_t dst_addr, void *data, int len, uint32_t timeout)
{
    struct rpmsg_channel *chan;

    chan = (struct rpmsg_channel *)ch;

    return rpmsg_send(chan->rpmsg_dev, chan->rpmsg_ept,
                      (unsigned long)dst_addr, (char *)data,
                      (unsigned long)len, (unsigned long)timeout);
}

void *rpmsg_channel_alloc_send_buffer(rpmsg_channel_t ch, int *len, uint32_t timeout)
{
    struct rpmsg_channel *chan;
    void *ret;
    unsigned long size;

    chan = (struct rpmsg_channel *)ch;
    size = (unsigned long)*len;

    ret = rpmsg_alloc_tx_buffer(chan->rpmsg_dev, &size, (unsigned long)timeout);

    *len = (int)size;
    return ret;
}

status_t rpmsg_channel_send_nocopy(rpmsg_channel_t ch, uint32_t dst_addr, void *data, int len)
{
    struct rpmsg_channel *chan;

    chan = (struct rpmsg_channel *)ch;

    return rpmsg_send_nocopy(chan->rpmsg_dev, chan->rpmsg_ept,
                             (unsigned long)dst_addr, data, (unsigned long)len);
}

status_t rpmsg_channel_recv(rpmsg_channel_t ch, void *buff, int buff_size, int *len, uint32_t timeout)
{
    osStatus_t ret;
    struct rpmsg_channel *chan;
    struct rpmsg_queue_element element;

    chan = (struct rpmsg_channel *)ch;
    *len = 0;

    ret = osMessageQueueGet(chan->queue, &element, NULL, timeout);
    if (ret != osOK) {
        ssdk_printf(SSDK_WARNING, "%s: message queue get failed.\n", __func__);
        return -1;
    }

    *len = MIN(buff_size, element.len);
    memcpy(buff, element.data, *len);
    rpmsg_release_rx_buffer(chan->rpmsg_dev, element.data);

    return 0;
}

status_t rpmsg_channel_recvfrom(rpmsg_channel_t ch, uint32_t *src, void *buff, int buff_size, int *len, uint32_t timeout)
{
    osStatus_t ret;
    struct rpmsg_channel *chan;
    struct rpmsg_queue_element element;

    chan = (struct rpmsg_channel *)ch;
    *len = 0;

    ret = osMessageQueueGet(chan->queue, &element, NULL, timeout);
    if (ret != osOK) {
        ssdk_printf(SSDK_WARNING, "%s: message queue get failed.\n", __func__);
        return -1;
    }

    *len = MIN(buff_size, element.len);
    memcpy(buff, element.data, *len);
    if (src)
        *src = element.src;
    rpmsg_release_rx_buffer(chan->rpmsg_dev, element.data);

    return 0;
}

rpmsg_channel_t rpmsg_channel_from_endpoint(struct rpmsg_endpoint *ept)
{
    return (rpmsg_channel_t)(ept->priv);
}

status_t rpmsg_channel_announce_info(rpmsg_channel_t ch, bool *need_announce, bool *announced)
{
    struct rpmsg_channel *chan;

    if (ch) {
        chan = (struct rpmsg_channel *)ch;
        *need_announce = chan->need_announce;
        *announced = chan->announced;
    }

    return 0;
}

status_t rpmsg_channel_set_announced(rpmsg_channel_t ch, bool status)
{
    struct rpmsg_channel *chan;

    chan = (struct rpmsg_channel *)ch;
    chan->announced = (status ? RPMSG_TRUE : RPMSG_FALSE);

    return 0;
}

char *rpmsg_channel_name(rpmsg_channel_t ch)
{
    struct rpmsg_channel *chan;

    chan = (struct rpmsg_channel *)ch;
    return &chan->name[0];
}

status_t rpmsg_channel_get_type_rproc(rpmsg_channel_t ch, uint32_t *type, uint32_t *rproc)
{
    struct rpmsg_channel *chan;

    chan = (struct rpmsg_channel *)ch;

    if (type) {
        *type = chan->rpmsg_dev->rpmsg_type;
    }

    if (rproc) {
        *rproc = chan->rpmsg_dev->remote_proc;
    }

    return 0;
}
