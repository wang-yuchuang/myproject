/*
 * Copyright (c) 2022 Semidrive Semiconductor, Inc.
 * All rights reserved.
 */

#include <driver/kunlun/mbox/sdrv_mbox_drv.h>
#include <reg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vpu_svc_cmd.h"
#include "vpu_virtio_svc.h"

#include <arch/cache.h>
#include <cmsis_os2.h>
#include <debug.h>
#include <driver/memmap.h>
#include <image_cfg.h>
#include <param.h>
#include <sdrv-virtio.h>
#include <sys/types.h>
#include <virtio_device.h>
#include <virtio_ids.h>
#include <virtio_queue.h>

#define v2p(va) (paddr_t)(va)
#define p2v(pa) (vaddr_t)(pa)

/*
 * RX & TX SIZE must be a power of 2,
 * otherwise, there will be problems with
 * unsigned values in overflow processing.
 **/
#define RX_SIZE 16
#define TX_SIZE 16

enum {
    VQ_RX = 0,
    VQ_TX,
    VQ_COUNT,
};

// ssdk for x9 mp core only used on x9hp and x9u, so enadble ap1 & ap2 here
static int vpu_rproc_id[] = {
    MBOX_RPROC_AP1, // 3,
    MBOX_RPROC_AP2, // 4,
    MBOX_RPROC_MP,
};

static char *vpu_virtio_name[][VPU_DEVICES_MAX] = {
    // ap1:
    {
        "VPU_CODA988_VIRTIO_AP1",
        "VPU_WAVE412_VIRTIO_AP1",
        "VPU_WAVE521_VIRTIO_AP1",
    },

    // ap2:
    {
        "VPU_CODA988_VIRTIO_AP2",
        "VPU_WAVE412_VIRTIO_AP2",
        "VPU_WAVE521_VIRTIO_AP2",
    },

    //mp
    {
        "VPU_CODA988_VIRTIO_MP",
        "VPU_WAVE412_VIRTIO_MP",
        "VPU_WAVE521_VIRTIO_MP",
    }
};

typedef struct vpu_virtio_info {
    void *vdev;
    uint16_t last[VQ_COUNT];
    uint32_t size[VQ_COUNT];
    uint64_t features;
    struct virtio_device_id dev_id;
    struct virtio_device_ops vpu_virtio_ops;

    int dev_idx;
    int ip_idx;
} vpu_virtio_info_t;

typedef struct vpu_virtio_svc {
    vpu_virtio_info_t vvi[VPU_MAX_IPCC_DEVICE][VPU_DEVICES_MAX];
    osMessageQueueId_t vpu_stream_queue[VPU_DEVICES_MAX];
    osThreadId_t *stream_handler;
} vpu_virtio_svc_t;

static vpu_virtio_svc_t g_vpu_virtio_svc;

static void vpu_virtio_set_cfg(void *dev, uint32_t offset, uint32_t len)
{
    vpu_virtio_info_t *vvi = dev;

    ssdk_printf(SSDK_INFO, "[%s][%d] vvi:%p, frontend writes %u bytes to %u \n", __func__, __LINE__,
                vvi, offset, len);
}

static void vpu_virtio_reset(void *dev)
{
    vpu_virtio_info_t *vvi = dev;

    ssdk_printf(SSDK_INFO, "[%s][%d] vvi:%p\n", __func__, __LINE__, vvi);

    for (int i = 0; i < VQ_COUNT; i++) {
        vvi->last[i] = 0;
    }
}

static int vpu_send_msg(int ipcc_devces_index, int vpu_devices_index, char *buf, int len)
{
    struct virtq_desc *desc;
    struct virtq_avail *avail;
    struct virtq_used *used;
    vpu_virtio_info_t *vvi;

    uint32_t id, num;
    uint16_t idx;
    char *dst;

    vvi = &g_vpu_virtio_svc.vvi[ipcc_devces_index][vpu_devices_index];
    num = virtio_device_num_get(vvi->vdev, VQ_TX);
    desc = (struct virtq_desc *)virtio_device_desc_get(vvi->vdev, VQ_TX);
    avail = (struct virtq_avail *)virtio_device_avail_get(vvi->vdev, VQ_TX);
    used = (struct virtq_used *)virtio_device_used_get(vvi->vdev, VQ_TX);
    if (!num || !avail || !desc || !used) {
        ssdk_printf(SSDK_ERR, "[%s][%d] %p: tx queue not ready!\n", __func__, __LINE__, vvi);
        return -1;
    } else {
        ssdk_printf(SSDK_DEBUG,
                    "[%s][%d] num:%d, avail->idx:%d desc:%p used->idx:%d last[VQ_TX]:%d\n",
                    __func__, __LINE__, num, avail->idx, desc, used->idx, vvi->last[VQ_TX]);
    }

    /* read index of next desc from avail queue */
    if (vvi->last[VQ_TX] == avail->idx) {
        ssdk_printf(SSDK_ERR, "[%s][%d] not enough desc, avail->idx:%x, last[VQ_TX]:%x\n", __func__,
                    __LINE__, avail->idx, vvi->last[VQ_TX]);
        return -1;
    }

    idx = vvi->last[VQ_TX] % num;
    id = avail->ring[idx];
    ssdk_printf(SSDK_DEBUG, "[%s][%d] vvi->last[VQ_TX]:%d, num:%d id:%d\n", __func__, __LINE__,
                vvi->last[VQ_TX], num, id);

    /* read buf info from desc */
    ssdk_printf(SSDK_DEBUG, "%p: send desc[%u] addr %llx len %u flags %u next %u\n", vvi, id,
                desc[id].addr, desc[id].len, desc[id].flags, desc[id].next);

    /* prepare data */
    dst = (char *)p2v(ap2p(desc[id].addr));
    memcpy(dst, buf, len);
    arch_clean_cache_range((addr_t)dst, len);
    ssdk_printf(SSDK_DEBUG, "[%s][%d] desc[id].addr:%llx, dst:%p, len:%d\n", __func__, __LINE__,
                desc[id].addr, dst, len);

    /* write index of next desc to used queue */
    idx = used->idx % num;
    used->ring[idx].id = id;
    used->ring[idx].len = len;
    ssdk_printf(SSDK_DEBUG, "[%s][%d] vused->idx:%d, num:%d id:%d\n", __func__, __LINE__, used->idx,
                num, id);

    used->idx++;

    /* kick remote */
    vvi->last[VQ_TX]++;
    virtio_device_notify(vvi->vdev);

    return 0;
}

static void vpu_task(void *para)
{
    vpu_ipcc_data_t pack;
    int dev_id = (int)para;

    if (dev_id >= VPU_DEVICES_MAX) {
        ssdk_printf(SSDK_ERR, "vpu_svc:invailed dev id:%d\n", dev_id);
        return;
    }

    while (1) {
        osMessageQueueGet(g_vpu_virtio_svc.vpu_stream_queue[dev_id], &pack, NULL, osWaitForever);
        vpu_service_process_the_request(&pack, dev_id);
    }
}

static void vpu_virtio_notify(void *dev)
{
    vpu_virtio_info_t *vvi = dev;
    vpu_ipcc_data_t *data = NULL;
    struct virtq_desc *desc;
    struct virtq_avail *avail;
    struct virtq_used *used;
    vaddr_t addr;
    u32 id, num;
    u16 idx, last_idx;
    int notify = 0;

    /* no need to handle TX */
    num = virtio_device_num_get(vvi->vdev, VQ_RX);
    desc = (struct virtq_desc *)virtio_device_desc_get(vvi->vdev, VQ_RX); // virtual queue base
    avail = (struct virtq_avail *)virtio_device_avail_get(vvi->vdev, VQ_RX);
    used = (struct virtq_used *)virtio_device_used_get(vvi->vdev, VQ_RX);
    if (!num || !avail || !desc || !used) {
        ssdk_printf(SSDK_ERR, "[%s][%d] %p: rx queue not ready!\n", __func__, __LINE__, vvi);
        return;
    } else {
        ssdk_printf(SSDK_DEBUG, "[%s][%d] num:%d, avail->idx:%d desc:%p used->idx:%d\n", __func__,
                    __LINE__, num, avail->idx, desc, used->idx);
    }

    for (;;) {
        /* avail->ring is a ring buffer, so when vpu_virtio_svc->last[VQ_RX] == avail->idx,
         * it means there is no data (the ring buffer is empty).
         * avail->idx points to the buffer idx to be filled,
         * vpu_virtio_svc->last[VQ_RX] instead indicates the buffer idx to be processed
         **/
        if (vvi->last[VQ_RX] == avail->idx)
            break;

        /*
         * which is responsible for handling the case
         * where vpu_virtio_svc->last[VQ_RX] exceeds the num index
         **/
        last_idx = vvi->last[VQ_RX] % num;
        id = avail->ring[last_idx];
        ssdk_printf(SSDK_DEBUG, "[%s][%d] vvi->last[VQ_RX]:%d, num:%d id:%d\n", __func__, __LINE__,
                    vvi->last[VQ_RX], num, id);

        /* read desc */
        addr = p2v(ap2p(desc[id].addr));
        arch_invalidate_cache_range((addr_t)addr, sizeof(vpu_ipcc_data_t));
        data = (vpu_ipcc_data_t *)addr;

        /* desc[id].addr is the physical address of the ap side,
         * which is first converted into the physical address of safety,
         * and then converted into a virtual address for the cpu access of safety
         **/
        ssdk_printf(SSDK_DEBUG,
                    "[%s][%d] desc[id].addr:%llx, addr:%x, ip_idx:%d, dev_idx:%d, cmd:%d len:%d\n",
                    __func__, __LINE__, desc[id].addr, addr, vvi->ip_idx, vvi->dev_idx, data->cmd,
                    data->cmd_param_full_length);

        /* handle data */
        data->ipcc_id = vvi->ip_idx;
        data->dev_id = vvi->dev_idx;
        osMessageQueuePut(g_vpu_virtio_svc.vpu_stream_queue[data->dev_id], data, 0, 0);

        /* write index of desc to used queue,
         * and the used structure is maintained by us,
         * so we don't need to reload the cache
         */
        idx = used->idx % num;
        used->ring[idx].id = id;
        used->ring[idx].len = 0;
        ssdk_printf(SSDK_DEBUG, "[%s][%d] vused->idx:%d, num:%d id:%d\n", __func__, __LINE__,
                    used->idx, num, id);

        used->idx++;

        vvi->last[VQ_RX]++;
        notify = 1;
    }

    if (notify)
        virtio_device_notify(vvi->vdev);
}

void vpu_virtio_svc_entry()
{
    int ret = 0;
    osThreadAttr_t vpu_svc_task_attr = {
        .name = "vpu_svc",
        .stack_size = 4096,
        .priority = osPriorityAboveNormal,
    };

    if (ARRAY_SIZE(vpu_rproc_id) == 0) {
        ssdk_printf(SSDK_ERR, "[%s][%d] will no vpu client terminal\n", __func__, __LINE__);
        return;
    }

    for (int i = 0; i < VPU_DEVICES_MAX; i++) {
        g_vpu_virtio_svc.vpu_stream_queue[i] =
            osMessageQueueNew(VPU_MAX_MESSAGE_QUEUE_COUNT, sizeof(vpu_ipcc_data_t), NULL);

        /*The CPU usage of vpu service is less than 10%,
         *and the main functions are completed by vpu hareware,
         *so increase the task priority of vpu service
         **/
        g_vpu_virtio_svc.stream_handler = osThreadNew(vpu_task, (void *)i, &vpu_svc_task_attr);
        if (!g_vpu_virtio_svc.stream_handler) {
            ssdk_printf(SSDK_ERR, "[%s][%d] create thread failed.\r\n", __func__, __LINE__);
            osMessageQueueDelete(g_vpu_virtio_svc.vpu_stream_queue[i]);
            return;
        }
    }

    ret = vpu_service_init_msg_process(vpu_send_msg);
    if (ret < 0) {
        ssdk_printf(SSDK_ERR, "vpu_service_init_msg_process failed\n");
        return;
    }

    for (unsigned int ip_idx = 0; ip_idx < ARRAY_SIZE(vpu_rproc_id); ip_idx++) {
        for (int dev_idx = 0; dev_idx < VPU_DEVICES_MAX; dev_idx++) {
            g_vpu_virtio_svc.vvi[ip_idx][dev_idx].dev_id.device = VIRTIO_ID_VIDEO_CODA988 + dev_idx;
            g_vpu_virtio_svc.vvi[ip_idx][dev_idx].dev_id.vendor =
                0xffffffff; /* match any vendor id */
            g_vpu_virtio_svc.vvi[ip_idx][dev_idx].size[VQ_RX] = RX_SIZE;
            g_vpu_virtio_svc.vvi[ip_idx][dev_idx].size[VQ_TX] = TX_SIZE;
            g_vpu_virtio_svc.vvi[ip_idx][dev_idx].features = (1ull << 32); /* VIRTIO_F_VERSION_1 */
            g_vpu_virtio_svc.vvi[ip_idx][dev_idx].vpu_virtio_ops.set_cfg = vpu_virtio_set_cfg;
            g_vpu_virtio_svc.vvi[ip_idx][dev_idx].vpu_virtio_ops.reset = vpu_virtio_reset;
            g_vpu_virtio_svc.vvi[ip_idx][dev_idx].vpu_virtio_ops.notify = vpu_virtio_notify;
            g_vpu_virtio_svc.vvi[ip_idx][dev_idx].dev_idx = dev_idx;
            g_vpu_virtio_svc.vvi[ip_idx][dev_idx].ip_idx = ip_idx;

            g_vpu_virtio_svc.vvi[ip_idx][dev_idx].vdev = virtio_device_create_common(
                g_vpu_virtio_svc.vvi[ip_idx][dev_idx].dev_id,
                g_vpu_virtio_svc.vvi[ip_idx][dev_idx].features,
                &g_vpu_virtio_svc.vvi[ip_idx][dev_idx].vpu_virtio_ops, VQ_COUNT,
                g_vpu_virtio_svc.vvi[ip_idx][dev_idx].size, vpu_rproc_id[ip_idx],
                vpu_virtio_name[ip_idx][dev_idx], osPriorityAboveNormal,
                &g_vpu_virtio_svc.vvi[ip_idx][dev_idx]);

            if (!g_vpu_virtio_svc.vvi[ip_idx][dev_idx].vdev) {
                ssdk_printf(SSDK_ERR, "[%s][%d] virtio_device_create_mmio failed\n", __func__,
                            __LINE__);
                return;
            }
        }
    }
}
