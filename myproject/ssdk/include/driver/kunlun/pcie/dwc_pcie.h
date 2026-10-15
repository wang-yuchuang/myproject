/**
 * @file dwc_pcie.h
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: dwc pcie header file.
 *
 * Revision History:
 * -----------------
 */

#ifndef DWC_PCIE_H_
#define DWC_PCIE_H_

#define PCIE1 (1)
#define PCIE2 (2)

#define PHY_REFCLK_USE_INTERNAL (0x1)
#define PHY_REFCLK_USE_EXTERNAL (0x2)
#define DIFFBUF_OUT_EN (0x1 << 31)

/* PCIe PHY NCR registers */
#define PCIE_PHY_NCR_CTRL0 (0x0)
#define PCIE_PHY_NCR_CTRL1 (0x4)
#define PCIE_PHY_NCR_CTRL4 (0x10)
#define PCIE_PHY_NCR_CTRL15 (0x3c)
#define PCIE_PHY_NCR_STS0 (0x80)

/* info in PCIe PHY NCR registers */
#define CR_ADDR_MODE_BIT    (0x1 << 29)
#define BIF_EN_BIT  (0x1 << 1)
#define PHY_REF_USE_PAD_BIT (0x1 << 5)
#define PHY_REF_ALT_CLK_SEL_BIT (0x1 << 8)
#define PHY_RESET_BIT (0x1 << 0)
#define CR_CKEN_BIT (0x1 << 24)

#define PCIE_CAP_PTR_OFFSET (0x34)
#define PCI_CAP_ID_EXP   (0x10) /* PCI Express */

/* PCIe ATU registers */
#define PCIE_ATU_BASE (0xc0000)
#define PCIE_ATU_REGION_CTRL_1 (0X0)
#define PCIE_ATU_REGION_CTRL_2 (0X4)
#define PCIE_ATU_LWR_BASE_ADDR (0x8)
#define PCIE_ATU_UPPER_BASE_ADDR (0xC)
#define PCIE_ATU_LIMIT_ADDR (0x10)
#define PCIE_ATU_LWR_TARGET_ADDR (0x14)
#define PCIE_ATU_UPPER_TARGET_ADDR (0x18)
#define PCIE_ATU_UPPER_LIMIT_ADDR (0x20)

/* info in PCIe ATU registers */
#define PCIE_OBATU_TYPE_BIT     (0x1f << 0);
#define PCIE_OBATU_TYPE_MEM     (0x0 << 0)
#define PCIE_OBATU_TYPE_IO      (0x2 << 0)
#define PCIE_OBATU_TYPE_CFG0        (0x4 << 0)
#define PCIE_OBATU_ENABLE_BIT (0x1 << 31)
#define PCIE_OBATU_DMA_BYPASS_BIT       (0x1 << 27)
#define PCIE_OBATU_CFG_SHIFT_MODE   (0x1 << 28)
#define PCIE_OBATU_INCREASE_REGION_SIZE (0x1 << 13)

#define PCIE_IBATU_TYPE_BIT     (0x1f << 0)
#define PCIE_IBATU_TYPE_MEM     (0x0 << 0)
#define PCIE_IBATU_ENABLE_BIT           (0x1 << 31)
#define PCIE_IBATU_MATCH_MODE_BIT   (0x1 << 30)
#define PCIE_IBATU_CFG_SHIFT_MODE_BIT   (0x1 << 28)
#define PCIE_IBATU_BAR_NUMBER_BIT (0x3 << 8)

/* PCIe CTRL NCR registers */
#define PCIE_CTRL_NCR_INTR0 (0x0)
#define PCIE_CTRL_NCR_INTEN0 (0x34)
#define PCIE_CTRL_NCR_INTEN1 (0x38)
#define PCIE_CTRL_NCR_INTEN2 (0x3c)
#define PCIE_CTRL_NCR_INTEN3 (0x40)
#define PCIE_CTRL_NCR_INTEN4 (0x44)
#define PCIE_CTRL_NCR_INTEN5 (0x48)
#define PCIE_CTRL_NCR_INTEN6 (0x4c)
#define PCIE_CTRL_NCR_INTEN7 (0x50)
#define PCIE_CTRL_NCR_INTEN8 (0x54)
#define PCIE_CTRL_NCR_INTEN9 (0x58)
#define PCIE_CTRL_NCR_INTEN10 (0x5c)
#define PCIE_CTRL_NCR_INTEN11 (0x60)
#define PCIE_CTRL_NCR_INTEN12 (0x64)
#define PCIE_CTRL_NCR_CTRL0 (0x68)
#define PCIE_CTRL_NCR_CTRL2 (0x70)
#define PCIE_CTRL_NCR_CTRL6 (0x80)

#define PCIE_CTRL_NCR_CTRL21 (0xbc)
#define PCIE_CTRL_NCR_CTRL22 (0xc0)
#define PCIE_CTRL_NCR_CTRL23 (0xc4)
#define PCIE_CTRL_NCR_CTRL24 (0xc8)
#define PCIE_CTRL_NCR_STS0 (0x200)

/* info in PCIe CTRL NCR registers */
#define APP_LTSSM_EN_BIT (0x1 << 2)
#define DEVICE_TYPE_BIT (0x1 << 3)
#define APP_HOLD_PHY_RST_BIT (0x1 << 6)
#define INTR_SMLH_LINK_UP (0x1 << 28)
#define INTR_RDLH_LINK_UP (0x1 << 27)
#define SMLH_LTSSM_STATE_MASK (0x3f)
#define SMLH_LTSSM_STATE_SHIFT (1)

/* Synopsys-specific PCIe configuration registers */
#define PCIE_TYPE1_STATUS_COMMAND   (0x4)
#define PCIE_TYPE1_MEM_LIMIT_MEM_BASE_REG (0X20)
#define PCIE_PORT_LINK_CONTROL      (0x710)
#define PCIE_LINK_WIDTH_SPEED_CONTROL   (0x80C)

/* info in Synopsys-specific PCIe configuration registers */
#define PORT_LINK_MODE_MASK     (0x3f << 16)
#define PORT_LINK_MODE_1_LANES      (0x1 << 16)
#define PORT_LINK_MODE_2_LANES      (0x3 << 16)
#define PORT_LINK_MODE_4_LANES      (0x7 << 16)
#define PORT_LINK_MODE_8_LANES      (0xf << 16)

#define PORT_LOGIC_LINK_WIDTH_MASK  (0x1f << 8)
#define PORT_LOGIC_LINK_WIDTH_1_LANES   (0x1 << 8)
#define PORT_LOGIC_LINK_WIDTH_2_LANES   (0x2 << 8)
#define PORT_LOGIC_LINK_WIDTH_4_LANES   (0x4 << 8)
#define PORT_LOGIC_LINK_WIDTH_8_LANES   (0x8 << 8)

#define  PCI_COMMAND_IO     (0x1)
#define  PCI_COMMAND_MEMORY (0x2)
#define  PCI_COMMAND_MASTER (0x4)

/* PCIe EP shadow CDM registers */
#define PCIE_EP_TYPE0_HDR_DBI2 (0x1000)
#define PCIE_EP_BAR0_MASK (0x10)
#define PCIE_EP_BAR1_MASK (0x14)
#define PCIE_EP_BAR2_MASK (0x18)
#define PCIE_EP_BAR3_MASK (0x1c)
#define PCIE_EP_BAR4_MASK (0x20)
#define PCIE_EP_BAR5_MASK (0x24)

/* PCIe Spec registers */
#define PCIE_TYPE0_DEVICE_ID_VENDOR_ID_REG (0X0)
#define PCIE_TYPE0_CLASS_CODE_REVISION_ID_REG (0x8)


/* PCI Express Capability Structure */
#define PCIE_LINK_CAPABILITIES_REG (0xc)
#define PCIE_LINK_CONTROL_LINK_STATUS_REG (0X10)
#define PCIE_LINK_CONTROL2_LINK_STATUS2_REG (0x30)

/* info in PCI Express Capability Structure registers */
#define PCIE_CAP_MAX_LINK_WIDTH_MASK    (0x3f << 4)
#define PCIE_CAP_MAX_LINK_WIDTH_1_LANES (0x1 << 4)
#define PCIE_CAP_MAX_LINK_WIDTH_2_LANES (0x2 << 4)
#define PCIE_CAP_MAX_LINK_WIDTH_4_LANES (0x4 << 4)
#define PCIE_CAP_MAX_LINK_WIDTH_8_LANES (0x8 << 4)

#define PCIE_CAP_TARGET_LINK_SPEED_MASK (0xf)
#define PCIE_CAP_TARGET_LINK_SPEED_GEN1 0x1
#define PCIE_CAP_TARGET_LINK_SPEED_GEN2 0x2
#define PCIE_CAP_TARGET_LINK_SPEED_GEN3 0x3

#define PCIE_CAP_LINK_SPEED_SHIFT (16)
#define PCIE_CAP_LINK_SPEED_MASK (0xf << PCIE_CAP_LINK_SPEED_SHIFT)

#define PCIE_MSI_CTRL (0x50)
#define PCIE_MSI_ADDR (0x54)
#define PCIE_MSI_UPPER_ADDR (0x58)

#define PCIE_MSI_VECTOR_SHIFT (17)
#define PCIE_MSI_VECTOR_MASK (0x7 << PCIE_MSI_VECTOR_SHIFT)
#define PCIE_MSI_VECTOR_16 (0x4 << PCIE_MSI_VECTOR_SHIFT)
#define PCIE_MSI_VECTOR_32 (0x5 << PCIE_MSI_VECTOR_SHIFT)

/* DMA */
#define PCIE_DMA_OFFSET (0xc8000)

#define  DMA_WRITE_ENGINE_EN (0XC)
#define  DMA_WRITE_DOORBELL (0x10)
#define  DMA_WRITE_INT_STS (0X4c)
#define  DMA_WRITE_INT_MASK (0X54)
#define  DMA_WRITE_INT_CLR (0X58)

#define  DMA_CTRL1_WRCH (0x0)
#define  DMA_SIZE_WRCH (0x8)
#define  DMA_SRC_LOW_WRCH (0xc)
#define  DMA_SRC_HI_WRCH (0x10)
#define  DMA_DST_LOW_WRCH (0x14)
#define  DMA_DST_HI_WRCH (0x18)

#define  DMA_READ_ENGINE_EN (0X2C)
#define  DMA_READ_DOORBELL (0x30)
#define  DMA_READ_INT_STS (0Xa0)
#define  DMA_READ_INT_MASK (0Xa8)
#define  DMA_READ_INT_CLR (0Xac)

#define  DMA_CTRL1_RDCH (0x0)
#define  DMA_SIZE_RDCH (0x8)
#define  DMA_SRC_LOW_RDCH (0xc)
#define  DMA_SRC_HI_RDCH (0x10)
#define  DMA_DST_LOW_RDCH (0x14)
#define  DMA_DST_HI_RDCH (0x18)

#ifndef ASSEMBLY

#include <stdint.h>
#include <compiler.h>
#include "pcie.h"

typedef struct kunlun_pcie {
    int pcie_index;
    int phy_refclk_sel;
    uint32_t pcie_cap;
    uint32_t ctrl_ncr_base;
    uint32_t phy_ncr_base;
    uint32_t phy_base;
    uint32_t dbi;
    uint64_t ecam_base;
    int link_width;
    int link_speed;
    unsigned long dma_base;
} kunlun_pcie_t;

typedef struct dwc_pcie_local_bar_cfg {
    pcie_bar_addr_type_t addr_type;
    bool prefetch;
    uint32_t size;
    uint32_t target;
    uint32_t is_io;
} dwc_pcie_local_bar_cfg_t;

typedef struct dwc_pcie_cfg {
    int controller;
    uint32_t ctrl_ncr_base;
    uint32_t phy_ncr_base;
    uint32_t phy_base;
    uint32_t dbi;
    uint64_t ecam_base;
    pcie_mode_t mode;
    bool refclk_use_internal;
    bool diffbuf_on;
    pcie_link_width_t width;
    pcie_link_speed_t speed;
    unsigned long dma_base;
    /* Bar size should be configured to non-zero
     * value if the bar is used, otherwise the
     * bar is disabled.
     */
    dwc_pcie_local_bar_cfg_t bar[6];
} dwc_pcie_cfg_t;

__BEGIN_CDECLS

void dwc_pcie_probe(pcie_dev_t *dev, dwc_pcie_cfg_t *cfg);

__END_CDECLS

#endif // !ASSEMBLY

#endif // !DWC_PCIE_H_

