
//*****************************************************************************
//
// Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
// Software License Agreement
//
//*****************************************************************************

#ifdef CONFIG_TOUCH

#include <driver/taishan/touch/touch_core.h>
#include <driver/taishan/pinctrl/taishan_pinctrl.h>

extern i2c_adap_dev_t *i2c8_adap_dev;
extern gpio_dev_t g_gpio_ap;

static struct touch_device ts_dev[] = {
    {
        .enable = TS_ENABLE,
        .device_name = "goodix",
        .adap = &i2c8_adap_dev,
        .i2c_addr = 0x5d,
        .screen_id = 0,
        .rst_pin = GPIO_E22,
        .irq_pin = GPIO_E23,
        .rst_gpiodev = &g_gpio_ap,
        .irq_gpiodev = &g_gpio_ap,
        .coord_cfg.x_max = 1280,
        .coord_cfg.y_max = 768,
    },
    {
        .enable = TS_DISABLE,
        .device_name = "goodix",
        .adap = &i2c8_adap_dev,
        .i2c_addr = 0x14,
        .screen_id = 0,
        .rst_pin = 0,
        .irq_pin = 0,
        .rst_gpiodev = &g_gpio_ap,
        .irq_gpiodev = &g_gpio_ap,
        .coord_cfg.x_max = 0,
        .coord_cfg.y_max = 0,
    },
};

void board_touch_init(void)
{
    int num = sizeof(ts_dev) / sizeof(ts_dev[0]);
    touch_init(ts_dev, num);
}
#endif

