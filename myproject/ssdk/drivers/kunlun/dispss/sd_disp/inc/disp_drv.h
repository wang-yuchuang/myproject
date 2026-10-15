/**
 * @file disp_drv.h
 * @brief display subsystem header file.
 *
 * @copyright Copyright (c) 2019-2029 Semidrive Semiconductor.
 *            All rights reserved.
 */
#ifndef __DISP_DRV_H__
#define __DISP_DRV_H__
#include "disp_common.h"

typedef int(*CALLBACK_EVENT_T)(int display_id, int64_t timestamp);

struct sdm_modeinfo {
    uint32_t width;
    uint32_t height;
    uint32_t format;
    uint32_t vrefresh;
    struct sis_info rtos_screen;
};

typedef struct mod_res_t {
    uint32_t res_id;
    paddr_t base;
    int index;
    int irqs[4]; //reset 0
} mod_res_t;

struct sdm_domain_res
{
    int num_dp;
    mod_res_t *dp;
    int num_dc;
    mod_res_t *dc;
    int num_intf;
    mod_res_t *intf;
};

typedef struct display_instance {
    int display_id;
    const char *name;
    struct display_resource *res;
    struct sdm_domain_res *dm;
    int panel_num;
    struct sdm_panel_info *panels_info[4];
    //struct sdm_panel *panels[4];
    //struct serdes_info *serdes[4];
    //struct backlight_info *bl_info[4];
    int found_panel;
    struct sdm_modeinfo info;
    CALLBACK_EVENT_T vsync;
    CALLBACK_EVENT_T hotplug;
    int os_type;
    bool inited;
    bool panel_post_end_done;
    bool is_need_register_irq;
} display_handle;

extern int disp_init(display_handle *handle);
extern int disp_uninit(display_handle *handle);
extern int disp_clear_layers(display_handle *handle, uint8_t mask, uint8_t z_order);
extern int disp_post_config(display_handle *handle,
    struct sdm_post_config *bufs);
extern int disp_panel_disconnect(display_handle *handle, int sub_id,
            struct sdm_panel *panel);
extern bool disp_panel_type_check(display_handle *handle, struct sdm_panel *panel);
extern int panel_init(display_handle *handle, struct sdm_panel *panel);
extern int display_if_init(display_handle *handle, struct sdm_panel *panel);
extern int display_if_deinit(display_handle *handle, struct sdm_panel *panel);
int callback_vsync_register(display_handle *handle, CALLBACK_EVENT_T func);
extern int disp_pq_config(display_handle *handle, struct sdm_pq_params *pq);
extern unsigned int disp_crc32_status(int display_id);
extern int disp_crc32_set(display_handle *handle, struct sdm_buffer *buf, struct crc_info *info);
extern int disp_crc32_disable(display_handle *handle);
u32 disp_crc32_get_value(display_handle *handle, uint8_t index);
extern int disp_safety_config(display_handle *handle, struct sdm_buffer *buf);
extern void disp_safety_exit(display_handle *handle);
extern void disp_dump_register(display_handle *handle);
#endif
