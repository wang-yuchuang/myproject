/**
 * @file ms_a03_dsp1_addr.h
 * @brief ms_a03_dsp1_addr header file.
 *
 * @copyright Copyright (c) 2021  Semidrive Semiconductor.
 *            All rights reserved.
 */

#ifndef _AK77XX_DSP1_ADDR_INCLUDED
#define _AK77XX_DSP1_ADDR_INCLUDED

// Component: SFADER_CLUSTER (fader_mul2_1)
#define     CRAM_ADDR_LEVEL_SFADER_CLUSTER  0x0000
#define     CRAM_ADDR_ATT_TIME_SFADER_CLUSTER   0x0001
#define     CRAM_ADDR_REL_TIME_SFADER_CLUSTER   0x0003

// Component: FADER_SAF (fader_mul2_2)
#define     CRAM_ADDR_LEVEL_FADER_SAF   0x0006
#define     CRAM_ADDR_ATT_TIME_FADER_SAF    0x0007
#define     CRAM_ADDR_REL_TIME_FADER_SAF    0x0009

// Component: SFADER_MEDIA_12 (fader_mul2_3)
#define     CRAM_ADDR_LEVEL_SFADER_MEDIA_12     0x000C
#define     CRAM_ADDR_ATT_TIME_SFADER_MEDIA_12  0x000D
#define     CRAM_ADDR_REL_TIME_SFADER_MEDIA_12  0x000F

// Component: MEDIA_GEQs_CH12 (biquad2_1)
#define     CRAM_ADDR_BAND1_MEDIA_GEQs_CH12     0x0012
#define     CRAM_ADDR_BAND2_MEDIA_GEQs_CH12     0x001C
#define     CRAM_ADDR_BAND3_MEDIA_GEQs_CH12     0x0021
#define     CRAM_ADDR_BAND4_MEDIA_GEQs_CH12     0x0026
#define     CRAM_ADDR_BAND5_MEDIA_GEQs_CH12     0x002B

// Component: SFADER_PHONE (fader_mul2_4)
#define     CRAM_ADDR_LEVEL_SFADER_PHONE    0x0030
#define     CRAM_ADDR_ATT_TIME_SFADER_PHONE     0x0031
#define     CRAM_ADDR_REL_TIME_SFADER_PHONE     0x0033

// Component: SFADER_NAV (fader_mul2_5)
#define     CRAM_ADDR_LEVEL_SFADER_NAV  0x0036
#define     CRAM_ADDR_ATT_TIME_SFADER_NAV   0x0037
#define     CRAM_ADDR_REL_TIME_SFADER_NAV   0x0039

// Component: MIXER6_1 (mixerl3u.6_1)
#define     CRAM_ADDR_VOL_IN1_MIXER6_1  0x003C
#define     CRAM_ADDR_VOL_IN2_MIXER6_1  0x003D
#define     CRAM_ADDR_VOL_IN3_MIXER6_1  0x003E
#define     CRAM_ADDR_VOL_IN4_MIXER6_1  0x003F
#define     CRAM_ADDR_VOL_IN5_MIXER6_1  0x0040
#define     CRAM_ADDR_VOL_IN6_MIXER6_1  0x0041

// Component: MIXER6_2 (mixerl3u.6_2)
#define     CRAM_ADDR_VOL_IN1_MIXER6_2  0x0042
#define     CRAM_ADDR_VOL_IN2_MIXER6_2  0x0043
#define     CRAM_ADDR_VOL_IN3_MIXER6_2  0x0044
#define     CRAM_ADDR_VOL_IN4_MIXER6_2  0x0045
#define     CRAM_ADDR_VOL_IN5_MIXER6_2  0x0046
#define     CRAM_ADDR_VOL_IN6_MIXER6_2  0x0047

// Component: Limiter2_1 (limiter2_1)
#define     CRAM_ADDR_ATT_TIME_Limiter2_1   0x004A
#define     CRAM_ADDR_REL_TIME_Limiter2_1   0x004C
#define     CRAM_ADDR_THRESHOLD_Limiter2_1  0x004F

// Component: SFADER_MEDIA_34 (fader_mul2_6)
#define     CRAM_ADDR_LEVEL_SFADER_MEDIA_34     0x0052
#define     CRAM_ADDR_ATT_TIME_SFADER_MEDIA_34  0x0053
#define     CRAM_ADDR_REL_TIME_SFADER_MEDIA_34  0x0055

// Component: MEDIA_GEQs_CH34 (biquad2_2)
#define     CRAM_ADDR_BAND1_MEDIA_GEQs_CH34     0x0058
#define     CRAM_ADDR_BAND2_MEDIA_GEQs_CH34     0x0062
#define     CRAM_ADDR_BAND3_MEDIA_GEQs_CH34     0x0067
#define     CRAM_ADDR_BAND4_MEDIA_GEQs_CH34     0x006C
#define     CRAM_ADDR_BAND5_MEDIA_GEQs_CH34     0x0071

// Component: Limiter2_2 (limiter2_2)
#define     CRAM_ADDR_ATT_TIME_Limiter2_2   0x0078
#define     CRAM_ADDR_REL_TIME_Limiter2_2   0x007A
#define     CRAM_ADDR_THRESHOLD_Limiter2_2  0x007D

// Component: Limiter2_3 (limiter2_3)
#define     CRAM_ADDR_ATT_TIME_Limiter2_3   0x0082
#define     CRAM_ADDR_REL_TIME_Limiter2_3   0x0084
#define     CRAM_ADDR_THRESHOLD_Limiter2_3  0x0087

// Component: Limiter2_4 (limiter2_4)
#define     CRAM_ADDR_ATT_TIME_Limiter2_4   0x008C
#define     CRAM_ADDR_REL_TIME_Limiter2_4   0x008E
#define     CRAM_ADDR_THRESHOLD_Limiter2_4  0x0091

#endif //#end of _AK77XX_DSP_ADDR_INCLUDED
