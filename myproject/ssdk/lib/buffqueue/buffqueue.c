/**
 * @file buffqueue.c
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#include <lib/buffqueue.h>
#include <cmsis_os2.h>
#include <string.h>
#include <arch/barriers.h>

#define VRING_ALIGN  CONFIG_BUFFQUEUE_VRING_ALIGN
#define wmb()  DSB
#define rmb()  DSB

struct vring_desc
{
    /* Address (guest-physical). */
    uint64_t addr;
    /* Length. */
    uint32_t len;
};

struct vring_avail
{
    uint16_t idx;
    uint16_t ring[1];
};

struct vring_used
{
    uint16_t idx;
    uint16_t ring[1];
};

struct vring
{
    unsigned int num;

    struct vring_desc *desc;
    struct vring_avail *avail;
    struct vring_used *used;
};

struct buffqueue
{
    uint32_t vring_size;
    uint32_t nentries;
    struct vring ring;
    uint16_t used_cons_idx;
    uint16_t available_idx;
};

static inline int vring_size(unsigned int num, unsigned long align)
{
    int size;

    size = num * sizeof(struct vring_desc);
    size += sizeof(struct vring_avail) + (num * sizeof(uint16_t)) + sizeof(uint16_t);
    size = (size + align - 1) & ~(align - 1);
    size += sizeof(struct vring_used) + (num * sizeof(uint16_t)) + sizeof(uint16_t);
    size = (size + align - 1) & ~(align - 1);
    return size;
}

static inline void vring_init(struct vring *vr, unsigned int num, uint8_t *p, unsigned long align)
{
    vr->num = num;
    vr->desc = (struct vring_desc *)p;
    vr->avail = (struct vring_avail *)(p + num * sizeof(struct vring_desc));
    vr->used = (struct vring_used *)(((unsigned long)&vr->avail->ring[num] + align - 1) & ~(align - 1));
}

static void ring_add_buffer(struct vring_desc *desc, uint16_t head_idx, void *buffer, uint32_t length)
{
    struct vring_desc *dp;

    dp = &desc[head_idx];
    dp->addr = (paddr_t)buffer;
    dp->len = length;
}

static void ring_update_used(struct buffqueue *q, uint16_t head_idx)
{
    uint16_t used_idx;

    used_idx = q->ring.used->idx & (q->nentries - 1);

    q->ring.used->ring[used_idx] = head_idx;

    wmb();

    q->ring.used->idx++;
}

buffqueue_t buffqueue_create(uint32_t size, uint32_t num)
{
    struct buffqueue *q;
    uint32_t alloc_size;
    uint8_t *ptr;

    if ((num & (num - 1)) != 0) {
        return NULL;
    }

    if ((size & (size - 1)) != 0) {
        return NULL;
    }

    alloc_size = sizeof(struct buffqueue)
        + vring_size(num, VRING_ALIGN) + size * num;

    q = (struct buffqueue *)osAlloc(alloc_size);
    if (q == NULL) {
        return NULL;
    }

    memset(q, 0x00, alloc_size);

    q->vring_size = size;
    q->nentries = num;
    ptr = (uint8_t *)q + sizeof(struct buffqueue);
    vring_init(&q->ring, num, ptr, VRING_ALIGN);
    ptr += vring_size(num, VRING_ALIGN);

    for (uint32_t i = 0; i < num; i++, ptr += size) {
        ring_add_buffer(q->ring.desc, i, ptr, size);
        ring_update_used(q, i);
    }

    return (buffqueue_t)q;
}

void *buffqueue_alloc_buffer(buffqueue_t bq, uint32_t *len, uint16_t *idx)
{
    struct buffqueue *q = (struct buffqueue *)bq;
    uint16_t used_idx, desc_idx;

    if (q->used_cons_idx == q->ring.used->idx) {
        return NULL;
    }

    used_idx = (uint16_t)(q->used_cons_idx & ((uint16_t)(q->nentries - 1U)));
    desc_idx = q->ring.used->ring[used_idx];

    rmb();

    *len = q->ring.desc[desc_idx].len;
    *idx = desc_idx;
    q->used_cons_idx++;

    return (void *)((paddr_t)(q->ring.desc[desc_idx].addr));
}

void buffqueue_enqueue(buffqueue_t bq, uint16_t idx)
{
    struct buffqueue *q = (struct buffqueue *)bq;
    uint16_t avail_idx;

    avail_idx = (uint16_t)(q->ring.avail->idx & ((uint16_t)(q->nentries - 1U)));
    q->ring.avail->ring[avail_idx] = idx;

    wmb();

    q->ring.avail->idx++;
}

void *buffqueue_dequeue(buffqueue_t bq, uint32_t *len, uint16_t *idx)
{
    struct buffqueue *q = (struct buffqueue *)bq;
    uint16_t head_idx = 0;
    void *buffer;

    if (q->available_idx == q->ring.avail->idx) {
        return NULL;
    }

    head_idx = (uint16_t)(q->available_idx++ & ((uint16_t)(q->nentries - 1U)));
    *idx = q->ring.avail->ring[head_idx];

    rmb();

    buffer = (void *)((paddr_t)(q->ring.desc[*idx].addr));
    *len = q->ring.desc[*idx].len;

    return buffer;
}

void buffqueue_free_buffer(buffqueue_t bq, uint16_t idx)
{
    struct buffqueue *q = (struct buffqueue *)bq;

    ring_update_used(q, idx);
}

void buffqueue_destroy(buffqueue_t bq)
{
    osFree(bq);
}
