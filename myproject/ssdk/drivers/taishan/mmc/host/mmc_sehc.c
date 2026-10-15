/*
 * mmc_sehc.c
 *
 * Copyright (c) 2018 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: mmc semidrive sehc code.
 *
 * Revision History:
 * -----------------
 *
 */
#include <stdlib.h>
#include <string.h>
#include <debug.h>
#include <param.h>
#include <driver/clk.h>

#include <driver/taishan/mmc/mmc_sdhci.h>
#include <driver/taishan/mmc/sdhci.h>

#define SEHC_MAX_TUNING_LOOP 140

/* SDHCI SEHC REG */
#define SDHCI_DLL_CTRL (0x100)
#define SDHCI_DLL_STATUS (0x104)

#define SDHCI_EMMC_CTRL_REG (0x10C)

#define SDHCI_TUNING_CTRL_REG (0x118)
#define SDHCI_TUNING_STATUS_REG (0x11C)

/* dll related macros */
#define SDHCI_DLL_SLV_TARGET_MASK 0x3f
#define SDHCI_DLL_SLV_TARGET_OFFSET 0x1
#define SDHCI_DLL_EN 0x00000001

#define SDHCI_DLL_REF_LOCK 0x00000001

/* emmc related macros */
#define SDHCI_RST_N 0x100
#define SDHCI_FIX_BLK_SIZE 0x20
#define SDHCI_CLK_STP_BLK_GAP 0x10
#define SDHCI_EMMC_EXT 0x2
#define SDHCI_ENHANCE_HS400_EN 0x1

/* tuning related macros */
#define SDHCI_TUNING_CLK_STOP_EN 0x80000000
#define SDHCI_DELAY_TAP_VALUE_MASK 0x3f
#define SDHCI_DELAY_TAP_VALUE_OFFSET 0x10
#define SDHCI_OVERRIDE_VAL_MASK 0x7f
#define SDHCI_OVERRIDE_VAL_OFFSET 0x9
#define SDHCI_OVERRIDE_EN 0x00000100
#define SDHCI_TAP_STEP_MASK 0x3
#define SDHCI_TAP_STEP_OFFSET 0x6
#define SDHCI_START_TAP_MASK 0x3f
#define SDHCI_START_TAP_OFFSET 0x0

#define SDHCI_TUNING_TAP_VALUE_MASK 0x7f
#define SDHCI_TUNING_TAP_VALUE_OFFSET 0x14
#define SDHCI_FAIL_NUM_MASK 0x7f
#define SDHCI_FAIL_NUM_OFFSET 0xc
#define SDHCI_PASS_NUM_MASK 0x7f
#define SDHCI_PASS_NUM_OFFSET 0x4
#define SDHCI_SEC_FAIL_FLAG 0x00000008
#define SDHCI_FIR_PASS_FLAG 0x00000004
#define SDHCI_FIR_FAIL_FLAG 0x00000002
#define SDHCI_TUNING_FAIL 0x00000001

/* SEHC TUNING related macros */
#define SEHC_TUNE_CTRL_MIN 20
#define SEHC_TUNE_CTRL_MAX 128
#define SEHC_TUNE_CTRL_STEP 1
#define SEHC_TUNE_WINDOW_MIN 5

static uint32_t sehc_set_clk(struct sdhci_host *host, unsigned int clock)
{
    uint32_t ret = 0;
    uint16_t clk_ctrl = 0;
    struct mmc_device *mmc = host->parent;
    struct mmc_config_data *mmc_cfg = &mmc->config;
    clk_node_t *clk = mmc_cfg->clk;

    /* Disable the clock */
    REG_WRITE16(host, clk_ctrl, SDHCI_CLK_CTRL_REG);

    if (clock == 0u)
        return 0;

    /*
     * DDR TIMING need twice base frequency.
     */
    if (clk != NULL) {
        if ((host->timing == SDHCI_EMMC_HS400_MODE)
                || (host->timing == SDHCI_EMMC_DDR52_MODE)
                || (host->timing == SDHCI_UHS_DDR50_MODE)) {
            if (clk_set_rate(clk, MIN(clock, host->max_clk_rate) * 2))
                return 1;
        }
        else {
            if (clk_set_rate(clk, MIN(clock, host->max_clk_rate)))
                return 1;
        }
    }

    ssdk_printf(SSDK_INFO, "switch card clock to %d\n", MIN(clock,
                host->max_clk_rate));

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

    return ret;
}

static inline int sehc_dll_config(struct sdhci_host *host)
{
    uint32_t ret = 0;
    uint16_t clk_ctrl = 0;
    uint32_t dll_ctrl = 0;

    ssdk_printf(SSDK_INFO, "sehc dll config!\n");

    /* Disable the clock */
    clk_ctrl = REG_READ16(host, SDHCI_CLK_CTRL_REG);
    clk_ctrl &= ~SDHCI_CLK_EN;
    REG_WRITE16(host, clk_ctrl, SDHCI_CLK_CTRL_REG);

    /* config dll_ctrl */
    dll_ctrl = REG_READ32(host, SDHCI_DLL_CTRL);
    dll_ctrl |= SDHCI_DLL_EN;
    dll_ctrl &= ~(SDHCI_DLL_SLV_TARGET_MASK << SDHCI_DLL_SLV_TARGET_OFFSET);
    dll_ctrl |= (0x1f << SDHCI_DLL_SLV_TARGET_OFFSET);
    REG_WRITE32(host, dll_ctrl, SDHCI_DLL_CTRL);

    /* Enable the clock */
    clk_ctrl |= SDHCI_CLK_EN;
    REG_WRITE16(host, clk_ctrl, SDHCI_CLK_CTRL_REG);

    ret = sdhci_wait_for_bit(host, SDHCI_DLL_STATUS, SDHCI_DLL_REF_LOCK, 0, 200);

    if (ret) {
        ssdk_printf(SSDK_CRIT, "sehc dll timeout!\n");
        return ret;
    }

    return ret;
}

static void sehc_set_uhs_mode(struct sdhci_host *host, uint32_t mode)
{

    sdhci_set_uhs_mode(host, mode);

    /* HS400 mode need config dll frequency */
    if (mode == SDHCI_EMMC_HS400_MODE) {
        if (sehc_dll_config(host))
            ssdk_printf(SSDK_CRIT, "sehc dll config failed!\n");
    }

    return;
}

static void sehc_config_pin(struct sdhci_host *host)
{
    return;
}

static void sehc_init(struct sdhci_host *host)
{
    return;
}

static void sehc_hw_reset(struct sdhci_host *host)
{
    uint32_t emmc_ctrl;
    emmc_ctrl = REG_READ32(host, SDHCI_EMMC_CTRL_REG);
    emmc_ctrl &= ~SDHCI_RST_N;
    REG_WRITE32(host, emmc_ctrl, SDHCI_EMMC_CTRL_REG);

    udelay(5);
    emmc_ctrl |= SDHCI_RST_N;
    REG_WRITE32(host, emmc_ctrl, SDHCI_EMMC_CTRL_REG);
}

static uint32_t sehc_tuning_sequence(struct sdhci_host *host, uint32_t opcode,
                                     uint32_t bus_width)
{
    uint32_t ret;
    uint32_t reg;
    struct mmc_command cmd;
    memset(&cmd, 0, sizeof(struct mmc_command));
    cmd.cmd_index = opcode;
    cmd.argument = 0x0;
    cmd.cmd_type = SDHCI_CMD_TYPE_NORMAL;
    cmd.resp_type = SDHCI_CMD_RESP_R1;
    cmd.trans_mode = SDHCI_MMC_READ;
    cmd.data_present = 0x0;
    cmd.data.data_ptr = NULL;
    cmd.data.num_blocks = 0x1;

    if (cmd.cmd_index == CMD21_SEND_TUNING_BLOCK &&
            bus_width == MMC_DATA_BUS_WIDTH_8BIT)
        cmd.data.blk_sz = 128;
    else
        cmd.data.blk_sz = 64;

    REG_WRITE16(host, cmd.data.blk_sz, SDHCI_BLKSZ_REG);

    if (host->caps.spec_version >= SDHCI_SPEC_VER4_NUM)
        REG_WRITE32(host, cmd.data.num_blocks, SDHCI_SDMASA_BLKCNT_REG);
    else
        REG_WRITE16(host, cmd.data.num_blocks, SDHCI_BLK_CNT_REG);

    /* Write to transfer mode register */
    REG_WRITE16(host, SDHCI_READ_MODE, SDHCI_TRANS_MODE_REG);

    for (int i = 0; i < SEHC_MAX_TUNING_LOOP; i++) {
        sdhci_send_command(host, &cmd);
        /* wait for read buf ready, timeout 50ms */
        ret = sdhci_wait_for_bit(host, SDHCI_NRML_INT_STS_REG,
                                 SDHCI_INT_DATA_AVAIL, 0, 50);

        if (ret) {
            ssdk_printf(SSDK_CRIT, "sdhci tuning timeout!\n");
            return ret;
        }

        reg = REG_READ16(host, SDHCI_HOST_CTRL2_REG);

        if (!(reg & SDHCI_EXEC_TUNING)) {
            /* check SDHCI_SAMPLE_CLK_SEL set,tuning is successful */
            if (reg & SDHCI_SAMPLE_CLK_SEL) {
                ssdk_printf(SSDK_INFO, "sdhci: tuning cycle count = %d\n", i);

                if (i < 40)
                    ssdk_printf(SSDK_CRIT, "SDHC: tuning cycle abnormal, num is %d!\n", i);

                return 0;
            }

            /* check FAIL_NUM_ > 0,also tuning is successful */
            reg = REG_READ32(host, SDHCI_TUNING_STATUS_REG);

            if ((reg >> SDHCI_FAIL_NUM_OFFSET) & SDHCI_FAIL_NUM_MASK) {
                ssdk_printf(SSDK_INFO, "sdhci: tuning cycle count = %d\n", i);

                if (i < 40)
                    ssdk_printf(SSDK_CRIT, "SDHC: tuning cycle abnormal, num is %d!\n", i);

                ssdk_printf(SSDK_INFO, "sdhci: tuning sw config override\n");

                uint32_t fail_num = 0;
                uint32_t pass_num = 0;
                uint32_t override_val = 0;

                fail_num = (reg >> SDHCI_FAIL_NUM_OFFSET) & SDHCI_FAIL_NUM_MASK;
                pass_num = (reg >> SDHCI_PASS_NUM_OFFSET) & SDHCI_PASS_NUM_MASK;
                override_val = (fail_num + pass_num) / 2;

                /* set override */
                reg = REG_READ32(host, SDHCI_TUNING_CTRL_REG);
                reg |= SDHCI_OVERRIDE_EN;
                reg &= ~(SDHCI_OVERRIDE_VAL_MASK << SDHCI_OVERRIDE_VAL_OFFSET);
                reg |= override_val << SDHCI_OVERRIDE_VAL_OFFSET;
                REG_WRITE32(host, reg, SDHCI_TUNING_CTRL_REG);
                /* set SAMPLE_CLK_SEL */
                reg = REG_READ16(host, SDHCI_HOST_CTRL2_REG);
                reg |= SDHCI_SAMPLE_CLK_SEL;
                REG_WRITE16(host, reg, SDHCI_HOST_CTRL2_REG);

                return 0;

            }

            break;
        }
    }

    return 1;
}

int sehc_execute_tuning(struct sdhci_host *host, uint32_t opcode,
                        uint32_t bus_width)
{
    uint32_t ret = 0;
    uint32_t reg;

    /* reset the tuning circuit */
    sdhci_reset_tuning(host);

    /* start the tuning circuit */
    sdhci_start_tuning(host);
    host->tuning_in_progress = 1;
    udelay(1);

    /* disable SDHCI_INT_DATA_AVAIL interrupt */
    reg = REG_READ16(host, SDHCI_NRML_INT_SIG_EN_REG);
    reg &= ~SDHCI_INT_DATA_AVAIL;
    REG_WRITE16(host, reg, SDHCI_NRML_INT_SIG_EN_REG);

    if (sehc_tuning_sequence(host, opcode, bus_width)) {
        ssdk_printf(SSDK_CRIT, "sdhci tuning failed, abort tuning state!\n");
        sdhci_abort_tuning(host);
        ret = 1;
    }

    /* clear SDHCI_INT_DATA_AVAIL */
    REG_WRITE16(host, SDHCI_INT_DATA_AVAIL, SDHCI_NRML_INT_STS_REG);
    /* enable SDHCI_INT_DATA_AVAIL interrupt */
    reg = REG_READ16(host, SDHCI_NRML_INT_SIG_EN_REG);
    reg |= SDHCI_INT_DATA_AVAIL;
    REG_WRITE16(host, reg, SDHCI_NRML_INT_SIG_EN_REG);

    host->tuning_in_progress = 0;
    return ret;
}

static void sehc_prepare_tuning(struct sdhci_host *host, uint32_t val)
{
    uint32_t reg;
    uint16_t clk = 0;

    clk = REG_READ16(host, SDHCI_CLK_CTRL_REG);
    clk &= ~SDHCI_CLK_EN;
    REG_WRITE16(host, clk, SDHCI_CLK_CTRL_REG);

    reg = REG_READ32(host, SDHCI_TUNING_CTRL_REG);
    reg |= SDHCI_OVERRIDE_EN;
    reg &= ~(SDHCI_OVERRIDE_VAL_MASK << SDHCI_OVERRIDE_VAL_OFFSET);
    reg |= val << SDHCI_OVERRIDE_VAL_OFFSET;
    REG_WRITE32(host, reg, SDHCI_TUNING_CTRL_REG);

    clk = REG_READ16(host, SDHCI_CLK_CTRL_REG);
    clk |= SDHCI_CLK_EN;
    REG_WRITE16(host, clk, SDHCI_CLK_CTRL_REG);
}

static void sehc_post_tuning(struct sdhci_host *host)
{
    sdhci_reset_tuning(host);
}

int sehc_sw_execute_tuning(struct sdhci_host *host, uint32_t opcode,
                           uint32_t bus_width)
{
    uint32_t ret = 0;
    uint32_t min = 0;
    uint32_t max = 0;
    uint32_t avg = 0;

    /* reset the tuning circuit */
    sdhci_reset_tuning(host);

    /* config the sample tuning circuit */
    uint16_t ctrl2;
    ctrl2 = REG_READ16(host, SDHCI_HOST_CTRL2_REG);
    ctrl2 |= SDHCI_SAMPLE_CLK_SEL;
    REG_WRITE16(host, ctrl2, SDHCI_HOST_CTRL2_REG);

    host->sw_tuning_in_progress = 1;
    udelay(1);

    /* find the mininum delay first which can not pass tuning */
    min = SEHC_TUNE_CTRL_MIN;

    while (min < SEHC_TUNE_CTRL_MAX) {
        sehc_prepare_tuning(host, min);

        if (mmc_send_tuning((struct mmc_device *)host->parent, opcode, bus_width))
            break;

        min += SEHC_TUNE_CTRL_STEP;
    }

    ssdk_printf(SSDK_INFO, "sw tuning not pass tuning %d\n", min);

    /* find the mininum delay first which can pass tuning */
    min = min + SEHC_TUNE_CTRL_STEP;
SW_TUNING:

    while (min < SEHC_TUNE_CTRL_MAX) {
        sehc_prepare_tuning(host, min);

        if (!mmc_send_tuning((struct mmc_device *)host->parent, opcode, bus_width))
            break;

        min += SEHC_TUNE_CTRL_STEP;
    }

    ssdk_printf(SSDK_INFO, "sw tuning pass tuning %d\n", min);

    /* find the mininum delay first which can not pass tuning */
    max = min + SEHC_TUNE_CTRL_STEP;

    while (max < SEHC_TUNE_CTRL_MAX) {
        sehc_prepare_tuning(host, max);

        if (mmc_send_tuning((struct mmc_device *)host->parent, opcode, bus_width))
            break;

        max += SEHC_TUNE_CTRL_STEP;
    }

    ssdk_printf(SSDK_INFO, "sw tuning not pass tuning %d\n", max);

    if (min >= SEHC_TUNE_CTRL_MAX) {
        ret = 1;
        ssdk_printf(SSDK_CRIT, "sw tuning find pass window error\n");
        goto out;
    }

    if ((max - min) > SEHC_TUNE_WINDOW_MIN) {
        avg = (min + max) / 2;
        ssdk_printf(SSDK_INFO, "sw tuning window %d ~ %d avg:%d\n", min, max, avg);
        sehc_prepare_tuning(host, avg);
        ret = mmc_send_tuning((struct mmc_device *)host->parent, opcode, bus_width);

        if (ret) {
            ssdk_printf(SSDK_CRIT, "sw tuning window %d ~ %d avg:%d, tuning cmd error\n",
                        min, max, avg);
            goto out;
        }
    }
    else {
        min = max + SEHC_TUNE_CTRL_STEP;
        ssdk_printf(SSDK_INFO, "sw tuning window < 5,goto sw_tuning\n");
        goto SW_TUNING;
    }

out:
    host->sw_tuning_in_progress = 0;
    return ret;

}

struct sdhci_ops sdhci_sehc_ops = {
    .priv_init = sehc_init,
    .hw_reset = sehc_hw_reset,
    .set_clock = sehc_set_clk,
    .set_uhs_mode = sehc_set_uhs_mode,
    .config_pin = sehc_config_pin,
    .platform_execute_tuning = sehc_sw_execute_tuning,
};

int mmc_platform_init(struct mmc_device *dev, void *priv_data)
{
    struct sdhci_host *host;
    struct mmc_config_data *mmc_cfg = &dev->config;
    clk_node_t *clk = mmc_cfg->clk;
    struct mmc_sehc *sehc = priv_data;

    host = &dev->host;
    host->priv_data = sehc;
    host->ops = &sdhci_sehc_ops;
    host->parent = dev;

    if (clk != NULL) {
        /* Init the ref clock: 400Khz */
        if (clk_set_rate(clk, 400000)) {
            ssdk_printf(SSDK_CRIT, "mmc set ref clock failed!\n");
            return -1;
        }
    }

    if (mmc_sdhci_init(dev)) {
        ssdk_printf(SSDK_CRIT, "mmc init failed!\n");
        return -1;
    }

    return 0;
}
