/**
 * @file tas5404.c
 * @brief tas5404 source file.
 *
 * @copyright Copyright (c) 2021  Semidrive Semiconductor.
 *            All rights reserved.
 */

#include <debug.h>
#include "tas5404.h"
#include "driver/i2c.h"
#include "param.h"

#define CODEC_TAS5404_DBG_PRT SSDK_DEBUG

/**
 * @brief register value table
 *
 */
static am_reg_t tas5404_regs[] = {
    {TAS5404_00_FAULT_REG1, 0x0, CACHE_INVALID},
    {TAS5404_01_FAULT_REG2, 0x0, CACHE_INVALID},
    {TAS5404_02_DIAG_REG1, 0x0, CACHE_INVALID},
    {TAS5404_03_DIAG_REG2, 0x0, CACHE_INVALID},
    {TAS5404_04_STAT_REG1, 0x0, CACHE_INVALID},
    {TAS5404_05_STAT_REG2, 0x0, CACHE_INVALID},
    {TAS5404_06_STAT_REG3, 0x0, CACHE_INVALID},
    {TAS5404_07_STAT_REG4, 0x0, CACHE_INVALID},
    {TAS5404_08_CTRL1, 0x0, CACHE_INVALID},
    {TAS5404_09_CTRL2, 0x0, CACHE_INVALID},
    {TAS5404_0A_CTRL3, 0x0, CACHE_INVALID},
    {TAS5404_0B_CTRL4, 0x0, CACHE_INVALID},
    {TAS5404_0C_CTRL5, 0x0, CACHE_INVALID},
    {TAS5404_0D_CTRL6, 0x0, CACHE_INVALID},
    {TAS5404_10_CTRL7, 0x0, CACHE_INVALID},
    {TAS5404_13_STAT_REG5, 0x0, CACHE_INVALID},
    {TAS5404_14_VIR_FAULT_N, 0x0, CACHE_INVALID},
    {TAS5404_15_VIR_CLIP_OTW_N, 0x0, CACHE_INVALID}
};

/**
 * @brief tas5404 control attribute table
 *
 */
static am_ctl_t tas5404_ctls[] = {
    {TAS5404_00_01_OTW, TAS5404_00_FAULT_REG1, 0x1, 0x0, CTL_NORMAL_TYPE},
    {TAS5404_00_02_DC_OFFSET, TAS5404_00_FAULT_REG1, 0x2, 0x0, CTL_NORMAL_TYPE},
    {TAS5404_00_04_OCSD, TAS5404_00_FAULT_REG1, 0x4, 0x0, CTL_NORMAL_TYPE},
    {TAS5404_00_08_OTSD, TAS5404_00_FAULT_REG1, 0x8, 0x0, CTL_NORMAL_TYPE},
    {TAS5404_00_10_CP_UV, TAS5404_00_FAULT_REG1, 0x10, 0x0, CTL_NORMAL_TYPE},
    {TAS5404_00_20_AVDD_UV, TAS5404_00_FAULT_REG1, 0x20, 0x0, CTL_NORMAL_TYPE},
    {TAS5404_00_40_PVDD_UV, TAS5404_00_FAULT_REG1, 0x40, 0x0, CTL_NORMAL_TYPE},
    {TAS5404_00_80_PVDD_OV, TAS5404_00_FAULT_REG1, 0x80, 0x0, CTL_NORMAL_TYPE},
    {TAS5404_01_01_OTSD_CH1, TAS5404_01_FAULT_REG2, 0x1, 0x0, CTL_NORMAL_TYPE},
    {TAS5404_01_02_OTSD_CH2, TAS5404_01_FAULT_REG2, 0x2, 0x0, CTL_NORMAL_TYPE},
    {TAS5404_01_04_OTSD_CH3, TAS5404_01_FAULT_REG2, 0x4, 0x0, CTL_NORMAL_TYPE},
    {TAS5404_01_08_OTSD_CH4, TAS5404_01_FAULT_REG2, 0x8, 0x0, CTL_NORMAL_TYPE},
    {
        TAS5404_01_10_DC_OFFSET_CH1, TAS5404_01_FAULT_REG2, 0x10, 0x0,
        CTL_NORMAL_TYPE
    },
    {
        TAS5404_01_20_DC_OFFSET_CH2, TAS5404_01_FAULT_REG2, 0x20, 0x0,
        CTL_NORMAL_TYPE
    },
    {
        TAS5404_01_40_DC_OFFSET_CH3, TAS5404_01_FAULT_REG2, 0x40, 0x0,
        CTL_NORMAL_TYPE
    },
    {
        TAS5404_01_80_DC_OFFSET_CH4, TAS5404_01_FAULT_REG2, 0x80, 0x0,
        CTL_NORMAL_TYPE
    },
    {TAS5404_02_01_CH1_S2G, TAS5404_02_DIAG_REG1, 0x1, 0x0, CTL_NORMAL_TYPE},
    {TAS5404_02_02_CH1_S2P, TAS5404_02_DIAG_REG1, 0x2, 0x0, CTL_NORMAL_TYPE},
    {TAS5404_02_04_CH1_SL, TAS5404_02_DIAG_REG1, 0x4, 0x0, CTL_NORMAL_TYPE},
    {TAS5404_02_08_CH1_OP, TAS5404_02_DIAG_REG1, 0x8, 0x0, CTL_NORMAL_TYPE},
    {TAS5404_02_10_CH2_S2G, TAS5404_02_DIAG_REG1, 0x10, 0x0, CTL_NORMAL_TYPE},
    {TAS5404_02_20_CH2_S2P, TAS5404_02_DIAG_REG1, 0x20, 0x0, CTL_NORMAL_TYPE},
    {TAS5404_02_40_CH2_SL, TAS5404_02_DIAG_REG1, 0x40, 0x0, CTL_NORMAL_TYPE},
    {TAS5404_02_80_CH2_OP, TAS5404_02_DIAG_REG1, 0x80, 0x0, CTL_NORMAL_TYPE},
    {TAS5404_03_01_CH3_S2G, TAS5404_03_DIAG_REG2, 0x1, 0x0, CTL_NORMAL_TYPE},
    {TAS5404_03_02_CH3_S2P, TAS5404_03_DIAG_REG2, 0x2, 0x0, CTL_NORMAL_TYPE},
    {TAS5404_03_04_CH3_SL, TAS5404_03_DIAG_REG2, 0x4, 0x0, CTL_NORMAL_TYPE},
    {TAS5404_03_08_CH3_OP, TAS5404_03_DIAG_REG2, 0x8, 0x0, CTL_NORMAL_TYPE},
    {TAS5404_03_10_CH4_S2G, TAS5404_03_DIAG_REG2, 0x10, 0x0, CTL_NORMAL_TYPE},
    {TAS5404_03_20_CH4_S2P, TAS5404_03_DIAG_REG2, 0x20, 0x0, CTL_NORMAL_TYPE},
    {TAS5404_03_40_CH4_SL, TAS5404_03_DIAG_REG2, 0x40, 0x0, CTL_NORMAL_TYPE},
    {TAS5404_03_80_CH4_OP, TAS5404_03_DIAG_REG2, 0x80, 0x0, CTL_NORMAL_TYPE},
    {TAS5404_04_01_PVDD_OV, TAS5404_04_STAT_REG1, 0x1, 0x0, CTL_NORMAL_TYPE},
    {TAS5404_04_02_PVDD_UV, TAS5404_04_STAT_REG1, 0x2, 0x0, CTL_NORMAL_TYPE},
    {TAS5404_04_04_AVDD_FAULT, TAS5404_04_STAT_REG1, 0x4, 0x0, CTL_NORMAL_TYPE},
    {TAS5404_04_08_CP_FAULT, TAS5404_04_STAT_REG1, 0x8, 0x0, CTL_NORMAL_TYPE},
    {
        TAS5404_04_10_OT_SHUTDOWN, TAS5404_04_STAT_REG1, 0x10, 0x0,
        CTL_NORMAL_TYPE
    },
    {TAS5404_04_E0_OTW_LEVEL, TAS5404_04_STAT_REG1, 0xe0, 0x0, CTL_NORMAL_TYPE},
    {TAS5404_05_01_HIZ_CH1, TAS5404_05_STAT_REG2, 0x1, 0x0, CTL_NORMAL_TYPE},
    {TAS5404_05_02_HIZ_CH2, TAS5404_05_STAT_REG2, 0x2, 0x0, CTL_NORMAL_TYPE},
    {TAS5404_05_04_HIZ_CH3, TAS5404_05_STAT_REG2, 0x4, 0x0, CTL_NORMAL_TYPE},
    {TAS5404_05_08_HIZ_CH4, TAS5404_05_STAT_REG2, 0x8, 0x0, CTL_NORMAL_TYPE},
    {
        TAS5404_05_10_LOW_LOW_CH1, TAS5404_05_STAT_REG2, 0x10, 0x0,
        CTL_NORMAL_TYPE
    },
    {
        TAS5404_05_20_LOW_LOW_CH2, TAS5404_05_STAT_REG2, 0x20, 0x0,
        CTL_NORMAL_TYPE
    },
    {
        TAS5404_05_40_LOW_LOW_CH3, TAS5404_05_STAT_REG2, 0x40, 0x0,
        CTL_NORMAL_TYPE
    },
    {
        TAS5404_05_80_LOW_LOW_CH4, TAS5404_05_STAT_REG2, 0x80, 0x0,
        CTL_NORMAL_TYPE
    },
    {
        TAS5404_06_01_PLAY_MODE_CH1, TAS5404_06_STAT_REG3, 0x1, 0x0,
        CTL_NORMAL_TYPE
    },
    {
        TAS5404_06_02_PLAY_MODE_CH2, TAS5404_06_STAT_REG3, 0x2, 0x0,
        CTL_NORMAL_TYPE
    },
    {
        TAS5404_06_04_PLAY_MODE_CH3, TAS5404_06_STAT_REG3, 0x4, 0x0,
        CTL_NORMAL_TYPE
    },
    {
        TAS5404_06_08_PLAY_MODE_CH4, TAS5404_06_STAT_REG3, 0x8, 0x0,
        CTL_NORMAL_TYPE
    },
    {TAS5404_06_10_MUTE_CH1, TAS5404_06_STAT_REG3, 0x10, 0x0, CTL_NORMAL_TYPE},
    {TAS5404_06_20_MUTE_CH2, TAS5404_06_STAT_REG3, 0x20, 0x0, CTL_NORMAL_TYPE},
    {TAS5404_06_40_MUTE_CH3, TAS5404_06_STAT_REG3, 0x40, 0x0, CTL_NORMAL_TYPE},
    {TAS5404_06_80_MUTE_CH4, TAS5404_06_STAT_REG3, 0x80, 0x0, CTL_NORMAL_TYPE},
    {
        TAS5404_07_01_LOADDIAG_MODE_CH1, TAS5404_07_STAT_REG4, 0x1, 0x0,
        CTL_NORMAL_TYPE
    },
    {
        TAS5404_07_02_LOADDIAG_MODE_CH2, TAS5404_07_STAT_REG4, 0x2, 0x0,
        CTL_NORMAL_TYPE
    },
    {
        TAS5404_07_04_LOADDIAG_MODE_CH3, TAS5404_07_STAT_REG4, 0x4, 0x0,
        CTL_NORMAL_TYPE
    },
    {
        TAS5404_07_08_LOADDIAG_MODE_CH4, TAS5404_07_STAT_REG4, 0x8, 0x0,
        CTL_NORMAL_TYPE
    },
    {
        TAS5404_07_10_OT_FOLDBACK_CH1, TAS5404_07_STAT_REG4, 0x10, 0x0,
        CTL_NORMAL_TYPE
    },
    {
        TAS5404_07_20_OT_FOLDBACK_CH2, TAS5404_07_STAT_REG4, 0x20, 0x0,
        CTL_NORMAL_TYPE
    },
    {
        TAS5404_07_40_OT_FOLDBACK_CH3, TAS5404_07_STAT_REG4, 0x40, 0x0,
        CTL_NORMAL_TYPE
    },
    {
        TAS5404_07_80_OT_FOLDBACK_CH4, TAS5404_07_STAT_REG4, 0x80, 0x0,
        CTL_NORMAL_TYPE
    },
    {TAS5404_08_03_GAIN_CH1, TAS5404_08_CTRL1, 0x3, 0x0, CTL_NORMAL_TYPE},
    {TAS5404_08_0C_GAIN_CH2, TAS5404_08_CTRL1, 0xc, 0x0, CTL_NORMAL_TYPE},
    {TAS5404_08_30_GAIN_CH3, TAS5404_08_CTRL1, 0x30, 0x0, CTL_NORMAL_TYPE},
    {TAS5404_08_C0_GAIN_CH4, TAS5404_08_CTRL1, 0xc0, 0x0, CTL_NORMAL_TYPE},
    {
        TAS5404_09_01_THERMAL_FOLDBACK, TAS5404_09_CTRL2, 0x1, 0x0,
        CTL_NORMAL_TYPE
    },
    {TAS5404_09_10_OC_LEVEL_CH1, TAS5404_09_CTRL2, 0x10, 0x0, CTL_NORMAL_TYPE},
    {TAS5404_09_20_OC_LEVEL_CH2, TAS5404_09_CTRL2, 0x20, 0x0, CTL_NORMAL_TYPE},
    {TAS5404_09_40_OC_LEVEL_CH3, TAS5404_09_CTRL2, 0x40, 0x0, CTL_NORMAL_TYPE},
    {TAS5404_09_80_OC_LEVEL_CH4, TAS5404_09_CTRL2, 0x80, 0x0, CTL_NORMAL_TYPE},
    {TAS5404_0A_03_FREQ_SEL, TAS5404_0A_CTRL3, 0x3, 0x0, CTL_NORMAL_TYPE},
    {
        TAS5404_0A_0C_CONF_CLIP_OTW_REPORT, TAS5404_0A_CTRL3, 0xc, 0x0,
        CTL_NORMAL_TYPE
    },
    {
        TAS5404_0A_10_HARD_STOP_MODE, TAS5404_0A_CTRL3, 0x10, 0x0,
        CTL_NORMAL_TYPE
    },
    {TAS5404_0A_20_PHASE_DIFF, TAS5404_0A_CTRL3, 0x20, 0x0, CTL_NORMAL_TYPE},
    {TAS5404_0A_40_SYNC_PULSE, TAS5404_0A_CTRL3, 0x40, 0x0, CTL_NORMAL_TYPE},
    {
        TAS5404_0A_80_CONF_THERMAL_REPORT, TAS5404_0A_CTRL3, 0x80, 0x0,
        CTL_NORMAL_TYPE
    },
    {TAS5404_0B_01_LOADDIAG_CH1, TAS5404_0B_CTRL4, 0x1, 0x0, CTL_NORMAL_TYPE},
    {TAS5404_0B_02_LOADDIAG_CH2, TAS5404_0B_CTRL4, 0x2, 0x0, CTL_NORMAL_TYPE},
    {TAS5404_0B_04_LOADDIAG_CH3, TAS5404_0B_CTRL4, 0x4, 0x0, CTL_NORMAL_TYPE},
    {TAS5404_0B_08_LOADDIAG_CH4, TAS5404_0B_CTRL4, 0x8, 0x0, CTL_NORMAL_TYPE},
    {TAS5404_0B_10_DC_DETECTION, TAS5404_0B_CTRL4, 0x10, 0x0, CTL_NORMAL_TYPE},
    {
        TAS5404_0B_20_TWEETER_DETECT, TAS5404_0B_CTRL4, 0x20, 0x0,
        CTL_NORMAL_TYPE
    },
    {TAS5404_0B_40_SLAVE_MODE, TAS5404_0B_CTRL4, 0x40, 0x0, CTL_NORMAL_TYPE},
    {TAS5404_0B_80_CFG_OSC_SYNC, TAS5404_0B_CTRL4, 0x80, 0x0, CTL_NORMAL_TYPE},
    {TAS5404_0C_01_UNMUTE_CH1, TAS5404_0C_CTRL5, 0x1, 0x0, CTL_NORMAL_TYPE},
    {TAS5404_0C_02_UNMUTE_CH2, TAS5404_0C_CTRL5, 0x2, 0x0, CTL_NORMAL_TYPE},
    {TAS5404_0C_04_UNMUTE_CH3, TAS5404_0C_CTRL5, 0x4, 0x0, CTL_NORMAL_TYPE},
    {TAS5404_0C_08_UNMUTE_CH4, TAS5404_0C_CTRL5, 0x8, 0x0, CTL_NORMAL_TYPE},
    {TAS5404_0C_10_UNMUTE, TAS5404_0C_CTRL5, 0x10, 0x0, CTL_NORMAL_TYPE},
    {TAS5404_0C_20_DC_SD, TAS5404_0C_CTRL5, 0x20, 0x0, CTL_NORMAL_TYPE},
    {TAS5404_0C_80_RESET, TAS5404_0C_CTRL5, 0x80, 0x0, CTL_NORMAL_TYPE},
    {TAS5404_0D_01_LOW_LOW_CH1, TAS5404_0D_CTRL6, 0x1, 0x0, CTL_NORMAL_TYPE},
    {TAS5404_0D_02_LOW_LOW_CH2, TAS5404_0D_CTRL6, 0x2, 0x0, CTL_NORMAL_TYPE},
    {TAS5404_0D_04_LOW_LOW_CH3, TAS5404_0D_CTRL6, 0x4, 0x0, CTL_NORMAL_TYPE},
    {TAS5404_0D_08_LOW_LOW_CH4, TAS5404_0D_CTRL6, 0x8, 0x0, CTL_NORMAL_TYPE},
    {TAS5404_0D_10_CH12_PBTL, TAS5404_0D_CTRL6, 0x10, 0x0, CTL_NORMAL_TYPE},
    {TAS5404_0D_20_CH34_PBTL, TAS5404_0D_CTRL6, 0x20, 0x0, CTL_NORMAL_TYPE},
    {TAS5404_10_03_DC_DETECT_VAL, TAS5404_10_CTRL7, 0x3, 0x0, CTL_NORMAL_TYPE},
    {TAS5404_10_04_CROSSTALK, TAS5404_10_CTRL7, 0x4, 0x0, CTL_NORMAL_TYPE},
    {
        TAS5404_10_08_DELAY20MS_LOAD_DIAG, TAS5404_10_CTRL7, 0x8, 0x0,
        CTL_NORMAL_TYPE
    },
    {TAS5404_10_10_S2P_S2G_4X, TAS5404_10_CTRL7, 0x10, 0x0, CTL_NORMAL_TYPE},
    {TAS5404_10_20_CM_RAMP_SPEED, TAS5404_10_CTRL7, 0x20, 0x0, CTL_NORMAL_TYPE},
    {TAS5404_10_80_SLOW_CM_MUTE, TAS5404_10_CTRL7, 0x80, 0x0, CTL_NORMAL_TYPE},
    {
        TAS5404_13_01_FOLDBACK_CH1, TAS5404_13_STAT_REG5, 0x1, 0x0,
        CTL_NORMAL_TYPE
    },
    {
        TAS5404_13_02_FOLDBACK_CH2, TAS5404_13_STAT_REG5, 0x2, 0x0,
        CTL_NORMAL_TYPE
    },
    {
        TAS5404_13_04_FOLDBACK_CH3, TAS5404_13_STAT_REG5, 0x4, 0x0,
        CTL_NORMAL_TYPE
    },
    {
        TAS5404_13_08_FOLDBACK_CH4, TAS5404_13_STAT_REG5, 0x8, 0x0,
        CTL_NORMAL_TYPE
    },
    {
        TAS5404_13_10_OTSHUTDOWN_CH1, TAS5404_13_STAT_REG5, 0x10, 0x0,
        CTL_NORMAL_TYPE
    },
    {
        TAS5404_13_20_OTSHUTDOWN_CH2, TAS5404_13_STAT_REG5, 0x20, 0x0,
        CTL_NORMAL_TYPE
    },
    {
        TAS5404_13_40_OTSHUTDOWN_CH3, TAS5404_13_STAT_REG5, 0x40, 0x0,
        CTL_NORMAL_TYPE
    },
    {
        TAS5404_13_80_OTSHUTDOWN_CH4, TAS5404_13_STAT_REG5, 0x80, 0x0,
        CTL_NORMAL_TYPE
    },
    {
        TAS5404_14_01_VIR_FAULT_N, TAS5404_14_VIR_FAULT_N, 0x1, 0x0,
        CTL_GPIO_TYPE
    },
    {
        TAS5404_15_01_VIR_CLIP_OTW_N, TAS5404_15_VIR_CLIP_OTW_N, 0x1, 0x0,
        CTL_GPIO_TYPE
    },
};

/**
 * @brief tas5404 initialize function
 *
 * @param dev
 * @return true
 * @return false
 */
static bool sdrv_tas5404_initialize(am_codec_t *dev)
{
    if (!dev) {
        printf(" %s dev is null \n", __FUNCTION__);
        return false;
    }

    return true;
}

static int tas5404_write_vir_reg(am_codec_t *dev, unsigned int reg,
                                 unsigned int val)
{
    am_ctl_t *am_ctl = am_codec_find_virctl(dev, reg);
    bool ret = false;

    if (!am_ctl) {
        printf(" %s can't find am_ctl \n", __FUNCTION__);
        return false;
    }

    if (am_ctl->put) {
        ret = am_ctl->put(dev, am_ctl->ctl_id, val);
        ssdk_printf(CODEC_TAS5404_DBG_PRT, "[tas5404] %s (VIR%XH, %XH)\n",
                __FUNCTION__, reg, val);
        return ret;
    }

    return true;
}

static int tas5404_read_vir_reg(am_codec_t *dev, unsigned int reg,
                                unsigned int *val)
{
    am_ctl_t *am_ctl = am_codec_find_virctl(dev, reg);
    bool ret = false;

    if (!am_ctl) {
        printf(" %s can't find am_ctl \n", __FUNCTION__);
        return false;
    }

    if (am_ctl->get) {
        ret = am_ctl->get(dev, am_ctl->ctl_id, val);
        ssdk_printf(CODEC_TAS5404_DBG_PRT, "[TAS5404] %s (VIR%XH, %XH)\n",
                __FUNCTION__, reg, *val);
        return ret;
    }

    return true;
}

/**
 * @brief read register from tas5404
 *
 * @param dev
 * @param reg
 * @param val
 *@return true
 * @return false
 */
static bool sdrv_tas5404_read_reg(am_codec_t *dev, unsigned int reg,
                                  unsigned int *val)
{
    tas5404_drvdata_t *drvdata = (tas5404_drvdata_t *)am_codec_get_drvdata(dev);
    unsigned char tx[3] = {0}, rx[3] = {0};
    int wlen, rlen;
    int ret;
    wlen = 1;
    rlen = 1;

    if (!drvdata) {
        printf(" %s err drvdata is null \n", __FUNCTION__);
        return false;
    }

    if (reg >= TAS5404_14_VIR_FAULT_N) {
        return tas5404_read_vir_reg(dev, reg, val);
    }

    tx[0] = (unsigned char)(reg & 0xFF);
    ret = i2c_write_read(drvdata->handle, drvdata->addr, tx, wlen, rx,
                                rlen);
    ssdk_printf(CODEC_TAS5404_DBG_PRT, "%s: read tas5404 reg=0x%x,val=0x%x\n",
            __func__, reg, rx[0]);

    if (ret < 0) {
        ssdk_printf(SSDK_ERR, "%s: error: read tas5404 reg=%x,ret=%x\n", __func__, reg,
                ret);
        return false;
    }

    *val = rx[0];
    return true;
}

/**
 * @brief write value to tas5404
 *
 * @param dev
 * @param reg
 * @param val
 * @return true
 * @return false
 */
static bool sdrv_tas5404_write_reg(am_codec_t *dev, unsigned int reg,
                                   unsigned int val)
{
    tas5404_drvdata_t *drvdata = (tas5404_drvdata_t *)am_codec_get_drvdata(dev);
    uint8_t addr, buf;
    int ret;

    if (!drvdata) {
        printf(" %s err drvdata is null \n", __FUNCTION__);
        return false;
    }

    ssdk_printf(CODEC_TAS5404_DBG_PRT, "%s: write: reg=%x, val=%x\n", __func__, reg,
            val);

    if (reg >= TAS5404_14_VIR_FAULT_N) {
        return tas5404_write_vir_reg(dev, reg, val);
    }

    addr = (uint8_t)(reg & 0xff);
    buf = (uint8_t)(val & 0xff);

    ret = i2c_write_reg8(drvdata->handle, drvdata->addr, addr, &buf, 1);

    if (ret < 0) {
        ssdk_printf(0, "%s: error: reg=%x, val=%x\n", __func__, reg, val);
        return false;
    }

    return true;
}

static bool sdrv_tas5404_writeable_reg(am_codec_t *dev, unsigned int reg)
{
    if (!dev) {
        printf(" %s err dev is null \n", __FUNCTION__);
        return false;
    }

    /* readable return true */
    switch (reg) {
        /* next writeable return true*/
        case TAS5404_08_CTRL1:
        case TAS5404_09_CTRL2:
        case TAS5404_0A_CTRL3:
        case TAS5404_0B_CTRL4:
        case TAS5404_0C_CTRL5:
        case TAS5404_0D_CTRL6:
        case TAS5404_10_CTRL7:
            return true;

        default:
            return false;
    }
}

static bool sdrv_tas5404_readable_reg(am_codec_t *dev, unsigned int reg)
{
    if (!dev) {
        printf(" %s dev is null \n", __FUNCTION__);
        return false;
    }

    return true;
}

static am_codec_ops_t tas5404_drv_ops = {
    .initialize = sdrv_tas5404_initialize,
    .read_reg = sdrv_tas5404_read_reg,
    .write_reg = sdrv_tas5404_write_reg,
    .writeable_reg = sdrv_tas5404_writeable_reg,
    .readable_reg = sdrv_tas5404_readable_reg,
};

am_codec_t *tas5404_codec_create(const char *codec_name)
{
    am_codec_t *codec = NULL;
    unsigned int regs_numb = ARRAY_SIZE(tas5404_regs);
    unsigned int ctls_numb = ARRAY_SIZE(tas5404_ctls);

    codec = am_codec_create(codec_name, &tas5404_drv_ops, tas5404_regs,
                            regs_numb, tas5404_ctls, ctls_numb);

    if (!codec) {
        ssdk_printf(0, "\t[tas5404] faild creat codec \r\n");
    }

    return codec;
}

void tas5404_codec_destroy(am_codec_t *codec)
{
    if (codec) {
        am_codec_destroy(codec);
    }
}
