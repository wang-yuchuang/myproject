/**
 * dwc_eth.c
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: dw eth mac drv
 *
 * Revision History:
 * -----------------
 */

#include <string.h>
#include "debug.h"
#include "param.h"
#include "arch/barriers.h"
#include "arch/cache.h"
#include "irq_num.h"
#include "board/board.h"
#include "driver/irq.h"
#include "dw_mac_lld.h"
#include "driver/taishan/eth/dwc_eth.h"
#include "driver/taishan/eth/phy.h"

#ifdef CONFIG_MEMMAP
#include "driver/misc/memmap.h"
#else
#define p2ap(a) (a)
#endif

static void *eqos_alloc_descs(struct eqos_priv *eqos, unsigned int num)
{
    /* Align dma descriptors base address & size to cache line to avoid
     * descriptors corrupted by cacahe line invalidation needed by other
     * address, if descriptors has not been wrote to ram.
     */
    eqos->desc_size = ALIGN(sizeof(struct eqos_desc),
                            (unsigned int)ARCH_DMA_MINALIGN);

    return (void *)osAllocAlign(eqos->desc_size, num * eqos->desc_size);
}

static void eqos_free_descs(void *descs)
{
    osFree(descs);
}

static struct eqos_desc *eqos_get_desc(struct eqos_priv *eqos, unsigned int num,
                                       bool rx)
{
    void *desc = (uint8_t *)eqos->descs +
                 ((rx ? EQOS_DESCRIPTORS_TX : 0) + num) * eqos->desc_size;
    return desc;
}

/**
* @brief Init the Controller DMA Block
*/
static inline void mac_init_dma(uint32_t regbase, struct eqos_priv *eqos)
{
    uint8_t ch_cnt = 0U;

    dwmac_dma_bus_init(regbase, DMA_AXI_MAX_OSR_LIMIT | DMA_AXI_BLEN256);

    for (ch_cnt = 0; ch_cnt < ETH_MAX_DMA_CHANNEL; ch_cnt++) {
        // DMA Control
        dwmac_dma_init_channel(regbase, 0, ch_cnt, eqos->dsl);
        // rx ring length
        dwmac_set_rx_ring_len(regbase, EQOS_DESCRIPTORS_RX - 1, ch_cnt);
        // init_rx_chan
        dwmac_dma_init_rx_chan(regbase, p2ap((paddr_t)eqos_get_desc(eqos, 0, true)),
                               EQOS_MAX_PACKET_SIZE, ch_cnt);
        // rx bufsize
        dwmac_set_bfsize(regbase, 2048, ch_cnt);
        // tx ring length
        dwmac_set_tx_ring_len(regbase, EQOS_DESCRIPTORS_TX - 1, ch_cnt);
        // init_tx_chan
        dwmac_dma_init_tx_chan(regbase, p2ap((paddr_t)eqos_get_desc(eqos, 0, false)),
                               ch_cnt);
        // set tx tail ptr
        dwmac_set_tx_tail_ptr(regbase, (uint32_t)eqos_get_desc(eqos, eqos->tx_desc_idx, false),
                               ch_cnt);
        // set rx tail ptr
        dwmac_set_rx_tail_ptr(regbase, p2ap((paddr_t)eqos_get_desc(eqos,
                                            EQOS_DESCRIPTORS_RX - 1, true)), ch_cnt);

        dwmac_enable_dma_irq(regbase, ch_cnt, false, false);
    }
}

/**
* @brief Init the Controller MTL Block
*/
static inline void mac_init_mtl(uint32_t regbase)
{
    uint8_t ch_cnt = 0U;

    dwmac_prog_mtl_rx_algorithms(regbase, MTL_RX_ALGORITHM_SP);

    dwmac_prog_mtl_tx_algorithms(regbase, MTL_OPERATION_SCHALG_WRR);

    for (ch_cnt = 0; ch_cnt < ETH_MAX_DMA_CHANNEL; ch_cnt++) {
        // rx mtl map with dma
        dwmac_map_mtl_dma(regbase, ch_cnt, ch_cnt);
        dwmac_rx_queue_enable(regbase, MTL_QUEUE_MODE_DCB, ch_cnt);
        dwmac_dma_tx_chan_op_mode(regbase, ch_cnt, SF_MODE, 4096, MTL_QUEUE_MODE_DCB);
        dwmac_dma_rx_chan_op_mode(regbase, ch_cnt, SF_MODE, 2048, MTL_QUEUE_MODE_DCB);
    }
}

static inline void mac_start(uint32_t regbase, bool start)
{
    uint8_t ch_cnt = 0U;

    for (ch_cnt = 0; ch_cnt < ETH_MAX_DMA_CHANNEL; ch_cnt++) {
        if (start) {
            // start rx
            dwmac_dma_start_rx(regbase, ch_cnt);
            // start tx
            dwmac_dma_start_tx(regbase, ch_cnt);
        }
        else {
            // stop rx
            dwmac_dma_stop_rx(regbase, ch_cnt);
            // stop tx
            dwmac_dma_stop_tx(regbase, ch_cnt);
        }
    }
}

static void eth_mac_init(dwc_eth_config_t *cfg, struct eqos_priv *eqos)
{
    //DMA core sw reset
    dwmac_dma_reset(cfg->base);

    uint32_t skip_len = eqos->desc_size - sizeof(struct eqos_desc);
    uint8_t bus_width = cfg->dma_bus_width ? cfg->dma_bus_width : 64;
    eqos->dsl = skip_len * 8 / bus_width;

    //DMA init
    mac_init_dma(cfg->base, eqos);

    //MTL init
    mac_init_mtl(cfg->base);

    //core init
    phy_speed_t speed = (cfg->phy[0].phy_addr == ~0) ?
                        PHY_SPEED_1000 : cfg->phy[0].speed;
    dwmac_core_init(cfg->base, cfg->mtu, speed, cfg->mac_addr);
    dwmac_mac_rx_queue_mcbc_routing(cfg->base, 0);
}

static inline void set_phyif_mode(dwc_eth_config_t *cfg)
{
    if (cfg->set_phy_intf)
        cfg->set_phy_intf(cfg->base, cfg->phy_intf_mode);
}

static inline void config_eth_clk(dwc_eth_config_t *cfg)
{
    if (cfg->timer_sec_clk)
        clk_prepare(cfg->timer_sec_clk);
    if (cfg->phy_ref_clk)
        clk_prepare(cfg->phy_ref_clk);
    if (cfg->rmii_clk)
        clk_prepare(cfg->rmii_clk);
    if (cfg->tx_clk)
        clk_prepare(cfg->tx_clk);
}

static inline void reset_eth(dwc_eth_config_t *cfg)
{
    if (cfg->rst)
        reset_ctl_reset(cfg->rst);
}

static err_t dwc_eth_init(struct net_driver_s *dev)
{
    int ret = 0;
    int i;
    dwc_eth_dev_t *eth_dev = (dwc_eth_dev_t *)dev->d_private;
    dwc_eth_config_t *cfg = eth_dev->config;
    struct eqos_priv *eqos;

    if (eth_dev->init)
        return 0;
    else
        eth_dev->init = true;

    //set SCR ethernet operating mode, e.g. RGMII, SGMII, etc.
    set_phyif_mode(cfg);

    //set ethernet Clk
    config_eth_clk(cfg);

    //ethernet reset
    reset_eth(cfg);

    eqos = osAlloc(sizeof(struct eqos_priv));
    ASSERT(eqos);
    memset(eqos, 0, sizeof(struct eqos_priv));

    eqos->rx_lock = osMutexNew(NULL);
    ASSERT(eqos->rx_lock);
    eqos->tx_lock = osMutexNew(NULL);
    ASSERT(eqos->tx_lock);

    eqos->descs = eqos_alloc_descs(eqos, EQOS_DESCRIPTORS_NUM);

    if (!eqos->descs) {
        ssdk_printf(SSDK_INFO, "%s: eqos_alloc_descs() failed\n", __func__);
        ret = -1;
    }

    eqos->tx_desc_idx = 0;
    eqos->rx_desc_idx = 0;

    /* Align tx dma buf base address & size to cache line to avoid tx
     * dma buf corrupted by cacahe line invalidation needed by other
     * address, if tx dma buf data has not been wrote to ram.
     */
    eqos->tx_dma_buf = (void *)osAllocAlign(CONFIG_ARCH_CACHE_LINE,
                                            EQOS_MAX_PACKET_SIZE);
    ASSERT(eqos->tx_dma_buf);
    memset(eqos->tx_dma_buf, 0, EQOS_MAX_PACKET_SIZE);

    /* rx dma buf base address & size should be cache line aligned.
     * Otherwise data before or after rx dma buf may locate in same
     * cache line with rx dma buf, when invalidate rx dma buf cache,
     * heap data before or after rx dma buf may be corrupted.
     */
    eqos->rx_dma_buf = (void *)osAllocAlign(CONFIG_ARCH_CACHE_LINE,
                                            EQOS_MAX_PACKET_SIZE * 4);
    ASSERT(eqos->rx_dma_buf);
    memset(eqos->rx_dma_buf, 0, EQOS_MAX_PACKET_SIZE * 4);

    //init phy
    phy_init(&eth_dev->phy_bus);

    //init mac
    eth_mac_init(cfg, eqos);

    // set up dma tx/rx descriptors
    memset(eqos->descs, 0, eqos->desc_size * EQOS_DESCRIPTORS_NUM);

    for (i = 0; i < EQOS_DESCRIPTORS_TX; i++) {
        struct eqos_desc *tx_desc = eqos_get_desc(eqos, i, false);
        arch_clean_cache_range((addr_t)tx_desc, sizeof(struct eqos_desc));
    }

    for (i = 0; i < EQOS_DESCRIPTORS_RX; i++) {
        struct eqos_desc *rx_desc = eqos_get_desc(eqos, i, true);
        rx_desc->des0 = (uint32_t)p2ap((paddr_t)eqos->rx_dma_buf +
                                       (i * EQOS_MAX_PACKET_SIZE));
        DMB;
        rx_desc->des3 = EQOS_DESC3_OWN | EQOS_DESC3_IOC | EQOS_DESC3_BUF1V;
        arch_clean_cache_range((addr_t)rx_desc, sizeof(struct eqos_desc));
        arch_invalidate_cache_range((addr_t)((char *)eqos->rx_dma_buf +
                                    (i * EQOS_MAX_PACKET_SIZE)), EQOS_MAX_PACKET_SIZE);
    }

    //enable rx/tx
    mac_start(cfg->base, true);

    eqos->started = true;

    eth_dev->eqos = eqos;

    return ret;
}

static inline void eqos_rx_lock(struct eqos_priv *eqos)
{
    osMutexAcquire(eqos->rx_lock, osWaitForever);
}

static inline void eqos_rx_unlock(struct eqos_priv *eqos)
{
    osMutexRelease(eqos->rx_lock);
}

static inline void eqos_tx_lock(struct eqos_priv *eqos)
{
    osMutexAcquire(eqos->tx_lock, osWaitForever);
}

static inline void eqos_tx_unlock(struct eqos_priv *eqos)
{
    osMutexRelease(eqos->tx_lock);
}

static err_t dwc_eth_txavail(struct net_driver_s *dev, struct pbuf *p)
{
    int i;
    struct eqos_desc *tx_desc;
    uint32_t offset = 0;
    struct pbuf *q = NULL;
    dwc_eth_dev_t *eth_dev = (dwc_eth_dev_t *)dev->d_private;
    dwc_eth_config_t *cfg = eth_dev->config;
    struct eqos_priv *eqos = eth_dev->eqos;

    if (p->tot_len > cfg->mtu) {
        ssdk_printf(SSDK_INFO, "eth tx len > MTU, %d\n", p->tot_len);
        return -1;
    }

    eqos_tx_lock(eqos);

    //copy data to pbuf
    for (q = p; q != NULL; q = q->next) {
        memcpy((uint8_t *)((uint8_t *)eqos->tx_dma_buf + offset),
               (uint8_t *)((uint8_t *)q->payload), q->len);
        offset += q->len;
    }

    arch_clean_cache_range((addr_t)eqos->tx_dma_buf, p->tot_len);

    tx_desc = eqos_get_desc(eqos, eqos->tx_desc_idx, false);

    eqos->tx_desc_idx ++;
    eqos->tx_desc_idx %= EQOS_DESCRIPTORS_TX;
    tx_desc->des0 = p2ap((paddr_t)eqos->tx_dma_buf);
    tx_desc->des1 = 0;
    tx_desc->des2 = p->tot_len;

    /*
     * Make sure that if HW sees the _OWN write below, it will see all the
     * writes to the rest of the descriptor too.
     */
    DMB;
    tx_desc->des3 = EQOS_DESC3_OWN | EQOS_DESC3_FD | EQOS_DESC3_LD | (p->tot_len);

    arch_clean_cache_range((addr_t)tx_desc, sizeof(struct eqos_desc));

    dwmac_set_tx_tail_ptr(cfg->base, (uint32_t)eqos_get_desc(eqos,
                          p2ap((paddr_t)eqos->tx_desc_idx), false), 0);

    eqos_tx_unlock(eqos);

    for (i = 0; i < 100000; i++) {
        arch_invalidate_cache_range((addr_t)tx_desc, sizeof(struct eqos_desc));

        if ((tx_desc->des3 & EQOS_DESC3_OWN) == 0) {
            return 0;
        }
    }

    ssdk_printf(SSDK_INFO, "%s: TX timeout\n", __func__);
    return -1;
}

static uint32_t dwc_eth_getmac(struct net_driver_s *dev, void *buf, uint32_t max_len)
{
    dwc_eth_dev_t *eth_dev = (dwc_eth_dev_t *)dev->d_private;
    uint8_t *mac_addr = eth_dev->config->mac_addr;

    if (mac_addr == NULL) {
        mac_addr = dwmac_get_default_mac_addr();
    }

    memcpy(buf, (void *)mac_addr, max_len);

    return max_len;
}

static uint32_t dwc_eth_getmtu(struct net_driver_s *dev)
{
    dwc_eth_dev_t *eth_dev = (dwc_eth_dev_t *)dev->d_private;

    return eth_dev->config->mtu;
}

static int dwc_eth_rx_poll(struct net_driver_s *dev)
{
    struct eqos_desc *rx_desc;
    uint32_t length = 0;
    uint8_t *recbuf;
    struct pbuf *rx_pbuf = NULL;
    dwc_eth_dev_t *eth_dev = (dwc_eth_dev_t *)dev->d_private;
    dwc_eth_config_t *cfg = eth_dev->config;
    struct eqos_priv *eqos = eth_dev->eqos;

    eqos_rx_lock(eqos);

    rx_desc = eqos_get_desc(eqos, eqos->rx_desc_idx, true);

    arch_invalidate_cache_range((addr_t)rx_desc, sizeof(struct eqos_desc));

    if (rx_desc->des3 & EQOS_DESC3_OWN) {
        eqos_rx_unlock(eqos);
        return -1;
    }

    /*
     * Make sure reading des3 before dma buffer.
     * Avoid speculative reading of buffer before des3 own bit cleared.
     */
    DMB;

    recbuf = (uint8_t *)eqos->rx_dma_buf + (eqos->rx_desc_idx *
                                            EQOS_MAX_PACKET_SIZE);
    length = rx_desc->des3 & 0x7FFF;

    arch_invalidate_cache_range((addr_t)((char *)recbuf), length);

    if (length < cfg->mtu) {
        rx_pbuf = pbuf_alloc(PBUF_RAW, length, PBUF_POOL);

        uint32_t offset = 0;
        struct pbuf *q = NULL;

        for (q = rx_pbuf; q != NULL; q = q->next) {
            memcpy( (uint8_t *)((uint8_t *)q->payload ), ((uint8_t *)recbuf + offset),
                    q->len);
            offset += q->len;
        }
    }
    else {
        ssdk_printf(SSDK_INFO, "len > 1500! desc3 = 0x%x\n", rx_desc->des3);
    }

    //free packet
    rx_desc->des0 = p2ap((paddr_t)recbuf);
    rx_desc->des1 = 0;
    rx_desc->des2 = 0;
    /*
     * Make sure DMA observes new des3 value after observing
     * new des0 & des1 & des2 value under any condition, especially
     * when des3 located in a different cache line and the cache line
     * is automatically evicted due to corresponding cache set full.
     */
    DMB;
    rx_desc->des3 = EQOS_DESC3_OWN | EQOS_DESC3_IOC | EQOS_DESC3_BUF1V;
    arch_clean_cache_range((addr_t)rx_desc, sizeof(struct eqos_desc));

    dwmac_set_rx_tail_ptr(cfg->base, p2ap((paddr_t)rx_desc), 0);
    eqos->rx_desc_idx++;
    eqos->rx_desc_idx %= EQOS_DESCRIPTORS_RX;

    eqos_rx_unlock(eqos);

    if (rx_pbuf)
        netdev_input(dev, rx_pbuf);

    return length;
}

static inline void dwc_eth_enable_dma_rx_int(dwc_eth_dev_t *dwc_eth)
{
    for (int chn = 0; chn < ETH_MAX_DMA_CHANNEL; chn++) {
        dwmac_enable_dma_rx_int(dwc_eth->config->base, chn, true);
    }
}

static inline void dwc_eth_disable_dma_rx_int(dwc_eth_dev_t *dwc_eth)
{
    for (int chn = 0; chn < ETH_MAX_DMA_CHANNEL; chn++) {
        dwmac_enable_dma_rx_int(dwc_eth->config->base, chn, false);
    }
}

/**
 * @brief try to read rx packet and send the packet to lwip stack.
 * Should be called in thread context.
 *
 * @param dev eth device
 * @param poll true for polling read, false for interrupt read
 * @return int total length of rx packets or -1 if rx nothing
 */
int dwc_eth_rx(struct net_driver_s *dev, bool poll)
{
    ASSERT(dev);

    if (poll)
        return dwc_eth_rx_poll(dev);
    else {
        dwc_eth_dev_t *dwc_eth = dev->d_private;
        int rx_pkt_len = 0;
        int len;

        dwc_eth_enable_dma_rx_int(dwc_eth);

        if (osOK != osSemaphoreAcquire(dwc_eth->rx_pkt_sem, osWaitForever))
            return -1;

        do {
            len = dwc_eth_rx_poll(dev);
            if (len != -1)
                rx_pkt_len += len;
        }
        while(len != -1);

        return rx_pkt_len ? rx_pkt_len : -1;
    }
}

static int dwc_eth_irq_handler(uint32_t irq, void *arg)
{
    dwc_eth_dev_t *dev = arg;
    uint32_t reg_base = dev->config->base;
    uint32_t status = dwmac_get_dma_int_status(reg_base);
    uint32_t sub_sts, detail;
    uint8_t chn = (irq == ENET1_SBD_INTR_NUM) ? 1 : 2;

    ssdk_printf(SSDK_DEBUG, "%s int status 0x%x\n", __func__, status);
    if (!status) {
        return -1;
    }

    if (status & (1 << 17)) {
        /* MAC interrupt */
        sub_sts = dwmac_get_mac_int_status(reg_base);
        ssdk_printf(SSDK_EMERG, "%s %d mac int status 0x%x\n", __func__, irq, sub_sts);
        if (sub_sts & (1 << 0)) {
            /* RGMII/SMII PHY link status changed. Read of MAC_PHYIF_Control_Status
             * register will clear the interrupt status.
             */
            detail = dwmac_get_mac_phyif_control_status(reg_base);
            if (detail & (1 << 19))
                ssdk_printf(SSDK_EMERG, "ETH%d phy link up\n", chn);
            else
                ssdk_printf(SSDK_EMERG, "ETH%d phy link down\n", chn);
        }
        if (sub_sts & (3 << 1)) {
            /* TBI/RTBI/SGMII PHY link status changed, or TBI/RTBI/SGMII PHY
             * auto-negotiation is completed. Read of MAC_AN_Status register
             * will clear the interrupt status.
             */
            detail = dwmac_get_mac_an_status(reg_base);
            ssdk_printf(SSDK_EMERG, "ETH%d mac an status 0x%x\n",
                        chn, detail);
        }
        if (sub_sts & (1 << 4)) {
            /* A magic packet or Wake-on-LAN packet is received.
             * Read of MAC_PMT_Control_Status register will clear
             * the interrupt status.
             */
            detail = dwmac_get_mac_pmt_control_status(reg_base);
            ssdk_printf(SSDK_EMERG, "ETH%d mac pmt ctrl status 0x%x\n",
                        chn, detail);
        }
        if (sub_sts & (1 << 5)) {
            /* LPI state entry or exit in the MAC Transmitter or Receiver.
             * Read of MAC_LPI_Control_Status register will clear the
             * interrupt status.
             */
            detail = dwmac_get_mac_lpi_control_status(reg_base);
            ssdk_printf(SSDK_EMERG, "ETH%d mac lpi ctrl status 0x%x\n",
                        chn, detail);
        }
    }

    if (status & (1 << 16)) {
        /* MTL interrupt */
        sub_sts = dwmac_get_mtl_int_status(reg_base);
        if (sub_sts & 0xFFFFFF00) {
            ssdk_printf(SSDK_EMERG, "ETH%d mtl int status 0x%x\n",
                        chn, sub_sts);
            ASSERT(0);
        }
        for (int i = 0; i < ETH_MAX_DMA_CHANNEL; i++) {
            if (sub_sts & (1 << i)) {
                detail = dwmac_get_mtl_q_int_status(reg_base, i);
                dwmac_clr_mtl_q_int_status(reg_base, i, detail);
                if (detail & (1 << 16))
                    ssdk_printf(SSDK_WARNING, "ETH%d mtl queue %d rx overflow\n",
                                chn, i);
                if (detail & (1 << 1))
                    ssdk_printf(SSDK_WARNING, "ETH%d mtl queue %d ABS updated\n",
                                chn, i);
                if (detail & (1 << 1))
                    ssdk_printf(SSDK_WARNING, "ETH%d mtl queue %d tx underflow\n",
                                chn, i);
            }
        }
    }

    if (status & 0x7F) {
        /* DMA interrupt */
        for (int chn = 0; chn < ETH_MAX_DMA_CHANNEL; chn++) {
            sub_sts = dwmac_get_dma_chn_status(reg_base, chn);
            dwmac_clear_dma_chn_int_status(reg_base, chn);
            if (sub_sts & BIT(6)) {
                /* DMA rx interrupt */
                ssdk_printf(SSDK_DEBUG, "%s: chn %d rx int\n", __func__, chn);
                dwmac_enable_dma_rx_int(reg_base, chn, false);
                osSemaphoreRelease(dev->rx_pkt_sem);
            }
            if (sub_sts & BIT(0)) {
                /* DMA tx interrupt */
                ssdk_printf(SSDK_DEBUG, "%s: chn %d rx int\n", __func__, chn);
                /* Nothing to do for now. */
            }
        }
    }

    return 0;
}

static int dwc_eqos_mdio_read(phy_bus_t *bus, uint32_t phyaddr,
                              uint32_t devaddr, uint32_t reg)
{
    dwc_eth_dev_t *dwc_eth_dev = containerof(bus, dwc_eth_dev_t, phy_bus);
    uint32_t reg_base = dwc_eth_dev->config->base;

    return mac_mdio_read(reg_base, phyaddr, devaddr, reg);
}

static int dwc_eqos_mdio_write(phy_bus_t *bus, uint32_t phyaddr,
                               uint32_t devaddr, uint32_t reg, uint32_t data)
{
    dwc_eth_dev_t *dwc_eth_dev = containerof(bus, dwc_eth_dev_t, phy_bus);
    uint32_t reg_base = dwc_eth_dev->config->base;

    return mac_mdio_write(reg_base, phyaddr, devaddr, reg, data);
}

static phy_bus_ops_t g_dwc_eqos_mdio_ops = {
    .mdio_read = dwc_eqos_mdio_read,
    .mdio_write = dwc_eqos_mdio_write
};

static void dwc_eth_phy_bus_init(phy_bus_t *bus)
{
    list_initialize(&bus->phy_dev_list);
    spin_lock_init(&bus->lock);
    bus->ref_cnt = 0;
    bus->ops = &g_dwc_eqos_mdio_ops;
}

static void eth_receive(void *arg)
{
    struct net_driver_s *dev = arg;

    while (true)
        dwc_eth_rx(dev, false);
}

static void dwc_eth_service_init(void *arg)
{
    struct net_driver_s *dev = arg;

    osThreadAttr_t attr = {
        .name = "eth_receive",
        .priority = osPriorityNormal
    };
    ASSERT(osThreadNew(eth_receive, dev, &attr));
}

int dwc_eth_probe(struct net_driver_s *dev, dwc_eth_config_t *cfg)
{
    ASSERT(dev && cfg);

    dev->d_init = dwc_eth_init;
    dev->d_txavail = dwc_eth_txavail;
    dev->d_getmac = dwc_eth_getmac;
    dev->d_getmtu = dwc_eth_getmtu;

    dwc_eth_dev_t *dw_eth_dev = osAlloc(sizeof(dwc_eth_dev_t));
    ASSERT(dw_eth_dev);
    dw_eth_dev->init = false;
    dw_eth_dev->config = cfg;
    dev->d_private = dw_eth_dev;

    dev->d_ipaddr.addr = (cfg->ip[3] << 24) | (cfg->ip[2] << 16) |
                         (cfg->ip[1] << 8) | cfg->ip[0];
    dev->d_netmask.addr = (cfg->mask[3] << 24) | (cfg->mask[2] << 16) |
                          (cfg->mask[1] << 8) | cfg->mask[0];
    dev->d_gateway.addr = (1 << 24) | (cfg->ip[2] << 16) |
                          (cfg->ip[1] << 8) | cfg->ip[0];

    dw_eth_dev->rx_pkt_sem = osSemaphoreNew(1, 0, NULL);
    ASSERT(dw_eth_dev->rx_pkt_sem);

    dwc_eth_phy_bus_init(&dw_eth_dev->phy_bus);

    for (int i = 0; cfg->phy[i].phy_addr != ~0; i++) {
        phy_dev_register(&dw_eth_dev->phy_bus, &cfg->phy[i]);
    }

    int ret = netdev_register(dev, NET_LL_ETHERNET);

    irq_attach(cfg->irq_num, dwc_eth_irq_handler, dw_eth_dev);
    irq_enable(cfg->irq_num);

    dwc_eth_service_init(dev);

    return ret;
}
