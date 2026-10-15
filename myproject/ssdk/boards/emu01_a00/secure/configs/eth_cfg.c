#ifdef CONFIG_ETH

#include <stdbool.h>
#include "reg.h"
#include "regs_base.h"
#include "irq_num.h"
#include "scr_hw.h"

#include "scr_cfg.h"

#include "eth_cfg.h"
#include "driver/kunlun/eth/dwc_eth.h"
#include "driver/kunlun/eth/phy.h"

#define PHY_INTF_SEL    0x2080

extern int rtl9010_init(phy_dev_t *dev);
extern int dp83848_init(phy_dev_t *dev);
extern int yt8521_init(phy_dev_t *dev);

#ifdef SUPPORT_PHYIF_AUTO_NEGOTIATE
static void set_phyif_mode(uint32_t base, eth_phy_intf_mode_type mode)
{
    uint32_t v;

    /* Config MII interface. */
    if (mode == ETH_PHY_INTF_SEL_MII)
        v = 0;
    else if (mode == ETH_PHY_INTF_SEL_RMII) {
        if (base == APB_ENET_QOS1_BASE) {
            const scr_signal_t rmii_oe = SCR_SF_SCR_ENET1_CLK_RMII_OE;
            scr_set(&g_scr_ctrl, &rmii_oe, 1);
        }
        else {
            const scr_signal_t rmii_oe = SCR_SF_SCR_ENET2_CLK_RMII_OE;
            scr_set(&g_scr_ctrl, &rmii_oe, 1);
        }

        v = 4;
    }
    else
        v = 1;

    writel(v, base + PHY_INTF_SEL);
}
#endif


static phy_dev_t g_phy_arr1[] = {
    {
        .phy_addr = 0,
        .duplex_mode = ETH_PHY_DUPLEX_MODE_FULL,
        .speed = PHY_SPEED_1000,
        .auto_negotiation = false,

	.init = yt8521_init
    },

    /* Invalid phy dev to indicate end of array. */
    {
        .phy_addr = ~0,
    }
};

#ifdef  PHY_DP83848
static phy_dev_t g_phy_arr2[] = {
    {
        .phy_addr = 4,
        .duplex_mode = ETH_PHY_DUPLEX_MODE_FULL,
        .speed = PHY_SPEED_100,
        .auto_negotiation = false,
        .init = dp83848_init
    },

    /* Invalid phy dev to indicate end of array. */
    {
        .phy_addr = ~0,
    }
};
#endif

#ifdef  PHY_rtl9010
static phy_dev_t g_phy_arr3[] = {
    {
        .phy_addr = 0,
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
#endif

static dwc_eth_config_t g_eth1_cfg = {
    .base = APB_ETH_QOS1_BASE,
    .irq_num = ETH1_SBD_INTR_NUM,
    .mtu = 1500,
    //.dma_bus_width = 64,
    .dma_bus_width = 128,
    .phy_intf_mode = ETH_PHY_INTF_SEL_RGMII,
    .set_phy_intf = NULL,
    .ip = {10, 18, 250, 50},
    .mask = {255, 255, 255, 0},

    .phy = g_phy_arr1
};

#ifdef  PHY_DP83848
static uint8_t g_eth2_mac_addr[6] = {0x00, 0x1f, 0x29, 0x00, 0x12, 0x16};
static dwc_eth_config_t g_eth2_cfg = {
    .base = APB_ENET_QOS1_BASE,
    .irq_num = ENET2_SBD_INTR_NUM,
    .mtu = 1500,
    .mac_addr = g_eth2_mac_addr,
    .dma_bus_width = 64,
    .phy_intf_mode = ETH_PHY_INTF_SEL_RMII,
    .set_phy_intf = set_phyif_mode,
    .ip = {10, 18, 252, 70},
    .mask = {255, 255, 255, 0},

    .phy = g_phy_arr2
};
#endif
#ifdef  PHY_rtl9010
static dwc_eth_config_t g_eth3_cfg = {
    .base = APB_ENET_QOS1_BASE,
    .irq_num = ENET1_SBD_INTR_NUM,
    .mtu = 1500,
    //.dma_bus_width = 64,
    .dma_bus_width = 128,
    .phy_intf_mode = ETH_PHY_INTF_SEL_RGMII,
    .set_phy_intf = NULL,
    .ip = {10, 18, 250, 50},
    .mask = {255, 255, 255, 0},

    .phy = g_phy_arr3
};
#endif

void board_eth_init(void)
{
    dwc_eth_probe(&g_eth1_cfg);
    
}

#endif
