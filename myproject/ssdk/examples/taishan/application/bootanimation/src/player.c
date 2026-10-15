/**
 * @file player.h
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: sdrv animation application player.
 *
 * Revision History:
 * -----------------
 */

#include "player.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "render.h"
#include "decoder.h"
#include "common.h"
#include "CLI_type.h"

#define MAX_PLAYER_BUFFER_NUM 2

#define  ANIM_EVENT_ABORT           (1u<<0)
#define  ANIM_EVENT_END             (1u<<1)
#define  ANIM_EVENT_ALL             (ANIM_EVENT_ABORT | ANIM_EVENT_END)

#define GUI_ALIGN_DOWN(a, size) ((a) & (~((size)-1)) )
#define GUI_ALIGN_UP(a, size)   (((a)+(size)-1) & (~ ((size)-1)))
#define IRAM_MEM_SIZE (0x1D0000)
extern char IRAM_AP_DATA[IRAM_MEM_SIZE] __SECTION(".apdata");

static osEventFlagsId_t anim_event;
static ANIM_STATE_E anim_state = E_ANIM_STATE_NONE;
static osSemaphoreId_t flush_vsync;

ANIM_COMPONENT_T g_anim_player = {
    .c_init = player_init,
    .c_work = player_work,
    .c_deinit = player_deinit,
};

static void decoder_run(void *ptr)
{
    int ret = 0;
    //decoder_run loop

    ANIM_INFO_T * anim_info_ptr = (ANIM_INFO_T *)ptr;
    ret = anim_info_ptr->a_decoder->c_init(anim_info_ptr);

    if (ret != 0) {
        ssdk_printf(SSDK_CRIT, "decoder_init failed\n");
        osEventFlagsSet(anim_event, ANIM_EVENT_ABORT);
        ret = anim_info_ptr->a_decoder->c_deinit(anim_info_ptr);
        osThreadExit();
        return;
    }

    // framebuffer malloced by player
    anim_info_ptr->a_data->buf_info->fb_num = MAX_PLAYER_BUFFER_NUM;

    for (int i = 0; i < anim_info_ptr->a_data->buf_info->fb_num; i++) {
        ssdk_printf(SSDK_CRIT, "animation pix_bytes %d\n", anim_info_ptr->a_data->buf_info->pix_bytes);

#ifdef BOOTANIM_USE_IRAM
        if (i == 0 && anim_info_ptr->a_data->buf_info->pix_bytes < (IRAM_MEM_SIZE+128)) {
            anim_info_ptr->a_data->buf_info->fb_addr[i] = (uint8_t *)GUI_ALIGN_UP((uint32_t)IRAM_AP_DATA, 128);
            ssdk_printf(SSDK_CRIT, "animation one buffer use iram %p,%p !!! \n", IRAM_AP_DATA, anim_info_ptr->a_data->buf_info->fb_addr[i]);
        } else
#endif
        {
            anim_info_ptr->a_data->buf_info->fb_addr[i] = (uint8_t *)osAllocAlign(32, anim_info_ptr->a_data->buf_info->pix_bytes);
        }
        ssdk_printf(SSDK_CRIT, "animation pix_bytes %p\n", anim_info_ptr->a_data->buf_info->fb_addr[i]);
    }
    anim_info_ptr->a_data->buf_info->fb_decoder = 0;
    anim_info_ptr->a_data->buf_info->fb_render = anim_info_ptr->a_data->buf_info->fb_num - 1;

#ifdef DEBUG_TIME_PRINT
    ssdk_printf(SSDK_CRIT, "animation init done time %d\n", current_time());
#endif
    while (anim_info_ptr->a_data->state == E_ANIM_STATE_PLAYING) {
#ifdef DEBUG_TIME_PRINT
        ssdk_printf(SSDK_CRIT, "animation decoder time %d\n", current_time());
#endif
        ret = anim_info_ptr->a_decoder->c_work(anim_info_ptr);
        if (ret < 0) {
            ssdk_printf(SSDK_CRIT, "decoder_work return ret=%d\n", ret);
            osEventFlagsSet(anim_event, ANIM_EVENT_END);
            break;
        } else {

            anim_info_ptr->a_data->buf_info->fb_decoder = (anim_info_ptr->a_data->buf_info->fb_decoder + 1) % anim_info_ptr->a_data->buf_info->fb_num;
            anim_info_ptr->a_data->buf_info->fb_render = (anim_info_ptr->a_data->buf_info->fb_render + 1) % anim_info_ptr->a_data->buf_info->fb_num;
            osSemaphoreRelease(flush_vsync);
        }
    }

    anim_info_ptr->a_decoder->c_deinit(anim_info_ptr);

    osThreadExit();
}

void start_decoder_thread(ANIM_INFO_T *anim_info_ptr)
{
    osThreadId_t decoder_thread;
    osThreadAttr_t attr = {
        .name = "animation_decoder",
        .stack_size = 4096,
        .priority = osPriorityNormal
    };

    decoder_thread = osThreadNew(decoder_run, (void*)anim_info_ptr, &attr);
}

static void render_run(void *ptr)
{
    int ret = 0;
    //render_run loop

    ANIM_INFO_T * anim_info_ptr = (ANIM_INFO_T *)ptr;
    ret = anim_info_ptr->a_render->c_init(anim_info_ptr);
    if (ret != 0) {
        ssdk_printf(SSDK_CRIT, "render_init failed\n");
        osEventFlagsSet(anim_event, ANIM_EVENT_ABORT);
        ret = anim_info_ptr->a_render->c_deinit(anim_info_ptr);
        osThreadExit();
        return;
    }

    while (anim_info_ptr->a_data->state == E_ANIM_STATE_PLAYING) {
        if (osSemaphoreAcquire(flush_vsync, osWaitForever) == osOK) {
            if (anim_info_ptr->a_data->state == E_ANIM_STATE_PLAYING) {
                anim_info_ptr->a_render->c_work(anim_info_ptr);
            } else {
                break;
            }
        } else {
            ssdk_printf(SSDK_WARNING, "render_work failed\n");
            break;
        }
    }

    anim_info_ptr->a_render->c_deinit(anim_info_ptr);

    osThreadExit();
}

void start_render_thread(ANIM_INFO_T *anim_info_ptr)
{
    osThreadId_t render_thread;
    osThreadAttr_t attr = {
        .name = "animation_render",
        .stack_size = 4096,
        .priority = osPriorityNormal
    };

    render_thread = osThreadNew(render_run, (void*)anim_info_ptr, &attr);
}

int player_init(ANIM_INFO_T *anim_info_ptr)
{
    flush_vsync = osSemaphoreNew(1, 0, NULL);
    anim_event = osEventFlagsNew(NULL);
    return 0;
}

int player_work(ANIM_INFO_T *anim_info_ptr)
{
    int32_t     flags;
    //animation main loop

    // start decoder thread loop
    start_decoder_thread(anim_info_ptr);

    // start render thread loop
    start_render_thread(anim_info_ptr);

    /* wait recv event happen */
    do {
        flags = osEventFlagsWait(anim_event, ANIM_EVENT_ALL, osFlagsWaitAny, osWaitForever);
        if (flags & ANIM_EVENT_ABORT) {
            ssdk_printf(SSDK_CRIT, "animation_run abort event\n");
            anim_state = ANIM_EVENT_END;
            break;
        } else if (flags & ANIM_EVENT_END) {
            ssdk_printf(SSDK_CRIT, "animation_run end event\n");
            anim_state = ANIM_EVENT_END;
            break;
        }
    } while (1);

    return 0;
}

int player_deinit(ANIM_INFO_T *anim_info_ptr) {
    osSemaphoreRelease(flush_vsync);
    osDelay(20);
    osSemaphoreDelete(flush_vsync);

    for (int i = 0; i < anim_info_ptr->a_data->buf_info->fb_num; i++) {
        if (NULL != anim_info_ptr->a_data->buf_info->fb_addr[i]) {
#ifdef BOOTANIM_USE_IRAM
            if (i == 0) {
                anim_info_ptr->a_data->buf_info->fb_addr[i] = NULL;
                continue;
            }
#endif
            osFree(anim_info_ptr->a_data->buf_info->fb_addr[i]);
            anim_info_ptr->a_data->buf_info->fb_addr[i] = NULL;
        }
    }

    osEventFlagsDelete(anim_event);
    return 0;
}
