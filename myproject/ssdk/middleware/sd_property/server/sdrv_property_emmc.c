#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <debug.h>
#include <board.h>
#include <cmsis_os2.h>
#include <CLI_type.h>

#include "sdrv_slimfs.h"
#include "sdrv_property_trans.h"
#include "sdrv_property_emmc.h"

#define PROPERTY_ITEM_LIST_MAX 20
#define PROPERTY_ITEMLIST_NAME "PROPERTY_INFO"

#define INFO_SIZE (sizeof(property_item_t) * PROPERTY_ITEM_LIST_MAX)

#define SDRV_FOUNDED 0
#define SDRV_NOT_FOUND 1
#define SDRV_FS_NAME_LEN 12
#define SDRV_FS_NAME "%02d_%s"
#define RW_PERSIST_PROPERTY "cdm.rw.persist."

// item_list
typedef struct {
    int id;
    uint8_t  name[CONFIG_SLIMFS_ITEM_NAME_LEN];
} property_item_t;

// item info
typedef struct {
    uint32_t id;
    property_item_t list[PROPERTY_ITEM_LIST_MAX];
} property_item_info_t;

static osMutexId_t g_list_lock;
static property_item_info_t g_prop_info;
static int g_prop_emmc_init_flag = -1;

#define EMMC_MAX_QUEUE_COUNT 20
static osMessageQueueId_t g_emmc_set_queue = NULL;
// static osMutexId_t g_queue_lock = osMutexNew(g_queue_lock);

int sdrv_property_is_emmc(char* name)
{
    return (strncmp(name, RW_PERSIST_PROPERTY, strlen(RW_PERSIST_PROPERTY)) == 0);
}

int sdrv_property_is_exist(char* name)
{
    int ret = 0;
    slimfs_search_t search = {0};
    property_store_info_t search_prop = {0};

    osMutexAcquire(g_list_lock, osWaitForever);
    for (int i = 0; i < PROPERTY_ITEM_LIST_MAX; i++) {
        if (-1 == g_prop_info.list[i].id)
            continue;

        memset(&search_prop, 0x00, sizeof(property_store_info_t));
        search.search_type = SLIMFS_SEARCH_BY_ID;
        search.id = g_prop_info.list[i].id;
        ret = slimfs_item_read(&search, &search_prop, sizeof(property_store_info_t));
        if (ret == 0) {
            if (0 == strcmp(search_prop.name, name)) {
                osMutexRelease(g_list_lock);
                return 0;
            }
        }
        else {
            ssdk_printf(SSDK_ERR, "[%d][%d][%s] get failed!\n", i, g_prop_info.list[i].id, g_prop_info.list[i].name);
        }
    }
    osMutexRelease(g_list_lock);

    return -1;
}

int drv_property_emmc_set_queue_init(void)
{
    // osMutexAcquire(g_queue_lock, osWaitForever);
    if (g_emmc_set_queue) {
        // osMutexRelease(g_queue_lock);
        return 0;
    }

    g_emmc_set_queue =
        osMessageQueueNew(EMMC_MAX_QUEUE_COUNT, sizeof(property_set_info_t), NULL);
    if (!g_emmc_set_queue) {
        ssdk_printf(SSDK_ERR, "property failed to create property_set_queue\n");
        return -1;
        // osMutexRelease(g_queue_lock);
    }

    // osMutexRelease(g_queue_lock);
    return 0;
}

// fs_name[15] : ID_NAME(ID[2], NAME[12])
// name只能放下12个字符，但是实际prop的name可能达到32个字符
// 截取12个字符放入，可能有重名的情况，这里追加id区分
int find_in_slimfs(char* prop_name)
{
    int ret = 0;
    slimfs_search_t search = {0};
    char fs_name[SDRV_FS_NAME_LEN + 1] = {0};
    int name_len = strlen(prop_name) - strlen(RW_PERSIST_PROPERTY);
    property_store_info_t find_prop = {0};

    memcpy(fs_name, prop_name + strlen(RW_PERSIST_PROPERTY), (SDRV_FS_NAME_LEN > name_len ? name_len : SDRV_FS_NAME_LEN));
    // ssdk_printf(SSDK_ERR, "fs_name[%s]\n", fs_name);

    osMutexAcquire(g_list_lock, osWaitForever);
    for (int i = 0; i < PROPERTY_ITEM_LIST_MAX; i++) {
        // list中的空项目
        if (-1 == g_prop_info.list[i].id)
            continue;
        // name匹配
        if (strstr((char*)g_prop_info.list[i].name, fs_name) == NULL)
            continue;

        memset(&find_prop, 0x00, sizeof(property_store_info_t));
        search.search_type = SLIMFS_SEARCH_BY_ID;
        search.id = g_prop_info.list[i].id;
        ret = slimfs_item_read(&search, &find_prop, sizeof(property_store_info_t));
        if (ret == 0) {
            // 对比从slimfs中取出的项目的全名和输入的全名是否一致
            if (0 == strcmp(find_prop.name, prop_name)) {
                osMutexRelease(g_list_lock);
                return i;
            }
            else {
                continue;
            }
        }
        else {
            continue;
        }
    }
    osMutexRelease(g_list_lock);

    return -1;
}

// id为存在item list中的第几个
int sdrv_property_emmc_set(int id, property_store_info_t* item)
{
    // printf

    if (item == NULL)
        return -1;

    int ret = 0;
    int item_id = id;
    slimfs_search_t search = {0};
    char fs_name[SDRV_FS_NAME_LEN + 1] = {0};
    char fs_id_name[CONFIG_SLIMFS_ITEM_NAME_LEN] = {0};
    int name_len = strlen(item->name) - strlen(RW_PERSIST_PROPERTY);

    if (item_id < 0) {
        // 追加或修改，需要判断emmc中是否有存在的name
        item_id = find_in_slimfs(item->name);

        if (item_id < 0) {
            // 追加
            osMutexAcquire(g_list_lock, osWaitForever);
            // 寻找list中空项目
            for (int i = 0; i < PROPERTY_ITEM_LIST_MAX; i++) {
                if (-1 == g_prop_info.list[i].id) {
                    item_id = i;
                    break;
                }
            }
            // list存满了，丢弃
            if (item_id < 0) {
                ssdk_printf(SSDK_ERR, "%s property emmc list full\n", __func__);
                osMutexRelease(g_list_lock);
                return -1;
            }
            // 合成存入fs的名字
            memcpy(fs_name, item->name + strlen(RW_PERSIST_PROPERTY), (SDRV_FS_NAME_LEN > name_len ? name_len : SDRV_FS_NAME_LEN));
            // ssdk_printf(SSDK_ERR, "fs_name[%s]\n", fs_name);
            sprintf(fs_id_name, SDRV_FS_NAME, item_id, fs_name);
            // ssdk_printf(SSDK_ERR, "fs_id_name[%s]\n", fs_id_name);
            // 存入slimfs
            slimfs_item_info_t fs_item = {0};
            fs_item.item_mgr.id = CONFIG_INVALID_ITEM_ID;
            fs_item.item_mgr.crc_en = 0;
            memcpy(fs_item.item_mgr.name, fs_id_name, sizeof(fs_id_name));
            fs_item.content = (void*)item;
            fs_item.item_mgr.len = sizeof(property_store_info_t);

            ret = slimfs_item_add(&fs_item);
            if (ret == 0) {
                // 更新list信息
                g_prop_info.list[item_id].id = fs_item.item_mgr.id;
                memcpy(g_prop_info.list[item_id].name, fs_id_name, sizeof(fs_id_name));
                // list信息存入slimfs
                search.search_type = SLIMFS_SEARCH_BY_ID;
                search.id = g_prop_info.id;
                ret = slimfs_item_update(&search, g_prop_info.list, INFO_SIZE);
                if (0 != ret) {
                    ssdk_printf(SSDK_ERR, "%s update failed\n", __func__);
                }
            }
            else {
                ssdk_printf(SSDK_ERR, "%s : slimfs_item_add property %s error.\n", __func__, search.name);
            }
            goto out;
        }
        // 跳出，走修改
    }

    // 修改
    osMutexAcquire(g_list_lock, osWaitForever);
    search.search_type = SLIMFS_SEARCH_BY_ID;
    search.id = g_prop_info.list[item_id].id;
    ret = slimfs_item_update(&search, item, sizeof(property_store_info_t));
    if (0 != ret) {
        ssdk_printf(SSDK_ERR, "%s update failed\n", __func__);
    }

out:
    osMutexRelease(g_list_lock);
    sdrv_property_list_set_val(-1, item->name, item->val, 1);

    return 0;
}

int sdrv_property_emmc_async_set(property_set_info_t* item)
{
    int ret = drv_property_emmc_set_queue_init();
    if (ret)
        return -1;

    osMessageQueuePut(g_emmc_set_queue, item, 0, osWaitForever);

    return 0;
}

static void sdrv_property_emmc_set_process(void *arg)
{
    int ret = 0;
    property_set_info_t pack = {0};

    while(1) {
        ret = osMessageQueueGet(g_emmc_set_queue, &pack, NULL, osWaitForever);
        if (ret == osOK) {
            // 会有部分可改写的存储在res中，用于初始化定义(第一次开机时值生效，后续看存储中的值)
            if (pack.origin == 2 && sdrv_property_is_exist(pack.item.name) == 0) {
                continue;
            }
            sdrv_property_emmc_set(-1, &(pack.item));
        }
    }

    osThreadExit();
}

void sdrv_property_emmc_set_init(void)
{
    int ret = drv_property_emmc_set_queue_init();
    if (ret)
        return;

    osThreadAttr_t attr;
    memset(&attr, 0x00, sizeof(attr));
    attr.name = "sdrv_property_emmc_set_process";
    attr.priority = osPriorityNormal;
    attr.stack_size = 2048;

    osThreadId_t thr = osThreadNew(sdrv_property_emmc_set_process, NULL, &attr);
    if (NULL == thr) {
        ssdk_printf(SSDK_ERR, "property failed to create sdrv_property_emmc_set_process!!\n\n");
    }
}

static int sdrv_property_emmc_dump(int argc, char *argv[])
{
    int ret = 0;
    slimfs_search_t search = {0};
    property_store_info_t search_prop = {0};

    osMutexAcquire(g_list_lock, osWaitForever);
    for (int i = 0; i < PROPERTY_ITEM_LIST_MAX; i++) {
        if (-1 == g_prop_info.list[i].id)
            continue;

        memset(&search_prop, 0x00, sizeof(property_store_info_t));
        search.search_type = SLIMFS_SEARCH_BY_ID;
        search.id = g_prop_info.list[i].id;
        ret = slimfs_item_read(&search, &search_prop, sizeof(property_store_info_t));
        if (ret == 0) {
            printf("[%d][%s][%s]\n", i, search_prop.name, search_prop.val);
        }
        else {
            ssdk_printf(SSDK_ERR, "[%d][%d][%s] get failed!\n", i, g_prop_info.list[i].id, g_prop_info.list[i].name);
        }
    }
    osMutexRelease(g_list_lock);

    return 0;
}

int sdrv_property_emmc_status(void)
{
    return g_prop_emmc_init_flag;
}

// 仅在开机时使用，向server的property list中追加emmc读取出来的项目
int sdrv_property_emmc_add_2_list()
{
    int ret = 0;
    slimfs_search_t search = {0};
    property_store_info_t find_prop = {0};

    // osMutexAcquire(g_list_lock, osWaitForever);
    for (int i = 0; i < PROPERTY_ITEM_LIST_MAX; i++) {
        if (-1 == g_prop_info.list[i].id)
            continue;

        memset(&find_prop, 0x00, sizeof(property_store_info_t));
        search.search_type = SLIMFS_SEARCH_BY_ID;
        search.id = g_prop_info.list[i].id;
        ret = slimfs_item_read(&search, &find_prop, sizeof(property_store_info_t));
        if (ret == 0) {
            while (sdrv_property_trans_get_syncing() > 0) {
                ssdk_printf(SSDK_ERR, "someone syncing!\n");
                osDelay(100);  // 100ms
            }
            sdrv_property_list_set_val(-1, find_prop.name, find_prop.val, 1);
        }
        else {
            ssdk_printf(SSDK_ERR, "[%d][%d][%s] get failed!\n", i, g_prop_info.list[i].id, g_prop_info.list[i].name);
        }
    }
    // osMutexRelease(g_list_lock);

    return 0;
}

static void sdrv_property_emmc_process(void *arg)
{
    int ret = 0;
    slimfs_search_t search = {0};
    slimfs_item_mgr_t itemMgr;

    printf("property emmc init start!\n");

    osMutexAcquire(g_list_lock, osWaitForever);
    memset(&g_prop_info, 0x00, sizeof(property_item_info_t));
    for (int i = 0; i < PROPERTY_ITEM_LIST_MAX; i++) {
        g_prop_info.list[i].id = -1; // 没有数据
    }
    g_prop_info.id = -1;
    osMutexRelease(g_list_lock);

    search.search_type = SLIMFS_SEARCH_BY_NAME;
    memcpy(search.name, PROPERTY_ITEMLIST_NAME, sizeof(PROPERTY_ITEMLIST_NAME));

wait_slimfs_ready:
    osMutexAcquire(g_list_lock, osWaitForever);
    ret = slimfs_item_query(&search, &itemMgr);
    if (ret == 0) {
        g_prop_info.id = itemMgr.id;
        ret = slimfs_item_read(&search, g_prop_info.list, INFO_SIZE);
        if (ret < 0) {
            ssdk_printf(SSDK_ERR, "%s : slimfs_item_read property %s error.\n", __func__, search.name);
            goto exit;
        }
    }
    else if (ret == SLIMFS_RET_NOT_FOUND) {
        ssdk_printf(SSDK_ERR, "slimfs_item_query not found property item, now add item.\n");

        slimfs_item_info_t item = {0};
        item.item_mgr.id = CONFIG_INVALID_ITEM_ID;
        item.item_mgr.crc_en = 0;
        memcpy(item.item_mgr.name, PROPERTY_ITEMLIST_NAME, sizeof(PROPERTY_ITEMLIST_NAME));
        item.content = g_prop_info.list;
        item.item_mgr.len = INFO_SIZE;

        ret = slimfs_item_add(&item);
        if (ret == 0) {
            g_prop_info.id = item.item_mgr.id;
        }
        else {
            ssdk_printf(SSDK_ERR, "%s : slimfs_item_add property %s error.\n", __func__, search.name);
            goto exit;
        }
    }
    else if (ret == SLIMFS_RET_INVALID_FS) {
        osMutexRelease(g_list_lock);
        ssdk_printf(SSDK_ERR, "%s : slimfs not ready.\n", __func__);
        osDelay(100);  // 100ms
        // To do retry count = 50?
        goto wait_slimfs_ready;
    }
    else {
        ssdk_printf(SSDK_ERR, "%s : slimfs_item_query property %s error.\n", __func__, search.name);
        goto exit;
    }

    sdrv_property_emmc_add_2_list();
    sdrv_property_emmc_set_init();

    g_prop_emmc_init_flag = 1;
    printf("property emmc init done!\n");

exit:
    osMutexRelease(g_list_lock);
    osThreadExit();
}

// 使用线程初始化，因为property启动较早，此时slimfs可能还未初始化
void sdrv_property_emmc_init(void)
{
    g_list_lock = osMutexNew(NULL);

    osThreadAttr_t attr;
    memset(&attr, 0x00, sizeof(attr));
    attr.name = "sdrv_property_emmc_process";
    attr.priority = osPriorityNormal;
    attr.stack_size = 2048;

    osThreadId_t thr = osThreadNew(sdrv_property_emmc_process, NULL, &attr);
    if (NULL == thr) {
        ssdk_printf(SSDK_ERR, "create emmc init thread failed!!\n");
    }
}

CLI_CMD("prop_emmc_dump", "show all emmc property", sdrv_property_emmc_dump);