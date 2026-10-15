/**
 * @file uart_lin_drv.h
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#ifndef INCLUDE_DRV_UART_LIN_H
#define INCLUDE_DRV_UART_LIN_H

#include <types.h>
#include <compiler.h>

#include <driver/lin.h>
#include <driver/uart.h>

#ifndef ASSEMBLY

__BEGIN_CDECLS

/* Break & Sync fields */
#define BREAK                   0x00u
#define SYNC                    0x55u

typedef struct uart_lin_pdu {
    uint8_t         pid;
    uint8_t         cs;
    uint8_t         drc;
    uint8_t         dl;
    uint8_t         check_sum;
    uint8_t         sdu_buf[LIN_FRAME_DATA_LENGTH];
} uart_lin_pdu_t;

typedef enum lin_frame_state {
    LIN_SEND_BREAK              = 0x01u,
    LIN_SEND_SYNC               = 0x02u,
    LIN_SEND_ID                 = 0x03u,
    LIN_SYNC_ERROR              = 0x04u,
    LIN_ID_ERROR                = 0x05u,
    LIN_MASTER_RESPONE_DATA     = 0x06u,
    LIN_SLAVE_RESPONE_DATA      = 0x07u,
    LIN_BUS_SLEEP               = 0x08u,
    LIN_IDLE                    = 0x09u,
    LIN_MASTER_CHECKSUM_ERROR   = 0x0Au,
    LIN_SLAVE_CHECKSUM_ERROR    = 0x0Bu,
    LIN_NOT_READY               = 0x0Cu
} lin_frame_state_t;

/* uart lin dev */
typedef struct uart_lin_dev {
    lin_dev_t           dev;
    uart_dev_t          *uart_dev;
    uint32_t            sub_state;
    lin_frame_state_t   frame_state;
    osEventFlagsId_t    complete;
    uart_lin_pdu_t      pdu;
    bool                master_response;
    uint8_t             master_length;
    bool                slave_response;
    uint8_t             slave_length;
    uint8_t             rx_buf[LIN_FRAME_DATA_LENGTH];
    char                rxdata;
} uart_lin_dev_t;

/**
 * @brief uart lin probe
 *
 * @param[in] dev lin dev
 * @param[in] uart_dev uart dev
 * @param[in] cfg lin config
 */
lin_dev_t *uart_lin_probe(uart_lin_dev_t *uart_lin, uart_dev_t *uart_dev,
                          lin_config_t *cfg);

__END_CDECLS

#endif

#endif
