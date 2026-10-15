/**
 * @file sdrv_slimfs_sysres_wrapper.c
 *
 * Copyright (c) 2023 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: system resource wrapper
 *
 * Revision History:
 * -----------------
 */
/* =========================== include file session ======================== */
#include <stdint.h>

#include "sdrv_slimfs_config.h"
#include "sdrv_slimfs_impl.h"
#include "sdrv_slimfs_log.h"

#if (CONFIG_SLIMFS_OS_TYPE == CONFIG_SLIMFS_OS_BAREMETAL)
#elif (CONFIG_SLIMFS_OS_TYPE == CONFIG_SLIMFS_OS_LK)
#include <kernel/mutex.h>
#include <sys_cnt.h>
#include <platform.h>
#include <lk/init.h>
#elif (CONFIG_SLIMFS_OS_TYPE == CONFIG_SLIMFS_OS_CMSIS)
#include <cmsis_os2.h>
#endif
/* =========================== marco define session ======================== */
/* =========================== enumeration define session ================== */
/* =========================== structure define session ==================== */
/* =========================== function declare session ==================== */
/* =========================== global value define session ================= */
/* =========================== static value define session ================= */
/* =========================== function implementation session ============= */
#if (CONFIG_SLIMFS_OS_TYPE == CONFIG_SLIMFS_OS_BAREMETAL)
mutex_wrap_t slimfs_mutex_init()
{
    static int mutex = 0;
    return &mutex;
}

int32_t slimfs_mutex_acquire(mutex_wrap_t mutex)
{
    return 0;
}

int32_t slimfs_mutex_release(mutex_wrap_t mutex)
{
    return 0;
}

void slimfs_mutex_deinit(mutex_wrap_t mutex)
{
    return;
}

uint32_t slimfs_get_us_tick(void)
{
    return 0;
}

#elif (CONFIG_SLIMFS_OS_TYPE == CONFIG_SLIMFS_OS_LK)
mutex_wrap_t slimfs_mutex_init()
{
    mutex_wrap_t mutex = NULL;
    mutex = malloc(sizeof(mutex_t));
    if (mutex == NULL) {
        SLIMFS_PRINT(SLIMFS_LOG_ERR, "malloc memory for mutex failed.\n");
        return NULL;
    }

    mutex_init((mutex_t *)mutex);

    return mutex;
}

int32_t slimfs_mutex_acquire(mutex_wrap_t mutex)
{
    if (mutex == NULL) {
        SLIMFS_PRINT(SLIMFS_LOG_ERR, "parameter error for %s.\n", __func__);
        return -1;
    }

    return mutex_acquire((mutex_t *)mutex);
}

int32_t slimfs_mutex_release(mutex_wrap_t mutex)
{
    if (mutex == NULL) {
        SLIMFS_PRINT(SLIMFS_LOG_ERR, "parameter error for %s.\n", __func__);
        return -1;
    }

    return mutex_release((mutex_t *)mutex);
}

void slimfs_mutex_deinit(mutex_wrap_t mutex)
{
    if (mutex == NULL) {
        SLIMFS_PRINT(SLIMFS_LOG_ERR, "parameter error for %s.\n", __func__);
        return ;
    }

    mutex_destroy((mutex_t *)mutex);
    return ;
}

uint32_t slimfs_get_us_tick(void)
{
    return (current_time());
}

int32_t slimfs_init_wrapper(void* args)
{
    return slimfs_init();
}

int32_t slimfs_init_nonblock(void)
{
    thread_t* thd = thread_create("slimfs_init", slimfs_init_wrapper, NULL, DEFAULT_PRIORITY, DEFAULT_STACK_SIZE);
    if (thd == NULL) {
        SLIMFS_PRINT(SLIMFS_LOG_ERR, "create thread failed for %s, init slimfs with block type\n", __func__);
        return slimfs_init();
    }
    thread_detach_and_resume(thd);
    return 0;
}

// set hook for slimfs init
LK_INIT_HOOK(slimfs_init, (lk_init_hook)slimfs_init, LK_INIT_LEVEL_TARGET+1);

#elif (CONFIG_SLIMFS_OS_TYPE == CONFIG_SLIMFS_OS_CMSIS)
mutex_wrap_t slimfs_mutex_init()
{
    osMutexId_t mutex = osMutexNew(NULL);

    return mutex;
}

int32_t slimfs_mutex_acquire(mutex_wrap_t mutex)
{
    if (mutex == NULL) {
        SLIMFS_PRINT(SLIMFS_LOG_ERR, "parameter error for %s.\n", __func__);
        return -1;
    }

    return osMutexAcquire((osMutexId_t)mutex, osWaitForever);
}

int32_t slimfs_mutex_release(mutex_wrap_t mutex)
{
    if (mutex == NULL) {
        SLIMFS_PRINT(SLIMFS_LOG_ERR, "parameter error for %s.\n", __func__);
        return -1;
    }

    return osMutexRelease((osMutexId_t)mutex);
}

void slimfs_mutex_deinit(mutex_wrap_t mutex)
{
    if (mutex == NULL) {
        SLIMFS_PRINT(SLIMFS_LOG_ERR, "parameter error for %s.\n", __func__);
        return;
    }

    osMutexDelete((osMutexId_t)mutex);
}

uint32_t slimfs_get_us_tick(void)
{
    return 0;
}

void slimfs_init_wrapper(void* args)
{
    slimfs_init();
    osThreadExit();
}

int32_t slimfs_init_nonblock(void)
{
    osThreadAttr_t attr;
    memset(&attr, 0x00, sizeof(attr));
    attr.stack_size = 4096;
    osThreadId_t thd = osThreadNew(slimfs_init_wrapper, NULL, &attr);
    if (thd == NULL) {
        SLIMFS_PRINT(SLIMFS_LOG_ERR, "create thread failed for %s, init slimfs with block type\n", __func__);
        slimfs_init();
    }

    return 0;
}

#else
error os type

#endif /** end CONFIG_SLIMFS_OS_TYPE */


/* end of file sdrv_slimfs_sysres_wrapper.c */