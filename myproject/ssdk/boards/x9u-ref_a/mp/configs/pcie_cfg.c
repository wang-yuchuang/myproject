/*
 * pcie_cfg.c
 *
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: Board level pcie configuration.
 *
 * Revision History:
 * -----------------
 */

#ifdef CONFIG_DWC_PCIE

#include "regs_base.h"
#include "driver/kunlun/pcie/dwc_pcie.h"
#include "pcie_cfg.h"

#define PCIE_RC   1

#define PCIE_PHY_BASE  APB_PCIE_PHY_BASE
#define PCIE_PHY_NCR_BASE (PCIE_PHY_BASE + 0x00010000)
#define PCIE1_DBI_BASE  APB_PCIE1_1_BASE
#define PCIE1_ECAM_BASE  PCIE1_BASE
#define PCIE2_DBI_BASE  APB_PCIE2_1_BASE
#define PCIE2_ECAM_BASE  PCIE2_BASE
#define PCIE1_CTRL_NCR_BASE (PCIE1_DBI_BASE + 0x000e0000)
#define PCIE2_CTRL_NCR_BASE (PCIE2_DBI_BASE + 0x000e0000)
#define PCIE1_MAP_ADDR 0x1c002000

pcie_dev_t g_pcie1_dev, g_pcie2_dev;
#if 1

static const dwc_pcie_cfg_t g_pcie_cfg[] = {
    {
        .controller = PCIE1,
        .ctrl_ncr_base = PCIE1_CTRL_NCR_BASE,
        .phy_ncr_base = PCIE_PHY_NCR_BASE,
        .phy_base = PCIE_PHY_BASE,
        .dbi = PCIE1_DBI_BASE,
        .ecam_base = PCIE1_ECAM_BASE,
    #if PCIE_RC
        .mode = PCIE_RC_MODE,
	.dma_base = PCIE1_MAP_ADDR,
    #else
        .mode = PCIE_EP_MODE,
	.dma_base = PCIE1_DBI_BASE + PCIE_DMA_OFFSET,
    #endif
        .refclk_use_internal = true,
        .diffbuf_on = true,
        .width = LINK_WIDTH_2_LANES,
        .speed = LINK_SPEED_GEN3,
    #if PCIE_RC
        .bar[0].addr_type = BAR_32BIT,
        .bar[0].prefetch = true,
        .bar[0].size = 0x10000000,
        .bar[0].target = 0x80000000,
	.bar[0].is_io = 0,
    #else
	.bar[0].addr_type = BAR_64BIT,
	.bar[0].prefetch = true,
	.bar[0].size = 0x100000,
	.bar[0].target = 0,
	.bar[0].is_io = 0,

        .bar[2].addr_type = BAR_64BIT,
        .bar[2].prefetch = true,
        .bar[2].size = 0x8000000,
        .bar[2].target = 0x30000000,
	.bar[2].is_io = 0,

        .bar[4].addr_type = BAR_32BIT,
        .bar[4].prefetch = true,
        .bar[4].size = 0x4000000,
        .bar[4].target = 0xB0000000,
	.bar[4].is_io = 0,
    #endif
    },
    {
        .controller = PCIE2,
        .ctrl_ncr_base = PCIE2_CTRL_NCR_BASE,
        .phy_ncr_base = PCIE_PHY_NCR_BASE,
        .phy_base = PCIE_PHY_BASE,
        .dbi = PCIE2_DBI_BASE,
        .ecam_base = PCIE2_ECAM_BASE,
        .mode = PCIE_RC_MODE,
        .refclk_use_internal = true,
        .diffbuf_on = true,
        .width = LINK_WIDTH_1_LANE,
        .speed = LINK_SPEED_GEN3,
	.dma_base = 0,
        .bar[0].addr_type = BAR_32BIT,
        .bar[0].prefetch = true,
        .bar[0].size = 0x10000000,
        .bar[0].target = 0x70000000,
	.bar[0].is_io = 0,
    },
};
#endif
void board_pcie_init(void)
{
    dwc_pcie_probe(&g_pcie1_dev, &g_pcie_cfg[0]);
//    dwc_pcie_probe(&g_pcie2_dev, &g_pcie_cfg[1]);
}

#endif
