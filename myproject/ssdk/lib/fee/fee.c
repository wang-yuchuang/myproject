/**
 * @file fee.c
 *
 * Copyright (c) 2022 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */


#include <debug.h>
#include <stdlib.h>
#include <string.h>
#include <param.h>
#include <types.h>
#include <cmsis_os2.h>
#include <arch/atomic.h>
#include <stdarg.h>

#include <lib/list.h>
#include <lib/fee/fee.h>

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

static int fee_disk_read(struct fee_dev *fee_dev, disk_addr_t addr,
                         uint8_t *dst, disk_size_t size)
{
    if (!IS_ALIGNED(dst, fee_dev->disk_mem_align_size)) {
        ssdk_printf(SSDK_CRIT,
                    "fee_disk_read dst not aligned to disk_mem_align_size\n");
        return -1;
    }

    if ((!IS_ALIGNED(addr, fee_dev->disk_access_size))
            || (!IS_ALIGNED(size, fee_dev->disk_access_size))) {
        ssdk_printf(SSDK_CRIT,
                    "fee_disk_read addr or size not aligned to disk_access_size\n");
        return -1;
    }

    struct disk_dev *disk_dev = &fee_dev->disk;

    block_t block = addr / fee_dev->disk_access_size;

    block_count_t count = size / fee_dev->disk_access_size;

    return disk_read_block(disk_dev, block, dst, count);
}

static int fee_disk_write(struct fee_dev *fee_dev, disk_addr_t addr,
                          const uint8_t *src, disk_size_t size)
{
    if (!IS_ALIGNED(src, fee_dev->disk_mem_align_size)) {
        ssdk_printf(SSDK_CRIT,
                    "fee_disk_write src not aligned to disk_mem_align_size\n");
        return -1;
    }

    if ((!IS_ALIGNED(addr, fee_dev->disk_access_size))
            || (!IS_ALIGNED(size, fee_dev->disk_access_size))) {
        ssdk_printf(SSDK_CRIT,
                    "fee_disk_write addr or size not aligned to disk_access_size\n");
        return -1;
    }

    struct disk_dev *disk_dev = &fee_dev->disk;

    block_t block = addr / fee_dev->disk_access_size;

    block_count_t count = size / fee_dev->disk_access_size;

    return disk_write_block(disk_dev, block, src, count);
}

static int fee_disk_erase(struct fee_dev *fee_dev, disk_addr_t addr,
                          disk_size_t size)
{
    if ((!IS_ALIGNED(addr, fee_dev->disk_sector_size))
            || (!IS_ALIGNED(size, fee_dev->disk_sector_size))) {
        ssdk_printf(SSDK_CRIT,
                    "fee_disk_erase addr or size not aligned to disk_sector_size\n");
        return -1;
    }

    struct disk_dev *disk_dev = &fee_dev->disk;

    uint32_t erase_block = addr / fee_dev->disk_sector_size;

    uint32_t count = size / fee_dev->disk_sector_size;

    return disk_erase_group(disk_dev, erase_block, count, DISK_ERASE_DEFAULT);
}

/* get sector_size record info based on sector_index */
static int fee_get_sector(struct fee_dev *fee_dev, uint16_t page_number,
                          uint16_t sector_index, uint8_t *sector)
{
    return fee_disk_read(fee_dev,
                         fee_dev->page_info[page_number].page_addr + (sector_index *
                                 fee_dev->record_buff_size), sector, fee_dev->record_buff_size);
}

/* configure page_status based on page_number */
static int fee_set_page_status(struct fee_dev *fee_dev, uint16_t page_number,
                               fee_page_status_t page_status)
{
    fee_dev->page_info[page_number].page_status = page_status;
    return fee_disk_write(fee_dev, fee_dev->page_info[page_number].page_addr,
                          (uint8_t *)&fee_dev->page_info[page_number].page_status,
                          sizeof(fee_page_status_t));
}

/* get page_status based on page_number */
static int fee_get_page_status(struct fee_dev *fee_dev, uint16_t page_number,
                               fee_page_status_t *page_status)
{
    if (fee_disk_read(fee_dev, fee_dev->page_info[page_number].page_addr,
                      (uint8_t *)&fee_dev->page_info[page_number].page_status,
                      sizeof(fee_page_status_t)))
        return -1;

    *page_status = fee_dev->page_info[page_number].page_status;
    return 0;
}

/* erases page based on page_number */
static int fee_erase_page(struct fee_dev *fee_dev, uint16_t page_number)
{
    return fee_disk_erase(fee_dev, fee_dev->page_info[page_number].page_addr,
                          fee_dev->page_info[page_number].page_size);
}

/* displays complete page information based on page_number */
static int fee_page_dump(struct fee_dev *fee_dev, uint16_t page_number)
{
    int ret = 0;

    for (int i = 0; i < fee_dev->page_info[page_number].page_buff_num; i++) {
        if (fee_get_sector(fee_dev, page_number, i,
                           (uint8_t *)fee_dev->record_data_buff))
            return -1;

        printf("***************Page:%d sector:%d dump*******************\n",
               page_number, i);
        hexdump8_fee_ex(fee_dev->record_data_buff, fee_dev->record_buff_size,
                        (uint64_t)((addr_t)fee_dev->record_data_buff));
    }

    return ret;
}

/* TODO:set/get block addr mask */
static int fee_block_addr_set(struct fee_dev *fee_dev, uint16_t page_number,
                              uint16_t block_number, uint16_t block_length)
{
    int ret = 0;

    return ret;
}

static int fee_block_addr_get(struct fee_dev *fee_dev, uint16_t page_number,
                              uint16_t block_number, uint32_t *block_addr)
{
    int ret = 0;

    return ret;
}

/* set record_status based on page_number and record_index */
static int fee_set_record_status(struct fee_dev *fee_dev, uint16_t page_number,
                                 disk_size_t record_index_offset, fee_record_status_t record_status)
{
    disk_addr_t addr = fee_dev->page_info[page_number].page_addr +
                       fee_dev->page_info[page_number].page_size;
    addr -= (record_index_offset + 1) * sizeof(struct fee_record_info);
    char IRAM_DATA[sizeof(fee_record_status_t) + CONFIG_ARCH_CACHE_LINE];
    fee_record_status_t *record_status_tmp = (fee_record_status_t *)ROUNDUP((unsigned int)IRAM_DATA, CONFIG_ARCH_CACHE_LINE);
    *record_status_tmp = record_status;
    return fee_disk_write(fee_dev, addr, (uint8_t *)record_status_tmp,
                          sizeof(fee_record_status_t));
}

/* set record_info based on page_number and record_index */
static int fee_set_record_info(struct fee_dev *fee_dev, uint16_t page_number,
                               disk_size_t record_index_offset, struct fee_record_info *record_info)
{
    disk_addr_t addr = fee_dev->page_info[page_number].page_addr +
                       fee_dev->page_info[page_number].page_size;
    addr -= (record_index_offset + 1) * sizeof(struct fee_record_info);
    return fee_disk_write(fee_dev, addr, (uint8_t *)record_info,
                          sizeof(struct fee_record_info));
}

/* get record_data_offset based on page_number and record_index_offset */
static int fee_get_record_data_offset(struct fee_dev *fee_dev,
                                      uint16_t page_number, disk_size_t record_index_offset,
                                      disk_size_t *record_data_offset)
{
    int ret = 0;
    uint32_t record_data_tmp = 0;

    struct fee_record_info *record_info = (struct fee_record_info *)
                                          fee_dev->record_data_offset_buff;
    uint32_t sector_num = DIV_ROUND_UP(record_index_offset,
                                       fee_dev->record_info_num);
    uint32_t sector_index = fee_dev->page_info[page_number].page_buff_num -
                            sector_num;
    uint32_t record_num = 0, record_index = 0;

    for (int i = sector_index; i < sector_index + sector_num; i++) {
        ret = fee_get_sector(fee_dev, page_number, i, (uint8_t *)record_info);

        if (ret) {
            ssdk_printf(SSDK_CRIT, "fee get record info %d failure\n", i);
            return -1;
        }

        if ((!IS_ALIGNED(record_index_offset, fee_dev->record_info_num))
                && (i == sector_index))
            record_num = record_index_offset % fee_dev->record_info_num;
        else
            record_num = fee_dev->record_info_num;

        record_index = fee_dev->record_info_num - record_num;

        for (int j = record_index; j < record_index + record_num; j++) {
            ssdk_printf(SSDK_INFO, "sector_index:%d  record_index:%d \n", i, j);
            record_data_tmp += ROUNDUP(record_info[j].record_len,
                                       fee_dev->disk_access_size);
        }
    }

    *record_data_offset = record_data_tmp;
    return ret;
}

/* write record data based on page_number and record_data_offset */
static int fee_write_record_data(struct fee_dev *fee_dev, uint16_t page_number,
                                 disk_size_t record_data_offset, const uint8_t *data_buffer, uint16_t length)
{
    /* Address offset, start address + page information + block addr + used data offset */
    disk_addr_t addr = fee_dev->page_info[page_number].page_addr +
                       FEE_BLOCK_ADDR_BIT_SIZE + sizeof(struct fee_page_info) + record_data_offset;
    uint16_t rlen = 0;
    uint8_t *buff = NULL;

    while (length) {
        /* length smaller than disk_access_size,added to disk_access_size */
        if (length < fee_dev->disk_access_size) {
            buff = fee_dev->record_data_buff;
            rlen = ROUNDUP(length, fee_dev->disk_access_size);
            memset(buff, 0xff, rlen);
            memcpy(buff, data_buffer, MIN(rlen, length));
        }
        else {
            /* mem address misaligned, data copy */
            if (IS_ALIGNED(data_buffer, fee_dev->disk_mem_align_size)) {
                buff = (uint8_t *)data_buffer;
                rlen = ROUNDDOWN(length, fee_dev->disk_access_size);
            }
            else {
                buff = fee_dev->record_data_buff;
                rlen = MIN(ROUNDDOWN(length, fee_dev->disk_access_size),
                           fee_dev->record_buff_size);
                memcpy(buff, data_buffer, rlen);
            }
        }

        if (fee_disk_write(fee_dev, addr, buff, rlen)) {
            ssdk_printf(SSDK_CRIT,
                        "fee_write_record_data page_number %d record_data_offset %lld failure\n",
                        page_number, record_data_offset);
            return -1;
        }

        data_buffer += MIN(rlen, length);
        addr += MIN(rlen, length);
        length -= MIN(rlen, length);
    }

    return 0;
}

/* read record data based on page_number and record_data_offset */
static int fee_read_record_data(struct fee_dev *fee_dev, uint16_t page_number,
                                disk_size_t record_data_offset, const uint8_t *data_buffer, uint16_t length)
{
    /* Address offset, start address + page information + used data offset */
    disk_addr_t addr = fee_dev->page_info[page_number].page_addr +
                       FEE_BLOCK_ADDR_BIT_SIZE + sizeof(struct fee_page_info) + record_data_offset;
    uint16_t rlen = 0;
    uint8_t *buff = NULL;

    while (length) {
        if (length < fee_dev->disk_access_size) {
            buff = fee_dev->record_data_buff;
            rlen = ROUNDUP(length, fee_dev->disk_access_size);
        }
        else {
            if (IS_ALIGNED(data_buffer, fee_dev->disk_mem_align_size)) {
                buff = (uint8_t *)data_buffer;
                rlen = ROUNDDOWN(length, fee_dev->disk_access_size);
            }
            else {
                buff = fee_dev->record_data_buff;
                rlen = MIN(ROUNDDOWN(length, fee_dev->disk_access_size),
                           fee_dev->record_buff_size);
            }
        }

        if (fee_disk_read(fee_dev, addr, buff, rlen)) {
            ssdk_printf(SSDK_CRIT,
                        "fee_read_record_data page_number %d record_data_offset %lld failure\n",
                        page_number, record_data_offset);
            return -1;
        }

        if (buff != data_buffer) {
            memcpy((void *)data_buffer, buff, MIN(rlen, length));
        }

        data_buffer += MIN(rlen, length);
        addr += MIN(rlen, length);
        length -= MIN(rlen, length);
    }

    return 0;
}

/* Based on page_number, write record */
static int fee_write_record(struct fee_dev *fee_dev, uint16_t page_number,
                            disk_size_t record_index_offset, uint16_t block_number,
                            disk_size_t record_data_offset, const uint8_t *data_buffer, uint16_t length)
{
    int ret = 0;

    ssdk_printf(SSDK_INFO,
                "fee_write_record page_number:%d block_number:%x record_index_offset:%lld record_data_offset:%llx length:%x data_buffer:%x~%x\n",
                page_number, block_number, record_index_offset, record_data_offset, length,
                *data_buffer, *(data_buffer + length - 1));

    /* Configuring record Information */
    char IRAM_DATA[sizeof(struct fee_record_info) + CONFIG_ARCH_CACHE_LINE];
    struct fee_record_info *record_info = (struct fee_record_info *)ROUNDUP((unsigned int)IRAM_DATA, CONFIG_ARCH_CACHE_LINE);
    record_info->record_status = FEE_RECORD_INVALID;
    record_info->record_num = block_number;
    record_info->record_len = length;
    ret = fee_set_record_info(fee_dev, page_number, record_index_offset,
                              record_info);

    if (ret) {
        ssdk_printf(SSDK_CRIT,
                    "fee_block_write set_record_info block_number %d failure\n", block_number);
        return -1;
    }

    /* Write record data */
    ret = fee_write_record_data(fee_dev, page_number, record_data_offset,
                                data_buffer, length);

    if (ret) {
        ssdk_printf(SSDK_CRIT,
                    "fee_block_write set_record_data block_number %d failure\n", block_number);
        return -1;
    }

    /* Configure record valid */
    ret = fee_set_record_status(fee_dev, page_number, record_index_offset,
                                FEE_RECORD_VALID);

    if (ret) {
        ssdk_printf(SSDK_CRIT,
                    "fee_block_write set_record_info FEE_RECORD_VALID status block_number %d failure\n",
                    block_number);
        return -1;
    }

    return ret;
}

/* set the block address mask
    block_number:The starting blocks
    block_length:Block number */
static int fee_addr_mask_set(uint8_t *addr, uint16_t block_number,
                             uint16_t block_length)
{
    int ret = 0;
    uint32_t addr_mask = 0;
    uint32_t addr_index = 0;
    uint32_t left_offset = 0;
    uint32_t rlen = 0;

    ssdk_printf(SSDK_INFO, "fee_addr_mask_set block_number %d block_length %d\n",
                block_number, block_length);

    while (block_length) {
        if (!IS_ALIGNED(block_number, FEE_CHAR_BIT)) {
            left_offset = block_number - ROUNDDOWN(block_number, FEE_CHAR_BIT);
            rlen = MIN(block_length, FEE_CHAR_BIT - left_offset);
            addr_mask = ((0x1 << rlen) - 1) << left_offset;
            addr_index = ROUNDDOWN(block_number, FEE_CHAR_BIT) / FEE_CHAR_BIT;
            addr[addr_index] |= addr_mask;
            ssdk_printf(SSDK_INFO,
                        "fee_addr_mask_set block_number not aligned part addr_index %d addr_mask %x\n",
                        addr_index, addr_mask);
            block_number += rlen;
            block_length -= rlen;
        }
        else if (block_length >= FEE_CHAR_BIT) {
            rlen = ROUNDDOWN(block_length, FEE_CHAR_BIT);
            addr_index = block_number / FEE_CHAR_BIT;
            memset(addr + addr_index, 0xff, rlen / FEE_CHAR_BIT);
            ssdk_printf(SSDK_INFO, "fee_addr_mask_set addr_index %d\n",
                        addr_index);
            block_number += rlen;
            block_length -= rlen;

        }
        else {
            rlen = block_length;
            addr_index = block_number / FEE_CHAR_BIT;
            addr_mask = (0x1 << rlen) - 1;
            addr[addr_index] |= addr_mask;
            ssdk_printf(SSDK_INFO,
                        "fee_addr_mask_set block_length not aligned part addr_index %d addr_mask %x\n",
                        addr_index, addr_mask);
            block_number += rlen;
            block_length -= rlen;
        }
    }

    return ret;
}

/* clear the block address mask
    block_number:The starting blocks
    block_length:Block number */
static int fee_addr_mask_clear(uint8_t *addr, uint16_t block_number,
                               uint16_t block_length)
{
    int ret = 0;
    uint32_t addr_mask = 0;
    uint32_t addr_index = 0;
    uint32_t left_offset = 0;
    uint32_t rlen = 0;

    ssdk_printf(SSDK_INFO, "fee_addr_mask_clear block_number %d block_length %d\n",
                block_number, block_length);

    while (block_length) {
        if (!IS_ALIGNED(block_number, FEE_CHAR_BIT)) {
            left_offset = block_number - ROUNDDOWN(block_number, FEE_CHAR_BIT);
            rlen = MIN(block_length, FEE_CHAR_BIT - left_offset);
            addr_mask = ~(((0x1 << rlen) - 1) << left_offset);
            addr_index = ROUNDDOWN(block_number, FEE_CHAR_BIT) / FEE_CHAR_BIT;
            addr[addr_index] &= addr_mask;
            ssdk_printf(SSDK_INFO,
                        "fee_addr_mask_clear block_number not aligned part addr_index %d addr_mask %x\n",
                        addr_index, addr_mask);
            block_number += rlen;
            block_length -= rlen;
        }
        else if (block_length >= FEE_CHAR_BIT) {
            rlen = ROUNDDOWN(block_length, FEE_CHAR_BIT);
            addr_index = block_number / FEE_CHAR_BIT;
            memset(addr + addr_index, 0x0, rlen / FEE_CHAR_BIT);
            ssdk_printf(SSDK_INFO, "fee_addr_mask_clear addr_index %d\n",
                        addr_index);
            block_number += rlen;
            block_length -= rlen;

        }
        else {
            rlen = block_length;
            addr_index = block_number / FEE_CHAR_BIT;
            addr_mask = ~((0x1 << rlen) - 1);
            addr[addr_index] &= addr_mask;
            ssdk_printf(SSDK_INFO,
                        "fee_addr_mask_clear block_length not aligned part addr_index %d addr_mask %x\n",
                        addr_index, addr_mask);
            block_number += rlen;
            block_length -= rlen;
        }
    }

    return ret;
}

static int fee_addr_mask_and(uint8_t *addr_mask1, uint8_t *addr_mask2,
                             uint8_t *addr_mask3, uint16_t block_length)
{
    for (int i = 0; i < block_length; i++) {
        addr_mask3[i] = addr_mask1[i] & addr_mask2[i];
    }

    return 0;
}

static int fee_addr_mask_or(uint8_t *addr_mask1, uint8_t *addr_mask2,
                            uint8_t *addr_mask3, uint16_t block_length)
{
    for (int i = 0; i < block_length; i++) {
        addr_mask3[i] = addr_mask1[i] | addr_mask2[i];
    }

    return 0;
}

static int fee_addr_mask_empty(uint8_t *addr_mask, uint16_t block_length)
{
    for (int i = 0; i < block_length; i++) {
        if (addr_mask[i])
            return 0;
    }

    return 1;
}

/* get the start block address and size based on the block address mask */
static int fee_addr_mask_block(uint8_t *addr_mask, uint32_t addr_size,
                               uint16_t *block_number, uint16_t *block_length)
{
    int ret = 0;
    uint32_t addr_block_index = 0;
    uint32_t addr_index = 0;
    uint32_t rlen = 0;

    /* The first bit that is 1 is block_number */
    for (int i = 0; i < addr_size; i++) {
        if (addr_mask[i]) {
            addr_block_index = i;
            ssdk_printf(SSDK_INFO, "addr_block_index:%d \n", addr_block_index);

            for (int j = 0; j < FEE_CHAR_BIT; j++) {
                if ((addr_mask[addr_block_index] >> j)  & 0x1) {
                    addr_index = j;
                    ssdk_printf(SSDK_INFO, "addr_index:%d \n", addr_index);
                    *block_number = DIVD_NUM(i, j, FEE_CHAR_BIT);
                    ssdk_printf(SSDK_INFO, "block_number:%d \n", *block_number);
                    goto match_num;
                }
            }
        }
    }

    return -1;

match_num:

    /* Offset back from block_number and calculate the length from the first 0 bit to block_length */
    for (int i = addr_block_index; i < addr_size; i++) {
        if (i != addr_block_index)
            addr_index = 0;

        for (int j = addr_index; j < FEE_CHAR_BIT; j++) {
            if ((addr_mask[i] >> j)  & 0x1) {
                rlen++;
                ssdk_printf(SSDK_INFO, "rlen:%d \n", rlen);
            }
            else
                goto match_len;
        }
    }

match_len:
    *block_length = rlen;
    return ret;
}

/* fee initial power-on initialization */
static int fee_format(struct fee_dev *fee_dev)
{
    /* Erase all pages */
    int ret = 0;

    for (int i = 0; i < fee_dev->page_number; i++) {
        ret = fee_erase_page(fee_dev, i);

        if (ret) {
            ssdk_printf(SSDK_CRIT, "fee_format erase page %d failure\n", i);
            return ret;
        }
    }

    /* configure Page 0 to FEE_PAGE_RECEIVE */
    ret = fee_set_page_status(fee_dev, 0, FEE_PAGE_ACTIVE);

    if (ret) {
        ssdk_printf(SSDK_CRIT, "fee_format set page 0 FEE_PAGE_RECEIVE failure\n");
        return ret;
    }

    return ret;
}

/* get the current record_index based on page_number */
static int fee_get_page_record_index(struct fee_dev *fee_dev,
                                     uint16_t page_number, disk_size_t *record_index_offset)
{
    int ret = 0;
    disk_size_t record_offset = 0;
    uint8_t record_match = 0;

    /* Get the latest record_info_offset(fee_record_info aligned) */
    struct fee_record_info *record_info = (struct fee_record_info *)
                                          fee_dev->record_info_buff;
    uint32_t sector_index = fee_dev->page_info[page_number].page_buff_num - 1;
    uint32_t record_index = 0;

    /* Traversal from the last sector */
    for (int i = sector_index; i >= 0 ; i--) {
        ret = fee_get_sector(fee_dev, page_number, i, (uint8_t *)record_info);

        if (ret) {
            ssdk_printf(SSDK_CRIT, "fee get record info %d failure\n", i);
            return -1;
        }

        /* Traversal from the last record */
        record_index = fee_dev->record_info_num - 1;

        for (int j = record_index; j >= 0; j--) {
            if (record_info[j].record_status == FEE_RECORD_ERASED) {
                ssdk_printf(SSDK_INFO, "sector_num:%d record_num:%d\n", sector_index - i,
                            record_index - j);
                record_match = 1;
                record_offset = DIVD_NUM(sector_index - i, record_index - j,
                                         fee_dev->record_info_num);
                break;
            }
        }

        if (record_match)
            break;
    }

    *record_index_offset = record_offset;
    return ret;
}

/* Get the current page ADDR mask based on page_number and record_index */
static int fee_set_record_addr_mask(struct fee_dev *fee_dev,
                                    uint16_t page_number, disk_size_t record_index_offset, uint8_t *addr_mask)
{
    int ret = 0;
    /* Traversal begins with the last sector */
    uint32_t sector_num = DIV_ROUND_UP(record_index_offset,
                                       fee_dev->record_info_num);
    uint32_t sector_index = fee_dev->page_info[page_number].page_buff_num -
                            sector_num;
    uint32_t record_num = 0, record_index = 0;
    struct fee_record_info *record_info = (struct fee_record_info *)
                                          fee_dev->record_info_buff;
    uint16_t block_length = 0;

    for (int i = sector_index; i < sector_index + sector_num; i++) {
        ret = fee_get_sector(fee_dev, page_number, i, (uint8_t *)record_info);

        if (ret) {
            ssdk_printf(SSDK_CRIT, "fee get record info %d failure\n", i);
            return -1;
        }

        if ((!IS_ALIGNED(record_index_offset, fee_dev->record_info_num))
                && (i == sector_index))
            record_num = record_index_offset % fee_dev->record_info_num;
        else
            record_num = fee_dev->record_info_num;

        record_index = fee_dev->record_info_num - record_num;

        for (int j = record_index; j < record_index + record_num; j++) {
            block_length = DIV_ROUND_UP(record_info[j].record_len, fee_dev->block_length);
            ssdk_printf(SSDK_INFO, "block_number:%d block_length %d\n",
                        record_info[j].record_num, block_length);
            fee_addr_mask_set(addr_mask, record_info[j].record_num, block_length);
        }
    }

    return ret;
}

static int fee_swap_data(struct fee_dev *fee_dev, uint16_t src_page,
                         uint16_t dst_page, disk_size_t record_info_offset_dst,
                         struct fee_record_info *record_info, disk_size_t record_data_offset_src,
                         disk_size_t record_data_offset_dst)
{
    int ret = 0;

    ret = fee_set_record_info(fee_dev, dst_page, record_info_offset_dst,
                              record_info);

    if (ret) {
        ssdk_printf(SSDK_CRIT,
                    "fee_block_write set_record_info block_number %d failure\n",
                    record_info->record_num);
        return -1;
    }

    fee_record_length_t record_len = 0, rlen = 0, addr_offset = 0;;
    record_len = record_info->record_len;
    disk_addr_t addr_src = fee_dev->page_info[src_page].page_addr +
                           FEE_BLOCK_ADDR_BIT_SIZE + sizeof(struct fee_page_info) + record_data_offset_src;
    disk_addr_t addr_dst = fee_dev->page_info[dst_page].page_addr +
                           FEE_BLOCK_ADDR_BIT_SIZE + sizeof(struct fee_page_info) + record_data_offset_dst;

    while (record_len) {
        rlen = MIN(record_len, fee_dev->record_buff_size);
        memset((void *)fee_dev->record_data_buff, 0xff, ROUNDUP(rlen,
                fee_dev->disk_access_size));
        /* Read the data */
        ret = fee_disk_read(fee_dev, addr_src + addr_offset, fee_dev->record_data_buff,
                            ROUNDUP(rlen, fee_dev->disk_access_size));

        if (ret) {
            ssdk_printf(SSDK_CRIT, "fee fee_read_record_data failure\n");
            return -1;
        }

        /* Write data */
        ret = fee_disk_write(fee_dev, addr_dst + addr_offset, fee_dev->record_data_buff,
                             ROUNDUP(rlen, fee_dev->disk_access_size));

        if (ret) {
            ssdk_printf(SSDK_CRIT, "fee fee_write_record failure\n");
            return -1;
        }

        addr_offset += rlen;
        record_len -= rlen;
    }

    /* Configure record valid */
    ret = fee_set_record_status(fee_dev, dst_page, record_info_offset_dst,
                                FEE_RECORD_VALID);

    if (ret) {
        ssdk_printf(SSDK_CRIT,
                    "fee_block_write set_record_info FEE_RECORD_VALID status block_number %d failure\n",
                    record_info->record_num);
        return -1;
    }

    return ret;
}

/* Two page swaps */
static int fee_page_swap_multiple(struct fee_dev *fee_dev, uint16_t active_page,
                                  uint16_t erase_page)
{
    int ret = 0;

    /* Setting page Status */
    if (fee_set_page_status(fee_dev, active_page, FEE_PAGE_VALID))
        return -1;

    if (fee_set_page_status(fee_dev, erase_page, FEE_PAGE_RECEIVE))
        return -1;

    /* Record information */
    disk_size_t record_info_offset_active = fee_dev->record_info_offset;
    disk_size_t record_info_offset_erase = 0;
    disk_size_t record_data_offset_erase = 0;
    disk_size_t record_info_offset_record = 0;
    disk_size_t record_data_offset_record = 0;

    uint16_t match_block = 0;
    uint16_t match_len = 0;
    disk_size_t record_addr_offset = 0;
    disk_size_t record_addr_len = 0;

    /* block address space */
    uint8_t *block_mask = FEE_ADDR_MASK_PRT(fee_dev->block_addr_mask,
                                            FEE_ADDR_MASK_BLOCK, fee_dev->block_addr_size);
    uint8_t *record_mask = FEE_ADDR_MASK_PRT(fee_dev->block_addr_mask,
                           FEE_ADDR_MASK_RECORD, fee_dev->block_addr_size);
    uint8_t *current_record_mask = FEE_ADDR_MASK_PRT(fee_dev->block_addr_mask,
                                   FEE_ADDR_MASK_RECORD_CURRENT, fee_dev->block_addr_size);
    uint8_t *tmp_record_mask = FEE_ADDR_MASK_PRT(fee_dev->block_addr_mask,
                               FEE_ADDR_MASK_RECORD_TMP, fee_dev->block_addr_size);

    /* copy data information */
    memset(record_mask, 0x0, fee_dev->block_addr_size);
    memcpy(record_mask, block_mask, fee_dev->block_addr_size);

    /* Traversal begins with the last sector */
    struct fee_record_info *record_info = (struct fee_record_info *)
                                          fee_dev->record_info_buff;
    uint32_t sector_num = DIV_ROUND_UP(record_info_offset_active,
                                       fee_dev->record_info_num);
    uint32_t sector_index = fee_dev->page_info[active_page].page_buff_num -
                            sector_num;
    uint32_t record_num = 0, record_index = 0;

    for (int i = sector_index; i < sector_index + sector_num; i++) {
        ret = fee_get_sector(fee_dev, active_page, i, (uint8_t *)record_info);

        if (ret) {
            ssdk_printf(SSDK_CRIT, "fee get record info %d failure\n", i);
            return -1;
        }

        if ((!IS_ALIGNED(record_info_offset_active, fee_dev->record_info_num))
                && (i == sector_index))
            record_num = record_info_offset_active % fee_dev->record_info_num;
        else
            record_num = fee_dev->record_info_num;

        record_index = fee_dev->record_info_num - record_num;

        for (int j = record_index; j < record_index + record_num; j++) {
            /* Record the effective */
            if (record_info[j].record_status == FEE_RECORD_VALID) {
                /* Judge the matching of record and block_mask, and swap the consistent parts */
                memset(current_record_mask, 0x0, fee_dev->block_addr_size);
                fee_addr_mask_set(current_record_mask, record_info[j].record_num,
                                  DIV_ROUND_UP(record_info[j].record_len, fee_dev->block_length));
                memset(tmp_record_mask, 0x0, fee_dev->block_addr_size);
                fee_addr_mask_and(current_record_mask, record_mask, tmp_record_mask,
                                  fee_dev->block_addr_size);

                /* When the matching part is not empty */
                while (!fee_addr_mask_empty(tmp_record_mask, fee_dev->block_addr_size)) {
                    /* Matches block & length */
                    fee_addr_mask_block(tmp_record_mask, fee_dev->block_addr_size, &match_block,
                                        &match_len);
                    /* Clears matching blocks */
                    fee_addr_mask_clear(tmp_record_mask, match_block, match_len);
                    fee_addr_mask_clear(record_mask, match_block, match_len);

                    /* Get record data */
                    record_info_offset_record =  DIVD_NUM(
                                                     fee_dev->page_info[active_page].page_buff_num - 1 - i,
                                                     fee_dev->record_info_num - 1 - j, fee_dev->record_info_num);

                    if (fee_get_record_data_offset(fee_dev, active_page, record_info_offset_record,
                                                   &record_data_offset_record))
                        return -1;

                    /* Calculate the data offset and matching address length of the matching part and record */
                    record_addr_offset = (match_block - record_info[j].record_num) *
                                         fee_dev->block_length;
                    record_addr_len = match_len * fee_dev->block_length;

                    /* Configuring record Information */
                    char IRAM_DATA[sizeof(struct fee_record_info) + CONFIG_ARCH_CACHE_LINE];
                    struct fee_record_info *record_info_erase = (struct fee_record_info *)ROUNDUP((unsigned int)IRAM_DATA, CONFIG_ARCH_CACHE_LINE);
                    record_info_erase->record_status = FEE_RECORD_INVALID;
                    record_info_erase->record_num = match_block;
                    record_info_erase->record_len = record_addr_len;

                    ret = fee_swap_data(fee_dev, active_page, erase_page, record_info_offset_erase,
                                        record_info_erase, record_data_offset_record + record_addr_offset,
                                        record_data_offset_erase);

                    if (ret) {
                        ssdk_printf(SSDK_CRIT, "fee_swap_data block_num %d failure\n",
                                    record_info[j].record_num);
                        return -1;
                    }

                    record_info_offset_erase++;
                    record_data_offset_erase += ROUNDUP(record_addr_len, fee_dev->disk_access_size);
                }
            }

            if (fee_addr_mask_empty(record_mask, fee_dev->block_addr_size))
                break;
        }

        if (fee_addr_mask_empty(record_mask, fee_dev->block_addr_size))
            break;
    }

    /* Updated fee_dev information */
    fee_dev->current_page = erase_page;
    fee_dev->record_info_offset = record_info_offset_erase;
    fee_dev->record_data_offset = record_data_offset_erase;
    memset(block_mask, 0x0, fee_dev->block_addr_size);
    ret = fee_set_record_addr_mask(fee_dev, erase_page, fee_dev->record_info_offset,
                                   block_mask);

    if (ret) {
        ssdk_printf(SSDK_CRIT, "fee set_record_addr_mask error\n");
        return ret;
    }

    /* TODO:configure block mask info */

    /* Setting page Status */
    if (fee_set_page_status(fee_dev, erase_page, FEE_PAGE_ACTIVE))
        return -1;

    if (fee_set_page_status(fee_dev, active_page, FEE_PAGE_ERASING))
        return -1;

    if (fee_erase_page(fee_dev, active_page))
        return -1;

    return ret;
}


/* TODO:Two page swaps singel block */
static int fee_page_swap_single(struct fee_dev *fee_dev, uint16_t active_page,
                                uint16_t erase_page)
{
    int ret = 0;

    return ret;
}

/* Two page swaps */
static int fee_page_swap(struct fee_dev *fee_dev, uint16_t active_page,
                         uint16_t erase_page)
{
    switch (fee_dev->page_mode) {
        case FEE_PAGE_BLOCK:
            return fee_page_swap_single(fee_dev, active_page, erase_page);

        case FEE_PAGE_EEPROM:
            return fee_page_swap_multiple(fee_dev, active_page, erase_page);

        default:
            ssdk_printf(SSDK_CRIT, "fee_page_swap page mode error\n");
            return -1;
    }

    return 0;
}

/* get the current page based on page information */
static int fee_page_set_two(struct fee_dev *fee_dev)
{
    int ret = 0;
    fee_page_status_t page0_status = 0;
    fee_page_status_t page1_status = 0;
    /* Getting page state */
    ret = fee_get_page_status(fee_dev, 0, &page0_status);

    if (ret) {
        ssdk_printf(SSDK_CRIT, "fee get page0 status error\n");
        return ret;
    }

    ret = fee_get_page_status(fee_dev, 1, &page1_status);

    if (ret) {
        ssdk_printf(SSDK_CRIT, "fee get page1 status error\n");
        return ret;
    }

    ssdk_printf(SSDK_INFO, "get current page:  page0_status %x page1_status %x\n",
                page0_status, page1_status);

    switch (page0_status) {
        case FEE_PAGE_ERASED:
            switch (page1_status) {
                case FEE_PAGE_ERASED:
                    fee_dev->current_page = 0;
                    break;

                case FEE_PAGE_ACTIVE:
                case FEE_PAGE_VALID:
                    fee_dev->current_page = 1;
                    break;

                default:
                    goto error;
            }

            break;

        case FEE_PAGE_RECEIVE:
            switch (page1_status) {
                case FEE_PAGE_VALID:
                    fee_dev->current_page = 1;
                    break;

                default:
                    goto error;
            }

            break;

        case FEE_PAGE_ACTIVE:
            switch (page1_status) {
                case FEE_PAGE_ERASED:
                case FEE_PAGE_VALID:
                case FEE_PAGE_ERASING:
                    fee_dev->current_page = 0;
                    break;

                default:
                    goto error;
            }

            break;

        case FEE_PAGE_VALID:
            switch (page1_status) {
                case FEE_PAGE_ERASED:
                case FEE_PAGE_RECEIVE:
                    fee_dev->current_page = 0;
                    break;

                case FEE_PAGE_ACTIVE:
                    fee_dev->current_page = 1;
                    break;

                default:
                    goto error;
            }

            break;

        case FEE_PAGE_ERASING:
            switch (page1_status) {
                case FEE_PAGE_ACTIVE:
                    fee_dev->current_page = 1;
                    break;

                default:
                    goto error;
            }

            break;

        default:
            goto error;
    }

    return ret;
error:
    ssdk_printf(SSDK_CRIT,
                "fee_page_set_two page0_status %x page1_status %x failure\n",
                page0_status, page1_status);
    return -1;
}

/* Page state information detection */
static int fee_page_check_two(struct fee_dev *fee_dev)
{
    int ret = 0;
    fee_page_status_t page0_status = 0;
    fee_page_status_t page1_status = 0;
    /* Getting page state */
    ret = fee_get_page_status(fee_dev, 0, &page0_status);

    if (ret) {
        ssdk_printf(SSDK_CRIT, "fee get page0 status error\n");
        return ret;
    }

    ret = fee_get_page_status(fee_dev, 1, &page1_status);

    if (ret) {
        ssdk_printf(SSDK_CRIT, "fee get page1 status error\n");
        return ret;
    }

    ssdk_printf(SSDK_CRIT, "two page:  page0_status %x page1_status %x\n",
                page0_status, page1_status);

    switch (page0_status) {
        case FEE_PAGE_ERASED:
            switch (page1_status) {
                case FEE_PAGE_ERASED:
                    ssdk_printf(SSDK_INFO,
                                "fee dev is initialized for the first time,fee_format\n");
                    /* Power on for the first time */
                    return fee_format(fee_dev);

                case FEE_PAGE_ACTIVE:
                    ssdk_printf(SSDK_INFO, "fee page information is normal\n");
                    /* Normal boot */
                    break;

                case FEE_PAGE_VALID:
                    /* Page1 data is exchanged to Page0 */
                    ssdk_printf(SSDK_INFO, "fee page1 data is full, exchange data to page0\n");
                    return fee_page_swap(fee_dev, 1, 0);

                default:
                    goto error;
            }

            break;

        case FEE_PAGE_RECEIVE:
            switch (page1_status) {
                case FEE_PAGE_VALID:
                    /* Page1 data is exchanged to Page0 */
                    ssdk_printf(SSDK_INFO,
                                "fee page1 data is full, erase page0 & exchange data to page0\n");

                    if (fee_erase_page(fee_dev, 0))
                        goto error;

                    return fee_page_swap(fee_dev, 1, 0);

                default:
                    goto error;
            }

            break;

        case FEE_PAGE_ACTIVE:
            switch (page1_status) {
                case FEE_PAGE_ERASED:
                    ssdk_printf(SSDK_INFO, "fee page information is normal\n");
                    /* Normal boot */
                    break;

                case FEE_PAGE_VALID:
                    fee_set_page_status(fee_dev, 1, FEE_PAGE_ERASING);

                case FEE_PAGE_ERASING:
                    ssdk_printf(SSDK_INFO, "fee erase page1\n");
                    /* Erase the page1 */
                    return fee_erase_page(fee_dev, 1);

                default:
                    goto error;
            }

            break;

        case FEE_PAGE_VALID:
            switch (page1_status) {
                case FEE_PAGE_ERASED:
                    ssdk_printf(SSDK_INFO, "fee page0 data is full, exchange data to page1\n");
                    /* Page0 Data is exchanged to page1 */
                    return fee_page_swap(fee_dev, 0, 1);

                case FEE_PAGE_RECEIVE:
                    /* Page0 Data is exchanged to page1 */
                    ssdk_printf(SSDK_INFO,
                                "fee page0 data is full, erase page1 & exchange data to page1\n");

                    if (fee_erase_page(fee_dev, 1))
                        goto error;

                    return fee_page_swap(fee_dev, 0, 1);

                case FEE_PAGE_ACTIVE:
                    ssdk_printf(SSDK_INFO, "fee erase page0\n");
                    fee_set_page_status(fee_dev, 0, FEE_PAGE_ERASING);
                    /* Erase the page1 */
                    return fee_erase_page(fee_dev, 0);

                default:
                    goto error;
            }

            break;

        case FEE_PAGE_ERASING:
            switch (page1_status) {
                case FEE_PAGE_ACTIVE:
                    ssdk_printf(SSDK_INFO, "fee erase page0\n");
                    /* Erase the page0 */
                    return fee_erase_page(fee_dev, 0);

                default:
                    goto error;
            }

            break;

        default:
            goto error;
    }

    return ret;
error:
    ssdk_printf(SSDK_CRIT, "page0_status %x page1_status %x failure\n",
                page0_status, page1_status);
    return -1;
}

int fee_init(struct fee_dev *fee_dev)
{
    int ret = 0;
    /* Configure basic page information */
    fee_dev->page_number = FEE_PAGE_NUMBER;

    if (!IS_ALIGNED(fee_dev->disk_addr, fee_dev->disk_sector_size)) {
        ssdk_printf(SSDK_CRIT, "fee disk addr:%lld not aligned to sector_size %d\n",
                    fee_dev->disk_addr, fee_dev->disk_sector_size);
        goto error;
    }

    if (!IS_ALIGNED(fee_dev->disk_size,
                    fee_dev->disk_sector_size * fee_dev->page_number)) {
        ssdk_printf(SSDK_CRIT, "fee size:%lld not aligned to (sector_size * %d):%d\n",
                    fee_dev->disk_size, fee_dev->page_number,
                    fee_dev->disk_sector_size * fee_dev->page_number);
        goto error;
    }

    disk_size_t page_size = fee_dev->disk_size / fee_dev->page_number;
    ssdk_printf(SSDK_INFO,
                "nor flash  Flash EEPROM Emulation addr:%llx size:%llx\n",
                fee_dev->disk_addr, fee_dev->disk_size);

    for (int i = 0; i < fee_dev->page_number; i++) {
        fee_dev->page_info[i].page_addr = fee_dev->disk_addr + (page_size * i);
        fee_dev->page_info[i].page_size = page_size;
        fee_dev->page_info[i].page_sector_num = fee_dev->page_info[i].page_size /
                                                fee_dev->disk_sector_size;
        ssdk_printf(SSDK_CRIT, "page %d addr:%llx size:%llx page_sector_num:%d\n",
                    i, fee_dev->page_info[i].page_addr, fee_dev->page_info[i].page_size,
                    fee_dev->page_info[i].page_sector_num);
    }

    /* set current page */
    switch (fee_dev->page_number) {
        case FEE_PAGE_NUMBER_TWO:
            ret = fee_page_set_two(fee_dev);

            if (ret < 0) {
                ssdk_printf(SSDK_CRIT, "two page mode set current page error \n");
                goto error;
            }

            break;

        default:
            ssdk_printf(SSDK_CRIT, "page mode not support \n");
            ret = -1;
            goto error;
    }

    ssdk_printf(SSDK_CRIT, "set current page %d\n", fee_dev->current_page);

    fee_dev->record_buff_size = MIN(FEE_RECORD_BUFF_SIZE, fee_dev->disk_sector_size);
    for (int i = 0; i < fee_dev->page_number; i++) {
        fee_dev->page_info[i].page_buff_num = fee_dev->page_info[i].page_size /
                                                fee_dev->record_buff_size;
        ssdk_printf(SSDK_CRIT, "page %d record_buff_size:%x page_buff_num:%d\n",
                    i, fee_dev->record_buff_size, fee_dev->page_info[i].page_buff_num);
    }
    /* Apply for the Record Info buffer */
    fee_dev->record_info_buff = osAllocAlign(fee_dev->disk_mem_align_size,
                                fee_dev->record_buff_size);

    if (NULL == fee_dev->record_info_buff) {
        ssdk_printf(SSDK_CRIT, "fee_dev record_info buff malloc error\n");
        goto error;
    }

    /* Apply for the Record Data buffer */
    fee_dev->record_data_buff = osAllocAlign(fee_dev->disk_mem_align_size,
                                fee_dev->record_buff_size);

    if (NULL == fee_dev->record_data_buff) {
        ssdk_printf(SSDK_CRIT, "fee_dev record_data buff malloc error\n");
        goto record_info_error;
    }

    /* Apply for calculate record data offset */
    fee_dev->record_data_offset_buff = osAllocAlign(fee_dev->disk_mem_align_size,
                                       fee_dev->record_buff_size);

    if (NULL == fee_dev->record_data_offset_buff) {
        ssdk_printf(SSDK_CRIT, "fee_dev record_data_offset buff malloc error\n");
        goto record_data_error;
    }

    /* Verify that the Record INFO is aligned to the sector */
    if (!IS_ALIGNED(fee_dev->record_buff_size, sizeof(struct fee_record_info))) {
        ssdk_printf(SSDK_CRIT,
                    "fee record_buff_size %x not aligned to fee_record_info size %d\n",
                    fee_dev->record_buff_size, sizeof(struct fee_record_info));
        goto record_data_offset_error;
    }

    fee_dev->record_info_num = fee_dev->record_buff_size / sizeof(
                                   struct fee_record_info);

    ret = fee_get_page_record_index(fee_dev, fee_dev->current_page,
                                    &fee_dev->record_info_offset);

    if (ret) {
        ssdk_printf(SSDK_CRIT, "fee get_page_record_index error\n");
        goto record_data_offset_error;
    }

    ret = fee_get_record_data_offset(fee_dev, fee_dev->current_page,
                                     fee_dev->record_info_offset, &fee_dev->record_data_offset);

    if (ret) {
        ssdk_printf(SSDK_CRIT, "fee get_record_data_offset error\n");
        goto record_data_offset_error;
    }

    ssdk_printf(SSDK_INFO, "record_info_offset %lld record_data_offset:%lld\n",
                fee_dev->record_info_offset, fee_dev->record_data_offset);

    fee_dev->block_addr_size = DIV_ROUND_UP(fee_dev->block_number, FEE_CHAR_BIT);
    fee_dev->block_addr_mask = osAllocAlign(fee_dev->disk_mem_align_size,
                                            fee_dev->block_addr_size * FEE_ADDR_MASK_MAX);

    if (NULL == fee_dev->block_addr_mask) {
        ssdk_printf(SSDK_CRIT, "fee_dev record_addr_mask buff malloc error\n");
        goto record_data_offset_error;
    }

    memset(fee_dev->block_addr_mask, 0x0,
           fee_dev->block_addr_size * FEE_ADDR_MASK_MAX);
    ret = fee_set_record_addr_mask(fee_dev, fee_dev->current_page,
                                   fee_dev->record_info_offset, fee_dev->block_addr_mask);

    if (ret) {
        ssdk_printf(SSDK_CRIT, "fee set_record_addr_mask error\n");
        goto record_addr_mask_error;
    }

    /* Check status information based on page mode */
    switch (fee_dev->page_number) {
        case FEE_PAGE_NUMBER_TWO:
            ret = fee_page_check_two(fee_dev);

            if (ret < 0) {
                ssdk_printf(SSDK_CRIT, "two page mode check error \n");
                goto record_addr_mask_error;
            }

            break;

        default:
            ssdk_printf(SSDK_CRIT, "page mode not support \n");
            ret = -1;
            goto record_addr_mask_error;
    }

    /* One final page was FEE_PAGE_ACTIVE and the rest were FEE_PAGE_ERASE */
    fee_page_status_t page_status;

    for (int i = 0; i < fee_dev->page_number; i++) {
        if (fee_get_page_status(fee_dev, i, &page_status))
            goto record_addr_mask_error;

        if (page_status == FEE_PAGE_ACTIVE) {
            fee_dev->current_page = i;
            break;
        }
    }

    if (fee_get_page_status(fee_dev, fee_dev->current_page, &page_status))
        goto record_addr_mask_error;

    if (page_status != FEE_PAGE_ACTIVE) {
        ssdk_printf(SSDK_CRIT, "page mode check FEE_PAGE_ACTIVE failure\n");
        goto record_addr_mask_error;
    }

    for (int i = 0; i < fee_dev->page_number; i++) {
        if (fee_get_page_status(fee_dev, i, &page_status))
            goto record_addr_mask_error;

        if ((i != fee_dev->current_page) && page_status != FEE_PAGE_ERASED) {
            ssdk_printf(SSDK_CRIT, "page mode check FEE_PAGE_ERASED failure\n");
            goto record_addr_mask_error;
        }
    }

    return ret;

record_addr_mask_error:
    free(fee_dev->block_addr_mask);
record_data_offset_error:
    free(fee_dev->record_data_offset_buff);
record_data_error:
    free(fee_dev->record_data_buff);
record_info_error:
    free(fee_dev->record_info_buff);
error:
    return -1;
}

int fee_exit(struct fee_dev *fee_dev)
{
    int ret = 0;
    free(fee_dev->block_addr_mask);
    free(fee_dev->record_data_offset_buff);
    free(fee_dev->record_data_buff);
    free(fee_dev->record_info_buff);
    return ret;
}

static int fee_page_check_size(struct fee_dev *fee_dev, uint16_t page_number,
                               uint16_t length)
{
    /* Fee Residual space detection */
    /* One more record info space must be reserved for initial use of the record index offset */
    disk_size_t used_size = sizeof(struct fee_page_info) + FEE_BLOCK_ADDR_BIT_SIZE +
                            fee_dev->record_data_offset + (fee_dev->record_info_offset * sizeof(
                                        struct fee_record_info) + sizeof(struct fee_record_info));

    if ((fee_dev->page_info[page_number].page_size - used_size) < length)
        return 0;
    else
        return 1;
}

/* Based on page_number, write record */
static int fee_write_record_block(struct fee_dev *fee_dev,
                                  uint16_t block_number,
                                  uint8_t *data_buffer, uint16_t length)
{
    int ret = 0;

    if (!fee_page_check_size(fee_dev, fee_dev->current_page,
                             length + sizeof(struct fee_record_info))) {
        ssdk_printf(SSDK_INFO,
                    "******************%d full page size %lld target size %d,swap page****************\n",
                    block_number, fee_dev->page_info[fee_dev->current_page].page_size,
                    length + sizeof(struct fee_record_info));
        uint16_t swap_page = 0;

        if (fee_dev->current_page == fee_dev->page_number - 1)
            swap_page = 0;
        else
            swap_page = fee_dev->current_page + 1;

        ret = fee_page_swap(fee_dev, fee_dev->current_page, swap_page);

        if (ret) {
            ssdk_printf(SSDK_CRIT, "fee fee_page_swap error\n");
            return -1;
        }

        if (!fee_page_check_size(fee_dev, fee_dev->current_page,
                                 length + sizeof(struct fee_record_info))) {
            ssdk_printf(SSDK_EMERG, "swap page end, still not enough space\n");
            return -1;
        }

    }

    /* Actually writing record */
    ret = fee_write_record(fee_dev, fee_dev->current_page,
                           fee_dev->record_info_offset, block_number, fee_dev->record_data_offset,
                           data_buffer, length);

    if (ret) {
        ssdk_printf(SSDK_CRIT, "fee fee_write_record error\n");
        return -1;
    }

    /* Update Status Information */
    fee_dev->record_info_offset++;
    fee_dev->record_data_offset += ROUNDUP(length, fee_dev->disk_access_size);
    fee_addr_mask_set(fee_dev->block_addr_mask, block_number, ROUNDUP(length,
                      fee_dev->block_length));

    /* TODO:set block addr mask */

    return ret;
}

/* Based on page_number, write record */
int fee_write_record_multiple(struct fee_dev *fee_dev, uint16_t block_number,
                              uint8_t *data_buffer, uint16_t length)
{
    return fee_write_record_block(fee_dev, block_number, data_buffer, length);
}

/* Read the record based on page_number */
int fee_read_record_multiple(struct fee_dev *fee_dev, uint16_t block_number,
                             uint8_t *data_buffer, uint16_t length)
{
    int ret = 0;

    /* block address space */
    uint8_t *block_mask = FEE_ADDR_MASK_PRT(fee_dev->block_addr_mask,
                                            FEE_ADDR_MASK_BLOCK, fee_dev->block_addr_size);
    uint8_t *record_mask = FEE_ADDR_MASK_PRT(fee_dev->block_addr_mask,
                           FEE_ADDR_MASK_RECORD, fee_dev->block_addr_size);
    uint8_t *current_record_mask = FEE_ADDR_MASK_PRT(fee_dev->block_addr_mask,
                                   FEE_ADDR_MASK_RECORD_CURRENT, fee_dev->block_addr_size);
    uint8_t *tmp_record_mask = FEE_ADDR_MASK_PRT(fee_dev->block_addr_mask,
                               FEE_ADDR_MASK_RECORD_TMP, fee_dev->block_addr_size);

    /* record info offset */
    disk_size_t record_info_offset_record = 0;
    disk_size_t record_data_offset_record = 0;

    /* match block */
    uint16_t match_block = 0;
    uint16_t match_len = 0;

    /* data information */
    disk_size_t record_addr_offset = 0;
    disk_size_t record_addr_len = 0;
    uint32_t data_addr_offset = 0;

    /* Matches the record part and the block part */
    memset(record_mask, 0x0, fee_dev->block_addr_size);
    fee_addr_mask_set(record_mask, block_number, length / fee_dev->block_length);
    fee_addr_mask_and(block_mask, record_mask, record_mask,
                      fee_dev->block_addr_size);

    /* Traversal begins with the last sector */
    struct fee_record_info *record_info = (struct fee_record_info *)
                                          fee_dev->record_info_buff;
    uint32_t sector_num = DIV_ROUND_UP(fee_dev->record_info_offset,
                                       fee_dev->record_info_num);
    uint32_t sector_index =
        fee_dev->page_info[fee_dev->current_page].page_buff_num - sector_num;
    uint32_t record_num = 0, record_index = 0;

    for (int i = sector_index; i < sector_index + sector_num; i++) {
        ret = fee_get_sector(fee_dev, fee_dev->current_page, i, (uint8_t *)record_info);

        if (ret) {
            ssdk_printf(SSDK_CRIT, "fee get record info %d failure\n", i);
            return -1;
        }

        if ((!IS_ALIGNED(fee_dev->record_info_offset, fee_dev->record_info_num))
                && (i == sector_index))
            record_num = fee_dev->record_info_offset % fee_dev->record_info_num;
        else
            record_num = fee_dev->record_info_num;

        record_index = fee_dev->record_info_num - record_num;

        for (int j = record_index; j < record_index + record_num; j++) {
            if (record_info[j].record_status == FEE_RECORD_VALID) {
                /* block_number and block_length of the current record */
                memset(current_record_mask, 0x0, fee_dev->block_addr_size);
                fee_addr_mask_set(current_record_mask, record_info[j].record_num,
                                  record_info[j].record_len / fee_dev->block_length);
                /* matches the matching parts of current_record_mask and record_mask */
                memset(tmp_record_mask, 0x0, fee_dev->block_addr_size);
                fee_addr_mask_and(current_record_mask, record_mask, tmp_record_mask,
                                  fee_dev->block_addr_size);

                while (!fee_addr_mask_empty(tmp_record_mask, fee_dev->block_addr_size)) {
                    /* Matches block & length */
                    fee_addr_mask_block(tmp_record_mask, fee_dev->block_addr_size, &match_block,
                                        &match_len);
                    /* Clears matching blocks */
                    fee_addr_mask_clear(tmp_record_mask, match_block, match_len);
                    fee_addr_mask_clear(record_mask, match_block, match_len);

                    /* Get record data information */
                    record_info_offset_record =  DIVD_NUM(
                                                     fee_dev->page_info[fee_dev->current_page].page_buff_num - 1 - i,
                                                     fee_dev->record_info_num - 1 - j, fee_dev->record_info_num);

                    if (fee_get_record_data_offset(fee_dev, fee_dev->current_page,
                                                   record_info_offset_record, &record_data_offset_record))
                        return -1;

                    /* Calculate the data offset and matching address length of the matching part and record */
                    record_addr_offset = (match_block - record_info[j].record_num) *
                                         fee_dev->block_length;
                    record_addr_len = match_len * fee_dev->block_length;
                    data_addr_offset = (match_block - block_number) * fee_dev->block_length;
                    /* Read the data */
                    ret = fee_read_record_data(fee_dev, fee_dev->current_page,
                                               record_data_offset_record + record_addr_offset, data_buffer + data_addr_offset,
                                               record_addr_len);

                    if (ret) {
                        ssdk_printf(SSDK_CRIT, "fee fee_read_record_data failure\n");
                        return -1;
                    }
                }

                if (fee_addr_mask_empty(record_mask, fee_dev->block_addr_size))
                    break;
            }

            if (fee_addr_mask_empty(record_mask, fee_dev->block_addr_size))
                break;
        }
    }

    return 0;
}

/* TODO:write/read singel block */
int fee_write_record_single(struct fee_dev *fee_dev, uint16_t block_number,
                            uint8_t *data_buffer, uint16_t length)
{
    int ret = 0;
    return ret;
}

int fee_read_record_single(struct fee_dev *fee_dev, uint16_t block_number,
                           uint16_t block_offset, uint8_t *data_buffer, uint16_t length)
{
    int ret = 0;
    return ret;
}
