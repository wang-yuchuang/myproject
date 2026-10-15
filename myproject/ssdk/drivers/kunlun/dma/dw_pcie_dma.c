/**
 * @file dw_pcie_dma.c
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: dwc pcie dma high level driver.
 *
 * Revision History:
 * -----------------
 */

#include <reg.h>
#include <debug.h>
#include <cmsis_os2.h>
#include <spinlock.h>
#include "driver/irq.h"
#include "driver/kunlun/dma/dw_pcie_dma.h"

#define DMA_CAP_BASE            0xC8000

#define DMA_READ_ENGINE_EN_OFF  (DMA_CAP_BASE + 0x2C)
#define DMA_READ_INT_MASK_OFF   (DMA_CAP_BASE + 0xA8)
#define DMA_CH_CONTROL1_OFF_RDCH(n) (DMA_CAP_BASE + (n) * 0x200 + 0x300)
#define DMA_TRANSFER_SIZE_OFF_RDCH(n)   (DMA_CAP_BASE + (n) * 0x200 + 0x308)
#define DMA_SAR_LOW_OFF_RDCH(n) (DMA_CAP_BASE + (n) * 0x200 + 0x30C)
#define DMA_SAR_HIGH_OFF_RDCH(n) (DMA_CAP_BASE + (n) * 0x200 + 0x310)
#define DMA_DAR_LOW_OFF_RDCH(n) (DMA_CAP_BASE + (n) * 0x200 + 0x314)
#define DMA_DAR_HIGH_OFF_RDCH(n) (DMA_CAP_BASE + (n) * 0x200 + 0x318)
#define DMA_READ_DOORBELL_OFF   (DMA_CAP_BASE + 0x30)
#define DMA_READ_INT_STATUS_OFF (DMA_CAP_BASE + 0xA0)
#define DMA_READ_INT_CLEAR_OFF  (DMA_CAP_BASE + 0xAC)

#define DMA_WRITE_ENGINE_EN_OFF  (DMA_CAP_BASE + 0x0C)
#define DMA_WRITE_INT_MASK_OFF   (DMA_CAP_BASE + 0x54)
#define DMA_CH_CONTROL1_OFF_WRCH(n) (DMA_CAP_BASE + (n) * 0x200 + 0x200)
#define DMA_TRANSFER_SIZE_OFF_WRCH(n)   (DMA_CAP_BASE + (n) * 0x200 + 0x208)
#define DMA_SAR_LOW_OFF_WRCH(n) (DMA_CAP_BASE + (n) * 0x200 + 0x20C)
#define DMA_SAR_HIGH_OFF_WRCH(n) (DMA_CAP_BASE + (n) * 0x200 + 0x210)
#define DMA_DAR_LOW_OFF_WRCH(n) (DMA_CAP_BASE + (n) * 0x200 + 0x214)
#define DMA_DAR_HIGH_OFF_WRCH(n) (DMA_CAP_BASE + (n) * 0x200 + 0x218)
#define DMA_WRITE_DOORBELL_OFF   (DMA_CAP_BASE + 0x10)
#define DMA_WRITE_INT_STATUS_OFF    (DMA_CAP_BASE + 0x4C)
#define DMA_WRITE_INT_CLEAR_OFF (DMA_CAP_BASE + 0x58)

/**
 * @brief Request free channel.
 * Among all the availiable channels, the first half channels are
 * used for memory reading, and the second half channels are used
 * for memory writing.
 */
static dma_chan_t *dw_pcie_dma_chan_req(dma_dev_t *dev, uint32_t *id)
{
    dma_chan_t *cptr = NULL;

    list_for_every_entry (&dev->channels, cptr, dma_chan_t, node) {

        if (0 == cptr->allocated) {
            if ((NULL == id) || (*id == cptr->chan_id)) {
                cptr->allocated = 1;
                return cptr;
            }
        }
    }

    return NULL;
}

static void dw_pcie_dma_chan_free(dma_chan_t *chan)
{
    chan->allocated = 0;
}

static bool dw_pcie_dma_prep_memrd(dma_chan_t *chan, void *dst,
                                   void *src, size_t buf_len,
                                   uint32_t flags)
{
    paddr_t base = chan->device->dma_base;
    uint32_t chn_id = chan->chan_id;

    writel(1, base + DMA_READ_ENGINE_EN_OFF);
    uint32_t val = readl(base + DMA_READ_INT_MASK_OFF);
    /* Each channel supports abort and done interrupt. */
    if (flags & DMA_INTERRUPT)
        val &= ~(0x10001 << chn_id);
    else
        val |= 0x10001 << chn_id;
    writel(val, base + DMA_READ_INT_MASK_OFF);
    writel(0x04000008, base + DMA_CH_CONTROL1_OFF_RDCH(chn_id));
    writel(buf_len, base + DMA_TRANSFER_SIZE_OFF_RDCH(chn_id));
    writel((uint32_t)src, base + DMA_SAR_LOW_OFF_RDCH(chn_id));
    writel(0, base + DMA_SAR_HIGH_OFF_RDCH(chn_id));
    writel((uint32_t)dst, base + DMA_DAR_LOW_OFF_RDCH(chn_id));
    writel(0, base + DMA_DAR_HIGH_OFF_RDCH(chn_id));

    return true;
}

static bool dw_pcie_dma_prep_memwr(dma_chan_t *chan, void *dst,
                                   void *src, size_t buf_len,
                                   uint32_t flags)
{
    dma_dev_t *dev = chan->device;
    paddr_t base = dev->dma_base;
    uint32_t chn_id = chan->chan_id - dev->channel_nr / 2;

    writel(1, base + DMA_WRITE_ENGINE_EN_OFF);
    uint32_t val = readl(base + DMA_READ_INT_MASK_OFF);
    /* Each channel supports abort and done interrupt. */
    if (flags & DMA_INTERRUPT)
        val &= ~(10001 << chn_id);
    else
        val |= 0x10001 << chn_id;
    writel(val, base + DMA_WRITE_INT_MASK_OFF);
    writel(0x04000008, base + DMA_CH_CONTROL1_OFF_WRCH(chn_id));
    writel(buf_len, base + DMA_TRANSFER_SIZE_OFF_WRCH(chn_id));
    writel((uint32_t)src, base + DMA_SAR_LOW_OFF_WRCH(chn_id));
    writel(0, base + DMA_SAR_HIGH_OFF_WRCH(chn_id));
    writel((uint32_t)dst, base + DMA_DAR_LOW_OFF_WRCH(chn_id));
    writel(0, base + DMA_DAR_HIGH_OFF_WRCH(chn_id));

    return true;
}

/**
 * @brief For memory read, one of EP BARs must be configured to
 * match src address. For memory write, one of EP BARs must be
 * configured to match dst address.
 */
static bool dw_pcie_dma_prep_memcpy(dma_chan_t *chan, void *dst,
                                    void *src, size_t buf_len,
                                    uint32_t flags)
{
    if (chan->chan_id < chan->device->channel_nr / 2)
        return dw_pcie_dma_prep_memrd(chan, dst, src, buf_len, flags);
    else
        return dw_pcie_dma_prep_memwr(chan, dst, src, buf_len, flags);
}

static void dw_pcie_dma_ch_enable(dma_chan_t *chan)
{
    uint32_t chn_id = chan->chan_id;
    dma_dev_t *dev = chan->device;
    irq_state_t state;

    state = spin_lock_irqsave(&chan->chan_lock);

    if (chn_id < dev->channel_nr / 2)
        writel(chn_id, dev->dma_base + DMA_READ_DOORBELL_OFF);
    else {
        chn_id -= dev->channel_nr / 2;
        writel(chn_id, dev->dma_base + DMA_WRITE_DOORBELL_OFF);
    }

    spin_unlock_irqrestore(&chan->chan_lock, state);
}

static bool dw_pcie_dma_ch_disable(dma_chan_t *chan)
{
    uint32_t chn_id = chan->chan_id;
    dma_dev_t *dev = chan->device;
    irq_state_t state;

    state = spin_lock_irqsave(&chan->chan_lock);

    if (chn_id < dev->channel_nr / 2) {
        uint32_t val = readl(dev->dma_base + DMA_CH_CONTROL1_OFF_RDCH(chn_id));
        /* Channel status must be running before stop it. */
        if (((val >> 5) & 3) == 1)
            writel((1 << 31) | chn_id, dev->dma_base + DMA_READ_DOORBELL_OFF);
    }
    else {
        chn_id -= dev->channel_nr / 2;
        uint32_t val = readl(dev->dma_base + DMA_CH_CONTROL1_OFF_WRCH(chn_id));
        /* Channel status must be running before stop it. */
        if (((val >> 5) & 3) == 1)
            writel((1 << 31) | chn_id, dev->dma_base + DMA_WRITE_DOORBELL_OFF);
    }

    spin_unlock_irqrestore(&chan->chan_lock, state);

    return true;
}

static dma_status_e dw_pcie_dma_get_chan_status(dma_chan_t *chan)
{
    uint32_t chn_id = chan->chan_id;
    dma_dev_t *dev = chan->device;

    if (chn_id < dev->channel_nr / 2) {
        uint32_t val = readl(dev->dma_base + DMA_READ_INT_STATUS_OFF);
        if (val & (1 << chn_id))
            return DMA_COMPLETED;
        else
            return DMA_IN_PROGRESS;
    }
    else {
        chn_id -= dev->channel_nr / 2;
        uint32_t val = readl(dev->dma_base + DMA_WRITE_INT_STATUS_OFF);
        if (val & (1 << chn_id))
            return DMA_COMPLETED;
        else
            return DMA_IN_PROGRESS;
    }
}

static int dw_pcie_dma_pre_init(dma_dev_t *dev)
{
    paddr_t base = dev->dma_base;

    /* Disable and clear all interrupts. */
    writel(~0, base + DMA_READ_INT_MASK_OFF);
    writel(~0, base + DMA_READ_INT_CLEAR_OFF);
    writel(~0, base + DMA_WRITE_INT_MASK_OFF);
    writel(~0, base + DMA_WRITE_INT_CLEAR_OFF);

    return 0;
}

static dma_ops_t g_pcie_dma_ops = {
    .init = dw_pcie_dma_pre_init,
    .request_chan = dw_pcie_dma_chan_req,
    .release_chan = dw_pcie_dma_chan_free,
    .prep_memcpy = dw_pcie_dma_prep_memcpy,
    .enable_chan = dw_pcie_dma_ch_enable,
    .disable_chan = dw_pcie_dma_ch_disable,
};

static inline uint32_t get_rd_chan_int_status(paddr_t base,
                                              uint32_t channel)
{
    return readl(base + DMA_READ_INT_STATUS_OFF) & (0x10001 << channel);
}

static inline void clear_rd_chan_int_status(paddr_t base,
                                            uint32_t channel)
{
    writel(0x10001 << channel, base + DMA_READ_INT_CLEAR_OFF);
}

static inline uint32_t get_wr_chan_int_status(paddr_t base,
                                              uint32_t channel)
{
    return readl(base + DMA_WRITE_INT_STATUS_OFF) & (0x10001 << channel);
}

static inline void clear_wr_chan_int_status(paddr_t base,
                                            uint32_t channel)
{
    writel(0x10001 << channel, base + DMA_WRITE_INT_CLEAR_OFF);
}

static int dw_pcie_dma_irq_handler(uint32_t irq, void *arg)
{
    dma_chan_t *chan = arg;
    paddr_t base = chan->device->dma_base;
    uint32_t rd_int_status = get_rd_chan_int_status(base, chan->chan_id);
    uint32_t wr_int_status = get_wr_chan_int_status(base, chan->chan_id);

    ssdk_printf(SSDK_INFO, "%s: dma 0x%x channel %d, rd int status 0x%x,"
                "wr int status 0x%x\n", __func__, base, chan->chan_id,
                rd_int_status, wr_int_status);

    if (rd_int_status) {
        clear_rd_chan_int_status(base, chan->chan_id);
        if (chan->irq_cb)
            chan->irq_cb(DMA_COMPLETED, rd_int_status, chan->context);
    }

    if (wr_int_status) {
        clear_wr_chan_int_status(base, chan->chan_id);
        /* The distance of each pair read & write channels is half of
         * channel number of this DMA controller.
         */
        dma_chan_t *wr_chan = chan + chan->device->channel_nr / 2;
        if (wr_chan->irq_cb)
            wr_chan->irq_cb(DMA_COMPLETED, wr_int_status, wr_chan->context);
    }

    return 0;
}

void dw_pcie_dma_init(dma_dev_t *dev)
{
    ASSERT(dev);

    dev->ops = &g_pcie_dma_ops;

    dma_init(dev);

    dma_chan_t *chan = osAlloc(sizeof(dma_chan_t) * dev->channel_nr);
    ASSERT(chan);

    for (int i = 0; i < dev->channel_nr; i++) {
        dma_chan_t *chn = &chan[i];

        spin_lock_init(&chn->chan_lock);

        chn->chan_id = i;
        chn->allocated = 0;
        chn->device = dev;
        list_add_tail(&dev->channels, &chn->node);

        /* Each pair of read & write channels share the same interrupt line. */
        if (i < dev->channel_nr / 2) {
            irq_attach(dev->irq_id + i, dw_pcie_dma_irq_handler, chn);
            irq_enable(dev->irq_id + i);
        }
    }
}
