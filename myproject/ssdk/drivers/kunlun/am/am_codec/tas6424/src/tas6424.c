/**
 * @file tas6424.c
 * @brief tas6424 source file.
 *
 * @copyright Copyright (c) 2021  Semidrive Semiconductor.
 *            All rights reserved.
 */

#include <debug.h>
#include <param.h>
#include "tas6424.h"
#include "driver/i2c.h"

#define CODEC_TAS6424_DBG_PRT SSDK_DEBUG

/**
 * @brief register value table
 *
 */
static am_reg_t tas6424_regs[] = {
    {TAS6424_00_MODE_CTRL, 0x0, CACHE_INVALID},
    {TAS6424_01_MISC_CTRL_1, 0x0, CACHE_INVALID},
    {TAS6424_02_MISC_CTRL_2, 0x0, CACHE_INVALID},
    {TAS6424_03_SAP_CTRL, 0x0, CACHE_INVALID},
    {TAS6424_04_CHANNEL_STATE_CTRL, 0x0, CACHE_INVALID},
    {TAS6424_05_CHANNEL_1_VOLUME_CTRL, 0x0, CACHE_INVALID},
    {TAS6424_06_CHANNEL_2_VOLUME_CTRL, 0x0, CACHE_INVALID},
    {TAS6424_07_CHANNEL_3_VOLUME_CTRL, 0x0, CACHE_INVALID},
    {TAS6424_08_CHANNEL_4_VOLUME_CTRL, 0x0, CACHE_INVALID},
    {TAS6424_09_DC_DIAG_CTRL_1, 0x0, CACHE_INVALID},
    {TAS6424_0A_DC_DIAG_CTRL_2, 0x0, CACHE_INVALID},
    {TAS6424_0B_DC_DIAG_CTRL_3, 0x0, CACHE_INVALID},
    {TAS6424_0C_DC_LOAD_DIAG_REPORT_1, 0x0, CACHE_INVALID},
    {TAS6424_0D_DC_LOAD_DIAG_REPORT_2, 0x0, CACHE_INVALID},
    {TAS6424_0E_DC_LOAD_DIAG_REPORT_3LINE_OUTPUT, 0x0, CACHE_INVALID},
    {TAS6424_0F_CHANNEL_STATE_REPORTING, 0x0, CACHE_INVALID},
    {TAS6424_10_CHANNEL_FAULTS, 0x0, CACHE_INVALID},
    {TAS6424_11_GLOBAL_FAULTS_1, 0x0, CACHE_INVALID},
    {TAS6424_12_GLOBAL_FAULTS_2, 0x0, CACHE_INVALID},
    {TAS6424_13_WARNINGS, 0x0, CACHE_INVALID},
    {TAS6424_14_PIN_CONTROL, 0x0, CACHE_INVALID},
    {TAS6424_15_AC_LOAD_DIAG_CTRL_1, 0x0, CACHE_INVALID},
    {TAS6424_16_AC_LOAD_DIAG_CTRL_2, 0x0, CACHE_INVALID},
    {TAS6424_29_VIR_STANDBY_N, 0x0, CACHE_INVALID},
    {TAS6424_30_VIR_MUTE_N, 0x0, CACHE_INVALID},
    {TAS6424_31_VIR_FAULT_N, 0x0, CACHE_INVALID},
    {TAS6424_32_VIR_WARN_N, 0x0, CACHE_INVALID},
};

/**
 * @brief tas6424 control attribute table
 *
 */
static am_ctl_t tas6424_ctls[] = {
    {TAS6424_00_80_RESET, TAS6424_00_MODE_CTRL, 0x80, 0x0, CTL_NORMAL_TYPE},
    {TAS6424_00_20_PBTL_CH34, TAS6424_00_MODE_CTRL, 0x20, 0x0, CTL_NORMAL_TYPE},
    {TAS6424_00_10_PBTL_CH12, TAS6424_00_MODE_CTRL, 0x10, 0x0, CTL_NORMAL_TYPE},
    {
        TAS6424_00_08_CH1_LO_MODE, TAS6424_00_MODE_CTRL, 0x8, 0x0,
        CTL_NORMAL_TYPE
    },
    {
        TAS6424_00_04_CH2_LO_MODE, TAS6424_00_MODE_CTRL, 0x4, 0x0,
        CTL_NORMAL_TYPE
    },
    {
        TAS6424_00_02_CH3_LO_MODE, TAS6424_00_MODE_CTRL, 0x2, 0x0,
        CTL_NORMAL_TYPE
    },
    {
        TAS6424_00_01_CH4_LO_MODE, TAS6424_00_MODE_CTRL, 0x1, 0x0,
        CTL_NORMAL_TYPE
    },
    {
        TAS6424_01_80_HPF_BYPASS, TAS6424_01_MISC_CTRL_1, 0x80, 0x0,
        CTL_NORMAL_TYPE
    },
    {
        TAS6424_01_60_OTW_CONTROL, TAS6424_01_MISC_CTRL_1, 0x60, 0x0,
        CTL_NORMAL_TYPE
    },
    {
        TAS6424_01_10_OC_CONTROL, TAS6424_01_MISC_CTRL_1, 0x10, 0x0,
        CTL_NORMAL_TYPE
    },
    {
        TAS6424_01_0C_VOLUME_RATE, TAS6424_01_MISC_CTRL_1, 0xc, 0x0,
        CTL_NORMAL_TYPE
    },
    {TAS6424_01_01_GAIN, TAS6424_01_MISC_CTRL_1, 0x1, 0x0, CTL_NORMAL_TYPE},
    {
        TAS6424_02_70_PWM_FREQUENCY, TAS6424_02_MISC_CTRL_2, 0x70, 0x0,
        CTL_NORMAL_TYPE
    },
    {TAS6424_02_04_SDM_OSR, TAS6424_02_MISC_CTRL_2, 0x4, 0x0, CTL_NORMAL_TYPE},
    {
        TAS6424_02_03_OUTPUT_PHASE, TAS6424_02_MISC_CTRL_2, 0x3, 0x0,
        CTL_NORMAL_TYPE
    },
    {
        TAS6424_03_C0_INPUT_SAMPLING_RATE, TAS6424_03_SAP_CTRL, 0xc0, 0x0,
        CTL_NORMAL_TYPE
    },
    {
        TAS6424_03_20_8_CH_TDM_SLOT_SELECT, TAS6424_03_SAP_CTRL, 0x20, 0x0,
        CTL_NORMAL_TYPE
    },
    {
        TAS6424_03_10_TDM_SLOT_SIZE, TAS6424_03_SAP_CTRL, 0x10, 0x0,
        CTL_NORMAL_TYPE
    },
    {
        TAS6424_03_08_TDM_SLOT_SELECT_2, TAS6424_03_SAP_CTRL, 0x8, 0x0,
        CTL_NORMAL_TYPE
    },
    {
        TAS6424_03_07_INPUT_FORMAT, TAS6424_03_SAP_CTRL, 0x7, 0x0,
        CTL_NORMAL_TYPE
    },
    {
        TAS6424_04_C0_CH1_STATE_CONTROL, TAS6424_04_CHANNEL_STATE_CTRL, 0xc0, 0x0,
        CTL_NORMAL_TYPE
    },
    {
        TAS6424_04_30_CH2_STATE_CONTROL, TAS6424_04_CHANNEL_STATE_CTRL, 0x30, 0x0,
        CTL_NORMAL_TYPE
    },
    {
        TAS6424_04_0C_CH3_STATE_CONTROL, TAS6424_04_CHANNEL_STATE_CTRL, 0xc, 0x0,
        CTL_NORMAL_TYPE
    },
    {
        TAS6424_04_03_CH4_STATE_CONTROL, TAS6424_04_CHANNEL_STATE_CTRL, 0x3, 0x0,
        CTL_NORMAL_TYPE
    },
    {
        TAS6424_05_FF_CH1_VOLUME, TAS6424_05_CHANNEL_1_VOLUME_CTRL, 0xff, 0x0,
        CTL_GAIN_TYPE
    },
    {
        TAS6424_06_FF_CH2_VOLUME, TAS6424_06_CHANNEL_2_VOLUME_CTRL, 0xff, 0x0,
        CTL_GAIN_TYPE
    },
    {
        TAS6424_07_FF_CH3_VOLUME, TAS6424_07_CHANNEL_3_VOLUME_CTRL, 0xff, 0x0,
        CTL_GAIN_TYPE
    },
    {
        TAS6424_08_FF_CH4_VOLUME, TAS6424_08_CHANNEL_4_VOLUME_CTRL, 0xff, 0x0,
        CTL_GAIN_TYPE
    },
    {
        TAS6424_09_80_DC_LDG_ABORT, TAS6424_09_DC_DIAG_CTRL_1, 0x80, 0x0,
        CTL_NORMAL_TYPE
    },
    {
        TAS6424_09_40_2X_RAMP, TAS6424_09_DC_DIAG_CTRL_1, 0x40, 0x0,
        CTL_NORMAL_TYPE
    },
    {
        TAS6424_09_20_2X_SETTLE, TAS6424_09_DC_DIAG_CTRL_1, 0x20, 0x0,
        CTL_NORMAL_TYPE
    },
    {
        TAS6424_09_02_LDG_LO_ENABLE, TAS6424_09_DC_DIAG_CTRL_1, 0x2, 0x0,
        CTL_NORMAL_TYPE
    },
    {
        TAS6424_09_01_LDG_BYPASS, TAS6424_09_DC_DIAG_CTRL_1, 0x1, 0x0,
        CTL_NORMAL_TYPE
    },
    {
        TAS6424_0A_F0_CH1_DC_LDG_SL, TAS6424_0A_DC_DIAG_CTRL_2, 0xf0, 0x0,
        CTL_NORMAL_TYPE
    },
    {
        TAS6424_0A_0F_CH2_DC_LDG_SL, TAS6424_0A_DC_DIAG_CTRL_2, 0xf, 0x0,
        CTL_NORMAL_TYPE
    },
    {
        TAS6424_0B_F0_CH3_DC_LDG_SL, TAS6424_0B_DC_DIAG_CTRL_3, 0xf0, 0x0,
        CTL_NORMAL_TYPE
    },
    {
        TAS6424_0B_0F_CH4_DC_LDG_SL, TAS6424_0B_DC_DIAG_CTRL_3, 0xf, 0x0,
        CTL_NORMAL_TYPE
    },
    {
        TAS6424_0C_80_CH1_S2G, TAS6424_0C_DC_LOAD_DIAG_REPORT_1, 0x80, 0x0,
        CTL_NORMAL_TYPE
    },
    {
        TAS6424_0C_40_CH1_S2P, TAS6424_0C_DC_LOAD_DIAG_REPORT_1, 0x40, 0x0,
        CTL_NORMAL_TYPE
    },
    {
        TAS6424_0C_20_CH1_OL, TAS6424_0C_DC_LOAD_DIAG_REPORT_1, 0x20, 0x0,
        CTL_NORMAL_TYPE
    },
    {
        TAS6424_0C_10_CH1_SL, TAS6424_0C_DC_LOAD_DIAG_REPORT_1, 0x10, 0x0,
        CTL_NORMAL_TYPE
    },
    {
        TAS6424_0C_08_CH2_S2G, TAS6424_0C_DC_LOAD_DIAG_REPORT_1, 0x8, 0x0,
        CTL_NORMAL_TYPE
    },
    {
        TAS6424_0C_04_CH2_S2P, TAS6424_0C_DC_LOAD_DIAG_REPORT_1, 0x4, 0x0,
        CTL_NORMAL_TYPE
    },
    {
        TAS6424_0C_02_CH2_OL, TAS6424_0C_DC_LOAD_DIAG_REPORT_1, 0x2, 0x0,
        CTL_NORMAL_TYPE
    },
    {
        TAS6424_0C_01_CH2_SL, TAS6424_0C_DC_LOAD_DIAG_REPORT_1, 0x1, 0x0,
        CTL_NORMAL_TYPE
    },
    {
        TAS6424_0D_80_CH3_S2G, TAS6424_0D_DC_LOAD_DIAG_REPORT_2, 0x80, 0x0,
        CTL_NORMAL_TYPE
    },
    {
        TAS6424_0D_40_CH3_S2P, TAS6424_0D_DC_LOAD_DIAG_REPORT_2, 0x40, 0x0,
        CTL_NORMAL_TYPE
    },
    {
        TAS6424_0D_20_CH3_OL, TAS6424_0D_DC_LOAD_DIAG_REPORT_2, 0x20, 0x0,
        CTL_NORMAL_TYPE
    },
    {
        TAS6424_0D_10_CH3_SL, TAS6424_0D_DC_LOAD_DIAG_REPORT_2, 0x10, 0x0,
        CTL_NORMAL_TYPE
    },
    {
        TAS6424_0D_08_CH4_S2G, TAS6424_0D_DC_LOAD_DIAG_REPORT_2, 0x8, 0x0,
        CTL_NORMAL_TYPE
    },
    {
        TAS6424_0D_04_CH4_S2P, TAS6424_0D_DC_LOAD_DIAG_REPORT_2, 0x4, 0x0,
        CTL_NORMAL_TYPE
    },
    {
        TAS6424_0D_02_CH4_OL, TAS6424_0D_DC_LOAD_DIAG_REPORT_2, 0x2, 0x0,
        CTL_NORMAL_TYPE
    },
    {
        TAS6424_0D_01_CH4_SL, TAS6424_0D_DC_LOAD_DIAG_REPORT_2, 0x1, 0x0,
        CTL_NORMAL_TYPE
    },
    {
        TAS6424_0E_08_CH1_LO_LDG, TAS6424_0E_DC_LOAD_DIAG_REPORT_3LINE_OUTPUT, 0x8,
        0x0, CTL_NORMAL_TYPE
    },
    {
        TAS6424_0E_04_CH2_LO_LDG, TAS6424_0E_DC_LOAD_DIAG_REPORT_3LINE_OUTPUT, 0x4,
        0x0, CTL_NORMAL_TYPE
    },
    {
        TAS6424_0E_02_CH3_LO_LDG, TAS6424_0E_DC_LOAD_DIAG_REPORT_3LINE_OUTPUT, 0x2,
        0x0, CTL_NORMAL_TYPE
    },
    {
        TAS6424_0E_01_CH4_LO_LDG, TAS6424_0E_DC_LOAD_DIAG_REPORT_3LINE_OUTPUT, 0x1,
        0x0, CTL_NORMAL_TYPE
    },
    {
        TAS6424_0F_C0_CH1_STATE_REPORT, TAS6424_0F_CHANNEL_STATE_REPORTING, 0xc0,
        0x0, CTL_NORMAL_TYPE
    },
    {
        TAS6424_0F_30_CH2_STATE_REPORT, TAS6424_0F_CHANNEL_STATE_REPORTING, 0x30,
        0x0, CTL_NORMAL_TYPE
    },
    {
        TAS6424_0F_0C_CH3_STATE_REPORT, TAS6424_0F_CHANNEL_STATE_REPORTING, 0xc,
        0x0, CTL_NORMAL_TYPE
    },
    {
        TAS6424_0F_03_CH4_STATE_REPORT, TAS6424_0F_CHANNEL_STATE_REPORTING, 0x3,
        0x0, CTL_NORMAL_TYPE
    },
    {
        TAS6424_10_80_CH1_OC, TAS6424_10_CHANNEL_FAULTS, 0x80, 0x0,
        CTL_NORMAL_TYPE
    },
    {
        TAS6424_10_40_CH2_OC, TAS6424_10_CHANNEL_FAULTS, 0x40, 0x0,
        CTL_NORMAL_TYPE
    },
    {
        TAS6424_10_20_CH3_OC, TAS6424_10_CHANNEL_FAULTS, 0x20, 0x0,
        CTL_NORMAL_TYPE
    },
    {
        TAS6424_10_10_CH4_OC, TAS6424_10_CHANNEL_FAULTS, 0x10, 0x0,
        CTL_NORMAL_TYPE
    },
    {
        TAS6424_10_08_CH1_DC, TAS6424_10_CHANNEL_FAULTS, 0x8, 0x0,
        CTL_NORMAL_TYPE
    },
    {
        TAS6424_10_04_CH2_DC, TAS6424_10_CHANNEL_FAULTS, 0x4, 0x0,
        CTL_NORMAL_TYPE
    },
    {
        TAS6424_10_02_CH3_DC, TAS6424_10_CHANNEL_FAULTS, 0x2, 0x0,
        CTL_NORMAL_TYPE
    },
    {
        TAS6424_10_01_CH4_DC, TAS6424_10_CHANNEL_FAULTS, 0x1, 0x0,
        CTL_NORMAL_TYPE
    },
    {
        TAS6424_11_10_INVALID_CLOCK, TAS6424_11_GLOBAL_FAULTS_1, 0x10, 0x0,
        CTL_NORMAL_TYPE
    },
    {
        TAS6424_11_08_PVDD_OV, TAS6424_11_GLOBAL_FAULTS_1, 0x8, 0x0,
        CTL_NORMAL_TYPE
    },
    {
        TAS6424_11_04_VBAT_OV, TAS6424_11_GLOBAL_FAULTS_1, 0x4, 0x0,
        CTL_NORMAL_TYPE
    },
    {
        TAS6424_11_02_PVDD_UV, TAS6424_11_GLOBAL_FAULTS_1, 0x2, 0x0,
        CTL_NORMAL_TYPE
    },
    {
        TAS6424_11_01_VBAT_UV, TAS6424_11_GLOBAL_FAULTS_1, 0x1, 0x0,
        CTL_NORMAL_TYPE
    },
    {
        TAS6424_12_10_OTSD, TAS6424_12_GLOBAL_FAULTS_2, 0x10, 0x0,
        CTL_NORMAL_TYPE
    },
    {
        TAS6424_12_08_CH1_OTSD, TAS6424_12_GLOBAL_FAULTS_2, 0x8, 0x0,
        CTL_NORMAL_TYPE
    },
    {
        TAS6424_12_04_CH2_OTSD, TAS6424_12_GLOBAL_FAULTS_2, 0x4, 0x0,
        CTL_NORMAL_TYPE
    },
    {
        TAS6424_12_02_CH3_OTSD, TAS6424_12_GLOBAL_FAULTS_2, 0x2, 0x0,
        CTL_NORMAL_TYPE
    },
    {
        TAS6424_12_01_CH4_OTSD, TAS6424_12_GLOBAL_FAULTS_2, 0x1, 0x0,
        CTL_NORMAL_TYPE
    },
    {TAS6424_13_20_VDD_POR, TAS6424_13_WARNINGS, 0x20, 0x0, CTL_NORMAL_TYPE},
    {TAS6424_13_10_OTW, TAS6424_13_WARNINGS, 0x10, 0x0, CTL_NORMAL_TYPE},
    {TAS6424_13_08_OTW_CH1, TAS6424_13_WARNINGS, 0x8, 0x0, CTL_NORMAL_TYPE},
    {TAS6424_13_04_OTW_CH2, TAS6424_13_WARNINGS, 0x4, 0x0, CTL_NORMAL_TYPE},
    {TAS6424_13_02_OTW_CH3, TAS6424_13_WARNINGS, 0x2, 0x0, CTL_NORMAL_TYPE},
    {TAS6424_13_01_OTW_CH4, TAS6424_13_WARNINGS, 0x1, 0x0, CTL_NORMAL_TYPE},
    {TAS6424_14_80_MASK_OC, TAS6424_14_PIN_CONTROL, 0x80, 0x0, CTL_NORMAL_TYPE},
    {
        TAS6424_14_40_MASK_OTSD, TAS6424_14_PIN_CONTROL, 0x40, 0x0,
        CTL_NORMAL_TYPE
    },
    {TAS6424_14_20_MASK_UV, TAS6424_14_PIN_CONTROL, 0x20, 0x0, CTL_NORMAL_TYPE},
    {TAS6424_14_10_MASK_OV, TAS6424_14_PIN_CONTROL, 0x10, 0x0, CTL_NORMAL_TYPE},
    {TAS6424_14_08_MASK_DC, TAS6424_14_PIN_CONTROL, 0x8, 0x0, CTL_NORMAL_TYPE},
    {
        TAS6424_14_04_MASK_ILIMIT, TAS6424_14_PIN_CONTROL, 0x4, 0x0,
        CTL_NORMAL_TYPE
    },
    {
        TAS6424_14_02_MASK_CLIP, TAS6424_14_PIN_CONTROL, 0x2, 0x0,
        CTL_NORMAL_TYPE
    },
    {TAS6424_14_01_MASK_OTW, TAS6424_14_PIN_CONTROL, 0x1, 0x0, CTL_NORMAL_TYPE},
    {
        TAS6424_15_80_CH12_PBTL12_GAIN, TAS6424_15_AC_LOAD_DIAG_CTRL_1, 0x80, 0x0,
        CTL_NORMAL_TYPE
    },
    {
        TAS6424_15_20_CH34_PBTL34_GAIN, TAS6424_15_AC_LOAD_DIAG_CTRL_1, 0x20, 0x0,
        CTL_NORMAL_TYPE
    },
    {
        TAS6424_15_08_CH1_ENABLE, TAS6424_15_AC_LOAD_DIAG_CTRL_1, 0x8, 0x0,
        CTL_NORMAL_TYPE
    },
    {
        TAS6424_15_04_CH2_ENABLE, TAS6424_15_AC_LOAD_DIAG_CTRL_1, 0x4, 0x0,
        CTL_NORMAL_TYPE
    },
    {
        TAS6424_15_02_CH3_ENABLE, TAS6424_15_AC_LOAD_DIAG_CTRL_1, 0x2, 0x0,
        CTL_NORMAL_TYPE
    },
    {
        TAS6424_15_01_CH4_ENABLE, TAS6424_15_AC_LOAD_DIAG_CTRL_1, 0x1, 0x0,
        CTL_NORMAL_TYPE
    },
    {
        TAS6424_16_80_AC_DIAGS_LOOPBACK, TAS6424_16_AC_LOAD_DIAG_CTRL_2, 0x80, 0x0,
        CTL_NORMAL_TYPE
    },
    {
        TAS6424_16_10_AC_TIMING, TAS6424_16_AC_LOAD_DIAG_CTRL_2, 0x10, 0x0,
        CTL_NORMAL_TYPE
    },
    {
        TAS6424_16_0C_AC_CURRENT, TAS6424_16_AC_LOAD_DIAG_CTRL_2, 0xc, 0x0,
        CTL_NORMAL_TYPE
    },
    {
        TAS6424_29_01_VIR_STANDBY_N, TAS6424_29_VIR_STANDBY_N, 0x1, 0x0,
        CTL_GPIO_TYPE
    },
    {TAS6424_30_01_VIR_MUTE_N, TAS6424_30_VIR_MUTE_N, 0x1, 0x0, CTL_GPIO_TYPE},
    {
        TAS6424_31_01_VIR_FAULT_N, TAS6424_31_VIR_FAULT_N, 0x1, 0x0,
        CTL_GPIO_TYPE
    },
    {TAS6424_32_01_VIR_WARN_N, TAS6424_32_VIR_WARN_N, 0x1, 0x0, CTL_GPIO_TYPE},
};

/**
 * @brief tas6424 initialize function
 *
 * @param dev
 * @return true
 * @return false
 */
static bool sdrv_tas6424_initialize(am_codec_t *dev) { return true; }

static int tas6424_write_vir_reg(am_codec_t *dev, unsigned int reg,
                                 unsigned int val)
{
    am_ctl_t *am_ctl = am_codec_find_virctl(dev, reg);
    bool ret = false;

    if (!am_ctl) {
        ssdk_printf(SSDK_ERR, " %s can't find am_ctl\n", __FUNCTION__);
        return false;
    }

    if (am_ctl->put) {
        ret = am_ctl->put(dev, am_ctl->ctl_id, val);
        ssdk_printf(CODEC_TAS6424_DBG_PRT, "[tas6424] %s (VIR%XH, %XH)\n",
                __FUNCTION__, reg, val);
        return ret;
    }

    return true;
}

static int tas6424_read_vir_reg(am_codec_t *dev, unsigned int reg,
                                unsigned int *val)
{
    am_ctl_t *am_ctl = am_codec_find_virctl(dev, reg);
    bool ret = false;

    if (!am_ctl) {
        ssdk_printf(SSDK_ERR, " %s can't find am_ctl\n", __FUNCTION__);
        return false;
    }

    if (am_ctl->get) {
        ret = am_ctl->get(dev, am_ctl->ctl_id, val);

        ssdk_printf(CODEC_TAS6424_DBG_PRT, "[tas6424] %s (VIR%XH, %XH)\n",
                __FUNCTION__, reg, *val);
        return ret;
    }

    return true;
}

/**
 * @brief read register from tas6424
 *
 * @param dev
 * @param reg
 * @param val
 * @return true
 * @return false
 */
static bool sdrv_tas6424_read_reg(am_codec_t *dev, unsigned int reg,
                                  unsigned int *val)
{
    tas6424_drvdata_t *drvdata = (tas6424_drvdata_t *)am_codec_get_drvdata(dev);
    unsigned char tx[3] = {0}, rx[3] = {0};
    int wlen, rlen;
    int ret;
    wlen = 1;
    rlen = 1;

    if (!drvdata) {
        ssdk_printf(SSDK_ERR, " %s drvdata is null\n", __FUNCTION__);
        return false;
    }

    if (reg >= TAS6424_29_VIR_STANDBY_N) {
        return tas6424_read_vir_reg(dev, reg, val);
    }

    tx[0] = (unsigned char)(reg & 0x7F);
    ret = i2c_read_reg8(drvdata->handle, drvdata->addr, tx[0], &rx[0], 1);

    if (ret < 0) {
        ssdk_printf(SSDK_ERR, "%s: error: read tas6424 reg=%x,ret=%x\n", __func__, reg,
                ret);
        return false;
    }

    *val = rx[0];
    return true;
}

/**
 * @brief write value to tas6424
 *
 * @param dev
 * @param reg
 * @param val
 * @return true
 * @return false
 */
static bool sdrv_tas6424_write_reg(am_codec_t *dev, unsigned int reg,
                                   unsigned int val)
{
    tas6424_drvdata_t *drvdata = (tas6424_drvdata_t *)am_codec_get_drvdata(dev);
    uint8_t addr, buf;
    int ret;

    if (!drvdata) {
        ssdk_printf(SSDK_ERR, " %s drvdata is null\n", __FUNCTION__);
        return false;
    }

    if (reg >= TAS6424_29_VIR_STANDBY_N) {
        return tas6424_write_vir_reg(dev, reg, val);
    }

    addr = (uint8_t)(reg & 0xff);
    buf = (uint8_t)(val & 0xff);
    ret = i2c_write_reg8(drvdata->handle, drvdata->addr, addr, &buf, 1);

    if (ret < 0) {
        ssdk_printf(SSDK_ERR, "%s: error: reg=%x, val=%x\n", __func__, reg, val);
        return false;
    }

    return true;
}

static bool sdrv_tas6424_writeable_reg(am_codec_t *dev, unsigned int reg)
{
    if (!dev) {
        ssdk_printf(SSDK_ERR, " %s dev is null\n", __FUNCTION__);
        return false;
    }

    /* readable return true */
    switch (reg) {
        /* next writeable return true */
        case TAS6424_00_MODE_CTRL:
        case TAS6424_01_MISC_CTRL_1:
        case TAS6424_02_MISC_CTRL_2:
        case TAS6424_03_SAP_CTRL:
        case TAS6424_04_CHANNEL_STATE_CTRL:
        case TAS6424_05_CHANNEL_1_VOLUME_CTRL:
        case TAS6424_06_CHANNEL_2_VOLUME_CTRL:
        case TAS6424_07_CHANNEL_3_VOLUME_CTRL:
        case TAS6424_08_CHANNEL_4_VOLUME_CTRL:
        case TAS6424_09_DC_DIAG_CTRL_1:
        case TAS6424_0A_DC_DIAG_CTRL_2:
        case TAS6424_0B_DC_DIAG_CTRL_3:
        case TAS6424_14_PIN_CONTROL:
        case TAS6424_15_AC_LOAD_DIAG_CTRL_1:
        case TAS6424_16_AC_LOAD_DIAG_CTRL_2:
        case TAS6424_29_VIR_STANDBY_N:
        case TAS6424_30_VIR_MUTE_N:
            return true;

        default:
            return false;
    }
}

static bool sdrv_tas6424_readable_reg(am_codec_t *dev, unsigned int reg)
{
    if (!dev) {
        ssdk_printf(SSDK_ERR, " %s dev is null\n", __FUNCTION__);
        return false;
    }

    /* readable return true */
    /* readable return true */
    switch (reg) {
        /* next writeable return true*/
        case TAS6424_29_VIR_STANDBY_N:
        case TAS6424_30_VIR_MUTE_N:
            return false;

        default:
            return true;
    }

    return true;
}

static am_codec_ops_t tas6424_drv_ops = {
    .initialize = sdrv_tas6424_initialize,
    .read_reg = sdrv_tas6424_read_reg,
    .write_reg = sdrv_tas6424_write_reg,
    .writeable_reg = sdrv_tas6424_writeable_reg,
    .readable_reg = sdrv_tas6424_readable_reg,

};

am_codec_t *tas6424_codec_create(const char *codec_name)
{

    unsigned int regs_numb = ARRAY_SIZE(tas6424_regs);
    unsigned int ctls_numb = ARRAY_SIZE(tas6424_ctls);
    am_codec_t *codec = NULL;

    codec = am_codec_create(codec_name, &tas6424_drv_ops, tas6424_regs,
                            regs_numb, tas6424_ctls, ctls_numb);

    if (!codec) {
        ssdk_printf(SSDK_ERR, "\t[tas6424] faild create codec \r\n");
    }

    return codec;
}

void tas6424_codec_destroy(am_codec_t *codec)
{
    if (codec) {
        am_codec_destroy(codec);
    }
}
