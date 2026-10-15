/*
 * sdrv_g2dlite_core.c
 *
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 * 011, 04/28/2020 BI create this file
 */
//#include <res.h>
#include <arch/cache.h>
#include <driver/kunlun/g2dlite/g2dlite.h>
#include <driver/kunlun/g2dlite/g2dlite_core.h>
#include <driver/kunlun/g2dlite/g2dlite_drv_log.h>
#include <g2dlite_reg.h>
#include <stdlib.h>
#include <g2dlite_errcode.h>

#ifndef udelay
#define udelay(x)                                                              \
    {                                                                          \
        volatile uint64_t count = 20ull * (uint64_t)x;                         \
        while (count != 0ull)                                                  \
            count--;                                                           \
    }
#endif

#define G2DLITE_INT IRQ_GIC1_G2D2_G2D_IRQ_NUM
#define G2DLITE_BASE APB_G2D2_BASE

#define G2DLITE_CLUT_SADDR 0x5400
#define G2DLITE_CLUT_EADDR 0x57FF

#define G2DLITE_PD_IDX 3
/*
g2dlite_profile_t g2dlite_prop = {
    .base_addr = G2DLITE_BASE,
    .irq       = G2DLITE_INT,
    .layer_n   = 2,
};*/

static const u8 r_wml_cfgs[8][5] = {
    /*fastcopy*/
    {0x00, 0x00, 0x00, 0x00, 0x40},
    /*GPipe Only(1palne)*/
    {0x30, 0x00, 0x00, 0x00, 0x10},
    /*GPipe Only(2palnes)*/
    {0x18, 0x18, 0x00, 0x00, 0x10},
    /*GPipe Only(3palnes)*/
    {0x18, 0x10, 0x10, 0x00, 0x08},
    /*SPipe Only(1palne)*/
    {0x00, 0x00, 0x00, 0x30, 0x10},
    /*GPipe(1palne) + Spipe*/
    {0x18, 0x00, 0x00, 0x18, 0x10},
    /*GPipe(2palnes) + Spipe*/
    {0x10, 0x10, 0x00, 0x10, 0x10},
    /*GPipe(3palnes) + Spipe*/
    {0x10, 0x08, 0x08, 0x10, 0x10}
};

static const u8 r_d_depth_cfgs[8][5] = {
    /*fastcopy*/
    {0x00, 0x00, 0x00, 0x00, 0x08},
    /*GPipe Only(1palne)*/
    {0x06, 0x00, 0x00, 0x00, 0x02},
    /*GPipe Only(2palnes)*/
    {0x03, 0x03, 0x00, 0x00, 0x02},
    /*GPipe Only(3palnes)*/
    {0x03, 0x02, 0x02, 0x00, 0x01},
    /*SPipe Only(1palne)*/
    {0x00, 0x00, 0x00, 0x06, 0x02},
    /*GPipe(1palne) + Spipe*/
    {0x03, 0x00, 0x00, 0x03, 0x02},
    /*GPipe(2palnes) + Spipe*/
    {0x02, 0x02, 0x00, 0x03, 0x01},
    /*GPipe(3palnes) + Spipe*/
    {0x02, 0x01, 0x01, 0x03, 0x01}
};

static const u8 r_c_depth_cfgs[8][5] = {
    /*fastcopy*/
    {0x00, 0x00, 0x00, 0x00, 0x08},
    /*GPipe Only(1palne)*/
    {0x06, 0x00, 0x00, 0x00, 0x02},
    /*GPipe Only(2palnes)*/
    {0x03, 0x03, 0x00, 0x00, 0x02},
    /*GPipe Only(3palnes)*/
    {0x03, 0x02, 0x02, 0x00, 0x01},
    /*SPipe Only(1palne)*/
    {0x00, 0x00, 0x00, 0x06, 0x02},
    /*GPipe(1palne) + Spipe*/
    {0x03, 0x00, 0x00, 0x03, 0x02},
    /*GPipe(2palnes) + Spipe*/
    {0x02, 0x02, 0x00, 0x03, 0x01},
    /*GPipe(3palnes) + Spipe*/
    {0x02, 0x01, 0x01, 0x03, 0x01}
};

static const u8 w_wml_cfgs[3][3] = {
    /*fastcopy && Wipe(1plane)*/
    {0x40, 0x00, 0x00},
    /*Wipe(2planes)*/
    {0x20, 0x20, 0x00},
    /*Wipe(3planes)*/
    {0x20, 0x10, 0x10}
};

static const u8 w_d_depth_cfgs[3][3] = {
    /*fastcopy && Wipe(1plane)*/
    {0x08, 0x00, 0x00},
    /*Wipe(2planes)*/
    {0x04, 0x04, 0x00},
    /*Wipe(3planes)*/
    {0x04, 0x02, 0x02}
};

static const u8 w_c_depth_cfgs[3][3] = {
    /*fastcopy && Wipe(1plane)*/
    {0x08, 0x00, 0x00},
    /*Wipe(2planes)*/
    {0x04, 0x04, 0x00},
    /*Wipe(3planes)*/
    {0x04, 0x02, 0x02}
};

static const uint32_t csc_param[5][15] = {
    {/*bt709 YCbCr to RGB(16-235)*/
     0x0800, 0x0000, 0x0C99, 0x0800, 0x3E81, 0x3C42, 0x0800, 0x0ED8, 0x0000,
     0x0000, 0x0000, 0x0000, 0x0000, 0x0200, 0x0200},
    {/*YCbCr to RGB*/
     0x094F, 0x0000, 0x0CC4, 0x094F, 0x3CE0, 0x397F, 0x094F, 0x1024, 0x0000,
     0x0000, 0x0000, 0x0000, 0x0040, 0x0200, 0x0200},
    {/*YUV to RGB*/
     0x0800, 0x0000, 0x091E, 0x0800, 0x3CD8, 0x3B5B, 0x0800, 0x1041, 0x0000,
     0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000},
    {/*RGB to YCbCr*/
     0x0264, 0x04B2, 0x00E9, 0x3EA0, 0x3D4A, 0x0416, 0x0416, 0x3C94, 0x3F57,
     0x0000, 0x0200, 0x0200, 0x0000, 0x0000, 0x0000},
    {/*RGB to YUV*/
     0x0000}};

static uint32_t G2DLITE_HSCALER[3][2] = {
    {0x2600, 0x29ff}, //{HS_0_SADDR, HS_0_EADDR}
    {0x3600, 0x39ff}, //{HS_1_SADDR, HS_1_EADDR}
    {0x4600, 0x49ff}  //{HS_2_SADDR, HS_2_EADDR}
};

static uint32_t G2DLITE_VSCALER[3][2] = {
    {0x2a00, 0x2dff}, //{VS_0_SADDR, VS_0_EADDR}
    {0x3a00, 0x3dff}, //{VS_1_SADDR, VS_1_EADDR}
    {0x4a00, 0x4dff}  //{VS_2_SADDR, VS_2_EADDR}
};

static unsigned int get_l_addr(unsigned long addr)
{
    unsigned int addr_l;

    addr_l = (unsigned int)(addr & 0x00000000FFFFFFFF);
    return addr_l;
}

static unsigned int get_h_addr(unsigned long addr)
{
    unsigned int addr_h;

    addr_h = (unsigned int)(addr & 0xFFFFFFFF00000000);
    return addr_h;
}

static void get_bpp_by_format(unsigned int format, unsigned char *bpp)
{
    switch (format) {
    case COLOR_RGB565:
    case COLOR_YUYV:
        bpp[0] = 16;
        break;
    case COLOR_RGB888:
        bpp[0] = 24;
        break;
    case COLOR_ARGB8888:
        bpp[0] = 32;
        break;
    default:
        break;
    }
}

static void g2dlite_com_regs_init(struct g2dlite_device *dev)
{
    unsigned long base = dev->reg_base;

    /*G2D_RA020*/
    /*g2dlite mask:1-disable irq, 0-enable irq*/
    /*bit[6]:  G2DLITE_FRM_DONE */
    /*bit[5]:  G2DLITE_TASK_DONE */
    /*bit[4]:  WDMA */
    /*bit[3]:  RLE_1 */
    /*bit[2]:  MLC */
    /*bit[1]:  RLE_0 */
    /*bit[0]:  RDMA */
    g2dlite_write(base, G2D_RA020, 0x3F); /*set G2DLITE_FRM_DONE enable*/
}

static void g2dlite_rdma_init(struct g2dlite_device *dev)
{
    const unsigned short wml_cfg[] = {0x08, 0x08, 0x08, 0x08, 0x08};
    const unsigned short d_depth_cfg[] = {0x01, 0x01, 0x01, 0x01, 0x01};
    const unsigned short c_depth_cfg[] = {0x01, 0x01, 0x01, 0x01, 0x01};
    const unsigned char sche_cfg[] = {0x04, 0x04, 0x04, 0x04, 0x04};
    const unsigned char p1_cfg[] = {0x30, 0x30, 0x30, 0x30, 0x30};
    const unsigned char p0_cfg[] = {0x10, 0x10, 0x10, 0x10, 0x10};
    const unsigned char burst_mode_cfg[] = {0x00, 0x00, 0x00, 0x00, 0x00};
    const unsigned char burst_len_cfg[] = {0x04, 0x04, 0x04, 0x04, 0x04};

    unsigned int val;
    unsigned long base = dev->reg_base;

    G2DLITE_LOG_INFO("enter\n");
    for (int i = 0; i < CHN_COUNT; i++)
        g2dlite_write(base, G2D_RB000(i), wml_cfg[i]);

    for (int i = 0; i < CHN_COUNT; i++)
        g2dlite_write(base, G2D_RB004(i), d_depth_cfg[i]);

    for (int i = 0; i < CHN_COUNT; i++)
        g2dlite_write(base, G2D_RB008(i), c_depth_cfg[i]);

    for (int i = 0; i < CHN_COUNT; i++) {
        val = reg_value(sche_cfg[i], 0, G2D_SHIFT16_RB00C,
                        G2D_MASK16_RB00C);
        val = reg_value(p1_cfg[i], val, G2D_SHIFT08_RB00C,
                        G2D_MASK08_RB00C);
        val = reg_value(p0_cfg[i], val, G2D_SHIFT00_RB00C,
                        G2D_MASK00_RB00C);
        g2dlite_write(base, G2D_RB00C(i), val);
    }

    for (int i = 0; i < CHN_COUNT; i++) {
        val = reg_value(burst_mode_cfg[i], 0, G2D_SHIFT03_RB010,
                        G2D_MASK03_RB010);
        val = reg_value(burst_len_cfg[i], val, G2D_SHIFT00_RB010,
                        G2D_MASK00_RB010);
        g2dlite_write(base, G2D_RB010(i), val);
    }

    g2dlite_write(base, G2D_RB520, 0);

    val = reg_value(1, 0, G2D_SHIFT01_RB400, G2D_MASK01_RB400);
    val = reg_value(1, val, G2D_SHIFT00_RB400, G2D_MASK00_RB400);
    g2dlite_write(base, G2D_RB400, val);

    G2DLITE_LOG_INFO("leave\n");
}

static void g2dlite_gpipe_init(struct g2dlite_device *dev)
{
    const unsigned char hs_nor_para = 0x09;
    const unsigned char hs_ratio_int = 0x01;
    const unsigned char hs_ratio_fra = 0x00;
    const unsigned char vs_nor_para = 0x06;
    unsigned int val;
    unsigned long base = dev->reg_base;

    G2DLITE_LOG_INFO("enter\n");

    /*YUVUP Bypass*/
    val = reg_value(1, 0, G2D_SHIFT00_RC044,
                    G2D_MASK00_RC044);
    g2dlite_write(base, G2D_RC044, val);

    /*CSC Bypass*/
    val = reg_value(1, 0, G2D_SHIFT00_RC200, G2D_MASK00_RC200);
    g2dlite_write(base, G2D_RC200, val);

    val = reg_value(csc_param[0][1], 0, G2D_SHIFT16_RC204,
                    G2D_MASK16_RC204);
    val = reg_value(csc_param[0][0], val, G2D_SHIFT00_RC204,
                    G2D_MASK00_RC204);
    g2dlite_write(base, G2D_RC204, val);

    val = reg_value(csc_param[0][3], 0, G2D_SHIFT16_RC208,
                    G2D_MASK16_RC208);
    val = reg_value(csc_param[0][2], val, G2D_SHIFT00_RC208,
                    G2D_MASK00_RC208);
    g2dlite_write(base, G2D_RC208, val);

    val = reg_value(csc_param[0][5], 0, G2D_SHIFT16_RC20C,
                    G2D_MASK16_RC20C);
    val = reg_value(csc_param[0][4], val, G2D_SHIFT00_RC20C,
                    G2D_MASK00_RC20C);
    g2dlite_write(base, G2D_RC20C, val);

    val = reg_value(csc_param[0][7], 0, G2D_SHIFT16_RC210,
                    G2D_MASK16_RC210);
    val = reg_value(csc_param[0][6], val, G2D_SHIFT00_RC210,
                    G2D_MASK00_RC210);
    g2dlite_write(base, G2D_RC210, val);

    val = reg_value(csc_param[0][9], 0, G2D_SHIFT16_RC214,
                    G2D_MASK16_RC214);
    val = reg_value(csc_param[0][8], val, G2D_SHIFT00_RC214,
                    G2D_MASK00_RC214);
    g2dlite_write(base, G2D_RC214, val);

    val = reg_value(csc_param[0][11], 0, G2D_SHIFT16_RC218,
                    G2D_MASK16_RC218);
    val = reg_value(csc_param[0][10], val, G2D_SHIFT00_RC218,
                    G2D_MASK00_RC218);
    g2dlite_write(base, G2D_RC218, val);

    val = reg_value(csc_param[0][13], 0, G2D_SHIFT16_RC21C,
                    G2D_MASK16_RC21C);
    val = reg_value(csc_param[0][12], val, G2D_SHIFT00_RC21C,
                    G2D_MASK00_RC21C);
    g2dlite_write(base, G2D_RC21C, val);

    val = reg_value(csc_param[0][14], 0, G2D_SHIFT00_RC220,
                    G2D_MASK00_RC220);
    g2dlite_write(base, G2D_RC220, val);

    /*HSCALER*/
    val = reg_value(hs_nor_para, 0, G2D_SHIFT08_RC300,
                    G2D_MASK08_RC300);
    g2dlite_write(base, G2D_RC300, val);

    g2dlite_write(base, G2D_RC304, 0);

    val =
        reg_value(hs_ratio_int, 0, G2D_SHIFT19_RC308, G2D_MASK19_RC308);
    val = reg_value(hs_ratio_fra, val, G2D_SHIFT00_RC308,
                    G2D_MASK00_RC308);
    g2dlite_write(base, G2D_RC308, val);

    /*VSCALER*/
    val = reg_value(vs_nor_para, val, G2D_SHIFT04_RC400,
                    G2D_MASK04_RC400);
    g2dlite_write(base, G2D_RC400, val);

    g2dlite_write(base, G2D_RC408, 0);

    g2dlite_write(base, G2D_RC40C, 0);

    g2dlite_write(base, G2D_RC410, 0);

    G2DLITE_LOG_INFO("leave\n");

    /*GP Trigger*/
    // g2dlite_write(base, GP_SDW_CTRL, 1);
}

static void g2dlite_spipe_init(struct g2dlite_device *dev)
{
    unsigned int val;
    unsigned long base = dev->reg_base;

    val = g2dlite_read(base, G2D_RD200);
    val = reg_value(1, val, G2D_SHIFT18_RD200, G2D_MASK18_RD200);
    val = reg_value(1, val, G2D_SHIFT16_RD200, G2D_MASK16_RD200);
    g2dlite_write(base, G2D_RD200, val);

    val = g2dlite_read(base, G2D_RD204);
    val = reg_value(1, val, G2D_SHIFT16_RD204, G2D_MASK16_RD204);
    g2dlite_write(base, G2D_RD204, val);

    val = g2dlite_read(base, G2D_RD208);
    val = reg_value(1, val, G2D_SHIFT16_RD208, G2D_MASK16_RD208);
    g2dlite_write(base, G2D_RD208, val);

    val = g2dlite_read(base, G2D_RD20C);
    val = reg_value(1, val, G2D_SHIFT16_RD20C, G2D_MASK16_RD20C);
    g2dlite_write(base, G2D_RD20C, val);
}

static void g2dlite_mlc_init(struct g2dlite_device *dev)
{
    unsigned int val;
    unsigned long base = dev->reg_base;
    const unsigned char prot_val[] = {0x7, 0x7};
    const unsigned char alpha_bld[] = {0xF, 0xF, 0xF};
    const unsigned char pd_out[] = {0x7, 0x7, 0x7};
    const unsigned char pd_des[] = {0x7, 0x7, 0x7};
    const unsigned char pd_src[] = {0x7, 0x7, 0x7};
    const unsigned char layer_out[] = {0xF, 0xF, 0xF};

    /*LAYER related*/
    for (int i = 0; i < MLC_LAYER_COUNT; i++) {
        val = reg_value(prot_val[i], 0, G2D_SHIFT08_RE000,
                        G2D_MASK08_RE000);
        val =
            reg_value(1, val, G2D_SHIFT02_RE000, G2D_SHIFT02_RE000);
        g2dlite_write(base, G2D_RE000(i), val);

        g2dlite_write(base, G2D_RE018(i), 0xFF);
    }

    /*PATH related*/
    for (int i = 0; i < MLC_PATH_COUNT; i++) {
        val =
            reg_value(alpha_bld[i], 0, G2D_SHIFT16_RE200, G2D_MASK16_RE200);
        val = reg_value(pd_out[i], val, G2D_SHIFT12_RE200, G2D_MASK12_RE200);
        val = reg_value(pd_des[i], val, G2D_SHIFT08_RE200, G2D_MASK08_RE200);
        val = reg_value(pd_src[i], val, G2D_SHIFT04_RE200, G2D_MASK04_RE200);
        val = reg_value(layer_out[i], val, G2D_SHIFT00_RE200,
                        G2D_MASK00_RE200);
        g2dlite_write(base, G2D_RE200(i), val);
    }

    g2dlite_write(base, G2D_RE234, 0xFFFF);

    g2dlite_write(base, G2D_RE240, 0);
}

static void g2dlite_wdma_init(struct g2dlite_device *dev)
{
    const unsigned short wml_cfg[] = {0x20, 0x10, 0x10};
    const unsigned short d_depth_cfg[] = {0x04, 0x02, 0x02};
    const unsigned short c_depth_cfg[] = {0x04, 0x02, 0x02};
    const unsigned char sche_cfg[] = {0x04, 0x02, 0x02};
    const unsigned char p1_cfg[] = {0x30, 0x30, 0x30};
    const unsigned char p0_cfg[] = {0x10, 0x10, 0x10};
    const unsigned char burst_mode_cfg[] = {0x00, 0x00, 0x00};
    const unsigned char burst_len_cfg[] = {0x04, 0x04, 0x04};
    unsigned int val;
    unsigned long base = dev->reg_base;

    for (int i = 0; i < WCHN_COUNT; i++)
        g2dlite_write(base, G2D_RG000(i), wml_cfg[i]);

    for (int i = 0; i < WCHN_COUNT; i++)
        g2dlite_write(base, G2D_RG004(i), d_depth_cfg[i]);

    for (int i = 0; i < WCHN_COUNT; i++)
        g2dlite_write(base, G2D_RG008(i), c_depth_cfg[i]);

    for (int i = 0; i < WCHN_COUNT; i++) {
        val = reg_value(sche_cfg[i], 0, G2D_SHIFT16_RG00C,
                        G2D_MASK16_RG00C);
        val = reg_value(p1_cfg[i], val, G2D_SHIFT08_RG00C,
                        G2D_MASK08_RG00C);
        val = reg_value(p0_cfg[i], val, G2D_SHIFT00_RG00C,
                        G2D_MASK00_RG00C);
        g2dlite_write(base, G2D_RG00C(i), val);
    }

    for (int i = 0; i < WCHN_COUNT; i++) {
        val = reg_value(burst_mode_cfg[i], 0, G2D_SHIFT03_RG010,
                        G2D_MASK03_RG010);
        val = reg_value(burst_len_cfg[i], val, G2D_SHIFT00_RG010,
                        G2D_MASK00_RG010);
        g2dlite_write(base, G2D_RG010(i), val);
    }

    g2dlite_write(base, G2D_RG520, 0);
}

static void g2dlite_wpipe_init(struct g2dlite_device *dev)
{
    unsigned int val;
    unsigned long base = dev->reg_base;

    /*YUVDOWN Bypass*/
    val = reg_value(1, 0, G2D_SHIFT08_RH008,
                    G2D_MASK08_RH008);
    g2dlite_write(base, G2D_RH008, val);

    /*CSC Bypass*/
    val = reg_value(1, 0, G2D_SHIFT00_RH040, G2D_MASK00_RH040);
    g2dlite_write(base, G2D_RH040, val);

    val = reg_value(csc_param[3][1], 0, G2D_SHIFT16_RH044,
                    G2D_MASK16_RH044);
    val = reg_value(csc_param[3][0], val, G2D_SHIFT00_RH044,
                    G2D_MASK00_RH044);
    g2dlite_write(base, G2D_RH044, val);

    val = reg_value(csc_param[3][3], 0, G2D_SHIFT16_RH048,
                    G2D_MASK16_RH048);
    val = reg_value(csc_param[3][2], val, G2D_SHIFT00_RH048,
                    G2D_MASK00_RH048);
    g2dlite_write(base, G2D_RH048, val);

    val = reg_value(csc_param[3][5], 0, G2D_SHIFT16_RH04C,
                    G2D_MASK16_RH04C);
    val = reg_value(csc_param[3][4], val, G2D_SHIFT00_RH04C,
                    G2D_MASK00_RH04C);
    g2dlite_write(base, G2D_RH04C, val);

    val = reg_value(csc_param[3][7], 0, G2D_SHIFT16_RH050,
                    G2D_MASK16_RH050);
    val = reg_value(csc_param[3][6], val, G2D_SHIFT00_RH050,
                    G2D_MASK00_RH050);
    g2dlite_write(base, G2D_RH050, val);

    val = reg_value(csc_param[3][9], 0, G2D_SHIFT16_RH054,
                    G2D_MASK16_RH054);
    val = reg_value(csc_param[3][8], val, G2D_SHIFT00_RH054,
                    G2D_MASK00_RH054);
    g2dlite_write(base, G2D_RH054, val);

    val = reg_value(csc_param[3][11], 0, G2D_SHIFT16_RH058,
                    G2D_MASK16_RH058);
    val = reg_value(csc_param[3][10], val, G2D_SHIFT00_RH058,
                    G2D_MASK00_RH058);
    g2dlite_write(base, G2D_RH058, val);

    val = reg_value(csc_param[3][13], 0, G2D_SHIFT16_RH05C,
                    G2D_MASK16_RH05C);
    val = reg_value(csc_param[3][12], val, G2D_SHIFT00_RH05C,
                    G2D_MASK00_RH05C);
    g2dlite_write(base, G2D_RH05C, val);

    val = reg_value(csc_param[3][14], 0, G2D_SHIFT00_RH060,
                    G2D_MASK00_RH060);
    g2dlite_write(base, G2D_RH060, val);
}

static u8 get_r_idx(struct g2dlite_input *ins)
{
    int g_nplanes = 0;

    switch (ins->layer_num) {
    case 1:
        if (ins->layer[0].layer == G2DLITE_TYPE_GPIPE) {
            g_nplanes = COLOR_GET_PLANE_COUNT(ins->layer[0].fmt);
            return g_nplanes;
        }
        if (ins->layer[0].layer == G2DLITE_TYPE_SPIPE) {
            return 4;
        }
    case 2:
        for (int i = 0; i < 2; i++) {
            if (ins->layer[i].layer == G2DLITE_TYPE_GPIPE) {
                g_nplanes = COLOR_GET_PLANE_COUNT(ins->layer[0].fmt);
                return (g_nplanes + 4);
            }
        }
    default:
        return 0;
    }
}

static u8 get_w_idx(struct g2dlite_output_cfg *output)
{
    return COLOR_GET_PLANE_COUNT(output->fmt) - 1;
}

static void g2dlite_rwdma_set(struct g2dlite_device *dev, u8 r_idx, u8 w_idx)
{
    unsigned int val;
    unsigned long base = dev->reg_base;

    G2DLITE_LOG_INFO("enter\n");
    for (int i = 0; i < CHN_COUNT; i++)
        g2dlite_write(base, G2D_RB000(i), r_wml_cfgs[r_idx][i]);

    for (int i = 0; i < CHN_COUNT; i++)
        g2dlite_write(base, G2D_RB004(i), r_d_depth_cfgs[r_idx][i]);

    for (int i = 0; i < CHN_COUNT; i++)
        g2dlite_write(base, G2D_RB008(i), r_c_depth_cfgs[r_idx][i]);

    val = reg_value(1, 0, G2D_SHIFT01_RB400, G2D_MASK01_RB400);
    val = reg_value(1, val, G2D_SHIFT00_RB400, G2D_MASK00_RB400);
    g2dlite_write(base, G2D_RB400, val);

    for (int i = 0; i < WCHN_COUNT; i++)
        g2dlite_write(base, G2D_RG000(i), w_wml_cfgs[w_idx][i]);

    for (int i = 0; i < WCHN_COUNT; i++)
        g2dlite_write(base, G2D_RG004(i),
                      w_d_depth_cfgs[w_idx][i]);

    for (int i = 0; i < WCHN_COUNT; i++)
        g2dlite_write(base, G2D_RG008(i),
                      w_c_depth_cfgs[w_idx][i]);

    G2DLITE_LOG_INFO("leave\n");
}

static void g2dlite_set_hscaler_coef(struct g2dlite_device *dev)
{
    const int hscaler_coef[33][5] = {
        {0, 0, 512, 0, 0},         {0, -5, 512, 5, 0},
        {0, -10, 511, 11, 0},      {0, -14, 509, 17, 0},
        {0, -18, 508, 23, -1},     {-1, -22, 507, 29, -1},
        {-1, -25, 503, 36, -1},    {-1, -28, 500, 43, -2},
        {-2, -31, 496, 51, -2},    {-2, -33, 491, 59, -3},
        {-3, -35, 486, 67, -3},    {-3, -37, 481, 75, -4},
        {-3, -39, 475, 84, -5},    {-4, -41, 471, 92, -6},
        {-5, -42, 464, 102, -7},   {-5, -43, 457, 111, -8},
        {-6, -43, 449, 121, -9},   {-6, -44, 442, 130, -10},
        {-7, -44, 435, 140, -12},  {-7, -44, 425, 151, -13},
        {-8, -44, 417, 161, -14},  {-8, -44, 408, 172, -16},
        {-9, -44, 399, 183, -17},  {-9, -43, 390, 193, -19},
        {-9, -42, 379, 204, -20},  {-10, -41, 369, 216, -22},
        {-10, -40, 358, 227, -23}, {-11, -39, 349, 238, -25},
        {-11, -38, 339, 249, -27}, {-11, -37, 327, 261, -28},
        {-11, -36, 317, 272, -30}, {-12, -34, 306, 283, -31},
        {-12, -33, 295, 295, -33},
    };
    unsigned int head, tail;
    unsigned long base = dev->reg_base;

    for (int i = 0; i < 33; i++) {
        head = ((hscaler_coef[i][0] & 0x7ff) << 12) |
               ((hscaler_coef[i][1] & 0x7ff) << 1) |
               ((hscaler_coef[i][2] & 0x400) >> 10);
        tail = ((hscaler_coef[i][2] & 0x3ff) << 22) |
               ((hscaler_coef[i][3] & 0x7ff) << 11) |
               (hscaler_coef[i][4] & 0x7ff);
        g2dlite_write(base, G2DLITE_HSCALER[0][0] + i * 4, head);
        g2dlite_write(base, G2DLITE_HSCALER[0][0] + 0x100 + i * 4, tail);
    }
}

static void g2dlite_set_vscaler_coef(struct g2dlite_device *dev)
{
    const int vscaler_coef[33][4] = {
        {0, 64, 0, 0},    {-1, 64, 1, 0},   {-1, 64, 1, 0},   {-2, 64, 2, 0},
        {-2, 63, 3, 0},   {-3, 63, 4, 0},   {-3, 62, 5, 0},   {-3, 62, 5, 0},
        {-4, 62, 6, 0},   {-4, 61, 7, 0},   {-4, 60, 8, 0},   {-5, 61, 9, -1},
        {-5, 60, 10, -1}, {-5, 59, 11, -1}, {-5, 57, 13, -1}, {-5, 56, 14, -1},
        {-5, 55, 15, -1}, {-5, 54, 16, -1}, {-5, 53, 17, -1}, {-5, 52, 19, -2},
        {-5, 51, 20, -2}, {-5, 50, 21, -2}, {-5, 49, 22, -2}, {-5, 47, 24, -2},
        {-5, 46, 25, -2}, {-5, 46, 26, -3}, {-5, 44, 28, -3}, {-5, 43, 29, -3},
        {-5, 41, 31, -3}, {-5, 40, 32, -3}, {-4, 39, 33, -4}, {-4, 37, 35, -4},
        {-4, 36, 36, -4},
    };
    unsigned int value;
    unsigned long base = dev->reg_base;

    for (uint32_t i = 0; i < 33; i++) {
        value = ((vscaler_coef[i][3] & 0xff) << 24) |
                ((vscaler_coef[i][2] & 0xff) << 16) |
                ((vscaler_coef[i][1] & 0xff) << 8) |
                ((vscaler_coef[i][0] & 0xff));
        g2dlite_write(base, G2DLITE_VSCALER[0][0] + i * 4, value);
    }
}

static int g2dlite_plane_frm_comp_set(struct g2dlite_device *dev,
                                      struct g2dlite_input_cfg *input)
{
    unsigned int compval;
    unsigned int frmc_val;
    unsigned char abits, ybits, ubits, vbits;
    unsigned char swap;
    unsigned char comp_swap;
    unsigned char is_yuv;
    unsigned char is_uvswap;
    unsigned char uv_mode;
    unsigned char data_mode;
    unsigned char buf_fmt;

    abits = COLOR_GET_A_BITS_NUM(input->fmt);
    ybits = COLOR_GET_Y_BITS_NUM(input->fmt);
    ubits = COLOR_GET_U_BITS_NUM(input->fmt);
    vbits = COLOR_GET_V_BITS_NUM(input->fmt);
    compval = reg_value(vbits, 0, G2D_SHIFT24_RC000, G2D_MASK24_RC000);
    compval = reg_value(ubits, compval, G2D_SHIFT16_RC000, G2D_MASK16_RC000);
    compval = reg_value(ybits, compval, G2D_SHIFT08_RC000, G2D_MASK08_RC000);
    compval = reg_value(abits, compval, G2D_SHIFT00_RC000, G2D_MASK00_RC000);

    swap = COLOR_GET_SWAP(input->fmt);
    comp_swap = COLOR_GET_COMP_SWAP(input->fmt);
    is_yuv = COLOR_GET_IS_YUV(input->fmt);
    is_uvswap = COLOR_GET_IS_UV_SWAP(input->fmt);
    uv_mode = COLOR_GET_UV_MODE(input->fmt);
    data_mode = COLOR_GET_DATA_MODE(input->fmt);
    buf_fmt = COLOR_GET_BUF_FMT(input->fmt);
    if ((input->layer == G2DLITE_TYPE_SPIPE) && input->rle.en) {
        data_mode = RLE_COMPR_MODE;
    }

    frmc_val =
        reg_value(swap, 0, G2D_SHIFT16_RC004, G2D_MASK16_RC004);
    frmc_val = reg_value(comp_swap, frmc_val, G2D_SHIFT12_RC004,
                         G2D_MASK12_RC004);
    frmc_val =
        reg_value(is_yuv, frmc_val, G2D_SHIFT08_RC004, G2D_MASK08_RC004);
    frmc_val =
        reg_value(is_uvswap, frmc_val, G2D_SHIFT07_RC004, G2D_MASK07_RC004);
    frmc_val =
        reg_value(uv_mode, frmc_val, G2D_SHIFT05_RC004, G2D_MASK05_RC004);
    frmc_val = reg_value(data_mode, frmc_val, G2D_SHIFT02_RC004, G2D_MASK02_RC004);
    frmc_val = reg_value(buf_fmt, frmc_val, G2D_SHIFT00_RC004, G2D_MASK00_RC004);

    if (input->layer == G2DLITE_TYPE_SPIPE) {
        g2dlite_cwrite(dev, 0, G2D_RD000, compval);
        g2dlite_cwrite(dev, 0, G2D_RD004, frmc_val);
    } else if (input->layer == G2DLITE_TYPE_GPIPE) {
        g2dlite_cwrite(dev, 0, G2D_RC000, compval);
        g2dlite_cwrite(dev, 0, G2D_RC004, frmc_val);
    } else {
        G2DLITE_LOG_ERR("input layer = %d,the number is incorrect\n",
                        input->layer);
        g2d_set_errcode(G2DLITE_MODULE_NO, G2D_ERRCODE_LAYER_TYPE, 0);
        return -1;
    }

    return 0;
}

static int g2dlite_wpipe_pixcomp_set(struct g2dlite_device *dev,
                                     struct g2dlite_output_cfg *output)
{
    unsigned int val;
    unsigned char swap;
    unsigned char abits, ybits, ubits, vbits;
    unsigned char uv_mode, buf_fmt, reforce;
    unsigned long base = dev->reg_base;

    switch (COLOR_GET_PLANE_COUNT(output->fmt)) {
    case 2:
    case 3:
        abits = 0;
        ybits = 8;
        ubits = 8;
        vbits = 8;
        break;
    default:
        abits = COLOR_GET_A_BITS_NUM(output->fmt);
        ybits = COLOR_GET_Y_BITS_NUM(output->fmt);
        ubits = COLOR_GET_U_BITS_NUM(output->fmt);
        vbits = COLOR_GET_V_BITS_NUM(output->fmt);
        break;
    }

    val = reg_value(vbits, 0, G2D_SHIFT24_RH004, G2D_MASK24_RH004);
    val = reg_value(ubits, val, G2D_SHIFT16_RH004, G2D_MASK16_RH004);
    val = reg_value(ybits, val, G2D_SHIFT08_RH004, G2D_MASK08_RH004);
    val = reg_value(abits, val, G2D_SHIFT00_RH004, G2D_MASK00_RH004);
    g2dlite_cwrite(dev, 0, G2D_RH004, val);

    swap = COLOR_GET_SWAP(output->fmt);
    uv_mode = COLOR_GET_UV_MODE(output->fmt);
    buf_fmt = COLOR_GET_BUF_FMT(output->fmt);
    reforce = (output->rotation == ROTATION_TYPE_NONE) ? 1 : 0;
    val = g2dlite_read(base, G2D_RH000);
    val = reg_value(swap, val, G2D_SHIFT16_RH000,
                    G2D_MASK16_RH000);
    val = reg_value(reforce, val, G2D_SHIFT08_RH000,
                    G2D_MASK08_RH000);
    val = reg_value(uv_mode, val, G2D_SHIFT06_RH000, G2D_MASK06_RH000);
    val = reg_value(buf_fmt, val, G2D_SHIFT04_RH000, G2D_MASK04_RH000);
    val = reg_value(output->rotation, val, G2D_SHIFT01_RH000,
                    G2D_MASK03_RH000 | G2D_MASK02_RH000 |
                        G2D_MASK01_RH000);
    g2dlite_cwrite(dev, 0, G2D_RH000, val);

    return 0;
}

static int g2dlite_wp_csc_set(struct g2dlite_device *dev,
                              struct g2dlite_output_cfg *output,
                              unsigned char flag)
{
    unsigned int val;
    unsigned char uv_mode;
    unsigned char is_yuv;
    unsigned char is_yuv420 = 0;
    unsigned long base = dev->reg_base;

    is_yuv = COLOR_GET_IS_YUV(output->fmt);
    uv_mode = COLOR_GET_UV_MODE(output->fmt);

    if (uv_mode == UV_YUV420)
        is_yuv420 = 1;

    if (flag & G2DLITE_FUN_BLEND) {
        val = g2dlite_read(base, G2D_RH040);
        val = reg_value(!is_yuv, val, G2D_SHIFT00_RH040,
                        G2D_MASK00_RH040);
        g2dlite_cwrite(dev, 0, G2D_RH040, val);
    }

    val = g2dlite_read(base, G2D_RH008);
    val = reg_value(!is_yuv420, val, G2D_SHIFT09_RH008,
                    G2D_MASK09_RH008);
    val = reg_value(!is_yuv, val, G2D_SHIFT08_RH008,
                    G2D_MASK08_RH008);
    g2dlite_cwrite(dev, 0, G2D_RH008, val);

    return 0;
}

static int g2dlite_init(struct g2dlite_device *dev)
{
    G2DLITE_LOG_INFO("enter\n");

    g2dlite_com_regs_init(dev);
    g2dlite_rdma_init(dev);
    g2dlite_gpipe_init(dev);
    g2dlite_spipe_init(dev);
    g2dlite_mlc_init(dev);
    g2dlite_wdma_init(dev);
    g2dlite_wpipe_init(dev);

    g2dlite_set_hscaler_coef(dev);
    g2dlite_set_vscaler_coef(dev);

    G2DLITE_LOG_INFO("leave\n");

    return 0;
}

static int g2dlite_reset(struct g2dlite_device *dev)
{
    unsigned int val;
    unsigned long base = dev->reg_base;

    G2DLITE_LOG_FUNC();
    val = g2dlite_read(base, G2D_RA000);
    val = reg_value(1, val, G2D_SHIFT31_RA000, G2D_MASK31_RA000);
    g2dlite_write(base, G2D_RA000, val);

    udelay(10);

    val = reg_value(0, val, G2D_SHIFT31_RA000, G2D_MASK31_RA000);
    g2dlite_write(base, G2D_RA000, val);

    return 0;
}

static int g2dlite_gpipe_set(struct g2dlite_device *dev, char cmdid,
                             struct g2dlite_input_cfg *input)
{
    const unsigned int p_hs = 524288;
    const unsigned int p_vs = 262114;

    float ratio;
    unsigned int val;
    unsigned int size;
    unsigned int vs_inc;
    unsigned char nplanes;
    int ratio_int, ratio_fra;
    unsigned long base = dev->reg_base;
    unsigned long input_addr;
    unsigned char is_yuv = 0;

    nplanes = COLOR_GET_PLANE_COUNT(input->fmt);
    if (!cmdid)
        g2dlite_plane_frm_comp_set(dev, input);

    /*pipe related*/
    size = reg_value(input->src.w - 1, 0, G2D_SHIFT00_RC008, G2D_MASK00_RC008);
    size = reg_value(input->src.h - 1, size, G2D_SHIFT16_RC008, G2D_MASK16_RC008);
    g2dlite_cwrite(dev, cmdid, G2D_RC008, size);

    input_addr = p2ap((paddr_t)input->addr[0]);
    g2dlite_cwrite(dev, cmdid, G2D_RC00C, get_l_addr(input_addr));
    g2dlite_cwrite(dev, cmdid, G2D_RC010, get_h_addr(input_addr));
    g2dlite_cwrite(dev, cmdid, G2D_RC02C, input->src_stride[0] - 1);
    val = reg_value(input->src.x, 0, G2D_SHIFT00_RC040, G2D_MASK00_RC040);
    val = reg_value(input->src.y, val, G2D_SHIFT16_RC040, G2D_MASK16_RC040);
    g2dlite_cwrite(dev, cmdid, G2D_RC040, val);

    if (nplanes > 1) {
        input_addr = p2ap((paddr_t)input->addr[1]);
        g2dlite_cwrite(dev, cmdid, G2D_RC014,
                       get_l_addr(input_addr));
        g2dlite_cwrite(dev, cmdid, G2D_RC018,
                       get_h_addr(input_addr));
        g2dlite_cwrite(dev, cmdid, G2D_RC030,
                       input->src_stride[1] - 1);
    }

    if (nplanes > 2) {
        input_addr = p2ap((paddr_t)input->addr[2]);
        g2dlite_cwrite(dev, cmdid, G2D_RC01C,
                       get_l_addr(input_addr));
        g2dlite_cwrite(dev, cmdid, G2D_RC020,
                       get_h_addr(input_addr));
        g2dlite_cwrite(dev, cmdid, G2D_RC034,
                       input->src_stride[2] - 1);
    }

    /*yuv params*/
    is_yuv = COLOR_GET_IS_YUV(input->fmt);
    val = g2dlite_read(base, G2D_RC044);
    val = reg_value(is_yuv, val, G2D_SHIFT31_RC044,
                    G2D_MASK31_RC044);
    val = reg_value(is_yuv ? 0 : 1, val, G2D_SHIFT00_RC044,
                    G2D_MASK00_RC044);
    g2dlite_cwrite(dev, cmdid, G2D_RC044, val);

    val = g2dlite_read(base, G2D_RC200);
    val =
        reg_value(is_yuv ? 0 : 1, val, G2D_SHIFT00_RC200, G2D_MASK00_RC200);
    g2dlite_cwrite(dev, cmdid, G2D_RC200, val);

    /*scaler part*/
    if (input->src.w == input->dst.w) {
        val = g2dlite_read(base, G2D_RC300);
        val = reg_value(0, val, G2D_SHIFT03_RC300,
                        G2D_MASK03_RC300);
        val = reg_value(0, val, G2D_SHIFT02_RC300,
                        G2D_MASK02_RC300);
        val = reg_value(0, val, G2D_SHIFT01_RC300,
                        G2D_MASK01_RC300);
        val = reg_value(0, val, G2D_SHIFT00_RC300,
                        G2D_MASK00_RC300);
        g2dlite_cwrite(dev, cmdid, G2D_RC300, val);

        val = reg_value(1, 0, G2D_SHIFT19_RC308, G2D_MASK19_RC308);
        val = reg_value(0, val, G2D_SHIFT00_RC308, G2D_MASK00_RC308);
        g2dlite_cwrite(dev, cmdid, G2D_RC308, val);
    } else {
        ratio = (float)input->src.w / input->dst.w;
        ratio_int = ratio;
        ratio_fra = (ratio - ratio_int) * p_hs;

        val = g2dlite_read(base, G2D_RC300);
        val = reg_value(1, val, G2D_SHIFT03_RC300,
                        G2D_MASK03_RC300);
        val = reg_value(1, val, G2D_SHIFT02_RC300,
                        G2D_MASK02_RC300);
        val = reg_value(1, val, G2D_SHIFT01_RC300,
                        G2D_MASK01_RC300);
        val = reg_value(1, val, G2D_SHIFT00_RC300,
                        G2D_MASK00_RC300);
        g2dlite_cwrite(dev, cmdid, G2D_RC300, val);

        val = reg_value(ratio_int, 0, G2D_SHIFT19_RC308,
                        G2D_MASK19_RC308);
        val = reg_value(ratio_fra, val, G2D_SHIFT00_RC308,
                        G2D_MASK00_RC308);
        g2dlite_cwrite(dev, cmdid, G2D_RC308, val);
    }
    g2dlite_cwrite(dev, cmdid, G2D_RC30C, input->dst.w - 1);

    size = reg_value(input->src.h - 1, 0, G2D_SHIFT16_RC108,
                     G2D_MASK16_RC108);
    size = reg_value(input->dst.w - 1, size, G2D_SHIFT00_RC108,
                     G2D_MASK00_RC108);
    g2dlite_cwrite(dev, cmdid, G2D_RC108, size);

    if (input->src.h == input->dst.h) {
        val = g2dlite_read(base, G2D_RC400);
        val = reg_value(0, val, G2D_SHIFT00_RC400,
                        G2D_MASK00_RC400);
        g2dlite_cwrite(dev, cmdid, G2D_RC400, val);

        g2dlite_cwrite(dev, cmdid, G2D_RC408, 0);
    } else {
        vs_inc = (1.0 * (input->src.h - 1)) / (input->dst.h - 1) * p_vs;
        val = g2dlite_read(base, G2D_RC400);
        val = reg_value(2, val, G2D_SHIFT00_RC400,
                        G2D_MASK00_RC400);
        g2dlite_cwrite(dev, cmdid, G2D_RC400, val);

        g2dlite_cwrite(dev, cmdid, G2D_RC408, vs_inc);
    }
    g2dlite_cwrite(dev, cmdid, G2D_RC404, input->dst.h - 1);

    return 0;
}

static int g2dlite_spipe_set(struct g2dlite_device *dev, char cmdid,
                             struct g2dlite_input_cfg *input)
{
    unsigned int val;
    unsigned int size;
    unsigned long input_addr;

    if (!cmdid)
        g2dlite_plane_frm_comp_set(dev, input);

    /*pipe related*/
    size = reg_value(input->src.w - 1, 0, G2D_SHIFT00_RC008, G2D_MASK00_RC008);
    size = reg_value(input->src.h - 1, size, G2D_SHIFT16_RC008, G2D_MASK16_RC008);
    g2dlite_cwrite(dev, cmdid, G2D_RD008, size);

    input_addr = p2ap((paddr_t)input->addr[0]);
    g2dlite_cwrite(dev, cmdid, G2D_RD00C, get_l_addr(input_addr));
    g2dlite_cwrite(dev, cmdid, G2D_RD010, get_h_addr(input_addr));
    g2dlite_cwrite(dev, cmdid, G2D_RD02C, input->src_stride[0] - 1);
    val = reg_value(input->src.x, 0, G2D_SHIFT00_RC040, G2D_MASK00_RC040);
    val = reg_value(input->src.y, val, G2D_SHIFT16_RC040, G2D_MASK16_RC040);
    g2dlite_cwrite(dev, cmdid, G2D_RD040, val);

    /*rle*/
    val = reg_value(input->rle.en, 0, G2D_SHIFT00_RD120, G2D_MASK00_RD120);
    if (input->rle.en) {
        val = reg_value(input->rle.rle_data_size, val, G2D_SHIFT01_RD120,
                        G2D_MASK01_RD120);

        g2dlite_cwrite(dev, cmdid, G2D_RD100, input->rle.rle_y_len);
        g2dlite_cwrite(dev, cmdid, G2D_RD110,
                       input->rle.rle_y_checksum);
    }
    g2dlite_cwrite(dev, cmdid, G2D_RD120, val);

    /*clut*/
    if (input->clut.en) {
        u8 bpa = COLOR_GET_A_BITS_NUM(input->fmt);
        u8 bpy = COLOR_GET_A_BITS_NUM(input->fmt);
        u8 bpu = COLOR_GET_A_BITS_NUM(input->fmt);
        u8 bpv = COLOR_GET_A_BITS_NUM(input->fmt);

        /*G2D_RD200*/
        val = g2dlite_read(dev->reg_base, G2D_RD200);
        if (bpa) {
            val = reg_value(0, val, G2D_SHIFT16_RD200, G2D_MASK16_RD200);
            val = reg_value(bpa, val, G2D_SHIFT00_RD200, G2D_MASK00_RD200);
        } else {
            val = reg_value(1, val, G2D_SHIFT16_RD200, G2D_MASK16_RD200);
        }
        g2dlite_cwrite(dev, cmdid, G2D_RD200, val);

        /*G2D_RD204*/
        val = g2dlite_read(dev->reg_base, G2D_RD204);
        if (bpy) {
            val = reg_value(0, val, G2D_SHIFT16_RD204, G2D_MASK16_RD204);
            val = reg_value(bpy, val, G2D_SHIFT00_RD204, G2D_MASK00_RD204);
        } else {
            val = reg_value(1, val, G2D_SHIFT16_RD204, G2D_MASK16_RD204);
        }
        g2dlite_cwrite(dev, cmdid, G2D_RD204, val);

        /*G2D_RD208*/
        val = g2dlite_read(dev->reg_base, G2D_RD208);
        if (bpu) {
            val = reg_value(0, val, G2D_SHIFT16_RD208, G2D_MASK16_RD208);
            val = reg_value(bpu, val, G2D_SHIFT00_RD208, G2D_MASK00_RD208);
        } else {
            val = reg_value(1, val, G2D_SHIFT16_RD208, G2D_MASK16_RD208);
        }
        g2dlite_cwrite(dev, cmdid, G2D_RD208, val);

        /*G2D_RD20C*/
        val = g2dlite_read(dev->reg_base, G2D_RD20C);
        if (bpv) {
            val = reg_value(0, val, G2D_SHIFT16_RD20C, G2D_MASK16_RD20C);
            val = reg_value(bpv, val, G2D_SHIFT00_RD20C, G2D_MASK00_RD20C);
        } else {
            val = reg_value(1, val, G2D_SHIFT16_RD20C, G2D_MASK16_RD20C);
        }
        g2dlite_cwrite(dev, cmdid, G2D_RD20C, val);
    } else {
        /*G2D_RD200*/
        val = g2dlite_read(dev->reg_base, G2D_RD200);
        val = reg_value(1, val, G2D_SHIFT16_RD200, G2D_MASK16_RD200);
        g2dlite_cwrite(dev, cmdid, G2D_RD200, val);

        /*G2D_RD204*/
        val = g2dlite_read(dev->reg_base, G2D_RD204);
        val = reg_value(1, val, G2D_SHIFT16_RD204, G2D_MASK16_RD204);
        g2dlite_cwrite(dev, cmdid, G2D_RD204, val);

        /*G2D_RD208*/
        val = g2dlite_read(dev->reg_base, G2D_RD208);
        val = reg_value(1, val, G2D_SHIFT16_RD208, G2D_MASK16_RD208);
        g2dlite_cwrite(dev, cmdid, G2D_RD208, val);

        /*G2D_RD20C*/
        val = g2dlite_read(dev->reg_base, G2D_RD20C);
        val = reg_value(1, val, G2D_SHIFT16_RD20C, G2D_MASK16_RD20C);
        g2dlite_cwrite(dev, cmdid, G2D_RD20C, val);
    }

    return 0;
}

static void g2dlite_close_fastcopy(struct g2dlite_device *dev)
{
    unsigned int val;
    unsigned long base = dev->reg_base;

    /*disable background in order to remove fastcopy
     *bring affect when fastcopy using before other
     *functions*/
    val = g2dlite_read(base, G2D_RF004);
    val = reg_value(0, val, G2D_SHIFT00_RF004,
                    G2D_MASK00_RF004);
    g2dlite_write(base, G2D_RF004, val);

    val = g2dlite_read(base, G2D_RE220);
    val = reg_value(0, val, G2D_SHIFT02_RE220, G2D_MASK02_RE220);
    val = reg_value(0, val, G2D_SHIFT01_RE220, G2D_MASK01_RE220);
    g2dlite_write(base, G2D_RE220, val);
}

static int g2dlite_mlc_set(struct g2dlite_device *dev, char cmdid,
                           struct g2dlite_input *ins)
{
    u32 val, sf_val[G2DLITE_MLC_MAX], path_val[MLC_PATH_MAX];
    u64 base = dev->reg_base;
    u8 pd_types_flag = 0;

    /*disable all layers*/
    for (int i = 0; i < G2DLITE_MLC_MAX; i++) {
        val = g2dlite_read(base, G2D_RE000(i));
        sf_val[i] = reg_value(0, val, G2D_SHIFT00_RE000, G2D_MASK00_RE000);
        g2dlite_cwrite(dev, cmdid, G2D_RE000(i), sf_val[i]);
    }

    /*disable all paths*/
    for (int i = 0; i < MLC_PATH_MAX; i++) {
        path_val[i] = 0xF777F;
        g2dlite_cwrite(dev, cmdid, G2D_RE200(i), path_val[i]);
    }

    if (ins->bg_layer.en) {
        struct g2dlite_bg_cfg *bgcfg = &ins->bg_layer;

        if (ins->pd_info.en && (ins->bg_layer.pd_type != PD_NONE)) {
            switch (ins->bg_layer.pd_type) {
            case PD_SRC:
                path_val[0] = reg_value(0, path_val[0], G2D_SHIFT04_RE200,
                                        G2D_MASK04_RE200);
                pd_types_flag |= PD_SRC;
                break;
            case PD_DST:
                path_val[0] = reg_value(0, path_val[0], G2D_SHIFT08_RE200,
                                        G2D_MASK08_RE200);
                pd_types_flag |= PD_DST;
                break;
            default:
			{
                G2DLITE_LOG_ERR(
                    "input bglayer porter-duff type isn't correct\n");
                g2d_set_errcode(G2DLITE_MODULE_NO, G2D_ERRCODE_PORTER_DUF, 0);
                return -1;
            }
        }

            path_val[0] = reg_value(G2DLITE_PD_IDX, path_val[0],
                                    G2D_SHIFT00_RE200, G2D_MASK00_RE200);
        } else {
            /*PATH with zorder*/
            path_val[0] = reg_value(bgcfg->zorder, path_val[0],
                                    G2D_SHIFT00_RE200, G2D_MASK00_RE200);
            g2dlite_cwrite(dev, cmdid, G2D_RE200(0), path_val[0]);

            path_val[bgcfg->zorder] =
                reg_value(0, path_val[bgcfg->zorder], G2D_SHIFT16_RE200,
                          G2D_MASK16_RE200);
            g2dlite_cwrite(dev, cmdid, G2D_RE200(bgcfg->zorder),
                           path_val[bgcfg->zorder]);
        }
    }

    for (int i = 0; i < G2DLITE_MLC_MAX; i++) {
        struct g2dlite_input_cfg *input = &ins->layer[i];

        if (!input->layer_en)
            continue;

        u8 layer_index = input->layer;
        u8 path_index = input->layer + 1;

        if (layer_index >= G2DLITE_MLC_MAX) {
            G2DLITE_LOG_ERR("input layer = %d, the number isn't correct\n",
                            input->layer);
            g2d_set_errcode(G2DLITE_MODULE_NO, G2D_ERRCODE_LAYER_INDEX, 0);
            return -1;
        }

        /*POS*/
        g2dlite_cwrite(dev, cmdid, G2D_RE004(layer_index), input->dst.x);
        g2dlite_cwrite(dev, cmdid, G2D_RE008(layer_index), input->dst.y);

        /*dst size*/
        val = reg_value(input->dst.h - 1, 0, G2D_SHIFT16_RE00C,
                        G2D_MASK16_RE00C);
        val = reg_value(input->dst.w - 1, val, G2D_SHIFT00_RE00C,
                        G2D_MASK00_RE00C);
        g2dlite_cwrite(dev, cmdid, G2D_RE00C(layer_index), val);

        if (ins->pd_info.en && (input->pd_type != PD_NONE)) {
            if (pd_types_flag == (PD_SRC | PD_DST)) {
                G2DLITE_LOG_ERR("porter-duff type was already input\n");
                g2d_set_errcode(G2DLITE_MODULE_NO, G2D_ERRCODE_PORTER_DUF, 0);
                return -1;
            }

            switch (input->pd_type) {
            case PD_SRC:
                path_val[0] = reg_value(path_index, path_val[0],
                                        G2D_SHIFT04_RE200, G2D_MASK04_RE200);
                pd_types_flag |= PD_SRC;
                break;
            case PD_DST:
                path_val[0] = reg_value(path_index, path_val[0],
                                        G2D_SHIFT08_RE200, G2D_MASK08_RE200);
                pd_types_flag |= PD_DST;
                break;
            default:
                G2DLITE_LOG_ERR(
                    "input layer%d porter-duff type isn't correct\n",
                    input->layer);
                return -1;
            }

            path_val[path_index] =
                reg_value(G2DLITE_PD_IDX, path_val[path_index],
                          G2D_SHIFT00_RE200, G2D_MASK00_RE200);

            switch (input->blend) {
            case BLEND_PIXEL_NONE:
            default:
                sf_val[layer_index] =
                    reg_value(1, sf_val[layer_index], G2D_SHIFT02_RE000,
                              G2D_MASK02_RE000);
                g2dlite_cwrite(dev, cmdid, G2D_RE018(layer_index),
                               input->alpha);
                break;
            case BLEND_PIXEL_COVERAGE:
                sf_val[layer_index] =
                    reg_value(0, sf_val[layer_index], G2D_SHIFT02_RE000,
                              G2D_MASK02_RE000);
                break;
            }
        } else {
            /*PATH with zorder*/
            path_val[path_index] =
                reg_value(input->zorder, path_val[path_index],
                          G2D_SHIFT00_RE200, G2D_MASK00_RE200);
            g2dlite_cwrite(dev, cmdid, G2D_RE200(path_index),
                           path_val[path_index]);

            path_val[input->zorder] =
                reg_value(path_index, path_val[input->zorder],
                          G2D_SHIFT16_RE200, G2D_MASK16_RE200);
            g2dlite_cwrite(dev, cmdid, G2D_RE200(input->zorder),
                           path_val[input->zorder]);

            /*Blend mode*/
            switch (input->blend) {
            case BLEND_PIXEL_NONE:
            default:
                sf_val[layer_index] =
                    reg_value(1, sf_val[layer_index], G2D_SHIFT02_RE000,
                              G2D_MASK02_RE000);
                g2dlite_cwrite(dev, cmdid, G2D_RE018(layer_index),
                               input->alpha);
                break;
            case BLEND_PIXEL_PREMULTI:
                sf_val[layer_index] =
                    reg_value(0, sf_val[layer_index], G2D_SHIFT02_RE000,
                              G2D_MASK02_RE000);

                path_val[input->zorder] = reg_value(1, path_val[input->zorder],
                                                    G2D_SHIFT29_RE200, G2D_MASK29_RE200);
                g2dlite_cwrite(dev, cmdid, G2D_RE200(input->zorder),
                               path_val[input->zorder]);
                break;
            case BLEND_PIXEL_COVERAGE:
                sf_val[layer_index] =
                    reg_value(0, sf_val[layer_index], G2D_SHIFT02_RE000,
                              G2D_MASK02_RE000);
                break;
            }
        }

        /*enable layer*/
        sf_val[layer_index] =
            reg_value(1, sf_val[layer_index], G2D_SHIFT00_RE000, G2D_MASK00_RE000);

        /*ckey*/
        sf_val[layer_index] =
            reg_value(input->ckey.en, sf_val[layer_index], G2D_SHIFT03_RE000,
                      G2D_MASK03_RE000);
        g2dlite_cwrite(dev, cmdid, G2D_RE000(layer_index),
                       sf_val[layer_index]);
        if (input->ckey.en) {
            g2dlite_cwrite(dev, cmdid, G2D_RE01C(layer_index),
                           input->ckey.alpha);
            val = reg_value(input->ckey.range.r_dn, 0, G2D_SHIFT00_RE020,
                            G2D_MASK00_RE020);
            val = reg_value(input->ckey.range.r_up, val,
                            G2D_SHIFT16_RE020, G2D_MASK16_RE020);
            g2dlite_cwrite(dev, cmdid, G2D_RE020(layer_index), val);
            val = reg_value(input->ckey.range.g_dn, 0, G2D_SHIFT00_RE020,
                            G2D_MASK00_RE020);
            val = reg_value(input->ckey.range.g_up, val,
                            G2D_SHIFT16_RE020, G2D_MASK16_RE020);
            g2dlite_cwrite(dev, cmdid, G2D_RE024(layer_index), val);
            val = reg_value(input->ckey.range.b_dn, 0, G2D_SHIFT00_RE020,
                            G2D_MASK00_RE020);
            val = reg_value(input->ckey.range.b_up, val,
                            G2D_SHIFT16_RE020, G2D_MASK16_RE020);
            g2dlite_cwrite(dev, cmdid, G2D_RE028(layer_index), val);
        }
    }

    if (ins->pd_info.en) {
        path_val[0] = reg_value(ins->pd_info.mode, path_val[0], G2D_SHIFT20_RE200,
                                G2D_MASK20_RE200);
        path_val[0] = reg_value(ins->pd_info.zorder, path_val[0],
                                G2D_SHIFT12_RE200, G2D_MASK12_RE200);
        path_val[ins->pd_info.zorder] =
            reg_value(G2DLITE_PD_IDX, path_val[ins->pd_info.zorder],
                      G2D_SHIFT16_RE200, G2D_MASK16_RE200);

        for (int i = 0; i < MLC_PATH_MAX; i++) {
            g2dlite_cwrite(dev, cmdid, G2D_RE200(i), path_val[i]);
        }

        /*We bypass alpha blend for output alpha.If we input 3 layers(2 layers
         for porter-duff, 1 layer for alpha blend with porter-duff output, we
         must clear this bit. G2DLite W-pipe output set 0xFF to alpha value.
        */
        val = g2dlite_read(base, G2D_RE220);
        if (ins->pd_info.alpha_need)
            val = reg_value(1, val, G2D_SHIFT00_RE220, G2D_MASK00_RE220);
        else
            val = reg_value(0, val, G2D_SHIFT00_RE220, G2D_MASK00_RE220);
        g2dlite_cwrite(dev, cmdid, G2D_RE220, val);
    } else {
        val = g2dlite_read(base, G2D_RE220);
        val = reg_value(0, val, G2D_SHIFT00_RE220, G2D_MASK00_RE220);
        g2dlite_cwrite(dev, cmdid, G2D_RE220, val);
    }

    return 0;
}

static int g2dlite_wpipe_set(struct g2dlite_device *dev, char cmdid,
                             struct g2dlite_output_cfg *output)
{
    unsigned int val;
    unsigned char nplanes;
    unsigned char bpp[3] = {0};
    unsigned long input_addr;

    if (!cmdid) {
        g2dlite_wpipe_pixcomp_set(dev, output);
        g2dlite_wp_csc_set(dev, output, G2DLITE_FUN_BLEND);
    }

    val = reg_value(output->height - 1, 0, G2D_SHIFT16_RA00C,
                    G2D_MASK16_RA00C);
    val = reg_value(output->width - 1, val, G2D_SHIFT00_RA00C,
                    G2D_MASK00_RA00C);
    g2dlite_cwrite(dev, cmdid, G2D_RA00C, val);

    nplanes = COLOR_GET_PLANE_COUNT(output->fmt);
    get_bpp_by_format(output->fmt, bpp);
    input_addr = p2ap((paddr_t)output->addr[0]);
    g2dlite_cwrite(dev, cmdid, G2D_RH010, get_l_addr(input_addr));
    g2dlite_cwrite(dev, cmdid, G2D_RH014, get_h_addr(input_addr));
    g2dlite_cwrite(dev, cmdid, G2D_RH028, output->stride[0] - 1);

    if (nplanes > 1) {
        input_addr = p2ap((paddr_t)output->addr[1]);
        g2dlite_cwrite(dev, cmdid, G2D_RH018, get_l_addr(input_addr));
        g2dlite_cwrite(dev, cmdid, G2D_RH01C, get_h_addr(input_addr));
        g2dlite_cwrite(dev, cmdid, G2D_RH02C, output->stride[1] - 1);
    }

    if (nplanes > 2) {
        input_addr = p2ap((paddr_t)output->addr[2]);
        g2dlite_cwrite(dev, cmdid, G2D_RH020, get_l_addr(input_addr));
        g2dlite_cwrite(dev, cmdid, G2D_RH024, get_h_addr(input_addr));
        g2dlite_cwrite(dev, cmdid, G2D_RH030, output->stride[2] - 1);
    }

    return 0;
}

static void g2dlite_slice_input(struct g2dlite_input_cfg *src,
                                struct g2dlite_input_cfg *slice_input,
                                unsigned int slice_w,
                                unsigned char max_slice_num,
                                unsigned char slice_idx)
{
    unsigned int slice_width;
    unsigned long src_addr;
    unsigned char nplanes;
    unsigned char bpp[3] = {0};
    /* copy */
    if (!slice_idx) {
        slice_input->layer = src->layer;
        slice_input->layer_en = src->layer_en;
        slice_input->fmt = src->fmt;
        slice_input->zorder = src->zorder;
        slice_input->src.x = src->src.x;
        slice_input->src.y = src->src.y;
        slice_input->src.h = src->src.h;
        slice_input->dst.x = src->dst.x;
        slice_input->dst.y = src->dst.y;
        slice_input->dst.h = src->dst.h;
        slice_input->ckey.en = src->ckey.en;
        slice_input->ckey = src->ckey;
        slice_input->blend = src->blend;
        slice_input->alpha = src->alpha;
        slice_input->src_stride[0] = src->src_stride[0];
        slice_input->src_stride[1] = src->src_stride[1];
        slice_input->src_stride[2] = src->src_stride[2];
        slice_input->src_stride[3] = src->src_stride[3];
    }

    /*change to slice*/
    nplanes = COLOR_GET_PLANE_COUNT(src->fmt);
    get_bpp_by_format(src->fmt, bpp);
    slice_width = (slice_idx == (max_slice_num - 1))
                      ? ((src->dst.w % slice_w) ?: slice_w)
                      : slice_w;
    slice_input->src.w = slice_width;
    slice_input->dst.w = slice_width;
    src_addr = src->addr[0] + slice_idx * slice_w * bpp[0] / 8;
    slice_input->addr[0] = src_addr;
    slice_input->src_stride[0] = src->src_stride[0];
    if (nplanes > 1) {
        src_addr = src->addr[1] + slice_idx * slice_w * bpp[1] / 8;
        slice_input->addr[1] = src_addr;
    }
    if (nplanes > 2) {
        src_addr = src->addr[2] + slice_idx * slice_w * bpp[2] / 8;
        slice_input->addr[2] = src_addr;
    }
}

static void g2dlite_slice_output(struct g2dlite_output_cfg *src,
                                 struct g2dlite_output_cfg *slice_output,
                                 unsigned int slice_w,
                                 unsigned char max_slice_num,
                                 unsigned char slice_idx)
{
    unsigned int slice_width;
    // unsigned long src_addr;
    unsigned char nplanes;
    unsigned char bpp[3] = {0};

    if (!slice_idx) {
        slice_output->height = src->height;
        slice_output->fmt = src->fmt;
        slice_output->stride[0] = src->stride[0];
        slice_output->stride[1] = src->stride[1];
        slice_output->stride[2] = src->stride[2];
        slice_output->stride[3] = src->stride[3];
        slice_output->rotation = src->rotation;
    }
    nplanes = COLOR_GET_PLANE_COUNT(src->fmt);
    get_bpp_by_format(src->fmt, bpp);
    slice_width = (slice_idx == (max_slice_num - 1))
                      ? ((src->width % slice_w) ?: slice_w)
                      : slice_w;
    slice_output->width = slice_width;
    switch (src->rotation) {
    case ROTATION_TYPE_NONE:
    case ROTATION_TYPE_VFLIP:
        slice_output->addr[0] = src->addr[0] + slice_idx * slice_w * bpp[0] / 8;
        if (nplanes > 1) {
            slice_output->addr[1] =
                src->addr[1] + slice_idx * slice_w * bpp[1] / 8;
        }
        if (nplanes > 2) {
            slice_output->addr[2] =
                src->addr[2] + slice_idx * slice_w * bpp[2] / 8;
        }
        break;
    case ROTATION_TYPE_ROT_90:
    case ROTATION_TYPE_HF_90:
        slice_output->addr[0] =
            src->addr[0] + slice_idx * slice_w * src->stride[0];
        if (nplanes > 1) {
            slice_output->addr[1] =
                src->addr[1] + slice_idx * slice_w * src->stride[1];
        }
        if (nplanes > 2) {
            slice_output->addr[2] =
                src->addr[2] + slice_idx * slice_w * src->stride[2];
        }
        break;
    case ROTATION_TYPE_HFLIP:
    case ROTATION_TYPE_ROT_180:
        slice_output->addr[0] =
            src->addr[0] +
            (src->width - (slice_idx * slice_w + slice_width)) * bpp[0] / 8;
        if (nplanes > 1) {
            slice_output->addr[1] =
                src->addr[1] +
                (src->width - (slice_idx * slice_w + slice_width)) * bpp[1] / 8;
        }
        if (nplanes > 2) {
            slice_output->addr[2] =
                src->addr[2] +
                (src->width - (slice_idx * slice_w + slice_width)) * bpp[2] / 8;
        }
        break;
    case ROTATION_TYPE_ROT_270:
    case ROTATION_TYPE_VF_90:
        slice_output->addr[0] =
            src->addr[0] +
            (src->width - (slice_idx * slice_w + slice_width)) * src->stride[0];
        if (nplanes > 1) {
            slice_output->addr[1] =
                src->addr[1] +
                (src->width - (slice_idx * slice_w + slice_width)) *
                    src->stride[1];
        }
        if (nplanes > 2) {
            slice_output->addr[2] =
                src->addr[2] +
                (src->width - (slice_idx * slice_w + slice_width)) *
                    src->stride[2];
        }
        break;
    default:
        break;
    }
}

static int g2dlite_rotation(struct g2dlite_device *dev,
                            struct g2dlite_input *ins)
{
    G2DLITE_LOG_INFO("enter\n");
    const unsigned int slice_w = 256;
    unsigned int slice_count;
    struct g2dlite_input slice_ins;
    memset(&slice_ins, 0, sizeof(struct g2dlite_input));
    struct g2dlite_input_cfg *input = &slice_ins.layer[0];
    struct g2dlite_output_cfg *output = &slice_ins.output;

    if (!dev || !ins) {
        G2DLITE_LOG_ERR("dev or input isn't inited.[dev:%p, ins:%p]\n", dev,
                        ins);
        return -1;
    }

    u8 r_idx = get_r_idx(ins);
    u8 w_idx = get_w_idx(&ins->output);
    g2dlite_rwdma_set(dev, r_idx, w_idx);
    g2dlite_close_fastcopy(dev);

    slice_count = (ins->layer[0].dst.w / slice_w) +
                  ((ins->layer[0].dst.w % slice_w) ? 1 : 0);
    for (unsigned int i = 0; i < slice_count; i++) {
        G2DLITE_LOG_INFO("slice[%d|%d]\n", i, slice_count);
        g2dlite_slice_input(&ins->layer[0], input, slice_w, slice_count, i);
        g2dlite_gpipe_set(dev, i, input);
        g2dlite_mlc_set(dev, i, &slice_ins);

        g2dlite_slice_output(&ins->output, output, slice_w, slice_count, i);
        g2dlite_wpipe_set(dev, i, output);
    }

    G2DLITE_LOG_INFO("leave\n");
    return 0;
}

static int g2dlite_bg_set(struct g2dlite_device *dev,
                          struct g2dlite_bg_cfg *bgcfg,
                          struct g2dlite_output_cfg *output)
{

    u32 val, a_addr_l, a_addr_h, a_stride;
    unsigned long base = dev->reg_base;
    unsigned long a_addr;
    u8 alpha_select = 0;

    if (bgcfg->aaddr) {
        a_addr = p2ap((paddr_t)bgcfg->aaddr);
        a_addr_l = get_l_addr(a_addr);
        a_addr_h = get_h_addr(a_addr);
        a_stride = bgcfg->astride;
        alpha_select = 1;
        if (!a_stride)
            a_stride = (output->width * bgcfg->bpa + 7) / 8 - 1;

        /*G2D_RF000*/
        val = g2dlite_read(base, G2D_RF000);
        val = reg_value(bgcfg->bpa, val, G2D_SHIFT00_RF000,
                        G2D_MASK00_RF000);
        g2dlite_cwrite(dev, 0, G2D_RF000, val);

        /*G2D_RF008*/
        val = reg_value(output->height - 1, 0, G2D_SHIFT16_RF008,
                        G2D_MASK16_RF008);
        val = reg_value(output->width - 1, val, G2D_SHIFT00_RF008,
                        G2D_MASK00_RF008);
        g2dlite_cwrite(dev, 0, G2D_RF008, val);

        /*G2D_RF00C*/
        val = reg_value(a_addr_l, 0, G2D_SHIFT00_RF00C, G2D_MASK00_RF00C);
        g2dlite_cwrite(dev, 0, G2D_RF00C, val);

        /*G2D_RF010*/
        val = reg_value(a_addr_h, 0, G2D_SHIFT00_RF010, G2D_MASK00_RF010);
        g2dlite_cwrite(dev, 0, G2D_RF010, val);

        /*G2D_RF02C*/
        val = reg_value(a_stride - 1, 0, G2D_SHIFT00_RF02C, G2D_MASK00_RF02C);
        g2dlite_cwrite(dev, 0, G2D_RF02C, val);

        /*AP_TRIGGLE*/
        // g2dlite_cwrite(base, AP_SDW_CTRL, 1)
    }

    /*G2D_RE220*/
    val = g2dlite_read(base, G2D_RE220);
    val = reg_value(bgcfg->g_alpha, val, G2D_SHIFT08_RE220, G2D_MASK08_RE220);
    val = reg_value(alpha_select, val, G2D_SHIFT02_RE220, G2D_MASK02_RE220);
    val = reg_value(1, val, G2D_SHIFT01_RE220, G2D_MASK01_RE220);
    g2dlite_write(base, G2D_RE220, val);

    /*G2D_RE224*/
    g2dlite_cwrite(dev, 0, G2D_RE224, bgcfg->color);

    return 0;
}

static int g2dlite_fill_rect(struct g2dlite_device *dev,
                             struct g2dlite_bg_cfg *bgcfg,
                             struct g2dlite_output_cfg *output)
{
    u32 val;
    u64 base = dev->reg_base;

    G2DLITE_LOG_INFO("enter\n");

    u8 w_idx = get_w_idx(output);
    g2dlite_rwdma_set(dev, 0, w_idx);
    g2dlite_close_fastcopy(dev);

    /*disable all layers*/
    for (int i = 0; i < G2DLITE_MLC_MAX; i++) {
        val = g2dlite_read(base, G2D_RE000(i));
        val = reg_value(0, val, G2D_SHIFT00_RE000, G2D_MASK00_RE000);
        g2dlite_cwrite(dev, 0, G2D_RE000(i), val);
    }

    /*disable all layer's paths*/
    for (int i = 1; i < MLC_PATH_MAX; i++) {
        g2dlite_cwrite(dev, 0, G2D_RE200(i), 0xF777F);
    }
    g2dlite_cwrite(dev, 0, G2D_RE200(0), 0x07770);

    g2dlite_bg_set(dev, bgcfg, output);

    g2dlite_wpipe_set(dev, 0, output);
    G2DLITE_LOG_INFO("leave\n");

    return 0;
}

static int g2dlite_fastcopy(struct g2dlite_device *dev, struct fcopy_t *in,
                            struct fcopy_t *out)
{
    u32 val, i_addr_l, i_addr_h, o_addr_l, o_addr_h;
    unsigned long base = dev->reg_base;
    unsigned long i_addr, o_addr;

    i_addr = p2ap((paddr_t)in->addr);
    i_addr_l = get_l_addr(i_addr);
    i_addr_h = get_h_addr(i_addr);
    o_addr = p2ap((paddr_t)out->addr);
    o_addr_l = get_l_addr(o_addr);
    o_addr_h = get_h_addr(o_addr);

    g2dlite_rwdma_set(dev, 0, 0);
    /*AP_FRM_CTRL*/
    val = g2dlite_read(base, G2D_RF004);
    val = reg_value(1, val, G2D_SHIFT00_RF004,
                    G2D_MASK00_RF004);
    g2dlite_cwrite(dev, 0, G2D_RF004, val);

    /*G2D_RF008*/
    val = reg_value(in->height - 1, 0, G2D_SHIFT16_RF008,
                    G2D_MASK16_RF008);
    val = reg_value(in->width - 1, val, G2D_SHIFT00_RF008,
                    G2D_MASK00_RF008);
    g2dlite_cwrite(dev, 0, G2D_RF008, val);

    /*G2D_RF00C*/
    val = reg_value(i_addr_l, 0, G2D_SHIFT00_RF00C, G2D_MASK00_RF00C);
    g2dlite_cwrite(dev, 0, G2D_RF00C, val);

    /*G2D_RF010*/
    val = reg_value(i_addr_h, 0, G2D_SHIFT00_RF010, G2D_MASK00_RF010);
    g2dlite_cwrite(dev, 0, G2D_RF010, val);

    /*G2D_RF02C*/
    val = reg_value(in->stride - 1, 0, G2D_SHIFT00_RF02C, G2D_MASK00_RF02C);
    g2dlite_cwrite(dev, 0, G2D_RF02C, val);

    /*G2D_RE220*/ /*Just for g2dlite rework*/
    val = g2dlite_read(base, G2D_RE220);
    val = reg_value(1, val, G2D_SHIFT02_RE220, G2D_MASK02_RE220);
    val = reg_value(1, val, G2D_SHIFT01_RE220, G2D_MASK01_RE220);
    g2dlite_cwrite(dev, 0, G2D_RE220, val);

    /*Config W-pipe*/
    val = reg_value(out->height - 1, 0, G2D_SHIFT16_RA00C,
                    G2D_MASK16_RA00C);
    val = reg_value(out->width - 1, val, G2D_SHIFT00_RA00C,
                    G2D_MASK00_RA00C);
    g2dlite_cwrite(dev, 0, G2D_RA00C, val);

    g2dlite_cwrite(dev, 0, G2D_RH010, o_addr_l);
    g2dlite_cwrite(dev, 0, G2D_RH014, o_addr_h);
    g2dlite_cwrite(dev, 0, G2D_RH028, out->stride - 1);

    G2DLITE_LOG_INFO("leave\n");

    return 0;
}

static int g2dlite_update(struct g2dlite_device *dev, struct g2dlite_input *ins)
{
    G2DLITE_LOG_INFO("enter\n");
    if (!dev || !ins) {
        G2DLITE_LOG_ERR("dev or input isn't inited.[dev:%p, ins:%p]\n", dev,
                        ins);
        return -1;
    }

    u8 r_idx = get_r_idx(ins);
    u8 w_idx = get_w_idx(&ins->output);
    g2dlite_rwdma_set(dev, r_idx, w_idx);
    g2dlite_close_fastcopy(dev);

    if (ins->bg_layer.en)
        g2dlite_bg_set(dev, &ins->bg_layer, &ins->output);

    for (unsigned int i = 0; i < ins->layer_num; i++) {
        if (ins->layer[i].layer == G2DLITE_TYPE_SPIPE) {
            g2dlite_spipe_set(dev, 0, &ins->layer[i]);
        } else {
            g2dlite_gpipe_set(dev, 0, &ins->layer[i]);
        }
    }
    g2dlite_mlc_set(dev, 0, ins);
    g2dlite_wpipe_set(dev, 0, &ins->output);

    G2DLITE_LOG_INFO("leave\n");

    return 0;
}

static int g2dlite_clut_index_set(struct g2dlite_device *dev, char *clut_src)
{
    u32 value;
    u32 offset = 0;
    u64 base = dev->reg_base;
    u32 size = (G2DLITE_CLUT_EADDR - G2DLITE_CLUT_SADDR + 1) / 4;

    for (u32 i = 0; i < size; i++, offset += 4) {
        value = (clut_src[offset + 3] << 24) | (clut_src[offset + 2] << 16) |
                (clut_src[offset + 1] << 8) | (clut_src[offset]);
        g2dlite_write(base, G2DLITE_CLUT_SADDR, value);
    }

    return 0;
}

static int g2dlite_enable(struct g2dlite_device *dev)
{
    unsigned int val;
    unsigned long base = dev->reg_base;
    unsigned char valid_count = 0;
    cmdfile_info *cmd;
    unsigned long input_addr;

    if (dev->write_mode == G2DLITE_CMD_WRITE) {
        for (int i = 0; i < G2DLITE_CMDFILE_MAX_NUM; i++) {
            cmd = &dev->cmd_info[i];
            if (!cmd->dirty) {
                valid_count = i;
                break;
            } else {
                cmd->dirty = 0;
            }
        }
        G2DLITE_LOG_INFO("valid_count = %d\n", valid_count);
        for (int i = 0; i < valid_count; i++) {
            cmd = &dev->cmd_info[i];
            if (i < valid_count - 1) {
                input_addr = p2ap((paddr_t)dev->cmd_info[i + 1].arg);
                G2DLITE_LOG_INFO("input_addr = 0x%lx\n", input_addr);
                cmd->arg[cmd->len++] = G2D_RA010;
                cmd->arg[cmd->len++] = get_l_addr(input_addr);
                cmd->arg[cmd->len++] = G2D_RA014;
                cmd->arg[cmd->len++] = get_h_addr(input_addr);
                cmd->arg[cmd->len++] = G2D_RA018;
                cmd->len++; // reserve for len value config
            } else {
                cmd->arg[cmd->len++] = G2D_RA01C;
                cmd->arg[cmd->len++] = 0;

                G2DLITE_LOG_INFO("G2D_RA01C = %d\n",
                                 G2D_RA01C);
            }
        }

        for (int i = 0; i < valid_count - 1; i++) {
            cmd = &dev->cmd_info[i];
            cmd->arg[cmd->len - 1] = dev->cmd_info[i + 1].len / 2 - 1;
            G2DLITE_LOG_INFO("dev->cmd_info[i + 1].len / 2 - 1 = 0x%x\n",
                             dev->cmd_info[i + 1].len / 2 - 1);
        }

        for (int i = 0; i < valid_count; i++) {
            cmd = &dev->cmd_info[i];
            arch_clean_cache_range((addr_t)cmd->arg, cmd->len * 4);
        }

        input_addr = p2ap((paddr_t)dev->cmd_info[0].arg);
        g2dlite_write(base, G2D_RA010, get_l_addr(input_addr));
        g2dlite_write(base, G2D_RA014, get_h_addr(input_addr));
        g2dlite_write(base, G2D_RA018, dev->cmd_info[0].len / 2 - 1);
        val = g2dlite_read(base, G2D_RA01C);
        val = reg_value(1, val, G2D_SHIFT03_RA01C,
                        G2D_MASK03_RA01C);
        g2dlite_write(base, G2D_RA01C, val);

        for (int i = 0; i < valid_count; i++) {
            dev->cmd_info[i].len = 0;
        }
    }

    val = g2dlite_read(base, G2D_RA000);
    val = reg_value(1, val, G2D_SHIFT00_RA000, G2D_MASK00_RA000);
    g2dlite_write(base, G2D_RA000, val);

    return 0;
}

static int g2dlite_irq_handler(u32 irq, void *arg)
{
    unsigned int value;
    struct g2dlite_device *dev = (struct g2dlite_device *)arg;
    unsigned long base = dev->reg_base;

    value = g2dlite_read(base, G2D_RA024);
    g2dlite_write(base, G2D_RA024, value);

    if (value & G2D_MASK06_RA020) {
        osSemaphoreRelease(dev->done_sema);
    }

    return 1;
}

const struct g2dlite_ops ops = {
    .init = g2dlite_init,
    .reset = g2dlite_reset,
    .clut_index_set = g2dlite_clut_index_set,
    .update = g2dlite_update,
    .rotation = g2dlite_rotation,
    .fill_rect = g2dlite_fill_rect,
    .fastcopy = g2dlite_fastcopy,
    .enable = g2dlite_enable,
    .irq_handler = g2dlite_irq_handler,
};
