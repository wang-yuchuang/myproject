/**
 * @file dw_uart_drv.h
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: dw uart dev interface.
 *
 * Revision History:
 * -----------------
 */

#ifndef INCLUDE_DRV_DW_UART_H
#define INCLUDE_DRV_DW_UART_H

#include <types.h>
#include <compiler.h>

#include "driver/uart.h"

#ifndef ASSEMBLY

__BEGIN_CDECLS

typedef struct dw_uart_dev {
    uart_dev_t           dev;
    uint32_t             fcr;
} dw_uart_dev_t;

/**
 * @brief dw uart probe
 *
 * @param[in] dw_uart dw uart dev
 * @param[in] cfg uart config
 * @return dev uart dev
 */
uart_dev_t *dw_uart_probe(dw_uart_dev_t *dw_uart, const uart_config_t *cfg);

__END_CDECLS

#endif

#endif
