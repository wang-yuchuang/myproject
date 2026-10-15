/**
 * @file disp_panel.c
 * @brief display subsystem source file.
 *
 * @copyright Copyright (c) 2019-2029 Semidrive Semiconductor.
 *            All rights reserved.
 */
#include <cmsis_os2.h>
#include <disp_drv_log.h>
#include <sdm_panel.h>
#include <disp_dsi.h>
#include <disp_dsi_api.h>
#include <disp_general_header.h>

extern struct dsi_context dsi_ctx[2];

int mipi_lcm_init(int dsi_index, struct sdm_panel *panel)
{
    struct dsi_cmd_desc *cmds = panel->mipi->init_data;
    struct dsi_context *ctx = NULL;
    struct dsi_core_ops *dsi_ops = NULL;
    uint32_t size = ARRAYSIZE(cmds);
    uint16_t len;
    uint32_t i;
    uint8_t dsi_if_count = 0, dsi_id = 0;

    if (dsi_index >= 2) {
        ctx = &dsi_ctx[dsi_id];
        dsi_if_count = 2;
    }
    else {
        ctx = &dsi_ctx[dsi_index];
        dsi_if_count = 1;
    }

    if (ctx == NULL)
        return -1;

    dsi_ops = ctx->ops;

    if (cmds == NULL)
        goto skip_send_cmd;

    dsi_ops->power_enable(ctx, 1);
    udelay(10*1000);

    dsi_ops->cmd_mode(ctx);

    for (i = 0; i < size; i++) {
        len = (cmds->wc_h << 8) | cmds->wc_l;
        if (panel->mipi->use_dcs)
            mipi_dsih_dcs_wr_cmd(ctx, ctx->dpi_video.virtual_channel,
                cmds->playload, len);
        else
            mipi_dsih_gen_wr_cmd(ctx, ctx->dpi_video.virtual_channel,
                cmds->playload, len);

        if (cmds->wait)
            osDelay(cmds->wait * 1000);

        cmds++;
    }

    dsi_ops->power_enable(ctx, 0);
    udelay(100);
    dsi_ops->video_mode(ctx);

skip_send_cmd:
    dsi_ops->dphy_enable_hs_clk(ctx, 1);

    dsi_ops->power_enable(ctx, 1);
    udelay(10*1000);

    //dsi_ops->int0_status(ctx);
    //dsi_ops->int1_status(ctx);
    dsi_if_count--;

    if (dsi_if_count > 0) {
        dsi_id++;
        ctx = &dsi_ctx[dsi_id];

        if (ctx == NULL)
            return -1;

        dsi_ops = ctx->ops;
        goto skip_send_cmd;
    }
    return 0;
}