/**
 * @file dwc_pcie.c
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: dwc pcie high level driver.
 *
 * Revision History:
 * -----------------
 */

#include <debug.h>
#include <reg.h>
#include <cmsis_os2.h>
#include "arch/irq.h"
#include "board.h"
#include "driver/kunlun/pcie/dwc_pcie.h"
#include "image_cfg.h"

#define LINK_AB_PCIE 0x18000000
#define LINK_AB_DDR_A5 (SERVICES_MEMBASE + 0x800000)
//#define LINK_AB_DDR_A5 (0xB8000000)
#define LINK_AB_SIZE 0x04000000

#define lower_32_bits(n) ((uint32_t)(n))
#define upper_32_bits(n) ((uint32_t)(((n) >> 16) >> 16))

#define PCIE_ATU_BUS(x)         (((x) & 0xff) << 24)
#define PCIE_ATU_DEV(x)         (((x) & 0x1f) << 19)
#define PCIE_ATU_FUNC(x)        (((x) & 0x7) << 16)

#define PCIE_GET_OB_ATU_REG(region) ((region) << 9)
#define PCIE_GET_IB_ATU_REG(region) (((region) << 9) | 1 << 8)
#define PCIE_IBATU_SET_BAR_NUMBER(index) (index << 8)

#define enter_critical_area()   state = arch_irq_save()
#define exit_critical_area()    arch_irq_restore(state)

#define OB_CFG_ATU_REGION   0

static int dwc_pcie_establish_link(pcie_dev_t *dev);



static inline void kunlun_phy_ncr_writel(struct kunlun_pcie *kunlun_pcie,
                                         uint32_t val, uint32_t reg)
{
    writel(val, kunlun_pcie->phy_ncr_base + reg);
}

static inline void kunlun_phy_ncr_writel_noprint(struct kunlun_pcie *kunlun_pcie,
                                                 uint32_t val, uint32_t reg)
{
    writel(val, kunlun_pcie->phy_ncr_base + reg);
}

static inline uint32_t kunlun_phy_ncr_readl(struct kunlun_pcie *kunlun_pcie,
                                            uint32_t reg)
{
    return readl(kunlun_pcie->phy_ncr_base + reg);
}

static inline void kunlun_ctrl_ncr_writel(struct kunlun_pcie *kunlun_pcie,
                                          uint32_t val, uint32_t reg)
{
    writel(val, kunlun_pcie->ctrl_ncr_base + reg);
}

static inline uint32_t kunlun_ctrl_ncr_readl(struct kunlun_pcie *kunlun_pcie,
                                             uint32_t reg)
{
    return readl(kunlun_pcie->ctrl_ncr_base + reg);
}

static inline void kunlun_dbi_writel(struct kunlun_pcie *kunlun_pcie,
                                     uint32_t val, uint32_t reg)
{
    writel(val, kunlun_pcie->dbi + reg);
}

static inline uint32_t kunlun_dbi_readl(struct kunlun_pcie *kunlun_pcie,
                                         uint32_t reg)
{
    return readl(kunlun_pcie->dbi + reg);
}

static inline void kunlun_dbi2_writel(struct kunlun_pcie *kunlun_pcie,
                                      uint32_t val, uint32_t reg)
{
    writel(val, kunlun_pcie->dbi + PCIE_EP_TYPE0_HDR_DBI2 + reg);
}

static void kunlun_pcie_phy_refclk_sel(struct kunlun_pcie *kunlun_pcie)
{
    uint32_t reg_val;

    uint32_t flag = kunlun_pcie->phy_refclk_sel & 0x3;
    bool diffbuf_out_en =  !!(kunlun_pcie->phy_refclk_sel >> 31);

    if (flag == PHY_REFCLK_USE_INTERNAL) {
        ssdk_printf(SSDK_INFO, "using internel ref clk\n");
        reg_val = kunlun_phy_ncr_readl(kunlun_pcie, PCIE_PHY_NCR_CTRL1);
       reg_val &= ~PHY_REF_USE_PAD_BIT;
        reg_val &= ~PHY_REF_ALT_CLK_SEL_BIT;
        kunlun_phy_ncr_writel(kunlun_pcie, reg_val, PCIE_PHY_NCR_CTRL1);
    }
    else if (flag == PHY_REFCLK_USE_EXTERNAL) {
        ssdk_printf(SSDK_INFO, "using externel ref clk\n");
        reg_val = kunlun_phy_ncr_readl(kunlun_pcie, PCIE_PHY_NCR_CTRL1);
        reg_val |= PHY_REF_USE_PAD_BIT;
        kunlun_phy_ncr_writel(kunlun_pcie, reg_val, PCIE_PHY_NCR_CTRL1);
    }
    else {
        ssdk_printf(SSDK_EMERG, "error phy_refclk_sel\n");
        ASSERT(0);
    }

    if (diffbuf_out_en) {
        ssdk_printf(SSDK_INFO, "diffbuf out enable\n");
        reg_val = kunlun_phy_ncr_readl(kunlun_pcie, PCIE_PHY_NCR_CTRL15);
        reg_val |= 0x1;
        kunlun_phy_ncr_writel(kunlun_pcie, reg_val, PCIE_PHY_NCR_CTRL15);
    }
}

static void kunlun_pcie_phy_init(struct kunlun_pcie *kunlun_pcie)
{
    uint32_t reg_val;

    kunlun_pcie_phy_refclk_sel(kunlun_pcie);

    reg_val = kunlun_phy_ncr_readl(kunlun_pcie, PCIE_PHY_NCR_CTRL0);
    reg_val |= CR_CKEN_BIT;
    kunlun_phy_ncr_writel(kunlun_pcie, reg_val, PCIE_PHY_NCR_CTRL0);

    reg_val = kunlun_phy_ncr_readl(kunlun_pcie, PCIE_PHY_NCR_CTRL4);
    reg_val &= ~CR_ADDR_MODE_BIT;
    kunlun_phy_ncr_writel(kunlun_pcie, reg_val, PCIE_PHY_NCR_CTRL4);

    reg_val = kunlun_phy_ncr_readl(kunlun_pcie, PCIE_PHY_NCR_CTRL0);
    reg_val |= BIF_EN_BIT;
    kunlun_phy_ncr_writel(kunlun_pcie, reg_val, PCIE_PHY_NCR_CTRL0);

    reg_val = kunlun_phy_ncr_readl(kunlun_pcie, PCIE_PHY_NCR_CTRL0);
    reg_val &= ~PHY_RESET_BIT;
    kunlun_phy_ncr_writel(kunlun_pcie, reg_val, PCIE_PHY_NCR_CTRL0);

    udelay(1000);
    reg_val = kunlun_phy_ncr_readl(kunlun_pcie, PCIE_PHY_NCR_CTRL0);

}

static int kunlun_pcie_find_capability(struct kunlun_pcie *kunlun_pcie, uint32_t cap)
{
    uint32_t val, id;
    int pos;

    val = kunlun_dbi_readl(kunlun_pcie, PCIE_CAP_PTR_OFFSET);
    pos = val & 0xff;

    while (pos) {
        val =  kunlun_dbi_readl(kunlun_pcie, pos);
        id = val & 0xff;

        if (id == cap)
            return pos;

        pos = (val >> 8) & 0xff;
    }

    return 0;
}

static uint32_t kunlun_pcie_get_ltssm_state(struct kunlun_pcie *kunlun_pcie)
{
    uint32_t reg_val;
    reg_val = kunlun_ctrl_ncr_readl(kunlun_pcie, PCIE_CTRL_NCR_STS0);
    reg_val = (reg_val >> SMLH_LTSSM_STATE_SHIFT) & SMLH_LTSSM_STATE_MASK;
    ssdk_printf(SSDK_INFO, "LTSSM STATE = 0x%x\n", reg_val);

    return reg_val;
}

static uint32_t kunlun_pcie_get_link_speed(struct kunlun_pcie *kunlun_pcie)
{
    uint32_t reg_val;
    reg_val = kunlun_dbi_readl(kunlun_pcie,
                               kunlun_pcie->pcie_cap + PCIE_LINK_CONTROL_LINK_STATUS_REG);
    reg_val = (reg_val & PCIE_CAP_LINK_SPEED_MASK) >> PCIE_CAP_LINK_SPEED_SHIFT;
    ssdk_printf(SSDK_INFO, "Link speed = 0x%x\n", reg_val);

    return reg_val;
}

static void kunlun_pcie_enable_ltssm(struct kunlun_pcie *kunlun_pcie)
{
    uint32_t reg_val = kunlun_ctrl_ncr_readl(kunlun_pcie, PCIE_CTRL_NCR_CTRL0);
    reg_val |= APP_LTSSM_EN_BIT;
    kunlun_ctrl_ncr_writel(kunlun_pcie, reg_val, PCIE_CTRL_NCR_CTRL0);
}

static void kunlun_pcie_core_init_rc(struct kunlun_pcie *kunlun_pcie)
{
    uint32_t reg_val, offset;

    kunlun_ctrl_ncr_writel(kunlun_pcie, 0, PCIE_CTRL_NCR_INTEN0);
    kunlun_ctrl_ncr_writel(kunlun_pcie, 0, PCIE_CTRL_NCR_INTEN1);
    kunlun_ctrl_ncr_writel(kunlun_pcie, 0, PCIE_CTRL_NCR_INTEN2);
    kunlun_ctrl_ncr_writel(kunlun_pcie, 0, PCIE_CTRL_NCR_INTEN3);
    kunlun_ctrl_ncr_writel(kunlun_pcie, 0, PCIE_CTRL_NCR_INTEN4);
    kunlun_ctrl_ncr_writel(kunlun_pcie, 0, PCIE_CTRL_NCR_INTEN5);
    kunlun_ctrl_ncr_writel(kunlun_pcie, 0, PCIE_CTRL_NCR_INTEN6);
    kunlun_ctrl_ncr_writel(kunlun_pcie, 0, PCIE_CTRL_NCR_INTEN7);
    kunlun_ctrl_ncr_writel(kunlun_pcie, 0, PCIE_CTRL_NCR_INTEN8);
    kunlun_ctrl_ncr_writel(kunlun_pcie, 0, PCIE_CTRL_NCR_INTEN9);
    kunlun_ctrl_ncr_writel(kunlun_pcie, 0, PCIE_CTRL_NCR_INTEN10);
    kunlun_ctrl_ncr_writel(kunlun_pcie, 0, PCIE_CTRL_NCR_INTEN11);
    kunlun_ctrl_ncr_writel(kunlun_pcie, 0, PCIE_CTRL_NCR_INTEN12);

   reg_val = kunlun_ctrl_ncr_readl(kunlun_pcie, PCIE_CTRL_NCR_CTRL0);
   reg_val &= ~APP_HOLD_PHY_RST_BIT;
   kunlun_ctrl_ncr_writel(kunlun_pcie, reg_val, PCIE_CTRL_NCR_CTRL0);

    reg_val = kunlun_ctrl_ncr_readl(kunlun_pcie, PCIE_CTRL_NCR_CTRL0);
    reg_val |= DEVICE_TYPE_BIT;
    kunlun_ctrl_ncr_writel(kunlun_pcie, reg_val, PCIE_CTRL_NCR_CTRL0);

    reg_val = kunlun_ctrl_ncr_readl(kunlun_pcie, PCIE_CTRL_NCR_CTRL0);


    reg_val = kunlun_ctrl_ncr_readl(kunlun_pcie, PCIE_CTRL_NCR_CTRL6);
    reg_val |= 1 << 30;
    kunlun_ctrl_ncr_writel(kunlun_pcie, reg_val, PCIE_CTRL_NCR_CTRL6);

    if (kunlun_pcie->pcie_index == PCIE2) {
        reg_val = kunlun_ctrl_ncr_readl(kunlun_pcie, PCIE_CTRL_NCR_CTRL2);
        reg_val &= ~0x3F;
        reg_val |= 0x1c;
        kunlun_ctrl_ncr_writel(kunlun_pcie, reg_val, PCIE_CTRL_NCR_CTRL2);
    }

    kunlun_ctrl_ncr_writel(kunlun_pcie, 0, PCIE_CTRL_NCR_CTRL21);
    kunlun_ctrl_ncr_writel(kunlun_pcie, 0, PCIE_CTRL_NCR_CTRL23);

    reg_val = kunlun_dbi_readl(kunlun_pcie, PCIE_PORT_LINK_CONTROL);
    reg_val &= ~PORT_LINK_MODE_MASK;
    reg_val |= ((2 * kunlun_pcie->link_width - 1) << 16);
    kunlun_dbi_writel(kunlun_pcie, reg_val, PCIE_PORT_LINK_CONTROL);

    reg_val = kunlun_dbi_readl(kunlun_pcie, PCIE_LINK_WIDTH_SPEED_CONTROL);
    reg_val &= ~PORT_LOGIC_LINK_WIDTH_MASK;
    reg_val |= (kunlun_pcie->link_width << 8);
    kunlun_dbi_writel(kunlun_pcie, reg_val, PCIE_LINK_WIDTH_SPEED_CONTROL);

    offset = kunlun_pcie_find_capability(kunlun_pcie, PCI_CAP_ID_EXP);

    if (!offset) {
        ssdk_printf(SSDK_EMERG, "%s : didn't find link capabilities!\n", __FUNCTION__);
        ASSERT(0);
    }

    kunlun_pcie->pcie_cap = offset;

    reg_val = kunlun_dbi_readl(kunlun_pcie,
                               offset + PCIE_LINK_CONTROL2_LINK_STATUS2_REG);
    reg_val &= ~(PCIE_CAP_TARGET_LINK_SPEED_MASK);
    reg_val |= kunlun_pcie->link_speed;
    kunlun_dbi_writel(kunlun_pcie, reg_val,
                      offset + PCIE_LINK_CONTROL2_LINK_STATUS2_REG);

    reg_val = kunlun_dbi_readl(kunlun_pcie, PCIE_LINK_CAPABILITIES_REG + offset);
    ssdk_printf(SSDK_DEBUG, "%s: max link width %d\n", __func__,
                (reg_val & PCIE_CAP_MAX_LINK_WIDTH_MASK) >> 4);

    reg_val = kunlun_dbi_readl(kunlun_pcie, PCIE_TYPE1_STATUS_COMMAND);
    reg_val &= 0xffff0000;
    reg_val |= PCI_COMMAND_IO | PCI_COMMAND_MEMORY | PCI_COMMAND_MASTER;
    kunlun_dbi_writel(kunlun_pcie, reg_val, PCIE_TYPE1_STATUS_COMMAND);

}

static void kunlun_pcie_core_init_ep(struct kunlun_pcie *kunlun_pcie)
{
    uint32_t reg_val, offset;

    kunlun_ctrl_ncr_writel(kunlun_pcie, 0, PCIE_CTRL_NCR_INTEN0);
    kunlun_ctrl_ncr_writel(kunlun_pcie, 0, PCIE_CTRL_NCR_INTEN1);
    kunlun_ctrl_ncr_writel(kunlun_pcie, 0, PCIE_CTRL_NCR_INTEN2);
    kunlun_ctrl_ncr_writel(kunlun_pcie, 0, PCIE_CTRL_NCR_INTEN3);
    kunlun_ctrl_ncr_writel(kunlun_pcie, 0, PCIE_CTRL_NCR_INTEN4);
    kunlun_ctrl_ncr_writel(kunlun_pcie, 0, PCIE_CTRL_NCR_INTEN5);
    kunlun_ctrl_ncr_writel(kunlun_pcie, 0, PCIE_CTRL_NCR_INTEN6);
    kunlun_ctrl_ncr_writel(kunlun_pcie, 0, PCIE_CTRL_NCR_INTEN7);
    kunlun_ctrl_ncr_writel(kunlun_pcie, 0, PCIE_CTRL_NCR_INTEN8);
    kunlun_ctrl_ncr_writel(kunlun_pcie, 0, PCIE_CTRL_NCR_INTEN9);
    kunlun_ctrl_ncr_writel(kunlun_pcie, 0, PCIE_CTRL_NCR_INTEN10);
    kunlun_ctrl_ncr_writel(kunlun_pcie, 0, PCIE_CTRL_NCR_INTEN11);
    kunlun_ctrl_ncr_writel(kunlun_pcie, 0, PCIE_CTRL_NCR_INTEN12);

    reg_val = kunlun_ctrl_ncr_readl(kunlun_pcie, PCIE_CTRL_NCR_CTRL0);
    reg_val &= ~APP_HOLD_PHY_RST_BIT;
    kunlun_ctrl_ncr_writel(kunlun_pcie, reg_val, PCIE_CTRL_NCR_CTRL0);

    reg_val = kunlun_ctrl_ncr_readl(kunlun_pcie, PCIE_CTRL_NCR_CTRL0);
    reg_val &= ~DEVICE_TYPE_BIT;
    kunlun_ctrl_ncr_writel(kunlun_pcie, reg_val, PCIE_CTRL_NCR_CTRL0);

    //clear tlpprex, defalult enable.
    kunlun_ctrl_ncr_writel(kunlun_pcie, 0, PCIE_CTRL_NCR_CTRL21);
    kunlun_ctrl_ncr_writel(kunlun_pcie, 0, PCIE_CTRL_NCR_CTRL23);

    reg_val = kunlun_dbi_readl(kunlun_pcie, PCIE_PORT_LINK_CONTROL);
    reg_val &= ~PORT_LINK_MODE_MASK;
    reg_val |= ((2 * kunlun_pcie->link_width - 1) << 16);
    kunlun_dbi_writel(kunlun_pcie, reg_val, PCIE_PORT_LINK_CONTROL);

    reg_val = kunlun_dbi_readl(kunlun_pcie, PCIE_LINK_WIDTH_SPEED_CONTROL);
    reg_val &= ~PORT_LOGIC_LINK_WIDTH_MASK;
    reg_val |= (kunlun_pcie->link_width << 8);
    kunlun_dbi_writel(kunlun_pcie, reg_val, PCIE_LINK_WIDTH_SPEED_CONTROL);

    offset = kunlun_pcie_find_capability(kunlun_pcie, PCI_CAP_ID_EXP);

    if (!offset) {
        ssdk_printf(SSDK_EMERG, "%s : didn't find link capabilities!\n", __FUNCTION__);
        ASSERT(0);
    }

    reg_val = kunlun_dbi_readl(kunlun_pcie,
                               offset + PCIE_LINK_CONTROL2_LINK_STATUS2_REG);
    reg_val &= ~(PCIE_CAP_TARGET_LINK_SPEED_MASK);
    reg_val |= kunlun_pcie->link_speed;
    kunlun_dbi_writel(kunlun_pcie, reg_val,
                      offset + PCIE_LINK_CONTROL2_LINK_STATUS2_REG);

    reg_val = kunlun_dbi_readl(kunlun_pcie, PCIE_LINK_CAPABILITIES_REG + offset);
    ssdk_printf(SSDK_DEBUG, "%s: max link width %d\n", __func__,
                (reg_val & PCIE_CAP_MAX_LINK_WIDTH_MASK) >> 4);
}

static inline void kunlun_atu_ob_writel(struct kunlun_pcie *kunlun_pcie,
                                        uint32_t index, uint32_t val,
                                        uint32_t reg)
{
    uint32_t offset = PCIE_GET_OB_ATU_REG(index);

    writel(val, kunlun_pcie->dbi + PCIE_ATU_BASE + offset + reg);
}

static inline uint32_t kunlun_atu_ob_readl(struct kunlun_pcie *kunlun_pcie,
                                           uint32_t index, uint32_t reg)
{
    uint32_t offset = PCIE_GET_OB_ATU_REG(index);

    return readl(kunlun_pcie->dbi + PCIE_ATU_BASE + offset + reg);
}

static inline void kunlun_atu_ib_writel(struct kunlun_pcie *kunlun_pcie,
                                        uint32_t index, uint32_t val,
                                        uint32_t reg)
{
    uint32_t offset = PCIE_GET_IB_ATU_REG(index);
    writel(val, kunlun_pcie->dbi + PCIE_ATU_BASE + offset + reg);
}

static inline uint32_t kunlun_atu_ib_readl(struct kunlun_pcie *kunlun_pcie,
                                           uint32_t index, uint32_t reg)
{
    uint32_t offset = PCIE_GET_IB_ATU_REG(index);

    return readl(kunlun_pcie->dbi + PCIE_ATU_BASE + offset + reg);
}

static bool kunlun_pcie_link_up(struct kunlun_pcie *kunlun_pcie)
{
    uint32_t reg_val = kunlun_ctrl_ncr_readl(kunlun_pcie, PCIE_CTRL_NCR_INTR0);
    if ((reg_val & INTR_SMLH_LINK_UP) != INTR_SMLH_LINK_UP)
        return false;

    if ((reg_val & INTR_RDLH_LINK_UP) != INTR_RDLH_LINK_UP)
        return false;

    return true;
}

static void kunlun_pcie_config_ob_atu_cfg(struct kunlun_pcie *kunlun_pcie, uint32_t index,
                                          uint32_t bus, uint32_t devnum, uint32_t func)
{
    uint32_t target_addr = PCIE_ATU_BUS(bus) | PCIE_ATU_DEV(devnum) | PCIE_ATU_FUNC(func);
    uint32_t reg_val;

    uint64_t limit_addr = kunlun_pcie->ecam_base + 0x1000 - 1;

    kunlun_atu_ob_writel(kunlun_pcie, index, lower_32_bits(kunlun_pcie->ecam_base),
                         PCIE_ATU_LWR_BASE_ADDR);
    kunlun_atu_ob_writel(kunlun_pcie, index, upper_32_bits(kunlun_pcie->ecam_base),
                         PCIE_ATU_UPPER_BASE_ADDR);
    kunlun_atu_ob_writel(kunlun_pcie, index, lower_32_bits(limit_addr),
                         PCIE_ATU_LIMIT_ADDR);

    kunlun_atu_ob_writel(kunlun_pcie, index, lower_32_bits(target_addr),
                         PCIE_ATU_LWR_TARGET_ADDR);
    kunlun_atu_ob_writel(kunlun_pcie, index, upper_32_bits(target_addr),
                         PCIE_ATU_UPPER_TARGET_ADDR);

    reg_val = kunlun_atu_ob_readl(kunlun_pcie, index, PCIE_ATU_REGION_CTRL_1);
    reg_val &= ~PCIE_OBATU_TYPE_BIT;
    reg_val |= PCIE_OBATU_TYPE_CFG0;
    kunlun_atu_ob_writel(kunlun_pcie, index, reg_val, PCIE_ATU_REGION_CTRL_1);

    reg_val = kunlun_atu_ob_readl(kunlun_pcie, index, PCIE_ATU_REGION_CTRL_2);
    reg_val |= PCIE_OBATU_ENABLE_BIT;
    kunlun_atu_ob_writel(kunlun_pcie, index, reg_val, PCIE_ATU_REGION_CTRL_2);

#if 0
    // set default bus/device/pf number and bar windows.
    writel(0x00ff0100, (kunlun_pcie->dbi + 0x18));
    writel(0x200000f0, (kunlun_pcie->dbi + 0x1c));
    writel(0x1ff01000, (kunlun_pcie->dbi + 0x20));
    writel(0xfff10001, (kunlun_pcie->dbi + 0x24));
    writel(0x00000005, (kunlun_pcie->dbi + 0x28));
    writel(0x00000005, (kunlun_pcie->dbi + 0x2c));
    writel(0x00110107, (kunlun_pcie->dbi + 0x04));
#endif
}

static void kunlun_pcie_disable_ob_atu(struct kunlun_pcie *kunlun_pcie, uint32_t index)
{
    uint32_t reg_val;

    kunlun_atu_ob_writel(kunlun_pcie, index,0, PCIE_ATU_LWR_BASE_ADDR);
    kunlun_atu_ob_writel(kunlun_pcie, index, 0, PCIE_ATU_UPPER_BASE_ADDR);
    kunlun_atu_ob_writel(kunlun_pcie, index, 0, PCIE_ATU_LIMIT_ADDR);

    kunlun_atu_ob_writel(kunlun_pcie, index, 0, PCIE_ATU_LWR_TARGET_ADDR);
    kunlun_atu_ob_writel(kunlun_pcie, index, 0, PCIE_ATU_UPPER_TARGET_ADDR);

    reg_val = kunlun_atu_ob_readl(kunlun_pcie, index, PCIE_ATU_REGION_CTRL_1);
    reg_val &= ~PCIE_OBATU_TYPE_BIT;
    kunlun_atu_ob_writel(kunlun_pcie, index, reg_val, PCIE_ATU_REGION_CTRL_1);

    reg_val = kunlun_atu_ob_readl(kunlun_pcie, index, PCIE_ATU_REGION_CTRL_2);
    reg_val &= ~PCIE_OBATU_ENABLE_BIT;
    kunlun_atu_ob_writel(kunlun_pcie, index, reg_val, PCIE_ATU_REGION_CTRL_2);
}

static void kunlun_pcie_config_ob_atu_mem(struct kunlun_pcie *kunlun_pcie, uint32_t index,
                                          uint64_t base_addr, uint64_t limit_addr,
                                          uint64_t target_addr)
{
    uint64_t size = limit_addr - base_addr + 1;
    uint32_t reg_val;

    if (size > 0xffffffff) {
        ssdk_printf(SSDK_EMERG, "Error ATU region size!!!\n");
        ASSERT(0);
    }

    kunlun_atu_ob_writel(kunlun_pcie, index, lower_32_bits(base_addr),
                         PCIE_ATU_LWR_BASE_ADDR);
    kunlun_atu_ob_writel(kunlun_pcie, index, upper_32_bits(base_addr),
                         PCIE_ATU_UPPER_BASE_ADDR);

    kunlun_atu_ob_writel(kunlun_pcie, index, lower_32_bits(limit_addr),
                         PCIE_ATU_LIMIT_ADDR);

    kunlun_atu_ob_writel(kunlun_pcie, index, lower_32_bits(target_addr),
                         PCIE_ATU_LWR_TARGET_ADDR);

    kunlun_atu_ob_writel(kunlun_pcie, index, upper_32_bits(target_addr),
                         PCIE_ATU_UPPER_TARGET_ADDR);

    reg_val = kunlun_atu_ob_readl(kunlun_pcie, index, PCIE_ATU_REGION_CTRL_1);
    reg_val &= ~PCIE_OBATU_TYPE_BIT;
    reg_val |= PCIE_OBATU_TYPE_MEM;
    kunlun_atu_ob_writel(kunlun_pcie, index, reg_val, PCIE_ATU_REGION_CTRL_1);

    reg_val = kunlun_atu_ob_readl(kunlun_pcie, index, PCIE_ATU_REGION_CTRL_2);
    reg_val |= PCIE_OBATU_ENABLE_BIT;
    kunlun_atu_ob_writel(kunlun_pcie, index, reg_val, PCIE_ATU_REGION_CTRL_2);
}

static void kunlun_pcie_config_ib_atu(struct kunlun_pcie *kunlun_pcie, uint32_t index,
                                      uint64_t target_addr, uint32_t bar_index)
{
    uint32_t reg_val;

    kunlun_atu_ib_writel(kunlun_pcie, index, lower_32_bits(target_addr),
                         PCIE_ATU_LWR_TARGET_ADDR);
    kunlun_atu_ib_writel(kunlun_pcie, index, upper_32_bits(target_addr),
                         PCIE_ATU_UPPER_TARGET_ADDR);

    reg_val = kunlun_atu_ib_readl(kunlun_pcie, index, PCIE_ATU_REGION_CTRL_1);
    reg_val &= ~PCIE_IBATU_TYPE_BIT;
    reg_val |= PCIE_IBATU_TYPE_MEM;
    kunlun_atu_ib_writel(kunlun_pcie, index, reg_val, PCIE_ATU_REGION_CTRL_1);

    reg_val = kunlun_atu_ib_readl(kunlun_pcie, index, PCIE_ATU_REGION_CTRL_2);
    reg_val &= ~PCIE_IBATU_BAR_NUMBER_BIT;
    reg_val |= (PCIE_IBATU_ENABLE_BIT | PCIE_IBATU_MATCH_MODE_BIT |
                PCIE_IBATU_SET_BAR_NUMBER(bar_index));
    kunlun_atu_ib_writel(kunlun_pcie, index, reg_val, PCIE_ATU_REGION_CTRL_2);
}

int kunlun_pcie_set_bar(struct kunlun_pcie *kunlun_pcie, uint32_t bar_index, uint64_t bar_base, uint32_t is_bar64)
{
    uint32_t reg_addr = 0x10 + bar_index * 4;
    uint32_t addr, high_addr;
    uint32_t reg_val;

    if (is_bar64 && ((bar_index % 2) == 1))
        return -1;
    addr = bar_base & 0xfffffff0;
    high_addr = (bar_base >> 32) & 0xffffffff;

    reg_val = readl(kunlun_pcie->ecam_base + reg_addr);
    reg_val &= 0xf;
    reg_val |= addr;
    writel(reg_val, kunlun_pcie->ecam_base + reg_addr);

    if (is_bar64)
        writel(high_addr, kunlun_pcie->ecam_base + reg_addr+4);
    return 0;
}

int kunlun_pcie_enable_all_bar(struct kunlun_pcie *kunlun_pcie)
{
    writel(0x00110002, kunlun_pcie->ecam_base + 0x4);
    return 0;
}

static void kunlun_pcie_rc_mode_init(struct kunlun_pcie *kunlun_pcie)
{
    ASSERT(kunlun_pcie);

    kunlun_pcie_core_init_rc(kunlun_pcie);
}

static void kunlun_pcie_ep_mode_init(struct kunlun_pcie *kunlun_pcie)
{
    uint32_t reg_val;

    ASSERT(kunlun_pcie);

    kunlun_pcie_core_init_ep(kunlun_pcie);

    reg_val = kunlun_dbi_readl(kunlun_pcie, PCIE_MSI_CTRL);
    reg_val &= ~PCIE_MSI_VECTOR_MASK;
    reg_val |= PCIE_MSI_VECTOR_32;
    kunlun_dbi_writel(kunlun_pcie, reg_val,  PCIE_MSI_CTRL);
}

static void kunlun_pcie_ep_set_class_code(struct kunlun_pcie *kunlun_pcie, uint32_t class_code)
{
    uint32_t reg_val = kunlun_dbi_readl(kunlun_pcie, PCIE_TYPE0_CLASS_CODE_REVISION_ID_REG);
    reg_val &= ~(0xffffff << 8);
    reg_val |= (class_code & 0xffffff) << 8;
    kunlun_dbi_writel(kunlun_pcie, reg_val, PCIE_TYPE0_CLASS_CODE_REVISION_ID_REG);
}

static void kunlun_pcie_config_local_bar_size(struct kunlun_pcie *kunlun_pcie, uint32_t index,
                                              pcie_bar_addr_type_t addr_type, bool prefetch,
                                              uint32_t size, uint32_t is_io)
{
    ASSERT((size & (size - 1)) == 0);

    uint32_t reg_addr = PCIE_EP_BAR0_MASK + index * 4;

    uint32_t mask = size ? (size - 1) : 0;
    kunlun_dbi2_writel(kunlun_pcie, mask, reg_addr);

    if (size)
        kunlun_dbi_writel(kunlun_pcie, (prefetch << 3) | (addr_type << 2) | (is_io << 0), reg_addr);
}

static void dwc_pcie_phy_init(struct kunlun_pcie *kp)
{
    static bool phy_init;
    irq_state_t state;

    enter_critical_area();

    if (!phy_init) {
        kunlun_pcie_phy_init(kp);
        phy_init = true;
    }

    exit_critical_area();
}
static void kunlun_pcie_set_device_id_vendor_id(struct kunlun_pcie *kunlun_pcie, uint32_t id)
{
    kunlun_dbi_writel(kunlun_pcie, id, PCIE_TYPE0_DEVICE_ID_VENDOR_ID_REG);
//    uint32_t reg_val =  kunlun_dbi_readl(kunlun_pcie, PCIE_TYPE0_DEVICE_ID_VENDOR_ID_REG);
 //   ssdk_printf("set device id = 0x%x, vender id = 0x%x\n", reg_val >> 16, reg_val & 0xffff);
}


int kunlun_pcie_config_ep_bar(struct kunlun_pcie *kunlun_pcie, uint32_t pf_index,
                        uint32_t atu_index, uint32_t bar_index, uint64_t bar_addr, uint64_t bar_len,
                        uint32_t is_pref, uint32_t is_bar64, uint32_t is_io)
{
    uint32_t flag = 0;
    uint32_t reg_addr = 0x10 + bar_index * 4;

    if (pf_index == 2)
        reg_addr += 0x20000;

    if (is_pref)
        flag |= 1 << 3;
    if (is_bar64)
        flag |= 1 << 2;
    if (is_io)
        flag |= 1 << 0;

    kunlun_dbi2_writel(kunlun_pcie, bar_len - 1, reg_addr);
    kunlun_dbi_writel(kunlun_pcie, flag, reg_addr);
    if (bar_index > 0)
        kunlun_pcie_config_ib_atu(kunlun_pcie, atu_index, bar_addr, bar_index);
    return 0;
}

#if 0
int kunlun_pcie_config_msi_num(struct kunlun_pcie *kunlun_pcie, uint32_t pf_index, uint32_t num)
{
    uint32_t reg_addr = PCIE_MSI_CTRL;
    uint32_t reg_val;

    if (num > 5)
        return -1;

    if (pf_index == 2)
        reg_addr += 0x20000;

    reg_val = kunlun_dbi_readl(kunlun_pcie, reg_addr);
    reg_val &= ~PCIE_MSI_VECTOR_MASK;
    reg_val |= (num << PCIE_MSI_VECTOR_SHIFT);
    kunlun_dbi_writel(kunlun_pcie, reg_val,  reg_addr);
    return 0;
}
#endif
int kunlun_pcie_config_pf_num(struct kunlun_pcie *kunlun_pcie, uint32_t num)
{
    uint32_t reg_val;

    reg_val = kunlun_dbi_readl(kunlun_pcie, 0x718);
    reg_val &= ~(0xff);
    reg_val |= num;
    kunlun_dbi_writel(kunlun_pcie, reg_val,  0x718);
    return 0;
}

int kunlun_pcie_config_vf_num(struct kunlun_pcie *kunlun_pcie, uint32_t pf_index, uint32_t num)
{
    uint32_t reg_val = (num) << 16 | (num);

    if (pf_index == 1)
        kunlun_dbi_writel(kunlun_pcie, reg_val,  0x178 + 0xc);
    else if (pf_index == 2)
        kunlun_dbi_writel(kunlun_pcie, reg_val,  0x20178 + 0xc);
    else
        return -1;
    return 0;
}




static void dwc_pcie_init(pcie_dev_t *dev, dwc_pcie_local_bar_cfg_t *bar)
{
    struct kunlun_pcie *kp = dev->priv;
	if(kp->pcie_index == PCIE1)
	{	/* only init pcie1 for now */

	    dwc_pcie_phy_init(kp);
	    if (dev->mode == PCIE_RC_MODE){
	       kunlun_pcie_rc_mode_init(kp);
		dwc_pcie_establish_link(dev);

		kunlun_pcie_config_ob_atu_cfg(kp, 0, 0, 0, 0);
	       kunlun_pcie_set_bar(kp, 0, 0x1c000000, 1);
	       kunlun_pcie_set_bar(kp, 4, LINK_AB_PCIE, 1);
	       kunlun_pcie_enable_all_bar(kp);
	    }

	    else{
		kunlun_pcie_ep_mode_init(kp);
		kunlun_pcie_config_pf_num(kp, 0);
		kunlun_pcie_config_vf_num(kp, 1, 0);


		kunlun_pcie_enable_ltssm(kp);
		kunlun_pcie_set_device_id_vendor_id(kp, 0x00011e8c);
		kunlun_pcie_ep_set_class_code(kp, 0x058000);
	    }

#if 0
	    uint8_t ib_atu_region = 0;
	    for (size_t i = 0; i < 6; i++) {
	        /* 1. RC only has 2 BARs.
	         * 2. When Kunlun pcie is as EP, BAR0 & BAR1 is reserved for PCIE REG,
	         * and must not be disabled.
	         */
	        if (dev->mode == PCIE_RC_MODE) {
	            if (i >= 2)
	                break;
	        }
	        else if (dev->mode == PCIE_EP_MODE) {
	            if ((i == 0) || ( i == 1))
	                continue;
	        }
	        kunlun_pcie_config_local_bar_size(kp, i, bar[i].addr_type, bar[i].prefetch,
	                                          bar[i].size, bar[i].is_io);

	        if (bar[i].size)
		{
			kunlun_pcie_config_ib_atu(kp, ib_atu_region, bar[i].target, i);
		}
	}

#endif

	    if (dev->mode == PCIE_EP_MODE){
		kunlun_pcie_config_ep_bar(kp, 1, 0, 0, 0, 0x100000, 0, 1, 0);
		kunlun_pcie_config_ep_bar(kp, 1, 1, 2, 0x30000000, 0x8000000, 0, 1, 0);
		kunlun_pcie_config_ep_bar(kp, 1, 1, 4, LINK_AB_DDR_A5, LINK_AB_SIZE, 0, 1, 0);

		kunlun_pcie_config_ob_atu_mem(kp, 1, 0x10000000, 0x17ffffff, 0x30000000);
		kunlun_pcie_config_ob_atu_mem(kp, 2, LINK_AB_PCIE, (LINK_AB_PCIE + LINK_AB_SIZE - 1), LINK_AB_DDR_A5);
	    }

	}
	else
	{
		/* TBD */
	}

}

static int dwc_pcie_establish_link(pcie_dev_t *dev)
{
    struct kunlun_pcie *kp = dev->priv;
    uint32_t reg_val;

    kunlun_pcie_enable_ltssm(kp);

    /* RC waits for link up. */
    if (dev->mode == PCIE_RC_MODE) {
        int count = 0;

        while (!kunlun_pcie_link_up(kp)) {
			//thread_sleep(50);
			udelay(5000);
			count++;

            kunlun_pcie_get_ltssm_state(kp);
            kunlun_pcie_get_link_speed(kp);
        reg_val = kunlun_dbi_readl(kp, 0x80);
        reg_val |= 0x20;
        kunlun_dbi_writel(kp, reg_val, 0x80);
            if (count == 10) {
                return -1;
            }
        }
    }

    return 0;
}

static uint32_t dwc_pcie_set_cfg_address(pcie_dev_t *dev, int bdf,
                                         uint32_t offset)
{
    struct kunlun_pcie *kp = dev->priv;
    uint8_t bus = PCIE_BUS(bdf);

    offset &= ~3;

    if (!bus)
        return kp->dbi + offset;
    else if (bus == 1) {
        kunlun_pcie_config_ob_atu_cfg(kp, OB_CFG_ATU_REGION, bus,
                                      PCIE_DEV(bdf), PCIE_FUNC(bdf));
        return kp->ecam_base + offset;
    }
    else {
        /* Not support PCIe enumeration now. */
        ASSERT(0);

        return -1;
    }
}

static void dwc_pcie_unset_cfg_address(pcie_dev_t *dev)
{
    struct kunlun_pcie *kp = dev->priv;

    kunlun_pcie_disable_ob_atu(kp, OB_CFG_ATU_REGION);
}

static int dwc_pcie_read_config(pcie_dev_t *dev, int bdf, uint32_t offset,
                                uint32_t *val)
{
    uint32_t addr = dwc_pcie_set_cfg_address(dev, bdf, offset);

    *val = readl(addr);

    if (PCIE_BUS(bdf))
        dwc_pcie_unset_cfg_address(dev);

    return 0;
}

static int dwc_pcie_write_config(pcie_dev_t *dev, int bdf, uint32_t offset,
                                  uint32_t val)
{
    uint32_t addr = dwc_pcie_set_cfg_address(dev, bdf, offset);

    writel(val, addr);

    if (PCIE_BUS(bdf))
        dwc_pcie_unset_cfg_address(dev);

    return 0;
}

static inline uint32_t dwc_pcie_get_link_width(pcie_link_width_t width)
{
    return (width == LINK_WIDTH_DEFAULT) ? LINK_WIDTH_1_LANE : width;
}

static inline uint32_t dwc_pcie_get_link_speed(pcie_link_speed_t speed)
{
    return (speed == LINK_SPEED_DEFAULT) ? LINK_SPEED_GEN1 : speed;
}



void kunlun_pcie_rc_set_dma_addr(struct kunlun_pcie *kunlun_pcie, unsigned long dma_addr)
{
    kunlun_pcie->dma_base = dma_addr;
}

int _pcie_dma_read(void *dma_base, int dma_channel, uint64_t dst, uint64_t src, uint32_t len)
{
    uint32_t val;
    int count = len/1024 + 1;
    void *dma_channel_base = dma_base + 0x300 + 0x200 * dma_channel;

    if (count < 100)
        count = 100;

    val = readl(dma_base + DMA_READ_ENGINE_EN);
    val |= 0x1;
    writel(val, dma_base + DMA_READ_ENGINE_EN);
    writel(0x8, dma_channel_base + DMA_CTRL1_RDCH);

    writel(lower_32_bits(src), dma_channel_base + DMA_SRC_LOW_RDCH);
    writel(upper_32_bits(src), dma_channel_base + DMA_SRC_HI_RDCH);

    writel(lower_32_bits(dst), dma_channel_base + DMA_DST_LOW_RDCH);
    writel(upper_32_bits(dst), dma_channel_base + DMA_DST_HI_RDCH);

    writel(len, dma_channel_base + DMA_SIZE_RDCH);
    writel(0, dma_base + DMA_READ_INT_MASK);

    writel(dma_channel, dma_base + DMA_READ_DOORBELL);

    while (1) {
        val = readl(dma_base + DMA_READ_INT_STS);
        if (val & (1 << (dma_channel + 16))) {
            ssdk_printf(SSDK_INFO, "read abort\n");
            writel(1 << (dma_channel + 16), dma_base + DMA_READ_INT_CLR);
            return -1;
        }
        if (val & (1 << dma_channel)) {
            writel((1 << dma_channel), dma_base + DMA_READ_INT_CLR);
            return 0;
        }

        if (count--) {
            osDelay(3);
        } else {
            ssdk_printf(SSDK_INFO, "read poll time out\n");
            return -2;
        }
    }
}

int kunlun_pcie_dma_read(pcie_dev_t *dev,
               int dma_channel, uint64_t dst, uint64_t src, uint32_t len)
{
    struct kunlun_pcie *kunlun_pcie = dev->priv;
    if (kunlun_pcie->dma_base == 0)
        return -1;

    return _pcie_dma_read((void *)kunlun_pcie->dma_base, dma_channel, dst, src, len);
}

int _pcie_dma_write(void *dma_base, int dma_channel, uint64_t dst, uint64_t src, uint64_t len)
{
    uint32_t val;
    int count = len/1024;
    void *dma_channel_base = dma_base + 0x200 + 0x200 * dma_channel;

    if (count < 100)
        count = 100;

    val = readl(dma_base + DMA_WRITE_ENGINE_EN);
    val |= 0x1;
    writel(val, dma_base + DMA_WRITE_ENGINE_EN);
    writel(0x8, dma_channel_base + DMA_CTRL1_WRCH);

    writel(lower_32_bits(src), dma_channel_base + DMA_SRC_LOW_WRCH);
    writel(upper_32_bits(src), dma_channel_base + DMA_SRC_HI_WRCH);

    writel(lower_32_bits(dst), dma_channel_base + DMA_DST_LOW_WRCH);
    writel(upper_32_bits(dst), dma_channel_base + DMA_DST_HI_WRCH);

    writel(len, dma_channel_base + DMA_SIZE_WRCH);
    writel(0, dma_base + DMA_WRITE_INT_MASK);

    writel(dma_channel, dma_base + DMA_WRITE_DOORBELL);

    while (1) {
        val = readl(dma_base + DMA_WRITE_INT_STS);
        if (val & (1 << (dma_channel + 16))) {
            ssdk_printf(SSDK_INFO, "write abort\n");
            writel(1 << (dma_channel + 16), dma_base + DMA_WRITE_INT_CLR);
            return -1;
        }
        if (val & (1 << dma_channel)) {
            writel(1 << (dma_channel), dma_base + DMA_WRITE_INT_CLR);
            return 0;
        }

        if (count--) {
            osDelay(3);
        } else {
            ssdk_printf(SSDK_INFO, "read poll time out\n");
            return -2;
        }
    }
}

int kunlun_pcie_dma_write(pcie_dev_t *dev,
               int dma_channel, uint64_t dst, uint64_t src, uint32_t len)
{
    struct kunlun_pcie *kunlun_pcie = dev->priv;
    if (kunlun_pcie->dma_base == 0)
        return -1;

    return _pcie_dma_write((void *)kunlun_pcie->dma_base, dma_channel, dst, src, len);
}

static pcie_ops_t g_pcie_ops = {
    .establish_link = dwc_pcie_establish_link,
    .read_config = dwc_pcie_read_config,
    .write_config = dwc_pcie_write_config,

    .dma_read = kunlun_pcie_dma_read,
    .dma_write = kunlun_pcie_dma_write,
};

void dwc_pcie_probe(pcie_dev_t *dev, dwc_pcie_cfg_t *cfg)
{
    kunlun_pcie_t *kunlun_pcie = osAlloc(sizeof(kunlun_pcie_t));
    ASSERT(kunlun_pcie);

    kunlun_pcie->pcie_index = cfg->controller;
    if (cfg->refclk_use_internal)
        kunlun_pcie->phy_refclk_sel = PHY_REFCLK_USE_INTERNAL;
    else
        kunlun_pcie->phy_refclk_sel = PHY_REFCLK_USE_EXTERNAL;
    kunlun_pcie->phy_refclk_sel |= ((cfg->diffbuf_on) & 1) << 31;
    kunlun_pcie->ctrl_ncr_base = cfg->ctrl_ncr_base;
    kunlun_pcie->phy_ncr_base = cfg->phy_ncr_base;
    kunlun_pcie->phy_base = cfg->phy_base;
    kunlun_pcie->dbi = cfg->dbi;
    kunlun_pcie->ecam_base = cfg->ecam_base;
    kunlun_pcie->link_width = dwc_pcie_get_link_width(cfg->width);
    kunlun_pcie->link_speed = dwc_pcie_get_link_speed(cfg->speed);
    kunlun_pcie->dma_base = cfg->dma_base;
    dev->controller = cfg->controller;
    dev->mode = cfg->mode;
    dev->ops = &g_pcie_ops;
    dev->priv = kunlun_pcie;
    dwc_pcie_init(dev, cfg->bar);
}
