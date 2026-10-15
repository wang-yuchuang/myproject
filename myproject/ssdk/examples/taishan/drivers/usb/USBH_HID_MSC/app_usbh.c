/**
 * @file app_usbh.c
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: USB Host HID+MSC test & demo application.
 *
 * Revision History:
 * -----------------
 */


/*
*********************************************************************************************************
*                                             INCLUDE FILES
*********************************************************************************************************
*/


#include  <cmsis_os2.h>
#include  <Source/usbh_core.h>
#include  <usb_bsp.h>
#include  "app_usbh.h"


/*
*********************************************************************************************************
*                                              LOCAL DEFINES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                         LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                 USB HOST STACK INTERNAL TASK'S STACKS
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                           LOCAL CONSTANTS
*********************************************************************************************************
*/

USBH_KERNEL_TASK_INFO  AsyncTaskInfo = {    /* ---------------- INFO ON ASYNC TASK ---------------- */
    osPriorityHigh,                         /* Async task priority.                                 */
    NULL,                                   /* Ptr to async task stack.                             */
    4096u                                   /* Size of async task stack.                            */
};

USBH_KERNEL_TASK_INFO  HubTaskInfo = {      /* ----------------- INFO ON HUB TASK ----------------- */
    osPriorityHigh1,                        /* Hub task priority.                                   */
    NULL,                                   /* Ptr to hub task stack.                               */
    4096u                                   /* Size of hub task stack.                              */
};

USBH_HC_CFG  USB0H_EHCI_Cfg_TAISHAN = {
    (CPU_ADDR)DEVICE_BASE(USB0H) + RUSB_EHCI_OFFSET,        /* Base addr of host controller hw registers.           */
    (CPU_ADDR)0x00000000u,                                  /* Base addr of host controller dedicated mem.          */
              0u,                                           /* Size      of host controller dedicated mem.          */
              DEF_ENABLED,                                  /* Does HC can access sys mem?                          */
              0x1000,                                       /* Data buf max len.                                    */
              12u,                                          /* Max nbr opened bulk EP.                              */
              12u,                                          /* Max nbr opened intr EP.                              */
              4u                                            /* Max nbr opened isoc EP.                              */
};

USBH_HC_CFG  USB0H_OHCI_Cfg_TAISHAN = {
    (CPU_ADDR)DEVICE_BASE(USB0H) + RUSB_OHCI_OFFSET,        /* Base addr of host controller hw registers.           */
    (CPU_ADDR)0x00000000u,                                  /* Base addr of host controller dedicated mem.          */
              0u,                                           /* Size      of host controller dedicated mem.          */
              DEF_ENABLED,                                  /* Does HC can access sys mem?                          */
              0x1000,                                       /* Data buf max len.                                    */
              12u,                                          /* Max nbr opened bulk EP.                              */
              12u,                                          /* Max nbr opened intr EP.                              */
              4u                                            /* Max nbr opened isoc EP.                              */
};


/*
*********************************************************************************************************
*                                        LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                       LOCAL CONFIGURATION ERRORS
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                            App_USBH_Init()
*
* Description : Initialize USB Host Stack and additional demos.
*
* Argument(s) : None.
*
* Return(s)   : DEF_OK      if successfull.
*               DEF_FAIL    otherwise.
*
* Note(s)     : None.
*********************************************************************************************************
*/

CPU_BOOLEAN  App_USBH_Init (void)
{
    USBH_ERR    err;
    CPU_INT08U  ohci_hc_nbr;
    CPU_INT08U  ehci_hc_nbr;


    printf("\n");
    printf("=============================\n");
    printf("=  USB HOST INITIALIZATION  =\n");
    printf("=============================\n");

    err = USBH_Init(&AsyncTaskInfo,
                    &HubTaskInfo);
    if (err != USBH_ERR_NONE) {
        printf("...could not initialize USB HOST Stack  w/err = %d\n\n", err);
        return (DEF_FAIL);
    }

#if (APP_CFG_USBH_MSC_EN == DEF_ENABLED)
    printf("... Initiliazing HOST Mass Storage class ...\n");
    err = App_USBH_MSC_Init();

    if (err != USBH_ERR_NONE) {
        printf("...could not initialize HOST Mass Storage Class w/err = %d\n\n", err);
        return (DEF_FAIL);
    }
#endif

#if (APP_CFG_USBH_HID_EN == DEF_ENABLED)
    printf("... Initiliazing HOST Human Interface Device class ...\n");
    err = App_USBH_HID_Init();

    if (err != USBH_ERR_NONE) {
        printf("...could not initialize HID class w/err = %d\n\n", err);
        return (DEF_FAIL);
    }
#endif

    ehci_hc_nbr = USBH_HC_Add ( &USB0H_EHCI_Cfg_TAISHAN,
                                &EHCI_DrvAPI,
                                &EHCI_RH_API,
                                &USB0H_EHCI_BSP_TAISHAN,
                                &err );
    if (err != USBH_ERR_NONE) {
        printf("...could not add EHCI controller w/err = %d\n\n", err);
        return (DEF_FAIL);
    }

    ohci_hc_nbr = USBH_HC_Add ( &USB0H_OHCI_Cfg_TAISHAN,
                                &OHCI_DrvAPI,
                                &OHCI_RH_API,
                                &USB0H_OHCI_BSP_TAISHAN,
                                &err );
    if (err != USBH_ERR_NONE) {
        printf("...could not add OHCI controller w/err = %d\n\n", err);
        return (DEF_FAIL);
    }

    /* Note: Start OHCI before EHCI! */
    err = USBH_HC_Start(ohci_hc_nbr);
    if (err != USBH_ERR_NONE) {
        printf("...could not start OHCI controller w/err = %d\n\n", err);
        return (DEF_FAIL);
    }

    err = USBH_HC_Start(ehci_hc_nbr);
    if (err != USBH_ERR_NONE) {
        printf("...could not start EHCI controller w/err = %d\n\n", err);
        return (DEF_FAIL);
    }

    return (DEF_OK);
}


#include <CLI_type.h>

static int usbh_hm_app(int argc, char *argv[])
{
    static char run_once = 0;

    if (!run_once) {
        run_once = 1;

        if (!App_USBH_Init())
        {
            printf("usb host start fail\n");
        }
    }

    return CLI_FALSE;
}

CLI_CMD("usbh_hm", "usbh_hm:\n usb host HID+MSC example application\n", usbh_hm_app);

