/**
 * phy.c
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: eth phy driver
 *
 * Revision History:
 * -----------------
 */

#include "driver/kunlun/eth/phy.h"

void phy_dev_register(phy_bus_t *bus, phy_dev_t *dev)
{
    irq_state_t state;

    state = spin_lock_irqsave(&bus->lock);
    list_add_tail(&bus->phy_dev_list, &dev->node);
    bus->ref_cnt++;
    dev->bus = bus;
    spin_unlock_irqrestore(&bus->lock, state);
}

void phy_init(phy_bus_t *bus)
{
    phy_dev_t *phy;

    list_for_every_entry(&bus->phy_dev_list, phy, phy_dev_t, node) {
        if (phy->init)
            phy->init(phy);
    }
}
