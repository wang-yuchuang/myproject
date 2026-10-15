/**
 * @file ssdrv_uart_drv.h
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: sdrv uart driver interface.
 *
 * Revision History:
 * -----------------
 */

#ifndef INCLUDE_DRV_SDRV_UART_H
#define INCLUDE_DRV_SDRV_UART_H

#include <types.h>
#include <compiler.h>

#include "driver/uart.h"

#ifndef ASSEMBLY

__BEGIN_CDECLS

/* sdrv uart dev */
typedef struct sdrv_uart_dev {
    uart_dev_t          dev;
    uint32_t            state;
    uint32_t            abr_cnt;
} sdrv_uart_dev_t;

/**
 * @brief sdrv uart probe
 *
 * @param[in] sdrv_uart sdrv uart dev
 * @param[in] cfg uart config
 * @return uart dev
 */
uart_dev_t *sdrv_uart_probe(sdrv_uart_dev_t *sdrv_uart, const uart_config_t *cfg);

__END_CDECLS

#endif

#endif
