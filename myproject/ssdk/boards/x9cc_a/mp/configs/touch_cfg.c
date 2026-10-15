/**
 * @file touch_cfg.h
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: touch device config implement file
 *
 * Revision History:
 * -----------------
*/
#ifdef CONFIG_TOUCH_KUNLUN_PLATFORM
#include <types.h>

#include <driver/kunlun/touch/touch.h>
#include <driver/pinctrl.h>
#include <driver/kunlun/touch/touch_vendor.h>

#include "i2c_cfg.h"
#include "serdes_cfg.h"
#include "pinmux_cfg.h"
#include "touch_cfg.h"

touch_dev_t* g_touch_dev[SCREEN_MAX];

static sdrv_touch_handle s_touch_drv[SCREEN_MAX];

/** android main touch config */
static touch_config_t s_android_main_cfg = {
    .screen_id = INFOTAINMENT,
    .enable = true,
    .name = "cyttsp6",
    .i2c_addr = 0x24,
    .i2c_dev = &g_i2c16_adap_dev,
    .coord_config = {
        .x_max = 2560,
        .y_max = 1600,
        .max_touch_num = 10,
    },
    .serdes_config = {
        .enable = true,
        .irq_pin = 7,
        .reset_pin = 8,
        .sirq_pin = 7,
        .serdes = &g_serdes_dev[SERDES_DEV_DSI1]
    },
    .irq_pin = {
        .pin_num = 233,
        .pin_dev = &g_gpio_dev,
    },
    .probe = cyttsp6_touch_probe
};

/** android aux1 config */
static touch_config_t s_android_aux1_cfg = {
    .screen_id = CLUSTER,
    .enable = true,
    .name = "goodix_aux1",
    .i2c_addr = 0x5d,
    .i2c_dev = &g_i2c14_adap_dev,
    .coord_config = {
        .x_max = 1920,
        .y_max = 720,
        .max_touch_num = 10,
    },
    .serdes_config = {
        .enable = true,
        .irq_pin = 4,
        .reset_pin = 5,
        .sirq_pin = 4,
        .serdes = &g_serdes_dev[SERDES_DEV_LVDS12],
    },
    .irq_pin = {
        .pin_num = 148,
        .pin_dev = &g_gpio_dev,
    },
    .probe = goodix_touch_probe
};

static touch_config_t* s_touch_cfg_tbl[SCREEN_MAX][5] = {
    [CLUSTER] = {
        &s_android_aux1_cfg,
        NULL
    },
    [INFOTAINMENT] = {
        &s_android_main_cfg,
        NULL
    },
    [ENTERTAINMENT] = {
        NULL
    },
    [CONTROLPANEL] = {
        NULL
    },
    [HUD] = {
        NULL
    }
};

int32_t board_touch_init()
{
    touch_dev_t* dev = NULL;

    for (int32_t touch_cnt = 0; touch_cnt < SCREEN_MAX; touch_cnt++) {

        uint8_t cfg_cnt = 0;
        dev = NULL;

        while (cfg_cnt < 5 &&
                s_touch_cfg_tbl[touch_cnt][cfg_cnt] != NULL) {
            if (s_touch_cfg_tbl[touch_cnt][cfg_cnt]->probe != NULL) {
                dev = s_touch_cfg_tbl[touch_cnt][cfg_cnt]->probe(
                        &s_touch_drv[touch_cnt]
                        , s_touch_cfg_tbl[touch_cnt][cfg_cnt]);
                if (dev != NULL)
                    break;
            }
            cfg_cnt++;
        }

        g_touch_dev[touch_cnt] = dev;
    }

    return 0;
}
#endif /* end CONFIG_TOUCH_KUNLUN_PLATFORM */
/* end of file touch_cfg.c */

