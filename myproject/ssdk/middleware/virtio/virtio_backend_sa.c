#include <arch/cache.h>
#include <debug.h>
#include <driver/kunlun/mbox/sdrv_mbox_drv.h>
#include <driver/mbox.h>
#include <driver/memmap.h>
#include <lib/list.h>
#include <mbox_cfg.h>
#include <reg.h>
#include <spinlock.h>
#include <stdlib.h>

#include <virtio_mmio.h>

typedef uint8_t     u8;
typedef uint16_t    u16;
typedef uint32_t    u32;
typedef uint64_t    u64;

typedef int8_t      s8;
typedef int16_t     s16;
typedef int32_t     s32;
typedef int64_t     s64;

#include "virtio_device_sa.h"

#define p2v(pa)     ((vaddr_t)(pa))
#define v2p(va)     ((paddr_t)(va))

#define CFG_SHIFT   11
#define ACK_SHIFT   12
#define MSG_COUNT   8

typedef struct {
    u16 flag;
    u8 offset;
    u8 len;
    struct list_node list;
} vm_t;

typedef struct {
    u32 max_size;
    u32 ready;

    u32 num;
    void *desc;
    void *avail;
    void *used;
} vq_t;

typedef struct {
    struct virtio_device_id id;
    u64 features;
    u64 driver_features;
    struct virtio_device_ops *ops;
    u32 reg_base;
    u32 cfg_size;
    sdrv_mbox_rproc_t mbox_rproc;
    u32 mbox_addr;
    void *priv;

    u32 irq_status;
    u32 config_gen;

    vaddr_t base;
    mbox_chan_t *chan;
#define VD_EVENT_KICK       0x1
    osEventFlagsId_t event;
    osThreadId_t task;

    spin_lock_t vmlock;
    osMutexId_t msg_lock;
    vm_t vms[MSG_COUNT];
    struct list_node vmlist;
    int pending_notify;

    struct list_node list;

    u32 nvqs;
    vq_t vqs[];
} vd_t;

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

    arch_clean_cache_range(vd->base, 0x100);
}

static void virtio_device_vm_queue_and_send(vd_t *vd, vm_t *vm)
{
    irq_state_t state;
    int found = false;

    state = spin_lock_irqsave(&vd->vmlock);
    for (int i = 0; i < MSG_COUNT; i++) {
        if (!vd->vms[i].flag) {
            vd->vms[i] = *vm;
            list_add_tail(&vd->vmlist, &vd->vms[i].list);

            found = true;
            break;
        }
    }
    spin_unlock_irqrestore(&vd->vmlock, state);

    if (found) {
        osEventFlagsSet(vd->event, VD_EVENT_KICK);
    } else {
        ssdk_printf(SSDK_CRIT,
                "vm underflow, flag %x offset %x len %u\n",
                vm->flag, vm->offset, vm->len);
    }
}

static int virtio_device_mbox_callback(void *arg, u8 *buf, u32 len)
{
    vd_t *vd;
    vm_t *vm;

    vd = (vd_t *)arg;
    if (!vd) {
        ssdk_printf(SSDK_CRIT, "invalid NULL virtio device\n");
        return -1;
    }

    if (len != 4) {
        ssdk_printf(SSDK_CRIT, "unexpected %u bytes\n", len);
        return -1;
    }

    vm = (vm_t *)buf;

    if (vm->flag & (1u << CFG_SHIFT)) {
        /* TODO consider adding monitors to avoid too many operations */
        if (vd->ops->set_cfg)
            vd->ops->set_cfg(vd->priv, vm->offset, vm->len);

        return 0;
    }

    switch (vm->offset) {
    case VIRTIO_MMIO_QUEUE_NOTIFY:
        {
            u32 qsel;

            arch_clean_invalidate_cache_range(vd->base + VIRTIO_MMIO_QUEUE_NOTIFY, sizeof(u32));
            qsel = readl(vd->base + VIRTIO_MMIO_QUEUE_NOTIFY);
            if (qsel < vd->nvqs) {
                vd->pending_notify = 1;
                osEventFlagsSet(vd->event, VD_EVENT_KICK);
            }
        }
        break;
    case VIRTIO_MMIO_STATUS:
        {
            arch_clean_invalidate_cache_range(vd->base + VIRTIO_MMIO_STATUS, sizeof(u32));
            if (!readl(vd->base + VIRTIO_MMIO_STATUS)) {
                vd->ops->reset(vd->priv);

                for (u32 i = 0; i < vd->nvqs; i++)
                    vd->vqs[i].ready = 0;

                /* notify frontend of reset done */
                virtio_device_vm_queue_and_send(vd, vm);
            }
        }
        break;
    case VIRTIO_MMIO_QUEUE_READY:
        {
            arch_clean_invalidate_cache_range(vd->base + VIRTIO_MMIO_QUEUE_SEL, sizeof(u32));
            u32 qsel = readl(vd->base + VIRTIO_MMIO_QUEUE_SEL);

            if (qsel < vd->nvqs) {
                vq_t *vq = &vd->vqs[qsel];
                arch_clean_invalidate_cache_range(vd->base + VIRTIO_MMIO_QUEUE_READY, sizeof(u32));
                vq->ready = readl(vd->base + VIRTIO_MMIO_QUEUE_READY);

                if (vq->ready) {
                    /* VIRTIO_MMIO_QUEUE_NUM and VIRTIO_MMIO_QUEUE_SEL on same cache line */
                    vq->num = readl(vd->base + VIRTIO_MMIO_QUEUE_NUM);

                    if (vq->num & (vq->num - 1)) {
                        vq->ready = 0;
                        ssdk_printf(SSDK_CRIT,
                                "vq[%u] num %u must be power of 2\n", qsel, vq->num);
                        break;
                    }

                    u64 l, h;

                    arch_clean_invalidate_cache_range(vd->base + VIRTIO_MMIO_QUEUE_DESC_LOW,
                            VIRTIO_MMIO_QUEUE_USED_HIGH-VIRTIO_MMIO_QUEUE_DESC_LOW+sizeof(u32));

                    l = readl(vd->base + VIRTIO_MMIO_QUEUE_DESC_LOW);
                    h = readl(vd->base + VIRTIO_MMIO_QUEUE_DESC_HIGH);
                    vq->desc = (void *)ap2p(p2v((h << 32) | l));

                    l = readl(vd->base + VIRTIO_MMIO_QUEUE_AVAIL_LOW);
                    h = readl(vd->base + VIRTIO_MMIO_QUEUE_AVAIL_HIGH);
                    vq->avail = (void *)ap2p(p2v((h << 32) | l));

                    l = readl(vd->base + VIRTIO_MMIO_QUEUE_USED_LOW);
                    h = readl(vd->base + VIRTIO_MMIO_QUEUE_USED_HIGH);
                    vq->used = (void *)ap2p(p2v((h << 32) | l));

                    ssdk_printf(SSDK_INFO,
                            "vq[%u] ready %u num %u desc %p avail %p used %p\n",
                            qsel, vq->ready, vq->num, vq->desc, vq->avail, vq->used);
                } else {
                    // TODO notify device if needed
                }
            }

            virtio_device_vm_queue_and_send(vd, vm);
        }
        break;
    case VIRTIO_MMIO_DEVICE_FEATURES_SEL:
        {
            arch_clean_invalidate_cache_range(
                    vd->base + VIRTIO_MMIO_DEVICE_FEATURES_SEL, sizeof(u32));
            u32 sel = readl(vd->base + VIRTIO_MMIO_DEVICE_FEATURES_SEL);

            writel(vd->features >> (32 * !!sel), vd->base + VIRTIO_MMIO_DEVICE_FEATURES);
            arch_clean_cache_range(vd->base + VIRTIO_MMIO_DEVICE_FEATURES, sizeof(u32));

            virtio_device_vm_queue_and_send(vd, vm);
        }
        break;
    case VIRTIO_MMIO_DRIVER_FEATURES:
        {
            arch_clean_invalidate_cache_range(
                    vd->base + VIRTIO_MMIO_DRIVER_FEATURES, sizeof(u32));
            u32 sel = readl(vd->base + VIRTIO_MMIO_DRIVER_FEATURES_SEL);
            vd->driver_features &= ~(0xffffffffull << (32 * !!sel));
            vd->driver_features |=
                ((u64)readl(vd->base + VIRTIO_MMIO_DRIVER_FEATURES)) << (32 * !!sel);

            virtio_device_vm_queue_and_send(vd, vm);
        }
        break;
    case VIRTIO_MMIO_QUEUE_SEL:
        {
            arch_clean_invalidate_cache_range(vd->base + VIRTIO_MMIO_QUEUE_SEL, sizeof(u32));
            u32 qsel = readl(vd->base + VIRTIO_MMIO_QUEUE_SEL);

            if (qsel < vd->nvqs) {
                vq_t *vq = &vd->vqs[qsel];
                writel(vq->ready, vd->base + VIRTIO_MMIO_QUEUE_READY);
                writel(vq->max_size, vd->base + VIRTIO_MMIO_QUEUE_NUM_MAX);
            } else {
                writel(0, vd->base + VIRTIO_MMIO_QUEUE_READY);
                writel(0, vd->base + VIRTIO_MMIO_QUEUE_NUM_MAX);
            }

            arch_clean_cache_range(vd->base + VIRTIO_MMIO_QUEUE_NUM_MAX,
                    VIRTIO_MMIO_QUEUE_READY-VIRTIO_MMIO_QUEUE_NUM_MAX+sizeof(u32));

            virtio_device_vm_queue_and_send(vd, vm);
        }
        break;
    default:
        {
            ssdk_printf(SSDK_CRIT, "unexpected offset %u\n", vm->offset);
        }
        break;
    }

    return 0;
}

static void virtio_device_thread(void *arg)
{
    int ret;
    vm_t *vm;
    vd_t *vd = arg;
    u32 flags;
    irq_state_t state;

    for (;;) {
        flags = osEventFlagsWait(vd->event,
                VD_EVENT_KICK, osFlagsWaitAny, osWaitForever);
        if (flags != VD_EVENT_KICK)
            continue;

        if (vd->pending_notify && vd->ops && vd->ops->notify) {
            vd->pending_notify = 0;
            vd->ops->notify(vd->priv);
        }

        if (!list_is_empty(&vd->vmlist)) {
            state = spin_lock_irqsave(&vd->vmlock);
            while ((vm = list_remove_head_type(&vd->vmlist, vm_t, list))) {
                spin_unlock_irqrestore(&vd->vmlock, state);

                vm->flag |= (1u << ACK_SHIFT);

                osMutexAcquire(vd->msg_lock, osWaitForever);
                ret = mbox_send(vd->chan, (u8 *)vm, 4, 1000);
                osMutexRelease(vd->msg_lock);
                if (ret < 0)
                    ssdk_printf(SSDK_CRIT,
                            "fail to reply %x %u\n", vm->flag, vm->offset);

                state = spin_lock_irqsave(&vd->vmlock);
                vm->flag = 0;
            }
            spin_unlock_irqrestore(&vd->vmlock, state);
        }
    }
}

static spin_lock_t g_vd_lock = SPIN_LOCK_INITIAL_VALUE;
static struct list_node g_vd_list = LIST_INITIAL_VALUE(g_vd_list);

void *virtio_device_create_mmio_sa(struct virtio_device_id id, u64 features,
        struct virtio_device_ops *ops, u32 nvqs, u32 max_sizes[],
        u32 reg_base, u32 cfg_size, sdrv_mbox_rproc_t mbox_rproc, int mbox_addr, void *dev)
{
    struct sdrv_mbox_chan_req chan_req;
    osThreadAttr_t attr;
    vd_t *vd;

    if (ops && ops->set_cfg && !cfg_size) {
        ssdk_printf(SSDK_CRIT, "invalid config area size %u\n", cfg_size);
        return NULL;
    }

    if (!nvqs) {
        ssdk_printf(SSDK_CRIT, "invalid virtqueue num %u\n", nvqs);
        return NULL;
    }

    vd = malloc(sizeof(vd_t) + nvqs * sizeof(vq_t));
    if (!vd) {
        ssdk_printf(SSDK_CRIT, "fail to alloc vd_t\n");
        return NULL;
    }

    vd->id = id;
    vd->features = features;
    vd->ops = ops;
    vd->nvqs = nvqs;
    vd->priv = dev;
    vd->reg_base = reg_base;
    vd->cfg_size = cfg_size;
    vd->mbox_addr = mbox_addr;
    vd->mbox_rproc = mbox_rproc;

    // TODO non-blocking
    chan_req.rproc = vd->mbox_rproc;
    chan_req.src_addr = vd->mbox_addr;
    chan_req.dest_addr = vd->mbox_addr;
    vd->chan = mbox_request_channel(g_mbox_dev, &chan_req);
    if (!vd->chan) {
        ssdk_printf(SSDK_CRIT, "fail to get mbox channel\n");
        goto out_free;
    }
    mbox_set_callback(vd->chan, virtio_device_mbox_callback, vd);

    spin_lock_init(&vd->vmlock);
    vd->msg_lock = osMutexNew(NULL);
    list_initialize(&vd->vmlist);
    for (int i = 0; i < MSG_COUNT; i++) {
        vd->vms[i].flag = 0;
        list_initialize(&vd->vms[i].list);
    }
    vd->pending_notify = 0;

    vd->event = osEventFlagsNew(NULL);

    attr.priority = osPriorityHigh;
    vd->task = osThreadNew(virtio_device_thread, vd, &attr);
    if (!vd->task) {
        ssdk_printf(SSDK_CRIT, "fail to create thread\n");
        goto out_chan;
    }

    vd->irq_status = 0;
    vd->config_gen = 0;

    for (u32 i = 0; i < vd->nvqs; i++) {
        vd->vqs[i].max_size = max_sizes[i];
        vd->vqs[i].ready = 0;
    }

    vd->base = p2v(vd->reg_base);

    ssdk_printf(SSDK_INFO, "virtio-mmio@0x%08x mb %u %u\n",
            vd->reg_base, vd->mbox_rproc, vd->mbox_addr);

    virtio_device_init_regs(vd);

    list_initialize(&vd->list);

    irq_state_t state;
    state = spin_lock_irqsave(&g_vd_lock);
    list_add_tail(&g_vd_list, &vd->list);
    spin_unlock_irqrestore(&g_vd_lock, state);

    return vd;

out_chan:
    mbox_release_channel(vd->chan);

out_free:
    free(vd);

    return NULL;
}

u32 virtio_device_num_get_sa(void *vdev, u32 vq)
{
    vd_t *vd = vdev;

    return vd->vqs[vq].ready ? vd->vqs[vq].num : 0;
}

void *virtio_device_desc_get_sa(void *vdev, u32 vq)
{
    vd_t *vd = vdev;

    return vd->vqs[vq].ready ? vd->vqs[vq].desc : NULL;
}

void *virtio_device_avail_get_sa(void *vdev, u32 vq)
{
    vd_t *vd = vdev;

    return vd->vqs[vq].ready ? vd->vqs[vq].avail : NULL;
}

void *virtio_device_used_get_sa(void *vdev, u32 vq)
{
    vd_t *vd = vdev;

    return vd->vqs[vq].ready ? vd->vqs[vq].used : NULL;
}

void *virtio_device_config_area_get_sa(void *vdev)
{
    vd_t *vd = vdev;

    return (void *)vd->base + VIRTIO_MMIO_CONFIG;
}

void virtio_device_notify_sa(void *vdev)
{
    vd_t *vd = vdev;

    if (!vd)
        return;

    vd->irq_status |= VIRTIO_MMIO_INT_VRING;
    writel(vd->irq_status, vd->base + VIRTIO_MMIO_INTERRUPT_STATUS);
    arch_clean_cache_range(vd->base + VIRTIO_MMIO_INTERRUPT_STATUS, sizeof(u32));

    vm_t vm = {
        .flag = 0,
        .offset = 0,
        .len = 0,
    };

    /* timeout 50 ms */
    int retry = 0, ret;
    while (retry < 50) {
        osMutexAcquire(vd->msg_lock, osWaitForever);
        ret = mbox_send(vd->chan, (u8 *)&vm, sizeof(vm), 1);
        osMutexRelease(vd->msg_lock);
        if (!ret)
            break;
    }

    if (ret)
        ssdk_printf(SSDK_CRIT, "fail to notify %u %u retry %d ret %d\n",
                vd->mbox_rproc, vd->mbox_addr, retry, ret);

    // TODO since we omit ACK in mailbox now, how to simulate int clear action
#if 0
    /* no need to check ACK since mailbox ensures message acknowledgement */
    writel(0, vd->base + VIRTIO_MMIO_INTERRUPT_STATUS);
    arch_clean_cache_range(vd->base + VIRTIO_MMIO_INTERRUPT_STATUS, sizeof(u32));
#endif
}

#if defined (WITH_LIB_CONSOLE)
static int virtio_device_dump(int argc, const cmd_args *argv)
{
    vd_t *vd, *tmp;

    list_for_every_entry_safe(&g_vd_list, vd, tmp, vd_t, list) {
        printf("Device: virtio-mmio@%x mbox %u 0x%x id %u %u features %llx %llx cfg %u priv %p\n",
                vd->reg_base, vd->mbox_rproc, vd->mbox_addr,
                vd->id.device, vd->id.vendor, vd->features, vd->driver_features,
                vd->cfg_size, vd->priv);
        printf("Virtqueues:\n");
        for (u32 i = 0; i < vd->nvqs; i++) {
            vq_t *vq = &vd->vqs[i];
            printf("    vq[%u] ready %u num %u(%u) desc %p avail %p used %p\n",
                    i, vq->ready, vq->num, vq->max_size,
                    vq->desc, vq->avail, vq->used);
        }
    }

    return 0;
}

STATIC_COMMAND_START
STATIC_COMMAND("vbe_dump", "dump current devices", virtio_device_dump)
STATIC_COMMAND_END(virtio_backend_sa);
#endif
