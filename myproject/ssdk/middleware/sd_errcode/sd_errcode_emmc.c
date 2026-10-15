/*
 * sd_errcode_emmc.c
 *
 * Copyright (c) 2022 SemiDrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 * set errcode to eMMC
 *
 * Revision History:
 * -----------------
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <debug.h>
#include <cmsis_os2.h>
#include "sd_errcode_emmc.h"
#include "sdrv_slimfs.h"

#define ERRCODE_EMMC_STOR_SIZE 16

#define ERRCODE_ITEM_INFO_NAME "ERRCODE_INFO"
static errcode_item_info_t *g_errcode_item_info = NULL;
static uint32_t g_errcode_info_item_id;

int errcode_emmc_init(void)
{
    int32_t ret = ERRCODE_OK;
    slimfs_search_t search = {0};
    slimfs_item_mgr_t itemMgr;
    int conter = 0;
    g_errcode_item_info = (errcode_item_info_t *)malloc(sizeof(errcode_item_info_t));
    memset(g_errcode_item_info, 0, sizeof(errcode_item_info_t));

    //Read errcode item info from slimFS
    search.search_type = SLIMFS_SEARCH_BY_NAME;
    memcpy(search.name, ERRCODE_ITEM_INFO_NAME, sizeof(ERRCODE_ITEM_INFO_NAME));
Retry:
    ret = slimfs_item_query(&search, &itemMgr);
    if (ret == 0) {
        g_errcode_info_item_id = itemMgr.id;
        ret = slimfs_item_read(&search, g_errcode_item_info, sizeof(errcode_item_info_t));
        if (ret < 0) {
            printf("%s : slimfs_item_read errcode %s error.\n", __func__, search.name);
            return ERRCODE_NG;
        }
    }
    else if (ret == SLIMFS_RET_NOT_FOUND) {
        printf("%s : slimfs_item_query errcode %s error, need add a new item.\n", __func__, search.name);

        //Write errcode item info to slimFS
        slimfs_item_info_t item = {0};
        item.item_mgr.id = CONFIG_INVALID_ITEM_ID;
        item.item_mgr.crc_en = 0;
        memcpy(item.item_mgr.name, ERRCODE_ITEM_INFO_NAME, sizeof(ERRCODE_ITEM_INFO_NAME));
        item.content = g_errcode_item_info;
        item.item_mgr.len = sizeof(errcode_item_info_t);

        ret = slimfs_item_add(&item);
        if (ret == 0) {
            g_errcode_info_item_id = item.item_mgr.id;
        }
        else {
            printf("%s : slimfs_item_add errcode %s error.\n", __func__, search.name);
            return ERRCODE_NG;
        }
    }
    else if (ret == SLIMFS_RET_INVALID_FS) {
        ERRCODE_DEBUG_PRINT("%s : slimfs is not inited, wait and retry...retry_counter = %d\n", __func__, conter);
        if (conter > 100) {
            printf("%s : slimfs init retry reach to max.\n", __func__);
            return ret;
        }
        conter++;
        osDelay(20);
        goto Retry;
    }
    else {
        printf("%s : slimfs_item_query errcode %s error, retry_counter = %d\n", __func__, search.name, conter);
        return ERRCODE_NG;
    }

    printf("%s : errcode emmc init done, retry_counter = %d\n", __func__, conter);
    return ret;
}

int errcode_emmc_deinit(void)
{
    if (g_errcode_item_info) {
        free(g_errcode_item_info);
        g_errcode_item_info = NULL;
    }

    return 0;
}

ERR_HANDLE errcode_set_to_emmc(int strid, uint8_t *data_buffer, uint16_t length)
{
    ERR_HANDLE handle = ERRCODE_OK;
    int32_t ret = 0;
    slimfs_search_t search;
    uint32_t tmp_errcode = 0;

    ERRCODE_DEBUG_PRINT("errcode_set_to_emmc start eMMC id = %d\n", strid);
    ERRCODE_DEBUG_PRINT("errcode_set_to_emmc length = %d\n", length);
    ERRCODE_DEBUG_PRINT("[");
    for (int i = 0; i < length; i++) {
        ERRCODE_DEBUG_PRINT(" %02x", data_buffer[i]);
    }
    ERRCODE_DEBUG_PRINT("]\n");

    //Update errcode item.
    memcpy(&tmp_errcode, data_buffer+4, 4);
    search.search_type = SLIMFS_SEARCH_BY_NAME;
    sprintf((char*)search.name, "%x", tmp_errcode);

    ret = slimfs_item_update(&search, data_buffer, length);
    if (ret == SLIMFS_RET_NOT_FOUND) {
        printf("%s : slimfs_item_update errcode %s not found, need add a new item.\n", __func__, search.name);
        if (g_errcode_item_info->item_num + 1 > CONFIG_ITEM_NUM_MAX) {
            printf("%s : errcode item num reach to MAX, need to extend storage space.\n", __func__);
            goto Label;
        }

        //Add a new errcode item.
        slimfs_item_info_t item = {0};
        item.item_mgr.id = CONFIG_INVALID_ITEM_ID;
        item.item_mgr.crc_en = 0;
        memcpy(item.item_mgr.name, search.name, sizeof(search.name));
        item.content = data_buffer;
        item.item_mgr.len = length;

        ret = slimfs_item_add(&item);
        if (ret == 0) {
            //Update errcode item info.
            errcode_item_t *tmp_item = &g_errcode_item_info->item[g_errcode_item_info->item_num];
            tmp_item->id = item.item_mgr.id;
            memcpy(tmp_item->name, item.item_mgr.name, sizeof(item.item_mgr.name));
            g_errcode_item_info->item_num++;

            search.search_type = SLIMFS_SEARCH_BY_ID;
            search.id = g_errcode_info_item_id;
            ret = slimfs_item_update(&search, g_errcode_item_info, sizeof(errcode_item_info_t));
            if (ret < 0) {
                printf("%s : slimfs_item_update errcode_info error.\n", __func__);
            }
        }
        else
        {
            printf("%s : slimfs_item_add errcode %s error.\n", __func__, search.name);
        }
    }

Label:

    return handle;
}

int errcode_dump_from_emmc(int strid, char* buf, int bufsize)
{
    int ret = ERRCODE_OK;
    bool dump_flag = true;
    int total_size = 0;
    slimfs_search_t search = {0};
    uint8_t data_buffer[ERRCODE_EMMC_STOR_SIZE] = {0};
    ERRCODE_DEBUG_PRINT("errcode_dump_from_emmc start eMMC id = %d\n", strid);

    if (strid != 2/*STR_ID_EMMC1*/) {
        printf("%s : Currently, Only support EMMC1!\n", __func__);
        goto Label;
    }

    printf("%s : Currently %d error codes are stored.\n", __func__, g_errcode_item_info->item_num);
    for (uint32_t i = 0; i < g_errcode_item_info->item_num; i++) {
        search.search_type = SLIMFS_SEARCH_BY_ID;
        search.id = g_errcode_item_info->item[i].id;
        memset(data_buffer, 0, ERRCODE_EMMC_STOR_SIZE);
        ret = slimfs_item_read(&search, data_buffer, ERRCODE_EMMC_STOR_SIZE);
        if (ret < 0) {
            printf("%s : slimfs_item_read errcode 0x%x error.\n", __func__, search.id);
            goto Label;
        }
        total_size += ERRCODE_EMMC_STOR_SIZE;

        if (buf && dump_flag) {
            int size = 0;
            size = (bufsize >= total_size) ? ERRCODE_EMMC_STOR_SIZE : (ERRCODE_EMMC_STOR_SIZE+bufsize-total_size);
            memcpy(buf, data_buffer, size);
            buf += size;
            if (total_size > bufsize) {
                dump_flag = false;
            }
        }

        printf("%08x [", i*16);
        for (int j = 0; j < ERRCODE_EMMC_STOR_SIZE; j++) {
            printf(" %02x", data_buffer[j]);
        }
        printf("]\n");
    }
Label:
    return total_size;
}

int errcode_clear_emmc(int strid)
{
    int ret = ERRCODE_OK;
    slimfs_search_t search = {0};

    ERRCODE_DEBUG_PRINT("errcode_clear_emmc start eMMC id = %d\n", strid);

    for (uint32_t i = 0; i < g_errcode_item_info->item_num; i++) {
        search.search_type = SLIMFS_SEARCH_BY_ID;
        search.id = g_errcode_item_info->item[i].id;
        ret = slimfs_item_del(&search);
        if (ret < 0) {
            printf("%s : slimfs_item_del errcode 0x%x error.\n", __func__, search.id);
        }
    }

    memset(g_errcode_item_info, 0, sizeof(errcode_item_info_t));

    //Update errcode item info.
    search.search_type = SLIMFS_SEARCH_BY_ID;
    search.id = g_errcode_info_item_id;
    ret = slimfs_item_update(&search, g_errcode_item_info, sizeof(errcode_item_info_t));
    if (ret < 0) {
        printf("%s : slimfs_item_update errcode_info error.\n", __func__);
    }

    return ret;
}
