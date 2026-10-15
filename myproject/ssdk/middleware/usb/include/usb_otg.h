/**
 * @file usb_otg.h
 * @brief usb otg support header file.
 *
 * @copyright Copyright (c) 2022  Semidrive Semiconductor.
 *            All rights reserved.
 */

#ifndef USB_OTG_H
#define USB_OTG_H


#include <cpu.h>


void USB_OTG_Start(CPU_INT08U otg_ctl_nbr, CPU_INT08U otg_nbr_dev);


#endif
