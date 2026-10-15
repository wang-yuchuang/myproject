/**
 * @file disk.h
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#ifndef DISK_H_
#define DISK_H_

#include <stdlib.h>
#include <string.h>
#include <types.h>
#include <lib/list.h>

#define DISK_ADDR_64BITS 1

struct disk_info;
struct disk_operations;
struct disk_cache;

/**
 * @brief size definition.
 */
#ifdef DISK_ADDR_64BITS
typedef uint64_t disk_addr_t;
#else
typedef uint32_t disk_addr_t;
#endif

#ifdef DISK_ADDR_64BITS
typedef uint64_t disk_size_t;
#else
typedef uint32_t disk_size_t;
#endif

#ifdef DISK_ADDR_64BITS
typedef uint64_t block_t;
#else
typedef uint32_t block_t;
#endif

typedef uint32_t block_count_t;
typedef uint32_t block_size_t;

#define DISK_BIT(n) (0x1 << n)

/**
 * @brief disk device name.
 */
#define DISK_MMC_NAME(n) "mmc"#n
#define DISK_MMC_BOOT_NAME(n,m) "mmc"#n"_boot"#m
#define DISK_MMC_RPMB_NAME(n,m) "mmc"#n"_RPMB"#m
#define DISK_MMC_GPP_NAME(n,m) "mmc"#n"_GPP"#m
#define DISK_NOR_FLASH_NAME(n) "norflash"#n
#define DISK_RAM_NAME(n) "memdisk"#n
#define DISK_USB_NAME(n) "usb"#n
#define DISK_MAPPED_NAME(n) "mapped"#n

/**
 * @brief disk device flags.
 */
#define DISK_FLAGS_CLR 0
#define DISK_FLAGS_SET 1

#define DISK_FLAGS_READ DISK_BIT(0) /* support read operations */
#define DISK_FLAGS_WRITE DISK_BIT(1) /* support write operations */
#define DISK_FLAGS_REWR (DISK_FLAGS_READ) | (DISK_FLAGS_WRITE)
#define DISK_FLAGS_DIRECT_IO DISK_BIT(2)

/**
 * @brief disk error.
 */
typedef enum {
    DISK_ERROR_NO_DEVICE = 1,
    DISK_ERROR_NO_PERMISSION,
    DISK_ERROR_INVALID_PARAMETER,
    DISK_ERROR_OUT_RANGE,
    DISK_ERROR_NO_FUN,
    DISK_ERROR_READ_ACCESS,
    DISK_ERROR_WRITE_ACCESS,
    DISK_ERROR_ERASE_ACCESS,
    DISK_ERROR_NO_ALIGNED,
    DISK_ERROR_BUSY,
} disk_error;

/**
 * @brief disk device status.
 */
typedef enum {
    DISK_STATUS_OK = 1,
    DISK_STATUS_ERR,
} disk_status_info;

/**
 * @brief disk erase flags.
 */
typedef enum {
    DISK_ERASE_DEFAULT = 1, /* erase to the device default value */
    DISK_ERASE_ZERO,        /* erase to 0x0 */
    DISK_ERASE_ONE,         /* erase to 0xff */
} disk_erase_flags;

/**
 * @brief disk device operations.
 */
struct disk_operations {
    /* disk_read operation callback function */
    int (*disk_read)(struct disk_info *info, uint8_t *dst, disk_addr_t addr,
                     disk_size_t size);
    /* disk_write operation callback function */
    int (*disk_write)(struct disk_info *info, const uint8_t *src, disk_addr_t addr,
                      disk_size_t size);

    /* disk_read_block operation callback function,blk_sz is disk_dev->block_size */
    int (*disk_read_block)(struct disk_info *info, uint8_t *dst, block_t block,
                           block_count_t count, block_size_t blk_sz);
    /* disk_write_block operation callback function,blk_sz is disk_dev->block_size */
    int (*disk_write_block)(struct disk_info *info, const uint8_t *src,
                            block_t block, block_count_t count, block_size_t blk_sz);

    /* disk_erase operation callback function */
    int (*disk_erase)(struct disk_info *info, disk_addr_t addr, disk_size_t size,
                      uint32_t flag);
    /* disk_erase_group operation callback function,erase_sz is disk_info->erase_size */
    int (*disk_erase_group)(struct disk_info *info, uint32_t erase_block,
                            uint32_t count, uint32_t erase_sz, uint32_t flag);

    /* disk_status operation callback function */
    int (*disk_status)(struct disk_info *info);
    /* disk_ioctloperation callback function */
    int (*disk_ioctl)(struct disk_info *info, uint32_t cmd, void *buff);
};

/**
 * @brief disk device info.
 */
struct disk_info {
    char *disk_name;                    /* disk device name, registration configuration */
    struct list_node node;              /* list node */
    struct disk_operations *disk_ops;   /* disk device operations instance, registration configuration */
    struct disk_cache *cache;           /* disk device cache instance, registration configuration */

    /* disk attribute */
    uint64_t disk_offset;               /* disk device addr offset, registration configuration, default is 0 */
    uint64_t disk_size;                 /* disk device size, registration configuration */
    uint32_t erase_size;                /* disk device erase size, registration configuration */

    uint32_t mem_align_size;            /* disk device block ops memory alignment size, registration configuration */
    uint32_t block_align_size;          /* disk device block ops block_sz alignment size, registration configuration */

    int use_count;                      /* using the counting */
    void *private_data;                 /* Private data */
};

/**
 * @brief disk dev, apply handle & message.
 */
struct disk_dev {
    struct disk_info *info;             /* disk device info */

    /* disk file attribute */
    uint32_t block_size;                /* disk device block ops block_size */
    uint32_t flags;                     /* disk device flags */
};

/* define API */
#define disk_erase_default(dev, addr, size) disk_erase(dev, addr, size, DISK_ERASE_DEFAULT)
#define disk_erase_group_default(dev, erase_block, count) disk_erase_group(dev, erase_block, count, DISK_ERASE_DEFAULT)

/**
 * @brief disk open
 *
 * @param[in] disk info name
 * @param[in] disk apply handle instance,for details,see disk_dev
 * @return 0 if OK, or a negative error code.
 */
int disk_open(const char *disk_name, struct disk_dev *dev);

/**
 * @brief disk close
 *
 * @param[in] disk apply handle instance,for details,see disk_dev
 * @return 0 if OK, or a negative error code.
 */
int disk_close(struct disk_dev *dev);

/**
 * @brief disk read
 *
 * @param[in] disk apply handle instance,for details,see disk_dev
 * @param[in] info addr
 * @param[in] dst memory
 * @param[in] info size
 * @return 0 if OK, or a negative error code.
 */
int disk_read(struct disk_dev *dev, disk_addr_t addr, uint8_t *dst,
              disk_size_t size);

/**
 * @brief disk write
 *
 * @param[in] disk apply handle instance,for details,see disk_dev
 * @param[in] info addr
 * @param[in] src memory
 * @param[in] info size
 * @return 0 if OK, or a negative error code.
 */
int disk_write(struct disk_dev *dev, disk_addr_t addr, const uint8_t *src,
               disk_size_t size);

/**
 * @brief disk read block
 *
 * @param[in] disk apply handle instance,for details,see disk_dev
 * @param[in] info block addr
 * @param[in] dst memory,must be mem_align_size alignment
 * @param[in] info block count
 * @return 0 if OK, or a negative error code.
 */
int disk_read_block(struct disk_dev *dev, block_t block, uint8_t *dst,
                    block_count_t count);

/**
 * @brief disk write block
 *
 * @param[in] disk apply handle instance,for details,see disk_dev
 * @param[in] info block addr
 * @param[in] src memory,must be mem_align_size alignment
 * @param[in] info block count
 * @return 0 if OK, or a negative error code.
 */
int disk_write_block(struct disk_dev *dev, block_t block, const uint8_t *src,
                     block_count_t count);

/**
 * @brief disk erase
 *
 * @param[in] disk apply handle instance,for details,see disk_dev
 * @param[in] info addr
 * @param[in] info size
 * @param[in] erase flag,DISK_ERASE_XXX
 * @return 0 if OK, or a negative error code.
 */
int disk_erase(struct disk_dev *dev, disk_addr_t addr, disk_size_t size,
               disk_erase_flags flag);

/**
 * @brief disk erase group
 *
 * @param[in] disk apply handle instance,for details,see disk_dev
 * @param[in] info erase group addr
 * @param[in] info erase group count
 * @param[in] erase flag,DISK_ERASE_XXX
 * @return 0 if OK, or a negative error code.
 */
int disk_erase_group(struct disk_dev *dev, uint32_t erase_block, uint32_t count,
                     disk_erase_flags flag);

/**
 * @brief disk sync, synchronize all cache data .
 *
 * @param[in] disk apply handle instance,for details,see disk_dev
 * @return 0 if OK, or a negative error code.
 */
int disk_sync(struct disk_dev *dev);

/**
 * @brief disk sync addr, synchronize cache data based on the address.
 *
 * @param[in] disk apply handle instance,for details,see disk_dev
 * @param[in] info addr
 * @param[in] info size
 * @return 0 if OK, or a negative error code.
 */
int disk_sync_addr(struct disk_dev *dev, disk_addr_t addr, disk_size_t size);

/**
 * @brief disk get disk size.
 *
 * @param[in] disk apply handle instance,for details,see disk_dev
 * @return disk_addr.
 */
disk_addr_t disk_size(struct disk_dev *dev);

/**
 * @brief disk get erase size.
 *
 * @param[in] disk apply handle instance,for details,see disk_dev
 * @return erase_size.
 */
uint32_t disk_erase_size(struct disk_dev *dev);

/**
 * @brief disk set block size.
 *
 * @param[in] disk apply handle instance,for details,see disk_dev
 * @param[in] blk_sz,must be block_align_size aligned
 * @return 0 if OK, or a negative error code.
 */
int disk_set_block_size(struct disk_dev *dev, uint32_t blk_sz);

/**
 * @brief disk get block size.
 *
 * @param[in] disk apply handle instance,for details,see disk_dev
 * @return blk_sz.
 */
uint32_t disk_get_block_size(struct disk_dev *dev);

/**
 * @brief disk set flags.
 *
 * @param[in] disk apply handle instance,for details,see disk_dev
 * @param[in] operation mode,DISK_FLAGS_CLR & DISK_FLAGS_SET
 * @param[in] disk flags,for details,see DISK_FLAGS_XXX
 * @return 0 if OK, or a negative error code.
 */
int disk_set_flags(struct disk_dev *dev, bool mode, uint32_t flags);

/**
 * @brief disk get flags.
 *
 * @param[in] disk apply handle instance,for details,see disk_dev
 * @return disk flags.
 */
uint32_t disk_get_flags(struct disk_dev *dev);

/**
 * @brief disk status.
 *
 * @param[in] disk apply handle instance,for details,see disk_dev
 * @return disk_status.
 */
int disk_status(struct disk_dev *dev);

/**
 * @brief disk ioctl.
 *
 * @param[in] disk apply handle instance,for details,see disk_dev
 * @param[in] command code,for details, see disk_xxx.h
 * @param[in] pointer to memory,command content
 * @return 0 if OK, or a negative error code.
 */
int disk_ioctl(struct disk_dev *dev, uint32_t cmd, void *buff);

/**
 * @brief disk register.
 *
 * @param[in] disk info instance,for details,see disk_info
 * @return 0 if OK, or a negative error code.
 */
int register_disk(struct disk_info *info);

/**
 * @brief disk unregister.
 *
 * @param[in] disk info instance,for details,see disk_info
 * @return 0 if OK, or a negative error code.
 */
int unregister_disk(struct disk_info *info);

/**
 * @brief disk init.
 *
 * @return 0 if OK, or a negative error code.
 */
int disk_init(void);

/**
 * @brief disk exit.
 *
 * @return 0 if OK, or a negative error code.
 */
int disk_exit(void);

#endif /* DISK_H_ */
