/**
 * @file disp_hal.h
 * @brief display subsystem header file.
 *
 * @copyright Copyright (c) 2019-2029 Semidrive Semiconductor.
 *            All rights reserved.
 */
#ifndef __DISP_HAL_H__
#define __DISP_HAL_H__
#include <disp_common.h>
#include <sdm_panel.h>
#include <disp_link_types.h>
#include <disp_drv.h>

#define PANEL_NOT_NEED_INIT 0

enum DISPLAY_SCREEN {
    NOT_FOUND = -1,
    CLUSTER,
    INFOTAINMENT,
    ENTERTAINMENT,
    CONTROLPANEL,
    HUD,
    SCREEN_MAX
};

enum DISPLAY_TYPE {
    DISPLAY_TYPE_INVALID = -1,
    DISPLAY_TYPE_MAX = 10,
};

enum {
    CLEAR_SPIPE_LAYER = 0x1,
    CLEAR_GPIPE_LAYER = 0x2,
    CLEAR_ALL_LAYERS =  0x3
};

enum {
    DIL2 = 0x0,
    RTOS = 0x1,
    SSDK = 0x2,
};

struct sdm_event {
    uint32_t type; //vsync, hotplug..
    uint32_t data;
    int64_t timestamp;
};

int hal_display_create_handle(display_handle **handle, enum DISPLAY_TYPE display);
void hal_display_release_handle(display_handle *handle);

// fill display_resource into handle res filed.
int hal_display_handle_install_resource(struct display_resource *res);
int hal_display_connect_all(struct display_resource *disp_res, int res_count, int os);
int hal_display_num(void);

int hal_sdm_callback_vsync_register(display_handle *handle, CALLBACK_EVENT_T func);
display_handle *hal_get_display_handle(enum DISPLAY_TYPE display);
display_handle *hal_get_display_handle_by_index(uint32_t index);

int hal_sdm_init(display_handle *handle);
int hal_sdm_deinit(display_handle *handle);
int hal_sdm_if_init(display_handle *handle, struct sdm_panel *panel);
int hal_sdm_clear_layers(display_handle * handle, uint8_t mask, uint8_t zorder);
int hal_sdm_post_config(display_handle *handle, struct sdm_post_config *post);
int hal_sdm_buffer_alloc(void);
int hal_sdm_buffer_free(void);
int hal_sdm_event_callback(display_handle *handle, int type, void *callback);
int hal_sdm_panel_connect(display_handle *handle, int sub_id, struct sdm_panel *panel);
int hal_sdm_panel_disconnect(display_handle *handle, int sub_id, struct sdm_panel *panel);
int hal_sdm_panel_type_check(display_handle *handle, struct sdm_panel *panels[], int n_panels);
int hal_sdm_set_pll_clk(display_handle *handle, uint32_t clk);
int hal_sdm_pq_config(display_handle *handle, struct sdm_pq_params *pq);
int disp_crc32_callback(int signal, void* args);
int hal_sdm_crc32_set(display_handle *handle, struct sdm_buffer *buf, struct crc_info *info);
int hal_sdm_crc32_disable(display_handle *handle);
u32 hal_sdm_crc32_get(display_handle *handle, uint8_t index);
int hal_sdm_safety_config(display_handle *handle, struct sdm_buffer *buf);
void hal_sdm_safety_exit(display_handle *handle);
int hal_sdm_dcrecovery(const char *arg, uint16_t rec_id);
#endif // __DISP_HAL_H__
