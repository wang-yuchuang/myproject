#include <reg.h>
#include <cmsis_os2.h>
#include <res_loader.h>
#include <string.h>
#include <debug.h>
#include <errno.h>
#include <driver/memmap.h>
#include <arch/cache.h>
#include <malloc.h>

#include "vpu_config.h"
#include "vpu_proto_client.h"

#include "dev/virtio.h"
#include "virtio_ids.h"

#define VPU_SVC_RPORC_ID            MBOX_RPROC_MP
#define VPU_VIRTIO_FIND_TIMEOUT     2000
#define VPU_VIRTIO_EVENT_TIMEOUT    2000

#define RX_SIZE                     16
#define TX_SIZE                     16
#define VPU_PROTO_BUF_SIZE          512 // for data of sizeof(vpu_ipcc_data_t)
#define v2p(va)                     (paddr_t)(va)
#define p2v(pa)                     (vaddr_t)(pa)

// client TX -> svc RX
enum {
    VQ_TX = 0,
    VQ_RX,
    VQ_COUNT,
};

static int vpu_svc_virtio_id[] = {
    VIRTIO_ID_VIDEO_CODA988,
    VIRTIO_ID_VIDEO_WAVE412,
    VIRTIO_ID_VIDEO_WAVE521,
};

typedef struct vpu_client_virtio {
    uint8_t svc_rproc_id;
    uint8_t svc_virtio_id;
    uint8_t *RX_buffer;
    osSemaphoreId_t sem;
    osMutexId_t mutex;
    struct virtio_device *dev;
    vpu_ipcc_data_t msg;
} vpu_client_virtio_t;

static vpu_client_virtio_t g_vpu_client[MAX_NUM_VPU_CORE];

static int vpu_virtio_client_msg_handle(struct virtio_device *dev, uint ring,
                                        const struct vring_used_elem *e)
{
    vpu_client_virtio_t *vpu_client = dev->priv;

    if (ring == VQ_TX) {
        u16 i = e->id;
        while (i < TX_SIZE) {
            struct vring_desc *desc = virtio_desc_index_to_desc(dev, ring, i);
            u16 n = (desc->flags & VRING_DESC_F_NEXT) ? desc->next : TX_SIZE;

            virtio_free_desc(dev, ring, i);
            i = n;
        }
    } else {
        /* VQ_RX is supposed to send only one desc a time */
        struct vring_desc *desc = virtio_desc_index_to_desc(dev, ring, e->id);
        if (desc->len != sizeof(vpu_ipcc_data_t)) {
            ssdk_printf(SSDK_CRIT, "[%s][%d] invailed RX msg\n", __func__, __LINE__);

            memset(&vpu_client->msg, 0x0, sizeof(vpu_ipcc_data_t));
            virtio_submit_chain(dev, ring, e->id);

            /* release semaphore */
            osSemaphoreRelease (vpu_client->sem);
            return -EINVAL;
        }

        vpu_ipcc_data_t *ipcc_msg = (vpu_ipcc_data_t *)p2v(ap2p(desc->addr));
        arch_invalidate_cache_range((vaddr_t)ipcc_msg, sizeof(vpu_ipcc_data_t));

        memcpy(&vpu_client->msg, ipcc_msg, sizeof(vpu_ipcc_data_t));
        virtio_submit_chain(dev, ring, e->id);

        /* release semaphore */
        osSemaphoreRelease (vpu_client->sem);
    }

    return 0;
}

// just for test
int vpu_proto_send_msg_to_service(int core_idx, vpu_ipcc_data_t *pack)
{
    int ret = 0;
    uint16_t desc_index;
    struct vring_desc *desc = NULL;

    osMutexAcquire(g_vpu_client[core_idx].mutex, osWaitForever);

    if (!g_vpu_client[core_idx].dev) {
        uint16_t id;

        g_vpu_client[core_idx].svc_rproc_id = VPU_SVC_RPORC_ID;
        g_vpu_client[core_idx].svc_virtio_id = vpu_svc_virtio_id[core_idx];

        virtio_mmio_find_timeout(g_vpu_client[core_idx].svc_rproc_id,
                                 g_vpu_client[core_idx].svc_virtio_id, &g_vpu_client[core_idx].dev,
                                 VPU_VIRTIO_FIND_TIMEOUT);

        if (!g_vpu_client[core_idx].dev) {
            ssdk_printf(SSDK_CRIT,
                        "[%s][%d] find vpu virtual io svc failed, rproc id:%d, virtio id:%d\n",
                        __func__, __LINE__, g_vpu_client[core_idx].svc_rproc_id,
                        g_vpu_client[core_idx].svc_virtio_id);
            ret = -1;
            goto DONE;
        }

        /* reset device to start */
        virtio_reset_device(g_vpu_client[core_idx].dev);

        /* check features here */
        uint64_t features = virtio_mmio_read_device_features(g_vpu_client[core_idx].dev);

        /* settle down features here */
        features &= 0;
        virtio_mmio_write_driver_features(g_vpu_client[core_idx].dev, features);

        /* read configs */
        // volatile struct virtio_mmc_config *cfg = g_vpu_client[core_idx].dev->config_ptr;
        // hexdump((void *)cfg, sizeof(struct virtio_mmc_config));

        /* we've seen the device */
        virtio_status_acknowledge_driver(g_vpu_client[core_idx].dev);

        /* alloc rings */
        ret = virtio_alloc_ring(g_vpu_client[core_idx].dev, VQ_TX, TX_SIZE);
        ret |= virtio_alloc_ring(g_vpu_client[core_idx].dev, VQ_RX, RX_SIZE);

        if (ret) {
            ssdk_printf(SSDK_CRIT, "[%s][%d] fail to alloc virtio ring\n", __func__, __LINE__);
            goto DONE;
        }

        g_vpu_client[core_idx].dev->irq_driver_callback = vpu_virtio_client_msg_handle;
        g_vpu_client[core_idx].dev->priv = &g_vpu_client[core_idx];

        virtio_status_driver_ok(g_vpu_client[core_idx].dev);

        // fill rx queue to serivce
        g_vpu_client[core_idx].RX_buffer = memalign(1024, RX_SIZE * VPU_PROTO_BUF_SIZE);
        if (!g_vpu_client[core_idx].RX_buffer) {
            ssdk_printf(SSDK_CRIT, "[%s][%d] failed alloc rx buffer\n", __func__, __LINE__);
            ret = -1;
            goto DONE;
        }

        for (int i = 0; i < RX_SIZE; i++) {
            id = virtio_alloc_desc(g_vpu_client[core_idx].dev, VQ_RX);
            desc = virtio_desc_index_to_desc(g_vpu_client[core_idx].dev, VQ_RX, id);

            desc->addr =
                p2ap(v2p((void *)g_vpu_client[core_idx].RX_buffer + i * VPU_PROTO_BUF_SIZE));
            desc->len = sizeof(vpu_ipcc_data_t);
            virtio_submit_chain(g_vpu_client[core_idx].dev, VQ_RX, id);
        }
    }

    if (!g_vpu_client[core_idx].dev) {
        ssdk_printf(SSDK_CRIT, "[%s][%d] invailed dev\n", __func__, __LINE__);
        ret = -1;
        goto DONE;
    }

    desc = virtio_alloc_desc_chain(g_vpu_client[core_idx].dev, VQ_TX, 1, &desc_index);

    if (desc == NULL) {
        ssdk_printf(SSDK_CRIT, "[%s][%d] not enough desc\n", __func__, __LINE__);
        ret = -1;
        goto DONE;
    }

    desc->addr = p2ap(v2p((void *)pack));
    desc->len = sizeof(vpu_ipcc_data_t);
    arch_clean_cache_range((vaddr_t)pack, sizeof(vpu_ipcc_data_t));
    virtio_submit_chain(g_vpu_client[core_idx].dev, VQ_TX, desc_index);
    virtio_kick(g_vpu_client[core_idx].dev, VQ_TX);

    if (osErrorTimeout ==
        osSemaphoreAcquire (g_vpu_client[core_idx].sem, VPU_VIRTIO_EVENT_TIMEOUT)) {
        ssdk_printf(SSDK_CRIT, "[%s][%d] vpu waiting event timedout\n", __func__, __LINE__);
        ret = -1;
        goto DONE;
    }

    memcpy(pack, &g_vpu_client[core_idx].msg, sizeof(vpu_ipcc_data_t));

DONE:
    osSemaphoreRelease(g_vpu_client[core_idx].sem);
    osMutexRelease (g_vpu_client[core_idx].mutex);
    return ret;
}

void vpu_virtio_local_client_init(void)
{
    for (int i = 0; i < MAX_NUM_VPU_CORE; i++) {
        g_vpu_client[i].mutex = osMutexNew(NULL);
        g_vpu_client[i].sem = osSemaphoreNew (1, 0, NULL);
    }
}