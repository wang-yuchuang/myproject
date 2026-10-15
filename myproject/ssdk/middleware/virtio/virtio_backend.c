#include <CLI_type.h>
#include <debug.h>
#include <driver/kunlun/mbox/sdrv_mbox_drv.h>
#include <driver/mbox.h>
#include <driver/memmap.h>
#include <lib/list.h>
#include <mbox_cfg.h>
#include <malloc.h>
#include <reg.h>
#include <regs_base.h>
#include <spinlock.h>
#include <stdlib.h>
#include <string.h>

#include "virtio_common.h"
#include <sdrv-virtio.h>
#include <virtio_device.h>
#include <virtio_ids.h>
#include <virtio_mmio.h>
typedef u16 le16;
typedef u32 le32;
typedef u64 le64;
#include <virtio_queue.h>


/* local debug */
#define LOCAL_DEBUG     0

#if LOCAL_DEBUG
#undef SSDK_INFO
#define SSDK_INFO       SSDK_CRIT
#undef SSDK_DEBUG
#define SSDK_DEBUG      SSDK_CRIT
#endif

#define __SA_COMPATIBLE 1

#if __SA_COMPATIBLE
#include "virtio_device_sa.h"

typedef struct {
    int sa;
    void *dev;
} wrapper_t;
#endif

#define p2v(pa)     ((vaddr_t)(pa))
#define v2p(va)     ((paddr_t)(va))

typedef struct {
    msg_t msg;
    u32 timeout;
} vm_t;

typedef struct {
    u32 max_size;
    u32 ready;

    u32 num;
    void *desc;
    void *avail;
    void *used;
} vq_t;

#define VD_ACTION_NOTIFY    0x1
/* same as thread name length */
#define VD_NAME_MAXLEN      32
#define VD_MAX_NVQS         2
#define VD_MAX_VQ_LEN       128

typedef struct {
#define VD_EVENT_KICK       0x1
    osEventFlagsId_t event;
    u32 action;
    u8 offset, len;
    osThreadId_t task;

    struct virtio_device_id id;
    u64 features;
    u64 driver_features;
    u32 cur_status;
    u32 irq_status;
    u32 config_gen;
    struct virtio_device_ops *ops;
    void *priv;

    struct list_node list;
    void *vs;
    u32 vid;
    char name[VD_NAME_MAXLEN];

    paddr_t reg;
    vaddr_t base;

    u32 nvqs;
    vq_t vqs[];
} vd_t;

enum {
    VQ_RX = 0,
    VQ_TX,
    VQ_COUNT,
};

#define RX_LEN 16
#define TX_LEN 16

typedef struct {
    void *vdev;
    /* if we don't make debug action, tx_lock can be omitted */
    osMutexId_t tx_lock;
    u16 last[VQ_COUNT];
    int ready;

    /* provider is for ap domain */
    int ap_domain;
} vps_t;

#define SERVER_LOCAL        1
#define SERVER_MBOX_SHMEM   2

#define VS_MSG_COUNT    16u
#define VS_MSG_MASK     (VS_MSG_COUNT-1)

typedef struct {
    void *vbe;
    int domain;
    int type;
    int ready;

    spin_lock_t lock;
    u32 cur_vid;
    struct list_node dev_list;

    /* provider service */
    vps_t *vps;

    /* LOCAL BEGIN */
    vbo_t vbo;
    /* LOCAL END */

    /* REMOTE BEGIN */
    spin_lock_t msg_lock;
    int __full;
    u16 __ri, __wi;
    vm_t __vms[VS_MSG_COUNT];

#define VS_EVENT_KICK       0x1
    osEventFlagsId_t event;
    osThreadId_t task;

    sdrv_mbox_rproc_t rproc;
    u32 addr;
    paddr_t reg;
    mbox_chan_t *chan;
    /* REMOTE END */
} vs_t;

typedef struct {
    int domain;
    vs_t servers[DOMAIN_MAX];
} vbe_t;

static vbe_t *__vbe = NULL;

static void virtio_server_send_msg(vs_t *vs, msg_t *msg, u32 timeout)
{
    irq_state_t state;
    int send = 0;

    state = spin_lock_irqsave(&vs->msg_lock);
    if (vs->__wi != vs->__ri || !vs->__full) {
        vs->__vms[vs->__wi].msg = *msg;
        vs->__vms[vs->__wi].timeout = timeout;

        vs->__wi = (vs->__wi + 1) & VS_MSG_MASK;
        vs->__full = vs->__wi == vs->__ri;

        send = 1;
    }
    spin_unlock_irqrestore(&vs->msg_lock, state);

    if (send) {
        osEventFlagsSet(vs->event, VS_EVENT_KICK);
    } else {
        ssdk_printf(SSDK_CRIT,
                "vbe: %s msg q full, omiting %x\n",
                get_domain_name(vs->domain), *(u32 *)msg);
    }
}

/* interface */
u32 virtio_device_num_get(void *vdev, u32 vq)
{
#if __SA_COMPATIBLE
    wrapper_t *wrapper = vdev;

    ssdk_printf(SSDK_DEBUG,
            "vbe: wrapper %p dev %p sa %d\n",
            wrapper, wrapper->dev, wrapper->sa);

    if (wrapper->sa) {
        return virtio_device_num_get_sa(wrapper->dev, vq);
    } else {
        vdev = wrapper->dev;
    }
#endif

    vd_t *vd = vdev;

    return vd->vqs[vq].ready ? vd->vqs[vq].num : 0;
}

/* interface */
void *virtio_device_desc_get(void *vdev, u32 vq)
{
#if __SA_COMPATIBLE
    wrapper_t *wrapper = vdev;

    if (wrapper->sa) {
        return virtio_device_desc_get_sa(wrapper->dev, vq);
    } else {
        vdev = wrapper->dev;
    }
#endif

    vd_t *vd = vdev;

    return vd->vqs[vq].ready ? vd->vqs[vq].desc : NULL;
}

/* interface */
void *virtio_device_avail_get(void *vdev, u32 vq)
{
#if __SA_COMPATIBLE
    wrapper_t *wrapper = vdev;

    if (wrapper->sa) {
        return virtio_device_avail_get_sa(wrapper->dev, vq);
    } else {
        vdev = wrapper->dev;
    }
#endif

    vd_t *vd = vdev;

    return vd->vqs[vq].ready ? vd->vqs[vq].avail : NULL;
}

/* interface */
void *virtio_device_used_get(void *vdev, u32 vq)
{
#if __SA_COMPATIBLE
    wrapper_t *wrapper = vdev;

    if (wrapper->sa) {
        return virtio_device_used_get_sa(wrapper->dev, vq);
    } else {
        vdev = wrapper->dev;
    }
#endif

    vd_t *vd = vdev;

    return vd->vqs[vq].ready ? vd->vqs[vq].used : NULL;
}

/* interface */
void *virtio_device_config_area_get(void *vdev)
{
#if __SA_COMPATIBLE
    wrapper_t *wrapper = vdev;

    if (wrapper->sa) {
        return virtio_device_config_area_get_sa(wrapper->dev);
    } else {
        vdev = wrapper->dev;
    }
#endif

    vd_t *vd = vdev;

    return (void *)vd->base + VIRTIO_MMIO_CONFIG;
}

/* interface */
void virtio_device_notify(void *vdev)
{
#if __SA_COMPATIBLE
    wrapper_t *wrapper = vdev;

    if (wrapper->sa) {
        virtio_device_notify_sa(wrapper->dev);
        return;
    } else {
        vdev = wrapper->dev;
    }
#endif

    vd_t *vd = vdev;

    if (!vd)
        return;

    /* TODO simulate interrupt clear */
    writel(VIRTIO_MMIO_INT_VRING, vd->base + VIRTIO_MMIO_INTERRUPT_STATUS);

    vs_t *vs = vd->vs;

    if (vs->type == SERVER_MBOX_SHMEM) {
        msg_t msg;
        memset(&msg, 0, sizeof(msg));
        msg.mt = VM_MT_IRQ;
        msg.vid = vd->vid & VM_VID_MASK;

        /* TODO consider move irq to provider */
        virtio_server_send_msg(vd->vs, &msg, 50);
    } else if (vs->type == SERVER_LOCAL) {
        virtio_agent_send_irq(vd->vid);
    } else {
        ssdk_printf(SSDK_CRIT,
                "vbe: %s invalid type %d\n",
                get_domain_name(vs->domain), vs->type);
    }
}

static void virtio_provider_send_text(vps_t *vps, const char *str)
{
    struct virtq_desc *desc;
    struct virtq_avail *avail;
    struct virtq_used *used;
    vpm_t *vpm;
    u32 id, num;
    u16 idx;
    size_t len, maxlen;
    paddr_t pa;
    vaddr_t va;
    osStatus_t err;

    num = virtio_device_num_get(vps->vdev, VQ_TX);
    desc = (struct virtq_desc *)virtio_device_desc_get(vps->vdev, VQ_TX);
    avail = (struct virtq_avail *)virtio_device_avail_get(vps->vdev, VQ_TX);
    used = (struct virtq_used *)virtio_device_used_get(vps->vdev, VQ_TX);
    if (!num || !avail || !desc || !used) {
        ssdk_printf(SSDK_CRIT, "vbe: provider TX not ready\n");
        return;
    }

    err = osMutexAcquire(vps->tx_lock, 1000);
    if (err != osOK) {
        ssdk_printf(SSDK_CRIT, "vbe: provider timeout %d\n", err);
        return;
    }

    if (vps->last[VQ_TX] == avail->idx) {
        ssdk_printf(SSDK_CRIT, "vbe: provider TX full\n");
        goto release;
    }

    idx = vps->last[VQ_TX] & (num - 1);
    id = avail->ring[idx];
    if (id >= num) {
        ssdk_printf(SSDK_CRIT, "vbe: invalid avail[%u] id %u\n", idx, id);
        goto release;
    }

    ssdk_printf(SSDK_INFO, "vbe: avail[%u] id %u\n", idx, id);

    pa = desc[id].addr;
    if (vps->ap_domain) {
        pa = ap2p(pa);
    }
    va = p2v(pa);

    len = strlen(str);
    /* deduct type, len and '\0' */
    maxlen = desc[id].len - sizeof(u32) - sizeof(u32) - 1;
    if (len > maxlen) {
        ssdk_printf(SSDK_INFO, "vbe: trim to %zu bytes\n", maxlen);
        len = maxlen;
    }

    vpm = (vpm_t *)va;
    vpm->type = VPM_TEXT;
    vpm->text.len = len;
    memcpy(vpm->text.str, str, len);
    vpm->text.str[len] = '\0';

    idx = used->idx & (num - 1);
    used->ring[idx].id = id;
    used->ring[idx].len = len + sizeof(u32) + sizeof(u32) + 1;
    used->idx++;

    ssdk_printf(SSDK_INFO, "vbe: used[%u] id %u\n", idx, id);

    vps->last[VQ_TX]++;
    osMutexRelease(vps->tx_lock);

    virtio_device_notify(vps->vdev);

    printf("vbe: send %u bytes\n", len);
    return;

release:
    osMutexRelease(vps->tx_lock);
}

static void virtio_provider_notify_device(vps_t *vps, vd_t *vd, int add)
{
    struct virtq_desc *desc;
    struct virtq_avail *avail;
    struct virtq_used *used;
    vpm_t *vpm;
    vd_t *vd2;
    u32 id, num;
    u16 idx;
    paddr_t pa;
    vaddr_t va;
    osStatus_t err;

#if __SA_COMPATIBLE
    wrapper_t *wrapper = vps->vdev;
    vd2 = (vd_t *)wrapper->dev;
#else
    vd2 = (vd_t *)vps->vdev;
#endif
    if (!(vd2->cur_status & (1 << 2)/* VIRTIO_STATUS_DRIVER_OK */)) {
        /*
         * fe has not initialized us so far
         * just return here to let fe query devices later
         */
        return;
    }

    num = virtio_device_num_get(vps->vdev, VQ_TX);
    desc = (struct virtq_desc *)virtio_device_desc_get(vps->vdev, VQ_TX);
    avail = (struct virtq_avail *)virtio_device_avail_get(vps->vdev, VQ_TX);
    used = (struct virtq_used *)virtio_device_used_get(vps->vdev, VQ_TX);
    if (!num || !avail || !desc || !used) {
        ssdk_printf(SSDK_CRIT, "vbe: provider TX not ready\n");
        return;
    }

    err = osMutexAcquire(vps->tx_lock, 1000);
    if (err != osOK) {
        ssdk_printf(SSDK_CRIT, "vbe: provider timeout %d\n", err);
        return;
    }

    if (vps->last[VQ_TX] == avail->idx) {
        ssdk_printf(SSDK_CRIT, "vbe: provider TX full\n");
        goto release;
    }

    idx = vps->last[VQ_TX] & (num - 1);
    id = avail->ring[idx];
    if (id >= num) {
        ssdk_printf(SSDK_CRIT, "vbe: invalid avail[%u] id %u\n", idx, id);
        goto release;
    }

    ssdk_printf(SSDK_INFO, "vbe: avail[%u] id %u\n", idx, id);

    pa = desc[id].addr;
    if (vps->ap_domain) {
        pa = ap2p(pa);
    }
    va = p2v(pa);

    vpm = (vpm_t *)va;
    vpm->type = VPM_DEVICE;
    vpm->device.num = 1;
    /* TODO mask (!add << 31) to notify a device remove */
    vpm->device.devs[0].vid = vd->vid;
    pa = vd->reg;
    if (vps->ap_domain) {
        pa = p2ap(pa);
    }
    vpm->device.devs[0].addr = pa;

    idx = used->idx & (num - 1);
    used->ring[idx].id = id;
    used->ring[idx].len = sizeof(u32) + sizeof(u32) + sizeof(vdi_t);
    used->idx++;

    ssdk_printf(SSDK_INFO, "vbe: used[%u] id %u\n", idx, id);

    vps->last[VQ_TX]++;
    osMutexRelease(vps->tx_lock);

    virtio_device_notify(vps->vdev);

    return;

release:
    osMutexRelease(vps->tx_lock);
}

static void virtio_provider_set_cfg(void *dev, u32 offset, u32 length)
{
    vps_t *vps = dev;

    if (length != sizeof(u32)) {
        ssdk_printf(SSDK_CRIT, "vbe: invalid cfg length %u\n", length);
        return;
    }

    ssdk_printf(SSDK_INFO,
            "vbe: set cfg offset %u length %u\n",
            offset, length);

    switch (offset) {
    case VP_CFG_ADDR_AP_DOMAIN:
        {
            void *cfg = virtio_device_config_area_get(vps->vdev);
            vps->ap_domain = readl(cfg + VP_CFG_ADDR_AP_DOMAIN);
        }
        break;
    default:
        {
            ssdk_printf(SSDK_CRIT,
                    "vbe: unknown offset %x for provider cfg\n",
                    offset);
        }
        break;
    }
}

static void virtio_provider_reset(void *dev)
{
    vps_t *vps = dev;

    vps->last[VQ_RX] = 0;
    vps->last[VQ_TX] = 0;
}

static void virtio_provider_notify(void *dev)
{
    vps_t *vps = dev;
    vs_t *vs;
    vd_t *vd, *tmp;
    struct virtq_desc *desc;
    struct virtq_avail *avail;
    struct virtq_used *used;
    u32 id, num, dev_num;
    u16 idx, last;
    int notify;
    paddr_t pa;
    vaddr_t va;
    vpm_t *vpm;
    irq_state_t state;
    struct list_node *node;
    int i;

    /* only need to handle RX */
    num = virtio_device_num_get(vps->vdev, VQ_RX);
    desc = (struct virtq_desc *)virtio_device_desc_get(vps->vdev, VQ_RX);
    avail = (struct virtq_avail *)virtio_device_avail_get(vps->vdev, VQ_RX);
    used = (struct virtq_used *)virtio_device_used_get(vps->vdev, VQ_RX);
    if (!num || !avail || !desc || !used) {
        ssdk_printf(SSDK_CRIT, "vbe: provider RX not ready\n");
        return;
    }

#if __SA_COMPATIBLE
    wrapper_t *wrapper = vps->vdev;
    vd = wrapper->dev;
#else
    vd = vps->vdev;
#endif
    vs = vd->vs;

    notify = 0;
    for (;;) {
        if (vps->last[VQ_RX] == avail->idx)
            break;

        last = vps->last[VQ_RX] & (num - 1);
        id = avail->ring[last];
        if (id >= num) {
            ssdk_printf(SSDK_CRIT, "vbe: invalid avail[%u] id %u\n", last, id);

            /* return this wrong id to peer */
            idx = used->idx & (num - 1);
            used->ring[idx].id = id;
            used->ring[idx].len = 0;
            used->idx++;

            vps->last[VQ_RX]++;
            continue;
        }

        pa = desc[id].addr;
        if (vps->ap_domain) {
            pa = ap2p(pa);
        }
        va = p2v(pa);

        /* provider data is also mapped as uc */
        vpm = (vpm_t *)va;
        if (vpm->type == VPM_TEXT) {
            printf("vbe: recv %u bytes from %s:\n",
                    vpm->text.len, get_domain_name(vs->domain));
            /* ensure ends with '\0' */
            vpm->text.str[vpm->text.len] = '\0';
            printf("vbe: %s\n", vpm->text.str);
        } else if (vpm->type == VPM_TIMESTAMP) {
            vpm->timestamp.server_recv = readl(p2v(APB_SYS_CNT_RO_BASE));
            vpm->timestamp.server_send = readl(p2v(APB_SYS_CNT_RO_BASE));
        } else if (vpm->type == VPM_DEVICE) {
            /* peer querys all devices, omit us self */
            state = spin_lock_irqsave(&vs->lock);
            dev_num = list_length(&vs->dev_list);
            vpm->device.num = dev_num - 1;
            if (dev_num > 1) {
                for (node = vs->dev_list.next->next, i = 0;
                        node != &vs->dev_list;
                        node = node->next, i++) {
                    tmp = containerof(node, vd_t, list);
                    vpm->device.devs[i].vid = tmp->vid;
                    pa = tmp->reg;
                    if (vps->ap_domain) {
                        pa = p2ap(pa);
                    }
                    vpm->device.devs[i].addr = pa;
                }
            }
            spin_unlock_irqrestore(&vs->lock, state);
        } else if (vpm->type == VPM_ARITHMETIC) {
            /* arithmetic msg may comes in groups */
            struct virtq_desc *_desc= &desc[id];
            vpm_t *_vpm = vpm;
            paddr_t _pa;

            for (;;) {
                if (_vpm->arithmetic._operator == '+') {
                    _vpm->arithmetic.result =
                        _vpm->arithmetic.operand1 + _vpm->arithmetic.operand2;
                } else if (_vpm->arithmetic._operator == '-') {
                    _vpm->arithmetic.result =
                        _vpm->arithmetic.operand1 - _vpm->arithmetic.operand2;
                } else if (_vpm->arithmetic._operator == '*') {
                    _vpm->arithmetic.result =
                        _vpm->arithmetic.operand1 * _vpm->arithmetic.operand2;
                } else if (_vpm->arithmetic._operator == '/') {
                    _vpm->arithmetic.result =
                        _vpm->arithmetic.operand1 / _vpm->arithmetic.operand2;
                } else {
                    _vpm->arithmetic.result = -1;
                }

                if (_desc->flags & VIRTQ_DESC_F_NEXT && (_desc->next < num)) {
                    _desc = &desc[_desc->next];
                    _pa = _desc->addr;
                    if (vps->ap_domain) {
                        _pa = ap2p(_pa);
                    }
                    _vpm = (vpm_t *)p2v(_pa);
                } else {
                    break;
                }
            }
        } else {
            ssdk_printf(SSDK_CRIT, "vbe: invalid msg type %u\n", vpm->type);
        }

        idx = used->idx & (num - 1);
        used->ring[idx].id = id;
        used->ring[idx].len = 0;
        used->idx++;

        vps->last[VQ_RX]++;
        notify = 1;
    }

    if (notify) {
        virtio_device_notify(vps->vdev);
    }
}

static struct virtio_device_ops virtio_provider_ops = {
    .set_cfg = virtio_provider_set_cfg,
    .reset = virtio_provider_reset,
    .notify = virtio_provider_notify,
};

static void virtio_device_init_regs(vd_t *vd)
{
    writel(0x74726976, vd->base + VIRTIO_MMIO_MAGIC_VALUE);
    writel(0x2, vd->base + VIRTIO_MMIO_VERSION);
    writel(vd->id.device, vd->base + VIRTIO_MMIO_DEVICE_ID);
    writel(vd->id.vendor, vd->base + VIRTIO_MMIO_VENDOR_ID);
    writel(0, vd->base + VIRTIO_MMIO_QUEUE_READY);
    writel(vd->irq_status, vd->base + VIRTIO_MMIO_INTERRUPT_STATUS);
    writel(0, vd->base + VIRTIO_MMIO_STATUS);
    writel(vd->config_gen, vd->base + VIRTIO_MMIO_CONFIG_GENERATION);
}

static void virtio_device_thread(void *arg)
{
    vd_t *vd = arg;
    u32 flags;

    for (;;) {
        flags = osEventFlagsWait(vd->event,
                VD_EVENT_KICK, osFlagsWaitAny, osWaitForever);

        /* if any error, just wait again */
        if (flags != VD_EVENT_KICK)
            continue;

        if (vd->action & VD_ACTION_NOTIFY) {
            vd->action &= ~VD_ACTION_NOTIFY;

            if (vd->ops->notify)
                vd->ops->notify(vd->priv);
        }
    }
}

static void *virtio_server_create_device(vs_t *vs, const char *name,
        osPriority_t priority, struct virtio_device_id id, u64 features,
        struct virtio_device_ops *ops, u32 nvqs, u32 max_sizes[], void *priv)
{
    vd_t *vd;
    irq_state_t state;
    osThreadAttr_t attr;

    vd = malloc(sizeof(vd_t) + nvqs * sizeof(vq_t));
    if (!vd) {
        ssdk_printf(SSDK_CRIT,
                "vbe: fail to alloc vd for device %u\n",
                id.device);
        return NULL;
    }

    strncpy(vd->name, name ? : "vdev", VD_NAME_MAXLEN-1);
    vd->name[VD_NAME_MAXLEN-1] = '\0';

    vd->event = osEventFlagsNew(NULL);
    vd->action = 0;

    memset(&attr, 0, sizeof(attr));
    attr.name = vd->name;
    attr.stack_size = 4096,
    attr.priority = priority;
    vd->task = osThreadNew(virtio_device_thread, vd, &attr);
    if (!vd->task) {
        ssdk_printf(SSDK_CRIT,
                "vbe: fail to create task for device %u\n",
                id.device);
        free(vd);
        return NULL;
    }

    vd->id = id;
    vd->features = features;
    vd->driver_features = 0;
    vd->cur_status = 0;
    vd->irq_status = 0;
    vd->config_gen = 0;
    vd->ops = ops;
    vd->priv = priv;

    vd->nvqs = nvqs;
    for (u32 i = 0; i < vd->nvqs; i++) {
        vd->vqs[i].max_size = max_sizes[i];
        vd->vqs[i].ready = 0;
    }

    list_initialize(&vd->list);
    vd->vs = vs;

    if (vs->type == SERVER_MBOX_SHMEM) {
        if (vd->id.device == VIRTIO_ID_PROVIDER) {
            state = spin_lock_irqsave(&vs->lock);
            vd->vid = 0;
            list_add_tail(&vs->dev_list, &vd->list);
            spin_unlock_irqrestore(&vs->lock, state);
        } else if (vd->id.device == VIRTIO_ID_SDHCI) {
            /* reserve id for sdhci since androidboot needs to know exact
             * name of boot device */
            state = spin_lock_irqsave(&vs->lock);
            vd->vid = 1;
            list_add_tail(&vs->dev_list, &vd->list);
            spin_unlock_irqrestore(&vs->lock, state);
        } else {
            state = spin_lock_irqsave(&vs->lock);
            vd->vid = vs->cur_vid++;
            list_add_tail(&vs->dev_list, &vd->list);
            spin_unlock_irqrestore(&vs->lock, state);
        }

        vd->reg = vs->reg
            + vd->vid * (VIRTIO_DEVICE_REG_SIZE + VIRTIO_DEVICE_CFG_SIZE);
        vd->base = p2v(vd->reg);
    } else if (vs->type == SERVER_LOCAL) {
        vd->base = (vaddr_t)memalign(0x100,
                VIRTIO_DEVICE_REG_SIZE + VIRTIO_DEVICE_CFG_SIZE);
        vd->reg = v2p((void *)vd->base);

        if (!vd->base) {
            ssdk_printf(SSDK_CRIT, "vbe: fail to alloc reg memory\n");
            free(vd);
            return NULL;
        }

        state = spin_lock_irqsave(&vs->lock);
        vd->vid = vs->cur_vid++;
        list_add_tail(&vs->dev_list, &vd->list);
        spin_unlock_irqrestore(&vs->lock, state);
    }

    virtio_device_init_regs(vd);

    ssdk_printf(SSDK_INFO,
            "vbe: create device %u at %x\n",
            vd->vid, vd->reg);

    /* notify frontend we have a new device */
    if (vd->vid) {
        virtio_provider_notify_device(vs->vps, vd, 1);
    }

    return vd;
}

static inline bool is_rproc_supported(sdrv_mbox_rproc_t rproc)
{
    return MBOX_RPROC_SAF <= rproc && rproc <= MBOX_RPROC_AP2;
}

/* interface */
void *virtio_device_create_mmio(struct virtio_device_id id, u64 features,
        struct virtio_device_ops *ops, u32 nvqs, u32 max_sizes[],
        u32 reg, u32 cfg_size, sdrv_mbox_rproc_t mbox_rproc, int mbox_addr, void *priv)
{
#if __SA_COMPATIBLE
    void *ptr = virtio_device_create_mmio_sa(id, features, ops,
            nvqs, max_sizes, reg, cfg_size, mbox_rproc, mbox_addr, priv);

    if (!ptr)
        return NULL;

    wrapper_t *wrapper = malloc(sizeof(wrapper_t));
    if (!wrapper) {
        ssdk_printf(SSDK_CRIT, "vbe: fail to alloc wrapper\n");
        return NULL;
    }

    wrapper->sa = 1;
    wrapper->dev = ptr;

    ssdk_printf(SSDK_DEBUG,
            "vbe: wrapper %p dev %p mmio\n",
            wrapper, wrapper->dev);

    return wrapper;
#else
    vbe_t *vbe = __vbe;
    if (!vbe) {
        ssdk_printf(SSDK_CRIT, "vbe: not ready\n");
        return NULL;
    }

    if (!ops) {
        ssdk_printf(SSDK_CRIT,
                "vbe: invalid empty ops for device %u\n",
                id.device);
        return NULL;
    }

    if (!nvqs || nvqs > VD_MAX_NVQS) {
        ssdk_printf(SSDK_CRIT,
                "vbe: invalid vq num %u for device %u\n",
                nvqs, id.device);
        return NULL;
    }

    for (u32 i = 0; i < nvqs; i++) {
        if (max_sizes[i] > VD_MAX_VQ_LEN) {
            ssdk_printf(SSDK_CRIT,
                    "vbe: invalid size %u for vq %u\n",
                    max_sizes[i], i);
            return NULL;
        }
    }

    if (!is_rproc_supported(mbox_rproc)) {
        ssdk_printf(SSDK_CRIT,
                "vbe: currently rproc %d is not supported\n",
                mbox_rproc);
        return NULL;
    }

    int domain = get_domain_from_rproc(mbox_rproc);
    if (domain < 0 || domain >= DOMAIN_MAX) {
        ssdk_printf(SSDK_CRIT,
                "vbe: invalid rproc %u\n", rproc);
        return NULL;
    }

    vs_t *vs = &vbe->servers[domain];
    if (!vs->ready) {
        ssdk_printf(SSDK_CRIT,
                "vbe: %s server not ready\n",
                get_domain_name(domain));
        return NULL;
    }

    return virtio_server_create_device(vs, "vdev",
            osPriorityNormal, id, features, ops, nvqs, max_sizes, priv);
#endif
}

/* interface */
void *virtio_device_create_common(struct virtio_device_id id, u64 features,
        struct virtio_device_ops *ops, u32 nvqs, u32 max_sizes[], sdrv_mbox_rproc_t rproc,
        const char *name, osPriority_t priority, void *priv)
{
    vbe_t *vbe = __vbe;
    if (!vbe) {
        ssdk_printf(SSDK_CRIT, "vbe: not ready\n");
        return NULL;
    }

    if (!ops) {
        ssdk_printf(SSDK_CRIT,
                "vbe: invalid empty ops for device %u\n",
                id.device);
        return NULL;
    }

    if (!nvqs || nvqs > VD_MAX_NVQS) {
        ssdk_printf(SSDK_CRIT,
                "vbe: invalid vq num %u for device %u\n",
                nvqs, id.device);
        return NULL;
    }

    for (u32 i = 0; i < nvqs; i++) {
        if (max_sizes[i] > VD_MAX_VQ_LEN) {
            ssdk_printf(SSDK_CRIT,
                    "vbe: invalid size %u for vq %u\n",
                    max_sizes[i], i);
            return NULL;
        }
    }

    int domain = get_domain_from_rproc(rproc);
    if (domain < 0 || domain >= DOMAIN_MAX) {
        ssdk_printf(SSDK_CRIT,
                "vbe: invalid rproc %u\n", rproc);
        return NULL;
    }

    vs_t *vs = &vbe->servers[domain];
    if (!vs->ready) {
        ssdk_printf(SSDK_CRIT,
                "vbe: %s server not ready\n",
                get_domain_name(domain));
        return NULL;
    }

#if __SA_COMPATIBLE
    void *ptr = virtio_server_create_device(vs, name,
            priority, id, features, ops, nvqs, max_sizes, priv);
    if (!ptr)
        return NULL;

    wrapper_t *wrapper = malloc(sizeof(wrapper_t));
    if (!wrapper) {
        ssdk_printf(SSDK_CRIT, "vbe: fail to alloc wrapper\n");
        return NULL;
    }

    wrapper->sa = 0;
    wrapper->dev = ptr;

    ssdk_printf(SSDK_DEBUG,
            "vbe: wrapper %p dev %p common\n",
            wrapper, wrapper->dev);

    return wrapper;
#else
    return virtio_server_create_device(vs, name,
            priority, id, features, ops, nvqs, max_sizes, priv);
#endif
}

static inline paddr_t virtio_server_addr_xlate(vs_t *vs, paddr_t addr)
{
    int ap;

    ap = vs->domain == DOMAIN_AP1 || vs->domain == DOMAIN_AP2;
    return ap ? ap2p(addr) : addr;
}

static void virtio_device_handle_reg_msg(vs_t *vs, vd_t *vd, msg_t *msg)
{
    switch (msg->offset) {
    case VIRTIO_MMIO_QUEUE_NOTIFY:
        {
            u32 qsel = readl(vd->base + VIRTIO_MMIO_QUEUE_NOTIFY);
            if (qsel < vd->nvqs) {
                vd->action |= VD_ACTION_NOTIFY;
                osEventFlagsSet(vd->event, VD_EVENT_KICK);
            }
        }
        break;
    case VIRTIO_MMIO_STATUS:
        {
            vd->cur_status = readl(vd->base + VIRTIO_MMIO_STATUS);
            if (!vd->cur_status) {
                if (vd->ops->reset) {
                    vd->ops->reset(vd->priv);
                }

                for (u32 i = 0; i < vd->nvqs; i++) {
                    vd->vqs[i].ready = 0;
                }

                vd->irq_status = 0;
                vd->config_gen = 0;
                virtio_device_init_regs(vd);
            }
        }
        break;
    case VIRTIO_MMIO_QUEUE_READY:
        {
            u32 qsel = readl(vd->base + VIRTIO_MMIO_QUEUE_SEL);
            if (qsel < vd->nvqs) {
                vq_t *vq = &vd->vqs[qsel];
                vq->ready = readl(vd->base + VIRTIO_MMIO_QUEUE_READY);

                if (vq->ready) {
                    vq->num = readl(vd->base + VIRTIO_MMIO_QUEUE_NUM);

                    if (vq->num & (vq->num - 1)) {
                        vq->ready = 0;
                        ssdk_printf(SSDK_CRIT,
                                "vbe: vq[%u] num %u must be power of 2\n",
                                qsel, vq->num);
                        break;
                    }

                    u64 l, h;

                    l = readl(vd->base + VIRTIO_MMIO_QUEUE_DESC_LOW);
                    h = readl(vd->base + VIRTIO_MMIO_QUEUE_DESC_HIGH);
                    vq->desc = (void *)p2v(virtio_server_addr_xlate(vs, (h << 32) | l));

                    l = readl(vd->base + VIRTIO_MMIO_QUEUE_AVAIL_LOW);
                    h = readl(vd->base + VIRTIO_MMIO_QUEUE_AVAIL_HIGH);
                    vq->avail = (void *)p2v(virtio_server_addr_xlate(vs, (h << 32) | l));

                    l = readl(vd->base + VIRTIO_MMIO_QUEUE_USED_LOW);
                    h = readl(vd->base + VIRTIO_MMIO_QUEUE_USED_HIGH);
                    vq->used = (void *)p2v(virtio_server_addr_xlate(vs, (h << 32) | l));

                    ssdk_printf(SSDK_INFO,
                            "vbe: vq[%u] ready %u num %u desc %p avail %p used %p\n",
                            qsel, vq->ready, vq->num, vq->desc, vq->avail, vq->used);
                } else {
                    /* TODO notify device if needed */
                }
            }
        }
        break;
    case VIRTIO_MMIO_DEVICE_FEATURES_SEL:
        {
            u32 sel = readl(vd->base + VIRTIO_MMIO_DEVICE_FEATURES_SEL);
            writel(vd->features >> (32 * !!sel), vd->base + VIRTIO_MMIO_DEVICE_FEATURES);
        }
        break;
    case VIRTIO_MMIO_DRIVER_FEATURES:
        {
            u32 sel = readl(vd->base + VIRTIO_MMIO_DRIVER_FEATURES_SEL);
            vd->driver_features &= ~(0xffffffffull << (32 * !!sel));
            vd->driver_features |=
                ((u64)readl(vd->base + VIRTIO_MMIO_DRIVER_FEATURES)) << (32 * !!sel);
        }
        break;
    case VIRTIO_MMIO_QUEUE_SEL:
        {
            u32 qsel = readl(vd->base + VIRTIO_MMIO_QUEUE_SEL);
            if (qsel < vd->nvqs) {
                vq_t *vq = &vd->vqs[qsel];
                writel(vq->ready, vd->base + VIRTIO_MMIO_QUEUE_READY);
                writel(vq->max_size, vd->base + VIRTIO_MMIO_QUEUE_NUM_MAX);
            } else {
                writel(0, vd->base + VIRTIO_MMIO_QUEUE_READY);
                writel(0, vd->base + VIRTIO_MMIO_QUEUE_NUM_MAX);
            }
        }
        break;
    default:
        {
            ssdk_printf(SSDK_CRIT, "vbe: unexpected offset %u\n", msg->offset);
        }
        break;
    }
}

static inline void virtio_server_handle_msg(vs_t *vs, msg_t *msg)
{
    vd_t *vd = NULL, *tmp;
    irq_state_t state;

    state = spin_lock_irqsave(&vs->lock);
    list_for_every_entry(&vs->dev_list, tmp, vd_t, list) {
        if (msg->vid == tmp->vid) {
            vd = tmp;
            break;
        }
    }
    spin_unlock_irqrestore(&vs->lock, state);

    if (!vd) {
        ssdk_printf(SSDK_CRIT,
                "vbe: dev %u not found for msg %x\n",
                msg->vid, *(u32 *)msg);
        return;
    }

    if (msg->mt == VM_MT_CFG) {
        if (vd->ops->set_cfg) {
            vd->ops->set_cfg(vd->priv, msg->offset, msg->length);
        }
    } else if (msg->mt == VM_MT_REG) {
        virtio_device_handle_reg_msg(vs, vd, msg);
    } else {
        ssdk_printf(SSDK_CRIT,
                "vbe: invalid msg type %d for msg %x\n",
                msg->mt, *(u32 *)msg);
        return;
    }
}

static int virtio_server_mbox_callback(void *arg, u8 *buf, u32 len)
{
    vs_t *vs = arg;
    if (!vs) {
        ssdk_printf(SSDK_CRIT, "vbe: invalid NULL virtio server\n");
        return -1;
    }

    if (len != sizeof(msg_t)) {
        ssdk_printf(SSDK_CRIT,
                "vbe: unexpected %u bytes from %s\n",
                len, get_domain_name(vs->domain));
        return -1;
    }

    msg_t *msg = (msg_t *)buf;

    ssdk_printf(SSDK_DEBUG,
            "vbe: recv %08x from %s\n",
            *(u32 *)msg, get_domain_name(vs->domain));

    /* boot msg doesn't care vid */
    if (msg->mt == VM_MT_BOOT) {
        msg->ac = 1;
        /* timeout 1s to ensure peer receives it */
        virtio_server_send_msg(vs, msg, 1000);
        return 0;
    }

    virtio_server_handle_msg(vs, msg);

    /*
     * send ACK if required
     * normally cfg or reg msgs from Linux fe requires ACK
     */
    if (msg->ar) {
        msg->ac = 1;
        virtio_server_send_msg(vs, msg, 1000);
    }

    return 0;
}

static void virtio_local_reg_changed(u32 vid, u8 offset, void *priv)
{
    vs_t *vs = priv;

    /* fake a msg to process */
    msg_t msg;
    msg.mt = VM_MT_REG;
    msg.ac = 0;
    msg.ar = 0;
    msg.vid = vid & VM_VID_MASK;
    msg.offset = offset;
    msg.length = 0;/* silly compiler */

    virtio_server_handle_msg(vs, &msg);
}

static void virtio_local_cfg_changed(u32 vid, u8 offset, u8 length, void *priv)
{
    vs_t *vs = priv;

    /* fake a msg to process */
    msg_t msg;
    msg.mt = VM_MT_CFG;
    msg.ac = 0;
    msg.ar = 0;
    msg.vid = vid & VM_VID_MASK;
    msg.offset = offset;
    msg.length = length;

    virtio_server_handle_msg(vs, &msg);
}

static int virtio_server_init_local(vs_t *vs)
{
#if __SA_COMPATIBLE
    wrapper_t *wrapper = vs->vps->vdev;
    vd_t *vd = wrapper->dev;
#else
    vd_t *vd = vs->vps->vdev;
#endif

    vs->vbo.on_reg_changed = virtio_local_reg_changed;
    vs->vbo.on_cfg_changed = virtio_local_cfg_changed;
    vs->ready = 1;

    virtio_agent_register_backend(&vs->vbo, vd->reg, vs);

    return 0;
}

static void virtio_server_thread(void *arg)
{
    vs_t *vs = arg;
    vm_t *vms;
    irq_state_t state;
    u32 flags;

    vms = malloc(sizeof(vm_t) * VS_MSG_COUNT);
    if (!vms) {
        ssdk_printf(SSDK_CRIT, "vfe: fail to alloc vms\n");
        return;
    }

    for (;;) {
        flags = osEventFlagsWait(vs->event,
                VS_EVENT_KICK, osFlagsWaitAny, osWaitForever);

        /* if any error, just wait again */
        if (flags != VS_EVENT_KICK)
            continue;

        for (;;) {
            u16 s = 0, n = 0;

            state = spin_lock_irqsave(&vs->msg_lock);
            if (vs->__ri != vs->__wi) {
                s = vs->__ri;
                n = (vs->__wi + VS_MSG_COUNT - vs->__ri) & VS_MSG_MASK;
                vs->__ri = vs->__wi;
            } else if (vs->__full) {
                s = vs->__ri;
                n = VS_MSG_COUNT;
                vs->__full = 0;
            }

            /* from s copy n vm to local */
            if (s + n > VS_MSG_COUNT) {
                memcpy(vms, &vs->__vms[s],
                        sizeof(vm_t) * (VS_MSG_COUNT - s));
                memcpy(&vms[VS_MSG_COUNT-s], vs->__vms,
                        sizeof(vm_t) * (s + n - VS_MSG_COUNT));
            } else {
                memcpy(vms, &vs->__vms[s], sizeof(vm_t) * n);
            }
            spin_unlock_irqrestore(&vs->msg_lock, state);

            if (!n)
                break;

            for (u16 i = 0; i < n; i++) {
                /* TODO check error */
                mbox_send(vs->chan, (u8 *)&vms[i].msg,
                        sizeof(msg_t), vms[i].timeout);
            }
        }
    }

    free(vms);
}

static int virtio_server_init_remote(vs_t *vs)
{
    struct sdrv_mbox_chan_req chan_req;
    osThreadAttr_t attr;

    spin_lock_init(&vs->msg_lock);
    vs->__full = 0;
    vs->__ri = vs->__wi = 0;

    vs->event = osEventFlagsNew(NULL);

    memset(&attr, 0, sizeof(attr));
    attr.name = "vfe";
    attr.stack_size = 4096,
    attr.priority = osPriorityHigh;
    vs->task = osThreadNew(virtio_server_thread, vs, &attr);
    if (!vs->task) {
        ssdk_printf(SSDK_CRIT, "vbe: fail to create server thread\n");
        return -1;
    }

    // TODO non-blocking IO
    chan_req.rproc = vs->rproc;
    chan_req.src_addr = vs->addr;
    chan_req.dest_addr = vs->addr;
    vs->chan = mbox_request_channel(g_mbox_dev, &chan_req);
    if (!vs->chan) {
        ssdk_printf(SSDK_CRIT,
                "vbe: fail to get mbox channel %d %x\n",
                vs->rproc, vs->addr);
        return -1;
    }
    mbox_set_callback(vs->chan, virtio_server_mbox_callback, vs);

    msg_t msg;
    memset(&msg, 0, sizeof(msg));
    msg.mt = VM_MT_BOOT;
    virtio_server_send_msg(vs, &msg, 1);

    vs->ready = 1;
    return 0;
}

static int virtio_backend_server_init(vbe_t *vbe, vs_t *vs)
{
    if (vbe->domain == vs->domain) {
        vs->type = SERVER_LOCAL;
        vs->cur_vid = 0;
    } else {
        vs->type = SERVER_MBOX_SHMEM;
        /* 0 and 1 is reserved */
        vs->cur_vid = 2;
    }

    spin_lock_init(&vs->lock);
    list_initialize(&vs->dev_list);

    /* create provider service */
    vps_t *vps = malloc(sizeof(vps_t));
    if (!vps) {
        ssdk_printf(SSDK_CRIT, "vbe: fail to alloc vps\n");
        return -1;
    }
    memset(vps, 0, sizeof(*vps));

    vps->tx_lock = osMutexNew(NULL);

    struct virtio_device_id id = {
        .device = VIRTIO_ID_PROVIDER,
        .vendor = -1,/* VIRTIO_DEV_ID_ANY */
    };
    u32 sizes[VQ_COUNT] = { RX_LEN, TX_LEN };

    void *ptr = virtio_server_create_device(vs, "vps",
            osPriorityNormal, id, 0/* TODO features */,
            &virtio_provider_ops, VQ_COUNT, sizes, vps);
    if (!ptr) {
        ssdk_printf(SSDK_CRIT, "vbe: fail to create device\n");
        return -1;
    }

#if __SA_COMPATIBLE
    wrapper_t *wrapper = malloc(sizeof(wrapper_t));
    if (!wrapper) {
        ssdk_printf(SSDK_CRIT, "vbe: fail to alloc wrapper\n");
        return -1;
    }

    wrapper->sa = 0;
    wrapper->dev = ptr;

    ssdk_printf(SSDK_DEBUG,
            "vbe: wrapper %p dev %p provider\n",
            wrapper, wrapper->dev);

    vps->vdev = wrapper;
#else
    vps->vdev = ptr;
#endif
    vs->vps = vps;

    if (vbe->domain == vs->domain) {
        virtio_server_init_local(vs);
    } else {
        virtio_server_init_remote(vs);
    }

    return 0;
}

void virtio_backend_init(void)
{
    vbe_t *vbe = malloc(sizeof(vbe_t));
    if (!vbe) {
        ssdk_printf(SSDK_CRIT,
                "vbe: fail to alloc %zu bytes\n",
                sizeof(vbe_t));
        return;
    }
    memset(vbe, 0, sizeof(*vbe));

    for (int i = 0; i < DOMAIN_MAX; i++) {
        vs_t *vs = &vbe->servers[i];

        vs->vbe = vbe;
        vs->domain = i;
        vs->ready = 0;
    }

    /* init from cfg */
    const cfg_t *cfg = get_backend_config();

    vbe->domain = cfg->my_domain;
    if (vbe->domain != DOMAIN_SAF
            && vbe->domain != DOMAIN_SEC
            && vbe->domain != DOMAIN_MP) {
        return;
    }

    ssdk_printf(SSDK_INFO,
            "vbe: we're on %s\n",
            get_domain_name(vbe->domain));

    for (int i = 0; i < cfg->peer_num; i++) {
        const peer_t *peer = &cfg->peers[i];
        vs_t *vs = &vbe->servers[peer->domain];

        /* mem_base 0 for remote means disabled */
        if (vs->domain != vbe->domain && !peer->mem_base)
            continue;

        if (vs->domain != vbe->domain) {
            vs->rproc = get_rproc_from_domain(vs->domain);
            vs->addr = peer->mbox_addr;
            vs->reg = peer->mem_base;
        }

        virtio_backend_server_init(vbe, vs);
    }

    __vbe = vbe;
    ssdk_printf(SSDK_INFO, "vbe: ready\n");
}

static void virtio_backend_dump(vbe_t *vbe)
{
    for (int i = 0; i < DOMAIN_MAX; i++) {
        vs_t *vs = &vbe->servers[i];

        if (!vs->ready)
            continue;

        if (vs->type == SERVER_MBOX_SHMEM) {
            printf("server %s msgs %u %u %d mbox %u %u mem %x devs:\n",
                    get_domain_name(vs->domain),
                    vs->__ri, vs->__wi, vs->__full,
                    vs->rproc, vs->addr, vs->reg);
        } else if (vs->type == SERVER_LOCAL) {
            printf("server %s devs:\n",
                    get_domain_name(vs->domain));
        }

        vd_t *vd, *tmp;
        list_for_every_entry_safe(&vs->dev_list, vd, tmp, vd_t, list) {
            printf(" - dev[%u] %s reg 0x%x id %u %d ft dev %llx drv %llx status %x priv %p\n",
                    vd->vid, vd->name, vd->reg,
                    vd->id.device, vd->id.vendor,
                    vd->features, vd->driver_features,
                    vd->cur_status, vd->priv);
            for (u32 i = 0; i < vd->nvqs; i++) {
                vq_t *vq = &vd->vqs[i];
                printf("     vq[%u] ready %u num %u(%u) desc %p avail %p used %p\n",
                        i, vq->ready, vq->num, vq->max_size,
                        vq->desc, vq->avail, vq->used);
            }
        }
    }
}

static int virtio_debug_usage(void)
{
    printf("Usage: vbe CMD OPTIONS\n");
    printf("\n");
    printf("Available CMDs:\n");
#if ENABLE_VIRTIO_DEBUG
    printf(" - dummy [rproc] [device id]\n");
    printf("        create a dummy backend, for device discovery test\n");
#endif
    printf(" - dump\n");
    printf("        dump servers and devices\n");
    printf(" - text [rproc] [string]\n");
    printf("        send string to a client on rproc\n");

    return 0;
}

#if ENABLE_VIRTIO_DEBUG
static void virtio_dummy_reset(void *dev)
{
    printf("vbe: dummy device %p reset\n", dev);
}

static struct virtio_device_ops virtio_dummy_ops = {
    .reset = virtio_dummy_reset,
};
#endif

static int virtio_backend_debug_entry(int argc, char *argv[])
{
    /* someone's already removed argv[0] for us */
    argc += 1;
    argv -= 1;

    if (argc < 2)
        return virtio_debug_usage();

    vbe_t *vbe = __vbe;
    if (!vbe) {
        printf("backend not ready\n");
        return 0;
    }

    if (!strncmp(argv[1], "dump", 4)) {
        virtio_backend_dump(vbe);
#if ENABLE_VIRTIO_DEBUG
    } else if (!strncmp(argv[1], "dummy", 5)) {
        if (argc < 4) {
            printf("too few args\n");
            return virtio_debug_usage();
        }

        sdrv_mbox_rproc_t rproc = atoi(argv[2]);
        struct virtio_device_id id = {
            .device = atoi(argv[3]),
            .vendor = -1,
        };
        u32 sizes[2] = { 128, 128 };
        void *dev = virtio_device_create_common(id, 0, &virtio_dummy_ops,
                2, sizes, rproc, "dummy", osPriorityLow, NULL);
        printf("create dummy device %p\n", dev);
#endif
    } else if (!strncmp(argv[1], "text", 4)) {
        if (argc < 4) {
            printf("too few args\n");
            return virtio_debug_usage();
        }

        sdrv_mbox_rproc_t rproc = atoi(argv[2]);
        int domain = get_domain_from_rproc(rproc);
        if (domain < 0 || domain >= DOMAIN_MAX) {
            printf("invalid rproc %s\n", argv[2]);
            return 0;
        }

        vs_t *vs = &vbe->servers[domain];
        if (!vs->ready || !vs->vps) {
            printf("server for rproc %u not ready\n", rproc);
            return 0;
        }

        virtio_provider_send_text(vs->vps, argv[3]);
    } else {
        printf("unknown CMD '%s'\n", argv[1]);
        return virtio_debug_usage();
    }

    return 0;
}

CLI_CMD("vbe", "virtio backend debug commands", virtio_backend_debug_entry);
