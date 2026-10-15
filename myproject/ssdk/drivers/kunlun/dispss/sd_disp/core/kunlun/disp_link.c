/*
* disp_sys_cfg.c
*
* Copyright (c) 2019 Semidrive Semiconductor.
* All rights reserved.
*
* Description:
*
* Revision History:
* -----------------
* 011, 10/21/2019 BI create this file
*/

#include <disp_drv_log.h>
#include <disp_general_header.h>
#include <disp_link.h>
#include <disp_mux_reg.h>
#include <disp_reg_rw.h>
#include <lvds_reg.h>
#include <stdlib.h>
#include <string.h>

static char lvds_sample_sync = 0;
static unsigned int res_count = 0;
static struct display_resource *g_display_res = NULL;
static unsigned char dc_pll_sel[DC_NUM] = {0};
static addr_t get_lvds_reg_base(void)
{
    static addr_t base = 0;

    if (base == 0) {
#if WITH_KERNEL_VM
        base = (addr_t) paddr_to_kvaddr(LVDS_BASE);
#else
        base = LVDS_BASE;
#endif
   }
    return base;
}

static addr_t get_disp_mux_reg_base(void)
{
    static addr_t base = 0;

    if (base == 0) {
#if WITH_KERNEL_VM
        base = (addr_t) paddr_to_kvaddr(DISP_MUX_BASE);
#else
        base = DISP_MUX_BASE;
#endif
   }
    return base;
}


static int if_link_value(uint8_t link)
{
    switch (link) {
    case LINK_DC1:  return 0;
    case LINK_DC2:  return 1;
    case LINK_DC3:  return 2;
    case LINK_DC4:  return 3;
    case LINK_DC5:  return 4;
    default:   return -1;
    }
}

static void conv_to_dplink(struct display_resource *dres,
    struct disp_dp_link *dp_link)
{
    switch (dres->dp_res) {
    case DP_RES_DP1:
        dp_link->dp1_link = dres->dc_res;
        break;
    case DP_RES_DP2:
        dp_link->dp2_link = dres->dc_res;
        break;
    case DP_RES_DP3:
        dp_link->dp3_link = dres->dc_res;
        break;
    case DP_RES_DP1_DP2:
        if (dres->dc_res == DC_RES_DC1_DC2) {
            dp_link->dp1_link = LINK_DC1;
            dp_link->dp2_link = LINK_DC2;
        } else {
            dp_link->dp1_link = dres->dc_res;
            dp_link->dp2_link = dres->dc_res;
        }
        break;
    default:
        break;
    }
}

static void conv_to_iflink(struct display_resource *dres,
    struct disp_if_link *if_link)
{
    switch(dres->if_res) {
    case IF_RES_PARALLEL:
        if_link->link_parallel = dres->dc_res;
        break;
    case IF_RES_MIPI_DSI1:
        if_link->link_dsi1 = dres->dc_res;
        break;
    case IF_RES_MIPI_DSI2:
        if_link->link_dsi2 = dres->dc_res;
        break;
    case IF_RES_LVDS1:
        if_link->link_lvds_group1_ch1 = dres->dc_res;
        break;
    case IF_RES_LVDS2:
        if_link->link_lvds_group1_ch2 = dres->dc_res;
        break;
    case IF_RES_LVDS3:
        if_link->link_lvds_group2_ch1 = dres->dc_res;
        break;
    case IF_RES_LVDS4:
        if_link->link_lvds_group2_ch2 = dres->dc_res;
        break;
    case IF_RES_LVDS1_LVDS2:
        if (dres->dc_res == DC_RES_DC1_DC2) {
            if (!dres->dc_ms_link_port_swap) {
                if_link->link_lvds_group1_ch1 = DC_RES_DC1;
                if_link->link_lvds_group1_ch2 = DC_RES_DC2;
            } else {
                if_link->link_lvds_group1_ch1 = DC_RES_DC2;
                if_link->link_lvds_group1_ch2 = DC_RES_DC1;
            }
        } else if (dres->dc_res == DC_RES_DC3_DC4){
            if (!dres->dc_ms_link_port_swap) {
                if_link->link_lvds_group1_ch1 = DC_RES_DC3;
                if_link->link_lvds_group1_ch2 = DC_RES_DC4;
            } else {
                if_link->link_lvds_group1_ch1 = DC_RES_DC4;
                if_link->link_lvds_group1_ch2 = DC_RES_DC3;
            }
        } else {
            if_link->link_lvds_group1_ch1 = dres->dc_res;
            if_link->link_lvds_group1_ch2 = dres->dc_res;
            lvds_sample_sync |= LVDS_SAMPLE_SYNC_GROP0;
        }

        if_link->link_lvds1_lvds2 = dres->dc_res;
        break;
    case IF_RES_LVDS3_LVDS4:
        if (dres->dc_res == DC_RES_DC1_DC2) {
            if (!dres->dc_ms_link_port_swap) {
                if_link->link_lvds_group2_ch1 = DC_RES_DC1;
                if_link->link_lvds_group2_ch2 = DC_RES_DC2;
            } else {
                if_link->link_lvds_group2_ch1 = DC_RES_DC2;
                if_link->link_lvds_group2_ch2 = DC_RES_DC1;
            }
        } else if (dres->dc_res == DC_RES_DC3_DC4){
            if (!dres->dc_ms_link_port_swap) {
                if_link->link_lvds_group2_ch1 = DC_RES_DC3;
                if_link->link_lvds_group2_ch2 = DC_RES_DC4;
            } else {
                if_link->link_lvds_group2_ch1 = DC_RES_DC4;
                if_link->link_lvds_group2_ch2 = DC_RES_DC3;
            }
        } else {
            if_link->link_lvds_group2_ch1 = dres->dc_res;
            if_link->link_lvds_group2_ch2 = dres->dc_res;
            lvds_sample_sync |= LVDS_SAMPLE_SYNC_GROP1;
        }

        if_link->link_lvds3_lvds4 = dres->dc_res;
        break;
    case IF_RES_DSI1_DSI2:
        /*billy*/
        if (dres->dc_res == DC_RES_DC1_DC2) {
            if (!dres->dc_ms_link_port_swap) {
                if_link->link_dsi1 = DC_RES_DC1;
                if_link->link_dsi2 = DC_RES_DC2;
            } else {
                if_link->link_dsi1 = DC_RES_DC2;
                if_link->link_dsi2 = DC_RES_DC1;
            }
        } else if (dres->dc_res == DC_RES_DC3_DC4){
            if (!dres->dc_ms_link_port_swap) {
                if_link->link_dsi1 = DC_RES_DC3;
                if_link->link_dsi2 = DC_RES_DC4;
            } else {
                if_link->link_dsi1 = DC_RES_DC4;
                if_link->link_dsi2 = DC_RES_DC3;
            }
        }

        if_link->link_dsi1_dsi2 = dres->dc_res;
        break;
    case IF_RES_LVDS1_4:
        dres->lvds_cmb = LVDS_CMB_TRUE;
        if (dres->dc_res == DC_RES_DC1_DC2)
            if_link->link_lvds_combine = CMB_LINK_DC1_DC2;
        else
            if_link->link_lvds_combine = CMB_LINK_DC3_DC4;

        lvds_sample_sync |= LVDS_SAMPLE_SYNC_4K;
        break;
    default:
        break;
    }
}

static int dp_link_init(struct disp_dp_link *dp_link)
{
    int val;
    addr_t base = get_disp_mux_reg_base();

    val = disp_read(base, MUX_RA040);
    /*DP1 LINK*/
    switch (dp_link->dp1_link) {
    case LINK_DC1:
        val = reg_value(1, 0, MUX_SHIFT00_RA040, MUX_MASK00_RA040);
        break;
    case LINK_DC2:
        val = reg_value(0, 0, MUX_SHIFT00_RA040, MUX_MASK00_RA040);
        break;
    }

    /*DP2 LINK*/
    switch (dp_link->dp2_link) {
    case LINK_DC1:
        val = reg_value(1, val, MUX_SHIFT01_RA040, MUX_MASK01_RA040);
        break;
    case LINK_DC2:
        val = reg_value(0, val, MUX_SHIFT01_RA040, MUX_MASK01_RA040);
        break;
    }

    /*DP3 LINK*/
    switch (dp_link->dp3_link) {
    case LINK_DC3:
        val = reg_value(0, val, MUX_SHIFT02_RA040, MUX_MASK02_RA040);
        break;
    case LINK_DC4:
        val = reg_value(1, val, MUX_SHIFT02_RA040, MUX_MASK02_RA040);
        break;
    }

    /*Set DP LINK value to Register*/
    disp_write(base, MUX_RA040, val);

    return 0;
}

static void lvds_combine_enable(bool enable)
{
    int val;
    addr_t base_lvds = get_lvds_reg_base();

    val = disp_read(base_lvds, LVDS_COMBINE);
    val = reg_value(enable ? 1 : 0, val, CMB_ENABLE_SHIFT, CMB_ENABLE_MASK);
    disp_write(base_lvds, LVDS_COMBINE, val);
}

static void lvds_combine_set_mode(uint8_t mode)
{
    int val;
    addr_t base_lvds = get_lvds_reg_base();

    val = disp_read(base_lvds, LVDS_COMBINE);
    val = reg_value(mode, val, CMB_MODE_SHIFT, CMB_MODE_MASK);
    disp_write(base_lvds, LVDS_COMBINE, val);
}
static int to_mux_x_sel_val(int link)
{
    switch (link) {
    case LINK_DC1:
        return SEL_DC1;
    case LINK_DC2:
        return SEL_DC2;
    case LINK_DC3:
        return SEL_DC3;
    case LINK_DC4:
        return SEL_DC4;
    case LINK_DC5:
        return SEL_DC5;
    case LINK_CMB_OUT_CH1:
        return SEL_CMB_OUT_CH1;
    case LINK_CMB_OUT_CH2:
        return SEL_CMB_OUT_CH2;
    default:
        return SEL_NOT_SUPPORT;
    }
}

static int to_mux_x_sel_pll_val(int link)
{
    switch (link) {
    case LINK_DC1:
        return dc_pll_sel[DC1];
    case LINK_DC2:
        return dc_pll_sel[DC2];
    case LINK_DC3:
        return dc_pll_sel[DC3];
    case LINK_DC4:
        return dc_pll_sel[DC4];
    case LINK_DC5:
        return dc_pll_sel[DC5];
    case LINK_CMB_OUT_CH1:
    case LINK_CMB_OUT_CH2:
    default:
        return -1;
    }
}

static int get_wrap_id(int res_index)
{
    int dc_id;

    switch ((g_display_res + res_index)->dc_res) {
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

    return dc_pll_sel[dc_id];
}
static void if_link_init(struct disp_if_link *if_link)
{
    int mux_sel_val, val;
    addr_t base = get_disp_mux_reg_base();
    addr_t base_lvds = get_lvds_reg_base();

    /*LINK DSI1*/
    val = if_link_value(if_link->link_dsi1);
    if (val >= 0) {
        disp_write(base, MUX_RA000, val);

        /*MUX_RA034 CLKSEL Config*/
        val = disp_read(base, MUX_RA034);
        val = reg_value(DSI_CLKSEL_CLK_GENERATION,val,
            MUX_SHIFT09_RA034, MUX_MASK09_RA034);
        disp_write(base, MUX_RA034, val);
    }

    /*LINK DSI2*/
    val = if_link_value(if_link->link_dsi2);
    if (val >= 0) {
        disp_write(base, MUX_RA004, val);

        /*MUX_RA03C CLKSEL Config*/
        val = disp_read(base, MUX_RA03C);
        val = reg_value(DSI_CLKSEL_CLK_GENERATION,val,
            MUX_SHIFT09_RA034, MUX_MASK09_RA034);
        disp_write(base, MUX_RA03C, val);
    }

    /*LINK PARALLEL*/
    val = if_link_value(if_link->link_parallel);
    if (val >= 0)
        disp_write(base, MUX_RA008, val);

    /*SPARE_3*/
    disp_write(base, MUX_RA088, lvds_sample_sync);

    mux_sel_val = disp_read(base, LVDS_MUX_SEL);

    /*Combine mode 1, all lvds use same PLL*/
    if ((if_link->link_lvds_combine == CMB_LINK_DC1_DC2) ||
        (if_link->link_lvds_combine == CMB_LINK_DC3_DC4)) {
        lvds_combine_enable(true);
        lvds_combine_set_mode(1);

        if (if_link->link_lvds_combine == CMB_LINK_DC1_DC2) {
            /*DISP MUX*/
            disp_write(base, MUX_RA00C, dc_pll_sel[DC1]);
            disp_write(base, MUX_RA010, dc_pll_sel[DC1]);
        } else {
            disp_write(base, MUX_RA014, dc_pll_sel[DC3]);
            disp_write(base, MUX_RA018, dc_pll_sel[DC3]);
        }

        mux_sel_val = reg_value(SEL_CMB_OUT_CH1, mux_sel_val,
                MUX0_SEL_SHIFT, MUX0_SEL_MASH);
        mux_sel_val = reg_value(SEL_NOT_SUPPORT, mux_sel_val,
                MUX1_SEL_SHIFT, MUX1_SEL_MASH);
        mux_sel_val = reg_value(SEL_CMB_OUT_CH2, mux_sel_val,
                MUX2_SEL_SHIFT, MUX2_SEL_MASH);
        mux_sel_val = reg_value(SEL_NOT_SUPPORT, mux_sel_val,
                MUX3_SEL_SHIFT, MUX3_SEL_MASH);

        if (if_link->link_lvds_combine == CMB_LINK_DC1_DC2)
            mux_sel_val = reg_value(CMB_SEL_DC1_DC2, mux_sel_val,
                    MUX4_SEL_SHIFT, MUX4_SEL_MASH);
        else
            mux_sel_val = reg_value(CMB_SEL_DC3_DC4, mux_sel_val,
                    MUX4_SEL_SHIFT, MUX4_SEL_MASH);

        if (if_link->link_lvds_combine == CMB_LINK_DC1_DC2)
            val = dc_pll_sel[DC1];
        else
            val = dc_pll_sel[DC3];
        mux_sel_val = reg_value(val, mux_sel_val, MUX5_SEL_SHIFT, MUX5_SEL_MASH);
        mux_sel_val = reg_value(val, mux_sel_val, MUX6_SEL_SHIFT, MUX6_SEL_MASH);
        mux_sel_val = reg_value(val, mux_sel_val, MUX7_SEL_SHIFT, MUX7_SEL_MASH);
        mux_sel_val = reg_value(val, mux_sel_val, MUX8_SEL_SHIFT, MUX8_SEL_MASH);
        disp_write(base_lvds, LVDS_MUX_SEL, mux_sel_val);
    } else {
        lvds_combine_enable(false);

        /*DISP MUX*/
        if ((if_link->link_dsi1_dsi2 == DC_RES_DC1_DC2) || (if_link->link_lvds1_lvds2 == DC_RES_DC1_DC2)) {
            disp_write(base, MUX_RA010, dc_pll_sel[DC1]);
            disp_write(base, MUX_RA018, dc_pll_sel[DC4]);
            /* mux5-6  */
            mux_sel_val = reg_value(0, mux_sel_val, MUX5_SEL_SHIFT, MUX5_SEL_MASH);
            mux_sel_val = reg_value(0, mux_sel_val, MUX6_SEL_SHIFT, MUX6_SEL_MASH);
            /* mux7*/
            val = to_mux_x_sel_pll_val(if_link->link_lvds_group2_ch1);
            if (val >= 0)
                mux_sel_val = reg_value(val, mux_sel_val, MUX7_SEL_SHIFT, MUX7_SEL_MASH);
            /* mux8*/
            val = to_mux_x_sel_pll_val(if_link->link_lvds_group2_ch2);
            if (val >= 0)
                mux_sel_val = reg_value(val, mux_sel_val, MUX8_SEL_SHIFT, MUX8_SEL_MASH);
        } else if ((if_link->link_dsi1_dsi2 == DC_RES_DC3_DC4) || (if_link->link_lvds3_lvds4 == DC_RES_DC3_DC4)) {
            disp_write(base, MUX_RA010, dc_pll_sel[DC2]);
            disp_write(base, MUX_RA018, dc_pll_sel[DC3]);
            /* mux5*/
            val = to_mux_x_sel_pll_val(if_link->link_lvds_group1_ch1);
            if (val >= 0)
                mux_sel_val = reg_value(val, mux_sel_val, MUX5_SEL_SHIFT, MUX5_SEL_MASH);
            /* mux6*/
            val = to_mux_x_sel_pll_val(if_link->link_lvds_group1_ch2);
            if (val >= 0)
                mux_sel_val = reg_value(val, mux_sel_val, MUX6_SEL_SHIFT, MUX6_SEL_MASH);
            /* mux7-8*/
                mux_sel_val = reg_value(2, mux_sel_val, MUX7_SEL_SHIFT, MUX7_SEL_MASH);
                mux_sel_val = reg_value(2, mux_sel_val, MUX8_SEL_SHIFT, MUX8_SEL_MASH);
        } else {
            disp_write(base, MUX_RA010, dc_pll_sel[DC2]);
            disp_write(base, MUX_RA018, dc_pll_sel[DC4]);
            /* mux5*/
            val = to_mux_x_sel_pll_val(if_link->link_lvds_group1_ch1);
            if (val >= 0)
                mux_sel_val = reg_value(val, mux_sel_val, MUX5_SEL_SHIFT, MUX5_SEL_MASH);
            /* mux6*/
            val = to_mux_x_sel_pll_val(if_link->link_lvds_group1_ch2);
            if (val >= 0)
                mux_sel_val = reg_value(val, mux_sel_val, MUX6_SEL_SHIFT, MUX6_SEL_MASH);
            /* mux7*/
            val = to_mux_x_sel_pll_val(if_link->link_lvds_group2_ch1);
            if (val >= 0)
                mux_sel_val = reg_value(val, mux_sel_val, MUX7_SEL_SHIFT, MUX7_SEL_MASH);
            /* mux8*/
            val = to_mux_x_sel_pll_val(if_link->link_lvds_group2_ch2);
            if (val >= 0)
                mux_sel_val = reg_value(val, mux_sel_val, MUX8_SEL_SHIFT, MUX8_SEL_MASH);
        }

        disp_write(base, MUX_RA00C, dc_pll_sel[DC1]);
        disp_write(base, MUX_RA014, dc_pll_sel[DC3]);
        disp_write(base, MUX_RA01C, dc_pll_sel[DC5]);

        val = to_mux_x_sel_val(if_link->link_lvds_group1_ch1);
        mux_sel_val = reg_value(val, mux_sel_val, MUX0_SEL_SHIFT, MUX0_SEL_MASH);

        val = to_mux_x_sel_val(if_link->link_lvds_group1_ch2);
        mux_sel_val = reg_value(val, mux_sel_val, MUX1_SEL_SHIFT, MUX1_SEL_MASH);

        val = to_mux_x_sel_val(if_link->link_lvds_group2_ch1);
        mux_sel_val = reg_value(val, mux_sel_val, MUX2_SEL_SHIFT, MUX2_SEL_MASH);

        val = to_mux_x_sel_val(if_link->link_lvds_group2_ch2);
        mux_sel_val = reg_value(val, mux_sel_val, MUX3_SEL_SHIFT, MUX3_SEL_MASH);

        disp_write(base_lvds, LVDS_MUX_SEL, mux_sel_val);
    }
}

static void disp_lvds_wrap_init(struct disp_if_link *if_link)
{
    int wrap_id, val, clk_sel, clk_en;
    addr_t base_lvds = get_lvds_reg_base();

    for (unsigned int i = 0; i < res_count; i++) {

        wrap_id = get_wrap_id(i);

        val = disp_read(base_lvds, LVDS_CHN_CLOCK_(wrap_id));
        if ((g_display_res + i)->lvds_mode == LVDS_MODE_DUAL) {
            clk_sel = SRC_SEL_3P5_CLOCK;
            clk_en = LVDS_CLK_EN | LVDS_CLK_X3P5_EN | LVDS_DSP_CLK_EN;
        } else {
            clk_sel = SRC_SEL_7_CLOCK;
            clk_en = LVDS_CLK_EN | LVDS_CLK_X7_EN | LVDS_DSP_CLK_EN;
        }
        val = reg_value(0, val, CHN_DIV_NUM_LVDS_SHIFT, CHN_DIV_NUM_LVDS_MASK);
        val = reg_value(clk_sel, val, CHN_SRC_SEL_SHIFT, CHN_SRC_SEL_MASK);
        val = reg_value(clk_en, val, CHN_GATING_EN_SHIFT, CHN_GATING_EN_MASK);
        disp_write(base_lvds, LVDS_CHN_CLOCK_(wrap_id), val);
    }
}

static char to_dsi_pixel_map(char pixel_bpp)
{
    switch (pixel_bpp) {
        case PIXEL_BPP_16:
            return DSI_PIXEL_MAP_RGB565;
        case PIXEL_BPP_18:
            return DSI_PIXEL_MAP_RGB666;
        case PIXEL_BPP_24:
            return DSI_PIXEL_MAP_RGB888;
        case PIXEL_BPP_30:
            return DSI_PIXEL_MAP_RGB101010;
        default:
            return DSI_PIXEL_MAP_RGB888;
    }
}
static void disp_dsi_pixel_map_init(void)
{
    int val;
    char pixel_map;
    addr_t base = get_disp_mux_reg_base();

    for(unsigned int i = 0; i < res_count; i++) {
        if ((g_display_res + i)->if_res == IF_RES_MIPI_DSI1) {
            /*MUX_RA030 CLKSEL Config*/
            pixel_map = to_dsi_pixel_map((g_display_res + i)->pixel_bpp);
            val = disp_read(base, MUX_RA030);
            val = reg_value(pixel_map, val,
                MUX_SHIFT00_RA030, MUX_MASK00_RA030);
            disp_write(base, MUX_RA030, val);
        }

        if ((g_display_res + i)->if_res == IF_RES_MIPI_DSI2) {
            /*MUX_RA038 CLKSEL Config*/
            pixel_map = to_dsi_pixel_map((g_display_res + i)->pixel_bpp);
            val = disp_read(base, MUX_RA038);
            val = reg_value(pixel_map, val,
                MUX_SHIFT00_RA030, MUX_MASK00_RA030);
            disp_write(base, MUX_RA038, val);
        }

        if ((g_display_res + i)->if_res == IF_RES_DSI1_DSI2) {
            /*MUX_RA030 CLKSEL Config*/
            pixel_map = to_dsi_pixel_map((g_display_res + i)->pixel_bpp);
            val = disp_read(base, MUX_RA030);
            val = reg_value(pixel_map, val,
                MUX_SHIFT00_RA030, MUX_MASK00_RA030);
            disp_write(base, MUX_RA030, val);

            /*MUX_RA038 CLKSEL Config*/
            val = disp_read(base, MUX_RA038);
            val = reg_value(pixel_map, val,
                MUX_SHIFT00_RA030, MUX_MASK00_RA030);
            disp_write(base, MUX_RA038, val);
        }
    }
}

void disp_dsi_to_csi_config(int if_res, int line_num,
    int hsize, int pixel_bpp, int colorcoding)
{
    unsigned int reg_ctrl1, reg_ctrl2;
    int csi_datatype, dsi_datatype;
    int val;
    addr_t base = get_disp_mux_reg_base();

    switch (if_res) {
    case IF_RES_MIPI_DSI1:
        reg_ctrl1 = MUX_RA030;
        reg_ctrl2 = MUX_RA034;
        break;
    case IF_RES_MIPI_DSI2:
        reg_ctrl1 = MUX_RA038;
        reg_ctrl2 = MUX_RA03C;
        break;
    default:
        DISPERR("dsi get index input is not correct\n");
        return;
    }

    switch (pixel_bpp) {
    case PIXEL_BPP_16:
        csi_datatype = 0x1e;
        dsi_datatype = 0x0e;
        break;
    case PIXEL_BPP_18:
        csi_datatype = 0x23;
        dsi_datatype = 0x1e;
        break;
    case PIXEL_BPP_24:
        csi_datatype = 0x24;
        dsi_datatype = 0x3e;
        break;
    default:
        csi_datatype = 0x24;
        dsi_datatype = 0x3e;
        break;
    }

    val = disp_read(base, reg_ctrl2);

    val = reg_value(1, val,
        MUX_SHIFT12_RA034, MUX_MASK12_RA034);
    val = reg_value(line_num, val,
        MUX_SHIFT13_RA034, MUX_MASK13_RA034);
    disp_write(base, reg_ctrl2, val);


    val = disp_read(base, reg_ctrl1);

    val = reg_value(1, val,
        MUX_SHIFT07_RA030, MUX_MASK07_RA030);
    val = reg_value(csi_datatype, val,
        MUX_SHIFT14_RA030, MUX_MASK14_RA030);
    val = reg_value(dsi_datatype, val,
        MUX_SHIFT08_RA030, MUX_MASK08_RA030);
    val = reg_value(1, val,
        MUX_SHIFT06_RA030, MUX_MASK06_RA030);
    disp_write(base, reg_ctrl1, val);
}

#ifdef _CONFIG_OS_SAFETY_
static int disp_link_get_cfg_info(void)
{
    addr_t addr_base = 0;
    addr_t *addr_ptr = NULL;
    static int cfg_get = 0;

    if (cfg_get)
        return 0;

    get_config_info(MODULE_DISPLAY_LINK, &addr_base);
    if (!addr_base) {
        DISPERR("Link get config info failed\n");
        return -1;
    }
    addr_ptr = (addr_t *)addr_base;
    res_count = *addr_ptr;
    g_display_res = (struct display_resource *)((int*)addr_ptr + 1);
    cfg_get = 1;

    return 0;
}
#else
static int disp_link_get_cfg_info(void)
{
    return 0;
}
#endif

int disp_link_init(uint8_t *dc_pll, struct display_resource *disp_res, unsigned int res_num)
{
    int ret;
    struct disp_link *link;
    struct disp_dp_link *dp_link;
    struct disp_if_link *if_link;
    unsigned int i = 0;

    if (!disp_res)
        disp_link_get_cfg_info();
    else {
        g_display_res = disp_res;
        res_count     = res_num;
    }

    for (i = 0; i < DC_NUM; i++) {
        dc_pll_sel[i] = dc_pll[i];
    }

    link = malloc(sizeof(struct disp_link));
    if (!link) {
        DISPERR("link malloc failed\n");
        return -1;
    }
    memset(link, 0, sizeof(struct disp_link));

    dp_link = &link->dp_link;
    if_link = &link->if_link;

    dp_link->dp1_link = -1;
    dp_link->dp2_link = -1;
    dp_link->dp3_link = -1;

    if_link->link_dsi1 = -1;
    if_link->link_dsi2 = -1;
    if_link->link_parallel = -1;
    if_link->link_lvds_combine = -1;
    if_link->link_lvds_group1_ch1 = -1;
    if_link->link_lvds_group1_ch2 = -1;
    if_link->link_lvds_group2_ch1 = -1;
    if_link->link_lvds_group2_ch2 = -1;

    for(i = 0; i < res_count; i++) {
        conv_to_dplink(g_display_res + i, dp_link);
        conv_to_iflink(g_display_res + i, if_link);
    }

    ret = dp_link_init(dp_link);
    if (ret) {
        DISPERR("dp link init failed.\n");
        goto FREE;
    }

#ifdef PROJECT_BF200
    if_link->link_parallel = LINK_DC3;
#endif

    if_link_init(if_link);
    disp_lvds_wrap_init(if_link);
    disp_dsi_pixel_map_init();

    DISPMSG("disp link init done\n");

FREE:
    free(link);

    return 0;
}

int disp_get_clks(struct disp_pll_clks* pclks)
{
    int pll_id;
    disp_link_get_cfg_info();

    pclks->count = res_count;
    pclks->pll_clks = malloc(sizeof(struct disp_pll_clk) * pclks->count);
    if (!pclks->pll_clks) {
        DISPERR("pll clks malloc failed\n");
        return -1;
    }

    for (unsigned int i = 0; i < res_count; i++) {
        pll_id = get_wrap_id(i);
        pclks->pll_clks[i].pll_id = pll_id;
        pclks->pll_clks[i].clk = (g_display_res + i)->clk;
    }

    return 0;
}
