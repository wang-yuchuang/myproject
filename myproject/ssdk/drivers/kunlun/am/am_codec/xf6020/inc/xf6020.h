/**
 * @file xf6020.h
 * @brief xf6020 header file.
 *
 * @copyright Copyright (c) 2021  Semidrive Semiconductor.
 *            All rights reserved.
 */

#ifndef __XF6020_H__
#define __XF6020_H__

#include "am_codec.h"

/**
 * @brief XF6020 Register Definition
 *
 */
#define XF6020_00_VOICE_MODE 0x0
#define XF6020_01_PARAM 0x1
#define XF6020_02_BYPASS 0x2
#define XF6020_0C_VERSION 0xc
#define XF6020_0D_VIR_RESET 0xd

/**
 * @brief XF6020 control definition
 *
 */
enum {
    XF6020_00_FF_VOICEMODE,
    XF6020_01_FF_PARAM,
    XF6020_02_FF_BYPASS,
    XF6020_0C_FF_VERSION,
    XF6020_0D_01_RESET,
    XF6020_CTL_NUMB,

};

/**
 * @brief voice mode definition.
 *
 */
typedef enum {
    FOUR_SND_AREAS = 0, /* four sound areas */
    TWO_SND_AREAS = 1,  /* two sound areas */
} XF6020VoiceMode;

/**
 * @brief xf6020's parameters
 *
 */
typedef enum {
    AEC_TEL = 0,
    AEC_MUSIC = 1,
    USE_BEAM_0 = 2,
    USE_BEAM_1 = 3,
    USE_BEAM_2 = 4,
    USE_BEAM_3 = 5,
    USE_BEAM_OFF = 6,

    WAKEUP_TWO = 7,
    WAKEUP_FOUR = 8,
    WAKEUP_OFF = 9,

    REC_BEAM = 10,
    REC_MAE = 11,
    REC_OFF = 12,

    DUAL_MAE = 13,
    DUAL_MAB = 14,
    DUAL_MAE_MICL = 15,
    DUAL_MAE_MICR = 16,
    DUAL_MAE_REFL = 17,
    DUAL_MAE_REFR = 18,
    DUAL_MAE_REFMIX = 19,
    DUAL_PHONE = 20,
    PARAME_VALUE_MAX
} XF6020ParamValue;

typedef struct xf6020_drvdata {
    void *handle;
    uint8_t addr;
    codec_protocol_t protocol;
} xf6020_drvdata_t;

am_codec_t *xf6020_codec_create(const char *codec_name);
void xf6020_codec_destroy(am_codec_t *codec);

#endif
