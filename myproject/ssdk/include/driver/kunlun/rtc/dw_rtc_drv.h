/**
 * @file dw_rtc_drv.h
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: dw rtc drv interface.
 *
 * Revision History:
 * -----------------
 */

#ifndef INCLUDE_DW_RTC_DRV_H
#define INCLUDE_DW_RTC_DRV_H

#include <types.h>
#include <compiler.h>

#include "driver/rtc.h"

#ifndef ASSEMBLY

__BEGIN_CDECLS

/**
 * @brief
 *
 */
rtc_dev_t *dw_rtc_probe(rtc_dev_t *dev, const rtc_config_t *cfg);

__END_CDECLS

#endif

#endif
