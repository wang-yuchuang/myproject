
//*****************************************************************************
//
// WARNING: Automatically generated file, don't modify anymore!!!
//
// Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
// Software License Agreement
//
//*****************************************************************************

#include "dispss_cfg.h"

#include <types.h>
#include <board.h>

/* dc device driver headers */
#include <driver/taishan/dispss/disp.h>
#include <driver/taishan/pinctrl/taishan_pinctrl.h>
#include <driver/gpio.h>
/* dc device drivers */
struct dc_dev g_dc_dev;

extern gpio_dev_t g_gpio_safety;

static void mdelay(int ms)
{
    udelay(ms*1000);
}

static void set_backlight_on(void)
{
    unsigned int pin = GPIO_M8; /* GPIO_E24 */
    gpio_dev_t *gpio_dev = &g_gpio_safety;

    mdelay(300); //AUO C123 POWER ON TIMING: LVDS_ON --> BL(>=300ms)

    gpio_set(gpio_dev, pin, 1);
}
/* dc config structures */
static const struct dc_config dc_cfg = {
    .base = DEVICE_BASE(DC),
    .irq = DEVICE_INTR(DC_DC_IRQ1),
};

/* dc initialization */
void board_dispss_init(void)
{
    sdrv_dc_probe(&g_dc_dev, &dc_cfg);
    sdrv_display_init(&g_dc_dev);
    //sdrv_display_logo(&g_dc_dev, 0);
    set_backlight_on();
}

