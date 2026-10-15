/**
 * @file lv_port_disp_templ.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "lvgl_gui.h"
#ifdef CONFIG_LVGL_SDRV_DISPLAY
#include "lvgl_disp_drv.h"
#endif
#ifdef CONFIG_LVGL_SDRV_FAT_FS
#include "lv_fs_sdrv.h"
#endif
#ifdef CONFIG_LVGL_SDRV_INPUT
#include "lv_port_indev.h"
#endif

#include <string.h>
#include "driver/timer.h"
//#include <cmsis_os2.h>
#include <board.h>
#include <debug.h>

#ifdef CONFIG_LVGL_SDRV_DECODER_PNG
#include "lv_png.h"
#endif
#ifdef CONFIG_LVGL_SDRV_DECODER_JPG
#include "lv_sjpg.h"
#endif

#define OS_TIMER_LEN            5 //s

static uint32_t last_tick = 0;

static void lv_tick_timercb(void *arg)
{
    uint32_t now = current_time();
    //ssdk_printf(SSDK_INFO, "timer cb %d\n", now);
    lv_tick_inc(now - last_tick);

    last_tick = now;
    return;
}

void lvgl_refresh_task(lv_task_t * task)
{
    lv_task_handler();
}

void lvgl_mainloop(void)
{
    while(1)
    {
        //ssdk_printf(SSDK_INFO, "mainloop ... %d", osKernelGetTickFreq());
        lv_task_handler();
        osDelay(50);
    }
}

lv_disp_t *g_registered_display[2] = {0};

lv_disp_t *get_display(int display_id)
{
    return g_registered_display[display_id];
}

void lvgl_init(void)
{
    static osTimerId_t lv_tick_timer;
    static int lvgl_inited = 0;
    osStatus_t status;
    ssdk_printf(SSDK_INFO, "lvgl_init start\n");
    if (lvgl_inited)
    {
        ssdk_printf(SSDK_INFO, "lvgl_init already！！\n");
        return;
    }

    lv_init();
#ifdef CONFIG_LVGL_SDRV_FAT_FS
    ssdk_printf(SSDK_ALERT, "init lvgl for fat fs \n");
    lv_port_fs_init();
    ssdk_printf(SSDK_ALERT, "init lvgl for fat fs end\n");
#endif
#ifdef CONFIG_LVGL_SDRV_DISPLAY
    ssdk_printf(SSDK_ALERT, "init lvgl for disp \n");
    lv_disp_t *disp = lvgl_lcd_display_init();
    g_registered_display[0] = disp;
    ssdk_printf(SSDK_ALERT, "init lvgl for disp end %p\n", disp);
#endif
#ifdef CONFIG_LVGL_SDRV_INPUT
    lv_port_indev_init(get_display(0));
#endif
#ifdef CONFIG_LVGL_SDRV_DECODER_PNG
    lv_png_init();
#endif
#ifdef CONFIG_LVGL_SDRV_DECODER_JPG
    lv_split_jpeg_init();
#endif

    lv_tick_timer = osTimerNew(lv_tick_timercb, osTimerPeriodic, NULL, NULL);
    if (!lv_tick_timer) {
        ssdk_printf(SSDK_INFO, "create timer fail\n");
    }

    status = osTimerStart(lv_tick_timer, OS_TIMER_LEN*osKernelGetTickFreq());
    if (status != osOK) {
        ssdk_printf(SSDK_INFO, "start timer fail\n");
        osTimerDelete(lv_tick_timer);
    }

    lvgl_inited = 1;
    ssdk_printf(SSDK_ALERT, "lvgl_init end\n");
}
