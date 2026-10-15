#ifdef CONFIG_ETH

#include "reg.h"
#include "regs_base.h"
#include "irq_num.h"
#include "board.h"
#include "driver/taishan/pinctrl/taishan_pinctrl.h"
#include "driver/gpio.h"
#include "driver/taishan/eth/dwc_eth.h"
#include "lwip/tcpip.h"

#define PHY_INTF_SEL    0x2080

extern int rtl9010_init(phy_dev_t *dev);

struct net_driver_s g_eth1_dev;

static void set_phyif_mode(uint32_t base, eth_phy_intf_mode_type mode)
{
    uint32_t v;

    /* Config MII interface. */
    if (mode == ETH_PHY_INTF_SEL_MII)
        v = 0;
    else if (mode == ETH_PHY_INTF_SEL_RMII) {
        /* set SCR_ENET2_CLK_RMII_OE to 1. */
        v = readl(APB_SCR_SF_BASE + (0x107c << 0));
        v |= 1 << 9;
        writel(v, APB_SCR_SF_BASE + (0x107c << 0));

        v = 4;
    }
    else
        v = 1;

    writel(v, base + PHY_INTF_SEL);
}

static phy_dev_t g_phy_arr[] = {
    {
        .phy_addr = 1,
        .duplex_mode = ETH_PHY_DUPLEX_MODE_FULL,
        .speed = PHY_SPEED_1000,
        .auto_negotiation = false,
        .init = rtl9010_init
    },

    /* Invalid phy dev to indicate end of array. */
    {
        .phy_addr = ~0,
    }
};

static dwc_eth_config_t g_eth1_cfg = {
    .base = APB_ENET1_BASE,
    .irq_num = ENET1_SBD_INTR_NUM,
    .mtu = 1500,
    .dma_bus_width = 64,
    .phy_intf_mode = ETH_PHY_INTF_SEL_RGMII,
    .set_phy_intf = set_phyif_mode,
    .ip = {10, 18, 252, 50},
    .mask = {255, 255, 255, 0},

    .phy = g_phy_arr
};

void board_eth_init(void)
{
    dwc_eth_probe(&g_eth1_dev, &g_eth1_cfg);
    tcpip_init(NULL, NULL);
}

#endif
