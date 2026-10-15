/**
 * @file usb_bsp.h
 * @brief usb board support header file.
 *
 * @copyright Copyright (c) 2021  Semidrive Semiconductor.
 *            All rights reserved.
 */

#ifndef BOARDS_USB_BSP_H
#define BOARDS_USB_BSP_H


#include  <usb_bsp_e3.h>


/*
 *  Restriction on wait cycle: The cycle of continuous accesses to registers of this core must be at least 67 ns.
 *  200MHz APB Clock * 67nS = 13.4
 *  Set BWAIT to 12, which means 12 wait cycles (14 access cycles).
 */
#define USB_CFG_BUSWAIT         (12)

/* If USB_OC pin is used and low level input corresponds to overcurrent, set USB_OC_INVERT to 0. */
#define USB_OC_INVERT           1


#endif
