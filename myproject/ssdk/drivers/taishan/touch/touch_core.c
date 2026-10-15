/**
 * @file touch_core.c
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: Semidrive touch framework file
 *
 * Revision History:
 * -----------------
 */

#include <string.h>
#include "driver/taishan/touch/touch_core.h"
#include "touch_vendor.h"

static struct touch_driver touch_drv[] = {
#ifdef CONFIG_TOUCH_GOODIX
    {
        .driver_name = "goodix",
        .probe = goodix_probe_device,
    },
#endif
};

void touch_init(struct touch_device *ts_dev, int ts_dev_num)
{
    struct touch_driver *ts_drv = touch_drv;
    int ts_drv_num = sizeof(touch_drv) / sizeof(touch_drv[0]);

    for (int i = 0; i < ts_drv_num; i++) {
        if (!ts_drv[i].probe)
            continue;

        for (int j = 0; j < ts_dev_num; j++) {
            if (ts_dev[j].enable
                    && !strcmp(ts_drv[i].driver_name, ts_dev[j].device_name))
                ts_drv[i].probe(&ts_dev[j]);
        }
    }
}
