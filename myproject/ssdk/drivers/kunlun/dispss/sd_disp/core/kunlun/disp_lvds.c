/*
* disp_lvds.c
*
* Copyright (c) 2019 Semidrive Semiconductor.
* All rights reserved.
*
* Description:
*
* Revision History:
* -----------------
* 011, 16/10/2019 BI create this file
*/
#include <disp_drv_log.h>
#include <disp_interface.h>
#include <disp_reg_rw.h>
#include <disp_config.h>
#include <reg.h>
#include <lvds_reg.h>
#include <stdlib.h>
#include <string.h>
#include <board.h>

enum LVDS_CHANNEL {
    LVDS_CH0 = 0,
    LVDS_CH1,
    LVDS_CH2,
    LVDS_CH3,
    LVDS_CH_MAX_NUM
};

enum {
    LVDS_SEPARATE_MODE = 0,
    LVDS_DUPLICATE_MODE,
    LVDS_DUAL_MODE
};

enum {
    LVDS_PIXEL_BPP_NC = -1,
    LVDS_PIXEL_BPP_16,
    LVDS_PIXEL_BPP_24,
    LVDS_PIXEL_BPP_30,
};

enum {
    SOFT_RESET_CH0 = 0,
    SOFT_RESET_CH1,
    SOFT_RESET_CH2,
    SOFT_RESET_CH3,
    SOFT_RESET_COMBINE
};



typedef enum {
    MODE_SEPARATE_SEL = 0b0,
    MODE_DUAL_SEL     = 0b1
} LVDS_MODE_SEL;

typedef enum {
    PIXEL_FORMAT_18_BIT = 0b00,
    PIXEL_FORMAT_24_BIT = 0b01,
    PIXEL_FORMAT_30_BIT = 0b10
} LVDS_PIXEL_FORMAT;

typedef enum {
    MAP_FORMAT_JEIDA = 0b0,
    MAP_FORMAT_SWPG = 0b01
} LVDS_MAP_FORMAT;

enum {
    DUALLODD_ODD = 0b0,
    DUALLODD_EVEN = 0b1
};

struct lvds_chx_info {
    uint32_t ch_idx;
    uint32_t ch_mode;
    uint32_t pixel_fmt;
    uint32_t map_format;
    uint32_t vsync_pol;
    uint8_t vbias_sel;
    uint8_t txen;
    uint8_t txdrv_vol;
    uint8_t txdrv_cur;
    uint8_t tx_cm;
    uint8_t rterm_en;
    uint8_t bias_en;
    uint8_t dual_port_swap;
};

static inline void delay(uint32_t delay)
{
    osDelay(delay);
}

static addr_t lvds_get_reg_base(void)
{
    static addr_t base = 0;

    if (base == 0)
        base = (addr_t) _ioaddr(LVDS_BASE);

    return base;
}

static int get_lvds_pixel_bpp(int pixel_bpp)
{
    switch (pixel_bpp) {
        case PIXEL_BPP_16:
            return LVDS_PIXEL_BPP_16;
        case PIXEL_BPP_24:
            return LVDS_PIXEL_BPP_24;
        case PIXEL_BPP_30:
            return LVDS_PIXEL_BPP_30;
        default:
            return LVDS_PIXEL_BPP_NC;
    }
}

static int lvds_soft_reset(int channel)
{
    int val;
    addr_t base = lvds_get_reg_base();

    switch(channel) {
    case SOFT_RESET_CH0:
        /*set to high*/
        val = disp_read(base, LVDS_SOFT_RESET);
        val = reg_value(1, val, CH0_SOFT_RESET_SHIFT, CH0_SOFT_RESET_MASK);
        disp_write(base, LVDS_SOFT_RESET,val);
        delay(1);
        /*set to low*/
        val = reg_value(0, val, CH0_SOFT_RESET_SHIFT, CH0_SOFT_RESET_MASK);
        disp_write(base, LVDS_SOFT_RESET,val);
        break;
    case SOFT_RESET_CH1:
        /*set to high*/
        val = disp_read(base, LVDS_SOFT_RESET);
        val = reg_value(1, val, CH1_SOFT_RESET_SHIFT, CH1_SOFT_RESET_MASK);
        disp_write(base, LVDS_SOFT_RESET,val);
        delay(1);
        /*set to low*/
        val = reg_value(0, val, CH1_SOFT_RESET_SHIFT, CH1_SOFT_RESET_MASK);
        disp_write(base, LVDS_SOFT_RESET,val);
        break;
    case SOFT_RESET_CH2:
        /*set to high*/
        val = disp_read(base, LVDS_SOFT_RESET);
        val = reg_value(1, val, CH2_SOFT_RESET_SHIFT, CH2_SOFT_RESET_MASK);
        disp_write(base, LVDS_SOFT_RESET,val);
        delay(1);
        /*set to low*/
        val = reg_value(0, val, CH2_SOFT_RESET_SHIFT, CH2_SOFT_RESET_MASK);
        disp_write(base, LVDS_SOFT_RESET,val);
        break;
    case SOFT_RESET_CH3:
        /*set to high*/
        val = disp_read(base, LVDS_SOFT_RESET);
        val = reg_value(1, val, CH3_SOFT_RESET_SHIFT, CH3_SOFT_RESET_MASK);
        disp_write(base, LVDS_SOFT_RESET,val);
        delay(1);
        /*set to low*/
        val = reg_value(0, val, CH3_SOFT_RESET_SHIFT, CH3_SOFT_RESET_MASK);
        disp_write(base, LVDS_SOFT_RESET,val);
        break;
    case SOFT_RESET_COMBINE:
        /*set to high*/
        val = disp_read(base, LVDS_SOFT_RESET);
        val = reg_value(1, val, CB_SOFT_RESET_SHIFT, CB_SOFT_RESET_MASK);
        disp_write(base, LVDS_SOFT_RESET,val);
        delay(1);
        /*set to low*/
        val = reg_value(0, val, CB_SOFT_RESET_SHIFT, CB_SOFT_RESET_MASK);
        disp_write(base, LVDS_SOFT_RESET,val);
        break;
    default:
        break;
    }

    return 0;
}

/*static void lvds_combine_cfg(LCM_LVDS_PARAMS *param)
{
    int val;
    addr_t base = lvds_get_reg_base();

    val = disp_read(base, LVDS_COMBINE);
    val = reg_value(param->vld_height, val,
        CMB_VLD_HEIGHT_SHIFT, CMB_VLD_HEIGHT_MASK);
    val = reg_value(param->gap, val, CMB_GAP_SHIFT, CMB_GAP_MASK);
    val = reg_value(param->vsync_pol, CMB_VSYNC_POL_SHIFT, CMB_VSYNC_POL_MASK);
    val = reg_value(param->hsync_pol, CMB_HSYNC_POL_SHIFT, CMB_HSYNC_POL_MASK);
    val = reg_value(param->data_en_pol,
        CMB_DATA_EN_POL_SHIFT, CMB_DATA_EN_POL_MASK);
    disp_write(base, LVDS_COMBINE);
}*/

static int lvds_init(struct intf_device *dev)
{
    return 0;
}

static int lvds_deinit(struct intf_device *dev)
{
    return 0;
}

static void lvds_channel_enable(addr_t base, struct lvds_chx_info *info, uint32_t enable)
{
    int val, i;

    val = disp_read(base, LVDS_CHN_PAD_COM_SET_(info->ch_idx));

    val = reg_value(!enable, val, CHN_TEST_PULLDN_SHIFT, CHN_TEST_PULLDN_MASK);

    disp_write(base, LVDS_CHN_PAD_COM_SET_(info->ch_idx), val);

    val = disp_read(base, LVDS_CHN_CTRL_(info->ch_idx));

    if (enable) {
       val = reg_value(1, val, CHN_EN_SHIFT, CHN_EN_MASK);
       disp_write(base, LVDS_CHN_CTRL_(info->ch_idx), val);
    } else {
       val = reg_value(0, val, CHN_EN_SHIFT, CHN_EN_MASK);
       disp_write(base, LVDS_CHN_CTRL_(info->ch_idx), val);
    }

    for (i = 0; i < 5; i++) {
        val = disp_read(base, LVDS_CHN_PAD_SET_(info->ch_idx) + 0x4 * i);

        val = reg_value(enable, val, CHN_TXEN_SHIFT, CHN_TXEN_MASK);
        val = reg_value(enable, val, CHN_RTERM_EN_SHIFT, CHN_RTERM_EN_MASK);

        disp_write(base, LVDS_CHN_PAD_SET_(info->ch_idx) + 0x4 * i, val);
    }
}

static void lvds_power_on_off(struct intf_device *dev, uint32_t enable)
{
    struct lvds_chx_info *chx_info;

    addr_t base = lvds_get_reg_base();

    chx_info = malloc(sizeof(struct lvds_chx_info));
    if (!chx_info) {
        DISPERR("lvds chx info malloc failed\n");
        return;
    }

    memset(chx_info, 0, sizeof(struct lvds_chx_info));

    if (dev->lvds_mode == LVDS_SEPARATE_MODE) {
        chx_info->ch_mode = LVDS_SEPARATE_MODE;
        switch (dev->intf_res) {
        case IF_RES_LVDS1:
            chx_info->ch_idx = LVDS_CH0;
            lvds_channel_enable(base, chx_info, enable);
            break;
        case IF_RES_LVDS2:
            chx_info->ch_idx = LVDS_CH1;
            lvds_channel_enable(base, chx_info, enable);
            break;
        case IF_RES_LVDS3:
            chx_info->ch_idx = LVDS_CH2;
            lvds_channel_enable(base, chx_info, enable);
            break;
        case IF_RES_LVDS4:
            chx_info->ch_idx = LVDS_CH3;
            lvds_channel_enable(base, chx_info, enable);
            break;
        case IF_RES_LVDS1_LVDS2:
            /*config lvds_ch0*/
            chx_info->ch_idx = LVDS_CH0;
            lvds_channel_enable(base, chx_info, enable);

            /*config lvds_ch1*/
            chx_info->ch_idx = LVDS_CH1;
            lvds_channel_enable(base, chx_info, enable);
            break;
        default:
            DISPERR("if res not correct.\n");
            goto FREE;
        }

    } else if (dev->lvds_mode == LVDS_DUAL_MODE) {
        chx_info->ch_mode = LVDS_DUAL_MODE;
        switch (dev->intf_res) {
        case IF_RES_LVDS1_LVDS2:
            /*config lvds_ch0*/
            chx_info->ch_idx = LVDS_CH0;
            lvds_channel_enable(base, chx_info, enable);

            /*config lvds_ch1*/
            chx_info->ch_idx = LVDS_CH1;
            lvds_channel_enable(base, chx_info, enable);
            break;
        case IF_RES_LVDS3_LVDS4:
            /*config lvds_ch2*/
            chx_info->ch_idx = LVDS_CH2;
            lvds_channel_enable(base, chx_info, enable);

            /*config lvds_ch3*/
            chx_info->ch_idx = LVDS_CH3;
            lvds_channel_enable(base, chx_info, enable);
            break;
        case IF_RES_LVDS1_4:
            /*config lvds ch0*/
            chx_info->ch_idx = LVDS_CH0;
            lvds_channel_enable(base, chx_info, enable);

            /*config lvds ch1*/
            chx_info->ch_idx = LVDS_CH1;
            lvds_channel_enable(base, chx_info, enable);

            /*config lvds ch2*/
            chx_info->ch_idx = LVDS_CH2;
            lvds_channel_enable(base, chx_info, enable);
            /*config lvds ch3*/
            chx_info->ch_idx = LVDS_CH3;
            lvds_channel_enable(base, chx_info, enable);
            break;
        default:
            DISPERR("Can not support this Channel: %d.\n", dev->intf_res);
            goto FREE;
        }
    }

FREE:
    free(chx_info);
}

static int lvds_power_on(struct intf_device *dev)
{
    lvds_power_on_off(dev, 1);

    return 0;
}

static int lvds_power_off(struct intf_device *dev)
{
    lvds_power_on_off(dev, 0);

    return 0;
}

static void lvds_channel_config(addr_t base, struct lvds_chx_info *info)
{
    int val, mux, ch_mode, i;

    if (info->ch_mode == LVDS_DUAL_MODE)
        ch_mode = MODE_DUAL_SEL;
    else
        ch_mode = MODE_SEPARATE_SEL;

    val = disp_read(base, LVDS_CHN_CTRL_(info->ch_idx));
    val = reg_value(ch_mode, val, CHN_DUALMODE_SHIFT, CHN_DUALMODE_MASK);
    val = reg_value(info->pixel_fmt, val, CHN_BPP_SHIFT, CHN_BPP_MASK);
    val = reg_value(info->map_format, val, CHN_FORMAT_SHIFT, CHN_FORMAT_MASK);
    val = reg_value(info->vsync_pol, val,
        CHN_VSYNC_POL_SHIFT, CHN_VSYNC_POL_MASK);

    if ((info->pixel_fmt == PIXEL_FORMAT_30_BIT) &&
        (info->ch_mode == LVDS_DUAL_MODE)) {
        if ((info->ch_idx == LVDS_CH0) || (info->ch_idx == LVDS_CH1))
            val = reg_value(DUALLODD_ODD, val,
                CHN_DUALODD_SHIFT, CHN_DUALODD_MASK);
        else
            val = reg_value(DUALLODD_EVEN, val,
                CHN_DUALODD_SHIFT, CHN_DUALODD_MASK);
    }

    if (info->pixel_fmt == PIXEL_FORMAT_30_BIT) {
        if ((info->ch_idx == LVDS_CH0) || (info->ch_idx == LVDS_CH2))
            mux = 0;
        else
            mux = 1;
    } else {
        if (info->ch_mode == LVDS_DUAL_MODE) {
            if (!info->dual_port_swap) {
                if ((info->ch_idx == LVDS_CH0) || (info->ch_idx == LVDS_CH2))
                    mux = 0;
                else
                    mux = 1;
            } else {
                if ((info->ch_idx == LVDS_CH0) || (info->ch_idx == LVDS_CH2))
                    mux = 1;
                else
                    mux = 0;
            }
        } else if (info->ch_mode == LVDS_SEPARATE_MODE) {
            if ((info->ch_idx == LVDS_CH0) || (info->ch_idx == LVDS_CH2))
                mux = 0;
            else
                mux = 2;
        } else {
            mux = 0;
        }
    }

    val = reg_value(mux, val, CHN_MUX_SHIFT, CHN_MUX_MASK);

    val = reg_value(0, val, CHN_EN_SHIFT, CHN_EN_MASK);
    disp_write(base, LVDS_CHN_CTRL_(info->ch_idx), val);

    val = disp_read(base, LVDS_CHN_PAD_COM_SET_(info->ch_idx));
    val = reg_value(1, val, CHN_TEST_PULLDN_SHIFT, CHN_TEST_PULLDN_MASK);
    disp_write(base, LVDS_CHN_PAD_COM_SET_(info->ch_idx), val);

    /*config 5 lanes setting*/
    for (i = 0; i < 5; i++) {
        val = disp_read(base, LVDS_CHN_PAD_SET_(info->ch_idx) + 0x4 * i);

        val = reg_value(info->vbias_sel, val, CHN_VBIAS_SEL_SHIFT, CHN_VBIAS_SEL_MASK);
        val = reg_value(info->txen, val, CHN_TXEN_SHIFT, CHN_TXEN_MASK);
        val = reg_value((info->txdrv_vol << 2) | info->txdrv_cur, val, CHN_TXDRV_SHIFT, CHN_TXDRV_MASK);
        val = reg_value(info->tx_cm, val, CHN_TXCM_SHIFT, CHN_TXCM_MASK);
        val = reg_value(info->rterm_en, val, CHN_RTERM_EN_SHIFT, CHN_RTERM_EN_MASK);
        val = reg_value(info->bias_en, val, CHN_BIAS_EN_SHIFT, CHN_BIAS_EN_MASK);

        disp_write(base, LVDS_CHN_PAD_SET_(info->ch_idx) + 0x4 * i, val);
    }
}

static void lvds_config(struct intf_device *dev, struct sdm_panel *panel)
{
    int val, lvds_pixel_bpp;
    addr_t base = lvds_get_reg_base();
    struct lvds_chx_info *chx_info;

    chx_info = malloc(sizeof(struct lvds_chx_info));
    if (!chx_info) {
        DISPERR("lvds chx info malloc failed\n");
        return;
    }
    memset(chx_info, 0, sizeof(struct lvds_chx_info));

    lvds_pixel_bpp = get_lvds_pixel_bpp(dev->pixel_bpp);
    chx_info->map_format = panel->display_timing->map_format;
    chx_info->vsync_pol = panel->display_timing->vsync_pol;
    chx_info->pixel_fmt = lvds_pixel_bpp;
    if (panel->lvds) {
        chx_info->vbias_sel = panel->lvds->vbias_sel;
        chx_info->txen = panel->lvds->txen;
        chx_info->txdrv_vol = panel->lvds->txdrv_vol;
        chx_info->txdrv_cur = panel->lvds->txdrv_cur;
        chx_info->tx_cm = panel->lvds->tx_cm;
        chx_info->rterm_en = panel->lvds->rterm_en;
        chx_info->bias_en = panel->lvds->bias_en;
        chx_info->dual_port_swap = panel->lvds->dual_port_swap;
    } else {
        chx_info->vbias_sel = 0x1;
        chx_info->txen = 0x0;
        chx_info->txdrv_vol = 0x0;
        chx_info->txdrv_cur = 0x1;
        chx_info->tx_cm = 0x1;
        chx_info->rterm_en = 0x0;
        chx_info->bias_en = 0x1;
        chx_info->dual_port_swap = 0x0;
    }

    val = disp_read(base, LVDS_BIAS_SET);
    val = reg_value(1, val, BIAS_EN_SHIFT, BIAS_EN_MASK);
    disp_write(base, LVDS_BIAS_SET, val);

    if (lvds_pixel_bpp == PIXEL_FORMAT_30_BIT) {
        if (dev->lvds_mode == LVDS_SEPARATE_MODE) {
            chx_info->ch_mode = LVDS_SEPARATE_MODE;
            switch (dev->intf_res) {
            case IF_RES_LVDS1_LVDS2:
                /*config lvds_ch0*/
                chx_info->ch_idx = LVDS_CH0;
                lvds_channel_config(base, chx_info);

                /*config lvds_ch1*/
                chx_info->ch_idx = LVDS_CH1;
                lvds_channel_config(base, chx_info);
                break;
            case IF_RES_LVDS3_LVDS4:
                /*config lvds_ch2*/
                chx_info->ch_idx = LVDS_CH2;
                lvds_channel_config(base, chx_info);

                /*config lvds_ch3*/
                chx_info->ch_idx = LVDS_CH3;
                lvds_channel_config(base, chx_info);
                break;
            default:
                DISPERR("Can not support this Channel: %d.\n", dev->intf_res);
                goto FREE;
            }
        }else if (dev->lvds_mode == LVDS_DUAL_MODE) {
            chx_info->ch_mode = LVDS_DUAL_MODE;
            /*config lvds_ch0*/
            chx_info->ch_idx = LVDS_CH0;
            lvds_channel_config(base, chx_info);

            /*config lvds_ch1*/
            chx_info->ch_idx = LVDS_CH1;
            lvds_channel_config(base, chx_info);

            /*config lvds_ch2*/
            chx_info->ch_idx = LVDS_CH2;
            lvds_channel_config(base, chx_info);

            /*config lvds_ch3*/
            chx_info->ch_idx = LVDS_CH3;
            lvds_channel_config(base, chx_info);
        }else {
            DISPERR("30_BIT can not support this lvds mode.\n");
            goto FREE;
        }
    }else {
        if (dev->lvds_mode == LVDS_SEPARATE_MODE) {
            chx_info->ch_mode = LVDS_SEPARATE_MODE;
            switch (dev->intf_res) {
            case IF_RES_LVDS1:
                chx_info->ch_idx = LVDS_CH0;
                lvds_channel_config(base, chx_info);
                break;
            case IF_RES_LVDS2:
                chx_info->ch_idx = LVDS_CH1;
                lvds_channel_config(base, chx_info);
                break;
            case IF_RES_LVDS3:
                chx_info->ch_idx = LVDS_CH2;
                lvds_channel_config(base, chx_info);
                break;
            case IF_RES_LVDS4:
                chx_info->ch_idx = LVDS_CH3;
                lvds_channel_config(base, chx_info);
                break;
            case IF_RES_LVDS1_LVDS2:
                /*config lvds_ch0*/
                chx_info->ch_idx = LVDS_CH0;
                lvds_channel_config(base, chx_info);

                /*config lvds_ch1*/
                chx_info->ch_idx = LVDS_CH1;
                lvds_channel_config(base, chx_info);
                break;
            default:
                DISPERR("if res not correct.\n");
                goto FREE;
            }
        }else if (dev->lvds_mode == LVDS_DUAL_MODE) {
            chx_info->ch_mode = LVDS_DUAL_MODE;
            switch (dev->intf_res) {
            case IF_RES_LVDS1_LVDS2:
                /*config lvds_ch0*/
                chx_info->ch_idx = LVDS_CH0;
                lvds_channel_config(base, chx_info);

                /*config lvds_ch1*/
                chx_info->ch_idx = LVDS_CH1;
                lvds_channel_config(base, chx_info);
                break;
            case IF_RES_LVDS3_LVDS4:
                /*config lvds_ch2*/
                chx_info->ch_idx = LVDS_CH2;
                lvds_channel_config(base, chx_info);

                /*config lvds_ch3*/
                chx_info->ch_idx = LVDS_CH3;
                lvds_channel_config(base, chx_info);
                break;
            case IF_RES_LVDS1_4:
                val = disp_read(base, LVDS_COMBINE);
                val = reg_value(panel->display_timing->hactive / 2 - 1, val,
                    CMB_VLD_HEIGHT_SHIFT, CMB_VLD_HEIGHT_MASK);
                disp_write(base, LVDS_COMBINE, val);
                /*config lvds ch0*/
                chx_info->ch_idx = LVDS_CH0;
                lvds_channel_config(base, chx_info);

                /*config lvds ch1*/
                chx_info->ch_idx = LVDS_CH1;
                lvds_channel_config(base, chx_info);

                /*config lvds ch2*/
                chx_info->ch_idx = LVDS_CH2;
                lvds_channel_config(base, chx_info);

                /*config lvds ch3*/
                chx_info->ch_idx = LVDS_CH3;
                lvds_channel_config(base, chx_info);
                break;
            default:
                DISPERR("Can not support this Channel: %d.\n", dev->intf_res);
                goto FREE;
            }
        }else if (dev->lvds_mode == LVDS_DUPLICATE_MODE) {
            chx_info->ch_mode = LVDS_DUPLICATE_MODE;
            switch (dev->intf_res) {
                case IF_RES_LVDS1_LVDS2:
                /*config lvds_ch0*/
                chx_info->ch_idx = LVDS_CH0;
                lvds_channel_config(base, chx_info);

                /*config lvds_ch1*/
                chx_info->ch_idx = LVDS_CH1;
                lvds_channel_config(base, chx_info);
                break;
            case IF_RES_LVDS3_LVDS4:
                /*config lvds_ch2*/
                chx_info->ch_idx = LVDS_CH2;
                lvds_channel_config(base, chx_info);

                /*config lvds_ch3*/
                chx_info->ch_idx = LVDS_CH3;
                lvds_channel_config(base, chx_info);
                break;
            default:
                DISPERR("Can not support this Channel.\n");
                goto FREE;
            }
        } else {
            DISPERR("Can not support this lvds mode.\n");
            goto FREE;
        }
    }

FREE:
    free(chx_info);
}

DISP_MODULE_DRIVER disp_driver_lvds = {
    .module = DISP_IF_MODULE_LVDS,
    .init   = lvds_init,
    .deinit = lvds_deinit,
    .config = lvds_config,
    .power_on = lvds_power_on,
    .power_off = lvds_power_off,
};

