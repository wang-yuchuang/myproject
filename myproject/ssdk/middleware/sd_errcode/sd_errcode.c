/*
 * sd_errcode.c
 *
 * Copyright (c) 2022 SemiDrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 * errcode library
 *
 * Revision History:
 * -----------------
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <debug.h>
#include <driver/kunlun/rtc/dw_rtc_drv.h>
#include <rtc_cfg.h>
#include <errcode_cfg.h>
#include <lib/sd_boot_pin/sd_boot_pin.h>
#include <lib/list.h>
#include <spinlock.h>
#include <cmsis_os2.h>
#include <board.h>
#include "reg.h"
#include "sd_errcode.h"
#include "sd_errcode_ospi.h"
#include "sd_errcode_emmc.h"
#include "sd_errcode_net.h"

#define SYS_CNT_CNTCVUREAD      (0x4)
#define MILLION                 (1000000)

#define ERRCODE_LEN_MAX 20
#define ERRCODE_SEND_ASYNC 1

#define ERRCODE_SUPPORT_OSPI       (1 << 0)
#define ERRCODE_SUPPORT_EMMC       (1 << 1)
#define ERRCODE_SUPPORT_NET        (1 << 2)

#define ERRCODE_MSG_MINORS 64
#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))

#define ERRCODE_EVENT_COMPLETE   0x1

#define errcode_mdelay(x) osDelay(x * 1000)

typedef struct {
    struct list_node list;
    uint8_t data[ERRCODE_LEN_MAX];
    uint16_t length;
    storage_id_t strid;
    uint32_t unique_id;
    uint32_t id;        //errcode_msg_minor index
} errcode_msg_t;

typedef errcode_msg_t *errcode_msg_handle_t;

typedef struct {
    errcode_msg_t errcode_msg;
    bool allocated;
} errcode_msg_minor_t;

typedef struct {
    errcode_msg_minor_t errcode_msg_minors[ERRCODE_MSG_MINORS];
    spin_lock_t lock;
} errcode_msg_mgr_t;

typedef struct {
    spin_lock_t lock;
    struct list_node errcode_list;
    osThreadId_t send_thread;
    osEventFlagsId_t event;
    osSemaphoreId_t sem;
    storage_id_t strid;
    bool errcode_inited;
    bool errcode_ospi_inited;
    bool errcode_emmc_inited;
    bool errcode_net_inited;
} errcode_context;

static errcode_context g_errcode_service;

static errcode_msg_mgr_t g_errcode_msg_mgr;

static storage_id_t errcode_get_support_strid(storage_id_t current_strid);

static int errcode_msg_get_id(errcode_msg_handle_t message)
{
    int id;

    id = message->id;

    return id;
}

static errcode_msg_handle_t find_free_errcode_msg(void)
{
    errcode_msg_handle_t errcode_msg = NULL;
    errcode_msg_mgr_t *errcode_msg_mgr = &g_errcode_msg_mgr;
    errcode_msg_minor_t *errcode_msg_minors = errcode_msg_mgr->errcode_msg_minors;
    irq_state_t state;
    int minor = 0;

    state = spin_lock_irqsave(&errcode_msg_mgr->lock);

    for (minor = 0; minor < (int)ARRAY_SIZE(errcode_msg_mgr->errcode_msg_minors);
         ++minor) {
        if (!errcode_msg_minors[minor].allocated) {
            errcode_msg_minors[minor].allocated = true;
            errcode_msg = &errcode_msg_minors[minor].errcode_msg;
            break;
        }
    }

    spin_unlock_irqrestore(&errcode_msg_mgr->lock, state);

    if (minor >= (int)ARRAY_SIZE(errcode_msg_mgr->errcode_msg_minors)) {
        printf("%s : errcode message buffer reach to MAX, need to extend message buffer.\n", __func__);
    }

    return errcode_msg;
}

static int release_errcode_msg(int minor)
{
    errcode_msg_mgr_t *errcode_msg_mgr = &g_errcode_msg_mgr;
    errcode_msg_minor_t *errcode_msg_minors = errcode_msg_mgr->errcode_msg_minors;
    irq_state_t state;

    if (minor >= (int)ARRAY_SIZE(errcode_msg_mgr->errcode_msg_minors)) {
        printf("%s : Invalid errcode message buffer index. minor = %d\n", __func__, minor);
        return -1;
    }

    state = spin_lock_irqsave(&errcode_msg_mgr->lock);
    errcode_msg_minors[minor].allocated = false;
    spin_unlock_irqrestore(&errcode_msg_mgr->lock, state);

    return 0;
}


static bool is_platform_emmc_only(void)
{
    bool ret = false;
#if 0
#if SUPPORT_PARALLEL_BOOT
    //Do nothing
#else
    if (boot_get_pin() != BOOT_PIN_0) {
        ret = true;
    }
#endif
#endif
    return ret;
}

static uint64_t semidrive_get_systime_us(void)
{
    uint64_t syscnt_low, syscnt_high, syscnt;
    int freq_mhz;

    syscnt_low = readl(APB_SYS_CNT_RO_BASE);
    syscnt_high = readl(APB_SYS_CNT_RO_BASE + SYS_CNT_CNTCVUREAD);

    syscnt = (syscnt_high << 32) | syscnt_low;

    freq_mhz = CKGEN_SYS_CNT / MILLION;

    return syscnt / freq_mhz;
}

static void errcode_send_async_thread(void *arg) {
    errcode_context *cxt = (errcode_context *)arg;
    errcode_msg_t *message = NULL;
    storage_id_t strid = STR_ID_INVALID;
    ERR_HANDLE handle = 0;
    handle = handle;
    uint8_t *data_buffer = NULL;
    data_buffer = data_buffer;
    uint16_t length = 0;
    length = length;
    int list_num = 0;
    irq_state_t state;

    for (;;) {
        osEventFlagsWait(cxt->event, ERRCODE_EVENT_COMPLETE, osFlagsWaitAll, osWaitForever);
        //osSemaphoreAcquire(cxt->sem, osWaitForever);

        state = spin_lock_irqsave(&cxt->lock);
        list_num = list_length(&cxt->errcode_list);
        spin_unlock_irqrestore(&cxt->lock, state);

        ERRCODE_DEBUG_PRINT("%s: list_num = %d\n", __func__, list_num);

        while (list_num--) {
            state = spin_lock_irqsave(&cxt->lock);
            message = list_remove_head_type(&cxt->errcode_list, errcode_msg_t, list);
            spin_unlock_irqrestore(&cxt->lock, state);

            if (message) {
                strid = message->strid;
                data_buffer = message->data;
                length = message->length;

                //Store an errcode to storage device
                switch(strid) {
                case STR_ID_OSPI1:
                case STR_ID_OSPI2:
#if defined(CONFIG_FEE_EEPROM)
                    handle = errcode_set_to_ospi(strid, data_buffer, length);
#else
                    printf("%s : Current platform does not support OSPI storage!\n", __func__);
#endif
                    break;
                case STR_ID_EMMC1:
                case STR_ID_EMMC2:
#if defined(CONFIG_SLIMFS)
                    handle = errcode_set_to_emmc(strid, data_buffer, length);
#else
                    printf("%s : Current platform does not support eMMC storage!\n", __func__);
#endif
                    break;
                case STR_ID_SOCKET:
#if defined(CONFIG_LWIP)
                    errcode_send_socket(data_buffer, length);
#else
                    printf("%s: The platform do not support socket!\n", __func__);
#endif
                    break;
                default :
                    printf("%s:Invalid storage!\n", __func__);
                    break;
                }

                release_errcode_msg(errcode_msg_get_id(message));
                message = NULL;
            }
            else {
                printf("%s: Invalid errcode message!\n", __func__);
            }
        }
    }

    return;
}

int system_errcode_early_init(void)
{
    int ret = ERRCODE_OK;

    storage_id_t str_id = g_strid;
    errcode_context *cxt = &g_errcode_service;
    int minor;
    errcode_msg_handle_t errcode_msg = NULL;
    errcode_msg_mgr_t *errcode_msg_mgr = &g_errcode_msg_mgr;
    memset(errcode_msg_mgr, 0, sizeof(errcode_msg_mgr_t));
    errcode_msg_minor_t *errcode_msg_minors = errcode_msg_mgr->errcode_msg_minors;

    memset(cxt, 0, sizeof(*cxt));

    spin_lock_init(&cxt->lock);

    list_initialize(&cxt->errcode_list);
    cxt->errcode_inited = true;
    cxt->errcode_ospi_inited = true;
    cxt->errcode_emmc_inited = true;
    cxt->errcode_net_inited = true;

#ifndef CONFIG_FEE_EEPROM
    if (str_id == STR_ID_OSPI1) {
        str_id = errcode_get_support_strid(str_id);
    }
#endif

#ifndef CONFIG_SLIMFS
    if (str_id == STR_ID_EMMC1) {
        str_id = errcode_get_support_strid(str_id);
    }
#endif

    cxt->strid = str_id;

    spin_lock_init(&errcode_msg_mgr->lock);
    for (minor = 0; minor < (int)ARRAY_SIZE(errcode_msg_mgr->errcode_msg_minors); ++minor) {
        errcode_msg = &errcode_msg_minors[minor].errcode_msg;
        memset(errcode_msg, 0, sizeof(errcode_msg_t));
        errcode_msg_minors[minor].allocated = false;
        errcode_msg->id = minor;
    }

    return ret;
}

static void system_errcode_init_thread(void *arg) {
    int ret = ERRCODE_OK;
    errcode_context *cxt = (errcode_context *)arg;

#if defined(CONFIG_LWIP)
    ret = errcode_net_init();
    if (ret != ERRCODE_OK) {
        printf("%s: errcode_net_init failed.\n", __func__);
        cxt->errcode_net_inited = false;
    }
#else
    cxt->errcode_net_inited = false;
#endif

#if defined(CONFIG_FEE_EEPROM)
    ret = errcode_ospi_init();
    if (ret != ERRCODE_OK) {
        printf("%s: errcode_ospi_init failed.\n", __func__);
        cxt->errcode_ospi_inited = false;
    }
#else
    cxt->errcode_ospi_inited = false;
#endif

#if defined(CONFIG_SLIMFS)
    ret = errcode_emmc_init();
    if (ret != ERRCODE_OK) {
        printf("%s: errcode_emmc_init failed.\n", __func__);
        cxt->errcode_emmc_inited = false;
    }
#else
    cxt->errcode_emmc_inited = false;
#endif

    printf("%s: system_errcode_init start...cxt->strid=%d, g_strid=%d\n", __func__, cxt->strid, g_strid);

    if (!cxt->errcode_net_inited && !cxt->errcode_ospi_inited && !cxt->errcode_emmc_inited) {
        cxt->errcode_inited = false;
        printf("%s: All errcode storage init failed.\n", __func__);
        goto Label;
    }

    cxt->event = osEventFlagsNew(NULL);
    //cxt->sem = osSemaphoreNew(5, 0, NULL);
    osThreadAttr_t errcode_task_attr = {
        .name = "errcode_send_task",
        .stack_size = 4096,
        .priority = osPriorityNormal2,
    };

    cxt->send_thread = osThreadNew(errcode_send_async_thread, (void *)cxt, &errcode_task_attr);
    if (!cxt->send_thread) {
        ssdk_printf(SSDK_ALERT, "%s: create thread failed.\r\n", __func__);
        cxt->errcode_inited = false;
        goto Label;
    }

    osEventFlagsSet(cxt->event, ERRCODE_EVENT_COMPLETE);
    //osSemaphoreRelease(cxt->sem);
Label:
    printf("%s: errcode init finish.\n", __func__);
    osThreadExit();
}

int system_errcode_init(void)
{
    int ret = ERRCODE_OK;
    errcode_context *cxt = &g_errcode_service;
    printf("%s: errcode init start...\n", __func__);

    if (cxt->strid == STR_ID_INVALID) {
        cxt->errcode_ospi_inited = false;
        cxt->errcode_emmc_inited = false;
        cxt->errcode_net_inited = false;
        cxt->errcode_inited = false;
        printf("%s: No available storage to store errcode, errcode init failed.\n", __func__);
        ret = ERRCODE_NG;
        goto Label;
    }

    osThreadAttr_t errcode_init_task_attr = {
        .name = "errcode_init_task",
        .stack_size = 4096,
        .priority = osPriorityNormal2,
    };

    osThreadId_t thread = osThreadNew(system_errcode_init_thread, (void *)cxt, &errcode_init_task_attr);
    if (!thread) {
        ssdk_printf(SSDK_ALERT, "%s: create thread failed.\r\n", __func__);
        cxt->errcode_inited = false;
        ret = ERRCODE_NG;
        goto Label;
    }
    printf("%s: errcode init done.\n", __func__);
Label:

    return ret;
}

int system_errcode_deinit(void)
{
    int ret = ERRCODE_OK;
    errcode_context *cxt = &g_errcode_service;
    ERRCODE_DEBUG_PRINT("system_errcode_deinit start...\n");

#if defined(CONFIG_SLIMFS)
    errcode_emmc_deinit();
#endif

#if defined(CONFIG_LWIP)
    errcode_net_deinit();
#endif
    cxt->errcode_inited = false;

    return ret;
}

int system_errcode_set_storage_id(storage_id_t strid)
{
    int ret = ERRCODE_OK;
    irq_state_t state;
    errcode_context *cxt = &g_errcode_service;

    state = spin_lock_irqsave(&cxt->lock);
    cxt->strid = strid;
    spin_unlock_irqrestore(&cxt->lock, state);

    ERRCODE_DEBUG_PRINT("%s : cxt->strid=%d\n", __func__, cxt->strid);

    return ret;
}

int system_errcode_get_storage_id()
{
    int ret = ERRCODE_OK;
    irq_state_t state;
    storage_id_t strid;
    errcode_context *cxt = &g_errcode_service;

    state = spin_lock_irqsave(&cxt->lock);
    strid = cxt->strid;
    spin_unlock_irqrestore(&cxt->lock, state);

    ERRCODE_DEBUG_PRINT("%s : cxt->strid=%d\n", __func__, cxt->strid);

    return strid;
}

static bool is_ospi_storage(storage_id_t strid)
{
    return ((strid == STR_ID_OSPI1) || (strid == STR_ID_OSPI2));
}

static bool is_emmc_storage(storage_id_t strid)
{
    return ((strid == STR_ID_EMMC1) || (strid == STR_ID_EMMC2));
}

static int errcode_get_support_storage_type(void)
{
    int type = 0;
    errcode_context *cxt = &g_errcode_service;
    cxt = cxt;
#if defined(CONFIG_FEE_EEPROM)
    if (cxt->errcode_ospi_inited) {
        type |= ERRCODE_SUPPORT_OSPI;
    }
#endif

#if defined(CONFIG_SLIMFS)
    if (cxt->errcode_emmc_inited) {
        type |= ERRCODE_SUPPORT_EMMC;
    }
#endif

#if defined(CONFIG_LWIP)
    if (cxt->errcode_net_inited) {
        type |= ERRCODE_SUPPORT_NET;
    }
#endif

    return type;
}

static storage_id_t errcode_get_support_strid(storage_id_t current_strid)
{
    int type = 0;
    storage_id_t strid = STR_ID_INVALID;

    type = errcode_get_support_storage_type();

    //priority OSPI > eMMC > NET

    if ((STR_ID_OSPI1 != current_strid) && (ERRCODE_SUPPORT_OSPI&type)) {
        strid = STR_ID_OSPI1;
    }
    else if ((STR_ID_EMMC1 != current_strid) && (ERRCODE_SUPPORT_EMMC&type)) {
        strid = STR_ID_EMMC1;
    }
    else if ((STR_ID_SOCKET != current_strid) && (ERRCODE_SUPPORT_NET&type)) {
        strid = STR_ID_SOCKET;
    }
    else {
        strid = STR_ID_INVALID;
    }

    return strid;
}

#if ERRCODE_SEND_ASYNC
static ERR_HANDLE errcode_set_internal(errcode_t* errcode, storage_id_t str_id)
{
    ERR_HANDLE handle = ERRCODE_OK;
    uint8_t data_buffer[ERRCODE_LEN_MAX] = {0};
    uint16_t length = 0;
    uint64_t timestamp = 0;
    uint32_t tmp_errcode = 0;
    uint8_t *ptr = NULL;
    uint16_t len = 0;
    storage_id_t strid;
    irq_state_t state;
    errcode_context *cxt = &g_errcode_service;
    int ret = 0;

    state = spin_lock_irqsave(&cxt->lock);
    strid = ((str_id == STR_ID_INVALID) ? cxt->strid : str_id);
    spin_unlock_irqrestore(&cxt->lock, state);

    ptr = data_buffer;

    if (strid >= STR_ID_MAX) {
        // printf("%s : Invalid storage device, strid = %d\n", __func__, strid);
        return ERRCODE_NG;
    }

    if (!errcode) {
        // printf("%s : Invalid param errcode = NULL!\n", __func__);
        return ERRCODE_NG;
    }

    ERRCODE_DEBUG_PRINT("%s: start strid = %d, id = 0x%x, no = 0x%x, errcode = 0x%x, user_data = 0x%x\n", \
        __func__, strid, errcode->mod_id, errcode->mod_no, errcode->error_code, errcode->user_data);

    //Get timestamp and create errcode
    tmp_errcode = ((errcode->mod_id << 24)&0xff000000) + \
        ((errcode->mod_no << 16)&0x00ff0000) + \
        (errcode->error_code & 0x0000ffff);

    //Check whether the errcode has been cached in errcode_list
    errcode_msg_t *errcode_message = NULL;
    state = spin_lock_irqsave(&cxt->lock);
    list_for_every_entry(&cxt->errcode_list, errcode_message, errcode_msg_t, list) {
        if (tmp_errcode == errcode_message->unique_id) {
            spin_unlock_irqrestore(&cxt->lock, state);
            //ERRCODE_DEBUG_PRINT("%s : The errcode has been cached in errcode_list, num = %d.\n", __func__, list_length(&cxt->errcode_list));
            return handle;
        }
    }
    spin_unlock_irqrestore(&cxt->lock, state);
#if defined(CONFIG_DW_RTC)
    struct rtc_time tm;
    rtc_dev_t *rtc_dev = NULL;
    rtc_dev = &g_rtc1_dev;
    ret = rtc_get_time(rtc_dev, &tm);
    if (ret < 0 ) {
        ssdk_printf(SSDK_EMERG, "%s ret get time fialed\n", __func__);
        return ERRCODE_NG;
    }
    timestamp = rtc_to_epoch(&tm);
#else
    timestamp = semidrive_get_systime_us() / 1000;
#endif
    ERRCODE_DEBUG_PRINT("%s: timestamp = %llu[%llx]\n", __func__, timestamp, timestamp);

    len = sizeof(errcode->user_data);
    memcpy(ptr, &errcode->user_data, len);
    length += len;
    ptr += len;

    len = sizeof(tmp_errcode);
    memcpy(ptr, &tmp_errcode, len);
    length += len;
    ptr += len;

    len = sizeof(timestamp);
    memcpy(ptr, &timestamp, len);
    length += len;

    ERRCODE_DEBUG_PRINT("%s: length = %d\n", __func__, length);
    ERRCODE_DEBUG_PRINT("[");
    for (int i = 0; i < length; i++) {
        ERRCODE_DEBUG_PRINT(" %02x", data_buffer[i]);
    }
    ERRCODE_DEBUG_PRINT("]\n");

    //Find free errcode_msg_t and push it to list_node
    errcode_msg_handle_t message = find_free_errcode_msg();
    if (message == NULL) {
        printf("%s : Can not find available errcode message.\n", __func__);
        return handle;
    }

    list_initialize(&message->list);
    memcpy(message->data, data_buffer, length);
    message->length = length;
    message->strid = strid;
    message->unique_id = tmp_errcode;

    state = spin_lock_irqsave(&cxt->lock);
    list_add_tail(&cxt->errcode_list, &message->list);
    spin_unlock_irqrestore(&cxt->lock, state);
    //ERRCODE_DEBUG_PRINT("%s: Add a new errcode to errcode_list, num = %d\n", __func__, list_length(&cxt->errcode_list));

    osEventFlagsSet(cxt->event, ERRCODE_EVENT_COMPLETE);
    //osSemaphoreRelease(cxt->sem);
    return handle;
}
#else
static ERR_HANDLE errcode_set_internal(errcode_t* errcode, storage_id_t str_id)
{
    ERR_HANDLE handle = ERRCODE_OK;
    int ret = 0;
    uint8_t data_buffer[ERRCODE_LEN_MAX] = {0};
    uint16_t length = 0;
    uint64_t timestamp = 0;
    uint32_t tmp_errcode = 0;
    uint8_t *ptr = NULL;
    uint16_t len = 0;
    storage_id_t strid = ((str_id == STR_ID_INVALID) ? g_strid : str_id);

    ptr = data_buffer;

    if (!cxt->errcode_inited) {
        printf("%s : errcode service is not inited!\n", __func__);
        return ERRCODE_NG;
    }

    if (strid >= STR_ID_MAX) {
        printf("%s : Invalid storage device, strid = %d\n", __func__, strid);
        return ERRCODE_NG;
    }

    if (!errcode) {
        printf("%s : Invalid param errcode = NULL!\n", __func__);
        return ERRCODE_NG;
    }

    ERRCODE_DEBUG_PRINT("errcode_set start strid = %d, mod_id = 0x%x, mod_no = 0x%x, error_code = 0x%x, user_data = 0x%x\n", \
        strid, errcode->mod_id, errcode->mod_no, errcode->error_code, errcode->user_data);

    //Get timestamp and create errcode
#if defined(CONFIG_DW_RTC)
    struct rtc_time tm;
    rtc_dev_t *rtc_dev = NULL;
    rtc_dev = &g_rtc1_dev;
    ret = rtc_get_time(rtc_dev, &tm);
    if (ret < 0 ) {
        ssdk_printf(SSDK_EMERG, "%s ret get time fialed\n", __func__);
        return -1;
    }
    timestamp = rtc_to_epoch(&tm);
#else
    timestamp = semidrive_get_systime_us() / 1000;
#endif
    ERRCODE_DEBUG_PRINT("errcode_set timestamp = %llu[%llx]\n", timestamp, timestamp);
    tmp_errcode = (errcode->mod_id << 24) + (errcode->mod_no << 16) + (errcode->error_code & 0xFFFF);

    len = sizeof(errcode->user_data);
    memcpy(ptr, &errcode->user_data, len);
    length += len;
    ptr += len;

    len = sizeof(tmp_errcode);
    memcpy(ptr, &tmp_errcode, len);
    length += len;
    ptr += len;

    len = sizeof(timestamp);
    memcpy(ptr, &timestamp, len);
    length += len;

    ERRCODE_DEBUG_PRINT("errcode_set length = %d\n", length);
    ERRCODE_DEBUG_PRINT("[");
    for (int i = 0; i < length; i++) {
        ERRCODE_DEBUG_PRINT(" %02x", data_buffer[i]);
    }
    ERRCODE_DEBUG_PRINT("]\n");

    switch(strid) {
    case STR_ID_OSPI1:
    case STR_ID_OSPI2:
#if defined(CONFIG_FEE_EEPROM)
        handle = errcode_set_to_ospi(strid, data_buffer, length);
#else
        printf("%s : Current platform does not support OSPI storage!\n", __func__);
#endif
        break;
    case STR_ID_EMMC1:
    case STR_ID_EMMC2:
#if defined(CONFIG_SLIMFS)
        handle = errcode_set_to_emmc(strid, data_buffer, length);
#else
        printf("%s : Current platform does not support eMMC storage!\n", __func__);
#endif
        break;
    case STR_ID_SOCKET:
#if defined(CONFIG_LWIP)
#if ERRCODE_NET_SEND_ASYNC
        errcode_send_socket(data_buffer, length);
#else
        errcode_set_to_net(NULL, data_buffer, length);
#endif
#else
        printf("%s: The platform do not support socket!\n", __func__);
#endif
        break;
    default :
        printf("%s:Invalid storage!\n", __func__);
        break;
    }

    return handle;
}
#endif

ERR_HANDLE errcode_set(errcode_t* errcode)
{
    storage_id_t str_id = STR_ID_INVALID;
    irq_state_t state;
    errcode_context *cxt = &g_errcode_service;

    if (!errcode) {
        ERRCODE_DEBUG_PRINT("%s : Invalid param, errcode == NULL!\n", __func__);
        return ERRCODE_NG;
    }

    if (!cxt->errcode_inited) {
        ERRCODE_DEBUG_PRINT("%s : errcode service is not inited!\n", __func__);
        return ERRCODE_NG;
    }

    if (cxt->strid == STR_ID_INVALID) {
        ERRCODE_DEBUG_PRINT("%s: No available storage to store errcode.\n", __func__);
        return ERRCODE_NG;
    }

    //Storage exception process, change to another avaliable str_id.
    state = spin_lock_irqsave(&cxt->lock);
    switch (errcode->mod_id) {
        case MOD_ID_SYS_SERVICE_STR_MMC:
            if (STR_ID_EMMC1 == cxt->strid) {
                str_id = errcode_get_support_strid(cxt->strid);
            }
            break;
        case MOD_ID_SYS_SERVICE_STR_OSPI:
            if (STR_ID_OSPI1 == cxt->strid) {
                str_id = errcode_get_support_strid(cxt->strid);
            }
            break;
        case MOD_ID_SYS_SERVICE_COMM:
            if ((errcode->error_code <= 0x0FFF) && (STR_ID_SOCKET == cxt->strid)) {
                str_id = errcode_get_support_strid(cxt->strid);
            }
            break;
        default:
            break;
    }
    spin_unlock_irqrestore(&cxt->lock, state);

    if (str_id != STR_ID_INVALID) {
        system_errcode_set_storage_id(str_id);
        ERRCODE_DEBUG_PRINT("%s : Change errcode storage to %d!\n", __func__, str_id);
    }

    return errcode_set_internal(errcode, str_id);
}

ERR_HANDLE errcode_set_by_strid(errcode_t* errcode, storage_id_t str_id)
{
    if (!errcode) {
        printf("%s : Invalid param, errcode == NULL!\n", __func__);
        return ERRCODE_NG;
    }

    return errcode_set_internal(errcode, str_id);
}

int errcode_transfer(const char* ip, errcode_t* errcode)
{
    int ret = ERRCODE_OK;
    uint8_t data_buffer[ERRCODE_LEN_MAX] = {0};
    uint16_t length = 0;
    uint64_t timestamp = 0;
    uint32_t tmp_errcode = 0;
    uint8_t *ptr = NULL;
    uint16_t len = 0;
    errcode_context *cxt = &g_errcode_service;

    ptr = data_buffer;

    if (!cxt->errcode_inited) {
        printf("%s : errcode service is not inited!\n", __func__);
        return ERRCODE_NG;
    }

    if (!errcode) {
        printf("%s : Invalid param errcode = NULL!\n", __func__);
        return ERRCODE_NG;
    }

    //Get timestamp and create errcode
#if defined(CONFIG_DW_RTC)
    struct rtc_time tm;
    rtc_dev_t *rtc_dev = NULL;
    rtc_dev = &g_rtc1_dev;
    ret = rtc_get_time(rtc_dev, &tm);
    if (ret < 0 ) {
        ssdk_printf(SSDK_EMERG, "%s ret get time fialed\n", __func__);
        return -1;
    }
    timestamp = rtc_to_epoch(&tm);
#else
    timestamp = semidrive_get_systime_us() / 1000;
#endif
    tmp_errcode = (errcode->mod_id << 24) + (errcode->mod_no << 16) + (errcode->error_code & 0xFFFF);

    len = sizeof(errcode->user_data);
    memcpy(ptr, &errcode->user_data, len);
    length += len;
    ptr += len;

    len = sizeof(tmp_errcode);
    memcpy(ptr, &tmp_errcode, len);
    length += len;
    ptr += len;

    len = sizeof(timestamp);
    memcpy(ptr, &timestamp, len);
    length += len;

    ERRCODE_DEBUG_PRINT("errcode_transfer length = %d\n", length);
    ERRCODE_DEBUG_PRINT("[");
    for (int i = 0; i < length; i++) {
        ERRCODE_DEBUG_PRINT(" %02x", data_buffer[i]);
    }
    ERRCODE_DEBUG_PRINT("]\n");
#if defined(CONFIG_LWIP)
#if ERRCODE_NET_SEND_ASYNC
    if (is_need_create_socket() || is_remote_server_changed(ip)) {
        errcode_close_socket();
        errcode_open_socket(ip, 0);
        errcode_send_socket(data_buffer, length);
    }
    else
    {
        errcode_send_socket(data_buffer, length);
    }
#else
    errcode_set_to_net(ip, data_buffer, length);
#endif
#else
    printf("%s: The platform do not support socket!\n", __func__);
#endif

    return ret;
}

int errcode_dump(storage_id_t strid, char* buf, int bufsize)
{
    int ret = ERRCODE_OK;
    errcode_context *cxt = &g_errcode_service;

    if (!cxt->errcode_inited) {
        printf("%s : errcode service is not inited!\n", __func__);
        return ERRCODE_NG;
    }

    if (is_ospi_storage(strid)) {
#if defined(CONFIG_FEE_EEPROM)
        errcode_dump_from_ospi(strid, buf, bufsize);
#else
        printf("%s : Current platform does not support OSPI storage!\n", __func__);
#endif
    }
    else if (is_emmc_storage(strid)) {
#if defined(CONFIG_SLIMFS)
        errcode_dump_from_emmc(strid, buf, bufsize);
#else
        printf("%s : Current platform does not support eMMC storage!\n", __func__);
#endif
    }
    else {
        printf("%s:Invalid storage!\n", __func__);
    }

    return ret;
}

int errcode_clear(storage_id_t strid)
{
    int ret = ERRCODE_OK;
    errcode_context *cxt = &g_errcode_service;

    if (!cxt->errcode_inited) {
        printf("%s : errcode service is not inited!\n", __func__);
        return ERRCODE_NG;
    }

    if (is_ospi_storage(strid)) {
#if defined(CONFIG_FEE_EEPROM)
        errcode_clear_ospi(strid);
#else
        printf("%s : Current platform does not support OSPI storage!\n", __func__);
#endif
    }
    else if (is_emmc_storage(strid)) {
#if defined(CONFIG_SLIMFS)
        errcode_clear_emmc(strid);
#else
        printf("%s : Current platform does not support eMMC storage!\n", __func__);
#endif
    }
    else {
        printf("%s:Invalid storage!\n", __func__);
    }

    return ret;
}

