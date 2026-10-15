/**
 * @file sdrv_slimfs.h
 *
 * Copyright (c) 2023 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: slimfs header file
 *
 * Revision History:
 * -----------------
 */
#ifndef __SDRV_SLIMFS_H__
#define __SDRV_SLIMFS_H__
/* =========================== include file session ======================== */
#include <stdlib.h>
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
#define SLIMFS_INVALID_ITEM_ID     CONFIG_INVALID_ITEM_ID
#define SLIMFS_FORMATED            0
#define SLIMFS_FORMATING           1
/* =========================== enumeration define session ================== */
/** slimfs slot type enum */
typedef enum {
    slot_32  = 1 << 5,   //32
    slot_64  = 1 << 6,   //64
    slot_128 = 1 << 7,   //128
    slot_256 = 1 << 8,   //256
    slot_512 = 1 << 9,   //512
    slot_1K  = 1 << 10,   //1024
    slot_2K  = 1 << 11,   //2048
    slot_4K  = 1 << 12,   //4096
    slot_8K  = 1 << 13,   //8192
    slot_16K  = 1 << 14,  //16384
    slot_32K  = 1 << 15,  //32768

    slot_type_invalid,
    slot_type_max = slot_type_invalid - 1,
}slot_type_enum_t;

/** item search type enum */
typedef enum {
    SLIMFS_SEARCH_BY_ID,
    SLIMFS_SEARCH_BY_NAME,

    SLIMFS_SEARCH_INVALID_TYPE,
}slimfs_search_type_enum_t;

/** slimfs return val */
typedef enum {
    SLIMFS_RET_OK           = 0,
    SLIMFS_RET_INVALID_FS   = -1,       /**< invalid slimfs partition format */
    SLIMFS_RET_INVALID_PARA = -2,       /**< invalid parameter for slimfs API */
    SLIMFS_RET_NOT_FOUND    = -3,       /**< not found item name */
    SLIMFS_RET_NO_SPACE     = -4,       /**< no spare item slot */
    SLIMFS_RET_MEDIUM_ERR   = -5,       /**< slimfs medium error */
    SLIMFS_RET_NO_MEMORY    = -6,       /**< malloc memory failed */
    SLIMFS_RET_DEV_BUSY     = -7,       /**< slimfs device busy */
    SLIMFS_RET_NAME_DUP     = -8,       /**< slimfs item name duplicate */
    SLIMFS_RET_DATA_DAMAGE  = -9,       /**< slimfs data damage */
    SLIMFS_RET_ERR_CACHE    = -10,      /**< slimfs cache error */
    SLIMFS_RET_ERR_OSRES    = -11,      /**< system resource error */
}slimfs_ret_enum_t;
/* =========================== structure define session ==================== */
/** slimfs header layout */
typedef struct {
    uint8_t magic[CONFIG_SLIMFS_MAGIC_SIZE];
    uint8_t major[CONFIG_SLIMFS_MAJOR_SIZE];
    uint8_t manor[CONFIG_SLIMFS_MANOR_SIZE];
    uint8_t slot_num;
    uint8_t format_sta   : 2;
    uint8_t reserved_bit : 6;
    uint8_t reserved[CONFIG_SLIMFS_RESERVED_SIZE];
}__attribute__((packed)) slimfs_header_t;

/** slimfs slot type define */
typedef struct {
    uint32_t start_block;
    uint32_t block_number;
    slot_type_enum_t slot_type;
}__attribute__((packed)) slimfs_slot_mgr_t;

/** slot type <-> n_index */
typedef struct {
    uint8_t n_index;
    uint32_t slot_type;
}slimfs_slot_nIndex_t;

/** slot manager info & slot used info */
typedef struct {
    slimfs_slot_mgr_t slot_mgr;
    uint32_t n_index;
    uint32_t slot_number;        // 当前类型slot的个数
    uint32_t slot_used;          // 当前类型slot的已用个数
}slot_info_t;

/** search structure */
typedef struct {
    slimfs_search_type_enum_t search_type;
    union {
        uint32_t id;
        uint8_t  name[CONFIG_SLIMFS_ITEM_NAME_LEN];
    };
}slimfs_search_t;

/** item manager header structure */
typedef struct {
    uint32_t id     : 31;
    uint32_t crc_en : 1;
    uint8_t  name[CONFIG_SLIMFS_ITEM_NAME_LEN];
    uint32_t len;
    // bool crc_en;
    uint16_t crc;
}__attribute__((packed)) slimfs_item_mgr_t;

/** item info */
typedef struct {
    slimfs_item_mgr_t   item_mgr;
    void*               content;
}slimfs_item_info_t;

#ifdef CONFIG_SLIMFS_INIT_BLOCK_TYPE
#define slimfs_init_main   slimfs_init
#else  /* !CONFIG_SLIMFS_INIT_BLOCK_TYPE */
#define slimfs_init_main   slimfs_init_nonblock
#endif /* end CONFIG_SLIMFS_INIT_BLOCK_TYPE */
/* =========================== function declare session ==================== */
/**
 * @brief slimfs init
 *
 * @return int32_t
 * @retval 0 success
 * @retval <0 failed
 */
int32_t slimfs_init(void);
int32_t slimfs_init_nonblock(void);

/**
 * @brief slimfs deinit
 *
 * @return int32_t
 * @retval 0 success
 * @retval <0 failed
 */
int32_t slimfs_deinit(void);

/**
 * @brief slimfs format data section, do not change slot type layout
 *
 * @return int32_t
 */
int32_t slimfs_format_data(void);

/**
 * @brief slimfs format slot & data section
 *
 * @param slotInfo[IN] format slot & data section input information, only attention
 *        to @slotInfo->slot_mgr.slot_type and @slotInfo->slot_number field. other
 *        field can be ignored in this API.
 * @param size[IN] slotInfo arry size
 * @return int32_t
 * @retval 0 success
 * @retval <0 failed
 */
int32_t slimfs_format_slot(const slot_info_t* slotInfo, int32_t size);

/**
 * @brief function keep same with API @slimfs_format_slot, but format slot with
 *        default parameter
 *
 * @return int32_t
 * @retval 0 sucess
 * @retval <0 failed
 */
int32_t slimfs_format_slot_default(void);

/**
 * @brief query slimfs header info
 *
 * @param header[OUT] slimfs header pointer
 * @return int32_t
 */
int32_t slimfs_header_query(slimfs_header_t* header);

/**
 * @brief query slim slots information
 *
 * @param slotsInfo[OUT] slotinfo arry, query slot info
 * @param slotsNum[OUT] slot type number
 * @return int32_t
 * @retval 0 success
 * @retval <0 failed
 */
int32_t slimfs_slots_query(slot_info_t* slotsInfo, uint32_t* slotsNum);

/**
 * @brief query slim item information
 *
 * @param search[IN] item search info
 * @param item[OUT] item information
 * @return int32_t
 * @retval 0 success
 * @retval <0 failed
 */
int32_t slimfs_item_query(const slimfs_search_t* search, slimfs_item_mgr_t* itemMgr);

/**
 * @brief dump slimfs all item info, content dump max len 32 byte
 *
 * @param NIndexMask[IN] dump item N_index bitmap, control witch type slots can be dumped
 * @param slotIndex[IN] dump item slotindex, control the slot with specified offset in
 *                      the slotType can be dumped
 * @param len[IN] dump item length, if do not use it, must setup as -1, then dump content len
 *                is MIN(32, item len), otherwise, dump content len is keep same as specified len
 */
void slimfs_item_dump(uint32_t NIndexMask, uint32_t slotIndex, int32_t len);

/**
 * @brief invoker can add item with itemId or itemName, if add the item with itemName,
 *        @item->item_mgr.id must be set as @SLIMFS_INVALID_ITEM_ID, and if the function process
 *        success, its out value indicate the effective itemId, if the function process failed, invoker
 *        should ignore the @item->item_mgr.id out value. otherwise, item filed must be set as an
 *        effective itemId.
 *
 * @param item[IN] add item info and content
 * @return int32_t
 * @retval 0 success
 * @retval <0 failed
 */
int32_t slimfs_item_add(slimfs_item_info_t* item);

/**
 * @brief delete item from slimfs
 *
 * @param search[IN] search type and search info
 * @return int32_t
 * @retval 0 success
 * @retval <0 failed
 */
int32_t slimfs_item_del(const slimfs_search_t* search);

/**
 * @brief update item content to slimfs
 *
 * @param search[IN] search type and search info
 * @param content[IN] content buffer
 * @param len[IN] content length
 * @return int32_t
 * @retval 0 success
 * @retval <0 failed
 */
int32_t slimfs_item_update(const slimfs_search_t* search, const void* content, uint32_t len);

/**
 * @brief read item content from slimfs
 *
 * @param search[IN] search type and search info
 * @param content[IN] read content buffer
 * @param len[IN] content length
 * @return int32_t
 * @retval 0 success
 * @retval <0 failed
 */
int32_t slimfs_item_read(const slimfs_search_t* search, void* content, uint32_t len);

/**
 * @brief control item crc switch on/off status
 *
 * @param search[IN] search item type, search by name or by id
 * @param sta[IN] item crc switch status, true enable crc, false disable crc
 * @return int32_t
 * @retval 0 success
 * @retval <0 failed
 */
int32_t slimfs_item_crc_switch(const slimfs_search_t* search, bool sta);

/** @} */
#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __SDRV_SLIMFS_H__ */
/* end of header file sdrv_slimfs.h */