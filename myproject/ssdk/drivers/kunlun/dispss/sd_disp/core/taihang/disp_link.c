/*
* disp_link.c
*
* Copyright (c) 2019 Semidrive Semiconductor.
* All rights reserved.
*
* Description:
*
* Revision History:
* -----------------
* 011, 02/14/2023 BI create this file
*/

#include <disp_link.h>
#include <disp_link_types.h>
#include <disp_reg_rw.h>
#include <disp_mux_reg.h>
#include <disp_drv_log.h>
#include <lvds_reg.h>

#include <stdlib.h>
#include <string.h>

static unsigned int res_count = 0;
static struct display_resource *g_display_res = NULL;
static uint8_t *dc_pll_sel = NULL;

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
        if_link->link_lvds1 = dres->dc_res;
        break;
    case IF_RES_LVDS2:
        if_link->link_lvds2 = dres->dc_res;
        break;
    case IF_RES_LVDS3:
        if_link->link_lvds3 = dres->dc_res;
        break;
    case IF_RES_LVDS4:
        if_link->link_lvds4 = dres->dc_res;
        break;
    case IF_RES_LVDS1_LVDS2:
        if (dres->dc_res == DC_RES_DC1_DC2) {
            if (!dres->dc_ms_link_port_swap) {
                if_link->link_lvds1 = DC_RES_DC1;
                if_link->link_lvds2 = DC_RES_DC2;
            } else {
                if_link->link_lvds1 = DC_RES_DC2;
                if_link->link_lvds2 = DC_RES_DC1;
            }
        } else if (dres->dc_res == DC_RES_DC3_DC4){
            if (!dres->dc_ms_link_port_swap) {
                if_link->link_lvds1 = DC_RES_DC3;
                if_link->link_lvds2 = DC_RES_DC4;
            } else {
                if_link->link_lvds1 = DC_RES_DC4;
                if_link->link_lvds2 = DC_RES_DC3;
            }
        } else {
            if_link->link_lvds1 = dres->dc_res;
            if_link->link_lvds2 = dres->dc_res;
        }

        if_link->link_lvds1_lvds2 = dres->dc_res;
        break;
    case IF_RES_LVDS3_LVDS4:
        if (dres->dc_res == DC_RES_DC1_DC2) {
            if (!dres->dc_ms_link_port_swap) {
                if_link->link_lvds3 = DC_RES_DC1;
                if_link->link_lvds4 = DC_RES_DC2;
            } else {
                if_link->link_lvds3 = DC_RES_DC2;
                if_link->link_lvds4 = DC_RES_DC1;
            }
        } else if (dres->dc_res == DC_RES_DC3_DC4){
            if (!dres->dc_ms_link_port_swap) {
                if_link->link_lvds3 = DC_RES_DC3;
                if_link->link_lvds4 = DC_RES_DC4;
            } else {
                if_link->link_lvds3 = DC_RES_DC4;
                if_link->link_lvds4 = DC_RES_DC3;
            }
        } else {
            if_link->link_lvds3 = dres->dc_res;
            if_link->link_lvds4 = dres->dc_res;
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

        break;
    default:
        break;
    }
}

static int dp_link_init(struct disp_dp_link *dp_link)
{
    int val;
    addr_t base = get_disp_mux_reg_base();

    val = disp_read(base, MUX_RB800);
    /*DP1 LINK*/
    switch (dp_link->dp1_link) {
    case LINK_DC1:
        val = reg_value(1, 0, MUX_SHIFT00_RB800, MUX_MASK00_RB800);
        break;
    case LINK_DC2:
        val = reg_value(0, 0, MUX_SHIFT00_RB800, MUX_MASK00_RB800);
        break;
    }

    /*DP2 LINK*/
    switch (dp_link->dp2_link) {
    case LINK_DC1:
        val = reg_value(1, val, MUX_SHIFT01_RB800, MUX_MASK01_RB800);
        break;
    case LINK_DC2:
        val = reg_value(0, val, MUX_SHIFT01_RB800, MUX_MASK01_RB800);
        break;
    }

    /*DP3 LINK*/
    switch (dp_link->dp3_link) {
    case LINK_DC3:
        val = reg_value(0, val, MUX_SHIFT02_RB800, MUX_MASK02_RB800);
        break;
    case LINK_DC4:
        val = reg_value(1, val, MUX_SHIFT02_RB800, MUX_MASK02_RB800);
        break;
    }

    /*Set DP LINK value to Register*/
    disp_write(base, MUX_RB800, val);

    return 0;
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

static int get_psr_sel_lvds1(int ps)
{
    switch (ps) {
        case PSR1_PS_SPLIT_0:
            return 0;
        case PSR2_PS_SPLIT_0:
            return 1;
        case PSR3_PS_SPLIT_0:
            return 2;
        default:
            DISPERR("LVDS1 MUX not support psr_sel (%d)!\n", ps);
            return -1;
    }
}

static int get_psr_sel_lvds2(int ps)
{
    switch (ps) {
        case PSR1_PS_SPLIT_1:
            return 0;
        case PSR2_PS_SPLIT_1:
            return 1;
        case PSR3_PS_SPLIT_1:
            return 2;
        default:
            DISPERR("LVDS2 MUX not support psr_sel (%d)!\n", ps);
            return -1;
    }
}

static int get_psr_sel_lvds3(int ps)
{
    switch (ps) {
        case PSR1_PS_SPLIT_2:
            return 0;
        case PSR2_PS_SPLIT_2:
            return 1;
        case PSR3_PS_SPLIT_0:
            return 2;
        default:
            DISPERR("LVDS3 MUX not support psr_sel (%d)!\n", ps);
            return -1;
    }
}

static int get_psr_sel_lvds4(int ps)
{
    switch (ps) {
        case PSR1_PS_SPLIT_3:
            return 0;
        case PSR2_PS_SPLIT_3:
            return 1;
        case PSR3_PS_SPLIT_1:
            return 2;
        default:
            DISPERR("LVDS4 MUX not support psr_sel (%d)!\n", ps);
            return -1;
    }
}

static int get_psr_sel_dsi1(int ps)
{
    switch (ps) {
        case PSR1_PS_SPLIT_1:
            return 0;
        case PSR2_PS_SPLIT_1:
            return 1;
        case PSR3_PS_SPLIT_0:
            return 2;
        default:
            DISPERR("DSI1 MUX not support psr_sel (%d)!\n", ps);
            return -1;
    }
}

static int get_psr_sel_dsi2(int ps)
{
    switch (ps) {
        case PSR1_PS_SPLIT_2:
            return 0;
        case PSR2_PS_SPLIT_2:
            return 1;
        case PSR3_PS_SPLIT_1:
            return 2;
        default:
            DISPERR("DSI2 MUX not support psr_sel (%d)!\n", ps);
            return -1;
    }
}

static int get_psr_sel_parallel(int ps)
{
    switch (ps) {
        case PSR1_PS_SPLIT_0:
            return 0;
        case PSR2_PS_SPLIT_0:
            return 1;
        case PSR3_PS_SPLIT_0:
            return 2;
        default:
            DISPERR("PARALLEL MUX not support psr_sel (%d)!\n", ps);
            return -1;
    }
}

static int get_psr_sel(int if_sel, int ps)
{
    switch (if_sel) {
        case LVDS1_PSR_SEL:
            return get_psr_sel_lvds1(ps);
        case LVDS2_PSR_SEL:
            return get_psr_sel_lvds2(ps);
        case LVDS3_PSR_SEL:
            return get_psr_sel_lvds3(ps);
        case LVDS4_PSR_SEL:
            return get_psr_sel_lvds4(ps);
        case DSI1_PSR_SEL:
            return get_psr_sel_dsi1(ps);
        case DSI2_PSR_SEL:
            return get_psr_sel_dsi2(ps);
        case PARALLEL_PSR_SEL:
            return get_psr_sel_parallel(ps);
        default:
            DISPERR("if_sel input value error (%d)!\n", if_sel);
            return -1;
    }
}

static int if_link_dsi1(int dc_idx)
{
    int val, ps;
    addr_t base = get_disp_mux_reg_base();

    if (dc_idx < 0)
        return 0;

    switch (dc_idx) {
        case LINK_DC1:
            ps = get_psr_sel(DSI1_PSR_SEL, PSR1_PS_SPLIT_1);

            val = disp_read(base, MUX_RC000);
            val = reg_value(PSR_ACT_BYPASS, val, MUX_SHIFT00_RC000, MUX_MASK00_RC000);
            disp_write(base, MUX_RC000, val);

            val = disp_read(base, MUX_RC004);
            val = reg_value(PSR_COMB_ACT_DISABLE, val, MUX_SHIFT01_RC004, MUX_MASK01_RC004);
            val = reg_value(PSR_SIN_ACT_ENABLE, val, MUX_SHIFT00_RC004, MUX_MASK00_RC004);
            disp_write(base, MUX_RC004, val);
            break;
        case LINK_DC2:
            ps = get_psr_sel(DSI1_PSR_SEL, PSR1_PS_SPLIT_1);

            val = disp_read(base, MUX_RC000);
            val = reg_value(PSR_ACT_BYPASS, val, MUX_SHIFT01_RC000, MUX_MASK01_RC000);
            disp_write(base, MUX_RC000, val);

            val = disp_read(base, MUX_RC004);
            val = reg_value(PSR_COMB_ACT_DISABLE, val, MUX_SHIFT01_RC004, MUX_MASK01_RC004);
            val = reg_value(PSR_SIN_ACT_ENABLE, val, MUX_SHIFT00_RC004, MUX_MASK00_RC004);
            disp_write(base, MUX_RC004, val);

            val = disp_read(base, MUX_RC010);
            val = reg_value(PSR_MUX_SEL_PSR2, val, MUX_SHIFT02_RC010, MUX_MASK02_RC010);
            disp_write(base, MUX_RC010, val);
            break;
        case LINK_DC3:
            ps = get_psr_sel(DSI1_PSR_SEL, PSR2_PS_SPLIT_1);

            val = disp_read(base, MUX_RC100);
            val = reg_value(PSR_ACT_BYPASS, val, MUX_SHIFT00_RC000, MUX_MASK00_RC000);
            disp_write(base, MUX_RC100, val);

            val = disp_read(base, MUX_RC104);
            val = reg_value(PSR_COMB_ACT_DISABLE, val, MUX_SHIFT01_RC004, MUX_MASK01_RC004);
            val = reg_value(PSR_SIN_ACT_ENABLE, val, MUX_SHIFT00_RC004, MUX_MASK00_RC004);
            disp_write(base, MUX_RC104, val);
            break;
        case LINK_DC4:
            ps = get_psr_sel(DSI1_PSR_SEL, PSR2_PS_SPLIT_1);

            val = disp_read(base, MUX_RC100);
            val = reg_value(PSR_ACT_BYPASS, val, MUX_SHIFT01_RC000, MUX_MASK01_RC000);
            disp_write(base, MUX_RC100, val);

            val = disp_read(base, MUX_RC104);
            val = reg_value(PSR_COMB_ACT_DISABLE, val, MUX_SHIFT01_RC004, MUX_MASK01_RC004);
            val = reg_value(PSR_SIN_ACT_ENABLE, val, MUX_SHIFT00_RC004, MUX_MASK00_RC004);
            disp_write(base, MUX_RC104, val);

            val = disp_read(base, MUX_RC110);
            val = reg_value(PSR_MUX_SEL_PSR2, val, MUX_SHIFT02_RC010, MUX_MASK02_RC010);
            disp_write(base, MUX_RC110, val);
            break;
        case LINK_DC5:
            ps = get_psr_sel(DSI1_PSR_SEL, PSR3_PS_SPLIT_0);

            val = disp_read(base, MUX_RC200);
            val = reg_value(PSR_ACT_BYPASS, val, MUX_SHIFT00_RC000, MUX_MASK00_RC000);
            disp_write(base, MUX_RC200, val);

            val = disp_read(base, MUX_RC204);
            val = reg_value(PSR_COMB_ACT_DISABLE, val, MUX_SHIFT01_RC004, MUX_MASK01_RC004);
            val = reg_value(PSR_SIN_ACT_ENABLE, val, MUX_SHIFT00_RC004, MUX_MASK00_RC004);
            disp_write(base, MUX_RC204, val);
            break;
         default:
            DISPERR("link_dsi1 input dc_idx (%d) error!\n", dc_idx);
            return -1;
    }

    /*MUX_RB000*/
    val = disp_read(base, MUX_RB000);
    val = reg_value(1, val, MUX_SHIFT16_RA800, MUX_MASK16_RA800);
    val = reg_value(PS_BRIDGE_MODE_MASTER, val, MUX_SHIFT07_RA800, MUX_MASK07_RA800);
    val = reg_value(ps, val, MUX_SHIFT00_RA800, MUX_MASK00_RA800);
    disp_write(base, MUX_RB000, val);

    /*MUX_RB008 CLKSEL Config*/
    val = disp_read(base, MUX_RB008);
    val = reg_value(DSI_CLKSEL_CLK_GENERATION,val,
        MUX_SHIFT09_RB008, MUX_MASK09_RB008);
    disp_write(base, MUX_RB008, val);

    return 0;
}

static int if_link_dsi2(int dc_idx)
{
    int val, ps;
    addr_t base = get_disp_mux_reg_base();

    if (dc_idx < 0)
        return 0;

    switch (dc_idx) {
        case LINK_DC1:
            ps = get_psr_sel(DSI2_PSR_SEL, PSR1_PS_SPLIT_2);

            val = disp_read(base, MUX_RC000);
            val = reg_value(PSR_ACT_BYPASS, val, MUX_SHIFT00_RC000, MUX_MASK00_RC000);
            disp_write(base, MUX_RC000, val);

            val = disp_read(base, MUX_RC004);
            val = reg_value(PSR_COMB_ACT_DISABLE, val, MUX_SHIFT01_RC004, MUX_MASK01_RC004);
            val = reg_value(PSR_SIN_ACT_ENABLE, val, MUX_SHIFT00_RC004, MUX_MASK00_RC004);
            disp_write(base, MUX_RC004, val);

            val = disp_read(base, MUX_RC010);
            val = reg_value(PSR_MUX_SEL_PSR0, val, MUX_SHIFT04_RC010, MUX_MASK04_RC010);
            disp_write(base, MUX_RC010, val);
            break;
        case LINK_DC2:
            ps = get_psr_sel(DSI2_PSR_SEL, PSR1_PS_SPLIT_2);

            val = disp_read(base, MUX_RC000);
            val = reg_value(PSR_ACT_BYPASS, val, MUX_SHIFT01_RC000, MUX_MASK01_RC000);
            disp_write(base, MUX_RC000, val);

            val = disp_read(base, MUX_RC004);
            val = reg_value(PSR_COMB_ACT_DISABLE, val, MUX_SHIFT01_RC004, MUX_MASK01_RC004);
            val = reg_value(PSR_SIN_ACT_ENABLE, val, MUX_SHIFT00_RC004, MUX_MASK00_RC004);
            disp_write(base, MUX_RC004, val);
            break;
        case LINK_DC3:
            ps = get_psr_sel(DSI2_PSR_SEL, PSR2_PS_SPLIT_2);

            val = disp_read(base, MUX_RC100);
            val = reg_value(PSR_ACT_BYPASS, val, MUX_SHIFT00_RC000, MUX_MASK00_RC000);
            disp_write(base, MUX_RC100, val);

            val = disp_read(base, MUX_RC104);
            val = reg_value(PSR_COMB_ACT_DISABLE, val, MUX_SHIFT01_RC004, MUX_MASK01_RC004);
            val = reg_value(PSR_SIN_ACT_ENABLE, val, MUX_SHIFT00_RC004, MUX_MASK00_RC004);
            disp_write(base, MUX_RC104, val);

            val = disp_read(base, MUX_RC110);
            val = reg_value(PSR_MUX_SEL_PSR0, val, MUX_SHIFT04_RC010, MUX_MASK04_RC010);
            disp_write(base, MUX_RC110, val);
            break;
        case LINK_DC4:
            ps = get_psr_sel(DSI2_PSR_SEL, PSR2_PS_SPLIT_2);

            val = disp_read(base, MUX_RC100);
            val = reg_value(PSR_ACT_BYPASS, val, MUX_SHIFT01_RC000, MUX_MASK01_RC000);
            disp_write(base, MUX_RC100, val);

            val = disp_read(base, MUX_RC104);
            val = reg_value(PSR_COMB_ACT_DISABLE, val, MUX_SHIFT01_RC004, MUX_MASK01_RC004);
            val = reg_value(PSR_SIN_ACT_ENABLE, val, MUX_SHIFT00_RC004, MUX_MASK00_RC004);
            disp_write(base, MUX_RC104, val);
            break;
        case LINK_DC5:
            ps = get_psr_sel(DSI2_PSR_SEL, PSR3_PS_SPLIT_1);

            val = disp_read(base, MUX_RC200);
            val = reg_value(PSR_ACT_BYPASS, val, MUX_SHIFT00_RC000, MUX_MASK00_RC000);
            disp_write(base, MUX_RC200, val);

            val = disp_read(base, MUX_RC204);
            val = reg_value(PSR_COMB_ACT_DISABLE, val, MUX_SHIFT01_RC004, MUX_MASK01_RC004);
            val = reg_value(PSR_SIN_ACT_ENABLE, val, MUX_SHIFT00_RC004, MUX_MASK00_RC004);
            disp_write(base, MUX_RC204, val);
            break;
         default:
            DISPERR("link_dsi2 input dc_idx (%d) error!\n", dc_idx);
            return -1;
    }

    /*MUX_RB000*/
    val = disp_read(base, MUX_RB100);
    val = reg_value(1, val, MUX_SHIFT16_RA800, MUX_MASK16_RA800);
    val = reg_value(PS_BRIDGE_MODE_MASTER, val, MUX_SHIFT07_RA800, MUX_MASK07_RA800);
    val = reg_value(ps, val, MUX_SHIFT00_RA800, MUX_MASK00_RA800);
    disp_write(base, MUX_RB100, val);

    /*MUX_RB008 CLKSEL Config*/
    val = disp_read(base, MUX_RB108);
    val = reg_value(DSI_CLKSEL_CLK_GENERATION,val,
        MUX_SHIFT09_RB008, MUX_MASK09_RB008);
    disp_write(base, MUX_RB108, val);

    return 0;
}

static int if_link_parallel(int dc_idx)
{
    int val, ps;
    addr_t base = get_disp_mux_reg_base();

    if (dc_idx < 0)
        return 0;

    switch (dc_idx) {
        case LINK_DC1:
            ps = get_psr_sel(PARALLEL_PSR_SEL, PSR1_PS_SPLIT_0);

            val = disp_read(base, MUX_RC000);
            val = reg_value(PSR_ACT_BYPASS, val, MUX_SHIFT00_RC000, MUX_MASK00_RC000);
            disp_write(base, MUX_RC000, val);

            val = disp_read(base, MUX_RC004);
            val = reg_value(PSR_COMB_ACT_DISABLE, val, MUX_SHIFT01_RC004, MUX_MASK01_RC004);
            val = reg_value(PSR_SIN_ACT_ENABLE, val, MUX_SHIFT00_RC004, MUX_MASK00_RC004);
            disp_write(base, MUX_RC004, val);
            break;
        case LINK_DC2:
            ps = get_psr_sel(PARALLEL_PSR_SEL, PSR1_PS_SPLIT_0);

            val = disp_read(base, MUX_RC000);
            val = reg_value(PSR_ACT_BYPASS, val, MUX_SHIFT01_RC000, MUX_MASK01_RC000);
            disp_write(base, MUX_RC000, val);

            val = disp_read(base, MUX_RC004);
            val = reg_value(PSR_COMB_ACT_DISABLE, val, MUX_SHIFT01_RC004, MUX_MASK01_RC004);
            val = reg_value(PSR_SIN_ACT_ENABLE, val, MUX_SHIFT00_RC004, MUX_MASK00_RC004);
            disp_write(base, MUX_RC004, val);

            val = disp_read(base, MUX_RC010);
            val = reg_value(PSR_MUX_SEL_PSR2, val, MUX_SHIFT00_RC010, MUX_MASK00_RC010);
            disp_write(base, MUX_RC110, val);
            break;
        case LINK_DC3:
            ps = get_psr_sel(PARALLEL_PSR_SEL, PSR2_PS_SPLIT_0);

            val = disp_read(base, MUX_RC100);
            val = reg_value(PSR_ACT_BYPASS, val, MUX_SHIFT00_RC000, MUX_MASK00_RC000);
            disp_write(base, MUX_RC100, val);

            val = disp_read(base, MUX_RC104);
            val = reg_value(PSR_COMB_ACT_DISABLE, val, MUX_SHIFT01_RC004, MUX_MASK01_RC004);
            val = reg_value(PSR_SIN_ACT_ENABLE, val, MUX_SHIFT00_RC004, MUX_MASK00_RC004);
            disp_write(base, MUX_RC104, val);
            break;
        case LINK_DC4:
            ps = get_psr_sel(PARALLEL_PSR_SEL, PSR2_PS_SPLIT_2);

            val = disp_read(base, MUX_RC100);
            val = reg_value(PSR_ACT_BYPASS, val, MUX_SHIFT01_RC000, MUX_MASK01_RC000);
            disp_write(base, MUX_RC100, val);

            val = disp_read(base, MUX_RC104);
            val = reg_value(PSR_COMB_ACT_DISABLE, val, MUX_SHIFT01_RC004, MUX_MASK01_RC004);
            val = reg_value(PSR_SIN_ACT_ENABLE, val, MUX_SHIFT00_RC004, MUX_MASK00_RC004);
            disp_write(base, MUX_RC104, val);

            val = disp_read(base, MUX_RC110);
            val = reg_value(PSR_MUX_SEL_PSR2, val, MUX_SHIFT00_RC010, MUX_MASK00_RC010);
            disp_write(base, MUX_RC110, val);
            break;
        case LINK_DC5:
            ps = get_psr_sel(PARALLEL_PSR_SEL, PSR3_PS_SPLIT_0);

            val = disp_read(base, MUX_RC200);
            val = reg_value(PSR_ACT_BYPASS, val, MUX_SHIFT00_RC000, MUX_MASK00_RC000);
            disp_write(base, MUX_RC200, val);

            val = disp_read(base, MUX_RC204);
            val = reg_value(PSR_COMB_ACT_DISABLE, val, MUX_SHIFT01_RC004, MUX_MASK01_RC004);
            val = reg_value(PSR_SIN_ACT_ENABLE, val, MUX_SHIFT00_RC004, MUX_MASK00_RC004);
            disp_write(base, MUX_RC204, val);
            break;
         default:
            DISPERR("link_parallel input dc_idx (%d) error!\n", dc_idx);
            return -1;
    }

    /*MUX_RB900*/
    val = disp_read(base, MUX_RB900);
    val = reg_value(1, val, MUX_SHIFT16_RA800, MUX_MASK16_RA800);
    val = reg_value(PS_BRIDGE_MODE_MASTER, val, MUX_SHIFT07_RA800, MUX_MASK07_RA800);
    val = reg_value(ps, val, MUX_SHIFT00_RA800, MUX_MASK00_RA800);
    disp_write(base, MUX_RB900, val);

    return 0;
}


static int if_link_lvds_comb(int dc_idx)
{
    int val, ps;
    addr_t base = get_disp_mux_reg_base();
    addr_t base_lvds = get_lvds_reg_base();

    if (dc_idx < 0)
        return 0;

    switch (dc_idx) {
        case CMB_LINK_DC1_DC2:
            val = disp_read(base, MUX_RC000);
            val = reg_value(PSR_ACT_ENABLE, val, MUX_SHIFT00_RC000, MUX_MASK00_RC000);
            val = reg_value(PSR_ACT_ENABLE, val, MUX_SHIFT01_RC000, MUX_MASK01_RC000);
            disp_write(base, MUX_RC000, val);

            val = disp_read(base, MUX_RC004);
            val = reg_value(PSR_CH_SPLIT_ENABLE, val, MUX_SHIFT19_RC004, MUX_MASK19_RC004);
            val = reg_value(PSR_CH_SPLIT_ENABLE, val, MUX_SHIFT03_RC004, MUX_MASK03_RC004);
            val = reg_value(PSR_COMB_ACT_ENABLE, val, MUX_SHIFT01_RC004, MUX_MASK01_RC004);
            val = reg_value(PSR_SIN_ACT_DISABLE, val, MUX_SHIFT00_RC004, MUX_MASK00_RC004);
            disp_write(base, MUX_RC004, val);

            /*TODO: need config PSR_1_CH0_COMB_CTRL&& PSR_1_CH1_COMB_CTRL  billy*/

            /*LVSD1_MUX*/
            ps = get_psr_sel(LVDS1_PSR_SEL, PSR1_PS_SPLIT_0);
            val = disp_read(base, MUX_RA800);
            val = reg_value(1, val, MUX_SHIFT16_RA800, MUX_MASK16_RA800);
            val = reg_value(PS_BRIDGE_MODE_MASTER, val, MUX_SHIFT07_RA800, MUX_MASK07_RA800);
            val = reg_value(ps, val, MUX_SHIFT00_RA800, MUX_MASK00_RA800);
            disp_write(base, MUX_RA800, val);

            /*LVSD2_MUX*/
            ps = get_psr_sel(LVDS2_PSR_SEL, PSR1_PS_SPLIT_1);
            val = disp_read(base, MUX_RA900);
            val = reg_value(1, val, MUX_SHIFT16_RA800, MUX_MASK16_RA800);
            val = reg_value(PS_BRIDGE_MODE_SLAVE, val, MUX_SHIFT07_RA800, MUX_MASK07_RA800);
            val = reg_value(ps, val, MUX_SHIFT00_RA800, MUX_MASK00_RA800);
            disp_write(base, MUX_RA900, val);

            /*LVSD3_MUX*/
            ps = get_psr_sel(LVDS3_PSR_SEL, PSR1_PS_SPLIT_2);
            val = disp_read(base, MUX_RAA00);
            val = reg_value(1, val, MUX_SHIFT16_RA800, MUX_MASK16_RA800);
            val = reg_value(PS_BRIDGE_MODE_SLAVE, val, MUX_SHIFT07_RA800, MUX_MASK07_RA800);
            val = reg_value(ps, val, MUX_SHIFT00_RA800, MUX_MASK00_RA800);
            disp_write(base, MUX_RAA00, val);

            /*LVSD4_MUX*/
            ps = get_psr_sel(LVDS4_PSR_SEL, PSR1_PS_SPLIT_3);
            val = disp_read(base, MUX_RAB00);
            val = reg_value(1, val, MUX_SHIFT16_RA800, MUX_MASK16_RA800);
            val = reg_value(PS_BRIDGE_MODE_SLAVE, val, MUX_SHIFT07_RA800, MUX_MASK07_RA800);
            val = reg_value(DB_SEL_LVDS1, val, MUX_SHIFT02_RA800, MUX_MASK02_RA800);
            val = reg_value(ps, val, MUX_SHIFT00_RA800, MUX_MASK00_RA800);
            disp_write(base, MUX_RAB00, val);

            /*MUX_RC010*/
            val = disp_read(base, MUX_RC010);
            val = reg_value(PSR_MUX_SEL_PSR1, val, MUX_SHIFT04_RC010, MUX_MASK04_RC010);
            val = reg_value(PSR_MUX_SEL_PSR2, val, MUX_SHIFT02_RC010, MUX_MASK02_RC010);
            disp_write(base, MUX_RC010, val);

            /*LVDS MUX_SEL*/
            val = disp_read(base_lvds, LVDS_MUX_SEL);
            val = reg_value(dc_pll_sel[DC1], val, MUX5_SEL_SHIFT, MUX5_SEL_MASK);
            val = reg_value(dc_pll_sel[DC1], val, MUX6_SEL_SHIFT, MUX6_SEL_MASK);
            val = reg_value(dc_pll_sel[DC1], val, MUX7_SEL_SHIFT, MUX7_SEL_MASK);
            val = reg_value(dc_pll_sel[DC1], val, MUX8_SEL_SHIFT, MUX8_SEL_MASK);
            disp_write(base_lvds, LVDS_MUX_SEL, val);
            break;
        case CMB_LINK_DC3_DC4:
            val = disp_read(base, MUX_RC100);
            val = reg_value(PSR_ACT_ENABLE, val, MUX_SHIFT00_RC000, MUX_MASK00_RC000);
            val = reg_value(PSR_ACT_ENABLE, val, MUX_SHIFT01_RC000, MUX_MASK01_RC000);
            disp_write(base, MUX_RC100, val);

            val = disp_read(base, MUX_RC104);
            val = reg_value(PSR_CH_SPLIT_ENABLE, val, MUX_SHIFT19_RC004, MUX_MASK19_RC004);
            val = reg_value(PSR_CH_SPLIT_ENABLE, val, MUX_SHIFT03_RC004, MUX_MASK03_RC004);
            val = reg_value(PSR_COMB_ACT_ENABLE, val, MUX_SHIFT01_RC004, MUX_MASK01_RC004);
            val = reg_value(PSR_SIN_ACT_DISABLE, val, MUX_SHIFT00_RC004, MUX_MASK00_RC004);
            disp_write(base, MUX_RC104, val);

            /*TODO: need config PSR_2_CH0_COMB_CTRL&& PSR_2_CH1_COMB_CTRL  billy*/

            /*LVSD1_MUX*/
            ps = get_psr_sel(LVDS1_PSR_SEL, PSR2_PS_SPLIT_0);
            val = disp_read(base, MUX_RA800);
            val = reg_value(1, val, MUX_SHIFT16_RA800, MUX_MASK16_RA800);
            val = reg_value(PS_BRIDGE_MODE_MASTER, val, MUX_SHIFT07_RA800, MUX_MASK07_RA800);
            val = reg_value(ps, val, MUX_SHIFT00_RA800, MUX_MASK00_RA800);
            disp_write(base, MUX_RA800, val);

            /*LVSD2_MUX*/
            ps = get_psr_sel(LVDS2_PSR_SEL, PSR2_PS_SPLIT_1);
            val = disp_read(base, MUX_RA900);
            val = reg_value(1, val, MUX_SHIFT16_RA800, MUX_MASK16_RA800);
            val = reg_value(PS_BRIDGE_MODE_SLAVE, val, MUX_SHIFT07_RA800, MUX_MASK07_RA800);
            val = reg_value(ps, val, MUX_SHIFT00_RA800, MUX_MASK00_RA800);
            disp_write(base, MUX_RA900, val);

            /*LVSD3_MUX*/
            ps = get_psr_sel(LVDS3_PSR_SEL, PSR2_PS_SPLIT_2);
            val = disp_read(base, MUX_RAA00);
            val = reg_value(1, val, MUX_SHIFT16_RA800, MUX_MASK16_RA800);
            val = reg_value(PS_BRIDGE_MODE_SLAVE, val, MUX_SHIFT07_RA800, MUX_MASK07_RA800);
            val = reg_value(ps, val, MUX_SHIFT00_RA800, MUX_MASK00_RA800);
            disp_write(base, MUX_RAA00, val);

            /*LVSD4_MUX*/
            ps = get_psr_sel(LVDS4_PSR_SEL, PSR2_PS_SPLIT_3);
            val = disp_read(base, MUX_RAB00);
            val = reg_value(1, val, MUX_SHIFT16_RA800, MUX_MASK16_RA800);
            val = reg_value(PS_BRIDGE_MODE_SLAVE, val, MUX_SHIFT07_RA800, MUX_MASK07_RA800);
            val = reg_value(DB_SEL_LVDS1, val, MUX_SHIFT02_RA800, MUX_MASK02_RA800);
            val = reg_value(ps, val, MUX_SHIFT00_RA800, MUX_MASK00_RA800);
            disp_write(base, MUX_RAB00, val);

            /*MUX_RC010*/
            val = disp_read(base, MUX_RC110);
            val = reg_value(PSR_MUX_SEL_PSR1, val, MUX_SHIFT04_RC010, MUX_MASK04_RC010);
            val = reg_value(PSR_MUX_SEL_PSR2, val, MUX_SHIFT02_RC010, MUX_MASK02_RC010);
            disp_write(base, MUX_RC110, val);

            /*LVDS MUX_SEL*/
            val = disp_read(base_lvds, LVDS_MUX_SEL);
            val = reg_value(dc_pll_sel[DC3], val, MUX5_SEL_SHIFT, MUX5_SEL_MASK);
            val = reg_value(dc_pll_sel[DC3], val, MUX6_SEL_SHIFT, MUX6_SEL_MASK);
            val = reg_value(dc_pll_sel[DC3], val, MUX7_SEL_SHIFT, MUX7_SEL_MASK);
            val = reg_value(dc_pll_sel[DC3], val, MUX8_SEL_SHIFT, MUX8_SEL_MASK);
            disp_write(base_lvds, LVDS_MUX_SEL, val);
            break;
        default:
            DISPERR("lvds combine input param (%d) error!\n", dc_idx);
            return -1;
    }

    return 0;
}

static int if_link_lvds12_dual(int dc_idx)
{
    int val, ps;
    addr_t base = get_disp_mux_reg_base();
    addr_t base_lvds = get_lvds_reg_base();

    if (dc_idx < 0)
        return 0;

    switch (dc_idx) {
        case LINK_DC1:
            val = disp_read(base, MUX_RC000);
            val = reg_value(PSR_ACT_BYPASS, val, MUX_SHIFT00_RC000, MUX_MASK00_RC000);
            disp_write(base, MUX_RC000, val);

            val = disp_read(base, MUX_RC004);
            val = reg_value(PSR_COMB_ACT_DISABLE, val, MUX_SHIFT01_RC004, MUX_MASK01_RC004);
            val = reg_value(PSR_SIN_ACT_ENABLE, val, MUX_SHIFT00_RC004, MUX_MASK00_RC004);
            disp_write(base, MUX_RC004, val);

            /*LVSD1_MUX*/
            ps = get_psr_sel(LVDS1_PSR_SEL, PSR1_PS_SPLIT_0);
            val = disp_read(base, MUX_RA800);
            val = reg_value(1, val, MUX_SHIFT16_RA800, MUX_MASK16_RA800);
            val = reg_value(PS_BRIDGE_MODE_MASTER, val, MUX_SHIFT07_RA800, MUX_MASK07_RA800);
            val = reg_value(ps, val, MUX_SHIFT00_RA800, MUX_MASK00_RA800);
            disp_write(base, MUX_RA800, val);

            /*LVSD2_MUX*/
            ps = get_psr_sel(LVDS2_PSR_SEL, PSR1_PS_SPLIT_1);
            val = disp_read(base, MUX_RA900);
            val = reg_value(1, val, MUX_SHIFT16_RA800, MUX_MASK16_RA800);
            val = reg_value(PS_BRIDGE_MODE_SLAVE, val, MUX_SHIFT07_RA800, MUX_MASK07_RA800);
            val = reg_value(ps, val, MUX_SHIFT00_RA800, MUX_MASK00_RA800);
            disp_write(base, MUX_RA900, val);
            break;
        case LINK_DC2:
            val = disp_read(base, MUX_RC000);
            val = reg_value(PSR_ACT_BYPASS, val, MUX_SHIFT01_RC000, MUX_MASK01_RC000);
            disp_write(base, MUX_RC000, val);

            val = disp_read(base, MUX_RC004);
            val = reg_value(PSR_COMB_ACT_DISABLE, val, MUX_SHIFT01_RC004, MUX_MASK01_RC004);
            val = reg_value(PSR_SIN_ACT_ENABLE, val, MUX_SHIFT00_RC004, MUX_MASK00_RC004);
            disp_write(base, MUX_RC004, val);

            val = disp_read(base, MUX_RC010);
            val = reg_value(PSR_MUX_SEL_PSR2, val, MUX_SHIFT00_RC010, MUX_MASK00_RC010);
            val = reg_value(PSR_MUX_SEL_PSR3, val, MUX_SHIFT02_RC010, MUX_MASK02_RC010);
            disp_write(base, MUX_RC010, val);

            /*LVSD1_MUX*/
            ps = get_psr_sel(LVDS1_PSR_SEL, PSR1_PS_SPLIT_0);
            val = disp_read(base, MUX_RA800);
            val = reg_value(1, val, MUX_SHIFT16_RA800, MUX_MASK16_RA800);
            val = reg_value(PS_BRIDGE_MODE_MASTER, val, MUX_SHIFT07_RA800, MUX_MASK07_RA800);
            val = reg_value(ps, val, MUX_SHIFT00_RA800, MUX_MASK00_RA800);
            disp_write(base, MUX_RA800, val);

            /*LVSD2_MUX*/
            ps = get_psr_sel(LVDS2_PSR_SEL, PSR1_PS_SPLIT_1);
            val = disp_read(base, MUX_RA900);
            val = reg_value(1, val, MUX_SHIFT16_RA800, MUX_MASK16_RA800);
            val = reg_value(PS_BRIDGE_MODE_SLAVE, val, MUX_SHIFT07_RA800, MUX_MASK07_RA800);
            val = reg_value(ps, val, MUX_SHIFT00_RA800, MUX_MASK00_RA800);
            disp_write(base, MUX_RA900, val);
            break;
        case LINK_DC3:
            val = disp_read(base, MUX_RC100);
            val = reg_value(PSR_ACT_BYPASS, val, MUX_SHIFT00_RC000, MUX_MASK00_RC000);
            disp_write(base, MUX_RC100, val);

            val = disp_read(base, MUX_RC104);
            val = reg_value(PSR_COMB_ACT_DISABLE, val, MUX_SHIFT01_RC004, MUX_MASK01_RC004);
            val = reg_value(PSR_SIN_ACT_ENABLE, val, MUX_SHIFT00_RC004, MUX_MASK00_RC004);
            disp_write(base, MUX_RC104, val);

            /*LVSD1_MUX*/
            ps = get_psr_sel(LVDS1_PSR_SEL, PSR2_PS_SPLIT_0);
            val = disp_read(base, MUX_RA800);
            val = reg_value(1, val, MUX_SHIFT16_RA800, MUX_MASK16_RA800);
            val = reg_value(PS_BRIDGE_MODE_MASTER, val, MUX_SHIFT07_RA800, MUX_MASK07_RA800);
            val = reg_value(ps, val, MUX_SHIFT00_RA800, MUX_MASK00_RA800);
            disp_write(base, MUX_RA800, val);

            /*LVSD2_MUX*/
            ps = get_psr_sel(LVDS2_PSR_SEL, PSR2_PS_SPLIT_1);
            val = disp_read(base, MUX_RA900);
            val = reg_value(1, val, MUX_SHIFT16_RA800, MUX_MASK16_RA800);
            val = reg_value(PS_BRIDGE_MODE_SLAVE, val, MUX_SHIFT07_RA800, MUX_MASK07_RA800);
            val = reg_value(ps, val, MUX_SHIFT00_RA800, MUX_MASK00_RA800);
            disp_write(base, MUX_RA900, val);
            break;
        case LINK_DC4:
            val = disp_read(base, MUX_RC100);
            val = reg_value(PSR_ACT_BYPASS, val, MUX_SHIFT01_RC000, MUX_MASK01_RC000);
            disp_write(base, MUX_RC100, val);

            val = disp_read(base, MUX_RC104);
            val = reg_value(PSR_COMB_ACT_DISABLE, val, MUX_SHIFT01_RC004, MUX_MASK01_RC004);
            val = reg_value(PSR_SIN_ACT_ENABLE, val, MUX_SHIFT00_RC004, MUX_MASK00_RC004);
            disp_write(base, MUX_RC104, val);

            val = disp_read(base, MUX_RC110);
            val = reg_value(PSR_MUX_SEL_PSR2, val, MUX_SHIFT00_RC010, MUX_MASK00_RC010);
            val = reg_value(PSR_MUX_SEL_PSR3, val, MUX_SHIFT02_RC010, MUX_MASK02_RC010);
            disp_write(base, MUX_RC110, val);

            /*LVSD1_MUX*/
            ps = get_psr_sel(LVDS1_PSR_SEL, PSR2_PS_SPLIT_0);
            val = disp_read(base, MUX_RA800);
            val = reg_value(1, val, MUX_SHIFT16_RA800, MUX_MASK16_RA800);
            val = reg_value(PS_BRIDGE_MODE_MASTER, val, MUX_SHIFT07_RA800, MUX_MASK07_RA800);
            val = reg_value(ps, val, MUX_SHIFT00_RA800, MUX_MASK00_RA800);
            disp_write(base, MUX_RA800, val);

            /*LVSD2_MUX*/
            ps = get_psr_sel(LVDS2_PSR_SEL, PSR2_PS_SPLIT_1);
            val = disp_read(base, MUX_RA900);
            val = reg_value(1, val, MUX_SHIFT16_RA800, MUX_MASK16_RA800);
            val = reg_value(PS_BRIDGE_MODE_SLAVE, val, MUX_SHIFT07_RA800, MUX_MASK07_RA800);
            val = reg_value(ps, val, MUX_SHIFT00_RA800, MUX_MASK00_RA800);
            disp_write(base, MUX_RA900, val);
            break;
        case LINK_DC5:
            val = disp_read(base, MUX_RC200);
            val = reg_value(PSR_ACT_BYPASS, val, MUX_SHIFT00_RC000, MUX_MASK00_RC000);
            disp_write(base, MUX_RC200, val);

            val = disp_read(base, MUX_RC204);
            val = reg_value(PSR_COMB_ACT_DISABLE, val, MUX_SHIFT01_RC004, MUX_MASK01_RC004);
            val = reg_value(PSR_SIN_ACT_ENABLE, val, MUX_SHIFT00_RC004, MUX_MASK00_RC004);
            disp_write(base, MUX_RC204, val);

            /*LVSD1_MUX*/
            ps = get_psr_sel(LVDS1_PSR_SEL, PSR3_PS_SPLIT_0);
            val = disp_read(base, MUX_RA800);
            val = reg_value(1, val, MUX_SHIFT16_RA800, MUX_MASK16_RA800);
            val = reg_value(PS_BRIDGE_MODE_MASTER, val, MUX_SHIFT07_RA800, MUX_MASK07_RA800);
            val = reg_value(ps, val, MUX_SHIFT00_RA800, MUX_MASK00_RA800);
            disp_write(base, MUX_RA800, val);

            /*LVSD2_MUX*/
            ps = get_psr_sel(LVDS2_PSR_SEL, PSR3_PS_SPLIT_1);
            val = disp_read(base, MUX_RA900);
            val = reg_value(1, val, MUX_SHIFT16_RA800, MUX_MASK16_RA800);
            val = reg_value(PS_BRIDGE_MODE_SLAVE, val, MUX_SHIFT07_RA800, MUX_MASK07_RA800);
            val = reg_value(ps, val, MUX_SHIFT00_RA800, MUX_MASK00_RA800);
            disp_write(base, MUX_RA900, val);
            break;
         default:
            DISPERR("lvds12 dual mode input dc_idx (%d) error!\n", dc_idx);
            return -1;
    }

    /*config MUX_RA700*/
    val = disp_read(base, MUX_RA700);
    val = reg_value(1, val, MUX_SHIFT02_RA700, MUX_MASK02_RA700);
    disp_write(base, MUX_RA700, val);

    /*LVDS MUX_SEL*/
    val = disp_read(base_lvds, LVDS_MUX_SEL);
    val = reg_value(dc_pll_sel[dc_idx], val, MUX5_SEL_SHIFT, MUX5_SEL_MASK);
    val = reg_value(dc_pll_sel[dc_idx], val, MUX6_SEL_SHIFT, MUX6_SEL_MASK);
    disp_write(base_lvds, LVDS_MUX_SEL, val);

    return 0;
}

static int if_link_lvds34_dual(int dc_idx)
{
    int val, ps;
    addr_t base = get_disp_mux_reg_base();
    addr_t base_lvds = get_lvds_reg_base();

    if (dc_idx < 0)
        return 0;

    switch (dc_idx) {
        case LINK_DC1:
            val = disp_read(base, MUX_RC000);
            val = reg_value(PSR_ACT_BYPASS, val, MUX_SHIFT00_RC000, MUX_MASK00_RC000);
            disp_write(base, MUX_RC000, val);

            val = disp_read(base, MUX_RC004);
            val = reg_value(PSR_COMB_ACT_DISABLE, val, MUX_SHIFT01_RC004, MUX_MASK01_RC004);
            val = reg_value(PSR_SIN_ACT_ENABLE, val, MUX_SHIFT00_RC004, MUX_MASK00_RC004);
            disp_write(base, MUX_RC004, val);

            val = disp_read(base, MUX_RC010);
            val = reg_value(PSR_MUX_SEL_PSR0, val, MUX_SHIFT04_RC010, MUX_MASK04_RC010);
            val = reg_value(PSR_MUX_SEL_PSR1, val, MUX_SHIFT06_RC010, MUX_MASK06_RC010);
            disp_write(base, MUX_RC010, val);

            /*LVSD3_MUX*/
            ps = get_psr_sel(LVDS3_PSR_SEL, PSR1_PS_SPLIT_2);
            val = disp_read(base, MUX_RAA00);
            val = reg_value(1, val, MUX_SHIFT16_RA800, MUX_MASK16_RA800);
            val = reg_value(PS_BRIDGE_MODE_MASTER, val, MUX_SHIFT07_RA800, MUX_MASK07_RA800);
            val = reg_value(ps, val, MUX_SHIFT00_RA800, MUX_MASK00_RA800);
            disp_write(base, MUX_RAA00, val);

            /*LVSD4_MUX*/
            ps = get_psr_sel(LVDS4_PSR_SEL, PSR1_PS_SPLIT_3);
            val = disp_read(base, MUX_RAB00);
            val = reg_value(1, val, MUX_SHIFT16_RA800, MUX_MASK16_RA800);
            val = reg_value(PS_BRIDGE_MODE_SLAVE, val, MUX_SHIFT07_RA800, MUX_MASK07_RA800);
            val = reg_value(ps, val, MUX_SHIFT00_RA800, MUX_MASK00_RA800);
            disp_write(base, MUX_RAB00, val);
            break;
        case LINK_DC2:
            val = disp_read(base, MUX_RC000);
            val = reg_value(PSR_ACT_BYPASS, val, MUX_SHIFT01_RC000, MUX_MASK01_RC000);
            disp_write(base, MUX_RC000, val);

            val = disp_read(base, MUX_RC004);
            val = reg_value(PSR_COMB_ACT_DISABLE, val, MUX_SHIFT01_RC004, MUX_MASK01_RC004);
            val = reg_value(PSR_SIN_ACT_ENABLE, val, MUX_SHIFT00_RC004, MUX_MASK00_RC004);
            disp_write(base, MUX_RC004, val);

            /*LVSD3_MUX*/
            ps = get_psr_sel(LVDS3_PSR_SEL, PSR1_PS_SPLIT_2);
            val = disp_read(base, MUX_RAA00);
            val = reg_value(1, val, MUX_SHIFT16_RA800, MUX_MASK16_RA800);
            val = reg_value(PS_BRIDGE_MODE_MASTER, val, MUX_SHIFT07_RA800, MUX_MASK07_RA800);
            val = reg_value(ps, val, MUX_SHIFT00_RA800, MUX_MASK00_RA800);
            disp_write(base, MUX_RAA00, val);

            /*LVSD2_MUX*/
            ps = get_psr_sel(LVDS4_PSR_SEL, PSR1_PS_SPLIT_3);
            val = disp_read(base, MUX_RAB00);
            val = reg_value(1, val, MUX_SHIFT16_RA800, MUX_MASK16_RA800);
            val = reg_value(PS_BRIDGE_MODE_SLAVE, val, MUX_SHIFT07_RA800, MUX_MASK07_RA800);
            val = reg_value(ps, val, MUX_SHIFT00_RA800, MUX_MASK00_RA800);
            disp_write(base, MUX_RAB00, val);
            break;
        case LINK_DC3:
            val = disp_read(base, MUX_RC100);
            val = reg_value(PSR_ACT_BYPASS, val, MUX_SHIFT00_RC000, MUX_MASK00_RC000);
            disp_write(base, MUX_RC100, val);

            val = disp_read(base, MUX_RC104);
            val = reg_value(PSR_COMB_ACT_DISABLE, val, MUX_SHIFT01_RC004, MUX_MASK01_RC004);
            val = reg_value(PSR_SIN_ACT_ENABLE, val, MUX_SHIFT00_RC004, MUX_MASK00_RC004);
            disp_write(base, MUX_RC104, val);

            val = disp_read(base, MUX_RC110);
            val = reg_value(PSR_MUX_SEL_PSR0, val, MUX_SHIFT04_RC010, MUX_MASK04_RC010);
            val = reg_value(PSR_MUX_SEL_PSR1, val, MUX_SHIFT06_RC010, MUX_MASK06_RC010);
            disp_write(base, MUX_RC110, val);

            /*LVSD3_MUX*/
            ps = get_psr_sel(LVDS3_PSR_SEL, PSR2_PS_SPLIT_2);
            val = disp_read(base, MUX_RAA00);
            val = reg_value(1, val, MUX_SHIFT16_RA800, MUX_MASK16_RA800);
            val = reg_value(PS_BRIDGE_MODE_MASTER, val, MUX_SHIFT07_RA800, MUX_MASK07_RA800);
            val = reg_value(ps, val, MUX_SHIFT00_RA800, MUX_MASK00_RA800);
            disp_write(base, MUX_RAA00, val);

            /*LVSD4_MUX*/
            ps = get_psr_sel(LVDS4_PSR_SEL, PSR2_PS_SPLIT_3);
            val = disp_read(base, MUX_RAB00);
            val = reg_value(1, val, MUX_SHIFT16_RA800, MUX_MASK16_RA800);
            val = reg_value(PS_BRIDGE_MODE_SLAVE, val, MUX_SHIFT07_RA800, MUX_MASK07_RA800);
            val = reg_value(ps, val, MUX_SHIFT00_RA800, MUX_MASK00_RA800);
            disp_write(base, MUX_RAB00, val);
            break;
        case LINK_DC4:
            val = disp_read(base, MUX_RC100);
            val = reg_value(PSR_ACT_BYPASS, val, MUX_SHIFT01_RC000, MUX_MASK01_RC000);
            disp_write(base, MUX_RC100, val);

            val = disp_read(base, MUX_RC104);
            val = reg_value(PSR_COMB_ACT_DISABLE, val, MUX_SHIFT01_RC004, MUX_MASK01_RC004);
            val = reg_value(PSR_SIN_ACT_ENABLE, val, MUX_SHIFT00_RC004, MUX_MASK00_RC004);
            disp_write(base, MUX_RC104, val);

            /*LVSD3_MUX*/
            ps = get_psr_sel(LVDS3_PSR_SEL, PSR2_PS_SPLIT_2);
            val = disp_read(base, MUX_RAA00);
            val = reg_value(1, val, MUX_SHIFT16_RA800, MUX_MASK16_RA800);
            val = reg_value(PS_BRIDGE_MODE_MASTER, val, MUX_SHIFT07_RA800, MUX_MASK07_RA800);
            val = reg_value(ps, val, MUX_SHIFT00_RA800, MUX_MASK00_RA800);
            disp_write(base, MUX_RAA00, val);

            /*LVSD4_MUX*/
            ps = get_psr_sel(LVDS4_PSR_SEL, PSR2_PS_SPLIT_3);
            val = disp_read(base, MUX_RAB00);
            val = reg_value(1, val, MUX_SHIFT16_RA800, MUX_MASK16_RA800);
            val = reg_value(PS_BRIDGE_MODE_SLAVE, val, MUX_SHIFT07_RA800, MUX_MASK07_RA800);
            val = reg_value(ps, val, MUX_SHIFT00_RA800, MUX_MASK00_RA800);
            disp_write(base, MUX_RAB00, val);
            break;
        case LINK_DC5:
            val = disp_read(base, MUX_RC200);
            val = reg_value(PSR_ACT_BYPASS, val, MUX_SHIFT00_RC000, MUX_MASK00_RC000);
            disp_write(base, MUX_RC200, val);

            val = disp_read(base, MUX_RC204);
            val = reg_value(PSR_COMB_ACT_DISABLE, val, MUX_SHIFT01_RC004, MUX_MASK01_RC004);
            val = reg_value(PSR_SIN_ACT_ENABLE, val, MUX_SHIFT00_RC004, MUX_MASK00_RC004);
            disp_write(base, MUX_RC204, val);

            /*LVSD3_MUX*/
            ps = get_psr_sel(LVDS3_PSR_SEL, PSR3_PS_SPLIT_0);
            val = disp_read(base, MUX_RAA00);
            val = reg_value(1, val, MUX_SHIFT16_RA800, MUX_MASK16_RA800);
            val = reg_value(PS_BRIDGE_MODE_MASTER, val, MUX_SHIFT07_RA800, MUX_MASK07_RA800);
            val = reg_value(ps, val, MUX_SHIFT00_RA800, MUX_MASK00_RA800);
            disp_write(base, MUX_RAA00, val);

            /*LVSD4_MUX*/
            ps = get_psr_sel(LVDS4_PSR_SEL, PSR3_PS_SPLIT_1);
            val = disp_read(base, MUX_RAB00);
            val = reg_value(1, val, MUX_SHIFT16_RA800, MUX_MASK16_RA800);
            val = reg_value(PS_BRIDGE_MODE_SLAVE, val, MUX_SHIFT07_RA800, MUX_MASK07_RA800);
            val = reg_value(ps, val, MUX_SHIFT00_RA800, MUX_MASK00_RA800);
            disp_write(base, MUX_RAB00, val);
            break;
         default:
            DISPERR("lvds34 dual mode input dc_idx (%d) error!\n", dc_idx);
            return -1;
    }

    /*config MUX_RA700*/
    val = disp_read(base, MUX_RA700);
    val = reg_value(1, val, MUX_SHIFT01_RA700, MUX_MASK01_RA700);
    disp_write(base, MUX_RA700, val);

    /*LVDS MUX_SEL*/
    val = disp_read(base_lvds, LVDS_MUX_SEL);
    val = reg_value(dc_pll_sel[dc_idx], val, MUX7_SEL_SHIFT, MUX7_SEL_MASK);
    val = reg_value(dc_pll_sel[dc_idx], val, MUX8_SEL_SHIFT, MUX8_SEL_MASK);
    disp_write(base_lvds, LVDS_MUX_SEL, val);

    return 0;
}

static int if_link_lvds1(int dc_idx)
{
    int val, ps;
    addr_t base = get_disp_mux_reg_base();
    addr_t base_lvds = get_lvds_reg_base();

    if (dc_idx < 0)
        return 0;

    switch (dc_idx) {
        case LINK_DC1:
            val = disp_read(base, MUX_RC000);
            val = reg_value(PSR_ACT_BYPASS, val, MUX_SHIFT00_RC000, MUX_MASK00_RC000);
            disp_write(base, MUX_RC000, val);

            val = disp_read(base, MUX_RC004);
            val = reg_value(PSR_COMB_ACT_DISABLE, val, MUX_SHIFT01_RC004, MUX_MASK01_RC004);
            val = reg_value(PSR_SIN_ACT_ENABLE, val, MUX_SHIFT00_RC004, MUX_MASK00_RC004);
            disp_write(base, MUX_RC004, val);

            /*LVSD1_MUX*/
            ps = get_psr_sel(LVDS1_PSR_SEL, PSR1_PS_SPLIT_0);
            val = disp_read(base, MUX_RA800);
            val = reg_value(1, val, MUX_SHIFT16_RA800, MUX_MASK16_RA800);
            val = reg_value(PS_BRIDGE_MODE_MASTER, val, MUX_SHIFT07_RA800, MUX_MASK07_RA800);
            val = reg_value(ps, val, MUX_SHIFT00_RA800, MUX_MASK00_RA800);
            disp_write(base, MUX_RA800, val);
            break;
        case LINK_DC2:
            val = disp_read(base, MUX_RC000);
            val = reg_value(PSR_ACT_BYPASS, val, MUX_SHIFT01_RC000, MUX_MASK01_RC000);
            disp_write(base, MUX_RC000, val);

            val = disp_read(base, MUX_RC004);
            val = reg_value(PSR_COMB_ACT_DISABLE, val, MUX_SHIFT01_RC004, MUX_MASK01_RC004);
            val = reg_value(PSR_SIN_ACT_ENABLE, val, MUX_SHIFT00_RC004, MUX_MASK00_RC004);
            disp_write(base, MUX_RC004, val);

            val = disp_read(base, MUX_RC010);
            val = reg_value(PSR_MUX_SEL_PSR2, val, MUX_SHIFT00_RC010, MUX_MASK00_RC010);
            disp_write(base, MUX_RC010, val);

            /*LVSD1_MUX*/
            ps = get_psr_sel(LVDS1_PSR_SEL, PSR1_PS_SPLIT_0);
            val = disp_read(base, MUX_RA800);
            val = reg_value(1, val, MUX_SHIFT16_RA800, MUX_MASK16_RA800);
            val = reg_value(PS_BRIDGE_MODE_MASTER, val, MUX_SHIFT07_RA800, MUX_MASK07_RA800);
            val = reg_value(ps, val, MUX_SHIFT00_RA800, MUX_MASK00_RA800);
            disp_write(base, MUX_RA800, val);
            break;
        case LINK_DC3:
            val = disp_read(base, MUX_RC100);
            val = reg_value(PSR_ACT_BYPASS, val, MUX_SHIFT00_RC000, MUX_MASK00_RC000);
            disp_write(base, MUX_RC100, val);

            val = disp_read(base, MUX_RC104);
            val = reg_value(PSR_COMB_ACT_DISABLE, val, MUX_SHIFT01_RC004, MUX_MASK01_RC004);
            val = reg_value(PSR_SIN_ACT_ENABLE, val, MUX_SHIFT00_RC004, MUX_MASK00_RC004);
            disp_write(base, MUX_RC104, val);

            /*LVSD1_MUX*/
            ps = get_psr_sel(LVDS1_PSR_SEL, PSR2_PS_SPLIT_0);
            val = disp_read(base, MUX_RA800);
            val = reg_value(1, val, MUX_SHIFT16_RA800, MUX_MASK16_RA800);
            val = reg_value(PS_BRIDGE_MODE_MASTER, val, MUX_SHIFT07_RA800, MUX_MASK07_RA800);
            val = reg_value(ps, val, MUX_SHIFT00_RA800, MUX_MASK00_RA800);
            disp_write(base, MUX_RA800, val);
            break;
        case LINK_DC4:
            val = disp_read(base, MUX_RC100);
            val = reg_value(PSR_ACT_BYPASS, val, MUX_SHIFT01_RC000, MUX_MASK01_RC000);
            disp_write(base, MUX_RC100, val);

            val = disp_read(base, MUX_RC104);
            val = reg_value(PSR_COMB_ACT_DISABLE, val, MUX_SHIFT01_RC004, MUX_MASK01_RC004);
            val = reg_value(PSR_SIN_ACT_ENABLE, val, MUX_SHIFT00_RC004, MUX_MASK00_RC004);
            disp_write(base, MUX_RC104, val);

            val = disp_read(base, MUX_RC110);
            val = reg_value(PSR_MUX_SEL_PSR2, val, MUX_SHIFT00_RC010, MUX_MASK00_RC010);
            disp_write(base, MUX_RC110, val);

            /*LVSD1_MUX*/
            ps = get_psr_sel(LVDS1_PSR_SEL, PSR2_PS_SPLIT_0);
            val = disp_read(base, MUX_RA800);
            val = reg_value(1, val, MUX_SHIFT16_RA800, MUX_MASK16_RA800);
            val = reg_value(PS_BRIDGE_MODE_MASTER, val, MUX_SHIFT07_RA800, MUX_MASK07_RA800);
            val = reg_value(ps, val, MUX_SHIFT00_RA800, MUX_MASK00_RA800);
            disp_write(base, MUX_RA800, val);
            break;
        case LINK_DC5:
            val = disp_read(base, MUX_RC200);
            val = reg_value(PSR_ACT_BYPASS, val, MUX_SHIFT00_RC000, MUX_MASK00_RC000);
            disp_write(base, MUX_RC200, val);

            val = disp_read(base, MUX_RC204);
            val = reg_value(PSR_COMB_ACT_DISABLE, val, MUX_SHIFT01_RC004, MUX_MASK01_RC004);
            val = reg_value(PSR_SIN_ACT_ENABLE, val, MUX_SHIFT00_RC004, MUX_MASK00_RC004);
            disp_write(base, MUX_RC204, val);

            /*LVSD1_MUX*/
            ps = get_psr_sel(LVDS1_PSR_SEL, PSR3_PS_SPLIT_0);
            val = disp_read(base, MUX_RA800);
            val = reg_value(1, val, MUX_SHIFT16_RA800, MUX_MASK16_RA800);
            val = reg_value(PS_BRIDGE_MODE_MASTER, val, MUX_SHIFT07_RA800, MUX_MASK07_RA800);
            val = reg_value(ps, val, MUX_SHIFT00_RA800, MUX_MASK00_RA800);
            disp_write(base, MUX_RA800, val);
            break;
        default:
            DISPERR("lvds1 single mode input dc_idx (%d) error!\n", dc_idx);
            return -1;
    }

    /*LVDS MUX_SEL*/
    val = disp_read(base_lvds, LVDS_MUX_SEL);
    val = reg_value(dc_pll_sel[dc_idx], val, MUX5_SEL_SHIFT, MUX5_SEL_MASK);
    disp_write(base_lvds, LVDS_MUX_SEL, val);

    return 0;
}

static int if_link_lvds2(int dc_idx)
{
    int val, ps;
    addr_t base = get_disp_mux_reg_base();
    addr_t base_lvds = get_lvds_reg_base();

    if (dc_idx < 0)
        return 0;

    switch (dc_idx) {
        case LINK_DC1:
            val = disp_read(base, MUX_RC000);
            val = reg_value(PSR_ACT_BYPASS, val, MUX_SHIFT00_RC000, MUX_MASK00_RC000);
            disp_write(base, MUX_RC000, val);

            val = disp_read(base, MUX_RC004);
            val = reg_value(PSR_COMB_ACT_DISABLE, val, MUX_SHIFT01_RC004, MUX_MASK01_RC004);
            val = reg_value(PSR_SIN_ACT_ENABLE, val, MUX_SHIFT00_RC004, MUX_MASK00_RC004);
            disp_write(base, MUX_RC004, val);

            /*LVSD2_MUX*/
            ps = get_psr_sel(LVDS2_PSR_SEL, PSR1_PS_SPLIT_1);
            val = disp_read(base, MUX_RA900);
            val = reg_value(1, val, MUX_SHIFT16_RA800, MUX_MASK16_RA800);
            val = reg_value(PS_BRIDGE_MODE_MASTER, val, MUX_SHIFT07_RA800, MUX_MASK07_RA800);
            val = reg_value(ps, val, MUX_SHIFT00_RA800, MUX_MASK00_RA800);
            disp_write(base, MUX_RA900, val);
            break;
        case LINK_DC2:
            val = disp_read(base, MUX_RC000);
            val = reg_value(PSR_ACT_BYPASS, val, MUX_SHIFT01_RC000, MUX_MASK01_RC000);
            disp_write(base, MUX_RC000, val);

            val = disp_read(base, MUX_RC004);
            val = reg_value(PSR_COMB_ACT_DISABLE, val, MUX_SHIFT01_RC004, MUX_MASK01_RC004);
            val = reg_value(PSR_SIN_ACT_ENABLE, val, MUX_SHIFT00_RC004, MUX_MASK00_RC004);
            disp_write(base, MUX_RC004, val);

            val = disp_read(base, MUX_RC010);
            val = reg_value(PSR_MUX_SEL_PSR2, val, MUX_SHIFT02_RC010, MUX_MASK02_RC010);
            disp_write(base, MUX_RC010, val);

            /*LVSD2_MUX*/
            ps = get_psr_sel(LVDS2_PSR_SEL, PSR1_PS_SPLIT_1);
            val = disp_read(base, MUX_RA900);
            val = reg_value(1, val, MUX_SHIFT16_RA800, MUX_MASK16_RA800);
            val = reg_value(PS_BRIDGE_MODE_MASTER, val, MUX_SHIFT07_RA800, MUX_MASK07_RA800);
            val = reg_value(ps, val, MUX_SHIFT00_RA800, MUX_MASK00_RA800);
            disp_write(base, MUX_RA900, val);
            break;
        case LINK_DC3:
            val = disp_read(base, MUX_RC100);
            val = reg_value(PSR_ACT_BYPASS, val, MUX_SHIFT00_RC000, MUX_MASK00_RC000);
            disp_write(base, MUX_RC100, val);

            val = disp_read(base, MUX_RC104);
            val = reg_value(PSR_COMB_ACT_DISABLE, val, MUX_SHIFT01_RC004, MUX_MASK01_RC004);
            val = reg_value(PSR_SIN_ACT_ENABLE, val, MUX_SHIFT00_RC004, MUX_MASK00_RC004);
            disp_write(base, MUX_RC104, val);

            /*LVSD2_MUX*/
            ps = get_psr_sel(LVDS2_PSR_SEL, PSR2_PS_SPLIT_1);
            val = disp_read(base, MUX_RA900);
            val = reg_value(1, val, MUX_SHIFT16_RA800, MUX_MASK16_RA800);
            val = reg_value(PS_BRIDGE_MODE_MASTER, val, MUX_SHIFT07_RA800, MUX_MASK07_RA800);
            val = reg_value(ps, val, MUX_SHIFT00_RA800, MUX_MASK00_RA800);
            disp_write(base, MUX_RA900, val);
            break;
        case LINK_DC4:
            val = disp_read(base, MUX_RC100);
            val = reg_value(PSR_ACT_BYPASS, val, MUX_SHIFT01_RC000, MUX_MASK01_RC000);
            disp_write(base, MUX_RC100, val);

            val = disp_read(base, MUX_RC104);
            val = reg_value(PSR_COMB_ACT_DISABLE, val, MUX_SHIFT01_RC004, MUX_MASK01_RC004);
            val = reg_value(PSR_SIN_ACT_ENABLE, val, MUX_SHIFT00_RC004, MUX_MASK00_RC004);
            disp_write(base, MUX_RC104, val);

            val = disp_read(base, MUX_RC110);
            val = reg_value(PSR_MUX_SEL_PSR2, val, MUX_SHIFT02_RC010, MUX_MASK02_RC010);
            disp_write(base, MUX_RC110, val);

            /*LVSD2_MUX*/
            ps = get_psr_sel(LVDS2_PSR_SEL, PSR2_PS_SPLIT_1);
            val = disp_read(base, MUX_RA900);
            val = reg_value(1, val, MUX_SHIFT16_RA800, MUX_MASK16_RA800);
            val = reg_value(PS_BRIDGE_MODE_MASTER, val, MUX_SHIFT07_RA800, MUX_MASK07_RA800);
            val = reg_value(ps, val, MUX_SHIFT00_RA800, MUX_MASK00_RA800);
            disp_write(base, MUX_RA900, val);
            break;
        case LINK_DC5:
            val = disp_read(base, MUX_RC200);
            val = reg_value(PSR_ACT_BYPASS, val, MUX_SHIFT00_RC000, MUX_MASK00_RC000);
            disp_write(base, MUX_RC200, val);

            val = disp_read(base, MUX_RC204);
            val = reg_value(PSR_COMB_ACT_DISABLE, val, MUX_SHIFT01_RC004, MUX_MASK01_RC004);
            val = reg_value(PSR_SIN_ACT_ENABLE, val, MUX_SHIFT00_RC004, MUX_MASK00_RC004);
            disp_write(base, MUX_RC204, val);

            /*LVSD2_MUX*/
            ps = get_psr_sel(LVDS2_PSR_SEL, PSR3_PS_SPLIT_1);
            val = disp_read(base, MUX_RA900);
            val = reg_value(1, val, MUX_SHIFT16_RA800, MUX_MASK16_RA800);
            val = reg_value(PS_BRIDGE_MODE_MASTER, val, MUX_SHIFT07_RA800, MUX_MASK07_RA800);
            val = reg_value(ps, val, MUX_SHIFT00_RA800, MUX_MASK00_RA800);
            disp_write(base, MUX_RA900, val);
            break;
        default:
            DISPERR("lvds2 single mode input dc_idx (%d) error!\n", dc_idx);
            return -1;
    }

    /*LVDS MUX_SEL*/
    val = disp_read(base_lvds, LVDS_MUX_SEL);
    val = reg_value(dc_pll_sel[dc_idx], val, MUX6_SEL_SHIFT, MUX6_SEL_MASK);
    disp_write(base_lvds, LVDS_MUX_SEL, val);

    return 0;
}

static int if_link_lvds3(int dc_idx)
{
    int val, ps;
    addr_t base = get_disp_mux_reg_base();
    addr_t base_lvds = get_lvds_reg_base();

    if (dc_idx < 0)
        return 0;

    switch (dc_idx) {
        case LINK_DC1:
            val = disp_read(base, MUX_RC000);
            val = reg_value(PSR_ACT_BYPASS, val, MUX_SHIFT00_RC000, MUX_MASK00_RC000);
            disp_write(base, MUX_RC000, val);

            val = disp_read(base, MUX_RC004);
            val = reg_value(PSR_COMB_ACT_DISABLE, val, MUX_SHIFT01_RC004, MUX_MASK01_RC004);
            val = reg_value(PSR_SIN_ACT_ENABLE, val, MUX_SHIFT00_RC004, MUX_MASK00_RC004);
            disp_write(base, MUX_RC004, val);

            val = disp_read(base, MUX_RC010);
            val = reg_value(PSR_MUX_SEL_PSR0, val, MUX_SHIFT04_RC010, MUX_MASK04_RC010);
            disp_write(base, MUX_RC010, val);

            /*LVSD3_MUX*/
            ps = get_psr_sel(LVDS3_PSR_SEL, PSR1_PS_SPLIT_2);
            val = disp_read(base, MUX_RAA00);
            val = reg_value(1, val, MUX_SHIFT16_RA800, MUX_MASK16_RA800);
            val = reg_value(PS_BRIDGE_MODE_MASTER, val, MUX_SHIFT07_RA800, MUX_MASK07_RA800);
            val = reg_value(ps, val, MUX_SHIFT00_RA800, MUX_MASK00_RA800);
            disp_write(base, MUX_RAA00, val);
            break;
        case LINK_DC2:
            val = disp_read(base, MUX_RC000);
            val = reg_value(PSR_ACT_BYPASS, val, MUX_SHIFT01_RC000, MUX_MASK01_RC000);
            disp_write(base, MUX_RC000, val);

            val = disp_read(base, MUX_RC004);
            val = reg_value(PSR_COMB_ACT_DISABLE, val, MUX_SHIFT01_RC004, MUX_MASK01_RC004);
            val = reg_value(PSR_SIN_ACT_ENABLE, val, MUX_SHIFT00_RC004, MUX_MASK00_RC004);
            disp_write(base, MUX_RC004, val);

            /*LVSD3_MUX*/
            ps = get_psr_sel(LVDS3_PSR_SEL, PSR1_PS_SPLIT_2);
            val = disp_read(base, MUX_RAA00);
            val = reg_value(1, val, MUX_SHIFT16_RA800, MUX_MASK16_RA800);
            val = reg_value(PS_BRIDGE_MODE_MASTER, val, MUX_SHIFT07_RA800, MUX_MASK07_RA800);
            val = reg_value(ps, val, MUX_SHIFT00_RA800, MUX_MASK00_RA800);
            disp_write(base, MUX_RAA00, val);
            break;
        case LINK_DC3:
            val = disp_read(base, MUX_RC100);
            val = reg_value(PSR_ACT_BYPASS, val, MUX_SHIFT00_RC000, MUX_MASK00_RC000);
            disp_write(base, MUX_RC100, val);

            val = disp_read(base, MUX_RC104);
            val = reg_value(PSR_COMB_ACT_DISABLE, val, MUX_SHIFT01_RC004, MUX_MASK01_RC004);
            val = reg_value(PSR_SIN_ACT_ENABLE, val, MUX_SHIFT00_RC004, MUX_MASK00_RC004);
            disp_write(base, MUX_RC104, val);

            val = disp_read(base, MUX_RC110);
            val = reg_value(PSR_MUX_SEL_PSR0, val, MUX_SHIFT04_RC010, MUX_MASK04_RC010);
            disp_write(base, MUX_RC110, val);

            /*LVSD3_MUX*/
            ps = get_psr_sel(LVDS3_PSR_SEL, PSR2_PS_SPLIT_2);
            val = disp_read(base, MUX_RAA00);
            val = reg_value(1, val, MUX_SHIFT16_RA800, MUX_MASK16_RA800);
            val = reg_value(PS_BRIDGE_MODE_MASTER, val, MUX_SHIFT07_RA800, MUX_MASK07_RA800);
            val = reg_value(ps, val, MUX_SHIFT00_RA800, MUX_MASK00_RA800);
            disp_write(base, MUX_RAA00, val);
            break;
        case LINK_DC4:
            val = disp_read(base, MUX_RC100);
            val = reg_value(PSR_ACT_BYPASS, val, MUX_SHIFT01_RC000, MUX_MASK01_RC000);
            disp_write(base, MUX_RC100, val);

            val = disp_read(base, MUX_RC104);
            val = reg_value(PSR_COMB_ACT_DISABLE, val, MUX_SHIFT01_RC004, MUX_MASK01_RC004);
            val = reg_value(PSR_SIN_ACT_ENABLE, val, MUX_SHIFT00_RC004, MUX_MASK00_RC004);
            disp_write(base, MUX_RC104, val);

            /*LVSD3_MUX*/
            ps = get_psr_sel(LVDS3_PSR_SEL, PSR2_PS_SPLIT_2);
            val = disp_read(base, MUX_RAA00);
            val = reg_value(1, val, MUX_SHIFT16_RA800, MUX_MASK16_RA800);
            val = reg_value(PS_BRIDGE_MODE_MASTER, val, MUX_SHIFT07_RA800, MUX_MASK07_RA800);
            val = reg_value(ps, val, MUX_SHIFT00_RA800, MUX_MASK00_RA800);
            disp_write(base, MUX_RAA00, val);
            break;
        case LINK_DC5:
            val = disp_read(base, MUX_RC200);
            val = reg_value(PSR_ACT_BYPASS, val, MUX_SHIFT00_RC000, MUX_MASK00_RC000);
            disp_write(base, MUX_RC200, val);

            val = disp_read(base, MUX_RC204);
            val = reg_value(PSR_COMB_ACT_DISABLE, val, MUX_SHIFT01_RC004, MUX_MASK01_RC004);
            val = reg_value(PSR_SIN_ACT_ENABLE, val, MUX_SHIFT00_RC004, MUX_MASK00_RC004);
            disp_write(base, MUX_RC204, val);

            /*LVSD3_MUX*/
            ps = get_psr_sel(LVDS1_PSR_SEL, PSR3_PS_SPLIT_0);
            val = disp_read(base, MUX_RAA00);
            val = reg_value(1, val, MUX_SHIFT16_RA800, MUX_MASK16_RA800);
            val = reg_value(PS_BRIDGE_MODE_MASTER, val, MUX_SHIFT07_RA800, MUX_MASK07_RA800);
            val = reg_value(ps, val, MUX_SHIFT00_RA800, MUX_MASK00_RA800);
            disp_write(base, MUX_RAA00, val);
            break;
        default:
            DISPERR("lvds3 single mode input dc_idx (%d) error!\n", dc_idx);
            return -1;
    }

    /*LVDS MUX_SEL*/
    val = disp_read(base_lvds, LVDS_MUX_SEL);
    val = reg_value(dc_pll_sel[dc_idx], val, MUX7_SEL_SHIFT, MUX7_SEL_MASK);
    disp_write(base_lvds, LVDS_MUX_SEL, val);

    return 0;
}

static int if_link_lvds4(int dc_idx)
{
    int val, ps;
    addr_t base = get_disp_mux_reg_base();
    addr_t base_lvds = get_lvds_reg_base();

    if (dc_idx < 0)
        return 0;

    switch (dc_idx) {
        case LINK_DC1:
            val = disp_read(base, MUX_RC000);
            val = reg_value(PSR_ACT_BYPASS, val, MUX_SHIFT00_RC000, MUX_MASK00_RC000);
            disp_write(base, MUX_RC000, val);

            val = disp_read(base, MUX_RC004);
            val = reg_value(PSR_COMB_ACT_DISABLE, val, MUX_SHIFT01_RC004, MUX_MASK01_RC004);
            val = reg_value(PSR_SIN_ACT_ENABLE, val, MUX_SHIFT00_RC004, MUX_MASK00_RC004);
            disp_write(base, MUX_RC004, val);

            val = disp_read(base, MUX_RC010);
            val = reg_value(PSR_MUX_SEL_PSR0, val, MUX_SHIFT06_RC010, MUX_MASK06_RC010);
            disp_write(base, MUX_RC010, val);

            /*LVSD4_MUX*/
            ps = get_psr_sel(LVDS4_PSR_SEL, PSR1_PS_SPLIT_3);
            val = disp_read(base, MUX_RAB00);
            val = reg_value(1, val, MUX_SHIFT16_RA800, MUX_MASK16_RA800);
            val = reg_value(PS_BRIDGE_MODE_MASTER, val, MUX_SHIFT07_RA800, MUX_MASK07_RA800);
            val = reg_value(ps, val, MUX_SHIFT00_RA800, MUX_MASK00_RA800);
            disp_write(base, MUX_RAB00, val);
            break;
        case LINK_DC2:
            val = disp_read(base, MUX_RC000);
            val = reg_value(PSR_ACT_BYPASS, val, MUX_SHIFT01_RC000, MUX_MASK01_RC000);
            disp_write(base, MUX_RC000, val);

            val = disp_read(base, MUX_RC004);
            val = reg_value(PSR_COMB_ACT_DISABLE, val, MUX_SHIFT01_RC004, MUX_MASK01_RC004);
            val = reg_value(PSR_SIN_ACT_ENABLE, val, MUX_SHIFT00_RC004, MUX_MASK00_RC004);
            disp_write(base, MUX_RC004, val);

            /*LVSD4_MUX*/
            ps = get_psr_sel(LVDS4_PSR_SEL, PSR1_PS_SPLIT_3);
            val = disp_read(base, MUX_RAB00);
            val = reg_value(1, val, MUX_SHIFT16_RA800, MUX_MASK16_RA800);
            val = reg_value(PS_BRIDGE_MODE_MASTER, val, MUX_SHIFT07_RA800, MUX_MASK07_RA800);
            val = reg_value(ps, val, MUX_SHIFT00_RA800, MUX_MASK00_RA800);
            disp_write(base, MUX_RAB00, val);
            break;
        case LINK_DC3:
            val = disp_read(base, MUX_RC100);
            val = reg_value(PSR_ACT_BYPASS, val, MUX_SHIFT00_RC000, MUX_MASK00_RC000);
            disp_write(base, MUX_RC100, val);

            val = disp_read(base, MUX_RC104);
            val = reg_value(PSR_COMB_ACT_DISABLE, val, MUX_SHIFT01_RC004, MUX_MASK01_RC004);
            val = reg_value(PSR_SIN_ACT_ENABLE, val, MUX_SHIFT00_RC004, MUX_MASK00_RC004);
            disp_write(base, MUX_RC104, val);

            val = disp_read(base, MUX_RC110);
            val = reg_value(PSR_MUX_SEL_PSR0, val, MUX_SHIFT06_RC010, MUX_MASK06_RC010);
            disp_write(base, MUX_RC110, val);

            /*LVSD4_MUX*/
            ps = get_psr_sel(LVDS4_PSR_SEL, PSR2_PS_SPLIT_3);
            val = disp_read(base, MUX_RAB00);
            val = reg_value(1, val, MUX_SHIFT16_RA800, MUX_MASK16_RA800);
            val = reg_value(PS_BRIDGE_MODE_MASTER, val, MUX_SHIFT07_RA800, MUX_MASK07_RA800);
            val = reg_value(ps, val, MUX_SHIFT00_RA800, MUX_MASK00_RA800);
            disp_write(base, MUX_RAB00, val);
            break;
        case LINK_DC4:
            val = disp_read(base, MUX_RC100);
            val = reg_value(PSR_ACT_BYPASS, val, MUX_SHIFT01_RC000, MUX_MASK01_RC000);
            disp_write(base, MUX_RC100, val);

            val = disp_read(base, MUX_RC104);
            val = reg_value(PSR_COMB_ACT_DISABLE, val, MUX_SHIFT01_RC004, MUX_MASK01_RC004);
            val = reg_value(PSR_SIN_ACT_ENABLE, val, MUX_SHIFT00_RC004, MUX_MASK00_RC004);
            disp_write(base, MUX_RC104, val);

            /*LVSD4_MUX*/
            ps = get_psr_sel(LVDS4_PSR_SEL, PSR2_PS_SPLIT_3);
            val = disp_read(base, MUX_RAB00);
            val = reg_value(1, val, MUX_SHIFT16_RA800, MUX_MASK16_RA800);
            val = reg_value(PS_BRIDGE_MODE_MASTER, val, MUX_SHIFT07_RA800, MUX_MASK07_RA800);
            val = reg_value(ps, val, MUX_SHIFT00_RA800, MUX_MASK00_RA800);
            disp_write(base, MUX_RAB00, val);
            break;
        case LINK_DC5:
            val = disp_read(base, MUX_RC200);
            val = reg_value(PSR_ACT_BYPASS, val, MUX_SHIFT00_RC000, MUX_MASK00_RC000);
            disp_write(base, MUX_RC200, val);

            val = disp_read(base, MUX_RC204);
            val = reg_value(PSR_COMB_ACT_DISABLE, val, MUX_SHIFT01_RC004, MUX_MASK01_RC004);
            val = reg_value(PSR_SIN_ACT_ENABLE, val, MUX_SHIFT00_RC004, MUX_MASK00_RC004);
            disp_write(base, MUX_RC204, val);

            /*LVSD4_MUX*/
            ps = get_psr_sel(LVDS4_PSR_SEL, PSR3_PS_SPLIT_1);
            val = disp_read(base, MUX_RAB00);
            val = reg_value(1, val, MUX_SHIFT16_RA800, MUX_MASK16_RA800);
            val = reg_value(PS_BRIDGE_MODE_MASTER, val, MUX_SHIFT07_RA800, MUX_MASK07_RA800);
            val = reg_value(ps, val, MUX_SHIFT00_RA800, MUX_MASK00_RA800);
            disp_write(base, MUX_RAB00, val);
            break;
        default:
            DISPERR("lvds4 single mode input dc_idx (%d) error!\n", dc_idx);
            return -1;
    }

    /*LVDS MUX_SEL*/
    val = disp_read(base_lvds, LVDS_MUX_SEL);
    val = reg_value(dc_pll_sel[dc_idx], val, MUX8_SEL_SHIFT, MUX8_SEL_MASK);
    disp_write(base_lvds, LVDS_MUX_SEL, val);

    return 0;
}

static int if_link_lvds(struct disp_if_link *if_link)
{
    int ret;

    if (if_link->link_lvds_combine >= 0) {
        ret = if_link_lvds_comb(if_link->link_lvds_combine);
        if (ret) {
            DISPERR("link lvds comb error!\n");
            return ret;
        }

    }else {
        /*TODO: Dual mode config in lvds ss.*/
        if (if_link->link_lvds1_lvds2 >= 0) {
            ret = if_link_lvds12_dual(if_link->link_lvds1_lvds2);
            if (ret) {
                DISPERR("link lvds12 error!\n");
                return ret;
            }
        } else {
            if (if_link->link_lvds1 >= 0) {
                ret = if_link_lvds1(if_link->link_lvds1);
                if (ret) {
                    DISPERR("link lvds1 error!\n");
                    return ret;
                }
            }

            if (if_link->link_lvds2 >= 0) {
                ret = if_link_lvds2(if_link->link_lvds2);
                if (ret) {
                    DISPERR("link lvds2 error!\n");
                    return ret;
                }
            }
        }

        if (if_link->link_lvds3_lvds4 >= 0) {
            ret = if_link_lvds34_dual(if_link->link_lvds3_lvds4);
            if (ret) {
                DISPERR("link lvds34 error!\n");
                return ret;
            }
        } else {
            if (if_link->link_lvds3 >= 0) {
                ret = if_link_lvds3(if_link->link_lvds3);
                if (ret) {
                    DISPERR("link lvds3 error!\n");
                    return ret;
                }
            }

            if (if_link->link_lvds4 >= 0) {
                ret = if_link_lvds4(if_link->link_lvds4);
                if (ret) {
                    DISPERR("link lvds4 error!\n");
                    return ret;
                }
            }
        }
    }

    return 0;
}
static int if_link_init(struct disp_if_link *if_link)
{
    int ret;
    /*LINK DSI1*/
    ret = if_link_dsi1(if_link->link_dsi1);
    if (ret) {
        DISPERR("link dsi1 error!\n");
        return ret;
    }

    /*LINK_DSI2*/
    ret = if_link_dsi2(if_link->link_dsi2);
    if (ret) {
        DISPERR("link dsi2 error!\n");
        return ret;
    }

    /*LINK PARALLEL*/
    ret = if_link_parallel(if_link->link_parallel);
    if (ret) {
        DISPERR("link parallel error!\n");
        return ret;
    }

    /*LINK LVDS*/
    ret = if_link_lvds(if_link);
    if (ret) {
        DISPERR("link lvds error!\n");
        return ret;
    }

    return 0;
}

static void disp_plls_init(void)
{
    addr_t base = get_disp_mux_reg_base();
    int dc_idx, pll_idx;
    int val;

    for (int i = 0; i < res_count; i++) {
        dc_idx = (g_display_res + i)->dc_res;
        pll_idx = (g_display_res + i)->pll_idx;
        switch (dc_idx) {
            case DC_RES_DC1:
                val = disp_read(base, MUX_RA000);
                val = reg_value(pll_idx, val, MUX_SHIFT00_RA000, MUX_MASK00_RA000);
                disp_write(base, MUX_RA000, val);
                dc_pll_sel[DC1] = pll_idx;
                break;
            case DC_RES_DC2:
                val = disp_read(base, MUX_RA100);
                val = reg_value(pll_idx, val, MUX_SHIFT00_RA000, MUX_MASK00_RA000);
                disp_write(base, MUX_RA100, val);
                dc_pll_sel[DC2] = pll_idx;
            break;
            case DC_RES_DC3:
                val = disp_read(base, MUX_RA200);
                val = reg_value(pll_idx, val, MUX_SHIFT00_RA000, MUX_MASK00_RA000);
                disp_write(base, MUX_RA200, val);
                dc_pll_sel[DC3] = pll_idx;
                break;
            case DC_RES_DC4:
                val = disp_read(base, MUX_RA300);
                val = reg_value(pll_idx, val, MUX_SHIFT00_RA000, MUX_MASK00_RA000);
                disp_write(base, MUX_RA300, val);
                dc_pll_sel[DC4] = pll_idx;
                break;
            case DC_RES_DC5:
                val = disp_read(base, MUX_RA400);
                val = reg_value(pll_idx, val, MUX_SHIFT00_RA000, MUX_MASK00_RA000);
                disp_write(base, MUX_RA400, val);
                dc_pll_sel[DC5] = pll_idx;
                break;
            case DC_RES_DC1_DC2:
                val = disp_read(base, MUX_RA000);
                val = reg_value(pll_idx, val, MUX_SHIFT00_RA000, MUX_MASK00_RA000);
                disp_write(base, MUX_RA000, val);
                dc_pll_sel[DC1] = pll_idx;

                val = disp_read(base, MUX_RA100);
                val = reg_value(pll_idx, val, MUX_SHIFT00_RA000, MUX_MASK00_RA000);
                disp_write(base, MUX_RA100, val);
                dc_pll_sel[DC2] = pll_idx;
                break;
            case DC_RES_DC3_DC4:
                val = disp_read(base, MUX_RA200);
                val = reg_value(pll_idx, val, MUX_SHIFT00_RA000, MUX_MASK00_RA000);
                disp_write(base, MUX_RA200, val);
                dc_pll_sel[DC3] = pll_idx;

                val = disp_read(base, MUX_RA300);
                val = reg_value(pll_idx, val, MUX_SHIFT00_RA000, MUX_MASK00_RA000);
                disp_write(base, MUX_RA300, val);
                dc_pll_sel[DC4] = pll_idx;
                break;
        }
    }
}

static void disp_lvds_wrap_init(void)
{
    addr_t base = get_lvds_reg_base();
    int wrap_id, clk_sel, clk_en;
    int val;

    for (int i = 0; i < res_count; i++) {
        wrap_id = (g_display_res + i)->pll_idx;

        if (wrap_id >= DISP_PLL1)
            continue;

        val = disp_read(base, LVDS_CHN_CLOCK_(wrap_id));
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
        disp_write(base, LVDS_CHN_CLOCK_(wrap_id), val);
    }
}

static char to_dsi_pixel_map(char pixel_bpp)
{
    switch (pixel_bpp) {
        case 16:
            return DSI_PIXEL_MAP_RGB565;
        case 18:
            return DSI_PIXEL_MAP_RGB666;
        case 24:
            return DSI_PIXEL_MAP_RGB888;
        case 30:
            return DSI_PIXEL_MAP_RGB101010;
        default:
            return DSI_PIXEL_MAP_RGB888;
    }
}

static void mipi_dsi_output_yuv422(int if_res, int hsize)
{
    addr_t base = get_disp_mux_reg_base();
    unsigned int reg_dsi_mux, reg_dsi_size;
    int val;

    switch (if_res) {
        case IF_RES_MIPI_DSI1:
            reg_dsi_mux = MUX_RB000;
            reg_dsi_size = MUX_RB00C;
            break;
        case IF_RES_MIPI_DSI2:
            reg_dsi_mux = MUX_RB100;
            reg_dsi_size = MUX_RB10C;
            break;
        default:
            DISPERR("dsi get index input is not correct\n");
            return;
    }

    val = disp_read(base, reg_dsi_mux);
    val = reg_value(0, val, MUX_SHIFT28_RB000, MUX_MASK28_RB000);
    disp_write(base, reg_dsi_mux, val);

    disp_write(base, reg_dsi_size, hsize);
}

int disp_link_init(uint8_t *dc_pll, struct display_resource *disp_res, unsigned int res_num)
{
    int ret = 0;
    struct disp_link *link;
    struct disp_dp_link *dp_link;
    struct disp_if_link *if_link;
    unsigned int i = 0;

    g_display_res = disp_res;
    res_count     = res_num;
    dc_pll_sel    = dc_pll;

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
    if_link->link_dsi1_dsi2 = -1;
    if_link->link_parallel = -1;
    if_link->link_lvds_combine = -1;
    if_link->link_lvds1 = -1;
    if_link->link_lvds2 = -1;
    if_link->link_lvds3 = -1;
    if_link->link_lvds4 = -1;
    if_link->link_lvds1_lvds2 = -1;
    if_link->link_lvds3_lvds4 = -1;

    for(i = 0; i < res_count; i++) {
        conv_to_dplink(g_display_res + i, dp_link);
        conv_to_iflink(g_display_res + i, if_link);
    }

    disp_plls_init();
    disp_lvds_wrap_init();

    ret = dp_link_init(dp_link);
    if (ret) {
        DISPERR("dp link init failed.\n");
        goto FREE;
    }

    ret = if_link_init(if_link);
    if (ret) {
        DISPERR("if link init failed.\n");
        goto FREE;
    }

    DISPMSG("disp link init done\n");

FREE:
    free(link);

    return ret;
}

int disp_get_clks(struct disp_pll_clks* pclks)
{
    return 0;
}

void disp_dsi_to_csi_config(int if_res, int line_num,
    int hsize, int pixel_bpp, int colorcoding)
{
    addr_t base = get_disp_mux_reg_base();
    unsigned int reg_ctrl1, reg_ctrl2;
    int csi_datatype, dsi_datatype;
    int yuv444toyuv422 = 0;
    int bpp = 24;
    int val;

    switch (if_res) {
        case IF_RES_MIPI_DSI1:
            reg_ctrl1 = MUX_RB004;
            reg_ctrl2 = MUX_RB008;
            break;
        case IF_RES_MIPI_DSI2:
            reg_ctrl1 = MUX_RB104;
            reg_ctrl2 = MUX_RB108;
            break;
        default:
            DISPERR("dsi get index input is not correct\n");
            return;
    }

    switch (colorcoding) {
        case COLORCODE_16BIT_CONFIG1:
        case COLORCODE_16BIT_CONFIG2:
        case COLORCODE_16BIT_CONFIG3:
            csi_datatype = 0x1e;
            dsi_datatype = 0x0e;
            bpp = 16;
            break;
        case COLORCODE_16BIT_YCC422:
            csi_datatype = 0x1e;
            dsi_datatype = 0x2c;
            yuv444toyuv422 = 1;
            bpp = 16;
            break;
        case COLORCODE_18BIT_CONFIG1:
        case COLORCODE_18BIT_CONFIG2:
            csi_datatype = 0x23;
            dsi_datatype = 0x1e;
            bpp = 18;
            break;
        case COLORCODE_24BIT:
        default:
            csi_datatype = 0x24;
            dsi_datatype = 0x3e;
            break;
    }

    if (yuv444toyuv422)
        mipi_dsi_output_yuv422(if_res, hsize);

    val = disp_read(base, reg_ctrl2);
    val = reg_value(1, val, MUX_SHIFT12_RB008, MUX_MASK12_RB008);
    val = reg_value(line_num, val, MUX_SHIFT16_RB008, MUX_MASK16_RB008);
    disp_write(base, reg_ctrl2, val);

    val = disp_read(base, reg_ctrl1);
    val = reg_value(1, val, MUX_SHIFT07_RB004, MUX_MASK07_RB004);
    val = reg_value(csi_datatype, val, MUX_SHIFT14_RB004, MUX_MASK14_RB004);
    val = reg_value(dsi_datatype, val, MUX_SHIFT08_RB004, MUX_MASK08_RB004);
    val = reg_value(1, val, MUX_SHIFT06_RB004, MUX_MASK06_RB004);
    val = reg_value(to_dsi_pixel_map(bpp), val, MUX_SHIFT00_RB004, MUX_MASK00_RB004);
    disp_write(base, reg_ctrl1, val);
}
