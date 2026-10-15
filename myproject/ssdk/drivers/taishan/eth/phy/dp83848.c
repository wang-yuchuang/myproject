/*
 * dp83848.c
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: eth phy dp83848 drv
 *
 * Revision History:
 * -----------------
 */

#include <stdlib.h>
#include "debug.h"
#include "CLI.h"
#include "board.h"
#include "driver/taishan/eth/phy.h"

#define MII_BMCR		0x00	/* Basic mode control register */
#define MII_BMSR		0x01	/* Basic mode status register  */
#define MII_PHYSID1		0x02	/* PHYS ID 1                   */
#define MII_PHYSID2		0x03	/* PHYS ID 2                   */
#define MII_ADVERTISE		0x04	/* Advertisement control reg   */
#define MII_LPA			0x05	/* Link partner ability reg    */
#define MII_EXPANSION		0x06	/* Expansion register          */

#define MII_PHYSTS      0x10
#define MII_PHYCR       0x19

/* Basic mode control register. */
#define BMCR_RESV		0x003f	/* Unused...                   */
#define BMCR_SPEED1000		0x0040	/* MSB of Speed (1000)         */
#define BMCR_CTST		0x0080	/* Collision test              */
#define BMCR_FULLDPLX		0x0100	/* Full duplex                 */
#define BMCR_ANRESTART		0x0200	/* Auto negotiation restart    */
#define BMCR_ISOLATE		0x0400	/* Isolate data paths from MII */
#define BMCR_PDOWN		0x0800	/* Enable low power state      */
#define BMCR_ANENABLE		0x1000	/* Enable auto negotiation     */
#define BMCR_SPEED100		0x2000	/* Select 100Mbps              */
#define BMCR_LOOPBACK		0x4000	/* TXD loopback bits           */
#define BMCR_RESET		0x8000	/* Reset to default state      */
#define BMCR_SPEED10		0x0000	/* Select 10Mbps               */

/* Basic mode status register */
#define BMSR_LINK_STATUS    0x4

/* Advertisement control register. */
#define ADVERTISE_SLCT		0x001f	/* Selector bits               */
#define ADVERTISE_CSMA		0x0001	/* Only selector supported     */
#define ADVERTISE_10HALF	0x0020	/* Try for 10mbps half-duplex  */
#define ADVERTISE_1000XFULL	0x0020	/* Try for 1000BASE-X full-duplex */
#define ADVERTISE_10FULL	0x0040	/* Try for 10mbps full-duplex  */
#define ADVERTISE_1000XHALF	0x0040	/* Try for 1000BASE-X half-duplex */
#define ADVERTISE_100HALF	0x0080	/* Try for 100mbps half-duplex */
#define ADVERTISE_1000XPAUSE	0x0080	/* Try for 1000BASE-X pause    */
#define ADVERTISE_100FULL	0x0100	/* Try for 100mbps full-duplex */
#define ADVERTISE_1000XPSE_ASYM	0x0100	/* Try for 1000BASE-X asym pause */
#define ADVERTISE_100BASE4	0x0200	/* Try for 100mbps 4k packets  */
#define ADVERTISE_PAUSE_CAP	0x0400	/* Try for pause               */
#define ADVERTISE_PAUSE_ASYM	0x0800	/* Try for asymetric pause     */
#define ADVERTISE_RESV		0x1000	/* Unused...                   */
#define ADVERTISE_RFAULT	0x2000	/* Say we can detect faults    */
#define ADVERTISE_LPACK		0x4000	/* Ack link partners response  */
#define ADVERTISE_NPAGE		0x8000	/* Next page bit               */

/* PHY status register */
#define PHYSTS_DUPLEX       0x40
#define PHYSTS_SPEED        0x20

/* PHY control register */
#define PHYCR_MDIX_EN       0x8000

#define PHY_CLI_EN  1

#if PHY_CLI_EN
static phy_dev_t *g_dp83848_dev;
#endif

static int dp83848_read(phy_dev_t *dev, uint32_t regaddr)
{
    phy_bus_t *bus = dev->bus;
    int ret = bus->ops->mdio_read(bus, dev->phy_addr, 0xFF, regaddr);
    if (ret < 0)
        ssdk_printf(SSDK_ERR, "dp83848_read err!\n");

    return ret;
}

static int dp83848_write(phy_dev_t *dev, uint32_t regaddr, uint16_t val)
{
    phy_bus_t *bus = dev->bus;
    if (bus->ops->mdio_write(bus, dev->phy_addr, 0xFF, regaddr, val)) {
        ssdk_printf(SSDK_ERR, "dp83848_write err!\n");
        return -1;
    }

    return 0;
}

static int dp83848_reset(phy_dev_t *dev)
{
    int timeout = 500;
    int reg_val;

    if (dp83848_write(dev, MII_BMCR, BMCR_RESET)) {
        ssdk_printf(SSDK_ERR, "PHY reset failed\n");
        return -1;
    }

    while (timeout--) {
        reg_val = dp83848_read(dev, MII_BMCR);
        if (reg_val < 0) {
            ssdk_printf(SSDK_ERR, "BMCR read failed\n");
            return -2;
        }
        if (!(reg_val & BMCR_RESET))
            break;

        udelay(1000);
    }

    if (reg_val & BMCR_RESET) {
        ssdk_printf(SSDK_ERR, "PHY reset timeout\n");
        return -3;
    }

    return 0;
}

static uint32_t dp83848_get_phy_id(phy_dev_t *dev)
{
    int reg_val;

    reg_val = dp83848_read(dev, MII_PHYSID1);
    if (reg_val < 0) {
        ssdk_printf(SSDK_ERR, "Read phy id1 error\n");
        return reg_val;
    }

    reg_val <<= 16;
    int temp = dp83848_read(dev, MII_PHYSID2);
    if (temp < 0) {
        ssdk_printf(SSDK_ERR, "Read phy id2 error\n");
        return temp;
    }
    reg_val |= temp;

    return reg_val;
}

static int dp83848_config_aneg(phy_dev_t *dev)
{
    int reg_val, ret, timeout = 10000;

    if (dev->auto_negotiation) {
        reg_val = dp83848_read(dev, MII_ADVERTISE);
        if (reg_val < 0) {
            ssdk_printf(SSDK_ERR, "Read advertise error\n");
            return reg_val;
        }

        reg_val |= ADVERTISE_100FULL | ADVERTISE_100HALF |
                   ADVERTISE_10FULL | ADVERTISE_10HALF;
        ret = dp83848_write(dev, MII_ADVERTISE, reg_val);
        if (ret) {
            ssdk_printf(SSDK_ERR, "Config advertisement register failed\n");
            return ret;
        }

        reg_val = dp83848_read(dev, MII_BMCR);
        if (reg_val < 0) {
            ssdk_printf(SSDK_ERR, "Read BMCR failed\n");
            return reg_val;
        }
        reg_val |= BMCR_ANENABLE;
        ret = dp83848_write(dev, MII_BMCR, reg_val);
        if (ret)
            ssdk_printf(SSDK_ERR, "Write BMCR failed\n");
    }
    else {
        reg_val = dp83848_read(dev, MII_BMCR);
        if (reg_val < 0) {
            ssdk_printf(SSDK_ERR, "Read BMCR failed\n");
            return reg_val;
        }

        reg_val &= ~BMCR_ANENABLE;

        if (dev->speed == PHY_SPEED_10)
            reg_val &= ~BMCR_SPEED100;
        else
            reg_val |= BMCR_SPEED100;

        if (dev->duplex_mode == ETH_PHY_DUPLEX_MODE_HALF)
            reg_val &= ~BMCR_FULLDPLX;
        else
            reg_val |= BMCR_FULLDPLX;

        ret = dp83848_write(dev, MII_BMCR, reg_val);
        if (ret)
            ssdk_printf(SSDK_ERR, "Write BMCR failed\n");
    }

    if (ret)
        return ret;

    if (dev->auto_negotiation) {
        ssdk_printf(SSDK_EMERG, "Wait for dp83848 link ...\n");
        while (timeout--) {
            reg_val = dp83848_read(dev, MII_BMSR);
            if ((reg_val >= 0) && (reg_val & BMSR_LINK_STATUS))
                break;

            udelay(1);
        }

        if ((reg_val < 0) || !(reg_val & BMSR_LINK_STATUS)) {
            ssdk_printf(SSDK_EMERG, "Link failed\n");
            return -1;
        }
        else
            ssdk_printf(SSDK_EMERG, "Link successfully\n");
    }

    return 0;
}

static int dp83848_startup(phy_dev_t *dev)
{
    int reg_val, ret;

    reg_val = dp83848_read(dev, MII_PHYSTS);
    if (reg_val < 0) {
        ssdk_printf(SSDK_ERR, "Read PHYSTS failed\n");
        return reg_val;
    }

    ssdk_printf(SSDK_INFO, "PHYSTS value 0x%x\n", reg_val);
    if (reg_val & PHYSTS_SPEED)
        dev->speed = PHY_SPEED_10;
    else
        dev->speed = PHY_SPEED_100;
    if (reg_val & PHYSTS_DUPLEX)
        dev->duplex_mode = ETH_PHY_DUPLEX_MODE_FULL;
    else
        dev->duplex_mode = ETH_PHY_DUPLEX_MODE_HALF;

    reg_val = dp83848_read(dev, MII_BMCR);
    if (reg_val < 0) {
        ssdk_printf(SSDK_ERR, "Read BMCR failed\n");
        return reg_val;
    }

    reg_val &= ~(BMCR_ISOLATE | BMCR_PDOWN);
    ret = dp83848_write(dev, MII_BMCR, reg_val);
    if (ret) {
        ssdk_printf(SSDK_ERR, "Config BMCR failed\n");
        goto out;
    }

    ret = dp83848_write(dev, MII_PHYCR, PHYCR_MDIX_EN | dev->phy_addr);
    if (ret) {
        ssdk_printf(SSDK_ERR, "Config PHYCR failed\n");
        goto out;
    }

    ssdk_printf(SSDK_INFO, "PHY status 0x%x\n", dp83848_read(dev, MII_BMSR));

out:
    return ret;
}

int dp83848_init(phy_dev_t *dev)
{
#if PHY_CLI_EN
    g_dp83848_dev = dev;
#endif

    if (dp83848_reset(dev))
        return -1;

    ssdk_printf(SSDK_INFO, "PHY ID 0x%x\n",
                dp83848_get_phy_id(dev));

    if (dp83848_config_aneg(dev))
        return -2;

    return dp83848_startup(dev);
}

#if PHY_CLI_EN
static int phy_dump(int argc, char *argv[])
{
    if (!g_dp83848_dev) {
        printf("dp83848 is not availiable\n");
        return 0;
    }

    if (argc == 0) {
        printf("phy <reg_num> [val]\n");
        return 0;
    }

    int reg_num = strtoul(argv[0], NULL, 0);
    if (argc == 1)
        printf("read reg %d: 0x%x\n", reg_num, dp83848_read(g_dp83848_dev, reg_num));
    else {
        int val = strtoul(argv[1], NULL, 0);
        printf("write reg %d val 0x%x result %d\n", reg_num, val,
               dp83848_write(g_dp83848_dev, reg_num, val));
    }

    return 0;
}

CLI_CMD("phy", "\r\nphy:\r\n phy reg dump\r\n", phy_dump);
#endif
