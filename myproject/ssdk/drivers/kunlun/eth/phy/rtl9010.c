/*
 * rtl9010.c
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: eth phy rtl9010 drv
 *
 * Revision History:
 * -----------------
 */

#include <stdlib.h>
#include "debug.h"
#include "CLI.h"
#include "board.h"
#include "driver/kunlun/eth/phy.h"

#define LINK_UP 0
#define LINK_DOWN   -100

#define PHY_CLI_EN  1

#if PHY_CLI_EN
static phy_dev_t *g_rtl9010_dev;
#endif

static int rtl9010_read(phy_dev_t *dev, uint32_t regaddr)
{
    phy_bus_t *bus = dev->bus;
    int ret = bus->ops->mdio_read(bus, dev->phy_addr, 0xFF, regaddr);
    if (ret < 0)
        ssdk_printf(SSDK_ERR, "rtl9010_read err!\n");

    return ret;
}

static int rtl9010_write(phy_dev_t *dev, uint32_t regaddr, uint16_t val)
{
    phy_bus_t *bus = dev->bus;
    if (bus->ops->mdio_write(bus, dev->phy_addr, 0xFF, regaddr, val)) {
        ssdk_printf(SSDK_ERR, "rtl9010_write err!\n");
        return -1;
    }

    return 0;
}

static int rtl9010_initial_config(phy_dev_t *dev)
{
    int mdio_data = 0;
    int timer = 2000; // set a 2ms timer

    // PHY Parameter Start //
    rtl9010_write(dev, 31, 0x0BC4);
    rtl9010_write(dev, 21, 0x16FE);

    rtl9010_write(dev, 27, 0xB820);
    rtl9010_write(dev, 28, 0x0010);
    rtl9010_write(dev, 27, 0xB830);
    rtl9010_write(dev, 28, 0x8000);
    rtl9010_write(dev, 27, 0xB800);

    mdio_data = (rtl9010_read(dev, 28) & 0x0040);
    while (mdio_data != 0x0040) {
        rtl9010_write(dev, 27, 0xB800);
        mdio_data = (rtl9010_read(dev, 28) & 0x0040);
        timer--;
        if (timer == 0) {
            return -1;
        }
    }

    rtl9010_write(dev, 27, 0x8020);
    rtl9010_write(dev, 28, 0x9100);
    rtl9010_write(dev, 27, 0xB82E);
    rtl9010_write(dev, 28, 0x0001);

    rtl9010_write(dev, 27, 0xB820);
    rtl9010_write(dev, 28, 0x0290);
    rtl9010_write(dev, 27, 0xA012);
    rtl9010_write(dev, 28, 0x0000);
    rtl9010_write(dev, 27, 0xA014);
    rtl9010_write(dev, 28, 0xD700);
    rtl9010_write(dev, 28, 0x880F);
    rtl9010_write(dev, 28, 0x262D);
    rtl9010_write(dev, 27, 0xA01A);
    rtl9010_write(dev, 28, 0x0000);
    rtl9010_write(dev, 27, 0xA000);
    rtl9010_write(dev, 28, 0x162C);
    rtl9010_write(dev, 27, 0xB820);
    rtl9010_write(dev, 28, 0x0210);
    rtl9010_write(dev, 27, 0xB82E);
    rtl9010_write(dev, 28, 0x0000);
    rtl9010_write(dev, 27, 0x8020);
    rtl9010_write(dev, 28, 0x0000);
    rtl9010_write(dev, 27, 0xB820);
    rtl9010_write(dev, 28, 0x0000);

    rtl9010_write(dev, 27, 0xB800);
    mdio_data = rtl9010_read(dev, 28) & 0x0040;
    timer = 2000; // set a 2ms timer
    while (mdio_data != 0x0000) {
        rtl9010_write(dev, 27, 0xB800);
        mdio_data = (rtl9010_read(dev, 28) & 0x0040);
        timer--;
        if (timer == 0) {
            return -2;
        }
    }
    // End //

    /* Set rx delay to 2ns. */
    rtl9010_write(dev, 27, 0xD04A);
    rtl9010_write(dev, 28, 7);

    /* Set tx delay to 2ns. */
    rtl9010_write(dev, 27, 0xD084);
    rtl9010_write(dev, 28, 0x4007);

    rtl9010_write(dev, 0, 0x8000);	// PHY soft-reset
    mdio_data = 0;
    while (mdio_data != 0x0140) {	// Check soft-reset complete
        mdio_data = rtl9010_read(dev, 0);
    }

    return 0;
}

static int rtl9010_initial_with_nway_config(phy_dev_t *dev)
{
    int mdio_data = 0;
    int timer = 2000; // set a 2ms timer

    // PHY Parameter Start //
    rtl9010_write(dev, 31,0x0A54);
    rtl9010_write(dev, 21,0xFA06);
    rtl9010_write(dev, 31,0x0BC4);
    rtl9010_write(dev, 21,0x16FE);

    rtl9010_write(dev, 27, 0xB820);
    rtl9010_write(dev, 28, 0x0010);
    rtl9010_write(dev, 27, 0xB830);
    rtl9010_write(dev, 28, 0x8000);
    rtl9010_write(dev, 27, 0xB800);
    mdio_data = (rtl9010_read(dev, 28) & 0x0040);
    while (mdio_data != 0x0040) {
        rtl9010_write(dev, 27, 0xB800);
        mdio_data = (rtl9010_read(dev, 28) & 0x0040);
        timer--;
        if (timer == 0) {
            return -1;
        }
    }

    rtl9010_write(dev, 27, 0x8020);
    rtl9010_write(dev, 28, 0x9100);
    rtl9010_write(dev, 27, 0xB82E);
    rtl9010_write(dev, 28, 0x0001);

    rtl9010_write(dev, 27, 0xB820);
    rtl9010_write(dev, 28, 0x0290);
    rtl9010_write(dev, 27, 0xA012);
    rtl9010_write(dev, 28, 0x0000);
    rtl9010_write(dev, 27, 0xA014);
    rtl9010_write(dev, 28, 0x2C03);
    rtl9010_write(dev, 28, 0x2C07);
    rtl9010_write(dev, 28, 0x2C0B);
    rtl9010_write(dev, 28, 0x6054);
    rtl9010_write(dev, 28, 0xA701);
    rtl9010_write(dev, 28, 0xD500);
    rtl9010_write(dev, 28, 0x2108);
    rtl9010_write(dev, 28, 0x4054);
    rtl9010_write(dev, 28, 0x8701);
    rtl9010_write(dev, 28, 0xA74A);
    rtl9010_write(dev, 28, 0x20DE);
    rtl9010_write(dev, 28, 0xD700);
    rtl9010_write(dev, 28, 0x880F);
    rtl9010_write(dev, 28, 0x262D);
    rtl9010_write(dev, 27, 0xA01A);
    rtl9010_write(dev, 28, 0x0000);
    rtl9010_write(dev, 27, 0xA004);
    rtl9010_write(dev, 28, 0x062C);
    rtl9010_write(dev, 27, 0xA002);
    rtl9010_write(dev, 28, 0x00DD);
    rtl9010_write(dev, 27, 0xA000);
    rtl9010_write(dev, 28, 0x7107);
    rtl9010_write(dev, 27, 0xB820);
    rtl9010_write(dev, 28, 0x0210);

    rtl9010_write(dev, 27, 0xB82E);
    rtl9010_write(dev, 28, 0x0000);
    rtl9010_write(dev, 27, 0x8020);
    rtl9010_write(dev, 28, 0x0000);
    rtl9010_write(dev, 27, 0xB820);
    rtl9010_write(dev, 28, 0x0000);

    rtl9010_write(dev, 27, 0xB800);
    mdio_data = (rtl9010_read(dev, 28) & 0x0040);
    timer = 2000; // set a 2ms timer
    while (mdio_data != 0x0000) {
        rtl9010_write(dev, 27, 0xB800);
        mdio_data = (rtl9010_read(dev, 28) & 0x0040);
        timer--;
        if (timer == 0) {
            return -2;
        }
    }
    // End //

    /* Set rx delay to 2ns. */
    rtl9010_write(dev, 27, 0xD04A);
    rtl9010_write(dev, 28, 7);

    /* Set tx delay to 2ns. */
    rtl9010_write(dev, 27, 0xD084);
    rtl9010_write(dev, 28, 0x4007);

    rtl9010_write(dev, 0, 0x8000);	// PHY soft-reset
    mdio_data = 0;
    while (mdio_data != 0x0140) {	// Check soft-reset complete
        mdio_data = rtl9010_read(dev, 0);
    }

    return 0;
}

static int rtl9010_get_link_status(phy_dev_t *dev)
{
    int mdio_data = 0;
    int locok = 0;
    int remok = 0;
    int link_status = 0;
    int pcs_status;
    int pcs_status_checkOK=0;

    mdio_data = rtl9010_read(dev, 1);
    mdio_data = rtl9010_read(dev, 1);//Read twice for current link status.
    link_status = (mdio_data & 0x0004);

    rtl9010_write(dev, 31, 0x0A60);
    mdio_data = rtl9010_read(dev, 16);
    pcs_status = (mdio_data & 0x00FF);

    if (dev->speed == PHY_SPEED_1000){
        mdio_data = rtl9010_read(dev, 10);
        locok = ((mdio_data & 0x2000) == 0x2000);
        remok = ((mdio_data & 0x1000) == 0x1000);
        if (pcs_status == 0x0037)
            pcs_status_checkOK = 1;
    }
    else {
        rtl9010_write(dev, 31, 0x0A64);
        mdio_data = rtl9010_read(dev, 23);
        locok = ((mdio_data & 0x0004) == 0x0004);
        remok = ((mdio_data & 0x0400) == 0x0400);
        if (pcs_status == 0x0044)
            pcs_status_checkOK = 1;
    }

    ssdk_printf(SSDK_INFO, "link_status = 0x%04X\r\n",link_status);
    ssdk_printf(SSDK_INFO, "locok = 0x%04X\r\n",locok);
    ssdk_printf(SSDK_INFO, "remok = 0x%04X\r\n",remok);
    ssdk_printf(SSDK_INFO, "pcs_status = 0x%04X\r\n",pcs_status);

    if ((link_status == 0x0004) & (locok) & (remok) &
        pcs_status_checkOK)
        return LINK_UP;
    else
        return LINK_DOWN;
}

int rtl9010_init(phy_dev_t *dev)
{
    int ret = 0, timeout = 1000;

#if PHY_CLI_EN
    g_rtl9010_dev = dev;
#endif

    if (dev->auto_negotiation)
        ret = rtl9010_initial_with_nway_config(dev);
    else
        ret = rtl9010_initial_config(dev);

    if (!ret && dev->auto_negotiation) {
        ssdk_printf(SSDK_EMERG, "Wait for rtl9010 link ...\n");
        while (timeout--) {
            ret = rtl9010_get_link_status(dev);
            if (ret == LINK_UP) {
                ssdk_printf(SSDK_EMERG, "Link successfully\n");
                return ret;
            }
        }
        ssdk_printf(SSDK_EMERG, "Link failed\n");
    }

    return ret;
}

#if PHY_CLI_EN
static int rtl9010_dump(int argc, char *argv[])
{
    if (!g_rtl9010_dev) {
        printf("rtl9010 is not availiable\n");
        return 0;
    }

    if (argc == 0) {
        printf("phy <reg_num> [val]\n");
        return 0;
    }

    int reg_num = strtoul(argv[0], NULL, 0);
    if (argc == 1)
        printf("read reg %d: 0x%x\n", reg_num, rtl9010_read(g_rtl9010_dev, reg_num));
    else {
        int val = strtoul(argv[1], NULL, 0);
        printf("write reg %d val 0x%x result %d\n", reg_num, val,
               rtl9010_write(g_rtl9010_dev, reg_num, val));
    }

    return 0;
}

CLI_CMD("rtl", "rtl reg dump", rtl9010_dump);
#endif
