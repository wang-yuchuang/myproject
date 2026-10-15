/**
 * @file sdrv_slimfs_impl.c
 *
 * Copyright (c) 2023 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: slimfs implement
 *
 * Revision History:
 * -----------------
 */
/* =========================== include file session ======================== */
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

#include "sdrv_slimfs_log.h"
#include "sdrv_slimfs_config.h"
#include "sdrv_slimfs_impl.h"
#include "sdrv_slimfs.h"
#include "common_utility.h"

#if defined(CONFIG_FLASH_TYPE_MMC)
#include <malloc.h>
#ifndef CACHE_LINE
#ifdef CONFIG_ARCH_CACHE_LINE
#define CACHE_LINE CONFIG_ARCH_CACHE_LINE
#endif
#endif
#endif /* end CONFIG_FLASH_TYPE_MMC */

/* =========================== marco define session ======================== */
#define MEDIUM_RW(hdl, rw, buf, blk_addr, blk_num) \
        if (hdl->medium_ops->rw) { \
            hdl->medium_ops->rw(hdl->rw_hdl \
                                       , (void *)buf \
                                       , blk_addr + CONFIG_FLASH_SECTOR_START \
                                       , blk_num); \
        } else { \
            return SLIMFS_RET_MEDIUM_ERR; \
        }

#ifndef ARRY_SIZE
#define ARRY_SIZE(x)    (sizeof(x)/sizeof(x[0]))
#endif /* ARRY_SIZE */

/** convert bytes number to string */
#define CONVERT_BYTES2UNIT(bytes, buf) \
    char buf[64]; \
    memset(buf, 0x00, 64);\
    convert_bytes2unit(bytes, buf);


/** Ensure that the potential cycle logs are printed
 *  only once during the power-on cycle */
#define LOGPRINT_ONCE(format, ...) \
    do {\
        static bool flg = false; \
        if (!flg) {\
            flg = true; \
            SLIMFS_PRINT(SLIMFS_LOG_ERR, format, ##__VA_ARGS__);} \
    }while(0)

/** itemId layout */
/**
*  [31]     [30:24]   [23:0]
*  crc_en | N_index | slot_index
*/
#define ITEMID_CRCEN_STARTBIT      31
#define ITEMID_CRCEN_BITLEN        1
#define ITEMID_NINDEX_STARTBIT     24
#define ITEMID_NINDEX_BITLEN       7
#define ITEMID_SLOTINDEX_STARTBIT  0
#define ITEMID_SLOTINDEX_BITLEN    24

/* =========================== enumeration define session ================== */
typedef enum {
    SLIMFS_TRAVERSE_CONTINUE = SLIMFS_RET_OK + 1,
    SLIMFS_TRAVERSE_BREAK,
} traverse_type_enum_t;
/* =========================== structure define session ==================== */
/** traverse action func typedef and para structure */
typedef struct {
    uint64_t start_blk;     /**< item mgr header pos in block device */
    uint32_t slot_type;     /**< slot type */
    uint32_t N_index;       /**< N_index */
    uint32_t slot_index;    /**< slot index for itemid */
}traverse_info_t;
typedef traverse_type_enum_t (*traverse_action_t)(slimfs_impl_t* hdl
                                                 , slimfs_item_slot_t* itemSlot
                                                 , traverse_info_t* traInfo
                                                 , void* usrCtx);

/** traverse action for find valid slot and check item name */
struct searchId_checkName {
    slimfs_item_mgr_t item;
    bool find_id_flg;
    bool dup_name_flg;
};
/** traverse action for match itemId for item name */
struct match_itemId {
    const char* name;
    uint32_t itemId;
};
/** traverse action for dump item condition */
struct dump_condition {
    uint32_t N_index_bitmap;
    uint32_t slot_index;
    int32_t len;
};

/** cache item name & item id pair */
struct cache_pair {
    uint32_t id;
    uint8_t  name[CONFIG_SLIMFS_ITEM_NAME_LEN];
};

/* =========================== function declare session ==================== */
static void dump_slimfs_mgr_info(slimfs_layout_t* layoutPtr, const char* prefix);
static bool slot_type_match_status(uint32_t len, uint32_t slotType);

/* =========================== global value define session ================= */
/* =========================== static value define session ================= */
/** slimfs working instance and handle */
static slimfs_impl_t s_slimfs_ins;
static slimfs_impl_t* s_slimfs_hdl = &s_slimfs_ins;

/** slimfs format instance and handle */
static slimfs_impl_t s_slimfs_format_ins;
static slimfs_impl_t* s_slimfs_format_hdl = &s_slimfs_format_ins;

/** slimfs format default config */
slot_info_t s_slot_info[] = {
    {
        .slot_mgr.slot_type = slot_32,
        .slot_number = CONFIG_SLIMFS_SLOT32_NUM,
    },
    {
        .slot_mgr.slot_type = slot_64,
        .slot_number = CONFIG_SLIMFS_SLOT64_NUM,
    },
    {
        .slot_mgr.slot_type = slot_128,
        .slot_number = CONFIG_SLIMFS_SLOT128_NUM,
    },
    {
        .slot_mgr.slot_type = slot_256,
        .slot_number = CONFIG_SLIMFS_SLOT256_NUM,
    },
    {
        .slot_mgr.slot_type = slot_512,
        .slot_number = CONFIG_SLIMFS_SLOT512_NUM,
    },
    {
        .slot_mgr.slot_type = slot_1K,
        .slot_number = CONFIG_SLIMFS_SLOT1K_NUM,
    },
    {
        .slot_mgr.slot_type = slot_2K,
        .slot_number = CONFIG_SLIMFS_SLOT2K_NUM,
    },
    {
        .slot_mgr.slot_type = slot_4K,
        .slot_number = CONFIG_SLIMFS_SLOT4K_NUM,
    },
    {
        .slot_mgr.slot_type = slot_8K,
        .slot_number = CONFIG_SLIMFS_SLOT8K_NUM,
    },
    {
        .slot_mgr.slot_type = slot_16K,
        .slot_number = CONFIG_SLIMFS_SLOT16K_NUM,
    },
    {
        .slot_mgr.slot_type = slot_32K,
        .slot_number = CONFIG_SLIMFS_SLOT32K_NUM,
    }
};

/* =========================== function implementation session ============= */
static bool match_cache_pair(const void* matchCtx, void* cacheCtx)
{
    if (cacheCtx == NULL ||
        matchCtx == NULL) {
        return false;
    }
    struct cache_pair* cpair = (struct cache_pair*)cacheCtx;
    char* name = (char *)matchCtx;

    if (strcmp((char *)cpair->name, name) == 0) {
        return true;
    }
    else {
        return false;
    }
}

static int32_t slimfs_header_check(slimfs_header_t* header)
{
    // check magic
    if (memcmp(header->magic, CONFIG_SLIMFS_MAGIC_NUMBER, strlen(CONFIG_SLIMFS_MAGIC_NUMBER)) != 0) {
        SLIMFS_PRINT(SLIMFS_LOG_ERR, "check slimfs header magic failed.");
        slimfs_hexdump(header->magic, CONFIG_SLIMFS_MAGIC_SIZE, 0);
        return -1;
    }

    //@TODO check major & manor

    if (header->format_sta == SLIMFS_FORMATING) {
        SLIMFS_PRINT(SLIMFS_LOG_ERR, "slimfs may be experience poweroff while format slots or data. need format again");
        return -1;
    }

    return 0;
}

static int32_t slimfs_mgr_size_check(slimfs_impl_t* hdl)
{
    uint32_t mgr_size = 0;
    uint32_t mgr_blk_num = 0;

    mgr_size = sizeof(slimfs_slot_mgr_t) * hdl->slimfs_ptr->header.slot_num;
    mgr_size += sizeof(slimfs_header_t);

    mgr_blk_num = (mgr_size / CONFIG_FLASH_SECTOR_SIZE) + 1;

    if (mgr_blk_num > 1) {
        hdl->slimfs_ptr = realloc(hdl->slimfs_ptr, mgr_blk_num * CONFIG_FLASH_SECTOR_SIZE);
        if (hdl->slimfs_ptr == NULL) {
            SLIMFS_PRINT(SLIMFS_LOG_ERR, "realloc memory for %s failed.", __func__);
            return SLIMFS_RET_NO_MEMORY;
        }

        // read remaining slog mgr info
        MEDIUM_RW(hdl, read, (uint8_t *)hdl->slimfs_ptr + CONFIG_FLASH_SECTOR_SIZE, 1, mgr_blk_num - 1);
    }

    return 0;
}

static int32_t medium_rw_with_slotbuf(slimfs_impl_t* slimfsHdl, uint8_t rw, uint64_t startBlk, uint32_t blkNum)
{
    if (startBlk >= slimfsHdl->start_blk &&
        (startBlk + blkNum) <= (slimfsHdl->start_blk + slimfsHdl->blk_number) &&
        rw == 'r') {

        if (startBlk != slimfsHdl->start_blk) {
            uint32_t offset = (startBlk - slimfsHdl->start_blk) * CONFIG_FLASH_SECTOR_SIZE;
            uint8_t* ptr = (uint8_t *)slimfsHdl->slot_buf + offset;
            memcpy(slimfsHdl->slot_buf, ptr, blkNum * CONFIG_FLASH_SECTOR_SIZE);
        }

        slimfsHdl->rw = rw;
        slimfsHdl->start_blk = startBlk;
        slimfsHdl->blk_number = blkNum;
        return 0;
    }

    slimfsHdl->rw = 0;
    slimfsHdl->start_blk = 0;
    slimfsHdl->blk_number = 0;
    switch (rw) {
        case 'r':
            MEDIUM_RW(slimfsHdl, read, slimfsHdl->slot_buf, startBlk, blkNum);
            break;
        case 'w':
            MEDIUM_RW(slimfsHdl, write, slimfsHdl->slot_buf, startBlk, blkNum);
            break;
        default:
            break;
    }
    slimfsHdl->rw = rw;
    slimfsHdl->start_blk = startBlk;
    slimfsHdl->blk_number = blkNum;

    return 0;
}

static int32_t slimfs_traverse_slot_type(slimfs_impl_t* hdl, slimfs_slot_mgr_t* slotMgr, traverse_action_t cb, void* usrCtx)
{
    assert(hdl);
    assert(hdl->slimfs_ptr);
    assert(hdl->slot_buf);
    assert(slotMgr);

    if (!hdl->init_flg) {
        return SLIMFS_RET_INVALID_FS;
    }

    int32_t ret = 0;
    traverse_info_t tra_info;
    uint32_t slot_index = 0;

    // update Nindex and slot type
    tra_info.N_index = slotMgr - hdl->slimfs_ptr->slot_mgr;
    tra_info.slot_type = slotMgr->slot_type;
    if (slotMgr->slot_type < CONFIG_FLASH_SECTOR_SIZE) {
        // slot type is smaller then sector size, one sector, multi slot
        for (uint32_t blk = 0; blk < slotMgr->block_number; blk++) {
            SLIMFS_PRINT(SLIMFS_LOG_DEBUG, "branch 1, blk=%d, slot_mgr_ptr[mgr].block_number=%d\n"
                        , blk
                        , slotMgr->block_number);
            medium_rw_with_slotbuf(hdl, 'r', blk + slotMgr->start_block, 1);

            //@TODO Optimize to reduce the number of writes
            uint8_t* pos = (uint8_t *)hdl->slot_buf;
            uint32_t offset = 0;
            slimfs_item_slot_t* item_slot = NULL;
            while (offset < CONFIG_FLASH_SECTOR_SIZE) {
                item_slot = (slimfs_item_slot_t*)(pos + offset);

                tra_info.slot_index = slot_index++;
                tra_info.start_blk = blk + slotMgr->start_block;
                if (cb) {
                    ret = cb(hdl
                            , item_slot
                            , &tra_info
                            , usrCtx);
                    if (ret == SLIMFS_TRAVERSE_BREAK) {
                        goto exit;
                    }
                    offset += slotMgr->slot_type;
                }
            }
        }
    }
    else {
        // slot type is bigger then sector size, multi sector one slot
        uint8_t blk_step = slotMgr->slot_type / CONFIG_FLASH_SECTOR_SIZE;
        for (uint32_t blk = 0; blk < slotMgr->block_number; blk += blk_step) {
            medium_rw_with_slotbuf(hdl, 'r', blk + slotMgr->start_block, 1);
            slimfs_item_slot_t* item_slot = (slimfs_item_slot_t *)hdl->slot_buf;
            if (cb) {
                tra_info.slot_index = slot_index++;
                tra_info.start_blk = blk + slotMgr->start_block;
                ret = cb(hdl
                        , item_slot
                        , &tra_info
                        , usrCtx);
                if (ret == SLIMFS_TRAVERSE_BREAK) {
                    goto exit;
                }
            }
        }
    }

    return SLIMFS_TRAVERSE_CONTINUE;

exit:
    return SLIMFS_TRAVERSE_BREAK;
}

static int32_t slimfs_traverse(slimfs_impl_t* hdl, traverse_action_t cb, void* usrCtx)
{
    assert(hdl);
    assert(hdl->slimfs_ptr);
    assert(hdl->slot_buf);

    SLIMFS_PRINT(SLIMFS_LOG_DEBUG, "entry %s.", __func__);

    if (!hdl->init_flg) {
        return SLIMFS_RET_INVALID_FS;
    }

    if (SLIMFS_LOG_LEV > SLIMFS_LOG_DEBUG) {
        dump_slimfs_mgr_info(hdl->slimfs_ptr, "slimfs traverse info");
    }

    uint32_t slot_types_num = hdl->slimfs_ptr->header.slot_num;
    slimfs_slot_mgr_t* slot_mgr_ptr = hdl->slimfs_ptr->slot_mgr;
    int32_t ret = 0;

    for (uint32_t mgr = 0; mgr < slot_types_num; mgr++) {
        SLIMFS_PRINT(SLIMFS_LOG_DEBUG, "mgr=%d, slot_types_num=%d, slot_mgr_ptr[mgr].slot_type=%d \n"
                , mgr
                , slot_types_num
                , slot_mgr_ptr[mgr].slot_type);
        ret = slimfs_traverse_slot_type(hdl, &slot_mgr_ptr[mgr], cb, usrCtx);
        if (ret == SLIMFS_TRAVERSE_BREAK) {
            goto exit;
        }
    }

exit:
    return 0;
}

static int32_t check_format_slot_info(const slot_info_t* slotInfo, int32_t size, slot_info_t* outSlotInfo, int32_t* outSize)
{
    if (slotInfo == NULL || outSlotInfo == NULL || outSize == NULL) {
        return SLIMFS_RET_INVALID_PARA;
    }

    const slot_info_t* slot_ptr = NULL;
    uint32_t slot_type = 0;
    uint32_t type_flg = 0;
    *outSize = 0;

    for (int32_t i = 0; i < size; i++) {
        slot_ptr = &slotInfo[i];
        slot_type = slot_ptr->slot_mgr.slot_type;

        if (((slot_type - 1) & slot_type) != 0 ||
            (slot_type < slot_32) ||
            (slot_type > slot_type_max)) {
            SLIMFS_PRINT(SLIMFS_LOG_ERR, "slot type: 0x%x is invalid for %s. \n", slot_type, __func__);
            return SLIMFS_RET_INVALID_PARA;
        }
        else if ((type_flg & slot_type) != 0) {
            // slotInfo is repeat
            SLIMFS_PRINT(SLIMFS_LOG_ERR, "slot type: 0x%x is repeat for %s. \n", slot_type, __func__);
            return SLIMFS_RET_INVALID_PARA;
        }
        else {
            if (slot_ptr->slot_number != 0) {
                type_flg |= slot_type;
                outSlotInfo[*outSize].slot_mgr.slot_type = slot_ptr->slot_mgr.slot_type;
                outSlotInfo[*outSize].slot_number = slot_ptr->slot_number;
                (*outSize)++;
            }
            else {
                // skip empty slot
            }
        }
    }

    return SLIMFS_RET_OK;
}

static int32_t get_medium_ops(slimfs_medium_ops_t** ops)
{
    if (ops == NULL) {
        return SLIMFS_RET_INVALID_PARA;
    }

#if defined(CONFIG_FLASH_TYPE_POSIX_FILE)
    *ops = slimfs_get_medium_file_ops();
#endif /* end CONFIG_FLASH_TYPE_POSIX_FILE */

#if defined(CONFIG_FLASH_TYPE_MMC)
    *ops = slimfs_get_medium_mmc_ops();
#endif /* end CONFIG_FLASH_TYPE_MMC */

    if (*ops == NULL) {
        SLIMFS_PRINT(SLIMFS_LOG_ERR, "get medium option funcs failed.");
        return SLIMFS_RET_MEDIUM_ERR;
    }

    return SLIMFS_RET_OK;
}

static traverse_type_enum_t traverse_action_format_data(slimfs_impl_t* slimfsHdl
                                                        , slimfs_item_slot_t* itemSlot
                                                        , traverse_info_t* traInfo
                                                        , void* usrCtx)
{
    if (slimfsHdl == NULL || itemSlot == NULL) {
        SLIMFS_PRINT(SLIMFS_LOG_ERR, "invalid parameter for %s.\n", __func__);
        return SLIMFS_TRAVERSE_BREAK;
    }
    SLIMFS_PRINT(SLIMFS_LOG_DEBUG, "entry %s.", __func__);

    itemSlot->item_mgr.id = CONFIG_INVALID_ITEM_ID;
    // memcpy(itemSlot->item_mgr.name, 0x00, CONFIG_SLIMFS_ITEM_NAME_LEN);
    strcpy((char *)itemSlot->item_mgr.name, "NONE");
    itemSlot->item_mgr.len = 0;

    // reduse write times for format slot
    if (traInfo->slot_type < CONFIG_FLASH_SECTOR_SIZE &&
        ((traInfo->slot_index + 1) % (CONFIG_FLASH_SECTOR_SIZE / traInfo->slot_type)) != 0) {
        return SLIMFS_TRAVERSE_CONTINUE;
    }

    MEDIUM_RW(slimfsHdl, write, slimfsHdl->slot_buf, traInfo->start_blk, 1);
    return SLIMFS_TRAVERSE_CONTINUE;
}

static traverse_type_enum_t traverse_action_searchId_checkName(slimfs_impl_t* slimfsHdl
                                                            , slimfs_item_slot_t* itemSlot
                                                            , traverse_info_t* traInfo
                                                            , void* usrCtx)
{
    if (usrCtx == NULL || slimfsHdl == NULL ||
        traInfo == NULL || itemSlot == NULL) {
        SLIMFS_PRINT(SLIMFS_LOG_ERR, "invalid parameter for %s.\n", __func__);
        return SLIMFS_TRAVERSE_BREAK;
    }

    struct searchId_checkName* sc_info = (struct searchId_checkName *)usrCtx;

    int32_t item_name_len = strlen((const char *)(sc_info->item.name));
    int32_t slot_name_len = strlen((const char *)(itemSlot->item_mgr.name));
    if (item_name_len == slot_name_len &&
        strncmp((char *)(itemSlot->item_mgr.name), (char *)(sc_info->item.name), slot_name_len) == 0) {
        // item name is duplicate
        sc_info->dup_name_flg = true;
        return SLIMFS_TRAVERSE_BREAK;
    }
    else {
        if (itemSlot->item_mgr.id == CONFIG_INVALID_ITEM_ID &&
            sc_info->find_id_flg == false &&
            slot_type_match_status(sc_info->item.len, traInfo->slot_type)) {
            sc_info->find_id_flg = true;
            uint32_t item_id = 0;
            set_regions(&item_id, ITEMID_NINDEX_STARTBIT, ITEMID_NINDEX_BITLEN, traInfo->N_index);
            set_regions(&item_id, ITEMID_SLOTINDEX_STARTBIT, ITEMID_SLOTINDEX_BITLEN, traInfo->slot_index);
            sc_info->item.id = item_id;
        }
        else {
            // do nothing
        }
    }

    return SLIMFS_TRAVERSE_CONTINUE;
}

static traverse_type_enum_t traverse_action_statistics_used_slot(slimfs_impl_t* slimfsHdl
                                                                , slimfs_item_slot_t* itemSlot
                                                                , traverse_info_t* traInfo
                                                                , void* usrCtx)
{
    if (usrCtx == NULL || slimfsHdl == NULL ||
        traInfo == NULL || itemSlot == NULL) {
        SLIMFS_PRINT(SLIMFS_LOG_ERR, "invalid parameter for %s.\n", __func__);
        return SLIMFS_TRAVERSE_BREAK;
    }
    slot_info_t* slots_info = (slot_info_t *)usrCtx;

    if (itemSlot->item_mgr.id != CONFIG_INVALID_ITEM_ID) {
        slots_info[traInfo->N_index].slot_used++;
    }

    return SLIMFS_TRAVERSE_CONTINUE;
}

static traverse_type_enum_t traverse_action_match_itemId(slimfs_impl_t* slimfsHdl
                                                        , slimfs_item_slot_t* itemSlot
                                                        , traverse_info_t* traInfo
                                                        , void* usrCtx)
{
    if (usrCtx == NULL || slimfsHdl == NULL ||
        traInfo == NULL || itemSlot == NULL) {
        SLIMFS_PRINT(SLIMFS_LOG_ERR, "invalid parameter for %s.\n", __func__);
        return SLIMFS_TRAVERSE_BREAK;
    }

    struct match_itemId* match_info = (struct match_itemId *)usrCtx;
    uint32_t cal_id = (traInfo->N_index << ITEMID_NINDEX_STARTBIT) | traInfo->slot_index;

    if (itemSlot->item_mgr.id != CONFIG_INVALID_ITEM_ID &&
        strlen(match_info->name) == strlen((const char *)(itemSlot->item_mgr.name)) &&
        strncmp(match_info->name
                , (const char *)(itemSlot->item_mgr.name)
                , strlen(match_info->name)) == 0 ) {
        if (cal_id == itemSlot->item_mgr.id) {
            match_info->itemId = itemSlot->item_mgr.id;
        }
        else {
            SLIMFS_PRINT(SLIMFS_LOG_ERR, "slot id field is invalid for item: %s, calculate id: %d, store id: 0x%x"
                        , match_info->name
                        , cal_id
                        , itemSlot->item_mgr.id);
            match_info->itemId = CONFIG_INVALID_ITEM_ID;
        }
        return SLIMFS_TRAVERSE_BREAK;
    }

    return SLIMFS_TRAVERSE_CONTINUE;
}

static traverse_type_enum_t traverse_action_dump_item(slimfs_impl_t* slimfsHdl
                                                     , slimfs_item_slot_t* itemSlot
                                                     , traverse_info_t* traInfo
                                                     , void* usrCtx)
{
    if (slimfsHdl == NULL || usrCtx == NULL ||
        traInfo == NULL || itemSlot == NULL) {
        SLIMFS_PRINT(SLIMFS_LOG_ERR, "invalid parameter for %s.\n", __func__);
        return SLIMFS_TRAVERSE_BREAK;
    }

    struct dump_condition* dump_cond = (struct dump_condition*)usrCtx;

    uint32_t dump_len = 0;

    SLIMFS_PRINT(SLIMFS_LOG_DEBUG, "entry %s, current slot id: 0x%x, dump cond NindexBitMap: 0x%x, dump cond slotIndex: %d, N_index bitMap: 0x%x\n"
                                , __func__
                                , itemSlot->item_mgr.id
                                , dump_cond->N_index_bitmap
                                , dump_cond->slot_index
                                , (1 << traInfo->N_index));

    if (itemSlot->item_mgr.id != CONFIG_INVALID_ITEM_ID) {
        if ((dump_cond->N_index_bitmap & (1 << traInfo->N_index)) &&
            (traInfo->slot_index == dump_cond->slot_index ||
            dump_cond->slot_index == 0xFFFFFF)) {

            dump_item_mgr_info(&itemSlot->item_mgr, NULL);
            if (dump_cond->len == -1) {
                dump_len = MIN(32, itemSlot->item_mgr.len);
            }
            else {
                dump_len = dump_cond->len;
            }
            slimfs_hexdump(itemSlot->content, dump_len, 0);
        }
        else {
            // do nothing
        }
    }

    return SLIMFS_TRAVERSE_CONTINUE;
}

static int32_t search_itemId_by_item_name(slimfs_impl_t* slimfsHdl, const char* itemName, uint32_t* itemId, int32_t len)
{
    if (itemName == NULL || itemId == NULL || strlen(itemName) >= CONFIG_SLIMFS_ITEM_NAME_LEN) {
        SLIMFS_PRINT(SLIMFS_LOG_ERR, "invalid parameter for %s.\n", __func__);
        return SLIMFS_RET_INVALID_PARA;
    }

    struct cache_pair pair;
    int32_t ret = slimfs_cache_match(slimfsHdl->cache_hdl, itemName, &pair);
    if (ret == 0) {
        *itemId = pair.id;
        return 0;
    }

    struct match_itemId match_info;
    match_info.name = itemName;
    match_info.itemId = CONFIG_INVALID_ITEM_ID;

    if (len > 0) {
        // confirm slot type depends item len, only traverse the slottype slots
        for (int32_t nindex = 0; nindex < slimfsHdl->slimfs_ptr->header.slot_num; nindex++) {
            if (slot_type_match_status(len, slimfsHdl->slimfs_ptr->slot_mgr[nindex].slot_type)) {
                slimfs_traverse_slot_type(slimfsHdl
                                         , &slimfsHdl->slimfs_ptr->slot_mgr[nindex]
                                         , traverse_action_match_itemId
                                         , &match_info);
                if (match_info.itemId != CONFIG_INVALID_ITEM_ID &&
                    itemId != NULL) {
                    *itemId = match_info.itemId;

                    // add cache, if invoke in del function, its will be clear latter.
                    memset(&pair, 0x00, sizeof(pair));
                    pair.id = match_info.itemId;
                    strcpy((char *)pair.name, itemName);
                    slimfs_cache_add(slimfsHdl->cache_hdl, (void *)itemName, &pair);

                    return 0;
                }
            }
        }
    }
    else {
        // traverse all slimfs slots
        slimfs_traverse(slimfsHdl, traverse_action_match_itemId, &match_info);
        if (match_info.itemId != CONFIG_INVALID_ITEM_ID &&
            itemId != NULL) {
            *itemId = match_info.itemId;

            // add cache, if invoke in del function, its will be clear latter.
            memset(&pair, 0x00, sizeof(pair));
            pair.id = match_info.itemId;
            strcpy((char *)pair.name, itemName);
            slimfs_cache_add(slimfsHdl->cache_hdl, (void *)itemName, &pair);

            return 0;
        }
    }

    return -1;
}

static void dump_slimfs_mgr_info(slimfs_layout_t* layoutPtr, const char* prefix)
{
    if (layoutPtr == NULL) {
        SLIMFS_PRINT(SLIMFS_LOG_ERR, "parameter error for %s.\n", __func__);
        return ;
    }
    const char * str = ((prefix == NULL) ? ("slimfs summary info") : (prefix));
    SLIMFS_PRINT(SLIMFS_LOG_INFO, "%s: \n", str);
    dump_slimfs_info(&layoutPtr->header, "  --");

    for (int32_t i = 0; i < layoutPtr->header.slot_num; i++) {
        printf("    --slot type: %5d, start block: %5d, block number: %5d \n"
                     , layoutPtr->slot_mgr[i].slot_type
                     , layoutPtr->slot_mgr[i].start_block
                     , layoutPtr->slot_mgr[i].block_number);
    }

    return;
}

static int32_t slimfs_init_impl(slimfs_impl_t* slimfsHdl)
{
    int32_t ret;
    // init system resource
    slimfsHdl->mutex = slimfs_mutex_init();
    if (slimfsHdl->mutex == NULL) {
        return SLIMFS_RET_ERR_OSRES;
    }

    // get medium ops
    if ((ret = get_medium_ops(&slimfsHdl->medium_ops)) != SLIMFS_RET_OK) {
        return ret;
    }

    // init medium
    if (slimfsHdl->medium_ops->init) {
        slimfsHdl->rw_hdl = slimfsHdl->medium_ops->init((void *)CONFIG_FLASH_PARTITION);
        if (slimfsHdl->rw_hdl == NULL) {
            SLIMFS_PRINT(SLIMFS_LOG_ERR, "medium ops init failed for getting rw handle.");
            return SLIMFS_RET_MEDIUM_ERR;
        }
    }
    else {
        return SLIMFS_RET_MEDIUM_ERR;
    }

    // get checksum function
    slimfsHdl->checksum_fun = get_check_func();
#if defined(CONFIG_FLASH_TYPE_MMC)
    slimfsHdl->slimfs_ptr = memalign(CACHE_LINE, CONFIG_FLASH_SECTOR_SIZE);
#else
    slimfsHdl->slimfs_ptr = malloc(CONFIG_FLASH_SECTOR_SIZE);
#endif /* end CONFIG_FLASH_TYPE_MMC */
    if (slimfsHdl->slimfs_ptr == NULL) {
        SLIMFS_PRINT(SLIMFS_LOG_ERR, "malloc memory for slimfs header failed.");
        return SLIMFS_RET_NO_MEMORY;
    }
    memset(slimfsHdl->slimfs_ptr, 0x00, CONFIG_FLASH_SECTOR_SIZE);

    MEDIUM_RW(slimfsHdl, read, slimfsHdl->slimfs_ptr, 0, 1);

    // check slimfs header
    if (slimfs_header_check(&slimfsHdl->slimfs_ptr->header) != 0) {
        return SLIMFS_RET_INVALID_FS;
    }

    // slimfs header & slot mgr size
    if ((ret = slimfs_mgr_size_check(slimfsHdl)) != 0) {
        return ret;
    }

    // malloc slot buffer deps on max slot type
    int32_t slot_type_max = slimfsHdl->slimfs_ptr->slot_mgr[slimfsHdl->slimfs_ptr->header.slot_num - 1].slot_type;
#if defined(CONFIG_FLASH_TYPE_MMC)
    slimfsHdl->slot_buf = memalign(CACHE_LINE, slot_type_max);
#else
    slimfsHdl->slot_buf = malloc(slot_type_max);
#endif /* end CONFIG_FLASH_TYPE_MMC */
    if (slimfsHdl->slot_buf == NULL) {
        SLIMFS_PRINT(SLIMFS_LOG_ERR, "malloc memory for slot buffer failed.");
        return SLIMFS_RET_NO_MEMORY;
    }

    // get slimfs cache handle
    slimfsHdl->cache_hdl = slimfs_cache_init(sizeof(struct cache_pair), match_cache_pair);
    if (slimfsHdl->cache_hdl == NULL) {
        return SLIMFS_RET_ERR_CACHE;
    }

    slimfsHdl->init_flg = true;

    if (slimfsHdl == s_slimfs_format_hdl &&
        slimfsHdl->slimfs_ptr) {
        slimfsHdl->slimfs_ptr->header.format_sta = SLIMFS_FORMATING;
        MEDIUM_RW(slimfsHdl, write, slimfsHdl->slimfs_ptr, 0, 1);
    }

    return SLIMFS_RET_OK;
}

static int32_t slimfs_deinit_impl(slimfs_impl_t* slimfsHdl)
{
    if (slimfsHdl == s_slimfs_format_hdl &&
        slimfsHdl->slimfs_ptr) {
        slimfsHdl->slimfs_ptr->header.format_sta = SLIMFS_FORMATED;
        MEDIUM_RW(slimfsHdl, write, slimfsHdl->slimfs_ptr, 0, 1);
    }

    slimfsHdl->init_flg = false;

    if (slimfsHdl->slimfs_ptr) {
        free(slimfsHdl->slimfs_ptr);
        slimfsHdl->slimfs_ptr = NULL;
    }

    if (slimfsHdl->slot_buf) {
        free(slimfsHdl->slot_buf);
        slimfsHdl->slot_buf = NULL;
    }

    if (slimfsHdl->cache_hdl) {
        slimfs_cache_deinit(slimfsHdl->cache_hdl);
        slimfsHdl->cache_hdl = NULL;
    }

    if (slimfsHdl->medium_ops &&
        slimfsHdl->medium_ops->deinit &&
        slimfsHdl->rw_hdl) {
        slimfsHdl->medium_ops->deinit(slimfsHdl->rw_hdl);
        slimfsHdl->rw_hdl = NULL;
    }

    // destroy system resource
    if (slimfsHdl->mutex) {
        slimfs_mutex_deinit(slimfsHdl->mutex);
        slimfsHdl->mutex = NULL;
    }

    return 0;
}

static int32_t calculate_slot_postion_withId(slimfs_impl_t* slimfsHdl
                                      , uint32_t itemId
                                      , uint64_t* startBlk
                                      , uint32_t* blkOff
                                      , uint32_t* slotType)
{
    if (slimfsHdl == NULL) {
        return SLIMFS_RET_INVALID_FS;
    }

    uint8_t N_index = get_regions(itemId, ITEMID_NINDEX_STARTBIT, ITEMID_NINDEX_BITLEN);
    uint32_t slot_index = get_regions(itemId, ITEMID_SLOTINDEX_STARTBIT, ITEMID_SLOTINDEX_BITLEN);

    if (N_index >= slimfsHdl->slimfs_ptr->header.slot_num) {
        SLIMFS_PRINT(SLIMFS_LOG_ERR, "invalid N_index: 0x%x(0x%x) for %s.\n", N_index, itemId, __func__);
        return SLIMFS_RET_INVALID_PARA;
    }

    uint64_t blks = 0;
    uint32_t blk_offset = 0;
    uint64_t pos = slimfsHdl->slimfs_ptr->slot_mgr[N_index].slot_type * slot_index;
    blks = pos / CONFIG_FLASH_SECTOR_SIZE;
    blk_offset = pos % CONFIG_FLASH_SECTOR_SIZE;

    if (blks >= slimfsHdl->slimfs_ptr->slot_mgr[N_index].block_number) {
        SLIMFS_PRINT(SLIMFS_LOG_ERR, "invalid slot_index: 0x%x(0x%x) for %s.\n", slot_index, itemId, __func__);
        return SLIMFS_RET_INVALID_PARA;
    }

    if (startBlk) {
        *startBlk = blks + slimfsHdl->slimfs_ptr->slot_mgr[N_index].start_block;
    }
    if (blkOff) {
        *blkOff = blk_offset;
    }
    if (slotType) {
        *slotType = slimfsHdl->slimfs_ptr->slot_mgr[N_index].slot_type;
    }

    return 0;
}

static bool slot_type_match_status(uint32_t len, uint32_t slotType)
{
    uint32_t require_size = sizeof(slimfs_item_mgr_t) + len;
    if (require_size > (slotType / 2) &&
        require_size <= slotType) {
        return true;
    }

    return false;
}

static int32_t slimfs_add_item_withId(slimfs_impl_t* slimfsHdl, slimfs_item_info_t* item)
{
    if (slimfsHdl == NULL ||
        slimfsHdl->slimfs_ptr == NULL ||
        item == NULL) {
        SLIMFS_PRINT(SLIMFS_LOG_ERR, "parameter error for %s.\n", __func__);
        return SLIMFS_RET_INVALID_PARA;
    }

    int32_t ret = 0;
    uint64_t start_blk;
    uint32_t blk_offset;
    uint32_t slot_type;

    // calculate start blk and offset in blk
    ret = calculate_slot_postion_withId(slimfsHdl, item->item_mgr.id, &start_blk, &blk_offset, &slot_type);
    if (ret != 0) {
        return ret;
    }

    // confirm item len is match with slot type
    if (!slot_type_match_status(item->item_mgr.len, slot_type)) {
        SLIMFS_PRINT(SLIMFS_LOG_ERR, "item len(%d) is do not match slot type(%d).", item->item_mgr.len, slot_type);
        return SLIMFS_RET_INVALID_PARA;
    }

    if (slot_type < CONFIG_FLASH_SECTOR_SIZE) {
        medium_rw_with_slotbuf(slimfsHdl, 'r', start_blk, 1);
    }
    else {
        // blk offset is 0
    }

    slimfs_item_slot_t* item_slot = (slimfs_item_slot_t *)((uint8_t *)slimfsHdl->slot_buf + blk_offset);
    // check slot is invalid
    if (item_slot->item_mgr.id != CONFIG_INVALID_ITEM_ID) {
        SLIMFS_PRINT(SLIMFS_LOG_ERR, "invalid item id: 0x%x, the slot is not empty.\n", item->item_mgr.id);
        return SLIMFS_RET_INVALID_PARA;
    }

    // write item to slot
    item_slot->item_mgr = item->item_mgr;
    if (item->item_mgr.crc_en &&
        slimfsHdl->checksum_fun) {
        // calculate checksum for item content
        item_slot->item_mgr.crc = slimfsHdl->checksum_fun(item->content, item->item_mgr.len);
    }
    memcpy(item_slot->content, item->content, item->item_mgr.len);

    medium_rw_with_slotbuf(slimfsHdl, 'w', start_blk, (ROUNDUP(slot_type, CONFIG_FLASH_SECTOR_SIZE)) / CONFIG_FLASH_SECTOR_SIZE);

    // add {itemId, itemName} to cache
    struct cache_pair pair;
    pair.id = item->item_mgr.id;
    strcpy((char *)pair.name, (char *)item->item_mgr.name);
    slimfs_cache_add(slimfsHdl->cache_hdl, item->item_mgr.name, &pair);

    return 0;
}

static int32_t slimfs_add_item_withName(slimfs_impl_t* slimfsHdl, slimfs_item_info_t* item)
{
    if (slimfsHdl == NULL ||
        slimfsHdl->slimfs_ptr == NULL ||
        item == NULL) {
        SLIMFS_PRINT(SLIMFS_LOG_ERR, "parameter error for %s.\n", __func__);
        return SLIMFS_RET_INVALID_PARA;
    }

    if (strlen((char *)item->item_mgr.name) >= CONFIG_SLIMFS_ITEM_NAME_LEN) {
        SLIMFS_PRINT(SLIMFS_LOG_ERR, "item name len is to long for slimfs\n");
        return SLIMFS_RET_INVALID_PARA;
    }

    // traversel for find invalid id
    struct searchId_checkName sc_info;

    sc_info.item = item->item_mgr;
    sc_info.dup_name_flg = false;
    sc_info.find_id_flg = false;
    slimfs_traverse(slimfsHdl, traverse_action_searchId_checkName, &sc_info);

    if (sc_info.dup_name_flg) {
        SLIMFS_PRINT(SLIMFS_LOG_WARN, "add item faild with error item name duplicate.\n");
        return SLIMFS_RET_NAME_DUP;
    }

    if (!sc_info.find_id_flg) {
        SLIMFS_PRINT(SLIMFS_LOG_WARN, "add item faild with error no spare slot.\n");
        return SLIMFS_RET_NO_SPACE;
    }

    // add item with id
    item->item_mgr.id = sc_info.item.id;
    return slimfs_add_item_withId(slimfsHdl, item);
}

static int32_t slimfs_item_del_withId(slimfs_impl_t* slimfsHdl, uint32_t itemId)
{
    if (slimfsHdl == NULL ||
        slimfsHdl->slimfs_ptr == NULL) {
        SLIMFS_PRINT(SLIMFS_LOG_ERR, "parameter error for %s.\n", __func__);
        return SLIMFS_RET_INVALID_PARA;
    }

    int32_t ret = 0;
    uint64_t start_blk;
    uint32_t blk_offset;
    uint32_t slot_type;

    // calculate start blk and item offset in blk
    ret = calculate_slot_postion_withId(slimfsHdl, itemId, &start_blk, &blk_offset, &slot_type);
    if (ret != 0) {
        return ret;
    }

    // read item slot
    medium_rw_with_slotbuf(slimfsHdl, 'r', start_blk, 1);

    slimfs_item_slot_t* item_slot = (slimfs_item_slot_t *)((uint8_t *)slimfsHdl->slot_buf + blk_offset);
    // check item id
    if (item_slot->item_mgr.id == CONFIG_INVALID_ITEM_ID) {
        SLIMFS_PRINT(SLIMFS_LOG_ERR, "the slot is already released.\n");
        return SLIMFS_RET_NOT_FOUND;
    }

    // remove cache info
    slimfs_cache_remove(slimfsHdl->cache_hdl, item_slot->item_mgr.name);

    item_slot->item_mgr.id = CONFIG_INVALID_ITEM_ID;
    memset(item_slot->item_mgr.name, 0x00, CONFIG_SLIMFS_ITEM_NAME_LEN);
    strcpy((char *)item_slot->item_mgr.name, "NONE");
    item_slot->item_mgr.crc_en = 0;
    item_slot->item_mgr.len = 0;

    medium_rw_with_slotbuf(slimfsHdl
                           , 'w'
                           , start_blk
                           , 1);

    return SLIMFS_RET_OK;
}

static int32_t slimfs_item_del_withName(slimfs_impl_t* slimfsHdl, const char* name)
{
    if (slimfsHdl == NULL ||
        name == NULL) {
        SLIMFS_PRINT(SLIMFS_LOG_ERR, "parameter error for %s.\n", __func__);
        return SLIMFS_RET_INVALID_PARA;
    }

    uint32_t item_id = CONFIG_INVALID_ITEM_ID;
    search_itemId_by_item_name(slimfsHdl, name, &item_id, 0);
    if (item_id == CONFIG_INVALID_ITEM_ID) {
        SLIMFS_PRINT(SLIMFS_LOG_ERR, "can not search item: %s in slimfs.\n", name);
        return SLIMFS_RET_NOT_FOUND;
    }

    // delete item withId
    return slimfs_item_del_withId(slimfsHdl, item_id);
}

static int32_t slimfs_item_update_withId(slimfs_impl_t* slimfsHdl, uint32_t itemId, const void* content, uint32_t len)
{
    if (slimfsHdl == NULL ||
        content == NULL) {
        SLIMFS_PRINT(SLIMFS_LOG_ERR, "parameter error for %s.\n", __func__);
        return SLIMFS_RET_INVALID_PARA;
    }

    int32_t ret = 0;
    uint64_t start_blk;
    uint32_t blk_offset;
    uint32_t slot_type;

    // calculate start blk and item offset in blk
    ret = calculate_slot_postion_withId(slimfsHdl, itemId, &start_blk, &blk_offset, &slot_type);
    if (ret != 0) {
        return ret;
    }

    ret = medium_rw_with_slotbuf(slimfsHdl
                                , 'r'
                                , start_blk
                                , (ROUNDUP(slot_type, CONFIG_FLASH_SECTOR_SIZE)) / CONFIG_FLASH_SECTOR_SIZE);

    slimfs_item_slot_t* item_slot = (slimfs_item_slot_t *)((uint8_t *)slimfsHdl->slot_buf + blk_offset);
    // check item id
    if (item_slot->item_mgr.id == CONFIG_INVALID_ITEM_ID) {
        SLIMFS_PRINT(SLIMFS_LOG_ERR, "the slot is already released.\n");
        return SLIMFS_RET_NOT_FOUND;
    }

    // check item lenght
    if (len != item_slot->item_mgr.len) {
        SLIMFS_PRINT(SLIMFS_LOG_ERR, "%s len(%d) is different from the length(%d) which stored in slimfs.\n", __func__, len, item_slot->item_mgr.len);
        return SLIMFS_RET_INVALID_PARA;
    }

    if (item_slot->item_mgr.crc_en &&
        slimfsHdl->checksum_fun) {
        // calculate crc value
        item_slot->item_mgr.crc = slimfsHdl->checksum_fun(content, len);
    }
    memcpy(item_slot->content, content, len);

    ret = medium_rw_with_slotbuf(slimfsHdl
                                , 'w'
                                , start_blk
                                , (ROUNDUP(slot_type, CONFIG_FLASH_SECTOR_SIZE)) / CONFIG_FLASH_SECTOR_SIZE);

    return ret;
}

static int32_t slimfs_item_update_withName(slimfs_impl_t* slimfsHdl, const char* name, const void* content, uint32_t len)
{
    if (slimfsHdl == NULL ||
        name == NULL ||
        content == NULL) {
        SLIMFS_PRINT(SLIMFS_LOG_ERR, "parameter error for %s.\n", __func__);
        return SLIMFS_RET_INVALID_PARA;
    }

    uint32_t item_id = CONFIG_INVALID_ITEM_ID;
    search_itemId_by_item_name(slimfsHdl, name, &item_id, len);
    if (item_id == CONFIG_INVALID_ITEM_ID) {
        SLIMFS_PRINT(SLIMFS_LOG_ERR, "can not search item: %s in slimfs.\n", name);
        return SLIMFS_RET_NOT_FOUND;
    }

    return slimfs_item_update_withId(slimfsHdl, item_id, content, len);
}

static int32_t slimfs_item_read_withId(slimfs_impl_t* slimfsHdl, uint32_t itemId, void* content, uint32_t len)
{
    if (slimfsHdl == NULL ||
        content == NULL) {
        SLIMFS_PRINT(SLIMFS_LOG_ERR, "parameter error for %s.\n", __func__);
        return SLIMFS_RET_INVALID_PARA;
    }

    int32_t ret = 0;
    uint64_t start_blk;
    uint32_t blk_offset;
    uint32_t slot_type;

    // calculate start blk and item offset in blk
    ret = calculate_slot_postion_withId(slimfsHdl, itemId, &start_blk, &blk_offset, &slot_type);
    if (ret != 0) {
        return ret;
    }

    ret = medium_rw_with_slotbuf(slimfsHdl
                                , 'r'
                                , start_blk
                                , (ROUNDUP(slot_type, CONFIG_FLASH_SECTOR_SIZE)) / CONFIG_FLASH_SECTOR_SIZE);

    slimfs_item_slot_t* item_slot = (slimfs_item_slot_t *)((uint8_t *)slimfsHdl->slot_buf + blk_offset);
    // check item id
    if (item_slot->item_mgr.id == CONFIG_INVALID_ITEM_ID) {
        SLIMFS_PRINT(SLIMFS_LOG_ERR, "the slot is already released.\n");
        return SLIMFS_RET_NOT_FOUND;
    }

    // check length
    if (len != item_slot->item_mgr.len) {
        SLIMFS_PRINT(SLIMFS_LOG_ERR, "%s len(%d) is different from the length(%d) which stored in slimfs.\n", __func__, len, item_slot->item_mgr.len);
        return SLIMFS_RET_INVALID_PARA;
    }

    uint16_t crc_val = item_slot->item_mgr.crc;
    if (item_slot->item_mgr.crc_en &&
        slimfsHdl->checksum_fun) {
        // calculate crc value, confirm data is valid
        crc_val = slimfsHdl->checksum_fun(item_slot->content, len);
    }

    if (crc_val != item_slot->item_mgr.crc) {
        SLIMFS_PRINT(SLIMFS_LOG_ERR, "item %s crc checksum failed.\n", (char *)item_slot->item_mgr.name);
        return SLIMFS_RET_DATA_DAMAGE;
    }
    else {
        // do nothing
    }

    memcpy(content, item_slot->content, len);
    return SLIMFS_RET_OK;
}

static int32_t slimfs_item_read_withName(slimfs_impl_t* slimfsHdl, const char* name, void* content, uint32_t len)
{
    if (slimfsHdl == NULL ||
        name == NULL ||
        content == NULL) {
        SLIMFS_PRINT(SLIMFS_LOG_ERR, "parameter error for %s.\n", __func__);
        return SLIMFS_RET_INVALID_PARA;
    }

    uint32_t item_id = CONFIG_INVALID_ITEM_ID;
    search_itemId_by_item_name(slimfsHdl, name, &item_id, len);
    if (item_id == CONFIG_INVALID_ITEM_ID) {
        SLIMFS_PRINT(SLIMFS_LOG_ERR, "can not search item: %s in slimfs.\n", name);
        return SLIMFS_RET_NOT_FOUND;
    }

    return slimfs_item_read_withId(slimfsHdl, item_id, content, len);
}

int32_t slimfs_item_crc_switch_withId(slimfs_impl_t* slimfsHdl, uint32_t itemId, bool sta)
{
    if (slimfsHdl == NULL) {
        SLIMFS_PRINT(SLIMFS_LOG_ERR, "parameter error for %s.\n", __func__);
        return SLIMFS_RET_INVALID_PARA;
    }

    int32_t ret = 0;
    uint64_t start_blk;
    uint32_t blk_offset;
    uint32_t slot_type;

    // calculate start blk and item offset in blk
    ret = calculate_slot_postion_withId(slimfsHdl, itemId, &start_blk, &blk_offset, &slot_type);
    if (ret != 0) {
        return ret;
    }

    ret = medium_rw_with_slotbuf(slimfsHdl
                                , 'r'
                                , start_blk
                                , (ROUNDUP(slot_type, CONFIG_FLASH_SECTOR_SIZE)) / CONFIG_FLASH_SECTOR_SIZE);

    slimfs_item_slot_t* item_slot = (slimfs_item_slot_t *)((uint8_t *)slimfsHdl->slot_buf + blk_offset);
    // check item id
    if (item_slot->item_mgr.id == CONFIG_INVALID_ITEM_ID) {
        SLIMFS_PRINT(SLIMFS_LOG_ERR, "the slot is already released.\n");
        return SLIMFS_RET_NOT_FOUND;
    }

    // switch crc to on, cal content crc valud and write
    if (sta) {
        item_slot->item_mgr.crc_en = 1;

        // calculate crc value
        if (slimfsHdl->checksum_fun) {
            item_slot->item_mgr.crc = slimfsHdl->checksum_fun(item_slot->content, item_slot->item_mgr.len);
        }
    }
    else {
        // do nothing
    }

    item_slot->item_mgr.crc_en = sta;

    // write item slot header to device
    medium_rw_with_slotbuf(slimfsHdl, 'w', start_blk, 1);

    return 0;
}

int32_t slimfs_item_crc_switch_withName(slimfs_impl_t* slimfsHdl, const char* name, bool sta)
{
    if (slimfsHdl == NULL ||
        name == NULL) {
        SLIMFS_PRINT(SLIMFS_LOG_ERR, "parameter error for %s.\n", __func__);
        return SLIMFS_RET_INVALID_PARA;
    }

    uint32_t item_id = CONFIG_INVALID_ITEM_ID;
    search_itemId_by_item_name(slimfsHdl, name, &item_id, 0);
    if (item_id == CONFIG_INVALID_ITEM_ID) {
        SLIMFS_PRINT(SLIMFS_LOG_ERR, "can not search item: %s in slimfs.\n", name);
        return SLIMFS_RET_NOT_FOUND;
    }

    return slimfs_item_crc_switch_withId(slimfsHdl, item_id, sta);
}

/******************************************************************************
**@brief slimfs init
******************************************************************************/
int32_t slimfs_init()
{
    SLIMFS_PRINT(SLIMFS_LOG_DEBUG, "entry %s.\n", __func__);

    int32_t ret = 0;

    if (s_slimfs_hdl->init_flg) {
        LOGPRINT_ONCE("slifs device is already init \n");
        return SLIMFS_RET_OK;
    }

    if (CONFIG_SLIMFS_AUTO_FORMAT) {
        int32_t retry = 0;
        while (retry++ < CONFIG_SLIMFS_RETRY_TIMES) {
            ret = slimfs_init_impl(s_slimfs_hdl);
            if (ret == SLIMFS_RET_INVALID_FS) {
                slimfs_deinit_impl(s_slimfs_hdl);
                // format slifms partition
                slimfs_format_slot_default();
                continue;
            }
            else if (ret == 0){
                break;
            }
            else {
                slimfs_deinit_impl(s_slimfs_hdl);
            }
        }
    }
    else {
        if ((ret = slimfs_init_impl(s_slimfs_hdl)) != 0) {
            slimfs_deinit_impl(s_slimfs_hdl);
            return ret;
        }
    }

    return ret;
}

/******************************************************************************
**@brief slimfs deinit
******************************************************************************/
int32_t slimfs_deinit()
{
    if (s_slimfs_hdl->init_flg == false) {
        SLIMFS_PRINT(SLIMFS_LOG_ERR, "slimfs already deinit, donot need deinit again.\n");
        return 0;
    }
    return slimfs_deinit_impl(s_slimfs_hdl);
}

/******************************************************************************
**@brief slim format data
******************************************************************************/
int32_t slimfs_format_data()
{
    if (s_slimfs_hdl &&
        s_slimfs_hdl->init_flg) {
        SLIMFS_PRINT(SLIMFS_LOG_ERR, "slimfs %s is in use, please deinit it first. and then do %s\n"
                     , s_slimfs_hdl->slimfs_ptr->header.magic, __func__);
        return SLIMFS_RET_DEV_BUSY;
    }
    SLIMFS_PRINT(SLIMFS_LOG_DEBUG, "entry %s.", __func__);

    // init format handle and format data
    slimfs_init_impl(s_slimfs_format_hdl);
    slimfs_traverse(s_slimfs_format_hdl, traverse_action_format_data, NULL);
    slimfs_deinit_impl(s_slimfs_format_hdl);
    return 0;
}

/******************************************************************************
**@brief slimfs_destoy_data
******************************************************************************/
int32_t slimfs_destroy_data(int32_t blk, uint32_t offset)
{
    if (s_slimfs_hdl &&
        s_slimfs_hdl->init_flg) {
        SLIMFS_PRINT(SLIMFS_LOG_ERR, "slimfs %s is in use, please deinit it first. and then do %s\n"
                     , s_slimfs_hdl->slimfs_ptr->header.magic, __func__);
        return SLIMFS_RET_DEV_BUSY;
    }
    SLIMFS_PRINT(SLIMFS_LOG_DEBUG, "entry %s.", __func__);

    // init format handle and format data
    slimfs_init_impl(s_slimfs_format_hdl);
    MEDIUM_RW(s_slimfs_format_hdl, read, s_slimfs_format_hdl->slot_buf, blk, 1);
    uint8_t* buf = (uint8_t *)s_slimfs_format_hdl->slot_buf;
    memset(buf + offset, 0x00, 1);
    MEDIUM_RW(s_slimfs_format_hdl, write, s_slimfs_format_hdl->slot_buf, blk, 1);
    slimfs_deinit_impl(s_slimfs_format_hdl);

    return 0;
}

/******************************************************************************
**@brief slim format slot
******************************************************************************/
int32_t slimfs_format_slot(const slot_info_t* slotInfo, int32_t size)
{
    if (s_slimfs_hdl &&
        s_slimfs_hdl->init_flg) {
        SLIMFS_PRINT(SLIMFS_LOG_ERR, "slimfs %s is in use, please deinit it first. and then do %s\n"
                     , s_slimfs_hdl->slimfs_ptr->header.magic, __func__);
        return SLIMFS_RET_DEV_BUSY;
    }

    if (slotInfo == NULL) {
        return SLIMFS_RET_INVALID_PARA;
    }

    int32_t ret = 0;
    int32_t effective_size = 0;
    slot_info_t* slot_info = NULL;
    slimfs_layout_t* slimfs_layout_ptr = NULL;

    slot_info = malloc(sizeof(slot_info_t) * size);
    if (slot_info == NULL) {
        return SLIMFS_RET_NO_MEMORY;
    }

    // slot info check
    ret = check_format_slot_info(slotInfo, size, slot_info, &effective_size);
    if (ret != SLIMFS_RET_OK) {
        goto exit;
    }

    //@TODO ensure slotinfo is ascending order


    // fill slimfs header
    uint32_t slimfs_mgr_size = sizeof(slimfs_header_t) + sizeof(slimfs_slot_mgr_t) * effective_size;
    slimfs_mgr_size = ROUNDUP(slimfs_mgr_size, CONFIG_FLASH_SECTOR_SIZE);
#if defined(CONFIG_FLASH_TYPE_MMC)
    slimfs_layout_ptr = memalign(CACHE_LINE, slimfs_mgr_size);
#else
    slimfs_layout_ptr = malloc(slimfs_mgr_size);
#endif /* end CONFIG_FLASH_TYPE_MMC */
    if (slimfs_layout_ptr == NULL) {
        SLIMFS_PRINT(SLIMFS_LOG_ERR, "malloc memory failed for %s.\n", __func__);
        ret =  SLIMFS_RET_NO_MEMORY;
        goto exit;
    }
    memset(slimfs_layout_ptr, 0x00, slimfs_mgr_size);
    memcpy(slimfs_layout_ptr->header.magic, CONFIG_SLIMFS_MAGIC_NUMBER, strlen(CONFIG_SLIMFS_MAGIC_NUMBER));
    memcpy(slimfs_layout_ptr->header.major, CONFIG_SLIMFS_MAJOR_NUMBER, CONFIG_SLIMFS_MAJOR_SIZE);
    memcpy(slimfs_layout_ptr->header.manor, CONFIG_SLIMFS_MANOR_NUMBER, CONFIG_SLIMFS_MANOR_SIZE);
    slimfs_layout_ptr->header.slot_num = effective_size;
    slimfs_layout_ptr->header.format_sta = SLIMFS_FORMATED;

    // calculate startblk & blk number
    uint32_t offset = slimfs_mgr_size / CONFIG_FLASH_SECTOR_SIZE;
    slimfs_slot_mgr_t* slot_mgr_ptr = slimfs_layout_ptr->slot_mgr;
    uint32_t slots_size = 0;

    for (int32_t i = 0; i < effective_size; i++) {
        // CONFIG_FLASH_SECTOR_SIZE alignment
        slots_size = slot_info[i].slot_mgr.slot_type * slot_info[i].slot_number;
        slots_size = ROUNDUP(slots_size, CONFIG_FLASH_SECTOR_SIZE);

        slot_mgr_ptr[i].slot_type = slot_info[i].slot_mgr.slot_type;
        slot_mgr_ptr[i].start_block = offset;
        slot_mgr_ptr[i].block_number = slots_size / CONFIG_FLASH_SECTOR_SIZE;
        offset += slot_mgr_ptr[i].block_number;
    }

    if (offset * CONFIG_FLASH_SECTOR_SIZE > CONFIG_SLIMFS_PARTITION_SIZE) {
        CONVERT_BYTES2UNIT(offset * CONFIG_FLASH_SECTOR_SIZE, buf1);
        CONVERT_BYTES2UNIT(CONFIG_SLIMFS_PARTITION_SIZE, buf2);
        SLIMFS_PRINT(SLIMFS_LOG_ERR, "not enough partition space for formating slimfs slot. expect: %s, actual: %s\n"
                    , buf1
                    , buf2);

        ret = SLIMFS_RET_NO_SPACE;
        goto exit;
    }

    dump_slimfs_mgr_info(slimfs_layout_ptr, "slimfs format slot info");

    // write slimfs header & slot mgr to flash
    slimfs_medium_ops_t* medium_ops = NULL;
    void* medium_hdl = NULL;
    get_medium_ops(&medium_ops);
    if (medium_ops->init &&
        medium_ops->write &&
        medium_ops->deinit) {
        medium_hdl = medium_ops->init((void *)CONFIG_FLASH_PARTITION);
        medium_ops->write(medium_hdl
                          , (void *)slimfs_layout_ptr
                          , CONFIG_FLASH_SECTOR_START
                          , slimfs_mgr_size / CONFIG_FLASH_SECTOR_SIZE);
        medium_ops->deinit(medium_hdl);
    }
    else {
        SLIMFS_PRINT(SLIMFS_LOG_ERR, "get medium ops for format slot failed.\n");
        goto exit;
    }

    // format data
    ret = slimfs_format_data();

exit:
    if (slot_info) {
        free(slot_info);
    }

    if (slimfs_layout_ptr) {
        free(slimfs_layout_ptr);
    }

    return ret;
}

/******************************************************************************
**@brief format slot use defaule config
******************************************************************************/
int32_t slimfs_format_slot_default()
{
    return slimfs_format_slot(s_slot_info, ARRY_SIZE(s_slot_info));
}

/******************************************************************************
**@brief query slimfs header info
******************************************************************************/
int32_t slimfs_header_query(slimfs_header_t* header)
{
    if (!s_slimfs_hdl->init_flg) {
        LOGPRINT_ONCE("slimfs is invalid for %s, please init first.\n", __func__);
        // SLIMFS_PRINT(SLIMFS_LOG_ERR, "slimfs is invalid for %s, please init first.\n", __func__);
        return SLIMFS_RET_INVALID_FS;
    }

    if (header == NULL) {
        return SLIMFS_RET_INVALID_PARA;
    }

    *header = s_slimfs_hdl->slimfs_ptr->header;

    return SLIMFS_RET_OK;
}

/******************************************************************************
**@brief query slimfs slots info
******************************************************************************/
int32_t slimfs_slots_query(slot_info_t* slotsInfo, uint32_t* slotsNum)
{
    if (!s_slimfs_hdl->init_flg) {
        LOGPRINT_ONCE("slimfs is invalid for %s, please init first.\n", __func__);
        return SLIMFS_RET_INVALID_FS;
    }

    if (slotsInfo == NULL) {
        return SLIMFS_RET_INVALID_PARA;
    }
    int32_t ret = 0;
    // acquire mutex and release mutex
    slimfs_mutex_acquire(s_slimfs_hdl->mutex);

    // assignment slot base infomation
    for (int32_t i = 0; i < s_slimfs_hdl->slimfs_ptr->header.slot_num; i++) {
        slotsInfo[i].slot_mgr = s_slimfs_hdl->slimfs_ptr->slot_mgr[i];
        slotsInfo[i].n_index = i;
        slotsInfo[i].slot_number =
            slotsInfo[i].slot_mgr.block_number * CONFIG_FLASH_SECTOR_SIZE / slotsInfo[i].slot_mgr.slot_type;
        slotsInfo[i].slot_used = 0;
    }

    if (slotsNum) {
        *slotsNum = s_slimfs_hdl->slimfs_ptr->header.slot_num;
    }

    // traverse slimfs for statistics used slot
    ret = slimfs_traverse(s_slimfs_hdl, traverse_action_statistics_used_slot, slotsInfo);

    slimfs_mutex_release(s_slimfs_hdl->mutex);

    return ret;
}

/******************************************************************************
**@brief slimfs query item info, including itemId, crc status & val, len info
******************************************************************************/
int32_t slimfs_item_query(const slimfs_search_t* search, slimfs_item_mgr_t* itemMgr)
{
    if (!s_slimfs_hdl->init_flg) {
        LOGPRINT_ONCE("slimfs is invalid for %s, please init first.\n", __func__);
        return SLIMFS_RET_INVALID_FS;
    }

    if (search == NULL || itemMgr == NULL) {
        return SLIMFS_RET_INVALID_PARA;
    }

    int32_t ret = 0;
    // acquire mutex and release mutex
    slimfs_mutex_acquire(s_slimfs_hdl->mutex);

    uint32_t item_id = CONFIG_INVALID_ITEM_ID;
    if (search->search_type == SLIMFS_SEARCH_BY_NAME) {
        search_itemId_by_item_name(s_slimfs_hdl, (const char *)search->name, &item_id, 0);
        if (item_id == CONFIG_INVALID_ITEM_ID) {
            SLIMFS_PRINT(SLIMFS_LOG_ERR, "can not search item: %s in slimfs.\n", search->name);
            ret = SLIMFS_RET_NOT_FOUND;
        }
    }
    else {
        item_id = search->id;
    }

    if (item_id != CONFIG_INVALID_ITEM_ID) {
        uint64_t start_blk;
        uint32_t blk_offset;
        uint32_t slot_type;

        // calculate start blk and offset in blk
        ret = calculate_slot_postion_withId(s_slimfs_hdl, item_id, &start_blk, &blk_offset, &slot_type);
        if (ret != 0) {
            goto exit;
        }

        // read block
        medium_rw_with_slotbuf(s_slimfs_hdl, 'r', start_blk, 1);

        slimfs_item_slot_t* item_slot = (slimfs_item_slot_t *)((uint8_t *)s_slimfs_hdl->slot_buf + blk_offset);

        *itemMgr = item_slot->item_mgr;
    }

exit:
    // release mutex
    slimfs_mutex_release(s_slimfs_hdl->mutex);

    return ret;
}

/******************************************************************************
**@brief slimfs item dump
******************************************************************************/
void slimfs_item_dump(uint32_t NIndexMask, uint32_t slotIndex, int32_t len)
{
    if (!s_slimfs_hdl->init_flg) {
        SLIMFS_PRINT(SLIMFS_LOG_ERR, "slimfs is invalid for %s, please init first.\n", __func__);
        return ;
    }

    // acquire mutex and release mutex
    slimfs_mutex_acquire(s_slimfs_hdl->mutex);

    struct dump_condition dump_con = {
        .N_index_bitmap = NIndexMask,
        .slot_index = slotIndex,
        .len = len,
    };
    slimfs_traverse(s_slimfs_hdl, traverse_action_dump_item, &dump_con);

    slimfs_mutex_release(s_slimfs_hdl->mutex);
    return;
}

/******************************************************************************
**@brief add item
******************************************************************************/
int32_t slimfs_item_add(slimfs_item_info_t* item)
{
    if (!s_slimfs_hdl->init_flg) {
        LOGPRINT_ONCE("slimfs is invalid for %s, please init first.\n", __func__);
        return SLIMFS_RET_INVALID_FS;
    }

    int32_t ret = 0;

    // acquire mutex and release mutex
    slimfs_mutex_acquire(s_slimfs_hdl->mutex);

    if (item->item_mgr.id != CONFIG_INVALID_ITEM_ID) {
        // traversel for find dup name
        struct searchId_checkName sc_info;

        sc_info.item = item->item_mgr;
        sc_info.dup_name_flg = false;
        sc_info.find_id_flg = false;

        slimfs_traverse(s_slimfs_hdl, traverse_action_searchId_checkName, &sc_info);
        if (sc_info.dup_name_flg) {
            SLIMFS_PRINT(SLIMFS_LOG_WARN, "add item faild with error item name duplicate.\n");
            slimfs_mutex_release(s_slimfs_hdl->mutex);
            return SLIMFS_RET_NAME_DUP;
        }

        ret = slimfs_add_item_withId(s_slimfs_hdl, item);
    }
    else {
        ret = slimfs_add_item_withName(s_slimfs_hdl, item);
    }

    slimfs_mutex_release(s_slimfs_hdl->mutex);
    return ret;
}

/******************************************************************************
**@brief del item
******************************************************************************/
int32_t slimfs_item_del(const slimfs_search_t* search)
{
    if (!s_slimfs_hdl->init_flg) {
        LOGPRINT_ONCE("slimfs is invalid for %s, please init first.\n", __func__);
        return SLIMFS_RET_INVALID_FS;
    }

    if (search == NULL) {
        SLIMFS_PRINT(SLIMFS_LOG_ERR, "parameter error for %s\n", __func__);
        return SLIMFS_RET_INVALID_PARA;
    }
    int32_t ret = 0;

    // acquire mutex and release mutex
    slimfs_mutex_acquire(s_slimfs_hdl->mutex);

    if (search->search_type == SLIMFS_SEARCH_BY_NAME) {

        ret = slimfs_item_del_withName(s_slimfs_hdl, (const char *)search->name);
    }
    else {
        ret = slimfs_item_del_withId(s_slimfs_hdl, search->id);
    }

    slimfs_mutex_release(s_slimfs_hdl->mutex);
    return ret;
}

/******************************************************************************
**@brief update item content
******************************************************************************/
int32_t slimfs_item_update(const slimfs_search_t* search, const void* content, uint32_t len)
{
    if (!s_slimfs_hdl->init_flg) {
        LOGPRINT_ONCE("slimfs is invalid for %s, please init first.\n", __func__);
        return SLIMFS_RET_INVALID_FS;
    }

    if (search == NULL ||
        content == NULL) {
        SLIMFS_PRINT(SLIMFS_LOG_ERR, "parameter error for %s\n", __func__);
        return SLIMFS_RET_INVALID_PARA;
    }

    int32_t ret = 0;

    // acquire mutex and release mutex
    slimfs_mutex_acquire(s_slimfs_hdl->mutex);

    if (search->search_type == SLIMFS_SEARCH_BY_NAME) {
        ret = slimfs_item_update_withName(s_slimfs_hdl, (const char*)search->name, content, len);
    }
    else if (search->search_type == SLIMFS_SEARCH_BY_ID) {
        ret = slimfs_item_update_withId(s_slimfs_hdl, search->id, content, len);
    }
    else {
        SLIMFS_PRINT(SLIMFS_LOG_ERR, "slimfs can not support search type: %d.\n", search->search_type);
        ret = SLIMFS_RET_INVALID_PARA;
    }

    slimfs_mutex_release(s_slimfs_hdl->mutex);

    return ret;
}

/******************************************************************************
**@brief read item content
******************************************************************************/
int32_t slimfs_item_read(const slimfs_search_t* search, void* content, uint32_t len)
{
    if (!s_slimfs_hdl->init_flg) {
        LOGPRINT_ONCE("slimfs is invalid for %s, please init first.\n", __func__);
        return SLIMFS_RET_INVALID_FS;
    }

    if (search == NULL ||
        content == NULL) {
        SLIMFS_PRINT(SLIMFS_LOG_ERR, "parameter error for %s\n", __func__);
        return SLIMFS_RET_INVALID_PARA;
    }

    int32_t ret = 0;

    // acquire mutex and release mutex
    slimfs_mutex_acquire(s_slimfs_hdl->mutex);

    if (search->search_type == SLIMFS_SEARCH_BY_NAME) {
        ret = slimfs_item_read_withName(s_slimfs_hdl, (const char*)search->name, content, len);
    }
    else if (search->search_type == SLIMFS_SEARCH_BY_ID) {
        ret = slimfs_item_read_withId(s_slimfs_hdl, search->id, content, len);
    }
    else {
        SLIMFS_PRINT(SLIMFS_LOG_ERR, "slimfs can not support search type: %d.\n", search->search_type);
        ret = SLIMFS_RET_INVALID_PARA;
    }

    slimfs_mutex_release(s_slimfs_hdl->mutex);

    return ret;
}

/******************************************************************************
**@brief item ctc switch status control
******************************************************************************/
int32_t slimfs_item_crc_switch(const slimfs_search_t* search, bool sta)
{
    if (!s_slimfs_hdl->init_flg) {
        SLIMFS_PRINT(SLIMFS_LOG_ERR, "slimfs is invalid for %s, please init first.\n", __func__);
        return SLIMFS_RET_INVALID_FS;
    }

    if (search == NULL) {
        SLIMFS_PRINT(SLIMFS_LOG_ERR, "parameter error for %s\n", __func__);
        return SLIMFS_RET_INVALID_PARA;
    }

    int32_t ret = 0;

    // acquire mutex and release mutex
    slimfs_mutex_acquire(s_slimfs_hdl->mutex);

    if (search->search_type == SLIMFS_SEARCH_BY_NAME) {
        ret = slimfs_item_crc_switch_withName(s_slimfs_hdl, (const char *)search->name, sta);
    }
    else if (search->search_type == SLIMFS_SEARCH_BY_ID) {
        ret = slimfs_item_crc_switch_withId(s_slimfs_hdl, search->id, sta);
    }
    else {
        SLIMFS_PRINT(SLIMFS_LOG_ERR, "slimfs can not support search type: %d.\n", search->search_type);
        ret = SLIMFS_RET_INVALID_PARA;
    }

    slimfs_mutex_release(s_slimfs_hdl->mutex);

    return ret;
}

/******************************************************************************
**@brief show slimfs cache info
******************************************************************************/
void slimfs_show_cache(void)
{
    if (!s_slimfs_hdl->init_flg) {
        SLIMFS_PRINT(SLIMFS_LOG_ERR, "slimfs is invalid for %s, please init first.\n", __func__);
        return;
    }

    slimfs_cache_dump(s_slimfs_hdl->cache_hdl);
}

/******************************************************************************
**@brief show slimfs debug info
******************************************************************************/
void slimfs_show_debug(void)
{
    if (!s_slimfs_hdl->init_flg) {
        SLIMFS_PRINT(SLIMFS_LOG_ERR, "slimfs is invalid for %s, please init first.\n", __func__);
        return;
    }
    slimfs_mutex_acquire(s_slimfs_hdl->mutex);

    int32_t slot_type_max = s_slimfs_hdl->slimfs_ptr->slot_mgr[s_slimfs_hdl->slimfs_ptr->header.slot_num - 1].slot_type;
    printf("slimfs header dump: \n");
    slimfs_hexdump(s_slimfs_hdl->slimfs_ptr, CONFIG_FLASH_SECTOR_SIZE, 0);

    printf("slot buffer dump: \n");
    slimfs_hexdump(s_slimfs_hdl->slot_buf, MIN(slot_type_max, 1024), 0);

    slimfs_mutex_release(s_slimfs_hdl->mutex);
}

/* end of file sdrv_slimfs_impl.c */