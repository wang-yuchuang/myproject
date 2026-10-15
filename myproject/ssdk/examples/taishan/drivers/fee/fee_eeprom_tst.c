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
#include <lib/fee/fee.h>
#include <lib/fee/fee_eeprom.h>

//#define LOCAL_CHAR_DUMP     1

#define PART_TABLE_SZIE 0x10000
#define PART_EEPROM_SZIE 0x30000

#define VIRTUAL_EEPROM_SIZE (8 * 1024)

/* Fee Address and size */
static disk_addr_t fee_addr = 0;
static disk_size_t fee_size = PART_EEPROM_SZIE;

/* Analog EEPROM memory and cache addresses */
static uint8_t *mem_eeprom = NULL;
static disk_addr_t flash_eeprom_addr = 0;
static disk_size_t flash_eeprom_size = VIRTUAL_EEPROM_SIZE;


/* Read/write cache address */
#define MEM_SIZE                0x200   /* 512 */
#define MEM_SIZE_TST            0x200   /* maximum length of data written each time */
#define MEM_OFFSET_SIZE         (0x20)
static uint8_t fee_buff[MEM_SIZE + MEM_OFFSET_SIZE] __CACHE_ALIGN;

#define TANSFER_SIZE            0x40

/* fee eeprom 0 */
struct fee_eeprom fee_eeprom0 = {
    .fee_con = {
        .disk_name = DISK_NOR_FLASH_NAME(0),
        .size = PART_EEPROM_SZIE,
        .mem_align_size = CONFIG_ARCH_CACHE_LINE,
        .access_align_size = 0x4,
        .pe_cycles = 10,

        .fee_name = FEE_EEPROM_NAME(0),
        .eeprom_size = 8 * 1024,
        .eeprom_align_size = 4,
        .eeprom_pe_cycles = 100,
    },
};

inline static int isprint(int c)
{
    return ((c >= 0x20) && (c < 0x7f));
}

inline static void hexdump8_fee_ex(const void *ptr, size_t len,
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
    hexdump8_fee_ex(ptr, len, (uint64_t)((addr_t)ptr));
}


/* Synchronize MEM information to flash */
static int fee_sync_flash(    uint8_t *mem_addr, uint32_t mem_size,
                              disk_addr_t eeprom_addr)
{
    int ret = 0;
    struct disk_dev flash_dev = {0};
    uint32_t blk_sz = 4;

    disk_open(DISK_NOR_FLASH_NAME(0), &flash_dev);
    uint32_t sector_size = disk_erase_size(&flash_dev);
    disk_erase_group(&flash_dev, ROUNDDOWN(eeprom_addr, sector_size) / sector_size,
                     ROUNDUP(mem_size, sector_size) / sector_size, DISK_ERASE_DEFAULT);

    disk_set_block_size(&flash_dev, blk_sz);
    ret = disk_write_block(&flash_dev, ROUNDDOWN(eeprom_addr, blk_sz) / blk_sz,
                           mem_addr, ROUNDUP(mem_size, blk_sz) / blk_sz);

    if (ret < 0) {
        printf("fee_sync_flash error\n");
        return -1;
    }

    disk_close(&flash_dev);
    return 0;
}

/* Synchronize Flash information to the MEM */
static int fee_sync_mem(    uint8_t *mem_addr, uint32_t mem_size,
                            disk_addr_t eeprom_addr)
{
    int ret = 0;
    struct disk_dev flash_dev = {0};
    uint32_t blk_sz = 4;

    disk_open(DISK_NOR_FLASH_NAME(0), &flash_dev);
    disk_set_block_size(&flash_dev, blk_sz);
    memset(mem_addr, 0, mem_size);
    ret = disk_read_block(&flash_dev, ROUNDDOWN(eeprom_addr, blk_sz) / blk_sz,
                          mem_addr, ROUNDUP(mem_size, blk_sz) / blk_sz);

    if (ret < 0) {
        printf("fee_sync_mem error\n");
        return -1;
    }

    disk_close(&flash_dev);
    return 0;
}

/* Fee Data is written randomly */
static int fee_data_tansfer(uint16_t addr, uint16_t size, uint8_t value)
{
    int ret = 0;
    uint32_t i;
    uint8_t mem_offset;

    /* Generating random mem offset */
    mem_offset = rand();
    mem_offset &= MEM_OFFSET_SIZE - 1;

    volatile uint8_t *p1 = (volatile uint8_t *)fee_buff + mem_offset;
    memset((void *)p1, 0, size);

    /* Generating random data */
    for (i = 0; i < size; i++, p1++) {
        if (value)
            *p1 = value;
        else
            *p1 = rand();

        if (*p1 == 0x0)
            *p1 = 0x1;
    }

    ret = fee_write_eeprom(FEE_EEPROM_NAME(0), addr, fee_buff + mem_offset, size);

    if (ret) {
        printf("fee_write_eeprom error\n");
        return -1;
    }

    memcpy(mem_eeprom + addr, fee_buff + mem_offset, size);
    return ret;
}

static int fee_random_tansfer(uint16_t tansfer_num)
{
    uint16_t addr = 0;
    uint16_t size = 0;
    uint16_t rand_val = 0;

    for (uint16_t i = 0; i < tansfer_num; i++) {
        rand_val = rand();
        addr = rand_val * 4;
        addr &= flash_eeprom_size - 1;

        rand_val = rand();
        size = rand_val * 4;
        size &= MEM_SIZE - 1;
        size = MIN(size, MEM_SIZE);
        size = MIN(size, flash_eeprom_size - addr);
//        printf("%d: addr = 0x%x, size = %x \n",
//               i, addr, size);

        if (fee_data_tansfer(addr, size, 0)) {
            printf("disk test failed, count = %d!\n", i + 1);
            return -1;
        }

    }

    return 0;
}

static int fee_check(uint8_t *mem_addr, uint32_t mem_size)
{
    int ret = 0;
    int buff_num = mem_size / MEM_SIZE;

    for (int i = 0; i < buff_num; i++) {
        memset(fee_buff, 0, MEM_SIZE);
        ret = fee_read_eeprom(FEE_EEPROM_NAME(0), i * MEM_SIZE, fee_buff, MEM_SIZE);

        if (ret) {
            printf("fee_read_eeprom error\n");
            return -1;
        }

        if (memcmp(fee_buff, mem_addr + (i * MEM_SIZE), MEM_SIZE)) {
            printf("*****************fee check error %d********************\n", i);
            printf("*****************fee data********************\n");
            hexdump8(fee_buff, MEM_SIZE);
            printf("*****************mem data********************\n");
            hexdump8(mem_addr + (i * MEM_SIZE), MEM_SIZE);
#ifdef LOCAL_CHAR_DUMP

            for (int j = 0; j < MEM_SIZE; j++) {
                if (fee_buff[j] != mem_addr[i * MEM_SIZE + j])
                    printf("\n disk error: 0x%x - 0x%x : i=%d\n", fee_buff[j],
                           mem_addr[i * MEM_SIZE + j], j);
            }

#endif
            return -1;
        }
    }

    return 0;
}

static int fee_eeprom_init(void)
{
    struct disk_dev flash_dev = {0};

    disk_open(DISK_NOR_FLASH_NAME(0), &flash_dev);
    disk_size_t flash_size = disk_size(&flash_dev);
    uint32_t sector_size = disk_erase_size(&flash_dev);

    fee_size = ROUNDUP(fee_size, sector_size * 2);
    disk_addr_t addr = flash_size - fee_size - PART_TABLE_SZIE;
    fee_addr = ROUNDDOWN(addr, sector_size);

    addr = flash_size - fee_size - ROUNDUP(flash_eeprom_size,
                                           sector_size) - PART_TABLE_SZIE;
    flash_eeprom_addr = ROUNDDOWN(addr, sector_size);

    disk_close(&flash_dev);


    printf("*************** FEE TST DUMP START *****************\n");
    printf("fee_addr:%llx\t  fee_size:%llx\n", fee_addr, fee_size);
    printf("flash_eeprom_addr:%llx\t  flash_eeprom_size:%llx\n", flash_eeprom_addr,
           flash_eeprom_size);
    printf("*************** FEE TST DUMP END *****************\n");
    return 0;
}

static int fee_tst_init(int argc, char *argv[])
{
    int ret = 0;
    struct fee_eeprom *fee_dev = &fee_eeprom0;
    struct disk_dev flash_dev = {0};
    printf("fee_eeprom_prepare\n");

    disk_open(fee_dev->fee_con.disk_name, &flash_dev);
    disk_size_t flash_size = disk_size(&flash_dev);
    uint32_t sector_size = disk_erase_size(&flash_dev);
    disk_close(&flash_dev);
    fee_dev->fee_con.size = ROUNDUP(fee_dev->fee_con.size, sector_size * 2);
    disk_addr_t addr = flash_size - fee_dev->fee_con.size - PART_TABLE_SZIE;
    fee_dev->fee_con.addr = ROUNDDOWN(addr, sector_size);

    ret = fee_register_eeprom(fee_dev);

    if (ret) {
        printf("fee_register_eeprom error\n");
        return 0;
    }

    fee_eeprom_init();

    return 0;
}
CLI_CMD("fee_tst_init",
        "fee_tst_init", fee_tst_init);

static int fee_tst_eeprom(int argc, char *argv[])
{
    if (argc != 2) {
        printf("command format:fee_tst fee_first count\n");
        return 0;
    }

    uint32_t fee_first = strtol(argv[0], NULL, 10);
    uint32_t count = strtol(argv[1], NULL, 10);

    uint8_t *mem_addr = NULL;
    uint32_t mem_size = flash_eeprom_size;
    disk_addr_t eeprom_addr = flash_eeprom_addr;

    uint32_t tansfer_num = 0;

    mem_addr = osAllocAlign(CONFIG_ARCH_CACHE_LINE, mem_size);

    if (NULL == mem_addr) {
        printf("osAllocAlign mem_eeprom error\n");
        return 0;
    }

    mem_eeprom = mem_addr;
    memset(mem_addr, 0, mem_size);

    if (!fee_first) {
        printf("fee not first, check data\n");

        if (fee_sync_mem(mem_addr, mem_size, eeprom_addr))
            return 0;

        if (fee_check(mem_addr, mem_size))
            return 0;
    }

    for (int i = 0; i < count; i++) {

        tansfer_num = rand();
        tansfer_num &= TANSFER_SIZE - 1;
        printf("fee_random_tansfer count is %d tansfer_num:%d\n", i, tansfer_num);

        if (fee_random_tansfer(tansfer_num)) {
            fee_sync_flash(mem_addr, mem_size, eeprom_addr);
            goto mem_error;
        }

        if (fee_check(mem_addr, mem_size))
            goto mem_error;

    }

    if (fee_sync_flash(mem_addr, mem_size, eeprom_addr))
        goto mem_error;

mem_error:
    free(mem_addr);
    return 0;
}
CLI_CMD("fee_tst_eeprom",
        "fee_tst_eeprom fee_first count", fee_tst_eeprom);

static int fee_erase_eeprom(int argc, char *argv[])
{
    int ret = 0;
    struct disk_dev flash_dev = {0};

    fee_eeprom_init();

    disk_open(DISK_NOR_FLASH_NAME(0), &flash_dev);
    uint32_t erase_size = disk_erase_size(&flash_dev);
    ret = disk_erase_group(&flash_dev, fee_addr / erase_size, fee_size / erase_size,
                           DISK_ERASE_DEFAULT);

    if (ret < 0) {
        printf("fee_erase error\n");
        return 0;
    }

    disk_close(&flash_dev);
    return 0;
}
CLI_CMD("fee_erase_eeprom", "fee_erase_eeprom",
        fee_erase_eeprom);

static int fee_tst_eeprom_all(int argc, char *argv[])
{
    if (argc != 2) {
        printf("command format:fee_tst fee_first count\n");
        return 0;
    }

    uint32_t fee_first = strtol(argv[0], NULL, 10);
    uint32_t count = strtol(argv[1], NULL, 10);

    uint8_t *mem_addr = NULL;
    uint32_t mem_size = flash_eeprom_size;
    disk_addr_t eeprom_addr = flash_eeprom_addr;

    uint16_t addr = 0;
    uint16_t size = 0;
    int ret = 0;

    mem_addr = osAllocAlign(CONFIG_ARCH_CACHE_LINE, mem_size);

    if (NULL == mem_addr) {
        printf("osAllocAlign mem_eeprom error\n");
        return 0;
    }

    mem_eeprom = mem_addr;
    memset(mem_addr, 0, mem_size);

    if (!fee_first) {
        printf("fee not first, check data\n");

        if (fee_sync_mem(mem_addr, mem_size, eeprom_addr))
            return 0;

        if (fee_check(mem_addr, mem_size))
            return 0;
    }

    for (int i = 0; i < count; i++) {

        /* Generating random mem offset */
        addr = rand();
        addr *= 4;
        addr &= flash_eeprom_size - 1;

        size = rand();
        size *= 4;
        size = MIN(size, flash_eeprom_size - addr);
        size &= flash_eeprom_size - 1;

        printf("%d: addr = 0x%x, size = %x \n", i, addr, size);

        volatile uint8_t *p1 = (volatile uint8_t *)mem_eeprom + addr;

        /* Generating random data */
        for (int j = 0; j < size; j++, p1++) {
            *p1 = rand();

            if (*p1 == 0x0)
                *p1 = 0x1;
        }

        ret = fee_write_eeprom(FEE_EEPROM_NAME(0), addr, mem_eeprom + addr, size);

        if (ret) {
            printf("fee_write_eeprom error\n");

            if (fee_sync_flash(mem_addr, mem_size, eeprom_addr))
                goto mem_error;

            goto mem_error;
        }


        if (fee_check(mem_addr, mem_size))
            goto mem_error;

    }

    if (fee_sync_flash(mem_addr, mem_size, eeprom_addr))
        goto mem_error;

mem_error:
    free(mem_addr);
    return 0;
}
CLI_CMD("fee_tst_eeprom_all",
        "fee_tst_eeprom_all fee_first count",
        fee_tst_eeprom_all);

static int fee_w8(int argc, char *argv[])
{
    int ret = 0;

    if (argc != 3) {
        printf("command format:fee_w addr size value\n");
        return 0;
    }

    uint32_t addr = strtol(argv[0], NULL, 16);
    uint32_t size = strtol(argv[1], NULL, 16);
    uint8_t value = strtol(argv[2], NULL, 16);

    printf("fee_w8 addr:%x size:%x value:%x\n", addr, size, value);

    uint8_t *write_buff = NULL;
    write_buff = osAllocAlign(CONFIG_ARCH_CACHE_LINE, size);

    if (NULL == write_buff) {
        printf("osAllocAlign fee_w8 mem_eeprom error\n");
        return 0;
    }

    memset(write_buff, value, size);
    ret = fee_write_eeprom(FEE_EEPROM_NAME(0), addr, write_buff, size);

    if (ret) {
        printf("fee_write_eeprom error\n");
        goto mem_error;
    }

mem_error:
    free(write_buff);
    return 0;
}
CLI_CMD("fee_w", "fee_w",
        fee_w8);

static int fee_r8(int argc, char *argv[])
{
    int ret = 0;

    if (argc != 2) {
        printf("command format:fee_r addr size\n");
        return 0;
    }

    uint32_t addr = strtol(argv[0], NULL, 16);
    uint32_t size = strtol(argv[1], NULL, 16);

    printf("fee_r8 addr:%x size:%x \n", addr, size);

    uint8_t *read_buff = NULL;
    read_buff = osAllocAlign(CONFIG_ARCH_CACHE_LINE, size);

    if (NULL == read_buff) {
        printf("osAllocAlign fee_r8 mem_eeprom error\n");
        return 0;
    }

    ret = fee_read_eeprom(FEE_EEPROM_NAME(0), addr, read_buff, size);

    if (ret) {
        printf("fee_read_eeprom error\n");
        goto mem_error;
    }

    hexdump8(read_buff, size);

mem_error:
    free(read_buff);
    return 0;
}
CLI_CMD("fee_r", "fee_r",
        fee_r8);
