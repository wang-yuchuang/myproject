/**
 * @file app_usbd.h
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: usb device MSC test & demo application.
 *
 * Revision History:
 * -----------------
 */


#ifndef  APP_USBD_MODULE_PRESENT
#define  APP_USBD_MODULE_PRESENT


#include  <stdbool.h>


bool  App_USBD_MSC_Init(unsigned char   dev_nbr,
                        unsigned char   cfg_hs,
                        unsigned char   cfg_fs);

#endif

