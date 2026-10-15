/*
* disp_ctrl.c
*
* Copyright (c) 2019 Semidrive Semiconductor.
* All rights reserved.
*
* Description:
*
* Revision History:
* -----------------
* 011, 06/28/2019 BI create this file
*/

#include <dc_reg.h>
#include <disp_ctrl.h>
#include <disp_data_type.h>
#include <disp_dc.h>
#include <disp_drv_log.h>
#include <disp_reg_rw.h>
#include <disp_config.h>
#include <stdlib.h>
#include <disp_errcode.h>

static const uint32_t csc_param[6][15] = {
    {/*bt601 YCbCr to RGB(0-255)*/
        0x094F, 0x0000, 0x0CC4, 0x094F,
        0x3CE0, 0x397F, 0x094F, 0x1024,
        0x0000, 0x0000, 0x0000, 0x0000,
        0x0040, 0x0200, 0x0200
    },
    {/*basic YUV to RGB*/
        0x0800, 0x0000, 0x091E, 0x0800,
        0x7CD8, 0x7B5B, 0x0800, 0x1041,
        0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000
    },

    {/*bt709 YCbCr to RGB(16-235)*/
        0x0800, 0x0000, 0x0C51, 0x0800,
        0x3E8A, 0x3C54, 0x0800, 0x0E87,
        0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0200, 0x0200
    },
    {/*RGB to YCbCr*/
        0x0000
    },
    {/*RGB to YUV*/
        0x0000
    },
    {//*RGB to BGR
        0x0000, 0x0000, 0x0800, 0x0000,
        0x0800, 0x0000, 0x0800, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000
     }
};

static unsigned int dc_triggle_type[MAX_DISP_NUM] = {DC_NO_UPDATE};

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

static float m_abs(float x)
{
    if(x < 0) x = -x;

    return x;
}

static float m_sin(float x)
{
     const float B = 1.2732395447;
     const float C = -0.4052847346;
     const float P = 0.2310792853;//0.225;
     float y = B * x + C * x * m_abs(x);

     y = P * (y * m_abs(y) - y) + y;

     return y;
 }


static float m_cos(float x)
{
    const float Q = 1.5707963268;
    const float PI =3.1415926536;

    x += Q;

    if(x > PI)
        x -= 2 * PI;

    return m_sin(x);
}

static void timing_to_tcon_timing(struct display_timing *timing,
        struct tcon_timing *t_timing)
{
    t_timing->hact = timing->hactive;
    t_timing->hsync = timing->hsync_len;
    t_timing->hsbp = timing->hsync_len + timing->hback_porch;
    t_timing->htol = timing->hactive + timing->hfront_porch
            + timing->hsync_len + timing->hback_porch;

    t_timing->vact = timing->vactive;
    t_timing->vsync = timing->vsync_len;
    t_timing->vsbp = timing->vsync_len + timing->vback_porch;
    t_timing->vtol = timing->vactive + timing->vfront_porch
            + timing->vsync_len + timing->vback_porch;
}

static void dc_sp_sdw_triggle(struct dc_device *dev)
{
    unsigned long base = dev->base;

    disp_write(base, DC_RDF00(SP_NORMAL), 1);
}

static void dc_gp_sdw_triggle(struct dc_device *dev)
{
    unsigned long base = dev->base;

    disp_write(base, DC_RCF00, 1);
}

static unsigned int dc_tcon_check_status(struct dc_device *dev)
{
    unsigned int val;
    unsigned long base = dev->base;

    val = disp_read(base, DC_RG010);

    return val & DC_MASK00_RG010;
}

static void dc_write_clear_flag(struct dc_device *dev)
{
    volatile unsigned long dc1_base = 0x0;
    volatile unsigned long base = 0x0;

#define INIT_FLAG ((1 << 20) | (1 << 10) | (1 << 0))
#define DP_NUM 3

    if (dev->dp_res != DP_RES_NC) {
        dc1_base = dev->base - dev->dc_idx * 0x10000;

        switch (dev->dp_res) {
            case DP_RES_DP1:
            case DP_RES_DP2:
            case DP_RES_DP3:
                base = dc1_base - (DP_NUM - dev->dp_res) * 0x10000;
                break;
            case DP_RES_DP1_DP2:
                base = dc1_base - (DP_NUM - DP_RES_DP1) * 0x10000;
                break;
            default:
                break;
        }
    }
    else
        base = dev->base;

    disp_write(base, DC_RE230, INIT_FLAG);
}

static void dc_force_update(struct dc_device *dev)
{
    static bool first_triggle[MAX_DISP_NUM] = {false};
    int id = dev->dc_idx;
    unsigned long base = dev->base;

    if (first_triggle[id])
        return;

    if (dc_tcon_check_status(dev))
        return;

    first_triggle[id] = true;
    disp_write(base, DC_RA008, 1);

    //mdelay(1000);
    //dc_write_clear_flag(dev);
}
static void dc_triggle(struct dc_device *dev)
{
    unsigned val = 0;
    int id = dev->dc_idx;
    unsigned long base = dev->base;

    if (dc_triggle_type[id] & DC_SP_UPDATE)
        dc_sp_sdw_triggle(dev);

    if (dc_triggle_type[id] & DC_GP_UPDATE)
        dc_gp_sdw_triggle(dev);

    if (dc_triggle_type[id] & (DC_MLC_UPDATE |
                           DC_CSC_UPDATE |
                           DC_GAMMA_UPDATE |
                           DC_DITHER_UPDATE))
        val = reg_value(1, val, DC_SHIFT01_RA004, DC_MASK01_RA004);

    if (dc_triggle_type[id] & DC_CRC_UPDATE)
        val = reg_value(1, val, DC_SHIFT03_RA004, DC_MASK03_RA004);

    if (dc_triggle_type[id] & DC_TCON_UPDATE)
        val = reg_value(1, val, DC_SHIFT02_RA004, DC_MASK02_RA004);

    disp_write(base, DC_RA004, val);

    val = reg_value(1, val, DC_SHIFT00_RA004, DC_MASK00_RA004);
    disp_write(base, DC_RA004, val);

    dc_triggle_type[id] = DC_NO_UPDATE;

    dc_force_update(dev);
}
static unsigned int dc_check_triggle_status(struct dc_device *dev)
{
    unsigned val = 0;
    unsigned long base = dev->base;

    val = disp_read(base, DC_RA004);
    return val & 0x1;
}

static void dc_ms_mode(struct dc_device *dev)
{
    unsigned long base = dev->base;
    int val;

    val = disp_read(base, DC_RA000);
    val = reg_value(dev->ms_info.ms_mode, val, DC_SHIFT01_RA000, DC_MASK01_RA000);
    // val = reg_value(1, val, DC_SHIFT03_RA000, DC_MASK03_RA000);
    // val = reg_value(1, val, DC_SHIFT02_RA000, DC_MASK02_RA000);
    disp_write(base, DC_RA000, val);
}

static void dc_irq_disable(struct dc_device *dev)
{
    unsigned long base = dev->base;

    DISPDBG("DC INT && SF_INT disable\n");
    disp_write(base, DC_RA020, DC_INIT_DEF_MASK);
    disp_write(base, DC_RA120, DC_INIT_DEF_MASK);
}

static void dc_rdma_init(struct dc_device *dev)
{
    const unsigned short wml_cfg[]       = {92, 46, 26, 92};
    const unsigned short d_depth_cfg[]   = {12, 5, 3, 12};
    const unsigned short c_depth_cfg[]   = {6, 3, 1, 6};
    const unsigned char sche_cfg[]       = {0x04, 0x01, 0x01, 0x04};
    const unsigned char p1_cfg[]         = {0x30, 0x20, 0x20, 0x30};
    const unsigned char p0_cfg[]         = {0x10, 0x08, 0x08, 0x10};
    const unsigned char burst_mode_cfg[] = {0x00, 0x00, 0x00, 0x00};
    const unsigned char burst_len_cfg[]  = {0x05, 0x05, 0x05, 0x05};
    const unsigned char wml_req_inv[]    = {0xa0, 0xa0, 0xa0, 0xa0};
    const unsigned char wml_down[]       = {1, 1, 1, 1};
    const unsigned char wml_up[]         = {6, 6, 6, 6};

    unsigned int val;
    unsigned long base = dev->base;

    for (int i = 0; i < RDMA_CHN_COUNT; i++) {
        disp_write(base, DC_RB000(i), wml_cfg[i]);
        disp_write(base, DC_RB400(i), wml_cfg[i]);
    }

    for (int i = 0; i < RDMA_CHN_COUNT; i++) {
        disp_write(base, DC_RB004(i), d_depth_cfg[i]);
        disp_write(base, DC_RB404(i), d_depth_cfg[i]);
    }

    for (int i = 0; i < RDMA_CHN_COUNT; i++) {
        disp_write(base, DC_RB008(i), c_depth_cfg[i]);
        disp_write(base, DC_RB408(i), c_depth_cfg[i]);
    }

    for (int i = 0; i < RDMA_CHN_COUNT; i++) {
        val = reg_value(sche_cfg[i], 0,
            DC_SHIFT16_RB00C,DC_MASK16_RB00C);
        val = reg_value(p1_cfg[i], val,
            DC_SHIFT08_RB00C, DC_MASK08_RB00C);
        val = reg_value(p0_cfg[i], val,
            DC_SHIFT00_RB00C, DC_MASK00_RB00C);
        disp_write(base, DC_RB00C(i), val);
        disp_write(base, DC_RB40C(i), val);
    }

    for (int i = 0; i < RDMA_CHN_COUNT; i++) {
        val = reg_value(burst_mode_cfg[i], 0,
            DC_SHIFT03_RB010, DC_MASK03_RB010);
        val = reg_value(burst_len_cfg[i], val,
            DC_SHIFT00_RB010, DC_MASK00_RB010);
        disp_write(base, DC_RB010(i), val);
        disp_write(base, DC_RB410(i), val);
    }

    for (int i = 0; i < RDMA_CHN_COUNT; i++) {
        val = reg_value(wml_req_inv[i], 0,
            DC_SHIFT16_RB01C, DC_MASK16_RB01C);
        val = reg_value(wml_down[i], val,
            DC_SHIFT04_RB01C, DC_MASK04_RB01C);
        val = reg_value(wml_up[i], val,
            DC_SHIFT00_RB01C, DC_MASK00_RB01C);
        disp_write(base, DC_RB01C(i), val);
        disp_write(base, DC_RB41C(i), val);
    }

    disp_write(base, DC_RB220, RDMA_INT_DEF_MASK);
    disp_write(base, DC_RB620, RDMA_INT_DEF_MASK);

    val = reg_value(1, 0, DC_SHIFT01_RB100, DC_MASK01_RB100);
    val = reg_value(1, val, DC_SHIFT00_RB100, DC_MASK00_RB100);
    disp_write(base, DC_RB100, val);
    disp_write(base, DC_RB500, val);
}

/*NOTE: Timing regs value = real vaule minus 1*/
static void dc_set_tcon_timing(struct dc_device *dev,
        struct display_timing *timing)
{
    unsigned int val;
    unsigned long base = dev->base;
    unsigned int kick_l, kick_m, kick_r;
    unsigned int kick_x, kick_y;
    struct tcon_timing t_timing;

    if (!dev->ms_info.ms_en) {
        timing_to_tcon_timing(timing, &t_timing);
    } else {
        struct display_timing d_timing;

        d_timing.hactive = timing->hactive / 2;
        d_timing.hfront_porch = timing->hfront_porch / 2;
        d_timing.hback_porch = timing->hback_porch / 2;
        d_timing.hsync_len = timing->hsync_len / 2;

        d_timing.vactive = timing->vactive;
        d_timing.vfront_porch = timing->vfront_porch;
        d_timing.vback_porch = timing->vback_porch;
        d_timing.vsync_len = timing->vsync_len;

        timing_to_tcon_timing(&d_timing, &t_timing);
    }

    /* Horizontal active size && total horizontal size in pixels */
    val = reg_value(t_timing.hact - 1, 0, DC_SHIFT16_RG000, DC_MASK16_RG000);
    val = reg_value(t_timing.htol - 1, val, DC_SHIFT00_RG000, DC_MASK00_RG000);
    disp_write(base, DC_RG000, val);

    /*Horizontal back porch size && wdith of hsync pulse in pixels.*/
    val = reg_value(t_timing.hsbp - 1, 0, DC_SHIFT16_RG004, DC_MASK16_RG004);
    val = reg_value(t_timing.hsync - 1, val, DC_SHIFT00_RG004, DC_MASK00_RG004);
    disp_write(base, DC_RG004, val);

    /*Vertical active size && total vertical size in lines.*/
    val = reg_value(t_timing.vact - 1, 0, DC_SHIFT16_RG008, DC_MASK16_RG008);
    val = reg_value(t_timing.vtol - 1, val, DC_SHIFT00_RG008, DC_MASK00_RG008);
    disp_write(base, DC_RG008, val);

    /*Vertical back porch size && width of vsync pulse in lines.*/
    val = reg_value(t_timing.vsbp - 1, 0, DC_SHIFT16_RG00C, DC_MASK16_RG00C);
    val = reg_value(t_timing.vsync - 1, val, DC_SHIFT00_RG00C, DC_MASK00_RG00C);
    disp_write(base, DC_RG00C, val);

    /*TCON TRCL*/
    val = reg_value(0, 0, DC_SHIFT06_RG010, DC_MASK06_RG010);
    val = reg_value(1, val, DC_SHIFT05_RG010, DC_MASK05_RG010);
    val = reg_value(timing->dsp_clk_pol, val,
            DC_SHIFT04_RG010, DC_MASK04_RG010);
    val = reg_value(timing->de_pol, val,
            DC_SHIFT03_RG010, DC_MASK03_RG010);
    val = reg_value(timing->vsync_pol, val,
            DC_SHIFT02_RG010, DC_MASK02_RG010);
    val = reg_value(timing->hsync_pol, val,
            DC_SHIFT01_RG010, DC_MASK01_RG010);
    disp_write(base, DC_RG010, val);

    //unsigned int vfp = t_timing.vtol - t_timing.vact - t_timing.vsbp;
    //unsigned int vbp = t_timing.vsbp - t_timing.vsync;
    //kick[0] < vtol - vsbp,so we set this kick = vact + vfp/2
    kick_l = t_timing.vact + 1;
    kick_m = kick_l + 1;
    kick_r = t_timing.vtol - 3; // must near vtol,because autoflush using this to calc.
    if (kick_m >= kick_r)
        kick_m = kick_r - 1;

    for (int i = 0; i < KICK_LAYER_COUNT; i++) {
        switch (i) {
            case 0:
                kick_y = kick_l; break;
            case 1:
            case 2:
            case 3:
            case 6:
                kick_y = kick_r; break;
            default:
                kick_y = kick_m; break;
        }
        kick_x = 20 + i * 20;

        if (kick_x > t_timing.htol - 1) kick_x = t_timing.htol - 1;

        val = reg_value(kick_y, 0,
            DC_SHIFT16_RG020, DC_MASK16_RG020);
        val = reg_value(kick_x, val,
            DC_SHIFT00_RG020, DC_MASK00_RG020);
        disp_write(base, DC_RG020(i), val);

        disp_write(base, DC_RG024(i), 1);

        if (i == 1)
            dev->aflu_time = (t_timing.htol * (t_timing.vtol - kick_y + 1) - kick_x) * dev->ratio;
    }

    dc_triggle_type[dev->dc_idx] |= DC_TCON_UPDATE;

    val = (timing->vsync_pol << DC_SHIFT09_RJ000)
            | (timing->hsync_pol << DC_SHIFT08_RJ000)
            | (timing->de_pol << DC_SHIFT07_RJ000);
    disp_write(base, DC_RJ000, val);
}

static void dc_tcon_init(struct dc_device *dev, struct sdm_panel *panel)
{
    //int delay;
    unsigned int val;
    unsigned long base = dev->base;
    struct display_timing *timing = panel->display_timing;
    dc_set_tcon_timing(dev, timing);

#if 0
    if ((panel->if_type == IF_TYPE_DSI) &&
        ((panel->mipi->burst_mode == 0) || (panel->mipi->burst_mode == 1))) {
        disp_write(base, DC_RG400, 0x3);

        disp_write(base, DC_RG410(0), 0x1);
        disp_write(base, DC_RG410(1), 0x5);
        disp_write(base, DC_RG410(2), 0x5);
        disp_write(base, DC_RG410(3), 0x9);

        delay = panel->mipi->delay_time;
        val = delay | ((delay + panel->display_timing->hsync_len) << 16);
        disp_write(base, DC_RG430(0), val);

        delay = 1;
        val = delay | ((delay + panel->display_timing->vsync_len) << 16);
        disp_write(base, DC_RG430(1), val);

        delay = panel->display_timing->vsync_len + panel->display_timing->vback_porch - 1;
        val =  delay | ((delay + panel->display_timing->vactive) << 16);
        disp_write(base, DC_RG430(2), val);

        delay = panel->display_timing->hsync_len + panel->display_timing->hback_porch - 1;
        val =  delay | ((delay + panel->display_timing->hactive) << 16);
        disp_write(base, DC_RG430(3), val);
    }
#endif
    /*TCON Trigger*/
    val = disp_read(base, DC_RA004);
    val =  reg_value(1, val, DC_SHIFT02_RA004, DC_MASK02_RA004);
    disp_write(base, DC_RA004, val);

}

static void dc_mlc_init(struct dc_device *dev, struct sdm_panel *panel)
{
    unsigned int val;
    unsigned long ratio, hblank, aflu, aflu_h, hactive;
    unsigned long base = dev->base;
    const unsigned char prot_val[]  = {0x07, 0x07, 0x07, 0x07};
    const unsigned char alpha_bld[] = {0xF, 0xF, 0xF, 0xF, 0xF};
    const unsigned char layer_out[] = {0xF, 0xF, 0xF, 0xF, 0xF};

    for (int i = 0; i < MLC_LAYER_COUNT; i++) {
        val = reg_value(prot_val[i], 0,
            DC_SHIFT08_RE000, DC_MASK08_RE000);
        val = reg_value(1, val,
            DC_SHIFT02_RE000,
            DC_SHIFT02_RE000);
        disp_write(base, DC_RE000(i), val);
        disp_write(base, DC_RF000(i), val);

        disp_write(base, DC_RE018(i), 0xFF);
        disp_write(base, DC_RF018(i), 0xFF);
    }

    for (int i = 0; i < MLC_PATH_COUNT; i++) {
        val = reg_value(alpha_bld[i], 0,
            DC_SHIFT16_RE200, DC_MASK16_RE200);
        val = reg_value(layer_out[i], val,
            DC_SHIFT00_RE200, DC_MASK00_RE200);
        disp_write(base, DC_RE200(i), val);
        disp_write(base, DC_RF200(i), val);
    }

    /* Setting this for using dp+dc with autoflush */
    ratio = dev->ratio * 4096;
    if (!dev->ms_info.ms_en) {
        hblank = panel->display_timing->hback_porch + panel->display_timing->hfront_porch
            + panel->display_timing->hsync_len;
        hactive = panel->display_timing->hactive;
    } else {
        hblank = (panel->display_timing->hback_porch + panel->display_timing->hfront_porch
            + panel->display_timing->hsync_len) / 2;
        hactive = panel->display_timing->hactive / 2;
    }

    aflu_h = hblank * ratio;
    aflu = dev->aflu_time - 1280;

    if (dev->dc_idx == 4) {
        disp_write(base, DC_RE000(2), 0x20002);
    }
    if ((dev->dc_idx != 4) && (dev->mlc_select & 0b01)) {
        val = disp_read(base, DC_RE000(2));
        val = reg_value(1, val, DC_SHIFT04_RE000, DC_MASK04_RE000);
        val = reg_value(1, val, DC_SHIFT00_RE000, DC_MASK00_RE000);
        disp_write(base, DC_RE000(2), val);

        disp_write(base, DC_RE02C(2), aflu);

        val = disp_read(base, DC_RE200(3));
        val = reg_value(1, val, DC_SHIFT00_RE200, DC_MASK00_RE200);
        disp_write(base, DC_RE200(3), val);

        val = disp_read(base, DC_RE200(1));
        val = reg_value(3, val, DC_SHIFT16_RE200, DC_MASK16_RE200);
        disp_write(base, DC_RE200(1), val);

        DISPDBG("sis_num = %d\n", panel->sis_num);
        if (panel->sis_num) {
            disp_write(base, DC_RE004(2), panel->sis[0].pos_x);
            disp_write(base, DC_RE008(2), panel->sis[0].pos_y);

            val = reg_value(panel->sis[0].height - 1, 0,
                DC_SHIFT16_RE00C, DC_MASK16_RE00C);
            val = reg_value(panel->sis[0].width - 1, val,
                DC_SHIFT00_RE00C, DC_MASK00_RE00C);
            disp_write(base, DC_RE00C(2), val);
        } else {
            val = reg_value(panel->display_timing->vactive - 1, 0,
                DC_SHIFT16_RE00C, DC_MASK16_RE00C);
            val = reg_value(hactive - 1, val,
                DC_SHIFT00_RE00C, DC_MASK00_RE00C);
            disp_write(base, DC_RE00C(2), val);
        }
    }

    if ((dev->dc_idx != 4) && (dev->mlc_select & 0b10)) {
        val = disp_read(base, DC_RE000(3));
        val = reg_value(1, val, DC_SHIFT04_RE000, DC_MASK04_RE000);
        val = reg_value(1, val, DC_SHIFT00_RE000, DC_MASK00_RE000);
        disp_write(base, DC_RE000(3), val);

        disp_write(base, DC_RE02C(3), aflu);

        val = disp_read(base, DC_RE200(4));
        val = reg_value(2, val, DC_SHIFT00_RE200, DC_MASK00_RE200);
        disp_write(base, DC_RE200(4), val);

        val = disp_read(base, DC_RE200(2));
        val = reg_value(4, val, DC_SHIFT16_RE200, DC_MASK16_RE200);
        disp_write(base, DC_RE200(2), val);

        if (panel->sis_num) {
            if (dev->ms_info.ms_mode == MASTER_MODE)
                disp_write(base, DC_RE004(3), panel->sis[1].pos_x);
            else
                disp_write(base, DC_RE004(3), panel->sis[1].pos_x - panel->display_timing->hactive / 2);
            disp_write(base, DC_RE008(3), panel->sis[1].pos_y);

            val = reg_value(panel->sis[1].height - 1, 0,
                DC_SHIFT16_RE00C, DC_MASK16_RE00C);
            val = reg_value(panel->sis[1].width - 1, val,
                DC_SHIFT00_RE00C, DC_MASK00_RE00C);
            disp_write(base, DC_RE00C(3), val);
        } else {
            val = reg_value(panel->display_timing->vactive - 1, 0,
                DC_SHIFT16_RE00C, DC_MASK16_RE00C);
            val = reg_value(hactive - 1, val,
                DC_SHIFT00_RE00C, DC_MASK00_RE00C);
            disp_write(base, DC_RE00C(3), val);
        }
    }

    disp_write(base, DC_RE238, aflu_h);
    disp_write(base, DC_RE234, ratio);
    disp_write(base, DC_RF234, 0xFFFF);
}

static void dc_spipe_init(struct dc_device *dev)
{
    unsigned int val;
    unsigned long base = dev->base;

    for (unsigned char i = 0; i < SP_COUNT; i++) {
        val = disp_read(base, DC_RD200(i));
        val = reg_value(1, val, DC_SHIFT16_RD200, DC_MASK16_RD200);
        disp_write(base, DC_RD200(i), val);

        val = disp_read(base, DC_RD204(i));
        val = reg_value(1, val, DC_SHIFT16_RD204, DC_MASK16_RD204);
        disp_write(base, DC_RD204(i), val);

        val = disp_read(base, DC_RD208(i));
        val = reg_value(1, val, DC_SHIFT16_RD208, DC_MASK16_RD208);
        disp_write(base, DC_RD208(i), val);

        val = disp_read(base, DC_RD20C(i));
        val = reg_value(1, val, DC_SHIFT16_RD20C, DC_MASK16_RD20C);
        disp_write(base, DC_RD20C(i), val);
    }
}

static void dc_gpipe_init(struct dc_device *dev)
{
    unsigned int val;
    unsigned long base = dev->base;

    val = reg_value(1, 0, DC_SHIFT00_RC044, DC_MASK00_RC044);
    disp_write(base, DC_RC044, val);

    val = reg_value(1, 0,DC_SHIFT00_RC200, DC_MASK00_RC200);
    disp_write(base, DC_RC200, val);

    val = reg_value(csc_param[0][1], 0,
        DC_SHIFT16_RC204, DC_MASK16_RC204);
    val = reg_value(csc_param[0][0], val,
        DC_SHIFT00_RC204, DC_MASK00_RC204);
    disp_write(base, DC_RC204, val);

    val = reg_value(csc_param[0][3], 0,
        DC_SHIFT16_RC208, DC_MASK16_RC208);
    val = reg_value(csc_param[0][2], val,
        DC_SHIFT00_RC208, DC_MASK00_RC208);
    disp_write(base, DC_RC208, val);

    val = reg_value(csc_param[0][5], 0,
        DC_SHIFT16_RC20C, DC_MASK16_RC20C);
    val = reg_value(csc_param[0][4], val,
        DC_SHIFT00_RC20C, DC_MASK00_RC204C);
    disp_write(base, DC_RC20C, val);

    val = reg_value(csc_param[0][7], 0,
        DC_SHIFT16_RC210, DC_MASK16_RC210);
    val = reg_value(csc_param[0][6], val,
        DC_SHIFT00_RC210, DC_MASK00_RC210);
    disp_write(base, DC_RC210, val);

    val = reg_value(csc_param[0][9], 0,
        DC_SHIFT16_RC214, DC_MASK16_RC214);
    val = reg_value(csc_param[0][8], val,
        DC_SHIFT00_RC214, DC_MASK00_RC214);
    disp_write(base, DC_RC214, val);

    val = reg_value(csc_param[0][11], 0,
        DC_SHIFT16_RC218, DC_MASK16_RC218);
    val = reg_value(csc_param[0][10], val,
        DC_SHIFT00_RC218, DC_MASK00_RC218);
    disp_write(base, DC_RC218, val);

    val = reg_value(csc_param[0][13], 0,
        DC_SHIFT16_RC21C, DC_MASK16_RC21C);
    val = reg_value(csc_param[0][12], val,
        DC_SHIFT00_RC21C, DC_MASK00_RC21C);
    disp_write(base, DC_RC21C, val);

    val = reg_value(csc_param[0][14], 0,
        DC_SHIFT00_RC220, DC_MASK00_RC220);
    disp_write(base, DC_RC220, val);
}

static void dc_csc_init(struct dc_device *dev, struct sdm_panel *panel)
{
    unsigned int val, bypass, idx;
    unsigned long base = dev->base;
    struct display_timing *timing = panel->display_timing;

    if (!timing->data_map_rb_swap){
        bypass = 1;
        idx = 0;
    } else {
        bypass = 0;
        idx = 5;
    }

    val = reg_value(bypass, 0, DC_SHIFT00_RH000, DC_MASK00_RH000);
    disp_write(base, DC_RH000, val);

    val = reg_value(csc_param[idx][1], 0, DC_SHIFT16_RH004, DC_MASK16_RH004);
    val = reg_value(csc_param[idx][0], val, DC_SHIFT00_RH004, DC_MASK00_RH004);
    disp_write(base, DC_RH004, val);

    val = reg_value(csc_param[idx][3], 0, DC_SHIFT16_RH008, DC_MASK16_RH008);
    val = reg_value(csc_param[idx][2], val, DC_SHIFT00_RH008, DC_MASK00_RH008);
    disp_write(base, DC_RH008, val);

    val = reg_value(csc_param[idx][5], 0, DC_SHIFT16_RH00C, DC_MASK16_RH00C);
    val = reg_value(csc_param[idx][4], val, DC_SHIFT00_RH00C, DC_MASK00_RH00C);
    disp_write(base, DC_RH00C, val);

    val = reg_value(csc_param[idx][7], 0, DC_SHIFT16_RH010, DC_MASK16_RH010);
    val = reg_value(csc_param[idx][6], val, DC_SHIFT00_RH010, DC_MASK00_RH010);
    disp_write(base, DC_RH010, val);

    val = reg_value(csc_param[idx][9], 0, DC_SHIFT16_RH014, DC_MASK16_RH014);
    val = reg_value(csc_param[idx][8], val, DC_SHIFT00_RH014, DC_MASK00_RH014);
    disp_write(base, DC_RH014, val);

    val = reg_value(csc_param[idx][11], 0, DC_SHIFT16_RH018, DC_MASK16_RH018);
    val = reg_value(csc_param[idx][10], val, DC_SHIFT00_RH018, DC_MASK00_RH018);
    disp_write(base, DC_RH018, val);

    val = reg_value(csc_param[idx][13], 0, DC_SHIFT16_RH01C, DC_MASK16_RH01C);
    val = reg_value(csc_param[idx][12], val, DC_SHIFT00_RH01C, DC_MASK00_RH01C);
    disp_write(base, DC_RH01C, val);

    val = reg_value(csc_param[idx][14], 0, DC_SHIFT00_RH020, DC_MASK00_RH020);
    disp_write(base, DC_RH020, val);

    dc_triggle_type[dev->dc_idx] |= DC_CSC_UPDATE;
}

static void dc_gamma_init(struct dc_device *dev)
{
    unsigned int val;
    unsigned long base = dev->base;

    val = reg_value(1, 0, DC_SHIFT00_RI000, DC_MASK00_RI000);
    disp_write(base, DC_RI000, val);

    dc_triggle_type[dev->dc_idx] |= DC_GAMMA_UPDATE;
}

static void dc_dither_init(struct dc_device *dev)
{
    unsigned int val;
    unsigned long base = dev->base;

    val = reg_value(1, 0, DC_SHIFT06_RI004, DC_MASK06_RI004);
    val = reg_value(1, val, DC_SHIFT00_RI004, DC_MASK00_RI004);
    disp_write(base, DC_RI004, val);

    dc_triggle_type[dev->dc_idx] |= DC_DITHER_UPDATE;
}

static void set_tcon_ctrl_en(struct dc_device *dev)
{
    unsigned int val;
    unsigned long base = dev->base;

    val = disp_read(base, DC_RG010);
    val = reg_value(1, val, DC_SHIFT00_RG010, DC_MASK00_RG010);
    disp_write(base, DC_RG010, val);
}

static int dc_plane_frm_comp_set(struct dc_device *dev,
        struct sdm_buffer *input)
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
    unsigned long base = dev->base;

    abits = COLOR_GET_A_BITS_NUM(input->fmt);
    ybits = COLOR_GET_Y_BITS_NUM(input->fmt);
    ubits = COLOR_GET_U_BITS_NUM(input->fmt);
    vbits = COLOR_GET_V_BITS_NUM(input->fmt);
    compval = reg_value(vbits, 0, DC_SHIFT24_RC000, DC_MASK24_RC000);
    compval = reg_value(ubits, compval, DC_SHIFT16_RC000, DC_MASK16_RC000);
    compval = reg_value(ybits, compval, DC_SHIFT08_RC000, DC_MASK08_RC000);
    compval = reg_value(abits, compval, DC_SHIFT00_RC000, DC_MASK00_RC000);

    swap = COLOR_GET_SWAP(input->fmt);
    comp_swap = COLOR_GET_COMP_SWAP(input->fmt);
    is_yuv = COLOR_GET_IS_YUV(input->fmt);
    is_uvswap = COLOR_GET_IS_UV_SWAP(input->fmt);
    uv_mode = COLOR_GET_UV_MODE(input->fmt);
    data_mode = COLOR_GET_DATA_MODE(input->fmt);
    buf_fmt = COLOR_GET_BUF_FMT(input->fmt);
    if ((input->layer == PIPE_TYPE_SPIPE) && input->rle.en)
        data_mode = RLE_COMPR_MODE;

    frmc_val = reg_value(swap, 0, DC_SHIFT16_RC004, DC_MASK16_RC004);
    frmc_val = reg_value(comp_swap, frmc_val, DC_SHIFT12_RC004, DC_MASK12_RC004);
    frmc_val = reg_value(is_yuv, frmc_val, DC_SHIFT07_RC004, DC_MASK07_RC004);
    frmc_val = reg_value(is_uvswap, frmc_val, DC_SHIFT06_RC004, DC_MASK06_RC004);
    frmc_val = reg_value(uv_mode, frmc_val, DC_SHIFT04_RC004, DC_MASK04_RC004);
    frmc_val = reg_value(data_mode, frmc_val, DC_SHIFT02_RC004, DC_MASK02_RC004);
    frmc_val = reg_value(buf_fmt, frmc_val, DC_SHIFT00_RC004, DC_MASK00_RC004);

    if (input->layer == PIPE_TYPE_SPIPE) {
        disp_write(base, DC_RD000(SP_NORMAL), compval);
        disp_write(base, DC_RD004(SP_NORMAL), frmc_val);
    } else if (input->layer == PIPE_TYPE_GPIPE) {
        disp_write(base, DC_RC000, compval);
        disp_write(base, DC_RC004, frmc_val);
    } else {
        DISPERR("input layer = %d,the number is incorrect\n", input->layer);
        return -1;
    }

    return 0;
}

static int dc_safety_config(struct dc_device *dev, struct sdm_buffer *layer)
{
    unsigned int val, compval, frmc_val;
    unsigned char comp_swap, is_yuv, is_uvswap;
    unsigned char uv_mode, data_mode, buf_fmt;
    unsigned char abits, ybits, ubits, vbits;
    unsigned int input_fmt = layer->fmt;
    unsigned int src_size;
    unsigned long base = dev->base;

    abits = COLOR_GET_A_BITS_NUM(input_fmt);
    ybits = COLOR_GET_Y_BITS_NUM(input_fmt);
    ubits = COLOR_GET_U_BITS_NUM(input_fmt);
    vbits = COLOR_GET_V_BITS_NUM(input_fmt);
    compval = reg_value(vbits, 0, DC_SHIFT24_RC000, DC_MASK24_RC000);
    compval = reg_value(ubits, compval, DC_SHIFT16_RC000, DC_MASK16_RC000);
    compval = reg_value(ybits, compval, DC_SHIFT08_RC000, DC_MASK08_RC000);
    compval = reg_value(abits, compval, DC_SHIFT00_RC000, DC_MASK00_RC000);

    comp_swap = COLOR_GET_COMP_SWAP(input_fmt);
    is_yuv = COLOR_GET_IS_YUV(input_fmt);
    is_uvswap = COLOR_GET_IS_UV_SWAP(input_fmt);
    uv_mode = COLOR_GET_UV_MODE(input_fmt);
    data_mode = COLOR_GET_DATA_MODE(input_fmt);
    buf_fmt = COLOR_GET_BUF_FMT(input_fmt);
    frmc_val = reg_value(comp_swap, 0, DC_SHIFT12_RC004, DC_MASK12_RC004);
    frmc_val = reg_value(is_yuv, frmc_val, DC_SHIFT07_RC004, DC_MASK07_RC004);
    frmc_val = reg_value(is_uvswap, frmc_val, DC_SHIFT06_RC004, DC_MASK06_RC004);
    frmc_val = reg_value(uv_mode, frmc_val, DC_SHIFT04_RC004, DC_MASK04_RC004);
    frmc_val = reg_value(data_mode, frmc_val, DC_SHIFT02_RC004, DC_MASK02_RC004);
    frmc_val = reg_value(buf_fmt, frmc_val, DC_SHIFT00_RC004, DC_MASK00_RC004);

    disp_write(base, DC_RD000(SP_SAFETY), compval);
    disp_write(base, DC_RD004(SP_SAFETY), frmc_val);


    /*pipe related*/
    src_size = reg_value(layer->src.w - 1, 0,
            DC_SHIFT00_RC008, DC_MASK00_RC008);
    src_size = reg_value(layer->src.h - 1, src_size,
            DC_SHIFT16_RC008, DC_MASK16_RC008);
    disp_write(base, DC_RD008(SP_SAFETY), src_size);
    disp_write(base, DC_RD00C(SP_SAFETY),  get_l_addr(p2ap(layer->addr[0])));
    disp_write(base, DC_RD010(SP_SAFETY), get_h_addr(p2ap(layer->addr[0])));
    disp_write(base, DC_RD02C(SP_SAFETY), layer->src_stride[0] - 1);
    disp_write(base, DC_RD040(SP_SAFETY), 0);

    /*S_SP Triggle*/
    disp_write(base, DC_RDF00(SP_SAFETY), 0x1);

    /*mlc related*/

    disp_write(base, DC_RF230, 0x3ff);

    disp_write(base, DC_RF004(MLC_LAYER_SP), layer->dst.x);
    disp_write(base, DC_RF008(MLC_LAYER_SP), layer->dst.y);
    disp_write(base, DC_RF00C(MLC_LAYER_SP), src_size);

    disp_write(base, DC_RF018(MLC_LAYER_SP), 0xFF);

    val = disp_read(base, DC_RF000(MLC_LAYER_SP));
    val = reg_value(1, val, DC_SHIFT02_RE000, DC_MASK02_RE000);
    val = reg_value(1, val, DC_SHIFT00_RE000, DC_MASK00_RE000);
    disp_write(base, DC_RF000(MLC_LAYER_SP), val);

    /*set z-order*/
    disp_write(base, DC_RF200(MLC_PATH_SP), 0x10001);

    /*SF_FLC_CTRL Triggle*/
    val = reg_value(1, 0, DC_SHIFT02_RA004, DC_MASK02_RA004);
    val = reg_value(1, val, DC_SHIFT01_RA004, DC_MASK01_RA004);
    val = reg_value(1, val, DC_SHIFT00_RA004, DC_MASK00_RA004);
    disp_write(base, DC_RA100, val);

    return 0;
}

static void dc_safety_exit(struct dc_device *dev)
{
    unsigned long base = dev->base;
    unsigned int val = 0;

    val = disp_read(base, DC_RA000);
    val = reg_value(1, val, DC_SHIFT00_RA000, DC_MASK00_RA000);
    disp_write(base, DC_RA000, val);

    val = reg_value(1, val, DC_SHIFT01_RA004, DC_MASK01_RA004);
    val = reg_value(1, val, DC_SHIFT00_RA004, DC_MASK00_RA004);
    disp_write(base, DC_RA004, val);
}

static int dc_init(struct dc_device *dev, struct sdm_panel *panel)
{
    if (!panel) {
        DISPERR("panel is [panel:%p]\n", panel);
        return -1;
    }

    DISPDBG("init dc%d start.\n", dev->dc_idx);

    if(dc_tcon_check_status(dev)) {
        DISPERR("tcon is run, dc%d init skip.\n", dev->dc_idx);
        return 0;
    }

    dc_ms_mode(dev);
    dc_irq_disable(dev);
    dc_rdma_init(dev);
    dc_tcon_init(dev, panel);
    dc_mlc_init(dev, panel);
    dc_spipe_init(dev);
    dc_gpipe_init(dev);
    dc_csc_init(dev, panel);
    dc_gamma_init(dev);
    dc_dither_init(dev);

    DISPDBG("init dc%d done.\n", dev->dc_idx);

    return 0;
}

static int dc_reset(struct dc_device *dev)
{
    unsigned int val;
    unsigned long base = dev->base;

    val = disp_read(base, DC_RA000);
    val = reg_value(1, val, DC_SHIFT31_RA000, DC_MASK31_RA000);
    disp_write(base, DC_RA000, val);

    return 0;
}

static bool fmt_is_spipe_support(unsigned int fmt)
{
    switch (fmt) {
    case COLOR_RGB565:
    case COLOR_BGR565:
    case COLOR_ARGB4444:
    case COLOR_BGRA4444:
    case COLOR_ARGB1555:
    case COLOR_BGRA1555:
    case COLOR_RGB666:
    case COLOR_BGR666:
    case COLOR_RGB888:
    case COLOR_BGR888:
    case COLOR_ARGB8888:
    case COLOR_BGRA8888:
    case COLOR_ARGB2101010:
    case COLOR_BGRA2101010:
        return true;
    default:
        return false;
    }
}

static void dc_clear_layers(struct dc_device *dev, u8 mask, u8 z_order)
{
    int val;
    u8 zorder;
    unsigned long base = dev->base;

    if (dev->mlc_select & 0b10) zorder = z_order + 3;
    else zorder = z_order + 2;

    switch (mask) {
        case CLEAR_SPIPE_LAYER:
            /*Disable Layer*/
            val = disp_read(base, DC_RE000(MLC_LAYER_SP));
            val = reg_value(0, val, DC_SHIFT00_RE000, DC_MASK00_RE000);
            disp_write(base, DC_RE000(MLC_LAYER_SP), val);

            /*Disable Path*/
            val = disp_read(base, DC_RE200(MLC_PATH_SP));
            val = reg_value(zorder, val, DC_SHIFT00_RE200, DC_MASK00_RE200);
            disp_write(base, DC_RE200(MLC_PATH_SP), val);

            val = disp_read(base, DC_RE200(zorder));
            val = reg_value(0x1, val, DC_SHIFT16_RE200, DC_MASK16_RE200);
            disp_write(base, DC_RE200(zorder), val);

            dc_triggle_type[dev->dc_idx] |= DC_SP_UPDATE;
            dc_triggle_type[dev->dc_idx] |= DC_MLC_UPDATE;

            break;
        case CLEAR_GPIPE_LAYER:
            /*Disable Layer*/
            val = disp_read(base, DC_RE000(MLC_LAYER_GP));
            val = reg_value(0, val, DC_SHIFT00_RE000, DC_MASK00_RE000);
            disp_write(base, DC_RE000(MLC_LAYER_GP), val);

            /*Disable Path*/
            val = disp_read(base, DC_RE200(MLC_PATH_GP));
            val = reg_value(zorder, val, DC_SHIFT00_RE200, DC_MASK00_RE200);
            disp_write(base, DC_RE200(MLC_PATH_GP), val);

            val = disp_read(base, DC_RE200(zorder));
            val = reg_value(0x2, val, DC_SHIFT16_RE200, DC_MASK16_RE200);
            disp_write(base, DC_RE200(zorder), val);

            dc_triggle_type[dev->dc_idx] |= DC_GP_UPDATE;
            dc_triggle_type[dev->dc_idx] |= DC_MLC_UPDATE;

            break;
        case CLEAR_ALL_LAYERS:
            for (int i = 0; i < MLC_LAYER_DP0; i++) {
                val = disp_read(base, DC_RE000(i));
                val = reg_value(0, val, DC_SHIFT00_RE000, DC_MASK00_RE000);
                disp_write(base, DC_RE000(i), val);
            }

            if (dev->mlc_select & 0b10) {
                val = disp_read(base, DC_RE200(1));
                val = reg_value(0xF, val, DC_SHIFT00_RE200, DC_MASK00_RE200);
                disp_write(base, DC_RE200(1), val);

                val = disp_read(base, DC_RE200(2));
                val = reg_value(0xF, val, DC_SHIFT00_RE200, DC_MASK00_RE200);
                disp_write(base, DC_RE200(2), val);

                val = disp_read(base, DC_RE200(3));
                val = reg_value(0xF, val, DC_SHIFT16_RE200, DC_MASK16_RE200);
                disp_write(base, DC_RE200(3), val);

                val = disp_read(base, DC_RE200(4));
                val = reg_value(0xF, val, DC_SHIFT16_RE200, DC_MASK16_RE200);
                disp_write(base, DC_RE200(4), val);
            } else {
                val = disp_read(base, DC_RE200(1));
                val = reg_value(0xF, val, DC_SHIFT00_RE200, DC_MASK00_RE200);
                disp_write(base, DC_RE200(1), val);

                val = disp_read(base, DC_RE200(2));
                val = reg_value(0xF, val, DC_SHIFT00_RE200, DC_MASK00_RE200);
                val = reg_value(0xF, val, DC_SHIFT16_RE200, DC_MASK16_RE200);
                disp_write(base, DC_RE200(2), val);

                val = disp_read(base, DC_RE200(3));
                val = reg_value(0xF, val, DC_SHIFT16_RE200, DC_MASK16_RE200);
                disp_write(base, DC_RE200(3), val);
            }

            dc_triggle_type[dev->dc_idx] |= DC_SP_UPDATE;
            dc_triggle_type[dev->dc_idx] |= DC_GP_UPDATE;
            dc_triggle_type[dev->dc_idx] |= DC_MLC_UPDATE;

            break;
        default:
            break;
    }

}

static int dc_offset_to_addr(struct sdm_buffer *input)
{
    unsigned char uv_mode = COLOR_GET_UV_MODE(input->fmt);
    unsigned char n_planes = COLOR_GET_PLANE_COUNT(input->fmt);

    if (n_planes == 1)
        return 0;

    switch (uv_mode) {
    case UV_YUV422:
        if (n_planes == 2) {
            input->addr[0] += input->start.y * input->src_stride[0] +
                input->start.x;
            input->addr[1] += input->start.y * input->src_stride[1] +
                input->start.x;
        }
        if (n_planes == 3) {
            input->addr[0] += input->start.y * input->src_stride[0] +
                input->start.x;
            input->addr[1] += input->start.y * input->src_stride[1] +
                input->start.x / 2;
            input->addr[2] += input->start.y * input->src_stride[2] +
                input->start.x / 2;
        }
        break;
    case UV_YUV420:
        if (n_planes == 2) {
            input->addr[0] += input->start.y * input->src_stride[0] +
                input->start.x;
            input->addr[1] += input->start.y / 2 * input->src_stride[1] +
                input->start.x;
        }
        if (n_planes == 3) {
            input->addr[0] += input->start.y * input->src_stride[0] +
                input->start.x;
            input->addr[1] += input->start.y / 2 * input->src_stride[1] +
                input->start.x / 2;
            input->addr[2] += input->start.y / 2 * input->src_stride[2] +
                input->start.x / 2;
        }
        break;
    case UV_YUV440:
        if (n_planes == 2) {
            input->addr[0] += input->start.y * input->src_stride[0] +
                input->start.x;
            input->addr[1] += input->start.y * input->src_stride[1] +
                input->start.x * 2;
        }
        break;
    default:
        break;
    }

    return 0;
}

static int rebuild_layers_input(int start, int end, struct sdm_buffer *input, struct sdm_buffer *re_input)
{
    int x1, x2;

    if (!input->layer_en) {
        re_input->layer_en = 0;
        return 0;
    }

    x1 = input->dst.x;
    x2 = x1 + input->dst.w;

    re_input->layer = input->layer;
    re_input->fmt = input->fmt;
    re_input->src.x = input->src.x;
    re_input->src.y = input->src.y;
    re_input->src.w = input->src.w;
    re_input->src.h = input->src.h;
    for (int i = 0; i < 4; i++) {
        re_input->addr[i] = input->addr[i];
        re_input->src_stride[i] = input->src_stride[i];
    }
    re_input->ckey_en = input->ckey_en;
    re_input->ckey = input->ckey;
    re_input->alpha_en = input->alpha_en;
    re_input->alpha = input->alpha;
    re_input->z_order = input->z_order;
    re_input->security = input->security;

    re_input->start.y = input->start.y;
    re_input->start.h = input->start.h;

    re_input->dst.y = input->dst.y;
    re_input->dst.h = input->dst.h;

    if ((x1 >= end) || (x2 <= start)) {
        re_input->layer_en = 0;
    } else if ((start <= x1) && (x2 <= end)) {
        re_input->layer_en = 1;
        re_input->start.x = input->start.x;
        re_input->start.w = input->start.w;
        re_input->dst.x = x1 - start;
        re_input->dst.w = input->dst.w;
    } else {
        re_input->layer_en = 1;
        if (x1 < start) {
            re_input->start.x = input->start.x + (start - input->dst.x);
            re_input->start.w = x2 - start;
            re_input->dst.x = 0;
            re_input->dst.w = x2 - start;
        } else {
            re_input->start.x = input->start.x;
            re_input->start.w = end - x1;
            re_input->dst.x = x1;
            re_input->dst.w = end - x1;
        }
    }

    return 0;
}

static int dc_update_layer(struct dc_device *dev, struct sdm_buffer *input)
{
    unsigned int val;
    unsigned char nplanes;
    unsigned int src_size;
    unsigned int src_offset;
    unsigned int zorder;
    unsigned long input_addr;
    unsigned long base = dev->base;
    unsigned char is_yuv = 0;

    if (!input) {
        DISPERR("input is [, input:%p]\n", input);

        disp_set_errcode(DISP_ERRCODE_ID,
            dev->display_id, DISP_ERRCODE_INPUT_BUF, 0);
        return -1;
    }

    nplanes = COLOR_GET_PLANE_COUNT(input->fmt);
    dc_plane_frm_comp_set(dev, input);

    src_size = reg_value(input->start.w - 1, 0, DC_SHIFT00_RC008, DC_MASK00_RC008);
    src_size = reg_value(input->start.h - 1, src_size,
            DC_SHIFT16_RC008, DC_MASK16_RC008);

    if (nplanes == 1) {
        src_offset = reg_value(input->start.x, 0, DC_SHIFT00_RC040, DC_MASK00_RC040);
        src_offset = reg_value(input->start.y, src_offset, DC_SHIFT16_RC040, DC_MASK16_RC040);
    } else {
        src_offset = 0;
    }

    dc_offset_to_addr(input);

    dev->last_zorder[input->layer] = input->z_order;
    if(dev->dc_idx != 4) {
        if (dev->mlc_select & 0b10) zorder = input->z_order + 3;
        else zorder = input->z_order + 2;
    } else {
        zorder = input->z_order;
    }

    if (input->layer == PIPE_TYPE_SPIPE) {
        input_addr = p2ap(input->addr[0]);
        /*pipe related*/
        disp_write(base, DC_RD008(SP_NORMAL), src_size);
        disp_write(base, DC_RD00C(SP_NORMAL), get_l_addr(input_addr));
        disp_write(base, DC_RD010(SP_NORMAL), get_h_addr(input_addr));
        disp_write(base, DC_RD02C(SP_NORMAL), input->src_stride[0] - 1);
        disp_write(base, DC_RD040(SP_NORMAL), src_offset);
        if (!input->rle.en) {
            val = reg_value(0, 0, DC_SHIFT00_RD120, DC_MASK00_RD120);
            disp_write(base, DC_RD120(0), val);
        } else {
            val = reg_value(input->rle.data_size, 0, DC_SHIFT01_RD120, DC_MASK01_RD120);
            val = reg_value(input->rle.en, val, DC_SHIFT00_RD120, DC_MASK00_RD120);
            disp_write(base, DC_RD120(0), val);

            disp_write(base, DC_RD100(0), input->rle.y_len);
            disp_write(base, DC_RD110(0), input->rle.y_checksum);
        }

        dc_triggle_type[dev->dc_idx] |= DC_SP_UPDATE;

        /*mlc related*/
        disp_write(base, DC_RE004(MLC_LAYER_SP), input->dst.x);
        disp_write(base, DC_RE008(MLC_LAYER_SP), input->dst.y);
        disp_write(base, DC_RE00C(MLC_LAYER_SP), src_size);

        if (input->alpha_en)
            disp_write(base, DC_RE018(MLC_LAYER_SP), input->alpha);

        val = disp_read(base, DC_RE000(MLC_LAYER_SP));
        val = reg_value(input->alpha_en, val,
                DC_SHIFT02_RE000, DC_MASK02_RE000);
        val = reg_value(1, val, DC_SHIFT00_RE000, DC_MASK00_RE000);
        disp_write(base, DC_RE000(MLC_LAYER_SP), val);

        /*set z-order*/
        val = disp_read(base, DC_RE200(MLC_PATH_SP));
        val = reg_value(zorder, val, DC_SHIFT00_RE200, DC_MASK00_RE200);
        disp_write(base, DC_RE200(MLC_PATH_SP), val);

        val = disp_read(base, DC_RE200(zorder));
        val = reg_value(0x1, val, DC_SHIFT16_RE200, DC_MASK16_RE200);
        disp_write(base, DC_RE200(zorder), val);

        dc_triggle_type[dev->dc_idx] |= DC_MLC_UPDATE;
     } else if (input->layer == PIPE_TYPE_GPIPE) {
        /*pipe related*/

        input_addr = p2ap(input->addr[0]);
        disp_write(base, DC_RC008, src_size);
        disp_write(base, DC_RC00C, get_l_addr(input_addr));
        disp_write(base, DC_RC010, get_h_addr(input_addr));
        disp_write(base, DC_RC02C, input->src_stride[0] - 1);
        disp_write(base, DC_RC040, src_offset);
        if (nplanes > 1) {
            input_addr = p2ap(input->addr[1]);
            disp_write(base, DC_RC014, get_l_addr(input_addr));
            disp_write(base, DC_RC018, get_h_addr(input_addr));
            disp_write(base, DC_RC030, input->src_stride[1] - 1);
        }
        if (nplanes > 2) {
            input_addr = p2ap(input->addr[2]);
            disp_write(base, DC_RC01C, get_l_addr(input_addr));
            disp_write(base, DC_RC020, get_h_addr(input_addr));
            disp_write(base, DC_RC034, input->src_stride[2] - 1);
        }
        is_yuv = COLOR_GET_IS_YUV(input->fmt);
        val = disp_read(base, DC_RC044);
        val = reg_value(is_yuv, val,
            DC_SHIFT31_RC044, DC_MASK31_RC044);
        val = reg_value(is_yuv ? 0 : 1, val,
            DC_SHIFT00_RC044, DC_MASK00_RC044);
        disp_write(base, DC_RC044, val);

        val = disp_read(base, DC_RC200);
        val = reg_value(is_yuv ? 0 : 1, val,
            DC_SHIFT00_RC200, DC_MASK00_RC200);
        disp_write(base, DC_RC200, val);

        dc_triggle_type[dev->dc_idx] |= DC_GP_UPDATE;

        /*mlc related*/
        disp_write(base, DC_RE004(MLC_LAYER_GP), input->dst.x);
        disp_write(base, DC_RE008(MLC_LAYER_GP), input->dst.y);
        disp_write(base, DC_RE00C(MLC_LAYER_GP), src_size);

        if (input->alpha_en)
            disp_write(base, DC_RE018(MLC_LAYER_GP), input->alpha);

        val = disp_read(base, DC_RE000(MLC_LAYER_GP));
        val = reg_value(input->alpha_en, val,
            DC_SHIFT02_RE000, DC_MASK02_RE000);
        val = reg_value(1, val, DC_SHIFT00_RE000, DC_MASK00_RE000);
        disp_write(base, DC_RE000(MLC_LAYER_GP), val);

        /*set z-order*/
        val = disp_read(base, DC_RE200(MLC_PATH_GP));
        val = reg_value(zorder, val, DC_SHIFT00_RE200, DC_MASK00_RE200);
        disp_write(base, DC_RE200(MLC_PATH_GP), val);

        val = disp_read(base, DC_RE200(zorder));
        val = reg_value(0x2, val, DC_SHIFT16_RE200, DC_MASK16_RE200);
        disp_write(base, DC_RE200(zorder), val);

        dc_triggle_type[dev->dc_idx] |= DC_MLC_UPDATE;
    } else {
        DISPERR("input layer = %d,the number isn't correct\n", input->layer);

        disp_set_errcode(DISP_ERRCODE_ID,
            dev->display_id, DISP_ERRCODE_LAYER_TYPE, 0);
        return -1;
    }

    return 0;
}

static int dc_update(struct dc_device *dev, struct sdm_post_config *post)
{
    int ret;
    //int pipe_flag[PIPE_TYPE_NUM] = {0};

    if (post->n_bufs > PIPE_TYPE_NUM) {
        DISPERR("Layers number over chip's ability\n");

        disp_set_errcode(DISP_ERRCODE_ID,
                dev->display_id, DISP_ERRCODE_INPUT_NBUFS, 0);
        return -1;
    }

    /*flag layers with suit pipes*/
#if 0
    for (unsigned int i = 0; i < post->n_bufs; i++) {
        if (fmt_is_spipe_support(post->bufs[i].fmt)) {
            if (!pipe_flag[PIPE_TYPE_SPIPE]) {
                post->bufs[i].layer = PIPE_TYPE_SPIPE;
                pipe_flag[PIPE_TYPE_SPIPE] = 1;
            } else {
                post->bufs[i].layer = PIPE_TYPE_GPIPE;
                pipe_flag[PIPE_TYPE_GPIPE] = 1;
            }
        } else {
            if (!pipe_flag[PIPE_TYPE_GPIPE]) {
                post->bufs[i].layer = PIPE_TYPE_GPIPE;
                pipe_flag[PIPE_TYPE_GPIPE] = 1;
            } else {
                DISPERR("Layers fmt over chip's ability\n");
                return -1;
            }
        }
    }
#endif
    /*config layers*/
    //dc_clear_layers(dev);
    for (unsigned int i = 0; i < post->n_bufs; i++) {
        if (!dev->ms_info.ms_en) {
#if 1
            struct sdm_buffer re_buf;

            memcpy(&re_buf, &post->bufs[i], sizeof(struct sdm_buffer));
            if (!re_buf.layer_en) {
                if (re_buf.layer_dirty)
                    dc_clear_layers(dev, re_buf.layer, dev->last_zorder[re_buf.layer]);
                continue;
            }

            ret = dc_update_layer(dev, &re_buf);
#else
            if (!post->bufs[i].layer_en)
                continue;
            ret = dc_update_layer(dev, &post->bufs[i]);
#endif
            if (ret < 0) {
                DISPERR("dc update layer failed\n");
                return -1;
            }
        } else {
            struct sdm_buffer re_buf;
            memset(&re_buf, 0, sizeof(struct sdm_buffer));

            rebuild_layers_input(dev->ms_info.start, dev->ms_info.end,
                &post->bufs[i], &re_buf);

            if (!re_buf.layer_en) {
                if (re_buf.layer_dirty)
                    dc_clear_layers(dev, re_buf.layer, dev->last_zorder[re_buf.layer]);
                continue;
            }

            ret = dc_update_layer(dev, &re_buf);
            if (ret < 0) {
                DISPERR("dc update layer failed\n");
                return -1;
            }
        }
    }

    return 0;
}

static void dc_vsync_enable(struct dc_device *dev, bool enable)
{
    unsigned int val;
    unsigned long base = dev->base;

    val = disp_read(base, DC_RA020);
    val = reg_value(enable ? 0: 1, val, DC_SHIFT03_RA02X, DC_MASK03_RA02X);
    disp_write(base, DC_RA020, val);
}

static int dc_irq_handler(uint32_t irq, void *arg)
{
    unsigned int value;
    struct dc_device *dev = (struct dc_device*)arg;
    unsigned long base = dev->base;
    static int cnt = 0;
    value = disp_read(base, DC_RA024);
    disp_write(base, DC_RA024, value);
    if (value & DC_MASK03_RA02X) {
        disp_vsync_callback(dev->display_id);
    }

    if (value & DC_MASK06_RA02X) {
    }

    if (value & DC_MASK05_RA02X) {
        if (!cnt) {
            DISPERR("dc[%d]:tcon underrun\n", dev->dc_idx);
		    disp_set_errcode(DISP_ERRCODE_ID,
                dev->display_id, DISP_ERRCODE_TCON_UNDERRUN, 0);
		}
        cnt++;
        if (cnt > 20)
            cnt = 0;
    }

    return 0;
}

static int dc_csc_set(struct dc_device *dev, struct sdm_pq_params *pq)
{
    unsigned int val;
    unsigned long base = dev->base;
    float aa[3][3]       = {0};
    float bb[3]          = {0};
    float cc[3]          = {0};
    unsigned int A[3][3] = {0};
    unsigned int B[3]    = {0};
    unsigned int C[3]    = {0};

#if 1
    aa[0][0] = (0.299 + 0.701 * m_cos(pq->chroma) * pq->saturation + 0.236 * m_sin(pq->chroma) * pq->saturation) * pq->contrast;
    aa[0][1] = (0.587 - 0.587 * m_cos(pq->chroma) * pq->saturation + 0.465 * m_sin(pq->chroma) * pq->saturation) * pq->contrast;
    aa[0][2] = (0.114 - 0.114 * m_cos(pq->chroma) * pq->saturation - 0.701 * m_sin(pq->chroma) * pq->saturation) * pq->contrast;

    aa[1][0] = (0.299 - 0.299 * m_cos(pq->chroma) * pq->saturation - 0.292 * m_sin(pq->chroma) * pq->saturation) * pq->contrast;
    aa[1][1] = (0.587 + 0.413 * m_cos(pq->chroma) * pq->saturation - 0.093 * m_sin(pq->chroma) * pq->saturation) * pq->contrast;
    aa[1][2] = (0.114 - 0.114 * m_cos(pq->chroma) * pq->saturation + 0.385 * m_sin(pq->chroma) * pq->saturation) * pq->contrast;

    aa[2][0] = (0.299 - 0.298 * m_cos(pq->chroma) * pq->saturation + 0.885 * m_sin(pq->chroma) * pq->saturation) * pq->contrast;
    aa[2][1] = (0.587 - 0.587 * m_cos(pq->chroma) * pq->saturation - 0.741 * m_sin(pq->chroma) * pq->saturation) * pq->contrast;
    aa[2][2] = (0.114 + 0.885 * m_cos(pq->chroma) * pq->saturation - 0.144 * m_sin(pq->chroma) * pq->saturation) * pq->contrast;


    bb[0] = pq->luminance;
    bb[1] = pq->luminance;
    bb[2] = pq->luminance;
#endif

#if 0//test data
    aa[0][0] = 0.299;
    aa[0][1] = 0.587;
    aa[0][2] = 0.144;

    aa[1][0] = -0.172;
    aa[1][1] = -0.339;
    aa[1][2] = 0.511;

    aa[2][0] = 0.511;
    aa[2][1] = -0.428;
    aa[2][2] = -0.083;


    bb[0] = 0;
    bb[1] = 128;
    bb[2] = 128;
#endif

    //calc params
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++) {
            A[i][j] = (int)(aa[i][j] * 2048) & 0x3FFF;
            printf("[%d][%d]: aa = %f,A = 0x%x\n", i, j, aa[i][j], A[i][j]);
    }

    for (int i = 0; i < 3; i++) {
        B[i] = (int)(bb[i] * 4) & 0x3FFF;
        C[i] = (int)(cc[i] * 4) & 0x3FFF;
    }

    //reg cfg
    for (int i = 0; i < 3; i++)
        printf("B[%d]=0x%x,C[%d]=0x%x\n", i, B[i], i, C[i]);

    val = reg_value(A[0][1], 0,
        DC_SHIFT16_RH004, DC_MASK16_RH004);
    val = reg_value(A[0][0], val,
        DC_SHIFT00_RH004, DC_MASK00_RH004);
    disp_write(base, DC_RH004, val);

    val = reg_value(A[1][0], 0,
        DC_SHIFT16_RH008, DC_MASK16_RH008);
    val = reg_value(A[0][2], val,
        DC_SHIFT00_RH008, DC_MASK00_RH008);
    disp_write(base, DC_RH008, val);

    val = reg_value(A[1][2], 0,
        DC_SHIFT16_RH00C, DC_MASK16_RH00C);
    val = reg_value(A[1][1], val,
        DC_SHIFT00_RH00C, DC_MASK00_RH00C);
    disp_write(base, DC_RH00C, val);

    val = reg_value(A[2][1], 0,
        DC_SHIFT16_RH010, DC_MASK16_RH010);
    val = reg_value(A[2][0], val,
        DC_SHIFT00_RH010, DC_MASK00_RH010);
    disp_write(base, DC_RH010, val);

    val = reg_value(B[0], 0,
        DC_SHIFT16_RH014, DC_MASK16_RH014);
    val = reg_value(A[2][2], val,
        DC_SHIFT00_RH014, DC_MASK00_RH014);
    disp_write(base, DC_RH014, val);

    val = reg_value(B[2], 0,
        DC_SHIFT16_RH018, DC_MASK16_RH018);
    val = reg_value(B[1], val,
        DC_SHIFT00_RH018, DC_MASK00_RH018);
    disp_write(base, DC_RH018, val);

    val = reg_value(C[1], 0,
        DC_SHIFT16_RH01C, DC_MASK16_RH01C);
    val = reg_value(C[0], val,
        DC_SHIFT00_RH01C, DC_MASK00_RH01C);
    disp_write(base, DC_RH01C, val);

    val = reg_value(C[2], 0,
        DC_SHIFT00_RH020, DC_MASK00_RH020);
    disp_write(base, DC_RH020, val);

    val = reg_value(0, 0, DC_SHIFT00_RH000, DC_MASK00_RH000);
    disp_write(base, DC_RH000, val);

    dc_triggle_type[dev->dc_idx] |= DC_CSC_UPDATE;
    return 0;
}

static void dc_crc32_set(struct dc_device *dev, struct crc_info *info)
{
    unsigned int value;
    unsigned long base = dev->base;

    if (info->global_enable) {
        value = disp_read(base, DC_RJ000);
        value &= ~DC_MASK00_RJ000;
        value |= info->global_enable & DC_MASK00_RJ000;
        disp_write(base, DC_RJ000, value);
    } else {
        value = disp_read(base, DC_RJ000);
        value &= ~DC_MASK00_RJ000;
        disp_write(base, DC_RJ000, value);

        for (int i = 0; i < CRC_BLK_COUNT; i++) {
            info->block[i].block_use = 0;
        }
    }

    for (int i = 0; i < CRC_BLK_COUNT; i++) {
        //printf("block_use[%d] = %d\n", i, info->block[i].block_use);
        if (info->block[i].block_use) {
            value = (1 << DC_SHIFT31_RJ010) | (info->block[i].cfg_start_y << DC_SHIFT16_RJ010) |
                    (info->block[i].cfg_start_x << DC_SHIFT00_RJ010);
            //printf("ctl0 = %x\n", value);
            disp_write(base, DC_RJ010(i), value);

            value = (info->block[i].cfg_end_y << DC_SHIFT16_RJ014) | (info->block[i].cfg_end_x << DC_SHIFT00_RJ014);
            //printf("ctl1 = %x\n", value);
            disp_write(base, DC_RJ014(i), value);

            value = info->block[i].expect_crc;
            //printf("expect_crc = %x\n", value);
            disp_write(base, DC_RJ018(i), value);
        } else {
            disp_write(base, DC_RJ010(i), 0);
        }
    }
    dc_triggle_type[dev->dc_idx] |= DC_CRC_UPDATE;
}

static void dc_crc32_disable(struct dc_device *dev, struct crc_info *info)
{
    unsigned int value;
    unsigned long base = dev->base;

    if (!info->global_enable) {
        value = disp_read(base, DC_RJ000);
        value &= ~DC_MASK00_RJ000;
        disp_write(base, DC_RJ000, value);

        for (int i = 0; i < CRC_BLK_COUNT; i++) {
            info->block[i].block_use = 0;
        }
    }

    for (int i = 0; i < CRC_BLK_COUNT; i++) {
        if (!info->block[i].block_use) {
            disp_write(base, DC_RJ010(i), 0);

            disp_write(base, DC_RJ014(i), 0);

            disp_write(base, DC_RJ018(i), 0);

            value = disp_read(base, DC_RJ008);
            value &= ~(1 << (i+8));
            disp_write(base, DC_RJ008, value);
        }
    }
}

static unsigned int dc_crc32_get_result_data(struct dc_device *dev, uint8_t block_index)
{
    unsigned long base = dev->base;

    return disp_read(base, DC_RJ01C(block_index));
}

static void dc_crc32_enable_irq(struct dc_device *dev, uint8_t block_index)
{
    unsigned int value;
    unsigned long base = dev->base;

    value = disp_read(base, DC_RJ008);
    value |= (1 << (block_index+8));
    disp_write(base, DC_RJ008, value);
}

static void dc_crc32_disable_irq(struct dc_device *dev, uint8_t block_index)
{
    unsigned int value;
    unsigned long base = dev->base;

    value = disp_read(base, DC_RJ008);
    value &= ~(1 << (block_index+8));
    disp_write(base, DC_RJ008, value);

}

const struct dc_operations kunlun_dc_ops = {
    .init = dc_init,
    .reset = dc_reset,
    .update = dc_update,
    .clear_layers = dc_clear_layers,
    .triggle = dc_triggle,
    .check_triggle_status = dc_check_triggle_status,
    .vsync_enable = dc_vsync_enable,
    .enable = set_tcon_ctrl_en,
    .irq_handler = dc_irq_handler,
    .csc_set = dc_csc_set,
    .check_tcon_status = dc_tcon_check_status,
    .crc32_set = dc_crc32_set,
    .crc32_disable = dc_crc32_disable,
    .crc32_get_result = dc_crc32_get_result_data,
    .crc32_enable_irq = dc_crc32_enable_irq,
    .crc32_disable_irq = dc_crc32_disable_irq,
    .safety_config = dc_safety_config,
    .safety_exit = dc_safety_exit,
};
