/**
 * @file sdm_display.c
 * @brief display subsystem source file.
 *
 * @copyright Copyright (c) 2019-2029 Semidrive Semiconductor.
 *            All rights reserved.
 */
#include <string.h>
#include "sdm_display.h"
#include "disp_data_type.h"
#include <disp_common.h>
#include <disp_drv_log.h>
#include <lib/list.h>
#include <stdlib.h>
#include <debug.h>
#include "disp_cfg.h"
#include <disp_errcode.h>

static int is_inited = false;

struct list_node g_disp_list;
struct list_node g_group_disp_list;

int sdm_display_change_clk(display_handle *handle, int fps)
{
    int ret = 0;
    uint32_t h_total = 0;
    uint32_t v_total = 0;
    uint8_t dual_mode = 0;

    struct sdm_panel *panel = handle->panels_info[handle->found_panel]->panel;

    dual_mode = (handle->res->lvds_mode == LVDS_MODE_DUAL) ? 2 : 1;

    h_total = panel->display_timing->hactive + panel->display_timing->hback_porch +
              panel->display_timing->hfront_porch + panel->display_timing->hsync_len;
    v_total = panel->display_timing->vactive + panel->display_timing->vback_porch +
              panel->display_timing->vfront_porch + panel->display_timing->vsync_len;

    if (handle->res->clk == 0) {
        handle->res->clk = h_total * v_total * fps * 7 / dual_mode;
        panel->display_timing->pll_clk = handle->res->clk;
    }

    ret = hal_sdm_set_pll_clk(handle, handle->res->clk);

    if ((panel->if_type == IF_TYPE_DSI) && panel->re_if_init) {
        panel->mipi->phy_freq = h_total * v_total * panel->fps / (panel->mipi->lane_number * 1000) *
            panel->mipi->video_bus_width * 1.1;

        DISPMSG("phy_freq:%d\n", panel->mipi->phy_freq);
    }

    DISPMSG("clk:%d\n", handle->res->clk);

    return ret;
}

static int sdm_display_handle_init(display_handle *handle,
    struct sdm_panel_info *panels[], int n_panels)
{
    int ret = 0;
    int found_panel = -1;
    int num_panel = 0;
    int i;
    struct sdm_panel *panel = NULL;

    num_panel = sdm_check_display_panel_num(handle);

    for (i = 0; i < num_panel; i++) {
        found_panel = sdm_panel_probe(handle, i, panels, n_panels);
        if (found_panel < 0) {
            DISPERR("get panel failed\n");
            disp_set_errcode(DISP_ERRCODE_ID,
                handle->display_id, DISP_ERRCODE_PROBE, 0);
            ret = -2;
            goto PANEL_FAIL;
        }

        panel = panels[found_panel]->panel;
        DISPMSG("disp[%d] %s init\n", handle->display_id, panel->panel_name);

        handle->panels_info[i] = panels[found_panel];
        handle->found_panel = found_panel;
        handle->info.width = panel->display_timing->hactive;
        handle->info.height = panel->display_timing->vactive;
        handle->info.format = 0;
        handle->info.vrefresh = 60;


        if (panel->display_timing->pll_clk)
            handle->res->clk = panel->display_timing->pll_clk;
        else if (panel->if_type == IF_TYPE_DSI)
            panel->re_if_init = true;

        if (panel->rtos_screen) {
            handle->info.rtos_screen.height = panel->rtos_screen->height;
            handle->info.rtos_screen.width = panel->rtos_screen->width;
            handle->info.rtos_screen.pos_x = panel->rtos_screen->pos_x;
            handle->info.rtos_screen.pos_y = panel->rtos_screen->pos_y;
        } else {
            handle->info.rtos_screen.height = handle->info.height;
            handle->info.rtos_screen.width = handle->info.width;
            handle->info.rtos_screen.pos_x = 0;
            handle->info.rtos_screen.pos_y = 0;
        }
    }

    if (handle->os_type == DIL2) {
        if (panel->panel_pre_power_on)
             panel->panel_pre_power_on(panels[found_panel]);

        hal_sdm_if_init(handle, panel);

        if (panel->panel_post_begin)
             panel->panel_post_begin(panels[found_panel]);

        ret = sdm_display_change_clk(handle, panel->fps);
        if (ret < 0) {
            disp_set_errcode(DISP_ERRCODE_ID,
                handle->display_id, DISP_ERRCODE_SETCLK, 0);
        }

        if (panel->re_if_init)
            hal_sdm_if_init(handle, panel);
    }

#if (defined(V_DEFAULT) || defined(V9TS_DEFAULT) || defined(G9V_DEFAULT))
    handle->is_need_register_irq = false;
#elif defined(TARGET_REFERENCE_X9U_B)
        handle->is_need_register_irq = true;
#else
    if(handle->display_id == 4)
        handle->is_need_register_irq = false;
    else
        handle->is_need_register_irq = true;
#endif

    if (hal_sdm_init(handle)) {
        DISPERR("hal_sdm_init failed\n");
        goto PANEL_FAIL;
    }

    sdm_display_t *d = (sdm_display_t *)malloc(sizeof(sdm_display_t));
    d->id = handle->display_id;
    d->handle = handle;
    list_add_tail(&g_disp_list, &d->node);
    return 0;

PANEL_FAIL:
    DISPERR("TODO: we need uninit panel here\n");
HANDLE_FAIL:
    hal_display_release_handle(handle);
    return ret;
}

int sdm_check_display_panel_num(display_handle *handle) {
    return handle->res->lvds_mode == LVDS_MODE_DUPLICATE? 2: 1;
}

int sdm_init(void) {
    list_initialize(&g_disp_list);
    return 0;
}

struct list_node *sdm_get_display_list(void) {
    return &g_disp_list;
}

int sdm_group_display_init_default(void) {
    struct list_node *head = sdm_get_display_list();
    sdm_display_t *sdm;

    list_for_every_entry(head, sdm, sdm_display_t, node) {
        DISPDBG("disp->id, disp->handle->display_id (%d, %d)\n",
            sdm->id, sdm->handle->display_id);
        if (sdm->handle) {
            list_add_tail(&g_group_disp_list, &sdm->node);
        }
    }
    return 0;
}

int sdm_group_display_register(sdm_display_t *sdm) {
    if (!sdm->handle) {
        DISPERR("register display hanlde is null\n");
        return -1;
    }
    list_add_tail(&g_group_disp_list, &sdm->node);
    return 0;
}

int sdm_group_display_post(struct sdm_post_config *post) {
    struct list_node *head = sdm_get_display_list();
    sdm_display_t *sdm;
    int index = 0;

    size_t len = list_length(&g_group_disp_list);
    if (post->n_bufs > len) {
        DISPERR("have %d buffer to post, but only %d displays work\n", (int)post->n_bufs, (int)len);
        return -1;
    }

    index = 0;
    list_for_every_entry(head, sdm, sdm_display_t, node) {
        struct sdm_post_config p;
        p.bufs = post->bufs;
        p.n_bufs = 1;
        p.custom_data_size = post->custom_data_size;
        p.custom_data = post->custom_data;
        p.post_noblock = post->post_noblock;
        sdm_post(sdm->handle, &p);
        index++;
    }

    return 0;
}

void sdm_screen_clear(display_handle *handle)
{
    struct sdm_post_config p;
    struct sdm_buffer bufs[2] = {0};
    int width, height;
    int i;
    void *logo = NULL;

    width = handle->info.width;
    height = handle->info.height;

    memset(bufs, 0, sizeof(struct sdm_buffer)*2);
    p.bufs = bufs;
    p.n_bufs = 2;
    p.post_noblock = 0;
    for (i = 0; i < 2;i++) {
        bufs[i].layer = i;
        bufs[i].layer_en = 0;
        bufs[i].fmt = COLOR_RGB565;
        bufs[i].alpha_en = 1;
        bufs[i].alpha = 0x0;
        bufs[i].ckey = 0;
        bufs[i].ckey_en = 0;
        bufs[i].src.x = 0;
        bufs[i].src.y = 0;
        bufs[i].src.w = width;
        bufs[i].src.h = height;

        bufs[i].start.x = 0;
        bufs[i].start.y = 0;
        bufs[i].start.w = width;
        bufs[i].start.h = height;
        bufs[i].addr[0] = (unsigned long)logo;
        bufs[i].src_stride[0] = width * 2;

        bufs[i].dst.x = 0;
        bufs[i].dst.y = 0;
        bufs[i].dst.w = width;
        bufs[i].dst.h = height;
        bufs[i].z_order = i;
    }
    sdm_post(handle, &p);
}

bool sdm_display_is_inited(void)
{
    return is_inited;
}

void sdm_display_init(struct display_resource *disp_res, int res_count, int os)
{
    int i;
    int num_display;
    int ret = 0;

    // connect all display
    hal_display_connect_all(disp_res, res_count, os);
    num_display = hal_display_num();

    sdm_init();

#ifdef CONFIG_DISP_CLIENT
    disp_client_init();
#endif

    DISPDBG("display num is %d\n", num_display);
    for(i = 0; i < num_display; i++) {
        display_handle *handle = hal_get_display_handle_by_index(i);
        handle->os_type = os;

        if (handle->res->gui_enable == PANEL_NOT_NEED_INIT) {
            continue;
        }
        if (handle->dm && handle->dm->dc) {
            //DISPDBG("init panel for display [%d]-- index [%d]\n", handle->display_id, i);
            int n_panels = ARRAY_SIZE(registered_panels[handle->display_id]);

            ret = sdm_display_handle_init(handle,
                registered_panels[handle->display_id], n_panels);
            if (ret < 0)
                continue;

            sdm_screen_clear(handle);
        }
    }
    DISPDBG("found %u displays connected\n", list_length(sdm_get_display_list()));
    is_inited = true;
}

