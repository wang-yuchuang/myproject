/**
 * @file animation.c
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: sdrv animation application.
 *
 * Revision History:
 * -----------------
 */

#include "animation.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "render.h"
#include "decoder.h"
#include "common.h"
#include "CLI_type.h"
/**--------------------------------------------------------------------------*
 **                         EXTERNAL DECLARE                                 *
 **--------------------------------------------------------------------------*/
extern ANIM_COMPONENT_T g_anim_player;
extern ANIM_COMPONENT_T g_anim_render;
extern ANIM_COMPONENT_T g_anim_decoder;

ANIM_CONFIG_INFO_T g_anim_config = {
    E_ANIM_TYPE_DYNAMIC_MJPEG,//ANIM_TYPE_E type;
    1,//int display_num;
    //ANIM_DISPLAY_INFO_T display_info[10]; //max 10
    {
        //dw, dh, aw, ah, show_anim
        {1440, 540, 1440, 540, 1},
    },
};
/**--------------------------------------------------------------------------*
 **                         LOCAL DEFINITION                                 *
 **--------------------------------------------------------------------------*/
ANIM_INFO_T *s_anim_info_t = NULL;


/*---------------------------------------------------------------------------*/
/*                          LOCAL FUNCTION                                                          */
/*---------------------------------------------------------------------------*/

int preinit_anim_info(void) {
    s_anim_info_t = (ANIM_INFO_T *) malloc(sizeof(ANIM_INFO_T));
    memset(s_anim_info_t, 0, sizeof(ANIM_INFO_T));

    // init by malloc
    s_anim_info_t->a_data = (ANIM_DATA_T *) malloc(sizeof(ANIM_DATA_T));
    memset(s_anim_info_t->a_data, 0, sizeof(ANIM_DATA_T));

    s_anim_info_t->a_data->buf_info = (ANIM_BUFFER_INFO_T *) malloc(sizeof(ANIM_BUFFER_INFO_T));
    memset(s_anim_info_t->a_data->buf_info, 0, sizeof(ANIM_BUFFER_INFO_T));

    // init by project
    s_anim_info_t->a_player = &g_anim_player;
    s_anim_info_t->a_render = &g_anim_render;
    s_anim_info_t->a_decoder = &g_anim_decoder;
    s_anim_info_t->a_data->config_info = &g_anim_config;

    s_anim_info_t->a_data->state = E_ANIM_STATE_NONE;

    return 0;
}

int deinit_anim_info(void) {
    if (s_anim_info_t->a_data->buf_info) {
        free(s_anim_info_t->a_data->buf_info);
        s_anim_info_t->a_data->buf_info = NULL;
    }
    if (s_anim_info_t->a_data) {
        free(s_anim_info_t->a_data);
        s_anim_info_t->a_data = NULL;
    }
    if (s_anim_info_t) {
        free(s_anim_info_t);
        s_anim_info_t = NULL;
    }

    return 0;
}

static void animation_run(void *arg)
{
    int ret = 0;

    preinit_anim_info();

    s_anim_info_t->a_data->state = E_ANIM_STATE_PLAYING;

    ret = s_anim_info_t->a_player->c_init(s_anim_info_t);
    if (ret) {
        ssdk_printf(SSDK_INFO, "animation_run: player init fail\n");
        goto error;
    }
    ret = s_anim_info_t->a_player->c_work(s_anim_info_t);
    if (ret) {
        ssdk_printf(SSDK_INFO, "animation_run: player work fail\n");
        goto error;
    }

error:
    s_anim_info_t->a_data->state = E_ANIM_STATE_STOP;

    ret = s_anim_info_t->a_player->c_deinit(s_anim_info_t);
    if (ret) {
        ssdk_printf(SSDK_INFO, "animation_run: player deinit fail\n");
    }

    deinit_anim_info();
    osThreadExit();
}

void animation_entry()
{
#ifdef DEBUG_TIME_PRINT
    ssdk_printf(SSDK_CRIT, "animation start time %d\n", current_time());
#endif
    osThreadId_t animation_thread;
    osThreadAttr_t attr = {
        .name = "animation",
        .stack_size = 4096,
        .priority = osPriorityNormal
    };

    animation_thread = osThreadNew(animation_run, NULL, &attr);
}

static int animation_start(int argc, char *argv[]) {
    animation_entry();
    return 0;
}

CLI_CMD("animation_test", "\r\animation_test:\r\n Start animation app\r\n", animation_start);