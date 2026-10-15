#include <debug.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <debug.h>

#include "res_loader.h"
#include "param.h"
#include "sdrv_property_store.h"
#include "sdrv_property_list.h"
#include "sdrv_property_trans.h"

#define PROP_MAX 1
const char *PROP_PATH[PROP_MAX] = {"early_app/property/default.prop"};
static property_res_t property_res[PROP_MAX] = {0};

int prop_res_load(void)
{
    uint32_t res_loaded = 0;

    // 校验信息
    // recv_head->crc = crc32(0, dev->recv_msg, (recv_head->len + MSG_HEAD_SIZE));
wait_res_ready:
    for(int i = 0; i < PROP_MAX; i++) {
        if (property_res[i].property_size > 0)
            continue;  // some one failed?
        property_res[i].property_size = res_size(PROP_PATH[i]);
        if (property_res[i].property_size <= 0) {
            ssdk_printf(SSDK_ERR, "%s : res not ready.\n", __func__);
            osDelay(100);  // 100ms
            // To do retry count = 50?
            goto wait_res_ready;
        }
        property_res[i].property_path = PROP_PATH[i];
        property_res[i].get_offset = 0;
        res_loaded++;
    }

    return res_loaded;
}

void prop_res_get(void* res_ptr, void* buf, int size, int* get_size)
{
    int ret = 0;
    property_res_t* res = NULL;

    if (NULL == res_ptr || NULL == buf || NULL == get_size) {
        ssdk_printf(SSDK_ERR, "Have NULL ptr!!\n");
        return;
    }
    res = (property_res_t*)res_ptr;

    if (res->get_offset >= res->property_size) {
        ssdk_printf(SSDK_ERR, "read resource finished\n");
        *get_size = 0;
        return;
    }

    ret = res_load(res->property_path, buf, size, res->get_offset);
    if (0 > ret) {
        ssdk_printf(SSDK_ERR, "res_load failed, ret[%d] path[%s] size[%d] offset[%d]\n",
                ret, res->property_path, res->property_size, res->get_offset);
        *get_size = 0;
    }

    res->get_offset += ret;
    *get_size = ret;
    ssdk_printf(SSDK_ERR, "buf_size[%d], res_size[%d], get_size[%d]\n", size, res->property_size, ret);
}

void prop_res_parse(void* buf, int size)
{
    ssdk_printf(SSDK_ERR, "prop_res_parse get\n");

    const char d[2] = "\n";
    const char dd[2] = "=";
    char* name = NULL;
    char* val = NULL;
    // struct sys_property_value* tmp_prop = NULL;
    char* token = NULL;

    token = strtok(buf, dd);
    while(NULL != token) {
        name = malloc(strlen(token) + 1);
        memset(name, 0x00, strlen(token) + 1);
        memcpy(name, token, strlen(token));

        token = strtok(NULL, d);
        if (NULL != token) {
            val = malloc(strlen(token) + 1);
            memset(val, 0x00, strlen(token) + 1);
            memcpy(val, token, strlen(token));
        }
        else {
            ssdk_printf(SSDK_ERR, "can not find \\n, error line\n");
            break;
        }

// res加载改为在线程中加载，res启动时间不确定，有可能在AP1/AP2取完list后加载完成
// 这里将res读出来的property改为，通过update的方式加入list
#if 0
        // ssdk_printf(SSDK_ERR, "i[%d], name[%s] val[%s]\n", sdrv_property_list_getlen(), name, val);
        tmp_prop = malloc(sizeof(struct sys_property_value));
        memset(tmp_prop, 0x00, sizeof(struct sys_property_value));
        tmp_prop->val = val;
        tmp_prop->name = name;
        tmp_prop->id = sdrv_property_list_getlen();
        sdrv_property_list_add(tmp_prop, 1);
        // prop已经拷贝到数组中保存，这里释放掉
        // name和val不能释放，数组中需要使用，由数组deinit的时候释放
        free(tmp_prop);
#endif
        while (sdrv_property_trans_get_syncing() > 0) {
            ssdk_printf(SSDK_ERR, "someone syncing!\n");
            osDelay(100);  // 100ms
        }
        sdrv_property_list_set_val(-1, name, val, 2);
        if (NULL != name)
            free(name);
        if (NULL != val)
            free(val);

        token = strtok(NULL, dd);
    }
}

static void sdrv_property_store_process(void *arg)
{
    void* input_buffer = NULL;
    int size = 0;

    printf("property store init start!\n");

    prop_res_load();

    for(int i = 0; i < PROP_MAX; i++) {
        if (0 < property_res[i].property_size) {
            size = ROUNDUP(property_res[i].property_size, 32);
            input_buffer = memalign(32, size);
            if(NULL == input_buffer) {
                ssdk_printf(SSDK_ERR, "ERROR: input_buffer is NULL\n");
                continue;
            }

            memset(input_buffer, 0x00, size);
            int ret_size = 0;
            prop_res_get(&property_res[i], input_buffer, size, &ret_size);
            ssdk_printf(SSDK_ERR, "file:\n%s", (char*)input_buffer);

            prop_res_parse(input_buffer, size);
        }
    }

    printf("property store init done!\n");
    osThreadExit();
}

void sdrv_property_store_init(void)
{
    osThreadAttr_t attr;
    memset(&attr, 0x00, sizeof(attr));
    attr.name = "sdrv_property_store_process";
    attr.priority = osPriorityNormal;
    attr.stack_size = 2048;

    osThreadId_t thr = osThreadNew(sdrv_property_store_process, NULL, &attr);
    if (NULL == thr) {
        ssdk_printf(SSDK_ERR, "create store init thread failed!!\n");
    }
}
