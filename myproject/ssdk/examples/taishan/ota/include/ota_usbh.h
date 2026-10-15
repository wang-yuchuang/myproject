/**
 * @file ota_usbh.h
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: ota from usb host application.
 *
 * Revision History:
 * -----------------
 */


#ifndef  APP_USBH_MODULE_PRESENT
#define  APP_USBH_MODULE_PRESENT


#include  <cpu.h>
#include  <lib_def.h>
#include  <cpu.h>
#include  <lib_mem.h>
#include  <Class/MSC/usbh_msc.h>

#define  APP_CFG_USBH_MSC_EN                    DEF_ENABLED


CPU_BOOLEAN  OTA_USBH_Init (void);
extern osEventFlagsId_t udisk_ready;

#endif
