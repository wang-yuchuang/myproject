/*
 * yt8521.c
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: eth phy yt8521 drv
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

#define MDIO_DEVAD_NONE  -1
#define REG_DEBUG_ADDR_OFFSET          0x1e
#define REG_DEBUG_DATA                 0x1f
#define YT8521_EXTREG_SLEEP_CONTROL1   0x27
#define YT8521_EN_SLEEP_SW_BIT         15

#if PHY_CLI_EN
static phy_dev_t *g_yt8521_dev;
#endif

static int yt8521_read(phy_dev_t *dev, uint32_t regaddr)
{
    phy_bus_t *bus = dev->bus;
    int ret = bus->ops->mdio_read(bus, dev->phy_addr, 0xFF, regaddr);
    if (ret < 0)
        ssdk_printf(SSDK_ERR, "yt8521_read err!\n");

    return ret;
}

static int yt8521_write(phy_dev_t *dev, uint32_t regaddr, uint16_t val)
{
    phy_bus_t *bus = dev->bus;
    if (bus->ops->mdio_write(bus, dev->phy_addr, 0xFF, regaddr, val)) {
        ssdk_printf(SSDK_ERR, "yt8521_write err!\n");
        return -1;
    }

    return 0;
}

static int ytphy_read_ext(phy_dev_t *dev, uint32_t regnum)
{
	int ret;
	int val;

	ret = yt8521_write(dev, REG_DEBUG_ADDR_OFFSET, regnum);
	if (ret < 0)
		return ret;

	val = yt8521_read(dev, REG_DEBUG_DATA);

	return val;
}

static int ytphy_write_ext(phy_dev_t *dev, uint32_t regnum, uint16_t val)
{
	int ret;

	ret = yt8521_write(dev, REG_DEBUG_ADDR_OFFSET, regnum);
	if (ret < 0)
		return ret;

	ret = yt8521_write(dev, REG_DEBUG_DATA, val);

	return ret;
}

static int yt8521_initial_config(phy_dev_t *dev)
{
	int ret;
	int val;

	/* disable auto sleep */
	val = ytphy_read_ext(dev, YT8521_EXTREG_SLEEP_CONTROL1);
	if (val < 0)
		return val;

	val &= (~BIT(YT8521_EN_SLEEP_SW_BIT));

	ret = ytphy_write_ext(dev, YT8521_EXTREG_SLEEP_CONTROL1, val);
	if (ret < 0)
		return ret;

	/* enable RXC clock when no wire plug */
	ret = ytphy_write_ext(dev, 0xa000, 0);
	if (ret < 0)
		return ret;

	val = ytphy_read_ext(dev, 0xc);
	if (val < 0)
		return val;
	val &= ~(1 << 12);
	ret = ytphy_write_ext(dev, 0xc, val);
	if (ret < 0)
		return ret;

	/* enable RX delay */
	val = ytphy_read_ext(dev, 0xa001);
	if(0 == (val & 0x100) ){
		ret = ytphy_write_ext(dev, 0xa001, val | 0x100);
		if (ret < 0)
			return ret;
		}
	/* config TX delay */
	val = ytphy_read_ext(dev, 0xa003);

	ret =  ytphy_write_ext(dev, 0xa003, val | 0x3);
		if (ret < 0)
			return ret;

	return ret;
}

static int yt8521_initial_with_nway_config(phy_dev_t *dev)
{
    int mdio_data = 0;

    while (mdio_data != 0x0140) {// Check soft-reset complete
        mdio_data = yt8521_read(dev, 0);
    }

    return 0;
}

static int yt8521_get_link_status(phy_dev_t *dev)
{
	return 0;
}

int yt8521_init(phy_dev_t *dev)
{
    int ret = 0, timeout = 1000;

#if PHY_CLI_EN
    g_yt8521_dev = dev;
#endif

 ssdk_printf(SSDK_EMERG, "@@@yt8521_init\n");
    if (dev->auto_negotiation)
        ret = yt8521_initial_with_nway_config(dev);
    else
        ret = yt8521_initial_config(dev);

    if (!ret && dev->auto_negotiation) {
        ssdk_printf(SSDK_EMERG, "Wait for yt8521 link ...\n");
        while (timeout--) {
            ret = yt8521_get_link_status(dev);
            if (ret == LINK_UP) {
                ssdk_printf(SSDK_EMERG, "Link successfully\n");
                return ret;
            }
        }
        ssdk_printf(SSDK_EMERG, "Link failed\n");
    }

    return ret;
}

