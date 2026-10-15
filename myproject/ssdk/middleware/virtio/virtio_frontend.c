#include <assert.h>
#include <CLI_type.h>
#include <debug.h>
#include <driver/mbox.h>
#include <driver/kunlun/mbox/sdrv_mbox_drv.h>
#include <lib/list.h>
#include <malloc.h>
#include <mbox_cfg.h>
#include <reg.h>
#include <stdlib.h>
#include <string.h>

#include <dev/virtio.h>
#include <dev/virtio/virtio_ring.h>
#include <virtio_ids.h>
#include <virtio_mmio.h>
#include "virtio_common.h"
#include "virtio_priv.h"


#define p2v(pa)     ((vaddr_t)(pa))
#define v2p(va)     ((paddr_t)(va))

/* vq length of virtio device provider */
#define VQ_TX 0
#define VQ_RX 1
#define TX_LEN 16
#define RX_LEN 16

/* TX can contain all 33 devices' info */
#define TX_BUFSIZE 512
#define RX_BUFSIZE 512
#define VIRTIO_PROVIDER_MEM_SIZE (TX_BUFSIZE*TX_LEN+RX_BUFSIZE*RX_LEN)

struct virtio_device_impl {
    struct virtio_device dev;
    u32 device_id;
    u32 vid;
    void *vc;
    paddr_t vq_base;
    paddr_t vq_cur;

    struct list_node list;
};

#define to_impl(dev) containerof(dev, struct virtio_device_impl, dev)

/* virtio device provider */
typedef struct {
    struct virtio_device *dev;
} vdp_t;

enum {
    VCA_NONE,
    VCA_BOOT,
    VCA_MSG,
    VCA_DEV,
};

/* virtio client action */
typedef struct {
    struct list_node list;
    u32 refcount;

    int action;
    int notify;
#define VCA_EVENT_DONE      0x1
    osEventFlagsId_t done;

    union {
        struct {
            msg_t msg;
            u32 timeout;
        } msg_param;
        vdi_t dev_param;
    };
} vca_t;

/* virtio finder */
typedef struct {
#define VF_EVENT_FOUND      0x1
    osEventFlagsId_t event;
    u32 device_id;
    struct virtio_device_impl *impl;
    struct list_node list;
} vf_t;

#define VC_FINDER_NUM       36
#define VC_ACTION_NUM       64

#define VC_MBOX_SHMEM       0
#define VC_NORMAL_MMIO      1
#define VC_LOCAL            2

/* virtio client */
typedef struct {
    void *vfe;
    int domain;
    int type;
    paddr_t base;

    spin_lock_t dev_lock;
    struct list_node dev_list;

    vdp_t *vdp;
    vf_t __finders[VC_FINDER_NUM];
    struct list_node finder_list;

    spin_lock_t lock;
    int boot_msg;
    osThreadId_t task;
#define VC_EVENT_KICK       0x1
    osEventFlagsId_t event;
    struct list_node action_list;
    struct list_node idle_list;
    vca_t __actions[VC_ACTION_NUM];

    u32 (*read)(struct virtio_device_impl *, u8);
    void (*write)(struct virtio_device_impl *, u8, u32);

    /* LOCAL BEGIN */
    vfo_t vfo;
    /* LOCAL END */

    /* REMOTE BEGIN */
    sdrv_mbox_rproc_t rproc;
    u32 addr;
    mbox_chan_t *chan;
    /* REMOTE END */
} vc_t;

/* virtio frontend */
typedef struct {
    int domain;
    vc_t clients[DOMAIN_MAX];
} vfe_t;

static inline u32 virtio_mmio_read(struct virtio_device_impl *impl, u8 offset)
{
    vc_t *vc = impl->vc;
    return vc->read(impl, offset);
}

static inline void virtio_mmio_write(struct virtio_device_impl *impl, u8 offset, u32 value)
{
    vc_t *vc = impl->vc;
    vc->write(impl, offset, value);
}

static vfe_t *__vfe;

/*
 * upref an action
 */
static void virtio_client_action_upref(vc_t *vc, vca_t *vca)
{
    irq_state_t state;

    state = spin_lock_irqsave(&vc->lock);
    vca->refcount++;
    spin_unlock_irqrestore(&vc->lock, state);
}

/*
 * downref an action
 */
static void virtio_client_action_downref(vc_t *vc, vca_t *vca)
{
    irq_state_t state;

    state = spin_lock_irqsave(&vc->lock);
    vca->refcount--;
    if (!vca->refcount) {
        list_add_tail(&vc->idle_list, &vca->list);
    }
    spin_unlock_irqrestore(&vc->lock, state);
}

/*
 * get an idle action and upref it
 */
static vca_t *virtio_client_action_get_and_upref(vc_t *vc)
{
    vca_t *vca = NULL;
    irq_state_t state;

    state = spin_lock_irqsave(&vc->lock);
    if (list_length(&vc->idle_list)) {
        vca = list_remove_head_type(&vc->idle_list, vca_t, list);
        vca->refcount++;
    }
    spin_unlock_irqrestore(&vc->lock, state);

    return vca;
}

static void virtio_client_action_queue(vc_t *vc, vca_t *vca)
{
    irq_state_t state;

    state = spin_lock_irqsave(&vc->lock);
    list_add_tail(&vc->action_list, &vca->list);
    spin_unlock_irqrestore(&vc->lock, state);
}

static vca_t *virtio_client_action_dequeue(vc_t *vc)
{
    vca_t *vca = NULL;
    irq_state_t state;

    state = spin_lock_irqsave(&vc->lock);
    if (list_length(&vc->action_list)) {
        vca = list_remove_head_type(&vc->action_list, vca_t, list);
    }
    spin_unlock_irqrestore(&vc->lock, state);

    return vca;
}

static int virtio_mmio_irq(void *arg)
{
    struct virtio_device *dev = (struct virtio_device *)arg;
    ssdk_printf(SSDK_DEBUG, "dev %p, index %u\n", dev, dev->index);

    struct virtio_device_impl *impl = to_impl(dev);
    uint32_t irq_status = virtio_mmio_read(impl, VIRTIO_MMIO_INTERRUPT_STATUS);
    ssdk_printf(SSDK_DEBUG, "status 0x%x\n", irq_status);

    int ret = 0;
    if (irq_status & 0x1) { /* used ring update */
        /* TODO doesn't care INTERRUPT_ACK for now */
#if 0
        virtio_mmio_write(impl, VIRTIO_MMIO_INTERRUPT_ACK, 0x1);
#endif

        /* cycle through all the active rings */
        for (uint r = 0; r < MAX_VIRTIO_RINGS; r++) {
            if ((dev->active_rings_bitmap & (1<<r)) == 0)
                continue;

            struct vring *ring = &dev->ring[r];
            ssdk_printf(SSDK_DEBUG, "ring %u: used flags 0x%hhx idx 0x%hhx last_used %u\n",
                    r, ring->used->flags, ring->used->idx, ring->last_used);

            uint cur_idx = ring->used->idx;
            for (uint i = ring->last_used; i != (cur_idx & ring->num_mask); i = (i + 1) & ring->num_mask) {
                ssdk_printf(SSDK_DEBUG, "looking at idx %u\n", i);

                // process chain
                struct vring_used_elem *used_elem = &ring->used->ring[i];
                ssdk_printf(SSDK_DEBUG, "id %u, len %u\n", used_elem->id, used_elem->len);

                assert(dev->irq_driver_callback);
                ret |= dev->irq_driver_callback(dev, r, used_elem);

                ring->last_used = (ring->last_used + 1) & ring->num_mask;
            }
        }
    }

    if (irq_status & 0x2) { /* config change */
        /* TODO doesn't care INTERRUPT_ACK for now */
#if 0
        virtio_mmio_write(impl, VIRTIO_MMIO_INTERRUPT_ACK, 0x2);
#endif

        if (dev->config_change_callback) {
            ret |= dev->config_change_callback(dev);
        }
    }

    ssdk_printf(SSDK_DEBUG, "exiting irq\n");

    return ret;
}

void virtio_dump_desc(const struct vring_desc *desc)
{
    printf("vring descriptor %p\n", desc);
    printf("\taddr  0x%llx\n", desc->addr);
    printf("\tlen   0x%x\n", desc->len);
    printf("\tflags 0x%hhx\n", desc->flags);
    printf("\tnext  0x%hhx\n", desc->next);
}

static void virtio_client_send_reg_update(vc_t *vc, u8 vid, u8 offset)
{
    /* always send mbox msg in client thread */
    if (vc->task == osThreadGetId()) {
        msg_t msg;
        memset(&msg, 0, sizeof(msg));
        msg.mt = VM_MT_REG;
        msg.vid = vid;
        msg.offset = offset;

        int err = mbox_send(vc->chan, (u8 *)&msg, sizeof(msg), 1000);
        if (err < 0) {
            /* we could just complain here if mailbox fails */
            ssdk_printf(SSDK_CRIT,
                    "vfe: %s dev %u fail to send msg %x err %d\n",
                    get_domain_name(vc->domain), vid, *(u32 *)&msg, err);
        }
    } else {
        vca_t *vca = virtio_client_action_get_and_upref(vc);
        vca->action = VCA_MSG;
        /* clear signal state */
        vca->notify = 1;
        osEventFlagsClear(vca->done, VCA_EVENT_DONE);

        msg_t *msg = &vca->msg_param.msg;
        memset(msg, 0, sizeof(*msg));
        msg->mt = VM_MT_REG;
        msg->vid = vid;
        msg->offset = offset;
        vca->msg_param.timeout = 1000;

        /* upref action so we can safely wait for it */
        virtio_client_action_upref(vc, vca);

        virtio_client_action_queue(vc, vca);
        osEventFlagsSet(vc->event, VC_EVENT_KICK);

        u32 flags = osEventFlagsWait(vca->done,
                VCA_EVENT_DONE, osFlagsWaitAny, osWaitForever);
        if (flags != VCA_EVENT_DONE) {
            ssdk_printf(SSDK_CRIT,
                    "vfe: %s dev %u fail to wait msg %x to be sent, flags %x\n",
                    get_domain_name(vc->domain), vid, *(u32 *)msg, flags);
        }

        /* release ref */
        virtio_client_action_downref(vc, vca);
    }
}

static void virtio_client_send_cfg_update(vc_t *vc, u8 vid, u8 offset, u8 length)
{
    /* always send mbox msg in client thread */
    if (vc->task == osThreadGetId()) {
        msg_t msg;
        memset(&msg, 0, sizeof(msg));
        msg.mt = VM_MT_CFG;
        msg.vid = vid;
        msg.offset = offset;
        msg.length = length;

        int err = mbox_send(vc->chan, (u8 *)&msg, sizeof(msg), 1000);
        if (err < 0) {
            /* we could just complain here if mailbox fails */
            ssdk_printf(SSDK_CRIT,
                    "vfe: %s dev %u fail to send msg %x err %d\n",
                    get_domain_name(vc->domain), vid, *(u32 *)&msg, err);
        }
    } else {
        vca_t *vca = virtio_client_action_get_and_upref(vc);
        vca->action = VCA_MSG;
        /* clear signal state */
        vca->notify = 1;
        osEventFlagsClear(vca->done, VCA_EVENT_DONE);

        msg_t *msg = &vca->msg_param.msg;
        memset(msg, 0, sizeof(*msg));
        msg->mt = VM_MT_CFG;
        msg->vid = vid;
        msg->offset = offset;
        msg->length = length;
        vca->msg_param.timeout = 1000;

        /* upref action so we can safely wait for it */
        virtio_client_action_upref(vc, vca);

        virtio_client_action_queue(vc, vca);
        osEventFlagsSet(vc->event, VC_EVENT_KICK);

        u32 flags = osEventFlagsWait(vca->done,
                VCA_EVENT_DONE, osFlagsWaitAny, osWaitForever);
        if (flags != VCA_EVENT_DONE) {
            ssdk_printf(SSDK_CRIT,
                    "vfe: %s dev %u fail to wait msg %x to be sent, flags %x\n",
                    get_domain_name(vc->domain), vid, *(u32 *)msg, flags);
        }

        /* release ref */
        virtio_client_action_downref(vc, vca);
    }
}

/*
 * reg read interface for MMIO+IRQ device
 */
static u32 __mmio_read(struct virtio_device_impl *impl, u8 offset)
{
    struct virtio_device *dev = &impl->dev;
    u32 *addr = (u32 *)((u8 *)dev->mmio_config + offset);
    return *addr;
}

/*
 * reg write interface for MMIO+IRQ device
 */
static void __mmio_write(struct virtio_device_impl *impl, u8 offset, u32 value)
{
    struct virtio_device *dev = &impl->dev;
    u32 *addr = (u32 *)((u8 *)dev->mmio_config + offset);
    *addr = value;
}

/*
 * reg read interface for shared_memory+mailbox device
 */
static u32 __mmio_read_mbox(struct virtio_device_impl *impl, u8 offset)
{
    struct virtio_device *dev = &impl->dev;
    u32 *addr = (u32 *)((u8 *)dev->mmio_config + offset);
    return *addr;
}

/*
 * reg write interface for shared_memory+mailbox device
 */
static void __mmio_write_mbox(struct virtio_device_impl *impl, u8 offset, u32 value)
{
    struct virtio_device *dev = &impl->dev;
    u32 *addr = (u32 *)((u8 *)dev->mmio_config + offset);
    *addr = value;

    if (offset == VIRTIO_MMIO_QUEUE_NOTIFY
            || offset == VIRTIO_MMIO_STATUS
            || offset == VIRTIO_MMIO_QUEUE_READY
            || offset == VIRTIO_MMIO_DEVICE_FEATURES_SEL
            || offset == VIRTIO_MMIO_DRIVER_FEATURES
            || offset == VIRTIO_MMIO_QUEUE_SEL)
        virtio_client_send_reg_update(impl->vc, impl->vid & VM_VID_MASK, offset);
}

/*
 * reg read interface for local device
 */
static u32 __mmio_read_local(struct virtio_device_impl *impl, u8 offset)
{
    struct virtio_device *dev = &impl->dev;
    u32 *addr = (u32 *)((u8 *)dev->mmio_config + offset);
    return *addr;
}

/*
 * reg write interface for local device
 */
static void __mmio_write_local(struct virtio_device_impl *impl, u8 offset, u32 value)
{
    struct virtio_device *dev = &impl->dev;
    u32 *addr = (u32 *)((u8 *)dev->mmio_config + offset);
    *addr = value;

    if (offset == VIRTIO_MMIO_QUEUE_NOTIFY
            || offset == VIRTIO_MMIO_STATUS
            || offset == VIRTIO_MMIO_QUEUE_READY
            || offset == VIRTIO_MMIO_DEVICE_FEATURES_SEL
            || offset == VIRTIO_MMIO_DRIVER_FEATURES
            || offset == VIRTIO_MMIO_QUEUE_SEL)
        virtio_agent_update_reg(impl->vid, offset);
}

void virtio_reset_device(struct virtio_device *dev)
{
    struct virtio_device_impl *impl = to_impl(dev);

    virtio_mmio_write(impl, VIRTIO_MMIO_STATUS, 0);
    dev->active_rings_bitmap = 0;
    impl->vq_cur = impl->vq_base;
}

u64 virtio_mmio_read_device_features(struct virtio_device *dev)
{
    struct virtio_device_impl *impl = to_impl(dev);

    virtio_mmio_write(impl, VIRTIO_MMIO_DEVICE_FEATURES_SEL, 0);
    u32 l = virtio_mmio_read(impl, VIRTIO_MMIO_DEVICE_FEATURES);
    virtio_mmio_write(impl, VIRTIO_MMIO_DEVICE_FEATURES_SEL, 1);
    u32 h = virtio_mmio_read(impl, VIRTIO_MMIO_DEVICE_FEATURES);

    return (((u64)h) << 32) | ((u64)l);
}

void virtio_mmio_write_driver_features(struct virtio_device *dev, u64 features)
{
    struct virtio_device_impl *impl = to_impl(dev);

    virtio_mmio_write(impl, VIRTIO_MMIO_DRIVER_FEATURES_SEL, 0);
    virtio_mmio_write(impl, VIRTIO_MMIO_DRIVER_FEATURES, features & 0xffffffffull);
    virtio_mmio_write(impl, VIRTIO_MMIO_DRIVER_FEATURES_SEL, 1);
    virtio_mmio_write(impl, VIRTIO_MMIO_DRIVER_FEATURES, (features >> 32) & 0xffffffffull);
}

u32 virtio_mmio_config_read_u32(struct virtio_device *dev, u8 offset)
{
    u32 *addr = (u32 *)((u8 *)dev->config_ptr + offset);
    return *addr;
}

void virtio_mmio_config_write_u32(struct virtio_device *dev, u8 offset, u32 value)
{
    u32 *addr = (u32 *)((u8 *)dev->config_ptr + offset);
    *addr = value;

    struct virtio_device_impl *impl = to_impl(dev);
    vc_t *vc = impl->vc;

    if (vc->type == VC_MBOX_SHMEM) {
        virtio_client_send_cfg_update(vc, impl->vid & VM_VID_MASK, offset, sizeof(u32));
    } else if (vc->type == VC_LOCAL) {
        virtio_agent_update_cfg(impl->vid, offset, sizeof(value));
    } else {
        ssdk_printf(SSDK_CRIT, "vfe: unimplemented cfg update ops\n");
    }
}

void virtio_status_acknowledge_driver(struct virtio_device *dev)
{
    struct virtio_device_impl *impl = to_impl(dev);

    u32 status = virtio_mmio_read(impl, VIRTIO_MMIO_STATUS);
    status |= VIRTIO_STATUS_ACKNOWLEDGE | VIRTIO_STATUS_DRIVER;
    virtio_mmio_write(impl, VIRTIO_MMIO_STATUS, status);
}

void virtio_free_desc(struct virtio_device *dev, uint ring_index, uint16_t desc_index)
{
    ssdk_printf(SSDK_DEBUG, "dev %p ring %u index %u free_count %u\n", dev, ring_index, desc_index, dev->ring[ring_index].free_count);
    dev->ring[ring_index].desc[desc_index].next = dev->ring[ring_index].free_list;
    dev->ring[ring_index].free_list = desc_index;
    dev->ring[ring_index].free_count++;
}

uint16_t virtio_alloc_desc(struct virtio_device *dev, uint ring_index)
{
    if (dev->ring[ring_index].free_count == 0)
        return 0xffff;

    assert(dev->ring[ring_index].free_list != 0xffff);

    uint16_t i = dev->ring[ring_index].free_list;
    struct vring_desc *desc = &dev->ring[ring_index].desc[i];
    desc->flags &= ~VRING_DESC_F_NEXT;

    dev->ring[ring_index].free_list = desc->next;
    dev->ring[ring_index].free_count--;

    return i;
}

struct vring_desc *virtio_alloc_desc_chain(struct virtio_device *dev, uint ring_index, size_t count, uint16_t *start_index)
{
    if (dev->ring[ring_index].free_count < count)
        return NULL;

    /* start popping entries off the chain */
    struct vring_desc *last = 0;
    uint16_t last_index = 0;
    while (count > 0) {
        uint16_t i = dev->ring[ring_index].free_list;
        struct vring_desc *desc = &dev->ring[ring_index].desc[i];

        dev->ring[ring_index].free_list = desc->next;
        dev->ring[ring_index].free_count--;

        if (last) {
            desc->flags = VRING_DESC_F_NEXT;
            desc->next = last_index;
        } else {
            // first one
            desc->flags = 0;
            desc->next = 0;
        }
        last = desc;
        last_index = i;
        count--;
    }

    if (start_index)
        *start_index = last_index;

    return last;
}

void virtio_submit_chain(struct virtio_device *dev, uint ring_index, uint16_t desc_index)
{
    ssdk_printf(SSDK_DEBUG, "dev %p, ring %u, desc %u\n", dev, ring_index, desc_index);

    /* add the chain to the available list */
    struct vring_avail *avail = dev->ring[ring_index].avail;
    avail->ring[avail->idx & dev->ring[ring_index].num_mask] = desc_index;
    avail->idx++;

#if LOCAL_TRACE
    hexdump(avail, 16);
#endif
}

void virtio_kick(struct virtio_device *dev, uint ring_index)
{
    ssdk_printf(SSDK_DEBUG, "dev %p, ring %u\n", dev, ring_index);

    struct virtio_device_impl *impl = to_impl(dev);

    virtio_mmio_write(impl, VIRTIO_MMIO_QUEUE_NOTIFY, ring_index);
}

int virtio_mmio_find_timeout(sdrv_mbox_rproc_t rproc,
        u32 id, struct virtio_device **pdev, u32 timeout)
{
    vfe_t *vfe = __vfe;
    vc_t *vc;
    vf_t *vf = NULL;
    struct virtio_device_impl *impl = NULL, *tmp;
    irq_state_t state;
    int domain, i;
    u32 flags;

    if (!pdev) {
        ssdk_printf(SSDK_CRIT, "vfe: invalid NULL arg pdev\n");
        return -1;
    }

    if (!vfe) {
        ssdk_printf(SSDK_CRIT, "vfe: frontend not ready\n");
        return -1;
    }

    domain = get_domain_from_rproc(rproc);
    if (domain < 0 || domain >= DOMAIN_MAX) {
        ssdk_printf(SSDK_CRIT, "vfe: invalid rproc %u\n", rproc);
        return -1;
    }

    vc = &vfe->clients[domain];

    state = spin_lock_irqsave(&vc->dev_lock);
    list_for_every_entry(&vc->dev_list, tmp, struct virtio_device_impl, list) {
        if (tmp->device_id == id) {
            impl = tmp;
            break;
        }
    }

    /* found it */
    if (impl) {
        *pdev = &impl->dev;
        spin_unlock_irqrestore(&vc->dev_lock, state);

        return 0;
    }

    /* not found and not gonna wait */
    if (!timeout) {
        *pdev = NULL;
        spin_unlock_irqrestore(&vc->dev_lock, state);

        return -1;
    }

    /* get an empty finder and waiting on it */
    for (i = 0; i < VC_FINDER_NUM; i++) {
        if (!vc->__finders[i].device_id) {
            osEventFlagsClear(vc->__finders[i].event, VF_EVENT_FOUND);
            vc->__finders[i].device_id = id;
            list_add_tail(&vc->finder_list, &vc->__finders[i].list);

            vf = &vc->__finders[i];
            break;
        }
    }
    spin_unlock_irqrestore(&vc->dev_lock, state);

    if (unlikely(!vf)) {
        ssdk_printf(SSDK_CRIT, "vfe: too few finders\n");
        return -1;
    }

    /* waiting for device */
    flags = osEventFlagsWait(vf->event,
            VF_EVENT_FOUND, osFlagsWaitAny, timeout);
    if (flags != VF_EVENT_FOUND) {
        ssdk_printf(SSDK_CRIT, "vfe: fail to wait\n");
        return -1;
    }

    state = spin_lock_irqsave(&vc->dev_lock);
    if (vf->impl) {
        /* device probed, finder is already deleted from list */
        impl = vf->impl;
        vf->device_id = 0;
        vf->impl = NULL;
    } else {
        /* device not probed, delete finder from list */
        vf->device_id = 0;
        vf->impl = NULL;
        list_delete(&vf->list);
    }
    spin_unlock_irqrestore(&vc->dev_lock, state);

    if (!impl) {
        return -1;
    }

    *pdev = &impl->dev;
    return 0;
}

static paddr_t virtio_client_alloc_vq(vc_t *vc,
        struct virtio_device_impl *impl, uint index, uint16_t len)
{
    struct virtio_device *dev = &impl->dev;

    if (dev->active_rings_bitmap & (1 << index)) {
        ssdk_printf(SSDK_CRIT, "vfe: vq[%u] already allocated\n", index);
        return 0;
    }

    /* allocate a ring */
    size_t size = vring_size(len, VIRTIO_VQ_ALIGN);
    size = (size + VIRTIO_VQ_ALIGN) & ~(VIRTIO_VQ_ALIGN - 1);
    ssdk_printf(SSDK_DEBUG, "need 0x%zx bytes\n", size);

    paddr_t pa = 0;

    if (vc->type == VC_MBOX_SHMEM) {
        if (impl->vq_cur + size > impl->vq_base + VIRTIO_DEVICE_VQ_SIZE) {
            ssdk_printf(SSDK_CRIT, "vfe: alloc %u for vq[%u] overflow\n", len, index);
            return 0;
        }

        pa = impl->vq_cur;
        impl->vq_cur += size;
    } else if (vc->type == VC_LOCAL) {
        vaddr_t va = (vaddr_t)memalign(VIRTIO_VQ_ALIGN, size);
        pa = v2p((void *)va);
    } else {
        ssdk_printf(SSDK_CRIT, "vfe: unimplemented type %d when allocating vq\n", vc->type);
    }

    return pa;
}

static inline int ispow2(u32 len)
{
    return !(len & (len - 1));
}

int virtio_alloc_ring(struct virtio_device *dev, uint index, uint16_t len)
{
    ssdk_printf(SSDK_DEBUG, "dev %p, index %u, len %u\n", dev, index, len);

    assert(dev);
    assert(len > 0 && ispow2(len));
    assert(index < MAX_VIRTIO_RINGS);
    assert(dev->mmio_config);

    if (len == 0 || !ispow2(len))
        return -1;

    struct virtio_device_impl *impl = to_impl(dev);

    virtio_mmio_write(impl, VIRTIO_MMIO_QUEUE_SEL, index);

    u32 ready = virtio_mmio_read(impl, VIRTIO_MMIO_QUEUE_READY);
    if (ready) {
        ssdk_printf(SSDK_CRIT, "vfe: dev %u vq[%u] already ready\n", impl->vid, index);
        return -1;
    }

    u32 max = virtio_mmio_read(impl, VIRTIO_MMIO_QUEUE_NUM_MAX);
    if (max < len) {
        ssdk_printf(SSDK_CRIT, "vfe: dev %u vq[%u] require %d but max %u\n",
                impl->vid, index, len, max);
        return -1;
    }

    /* compute the physical address */
    paddr_t pa = virtio_client_alloc_vq((vc_t *)impl->vc, impl, index, len);
    if (!pa) {
        ssdk_printf(SSDK_CRIT, "vfe: dev %u fail to alloc vq[%u]\n", impl->vid, index);
        return -1;
    }

    void *vptr = (void *)p2v(pa);

    /* initialize the ring */
    struct vring *ring = &dev->ring[index];

    vring_init(ring, len, vptr, VIRTIO_VQ_ALIGN);
    ring->free_list = 0xffff;
    ring->free_count = 0;

    /* make sure avail and used area is clear */
    ring->avail->flags = 0;
    ring->avail->idx = 0;
    ring->used->flags = 0;
    ring->used->idx = 0;

    /* add all the descriptors to the free list */
    for (uint i = 0; i < len; i++) {
        virtio_free_desc(dev, index, i);
    }

    /* register the ring with the device */
    struct vring vr;
    vring_init(&vr, len, (void *)pa, VIRTIO_VQ_ALIGN);
    virtio_mmio_write(impl, VIRTIO_MMIO_QUEUE_NUM, len);
    virtio_mmio_write(impl, VIRTIO_MMIO_QUEUE_DESC_LOW,
            (u64)((paddr_t)vr.desc) & 0xffffffffull);
    virtio_mmio_write(impl, VIRTIO_MMIO_QUEUE_DESC_HIGH,
            ((u64)((paddr_t)vr.desc) >> 32) & 0xffffffffull);
    virtio_mmio_write(impl, VIRTIO_MMIO_QUEUE_AVAIL_LOW,
            (u64)((paddr_t)vr.avail) & 0xffffffffull);
    virtio_mmio_write(impl, VIRTIO_MMIO_QUEUE_AVAIL_HIGH,
            ((u64)((paddr_t)vr.avail) >> 32) & 0xffffffffull);
    virtio_mmio_write(impl, VIRTIO_MMIO_QUEUE_USED_LOW,
            (u64)((paddr_t)vr.used) & 0xffffffffull);
    virtio_mmio_write(impl, VIRTIO_MMIO_QUEUE_USED_HIGH,
            ((u64)((paddr_t)vr.used) >> 32) & 0xffffffffull);

    virtio_mmio_write(impl, VIRTIO_MMIO_QUEUE_READY, 0x1);

    /* mark the ring active */
    dev->active_rings_bitmap |= (1 << index);

    return 0;
}

void virtio_status_driver_ok(struct virtio_device *dev)
{
    struct virtio_device_impl *impl = to_impl(dev);

    u32 status = virtio_mmio_read(impl, VIRTIO_MMIO_STATUS);
    status |= VIRTIO_STATUS_DRIVER_OK;
    virtio_mmio_write(impl, VIRTIO_MMIO_STATUS, status);
}

void virtio_status_failed(struct virtio_device *dev)
{
    struct virtio_device_impl *impl = to_impl(dev);

    u32 status = virtio_mmio_read(impl, VIRTIO_MMIO_STATUS);
    status |= VIRTIO_STATUS_FAILED;
    virtio_mmio_write(impl, VIRTIO_MMIO_STATUS, status);
}

static struct virtio_device_impl *virtio_client_find_device_with_id(vc_t *vc, u32 vid)
{
    struct virtio_device_impl *impl = NULL, *tmp;
    irq_state_t state;

    state = spin_lock_irqsave(&vc->dev_lock);
    list_for_every_entry(&vc->dev_list, tmp, struct virtio_device_impl, list) {
        if (tmp->vid == vid) {
            impl = tmp;
            break;
        }
    }
    spin_unlock_irqrestore(&vc->dev_lock, state);

    return impl;
}

static void virtio_provider_send_text(vdp_t *vdp, const char *str)
{
    struct virtio_device *dev = vdp->dev;
    struct vring_desc *desc;
    vpm_t *vpm;
    size_t len;
    u16 id;

    id = virtio_alloc_desc(dev, VQ_TX);
    if (id == 0xffff) {
        ssdk_printf(SSDK_CRIT, "vfe: not enough desc\n");
        return;
    }

    desc = virtio_desc_index_to_desc(dev, VQ_TX, id);
    vpm = (vpm_t *)p2v(desc->addr);

    len = strlen(str);
    if (len > VPM_TEXT_MAXLEN) {
        ssdk_printf(SSDK_CRIT, "vfe: trim length from %zu to %u\n", len, VPM_TEXT_MAXLEN);
        len = VPM_TEXT_MAXLEN;
    }

    vpm->type = VPM_TEXT;
    vpm->text.len = len;
    memcpy(vpm->text.str, str, len);
    vpm->text.str[len] = '\0';

    virtio_submit_chain(dev, VQ_TX, id);
    virtio_kick(dev, VQ_TX);

    ssdk_printf(SSDK_INFO, "vfe: send %u bytes\n", len);
}

static inline void virtio_provider_generate_arithmetic(vpm_t *vpm)
{
    static const char __ops[4] = { '+', '-', '*', '/' };

    vpm->type = VPM_ARITHMETIC;
    vpm->arithmetic.operand1 = rand() & 0xff;
    vpm->arithmetic.operand2 = rand() & 0xff;
    vpm->arithmetic._operator = __ops[rand() & 0x3];
    vpm->arithmetic.result = 0;
}

static void virtio_provider_send_chain(vdp_t *vdp, u32 num)
{
    struct virtio_device *dev = vdp->dev;
    struct vring_desc *desc;
    vpm_t *vpm;
    u16 i, start;

    if (num < 2 || num > TX_LEN) {
        ssdk_printf(SSDK_CRIT,
                "vfe: invalid chain num %u, should be in [2, %u]\n",
                num, TX_LEN);
        return;
    }

    ssdk_printf(SSDK_INFO, "vfe: desc before alloc\n");
    for (i = 0; i < TX_LEN; i++) {
        desc = &dev->ring[VQ_TX].desc[i];
        ssdk_printf(SSDK_INFO,
                "vfe: desc[%u] addr %llx len %x flags %x next %u\n",
                i, desc->addr, desc->len, desc->flags, desc->next);
    }

    desc = virtio_alloc_desc_chain(dev, VQ_TX, num, &start);
    if (!desc) {
        ssdk_printf(SSDK_CRIT, "vfe: not enough desc\n");
        return;
    }

    ssdk_printf(SSDK_INFO, "vfe: alloc %u start %u\n", num, start);

    for (i = 0; i < num; i++) {
        vpm = (vpm_t *)p2v(desc->addr);
        virtio_provider_generate_arithmetic(vpm);
        desc = virtio_desc_index_to_desc(dev, VQ_TX, desc->next);
    }

    ssdk_printf(SSDK_INFO, "vfe: desc after alloc\n");
    for (i = 0; i < TX_LEN; i++) {
        desc = &dev->ring[VQ_TX].desc[i];
        ssdk_printf(SSDK_INFO,
                "vfe: desc[%u] addr %llx len %x flags %x next %u\n",
                i, desc->addr, desc->len, desc->flags, desc->next);
    }

    ssdk_printf(SSDK_INFO,
            "vfe: avail before submit %u\n",
            dev->ring[VQ_TX].avail->idx);

    virtio_submit_chain(dev, VQ_TX, start);

    ssdk_printf(SSDK_INFO,
            "vfe: avail after submit %u\n",
            dev->ring[VQ_TX].avail->idx);

    virtio_kick(dev, VQ_TX);
}

static void virtio_provider_query_devices(vc_t *vc)
{
}

static int virtio_client_mbox_callback(void *arg, u8 *buf, u32 len)
{
    vc_t *vc = (vc_t *)arg;
    msg_t *msg = (msg_t *)buf;

    if (unlikely(len != sizeof(msg_t))) {
        ssdk_printf(SSDK_CRIT, "vfe: invalid msg len %u\n", len);
        return -1;
    }

    ssdk_printf(SSDK_DEBUG, "vfe: recv %08x from %s\n",
            *(u32 *)buf, get_domain_name(vc->domain));

    switch (msg->mt) {
    case VM_MT_BOOT:
        {
            vca_t *vca = virtio_client_action_get_and_upref(vc);
            if (!vca) {
                ssdk_printf(SSDK_CRIT, "vfe: not enough vca for %s, boot req lost\n",
                        get_domain_name(vc->domain));
                break;
            }

            vca->action = VCA_BOOT;
            vca->notify = 0;
            virtio_client_action_queue(vc, vca);
            osEventFlagsSet(vc->event, VC_EVENT_KICK);
        }
        break;
    case VM_MT_IRQ:
        {
            /* for LK fe, just call user callback in irq context */
            struct virtio_device_impl *impl;

            impl = virtio_client_find_device_with_id(vc, msg->vid);
            if (unlikely(!impl)) {
                ssdk_printf(SSDK_CRIT, "vfe: no fe has vid %u\n", msg->vid);
                break;
            }

            virtio_mmio_irq(&impl->dev);
        }
        break;
    default:
        {
            ssdk_printf(SSDK_CRIT, "vfe: unhandled msg %x\n", *(u32 *)msg);
        }
        break;
    }

    return 0;
}

static struct virtio_device_impl *virtio_client_detect_device(vc_t *vc, paddr_t addr, u32 vid)
{
    struct virtio_device_impl *impl;

    impl = malloc(sizeof(struct virtio_device_impl));
    if (!impl) {
        ssdk_printf(SSDK_CRIT, "vfe: fail to alloc virtio_device_impl\n");
        return NULL;
    }
    memset(impl, 0, sizeof(*impl));

    impl->vid = vid;
    impl->vc = vc;

    vaddr_t vptr = p2v(addr);
    volatile struct virtio_mmio_config *mmio = (struct virtio_mmio_config *)vptr;
    impl->dev.mmio_config = mmio;
    impl->dev.config_ptr = (void *)mmio->config;

    u32 magic = virtio_mmio_read(impl, VIRTIO_MMIO_MAGIC_VALUE);
    if (magic != VIRTIO_MMIO_MAGIC) {
        ssdk_printf(SSDK_CRIT, "vfe: invalid magic %x\n", magic);
        free(impl);
        return NULL;
    }

    impl->device_id = virtio_mmio_read(impl, VIRTIO_MMIO_DEVICE_ID);
    impl->dev.valid = 1;
    list_initialize(&impl->list);

    /* only VC_MBOX_SHMEM type needs this */
    impl->vq_base = impl->vq_cur = 0;
    if (vc->type == VC_MBOX_SHMEM) {
        impl->vq_base = vc->base
            + VIRTIO_DEVICE_REG_TOTAL/* reg+cfg area for all devices */
            + VIRTIO_PROVIDER_MEM_SIZE/* provider buffer area */
            + impl->vid * VIRTIO_DEVICE_VQ_SIZE;/* vq area for each device */
        impl->vq_cur = impl->vq_base;
    }

    /* TODO check id conflict */
    irq_state_t state;
    state = spin_lock_irqsave(&vc->dev_lock);
    list_add_tail(&vc->dev_list, &impl->list);
    spin_unlock_irqrestore(&vc->dev_lock, state);

    ssdk_printf(SSDK_INFO, "vfe: detect vdev %u id %u\n", impl->vid, impl->device_id);

    return impl;
}

static void virtio_provider_handle_text_msg_locked(vdp_t *vdp, vpm_t *vpm)
{
    struct virtio_device *dev = vdp->dev;
    struct virtio_device_impl *impl = to_impl(dev);
    vc_t *vc = impl->vc;

    printf("vfe: recv %u bytes from %s:\n",
            vpm->text.len, get_domain_name(vc->domain));

    vpm->text.str[vpm->text.len] = '\0';
    printf("vfe: %s\n", vpm->text.str);
}

static void virtio_provider_handle_dev_msg_locked(vdp_t *vdp, vpm_t *vpm)
{
    struct virtio_device *dev = vdp->dev;
    struct virtio_device_impl *impl = to_impl(dev);
    struct virtio_device_impl *tmp;
    vc_t *vc = impl->vc;
    vca_t *vca;
    int probe = 0;

    for (u32 i = 0; i < vpm->device.num; i++) {
        tmp = virtio_client_find_device_with_id(vc,
                vpm->device.devs[i].vid);

        /* already probe */
        if (tmp)
            continue;

        /* ask provider thread to probe this */
        vca = virtio_client_action_get_and_upref(vc);
        if (unlikely(!vca)) {
            ssdk_printf(SSDK_CRIT, "vfe: not enough vca, consider enlarge list\n");
            break;
        }

        vca->action = VCA_DEV;
        vca->notify = 0;
        vca->dev_param = vpm->device.devs[i];
        virtio_client_action_queue(vc, vca);
        probe = 1;
    }

    if (probe)
        osEventFlagsSet(vc->event, VC_EVENT_KICK);
}

static inline void virtio_provider_handle_arithmetic_msg_locked(vdp_t *vdp, vpm_t *vpm)
{
    ssdk_printf(SSDK_INFO, "vfe: %u %c %u = %u\n",
            vpm->arithmetic.operand1,
            (char)vpm->arithmetic._operator,
            vpm->arithmetic.operand2,
            vpm->arithmetic.result);
}

static int virtio_provider_callback(struct virtio_device *dev,
        uint ring, const struct vring_used_elem *e)
{
    struct vring_desc *desc;
    vpm_t *vpm;
    u16 i, n;

    if (ring == VQ_TX) {
        /* VQ_TX may send desc chain to peer */
        i = e->id;
        while (i < TX_LEN) {
            desc = virtio_desc_index_to_desc(dev, VQ_TX, i);
            vpm = (vpm_t *)p2v(desc->addr);

            if (vpm->type == VPM_TEXT) {
                /* simply free this */
            } else if (vpm->type == VPM_TIMESTAMP) {
                /* TODO handle timestamp msg */
            } else if (vpm->type == VPM_DEVICE) {
                /* query result from be */
                virtio_provider_handle_dev_msg_locked(dev->priv, vpm);
            } else if (vpm->type == VPM_ARITHMETIC) {
                /* show arithmetic result */
                virtio_provider_handle_arithmetic_msg_locked(dev->priv, vpm);
            } else {
                /* invalid msg type, ignore this */
            }

            n = desc->flags & VRING_DESC_F_NEXT ? desc->next : TX_LEN;
            virtio_free_desc(dev, VQ_TX, i);
            i = n;
        }
    } else {
        /* VQ_RX is supposed to send only one desc a time */
        desc = virtio_desc_index_to_desc(dev, ring, e->id);
        vpm = (vpm_t *)p2v(desc->addr);

        if (vpm->type == VPM_TEXT) {
            /* print it out, just for debug purpose */
            virtio_provider_handle_text_msg_locked(dev->priv, vpm);
        } else if (vpm->type == VPM_TIMESTAMP) {
            /* TODO handle timestamp msg */
        } else if (vpm->type == VPM_DEVICE) {
            /* new device added */
            virtio_provider_handle_dev_msg_locked(dev->priv, vpm);
        } else {
            /* invalid msg type, ignore this */
        }

        virtio_submit_chain(dev, ring, e->id);
    }

    return 0;
}

static void virtio_client_init_provider(vc_t *vc, struct virtio_device *dev)
{
    vfe_t *vfe;
    vdp_t *vdp;
    vpm_t *vpm;
    struct vring_desc *desc;
    u64 features;
    paddr_t pa;
    int err;
    u16 id, ids[TX_LEN];

    vdp = malloc(sizeof(vdp_t));
    if (!vdp) {
        ssdk_printf(SSDK_CRIT,
                "vfe: fail to alloc vdp %s\n",
                get_domain_name(vc->domain));
        return;
    }
    vdp->dev = dev;

    /* reset device to start */
    virtio_reset_device(dev);

    /* get device features */
    features = virtio_mmio_read_device_features(dev);

    /* settle down driver features */
    features &= 0;
    virtio_mmio_write_driver_features(dev, features);

    /* tell be we're from ap domain, so be will use ap2p on physical address */
    vfe = vc->vfe;
    if (vfe->domain == DOMAIN_AP1 || vfe->domain == DOMAIN_AP2) {
        virtio_mmio_config_write_u32(dev, VP_CFG_ADDR_AP_DOMAIN, 1);
    }

    /* we've seen the device */
    virtio_status_acknowledge_driver(dev);

    /* alloc rings */
    err = virtio_alloc_ring(dev, VQ_TX, TX_LEN);
    err |= virtio_alloc_ring(dev, VQ_RX, RX_LEN);
    if (err) {
        ssdk_printf(SSDK_CRIT,
                "vfe: fail to alloc vdp ring %s\n",
                get_domain_name(vc->domain));
        return;
    }

    dev->irq_driver_callback = virtio_provider_callback;
    dev->priv = vdp;

    /* everything is ok now */
    virtio_status_driver_ok(dev);

    if (vc->type == VC_MBOX_SHMEM) {
        pa = vc->base + VIRTIO_DEVICE_REG_TOTAL;
    } else if (vc->type == VC_LOCAL) {
        vaddr_t va = (vaddr_t)memalign(0x100, VIRTIO_PROVIDER_MEM_SIZE);
        pa = v2p((void *)va);
    } else {
        ssdk_printf(SSDK_CRIT,
                "vfe: unimplemented type %d when allocating vdp buf\n",
                vc->type);
        return;
    }

    /* setup desc for TX */
    for (int i = 0; i < TX_LEN; i++) {
        id = virtio_alloc_desc(dev, VQ_TX);
        desc = virtio_desc_index_to_desc(dev, VQ_TX, id);

        desc->addr = pa;
        pa += TX_BUFSIZE;
        desc->len = TX_BUFSIZE;
        ids[i] = id;
    }
    for (int i = 0; i < RX_LEN; i++) {
        virtio_free_desc(dev, VQ_TX, ids[i]);
    }

    /* setup desc for RX */
    for (int i = 0; i < RX_LEN; i++) {
        id = virtio_alloc_desc(dev, VQ_RX);
        desc = virtio_desc_index_to_desc(dev, VQ_RX, id);

        desc->addr = pa;
        pa += TX_BUFSIZE;
        desc->len = RX_BUFSIZE;
        virtio_submit_chain(dev, VQ_RX, id);
    }

    /* query backend to see what devices we have */
    id = virtio_alloc_desc(dev, VQ_TX);
    desc = virtio_desc_index_to_desc(dev, VQ_TX, id);
    vpm = (vpm_t *)p2v(desc->addr);
    vpm->type = VPM_DEVICE;
    vpm->device.num = 0;
    virtio_submit_chain(dev, VQ_TX, id);
    virtio_kick(dev, VQ_TX);

    vc->vdp = vdp;
}

static void virtio_client_thread(void *arg)
{
    vc_t *vc = arg;
    vca_t *vca;
    u32 flags;

    for (;;) {
        flags = osEventFlagsWait(vc->event,
                VC_EVENT_KICK, osFlagsWaitAny, osWaitForever);
        if (flags != VC_EVENT_KICK)
            continue;

        while ((vca = virtio_client_action_dequeue(vc))) {
            switch (vca->action) {
            case VCA_BOOT:
                {
                    if (vc->boot_msg) {
                        /* TODO be has rebooted, need to remove all devices */
                        ssdk_printf(SSDK_INFO,
                                "vfe: remove all devices %s\n",
                                get_domain_name(vc->domain));
                    }

                    /* virtio provider is first virtio device */
                    struct virtio_device_impl *impl = virtio_client_detect_device(vc, vc->base, 0);
                    if (!impl) {
                        ssdk_printf(SSDK_CRIT,
                                "vfe: fail to init virtio provider %s\n",
                                get_domain_name(vc->domain));
                        break;
                    }

                    /* check device id */
                    if (impl->device_id != VIRTIO_ID_PROVIDER) {
                        ssdk_printf(SSDK_CRIT,
                                "vfe: invalid id %x for virtio device provider %s\n",
                                impl->device_id, get_domain_name(vc->domain));
                        break;
                    }

                    virtio_client_init_provider(vc, &impl->dev);
                    vc->boot_msg = 1;
                }
                break;
            case VCA_MSG:
                {
                    /* other thread would like us to send a mbox msg */
                    msg_t *msg = (msg_t *)&vca->msg_param.msg;
                    int ret, retry = 0;

                    ret = mbox_send(vc->chan, (u8 *)&vca->msg_param.msg,
                            sizeof(msg_t), vca->msg_param.timeout);

                    if (msg->mt == VM_MT_BOOT) {
                        /* retry 1000ms for BOOT msg if no hw chan */
                        while (ret == ERR_NO_HW_CHAN && retry < 100) {
                            osDelay(10);
                            ret = mbox_send(vc->chan,
                                    (u8 *)&vca->msg_param.msg,
                                    sizeof(msg_t),
                                    vca->msg_param.timeout);
                            retry++;
                        }

                        if (ret == ERR_NO_HW_CHAN)
                            ssdk_printf(SSDK_CRIT,
                                    "BOOT msg to %d fail after 1000ms\n",
                                    vc->domain);
                    }
                }
                break;
            case VCA_DEV:
                {
                    struct virtio_device_impl *impl;

                    impl = virtio_client_detect_device(vc,
                            vca->dev_param.addr, vca->dev_param.vid);
                    if (!impl) {
                        ssdk_printf(SSDK_CRIT,
                                "vfe: fail to detect device %u at %x\n",
                                vca->dev_param.vid, vca->dev_param.addr);
                        break;
                    }

                    vf_t *vf = NULL, *tmp;
                    irq_state_t state;

                    /* check if anyone is waiting for this device */
                    state = spin_lock_irqsave(&vc->dev_lock);
                    list_for_every_entry(&vc->finder_list, tmp, vf_t, list) {
                        if (tmp->device_id == impl->device_id) {
                            vf = tmp;
                            break;
                        }
                    }

                    if (vf) {
                        vf->impl = impl;
                        list_delete(&vf->list);
                        spin_unlock_irqrestore(&vc->dev_lock, state);
                        osEventFlagsSet(vf->event, VF_EVENT_FOUND);
                    } else {
                        spin_unlock_irqrestore(&vc->dev_lock, state);
                    }
                }
                break;
            default:
                break;
            }

            if (vca->notify) {
                osEventFlagsSet(vca->done, VCA_EVENT_DONE);
            }

            virtio_client_action_downref(vc, vca);
        }
    }

    ssdk_printf(SSDK_CRIT,
            "vfe: client thread exit %s\n",
            get_domain_name(vc->domain));
}

static void virtio_local_on_backend_found(paddr_t reg, void *priv)
{
    vc_t *vc = priv;
    vc->base = reg;

    vca_t *vca = virtio_client_action_get_and_upref(vc);
    if (!vca) {
        ssdk_printf(SSDK_CRIT,
                "vfe: not enough vca for %s, boot req lost\n",
                get_domain_name(vc->domain));
        return;
    }

    vca->action = VCA_BOOT;
    vca->notify = 0;
    virtio_client_action_queue(vc, vca);
    osEventFlagsSet(vc->event, VC_EVENT_KICK);
}

static void virtio_local_on_irq(u32 vid, void *priv)
{
    vc_t *vc = priv;
    struct virtio_device_impl *impl;

    impl = virtio_client_find_device_with_id(vc, vid);
    if (impl) {
        /*
         * simulate a irq context
         * TODO current event wrapper doesn't support call
         * event_signal() when int disabled
         */

        //arch_disable_ints();
        virtio_mmio_irq(&impl->dev);
        //arch_enable_ints();
    }
}

static int virtio_client_init_local(vfe_t *vfe, vc_t *vc)
{
    vc->type = VC_LOCAL;

    vc->read = __mmio_read_local;
    vc->write = __mmio_write_local;

    vc->vfo.on_backend_found = virtio_local_on_backend_found;
    vc->vfo.on_irq = virtio_local_on_irq;

    virtio_agent_register_frontend(&vc->vfo, vc);

    return 0;
}

#if (__HAS_MMU == 0)
static inline int virtio_client_remap(vc_t *vc)
{
    /* on R5 use mpu mapping in platform.c */
    return 0;
}
#else
/*
 * remap as uc
 */
static int virtio_client_remap(vc_t *vc)
{
    paddr_t pa = vc->base;
    size_t sz = VIRTIO_MEMSIZE;
    vaddr_t va = p2v(pa);

    ssdk_printf(SSDK_INFO, "vfe: remap %zx(%zx) size %zx as uc\n", pa, va, sz);

    vmm_aspace_t *aspace = vmm_get_kernel_aspace();
    int err = arch_mmu_unmap(&aspace->arch_aspace,
            va,
            sz / PAGE_SIZE);
    if (err) {
        ssdk_printf(SSDK_CRIT, "vfe: unmap failed %d\n", err);
        return -1;
    }

    err = arch_mmu_map(&aspace->arch_aspace,
            va,
            pa,
            sz / PAGE_SIZE,
            ARCH_MMU_FLAG_UNCACHED);
    if (err) {
        ssdk_printf(SSDK_CRIT, "vfe: map failed %d\n", err);
        return -1;
    }

    return 0;
}
#endif

static int virtio_client_init_remote(vfe_t *vfe, vc_t *vc)
{
    vc->type = VC_MBOX_SHMEM;

    vc->read = __mmio_read_mbox;
    vc->write = __mmio_write_mbox;

    /* TODO currently ssdk is only for safety and mp */
#if 0
    if (virtio_client_remap(vc) < 0) {
        ssdk_printf(SSDK_CRIT, "vfe: fail to remap as uc\n");
        return -1;
    }
#endif

    struct sdrv_mbox_chan_req chan_req;
    chan_req.rproc = vc->rproc;
    chan_req.src_addr = vc->addr;
    chan_req.dest_addr = vc->addr;
    vc->chan = mbox_request_channel(g_mbox_dev, &chan_req);
    if (!vc->chan) {
        ssdk_printf(SSDK_CRIT,
                "vfe: fail to get mbox channel %d %x\n",
                vc->rproc, vc->addr);
        return -1;
    }
    mbox_set_callback(vc->chan, virtio_client_mbox_callback, vc);

    vca_t *vca = virtio_client_action_get_and_upref(vc);
    /* vca will never be NULL here */
    vca->action = VCA_MSG;
    vca->notify = 0;

    /* send boot msg */
    msg_t *msg = &vca->msg_param.msg;
    memset(msg, 0, sizeof(*msg));
    msg->mt = VM_MT_BOOT;
    msg->ac = 0;
    msg->ar = 1;
    vca->msg_param.timeout = 1;

    virtio_client_action_queue(vc, vca);
    osEventFlagsSet(vc->event, VC_EVENT_KICK);

    return 0;
}

static inline void virtio_frontend_client_init(vfe_t *vfe, vc_t *vc)
{
    osThreadAttr_t attr;

    vc->read = NULL;
    vc->write = NULL;

    spin_lock_init(&vc->lock);
    spin_lock_init(&vc->dev_lock);
    list_initialize(&vc->dev_list);

    list_initialize(&vc->finder_list);
    for (int i = 0; i < VC_FINDER_NUM; i++) {
        vf_t *vf = &vc->__finders[i];

        vf->event = osEventFlagsNew(NULL);
        vf->device_id = 0;
        vf->impl = NULL;
        list_initialize(&vf->list);
    }

    vc->event = osEventFlagsNew(NULL);
    list_initialize(&vc->action_list);
    list_initialize(&vc->idle_list);
    for (int i = 0; i < VC_ACTION_NUM; i++) {
        vca_t *vca = &vc->__actions[i];

        list_initialize(&vca->list);
        vca->refcount = 0;
        vca->action = VCA_NONE;
        vca->notify = 0;
        vca->done = osEventFlagsNew(NULL);

        list_add_tail(&vc->idle_list, &vca->list);
    }

    vc->boot_msg = 0;

    memset(&attr, 0, sizeof(attr));
    attr.name = "virtio-client";
    attr.stack_size = 4096,
    attr.priority = osPriorityNormal;
    vc->task = osThreadNew(virtio_client_thread, vc, &attr);
    if (!vc->task) {
        ssdk_printf(SSDK_CRIT,
                "vfe: fail to create thread %s\n",
                get_domain_name(vc->domain));
        return;
    }

    if (vfe->domain == vc->domain) {
        virtio_client_init_local(vfe, vc);
    } else {
        virtio_client_init_remote(vfe, vc);
    }
}

void virtio_frontend_init(void)
{
    vfe_t *vfe = malloc(sizeof(vfe_t));
    if (!vfe) {
        ssdk_printf(SSDK_CRIT,
                "vfe: fail to alloc %zu bytes\n",
                sizeof(vfe_t));
        return;
    }
    memset(vfe, 0, sizeof(*vfe));

    for (int i = 0; i < DOMAIN_MAX; i++) {
        vc_t *vc = &vfe->clients[i];

        vc->vfe = vfe;
        vc->domain = i;
        vc->type = VC_NORMAL_MMIO;
    }

    /* init from cfg */
    const cfg_t *cfg = get_frontend_config();

    vfe->domain = cfg->my_domain;
    for (int i = 0; i < cfg->peer_num; i++) {
        const peer_t *peer = &cfg->peers[i];
        vc_t *vc = &vfe->clients[peer->domain];

        /* mem_base 0 for remote means disabled */
        if (vc->domain != vfe->domain && !peer->mem_base)
            continue;

        if (vc->domain != vfe->domain) {
            vc->rproc = get_rproc_from_domain(vc->domain);
            vc->addr = peer->mbox_addr;
            vc->base = peer->mem_base;

            if (vfe->domain == DOMAIN_AP1 || vfe->domain == DOMAIN_AP2)
                vc->base += 0x10000000;
        }

        virtio_frontend_client_init(vfe, vc);
    }

    __vfe = vfe;
}

static void virtio_frontend_dump(vfe_t *vfe)
{
    for (int i = 0; i < DOMAIN_MAX; i++) {
        vc_t *vc = &vfe->clients[i];
        vdp_t *vdp = vc->vdp;

        if (!vc->vdp)
            continue;

        if (vc->type == VC_MBOX_SHMEM) {
            printf("client %s vdp %p  mbox %u %u mem %x devs:\n",
                    get_domain_name(vc->domain), vdp->dev->mmio_config,
                    vc->rproc, vc->addr, vc->base);
        } else if (vc->type == VC_LOCAL) {
            printf("client %s vdp %p devs:\n",
                    get_domain_name(vc->domain), vdp->dev->mmio_config);
        }

        struct virtio_device_impl *impl, *tmp;
        list_for_every_entry_safe(&vc->dev_list, impl, tmp, struct virtio_device_impl, list) {
            printf(" - dev[%u] id %u mmio %p priv %p vqs:\n",
                    impl->vid, impl->device_id,
                    impl->dev.mmio_config, impl->dev.priv);
            for (int j = 0; j < MAX_VIRTIO_RINGS; j++) {
                struct vring *r = &impl->dev.ring[j];
                printf("     vq[%d] num %u free %u last %u desc %p avail %p used %p\n",
                        j, r->num, r->free_count, r->last_used,
                        r->desc, r->avail, r->used);
            }
        }
    }
}

static int virtio_debug_usage(void)
{
    printf("Usage: vfe CMD OPTIONS\n");
    printf("\n");
    printf("Available CMDs:\n");
    printf(" - chain [rproc] [num]\n");
    printf("        send a chain of desc to peer, see if it works\n");
    printf(" - dump\n");
    printf("        dump clients and devices\n");
    printf(" - find [rproc] [id] [timeout]\n");
    printf("        find device with id from rproc, timeout in ms\n");
#if ENABLE_VIRTIO_DEBUG
    printf(" - dummy [rproc] [id]\n");
    printf("        init a dummy device with id from rproc\n");
#endif
#if 0
    printf(" - query [rproc]\n");
    printf("        query devices on rproc\n");
#endif
    printf(" - text [rproc] [string]\n");
    printf("        send string to a client on rproc\n");

    return 0;
}

static int virtio_frontend_debug_entry(int argc, char *argv[])
{
    /* someone's already removed argv[0] for us */
    argc += 1;
    argv -= 1;

    if (argc < 2)
        return virtio_debug_usage();

    vfe_t *vfe = __vfe;
    if (!vfe) {
        printf("frontend not ready\n");
        return 0;
    }

    if (!strncmp(argv[1], "dump", 4)) {
        virtio_frontend_dump(vfe);
    } else if (!strncmp(argv[1], "find", 4)) {
        if (argc < 5) {
            printf("too few args\n");
            return virtio_debug_usage();
        }

        struct virtio_device *dev;
        u32 id = atoi(argv[3]);
        int err = virtio_mmio_find_timeout(
                atoi(argv[2]), id, &dev, atoi(argv[4]));
        if (err < 0) {
            printf("device %u not found, err %d\n", id, err);
            return 0;
        }

        printf("find device %u at %p\n", id, dev->mmio_config);
#if ENABLE_VIRTIO_DEBUG
    } else if (!strncmp(argv[1], "dummy", 5)) {
        if (argc < 4) {
            printf("too few args\n");
            return virtio_debug_usage();
        }

        struct virtio_device *dev;
        u32 id = atoi(argv[3]);
        int err = virtio_mmio_find_timeout(atoi(argv[2]), id, &dev, 0);
        if (err < 0) {
            printf("device %u not found, err %d\n", id, err);
            return 0;
        }
        printf("found dummy device %p\n", dev);

        virtio_reset_device(dev);
        virtio_status_acknowledge_driver(dev);
        for (int i = 0; i < 2; i++) {
            err = virtio_alloc_ring(dev, i, 128);
            if (err) {
                printf("fail to alloc ring %d for dummy device %p\n", i, dev);
                virtio_status_failed(dev);
                return -1;
            }
        }

        virtio_status_driver_ok(dev);
        printf("dummy device %p ready\n", dev);
#endif
    } else if (!strncmp(argv[1], "query", 5)) {
        /* TODO remove this */
        if (argc < 3) {
            printf("too few args\n");
            return virtio_debug_usage();
        }

        sdrv_mbox_rproc_t rproc = atoi(argv[2]);
        int domain = get_domain_from_rproc(rproc);
        if (domain < 0 || domain >= DOMAIN_MAX) {
            printf("invalid rproc %s\n", argv[2]);
            return 0;
        }

        vc_t *vc = &vfe->clients[domain];
        if (!vc->vdp) {
            printf("client for rproc %u not ready\n", rproc);
            return 0;
        }

        virtio_provider_query_devices(&vfe->clients[domain]);
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

        vc_t *vc = &vfe->clients[domain];
        if (!vc->vdp) {
            printf("client for rproc %u not ready\n", rproc);
            return 0;
        }

        virtio_provider_send_text(vc->vdp, argv[3]);
    } else if (!strncmp(argv[1], "chain", 5)) {
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

        vc_t *vc = &vfe->clients[domain];
        if (!vc->vdp) {
            printf("client for rproc %u not ready\n", rproc);
            return 0;
        }

        virtio_provider_send_chain(vc->vdp, atoi(argv[3]));
    } else {
        printf("unknown CMD '%s'\n", argv[1]);
        return virtio_debug_usage();
    }

    return 0;
}

CLI_CMD("vfe", "virtio frontend debug commands", virtio_frontend_debug_entry);
