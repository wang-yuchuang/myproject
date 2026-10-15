#include <debug.h>
#include <stdlib.h>
#include <string.h>
#include <rpmsg.h>
#include <sd_errcode.h>

#include "rpmsg_internal.h"

struct rpmsg_endpoint *rpmsg_create_ept(struct rpmsg_instance *ins,
                                        unsigned long addr,
                                        rpmsg_rx_cb_t rx_cb,
                                        void *rx_cb_data)
{
    struct rpmsg_endpoint *ept = RPMSG_NULL;

    osMutexAcquire(ins->mutex, osWaitForever);

    if (addr == RPMSG_ADDR_ANY) {
        for (unsigned long i = 1; i < RPMSG_ADDR_ANY; i++) {
            if (rpmsg_get_ept(ins, i) == RPMSG_NULL) {
                addr = i;
                break;
            }
        }

        if (addr == RPMSG_ADDR_ANY) {
            ERRCODE_PRINT(MOD_ID_SYS_SERVICE_COMM,
                    ins->remote_proc, COM_IPCC_HAL_RECV_MSG_DROP, 0);
            goto ret;
        }
    }
    else if (rpmsg_get_ept(ins, addr) != RPMSG_NULL) {
        ERRCODE_PRINT(MOD_ID_SYS_SERVICE_COMM,
                ins->remote_proc, COM_IPCC_RPMSG_CREATE_EXIST, addr);
        ept = RPMSG_NULL;
        goto ret;
    }

    ept = osAlloc(sizeof(struct rpmsg_endpoint));
    if (ept == RPMSG_NULL) {
        ERRCODE_PRINT(MOD_ID_SYS_SERVICE_COMM, ins->remote_proc,
                COM_IPCC_MALLOC_FAIL, sizeof(struct rpmsg_endpoint));
        goto ret;
    }

    memset(ept, 0x0, sizeof(struct rpmsg_endpoint));

    ept->addr = addr;
    ept->rx_cb = rx_cb;
    ept->rx_cb_data = rx_cb_data;
    list_add_tail(&ins->endpoints, &ept->node);

ret:
    osMutexRelease(ins->mutex);
    return ept;
}

int rpmsg_destroy_ept(struct rpmsg_instance *ins, struct rpmsg_endpoint *ept)
{
    osMutexAcquire(ins->mutex, osWaitForever);
    list_delete(&ept->node);
    osMutexRelease(ins->mutex);
    osFree(ept);
    return RPMSG_SUCCESS;
}

int rpmsg_send(struct rpmsg_instance *ins,
               struct rpmsg_endpoint *ept,
               unsigned long dst,
               char *data,
               unsigned long size,
               unsigned long timeout)
{
    void *buffer;
    int ret;
    unsigned long alloc_size = size;

    if (!rpmsg_is_link_up(ins)) {
        ERRCODE_PRINT(MOD_ID_SYS_SERVICE_COMM, ins->remote_proc,
                COM_IPCC_RPMSG_SEND_NOT_READY, 0);
        return RPMSG_NOT_READY;
    }

    if (ins->lower_ops->tx_direct != NULL) {
        osMutexAcquire(ins->mutex, osWaitForever);
        ret = ins->lower_ops->tx_direct(ins, ept->addr, dst, data, size, timeout);
        osMutexRelease(ins->mutex);
        return ret;
    }

    buffer = rpmsg_alloc_tx_buffer(ins, &alloc_size, timeout);
    if (buffer == RPMSG_NULL) {
        ERRCODE_PRINT(MOD_ID_SYS_SERVICE_COMM, ins->remote_proc,
                COM_IPCC_RPMSG_SEND_NO_BUF, size);
        return RPMSG_ERR_NO_MEM;
    }

    size = MIN(size, alloc_size);
    memcpy(buffer, data, size);

    return rpmsg_send_nocopy(ins, ept, dst, buffer, size);
}

int rpmsg_send_nocopy(struct rpmsg_instance *ins,
                      struct rpmsg_endpoint *ept,
                      unsigned long dst,
                      void *data,
                      unsigned long size)
{
    struct rpmsg_std_msg *rpmsg_msg;
    unsigned long src;
    struct rpmsg_hdr_reserved *reserved;
    int retval;

    if (!rpmsg_is_link_up(ins)) {
        ERRCODE_PRINT(MOD_ID_SYS_SERVICE_COMM, ins->remote_proc,
                COM_IPCC_RPMSG_SEND_NOT_READY, 0);
        return RPMSG_NOT_READY;
    }

    src = ept->addr;
    rpmsg_msg = RPMSG_STD_MSG_FROM_BUF(data);

    /* Initialize RPMSG header. */
    rpmsg_msg->hdr.dst = dst;
    rpmsg_msg->hdr.src = src;
    rpmsg_msg->hdr.len = size;
    rpmsg_msg->hdr.flags = 0;

    reserved = (struct rpmsg_hdr_reserved *)&rpmsg_msg->hdr.reserved;

    osMutexAcquire(ins->mutex, osWaitForever);
    retval = ins->lower_ops->tx(ins, (void *)rpmsg_msg,
        size + sizeof(struct rpmsg_std_hdr), reserved->idx);
    osMutexRelease(ins->mutex);

    return retval;
}

void *rpmsg_alloc_tx_buffer(struct rpmsg_instance *ins,
                            unsigned long *size,
                            unsigned long timeout)
{
    struct rpmsg_std_msg *rpmsg_msg;
    struct rpmsg_hdr_reserved *reserved;
    void *buffer;
    unsigned short idx;
    unsigned int tick_count = 0;
    unsigned long alloc_size;

    if (!rpmsg_is_link_up(ins)) {
        ERRCODE_PRINT(MOD_ID_SYS_SERVICE_COMM, ins->remote_proc,
                COM_IPCC_RPMSG_SEND_NOT_READY, 0);
        return RPMSG_NULL;
    }

    alloc_size = *size + sizeof(struct rpmsg_std_hdr);

    osMutexAcquire(ins->mutex, osWaitForever);
    buffer = ins->lower_ops->tx_alloc(ins, &alloc_size, &idx);
    osMutexRelease(ins->mutex);

    if (buffer == RPMSG_NULL && timeout == 0) {
        ERRCODE_PRINT(MOD_ID_SYS_SERVICE_COMM, ins->remote_proc,
                COM_IPCC_RPMSG_SEND_NO_BUF, *size);
        return RPMSG_NULL;
    }

    while (buffer == RPMSG_NULL) {
        osDelay(RPMSG_MS_2_TICK(RPMSG_MS_PER_INTERVAL));

        osMutexAcquire(ins->mutex, osWaitForever);
        buffer = ins->lower_ops->tx_alloc(ins, &alloc_size, &idx);
        osMutexRelease(ins->mutex);

        tick_count += RPMSG_MS_PER_INTERVAL;
        if (tick_count >= timeout && buffer == RPMSG_NULL) {
            ERRCODE_PRINT(MOD_ID_SYS_SERVICE_COMM, ins->remote_proc,
                    COM_IPCC_RPMSG_SEND_NO_BUF, *size);
            return RPMSG_NULL;
        }
    }

    rpmsg_msg = (struct rpmsg_std_msg *)buffer;
    /* keep idx and totlen information for nocopy tx function */
    reserved = (struct rpmsg_hdr_reserved *)&rpmsg_msg->hdr.reserved;
    reserved->idx = idx;
    *size = alloc_size - sizeof(struct rpmsg_std_hdr);

    return rpmsg_msg->data;
}

int rpmsg_release_rx_buffer(struct rpmsg_instance *ins, void *rxbuf)
{
    struct rpmsg_std_msg *rpmsg_msg;
    struct rpmsg_hdr_reserved *reserved;
    int retval;

    rpmsg_msg = RPMSG_STD_MSG_FROM_BUF(rxbuf);
    /* Get the pointer to the reserved field that contains buffer size and the index */
    reserved = (struct rpmsg_hdr_reserved *)&rpmsg_msg->hdr.reserved;

    osMutexAcquire(ins->mutex, osWaitForever);
    retval = ins->lower_ops->rx_free(ins, rpmsg_msg,
        rpmsg_msg->hdr.len + sizeof(struct rpmsg_std_hdr), reserved->idx);
    osMutexRelease(ins->mutex);

    return retval;
}

int rpmsg_is_link_up(struct rpmsg_instance *ins)
{
    if (ins && ins->ins_ops->get_link_status) {
        return (ins->ins_ops->get_link_status(ins) == RPMSG_LINKSTATE_UP);
    }
    else {
        return RPMSG_FALSE;
    }
}

int rpmsg_kick_remote(struct rpmsg_instance *ins)
{
    if (ins && ins->ins_ops->kick_remote) {
        ins->ins_ops->kick_remote(ins);
    }

    return RPMSG_SUCCESS;
}

struct rpmsg_endpoint *rpmsg_get_ept(struct rpmsg_instance *ins,
                                     unsigned long addr)
{
    struct rpmsg_endpoint *ept;

    if (!ins)
        return RPMSG_NULL;

    list_for_every_entry(&ins->endpoints, ept,
                         struct rpmsg_endpoint, node) {
        if (ept->addr == addr) {
            return ept;
        }
    }

    ERRCODE_PRINT(MOD_ID_SYS_SERVICE_COMM,
            ins->remote_proc, COM_IPCC_RPMSG_NO_EPT, addr);

    return RPMSG_NULL;
}

int rpmsg_master_init(struct rpmsg_instance *ins, void *cfg)
{
    if (!ins) {
        ERRCODE_PRINT(MOD_ID_SYS_SERVICE_COMM,
                0, COM_IPCC_INVALID_ARGS, 0);
        return RPMSG_ERR_PARAM;
    }

    ins->mutex = osMutexNew(RPMSG_NULL);
    list_initialize(&ins->endpoints);
    return ins->ins_ops->master_init(ins, cfg);
}

int rpmsg_remote_init(struct rpmsg_instance *ins, void *cfg)
{
    if (!ins) {
        ERRCODE_PRINT(MOD_ID_SYS_SERVICE_COMM,
                0, COM_IPCC_INVALID_ARGS, 0);
        return RPMSG_ERR_PARAM;
    }

    ins->mutex = osMutexNew(RPMSG_NULL);
    list_initialize(&ins->endpoints);
    return ins->ins_ops->remote_init(ins, cfg);
}

int rpmsg_deinit(struct rpmsg_instance *ins)
{
    if (!ins) {
        ERRCODE_PRINT(MOD_ID_SYS_SERVICE_COMM,
                0, COM_IPCC_INVALID_ARGS, 0);
        return RPMSG_ERR_PARAM;
    }

    struct rpmsg_endpoint *ept;

    ins->ins_ops->deinit(ins);

    osMutexAcquire(ins->mutex, osWaitForever);
    ept = list_remove_head_type(&ins->endpoints, struct rpmsg_endpoint, node);
    while (ept != RPMSG_NULL) {
        osFree(ept);
        ept = list_remove_head_type(&ins->endpoints, struct rpmsg_endpoint, node);
    }
    osMutexRelease(ins->mutex);

    osMutexDelete(ins->mutex);
    return RPMSG_SUCCESS;
}
