/**
 * @file ms_a03_dsp2_addr.h
 * @brief ms_a03_dsp2_addr header file.
 *
 * @copyright Copyright (c) 2021  Semidrive Semiconductor.
 *            All rights reserved.
 */

#ifndef _AK77XX_DSP2_ADDR_INCLUDED
#define _AK77XX_DSP2_ADDR_INCLUDED

// Component: SFADER_ADC1 (fader_mul2_1)
#define     CRAM_ADDR_LEVEL_SFADER_ADC1     0x0000
#define     CRAM_ADDR_ATT_TIME_SFADER_ADC1  0x0001
#define     CRAM_ADDR_REL_TIME_SFADER_ADC1  0x0003

// Component: HPF2_1 (biquad2_1)
#define     CRAM_ADDR_BAND1_HPF2_1  0x0006

// Component: SFADER_ADC2 (fader_mul2_2)
#define     CRAM_ADDR_LEVEL_SFADER_ADC2     0x0010
#define     CRAM_ADDR_ATT_TIME_SFADER_ADC2  0x0011
#define     CRAM_ADDR_REL_TIME_SFADER_ADC2  0x0013

// Component: HPF2_2 (biquad2_2)
#define     CRAM_ADDR_BAND1_HPF2_2  0x0016

// Component: SWITCH_ADC2L_ECHOREF (mixerl2_1)
#define     CRAM_ADDR_VOL_IN1_SWITCH_ADC2L_ECHOREF  0x0020
#define     CRAM_ADDR_VOL_IN2_SWITCH_ADC2L_ECHOREF  0x0021

// Component: SWITCH_ADC2R_ECHOREF (mixerl2_2)
#define     CRAM_ADDR_VOL_IN1_SWITCH_ADC2R_ECHOREF  0x0022
#define     CRAM_ADDR_VOL_IN2_SWITCH_ADC2R_ECHOREF  0x0023

// Component: SFADER_FM_DIG (fader_mul2_3)
#define     CRAM_ADDR_LEVEL_SFADER_FM_DIG   0x0024
#define     CRAM_ADDR_ATT_TIME_SFADER_FM_DIG    0x0025
#define     CRAM_ADDR_REL_TIME_SFADER_FM_DIG    0x0027

// Component: SFADER_TBOX_DIG (fader_mul2_4)
#define     CRAM_ADDR_LEVEL_SFADER_TBOX_DIG     0x002A
#define     CRAM_ADDR_ATT_TIME_SFADER_TBOX_DIG  0x002B
#define     CRAM_ADDR_REL_TIME_SFADER_TBOX_DIG  0x002D

// Component: SFADER_FM_ANA (fader_mul2_5)
#define     CRAM_ADDR_LEVEL_SFADER_FM_ANA   0x0030
#define     CRAM_ADDR_ATT_TIME_SFADER_FM_ANA    0x0031
#define     CRAM_ADDR_REL_TIME_SFADER_FM_ANA    0x0033

// Component: SFADER_TBOX_ANA (fader_mul2_6)
#define     CRAM_ADDR_LEVEL_SFADER_TBOX_ANA     0x0036
#define     CRAM_ADDR_ATT_TIME_SFADER_TBOX_ANA  0x0037
#define     CRAM_ADDR_REL_TIME_SFADER_TBOX_ANA  0x0039

// Component: MIXER4_1 (mixerl3u.4_1)
#define     CRAM_ADDR_VOL_IN1_MIXER4_1  0x003C
#define     CRAM_ADDR_VOL_IN2_MIXER4_1  0x003D
#define     CRAM_ADDR_VOL_IN3_MIXER4_1  0x003E
#define     CRAM_ADDR_VOL_IN4_MIXER4_1  0x003F

// Component: MIXER4_2 (mixerl3u.4_2)
#define     CRAM_ADDR_VOL_IN1_MIXER4_2  0x0040
#define     CRAM_ADDR_VOL_IN2_MIXER4_2  0x0041
#define     CRAM_ADDR_VOL_IN3_MIXER4_2  0x0042
#define     CRAM_ADDR_VOL_IN4_MIXER4_2  0x0043

#endif //#end of _AK77XX_DSP_ADDR_INCLUDED
