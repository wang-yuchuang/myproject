/**
 * @file sdrv_slimfs_impl.h
 *
 * Copyright (c) 2023 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: slimfs implement header file
 *
 * Revision History:
 * -----------------
 */
#ifndef __SDRV_SLIMFS_IMPL_H__
#define __SDRV_SLIMFS_IMPL_H__
/* =========================== include file session ======================== */
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "sdrv_slimfs_cache.h"
#include "sdrv_slimfs.h"
#include "sdrv_slimfs_log.h"

#ifdef __cplusplus
extern "C"{
#endif /* __cplusplus */
/**
* @defgroup XXX XXX
* @brief  XXX service
* @{
*/
/* =========================== marco define session ======================== */

/* =========================== enumeration define session ================== */
/* =========================== structure define session ==================== */
/** mutex wrapper handle typedef */
typedef void* mutex_wrap_t;

/** item content checksum funtion typedef */
typedef uint16_t (*checksum_fun_t)(const uint8_t* buf, uint32_t len);

/** medium option API */
typedef struct {
    void* (*init)(void* arg);
    int32_t (*read)(void* hdl, uint8_t* rx_buf, uint64_t blk_addr, uint32_t blk_number);
    int32_t (*write)(void* hdl, const uint8_t* tx_buf, uint64_t blk_addr, uint32_t blk_number);
    int32_t (*erase)(void* hdl, uint64_t bkl_addr, uint32_t len);
    void (*deinit)(void* hdl);
}slimfs_medium_ops_t;

/** slimfs header & slot manager section layout structure typedef */
typedef struct {
    slimfs_header_t         header;
    slimfs_slot_mgr_t       slot_mgr[0];
}__attribute__((packed)) slimfs_layout_t;

typedef struct {
    bool                 init_flg;
    void*                rw_hdl;
    slimfs_layout_t*     slimfs_ptr;        /**< slimfs header buffer */

    slimfs_medium_ops_t* medium_ops;

    void*                slot_buf;          /**< memory buffer for rw slot block, buffer size keep same as max slotType */
    uint32_t             start_blk;
    uint32_t             blk_number;
    uint8_t              rw;
    mutex_wrap_t         mutex;

    slimfs_cache_hdl    cache_hdl;          /**< cache handle */
    checksum_fun_t      checksum_fun;       /**< checksum function */
}slimfs_impl_t;

/** item slot structure */
typedef struct {
    slimfs_item_mgr_t item_mgr;
    uint8_t           content[0];
}__attribute__((packed)) slimfs_item_slot_t;

/* =========================== function declare session ==================== */
/** medium operate API funcs  */
slimfs_medium_ops_t* slimfs_get_medium_file_ops(void);
slimfs_medium_ops_t* slimfs_get_medium_mmc_ops(void);
/** checksum func */
checksum_fun_t get_check_func(void);

/** system resource wrapper */
mutex_wrap_t slimfs_mutex_init(void);
int32_t slimfs_mutex_acquire(mutex_wrap_t mutex);
int32_t slimfs_mutex_release(mutex_wrap_t mutex);
void slimfs_mutex_deinit(mutex_wrap_t mutex);

/** internal for test */
int32_t slimfs_destroy_data(int32_t blk, uint32_t offset);
void slimfs_show_cache(void);
void slimfs_show_debug(void);
/* =========================== global value define session ================= */
/* =========================== static value define session ================= */
/* =========================== function implementation session ============= */

/** @} */
#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __SDRV_SLIMFS_IMPL_H__ */
/* end of header file sdrv_slimfs_impl.h */