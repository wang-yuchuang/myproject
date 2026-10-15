/**
 * @file sdrv_lin_drv.h
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#ifndef INCLUDE_DRV_SDRV_LIN_H
#define INCLUDE_DRV_SDRV_LIN_H

#include <types.h>
#include <compiler.h>

#include <cmsis_os2.h>
#include <driver/lin.h>

#ifndef ASSEMBLY

__BEGIN_CDECLS

typedef struct sdrv_lin_pdu {
    uint8_t         pid;
    uint8_t         cs;
    uint8_t         drc;
    uint8_t         dl;
    uint8_t         tx_sdu[LIN_PDU_LENGTH];
    uint8_t         rx_pdu[LIN_FRAME_DATA_LENGTH];
} sdrv_lin_pdu_t;

/* sdrv lin dev */
typedef struct sdrv_lin_dev {
    lin_dev_t           dev;
    uint32_t            sub_state;
    osEventFlagsId_t    complete;
    sdrv_lin_pdu_t      pdu;
} sdrv_lin_dev_t;

/**
 * @brief sdrv lin probe
 *
 * @param[in] dev lin dev
 */
lin_dev_t *sdrv_lin_probe(sdrv_lin_dev_t *sdrv_lin);

__END_CDECLS

#endif

#endif
