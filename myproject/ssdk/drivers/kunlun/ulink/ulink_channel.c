/*
 * Copyright (c) 2020 Semidrive Semiconductor, Inc.
 * All rights reserved.
 */

#include <stdlib.h>
#include <bits.h>
#include <debug.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <lib/list.h>
#include <driver/mbox.h>
#include <mbox_cfg.h>

#include "ulink.h"
#if defined(CONFIG_KUNLUN_MBOX)
#include <driver/kunlun/mbox/sdrv_mbox_drv.h>
#endif

#define BLOCK_SIZE 0x1000
#define ULINK_CH_EVENT_COMPLETE 0x1

extern ulink_channel_desc_t *ulink_channel_server_desc[];
extern ulink_mini_client_t ulink_mini_client[];

typedef struct {
    ulink_channel_t ch;
    ulink_channel_desc_t *desc;
    struct list_node list;
    osEventFlagsId_t rx_event;
    uint8_t rx_buf[BLOCK_SIZE];
    uint32_t rx_size;
} ulink_channel_inner_t;

static void payload(ulink_channel_inner_t *ch_inner, uint8_t *data,
                    uint32_t size)
{
    memcpy(ch_inner->rx_buf, data, size);
    ch_inner->rx_size = size;
    if (ch_inner->ch.callback)
        ch_inner->ch.callback(&ch_inner->ch, ch_inner->rx_buf, size);
    else
        osEventFlagsSet(ch_inner->rx_event, ULINK_CH_EVENT_COMPLETE);
}

static ulink_channel_inner_t *find_channel(ulink_channel_desc_t *desc,
                                           uint32_t rproc, uint32_t addr_sub)
{
    ulink_channel_inner_t *temp_ch;

    osMutexAcquire(desc->ch_lock, osWaitForever);
    list_for_every_entry (&desc->channels, temp_ch, ulink_channel_inner_t,
                          list) {
        if (temp_ch->ch.addr_sub == addr_sub) {
            osMutexRelease(desc->ch_lock);
            return temp_ch;
        }
    }
    osMutexRelease(desc->ch_lock);
    return NULL;
}

static void recive_thread(void *data)
{
    ulink_channel_desc_t *desc = data;
    ulink_channel_block_t *block =
        (ulink_channel_block_t *)(desc->rx_mem +
                                  desc->rx_head->rx * BLOCK_SIZE);
    ulink_channel_inner_t *ch_inner;

    while (1) {
        osEventFlagsWait(desc->event, ULINK_CH_EVENT_COMPLETE, osFlagsWaitAll,
                         osWaitForever);
		osEventFlagsClear(desc->event, 0x00);
        while (block->magic == MAGIC_READY) {
            ch_inner = find_channel(desc, block->addr_main, block->addr_sub);
          if (ch_inner)
		{
                	payload(ch_inner, block->payload, block->size);
		}
            block->magic = MAGIC_CLEAN;
            desc->rx_head->rx++;
            if (desc->rx_head->rx >= desc->rx_head->block_num)
                desc->rx_head->rx = 1;
            block = (ulink_channel_block_t *)(desc->rx_mem +
                                              desc->rx_head->rx * BLOCK_SIZE);
        }
    }
    return;
}

static void recive_gpio(void *data)
{
	ulink_channel_desc_t *desc = data;
	osEventFlagsSet(desc->event, ULINK_CH_EVENT_COMPLETE);
}

static int recive_mbox(void *arg, uint8_t *mssg, uint32_t len)
{
	ulink_channel_desc_t *desc = (ulink_channel_desc_t *)arg;
	osEventFlagsSet(desc->event, ULINK_CH_EVENT_COMPLETE);
	return 0;
}

static void channel_init(ulink_channel_desc_t *desc, uint32_t rproc)
{
    struct sdrv_mbox_chan_req chan_req;

    desc->tx_lock = osMutexNew(NULL);
    desc->rx_lock = osMutexNew(NULL);
    desc->ch_lock = osMutexNew(NULL);

    list_initialize(&desc->channels);
    desc->event = osEventFlagsNew(NULL);
    desc->tx_head = (ulink_channel_head_t *)desc->tx_mem;
    desc->tx_head->block_num = desc->mem_size / BLOCK_SIZE;
    if ((desc->tx_head->tx == 0) ||
        (desc->tx_head->tx >= desc->tx_head->block_num))
        desc->tx_head->tx = 1;
    desc->rx_head = (ulink_channel_head_t *)desc->rx_mem;
    desc->rx_head->block_num = desc->mem_size / BLOCK_SIZE;
    if ((desc->rx_head->rx == 0) ||
        (desc->rx_head->rx >= desc->rx_head->block_num))
        desc->rx_head->rx = 1;

    chan_req.rproc = rproc;
#ifdef CONFIG_KUNLUN_MBOX
    chan_req.src_addr = (int)desc->rx_irq;
    chan_req.dest_addr = (int)desc->rx_irq;
#endif

    if (desc->irq_type == ulink_irq_gpio)
        ulink_request_irq(desc->rx_irq, recive_gpio, desc);
    else if (desc->irq_type == ulink_irq_mbox) {
        desc->chan = mbox_request_channel(g_mbox_dev, &chan_req);
        mbox_set_callback(desc->chan, recive_mbox, (void *)desc);
   }

    osThreadNew(recive_thread, desc, NULL);
}

static void mini_client_init(ulink_mini_client_t *client)
{
	ulink_irq_set_route(client->irq_to_mbox, client->mbox_to_irq, client->rproc,
                        client->addr,client);
}

static int ulink_send(ulink_channel_desc_t *desc, uint32_t addr_main,
                      uint32_t addr_sub, uint8_t *data, uint32_t size)
{
    ulink_channel_block_t *block;
    int wait_count = 0;

    /*wait pcie ready*/
    if (desc->tx_head->block_num == 0xffffffff)
        // return -EBUSY;
        return -1;

    osMutexAcquire(desc->tx_lock, osWaitForever);
    block = (ulink_channel_block_t *)(desc->tx_mem +
                                      desc->tx_head->tx * BLOCK_SIZE);
    while (block->magic == MAGIC_READY) {
        osDelay(1000);
        if (wait_count++ > 1000) {
            osMutexRelease(desc->tx_lock);
            return -1;
        }
    }
    block->addr_main = addr_main;
    block->addr_sub = addr_sub;
    block->size = size;
    memcpy(block->payload, data, size);
    block->magic = MAGIC_READY;
    desc->tx_head->tx++;
    if (desc->tx_head->tx >= desc->tx_head->block_num)
        desc->tx_head->tx = 1;
    osMutexRelease(desc->tx_lock);
    if (desc->irq_type == ulink_irq_gpio)
    	{
		ulink_trigger_irq(desc->tx_irq);
    	}
    else if (desc->irq_type == ulink_irq_mbox) {
           mbox_send(desc->chan, 0, 0, 1000);
    }

    return 0;
}

void ulink_channel_init(void)
{
    int i;
    for (i = 0; i < ALL_DP_CPU_MAX; i++) {
        if (ulink_channel_server_desc[i] != NULL)
            channel_init(ulink_channel_server_desc[i], i);
    }

/* for X9U & cc  board */
    i = 0;
    while (ulink_mini_client[i].rproc < DP_CPU_MAX) {
        mini_client_init(&ulink_mini_client[i]);
    }
}

static bool check_channel_exist(ulink_channel_desc_t *desc, uint32_t addr_sub)
{
    ulink_channel_inner_t *temp_ch;

    osMutexAcquire(desc->ch_lock, osWaitForever);
    list_for_every_entry (&desc->channels, temp_ch, ulink_channel_inner_t,
                          list) {
        if (temp_ch->ch.addr_sub == addr_sub) {
            osMutexRelease(desc->ch_lock);
            return true;
        }
    }
    osMutexRelease(desc->ch_lock);
    return false;
}

ulink_channel_t *ulink_request_channel(uint32_t rproc, uint32_t addr_sub,
                                       ulink_rx_callback f)
{
    ulink_channel_desc_t *desc;
    ulink_channel_inner_t *ch_inner;

    if (rproc >= ALL_DP_CPU_MAX)
        return NULL;

    desc = ulink_channel_server_desc[rproc];
    if (desc == NULL)
        return NULL;

    if (check_channel_exist(desc, addr_sub))
        return NULL;

    ch_inner = malloc(sizeof(ulink_channel_inner_t));
    if (!ch_inner) {
        return NULL;
    }

    ch_inner->ch.rproc = rproc;
    ch_inner->ch.addr_sub = addr_sub;
    ch_inner->desc = desc;
    if (f)
        ch_inner->ch.callback = f;
    else
        ch_inner->rx_event = osEventFlagsNew(NULL);

    osMutexAcquire(desc->ch_lock, osWaitForever);
    list_add_head(&desc->channels, &ch_inner->list);
    osMutexRelease(desc->ch_lock);

    return &ch_inner->ch;
}

void ulink_release_channel(ulink_channel_t *ch)
{
    ulink_channel_inner_t *ch_inner =
        containerof(ch, ulink_channel_inner_t, ch);
    ulink_channel_desc_t *desc = ch_inner->desc;

    osMutexAcquire(desc->ch_lock, osWaitForever);
    list_delete(&ch_inner->list);
    osMutexRelease(desc->ch_lock);
    free(ch_inner);
}

int ulink_channel_send(ulink_channel_t *ch, uint8_t *data, uint32_t size)
{
    ulink_channel_inner_t *ch_inner =
        containerof(ch, ulink_channel_inner_t, ch);
    uint32_t send_size;

    if (size > (BLOCK_SIZE - sizeof(ulink_channel_block_t)))
        send_size = BLOCK_SIZE - sizeof(ulink_channel_block_t);
    else
        send_size = size;

    return ulink_send(ch_inner->desc, ch->rproc, ch->addr_sub, data, send_size);
}

uint32_t ulink_channel_rev(ulink_channel_t *ch, uint8_t **data)
{
    ulink_channel_inner_t *ch_inner =
        containerof(ch, ulink_channel_inner_t, ch);
    osEventFlagsWait(ch_inner->rx_event, ULINK_CH_EVENT_COMPLETE,
                     osFlagsWaitAll, osWaitForever);

    *data = ch_inner->rx_buf;
    return ch_inner->rx_size;
}
