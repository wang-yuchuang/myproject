/**
 * @file touch_event.h
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: Semidrive touch framework file
 *
 * Revision History:
 * -----------------
 */

#ifndef _TOUCH_EVENT_
#define _TOUCH_EVENT_

#include <driver/taishan/touch/touch_core.h>

#define TS_COORD_METADATA_SIZE sizeof(struct touch_coord_data)
#define TS_HEAD_METADATA_SIZE sizeof(struct touch_head_data)

struct touch_event {
    struct ts_coord_config coord_cfg;
    bool inited;
    int x;
    int y;
    bool pressed;
    int screen_id;
};

struct touch_head_data {
    uint8_t key_value;
    uint8_t touch_num;
};

struct touch_coord_data {
    uint8_t id;
    uint16_t x;
    uint16_t y;
    uint16_t w;
};

struct touch_report_data {
    struct touch_head_data head_data;
    struct touch_coord_data coord_data[10];
};

int touch_event_init_info(const struct ts_coord_config *coord_cfg,
                          int screen_id);
int touch_event_get_info(int *x_max, int *y_max,
                         int *x_off, int *y_off, int screen_id);
int touch_event_report(void *data, int len, int screen_id);
bool touch_event_get(int *x, int *y, int screen_id);

#endif //_TOUCH_EVENT_

