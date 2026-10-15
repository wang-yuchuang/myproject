/**
 * @file touch_event.c
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: Semidrive touch framework file
 *
 * Revision History:
 * -----------------
 */

#include <debug.h>
#include <stdio.h>
#include <types.h>
#include <stdlib.h>
#include <string.h>
#include <driver/taishan/touch/touch_core.h>
#include <driver/taishan/touch/touch_event.h>

//#define TOUCH_EVENT_DBG
#define TOUCH_NUM 2

static struct touch_event ts_event[TOUCH_NUM];

int touch_event_init_info(const struct ts_coord_config *coord_cfg, int screen_id)
{
    if (screen_id >= TOUCH_NUM) {
        ssdk_printf(SSDK_ERR, "%s, invalid screen id\n", __func__);
        return -1;
    }

    ts_event[screen_id].coord_cfg.x_max = coord_cfg->x_max;
    ts_event[screen_id].coord_cfg.y_max = coord_cfg->y_max;
    ts_event[screen_id].coord_cfg.x_offset = coord_cfg->x_offset;
    ts_event[screen_id].coord_cfg.y_offset = coord_cfg->y_offset;
    ts_event[screen_id].coord_cfg.swap_xy = coord_cfg->swap_xy;
    ts_event[screen_id].coord_cfg.invert_x = coord_cfg->invert_x;
    ts_event[screen_id].coord_cfg.invert_y = coord_cfg->invert_y;
    ts_event[screen_id].inited = true;
    return 0;
}

int touch_event_get_info(int *x_max, int *y_max, int *x_off, int *y_off,
                         int screen_id)
{
    if (screen_id >= TOUCH_NUM || !ts_event[screen_id].inited) {
        ssdk_printf(SSDK_ERR, "%s, invalid screen id or not inited\n", __func__);
        return -1;
    }

    *x_max = ts_event[screen_id].coord_cfg.x_max;
    *y_max = ts_event[screen_id].coord_cfg.y_max;
    *x_off = ts_event[screen_id].coord_cfg.x_offset;
    *y_off = ts_event[screen_id].coord_cfg.y_offset;
    return 0;
}

int touch_event_report(void *data, int len, int screen_id)
{
    int x, y;
    struct touch_report_data *report_data = data;

    if (screen_id < 0 || screen_id >= TOUCH_NUM || !ts_event[screen_id].inited) {
        ssdk_printf(SSDK_ERR, "invalid screen id %d, or not inited\n", screen_id);
        return 0;
    }

    if (len <= TS_HEAD_METADATA_SIZE) {
        ts_event[screen_id].pressed = false;
        return 0;
    }

    for (int i = 0; i < 1; i++) {
        x = report_data->coord_data[i].x - ts_event[screen_id].coord_cfg.x_offset;
        y = report_data->coord_data[i].y - ts_event[screen_id].coord_cfg.y_offset;

        if (x < 0 || x > ts_event[screen_id].coord_cfg.x_max ||
                y < 0 || y > ts_event[screen_id].coord_cfg.y_max)
            return 0;

        ts_event[screen_id].x = x;
        ts_event[screen_id].y = y;
        ts_event[screen_id].screen_id = screen_id;
        ts_event[screen_id].pressed = true;
    }

#ifdef TOUCH_EVENT_DBG
    ssdk_printf(SSDK_ERR, "%s: x=%d, y=%d, screen_id=%d, pressed=%d\n",
                __func__, ts_event[screen_id].x, ts_event[screen_id].y,
                ts_event[screen_id].screen_id, ts_event[screen_id].pressed);
#endif
    return 0;
}

bool touch_event_get(int *x, int *y, int screen_id)
{
    if (screen_id >= TOUCH_NUM) {
        ssdk_printf(SSDK_ERR, "%s, invalid screen id\n", __func__);
        return false;
    }

    *x = ts_event[screen_id].x;
    *y = ts_event[screen_id].y;
    return ts_event[screen_id].pressed;
}
