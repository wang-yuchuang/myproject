/*
 * mmc_dwcmshc.c
 *
 * Copyright (c) 2018 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: mmc semidrive dwcmshc code.
 *
 * Revision History:
 * -----------------
 * 0.1, 12/9/2019 init version
 */
#include <debug.h>
#include <param.h>
#include <driver/clk.h>
#include <driver/scr.h>

#include <driver/kunlun/mmc/mmc_sdhci.h>
#include <driver/kunlun/mmc/sdhci.h>

#include <driver/kunlun/mmc/mmc_dwcmshc.h>

#define SDHCI_VENDER_EMMC_CTRL_REG (0x2C)

#define SDHCI_IS_EMMC_CARD_MASK BIT(0)
#define SDHCI_EMMC_RST_N_MASK BIT(2)
#define SDHCI_EMMC_RST_N_OE_MASK BIT(3)

#define DWC_MSHC_PTR_PHY_REGS 0x300
#define DWC_MSHC_PHY_CNFG (DWC_MSHC_PTR_PHY_REGS + 0x0)
#define PAD_SN_LSB 20
#define PAD_SN_MASK 0xF
#define PAD_SN_DEFAULT ((0x8 & PAD_SN_MASK) << PAD_SN_LSB)
#define PAD_SP_LSB 16
#define PAD_SP_MASK 0xF
#define PAD_SP_DEFAULT ((0x9 & PAD_SP_MASK) << PAD_SP_LSB)
#define PHY_PWRGOOD BIT(1)
#define PHY_RSTN BIT(0)

#define DWC_MSHC_CMDPAD_CNFG (DWC_MSHC_PTR_PHY_REGS + 0x4)
#define DWC_MSHC_DATPAD_CNFG (DWC_MSHC_PTR_PHY_REGS + 0x6)
#define DWC_MSHC_CLKPAD_CNFG (DWC_MSHC_PTR_PHY_REGS + 0x8)
#define DWC_MSHC_STBPAD_CNFG (DWC_MSHC_PTR_PHY_REGS + 0xA)
#define DWC_MSHC_RSTNPAD_CNFG (DWC_MSHC_PTR_PHY_REGS + 0xC)
#define TXSLEW_N_LSB 9
#define TXSLEW_N_MASK 0xF
#define TXSLEW_P_LSB 5
#define TXSLEW_P_MASK 0xF
#define WEAKPULL_EN_LSB 3
#define WEAKPULL_EN_MASK 0x3
#define RXSEL_LSB 0
#define RXSEL_MASK 0x3

#define DWC_MSHC_COMMDL_CNFG (DWC_MSHC_PTR_PHY_REGS + 0x1C)
#define DWC_MSHC_SDCLKDL_CNFG (DWC_MSHC_PTR_PHY_REGS + 0x1D)
#define DWC_MSHC_SDCLKDL_DC (DWC_MSHC_PTR_PHY_REGS + 0x1E)
#define DWC_MSHC_SMPLDL_CNFG (DWC_MSHC_PTR_PHY_REGS + 0x20)
#define DWC_MSHC_ATDL_CNFG (DWC_MSHC_PTR_PHY_REGS + 0x21)

#define DWC_MSHC_DLL_CTRL (DWC_MSHC_PTR_PHY_REGS + 0x24)
#define DWC_MSHC_DLL_CNFG1 (DWC_MSHC_PTR_PHY_REGS + 0x25)
#define DWC_MSHC_DLL_CNFG2 (DWC_MSHC_PTR_PHY_REGS + 0x26)
#define DWC_MSHC_DLLDL_CNFG (DWC_MSHC_PTR_PHY_REGS + 0x28)
#define DWC_MSHC_DLL_OFFSET (DWC_MSHC_PTR_PHY_REGS + 0x29)
#define DWC_MSHC_DLLLBT_CNFG (DWC_MSHC_PTR_PHY_REGS + 0x2C)
#define DWC_MSHC_DLL_STATUS (DWC_MSHC_PTR_PHY_REGS + 0x2E)
#define ERROR_STS BIT(1)
#define LOCK_STS BIT(0)

#define DWC_MSHC_PHY_PAD_SD_CLK                                                \
    ((1 << TXSLEW_N_LSB) | (3 << TXSLEW_P_LSB) | (0 << WEAKPULL_EN_LSB) |      \
     (1 << RXSEL_LSB))
#define DWC_MSHC_PHY_PAD_SD_DAT                                                \
    ((1 << TXSLEW_N_LSB) | (3 << TXSLEW_P_LSB) | (1 << WEAKPULL_EN_LSB) |      \
     (1 << RXSEL_LSB))
#define DWC_MSHC_PHY_PAD_SD_STB                                                \
    ((1 << TXSLEW_N_LSB) | (3 << TXSLEW_P_LSB) | (2 << WEAKPULL_EN_LSB) |      \
     (1 << RXSEL_LSB))

#define DWC_MSHC_PHY_PAD_EMMC_CLK                                              \
    ((2 << TXSLEW_N_LSB) | (2 << TXSLEW_P_LSB) | (0 << WEAKPULL_EN_LSB) |      \
     (0 << RXSEL_LSB))
#define DWC_MSHC_PHY_PAD_EMMC_DAT                                              \
    ((2 << TXSLEW_N_LSB) | (2 << TXSLEW_P_LSB) | (1 << WEAKPULL_EN_LSB) |      \
     (1 << RXSEL_LSB))
#define DWC_MSHC_PHY_PAD_EMMC_STB                                              \
    ((2 << TXSLEW_N_LSB) | (2 << TXSLEW_P_LSB) | (2 << WEAKPULL_EN_LSB) |      \
     (1 << RXSEL_LSB))

static inline void mshc_phy_pad_config(struct sdhci_host *host,
                                       uint32_t card_type)
{
    uint16_t clk_ctrl;

    /* Disable the card clock */
    clk_ctrl = REG_READ16(host, SDHCI_CLK_CTRL_REG);
    clk_ctrl &= ~SDHCI_CLK_EN;
    REG_WRITE16(host, clk_ctrl, SDHCI_CLK_CTRL_REG);

    if (card_type >= MMC_TYPE_MMCHC) {
        REG_WRITE16(host, DWC_MSHC_PHY_PAD_EMMC_DAT, DWC_MSHC_CMDPAD_CNFG);
        REG_WRITE16(host, DWC_MSHC_PHY_PAD_EMMC_DAT, DWC_MSHC_DATPAD_CNFG);
        REG_WRITE16(host, DWC_MSHC_PHY_PAD_EMMC_CLK, DWC_MSHC_CLKPAD_CNFG);
        REG_WRITE16(host, DWC_MSHC_PHY_PAD_EMMC_STB, DWC_MSHC_STBPAD_CNFG);
        REG_WRITE16(host, DWC_MSHC_PHY_PAD_EMMC_DAT, DWC_MSHC_RSTNPAD_CNFG);
    }
    else {
        REG_WRITE16(host, DWC_MSHC_PHY_PAD_SD_DAT, DWC_MSHC_CMDPAD_CNFG);
        REG_WRITE16(host, DWC_MSHC_PHY_PAD_SD_DAT, DWC_MSHC_DATPAD_CNFG);
        REG_WRITE16(host, DWC_MSHC_PHY_PAD_SD_CLK, DWC_MSHC_CLKPAD_CNFG);
        REG_WRITE16(host, DWC_MSHC_PHY_PAD_SD_STB, DWC_MSHC_STBPAD_CNFG);
        REG_WRITE16(host, DWC_MSHC_PHY_PAD_SD_DAT, DWC_MSHC_RSTNPAD_CNFG);
    }

    return;
}

static inline void mshc_phy_delay_config(struct sdhci_host *host)
{
    REG_WRITE8(host, 1, DWC_MSHC_COMMDL_CNFG);
    REG_WRITE8(host, 0, DWC_MSHC_SDCLKDL_CNFG);
    REG_WRITE8(host, 8, DWC_MSHC_SMPLDL_CNFG);
    REG_WRITE8(host, 8, DWC_MSHC_ATDL_CNFG);
    return;
}

static inline int mshc_phy_dll_config(struct sdhci_host *host)
{
    uint32_t ret;
    uint16_t clk_ctrl = 0;

    REG_WRITE8(host, 0, DWC_MSHC_DLL_CTRL);

    /* Disable the clock */
    clk_ctrl = REG_READ16(host, SDHCI_CLK_CTRL_REG);
    clk_ctrl &= ~SDHCI_CLK_EN;
    REG_WRITE16(host, clk_ctrl, SDHCI_CLK_CTRL_REG);

    REG_WRITE8(host, 0x20, DWC_MSHC_DLL_CNFG1);
    // TODO: set the dll value by real chip
    REG_WRITE8(host, 0, DWC_MSHC_DLL_CNFG2);
    REG_WRITE8(host, 0x60, DWC_MSHC_DLLDL_CNFG);
    REG_WRITE8(host, 0, DWC_MSHC_DLL_OFFSET);
    REG_WRITE16(host, 0, DWC_MSHC_DLLLBT_CNFG);

    /* Enable the clock */
    clk_ctrl |= SDHCI_CLK_EN;
    REG_WRITE16(host, clk_ctrl, SDHCI_CLK_CTRL_REG);

    REG_WRITE16(host, 0x1, DWC_MSHC_DLL_CTRL);

    ret = sdhci_wait_for_bit(host, DWC_MSHC_DLL_STATUS, LOCK_STS, 0, 150);

    if (!ret) {
        ret = REG_READ8(host, DWC_MSHC_DLL_STATUS) & ERROR_STS;
    }

    return ret;
}

static int mshc_phy_init(struct sdhci_host *host)
{
    int ret;
    uint32_t reg = 0;

    REG_WRITE32(host, 0, DWC_MSHC_PHY_CNFG);

    /* Disable the clock */
    REG_WRITE16(host, 0, SDHCI_CLK_CTRL_REG);

    mshc_phy_pad_config(host, MMC_TYPE_MMCHC);
    mshc_phy_delay_config(host);

    ret = sdhci_wait_for_bit(host, DWC_MSHC_PHY_CNFG, PHY_PWRGOOD, 0, 150);

    if (!ret) {
        reg = PAD_SN_DEFAULT | PAD_SP_DEFAULT;
        REG_WRITE32(host, reg, DWC_MSHC_PHY_CNFG);
        /* de-assert the phy */
        reg |= PHY_RSTN;
        REG_WRITE32(host, reg, DWC_MSHC_PHY_CNFG);
    }

    return ret;
}

static uint32_t mshc_set_clk(struct sdhci_host *host, unsigned int clock)
{
    uint32_t ret = 0;
    uint16_t clk_ctrl = 0;
    struct mmc_device *mmc = host->parent;
    struct mmc_config_data *mmc_cfg = &mmc->config;
    clk_node_t *clk = mmc_cfg->clk;

    /* Disable the clock */
    REG_WRITE16(host, clk_ctrl, SDHCI_CLK_CTRL_REG);

    /*
     * Beacuse the clock will be 2 dvider by mshc model,
     * so we need twice base frequency.
     */
    if (clk_set_rate(clk, MIN(clock, host->max_clk_rate) * 2))
        return 1;

    clk_ctrl |= SDHCI_INT_CLK_EN;
    REG_WRITE16(host, clk_ctrl, SDHCI_CLK_CTRL_REG);

    /* Check for clock stable, timeout 150ms */
    ret =
        sdhci_wait_for_bit(host, SDHCI_CLK_CTRL_REG, SDHCI_CLK_STABLE, 0, 150);

    if (ret) {
        ssdk_printf(SSDK_CRIT, "Error: sdhci clock wait stable timeout!");
        return ret;
    }

    clk_ctrl |= SDHCI_CLK_PLL_EN;
    REG_WRITE16(host, clk_ctrl, SDHCI_CLK_CTRL_REG);

    /* Check for clock stable, timeout 150ms */
    ret =
        sdhci_wait_for_bit(host, SDHCI_CLK_CTRL_REG, SDHCI_CLK_STABLE, 0, 150);

    if (ret) {
        ssdk_printf(SSDK_CRIT, "Error: sdhci clock wait stable timeout!");
        return ret;
    }

    /* Now clock is stable, enable it */
    clk_ctrl = REG_READ16(host, SDHCI_CLK_CTRL_REG);
    clk_ctrl |= SDHCI_CLK_EN;
    REG_WRITE16(host, clk_ctrl, SDHCI_CLK_CTRL_REG);
    return 0;
}

static void mshc_config_pin(struct sdhci_host *host)
{
    return;
}

static void mshc_init(struct sdhci_host *host)
{
    uint16_t reg;
    uint32_t card_is_emmc;
    uint16_t vender_base;

    /* dwcmshc need set card type, emmc or other */
    card_is_emmc = !host->card_type;

    /* read verder base register address */
    vender_base = REG_READ16(host, SDHCI_VENDOR_BASE_REG) & 0xFFF;

    reg = REG_READ16(host, vender_base + SDHCI_VENDER_EMMC_CTRL_REG);
    reg &= ~SDHCI_IS_EMMC_CARD_MASK;
    reg |= card_is_emmc;
    REG_WRITE16(host, reg, vender_base + SDHCI_VENDER_EMMC_CTRL_REG);

    if (mshc_phy_init(host))
        ssdk_printf(SSDK_CRIT, "mshc: phy init failed!\n");
}

static void mshc_hw_reset(struct sdhci_host *host)
{
    uint16_t reg;
    uint16_t vender_base;

    vender_base = REG_READ16(host, SDHCI_VENDOR_BASE_REG) & 0xFFF;

    reg = REG_READ16(host, vender_base + SDHCI_VENDER_EMMC_CTRL_REG);
    reg |= SDHCI_EMMC_RST_N_OE_MASK;
    reg &= ~SDHCI_EMMC_RST_N_MASK;
    REG_WRITE16(host, reg, vender_base + SDHCI_VENDER_EMMC_CTRL_REG);

    udelay(5);
    reg |= SDHCI_EMMC_RST_N_MASK;
    REG_WRITE16(host, reg, vender_base + SDHCI_VENDER_EMMC_CTRL_REG);
}

static void mshc_set_uhs_mode(struct sdhci_host *host, uint32_t mode)
{
    struct mmc_dwcmshc *mshc = host->priv_data;

    sdhci_set_uhs_mode(host, mode);

    /* if ddr mode, need set ddr_mode enable in scr */
    uint32_t ddr_mode = 0;

    /* it may be high speed DDR or HS400 */
    if (mode > SDHCI_EMMC_HS200_MODE)
        ddr_mode = 1;

    /* set mshc ddr mode in scr */
    scr_set(mshc->scr_base, &mshc->ddr_mode_scr, ddr_mode);

    if (mode == SDHCI_EMMC_HS400_MODE) {
        if (mshc_phy_dll_config(host))
            ssdk_printf(SSDK_CRIT, "mshc phy dll config failed!\n");
    }

    return;
}

static inline void enable_tune_clk_stop(struct sdhci_host *host)
{
    uint32_t reg;
    uint16_t vender_base;

    /* read verder base register address */
    vender_base = REG_READ16(host, SDHCI_VENDOR_BASE_REG) & 0xFFF;

    reg = REG_READ32(host, vender_base + SDHCI_VENDER_AT_CTRL_REG);
    reg &= ~(SDHCI_TUNE_SWIN_TH_VAL_MASK << SDHCI_TUNE_SWIN_TH_VAL_LSB);
    reg |= (0xF << SDHCI_TUNE_SWIN_TH_VAL_LSB);
    reg |= SDHCI_TUNE_CLK_STOP_EN_MASK;
    REG_WRITE32(host, reg, vender_base + SDHCI_VENDER_AT_CTRL_REG);
}

static int mshc_execute_tuning(struct sdhci_host *host, uint32_t opcode,
                               uint32_t bus_width)
{
    uint32_t ret = 0;
    uint16_t clk_ctrl = 0;

    /* reset the tuning circuit */
    sdhci_reset_tuning(host);
    enable_tune_clk_stop(host);
    sdhci_reset(host, SOFT_RESET_CMD | SOFT_RESET_DATA);

    /*
     * For avoid giltches, need disable the card clock before set
     * EXEC_TUNING bit.
     */
    clk_ctrl = REG_READ16(host, SDHCI_CLK_CTRL_REG);
    clk_ctrl &= ~SDHCI_CLK_EN;
    REG_WRITE16(host, clk_ctrl, SDHCI_CLK_CTRL_REG);
    udelay(1);

    /* start the tuning circuit */
    sdhci_start_tuning(host);
    host->tuning_in_progress = 1;
    udelay(1);

    /* enable the card clock */
    clk_ctrl |= SDHCI_CLK_EN;
    REG_WRITE16(host, clk_ctrl, SDHCI_CLK_CTRL_REG);
    udelay(1);

    if (sdhci_tuning_sequence(host, opcode, bus_width)) {
        ssdk_printf(SSDK_CRIT, "sdhci tuning failed, abort tuning state!\n");
        sdhci_abort_tuning(host);
        ret = 1;
    }

    host->tuning_in_progress = 0;
    return ret;
}

struct sdhci_ops sdhci_dwcmshc_ops = {
    .host_init = sdhci_init,
    .priv_init = mshc_init,
    .hw_reset = mshc_hw_reset,
    .set_clock = mshc_set_clk,
    .config_pin = mshc_config_pin,
    .set_uhs_mode = mshc_set_uhs_mode,
    .platform_execute_tuning = mshc_execute_tuning,
    .execute_tuning = sdhci_execute_tuning,
    .set_bus_width = sdhci_set_bus_width,
    .send_command = sdhci_send_command,
};

int mmc_platform_init(struct mmc_device *dev, void *priv_data)
{
    struct sdhci_host *host;
    struct mmc_config_data *mmc_cfg = &dev->config;
    clk_node_t *clk = mmc_cfg->clk;
    struct mmc_dwcmshc *mshc = priv_data;

    host = &dev->host;
    host->priv_data = mshc;
    host->ops = &sdhci_dwcmshc_ops;
    host->parent = dev;

    /* Init the ref clock: 400Khz */
    if (clk_set_rate(clk, 400000 * 2)) {
        ssdk_printf(SSDK_CRIT, "mmc set ref clock failed!\n");
        return -1;
    }

    if (mmc_sdhci_init(dev)) {
        ssdk_printf(SSDK_CRIT, "mmc init failed!\n");
        return -1;
    }
    return 0;
}
