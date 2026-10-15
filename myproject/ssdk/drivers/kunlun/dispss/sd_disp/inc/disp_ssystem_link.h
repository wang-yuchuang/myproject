/**
 * @file disp_ssystem_link.h
 * @brief display subsystem header file.
 *
 * @copyright Copyright (c) 2019-2029 Semidrive Semiconductor.
 *            All rights reserved.
 */
#ifndef __DISP_SSYTEM_LINK_H__
#define __DISP_SSYTEM_LINK_H__

enum {
    DSP_CLK_PLL_LVDS1 = 0x0,
    DSP_CLK_PLL_LVDS2 = 0x1,
    DSP_CLK_PLL_LVDS3 = 0x2,
    DSP_CLK_PLL_LVDS4 = 0x3,
    DSP_CLK_PLL_DISP  = 0x4
};

struct disp_pll_clk{
    int pll_id;
    int clk;
};

struct disp_pll_clks {
    int count;
    struct disp_pll_clk *pll_clks;
};

int disp_link_init(uint8_t *dc_pll, struct display_resource *disp_res, unsigned int res_num);
int disp_get_clks(struct disp_pll_clks* pclks);
void disp_dsi_to_csi_config(int if_res, int line_num,
    int hsize, int pixel_bpp, int colorcoding);
#endif