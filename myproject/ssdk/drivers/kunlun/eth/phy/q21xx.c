/*
* q21xx.c
*
* Copyright (c) 2020 Semidrive Semiconductor.
* All rights reserved.
*
* Description: eth phy q21xx drv
*
* Revision History:
* -----------------
*/

#include "debug.h"
#include "board/board.h"
#include "driver/kunlun/eth/dw_mac_lld.h"
#include "q21xx.h"

#define     msleep(x)     udelay(x * 1000)

/**
 * @description: mii read data from phy
 * @return {*}
 * @param {phy_dev_t *} phy device
 * @param {uint32} devaddr
 * @param {uint32} regaddr
 */
static uint16_t ma211x_read(phy_dev_t *dev, uint32_t devaddr,
                            uint32_t regaddr)
{
    phy_bus_t *bus = dev->bus;
    return (bus->ops->mdio_read(bus, dev->phy_addr, devaddr, regaddr));
}

/**
 * @description: mii write data to phy
 * @return {*}
 * @param {phy_dev_t *} phy device
 * @param {uint32} devaddr
 * @param {uint32} regaddr
 * @param {uint16_t} val
 */
static int32_t ma211x_write(phy_dev_t *dev, uint32_t devaddr,
                            uint32_t regaddr, uint16_t val)
{
    phy_bus_t *bus = dev->bus;
    if (bus->ops->mdio_write(bus, dev->phy_addr, devaddr, regaddr, val)) {
        ssdk_printf(SSDK_INFO, "mac_mdio_write err!\n");
        return -1;
    }

    return 0;
}

/**
 * @description: get autonego status
 * @param {*}
 * @return 1 enable autonego;0 disable autonego
 * @param {phy_dev_t *} phy device
 */
static int32_t ma211x_get_autonego_enable(phy_dev_t *dev)
{
    return (0x0 != (ma211x_read(dev, 7, 0x0200) & 0x1000));
}

/**
 * @description: set autonego status
 * @return {*}
 * @param {phy_dev_t *} phy device
 */
static void ma211x_set_autonego_enalbe(phy_dev_t *dev)
{
    if (dev->auto_negotiation)
        ma211x_write(dev, 7, 0x0200, 0x1200);
    else
        ma211x_write(dev, 7, 0x0200, 0);
}

/**
 * @description: get link speed
 * @return 1 1000MB; 0 100MB
 * @param {phy_dev_t *} phy device
 */
int32_t ma211x_get_speed(phy_dev_t *dev)
{
    if (ma211x_get_autonego_enable(dev))
        return ((ma211x_read(dev, 7, 0x801a) & 0x4000) >> 14);
    else
        return (ma211x_read(dev, 1, 0x0834) & 0x0001);
}

/**
 * @description: set autonego role
 * @return {*}
 * @param {phy_dev_t *} phy device
 */
static int32_t ma211x_set_master(phy_dev_t *dev)
{
    uint16_t data = ma211x_read(dev, 1, 0x0834);

    if (dev->conn_neg_mode == ETH_PHY_CONN_NEG_MASTER)
        data |= 0x4000;
    else
        data &= 0xBFFF;

    return ma211x_write(dev, 1, 0x0834, data);
}

/**
 * @description:get link status
 * @return {*}
 * @param {phy_dev_t *} phy device
 */
int32_t ma211x_check_link(phy_dev_t *dev)
{
    uint32_t data;

    if (ma211x_get_speed(dev) == MRVL_88Q2112_1000BASE_T1) {
        data = ma211x_read(dev, 3, 0x0901);
        data = ma211x_read(dev, 3, 0x0901);
    }
    else
        data = ma211x_read(dev, 3, 0x8109);

    if (0x4 == (data & 0x4))
        return 1;
    else
        return 0;
}

/**
 * @description: init 1000mbits mode
 * @return {*}
 * @param {phy_dev_t *} phy device
 */
static int32_t ma211x_ge_init(phy_dev_t *dev)
{
    uint16_t data;

    ma211x_write(dev, 1, 0x0900, 0x4000);

    data = ma211x_read(dev, 1, 0x0834);
    data = (data & 0xFFF0) | 0x0001;
    ma211x_write(dev, 1, 0x0834, data);

    ma211x_write(dev, 3, 0xFFE4, 0x07B5);
    ma211x_write(dev, 3, 0xFFE4, 0x06B6);
    msleep(100);

    ma211x_write(dev, 3, 0xFFDE, 0x402F);
    ma211x_write(dev, 3, 0xFE2A, 0x3C3D);
    ma211x_write(dev, 3, 0xFE34, 0x4040);
    ma211x_write(dev, 3, 0xFE2A, 0x3C1D);
    ma211x_write(dev, 3, 0xFE34, 0x0040);
    ma211x_write(dev, 7, 0x8032, 0x0064);
    ma211x_write(dev, 7, 0x8031, 0x0A01);
    ma211x_write(dev, 7, 0x8031, 0x0C01);
    ma211x_write(dev, 3, 0xFE0F, 0x0000);
    ma211x_write(dev, 3, 0x800C, 0x0000);
    ma211x_write(dev, 3, 0x801D, 0x0800);
    ma211x_write(dev, 3, 0xFC00, 0x01C0);
    ma211x_write(dev, 3, 0xFC17, 0x0425);
    ma211x_write(dev, 3, 0xFC94, 0x5470);
    ma211x_write(dev, 3, 0xFC95, 0x0055);
    ma211x_write(dev, 3, 0xFC19, 0x08D8);
    ma211x_write(dev, 3, 0xFC1a, 0x0110);
    ma211x_write(dev, 3, 0xFC1b, 0x0A10);
    ma211x_write(dev, 3, 0xFC3A, 0x2725);
    ma211x_write(dev, 3, 0xFC61, 0x2627);
    ma211x_write(dev, 3, 0xFC3B, 0x1612);
    ma211x_write(dev, 3, 0xFC62, 0x1C12);
    ma211x_write(dev, 3, 0xFC9D, 0x6367);
    ma211x_write(dev, 3, 0xFC9E, 0x8060);
    ma211x_write(dev, 3, 0xFC00, 0x01C8);
    // tx disable
    ma211x_write(dev, 3, 0x8000, 0x0000);
    ma211x_write(dev, 3, 0x8016, 0x0011);

    ma211x_write(dev, 3, 0xFDA3, 0x1800);

    ma211x_write(dev, 3, 0xFE02, 0x00C0);
    ma211x_write(dev, 3, 0xFFDB, 0x0010);
    ma211x_write(dev, 3, 0xFFF3, 0x0020);
    ma211x_write(dev, 3, 0xFE40, 0x00A6);

    ma211x_write(dev, 3, 0xFE60, 0x0000);
    ma211x_write(dev, 3, 0xFE04, 0x0008);
    ma211x_write(dev, 3, 0xFE2A, 0x3C3D);
    ma211x_write(dev, 3, 0xFE4B, 0x9334);

    ma211x_write(dev, 3, 0xFC10, 0xF600);
    ma211x_write(dev, 3, 0xFC11, 0x073D);
    ma211x_write(dev, 3, 0xFC12, 0x000D);
    // RGMII tx delay
    ma211x_write(dev, 31, 0x8001, 0x4000);
    return 0;
}

/**
 * @description: set phy mode
 * @return {*}
 * @param {phy_dev_t *} phy device
 * @param {uint32} op_mode
 */
static int32_t ma211x_ge_set_opmode(phy_dev_t *dev, uint32_t op_mode)
{
    if (op_mode == MRVL_88Q211X_MODE_LEGACY) {
        // enable 1000 BASE-T1 legacy mode support
        ma211x_write(dev, 3, 0xFDB8, 0x0001);
        ma211x_write(dev, 3, 0xFD3D, 0x0C14);
    }
    else {
        // set back to default compliant mode setting
        ma211x_write(dev, 3, 0xFDB8, 0x0000);
        ma211x_write(dev, 3, 0xFD3D, 0x0000);
    }

    ma211x_write(dev, 1, 0x0902, op_mode | MRVL_88Q211X_MODE_ADVERTISE);
    return 0;
}

/**
 * @description: soft reset
 * @return {*}
 * @param {phy_dev_t *} phy device
 */
static int32_t ma211x_ge_soft_reset(phy_dev_t *dev)
{
    uint16_t data;

    if (ma211x_get_autonego_enable(dev))
        ma211x_write(dev, 3, 0xFFF3, 0x0024);

    // enable low-power mode
    data = ma211x_read(dev, 1, 0x0000);
    ma211x_write(dev, 1, 0x0000, data | 0x0800);

    ma211x_write(dev, 3, 0xFFF3, 0x0020);
    ma211x_write(dev, 3, 0xFFE4, 0x000C);
    msleep(100);

    ma211x_write(dev, 3, 0xffe4, 0x06B6);

    // disable low-power mode
    ma211x_write(dev, 1, 0x0000, data & 0xF7FF);
    msleep(100);

    ma211x_write(dev, 3, 0xFC47, 0x0030);
    ma211x_write(dev, 3, 0xFC47, 0x0031);
    ma211x_write(dev, 3, 0xFC47, 0x0030);
    ma211x_write(dev, 3, 0xFC47, 0x0000);
    ma211x_write(dev, 3, 0xFC47, 0x0001);
    ma211x_write(dev, 3, 0xFC47, 0x0000);

    ma211x_write(dev, 3, 0x0900, 0x8000);
    // disable transmitters bit14
    ma211x_write(dev, 1, 0x0900, 0x0001 << 14);
    ma211x_write(dev, 3, 0xFFE4, 0x000C);
    return 0;
}

/**
 * @description: init 100mbits mode
 * @return {*}
 * @param {phy_dev_t *} phy device
 */
static int32_t ma211x_fe_init(phy_dev_t *dev)
{
    uint16_t data = 0;

    ma211x_write(dev, 3, 0xFA07, 0x0202);

    data = ma211x_read(dev, 1, 0x0834);
    data = data & 0xFFF0;
    ma211x_write(dev, 1, 0x0834, data);
    msleep(50);

    ma211x_write(dev, 3, 0x8000, 0x0000);
    ma211x_write(dev, 3, 0x8100, 0x0200);
    ma211x_write(dev, 3, 0xFA1E, 0x0002);
    ma211x_write(dev, 3, 0xFE5C, 0x2402);
    ma211x_write(dev, 3, 0xFA12, 0x001F);
    ma211x_write(dev, 3, 0xFA0C, 0x9E05);
    ma211x_write(dev, 3, 0xFBDD, 0x6862);
    ma211x_write(dev, 3, 0xFBDE, 0x736E);
    ma211x_write(dev, 3, 0xFBDF, 0x7F79);
    ma211x_write(dev, 3, 0xFBE0, 0x8A85);
    ma211x_write(dev, 3, 0xFBE1, 0x9790);
    ma211x_write(dev, 3, 0xFBE3, 0xA39D);
    ma211x_write(dev, 3, 0xFBE4, 0xB0AA);
    ma211x_write(dev, 3, 0xFBE5, 0x00B8);
    ma211x_write(dev, 3, 0xFBFD, 0x0D0A);
    ma211x_write(dev, 3, 0xFBFE, 0x0906);
    ma211x_write(dev, 3, 0x801D, 0x8000);
    ma211x_write(dev, 3, 0x8016, 0x0011);

    // reset pcs
    ma211x_write(dev, 3, 0x0900, 0x8000);
    ma211x_write(dev, 3, 0xFA07, 0x0200);
    msleep(100);

    ma211x_write(dev, 31, 0x8001, 0xc000);
    return 0;
}

/**
 * @description: init phy
 * @return {*}
 * @param {phy_dev_t *} phy device
 */
int32_t ma211x_config_init(phy_dev_t *dev)
{
    ma211x_set_master(dev);
    ma211x_set_autonego_enalbe(dev);

    if (PHY_SPEED_100 == dev->speed) {
        ma211x_fe_init(dev);
    }
    else if (PHY_SPEED_1000 == dev->speed) {
        ma211x_ge_init(dev);
        ma211x_ge_set_opmode(dev, MRVL_88Q211X_MODE_DEFAULT);
        ma211x_ge_soft_reset(dev);
    }
    else {
        return -1;
    }

    return 0;
}

/**
 * @description: enable phy
 * @return {*}
 * @param {phy_dev_t *} phy device
 */
void enable_transmitters(phy_dev_t *dev)
{
    // enable transmitters bit14
    ma211x_write(dev, 1, 0x0900, 0x0000);
}

/**
 * @description: disable phy
 * @return {*}
 * @param {phy_dev_t *} phy device
 */
void disable_transmitters(phy_dev_t *dev)
{
    // disable Transmitters bit14
    ma211x_write(dev, 1, 0x0900, 0x01 << 14 );
}

/**
 * @description:
 * @return {*}
 * @param {phy_dev_t *} phy device
 * @param {uint32} mode
 */
void set_tx_mode(phy_dev_t *dev, uint32_t mode)
{
    uint32_t val = ma211x_read(dev, 3, 0x8000);

    if (mode == ETH_PHY_PHYTXMODE_NORMAL) {
        val &= ~(0x01 << 3);
        ma211x_write(dev, 3, 0x8000, val);
    }
    else if (mode == ETH_PHY_PHYTXMODE_TX_OFF) {
        val = ma211x_read(dev, 3, 0x8000);
        val |= 0x01 << 3;
        // enable transmitters bit14
        ma211x_write(dev, 3, 0x8000, val);
    }
}

/**
 * @description:
 * @return {*}
 * @param {phy_dev_t *} phy device
 */
uint32_t get_signal_quality(phy_dev_t *dev)
{
    uint32_t val;
    val = ma211x_read(dev, 3, 0x8230);
    val >>= 12;
    val &= 0xF;
    return val;
}

/**
 * @description:
 * @return {*}
 * @param {phy_dev_t *} phy device
 * @param {uint32} mode
 */
void set_loopback_mode(phy_dev_t *dev, uint32_t mode)
{
    uint32_t val;

    if (mode == ETH_PHY_PHYLOOPBACK_INTERNAL) {
        val = ma211x_read(dev, 3, 0x0900);
        val |= 0x01 << 14;
        ma211x_write(dev, 3, 0x0900, val);
    }
    else if (mode == ETH_PHY_PHYLOOPBACK_REMOTE) {
        if (ma211x_get_speed(dev) == 0) {
            val = ma211x_read(dev, 3, 0x8100);
            val |= 0x01 << 14;
            ma211x_write(dev, 3, 0x8100, val);
        }
        else {
            val = ma211x_read(dev, 3, 0xFD00);
            val |= 0x01 << 10;
            ma211x_write(dev, 3, 0xFD00, val);
        }
    }
}

/**
 * @description:
 * @return {*}
 * @param {phy_dev_t *} phy device
 * @param {uint32} mode
 */
int32_t set_test_mode(phy_dev_t *dev, uint32_t mode)
{
    uint32_t val;

    if (mode != ETH_PHY_PHYTESTMODE_3) {
        val = ma211x_read(dev, 1, 0x0904);
        val &= 0x7 << 13;
        val |= mode << 13;
        ma211x_write(dev, 1, 0x0904, val);
        return 0;
    }

    return -1;
}

/**
 * @description:
 * @return {*}
 * @param {phy_dev_t *} phy device
 */
uint32_t phy_is_duplex_mode(phy_dev_t *dev)
{
    uint32_t val;
    val = ma211x_read(dev, 1, 0x8004);

    if (val & (0x01 << 12)) {
        return 1;
    }

    return 0;
}

int ma21xx_init(phy_dev_t *dev)
{
    ma211x_config_init(dev);
    enable_transmitters(dev);

    return 0;
}
