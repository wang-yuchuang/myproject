#include <stdlib.h>
#include <string.h>
#include <CLI.h>
#include <cmsis_os2.h>
#include <stdio.h>
#include <debug.h>
#include <board.h>

#include "sdrv_property_list.h"

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define NULL_NAME " "
#define NULL_VAL " "
#define ZERO_VAL "0"

struct sys_property_client {
    char* val;
    char* name;
    int id;

    void *callback[PROP_CB_MAX];
    void *usr_data[PROP_CB_MAX];
    int cb_flag[PROP_CB_MAX];
    osEventFlagsId_t event;
};

typedef enum str_flag {
    STR_NAME = 0,
    STR_NAME_VAL = 1,
    STR_VAL = 2,
    STR_MAX,
} str_flag_enum;

typedef struct sys_property_client sys_property_client_t;

// 通过文件取出的，本域需要关注的property
static struct sys_property_value *g_property_table = NULL;
static int g_sys_property_count = 0;

static sys_property_client_t *g_sdrv_prop = NULL;
static int g_sdrv_prop_count = 0;
static osMutexId_t g_list_lock;
static char prop_sparator[] = "\n";

extern int prop_client_send_set_val(char* update_msg, int msg_len);

int sdrv_property_is_persist(char* name)
{
    return (strncmp(name, "cdm.persist", 11) == 0);
}

int is_sdrv_prop(char* name)
{
  return strncmp(name, "cdm.", 4) == 0;
}

int sdrv_check_name(char* name)
{
    if (name == NULL)
        return -1;
    // 留出\0
    if (strlen(name) > PROP_NAME_MAX - 1) {
        printf("name too long\n");
        return -1;
    }
    if(!is_sdrv_prop(name)) {
        printf("not support name\n");
        return -1;
    }

    return 0;
}

int sdrv_check_val(char* val)
{
    if (val == NULL)
        return -1;
    // 留出\0
    if (strlen(val) > PROP_VALUE_MAX - 1) {
        printf("value too long\n");
        return -1;
    }

    return 0;
}

int find_id_with_name(char* name)
{
    int ret = -1;
    for (int i = 0; i < g_sdrv_prop_count; i++) {
        if ( (strlen(g_sdrv_prop[i].name) == strlen(name))
                && (0 == strcmp(g_sdrv_prop[i].name, name)) ) {
            ret = g_sdrv_prop[i].id;
            break;
        }
    }
    return ret;
}

int sdrv_property_add_callback(int property_id, void* name, void* cb, void *data)
{
    int save_id = -1;

    if (cb == NULL)
        return -1;

    if (name) {
        if (sdrv_check_name(name))
            return -1;

        property_id = find_id_with_name(name);
    }

    if (0 > property_id || g_sdrv_prop_count < property_id) {
        return -1;
    }

    if (NULL == g_sdrv_prop) {
        return -1;
    }

    osMutexAcquire(g_list_lock, osWaitForever);
    for (int i = 0; i < PROP_CB_MAX; i++) {
        // 已经存在的内容
        if (g_sdrv_prop[property_id].callback[i] == cb) {
            save_id = -1;
            break;
        }
        // 找到第一个空闲的位置
        if (save_id == -1 && g_sdrv_prop[property_id].callback[i] == NULL) {
            save_id = i;
        }
    }
    if (save_id >= 0 && save_id < PROP_CB_MAX) {
        g_sdrv_prop[property_id].callback[save_id] = cb;
        g_sdrv_prop[property_id].usr_data[save_id] = data;
        g_sdrv_prop[property_id].cb_flag[save_id] = 0; // system_on_changed_cb
        if (name)
            g_sdrv_prop[property_id].cb_flag[save_id] = 1; // property_on_changed_cb
    }
    osMutexRelease(g_list_lock);

    if (save_id < 0 || save_id >= PROP_CB_MAX) {
        ssdk_printf(SSDK_ERR, "sdrv_property_add_callback [%d] add failed(list full)\n", property_id);
        return -1;
    }

    ssdk_printf(SSDK_ERR, "sdrv_property_add_callback [%d] add[%d]\n", property_id, save_id);
    return 0;
}

int sdrv_property_list_wait_timeout(int property_id, void* name, void* expect, uint32_t ms)
{
    char* tmp_expect = NULL;
    int ret = 0;
    int expect_len = 0;
    uint32_t start = 0;
    uint32_t end = 0;

    if (sdrv_check_val(expect))
        return -1;

    if (name) {
        if (sdrv_check_name(name))
            return -1;

        property_id = find_id_with_name(name);
    }

    if (0 > property_id || g_sdrv_prop_count < property_id) {
        return -1;
    }

    if (NULL == g_sdrv_prop) {
        return -1;
    }

    osMutexAcquire(g_list_lock, osWaitForever);
    // 如果就是属性就是期待值，直接返回
    if (strcmp(g_sdrv_prop[property_id].val, (char*)expect) == 0) {
        osMutexRelease(g_list_lock);
        return 0;
    }

    expect_len = strlen((char*)expect);
    tmp_expect = malloc(expect_len + 1);
    if (NULL == tmp_expect) {
        ssdk_printf(SSDK_ERR, "tmp_expect malloc failed!!\n");
        osMutexRelease(g_list_lock);
        return -1;
    }
    memset(tmp_expect, 0x00, expect_len + 1);
    memcpy(tmp_expect, expect, strlen((char*)expect));

    if (g_sdrv_prop[property_id].event == NULL)
        g_sdrv_prop[property_id].event = osEventFlagsNew(NULL);
    osMutexRelease(g_list_lock);

wait_again:
    start = current_time();
    ret = osEventFlagsWait(g_sdrv_prop[property_id].event, PROPERTY_EVENT_COMPLETE, osFlagsWaitAll, ms);
    if (ret < 0) {
        ssdk_printf(SSDK_ERR, "wait property(%d) @condition(%s) timeout \n", property_id, (char*)expect);
    }
    else {
        end = current_time();
        osMutexAcquire(g_list_lock, osWaitForever);
        if (strcmp(g_sdrv_prop[property_id].val, (char*)expect) != 0) {
            osMutexRelease(g_list_lock);
            ms -= (end - start);
            goto wait_again;
        }
        osMutexRelease(g_list_lock);
    }

    free(tmp_expect);

    return ret;
}

void sdrv_property_onchanged(int property_id, char* name, char* old, char* val)
{
    int i_old = 0;
    int i_val = 0;
    if (0 > property_id || g_sdrv_prop_count < property_id) {
        return;
    }

    if (NULL == g_sdrv_prop) {
        return;
    }

    if (0 == strcmp(old, val)) {
        // do nothing
    }

    for (int i = 0; i < PROP_CB_MAX; i++) {
        if (g_sdrv_prop[property_id].callback[i]) {
            if (g_sdrv_prop[property_id].cb_flag[i]) {
                property_on_changed_cb tmp_cb = (property_on_changed_cb)g_sdrv_prop[property_id].callback[i];
                tmp_cb(property_id, name, old, val, g_sdrv_prop[property_id].usr_data[i]);
            }
            else {
                i_old = atoi(old);
                i_val = atoi(val);
                system_on_changed_cb tmp_cb = (system_on_changed_cb)g_sdrv_prop[property_id].callback[i];
                tmp_cb(property_id, i_old, i_val, g_sdrv_prop[property_id].usr_data[i]);
            }
        }
    }

    if (g_sdrv_prop[property_id].event) {
        osEventFlagsSet(g_sdrv_prop[property_id].event, PROPERTY_EVENT_COMPLETE);
    }
}

// 检查收到的property项是否是本域需要关注的，不关注的id设置为-1
int chdeck_domain_set_fllower(int id)
{
    int ret_id = -1;
    for (int i = 0; i < g_sys_property_count; i++) {
        if (id == g_property_table[i].id) {
            ret_id = id;
            break;
        }
    }
    return ret_id;
}

int sdrv_property_item_check(sys_property_client_t *property_table, int num)
{
    // printf("1 [%d][%s][%s]\n", property_table->id, property_table->name, property_table->val);

    sys_property_client_t *item = NULL;
    int i = 0;

    for (i = 0; i < num; i++) {
        if (1 == num)
            item = property_table;
        else
            item = &(property_table[i]);

        if (0 == item->id) {
            item->id = g_sdrv_prop_count;
        }

        item->id = chdeck_domain_set_fllower(item->id);

        if (NULL == item->name) {
            // printf("NULL_NAME: %d\n", sizeof(NULL_NAME));
            item->name = (char*)malloc(sizeof(NULL_NAME));
            memset(item->name, 0x00, sizeof(NULL_NAME));
            memcpy(item->name, NULL_NAME, strlen(NULL_NAME));
        }

        if (NULL == item->val) {
            // printf("ZERO_VAL: %d\n", strlen(ZERO_VAL));
            item->val = (char*)malloc((sizeof(ZERO_VAL)));
            memset(item->val, 0x00, sizeof(ZERO_VAL));
            memcpy(item->val, ZERO_VAL, strlen(ZERO_VAL));
        }
    }

    // printf("2 [%d][%s][%s]\n", property_table->id, property_table->name, property_table->val);

    return 0;
}

// list维护的函数
// 部分追踪时，应预留id的位置
int sdrv_property_list_add(sys_property_client_t *property_table, int num)
{
    int ret = 0;
    sys_property_client_t *property_area = NULL;
    int num_bak = 0;

    osMutexAcquire(g_list_lock, osWaitForever);
    sdrv_property_item_check(property_table, num);
    // printf("list add [%d] name[%s] val[%s]\n", property_table->id, property_table->name, property_table->val);

    if (property_table && num) {
        // add
        if (g_sdrv_prop && g_sdrv_prop_count) {
            num_bak = g_sdrv_prop_count;
            g_sdrv_prop_count += num;

            property_area = malloc(sizeof(sys_property_client_t) * g_sdrv_prop_count);
            if (!property_area) {
                printf("add not enough memory for properties %d\n", g_sdrv_prop_count);
                ret = -1;
                goto fail_and_free;
            }

            memcpy(property_area, g_sdrv_prop, sizeof(sys_property_client_t) * num_bak);
            memcpy(property_area + num_bak, property_table, sizeof(sys_property_client_t) * num);
            // 如果频繁的malloc/free会产生内存碎片化
            free(g_sdrv_prop);
            g_sdrv_prop = property_area;
        }
        // init
        else {
            property_area = malloc(sizeof(sys_property_client_t) * num);
            if (!property_area) {
                printf("init not enough memory for properties %d\n", num);
                ret = -1;
                goto fail_and_free;
            }

            memcpy(property_area, property_table, sizeof(sys_property_client_t) * num);
            g_sdrv_prop = property_area;
            g_sdrv_prop_count = num;
        }
    }

fail_and_free:
    osMutexRelease(g_list_lock);
    return ret;
}

int sdrv_property_list_getlen(void)
{
    return g_sdrv_prop_count;
}

int sdrv_property_client_set_val(int property_id, void* name, void* val)
{
    char* update_msg = NULL;
    int update_msg_len = 0;
    int cpoy_idx = 0;
    int* prop_id = NULL;

    if (NULL == val) return -1;
    if (property_id >= g_sdrv_prop_count) return -1;

    update_msg_len = sizeof(int) + PROP_VALUE_MAX + PROP_NAME_MAX + strlen(prop_sparator);
    update_msg = malloc(update_msg_len);
    memset(update_msg, 0x00, update_msg_len);

    prop_id = (int*)update_msg;
    *prop_id = property_id;
    cpoy_idx = sizeof(int);

    if (NULL != name) {
        memcpy(update_msg + cpoy_idx, name, strlen(name));
        cpoy_idx += strlen(name);
    }

    memcpy(update_msg + cpoy_idx, prop_sparator, strlen(prop_sparator));
    cpoy_idx += strlen(prop_sparator);

    memcpy(update_msg + cpoy_idx, val, strlen(val));
    cpoy_idx += strlen(val);

    prop_client_send_set_val(update_msg, cpoy_idx);

    free(update_msg);

    return 0;
}

int sdrv_property_list_set_val(int property_id, void* val)
{
    char* csave = NULL;
    sys_property_client_t *property = NULL;

    if (NULL == val) {
        return -1;
    }

    if (property_id < 0 || property_id >= g_sdrv_prop_count) {
        return -1;
    }

    if (NULL == g_sdrv_prop) {
        return -1;
    }

    osMutexAcquire(g_list_lock, osWaitForever);

    property = &g_sdrv_prop[property_id];

    csave = (char*)val;

    // for wait/callback
    sdrv_property_onchanged(property_id, property->name, property->val, csave);

    if (NULL != property->val) {
        // printf("old [%d] val[%s]\n", property_id, property->val);
        free(property->val);
        property->val = NULL;
    }
    property->val = malloc(strlen(csave) + 1);
    memset(property->val, 0x00, strlen(csave) + 1);
    memcpy(property->val, csave, strlen(csave));
    // printf("set [%d] val[%s]\n", property_id, property->val);

    osMutexRelease(g_list_lock);

    return 0;
}

int sdrv_property_list_get_val(int property_id, void* val)
{
    sys_property_client_t *property = NULL;
    char* cret = NULL;

    if (NULL == val) {
        return -1;
    }

    if (property_id < 0 || property_id >= g_sdrv_prop_count) {
        return -1;
    }

    if (NULL == g_sdrv_prop) {
        return -1;
    }

    osMutexAcquire(g_list_lock, osWaitForever);

    property = &g_sdrv_prop[property_id];

    cret = (char*)val;
    memcpy(cret, property->val, strlen(property->val));
    // printf("return char val[%s]\n", cret);

    osMutexRelease(g_list_lock);

    return 0;
}

// id list
char* get_list_str(int* list_len)
{
    char* ret = NULL;

    osMutexAcquire(g_list_lock, osWaitForever);

    ret = (char*)malloc(g_sdrv_prop_count);
    if (NULL == ret) {
        printf("get_list_str malloc failed[%d]\n", g_sdrv_prop_count);
    }

    memset(ret, 0x00, g_sdrv_prop_count);
    for (int i = 0; i < g_sdrv_prop_count; i++) {
        if (-1 != g_sdrv_prop[i].id) {
            ret[i] = 0xFF;
        }
    }

    *list_len = g_sdrv_prop_count;

    osMutexRelease(g_list_lock);

    return ret;
}

// name
// name + val
int sdrv_property_str2list(int flag, char* str, int len)
{
    char* name = NULL;
    char* val = NULL;
    const char d[2] = "\n";
    sys_property_client_t* tmp_prop = NULL;
    char* token = NULL;
    int i = 0;

    // mutex_lock(&g_list_lock);
    // name or name + val
    if (STR_VAL != flag) {
        token = strtok(str, d);
        while(NULL != token) {
            // name
            name = malloc(strlen(token) + 1);
            memset(name, 0x00, strlen(token) + 1);
            memcpy(name, token, strlen(token));

            // val
            if (STR_NAME_VAL == flag) {
                token = strtok(NULL, d);
                if (NULL != token) {
                    val = (char*)malloc(strlen(token) + 1);
                    memset(val, 0x00, strlen(token) + 1);
                    memcpy(val, token, strlen(token));
                }
                else {
                    printf("can not find \\n, error line\n");
                    break;
                }
            }
            else {
                val = (char*)malloc((sizeof(ZERO_VAL)));
                memset(val, 0x00, sizeof(ZERO_VAL));
                memcpy(val, ZERO_VAL, strlen(ZERO_VAL));
            }

            tmp_prop = malloc(sizeof(sys_property_client_t));
            memset(tmp_prop, 0x00, sizeof(sys_property_client_t));

            tmp_prop->name = name;
            tmp_prop->val = val;
            tmp_prop->id = g_sdrv_prop_count;

            // printf("list[%d] name[%s] val[%s]\n", tmp_prop->id, tmp_prop->name, tmp_prop->val);
            sdrv_property_list_add(tmp_prop, 1);
            // prop已经拷贝到数组中保存，这里释放掉
            // name和val不能释放，数组中需要使用，由数组deinit的时候释放
            free(tmp_prop);
            token = strtok(NULL, d);
        }
    }
    else {
        token = strtok(str, d);
        while(NULL != token) {
            if (0 != strcmp(NULL_VAL, token)) {
                sdrv_property_list_set_val(i, token);
            }
            i++;
            token = strtok(NULL, d);
        }
    }

    // mutex_unlock(&g_list_lock);
    return 0;
}

void sdrv_property_list_init(struct sys_property_value *property_table, int num)
{
    g_property_table = property_table;
    g_sys_property_count = num;

    g_list_lock = osMutexNew(NULL);
}


int show_property(int argc, char* argv[])
{
    for(int i = 0; i < g_sdrv_prop_count; i++) {
        if (-1 == g_sdrv_prop[i].id) continue;
        printf("[%d][%s][%s]\n", g_sdrv_prop[i].id, g_sdrv_prop[i].name, g_sdrv_prop[i].val);
    }

    return 0;
}

CLI_CMD("prop_show", "show all property", show_property);
