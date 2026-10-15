#include <stdlib.h>
#include <string.h>
#include <debug.h>
#include <CLI_type.h>
#include <board.h>

#include "sdrv_property_list.h"
#if defined(CONFIG_SLIMFS)
#include "sdrv_property_emmc.h"
#endif

#define NULL_NAME " "
#define ZERO_VAL "0"
#define MALLOC_SIZE 500
static char prop_sparator[] = "\n";
static char null_val[] = " ";

#define PROPERTY_IN_CALLBACK_BIT        (0x1000)
#define PROPERTY_WAIT_STATUS_BIT        (0x8000)

#if (SUPPORT_STR_MODE && SUPPORT_EXTRA_GPR)
#define PROPERTY_STR (1)
#else
#define PROPERTY_STR (0)
#endif

typedef enum str_flag {
    STR_NAME = 0,
    STR_NAME_VAL = 1,
    STR_VAL = 2,
    STR_MAX,
} str_flag_enum;

static osMutexId_t g_list_lock;
static struct sys_property_value *g_sdrv_prop = NULL;
static int g_sdrv_prop_count = 0;

extern int prop_update_fllower(int property_id, char* update_msg, int msg_len);
extern int prop_send_item_add(char* update_msg, int msg_len);

int sdrv_property_is_persist(char* name)
{
    return (strncmp(name, "cdm.persist", 11) == 0);
}

int sdrv_property_is_ro(char* name)
{
    return (strncmp(name, "cdm.ro", 6) == 0);
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

// 追加/删除/查找/修改

int sdrv_property_item_check(struct sys_property_value *property_table, int num)
{
    // ssdk_printf(SSDK_ERR, "1 [%d][%s][%s]\n", property_table->id, property_table->name, property_table->val);

    struct sys_property_value *item = NULL;
    for (int i = 0; i < num; i++) {
        if (1 == num)
            item = property_table;
        else
            item = &(property_table[i]);

        if (0 == item->id) {
            item->id = g_sdrv_prop_count;
        }

        if (NULL == item->name) {
            // ssdk_printf(SSDK_ERR, "NULL_NAME: %d\n", sizeof(NULL_NAME));
            item->name = (char*)malloc((sizeof(NULL_NAME)));
            memset(item->name, 0x00, sizeof(NULL_NAME));
            memcpy(item->name, NULL_NAME, strlen(NULL_NAME));
        }

        if (NULL == item->val) {
            // ssdk_printf(SSDK_ERR, "ZERO_VAL: %d\n", strlen(ZERO_VAL));
            item->val = (char*)malloc((sizeof(ZERO_VAL)));
            memset(item->val, 0x00, sizeof(ZERO_VAL));
            memcpy(item->val, ZERO_VAL, strlen(ZERO_VAL));
        }

        // DP_CPU_MAX
        for(int j = 0; j < 10; j++) {
            item->fllower[j] = -1;
        }
    }

    // ssdk_printf(SSDK_ERR, "2 [%d][%s][%s]\n", property_table->id, property_table->name, property_table->val);

    return 0;
}

// list维护的函数
int sdrv_property_list_add(struct sys_property_value *property_table, int num)
{
    int ret = 0;
    struct sys_property_value *property_area = NULL;
    int num_bak = 0;

    osMutexAcquire(g_list_lock, osWaitForever);
    sdrv_property_item_check(property_table, num);

    if (property_table && num) {
        // add
        if (g_sdrv_prop && g_sdrv_prop_count) {
            num_bak = g_sdrv_prop_count;
            g_sdrv_prop_count += num;

            property_area = malloc(sizeof(struct sys_property_value) * g_sdrv_prop_count);
            if (!property_area) {
                ssdk_printf(SSDK_ERR, "not enough memory for properties %d\n", g_sdrv_prop_count);
                // ret = ERR_NO_MEMORY;
                ret = -5;
                goto fail_and_free;
            }

            memcpy(property_area, g_sdrv_prop, sizeof(struct sys_property_value) * num_bak);
            memcpy(property_area + num_bak, property_table, sizeof(struct sys_property_value) * num);
            // 如果频繁的malloc/free会产生内存碎片化
            free(g_sdrv_prop);
            g_sdrv_prop = property_area;
        }
        // init
        else {
            property_area = malloc(sizeof(struct sys_property_value) * num);
            if (!property_area) {
                ssdk_printf(SSDK_ERR, "not enough memory for properties %d\n", num);
                // ret = ERR_NO_MEMORY;
                ret = -5;
                goto fail_and_free;
            }

            memcpy(property_area, property_table, sizeof(struct sys_property_value) * num);
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

#if PROPERTY_STR
extern void save_property_to_system(int index, uint32_t val);
#endif

// id < 0 -> list add
// id >= 0 && id < g_sdrv_prop_count list set
int sdrv_property_list_set_val(int property_id, void* name, void* val, int store)
{
    char* csave = NULL;
    struct sys_property_value *property = NULL;
    char* update_msg = NULL;
    int update_msg_len = 0;
    int ret = 0;
    int process_id = property_id;
    int cpoy_idx = 0;
#if PROPERTY_STR
    int ival = 0;
#endif

    if (NULL == val) return -1;
    if (NULL == g_sdrv_prop) return -1;
    if (process_id >= g_sdrv_prop_count) return -1;

    if (NULL != name) {
#if defined(CONFIG_SLIMFS)
        // property_list 更新时，先检查是否是emmc存储的，如果是，先由emmc处理，然后转回给property_list
        if (sdrv_property_is_emmc(name) && (store != 1)) {
            printf("emmc property [%s][%s]\n", (char*)name, (char*)val);

            property_set_info_t info = {0};
            info.origin = store; // 0:user 1:emmc 2:res
            csave = (char*)name;
            memcpy(info.item.name, csave, strlen(csave));
            csave = (char*)val;
            memcpy(info.item.val, csave, strlen(csave));

            sdrv_property_emmc_async_set(&info);
            return 0;
        }
#endif

        // have same name?
        process_id = find_id_with_name(name);
    }

    // list add new item
    if (process_id < 0) {
        if (NULL == name) return -1;

        property = malloc(sizeof(struct sys_property_value));
        memset(property, 0x00, sizeof(struct sys_property_value));

        csave = (char*)name;
        property->name = malloc(strlen(csave) + 1);
        memset(property->name, 0x00, strlen(csave) + 1);
        memcpy(property->name, csave, strlen(csave));

        csave = (char*)val;
        property->val = malloc(strlen(csave) + 1);
        memset(property->val, 0x00, strlen(csave) + 1);
        memcpy(property->val, csave, strlen(csave));

        property->id = g_sdrv_prop_count;

        sdrv_property_list_add(property, 1);
        ret = property->id;

        update_msg_len = sizeof(int) + PROP_VALUE_MAX + PROP_NAME_MAX + strlen(prop_sparator);
        update_msg = malloc(update_msg_len);
        memset(update_msg, 0x00, update_msg_len);
        int* prop_id = (int*)update_msg;
        *prop_id = property->id;
        cpoy_idx = sizeof(int);
        memcpy(update_msg + cpoy_idx, property->name, strlen(property->name));
        cpoy_idx += strlen(property->name);
        memcpy(update_msg + cpoy_idx, prop_sparator, strlen(prop_sparator));
        cpoy_idx += strlen(prop_sparator);
        memcpy(update_msg + cpoy_idx, property->val, strlen(property->val));
        cpoy_idx += strlen(property->val);
        prop_send_item_add(update_msg, cpoy_idx);

        free(update_msg);
        free(property);
        // prop已经拷贝到数组中保存，这里释放掉
        // name和val不能释放，数组中需要使用，由数组deinit的时候释放
        return ret;
    }

    // list update item val
    {
        osMutexAcquire(g_list_lock, osWaitForever);

        property = &g_sdrv_prop[process_id];

        if (sdrv_property_is_ro(property->name)
         || sdrv_property_is_persist(property->name)) {
            printf("can't write ro/persist [%s]\n", property->name);
            osMutexRelease(g_list_lock);
            return 0;
        }

        csave = (char*)val;

        // 值未变更，直接返回
        if (strcmp(property->val, csave) == 0) {
            osMutexRelease(g_list_lock);
            return 0;
        }

        // for wait/callback
        sdrv_property_onchanged(process_id, property->name, property->val, csave);

        if (NULL != property->val) {
            // ssdk_printf(SSDK_ERR, "old [%d] val[%s]\n", process_id, property->val);
            free(property->val);
            property->val = NULL;
        }
        property->val = malloc(strlen(csave) + 1);
        memset(property->val, 0x00, strlen(csave) + 1);
        memcpy(property->val, csave, strlen(csave));
        // ssdk_printf(SSDK_ERR, "set [%d] val[%s]\n", process_id, property->val);

#if PROPERTY_STR
        ival = atoi(csave);
        save_property_to_system(process_id, ival);
#endif

        update_msg_len = sizeof(int) + PROP_VALUE_MAX;
        update_msg = malloc(update_msg_len);
        memset(update_msg, 0x00, update_msg_len);
        int* prop_id = (int*)update_msg;
        *prop_id = process_id;
        cpoy_idx = sizeof(int);
        memcpy(update_msg + cpoy_idx, property->val, strlen(csave));
        cpoy_idx += strlen(property->val);

        osMutexRelease(g_list_lock);

        // send update msg
        prop_update_fllower(process_id, update_msg, cpoy_idx);
        free(update_msg);
    }

    return 0;
}

int sdrv_property_list_get_val(int property_id, void* val)
{
    struct sys_property_value *property = NULL;
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
    // ssdk_printf(SSDK_ERR, "return char [%d]val[%s]\n", property_id, cret);

    osMutexRelease(g_list_lock);
    return 0;
}

char* __prop_strncpy(char* dst, char* src, int src_len, int* dst_mem_len)
{
    char* ret_str = dst;
    int dst_len = 0;

    if (NULL == src || NULL == dst_mem_len) return NULL;

    if (NULL == ret_str) {
        ret_str = (char*)malloc(MALLOC_SIZE);
        memset(ret_str, 0x00, MALLOC_SIZE);
        if (NULL == ret_str) {
            ssdk_printf(SSDK_ERR, "[%s] malloc failed[1]!\n", __func__);
            return NULL;
        }
        *dst_mem_len = MALLOC_SIZE;
    }

    dst_len = strlen(ret_str);
    if (dst_len + src_len >= *dst_mem_len) {
        if (dst_len + src_len >= (2 * (*dst_mem_len))) {
            ssdk_printf(SSDK_ERR, "dst + src >= 2mem!!\n");
            return NULL;
        }
        char* tmp_str = (char*)malloc((*dst_mem_len) * 2);
        if (NULL == tmp_str) {
            ssdk_printf(SSDK_ERR, "[%s] malloc failed[2]!\n", __func__);
            return NULL;
        }
        *dst_mem_len *= 2;
        memset(tmp_str, 0x00, *dst_mem_len);
        strncpy(tmp_str, ret_str, dst_len);
        free(ret_str);
        ret_str = tmp_str;
    }
    strncpy(ret_str + dst_len, src, src_len);

    return ret_str;
}

int set_property_fllower(int property_id, int flag, int cpu_id);
// name
// name + val
// val
char* get_list_str(int flag, char* id_list, int id_list_len, int cpu_id)
{
    int mem_len = 0;
    char* ret = NULL;
    char* ret_tmp = NULL;

    if (NULL == g_sdrv_prop) return NULL;
    if (STR_MAX <= flag) return NULL;
    if (STR_VAL == flag && NULL == id_list) return NULL;
    if (STR_VAL == flag && id_list_len > g_sdrv_prop_count) return NULL;

    osMutexAcquire(g_list_lock, osWaitForever);

    // name or name + val
    if (STR_VAL != flag) {
        for (int i = 0; i < g_sdrv_prop_count; i++) {
            // name
            ret_tmp = __prop_strncpy(ret, g_sdrv_prop[i].name, strlen(g_sdrv_prop[i].name), &mem_len);
            if (NULL != ret_tmp) ret = ret_tmp;
            // sparator
            ret_tmp = __prop_strncpy(ret, prop_sparator, strlen(prop_sparator), &mem_len);
            if (NULL != ret_tmp) ret = ret_tmp;
            if (STR_NAME_VAL == flag) {
                // val
                ret_tmp = __prop_strncpy(ret, g_sdrv_prop[i].val, strlen(g_sdrv_prop[i].val), &mem_len);
                if (NULL != ret_tmp) ret = ret_tmp;
                // sparator
                ret_tmp = __prop_strncpy(ret, prop_sparator, strlen(prop_sparator), &mem_len);
                if (NULL != ret_tmp) ret = ret_tmp;

                set_property_fllower(i, 1, cpu_id);
            }
        }
    }
    // val
    else {
        for (int i = 0; i < id_list_len; i++) {
            if (0xFF == id_list[i]) {
                ret_tmp = __prop_strncpy(ret, g_sdrv_prop[i].val, strlen(g_sdrv_prop[i].val), &mem_len);
                if (NULL != ret_tmp) ret = ret_tmp;

                set_property_fllower(i, 1, cpu_id);
            }
            else {
                // 占位
                ret_tmp = __prop_strncpy(ret, null_val, strlen(null_val), &mem_len);
                if (NULL != ret_tmp) ret = ret_tmp;
            }
            // sparator
            ret_tmp = __prop_strncpy(ret, prop_sparator, strlen(prop_sparator), &mem_len);
            if (NULL != ret_tmp) ret = ret_tmp;
        }
    }

    osMutexRelease(g_list_lock);
    return ret;
}

// 使用时循环检测使用，直到返回值为-1
int get_property_fllower(int property_id, int* last)
{
    int ret = -1;
    // DP_CPU_MAX
    if (10 < *last || 0 > *last) return -1;

    osMutexAcquire(g_list_lock, osWaitForever);
    ret = g_sdrv_prop[property_id].fllower[*last];
    osMutexRelease(g_list_lock);
    (*last)++;

    return ret;
}

int set_property_fllower(int property_id, int flag, int cpu_id)
{
    int ret = -1;

    // add fllower  // DP_CPU_MAX
    for (int i = 0; i < 10; i++) {
        if (cpu_id == g_sdrv_prop[property_id].fllower[i]) {
            ret = 1;
            break;
        }
        if (-1 == g_sdrv_prop[property_id].fllower[i]) {
            g_sdrv_prop[property_id].fllower[i] = cpu_id;
            ret = 0;
            break;
        }
    }

    return ret;
}


// 防止频繁拷贝大片的内存，是否可以追加一个独立的函数，缓存单个追加的方法


void sdrv_property_list_init(void)
{
    g_list_lock = osMutexNew(NULL);
}


static int show_property(int argc, char *argv[])
{
    for(int i = 0; i < g_sdrv_prop_count; i++) {
        printf("[%d][%s][%s]", g_sdrv_prop[i].id, g_sdrv_prop[i].name, g_sdrv_prop[i].val);
        printf("[");
        for(int j = 0; j < 10; j++) {
            printf("%d ", g_sdrv_prop[i].fllower[j]);
        }
        printf("]\n");
    }

    return 0;
}

CLI_CMD("prop_show", "show all property", show_property);
