#include <stdlib.h>
#include <string.h>
#include <param.h>
#include <regs_base.h>
#include <cmsis_os2.h>
#include <debug.h>
#include <arch/cache.h>
#include <driver/clk.h>
#include <driver/uart.h>
#include <scr_hw.h>
#include <types.h>
#include <driver/arch_timer.h>

#ifdef CONFIG_VIRTIO_MMC_FRONTEND
#include <virtio_config.h>
#endif

#include "CLI.h"
#include "driver/kunlun/mmc/mmc_dwcmshc.h"
#include "driver/kunlun/mmc/mmc_sdhci.h"

//#define LOCAL_DEBUG_FLAG
static struct mmc_device mmc_dev = {0};
scr_dev_t src_base_dev = {
    .base = APB_SCR_SEC_BASE,
};
static struct mmc_dwcmshc priv_data = {
    .scr_base = &src_base_dev,
    .ddr_mode_scr = SCR_SEC__mshc1_ddr_mode,
};
static uint8_t tx_buf[512 * 35] __CACHE_ALIGN;
static uint8_t rx_buf[512 * 35] __CACHE_ALIGN;

#define MMC_RW_RATE_COUNT 64
static uint8_t rw_rate_buf[512 * MMC_RW_RATE_COUNT] __CACHE_ALIGN;

extern clk_node_t g_ckgen_ip_emmc1;

static void rstgen_module_rst(addr_t base, unsigned int idx, unsigned int rst_b)
{
    unsigned int rval;
    uint32_t count = 10000;
    const uint32_t module_rst_offset = 0x100 << 10;
    addr_t addr = base + module_rst_offset + ((0x4 * idx) << 10);
    rval = readl(addr);
    writel(((rval & (~(0x1 << 1))) | (0x1 << 1)), addr);

    //Wait module reset enable done.
    while ((count--) && (!((readl(addr) & (0x1 << 1)) == (0x1 << 1))))
        ;

    rval = readl(addr);
    writel(((rval & (~(0x1 << 0))) | rst_b), addr);
    //Wait module reset done.
    count = 10000;

    while ((count--) && ((readl(addr) & (0x1 << 30)) != rst_b << 30))
        ;
}

static int mmc_test_tansfer(struct mmc_device *mmc, uint32_t block,
                            uint32_t data,
                            uint32_t num)
{
    uint32_t ret = 0, capacity = 0;
    uint32_t block_size = MMC_BLK_SZ;
    uint32_t erase_grp_size;

    erase_grp_size = mmc_get_erase_group_size(mmc);
    capacity       = mmc_get_size(mmc, PART_ACCESS_DEFAULT);

    memset(tx_buf, 0, block_size * (num + 1));
    memset(rx_buf, 0, block_size * (num + 1));

    uint32_t i;
    volatile uint32_t *p1 = (volatile uint32_t *)tx_buf;
    volatile uint32_t *p2 = (volatile uint32_t *)rx_buf;

    for (i = 0; i < 128 * (num + 1); i++, p1++) {
        if (data == UINT32_MAX) {
            *p1 = rand();
        }
        else {
            *p1 = data;
        }
    }

#ifdef LOCAL_DEBUG_FLAG
    printf("mmc original data: %s!\n", tx_buf);
    hexdump(tx_buf, 512);
#endif

    if (mmc_sdhci_write(mmc, tx_buf, block, num)) {
        printf("mmc write failed!\n");
        return -1;
    }

    if (mmc_sdhci_read(mmc, rx_buf, block, num)) {
        printf("mmc read failed!\n");
        return -1;
    }

#ifdef LOCAL_DEBUG_FLAG
    printf("mmc read data: %s!\n", rx_buf);
    hexdump(rx_buf, 512);
#endif

    /* check data error */
    p1 = (volatile uint32_t *)tx_buf;

    for (i = 0; i < 128 * num; i++, p1++, p2++) {
        if (*p1 != *p2) {
            printf("\n mmc error: 0x%x - 0x%x : i=%d\n", *p1, *p2,
                   i);
            ret += 1;
            break;
        }
    }

    /* chack data exceed */
    p2 = (volatile uint32_t *)rx_buf + 128 * num;

    for (i = 0; i < 128; i++, p2++) {
        if (*p2 != 0) {
            printf("\n mmc exceed error: 0x%x : i=%d\n", *p2, i);
            ret += 1;
            break;
        }
    }

    block = capacity / erase_grp_size;

    if (block != 0) {
        if (mmc_sdhci_erase(mmc, 0, erase_grp_size * block)) {
            printf("mmc erase failed!\n");
            return -1;
        }

        if (mmc_sdhci_read(mmc, rx_buf, 0, num)) {
            printf("mmc read failed!\n");
            return -1;
        }

#ifdef LOCAL_DEBUG_FLAG
        printf("mmc read data: %s!\n", rx_buf);
        hexdump(rx_buf, 512);
#endif

        p2 = (volatile uint32_t *)rx_buf;

        for (i = 0; i < 128 * num; i++, p2++) {
            if (*p2 != 0) {
                printf("\n mmc erase error: 0x%x : i=%d, block=%d, erase_grp_size=%d\n", *p2, i,
                       block, erase_grp_size);
                ret += 1;
                break;
            }
        }
    }

    return ret;
}

static int mmc_test(int argc, char *argv[])
{
    int ret = 0;
    struct mmc_device *mmc = &mmc_dev;
    struct mmc_config_data *mmc_cfg = &mmc->config;
    uint32_t block, count, capacity;

    mmc_cfg->slot = 0;
    mmc_cfg->irq = 142;
    mmc_cfg->max_clk_rate = 200000000;
    mmc_cfg->bus_width = MMC_BUS_WIDTH_8BIT;
    mmc_cfg->voltage = SDHCI_EMMC_VOL_1_8;
    mmc_cfg->hs200_support = 0;
    mmc_cfg->hs400_support = 0;
    mmc_cfg->clk = &g_ckgen_ip_emmc1;
#if CONFIG_ARCH_ARM
    mmc_cfg->sdhc_base = 0xf4180000;
#else
    mmc_cfg->sdhc_base = 0x34180000;
#endif

    // reset mshc1
    rstgen_module_rst(APB_RSTGEN_SEC_BASE, 14, 0x1);

#ifdef CONFIG_VIRTIO_MMC_FRONTEND

    if (vmmc_platform_init(mmc, &priv_data)) {
#else

    if (mmc_platform_init(mmc, &priv_data)) {
#endif
        printf("platform init failed!\n");
        ret += 1;
        return ret;
    }

    capacity = mmc_get_size(mmc, PART_ACCESS_DEFAULT);
    block    = capacity / MMC_BLK_SZ;
    printf("mmc get capacity with %u, sector with count = %d\n", capacity, block);

    for (int i = 0; i < 100; i++) {
        count = (rand() % (block - 1)) + 1;
        count &= 0x1f;
        printf("%d: addr = 0x%x, count = %u\n", i, block - count, count);

        if (mmc_test_tansfer(mmc, block - count, UINT32_MAX, count)) {
            printf("mmc test failed, count = %d!\n", i);
            ret += 1;
            break;
        }
    }

    if (ret)
        printf("mmc test failed!\n");
    else
        printf("mmc test success!\n");

    return 0;
}

/*20
 * Register Internel CLI commands.
 */
CLI_CMD("mmc_test", "Start mmc test", mmc_test);


static int mmc_rw_rate(int argc, char *argv[])
{
    int ret = 0;
    struct mmc_device *mmc = &mmc_dev;
    struct mmc_config_data *mmc_cfg = &mmc->config;
    uint32_t block, count, capacity;
    uint32_t addr = 0;
    uint32_t size = 0;
    uint32_t block_num = MMC_RW_RATE_COUNT;
    uint32_t block_size = 0;
    uint32_t rw_count = 1000;
    uint64_t tick_satrt, tick_end, tick_avg;
    uint32_t rw_rate = 0;

    mmc_cfg->slot = 0;
    mmc_cfg->irq = 142;
    mmc_cfg->max_clk_rate = 200000000;
    mmc_cfg->bus_width = MMC_BUS_WIDTH_8BIT;
    mmc_cfg->voltage = SDHCI_EMMC_VOL_1_8;
    mmc_cfg->hs200_support = 0;
    mmc_cfg->hs400_support = 0;
    mmc_cfg->clk = &g_ckgen_ip_emmc1;
#if CONFIG_ARCH_ARM
    mmc_cfg->sdhc_base = 0xf4180000;
#else
    mmc_cfg->sdhc_base = 0x34180000;
#endif

    // reset mshc1
    rstgen_module_rst(APB_RSTGEN_SEC_BASE, 14, 0x1);

#ifdef CONFIG_VIRTIO_MMC_FRONTEND

    if (vmmc_platform_init(mmc, &priv_data)) {
#else

    if (mmc_platform_init(mmc, &priv_data)) {
#endif
        printf("platform init failed!\n");
        ret += 1;
        return ret;
    }

    capacity = mmc_get_size(mmc, PART_ACCESS_DEFAULT);
    block    = capacity / MMC_BLK_SZ;
    printf("mmc get capacity with %u, sector with count = %d\n", capacity, block);

    block_size = MMC_BLK_SZ;

    for (int i = 0; i < 3; i++) {
        printf("\n ***************************mmc rw rate test*****************\n");

        for (count = 1; count <= block_num; count *= 2) {
            uint8_t *mem = rw_rate_buf;

            addr = 0;
            tick_avg = 0;

            for (int j = 0; j < rw_count; j++) {
                tick_satrt = arch_timer_get_current();
                mmc_sdhci_read(mmc, mem, addr, count);
                tick_end = arch_timer_get_current();
                tick_avg += tick_end - tick_satrt;
                addr += count;
//                printf(" mmc rw rate test read tick_satrt:%lld tick_end:%lld\n", tick_satrt, tick_end);
            }

            size = count * block_size;
            tick_avg = tick_avg / rw_count;
            rw_rate = size / tick_avg;
            printf(" mmc rw rate test sequence read len:%x time_avg:%lldus\n", size,
                   tick_avg);
            printf(" mmc rw rate test sequence read rate:%d M/S\n", rw_rate);

            addr = 0;
            tick_avg = 0;

            for (int j = 0; j < rw_count; j++) {
                tick_satrt = arch_timer_get_current();
                mmc_sdhci_write(mmc, mem, addr, count);
                tick_end = arch_timer_get_current();
                tick_avg += tick_end - tick_satrt;
                addr += count;
//                printf(" mmc rw rate test read tick_satrt:%lld tick_end:%lld\n", tick_satrt, tick_end);
            }

            size = count * block_size;
            tick_avg = tick_avg / rw_count;
            rw_rate = size / tick_avg;
            printf(" mmc rw rate test sequence write len:%x time_avg:%lldus\n", size,
                   tick_avg);
            printf(" mmc rw rate test sequence write rate:%d M/S\n", rw_rate);

        }
    }

    for (int i = 0; i < 3; i++) {
        printf("\n ***************************mmc rw rate test*****************\n");

        for (count = 1; count <= block_num; count *= 2) {
            uint8_t *mem = rw_rate_buf;

            addr = 0;
            tick_avg = 0;

            for (int j = 0; j < rw_count; j++) {
                addr = rand() % 0xfff;
                tick_satrt = arch_timer_get_current();
                mmc_sdhci_read(mmc, mem, addr, count);
                tick_end = arch_timer_get_current();
                tick_avg += tick_end - tick_satrt;
//                printf(" mmc rw rate test read tick_satrt:%lld tick_end:%lld\n", tick_satrt, tick_end);
            }

            size = count * block_size;
            tick_avg = tick_avg / rw_count;
            rw_rate = size / tick_avg;
            printf(" mmc rw rate test Random read len:%x time_avg:%lldus\n", size,
                   tick_avg);
            printf(" mmc rw rate test Random read rate:%d M/S\n", rw_rate);

            addr = 0;
            tick_avg = 0;

            for (int j = 0; j < rw_count; j++) {
                addr = rand() % 0xfff;
                tick_satrt = arch_timer_get_current();
                mmc_sdhci_write(mmc, mem, addr, count);
                tick_end = arch_timer_get_current();
                tick_avg += tick_end - tick_satrt;
//                printf(" mmc rw rate test read tick_satrt:%lld tick_end:%lld\n", tick_satrt, tick_end);
            }

            size = count * block_size;
            tick_avg = tick_avg / rw_count;
            rw_rate = size / tick_avg;
            printf(" mmc rw rate test Random write len:%x time_avg:%lldus\n", size,
                   tick_avg);
            printf(" mmc rw rate test Random write rate:%d M/S\n", rw_rate);

        }
    }

    return 0;
}

CLI_CMD("mmc_rw_rate", "mmc_rw_rate", mmc_rw_rate);
