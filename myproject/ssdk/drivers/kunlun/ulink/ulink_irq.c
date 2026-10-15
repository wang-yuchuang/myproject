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
#include <driver/gpio.h>

#include "ulink.h"
extern gpio_dev_t g_gpio_dev;

#include <driver/mbox.h>
#include <mbox_cfg.h>

#include "driver/irq.h"

#if defined(CONFIG_KUNLUN_MBOX)
#include <driver/kunlun/mbox/sdrv_mbox_drv.h>
#endif
#if 0
#define ULINK_IRQ_TYPE IRQ_TYPE_LEVEL_LOW

#if ((ULINK_IRQ_TYPE) == (IRQ_TYPE_LEVEL_HIGH))
#define TRI_GPIO_IRQ 1
#define CLR_GPIO_IRQ 0
#elif ((ULINK_IRQ_TYPE) == (IRQ_TYPE_LEVEL_LOW))
#define TRI_GPIO_IRQ 0
#define CLR_GPIO_IRQ 1
#endif
#endif
#define TRI_GPIO_IRQ 0
#define CLR_GPIO_IRQ 1


#define IRQ_MAX_HANDLE 0x100

#define IRQ_FIFO_TX CLIENT_BASE
#define IRQ_FIFO_RX SERVER_BASE

#define TIMER_VALUE 1

#define IRQ_INIT_MAGIC 0xDEADBEEF

#define ULINK_EVENT_COMPLETE   0x1




typedef struct {
    uint32_t irq_num[IRQ_MAX_HANDLE];
    uint32_t irq_tri[IRQ_MAX_HANDLE];
    uint32_t irq_clr[IRQ_MAX_HANDLE];
    uint32_t irq_reg_nums;
    uint32_t irq_magic;
}ulink_irq_status_t;

typedef struct {
    void(*handle)(void *);
    void *data;
} ulink_irq_desc_t;

static ulink_irq_status_t *irq_status_tx = (ulink_irq_status_t *)IRQ_FIFO_TX;
static ulink_irq_status_t *irq_status_rx = (ulink_irq_status_t *)IRQ_FIFO_RX;
static osMutexId_t          ulink_lock;
static bool tx_state;
static ulink_irq_desc_t ulink_irq_desc[IRQ_MAX_HANDLE];
static osEventFlagsId_t tx_event;
static osEventFlagsId_t rx_event;




//extern const domain_res_t g_gpio_res;

static void route_irq(uint8_t irq_num)
{
    if ((irq_num < IRQ_MAX_HANDLE) &&
            (ulink_irq_desc[irq_num].handle != NULL)) {
        ulink_irq_desc[irq_num].handle(ulink_irq_desc[irq_num].data);
    }
}

static void do_rx_irq(void *arg)
{
    int i;

    while(1) {
        osEventFlagsWait(rx_event, ULINK_EVENT_COMPLETE, osFlagsWaitAll, osWaitForever);
    	osEventFlagsSet(rx_event, 0);

        for (i = 0; i < irq_status_rx->irq_reg_nums; i++) {
            if (irq_status_rx->irq_clr[i] != irq_status_rx->irq_tri[i]) {
                irq_status_rx->irq_clr[i] = irq_status_rx->irq_tri[i];
            route_irq((uint8_t)(irq_status_rx->irq_num[i]));
            }
        }
    	gpio_int_enable(&g_gpio_dev, ULINK_GPIO_RX, true);
    }
    return;
}

static bool check_tx_done(void)
{
    void *dio_handle;
    int ret = false;
    int i;
   osEventFlagsSet(tx_event, 0);

	i = 0;
   if (irq_status_tx->irq_magic == IRQ_INIT_MAGIC) {
        for (i = 0; i < irq_status_tx->irq_reg_nums; i++) {
            if (irq_status_tx->irq_clr[i] != irq_status_tx->irq_tri[i])
                break;
        }
        if (i == irq_status_tx->irq_reg_nums) {
            ret = true;
        } else {
            ret = false;
        }
    } else {
        ret = true;
    }

    if (ret) {
        if (tx_state == true) {
	   gpio_set(&g_gpio_dev,ULINK_GPIO_TX,CLR_GPIO_IRQ);
           tx_state = false;
        }
    } else {
        if (tx_state == false) {
	    gpio_set(&g_gpio_dev,ULINK_GPIO_TX,TRI_GPIO_IRQ);
            tx_state = true;
        }
    }
    return ret;
}


static void ulink_irq_handle(struct gpio_dev *dev, int pin, void *arg)
{
	gpio_int_enable(&g_gpio_dev, ULINK_GPIO_RX, false);
	osEventFlagsSet(rx_event, ULINK_EVENT_COMPLETE);
}


static void do_tx_irq(void *arg)
{
    uint32_t irq;
  while(1) {
        if (check_tx_done()) {
           osEventFlagsWait(tx_event, ULINK_EVENT_COMPLETE, osFlagsWaitAll, osWaitForever);
        } else {
            osDelay(TIMER_VALUE * osKernelGetTickFreq() / 1000);
        }
    }
    return ;
}

static void route_irq_2_ap(void *data)
{
    ulink_mini_client_t *client = (ulink_mini_client_t *)data;
    mbox_send(client->chan, 0,0, 1000);
}

static int route_ap_2_irq(void *arg, uint8_t *mssg, uint32_t len)
{
    ulink_mini_client_t *client = (ulink_mini_client_t *)arg;
    ulink_trigger_irq(client->mbox_to_irq);

	return 0;
}

void ulink_irq_init(void)
{

#ifdef CONFIG_UNILINK_IRQ_FORWARD
    gpio_config(&g_gpio_dev, ULINK_GPIO_TX, GPIO_OUT);
    gpio_set(&g_gpio_dev,ULINK_GPIO_TX,CLR_GPIO_IRQ);
    tx_state = false;

    tx_event = osEventFlagsNew(NULL);
    osThreadNew(do_tx_irq, NULL, NULL);

    rx_event = osEventFlagsNew(NULL);
    osThreadNew(do_rx_irq, NULL, NULL);

/* for X9U  board */
    if (irq_status_rx->irq_reg_nums >= 0xff)
        irq_status_rx->irq_reg_nums = 0;

    irq_status_rx->irq_magic = IRQ_INIT_MAGIC;


    gpio_config(&g_gpio_dev,ULINK_GPIO_RX,GPIO_INTR_LOW_LEVEL);
    gpio_attach_irq(&g_gpio_dev,ULINK_GPIO_RX,ulink_irq_handle,NULL);
    gpio_int_enable(&g_gpio_dev, ULINK_GPIO_RX, true);
    ulink_lock = osMutexNew(NULL);
#else
    tx_state = false;
    tx_event = osEventFlagsNew(NULL);
    rx_event = osEventFlagsNew(NULL);
    ulink_lock = osMutexNew(NULL);
#endif
}

void ulink_trigger_irq(int8_t irq_num)
{
    void *dio_handle;
    int wait_count = 0;
    int i;
    uint32_t irq = 0;

    osMutexAcquire(ulink_lock, osWaitForever);
  if(irq_status_tx->irq_reg_nums > 100)
  {
  	irq_status_tx->irq_reg_nums  = 0;
  }
   for (i = 0; i < irq_status_tx->irq_reg_nums; i++) {
        if (irq_num == irq_status_tx->irq_num[i]) {
            irq_status_tx->irq_tri[i]++;
            break;
        }
    }

    osMutexRelease(ulink_lock);
    if (i < irq_status_tx->irq_reg_nums){
   	osEventFlagsSet(tx_event, ULINK_EVENT_COMPLETE);
	}
 
}

void ulink_request_irq(int8_t irq_num, void(*func)(void *), void *data)
{
    uint32_t irq_reg_nums;

    ulink_irq_desc[irq_num].handle = func;
    ulink_irq_desc[irq_num].data = data;
    irq_reg_nums = irq_status_rx->irq_reg_nums;
    irq_status_rx->irq_num[irq_reg_nums] = irq_num;
    irq_status_rx->irq_tri[irq_reg_nums] = 0;
    irq_status_rx->irq_clr[irq_reg_nums] = 0;
    irq_reg_nums++;
    irq_status_rx->irq_reg_nums = irq_reg_nums;
}

void ulink_irq_set_route(uint32_t irq_to_mbox, uint32_t mbox_to_irq,
                            uint32_t rproc, uint32_t addr,ulink_mini_client_t*client)
{

    struct sdrv_mbox_chan_req chan_req;
    chan_req.rproc = rproc;
 //   chan_req.src_addr = (int)mbox_to_irq;
//    chan_req.dest_addr = (int)mbox_to_irq;
    chan_req.src_addr = (int)addr;
    chan_req.dest_addr = (int)addr;

    if(client == NULL)
    {
    	return;
    }
    client->chan = mbox_request_channel(g_mbox_dev, &chan_req);
     if(client->chan == NULL)
     {
     	return;
     }
   mbox_set_callback(client->chan, route_ap_2_irq, (void *)client);
   ulink_request_irq(irq_to_mbox, route_irq_2_ap, (void *)client);


}


#if 0

static int do_ulinkd(int argc, const cmd_args *argv)
{
    void *dio_handle;
    hal_mb_chan_t *mchan;
    uint32_t irq_reg_nums;
    uint32_t irq_num;
    uint32_t i;
    uint32_t debug_magic;

    if (argc < 2) {
        printf("tx_stat rx_stat rx_irqs tx_irqs all");
        return 0;
    }


    if (!strcmp(argv[1].str, "tx_stat")) {
        debug_magic = 1;
    } else if (!strcmp(argv[1].str, "rx_stat")) {
        debug_magic = 2;
    } else if (!strcmp(argv[1].str, "rx_irqs")) {
        debug_magic = 4;
    } else if (!strcmp(argv[1].str, "tx_irqs")) {
        debug_magic = 8;
    } else if (!strcmp(argv[1].str, "all")) {
        debug_magic = 0xf;
    } else {
        debug_magic = 0;
    }

    if (debug_magic & 0x1) {
        printf("tx stat is %x\n", tx_state);
        printf("gpio stat is %x\n", gpio_get(&g_gpio_dev, ULINK_GPIO_TX));

    }

    if (debug_magic & 0x2) {
        printf("rx gpio stat is %x\n", gpio_get(&g_gpio_dev, ULINK_GPIO_RX));
    }

    if (debug_magic & 0x4) {
        irq_reg_nums = irq_status_rx->irq_reg_nums;
        printf("rx irq state: \n");
        printf("reg irq num is %d\n", irq_reg_nums);
        for (i = 0; i < irq_reg_nums; i++) {
            printf("irq%d \n", i);
            irq_num = irq_status_rx->irq_num[i];
            printf("irqnum is %x\n", irq_num);
            printf("irqtri is %x\n", irq_status_rx->irq_tri[i]);
            printf("irqclr is %x\n", irq_status_rx->irq_clr[i]);
            if (route_irq_2_ap == ulink_irq_desc[irq_num].handle) {
                printf("this irq send to ap\n");
                mchan = ulink_irq_desc[irq_num].data;
                printf("mbox is %x\n", mchan);
                printf("mbox rproc is %x\n", mchan->remote_proc);
                printf("mbox dest addr  is %x\n", mchan->dest_addr);
            }
        }
    }

    if (debug_magic & 0x8) {
        irq_reg_nums = irq_status_tx->irq_reg_nums;
        printf("tx irq state: \n");
        printf("reg irq num is %d\n", irq_reg_nums);
        for (i = 0; i < irq_reg_nums; i++) {
            printf("irq%d \n", i);
            irq_num = irq_status_tx->irq_num[i];
            printf("irqnum is %x\n", irq_num);
            printf("irqtri is %x\n", irq_status_tx->irq_tri[i]);
            printf("irqclr is %x\n", irq_status_tx->irq_clr[i]);
        }
    }
    return 0;
}
#endif

#if defined(WITH_LIB_CONSOLE)
STATIC_COMMAND_START
STATIC_COMMAND("ulinkd", "unilink debug module", do_ulinkd)
STATIC_COMMAND_END(ulinkd);
#endif
