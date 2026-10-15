/**
 * @file sdrv_slimfs_cache.h
 *
 * Copyright (c) 2023 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: slimfs item name and item Id pair cache header file
 *
 * Revision History:
 * -----------------
 */
#ifndef __SDRV_SLIMFS_CACHE_H__
#define __SDRV_SLIMFS_CACHE_H__
/* =========================== include file session ======================== */
#include <stdint.h>
#include <stdbool.h>

#include "sdrv_slimfs_config.h"

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
typedef void* slimfs_cache_hdl;
typedef bool (*matching_fun_t)(const void* matchCtx, void* cacheCtx);

typedef struct {
    uint32_t id;
    char name[CONFIG_SLIMFS_ITEM_NAME_LEN];
}slimfs_itemIdName_pair_t;

typedef struct {
    uint16_t index;
    int16_t pre;
    int16_t next;
    void*   cache_ctx;
    // slimfs_itemIdName_pair_t pair;
}slimfs_cache_node_t;

typedef struct {
    int16_t front;     /**< the oldest cache node */
    int16_t rear;      /**< the newst or the last used cache node*/
    uint16_t cache_ctx_size;
    uint32_t used_num;
    matching_fun_t cb;

    slimfs_cache_node_t cache_node[CONFIG_SLIMFS_CACHE_NUM];
}slimfs_cache_t;

/* =========================== function declare session ==================== */
slimfs_cache_hdl slimfs_cache_init(int32_t ctxSize, matching_fun_t cb);
void slimfs_cache_deinit(slimfs_cache_hdl hdl);
int32_t slimfs_cache_add(slimfs_cache_hdl hdl, const void* matchCtx, void* cacheCtx);
int32_t slimfs_cache_remove(slimfs_cache_hdl hdl, const void* matchCtx);
int32_t slimfs_cache_match(slimfs_cache_hdl hdl, const void* matchCtx, void* cacheCtx);
void slimfs_cache_dump(slimfs_cache_hdl hdl);
/* =========================== global value define session ================= */
/* =========================== static value define session ================= */
/* =========================== function implementation session ============= */

/** @} */
#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __SDRV_SLIMFS_CACHE_H__ */
/* end of header file sdrv_slimfs_cache.h */