/**
 * @file sdm_display.h
 * @brief display subsystem header file.
 *
 * @copyright Copyright (c) 2019-2029 Semidrive Semiconductor.
 *            All rights reserved.
 */
#ifndef SDM_DISPLAY_H
#define SDM_DISPLAY_H

#ifdef __cplusplus
extern "C" {
#endif
#include "disp_hal.h"
#include <disp_panels.h>
#include <lib/list.h>

#ifdef CONFIG_DISP_CLIENT
#include <disp_client.h>
#endif

typedef struct {
    struct list_node node;
    int id;
    display_handle *handle;
} sdm_display_t;
int sdm_check_display_panel_num(display_handle *handle);
int sdm_init(void);

/**
 *@mask:It is a flag, intend to tell drv which layer we want to close.
 *CLEAR_SPIPE_LAYER, CLEAR_GPIPE_LAYER, CLEAR_ALL_LAYERS.
 *@zorder:The layer's current zorder.
 */
static inline int sdm_clear_layer(display_handle *handle, uint8_t mask, uint8_t zorder)
{
    return hal_sdm_clear_layers(handle, mask, zorder);
}

static inline int sdm_clear_display(display_handle *handle)
{
#ifdef CONFIG_DISP_CLIENT
    disp_client_clear(handle);
#endif
    return hal_sdm_clear_layers(handle, CLEAR_ALL_LAYERS, 0);
}

static inline int sdm_post(display_handle *handle, struct sdm_post_config *post)
{
    int ret;
    struct sdm_panel *panel = NULL;

#ifdef CONFIG_DISP_CLIENT
    if (post->post_noblock)
        ret = disp_client_add(handle, post);
    else
#endif
        ret = hal_sdm_post_config(handle, post);

    if (handle->panel_post_end_done)
        goto out;

    int found_panel = handle->found_panel;
    panel = handle->panels_info[found_panel]->panel;

    if (panel->panel_post_end) {
        panel->panel_post_end(handle->panels_info[found_panel]);
        handle->panel_post_end_done = true;
    }

out:
    return ret;
}

static inline int sdm_callback_vsync_register(display_handle *handle, CALLBACK_EVENT_T func)
{
    return hal_sdm_callback_vsync_register(handle, func);
}

static inline int sdm_pq_set(display_handle *handle, struct sdm_pq_params *pq)
{
    return hal_sdm_pq_config(handle, pq);
}
struct list_node *sdm_get_display_list(void);

int sdm_group_display_init_default(void);
int sdm_group_display_register(sdm_display_t *sdm);
int sdm_group_display_post(struct sdm_post_config *post);

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif /*SDM_DISPLAY_H*/