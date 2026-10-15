/**
 * @file dw_spi_drv.c
 * @brief dw spi driver file.
 *
 * @copyright Copyright (c) 2020  Semidrive Semiconductor.
 *            All rights reserved.
 */

#include <compiler.h>
#include <debug.h>
#include <param.h>
#include <reg.h>
#include <spinlock.h>
#include <string.h>
#include <malloc.h>
#include <board.h>
#include <sd_errcode.h>
#include <arch/cache.h>
#include <driver/irq.h>
#include <driver/scr.h>
#include <driver/spi.h>
#include "driver/kunlun/spi/dw_spi_drv.h"
#include "regs_base.h"
#include "dw_spi.h"
#include "scr_cfg.h"
#include "scr_hw.h"

#define DW_SPI_EVENT (0x1)

#define DW_SPI_ERR_RXO (-1)
#define DW_SPI_ERR_RXU (-2)
#define DW_SPI_ERR_TXO (-4)
#define DW_SPI_ERR_TMIEOUT (-7)
#define DW_SPI_ERR_REQ_MEM (-8)
#define DW_SPI_ERR_REQ_DMA_RX (-9)
#define DW_SPI_ERR_REQ_DMA_TX (-10)
#define DW_SPI_ERR_PRE_DMA_RX (-11)
#define DW_SPI_ERR_PRE_DMA_TX (-12)
#define DW_SPI_ERR_STAT_DMA_RX (-13)
#define DW_SPI_ERR_STAT_DMA_TX (-14)

static void dw_spi_master_init_cfg(struct spi_device *dev)
{
    paddr_t base = dev->ctrl->ctrl_cfg.base;
    struct dw_spi_controller *dw_ctrl = dev->ctrl->priv;

    /* set frame size */
    spi_lld_set_frame_size(base, dev->dev_cfg.bits_per_word);

    /* set clock polarity */
    spi_lld_set_clk_pol(base, !!(dev->dev_cfg.mode & SPI_CPOL));

    /* set clock phase */
    spi_lld_set_clk_phase(base, !!(dev->dev_cfg.mode & SPI_CPHA));

    /* set loop */
    spi_lld_set_srl(base, !!(dev->dev_cfg.mode & SPI_LOOP));

    /* set clk */
    spi_lld_set_clk(base, dw_ctrl->sclk_hz, dev->dev_cfg.freq_hz);

    /* set cs toggle */
    spi_lld_set_sste(base, !!(dev->dev_cfg.mode & SPI_CS_WORD));

    /* mask all irq */
    spi_lld_mask_irq(base, SPI_IMR_ALL_IRQ);
}

static void dw_spi_reset_chip(paddr_t base)
{
    spi_lld_enable(base, false);
    spi_lld_mask_irq(base, SPI_IMR_ALL_IRQ);
    spi_lld_set_ctrl1_odd(base, 0x6);
    spi_lld_enable(base, true);
}

static uint32_t dw_spi_hw_init(paddr_t base)
{
    uint32_t fifo;
    dw_spi_reset_chip(base);

    for (fifo = 1; fifo < 256; fifo++) {
        spi_lld_set_txftl(base, fifo);

        if (fifo != spi_lld_get_txftl(base))
            break;
    }

    spi_lld_set_txftl(base, 0);
    return (fifo == 1) ? 0 : fifo;
}

/* Return the max entries we can fill into tx fifo */
static inline uint32_t tx_max(paddr_t base, struct dw_spi_controller *dw_ctrl)
{
    uint32_t tx_room, rxtx_gap;

    tx_room = dw_ctrl->fifo_len - spi_lld_get_txfl(base);
    rxtx_gap = dw_ctrl->fifo_len - (dw_ctrl->rx_len - dw_ctrl->tx_len);

    return MIN(dw_ctrl->tx_len, MIN(tx_room, rxtx_gap));
}

/* Return the max entries we should read out of rx fifo */
static inline uint32_t rx_max(paddr_t base, struct dw_spi_controller *dw_ctrl)
{
    return MIN(dw_ctrl->rx_len, spi_lld_get_rxfl(base));
}

static void dw_writer(paddr_t base, struct dw_spi_controller *dw_ctrl)
{
    uint32_t max = tx_max(base, dw_ctrl);
    uint32_t txw = 0;

    while (max--) {
        if (dw_ctrl->tx_buf) {
            if (dw_ctrl->n_bytes == 1)
                txw = *(uint8_t *)(dw_ctrl->tx_buf);
            else if (dw_ctrl->n_bytes == 2)
                txw = *(uint16_t *)(dw_ctrl->tx_buf);
            else
                txw = *(uint32_t *)(dw_ctrl->tx_buf);

            dw_ctrl->tx_buf += dw_ctrl->n_bytes;
        }

        spi_lld_write_data(base, txw);
        --dw_ctrl->tx_len;
    }
}

static void dw_reader(paddr_t base, struct dw_spi_controller *dw_ctrl)
{
    uint32_t max = rx_max(base, dw_ctrl);
    uint32_t rxw = 0;

    while (max--) {
        rxw = spi_lld_read_data(base);

        if (dw_ctrl->rx_buf) {
            if (dw_ctrl->n_bytes == 1)
                *(uint8_t *)(dw_ctrl->rx_buf) = rxw;
            else if (dw_ctrl->n_bytes == 2)
                *(uint16_t *)(dw_ctrl->rx_buf) = rxw;
            else
                *(uint32_t *)(dw_ctrl->rx_buf) = rxw;

            dw_ctrl->rx_buf += dw_ctrl->n_bytes;
        }

        --dw_ctrl->rx_len;
    }
}

static int dw_spi_check_status(paddr_t base, bool raw)
{
    uint32_t irq_status;
    int ret = 0;

    if (raw)
        irq_status = spi_lld_get_risr(base);
    else
        irq_status = spi_lld_get_isr(base);

    if (irq_status & (1u << SPI_ISR_RXOIS_FIELD_OFFSET))
        ret += DW_SPI_ERR_RXO;

    if (irq_status & (1u << SPI_ISR_RXUIS_FIELD_OFFSET))
        ret += DW_SPI_ERR_RXU;

    if (irq_status & (1u << SPI_ISR_TXOIS_FIELD_OFFSET))
        ret += DW_SPI_ERR_TXO;

    if (ret)
        dw_spi_reset_chip(base);

    return ret;
}

static int dw_spi_poll_transfer(struct dw_spi_controller *dw_ctrl)
{
    int ret = 0;
    struct spi_controller *ctrl = dw_ctrl->ctrl;
    struct spi_msg *msg = dw_ctrl->cur_msg;

    for (int i = 0; i < msg->trans_num; i++) {
        dw_ctrl->buf_len = msg->trans_arr[i].len;
        dw_ctrl->tx_buf = (void *)msg->trans_arr[i].tx_buf;
        dw_ctrl->tx_len = msg->trans_arr[i].len / dw_ctrl->n_bytes;
        dw_ctrl->rx_buf = msg->trans_arr[i].rx_buf;
        dw_ctrl->rx_len = dw_ctrl->tx_len;

        do {
            dw_writer(ctrl->ctrl_cfg.base, dw_ctrl);
            dw_reader(ctrl->ctrl_cfg.base, dw_ctrl);
            ret = dw_spi_check_status(ctrl->ctrl_cfg.base, true);

            if (ret)
                return ret;
        }
        while (dw_ctrl->rx_len);
    }

    return ret;
}

static int dw_spi_master_irq_handler(uint32_t irq, void *arg)
{
    int ret;
    struct dw_spi_controller *dw_ctrl = arg;
    struct spi_controller *ctrl = dw_ctrl->ctrl;

    uint16_t irq_status = spi_lld_get_isr(ctrl->ctrl_cfg.base) & 0x3f;
    if (!irq_status)
        return 0;

    ret = dw_spi_check_status(ctrl->ctrl_cfg.base, false);
    if (ret) {
        dw_ctrl->trans_stat = ret;
        osEventFlagsSet(dw_ctrl->event, DW_SPI_EVENT);
        return 0;
    }

    dw_reader(ctrl->ctrl_cfg.base, dw_ctrl);

    if (!dw_ctrl->rx_len) {
        spi_lld_mask_irq(ctrl->ctrl_cfg.base, SPI_IMR_ALL_IRQ);
        osEventFlagsSet(dw_ctrl->event, DW_SPI_EVENT);
    }
    else if (dw_ctrl->rx_len <= spi_lld_get_rxftl(ctrl->ctrl_cfg.base)) {
        spi_lld_set_rxftl(ctrl->ctrl_cfg.base, dw_ctrl->rx_len - 1);
    }

    if (irq_status & (0x1 << SPI_ISR_TXEIS_FIELD_OFFSET)) {
        dw_writer(ctrl->ctrl_cfg.base, dw_ctrl);

        if (!dw_ctrl->tx_len)
            spi_lld_mask_irq(ctrl->ctrl_cfg.base, SPI_ISR_TXEIS_FIELD_OFFSET);
    }

    return 0;
}

static int dw_spi_intp_transfer(struct dw_spi_controller *dw_ctrl)
{
    uint32_t flags;
    uint16_t level;
    uint8_t mask;

    struct spi_controller *ctrl = dw_ctrl->ctrl;
    struct spi_msg *msg = dw_ctrl->cur_msg;

    for (int i = 0; i < msg->trans_num; i++) {
        dw_ctrl->buf_len = msg->trans_arr[i].len;
        dw_ctrl->tx_buf = (void *)msg->trans_arr[i].tx_buf;
        dw_ctrl->tx_len = msg->trans_arr[i].len / dw_ctrl->n_bytes;
        dw_ctrl->rx_buf = msg->trans_arr[i].rx_buf;
        dw_ctrl->rx_len = dw_ctrl->tx_len;

        level = MIN(dw_ctrl->fifo_len / 2, dw_ctrl->tx_len);
        spi_lld_set_txftl(ctrl->ctrl_cfg.base, level);
        spi_lld_set_rxftl(ctrl->ctrl_cfg.base, level - 1);

        mask = (0x1 << SPI_IMR_RXFIM_FIELD_OFFSET | 0x1 << SPI_IMR_RXOIM_FIELD_OFFSET |
                0x1 << SPI_IMR_RXUIM_FIELD_OFFSET | 0x1 << SPI_IMR_TXOIM_FIELD_OFFSET |
                0x1 << SPI_IMR_TXEIM_FIELD_OFFSET);

        spi_lld_umask_irq(ctrl->ctrl_cfg.base, mask);
        flags = osEventFlagsWait(dw_ctrl->event, DW_SPI_EVENT, osFlagsWaitAll, 3000);
        if (osErrorTimeout == flags)
            dw_ctrl->trans_stat = DW_SPI_ERR_TMIEOUT;

        if (dw_ctrl->trans_stat < 0)
            return dw_ctrl->trans_stat;
    }

    return 0;
}

static void dw_spi_dma_tx_callback(uint32_t status, uint32_t param,
                                   void *context)
{
    struct dw_spi_controller *dw_ctrl = context;
    dw_ctrl->dma_tx_stat = status;

    if (DMA_ERR == dw_ctrl->dma_rx_stat) {
        dw_ctrl->trans_stat = DW_SPI_ERR_STAT_DMA_RX;
        osEventFlagsSet(dw_ctrl->event, DW_SPI_EVENT);
        return;
    }
    else if (DMA_COMPLETED == dw_ctrl->dma_rx_stat) {
        if (DMA_COMPLETED == status) {
            osEventFlagsSet(dw_ctrl->event, DW_SPI_EVENT);
        }
        else if (DMA_ERR == status) {
            dw_ctrl->trans_stat = DW_SPI_ERR_STAT_DMA_TX;
            osEventFlagsSet(dw_ctrl->event, DW_SPI_EVENT);
        }
        else
            ssdk_printf(SSDK_ERR, "%s, %u\n", __func__, status);

        return;
    }

    if ((DMA_COMPLETED != status) && (DMA_ERR == status))
        ssdk_printf(SSDK_ERR, "%s, %u\n", __func__, status);
}

static void dw_spi_dma_rx_callback(uint32_t status, uint32_t param,
                                   void *context)
{
    struct dw_spi_controller *dw_ctrl = context;
    dw_ctrl->dma_rx_stat = status;

    if (DMA_ERR == dw_ctrl->dma_tx_stat) {
        dw_ctrl->trans_stat = DW_SPI_ERR_STAT_DMA_TX;
        osEventFlagsSet(dw_ctrl->event, DW_SPI_EVENT);
        return;
    }
    else if (DMA_COMPLETED == dw_ctrl->dma_tx_stat) {
        if (DMA_COMPLETED == status) {
            osEventFlagsSet(dw_ctrl->event, DW_SPI_EVENT);
        }
        else if (DMA_ERR == status) {
            dw_ctrl->trans_stat = DW_SPI_ERR_STAT_DMA_RX;
            osEventFlagsSet(dw_ctrl->event, DW_SPI_EVENT);
        }
        else
            ssdk_printf(SSDK_ERR, "%s, %u\n", __func__, status);

        return;
    }

    if ((DMA_COMPLETED != status) && (DMA_ERR == status))
        ssdk_printf(SSDK_ERR, "%s, %u\n", __func__, status);
}

static int dw_spi_dma_transfer(struct dw_spi_controller *dw_ctrl)
{
    int ret;
    uint32_t flags;
    struct spi_controller *ctrl = dw_ctrl->ctrl;
    struct spi_msg *msg = dw_ctrl->cur_msg;
    struct dma_dev_cfg spi_tx_cfg = {0};
    struct dma_dev_cfg spi_rx_cfg = {0};
    dma_dev_bus_width_e addr_width;

    spi_lld_set_dmatdl(ctrl->ctrl_cfg.base, 4);
    spi_lld_set_dmardl(ctrl->ctrl_cfg.base, 0);

    dw_ctrl->spi_tx_ch = dma_req_chan(*dw_ctrl->dma_dev);
    if (!dw_ctrl->spi_tx_ch) {
        ssdk_printf(SSDK_ERR, "spi%d, dma_req_chan tx fail\n", ctrl->ctrl_cfg.id);
        return  DW_SPI_ERR_REQ_DMA_TX;
    }

    dw_ctrl->spi_rx_ch = dma_req_chan(*dw_ctrl->dma_dev);
    if (!dw_ctrl->spi_rx_ch) {
        dma_free_chan(dw_ctrl->spi_tx_ch);
        ssdk_printf(SSDK_ERR, "spi%d, dma_req_chan rx fail\n", ctrl->ctrl_cfg.id);
        return  DW_SPI_ERR_REQ_DMA_RX;
    }

    for (int i = 0; i < msg->trans_num; i++) {
        dw_ctrl->buf_len = msg->trans_arr[i].len;
        uint32_t alloc_len = ROUNDUP(dw_ctrl->buf_len, CONFIG_ARCH_CACHE_LINE);

        dw_ctrl->tx_buf = memalign(CONFIG_ARCH_CACHE_LINE, alloc_len);
        if (!dw_ctrl->tx_buf) {
            ssdk_printf(SSDK_ERR, "spi%d, alloc tx mem fail\n", ctrl->ctrl_cfg.id);
            ctrl->trans_stat = DW_SPI_ERR_REQ_MEM;
            goto end1;
        }

        dw_ctrl->rx_buf = memalign(CONFIG_ARCH_CACHE_LINE, alloc_len);
        if (!dw_ctrl->tx_buf) {
            ssdk_printf(SSDK_ERR, "spi%d, alloc rx mem fail\n", ctrl->ctrl_cfg.id);
            ctrl->trans_stat = DW_SPI_ERR_REQ_MEM;
            goto end2;
        }

        memset(dw_ctrl->tx_buf, 0, dw_ctrl->buf_len);
        memset(dw_ctrl->rx_buf, 0, dw_ctrl->buf_len);

        if (msg->trans_arr[i].tx_buf)
            memcpy(dw_ctrl->tx_buf, msg->trans_arr[i].tx_buf, dw_ctrl->buf_len);

        if (dw_ctrl->n_bytes == 1)
            addr_width = DMA_DEV_BUSWIDTH_1_BYTE;
        else if (dw_ctrl->n_bytes == 2)
            addr_width = DMA_DEV_BUSWIDTH_2_BYTES;
        else
            addr_width = DMA_DEV_BUSWIDTH_4_BYTES;

        spi_tx_cfg.direction = DMA_MEM2DEV;
        spi_tx_cfg.dst_addr = (addr_t)(ctrl->ctrl_cfg.base + SPI_DR0_BASE);
        spi_tx_cfg.src_addr = (addr_t)dw_ctrl->tx_buf;
        spi_tx_cfg.src_addr_width = addr_width;
        spi_tx_cfg.dst_addr_width = addr_width;
        spi_tx_cfg.src_maxburst = 1;
        spi_tx_cfg.dst_maxburst = 1;
        dw_ctrl->spi_tx_ch->irq_cb = dw_spi_dma_tx_callback;
        dw_ctrl->spi_tx_ch->context = dw_ctrl;

        spi_rx_cfg.direction = DMA_DEV2MEM;
        spi_rx_cfg.dst_addr = (addr_t)dw_ctrl->rx_buf;
        spi_rx_cfg.src_addr = (addr_t)(ctrl->ctrl_cfg.base + SPI_DR0_BASE);
        spi_rx_cfg.src_addr_width = addr_width;
        spi_rx_cfg.dst_addr_width = addr_width;
        spi_rx_cfg.src_maxburst = 1;
        spi_rx_cfg.dst_maxburst = 1;
        dw_ctrl->spi_rx_ch->irq_cb = dw_spi_dma_rx_callback;
        dw_ctrl->spi_rx_ch->context = dw_ctrl;

        dma_config_chan(dw_ctrl->spi_tx_ch, &spi_tx_cfg);
        dma_config_chan(dw_ctrl->spi_rx_ch, &spi_rx_cfg);

        ret = dma_prep_dev(dw_ctrl->spi_tx_ch, dw_ctrl->tx_buf, dw_ctrl->buf_len,
                           DMA_INTERRUPT);
        if (!ret) {
            ssdk_printf(SSDK_ERR, "spi%d, dma_prep_dev tx fail\n", ctrl->ctrl_cfg.id);
            ctrl->trans_stat = DW_SPI_ERR_PRE_DMA_TX;
            goto end3;
        }

        ret = dma_prep_dev(dw_ctrl->spi_rx_ch, dw_ctrl->rx_buf, dw_ctrl->buf_len,
                           DMA_INTERRUPT);
        if (!ret) {
            ssdk_printf(SSDK_ERR, "spi%d, dma_prep_dev rx fail\n", ctrl->ctrl_cfg.id);
            ctrl->trans_stat = DW_SPI_ERR_PRE_DMA_RX;
            goto end3;
        }

        arch_clean_cache_range((addr_t)dw_ctrl->tx_buf, alloc_len);
        arch_invalidate_cache_range((addr_t)dw_ctrl->rx_buf, alloc_len);

        dw_ctrl->dma_tx_stat = DMA_IN_PROGRESS;
        dw_ctrl->dma_rx_stat = DMA_IN_PROGRESS;
        dma_start(dw_ctrl->spi_tx_ch);
        dma_start(dw_ctrl->spi_rx_ch);
        spi_lld_enable_dma(ctrl->ctrl_cfg.base, true, true);

        flags = osEventFlagsWait(dw_ctrl->event, DW_SPI_EVENT, osFlagsWaitAll, 3000);
        dma_stop(dw_ctrl->spi_tx_ch);
        dma_stop(dw_ctrl->spi_rx_ch);
        spi_lld_enable_dma(ctrl->ctrl_cfg.base, false, false);

        if (osErrorTimeout == flags) {
            ctrl->trans_stat = DW_SPI_ERR_TMIEOUT;
            goto end3;
        }

        if (ctrl->trans_stat)
            goto end3;

        ret = dw_spi_check_status(ctrl->ctrl_cfg.base, true);
        if (ret) {
            ctrl->trans_stat = ret;
            goto end3;
        }

        if (msg->trans_arr[i].rx_buf)
            memcpy(msg->trans_arr[i].rx_buf, dw_ctrl->rx_buf, dw_ctrl->buf_len);

        free(dw_ctrl->tx_buf);
        free(dw_ctrl->rx_buf);
    }

    dma_free_chan(dw_ctrl->spi_tx_ch);
    dma_free_chan(dw_ctrl->spi_rx_ch);
    return 0;

end3:
    free(dw_ctrl->rx_buf);
end2:
    free(dw_ctrl->tx_buf);
end1:
    dma_free_chan(dw_ctrl->spi_tx_ch);
    dma_free_chan(dw_ctrl->spi_rx_ch);
    return ctrl->trans_stat;
}

static int dw_spi_transfer(struct spi_controller *ctrl, struct spi_msg *msg)
{
    int ret;
    struct dw_spi_controller *dw_ctrl = ctrl->priv;
    dw_ctrl->cur_msg = msg;
    dw_ctrl->trans_stat = 0;
    dw_ctrl->n_bytes = DIV_ROUND_UP(msg->dev->dev_cfg.bits_per_word, 8);

    spi_lld_enable(ctrl->ctrl_cfg.base, false);
    dw_spi_master_init_cfg(msg->dev);

    if (msg->dev->dev_cfg.cs_gpio_enable)
        gpio_set(dw_ctrl->gpio_dev, msg->dev->dev_cfg.cs_gpio, 0);

    spi_lld_set_cs(ctrl->ctrl_cfg.base, true, msg->dev->dev_cfg.chip_select);
    spi_lld_enable(ctrl->ctrl_cfg.base, true);

    if ((osKernelGetState() < osKernelRunning)
            || (ctrl->ctrl_cfg.opmode == SPI_POLL)) {
        ret = dw_spi_poll_transfer(dw_ctrl);
    }
    else if (ctrl->ctrl_cfg.opmode == SPI_DMA) {
        ret = dw_spi_dma_transfer(dw_ctrl);
    }
    else {
        irq_enable(ctrl->ctrl_cfg.irq);
        ret = dw_spi_intp_transfer(dw_ctrl);
        irq_disable(ctrl->ctrl_cfg.irq);
    }

    spi_lld_enable(ctrl->ctrl_cfg.base, false);

    if (msg->dev->dev_cfg.cs_gpio_enable)
        gpio_set(dw_ctrl->gpio_dev, msg->dev->dev_cfg.cs_gpio, 1);

    spi_lld_set_cs(ctrl->ctrl_cfg.base, false, msg->dev->dev_cfg.chip_select);

    return ret;
}

static int dw_spi_master_init(struct spi_controller *ctrl)
{
    struct dw_spi_controller *dw_ctrl = ctrl->priv;

    dw_ctrl->sclk_hz = clk_get_rate(dw_ctrl->clk);
    dw_ctrl->fifo_len = dw_spi_hw_init(ctrl->ctrl_cfg.base);
    dw_ctrl->event = osEventFlagsNew(NULL);
    ctrl->max_freq_hz = dw_ctrl->sclk_hz / 4;
    ctrl->min_freq_hz = DIV_ROUND_UP(dw_ctrl->sclk_hz, 65534);
    ctrl->bits_per_word_mask = SPI_BPW_RANGE_MASK(4, 32);
    irq_attach(ctrl->ctrl_cfg.irq, dw_spi_master_irq_handler, (void *)dw_ctrl);
    return 0;
}

static int dw_spi_slave_transfer(struct spi_controller *ctrl,
                                 struct spi_msg *msg)
{
    int cnt = 0;
    struct dw_spi_controller *dw_ctrl = ctrl->priv;
    dw_ctrl->trans_stat = 0;

    memset(dw_ctrl->tx_buf, 0, msg->trans_arr->len);
    memset(dw_ctrl->rx_buf, 0, msg->trans_arr->len);

    if (msg->trans_arr->tx_buf)
        memcpy(dw_ctrl->tx_buf, msg->trans_arr->tx_buf, msg->trans_arr->len);

    arch_clean_cache_range((addr_t)dw_ctrl->tx_buf, dw_ctrl->buf_len);
    arch_invalidate_cache_range((addr_t)dw_ctrl->rx_buf, dw_ctrl->buf_len);
    spi_lld_enable(ctrl->ctrl_cfg.base, true);

    while (spi_lld_is_busy(ctrl->ctrl_cfg.base)) {
        cnt++;
        udelay(1);

        if (cnt > 100000000) {
            dw_ctrl->trans_stat = DW_SPI_ERR_TMIEOUT;
            return dw_ctrl->trans_stat;
        }
    }

    spi_lld_enable_dma(ctrl->ctrl_cfg.base, true, true);
    dw_ctrl->dma_tx_stat = DMA_IN_PROGRESS;
    dw_ctrl->dma_rx_stat = DMA_IN_PROGRESS;
    dma_start(dw_ctrl->spi_tx_ch);
    dma_start(dw_ctrl->spi_rx_ch);
    osEventFlagsWait(dw_ctrl->event, DW_SPI_EVENT, osFlagsWaitAll, osWaitForever);
    dma_stop(dw_ctrl->spi_tx_ch);
    dma_stop(dw_ctrl->spi_rx_ch);
    spi_lld_enable_dma(ctrl->ctrl_cfg.base, false, false);
    spi_lld_enable(ctrl->ctrl_cfg.base, false);

    if (msg->trans_arr->rx_buf)
        memcpy(msg->trans_arr->rx_buf, dw_ctrl->rx_buf, msg->trans_arr->len);

    if (dw_ctrl->trans_stat)
        return dw_ctrl->trans_stat;

    return dw_spi_check_status(ctrl->ctrl_cfg.base, true);
}

static int dw_spi_slave_irq_handler(uint32_t irq, void *arg)
{
    return 0;
}

static int dw_spi_slave_init(struct spi_controller *ctrl)
{
    int ret;
    struct dw_spi_controller *dw_ctrl = ctrl->priv;
    paddr_t base = ctrl->ctrl_cfg.base;
    struct dma_dev_cfg spi_tx_cfg = {0};
    struct dma_dev_cfg spi_rx_cfg = {0};
    dw_ctrl->event = osEventFlagsNew(NULL);
    dw_ctrl->buf_len = ROUNDUP(ctrl->ctrl_cfg.slave_buf_len, CONFIG_ARCH_CACHE_LINE);

    spi_lld_enable(base, false);
    spi_lld_set_sste(base, false);
    spi_lld_set_frame_size(base, ctrl->ctrl_cfg.slave_bits_per_word);
    spi_lld_set_clk_pol(base, 0);
    spi_lld_set_clk_phase(base, 0);
    spi_lld_set_dmatdl(base, 4);
    spi_lld_set_dmardl(base, 0);
    spi_lld_mask_irq(base, SPI_IMR_ALL_IRQ);
    irq_attach(ctrl->ctrl_cfg.irq, dw_spi_slave_irq_handler, (void *)dw_ctrl);

    dw_ctrl->spi_tx_ch = dma_req_chan(*dw_ctrl->dma_dev);
    if (!dw_ctrl->spi_tx_ch) {
        ssdk_printf(SSDK_ERR, "spi%d, slv dma_req_chan tx fail\n", ctrl->ctrl_cfg.id);
        return  DW_SPI_ERR_REQ_DMA_TX;
    }

    dw_ctrl->spi_rx_ch = dma_req_chan(*dw_ctrl->dma_dev);
    if (!dw_ctrl->spi_rx_ch) {
        dma_free_chan(dw_ctrl->spi_tx_ch);
        ssdk_printf(SSDK_ERR, "spi%d, slv dma_req_chan rx fail\n", ctrl->ctrl_cfg.id);
        return  DW_SPI_ERR_REQ_DMA_RX;
    }

    dw_ctrl->tx_buf = memalign(CONFIG_ARCH_CACHE_LINE, dw_ctrl->buf_len);
    if (!dw_ctrl->tx_buf) {
        dma_free_chan(dw_ctrl->spi_tx_ch);
        dma_free_chan(dw_ctrl->spi_rx_ch);
        ssdk_printf(SSDK_ERR, "spi%d, slv alloc tx mem fail\n", ctrl->ctrl_cfg.id);
        return DW_SPI_ERR_REQ_MEM;
    }

    dw_ctrl->rx_buf = memalign(CONFIG_ARCH_CACHE_LINE, dw_ctrl->buf_len);
    if (!dw_ctrl->rx_buf) {
        dma_free_chan(dw_ctrl->spi_tx_ch);
        dma_free_chan(dw_ctrl->spi_rx_ch);
        free(dw_ctrl->tx_buf);
        ssdk_printf(SSDK_ERR, "spi%d, slv alloc rx mem fail\n", ctrl->ctrl_cfg.id);
        return DW_SPI_ERR_REQ_MEM;
    }

    memset(dw_ctrl->tx_buf, 0, dw_ctrl->buf_len);
    memset(dw_ctrl->rx_buf, 0, dw_ctrl->buf_len);

    spi_tx_cfg.direction = DMA_MEM2DEV;
    spi_tx_cfg.dst_addr = (addr_t)(ctrl->ctrl_cfg.base + SPI_DR0_BASE);
    spi_tx_cfg.src_addr = (addr_t)dw_ctrl->tx_buf;
    spi_tx_cfg.src_addr_width = DMA_DEV_BUSWIDTH_1_BYTE;
    spi_tx_cfg.dst_addr_width = DMA_DEV_BUSWIDTH_1_BYTE;
    spi_tx_cfg.src_maxburst = 1;
    spi_tx_cfg.dst_maxburst = 1;
    dw_ctrl->spi_tx_ch->irq_cb = dw_spi_dma_tx_callback;
    dw_ctrl->spi_tx_ch->context = dw_ctrl;

    spi_rx_cfg.direction = DMA_DEV2MEM;
    spi_rx_cfg.dst_addr = (addr_t)dw_ctrl->rx_buf;
    spi_rx_cfg.src_addr = (addr_t)(ctrl->ctrl_cfg.base + SPI_DR0_BASE);
    spi_rx_cfg.src_addr_width = DMA_DEV_BUSWIDTH_1_BYTE;
    spi_rx_cfg.dst_addr_width = DMA_DEV_BUSWIDTH_1_BYTE;
    spi_rx_cfg.src_maxburst = 1;
    spi_rx_cfg.dst_maxburst = 1;
    dw_ctrl->spi_rx_ch->irq_cb = dw_spi_dma_rx_callback;
    dw_ctrl->spi_rx_ch->context = dw_ctrl;

    dma_config_chan(dw_ctrl->spi_tx_ch, &spi_tx_cfg);
    dma_config_chan(dw_ctrl->spi_rx_ch, &spi_rx_cfg);

    ret = dma_prep_dev(dw_ctrl->spi_tx_ch, dw_ctrl->tx_buf,
                       ctrl->ctrl_cfg.slave_buf_len, DMA_INTERRUPT);
    if (!ret) {
        dma_free_chan(dw_ctrl->spi_tx_ch);
        dma_free_chan(dw_ctrl->spi_rx_ch);
        free(dw_ctrl->tx_buf);
        free(dw_ctrl->rx_buf);
        ssdk_printf(SSDK_ERR, "spi%d, slv dma_prep_dev tx fail\n", ctrl->ctrl_cfg.id);
        return DW_SPI_ERR_PRE_DMA_TX;
    }

    ret = dma_prep_dev(dw_ctrl->spi_rx_ch, dw_ctrl->rx_buf,
                       ctrl->ctrl_cfg.slave_buf_len, DMA_INTERRUPT);
    if (!ret) {
        dma_free_chan(dw_ctrl->spi_tx_ch);
        dma_free_chan(dw_ctrl->spi_rx_ch);
        free(dw_ctrl->tx_buf);
        free(dw_ctrl->rx_buf);
        ssdk_printf(SSDK_ERR, "spi%d, slv dma_prep_dev rx fail\n", ctrl->ctrl_cfg.id);
        return DW_SPI_ERR_PRE_DMA_RX;
    }

    return 0;
}

static int dw_spi_init(struct spi_controller *ctrl)
{
    int ret;
    struct dw_spi_controller *dw_ctrl = ctrl->priv;

    ASSERT(ctrl);

    if (ctrl->ctrl_cfg.slave)
        scr_set(dw_ctrl->scr_dev, &dw_ctrl->scr_sig, 1);
    else
        scr_set(dw_ctrl->scr_dev, &dw_ctrl->scr_sig, 0);

    if (dw_ctrl->clk)
        clk_enable(dw_ctrl->clk);

    if (ctrl->ctrl_cfg.slave)
        ret = dw_spi_slave_init(ctrl);
    else
        ret = dw_spi_master_init(ctrl);

    if (ret && dw_ctrl->clk)
        clk_disable(dw_ctrl->clk);

    return ret;
}

static void dw_spi_deinit(struct spi_controller *ctrl)
{
    ASSERT(ctrl);
    struct dw_spi_controller *dw_ctrl = ctrl->priv;

    if (dw_ctrl->clk)
        clk_disable(dw_ctrl->clk);

    irq_disable(ctrl->ctrl_cfg.irq);
    irq_detach(ctrl->ctrl_cfg.irq);
}

static struct spi_controller_ops dw_spi_ops = {
    .spi_init = dw_spi_init,
    .spi_deinit = dw_spi_deinit,
    .spi_transfer = dw_spi_transfer,
    .spi_slv_transfer = dw_spi_slave_transfer,
};

int dw_spi_controller_probe(struct spi_controller *ctrl,
                            struct dw_spi_controller *dw_ctrl, const struct spi_controller_config *cfg)
{
    ASSERT(ctrl && dw_ctrl && cfg);

    memset(ctrl, 0, sizeof(struct spi_controller));

    dw_ctrl->ctrl = ctrl;
    ctrl->priv = dw_ctrl;
    ctrl->ops = &dw_spi_ops;

    return spi_controller_init(ctrl, cfg);
}

