/**
 * @file sdrv_watchdog_drv.h
 * @brief semidrive watchdog driver header.
 *
 * @copyright Copyright (c) 2021  Semidrive Semiconductor.
 *            All rights reserved.
 */
#ifndef SDRV_WATCHDOG_DRV_H_
#define SDRV_WATCHDOG_DRV_H_

#include "driver/watchdog.h"
#include "driver/irq.h"

typedef enum {
    WATCHDOG1,
    WATCHDOG2,
    WATCHDOG3,
    WATCHDOG4,
    WATCHDOG5,
    WATCHDOG6,
} watchdog_id_e;

typedef enum {
    EXT_RST_LEVEL,
    EXT_RST_PULSE,
} ext_rst_mode_e;

typedef enum {
    EXT_RST_LOW_ACTIVE,
    EXT_RST_HIGH_ACTIVE,
} ext_rst_pol_e;

typedef struct sdrv_watchdog_device {
    paddr_t       base;
    int32_t       irq;
    irq_handler   isr;
    void          *isr_arg;

    watchdog_id_e wdt_id;
    bool          int_rst;

    ext_rst_mode_e ext_rst_mode;
    ext_rst_pol_e  ext_rst_pol;
    bool           ext_rst;
} sdrv_watchdog_device_t;

int32_t sdrv_watchdog_init(struct watchdog_device *wdd,
                           struct sdrv_watchdog_device *sdrv_wdd);

#endif /* SDRV_WATCHDOG_DRV_H_ */