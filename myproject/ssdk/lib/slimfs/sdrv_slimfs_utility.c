/**
 * @file sdrv_slimfs_utility.c
 *
 * Copyright (c) 2023 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: utility for slimfs
 *
 * Revision History:
 * -----------------
 */
/* =========================== include file session ======================== */
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "sdrv_slimfs_config.h"
#include "sdrv_slimfs_impl.h"
#include "sdrv_slimfs.h"
#include "sdrv_slimfs_log.h"
#include "common_utility.h"
/* =========================== marco define session ======================== */
#if (CONFIG_SLIMFS_OS_TYPE == CONFIG_SLIMFS_OS_BAREMETAL)
#define PARA_INDEX                  0
#elif (CONFIG_SLIMFS_OS_TYPE == CONFIG_SLIMFS_OS_LK)
#include <lib/console.h>
#define PARA_INDEX                  1
#elif (CONFIG_SLIMFS_OS_TYPE == CONFIG_SLIMFS_OS_CMSIS)
#include <CLI.h>
#define PARA_INDEX                  0
#endif
/* =========================== enumeration define session ================== */
/* =========================== structure define session ==================== */
/* type of subcommand function */
typedef void (*cmd_entry)(int32_t argc, char* argv[]);
typedef struct subCmd_mgr {
    const char* sub_cmd;
    int32_t par_num;
    cmd_entry entry;
}subCmd_mgr_t;
/* =========================== function declare session ==================== */
/* =========================== global value define session ================= */
/* =========================== static value define session ================= */
/* =========================== function implementation session ============= */

static int32_t get_options_keyval(char* arg, char** key, char** val)
{
    if (arg == NULL || key == NULL || val == NULL) {
        SLIMFS_PRINT(SLIMFS_LOG_ERR, "parameter error for %s\n", __func__);
        return -1;
    }
    char arg_buf[256];
    strcpy(arg_buf, arg);

    char *p = arg;
    *key = NULL;
    *val = NULL;

    while (*p != '\0') {
        if (*p != '-' && *key == NULL) {
            *key = p;
        }
        if (*p == '=') {
            *p = '\0';
            *val = ++p;
            break;
        }
        p++;
    }

    if (*key == NULL || *val == NULL) {
        SLIMFS_PRINT(SLIMFS_LOG_ERR, "invalid args: %s\n", arg_buf);
        return -1;
    }

    return 0;
}

/**
 * @brief slimfs init
 *
 * @param argc parameter number: 0
 * @param argv
 */
static void slimfs_utl_init(int32_t argc, char* argv[])
{
    slimfs_init();
}

/**
 * @brief
 *
 */
static void slimfs_utl_deinit(int32_t argc, char* argv[])
{
    slimfs_deinit();
}

/**
 * @brief dump slimfs item info
 *
 * @param argc args number
 * @param argv args
 *  -nindexs=<N_index bit map>
 *  -slotindex=<slot index>, 0xFFFFFF is invalid slotindex, ignore it
 *  -len=<dump content len>
 */
static void slimfs_utl_dump(int32_t argc, char* argv[])
{
    if (slimfs_init() != SLIMFS_RET_OK) {
        printf("slimfs init failed for %s.\n", __func__);
        return;
    }

    char* key = NULL;
    char* val = NULL;
    char* p = NULL;
    slot_info_t* slots = NULL;
    int32_t ret = 0;
    uint32_t Nindex_bit_map = slot_type_max - 1;
    uint32_t slot_index = 0xFFFFFF;
    uint16_t ret_val = 0;
    int32_t len = -1;

    for (int32_t i = 0; i < argc; i++) {
        if (get_options_keyval(argv[i], &key, &val) != 0) {
            return ;
        }

        if (strcmp(key, "nindexs") == 0) {
            ret_val = strtodigit(val, &p, 0);
            if (val == p) {
                printf("invalid value: %s for args: %s\n", val, key);
                return ;
            }
            Nindex_bit_map = ret_val;
        }
        else if (strcmp(key, "slotindex") == 0) {
            ret_val = strtodigit(val, &p, 0);
            if (val == p) {
                printf("invalid value: %s for args: %s\n", val, key);
                return ;
            }
            slot_index = ret_val;
        }
        else if (strcmp(key, "len") == 0) {
            len = strtodigit(val, NULL, 0);
        }
        else {
            printf("invalid options %s for slimfs dump.\n", argv[i]);
            return ;
        }
    }

    if ((Nindex_bit_map == (slot_type_max - 1)) && slot_index == 0xFFFFFF) {
        printf("dump slimfs all slot item:\n");
    }
    else {
        char buf[256];
        memset(buf, 0x00, sizeof(buf));

        slimfs_header_t slimfs_header;
        ret = slimfs_header_query(&slimfs_header);
        if (ret != 0) {
            return;
        }

        slot_info_t* slots = malloc(sizeof(slot_info_t) * slimfs_header.slot_num);
        if (slots == NULL) {
            printf("malloc memory failed.\n");
            return;
        }

        ret = slimfs_slots_query(slots, NULL);
        if (ret != 0) {
            goto exit;
        }
        for (int32_t i = 0; i < slimfs_header.slot_num; i++) {
            if (Nindex_bit_map & (1 << slots[i].n_index)) {
                sprintf(&buf[strlen(buf)], "%d,", slots[i].slot_mgr.slot_type);
            }
        }
        buf[strlen(buf) - 1] = '\0';

        if (slot_index == 0xFFFFFF) {
            printf("dump slimfs %s slots item:\n", buf);
        }
        else {
            printf("dump slimfs %s slots item, with slot index: %d\n", buf, slot_index);
        }
    }

    slimfs_item_dump(Nindex_bit_map, slot_index, len);

exit:
    free(slots);

    return;
}

/**
 * @brief utl add item
 *
 * @param argc args number, variable
 * @param argv
 *  -name=[name]
 *  -id=<id>
 *  -crcen=1/0
 *  -len=[number]
 *  -content=[string]
 */
static void slimfs_utl_add(int32_t argc, char* argv[])
{
    if (slimfs_init() != SLIMFS_RET_OK) {
        printf("slimfs init failed for %s.\n", __func__);
        return ;
    }
    char* key = NULL;
    char* val = NULL;
    char* p = NULL;
    void* content_ptr = NULL;
    char* name_ptr = NULL;
    char* measure_ptr = NULL;
    // uint32_t digit_val = 0;
    slimfs_item_info_t item_info = {
        .content = NULL,
    };
    item_info.item_mgr.id = CONFIG_INVALID_ITEM_ID;

    for (int32_t i = 0; i < argc; i++) {
        if (get_options_keyval(argv[i], &key, &val) != 0) {
            return ;
        }

        if (strcmp(key, "name") == 0) {
            name_ptr = val;
        }
        else if (strcmp(key, "id") == 0) {
            item_info.item_mgr.id = strtodigit(val, &p, 0);

        }
        else if (strcmp(key, "crcen") == 0) {
            item_info.item_mgr.crc_en = strtodigit(val, &p, 0);
        }
        else if (strcmp(key, "len") == 0) {
            item_info.item_mgr.len = strtodigit(val, &p, 0);
            item_info.content = malloc(item_info.item_mgr.len);
            if (item_info.content == NULL) {
                printf("malloc memory failed.\n");
                break;
            }
            memset(item_info.content, 0x00, item_info.item_mgr.len);
        }
        else if (strcmp(key, "content") == 0) {
            content_ptr = val;
        }
        else if (strcmp(key, "measure") == 0) {
            measure_ptr = val;
        }
        else {
            printf("invalid options: %s for [slimfs add] command.\n", argv[i]);
            if (item_info.content) {
                free(item_info.content);
            }
            return;
        }
    }

    if (item_info.content != NULL &&
        content_ptr != NULL &&
        name_ptr != NULL &&
        strlen(name_ptr) < CONFIG_SLIMFS_ITEM_NAME_LEN &&
        strlen(content_ptr) < item_info.item_mgr.len) {
        uint32_t begin = 0, end = 0;
        int32_t ret = 0;

        strcpy(item_info.content, content_ptr);
        strcpy((char *)item_info.item_mgr.name, name_ptr);
        begin = slimfs_get_us_tick();
        ret = slimfs_item_add(&item_info);
        end = slimfs_get_us_tick();

        if (ret == 0 &&
            measure_ptr != NULL &&
            strcmp(measure_ptr, "on") == 0) {
            printf("%s cost time %d ms.\n", __func__, end - begin);
        }
    }
    else {
        printf("please ensure -len & -content & -name options\n");
    }

    if (item_info.content) {
        free(item_info.content);
    }

    return;
}

/**
 * @brief delete item
 *
 * @param argc parameter number: 1
 * @param argv
 * -id=<itemId> delete item by itemid
 * -name=<itemName> delete item by item name
 */
static void slimfs_utl_del(int32_t argc, char* argv[])
{
    if (slimfs_init() != SLIMFS_RET_OK) {
        printf("slimfs init failed for %s.\n", __func__);
        return ;
    }

    char* key = NULL;
    char* val = NULL;
    char* measure_ptr = NULL;
    slimfs_search_t search = {
        .id = SLIMFS_SEARCH_INVALID_TYPE,
    };

    for (int32_t i = 0; i < argc; i++) {
        if (get_options_keyval(argv[i], &key, &val) != 0) {
            return ;
        }

        if (strcmp(key, "name") == 0) {
            search.search_type = SLIMFS_SEARCH_BY_NAME;
            if (strlen(val) >= CONFIG_SLIMFS_ITEM_NAME_LEN) {
                search.search_type = SLIMFS_SEARCH_INVALID_TYPE;
            }
            else {
                strcpy((char *)search.name, val);
            }
        }
        else if (strcmp(key, "id") == 0) {
            search.search_type = SLIMFS_SEARCH_BY_ID;
            search.id = strtodigit(val, NULL, 0);
        }
        else if (strcmp(key, "measure") == 0) {
            measure_ptr = val;
        }
        else {
            printf("invalid options: %s for [slimfs del] command.\n", argv[0]);
            return;
        }
    }
    if (search.id != SLIMFS_SEARCH_INVALID_TYPE) {
        uint32_t begin = 0, end = 0;
        int32_t ret = 0;

        begin = slimfs_get_us_tick();
        ret = slimfs_item_del(&search);
        end = slimfs_get_us_tick();

        if (ret == 0 &&
            measure_ptr != NULL &&
            strcmp(measure_ptr, "on") == 0) {
            printf("%s cost time %d ms.\n", __func__, end - begin);
        }
    }
    else {
        printf("please ensure -name or -len options\n");
    }
    return;
}

/**
 * @brief slimfs write item
 *
 * @param argc sub cmd args number
 * @param argv sub cmd args
 * -id=[id]
 * -name=[name]
 * -len=[lenght]
 * -content=[item content]
 */
static void slimfs_utl_write(int32_t argc, char* argv[])
{
    if (slimfs_init() != SLIMFS_RET_OK) {
        printf("slimfs init failed for %s.\n", __func__);
        return ;
    }

    slimfs_search_t search = {
        .search_type = SLIMFS_SEARCH_INVALID_TYPE,
    };
    uint32_t len = 0;
    uint8_t* content = NULL;
    void* content_ptr = NULL;
    char* measure_ptr = NULL;

    char* key = NULL;
    char* val = NULL;
    char* p = NULL;

    for (int32_t i = 0; i < argc; i++) {
        if (get_options_keyval(argv[i], &key, &val) != 0) {
            return ;
        }

        if (strcmp(key, "name") == 0) {
            search.search_type = SLIMFS_SEARCH_BY_NAME;

            if (strlen(val) >= CONFIG_SLIMFS_ITEM_NAME_LEN) {
                search.search_type = SLIMFS_SEARCH_INVALID_TYPE;
            }
            else {
                strcpy((char *)search.name, val);
            }
        }
        else if (strcmp(key, "id") == 0) {
            search.search_type = SLIMFS_SEARCH_BY_ID;
            search.id = strtodigit(val, NULL, 0);
        }
        else if (strcmp(key, "len") == 0) {
            len = strtodigit(val, &p, 0);
            content = malloc(len);
            if (content == NULL) {
                printf("malloc memory failed.\n");
                break;
            }
            memset(content, 0x00, len);
        }
        else if (strcmp(key, "content") == 0) {
            content_ptr = val;
        }
        else if (strcmp(key, "measure") == 0) {
            measure_ptr = val;
        }
        else {
            printf("invalid options: %s for [slimfs write] command.\n", argv[i]);
            return;
        }
    }

    if (content != NULL &&
        content_ptr != NULL &&
        (strlen(content_ptr) < len) &&
        search.search_type != SLIMFS_SEARCH_INVALID_TYPE) {

        uint32_t begin = 0, end = 0;
        int32_t ret = 0;

        strcpy((char *)content, content_ptr);
        begin = slimfs_get_us_tick();
        ret = slimfs_item_update(&search, content, len);
        end = slimfs_get_us_tick();

        if (ret == 0 &&
            measure_ptr != NULL &&
            strcmp(measure_ptr, "on") == 0) {
            printf("%s cost time %d ms.\n", __func__, end - begin);
        }
    }
    else {
        printf("please ensure -len & -content & -name or -id options.\n");
    }

    if (content) {
        free(content);
    }
    return;
}

static void show_item_content(const void* content, int32_t showLen, const char* type)
{
    if (strcmp(type, "str") == 0) {
        printf("%s\n", (char *)content);
    }
    else if (strcmp(type, "int") == 0) {
        printf("%d\n", *(int32_t *)content);
    }
    else if (strcmp(type, "float") == 0) {
        printf("%f\n", *(float *)content);
    }
    else if (strcmp(type, "hex") == 0) {
        printf("\n");
        slimfs_hexdump(content, showLen, 0);
    }
    else {
        printf("\n");
        slimfs_hexdump(content, showLen, 0);
    }
}

/**
 * @brief slimfs item read
 *
 * @param argc sub cmd args number
 * @param argv sub cmd args
 * -id=<itemId>
 * -name=<item name>
 * -len=[length] keep same with item add len
 * -type=<str/float/bin/int> item content show type
 */
static void slimfs_utl_read(int32_t argc, char* argv[])
{
    if (slimfs_init() != SLIMFS_RET_OK) {
        printf("slimfs init failed for %s.\n", __func__);
        return ;
    }
    slimfs_search_t search = {
        .search_type = SLIMFS_SEARCH_INVALID_TYPE,
    };
    uint32_t len = 0;
    uint32_t show_len = 0;
    uint8_t* content = NULL;
    char* measure_ptr = NULL;
    const char* type = NULL;

    char* key = NULL;
    char* val = NULL;

    for (int32_t i = 0; i < argc; i++) {
        if (get_options_keyval(argv[i], &key, &val) != 0) {
            return ;
        }

        if (strcmp(key, "name") == 0) {
            search.search_type = SLIMFS_SEARCH_BY_NAME;

            if (strlen(val) >= CONFIG_SLIMFS_ITEM_NAME_LEN) {
                search.search_type = SLIMFS_SEARCH_INVALID_TYPE;
            }
            else {
                strcpy((char *)search.name, val);
            }
        }
        else if (strcmp(key, "id") == 0) {
            search.search_type = SLIMFS_SEARCH_BY_ID;
            search.id = strtodigit(val, NULL, 0);
        }
        else if (strcmp(key, "len") == 0) {
            len = strtodigit(val, NULL, 0);
            content = malloc(len);
            if (content == NULL) {
                printf("malloc memory failed.\n");
                break;
            }
            memset(content, 0x00, len);
        }
        else if (strcmp(key, "type") == 0) {
            type = val;
        }
        else if (strcmp(key, "showlen") == 0) {
            show_len = strtodigit(val, NULL, 0);
        }
        else if (strcmp(key, "measure") == 0) {
            measure_ptr = val;
        }
        else {
            printf("invalid options: %s for [slimfs read] command.\n", argv[i]);
            return;
        }
    }

    if (content != NULL &&
        search.search_type != SLIMFS_SEARCH_INVALID_TYPE) {
        uint32_t begin, end = 0;
        int32_t ret = 0;

        begin = slimfs_get_us_tick();
        ret = slimfs_item_read(&search, content, len);
        end = slimfs_get_us_tick();

        if (ret == 0) {
            printf("read item result: ");
            show_item_content(content
                            , ((show_len == 0) ? (MIN(len, 32)) : (show_len))
                            , ((type == NULL) ? ("bin") : (type)));
            if (measure_ptr != NULL &&
                strcmp(measure_ptr, "on") == 0) {
                printf("%s cost time %d ms.\n", __func__, end - begin);
            }
        }
    }
    else {
        printf("please ensure -len & -name or -id options.\n");
    }

    if (content) {
        free(content);
    }
    return;
}

/**
 * @brief slimfs format
 *
 * @param argc sub cmd args number
 * @param argv sub cmd args
 * -type=[format type] data/slot
 * -slots=<slotType,slotNumber,...>
 */
static void slimfs_utl_format(int32_t argc, char* argv[])
{
    char* key = NULL;
    char* val = NULL;
    char* type = NULL;
    char* slots_arry[50];   /**< type | number | type | number ... */
    slot_info_t slots_config[25];
    int32_t config_num = 0;
    char* measure_ptr = NULL;

    for (int32_t i = 0; i < argc; i++) {
        if (get_options_keyval(argv[i], &key, &val) != 0) {
            return ;
        }

        if (strcmp(key, "type") == 0) {
            type = val;
        }
        else if (strcmp(key, "slots") == 0) {
            config_num = split_str(val, ',', slots_arry);
            if ((config_num == -1) || ((config_num % 2) != 0)) {
                printf("slots options for format slot is invalid.\n");
                return ;
            }

            for (int32_t i = 0; i < (config_num / 2); i++) {
                slots_config[i].slot_mgr.slot_type = strtodigit(slots_arry[2 * i], NULL, 0);
                slots_config[i].slot_number = strtodigit(slots_arry[2 * i + 1], NULL, 0);
            }
        }
        else if (strcmp(key, "measure") == 0) {
            measure_ptr = val;
        }
        else {
            printf("invalid options: %s for [slimfs format] command.\n", argv[i]);
            return;
        }
    }

    if (type == NULL) {
        printf("please specifie the foramat type: data or slot\n");
        return;
    }

    uint32_t begin, end = 0;
    begin = slimfs_get_us_tick();
    if (strcmp(type, "data") == 0) {
        // data format
        slimfs_format_data();
    }
    else if (strcmp(type, "slot") == 0) {
        if (config_num == 0) {
            // slot format use default para
            slimfs_format_slot_default();
        }
        else {
            slimfs_format_slot(slots_config, config_num / 2);
        }
    }
    else {
        // do nothing
    }
    end = slimfs_get_us_tick();
    if (measure_ptr != NULL &&
        strcmp(measure_ptr, "on") == 0) {
        printf("%s cost time %d ms.\n", __func__, end - begin);
    }
    return;
}

/**
 * @brief query fs/slot/item information
 *
 * @param argc sub cmd args number
 * @param argv sub cmd args
 * -type=[queryType] slimfs/slot/item
 * -id=<itemId> if type is item, need this option
 * -name=<itemName> if type is item, need this option
 */
void slimfs_utl_query(int32_t argc, char* argv[])
{
    if (slimfs_init() != SLIMFS_RET_OK) {
        printf("slimfs init failed for %s.\n", __func__);
        return ;
    }

    char* key = NULL;
    char* val = NULL;
    char* type = NULL;

    slimfs_header_t header;
    slot_info_t slots_info[25];
    uint32_t slots_num = 0;
    slimfs_search_t search = {
        .id = SLIMFS_SEARCH_INVALID_TYPE,
    };
    slimfs_item_mgr_t item_mgr;

    int32_t ret = 0;

    for (int32_t i = 0; i < argc; i++) {
        if (get_options_keyval(argv[i], &key, &val) != 0) {
            return ;
        }

        if (strcmp(key, "type") == 0) {
            type = val;
        }
        else if (strcmp(key, "id") == 0) {
            search.search_type = SLIMFS_SEARCH_BY_ID;
            search.id = strtodigit(val, NULL, 0);
        }
        else if (strcmp(key, "name") == 0) {
            search.search_type = SLIMFS_SEARCH_BY_NAME;
            if (strlen(val) >= CONFIG_SLIMFS_ITEM_NAME_LEN) {
                search.search_type = SLIMFS_SEARCH_INVALID_TYPE;
            }
            else {
                strcpy((char *)search.name, val);
            }
        }
        else {
            printf("invalid options: %s for [slimfs query] command.\n", argv[i]);
            return;
        }
    }

    if (type == NULL) {
        printf("please specifie the query type: fs or slot or item\n");
        return;
    }

    if (strcmp(type, "fs") == 0) {
        ret = slimfs_header_query(&header);
        if (ret != 0) {
            printf("query slimfs header info failed.\n");
            return;
        }

        printf("query slimfs info: \n");
        dump_slimfs_info(&header, NULL);
    }
    else if (strcmp(type, "slot") == 0) {
        ret = slimfs_slots_query(slots_info, &slots_num);
        if (ret != 0) {
            printf("query slimfs slots info failed.\n");
            return;
        }

        printf("query slots info: \n");
        for (uint32_t i = 0; i < slots_num; i++) {
            printf("slot %d info: \n", i);
            dump_slots_info(&slots_info[i], "  --");
        }
    }
    else if (strcmp(type, "item") == 0) {
        if (search.id == SLIMFS_SEARCH_INVALID_TYPE) {
            printf("please specify item name or itemid.\n");
            return;
        }

        ret = slimfs_item_query(&search, &item_mgr);
        if (ret != 0) {
            printf("query slimfs item info failed.\n");
            return;
        }

        printf("query item info: \n");
        dump_item_mgr_info(&item_mgr, "  ");
    }
    else {
        // do nothing
        printf("invalid query type: %s.\n", type);
    }

    return;
}

/**
 * @brief utl crc control
 *
 * @param argc sub cmd args number
 * @param argv sub cmd args
 * -name=<itemName>
 * -id=<itemId>
 * -crc=[switch sta]    on/off
 */
void slimfs_utl_crc(int32_t argc, char* argv[])
{
    if (slimfs_init() != SLIMFS_RET_OK) {
        printf("slimfs init failed for %s.\n", __func__);
        return ;
    }

    char* key = NULL;
    char* val = NULL;
    char* sta = NULL;

    slimfs_search_t search = {
        .id = SLIMFS_SEARCH_INVALID_TYPE,
    };
    bool switch_sta = false;

    for (int32_t i = 0; i < argc; i++) {
        if (get_options_keyval(argv[i], &key, &val) != 0) {
            return ;
        }

        if (strcmp(key, "crc") == 0) {
            sta = val;
        }
        else if (strcmp(key, "id") == 0) {
            search.search_type = SLIMFS_SEARCH_BY_ID;
            search.id = strtodigit(val, NULL, 0);
        }
        else if (strcmp(key, "name") == 0) {
            search.search_type = SLIMFS_SEARCH_BY_NAME;
            if (strlen(val) >= CONFIG_SLIMFS_ITEM_NAME_LEN) {
                search.search_type = SLIMFS_SEARCH_INVALID_TYPE;
            }
            else {
                strcpy((char *)search.name, val);
            }
        }
        else {
            printf("invalid options: %s for [slimfs crc] command.\n", argv[i]);
            return;
        }
    }

    if (sta == NULL) {
        printf("-crc args is required.\n");
        return;
    }
    if (search.search_type == SLIMFS_SEARCH_INVALID_TYPE) {
        printf("-name or -id args is required.\n");
        return;
    }

    if (strcmp(sta, "on") == 0) {
        switch_sta = true;
    }
    else if (strcmp(sta, "off") == 0) {
        switch_sta = false;
    }
    else {
        printf("-crc args is invalid\n");
        return;
    }

    slimfs_item_crc_switch(&search, switch_sta);

    return;
}

void slimfs_utl_destroy(int32_t argc, char* argv[])
{
    char* key = NULL;
    char* val = NULL;

    int32_t blk = -1;
    int32_t offset = -1;

    for (int32_t i = 0; i < argc; i++) {
        if (get_options_keyval(argv[i], &key, &val) != 0) {
            return ;
        }

        if (strcmp(key, "blk") == 0) {
            blk = strtodigit(val, NULL, 0);
        }
        else if (strcmp(key, "offset") == 0) {
            offset = strtodigit(val, NULL, 0);
        }
        else {
            printf("invalid options: %s for [slimfs destroy] command.\n", argv[i]);
            return;
        }
    }

    if (blk != -1 && offset != -1) {
        slimfs_destroy_data(blk, offset);
        printf("destroy slimfs data with block: 0x%x(%d), offset: %d\n", blk, blk, offset);
    }
    else {
        printf("destroy data failed.\n");
    }

    return;
}

void slimfs_utl_showcache(int32_t argc, char* argv[])
{
    slimfs_show_cache();
}

void slimfs_utl_debug(int32_t argc, char* argv[])
{
    slimfs_show_debug();
}
/* sub command manager table */
static subCmd_mgr_t s_subCmd_mgr_tbl[] = {
    {"init"    , 0, slimfs_utl_init},
    {"dump"    , -1, slimfs_utl_dump},
    {"add"     , -1, slimfs_utl_add},
    {"del"     , -1, slimfs_utl_del},
    {"write"   , -1, slimfs_utl_write},
    {"read"    , -1, slimfs_utl_read},
    {"query"   , -1, slimfs_utl_query},
    {"format"  , -1, slimfs_utl_format},
    {"crc"     , -1, slimfs_utl_crc},
    {"deinit"  , 0,  slimfs_utl_deinit},
    {"destroy" , 2,  slimfs_utl_destroy},
    {"showcache", 0,  slimfs_utl_showcache},
    {"debug"   , 0,  slimfs_utl_debug},
    {NULL}
};

static void subCmd_help_msg(void)
{
    printf("Usage: slimfs [subCmd] [required args] <optional args>\n");
    printf("slimfs subcommand and args show as below: \n");

    printf("\n");
    printf("  init                          init slimfs for current os\n");

    printf("\n");
    printf("  deinit                        deinit slimfs for current os\n");

    printf("\n");
    printf("  dump                          dump slimfs item info and content\n");
    printf("  <optional args>\n");
    printf("    -nindexs=N_indexBitMap      bitmap mask, control which type slots can be dump\n");
    printf("    -slotindex=slotIndex        slot index, control which offset slots can be dump\n");
    printf("    -len=dumpLen                dump content len, normally use it with -nindexs and -slotindex args\n");

    printf("\n");
    printf("  add                           add item to slimfs\n");
    printf("  [required args]\n");
    printf("    -name=itemName              item name for add\n");
    printf("    -crcen=crcswitch            0 crc disable, 1 crc enable\n");
    printf("    -len=length                 item content length\n");
    printf("    -content=string             add item content, tools can only add as string, and its length is do not\n");
    printf("                                need keep same with -len, but it will allocate slot depends -len value\n");
    printf("  <optional args>\n");
    printf("    -id=itemId                  itemId, which include N_index and slot index, can quickly index the\n");
    printf("                                corresponding location of the slot\n");
    printf("    -measure=on                 measure add item time cost, default off\n");

    printf("\n");
    printf("  del                           delete the item from slimfs\n");
    printf("  <optional args>               -name and -id options just have to pick one option\n");
    printf("    -name=itemName              item name for delete\n");
    printf("    -id=itemId                  itemId\n");
    printf("    -measure=on                 measure delete item time cost, default off\n");

    printf("\n");
    printf("  write                         update the item content in slimfs\n");
    printf("  [required args]\n");
    printf("    -len=length                 item content length\n");
    printf("    -content=string             item content, string type\n");
    printf("  <optional args>               -name and -id options just have to pick one option\n");
    printf("    -name=itemName              item name for write\n");
    printf("    -id=itemId                  itemId\n");
    printf("    -measure=on                 measure update item time cost, default off\n");

    printf("\n");
    printf("  read                          read the item content in slimfs\n");
    printf("  [required args]\n");
    printf("    -len=length                 item content length\n");
    printf("  <optional args>               -name and -id options just have to pick one option\n");
    printf("    -name=itemName              item name for read\n");
    printf("    -id=itemId                  itemId\n");
    printf("    -type=showType              item content show type, options is: str/int/float/hex, default show as hex\n");
    printf("    -showlen=showLength         read item content show length, default is 32 bytes\n");
    printf("    -measure=on                 measure read item time cost, default off\n");

    printf("\n");
    printf("  query                         query slimfs info\n");
    printf("  [required args]\n");
    printf("    -type=type                  query slimfs info type, options is: fs/slot/item\n");
    printf("  <optional args>               only when -type=item, -name and -id options just have to pick one option\n");
    printf("    -name=itemName              item name for query\n");
    printf("    -id=itemId                  itemId\n");

    printf("\n");
    printf("  format                        format slimfs\n");
    printf("  [required args]\n");
    printf("    -type=type                  format slimfs type, options is: data/slots, format data only clear item\n");
    printf("                                in slot, do not change slot type layout; format slots will change slot type\n");
    printf("                                layout depends on default config or -slots args\n");
    printf("  <optional args>\n");
    printf("    -slots=formatType           indicates slot type and this type numbers show as: slotType,slotNumber...\n");
    printf("                                egs: -slots=32,16,64,8,128,8 max number for sepcifies types is 20 \n");

    printf("\n");
    printf("  crc                           crc switch control\n");
    printf("  [required args]\n");
    printf("    -crc=status                 crc verify switch status, options is: on/off\n");
    printf("  <optional args>               only when -type=item, -name and -id options just have to pick one option\n");
    printf("    -name=itemName              item name for crc control\n");
    printf("    -id=itemId                  itemId\n");

    printf("\n");
    printf("  destroy                       destroy mmc data, for checksum function test\n");
    printf("  [required args]\n");
    printf("    -blk=blkNumber              destroy mmc data block number\n");
    printf("    -offset=byteOffset          destroy mmc data offset within the block\n");

    printf("\n");
    printf("  showcache                     show item name & item Id cache info\n");

    printf("\n");
    printf("  debug                         show slimfs header and slot buffer debug msg\n");
    return;
}

int32_t slimfs_utility_main(int32_t argc, char* argv[])
{
    // parameter check
    if (argv[PARA_INDEX] == NULL ||
        strncmp(argv[PARA_INDEX], "?", strlen("?")) == 0 ||
        strncmp(argv[PARA_INDEX], "-h", strlen("-h")) == 0) {
            subCmd_help_msg();
            return 0;
    }

    // command parser
    int32_t i = -1;
    int32_t subcmd_parnum = argc - PARA_INDEX - 1;
    while (subcmd_parnum >= 0 &&
            s_subCmd_mgr_tbl[++i].sub_cmd != NULL) {
        if (strncmp(argv[PARA_INDEX], s_subCmd_mgr_tbl[i].sub_cmd, strlen(argv[PARA_INDEX])) == 0 &&
            strlen(argv[PARA_INDEX]) == strlen(s_subCmd_mgr_tbl[i].sub_cmd)) {
            if (subcmd_parnum == s_subCmd_mgr_tbl[i].par_num ||
                s_subCmd_mgr_tbl[i].par_num == -1) {
                s_subCmd_mgr_tbl[i].entry(subcmd_parnum, &argv[PARA_INDEX + 1]);
            }
            else {
                SLIMFS_PRINT(SLIMFS_LOG_ERR, "parameter number invalid for sub cmd: %s, expect: %d, actual: %d\n"
                             , s_subCmd_mgr_tbl[i].sub_cmd
                             , s_subCmd_mgr_tbl[i].par_num
                             , subcmd_parnum);
            }
            break;
        }
        else {
            // continue check subcmd
        }
    }

    if (i == -1 ||
        s_subCmd_mgr_tbl[i].sub_cmd == NULL) {
        SLIMFS_PRINT(SLIMFS_LOG_ERR, "do not support sub cmd: %s for slimfs utility\n", argv[PARA_INDEX]);
        return 0;
    }

    return 0;
}

#ifndef CONFIG_SLIMFS_TEST_WITH_PC
// register hook command
#if defined(WITH_LIB_CONSOLE)
int32_t slimfs_utility_main_wrapper(int32_t argc, const cmd_args* argv)
{
    char* args[10];
    for (int32_t i = 0; i < argc; i++) {
        args[i] = (char *)argv[i].str;
    }

    return slimfs_utility_main(argc, args);
}

STATIC_COMMAND_START
STATIC_COMMAND("slimfs", "slimfs utilitys. slimfs -h for more information",
               (console_cmd)slimfs_utility_main_wrapper)
STATIC_COMMAND_END(slimfs);
#endif

#if defined(CONFIG_CLI)
CLI_CMD("slimfs", "slimfs utilitys. slimfs -h for more information", slimfs_utility_main);
#endif /* end  CONFIG_CLI*/

#else  /* CONFIG_SLIMFS_TEST_WITH_PC */
#if defined(CONFIG_SLIMFS_TEST_CMD_MODE)
int32_t main(int32_t argc, char* argv[])
{
    return slimfs_utility_main(argc - 1, &argv[1]);
}

#else
int32_t main(int32_t argc, char* argv[])
{
    char buf[256] = "";
    int32_t ret = 0;
    char*   args[20];
    int32_t argn = 0;

    while (1) {
        printf("#slimfs ");
        ret = get_line(buf, sizeof(buf));
        if (ret != 0) {
            continue;
        }
        printf("%s\n", buf);

        argn = split_str(buf, ' ', args);
        ret = slimfs_utility_main(argn, args);
        if (ret != 0) {
            break;
        }
    }

    return 0;
}
#endif /* CONFIG_SLIMFS_TEST_CMD_MODE */
#endif /* end CONFIG_SLIMFS_TEST_WITH_PC */

/* end of file sdrv_slimfs_utility.c */