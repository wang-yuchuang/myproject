/**
 * phy.h
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: eth phy driver header file
 *
 * Revision History:
 * -----------------
 */

#ifndef _PHY_H_
#define _PHY_H_

#include <stdint.h>
#include "lib/list.h"
#include "spinlock.h"
#include "../../../../drivers/taishan/eth/dw_mac_lld.h"

typedef enum {
    /* Automatic Negotiation */
    ETH_PHY_CONN_NEG_AUTO = 0,
    /* Master */
    ETH_PHY_CONN_NEG_MASTER,
    /* Slave */
    ETH_PHY_CONN_NEG_SLAVE
} eth_phy_conn_neg_mode_type;

typedef enum {
    /* Half duplex Ethernet connection */
    ETH_PHY_DUPLEX_MODE_HALF = 0,
    /* 0x01 Full duplex Ethernet connection */
    ETH_PHY_DUPLEX_MODE_FULL
} eth_duplex_mode_type;

typedef enum phy_speed{
    PHY_SPEED_10 = 0x0,
    PHY_SPEED_100,
    PHY_SPEED_1000,
} phy_speed_t;

typedef struct phy_bus phy_bus_t;

typedef struct phy_bus_ops {
    int (*mdio_read)(phy_bus_t *bus, uint32_t phyaddr, uint32_t devaddr,
                     uint32_t regaddr);
    int (*mdio_write)(phy_bus_t *bus , uint32_t phyaddr, uint32_t devaddr,
                      uint32_t regaddr, uint32_t data);
} phy_bus_ops_t;

struct phy_bus {
    phy_bus_ops_t *ops;

    spin_lock_t lock;
    uint32_t ref_cnt;
    struct list_node phy_dev_list;
};

typedef struct phy_dev phy_dev_t;

struct phy_dev {
    struct list_node node;
    phy_bus_t *bus;

    uint32_t phy_addr;
    eth_phy_conn_neg_mode_type conn_neg_mode;
    eth_duplex_mode_type duplex_mode;
    phy_speed_t speed;
    bool auto_negotiation;
    int (*init)(phy_dev_t *dev);
};

void phy_dev_register(phy_bus_t *bus, phy_dev_t *dev);
void phy_init(phy_bus_t *bus);

#endif
