/********************************************************
 *        Copyright(c) 2020    Semidrive                *
 *        All rights reserved.                          *
 ********************************************************/

#ifndef __MCU_XTAL_H__
#define __MCU_XTAL_H__

//#include <Mcal.h>
#include "Mcal.h"

#define XTAL_SETTING_OFF    0x0U
#define BM_XTAL_SETTING_XTAL_LOCK    (0x01U << 31U)

typedef enum {
    SEL_OSC,
    SEL_XTAL1,
    SEL_XTAL2,
} osc_out_sel_e;

typedef enum {
    FSRC_SAF,
    FSRC_SEC,
    FSRC_HIS,
    FSRC_HPI,
    FSRC_DDR,
} fsrefclk_sel_e;

boolean ana_is_xtal_ready(uint32 base);
void ana_osc24_output_sel(uint32 fsrc, osc_out_sel_e sel);
osc_out_sel_e ana_osc24_get_sel(uint32 fsrc);

/* 10,000 clk (at most) needed for the xtal to be locked */
#define XTAL_LOCK_TIMEOUT_us    4200

#endif  /* __MCU_XTAL_H__ */
