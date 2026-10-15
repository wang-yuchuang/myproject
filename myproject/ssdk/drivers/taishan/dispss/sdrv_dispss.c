/*
* sdrv_display.c
*
* Copyright (c) 2019 Semidrive Semiconductor.
* All rights reserved.
*
* Description:
*
* Revision History:
* -----------------
* 011, 11/11/2021 BILLY create this file
*/
#include <board.h>
#include <driver/irq.h>
#include <driver/taishan/dispss/disp.h>
#include <driver/taishan/dispss/dispss_log.h>
#include <driver/taishan/dispss/disp_data_type.h>
#include <driver/taishan/dispss/logo_rgb565.h>
#ifdef CONFIG_LVDS_LT9211_TO_MIPI_480x1920_LCD
#include "lcm/lt9211/lt9211.h"
#endif
#ifdef CONFIG_LVDS_LT9211_1PORT_TO_2PORT_1920x720_LCD
#include "lcm/lvds_lt9211_1port_to_2port/lvds_lt9211_1port_to_2port.h"
#endif
#include "clock_cfg.h"

extern struct sdrv_panel *sdrv_panels[];

void sdrv_display_logo(struct dc_dev *dev, unsigned long logo)
{
    struct post_cfg post;
    struct surface layer[2] = {0};
    int width = dev->panel->timing->hactive;
    int height = dev->panel->timing->vactive;
    sdrv_display_set_bg_color(dev, 0x3fffffff);
    {
        layer[0].id = 1;
        layer[0].dirty = 1;
        layer[0].en = 1;
        layer[0].fmt = COLOR_RGB565;

        layer[0].src.x = 0;
        layer[0].src.y = 0;
        layer[0].src.w = image.width;
        layer[0].src.h = image.height;

        layer[0].start.x = 0;
        layer[0].start.y = 0;
        layer[0].start.w = image.width;
        layer[0].start.h = image.height;

        layer[0].addr[0] = (unsigned long)image.y;
        layer[0].src_stride[0] = image.width * 2;

        layer[0].dst.x = (width - image.width) / 2;
        layer[0].dst.y = (height - image.height) / 2;
        layer[0].dst.w = image.width;
        layer[0].dst.h = image.height;

        layer[0].z_order = 0;
        layer[0].alpha_en = 1;
        layer[0].alpha = 0xff;
        layer[0].ckey_en = 0;
    }

    post.num_layers = 1;
    post.layers = layer;

    dev->ops->update(dev, &post);
    dev->ops->triggle(dev);
    dev->ops->enable(dev);
    sdrv_display_set_bg_color(dev, 0x0);

#ifdef CONFIG_LVDS_LT9211_TO_MIPI_480x1920_LCD
    LT9211_MainLoop();
#endif

#ifdef CONFIG_LVDS_LT9211_1PORT_TO_2PORT_1920x720_LCD
    LT9211_LVDS2LVDS_MainLoop();
#endif
}

void sdrv_display_blank(struct dc_dev *dev)
{
    struct post_cfg post;
    struct surface layer[2] = {0};
    int width = dev->panel->timing->hactive;
    int height = dev->panel->timing->vactive;

    {
        layer[0].id = 1;
        layer[0].dirty = 0;
        layer[0].en = 0;
        layer[0].fmt = COLOR_RGB565;

        layer[0].src.x = 0;
        layer[0].src.y = 0;
        layer[0].src.w = width;
        layer[0].src.h = height;

        layer[0].start.x = 0;
        layer[0].start.y = 0;
        layer[0].start.w = width;
        layer[0].start.h = height;

        layer[0].addr[0] = 0;
        layer[0].src_stride[0] = width * 2;

        layer[0].dst.x = 0;
        layer[0].dst.y = 0;
        layer[0].dst.w = width;
        layer[0].dst.h = height;

        layer[0].z_order = 0;
        layer[0].alpha_en = 1;
        layer[0].alpha = 0xff;
        layer[0].ckey_en = 0;
    }

    post.num_layers = 1;
    post.layers = layer;

    dev->ops->update(dev, &post);
    dev->ops->triggle(dev);
    dev->ops->enable(dev);
}

int sdrv_display_get_width(struct dc_dev *dev)
{
    return dev->panel->timing->hactive;
}

int sdrv_display_get_height(struct dc_dev *dev)
{
    return dev->panel->timing->vactive;
}

void sdrv_display_set_bg_color(struct dc_dev *dev, int color)
{
    dev->ops->set_bg_color(dev, color);
}

void sdrv_display_post(struct dc_dev *dev, struct post_cfg *post)
{
    unsigned int post_time = current_time();

    dev->ops->update(dev, post);
    dev->ops->hsdk_enable(dev, 0, false);
    dev->ops->triggle(dev);
    dev->ops->enable(dev);

    if (dev->app.id == ASW_APP_EMIRROR)
        return;

    while (dev->ops->check_triggle_status(dev)) {
        while (post_time > dev->vsync_wait) {
            osDelay(1);
        }
    }
}

void sdrv_dc_handshake(struct dc_dev *dev,bool enable, int mode, struct post_cfg *post)
{
    if (enable)
        dev->ops->update(dev, post);
    dev->ops->hsdk_enable(dev, mode, enable);
    dev->ops->triggle(dev);
}

void sdrv_update_csi_detect(struct dc_dev *dev, struct post_cfg *post,
    int mask_count, int ratio, bool enable)
{
    dev->ops->update_csi_detect(dev, post, mask_count, ratio, enable);
}

void sdrv_panel_probe(struct sdrv_panel *panel)
{
    switch (panel->if_type) {
        case IF_TYPE_HDMI:
            //sdrv_adv_init();
            break;
        case IF_TYPE_LVDS:
            sdrv_lvds_init(panel);
            break;
        default:
            break;
    }
#ifdef CONFIG_LVDS_LT9211_TO_MIPI_480x1920_LCD
    LT9211_Main();
#endif

#ifdef CONFIG_LVDS_LT9211_1PORT_TO_2PORT_1920x720_LCD
    LT9211_LVDS2LVDS_Main();
#endif
}

void sdrv_display_clk_init(struct display_timing *timing)
{
    long pixel_clk;
    clk_node_t *clk_root = CLK_NODE(g_pll_lvds_root);
    clk_node_t *clk_nodiv = CLK_NODE(g_pll_lvds_nodiv);
    clk_node_t *clk_div2 = CLK_NODE(g_pll_lvds_div2);
    clk_node_t *clk_div7 = CLK_NODE(g_pll_lvds_div7 );

    if (!timing->fps) {
        int fps = clk_div7->rate / ((timing->hactive + timing->hfront_porch + timing->hback_porch + timing->hsync_len)
            * (timing->vactive + timing->vfront_porch + timing->vback_porch + timing->vsync_len));
        DISPSS_LOG_INFO("using default pll clk, fps = %d\n", fps);
        return;
    }

    pixel_clk = (timing->hactive + timing->hfront_porch + timing->hback_porch + timing->hsync_len)
            * (timing->vactive + timing->vfront_porch + timing->vback_porch + timing->vsync_len)
            * timing->fps;

    for (int r = 2; r < 10; r++) {
        clk_root->rate = pixel_clk * 7 * r;
        if (clk_root->rate >= 500000000)
            break;
    }
    clk_nodiv->rate = clk_root->rate;
    clk_prepare(clk_root);
    clk_prepare(clk_nodiv);

    clk_set_rate(clk_div7, pixel_clk);
    clk_set_rate(clk_div2, pixel_clk * 7);
}

void sdrv_display_init(struct dc_dev *dev)
{
    DISPSS_LOG_FUNC();

    //clk set
    sdrv_display_clk_init(dev->panel->timing);

    //disp mux
    sdrv_disp_mux_init(dev->panel);

    //init interface and lcm
    sdrv_panel_probe(dev->panel);

    //init dc
    dev->ops->init(dev);
    dev->ops->vsync_enable(dev, true);

    DISPSS_LOG_INFO("init done!\n");
}

void sdrv_dc_probe(struct dc_dev *dev, const struct dc_config *dc_cfg)
{
    dev->base = dc_cfg->base;
    dev->irq = dc_cfg->irq;
    dev->ops = &dc_ops;
    dev->panel = sdrv_panels[0];
    dev->vsync_wait = 0;
    dev->app.id = 0;
    dev->crc32 = dc_cfg->crc32;
    irq_attach(dev->irq, dev->ops->irq_handler, dev);
    irq_enable(dev->irq);
}