/*
 * eic.h
 *
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */
#ifndef __KUNLUN_EIC_H__
#define __KUNLUN_EIC_H__

#include <assert.h>
#include <stdbool.h>

/*
 * EIC modules. These EIC modules are same IPs with different
 * enable and injection signals.
 *
 * NOTE - do not modify the order of this enum.
 */
enum eic {
    KUNLUN_EIC_SAF = 0,
    KUNLUN_EIC_SEC,
    KUNLUN_EIC_HPI,
    KUNLUN_EIC_VSN,
    KUNLUN_EIC_MAX,
};

/*
 * EIC signal id format:
 *
 *  [31:24] eic_id (see enum eic)
 *  [23:16] injection point (0 ~ 95)
 *  [15:0]  injection bit (0 ~ 511)
 */
enum eic_signal;

#include "kunlun_eic_hw.h"


void eic_enable_injectpoint(enum eic eic, uint32_t point,
                            bool enable);
void eic_inject(enum eic_signal signal, bool inject);

#endif
