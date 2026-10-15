/*
* disp_link.h
*
* Copyright (c) 2019 Semidrive Semiconductor.
* All rights reserved.
*
* Description:
*
* Revision History:
* -----------------
* 011, 10/31/2019 BI create this file
*/
#ifndef __DISP_LINK__
#define __DISP_LINK__
#include <disp_link_types.h>
#include <disp_ssystem_link.h>

/*---------Possible Options Part----------*/
/**
 * Each enumeration lists the possible options for the module.
 */
enum {
    SRC_SEL_7_CLOCK   = 0b0,
    SRC_SEL_3P5_CLOCK = 0b1
};

enum {
    LVDS_CLK_EN      = 0b00001,
    LVDS_CLK_X7_EN   = 0b00010,
    LVDS_CLK_X3P5_EN = 0b00100,
    LVDS_DBG_CLK_EN  = 0b01000,
    LVDS_DSP_CLK_EN  = 0b10000
};

enum {
    LVDS_SAMPLE_SYNC_GROP0 = 0b001,
    LVDS_SAMPLE_SYNC_GROP1 = 0b010,
    LVDS_SAMPLE_SYNC_4K    = 0b100
};

enum {
    DSI_CLKSEL_CLK_STOPPED = 0,
    DSI_CLKSEL_CLK_GENERATION,
    DSI_CLKSEL_BUFFERD_CLKEXT,
    DSI_CLKSEL_FORBIDDEN
};


/*-----------LINK part--------------*/
enum IF_LINK {
    LINK_DC1 = DC_RES_DC1,
    LINK_DC2,
    LINK_DC3,
    LINK_DC4,
    LINK_DC5,
    LINK_CMB_OUT_CH1,
    LINK_CMB_OUT_CH2
};

/*For LVDS Combine LINK.*/
enum LVDS_CMB_LINK {
    CMB_LINK_NO_LINK = -1,
    CMB_LINK_DC1_DC2,
    CMB_LINK_DC3_DC4
};

/**
 *This structure represents the link relationship between DP and DC.
 */
struct disp_dp_link {
    char dp1_link;/*can select: LINK_DC1 or LINK_DC2*/
    char dp2_link;/*can select: LINK_DC1 or LINK_DC2*/
    char dp3_link;/*can select: LINK_DC3 or LINK_DC4*/
};

/**
 *This structure represents the link relationship between DC and interface.
 */
struct disp_if_link {
    char link_dsi1;/*can select LINK_DC1~5,other meaing not link*/
    char link_dsi2;/*can select LINK_DC1~5,other meaing not link*/
    char link_dsi1_dsi2;
    char link_parallel;/*can select LINK_DC1~5,other meaing not link*/
    char link_lvds_combine;
    char link_lvds_group1_ch1;
    char link_lvds_group1_ch2;
    char link_lvds_group2_ch1;
    char link_lvds_group2_ch2;
    char link_lvds1_lvds2;
    char link_lvds3_lvds4;
};

struct disp_link {
    struct disp_dp_link dp_link;
    struct disp_if_link if_link;
};


/*-----------Register Set Part-------------*/
enum {
    SEL_DC1 = 0b000,
    SEL_DC2 = 0b001,
    SEL_DC3 = 0b010,
    SEL_DC4 = 0b011,
    SEL_DC5 = 0b100,
    SEL_CMB_OUT_CH1 = 0b101,
    SEL_CMB_OUT_CH2 = 0b110,
    SEL_NOT_SUPPORT = 0b111
};

enum {
    CMB_SEL_DC1_DC2 = 0b0,
    CMB_SEL_DC3_DC4 = 0b1
};

enum {
    SEL_LVDS_PLL1_X7 = 0b00,
    SEL_LVDS_PLL2_X7 = 0b01,
    SEL_LVDS_PLL3_X7 = 0b10,
    SEL_LVDS_PLL4_X7 = 0b11
};

enum {
    DSI_PIXEL_MAP_RGB101010 = 0b00,
    DSI_PIXEL_MAP_RGB565    = 0b01,
    DSI_PIXEL_MAP_RGB666    = 0b10,
    DSI_PIXEL_MAP_RGB888    = 0b11,
};

/*------DC Match PLL Information----*/
enum {
    DC1 = 0,
    DC2,
    DC3,
    DC4,
    DC5,
    DC_NUM
};

#endif //__DISP_LINK__
