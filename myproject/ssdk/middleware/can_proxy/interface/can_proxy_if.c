/*
 * Interface of can proxy
 * Copyright (c) 2020 Semidrive Semiconductor, Inc.
 * All rights reserved.
 */
#include <debug.h>
#include <assert.h>
#include <can_proxy.h>
#include <sd_errcode.h>

typedef struct cp_if_set_t {
    cp_if_t interface[cpifBusMax][MAX_INTERFACE_PER_BUS];
    uint32_t if_id_bitmap[cpifBusMax];
} cp_if_set_t;

static cp_if_set_t xLocalCPIFS;
static int dst_ap = DST_AP;
static int dst_saf = DST_SAF;

cp_if_t *cp_get_interface(int bus, uint32_t if_id)
{
    if (bus >= cpifBusMax || if_id >= MAX_INTERFACE_PER_BUS)
        return NULL;

    return &xLocalCPIFS.interface[bus][if_id];
}

static void cp_interface_dispatch(cp_if_t *cpif)
{
    cp_if_set_t *cpifs = &xLocalCPIFS;
    void *pdata = NULL;
    size_t size = 0;
    uint32_t bus_bitmap;
    uint32_t if_id_bitmap;
    size = cp_bundle_detach(cpif->bind_if_id, &pdata);

    if (!size) {
        printf("error to detach!\n");
        return;
    }

    CPDBG("==== Dispatcher %d Bundle:0x%x ====\n", size,
          cpif->bind_if_id);
    REV_BUNDLE_ID(cpif->bind_if_id, bus_bitmap, if_id_bitmap);

    for (int bus = 0; bus < cpifBusMax; bus++) {
        if (BUS_BITMAP(bus) & bus_bitmap) {
            CPDBG("Found Target bus %d\n", bus);

            for (int if_id = 0; if_id < MAX_INTERFACE_PER_BUS; if_id++) {
                if (INTERFACE_ID_BITMAP(if_id) & if_id_bitmap &&
                        INTERFACE_ID_BITMAP(if_id) & cpifs->if_id_bitmap[bus]) {
                    CPDBG("Found Target Interface %d\n", if_id);

                    if (cpifs->interface[bus][if_id].tx &&
                            (cpifs->interface[bus][if_id].if_state == cpifStateRunning)) {
                        cpifs->interface[bus][if_id].tx(&cpifs->interface[bus][if_id], pdata,
                                                            size);
                    }
                }
            }
        }
    }
}

/* main worker of routing */
static void cp_interface_worker(void *dst)
{
    cp_if_set_t *cpifs = &xLocalCPIFS;
    cp_if_t *pif = NULL;
    size_t len = 0;
    __CP_CREATE_PFRAME(cp_if_frame_t, CP_CAN_MTU, frame);
    int dst_i = *((int *)dst);

    while (1) {
        pif = NULL;
        len = cp_if_frame_get(frame, dst_i, CP_CAN_MTU);

        if (len) {
            CPDBG("Worker recv:%d\n", len);
            pif = &cpifs->interface[frame->bus][frame->if_id];

            if (pif->rxcb) {
                pif->rxcb(pif, frame->data, len);
            }
            else {
                CPDBG("if type %d bus %d has no rxcb\n", frame->bus, frame->if_id);
            }
        }
        else {
            CPDBG("Worker recv error!\n");
            return;
        }

        cp_interface_dispatch(pif);
    }
}

void cp_interface_start(void)
{
    for (int if_id = 0; if_id < MAX_INTERFACE_CAN_BUS; if_id++) {
        if (xLocalCPIFS.interface[cpifBusCan][if_id].init) {
            CPDBG("init %d:%d cp interface\n", cpifBusCan, if_id);

            if (xLocalCPIFS.interface[cpifBusCan][if_id].init(
                        &xLocalCPIFS.interface[cpifBusCan][if_id])) {
                CPDBG("init %d:%d cp interface fail.\n", cpifBusCan, if_id);
                xLocalCPIFS.if_id_bitmap[cpifBusCan] &= ~(INTERFACE_ID_BITMAP(if_id));
            }
        }
    }

    osThreadAttr_t rpmsg_attr = {.name = "cp_interface_worker", .stack_size = 1024, .priority = 9};
    osThreadId_t *interface_worker_to_rpmsg = osThreadNew(cp_interface_worker, (void *)&dst_ap, &rpmsg_attr);
    if (!interface_worker_to_rpmsg) {
        ERRCODE_PRINT(MOD_ID_CANFD, 0xff, CAN_ERR_NO_THREAD, 0U);
        ssdk_printf(SSDK_CRIT, "can proxy rpmsg thread create err\n");
    }

    osThreadAttr_t can_attr = {.name = "cp_interface_worker", .stack_size = 1024, .priority = 9};
    osThreadId_t *interface_worker_to_can = osThreadNew(cp_interface_worker, (void *)&dst_saf, &can_attr);
    if (!interface_worker_to_can) {
        ERRCODE_PRINT(MOD_ID_CANFD, 0xff, CAN_ERR_NO_THREAD, 0U);
        ssdk_printf(SSDK_CRIT, "can proxy can thread create err\n");
    }
}

void cp_interface_setting(
    cp_if_bus_t bus,
    uint32_t if_id,
    cp_if_init_t init_proc,
    cp_if_tx_t tx_proc,
    cp_if_rx_cb_t rxcb_proc,
    void *container,
    size_t ctn_size)
{
    ASSERT(bus < cpifBusMax && if_id < MAX_INTERFACE_PER_BUS);
    cp_if_t *pif = &xLocalCPIFS.interface[bus][if_id];
    pif->bus = bus;
    pif->if_id = if_id;
    pif->init = init_proc;
    pif->tx = tx_proc;
    pif->rxcb = rxcb_proc;
    pif->container = container;
    pif->ctn_size = ctn_size;
    pif->bind_if_id = NO_BIND;
    xLocalCPIFS.if_id_bitmap[bus] |= INTERFACE_ID_BITMAP(if_id);
    CPDBG("Setting interface:%d:%d bitmap:0x%x\n", bus, if_id,
          xLocalCPIFS.if_id_bitmap[bus]);
}

void cp_interface_duplicate(cp_if_bus_t bus, uint32_t if_id,
                            void *container)
{
    ASSERT(bus < cpifBusMax && if_id < MAX_INTERFACE_PER_BUS);
    cp_if_t *pif = &xLocalCPIFS.interface[bus][if_id];

    for (int i = 0; i < MAX_INTERFACE_PER_BUS; i++) {
        if (INTERFACE_ID_BITMAP(i) & xLocalCPIFS.if_id_bitmap[bus]) {
            CPDBG("Found Duplicated Interface %d\n", i);
            pif->bus = bus;
            pif->if_id = if_id;
            pif->init = xLocalCPIFS.interface[bus][i].init;
            pif->tx = xLocalCPIFS.interface[bus][i].tx;
            pif->rxcb = xLocalCPIFS.interface[bus][i].rxcb;
            pif->container = container;
            pif->ctn_size = xLocalCPIFS.interface[bus][i].ctn_size;
            break;
        }
    }

    xLocalCPIFS.if_id_bitmap[bus] |= INTERFACE_ID_BITMAP(if_id);
    CPDBG("Init interface:%d:%d bitmap:0x%x\n", bus, if_id,
          xLocalCPIFS.if_id_bitmap[bus]);
}