/*
 * property.c
 *
 * Copyright (c) 2019 SemiDrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 * property library
 *
 * Revision History:
 * -----------------
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <string.h>
#include <cmsis_os2.h>
#include <board.h>
#include <debug.h>
#include <rpmsg_service.h>
#include <rpmsg_echo.h>
#include <rpmsg_channel.h>
#include <driver/timer.h>
#include <driver/kunlun/syscnt/sdrv_syscnt_drv.h>
#include <CLI_type.h>
#include <rpc/dcf_rpc.h>
#include <rpc/rpmsg_common.h>
#include <property_cfg.h>
#include "sd_property.h"
#include "sdrv_property_store.h"
#if defined(CONFIG_SLIMFS)
#include "sdrv_property_emmc.h"
#endif
#include "sdrv_property_list.h"
#include "sdrv_property_trans.h"

static struct sys_property_value *_properties = &ss_properties[0];

#define PROPERTY_WAIT_DEFAULT_TIME (5000)

#if (SUPPORT_STR_MODE && SUPPORT_EXTRA_GPR)
#define PROPERTY_STR (1)
#else
#define PROPERTY_STR (0)
#endif

#if PROPERTY_STR
#include <sdrv_extra_gpr.h>
static struct sys_prop_store prop_store_table[] = {
    {.index = DMP_ID_CLUSTER_STATUS, .key = SDRV_EXTRA_GPR_NAME_PROP_CLUSTER, .need_store = true},
    {.index = DMP_ID_ANDROID_BOOT_STATUS, .key = SDRV_EXTRA_GPR_NAME_PROP_ANDROID, .need_store = true},
};
#endif

#define MAX_QUEUE_COUNT 20
static osMessageQueueId_t g_property_set_queue = NULL;
struct property_set_data {
    int id;
    char *name;
    char *val;
};
// static osMutexId_t g_queue_lock = osMutexNew(g_queue_lock);

int sdrv_property_set_queue_init(void)
{
    // osMutexAcquire(g_queue_lock, osWaitForever);
    if (g_property_set_queue) {
        // osMutexRelease(g_queue_lock);
        return 0;
    }

    g_property_set_queue =
        osMessageQueueNew(MAX_QUEUE_COUNT, sizeof(struct property_set_data), NULL);
    if (!g_property_set_queue) {
        printf("property failed to create property_set_queue\n");
        return -1;
        // osMutexRelease(g_queue_lock);
    }

    // osMutexRelease(g_queue_lock);
    return 0;
}

int system_property_get(int property_id, int* val)
{
    int ret = 0;
    char c[100] = {0};

    ret = sdrv_property_list_get_val(property_id, c);
    *val = atoi(c);

    return ret;
}

int system_property_set(int property_id, int val)
{
    struct property_set_data pack;
    char c[100] = {0};
    int val_len = 0;

    int ret = sdrv_property_set_queue_init();
    if (ret)
        return -1;

    sprintf(c, "%d", val);
    val_len = strlen(c);

    pack.id = property_id;
    pack.name = NULL;
    pack.val = (char*)malloc(val_len + 1);  // free in sdrv_property_set_process()
    if (NULL == pack.val) {
        printf("%s pack name malloc failed!!\n", __func__);
    }
    memset(pack.val, 0x00, val_len + 1);
    memcpy(pack.val, c, val_len);

    osMessageQueuePut(g_property_set_queue, &pack, 0, osWaitForever);
    // return sdrv_property_list_set_val(property_id, NULL, c, 0);

    return 0;
}

#if PROPERTY_STR
void save_property_to_system(int index, uint32_t val)
{
    int i;
    int len = sizeof(prop_store_table)/sizeof(struct sys_prop_store);
    struct sys_prop_store *ps = NULL;

    for (i = 0; i < len; i++) {
        ps = &prop_store_table[i];
        if (ps->index == index && ps->need_store == true) {
            sdrv_extra_gpr_set(ps->key, val);
            break;
        }
    }

    return;
}

static void get_property_from_system(void)
{
    int i;
    int len = sizeof(prop_store_table)/sizeof(struct sys_prop_store);
    struct sys_prop_store *ps = NULL;
    int index;
    uint32_t val;

    for (i = 0; i < len; i++) {
        ps = &prop_store_table[i];
        if (sdrv_extra_gpr_get(ps->key, &val)) {
            continue;
        }
        index = ps->index;
        system_property_set(index, val);
    }

    return;
}
#endif

int sdrv_property_wait_timeout(int property_id, void* name, void* expect, uint32_t ms)
{
    return sdrv_property_list_wait_timeout(property_id, name, expect, ms);
}

int system_property_wait_timeout(int property_id, int expect, uint32_t ms)
{
    char c[100] = {0};

    sprintf(c, "%d", expect);
    return sdrv_property_wait_timeout(property_id, NULL, c, ms);
}

int system_property_wait_condition(int property_id, int expect)
{
    return system_property_wait_timeout(property_id, expect, PROPERTY_WAIT_DEFAULT_TIME);
}

int sdrv_property_observe(int property_id, void* name, property_on_changed_cb cb, void *data)
{
    return sdrv_property_add_callback(property_id, name, cb, data);
}

int system_property_observe(int property_id, system_on_changed_cb cb, void *data)
{
    return sdrv_property_add_callback(property_id, NULL, cb, data);
}

static void sdrv_property_set_process(void *arg)
{
    int ret = 0;
    struct property_set_data pack;
    pack.id = -1;
    pack.name = NULL;
    pack.val = NULL;

    while(1) {
        ret = osMessageQueueGet(g_property_set_queue, &pack, NULL, osWaitForever);
        if (ret == osOK) {
            sdrv_property_list_set_val(pack.id, pack.name, pack.val, 0);
            // free
            pack.id = -1;
            if (pack.name) {
                free(pack.name);
                pack.name = NULL;
            }
            if (pack.val) {
                free(pack.val);
                pack.val = NULL;
            }
        }
    }

    osThreadExit();
}

void sdrv_property_set_init(void)
{
    int ret = sdrv_property_set_queue_init();
    if (ret)
        return;

    osThreadAttr_t attr;
    memset(&attr, 0x00, sizeof(attr));
    attr.name = "sdrv_property_set_process";
    attr.priority = osPriorityNormal;
    attr.stack_size = 2048;

    osThreadId_t thr = osThreadNew(sdrv_property_set_process, NULL, &attr);
    if (NULL == thr) {
        ssdk_printf(SSDK_ERR, "property failed to create sdrv_property_set_init!!\n\n");
    }
}

static osMutexId_t property_lock;

int sdrv_property_init(struct sys_property_value *property_table, int num)
{
    int ret = 0;

    if (NULL== property_table) {
        property_table = _properties;
        num = board_property_get_array_size();
    }

    property_lock = osMutexNew(NULL);
    osMutexAcquire(property_lock, osWaitForever);

    sdrv_property_list_init();

    // ram base property init
    sdrv_property_list_add(property_table, num);
    // persistent property init
    sdrv_property_store_init();
#if defined(CONFIG_SLIMFS)
    sdrv_property_emmc_init();
#endif

    sdrv_property_set_init();

#if PROPERTY_STR
    get_property_from_system();
#endif

    sdrv_property_trans_init();

    osMutexRelease(property_lock);
    ssdk_printf(SSDK_ERR, "%s started\n", __func__);

    return ret;
}

int sdrv_property_set(int property_id, void* name, void* val)
{
    struct property_set_data pack;
    int len = 0;
    char* cname = (char*)name;
    char* cval = (char*)val;

    if (sdrv_check_val(val))
        return -1;

    // set时有可能通过id追加/修改，这时name可以是空
    if (name != NULL) {
        if (sdrv_check_name(name))
            return -1;

        // 禁止追加/修改只读的持久存储的属性
        if (sdrv_property_is_persist(name))
            return -1;
    }

    int ret = sdrv_property_set_queue_init();
    if (ret)
        return -1;

    pack.id = property_id;
    pack.name = NULL;
    if (cname) {
        len = strlen(cname);
        pack.name = (char*)malloc(len + 1);  // free in sdrv_property_set_process()
        memset(pack.name, 0x00, len + 1);
        memcpy(pack.name, cname, len);
    }
    pack.val = NULL;
    if (cval) {
        len = strlen(cval);
        pack.val = (char*)malloc(len + 1);  // free in sdrv_property_set_process()
        memset(pack.val, 0x00, len + 1);
        memcpy(pack.val, cval, len);
    }

    osMessageQueuePut(g_property_set_queue, &pack, 0, osWaitForever);
    // return sdrv_property_list_set_val(property_id, name, val, 0);

    return 0;
}

int sdrv_property_get(int property_id, void* name, void* val)
{
    if (sdrv_check_val(val))
        return -1;

    if (property_id < 0) {
        if (sdrv_check_name(name))
            return -1;

        property_id = find_id_with_name((char*)name);
        if (property_id < 0) {
            return -1;
        }
    }
    return sdrv_property_list_get_val(property_id, val);
}

static int property_set(int argc, char *argv[])
{
    int i = 0;
    int property_id = -1;
    char tmp_name[100] = {0};
    char val[100] = {0};
    char* name = NULL;

    if (argc < 2) {
        printf("setprop name val\n");
        return 0;
    }

    memcpy(tmp_name, argv[i], strlen(argv[i]));
    name = tmp_name;

    i++;
    memcpy(val, argv[i], strlen(argv[i]));

    sdrv_property_set(property_id, name, val);

    return 0;
}

static int property_get(int argc, char *argv[])
{
    int i = 0;
    int property_id = -1;
    char val[100] = {0};
    char tmp_name[100] = {0};
    char* name = NULL;

    if (argc < 1) {
        printf("getprop name\n");
        return 0;
    }

    memcpy(tmp_name, argv[i], strlen(argv[i]));
    name = tmp_name;

    if (0 > sdrv_property_get(property_id, name, val)) {
        ssdk_printf(SSDK_ERR, "get failed!! property_id or name not find\n");
    }
    else {
        if (0 > property_id) {
            printf("[%s] : [%s]\n", name, val);
        }
        else {
            printf("[%d] : [%s]\n", property_id, val);
        }

    }

    return 0;
}

static void wait_prop_system_cb(int pid, int old_val, int new_val, void* param)
{
    printf("waitprop [%d][%d]\n", pid, new_val);
}

static void wait_prop_property_cb(int pid, void* name, void* old_val, void* new_val, void* param)
{
    printf("waitprop [%d][%s][%s]\n", pid, (char*)name, (char*)new_val);
}

// waitprop name val timeout
static int property_wait(int argc, char *argv[])
{
    int i = 0;
    char name[100] = {0};
    char val[100] = {0};
    int timeout = 0;
    int prop_id = -1;
    int ret = 0;

    if (argc < 3) {
        printf("waitprop name val timeout\n");
        return 0;
    }

    memcpy(name, argv[i], strlen(argv[i]));

    i++;
    memcpy(val, argv[i], strlen(argv[i]));

    i++;
    timeout = atoi(argv[i]);

    // printf("waitprop [%s][%s][%d]\n", name, val, timeout);

    prop_id = find_id_with_name(name);
    if (prop_id < 0) {
        printf("name not exist!\n");
    }

    system_property_observe(prop_id, wait_prop_system_cb, NULL);
    sdrv_property_observe(-1, name, wait_prop_property_cb, NULL);

#if 1
    ret = sdrv_property_wait_timeout(prop_id, name, val, timeout);
    if (ret < 0) {
        printf("wait [%s] for [%s] failed!!\n", name, val);
    }
#else
    // 旧接口测试
    int tmp_val = atoi(val);
    ret = system_property_wait_timeout(prop_id, tmp_val, timeout);
    if (ret < 0) {
        printf("wait [%s] for [%s] failed!!\n", name, val);
    }
#endif

    return 0;
}

CLI_CMD("setprop", "set property", property_set);
CLI_CMD("getprop", "get property", property_get);
CLI_CMD("waitprop", "wait property", property_wait);

