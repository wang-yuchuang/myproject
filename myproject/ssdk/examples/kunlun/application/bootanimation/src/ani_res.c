/**
 * @file ani_res.c
 *
 * Copyright (c) 2022 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: sdrv boot animation application.
 *
 * Revision History:
 * -----------------
 */
#include <string.h>
#include <malloc.h>
#include <FreeRTOS.h>
#include <heap_ext.h>
#include <res_loader.h>
#include "ani_def.h"
#include "ani_cfg.h"
#include "launcher.h"
#include "animation.h"
#include "app_log.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "animation"

#define RES_IDX_ANIMATION (0)
#define RES_IDX_LOGO      (1)
#define RES_IDX_MAX       (2)
#define RES_LOAD_SIZE     (2048*1024)

typedef struct _ani_res_t {
    const char* path;
    char* data;
    uint32_t offset;
    uint32_t size;
} ani_res_t;

static osMutexId_t mjpeg_mutex = NULL;
static ani_res_t ani_res[RES_IDX_MAX];
static int curr_pos;

extern osEventFlagsId_t res_loader_ready;

static int ani_res_loader_init(ani_task_token_t* task)
{
    APP_CRIT("");
    if (task) {
        memset(ani_res, 0, sizeof(ani_res));
        ani_res[RES_IDX_ANIMATION].path = ANI_RES_PATH_BA;
        ani_res[RES_IDX_ANIMATION].size = res_size(ani_res[RES_IDX_ANIMATION].path);
        ani_res[RES_IDX_ANIMATION].data = ExtMallocAligned(ani_res[RES_IDX_ANIMATION].size, 32);
        ani_res[RES_IDX_ANIMATION].offset = 0;
        if (!ani_res[RES_IDX_ANIMATION].data) {
            APP_ERR("ExtMallocAligned data fail.");
            return -1;
        }
        memset(ani_res[RES_IDX_ANIMATION].data, 0, ani_res[RES_IDX_ANIMATION].size);

#if CONFIG_LOOP_LOGO
        ani_res[RES_IDX_LOGO].path = ANI_RES_PATH_BA_LOOP;
        ani_res[RES_IDX_LOGO].size = res_size(ani_res[RES_IDX_LOGO].path);
        ani_res[RES_IDX_LOGO].data = ExtMallocAligned(ani_res[RES_IDX_LOGO].size, 32);
        ani_res[RES_IDX_LOGO].offset = 0;
        if (!ani_res[RES_IDX_LOGO].data) {
            APP_ERR("ExtMallocAligned data fail.");
            return -1;
        }
        memset(ani_res[RES_IDX_LOGO].data, 0, ani_res[RES_IDX_LOGO].size);
#endif

        mjpeg_mutex = osMutexNew(NULL);
        osEventFlagsSet(res_loader_ready, 1);

        return 0;
    }

    return -1;
}

static void ani_res_loader_entry(ani_task_token_t* task)
{
    APP_CRIT("");
    if (task && task->support_res >= 0 && task->support_res < ANI_RES_CNT) {
        int res = task->support_res;
        osStatus_t sts;
        bool loaded;
        while (task->running) {
            loaded = false;
            sts = osMutexAcquire(mjpeg_mutex, 100);
            if (sts == osOK) {
                for (int i = 0; i < RES_IDX_MAX; i++) {
                    if (!ani_res[i].path || ani_res[i].offset >= ani_res[i].size) {
                        continue;
                    }
                    else {
                        uint32_t offset = ani_res[i].offset;
                        int len = res_load(ani_res[i].path, &ani_res[i].data[offset], RES_LOAD_SIZE, offset);
                        if (len > 0)
                            ani_res[i].offset += len;
                        loaded = true;
                        break;
                    }
                }
                osMutexRelease(mjpeg_mutex);
                if (!loaded)
                    osDelay(10);
            }
        }
    }
}

static void ani_res_loader_exit(ani_task_token_t* task)
{
    APP_CRIT("");
    if (task) {
        osEventFlagsClear(res_loader_ready, 1);

        for (int i = 0; i < RES_IDX_MAX; i++) {
            if (ani_res[i].data) {
                ExtFree(ani_res[i].data);
                ani_res[i].data = NULL;
            }
        }
        memset(ani_res, 0, sizeof(ani_res));

        if (mjpeg_mutex) {
            osMutexDelete(mjpeg_mutex);
            mjpeg_mutex = NULL;
        }

        osEventFlagsSet(task->terminate_sig, ANI_MASK(task->id));
        msg_t msg = { .msgid = ani_evt_elapsed, .param = task->id };
        ani_set_evt(&msg);
    }
    osThreadExit();
}

void ani_res_loader_task(void* args)
{
    APP_CRIT("");
    if (args) {
        ani_task_token_t* task = (ani_task_token_t*)args;
        if (!ani_res_loader_init(task)) {
            ani_res_loader_entry(task);
        }
        ani_res_loader_exit(task);
    }
}

uint32_t ani_res_loader_wait_ready(uint32_t timeout)
{
    return osEventFlagsWait(res_loader_ready, 1, osFlagsNoClear, timeout);
}

void ani_res_loader_set_path(const char* path)
{
    if (!path)
        return;

    for (int i = 0; i < RES_IDX_MAX; i++) {
        if (!strcmp(ani_res[i].path, path)) {
            curr_pos = i;
            break;
        }
    }
}

int ani_res_loader_get_buffer(int offset, char* output, int len)
{
    if (!output)
        return 0;

    int load_len = 0;
    osStatus_t sts = osMutexAcquire(mjpeg_mutex, osWaitForever);

    if (offset >= ani_res[curr_pos].size)
        goto exit;

    if ((ani_res[curr_pos].offset == ani_res[curr_pos].size)
        || (offset+len < ani_res[curr_pos].offset)) {
        load_len = offset+len>ani_res[curr_pos].size ? ani_res[curr_pos].size-offset : len;
        memcpy(output, &ani_res[curr_pos].data[offset], load_len);
    }
    else {
        /* wait loading */
        do {
            if (sts == osOK)
                osMutexRelease(mjpeg_mutex);
            osDelay(10);
            sts = osMutexAcquire(mjpeg_mutex, osWaitForever);
            if ((ani_res[curr_pos].offset == ani_res[curr_pos].size)
                || (offset+len < ani_res[curr_pos].offset)) {
                load_len = offset+len>ani_res[curr_pos].size ? ani_res[curr_pos].size-offset : len;
                memcpy(output, &ani_res[curr_pos].data[offset], load_len);
                break;
            }
        } while (1);
    }

exit:
    if (sts == osOK)
        osMutexRelease(mjpeg_mutex);

    return load_len;
}
/* EOF */
