//*****************************************************************************
//
// Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
// Software License Agreement
//
//*****************************************************************************

#ifndef BOARDS_MMC_CFG_H
#define BOARDS_MMC_CFG_H

#if defined(CONFIG_KUNLUN_MMC) && defined(CONFIG_DISK_MMC)

#include <driver/kunlun/mmc/mmc_sdhci.h>
#include <driver/kunlun/mmc/sdhci.h>
#include <lib/disk/disk.h>
#include <lib/disk/disk_mmc.h>

/* mmc part index */
typedef enum {
    DISK_MMC_PART_DEFAULT = 0,
    DISK_MMC_PART_NUM,
} mmc_part;

#define DISK_MMC_PART(n) BIT(n)

/* mmc device config */
struct mmc_dev_config {
    /* disk device name */
    uint8_t slot;          /* Sdcc slot used */
    addr_t sdhc_base;      /* Base address for the sdhc */
    uint32_t irq;          /* irq num */
    uint16_t bus_width;    /* Bus width used */
    uint32_t max_clk_rate; /* Max clock rate supported */
    uint32_t voltage;      /* bus voltage */
    uint8_t ddr50_support; /* SDHC ddr50 mode supported or not */
    uint8_t hs200_support; /* SDHC HS200 mode supported or not */
    uint8_t hs400_support; /* SDHC HS400 mode supported or not */
    uint8_t hw_reset_support; /* SDHC hardware reset supported or not */
    uint8_t use_io_switch; /* IO pad switch flag for shared sdc controller */
    void *clk;             /* Clock config data */
    void *clk_low;         /* Clock LOW config data */
};

/* mmc part config */
struct mmc_part_config {
    /* disk device name */
    char *disk_name;
};

/* mmc config */
struct mmc_config {
    struct mmc_dev_config *mmc_config;
    uint32_t part_info;
    struct mmc_part_config *part_config[DISK_MMC_PART_NUM];
};

/* mmc info */
struct mmc_info {
    struct mmc_config config;
    struct mmc_device dev;
    struct disk_mmc_info info[DISK_MMC_PART_NUM];
};

/* mmc part default API */
static inline int mmc_part_default_read(void *mmc_dev, uint8_t *dst,
                                        block_t block, block_count_t count)
{
    return mmc_sdhci_read_part((struct mmc_device *)mmc_dev, dst, block, count,
                               PART_ACCESS_DEFAULT);
}

static inline int mmc_part_default_write(void *mmc_dev, const uint8_t *src,
        block_t block, block_count_t count)
{
    return mmc_sdhci_write_part((struct mmc_device *)mmc_dev, (void *)src, block, count,
                                PART_ACCESS_DEFAULT);
}

static inline int mmc_part_default_erase(void *mmc_dev, block_t block,
        disk_size_t size)
{
    return mmc_sdhci_erase_part(mmc_dev, block, size, PART_ACCESS_DEFAULT);
}

/* mmc init functions */
extern void board_mmc_init(void);

#endif

#endif /* BOARDS_MMC_CFG_H */
