/**
 * @file sdrv_slimfs_config.h
 *
 * Copyright (c) 2023 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: config for slimfs
 *
 * Revision History:
 * -----------------
 */
#ifndef __SDRV_SLIMFS_CONFIG_H__
#define __SDRV_SLIMFS_CONFIG_H__
/* =========================== include file session ======================== */
#ifdef __cplusplus
extern "C"{
#endif /* __cplusplus */
/**
* @defgroup XXX XXX
* @brief  XXX service
* @{
*/
/* =========================== marco define session ======================== */
// #define CONFIG_SLIMFS_TEST_WITH_PC
// #define CONFIG_SLIMFS_TEST_CMD_MODE

/** choice for slimfs storage medium */
// #define CONFIG_FLASH_TYPE_POSIX_FILE    0
#define CONFIG_FLASH_TYPE_MMC           1


/** medium factor configure */
#define CONFIG_FLASH_SECTOR_SIZE        512

#ifndef CONFIG_FLASH_SECTOR_START
#define CONFIG_FLASH_SECTOR_START       0           /**< variant */
#endif /* end CONFIG_FLASH_SECTOR_START */

#ifndef CONFIG_FLASH_PARTITION
#define CONFIG_FLASH_PARTITION          "slimfs"      /**< variant */
#endif /* end CONFIG_FLASH_PARTITION */

#define CONFIG_SLIMFS_PARTITION_SIZE    0x2000000   // 32M


/** slimfs itemId & name cache number */
#define CONFIG_SLIMFS_CACHE_NUM         20


/** slimfs header manager config */
#define CONFIG_SLIMFS_MAGIC_SIZE        16
#define CONFIG_SLIMFS_MAJOR_SIZE        4
#define CONFIG_SLIMFS_MANOR_SIZE        4
#define CONFIG_SLIMFS_RESERVED_SIZE     6

#ifndef CONFIG_SLIMFS_MAGIC_NUMBER
#define CONFIG_SLIMFS_MAGIC_NUMBER      "sdrv-fs"  /**< variant */
#endif /* end CONFIG_SLIMFS_MAGIC_NUMBER */

#define CONFIG_SLIMFS_MAJOR_NUMBER      "001"
#define CONFIG_SLIMFS_MANOR_NUMBER      "002"


/** item mgr header config */
#define CONFIG_SLIMFS_ITEM_NAME_LEN     16
#define CONFIG_INVALID_ITEM_ID          0x7FFFFFFF


/** slot number default config  */
#ifndef CONFIG_SLIMFS_SLOT32_NUM
#define CONFIG_SLIMFS_SLOT32_NUM        200
#endif
#ifndef CONFIG_SLIMFS_SLOT64_NUM
#define CONFIG_SLIMFS_SLOT64_NUM        1000
#endif
#ifndef CONFIG_SLIMFS_SLOT128_NUM
#define CONFIG_SLIMFS_SLOT128_NUM       100
#endif
#ifndef CONFIG_SLIMFS_SLOT256_NUM
#define CONFIG_SLIMFS_SLOT256_NUM       100
#endif
#ifndef CONFIG_SLIMFS_SLOT512_NUM
#define CONFIG_SLIMFS_SLOT512_NUM       100
#endif
#ifndef CONFIG_SLIMFS_SLOT1K_NUM
#define CONFIG_SLIMFS_SLOT1K_NUM        10
#endif
#ifndef CONFIG_SLIMFS_SLOT2K_NUM
#define CONFIG_SLIMFS_SLOT2K_NUM        10
#endif
#ifndef CONFIG_SLIMFS_SLOT4K_NUM
#define CONFIG_SLIMFS_SLOT4K_NUM        10
#endif
#ifndef CONFIG_SLIMFS_SLOT8K_NUM
#define CONFIG_SLIMFS_SLOT8K_NUM        10
#endif
#ifndef CONFIG_SLIMFS_SLOT16K_NUM
#define CONFIG_SLIMFS_SLOT16K_NUM       2
#endif
#ifndef CONFIG_SLIMFS_SLOT32K_NUM
#define CONFIG_SLIMFS_SLOT32K_NUM       1
#endif

/** runtime os type */
#define CONFIG_SLIMFS_OS_BAREMETAL      0
#define CONFIG_SLIMFS_OS_LK             1
#define CONFIG_SLIMFS_OS_CMSIS          2

#ifndef CONFIG_SLIMFS_OS_TYPE
#define CONFIG_SLIMFS_OS_TYPE           CONFIG_SLIMFS_OS_BAREMETAL  /**< variant */
#endif /* CONFIG_OS_TYPE */

#define CONFIG_SLIMFS_AUTO_FORMAT       1
#define CONFIG_SLIMFS_RETRY_TIMES       3

/** @} */
#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __SDRV_SLIMFS_CONFIG_H__ */
/* end of header file sdrv_slimfs_config.h */