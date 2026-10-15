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
#include "CLI_type.h"

#include <lib/disk/disk.h>

//#define LOCAL_DEBUG_FLAG 1
//#define MULTI_PART 1

struct disk_dev mmc_dev;

static uint8_t mmc_tx_buf[512 * 9] __CACHE_ALIGN;
static uint8_t mmc_rx_buf[512 * 9] __CACHE_ALIGN;

#ifdef MULTI_PART
struct disk_dev boot1_dev;
struct disk_dev boot2_dev;

static uint8_t boot1_tx_buf[512 * 9] __CACHE_ALIGN;
static uint8_t boot1_rx_buf[512 * 9] __CACHE_ALIGN;

static uint8_t boot2_tx_buf[512 * 9] __CACHE_ALIGN;
static uint8_t boot2_rx_buf[512 * 9] __CACHE_ALIGN;
#endif

inline static int isprint(int c)
{
    return ((c >= 0x20) && (c < 0x7f));
}

inline static void hexdump8_sehc_ex(const void *ptr, size_t len,
                                    uint64_t disp_addr)
{
    addr_t address = (addr_t)ptr;
    size_t count;
    size_t i;
    const char *addr_fmt = ((disp_addr + len) > 0xFFFFFFFF)
                           ? "0x%016llx: "
                           : "0x%08llx: ";

    for (count = 0 ; count < len; count += 16) {
        printf(addr_fmt, disp_addr + count);
        for (i = 0; i < MIN(len - count, 16); i++) {
            printf("%02x ", *(const uint8_t *)(address + i));
        }

        for (; i < 16; i++) {
            printf("   ");
        }

        printf("|");

        for (i = 0; i < MIN(len - count, 16); i++) {
            char c = ((const char *)address)[i];
            printf("%c", isprint(c) ? c : '.');
        }

        printf("\n");
        address += 16;
    }
}

static inline void hexdump8(const void *ptr, size_t len)
{
    hexdump8_sehc_ex(ptr, len, (uint64_t)((addr_t)ptr));
//    hexdump8_ex(ptr, len, (uint64_t)((addr_t)ptr));
}

static int disk_mmc_test_tansfer(struct disk_dev *mmc_dev, uint64_t addr, uint64_t size)
{
    uint32_t ret = 0;
    uint32_t i;
    volatile uint8_t *p1 = NULL;
    volatile uint8_t *p2 = NULL;

    memset(mmc_tx_buf, 0, size + 512);
    memset(mmc_rx_buf, 0, size + 512);

    /* 生成随机数据 */
    p1 = (volatile uint8_t *)mmc_tx_buf;
    for (i = 0; i < size; i++, p1++) {
        *p1 = rand();
    }

#ifdef LOCAL_DEBUG_FLAG
    printf("disk original data mmc_tx_buf!\n");
    hexdump8(mmc_tx_buf, 64);
#endif


    /* 写入数据到mmc分区 */
    if (disk_write(mmc_dev, addr, mmc_tx_buf, size)) {
        printf("disk write failed!\n");
        return -1;
    }

    p2 = (volatile uint8_t *)mmc_rx_buf;
    if (disk_read(mmc_dev, addr, mmc_rx_buf, size)) {
        printf("disk read failed!\n");
        return -1;
    }

#ifdef LOCAL_DEBUG_FLAG
    printf("disk original data mmc_rx_buf!\n");
    hexdump8(mmc_rx_buf, 64);
#endif

    /* check data error */
    p1 = (volatile uint8_t *)mmc_tx_buf;
    p2 = (volatile uint8_t *)mmc_rx_buf;
    for (i = 0; i < size; i++, p1++, p2++) {
        if (*p1 != *p2) {
            printf("\n disk error: 0x%x - 0x%x : i=%d\n", *p1, *p2,
                   i);
            ret += 1;
            break;
        }
    }

    /* 擦除数据 */
    if (disk_erase_default(mmc_dev, addr, size)) {
        printf("disk erase failed!\n");
        return -1;
    }

    /* 读取判断 */
    if (disk_read(mmc_dev, addr, mmc_rx_buf, size)) {
        printf("disk read failed!\n");
        return -1;
    }

#ifdef LOCAL_DEBUG_FLAG
    printf("disk original data mmc_rx_buf!\n");
    hexdump8(mmc_rx_buf, 32);
#endif

    p2 = (volatile uint8_t *)mmc_rx_buf;
    for (i = 0; i < size; i++, p2++) {
        if (*p2 != 0) {
            printf("\n disk erase error: 0x%x : i=%d, size=%lld\n", *p2, i, size);
            ret += 1;
            break;
        }
    }

    return ret;
}

#ifdef MULTI_PART
static int disk_mmc_multi_test_tansfer(struct disk_dev *mmc_dev, struct disk_dev *boot1_dev, struct disk_dev *boot2_dev,
                uint32_t addr, uint32_t size)
{
    uint32_t ret = 0;
    uint32_t i;
    volatile uint8_t *p1 = NULL;
    volatile uint8_t *p2 = NULL;

    memset(mmc_tx_buf, 0, size + 512);
    memset(mmc_rx_buf, 0, size + 512);
    memset(boot1_tx_buf, 0, size + 512);
    memset(boot1_rx_buf, 0, size + 512);
    memset(boot2_tx_buf, 0, size + 512);
    memset(boot2_rx_buf, 0, size + 512);

    /* 生成随机数据 */
    p1 = (volatile uint8_t *)mmc_tx_buf;
    for (i = 0; i < size; i++, p1++) {
        *p1 = rand();
    }

    p1 = (volatile uint8_t *)boot1_tx_buf;
    for (i = 0; i < size; i++, p1++) {
        *p1 = rand();
    }

    p1 = (volatile uint8_t *)boot2_tx_buf;
    for (i = 0; i < size; i++, p1++) {
        *p1 = rand();
    }

#ifdef LOCAL_DEBUG_FLAG
    printf("disk original data mmc_tx_buf!\n");
    hexdump8(mmc_tx_buf, 64);

    printf("disk original data boot1_tx_buf!\n");
    hexdump8(boot1_tx_buf, 64);

    printf("disk original data boot2_tx_buf!\n");
    hexdump8(boot2_tx_buf, 64);
#endif


    /* 写入数据到mmc分区 */
    if (disk_write(mmc_dev, addr, mmc_tx_buf, size)) {
        printf("disk write failed!\n");
        return -1;
    }

    if (disk_write(boot1_dev, addr, boot1_tx_buf, size)) {
        printf("disk write failed!\n");
        return -1;
    }

    if (disk_write(boot2_dev, addr, boot2_tx_buf, size)) {
        printf("disk write failed!\n");
        return -1;
    }

    p2 = (volatile uint8_t *)mmc_rx_buf;
    if (disk_read(mmc_dev, addr, mmc_rx_buf, size)) {
        printf("disk read failed!\n");
        return -1;
    }

#ifdef LOCAL_DEBUG_FLAG
    printf("disk original data mmc_rx_buf!\n");
    hexdump8(mmc_rx_buf, 64);
#endif

    /* check data error */
    p1 = (volatile uint8_t *)mmc_tx_buf;
    p2 = (volatile uint8_t *)mmc_rx_buf;
    for (i = 0; i < size; i++, p1++, p2++) {
        if (*p1 != *p2) {
            printf("\n disk error: 0x%x - 0x%x : i=%d\n", *p1, *p2,
                   i);
            ret += 1;
            break;
        }
    }

    /* chack data exceed */
    p2 = (volatile uint8_t *)mmc_rx_buf + addr;
    for (i = 0; i < 128; i++, p2++) {
        if (*p2 != 0) {
            printf("\n disk exceed error: 0x%x : i=%d\n", *p2, i);
            ret += 1;
            break;
        }
    }

    p2 = (volatile uint8_t *)boot1_rx_buf;
    if (disk_read(boot1_dev, addr, boot1_rx_buf, size)) {
        printf("disk read failed!\n");
        return -1;
    }

#ifdef LOCAL_DEBUG_FLAG
    printf("disk original data boot1_rx_buf!\n");
    hexdump8(boot1_rx_buf, 64);
#endif

    /* check data error */
    p1 = (volatile uint8_t *)boot1_tx_buf;
    p2 = (volatile uint8_t *)boot1_rx_buf;
    for (i = 0; i < size; i++, p1++, p2++) {
        if (*p1 != *p2) {
            printf("\n disk error: 0x%x - 0x%x : i=%d\n", *p1, *p2,
                   i);
            ret += 1;
            break;
        }
    }

    /* chack data exceed */
    p2 = (volatile uint8_t *)boot1_rx_buf + addr;
    for (i = 0; i < 128; i++, p2++) {
        if (*p2 != 0) {
            printf("\n disk exceed error: 0x%x : i=%d\n", *p2, i);
            ret += 1;
            break;
        }
    }

    p2 = (volatile uint8_t *)boot2_rx_buf;
    if (disk_read(boot2_dev, addr, boot2_rx_buf, size)) {
        printf("disk read failed!\n");
        return -1;
    }

#ifdef LOCAL_DEBUG_FLAG
    printf("disk original data boot2_rx_buf!\n");
    hexdump8(boot2_rx_buf, 64);
#endif

    /* check data error */
    p1 = (volatile uint8_t *)boot2_tx_buf;
    p2 = (volatile uint8_t *)boot2_rx_buf;
    for (i = 0; i < size; i++, p1++, p2++) {
        if (*p1 != *p2) {
            printf("\n disk error: 0x%x - 0x%x : i=%d\n", *p1, *p2,
                   i);
            ret += 1;
            break;
        }
    }

    /* chack data exceed */
    p2 = (volatile uint8_t *)boot2_rx_buf + addr;
    for (i = 0; i < 128; i++, p2++) {
        if (*p2 != 0) {
            printf("\n disk exceed error: 0x%x : i=%d\n", *p2, i);
            ret += 1;
            break;
        }
    }

    /* 擦除数据 */
    if (disk_erase_default(mmc_dev, addr, size)) {
        printf("disk erase failed!\n");
        return -1;
    }

    if (disk_erase_default(boot1_dev, addr, size)) {
        printf("disk erase failed!\n");
        return -1;
    }

    if (disk_erase_default(boot2_dev, addr, size)) {
        printf("disk erase failed!\n");
        return -1;
    }

    /* 读取判断 */
    if (disk_read(mmc_dev, addr, mmc_rx_buf, size)) {
        printf("disk read failed!\n");
        return -1;
    }

#ifdef LOCAL_DEBUG_FLAG
    printf("disk original data mmc_rx_buf!\n");
    hexdump8(mmc_rx_buf, 32);
#endif

    p2 = (volatile uint8_t *)mmc_rx_buf;
    for (i = 0; i < size; i++, p2++) {
        if (*p2 != 0) {
            printf("\n disk erase error: 0x%x : i=%d, size=%d\n", *p2, i, size);
            ret += 1;
            break;
        }
    }

    if (disk_read(boot1_dev, addr, boot1_rx_buf, size)) {
        printf("disk read failed!\n");
        return -1;
    }

#ifdef LOCAL_DEBUG_FLAG
    printf("disk original data boot1_rx_buf!\n");
    hexdump8(boot1_rx_buf, 32);
#endif

    p2 = (volatile uint8_t *)boot1_rx_buf;
    for (i = 0; i < size; i++, p2++) {
        if (*p2 != 0) {
            printf("\n disk erase error: 0x%x : i=%d, size=%d\n", *p2, i, size);
            ret += 1;
            break;
        }
    }

    if (disk_read(boot2_dev, addr, boot2_rx_buf, size)) {
        printf("disk read failed!\n");
        return -1;
    }

#ifdef LOCAL_DEBUG_FLAG
    printf("disk original data boot2_rx_buf!\n");
    hexdump8(boot2_rx_buf, 32);
#endif

    p2 = (volatile uint8_t *)boot2_rx_buf;
    for (i = 0; i < size; i++, p2++) {
        if (*p2 != 0) {
            printf("\n disk erase error: 0x%x : i=%d, size=%d\n", *p2, i, size);
            ret += 1;
            break;
        }
    }

    return ret;
}
#endif

static int disk_mmc_tst(int argc, char *argv[])
{
    uint64_t addr = 0;
    uint64_t size = 0;
    uint64_t count;
    uint64_t rand_val = 0;
    struct disk_dev *l_dev = &mmc_dev;
#ifdef MULTI_PART
    struct disk_dev *l_dev_boot1 = &boot1_dev;
    struct disk_dev *l_dev_boot2 = &boot2_dev;
#endif

    if(argc != 1) {
        printf("The command format: disk_mmc_tst count\n");
        return 0;
    }

    count = strtol(argv[0], NULL, 10) * 1000;
    printf("count is %lld\n", count);

    disk_open(DISK_MMC_NAME(0), l_dev);
#ifdef MULTI_PART
    disk_open(DISK_MMC_BOOT_NAME(0,1), l_dev_boot1);
    disk_open(DISK_MMC_BOOT_NAME(0,2), l_dev_boot2);
#endif

    printf("disk %s, size = %lld block size=%d erase size=%d!\n",
        l_dev->info->disk_name,disk_size(l_dev), disk_erase_size(l_dev), disk_get_block_size(l_dev));
    for (uint64_t i = 1; i < count; i++) {
#ifdef MULTI_PART
       rand_val = rand();
       rand_val &= 0xf;
       addr += rand_val * 32;
       rand_val = rand();
       rand_val &= 0xff;
       addr += rand_val * 512;

       size = rand();
       size &= 0xfff;
#else
       rand_val = rand();
       rand_val &= 0xdfffff;
       addr = rand_val * 32;

       size = rand();
       size &= 0xfff;
#endif
       printf("%lld: addr = 0x%llx, size = %lld \n",
              i, addr, size);

#ifdef MULTI_PART
       if (disk_mmc_multi_test_tansfer(l_dev ,l_dev_boot1, l_dev_boot2, addr, size)) {
           printf("disk test failed, count = %lld!\n", i);
           break;
       }
#else
    if (disk_mmc_test_tansfer(l_dev, addr, size)) {
        printf("disk test failed, count = %lld!\n", i);
        break;
    }
#endif
    }
    disk_close(l_dev);
#ifdef MULTI_PART
    disk_close(l_dev_boot1);
    disk_close(l_dev_boot2);
#endif
    return 0;
}

CLI_CMD("disk_mmc_tst", "\r\ndisk_mmc_tst:\r\n disk_mmc_tst count\r\n", disk_mmc_tst);

static int disk_sd_tst(int argc, char *argv[])
{
    uint64_t addr = 0;
    uint64_t size = 0;
    uint64_t count;
    uint64_t rand_val = 0;
    struct disk_dev *l_dev = &mmc_dev;

    if(argc != 1) {
        printf("The command format: disk_mmc_tst count\n");
        return 0;
    }

    count = strtol(argv[0], NULL, 10) * 1000;
    printf("count is %lld\n", count);

    disk_open(DISK_MMC_NAME(1), l_dev);

    printf("disk %s, size = %lld block size=%d erase size=%d!\n",
        l_dev->info->disk_name,disk_size(l_dev), disk_erase_size(l_dev), disk_get_block_size(l_dev));
    for (uint64_t i = 1; i < count; i++) {
        rand_val = rand();
       rand_val &= 0xffff;
       addr = rand_val * 32;

       size = rand();
       size &= 0xfff;
       printf("%lld: addr = 0x%llx, size = %lld \n",
              i, addr, size);

    if (disk_mmc_test_tansfer(l_dev, addr, size)) {
        printf("disk test failed, count = %lld!\n", i);
        break;
    }
    }
    disk_close(l_dev);
    return 0;
}

CLI_CMD("disk_sd_tst", "\r\ndisk_sd_tst:\r\n disk_sd_tst count\r\n", disk_sd_tst);

static int disk_mmc_sdram(int argc, char *argv[])
{
    struct disk_dev *l_dev = &mmc_dev;
    uint32_t addr = 0x30000000;
    uint32_t size = 16 * 1024 * 1024;

    uint32_t check_addr = 0x32000000;
    uint32_t check_size = 2 * 1024 * 1024;
    uint32_t check_num = size / check_size;

    disk_open(DISK_MMC_NAME(0), l_dev);

    printf("disk %s, size = %lld block size=%d erase size=%d!\n",
        l_dev->info->disk_name,disk_size(l_dev), disk_erase_size(l_dev), disk_get_block_size(l_dev));

    uint8_t *p1 = (uint8_t *)addr;
    uint8_t *p2 = (uint8_t *)check_addr;
    for (int i = 0; i < size; i++, p1++) {
        *p1 = rand();
    }

    /* 写入数据到mmc分区 */
    if (disk_write(l_dev, 0x0, (uint8_t *)addr, size)) {
        printf("disk write failed!\n");
        return 0;
    }

    for(int i = 0; i < check_num; i++) {
        if(disk_read(l_dev, 0x0 + i * check_size, (uint8_t *)check_addr, check_size)) {
            printf("disk read failed!\n");
            return 0;
        }
        /* check data error */
        p1 = (uint8_t *)addr + i * check_size;
        p2 = (uint8_t *)check_addr;
        for (int j = 0; j < check_size; j++, p1++, p2++) {
            if (*p1 != *p2) {
                printf("\n disk error: 0x%x - 0x%x : i=%d\n", *p1, *p2,
                       j);
                goto out;
            }
        }
    }
    printf("\n disk check write addr: 0x%x size: 0x%x ok\n",addr, size);

    for(int i = 0; i < check_num; i++) {
        p1 = (uint8_t *)addr + i * check_size;
        for (int j = 0; j < check_size; j++, p1++) {
            *p1 = rand();
        }
        /* 写入数据到mmc分区 */
        if (disk_write(l_dev, 0x0 + i * check_size, (uint8_t *)addr + i * check_size, check_size)) {
            printf("disk write failed!\n");
            return 0;
        }
    }

    if(disk_read(l_dev, 0x0, (uint8_t *)check_addr, size)) {
        printf("disk read failed!\n");
        return 0;
    }

    p1 = (uint8_t *)addr;
    p2 = (uint8_t *)check_addr;
    for(int i = 0; i < size; i++, p1++, p2++) {
        if (*p1 != *p2) {
            printf("\n disk error: 0x%x - 0x%x : i=%d\n", *p1, *p2, i);
            goto out;
        }
    }
    printf("\n disk check read addr: 0x%x size: 0x%x ok\n",addr, size);

out:
    disk_close(l_dev);
    return 0;
}
CLI_CMD("disk_mmc_sdram", "\r\ndisk_mmc_sdram:\r\n disk_mmc_sdram\r\n", disk_mmc_sdram);

