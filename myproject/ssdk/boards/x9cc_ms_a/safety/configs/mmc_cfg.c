/*
 * Copyright (c) 2021-2029 Semidrive Incorporated.
 * All rights reserved.
 * Software License Agreement.
 */
#if defined(CONFIG_KUNLUN_MMC) && defined(CONFIG_DISK_MMC)

#include <arch/barriers.h>
#include <arch/cache.h>
#include <board.h>
#include <cmsis_os2.h>
#include <debug.h>
#include <driver/clk.h>
#include <driver/scr.h>
#include <driver/uart.h>
#include <param.h>
#include <regs_base.h>
#include <stdlib.h>
#include <string.h>
#include <types.h>

#include "clock_cfg.h"
#include "ospi_cfg.h"
#include "rstgen_cfg.h"
#include "scr_cfg.h"
#include "scr_hw.h"

#include <virtio_config.h>

#include "lib/disk/disk_mmc.h"
#include "driver/kunlun/mmc/mmc_dwcmshc.h"
#include "driver/kunlun/mmc/mmc_sdhci.h"
#include "mmc_cfg.h"

// static struct mmc_device mmc_dev = {0};

/** mmc device config */
struct mmc_dev_config dev_config = {
    .slot = 0,
    .irq = 142,
    .max_clk_rate = 200000000,
    .bus_width = MMC_BUS_WIDTH_8BIT,
    .voltage = SDHCI_EMMC_VOL_1_8,
    .hs200_support = 0,
    .hs400_support = 0,
    .clk = &g_ckgen_ip_emmc1,
#if CONFIG_ARCH_ARM
    .sdhc_base = 0xf4180000,
#else
    .sdhc_base = 0x34180000,
#endif
};

/** mmc part config */
struct mmc_part_config def_part_config = {
    .disk_name = DISK_MMC_NAME(0),
};

/** mmc device disk info */
static struct mmc_info vmmc_info = {
    .config = {
        .part_config = &def_part_config,
        .dev_config = &dev_config,
    },
};

int32_t mmc_prepare(struct mmc_info *mmc_info)
{
    struct mmc_config* mmc_cfg = &mmc_info->config;
    /* config & init mmc device */
    struct mmc_device *mmc = &mmc_info->dev;
    struct mmc_config_data *mmc_cfg_data = &mmc->config;
    struct mmc_dev_config *mmc_dev_cfg = mmc_cfg->dev_config;
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

    /** register mmc device */
    if (vmmc_platform_init(mmc, NULL)) {
        ssdk_printf(SSDK_CRIT, "platform init mmc%d failed!\n",mmc_cfg_data->slot);
        return 1;
    }


    struct disk_mmc_info *info = &mmc_info->info;
    struct mmc_part_config *part_cfg = mmc_cfg->part_config;
    struct disk_mmc_config *disk_mmc_cfg = &info->config;

    /** disk info config */
    disk_mmc_cfg->disk_name = part_cfg->disk_name;
    disk_mmc_cfg->disk_size = mmc_get_size(mmc, PART_ACCESS_DEFAULT);
    disk_mmc_cfg->erase_size = mmc_get_erase_group_size(mmc);
    disk_mmc_cfg->sector_size = mmc_get_block_size(mmc);
    disk_mmc_cfg->mmc_dev = (void *)mmc;

    disk_mmc_cfg->mmc_read = mmc_part_default_read;
    disk_mmc_cfg->mmc_write = mmc_part_default_write;
    disk_mmc_cfg->mmc_erase = mmc_part_default_erase;

    /** register disk */
    if (register_mmc_disk(info)) {
        ssdk_printf(SSDK_CRIT, "platform init disk %s failed!\n",disk_mmc_cfg->disk_name);
        return 1;
    }
    return 0;
}


void board_mmc_init(void)
{
    osDelay(500);
    mmc_prepare(&vmmc_info);
}

#endif
