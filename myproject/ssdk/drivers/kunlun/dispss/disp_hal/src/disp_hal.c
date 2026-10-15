/**
 * @file disp_hal.c
 * @brief display subsystem source file.
 *
 * @copyright Copyright (c) 2019-2029 Semidrive Semiconductor.
 *            All rights reserved.
 */
#include <disp_hal.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <spinlock.h>
#include <disp_ssystem_link.h>
#include <clock_cfg.h>
#include <disp_cfg.h>
#include <regs_base.h>
#include <disp_drv_log.h>

/********** global **********/
display_handle g_instances[DISPLAY_TYPE_MAX] = {0};
spin_lock_t disp_spin_lock = SPIN_LOCK_INITIAL_VALUE;
int g_display_num = 0;

typedef struct
{
    int enum_res_id;
    uint32_t base[8];
    int resources[8];
} enum_res_to_idx;

#if DP_INIT
//dp
enum_res_to_idx table_dp_res[] = {
    {DP_RES_DP1, {APB_DP1_BASE}, {DP_RES_DP1, DP_RES_NC}},
    {DP_RES_DP2, {APB_DP2_BASE}, {DP_RES_DP2, DP_RES_NC}},
    {DP_RES_DP3, {APB_DP3_BASE}, {DP_RES_DP3, DP_RES_NC}},
    {DP_RES_DP1_DP2, {APB_DP1_BASE, APB_DP2_BASE},
        {DP_RES_DP1, DP_RES_DP2, DP_RES_NC}},
    {-1, {-1}},
};
#endif

//dc
enum_res_to_idx table_dc_res[] = {
    {DC_RES_DC1, {APB_DC1_BASE}, {DC_RES_DC1, DC_RES_NC}},
    {DC_RES_DC2, {APB_DC2_BASE}, {DC_RES_DC2, DC_RES_NC}},
    {DC_RES_DC3, {APB_DC3_BASE}, {DC_RES_DC3, DC_RES_NC}},
    {DC_RES_DC4, {APB_DC4_BASE}, {DC_RES_DC4, DC_RES_NC}},
    {DC_RES_DC5, {APB_DC5_BASE}, {DC_RES_DC5, DC_RES_NC}},
    {DC_RES_DC1_DC2, {APB_DC1_BASE, APB_DC2_BASE},
        {DC_RES_DC1, DC_RES_DC2, DC_RES_NC}},
    {DC_RES_DC3_DC4, {APB_DC3_BASE, APB_DC4_BASE},
        {DC_RES_DC3, DC_RES_DC4, DC_RES_NC}},
    {-1, {-1}},
};

enum_res_to_idx table_if_res[] = {
    //lvds
    {IF_RES_LVDS1, {APB_LVDS1_BASE}, {IF_RES_LVDS1, IF_RES_NC}},
    {IF_RES_LVDS2, {APB_LVDS2_BASE}, {IF_RES_LVDS2, IF_RES_NC}},
    {IF_RES_LVDS3, {APB_LVDS3_BASE}, {IF_RES_LVDS3, IF_RES_NC}},
    {IF_RES_LVDS4, {APB_LVDS4_BASE}, {IF_RES_LVDS4, IF_RES_NC}},
    {IF_RES_LVDS1_LVDS2, {APB_LVDS1_BASE, APB_LVDS2_BASE},
        {IF_RES_LVDS1, IF_RES_LVDS2, IF_RES_NC}},
    {IF_RES_LVDS3_LVDS4, {APB_LVDS3_BASE, APB_LVDS4_BASE},
        {IF_RES_LVDS3, IF_RES_LVDS4, IF_RES_NC}},
    {IF_RES_LVDS1_4, {APB_LVDS1_BASE, APB_LVDS2_BASE, APB_LVDS3_BASE, APB_LVDS4_BASE},
        {IF_RES_LVDS1, IF_RES_LVDS2, IF_RES_LVDS3, IF_RES_LVDS4, IF_RES_NC}},
    //mipi
    {IF_RES_MIPI_DSI1, {APB_MIPI_DSI1_BASE}, {IF_RES_MIPI_DSI1, IF_RES_NC}},
    {IF_RES_MIPI_DSI2, {APB_MIPI_DSI2_BASE}, {IF_RES_MIPI_DSI2, IF_RES_NC}},
    {IF_RES_DSI1_DSI2, {APB_MIPI_DSI1_BASE, APB_MIPI_DSI2_BASE},
        {IF_RES_MIPI_DSI1, IF_RES_MIPI_DSI2, IF_RES_NC}},
    {-1, {-1}},
};

uint8_t dc_pll[5] = {
#if (defined(TARGET_REFERENCE_X9U_A) || defined(TARGET_REFERENCE_X9U_B))
    /*X9U*/
    DSP_CLK_PLL_LVDS1, //DC1 safety Infotainmen
    DSP_CLK_PLL_LVDS2, //DC2 safety Cluster
    DSP_CLK_PLL_LVDS3, //DC3 safety ControlPanel
    DSP_CLK_PLL_LVDS1, //DC4 safety Entertainment
    DSP_CLK_PLL_LVDS4  //DC5 ecockpit HUD
#else
    /*X9 high(plus)*/
    DSP_CLK_PLL_LVDS1, //DC1 safety Infotainmen
    DSP_CLK_PLL_LVDS2, //DC2 safety Cluster
    DSP_CLK_PLL_LVDS3, //DC3 safety ControlPanel
    DSP_CLK_PLL_LVDS4, //DC4 safety Entertainment
    DSP_CLK_PLL_LVDS4  //DC5 ecockpit HUD
#endif
};

int get_resource_from_table(int table_item, const enum_res_to_idx *table,
                            int *num_out, mod_res_t **out)
{
    const enum_res_to_idx *item = NULL;
    mod_res_t *r = NULL;
    int res_num = 0;
    int i;

    if (table_item == -1) {
        goto FAIL_DOMAIN;
    }

    for (i = 0;table[i].enum_res_id != -1; i++) {
        if (table[i].enum_res_id == table_item) {
            item = &table[i];
            break;
        }
    }
    if (item == NULL) {
        goto FAIL_DOMAIN;
    }

    for (i = 0, res_num = 0;; i++)
    {
        if (item->resources[i] == -1)
            break;
        res_num++;
    }
    r = malloc(sizeof(mod_res_t) * res_num);

    for (i = 0; i < res_num; i++)
    {
        r[i].res_id = item->resources[i];
        r[i].base = item->base[i];
        if (!r[i].base)
            goto FAIL_DOMAIN;
    }
    *num_out = res_num;
    *out = r;
    return 0;

FAIL_DOMAIN:
    if (r)
        free(r);
    *out = NULL;
    return -1;
}

int install_resource(struct display_resource *res, struct sdm_domain_res **out)
{
    struct sdm_domain_res *dm = NULL;
    int ret = 0;

    dm = (struct sdm_domain_res *)malloc(sizeof(struct sdm_domain_res));
    if (!dm) {
        DISPERR("malloc dm failed\n");
        goto DM_FAIL;
    }
    memset(dm, 0, sizeof(struct sdm_domain_res));

#if DP_INIT
//dp
    ret = get_resource_from_table(res->dp_res, table_dp_res, &dm->num_dp, &dm->dp);
    if (ret)
    {
        DISPERR("get dp failed\n");
        goto DM_FAIL;
    }
#endif

//dc
    ret = get_resource_from_table(res->dc_res, table_dc_res, &dm->num_dc, &dm->dc);
    if (ret)
    {
        DISPERR("get dc failed\n");
        goto DM_FAIL;
    }

//if-res
    switch (res->if_res)
    {
    case IF_RES_MIPI_DSI1:
    case IF_RES_MIPI_DSI2:
    case IF_RES_DSI1_DSI2:
        ret = get_resource_from_table(res->if_res, table_if_res, &dm->num_intf, &dm->intf);
        if (ret) {
            DISPERR("get dsi failed\n");
            goto DM_FAIL;
        }
        break;
    case IF_RES_LVDS1 ... IF_RES_LVDS4:
    case IF_RES_LVDS1_LVDS2:
    case IF_RES_LVDS3_LVDS4:
        ret = get_resource_from_table(res->if_res, table_if_res, &dm->num_intf, &dm->intf);
        if (ret) {
            DISPERR("get lvds failed\n");
            goto DM_FAIL;
        }
        break;
    default:
        break;
    }

	*out = dm;
    return 0;
DM_FAIL:
    if (dm->dp)
        free(dm->dp);
    if (dm->dc)
        free(dm->dc);
    if (dm->intf)
        free(dm->intf);
    if (dm)
        free(dm);
    return ret;
}

static int get_wrap_id(display_handle *handle)
{
    int dc_id;

    switch (handle->res->dc_res) {
        case DC_RES_DC1:
        case DC_RES_DC1_DC2:
            dc_id = 0; break;
        case DC_RES_DC2:
            dc_id = 1; break;
        case DC_RES_DC3:
        case DC_RES_DC3_DC4:
            dc_id = 2; break;
        case DC_RES_DC4:
            dc_id = 3; break;
        case DC_RES_DC5:
            dc_id = 4; break;
        default:
            dc_id = 0; break;
    }

    return dc_pll[dc_id];
}

int hal_sdm_set_pll_clk(display_handle *handle, uint32_t clk)
{
    struct {
        int idx;
        clk_node_t *clk_id;
        clk_node_t *clean_ref1;
        clk_node_t *clean_ref2;
    } idx_to_clkid[] = {
        {DSP_CLK_PLL_LVDS1, &g_pll_lvds1_div_a, &g_pll_lvds1_root, &g_pll_lvds1_div_b},
        {DSP_CLK_PLL_LVDS2, &g_pll_lvds2_div_a, &g_pll_lvds2_root, &g_pll_lvds2_div_b},
        {DSP_CLK_PLL_LVDS3, &g_pll_lvds3_div_a, &g_pll_lvds3_root, &g_pll_lvds3_div_b},
        {DSP_CLK_PLL_LVDS4, &g_pll_lvds4_div_a, &g_pll_lvds4_root, &g_pll_lvds4_div_b},
        {-1, 0, 0, 0},
    };

    int ret = 0;
    int pll_id = 0;
    uint8_t j = 0;
    static int pll_clk_store[5] = {0, 0, 0, 0, 0};
    static int pll_fps_store[5] = {0, 0, 0, 0, 0};
    struct sdm_panel *panel;

    if (!handle || !handle->res || (handle->res->dc_res < 0))
        return 0;

    panel = handle->panels_info[handle->found_panel]->panel;

    for (j = 0; idx_to_clkid[j].idx != -1; j++) {
        pll_id = get_wrap_id(handle);

        if (pll_id == idx_to_clkid[j].idx)
            break;
    }

    if (idx_to_clkid[j].idx == -1) {
        DISPERR("disp pll cfg -- get clk_id by idx failed.\n");
        ret = -1;
        goto out;
    }

    if (pll_clk_store[pll_id]) {
        DISPDBG("clk %d has inited\n", pll_id);
        handle->res->clk = pll_clk_store[pll_id];
        panel->fps = pll_fps_store[pll_id];
        goto out;
    }

    if (lvds_clk_set_rate(CLK_NODE(*idx_to_clkid[j].clk_id), clk)) {
        DISPERR("dc set pll div a clock failed!\n");
        ret = -1;
        goto out;
    }

    pll_clk_store[pll_id] = clk;
    pll_fps_store[pll_id] = panel->fps;

out:
    return ret;
}

int hal_sdm_panel_connect(display_handle *handle, int sub_id, struct sdm_panel *panel)
{
    if (true == disp_panel_type_check(handle, panel))
        return 0;

    return -1;
}

int hal_sdm_panel_type_check(display_handle *handle, struct sdm_panel *panels[], int n_panels)
{
    int found = -1;

    for (int i = 0; i < n_panels; i++)
    {
        struct sdm_panel *p = panels[i];
        //LOGD("check panel %s...\n", p->panel_name);
        if (true == disp_panel_type_check(handle, p)) {
            found = i;
            break;
        }
    }

    return found;
}

int hal_sdm_if_init(display_handle *handle, struct sdm_panel *panel)
{
    int ret;

    if (panel->mipi) {
        if (panel->mipi->out_csi) {
            int line_num = panel->display_timing->vfront_porch + panel->display_timing->vactive +
            panel->display_timing->vback_porch / 2;

            disp_dsi_to_csi_config(handle->res->if_res, line_num, panel->display_timing->hactive,
            handle->res->pixel_bpp, panel->mipi->color_coding);

            DISPMSG("disp:enable dsi to csi\n");
        }
    }

    /*interface init*/
    ret = display_if_init(handle, panel);
    if (ret < 0) {
        DISPERR("Interface init failed\n");
        return false;
    }

    if (0)/*(panel_compare_id()) billy*/ {
        ret = display_if_deinit(handle, panel);
        if (ret < 0) {
            DISPERR("Interface deinit failed\n");
        }
        return false;
    }

    /*Panel init power*/
    //panel_init_power(); //billy
    ret = panel_init(handle, panel);
    if (ret < 0) {
        DISPERR("panel init failed\n");
        return false;
    }

    return true;
}

int hal_sdm_panel_disconnect(display_handle *handle, int sub_id, struct sdm_panel *panel)
{
    return disp_panel_disconnect(handle, sub_id, panel);
}

/*
 * os: 0, rtos; 1, dil2; 3, ssdk
 */
int hal_display_connect_all(struct display_resource *disp_res, int res_count, int os)
{
    unsigned int i;
    int count = 0;

    if (DIL2 == os)
        disp_link_init(dc_pll, disp_res, res_count);

    for (i = 0; i < ARRAY_SIZE(g_instances); i++)
        g_instances[i].display_id = NOT_FOUND;

    /*init display_num to zero*/
    g_display_num = 0;
    for (i = 0; i < res_count; i++)
    {
        struct display_resource *res = disp_res + i;
        if (hal_display_handle_install_resource(res))
        {
            DISPERR("Error: probe display %d-%d failed\n", i, res->type);
            continue;
        }
        count++;
    }
    g_display_num = count;

    return 0;
}

int hal_display_num()
{
    return g_display_num;
}

display_handle *hal_get_display_handle(enum DISPLAY_TYPE display)
{
	int i;
	for (i = 0; i < DISPLAY_TYPE_MAX;i++) {
		display_handle *handle = &g_instances[i];
		if (handle->display_id == display)
			return handle;
	}

    return NULL;
}

display_handle *hal_get_display_handle_by_index(uint32_t index)
{
    if (index < DISPLAY_TYPE_MAX)
        return &g_instances[index];

    return NULL;
}

int hal_sdm_callback_vsync_register(display_handle *handle, CALLBACK_EVENT_T func)
{
    if (handle->display_id != -1) {
        return callback_vsync_register(handle, func);
    }
    else
        return -1;
}

int hal_display_handle_install_resource(struct display_resource *res)
{
    static uint8_t index = 0;
    if (res->type > DISPLAY_TYPE_MAX || res->type < DISPLAY_TYPE_INVALID)
    {
        DISPERR("Error: invalid display type: %d", res->type);
        return -1;
    }

    if (res->type == DISPLAY_TYPE_INVALID)
    {
        DISPERR("display may not choosed");
        return -2;
    }

    display_handle *handle = &g_instances[index++];
    handle->res = res;
    handle->display_id = res->type;
    handle->name = res->name;
    handle->hotplug = NULL;
    handle->vsync = NULL;
    handle->inited = false;
    handle->panel_post_end_done = false;

    return install_resource(res, &handle->dm);
}

int hal_display_create_handle(display_handle **handle, enum DISPLAY_TYPE display)
{
    if (hal_display_num())
        *handle = hal_get_display_handle(display);
    else
    {
        DISPERR("can not found any display\n");
        *handle = NULL;
        return -1;
    }

    return 0;
}

void hal_display_release_handle(display_handle *handle)
{
    DISPDBG("%d:%s released\n", handle->display_id, handle->name);
}

int hal_sdm_init(display_handle *handle)
{
    return disp_init(handle);
}

int hal_sdm_deinit(display_handle *handle)
{
    return disp_uninit(handle);
}

int hal_sdm_clear_layers(display_handle *handle, uint8_t mask, uint8_t zorder)
{
    return disp_clear_layers(handle, mask, zorder);
}

int hal_sdm_post_config(display_handle *handle, struct sdm_post_config *post)
{
    struct sdm_panel *panel = handle->panels_info[handle->found_panel]->panel;

    /*For sis funciton*/
    if (panel->rtos_screen) {
        for (size_t i = 0; i < post->n_bufs; i++)
            if (post->bufs[i].layer_en) {
                post->bufs[i].dst.x += handle->info.rtos_screen.pos_x;
                post->bufs[i].dst.y += handle->info.rtos_screen.pos_y;
            }
    }

    return disp_post_config(handle, post);
}

int hal_sdm_pq_config(display_handle *handle, struct sdm_pq_params *pq)
{
    return disp_pq_config(handle, pq);
}

int disp_crc32_callback(int signal, void* args)
{
    unsigned int val = 0;
    int *display_id = (int *)args;

    val = disp_crc32_status(*display_id);
    DISPERR("val:0x%x\n", val);

    for (int i = 0; i < 8; i++) {
        if (val & (1 << i))
            DISPERR("block[%d] crc err\n", i);
    }

    return 0;
}


int hal_sdm_crc32_set(display_handle *handle, struct sdm_buffer *buf, struct crc_info *info)
{
    return disp_crc32_set(handle, buf, info);
}

int hal_sdm_crc32_disable(display_handle *handle)
{
    return disp_crc32_disable(handle);
}

u32 hal_sdm_crc32_get(display_handle *handle, uint8_t index)
{
	return disp_crc32_get_value(handle, index);
}

int hal_sdm_safety_config(display_handle *handle, struct sdm_buffer *buf)
{
    return disp_safety_config(handle, buf);
}

void hal_sdm_safety_exit(display_handle *handle)
{
    disp_safety_exit(handle);
}

int hal_sdm_dcrecovery(const char *arg, uint16_t rec_id)
{
    int rec_dp_res = 0;
    display_handle *handle = NULL;
    u8 i = 0;
    dprintf(0, "disp recovery:%s recid:%d\n", arg, rec_id);

    if (strstr(arg, "dp1") && strstr(arg, "dp2"))
        rec_dp_res = DP_RES_DP1_DP2;
    else if (strstr(arg, "dp1"))
        rec_dp_res = DP_RES_DP1;
    else if (strstr(arg, "dp2"))
        rec_dp_res = DP_RES_DP2;
    else if (strstr(arg, "dp3"))
        rec_dp_res = DP_RES_DP3;

    for (i = 0; i < SCREEN_MAX; i++)
    {
        handle = hal_get_display_handle(i);
        if (handle && handle->res->dp_res == rec_dp_res)
            break;
    }

    if (i == SCREEN_MAX) {
        dprintf(0, "%s can't find res:%s\n", __func__, arg);
        return -1;
    }

    disp_dump_register(handle);
    return 0;
}
