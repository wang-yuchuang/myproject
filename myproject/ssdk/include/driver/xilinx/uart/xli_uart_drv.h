/**
 * @file xli_uart_drv.h
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: Xilinx uart driver interface.
 *
 * Revision History:
 * -----------------
 */

#ifndef INCLUDE_DRV_XLI_UART_H
#define INCLUDE_DRV_XLI_UART_H

#include <types.h>
#include <compiler.h>

#include "driver/uart.h"

#ifndef ASSEMBLY

__BEGIN_CDECLS

/* xli ttc dev */
typedef struct xli_uart_dev {
    uart_dev_t          dev;
} xli_uart_dev_t;

/**
 * @brief xli uart probe
 *
 * @param[in] xli_uart xli uart dev
 * @param[in] cfg uart config
 * @return dev uart dev
 */
uart_dev_t *xli_uart_probe(xli_uart_dev_t *xli_uart, const uart_config_t *cfg);

__END_CDECLS

#endif

#endif
