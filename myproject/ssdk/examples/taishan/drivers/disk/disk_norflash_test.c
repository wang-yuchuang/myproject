#include <lib/disk/disk.h>
#include <debug.h>
#include <param.h>
#include <CLI_type.h>
#include <cmsis_os2.h>

#define USE_PRBS7_TEST_PATTERN

static uint8_t buf[4096 * 2] __ALIGNED(32) = {0};
static uint8_t rx_buf[4096 * 2 + 32] __ALIGNED(32) = {0};
static uint8_t prbs7[512] = {0};

int prbs7_pattern_gen(uint8_t *buf, uint32_t len, bool repeat)
{
    uint8_t start = 0x01;
    uint8_t a = start;
    int i;

    printf("The prbs7 data:\n");
    for (i = 1; i < len; i++) {
        uint8_t newbit = (((a >> 6) ^ (a >> 5)) & 1);
        printf ("%x", newbit);
        a = ((a << 1) | newbit) & 0x7f;
        buf[i - 1] = newbit;

        if (!repeat) {
            if (a == start) {
                printf("\n repetition period is %d\n", i);
                return i;
            }
        }

        if (i % 7 == 0) {
            printf (" ");
        }
    }
    printf("\n");

    if (repeat)
        return i;

    return -1;
}

#ifdef CONFIG_DISK_NOR_FLASH_BYTES_API_SUPPORT
static int disk_norflash_test(int  argc, char *argv[])
{
    int ret;
    struct disk_dev dev = {0};

    ssdk_printf(SSDK_EMERG, "disk norflash test start\n");

    ret = disk_open(DISK_NOR_FLASH_NAME(0), &dev);
    if (ret) {
        ssdk_printf(SSDK_EMERG, "disk_open error\n");
        return CLI_FALSE;
    }

    for (int i = 0; i < sizeof(buf); i++) {
        buf[i] = i % 255;
    }

    if (disk_write(&dev, 600, buf, sizeof(buf))) {
        ssdk_printf(SSDK_EMERG, "disk_write error\n");
        return CLI_FALSE;
    }

    memset(buf, 0, sizeof(buf));
    if (disk_read(&dev, 600, buf, sizeof(buf))) {
        ssdk_printf(SSDK_EMERG, "disk_read error\n");
        return CLI_FALSE;
    }

    for (int i = 0; i < sizeof(buf); i++) {
        if (buf[i] != i % 255) {
            ssdk_printf(SSDK_EMERG, "index: %d, beval: %x, afval: %x\n",
                        i, i % 255, buf[i]);
        }
    }

    if (disk_write(&dev, 0, buf, 600)) {
        ssdk_printf(SSDK_EMERG, "disk_write error\n");
        return CLI_FALSE;
    }

    if (disk_write(&dev, 600 + 0x1000, buf, 0x1000 - 600)) {
        ssdk_printf(SSDK_EMERG, "disk_write error\n");
        return CLI_FALSE;
    }

    if (disk_erase(&dev, 600, 0x1000, DISK_ERASE_DEFAULT)) {
        ssdk_printf(SSDK_EMERG, "disk_erase error\n");
        return CLI_FALSE;
    }

    memset(buf, 0, sizeof(buf));
    if (disk_read(&dev, 0, buf, sizeof(buf))) {
        ssdk_printf(SSDK_EMERG, "disk_read error\n");
        return CLI_FALSE;
    }

    for (int i = 0; i < 0x2000; i++) {
        if (i >= 600 && i < (600 + 0x1000)) {
            if (buf[i] != 0xff) {
                ssdk_printf(SSDK_EMERG, "index: %d, beval: %x, afval: %x\n",
                            i, 0xff, buf[i]);
            }
        } else {
            if (buf[i] == 0xff) {
                ssdk_printf(SSDK_EMERG, "index: %d, beval: %x, afval: %x\n",
                            i, 0xff, buf[i]);
            }
        }
    }

    disk_close(&dev);

    ssdk_printf(SSDK_EMERG, "disk norflash test end\n");

    return CLI_FALSE;
}

static int disk_norflash_rand_xfer(uint32_t addr, uint32_t len, uint32_t mask)
{
    int ret = 0;
    uint32_t i;
    uint32_t exceed_len;
    volatile uint32_t *p1 = (volatile uint32_t *)buf;
    volatile uint32_t *p2 = (volatile uint32_t *)rx_buf;
    struct disk_dev dev = {0};

    ssdk_printf(SSDK_EMERG, "disk norflash rand test start\n");

    ret = disk_open(DISK_NOR_FLASH_NAME(0), &dev);
    if (ret) {
        ssdk_printf(SSDK_EMERG, "disk_open error\n");
        return -1;
    }

    struct disk_info *info = dev.info;
    uint32_t erase_size = info->erase_size;

    memset(buf, 0, len);
    memset(rx_buf, 0x55, len + 32);

    for (i = 0; i < len / 4; i++, p1++) {
        *p1 = rand();
    }

    /* erase the exceed check data's sector */
    if (disk_erase(&dev, ROUNDDOWN(addr, erase_size), ROUNDUP(addr % erase_size + len + 16, erase_size), DISK_ERASE_DEFAULT)) {
        ssdk_printf(SSDK_EMERG, "%s:%d: disk_erase error\n", __func__, __LINE__);
        ret = -1;
        goto release_handle;
    }

    if (disk_write(&dev, addr, buf, len)) {
        ssdk_printf(SSDK_EMERG, "%s:%d: disk_write error\n", __func__, __LINE__);
        ret = -1;
        goto release_handle;
    }

    /* If data in two sectors */
    if (ROUNDDOWN(addr + len, erase_size) > addr) {
        exceed_len = len + 16u;
    } else {
        exceed_len = MIN(len + 16u, erase_size - addr % erase_size);
    }

    if (disk_read(&dev, addr, rx_buf, exceed_len)) {
        ssdk_printf(SSDK_EMERG, "%s:%d: disk_read error\n", __func__, __LINE__);
        ret = -1;
        goto release_handle;
    }

    /* check data error */
    p1 = (volatile uint32_t *)buf;
    p2 = (volatile uint32_t *)rx_buf;
    for (i = 0; i < len / 4; i++, p1++, p2++) {
        if ((*p1 & mask) != (*p2 & mask)) {
            ssdk_printf(SSDK_EMERG, "\n%s:%d: data error: 0x%x - 0x%x : i=%d\n",
                __func__, __LINE__, *p1, *p2, i);
            ret = -1;
            goto release_handle;
        }
    }

    /* chack data exceed */
    for (i = 0; i < (exceed_len - len); i++) {
        if (*(rx_buf + len + i) != 0xff) {
            ssdk_printf(SSDK_EMERG, "\n%s:%d: data exceed error: 0x%x : i=%d\n",
                        __func__, __LINE__, *(rx_buf + len + i), i);
            ret = -1;
        }
    }

release_handle:
    disk_close(&dev);
    if (ret == 0u) {
        ssdk_printf(SSDK_EMERG, "disk norflash rand test succ\n");
    }
    return ret;
}

static int disk_norflash_rand_test(int argc, char *argv[])
{
    uint32_t addr = strtoul(argv[0], 0, 0);
    uint32_t len = strtoul(argv[1], 0, 0);
    uint32_t mask = strtoul(argv[2], 0, 0);

    disk_norflash_rand_xfer(addr, len, mask);

    return CLI_FALSE;
}

static int disk_norflash_stress_test(int argc, char *argv[])
{
    uint32_t addr;
    uint32_t len;
    uint32_t mask = 0xffffffff; // data mask for test part data pin

    uint32_t times = strtoul(argv[0], 0, 0);

    for (int i = 0; i < times; i++) {
        addr = (rand() & 0x6fffff);
        addr = ROUNDDOWN(addr, 4u); // addr 4bytes aligned
        len = rand() & 0x1fff;
        len = ROUNDUP(len, 4u); // addr 4bytes aligned
        len = ((len + addr) > 0x700000) ? (0x700000 - addr) : len;

        ssdk_printf(SSDK_EMERG,"\n disk norflash stress test times = %d, addr = 0x%x, lens = %d\n", i, addr, len);

        if (disk_norflash_rand_xfer(addr, len, mask)) {
            ssdk_printf(SSDK_EMERG, "\n disk norflash stress test failed\n");
            break;
        }
        osDelay(10);
    }

    return CLI_FALSE;
}
#endif

static int disk_norflash_xfer(int argc, char *argv[])
{
    int ret = 0;
    uint32_t addr = 0x100000;
    uint32_t len = 8192;
    uint32_t mask = 0xffffffff;
    uint32_t i;
    volatile uint32_t *p1 = (volatile uint32_t *)buf;
    volatile uint32_t *p2 = (volatile uint32_t *)rx_buf;
    struct disk_dev dev = {0};

    if (argc != 0)
        addr = strtoul(argv[0], 0, 0);

    ssdk_printf(SSDK_EMERG, "disk norflash rand test start\n");

    ret = disk_open(DISK_NOR_FLASH_NAME(0), &dev);
    if (ret) {
        ssdk_printf(SSDK_EMERG, "disk_open error\n");
        return -1;
    }

    struct disk_info *info = dev.info;
    uint32_t erase_size = info->erase_size;

    memset(buf, 0, len);
    memset(rx_buf, 0x55, len + 32);

    for (i = 0; i < len / 4; i++, p1++) {
        *p1 = rand();
    }

    printf("disk test addr = 0x%x, len = 0x%x, block size = %d\n", addr, len, dev.block_size);
    if (disk_erase_group(&dev, addr / erase_size,
                   DIV_ROUND_UP(len, erase_size) + 1, DISK_ERASE_DEFAULT)) {
        ssdk_printf(SSDK_EMERG, "%s:%d: disk_erase error\n", __func__, __LINE__);
        ret = -1;
        goto release_handle;
    }

    if (disk_write_block(&dev, addr / dev.block_size, buf, len / dev.block_size)) {
        ssdk_printf(SSDK_EMERG, "%s:%d: disk_write error\n", __func__, __LINE__);
        ret = -1;
        goto release_handle;
    }

    if (disk_read_block(&dev, addr / dev.block_size, rx_buf, (len + 32) / dev.block_size)) {
        ssdk_printf(SSDK_EMERG, "%s:%d: disk_read error\n", __func__, __LINE__);
        ret = -1;
        goto release_handle;
    }

    /* check data error */
    p1 = (volatile uint32_t *)buf;
    p2 = (volatile uint32_t *)rx_buf;
    for (i = 0; i < len / 4; i++, p1++, p2++) {
        if ((*p1 & mask) != (*p2 & mask)) {
            ssdk_printf(SSDK_EMERG, "\n%s:%d: data error: 0x%x - 0x%x : i=%d addr = 0x%x\n",
                __func__, __LINE__, *p1, *p2, i, addr + i * 4);
            printf("origin data: \n");
            hexdump((const void *)p1, 32);
            printf("read data: \n");
            hexdump((const void *)p2, 32);
            ret = -1;
            goto release_handle;
        }
    }

    /* chack data exceed */
    for (i = 0; i < 32; i++, p2++) {
        if (*(rx_buf + len + 1) != 0xff) {
            ssdk_printf(SSDK_EMERG, "\n%s:%d: data exceed error: 0x%x : i=%d\n",
                        __func__, __LINE__, *(rx_buf + len + 1), i);
            ret = -1;
        }
    }

release_handle:
    disk_close(&dev);
    if (ret == 0u) {
        ssdk_printf(SSDK_EMERG, "disk norflash rand test succ\n");
    }
    return CLI_FALSE;
}

static int disk_norflash_erase(int argc, char *argv[])
{
    int ret;
    uint32_t addr = strtoul(argv[0], 0, 0);
    uint32_t len = 256;
    struct disk_dev dev = {0};

    ret = disk_open(DISK_NOR_FLASH_NAME(0), &dev);
    if (ret) {
        ssdk_printf(SSDK_EMERG, "disk_open error\n");
        return -1;
    }

    struct disk_info *info = dev.info;
    uint32_t erase_size = info->erase_size;

    printf("disk test erase addr = 0x%x, block size = %d\n", addr, dev.block_size);
    if (disk_erase_group(&dev, addr / erase_size, DIV_ROUND_UP(len, erase_size),
                         DISK_ERASE_DEFAULT)) {
        ssdk_printf(SSDK_EMERG, "%s:%d: disk_erase error\n", __func__, __LINE__);
        ret = -1;
        goto release_handle;
    }


release_handle:
    disk_close(&dev);
    if (ret == 0u) {
        ssdk_printf(SSDK_EMERG, "disk norflash erase test succ\n");
    }
    return CLI_FALSE;
}

static int disk_norflash_write(int argc, char *argv[])
{
    int ret;
    uint32_t addr = strtoul(argv[0], 0, 0);
    uint32_t len = 256;
    struct disk_dev dev = {0};

    memset(buf, 0, len);

#ifdef USE_PRBS7_TEST_PATTERN
    int period = prbs7_pattern_gen(prbs7, 512, true);
    if (period < 0) {
        return -1;
    }

    for (int i = 0; i < len; i++) {
        for (int j = 0; j < 8; j++) {
            buf[i] |= (prbs7[i+j] << j);
        }
    }
#else
    uint32_t i;
    volatile uint32_t *p1 = (volatile uint32_t *)buf;

    for (i = 0; i < len / 4; i++, p1++) {
        *p1 = 0x55aa55aa;
    }
#endif
    hexdump((uint32_t *)buf, len);

    ret = disk_open(DISK_NOR_FLASH_NAME(0), &dev);
    if (ret) {
        ssdk_printf(SSDK_EMERG, "disk_open error\n");
        return -1;
    }

    printf("disk test write addr = 0x%x, block size = %d\n", addr, dev.block_size);
    if (disk_write_block(&dev, addr / dev.block_size, buf, len / dev.block_size)) {
        ssdk_printf(SSDK_EMERG, "%s:%d: disk_write error\n", __func__, __LINE__);
        ret = -1;
        goto release_handle;
    }

release_handle:
    disk_close(&dev);
    if (ret == 0u) {
        ssdk_printf(SSDK_EMERG, "disk norflash write test succ\n");
    }
    return CLI_FALSE;
}

static int disk_norflash_read(int argc, char *argv[])
{
    int ret;
    uint32_t times = 1;
    uint32_t addr = strtoul(argv[0], 0, 0);
    uint32_t len = 256;
    struct disk_dev dev = {0};

    if (argc == 2) {
        times = strtoul(argv[1], 0, 0);
    }

    memset(rx_buf, 0, len);

    ret = disk_open(DISK_NOR_FLASH_NAME(0), &dev);
    if (ret) {
        ssdk_printf(SSDK_EMERG, "disk_open error\n");
        return -1;
    }

    printf("disk test read addr = 0x%x, block size = %d\n", addr, dev.block_size);
    for (int i = 0; i < times; i++) {
        printf("disk test read times = %d\n", i + 1);
        if (disk_read_block(&dev, addr / dev.block_size, rx_buf, (len + 32) / dev.block_size)) {
            ssdk_printf(SSDK_EMERG, "%s:%d: disk_read error\n", __func__, __LINE__);
            ret = -1;
            goto release_handle;
        }

        hexdump((uint32_t *)rx_buf, len);
    }

release_handle:
    disk_close(&dev);
    if (ret == 0u) {
        ssdk_printf(SSDK_EMERG, "disk norflash read test succ\n");
    }
    return CLI_FALSE;
}

#ifdef CONFIG_DISK_NOR_FLASH_BYTES_API_SUPPORT
CLI_CMD("disk_norflash_test", "\r\ndisk_norflash_test\r\n disk norflash test\r\n", disk_norflash_test);
CLI_CMD("disk_norflash_rand_test", "\r\ndisk_norflash_rand_test\r\n disk norflash rand test\r\n", disk_norflash_rand_test);
CLI_CMD("disk_norflash_stress_test", "\r\ndisk_norflash_stress_test\r\n disk norflash stress test\r\n", disk_norflash_stress_test);
#endif
CLI_CMD("disk_norflash_xfer", "\r\ndisk_norflash_xfer\r\n disk norflash test\r\n", disk_norflash_xfer);
CLI_CMD("disk_norflash_erase", "\r\ndisk_norflash_erase\r\n disk norflash erase test\r\n", disk_norflash_erase);
CLI_CMD("disk_norflash_write", "\r\ndisk_norflash_write\r\n disk norflash write test\r\n", disk_norflash_write);
CLI_CMD("disk_norflash_read", "\r\ndisk_norflash_read\r\n disk norflash read test\r\n", disk_norflash_read);
