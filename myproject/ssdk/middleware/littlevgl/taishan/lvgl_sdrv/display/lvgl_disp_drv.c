/*
* lvgl_disp_drv.c
*
* Copyright (c) 2019 Semidrive Semiconductor.
* All rights reserved.
*
* Description:
*
* Revision History:
* -----------------
* 011, 07/13/2019 BI create this file
*/
#include <cmsis_os2.h>
#include <debug.h>
#include "lvgl_disp_drv.h"
#include <driver/taishan/dispss/disp.h>
#include <driver/taishan/dispss/disp_data_type.h>
#include <debug.h>
#include <arch/cache.h>
#if WITH_KERNEL_VM
#include <kernel/vm.h>
#endif

#define SCREEN_MAX 3

#define LOGO_LAYER         0
#define BOOT_MENU_LAYER    1

#if WITH_KERNEL_VM
#define v2p(va)    (paddr_t)(vaddr_to_paddr(va))
#define p2v(pa)    (vaddr_t)(paddr_to_kvaddr(pa))
#else
#define v2p(va)    (paddr_t)(va)
#define p2v(pa)    (vaddr_t)(pa)
#endif
#ifdef CONFIG_LVGL_SDRV_DISPLAY
extern struct dc_dev g_dc_dev;
#endif
#if LV_USE_GPU_SEMIDRIVE_G2D
extern struct g2dlite_device g2dlite_dev;
#endif
unsigned int sd_get_color_format(void)
{
    unsigned int format;
#if LV_COLOR_DEPTH == 16
    format = COLOR_RGB565;
#elif LV_COLOR_DEPTH == 32
    format = COLOR_ARGB8888;
#else
#error "Invalid LV_COLOR_DEPTH in lv_conf.h! Set it to 16 or 32!"
#endif

    return format;
}

unsigned char sd_get_bpp_by_format(unsigned int format)
{
    unsigned char bpp = 0;

    switch (format) {
        case COLOR_RGB565:
            bpp = 2;
            break;
        case COLOR_ARGB8888:
            bpp = 4;
            break;
        default:
            ssdk_printf(SSDK_INFO, "Not support this format\n");
            break;
    }
    return bpp;
}

int area_valid_check(const lv_area_t * area)
{
    int lcd_w, lcd_h;

    lcd_w = lv_area_get_width(area);
    lcd_h = lv_area_get_height(area);

    ssdk_printf(SSDK_INFO, "lcd_w = %d, lcd_h = %d\n", lcd_w, lcd_h);
    ssdk_printf(SSDK_INFO, "area x1 = %d, area y1 = %d\n", area->x1, area->y1);
    ssdk_printf(SSDK_INFO, "area x2 = %d, area y2 = %d\n", area->x2, area->y2);
    if ((area->x2 > lcd_w) || (area->y2 > lcd_h)) {
        ssdk_printf(SSDK_INFO, "area size is over lcd size\n");
        return -1;
    }

    return 0;
}

int sd_disp_post(struct _disp_drv_t * disp_drv, const lv_area_t * area,
                lv_color_t *buf_bottom, lv_color_t *buf_top,
                uint32_t length, lv_opa_t opa) {
    struct post_cfg post;
    int i;
    int n_layers;
    lv_color_t *bufs[] = {buf_bottom, buf_top};
    ssdk_printf(SSDK_INFO, "sd_disp_post start\n");

    struct disp_data *data = (struct disp_data*)disp_drv->user_data;

    memset(&post, 0, sizeof(struct post_cfg));
    if (!buf_bottom) {
        ssdk_printf(SSDK_ERR, "Error: buf1 is null\n");
        return -3;
    }
    n_layers = 1;
    if (buf_top) {
        n_layers ++;
    }
    post.layers = (struct surface*)osAlloc(sizeof(struct surface) * n_layers);
    if (!post.layers) {
        ssdk_printf(SSDK_ERR, "Error: malloc surface failed\n");
        return -2;
    }

    post.num_layers = n_layers;
    for (i = 0; i < n_layers; i++) {
        struct surface *layer = &post.layers[i];
        layer->addr[0] = (unsigned long)v2p((void *)bufs[i]);
        if (i == 1)
            layer->alpha = opa;
        else
            layer->alpha = 0xff;
        layer->id = 0;
        layer->dirty = 1;
        layer->alpha_en = 0;// for bg transp 1->0
        layer->ckey = 0;
        layer->ckey_en = 0;
        layer->fmt = sd_get_color_format();
        layer->en = 1;
        layer->src.x = area->x1;
        layer->src.y = area->y1;
        layer->src.w = lv_area_get_width(area);
        layer->src.h = lv_area_get_height(area);

        layer->start.x = area->x1;
        layer->start.y = area->y1;
        layer->start.w = lv_area_get_width(area);
        layer->start.h = lv_area_get_height(area);
        layer->src_stride[0] = lv_area_get_width(area) * sd_get_bpp_by_format(layer->fmt);

        // dc do not support scaling.
        layer->dst.x = area->x1 + LVGL_DISPLAY_OFFSET_X;
        layer->dst.y = area->y1 + LVGL_DISPLAY_OFFSET_Y;
        layer->dst.w = lv_area_get_width(area);
        layer->dst.h = lv_area_get_height(area);
        layer->z_order = 1;

        ssdk_printf(SSDK_INFO, "post_param: %p,0x%x,%d,%d,%d,%d,%d,strid=%d\n", (void *)layer->addr[0],
            layer->alpha, layer->fmt, layer->src.x, layer->src.y, layer->src.w,
            layer->src.h, layer->src_stride[0]);
    }

    sdrv_display_post(data->dc, &post);

    osFree(post.layers);
    ssdk_printf(SSDK_INFO, "sd_disp_post end\n");
    return 0;
}
#if 0
void sd_gpu_blend(lv_disp_drv_t * disp_drv, lv_color_t * dest, const lv_color_t * src, uint32_t length, lv_opa_t opa) {
    struct disp_data *data = (struct disp_data*)disp_drv->user_data;
    struct g2dlite_input input;
    ssdk_printf(SSDK_INFO, "gpu blend......: length %d\n", length);

    memset(&input, 0, sizeof(struct g2dlite_input));
    input.layer_num = 2;
    #if LV_COLOR_DEPTH == 32
    for (int i = 0; i < input.layer_num; i++) {
        input.layer[i].layer = i;
        input.layer[i].layer_en = 1;

        input.layer[i].fmt = COLOR_ARGB8888;
        input.layer[i].zorder = i;
        input.layer[i].src.x = 0;
        input.layer[i].src.y = 0;
        input.layer[i].src.w = length;
        input.layer[i].src.h = 1;

        input.layer[i].addr[0] = i == 0? (unsigned long)dest: (unsigned long)src;

        input.layer[i].src_stride[0] = length * 4;
        input.layer[i].dst.x = 0;
        input.layer[i].dst.y = 0;
        input.layer[i].dst.w = length;
        input.layer[i].dst.h = 1;
        input.layer[i].ckey.en = 0;
        input.layer[i].blend = BLEND_PIXEL_COVERAGE;
        input.layer[i].alpha = opa;
    }

    input.output.width = length;
    input.output.height = 1;
    input.output.fmt = COLOR_ARGB8888;
    input.output.addr[0] = (unsigned long)dest;
    input.output.stride[0] = length * 4;
    input.output.rotation = 0;
    hal_g2dlite_blend(data->g2d, &input);
    #endif

}
#endif
void disp_flush(struct _disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p)
{
#if LV_USE_USER_DATA
//    struct disp_data *data = (struct disp_data*)disp_drv->user_data;
#endif
    ssdk_printf(SSDK_INFO, "disp_flush start\n");
    // arch_clean_cache_range((addr_t)color_p, disp_drv->hor_res * 4);
    arch_clean_invalidate_cache_range((addr_t)color_p, disp_drv->hor_res * disp_drv->ver_res * 4);

    sd_disp_post(disp_drv, area, color_p, NULL, 0, 0xff);

    lv_disp_flush_ready(disp_drv);
    ssdk_printf(SSDK_INFO, "disp_flush end\n");
}

/** OPTIONAL: Called after every refresh cycle to tell the rendering and flushing time + the
* number of flushed pixels */
static void sd_monitor_cb(struct _disp_drv_t * disp_drv, uint32_t time, uint32_t px)
 {
    if (time % 1000 == 5)
        ssdk_printf(SSDK_INFO, "time: %d, time %d\n", lv_tick_get(), time);
 }

#if LV_USE_GPU
/* If your MCU has hardware accelerator (GPU) then you can use it to blend to memories using opacity
 *  * It can be used only in buffered mode (LV_VDB_SIZE != 0 in lv_conf.h)*/
static void gpu_blend(lv_disp_drv_t * disp_drv, lv_color_t * dest, const lv_color_t * src, uint32_t length, lv_opa_t opa)
{
    /*It's an example code which should be done by your GPU*/
#if 0
    uint32_t i;
    // ssdk_printf(SSDK_INFO, "gpu blend......: length %d\n", length);
    for(i = 0; i < length; i++) {
        dest[i] = lv_color_mix(dest[i], src[i], opa);
    }
    arch_clean_cache_range((addr_t)dest, disp_drv->hor_res * 4);
#else
    sd_gpu_blend(disp_drv, dest, src, length, opa);
#endif
}

/* If your MCU has hardware accelerator (GPU) then you can use it to fill a memory with a color
 *  * It can be used only in buffered mode (LV_VDB_SIZE != 0 in lv_conf.h)*/
static void gpu_fill_cb(lv_disp_drv_t * disp_drv, lv_color_t * dest_buf, lv_coord_t dest_width,
                        const lv_area_t * fill_area, lv_color_t color)
{
    /*It's an example code which should be done by your GPU*/
    lv_coord_t x, y;
    // ssdk_printf(SSDK_INFO, "gpu fill.....(%d, %d, %d, %d).\n", fill_area->x1, fill_area->y1, fill_area->x2, fill_area->y2);
    lv_color_t *p = dest_buf;
    dest_buf += dest_width * fill_area->y1; /*Go to the first line*/
    // arch_clean_cache_range((addr_t)dest_buf, dest_width * (fill_area->y2 - fill_area->y1) * 4);
    for(y = fill_area->y1; y < fill_area->y2; y++) {
        for(x = fill_area->x1; x < fill_area->x2; x++) {
            dest_buf[x] = color;
            // dest_buf[x] = lv_color_mix(LV_COLOR_ORANGE, color , LV_OPA_100);
        }
        dest_buf+=dest_width;    /*Go to the next line*/
    }

}

#endif

struct g2dlite {
    int index;
    addr_t reg_addr;
    uint32_t irq_num;
};

lv_disp_t *lvgl_lcd_display_init()
{
    //int ret;
    struct disp_data *data;

    int width = LVGL_DISPLAY_WIDTH;
    int height = LVGL_DISPLAY_HEIGHT;

    ssdk_printf(SSDK_INFO, "lvgl_lcd_display_init start\n");
#ifdef USE_STATIC_ADDRESS
    lv_color_t *buf3_1 = (lv_color_t *)0x32000000;
    lv_color_t *buf3_2 = (lv_color_t *)0x33000000;
#else
    lv_color_t *buf3_1 = (lv_color_t *)osAllocAlign(0x1000, width * height * LV_COLOR_DEPTH / 8);
    lv_color_t *buf3_2 = (lv_color_t *)osAllocAlign(0x1000, width * height * LV_COLOR_DEPTH / 8);
#endif
    memset(buf3_1, 0, width * height * LV_COLOR_DEPTH / 8);
    memset(buf3_2, 0, width * height * LV_COLOR_DEPTH / 8);

    lv_disp_buf_t *disp_buf_3 = osAlloc(sizeof(lv_disp_buf_t));

    lv_disp_buf_init(disp_buf_3, buf3_1, buf3_2, width * height);   /*Initialize the display buffer*/
    ssdk_printf(SSDK_ALERT, "register buffers: %p ,%p for disp\n", buf3_1, buf3_2);
    lv_disp_drv_t disp_drv;                         /*Descriptor of a display driver*/
    lv_disp_drv_init(&disp_drv);                    /*Basic initialization*/

    data = (struct disp_data *) osAlloc(sizeof(struct disp_data));
    if (!data) {
        ssdk_printf(SSDK_ALERT, "create disp_data failed\n");
    }
    /*Set up the functions to access to your display*/

    /*Set the resolution of the display*/
    disp_drv.hor_res = width;
    disp_drv.ver_res = height;

    /*Used to copy the buffer's content to the display*/
    disp_drv.flush_cb = disp_flush;
    disp_drv.monitor_cb = sd_monitor_cb;

    /*Set a display buffer*/
    disp_drv.buffer = disp_buf_3;
#ifdef CONFIG_LVGL_SDRV_DISPLAY
    data->dc = (void *)&g_dc_dev;
#endif
#if LV_USE_GPU_SEMIDRIVE_G2D
    data->g2d = (void *)&g2dlite_dev;
#endif
    disp_drv.user_data = data;

#if LV_USE_GPU
    /*Optionally add functions to access the GPU. (Only in buffered mode, LV_VDB_SIZE != 0)*/

    /*Blend two color array using opacity*/
    disp_drv.gpu_blend_cb = gpu_blend;

    /*Fill a memory array with a color*/
    disp_drv.gpu_fill_cb = gpu_fill_cb;
#endif
    /*Finally register the driver*/
    lv_disp_t *disp = lv_disp_drv_register(&disp_drv);
    ssdk_printf(SSDK_INFO, "lvgl_lcd_display_init end %p\n",disp);
    return disp;
}


