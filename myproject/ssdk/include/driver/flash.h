/**
 * @file flash.h
 * @brief  Flash API header.
 *
 * @copyright Copyright (c) 2020  Semidrive Semiconductor.
 *            All rights reserved.
 */

#ifndef FLASH_API_H_
#define FLASH_API_H_

#ifdef __cplusplus
extern "C" {
#endif

enum flash_opt {
    FLASH_OPT_NONE = 0,
    FLASH_OPT_READ,
    FLASH_OPT_WRITE,
    FLASH_OPT_ERASE,
    FLASH_OPT_MAX,
};

typedef uint64_t flash_addr_t;

typedef uint64_t flash_size_t;

enum flash_opt_result {
    FLASH_OPT_COMPLETE = 0,
    FLASH_OPT_FAILED,
    FLASH_OPT_PENDING,
    FLASH_OPT_INCONSISTENT,
    FLASH_OPT_INVALID,
    FLASH_OPT_RESULET_MAX,
};

struct flash_xfer_info {
    enum flash_opt opt_type;
    enum flash_opt_result opt_result;
    flash_addr_t addr;
    uint8_t *buf;
    flash_size_t size;
};

typedef void (*flash_notification)(enum flash_opt type,
                                   enum flash_opt_result result);

#ifdef __cplusplus
}
#endif
#endif
