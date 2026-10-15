/**
 * @file fee.h
 *
 * Copyright (c) 2022 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#ifndef DISK_FEE_H_
#define DISK_FEE_H_

#include <lib/disk/disk.h>

#define FEE_PAGE_NUMBER_TWO                     2 /* page number, two page schemes are considered to have the highest space utilization */
#define FEE_PAGE_NUMBER                         FEE_PAGE_NUMBER_TWO

#define FEE_CHAR_BIT                            8 /* char indicates the number of bits */

#define FEE_RECORD_BUFF_SIZE                    0x1000

#define DIVD_NUM(div, rem, coe) ((div) * (coe) + (rem)) /* compute the dividend */
#define DIV_NUM(num, coe) ((num) / (coe))               /* calculate the divisor */
#define REM_NUM(num, coe) ((num) % (coe))               /* calculate the remainder */

/* page status type */
typedef uint32_t fee_page_status_t;
/* page status definitions */
#define FEE_PAGE_ERASED                 ((fee_page_status_t)0xFFFFFFFF)     /* PAGE is empty */
#define FEE_PAGE_RECEIVE                ((fee_page_status_t)0xFFFFFFA5)     /* PAGE is marked to receive data */
#define FEE_PAGE_ACTIVE                 ((fee_page_status_t)0xFFFFA5A5)     /* PAGE is marked to store new data */
#define FEE_PAGE_VALID                  ((fee_page_status_t)0xFFA5A5A5)     /* PAGE is full */
#define FEE_PAGE_ERASING                ((fee_page_status_t)0xA5A5A5A5)     /* PAGE is marked to be erase */

/* page mode type */
typedef uint32_t fee_page_mode_t;
#define FEE_PAGE_BLOCK                  ((fee_page_mode_t)0xFFFFA5A5)     /* PAGE mode is block */
#define FEE_PAGE_EEPROM                 ((fee_page_mode_t)0xA5A5A5A5)     /* PAGE mode is eeprom */

#define FEE_BLOCK_ADDR_BIT_SIZE         (0x10000 >> 0x3)

/* page information */
struct fee_page_info {
    fee_page_status_t page_status;
};

typedef uint32_t fee_record_status_t;
typedef uint16_t fee_record_number_t; /* block number range is 64K */
typedef uint16_t fee_record_length_t; /* block size range is 64K */

/* block status definitions */
#define FEE_RECORD_ERASED                ((fee_record_status_t)0xFFFFFFFF)     /* block is empty */
#define FEE_RECORD_INVALID               ((fee_record_status_t)0xFFFFA5A5)     /* block is marked to invalid */
#define FEE_RECORD_VALID                 ((fee_record_status_t)0xA5A5A5A5)     /* block is marked to valid */

/* record information */
struct fee_record_info {
    fee_record_status_t record_status;
    fee_record_number_t record_num;
    fee_record_length_t record_len;
};

/* page information */
struct fee_page {
    disk_addr_t
    page_addr;         /* page start address in the physical layer device, sector_size aligned */
    disk_size_t page_size;         /* page size,sector_size aligned */
    uint16_t    page_sector_num;   /* number of sectors in page */
    uint16_t    page_buff_num;   /* number of sectors in page */
    fee_page_status_t page_status __ALIGNED(CONFIG_ARCH_CACHE_LINE);
};

/* address mask index */
typedef enum {
    FEE_ADDR_MASK_BLOCK = 0,
    FEE_ADDR_MASK_RECORD,
    FEE_ADDR_MASK_RECORD_CURRENT,
    FEE_ADDR_MASK_RECORD_TMP,
    FEE_ADDR_MASK_MAX,
} fee_addr_mask_index;

#define FEE_ADDR_MASK_PRT(addr, addr_index, addr_size) ((addr) + ((addr_index) * (addr_size)))

/* fee device */
struct fee_dev {
    /* disk_dev */
    struct disk_dev disk;           /* disk device */
    disk_addr_t disk_addr;          /* disk start address */
    disk_size_t disk_size;          /* disk size */
    uint16_t disk_mem_align_size;
    uint16_t disk_access_size;
    uint32_t disk_sector_size;

    /* page info */
    uint16_t page_number;           /* PAGE_NUMBER */
    struct fee_page page_info[FEE_PAGE_NUMBER];
    fee_page_mode_t page_mode;

    /* record info */
    uint16_t block_number;                  /* block number range, 1 ~ block_number */
    uint16_t block_length;                  /* single block size range, 1 ~ block_length */

    uint8_t *record_info_buff;              /* used for record information cache, the length is sector_size */
    uint8_t *record_data_buff;              /* used for record data cache, length sector_size */
    uint8_t *record_data_offset_buff;       /* used for calculate record data offset, length sector_size */
    uint16_t record_info_num;               /* number of record info in record_info_buff */
    uint16_t record_buff_size;              /* record buff size */

    /* fee dev information at run time */
    uint16_t current_page;                  /* current page page */
    disk_size_t record_info_offset;         /* current record_info index offset */
    disk_size_t record_data_offset;         /* current record_data addr offset*/

    uint8_t *block_addr_mask;               /* used to store address mask information */
    uint16_t block_addr_size;               /* number of block addr mask size,block_addr_mask size */
};

/* fee APIs */
int fee_init(struct fee_dev *fee_dev);
int fee_exit(struct fee_dev *fee_dev);

/* fee multiple block operations, length block_length aligned */
int fee_write_record_multiple(struct fee_dev *fee_dev, uint16_t block_number,
                              uint8_t *data_buffer, uint16_t length);
int fee_read_record_multiple(struct fee_dev *fee_dev, uint16_t block_number,
                             uint8_t *data_buffer, uint16_t length);
/* fee single block operations,length <= block_length */
int fee_write_record_single(struct fee_dev *fee_dev, uint16_t block_number,
                            uint8_t *data_buffer, uint16_t length);
int fee_read_record_single(struct fee_dev *fee_dev, uint16_t block_number,
                           uint16_t block_offset, uint8_t *data_buffer, uint16_t length);

#endif /* DISK_MMC_H_ */

