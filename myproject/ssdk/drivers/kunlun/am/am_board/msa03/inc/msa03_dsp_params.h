/**
 * @file msa03_dsp_params.h
 * @brief msa03_dsp_params header file.
 *
 * @copyright Copyright (c) 2021  Semidrive Semiconductor.
 *            All rights reserved.
 */

#ifndef __MSA03_DSP_PARAMS_H__
#define __MSA03_DSP_PARAMS_H__
#include "ms_a03_dsp1_addr.h"
#include "ms_a03_dsp2_addr.h"

#define HIFI_PLAYBACK_MEIDA_CH12_GAIN       CRAM_ADDR_LEVEL_SFADER_MEDIA_12
#define HIFI_PLAYBACK_MEIDA_CH34_GAIN       CRAM_ADDR_LEVEL_SFADER_MEDIA_34
#define HIFI_PLAYBACK_PHONE_GAIN            CRAM_ADDR_LEVEL_SFADER_PHONE
#define HIFI_PLAYBACK_NAV_GAIN              CRAM_ADDR_LEVEL_SFADER_NAV
#define HIFI_CAPTURE_CH12_GAIN              CRAM_ADDR_LEVEL_SFADER_ADC1
#define HIFI_CAPTURE_CH34_GAIN              CRAM_ADDR_LEVEL_SFADER_ADC2
#define SAFETY_PLAYBACK_GAIN                CRAM_ADDR_LEVEL_FADER_SAF
#define BT_PLAYBACK_GAIN                    CRAM_ADDR_LEVEL_SFADER_PHONE
#define BT_CAPTURE_GAIN                     CRAM_ADDR_LEVEL_SFADER_ADC1
#define CLUSTER_PLAYBACK_GAIN               CRAM_ADDR_LEVEL_SFADER_CLUSTER
#define CLUSTER_CAPTURE_CH12_GAIN           CRAM_ADDR_LEVEL_SFADER_ADC1
#define FM_PLAYBACK_GAIN                    CRAM_ADDR_LEVEL_SFADER_FM_DIG
#define TBOX_PLAYBACK_GAIN                  CRAM_ADDR_LEVEL_SFADER_TBOX_DIG
#endif