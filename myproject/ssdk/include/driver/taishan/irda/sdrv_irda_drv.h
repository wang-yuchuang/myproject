/**
 * @file sdrv_irda_drv.h
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#ifndef INCLUDE_DRV_SDRV_IRDA_H
#define INCLUDE_DRV_SDRV_IRDA_H

#include <types.h>
#include <compiler.h>

#include <cmsis_os2.h>
#include <driver/irda.h>

#ifndef ASSEMBLY

__BEGIN_CDECLS

/* sdrv irda dev */
typedef struct sdrv_irda_dev {
    irda_dev_t          dev;
    uint8_t             status;
    osEventFlagsId_t    complete;
    irda_cb_t           cb;
    void                *arg;
    char                *rx_buffer;
    uint8_t             rx_len;
} sdrv_irda_dev_t;

/**
 * @brief sdrv irda probe
 *
 * @param[in] dev irda dev
 */
irda_dev_t *sdrv_irda_probe(sdrv_irda_dev_t *sdrv_irda);

__END_CDECLS

#endif

#endif
