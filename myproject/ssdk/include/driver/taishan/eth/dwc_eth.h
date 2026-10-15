/**
 * dwc_eth.h
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: dw eth mac drv head file
 *
 * Revision History:
 * -----------------
 */

#ifndef DWC_ETH_QOS_H
#define DWC_ETH_QOS_H

#include <stdint.h>
#include <stdbool.h>
#include "driver/scr.h"
#include "driver/clk.h"
#include "driver/reset.h"
#include "cmsis_os2.h"
#include "netdev.h"
#include "../../../../drivers/taishan/eth/dw_mac_lld.h"
#include "driver/taishan/eth/phy.h"

/* assume ARCH_DMA_MINALIGN >= 16; 16 is the EQOS HW minimum */
#define ARCH_DMA_MINALIGN CONFIG_ARCH_CACHE_LINE    /* Should be cache line aligned */

#define EQOS_DESCRIPTOR_ALIGN  ARCH_DMA_MINALIGN
#define EQOS_DESCRIPTORS_TX    4
#define EQOS_DESCRIPTORS_RX    4
#define EQOS_DESCRIPTORS_NUM   (EQOS_DESCRIPTORS_TX + EQOS_DESCRIPTORS_RX)
#define EQOS_BUFFER_ALIGN      ARCH_DMA_MINALIGN
#define EQOS_MAX_PACKET_SIZE   ALIGN(1568, ARCH_DMA_MINALIGN)
#define EQOS_RX_BUFFER_SIZE    (EQOS_DESCRIPTORS_RX * EQOS_MAX_PACKET_SIZE)

/* DMA descriptor */
struct eqos_desc {
    volatile uint32_t des0;
    volatile uint32_t des1;
    volatile uint32_t des2;
    volatile uint32_t des3;
};

#define EQOS_DESC3_OWN      BIT(31)
#define EQOS_DESC3_IOC      BIT(30)
#define EQOS_DESC3_FD       BIT(29)
#define EQOS_DESC3_LD       BIT(28)
#define EQOS_DESC3_BUF1V    BIT(24)

typedef enum {
    ETHERNET1_IDX = 0,
    ETHERNET2_IDX,
} eth_idx_type;

typedef enum {
    /* Controller Disable */
    ETH_MODE_DOWN = 0,
    /* Controller Enable */
    ETH_MODE_ACTIVE,
} eth_mode_type;

typedef enum {
    /* add the MAC address to the filter, meaning allow reception */
    ETH_ADD_TO_FILTER = 0,
    /* remove the MAC address from the filter, meaning reception is blocked in the lower layer */
    ETH_REMOVE_FROM_FILTER,

} eth_filter_action_type;

typedef enum {
    ETH_MAC_LAYER_SPEED_100M = 0,
    ETH_MAC_LAYER_SPEED_10G,
    ETH_MAC_LAYER_SPEED_10M,
    ETH_MAC_LAYER_SPEED_1G
} eth_mac_layer_speed_type;

typedef enum {
    /* MAC layer interface (data) bandwith class 1Gbit/s (e.g. GMII, RGMII, SGMII, RvGMII, USGMII)*/
    ETH_MAC_LAYER_TYPE_XGMII = 0,
    /* MAC layer interface (data) bandwith class 100Mbit/s (e.g. RMII, RvMII, SMII, RvMII) */
    ETH_MAC_LAYER_TYPE_XMII,
    /* MAC layer interface (data) bandwith class 10Gbit/s */
    ETH_MAC_LAYER_TYPE_XXGMII,
} eth_mac_layer_type;

typedef enum {
    LIGHT = 0,
    REDUCED,
    REVERSED,
    SERIAL,
    STANDARD,
    UNIVERSAL_SERIAL
} eth_mac_layer_sub_type;

typedef enum {
    ETH_PHY_INTF_SEL_GMII = 0,
    ETH_PHY_INTF_SEL_RGMII,
    ETH_PHY_INTF_SEL_SGMII,
    ETH_PHY_INTF_SEL_TBI,
    ETH_PHY_INTF_SEL_RMII,
    ETH_PHY_INTF_SEL_RTBI,
    ETH_PHY_INTF_SEL_SMII,
    ETH_PHY_INTF_SEL_REVMII,
    ETH_PHY_INTF_SEL_MII,
} eth_phy_intf_mode_type;

typedef struct {
    uint32_t base;
    uint32_t irq_num;
    uint32_t mtu;
    uint8_t *mac_addr;

    /* Bus width, unit: bit. */
    uint8_t dma_bus_width;

    /* clock & reset */
    clk_node_t *tx_clk;
    clk_node_t *rmii_clk;
    clk_node_t *phy_ref_clk;
    clk_node_t *timer_sec_clk;
    reset_sig_t *rst;

    /* mii interface config */
    eth_phy_intf_mode_type phy_intf_mode;
    void (*set_phy_intf)(uint32_t base,
                         eth_phy_intf_mode_type mode);

    /* ipv4 config */
    uint8_t ip[4];
    uint8_t mask[4];

    /* All PHYs connected to this mac */
    phy_dev_t *phy;
} dwc_eth_config_t;

struct eqos_priv {
    osMutexId_t rx_lock;
    osMutexId_t tx_lock;
    void *descs;
    int tx_desc_idx, rx_desc_idx;
    unsigned int desc_size;
    /* Channel control register DSL bits. */
    uint32_t dsl;
    void *tx_dma_buf;
    void *rx_dma_buf;
    bool started;
};

typedef struct dwc_eth_dev {
    bool init;
    osSemaphoreId_t rx_pkt_sem;
    dwc_eth_config_t *config;
    struct eqos_priv *eqos;
    phy_bus_t phy_bus;
} dwc_eth_dev_t;

int dwc_eth_rx(struct net_driver_s *dev, bool poll);
int dwc_eth_probe(struct net_driver_s *dev, dwc_eth_config_t *cfg);

#endif
