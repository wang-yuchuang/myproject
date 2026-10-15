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

#define LOCAL_DEBUG_FLAG 1

struct disk_dev dev;
static uint8_t tx_buf[512 * 35] __CACHE_ALIGN;
static uint8_t rx_buf[512 * 35] __CACHE_ALIGN;

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

static int disk_test_tansfer(struct disk_dev *dev, uint32_t addr, uint32_t size)
{
    uint32_t ret = 0;

    memset(tx_buf, 0, size + 512);
    memset(rx_buf, 0, size + 512);

    uint32_t i;
    volatile uint8_t *p1 = (volatile uint8_t *)tx_buf;
    volatile uint8_t *p2 = (volatile uint8_t *)rx_buf;

    for (i = 0; i < size; i++, p1++) {
        *p1 = rand();
    }

#ifdef LOCAL_DEBUG_FLAG
    printf("disk original data: %s!\n", tx_buf);
    hexdump8(tx_buf, 512);
#endif

    if (disk_write(dev, addr, tx_buf, size)) {
        printf("disk write failed!\n");
        return -1;
    }

    if (disk_read(dev, addr, rx_buf, size)) {
        printf("disk read failed!\n");
        return -1;
    }

#ifdef LOCAL_DEBUG_FLAG
    printf("disk read data: %s!\n", rx_buf);
    hexdump8(rx_buf, 512);
#endif

    /* check data error */
    p1 = (volatile uint8_t *)tx_buf;
    p2 = (volatile uint8_t *)rx_buf;
    for (i = 0; i < size; i++, p1++, p2++) {
        if (*p1 != *p2) {
            printf("\n disk error: 0x%x - 0x%x : i=%d\n", *p1, *p2,
                   i);
            ret += 1;
            break;
        }
    }

    /* chack data exceed */
    p2 = (volatile uint8_t *)rx_buf + addr;

    for (i = 0; i < 128; i++, p2++) {
        if (*p2 != 0) {
            printf("\n disk exceed error: 0x%x : i=%d\n", *p2, i);
            ret += 1;
            break;
        }
    }

    if (disk_erase_default(dev, addr, size)) {
        printf("disk erase failed!\n");
        return -1;
    }

    if (disk_read(dev, addr, rx_buf, size)) {
        printf("disk read failed!\n");
        return -1;
    }

#ifdef LOCAL_DEBUG_FLAG
    printf("disk read data: %s!\n", rx_buf);
    hexdump8(rx_buf, 512);
#endif

    p2 = (volatile uint8_t *)rx_buf;

    for (i = 0; i < size; i++, p2++) {
        if (*p2 != 0) {
            printf("\n disk erase error: 0x%x : i=%d, size=%d\n", *p2, i, size);
            ret += 1;
            break;
        }
    }

    return ret;
}

static int disk_test_block_tansfer(struct disk_dev *dev, uint32_t block, uint32_t count)
{
    uint32_t ret = 0;
    uint32_t block_size = disk_get_block_size(dev);

    memset(tx_buf, 0, block_size * (count + 1));
    memset(rx_buf, 0, block_size * (count + 1));

    uint32_t i;
    volatile uint8_t *p1 = (volatile uint8_t *)tx_buf;
    volatile uint8_t *p2 = (volatile uint8_t *)rx_buf;

    for (i = 0; i < block_size * count; i++, p1++) {
        *p1 = rand();
    }

#ifdef LOCAL_DEBUG_FLAG
    printf("disk original data: %s!\n", tx_buf);
    hexdump8(tx_buf, 512);
#endif

    if (disk_write_block(dev, block, tx_buf, count)) {
        printf("disk write failed!\n");
        return -1;
    }

    if (disk_read_block(dev, block, rx_buf, count)) {
        printf("disk read failed!\n");
        return -1;
    }

#ifdef LOCAL_DEBUG_FLAG
    printf("disk read data: %s!\n", rx_buf);
    hexdump8(rx_buf, 512);
#endif

    /* check data error */
    p1 = (volatile uint8_t *)tx_buf;
    p2 = (volatile uint8_t *)rx_buf;
    for (i = 0; i < block_size * count; i++, p1++, p2++) {
        if (*p1 != *p2) {
            printf("\n disk error: 0x%x - 0x%x : i=%d\n", *p1, *p2,
                   i);
            ret += 1;
            break;
        }
    }

    /* chack data exceed */
    p2 = (volatile uint8_t *)rx_buf + block_size * count;

    for (i = 0; i < block_size; i++, p2++) {
        if (*p2 != 0) {
            printf("\n disk exceed error: 0x%x : i=%d\n", *p2, i);
            ret += 1;
            break;
        }
    }

    if (disk_erase_default(dev, block * block_size, count * block_size)) {
        printf("disk erase failed!\n");
        return -1;
    }

    if (disk_read_block(dev, block, rx_buf, count)) {
        printf("disk read failed!\n");
        return -1;
    }

#ifdef LOCAL_DEBUG_FLAG
    printf("disk read data: %s!\n", rx_buf);
    hexdump8(rx_buf, 512);
#endif

    p2 = (volatile uint8_t *)rx_buf;

    for (i = 0; i < block_size * count; i++, p2++) {
        if (*p2 != 0) {
            printf("\n disk erase error: 0x%x : i=%d\n", *p2, i);
            ret += 1;
            break;
        }
    }

    return ret;
}

static int disk_ram_tst(int argc, char *argv[])
{
    uint32_t addr = 0;
    uint32_t size = 0;
    struct disk_dev *l_dev = &dev;

    disk_open(DISK_RAM_NAME(1), l_dev);

   printf("disk %s, size = %lld block size=%d erase size=%d!\n",
        l_dev->info->disk_name,disk_size(l_dev), disk_erase_size(l_dev), disk_get_block_size(l_dev));
   for (int i = 1; i < 5; i++) {
       addr = rand();
       addr &= 0xfff;
       size = rand();
       size &= 0xff;
       printf("%d: addr = 0x%x, size = %d \n",
              i, addr, size);

       if (disk_test_tansfer(l_dev , addr, size)) {
           printf("disk test failed, count = %d!\n", i);
           break;
       }
   }

   for (int i = 1; i < 5; i++) {
       addr = rand();
       addr &= 0xf;
       size = rand();
       size &= 0xf;
       printf("%d: addr = 0x%x, size = %d \n",
              i, addr, size);

       if (disk_test_block_tansfer(l_dev , addr, size)) {
           printf("disk test failed, count = %d!\n", i);
           break;
       }
   }

   disk_close(l_dev);

    return 0;
}

CLI_CMD("disk_ram_tst", "\r\ndisk_ram_tst:\r\n disk_ram_tst\r\n", disk_ram_tst);
