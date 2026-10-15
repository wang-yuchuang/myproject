
//*****************************************************************************
//
// Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
// Software License Agreement
//
//*****************************************************************************

#if defined(CONFIG_KUNLUN_MMC) && defined(CONFIG_DISK_MMC)

#include <stdlib.h>
#include <regs_base.h>
#include <cmsis_os2.h>
#include <arch/cache.h>
#include <debug.h>
#include <driver/clk.h>
#include <driver/uart.h>
#include <types.h>
#include <string.h>
#include <param.h>
#include <board.h>

#include <scr_hw.h>
#include "clock_cfg.h"
#include "mmc_cfg.h"
#include "driver/kunlun/mmc/mmc_dwcmshc.h"

/* mmc device0 config */
struct mmc_dev_config mmc_con0 = {
    .slot = 0,
    .irq = MSHC1_INTR_NUM,
    .sdhc_base = SD1_BASE,
    .clk = CLK_NODE(g_ckgen_ip_emmc1),
    .max_clk_rate = 200000000,
    .bus_width = MMC_BUS_WIDTH_8BIT,
    .voltage = SDHCI_VOL_1_8,
    .ddr50_support = 0,
    .hs200_support = 0,
    .hs400_support = 0,
    .hw_reset_support = 1,
};

/* mmc device0 part config */
static struct mmc_part_config mmc0_part_def = {
    .disk_name = DISK_MMC_NAME(0),
};

/* mmc info0 */
static struct mmc_info mmc_info0 = {
    .config = {
        .mmc_config = &mmc_con0,
        .part_info = DISK_MMC_PART(DISK_MMC_PART_DEFAULT),
        .part_config[DISK_MMC_PART_DEFAULT] = &mmc0_part_def,
    },
};

const struct mmc_info *board_mmc[] = {
    &mmc_info0,
};

static scr_dev_t src_base_dev = {
   .base = APB_SCR_SEC_BASE,
};
static struct mmc_dwcmshc priv_data = {
    .scr_base = &src_base_dev,
    .ddr_mode_scr = SCR_SEC__mshc1_ddr_mode,
};

int mmc_prepare(struct mmc_info *mmc_info)
{
    struct mmc_config *mmc_cfg = &mmc_info->config;

    /* config & init mmc device */
    struct mmc_device *mmc = &mmc_info->dev;
    struct mmc_config_data *mmc_cfg_data = &mmc->config;
    struct mmc_dev_config *mmc_dev_cfg = mmc_cfg->mmc_config;

    mmc_cfg_data->slot = mmc_dev_cfg->slot;
    mmc_cfg_data->sdhc_base = mmc_dev_cfg->sdhc_base;
    mmc_cfg_data->irq = mmc_dev_cfg->irq;
    mmc_cfg_data->bus_width = mmc_dev_cfg->bus_width;
    mmc_cfg_data->max_clk_rate = mmc_dev_cfg->max_clk_rate;
    mmc_cfg_data->voltage = mmc_dev_cfg->voltage;
    mmc_cfg_data->ddr50_support = mmc_dev_cfg->ddr50_support;
    mmc_cfg_data->hs200_support = mmc_dev_cfg->hs200_support;
    mmc_cfg_data->hs400_support = mmc_dev_cfg->hs400_support;
    mmc_cfg_data->hw_reset_support = mmc_dev_cfg->hw_reset_support;
    mmc_cfg_data->use_io_switch = 0;
    mmc_cfg_data->clk = mmc_dev_cfg->clk;

    /* register mmc device */
    if (mmc_platform_init(mmc, &priv_data)) {
        ssdk_printf(SSDK_CRIT, "platform init mmc%d failed!\n",mmc_cfg_data->slot);
        return 1;
    }

    /* loop to initialize MMC Part */
    for (mmc_part part_inx = DISK_MMC_PART_DEFAULT; part_inx < DISK_MMC_PART_NUM;
            part_inx++) {
        /* config & init mmc disk */
        if ((mmc_cfg->part_info & DISK_MMC_PART(part_inx))
                && (mmc_cfg->part_config[part_inx])) {

            struct disk_mmc_info *info = &mmc_info->info[part_inx];
            struct mmc_part_config *part_cfg = mmc_cfg->part_config[part_inx];
            struct disk_mmc_config *disk_mmc_cfg = &info->config;

            /* disk info config */
            disk_mmc_cfg->disk_name = part_cfg->disk_name;
            disk_mmc_cfg->disk_size = mmc_get_size(mmc, part_inx);
            disk_mmc_cfg->erase_size = mmc_get_erase_group_size(mmc);
            disk_mmc_cfg->sector_size = mmc_get_block_size(mmc);
            disk_mmc_cfg->mmc_dev = (void *)mmc;

            switch (part_inx) {
                case DISK_MMC_PART_DEFAULT:
                    disk_mmc_cfg->mmc_read = mmc_part_default_read;
                    disk_mmc_cfg->mmc_write = mmc_part_default_write;
                    disk_mmc_cfg->mmc_erase = mmc_part_default_erase;
                    break;

                default:
                    break;
            }

            /* register mmc disk */
            if (register_mmc_disk(info)) {
                ssdk_printf(SSDK_CRIT, "platform init disk %s failed!\n",disk_mmc_cfg->disk_name);
                return 1;
            }
        }
    }

    return 0;
}

void board_mmc_init(void)
{
    /* Prepare mmc & disk tree. */
    for (uint32_t i = 0; i < ARRAY_SIZE(board_mmc); i++) {
        mmc_prepare((struct mmc_info *)board_mmc[i]);
    }
}
#endif
