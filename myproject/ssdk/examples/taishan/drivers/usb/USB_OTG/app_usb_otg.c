/**
 * @file app_usb_otg.c
 *
 * Copyright (c) 2022 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: usb OTG HID test & demo application.
 *
 * Revision History:
 * -----------------
 */


#include  "app_usbd.h"
#include  "app_usbh.h"
#include  <board.h>
#include  <usb_bsp.h>
#include  <usb_otg.h>
#include  <CLI_type.h>


static int usb_otg_app(int argc, char *argv[])
{
    static char run_once = 0;
    int dev_nbr;

    if (run_once) {
        return CLI_FALSE;
    }

    run_once = 1;

    dev_nbr = App_USBD_Init();
    if (dev_nbr < 0)
    {
        printf("usb device start fail\n");
    }

    if (!App_USBH_Init())
    {
        printf("usb host start fail\n");
    }

    if (dev_nbr >= 0) {
        USB_OTG_Start(0, dev_nbr);
    }

    return CLI_FALSE;
}

CLI_CMD("usb_otg", "usb_otg:\n usb OTG HID example application\n", usb_otg_app);

